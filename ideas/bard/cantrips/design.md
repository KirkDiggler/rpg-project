# The cast door — a level-1 bard casts two cantrips

**Date:** 2026-09-08
**Status:** Design. One slice, cut. Rung 2 of the bard pilot, narrowed by Kirk to cantrips only.
**Umbrella:** `ideas/spells/` — levels 1–3, and the choices at 3 made real.
**Brainstorm:** rpg-project#391 (`ideas/spells/brainstorm.md`) §5.2, §5.3, §6.2, §7.
**Slice one:** rpg-project#397 (`ideas/bard/level-1/design.md`) and rpg-project#398
(`ideas/session-combat/interrupt/post-roll/design.md`), both shipped 2026-09-08.

Kirk's ruling that cut this slice: **the cast door with two cantrips, Vicious Mockery and
True Strike. No slots, no level-1 spells.** Charm Person and the world graph are slice
three; slots are rung 3's own shape.

---

## What this slice is

A level-1 bard chooses 2 of its 11 cantrips at creation, and in a fight the dock offers a
**Cast** row per cantrip that this build can actually cast. Two can be:

- **True Strike** — a self condition that gives advantage on the caster's next attack
  against the named target. No roll, no save.
- **Vicious Mockery** — the target makes a Wisdom save against the bard's spell save DC;
  on a failure it takes 1d4 psychic and has disadvantage on its next attack roll.

Between them they are the two halves of every cantrip: *deliver something* and *roll to see
whether you deliver it*. Together they are the whole door.

## The facts this stands on

Toolkit `origin/main`, protos `origin/main`, rpg-api `origin/dev`, web `origin/dev`.

- **The sheet already holds the spells, and nothing asks for them or reads them.**
  `character.Data.KnownCantrips` / `KnownSpells` (`character/data.go:98-99`), compiled
  through `refs.Spells.ByID` and refused if unknown (`character/draft.go:1203-1218`,
  refusal `:1212`), read back at `character/character.go:1559`, `:1563`. Outside
  `character/` and `resolution/long_rest.go:133` **nothing consumes them.** Both refs exist:
  `refs/spells.go:48-49` (`"true-strike"`, `"vicious-mockery"`), lookup at `:543`.
- **The requirement keys exist and are unreferenced.** `choices/choice_ids.go:178-179`.
  `getBardRequirements` (`choices/requirements.go:366`) leaves `Cantrips` nil on purpose and
  says why at `:421` — *"They come back with the cast door (rung 2)."* The validator checks
  it only when non-nil (`choices/validation.go:107`); the checker is `validateCantrips` (`:530`).
- **Dispatch by profile arm is one function with one arm.** `actions.Definition` says
  *"Exactly one profile must be populated"* and holds only `Attack`
  (`combat/actions/definition.go:14-20`, refusal `:30`); `NewAction` branches on
  `Definition.Attack != nil` (`resolution/action.go:19-34`). `AttackCategorySpell` is
  declared and unbuilt (`combat/actions/attack.go:27`).
- **The contest machine is condition-only and carries no damage.** `ContestInput` is
  saver-centric (`resolution/contest.go:21-28`); `ImposedEffect` holds a ref and prose
  (`:32`); `ContestOutcome` has no damage (`:38-44`). It refuses any policy but `Negated`
  (`:69-71`) and any recurrence (`:72-74`). `NewContest` is exported (`:136`) with one
  caller, the strike rider (`strike.go:752`). It requests `NewSave` (`save.go:80`) at
  `contest.go:238-246`, DC off the gate at `:171`.
- **There is no damage delivery outside a strike.** The only application is `m.applyDamage`
  inside `strikeMachine` (`strike.go:692`, seam `:127`) over `combat/combatant.go:157`. The
  activation machine's effect set is closed to healing, condition-applied, condition-removed
  and capacity (`resolution/activation.go:63-76`) — **no damage kind** — and it calls
  `actor.ActivateAbility` (`:524`) after enforcing the sheet's own `TargetKind` (`:469`).
- **There is no spell save DC anywhere.** The inputs exist:
  `classes.Data.SpellcastingAbility` (`classes/data.go:26`, bard CHA `:251`) and
  `character.ProficiencyBonus()` (`character/character.go:201`). `DCSource` has three
  implementations, all static or derived-from-damage (`saves/gate.go:100`, `:109`, `:115`),
  sealed at `:96` with an extension discipline at `:73`.
- **Advantage is contributed by subscribers, counted by the machine.** `strike.go:338-339`
  counts, `:344` cancels, `:437-439` projects. `conditions/helped.go:165` appends advantage;
  `conditions/dodging.go:163` appends disadvantage. `conditions/inspired.go` is the newest
  worked condition (subscriptions `:116`-`:141`, opaque JSON `:176`, `:186`); the factory's
  arm list is closed and refuses an unknown id (`conditions/factory.go:70-109`).
- **Afford offers six verbs and none is a cast.** `session/afford.go:41-89`, request list
  `:421-424`, `Declaration` `:155-240`. `buildActivationOffers` iterates
  `sheet.AvailableAbilities()` (`session/activations.go:49`, `:59`) and prices nothing —
  it projects (`:20-32`); candidate machinery at `:304-384`.
- **`Activate` passes `Cost: nil` ON PURPOSE** (`session/activate.go:272`, doc `:106-108`,
  mirror `resolution/activation.go:352-361`), and slice one's note on rpg-project#397
  records that "paid at the door" **did not survive** for an activation, because
  `activateFeature` charges the economy underneath.
- **The wire has no cast and no save.** `Verb` ends at `VERB_REACT = 6`
  (`session/v1alpha1/types.proto:449`); `EventKind` at `ROLL_WINDOW_OPENED = 26`
  (`events.proto:240`); the body oneof's last tag is 32 (`:348`). `ActivationResult` has
  four result arms and 6 is free (`:673-680`); `HealingApplied` carries a `RollCalculation`
  (`:690-710`); `DeathSaveRolled` is eleven death-shaped fields plus a continuation
  (`:389-419`); `AbilityRef` is at `types.proto:1092`.
- **The `Spell` enum is 60 values across four fields.** `enums.proto:411-484`;
  `SpellOptions.available` (`choices.proto:149`), `SpellSelection.spells` (`:230`),
  `SourceRef.spell` (`common.proto:205`), `SpellInfo.spell_id` (`character.proto:842`).
  `Character` has no known-spell field (it ends at 29, `character.proto:231`). The breaking
  policy is `FILE` (`buf.yaml:7-9`), enforced at `.github/workflows/ci.yml:37` and skippable
  only by the `breaking-change-approved` label (`:40-43`).
- **rpg-api discards spell choices in both draft arms** — `handler.go:299-303`
  (*"For now, skip spells"*), `:232-239` for race. No cantrip arm in `loadAllClassChoices`
  (`converters.go:2316`, TODO `:2365`); `converters.go:343-352` projects every stored spell
  as `SPELL_UNSPECIFIED`; `convertSpellToProtoEnum` (`:3112`) has no non-test caller.
  **An unmapped beat kind demotes silently** — `eventKindToProto` defaults to
  `EVENT_KIND_UNKNOWN` (`convert.go:635-636`), `setEventBody`'s default leaves the body nil
  (`:933-937`). That is the ordering constraint between the proto and api PRs.
- **The dock drops a verb it does not know.** `ActionDock.tsx:507-514` is an allowlist, not
  a switch; label falls to `'Move'` (`:71`), icon to `'➜'` (`:101`). `story.ts`'s switch, by
  contrast, **is** exhaustive over `body.case` (`:210`) — a new arm is a compile error.
- **The dice request is hard-wired to a d20.** `presentation.ts:553-555` sets `kind`, preset
  and authoritative roll, with a 1–20 guard at `:538-539`; fact categories are the closed
  set `'attack' | 'death-save' | 'other'` (`:137`) and anything else is marked conflicted
  (`:847-853`, `:938-944`). The d4 already exists in the runtime
  (`conceptDiceRuntimeProvider.ts:31`); no damage die has ever been animated
  (`story.ts:174-177` narrates them).

### Where the evidence contradicts the brief

Three, named rather than smoothed over.

1. **Vicious Mockery does not need a policy change.** The brief expected the contest's
   `Negated`-only refusal (`contest.go:69-71`) to be the delta. RAW 2014 is *"on a failed
   save it takes 1d4 psychic damage and has disadvantage on its next attack roll"* — a
   successful save negates **both** halves. `Negated` is exactly right, and stays. The delta
   is damage, not policy. Half-on-success arrives with a spell that has one.
2. **Contest's delivery is condition-only, and that is a missing effect kind rather than a
   missing machine.** The steps Vicious Mockery needs already run: Request(save) → policy →
   deliver → Done (`contest.go:144-215`). What is absent is a way for the delivery to be
   damage. `ImposedEffect` is `{Ref, Description}` (`contest.go:32-35`) and there is exactly
   one delivery `Gather` (`publishPreparedCondition`, `:111-131`). An earlier draft of this
   design answered that with a `castMachine`; Kirk's ruling deletes it. **An effect kind
   never adds a machine.**
3. **A cast IS paid at the door, and it is the first thing that is.** Slice one's R1 failed
   because `activateFeature` charges the economy underneath an activation. A cast has nothing
   underneath it, so the open ruling on rpg-project#397 is answered in the cast's favour
   rather than re-opened. R10.

## Ownership — every noun, and who holds it

| Noun | Owner | Why it cannot live anywhere else |
|---|---|---|
| What a cantrip *does* (range, target rule, gate, damage, conditions) | toolkit `spells` content, compiled to an `actions.Definition` | ADR-0045: content declares the arm and nothing dispatches on identity (`resolution/ARCHITECTURE.md:89-97`); a weapon already reaches resolution this way (`session/offers.go:286`) |
| Which arm a definition selects | toolkit `resolution` — one new branch in `NewAction` | `action.go:19-34` is the single place content chooses a sequence, and it has one arm today |
| The bard's cantrip choice and its count | toolkit `character/choices` | requirements are the only authority on what a class must choose (`requirements.go:366`) |
| The two chosen refs | toolkit `character` — `KnownCantrips` on the sheet | §5.2: spells known are content refs on the sheet, and the field is already there |
| The spell save DC | toolkit `character` — the caster answers it | it is a property of the caster known before any machine starts, not a formula over an event; see R4 |
| The price (one action) | toolkit `session` compiles it, `resolution` charges it at the door | the door is the only place a price moves (`cost.go:148-173`) |
| Who may be targeted, and how far | toolkit `session` | only the session holds positions, sight and standing (`activations.go:301-362`) |
| The advantage True Strike grants | toolkit `conditions` — a condition on the caster | the attack chain's advantage is contributed by subscribers (`strike.go:338`), and a condition on the bus is the only ongoing effect |
| The disadvantage Vicious Mockery imposes | toolkit `conditions` — a condition on the target | same law, other end |
| The cast beat, the save beat, the result beats | toolkit `encounter` | *"the composition is the only author of its record"*; the session appends nothing directly |
| The verb the player reads | toolkit `session` Afford, projected by protos | `ActionDock.tsx:57-59` — the server authors the label |
| Whether a cantrip is castable at all | the content table: a known ref with no profile mints no row | fail closed; a row that resolved to nothing would be a lie at the door |

## Rulings

**R1 — One `Cast` verb, and it is a new verb rather than `Activate` with a spell ref.**
`ActivateRequest` is `{session, member, declaration_id, target}` with no ability ref by
design (`service.proto:500-519`), so a cast could physically ride it. It should not:
`buildActivationOffers` reads `sheet.AvailableAbilities()` (`activations.go:59`) and the
activation machine calls `actor.ActivateAbility` (`resolution/activation.go:524`) after
enforcing the sheet's own `TargetKind` (`:469`) — a cantrip is on neither list, and the
effect set has no damage (`:63-76`). Teaching abilities to be spells buys one saved enum
value and pays with an ability surface that is half spells. What comes next — a slot at the
door, Counterspell at the door, upcast — is cast-shaped, not activation-shaped, so the
rename is owed either now or later at a higher price. So: `VERB_CAST` on Afford and a `Cast`
RPC mirroring `Activate` field for field, with **no spell ref on the request** — the
declaration id already names it, exactly as `service.proto:500-507` argues for Activate.
*Scope:* the verb and the request shape. Not what a slot costs, and not who may interrupt a
cast; both arrive with their own customer.

**R2 — No new machine. Vicious Mockery is the contest with a damage effect kind, and the
widening is additive.**
§6 of the brainstorm is the rule: *a machine exists only when we need a new shape or
different steps.* Vicious Mockery needs neither. The contest already runs Request(save) →
outcome policy → deliver → Done (`contest.go:144-215`), which is save-then-effect exactly.
Its delivery is condition-only, and that is a **missing effect kind**.

Four additive widenings, and nothing else moves:

| Type | Today | Delta |
|---|---|---|
| `ContestInput` (`contest.go:21-29`) | `Gate, SaverID, Application, Cause, DamageTaken, Roller` | one field, `Damage []damage.Damage` — the declared consequence beside the declared condition |
| `ImposedEffect` (`contest.go:32-35`) | `Ref, Description` | a `Kind` (`condition` \| `damage`) and the damage's amount and components; the existing path sets `Kind: condition` and reads identically |
| the delivery step (`publishPreparedCondition`, `contest.go:111-131`) | one `Gather` publishing `ConditionAppliedTopic` | a sibling `Gather` applying damage through `combat.ApplyDamage` (`combat/combatant.go:157`), chained through the `next()` the existing one already takes |
| `resolve` (`contest.go:201-215`) | success → `Done`; failure → publish, then `Imposed` | failure chains damage then condition; **the success branch is untouched** |

**A negated save still delivers nothing.** `resolve` returns `Done` on success at
`contest.go:209-210` before any delivery runs, so the policy refusal at `:69-71`
(`Negated` only) and the recurrence refusal at `:72-74` both stay exactly as they are. RAW
2014 agrees: a successful Wisdom save negates the damage *and* the rider. Half-on-success
arrives with a spell that has one.

`ContestOutcome` (`contest.go:38-45`) does not change shape at all — `Imposed
[]ImposedEffect` simply now carries two kinds.

*Rename note, deliberately not taken:* with a damage kind, "contest" describes one of the
machine's two consequences rather than the machine. The honest name is closer to *the save
and what it costs*. Renaming is a mechanical change across one exported constructor and its
one caller, and it should ride the third customer rather than this slice — a rename here
would be the largest diff in a slice whose point is that no machine was added.
*Scope:* one saver, one gate, one condition, one damage set. Fan-out is §3.1.

**R2a — True Strike enters no machine of its own either; it is a delivery.**
A cantrip with no gate has nothing to resolve. Its condition is published on the bus and
read back by **the collector the activation results already use**:
`newActivationEffectCollector` subscribes to `ConditionAppliedTopic`
(`resolution/activation.go:145-160`), `captureConditionApplied` turns the event into an
`ActivationEffect{Kind: EffectConditionApplied}` (`:230-250`), and the whole thing runs
inside one `Gather` (`activationMachine.step`, `:503-560`). The publish itself is
`contest.go:111-131`'s body verbatim. The session then records those effects through the
path it already walks — `activationResults` (`session/activate.go:327`) into
`RecordActivation` (`:296-305`).
So the gateless arm is the **activation machine's delivery**, given a prepared condition
instead of an ability ref — one additive arm on `ActivationInput` (`activation.go:23-59`)
beside the ability contract at `:469`. No new machine, no new collector, no new beat path.
*Scope:* a condition on the caster or on one target. Healing rides the same collector the
day a cantrip heals.

**R3 — A cantrip is content that compiles to an `actions.Definition`, and `Definition` gains
its second profile arm.** It already says *"exactly one profile must be populated"* and holds
one (`definition.go:14-20`). This adds `Cast *CastProfile`, a second branch in `NewAction`
(`action.go:19-34`), and second arms in `Validate` and `Clone`. **Nothing in `CastProfile`
names a spell**, so a warlock's Eldritch Blast or a monster's innate cast declares a profile
rather than asking for a case (ADR-0045, `resolution/ARCHITECTURE.md:89-97`). The 11 bard
cantrips live in `rulebooks/dnd5e/spells/`, two with a profile and nine with none.
**The word "cast" lives at the door and nowhere below it.** `Cast` is a verb on Afford, a
verb on the wire and a profile arm on a definition. `resolution` never learns the word: it
sees a definition whose cast arm carries a gate or does not, and picks the contest or the
delivery accordingly (R2, R2a). Nothing in `resolution`, `conditions` or `encounter` names a
spell, a cantrip or a school.
*Scope:* cantrips. A levelled spell adds a pool to `Cost` and nothing to this shape.

**R4 — The spell save DC is answered by the caster, and travels as `DCStatic`.**
`DCSource` is sealed with a stated discipline — a new case must cite a RAW rule — and exists
for DCs *derived at resolution time* from a `DCInput` (`saves/gate.go:55-96`, discipline at
`:73`). A spell save DC derives from nothing the machine sees: it is 8 + proficiency +
spellcasting-ability modifier, all known before the door. So `character.SpellSaveDC()`
computes it from `ProficiencyBonus()` (`character/character.go:201`) and the class's
`SpellcastingAbility` (`classes/data.go:26`, bard CHA `:251`); the session puts the number on
the gate as `DCStatic` (`gate.go:100`); and `DCKindStatic` is what the wire says, which is
also what the player must read — *"DC 13"*, not *"some number"* (`gate.go:92`).
*The special case above* is a fourth `DCSource`. *The primitive below* is the caster owning
its own DC, which every future spell, every monster's innate DC and the save-DC-on-the-sheet
projection all read from. Recommend the primitive.
*Scope:* one caster, one DC. A feature that raises a single spell's DC is a modifier on the
sheet's answer, not a fourth source.

**R5 — True Strike grants advantage against a named target, and concentration is ignored
this slice — stated, not hidden.** `dnd5e:conditions:true_strike` is applied to the
**caster**, parameterised with the target's member id, and subscribes to the attack chain
exactly as `conditions/helped.go:165` does: when the attack's target matches, append one
`AdvantageSource` and consume itself. It ends at the caster's turn end (`raging.go:73` is the
worked subscription) and on combat end.
**Divergence, one line: RAW's True Strike is a concentration cantrip whose advantage applies
on your *next* turn; here it applies to the caster's next attack against that target, the
condition ends at the end of the caster's next turn, and there is no concentration.**
Concentration is §3.2's owning condition, the largest single missing shape, and it is not
bought here for one cantrip.
*Scope:* True Strike's divergence alone. It does not license ignoring concentration for a
levelled spell, which is exactly when the shape has to exist.

**R6 — Vicious Mockery is range and sight, and hearing stays shelved.** RAW is *"a creature
you can see within range"* that *"can hear you"*. Sight exists — `intel.Holding` is what
every candidate builder consults (`session/activations.go:304-322`) — and hearing does not,
which slice one already ruled (#397 R5). Target rule: a member **not the caster**, held in
the caster's sight, **within 60 ft**. True Strike's range is 30 ft by the same builder.
Deafness and silence stay on the shelf, named.

**R7 — The save is its own beat, and it is one new kind. The cast's consequences reuse
`ACTIVATION_RESULT`.** The player must see the roll, the DC and the outcome, and no existing
body carries them honestly: `DeathSaveRolled` is death-shaped — `stabilized`, `dead`,
`hp_restored`, a continuation (`events.proto:389-419`) — whose zero values would lie on every
ordinary save, and `Door` carries `total`/`dc`/`beaten` but is a door. So
`EVENT_KIND_SAVED = 27` with `Saved{saver, ability, roll, total, dc, succeeded, SpellRef
source}` at body tag 33, and `EVENT_KIND_CAST = 28` with `Cast{actor, SpellRef spell,
target}` at tag 34, `SpellRef` mirroring `AbilityRef` (`types.proto:1092`).
The *effects* are reused. `ActivationResult{actor, oneof result}` already carries
condition-applied and condition-removed (`events.proto:673-680`, `:714-729`) — precisely what
both cantrips deliver — and its one-arm invariant is already defended in rpg-api
(`convert.go:979-981`). It gains **one arm**, `DamageApplied = 6`, modelled on
`HealingApplied` (`:690-710`) including its `RollCalculation`, so the 1d4 face reaches the
client. The message's *name* is the only wrong thing about it, and renaming a proto message
is a break bought for a word the player never sees — the trade rpg-project#398's R6 made for
`STRIKE`/`HOLD`.

**R8 — Spells travel as `dnd5e:spells:<id>` ref strings, replacing the `Spell` enum on the
two choice fields, and the enum is not deleted in this slice.** Kirk's ruling from #397: ref
strings **replacing** the enum, never side by side. `SpellOptions.available`
(`choices.proto:149`) and `SpellSelection.spells` (`:230`) become `repeated string`;
`Character` gains `known_cantrips = 30` and `known_spells = 31`. That is `FIELD_SAME_TYPE`
under the repo's `FILE` policy (`buf.yaml:7-9`) and **needs the `breaking-change-approved`
label**, which is Kirk's to apply. The enum stays declared, because `SourceRef.spell`
(`common.proto:205`) and `SpellInfo.spell_id` (`character.proto:842`) still use it; deleting
it here would be an unrelated break in the same PR. *Ids are hyphenated:*
`dnd5e:spells:true-strike`, `dnd5e:spells:vicious-mockery` (`refs/spells.go:48-49`).

**R10 — A cantrip is paid at the door, and it is the first thing in the stack that really
is.** The whole price of a cantrip is one action: no pool, no slot, no charge on any
feature. The session compiles `SpendProfile{Slots: {ActionStandard: 1}}`, hands it to
`Resolve` as a non-nil `Input.Cost`, and `payAtTheDoor` charges it after pure machine
preflight and before the first yielded step (`resolution/cost.go:148-173`, `Input.Cost`'s own
doc at `resolve.go:83-92`). The charge is all-or-none by the gate's construction, so a bard
with no action left is refused **before anything moves** — no publish, no roll, no dirty
sheet — and the price is what Afford already showed on the row.
This is what slice one could not have. rpg-project#397's R1 tried to charge an activation at
the door and failed, because `activateFeature` checks the economy and the feature spends its
own pool underneath, so a door that charged either one made the feature refuse itself. A cast
has nothing underneath it: no `ActivateAbility`, no feature-owned spend, no second currency.
The door charges once and the machine is never told, which is the ignorance
`resolve.go:83-92` asks for.
*Scope:* cantrips, whose only price is an action. A levelled spell adds a pool entry to the
same profile and changes nothing about who charges it.

**R9 — All 11 bard cantrips are offered, and nine of them cast nothing.**
`bard-cantrips-1` returns to `getBardRequirements` with count 2 over the 11-cantrip list. A
known cantrip whose content carries no `CastProfile` mints **no Cast row** — the Afford
compiler skips it, fail closed, rather than offering a row that resolves to nothing. The
consequence is real: **a bard who picks Mage Hand and Light gets no Cast rows at all**, and
the walk below exercises exactly that.
**Both options stay open; Kirk has not ruled.** The alternative is to gate the option list
to castable cantrips, the way the class list is gated to classes with behaviour — *"A CLASS
IS ADDED HERE WHEN IT HAS BEHAVIOUR"* (`ClassSelectionModal.tsx:27-33`). Its cost is **one
line** in `getBardRequirements` (`choices/requirements.go:366`): the `Options` slice is the
two castable refs rather than the eleven.
**Recommend the gate.** It is the rule this stack already applies one level up, it is
fail-closed rather than fail-quiet, and a menu that offers nine picks which produce no row is
exactly the shape rpg-project#397's walk kept finding — an affordance with nothing behind it.
The cost is that "choose 2 of 2" is not a choice at level 1, which is honest about where the
build is and disappears the moment a third cantrip gets a profile.
*Trigger to revisit either way:* the ninth cantrip with a profile, at which point the gate
and the full list are the same list.

## Shape, per module (bottom-up)

**1. `combat/actions`** — `CastProfile` and `Definition.Cast` beside `Attack`, with
`Validate`/`Clone` arms (`definition.go:14-53`). Range in feet, a target rule
(`self` | `one_creature`), an optional `*saves.SaveGate`, `damage.Damage` instances and
`ConditionApplication`s — every one of them a type that already exists.

**2. `spells`** — a table keyed by ref id returning a `*actions.Definition`. Vicious Mockery:
range 60, one creature, gate `{WIS, Negated, RecurrenceNone, DCStatic(n)}`, 1d4 psychic, one
condition. True Strike: range 30, one creature, no gate, one condition on the caster. Nine
other bard cantrips, no profile.

**3. `conditions`** — `true_strike` on the caster (target member id in params; appends one
`AdvantageSource` when the attack's target matches, then consumes itself; ends at the
caster's turn end and on combat end) and `vicious_mockery` on the target (one
`DisadvantageSource` on its next attack roll, then consumes itself). Both are
`helped.go:165` and `dodging.go:163` with a different end, persisted the way
`inspired.go:176-196` persists. Two arms in `factory.go:70-109`.

**4. `character`** — `SpellSaveDC()` (R4). `getBardRequirements` (`requirements.go:366`)
returns `CantripRequirement{ID: BardCantrips1, Count: 2, Options: <11 refs>}`, with
`Options` changing from `[]spells.Spell` to ref-id strings — which is what
`compileKnownSpells` already resolves (`draft.go:1210`).

**5. `resolution`** — the `Definition.Cast != nil` branch in `NewAction`
(`action.go:19-34`), which reads the profile's arms and returns a machine that already
exists: **gated → `NewContest`** (`contest.go:136`), **gateless → the activation machine's
delivery** (`activation.go:503-560`). The contest's four additive widenings are in R2's
table; `ActivationInput` (`:23-59`) gains one arm for a prepared condition. **No new machine
and no new file.**

**6. `encounter`** — `RecordCast` beside `RecordActivation` (`activation.go:22-37`): one
`cast` beat, one `saved` beat when the profile carried a gate, one result beat per delivered
effect, reusing the closed result kinds (`:173`) plus `damage-applied`. The roll-fact refusal
at `:394-408` grows exactly one arm so damage may carry an amount and a calculation.

**7. `session`** — `VerbCast` beside the six (`afford.go:41-89`, request list `:421-424`) and
`Declaration.Spell`. `buildCastOffers` in `session/casts.go`, sibling to
`buildActivationOffers` (`activations.go:49`): iterate `KnownCantrips()`, look each ref up in
the content table, **skip a ref with no profile** (R9), mint a declaration at `SlotAction`
with candidates from the profile's range and target rule, reusing the sight/standing/reach
machinery at `activations.go:304-384` with a hostile-side twin. `session/cast.go` mirrors
`session/activate.go:133-308` step for step, with one difference: **`Cost` is non-nil.**

**8. protos** — breaking (R8): the two choice fields to `repeated string`, plus
`Character.known_cantrips = 30` / `known_spells = 31`. Additive: `VERB_CAST = 7`,
`CastRequest`/`CastResponse` + `rpc Cast` mirroring `service.proto:508-553`, `:1641`,
`EVENT_KIND_SAVED = 27` and `EVENT_KIND_CAST = 28` with bodies at tags 33 and 34, `SpellRef`,
and `ActivationResult.damage_applied = 6` + `DamageApplied` (R7).

**9. rpg-api** — `handler.go:299-303`'s no-op spells loop becomes real and keeps the refs,
alongside the `CHOICE_CATEGORY_CANTRIPS` arm that has never existed. `createCantripChoice`
beside `createSkillChoice` (`converters.go:2372`), appended at `:2363-2365`;
`converters.go:343-352` stops emitting `SPELL_UNSPECIFIED`; `convertSpellToProtoEnum`
(`:3112`) is deleted with its last reference; `convertCharacterToProto` (`:1107`) projects
the known refs at `:1247`. A new `session/v1alpha1/cast.go` mirroring `activate.go:25-45`;
`Manager` gains `Cast` (`handler.go:27-55`). `eventKindToProto` (`convert.go:569`) and
`setEventBody` (`:696`) gain arms for both kinds **in the same change as the proto**;
`activationResultBodyToProto` (`:952`) grows the damage arm inside its one-arm invariant.

**10. rpg-dnd5e-web** — `ChoiceRenderer.tsx:29` gains a `CHOICE_CATEGORY_CANTRIPS` branch
rendering `EnumChoice` (`:82`), whose `chooseCount > 1` already drives the grouped checkbox
grid (`:107-114`); `ClassSelectionModal.tsx:336-428` gains a validation arm and submits the
refs in the one `UpdateClass` call; `SpellSelectionModal.tsx` and its dropped result
(`InteractiveCharacterSheet.tsx:1908-1911`) are **deleted**. The dock takes three edits
(`ActionDock.tsx:507-514`, `:52-72`, `:96-102`); targeting takes CAST in the two
member-targeting predicates the slice-one fix widened (`TargetSurface.tsx:47`,
`useSessionCombatExperience.ts:342`) plus an arming branch beside `:556`, and `selection.ts`
needs nothing. `story.ts` gains `cast`, `saved` and `activationResult.damageApplied` arms —
compile errors until written — with the save line copying the door's shape verbatim
(`story.ts:347-349`). Two ref-keyed rows at `characterPresentation.ts:49`. `presentation.ts`
gains a `'save'` fact category beside `:137` and `createRequest` takes its kind, preset and
range from the fact rather than the hard-coded `:553-555`.

## Slice-one presentation gaps, folded

**In scope:** the dice-vs-story conflict (`presentation.ts:847-853`). It is the same defect
the save roll hits, and the category fix serves both.
**Shelved:** the d20 rolling at window open rather than after the answer (the post-roll
window's own timing bug); the taken d6's face on the outcome beat, which R7 makes cheaper
since `DamageApplied`'s `RollCalculation` is the shape it wants; animating a damage die,
which no beat has ever done and which the runtime's d4 is already ready for; and rpg-api
dropping class resources from the sheet projection (`converters.go:1251-1255`), which
returns with slots because a cantrip needs no pool.

## The seven principles, run against this

**Ownership before mechanism** — the table above; the contested noun was the save DC, and R4
gave it to the caster rather than to the gate's sealed formula set. **Rulings carry scope** —
every ruling names what it does not settle. **Load-bearing or deferred** — concentration,
half-on-success, recurrence, fan-out, slots and hearing are all deferred with a named
trigger. **Zero values tell the truth** — `ImposedEffect` gains a `Kind` rather than an amount that
reads 0 for a condition (R2), `Saved` is its own body rather than eleven death-save fields
reading zero (R7), and a cantrip with no profile mints no row rather than an unavailable one
(R9). **Call sites read as statements** —
`NewAction` still reads "content chose the arm". **Record tells the truth same-day** — cast,
save and one result beat per effect, through `RecordCast`. **Fail closed loudly** — an
unknown spell ref is refused at compile (`draft.go:1212`), an unknown condition id at
`factory.go:109`, a definition with two profiles at `definition.go:30`, and a target out of
range is a candidate shortfall before it is chosen rather than an error after.

## Done-when — Kirk's walk

1. Create a level-1 bard. The class modal shows a **choose 2** cantrip grid — over 11
   options, or over the 2 castable ones if Kirk takes R9's gate — in the same checkbox grid
   the skills use. Pick Vicious Mockery and True Strike. The
   character finalizes and the sheet lists both cantrips.
2. Enter a fight. The dock shows two **Cast** rows, each priced as an action.
3. **True Strike** on the skeleton. The log shows the cast and the condition on the bard.
   The bard's next attack against the skeleton **shows advantage** — two d20s, the advantage
   source named — and the condition is gone afterwards.
4. **Vicious Mockery** on the skeleton. The log shows a **save beat**: the skeleton's Wisdom
   roll and total against the bard's DC, and whether it succeeded. On a failure, psychic
   damage lands with its 1d4 in the roll calculation, and a disadvantage rider shows on the
   skeleton's condition list; its next attack rolls at disadvantage. On a success, nothing
   happens and the log says so.
5. A bard who chose two cantrips with no behaviour — or a fighter — **offers no Cast rows**.

## Shelf

- **Spell slots as pools** (§5.1) and every levelled spell. Rung 3, with Healing Word.
- **Charm Person and the world graph** (§3.5). Slice three, as Kirk ruled.
- **Concentration — the owning condition** (§3.2), the largest missing shape, and the reason
  R5's divergence is a divergence rather than a bug.
- **Half on success and recurrence** (§3.4). Both still refused at `contest.go:69-74`, and
  the contest's success branch returns `Done` before any delivery runs (`:209-210`).
- **Fan-out** (§3.1) — one declaration, many targets. Thunderwave, and the first monster
  multiattack, arrive on the same loop.
- **Hearing, deafness and silence** as a capability, so R6's range-and-sight rule can narrow.
- **Out-of-bubble casting.** A cantrip costs an action and actions exist only inside a fight
  (`encounter/field.go:1106-1113`). Prestidigitation in a corridor needs §6.2's ritual row.
- **Deleting the `Spell` enum**, once `SourceRef.spell` and `SpellInfo.spell_id` are retired.
- **Counterspell at the door** — the third window producer, at `payAtTheDoor` rather than at
  a step or a roll.
- **The save DC on the character sheet.** R4 makes it answerable; nothing projects it yet.
- **Monster brains do not cast and are unchanged.** A monster does not know a bard holds a
  cantrip and will not play around it. Correct for now; a `Decider` input the day it is not.

## What would change the design

- **If Kirk gates the cantrip list to castable spells** (R9's alternative), the requirement
  builder changes by one line and nothing else in this design moves.
- **If a third and fourth consequence kind arrive** — a push, a forced move, a world fact —
  then `ImposedEffect`'s `Kind` is carrying a vocabulary rather than a discriminator, and the
  delivery `Gather`s should become a list the machine walks rather than a chain it hand-rolls.
  That is the trigger to revisit R2, and the trigger to take the rename with it.
- **If concentration lands first**, R5's divergence is deleted rather than kept, and True
  Strike becomes the shape's first customer instead of its excuse.
- **If the `breaking-change-approved` label is refused**, the ref-string swap becomes a
  `v1alpha2` bump (the repo's own second escape hatch, `ci.yml:43`) and every consumer
  re-pins in one wave — a larger change, not a smaller one, and worth saying before it is
  discovered.
- **If Vicious Mockery's damage should be visible as a rolling die** rather than narrated,
  the dice work grows from one category to a second interaction kind, and that is a
  presentation slice rather than an addition to this one.

---

## Issue seeds

One block per module that changes. Written to be cut verbatim.

### protos — `dnd5e:spells:<id>` ref strings, the Cast verb, and two beats

**Assumptions.**
1. Kirk applies `breaking-change-approved`; without it CI blocks at `.github/workflows/ci.yml:37`.
2. `SpellOptions.available` (`choices.proto:149`) and `SpellSelection.spells` (`:230`) have
   no consumer that needs the enum after this change — rpg-api's only readers are the
   discard arms and `convertSpellToProtoEnum`.
3. `EVENT_KIND_SAVED = 27` and `EVENT_KIND_CAST = 28` are free, and body tags 33/34 are free
   (`events.proto:240`, `:348`).
4. `ActivationResult`'s result oneof has 5 arms and 6 is free (`:673-680`).

**Definition of done.** `buf lint`, `buf format`, `buf breaking` (with the label) and
`buf generate` all clean; generated Go and TS compile. Must survive: nothing else in this
slice can start until the generated packages carry `VERB_CAST` and both event kinds.

**Depends on.** Nothing. This merges first.

### rpg-toolkit root (`combat/actions`, `spells`, `conditions`, `character`) — the profile, the content, the two conditions, the DC

**Assumptions.**
1. `Definition` can carry a second profile arm without breaking `NewAction`'s one caller
   shape (`action.go:19-34`).
2. The 11 bard cantrips are the 2014 PHB list, and only two carry a profile.
3. `CantripRequirement.Options` may change element type; `compileKnownSpells` already
   resolves ref ids (`draft.go:1210`).
4. `character.SpellSaveDC()` needs only `ProficiencyBonus()` and the class's
   `SpellcastingAbility` — no new sheet field.
5. Both conditions can be built from `conditions/helped.go` and `conditions/dodging.go`
   with a different end.

**Definition of done.** Unit tests: a definition with two profiles is refused; a definition
with a cast profile validates and clones without aliasing; a bard draft finalizes with
exactly 2 cantrips and refuses 1 or 3; an unknown cantrip id is refused at compile; a level-1
bard's `SpellSaveDC()` is 8 + 2 + CHA mod; the `true_strike` condition appends advantage
against its named target and nothing against another, and is gone after the caster's turn;
the `vicious_mockery` condition appends disadvantage to exactly one attack roll; both
round-trip through JSON; a level-1 bard still projects its status view. Must survive: a bard
finalizes on the walk env with two cantrips on the sheet.

**Depends on.** Nothing (toolkit-internal). Ships before resolution.

### rpg-toolkit `resolution` — contest delivers damage, and `NewAction` gains its cast branch

**Assumptions.**
1. `ContestInput`, `ImposedEffect` and the delivery step can be widened additively; every
   existing caller is the strike rider (`strike.go:752`) and reads `Imposed` unchanged.
2. `combat.ApplyDamage` over `combat/combatant.go:157` is reachable from a contest `Gather`;
   `Participants.Character`/`Monster` give the concrete `Combatant`.
3. The success branch (`contest.go:209-210`) needs no change, so the `Negated`-only refusal
   (`:69-71`) and the recurrence refusal (`:72-74`) stay exactly as they are.
4. `ActivationInput` can carry a prepared condition instead of an ability ref without
   disturbing the ability contract at `activation.go:469`.
5. `NewAction` picks the arm from the profile and never reads a spell id.

**Definition of done.** Scenes: a failed save delivers damage **and** the condition, in that
order, and `Imposed` carries both kinds; a successful save delivers neither and still records
roll, total and DC; a contest with no damage declared is byte-identical to today (the
regression that matters); a gateless cast applies its condition and the collector reports one
`EffectConditionApplied`; a cast with an empty action budget is refused at the door and
charges nothing; a strike with a save rider is unchanged. Must survive: the walk's steps 3
and 4.

**Depends on.** toolkit root.

### rpg-toolkit `encounter` — `RecordCast`

**Assumptions.**
1. The cast, the save and the delivered effects are three beat shapes, and the result kinds
   are `RecordActivation`'s plus `damage-applied` (`activation.go:22-37`, `:173`).
2. `rollFactsActivationResultField` (`:394-408`) refuses `Amount` on non-healing kinds and
   needs exactly one new arm.

**Definition of done.** Tests: a gated cast appends cast + saved + one result beat per
effect, in that order; an ungated cast appends no saved beat; a damage result carries its
amount and calculation and a condition result still refuses one. Must survive: the walk's
log reads cast → save → damage → condition, in order.

**Depends on.** toolkit root, resolution.

### rpg-toolkit `session` — `VerbCast`, the offers, the seam

**Assumptions.**
1. `sheet.KnownCantrips()` is populated for a finalized bard and empty for everyone else.
2. A known ref with no cast profile mints no declaration (R9), and that is not an error.
3. The candidate builders' sight/standing/reach machinery
   (`activations.go:304-384`) serves a hostile target with a hostile-side twin.
4. `Cost` non-nil on the cast path is charged exactly once.

**Definition of done.** Scenes: a bard with two castable cantrips is offered two CAST rows
at the action slot; a bard with two non-castable cantrips is offered none; a fighter is
offered none; a target at 65 ft is a candidate shortfall rather than an error; a target the
caster cannot see is not a candidate; casting spends the action and a second cast the same
turn shows no budget; the save beat and the result beats reach the session projection.
Must survive: every step of the walk.

**Depends on.** toolkit root, resolution, encounter, protos.

### rpg-api — the draft arm, the sheet projection, the Cast handler, the converters

**Assumptions.**
1. Cantrip choices arrive as ref strings and are stored verbatim; the toolkit refuses
   unknown ids, so rpg-api validates nothing about spells.
2. `convertSpellToProtoEnum` (`converters.go:3112`) has no remaining caller after
   `converters.go:343-352` changes.
3. `Cast` mirrors `Activate` in the `Manager` interface (`handler.go:27-55`) and the mock
   regenerates cleanly.
4. Both new event kinds must land in `eventKindToProto` (`convert.go:569`) and `setEventBody`
   (`:696`) in this same change, or they demote silently (`:635-636`).

**Definition of done.** Acceptance: a bard draft finalizes with two cantrips and reads them
back off the character; through the real session handler, Afford carries two CAST
declarations, `Cast` produces a cast beat, `Cast` on Vicious Mockery produces a save beat
with roll/total/DC/outcome and a damage result; no beat in the walked session arrives as
`EVENT_KIND_UNKNOWN`. Must survive: the walk on the local env.

**Depends on.** protos, toolkit session (pinned to minted tags).

### rpg-dnd5e-web — the cantrip picker, the Cast row, the save story, the save dice

**Assumptions.**
1. `EnumChoice` (`EnumChoice.tsx:82`) renders a choose-2 grid off `chooseCount` with no
   change beyond a new `ChoiceRenderer` branch (`:29`).
2. Deleting `SpellSelectionModal.tsx` and its call site
   (`InteractiveCharacterSheet.tsx:1897-1913`) removes the only consumer of
   `listSpellsByLevel`.
3. `story.ts`'s switch is exhaustive over `body.case` (`:210`), so the new arms are compile
   errors rather than silent gaps.
4. Adding a `'save'` fact category to `presentation.ts` (`:137`) and parameterising
   `createRequest` (`:553-555`) is enough to animate the save's d20; the damage die is out
   of scope.

**Definition of done.** `npm run ci-check` with the log grepped for `✗`/`❌` (it exits 0 on
failure). Playwright: a bard finalizes with two cantrips chosen through the grid. In a
session: two CAST rows render with server-authored labels, the target ring highlights
candidates, the save beat renders as "N against DC M · Succeeded/Failed", both conditions
render with their own icon and tone, and the save's d20 animates without a
`dice response conflicts with typed Story` warning. Must survive: every step of the walk.

**Depends on.** protos, rpg-api.
