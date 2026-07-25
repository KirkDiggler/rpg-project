# Dungeon Authoring — YAML dungeon definitions (v1)

## Status: Design — approved in session by Kirk 2026-07-23; this PR is the review surface. Implementation slices dispatch after merge-approval of this design. Extended by the §Design delta — static placement section (approved in session 2026-07-24) before implementation began.

North star: **authoring a dungeon should be editing one data file, not touching three Go files across two repos.** First for us (the dev team), later for designer tools, someday maybe player uploads — which is a *constraint* (the format must be strictly validatable), not a feature.

Driving acceptance case (Kirk): **author a 4-room crypt — entrance, a chamber with clues, a trap room to cross, a boss tomb — without writing Go.** v1 ships the fields that need (near-)zero new rules machinery — the two honest exceptions are a monster-ref registry and a batch seeding entry point, both plumbing over constructors and placement machinery that already exist, not new rules content. Every richer concept gets a reserved seat, not an implementation.

## Context — what exists today (verified 2026-07-23)

- `encounter.InitDungeon` (rpg-toolkit `encounter/dungeon.go`) generates an N-region linear-chain dungeon from `DungeonParams{Regions[], Connectors[], Height, RandomSeed, Theme}`; regions carry `{ID, Archetype, Width, Pattern, Obstacles[]}`; connectors carry `{DoorID, Locked, LockDC, LockAbility}`. Obstacle placement is path-safe and best-effort (#818/#819).
- The only "authored" dungeon is `CryptDungeonParams` (`encounter/crypt_dungeon.go`) — a compiled Go constructor.
- Monster composition lives in a **hardcoded table in rpg-api** (`internal/orchestrators/lobby/crypt_monster_seed.go`) whose call order is load-bearing (boss must seed before entrance) and enforced only by a comment.
- rpg-api resolves a dungeon **key** (`resolveDungeonSpec`, `dungeon_spec.go`) → calls the toolkit constructor. The wire (v1alpha2) already carries zones/archetypes, theme, walls, obstacles, monsters — **v1 of this design needs no proto or web changes.**

So one dungeon's definition is scattered across three sites in two repos. This design collapses it into one file.

## Decisions (resolved in the 2026-07-23 brainstorm)

1. **Authoring-first, not lobby knobs.** The configuring user today is the dev team editing files. Host-facing knobs (difficulty/length dials in the lobby) come later as parameters that select/override authored templates — knobs are a *view* over this layer, not a rival system.
2. **The authored unit is one dungeon per file.** Composition into mega-dungeons/quest lines is future work; dungeons are referenceable by `key` so that door stays open. Nothing else is built for it.
3. **Flow-level authoring; the generator still rolls geometry.** A spec fixes room count, archetypes, sizes, monsters, locks; each run's floor plan is rolled from the runtime seed. Hand-drawn room layouts are a reserved seat (`rooms[].layout`), not v1.
4. **Schema + compiler live in the toolkit; content files live in rpg-api.** The schema is rules-adjacent (the moment "difficulty: hard" means SRD XP-budget math, interpretation must sit with the rules engine) and validation must live where future untrusted uploads would be checked. Content iterates at api deploy speed — add a YAML file, restart, new dungeon; no toolkit release.
5. **v1 is explicit-only.** Literal monster refs and counts, literal sizes, literal boss. The difficulty engine (SRD XP budgets per party level, CR-banded pools) is the designed-for v2, not v1.

## Components

### rpg-toolkit: new sub-package `encounter/dungeonspec`

- **Schema structs** (versioned): `DungeonSpec{Version, Key, Name, Theme, Height, Rooms[], Connectors[]}`; `RoomSpec{ID, Archetype, Width, Monsters[], Boss?, Obstacles[], Pattern?}` — **height is dungeon-level, not per-room**, because that is what the generator actually supports (`DungeonParams.Height` is shared by every region; `DungeonRegionParams` has only `Width`). The schema stays honest to the engine rather than promising a knob that silently wouldn't work; per-room `height` is a reserved seat if the generator ever grows per-region heights; `MonsterEntry{Ref, Count}`; `BossEntry{Ref}`; `ObstacleEntry{Ref, Count, BlocksMovement?, BlocksLoS?}` (blocks default true/true); `ConnectorSpec{From, To, Locked?{DC, Ability}}`. v1 deliberately drops `DungeonConnectorParams.LockTool` (tool-proficiency unlock) — `locked.tool` is a reserved seat, added when a spec first needs it.
- **Strict decode**: YAML via `gopkg.in/yaml.v3` with `KnownFields(true)` — unknown fields are errors, so `mosnters:` fails loudly instead of silently producing an empty room.
- **Validation** (all checks table-tested): `version == 1`; `key`/`name` non-empty; ≥2 rooms with unique ids; connectors reference declared room ids and form a connected linear chain (v1 generator constraint); exactly one room with archetype `boss`; `boss` entry only on the boss room; refs match `module:type:id` shape **and monster/boss refs resolve in the monster registry** (author-time failure, not spawn-time); `pattern` ∈ {`empty`, `scattered`} (the generator silently falls back on unknown patterns — validation refuses instead); sizes mirror the generator's real constraints — `height` ≥ 4, every room `width` ≥ 4, **and the boss room's primary playable axis `min(boss.width, height)` exceeds 6** (the `ArchetypeBoss` invariant; without this rule a width-5 boss room passes naive minimums and fails at `InitDungeon`); counts ≥ 1; lock DC in a sane band (1–30); ability is a known ability ref.
- **Compiler**: `Load(bytes []byte) (CompiledDungeon, error)` where

  ```go
  type CompiledDungeon struct {
      Params DungeonParams        // geometry: regions, connectors, theme
      Spawns []SpawnInstruction   // ordered: boss first, then entrance-to-boss chain order
  }
  type SpawnInstruction struct {
      RoomID     string
      MonsterRef string
      Count      int
  }
  ```

  The compiler orders `Spawns` **boss-first**, turning today's comment-enforced "seed boss before entrance or AddMonster's reinforcement path drags it into initiative" invariant into data. The fragile call-order convention in `crypt_monster_seed.go` is deleted by this design, not re-documented. (Ordering is defense-in-depth; the real fix is the seeding invariant below.)

- **Monster registry** (rulebooks/dnd5e `monsters` package): monsters today are one-off constructors (`monsters.NewSkeleton`, `monsters.NewSkeletonCaptain`, …) with **no existing map from a ref string back to its constructor** — unlike obstacle refs, which stay opaque end to end. New: `monsters.ByRef(ref string) (Constructor, bool)` over the canonical `refs.Monsters.*` set. This is a lookup table over constructors that already exist — no new stat interpretation — but it is honestly new rules-layer plumbing, and it lives toolkit-side because matching a ref to a stat bundle is a rules concern. (The `encounter` module already depends on `rulebooks/dnd5e`, so `Load` can validate refs against the registry at author time — a bad ref fails at file load, never at spawn.)

- **Spawn execution: `Encounter.SeedMonsters(spawns []SpawnInstruction)`** — toolkit-side, replacing rpg-api's hand-rolled anchor placement. Two things today's code deliberately dodges that this must own explicitly:
  1. **N > 1 monsters per room.** `crypt_monster_seed.go` places exactly one deterministic-anchor monster per region *by design*, to avoid `AddMonster`'s combat-visibility reinforcement path misfiring. `SeedMonsters` places N per room via seeded deterministic selection from the region's **safe-cell pool** — the same exclusions obstacle placement already uses (never a wall, door, required-path, or occupied cell), plus the existing out-of-sight-from-the-region's-entrance-door oracle where applicable.
  2. **The seeding invariant: combat entry never observes a partially-seeded dungeon.** The whole boss-before-entrance fragility exists because monsters added after a visibility pair forms get reinforced into a live initiative. `SeedMonsters` executes the full plan under that invariant — candidate mechanisms: seed before party placement, or batch semantics that defer combat-entry evaluation until the plan completes. The implementation plan picks based on `StartEncounter`'s actual sequencing; boss-first ordering stays as defense-in-depth either way. Trap for the implementer, verified on origin/main: `AddPlayer` does NOT call `checkCombatEntry` (only `AddMonster`/`Move` do) — so the seed-before-party mechanism silently loses today's "combat starts immediately if you spawn already visible" behavior unless an explicit `checkCombatEntry()` runs after the last `AddPlayer`.

### rpg-api: content hosting + execution (zero rules)

- `content/dungeons/*.yaml`, embedded with `go:embed`; optional `RPG_CONTENT_DIR` env override so dev iteration is edit-file-restart, no rebuild.
- Startup: every embedded/override file is loaded through `dungeonspec.Load`; failures log the validation error and disable that key (fail fast, never player-facing).
- `resolveDungeonSpec(key)`: hardcoded switch → lookup by each file's `key:` field.
- `StartEncounter`: `Load` → `InitDungeon(compiled.Params, seed)` → `enc.SeedMonsters(compiled.Spawns)`. Api never interprets a spec and no longer sequences monster placement at all — it moves bytes and calls two toolkit entry points. (`crypt_monster_seed.go` deletes entirely.)

### protos / rpg-dnd5e-web: no changes

The wire already carries zones + archetypes (protos#196), theme, walls/doors, obstacles (api#702), and monsters. A YAML-authored dungeon is indistinguishable on the wire from today's hardcoded crypt. (Client-side theming/dressing consume these via rpg-dnd5e-web#558/#569 — independent work.)

## Schema v1 — reference example

```yaml
version: 1
key: sunken-crypt
name: The Sunken Crypt
theme: crypt
height: 8                      # shared by every room — the generator's real shape
rooms:
  - id: entrance
    archetype: entrance
    width: 10
    monsters:
      - { ref: "dnd5e:monsters:skeleton", count: 2 }
    obstacles:
      - { ref: "dnd5e:props:obelisk", count: 1 }
      - { ref: "dnd5e:props:pillar", count: 2 }
  - id: gallery
    archetype: chamber
    width: 8
    monsters:
      - { ref: "dnd5e:monsters:ghoul", count: 1 }
  - id: trap-crossing            # v1: an empty corridor; trap content lands in the
    archetype: corridor          # reserved `interactions` seat (see Deferred)
    width: 6
  - id: tomb
    archetype: boss
    width: 12
    boss: { ref: "dnd5e:monsters:skeleton-captain" }
    obstacles:
      - { ref: "dnd5e:props:coffin", count: 1, blocks_los: false }
      - { ref: "dnd5e:props:altar", count: 1 }
connectors:
  - { from: entrance, to: gallery }
  - { from: gallery, to: trap-crossing }
  - { from: trap-crossing, to: tomb, locked: { dc: 12, ability: dex } }
```

Notes:
- **The seed is not in the file.** A spec describes a dungeon; a seed picks one realization. Seed stays a runtime `StartEncounter` input (and rpg-toolkit#836 makes the resolved seed persistent/loggable so any realization is reproducible).
- `pattern` (optional, per room): `empty` (default, per rpg-toolkit#835 — set pieces carry the cover role) or `scattered` (today's `PatternRandom`).
- `theme` flows verbatim to `SpaceData.Theme` → wire `Space.theme` (opaque to toolkit, consumed by the client).

## Reserved seats (designed-for, deliberately absent in v1)

| Seat | Where it lands later | What it will hold |
|---|---|---|
| `rooms[].monsters[].pool` / top-level `difficulty` | toolkit compiler | SRD XP-budget difficulty engine: thresholds per party level × size, CR-banded pools — the first v2 feature and the reason the schema is toolkit-owned |
| `rooms[].interactions` | toolkit + existing `Interact`/`InputRequired` verb | traps, perception/insight/religion checks, clues, puzzles — discovery content riding the proven skill-check flow (locked doors use it today) |
| `rooms[].layout` | toolkit generator | hand-drawn room geometry (explicit hexes) when flow-level isn't enough |
| `rooms[].height` | toolkit generator | per-region heights, if the generator ever drops the shared-height constraint |
| `loot` (room- or dungeon-level) | toolkit + inventory (equipment shipped 2026-07-21) | loot tables rolled on chest-open / boss-kill |
| monster `temperament` | toolkit AI | aggressive / sneaky behavior archetypes |
| dungeon references in a parent file | future quest-line format | composition via `key` |

Each seat is an additive field on an existing list/struct; none requires reshaping v1 files.

## Data flow

```
StartEncounter(key, seed?)
  → api: content lookup by key → raw bytes            (content store)
  → toolkit: dungeonspec.Load(bytes)                  (validate + compile)
  → api: enc.InitDungeon(compiled.Params, seed)       (geometry/walls/obstacles/doors)
  → api: enc.SeedMonsters(compiled.Spawns)            (registry lookup + N-per-room safe-cell
                                                       placement + atomic seeding, toolkit-side)
  → existing projection → wire                        (zones, theme, walls, obstacles, entities)
```

Per-call `Load` (files are tiny; no cache invalidation complexity) **plus** startup validation of every file.

## Errors & versioning

| Failure | Behavior |
|---|---|
| Malformed YAML / failed validation | Startup: log with file+field detail, disable the key. Runtime request for a disabled key: `InvalidArgument` carrying the validation message. |
| Unknown key | `NotFound` (existing behavior). |
| Compiles but generator rejects (should be near-unreachable — validation mirrors generator minimums) | Existing `InitDungeon` error path. |
| `version: 2` file under a v1 loader | "unsupported spec version" at load — future loaders add versions; old files keep working. |

## Migration — this design deletes the scatter

1. Port the canonical crypt to `content/dungeons/crypt.yaml`.
2. **Parity test**: `crypt.yaml` compiles to params equivalent to `CryptDungeonParams(seed, …)` (regions, connectors, lock config, obstacles) and a spawn plan equivalent to today's table — asserted field-by-field.
3. Delete `crypt_monster_seed.go`'s table + order-dependency comment; `StartEncounter` drives spawns from `CompiledDungeon.Spawns`.
4. `CryptDungeonParams` retires to test-fixture duty (or deletes once the parity test soaks).
5. Ship `sunken-crypt.yaml` (the 4-room acceptance case above) as the second file — a dungeon nobody hand-coded.

## Testing

- **Toolkit `dungeonspec`**: decode round-trip; validation table tests (every rule red+green, including unknown-field strictness, unknown `pattern`, and unresolvable monster refs); crypt parity; determinism (same spec+seed → identical `DungeonParams` and layout); spawn-order test (boss first, always).
- **Toolkit registry + seeding**: every canonical `refs.Monsters.*` ref resolves via `ByRef`; `SeedMonsters` places N-per-room only on safe cells (never wall/door/required-path/occupied — same exclusions as obstacle placement, property-tested across seeds); the seeding invariant pinned by regression — entrance monsters + boss seeded with a party present never yields a partial-roster initiative (the exact bug class `crypt_monster_seed.go`'s call-order comment guarded).
- **rpg-api**: startup test — every shipped content file must load (a broken commit fails CI, not prod); integration test `StartEncounter("sunken-crypt")` → snapshot has 4 zones with declared archetypes, per-zone monster counts match, boss connector locked with DC 12.
- **Acceptance (manual, the bar)**: edit a new YAML, restart api, walk the dungeon in the testbed — no Go touched.

## Layer-honesty ledger

| Touch | Layer | Notes |
|---|---|---|
| Schema, validation, compile → `DungeonParams` + ordered `Spawns` | toolkit | rules-adjacent interpretation; new dep `gopkg.in/yaml.v3` in the encounter module |
| Monster registry (`monsters.ByRef`) | toolkit (rulebooks/dnd5e) | new plumbing over existing constructors; ref→stat-bundle is rules-owned |
| `Encounter.SeedMonsters` — N-per-room safe-cell placement + atomic-seeding invariant | toolkit | placement/visibility semantics are rules; retires api-side sequencing |
| Content files, key lookup, startup validation, two toolkit calls | api | orchestration by key; zero interpretation |
| Wire | — | **no changes** |
| Web | — | **no changes** |

## Open questions (for this PR's review)

1. Package name: `encounter/dungeonspec` vs a top-level `dungeonspec` module — proposed: sub-package of `encounter` (it compiles to `encounter` types; a separate module adds release friction for no boundary gain).
2. YAML dep in the toolkit: `gopkg.in/yaml.v3` strict mode as proposed, or keep the toolkit YAML-free (accept JSON bytes; api converts)? Proposed: yaml.v3 — the format is the contract, and validation should see the author's actual file.
3. Does `CryptDungeonParams` delete immediately after parity, or soak one release as a fixture? Proposed: soak.

---

## Design delta — static placement (`place`) — approved 2026-07-24

**Status:** approved by Kirk in session 2026-07-24; this PR is the review
surface. v1 had not started implementation, so this delta revises v1 itself —
v1 + this delta implement together as one wave.

### North star, sharpened

The POLYGON Dungeon pack promo shot is the explicit visual bar. The goal is a
**fully-authored reference dungeon** — every prop, monster, and light pool
placed by hand as data — that (a) is walkable in the real game route and (b)
becomes the measuring stick for procedural output. Kirk's framing: start fully
explicit, then release aspects to randomness one piece at a time, learning what
each release costs visually. This requires the explicit⟷random dial to be
**per-item, not global** — which v1's flow-level-only schema cannot express.

Accepted carve-out from the bar: **elevation** (the promo shot's sunken arena,
stairs, balconies). The hex engine is single-level; we chase the shot's mood
(light pools, density, rhythm) on one level. Camera framing is a separate known
gap, folded into the reference-dungeon milestone (M3 below).

### Schema: the `place` block

`RoomSpec` gains one optional field:

```yaml
- id: tomb
  archetype: boss
  width: 12
  boss: { ref: "dnd5e:monsters:skeleton-captain", at: [7, 5] }
  place:
    - { ref: "dnd5e:props:coffin",        at: [6, 3], blocks_los: false }
    - { ref: "dnd5e:props:altar",         at: [9, 3] }
    - { ref: "dnd5e:props:statue-reaper", at: [1, 1] }
    - { ref: "dnd5e:props:brazier",       at: [3, 1] }
    - { ref: "dnd5e:props:brazier",       at: [3, 6] }
    - { ref: "dnd5e:monsters:skeleton",   at: [4, 2] }
```

- **One mixed list, routed by ref type.** `dnd5e:props:*` → placed obstacle;
  `dnd5e:monsters:*` → placed spawn. Any other ref type is a validation error.
  `PlacedEntry{Ref, At [2]int, BlocksMovement *bool, BlocksLoS *bool}` — the
  blocking flags carry the same defaults/semantics as `ObstacleEntry` and are
  meaningful only for props (validation rejects them on monster entries).
- **`BossEntry` gains optional `at`.** The boss designation stays where v1 put
  it (`boss:` on the boss room — all archetype invariants unchanged); `at` pins
  its position. A boss ref may NOT also appear in `place` (validation error —
  one authority for the boss).
- **The dial is per-item.** `place` coexists with the count-based `obstacles`
  and `monsters` lists in the same room: placed entries are honored first, then
  count-based entries roll into the remaining safe cells (placed cells are
  excluded from the roll pool). A fully static room uses `place` only. There is
  NO `mode:` flag — what the author wrote is the mode.
- **Coordinates are room-local**: `at: [col, row]` in the room's own grid, so a
  room definition is self-contained and reorderable. Pinned against the
  engine's real coordinate frame (verified on rpg-toolkit `origin/main`,
  `encounter/dungeon.go`): `col` is the region's local X offset in
  `[0, width)`, `row` is the shared dungeon `height`'s Y offset in
  `[0, height)` — exactly the local `(x, y)` pair `placeRegionObstacles`/
  `regionObstacleCandidates` already index rolled obstacle candidates by,
  before the region's own `offsetX` (`starts[i]` in `generateDungeonLayout`)
  gets added to land on the dungeon's one absolute hex grid. rpg-api#399
  already unified the dungeon onto one continuous absolute space per encounter
  (regions are X-offset windows into it, not separate rooms); `place`'s
  room-local coordinates are purely an authoring convenience layered on top of
  that — the compiler must NOT attempt the offset translation itself (that's
  `generateDungeonLayout`'s own layout-time arithmetic, and duplicating it in
  `dungeonspec` would drift the moment column math changes there); the engine
  adds `offsetX` at placement time, exactly as it already does for rolled
  candidates. Row `height/2` (`doorRow`, see Validation below) is reserved in
  every room regardless of archetype and is therefore not addressable by
  `place`.
- **Lighting stays emergent — no lighting fields.** The client derives mood
  lights from light-emitting prop positions (brazier/torch/candle; web #588).
  Placing braziers IS authoring the light pools. Mesh color/material authoring
  is likewise absent.

### Validation additions (load-time, same file-or-it-doesn't-load contract)

- `at` in-bounds: `col` in `[0, room.width)`, `row` in `[0, height)` (the
  shared dungeon height, per v1's existing per-dungeon-not-per-room rule).
- At most one placed entry per cell — a room's `place` entries and its
  (optional) `boss.at`, if set, share one collision domain; a boss pinned onto
  the same cell as a placed prop is the same error as two placed props
  colliding.
- **No placement on row `height/2`** (`doorRow`). This is the engine's
  guaranteed clear lane through every room — the same lane rolled obstacles
  already respect — and it's what makes load-time traversability free: an
  author never has to reason about pathing when placing static props, just
  avoid this one row. Verified against
  `encounter/dungeon.go`'s `placeRegionObstacles`/`regionObstacleCandidates`:
  the engine already excludes this entire row from every region's obstacle
  candidate pool, in every region regardless of archetype — a uniform,
  over-conservative reservation (a boss/interior region's actual required path
  spans the row's full width; an entrance/terminal region's spans only half),
  not an archetype-specific one. Forbidding `place` on that same row therefore
  gives placed entries the IDENTICAL traversability guarantee rolled obstacles
  already have, by construction — no separate path-oracle re-implementation is
  needed at spec-load time; the row exclusion alone is provably sufficient.
  This sharpens the original framing ("no placement on a door cell"): the
  literal door hex sits in the boundary column *between* two regions, which is
  outside any single room's own local coordinate space and was never
  addressable via `at` to begin with — the real, engine-verified rule is the
  full reserved row, not the door hex itself.
- Ref type must be props or monsters; monster refs resolve via the registry;
  prop refs stay opaque end-to-end (client-resolved), exactly like v1
  obstacles.
- Placed entries are hard guarantees (fail the file), while count-based rolls
  keep today's best-effort semantics (#818/#819) in whatever cells remain.

### Compiler & engine

- `DungeonRegionParams` grows `PlacedObstacles []PlacedObstacleSpec{Ref, At,
  BlocksMovement, BlocksLoS}` (exact naming per engine conventions); `At` is
  the same room-local `(col, row)` pair the schema carries, untranslated —
  `InitDungeon` places them verbatim after geometry, before rolled obstacles;
  the rolled pool excludes placed cells.
- `SpawnInstruction` gains an optional position (`At`); placed monsters compile
  to single-count instructions with `At` set; rolled monsters keep `Count` with
  no position. Spawn ordering (boss-first) is unchanged.
- **The atomic-seeding invariant applies identically to placed monsters** —
  combat entry never observes a partially-seeded dungeon. If an author places a
  monster within spawn line-of-sight of the party, combat starts immediately:
  that is correct authored behavior, not a bug.
- **Wire and client: still zero changes.** Placed and rolled entities are
  indistinguishable on the wire; every renderer improvement applies to both.

### Explicitly out of scope for this delta (decisions, not omissions)

- **Table-rolls / choose-from-list**: deliberately NOT designed — not even as a
  reserved sketch. Kirk 2026-07-24: fully-explicit and fully-rolled bracket the
  space; the middle notch gets designed after both ends have been played.
- **Interactions** (e.g. "hidden monster wakes when the party nears the
  coffin"): stays in the existing reserved `interactions` seat.
- **Per-room height**: still a reserved seat (real generator constraint).
- **Elevation / multi-level**: accepted carve-out, see north star.
- **In-client tuning panel** ("that room was too tough → adjust → go again"):
  its own small design once static rooms are walkable (M4). Until then the loop
  is edit-YAML → restart api (`RPG_CONTENT_DIR`, no rebuild) → replay, plus the
  workbench for instant previews.

### Delivery milestones (plan.md is revised to this order)

- **M1 — "The Tomb, walkable."** Registry (slice A unchanged); schema/
  validation/compiler including `place` (slice B extended); the engine
  placement path (new slice); workbench CLI whose ASCII floor plan renders
  placed entries (slice D extended); api content hosting wiring a NEW dungeon
  key through the spec path (slice E, lightened) — the existing crypt keeps its
  legacy hardcoded path, untouched. Acceptance: edit `reference-tomb.yaml` (a
  minimal entrance room + the fully-placed tomb — two rooms, honoring the ≥2
  rooms/entrance-spawn constraints honestly), restart the api, walk the room in
  the real game route.
- **M2 — Migration.** `SeedMonsters` for count-based multi-monster rooms
  (slice C as planned — the invariant-heavy engine work lands after the tool
  proves itself); crypt ported to YAML behind the parity test;
  `crypt_monster_seed.go` and the legacy path deleted.
- **M3 — The reference dungeon.** Content milestone: grow the tomb into the
  full multi-room Synty-bar dungeon (dense placement, brazier light pools,
  banners, statues); camera framing; screenshot-harness evidence against the
  inspiration shot is the acceptance artifact. This dungeon becomes the
  procedural quality bar.
- **M4 — The tuning loop** (future design, do not build in this wave): the
  in-client control panel — restart encounter, adjust, go again.
