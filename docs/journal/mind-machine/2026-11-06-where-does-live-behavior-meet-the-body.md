# Where does live behavior meet the creature's body?

Written: **2026-11-06**.
Work: [live S1 adoption — #866](https://github.com/KirkDiggler/rpg-project/issues/866),
under [#859](https://github.com/KirkDiggler/rpg-project/issues/859).
Previous entry: [The executable two-guard component](2026-11-06-can-one-authored-body-drive-two-patrols.md).

## Question

The operator asked for the live-consumer plan after `mind/behavior/v0.20.0`
shipped. Which existing owner must consume its decisions, and what must survive
from authoring through real movement, a reaction window and reload?

## Evidence

Inspected toolkit `9922e218d7942a8cf048af46cc2090f2e77d4fd1`, web
`2150256f9e067c285c7723daf12f4ec54a03bc77`, API
`590a201ac490bbbd4ca0c2e593e958d3e5f33463`; project planning starts from the
unmerged record branch at `5e1eb58d486933fe12cc058eae1141602d630448`.

- [`MemberInput`](https://github.com/KirkDiggler/rpg-toolkit/blob/9922e218d7942a8cf048af46cc2090f2e77d4fd1/rulebooks/dnd5e/encounter/field.go)
  binds `BlockingStates` and `Posts` to its existing machine. The state is read,
  not mirrored as another physical flag. `session.Launch` carries those fields
  together, and `MemberData` saves the recipe and private blob.
- [`TableDriver.Act`](https://github.com/KirkDiggler/rpg-toolkit/blob/9922e218d7942a8cf048af46cc2090f2e77d4fd1/rulebooks/dnd5e/encounter/tabledriver.go)
  rolls and translates. Its private `intentFor` already performs translation
  without randomness. Calling Act after the new composer would select twice.
- [`executeTurnIntent`](https://github.com/KirkDiggler/rpg-toolkit/blob/9922e218d7942a8cf048af46cc2090f2e77d4fd1/rulebooks/dnd5e/encounter/clocks.go)
  reports turn-end, not arrival. A blocked or budget-short Routed walk still
  ends the turn. [`turnPause`](https://github.com/KirkDiggler/rpg-toolkit/blob/9922e218d7942a8cf048af46cc2090f2e77d4fd1/rulebooks/dnd5e/encounter/pause.go)
  has no behavior attachment/selected-key/goal claim for the eventual completion.
- [`wakebeats.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/9922e218d7942a8cf048af46cc2090f2e77d4fd1/rulebooks/dnd5e/encounter/wakebeats.go)
  queues most inputs until a raise, but `barred` is now a synchronous targeted
  exception. Its stored heard input has no kind; the component fixture's
  `heard.kind` reader cannot simply be assumed present in this consumer.
- [`machine.Listeners.Set`](https://github.com/KirkDiggler/rpg-toolkit/blob/9922e218d7942a8cf048af46cc2090f2e77d4fd1/mind/machine/listeners.go)
  keeps the first listen per beat. Concatenating role scopes would lose listeners.
- [`compelledDriver.Act`](https://github.com/KirkDiggler/rpg-toolkit/blob/9922e218d7942a8cf048af46cc2090f2e77d4fd1/rulebooks/dnd5e/session/compelled.go)
  must run before ordinary choice: commanded turns never ask the brain. Existing
  `Capabilities.Roller` already supplies shared dice; this is not a missing RNG
  capability, and calling the composer ahead of Driver would bypass compulsion.
- [`validateSiteScope`](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/2150256f9e067c285c7723daf12f4ec54a03bc77/src/concepts/world-building/siteScope.ts)
  preserves unknown root fields through `unread`, and `encodeSingleRoomDungeon`
  re-splices original source. That is useful preservation machinery, not proof
  that a new behavior field is editable and consumed by the server.

A focused unchanged encounter baseline passed **0.030s**: `TestHeldSuite`,
`TestADwellReloadedWhilePausedSpendsNone`, `TestABumpMatchesOnlyItsOwnBeat`,
`TestBystandersHearTheAnswerOnTheNextRaise` and `TestTheHoldIsAStateAtAPlace`.
The command is recorded in the [consumer plan](../../../ideas/behavior-bindings/plan-consumer.md).
No new consumer test, API/web gate, installation or live-stack walk ran.

## Interpretation

This is not a dependency bump. The existing creature machine owns a state that
other tools already use, and its picks are part of actual execution. A second
runner without an explicit ownership/arbitration rule could grant competing
actions or silently change blocking/posts.

The recommendation is one composer for an opted-in actor, importing the old
creature recipe/table as an explicit named instance and explicitly naming the
body-state source. Physical permission remains encounter-owned; private duties
do not all gain permission authority. Legacy actors without a profile keep their
existing behavior. That is proposed R4, not an accepted consequence of R1–R3.

The remainder of the conditional plan makes the other seams concrete: preserve
compulsion-before-choice, translate the selected answer without rerolling, carry
an actual-completion claim through the existing held-turn path, validate/lower
source values once and preserve them in Studio. A successful turn ending cannot
stand in for a reached waypoint.

## Next

Review [the consumer proposal](../../../ideas/behavior-bindings/consumer-design.md)
and [walkthrough](../../../ideas/behavior-bindings/consumer.md), settle
[R4 in its checklist](https://github.com/KirkDiggler/rpg-project/pull/877#issuecomment-6693233263), and
reconcile the [conditional task plan](../../../ideas/behavior-bindings/plan-consumer.md)
with that choice. The plan names files, candidate interfaces, tests, gates and
release dependencies, but is deliberately not labeled executable while the
state-owner decision remains open. This planning request implements no runtime
or authoring fields and does not merge the earlier record PR.

**Review follow-up, 2026-11-06:** the operator adopted
[review 5805884858](https://github.com/KirkDiggler/rpg-project/pull/877#pullrequestreview-5805884858)
and the assessment. R4–R7 now select a canonical runtime rather than permanent
opt-in branches, typed delegation, distinct world/fight opportunities and a
YAML/preservation-only first delivery. The original alternatives above remain
as the record of the investigation. This does not collapse independent movement,
needs or other machines into one actor-wide state graph. Exact source/instance
and selection-lane contracts remain to be finished before runtime changes.
No runtime files were modified in this planning revision. #877 was retargeted
to main before #868 can merge or its branch be deleted; neither PR was merged.

**Source and lane follow-up, 2026-11-06:** the operator accepted the focused
[source → instance contract](../../../ideas/behavior-bindings/source-instance-contract.md),
recorded as R8. The subsequent [selection-lane section](../../../ideas/behavior-bindings/selection-lane-contract.md)
distinguishes observation, independent data-only progress, authorized action
requests, no-candidate versus explicit Hold and actual completion. Seven existing
Composer tests pass unchanged in 0.006s; they support those component semantics,
not a live consumer. The remaining normalization/parity section must address the
old dispatcher's transition-before-pick behavior rather than assume it matches
composer observation/commit semantics. No runtime files changed in this pass.

## Implementation follow-up — 2026-11-06

**Question:** Must old experiments survive before we can build the canonical
playtest consumer? The operator answered no: rebuild the site and fixtures;
backwards compatibility is unnecessary ([R9](https://github.com/KirkDiggler/rpg-project/pull/877#issuecomment-6698028412)).
The earlier normalization/parity requirement above is superseded, not another
approval gate.

**Evidence:** [Provider #2292](https://github.com/KirkDiggler/rpg-toolkit/pull/2292)
implements inspection/state/wake access. Its regression first selected a sleeping
role at time three despite its wake at eight. The fixed code passes full-module
race/lint/CI; [independent review](https://github.com/KirkDiggler/rpg-toolkit/pull/2292#pullrequestreview-5808481734)
found no issues and killed early-wake, live-pointer and overflow mutants.
[Consumer draft #2293](https://github.com/KirkDiggler/rpg-toolkit/pull/2293) adds
the shared selected-answer translation boundary and fixes queued heard-kind loss
through actual sound creation, EncounterData JSON and load. Exact d200 face 101
keeps the second selected goal without another roll. Clang then voice each retains
its own kind; missing kind stays unknown. Full encounter-module race/lint and the
scoped hook pass at `440c681c`.

**Interpretation:** These are concrete provider and preparatory consumer results,
not the live patrol journey. The old runner is still active in the draft; it is
not accepted as a permanent branch. Neither PR is merged or deployed.

**Next:** Install canonical instance/configuration ownership and designated body
state; replace dispatch with lawful opportunities and addressed real completion;
carry held work through pause/reload. Rebuilt source and Studio/server/local-stack
proof follow in the same adoption wave, without a legacy migration prerequisite.
