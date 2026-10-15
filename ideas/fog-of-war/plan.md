# Fog of war — implementation plan

Design: [design.md](design.md). Walkthrough: [README.md](README.md).
Issue: [rpg-project#586](https://github.com/KirkDiggler/rpg-project/issues/586).

## Header

**Goal and scope.** The session play view draws, per seat, the hexes its
character sees now (lit), the hexes it remembers (grey, with remembered props,
doors and creatures), and nothing else. Discovery moves from whole rooms to
single cells.

**Non-goals.** Authored light levels and region lighting (encounter-studio R19).
Stealth, hearing and passive Perception changes. The sites layer. More than one
screen per seat. The legacy v1alpha2 encounter route and its
`HexKnowledgeChanged` stream, which nothing here builds on. Pooling remembered
under discovery sharing (design O1). R8 (default off) is Task 7, its own slice after this wave (rpg-project#595): the choice lives in the session's character profile, because the persisted bool's zero value means shared and a web-only flip reset an opted-in player on reload.

**Authority.** design.md R1–R5 and R8, operator rulings of 2026-10-15 on rpg-project#586.

**Constraints.**
- One sight test: every seen-now and learning answer comes from
  `sightReach.reachesCell` (design Law, R5).
- Seen-now is derived on read, never persisted (design Law).
- Wire changes are additive; `ROOM_REVEALED` is deprecated, not removed
  (rpg-api-protos instructions: deprecate, do not break).
- rpg-toolkit: one nearest-`go.mod` module per PR (`rulebooks/dnd5e/encounter`
  and `rulebooks/dnd5e/session` are separate modules).
- Outside-in development on pseudo-versions, inside-out merge
  (rpg-project CLAUDE.md "How a wave is shaped"). API and web PRs stay draft
  while they pin pseudo-versions.
- Studio walk ports are fixed: web 3030, api 8115.

**Inspected baseline.**

| Repo | Ref | Commit |
|---|---|---|
| rpg-toolkit | origin/main | d7d88c42 |
| rpg-api | origin/dev | 653555d8 |
| rpg-api-protos | origin/main | 7e282c4 |
| rpg-dnd5e-web | origin/dev | ea8ae8f6 |

**Sequence.**

```
develop (pseudo-versions): T1 → T2 → T3 → T4 → T5 → T6 walk
merge (inside-out):        T3 protos (may merge first, publishes bindings)
                           T1 encounter tag → T2 session pins tag, tag
                           → T4 api pins released session + protos → T5 web pins protos
re-verify:                 T6 re-walk on released pins before T4/T5 leave draft
```

T3 can be written in parallel with T1 because its schema is fixed by this plan.
T2 needs T1's interfaces; T4 needs T2 and T3; T5 needs T3's bindings and a T4
server to walk against.

**Open items.**
- Resolved assumption: the event kind and oneof field numbers are the next free
  values at the protos head when T3 opens; other waves are adding kinds today.
- Resolved assumption: seen-now reaches the client through `GetView`, refreshed
  by the events that already schedule a `view` refresh plus the new
  cells-revealed event (T5).
- Deferred by the design: O1 pooling, O2 tint.
- No blocker.

## Task 1: Cell discovery, seen-now and per-cell atlas (rpg-toolkit encounter)

**Delivers:** R1, R2, R5 in the engine. A member learns individual cells; the
atlas withholds unknown cells; `VisibleCellsFor` answers seen-now.

**Owner:** rpg-toolkit, module `rulebooks/dnd5e/encounter`.

**Prerequisites:** none.

**Files:**
- Modify `roomknowledge.go`: replace `roomKnownPrefix`, `knownRooms`,
  `discoverRooms`, `learnRoom`, `discoverOpenedRooms` with cell learning; change
  `undiscoveredFrom` to test known cells instead of known regions.
- Modify `roomreveal.go`: replace `appendRoomRevealedBeat` and
  `BeatRoomRevealed` with `appendCellsRevealedBeat` and `BeatCellsRevealed`.
- Modify `sightreach.go` and `encounter.go`: extract the `sightReach`
  construction in `rebuildPercepts` into one helper both the pass and
  `VisibleCellsFor` call.
- Create `visiblecells.go` with `VisibleCellsFor`.
- Modify the journal-fact load validation that `TestInvalidRoomKnowledgeIsRejectedOnLoad` exercises.
- Modify `projection.go` doc comment on `AtlasFor` (per-cell discovery).
- Modify `doc.go` where it describes room discovery.
- Tests: rewrite `roomknowledge_test.go` as cell knowledge; create
  `visiblecells_test.go`; update `structural_reveal_test.go`,
  `conceal_test.go`, `audience_internal_test.go`, `data_test.go`, and
  `dungeonspec/door_threshold_floor_test.go`, `dungeonspec/prop_presentation_test.go`
  where they assert whole-room atlases.

**Interfaces:**
- Fact kind `known:cells`. Actor, Subject and Audience are the member alone.
  Payload JSON `{"cells":[{"x":int,"y":int},…]}`, sorted by `cellBefore`, only
  cells not already known. Folded by union into `knownCells(member)
  map[spatial.Position]bool`.
- Old `known:room:<id>` facts load without error and contribute nothing.
- `func (e *Encounter) VisibleCellsFor(in *VisibleCellsInput) (*VisibleCellsOutput, error)`
  with `VisibleCellsInput{Member MemberID}` and
  `VisibleCellsOutput{Cells []spatial.Position}`. Cells are sorted by
  `cellBefore`, are a subset of `AtlasFor(member).Cells`, and exclude cells
  `hiddenFrom(member)` conceals. `ErrNotMember` for an unknown member;
  an unplaced member answers an empty, non-nil slice.
- Beat `cells_revealed`, one per member per pass that learned cells, never
  before scene-opened. Payload keys: `beat`, `regions` (each
  `{id, archetype…, cells}` carrying only newly known cells of that region),
  `scenery`, `props`, `boundaries`, `segments`, `sealed`, `doorways`, `placed`,
  `exits`, plus `addStructuralReveal` and `addPropPresentationReveal` keys,
  all as differences of `AtlasFor` before and after.

**Behavior:**
- Sight pass: for each observer, every field cell that is not known and
  answers `reachesCell` true is learned in one fact, concealed cells included:
  concealment keeps withholding a concealed cell from the atlas and from
  seen-now (both start from `hiddenFrom`), and the `known:cells` fact is per
  member and never on the wire, so a known concealed cell surfaces nothing.
  (Corrected 2026-10-15 on rpg-toolkit#2056: skipping concealed cells hid
  unlisted props on secret floor, against the concealment law.)
- Opening a door teaches the actor that door's edge cells (both sides); a
  world-side open with no actor teaches nothing. The sight pass after the open
  teaches what is seen.
- Explicit concealment reveal keeps teaching its cells as today.
- `undiscoveredFrom`: a cell is unexplored when it is not known and not taught
  by a concealment reveal. Scenery bordering known floor and wall footing stay
  presented by the existing rules, applied per known cell.
- A door is withheld when none of its edge cells is known; presented otherwise.
- No synthesized boundary for unexplored cells (existing pass 3 already limits
  synthesis to concealment).

**Tests:**
- `TestSightLearnsOnlyReachedCells`: corridor-and-room fixture, observer reach
  covering three of six room cells through an archway. `AtlasFor(a).Cells`
  contains the corridor and exactly those three; the other three are absent;
  their region entry carries only the three.
- `TestRememberedCellsSurviveLossOfSightAndReload`: after moving away, the
  three cells stay in `AtlasFor`; `VisibleCellsFor` no longer contains them;
  JSON reload yields the same atlas.
- `TestVisibleCellsIsPerMemberAndSubsetOfAtlas`: two members in different
  rooms; each answer contains only its own reached cells; each is a subset of
  its own atlas; neither contains the other's.
- `TestVisibleCellsExcludesSightAreaAndConcealment`: a sight area across the
  archway removes the far cells from seen-now but not from the atlas; a
  concealed cell never appears.
- `TestVisibleCellsUnplacedAndUnknownMember`: unplaced answers empty non-nil;
  unknown member answers `ErrNotMember`.
- `TestOpeningTeachesOnlyTheDoorEdge`: reach 0 opener learns the two edge
  cells and nothing of the far room (replaces
  `TestOpeningTeachesLayoutEvenWithNoSightBeyondTheDoor`).
- `TestUnseenDoorIsAbsentAndSeenDoorHasASighting`: before any edge cell is
  known the door is absent from `AtlasFor` doorways, structural doors and
  `DoorSightings`; after the pass that learns an edge cell by sight it is in
  both `AtlasFor` and `DoorSightings`.
- `TestUnexploredCellsSynthesizeNoWall`: the boundary between known and unknown
  ordinary floor carries no synthesized `AtlasBoundary`.
- `TestCellsRevealedBeatIsOnePerPassAndRecipientScoped`: one beat per learning
  observer, audience that observer, regions partial, nothing for an unchanged
  recheck (keeps the `TestLayoutDiscoveryDoesNotRepeatOrAlias` assertion).
- `TestInvalidCellKnowledgeIsRejectedOnLoad`: a listed cell off the field, a
  widened audience, a foreign subject or actor each fail `LoadEncounter`; an
  old `known:room` fact loads and teaches nothing.
- Goldens: `dungeonspec/testdata/reference-tomb.atlas.json` is the author's
  full `Atlas()` and does not change; a diff there is a bug. Every changed
  expectation is an `AtlasFor` or reveal-payload assertion, because those now
  carry the reached cells instead of the whole room.

- [ ] Write the tests above; confirm the per-cell ones fail on main.
- [ ] Implement cell learning, `VisibleCellsFor`, the beat and load validation.
- [ ] Update the listed tests that pinned whole rooms, each to the cells its fixture reaches.
- [ ] Update `doc.go` and the `AtlasFor` comment.

**Verification:** from the T1 worktree,
`go test ./rulebooks/dnd5e/encounter/...` passes; `make pre-commit` passes.
The golden test passes unchanged.

**Completion evidence:** commit, test output, a tagged encounter version after
merge.

## Task 2: Seat projection of seen-now and cells-revealed (rpg-toolkit session)

**Delivers:** R1, R5 at the session SDK.

**Owner:** rpg-toolkit, module `rulebooks/dnd5e/session`.

**Prerequisites:** T1 (pseudo-version during development, released tag to merge).

**Files:** modify `knowledge.go` (`ViewOutput`), `read.go` (`projectView`),
`events.go` (kind mapping and decode arms); create `cells_revealed.go`;
modify `room_revealed.go` to mark `EventRoomRevealed` deprecated; modify
`doc.go`. Tests: `events_internal_test.go`, `structural_patch_internal_test.go`,
`structural_session_test.go`, `prop_presentation_internal_test.go`, a new
`visible_cells_test.go`.

**Interfaces:**
- `ViewOutput.VisibleCells []spatial.Position` with JSON `visible_cells`,
  always non-nil, copied from `encounter.VisibleCellsOutput.Cells`.
- `const EventCellsRevealed EventKind = "cells_revealed"` mapped from
  `encounter.BeatCellsRevealed`.
- `CellsRevealedBody` with the fields of `RoomRevealedBody` except `Region`,
  replaced by `Regions []AtlasRegion` (partial regions, merge by union).
  Structural rows and prop presentations decode through the shared helpers;
  a row missing identity refuses the whole body.

**Behavior:** `Manager.Knowledge` carries `View.VisibleCells` through
`projectView` unchanged. `Manager.View` does the same. No filtering, union or
computation in the session.

**Tests:**
- `TestViewCarriesVisibleCellsForTheCallerOnly`: two seats, `View` for each
  returns its own encounter answer exactly.
- `TestKnowledgeViewAndAtlasAgree`: every `VisibleCells` entry is in
  `Knowledge.Atlas.Cells`.
- `TestCellsRevealedDecodes`: a beat with two partial regions decodes to two
  `Regions`; a malformed structural row yields no body.
- `TestCellsRevealedPatchEqualsFreshAtlas`: applying the event to the prior
  atlas equals a fresh `Knowledge.Atlas` (extends the structural patch test).

- [ ] Tests, implementation, docs, `make pre-commit`.

**Verification:** `go test ./rulebooks/dnd5e/session/...`; `make pre-commit`.

**Completion evidence:** commit, test output, session tag after merge.

## Task 3: Wire contract (rpg-api-protos)

**Delivers:** the additive wire for R1, R2, R5.

**Owner:** rpg-api-protos, `dnd5e/api/session/v1alpha1`.

**Prerequisites:** none for authoring; schema fixed here.

**Files:** `service.proto`, `events.proto`.

**Interfaces:**
- `GetViewResponse`: `repeated Position visible_cells = 5;` Doc: the cells this
  authenticated member sees now, a subset of its atlas cells, sorted as
  `AtlasRegion.cells`; empty means it sees nothing now; never a party union.
- `GetAtlasResponse` doc rewritten: fixed data for the cells this member has
  learned; unknown cells and everything touching only them are absent; full
  room geometry is not supplied behind obstructions; the concealment absence
  law is unchanged. Remove the text stating that client fog does not hide
  unexplored space and that full room geometry is supplied.
- `EventKind`: `EVENT_KIND_CELLS_REVEALED = <next free>;`
- `Event.body` oneof: `CellsRevealed cells_revealed = <next free>;`
- `message CellsRevealed`: `repeated AtlasRegion regions = 1` (partial; merge
  cells by region id with union), then `props`, `boundaries`, `segments`,
  `sealed`, `doorways`, `placed`, `exits`, `scenery`, `structural_walls`,
  `structural_doors`, `structural_wall_openings_replacements`,
  `prop_presentations` with the same types and doc semantics as
  `RegionRevealed`.
- `EVENT_KIND_ROOM_REVEALED` and the `room_revealed` oneof field gain
  deprecation comments (no longer produced). Use `[deprecated = true]` on the
  field.

**Behavior:** not applicable; contract only.

**Tests:** no hand-written tests in protos. Evidence is `make test` (lint,
format-check, proto-options, breaking) and `make generate` completing.

- [ ] Edit, `make format`, `make test`, `make generate`.

**Verification:** `make test` passes with no breaking change reported.

**Completion evidence:** PR, CI green, published bindings after merge.

## Task 4: Convert and pin (rpg-api)

**Contract added 2026-10-15 (lead):** `GetViewResponse.visible_cells` is never empty for a seated member: a seat that sees nothing still receives its own cell, so the web may treat an EMPTY list as "this server does not send visible cells" and draw every known cell lit (the pre-fog look). proto3 cannot tell absent from empty; this rule is what lets rpg-dnd5e-web#1287 sit on dev ahead of Tasks 1, 2 and 4 without greying the board.


**Delivers:** R1, R5 on the wire for each caller's seat.

**Owner:** rpg-api, `internal/handlers/dnd5e/session/v1alpha1`.

**Prerequisites:** T2 and T3 (pseudo-versions to develop; released tags to merge).

**Files:** `get_view.go`, `get_knowledge.go`, `convert.go` (view conversion and
the event arm for `sdk.CellsRevealedBody`), `convert_structural.go` and
`convert_prop_presentation.go` if the shared row converters need a new caller;
`go.mod`. Tests: `get_view_test.go`, `get_knowledge_test.go`, `convert_test.go`,
`structural_convert_test.go`, `stream_events_test.go`.

**Interfaces:** consumes `ViewOutput.VisibleCells` and `CellsRevealedBody`;
produces `GetViewResponse.visible_cells` and `Event.cells_revealed`.

**Behavior:** every read stays bound by `callerActingAs`. Conversion copies
cells; it never derives them.

**Tests:**
- `TestGetViewCarriesVisibleCells`: fake manager returns three cells; the
  response carries the same three in order.
- `TestGetViewRefusesAnotherMembersSeat`: the caller naming a member it does
  not control gets `PermissionDenied` and no cells.
- `TestConvertCellsRevealed`: two partial regions and a structural wall row
  convert field for field.
- `TestStreamCellsRevealedReachesOnlyItsRecipient`: a recipient-scoped beat
  appears on that recipient's stream only.

- [ ] Pin, tests, conversion, `make pre-commit`.

**Verification:** `make pre-commit`; the integration walk in T6.

**Completion evidence:** draft PR while pinned to pseudo-versions; ready after
the released pins and the T6 re-walk.

## Task 5: Draw three states (rpg-dnd5e-web)

**Delivers:** R2, R3, R5 on screen.

**Owner:** rpg-dnd5e-web, `src/components/session` and `src/api`.

**Prerequisites:** T3 bindings; a T4 server for the walk.

**Files:**
- Create `src/components/session/cellKnowledge.ts`: a pure function from atlas
  cells and `visible_cells` to `{ state: Map<key, 'seen' | 'remembered'>, edge: Map<key, number> }`.
- Modify `src/components/session/applyReveal.ts`: add `applyCellsRevealed`
  (union region cells by id; merge the other records as `applyRegionRevealed`
  does).
- Modify `src/api/useSessionKnowledge.ts`: apply `cellsRevealed` events; keep
  the `roomRevealed` arm for a server that still sends it.
- Modify `src/components/session/sessionRefreshKeys.ts`: `cellsRevealed`
  schedules `view` and `roster`, as `roomRevealed` does. The member's own
  `moved` schedules `where`, `afford` and `turn` today but not `view`; it gains
  `view`, because the mover's own seen-now changes on every step.
- Modify `src/components/session/atlasToScene3D.ts` (`buildScene3D`) and
  `DungeonEnvironment.tsx`: floor per cell by state, composing the edge weight
  with `floorExposureByCell`.
- Modify `src/components/session/AtlasWalls.tsx`: pass remembered sets so
  walls and doors bordering only remembered cells use the remembered
  treatment.
- Modify `src/components/session/PropPresentationEnvironment.tsx`: remove the
  `prop-door-unknown-*` wireframe branch; a presented door without an observed
  state draws closed in the remembered treatment.
- Modify `src/components/session/SessionEncounterView.tsx`: build
  `cellKnowledge` and hand it to the scene; fixed props on remembered cells
  join the remembered set.
- `src/components/hex-grid/sceneKnowledge.ts`: reuse the crypt-memory
  materials as the remembered treatment; the tint is one constant (O2).
- Tests beside each file.

**Interfaces:** consumes `GetViewResponse.visibleCells`,
`Event.body.case === 'cellsRevealed'`, the atlas as today.

**Behavior:**
- A cell in `visibleCells` is `seen`; an atlas cell not in it is `remembered`;
  any other cell is never drawn.
- Edge weight: a remembered cell adjacent to a seen cell gets an intermediate
  brightness, one step further gets a smaller one, beyond two steps none. The
  gradient is assigned only to atlas cells.
- Walls and doors take the brightest state of the cells they border.
- Live creatures come from sightings as today; remembered sightings draw with
  `knowledgeState='remembered'`.
- No line-of-sight call, raycast or polygon in this code.

**Tests:**
- `cellKnowledge.test.ts`: seen, remembered and absent for a three-cell line;
  edge weights at distance 1 and 2 and zero at 3; an absent neighbour receives
  no weight; empty `visibleCells` makes every atlas cell remembered.
- `applyReveal.test.ts`: `applyCellsRevealed` twice on the same region unions
  cells; replay is idempotent.
- `useSessionKnowledge.test.ts`: a `cellsRevealed` event patches the atlas and
  advances `seq`.
- `sessionRefreshKeys.test.ts`: `cellsRevealed` returns `view` and `roster`;
  the member's own `moved` includes `view`; another member's `moved` still
  returns `view` alone.
- `PropPresentationEnvironment.test.tsx`: no mesh named `prop-door-unknown-*`
  for a door without state; that door renders as a remembered closed door.
- `atlasToScene3D.test.ts`: remembered cells carry the remembered floor flag.

- [ ] Tests, implementation, `npm run ci-check`.

**Verification:** `npm run ci-check`; screenshot check with
`node tools/browser/screenshot.mjs` from the game-dev root during T6.

**Completion evidence:** draft PR pinned to the T3 bindings; walk evidence from
T6.

## Task 6: The castle walk

**Delivers:** acceptance for R1–R5 end to end.

**Owner:** the walk agent, game-dev workspace infrastructure; no code.

**Prerequisites:** T1–T5 heads on pseudo-versions, built in a detached walk
worktree at each PR head, served on web 3030 and api 8115.

**Scenarios (each one click or move per seam):**
1. **Corridor.** Stand in the castle cell-block corridor. Only the hexes your
   character reaches draw; the rest of the castle draws nothing.
2. **Archway slice.** Look through the archway into the prison cell. Only the
   framed hexes appear; the hexes behind the jamb do not.
3. **Grey after leaving.** Walk out of line of the cell. Its hexes, walls and
   cot stay as a grey shadow; the jailer stays as a remembered figure.
4. **Corner.** Turn the corner onto the guard. The guard's hexes and the guard
   draw lit on the same step; no wedge or cone appears.
5. **Unseen door.** Before any of its edge hexes are seen, a door draws
   nothing, the same as the wall around it; first sight shows it as it is.
6. **Second seat.** A second browser seated as another character in the
   stairwell sees none of the cell block, and its own lit hexes differ.

**Verification:** a screenshot per scenario, posted with a `- [ ]` checklist on
rpg-project#586 before merge; re-walk after pins are released.

**Completion evidence:** the checked list on the issue.

## Task 7: Sharing is a choice (rpg-toolkit session) — R8

**Delivers:** R8. A character who has never chosen sees privately; a party pools
by choosing to; the stored preference tells "never chosen" from "chose shared"
so a later default change moves only the undecided.

**Owner:** rpg-toolkit, module `rulebooks/dnd5e/session`. The encounter module,
the wire and the web do not change: `KnowledgeOutput.DiscoverySharing` already
carries the resolved value, the web checkbox already reads it and writes only on
a click, and `SetDiscoverySharingRequest.sharing` is already explicit.

**Prerequisites:** Tasks 1–6 shipped (session v0.137.0).

**Files:** modify `exploration.go` (`ExplorationData`, `prepareExploration`,
`saveExploration`, `SetDiscoverySharing`), `write.go` (`joinInputFor`),
`doc.go` (the discovery paragraph). Tests: `exploration_test.go`,
`exploration_review_test.go`.

**Interfaces:**
- `ExplorationData` gains `DiscoverySharing *bool` (JSON `discovery_sharing`,
  omitted when nil). Nil means the character has not chosen. `PrivateDiscoveries`
  (JSON `private`) stays as the legacy field, read only when `DiscoverySharing`
  is nil, so an old record keeps its meaning.
- Resolution, one function used by `prepareExploration` and `joinInputFor`:
  a record with `DiscoverySharing` set → that value; a record without it →
  `!PrivateDiscoveries` (legacy: absent meant shared); no record (`ErrNotFound`)
  → private. The resolved bool is what `RestoreDiscoveryInput.Private` and
  `JoinInput.PrivateDiscoveries` receive; the profile itself is never mutated by
  resolution, so a default is never written down as a choice.
- Where "no record" lives: `prepareExploration` is the only place that sees
  `ErrNotFound`, and it stages a not-found character as a bare
  `{Character: id}`, which is indistinguishable from a legacy shared record.
  So `prepareExploration` resolves once and writes the result into a new
  `writeScope.explorationSharing map[string]bool`; `joinInputFor` reads that
  map (a missing entry is private) instead of resolving from the staged copy.
- `ExplorationData` now holds a pointer, so `cloneExploration` copies the
  pointee and the changed-check in `saveExploration` compares values
  (`DiscoverySharing` nil-ness and pointee), never addresses.
- `SetDiscoverySharing` is the only writer: it sets `DiscoverySharing` to the
  explicit value (and `PrivateDiscoveries` to its complement, so the two fields
  never disagree) on the staged profile before `commit`. `saveExploration` no
  longer derives the preference from the encounter; it saves a staged profile
  only when it differs from the loaded one. Choosing the value the default
  already resolves to is still a choice and is saved.

**Tests (names and the assertion that fails on the old code):**
- `TestUnchosenSharingResolvesPrivate`: a character with no profile joins;
  `Knowledge` reports `DiscoverySharing` false; after the run, the repository
  holds no record for that character (resolution wrote nothing).
- `TestLegacyRecordWithoutPrivateStaysShared`: repository seeded with
  `{"character":"alice"}` → `DiscoverySharing` true; seeded with
  `{"character":"alice","private":true}` → false.
- `TestChoiceIsSavedAndSurvivesReload`: `SetDiscoverySharing` true on an
  unchosen character saves `{"character":…,"discovery_sharing":true}`; a new
  manager over the same repository and the same saved encounter resolves
  shared; setting false saves `discovery_sharing:false, private:true` and
  resolves private.
- `TestChoosingTheDefaultIsStillAChoice`: `SetDiscoverySharing` false on an
  unchosen character writes a record with `discovery_sharing:false`.
- Existing tests that assert `profiles.data["alice"].PrivateDiscoveries` after a
  set move to the new field; existing legacy-JSON tests keep passing.
- Existing tests that lean on the shared default: `conceal_test.go`
  `TestAJoinIntoHiddenSpaceStaysHidden` asserts alice and bob hear david's find
  check because sharing was on by default; it is about the join staying hidden.
  David's find check rolls inside `Join` itself, before he could choose, so the
  test seeds his profile with `DiscoverySharing` true before the join; the
  conceal suite keeps a handle to its fake explorations repository (for example
  `s.explorations`) so a scene can seed it. The assertion stands. The `walkToFind` comment stops claiming a shared default.
  Any other test that goes red for the same reason gets the same treatment:
  seed or choose shared where sharing is the subject, flip the assertion where
  the default is.

**Verification:** `make test` and `make lint` in `rulebooks/dnd5e/session`;
rpg-api draft PR pinning the session pseudo-version with
`internal/orchestrators/session/exploration_test.go` round-tripping
`DiscoverySharing`; walk on the castle stack: a fresh character's "Share
discoveries with party" box is unchecked, stays checked across a reload after
one click, stays unchecked across a reload after a second click, and a redis
record `character-exploration:v1:<id>` seeded as `{"character":"<id>"}` BEFORE
that character joins shows checked (the encounter's persisted preference is
restored only at prepare, never on a knowledge read).

**Completion evidence:** the walk checklist on rpg-project#595; tag
`rulebooks/dnd5e/session/vNEXT`; rpg-api dev re-pinned.

**Cost named:** a member of an already-saved encounter whose profile has no
record and who was shared by absence becomes private at the next prepare. Old
records that exist only because they carry legacy `checks` stay shared, as
their absence of `private` always meant. R8 accepts both; nothing else moves.

## Task 8: A wall is known where sight reached its footprint (R9)

**Delivers:** R9 end to end. A presented wall no longer foots the atlas; the
wall record carries the hexes its blocker footprint overlaps; the web grades
each stretch by those hexes. The west wall of the cell block, seen only at its
south corner, draws only its corner piece (rpg-project#600).

**Owners, in build order (outside-in on pseudo-versions, inside-out merge):**

**T8a — rpg-api-protos** (DONE: rpg-api-protos#395 → v0.1.238)
`dnd5e/api/session/v1alpha1/types.proto`:
`AtlasStructuralWall` gains `repeated Position footprint_cells = 9;`. Doc: the
floor cells this wall's blocker footprint overlaps by positive area, in order
along the wall from `from` to `to`; the client grades the wall's stretches by
the knowledge of these cells and never re-derives them from the line. Additive;
`make test` breaking check.

**T8b — rpg-toolkit `rulebooks/dnd5e/encounter`:**
- `projection.go`: delete presentation footing, both the legacy-segment loop
  (`seg.Footprint` → footing) and the structural-wall loop (`TraceFootprint` at
  cell centres). `footing` ceases to exist; the output loop's `hidden && !footing`
  becomes `hidden`. The presentation rule itself (a boundary is presented when
  any support cell is known, `footprintObservationCells`) does not change.
- `AtlasStructuralWall` (the atlas record the session converts) gains
  `FootprintCells []spatial.Position`: the SAME answer that decides
  presentation, `footprintObservationCells(p.Placement)` (positive-area
  coverage), so a presented wall never carries an empty list; sorted by
  projection of each cell centre onto the wall line (`from` → `to`), ties by
  the existing position ordering. Every cell, known or not: the record is the
  wall's geometry, the recipient's knowledge is in `Atlas.Cells`.
- The masquerade covers (R9, second law): in `seesCell`, for an observer who
  has not found a bound door, the door set in a structural wall's opening
  counts as that wall's face and teaches the threshold hex as a span does.
  `AtlasFor` presents a learned threshold under a still-withheld bound door as
  sealed scenery nobody owns, so the non-knower's atlas equals the solid-wall
  twin's. Without this, dropping presentation footing leaves a hole at every
  unfound secret door (`TestKnownWallKeepsFloorUnderAClosedDoor/secret=true`
  fails). Concealed footing under a wall stays withheld (design O4).
- `AtlasFor` predicate, exactly: a cell enters `Cells` when the member knows
  it (journal `known:cells`) and is not withheld by concealment; a learned
  threshold hex under a still-withheld bound door enters `Cells` and `Sealed`
  only, as scenery nobody owns. Concealed footing stays withheld as on
  origin/main (design O4). The journal stores cells, not how they were learned.
- Tests: `footing_test.go` `TestA11_APresentedWallStandsOnFloorTheWatcherCanSee`
  becomes `TestA11_APresentedWallEqualsItsSceneryTwin`: a twin comparison, the
  non-knower's `AtlasFor` equals the scenery twin's `AtlasFor` on `Cells` and
  `Sealed`, never the twin's full `Atlas()`. The fixture must use a structural
  wall with placed blocking spans (the hugging layout authored as a structural
  wall), not the legacy `hugSeam` segment, which has no placed face for
  `screensOf` to fire on; `TestA11_AWithheldWallDoesNotFoot`
  stays. New `TestStructuralWallCarriesItsFootprintCells` (an edge wall lists the
  hex on either side; a centre-line wall lists its footing hexes and the
  straddled neighbours; order follows the line). New
  `TestSightLearnsAWallsFootingByItsFace` (a member who reaches the floor beside
  a wall span learns the footing hex behind that face and no other footing hex;
  this is the existing `seesCell` cover rule, pinned for walls). New
  `TestAnUnfoundSecretDoorTeachesItsThresholdAsTheWallWould` (non-knower in the
  corridor learns the threshold hex under an unfound secret door exactly as
  under its twin; the knower's atlas unchanged). UNCHANGED and passing (byte-identical):
  `dungeonspec/door_threshold_floor_test.go` `TestKnownWallKeepsFloorUnderAClosedDoor`
  (both arms; it is the twin gate), `dungeonspec/single_room_discovery_test.go`
  `TestOrdinaryDiscoveryOpaqueFootingIsVisibleFromBothSides` (a concealment
  that selects only the footing hex stays withheld), the explicit
  concealment tests. The A11 twin fixture uses an ordinary unexplored room,
  not a concealed one. Updated to the new rule: `revealsegments_test.go`
  `TestSealedIsAReplacementWithinTheRoomAndNotAnAddition` and the
  `cells_revealed`/`sealed` assertions: footing appears where sight learned
  it, nowhere else. The full-atlas golden (author's view) must not
  change except for the new `FootprintCells` field.

**T8c — rpg-toolkit `rulebooks/dnd5e/session`:** the session has its own wall
type and copies the record field by field, and the same record rides the
reveal bodies: add `FootprintCells` to the session wall type, the atlas
conversion and the `cells_revealed`/structural reveal bodies. Test:
`convert_internal_test.go` round-trips the field; a reveal-body test carries it.

**T8d — rpg-api** `internal/handlers/dnd5e/session/v1alpha1/convert_structural.go`:
map `FootprintCells` → `footprint_cells`. Test: the structural wall convert test
asserts the positions and order. Pin T8b/T8c pseudo-versions, draft until tags.

**T8e — rpg-dnd5e-web** `src/components/session/atlasToScene3D.ts`,
`cellBorders.ts`:
- Structural walls: replace `stretchesBesideLine(line…)` with
  `stretchesAlongCells(line, footprintCells, stateOf)`: each footprint cell is a
  sample at its centre's projection onto the line (t clamped 0–1); samples are
  grouped by t (cells straddling an edge share a t, within 1e-6 of the length)
  and a group takes the brightest state of its cells; groups sorted by t;
  stretches split midway between groups whose state differs, exactly as
  `stretchesBesideLine` splits today. `footprint_cells` reaches the render unit
  through the atlas → `StructuralWallSurface` conversion (`structuralLayout.ts`),
  cells keyed as atlas cells are keyed today. A wall with no `footprint_cells`
  (older server) keeps today's side sampling, so dev ahead of the api does not
  go blank.
- Ordinary hex-edge wall runs and structural doors keep side sampling (an
  edge run's two side cells are exact).
- Tests in `atlasToScene3D.test.ts`: "a centre-line wall known at one footing
  hex draws only that stretch" (the replay case: a 23-sample vertical wall with
  footing hexes on odd rows, only the south corner's hexes known → one stretch
  at the corner, nothing else); "a wall whose footprint straddles an edge takes
  the brighter side"; "a wall without footprint_cells grades as before".
  `cellBorders.test.ts` (new) pins the projection and the split rule.

**Walk (T8f, castle stack :3030/:8115, api at T8d head, web at T8e head):**
1. Enter the cell block from the north door; stand so the west wall's south
   corner is in sight and the room interior is not: the west wall draws only
   its corner piece, no stubs, no floor beside the stubs.
2. Walk into the room: the wall fills in stretch by stretch as footing hexes
   are seen; the north wall with the door stays whole.
3. Replay script from the investigation re-run against the new atlas: zero
   absent-between-known stretches on the west wall.
Checklist on rpg-project#600.

**Cost named:** a member who learned a room by sight before this change keeps
its journal; the sealed footing they were shown disappears from the atlas
until sight reaches it. A closed door's threshold floor draws when its door is
seen (cover rule), not before.

## Requirement coverage

| In-scope requirement / acceptance scenario | Implementing task(s) | Concrete proof |
|---|---|---|
| R1 per-seat, per-hex live sight | T1, T2, T4, T5 | `TestVisibleCellsIsPerMemberAndSubsetOfAtlas`, `TestViewCarriesVisibleCellsForTheCallerOnly`, `TestGetViewRefusesAnotherMembersSeat`, walk 1, 6 |
| R1 no whole-region reveal | T1 | `TestSightLearnsOnlyReachedCells`, `TestOpeningTeachesOnlyTheDoorEdge`, walk 2 |
| R2 never-seen draws nothing | T1, T5 | `TestUnseenDoorIsAbsentAndSeenDoorHasASighting`, `cellKnowledge.test.ts`, walk 5 |
| R2 first sight shows the door as it is | T1, T5 | same test's sighting half; `PropPresentationEnvironment.test.tsx` |
| R3 remembered grey with props and creatures as last seen | T1, T5 | `TestRememberedCellsSurviveLossOfSightAndReload`, `atlasToScene3D.test.ts`, walk 3 |
| R4 memory per seat | T1 | `known:cells` audience in `TestInvalidCellKnowledgeIsRejectedOnLoad`, `TestCellsRevealedBeatIsOnePerPassAndRecipientScoped`, walk 6 |
| R5 no cone, soft edge | T5 | `cellKnowledge.test.ts` edge weights, walk 4 |
| R8 sharing defaults off; never-chosen is distinguishable; legacy keeps meaning | T7 | `TestUnchosenSharingResolvesPrivate`, `TestLegacyRecordWithoutPrivateStaysShared`, `TestChoiceIsSavedAndSurvivesReload`, `TestChoosingTheDefaultIsStillAChoice`, walk on #595 |
| R9 wall known where sight reached its footprint; no presentation footing | T8a–T8e | `TestA11_APresentedWallEqualsItsSceneryTwin`, `TestStructuralWallCarriesItsFootprintCells`, `TestSightLearnsAWallsFootingByItsFace`, web "centre-line wall known at one footing hex", walk 1–3 on #600 |
| R9 the masquerade is the wall: a non-knower's atlas equals the solid-wall twin's at a secret door | T8b | `TestKnownWallKeepsFloorUnderAClosedDoor` (both arms, unchanged), `TestAnUnfoundSecretDoorTeachesItsThresholdAsTheWallWould`, `TestA11_APresentedWallEqualsItsSceneryTwin` |
| R5 client computes no visibility | T5 | review check: no raycast or line-of-sight import under `src/components/session` |
| Unknown floor is not a wall | T1 | `TestUnexploredCellsSynthesizeNoWall` |
| Sight areas and concealment stay separate | T1 | `TestVisibleCellsExcludesSightAreaAndConcealment` |
| Additive wire | T3 | `make test` breaking check |
| Patched atlas equals fresh atlas | T2, T5 | `TestCellsRevealedPatchEqualsFreshAtlas`, `applyReveal.test.ts` |

Deferred by the design, not covered: O1 pooling, O2 tint.

## Provider/consumer seams

| Provider task/module | Consumer task/module | Produced vs consumed contract | Availability/dependency | Proof |
|---|---|---|---|---|
| T1 encounter `VisibleCellsFor` | T2 session `projectView` | `VisibleCellsOutput.Cells []spatial.Position`, sorted, non-nil, subset of atlas | T1 pseudo-version, then tag | `TestKnowledgeViewAndAtlasAgree` |
| T1 encounter beat `cells_revealed` | T2 session `CellsRevealedBody` | payload keys listed in T1, `regions` partial | same | `TestCellsRevealedDecodes`, `TestCellsRevealedPatchEqualsFreshAtlas` |
| T1 encounter `AtlasFor` per cell | T2 `Manager.Knowledge` | existing `Atlas` type, per-cell contents | existing type; contents change | session structural tests updated in T2 |
| T2 session `ViewOutput.VisibleCells` | T4 api `GetView`, `GetKnowledge` | JSON `visible_cells` → proto field 5 | T2 pseudo-version, then tag | `TestGetViewCarriesVisibleCells` |
| T2 session `EventCellsRevealed` | T4 api `convert.go`, `stream_events.go` | `CellsRevealedBody` → `CellsRevealed` | same | `TestConvertCellsRevealed`, `TestStreamCellsRevealedReachesOnlyItsRecipient` |
| T3 protos `visible_cells`, `CellsRevealed` | T4 api, T5 web | field and message names in T3 | T3 merge publishes bindings | T4 compile; T5 `ci-check` |
| T7 session resolved preference | existing api `GetKnowledge` → web checkbox | `KnowledgeOutput.DiscoverySharing *bool` unchanged; value now resolves nil-profile to false | session tag, api re-pin | `TestUnchosenSharingResolvesPrivate`, walk on #595 |
| T8b encounter `AtlasStructuralWall.FootprintCells` | T8c session → T8d api `footprint_cells` → T8e web grading | positions in line order; empty from an older server keeps side sampling | T8b pseudo-version, then tag; T8a bindings | `TestStructuralWallCarriesItsFootprintCells`, api convert test, web "without footprint_cells grades as before" |
| T4 api stream and reads | T5 web `useSessionKnowledge`, `sessionRefreshKeys` | `Event.body.case 'cellsRevealed'`; `GetViewResponse.visibleCells` | T4 walk server | T6 walk 1–6 |
| Existing web refresh map | T5 seen-now freshness | events that schedule `view` today, plus `cellsRevealed` and the member's own `moved` | inspected `sessionRefreshKeys.ts` at the baseline: own `moved` does not schedule `view` | `sessionRefreshKeys.test.ts`, T6 walk 3 and 4 |

T1 and T2 touch different modules and share no files. T4 and T5 share no
files. Two web changes in other waves may touch `SessionEncounterView.tsx`;
T5 catches up by commit on top, never rebase.

## Plan check findings

- `applyRegionRevealed` replaces a region by id. Reusing `ROOM_REVEALED` for
  partial regions would truncate a client's cached region, so T3 adds a new
  kind and T5 a union merge. Disposition: built into T3 and T5.
- `discoveryAudience` treats sharing as on by default. Building the R4 fold
  would pool memory by default. Disposition: design O1, not built.
- The `AtlasFor` door rule needs no `DoorSightings` read to satisfy R2,
  because a cell learned by sight and a door observed by sight come from the
  same `reachesCell` over the same support cells. A door learned by door-edge
  opening or concealment reveal can lack a sighting. Disposition: T5 draws it
  closed in the remembered treatment; T1 pins the sight path.
- The web's refresh map does not refresh `view` on the member's own move,
  so seen-now would go stale on every step. Disposition: T5 adds `view` to
  the own-move keys.
- The full-atlas golden is the author's view and must not change.
  Disposition: stated in T1 tests.

## Record

Shipped 2026-10-16: rpg-toolkit#2056 → encounter v0.133.0 · rpg-toolkit#2058 → session v0.137.0 · rpg-api-protos#394 → v0.1.237 · rpg-api#1109 → dev ff90305e · rpg-dnd5e-web#1287 → dev 5842e048. Walk on the real stack: six scenarios, re-walked after four web fixes (reveal race, absent stretches not drawn per the operator, structural piece cuts, remembered surface tint). R8 shipped 2026-10-16 as Task 7: rpg-toolkit#2059 → session v0.137.1 · rpg-api#1110 → dev 9ede9ec4; no wire or web change; five-scenario walk on the castle stack, checklist on rpg-project#595. Open: seen-now lag after own move (rpg-dnd5e-web issue); per-seat view read cost on large floors. Record on rpg-project#586.
