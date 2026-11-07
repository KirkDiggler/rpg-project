# Run — 2026-11-06 — Courtyard S2 plan

Slice [#874](https://github.com/KirkDiggler/rpg-project/issues/874), story [#867](https://github.com/KirkDiggler/rpg-project/issues/867).

## Shipped

- Prepared the region-ground law, walkthrough, overview and checked implementation plan; this documentation does not claim shipped ground authoring — [S2](https://github.com/KirkDiggler/rpg-project/issues/874).

## Decided

- KirkDiggler agreed to per-region stone-default bases, explicit paint surviving base changes, erase-to-base, level ground and hex boundaries; S2 execution is authorized — [story agreements](https://github.com/KirkDiggler/rpg-project/issues/867).
- Use the existing typed region projection, not the retired full-source scene path; preserve separate provider module PRs and the proto publication gate — [S2 contract](https://github.com/KirkDiggler/rpg-project/issues/874).

- After confirming the broad Assets Python suite is not a CI test job, the operator directed current courtyard work to retain the baseline notes, verify new work and investigate useful validation separately; no tests are removed and no merge/deployment authority is inferred — [operator disposition](https://github.com/KirkDiggler/rpg-project/issues/874#issuecomment-6712580255).

## Found

- Existing Kingdom ground color images contain faceted variation; floor-resource candidates can use those images without weakening generic prop volume/grounding checks — [S2](https://github.com/KirkDiggler/rpg-project/issues/874).
- Web baseline region, draft and native document tests passed 104/104; partial region merge and owner edits need explicit paint handling — [S2 plan](https://github.com/KirkDiggler/rpg-project/blob/design/867-courtyard-ground/ideas/courtyard-ground/plan.md).

- Generated contracts now feed a playable isolated stack: native save/play, recipient-specific discovery, actor/NPC movement, reload and source immutability were exercised; final castle acceptance and released-provider integration remain open — [joined checkpoint](https://github.com/KirkDiggler/rpg-project/issues/874#issuecomment-6701542844).
- Matched 431-cell hardware probes reduced maximum draw calls 369→71 but kept p95 at 16.8 ms and increased startup-inclusive p99 40.7→65.1 ms. An unused cached-source upload was removed after measurement and a failing renderer-counter regression; no blanket performance win is claimed — [measurement](https://github.com/KirkDiggler/rpg-project/issues/874#issuecomment-6701542844).
- The full Assets gate completed with 21 failures and 15 errors; all 36 identities reproduce on unchanged base, matching the existing blocker rather than authorizing a waiver — [Assets #333](https://github.com/KirkDiggler/rpg-game-assets/issues/333#issuecomment-6701160384).

- Independent reviews are clear for all current implementation heads. Assets source-custody checking had one Important defect, reproduced and fixed with a real CLI regression, then independently closed; the shared baseline gate remains unwaived — [review closure and gate](https://github.com/KirkDiggler/rpg-project/issues/874#issuecomment-6712221345).

- GitHub lists only dynamic Copilot review for Assets, with no checked-in workflows or PR test checks. Fresh nine-test ground/source/inventory/stage checks pass; measuring validation value and change triggers is a separate follow-up rather than an invented permanent policy — [Assets #343](https://github.com/KirkDiggler/rpg-game-assets/issues/343).

## Mistaken

- Review handoffs named earlier failed encounter/API logs instead of the final receipts. Exact-head CI and the actual final local transcripts resolved the evidence; subsequent handoffs distinguish current, failed and baseline receipts rather than infer a pass from a filename — [corrected API evidence](https://github.com/KirkDiggler/rpg-api/pull/1190#pullrequestreview-5814253327).

- Initial inspection treated the dirt models' Y extent as slab thickness; triangle inspection showed an uneven upward surface, and the accepted visual proof separated level geometry from faceted appearance — [ground proof](https://github.com/KirkDiggler/rpg-project/issues/870#issuecomment-6672515656).
