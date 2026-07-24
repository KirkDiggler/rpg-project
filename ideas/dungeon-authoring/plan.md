# Dungeon Authoring (YAML dungeon definitions v1) — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. This team's additional law: every PR goes through Copilot ack + independent gate review before Kirk merges; posts open with `— implementer (asset-pipeline), on behalf of KirkDiggler`.

**Goal:** Authoring a dungeon becomes editing one YAML file — schema/validation/compiler in the toolkit, content files in rpg-api, with a workbench CLI to try seeds without a server.

**Architecture:** A new toolkit sub-package `encounter/dungeonspec` strictly decodes + validates a versioned YAML spec and compiles it to the existing `DungeonParams` plus an ordered spawn plan; a new `monsters.ByRef` registry makes monster refs resolvable at author time; a new `Encounter.SeedMonsters` executes the plan under the atomic-seeding invariant. rpg-api hosts content files (go:embed + dev dir override), resolves keys to bytes, and calls exactly two toolkit entry points. No proto or web changes.

**Tech Stack:** Go (toolkit modules `encounter`, `rulebooks/dnd5e`; rpg-api), `gopkg.in/yaml.v3` (strict, KnownFields), go:embed, testify.

**Spec:** `rpg-project/ideas/dungeon-authoring/design.md` (approved via rpg-project PR #117, merged 2026-07-24). One delta vs the spec, flagged: `ObstacleSpec.PreferBorder` (added by rpg-toolkit#840 after the design froze) is exposed as optional `prefer_border` in the room-obstacle schema — additive, default false, consistent with the design's honest-to-the-engine rule.

**Ground rules for every task:** TDD (write the failing test first, watch it fail, then implement); `go test -count=1 -race ./...` in the touched module before every commit; never weaken an existing assertion; commit after each green step. rpg-api extra: commit BEFORE running any `make` target (ci-checks.sh runs `git checkout -- .`), and run non-short integration (`go test -count=1 -race ./...` without `-short`) before push.

---

## Slice A — rpg-toolkit: `monsters.ByRef` registry (module `rulebooks/dnd5e`)

The smallest unblockernothing else can validate monster refs without it.

### Task A1: Registry over existing constructors

**Files:**
- Create: `rulebooks/dnd5e/monster/monsters/registry.go` (NOTE the real path: constructors live in the `monsters` package nested under `monster/` — `rulebooks/dnd5e/monsters/` does NOT exist; verify with `ls rulebooks/dnd5e/monster/monsters/` and by reading rpg-api's `crypt_monster_seed.go` import line before creating anything)
- Test: `rulebooks/dnd5e/monster/monsters/registry_test.go`
- Read first: `rulebooks/dnd5e/monster/monsters/` (constructor naming: `NewSkeleton`, `NewSkeletonCaptain`, `NewGhoul`, `NewZombie`, …) and `rulebooks/dnd5e/refs/monsters.go` (canonical refs — ids are hyphenated, e.g. `skeleton-captain`). CAVEAT: `NewGoblin` lives in the PARENT `monster` package, not the `monsters` subpackage — same signature, different import; register it via a closure at the map literal (`func(...) ... { return monster.NewGoblin(...) }`), never by moving it.

- [ ] **Step 1: Write the failing test**

```go
package monsters_test

import (
	"testing"

	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/monster/monsters"
	"github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/refs"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestByRef_EveryCanonicalMonsterResolves(t *testing.T) {
	// The canonical refs list is the contract: every published monster
	// ref must resolve to a constructor. Enumerate refs.Monsters.*
	// explicitly (there is no reflection-friendly list — that's fine,
	// this test IS the list).
	for _, ref := range []string{
		refs.Monsters.Skeleton().String(),
		refs.Monsters.SkeletonCaptain().String(),
		refs.Monsters.Ghoul().String(),
		refs.Monsters.Zombie().String(),
		refs.Monsters.Goblin().String(),
	} {
		ctor, ok := monsters.ByRef(ref)
		require.True(t, ok, "ref %q must resolve", ref)
		require.NotNil(t, ctor, "ref %q must have a constructor", ref)
	}
}

func TestByRef_UnknownRefReturnsFalse(t *testing.T) {
	ctor, ok := monsters.ByRef("dnd5e:monsters:beholder")
	assert.False(t, ok)
	assert.Nil(t, ctor)
}
```

Adjust the enumerated list to the refs that actually exist in `refs/monsters.go` — read it, don't guess. If a canonical ref exists with NO constructor, that is a real finding: exclude it here, note it in the PR body, and do not fabricate a constructor.

- [ ] **Step 2: Run to verify failure** — `cd rulebooks/dnd5e && go test ./monster/monsters/ -run TestByRef -v` → FAIL: `undefined: monsters.ByRef`

- [ ] **Step 3: Implement**

```go
package monsters

// Constructor builds a monster's full stat bundle. It matches the
// signature shared by the existing NewSkeleton/NewGhoul/... constructors —
// read one and mirror its exact signature here (they are uniform).
type Constructor = func( /* mirror the real signature */ ) /* real return */

// byRef maps each canonical monster ref string (refs.Monsters.*.String(),
// full "dnd5e:monsters:<id>" form) to its existing constructor. This is a
// lookup table over constructors that already exist — no new stat
// interpretation lives here (design.md §Monster registry).
var byRef = map[string]Constructor{
	// populate from refs.Monsters.* — one line per published monster
}

// ByRef resolves a canonical monster ref to its constructor. The bool
// reports whether the ref is known — callers validate at author/load
// time so a bad ref fails a file, never a spawn.
func ByRef(ref string) (Constructor, bool) {
	c, ok := byRef[ref]
	return c, ok
}
```

The `Constructor` alias must be the real shared signature — verify all listed constructors share it (`go doc` each); if one diverges, wrap it in a closure at the map literal rather than changing the constructor.

- [ ] **Step 4: Run to verify pass** — same command → PASS
- [ ] **Step 5: Module hygiene + commit** — `go test -count=1 -race ./...` in `rulebooks/dnd5e`; `gofmt -l .` clean; commit `feat(dnd5e): monsters.ByRef registry over existing constructors (#<issue>)`

---

## Slice B — rpg-toolkit: `encounter/dungeonspec` schema + strict decode + validation

### Task B1: Schema structs and strict YAML decode

**Files:**
- Create: `encounter/dungeonspec/spec.go` (structs), `encounter/dungeonspec/decode.go`
- Test: `encounter/dungeonspec/decode_test.go`
- Modify: `encounter/go.mod` (add `gopkg.in/yaml.v3`)

- [ ] **Step 1: Failing tests — round-trip + unknown-field strictness**

```go
package dungeonspec_test

func TestDecode_RoundTripsTheReferenceSpec(t *testing.T) {
	spec, err := dungeonspec.Decode([]byte(referenceYAML)) // fixture below
	require.NoError(t, err)
	assert.Equal(t, 1, spec.Version)
	assert.Equal(t, "sunken-crypt", spec.Key)
	assert.Equal(t, 8, spec.Height)
	require.Len(t, spec.Rooms, 4)
	assert.Equal(t, "entrance", spec.Rooms[0].ID)
	require.Len(t, spec.Connectors, 3)
	require.NotNil(t, spec.Connectors[2].Locked)
	assert.Equal(t, 12, spec.Connectors[2].Locked.DC)
}

func TestDecode_UnknownFieldFailsLoudly(t *testing.T) {
	_, err := dungeonspec.Decode([]byte("version: 1\nmosnters: []\n"))
	require.Error(t, err) // the typo class the design promises dies at load
	assert.Contains(t, err.Error(), "mosnter") // yaml.v3 names the field
}
```

The `referenceYAML` fixture is the design doc's reference example verbatim (the sunken-crypt 4-room file, §Schema v1) — copy it from `ideas/dungeon-authoring/design.md`, don't retype from memory.

- [ ] **Step 2: Run → FAIL** (`cd encounter && go test ./dungeonspec/ -v`) — package doesn't exist yet
- [ ] **Step 3: Implement structs + decode**

```go
package dungeonspec

type DungeonSpec struct {
	Version    int             `yaml:"version"`
	Key        string          `yaml:"key"`
	Name       string          `yaml:"name"`
	Theme      string          `yaml:"theme"`
	Height     int             `yaml:"height"`
	Rooms      []RoomSpec      `yaml:"rooms"`
	Connectors []ConnectorSpec `yaml:"connectors"`
}

type RoomSpec struct {
	ID        string          `yaml:"id"`
	Archetype string          `yaml:"archetype"` // entrance|chamber|corridor|boss
	Width     int             `yaml:"width"`
	Pattern   string          `yaml:"pattern,omitempty"` // ""(=empty)|empty|scattered
	Monsters  []MonsterEntry  `yaml:"monsters,omitempty"`
	Boss      *BossEntry      `yaml:"boss,omitempty"`
	Obstacles []ObstacleEntry `yaml:"obstacles,omitempty"`
}

type MonsterEntry struct {
	Ref   string `yaml:"ref"`
	Count int    `yaml:"count"`
}

type BossEntry struct {
	Ref string `yaml:"ref"`
}

type ObstacleEntry struct {
	Ref            string `yaml:"ref"`
	Count          int    `yaml:"count"`
	BlocksMovement *bool  `yaml:"blocks_movement,omitempty"` // nil => true
	BlocksLoS      *bool  `yaml:"blocks_los,omitempty"`      // nil => true
	PreferBorder   bool   `yaml:"prefer_border,omitempty"`   // post-design delta: toolkit#840
}

type ConnectorSpec struct {
	From   string      `yaml:"from"`
	To     string      `yaml:"to"`
	Locked *LockedSpec `yaml:"locked,omitempty"`
}

type LockedSpec struct {
	DC      int    `yaml:"dc"`
	Ability string `yaml:"ability"`
}
```

Decode uses `yaml.NewDecoder` with `KnownFields(true)` — that is the strictness mechanism; plain `yaml.Unmarshal` silently drops unknown fields and must not be used.

- [ ] **Step 4: Run → PASS**
- [ ] **Step 5: Commit** `feat(dungeonspec): schema structs + strict YAML decode (#<issue>)`

### Task B2: Validation, table-tested

**Files:**
- Create: `encounter/dungeonspec/validate.go`
- Test: `encounter/dungeonspec/validate_test.go`

- [ ] **Step 1: Failing table test — one row per rule, red and green side**

Every rule from design.md §Validation gets a row: version==1; key/name non-empty; ≥2 rooms, unique ids; connectors reference declared rooms and form a connected linear chain; exactly one `boss` archetype; `boss` entry only on the boss room; ref shape `module:type:id`; monster/boss refs resolve via `monsters.ByRef` (author-time failure); pattern ∈ {"", "empty", "scattered"}; height ≥ 4; every width ≥ 4; boss primary axis `min(boss.width, height) > 6`; counts ≥ 1; lock DC 1–30; lock ability is a known ability ref (validate against the `rulebooks/dnd5e/abilities` package the crypt constructor already imports). Structure:

```go
func TestValidate_Table(t *testing.T) {
	cases := []struct {
		name    string
		mutate  func(*dungeonspec.DungeonSpec) // break one rule on a valid base
		wantErr string                          // substring; "" = valid
	}{
		{"valid reference spec", func(s *dungeonspec.DungeonSpec) {}, ""},
		{"version 2 rejected", func(s *dungeonspec.DungeonSpec) { s.Version = 2 }, "unsupported spec version"},
		{"boss width 5 fails axis rule", func(s *dungeonspec.DungeonSpec) { s.Rooms[3].Width = 5 }, "primary axis"},
		{"unresolvable monster ref", func(s *dungeonspec.DungeonSpec) { s.Rooms[0].Monsters[0].Ref = "dnd5e:monsters:beholder" }, "unknown monster"},
		{"broken chain", func(s *dungeonspec.DungeonSpec) { s.Connectors[1].To = "tomb" }, "linear chain"},
		// ...one row per remaining rule, both directions where meaningful
	}
	// decode referenceYAML fresh per case, apply mutate, assert Validate error
}
```

- [ ] **Step 2: Run → FAIL** (`Validate` undefined)
- [ ] **Step 3: Implement `Validate(spec) error`** — plain sequential checks, error messages naming field + file-fixable detail. The chain check: connectors[i] must join rooms[i] and rooms[i+1] by id (the v1 generator constraint — same order the design's example uses).
- [ ] **Step 4: Run → PASS; Step 5: Commit** `feat(dungeonspec): validation — every generator constraint mirrored at load (#<issue>)`

### Task B3: Compiler → `CompiledDungeon` + crypt parity

**Files:**
- Create: `encounter/dungeonspec/compile.go`
- Test: `encounter/dungeonspec/compile_test.go`
- Create: `encounter/dungeonspec/testdata/crypt.yaml` (port of today's canonical crypt — same regions/sizes/patterns/obstacles incl. `prefer_border` flags, same lock, same monster set as `crypt_monster_seed.go`'s table: 1 skeleton entrance, skeleton-captain boss)

- [ ] **Step 1: Failing tests**

**Door-ID rule (resolves the design's open thread — this plan's decision):** the compiler GENERATES deterministic door ids from content: connector *i* gets `"door-<from>-<to>"` (e.g. `door-entrance-gallery`). No caller-supplied ids, no variadic args, works for any N connectors, stable across runs of the same spec. Door ids are per-encounter opaque handles on the wire (`Wall.id` → `Interact`), so the value change vs today's api-chosen names is behaviorally free. `Load`'s full signature is therefore `Load(raw []byte) (CompiledDungeon, error)` — nothing else.

```go
func TestLoad_CryptParity(t *testing.T) {
	compiled, err := dungeonspec.Load(cryptYAML)
	require.NoError(t, err)
	// Feed CryptDungeonParams the compiler's OWN generated ids so the
	// comparison isolates real structure (regions/pattern/obstacles/lock),
	// not id-naming:
	want := encounter.CryptDungeonParams(0,
		"door-entrance-corridor", "door-corridor-boss")
	// Field-by-field: Height, Theme, per-region ID/Archetype/Width/Pattern,
	// obstacle specs (refs, counts, flags, PreferBorder), connector lock
	// config (DoorID now included — both sides carry the generated names).
	assert.Equal(t, want.Height, compiled.Params.Height)
	// ... exhaustive
}

func TestLoad_GeneratesDeterministicDoorIDs(t *testing.T) {
	compiled, _ := dungeonspec.Load(referenceYAML) // 4 rooms, 3 connectors
	require.Len(t, compiled.Params.Connectors, 3)
	assert.Equal(t, core.EntityID("door-entrance-gallery"),
		compiled.Params.Connectors[0].DoorID)
	assert.Equal(t, core.EntityID("door-trap-crossing-tomb"),
		compiled.Params.Connectors[2].DoorID)
}

// PATTERN-DEFAULT TRAP (advisory made explicit): the engine treats an
// empty DungeonRegionParams.Pattern as PatternRandom. The SPEC's default
// is empty (design.md). The compiler must therefore map "" and "empty" →
// environments.PatternEmpty and "scattered" → environments.PatternRandom
// EXPLICITLY — never pass the zero value through. The crypt parity test
// goes red if this is wrong (CryptDungeonParams sets PatternEmpty
// explicitly), but write the mapping deliberately, not by debugging.

func TestLoad_SpawnsAreBossFirst(t *testing.T) {
	compiled, _ := dungeonspec.Load(cryptYAML)
	require.NotEmpty(t, compiled.Spawns)
	assert.Equal(t, "tomb-or-boss-room-id", compiled.Spawns[0].RoomID)
	// then entrance-to-boss chain order for the rest
}

func TestLoad_Deterministic(t *testing.T) {
	a, _ := dungeonspec.Load(cryptYAML)
	b, _ := dungeonspec.Load(cryptYAML)
	assert.Equal(t, a, b)
}
```

- [ ] **Step 2: Run → FAIL; Step 3: Implement**

```go
type CompiledDungeon struct {
	Params encounter.DungeonParams
	Spawns []SpawnInstruction // boss first, then entrance→boss chain order
}

type SpawnInstruction struct {
	RoomID     string
	MonsterRef string
	Count      int
}

// Load decodes, validates, and compiles a spec in one call — the only
// entry point callers use. Door entity ids are COMPILER-GENERATED from
// content ("door-<from>-<to>" per connector, see the Door-ID rule above) —
// callers supply nothing but the bytes.
func Load(raw []byte) (CompiledDungeon, error)
```

Boss-first ordering is defense-in-depth (design.md §Compiler); the real safety is Slice C's invariant. Note: `dungeonspec` imports `encounter` (parent package) — if Go disallows the cycle you need (encounter importing dungeonspec later), keep dungeonspec import-free of encounter internals except exported types; check the dependency direction BEFORE writing code: dungeonspec → encounter is the only direction used (api calls both).

- [ ] **Step 4: Run → PASS; Step 5: Full module suite + commit** `feat(dungeonspec): compiler — CompiledDungeon with boss-first spawn plan + crypt parity (#<issue>)`

---

## Slice C — rpg-toolkit: `Encounter.SeedMonsters` (atomic seeding)

### Task C1: The invariant test first

**Files:**
- Create: `encounter/seed_monsters.go`, `encounter/seed_monsters_test.go`
- Read first: `encounter/encounter.go` `AddMonster` (the reinforcement path: unconditional initiative append when `Mode == TurnBased`), `combat.go` `checkCombatEntry`, and `crypt_monster_seed.go` in **rpg-api** (`internal/orchestrators/lobby/`) — the fragility being retired.

- [ ] **Step 1: Failing regression test — the partial-roster bug cannot happen**

```go
func TestSeedMonsters_CombatEntryNeverSeesPartialRoster(t *testing.T) {
	// Party placed at entrance; entrance monster spawn-visible to a player;
	// boss in the boss room. Pre-fix behavior with naive per-add seeding in
	// the WRONG order: entrance add → visibility pair → combat starts →
	// boss add → boss reinforced into initiative. SeedMonsters must yield:
	// combat may start AFTER the batch, initiative contains ONLY the
	// engaged (visible) monsters, never the unseen boss.
	// Build via InitDungeon with a fixed seed + forced spawn adjacency.
}

func TestSeedMonsters_SpawnVisibleStillStartsCombat(t *testing.T) {
	// The AddPlayer-doesn't-check trap (design.md, verified on main):
	// batching must NOT lose "combat starts immediately if you spawn
	// already visible" — one checkCombatEntry evaluation after the batch.
}

func TestSeedMonsters_NPerRoomSafeCells(t *testing.T) {
	// count>1 in one room: all placed on distinct, walkable, non-door,
	// non-required-path cells; deterministic per seed. Property-style over
	// 25 seeds.
}
```

- [ ] **Step 2: Run → FAIL; Step 3: Implement**

Mechanism (the design left two candidates; this plan picks **batch-with-deferred-entry** because it keeps today's call order and today's spawn-visibility semantics): `SeedMonsters(spawns []dungeonspec.SpawnInstruction)`—hmm, dependency direction: take a local `SpawnInstruction` mirror or accept `(roomID string, ref string, count int)` tuples; pick whichever keeps encounter free of a dungeonspec import and document it—resolves each ref via `monsters.ByRef` (error on miss — validation should make this unreachable), places N instances per room on safe cells (reuse the candidate-pool machinery `placeRegionObstacles` uses — same exclusions plus occupied-cell; extract a shared helper only if it stays behavior-identical, with its own test), adds all monsters with combat-entry evaluation suppressed, then runs ONE `checkCombatEntry` pass. Boss-first order preserved from the plan.

- [ ] **Step 4: Run → PASS; the full encounter suite MUST stay green** — especially the crypt connectivity, boss-axis, and obstacle suites.
- [ ] **Step 5: Commit** `feat(encounter): SeedMonsters — batch seeding under the atomic combat-entry invariant (#<issue>)`

---

## Slice D — rpg-toolkit: dungeon workbench CLI (observability)

### Task D1: `key+seed in → verdict + compiled dump + ASCII floor plan out`

**Files:**
- Create: `encounter/cmd/dungeonspec-workbench/main.go` (thin), `encounter/dungeonspec/workbench.go` (testable core), `encounter/dungeonspec/workbench_test.go`

- [ ] **Step 1: Failing tests on the core (not main)**

```go
func TestWorkbenchReport_ValidSpec(t *testing.T) {
	report, err := dungeonspec.WorkbenchReport(cryptYAML, 42)
	require.NoError(t, err)
	assert.Contains(t, report, "VALID")
	assert.Contains(t, report, "seed 42")
	assert.Contains(t, report, "boss")     // spawn plan section
	assert.Contains(t, report, "#")        // ASCII map: walls
}

func TestWorkbenchReport_InvalidSpecShowsVerdict(t *testing.T) {
	report, err := dungeonspec.WorkbenchReport([]byte("version: 1\n"), 1)
	require.Error(t, err)
	assert.Contains(t, report, "INVALID")
}
```

- [ ] **Step 2: Run → FAIL; Step 3: Implement** — `WorkbenchReport(raw []byte, seed int64) (string, error)`: Load → InitDungeon on a throwaway encounter at the seed → render SpaceData as ASCII (`.` floor, `#` degenerate wall cell, `|`/`-` perimeter edge markers on cell borders if cheap—else fold into `#` and say so, `D` door, `o` obstacle, region ids as a legend). `main.go` = flag parsing (`-file`, `-seed`, `-n` for a multi-seed sweep) + print. Usage doc comment: this is the fast authoring loop — edit YAML → see layouts in seconds, no server (rpg-project#117 observability addendum).
- [ ] **Step 4: Run → PASS; Step 5: Commit** `feat(dungeonspec): workbench — try a spec at a seed without a server (#<issue>)`

Toolkit PRs: ship as 2 PRs — **A+B+D first** ("spec + registry + workbench": D depends only on B's `Load`, and shipping it early gives Kirk the fast authoring loop immediately), then **C alone** ("SeedMonsters": the riskiest piece — the atomic-seeding invariant — lands in its own reviewable unit). Each independently green, each gated. After the final toolkit merge the auto-tagger cuts the release consumed in Slice E.

---

## Slice E — rpg-api: content hosting + wiring + migration

### Task E1: Content dir + embedded validation-at-startup

**Files:**
- Create: `content/dungeons/crypt.yaml` (the parity file — byte-identical intent to testdata/crypt.yaml), `content/dungeons/sunken-crypt.yaml` (the 4-room acceptance file from design.md), `internal/content/content.go` (go:embed + `RPG_CONTENT_DIR` override + `SpecByKey`), `internal/content/content_test.go`
- Modify: `go.mod` (toolkit encounter bump to the release containing A–D)

- [ ] **Step 1: Failing tests**

```go
func TestEveryEmbeddedSpecLoads(t *testing.T) {
	for key, raw := range content.AllSpecs() {
		_, err := dungeonspec.Load(raw) // door ids are compiler-generated; no args
		require.NoError(t, err, "embedded spec %q must load — a broken commit fails CI, not prod", key)
	}
}

func TestSpecByKey_UnknownKey(t *testing.T) {
	_, ok := content.SpecByKey("atlantis")
	assert.False(t, ok)
}

func TestContentDirOverride(t *testing.T) {
	// RPG_CONTENT_DIR pointing at a temp dir with one yaml → that key
	// resolves; embedded keys still resolve (override AUGMENTS, embedded
	// remains fallback — or document replace semantics; pick AUGMENT).
}
```

- [ ] **Steps 2–4: red → implement → green.** Key detail: `SpecByKey` indexes by each file's `key:` field (decode header cheaply), not filename.
- [ ] **Step 5: Commit** `feat(content)#<issue>: embedded dungeon specs + dev dir override, validated at startup`

### Task E2: StartEncounter wiring + deleting the scatter

**Files:**
- Modify: `internal/orchestrators/lobby/dungeon_spec.go` (`resolveDungeonSpec`: switch → content lookup + `dungeonspec.Load`), `internal/orchestrators/lobby/start_encounter.go` (wire `SeedMonsters(compiled.Spawns)`; ALSO delete `regionEntryAnchor` + `anchorProbeEntity` here — their only callers live in the file below, so leaving them is orphaned dead code)
- Delete: `internal/orchestrators/lobby/crypt_monster_seed.go`, `internal/orchestrators/lobby/crypt_monster_seed_internal_test.go` (white-box tests calling regionMonsterAnchor/buildMonsterSeedGroups/seedRegionMonsters — compile-breaks the package if left; its coverage is superseded by Slice C's SeedMonsters invariant tests), AND `internal/orchestrators/lobby/region_entry_anchor_internal_test.go` (the dedicated test that would otherwise keep the orphaned anchor helpers alive)
- Test: extend `internal/orchestrators/lobby/start_encounter_test.go`; the two REAL integration gates by name: `internal/integration/dungeon_crypt_test.go` (byte-for-byte wire-behavior gate — must stay green untouched) and `internal/integration/lobby_crypt_monster_seed_test.go` (real-Redis composition/determinism suite — its assertions check archetype/ref counts + same-seed determinism, not positions, so it should stay green; UPDATE its package doc, which narrates the retired call-order fragility, to describe the new SeedMonsters invariant instead — keep the tests, fix the story)

- [ ] **Step 1: Failing integration test** — `StartEncounter("sunken-crypt")` → snapshot has 4 zones with the declared archetypes, per-zone monster counts match the YAML, boss connector locked DC 12; and the EXISTING crypt gates above still pass on wire expectations (parity is the migration proof).
- [ ] **Step 2: Run → FAIL (non-short, Redis testcontainer); Step 3: Implement** — api calls exactly:

```go
raw, ok := content.SpecByKey(input.DungeonKey)   // NotFound if !ok
compiled, err := dungeonspec.Load(raw)           // InvalidArgument on err (disabled-key path)
compiled.Params.RandomSeed = seed                 // seed is a FIELD, not a call arg —
err = enc.InitDungeon(compiled.Params)            // InitDungeon takes ONE argument
err = enc.SeedMonsters(compiled.Spawns)
```

  Delete `crypt_monster_seed.go` + the anchor helpers + their internal test; `CryptDungeonParams` stays in the toolkit as the parity-test fixture (soak per design; do NOT delete this release).
- [ ] **Step 4: Full non-short suite green; Step 5: Commit** `feat(lobby)#<issue>: StartEncounter builds dungeons from content specs — crypt_monster_seed.go deleted`

### Task E3: Error surface

- [ ] Startup logs one line per invalid file (file, field, reason) and disables the key; requesting a disabled key → `InvalidArgument` carrying the validation message; unknown key → `NotFound` (existing). Table-test the handler mapping. Commit `feat(lobby)#<issue>: content validation error surface`.

---

## Acceptance (the bar from the design, run by a human or the parity stack)

1. Edit `sunken-crypt.yaml` (change a width, add a monster), restart the api (`RPG_CONTENT_DIR` for the fast loop), start a fresh encounter — the change is live. **No Go was touched.**
2. `go run ./cmd/dungeonspec-workbench -file content/dungeons/sunken-crypt.yaml -seed 7` prints VALID, the spawn plan (boss first), and a legible ASCII floor plan.
3. `make local-prod` (rpg-deployment#57) against the new images: the crypt plays exactly as before the migration (parity), and sunken-crypt is selectable by key.

## Explicitly deferred (do not build)

Difficulty engine (SRD XP budgets/pools), `interactions` (traps/clues), loot, `rooms[].layout`, `locked.tool`, temperaments, quest-line composition, per-room heights — all reserved seats per design.md.
