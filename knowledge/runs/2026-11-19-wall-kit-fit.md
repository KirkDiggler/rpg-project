# Run — 2026-11-19 — Consistent wall-kit fitting

## Shipped

- Recorded the agreed height/finish brief and checked provider/consumer task plan; implementation is not complete — [Courtyard S6](https://github.com/KirkDiggler/rpg-project/issues/1022).

- The first real Studio checkpoint uses existing Castle Wall 01 and Single Doorway01 at heights 3.75 and 6, with matching selected gray charts, unchanged leaves and working refusal/edit/Undo; the wider inventory remains in progress — [Assets draft #348](https://github.com/KirkDiggler/rpg-game-assets/pull/348), [Web draft #1465](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1465).

- Prepared nine additional existing castle refs with fixed features, repeat bands and single top caps; provider `05cfd6a`. Actual :3191 Studio renders for Wall04 and Window03 measure 3.75/6 with unchanged fixed details and matching facade charts. Accepted :3190 is unchanged; generic section placement, remaining inventory, Original/default charts and native/play integration remain open — [slice](https://github.com/KirkDiggler/rpg-project/issues/1022).

## Decided

- Reuse declared fixed/repeatable parts, existing source custody and shared charts instead of introducing another per-model fitting path or source registry — [agreement](https://github.com/KirkDiggler/rpg-project/issues/1022).
- Make equal physical height and matching finishes the first checkpoint of the broader wall-section goal, not a claim that general one-off decorative/window placement is already delivered — [scope](https://github.com/KirkDiggler/rpg-project/issues/1022).

- Preserve shaped upper caps once instead of repeating them. Repeat only an explicitly selected constant-profile band with continuous backing; source partitioning preserves geometry and UVs. A recessed facade need not fill every nominal depth plane — [plan](../../ideas/wall-kit-fit/plan.md).

## Found

- The old Double Door has an `above` part, Single Doorway01 does not, and structural parents scale their children outside the prop's upper-row mechanism; the generic wall selector also admits every valid non-leaf catalog entry — [source-grounded plan](../../ideas/wall-kit-fit/plan.md).

- Existing wall/door refs can retain their identity and default wall maps while moving into the authored configuration path; body fitting and automatic corner opt-in remain separate — [source and integration checkpoint](https://github.com/KirkDiggler/rpg-project/issues/1022).

- Focused provider checks: 74 passed; Blender partition checks: 5 passed. Additional corner checks had 25 passes and one unchanged Fit01 archived-byte mismatch reproduced with prior scripts/test/fixture; it is retained as a failure, not called passing. No broad Assets campaign — [private evidence](https://github.com/KirkDiggler/rpg-game-assets/tree/feat/wall-kit-fit/evidence/1022-wall-kit/feature-batch).

## Mistaken

- Treating the existing wall picker as an already-curated kit would conceal unconfigured assets; the plan requires an explicit asset/source/result inventory rather than silently limiting the pass to Fit01/Single01 — [plan](../../ideas/wall-kit-fit/plan.md).
