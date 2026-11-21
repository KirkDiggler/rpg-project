# Run — 2026-11-12 — Courtyard S5 release

Delivery [Project #919](https://github.com/KirkDiggler/rpg-project/issues/919), broader follow-up [Web #1402](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1402).

## Shipped

- All five authorized implementation PRs merged: Assets34532d11dc, encounter23561fedb460/v0.204.0, session2357d7069af9/v0.189.0, API1226212b38f2 and Web1404a09b555e on dev; final API preserves newer compatible session v0.191.0, Web TSv0.1.270 and actual merged material pin — [release table](../../ideas/studio-appearance/brief.md#released-chain).
- Shared manifest-driven Appearance controls and existing normal-bearing wall finishes through captured Save & Play/reload, with both operator looks accepted and one independent round closed — [slice evidence](https://github.com/KirkDiggler/rpg-project/issues/919#issuecomment-6807507497).

## Decided

- Paused at real downstream castle-copy/recognition failures until upstream API adoption landed, rather than waive checks or absorb another gameplay migration; existing merge authority remained valid when resuming — [release hold](https://github.com/KirkDiggler/rpg-project/issues/919#issuecomment-6831134940).
- Reused accepted visual evidence for unchanged appearance code after mechanical base/pin refreshes; no second art/general-castle round — [operator acceptance](https://github.com/KirkDiggler/rpg-project/issues/919#issuecomment-6808054615).
- Archived the actual screenshots/JSON/drivers in private Assets before cleanup; no licensed imagery enters public Web/Project. Operator deployment and the separate Project#915 merge remain outside the five-PR authorization — [handoff](../../ideas/studio-appearance/brief.md#close-out-and-authority).

## Found

- F1: full Atlas omitted face finishes while AtlasFor retained them. The corrected constructor and two-reader alias regression pass; one fixture now reports eight already-authored face facts without other changes — [closure](https://github.com/KirkDiggler/rpg-toolkit/pull/2356#pullrequestreview-5866243712).
- Final Web local gate passes10,406tests/five skipped; exact-head CI passes and merged tree equals the gated head. API full local/test/build gates pass at released pins — [Web release closure](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1404#pullrequestreview-5883271746).
- Session tag lookup briefly lagged at the Go proxy; direct resolution verified v0.189.0 against actual merged d7069af9. No hand-written version, replacement or gate bypass was used — [release checkpoint](https://github.com/KirkDiggler/rpg-project/issues/919#issuecomment-6831134940).

## Mistaken

- The earlier full-authoring omission was missed by player-only tests; the independent review supplied the counterexample and the fix now covers both readers. The adjacent older MaterialOverrides omission is tracked separately, not silently bundled — [follow-up](https://github.com/KirkDiggler/rpg-toolkit/issues/2382).
