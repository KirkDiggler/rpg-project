---
name: When things fire — implementation plan
status: PLAN derived 2026-11-02 from design.md R1–R3 (all ruled on rpg-project#826); checked against the inspected baseline below.
authority: design.md (Law, R1–R3), story rpg-project#825, lane rpg-project#826
planning guide: ../../.agents/skills/design/planning.md
---

# When things fire — implementation plan

## Goal and scope

Every moved beat names its kind (`own`, `compelled`, `displaced`) from the
encounter to the web. Provoking and facing are read from the kind. `forced`
stays as a deprecated mirror of `displaced`.

**Non-goals.** Command flee Dashing. A second hold for a cast that pushes two
creatures and provokes. Halt (its own issue, rpg-project#830) and Drop. A `kind` field in content (`CastMove`,
`MoveDirective` keep `Provokes`; the session names the kind from it). Doors
on a directed walk.

**Repositories and order.** Develop outside-in, merge inside-out. Toolkit:
one module per PR.

| slice | repo / module | base | PR shape |
|---|---|---|---|
| S1 | rpg-toolkit `encounter` → `session`; rpg-api-protos; rpg-api; rpg-dnd5e-web | main / main / main / dev / dev | encounter PR; session re-pin PR; protos PR; api PR; web PR |
| S2 | rpg-toolkit `encounter` → `session`; rpg-api re-pin | main / main / dev | encounter PR; session re-pin PR; api re-pin PR |

S1 first. S2 after S1's encounter tag (it reads the kind).

## Finish line

- Sub-issues for S1 and S2 closed by their "done means"; each slice walked
  by the operator on :3030; #825's slices table current.
- On the castle: a Thunderwave push slides and draws no opportunity attack;
  a Dissonant Whispers flee walks, turns to face its flight, and draws the
  fighter's opportunity attack; a Command flee walks and draws it too (a guard; a goblin disengages first); a
  commanded goblin still takes its own opportunity attack.

## Requirement labels

| label | requirement | source |
|---|---|---|
| K1 | Every moved beat carries `kind` when it is `compelled` or `displaced`; absent means `own`. | R1, Law |
| K2 | `forced: true` is written exactly when the kind is `displaced`, on the beat, the session walk and the wire. | R1 |
| K3 | The opportunity-attack fold is suppressed exactly for `displaced` steps. | R1 |
| K4 | A creature's own Move and a player's step are `own`; a Routed intent carries its kind (zero = own), and only the compelled driver's intent sets `compelled`; `Direct` takes its kind from its caller and refuses none. | R1 |
| K5 | A held directive and a paused turn resume with the kind they were stored with; a stored pause from before the kind loads by derivation. | R1 |
| K6 | The session's walk segment breaks on a change of kind; a legacy beat with no `kind` derives one from `forced` alone (true → displaced, else own). | R1 |
| K7 | The wire carries `MoveKind kind = 5`; `forced = 4` is deprecated, still set. | R1 |
| K8 | The web slides `displaced`, walks `own` and `compelled`, falls back to `forced` when the kind is unspecified, and feeds a local member's non-own walk. | R1 |
| F1 | A `compelled` walk turns along each step; a `displaced` walk keeps facing. | R2 |
| F2 | Budgets unchanged: Dissonant Whispers one speed; Command the turn's movement. | R2 |
| P1 | Dissonant Whispers spends the reaction and is not taken when it is spent; Command never spends the reaction. | R3 |
| G1 | castle tests, `directive_test.go`, `held_test.go`, `pause_test.go`, `act_fold_internal_test.go`, api `displacement_acceptance_test.go` keep passing with the new fields. | must keep working |

## Constraints

- Toolkit: `go test -race ./...` and `golangci-lint run ./...` per module;
  no `go.work`, no local `replace` in a PR; consumers pin by commit, then by
  tag minted on merge.
- Protos: deprecate, never break (`buf breaking` against main).
- rpg-api: `make test`, `make pre-commit`. Web: `npm run ci-check`.
- Team signature only; one review round per slice; the slice staged on
  :3030 for the operator's walk before merge-out.

## Inspected baseline

| repo | revision | symbols |
|---|---|---|
| rpg-toolkit | origin/main f1291280 (encounter v0.172.0, session v0.159.0, resolution v0.74.0, umbrella v0.213.1) | `encounter/encounter.go`: moved-beat payload `forced` :1701-1708. `encounter/directive.go`: `DirectInput.Provokes` :548-556, `Direct` :636, `walkPath(... !in.Provokes, true)` :669, held pause `forced` :704. `encounter/clocks.go`: Routed arm :1157-1267 (`Dash` :1181, `disengageBeforeWalk` :1235, `walkFor` :1246, `faceWalkEnd` :1260), `walkPath` :1584 (`opens := !imposed` :1594, `action.forced` :1718, `turnAlong` :1721), `walkFor` :1895 (`walkPath(... false, false)` :1903), `walkWorld` :1915 (`turnAlong` :1949). `encounter/step.go`: `executedAction.forced` :61. `encounter/turndriver.go`: `MoveStep.Forced` :1009-1017. `encounter/tabledriver.go`: Routed intents :260, :283, :302, :439, :446, :488. `encounter/clocks.go`: Dash cause :1066-1073. `encounter/step.go`: player `Step` beat :153. `session/turndriver.go`: `routedToEncounter` :539. `encounter/held.go`: :103, :145, :157. `encounter/pause.go`: `pause.forced` :133, `PauseData.Forced` :236, save :338, load :422, refusal :508, resume :980, :993. `encounter/facing.go`: `turnAlong` :181. `session/act_family.go`: `walkPiece` :242-274. `session/act_bodies.go`: `WalkAct.Forced` :37-38. `session/mover.go`: `ForcedBy: forcedBy(step)` :92, `forcedBy` :155. `session/castmove.go`: budget :116-119, `walkCastPushes` :209, `Provokes: push.move.Provokes` :223. `session/compelled.go`: `compelledIntent` :318-363. `resolution/obey.go`: words :32-36, `newObey` switch :169-173, `Start` :180-195, `walk` :211. `resolution/contest.go`: `MoveDirective` :140, `payForMove` :790. `spells/cast.go`: Command constants :136, Thunderwave :566 (Move :609-612), Dissonant Whispers Move :645-650, Command menu :705-709. `spells/data.go`: Command description :194. `conditions/opportunity_attack.go`: `canReact` :131. Tests: `directive_test.go` `TestADirectiveThatProvokesSaysSoToTheMover` :260, `TestAPushedCreatureKeepsItsFacing` :283, `TestAChosenWalkIsNeitherForcedNorCaused` :309; `act_fold_internal_test.go` :84, :104, `TestWalkActsDistinguishWalkRoutAndPush` :115; `resolution/obey_test.go` `TestEveryWordOnCommandsMenuIsAWordThisPackageObeys` :239. |
| rpg-api-protos | origin/main f26fb6e (v0.1.252) | `dnd5e/api/session/v1alpha2/act.proto`: `WalkAct` :368-377 (`bool forced = 4` :376) |
| rpg-api | origin/dev cbb0720 | `internal/handlers/dnd5e/session/v1alpha2/convert_act.go`: `walkToProto` :558-565; tests `convert_act_test.go`, `internal/integration/session/displacement_acceptance_test.go` |
| rpg-dnd5e-web | origin/dev 5b650740 | `src/components/session/arrivingWalk.ts` :15-36; `src/components/hex-grid/useHexMovePath.ts` `WalkAnimationGate.forced` :102, `forcedMoveRef` :298-405, `advanceDisplacementFrame`; `src/components/hex-grid/HexEntity.tsx` `moveForced` :202, :424, :520, :911-920; `src/components/hex-grid/ClassCharacterModel.tsx` `isDisplaced` :113, :487-511, :563; `src/components/session/SessionCanvas.tsx` :864, :900; tests `arrivingWalk.test.ts`, `forcedDisplacement.test.tsx`, `forcedMovePath.test.tsx` |

## Sequence

1. S1: T1 (encounter) → tag → T2 (session) → tag. T3 (protos) in parallel with
   T1/T2 → tag. T4 (api, on the session and protos tags) → T5 (web, on the
   protos tag) → stage on :3030 → walk → merge out.
2. S2: T6 (encounter, on T1's tag) → tag → T7 (session and api re-pin) →
   stage → walk.

## Open items

- Resolved: absent `kind` on a stored beat means `own`, full stop; `kind` is
  written only for `compelled` and `displaced`, so every `own` golden is
  unchanged. A legacy record derives from `forced` alone. A cause never
  derives a kind: a Dash (`table/dash`) and every table walk (`table/*`)
  carry a cause and are `own`.
- Resolved: a turn pause stored before the change resumes `own` (its stored
  `forced` is false). A Command turn paused across the deploy is drawn as a
  walk either way and still provokes; nothing is keyed on the Commanded ref.
- Resolved: Halt leaves this story as rpg-project#830 (not ruled on #826).
- Resolved: S1 changes no facing; S2 owns the facing law, so S1's walk is a
  pure wire change and S2's walk is a pure facing change.
- Resolved: content keeps `Provokes`; the session's crossing maps true to
  `compelled` and false to `displaced` in one function (design, Findings).
- Deferred by design: Dash on Command flee; the second hold; Drop.

## Task contracts

### Task 1 (S1): the encounter names the kind

**Delivers:** K1, K2, K3 (the Mover side), K4, K5.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** none.
**Files:** modify `step.go` (`executedAction.forced` → `kind MoveKind`),
`encounter.go` (moved-beat payload), `directive.go` (`DirectInput`),
`clocks.go` (`walkPath`, `walkFor`, `walkWorld`, the Move and Routed arms),
`turndriver.go` (`Routed.Kind`),
`turndriver.go` (`MoveStep`), `held.go`, `pause.go`; new `movekind.go`,
`movekind_test.go`; update `directive_test.go`, `held_test.go`,
`pause_test.go`, castle goldens if a compelled walk is pinned.
**Interfaces:**
- `type MoveKind string` with `MoveOwn = "own"`, `MoveCompelled =
  "compelled"`, `MoveDisplaced = "displaced"`. The zero value reads as own:
  `func (k MoveKind) Displaced() bool` is true only for `MoveDisplaced`.
- `DirectInput.Kind MoveKind` replaces `DirectInput.Provokes`. `Direct`
  refuses a kind that is not `MoveCompelled` or `MoveDisplaced` with a new
  `ErrBadMoveKind` (fail closed: a directive that names no kind is a bug).
- `MoveStep.Kind MoveKind` replaces `MoveStep.Forced`.
- `Routed.Kind MoveKind`, zero meaning own. The table driver's intents
  (`tabledriver.go`) leave it zero. The Routed arm refuses `MoveDisplaced`
  (a turn walks on its own legs) with `ErrBadMoveKind`.
- `walkPath(..., cause core.Ref, kind MoveKind, imposed bool)`; `walkFor`
  and `walkWorld` take the kind. The Move arm passes `MoveOwn`; the Routed
  arm passes `it.Kind`; `Direct` passes `in.Kind`; the player's `Step`
  (`step.go`) stays `own` by zero value.
- Moved-beat payload: `"kind": "compelled" | "displaced"`, absent for own;
  `"forced": true` when the kind is displaced.
- `pause.kind MoveKind`; `PauseData.MoveKind string json:"move_kind,omitempty"`.
  `PauseData.Forced` is still written when the kind is displaced. Load: a
  stored `move_kind` wins; else `Forced` → displaced; else own. The
  refusal "a turn is never forced" becomes "a turn is never displaced".
**Behavior:** facing and doors unchanged (`imposed` stays as today). A paused
Routed turn stores its intent's kind and resumes with it, so its later
beats keep the kind.
**Tests:**
- `TestAMovedBeatNamesItsKind`: an own Move writes neither `kind` nor
  `forced`; a Routed intent with zero kind (a table chase) writes neither;
  a Routed intent with `MoveCompelled` writes `kind: compelled` and no
  `forced`; a Dash writes its cause and no `kind`; `Direct`
  with `MoveDisplaced` writes `kind: displaced, forced: true`; `Direct` with
  `MoveCompelled` writes `kind: compelled` and no `forced`.
- `TestADirectiveWithNoKindIsRefused`: empty and `MoveOwn` → `ErrBadMoveKind`, nothing moves.
- `TestARoutedTurnIsNeverDisplaced`: `Routed{Kind: MoveDisplaced}` → `ErrBadMoveKind`.
- `TestADirectiveThatProvokesSaysSoToTheMover` → asserts `MoveStep.Kind == MoveCompelled`; a push asserts `MoveDisplaced`.
- `TestAHeldDirectiveResumesWithItsKind`: a compelled directive paused on a window resumes with every later beat `compelled`.
- `TestAPausedRoutedTurnResumesWithItsKind`: a compelled Routed turn paused on a window resumes with `kind: compelled`; a table Routed turn resumes with no kind.
- `TestAStoredPauseLoadsItsKind`: `move_kind` round trip; legacy `forced: true` → displaced; legacy pause without either → own.

- [ ] Add the tests; see them fail.
- [ ] Implement the type, the payload, the walk sites, the resumes.
- [ ] `go test -race ./... && golangci-lint run ./...` in `rulebooks/dnd5e/encounter`.

**Verification:** the commands above, green; `castletest` unchanged except
compelled beats gaining `kind`.
**Completion evidence:** PR link, the encounter tag, test names green.

### Task 2 (S1): the session reads and folds by kind

**Delivers:** K3, K6; K2 on the session walk.
**Owner:** rpg-toolkit `rulebooks/dnd5e/session`.
**Prerequisites:** Task 1 tagged.
**Files:** `go.mod` (encounter tag), `act_bodies.go` (`WalkAct.Kind`),
`act_family.go` (`walkPiece`), `mover.go` (`forcedBy`), `castmove.go`
(`walkCastPushes`), `compelled.go` (`compelledIntent`); tests `act_fold_internal_test.go`,
`castmove_internal_test.go`, `compelled_internal_test.go`.
**Interfaces:**
- `WalkAct.Kind MoveKind json:"kind"` (session's own `MoveKind` string, same
  three words); `WalkAct.Forced` kept, doc'd deprecated, set `Kind ==
  displaced`.
- `walkPiece` decodes `kind`; absent derives from `forced` alone: true →
  displaced, else own. `absorb` compares mover, cause and kind.
- `forcedBy(step)` returns the cause only when `step.Kind.Displaced()`.
- `compelledIntent` (`compelled.go`) sets `Routed.Kind: encounter.MoveCompelled`;
  `routedToEncounter` (`turndriver.go`) leaves it zero.
- `moveKindFor(provokes bool) encounter.MoveKind`: true → compelled, false →
  displaced; `walkCastPushes` passes `Kind: moveKindFor(push.move.Provokes)`.
**Behavior:** opportunity attacks unchanged: displaced suppressed, own and
compelled provoke.
**Tests:**
- `TestWalkActsDistinguishWalkRoutAndPush` extended: own, a Dissonant Whispers flee (compelled), a Command flee (compelled), a Thunderwave push (displaced, `Forced` true).
- `TestALegacyBeatDerivesItsKind`: three payloads without `kind`.
- `TestAWalkSegmentBreaksWhenTheKindChanges`: same mover and cause, compelled then displaced → two walks.
- `castmove_internal_test.go`: Thunderwave directive carries `MoveDisplaced`, Dissonant Whispers `MoveCompelled`.

- [ ] Re-pin encounter; add the tests; implement; `go test -race ./... && golangci-lint run ./...` in `rulebooks/dnd5e/session`.

**Verification:** as above. **Completion evidence:** PR link, session tag.

### Task 3 (S1): the wire carries the kind

**Delivers:** K7.
**Owner:** rpg-api-protos.
**Prerequisites:** none.
**Files:** `dnd5e/api/session/v1alpha2/act.proto`; generated Go and TS.
**Interfaces:**
```proto
// MoveKind is how a body moved: its own walk, its legs under an order, or
// moved without its legs.
enum MoveKind {
  MOVE_KIND_UNSPECIFIED = 0;
  MOVE_KIND_OWN = 1;
  MOVE_KIND_COMPELLED = 2;
  MOVE_KIND_DISPLACED = 3;
}
message WalkAct {
  ...
  bool forced = 4 [deprecated = true]; // mirror of kind == DISPLACED
  MoveKind kind = 5;
}
```
**Behavior:** additive; `buf breaking` passes.
**Tests:** none hand-written (generated bindings); `buf lint`, `buf breaking`.

- [ ] Edit, generate, lint, breaking check; PR; tag on merge.

**Completion evidence:** PR link, protos tag.

### Task 4 (S1): the api converts the kind

**Delivers:** K7 at the server.
**Owner:** rpg-api.
**Prerequisites:** Task 2 and Task 3 tagged.
**Files:** `go.mod`; `internal/handlers/dnd5e/session/v1alpha2/convert_act.go`
(`walkToProto`); `convert_act_test.go`;
`internal/integration/session/displacement_acceptance_test.go`.
**Interfaces:** `walkToProto` sets `Kind` (own → OWN, compelled → COMPELLED,
displaced → DISPLACED, empty → OWN) and keeps `Forced: b.Forced`.
**Tests:** a table over the three kinds in `convert_act_test.go`; the
acceptance test asserts `MOVE_KIND_DISPLACED` and `forced` for Thunderwave,
`MOVE_KIND_COMPELLED` and no `forced` for Dissonant Whispers.

- [ ] Re-pin; implement; `make test && make pre-commit`.

**Completion evidence:** PR link.

### Task 5 (S1): the web animates by kind

**Delivers:** K8.
**Owner:** rpg-dnd5e-web.
**Prerequisites:** Task 3 tagged (package bump); Task 4 on the walk env.
**Files:** `package.json` (protos version); `src/components/session/arrivingWalk.ts`;
`src/components/session/moveController.ts` (`forced` :56, :119-125),
`src/components/session/useMoveController.ts` (:19, :89-92),
`src/components/session/useWalkPresentation.ts` (:14, :90);
`src/components/hex-grid/useHexMovePath.ts` (`WalkAnimationGate`);
`src/components/hex-grid/HexEntity.tsx`; `src/components/session/SessionCanvas.tsx`;
tests `arrivingWalk.test.ts`, `forcedDisplacement.test.tsx`, `forcedMovePath.test.tsx`.
**Interfaces:**
- `ArrivingWalk.kind: 'own' | 'compelled' | 'displaced'` replaces `forced`.
  `MOVE_KIND_UNSPECIFIED` reads `forced` (true → displaced, else own).
- A local member's walk is fed when its kind is not own (no Move answer
  carries a compelled or displaced route).
- `WalkAnimationGate.displaced` replaces `forced`; `HexEntity`'s
  `moveForced` becomes `moveKind`; `isDisplaced` is `moveKind === 'displaced'`.
**Tests:** `arrivingWalk.test.ts`: the three kinds; unspecified with
`forced: true` → displaced; unspecified without → own; the local member's
compelled walk is fed and its own walk is not. The two displacement tests
keep passing, keyed on the kind.

- [ ] Bump; implement; `npm run ci-check`.

**Completion evidence:** PR link, the walk on :3030.

### Task 6 (S2): facing follows the legs

**Delivers:** F1, F2 (asserted unchanged).
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** Task 1 tagged.
**Files:** `clocks.go` (`walkPath`), `held.go` (resume), `facing.go` (doc);
`directive_test.go`.
**Interfaces:** `walkPath` turns along a step unless `kind.Displaced()`;
`imposed` keeps only its door meaning (`opens := !imposed`). `held.go`'s
resume of the first cell turns along under the same rule.
**Behavior:** a Dissonant Whispers flee turns to face each step and ends
facing its last step; a Thunderwave push keeps facing; a Command flee is
unchanged (it already turns, and ends by `routedFace`).
**Tests:** `TestAWhisperedCreatureTurnsToFaceItsFlight` (`Direct` with
`MoveCompelled` along a line away → facing equals the bearing of the last
step); `TestAPushedCreatureKeepsItsFacing` unchanged; a held compelled
directive resumed turns on its first resumed cell; budgets: an existing
castmove test pins Dissonant Whispers' cells to the target's speed.

- [ ] Tests first; implement; `go test -race ./... && golangci-lint run ./...`.

**Completion evidence:** PR link, encounter tag.

### Task 7 (S2): re-pins

**Owner:** rpg-toolkit `session`; rpg-api. **Prerequisites:** Task 6 tagged.
**Files:** `go.mod` in each. **Tests:** existing suites green.
**Completion evidence:** PR links; staged on :3030.

## Walk checklists

The walk site is the castle on :3030: a bard for Dissonant Whispers and
Thunderwave, the cleric or the bard for Command, the fighter's opportunity
attack as the proof of provoking.

**S1** (a gate guard for the provoking rows: castle goblins carry Nimble
Escape, and a commanded goblin disengages before it flees)
- [ ] Fighter beside a guard; the bard casts Thunderwave and the guard fails → it slides out, no walk cycle, no turn, no swing from the fighter.
- [ ] Fighter beside a guard; the bard casts Dissonant Whispers and the guard fails → it walks away and the fighter is offered its opportunity attack.
- [ ] Fighter beside a guard; Command Flee and the guard fails → on its turn it walks away and the fighter is offered its opportunity attack.
- [ ] Fighter beside a goblin; Command Flee and the goblin fails → it disengages (its bonus action) and walks away with no swing offered.
- [ ] The guard that just fled from Dissonant Whispers → walking the fighter out of its reach draws no swing (its reaction is spent).
- [ ] Dissonant Whispers again on that guard before its turn → it takes the damage and does not move ("has no reaction to spend").
- [ ] A commanded goblin, before and after its compelled turn → walking out of its reach draws its opportunity attack.

**S2**
- [ ] Dissonant Whispers → the guard turns to face its flight and ends facing away, with no snap back.
- [ ] Thunderwave → the guard keeps the facing it had.
- [ ] Command Flee → unchanged: it turns along its route.


— cross-team agent, on behalf of KirkDiggler
