# Item/effects investigation — starting with the Second Wind ring

Working journal for [Keep equipment S1 — #862](https://github.com/KirkDiggler/rpg-project/issues/862)
and [S2 — Rage and encounter-independent loot #873](https://github.com/KirkDiggler/rpg-project/issues/873),
under [#850](https://github.com/KirkDiggler/rpg-project/issues/850). The original folder name is retained so existing links stay valid.

- [ADR-0048: accepted composition/ownership decision; item APIs remain experimental](https://github.com/KirkDiggler/rpg-toolkit/blob/main/docs/adr/0048-equipment-granted-abilities.md).
- [Existing effects provider story — #703](https://github.com/KirkDiggler/rpg-project/issues/703).
- [Overall keep experience — #702](https://github.com/KirkDiggler/rpg-project/issues/702).

This journal records the evidence behind the ring's composition. The issue owns
delivery state; the ADR owns the accepted composition/ownership boundaries. The separate
mind/machine investigation is not a prerequisite or a shared-package decision.

## Entries

- [2026-11-04 — What does a ring actually need to grant?](2026-11-04-what-the-ring-grants.md)
- [2026-11-04 — The ring supplies the Second Wind level](2026-11-04-ring-supplies-the-level.md)
- [2026-11-04 — The item composes usage, its pool and reusable effects](2026-11-04-item-composes-usage-and-effects.md)
- [2026-11-04 — Normal recovery is configuration, not a different effect](2026-11-04-normal-recovery-as-configuration.md)
- [2026-11-04 — The first item consumer exposes the grant seam](2026-11-04-consumer-probe-exposes-the-seams.md)
- [2026-11-04 — The consumer makes the construction API hard to read](2026-11-04-consumer-feedback-cleans-the-provider-api.md)
- [2026-11-04 — Let the package supply the constructor's noun](2026-11-04-let-the-package-name-the-constructor.md)
- [2026-11-05 — Composition layers matter more than provisional constructor names](2026-11-05-composition-layers-and-provisional-names.md)
- [2026-11-05 — From a test probe to an item composer](2026-11-05-from-probe-to-item-composer.md)
- [2026-11-05 — Accepting the ownership decision, not freezing the example](2026-11-05-accepting-the-ownership-decision.md)
- [2026-11-05 — Rage, retrieved loot, and an items-first extraction path](2026-11-05-rage-loot-and-items-first-extraction.md)
- [2026-11-06 — From the walkthrough to a concrete extraction plan](2026-11-06-from-walkthrough-to-extraction-plan.md)
- [2026-11-06 — What does Compose name, and where does it belong?](2026-11-06-what-compose-names.md)
- [2026-11-07 — When is the extraction actually adopted?](2026-11-07-from-pushed-code-to-released-adoption.md)
- [2026-11-07 — Which saved data does the next Rage implementation need?](2026-11-07-choosing-the-current-rage-data.md)
- [2026-11-07 — Does shared creation survive the native readers?](2026-11-07-following-raging-through-native-readers.md)
- [2026-11-07 — What did the damage fixture fail to exercise?](2026-11-07-when-native-damage-disagrees-with-the-fixture.md)
- [2026-11-07 — When is the shared Raging cut actually adopted?](2026-11-07-releasing-the-shared-raging-cut.md)
- [2026-11-08 — What if concentration is one of the effects?](2026-11-08-when-upkeep-is-an-effect-too.md)
- [2026-11-08 — What can still happen while an effect ends?](2026-11-08-what-can-still-happen-while-an-effect-ends.md)
