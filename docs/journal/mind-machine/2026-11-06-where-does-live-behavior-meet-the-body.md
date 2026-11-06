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
and [walkthrough](../../../ideas/behavior-bindings/consumer.md), settle R4, and
reconcile the [conditional task plan](../../../ideas/behavior-bindings/plan-consumer.md)
with that choice. The plan names files, candidate interfaces, tests, gates and
release dependencies, but is deliberately not labeled executable while the
state-owner decision remains open. This planning request implements no runtime
or authoring fields and does not merge the earlier record PR.
