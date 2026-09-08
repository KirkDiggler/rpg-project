# Concentration — one condition owns what a spell left behind

**Date:** 2026-09-08
**Status:** Design. One slice, cut. Slice three of the bard/spells initiative.
**Umbrella:** `ideas/spells/` — levels 1–3, and the choices at 3 made real.
**Brainstorm:** rpg-project#391 (`ideas/spells/brainstorm.md`) §3.2, §6.2 ("Concentration — an
owning condition", the row marked *missing* and *the largest single piece*).
**Slice two:** rpg-project#405 (`ideas/bard/cantrips/design.md`), shipped 2026-09-08 as
dnd5e v0.148.0 / encounter v0.67.0 / resolution v0.36.0 / session v0.68.0, protos #310/#311,
rpg-api#949, web#994.

Kirk's ruling that cut this slice: **concentration, alone.** One shape — an owning condition
on the caster that holds child effects and ends them together. Its first customer is a
retrofit, so the slice needs no new content to prove itself.

---

## What this slice is

A spell that says *concentration* puts a condition on its caster. That condition **owns** the
effects the spell left on the board, and when it ends, they end with it. It ends on four
triggers:

1. **The caster takes damage** and fails a Constitution save at DC max(10, half the damage).
2. **The caster casts another concentration spell** — the older one drops.
3. **The spell's own duration runs out**, on the clock the rulebook already publishes.
4. **The fight ends**, on the boundary the rulebook already fans per member.

The first customer is **True Strike**, which shipped in slice two *without* concentration and
said so in its own doc comment. Retrofitting it is the whole proof: no new spell, no new
content table row, and the divergence slice two wrote down gets deleted rather than kept.

The second customer is named only: **Bless**, arriving with the cleric in a collaborator's
lane. Bless is also "choose up to N targets in an area", which is **fan-out** — a different
shape, out of scope, on the shelf.

## The facts this stands on

Toolkit `origin/main` (`e894540e`), protos `origin/main` (`9b7b8ef`), rpg-api `origin/dev`
(`67c06da`), web `origin/dev` (`b2a71c2a`). Four read-only surveys.

**The two halves of concentration that already exist, unwired.**

- `saves.DCHalfDamageFloorTen()` (`saves/gate.go:113-117`) computes exactly
  `max(10, floor(damage/2))` (`gate.go:135-146`), its kind is already commented *"concentration's
  max(10, damage / 2)"* (`gate.go:51`), and **it has no production caller** — only
  `gate_test.go` and the JSON router at `gate.go:330`.
- `dnd5eEvents.SaveTriggerConcentration` (`events/events.go:411-412`) is declared and has
  **no producer and no consumer** anywhere in Go.
- `SaveCause{Trigger, EffectRef, InstigatorID, InstigatorType}` (`events/events.go:419-424`)
  is the one shape that already pairs a trigger kind with a spell ref and a caster id, and the
  cast slice uses it (`resolution/contest.go:453-458`).
- The contest already feeds a `DCSource` from damage: `contest.go:536`
  `dc := m.in.Gate.DC(saves.DCInput{DamageTaken: m.in.DamageTaken})`. That line is the whole
  DC story, already written, for a different customer.

**The clock is finished for three of the four triggers.**

- Combat end fans **one boundary per member** — `encounter/clocks.go:723-737` builds a
  `Boundary{Kind: CombatEnded, Subject: id}` for each, precisely so a subject-less ending
  expires nothing (`turndriver.go:355-364`); `resolution/boundary.go:125` publishes
  `CombatEndTopic` with `CombatEndEvent{SubjectID}`.
- Turn end is `TurnEndTopic` (`events/events.go:1058`), and **a duration is counted today as
  an int of turn ends on the condition itself** — `TrueStrikeTurnEnds = 2`
  (`conditions/true_strike.go:24-31`), decremented at `:251-262`, persisted at `:40`.
- Long rest has a shared safety-net helper every condition already uses,
  `conditions/rest.go:15-38`.
- There is **no round boundary and no minute clock**, deliberately (`turndriver.go:333-348`,
  Kirk 2026-08-27). "Up to 1 minute" has no representation and this slice does not build one.

**Conditions persist, and only on the sheet.**

`character.Data.Conditions []json.RawMessage` (`character/data.go:107`), written at
`character/character.go:1073-1082`, reloaded through the ref-keyed registry
`conditions/loader.go:17-193` which refuses an unknown ref (`loader.go:211`). Built through
`conditions/factory.go:39-122`, which also refuses an unknown id (`:113-115`).
`EncounterData` (`encounter/data.go:26-118`) and `SessionData` (`session/data.go`) hold no
condition state at all. **The sheet is the only durable home**, and a concentrating caster is
per-member persistent state, so it is a condition. That is not a preference; it is the only
store.

**The precedent for a caster-side condition pointing at another member** is
`TrueStrikeConditionData{Ref, MemberID, TargetID, SourceRef, TurnEndsLeft}`
(`conditions/true_strike.go:35`). Concentration is that shape with a list instead of one id.

### Where the evidence contradicts the brief

Four, named rather than smoothed over. Two of them move the design.

**1. There is no damage-applied event on the bus, so a condition cannot subscribe to damage.**
The brief says the CON save is *"the contest/save machine fed from the damage-applied path"*.
That path does not publish. `resolution/strike.go:701-728` deliberately does **not** publish
`DamageReceivedEvent`, and says why: `monster.SheetKeeper` treats `DamageReceivedTopic` as an
*instruction* (`monster/load.go:210`) and a publish would double-apply. Retiring that is
toolkit#977, not this slice. `character.ApplyDamage` (`character/character.go:611-658`) takes
no bus and publishes nothing (`:631-633`).
So the trigger cannot be a subscription. **It is a second interaction, driven by the session
from what the first interaction returned.** R2 below.

**2. A nested `Resolve` is refused, so the save cannot run inside the strike either.**
Inside a running `Resolve` the reconstructed encounter is handed
`RefusingStriker`/`RefusingMover`/`RefusingAnnouncer` (`resolution/resolve.go:352-374`). The
only in-band way one machine runs another is a `Request` step (`resolution/step.go:74`). A
concentration check therefore either becomes a `Request` inside every machine that can deal
damage — two today, more later — or one interaction after the fact. The stack already has a
worked answer for *"the composition noticed something and cannot publish it"*: the
`Announcer` seam (`session/announcer.go:38-48`). Concentration is that pattern's third user.

**3. The door cannot express "you are already concentrating".**
The brief calls the recast rule an activation-door rule. `payAtTheDoor` (`resolution/cost.go:284-309`)
knows only a `combat.SpendProfile`, whose `Requires` is *"keyed capacity that must be PRESENT
and is never spent"* and is explicitly **not a predicate language**
(`combat/spend_profile.go:47-56`, `:82-90`). And the drop is a *mutation*, while `Start` is
contractually pure preflight — *"Start is pure preflight and runs before payment"*
(`resolve.go:416-419`). So the drop is **the first yielded step after the charge**, not the
door. R3. That ordering is also the correct rule: you drop the old spell because the new one
was actually cast, not because you tried.

**4. The applied amount is honest from a strike and not from a cast.**
A strike folds the damage chain — `strike.go:532` → `strike.go:879-889` →
`combat.FinalDamage` at `strike.go:674` — so resistance and immunity are already applied
before `applied.TotalDamage` (`strike.go:698`). The cast slice's damage delivery
**does not fold the chain at all** (`resolution/contest.go:280-290`, `FinalDamage` at `:312`
over unfolded components), so a resistant target takes full damage from a cantrip today. That
is a slice-two gap, not a concentration gap, and it makes a concentration DC from cast damage
too high. **Recorded, not fixed here** — R9 and the shelf.

## Ownership — every noun, and who holds it

| Noun | Owner | Why it cannot live anywhere else |
|---|---|---|
| The fact that a caster is concentrating | toolkit `conditions` — one condition on the caster's sheet | conditions are the only per-member persistent, bus-applied state (`character/data.go:107`); `EncounterData` and `SessionData` hold none |
| Which spell is being concentrated on | that condition's own blob | the same law slice two applied: the source ref is a field the condition was constructed with (`true_strike.go:77`, `vicious_mockery.go:60`) |
| Which effects the spell left behind | that condition's own blob, as (member, condition ref) pairs | `TrueStrikeConditionData.TargetID` (`true_strike.go:35`) is this shape with one id; ending them is the owner's job, so the owner holds the list |
| Whether a spell requires concentration | toolkit `combat/actions` — `CastProfile` | the profile is where content declares what a cast *is*, and it *"names no spell"* (`combat/actions/cast.go:49-58`) |
| How long the spell lasts | the concentrating condition, as a turn-end count | one clock per spell. Today True Strike counts its own turn ends (`true_strike.go:251-262`); after this slice the owner counts and the child does not, because two clocks on one spell is two answers to one question |
| The concentration DC | toolkit `saves` — `DCHalfDamageFloorTen` | it derives at resolution time from `DCInput{DamageTaken}` (`gate.go:62-66`), which is exactly what the sealed set exists for (`gate.go:68-97`). Contrast R4 of the cast slice, where the spell save DC derived from nothing the machine saw and went to the caster |
| Deciding a save happened at all | toolkit `session` — the seam after the interaction | resolution cannot publish damage (contradiction 1) and cannot nest (contradiction 2); the session is the only place that sees a finished interaction and can start another (`announcer.go:38-48`) |
| How much damage was taken | toolkit `resolution` — reported on `Output` | the machine that applied the damage is the only thing that knows the applied number (`strike.go:698`, `contest.go:335`) |
| Ending a child on another member's sheet | toolkit `conditions` — the removal **fact**, honoured by that member's keeper | `ConditionRemovedEvent` is *"A FACT, NOT A COMMAND"* (`events/events.go:678-687`); there is no cross-actor `RemoveCondition` and this slice does not add one |
| The record of a break, and why | toolkit `encounter` — one beat | *the composition is the only author of its record*; the session appends nothing directly |
| The badge the player reads | web, off the condition view that already exists | `characterPresentation.ts:38-66` is already a ref-keyed table; a new row costs no proto |
| Whether other players can see it | protos — one bool on the session-lane roster row | `Participant.active` (`types.proto:883`) is the existing per-member boolean; nothing else on that lane carries member state |

## Rulings

**R1 — Concentration is one condition on the caster, `dnd5e:conditions:concentrating`, and it
holds its children as (member, ref) pairs.**
`ConcentratingConditionData{Ref, MemberID, SpellRef, SpellName, TurnEndsLeft, Children []ChildRef}`
where `ChildRef{MemberID, ConditionRef string}`. It is built by the factory
(`conditions/factory.go:70-115`, one arm) and loaded by the registry
(`conditions/loader.go:17-193`, one entry), exactly as every other condition. It subscribes to
three things: `TurnEndTopic` (its own duration), `CombatEndTopic` (`events/events.go:1090`),
and `ConditionRemovedTopic` (`:1073`) so it notices its own children ending. `conditions/rest.go`'s
shared long-rest net removes it like any other.
**Why (member, ref) pairs and not condition pointers:** a pointer would not survive the blob
round-trip through `character/data.go:107`, and the removal channel is already keyed on
member id plus ref string (`ConditionRemovedEvent`, `events.go:689-693`). The pair *is* the
address the removal fact takes.
*Scope:* one owner, N children, one spell. It is not a generic effect graph — a child that
itself owns children is a shape nobody has asked for.

**R2 — The damage save is a second interaction, driven by the session from what the first
interaction reported. `Output` gains one field.**
`resolution.Output` gains `DamageTaken []DamageTaken{MemberID string, Amount int, DroppedToZero bool}`,
filled by the two places that apply damage today — `strike.go:692-698` and
`contest.go:291-351` — and by every future one. The session, after any `Resolve` that reports
damage, asks each damaged member's sheet whether it is concentrating and, if so, runs **one
more `Resolve`** with `Cost: nil`, exactly as `announcer.go:55-79` does for a boundary.
*The special case above* is a `Request(save)` step inserted into the strike machine, then
again into the contest, then into whatever applies damage next — the concentration rule
copied once per damage source. *The primitive below* is **resolution reporting what damage it
applied**, which the session then acts on in one place. Recommend the primitive: one field,
one seam, and every future damage source is covered the day it fills the field.
*Scope:* damage as a break trigger. It is not a general "after the interaction" hook and it
does not make `Output.DamageTaken` a subscription surface — it is a report, read once by its
one caller.

**R2a — The save is automatic. It is not a `Pose` and not a window.**
RAW 2014 gives the caster no choice: the save simply happens. The machinery for a
non-interrupt save is a plain sub-machine — `resolution/save.go:80 NewSave`, already run as a
`Request` by the contest (`contest.go:646-658`) — and a sub-machine **structurally cannot
pose** (`step.go:167-176`). Stated honestly: posing *is* available inside a resolution
machine now (`resolution/strike_pose.go:229`, shipped with the post-roll window), so choosing
not to use it here is a rules decision, not a missing capability.

**R3 — The recast drop is the first yielded step of the new cast, after the charge, and it is
a step in `castMachine`.**
`castMachine.Start` runs the inner machine's Start (`resolution/action.go:173-191`) so a bad
cast is refused while `Resolve` is still in pure preflight. The drop cannot go there — Start
mutates nothing (`resolve.go:416-419`). It cannot go in the door — the door charges a
`SpendProfile` and nothing else (`cost.go:284-309`). So when the definition's
`CastProfile.Concentration != nil` and the caster already carries the concentrating condition,
`castMachine` yields one leading `Gather` named *"drop concentration on <spell>"* that
publishes the removals, then proceeds to the inner machine's steps.
The ordering is the rule: **preflight → charge → drop the old spell → resolve the new one**
(`resolve.go:416-427`). A cast refused at the door drops nothing, which is what RAW means by
"when you cast another spell that requires concentration".
*Scope:* a concentration cast displacing a concentration cast. It says nothing about
Counterspell or any other declaration-time interrupt; those are the door's third producer and
still shelved.

**R4 — The save is the contest, with one new imposed-effect kind: a condition removed. No new
machine.**
The brainstorm's rule stands: *a machine exists only when we need a new shape or different
steps*, and *an effect kind never adds a machine*. The steps here are the contest's exactly —
Request(save) → outcome policy → deliver → Done (`contest.go:144-215`) — and the DC is already
plumbed from `DamageTaken` at `contest.go:536`. What is absent is a delivery that **removes**
rather than applies.

| Type | Today | Delta |
|---|---|---|
| `ImposedEffectKind` (`contest.go:63-71`) | `ImposedCondition`, `ImposedDamage` | one row, `ImposedConditionRemoved` |
| `ContestInput` (`contest.go:25-56`) | `Gate, SaverID, Application, Damage, SourceName, Cause, DamageTaken, Roller` | one field, `Removal *ConditionRemoval{MemberID string, Refs []string}` — the declared consequence beside the declared condition and the declared damage |
| the delivery (`contest.go:613-623`) | failure → damage, then condition | failure → damage, then condition, then removals, publishing `ConditionRemovedTopic` per pair |
| `Application` | required | may be nil when a `Removal` or `Damage` is declared — the same widening slice two made for damage |

The gate is `{Constitution, Negated, RecurrenceNone, DCHalfDamageFloorTen()}` and the cause is
`SaveCause{Trigger: SaveTriggerConcentration, EffectRef: <the spell>, InstigatorID: <the
damager>}` — **the first driver either of those two constants has ever had.** `Negated` is
correct and the refusals at `contest.go:159-164` stay exactly as they are: a made save keeps
the spell whole, and `contest.go:595-597` already returns before any delivery on success.
*Scope:* removal as a consequence of a save. It is not a `Dispel Magic` verb and it is not a
cross-actor removal API — the delivery publishes the fact, and each keeper honours it.

**R5 — The keeper must call `Remove` on a condition it prunes. This is a real leak today and
concentration is the first thing that trips it.**
`onConditionRemoved` (`character/character.go:1195-1216`, mirror `monster/monster.go:553-573`)
matches on ref string and drops the behavior from the slice — **it never calls
`Remove(ctx, bus)`**, so the pruned condition's subscriptions stay live on the bus. That is
invisible today because the only publishers of `ConditionRemovedTopic` are conditions ending
*themselves*, which call their own `Remove` right after the publish (e.g.
`true_strike.go:275-288`). The moment an owner ends someone else's condition, the child keeps
listening from a list it is no longer in.
The fix is in the keeper, not in concentration: **if `cond.IsApplied()`, call
`cond.Remove(ctx, bus)` before dropping it.** `IsApplied()` is already on the interface
(`events/events.go:138-159`) and makes the second call a no-op, so the self-ending path is
unchanged. *The special case above* is having the concentrating condition hold live pointers
to its children and call `Remove` itself, which cannot survive the JSON round-trip. *The
primitive below* is the removal fact meaning what it says on every sheet. Recommend the
primitive.

**R6 — Concentration ends when its last child ends, and the beat says so.**
The owner subscribes to `ConditionRemovedTopic` and drops a child from its list when that
child's (member, ref) is removed by anything — consumed on use, expired, dispelled. When the
list empties, the spell is over and the concentrating condition ends itself with reason
`spell_ended`.
Without this, a bard whose True Strike was consumed by an attack still reads as concentrating
and still drops "nothing" on the next concentration cast — a visible lie, and exactly the
shape of affordance-with-nothing-behind-it that slice one's walk kept finding.
*Scope:* the owner watches its own children. It does not watch anything else, and a spell with
zero children — none exists yet — would end on its duration alone.

**R7 — Four reasons, and `caster_down` is the fourth. Incapacitated as a condition is
deferred.**
The reasons are `damage` (a failed save), `recast`, `duration`, `combat_end`,
`spell_ended` (R6) and `caster_down`. `Incapacitated` is a ref with **no behavior at all** —
no file, no factory arm, no loader entry (`refs/conditions.go:64` and nothing else) — and
building the general condition is a different slice. But the caster dropping to 0 HP is the
one incapacitating case this seam already sees for free: `ApplyDamageOutput.DroppedToZero`
(`combat/combatant.go:36-45`) rides the same report R2 adds.
**Recommend taking it.** The cost is one boolean read at a seam that is already there; the
cost of leaving it out is a downed bard who concentrates forever, which the walk will find on
its first knockdown. **Open ruling — this is the one place the slice reaches past its
smallest cut, and it is Kirk's call.**
*Scope:* dropping to 0 HP. Paralysis, stunning, sleep and the general Incapacitated condition
stay on the shelf with the rest of the thirteen inert refs.

**R8 — True Strike's retrofit moves the clock onto the owner and deletes the divergence.**
`CastProfile` gains `Concentration *CastConcentration{TurnEnds int}` — a pointer, because a
`bool` plus a duration field that means nothing when the bool is false is a zero value that
lies. True Strike declares `{TurnEnds: 2}`, which is the count it carries today
(`TrueStrikeTurnEnds = 2`, `true_strike.go:24-31`), and **`TrueStrikeConditionData.TurnEndsLeft`
and its turn-end subscription are deleted** (`true_strike.go:40`, `:131`, `:251-262`). The
child keeps consumed-on-use (`:245`), which is a trigger and not a clock; the owner keeps the
clock. One spell, one duration, one answer.
The doc comment at `true_strike.go:58-65` — *"THERE IS NO CONCENTRATION… the day a levelled
spell needs it is the day it has to exist"* — is deleted, not amended.
Persisted True Strike blobs from before this change lose a field. Pre-release, no consumer,
no migration (`no-backcompat-baggage`).
*Scope:* True Strike. Vicious Mockery is not a concentration spell and does not change.

**R9 — The DC uses the applied amount, and the cast path's missing chain fold is recorded as
a gap rather than fixed here.**
`DamageTaken` is the number the sheet actually took: `applied.TotalDamage`
(`strike.go:698`) for a strike, `contest.go:335`'s applied total for cast damage. From a
strike that number is already post-resistance and post-immunity (`FinalDamage` at
`strike.go:674`, after the chain fold at `:879-889`). From a cast **it is not** — the cast
damage path folds no chain at all (`contest.go:280-290`), so a resistant caster hit by a
cantrip is asked for a higher DC than RAW allows.
That is a slice-two defect with a wider blast radius than concentration, and fixing it here
would make the largest diff in the slice be in someone else's shape. **Recorded, with its own
toolkit issue, named on the shelf.** The concentration code reads the reported number and is
correct the day the fold lands.

**R10 — One new beat kind, and the save rides the beat that already exists.**
`EVENT_KIND_CONCENTRATION_ENDED = 29` with `ConcentrationEnded{caster, SpellRef spell, reason}`
at body tag 35 — both confirmed free (`events.proto:259`, `:369`), and both additive under the
repo's `FILE` breaking policy (`buf.yaml`, gate at `.github/workflows/ci.yml:36-43`), so **no
`breaking-change-approved` label is needed.**
The save itself needs nothing new: `EVENT_KIND_SAVED` shipped in slice two with
`{saver, ability, roll, total, dc, succeeded, source}` (`events.proto:797`), which is exactly a
concentration check. The ended children are `ResultConditionRemoved`, already in encounter's
closed result set (`encounter/activation.go:24-50`).
**Why a dedicated kind rather than reading it off the removals:** three of the six reasons
produce no save at all, and a child ending on *another member's* sheet with no cast beat near
it reads as a random condition drop. The beat is the only thing that says *the spell ended, and
this is why*.
*Scope:* the break. There is no "concentration started" beat — the cast beat already is one.

**R11 — Other players see a concentrating member through one bool on the roster lane; the
caster sees a badge for free.**
The owner's own view costs **zero proto**: `character.StatusView.Conditions`
(`character/status_view.go:115`) already reaches `CharacterData.conditions`
(`encounter/types.proto:440`) and renders through a ref-keyed table
(`characterPresentation.ts:38-66`), so one row keyed `dnd5e:conditions:concentrating` is the
whole client change. That path needs the condition added to the display catalog
(`conditions/display.go:43`), which is a hard error on an unknown ref.
For everyone else there is nothing: `Participant` (`types.proto:879`) carries no conditions and
no flags, and `PublicMemberInfo` says in its own comment that it is identity, not per-turn
state. So `bool concentrating = 8` on `Participant`, mirroring `bool active = 5`.
**Recommend taking it.** A break beat about a member whose concentrating state was never
visible is a beat with no setup. **Open ruling — the alternative is to defer it and ship the
owner badge alone.**

**R12 — Monsters do not concentrate, and nothing pretends they do.**
No monster casts today; `spells.CastDefinition` is reached only from `session/casts.go:83`
over a character's known cantrips. `Monster` gets the same keeper fix as `Character` (R5)
because the fix is about the removal fact, not about casting. When a monster casts, it holds
the same condition on the same sheet field (`monster/data.go:42`) with no design change.

## Shape, per module (bottom-up)

**1. `refs`** — `Conditions.Concentrating()` beside the fifteen core rows
(`refs/conditions.go:59-75`); id `concentrating`, underscore-free like the ids it joins.

**2. `combat/actions`** — `CastConcentration{TurnEnds int}` and
`CastProfile.Concentration *CastConcentration` (`cast.go:65-86`), with arms in the profile's
`Validate` (`cast.go:118-135`) and in `Clone`. Nothing in the type names a spell.

**3. `conditions`** — `concentrating.go`. Data blob per R1; `Apply` subscribes `TurnEndTopic`,
`CombatEndTopic`, `ConditionRemovedTopic`; `end(ctx, reason)` publishes one
`ConditionRemovedEvent` per child pair and then its own, following
`vicious_mockery.go:232-245` verbatim. One factory arm (`factory.go:70-115`), one loader entry
(`loader.go:17-193`), one display-catalog row (`display.go:43-80`), and the long-rest net
(`rest.go:15-38`). `true_strike.go` loses `TurnEndsLeft` and its turn-end subscription (R8).

**4. `character` / `monster`** — `onConditionRemoved` calls `Remove` when `IsApplied()`
(`character/character.go:1195-1216`, `monster/monster.go:553-573`) — R5, and the one change in
this slice that fixes a defect rather than adding a shape.
`Character.Concentration() (ConcentrationView, bool)` reads its own conditions
(`character.go:590-592`) and answers spell ref, name and child count. **The caster answers
whether it is concentrating**, the same law R4 of the cast slice applied to the save DC. No
caller type-asserts over `GetConditions()`.

**5. `resolution`** — `Output.DamageTaken []DamageTaken` (R2), filled at `strike.go:692-698`
and `contest.go:291-351`. `ImposedConditionRemoved` and `ContestInput.Removal` (R4), with the
removal delivery chained after the condition delivery at `contest.go:613-623`. `castMachine`
gains its leading drop `Gather` (R3) at `action.go:173-191`'s machine, and `newCast`
(`action.go:67-105`) applies the concentrating condition to the caster when the profile
declares concentration — one more prepared condition through the path
`bindCounterpart`/`prepareCondition` already walks (`action.go:376-379`, `contest.go:179-193`).
**No new machine and no new file in `resolution` beyond the damage report.**

**6. `encounter`** — `BeatConcentrationEnded = "concentration_ended"` beside `BeatCast` and
`BeatSaved` (`encounter/cast.go:21`, `:30`), its payload struct beside `castPayload` (`:103`),
and `RecordConcentrationEnded` beside `RecordCast` (`cast.go:167`), appending through the same
`appendBeat`/`audienceFor` transaction loop (`cast.go:174-186`). The result kinds are
unchanged: `ResultConditionRemoved` already exists (`activation.go:24-50`) and the roll-fact
refusal (`activation.go:465-481`) already forbids an amount on it, which is correct.

**7. `session`** — `session/concentration.go`: one helper called after every `Resolve` that
can damage (`attack.go:307`, `cast.go:295`, `mover.go:154`, `react_post_roll.go:84`) which,
for each `Output.DamageTaken` entry whose member is concentrating, runs a second `Resolve`
with `Cost: nil`, then `RecordConcentrationEnded`. `EventConcentrationEnded` and
`ConcentrationEndedBody` in `session/types.go` beside `EventCast`/`CastBody` (`:763`, `:1304`),
and the beat-string arm in the kind switch at `session/events.go:750-830`. `Participant`
gains `Concentrating bool` (R11), filled from the sheet in `session/convert.go`.
**Ordering at the seam:** the damaging interaction is saved and recorded first (`cast.go:334-341`
is explicit that save-then-record is deliberate), then the concentration interaction runs,
saves and records. Two interactions, in that order, so the log reads *damage → save → spell
ended*.

**8. protos** — additive only, no label. `EVENT_KIND_CONCENTRATION_ENDED = 29`
(`events.proto:259`), `ConcentrationEnded concentration_ended = 35` (`:369`) with
`{string caster = 1; SpellRef spell = 2; string reason = 3;}`, `bool concentrating = 8` on
`Participant` (`types.proto:900`). `reason` is an open string, following `Saved.ability`
rather than `FightEnded.cause` — the six reasons are a rulebook vocabulary, not a wire
contract, and the client renders the sentence the server authored.

**9. rpg-api** — one arm in `eventKindToProto` (`convert.go:644-649`) and one in `setEventBody`
(`:921-942`), **in the same change**, because an unmapped kind demotes silently to
`EVENT_KIND_UNKNOWN` with a nil body and the Go type switch has no exhaustiveness check
(`convert.go:640-643`). `participantToProto` (`:1732`) fills `Concentrating`. The owner's
condition view needs **no api change** — `mapStatus` (`character_data.go:136-168`) already
copies whatever the toolkit's status view carries.

**10. rpg-dnd5e-web** — a `case 'concentrationEnded'` arm in `buildOtherStory`
(`story.ts:242-247`), following `case 'cast'` (`:408-421`) verbatim: kind re-check, `memberName`,
`spellName`, frozen exchange, headline with no trailing period. **This will not fail the build
if forgotten** — the switch's declared return type includes `undefined` and there is no
`assertNever`, so the arm is added by hand. One row in `characterPresentation.ts:38-66` keyed
`dnd5e:conditions:concentrating` for the owner's badge, and a badge on `InitiativeEntry`
(`CombatExperience.tsx:77-100`) off `participant.concentrating`, following the
`participant.active` class-plus-tooltip pattern at `:87-88`.

## The seven principles, run against this

**Ownership before mechanism** — the table above; the contested nouns were *who runs the save*
(R2: the session, because resolution can neither publish damage nor nest) and *who counts the
duration* (R8: the owner, because two clocks on one spell is two answers).
**Rulings carry scope** — every ruling names what it does not settle; R3 in particular refuses
to become Counterspell's door rule and R4 refuses to become Dispel Magic.
**Load-bearing or deferred** — Incapacitated, minute durations, fan-out, Bless's area
targeting, hearing, and the cast path's chain fold are all deferred with a named trigger. R7 is
the one deliberate reach past the smallest cut and it is flagged as Kirk's call.
**Zero values tell the truth** — `CastProfile.Concentration` is a pointer, not a bool beside a
duration that means nothing when false (R8); `ImposedEffect` gains a kind rather than a
removal expressed as an application with an empty payload (R4); a member who is not
concentrating carries no condition, so `Concentrating` is false because the sheet says so.
**Call sites read as statements** — `Character.Concentration()` rather than a type assertion
over `GetConditions()`; `Output.DamageTaken` is a report the seam reads, not a hook.
**Record tells the truth same-day** — six reasons on one beat, and the save that produced one
of them is the same `SAVED` beat every other save uses.
**Fail closed loudly** — an unknown condition id is refused at the factory
(`factory.go:113-115`) and at the loader (`loader.go:211`); an unknown ref in the display
catalog is a hard error (`display.go:38-41`); a boundary kind this build does not know is
refused at `boundary.go:83-85`; and R5 turns a silent subscription leak into a removal that
actually removes.

## What the tests must prove

**Toolkit unit.** A concentrating condition round-trips through JSON with its children intact.
Ending it publishes one removal per child **and** its own, in that order. A child removed by
anything else leaves the owner's list and, when the list empties, ends the owner with reason
`spell_ended` (R6). The owner ends on its turn-end count reaching zero, on combat end, and on
long rest. `Character.Concentration()` answers false with no condition and answers the spell
ref with one. **The keeper regression (R5): a condition removed by a publish it did not make
is unsubscribed** — assert the pruned condition's chain handler no longer fires, which is a
test that fails on today's code.

**Resolution scenes.** A failed concentration save at DC `max(10, damage/2)` removes the owner
and every child; a made save removes nothing and still records roll, total and DC. A contest
with no `Removal` declared is byte-identical to today — the regression that matters. A
concentration cast while concentrating drops the old spell **after** the charge, and a cast
refused at the door drops nothing. `Output.DamageTaken` carries the applied number and
`DroppedToZero`, from both the strike path and the cast damage path.

**Encounter.** A break appends exactly one `concentration_ended` beat plus one
`condition-removed` result per child; a `condition-removed` result still refuses roll facts
(`activation.go:465-481`); an ungated break — duration, combat end — appends no `saved` beat.

**Session scenes.** A bard concentrating on True Strike is struck for 9 and rolls a CON save
at DC 10; for 30, at DC 15. A failed save ends the spell and the target's condition is gone
from the target's sheet. A second concentration cast drops the first, and the log carries the
`recast` reason. A bard downed to 0 HP loses concentration (R7). A non-concentrating member
taking damage runs **no second interaction at all** — the seam must be free when nobody is
concentrating.

**Web.** `npm run ci-check` with the log grepped for `✗`/`❌` (it exits 0 on failure). The
concentration-ended line renders through `story.ts`; the owner's badge renders; the roster
badge renders off `participant.concentrating`.

## Done-when — Kirk's walk

1. A level-1 bard with True Strike enters a fight and casts it on the skeleton. The dock and
   the log read as they do today, **and** the bard's own status now shows a concentrating
   badge naming True Strike. Other players see the roster marker.
2. The skeleton hits the bard. The log shows a **Constitution save** with its roll, total and
   a DC that is 10 for a small hit and half the damage for a big one. On a success the badge
   stays and the advantage is still there.
3. Force a failure. The log reads *the bard loses concentration on True Strike*, the badge
   clears, and the bard's **next attack against the skeleton has no advantage** — the child
   went with the parent.
4. Cast True Strike again, then cast it again on a different target without being hit. The
   first drops with reason `recast`, exactly one badge stands, and the log says which spell
   ended.
5. Let True Strike be consumed by an attack instead. Concentration ends by itself
   (`spell_ended`), the badge clears, and no save was rolled.
6. Let it run out. The badge clears at the end of the caster's next turn, and it clears again
   when the fight ends, with no stale badge on the post-fight screen.
7. Knock the bard to 0. Concentration ends (R7, if Kirk takes it).

## Shelf

- **Bless, and "choose up to N targets in an area"** — fan-out (§3.1 and §6.2), a separate
  shape and explicitly out of this slice. Concentration is ready for it: Bless is one owner
  with three children instead of one.
- **Incapacitated, and the other twelve inert core condition refs** (`refs/conditions.go:59-75`).
  R7 buys only the drop-to-zero case. Paralysis, stunning and sleep ending concentration
  arrive with the condition that causes them.
- **"Up to 1 minute" and every non-turn duration.** There is no minute clock and no round
  boundary (`turndriver.go:333-348`). Every concentration spell this slice can express counts
  turn ends. The first spell whose duration is genuinely minutes is the trigger.
- **The cast damage path folds no chain** (`contest.go:280-290`) — resistance and immunity do
  not apply to cantrip damage, and the concentration DC inherits the error. Its own toolkit
  issue, filed with this slice, fixed on its own.
- **`DamageReceivedTopic` as a fact rather than an instruction** (toolkit#977). When that
  lands, R2's report and the seam could become a subscription — and should be re-examined
  then, not before.
- **Dispel Magic and Counterspell.** R4 gives removal a delivery and R3 gives the door a
  displacement rule; neither is a verb, and Counterspell is still the door's unbuilt third
  window producer.
- **Concentration on the monster side.** R12: no monster casts, so nothing changes until one
  does.
- **A concentration save the caster can modify** — War Caster, advantage on the check.
  ADR-0029 already proposes branching on `SaveTriggerConcentration`; the trigger now has a
  driver, so the feat becomes a chain subscriber and nothing else.

## What would change the design

- **If Kirk refuses R7**, `caster_down` comes out and a downed caster keeps its badge until
  the fight ends. One reason less, no structural change, and a known lie until Incapacitated
  is built.
- **If Kirk refuses R11's roster bool**, the owner's badge still ships for free and only the
  caster sees it. The break beat then arrives with no setup for everyone else.
- **If a spell needs to end a *world fact* rather than a condition** — a wall of fire, a
  region effect — then `Removal` is carrying a vocabulary rather than an address, and the
  owner's children become a list of typed handles. That is the trigger to revisit R1, and it
  arrives with the first persistent-area spell.
- **If a second thing ever needs `Output.DamageTaken`** — a retaliation aura, a
  damage-triggered feature — then R2's report is a real seam and deserves a name of its own
  rather than a field the session reads. One customer is a field; two is a shape.
- **If toolkit#977 lands first**, R2 gets simpler: the condition subscribes to damage the way
  `raging.go:80` already does, and the session seam disappears. It is worth checking that
  issue's state before starting.

---

## Issue seeds

One block per module that changes. Written to be cut verbatim. **Not filed.**

### protos — the concentration-ended beat and the roster flag

**Assumptions.**
1. `EVENT_KIND_CONCENTRATION_ENDED = 29` is free (`events.proto:259` ends at 28) and body tag
   35 is free (`:369` ends at 34).
2. `Participant` field 8 is free (`types.proto:879-900` ends at 7).
3. Adding an enum value, a oneof arm and a new field is **non-breaking** under `FILE`
   (`buf.yaml`), so no `breaking-change-approved` label is needed.
4. `reason` is an open string; the client never branches on it for a rule.

**Definition of done.** `buf lint`, `buf format`, `buf breaking` and `buf generate` clean with
no label; generated Go and TS compile. Must survive: nothing downstream can start until the
generated packages carry the kind, the body and the field.

**Depends on.** Nothing. Merges first.

### rpg-toolkit root (`refs`, `combat/actions`, `conditions`, `character`, `monster`) — the condition, the profile arm, and the keeper fix

**Assumptions.**
1. `CastProfile` can carry a nullable `Concentration` arm without disturbing `Validate`'s
   existing refusals (`cast.go:118-135`).
2. A new condition needs exactly four registrations: factory arm, loader entry, display-catalog
   row, long-rest net.
3. `ConditionBehavior.IsApplied()` makes a second `Remove` a safe no-op, so the keeper fix does
   not disturb self-ending conditions.
4. Deleting `TrueStrikeConditionData.TurnEndsLeft` needs no migration (pre-release, no
   consumer).
5. `Character.Concentration()` reads only the sheet's own conditions — no new sheet field.

**Definition of done.** Unit tests as listed under "Toolkit unit" above, including **the keeper
regression that fails on today's code**: a condition pruned by a removal fact it did not
publish must be unsubscribed. True Strike still grants advantage against its named target and
is still consumed on use. Every condition still round-trips. Must survive: a bard finalizes and
enters a fight on the walk env with True Strike on the sheet.

**Depends on.** Nothing (toolkit-internal). Ships before resolution.

### rpg-toolkit `resolution` — the damage report, removal as a consequence, the recast drop

**Assumptions.**
1. `Output.DamageTaken` can be added additively; every existing caller ignores it.
2. `ContestInput.Application` may be nil when a `Removal` is declared, the same widening the
   cast slice made for `Damage`.
3. `castMachine` can yield a leading `Gather` before delegating to the inner machine
   (`action.go:173-191`), and that step runs after `payAtTheDoor` (`resolve.go:416-427`).
4. `DCHalfDamageFloorTen()` needs no change — `contest.go:536` already feeds it
   `DCInput{DamageTaken}`.
5. The `Negated`-only and no-recurrence refusals (`contest.go:159-164`) stay exactly as they
   are.

**Definition of done.** Scenes as listed under "Resolution scenes", with **a contest that
declares no removal byte-identical to today** as the regression that matters. A cast refused at
the door drops nothing. Must survive: walk steps 2, 3 and 4.

**Depends on.** toolkit root.

### rpg-toolkit `encounter` — `RecordConcentrationEnded`

**Assumptions.**
1. The beat is one new const plus one payload struct beside `BeatCast`/`BeatSaved`
   (`cast.go:21`, `:30`, `:103`), appended through the same transaction loop.
2. The result kinds do not change — `ResultConditionRemoved` already exists
   (`activation.go:24-50`).
3. The roll-fact refusal (`activation.go:465-481`) needs no new arm, because a removal carries
   no amount.

**Definition of done.** Tests: a break appends one `concentration_ended` beat plus one
`condition-removed` result per child, in that order; an ungated break appends no `saved` beat;
a `condition-removed` result still refuses roll facts; the beat is refused on a closed
encounter. Must survive: the walk's log reads damage → save → spell ended, in order.

**Depends on.** toolkit root, resolution.

### rpg-toolkit `session` — the seam, the beat, the roster flag

**Assumptions.**
1. Every `Resolve` that can damage is one of four call sites (`attack.go:307`, `cast.go:295`,
   `mover.go:154`, `react_post_roll.go:84`) and each can call one shared helper.
2. A second `Resolve` with `Cost: nil` from the session is the shipped pattern
   (`announcer.go:55-79`), and starting it after the first has committed avoids the nested-resolve
   refusal (`resolve.go:352-374`).
3. `Participant` is built in one place (`session/convert.go`) and the sheet is already loaded
   there.
4. A member who is not concentrating costs zero extra interactions.

**Definition of done.** Scenes as listed under "Session scenes", including **no second
interaction when nobody is concentrating**. The typed body and the beat-string arm both land,
and a break survives a session reload (the condition is on the sheet). Must survive: every step
of the walk.

**Depends on.** toolkit root, resolution, encounter.

### rpg-api — two convert arms and one participant field

**Assumptions.**
1. Both `eventKindToProto` (`convert.go:644-649`) and `setEventBody` (`:921-942`) arms land in
   the same change; omitting either compiles and demotes silently (`:640-643`).
2. `spellRefToProto` (`:1090`) already serves the body's spell field.
3. `participantToProto` (`:1732`) is the only place the roster flag is filled.
4. The owner's condition view needs **no api change** — `mapStatus`
   (`character_data.go:136-168`) copies whatever the toolkit status view carries.

**Definition of done.** Acceptance through the real session handler: a concentration break
arrives as `EVENT_KIND_CONCENTRATION_ENDED` with a populated body, **no beat in the walked
session arrives as `EVENT_KIND_UNKNOWN`**, the CON save arrives as `EVENT_KIND_SAVED`, and the
roster carries the flag. Must survive: the walk on the local env.

**Depends on.** protos, toolkit session (pinned to minted tags).

### rpg-dnd5e-web — the break line and two badges

**Assumptions.**
1. `buildOtherStory`'s switch is **not** exhaustiveness-enforced (`story.ts:242-247`, no
   `assertNever`, `noImplicitReturns` off), so the arm is added by hand and nothing fails the
   build if it is forgotten.
2. `characterPresentation.ts:38-66` is a ref-keyed table and one row gives the owner's badge.
3. `InitiativeEntry` (`CombatExperience.tsx:77-100`) is the only component that iterates
   members, and `participant.active` (`:87-88`) is the class-plus-tooltip pattern to copy.

**Definition of done.** `npm run ci-check` with the log grepped for `✗`/`❌` (it exits 0 on
failure). Playwright or a walk screenshot: the concentrating badge appears on cast and clears
on break, the roster marker shows for other members, and the log line reads *loses
concentration on True Strike* with the reason legible. Must survive: every step of the walk.

**Depends on.** protos, rpg-api.

### rpg-toolkit `resolution` (separate, filed with this slice) — cast damage folds no chain

**Assumptions.** `applyPreparedDamage` (`contest.go:291-351`) calls `combat.FinalDamage`
(`:312`) over components that were never folded through `DamageChain` (`:280-290`), unlike the
strike path (`strike.go:532`, `:879-889`, `:674`).

**Definition of done.** A resistant target takes half damage from Vicious Mockery, and the
concentration DC computed from that damage uses the halved number. Not part of the
concentration slice; concentration is correct the day this lands.

**Depends on.** Nothing. Independent of this slice.
