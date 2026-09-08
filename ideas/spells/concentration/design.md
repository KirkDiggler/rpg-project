# Concentration — one condition owns what a spell left behind

**Date:** 2026-09-08
**Status:** Design. One slice, cut. Slice three of the bard/spells initiative.
**Umbrella:** `ideas/spells/` — levels 1–3, and the choices at 3 made real.
**Brainstorm:** rpg-project#391 (`ideas/spells/brainstorm.md`) §3.2, §6.2 ("Concentration — an
owning condition", the row marked *missing* and *the largest single piece*).
**Slice two:** rpg-project#405 (`ideas/bard/cantrips/design.md`), shipped 2026-09-08 as
dnd5e v0.148.0 / encounter v0.67.0 / resolution v0.36.0 / session v0.68.0, protos #310/#311,
rpg-api#949, web#994.

Kirk's rulings that shaped this slice:

- **Concentration, alone.** One shape — an owning condition on the caster that holds child
  effects and ends them together. Its first customer is a retrofit, so the slice needs no new
  content to prove itself.
- **The machines are the shelf we build on.** Things that survived the refactor unwired are not
  a head start; they are the sign nothing needed them. Nothing here argues from a leftover.
- **It stays in resolution.** *"resolution is the place that should resolve things. it shouldn't
  have to leak out."* The check runs inside the interaction that caused it. The session starts
  nothing and decides nothing.

---

## What this slice is

A spell that says *concentration* puts a condition on its caster. That condition **owns** the
effects the spell left on the board, and when it ends, they end with it. It ends on the caster
failing a Constitution check after taking damage, on the caster casting another concentration
spell, on the spell's own duration, and on the fight ending.

The damage check is not a separate event later. **It happens inside the swing that caused it**:
the strike applies its damage, says so on the interaction's own bus, the caster's concentrating
condition answers with a request for a check, and the strike runs that check as a nested step
before it finishes. One interaction, one record: *hit → save → concentration ended*.

The first customer is **True Strike**, retrofitted. The second is named only — **Bless**,
arriving with the cleric in a collaborator's lane, whose "choose up to N targets in an area" is
fan-out and is not this shape.

## The strip — three hops, and the only part with no prior art

Everything else in this design is a machine doing what it already does with one more input.
This is the part the stack has never done: **a buff sitting on somebody else's sheet has to come
off when the caster's concentration breaks.**

Nothing in the toolkit can reach across and delete a condition from another member. There is no
`RemoveCondition` on the bus, on the combatant, or on any sheet. What exists is a **fact**:
`ConditionRemovedEvent{MemberID, ConditionRef, Reason}` (`events/events.go:689-693`), whose own
doc says *"A FACT, NOT A COMMAND… what a keeper does about that is the keeper's rule"*
(`:678-687`). So the strip is not a reach. It is three hops, each already a thing the stack
does, wired together for the first time.

```
  the owner holds ADDRESSES, not pointers
  ConcentratingConditionData{ SpellRef, TurnEndsLeft,
      Children: [ {member: skeleton-1, ref: dnd5e:conditions:true_strike} , … ] }
                    │
       hop 1        │  a nested check inside the damaging interaction fails,
                    │  and its delivery publishes one removal per address
                    ▼
  strike applies damage → publishes the damage-taken fact → the owner appends a
  follow-up → the strike yields Request(contest) → the check fails → the contest's
  delivery Gather publishes ConditionRemovedTopic per address
       (the same delivery step that already publishes an application)
                    │
       hop 2        │  the fact travels the interaction's bus
                    ▼
  each member's keeper hears its own address and honours it on its OWN sheet
       character.onConditionRemoved / monster.onConditionRemoved
       — today it drops the behavior from the slice and NEVER unsubscribes it (R5)
                    │
       hop 3        │  the owner ends itself
                    ▼
  the concentrating condition publishes its own removal and Removes,
  encounter records one concentration_ended beat carrying WHY
```

**Why addresses and not pointers.** The owner is persisted as an opaque blob in
`character.Data.Conditions []json.RawMessage` (`character/data.go:107`) and rebuilt through the
ref-keyed loader (`conditions/loader.go:17-193`). A live pointer to another member's condition
object cannot survive that round trip. A `{member, ref}` pair can, and it is already the exact
address the removal fact takes. The nearest existing shape is
`TrueStrikeConditionData{Ref, MemberID, TargetID, SourceRef, TurnEndsLeft}`
(`conditions/true_strike.go:35`) — a caster-side condition that names another member. This is
that, with a list.

**Why hop 2 is not free today, and what it costs.** `onConditionRemoved`
(`character/character.go:1195-1216`, mirror `monster/monster.go:553-573`) matches on the ref
string and drops the behavior from the sheet's slice — **it never calls `Remove(ctx, bus)`**, so
the pruned condition's subscriptions stay live. Invisible right now because the only publishers
of a removal are conditions ending *themselves*, which call their own `Remove` immediately after
publishing (`true_strike.go:275-288`). Concentration is the first thing that publishes a removal
for someone else, and the first thing that trips the leak. R5 fixes the keeper, not
concentration.

**Why the beat is separate from the removals.** A `condition-removed` result landing on a
skeleton's sheet, with no cast beat anywhere near it, reads as a random drop. The
`concentration_ended` beat is the only thing in the record that says *the spell ended, and this
is why*. R10.

## The facts this stands on

Toolkit `origin/main` (`e894540e`), protos `origin/main` (`9b7b8ef`), rpg-api `origin/dev`
(`67c06da`), web `origin/dev` (`b2a71c2a`). Every fact below is a machine, a step, or a seam
that runs on the composable stack today.

**A machine already nests another machine, on the same bus, over the same cast, and that
sameness is the point.** `Request` (`resolution/step.go:60-80`): *"the driver is what actually
runs it, on the same bus and over the same cast. That sameness is load-bearing: the sub-machine
folds its chains on the interaction's own bus, so an effect attached for this interaction
contributes to the requested save exactly as it would to a direct one."* The strike already
nests a contest for its save riders (`strike.go:752`, `:909`); the contest already nests a save
(`contest.go:646-658`). Nesting one more is not a new capability.

**A machine already yields a dynamic number of nested requests, and the codebase says why one
per item.** `movementMachine.react(i int)` (`resolution/movement.go:297-330`) walks a slice of
triggers it collected mid-interaction and returns one `Request` per trigger, resuming at `i+1`:
*"One step per trigger rather than one step resolving all of them, for the reason
boundaryMachine yields one per crossing: each is a separate thing that happened, and every yield
point is a legal suspension point."* Two concentrating casters caught by one blast are two
follow-ups and two nested checks, with no special case and no new machinery.

**A requested machine may not suspend, and the driver refuses it by name.**
`drive` (`step.go:167-176`): *"a requested machine that suspended would strand the machine that
requested it: the requester's continuation is a Go closure on this stack, and nothing serializes
it."* So a nested check is structurally automatic. R2a.

**Applying damage is deliberately bus-free, and publishing from a sheet would publish onto the
wrong bus.** `strike.go:692-693`: *"Bus-free, and the only phase that is: applying damage is the
sheet's own business and takes no bus on either a character or a monster."* And
`movementMachine.bill` (`movement.go:344-352`) states the law directly: *"A Gather rather than a
bare publish, for the reason every other publish in this package is one: the bus belongs to the
driver, and the interaction's own bus is where the reactor's condition is attached. Reaching for
a bus captured out of an earlier step would publish onto whatever bus that step happened to run
on, which is the same rule-in-the-wiring this machine exists to avoid."* R2 follows from this.

**`DamageReceivedTopic` means "damage has landed" and one subscriber treats it as an
instruction.** `strike.go:701-728` measured it — a 4-damage bite took a wolf from 11 to 3 twice
with a publish in place, because `monster.SheetKeeper` calls `TakeDamage` on it
(`monster/load.go:210`). The topic is not reusable until #977 converts Undead Fortitude. **The
new fact sidesteps it rather than waiting on it.**

**A contest is a check with a consequence, and its consequences are already a chain.**
`NewContest` (`contest.go:463`) runs Request(save) → outcome policy → deliver → Done
(`:144-215`). `SaveInput.SaverID` is resolved from the cast against both rosters
(`save.go:88-103`), so **the saver need not be the interaction's target** — a caster hit by a
skeleton saves inside the skeleton's swing. The delivery today is one `Gather` publishing a
condition (`:242-257`) and, since slice two, a sibling applying damage (`:291-351`), chained on
failure at `:613-623`. A made save returns before any delivery (`:595-597`).

**A DC that is settled before the machine runs travels as `DCStatic`.** That is slice two's R4,
already shipped: the caster answers its own spell save DC and the number rides the gate as
`saves.DCStatic` (`gate.go:100`), because the sealed `DCSource` set exists for DCs *derived at
resolution time* from a `DCInput` (`gate.go:55-97`).

**`SaveCause` is live and carries a spell ref and an instigator.**
`SaveCause{Trigger, EffectRef, InstigatorID, InstigatorType}` (`events/events.go:419-424`) is
filled by the cast slice on every cantrip save (`contest.go:453-458`, `action.go:81-85`). It is
the shape that says *this check happened because of that spell, at that creature's hands.*

**The clock is finished for two of the four triggers, and counts turn ends for the third.**
Combat end fans one `Boundary{Kind: CombatEnded, Subject: id}` **per member**
(`encounter/clocks.go:723-737`), precisely so a subject-less ending expires nothing
(`turndriver.go:355-364`), published as `CombatEndTopic` at `resolution/boundary.go:125`. Turn
end is `TurnEndTopic` (`events/events.go:1058`), and a duration today is an int of turn ends held
by the condition itself (`true_strike.go:24-31`, `:251-262`). There is **no round boundary and no
minute clock**, deliberately (`turndriver.go:333-348`, Kirk 2026-08-27).

**Conditions are the only per-member persistent state, and the sheet is their only home.**
`character.Data.Conditions` (`character/data.go:107`), built through `conditions/factory.go:39-122`
and reloaded through `conditions/loader.go:17-193`, both of which refuse an unknown id
(`:113-115`, `:211`). `EncounterData` (`encounter/data.go:26-118`) and `SessionData` hold no
condition state. There is no second store to invent.

**The record already has the beat a check produces.** `EVENT_KIND_SAVED` shipped in slice two
with `{saver, ability, roll, total, dc, succeeded, source}` (`events.proto:797`), and
`ResultConditionRemoved` is already in encounter's closed result set
(`encounter/activation.go:24-50`) with the roll-fact refusal that forbids it an amount
(`:465-481`).

**A strike's applied number is honest; a cast's is not.** The strike folds the damage chain
(`strike.go:532` → `:879-889`) and runs `combat.FinalDamage` (`:674`) before applying, so
resistance and immunity are in the number. The cast damage path **folds no chain**
(`contest.go:280-290`). R9.

**Two leftovers, and this design deletes one.** `saves.DCHalfDamageFloorTen()`
(`gate.go:113-146`) and `dnd5eEvents.SaveTriggerConcentration` (`events/events.go:411-412`) both
survived the refactor with no producer and no consumer. The DC helper is a `DCSource`, and R4
uses no `DCSource` — the condition settles the number and it travels as `DCStatic`, exactly as
slice two's spell save DC does. **So the helper is deleted in the retrofit, along with the
`DCKindHalfDamageFloorTen` row and the "concentration's max(10, damage / 2)" comment at
`gate.go:51`.** The trigger constant is a `SaveTrigger` string in `SaveCause`, which is live
machine evidence, so it is adopted as-is.

### Where the evidence contradicts the brief

**The check cannot be fed from a damage-applied path, because there is no such path**, and the
one topic that means "damage landed" is treated as a command by half the roster (above). The
brief's phrasing survives as intent; the mechanism is a new fact. R2.

**The door cannot express "you are already concentrating".** `payAtTheDoor`
(`resolution/cost.go:284-309`) knows only a `combat.SpendProfile`, whose `Requires` is *"keyed
capacity that must be PRESENT and is never spent"* and is explicitly **not a predicate language**
(`combat/spend_profile.go:47-56`, `:82-90`). And the drop is a mutation, while `Start` is
contractually pure preflight (`resolve.go:416-419`). The drop is the first yielded step **after**
the charge. R3.

## Ownership — every noun, and who holds it

| Noun | Owner | Why it cannot live anywhere else |
|---|---|---|
| The fact that damage landed | toolkit `resolution` — the machine that applied it, as a `Gather` on the interaction's bus | applying is bus-free by design (`strike.go:692-693`) and a sheet's parked bus is a captured bus (`movement.go:344-352`). Only the machine holds the driver's bus |
| The request for a check | toolkit `conditions` — the subscriber that appended it | the rule "I break on damage" is the condition's rule, and a condition is where an ongoing rule lives. It describes; it does not act |
| The roll | toolkit `resolution` — always the machine | **subscribers describe, machines roll.** A follow-up carries no dice, no roller and no outcome; the only thing that can produce a number is a machine the driver runs |
| The fact that a caster is concentrating | toolkit `conditions` — one condition on the caster's sheet | conditions are the only per-member persistent, bus-applied state (`character/data.go:107`) |
| Which spell, and which effects it left | that condition's blob, children as `{member, ref}` addresses | a pointer cannot survive the blob round trip, and the pair is already the address the removal fact takes |
| Whether a spell requires concentration | toolkit `combat/actions` — `CastProfile` | the profile is where content declares what a cast *is*, and it *"names no spell"* (`combat/actions/cast.go:49-58`) |
| How long the spell lasts | the concentrating condition, as a turn-end count | one clock per spell; two clocks is two answers to one question |
| The difficulty of the check | the concentrating condition, settled from the reported amount | the same law slice two applied to the spell save DC: a number known before the machine runs travels as `DCStatic` rather than as a formula the machine evaluates |
| Removing a child from another member's sheet | toolkit `conditions` — the removal **fact**, honoured by that member's keeper | `ConditionRemovedEvent` is *"A FACT, NOT A COMMAND"* (`events/events.go:678-687`) |
| The record of a break, and why | toolkit `encounter` — one beat | *the composition is the only author of its record* |
| Starting an interaction | toolkit `session` — and it starts **none** for concentration | Kirk's ruling. The session passes the interaction's outcome to the record and makes no decision |
| The badge the caster reads | web, off the condition view that already exists | `characterPresentation.ts:38-66` is a ref-keyed table; one row costs no proto |
| Whether other players can see it | protos — one bool on the session-lane roster row | `Participant.active` (`types.proto:883`) is the existing per-member boolean |

## Rulings

**R1 — Concentration is one condition on the caster, `dnd5e:conditions:concentrating`, and it
holds its children as `{member, ref}` addresses.**
`ConcentratingConditionData{Ref, MemberID, SpellRef, SpellName, TurnEndsLeft, Children []ChildRef}`
where `ChildRef{MemberID, ConditionRef string}`. Built by the factory
(`conditions/factory.go:70-115`, one arm) and loaded by the registry
(`conditions/loader.go:17-193`, one entry), exactly as every other condition. It subscribes to
four things: the new damage-taken fact (R2), `TurnEndTopic` (its own duration),
`CombatEndTopic` (`events/events.go:1090`), and `ConditionRemovedTopic` (`:1073`) so it notices
its own children ending. The shared long-rest net (`conditions/rest.go:15-38`) removes it like
any other.
*Scope:* one owner, N children, one spell. It is not a generic effect graph — a child that itself
owns children is a shape nobody has asked for.

**R2 — A damage-taken FACT with a return channel. Subscribers describe; machines roll. The
machine that applied the damage runs what comes back, in the same interaction.**

This is the damage chain turned around. The chain asks *"what do the rules do to damage in
flight"* and a subscriber answers by appending a modifier. The new fact says *"damage landed"*
and a subscriber answers by appending a **request for a check**. Same bus, same pattern, one
tense later.

```go
// events — a NOTIFICATION, published after the number is settled and on the sheet.
type DamageTakenEvent struct {
    MemberID      string
    Amount        int          // the applied total, post-fold
    DamageType    damage.Type
    DroppedToZero bool
    Cause         SaveCause    // what dealt it, and by whom

    FollowUps []FollowUp       // THE RETURN CHANNEL — appended by subscribers
}

// FollowUp is DATA. It describes a check somebody owes. It carries no roller,
// no dice, no outcome, and no way to obtain one.
type FollowUp struct {
    SaverID     string
    Ability     Ability
    DC          int          // already settled by whoever appended this
    Cause       SaveCause
    OnFailure   Consequence  // this slice: remove these addresses, end that owner
}
```

**The hard line, stated so it cannot be crossed later: nothing in a follow-up can roll.** A
follow-up has no `Roller` field, no bus, and no callback. The alternative that this shape exists
to forbid is a subscriber that rolls dice off the bus in its handler and reports a result — which
would put a rules decision in a subscription order, hide the roll from the record, and make the
outcome depend on who subscribed first. The channel is the seam where that could creep back in,
so the type is the guard: **a follow-up is a question, and only the driver's machines answer
questions.**

**The publish goes in the machine, not in `ApplyDamage`.** Both were live options.

- *The sheet* would cover every damage source by construction, including future traps and falls.
  But `ApplyDamage` is bus-free on purpose — *"applying damage is the sheet's own business and
  takes no bus on either a character or a monster"* (`strike.go:692-693`) — and the bus parked on
  a sheet is a **captured** bus, which `movementMachine.bill` refuses by name as
  "rule-in-the-wiring" (`movement.go:344-352`). Worse, the follow-ups have to come *back* to the
  machine that will run them, so a sheet publish means `combat.ApplyDamageResult` grows a field
  shaped like a resolution step, on an interface both `Character` and `Monster` implement. That
  is the sheet learning what a saving throw is.
- *The machine* holds the driver's bus already, is the thing that will run the follow-ups, and is
  where every other publish in the package lives. The "future traps and falls" worry is answered
  by an invariant rather than by coverage: **damage reaches a sheet only inside an interaction.**
  `ApplyDamage` has exactly two callers today, both machines, and the session starts everything
  through `Resolve`. The day something applies damage outside an interaction, that is the defect
  to fix, not a missing publish to add.

**Recommend the machine.** One shared step, named once, in `resolution/damagetaken.go`:

- `reportDamage(...) Gather` — publishes the fact on the driver's bus and keeps what came back.
- `runFollowUps(i int, next ...) Step` — one `Request(NewContest)` per follow-up, resuming at
  `i+1`, modelled on `movementMachine.react(i)` (`movement.go:297-330`) line for line.

Both damage-applying machines call the same two things: the strike right after its apply
(`strike.go:692-700`, where `afterDamage` is today), and the contest right after its cast damage
(`contest.go:291-351`). **A third damage source gets concentration by calling the same step**,
which is the whole reason it is named once.

**`Output.DamageTaken` is dropped.** The previous draft reported damage out to the session so the
session could start a second interaction. The session no longer starts one, and concentration was
that field's only reader, so it does not exist.

**The recursion bound, stated rather than discovered.** A follow-up's contest could in principle
apply damage, publish another fact, and collect more follow-ups without limit. It cannot here,
structurally: **a `Consequence` in this slice is removal only, and a contest built from a
follow-up is refused if it declares `Damage`.** Depth is exactly one. The day a consequence
damages — a retaliation aura, a shield of thorns — that refusal is what has to be replaced, and
it must be replaced with an explicit bound rather than deleted.
*Scope:* damage as a break trigger, and one consequence kind. `DamageTakenEvent` is a
notification with a typed return channel, not a general "after anything" hook.

**R2a — The check is automatic, and the machinery enforces it.**
RAW 2014 gives the caster no choice, and a nested machine **cannot** suspend: `drive` refuses a
posed sub-machine by name because *"the requester's continuation is a Go closure on this stack,
and nothing serializes it"* (`step.go:167-176`). Stated honestly: posing is available to a
top-level machine (`resolution/strike_pose.go:229`, shipped with the post-roll window), so a
future War Caster-style choice would be a redesign of where the check runs, not a flag. Today,
automatic is both the rule and the shape.

**R3 — The recast drop is the first yielded step of the new cast, after the charge.**
`castMachine.Start` runs the inner machine's Start (`resolution/action.go:173-191`) so a bad cast
is refused while `Resolve` is still in pure preflight. The drop cannot go there — Start mutates
nothing (`resolve.go:416-419`) — and it cannot go in the door. So when the definition's
`CastProfile.Concentration != nil` and the caster already carries the concentrating condition,
`castMachine` yields one leading `Gather` named *"drop concentration on &lt;spell&gt;"* that
publishes the removals, then proceeds. The ordering is the rule: **preflight → charge → drop the
old spell → resolve the new one** (`resolve.go:416-427`). A cast refused at the door drops
nothing.
*Scope:* a concentration cast displacing a concentration cast. It says nothing about Counterspell
or any other declaration-time interrupt.

**R4 — The check is a nested contest, and the removal is its consequence. No new machine.**

A follow-up is a `ContestInput` in all but name, and the machine turns it into one. Designed from
the contest outward:

| What the check needs | What the contest already does | Delta |
|---|---|---|
| roll a CON save for a member who is not the target | `SaverID` resolved from the cast against both rosters (`save.go:88-103`), rolled by `NewSave` as a `Request` (`contest.go:646-658`) | none |
| a DC settled before the machine runs | takes a gate whose `DCSource` may be `DCStatic` (`gate.go:100`) — slice two's spell save DC arrives this way | none. The condition settles the number; R4's formula is below |
| success means nothing happens | `Negated`-only policy, returning before any delivery (`:159-161`, `:595-597`) | none. The refusals at `:159-164` stay exactly as they are |
| failure strips the owner and its children | one delivery `Gather` publishing a condition applied (`:242-257`), a sibling applying damage (`:291-351`), chained at `:613-623` | **a third sibling that publishes removals** |

So the widening is additive and small:

| Type | Today | Delta |
|---|---|---|
| `ImposedEffectKind` (`contest.go:63-71`) | `ImposedCondition`, `ImposedDamage` | one row, `ImposedConditionRemoved` |
| `ContestInput` (`contest.go:25-56`) | `Gate, SaverID, Application, Damage, SourceName, Cause, DamageTaken, Roller` | one field, `Removal *ConditionRemoval{Addresses []ChildRef, Owner ChildRef, Reason string}` |
| the delivery chain (`contest.go:613-623`) | damage, then condition | damage, then condition, then removals, publishing `ConditionRemovedTopic` per address and last for the owner |
| `Application` | required | may be nil when a `Removal` or `Damage` is declared — the same widening slice two made for damage |
| construction from a follow-up | n/a | **refused if it declares `Damage`** — R2's recursion bound |

**The DC, ruled here: `max(10, floor(damage taken / 2))`, from the applied amount.** That is RAW
2014, and this slice is choosing it, not inheriting it. The concentrating condition computes it
in its handler — it was handed the amount by the fact, and arithmetic is description, not rolling
— and the number rides the gate as `DCStatic`. **This is why `DCHalfDamageFloorTen` is deleted
rather than adopted:** it is a `DCSource`, and a `DCSource` exists for a DC the machine derives at
resolution time from a `DCInput` (`gate.go:55-97`). Here nothing is derived at resolution time.
Keeping it would be a second place a rule could live.

The cause is `SaveCause{Trigger: SaveTriggerConcentration, EffectRef: <the spell>, InstigatorID:
<whatever dealt the damage>}`, filled the way the cast slice fills it (`action.go:81-85`), so the
saved beat says which spell was at stake and who threatened it.
*Scope:* removal as a consequence of a check. It is not a Dispel Magic verb and it is not a
cross-actor removal API — the delivery publishes facts, and each keeper honours them.

**R5 — The keeper must call `Remove` on a condition it prunes. Hop 2 of the strip is broken
today, and concentration is the first thing that walks it.**
`onConditionRemoved` (`character/character.go:1195-1216`, mirror `monster/monster.go:553-573`)
matches on ref string and drops the behavior from the slice — it never calls `Remove(ctx, bus)`,
so the pruned condition's subscriptions stay live. Invisible today because only self-ending
conditions publish, and they call their own `Remove` right after (`true_strike.go:275-288`). The
moment an owner ends someone else's condition, the child keeps listening from a list it is no
longer in.
Fix: **if `cond.IsApplied()`, call `cond.Remove(ctx, bus)` before dropping it.** `IsApplied()` is
already on the interface (`events/events.go:138-159`), so the self-ending path is unchanged.
*The special case above* is the concentrating condition holding live pointers to its children and
calling `Remove` itself, which cannot survive the JSON round trip. *The primitive below* is the
removal fact meaning what it says on every sheet. Recommend the primitive.

**R6 — Concentration ends when its last child ends, and the beat says so.**
The owner subscribes to `ConditionRemovedTopic` and drops a child from its list when that child's
address is removed by anything — consumed on use, expired, dispelled. When the list empties, the
spell is over and the owner ends itself with reason `spell_ended`.
Without this, a bard whose True Strike was consumed by an attack still reads as concentrating and
still drops "nothing" on the next concentration cast — a visible lie, and exactly the shape of
affordance-with-nothing-behind-it that slice one's walk kept finding.
*Scope:* the owner watches its own children and nothing else.

**R7 — Four reasons, and `caster_down` is the fourth. Incapacitated as a condition is deferred.**
The reasons are `damage` (a failed check), `recast`, `duration`, `combat_end`, `spell_ended` (R6)
and `caster_down`. `Incapacitated` is a ref with **no behavior at all** — no file, no factory arm,
no loader entry (`refs/conditions.go:64` and nothing else) — and building the general condition is
a different slice.
**Under R2 this now costs nothing outside the condition.** `DroppedToZero` rides the damage-taken
fact, so the concentrating condition reads it in the handler it already has and ends **with no
check at all** — no follow-up, no nested contest, no save beat, because a caster at 0 does not
roll to keep a spell. Zero lines outside `concentrating.go`.
**Recommend taking it.** The cost of leaving it out is a downed bard who concentrates forever,
which the walk will find on its first knockdown. **Open ruling — this is the one place the slice
reaches past its smallest cut, and it is Kirk's call.**
*Scope:* dropping to 0 HP. Paralysis, stunning, sleep and the general Incapacitated condition stay
on the shelf with the rest of the thirteen inert refs.

**R8 — True Strike's retrofit moves the clock onto the owner and deletes the divergence.**
`CastProfile` gains `Concentration *CastConcentration{TurnEnds int}` — a pointer, because a `bool`
plus a duration field that means nothing when the bool is false is a zero value that lies. True
Strike declares `{TurnEnds: 2}`, the count it carries today (`TrueStrikeTurnEnds = 2`,
`true_strike.go:24-31`), and **`TrueStrikeConditionData.TurnEndsLeft` and its turn-end
subscription are deleted** (`:40`, `:131`, `:251-262`). The child keeps consumed-on-use (`:245`),
which is a trigger and not a clock. One spell, one duration, one answer. The doc comment at
`true_strike.go:58-65` — *"THERE IS NO CONCENTRATION… the day a levelled spell needs it is the day
it has to exist"* — is deleted, not amended.
Persisted True Strike blobs from before this change lose a field. Pre-release, no consumer, no
migration.
*Scope:* True Strike. Vicious Mockery is not a concentration spell and does not change.

**R9 — The DC uses the applied amount, and the cast path's missing chain fold is recorded as a gap
rather than fixed here.**
The fact carries the number the sheet actually took — `applied.TotalDamage` (`strike.go:698`) for
a strike, `contest.go:335`'s applied total for cast damage. From a strike that number is already
post-resistance and post-immunity (`FinalDamage` at `:674`, after the fold at `:879-889`). From a
cast **it is not**: the cast damage path folds no chain (`contest.go:280-290`), so a resistant
caster hit by a cantrip is asked for a higher DC than RAW allows.
Under R2 this now reaches concentration through one field of one event, which makes the gap
cheaper to describe and no less real. That is a slice-two defect with a wider blast radius than
concentration, and fixing it here would make the largest diff in the slice be in someone else's
shape. **Recorded, with its own toolkit issue, named on the shelf.** The concentration code reads
the reported number and is correct the day the fold lands.

**R10 — One new beat kind, and the check rides the beat that already exists.**
`EVENT_KIND_CONCENTRATION_ENDED = 29` with `ConcentrationEnded{caster, SpellRef spell, reason}` at
body tag 35 — both confirmed free (`events.proto:259`, `:369`) and both additive under the repo's
`FILE` breaking policy (`buf.yaml`, gate at `.github/workflows/ci.yml:36-43`), so **no
`breaking-change-approved` label is needed.** The check itself needs nothing new:
`EVENT_KIND_SAVED` shipped in slice two with exactly the right fields (`events.proto:797`), and the
stripped children are `ResultConditionRemoved`, already in encounter's closed result set.
Because the check is nested, the beats are **one train from one interaction**: struck, then saved,
then concentration_ended, then one condition-removed per address — in the order the machine
produced them, appended by the same `RecordAttack`/`RecordCast` call the session already makes.
**Why a dedicated kind rather than inferring the break from the removals:** three of the six
reasons produce no check at all, and hop 2 lands removals on *other members'* sheets, where they
read as random drops.
*Scope:* the break. There is no "concentration started" beat — the cast beat already is one.

**R11 — Other players see a concentrating member through one bool on the roster lane; the caster
sees a badge for free.**
The caster's own view costs **zero proto**: `character.StatusView.Conditions`
(`character/status_view.go:115`) already reaches `CharacterData.conditions`
(`encounter/types.proto:440`) and renders through a ref-keyed table
(`characterPresentation.ts:38-66`), so one row keyed `dnd5e:conditions:concentrating` is the whole
client change. That path needs the condition added to the display catalog (`conditions/display.go:43`),
which is a hard error on an unknown ref.
For everyone else there is nothing: `Participant` (`types.proto:879`) carries no conditions and no
flags, and `PublicMemberInfo` says in its own comment that it is identity, not per-turn state. So
`bool concentrating = 8` on `Participant`, mirroring `bool active = 5`.
**Recommend taking it.** A break beat about a member whose concentrating state was never visible is
a beat with no setup. **Open ruling — the alternative is to defer it and ship the caster's badge
alone.**

**R12 — Monsters do not concentrate, and nothing pretends they do.**
No monster casts today; `spells.CastDefinition` is reached only from `session/casts.go:83` over a
character's known cantrips. `Monster` gets the same keeper fix as `Character` (R5), because that
fix is about the removal fact and not about casting. When a monster casts, it holds the same
condition on the same sheet field (`monster/data.go:42`) with no design change.

## Shape, per module (bottom-up)

**1. `refs`** — `Conditions.Concentrating()` beside the fifteen core rows
(`refs/conditions.go:59-75`); id `concentrating`.

**2. `events`** — `DamageTakenTopic` beside the other topic constants (`events.go:1055-1090`),
`DamageTakenEvent` with its `FollowUps` return channel, `FollowUp`, and the `Consequence` type
(R2). `SaveTriggerConcentration` (`:411-412`) gets its first producer. No dependency on `saves` or
`resolution`, which is why the follow-up's DC is a settled `int` rather than a `DCSource`.

**3. `combat/actions`** — `CastConcentration{TurnEnds int}` and
`CastProfile.Concentration *CastConcentration` (`cast.go:65-86`), with arms in `Validate`
(`:118-135`) and `Clone`. Nothing in the type names a spell.

**4. `conditions`** — `concentrating.go`. Blob per R1; `Apply` subscribes `DamageTakenTopic`,
`TurnEndTopic`, `CombatEndTopic` and `ConditionRemovedTopic`. The damage handler matches its own
member id, reads `DroppedToZero` (R7) and otherwise appends **one** follow-up carrying
`{SaverID: self, Ability: CON, DC: max(10, amount/2), Cause, OnFailure: remove(children) + end(self, damage)}`.
`end(ctx, reason)` publishes one `ConditionRemovedEvent` per child address and then its own,
following `vicious_mockery.go:232-245` verbatim. One factory arm, one loader entry, one
display-catalog row (`display.go:43-80`), and the long-rest net. `true_strike.go` loses
`TurnEndsLeft` and its turn-end subscription (R8).

**5. `character` / `monster`** — hop 2: `onConditionRemoved` calls `Remove` when `IsApplied()`
(`character/character.go:1195-1216`, `monster/monster.go:553-573`) — R5, and the one change in this
slice that fixes a defect rather than adding a shape.
`Character.Concentration() (ConcentrationView, bool)` reads its own conditions
(`character.go:590-592`) and answers spell ref, name and child count, for R3's door check and
R11's flag. **The caster answers whether it is concentrating**; no caller type-asserts over
`GetConditions()`. `ApplyDamage` is unchanged and stays bus-free.

**6. `resolution`** — `damagetaken.go`: `reportDamage` (a `Gather` publishing the fact on the
driver's bus and keeping the follow-ups) and `runFollowUps(i)` (one `Request(NewContest)` per
follow-up, resuming at `i+1`), modelled on `movementMachine.react` (`movement.go:297-330`). The
strike calls both where `afterDamage` sits today (`strike.go:700-728`); the contest calls both
after its cast damage (`contest.go:291-351`). `ImposedConditionRemoved` and
`ContestInput.Removal` with the delivery chained at `:613-623`, and the construction refusal on a
follow-up that declares damage (R2's bound). `castMachine` gains its leading drop `Gather` (R3),
and `newCast` (`action.go:67-105`) applies the concentrating condition to the caster when the
profile declares concentration — one more prepared condition through the path
`bindCounterpart`/`prepareCondition` already walks (`action.go:376-379`, `contest.go:179-193`).
`StrikeOutcome` and `CastOutcome` gain `FollowUps []FollowUpOutcome{Save SaveOutcome, Ended *ConcentrationEnded}`
so the record can be written from the outcome. **No new machine, and no `Output.DamageTaken`.**

**7. `encounter`** — `BeatConcentrationEnded = "concentration_ended"` beside `BeatCast` and
`BeatSaved` (`cast.go:21`, `:30`), its payload struct beside `castPayload` (`:103`), and the
follow-up beats appended inside the existing `RecordAttack`/`RecordCast` transaction loops
(`cast.go:174-186`) so the whole train is one record. Result kinds unchanged:
`ResultConditionRemoved` already exists (`activation.go:24-50`) and the roll-fact refusal
(`:465-481`) already forbids it an amount, which is correct.

**8. `session`** — **the session starts no interaction and makes no decision.** It gains
`EventConcentrationEnded` and `ConcentrationEndedBody` in `session/types.go` beside
`EventCast`/`CastBody` (`:763`, `:1304`), the beat-string arm in the kind switch at
`session/events.go:750-830`, and `Participant.Concentrating` filled from the sheet in
`session/convert.go` (R11). The outcome-to-beats mapping it already does for a strike and a cast
(`session/castoutcome.go`) grows one arm for the follow-ups. That is the whole session diff, and
none of it resolves anything.

**9. protos** — additive only, no label. `EVENT_KIND_CONCENTRATION_ENDED = 29`
(`events.proto:259`), `ConcentrationEnded concentration_ended = 35` (`:369`) with
`{string caster = 1; SpellRef spell = 2; string reason = 3;}`, and `bool concentrating = 8` on
`Participant` (`types.proto:900`). `reason` is an open string, following `Saved.ability` rather
than `FightEnded.cause` — the six reasons are a rulebook vocabulary, not a wire contract.

**10. rpg-api** — one arm in `eventKindToProto` (`convert.go:644-649`) and one in `setEventBody`
(`:921-942`), **in the same change**, because an unmapped kind demotes silently to
`EVENT_KIND_UNKNOWN` with a nil body and the Go type switch has no exhaustiveness check
(`convert.go:640-643`). `participantToProto` (`:1732`) fills `Concentrating`. The caster's
condition view needs **no api change** — `mapStatus` (`character_data.go:136-168`) copies whatever
the toolkit status view carries.

**11. rpg-dnd5e-web** — a `case 'concentrationEnded'` arm in `buildOtherStory`
(`story.ts:242-247`), following `case 'cast'` (`:408-421`) verbatim. **This will not fail the build
if forgotten** — the switch's declared return type includes `undefined` and there is no
`assertNever`, so the arm is added by hand. One row in `characterPresentation.ts:38-66` keyed
`dnd5e:conditions:concentrating`, and a badge on `InitiativeEntry` (`CombatExperience.tsx:77-100`)
off `participant.concentrating`, following the `participant.active` class-plus-tooltip pattern at
`:87-88`.

## The seven principles, run against this

**Ownership before mechanism** — the table above, and three nouns option D forced apart that a
subscriber-rolls design would have merged: **the fact** belongs to the machine that applied the
damage, because only it holds the driver's bus; **the request for a check** belongs to the
condition, because "I break on damage" is the condition's rule; **the roll** belongs to the
machine, always. Subscribers describe, machines roll.
**Rulings carry scope** — every ruling names what it does not settle. R2 names its recursion
bound; R3 refuses to become Counterspell's door rule; R4 refuses to become Dispel Magic.
**Load-bearing or deferred** — Incapacitated, minute durations, fan-out, Bless's area targeting,
hearing, and the cast path's chain fold are all deferred with a named trigger. R7 is the one
deliberate reach past the smallest cut and now costs nothing outside one file. `DCHalfDamageFloorTen`
is deleted rather than kept "in case", and `Output.DamageTaken` is not added at all.
**Zero values tell the truth** — `CastProfile.Concentration` is a pointer, not a bool beside a
duration that means nothing when false (R8); `ImposedEffect` gains a kind rather than a removal
expressed as an application with an empty payload (R4); a `FollowUp` with no consequence is not
expressible, because the consequence is the reason it exists.
**Call sites read as statements** — `reportDamage` then `runFollowUps` reads as "say what landed,
then answer what came back"; `Character.Concentration()` rather than a type assertion over
`GetConditions()`.
**Record tells the truth same-day** — one interaction, one beat train: struck, saved,
concentration_ended, condition-removed per address, in the order the machine produced them.
**Fail closed loudly** — a follow-up that declares damage is refused at construction (R2's bound);
an unknown condition id is refused at the factory (`factory.go:113-115`) and the loader
(`loader.go:211`); an unknown ref in the display catalog is a hard error (`display.go:38-41`); a
requested machine that poses is refused by name (`step.go:167-176`); and R5 turns a silent
subscription leak into a removal that actually removes.

## What the tests must prove

**Toolkit unit.** The owner round-trips through JSON with its child addresses intact. Its damage
handler appends **exactly one** follow-up when the member id matches and **none** when it does
not, with `DC == max(10, amount/2)` across 1, 9, 20 and 31 damage. It appends **no follow-up and
ends immediately** when `DroppedToZero` is set (R7). Ending it publishes one removal per address
**and** its own, in that order. A child removed by anything else leaves the list, and when the
list empties the owner ends with reason `spell_ended` (R6). It ends on its turn-end count, on
combat end and on long rest. **The hop-2 regression (R5): a condition pruned by a removal it did
not publish is unsubscribed** — assert the pruned condition's chain handler no longer fires. That
test fails on today's code.

**Resolution scenes.** A strike against a concentrating caster publishes the fact once, yields
one nested `Request`, and the failed check removes the owner and every child — all inside one
`Resolve`, with no second interaction started by anyone. A made check removes nothing and still
records roll, total and DC. **Two concentrating members damaged by one interaction produce two
follow-ups and two nested checks**, in append order. A member who is not concentrating produces
**zero** follow-ups and zero nested steps. A follow-up that declares damage is refused at
construction. A contest with no `Removal` declared is byte-identical to today — the regression
that matters. A concentration cast while concentrating drops the old spell **after** the charge,
and a cast refused at the door drops nothing.

**Encounter.** One interaction appends struck, saved, concentration_ended and one
condition-removed per address, in that order; an ungated break — duration, combat end,
spell_ended, caster_down — appends no `saved` beat; a `condition-removed` result still refuses
roll facts (`activation.go:465-481`).

**Session scenes.** A bard concentrating on True Strike is struck for 9 and rolls a CON save at DC
10; for 30, at DC 15. A failed check ends the spell and **the target's condition is gone from the
target's sheet** — the strip, end to end. The whole thing arrives as one story train from one
`Attack` call. A second concentration cast drops the first with reason `recast`. A bard downed to
0 loses concentration with no save rolled (R7). A break survives a session reload.

**Web.** `npm run ci-check` with the log grepped for `✗`/`❌` (it exits 0 on failure). The
concentration-ended line renders through `story.ts`; the caster's badge renders; the roster badge
renders off `participant.concentrating`.

## Done-when — Kirk's walk

1. A level-1 bard with True Strike enters a fight and casts it on the skeleton. The dock and the
   log read as they do today, **and** the bard's own status now shows a concentrating badge naming
   True Strike. Other players see the roster marker.
2. The skeleton hits the bard. **In the same log entry as the hit**, a Constitution check appears
   with its roll, total, and a DC that is 10 for a small hit and half the damage for a big one. On
   a success the badge stays and the advantage is still there.
3. Force a failure. The same entry continues: *the bard loses concentration on True Strike*, the
   badge clears, and the bard's **next attack against the skeleton has no advantage** — the child
   went with the parent. That is the strip, on screen, inside the swing that caused it.
4. Cast True Strike, then cast it again on a different target without being hit. The first drops
   with reason `recast`, exactly one badge stands, and the log says which spell ended.
5. Let True Strike be consumed by an attack instead. Concentration ends by itself (`spell_ended`),
   the badge clears, and no check was rolled.
6. Let it run out. The badge clears at the end of the caster's next turn, and clears again when
   the fight ends, with no stale badge on the post-fight screen.
7. Knock the bard to 0. Concentration ends with **no save rolled** (R7, if Kirk takes it).

## Shelf

- **Bless, and "choose up to N targets in an area"** — fan-out (§3.1, §6.2), a separate shape.
  Concentration is ready for it: Bless is one owner with three addresses instead of one.
- **Incapacitated, and the other twelve inert core condition refs** (`refs/conditions.go:59-75`).
  R7 buys only the drop-to-zero case, and buys it inside the condition.
- **"Up to 1 minute" and every non-turn duration.** No minute clock and no round boundary
  (`turndriver.go:333-348`).
- **The cast damage path folds no chain** (`contest.go:280-290`) — resistance does not apply to
  cantrip damage and the DC inherits the error. Its own toolkit issue, filed with this slice.
- **A second consequence kind, and the recursion bound that comes with it.** A consequence that
  deals damage would republish the fact from inside a nested contest. R2 refuses damage on a
  follow-up today; the first retaliation effect has to replace that refusal with an explicit
  depth bound.
- **`DamageReceivedTopic` retiring** (toolkit#977). When Undead Fortitude converts, the old topic
  and the new fact should be reconciled into one — but the return channel is the part worth
  keeping, and the reconciliation must not turn subscribers back into rollers.
- **Dispel Magic and Counterspell.** R4 gives removal a delivery and R3 gives the door a
  displacement rule; neither is a verb.
- **Concentration on the monster side.** R12: nothing changes until a monster casts.
- **A modifiable concentration check** — War Caster, advantage on the roll — is a
  `SavingThrowChain` subscriber on the nested save, which folds on the interaction's own bus
  (`step.go:60-68`), so it works the day the feat exists.

## What would change the design

- **If Kirk refuses R7**, `caster_down` comes out and a downed caster keeps its badge until the
  fight ends. One reason less, no structural change, and a known lie until Incapacitated exists.
- **If Kirk refuses R11's roster bool**, the caster's badge still ships for free and only they see
  it. The break beat then arrives with no setup for everyone else.
- **If a second subscriber ever appends a follow-up** — a trap that triggers a Dex save on damage,
  a feature that checks on being bloodied — then `DamageTakenEvent` is a real seam rather than
  concentration's private channel, and the follow-up vocabulary deserves its own ADR. One customer
  is a field; two is a shape.
- **If a consequence ever needs to damage**, R2's refusal is what has to move, and it must move to
  an explicit depth bound rather than be deleted.
- **If a spell needs to end a *world fact* rather than a condition** — a wall of fire, a region
  effect — then `Removal` is carrying a vocabulary rather than an address, and the owner's children
  become a list of typed handles. That is the trigger to revisit R1, and it arrives with the first
  persistent-area spell.
- **If the DC ruling changes** — a variant table, a flat DC — only the condition's handler moves.
  The fact, the follow-up, the contest and the beat are all indifferent to what number was
  settled, which is the point of settling it in one place.

---

## Issue seeds

One block per module that changes. Written to be cut verbatim. **Not filed.**

### protos — the concentration-ended beat and the roster flag

**Assumptions.**
1. `EVENT_KIND_CONCENTRATION_ENDED = 29` is free (`events.proto:259` ends at 28) and body tag 35 is
   free (`:369` ends at 34).
2. `Participant` field 8 is free (`types.proto:879-900` ends at 7).
3. Adding an enum value, a oneof arm and a field is **non-breaking** under `FILE` (`buf.yaml`), so
   no `breaking-change-approved` label is needed.
4. `reason` is an open string; the client never branches on it for a rule.

**Definition of done.** `buf lint`, `buf format`, `buf breaking` and `buf generate` clean with no
label; generated Go and TS compile. Must survive: nothing downstream can start until the generated
packages carry the kind, the body and the field.

**Depends on.** Nothing. Merges first.

### rpg-toolkit root (`events`, `refs`, `combat/actions`, `conditions`, `character`, `monster`) — the fact, the owner, the profile arm, and hop 2

**Assumptions.**
1. `events` may not import `saves` or `resolution`, so `FollowUp.DC` is a settled `int` and the
   consequence is plain data.
2. `DamageTakenTopic` is a new topic and touches none of `DamageReceivedTopic`'s five subscribers
   (toolkit#977 is untouched).
3. `CastProfile` can carry a nullable `Concentration` arm without disturbing `Validate`'s existing
   refusals (`cast.go:118-135`).
4. A new condition needs exactly four registrations: factory arm, loader entry, display-catalog
   row, long-rest net.
5. `ConditionBehavior.IsApplied()` makes a second `Remove` a safe no-op, so the keeper fix does not
   disturb self-ending conditions.
6. Deleting `TrueStrikeConditionData.TurnEndsLeft` needs no migration (pre-release, no consumer).
7. `saves.DCHalfDamageFloorTen`, `DCKindHalfDamageFloorTen` and their tests are deleted with no
   remaining reference.

**Definition of done.** The toolkit unit tests above, including **the hop-2 regression that fails
on today's code**. The DC table (1, 9, 20, 31 damage) is asserted. True Strike still grants
advantage against its named target and is still consumed on use. Every condition still
round-trips. `go build ./...` after the DC helper is deleted. Must survive: a bard finalizes and
enters a fight on the walk env with True Strike on the sheet.

**Depends on.** Nothing (toolkit-internal). Ships before resolution.

### rpg-toolkit `resolution` — publish the fact, run the follow-ups, deliver removals, drop on recast

**Assumptions.**
1. `reportDamage` and `runFollowUps` live in one new file and are called by both damage-applying
   machines, at `strike.go:700` and after `contest.go:291-351`.
2. `runFollowUps(i)` may be modelled on `movementMachine.react(i)` (`movement.go:297-330`) without
   change of shape.
3. `ContestInput.Application` may be nil when a `Removal` is declared, the same widening slice two
   made for `Damage`.
4. A contest built from a follow-up is refused if it declares `Damage` — the recursion bound.
5. `castMachine` can yield a leading `Gather` before delegating to the inner machine
   (`action.go:173-191`), and that step runs after `payAtTheDoor` (`resolve.go:416-427`).
6. The `Negated`-only and no-recurrence refusals (`contest.go:159-164`) stay exactly as they are.
7. `Output` gains **nothing**; the follow-up outcomes ride `StrikeOutcome`/`CastOutcome`.

**Definition of done.** The resolution scenes above, with **a contest that declares no removal
byte-identical to today** as the regression that matters, and **zero nested steps when nobody is
concentrating**. Two concentrating members in one interaction produce two nested checks. A
follow-up declaring damage is refused. Must survive: walk steps 2, 3 and 4.

**Depends on.** toolkit root.

### rpg-toolkit `encounter` — the follow-up beats in the existing record calls

**Assumptions.**
1. The beat is one new const plus one payload struct beside `BeatCast`/`BeatSaved` (`cast.go:21`,
   `:30`, `:103`), appended inside the existing `RecordAttack`/`RecordCast` transaction loops so
   the train is one record.
2. The result kinds do not change — `ResultConditionRemoved` already exists
   (`activation.go:24-50`).
3. The roll-fact refusal (`activation.go:465-481`) needs no new arm, because a removal carries no
   amount.

**Definition of done.** One interaction appends struck, saved, concentration_ended and one
condition-removed per address, in that order; an ungated break appends no `saved` beat; a
`condition-removed` result still refuses roll facts; the beat is refused on a closed encounter.
Must survive: the walk's log reads the whole break inside the hit that caused it.

**Depends on.** toolkit root, resolution.

### rpg-toolkit `session` — projection only, no interaction

**Assumptions.**
1. The session **starts no interaction** for concentration and makes no decision about one.
2. The outcome-to-beats mapping it already does (`session/castoutcome.go`) grows one arm for
   follow-up outcomes.
3. `Participant` is built in one place (`session/convert.go`) and the sheet is already loaded
   there.

**Definition of done.** The session scenes above: the whole break arrives as one story train from
one `Attack` call, the typed body and the beat-string arm both land, and a break survives a
session reload. **Assert that no code path in `session` constructs a concentration check.** Must
survive: every step of the walk.

**Depends on.** toolkit root, resolution, encounter.

### rpg-api — two convert arms and one participant field

**Assumptions.**
1. Both `eventKindToProto` (`convert.go:644-649`) and `setEventBody` (`:921-942`) arms land in the
   same change; omitting either compiles and demotes silently (`:640-643`).
2. `spellRefToProto` (`:1090`) already serves the body's spell field.
3. `participantToProto` (`:1732`) is the only place the roster flag is filled.
4. The caster's condition view needs **no api change** — `mapStatus` (`character_data.go:136-168`)
   copies whatever the toolkit status view carries.

**Definition of done.** Acceptance through the real session handler: a break arrives as
`EVENT_KIND_CONCENTRATION_ENDED` with a populated body, **no beat in the walked session arrives as
`EVENT_KIND_UNKNOWN`**, the CON check arrives as `EVENT_KIND_SAVED`, and the roster carries the
flag. Must survive: the walk on the local env.

**Depends on.** protos, toolkit session (pinned to minted tags).

### rpg-dnd5e-web — the break line and two badges

**Assumptions.**
1. `buildOtherStory`'s switch is **not** exhaustiveness-enforced (`story.ts:242-247`, no
   `assertNever`, `noImplicitReturns` off), so the arm is added by hand and nothing fails the build
   if it is forgotten.
2. `characterPresentation.ts:38-66` is a ref-keyed table and one row gives the caster's badge.
3. `InitiativeEntry` (`CombatExperience.tsx:77-100`) is the only component that iterates members,
   and `participant.active` (`:87-88`) is the class-plus-tooltip pattern to copy.

**Definition of done.** `npm run ci-check` with the log grepped for `✗`/`❌` (it exits 0 on
failure). Playwright or a walk screenshot: the concentrating badge appears on cast and clears on
break, the roster marker shows for other members, and the hit, the check and the break read as one
run of log entries. Must survive: every step of the walk.

**Depends on.** protos, rpg-api.

### rpg-toolkit `resolution` (separate, filed with this slice) — cast damage folds no chain

**Assumptions.** `applyPreparedDamage` (`contest.go:291-351`) calls `combat.FinalDamage` (`:312`)
over components that were never folded through `DamageChain` (`:280-290`), unlike the strike path
(`strike.go:532`, `:879-889`, `:674`).

**Definition of done.** A resistant target takes half damage from Vicious Mockery, and the
concentration DC computed from that damage uses the halved number. Not part of the concentration
slice; concentration is correct the day this lands.

**Depends on.** Nothing. Independent of this slice.
