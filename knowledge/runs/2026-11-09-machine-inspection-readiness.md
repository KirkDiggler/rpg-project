# Run — 2026-11-09 — machine inspection readiness

processed: 2026-11-12T165555Z

## Shipped

- Published the narrowly authorized proto#426 as official v0.1.263, adopted its generated bindings in API/web, and completed exact-wire QA and retained-reviewer closure; implementation PRs remain unmerged pending authorized inside-out release/re-pins — [final readiness](https://github.com/KirkDiggler/rpg-project/issues/898#issuecomment-6731477380).

## Decided

- Make the walkthrough the architecture entry point and the story the single current status, because the operator asked to understand flow and direction rather than follow a chain of gates/commits — [#898](https://github.com/KirkDiggler/rpg-project/issues/898).
- Treat the operator's subsequent clearance as the requested narrow proto publication authorization, not permission to merge the whole implementation wave; broader optimization remains separate — [publication and remaining authority](https://github.com/KirkDiggler/rpg-project/issues/898#issuecomment-6731477380).
- Reuse the same independent reviewer and upstream closures rather than restart whole reviews after catch-up or a bounded scalar fix — [round verdict](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1400#pullrequestreview-5820582677).
- Keep actual Exit events visible in the QA record and stop the affected script rather than restoring departed actors to manufacture a complete pass — [QA checkpoint](https://github.com/KirkDiggler/rpg-project/issues/900#issuecomment-6730010080).

## Found

- Official proto v0.1.263 resolves generated commit cb53a2b66f6a85f4bbf0131040fa4c804ab6b891. API's nested gen/go module uses that published commit's Go pseudo-version; web uses the official tag. This differs from unreleased toolkit development pins — [provenance and review](https://github.com/KirkDiggler/rpg-api/pull/1207#pullrequestreview-5821167418).
- Two real browser clients retained9007199254740993 as bigint, matched all41live/read records including raw bytes and reproduced them after reconnect;40published capture records matched retained payloads, excluding the nonpublished constructor seed. The actual captured inspector renders the same value — [QA receipt](https://github.com/KirkDiggler/rpg-project/issues/898#issuecomment-6731477380).
- Joined S12a/inspection session and current-content API pass their actual gates; the other session's unmerged Redis adapter is not silently included — [checkpoint](https://github.com/KirkDiggler/rpg-project/issues/900#issuecomment-6730010080).
- Review found missing scalar data rendered as null. The actual captured fixture now reports unavailable/incomplete evidence, with0/false/empty/bigint controls and full local/current-head CI green — [W1 closure](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1400#pullrequestreview-5820721432).
- Fresh owned actors completed the previously interrupted castle checks: cones, crossing, one-way exit and a fresh closed-gate fight. Actual checked-dialogue failure was retained rather than rerolled; successful checked timing remains a deterministic control. Setup polling of the deliberately abandoned earlier run is distinguished from successful fight actions — [qualified QA](https://github.com/KirkDiggler/rpg-project/issues/898#issuecomment-6731477380).
- API c149f7a7 and web f71e6258 pass full local/current-head remote gates; web's actual full transcript has10005passed/6skipped. The same reviewer closes W1/R9 with no new Critical/Important finding — [web closure](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1400#pullrequestreview-5821167594).

## Mistaken

- Appending superseding checkpoints left older contradictory slice/status prose in the story, and the walkthrough omitted the raw-JSON transport explanation. The status body is reconciled and the walkthrough now separates existing JSON payloads, protobuf transport and lossless decoding — [#898](https://github.com/KirkDiggler/rpg-project/issues/898).
- The toolkit hook's cache hit was not valid joined-tree evidence: its relative-path bug hashes zero files. An uncached run actually passed; the existing issue, not another duplicate, holds the mechanism — [toolkit#1736](https://github.com/KirkDiggler/rpg-toolkit/issues/1736#issuecomment-6729481384).
- The QA driver initially encountered no-such-member as an unexplained stop; stored records and server logs identify a real Exit RPC, not state loss from diagnostics. The source of that call is not guessed — [checkpoint](https://github.com/KirkDiggler/rpg-project/issues/900#issuecomment-6730010080).
