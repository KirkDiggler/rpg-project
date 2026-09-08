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

Kirk's ruling on the recut, and the frame for everything below: **the machines are the shelf
we build on.** Things that survived the refactor unwired are not a head start — they are the
sign that nothing needed them. Nothing in this design argues from a leftover.

---

## What this slice is

A spell that says *concentration* puts a condition on its caster. That condition **owns** the
effects the spell left on the board, and when it ends, they end with it. It ends on the caster
failing a Constitution check after taking damage, on the caster casting another concentration
spell, on the spell's own duration, and on the fight ending. The first customer is **True
Strike**, retrofitted; the second is named only — **Bless**, arriving with the cleric in a
collaborator's lane, whose "choose up to N targets in an area" is fan-out and is not this
shape.

## The strip — three hops, and the only part with no prior art

Everything else in this design is a machine doing what it already does with one more input.
This is the part the stack has never done: **a buff sitting on somebody else's sheet has to
come off when the caster's concentration breaks.**

Nothing in the toolkit can reach across and delete a condition from another member. There is
no `RemoveCondition` on the bus, on the combatant, or on any sheet. What exists is a **fact**:
`ConditionRemovedEvent{MemberID, ConditionRef, Reason}` (`events/events.go:689-693`), whose
own doc says *"A FACT, NOT A COMMAND… what a keeper does about that is the keeper's rule"*
(`:678-687`). So the strip is not a reach. It is three hops, each one already a thing the
stack does, wired together for the first time.

```
  the owner holds ADDRESSES, not pointers
  ConcentratingConditionData{ SpellRef, TurnEndsLeft,
      Children: [ {member: skeleton-1, ref: dnd5e:conditions:true_strike} , … ] }
                    │
       hop 1        │  the contest delivers a removal
                    ▼
  contest failure → one Gather publishes ConditionRemovedTopic per address
       (the same delivery step that already publishes an application)
                    │
       hop 2        │  the fact travels the bus
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
string and drops the behavior from the sheet's slice — **it never calls `Remove(ctx, bus)`**,
so the pruned condition's subscriptions stay live. That is invisible right now because the only
publishers of a removal are conditions ending *themselves*, which call their own `Remove`
immediately after publishing (`true_strike.go:275-288`). Concentration is the first thing that
publishes a removal for someone else, and it is the first thing that trips the leak. R5 fixes
the keeper, not concentration.

**Why the beat is separate from the removals.** A `condition-removed` result landing on a
skeleton's sheet, with no cast beat anywhere near it, reads as a random drop. The
`concentration_ended` beat is the only thing in the record that says *the spell ended, and this
is why*. R10.

## The facts this stands on

Toolkit `origin/main` (`e894540e`), protos `origin/main` (`9b7b8ef`), rpg-api `origin/dev`
(`67c06da`), web `origin/dev` (`b2a71c2a`). Four read-only surveys. Every fact below is a
machine, a step, or a seam that runs on the composable stack today.

**The contest is a check with a consequence, and its DC already comes from a number the
caller supplies.** `NewContest` (`contest.go:463`) runs Request(save) → outcome policy →
deliver → Done (`:144-215`). Its DC is not a constant: `contest.go:536` reads
`m.in.Gate.DC.DC(saves.DCInput{DamageTaken: m.in.DamageTaken})`, where `DamageTaken` is a
field the *caller* fills (`ContestInput`, `:25-56`). So a check whose difficulty is a function
of damage is a shape the machine already has — what it has never had is a caller who wanted
one, and a delivery that **removes** instead of applying. The delivery today is one `Gather`
publishing a condition (`contest.go:242-257`) and, since slice two, a sibling applying damage
(`:291-351`), chained on failure at `:613-623`. A made save returns before any delivery
(`:595-597`).

**`SaveCause` is live and carries a spell ref and an instigator.**
`SaveCause{Trigger, EffectRef, InstigatorID, InstigatorType}` (`events/events.go:419-424`) is
filled by the cast slice on every cantrip save (`resolution/contest.go:453-458`,
`action.go:81-85`). It is the shape that says *this check happened because of that spell, at
that creature's hands* — which is exactly what a concentration check has to say.

**Machines cannot start machines out of band, and cannot nest.** The only in-band way one
machine runs another is a `Request` step (`resolution/step.go:74`); inside a running `Resolve`
the reconstructed encounter is handed `RefusingStriker`/`RefusingMover`/`RefusingAnnouncer`
(`resolve.go:352-374`). Nothing in the repo starts a `Resolve` from inside a bus handler.
**The session is the only thing that starts an interaction**, and it already does so for
something that is not a player's declaration: `session/announcer.go:38-79` turns a turn
boundary the composition noticed into its own `Resolve` with `Cost: nil`, because *"the
composition… holds no bus (ADR-0038)… so a boundary becomes a resolution"*.

**Damage is applied inside a machine and announced to nobody.**
`strike.go:692-698` applies it and records `applied.TotalDamage`; `contest.go:291-351` is the
second site since slice two. Neither publishes, and `strike.go:701-728` says why: the monster
keeper treats `DamageReceivedTopic` as an *instruction* (`monster/load.go:210`) and a publish
would double-apply. `character.ApplyDamage` (`character/character.go:611-658`) takes no bus at
all. **A condition cannot learn that its holder was hit.**

**A strike's applied number is honest; a cast's is not.** The strike folds the damage chain
(`strike.go:532` → `:879-889`) and runs `combat.FinalDamage` (`:674`) before applying, so
resistance and immunity are in the number. The cast damage path **folds no chain**
(`contest.go:280-290`). R9.

**The clock is finished for two of the four triggers, and counts turn ends for the third.**
Combat end fans one `Boundary{Kind: CombatEnded, Subject: id}` **per member**
(`encounter/clocks.go:723-737`), precisely so a subject-less ending expires nothing
(`turndriver.go:355-364`), published as `CombatEndTopic` at `resolution/boundary.go:125`. Turn
end is `TurnEndTopic` (`events/events.go:1058`), and a duration today is an int of turn ends
held by the condition itself (`true_strike.go:24-31`, `:251-262`). There is **no round
boundary and no minute clock**, deliberately (`turndriver.go:333-348`, Kirk 2026-08-27).

**Conditions are the only per-member persistent state, and the sheet is their only home.**
`character.Data.Conditions` (`character/data.go:107`), built through
`conditions/factory.go:39-122` and reloaded through `conditions/loader.go:17-193`, both of
which refuse an unknown id (`:113-115`, `:211`). `EncounterData` (`encounter/data.go:26-118`)
and `SessionData` hold no condition state. There is no second store to invent.

**The record already has the two beats a check produces.** `EVENT_KIND_SAVED` shipped in
slice two with `{saver, ability, roll, total, dc, succeeded, source}`
(`events.proto:797`), and `ResultConditionRemoved` is already in encounter's closed result set
(`encounter/activation.go:24-50`) with the roll-fact refusal that forbids it an amount
(`:465-481`).

**Two leftovers, named once and not leaned on.** `saves.DCHalfDamageFloorTen()`
(`gate.go:113-146`) and `dnd5eEvents.SaveTriggerConcentration` (`events/events.go:411-412`)
both survived the refactor with no producer, no consumer and no test but their own. They are
not evidence this slice is cheap. **Use each only if it fits the ruling exactly; delete it in
the retrofit if it does not.**

### Where the evidence contradicts the brief

Three, named rather than smoothed over. All three move the design.

**1. The check cannot be fed from a damage-applied path, because there is no such path.** The
brief says the CON save is *"the contest/save machine fed from the damage-applied path"*.
Nothing publishes damage (see above), and retiring the instruction-shaped
`DamageReceivedTopic` is toolkit#977, not this slice. The cause has to be carried, not
overheard. R2.

**2. The check cannot run inside the strike either**, because a nested `Resolve` is refused
and the alternative — a `Request` step inserted into every machine that can deal damage — is
the concentration rule copied once per damage source. R2.

**3. The door cannot express "you are already concentrating".** The brief calls the recast rule
an activation-door rule. `payAtTheDoor` (`resolution/cost.go:284-309`) knows only a
`combat.SpendProfile`, whose `Requires` is *"keyed capacity that must be PRESENT and is never
spent"* and is explicitly **not a predicate language** (`combat/spend_profile.go:47-56`,
`:82-90`). And the drop is a mutation, while `Start` is contractually pure preflight —
*"Start is pure preflight and runs before payment"* (`resolve.go:416-419`). The drop is the
first yielded step **after** the charge. R3. That ordering is also the correct rule: you drop
the old spell because the new one was actually cast, not because you tried.

## Ownership — every noun, and who holds it

| Noun | Owner | Why it cannot live anywhere else |
|---|---|---|
| The fact that a caster is concentrating | toolkit `conditions` — one condition on the caster's sheet | conditions are the only per-member persistent, bus-applied state (`character/data.go:107`); `EncounterData` and `SessionData` hold none |
| Which spell is being concentrated on | that condition's own blob | the law slice two applied: the source ref is a field the condition was constructed with (`true_strike.go:77`, `vicious_mockery.go:60`) |
| Which effects the spell left behind | that condition's blob, as `{member, ref}` addresses | a pointer cannot survive the blob round trip, and the pair is already the address the removal fact takes |
| Whether a spell requires concentration | toolkit `combat/actions` — `CastProfile` | the profile is where content declares what a cast *is*, and it *"names no spell"* (`combat/actions/cast.go:49-58`) |
| How long the spell lasts | the concentrating condition, as a turn-end count | one clock per spell. Today True Strike counts its own turn ends (`true_strike.go:251-262`); after this slice the owner counts and the child does not, because two clocks on one spell is two answers to one question |
| The difficulty of the check | this slice's ruling, expressed as a `DCSource` fed the reported damage | the contest already asks a `DCSource` for a DC given `DCInput{DamageTaken}` (`contest.go:536`); what number that source returns is a rules decision, and R4 makes it |
| That a check is owed at all | toolkit `session` — the seam after the interaction | resolution cannot publish damage and cannot nest; the session is the only thing that starts an interaction (`announcer.go:38-79`) |
| How much damage was taken | toolkit `resolution` — reported on `Output` | the machine that applied the damage is the only thing that knows the applied number (`strike.go:698`, `contest.go:335`) |
| Removing a child from another member's sheet | toolkit `conditions` — the removal **fact**, honoured by that member's keeper | `ConditionRemovedEvent` is *"A FACT, NOT A COMMAND"* (`events/events.go:678-687`); there is no cross-actor removal verb and this slice does not add one |
| The record of a break, and why | toolkit `encounter` — one beat | *the composition is the only author of its record*; the session appends nothing directly |
| The badge the caster reads | web, off the condition view that already exists | `characterPresentation.ts:38-66` is a ref-keyed table; one row costs no proto |
| Whether other players can see it | protos — one bool on the session-lane roster row | `Participant.active` (`types.proto:883`) is the existing per-member boolean; nothing else on that lane carries member state |

## Rulings

**R1 — Concentration is one condition on the caster, `dnd5e:conditions:concentrating`, and it
holds its children as `{member, ref}` addresses.**
`ConcentratingConditionData{Ref, MemberID, SpellRef, SpellName, TurnEndsLeft, Children []ChildRef}`
where `ChildRef{MemberID, ConditionRef string}`. Built by the factory
(`conditions/factory.go:70-115`, one arm) and loaded by the registry
(`conditions/loader.go:17-193`, one entry), exactly as every other condition. It subscribes to
three things: `TurnEndTopic` (its own duration), `CombatEndTopic` (`events/events.go:1090`),
and `ConditionRemovedTopic` (`:1073`) so it notices its own children ending. The shared
long-rest net (`conditions/rest.go:15-38`) removes it like any other.
*Scope:* one owner, N children, one spell. It is not a generic effect graph — a child that
itself owns children is a shape nobody has asked for.

**R2 — The cause of the check is carried, not overheard. Resolution reports the damage it
applied; the session starts the concentration check as its own interaction.**
This is the ruling the rest of the damage trigger follows from, and it is a statement about
causation before it is a statement about plumbing: **nothing subscribes to damage and rolls in
the dark.** The machine that dealt the damage says so, and the thing that owns starting
interactions decides a check is owed.

- `resolution.Output` gains `DamageTaken []DamageTaken{MemberID string, Amount int, DroppedToZero bool}`,
  filled by the two places that apply damage today (`strike.go:692-698`, `contest.go:291-351`)
  and by every future one.
- The session, after any `Resolve` that reports damage, asks each damaged member's sheet
  whether it is concentrating and, if so, runs **one more `Resolve`** with `Cost: nil` —
  precisely the move `session/announcer.go:55-79` already makes for a boundary the composition
  noticed and could not publish.

*The special case above* is a `Request(save)` step inserted into the strike machine, then again
into the contest, then into whatever applies damage next — the rule copied once per damage
source, each copy able to drift. *The primitive below* is **resolution reporting what damage it
applied**, read once by one seam. Recommend the primitive: every future damage source is
covered the day it fills the field.
*Scope:* damage as a break trigger. `Output.DamageTaken` is a report with one reader, not a
subscription surface and not a general post-interaction hook.

**R2a — The check is automatic. It is not a `Pose` and not a window.**
RAW 2014 gives the caster no choice. The machinery for a non-interrupt roll is a plain
sub-machine — `resolution/save.go:80 NewSave`, already run as a `Request` by the contest
(`contest.go:646-658`) — and a sub-machine structurally cannot pose (`step.go:167-176`). Stated
honestly: posing **is** available inside a resolution machine now
(`resolution/strike_pose.go:229`, shipped with the post-roll window), so declining it here is a
rules decision, not a missing capability.

**R3 — The recast drop is the first yielded step of the new cast, after the charge.**
`castMachine.Start` runs the inner machine's Start (`resolution/action.go:173-191`) so a bad
cast is refused while `Resolve` is still in pure preflight. The drop cannot go there — Start
mutates nothing (`resolve.go:416-419`) — and it cannot go in the door (contradiction 3). So
when the definition's `CastProfile.Concentration != nil` and the caster already carries the
concentrating condition, `castMachine` yields one leading `Gather` named *"drop concentration on
&lt;spell&gt;"* that publishes the removals, then proceeds. The ordering is the rule:
**preflight → charge → drop the old spell → resolve the new one** (`resolve.go:416-427`). A cast
refused at the door drops nothing.
*Scope:* a concentration cast displacing a concentration cast. It says nothing about
Counterspell or any other declaration-time interrupt.

**R4 — The check is the contest, given a Constitution gate, a reported damage number, and one
new consequence: a condition removed. The DC is this slice's ruling.**

Designed from the machine outward. A contest is Request(save) → outcome policy → deliver →
Done. To be a concentration check it needs four things, and three of them are inputs it already
takes:

| What the check needs | What the contest already does | Delta |
|---|---|---|
| roll a CON save for the caster | `SaverID` + gate ability, rolled by `NewSave` as a `Request` (`contest.go:646-658`) | none |
| a DC that is a function of the damage | asks a `DCSource` for a DC given `DCInput{DamageTaken}` (`contest.go:536`), the number filled by the caller (`:25-56`) | **which** number it returns — R4's formula, below |
| success means nothing happens | `Negated`-only policy, returning before any delivery (`:159-161`, `:595-597`) | none. The refusals at `:159-164` stay exactly as they are |
| failure strips the owner and its children | one delivery `Gather` publishing a condition applied (`:242-257`), a sibling applying damage (`:291-351`), chained at `:613-623` | **a third sibling that publishes removals** |

So the widening is additive and small:

| Type | Today | Delta |
|---|---|---|
| `ImposedEffectKind` (`contest.go:63-71`) | `ImposedCondition`, `ImposedDamage` | one row, `ImposedConditionRemoved` |
| `ContestInput` (`contest.go:25-56`) | `Gate, SaverID, Application, Damage, SourceName, Cause, DamageTaken, Roller` | one field, `Removal *ConditionRemoval{MemberID string, Refs []string}` |
| the delivery chain (`contest.go:613-623`) | damage, then condition | damage, then condition, then removals, publishing `ConditionRemovedTopic` per address |
| `Application` | required | may be nil when a `Removal` or `Damage` is declared — the same widening slice two made for damage |

**The DC, ruled here rather than inherited: `max(10, floor(damage taken / 2))`, from the
applied amount.** That is RAW 2014, and it is a rule this slice is choosing, not a formula it
found lying around. It is expressed as a `DCSource` because that is the seam the contest reads
(`gate.go:87-97`, whose extension discipline at `:68-86` asks a new case to cite a RAW rule —
this one does). An unwired `DCHalfDamageFloorTen()` (`gate.go:113-146`) survived the refactor
computing this shape; **if its arithmetic matches the ruling exactly it is adopted as the
implementation, and if it does not it is deleted in the retrofit rather than bent.** The ruling
is the authority; the leftover is at most a saved afternoon.

The cause is `SaveCause{Trigger, EffectRef: <the spell>, InstigatorID: <the damager>}`, filled
the way the cast slice fills it (`action.go:81-85`). The trigger value names concentration; the
unwired `SaveTriggerConcentration` constant (`events.go:411-412`) is adopted **only** because
its string is already the word this cause needs, and it is deleted and re-declared if the
`SaveTrigger` vocabulary moves.
*Scope:* removal as a consequence of a check. It is not a Dispel Magic verb and it is not a
cross-actor removal API — the delivery publishes facts, and each keeper honours them.

**R5 — The keeper must call `Remove` on a condition it prunes. Hop 2 of the strip is broken
today, and concentration is the first thing that walks it.**
`onConditionRemoved` (`character/character.go:1195-1216`, mirror `monster/monster.go:553-573`)
matches on ref string and drops the behavior from the slice — it never calls `Remove(ctx, bus)`,
so the pruned condition's subscriptions stay live on the bus. Invisible today because only
self-ending conditions publish, and they call their own `Remove` right after
(`true_strike.go:275-288`). The moment an owner ends someone else's condition, the child keeps
listening from a list it is no longer in.
Fix: **if `cond.IsApplied()`, call `cond.Remove(ctx, bus)` before dropping it.** `IsApplied()` is
already on the interface (`events/events.go:138-159`), so the self-ending path is unchanged.
*The special case above* is the concentrating condition holding live pointers to its children and
calling `Remove` itself, which cannot survive the JSON round trip. *The primitive below* is the
removal fact meaning what it says on every sheet. Recommend the primitive.

**R6 — Concentration ends when its last child ends, and the beat says so.**
The owner subscribes to `ConditionRemovedTopic` and drops a child from its list when that
child's address is removed by anything — consumed on use, expired, dispelled. When the list
empties, the spell is over and the owner ends itself with reason `spell_ended`.
Without this, a bard whose True Strike was consumed by an attack still reads as concentrating
and still drops "nothing" on the next concentration cast — a visible lie, and exactly the shape
of affordance-with-nothing-behind-it that slice one's walk kept finding.
*Scope:* the owner watches its own children and nothing else.

**R7 — Four reasons, and `caster_down` is the fourth. Incapacitated as a condition is
deferred.**
The reasons are `damage` (a failed check), `recast`, `duration`, `combat_end`, `spell_ended`
(R6) and `caster_down`. `Incapacitated` is a ref with **no behavior at all** — no file, no
factory arm, no loader entry (`refs/conditions.go:64` and nothing else) — and building the
general condition is a different slice. But the caster dropping to 0 HP is the one
incapacitating case this seam already sees for free: `ApplyDamageOutput.DroppedToZero`
(`combat/combatant.go:36-45`) rides the same report R2 adds.
**Recommend taking it.** The cost is one boolean read at a seam that is already there; the cost
of leaving it out is a downed bard who concentrates forever, which the walk will find on its
first knockdown. **Open ruling — this is the one place the slice reaches past its smallest cut,
and it is Kirk's call.**
*Scope:* dropping to 0 HP. Paralysis, stunning, sleep and the general Incapacitated condition
stay on the shelf with the rest of the thirteen inert refs.

**R8 — True Strike's retrofit moves the clock onto the owner and deletes the divergence.**
`CastProfile` gains `Concentration *CastConcentration{TurnEnds int}` — a pointer, because a
`bool` plus a duration field that means nothing when the bool is false is a zero value that
lies. True Strike declares `{TurnEnds: 2}`, the count it carries today
(`TrueStrikeTurnEnds = 2`, `true_strike.go:24-31`), and **`TrueStrikeConditionData.TurnEndsLeft`
and its turn-end subscription are deleted** (`:40`, `:131`, `:251-262`). The child keeps
consumed-on-use (`:245`), which is a trigger and not a clock. One spell, one duration, one
answer. The doc comment at `true_strike.go:58-65` — *"THERE IS NO CONCENTRATION… the day a
levelled spell needs it is the day it has to exist"* — is deleted, not amended.
Persisted True Strike blobs from before this change lose a field. Pre-release, no consumer, no
migration.
*Scope:* True Strike. Vicious Mockery is not a concentration spell and does not change.

**R9 — The DC uses the applied amount, and the cast path's missing chain fold is recorded as a
gap rather than fixed here.**
`DamageTaken` is the number the sheet actually took — `applied.TotalDamage` (`strike.go:698`)
for a strike, `contest.go:335`'s applied total for cast damage. From a strike that number is
already post-resistance and post-immunity (`FinalDamage` at `:674`, after the fold at
`:879-889`). From a cast **it is not**: the cast damage path folds no chain
(`contest.go:280-290`), so a resistant caster hit by a cantrip is asked for a higher DC than RAW
allows.
That is a slice-two defect with a wider blast radius than concentration, and fixing it here
would make the largest diff in the slice be in someone else's shape. **Recorded, with its own
toolkit issue, named on the shelf.** The concentration code reads the reported number and is
correct the day the fold lands.

**R10 — One new beat kind, and the check rides the beat that already exists.**
`EVENT_KIND_CONCENTRATION_ENDED = 29` with `ConcentrationEnded{caster, SpellRef spell, reason}`
at body tag 35 — both confirmed free (`events.proto:259`, `:369`) and both additive under the
repo's `FILE` breaking policy (`buf.yaml`, gate at `.github/workflows/ci.yml:36-43`), so **no
`breaking-change-approved` label is needed.** The check itself needs nothing new:
`EVENT_KIND_SAVED` shipped in slice two with exactly the right fields (`events.proto:797`), and
the stripped children are `ResultConditionRemoved`, already in encounter's closed result set.
**Why a dedicated kind rather than inferring the break from the removals:** three of the six
reasons produce no check at all, and hop 2 lands removals on *other members'* sheets, where
they read as random drops. The beat is the only thing that says the spell ended and why.
*Scope:* the break. There is no "concentration started" beat — the cast beat already is one.

**R11 — Other players see a concentrating member through one bool on the roster lane; the
caster sees a badge for free.**
The caster's own view costs **zero proto**: `character.StatusView.Conditions`
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
caster's badge alone.**

**R12 — Monsters do not concentrate, and nothing pretends they do.**
No monster casts today; `spells.CastDefinition` is reached only from `session/casts.go:83` over
a character's known cantrips. `Monster` gets the same keeper fix as `Character` (R5), because
that fix is about the removal fact and not about casting. When a monster casts, it holds the
same condition on the same sheet field (`monster/data.go:42`) with no design change.

## Shape, per module (bottom-up)

**1. `refs`** — `Conditions.Concentrating()` beside the fifteen core rows
(`refs/conditions.go:59-75`); id `concentrating`.

**2. `combat/actions`** — `CastConcentration{TurnEnds int}` and
`CastProfile.Concentration *CastConcentration` (`cast.go:65-86`), with arms in `Validate`
(`:118-135`) and `Clone`. Nothing in the type names a spell.

**3. `conditions`** — `concentrating.go`. Blob per R1; `Apply` subscribes `TurnEndTopic`,
`CombatEndTopic` and `ConditionRemovedTopic`; `end(ctx, reason)` publishes one
`ConditionRemovedEvent` per child address and then its own, following
`vicious_mockery.go:232-245` verbatim. One factory arm, one loader entry, one display-catalog
row (`display.go:43-80`), and the long-rest net. `true_strike.go` loses `TurnEndsLeft` and its
turn-end subscription (R8).

**4. `character` / `monster`** — hop 2: `onConditionRemoved` calls `Remove` when `IsApplied()`
(`character/character.go:1195-1216`, `monster/monster.go:553-573`) — R5, and the one change in
this slice that fixes a defect rather than adding a shape.
`Character.Concentration() (ConcentrationView, bool)` reads its own conditions
(`character.go:590-592`) and answers spell ref, name and child count. **The caster answers
whether it is concentrating**; no caller type-asserts over `GetConditions()`.

**5. `resolution`** — `Output.DamageTaken` (R2), filled at `strike.go:692-698` and
`contest.go:291-351`. `ImposedConditionRemoved` and `ContestInput.Removal` (R4), the removal
delivery chained after the condition delivery at `contest.go:613-623`, and the CON `DCSource`
the ruling names. `castMachine` gains its leading drop `Gather` (R3), and `newCast`
(`action.go:67-105`) applies the concentrating condition to the caster when the profile
declares concentration — one more prepared condition through the path
`bindCounterpart`/`prepareCondition` already walks (`action.go:376-379`, `contest.go:179-193`).
**No new machine and no new file beyond the damage report.**

**6. `encounter`** — `BeatConcentrationEnded = "concentration_ended"` beside `BeatCast` and
`BeatSaved` (`cast.go:21`, `:30`), its payload struct beside `castPayload` (`:103`), and
`RecordConcentrationEnded` beside `RecordCast` (`:167`), appending through the same
`appendBeat`/`audienceFor` transaction loop (`:174-186`). Result kinds unchanged:
`ResultConditionRemoved` already exists (`activation.go:24-50`) and the roll-fact refusal
(`:465-481`) already forbids it an amount, which is correct.

**7. `session`** — `session/concentration.go`: one helper called after every `Resolve` that can
damage (`attack.go:307`, `cast.go:295`, `mover.go:154`, `react_post_roll.go:84`) which, for each
`Output.DamageTaken` entry whose member is concentrating, runs a second `Resolve` with
`Cost: nil`, then `RecordConcentrationEnded`. `EventConcentrationEnded` and
`ConcentrationEndedBody` in `session/types.go` beside `EventCast`/`CastBody` (`:763`, `:1304`),
and the beat-string arm in the kind switch at `session/events.go:750-830`. `Participant` gains
`Concentrating bool` (R11), filled from the sheet in `session/convert.go`.
**Ordering at the seam:** the damaging interaction is saved and recorded first
(`cast.go:334-341` is explicit that save-then-record is deliberate), then the concentration
interaction runs, saves and records. Two interactions, in that order, so the log reads
*damage → check → spell ended*.

**8. protos** — additive only, no label. `EVENT_KIND_CONCENTRATION_ENDED = 29`
(`events.proto:259`), `ConcentrationEnded concentration_ended = 35` (`:369`) with
`{string caster = 1; SpellRef spell = 2; string reason = 3;}`, and `bool concentrating = 8` on
`Participant` (`types.proto:900`). `reason` is an open string, following `Saved.ability` rather
than `FightEnded.cause` — the six reasons are a rulebook vocabulary, not a wire contract, and
the client renders the sentence the server authored.

**9. rpg-api** — one arm in `eventKindToProto` (`convert.go:644-649`) and one in `setEventBody`
(`:921-942`), **in the same change**, because an unmapped kind demotes silently to
`EVENT_KIND_UNKNOWN` with a nil body and the Go type switch has no exhaustiveness check
(`convert.go:640-643`). `participantToProto` (`:1732`) fills `Concentrating`. The caster's
condition view needs **no api change** — `mapStatus` (`character_data.go:136-168`) copies
whatever the toolkit status view carries.

**10. rpg-dnd5e-web** — a `case 'concentrationEnded'` arm in `buildOtherStory`
(`story.ts:242-247`), following `case 'cast'` (`:408-421`) verbatim: kind re-check, `memberName`,
`spellName`, frozen exchange, headline with no trailing period. **This will not fail the build
if forgotten** — the switch's declared return type includes `undefined` and there is no
`assertNever`, so the arm is added by hand. One row in `characterPresentation.ts:38-66` keyed
`dnd5e:conditions:concentrating`, and a badge on `InitiativeEntry`
(`CombatExperience.tsx:77-100`) off `participant.concentrating`, following the
`participant.active` class-plus-tooltip pattern at `:87-88`.

## The seven principles, run against this

**Ownership before mechanism** — the table above; the contested nouns were *who decides a check
is owed* (R2: the session, carrying a cause resolution reported, because nothing may roll in the
dark) and *who counts the duration* (R8: the owner, because two clocks on one spell is two
answers).
**Rulings carry scope** — every ruling names what it does not settle; R3 refuses to become
Counterspell's door rule and R4 refuses to become Dispel Magic.
**Load-bearing or deferred** — Incapacitated, minute durations, fan-out, Bless's area targeting,
hearing, and the cast path's chain fold are all deferred with a named trigger. R7 is the one
deliberate reach past the smallest cut and it is flagged as Kirk's call. The two unwired
leftovers are load-bearing only if they fit the ruling exactly; otherwise they go.
**Zero values tell the truth** — `CastProfile.Concentration` is a pointer, not a bool beside a
duration that means nothing when false (R8); `ImposedEffect` gains a kind rather than a removal
expressed as an application with an empty payload (R4); a member who is not concentrating
carries no condition, so `Concentrating` is false because the sheet says so.
**Call sites read as statements** — `Character.Concentration()` rather than a type assertion over
`GetConditions()`; `Output.DamageTaken` is a report the seam reads, not a hook.
**Record tells the truth same-day** — six reasons on one beat, and the check that produced one of
them is the same `SAVED` beat every other save uses.
**Fail closed loudly** — an unknown condition id is refused at the factory
(`factory.go:113-115`) and the loader (`loader.go:211`); an unknown ref in the display catalog is
a hard error (`display.go:38-41`); a boundary kind this build does not know is refused at
`boundary.go:83-85`; and R5 turns a silent subscription leak into a removal that actually
removes.

## What the tests must prove

**Toolkit unit.** The owner round-trips through JSON with its child addresses intact. Ending it
publishes one removal per address **and** its own, in that order. A child removed by anything
else leaves the owner's list, and when the list empties the owner ends with reason
`spell_ended` (R6). The owner ends on its turn-end count reaching zero, on combat end, and on
long rest. `Character.Concentration()` answers false with no condition and the spell ref with
one. **The hop-2 regression (R5): a condition pruned by a removal it did not publish is
unsubscribed** — assert the pruned condition's chain handler no longer fires. That test fails on
today's code.

**Resolution scenes.** A failed check at DC `max(10, damage/2)` removes the owner and every
child; a made check removes nothing and still records roll, total and DC. A contest with no
`Removal` declared is byte-identical to today — the regression that matters. A concentration
cast while concentrating drops the old spell **after** the charge, and a cast refused at the
door drops nothing. `Output.DamageTaken` carries the applied number and `DroppedToZero`, from
both the strike path and the cast damage path.

**Encounter.** A break appends exactly one `concentration_ended` beat plus one
`condition-removed` result per child; a `condition-removed` result still refuses roll facts
(`activation.go:465-481`); an ungated break — duration, combat end — appends no `saved` beat.

**Session scenes.** A bard concentrating on True Strike is struck for 9 and rolls a CON save at
DC 10; for 30, at DC 15. A failed check ends the spell and **the target's condition is gone from
the target's sheet** — the strip, end to end. A second concentration cast drops the first with
reason `recast`. A bard downed to 0 loses concentration (R7). A non-concentrating member taking
damage runs **no second interaction at all** — the seam must be free when nobody is
concentrating.

**Web.** `npm run ci-check` with the log grepped for `✗`/`❌` (it exits 0 on failure). The
concentration-ended line renders through `story.ts`; the caster's badge renders; the roster badge
renders off `participant.concentrating`.

## Done-when — Kirk's walk

1. A level-1 bard with True Strike enters a fight and casts it on the skeleton. The dock and the
   log read as they do today, **and** the bard's own status now shows a concentrating badge
   naming True Strike. Other players see the roster marker.
2. The skeleton hits the bard. The log shows a **Constitution check** with its roll, total and a
   DC that is 10 for a small hit and half the damage for a big one. On a success the badge stays
   and the advantage is still there.
3. Force a failure. The log reads *the bard loses concentration on True Strike*, the badge
   clears, and the bard's **next attack against the skeleton has no advantage** — the child went
   with the parent. That is the strip, on screen.
4. Cast True Strike, then cast it again on a different target without being hit. The first drops
   with reason `recast`, exactly one badge stands, and the log says which spell ended.
5. Let True Strike be consumed by an attack instead. Concentration ends by itself
   (`spell_ended`), the badge clears, and no check was rolled.
6. Let it run out. The badge clears at the end of the caster's next turn, and clears again when
   the fight ends, with no stale badge on the post-fight screen.
7. Knock the bard to 0. Concentration ends (R7, if Kirk takes it).

## Shelf

- **Bless, and "choose up to N targets in an area"** — fan-out (§3.1, §6.2), a separate shape.
  Concentration is ready for it: Bless is one owner with three addresses instead of one.
- **Incapacitated, and the other twelve inert core condition refs** (`refs/conditions.go:59-75`).
  R7 buys only the drop-to-zero case.
- **"Up to 1 minute" and every non-turn duration.** No minute clock and no round boundary
  (`turndriver.go:333-348`). Every concentration spell this slice can express counts turn ends.
- **The cast damage path folds no chain** (`contest.go:280-290`) — resistance does not apply to
  cantrip damage and the DC inherits the error. Its own toolkit issue, filed with this slice.
- **`DamageReceivedTopic` as a fact rather than an instruction** (toolkit#977). Even then R2's
  framing stands: a condition hearing about damage and rolling on its own is the thing R2 rules
  against. Worth re-reading that issue before starting, not to invert the design.
- **Dispel Magic and Counterspell.** R4 gives removal a delivery and R3 gives the door a
  displacement rule; neither is a verb.
- **Concentration on the monster side.** R12: nothing changes until a monster casts.
- **A modifiable concentration check** — War Caster, advantage on the roll — becomes a chain
  subscriber on the check R4 defines, and nothing else.

## What would change the design

- **If Kirk refuses R7**, `caster_down` comes out and a downed caster keeps its badge until the
  fight ends. One reason less, no structural change, and a known lie until Incapacitated exists.
- **If Kirk refuses R11's roster bool**, the caster's badge still ships for free and only they
  see it. The break beat then arrives with no setup for everyone else.
- **If a spell needs to end a *world fact* rather than a condition** — a wall of fire, a region
  effect — then `Removal` is carrying a vocabulary rather than an address, and the owner's
  children become a list of typed handles. That is the trigger to revisit R1, and it arrives with
  the first persistent-area spell.
- **If a second thing ever needs `Output.DamageTaken`** — a retaliation aura, a damage-triggered
  feature — then R2's report is a real seam and deserves a name of its own rather than a field the
  session reads. One customer is a field; two is a shape.
- **If the DC ruling changes** — a variant table, a flat DC — only the `DCSource` moves. The
  contest, the strip and the beat are indifferent to what number comes back, which is the point of
  ruling it in one place.

---

## Issue seeds

One block per module that changes. Written to be cut verbatim. **Not filed.**

### protos — the concentration-ended beat and the roster flag

**Assumptions.**
1. `EVENT_KIND_CONCENTRATION_ENDED = 29` is free (`events.proto:259` ends at 28) and body tag 35
   is free (`:369` ends at 34).
2. `Participant` field 8 is free (`types.proto:879-900` ends at 7).
3. Adding an enum value, a oneof arm and a field is **non-breaking** under `FILE` (`buf.yaml`),
   so no `breaking-change-approved` label is needed.
4. `reason` is an open string; the client never branches on it for a rule.

**Definition of done.** `buf lint`, `buf format`, `buf breaking` and `buf generate` clean with no
label; generated Go and TS compile. Must survive: nothing downstream can start until the
generated packages carry the kind, the body and the field.

**Depends on.** Nothing. Merges first.

### rpg-toolkit root (`refs`, `combat/actions`, `conditions`, `character`, `monster`) — the owner, the profile arm, and hop 2

**Assumptions.**
1. `CastProfile` can carry a nullable `Concentration` arm without disturbing `Validate`'s
   existing refusals (`cast.go:118-135`).
2. A new condition needs exactly four registrations: factory arm, loader entry, display-catalog
   row, long-rest net.
3. `ConditionBehavior.IsApplied()` makes a second `Remove` a safe no-op, so the keeper fix does
   not disturb self-ending conditions.
4. Deleting `TrueStrikeConditionData.TurnEndsLeft` needs no migration (pre-release, no consumer).
5. `Character.Concentration()` reads only the sheet's own conditions — no new sheet field.

**Definition of done.** The unit tests listed above, including **the hop-2 regression that fails
on today's code**: a condition pruned by a removal fact it did not publish must be unsubscribed.
True Strike still grants advantage against its named target and is still consumed on use. Every
condition still round-trips. Must survive: a bard finalizes and enters a fight on the walk env
with True Strike on the sheet.

**Depends on.** Nothing (toolkit-internal). Ships before resolution.

### rpg-toolkit `resolution` — the damage report, removal as a consequence, the recast drop

**Assumptions.**
1. `Output.DamageTaken` can be added additively; every existing caller ignores it.
2. `ContestInput.Application` may be nil when a `Removal` is declared, the same widening slice two
   made for `Damage`.
3. `castMachine` can yield a leading `Gather` before delegating to the inner machine
   (`action.go:173-191`), and that step runs after `payAtTheDoor` (`resolve.go:416-427`).
4. The CON `DCSource` returns `max(10, floor(damage/2))` per R4; the unwired
   `DCHalfDamageFloorTen()` is adopted only if its arithmetic matches exactly, and deleted
   otherwise.
5. The `Negated`-only and no-recurrence refusals (`contest.go:159-164`) stay exactly as they are.

**Definition of done.** The resolution scenes above, with **a contest that declares no removal
byte-identical to today** as the regression that matters. A cast refused at the door drops
nothing. Must survive: walk steps 2, 3 and 4.

**Depends on.** toolkit root.

### rpg-toolkit `encounter` — `RecordConcentrationEnded`

**Assumptions.**
1. The beat is one new const plus one payload struct beside `BeatCast`/`BeatSaved` (`cast.go:21`,
   `:30`, `:103`), appended through the same transaction loop.
2. The result kinds do not change — `ResultConditionRemoved` already exists
   (`activation.go:24-50`).
3. The roll-fact refusal (`activation.go:465-481`) needs no new arm, because a removal carries no
   amount.

**Definition of done.** A break appends one `concentration_ended` beat plus one
`condition-removed` result per child, in that order; an ungated break appends no `saved` beat; a
`condition-removed` result still refuses roll facts; the beat is refused on a closed encounter.
Must survive: the walk's log reads damage → check → spell ended, in order.

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

**Definition of done.** The session scenes above, including **no second interaction when nobody is
concentrating**, and the strip proven end to end: the child is gone from the *target's* sheet. The
typed body and the beat-string arm both land, and a break survives a session reload. Must survive:
every step of the walk.

**Depends on.** toolkit root, resolution, encounter.

### rpg-api — two convert arms and one participant field

**Assumptions.**
1. Both `eventKindToProto` (`convert.go:644-649`) and `setEventBody` (`:921-942`) arms land in the
   same change; omitting either compiles and demotes silently (`:640-643`).
2. `spellRefToProto` (`:1090`) already serves the body's spell field.
3. `participantToProto` (`:1732`) is the only place the roster flag is filled.
4. The caster's condition view needs **no api change** — `mapStatus`
   (`character_data.go:136-168`) copies whatever the toolkit status view carries.

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
break, the roster marker shows for other members, and the log line reads *loses concentration on
True Strike* with the reason legible. Must survive: every step of the walk.

**Depends on.** protos, rpg-api.

### rpg-toolkit `resolution` (separate, filed with this slice) — cast damage folds no chain

**Assumptions.** `applyPreparedDamage` (`contest.go:291-351`) calls `combat.FinalDamage` (`:312`)
over components that were never folded through `DamageChain` (`:280-290`), unlike the strike path
(`strike.go:532`, `:879-889`, `:674`).

**Definition of done.** A resistant target takes half damage from Vicious Mockery, and the
concentration DC computed from that damage uses the halved number. Not part of the concentration
slice; concentration is correct the day this lands.

**Depends on.** Nothing. Independent of this slice.
