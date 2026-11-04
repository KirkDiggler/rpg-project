---
name: The guard holds the gate — implementation plan
status: PLAN derived 2026-11-03 from design.md R1–R7 (all ruled on rpg-project#835); checked against the inspected baseline below; P1 amended 2026-11-03 on rpg-project#841; S1 DONE, S1b (R5) added 2026-11-03.
authority: design.md (Law, R1–R7), story rpg-project#836, lane rpg-project#835
planning guide: ../../.agents/skills/design/planning.md
---

# The guard holds the gate — implementation plan

## Goal and scope

A creature's state says whether its body blocks. A posted gate guard holds
the castle's crossing; the player walks into him and is stopped by name; a
bump writes a `barred` beat he answers in his own voice; a cowed or turned
guard opens the cell at once and walks off it on his next beat. A post
holds the passage its state names, not only his cell (R5).

**Non-goals.** A barred or locked gate door. A permit on movement.
Conditions that stop a walk. A bump that costs anything. The session
playing a dialogue line. `Prevented` in resolution. The captain reacting to
the guard's voice. A walk-env way to teach a fact.

**Repositories and order.** Develop outside-in, merge inside-out. Toolkit:
one module per PR. `dungeonspec` is a package of the encounter module.

| slice | repo / module | base | PR shape |
|---|---|---|---|
| S1 | rpg-toolkit `encounter` → `session` re-pin; rpg-api; rpg-dnd5e-web; rpg-project | main / main / dev / dev / main | encounter PR (code, castle mirror, castle tests); session re-pin PR; api PR (pin + the same content hunk); web PR (planner); walk skill PR |
| S1b | rpg-toolkit `encounter` → `session`; rpg-api-protos; rpg-api; rpg-dnd5e-web; rpg-project | main / main / main / dev / dev / main | encounter PR (code, castle mirror, castle tests); session PR; protos PR; api PR (pin, convert, the same content hunk); web PR (planner); walk skill PR |
| S1c | rpg-toolkit `encounter` → `session`; rpg-api; rpg-dnd5e-web (none expected); rpg-project | main / main / dev / dev / main | encounter PR (code, castle mirror, castle tests, `gateUnheld` deleted); session PR; api PR (pin + the same content hunk); walk skill PR |
| S2 | rpg-toolkit `encounter` → `session`; rpg-api-protos; rpg-api; rpg-dnd5e-web | main / main / main / dev / dev | encounter PR (code, castle mirror, castle test); session PR; protos PR; api PR (pin, convert, the same content hunk); web PR |

S1 is DONE: the primitive "a posted body blocks its cell" (rpg-toolkit
#2258 merged, encounter v0.175.0; session #2259, rpg-api #1183, web #1385
merging out). Its walk finding (rpg-project#841): the gate is a two-cell
opening and the party stepped through beside the guard; a talk with the
pedlar drew him off. R5 answers the first; the second waits for the use
case (design, Open). S1b next, on S1's tags. S1b is DONE after its walk
(Kirk, 2026-11-04: the held door refused, a won check released it;
encounter v0.176.0, rpg-toolkit #2263), merging out. S1c, the post's
settings (R6), follows on S1b's tags. S2 after S1b's encounter tag:
the halt it voices is the keep's "reaching the entrance halts the party".

**The castle file lives twice.** rpg-api's `content/the-mad-king.yaml` and
the toolkit's `encounter/dungeonspec/testdata/world-builder-v4-castle.yaml`
stay byte-identical. The encounter task edits the mirror with its castle
tests; the api task copies the same hunk. Neither waits on the other's
merge.

## Finish line

- Sub-issues for S1, S1b, S1c and S2 closed by their "done means"; each slice walked
  by the operator on :3030; #836's slices table current.
- On the castle, free roam: walking past the gate guard where he is the
  only way, the crossing in front of the shut gate door, stops the walk
  with his name; with S2 he
  says "Whoa there, buddy. Can I help you?", and a second bump gets his
  shorter line. A cowed or turned guard opens the crossing and walks to his
  door post (proved by castle tests; the dice are not walked).
- With S1b, the gate door open and `gate-1` posted, no step through either
  cell of the opening passes, alert or not; the stop names him; a drawn,
  cowed, turned or downed guard opens it, and with `gate-2` also posted on the
  opening, downing one does not.

## Requirement labels

| label | requirement | source |
|---|---|---|
| B1 | A member whose `BlockingStates` is non-empty blocks exactly while its current state is in it; a member with an empty set blocks as `BlocksMovement` says, whatever machine it runs. | R1, Law |
| B2 | The blocking answer is read from the state by every reader (the fold, spatial's placement check, the board, the observed holding); a transition opens or closes the cell in the beat it fires. | R1, R3, Law |
| B3 | A hostile body blocks whatever its state; downed, ally and pass-through rows unchanged. | Law |
| B4 | `BlockingStates` round-trips with the recipe on `MemberData` and `ReserveData`; a record without it loads as today; a stored state the recipe lacks is refused at load. | R1 |
| B5 | What an observer holds about a body's blocking follows the state. | Law |
| B6 | A step refused by a body returns `ErrBodyInTheWay`; its public reason names the body the mover observes, else "someone"; a driven walk still stops silently. | Law |
| C1 | The dialect accepts `blocks: true` on a state and refuses it on a terminal state. | R1 |
| P1 | The web plans round a seen Blocked body; it plans through a seen, non-hostile one only when no route round exists; a goal on his cell plans no path through him (clicking a body's cell means approach to speak); a hostile body stays unplannable; the walk stops where the server says. The first cut also planned through him when his cell was the goal, dropped on the rpg-project#841 review. | Law (the gesture) |
| Q1 | The castle's sneak and open-the-gate steps are re-aimed under R4 A, in the tests and the walk skill. | R4 |
| D1 | A player's step refused by a body (never a wall, door, field or masked refusal) writes a `barred` beat {mover, barrier, cell}, every time; a driven walk writes none. | R2 |
| D2 | `barred` is a targeted machine beat on the barrier, matched on him alone in the Move that bumped; nothing else that landed is matched; the reaction spends nothing. | R2, Law |
| D3 | The barrier's `pick` on `barred` writes an `answered` beat whose `say` is a voice; its hearers' `heard` waits for the next raise. | R2 |
| D4 | A bump on the turn clock pays no movement; the steps already taken stay. | Law |
| D5 | `Step` answers a body stop with output (beats, intel deltas, any fight formed or outcome, the stop reason), not an error; `Move` folds it and reports `stopped`. | Law |
| W1 | The session decodes `barred` into a `BarredAct`; the wire carries it additively; the api converts it; the web folds it with the answer and shows the line as the guard's speech. | R2 |
| G1 | castle tests, `step_test.go`, `blocksmovement_test.go`, `wakebeats_internal_test.go`, `machine_internal_test.go`, session `move` tests, web `atlasPath.test.ts` keep passing. | must keep working |
| H1 | A state may name a post: the door whose opening it holds and the cells a holder stands on. While a member is in that state, up and on a post cell, the door's crossings (an edge door's edges, a footprint door's crossings) are closed to every mover not allied with him, open or shut, any width; his cell still blocks. | R5 |
| H2 | The hold is a state at a place, read on every step and route, per mover, beside the masquerade and before the door pass; it is never in the shared crossing memo, and a walk never opens a held door. | R5 |
| H3 | The holder is the post: an opening is held while any member is in a holding state, up and on a post cell; killing one does not open it. | R5 |
| H4 | A step across a held edge stops with `ErrBodyInTheWay`; its reason names the nearest holder the mover observes ("<name> bars the way"), else "someone bars the way"; driven walks route around a held edge and stop silently. | R5 |
| H5 | The posts are stored beside `BlockingStates`; an observer holds what it saw of the hold; the sighting says which door a seen holder bars against the viewer. | R5 |
| H6 | The atlas lists a footprint door's crossings as doorways, marked `crossing` (additive field on `AtlasDoorway`); renderers draw a doorway only for an edge door; the web plans a shut or held door as shut; the second plan still runs only when no route round exists. | R5, P1 |

## Constraints

- Toolkit: `go test -race ./...` and `golangci-lint run ./...` per module;
  no `go.work`, no local `replace` in a PR; consumers pin by commit, then by
  tag minted on merge.
- `mind/machine` and `mind/behavior` are not changed; `blocks` and `barred`
  are the encounter's words.
- Protos: deprecate, never break (`buf breaking` against main).
- rpg-api: `make test`, `make pre-commit`. Web: `npm run ci-check`.
- Content is castle QA content: it goes into the castle file and its
  mirror, never a one-off site.
- Team signature only; one review round per slice; the slice staged on
  :3030 for the operator's walk before merge-out.

## Inspected baseline

| repo | revision | symbols |
|---|---|---|
| rpg-toolkit | origin/main 20a124ad (encounter v0.174.0, session v0.161.0) | `encounter/cellfacts.go`: `Passage` :12-27, `CellAt` doc :115-135, member contributor :303-330, `blockedBy` :478-503, `occupantPassage` :528-540. `encounter/encounter.go`: member creation :992, :1015, record load :1261, `join` :2601-2632, `memberEntity.BlocksMovement` :3048-3050. `encounter/board.go`: :167, :175. `encounter/data.go`: `ReserveData.Recipe` :525, `MemberData.Recipe` :2166. `encounter/observedpassage.go`: :43. `encounter/step.go`: `Step` refusal :138-141, `stepMember` :218-410, last-step pass-through :334-336, body refusal :347-352, `stepTo` silent list :440-452. `encounter/field.go`: `StepOutput` :1728, `IntelDeltas` :1763, `Outcome` :1644. `encounter/walkend.go`: `StepObstructedError.PublicReason` :35-48. `encounter/machine.go`: `machineVocabulary` :92, `vocabularyFor` :117, `programFor` :339-350, `runPick` :516, `MachineBeats` :703, `MachineBeatScoped` :726, `ValidateMachine` :751. `encounter/wakebeats.go`: law :19-50, beat words :57-80, `landHeard` :173, `landFactBeat` :212, `matchOne` :320, `react` :392. `encounter/clocks.go`: `runIntents` :588, pick then `answerTime` :628-638. `encounter/answer.go`: voice :239-275, `answerTime` :254. `encounter/dungeonspec/states.go`: `StateSpec` :64, `ValidateMachine` call :319. `encounter/castletest/castle_king_test.go`: fixture :42. `encounter/castletest/castle_king_script_test.go`: cells :53-98, `TestMadKingScriptASneakWindowAtTheDoor` :453, `TestMadKingScriptOpeningTheGateBringsTheFarRoom` :670. `session/move.go`: `CanPay` :446, obstruction :455-461, `Pay` :473. `session/act_family.go`: `answeredPiece` :725-765. `session/act_bodies.go`: `TableAnswer` :394-415. `tools/spatial/room.go`: `CanPlaceEntity` reads `BlocksMovement()` :452. |
| rpg-toolkit (S1b) | origin/main 9c745794 (encounter v0.175.0) | `encounter/machine.go`: `memberBlocks` :328. `encounter/cellfacts.go`: memoized `crossingBlocked` :188. `encounter/placed_props.go`: `doorAcrossCrossing` :732, `doorCrossings` :761, `Encounter.crossingBlocked` :1067. `encounter/step.go`: masquerade check :310, silent list :454, `bodyRefusalError` :483-497. `encounter/clocks.go`: `walkerEdge` :2571. `encounter/observedpassage.go`: `ObservedPassages` :14. `encounter/atlas.go`: doorways from edges :628-634. `encounter/wakebeats.go`: `landDeedBeat` :200-210. `encounter/dungeonspec/states.go`: `guardOf` :337-358. `encounter/dungeonspec/single_room_doors.go`: wall door id :300-312. `encounter/castletest/castle_king_test.go`: `kingGateDoor` :71-72. `session/read.go`: sighting passage :457-476. Mirror: gate opening :725-731, `post` rows :321-330, `gate-1` states :1717-1758. |
| rpg-api-protos (S1b) | origin/main 22c45cf (v0.1.253) | `session/v1alpha1/types.proto`: `Sighting` :990, `passage = 11` :1067; `DoorInfo` :2639-2646 |
| rpg-api-protos | origin/main 22c45cf (v0.1.253) | `dnd5e/api/session/v1alpha2/act.proto`: act body oneof (`table_answer = 34` :104) |
| rpg-api | origin/dev 6cb2d92 | `content/the-mad-king.yaml`: gate tables :272-327, dispositions :524-531, gate guards :1544-1565, `gate-1` states :1681-1698, captain :1705-1707, stand-aside :1723-1737, turn :1738-1747, gate door :2012-2015; `internal/handlers/dnd5e/session/v1alpha2/convert_act.go` |
| rpg-dnd5e-web | origin/dev 1334dcd5 | `src/components/session/occupantPassages.ts` `indexOccupantPassages` :6, BLOCKED :14; `src/components/session/atlasPath.ts` `edgePassable` :163-175, `findAtlasPath` :187; `src/components/session/stanceRing.ts` :47, `partySide.ts` :39; `src/components/session/combat-experience/describeAct.ts` `tableAnswer` :478, :844; `foldLogRows.ts` :32-41 |
| rpg-project | origin/main 2a666a2 | `.agents/skills/walk/SKILL.md`: castle steps 2 and 5 :17, :20; dice rule :25-30 |

## Sequence

1. S1: T1 (encounter, with the castle mirror and its tests) → tag → T2
   (session re-pin) → tag → T3 (api pin + content hunk). T4 (web planner)
   in parallel with T1. T5 (walk skill) with T3. Stage T3 + T4 on :3030 →
   walk → merge out.
2. S1b: T11 (encounter, on T1's tag, with the mirror) → tag → T12
   (session) → tag. T13 (protos) in parallel → tag. T14 (api, on the
   session and protos tags) → T15 (web, on the protos tag). T16 (walk
   skill) with T14. Stage → walk → merge out.
3. S1c: T17 (encounter, on T11's tag, with the mirror) → tag → T18
   (session) → tag → T19 (api pin + content hunk). T20 (web) only if T18
   finds the planner needs the direction. T21 (walk skill) with T19.
   Stage → walk → merge out.
4. S2: T6 (encounter, on T11's tag, with the mirror) → tag → T7 (session) →
   tag. T8 (protos) in parallel → tag. T9 (api, on the session and protos
   tags) → T10 (web, on the protos tag) → stage → walk → merge out.

## Open items

- Staging finding (rpg-project#852): the env's served content shelf must
  be copied from the api's content after any `up` that changes content,
  then a fresh castle launched; launched members keep their old shape. The
  first S1b walk served the old castle (no `holds:`) and walked straight
  through the gate. Every staging checklist for a content slice carries
  the shelf copy.

- Resolved: R4 A. `gate-1` holds the crossing continuously while
  `posted` and never leaves it by time; `gate-2` keeps its loop; the sneak
  test is re-aimed to draw `gate-1` off and cross on `gate-2`'s blind beat.
- Resolved: the state binding is the encounter's (`MemberInput.BlockingStates`,
  stored beside the recipe), compiled from the dialect's `blocks: true`;
  `mind/machine` is untouched.
- Resolved: blocking is derived from the current state on read; the canvas
  entity reads through the encounter rather than holding a copy.
- Resolved: the guard's line rides the existing `answered` beat and table
  answer; S2's wire addition is the `barred` act only.
- Resolved: `barred` lands on the barrier alone; bystanders hear his voice
  on the next raise. The captain's reaction is not in this story.
- Resolved: rows gated on a die are covered by tests and not walked
  (`.agents/skills/walk/SKILL.md:48-53`); no walk-env mechanism teaches a
  fact today, and none is built here.
- Resolved 2026-11-03 (rpg-project#841, the lead's call on the web planner
  review): P1 keeps only the "no route round exists" leg. The lead:
  "clicking a body's cell already means 'approach to speak' in the web, an earlier operator choice this story does not reverse. The planner's second plan therefore drops its 'his cell is the goal' leg and keeps only 'no route round exists'."
- Accepted from Task 1's build (rpg-toolkit#2258), recorded in Task 1
  below: the canvas treats a downed blocking monster as the fold does; the
  troupe success teaches `troupe-admitted`.
- Dropped (Task 1, the lead's call on the rpg-toolkit#2258 review): the
  `ValidateWalkEnd` relaxation that let a walk aim at a seen, non-hostile
  blocking body. The #841 gesture call removed its caller: no walk aims at
  his cell now, and the session checks only the walk's last cell
  (`session/move.go:172`). A liberty with no caller is not granted; it
  returns with the use case that needs it. #2258 removes it in its
  follow-up push.
- Open (Task 1): the roster row's `Member.BlocksMovement` reports the
  state, not the declaration, so every reader gives one answer (B2). It
  lands in rpg-toolkit#2258 or in Task 2, as the #2258 review decides.

- Open, content (the lead's call, to confirm): `greet` is a plain line
  that cannot fail and costs nothing, so teaching `gate-pass` on it lets
  any party that says good evening through the gate both ways. The
  recommendation is the troupe success alone (a check) plus the existing
  stand-asides; greet stays in Task 17 as asked until this is confirmed.
- Resolved: R6 A with two knobs (Kirk, 2026-11-04, lane #835). A post
  holds both ways by default; its setting may name a direction
  (`against`) and a pass fact (`pass`). The castle stays both ways and
  gains `gate-pass`; the ten outbound routes earn it or stand him aside,
  and `castleHost.gateUnheld` is deleted (S1c).

## Task contracts

### Task 1 (S1): a body blocks by its state

**Delivers:** B1–B6, C1, Q1 in the toolkit.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter` (with `dungeonspec`).
**Prerequisites:** none.
**Files:** modify `encounter.go` (`MemberInput.BlockingStates`, the member
record, `memberEntity`), `data.go` (`MemberData.BlockingStates`,
`ReserveData.BlockingStates`), `cellfacts.go` (member contributor),
`board.go`, `observedpassage.go` and the holding refresh, `step.go` (body
refusal, `stepTo` silent list), `walkend.go` (public reason), `errors.go`
(`ErrBodyInTheWay`), `machine.go` (validate `BlockingStates` against the
recipe); `dungeonspec/states.go` (`StateSpec.Blocks`);
`dungeonspec/testdata/world-builder-v4-castle.yaml` (the content below);
new `postedbody_test.go`; update `blocksmovement_test.go`, `dungeonspec`
tests, castle tests.
**Interfaces:**
- `MemberInput.BlockingStates []string`: the states in which the body
  blocks. Refused when it names a state the recipe lacks, or when it is set
  without an authored recipe (fail closed: a blocking state with nothing
  to name is a bug).
- `MemberData.BlockingStates` and `ReserveData.BlockingStates`,
  `json:"blocking_states,omitempty"`, beside `Recipe`. Absent loads as
  empty (today's behaviour); a stored name the recipe lacks is refused at
  load with `ErrInvalidData`.
- `func (e *Encounter) memberBlocks(id MemberID) bool`: when the member's
  `BlockingStates` is non-empty, its current machine state is in it; else
  `BlocksMovement`. Every machine counts: the default recipe never carries
  blocking states, so a tabled monster declared `BlocksMovement: true`
  still blocks.
- `memberEntity` drops its bool and holds a read through the encounter
  (`blocks func() bool`, bound to `memberBlocks` at creation, at Join, at
  load and on the board). Spatial's `CanPlaceEntity`, the contributor, the
  board and the observed holding all read it. No second copy is stored.
- `StateSpec.Blocks bool yaml:"blocks,omitempty"`; the compiler collects
  the blocking states into `BlockingStates`; `blocks` on a terminal state
  is refused.
- `ErrBodyInTheWay`, not wrapping `ErrBadPlacement`; a body-blocked step's
  `StepObstructedError` wraps it. `PublicReason()` checks it first and
  answers `"<name> stands in the way"` for a body the mover observes and
  `"someone stands in the way"` otherwise. `stepTo` treats it as a silent
  stop.
**Content (in the mirror; T3 copies the hunk):**
- `gate-1` states: `posted` (`blocks: true`) holds the cell beneath the
  gate, in front of the shut door, continuously (R4 A); `alert` blocks too. `gate-2` keeps its loop
  unchanged.
  `posted` and `alert` → `cowed` on `fact` when `fact:gate-cowed`; → `turned`
  on `fact` when `fact:guards-turned`; `cowed` and `turned` do not block.
- `gate-patrol`: `posted`'s key stands the crossing; a `cowed` key walks
  him to the west door post and stands; `guards-turned` keeps its march.
- The captain's stand-aside line unchanged.
- The captain's troupe success teaches `troupe-admitted`; `posted` and
  `alert` → `cowed` on it, so the QA script's troupe route is let through
  the gate (accepted from rpg-toolkit#2258).
- `posted`'s key draws him toward a voice heard this round and otherwise
  stands the crossing. The captain beside the crossing draws nobody: an
  observation of the #2258 fixture that no test names.
**Accepted from the build (rpg-toolkit#2258):**
- The canvas treats a downed blocking monster as the fold does: standable.
  The canvas asks participation only for a blocking monster on a cell
  someone is placed on.
**Behavior:** the fold is unchanged. A transition that enters or leaves a
blocking state refreshes the observed holding of that body for every
observer that sees it.
**Tests:**
- `TestAPostedNeutralBodyBlocksAStepAndARoute`: a step onto its cell is
  refused with `ErrBodyInTheWay`; a route to the far side goes round or is
  not found.
- `TestLeavingThePostOpensTheCellInThatBeat`: a `fact` transition to a
  non-blocking state; the next step passes in the same raise.
- `TestTheCanvasAndTheFoldAgreeAfterATransition`: `CanPlaceEntity` and
  `CellAt` give the same answer before and after.
- `TestAHostileBodyBlocksInAnyState`.
- `TestATabledMonsterDeclaredBlockingStillBlocks`: default recipe, empty
  set, `BlocksMovement: true` → Blocked; false → PassThrough.
- `TestAStoredEncounterLoadsItsBlocking`: save a member and a reserve in a
  blocking state, load, the step is refused; a record without the field
  loads as today; an unknown stored state is refused.
- `TestBlockingStatesAreRefusedWithoutARecipe` and for an unknown state.
- `TestABodyRefusalNamesTheBody`: observed → its name; unobserved → "someone".
- `TestADrivenWalkStopsSilentlyAtABody`.
- dungeonspec: `blocks: true` compiles; on a terminal state it is refused.
- Castle (the mirror): the party's walk toward the courtyard stops one cell
  short of the guard with his name; `gate-cowed` taught to a guard who
  witnessed it opens the next step and his next beat walks him to the door
  post; a guard who did not witness it keeps blocking; `guards-turned`
  opens it too. `TestMadKingScriptASneakWindowAtTheDoor` is re-aimed: the
  crossing holds against the sneaker while `gate-1` is posted, and passes
  once he is drawn off by a noise and the sneaker crosses on `gate-2`'s
  blind beat. A posted `gate-1` stands the crossing through any number of
  `time` beats. And
  `TestMadKingScriptOpeningTheGateBringsTheFarRoom` teaches `gate-cowed`
  before the party stands beneath the gate.

- [ ] Add the tests; see them fail.
- [ ] Implement the binding, the read, the storage, the refusal, the dialect, the content.
- [ ] `go test -race ./... && golangci-lint run ./...` in `rulebooks/dnd5e/encounter`.

**Verification:** the commands above, green.
**Completion evidence:** PR link, the encounter tag, test names green.

### Task 2 (S1): the session re-pins

**Delivers:** B6 at the session (the new reason reaches `stop_reason`).
**Owner:** rpg-toolkit `rulebooks/dnd5e/session`.
**Prerequisites:** Task 1 tagged.
**Files:** `go.mod`; `move` tests. No `read.go` mapping: a body refusal
always arrives inside `StepObstructedError`, which `Move` catches first.
**Tests:** a Move into a posted body answers `stopped` with the guard's
name in `stop_reason` and the steps before it kept.

- [ ] Re-pin; add the test; `go test -race ./... && golangci-lint run ./...`.

**Completion evidence:** PR link, session tag.

### Task 3 (S1): the gate is held on the server

**Delivers:** B1 on the castle served by the api.
**Owner:** rpg-api.
**Prerequisites:** Task 2 tagged.
**Files:** `go.mod`; `content/the-mad-king.yaml` (Task 1's content hunk,
byte for byte).
**Tests:** `make test`; a diff of the two castle files is empty.

- [ ] Re-pin; copy the hunk; `make test && make pre-commit`; stage on :3030.

**Completion evidence:** PR link.

### Task 4 (S1): the web walks into him

**Delivers:** P1.
**Owner:** rpg-dnd5e-web.
**Prerequisites:** none (the passage is on the wire today).
**Files:** `src/components/session/occupantPassages.ts`,
`src/components/session/atlasPath.ts`; `atlasPath.test.ts`.
**Interfaces:** `indexOccupantPassages` keeps a seen body whose passage is
`BLOCKED` and whose stance is not hostile (the stance the wire already
carries, `stanceRing.ts:47`, `partySide.ts:39`) out of `blockedCells`, so
`edgePassable` and `findAtlasPath` route through it, but only on a
second plan: the first plan keeps every Blocked body blocked, and the
second runs only when the first finds no path. A goal on such a body's cell
is approach to speak and plans no path through him (P1, amended on
rpg-project#841). The hover preview draws a path into him only on the
second plan. A hostile `BLOCKED` body stays blocked in both. The walk stops
where the Move's answer says.
**Tests:** in an open room with an equal detour round a non-hostile
blocked body, the path goes round and never enters his cell (no bump); in
a one-cell corridor he fills, the path goes through him; his cell as
goal → no plan through him; a hostile one is never planned through.

- [ ] Implement; `npm run ci-check`; stage with Task 3.

**Completion evidence:** PR link, the walk on :3030.

### Task 5 (S1): the walk script follows the post

**Delivers:** Q1 in the walk skill.
**Owner:** rpg-project.
**Prerequisites:** none (R4 ruled A).
**Files:** `.agents/skills/walk/SKILL.md` steps 2 and 5: the sneak goes
draw `gate-1` off the crossing (a voice heard this round draws him; the
captain beside the crossing drawing nobody is a fixture observation no test
names), then cross on `gate-2`'s blind
beat; step 5 walks past him where he is the only way (a click beyond the
crossing, never his cell) and stops one cell short by his name; the door
opens from beside the crossing, and the crossing is stood on only once
emptied; standing aside and the troupe admission are dice-gated and
covered by tests, not walked.

- [ ] Edit; `scripts/check-doc-genres.sh`; PR beside Task 3.

**Completion evidence:** PR link.

### Task 11 (S1b): a post holds a passage

**Delivers:** H1–H5 in the encounter; the atlas half of H6.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter` (with `dungeonspec`).
**Prerequisites:** Task 1 tagged (encounter v0.175.0).
**Files:** `field.go` (`MemberInput.Posts`), `data.go`
(`MemberData.Posts`, `ReserveData.Posts`), `machine.go` (validation), new
`hold.go` (`Post`, `memberHolds`, `heldAgainst`), `step.go` (the held-edge
check beside the masquerade, before the door pass), `clocks.go`
(`walkerEdge` reads it per mover, before `doorPast`), `walkend.go` /
`step.go` (the held reason), `observedpassage.go` and the holding refresh
(what an observer saw of the hold), `atlas.go` (a footprint door's
crossings as doorways); `dungeonspec/states.go` (`StateSpec.Holds`); the
castle mirror; new `hold_test.go`; castle tests.
**Interfaces:**
- `type Post struct { Door DoorID; Cells []spatial.Position }`.
  `MemberInput.Posts map[string]Post`: state → the post that state holds.
  Empty `Cells` means the cell the member is placed at. Refused when a
  state is not in the recipe, the door is unknown, a cell is not floor, or
  there is no authored recipe.
- `MemberData.Posts` and `ReserveData.Posts`, `json:"posts,omitempty"`,
  beside `BlockingStates`; absent loads as none; an unknown state, door or
  cell is refused at load (`ErrInvalidData`).
- `func (e *Encounter) memberHolds(id MemberID) (DoorID, bool)`: the door
  of the post its current state names, true only while the member is up
  and stands on one of the post's cells. Read on every call, never stored,
  so leaving the cell (any walk, a push) empties the hold at once and
  standing there again restores it in that step.
- The opening's crossings: an edge door's `edges`; a footprint door's
  `doorCrossings` (`placed_props.go:761`), computed once per door.
- `func (e *Encounter) heldAgainst(mover MemberID, from, to spatial.Position) []MemberID`:
  the members holding a door whose crossings include this one, not allied
  with the mover, nearest first.
- Order: the hold is read before the door pass. `walkerEdge` closes a held
  edge before it asks `doorPast`, and a player's step refuses a held edge
  before any door is opened, so a walk never opens a held door.
- R7: `ErrDoorHeld` (errors.go). OpenDoor, CloseDoor and Unlock read the
  hold after reach and before the door's state (validation order: … →
  reach → held → locked/already open/not locked → …, `doorverbs.go:128`,
  `:221`, `:383`): an actor not allied with any holder of the door is
  refused with a `doorHeldError` wrapping `ErrDoorHeld`, whose
  `PublicReason()` names the nearest holder the actor observes ("the Guard
  bars the door"), else "someone bars the door". Atomic: no state change,
  no beat (S2 adds the `barred` beat). The held arm runs after the door is
  found and its id checked, so `ErrNoDoor` and `ErrBadDoor` keep their own
  answers and are never reported as a hold.
- `func (e *Encounter) DoorHeldFor(actor MemberID, door DoorID) (held bool, reason string, err error)`:
  the same read as the verbs' arm, as a query, so the session can ask it
  before it rolls Unlock's check or poses a Guidance pause.
- A held step returns `StepObstructedError` wrapping `ErrBodyInTheWay`;
  `PublicReason()` answers "<name> bars the way" for the nearest holder the
  mover observes, else "someone bars the way". Driven walks route round a
  held edge and stop silently at one.
- The observer's holding of a member gains the door it holds, refreshed
  when the hold changes (state or cell), as `BlocksMovement` is;
  `ObservedHolds(view) map[MemberID]DoorID` answers, for a viewer, the
  doors seen holders bar against it (allies omitted).
- `Atlas()` lists a footprint door's crossings as `AtlasDoorway{Door, From,
  To, Crossing: true}` beside an edge door's edges (`Crossing` false,
  `atlas.go:628-634`), under the same concealment rule as edge doorways.
  `Crossing` says the pair is a footprint door's crossing, which the map
  already draws as its placed asset: readers that plan and grade fog read
  both kinds, and renderers draw a doorway only for an edge door. The
  reveal beats that carry doorways (`revealbeat.go:226`,
  `roomreveal.go:165`) list a footprint door's crossings the same way, with
  `crossing`, so a revealed footprint door joins the cached atlas too.
- `StateSpec.Holds`: `holds: { door: <authored id>, at: [{ q, r }, …] }`,
  cells in the same `{ q, r }` form a table's `toward: { at }` uses.
  dungeonspec resolves the authored door id to the encounter's
  (`the-mad-king/gate-wall-door`, `single_room_doors.go:308`) and refuses
  an unknown opening or door id, a non-floor cell, and `holds` on a
  terminal state.
- No new state and no guard word: every leaving already moves him off his
  cells. A state that must change on a kind of sound would ride a
  `heard:<kind>` beat, landed as `deed:<word>` is (`wakebeats.go:200-210`);
  none is needed here.
**Content (in the mirror; T14 copies the hunk):** `gate-1` `posted` and
`alert`: `holds: { door: gate-wall-door, at: [{ q: -3, r: 7 }, { q: -4, r: 7 }] }`,
the two gatehouse-side cells of the opening (the cell he stands on and the
one the party stepped through in the #841 record); `blocks: true` as
before. The `post` key's rows are unchanged (mirror `:321-330`).
`TestMadKingScriptASneakWindowAtTheDoor` keeps its S1 aim: a voice draws
`gate-1` off his cells, and the sneaker crosses on `gate-2`'s blind beat.
**Tests:**
- `TestAPostHoldsBothCellsOfAFootprintOpening`: a two-cell open footprint
  door, a holder on one post cell; a step through either cell is refused
  with his name; his own cell is refused as before.
- `TestAPostHoldsAnEdgeDoor`: the same through an edge door's edges.
- `TestAHoldWorksOnAnOpenAndAShutDoor`.
- `TestAWalkNeverOpensAHeldDoor`: a shut held door on a walk that opens
  doors; the door stays shut and the stop names the holder.
- `TestAHeldDoorRefusesItsVerbs` (R7): OpenDoor, CloseDoor and Unlock by a
  non-allied actor in reach of a held door are refused with `ErrDoorHeld`,
  naming the observed holder ("Guard bars the door"), else "someone";
  nothing changes and no beat is written. A held locked door names the
  guard, not the lock.
- `TestAnAllyAndTheHolderWorkAHeldDoor`: an ally of the holder, and the
  holder himself, open and close it as before.
- `TestTheCellsInFrontOfAHeldDoorStayReachable`: a walk to the cell
  beside the guard, in reach of the door, is not stopped.
- `TestAnAllyCrossesAHeldOpening`.
- `TestTheHoldIsAStateAtAPlace`: off his cells in a holding state, the
  opening passes; back on a cell, it is held in the same step; a
  non-holding state on his cell holds nothing.
- `TestAnyLeavingEmptiesTheHold`: walked to a voice, to a clang, on a
  chase, and pushed by a directive; each opens the passage.
- `TestAnAlertGuardStillHolds`: alert on his post, the opening is held.
- `TestAnOccupiedPostCellIsNotRetaken`: a stranger on his only post cell;
  he holds nothing until it is free; with two post cells, he holds from
  the other.
- `TestADownedHolderHoldsNothing`.
- `TestTheGroupHoldsTheGate`: two members posted on one door; downing one
  leaves it held; downing both opens it.
- `TestARouteGoesRoundAHeldOpening` and `TestADrivenWalkStopsSilentlyAtAHeldEdge`.
- `TestAnUnseenHolderIsSomeone`.
- `TestAStoredPostLoads`: member and reserve; unknown state, door or cell refused.
- `TestTheAtlasListsAFootprintDoorsCrossings`, and a hidden one's are
  withheld as an edge door's are.
- dungeonspec: `holds` compiles to the qualified door id and its cells;
  an unknown id, a non-floor cell and a terminal state are refused.
- Castle (the mirror): with the gate door open and `gate-1` posted, the
  party cannot step through either cell of the opening, nor while he is
  alert on his post; it can while he is drawn off and once he is cowed,
  turned or downed; with `gate-2` given a post on the same door in the
  test, downing `gate-1` alone does not open it.

- [ ] Tests first; implement; `go test -race ./... && golangci-lint run ./...` in `rulebooks/dnd5e/encounter`; run record.

**Completion evidence:** PR link, encounter tag.

### Task 12 (S1b): the session carries the hold

**Delivers:** H4 at the Move, H5 at the sighting, R7's refusal at the
door verbs.
**Owner:** rpg-toolkit `rulebooks/dnd5e/session`.
**Prerequisites:** Task 11 tagged.
**Files:** `go.mod`; `read.go` (beside `ObservedPassages`, `read.go:457`:
`Sighting.HoldsDoor` from `ObservedHolds`); the sighting type; `types.go`
(`AtlasDoorway.Crossing bool json:"crossing,omitempty"` on the session's own
doorway type, :643; the door-revealed body's doc at :1315-1324, which says
a footprint door has no doorways, now says its crossings arrive marked);
`convert.go` (the atlas copy, :178-183, copies `Crossing`); `act_family.go`
(the reveal act's doorways, :1102-1126, decode `crossing`); `read.go`
(the translate, :740-756, returns bare sentinels and drops the message
today: its new arm wraps the session's `ErrDoorHeld` in a
`DoorHeldError{Reason}` carrying the encounter's public reason, and
`errors.Is(err, ErrDoorHeld)` still holds); `doors.go` and
`close_door.go` (OpenDoor and CloseDoor answer it as their refusal;
Unlock asks `DoorHeldFor` first, `doors.go:256`, before it rolls the lock
check or poses a Guidance pause, `:363`); tests.
**Tests:** a Move through the held opening answers `stopped` with "Guard
bars the way"; the viewer's sighting of a posted `gate-1` on his post names
`the-mad-king/gate-wall-door`; an allied viewer's, a cowed guard's and a
drawn-off guard's do not; the atlas the session serves carries the castle
gate's crossings as doorways with `Crossing` true and an edge door's
doorways with it false; a reveal act for a footprint door carries its
crossings with `Crossing` true, and an edge door's reveal carries false.
OpenDoor, CloseDoor and Unlock on the castle gate from the cell beside a
posted `gate-1` are refused with `ErrDoorHeld` and the error text "Guard
bars the door"; Unlock rolls nothing and poses no pause; once he is
cowed they pass.

- [ ] Re-pin; implement; `go test -race ./... && golangci-lint run ./...`.

**Completion evidence:** PR link, session tag.

### Task 13 (S1b): the wire says which door a body holds

**Delivers:** H5 on the wire.
**Owner:** rpg-api-protos.
**Prerequisites:** none.
**Files:** `dnd5e/api/session/v1alpha1/types.proto` (`Sighting`,
`AtlasDoorway`); `dnd5e/api/session/v1alpha2/act_geometry.proto`
(`AtlasDoorway`, `RevealedDoor`); generated Go and TS.
**Interfaces:** `bool crossing = 8;` on the v1alpha1 `AtlasDoorway` (the
atlas) and `bool crossing = 4;` on the v1alpha2 `AtlasDoorway` (the act
stream; 1–3 used, nothing reserved): true for a footprint door's crossing,
false (the zero value, every doorway today) for an edge door's edge.
`RevealedDoor`'s doc, which says its doorways are empty for a footprint
door, now says a footprint door's crossings arrive marked `crossing`. The web cannot tell the kinds from its door map:
`DoorInfo` carries only the id, state and lock (`types.proto:2639-2646`).
`string holds_door = 12;` on `Sighting`: the door id
(`DoorInfo.door`, `AtlasDoorway.connection`) whose opening this body bars
against the viewer; empty when none. The door's own state stays its own:
a hold is a body's fact, not a door state (R1 B). It rides the sighting
because the sighting already refreshes when what the viewer holds about a
body changes; `DoorInfo` refreshes only on door beats.
**Tests:** none hand-written; `buf lint`, `buf breaking`.

- [ ] Edit, generate, lint, breaking check; PR; tag on merge.

**Completion evidence:** PR link, protos tag.

### Task 14 (S1b): the api converts and the gate holds its passage

**Owner:** rpg-api.
**Prerequisites:** Task 12 and Task 13 tagged.
**Files:** `go.mod`; the sighting conversion; the v1alpha1 atlas
converter and the v1alpha2 act converter (the revealed door's doorways),
both copying `crossing`; their tests;
`internal/handlers/dnd5e/sdkerr/errors.go` (`sdk.ErrDoorHeld` joins the
FailedPrecondition bucket beside `ErrLocked` and `ErrDoorShut`, :237-238;
the error text, "Guard bars the door", rides the status message) and
`errors_test.go` (its exhaustiveness table, :227, gains the row);
`content/the-mad-king.yaml` (Task 11's hunk, byte for byte).
The web needs no change for R7: its door notice already shows the error
text.
**Behavior:** the atlas's doorways and a revealed door's doorways carry
`crossing` on both wires.
Corrected 2026-11-03 by the lead: the v1alpha2 helper is `doorwaysToProto`
in `internal/handlers/dnd5e/session/v1alpha2/convert_geometry.go`; this
task first named `convert_act.go`.
**Tests:** convert test for `holds_door`; a v1alpha1 atlas test that the
castle gate's crossings arrive with `crossing` true and an edge door's
false; a v1alpha2 act test that a revealed footprint door's crossings
arrive with `crossing` true and an edge door's false; the two castle files diff empty;
`make test && make pre-commit`.

- [ ] Re-pin; implement; stage on :3030.

**Completion evidence:** PR link.

### Task 15 (S1b): the web plans a held opening as shut

**Delivers:** H6.
**Owner:** rpg-dnd5e-web.
**Prerequisites:** Task 13 tagged.
**Files:** `package.json`; `src/components/session/atlasPath.ts`
(`buildAtlasPathIndex` takes the held door ids and adds their doorway
edges to `shutDoorEdges`); the caller that builds the index from live
sightings; the renderers that read `atlas.doorways`:
`atlasWallRuns.ts` (`doorGaps`, :227-270), `AtlasWalls.tsx` (:266),
`atlasToScene3D.ts` (fog, :367-382), `SessionEncounterView.tsx` (door
targets, :484); `atlasPath.test.ts`, `atlasWallRuns.test.ts`.
**Interfaces:** the atlas's doorways now include a footprint door's
crossings (Task 11), marked `crossing: true`. The planner reads both kinds,
so a shut footprint door is planned shut, which it was not before. Fog
grading reads both kinds. The renderers (`doorGaps`, the wall cut, the
door leaf, door targets through a gap) skip a `crossing` doorway: the
footprint door is already drawn, and targeted, as its placed asset. A door
named by a live (not remembered) sighting's `holdsDoor` is planned as
shut. The second plan (P1) still runs only when
no route round exists, and plans through a held door then.
**Tests:** with a footprint-door fixture (crossings as doorways, no
edges): a shut one is not planned through; an open one held by a seen body
is not planned through when a way round exists; with no way round, the second plan crosses it; a hold
that clears (the sighting drops `holdsDoor`) opens the edge on the next
build; a remembered sighting holds nothing; a footprint door's crossings
produce no `doorGaps` and no wall cut, while an edge door's still do; a
footprint door revealed under fog (its crossings appended from the act's
`RevealedDoor`) produces no door gap either.
From the rpg-dnd5e-web#1388 review (a minor): the web's reveal test covers
the room-reveal path, which merges into the atlas through
`applyCellsRevealed`; a revealed door's doorways reach the atlas through
the GetAtlas refresh, and the test proves only that the act bridge keeps
`crossing`.

- [ ] Bump; implement; `npm run ci-check`.

**Completion evidence:** PR link, the walk on :3030.

### Task 16 (S1b): the walk script holds the passage

**Owner:** rpg-project. **Prerequisites:** Task 14 staged.
**Files:** `.agents/skills/walk/SKILL.md`: the S1b rows below in the castle
steps.

- [ ] Edit; `scripts/check-doc-genres.sh`; PR beside Task 14.

**Completion evidence:** PR link.

### Task 17 (S1c): the post's settings, direction and pass fact

**Delivers:** R6 in the encounter.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter` (with `dungeonspec`).
**Prerequisites:** Task 11 tagged (encounter v0.176.0).
**Files:** `post.go` (`Post`, `PostData`, `heldAgainst`, `DoorHeldFor`,
`refuseHeldDoor`, `ObservedHolds`), `region.go` (the post's side),
`dungeonspec/states.go` (`holds:` gains `against` and `pass`); the castle
mirror; `castletest` (the ten outbound routes; `castleHost.gateUnheld`
deleted); new tests in `post_test.go`.
**Interfaces:**
- `Post` gains `Against PostDirection` (`"both"` default, `"entering"`,
  `"leaving"`) and `Pass FactID` (empty: none). `PostData` stores both,
  `omitempty`; a record without them loads as both ways and no pass.
- The post's side is the region its cells stand in (`RegionAt`). A post
  naming a direction whose cells stand in more than one region is refused
  at the door and at load: its side would be ambiguous.
- `heldAgainst` (`post.go:301`) keeps a holder only when the crossing is
  barred for this mover: the direction matches the side `from` stands on
  (entering: `from` on the post's side; leaving: `from` on the far side;
  both: either), and the mover does not know the pass fact
  (`knowsFact`, `world.go:480`). Allies and the holder pass as today.
- `DoorHeldFor` and the door verbs read the same two knobs, with the
  actor's own cell as the side it acts from.
- `ObservedHolds` names a door for a viewer only when the hold bars that
  viewer from where it stands now, so `Sighting.holds_door` stays the
  web's whole answer; the session reads it on every sighting read
  (`read.go:457`), so learning the pass or crossing sides refreshes it.
- dungeonspec: `holds: { door, at, against, pass }`; `against` is one of
  the three words; `pass` names a fact; an unknown word is refused.
**Content (in the mirror; T19 copies the hunk):** the castle stays both
ways. `gate-1`'s `posted` and `alert` posts gain `pass: gate-pass`. The
captain's `troupe` success and `greet` line teach `gate-pass` to their
witnesses (the lead's call; see Open items on `greet`).
**Castle routes:** the ten routes that leave through the gate (the cells,
companion, keep and gate tests that set `gateUnheld`) are rewritten to
earn the pass (`troupe` or `greet` before they leave) or to stand him
aside (cowed, turned), each as its story would; `castleHost.gateUnheld`
and its doc in `castle_king_test.go` are deleted.
**Tests:**
- `TestAPostHoldsBothWaysByDefault`.
- `TestAnEnteringPostLetsTheLeaversOut` and `TestALeavingPostLetsTheArrivalsIn`.
- `TestAPassFactWalksThroughAHold`: a mover who knows it crosses and opens
  the door; one who does not is barred; an ally needs none.
- `TestThePassIsTheMoversOwn`: a companion who did not witness the line is
  still barred.
- `TestADirectedPostStraddlingTwoRegionsIsRefused`.
- `TestTheDoorVerbsReadTheSettings`: from each side, with and without the pass.
- `TestObservedHoldsFollowTheSettings`: a viewer with the pass, and a
  viewer on the open side of a one-way post, see no `holds_door`.
- `TestAStoredPostKeepsItsSettings`; legacy record loads as both ways.
- dungeonspec: the knobs compile; an unknown direction is refused.
- Castle: a party that came in by the kitchen is barred at the gate on the
  way out; one that won the troupe line walks out.

- [ ] Tests first; implement; rewrite the routes; delete `gateUnheld`;
      `go test -race ./... && golangci-lint run ./...`; run record.

**Completion evidence:** PR link, encounter tag.

### Task 18 (S1c): the session re-pins

**Owner:** rpg-toolkit `rulebooks/dnd5e/session`.
**Prerequisites:** Task 17 tagged.
**Files:** `go.mod`; castle tests.
**Wire:** none. `holds_door` already names a door only when it bars the
viewer from where it stands (Task 17), so the web's planner needs no
direction field; a path that crosses a one-way opening twice is the one
case it cannot preview, and the server's stop answers it.
**Tests:** a castle test that the pass fact walks through and its
sighting drops `holds_door`; a test post set `against: leaving` lets an
arrival in and stops the same party walking back out.

- [ ] Re-pin; `go test -race ./... && golangci-lint run ./...`.

**Completion evidence:** PR link, session tag.

### Task 19 (S1c): the api serves the settings

**Owner:** rpg-api. **Prerequisites:** Task 18 tagged.
**Files:** `go.mod`; `content/the-mad-king.yaml` (Task 17's hunk, byte for
byte). **Tests:** `make test && make pre-commit`; the two castle files
diff empty. Stage on :3030, with the content shelf copied (Open items).

**Completion evidence:** PR link.

### Task 20 (S1c): the web, only if needed

**Owner:** rpg-dnd5e-web. **Prerequisites:** Task 18's finding.
None expected: `holds_door` carries the direction's effect per viewer. If
Task 18 finds a case it cannot carry, this task adds the protos field and
the planner reads it.

### Task 21 (S1c): the walk script follows the settings

**Owner:** rpg-project. **Prerequisites:** Task 19 staged.
**Files:** `.agents/skills/walk/SKILL.md`: the S1c rows below.

- [ ] Edit; `scripts/check-doc-genres.sh`; PR beside Task 19.

**Completion evidence:** PR link.

### Task 6 (S2): the bump is a beat the body hears

**Delivers:** D1–D3, D5 in the encounter.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** Task 11 tagged.
**Files:** `beats.go` (`BeatBarred`), `step.go` (the body stop), `field.go`
(`StepOutput.Stopped`), `wakebeats.go` (`beatBarred`), `machine.go`
(vocabulary, `MachineBeats`, `MachineBeatScoped`), `dungeonspec/states.go`
(doc), the castle mirror (content below); new `barred_test.go`; a castle
test.
**Interfaces:**
- `BeatBarred = "barred"`, payload `{"mover", "barrier", "cell"}`, written
  by `Step` once per step refused by a member, on its cell or on an edge
  its post holds (the barrier is then the nearest holder, H4); never by
  `stepTo`, and never for a wall, door, field or masked refusal. This is
  the keep's halt at the entrance.
- R7: a door verb refused by a hold is the bump too: it writes `barred`
  {actor, the nearest holder, the door's cell} and matches it on the
  holder as a step's bump does. A refused door verb also returns its
  output; the field is named when S2 is cut.
- `beatBarred machine.Beat = "barred"`, scoped. `Step` builds one
  targeted `wakeBeat{Beat: beatBarred, Member: barrier, Subject: mover,
  Cell: &cell}` and hands it to `matchOne` directly. It is never appended
  to `e.landed`, so no other landed beat is matched early
  (`wakebeats.go:34-38` holds for everything else). The barrier's pick runs
  through `react` and `runPick`, spends nothing, and its answer's voice
  lands `heard` on `e.landed` for the next raise.
- `StepOutput.Stopped *StepStop` with `Barrier MemberID`, `Cell
  spatial.Position`, `Reason string` (the public reason). A body stop
  returns `(*StepOutput, nil)` with `Stopped` set, no movement, and the
  beats, `IntelDeltas`, fight formed and `Outcome` the reaction produced.
  Walls, doors and fields still return `StepObstructedError`.
**Content (in the mirror; T9 copies the hunk):** `gate-1` `posted`: `{ on:
barred, to: challenged, do: [{ pick: barred }, { anchor: true }] }`;
`challenged` blocks, `{ on: barred, do: [{ pick: challenged }] }`, back to
`posted` after a few rounds; `gate-patrol` keys `barred: [{ say: "Whoa
there, buddy. Can I help you?" }]` and `challenged: [{ say: <shorter
line> }]`.
**Tests:**
- `TestABumpWritesBarredThenTheAnswer`: one `Step`; the record holds
  `barred` then the pick and `answered` with the line, in that order;
  `Stopped` names the guard.
- `TestABumpMatchesOnlyItsOwnBeat`: a clang landed earlier short of a pace
  is still unmatched after the bump.
- `TestBumpingAWallWritesNothingNew`; and a shut door; and a masked wall.
- `TestADrivenWalkWritesNoBarred`.
- `TestASecondBumpGetsTheChallengedRow`.
- `TestABumpWithNoListenerWritesOnlyBarred`.
- `TestABumpThatFormsAFightReturnsIt`: a barred row that teaches
  `party-recognized`; `Formed` and the intel deltas come back on the output.
- Castle (the mirror): walking into `gate-1` writes `barred` and his line.

- [ ] Tests first; implement; `go test -race ./... && golangci-lint run ./...`.

**Completion evidence:** PR link, encounter tag.

### Task 7 (S2): the session folds the bump

**Delivers:** D4, D5 and W1 at the session.
**Owner:** rpg-toolkit `rulebooks/dnd5e/session`.
**Prerequisites:** Task 6 tagged.
**Files:** `go.mod`; `move.go` (read `Stopped`); `act_bodies.go`
(`BarredAct{Mover, Barrier Party; Cell}`), `act_family.go` (decode
`barred`); tests.
**Interfaces:** `Move` treats `out.Stopped` as the end of the walk: it sets
`stopReason` from `Stopped.Reason`, folds the output's deltas, fight and
outcome as any step's, pays nothing for it, and returns.
**Tests:** a bump Move answers the walk, the `BarredAct`, then the table
answer with the line; `TestABumpOnTheTurnClockPaysNoMovement` (movement
left equals movement before the refused step, the steps before it paid); a
bump that forms a fight reports the fight; a legacy record without
`barred` folds as before.

- [ ] Re-pin; implement; `go test -race ./... && golangci-lint run ./...`.

**Completion evidence:** PR link, session tag.

### Task 8 (S2): the wire carries the bump

**Delivers:** W1 on the wire.
**Owner:** rpg-api-protos.
**Prerequisites:** none.
**Files:** `dnd5e/api/session/v1alpha2/act.proto`; generated Go and TS.
**Interfaces:** a `BarredAct { string mover; string barrier; Position cell; }`
in the act body oneof at the next free field number. Additive; `buf
breaking` passes.
**Tests:** none hand-written; `buf lint`, `buf breaking`.

- [ ] Edit, generate, lint, breaking check; PR; tag on merge.

**Completion evidence:** PR link, protos tag.

### Task 9 (S2): the api converts and the guard speaks

**Delivers:** W1 at the server; the castle's line.
**Owner:** rpg-api.
**Prerequisites:** Task 7 and Task 8 tagged.
**Files:** `go.mod`; `convert_act.go` (`barredToProto`); its test;
`content/the-mad-king.yaml` (Task 6's hunk, byte for byte).
**Tests:** convert test for the act; the two castle files diff empty;
`make test && make pre-commit`.

- [ ] Re-pin; implement; stage on :3030.

**Completion evidence:** PR link.

### Task 10 (S2): the web shows him speak

**Delivers:** W1 at the client.
**Owner:** rpg-dnd5e-web.
**Prerequisites:** Task 8 tagged; Task 9 on the walk env.
**Files:** `package.json`; `describeAct.ts` and `foldLogRows.ts` (the
barred row folds with the answer that follows it); the speech display that
shows a table answer's `say` at the speaker.
**Tests:** a barred act followed by its answer renders one row naming the
guard and his line; the line shows as speech at the guard, not a tooltip.

- [ ] Bump; implement; `npm run ci-check`.

**Completion evidence:** PR link, the walk on :3030.

## Walk checklists

The walk site is the castle on :3030 in free roam, the party walking in from
the start. The gesture is walking past the guard where he is the only
way: click a cell beyond the crossing (beneath the gate, in front of the
shut door), never his cell, which is approach to speak. Rows gated
on a die are covered by tests and not walked
(`.agents/skills/walk/SKILL.md:48-53`): standing aside needs Intimidation DC
14 and then the weight-3 row; turning needs Persuasion DC 15. Those rows
name their tests.

**S1**
- [ ] Walk past the guard where he is the only way (click beyond the crossing) → the walk stops one cell short of him, and the stop reason names him.
- [ ] Walk past the other gate guard (not posted) → the walk crosses her cell as before.
- [ ] Covered, not walked (`TestMadKingTheGuardHoldsTheGate`): cowed with the guard watching opens the crossing and he walks to his door post; a guard who did not witness it keeps blocking; turned opens it too.
- [ ] Covered, not walked (`TestMadKingScriptTheTroupeNeedsTheFeast`): the troupe let through, a won check, opens the crossing, and the party steps beneath the gate.
- [ ] The re-aimed sneak step (walk skill step 2): a voice draws `gate-1` off the crossing for a round, then cross on `gate-2`'s blind beat (`TestMadKingScriptASneakWindowAtTheDoor`).

**S1b** (`gate-1` posted; the gate door launches shut)
- [ ] OpenDoor on the gate from the cell beside the posted guard → refused, "Guard bars the door"; the door stays shut (R7).
- [ ] Hover the courtyard from the free cell beside him and from inside the gatehouse → no path goes through either cell of the opening.
- [ ] Covered, not walked (the door cannot be opened while he holds it, R7): a step through either cell of the open, held opening stops with "Guard bars the way" (`TestMadKingThePostHoldsTheGatesPassage`, which opens the door in the fixture). His cell is never clicked: it stays approach to speak.
- [ ] The gate still looks like one door: no extra leaves, no cut in the wall at its crossings.
- [ ] Walk past `gate-2` (not posted) → crosses as before.
- [ ] Drawn off, walked (a voice, not a die): with the gate held, speak to someone within his earshot (the pedlar on Kirk's walk; today any voice heard this round draws him) → `gate-1` walks off his post → OpenDoor on the gate now passes → hover the courtyard → the path through the gate appears → walk through; the next round he walks back and holds again. What draws a post is still R5's open content call, so the lure may change.
- [ ] Covered, not walked (`TestMadKingThePostHoldsTheGatesPassage` in rpg-toolkit#2263, `TestMadKingTheGuardHoldsTheGate`, `TestMadKingScriptTheTroupeNeedsTheFeast`; session `TestCastleThePostHoldsTheGatesOpening` in rpg-toolkit#2265): alert on his post, still held; cowed, turned or downed, the opening passes; the troupe let through cows him, and cowed he holds nothing; with a second holder on the same door, downing one does not open it (a test stands `gate-2` there with `castleHost.postedLike`; the castle posts only `gate-1`).

**S1c** (`gate-1` posted; the castle both ways with `gate-pass`)
- [ ] Greet the captain, then OpenDoor on the gate → it opens, and the walk crosses the opening; the guard says nothing (S2 adds his question for the barred).
- [ ] A fresh run, in by the kitchen: walk back out to the gate → stopped at the opening, "Guard bars the way"; OpenDoor refused.
- [ ] Covered, not walked: the troupe line (a check) teaches the pass; a one-way post (`against`) is a test post, since the castle stays both ways.

**S2**
- [ ] Walk past the guard where he is the only way → he says "Whoa there, buddy. Can I help you?" as speech at his cell, and the walk stops.
- [ ] Walk past him again → his shorter line.
- [ ] Walk into a wall or the shut door elsewhere → no line, no new beat.

Run records go in `rulebooks/dnd5e/knowledge/runs/<date>-<name>.md` in
rpg-toolkit, one per toolkit PR.

— cross-team agent, on behalf of KirkDiggler
