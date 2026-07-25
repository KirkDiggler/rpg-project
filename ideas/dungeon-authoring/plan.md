# Dungeon Authoring (YAML dungeon definitions v1 + static placement delta) — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. This team's additional law: every PR goes through Copilot ack + independent gate review before Kirk merges; posts open with `— implementer (asset-pipeline), on behalf of KirkDiggler`.

**Goal:** Authoring a dungeon becomes editing one YAML file — schema/validation/compiler in the toolkit, content files in rpg-api, with a workbench CLI to try seeds without a server. The static-placement delta (design.md §Design delta, approved 2026-07-24) adds a per-item `place` block so an author can pin exact props/monsters instead of only rolling counts — the explicit⟷random dial is per-item, not global.

**Architecture:** A new toolkit sub-package `encounter/dungeonspec` strictly decodes + validates a versioned YAML spec — rooms' count-based `obstacles`/`monsters` AND the delta's per-item `place` block — and compiles it to the existing `DungeonParams` (extended with placed obstacles) plus an ordered spawn plan (extended with optional placed positions); a new `monsters.ByRef` registry makes monster refs resolvable at author time; `encounter/dungeon.go`'s placement path is extended to place `place` entries verbatim and exclude those cells from the rolled candidate pool; a new `Encounter.SeedMonsters` executes the spawn plan under the atomic-seeding invariant, starting (M1) with placed (position-bearing) instructions and extended (M2) to count-based multi-monster rolling. rpg-api hosts content files (go:embed + dev dir override), resolves keys to bytes, and calls the toolkit's placement/spawn entry points — the legacy crypt path is untouched until M2's migration. No proto or web changes.

**Tech Stack:** Go (toolkit modules `encounter`, `rulebooks/dnd5e`; rpg-api), `gopkg.in/yaml.v3` (strict, KnownFields), go:embed, testify.

**Spec:** `rpg-project/ideas/dungeon-authoring/design.md` — the original v1 design (approved via rpg-project PR #117, merged 2026-07-24) plus its `§Design delta — static placement` section (approved in session 2026-07-24, same file, before implementation began). One delta vs the v1 spec, flagged: `ObstacleSpec.PreferBorder` (added by rpg-toolkit#840 after the v1 design froze) is exposed as optional `prefer_border` in the room-obstacle schema — additive, default false, consistent with the design's honest-to-the-engine rule.

**Milestones (this plan is organized by them, matching design.md §Design delta):**
- **M1 — "The Tomb, walkable."** This plan's primary scope: Slice A (registry, unchanged), Slice B (schema/validation/compiler, extended for `place`), a new engine placement path, Slice D (workbench, extended), and a lightened Slice E (content hosting for one new dungeon key; the crypt's legacy hardcoded path stays untouched).
- **M2 — Migration.** Slice C completes `SeedMonsters` for count-based multi-monster rooms; the crypt is ported to YAML behind a parity test; the legacy scatter (`crypt_monster_seed.go`, its anchor helpers, and the M1-added resolution branch) is deleted.
- **M3 — The reference dungeon.** Content/evidence milestone; no new schema/engine/api code anticipated — see the pointer section below.
- **M4 — The tuning loop.** Future design, not built in this wave.

**Ground rules for every task:** TDD (write the failing test first, watch it fail, then implement); `go test -count=1 -race ./...` in the touched module before every commit; never weaken an existing assertion; commit after each green step. rpg-api extra: commit BEFORE running any `make` target (ci-checks.sh runs `git checkout -- .`), and run non-short integration (`go test -count=1 -race ./...` without `-short`) before push.

---

# Milestone M1 — "The Tomb, walkable."

Acceptance for this milestone (full detail under Acceptance below): edit `content/dungeons/reference-tomb.yaml`, restart the api, walk the room in the real game route — no Go touched.

## Slice A — rpg-toolkit: `monsters.ByRef` registry (module `rulebooks/dnd5e`)

Unchanged by the delta — the smallest unblocker, nothing else can validate monster refs without it.

### Task A1: Registry over existing constructors

**Files:**
- Create: `rulebooks/dnd5e/monster/monsters/registry.go` (NOTE the real path: constructors live in the `monsters` package nested under `monster/` — `rulebooks/dnd5e/monsters/` does NOT exist; verify with `ls rulebooks/dnd5e/monster/monsters/` and by reading rpg-api's `crypt_monster_seed.go` import line before creating anything)
- Test: `rulebooks/dnd5e/monster/monsters/registry_test.go`
- Read first: `rulebooks/dnd5e/monster/monsters/` (constructor naming: `NewSkeleton`, `NewSkeletonCaptain`, `NewGhoul`, `NewZombie`, …) and `rulebooks/dnd5e/refs/monsters.go` (canonical refs — ids are hyphenated, e.g. `skeleton-captain`). CAVEAT (reverified against rpg-toolkit `origin/main` 2026-07-24): `NewGoblin` lives in the PARENT `monster` package (`rulebooks/dnd5e/monster/monster.go:228`, signature `func NewGoblin(id string) *Monster`), not the `monsters` subpackage, and its return type is `*Monster` (unqualified — it's already inside package `monster`), while the subpackage constructors (`NewSkeleton`, `NewGhoul`, `NewZombie`, `NewSkeletonCaptain`) all return `*monster.Monster`. Register `NewGoblin` via a closure at the map literal (`func(id string) *monster.Monster { return monster.NewGoblin(id) }`), never by moving it.

**Constructor inventory (verified directly against rpg-toolkit `origin/main` today — every file in `rulebooks/dnd5e/monster/monsters/`, `rulebooks/dnd5e/monster/monster.go`, and `rulebooks/dnd5e/refs/monsters.go`, read individually, not inferred from filenames):** 11 of the 15 canonical refs have a constructor. Nine are direct: `skeleton` (`NewSkeleton`), `zombie` (`NewZombie`), `skeleton-captain` (`NewSkeletonCaptain`), `ghoul` (`NewGhoul`), `giant-rat` (`NewGiantRat`), `wolf` (`NewWolf`), `brown-bear` (`NewBrownBear`), `thug` (`NewThug`), `goblin` (`monster.NewGoblin`, parent package, per the caveat above). Two are NON-obvious and easy to get wrong from the function name alone: `bandit.go` defines `NewBanditMelee` and `NewBanditRanged` — NOT `NewBandit`/`NewBanditArcher` as their names might suggest — but reading each constructor's body shows `NewBanditMelee`'s `monster.Config.Ref` is `refs.Monsters.Bandit()` and `NewBanditRanged`'s is `refs.Monsters.BanditArcher()`. So `bandit` resolves via `NewBanditMelee` and `bandit-archer` resolves via `NewBanditRanged` — both real, both easy to miss by pattern-matching names instead of reading the `Ref:` field each constructor actually sets. Four refs have NO constructor anywhere: `skeleton-archer`, `giant-spider`, `giant-wolf-spider`, `bandit-captain` — exclude these from the enumerated test list below, note them in the PR body, do not fabricate constructors for them. Because `bandit.go`'s function names don't match their refs, Step 3's map-literal population must be driven by each constructor's actual `Config.Ref` value (read the source), never guessed from the ref string.

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
	// The 11 refs verified to have a real constructor today (see the
	// inventory above) — bandit/bandit-archer resolve via NewBanditMelee/
	// NewBanditRanged despite the name mismatch; the other 4 canonical
	// refs (skeleton-archer, giant-spider, giant-wolf-spider,
	// bandit-captain) have no constructor and are deliberately absent.
	for _, ref := range []string{
		refs.Monsters.Skeleton().String(),
		refs.Monsters.Zombie().String(),
		refs.Monsters.SkeletonCaptain().String(),
		refs.Monsters.Ghoul().String(),
		refs.Monsters.GiantRat().String(),
		refs.Monsters.Wolf().String(),
		refs.Monsters.BrownBear().String(),
		refs.Monsters.Bandit().String(),
		refs.Monsters.BanditArcher().String(),
		refs.Monsters.Thug().String(),
		refs.Monsters.Goblin().String(),
	} {
		ctor, ok := monsters.ByRef(ref)
		require.True(t, ok, "ref %q must resolve", ref)
		require.NotNil(t, ctor, "ref %q must have a constructor", ref)
	}
}

func TestByRef_UnconstructedCanonicalRefReturnsFalse(t *testing.T) {
	// bandit-captain IS a canonical ref (refs.Monsters.BanditCaptain()) but
	// has no constructor today — distinct from an entirely fictional ref,
	// and worth its own case so a future constructor addition here doesn't
	// silently get missed by only testing a made-up name.
	ctor, ok := monsters.ByRef(refs.Monsters.BanditCaptain().String())
	assert.False(t, ok)
	assert.Nil(t, ctor)
}

func TestByRef_UnknownRefReturnsFalse(t *testing.T) {
	ctor, ok := monsters.ByRef("dnd5e:monsters:beholder")
	assert.False(t, ok)
	assert.Nil(t, ctor)
}
```

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

## Slice B — rpg-toolkit: `encounter/dungeonspec` schema + strict decode + validation (extended for `place`)

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
	Place     []PlacedEntry   `yaml:"place,omitempty"` // delta addition — see below
}

type MonsterEntry struct {
	Ref   string `yaml:"ref"`
	Count int    `yaml:"count"`
}

type BossEntry struct {
	Ref string  `yaml:"ref"`
	At  *[2]int `yaml:"at,omitempty"` // delta addition: nil = unpinned (v1 behavior)
}

type ObstacleEntry struct {
	Ref            string `yaml:"ref"`
	Count          int    `yaml:"count"`
	BlocksMovement *bool  `yaml:"blocks_movement,omitempty"` // nil => true
	BlocksLoS      *bool  `yaml:"blocks_los,omitempty"`      // nil => true
	PreferBorder   bool   `yaml:"prefer_border,omitempty"`   // post-design delta: toolkit#840
}

// PlacedEntry is one static placement (design.md §Design delta) — routed by
// Ref's `module:type:id` type segment at VALIDATE time (props → obstacle,
// monsters → spawn; see Task B2), not at decode time, so a bad ref type
// fails validation with a clear message instead of a decode error.
type PlacedEntry struct {
	Ref            string `yaml:"ref"`
	At             [2]int `yaml:"at"` // [col, row], room-local — see design.md §Design delta
	BlocksMovement *bool  `yaml:"blocks_movement,omitempty"`
	BlocksLoS      *bool  `yaml:"blocks_los,omitempty"`
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

- [ ] **Step 6 (delta addition): failing test — `place` round-trips, ref-type-agnostic at decode time**

```go
func TestDecode_PlaceBlockRoundTrips(t *testing.T) {
	spec, err := dungeonspec.Decode([]byte(placedTombYAML)) // design.md's tomb example, copied verbatim
	require.NoError(t, err)
	tomb := spec.Rooms[len(spec.Rooms)-1]
	require.Len(t, tomb.Place, 6) // coffin, altar, statue-reaper, brazier x2, skeleton
	assert.Equal(t, "dnd5e:props:coffin", tomb.Place[0].Ref)
	assert.Equal(t, [2]int{6, 3}, tomb.Place[0].At)
	require.NotNil(t, tomb.Place[0].BlocksLoS)
	assert.False(t, *tomb.Place[0].BlocksLoS)
	assert.Equal(t, "dnd5e:monsters:skeleton", tomb.Place[5].Ref)
	require.NotNil(t, tomb.Boss.At)
	assert.Equal(t, [2]int{7, 5}, *tomb.Boss.At)
}
```

- [ ] **Step 7: Run → FAIL** (`Place`/`Boss.At` undefined) → already implemented above in Step 3's struct listing, so this is a red→green pair against that same implementation, not a second implementation pass.
- [ ] **Step 8: Commit** `feat(dungeonspec): place block + boss.at structs and decode (#<issue>)`

**Fixture note (advisory, verified by hand-checking every row used):** `placedTombYAML` is not the bare `- id: tomb ...` fragment shown in design.md's "Schema: the `place` block" section in isolation — it's that fragment wrapped in a complete, valid two-room file: `version: 1`, a `key: reference-tomb` (the same key the M1 acceptance file in Slice E ships under — this fixture IS `content/dungeons/reference-tomb.yaml`'s content, not a separate lookalike), `height: 8` SPECIFICALLY, a minimal `entrance` room, one connector `{from: entrance, to: tomb}`, then the tomb room verbatim. `height: 8` is load-bearing for the fixture, not an arbitrary choice: the tomb room's placements use rows 1, 2, 3, 5, and 6 (boss at row 5, coffin/altar at row 3, statue/skeleton/braziers at rows 1/1/2/6) — `height: 8` puts `doorRow` at row 4 (clear of all of them); `height: 10` would move `doorRow` to row 5 and collide with the boss's pinned position, breaking the fixture the same way the original (now-fixed) row-4 collision did.

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
		{"broken chain", func(s *dungeonspec.DungeonSpec) { s.Connectors[1].To = "tomb" }, "linear chain"},
		// ...one row per remaining rule, both directions where meaningful
	}
	// decode validM1YAML fresh per case, apply mutate, assert Validate error
}
```

**Base fixture, corrected (consequence of the M1-only monster-pinning restriction below — this replaces the original "decode `referenceYAML`" plan, which does NOT satisfy that restriction):** the table's base is `validM1YAML`, NOT `referenceYAML` directly — same 4-room/3-connector shape (`entrance`→`gallery`→`trap-crossing`→`tomb`, same ids/widths/archetypes/connectors/lock DC 12, `height: 8`) so every existing row above keeps working unmodified (`Rooms[3]` is still the tomb, `Connectors[1]` still exists), with two changes to make it M1-valid: `entrance`/`gallery` drop their count-based `monsters:` blocks (their count-based `obstacles:` stay — unaffected by the restriction); the `tomb` room's `boss:` gains `at: [7, 5]` and its `obstacles:` list is replaced by the delta's full `place:` list (coffin/altar/statue-reaper/brazier×2/skeleton, occupying rows 1, 2, 3, 5, and 6 — clear of `doorRow` (row 4) per the earlier fix) — so `validM1YAML` is ALSO the base every `place`/`boss.at` row below uses via the `tomb(s)` helper, one fixture, not two. The original "unresolvable monster ref" row (`s.Rooms[0].Monsters[0].Ref = ...`) is REMOVED here, not merely edited: with `monsters:` forbidden outright in M1, there is no resolvable-vs-unresolvable distinction left to test on a count-based entry — that coverage already exists below as "place monster ref unresolvable" (a `place` monster entry, which still must resolve), and full count-based ref-resolution coverage returns in M2's Task C0/C2 once `testdata/crypt.yaml` is loadable.

**Implementation-order note (a real interaction this fixture swap introduces):** because `validM1YAML`'s tomb room now carries `at`-bearing `place` entries (column 9 requires `width ≥ 10`), the pre-existing "boss width 5 fails axis rule" row shrinks the room enough to ALSO put those entries out of bounds — if `Validate`'s per-`place`-cell bounds check runs before its boss-primary-axis check, this row would fail on "out of bounds" instead of the intended "primary axis" substring. Order `Validate`'s checks so the boss-axis rule (a room-level/structural check) runs before per-cell `place` bounds checks — a more useful error for an author shrinking a boss room anyway ("your boss room is too small," not "and also one of your ten placements happens to fall off the edge because of that").

- [ ] **Step 2: Run → FAIL** (`Validate` undefined)
- [ ] **Step 3: Implement `Validate(spec) error`** — plain sequential checks, error messages naming field + file-fixable detail. The chain check: connectors[i] must join rooms[i] and rooms[i+1] by id (the v1 generator constraint — same order the design's example uses).
- [ ] **Step 4: Run → PASS; Step 5: Commit** `feat(dungeonspec): validation — every generator constraint mirrored at load (#<issue>)`

**Delta addition — validation rows for `place`/`boss.at`.** Use a fixture with a `place`-bearing room (design.md's tomb) and a small helper (`tomb(s) *dungeonspec.RoomSpec` returning `&s.Rooms[len(s.Rooms)-1]`) so mutate funcs don't repeat magic indices as the fixture grows:

```go
{"place at out of bounds (col)", func(s *dungeonspec.DungeonSpec) {
	// row 3 deliberately, NOT the entry's real row 4 — height/2 (doorRow)
	// is ALSO row 4 (height:8), so col-99 at row 4 would double-break
	// (col OOB + reserved row) and this row wouldn't isolate which check
	// actually fired.
	tomb(s).Place[0].At = [2]int{99, 3}
}, "out of bounds"},
{"place at out of bounds (row)", func(s *dungeonspec.DungeonSpec) {
	tomb(s).Place[0].At = [2]int{6, 99}
}, "out of bounds"},
{"place collides with another placed entry", func(s *dungeonspec.DungeonSpec) {
	tomb(s).Place[1].At = tomb(s).Place[0].At
}, "already placed"},
{"place collides with boss.at", func(s *dungeonspec.DungeonSpec) {
	tomb(s).Place[0].At = *tomb(s).Boss.At
}, "already placed"},
{"place on reserved row (height/2) rejected", func(s *dungeonspec.DungeonSpec) {
	tomb(s).Place[0].At = [2]int{6, s.Height / 2}
}, "reserved row"},
{"place ref of unknown type rejected", func(s *dungeonspec.DungeonSpec) {
	// Place[2] (statue-reaper) deliberately, NOT Place[0] (coffin, which
	// sets blocks_los): this row must isolate the ref-type rule alone,
	// not also trip "blocks_los only valid on props" on the same mutated
	// entry (see the check-ordering note below).
	tomb(s).Place[2].Ref = "dnd5e:traps:pit"
}, "must be props or monsters"},
{"blocks_los set on a monster place entry is rejected", func(s *dungeonspec.DungeonSpec) {
	f := false
	tomb(s).Place[5].BlocksLoS = &f // Place[5] is the skeleton (monster) entry
}, "blocks_los only valid on props"},
{"boss ref duplicated in place is rejected", func(s *dungeonspec.DungeonSpec) {
	tomb(s).Place = append(tomb(s).Place, dungeonspec.PlacedEntry{Ref: tomb(s).Boss.Ref, At: [2]int{0, 0}})
}, "boss ref may not also appear in place"},
{"place monster ref unresolvable", func(s *dungeonspec.DungeonSpec) {
	tomb(s).Place[5].Ref = "dnd5e:monsters:beholder"
}, "unknown monster"},
{"place rejected when room pattern is scattered", func(s *dungeonspec.DungeonSpec) {
	// tomb has both place entries AND a pinned boss.at — either alone
	// triggers this rule; Validate's OR condition is exercised either way.
	tomb(s).Pattern = "scattered"
}, "place/boss.at not allowed with pattern: scattered"},
{"count-based monsters: entry rejected in M1", func(s *dungeonspec.DungeonSpec) {
	s.Rooms[0].Monsters = append(s.Rooms[0].Monsters,
		dungeonspec.MonsterEntry{Ref: "dnd5e:monsters:skeleton", Count: 2})
}, "rolled monster placement lands in M2"},
{"unpinned boss (no at) rejected in M1", func(s *dungeonspec.DungeonSpec) {
	tomb(s).Boss.At = nil
}, "rolled monster placement lands in M2"},
```

**Reserved-row rule, pinned against real engine code (verified on rpg-toolkit `origin/main`, `encounter/dungeon.go`, 2026-07-24):** `row == height/2` is rejected for EVERY room regardless of archetype. `placeRegionObstacles`/`regionObstacleCandidates` already exclude that entire row from the rolled-obstacle candidate pool in every region, uniformly — a boss/interior region's actual required path spans the row's full width, an entrance/terminal region's spans only half, but the exclusion doesn't discriminate; it's the same over-conservative reservation everywhere. Rejecting `place` on that row therefore gives placed entries the IDENTICAL traversability guarantee rolled obstacles already have, by construction — this ONE check is both the "no door cell" rule and the "room stays traversable" rule from design.md's validation list; no separate path-oracle re-implementation is needed at spec-load time.

**Check-ordering note (a real interaction between two rules on the same entry):** run the ref-type check (`must be props or monsters`) BEFORE the flags-only-on-props check in `Validate`'s sequence. The "place ref of unknown type rejected" row above deliberately targets `Place[2]` (statue-reaper, no `blocks_movement`/`blocks_los` override) rather than `Place[0]` (coffin, which DOES set `blocks_los`) specifically so this row exercises exactly one rule regardless of implementation order — but ordering ref-type first is still the right call generally: an entry with an unrecognized ref type should always report THAT error, not whichever of the two checks happens to run first when both could theoretically fire.

**Scattered-pattern rule (Issue: `place` × `pattern: scattered` is a seed-dependent runtime failure otherwise).** `pattern: scattered` compiles to `environments.PatternRandom`, whose interior walls are seed-rolled — no `at` cell (or pinned `boss.at`) can be guaranteed clear or non-wall at author time for that room, so the load-time "a file that loads is a file that plays" contract can't hold. `Validate` rejects `place` (non-empty) or a pinned `boss.at` in any room whose `pattern` is `scattered`, full stop — a load-time error, not a runtime surprise on an unlucky seed. Revisitable, not permanent: a future design could let `place` coexist with `scattered` (e.g. re-rolling walls around the fixed cells), but that's new design work, out of scope for this delta.

**M1-only monster-pinning restriction (Issue: `SeedMonsters` in M1 only handles `At`-bearing spawns — see Task N2 below).** Until M2's Slice C lands count-based safe-cell rolling, a spec that used `monsters:` (count-based) or an unpinned `boss` (no `at`) would compile successfully today but fail at `SeedMonsters` runtime — exactly the "compiles but the engine can't actually do it" gap the file-load contract exists to prevent. `Validate` therefore rejects, in M1: any room with a non-empty `monsters:` list, and any `boss` room whose `boss.at` is nil. Rolled *obstacles* (`obstacles:`, count-based) are UNAFFECTED by this restriction — that placement machinery (`placeRegionObstacles`) already exists in the engine and needs no M2 work. This is temporary and load-bearing to remove correctly: M2's Slice C's Task C0 deletes both checks from `Validate` as one of its first steps, with a test proving a previously-rejected spec now loads and seeds correctly.

**Fixture consequence (a real, unavoidable interaction, not an oversight):** this restriction means `referenceYAML` (design.md's original 4-room sunken-crypt example, which uses count-based `monsters:` in its entrance and gallery rooms, decoded verbatim) and `testdata/crypt.yaml` (which mirrors `crypt_monster_seed.go`'s real one-monster-per-region table via `monsters:`, not `place` — see Task B3's note on why the crypt genuinely can't be represented as pinned) both fail `Validate` in M1. That's correct, not a bug: both are M2-era specs. Concretely:
- The table's base fixture is `validM1YAML` (see above), not `referenceYAML` directly — same shape, M1-valid content.
- Add one more table row using `referenceYAML` itself as a NEGATIVE case: `{"referenceYAML's own count-based monsters are M1-invalid", func(s *dungeonspec.DungeonSpec) {}, "rolled monster placement lands in M2"}`, decoding `referenceYAML` fresh for this ONE row instead of `validM1YAML` — this turns what would otherwise be an inconsistency into a positive proof that the restriction fires exactly where expected. **This row is itself temporary and must be REMOVED (not flipped to `""`) as part of M2's Task C0** — once C0 lifts the restriction, `referenceYAML` validates cleanly, and this row's own `wantErr` assertion would be backwards; Task C0's own Step 1 (`TestValidate_UnpinnedMonstersAllowedOnceSeedMonstersRolls`) already asserts the positive direction for exactly this fixture, so keeping both would duplicate coverage rather than cleanly replace it. Task C0's removal list (below) covers all three: the two `Validate` checks AND this table row.
- `TestDecode_RoundTripsTheReferenceSpec` (Task B1, Decode-only, never calls `Validate`) is unaffected either way.

- [ ] **Step 6: Run → FAIL; Step 7: Implement the above checks in `Validate`; Step 8: Run → PASS; Step 9: Commit** `feat(dungeonspec): validation — place block cell/type/collision/pattern/M1-monster-pinning rules (#<issue>)`

### Task B3: Compiler → `CompiledDungeon` (M1: `place`-routing + door ids; crypt parity moves to M2)

**Files:**
- Create: `encounter/dungeonspec/compile.go`
- Test: `encounter/dungeonspec/compile_test.go`

**Scope note (consequence of the M1-only monster-pinning restriction above):** `testdata/crypt.yaml` and its parity tests are NOT part of this M1 task — see the fixture consequence under Task B2. The crypt's real monster placement (`crypt_monster_seed.go`'s `regionMonsterAnchor`) computes its anchor cell at RUNTIME from the generated room's actual door-adjacent walkable geometry — it is seed/geometry-dependent, not a fixed `at: [col,row]` known at authoring time, so representing it faithfully in `dungeonspec` needs count-based `monsters:` semantics (M2), not `place`. `TestLoad_CryptParity`, `TestLoad_SpawnsAreBossFirst`, and `TestLoad_Deterministic` (all `cryptYAML`-based) move to M2's Slice C as Task C2, run once that slice lifts the M1 restriction. This task proves the compiler's structural logic (door ids, pattern mapping, `place` routing) entirely on the M1-valid `placedTombYAML` fixture (`referenceYAML` itself is decode-only in M1 — see Task B2's fixture consequence for why).

- [ ] **Step 1: Failing tests**

**Door-ID rule, corrected (Issue: the original `"door-<from>-<to>"` sketch collides with the untouched crypt gate — verified against real rpg-api code, not just the design's prose):** the compiler generates `"<key>-door-<from>-<to>"` per connector — e.g. `key: reference-tomb`, connector `entrance→tomb` → `"reference-tomb-door-entrance-tomb"`. This is not an arbitrary choice: `internal/integration/dungeon_crypt_test.go:62-63` and `internal/handlers/dnd5e/v2/encounter/project_test.go:1125-1126` on rpg-api `origin/main` both hardcode `cryptEntranceDoorID = "crypt-door-entrance-corridor"` and `cryptBossDoorID = "crypt-door-corridor-boss"` as their own test constants — exactly `"<key>-door-<from>-<to>"` with `key: crypt`. Once the crypt migrates to content in M2 under `key: crypt`, the compiler reproduces these two ids byte-for-byte with zero special-casing, which is what keeps E4's "the existing crypt gates stay green untouched" claim actually true. No caller-supplied ids, no variadic args, works for any N connectors, stable across runs of the same spec. `Load`'s full signature is therefore `Load(raw []byte) (CompiledDungeon, error)` — nothing else.

```go
func TestLoad_GeneratesDeterministicDoorIDs(t *testing.T) {
	// placedTombYAML: key "reference-tomb", 2 rooms (entrance, tomb), 1 connector —
	// the M1-valid fixture (referenceYAML's own count-based monsters fail Validate
	// in M1; see Task B2's fixture consequence). N-connector coverage (3+ doors in
	// one file) resumes in M2 once referenceYAML/cryptYAML are loadable again.
	compiled, err := dungeonspec.Load(placedTombYAML)
	require.NoError(t, err)
	require.Len(t, compiled.Params.Connectors, 1)
	assert.Equal(t, core.EntityID("reference-tomb-door-entrance-tomb"),
		compiled.Params.Connectors[0].DoorID)
}

// PATTERN-DEFAULT TRAP (advisory made explicit): the engine treats an
// empty DungeonRegionParams.Pattern as PatternRandom. The SPEC's default
// is empty (design.md). The compiler must therefore map "" and "empty" →
// environments.PatternEmpty and "scattered" → environments.PatternRandom
// EXPLICITLY — never pass the zero value through. placedTombYAML's own
// rooms are both empty-pattern (no room in it is scattered — Task B2's
// scattered-rejection rows exercise scattered ONLY in combination with
// place/boss.at, never the plain mapping), so the scattered→PatternRandom
// direction needs its OWN small fixture, inline here, decoupled from
// placedTombYAML/validM1YAML entirely:

func TestLoad_ScatteredPatternMapsToPatternRandom(t *testing.T) {
	// Throwaway 2-room fixture, no place/obstacles/monsters at all — this
	// test is purely about the pattern string→engine-constant mapping,
	// nothing else. Keep it minimal on purpose.
	const scatteredYAML = `
version: 1
key: pattern-mapping-check
name: Pattern Mapping Check
height: 8
rooms:
  - {id: entrance, archetype: entrance, width: 6}
  - {id: room-two, archetype: chamber, width: 6, pattern: scattered}
connectors:
  - {from: entrance, to: room-two}
`
	compiled, err := dungeonspec.Load([]byte(scatteredYAML))
	require.NoError(t, err) // scattered is legal on its own — design.md's clarification
	assert.Equal(t, environments.PatternRandom, compiled.Params.Regions[1].Pattern)
	assert.Equal(t, environments.PatternEmpty, compiled.Params.Regions[0].Pattern) // default direction, same test
}
```

Do NOT make `reference-tomb.yaml`/`placedTombYAML` scattered to get this coverage for free — `PatternEmpty` is the deliberate M3-bound choice (rpg-toolkit#835 moved the crypt off `PatternRandom` for exactly the "intact walls, decay carried by dressing" look this delta's reference dungeon is chasing); a throwaway inline fixture keeps that art-direction choice and this plumbing test independent of each other.

- [ ] **Step 2: Run → FAIL; Step 3: Implement**

```go
// SpawnInstruction is encounter's OWN type (defined in Task N2's
// encounter/seed_monsters.go, alongside SeedMonsters) — dungeonspec
// re-exports it via a plain type ALIAS, not a second struct, so
// CompiledDungeon.Spawns can be passed directly to
// enc.SeedMonsters(compiled.Spawns) with zero conversion (Issue: a
// dungeonspec-local mirror struct would NOT satisfy SeedMonsters'
// []encounter.SpawnInstruction parameter — Go has no implicit struct
// conversion for function arguments, this would simply fail to compile).
// Dependency direction, unchanged from the rest of this package:
// dungeonspec already imports encounter; encounter never imports
// dungeonspec (that's the cycle constraint from Step 3's note below) —
// so the type must be encounter-owned for both sides to share it, and
// dungeonspec's alias is purely a naming convenience for this package's
// own callers.
type SpawnInstruction = encounter.SpawnInstruction

type CompiledDungeon struct {
	Params encounter.DungeonParams
	Spawns []SpawnInstruction // boss first, then entrance→boss chain order
}

// Load decodes, validates, and compiles a spec in one call — the only
// entry point callers use. Door entity ids are COMPILER-GENERATED from
// content ("<key>-door-<from>-<to>" per connector, see the Door-ID rule
// above) — callers supply nothing but the bytes.
func Load(raw []byte) (CompiledDungeon, error)
```

Boss-first ordering is defense-in-depth (design.md §Compiler); the real safety is Slice C's invariant. Note: `dungeonspec` imports `encounter` (parent package) — if Go disallows the cycle you need (encounter importing dungeonspec later), keep dungeonspec import-free of encounter internals except exported types; check the dependency direction BEFORE writing code: dungeonspec → encounter is the only direction used (api calls both).

- [ ] **Step 4: Run → PASS; Step 5: Full module suite + commit** `feat(dungeonspec): compiler — CompiledDungeon, key-scoped door ids (#<issue>)`

**Delta addition — compiling `place` into `PlacedObstacles` + `At`-bearing `SpawnInstruction`s.**

```go
func TestLoad_PlaceRoutesByRefType(t *testing.T) {
	compiled, err := dungeonspec.Load(placedTombYAML)
	require.NoError(t, err)
	tombRegion := regionByID(compiled.Params.Regions, "tomb")
	require.Len(t, tombRegion.PlacedObstacles, 5) // coffin, altar, statue-reaper, brazier x2
	assert.Equal(t, "dnd5e:props:coffin", tombRegion.PlacedObstacles[0].Ref)
	assert.Equal(t, encounter.LocalHex{Col: 6, Row: 3}, tombRegion.PlacedObstacles[0].At)
	assert.False(t, tombRegion.PlacedObstacles[0].BlocksLoS)

	// BLOCKING-DEFAULT TRAP (advisory made explicit, same class as the
	// PATTERN-DEFAULT TRAP above): PlacedEntry.BlocksMovement/BlocksLoS are
	// *bool with nil => true (design.md: "same defaults/semantics as
	// ObstacleEntry"), but PlacedObstacleSpec.BlocksMovement/BlocksLoS
	// (engine-side, Task N1) are plain bool, whose Go zero value is FALSE.
	// A compiler that just dereferences a nil *bool panics; one that
	// silently treats nil as the zero value inverts the default (an
	// unflagged placed prop would compile to "blocks nothing," backwards
	// from the design's intent). The altar entry in placedTombYAML sets
	// NEITHER flag, so it's the direct test of this: it must compile to
	// BlocksMovement: true, BlocksLoS: true (matching the existing
	// crypt_dungeon.go precedent: altar is a structural piece, blocks both).
	altar := tombRegion.PlacedObstacles[1]
	assert.Equal(t, "dnd5e:props:altar", altar.Ref)
	assert.True(t, altar.BlocksMovement)
	assert.True(t, altar.BlocksLoS)

	var skeletonSpawn *dungeonspec.SpawnInstruction
	for i := range compiled.Spawns {
		if compiled.Spawns[i].MonsterRef == "dnd5e:monsters:skeleton" {
			skeletonSpawn = &compiled.Spawns[i]
		}
	}
	require.NotNil(t, skeletonSpawn)
	require.NotNil(t, skeletonSpawn.At)
	assert.Equal(t, encounter.LocalHex{Col: 4, Row: 2}, *skeletonSpawn.At)
	assert.Equal(t, 1, skeletonSpawn.Count)
}

func TestLoad_BossAtCompilesToSpawnPosition(t *testing.T) {
	compiled, _ := dungeonspec.Load(placedTombYAML)
	boss := compiled.Spawns[0] // boss-first ordering unchanged
	require.NotNil(t, boss.At)
	assert.Equal(t, encounter.LocalHex{Col: 7, Row: 5}, *boss.At)
}
```

**Naming/architecture note (this plan's decision, load-bearing for the engine slice below):** `encounter.LocalHex{Col, Row int}` here names the exact type the new engine slice introduces (see Task N1) — the compiler reuses it rather than inventing a parallel `dungeonspec`-local coordinate type. The critical constraint: `At` stays ROOM-LOCAL (pre-`offsetX`) all the way from YAML into `DungeonParams.Regions[i].PlacedObstacles` and `SpawnInstruction.At` — the compiler never computes a region's `offsetX` itself. That arithmetic (`starts[i]` in `generateDungeonLayout`) is layout-time-only and depends on every region's width in chain order; duplicating it in `dungeonspec` would silently drift the moment column math changes there. The engine adds `offsetX` at placement time, exactly as it already does internally for rolled candidates.

The compiler's nil→true mapping for `PlacedEntry.BlocksMovement`/`BlocksLoS` (see the BLOCKING-DEFAULT TRAP above) is the same shape as `ObstacleEntry`'s existing (unwritten-but-implied) nil→true mapping for count-based obstacles — if that mapping isn't ALREADY implemented as part of the original v1 compiler work, implement both here together rather than leaving one inconsistent with the other.

- [ ] **Step 6: Run → FAIL; Step 7: Implement** the ref-type routing (props → append to the region's `PlacedObstacles`, mapping nil `BlocksMovement`/`BlocksLoS` to `true` per the trap above; monsters → append an `At`-bearing `SpawnInstruction`, in `place` list order, boss spawn first as already ordered); **Step 8: Run → PASS; Step 9: Commit** `feat(dungeonspec): compile place block into PlacedObstacles + positioned spawns (#<issue>)`

---

## New M1 slice — rpg-toolkit: engine placement path (`encounter/dungeon.go` + new `encounter/seed_monsters.go`)

Genuinely new engine code the delta requires — none of it exists yet on rpg-toolkit `origin/main` (verified 2026-07-24: no `dungeonspec`, `PlacedObstacles`, `PlacedObstacleSpec`, `LocalHex`, or `SeedMonsters` anywhere in the module). The facts below ARE verified against `origin/main` today and must not drift when this task is implemented:

- `DungeonRegionParams.Obstacles []ObstacleSpec` and `placeRegionObstacles`/`regionObstacleCandidates`/`wallCubeSet`/`drawObstacles`/`drawObstaclesFrom` are real, in `encounter/dungeon.go`, exactly as described below.
- `regionObstacleCandidates` scans LOCAL `(x, y)` with `x` in `[0, width)`, `y` in `[0, height)`, excluding `y == doorRow` (`doorRow := params.Height / 2`) and wall cells, THEN `placeRegionObstacles`/`generateDungeonLayout` translate to absolute cube coordinates via `spatial.OffsetCoordinateToCubeWithOrientation(spatial.Position{X: float64(x + offsetX), Y: float64(y)}, ...)` — this is the exact frame `place`'s `[col, row]` maps onto with zero conversion beyond that same `+offsetX`.
- `encounter/combat.go`'s `checkCombatEntry()` self-transitions FREE_ROAM→TURN_BASED via LoS scan, gated idempotent on mode. `encounter/encounter.go`'s `AddMonster` calls `checkCombatEntry()` on every invocation; `AddPlayer` does NOT (reverified today — this is still true on `origin/main`, matching design.md's already-cited finding).
- Existing test files to extend, not fork: `encounter/obstacle_placement_test.go` (obstacle placement), `encounter/boss_primary_axis_test.go` and `encounter/perimeter_edge_walls_test.go` (must stay green untouched by this task).

### Task N1: `DungeonRegionParams.PlacedObstacles` — verbatim placement, excluded from the rolled pool

**Files:**
- Modify: `encounter/dungeon.go` (`DungeonRegionParams` gains `PlacedObstacles []PlacedObstacleSpec`; extend `placeRegionObstaclesParams`, `placeRegionObstacles`, `regionObstacleCandidates`)
- Test: `encounter/obstacle_placement_test.go` (existing file)
- Read first, in full: `placeRegionObstacles`'s and `regionObstacleCandidates`'s doc comments — they already spell out the candidate-pool/doorRow-exclusion invariants this task must preserve exactly, and the `PreferBorder` two-pool partition (rpg-toolkit#839/#840) this task must not disturb.

- [ ] **Step 1: Failing tests**

```go
func TestInitDungeon_PlacedObstaclesLandVerbatim(t *testing.T) {
	// A region with PlacedObstacles: []PlacedObstacleSpec{{Ref: "dnd5e:props:coffin",
	// At: LocalHex{Col: 6, Row: 3}}} places at EXACTLY local (6,3) translated by that
	// region's offsetX — assert the resulting ObstacleData.Position directly (the
	// same OffsetCoordinateToCubeWithOrientation conversion regionObstacleCandidates
	// uses), not just "an obstacle with this ref exists somewhere in the region."
}

func TestInitDungeon_RolledObstaclesNeverUsePlacedCells(t *testing.T) {
	// A region with both PlacedObstacles and Obstacles (rolled, Count sized so the
	// candidate pool minus placed cells is tight): across a sweep of seeds, no
	// rolled ObstacleData ever lands on a placed cell's cube coordinate.
}

func TestInitDungeon_PlacedObstacleOnReservedRowRejected(t *testing.T) {
	// InitDungeon rejects a PlacedObstacleSpec whose At.Row == height/2 —
	// belt-and-suspenders with dungeonspec's own load-time check (Task B2),
	// since InitDungeon is a public toolkit entry point other callers besides
	// dungeonspec could reach directly.
}

func TestInitDungeon_PlacedObstacleCollisionRejected(t *testing.T) {
	// Two PlacedObstacleSpecs at the same At, or one at a wall cell (PatternRandom
	// region), is a hard InitDungeon error — placed entries are guarantees, not
	// best-effort (design.md §Validation).
}
```

- [ ] **Step 2: Run → FAIL; Step 3: Implement**

```go
// PlacedObstacleSpec pins one obstacle instance to an exact room-local cell
// — verbatim placement, not a candidate-pool roll (design.md §Design delta).
// encounter never interprets Ref, mirroring ObstacleSpec's existing
// content-agnostic contract.
type PlacedObstacleSpec struct {
	Ref            string
	At             LocalHex
	BlocksMovement bool
	BlocksLoS      bool
}

// LocalHex is a region-local (pre-offsetX) grid cell: Col in [0,width),
// Row in [0,height) (the dungeon-shared height) — exactly the local (x,y)
// frame regionObstacleCandidates already scans (see that function's doc).
type LocalHex struct{ Col, Row int }
```

Extend `DungeonRegionParams` with `PlacedObstacles []PlacedObstacleSpec`. Inside `placeRegionObstacles` (or a small helper it calls first): for each `PlacedObstacleSpec`, reject `At.Row == doorRow`, reject a cell already claimed by another placed entry, reject a wall cell (check against the same `wallCubes` set `regionObstacleCandidates` already builds), otherwise place it verbatim (`ObstacleData{Position: core.HexFromCube(spatial.OffsetCoordinateToCubeWithOrientation(...))}`, same conversion as existing rolled placement). Thread the resulting set of placed absolute cube coordinates into `regionObstacleCandidates` (or the border/interior partition in `placeRegionObstacles`, for regions using `PreferBorder`) as an ADDITIONAL exclusion alongside the existing `wallCubes`/`doorRow` exclusions, so rolled obstacles never draw a placed cell.

- [ ] **Step 4: Run → PASS — and the full existing obstacle/boss-axis/perimeter suites (`obstacle_placement_test.go`, `boss_primary_axis_test.go`, `perimeter_edge_walls_test.go`) MUST stay green untouched. This task only ADDS a placed-cell exclusion on top of the existing candidate pool; a region with zero `PlacedObstacles` must produce byte-identical output to today.**
- [ ] **Step 5: Commit** `feat(encounter): PlacedObstacleSpec — verbatim obstacle placement, excluded from the rolled pool (#<issue>)`

### Task N2: placed-monster spawning under the atomic-seeding invariant (M1-scoped `SeedMonsters`)

**Files:**
- Create: `encounter/seed_monsters.go`, `encounter/seed_monsters_test.go`
- Read first (verified above, today): `encounter/encounter.go`'s `AddMonster`/`AddPlayer`, `encounter/combat.go`'s `checkCombatEntry`.

**Scoping call (this plan's decision, resolving an ambiguity the delta text leaves open):** design.md's `SpawnInstruction.At` split ("placed monsters compile to single-count instructions with `At` set; rolled monsters keep `Count` with no position") describes the COMPILER's output shape only — it does not say how much of the ENGINE side lands in M1 vs M2. Given the atomic-seeding invariant applies identically to placed monsters (per the delta), M1 must build `SeedMonsters`' full batching machinery (suppress combat-entry checks across the batch, one `checkCombatEntry()` pass at the end) even though it only NEEDS to place `At`-bearing instructions — a bare loop of individual `AddMonster` calls would reintroduce exactly the partial-roster bug class the invariant exists to prevent (`AddMonster` calls `checkCombatEntry()` on every invocation). M2's Slice C then EXTENDS this same function with the harder, invariant-heavy N-per-room safe-cell-rolling machinery for `At == nil` instructions — the design's own sequencing rationale ("the invariant-heavy engine work lands after the tool proves itself"). M1's own acceptance dungeon (`reference-tomb.yaml`) is fully placed, so it never exercises the rolled path — this scoping costs M1's acceptance nothing. Belt-and-suspenders: Task B2's `Validate` already rejects any spec that WOULD produce an `At == nil` instruction, so this is unreachable via the content-hosting path in M1 — the rejection here is for `SeedMonsters`' other, non-dungeonspec callers (defense-in-depth, same posture as `monsters.ByRef`'s "error on miss").

```go
// SpawnInstruction describes one monster spawn compiled from a dungeon
// spec — encounter-owned (not dungeonspec-owned; see Task B3's note on
// why dungeonspec only aliases this type) so rpg-api can call
// enc.SeedMonsters(compiled.Spawns) directly, with compiled.Spawns typed
// as []encounter.SpawnInstruction via dungeonspec's alias, zero conversion.
type SpawnInstruction struct {
	RoomID     string
	MonsterRef string
	Count      int
	At         *LocalHex // nil = rolled (M2 only in this milestone); non-nil = placed (M1)
}
```

**offsetX at the SeedMonsters call site — a real gap, not a detail (verified against rpg-toolkit `origin/main`):** Task N1's translation (`local.Col/Row + offsetX`) runs INSIDE `generateDungeonLayout`, where `starts[i]` (offsetX) is a local variable already in scope. `SeedMonsters` runs AFTER `InitDungeon` returns — by then `DungeonParams` (and `starts[i]` with it) is gone; all `SeedMonsters` has is `e.data.Space.Regions []RegionData`, and `RegionData{ID string, Archetype RegionArchetype, Hexes core.HexSet}` (confirmed, `encounter/data.go`) carries no offset field at all. So `SeedMonsters` cannot do `local.Col + offsetX` directly — it has to recover `offsetX` from what IS persisted.

Recovery is simpler than it sounds, and doesn't need a new persisted field: every region occupies a contiguous local-X range `[0, width)` at absolute `X` range `[offsetX, offsetX+width)` (`generateDungeonLayout`'s `starts[i]` construction), so the MINIMUM absolute X across any hex in that region's `Hexes` set recovers `offsetX` exactly (local x=0's row is always a member, for every row 0..height-1, since `regionCubes` enumerates the region's full rectangle). Even better than the raw cube-coordinate route: `core.Hex` already exposes `(h Hex) ToPosition() spatial.Position` (`encounter/core/spatial.go`) — a ready-made one-call conversion (cube → offset, pointy-top orientation hardcoded) — so no manual `CubeCoordinate`/orientation plumbing is needed at all:

```go
// regionOffsetX recovers a region's generateDungeonLayout offsetX
// (starts[i]) from its persisted RegionData.Hexes -- InitDungeon discards
// DungeonParams once the layout is built, and RegionData carries no
// offset field (encounter/data.go), so this is the only source left.
// Every region's local x=0 column is a member of Hexes for every row (see
// regionCubes), so the minimum absolute X across the set IS offsetX.
func regionOffsetX(hexes core.HexSet) int {
	minX := 0
	first := true
	for h := range hexes {
		x := int(math.Round(h.ToPosition().X))
		if first || x < minX {
			minX = x
			first = false
		}
	}
	return minX
}
```

`SeedMonsters` locates the target `SpawnInstruction.RoomID`'s `RegionData` in `e.data.Space.Regions` (linear scan — a handful of rooms per dungeon, no index needed), calls `regionOffsetX` on its `Hexes`, then converts `LocalHex{Col, Row}` → absolute the same way `regionObstacleCandidates` already does: `spatial.OffsetCoordinateToCubeWithOrientation(spatial.Position{X: float64(offsetX + local.Col), Y: float64(local.Row)}, spatial.HexOrientationPointyTop)` → `core.HexFromCube(...)`. M2's Task C1 (count-based safe-cell rolling) reuses this SAME `regionOffsetX` helper for its own region-local reasoning — write it once here, don't duplicate it there.

- [ ] **Step 1: Failing tests**

```go
func TestSeedMonsters_PlacedMonstersSpawnAtTheirCells(t *testing.T) {
	// SpawnInstruction{RoomID: "tomb", MonsterRef: "dnd5e:monsters:skeleton",
	// Count: 1, At: &LocalHex{Col: 4, Row: 2}} resolves via monsters.ByRef and
	// lands at exactly that region-local cell, translated by regionOffsetX
	// (above) — same conversion as Task N1's obstacles, minus the shortcut
	// N1 gets from already being inside generateDungeonLayout.
}

func TestRegionOffsetX_RecoversStartsIFromPersistedHexes(t *testing.T) {
	// Build a 2-region InitDungeon, read back e.data.Space.Regions, assert
	// regionOffsetX(region[1].Hexes) == the SAME offsetX generateDungeonLayout
	// used internally (region[0].Width + 1, per its own doc). Tests the
	// helper in isolation from SeedMonsters' other machinery.
}

func TestSeedMonsters_CombatEntryNeverSeesPartialRoster(t *testing.T) {
	// Same regression shape as design.md's originally-specced Slice C: boss
	// placed at a cell NOT visible from the party's spawn, entrance monster
	// placed at a cell that IS. SeedMonsters must add both under suppressed
	// combat-entry evaluation, then run ONE checkCombatEntry pass — initiative
	// must never contain the boss if only the entrance monster was actually
	// visible when the batch committed.
}

func TestSeedMonsters_SpawnVisibleStillStartsCombat(t *testing.T) {
	// The AddPlayer-doesn't-check trap, reverified on today's origin/main:
	// batching must not lose "combat starts immediately if you spawn already
	// visible."
}

func TestSeedMonsters_UnpinnedInstructionReturnsNotYetSupported(t *testing.T) {
	// At == nil, REGARDLESS of Count (including Count == 1 — an unpinned
	// single-monster room is just as unimplemented in M1 as a Count > 1
	// room; there's no safe-cell-rolling machinery for either yet). M2's
	// Slice C replaces this error path with real safe-cell rolling for
	// every At == nil instruction.
}
```

- [ ] **Step 2: Run → FAIL; Step 3: Implement** — `SeedMonsters(spawns []SpawnInstruction) error`: for each spawn in order (boss-first, per the compiler), if `At != nil`, resolve `MonsterRef` via `monsters.ByRef` (error on miss — validation should make this unreachable), locate the spawn's `RoomID` in `e.data.Space.Regions`, translate `At` to an absolute `core.Hex` via `regionOffsetX` + the offset conversion above, and stage an `AddMonster` call at that position; if `At == nil` (any `Count`), return the M1 scope-boundary error above. Run every staged `AddMonster` with combat-entry evaluation SUPPRESSED (a package-private variant or internal flag — implementer's call, but it must not change `AddMonster`'s existing public contract for non-dungeon callers), then run exactly ONE `checkCombatEntry()` pass after the whole batch.
- [ ] **Step 4: Run → PASS; the full encounter suite MUST stay green** — especially existing combat-entry coverage in `combat_test.go`.
- [ ] **Step 5: Commit** `feat(encounter): SeedMonsters — placed-monster batch seeding under the atomic combat-entry invariant (#<issue>)`

---

## Slice D — rpg-toolkit: dungeon workbench CLI (observability)

### Task D1: `key+seed in → verdict + compiled dump + ASCII floor plan out`

**Files:**
- Create: `encounter/cmd/dungeonspec-workbench/main.go` (thin), `encounter/dungeonspec/workbench.go` (testable core), `encounter/dungeonspec/workbench_test.go`

- [ ] **Step 1: Failing tests on the core (not main)**

```go
func TestWorkbenchReport_ValidSpec(t *testing.T) {
	// placedTombYAML, not cryptYAML: testdata/crypt.yaml doesn't exist (and
	// wouldn't Validate) until M2's Task C2 lifts the M1-only monster-pinning
	// restriction — see Task B2's fixture consequence.
	report, err := dungeonspec.WorkbenchReport(placedTombYAML, 42)
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

**Delta addition — placed entries render at their exact coordinates:**

- [ ] **Step 6: Failing test**

```go
func TestWorkbenchReport_PlacedEntriesAtExactCoordinates(t *testing.T) {
	report, err := dungeonspec.WorkbenchReport(placedTombYAML, 7)
	require.NoError(t, err)
	// At minimum, the report must name every placed ref in its spawn-plan/
	// legend section at its exact compiled coordinate — not just "an
	// obstacle somewhere in the room." A distinct ASCII marker for placed
	// vs rolled (e.g. 'P' vs 'o') is nice-to-have, author's call.
	assert.Contains(t, report, "coffin")
}
```

- [ ] **Step 7: Run → FAIL; Step 8: Implement** (render `PlacedObstacles`/`At`-bearing spawns with their exact coordinates, distinct marker optional); **Step 9: Commit** `feat(dungeonspec): workbench — render placed entries at their exact coordinates (#<issue>)`

**Toolkit PR sequencing:** ship as 2 PRs — **A + B + N (new engine placement slice) + D first** ("spec + registry + placement path + workbench": the M1 toolkit surface, independently green, independently gated), then **C alone in M2** ("SeedMonsters grows count-based rolling": the riskiest remaining piece lands in its own reviewable unit). After each toolkit merge the auto-tagger cuts the release rpg-api's Slice E bumps to.

---

## Slice E (M1, lightened) — rpg-api: content hosting, spec-backed key resolution alongside the legacy crypt path

**Verified code-anchor finding (rpg-api `origin/main`, 2026-07-24 — corrects the original plan's paraphrase):** `resolveDungeonSpec` (`internal/orchestrators/lobby/dungeon_spec.go`) is a `map[DungeonKey]dungeonSpecBuilder` lookup (`dungeonSpecs`), not literally a switch statement, with signature `(DungeonKey, tkenc.DungeonParams, error)` — it carries NO spawn/monster information. `StartEncounter` (`internal/orchestrators/lobby/start_encounter.go`) calls `resolveDungeonSpec` once before the lobby lock, runs `enc.InitDungeon(dungeonParams)`, adds players, then SEPARATELY calls `o.seedRegionMonsters(ctx, enc, encID, dungeonParams, dungeonKey)` — and `seedRegionMonsters` is crypt-specific (built on `cryptMonsterSeedSpecs()`'s hardcoded skeleton+captain table via `regionMonsterAnchor`/`buildMonsterSeedGroups`), not generic over an arbitrary `CompiledDungeon.Spawns` plan. This means a content-backed dungeon key cannot just add a branch inside `resolveDungeonSpec` and reuse the existing monster-seeding call site unchanged — it needs its own spawn path alongside it.

**Do NOT modify or delete** (verified real, unchanged names today; M2 retires these, not M1): `internal/orchestrators/lobby/crypt_monster_seed.go` and its `cryptMonsterSeedSpecs`, `regionAnchorDoor`, `regionMonsterAnchor`, `buildMonsterSeedGroups`, `(*Orchestrator) seedRegionMonsters`; nor `regionEntryAnchor`/`anchorProbeEntity` in `start_encounter.go`.

### Task E1: content dir + embedded validation-at-startup

**Files:**
- Create: `content/dungeons/reference-tomb.yaml` (the delta's M1 acceptance file — a minimal entrance room + the fully-placed tomb, two rooms, honoring the ≥2-rooms/entrance-spawn constraints honestly), `internal/content/content.go` (go:embed + `RPG_CONTENT_DIR` override + `SpecByKey`), `internal/content/content_test.go`
- Modify: `go.mod` (toolkit encounter bump to the release containing A/B/N/D)

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
	// resolves; embedded keys not present in the override dir still
	// resolve (override AUGMENTS the embedded set, it doesn't replace it
	// wholesale). On a KEY COLLISION specifically — the override dir has a
	// file whose `key:` matches an embedded one — the override WINS: this
	// is what makes the edit-restart authoring loop actually work (Task
	// E1's whole point, and the M1 acceptance bar in Slice E's intro) —
	// an author editing content/dungeons/reference-tomb.yaml locally via
	// RPG_CONTENT_DIR must see their edit, not the stale embedded copy.
}
```

- [ ] **Steps 2–4: red → implement → green.** Key detail: `SpecByKey` indexes by each file's `key:` field (decode header cheaply), not filename.
- [ ] **Step 5: Commit** `feat(content)#<issue>: embedded dungeon specs + dev dir override, validated at startup`

### Task E2 (lightened for M1): `StartEncounter` grows a spec-backed branch, crypt path untouched

**Three-state contract, not a bool (Issue: a plain `bool` can't express "found but disabled").** A content-backed key can resolve three ways: (a) found and valid → a `CompiledDungeon` ready to use; (b) found in content but failed `dungeonspec.Load` at startup → the STORED validation error, which the handler must map to `InvalidArgument` carrying the message (design.md's Errors table has always said this — a bare `bool` silently collapses this into "not found," which would wrongly fall through to the legacy map and then to `NotFound`, losing the validation detail entirely); (c) not a content-backed key at all → falls through to the legacy `dungeonSpecs` map, `NotFound` if absent there too. This needs a startup-built registry, not a per-request `Load` call (per-request re-parsing is also wrong for the same reason Task E1's `TestEveryEmbeddedSpecLoads` already treats this as a startup concern, not a request-time one).

**Files:**
- Modify: `internal/orchestrators/lobby/dungeon_spec.go` (new `contentSpecResult{compiled dungeonspec.CompiledDungeon, err error}`; new `loadContentSpecs() map[DungeonKey]contentSpecResult` — iterates `content.AllSpecs()` once, calls `dungeonspec.Load` per key, logs one line per failure per Task E3, stores the result either way; new `DisabledDungeonKeyError{Key DungeonKey, Cause error}` implementing `Error()`/`Unwrap()` so the stored message survives to the handler layer; `resolveContentDungeonSpec(key DungeonKey) (compiled dungeonspec.CompiledDungeon, err error, found bool)` — a three-value return, NOT the bool this task originally sketched — looks up the pre-built registry, wrapping a stored load failure in `DisabledDungeonKeyError`), `internal/orchestrators/lobby/orchestrator.go` (or wherever `New`/`Config` lives — call `loadContentSpecs()` once at construction, store the result on `*Orchestrator`; verify the real constructor's name/file before writing this — not yet confirmed against `origin/main` in this pass)
- Modify: `internal/orchestrators/lobby/start_encounter.go` (after the existing `resolveDungeonSpec`/`InitDungeon`/player-add-loop sequence: branch on `resolveContentDungeonSpec`'s three-value return — `found && err == nil` uses `enc.SeedMonsters(compiled.Spawns)` INSTEAD of `o.seedRegionMonsters`; `found && err != nil` returns the `DisabledDungeonKeyError` immediately, before touching the lobby lock, same posture as `resolveDungeonSpec`'s existing "fail loudly, zero side effects" comment; `!found` falls through to `o.seedRegionMonsters` exactly as today, unchanged)
- Test: extend `internal/orchestrators/lobby/start_encounter_test.go`; new integration test exercising `StartEncounter("reference-tomb")`; a disabled-key case using a deliberately-broken `RPG_CONTENT_DIR` override file

- [ ] **Step 1: Failing integration test** — `StartEncounter("reference-tomb")` → snapshot has 2 zones (entrance + tomb) with declared archetypes, the tomb's placed props/monsters land at their compiled positions, boss present; the EXISTING crypt gates (`internal/integration/dungeon_crypt_test.go`, `internal/integration/lobby_crypt_monster_seed_test.go`) stay green UNTOUCHED — this task must not perturb the crypt's call path at all.
- [ ] **Step 2: Run → FAIL (non-short, Redis testcontainer); Step 3: Implement** the branch described above:

```go
compiled, contentErr, found := o.resolveContentDungeonSpec(in.DungeonKey)
if found {
	if contentErr != nil {
		return nil, contentErr // *DisabledDungeonKeyError; lobbyStatusError maps it (Task E3)
	}
	compiled.Params.RandomSeed = in.RandomSeed // seed is a FIELD, not a call arg
	if err := enc.InitDungeon(compiled.Params); err != nil {
		return nil, fmt.Errorf("init dungeon (key=%q) for encounter %q: %w", in.DungeonKey, encID, err)
	}
	// ... existing player-add loop, unchanged ...
	if err := enc.SeedMonsters(compiled.Spawns); err != nil {
		return nil, fmt.Errorf("seed monsters for encounter %q: %w", encID, err)
	}
} else {
	// existing resolveDungeonSpec / o.seedRegionMonsters path, byte-for-byte unchanged
	// (ErrUnknownDungeonKey if resolveDungeonSpec doesn't recognize it either)
}
```

- [ ] **Step 4: Full non-short suite green; Step 5: Commit** `feat(lobby)#<issue>: StartEncounter resolves content-backed dungeon keys via SeedMonsters, crypt path untouched`

### Task E3: Error surface

**Verified finding, corrects the original plan's assumption (rpg-api `origin/main`, `internal/handlers/dnd5e/lobby/v1alpha1/status.go`):** `lobbyStatusError` (the handler-layer sentinel→gRPC-code mapper, shared by every lobby RPC) does NOT currently have a case for `ErrUnknownDungeonKey` at all — that sentinel falls through to the function's default `Internal` case today. `dungeon_spec.go`'s own comment already admits this ("unclassified sentinel errors fall through lobbyStatusError's default codes.Internal case, an honest mapping for a case no real client can trigger yet") — it was never wired up because no real proto/handler surface has ever threaded a caller-supplied `DungeonKey` through (confirmed: the `StartEncounter` handler builds `StartEncounterInput{PlayerID, LobbyID}` only — `DungeonKey`/`RandomSeed` are never set from the request today). So "unknown key → `NotFound` (existing behavior)" was ASPIRATIONAL in the original plan, not actually true. This task makes it true:

- [ ] Add two cases to `lobbyStatusError` (`internal/handlers/dnd5e/lobby/v1alpha1/status.go`): `errors.Is(err, lobbyorch.ErrUnknownDungeonKey)` → `status.Error(codes.NotFound, "unknown dungeon key")`; `errors.As(err, &disabledErr)` (Task E2's `*lobbyorch.DisabledDungeonKeyError`) → `status.Error(codes.InvalidArgument, disabledErr.Error())`, carrying the stored validation message. Table-test both, plus the existing cases staying green (this function is an exhaustive switch shared by every lobby RPC handler — adding two cases must not perturb the others).
- [ ] Startup (`loadContentSpecs`, Task E2) logs one line per invalid file (file, field, reason) via whatever logger `Orchestrator`'s construction path already uses, and stores the error rather than dropping the key or panicking.
- [ ] Commit `feat(lobby)#<issue>: content validation error surface — disabled-key InvalidArgument, unknown-key NotFound wired into lobbyStatusError`.

---

# Milestone M2 — Migration

## Slice C (M2) — rpg-toolkit: `Encounter.SeedMonsters` grows count-based multi-monster rolling

**This slice EXTENDS `encounter/seed_monsters.go` from M1's Task N2** — that task already built `SeedMonsters`' atomic-batching invariant and its `At`-bearing (placed) path, and explicitly punted any `SpawnInstruction` with `At == nil` (regardless of `Count`, per Task N2's broadened M1 scope boundary) to a "not yet supported" error. This slice replaces that error path with the real N-per-room safe-cell-rolling machinery design.md's v1 body always specced — the invariant-heavy engine work sequenced after the placement-only path (M1) proved the batching mechanism out. It also LIFTS the M1-only `Validate` restriction from Task B2 (count-based `monsters:` and an unpinned `boss` were rejected at load time purely because `SeedMonsters` couldn't fulfill them yet) — that restriction's whole justification disappears once this slice ships.

**Execution order: Task C1 → Task C0 → Task C2 — deliberately NOT numeric order.** Task C1 (the actual rolling implementation) doesn't depend on Task C0 (lifting the `Validate` restriction) at all — `Validate`'s restriction and `SeedMonsters`' capability are two independent things that happen to need to change together eventually. Landing C0 before C1 would open an intermediate commit where `Validate` accepts specs (count-based `monsters:`, unpinned `boss`) that `SeedMonsters` still can't actually seed — the exact "compiles but the engine can't do it" gap the restriction exists to prevent, just reopened for one commit. C1 first, entirely on its own merits (testable via direct `SeedMonsters` calls, no `dungeonspec.Load` involved); C0 second, once C1 has actually proven the capability exists; C2 third, since crypt parity needs C0's lift to even load `testdata/crypt.yaml`. Task names keep their B2/N2-era numbering (C0/C1/C2) for cross-reference continuity — only the READ/EXECUTION order changes.

### Task C1: The invariant test first

**Files:**
- Modify: `encounter/seed_monsters.go`, `encounter/seed_monsters_test.go` (both created by M1's Task N2 — this slice extends, doesn't recreate them)
- Read first: `encounter/encounter.go`'s `AddMonster` (the reinforcement path: unconditional initiative append when `Mode == TurnBased`), `combat.go`'s `checkCombatEntry`, and `crypt_monster_seed.go` in **rpg-api** (`internal/orchestrators/lobby/`) — the fragility being retired (verified real, unchanged names as of 2026-07-24: `cryptMonsterSeedSpecs`, `regionAnchorDoor`, `regionMonsterAnchor`, `buildMonsterSeedGroups`, `(*Orchestrator) seedRegionMonsters`).

**Reuses `regionOffsetX` from Task N2** (`encounter/seed_monsters.go` — recovering a region's `offsetX` from its persisted `RegionData.Hexes` since `InitDungeon` discards `DungeonParams`) for its own region-local safe-cell reasoning below — don't write a second version of this helper.

- [ ] **Step 1: Failing regression test — the partial-roster bug cannot happen, now for rolled multi-monster rooms too**

```go
func TestSeedMonsters_CombatEntryNeverSeesPartialRoster_RolledRooms(t *testing.T) {
	// Party placed at entrance; entrance room has a rolled Count>1 monster
	// group, spawn-visible to a player; boss room has a placed (At-bearing)
	// boss, not visible. Pre-fix behavior with naive per-add seeding in the
	// WRONG order: entrance add → visibility pair → combat starts → boss add
	// → boss reinforced into initiative. SeedMonsters must yield: combat may
	// start AFTER the batch, initiative contains ONLY the engaged (visible)
	// monsters, never the unseen boss — same invariant Task N2 already
	// proved for placed-only rooms, now covering rolled ones too.
}

func TestSeedMonsters_NPerRoomSafeCells(t *testing.T) {
	// count>1 in one room: all placed on distinct, walkable, non-door,
	// non-required-path cells; deterministic per seed. Property-style over
	// 25 seeds.
}

func TestSeedMonsters_RolledNeverUsesAPlacedCell(t *testing.T) {
	// A room mixing place (M1) and count-based monsters/obstacles in the
	// same room (design.md's per-item dial): rolled monster placement must
	// exclude cells already claimed by place entries — the same exclusion
	// Task N1 already wired for obstacles, now exercised for monsters too.
}
```

- [ ] **Step 2: Run → FAIL; Step 3: Implement**

Mechanism (the design left two candidates; this plan picks **batch-with-deferred-entry** because it keeps today's call order and today's spawn-visibility semantics, and M1's Task N2 already built exactly this skeleton): extend `SeedMonsters` so a `SpawnInstruction` with `At == nil` (any `Count`) resolves its ref via `monsters.ByRef` (error on miss — validation should make this unreachable) and places `Count` instances per room on safe cells (reuse the candidate-pool machinery Task N1's `placeRegionObstacles`/`regionObstacleCandidates` uses — same exclusions plus occupied-cell and placed-cell; extract a shared helper only if it stays behavior-identical, with its own test), staged under the SAME suppressed-combat-entry batching Task N2 already built, with ONE `checkCombatEntry()` pass covering both placed and rolled spawns together. Boss-first order preserved from the compiler.

- [ ] **Step 4: Run → PASS; the full encounter suite MUST stay green** — especially the crypt connectivity, boss-axis, and obstacle suites, and every M1 `seed_monsters_test.go` case (the placed path must not regress).
- [ ] **Step 5: Commit** `feat(encounter): SeedMonsters — count-based multi-monster safe-cell rolling (#<issue>)`

### Task C0: Lift the M1-only `Validate` restriction

Runs SECOND (after Task C1 above), per this slice's execution-order note — `SeedMonsters` must actually be able to roll before `Validate` starts allowing specs that need it to.

**Files:**
- Modify: `encounter/dungeonspec/validate.go`, `encounter/dungeonspec/validate_test.go`

- [ ] **Step 1: Failing test — a previously-rejected spec now loads and seeds correctly**

```go
func TestValidate_UnpinnedMonstersAllowedOnceSeedMonstersRolls(t *testing.T) {
	// referenceYAML (design.md's 4-room sunken-crypt example, count-based
	// monsters: in entrance/gallery) and testdata/crypt.yaml (see Task C2)
	// both currently fail Validate with "rolled monster placement lands in
	// M2" (Task B2). Once this task removes those two checks, both must
	// Validate cleanly — this test is the removal's own proof, not just a
	// deletion.
}
```

- [ ] **Step 2: Run → FAIL** (still rejected); **Step 3: Implement** — delete the "count-based `monsters:` entry rejected in M1" and "unpinned boss (no `at`) rejected in M1" checks from `Validate` (added in Task B2). Three things need removing from Task B2's table test, not two: the two corresponding rows themselves, AND the round-1-added `"referenceYAML's own count-based monsters are M1-invalid"` row (Task B2's fixture-consequence note) — that third row's `wantErr` asserts the OLD, now-wrong direction, and this task's own Step 1 above already covers the positive case for the same fixture, so removing (not flipping) it is the correct fix, not a coverage gap.
- [ ] **Step 4: Run → PASS; Step 5: Commit** `feat(dungeonspec): lift M1-only unpinned-monster restriction now that SeedMonsters can roll (#<issue>)`

### Task C2: crypt compiler parity (unblocked by Task C0)

**Files:**
- Create: `encounter/dungeonspec/testdata/crypt.yaml` (port of today's canonical crypt — same regions/sizes/patterns/obstacles incl. `prefer_border` flags, same lock, same monster set as `crypt_monster_seed.go`'s table: 1 skeleton entrance, skeleton-captain boss, `key: crypt` so the compiler's `"<key>-door-<from>-<to>"` rule reproduces `crypt-door-entrance-corridor`/`crypt-door-corridor-boss` byte-for-byte)
- Test: `encounter/dungeonspec/compile_test.go` (extends Task B3's file — these three tests move here from that M1 task, per its scope note, now that `testdata/crypt.yaml`'s count-based monster entries pass `Validate` following Task C0)

- [ ] **Step 1: Failing tests**

```go
func TestLoad_CryptParity(t *testing.T) {
	compiled, err := dungeonspec.Load(cryptYAML)
	require.NoError(t, err)
	// cryptYAML's key is "crypt" — the compiler's door ids must therefore
	// equal today's real rpg-api constants EXACTLY, not just "some
	// generated id": verified against internal/integration/
	// dungeon_crypt_test.go and internal/handlers/dnd5e/v2/encounter/
	// project_test.go on rpg-api origin/main, both of which hardcode these
	// two strings as their own test constants.
	want := encounter.CryptDungeonParams(0,
		"crypt-door-entrance-corridor", "crypt-door-corridor-boss")
	// Field-by-field: Height, Theme, per-region ID/Archetype/Width/Pattern,
	// obstacle specs (refs, counts, flags, PreferBorder), connector lock
	// config (DoorID now included — both sides carry the same names).
	assert.Equal(t, want.Height, compiled.Params.Height)
	// ... exhaustive
}

func TestLoad_SpawnsAreBossFirst(t *testing.T) {
	compiled, _ := dungeonspec.Load(cryptYAML)
	require.NotEmpty(t, compiled.Spawns)
	assert.Equal(t, "boss", compiled.Spawns[0].RoomID) // crypt's boss region id
	// then entrance-to-boss chain order for the rest
}

func TestLoad_Deterministic(t *testing.T) {
	a, _ := dungeonspec.Load(cryptYAML)
	b, _ := dungeonspec.Load(cryptYAML)
	assert.Equal(t, a, b)
}
```

- [ ] **Step 2: Run → FAIL; Step 3: Implement** — no new compiler logic expected here (Task B3's `place`-routing and door-id rule already generalize to count-based rooms); this task should mostly be fixture-writing and assertion-tightening. If it surfaces a real gap in Task B3's implementation, that's a genuine finding to fix here, not a scope violation.
- [ ] **Step 4: Run → PASS; Step 5: Commit** `feat(dungeonspec): crypt compiler parity — count-based monsters now loadable (#<issue>)`

## Migration — crypt to YAML, legacy path deleted

### Task E4 (M2): Port the crypt to content, retire the legacy scatter

**Files:**
- Create: `content/dungeons/crypt.yaml` (the parity file — byte-identical intent to `testdata/crypt.yaml`), `content/dungeons/sunken-crypt.yaml` (the 4-room acceptance file from design.md's original v1 body, if not already shipped as part of `reference-tomb.yaml`'s evolution toward M3)
- Modify: `internal/orchestrators/lobby/dungeon_spec.go` (fold `resolveContentDungeonSpec` and `resolveDungeonSpec` back into ONE resolution path now that every key — crypt included — is content-backed; delete the `dungeonSpecs` map and `defaultDungeonKey`'s builder-closure indirection — the underlying startup-registry mechanism from Task E2 stays exactly as built, only the "does a legacy fallback exist" branch goes away), `internal/orchestrators/lobby/start_encounter.go` (collapse the M1 branch: every key now goes through the SAME registry-backed `resolveContentDungeonSpec` → `enc.SeedMonsters(compiled.Spawns)`; delete the `o.seedRegionMonsters` call entirely), `go.mod` (toolkit encounter bump to the release containing M2's Slice C — the `SeedMonsters` count-based rolling this task's `sunken-crypt`/multi-monster rooms now depend on, same bump pattern as Task E1's `go.mod` line for M1's A/B/N/D release)
- Delete: `internal/orchestrators/lobby/crypt_monster_seed.go`, `internal/orchestrators/lobby/crypt_monster_seed_internal_test.go` (white-box tests calling `regionMonsterAnchor`/`buildMonsterSeedGroups`/`seedRegionMonsters` — compile-breaks the package if left; coverage superseded by M2's `SeedMonsters` invariant tests), AND `internal/orchestrators/lobby/region_entry_anchor_internal_test.go` (the dedicated test that would otherwise keep the orphaned `regionEntryAnchor`/`anchorProbeEntity` helpers alive — delete those helpers too, their only callers live in the deleted files)
- Test: extend `internal/orchestrators/lobby/start_encounter_test.go`; the two REAL integration gates by name: `internal/integration/dungeon_crypt_test.go` (byte-for-byte wire-behavior gate — must stay green untouched on wire expectations) and `internal/integration/lobby_crypt_monster_seed_test.go` (real-Redis composition/determinism suite — CORRECTED characterization, verified by reading the file: it DOES compare `m.Position` values, in two of its three test methods — `TestStartEncounter_RealRedis_SameSeedByteIdenticalPositions` asserts two independent same-seed runs produce identical per-archetype positions, and `TestStartEncounter_RealRedis_PartySizeInvariant` asserts every party size's positions equal a `reference` map captured from the FIRST run — but NEVER against a literal hardcoded coordinate; only `TestStartEncounter_RealRedis_CryptComposition` checks archetype/ref counts without touching positions. Conclusion unchanged: all three stay green, since the migration preserves same-seed determinism and party-size invariance regardless of which mechanism produces the positions. UPDATE its package doc, which narrates the retired call-order fragility, to describe the new `SeedMonsters` invariant instead — keep the tests, fix the story)

- [ ] **Step 1: Failing integration test** — `StartEncounter("sunken-crypt")` (or the migrated `crypt` key) → snapshot has the declared zones/archetypes, per-zone monster counts match the YAML, boss connector locked DC 12; and the EXISTING crypt gates above still pass on wire expectations (parity is the migration proof).
- [ ] **Step 2: Run → FAIL (non-short, Redis testcontainer); Step 3: Implement** — every key now goes through the SAME three-state registry Task E2 already built (`loadContentSpecs()`/`resolveContentDungeonSpec`, built once at startup — not a live `content.SpecByKey` + `dungeonspec.Load` per request, which would re-parse YAML on every call and lose the pre-built error detail Task E3's `DisabledDungeonKeyError` carries):

```go
compiled, contentErr, found := o.resolveContentDungeonSpec(input.DungeonKey)
if !found {
	return nil, lobbyorch.ErrUnknownDungeonKey // NotFound, per Task E3's wiring — no legacy map left to fall through to
}
if contentErr != nil {
	return nil, contentErr // *DisabledDungeonKeyError -> InvalidArgument, per Task E3
}
compiled.Params.RandomSeed = seed // seed is a FIELD, not a call arg
err = enc.InitDungeon(compiled.Params) // InitDungeon takes ONE argument
err = enc.SeedMonsters(compiled.Spawns)
```

  Delete `crypt_monster_seed.go` + the anchor helpers + their internal tests; `CryptDungeonParams` stays in the toolkit as the parity-test fixture (soak per design; do NOT delete this release).
- [ ] **Step 4: Full non-short suite green; Step 5: Commit** `feat(lobby)#<issue>: StartEncounter builds every dungeon from content specs — crypt_monster_seed.go and the M1 dual-path branch both deleted`

---

# Milestone M3 — The reference dungeon (pointer, not a code slice)

Content milestone, not an engineering slice: grow `reference-tomb.yaml` into the full multi-room Synty-bar dungeon (dense placement, brazier light pools, banners, statues) using M1's tooling (`place` + the workbench) as-is; add camera framing (a separate known gap, folded in here per design.md's north star); screenshot-harness evidence against the POLYGON Dungeon promo shot is the acceptance artifact. This dungeon becomes the procedural quality bar. No new schema/engine/api code is anticipated here — if authoring the reference dungeon surfaces a real gap in `place`/validation/the engine placement path, that's new design work to bring back for review, not a silently-absorbed M3 task.

# Milestone M4 — The tuning loop (future design, do not build in this wave)

The in-client control panel ("that room was too tough → adjust → go again") gets its own small design once M1's static rooms are walkable and M3's reference dungeon exists to tune against. Not scoped here.

---

## Acceptance (the bar from the design, run by a human or the parity stack)

1. **(M1)** Edit `content/dungeons/reference-tomb.yaml` (move a placed prop, add a placed monster), restart the api (`RPG_CONTENT_DIR` for the fast loop), start a fresh encounter — the change is live, walkable in the real game route. **No Go was touched.**
2. **(M1)** `go run ./cmd/dungeonspec-workbench -file content/dungeons/reference-tomb.yaml -seed 7` prints VALID, the spawn plan (boss first, placed positions shown), and a legible ASCII floor plan with placed entries at their exact coordinates.
3. **(M2)** `make local-prod` (rpg-deployment#57) against the migrated images: the crypt plays exactly as before the migration (parity), and the crypt is selectable by key through the same content-backed path as every other dungeon.

## Explicitly deferred (do not build)

Difficulty engine (SRD XP budgets/pools), `interactions` (traps/clues), loot, `rooms[].layout`, `locked.tool`, temperaments, quest-line composition, per-room heights — all reserved seats per design.md's v1 body. Added by the delta: **table-rolls / choose-from-list** (deliberately not designed — Kirk 2026-07-24: fully-explicit and fully-rolled bracket the space; the middle notch gets designed after both ends have been played); **the in-client tuning panel** (M4, future design, once M1's static rooms are walkable and M3's reference dungeon exists to tune against).
