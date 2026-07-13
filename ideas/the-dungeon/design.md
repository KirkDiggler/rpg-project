# The Dungeon: walk into a room and the fight starts

## Status: Design — survey-verified 2026-07-13 (file:line for every claim); two forks await Kirk

## The Problem

Combat entry is dev tooling: `devseed --inject-combat` writes a goblin + TURN_BASED
into Redis out-of-process, and clients learn about it via reconnect. There are no
rooms — the encounter stack has no space concept (visibility is a pure SightRange
radius), no walls, no monster seeding. The Dungeon Night capstone needs all of it.

This design is also **debt repayment**: the 2026-07-13 foundation audit ranked the
platform's dishonesty, and wave 1 below directly retires parts of three of its top-six
items (the Hex/spatial duplication, the `Space.Walls` contract lie, and the start of
relocating dungeon rules content into the toolkit).

## Survey verdicts (don't re-derive; citations in the survey record)

- **The Space bridge is small, not the full ADR-0034 consolidation.** Wall mechanics
  already exist and work in `tools/spatial` (`WallEntity` blocks LOS and movement via
  `BasicRoom.IsLineOfSightBlocked` / `canPlaceEntityUnsafe`); serialization exists
  (`environments.EnvironmentData.Walls`); `encounter/core.Hex` and
  `spatial.CubeCoordinate` are the same cube math with different field names (~20-30
  line bridge). Perception has zero dnd5e imports — threading a `spatial.Room` in is
  additive. ADR-0034's module split remains worthwhile but is orthogonal.
- **Combat entry is a toolkit rule, by precedent.** The toolkit already self-transitions
  mode at combat END (`checkEncounterEnd` fires when the last hostile dies; `SetMode`
  *rejects* external `ModeEnded`). Entry mirrors it: when a player↔monster visibility
  pair forms (the same LOS path that publishes `EntityAppeared`), the encounter flips
  itself to TURN_BASED and the existing #638 kick drives an NPC-first order. `SetMode`'s
  only production caller today is the dev inject — the real trigger replaces scaffolding.
- **Monster seeding: fix the spawn stub, don't bypass it.** `tools/spawn` dies on one
  literal stub (`getRoomFromSpatial` — it needs a `RoomOrchestrator.GetRoom` lookup that
  already exists in the same package). Fixing it unblocks the entire existing spawn
  engine. The `rulebooks/dnd5e/dungeon` data model (`MonsterPlacementData`,
  `SpawnZoneData`) is well-shaped but has ZERO callers — design value later, no
  integration credit now.
- **Zero proto changes.** `Space{hexes, walls, entities, zones}`, `Wall` (doors are
  walls with `DOOR_*` kinds), and `GeometryRevealed` (hexes+walls) all exist on the
  wire, unpopulated. Monster entities already stream LOS-filtered. Producers are the
  gap, not schema. Web is two mechanical fixes: HexGrid's `Wall` import points at the
  old v1alpha1 type, and EncounterMap never threads a `walls` prop (rendering itself —
  `ShadedHexWall` — is built and waiting).
- **Corrected premise**: "doors already flow revealed_walls" was wire-shape only — no
  wall data model exists anywhere in `encounter/` today. `DoorData` is `{ID, Position,
  Open, Locked}`, not geometry.

## Decisions (made by principle; not open)

- **Space persists as a snapshot, not seed-regeneration**: `SpaceData{Walls
  []WallSegmentData}` on `encounter.Data`, populated once from `environments.QuickRoom`
  at encounter creation. DoorData already persists mutable state directly; destroyed
  walls/opened doors can't replay from a seed. (Pick ONE representation.)
- **Whole-room wall visibility for wave 1** — no per-viewer wall reveal yet; fog-of-war
  partial knowledge is wave 2+ if wanted.
- **"Hostile" == "is a monster" for wave 1** — no faction model exists; the combat-entry
  predicate names the seam where factions land later.

## Forks — RESOLVED (Kirk, 2026-07-13)

1. **Combat-entry check runs INLINE at the mutation sites** (`Move`/`AddMonster`),
   exactly like `checkEncounterEnd` — symmetric, cannot be forgotten. Kirk: "def
   inline." The kicked-method alternative's silent-no-combat failure mode is the
   worse bug class.
2. **Wave-1 seeding is fixed placement of 2 goblins** via `monster.NewGoblin` through
   the fixed spawn engine; `rulebooks/dnd5e/dungeon`'s placement data model becomes
   real `SpawnConfig` input in wave 2.

## Wave 1 — the walled room ("walk into a room, the fight starts")

All toolkit steps first (one toolkit PR or two small ones), then api, then web:

1. `SpaceData{Walls}` on `encounter.Data`, populated from `environments.QuickRoom` at
   creation; room registered with a `RoomOrchestrator` in the same step (this is also
   half of the spawn-stub fix).
2. Hex↔CubeCoordinate↔Position converter (~20-30 lines).
3. Wall-aware `VisibleHexesAt`/`CanSeeAt` via `room.IsLineOfSightBlocked` (replaces the
   radius stub's blindness to geometry; SightRange still caps distance).
4. Wall-blocked movement via `room.CanPlaceEntity`.
5. Combat-entry self-transition on player↔monster visibility-pair formation (fork 1's
   answer decides the exact shape); the #638 kick already handles NPC-first orders.
6. `tools/spawn`: fix `getRoomFromSpatial` via `RoomOrchestrator` wiring.
7. rpg-api: populate `Space.Walls` in the projector — zero proto changes.
8. rpg-api: seed 2 goblins through the fixed spawn engine at StartEncounter (replaces
   `--inject-combat` as the combat source; the inject tool stays for playtest control).
9. web: swap HexGrid's `Wall` import to the v1alpha2 type + thread `walls` through
   EncounterMap.

**Done when (playtest bar):** party assembles via the lobby, `StartEncounter` places
them in a walled room containing 2 goblins, someone moves, a goblin crosses into view,
**combat starts by rule** — no devseed, no restart — walls render, LOS respects them,
movement can't cross them, the fight narrates to victory.

## Wave 2+ (sequenced, not designed here)

Full ADR-0034 consolidation; multi-room + doors as real wall-geometry links;
`rulebooks/dnd5e/dungeon` placement data as SpawnConfig; faction model; per-viewer wall
reveal; dungeon settings (theme/difficulty/length — old-vs-new gap row 3).

## Relationship to the foundation-audit week plan

Runs as wave 3 of the bonus week, after: (1) the v1-stack deletion in rpg-api (audit
debt #1 — unblocked: the web makes zero v1alpha1 encounter RPC calls, type-imports
only) and (2) the web character-sheet honesty fix (audit debt #3). The reconnect-
fidelity theme (`last_seen_sequence` + web resume #444 + snapshot statuses toolkit#754)
is the week's stretch wave.

## Load-bearing digest

- The walled room is a small internal toolkit bridge — the heavy machinery (wall LOS,
  movement blocking, serialization, spawn engine) already exists; wave 1 connects it.
- Combat entry mirrors combat end: the toolkit self-transitions; the api only kicks.
- Zero contract changes; the web work is two mechanical wirings.
- Building the room retires foundation debt (#2 partially, #4's Walls half, starts #5)
  — game progress and layer honesty are the same motion this week.
