---
name: The guard holds the gate — implementation plan
status: PLAN derived 2026-11-03 from design.md R1–R3 (all ruled on rpg-project#835); checked against the inspected baseline below.
authority: design.md (Law, R1–R3), story rpg-project#836, lane rpg-project#835
planning guide: ../../.agents/skills/design/planning.md
---

# The guard holds the gate — implementation plan

## Goal and scope

A creature's state says whether its body blocks. A posted gate guard holds
the castle's crossing, a bump on him writes a `barred` beat he answers in
his own voice, and a cowed or turned guard opens the cell at once and walks
off it on his next beat.

**Non-goals.** A barred or locked gate door. A permit on movement.
Conditions that stop a walk. A bump that costs anything. The session
playing a dialogue line. `Prevented` in resolution.

**Repositories and order.** Develop outside-in, merge inside-out. Toolkit:
one module per PR. `dungeonspec` is a package of the encounter module.

| slice | repo / module | base | PR shape |
|---|---|---|---|
| S1 | rpg-toolkit `encounter` → `session` re-pin; rpg-api | main / main / dev | encounter PR; session re-pin PR; api PR (pin + content) |
| S2 | rpg-toolkit `encounter` → `session`; rpg-api-protos; rpg-api; rpg-dnd5e-web | main / main / main / dev / dev | encounter PR; session PR; protos PR; api PR (pin, convert, content); web PR |

S1 first. S2 after S1's encounter tag (it bumps into a posted body).

## Finish line

- Sub-issues for S1 and S2 closed by their "done means"; each slice walked
  by the operator on :3030; #836's slices table current.
- On the castle, free roam: the party's walk to the courtyard stops at the
  gate guard, who says "Whoa there, buddy. Can I help you?"; a second bump
  gets his shorter line; the captain turns at the voice; after the captain
  is cowed in the guard's sight, the next step passes and the guard walks
  to his door post.

## Requirement labels

| label | requirement | source |
|---|---|---|
| B1 | A member with states blocks exactly while its current state is one its states mark `blocks`; a member with no states blocks as `BlocksMovement` says. | R1, Law |
| B2 | The blocking answer is read from the state on every fold; a transition opens or closes the cell in the beat it fires. | R1, R3 |
| B3 | A hostile body blocks whatever its state; downed, ally and pass-through rows unchanged. | Law |
| B4 | A stored encounter loads its members' current states and so their blocking; a record from before the change loads as today. | R1 |
| B5 | What an observer holds about a body's blocking follows the state, so routes planned from observation agree with the step. | Finding |
| B6 | A step refused by a body returns its own sentinel; its public reason names the body the mover observes, else "someone"; a driven walk still stops silently. | Law |
| C1 | The dialect accepts `blocks: true` on a state and refuses it on a terminal state. | R1 |
| D1 | A step refused by a body (never a wall, door or field) writes a `barred` beat {mover, barrier, cell} before the walk stops, every time. | R2 |
| D2 | `barred` is a machine beat; it lands on the barrier when it listens; it is matched in the Move that wrote it, spends nothing, asks nobody the time. | R2 |
| D3 | The barrier's `pick` on `barred` writes an `answered` beat whose `say` is a voice its hearers hear. | R2 |
| D4 | A bump on the turn clock pays no movement; the steps already taken stay. | Law |
| W1 | The session decodes `barred` into a `BarredAct`; the wire carries it additively; the api converts it; the web shows the barrier's answer as his speech at his cell. | R2 |
| G1 | castle tests, `step_test.go`, `blocksmovement_test.go`, `wakebeats_internal_test.go`, `machine_internal_test.go`, session `move` tests keep passing. | must keep working |

## Constraints

- Toolkit: `go test -race ./...` and `golangci-lint run ./...` per module;
  no `go.work`, no local `replace` in a PR; consumers pin by commit, then by
  tag minted on merge.
- `mind/machine` is not changed; `blocks` is the encounter's word.
- Protos: deprecate, never break (`buf breaking` against main).
- rpg-api: `make test`, `make pre-commit`. Web: `npm run ci-check`.
- Content is castle QA content: it goes into `the-mad-king.yaml`, never a
  one-off site.
- Team signature only; one review round per slice; the slice staged on
  :3030 for the operator's walk before merge-out.

## Inspected baseline

| repo | revision | symbols |
|---|---|---|
| rpg-toolkit | origin/main 20a124ad (encounter v0.174.0, session v0.161.0) | `encounter/cellfacts.go`: `Passage` :12-27, `CellAt` doc :115-135, member contributor :303-330, `blockedBy` :478-503, `occupantPassage` :528-540. `encounter/encounter.go`: member creation :992, :1015, record load :1261, `join` :2601-2632, `memberEntity.BlocksMovement` :3048-3050. `encounter/board.go`: :167, :175. `encounter/observedpassage.go`: :43. `encounter/step.go`: `stepMember` :218-410, last-step pass-through :334-336, body refusal :347-352, `stepTo` silent list :440-452. `encounter/walkend.go`: `StepObstructedError.PublicReason` :35-48. `encounter/machine.go`: `machineVocabulary` :92, `vocabularyFor` :117, `MachineBeats` :703, `MachineBeatScoped` :726, `ValidateMachine` :751. `encounter/wakebeats.go`: beat words :57-80, `landHeard` :186, `landFactBeat` :212. `encounter/worldtime.go`: `wake` :401, `wakePass` :420. `encounter/machine.go`: `runPick` :516. `encounter/answer.go`: voice :239-250. `encounter/dungeonspec/states.go`: `StatesSpec`, `StateSpec` :63-67, `ValidateMachine` call :319. `session/move.go`: obstruction :455-461. `session/act_family.go`: `answeredPiece` :725-765. `session/act_bodies.go`: `TableAnswer` :394-415. Tests: `encounter/blocksmovement_test.go` `TestBlocksMovementSuite` :27; `castletest/castle_king_script_test.go` cells :53-98, `TestMadKingScriptASneakWindowAtTheDoor` :453, `TestMadKingScriptOpeningTheGateBringsTheFarRoom` :670. |
| rpg-api-protos | origin/main 22c45cf (v0.1.253) | `dnd5e/api/session/v1alpha2/act.proto`: act body oneof (`table_answer = 34` :104) |
| rpg-api | origin/dev 6cb2d92 | `content/the-mad-king.yaml`: gate tables :272-327, dispositions :524-531, gate guards :1544-1565, `gate-1` states :1681-1698, stand-aside :1723-1737, gate door :2012-2015; `internal/handlers/dnd5e/session/v1alpha2/convert_act.go` |
| rpg-dnd5e-web | origin/dev 1334dcd5 | `src/components/session/combat-experience/describeAct.ts` `tableAnswer` :478, :844; `foldLogRows.ts` :32-41 |

## Sequence

1. S1: T1 (encounter) → tag → T2 (session re-pin) → tag → T3 (api pin +
   content) → stage on :3030 → walk → merge out.
2. S2: T4 (encounter, on T1's tag) → tag → T5 (session) → tag. T6 (protos)
   in parallel → tag. T7 (api, on the session and protos tags) → T8 (web,
   on the protos tag) → stage → walk → merge out.

## Open items

- Resolved: the state binding is the encounter's (`MemberInput.BlockingStates`),
  compiled from the dialect's `blocks: true`; `mind/machine` is untouched
  (design, Findings).
- Resolved: blocking is derived from the current state on read, never
  written beside it, so no transition can forget to set it.
- Resolved: the guard's line rides the existing `answered` beat and table
  answer; S2's wire addition is the `barred` act only.
- Resolved: `barred` lands on the barrier; bystanders hear his voice
  (design, Assumptions). The bump landing on bystanders waits for a veto.
- Open, decided on T3's PR and walked: the sneak window against a post on
  the crossing (design, Open). Default: the crossing is one post of
  `gate-1`'s loop.

## Task contracts

### Task 1 (S1): a body blocks by its state

**Delivers:** B1–B6, C1.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter` (with `dungeonspec`).
**Prerequisites:** none.
**Files:** modify `encounter.go` (`MemberInput.BlockingStates`, the member
record, `memberEntity`), `cellfacts.go` (member contributor reads the
encounter's answer), `board.go`, `observedpassage.go` and the holding
refresh, `step.go` (body refusal, `stepTo` silent list), `walkend.go`
(public reason), `errors.go` (`ErrBodyInTheWay`), `machine.go` (validate
`BlockingStates` against the recipe); `dungeonspec/states.go`
(`StateSpec.Blocks`); new `postedbody_test.go`; update
`blocksmovement_test.go`, `dungeonspec` tests, castle tests.
**Interfaces:**
- `MemberInput.BlockingStates []string`: the states in which the body
  blocks. Refused when it names a state the recipe lacks, or when it is set
  with no `Machine` (fail closed: a blocking state with no machine is a
  bug).
- `func (e *Encounter) memberBlocks(id MemberID) bool`: the member's current
  machine state is in `BlockingStates`, else `BlocksMovement` when it has no
  machine. Every reader of a member's blocking calls it: the contributor,
  the board, the observed holding.
- `StateSpec.Blocks bool yaml:"blocks,omitempty"`; the compiler collects
  the blocking states into `BlockingStates`; `blocks` on a terminal state
  is refused.
- `ErrBodyInTheWay`; a body-blocked step's `StepObstructedError` wraps it,
  and `PublicReason()` is `"<name> stands in the way"` for a body the mover
  observes and `"someone stands in the way"` otherwise. `stepTo` treats
  `ErrBodyInTheWay` as a silent stop.
**Behavior:** the fold is unchanged. A transition that enters or leaves a
blocking state refreshes the observed holding of that body for every
observer that sees it. Stored encounters carry the machine state already;
nothing new is stored.
**Tests:**
- `TestAPostedNeutralBodyBlocksAStepAndARoute`: a neutral member in a
  blocking state; a step onto its cell is refused with `ErrBodyInTheWay`; a
  route to the far side goes round or is not found.
- `TestLeavingThePostOpensTheCellInThatBeat`: a `fact` transition to a
  non-blocking state; the next step passes in the same raise.
- `TestAHostileBodyBlocksInAnyState`: a non-blocking state, hostile stance → Blocked.
- `TestAMemberWithoutStatesBlocksAsDeclared`: `BlocksMovement` true and false, as today.
- `TestAStoredEncounterLoadsItsBlocking`: save in a blocking state, load, step refused.
- `TestBlockingStatesAreRefusedWithoutAMachine` and for an unknown state.
- `TestABodyRefusalNamesTheBody`: observed → its name; unobserved → "someone".
- `TestADrivenWalkStopsSilentlyAtABody`.
- dungeonspec: `blocks: true` compiles; `blocks` on a terminal state is refused.
- Castle (`castletest`, on T3's content through the castle fixture): the
  party's walk to the courtyard stops one cell short of the crossing with
  the guard's name; after `gate-cowed` is taught to the guard the next step
  passes; `TestMadKingScriptASneakWindowAtTheDoor` keeps passing on the
  loop that keeps the window (or is rewritten per T3's ruling).

- [ ] Add the tests; see them fail.
- [ ] Implement the binding, the read, the refusal, the dialect.
- [ ] `go test -race ./... && golangci-lint run ./...` in `rulebooks/dnd5e/encounter`.

**Verification:** the commands above, green.
**Completion evidence:** PR link, the encounter tag, test names green.

### Task 2 (S1): the session re-pins

**Delivers:** B6 at the session (the new reason reaches `stop_reason`).
**Owner:** rpg-toolkit `rulebooks/dnd5e/session`.
**Prerequisites:** Task 1 tagged.
**Files:** `go.mod`; `read.go` (map `ErrBodyInTheWay` to the obstruction it
is, beside `ErrDoorShut`); `move` tests.
**Tests:** a Move into a posted body answers `stopped` with the guard's
name in `stop_reason` and the steps before it kept.

- [ ] Re-pin; implement; `go test -race ./... && golangci-lint run ./...`.

**Completion evidence:** PR link, session tag.

### Task 3 (S1): the gate is held

**Delivers:** B1 on the castle; R3's step aside as content.
**Owner:** rpg-api.
**Prerequisites:** Task 2 tagged.
**Files:** `go.mod`; `content/the-mad-king.yaml`.
**Content:**
- `gate-1` states: `posted` (`blocks: true`) holds the cell beneath the
  gate; `cowed` and `turned` do not block. `posted` → `cowed` on
  `fact` when `fact:gate-cowed`; `posted` and `alert` → `turned` on `fact`
  when `fact:guards-turned`; `alert` blocks too when he stands the post.
- `gate-patrol`: the loop includes the crossing post (default: one post of
  the loop, per the design's Open); a `cowed` key walks him to the west door
  post and stands; `guards-turned` keeps its march.
- The captain's stand-aside line unchanged.
**Tests:** `make test`; the castle tests in the toolkit read this file
through their fixture, so the content and T1's castle tests land together.

- [ ] Re-pin; edit; `make test && make pre-commit`; stage on :3030.

**Completion evidence:** PR link, the walk on :3030.

### Task 4 (S2): the bump is a beat the body hears

**Delivers:** D1–D4.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** Task 1 tagged.
**Files:** `beats.go` (`BeatBarred`), `step.go` (write it on a body
refusal), `wakebeats.go` (`beatBarred`, `landBarred`), `machine.go`
(vocabulary, `MachineBeats`, `MachineBeatScoped`), `worldtime.go` (the
non-raising match after a Move's bump), `dungeonspec/states.go` (doc);
new `barred_test.go`.
**Interfaces:**
- `BeatBarred = "barred"`, payload `{"mover", "barrier", "cell"}`, written
  once per refused step whose blocking contributor is a member; never for a
  wall, door, field or masked refusal.
- `beatBarred machine.Beat = "barred"`: scoped; `landBarred` lands it on
  the barrier, when it listens, with the cell and distance zero.
- After the refusal the Move matches what landed with the pass that raises
  nothing (`wakePass(false, false)`), so the barrier's pick runs as a
  reaction in the same Move and writes its `answered` beat and voice.
**Behavior:** a bump with no listener writes `barred` and nothing else. A
repeated bump writes `barred` again. On the turn clock the refused step is
not paid.
**Tests:**
- `TestABumpWritesBarredThenTheAnswer`: a posted guard with `on: barred`
  picking a `say` row; one Move; the record holds `barred` then `answered`
  with the line, in that order.
- `TestBumpingAWallWritesNothingNew`; and a shut door; and a masked wall.
- `TestABumpOnTheTurnClockPaysNoMovement`: movement left equals movement
  before the refused step.
- `TestTheGuardsAnswerWakesTheCaptain`: the captain's `heard within N`
  transition fires on the guard's voice in the same Move.
- `TestASecondBumpGetsTheChallengedRow`: `posted` → `challenged` on the
  first `barred`; the second `barred` picks the other key.
- `TestABumpWithNoListenerLandsNothing`.

- [ ] Tests first; implement; `go test -race ./... && golangci-lint run ./...`.

**Completion evidence:** PR link, encounter tag.

### Task 5 (S2): the session folds the bump

**Delivers:** W1 at the session.
**Owner:** rpg-toolkit `rulebooks/dnd5e/session`.
**Prerequisites:** Task 4 tagged.
**Files:** `go.mod`; `act_bodies.go` (`BarredAct{Mover, Barrier Party; Cell}`),
`act_family.go` (decode `barred`); the projection rule beside the walk's;
tests.
**Tests:** a bump Move answers the walk, the `BarredAct`, then the table
answer with the line; a legacy record without `barred` folds as before.

- [ ] Re-pin; implement; `go test -race ./... && golangci-lint run ./...`.

**Completion evidence:** PR link, session tag.

### Task 6 (S2): the wire carries the bump

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

### Task 7 (S2): the api converts and the guard speaks

**Delivers:** W1 at the server; the castle's line.
**Owner:** rpg-api.
**Prerequisites:** Task 5 and Task 6 tagged.
**Files:** `go.mod`; `convert_act.go` (`barredToProto`); its test;
`content/the-mad-king.yaml`.
**Content:** `gate-1` `posted`: `{ on: barred, to: challenged, do: [{ pick:
barred }, { anchor: true }] }`; `challenged` blocks, `{ on: barred, do: [{
pick: challenged }] }`, back to `posted` after a few rounds; `gate-patrol`
keys `barred: [{ say: "Whoa there, buddy. Can I help you?" }]` and
`challenged: [{ say: <shorter line> }]`.
**Tests:** convert test for the act; `make test && make pre-commit`.

- [ ] Re-pin; implement; stage on :3030.

**Completion evidence:** PR link.

### Task 8 (S2): the web shows him speak

**Delivers:** W1 at the client.
**Owner:** rpg-dnd5e-web.
**Prerequisites:** Task 6 tagged; Task 7 on the walk env.
**Files:** `package.json`; `describeAct.ts` and `foldLogRows.ts` (the
barred row folds with the answer that follows it); the speech display that
shows a table answer's `say` at the speaker.
**Tests:** a barred act followed by its answer renders one row naming the
guard and his line; the line shows as speech at the guard, not a tooltip.

- [ ] Bump; implement; `npm run ci-check`.

**Completion evidence:** PR link, the walk on :3030.

## Walk checklists

The walk site is the castle on :3030 in free roam, the party walking in from
the start. Rows gated on a die (the stand-aside check's weight-1 "GUARDS!"
answer, a failed check) are covered by tests and not walked.

**S1**
- [ ] Walk to the courtyard → the walk stops in front of the guard on the crossing, and the stop reason names him.
- [ ] Walk around him → no route through; the gate door cannot be reached.
- [ ] Speak to the captain, stand aside, and win with the guard watching → the next walk through the crossing passes, and the guard walks to his door post on his next beat.
- [ ] A fresh run, the captain turned (`guards-turned`) → the crossing opens and the guard marches with the captain.
- [ ] The sneak window, as T3 rules it → walked or retired, and the PR says which.

**S2**
- [ ] Walk into the guard → he says "Whoa there, buddy. Can I help you?" as speech at his cell.
- [ ] Walk into him again → his shorter line.
- [ ] The captain turns toward the voice.
- [ ] Walk into a wall or the shut door elsewhere → no line, no new beat.

Run records go in `rulebooks/dnd5e/knowledge/runs/<date>-<name>.md` in
rpg-toolkit, one per toolkit PR.

— cross-team agent, on behalf of KirkDiggler
