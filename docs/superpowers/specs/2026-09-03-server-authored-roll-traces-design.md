# Server-Authored Roll Traces Across D&D 5e Results

**Status:** Approved by the human director on 2026-09-03<br>
**Issue:** [rpg-project#361](https://github.com/KirkDiggler/rpg-project/issues/361)<br>
**Follow-up to:** [rpg-project#342](https://github.com/KirkDiggler/rpg-project/issues/342)

## Summary

D&D 5e keeps one chain and one durable result shape per domain: attacks,
damage, healing, saves, checks, initiative, and other rule outcomes remain
semantically distinct. They share small server-authored primitives for dice
provenance and sourced additive modifiers.

The first delivery proves that boundary end to end with two existing mechanics:

1. Great Weapon Fighting damage preserves the original faces, ordered rerolls,
   final faces, dice notation, and modifier sources already produced by the
   toolkit damage chain.
2. Second Wind produces a healing-chain calculation carrying its `1d10` face
   and Fighter-level modifier. That same calculation reaches the post-clamp
   healing result without being reduced and reconstructed along the way.

Story and Debug render these typed facts. The web does not recognize feature
refs, infer dice from totals, or implement D&D arithmetic.

## Problem

The merged activation-result path can currently report:

```text
Second Wind rolled 6 + 1 = 7; 2 applied (8 → 10 HP).
```

That line does not identify the die or the modifier. Because a Fighter's Hit Die
is also a d10, a player can reasonably mistake the Second Wind roll for spending
a Hit Die. Second Wind actually rolls its own `1d10 + Fighter level` and spends
only its Second Wind use.

This is not only a healing omission. The toolkit's root `DamageChainEvent`
already retains substantially richer damage facts:

- declared dice notation;
- original die faces;
- final die faces;
- ordered rerolls with die index, before, after, and reason;
- source-attributed flat components and multipliers.

Great Weapon Fighting already writes its rerolls into that chain. The current
Session recording adapter deliberately keeps only dice notation, final rolls,
flat bonus, damage type, and multiplier. `OriginalDiceRolls` and `Rerolls` are
lost before durable Story. The older v1 encounter proto also demonstrates the
former contract: its `DamageComponent` carried original rolls, final rolls, and
`RerollEvent` history.

Second Wind takes a different thin path. It rolls a dice pool, collapses the
result to `Roll` and `Modifier`, publishes `HealingReceivedEvent`, and the HP
owner republishes those scalar fields with clamp facts in
`HealingAppliedEvent`. Adding more healing-only scalar fields would preserve
that divergence rather than restore a consistent system.

## Goals

1. Define reusable D&D 5e dice-trace and sourced-modifier primitives.
2. Keep domain-specific chains and durable result bodies.
3. Preserve provider-authored roll facts through toolkit, encounter, Session,
   proto, API, live stream, catch-up, Story, and Debug.
4. Make future domain adoption mechanical and local: emit the shared
   primitives, carry them on the domain result, map them field-for-field, and
   use the shared formatter.
5. Preserve every modifier that participated, including a real zero modifier,
   with provider-authored identity or label.
6. Carry enough ordered data for a later visual layer to settle original dice
   and then flip each rerolled die, without implementing that animation here.
7. Validate new traces before durable append instead of repairing or inferring
   missing facts downstream.

## Non-goals

- One universal `RollResult` or one universal chain for attacks, healing,
  damage, saves, and checks.
- Migrating every existing roll site in this delivery. Saves, checks,
  initiative, Hit Dice, and future spell rolls adopt the primitives
  incrementally.
- Rendering damage or healing dice in 3D.
- Replaying live animation during catch-up.
- Spending Hit Dice when Second Wind is used.
- Moving D&D rules, ref interpretation, modifier lookup, or arithmetic into
  rpg-api or the web.
- Changing combat-log audience policy.
- Reworking damage resistance, vulnerability, immunity, critical, advantage,
  or disadvantage rules. Their domain results remain authoritative.

## Decision: generic roll provenance inside domain-specific results

The complete meaning of a result is not generic:

- an attack compares a d20 total with AC and decides hit, miss, and critical;
- damage applies typed components and multipliers;
- healing distinguishes requested healing from post-clamp applied healing;
- a saving throw compares a total with a DC;
- Hit Dice additionally spend a resource.

Those results and their chains remain separate. What they share is how dice and
additive modifiers are described.

### Shared dice trace

A dice trace describes one homogeneous physical group as rolled. Exact Go and
proto names may follow repository conventions, but the contract contains:

- **notation** — the physical dice expression represented by the group;
- **original faces** — ordered values before rerolls;
- **ordered rerolls**, each containing:
  - a stable die index;
  - the value before the reroll;
  - the value after the reroll;
  - provider-authored source ref and display name/label;
- **final faces** — the value at every stable die index after all rerolls;
- **kept indices** — which final faces participate after advantage,
  disadvantage, or another keep/drop rule; empty means every final face is
  kept;
- **authoritative subtotal** — the sum of the kept final faces as asserted by
  the provider.

The trace describes actual physical dice, not only a rule's printed base pool.
For example, a critical can retain its domain's printed damage expression while
its trace describes the doubled dice actually rolled. A d20 rolled with
advantage can describe two physical d20 faces and identify the kept index while
the attack result separately names the advantage sources.

### Shared sourced additive modifier

A modifier contains:

- a signed amount, including zero when that modifier participated;
- a provider-authored source ref when the source has a catalog identity;
- a provider-authored source display name;
- an optional provider-authored label when the relevant term is more specific
  than the source name, such as `Fighter level`.

Absence means the modifier did not participate. Zero does not mean absence.
The web never maps a ref to a rule or invents a label.

### Domain-owned calculation and total

A domain result arranges ordered dice traces and modifiers and supplies its
authoritative total. The common primitives are not an executable arithmetic
AST. Domain-specific transformations stay with the domain:

- damage type, critical state, and multipliers remain damage facts;
- AC and hit/miss remain attack facts;
- DC and success/failure remain save/check facts;
- requested/applied healing and HP before/after remain healing facts.

This keeps the reusable contract small while preserving every fact needed for
consistent arithmetic presentation.

## Domain flow

### Great Weapon Fighting damage

The root damage chain is authoritative. A representative result is:

```text
Greatsword damage
  2d6 original [1, 5]
  Great Weapon Fighting rerolled die 0: 1 → 4
  final [4, 5], subtotal 9
  +3 Strength
  total 12 slashing damage
```

The Great Weapon Fighting provider replaces its untyped
`great_weapon_fighting` reason with the canonical condition ref
`dnd5e:conditions:fighting_style_great_weapon_fighting` and display name
`Great Weapon Fighting`. Session does not parse that ref or reason.

Resolution preserves the root chain's components. The Session-to-encounter
adapter clones the full dice trace rather than selecting only final rolls. The
encounter composition validates and persists a neutral primitive mirror without
importing the D&D root module, consistent with its current dependency boundary.
Session projection, proto, API, and web preserve the same order and values.

Damage remains a domain composition. Multipliers such as resistance or immunity
are not recast as additive modifiers.

### Second Wind healing

Second Wind must no longer be a special reduced roll path. Its resolved healing
chain/result contains:

```text
source: dnd5e:features:second_wind / Second Wind
  1d10 original/final [6], subtotal 6
  +1 Fighter level
  requested total 7
```

The Fighter-level modifier is authored by the feature and carries the canonical
Fighter source identity plus the label `Fighter level`. Neither API nor web
learns that rule.

The calculation travels on the healing request/received event. The character or
monster that owns HP applies the clamp and publishes `HealingAppliedEvent` with
an immutable copy of that calculation plus:

- requested healing;
- applied healing;
- HP before;
- HP after;
- target and healing source identity.

The activation collector copies this resolved healing result without flattening
or rebuilding it. Encounter records the activation and ordered result as it
does today. Session persists and projects it.

The existing deterministic-roller gap in
[rpg-toolkit#1427](https://github.com/KirkDiggler/rpg-toolkit/issues/1427)
becomes part of the toolkit delivery: Second Wind must consume the interaction's
scoped roller rather than process-global randomness so exact trace tests do not
replace `crypto/rand.Reader`.

## Transport and presentation

### Proto

Define reusable additive messages for:

- dice trace;
- reroll trace;
- source identity;
- sourced additive modifier.

Domain messages reference these shared messages. Damage and healing retain
separate bodies. Existing enum and oneof assignments remain unchanged; new
fields use the next free tags and pass the repository's canonical breaking
check.

### API

rpg-api maps every field and ordered repeated value directly. It does not:

- sum faces or modifiers;
- decide which values were kept;
- identify Great Weapon Fighting or Second Wind;
- infer labels from refs;
- reconstruct missing traces from aggregate totals.

### Web

One shared formatter renders the generic dice and modifier primitives. Domain
formatters provide only domain wording and facts around it.

Representative Story details are:

```text
Greatsword rolled 2d6 [1 → 4, 5] + 3 Strength = 12 slashing damage.
Second Wind rolled 1d10 [6] + 1 Fighter level = 7; 2 applied (8 → 10 HP).
```

Exact punctuation can be refined during web implementation, but all provider
facts must remain visible. Debug prints every trace field losslessly, including
indices, original/final arrays, reroll sources, modifier sources, subtotals, and
domain totals.

All nested trace fields participate in duplicate/conflict identity so two
same-sequence events that differ only in a reroll or modifier cannot be treated
as identical.

## Validation and compatibility

New traces are validated before durable append. Validation includes:

- non-empty, valid dice notation;
- legal face values for the die size;
- matching original/final cardinality;
- stable in-range reroll indices;
- each ordered reroll's `before` matching the value current at that index;
- each reroll's `after` becoming the next current value;
- unique, in-range kept indices;
- subtotal matching the kept final faces;
- domain total matching its supplied traces and additive modifiers where that
  domain declares an additive calculation;
- required provider labels and canonical source refs where a catalog source
  exists.

Validation is structural and arithmetic, not a second implementation of D&D
eligibility. It can prove that `1 → 4` and the total agree; it does not decide
whether Great Weapon Fighting was allowed to reroll that die.

Proto additions are backward compatible. Old persisted events with no trace
continue to render their current aggregate form. New events after cutover must
not silently degrade to inferred aggregate-only detail. Strict known-body JSON
decoding continues to reject duplicate keys and forbidden nulls.

Live and catch-up use the same persisted event and formatter. Catch-up does not
re-roll, recalculate, or replay animation.

## Migration path

The reusable contract is proven narrowly before broader adoption:

1. **Toolkit root primitives and roller seam**
   - Introduce shared trace/modifier types and validation/helpers.
   - Route the scoped interaction roller into feature activation, resolving
     rpg-toolkit#1427.
2. **Damage preservation**
   - Upgrade Great Weapon Fighting's reroll source identity.
   - Preserve root damage traces through resolution, encounter, and Session.
3. **Healing chain**
   - Add the domain-specific healing calculation.
   - Have Second Wind publish it once and HP owners append clamp facts.
   - Preserve it through activation capture, encounter, and Session.
4. **Proto**
   - Add reusable trace messages and additive fields to domain bodies.
5. **API**
   - Pin published toolkit/proto versions and map field-for-field.
6. **Web**
   - Add the shared trace formatter and domain Story/Debug composition.
7. **Isolated live acceptance**
   - Run the full toolkit → API → web stack locally without changing the shared
     primary during experimentation.

Saves, checks, initiative, Hit Dice, and other roll domains migrate later using
the same primitives. If a domain cannot adopt them with a small adapter, that
mismatch is evidence to refine the shared primitive before proliferating
another parallel representation.

## Acceptance criteria

### Great Weapon Fighting

- A deterministic greatsword example begins with `[1, 5]`, records an ordered
  `1 → 4` reroll sourced to Great Weapon Fighting, and ends with `[4, 5]`.
- Dice notation, Strength or other participating additive modifiers, damage
  type, and authoritative total survive every boundary.
- Story and Debug expose the original, reroll, final, source, and arithmetic
  facts.

### Second Wind

- A deterministic example carries `1d10 [6] + 1 Fighter level = 7` from the
  healing calculation.
- At HP 8/10 it records requested 7, applied 2, and HP 8 → 10.
- It spends Second Wind, not Hit Dice.
- No consumer recognizes the Second Wind ref to construct those facts.

### Cross-cutting

- Live delivery and reconnect/GetStory produce equivalent entries with no
  duplicates.
- Old aggregate-only persisted events remain readable.
- Malformed new traces fail before append and do not partially persist.
- Mutation/conflict tests cover every new nested trace field.
- Focused module tests, proto breaking checks, API integration, web CI, and an
  isolated local full-stack proof pass.
- No damage/healing 3D dice behavior is added in this delivery.

## Rejected alternatives

### One universal roll/result event

Rejected because domain outcomes are not semantically interchangeable. It would
move domain rules into consumers or grow into a generic arithmetic language.

### Universal arithmetic AST

Rejected as unnecessary. Modeling every addition, multiplication, clamp,
keep/drop operation, and future rule as executable nodes would make migration
harder, not easier.

### Independent damage and healing enrichments

Rejected because it would restore GWF fields and add Second Wind fields while
leaving two parallel, incompatible descriptions of the same dice provenance.
That is the inconsistency this work exists to remove.

### Web reconstruction

Rejected because totals do not uniquely determine dice, rerolls, kept values,
or modifier sources. It would also make live and catch-up behavior dependent on
client rule knowledge.
