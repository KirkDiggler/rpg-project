# The Dungeon — Wave 2: multi-room, doors, traversal, a boss room

## Status: Design — survey-verified 2026-07-19 (file:line for every claim). Extends `design.md` (wave 1). Six forks await Kirk (§Forks).

North star (May 2026 playtest): **4-player co-op, multi-room dungeon, locked doors, a boss room** — mouse-only, no devseed.

## The headline finding: this wave is mostly ASSEMBLY, not invention

Wave 1 ("the walled room", rpg-toolkit#757) plus "Wave 2.7/2.9" work and the dead multi-room orchestrator mean most shapes already exist. The honest inventory before designing anything new:

**Already built (verified):**

- **Door verbs + lock/unlock skill check** — `encounter/data.go:170` `DoorData{ID, Position core.Hex, Open, Locked, LockDC, LockAbility, LockTool}` on `Data.Doors`; `encounter/encounter.go:1013` `Encounter.OpenDoor(playerID, doorID)`; `encounter/prompts.go:130` `AttemptUnlock` issues a skill-check prompt when `Locked`, `SubmitCheck` on success clears `Locked` then calls `OpenDoor` internally (`prompts.go:268`). The toolkit deliberately does **not** gate `OpenDoor` on `Locked` — routing locked→unlock is the orchestrator's job.
- **API door interaction, end to end** — `internal/orchestrators/encounter/v2/interact.go:66` `Interact` loads the encounter, classifies `data.Doors[TargetEntityID]`, routes locked→`AttemptUnlock` (caller-private skill-check prompt) / unlocked→`OpenDoor` (publishes `DoorOpened` + `GeometryRevealed`), persists. Handler maps sentinels to status codes. **Wired.**
- **`Interact` works today** — `Interact(encounterId, targetEntityId, "open")` is live end to end; the harness debug text-input button drives it (web survey). The web `useInteract` hook exists (`src/api/useInteract.ts`).
- **The wire** — `WallKind{SOLID, DOOR_CLOSED, DOOR_OPEN, WINDOW}` (types.proto:82); `Wall{from, to, kind}` (types.proto:152, "a barrier between two adjacent hexes" — `from`/`to` may differ); `Space{hexes, walls, entities, zones}` (types.proto:184, "no rooms on the wire"); `Interact` (service.proto:54,185); `DoorOpened{door_entity_id, revealed_hexes, revealed_walls, removed_walls}` (events.proto:252); `DoorClosed` (events.proto:261); `GeometryRevealed{hexes, walls}` (events.proto:102).
- **Wall geometry + LoS + combat entry** — `SpaceData{Walls []environments.WallSegmentData, Width, Height}` (data.go:80); `WallSegmentData{Start, End spatial.CubeCoordinate, BlocksMovement, BlocksLoS}` (environments/environment_data.go:188) — **segments already carry per-segment blocks-flags**; wall-aware `VisibleHexesAt`/`CanSeeAt`/`truncateAtWall` (space.go); inline combat self-entry on visibility-pair formation; terminal end via `checkEncounterEnd`→`ModeEnded`+`EncounterEndedEvent` (death.go:393).
- **The full multi-room orchestrator — as DEAD CODE** — `tools/spatial/orchestrator.go:58` `RoomOrchestrator` interface, `orchestrator.go:12` `ConnectionTypeDoor`, `connection.go:8` `BasicConnection` with `IsPassable` (connection.go:79), `basic_orchestrator.go:247` `AddConnection`, `:351` `MoveEntityBetweenRooms`, `:403` `CanMoveEntityBetweenRooms`, plus `FindPath`. All tested; **encounter only ever calls `AddRoom` once as a single-room container.** Per ADR-0015 these connections are *abstract* (logical links; position is the game layer's job).
- **Room-aware spawn** — spawn engine (spawn PR #770) has `PositionOracle` + `FixedPositions` + `SpawnConfig.Seed`, single-room-scoped via `GetRoom`; rpg-api `seedGoblins` builds a `PositionOracle` on `perception.CanSeeAt` for out-of-sight placement (status.md:70-74). #760/#764 both **closed**.
- **Web renders doors from wall kind** — `src/components/hex-grid/syntyHexWallHelpers.ts:42` `edgePieceKind(WallKind)` maps `DOOR_CLOSED`/`DOOR_OPEN`/`WINDOW` (44-47) to door GLB pieces; the set-membership wall renderer "extends automatically to DOOR_*" (`src/hooks/dungeonMapGeometry.ts:87`); door state is tracked (`src/hooks/useEncounterState.ts:293`, `DOOR_CLOSED`→`DOOR_OPEN` overwrites the entry). Door-open is a **Y-rotation pose of the same GLB** (needs visual QA); an isolated DOOR cell renders **6 door pairs** (unit-test-proven) — the multiplicity constraint.

**The real gaps (this is the whole wave):**

1. **Doors don't block, and opening reveals nothing.** `OpenDoor` (encounter.go:1013) flips `door.Open` and calls `perception.ProjectDoorOpen`, but never touches `e.room`'s walls — a **closed door blocks neither movement nor LoS today**, and `ProjectDoorOpen` (perception/project.go:135) only re-reveals the door cell's already-visible neighbors (its `door` param is commented *"reserved for future-slice wall logic"*). Walls block but can't open; doors open but don't block or reveal.
2. **There is one room.** Live space = one `environments.QuickRoom` via `enc.InitRoom(20,20,PatternRandom)` (`internal/orchestrators/lobby/start_encounter.go:171`), party at placeholder `roomCenterHex()` (:92), 2 goblins. No multi-chamber generation, no entrance concept.
3. **Unseeded RNG (rpg-toolkit#787).** `RandomSeed` defaults 0 (`wall_patterns.go:129` `rand.NewSource(params.RandomSeed)`; `room_builder.go:174` only seeds when non-zero), so every dungeon is identical.
4. **The client click surface is missing.** The web renders door geometry and tracks state, but has no way to know *which rendered wall is a clickable door with which id*: `EncounterView.tsx:19-22` + `:223-225` document that "HexGrid's door-click surface needs a v2-shaped `DoorInfo[]` the v2 stream doesn't accumulate." The old v1alpha1 `DoorInfo[]`/`HexDoor` path is dead code (never populated).

---

## The crux, resolved first: are doors walls or entities?

The contradiction the design must settle before anything else:

- **The wire says doors are kind-flipped WALLS** — `WallKind.DOOR_*`, "opening changes the kind, not the wall's existence" (types.proto:80). The web renders doors *from wall kind* (syntyHexWallHelpers.ts:42).
- **The toolkit says doors are positioned ENTITIES** — `DoorData{ID, Position, Open, Locked, LockDC…}` keyed by ID on `Data.Doors`, separate from walls (which are degenerate `Start==End` segments with no kind field). `Interact` targets an **entity id**; `DoorOpened.door_entity_id` names one.

**Resolution: the `DoorData` entity is the single source of truth; the wall kind is its projected geometry.** A door is one internal object — `DoorData` (id, position, open/locked, lock params) — that owns interactivity and lock state and is targeted by `Interact(door.ID)`. Its geometry is *derived at projection time* into a DOOR-kind `Wall` for the renderer. This honors both existing shapes without either being a second source of truth: the entity is truth, the wall is a view. `DoorData` (id, position, open, locked) **is** the "v2-shaped DoorInfo" the web asked for — so the web assembles its door-click surface directly from the DOOR-kind walls once those carry the id, with no parallel list to correlate.

**The one bridge this requires: `Wall.id`.** The web reads a `DOOR_*` wall and must learn the door's id to send `Interact(id)`. Walls today are `{from, to, kind}` — no id. So the wave's single load-bearing wire addition is `Wall.id` (`optional string`, additive): the projector copies `DoorData.ID` onto the door's `Wall`; solid walls leave it empty; `DoorOpened.door_entity_id` is that same id. With it, each `DOOR_*` wall self-describes — geometry (kind → GLB pose), passage edge (from/to), and identity (id → `Interact`) — and the web needs no position-correlation and no separate door list.

Alternatives weighed and rejected:

- **A new `Space.doors []Door` list (the literal "DoorInfo[]").** Matches the web's comment wording and the `DoorData` shape 1:1, but it is *more* wire surface (a new message + repeated field) and forces the web to correlate two lists (walls for geometry, doors for identity) by position — for no gain over one `Wall.id`. The DOOR-kind walls already *are* the door list once they carry ids.
- **Doors as `Space.entities` entries.** `entities` are real-time, LOS-filtered, moving objects; doors are sticky geometry with a different lifecycle. Wrong bucket, and it splits the door across entities+walls.
- **Zero wire change via position-matching a hidden door record.** There is no door record on the wire to match against without adding one; "zero change" isn't actually reachable for the *click surface*. `Wall.id` is the smallest honest addition.

Locked state on the wire is a **separate, optional** touch (see §Q2): `WALL_KIND_DOOR_LOCKED` gives a distinct locked visual, but without it a locked door renders as `DOOR_CLOSED` and its locked-ness surfaces as the skill-check prompt on click. So the *required* wire surface of the whole wave is exactly `Wall.id`.

---

## Design question 1 — Dungeon topology

**Recommendation: ONE continuous `Space` / one `spatial.Room` spanning the whole dungeon; "rooms" are wall-partitioned regions within it, joined by DOOR-kind cells. Do NOT adopt the multi-room orchestrator.**

This deviates from the "assembly not invention" default (the orchestrator is right there, tested), so here is the hard argument, because the deviation is load-bearing.

**Weighing the front-runner (multiple `spatial.Room`s + `RoomOrchestrator`/`Connection`s):** it exists and is tested (orchestrator.go:58, basic_orchestrator.go:247/351/403). But it solves a *different* problem: ADR-0015 connections are **abstract** — "logical relationships, not spatial constraints"; `MoveEntityBetweenRooms` removes the entity from the source room and emits a transition event for the *game layer* to place it. That is theater-of-mind navigation between separately-coordinated rooms. A tactical hex dungeon needs the opposite: the party physically stands on a doorway cell, sees *through* it, and walks *across* it in one coordinate system. Adopting the orchestrator would force us to:

1. **Re-add precise positions ADR-0015 deliberately dropped** — the door must be at a specific hex, walkable-through and LoS-gated, which abstract connections don't model.
2. **Invent cross-room LoS ourselves** — the orchestrator has no line of sight across a connection; reveal-through-a-doorway (the whole point) isn't something it provides.
3. **Rewrite the proven wave-1 core** — `checkCombatEntry`/`CanSeeAt`/`truncateAtWall` all close over **one** `e.room`, with no per-entity room membership anywhere (survey seam). Multiple rooms means teaching every one of those about room membership and cross-room queries.
4. **Flatten to one `Space` for the wire anyway** — the wire is *"continuous flat hex map — no rooms on the wire"* (types.proto:179), hexes/walls sticky per character, entities LOS-filtered. N rooms would be collapsed to one `Space` on projection regardless.

So reusing the orchestrator is *more* invention (precise positions + cross-room LoS + a membership rewrite + wire-flattening) than extending the single continuous space, and it buys nothing the north star needs. The very seam the survey flags — "everything closes over one `e.room`" — is the argument *for* staying single-room-as-whole-dungeon: keep one `e.room` that **is** the whole dungeon and none of that machinery changes; walls + door cells gate LoS/movement/combat-entry across regions for free.

**The continuous model still gets a first-class "region" concept** — reuse `Zone` (types.proto:162), already "optional metadata attached to a region of hexes," to label chambers for spawn scoping, entrance anchoring, and (later) ambient/trigger hooks. That is the honest, cheap half of the orchestrator's value without its coordinate-splitting cost. The dead orchestrator is not deleted by this wave; it remains the seam if a future need (procedural mega-dungeons, cross-*encounter* travel) ever wants genuinely separate spaces.

**How many rooms for the first slice: 2 chambers + a boss chamber = 3 regions, 2 doors (one plain, one locked)** — the minimum that proves every north-star word. Layout is a short linear chain for slice 1 (entrance → door → mid → locked door → boss); branching is a later generation concern.

---

## Design question 2 — Doors: data model + blocking + passage edge

Builds on the crux (§above): `DoorData` is truth; it projects to a `Wall{id, from, to, kind=DOOR_*}`.

**Blocking + opening (the core toolkit integration).** Today a closed door doesn't block and opening reveals nothing (gap 1). Fix:

- **Closed doors block via the existing wall machinery.** A closed `DoorData` at its `Position` must present as a blocking cell to `IsLineOfSightBlocked`/`CanPlaceEntity`. Recommended: `rebuildRoomFromData` (which already reconstructs wall entities each load) registers a blocking wall entity at each **closed** door's `Position`, derived from `Data.Doors` — so `DoorData` stays the single source of door truth and `SpaceData.Walls` stays *solid* walls only. (Trivial alternative: generation writes a co-located `WallSegmentData` with `BlocksMovement/BlocksLoS` and `OpenDoor` toggles it — `WallSegmentData` already has those flags, environment_data.go:188. Either works; the derived-from-`Doors` path avoids redundant state.)
- **`OpenDoor` unblocks + reveals** (extend encounter.go:1013): after `door.Open = true`, the door's cell stops blocking (next rebuild, or flag-toggle), **then** re-run reveal. Fix `ProjectDoorOpen` (project.go:135 — its `door` param is already reserved for this): with the cell now transparent, re-run `VisibleHexesAt` and reveal the delta — the next chamber comes into view **progressively through the doorway**, SightRange-capped, blocked by that chamber's own walls. Publish it via `DoorOpened.revealed_hexes`/`revealed_walls` (already on the wire).

**The passage edge (multiplicity constraint #648-comment-4, web-confirmed: an isolated DOOR cell renders 6 door pairs).** The proto `Wall{from, to}` may differ (types.proto:152). Project a door as `Wall{from: doorCell, to: passageNeighbor, kind: DOOR_CLOSED}` — the from→to pair **is** the one designated passage edge, so the renderer draws a single door frame on that edge. Solid walls stay `from==to`. Zero new geometry fields; the existing renderer keys the pose off `kind` (Y-rotation, syntyHexWallHelpers.ts) and the edge off from/to.

**Locked doors reuse the existing skill-check verb.** `Interact(target=door)` → orchestrator reads `DoorData.Locked` → locked returns `InputRequired{skill_check}` (`AttemptUnlock`), the client resolves via `SubmitCheck`, success unlocks-and-opens (all wired: interact.go:92, prompts.go). **No new RPC, no new toolkit verb.** Optional wire polish: `WALL_KIND_DOOR_LOCKED` (additive enum) for a distinct locked visual; without it, a locked door renders as `DOOR_CLOSED` and locked-ness surfaces on click.

**Wire touches (both additive; one required):**

| Touch | Required? | Why |
|---|---|---|
| `Wall.id` (`optional string`) | **required** | door click → `Interact(id)`; the crux bridge |
| `WALL_KIND_DOOR_LOCKED` | optional | distinct locked visual; else locked renders as closed |

Everything else (`Interact`, `DoorOpened`, `DoorClosed`, `GeometryRevealed`, `Space`, `Wall.from/to/kind`) is reused as-is.

**Resolve the latent unread-field case:** the web currently reads only `DoorOpened.door_entity_id`; `revealed_hexes`/`revealed_walls` go unconsumed. This design **consumes them** (they carry the through-doorway reveal), retiring the ambiguity. `removed_walls` is used only when an open action genuinely removes a barrier (rare); populate it only then.

---

## Design question 3 — Traversal + reveal

- **What reveals: progressive LoS through the opening** (§Q2). You see through the doorway, then more as you step in — honest to per-hex sticky reveal + SightRange, and it doesn't spoil the next chamber's monster positions the way whole-room reveal would.
- **Entrance-anchored spawn (Kirk, #648 2026-07-18): the continuous model makes it nearly free.** Chamber 1's spawn anchors to a designated **dungeon-entrance cell** just inside its perimeter, replacing `roomCenterHex()` (start_encounter.go:92, self-documented as a safest-margin placeholder, not a design). Because the party never re-spawns when crossing a door in one continuous space, **Kirk's "room N's spawn anchors to the traversed door" clause becomes moot** — there is no per-room respawn to anchor. Confirm with Kirk (his comment assumed a per-room-transition model). The initial spawn still composes with the wall-aware, verified-placeable, near-the-entrance search flagged in status.md.
- **Party split across rooms: allowed, ungated.** One space → members roam freely; combat entry is per visibility-pair, so it already handles a lone scout opening a door and drawing the next fight. No gate (consistent with `feedback_no_logic_in_web`). Playtest-watch: a solo scout can trigger the boss while the party is a room away — emergent co-op, acceptable for slice 1; flag, don't gate.

---

## Design question 4 — The boss room

**What makes it the boss room mechanically for slice 1, honest to existing systems:**

1. **A locked door gates it** — the north-star "locked door", unlock via the existing `AttemptUnlock`→`SubmitCheck` skill check.
2. **A tougher single monster.** Only `monster.NewGoblin` exists (monster.go:228) — no WarChief. Cheapest honest option: `monster.NewGoblinBoss` following the exact `NewGoblin` pattern (more HP/damage, same attack/move AI, same kit) — stat data, not new mechanics. No boss-specific AI in slice 1.
3. **The boss chamber is terminal; its clear ends the dungeon.** Seed **all** chambers' monsters at StartEncounter, each out-of-sight per its region. Per-chamber fights start as sightlines form (existing combat entry). `checkEncounterEnd` (death.go:393) fires only when the **last hostile anywhere** dies — the boss — so "clear the dungeon" == "kill every seeded monster," and completion is the **existing** terminal transition (`ModeEnded` + `EncounterEndedEvent`), no new end-logic. Killing chamber-1 goblins won't end it while the boss lives.

**Dungeon completion reuses rpg-api#663's `EndEncounter(reason)` seam** — #663 adds an administrative terminal transition with a `reason` taxonomy (`abandoned`) over the same `ModeEnded` path. Victory is the same transition with `reason:"victory"/"completed"`; #663's liveness-refuses-resume then also protects a *completed* dungeon from re-imprisoning players on reconnect. Coordinate so the reason taxonomy covers both.

---

## Design question 5 — Generation (seeded, multi-room, connected)

**Recommendation: multi-chamber generation is a TOOLKIT capability (extend `tools/environments`); rpg-api orchestrates by key (length/layout/theme/seed) and receives geometry back.** The live generator already is the toolkit's (`enc.InitRoom`→`environments.QuickRoom`, start_encounter.go:171) — the correct layer, geometry being the toolkit's per the boundary rule.

Wave 2 extends it to emit a **multi-chamber `SpaceData`**: N chambers at non-overlapping offsets in one coordinate space, wall-partitioned, one DOOR-kind cell per adjacent-chamber pair, a **spanning-tree connectivity guarantee** (every chamber reachable from the entrance through doors), a designated entrance cell in chamber 1, and per-chamber spawn regions (as `Zone`s) for seeding. Each chamber's interior walls reuse the existing `QuickRoom`/`RandomPattern` generator per chamber for variety.

**Absorb rpg-toolkit#787 (unseeded RNG) here.** Thread a real seed into `PatternParams.RandomSeed` (wall_patterns.go:129; room_builder.go:174 only seeds when non-zero): default to entropy, with an **explicit seed param** plumbed for reproducible tests/fixtures (`feedback_devseed_fixture_per_wave`). Audit deterministic-layout tests before flipping the default. StartEncounter passes the seed (entropy in prod, fixed in the wave's named devseed fixture).

**Monster + boss seeding: per-region, out-of-sight, via the existing spawn engine.** The engine now has `PositionOracle` + `FixedPositions` + `SpawnConfig.Seed`, single-room-scoped via `GetRoom` (spawn PR #770). With one `e.room` (the whole dungeon), scope each chamber's seed to that chamber's `Zone` region and keep monsters **out of sight from that region's entrance door** (the same `perception.CanSeeAt` oracle wave-1 uses from the room center — generalized to the door). `FixedPositions` pins the boss; `Seed` gives fixtures determinism. The `rulebooks/dnd5e/dungeon` placement model (`MonsterPlacementData`, `SpawnZoneData` — zero callers today) is the natural home for the per-room-type spawn tables the generator feeds; wave 2 gives it its first real caller.

**Fork — the dormant rpg-api multi-room generator.** rpg-api already contains a rich but **unwired** generator: `internal/components/dungeon/` (`LayoutGenerator`→RoomSlots+Connections+StartRoom+BossRoom, `ShapeGenerator`, `PerimeterUpdater` with door openings, `FeatureGenerator` with spawn zones, `EncounterGenerator` with `IsBossRoom`/`BossPool`/`CRBudget`). Its callers are only within its own package + `spawner/dungeon_adapter.go`; StartEncounter does **not** use it. Geometry generation in rpg-api is a boundary violation (this is the Architecture-Honesty chapter). Recommendation: its *design intent* informs the toolkit generator's API, but geometry-producing code lives in the toolkit; the api component is retired or thinned to an orchestration shell passing settings by key. **Kirk decides its fate** — the biggest layer call of the wave.

---

## Design question 6 — Slice plan

Ordered, each independently mergeable and playtest-verifiable. Toolkit→api→web within a slice. `Wall.id` lands in its own additive rpg-api-protos PR ahead of the api leg that populates it.

**Slice 0 — Seeded room variety (toolkit only). Closes rpg-toolkit#787.** Entropy-default seed + explicit seed param on the room generator; audit deterministic tests first. *Playtest bar:* two consecutive `StartEncounter`s show different wall layouts; the named devseed fixture still reproduces a fixed one.

**Slice 1 — Doors block and reveal (toolkit only).** Closed `DoorData` cell blocks via the wall machinery; `OpenDoor` unblocks and `ProjectDoorOpen` re-runs LoS to reveal through the doorway. Fold in the known gate finding: **NPC-direction event visibility is radius-only and leaks through walls** (#648 body) — make it wall-aware here, or a closed door leaks the next chamber's monster cues. *Playtest bar (toolkit integration test; observable via api in Slice 2):* a closed door blocks LoS+movement; opening it reveals the cells beyond and lets movement through.

**Slice 2 — Two-chamber dungeon, end to end (toolkit→protos→api→web). THE multi-room slice.** Toolkit: generator emits 2 chambers + 1 plain door + entrance cell + per-chamber spawn `Zone`s. Protos: additive `Wall.id`. API: build the 2-chamber space, project the door as `Wall{id, from, to, DOOR_CLOSED}`, entrance-anchored spawn (replace `roomCenterHex()`), seed goblins per chamber out-of-sight. Web: attach a click surface to `DOOR_*` walls (id → `useInteract`), render open/closed pose, consume `DoorOpened.revealed_*` to reveal chamber 2, door-blocked walkability. *Playtest bar:* spawn at chamber-1 entrance, fight the goblins, click the closed door → it opens → chamber 2 reveals through the doorway → walk through → chamber-2 goblins appear (`EntityAppeared`) → combat by rule.

**Slice 3 — Locked boss door + boss chamber + completion (toolkit→protos→api→web).** Toolkit: `monster.NewGoblinBoss`; generator adds a 3rd (boss) chamber behind a locked door; boss-clear rides the existing terminal end. Protos (optional): `WALL_KIND_DOOR_LOCKED`. API: project the locked door, seed the boss out-of-sight, wire completion to #663's `EndEncounter(reason:"victory")` + liveness-refuses-resume. Web: locked-door render + skill-check prompt on click (`InputRequired`→`SubmitCheck`, already wired), victory state. *Playtest bar:* clear chamber 2, reach the locked boss door, click → skill-check → resolve → unlock+open → enter boss chamber → boss appears → defeat it → dungeon completes (resume does not re-imprison).

### The wave's closing playtest script (mouse-only, the done-bar)

1. Party of 4 assembles via the lobby; `StartEncounter` drops them **just inside the chamber-1 entrance** (not center).
2. Chamber-1 goblins come into view on a move → combat by rule → fight to their deaths.
3. Click the closed door → it opens → chamber 2 reveals **through the doorway**.
4. Move through → chamber-2 goblins appear → combat.
5. Clear chamber 2 → reach the **locked** boss door → click → skill-check prompt → resolve → unlock + open.
6. Enter the boss chamber → the boss (tough goblin) appears → fight to victory.
7. Boss dies (last hostile) → **dungeon completes**; reconnecting resumes to an endable state, not an inescapable fight.

No devseed injection, no server restart, mouse-only throughout.

---

## Layer-honesty ledger (every wire touch)

| Touch | Layer | Additive? | Required? | Why |
|---|---|---|---|---|
| `Wall.id` (`optional string`) | protos | yes | **yes** | door addressability for `Interact`/`DoorOpened`; the crux bridge |
| `WALL_KIND_DOOR_LOCKED` | protos | yes | no | distinct locked visual; else locked renders as closed |
| Door blocks/reveals; `OpenDoor` unblocks + `ProjectDoorOpen` re-runs LoS | toolkit | n/a | — | rules/geometry — toolkit-owned |
| Multi-chamber generator + `#787` seed | toolkit | n/a | — | geometry generation — toolkit-owned |
| `monster.NewGoblinBoss` | toolkit | n/a | — | rules content — toolkit-owned |
| Build multi-chamber space; project doors (+id); entrance spawn; per-region seed | api | n/a | — | orchestration by key — no rules |
| Door click surface + open/closed pose + reveal | web | n/a | — | renders truth, sends intent |

Nothing subtractive; no breaking change; no `Space`-shape change beyond `Wall.id` (+ optional enum value).

---

## Forks — awaiting Kirk

1. **Topology.** Recommend one continuous `Space` (regions = wall-partitioned `Zone`s), NOT the multi-room orchestrator (dead code, abstract, LoS-blind — reusing it is more work than extending the proven single room). The biggest call; argued hard in §Q1.
2. **Generation ownership / fate of `internal/components/dungeon`.** Recommend toolkit-owned geometry; retire or thin the dormant api generator. (§Q5)
3. **Door representation.** Recommend `DoorData` entity as truth, projecting to a `Wall{id, from, to, DOOR_kind}` — the entity/wall crux resolution; needs `Wall.id`. Reject a separate `Space.doors[]` list and door-as-`entities`. (§Crux)
4. **Locked-door unlock mechanic.** Recommend the existing skill-check (`AttemptUnlock`→`SubmitCheck`, wired). Reject key-item and room-clear-gate for slice 1. (§Q2/Q4)
5. **Spawn semantics.** Recommend continuous walk-through (entrance-anchored initial spawn only; no per-room respawn) — confirm, since Kirk's #648 comment assumed per-room transitions. (§Q3)
6. **First-slice chamber count + party split.** Recommend 3 regions (2 + boss), 2 doors (1 plain, 1 locked); party split allowed/ungated. Confirm the split is acceptable (lone scout can trigger the boss). (§Q1/Q3)

## Load-bearing digest

- **Wave 2 is mostly assembly.** Door verbs, lock/unlock skill check, api `Interact` routing, wall geometry+LoS, combat entry, terminal end, and the web's door *renderer* all exist. The real gaps: closed doors must *block* and opening must *reveal through the doorway* (both in the toolkit), multi-chamber *generation* (toolkit), and the web *click surface* (needs `Wall.id`).
- **The crux — doors are `DoorData` entities (truth) projecting to DOOR-kind walls (geometry).** One additive field, `Wall.id`, bridges click→`Interact`; that is the wave's only *required* wire change. `WALL_KIND_DOOR_LOCKED` is optional polish.
- **Topology: one continuous `Space`; regions are wall-partitioned `Zone`s.** The multi-room orchestrator is tested dead code solving theater-of-mind navigation — adopting it means precise-positions + cross-room LoS + a `one-e.room` rewrite + wire-flattening, i.e. more invention than extension. Keep it as a future seam, don't build on it now.
- **Passage edge = `Wall.from`/`to`** (already present); resolves the web-proven 6-door-pair multiplicity. The open verb is `Interact` (wired).
- **Seed all chambers up front, out-of-sight** (spawn engine's `PositionOracle`/`FixedPositions`/`Seed`). Per-chamber combat entry and dungeon completion fall out of existing machinery; completion shares #663's terminal `EndEncounter(reason)` seam.
- **Generation belongs in the toolkit** (absorb #787's `RandomSeed`); rpg-api's rich dungeon generator is dormant and geometry-in-api is the boundary lie this chapter retires.
