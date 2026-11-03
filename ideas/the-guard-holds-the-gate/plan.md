---
name: The guard holds the gate — implementation plan
status: PLAN derived 2026-11-03 from design.md R1–R4 (all ruled on rpg-project#835); checked against the inspected baseline below; P1 amended 2026-11-03 on rpg-project#841.
authority: design.md (Law, R1–R4), story rpg-project#836, lane rpg-project#835
planning guide: ../../.agents/skills/design/planning.md
---

# The guard holds the gate — implementation plan

## Goal and scope

A creature's state says whether its body blocks. A posted gate guard holds
the castle's crossing; the player walks into him and is stopped by name; a
bump writes a `barred` beat he answers in his own voice; a cowed or turned
guard opens the cell at once and walks off it on his next beat.

**Non-goals.** A barred or locked gate door. A permit on movement.
Conditions that stop a walk. A bump that costs anything. The session
playing a dialogue line. `Prevented` in resolution. The captain reacting to
the guard's voice. A walk-env way to teach a fact.

**Repositories and order.** Develop outside-in, merge inside-out. Toolkit:
one module per PR. `dungeonspec` is a package of the encounter module.

| slice | repo / module | base | PR shape |
|---|---|---|---|
| S1 | rpg-toolkit `encounter` → `session` re-pin; rpg-api; rpg-dnd5e-web; rpg-project | main / main / dev / dev / main | encounter PR (code, castle mirror, castle tests); session re-pin PR; api PR (pin + the same content hunk); web PR (planner); walk skill PR |
| S2 | rpg-toolkit `encounter` → `session`; rpg-api-protos; rpg-api; rpg-dnd5e-web | main / main / main / dev / dev | encounter PR (code, castle mirror, castle test); session PR; protos PR; api PR (pin, convert, the same content hunk); web PR |

S1 first. S2 after S1's encounter tag (it bumps into a posted body).

**The castle file lives twice.** rpg-api's `content/the-mad-king.yaml` and
the toolkit's `encounter/dungeonspec/testdata/world-builder-v4-castle.yaml`
stay byte-identical. The encounter task edits the mirror with its castle
tests; the api task copies the same hunk. Neither waits on the other's
merge.

## Finish line

- Sub-issues for S1 and S2 closed by their "done means"; each slice walked
  by the operator on :3030; #836's slices table current.
- On the castle, free roam: walking past the gate guard where he is the
  only way, the crossing in front of the shut gate door, stops the walk
  with his name; with S2 he
  says "Whoa there, buddy. Can I help you?", and a second bump gets his
  shorter line. A cowed or turned guard opens the crossing and walks to his
  door post (proved by castle tests; the dice are not walked).

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
| rpg-api-protos | origin/main 22c45cf (v0.1.253) | `dnd5e/api/session/v1alpha2/act.proto`: act body oneof (`table_answer = 34` :104) |
| rpg-api | origin/dev 6cb2d92 | `content/the-mad-king.yaml`: gate tables :272-327, dispositions :524-531, gate guards :1544-1565, `gate-1` states :1681-1698, captain :1705-1707, stand-aside :1723-1737, turn :1738-1747, gate door :2012-2015; `internal/handlers/dnd5e/session/v1alpha2/convert_act.go` |
| rpg-dnd5e-web | origin/dev 1334dcd5 | `src/components/session/occupantPassages.ts` `indexOccupantPassages` :6, BLOCKED :14; `src/components/session/atlasPath.ts` `edgePassable` :163-175, `findAtlasPath` :187; `src/components/session/stanceRing.ts` :47, `partySide.ts` :39; `src/components/session/combat-experience/describeAct.ts` `tableAnswer` :478, :844; `foldLogRows.ts` :32-41 |
| rpg-project | origin/main 2a666a2 | `.agents/skills/walk/SKILL.md`: castle steps 2 and 5 :17, :20; dice rule :25-30 |

## Sequence

1. S1: T1 (encounter, with the castle mirror and its tests) → tag → T2
   (session re-pin) → tag → T3 (api pin + content hunk). T4 (web planner)
   in parallel with T1. T5 (walk skill) with T3. Stage T3 + T4 on :3030 →
   walk → merge out.
2. S2: T6 (encounter, on T1's tag, with the mirror) → tag → T7 (session) →
   tag. T8 (protos) in parallel → tag. T9 (api, on the session and protos
   tags) → T10 (web, on the protos tag) → stage → walk → merge out.

## Open items

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
- Open (Task 1, for the rpg-toolkit#2258 review to keep or drop):
  `ValidateWalkEnd` lets a walk aim at a seen, non-hostile blocking body.
  The builder added it so the gesture reached the step, but the #841 call
  removed its caller: no walk aims at his cell now, and the session checks
  only the walk's last cell (`session/move.go:172`). A hostile or
  pass-through goal is still refused either way.
- Open (Task 1): the roster row's `Member.BlocksMovement` reports the
  state, not the declaration, so every reader gives one answer (B2). It
  lands in rpg-toolkit#2258 or in Task 2, as the #2258 review decides.

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

### Task 6 (S2): the bump is a beat the body hears

**Delivers:** D1–D3, D5 in the encounter.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** Task 1 tagged.
**Files:** `beats.go` (`BeatBarred`), `step.go` (the body stop), `field.go`
(`StepOutput.Stopped`), `wakebeats.go` (`beatBarred`), `machine.go`
(vocabulary, `MachineBeats`, `MachineBeatScoped`), `dungeonspec/states.go`
(doc), the castle mirror (content below); new `barred_test.go`; a castle
test.
**Interfaces:**
- `BeatBarred = "barred"`, payload `{"mover", "barrier", "cell"}`, written
  by `Step` once per step refused by a member; never by `stepTo`, and never
  for a wall, door, field or masked refusal.
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

**S2**
- [ ] Walk past the guard where he is the only way → he says "Whoa there, buddy. Can I help you?" as speech at his cell, and the walk stops.
- [ ] Walk past him again → his shorter line.
- [ ] Walk into a wall or the shut door elsewhere → no line, no new beat.

Run records go in `rulebooks/dnd5e/knowledge/runs/<date>-<name>.md` in
rpg-toolkit, one per toolkit PR.

— cross-team agent, on behalf of KirkDiggler
