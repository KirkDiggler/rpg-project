# Run — 2026-11-10 — Courtyard ground and texture merge-out

## Shipped

- Merged all ten implementation PRs inside-out under KirkDiggler's explicit authorization: private Assets#342/#344, toolkit#2289/#2290/#2330/#2333, API#1190/#1209 and Web#1395/#1401; exact hashes and release table are retained on the slice — [release record](https://github.com/KirkDiggler/rpg-project/issues/895).
- CI minted encounter v0.186.1 then v0.187.0 and session v0.173.1 then v0.174.0; final API dev2253b33e pins the latter pair, while Web dev3be3c439 pins merged material6345db54 and ground877de99a. Current base's actual published proto versions were preserved, not downgraded — [release record](https://github.com/KirkDiggler/rpg-project/issues/895).
- Closed ground S2 under the operator's accepted scoped verification direction. S3 remains open for the canonical wall composition fold requiring the operator's tree-containing YAML; no deployment or invented placement — [ground closure](https://github.com/KirkDiggler/rpg-project/issues/874).

## Decided

- Retained normal source/hash/seal checks and accepted exactly the documented one-pixel brick refresh because the operator explicitly chose that source-derived resource, rather than substituting installed bytes to conceal historical numerical drift — [ruling](https://github.com/KirkDiggler/rpg-project/issues/895#issuecomment-6731598565).
- Kept incoming approved CI/test/diagnostic baselines and used merge commits, regenerated fixtures and released tags because the wave must compose with other work, not freeze or overwrite it. API's strict up-to-date rejection was satisfied by another clean dev merge and fresh gates, not bypassed — [merge authority/order](https://github.com/KirkDiggler/rpg-project/issues/895#issuecomment-6746673256).
- Retained the shared isolated walk environment/worktrees because S3 composition work still needs its handoff; cleanup of the shared API state could disrupt that work. Operator3030/8115 and Lab3180 stay untouched — [remaining scope](https://github.com/KirkDiggler/rpg-project/issues/895).

## Found

- The encounter root timeout cleared after adopting upstream small-keep/CI-budget changes; later current-main integration used the approved heavy/rest/subpackage shards. Actual gates, not a local pass substituted for red CI, authorized release — [encounter release](https://github.com/KirkDiggler/rpg-toolkit/pull/2330).
- Web CI's initial no-job failure was a GitHub internal-server error. One unchanged retry exposed a real toast assertion: a passive initial reset could overwrite dismissal. A deterministic layout-before-passive-effects regression was RED, then GREEN with ending-keyed state; focused independent review and final CI passed — [toast bug](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1403).
- Final Web CI at9809f6ce passed10,197 tests/six skipped,790 files/one skipped; merged3be3c439 has exactly that gated tree. Actual source/runtime checks establish the final material harness equals reviewed/played d030e2a bytes; normal native/play proof is reused without another art approval or new performance claim — [final head verdict](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1401#pullrequestreview-5834724474).

## Mistaken

- A merge message claimed the encounter tree was unchanged from22701ae0 before interpreting the diff, but33 separately approved upstream behavior-record files had changed. The correction is explicit; final identity claims use tree-hash equality or `git diff --exit-code`, not a nonempty stat command whose exit code is still zero — [correction](https://github.com/KirkDiggler/rpg-toolkit/pull/2330#issuecomment-6753056012).
- Initial merge checks found no checks on conflicted API heads, and an attempted API merge was rejected after dev advanced despite previously green head checks. Refreshed bases, dependencies and current-head reviews/gates resolved those conditions; no protection override was used — [release record](https://github.com/KirkDiggler/rpg-project/issues/895).
