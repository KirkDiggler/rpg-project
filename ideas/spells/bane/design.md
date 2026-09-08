# Bane — one paid declaration, three saves, one visible d4

**Date:** 2026-09-08
**Status:** Proposed design, revised after bounded R&D and Kirk's oldest-source ruling.
**Umbrella:** `ideas/spells/` — levels 1–3, and the choices at 3 made real.
**Related journey:** [rpg-project#243](https://github.com/KirkDiggler/rpg-project/issues/243).
**Research:** [rpg-toolkit#1593](https://github.com/KirkDiggler/rpg-toolkit/issues/1593) and [#1595](https://github.com/KirkDiggler/rpg-toolkit/issues/1595).

This is a design, not an implementation report or plan; names are candidate contracts unless called existing.

---

## The player promise

A bard chooses Bane, selects one to three creatures within 30 feet, and casts once. One spell slot
and one action are charged once. Each target makes its own Charisma save in the order selected.
Targets that fail become baned under one concentration hold on the bard; targets that save do not.
If all save, the cast is still paid and the bard still holds a zero-child concentration owner.

Later, an affected creature's attack or saving throw visibly includes the selected Bane d4:

```text
the goblin attacks the bard       d20 14 + 4 - d4 3 (Bane, bard A) = 15 · misses
the goblin makes a DEX save       d20 12 + 2 - d4 1 (Bane, bard A) = 13 · fails
```

The UI receives the selected contributor's entity ID and the resolved dice facts. It looks up that
player's dice set for presentation. Dice styles or skins are not rule facts, and Bane adds no manual
roll, confirmation, or second player interaction.

The clean responsibility chain is:

```text
D&D rulebook compiles a concrete price and cast profile
  -> the existing action door preflights and pays once
  -> the cast fans out through the existing per-target contest
  -> one caster-side concentration owner holds every delivered child
  -> eligible conditions describe unsigned dice contributions
  -> the attack/save roller evaluates those descriptions
  -> generic RollCalculation retains faces, sign, content, and source entity
  -> records and wire project the same facts
  -> the UI presents them
```

API and UI do not re-derive class progression, slot counts, Bane arithmetic, or overlap selection.
Dice presentation policy belongs to the UI, not the rulebook.

---

## Rules and slice boundary

This slice chooses **Bane rather than Thunderwave**. Bane (2014) is a 1st-level enchantment, range 30
feet, concentration up to 1 minute: up to three creatures make Charisma saves, and each failure
subtracts 1d4 from its attack rolls and saving throws for the duration.

Bane combines one paid declaration, per-target save gates, one-to-many ownership, and an automatic
contributed die without also requiring an area or movement system.

This slice deliberately does **not** add:

- Bless content; Bless is a later customer of the same additive contribution mechanism.
- Upcasting or cast-level selection.
- Area templates, line-of-sight rules, half damage on a save, or forced movement.
- A modifier to ability checks; Bane affects attacks and saving throws only.
- A dice-expression language, signed dice notation, or mixed fixed/dice term union.
- Dice skins, UI-selected rollers, or a confirmation step for an automatic d4.
- A speculative multiclass, Pact Magic, Arcane Recovery, or general spell-recovery rewrite.

Kirk previously chose Bane over Thunderwave. Kirk's latest overlap ruling is also settled here:
for overlapping equal-potency Bane applications, the **oldest active source contributes**. It
supersedes the earlier research recommendation of newest.

---

## Current truth versus this proposal

The bounded R&D reviewed exact toolkit head
`b3f899c51f4adfa3eecb75d2c518bb92bb756be1`. At that head, these are shipped facts:

- `combat.SpendProfile` and the existing door check/debit an action plus keyed resource pools once;
  `Character.Resources` participates in that door, persistence, dirty marking, and reset.
- Dedicated `SpellSlots` persists and **already resets on LongRest**, but cannot pay through the
  current character ledger.
- Cast ingress iterates `KnownCantrips`, adds an action-only price in session, and resolves one target.
  `KnownSpells` persists but is not current cast-offer ingress.
- Attack/save chains have fixed lanes but no unresolved described-dice lane. Generic
  `RollComponent`/`RollCalculation` keeps positive dice facts but always adds the dice subtotal.
- The version 1 Inspiration freeze stores scalar arithmetic, not a contributed-die trace.
- Concentration child addresses are recipient plus condition ref. Ref-wide removal can remove both
  same-ref applications and make another owner forget its child.
- Recipient condition JSON preserves application order; subscription order and source-ID sort do not.
- One concentration owner counts its caster's turn ends, not every creature's turn.

The disposable R&D exposed these seams and tested candidate composition. It did **not** ship Bane,
fan-out, qualified keepers, described-dice production fields, attack/save calculation projection, or
levelled spell ingress. Its passing tests are not passing Bane acceptance. Everything below is proposed.

---

## 1. Spell knowledge, slot authority, and one payment

### Rulebook-owned compilation

The D&D rulebook owns whether Bane is known/prepared and castable, the class progression and slot
seeding, and Bane's level, target profile, save gate, duration, and concrete price. That price uses the
existing `combat.SpendProfile`:

```text
Slots[standard action] = 1
Pools[SpellSlotResourceKey(1)] = 1
```

`SpellSlotResourceKey(level)` is a candidate typed constructor in the existing resource-key vocabulary,
not a new currency. `combat.Pay` sees only an action cost and typed pool key. The game server/session
projects the compiled definition and targets; it carries no class table, slot maximum, authorization
rule, or ref-to-level inference.

### Levelled-spell ingress

Bane enters through a rulebook-owned levelled-spell authorization list: a bard's known levelled
spells, not `KnownCantrips`; a prepared caster's prepared levelled spells when supported. Both produce
concrete priced cast definitions, so session need not know the class authorization model. Existing
cantrip ingress remains action-only and Bane is never relabeled as a cantrip.

### One mutable slot authority

Canonical slot state is one recoverable resource per spell level. `SpellSlots` and `Resources` must not
both mutate the same capacity; payment, post-payment persistence, and recovery use resources only.
Dedicated slots already reset on LongRest—the gap is payment reachability and dual representation, not
basic recovery. Legacy records still need the migration/load choice under Remaining decisions, but
either choice converges on one writer and authority.

### Payment boundary and failure claim

Before charging, validate every knowable target-list error: count, empty/duplicate IDs, interaction
membership, target eligibility, and existing 30-foot reach for each target. Offer construction limits
choices; execution revalidates stale declarations. The complete list and every child contest preflight
before the door.

The door checks the whole profile and either refuses without any debit or pays once. Refusal preserves
action, level-1 resource, old owner/children, and dice stream. This design does **not** claim rollback
after an unforeseen runtime, event-publication, repository, or persistence failure; its narrower claim
is that known declaration errors preflight and ordinary inability to pay mutates nothing.

---

## 2. One cast fans out through existing contests

A cast declaration carries an ordered target list. Singular casts use a list of one rather than a
parallel singular spelling.

```text
CastTargetOutcome { TargetID, Save, Applied }
CastOutcome       { Spell, CasterID, Targets []CastTargetOutcome, FollowUps }
```

For Bane the target rule accepts one to three distinct creatures in range. The machine preserves the
caster's target order. It requests the existing save-then-effect contest once per target, collecting
one save and one applied-effect list per target. A successful save has an empty applied list; a failed
save delivers one Baned condition.

This produces:

- one player declaration and one cast record/beat naming the ordered target list;
- one payment for the declaration;
- one per-target result list, including successful saves rather than silently dropping them;
- one concentration owner on the caster;
- zero to three source-qualified child effects under that owner.

If every target saves, the paid cast still creates the proposed zero-child owner. That is a deliberate
Bane rule proposal retained from the original design, not evidence of current shipped behavior.

For a new concentration cast, ordering is fixed:

```text
whole-list preflight -> payment -> synchronously drop old owner and children
                     -> ordered target contests and delivery -> install new owner
```

Old concentration is not dropped during preflight and is not dropped when payment is refused. It is
dropped after successful payment and before any new target receives Bane. Every newly delivered child
carries the source-qualified address described below.

A post-payment failure is reported honestly; no compensating refund or resurrection of old
concentration is promised without separate transactional evidence.

---

## 3. One duration clock, with an honest expiry boundary

Bane creates one `ConcentratingCondition` on the caster. That owner contains all child addresses and
is the only duration clock. A target that saved has no child. A suppressed overlapping Bane remains a
normal child of its own caster's owner.

“Up to 1 minute” means **10 rounds**, not ten arbitrary turns while walking the roster. Other
creatures' turn ends do not decrement the Bane owner. A child never starts or resets its own clock.

Current concentration decrements at the caster's turn end, including the casting turn's end. The exact
Bane boundary is therefore not automatic: decide whether that end consumes tick one or is graced to
the corresponding boundary ten rounds later. This design neither silently changes the shared clock
nor asserts `TurnEnds: 10` is necessarily 60 seconds; Kirk's remaining ruling appears below.

Breaking concentration, dropping it, reaching the chosen expiry boundary, or otherwise ending the
owner removes all of that owner's source-qualified children together.

---

## 4. Conditions describe dice; rolling machinery evaluates them

### A separate described-dice lane

Attack and saving-throw chain events gain a collection beside their existing fixed modifier lanes:

```go
type DiceContribution struct {
    Source   RollSource // authored content ref, display name, optional role
    SourceID string     // entity whose effect contributes; the Bane caster here
    Dice     string     // one unsigned homogeneous pool, e.g. "1d4"
    Subtract bool       // false adds; true subtracts
}

DiceContributions []DiceContribution
```

This is not a fixed-or-dice union. Existing fixed attack and save fields keep their current jobs.
A Baned condition appends one description when it is selected as applicable. It receives no roller,
rolls no d4, stores no face, and cannot flatten `-3` into an anonymous scalar.

Descriptions validate completely before any roll consumes RNG: content source and source entity are
present, notation is one unsigned homogeneous positive pool, and no sign, composite term, or
expression grammar is accepted.

### One generic evaluator and result vocabulary

Attack and save rolling machinery share the same contribution evaluator. The physical pool always
contains positive faces. A narrow evolution of the existing generic result records the arithmetic
operator:

```go
type RollComponent struct {
    Source       RollSource
    SourceID     string     // selected contributing entity, separate from Source.Ref
    Dice         *DiceTrace
    Modifier     *int
    SubtractDice bool       // applies only to Dice.Subtotal; default remains additive
}
```

`SubtractDice` is a candidate name. `DiceTrace.OriginalRolls`, rerolls, final faces, kept indices, and
subtotal remain positive physical facts. Validation adds or subtracts that subtotal exactly once.
A subtractive component does not use a negative face, signed notation, or a compensating negative
modifier. Existing signed fixed modifiers remain fixed modifiers.

The result-side `RollComponent` may already carry dice and a fixed modifier from one source; the
Params-side contribution intentionally describes only dice. That asymmetry avoids redesigning fixed
lanes without a Bane requirement.

There is no `BaneDieResult`, Bane-specific proto result, or proposed `RollModifier` wire lane. The
existing generic `RollCalculation` is the authoritative explanation for both additive and subtractive
components.

### Eligibility and deterministic RNG order

D&D applicability selects contributors **before any roll**. After that selection:

1. roll one d20, or the existing two d20s for advantage/disadvantage and keep the existing winner;
2. evaluate each selected dice contribution once, in the selected persisted-application order;
3. build and validate the generic calculation total;
4. apply existing attack natural-1/natural-20 policy or ordinary save arithmetic.

Advantage changes only the d20 pool. It never creates a second Bane d4. Each eligible application
contributes at most one pool to one roll.

Natural attack miss/critical policy stays in attack settlement. Saving-throw natural 1/20 remains
informational. The generic dice evaluator owns neither rule.

### Every save path, including concentration

The same calculation travels through:

- ordinary attack outcomes and attack records;
- ordinary saving-throw results and per-target cast-save records;
- concentration saving-throw results and records;
- encounter/session mirrors and the generic wire projection used by the UI.

The existing scalar roll/total fields may remain compatibility projections, but they must equal the
validated authoritative calculation. Concentration conversion must not narrow a resolved save back
to only roll, total, DC, and success.

A creature affected by Bane therefore subtracts the selected d4 from a concentration save too. Bane
has no exception for the save made to maintain another spell.

### Inspiration freezes the settled calculation

The attack machine settles the d20 and automatic contributions before posing Inspiration. The frozen
payload retains the full calculation, including selected source ID, content ref, notation, positive
face, operator, and total. Resume validates and reuses that calculation. It does not refold conditions,
reselect a Bane source, reroll a d4, or reconstruct a face from a stale scalar. A spent Inspiration die
is appended as its own sourced component and the total is revalidated.

This proposal bumps the exact `frozenStrikeVersion` from v1 to a new version and fails closed on
already-open v1 windows. It deliberately does not translate scalar v1 windows into invented dice
facts. This is an explicit compatibility break: those windows cannot resume through the new reader.
No automatic action reopening, refund, or recovery is promised; existing sessions with such windows
need deliberate operational handling before adoption. A silent reroll or false attribution is not recovery.

---

## 5. Source-qualified application ownership

For this slice, a durable spell-owned condition address is:

```text
{ recipient entity ID, condition ref, source entity ID }
```

For Bane, `SourceID` is the caster. `SourceRef` is adjacent provenance naming the Bane spell/content.
It is not the caster, is not part of address equality, and must never be repurposed as an application
address. “What rule is this?” and “who imposed this instance?” remain separate facts.

The same qualified address is common across:

- Baned condition runtime/data and JSON;
- character and monster condition keepers;
- condition-removal facts;
- the concentrating owner's child list, matching, and JSON;
- imposed-effect/result facts used to register new children.

Qualified removals compare all three fields. Ending bard A's concentration removes bard A's Bane and
bard A's child entry only. It cannot remove bard B's condition or cause bard B's owner to forget its
child. Acceptance must use two actual owners because one-owner coverage misses the bookkeeping bug.

Existing unqualified lifecycle facts remain ref-wide for legacy conditions until deliberately
migrated. Compatibility is explicit, not a wildcard for new work: every spell-owned Bane effect and
child must have a non-empty source entity, and new qualified removal paths must not silently fall back
to `{recipient, ref}` matching.

No minted application ID is justified. The supported concentration cast path synchronously removes a
same-caster old Bane before delivering the new one, so old and new generations with the same address
do not coexist. An application ID becomes necessary only if a supported rule later permits concurrent
same-source/same-recipient/same-condition applications or genuinely deferred cleanup.

The test-only ownership model used upsert behavior for convenience; that is not shipped behavior and
is not this design's reapply contract. Same-address reapplication on this cast path means
**drop old synchronously, then deliver new**.

---

## 6. Overlap: oldest active equal-potency source contributes

Ownership answers which application exists and which owner may remove it. Selection answers which
existing application contributes to a roll. They are deliberately different operations.

For this Bane slice:

- all Bane applications have equal potency because upcasting is out of scope;
- the oldest active Bane application on the recipient contributes;
- newer Bane applications remain active, owned, persisted, and counting down on their original
  concentration clocks;
- when the oldest ends, the next-oldest surviving Bane contributes on subsequent rolls;
- no suppressed duration restarts or pauses when the winner changes;
- Bane and Bless belong to different stacking groups, so one selected Bane and one selected Bless may
  both contribute to the same eligible roll.

The age authority is the recipient's persisted condition application order. Reload must preserve it.
The selector must not use bus registration order, load-time subscription order, lexical source IDs,
content-ref sort, or the face rolled.

The selector belongs to the **D&D recipient-condition applicability layer**, which owns ordered active
applications and D&D stacking. It receives recipient ID, roll kind, and persisted-order condition views
with qualified addresses and source spell refs. Bane's rule selects the first active Bane application;
this slice needs no potency ranking or generic dice-description capability on the view. Before RNG,
it emits ordered eligible addresses; only those conditions append descriptions. That projection is
also evaluation order, so collection must materialize against it rather than event-subscriber append order.

Generic dice arithmetic never groups by `SourceRef`, chooses a winner, or rolls all Banes. The UI also
never selects. Current `MakeSavingThrow` has bus+saver ID but no ordered condition projection, so the
exact local keeper-to-roll carrier is absent. Implementation must choose an explicit recipient-local
input seam—not a global registry or subscription order. This is a boundary choice, not an open rule.

---

## 7. Records, wire, and presentation

One cast record names the caster, Bane content ref, and ordered target list. Each target entry retains
its complete Charisma contest and whether/what was applied. Saved targets remain visible. Applied
Baned effects retain their qualified source for ownership and attribution.

Every affected attack/save record carries the generic `RollCalculation`. The selected component
includes:

- Bane's content ref and authored display name;
- the selected caster's source entity ID;
- unsigned notation and positive original/final faces;
- the subtract operator;
- the checked component subtotal and calculation total.

The wire mirrors those generic facts rather than creating a Bane field. The UI uses content metadata
to label Bane, source entity ID to look up that player's dice set, and resolved dice facts to present
the actual roll. It receives only the selected contributor, never suppressed candidate sources and
never dice-style data from the toolkit.

A content ref remains useful when the source entity is absent for non-player content, but Bane's new
spell-owned contribution rejects a missing source entity rather than degrading to content-only
attribution.

---

## Acceptance scenes — future proof, not current evidence

These scenes define implementation acceptance. They are not passing today.

1. **Payment refusal:** without a level-1 resource or standard action, refusal changes neither currency, old concentration/children, nor dice stream.
2. **Invalid third target:** an absent, duplicate, ineligible, or out-of-range third target fails the whole declaration before payment or concentration drop.
3. **Mixed saves:** three valid targets pay once and roll three ordered Charisma contests; only failures receive source-qualified Baned conditions.
4. **All save:** the action and one level-1 resource are spent once, old concentration drops, and one new zero-child Bane owner remains.
5. **One payment, many results:** one three-target cast record has one debit, three ordered target results, and one owner—not three casts/prices.
6. **Two source-specific owners:** ending bard A's owner removes only A's effect/child; bard B's effect and owner child list remain.
7. **Oldest winner switches:** A contributes while oldest; after A ends, B contributes with its original remaining duration and no clock reset.
8. **Reload ordering:** two overlapping Banes retain application order and select the same oldest contributor after reload.
9. **Attack trace:** advantage/disadvantage rolls its normal d20 pool plus one selected Bane d4; calculation/wire show content, caster ID, positive face, subtract operator, and checked total.
10. **Save trace:** an ordinary save retains contributor identity and actual dice facts through result, encounter record, session projection, and wire.
11. **Concentration save:** a baned creature's concentration save subtracts one selected Bane d4 through the same generic calculation lane.
12. **Inspiration resume:** keep/spend reuses the frozen Bane calculation without reselect/reroll; spending adds only Inspiration.
13. **Bane and Bless groups:** a synthetic additive contributor may prove the mechanism without implementing Bless; it and selected Bane each contribute once.
14. **Unchanged cantrips:** one-target cantrips remain lists of one with action-only prices and existing player flow.
15. **Expiry boundary:** an explicit scene pins Kirk's ten-round convention; roster turns do not decrement the owner.

---

## Affected modules — scope, not a train

This table names the likely ownership surface. It is not sequencing, issue slicing, or an
implementation plan.

| Surface | Scope of the proposed contract |
|---|---|
| D&D character/spell rules | known/prepared levelled ingress, class slot seeding, typed level resource key, concrete price |
| D&D combat/actions | up-to-three target profile and ordered list validation |
| D&D resolution | whole-list preflight, one payment, ordered contest fan-out, freeze/reuse, generic calculation |
| D&D conditions/events | Baned description, applicability input, qualified address/removal, one owner with N children |
| D&D saves/encounter/session | contribution evaluation and full generic calculation through ordinary/concentration records |
| API protos | target/result lists and generic roll-calculation source entity/operator projection |
| rpg-api | reference/ID translation and pass-through only; no Bane or class rules |
| rpg-dnd5e-web | up-to-three picker, per-target story, resolved d4 display, source-player dice-set lookup |

---

## Rejected alternatives and trade-offs

- **Dedicated spell-slot currency:** widens/bridges the generic payer; existing resource pools already traverse the real door.
- **Dual mutable `SpellSlots` plus `Resources`:** can disagree on spend/recovery; compatibility may read old data, but only resources write.
- **A second multi-target cast type or N records:** forks consumers and misreports one declaration; canonical targets are a list.
- **Condition-owned d4 rolling:** erases dice/source facts; conditions describe and rolling machines roll.
- **A fixed-or-dice Params union:** buys mixed-category narration order without a Bane/Bless customer.
- **Bane-specific `DieResult`/`RollModifier`:** duplicates generic calculation; evolve `RollComponent` once.
- **Negative faces or subtractive face plus `-face`:** falsifies physical dice or subtracts twice.
- **Content-only attribution:** cannot identify whose dice set contributed; retain content and source entity separately.
- **Application IDs now:** add allocation/persistence without a supported same-source concurrent Bane lifecycle.
- **Newest, source-ID sort, or roll-all/select-face:** contradicts Kirk or uses incidental/non-durable facts.
- **A global effect registry:** duplicates the recipient keeper; use its local ordered projection.

---

## Remaining decisions before implementation

These are the only unresolved choices in this revision; the oldest-source and entity-attribution
rulings are closed.

1. **Legacy slot records:** choose one-way migration or one bounded release of read-only legacy load translation; only resources write/mutate.
2. **Ten-round expiry boundary:** Kirk chooses whether casting-turn end consumes tick one or is graced to the corresponding boundary ten rounds later.
3. **Recipient-order carrier:** select the explicit local keeper-to-roll input seam; shipped save entrypoints lack it, and subscription order/global registry are disallowed.

Runtime failure compensation after a successful payment is not presented as an unresolved promise:
this design makes no rollback guarantee. Stronger/unequal-potency stacking, upcasting, multiclass slot
math, Pact Magic, and Bless content are outside this level-1 Bane slice rather than hidden decisions.

---

## Research record and evidence boundary

The research trackers are on Project 19 as **Cross-team / Learn / In Review**:

- [toolkit#1593 — paid cast](https://github.com/KirkDiggler/rpg-toolkit/issues/1593) and its [reviewed receipt](https://github.com/KirkDiggler/rpg-toolkit/issues/1593#issuecomment-5588458819).
- [toolkit#1595 — contributions/ownership](https://github.com/KirkDiggler/rpg-toolkit/issues/1595) and its [reviewed receipt](https://github.com/KirkDiggler/rpg-toolkit/issues/1595#issuecomment-5593172235).
- [Kirk's final oldest-source ruling](https://github.com/KirkDiggler/rpg-project/pull/409#issuecomment-5593394554).

Those experiments used disposable test scaffolding around real shipped entrypoints. Their real evidence
covers the existing payment door, resource persistence/rest, current frozen-window information loss,
and the current same-ref ownership failure. Their models compare candidate fan-out, signed dice facts,
and source-qualified addresses. They do not constitute production Bane tests or adopt architecture by
themselves.

This design incorporates their bounded recommendation while preserving that distinction.

— cross-team agent, on behalf of KirkDiggler
