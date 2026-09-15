# The post-roll window — a machine stops after the die

**Date:** 2026-09-07
**Status:** Design. One slice, cut. Slice one of the bard pilot includes it.
**Umbrella:** `ideas/session-combat/interrupt/` — continues the shipped OA window
(`../design.md`, `../plan.md`, rpg-project#316 rung 3).
**Customer above:** `ideas/bard/level-1/design.md` (rpg-project#397), whose ruling 2 this
replaces.
**Brainstorm:** rpg-project#391 `ideas/spells/brainstorm.md` §6 (Post-roll window: "partial
— ledger and React exist, the producer inside a machine is unbuilt") and §7.

---

## What prompted this

The OA window shipped and **has never been walked**. The monster brain targets the closest
member and never walks past anyone, so the one producer of a window in the whole stack
poses nothing in an ordinary fight. Kirk's call: the post-roll window goes into slice one,
because Bardic Inspiration's spend is a window that opens **every fight, on a player's own
attack**, and so it is the first honest walk of the machinery as well as the bard's real
choice.

It is also the primitive below five separate features. Cutting Words, Shield done right,
Silvery Barbs, Counterspell and concentration all need the same thing: a resolution machine
that stops after a roll is seen and before its outcome is read. One customer above it today.

## The facts this stands on

Toolkit `origin/main` `aaa1d3c3`; protos `origin/main`; web `origin/dev`.

- **The ledger, the freeze, the verb and the wire are shipped.** `interrupt.Ledger` with
  `Pose`/`Answer`/`Open`/`ToData` (`play/interrupt/ledger.go:64`, `:139`, `:230`);
  `SessionData.Windows` (`session/data.go:64`); the two-opener split
  (`session/write.go:1008-1031`, `frozen()` at `:1041`); `Manager.React`
  (`session/react.go:118`); `VerbReact`/`SlotReaction` (`session/afford.go:89`, `:113`);
  `ShortfallWindowOpen` (`session/types.go:2082`); `ReactChoice` STRIKE/HOLD, the `React`
  RPC and `WindowOpened` on the wire (`session/v1alpha1/types.proto:508`,
  `service.proto:571`, `events.proto:1107`).
- **The window payload already has a kind discriminator, and it is checked.**
  `windowPayloadKind = "reaction"` with the comment *"One value today; it is written and
  checked so a payload from a build that poses something else is REFUSED"*
  (`session/window.go:33`, refusal at `:92-95`). This design is the second value.
- **`React` already tolerates a window whose answer resumes no turn.** It calls
  `ResumeTurn` only `if len(open) == 0 && scope.enc.Paused()` (`session/react.go:182`).
- **The strike machine is re-enterable by construction, and says so.** *"Every phase
  boundary is a yielded step, so the machine's own fields are the only state there is and
  nothing accumulates on the Go stack between phases — which is what the reaction windows
  of ADR-0027 will need"* (`resolution/strike.go:97-101`).
- **`Step` is sealed to Gather | Request | Done, and names Pose as absent on purpose**:
  *"ADR-0038 also names Pose, which lands with the caller that forces it"*
  (`resolution/step.go:25-34`). `Request` runs its sub-machine to Done inline (`:119-138`).
- **The d20 and everything read off it sit in one function.** `afterAttackChain` folds,
  rolls (`strike.go:301-314`), sets `Roll`/`Total`/`Hit`/`Critical` (`:316-332`), then
  yields `publishPostAttackRoll` whose continuation runs damage (`:353-362`). The outcome
  fields are all scalars (`StrikeOutcome`, `strike.go:44-82`).
- **`Resolve` keeps nothing alive.** *"Nothing survives the call — not the bus, not a
  loaded sheet, not a subscription"* (`resolution/resolve.go:296-298`). `Output` is data
  (`:240-260`).
- **The door pays once, before the machine.** `Input.Cost` is paid *"after pure machine
  preflight and before execution, and the machine is never told"* (`resolution/resolve.go:83-92`).
- **The struck beat already carries the numbers.** `RecordInput.Values` with `ValueRoll` and
  `ValueTotal` (`encounter/outcome.go:93-96`, `:132`), plus `Reaction`,
  `AdvantageSources`, `PresentationID` (`:150`, `:160`, `:195`).
- **The dock already labels a reaction from the server.** `declaration.reaction?.name`
  (`web ActionDock.tsx:65-66`), the two buttons at `:362-395`.

### Where the evidence contradicts the brief

**The encounter's `PausedTurn` cannot hold this pause, and must not be asked to.** It is a
*driven-turn remainder*: member, round, from, to, `remaining` path, moved, budget, intent,
bound (`encounter/pause.go:98-110`), and `validatePausedTurn` refuses one with
`len(Remaining) == 0` (`:259-261`). A player's own Attack is not a driven turn and has no
path. So for a post-roll window `Encounter.Paused()` stays **false**, `ResumeTurn` is never
called, and the guard already shipped at `react.go:182` is what makes that correct with no
change. The action-economy question ("is the action spent twice?") answers itself the same
way: the cost was charged at the door of the first `Resolve` and the resume passes
`Cost: nil`.

## Ownership — every noun, and who holds it

| Noun | Owner | Why it cannot live anywhere else |
|---|---|---|
| The pause point inside a strike | toolkit `resolution` — a new `Pose` step | `Step` is sealed here because *"every yield point is also a legal suspension point"* (`step.go:25-27`); a suspension expressed anywhere else is a case nobody can resume |
| What the machine froze | the machine, as opaque bytes on `Pose` | resolution drives steps over data and does not read a rulebook's meaning; a typed frozen-strike field here would put the dnd5e attack inside the driver |
| The offer ("a d6 is spendable on this roll") | toolkit `conditions` — the `inspired` condition, on a new offer chain | only something attached to the roller can know it holds one, and a condition on the bus is the only shape for an ongoing effect |
| The die's face | the machine, rolled with the machine's own roller | the machine is what rolls; R1's `ReactionTakenEvent` is the precedent for telling the effect afterwards |
| The open window and its audience | toolkit `session` — the `interrupt.Ledger` it already persists | `SessionData.Windows` is the one durable record of who has been asked (`data.go:64`) |
| The freeze | `Manager.openForChange` | shipped, structural, and generic over window kinds already (`write.go:1008`) |
| The paused turn of a driven monster | toolkit `encounter` | unchanged, and **not involved here** — see above |
| The narration of the pause | toolkit `encounter`, as a beat | *"the composition is the only author of its record"*; the session appends nothing directly |
| The label on the button | the server, through `Declaration.reaction.name` | `ActionDock.tsx:62` states the law |

## Rulings

**R1 — The pause is a new sealed step, `Pose`, and it lands here because this is the caller
that forces it.** `step.go:29-31` says Pose *"lands with the caller that forces it — the
walk machine — rather than now, when it would be an enumeration against a hypothetical."*
The walk machine is not that caller and never was: the OA window pauses at the **mover
seam**, outside resolution entirely, before a step resolves. This is the first caller that
forces it, and the hypothetical is over.

```
Pose{ Ask Ask; Frozen []byte }
Ask{ Audience string; Offer OfferRef; Options []string; Roll, Total int }
```

`Frozen` is machine-authored bytes resolution never reads. `driveStep` returns it up;
`Output` gains `Posed *Pose` and `Outcome` is nil when it is set. *Zero values tell the
truth:* a caller that switches on `Outcome` today gets a nil it must handle, not a
zero-valued `StrikeOutcome` that reads as "missed for 0".
*Scope:* one Pose per machine run. A machine that poses twice in one call is not designed
here and is not refused here either — the driver simply returns the first one.

**R2 — The pause sits between the d20 and `PostAttackRollChain`, not after it.** RAW is
*"after rolling the d20 and before the DM says whether the roll succeeds or fails"*, and the
mechanical reason is stronger: Shield subscribes to `PostAttackRollChain` and reads
`WouldHit` (`strike.go:337-351`). A window opened after that chain would let Shield react to
a total the inspiration die had not yet joined, and `WouldHit` would be recorded twice with
two different answers. So `afterAttackChain` folds a new offer chain immediately after
`m.outcome.Total` is set (`strike.go:317`) and yields `Pose` before building the post-roll
event.
*Scope:* this orders the inspiration die against the post-roll chain. It does not settle
what happens when two post-roll windows want the same boundary; see the shelf.

**R3 — Resume re-enters the machine with the recorded roll. Nothing is re-rolled and
nothing is re-folded.** The frozen half carries what the first call computed:
attacker, target, presentation id, the definition, `Roll`, `AttackBonus`, `TargetAC`,
`CriticalThreshold`, the advantage/disadvantage source refs, and the offer. A resumed
machine (`resolution.NewStrikeResumed`) starts at `Pose`'s continuation: apply the answer,
recompute `Hit`, publish the post-roll chain once, and run damage.
**Re-folding the attack chain would be a second fold with side effects** — subscribers that
record an attempt would record two — so it does not happen. That is also why the fold is
stored rather than recomputed, exactly as the shipped payload stores the definition:
*"THE DEFINITION IS STORED rather than recompiled"* (`window.go:38-44`).
*The natural 1 and the natural 20 do not move.* `Hit` is recomputed only through the
arithmetic branch (`strike.go:324-331`); a d6 added to a natural 1 still misses, and the
crit is still the face of the d20. Fail closed: a frozen roll outside 1–20, or a total that
does not equal roll plus bonus, is `ErrInvalidSession`, not a repaired guess.

**R4 — The offer is a chain the roller's own effects answer, and the die is spent when it is
TAKEN.** A new `dnd5eEvents.PostRollOfferChain`, folded between the roll and the pose; a
subscriber appends `Offer{Ref, Name, Audience, Die}`. The `inspired` condition is the one
subscriber. On `spend`, the machine rolls the die with its own roller, adds the face, and
publishes `OfferTakenEvent{Audience, Ref, Face}`; the condition consumes itself on that
event and publishes `ConditionRemovedTopic{Reason: "spent"}`.
This is rung 3's R1 one layer up — *"the reaction is spent when it is TAKEN, not when it is
offered"* (`../plan.md`, and `ReactHold` *"COSTS NOTHING"* at `session/react.go:29-33`) — so
declining costs nothing here for the same reason and by the same mechanism.
*Scope:* the chain is folded on an **attack** roll only. Saves and ability checks are the
same shape and are not in this slice.

**R5 — Audience is the roller, and this slice extends to nobody else.** The subscriber names
its own audience, so the chain is already wide enough for Cutting Words (a hostile bard on
an enemy's roll) and Shield (the target of the attack). Neither is built here, and neither
is implied. What this slice poses: **one window, to the member whose d20 was just rolled,
because they hold a spendable die.** An offer whose audience is not the roller is refused
with `ErrNotOffered` rather than posed to somebody this slice has not designed a freeze for.
Fail closed loudly.

**R6 — `STRIKE` and `HOLD` stay as shipped, read as "take it" and "decline it", and zero
proto rows move for the choice.** The proto already frames them that way: *"take the
reaction, or let it pass"* (`types.proto:495-496`). Renaming an enum value is a source break
for every client and a buf breaking-change flag, bought for a word the player never sees —
the dock labels its buttons from `declaration.reaction.name` (`ActionDock.tsx:65-66`), which
will read "Bardic Inspiration".
*The special case above* is a third and fourth enum value per new reaction.
*The primitive below* is the window owning its own option strings on the wire, which the
ledger already models (`interrupt.Option` is an opaque string, `ledger.go:15-17`) and which
the session deliberately narrowed at its seam. **Recommend the primitive — and not in this
slice**, because it lands with the third reaction whose answer is genuinely a different
verb (Counterspell), not with the second whose two answers are still take and decline.
Shelved with its trigger named.

**R7 — The window's beat is a new kind, and this is the one unavoidable proto change.**
The audience must see *what they rolled* to decide, and there is no earlier beat to read it
from: the struck beat is written after the machine finishes, which is after the answer. So
the numbers travel on the beat that announces the pause.
`WindowOpened` cannot carry them honestly. It is movement-shaped — `mover`, `from`, `to`,
all documented as load-bearing (`events.proto:1107-1124`) — and a post-roll window has no
mover and no cells. Widening it means three fields whose zero values lie on every post-roll
beat.
So: `EVENT_KIND_ROLL_WINDOW_OPENED` and `RollWindowOpened{string audience, ReactionRef
offer, int32 roll, int32 total}` — additive, buf-clean, `WindowOpened` untouched. In the
toolkit, `encounter.BeatRollWindowOpened` beside `BeatWindowOpened` (`encounter/pause.go:25`)
and an exported append the session can reach, since the existing one is private to the
paused walk (`pause.go:306`).
**The target AC is not on it.** The client already knows the AC only when the server has
shown it; a window that leaked it would tell a player whether their swing lands before they
choose, which is the whole decision. The struck beat says `against` afterwards, as it does
today.

**R8 — There is no timer, and no default answer.** Rung 3's ruling stands unchanged: an
absent player freezes the fight, visibly, and in playtest that is right. The difference
worth naming is that this window freezes the fight **on the roller's own turn**, so the
person it waits on is the person who just acted — the failure mode is one player stalling
their own turn rather than everybody else's. A timer is still shelf, and when it arrives it
answers `hold` here rather than `strike`: keeping a resource is the safe default, taking one
is not.

## Shape, module by module (bottom-up)

**1. `rulebooks/dnd5e/events` — the offer chain.** `PostRollOfferChain` +
`PostRollOfferEvent{AttackerID, TargetID, Roll, AttackBonus, Total, Offers []Offer}`;
`OfferTakenTopic` + `OfferTakenEvent{Audience, Ref, Face}`. Patch/minor tag.

**2. `resolution` — `Pose`, and the resumed strike.** `Pose` as a fourth `Step` with its
`isStep()`; `driveStep` returns it rather than looping (`step.go:112-162`); `Output.Posed`.
`strikeMachine.afterAttackChain` gains the offer fold and the pose after `strike.go:317`;
`NewStrikeResumed(*StrikeResumeInput)` starts at that continuation. `frozenStrike` is the
serialized half, versioned, with an explicit `Kind`.
Tests: a strike with no offer is byte-identical to today (this is the regression that
matters); a strike with an offer poses and produces no outcome; a resume with `spend` adds
one face and recomputes hit; with `keep` it does not; a natural 1 stays a miss and a natural
20 stays a crit either way; a frozen roll of 0 or 21 is refused; the post-roll chain
publishes exactly once across the pair. Minor tag.

**3. `conditions` — `inspired` subscribes to the offer chain.** Instead of adding to
`AttackBonus` on the attack chain (PR #397's R2/R3), it appends an `Offer` on
`PostRollOfferChain` and consumes itself on `OfferTakenEvent` for its own ref and member.
This deletes #397's automatic spend and, with it, the unsourced-`AttackBonus` problem: the
face rides `OfferTakenEvent` and reaches the beat by name.

**4. `session` — a second window kind, and a second React arm.**
- `windowPayloadKind` gains `"post_roll"`; `thawWindowPayload` switches on it
  (`window.go:87-104`) and the three readers each get a branch: `reactDeclaration`
  (`afford.go:583`), `React` (`react.go:147`), and the new resume.
- The post-roll payload holds the audience, the offer ref and name, the frozen strike bytes,
  and the roll/total for the beat.
- `Attack` (`attack.go:294`): a `Resolve` that returns `Posed` records **no** struck beat,
  poses one window, appends the roll-window beat, writes `data.Windows`, and commits with
  the cost already paid and the sheets dirty. `AttackOutput` gains `Paused bool` — the
  roller is the one member who learns this synchronously.
- `React`: `ReactStrike` on a post-roll window resumes rather than swings — a second
  `Resolve` with `NewStrikeResumed`, `Cost: nil`, and the answer; then the struck/missed
  beat the first call did not write. `ReactHold` does the same with the die kept.
- `reactDeclaration` for this kind: `TargetKind: TargetNone`, no candidates. The mover
  candidate at `afford.go:604` is a fact about a step, and there is no step.
- Freeze, `ShortfallWindowOpen`, `openForChange`/`openForWrite`, `selectWindow`,
  `answerWindow`: **reused exactly as shipped, no change.**
Tests: the walk scene below; a save/load between pose and answer round-trips; every change
verb including `EndTurn` is `ErrWindowOpen` while it is open (`turn.go:274` already opens
that way); `Encounter.Paused()` is false throughout. Minor tag.

**5. protos.** `EVENT_KIND_ROLL_WINDOW_OPENED`, `RollWindowOpened`, one `oneof` arm on the
event body. Nothing else — no new verb, no new choice, no declaration field.

**6. rpg-api.** Pins; `eventKindToProto` + `setEventBody` arm; `AttackResponse.paused`;
`ownership_test.go` row. No rule moves.

**7. web.** `reactionWindow.ts`: `reactionWindowMover` must tolerate a window with no mover
(`ActionDock.tsx:346-348`). The dock's prose at `:358` is movement-specific and becomes two
strings chosen by window kind — "Spend it, or keep it. The fight waits on you." The panel
shows the roll and total from the beat. `story.ts` gains the arm; `sessionRefreshKeys.ts` a
row.

## Done-when

**resolution.** A strike with nobody offering anything is unchanged, byte for byte, in
outcome and in beats. A strike with an offer returns `Posed` and no outcome. Resume adds one
face, once, and the fight arithmetic is the frozen roll plus the bonus plus the face.

**session.** Afford shows the roller a REACT declaration named "Bardic Inspiration" and
shows every other member `ShortfallWindowOpen`. `Encounter.Paused()` is false. A restart of
rpg-api between the pose and the answer changes nothing.

**The walk.** A level-1 bard inspires the fighter as a bonus action; the pool drops by one.
The fighter attacks. The dock stops on a panel showing the d20 and the total so far, with
"Bardic Inspiration" and two buttons. **Spend:** the struck beat's total is the d20 plus the
modifier plus the d6, a spend beat says the inspiration was used, and the die is gone.
**Keep:** the beat shows the total without it and the fighter still carries the die into the
next attack. Both runs, same fight.

## Shelf

- **Two windows on one roll** — Shield and Cutting Words both wanting the same boundary. One
  `Pose` per machine run is what this slice drives.
- **Window-owned option strings on the wire** (R6's primitive), when a third reaction's
  answers are not take/decline.
- **The offer chain on saves and ability checks**, which is the rest of Bardic Inspiration
  as written.
- **Cutting Words** — the same chain, audience a hostile bard, and the first window posed to
  somebody other than the roller.
- **Shield done right** — a costed option on this window instead of the readiness flag
  (`conditions/shield_spell.go:194-207`).
- **A timer**, answering `hold`.
- **Concentration**, which is this window's shape on a save the holder did not declare.

## What would change the design

- **If a second post-roll customer lands before this ships**, R5's single-audience rule and
  the one-Pose-per-run limit both have to give, and the option list moves to the wire.
- **If the monster brain learns to walk past people**, the OA window gets its own first walk
  and stops depending on this one to prove the machinery.
- **If Kirk wants the AC visible in the window**, the decision changes from "is a d6 worth
  it" to "does a d6 close the gap", which is a different game and a different design.
- **If `AttackChainEvent` turns out not to round-trip cleanly**, the frozen half narrows to
  the scalars the struck beat projects (`session/events.go:1293-1346`) and the fold is
  rebuilt from them rather than stored whole.
