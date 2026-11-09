# Run — 2026-11-09 — machine inspection readiness

## Shipped

- Prepared the raw-JSON proto provider, reconciled upstream session/content changes, completed the independent implementation round and fixed its single Important finding; no implementation PR or proto is merged — [#898](https://github.com/KirkDiggler/rpg-project/issues/898).

## Decided

- Make the walkthrough the architecture entry point and the story the single current status, because the operator asked to understand flow and direction rather than follow a chain of gates/commits — [#898](https://github.com/KirkDiggler/rpg-project/issues/898).
- Treat the operator's merge-readiness direction as approval to finish raw JSON and gates, not permission to merge the whole wave; generated proto publication requires its own concrete action — [publication gate](https://github.com/KirkDiggler/rpg-api-protos/pull/426#issuecomment-6729481164).
- Reuse the same independent reviewer and upstream closures rather than restart whole reviews after catch-up or a bounded scalar fix — [round verdict](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1400#pullrequestreview-5820582677).
- Keep actual Exit events visible in the QA record and stop the affected script rather than restoring departed actors to manufacture a complete pass — [QA checkpoint](https://github.com/KirkDiggler/rpg-project/issues/900#issuecomment-6730010080).

## Found

- The additive raw record field has independent MERGE-READY and exact-head CI green; API/web consumption waits for CI's post-merge generated bindings — [proto review](https://github.com/KirkDiggler/rpg-api-protos/pull/426#pullrequestreview-5820445957).
- Joined S12a/inspection session and current-content API pass their actual gates; the other session's unmerged Redis adapter is not silently included — [checkpoint](https://github.com/KirkDiggler/rpg-project/issues/900#issuecomment-6730010080).
- Review found missing scalar data rendered as null. The actual captured fixture now reports unavailable/incomplete evidence, with0/false/empty/bigint controls and full local/current-head CI green — [W1 closure](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1400#pullrequestreview-5820721432).
- Two-client castle checks established neutral entry, posted versus looping guards, named sneak refusal, free greeting, held-door refusal and voice distraction; remaining crossing/cones/fight checks are not claimed — [QA evidence](https://github.com/KirkDiggler/rpg-project/issues/900#issuecomment-6730010080).

## Mistaken

- Appending superseding checkpoints left older contradictory slice/status prose in the story, and the walkthrough omitted the raw-JSON transport explanation. The status body is reconciled and the walkthrough now separates existing JSON payloads, protobuf transport and lossless decoding — [#898](https://github.com/KirkDiggler/rpg-project/issues/898).
- The toolkit hook's cache hit was not valid joined-tree evidence: its relative-path bug hashes zero files. An uncached run actually passed; the existing issue, not another duplicate, holds the mechanism — [toolkit#1736](https://github.com/KirkDiggler/rpg-toolkit/issues/1736#issuecomment-6729481384).
- The QA driver initially encountered no-such-member as an unexplained stop; stored records and server logs identify a real Exit RPC, not state loss from diagnostics. The source of that call is not guessed — [checkpoint](https://github.com/KirkDiggler/rpg-project/issues/900#issuecomment-6730010080).
