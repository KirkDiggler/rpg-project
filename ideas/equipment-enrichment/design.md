# Equipment Data Enrichment

**Date:** 2026-03-21 (implementation record updated 2026-08-04)
**Status:** Implementation complete
**Scope:** Authoritative, enriched equipment-category choices in character creation

## Problem resolved

Character creation originally showed equipment names while the web independently
reconstructed which items belonged to a category such as “choose a martial
weapon.” That left two problems:

1. Players could not compare the meaningful weapon and armor facts in the
   selection control.
2. The web had rules-shaped category-membership logic that could drift from the
   toolkit validator, especially around full simple/martial sets and exclusions.

The delivered design makes the toolkit the sole authority for both enumeration
and acceptance, carries its concrete enriched options through the wire, and
renders those options directly.

## Delivered boundary design

```
Toolkit eligibility + enriched concrete options
  -> additive protobuf options
  -> API maps options 1:1; retains legacy category metadata
  -> web rich-card rendering of authoritative options
```

### Toolkit: resolve and validate in one authority

The toolkit resolves every `EquipmentCategoryChoice` to deterministic,
duplicate-free concrete `EquipmentItem` options with quantity `1` and resolved
detail. `choices.EligibleEquipment` is the authoritative ordered
category-resolution path for requirement expansion, category validation, and
nested draft application.

This is deliberately not a client or API lookup. It retains the full registry
semantics and existing exclusions:

- A broad **simple** category means the simple melee **and** simple ranged
  registries; a broad **martial** category likewise means martial melee **and**
  martial ranged registries. Narrow categories such as `simple-melee` retain
  their named scope.
- Monk starting equipment remains **shortsword OR any simple weapon**. The
  “any simple weapon” branch includes simple melee and simple ranged weapons.
  This is a starting-equipment rule and is distinct from Martial Arts guidance;
  neither should be used to reinterpret the other.
- Special weapons such as `unarmed-strike` remain available where their own
  rules allow them, but are not advertised as equipment-category options and
  cannot be accepted through that category-selection path.

The initial resolver implementation is [rpg-toolkit#877](https://github.com/KirkDiggler/rpg-toolkit/pull/877).
The follow-up closed the integrity gap in which nested `Draft.SetClass` checked
only registry existence and could persist an unadvertised selection:
[rpg-toolkit#879](https://github.com/KirkDiggler/rpg-toolkit/pull/879). It
requires the resolved category set before recording a nested selection and
rejects invalid legacy persisted category data during finalization.

### Protobuf: additive concrete options

[rpg-api-protos#207](https://github.com/KirkDiggler/rpg-api-protos/pull/207)
adds this wire-compatible field to the existing category-choice message:

```protobuf
repeated EquipmentItem options = 6;
```

The field reuses `EquipmentItem` and its existing enriched equipment detail.
Existing consumers remain wire-compatible; once mapped by the API, consumers
that read `options` receive the concrete eligible list without another
eligibility lookup.

### API: map authoritative options 1:1

[rpg-api#764](https://github.com/KirkDiggler/rpg-api/pull/764) maps the
ordered toolkit-resolved `EquipmentCategoryChoice.Options` straight to the
proto `options` field using the existing concrete-item mapping. It still
maps/derives legacy weapon-category metadata for backward compatibility, but
never uses that metadata to reconstruct, filter, or sort authoritative concrete
options; toolkit retains eligibility authority. Its real RPC coverage compares
full Fighter martial and Monk simple option lists, and exercises persisted valid
and invalid selections through the creation path.

### Web: rich cards and one option source

The presentation seam was first delivered in
[rpg-dnd5e-web#670](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/670): an
accessible production category dropdown renders compact rich equipment cards
from live equipment data, including weapon damage/properties/range and armor
AC/category facts.

The final integration in
[rpg-dnd5e-web#692](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/692)
renders authoritative `EquipmentCategoryChoice.options` through that rich
dropdown. It preserves API order and selection IDs, multiple slots, duplicate
selections, submission, and reopened-draft hydration. The category-to-type
reconstruction and its `ListEquipmentByType` fetch were deleted from this
route: no client eligibility inference, fallback option source, or Monk
exception remains.

## Verification evidence

| Concern | Merged evidence |
| --- | --- |
| Rich production dropdown cards | [rpg-dnd5e-web#670](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/670) |
| Toolkit resolved options | [rpg-toolkit#877](https://github.com/KirkDiggler/rpg-toolkit/pull/877) |
| Expansion/validation parity and persisted-data guard | [rpg-toolkit#879](https://github.com/KirkDiggler/rpg-toolkit/pull/879) |
| Additive proto contract | [rpg-api-protos#207](https://github.com/KirkDiggler/rpg-api-protos/pull/207) |
| API pass-through mapping | [rpg-api#764](https://github.com/KirkDiggler/rpg-api/pull/764) |
| Web direct authoritative consumption | [rpg-dnd5e-web#692](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/692) |

The merged PR evidence records focused and full checks at each layer: toolkit
race/lint and category suites; proto format/lint/breaking/generation/compile
checks; API converter, real Redis creation-path, race, build, vet, and test
checks; and web component coverage plus `npm run ci-check`.

## Compatibility and scope boundary

The proto change is additive and uses the next unused field tag. The completed
web cutover has a single source for category options, so the UI cannot disagree
with a retained lookup path. This wave adds no new equipment items and makes no
inventory or combat UI change.

## Historical lessons retained

The pre-contract live-data implementation was useful: it proved the rich-card
interaction against production data without inventing a fixture-only surface.
It was not sufficient as the final category-choice design because display data
does not answer which concrete items are valid. The persisted `unarmed-strike`
case found during integration reinforced the same lesson: enumeration,
validation, and persistence must share the toolkit authority rather than merely
checking whether an ID exists in a registry.
