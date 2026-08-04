# Idea: Equipment Data Enrichment

Equipment category choices now arrive at character creation as authoritative,
fully detailed options, so players can compare meaningful equipment facts without
the web reimplementing rules eligibility.

## Milestone

4class-dungeon

## Status

**Implementation complete — 2026-08-04.** All six delivery PRs are merged. The
web's final category-choice path consumes toolkit-resolved options directly; it
no longer reconstructs category membership or calls `ListEquipmentByType` for
this route. This records completion of the implementation wave, not a claim
about a separate release or deployment.

## Delivered architecture

```
Toolkit resolves and validates eligible, enriched concrete options
  -> additive proto options field
  -> API maps those options without rules logic
  -> web renders those options as rich cards
```

- **Toolkit is authoritative.** It resolves deterministic, duplicate-free
  `EquipmentCategoryChoice` options from the same eligibility path used for
  validation. A broad **simple** or **martial** category includes both its
  melee and ranged registry members; a specifically narrow category remains
  narrow.
- **Validation protects persistence.** Nested class/draft selection is checked
  against the resolved category set before it is recorded, and legacy invalid
  persisted category selections are rejected at finalization.
- **Proto is additive.** `EquipmentCategoryChoice.options` is a repeated
  `EquipmentItem` field at tag `6`, reusing the existing enriched item shape.
- **API is a translator, not a rulebook.** It preserves the toolkit-resolved
  ordered options and does not infer category membership or eligibility.
- **Web has one source of category options.** It renders the API-provided
  `options` with the accessible rich dropdown/card and has no category-to-type
  reconstruction, fallback option source, or `ListEquipmentByType` fetch on
  this route.

### Monk rule clarity

Monk starting equipment remains **shortsword OR any simple weapon**. “Any
simple weapon” includes both simple melee and simple ranged weapons. This
starting-equipment choice is distinct from Martial Arts guidance; neither is a
shortcut for the other.

## Merged evidence

| Delivered concern | Evidence |
| --- | --- |
| Rich cards in the production category dropdown using live equipment data | [rpg-dnd5e-web#670](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/670) |
| Authoritative toolkit category options | [rpg-toolkit#877](https://github.com/KirkDiggler/rpg-toolkit/pull/877) |
| Validator parity and persisted-selection protection | [rpg-toolkit#879](https://github.com/KirkDiggler/rpg-toolkit/pull/879) |
| Additive `EquipmentCategoryChoice.options` contract | [rpg-api-protos#207](https://github.com/KirkDiggler/rpg-api-protos/pull/207) |
| Thin API mapping of resolved options | [rpg-api#764](https://github.com/KirkDiggler/rpg-api/pull/764) |
| Web sole-source adoption of authoritative options | [rpg-dnd5e-web#692](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/692) |

## Lasting lessons

- Rich display data alone does not establish rules eligibility. The earlier
  live-data card work proved the presentation seam; the completed contract
  removes the duplicated eligibility decision.
- Enumeration and acceptance must use the same toolkit authority. Otherwise a
  value can be present in a draft even though it was never advertised to the
  player.
- Keep the API and web thin: a client-side eligibility reconstruction can drift
  as registries, exclusions, and class rules change.

## Repositories

rpg-toolkit, rpg-api-protos, rpg-api, rpg-dnd5e-web
