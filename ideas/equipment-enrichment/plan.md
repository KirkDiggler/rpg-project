---
name: Equipment Enrichment — Implementation Record
updated: 2026-08-04
status: implementation complete — all delivery PRs merged
confidence: verified by merged cross-repository evidence
---

# Equipment Enrichment — completed delivery record

This is the execution record for the equipment-category-options wave described
in `design.md`. All implementation gates below are marked complete only where
the linked PR is merged. This records merged implementation work; it does not
make a separate production-release claim.

## Completed gates

- [x] Rich equipment cards render inside the production category dropdown from
  live equipment data — [rpg-dnd5e-web#670](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/670), merged 2026-08-02.
- [x] Toolkit resolves authoritative, deterministic category options with
  enriched item detail — [rpg-toolkit#877](https://github.com/KirkDiggler/rpg-toolkit/pull/877), merged 2026-08-03.
- [x] Toolkit expansion, nested selection validation, and persisted-data
  protection share the authoritative category set — [rpg-toolkit#879](https://github.com/KirkDiggler/rpg-toolkit/pull/879), merged 2026-08-03.
- [x] The additive `repeated EquipmentItem options = 6` proto contract is
  merged — [rpg-api-protos#207](https://github.com/KirkDiggler/rpg-api-protos/pull/207), merged 2026-08-03.
- [x] API maps resolved options without eligibility logic — [rpg-api#764](https://github.com/KirkDiggler/rpg-api/pull/764), merged 2026-08-03.
- [x] Web reads authoritative options as its sole category-option source and
  deletes the category lookup/reconstruction route — [rpg-dnd5e-web#692](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/692), merged 2026-08-04.

## Delivered contract

1. **Toolkit authority:** category expansion and acceptance use the same
   `EligibleEquipment` path. A broad simple or martial category includes both
   its melee **and** ranged registries. Existing exclusions remain enforced.
2. **Monk precision:** Monk starting equipment is **shortsword OR any simple
   weapon**; “any simple weapon” includes simple melee and simple ranged
   weapons. That starting-equipment choice is separate from Martial Arts
   guidance.
3. **Contract shape:** `EquipmentCategoryChoice.options` is an additive
   repeated `EquipmentItem` field at tag `6`; each option carries the existing
   resolved equipment detail.
4. **Thin consumers:** API maps toolkit output without rules knowledge. The web
   renders those ordered options and selection IDs directly, with no category
   inference, fallback option list, or `ListEquipmentByType` fetch on this
   route.
5. **Persistence safety:** a selection must belong to the resolved category set
   before nested draft application records it; invalid legacy persisted
   category data is rejected when finalized.

## Execution and verification record

| Layer | Delivered evidence | Recorded verification |
| --- | --- | --- |
| Rich card UI | [web#670](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/670) | Production-route dropdown/card tests and `npm run ci-check` |
| Toolkit options | [toolkit#877](https://github.com/KirkDiggler/rpg-toolkit/pull/877) | D&D 5e race tests, lint, and category-option suite |
| Toolkit parity/persistence | [toolkit#879](https://github.com/KirkDiggler/rpg-toolkit/pull/879) | Red regression, race suites, full D&D 5e tests and lint |
| Proto contract | [protos#207](https://github.com/KirkDiggler/rpg-api-protos/pull/207) | `buf` format/lint/breaking/generation, Go/TS compile, repo tests |
| API mapping | [api#764](https://github.com/KirkDiggler/rpg-api/pull/764) | Converter and real Redis creation-path coverage, race, build, vet, tests |
| Sole-source web cutover | [web#692](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/692) | Direct-authority component coverage and `npm run ci-check` |

The integration sequence was toolkit options, additive proto contract, toolkit
validation hardening, API mapping, then web sole-source adoption. The rich-card
work preceded the final source cutover and supplied the reusable presentation
seam.

## Scope boundary and retained lessons

This delivery adds neither new equipment items nor inventory/combat UI. It
replaces a duplicated category-eligibility decision with a single toolkit-owned
one.

The early live-data dropdown work was worthwhile because it proved the player
presentation against real equipment data. The later `unarmed-strike` regression
showed why that alone was insufficient: registry existence is not category
eligibility. Keeping enumeration, validation, nested application, and
persistence protection on one toolkit authority prevents that drift.
