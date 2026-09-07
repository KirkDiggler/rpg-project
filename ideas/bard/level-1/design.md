# A level-1 bard inspires an ally — design

**Date:** 2026-09-07
**Status:** Design. One slice, cut. Rung 1 of the bard pilot and nothing else.
**Umbrella:** `ideas/spells/` — levels 1–3, and the choices at 3 made real.
**Brainstorm:** rpg-project#391 (`ideas/spells/brainstorm.md`), §6 shapes and §7 rungs.
**Depends on:** the post-roll window, designed at
`ideas/session-combat/interrupt/post-roll/design.md` (rpg-project#398) and **included in
this slice**. Every other seam is shipped.

---

## What prompted this

Kirk's two framing rulings from the brainstorm, and they govern every line below:

> *"plan on the new places only ... a machine exists only when we need a new shape or
> different steps. Never a spell-named machine."*

> *"our main goal is to get level 1 to 3 and be able to choose the choices that happen at
> level 3."*

Slice one is the smallest honest step toward that: **a level-1 bard, with Bardic
Inspiration, and no spells.** No cast verb, no slot, no spells-known field, no level-up.
One die, granted to an ally at a price, spent on their next roll.

This is §7's rung 1, and it is chosen because it is a **Grant × condition(held die)** —
one shape the stack is one delta away from, and one effect kind it already carries.

## The facts this stands on (toolkit `origin/main`, rpg-api `origin/dev`, web `origin/dev`)

- **The bard is data-complete and behaviour-empty.** `classes/data.go:233-262` — hit dice 8,
  primary CHA, saves DEX+CHA, light armour, the five weapons. `classes.GetGrants` is a
  switch over four classes and returns nil for the bard (`classes/grant.go:77-91`), so a
  bard compiles with no features and no conditions.
- **A cross-entity condition grant is shipped and running.** Help publishes
  `ConditionAppliedEvent{Target: input.Target}` — a target other than the activator —
  and the recipient's own `SheetKeeper` applies it (`combatabilities/help.go:94-99`,
  `character/sheet_keeper.go:150`). Afford compiles Help as an Activate declaration with
  `TargetMember` and a candidate universe (`session/activations.go:80-113`), the client
  arms targeting off it, and `resolution.NewActivation` carries `TargetID` through
  (`resolution/activation.go:36-40`).
- **A ruled range constant is the precedent for reach.** `helpReachFeet = 5`
  (`session/activations.go:266`), with Kirk's own divergence-from-RAW note above it, and
  `inRange(enc, from, to, feet)` doing the check (`session/activations.go:358`).
- **Pools are expressible and unexercised.** `SpendProfile.Pools`
  (`combat/spend_profile.go:80`); the ledger answers `PoolLeft` and `SpendPool`
  (`character/ledger.go:178`, `:193`); `payAtTheDoor` charges all-or-none
  (`resolution/cost.go:148-173`). Nothing this rulebook compiles emits a `Pools` entry.
- **A per-rest pool is one table row.** `initializeClassResources` builds a
  `RecoverableResource` with a `ResetType` per class (`character/draft.go:1944-1975`), and
  `Character.LongRest` restores every long-rest resource to full
  (`character/character.go:411-421`). Rage's charges are exactly this.
- **A condition on the bus is the only ongoing effect, and Raging is the worked example**:
  attach, subscribe to the chains it modifies plus `RestTopic` and `CombatEndTopic`,
  publish removal, detach (`conditions/raging.go:73-165`, `:118`, `:148`).
- **A flat bonus on an attack roll already has a writer.** Archery does
  `e.AttackBonus += 2` on the attack chain (`conditions/fighting_style_archery.go:134`).
  Saves and checks carry *sourced* bonus lists (`events/events.go:426`, `:478`); the attack
  chain carries only the unsourced scalar (`events/events.go:286`). That asymmetry is real
  and is ruled on below.
- **A grant and a spend both have a beat and a wire, already.**
  `Encounter.RecordActivation` appends one activated beat plus one result beat per result,
  with `condition-applied` and `condition-removed` among its four closed kinds
  (`encounter/activation.go:22-37`, `:173`). Session projects them
  (`session/types.go:1311`, `:1319`), and the protos carry them:
  `EVENT_KIND_ACTIVATION_RESULT` with `ConditionApplied` / `ConditionRemoved`
  (`rpg-api-protos dnd5e/api/session/v1alpha1/events.proto:174`, `:661-717`).
- **The web dock needs no change.** *"The server authors the label. There is deliberately no
  ref-to-name table here"* (`ActionDock.tsx:53-56`).

### Where the instructions and the evidence disagree

Two, named rather than smoothed over.

- **Rage is not "paid at the door", and no activation is.** `Input.Cost` must stay nil for
  an activation, and that is a stated constraint rather than an omission: the ability
  spends its own slot and its own pool, so a `Cost` beside it would bill twice
  (`resolution/activation.go:354-363`). Rage debits `RageCharges` inside its own
  `Activate` (`features/rage.go:146`) and `activateFeature` consumes the action type
  (`character/action_economy.go:422`). **There is no existing feature paid at the door.**
  Bardic Inspiration is the first, and that is the Grant shape's entire delta.
- **`conditions/helped.go` is not legacy.** `refs.Conditions.Helped()` has a live factory
  arm (`conditions/factory.go:105`), Help is a live combat ability, and Afford offers it.
  It is prior art on the live path, not a dead file. Its *mechanism* is still the wrong
  template — an advantage source that expires on the granter's turn — and this design
  reuses its **seam**, not its shape.

## Ownership — every noun, and who holds it

| Noun | Owner | Why it cannot live anywhere else |
|---|---|---|
| The bard's level-1 row (hit die, saves, feature) | toolkit `classes` | `Data` is "what you are", `Grant` is "what you get"; the split is stated at `grant.go:20-22` |
| The `inspiration` pool and its max | toolkit `character` | pool maxes are compiled at finalize by `initializeClassResources` (`draft.go:1944`), and the sheet is what a rest restores |
| The price (bonus action + 1 use) | toolkit `combat.SpendProfile`, charged by `resolution` | the door is the only place a price moves (`cost.go:148-173`); a feature that charged itself would make Afford's "read the price off the same profile" a fiction |
| Who may be targeted, and how far | toolkit `session` | reach is a question about the board, and only the session holds positions, sight and standing (`activations.go:301-362`) |
| The held die, its size, its source | toolkit `conditions` — a condition on the recipient | it is an ongoing effect on a member, and a condition on the bus is the only shape for one |
| When the die expires | the condition's own subscriptions | Raging's law: a condition decides its own end (`raging.go:118`, `:148`) |
| The spend (which roll the die joins) | the condition, on the chain it subscribes to | the roll is not the bard's; only something attached to the recipient can join it |
| The grant beat and the spend beat | toolkit `encounter`, via `RecordActivation` | the composition is the only author of its record (`outcome.go` RecordInput doc) |
| Whether a bard may be created | rpg-dnd5e-web's allowlist, one line | `ClassSelectionModal.tsx:28` is the only occurrence in the repo |

## Rulings

**R1 — Bardic Inspiration is paid at the door, and it is the first thing that is.**
The feature spends nothing itself. The session compiles a `SpendProfile{Slots:
{ActionBonus: 1}, Pools: {inspiration: 1}}` and hands it to `Resolve` as a `Cost`, and
`payAtTheDoor` charges it all-or-none before the first step. This is the Grant shape's
delta over Activation, and it means `activation.go:354-363`'s "Cost must stay nil"
narrows: it stays nil for an ability that spends its own, and this one does not.
*Scope:* this ruling is about **new** priced activations. It does not re-plumb Rage,
Second Wind, or the six combat abilities, and it does not claim they are wrong.

**R2 — The spend is the post-roll window, and the window is in this slice.**
RAW 2014: *"the creature can roll the die and add the number rolled to one ability check,
attack roll, or saving throw it makes, after rolling the d20 and before the DM says
whether the roll succeeds or fails."* **We do not diverge. The player is asked.**

An earlier draft of this design spent the die automatically on the first eligible roll,
because the producer of a post-roll window inside a machine did not exist — `Step` is
sealed to `Gather | Request | Done` with `Pose` named and unbuilt
(`resolution/step.go:25-34`). Kirk's call is that the window is built here instead, and
the reason is not the bard: **the opportunity-attack window shipped and has never been
walked**, since the monster brain targets the closest member and never walks past anyone.
The inspiration die is a window that opens every fight, on a player's own attack, so it is
the machinery's first real walk as well as the bard's real choice. It is also the
primitive below Cutting Words, Shield-done-right, Silvery Barbs, Counterspell and
concentration, all of which need a machine that stops after a roll is seen.

The mechanism is designed in full at
`ideas/session-combat/interrupt/post-roll/design.md`. What this design consumes from it:
a `Pose` step between the d20 and `PostAttackRollChain`; a `PostRollOfferChain` the
recipient's own `inspired` condition answers with an offer; the shipped ledger, freeze,
Afford row and `React` verb, reused unchanged; and `OfferTakenEvent`, on which the
condition rolls its die and consumes itself — the die is spent when it is **taken**, so
declining costs nothing.
*Scope:* this ruling covers the attack roll. The offer chain on saving throws and ability
checks is the same shape and is shelved there, not built here — so the die's third and
second uses arrive with the chain's second fold, not with a second mechanism.

**R3 — The die never touches the attack chain, so the unsourced-`AttackBonus` problem does
not arise.**
The attack chain carries only `AttackBonus int` (`events/events.go:286`), which Archery
writes into blind (`fighting_style_archery.go:134`), while saves and checks carry sourced
lists (`:426`, `:478`). A die added there would be a number nothing names. With R2 the die
is added **after** the chain has folded and the d20 is known, by the machine, and its face
travels on `OfferTakenEvent` with the offering condition's ref on it. So provenance is
answered by the mechanism rather than paid for with a beat.
`AttackBonusSource` on `AttackChainEvent` remains the right primitive for effects that do
modify the roll before it is made — Archery is its first customer, not this die — and it
stays on the shelf with its own reason rather than as this slice's debt.

**R4 — The spend is still reported as an activation-result transaction, and the face is
still not a number on it.** The resumed Attack calls `Encounter.RecordActivation` with
`Ability` = the inspiration ref and one `ResultConditionRemoved{Target: recipient, Ref:
inspired, Reason: "spent"}` (`encounter/activation.go:342-357`). `condition-removed`
**forbids `Amount`** — `rollFactsActivationResultField` refuses every roll fact on a
non-healing kind (`:394-408`) — so the face is inside the attack's `Total` and nothing
names it there either.
What changes is that this no longer hides anything: the player **chose** to add it, having
seen the roll it was added to, and the window's own beat carries that roll and that total
(`RollWindowOpened`, the one proto row the window design adds). The story reads window,
then choice, then a struck beat whose total is higher. Naming the face on the spend beat
is still worth doing and is still shelf.

**R5 — "A creature who can hear you" is range only.** No hearing primitive exists;
sight does (`intel.Holding`, used by every candidate builder), hearing does not. The
target rule is: a **member of the actor's own kind** (the whole of allyhood at this seam,
`activations.go:270-276`), **not the actor**, **standing**, **within 60 ft**. Deafness,
silence and a hearing capability go on the shelf. Named, not discovered.

**R6 — No spell slots and no spells-known on the sheet, and that is a deletion, not a
no-op.** `compileSpellSlots` already gives any bard two first-level slots
(`character/draft.go:1332-1334`), unconditionally, and nothing in the stack spends a slot
or offers a spell. Two slots that nothing can reach is a zero value that lies. **Remove
`classes.Bard` from that switch arm in this slice**; rung 2 restores it as pools (§5.1)
and deletes the map. This also keeps the bard out of `long_rest.go:130`'s clone-instead-of-
reset defect, which stays true and stays invisible.

**R7 — One die per creature; a second grant on an already-inspired creature is refused at
the door, not silently replaced.** RAW says a creature can have only one Bardic
Inspiration die at a time. Refusing is the fail-closed answer: replacing would spend a
use to overwrite a use, and the player would see a charge and no change. The refusal is a
`Shortfall` on the candidate row — "already inspired" — the same way "ally is down" and
"ally out of reach" are (`activations.go:354-361`), so it is visible before it is chosen.

**R8 — The die lasts until the fight ends or the party rests, because ten minutes is not a
boundary.** `BoundaryKind` is `turn_started | turn_ended | combat_ended`
(`encounter/turndriver.go:349-365`) — there is no round boundary and no minute. The
condition subscribes to `CombatEndTopic` and `RestTopic` exactly as Raging does
(`raging.go:148`, `:118`). **Divergence: ten minutes becomes "this fight".** Out of a
bubble nothing crosses any boundary at all, so a die granted in free roam persists until a
rest; that is the honest consequence of having no clock there, and it is on the shelf.

## Shape, per module

**1. toolkit `classes` — the bard's level-1 row.**
`getBardGrants()` beside the four that exist, and a `case Bard:` arm in `GetGrants`
(`grant.go:77-91`). One grant at level 1: `Features: []FeatureRef{{Ref:
"dnd5e:features:bardic_inspiration"}}`, plus the armour and weapon proficiencies the
class already declares as `Data`. Hit dice and saving throws stay in `Data` — they are
"what you are" and the doc says so. No `ConditionRef`: the inspired condition is granted
in play, never at creation.
Also here: `getBardRequirements`'s skill options. It sets `Options: nil` with the comment
*"Bards can choose ANY 3 skills"* (`character/choices/requirements.go:365-372`), and
rpg-api's `createSkillChoice` returns **nil** when the option list is empty
(`converters.go:2372-2375`). So the answer to "what stopped admitting the input?" for
skills is a **sentinel nobody implements**: empty means "any" on one side and "none" on
the other, and the bard is offered no skill choice at all. Enumerate the eighteen skills.
"Any" is not a concept the stack needs for one class.

**2. toolkit `character` — the pool and the feature.**
`resources.Inspiration coreResources.ResourceKey = "inspiration"` beside `RageCharges`
(`resources/keys.go:18`), with a `DisplayName` arm so the status view can carry it
(`keys.go:49-66`). A `case classes.Bard:` in `initializeClassResources` (`draft.go:1944`)
building a `RecoverableResource{Maximum: max(1, chaMod), ResetType: ResetLongRest}` —
Rage's arm with a different number. `LongRest` then restores it with no new code
(`character.go:411-421`).
`features/bardic_inspiration.go` + a factory arm (`features/factory.go:65-84`).
`ActionType() == coreCombat.ActionBonus`. **Its `Activate` spends nothing** — R1 — it
publishes the condition and returns. A `targetKindForRef` arm returning
`TargetKindSingleEntity`, beside Help's (`action_economy.go:284-298`).

**3. toolkit `conditions` — `inspired`, on the recipient.**
`refs.Conditions.Inspired()` and a factory arm (`factory.go:70-106`). Parameters: the die
(`d6`) and the source (the bard's member id) — persisted as opaque JSON, loaded back, the
way every condition on that list is. Subscriptions, three rather than five:
`PostRollOfferChain` (append `Offer{Ref, Name, Audience: the recipient, Die: "1d6"}` — it
offers, it does not add), `OfferTakenTopic` (its own ref and member only: publish
`ConditionRemovedTopic` with `Reason: "spent"` and unsubscribe), then `CombatEndTopic` and
`RestTopic` for R8's end. **It never subscribes to `AttackChain`** — R3. Helped's own note
applies, that unsubscribing mid-dispatch is safe because the bus snapshots subscribers
first (`conditions/helped.go:151-153`).
The save and ability-check arms are the same offer on two more folds and land with the
window design's own shelf item, not here.

**4. toolkit `session` — the offer, the price, the beats.**
`buildActivationOffers` (`activations.go:48`) already emits a row per ability with the
right slot and label. Two additions: an `inspirationCandidates` builder beside
`helpCandidates` (same holdings, same standing consult, `inspirationReachFeet = 60`,
plus R5's not-self and R7's not-already-inspired rows), and a compiled `Cost` on the
Activate path where today it is *"nil ON PURPOSE"* (`session/activate.go:272`). The grant
beat is `RecordActivation` with one `ResultConditionApplied` — the call the verb already
makes (`activate.go:296-305`). The spend beat is R4, recorded by the **resumed** attack —
`React`'s post-roll arm, not `Attack` — when the machine reports the offer taken.

**5. rpg-api — nothing but pins.** The class list iterates `classes.ClassData` wholesale
and already returns the bard (`orchestrator.go:747-756`). Skill choices already reach the
sheet through `UpdateClass`'s `ClassChoices.Skills`
(`handlers/dnd5e/v1alpha1/character/handler.go:288-292`) — note `UpdateSkills` is
`Unimplemented` (`handler.go:541`) and is not the path. The session handlers are
proto↔SDK translation with no rule in them. Re-pin and re-run.

**6. rpg-dnd5e-web — one line, and a check.** Delete `Class.BARD` into `ALLOWED_CLASSES`
(`ClassSelectionModal.tsx:28`). The dock is untouched (server-authored label). The
recipient's own panel already lists conditions generically with a fail-closed glyph
(`characterPresentation.ts:66-83`), and `conditionIcons.ts:192` already holds an
`inspired` entry with 🎵 — written for this, unreached.
**The bard cannot see the die they granted.** `GetCharacterData` is owner-gated
(`SessionEncounterView.tsx:244`, `SessionCanvas.tsx:633`), so a condition on an ally is
visible to that ally and to everyone through the two beats, and nowhere else. A shared
"who is inspired" badge needs public roster conditions, which is rpg-toolkit#940's
audience work. Shelf.

**7. protos — no change.** The grant and the spend are both
`EVENT_KIND_ACTIVATION_RESULT` bodies that exist (`events.proto:174`, `:661-717`); the
Activate verb, its declaration, its target kind and the cost badge are all shipped.
Confirmed by reading, not assumed.

## Done-when, per layer

**toolkit `resolution` / `session` scenes.** Grant spends one use and one bonus action.
Refuse self. Refuse a target at 65 ft. Refuse with an empty pool, before anything moves.
Refuse a second grant on an already-inspired creature, as a candidate shortfall rather
than an error. An inspired recipient's attack **poses a window** instead of resolving:
Afford shows them a REACT row named "Bardic Inspiration" and everyone else
`ShortfallWindowOpen`. Answering `strike` adds exactly one face and consumes the die;
answering `hold` adds nothing and leaves the die in hand for the next attack. An attack by
an uninspired member poses nothing and is unchanged. Consumed exactly once — a second
attack after a spend poses no window. Ends on combat end. Ends on a rest. Long rest restores
uses to `max(1, chaMod)`. A grant with a full profile and an empty ledger charges nothing
at all (all-or-none).

**rpg-api acceptance.** A bard draft finalizes with three chosen skills, a
`bardic_inspiration` feature, an `inspiration` pool at its CHA max, and **no spell slots**
(R6). Through the real session handler: Afford offers the Activate declaration with
member candidates; Activate produces an activated beat and a condition-applied result; the
inspired member's Attack returns paused and their next Afford carries the REACT row.

**web integration.** The bard appears in the class modal. The dock shows the
server-authored label with the bonus-action slot and the pool as its price. The
recipient's condition list shows the die. The reaction panel from the OA window renders a
post-roll window: the d20 and the total so far, and two buttons.

**The walk.** Create a level-1 bard. Enter a fight. Inspire the fighter as a bonus action;
the log shows the grant and the pool drops by one. The fighter attacks and the dock stops
on a panel showing the d20 and the total so far, with "Bardic Inspiration" and two
buttons. **Spend:** the struck beat's total is the roll plus the modifier plus the d6, a
spend beat says the inspiration was used, and the die is gone — the fighter's next attack
poses nothing. **Keep:** the total carries no die and the fighter still holds it into the
next attack, which poses the window again. Both runs, same fight. Long rest; the uses come
back.

## Shelf

- **`AttackBonusSource` on the attack chain**, mirroring the save and check bonus lists,
  projected onto the struck beat. R3's primitive; Archery is its first customer, and the
  inspiration die is no longer waiting on it.
- **The offer chain on saving throws and ability checks**, which is the rest of Bardic
  Inspiration as written and the window design's own shelf item.
- **Naming the die's face on the spend beat**, which `condition-removed` forbids today
  (R4).
- **Cutting Words** — the same window, audience a hostile bard, the first one posed to
  somebody other than the roller.
- **Spellcasting**: the Cast door, slots as pools (§5.1), spells-known on the sheet
  (§5.2). Rung 2, Healing Word.
- **Instruments and tool proficiency.** The requirement exists
  (`choices/requirements.go` `BardInstruments`, count 3) and nothing consumes it.
- **Hearing, deafness and silence** as a capability, so R5's range-only rule can narrow.
- **Duration outside a bubble**: ten minutes and one hour are not boundaries, and free
  roam crosses none at all.
- **Levels 2 and 3**: Jack of All Trades (a passive check-chain bonus, the cheapest thing
  in the class), Song of Rest, the College choice — all gated on the level-up table work,
  which §4 owns.
- **The bard seeing their own die on an ally**: public roster conditions,
  rpg-toolkit#940's audience grain.
- **Monster brains ignore inspiration.** A monster does not know an ally carries a d6 and
  will not target around it. Correct for now; a `Decider` input the day it is not.

## What would change the design

- **If the post-roll window slips out of this slice**, the die has no honest spend: the
  automatic default this design used to carry is deleted, not held in reserve, so the
  fallback is to ship the grant with no spend at all rather than to ship a spend nobody
  chose.
- **If a second post-roll customer appears before this merges**, the window's
  single-audience and one-pose-per-run rulings both have to give, and that is a change to
  rpg-project#398 rather than to this design.
- **If Kirk wants the bard's own screen to show the die**, this slice grows a public
  condition projection, which is an audience change and belongs in its own design.
- **If a second priced activation appears before this merges**, R1 stops being a first and
  the profile compilation should move out of the bard's path into whatever both share.
