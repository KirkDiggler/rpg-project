# Round: intent in, record out

> The files this document names were removed from this folder when the idea's code left rpg-project. They read as written at [the commit that held them](https://github.com/KirkDiggler/rpg-project/tree/f24359d38617b4c9b30f833ddc621efcd94f6f08/ideas/unreal-client/spike); where each went is in the [spike's index](README.md#where-the-code-went).

A working plan for the spike's last round. Law: [../design.md](../design.md),
the rule **Intent in, record out (R1)** and **The same door (R1)**; O1 is
not decided here: the client's interaction model stays open, and this round
only proves the law with the smallest presentation that shows it.

## Goal and scope

The prototype sends verbs and renders what the record carries. Out of scope:
the interaction model (O1), Blueprint, the client's repository (O3), a
game mode or HUD worth keeping, audio.

## Finish line

Against the real stack, as `toolkit-sandbox-bard`, in a standalone game window
started from Unreal: a click on the floor walks the figure there along the
record's steps, a key ends the turn and the other figures' driven turns play
from the record, a click on an adjacent enemy swings. Frame times measured
over the burst an `EndTurn` causes. Evidence: automation tests green against
the engine's in-process server, the logs and numbers in the spike record, a
run record, the PR updated.

## Authority

design.md R1 (settled, KirkDiggler, 2026-11-23); the operator's go-ahead for
this round (2026-11-26).

## Constraints

- Every verb goes through `rpg-api-protos`' `SessionService`; nothing is
  computed client-side (design Law 1, 3).
- On the turn clock every verb carries a declaration `Afford` handed out;
  on the world clock `Move` carries none (acceptance_test.go:462, :496).
- Core stays plain C++: no Unreal type in `RpgWire/Public` beyond the export
  macro; blocking calls run off the game thread and marshal back with
  `AsyncTask(ENamedThreads::GameThread, …)`.
- Licensed content never enters a public repo; frames stay local.
- Every `.cpp` includes what it uses; `perfile_check.py` stays clean.

## Inspected baseline

- `rpg-api-protos` `dev`: `dnd5e/api/session/v1alpha1/service.proto` rpcs
  `Afford`, `Move`, `Attack`, `EndTurn`, `Exit`; requests at :206, :309,
  :523, :597. `types.proto:1390` `Declaration { verb, slot, available, id }`.
- `v1alpha2/record.proto`: `RecordEvent { id, at, act, beat, fields_json }`;
  beats as written ("moved", "struck", "turn-ended").
- `rpg-api` `internal/integration/session/acceptance_test.go:432-580`: the
  reference client, two clocks. `selectors_test.go:16` picks a declaration by
  verb, requiring `available`.
- Unreal project `~/UnrealEngine/Projects/RpgUeProof` at spike commit
  89c5a7e: `RpgSession.h` (138), `RpgPlayLabActor.cpp` (318).

## Sequence

Task 1 (core verbs, proven) → Task 2 (motion planner, pure) → Task 3 (play
lab moves and sends) → Task 4 (a window, click and key) → Task 5 (frame time
under the burst) → Task 6 (the record). Tasks 2 and 5 touch the lab module
after Task 3 lands; one writer, sequential.

## Open items

- The exact `fields_json` of `moved` and `struck` is established by
  measurement in Task 1 and becomes Task 2's fixture.
- Whether the bard's first turn holds declarations is established in Task 1;
  the tests assert what the law guarantees and log the rest.

---

### Task 1: Verbs in the core, proven against the engine's own server

**Delivers:** R1 "intent in, the same door": `Afford`, `Move`, `Attack`,
`EndTurn`, `Exit` as blocking core calls; the engine's refusals surfaced
verbatim.
**Owner:** Unreal plugin, module `RpgWire`; engine-side test in the spike's
`go-side/`.
**Prerequisites:** rpg-api worktree `.worktrees/ue-spike`; the Go server
serving on `127.0.0.1:50151`.
**Files:**
- Modify: `Source/RpgWire/Public/RpgSession.h`, `Private/RpgSession.cpp`
- Create: `Source/RpgWire/Private/Tests/SessionVerbsTest.cpp`
- Modify: `spike/go-side/zz_ue_spike_test.go` (seeds two players; adds a run
  with a monster adjacent; writes them to the info file)

**Interfaces (produced):**
```cpp
struct Declaration { std::string id; int verb = 0; int slot = 0; bool available = false; }; // verb: dnd5e…v1alpha1::Verb
struct Walk { std::vector<Atlas::Cell> steps; bool joined_combat = false; bool continues = false; std::string stop_reason; };
struct Swing { int roll = 0; int total = 0; int against = 0; bool hit = false; bool critical = false; int damage = 0; };
bool Afford(session, member, std::vector<Declaration>* out, std::string* error);
bool Move(session, member, const std::vector<Atlas::Cell>& path, const std::string& declaration_id, Walk* out, std::string* error);
bool Attack(session, attacker, target, declaration_id, Swing* out, std::string* error);
bool EndTurn(session, member, declaration_id, std::string* next, bool* round_wrapped, std::string* error);
bool Exit(session, member, std::string* error);
```
`declaration_id` empty sends none. `error` is the engine's status message.

**Behavior:** each call blocks; false with the engine's message when refused.

**Tests** (automation, names under `RpgWire.Session.`; env `RPGWIRE_TEST_ADDR`,
`RPGWIRE_TEST_INFO`; a test fails with a clear message when either is unset):
- `FreshGameFreesASeatHeldElsewhere`: StartGame twice as `player-ue-verbs`;
  the second is refused quoting `seated in session`; `Exit(that)`; StartGame
  succeeds.
- `AffordListsDeclarationsWithIds`: after a fresh game, `Afford` succeeds,
  ≥1 declaration, every `available` one has a non-empty id; all are logged.
- `MoveOnTheTurnClockUsesTheDeclaration`: with the `VERB_MOVE` declaration,
  a path of one neighbour that is in the atlas's cells → `Walk.steps` ≥ 1 and
  `steps[0] == path[0]`.
- `MoveWithoutDeclarationOnTheTurnClockIsRefused`: false, message logged.
- `EndTurnHandsTheTurnOn`: `VERB_END_TURN` declaration → `next` non-empty.
- `AttackOnAFarTargetIsRefused`: castle run, a roster monster far away →
  false, the engine's message logged.
- `AttackOnAnAdjacentMonsterIsResolvedByTheEngine`: the tomb run from the
  info file → `Swing.against > 0`, roll and total logged.
- After the move and the attack: print every beat the record holds for those
  acts (`GetRecord` via `RecordStream`'s catch-up is enough) — the captured
  `moved` and `struck` JSON is Task 2's fixture and goes in the evidence log.

- [ ] Extend the Go test; start the server; confirm it serves the two runs.
- [ ] Write the tests; build; run: they fail to compile (no such methods).
- [ ] Implement the six calls; build; run: each fails on its assertion or
      passes with the observation logged; tighten to what the law guarantees.
- [ ] `perfile_check.py` clean; commit.

**Verification:** `UnrealEditor-Cmd $P/RpgUeProof.uproject -ExecCmds="Automation RunTests RpgWire" -TestExit="Automation Test Queue Empty" -ReportExportPath=$P/Saved/Automation -nullrhi -unattended -NoSound -stdout -FullStdOutLogOutput` → every `RpgWire.Session.` test `Success`, none `Fail`.
**Completion evidence:** the automation report plus the log of declarations, refusals, roll and beats, copied to `spike/unreal/evidence-2026-11-26-verbs.log`.

### Task 2: Motion from a beat, and a hex line — pure functions

**Delivers:** the two decisions a record-driven client makes, testable
without a world: what a beat means for a figure, and which cells a walk
takes.
**Owner:** module `RpgAssetLab`.
**Prerequisites:** Task 1's captured beats.
**Files:** Create `RpgAssetLab/Private/RpgMotion.h`, `RpgMotion.cpp`,
`Private/Tests/RpgMotionTest.cpp`; Build.cs unchanged.

**Interfaces (produced):**
```cpp
struct FMotionStep { enum class Kind { Walk, Swing, Turn }; Kind kind; FString member; FString target; FIntPoint from; FIntPoint to; };
// false when the beat moves nobody; appends zero or more steps otherwise.
bool PlanMotion(const FString& beat, const FString& fields_json, TArray<FMotionStep>& out);
TArray<FIntPoint> HexLine(FIntPoint from, FIntPoint to); // axial (q, r), cube lerp, includes both ends
```
**Tests** (`RpgAssetLab.Motion.`, fixtures = Task 1's captured JSON):
- `AMovedBeatWalksItsMemberFromTheOldCellToTheNew` — one Walk step, member
  and cells as captured.
- `AStruckBeatMakesTheAttackerSwingAtTheTarget` — one Swing step.
- `ATurnEndedBeatHandsTheTurnToNext` — one Turn step.
- `ABeatThatMovesNobodyPlansNothing` — `joined`, `tick`, malformed JSON → false.
- `HexLineOfOneStepIsBothEnds`, `HexLineIsContiguousAndEndsWhereAsked` — every
  pair of neighbours in the line is adjacent; first and last match.

- [ ] Write the tests with the fixtures; build; run: compile failure.
- [ ] Implement; build; run: green. - [ ] Commit.

**Verification:** the automation command with filter `RpgAssetLab.Motion`.
**Completion evidence:** report; fixtures quoted in the test file.

### Task 3: The play lab sends verbs and moves figures from the record

**Delivers:** `rpg.verb …` through the core; figures animate from beats.
**Owner:** `RpgPlayLabActor`.
**Prerequisites:** Tasks 1, 2.
**Files:** Modify `RpgPlayLabActor.h/.cpp`.
**Interfaces (consumed):** Task 1 calls; Task 2 `PlanMotion`, `HexLine`.
**Interfaces (produced):** console command `rpg.verb <afford|move <q> <r>|attack <target>|end>`;
  the actor ticks and drains each figure's queue: Walk over `kWalkSeconds`
  (0.4) per cell, Swing turns to the target and lunges over `kSwingSeconds`
  (0.5), Turn moves a marker above `next`. Tracks each figure's cell from
  `joined_at` and `moved`.
**Behavior:** a verb runs off the game thread; its response is logged; motion
follows the record only, so a monster's walk and the player's look the same.
Failure: a refused verb logs the engine's message and plans nothing.
**Tests:** the integration scenario is the proof (the functions are thin
over Tasks 1 and 2): in a `-game -RenderOffscreen` run, `rpg.play`, then
`rpg.verb move` to a neighbour, `rpg.verb end`: the log shows the figure at
the old cell when the beat arrives and at the new cell `kWalkSeconds` later,
the turn marker on the next member, and every beat handled counted.

- [ ] Run the scenario before the change: no `rpg.verb`, figures never move
      after placement (the red).
- [ ] Implement; build; run the scenario: green as defined above; two
      evidence frames (mid-walk, at rest) saved locally. - [ ] Commit.

**Verification:** the run's log lines `VERB`, `MOTION`, `MARKER`, `BEATS`.
**Completion evidence:** `evidence-2026-11-26-motion.log`, frames.

### Task 4: A window, a click and a key

**Delivers:** the same verbs from input, in a standalone game.
**Owner:** `RpgPlayLabActor`; project `DefaultEngine.ini` (game mode).
**Prerequisites:** Task 3.
**Files:** Modify `RpgPlayLabActor.h/.cpp`.
**Interfaces (produced):** in a game world the lab binds, through the first
  `APlayerController`, `LeftMouseButton` and `E` (pressed). A click that hits
  a figure sends Attack on it; one that hits the floor sends Move along
  `HexLine(current, clicked cell)`, with the declaration `Afford` gives;
  `E` sends EndTurn. The controller views the scene's angled camera, shows
  the cursor, and the default pawn is hidden. `rpg.click <sx> <sy>` injects
  a click at screen coordinates through the same path.
**Behavior:** a click with no current cell logs and does nothing; a refused
verb is Task 3's failure case.
**Tests:** integration: `rpg.click` at the projected centre of a neighbour
cell → a Move; at a figure → an Attack (refused if far, with the message);
`E` injected → EndTurn.

- [ ] Run in `-game -windowed` on the desktop before the change: clicks do
      nothing (the red).
- [ ] Implement; build; run with injections: green; one frame with the
      cursor visible. - [ ] Commit.

**Verification:** log lines `INPUT`, then Task 3's.
**Completion evidence:** `evidence-2026-11-26-input.log`, frame.

### Task 5: Frame time under a burst

**Delivers:** the cost, measured.
**Owner:** `RpgPlayLabActor`.
**Prerequisites:** Task 3.
**Files:** Modify `RpgPlayLabActor.cpp`.
**Interfaces (produced):** on each Tick the game-thread delta is sampled; an
`EndTurn` opens a 3 s window; at its end one line: `FRAMES burst: beats N,
frames M, mean A ms, p99 B ms, max C ms, longest gap D ms`, and the same for
a 3 s idle window before it as the control.
**Tests:** integration: the two lines exist in the run of Task 3's scenario
and the idle line's max is below the burst's.

- [ ] Run the scenario before the change: no `FRAMES` lines (the red).
- [ ] Implement; build; run: green. - [ ] Commit.

**Verification:** `FRAMES` lines. **Completion evidence:** in the motion log.

### Task 6: The record

**Delivers:** the round's evidence and its reading.
**Files:** `spike/unreal/README.md` (leg 6), `knowledge/runs/2026-11-26-unreal-client-verbs.md` (four sections), synced sources and evidence logs under `spike/unreal/`.
- [ ] Write; `scripts/check-doc-genres.sh`; commit; push; comment on the
      design PR with the numbers and what remains open.

## Visible plan checks

### Requirement coverage

| In-scope requirement | Task(s) | Concrete proof |
|---|---|---|
| Intent in: verbs through the contract | 1, 3, 4 | Task 1's automation tests; Task 3/4 log lines `VERB` |
| Record out: motion from beats only | 2, 3 | Task 2's fixtures-to-steps tests; Task 3 `MOTION` timings |
| The same door: declarations, refusals verbatim | 1 | `…IsRefused` tests logging the engine's message |
| Frame time under the burst | 5 | `FRAMES` lines, burst against idle |
| Evidence in the record | 6 | run record, leg 6, logs |

Explicit deferral: O1 (the interaction model), by the design.

### Provider/consumer seams

| Provider | Consumer | Contract | Availability | Proof |
|---|---|---|---|---|
| Task 1 `rpgwire::Session` | Tasks 3, 4 | the six calls above | after Task 1 | automation tests, `VERB` lines |
| Task 1 captured beats | Task 2 | `moved`, `struck`, `turn-ended` JSON | after Task 1 | fixtures in the test |
| Task 2 `PlanMotion`, `HexLine` | Tasks 3, 4 | signatures above | after Task 2 | Task 2 tests, `MOTION` lines |
| `rpg-api` in-process server | Task 1 | gRPC on `127.0.0.1:50151`, Dev scheme | started before Task 1 | the tests connect |
| docker stack `local/ue-play` | Tasks 3–5 | gRPC on the api container | up | `rpg.play` builds the scene |
