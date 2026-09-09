# Bane — one paid declaration, three saves, one visible d4

**Date:** 2026-09-09
**Status:** **APPROVED** — Kirk's eight final rulings are incorporated; implementation has not started.
**Umbrella:** `ideas/spells/` — levels 1–3, and the choices at 3 made real.
**Related journey:** [rpg-project#243](https://github.com/KirkDiggler/rpg-project/issues/243).
**Research:** [rpg-toolkit#1593](https://github.com/KirkDiggler/rpg-toolkit/issues/1593) and [#1595](https://github.com/KirkDiggler/rpg-toolkit/issues/1595), both In Review on Project 19.

Start with [the layer overview](overview.md) for responsibilities, named seams, and lifecycle diagrams.
It explains the composable layer Bane joins; this document specifies the Bane behavior within it.

This document is the approved behavior and architecture for the Bane slice. It does not claim that
production code exists, that production tests pass, or that a release has occurred.

---

## The player promise

A level-1 Bard can choose Bane as this slice's **one supported level-1 spell choice**. The bard selects
one to three distinct creatures within 30 feet and casts once. One standard action and one level-1
spell-slot resource are charged once. Each target makes its own Charisma save in the selected order.
A target that fails receives Bane; a target that saves does not.

All delivered Banes belong to one concentration condition on the caster. If all targets save, the cast
is still paid and the caster still holds that zero-child concentration condition. A failed payment or
an invalid declaration changes no action, spell resource, concentration owner, child condition, or dice
stream.

On every affected attack roll or saving throw, including a concentration save and a death saving throw,
the resolved calculation contains exactly one selected Bane d4:

```text
goblin attacks bard          d20 14 + 4 - d4 [3] (Bane, bard-a) = 15 · misses
goblin makes a DEX save      d20 12 + 2 - d4 [1] (Bane, bard-a) = 13 · fails
```

The physical d4 face remains positive. Its component says that the subtotal is subtracted. The generic
result retains Bane's content ref, the selected caster's entity ID, the notation and actual face, the
operator, and the checked total. A client may present those authoritative facts. It does not roll a
second die, choose a Bane source, or recompute the total.

The selected contributor entity ID is the contract needed by a future player-dice-set lookup. Dice-set
lookup, customization, animation, and a new Story/dice renderer or preset system are not part of this
slice and are not acceptance blockers.

---

## Rule and slice boundary

Bane (2014) is a 1st-level enchantment with a 30-foot range and concentration up to 1 minute. Up to
three creatures make Charisma saving throws. Until the spell ends, a creature that failed subtracts
`1d4` from every attack roll and saving throw.

This slice proves one new spell shape at a time: one paid levelled declaration, an ordered target list,
per-target save gates, one-to-many concentration ownership, and an automatic contributed die. It does
not add:

- Bless content, upcasting, cast-level selection, or unequal-potency stacking;
- additional Bard spells merely to fill the eventual rules-as-written known-spell count;
- prepared-caster acquisition, multiclass spell slots, Pact Magic, or Arcane Recovery;
- area templates, line-of-sight rules beyond the current candidate/reach seam, half damage, or movement;
- Bane on ability checks (it applies to attacks and saving throws only);
- a dice-expression grammar, signed notation, negative physical faces, or a fixed/dice expression union;
- a Bane-specific result lane, manual d4 request, session-owned bus, or session-owned roll;
- player dice-set persistence, lookup, customization, animation, or new dice presets.

The existing full class progression tables remain factual source data. They must not be edited to claim
that a Bard's eventual rule is “one known spell.” The supported creation requirement is deliberately a
smaller product offering: choose the one implemented spell from the one implemented option.

---

## Responsibility chain

The supported direction is composable encounter plus resolution, using composable play/world:

```text
D&D creation requirements offer one supported Bane ref
  -> the D&D rulebook compiles knowledge, slot resource, profile, and concrete price
  -> session compiles ordinary Cast declarations from rulebook output
  -> the existing resolution door preflights and pays the whole price once
  -> one cast machine runs ordered per-target save/delivery machines
  -> recipient-owned condition lists select applicable conditions in persisted order
  -> the selected Bane condition describes its own d4 contribution
  -> resolution supplies those facts to shared attack/save/death-save rolling
  -> generic RollCalculation preserves the settled facts
  -> encounter records and session/API wire copy them
  -> the web sends target IDs and presents provider-authored results
```

The old top-level encounter, registries, and session arithmetic are evidence of earlier behavior, not
templates. Resolution owns one event bus per interaction and tears it down. Step machines never receive
the bus. Session supplies repositories, identifiers, and a roller; it neither selects Bane nor rolls a
die. Encounter records neutral facts and does not interpret Bane. API converts fields. The web renders
and calls.

Before implementation grows, the narrow stale toolkit guides identified in the implementation plan are
aligned with this direction. Historical ADRs are not rewritten.

---

## 1. Acquisition and one slot authority

### One supported Bard choice

Level-1 Bard requirements reuse the existing `choices.BardSpells1` ID and
`choices.SpellbookRequirement` path:

```text
ID:          BardSpells1 ("bard-spells-1")
Count:       1
SpellLevel:  1
Options:     [spells.Bane]
Label:       "Choose 1 supported 1st-level spell"
```

The API projects it through the existing `SpellOptions.available_refs` field as
`dnd5e:spells:bane`, with `CHOICE_CATEGORY_SPELLS`. Creation submits the existing
`SpellSelection.spell_refs` shape. Validation accepts exactly one ref from this supported offering; it
does not demand four implemented spells, auto-add content, or accept an unsupported ref. Finalization
stores the canonical ref in `KnownSpells`. `KnownCantrips` remains separate and unchanged.

This is a known-spell path for Bard only. No prepared-caster path is created.

### Recoverable resources are canonical

The only mutable level-1 slot authority is the existing recoverable resource ledger. This slice adds the
D&D resource key:

```go
const SpellSlotLevel1 coreResources.ResourceKey = "spell_slot_level_1"
```

A finalized level-1 Bard receives a long-rest recoverable resource with current and maximum value `2`,
read from the existing Bard level-1 class progression. The slice does not seed other caster classes or
higher slot levels.

Bane's rulebook-compiled `combat.SpendProfile` is:

```text
Slots[standard action] = 1
Pools[SpellSlotLevel1] = 1
```

Cantrip definitions remain action-only. `combat.CanPay` and `combat.Pay` remain the atomic door;
character resource persistence, dirty marking, and `LongRest` remain the spend/recovery path.

The speculative character `SpellSlots` runtime/data field and the unused `Character.spell_slots` proto
field are deleted. The proto field number and name are reserved. There is no migration, converter,
legacy reader, dual writer, or fallback. This is an intentional pre-playtest contract retirement, not
a request to wipe any environment.

### Rulebook-owned authorization and price

The compiler uses a named `CastDefinitionInput` with `Spell` and `SpellSaveDC` fields. The DC is not a
spell, slot, or character level. No unused future fields, target list, roller, bus, or whole character
are added to this compilation input; execution has its own resolution input.

`spells.CastDefinition` continues to return nil for content this build cannot cast. It now returns a
complete cost as well as the profile for supported player-cast content. Session intersects
`KnownCantrips` and `KnownSpells` with non-nil definitions and copies the rulebook's price; it does not
infer a spell level from a ref or own a Bard table.

Thus only a Bard that selected Bane receives a Bane Cast declaration, and only Bane is newly enabled.
Other catalog entries remain known content without cast behavior and receive no unusable Cast row.

---

## 2. Ordered target list and one payment

A cast uses one target-list spelling at every layer. Existing singular casts use a list of one; a
self/no-target cast uses an empty list. Bane declares minimum 1 and maximum 3 creature targets. The
public declaration exposes `max_targets = 3`; the existing `TARGET_KIND_MEMBER`, candidates, availability,
and provider-authored refusal reasons remain authoritative.

Before payment, toolkit declaration preflight and resolution validate all knowable declaration errors; the API host only translates the request:

- target count is within the profile's minimum and maximum;
- IDs are non-empty and distinct;
- every target belongs to the interaction and is currently eligible;
- every target remains in the declaration's candidate universe and within the current 30-foot reach;
- each target's save and condition delivery can be constructed.

An invalid third target fails the whole declaration. No target saves, no RNG is consumed, no action or
pool is debited, and old concentration remains.

The resolution result is one cast with ordered per-target outcomes:

```go
type CastTargetOutcome struct {
    TargetID string
    Save     *ContestOutcome
    Applied  []ImposedEffect
}

type CastOutcome struct {
    Spell     core.Ref
    CasterID  string
    Targets   []CastTargetOutcome
    FollowUps []FollowUpOutcome
}
```

Resolution pays the definition's one `SpendProfile`, then runs the existing contest/delivery sequence
for each target in caller order. A successful save has an empty `Applied`; a failed save has one Baned
condition. The encounter writes one Cast beat naming the ordered target list, followed by each target's
Saved and condition-result beats in that order. It does not write three casts or three prices.

For a new concentration cast, ordering is fixed:

```text
whole-list pure preflight -> one atomic payment -> synchronously drop old owner and children
                          -> ordered target contests/delivery -> install one new owner
```

A refused payment preserves the old owner and children. A paid all-save cast still drops the old
concentration and installs the new zero-child owner. After successful payment, an unforeseen runtime,
publication, repository, or persistence failure is reported honestly; this design does not promise a
refund or resurrection transaction it has not proved.

After the first successful cast, a same-turn Afford refresh shows Bane unavailable from the now-spent
action. A second same-turn Cast attempt is refused before mutation or RNG. The remaining level-1 pool
point is not mistaken for another action.

---

## 3. One owner clock and qualified children

Bane creates one `ConcentratingCondition` on the caster and zero to three Baned children. The owner is
the only duration clock. Every recipient shares it regardless of initiative position; children never
pause, restart, or count independently.

Bane's profile sets `TurnEnds = 10` and `SkipFirstTurnEnd = true`. The owner's persisted
`SkipNextTurnEnd` flag is true at creation. The casting turn's end consumes that flag without
decrementing `TurnEndsLeft`. The next ten caster turn ends decrement the count; the tenth ends the
owner and all of its children. Other members' turn ends do nothing. Reload preserves both the remaining
count and whether the one grace was consumed.

This policy is per concentration profile. True Strike keeps its current two-turn-end duration behavior;
Bane does not introduce a global concentration grace. Rage keeps its separate round/activity sustain
semantics and is not reused for spell duration.

Every spell-owned Bane address is source-qualified:

```go
type ConditionAddress struct {
    MemberID     string `json:"member_id"`
    ConditionRef string `json:"condition_ref"`
    SourceID     string `json:"source_id"`
}
```

For a Baned child, `MemberID` is the recipient, `ConditionRef` is the Baned condition ref, and `SourceID`
is the Bane caster. For the concentration owner, member and source are the caster. `SourceRef` remains
adjacent provenance for the Bane spell and is not address identity.

The same `ConditionAddress` is projected from the persisted condition/owner facts into imposed-effect,
condition-removal, owner child-list/JSON, and concentration-ended records. Matching compares all three
fields; empty `SourceID` is a value for existing unqualified conditions, never a wildcard. Every new
Bane condition and child rejects an empty source ID. The persisted Baned condition is the source of
truth for its Bane ref and caster ID; downstream addresses are copies used for matching, not additional
mutable source identities. No application ID is introduced.

A supported same-caster recast synchronously removes the old application before delivering the new one,
so two generations with the same qualified address never coexist. An application ID becomes justified
only if a later supported rule permits concurrent same-source/same-recipient/same-condition instances.

Ending bard A's owner removes only A's Bane children and A's owner bookkeeping. Bard B's same-ref Bane,
owner, child list, and remaining clock survive.

---

## 4. Recipient-owned applicability and condition description

The missing keeper-to-roll carrier is closed by one narrow D&D contract in
`rulebooks/dnd5e/events`; it is not a global registry and exposes no mutator or bus:

```go
type RollKind string

const (
    RollKindAttack      RollKind = "attack"
    RollKindSavingThrow RollKind = "saving_throw"
)

type DescribeRollContributionsInput struct {
    Kind RollKind
}

type DescribeRollContributionsOutput struct {
    Contributions []DiceContribution
}

type RollConditionOwner interface {
    DescribeRollContributions(
        input *DescribeRollContributionsInput,
    ) (*DescribeRollContributionsOutput, error)
}
```

`Character` and `Monster` implement `RollConditionOwner` from their own condition slices. Resolution's
`Participants` returns that interface for one participant ID. The owner scans its active conditions in
the exact slice order reconstructed from persisted `Data.Conditions`.

For this slice, all Banes are equal potency. The owner selects the first active Baned condition, then
asks **that condition** to describe itself. Later Banes remain active, owned, persisted, and counting
down. When the oldest ends, the next-oldest surviving Bane is selected on the next roll with its original
remaining owner duration. No duration resets or pauses.

Selection never uses subscription order, load/attach order, source-ID sorting, content-ref sorting, or a
rolled face. Generic dice arithmetic and the UI never select. Bane and a future Bless use different
stacking identities; a synthetic additive contribution may test composition without implementing Bless.

The selected `BanedCondition` returns one value and receives no roller. These exact signatures are a
concrete implementation proposal beneath the approved behavior, not separately approved API minutiae:

```go
type DiceContribution struct {
    Source   RollSource // Bane content ref/name and selected caster SourceID
    Dice     string     // unsigned homogeneous pool: "1d4"
    Subtract bool
}
```

`RollSource.SourceID` is the sole contributor-identity field in the contribution/calculation model.
The condition projects its persisted Bane ref and caster ID into that source once; evaluators and wire
converters copy it and never maintain a sibling contributor ID. The condition returns the value only for
`RollKindAttack` and `RollKindSavingThrow`. It stores source facts but no die face. This separates
persisted data (recipient, Bane ref, caster ID) from attached descriptive behavior (eligibility and
description) without making the condition, session, or event bus roll.

---

## 5. Generic roll calculation

Attack and saving-throw chain inputs gain `[]DiceContribution` beside existing fixed modifier lanes.
Descriptions validate before RNG: Bane requires a valid content ref, provider name, non-empty source
entity, unsigned homogeneous positive notation, and a subtract flag. Signed notation, composite terms,
negative faces, and arbitrary expressions are refused.

The shared evaluator rolls each selected contribution once and emits the existing generic result shape,
extended as follows:

```go
type RollSource struct {
    Ref      *core.Ref
    Name     string
    Label    string
    SourceID string
}

type RollComponent struct {
    Source       RollSource
    Dice         *DiceTrace
    Modifier     *int
    SubtractDice bool
}
```

`SourceID` is on `RollSource` in the public wire and is kept here on the generic source value; it never
replaces `Source.Ref`. `SubtractDice` applies only to `Dice.Subtotal`; an optional fixed modifier on the
same component remains signed and additive. Physical original/final faces, rerolls, kept indices, and
subtotal are always positive. Existing calculations default to additive.

Read the recipient's current attached condition state when the roll step executes, after earlier
interaction mutations such as ending old concentration. Do not cache contribution selection during
whole-cast preflight. The ordering convention is preserved by serialization; the input is not a stale
repository snapshot.

The authoritative order is:

1. select applicable conditions from the recipient's current application order;
2. roll the normal one/two d20 pool and apply existing advantage/disadvantage keep policy;
3. roll each selected contribution once in selected order;
4. build and validate one `RollCalculation`;
5. apply domain policy from the original d20 face and checked total.

Advantage changes only the d20 pool. It does not roll a second Bane d4. Natural attack miss/critical
policy remains in attack settlement. Ordinary saving throws have no natural-1/natural-20 override.

Existing strict source validation is preserved: every d20, fixed, reroll, Inspiration, and Bane
component has a valid canonical `RollSource.Ref` and provider name. New roll assembly receives the
source that owns the fact rather than manufacturing an anonymous component: an attack definition ref
owns its attack d20 and fixed attack modifier; an ordinary/concentration save's validated
`SaveCause.EffectRef` owns its d20 while the ability ref owns its fixed modifier; the separate death-save
evaluator receives a proposed canonical `refs.Actions.DeathSave()` for its d20 (and emits no invented
zero-modifier component); Inspiration uses its existing condition/feature ref; and Bane uses its spell
ref plus caster `SourceID`. `SavingThrowInput` therefore gains required `D20Source` and
`ModifierSource` values and `DeathSaveInput` gains required `D20Source`. `ValidateRollCalculation` is
not weakened to accept a missing ref for convenience.

### Every saving throw, including the separate death-save lane

The ordinary and concentration save paths use `RollKindSavingThrow` and carry the same calculation.
Consequently a baned caster subtracts the selected d4 when making a Constitution save to maintain a
*different* concentration spell.

Death saves currently bypass the ordinary saving-throw chain and call a separate `saves.MakeDeathSave`.
That path is explicitly in scope: resolution obtains the dying recipient's same
`RollConditionOwner` output, supplies it to death-save evaluation, and records the same generic
calculation. Natural 1 and natural 20 remain classified from the d20 face. For rolls 2–19, the checked
total after Bane determines success against DC 10. This is required for the promise “every saving
throw”; it is not silently assumed from ordinary-save coverage.

Ability checks remain unchanged and receive no Bane contribution.

---

## 6. Freeze, records, and wire

The attack machine settles the d20 and automatic contributions before posing Bardic Inspiration. The
version-2 frozen strike contains the complete `RollCalculation`. Resume validates and reuses it; it does
not refold conditions, reselect a Bane source, or reroll the d4. Answering `OfferKeep` keeps the
calculation unchanged. Answering `OfferSpend` appends only the Inspiration component and revalidates the
total.

Open version-1 frozen strikes fail closed under the new reader. There is no converter that fabricates
missing source/die facts and no dual reader. This is an accepted pre-playtest compatibility break, not a
claim of refund or automatic recovery.

One generic calculation shape crosses:

- attack outcomes and hit/miss records;
- ordinary, cast-target, concentration, and death-save results/records;
- version-2 frozen Inspiration windows;
- encounter/session mirrors and API wire.

Proto contract changes use one clean alpha shape:

- singular `CastRequest.target` and `Cast.target` are retired; their numbers and names are reserved, and
  new repeated `targets` fields use new tags;
- `Declaration.max_targets` carries `1` for current member-targeted cantrips and `3` for Bane;
- `RollSource.source_id` and `RollComponent.subtract_dice` extend the generic trace;
- attack response, Struck, Missed, Saved, DeathSave response/event, and RollWindowOpened carry the same
  `RollCalculation`;
- ConditionApplied/ConditionRemoved carry `source_id` for qualified ownership;
- `Character.spell_slots` tag/name are retired and reserved.

This is field/tag allocation and an intentional alpha contract break, not a data migration. Retired
numbers are never reused. There are no dual target writers/readers and no old SpellSlots converter.

Scalar roll/total fields may remain as domain summaries where current consumers need them, but they must
equal the authoritative calculation and are never used to reconstruct missing components. The web's
save Story removes `total - roll` inference and formats the provider calculation, including the subtract
operator and selected `source_id`. It may resolve that entity against the existing roster for a name.
It adds no player-dice-set lookup or 3-D d4 acceptance requirement.

---

## Acceptance contract

Implementation acceptance uses production entrypoints and persistence boundaries, not research models.

1. **Payment refusal:** missing standard action or level-1 pool refuses without action/pool debit, old
   owner/child change, or RNG use.
2. **Same-turn refresh:** one paid cast leaves one pool point, refreshed Afford reports the Bane row
   unavailable for the spent action, and a second same-turn Cast refuses without further mutation/RNG.
3. **Invalid third target:** empty, duplicate, absent, ineligible, or out-of-range third target refuses the
   entire list before payment or concentration drop.
4. **One supported acquisition:** real creation offers `bard-spells-1`, count 1, only
   `dnd5e:spells:bane`; finalization/reload preserves that one `KnownSpells` ref and seeds two level-1
   resource uses.
5. **Resource lifecycle:** one Bane spends one slot use; persistence retains the spend; LongRest restores
   two; no `SpellSlots` state or proto projection remains.
6. **Mixed saves:** one three-target cast pays once, rolls three ordered Charisma contests, applies Bane
   only to failures, and records one Cast plus all three outcomes.
7. **All-save replacement:** begin with the caster's old concentration owner and qualified children plus
   an unrelated caster's overlapping owner/children. Payment occurs once, then only the casting owner's
   old owner/children drop, all new targets save, and exactly the new zero-child owner remains; the
   unrelated owner, children, and clock are preserved.
8. **Two real owners:** ending bard A's actual concentration owner removes only A-qualified conditions
   and child entries; bard B's owner/conditions/children remain.
9. **Oldest and reload:** the oldest recipient application is unchanged after ToData/load; removing it
   hands contribution to the next-oldest without resetting either owner's duration.
10. **Duration:** recipients before and after the caster in initiative share one owner; casting-turn end
    is skipped, other member ends do nothing, and the tenth subsequent caster turn end expires all
    children.
11. **Attack calculation:** advantage rolls its normal d20 pool and exactly one Bane d4; positive face,
    subtract operator, Bane ref, selected caster ID, and checked total survive response/record/wire.
12. **Ordinary save calculation:** the same facts survive a non-concentration saving throw.
13. **Concentration save:** a baned concentration owner subtracts exactly one selected Bane d4 and both
    held and failed checks retain the full calculation; failed removal stays source-qualified.
14. **Death save:** the separate real DeathSave path applies Bane to the DC-10 total for d20 2–19 while
    natural 1/20 still use the face; response/event/wire retain the calculation and contributor ID.
15. **Inspiration:** `OfferKeep` and `OfferSpend` after reload reuse the frozen Bane face and selected
    caster; neither branch reselects/rerolls, and `OfferSpend` appends only Inspiration.
16. **Regression:** current cantrip casts remain list-of-one/action-only; True Strike timing and effect
    consumption remain unchanged; Rage sustain remains unchanged.

Research receipts establish why these seams are needed. They are not substitutes for these production
acceptance scenes.

---

## Delivery and publication

Bane is one coherent cross-repository wave, not one PR per discovered seam. Local development and later
publication are separate:

1. Protos are the early real-PR exception because CI mints the generated Go/TypeScript artifacts.
2. Toolkit work stays on one wave branch. Provider commits are pushed, and dependent toolkit modules and
   the local API module graph resolve those **actual commit SHAs** to Go pseudo-versions. No pseudo-version
   string is written by hand.
3. The local API branch pins those actual pseudo-versions in `go.mod`/`go.sum`; no committed
   `replace`/`go.work` is used. Local proof uses the existing named-stack contract:
   `envs/local/bane.env` plus `scripts/dev-env.sh up local/bane` and `status local/bane`. The manifest
   names actual pushed API and web refs and verified-free host ports. It omits `RPG_TOOLKIT_REF` and
   `RPG_TOOLKIT_PATH`, because the API's committed pseudo-pinned module graph is the toolkit source.
4. The isolated `rpg-local--bane` namespace gets its own API image, Redis, and sandbox. Proof may create
   it only after both its manifest and runtime state are confirmed unclaimed; it never stops, restarts,
   or wipes a primary, lab, or otherwise-owned environment. Character, Afford, Cast, React, record, and
   stream paths are exercised through that named stack.
5. An independent engine/contract review examines the exact tested heads before publication.
6. Later, providers merge inside-out. Toolkit consumers wait for CI-minted real module tags; API/web then
   adopt the real generated/tagged dependencies. Protos already use their CI-generated artifacts.

Nothing in this design authorizes an automatic merge, force-push, production/shared-stack deployment,
release, environment wipe, or destructive cleanup. Only the unclaimed named local proof stack above may be started.

---

## Rejected alternatives

- Four nominal Bard spell picks or unusable spell rows: dishonest content; one supported Bane choice is
  the settled incremental policy.
- Prepared-caster support: a different acquisition/authorization shape with no need in this proof.
- Dedicated/dual `SpellSlots`: a second authority beside payable recoverable resources.
- Migration, legacy readers, or frozen-strike conversion: preserves speculative/dead state by inventing
  facts the old shape never held.
- N casts/N prices for three targets: misreports one declaration and complicates consumers.
- Per-child duration or initiative-relative clocks: gives one spell several expiry answers.
- Subscription/source-ID sorting or roll-all/select-face overlap: uses incidental facts instead of the
  recipient's persisted application order.
- Condition-owned rolling, session rolling, or session bus ownership: hides RNG and breaks resolution's
  interaction boundary.
- Negative faces or a Bane-specific result: falsifies physical dice or duplicates `RollCalculation`.
- Application IDs: no supported concurrent same-source generation requires one.
- Toolkit/UI dice-style contract: contributor identity and authoritative facts are sufficient for this
  slice; presentation configuration has a separate owner and lifecycle.

---

## Authority and evidence boundary

The settled rulings are published durably in
[rpg-project PR #409 comment 5594028277](https://github.com/KirkDiggler/rpg-project/pull/409#issuecomment-5594028277)
and the latest acquisition/development clarification in
[comment 5594499984](https://github.com/KirkDiggler/rpg-project/pull/409#issuecomment-5594499984).
They supersede the previous open choices about slot migration, expiry, recipient carrier, acquisition,
local provider publication, and UI dice-set lookup.

The R&D issues and receipts remain evidence about current seams only. They do not claim Bane is shipped
or its production acceptance passes.

— cross-team agent, on behalf of KirkDiggler
