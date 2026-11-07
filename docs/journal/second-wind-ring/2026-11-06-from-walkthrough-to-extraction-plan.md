# From the walkthrough to a concrete extraction plan

Written: 2026-11-06. Work: [effects extraction #705](https://github.com/KirkDiggler/rpg-project/issues/705), informed by item slices #862 and #873.

## Question

What is the first bounded implementation cut after agreeing on the layers and their paper trail?

## Evidence

The operator cleared the documentation for merge. Toolkit [#2288](https://github.com/KirkDiggler/rpg-toolkit/pull/2288) merged at `aca78cdf`; project [#878](https://github.com/KirkDiggler/rpg-project/pull/878) merged at `07516c5`. Head-specific documentation verdicts and checks are recorded on the PRs. No item/example PR was included in that merge permission.

The [first-cut extraction plan](https://github.com/KirkDiggler/rpg-toolkit/blob/22a6acefd054fa8ef5d8797882b59076b41b8d47/docs/ideas/item-effect-composition/plan.md), in [toolkit#2291](https://github.com/KirkDiggler/rpg-toolkit/pull/2291), inventories the existing generic provider, both item consumers, import seams, live documentation, CI module discovery and first-tag behavior. It names exact tasks/tests and separates development pins from released adoption.

The earlier record deliberately leaves the module home until extraction. The proposed home is top-level `effects/`, keeping `effects/machine` in the same release unit. That recommendation is H1 on #705, not an operator ruling silently supplied by this session. No source directory or consumer import has moved.

## Interpretation

The first cut can promote the proven generic runtime without bundling rulebook vocabulary relocation, production condition factories, legacy condition migration, editor descriptors or storage APIs. Those need their own consumer contracts rather than becoming hidden prerequisites.

A plan can be concrete and structurally checked while still blocked on a named architectural choice. It is not execution-ready until H1 is settled. Once that choice is recorded, the faithful plan does not need another procedural approval cycle.

## Next

Resolve H1, then execute the provider move and real consumer re-pins against pushed commits. Observe CI's actual release tag only after an authorized provider merge. Write the implementation record from those results, not from this plan's expectations.
