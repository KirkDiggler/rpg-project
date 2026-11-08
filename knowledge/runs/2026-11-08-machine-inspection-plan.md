# Run — 2026-11-08 — machine inspection plan

## Shipped

- Recorded the approved symbol-plus-trace direction and three linked slices; implementation and live verification are not complete — [#898](https://github.com/KirkDiggler/rpg-project/issues/898).

## Decided

- Extend the existing table-symbol/record pattern rather than add a debug interpreter or a new RPC, because the record already carries opaque toolkit-owned facts — [#674](https://github.com/KirkDiggler/rpg-project/issues/674).
- Preserve the same evaluator with optional returned trace values, because inspecting alternatives must not introduce extra guard reads or commits — [#899](https://github.com/KirkDiggler/rpg-project/issues/899).
- Keep symbols outside the browser's500-row window and retain them in captured records, because an old outcome cannot be explained using today's edited definition — [#901](https://github.com/KirkDiggler/rpg-project/issues/901).
- Use a bounded after-save host capture with explicit start/gap status, not an exactly-once audit-store promise; database and analytics selection are deferred — [#900](https://github.com/KirkDiggler/rpg-project/issues/900).

## Found

- Four toolkit providers and API capture have local/remote gates green; the web reader has a full local gate and renders the owned live/captured preview. Nothing is merged; final QA/review wait on the operator's look and R9 — [#898](https://github.com/KirkDiggler/rpg-project/issues/898).
- Machine Step already short-circuits and returns a reason, while Composer reports only fired committed steps; a full diagnostic needs actual guard traces plus explicit proposal/commit dispositions — [#899](https://github.com/KirkDiggler/rpg-project/issues/899).
- Session retains server history unbounded; the immediate symbol-loss case is the browser's bounded visible row list, so no server storage redesign is needed for this reader lifetime — [#901](https://github.com/KirkDiggler/rpg-project/issues/901).
- AUTH_DEV_MODE already names local development as never production and can gate record access separately from server capture; normal StoryService stays unchanged — [#900](https://github.com/KirkDiggler/rpg-project/issues/900).

## Mistaken

- The initial no-proto-change assumption was too broad: the actual Struct converter rounds9007199254740993 to9007199254740992. Exact JSONL survives; the precision-dependent live codec pauses for an explicit wire ruling rather than silently weakening exactness — [R9](https://github.com/KirkDiggler/rpg-project/pull/902#issuecomment-6727648247).
- Initial path searches assumed an API providers directory and encounter retention.go; actual owners are session construction, encounter.readrecord.go/data.go, and the web feed. No source was changed from the guessed paths — [#898](https://github.com/KirkDiggler/rpg-project/issues/898).
