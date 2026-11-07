# 2026-11-07 — Does shared creation survive the native readers?

Written 2026-11-07, during [Composable conditions #703](https://github.com/KirkDiggler/rpg-project/issues/703) / [native adoption S12 #884](https://github.com/KirkDiggler/rpg-project/issues/884).

## Question

After [choosing the current saved contract](2026-11-07-choosing-the-current-rage-data.md), Kirk approved the complete Raging-only design and proceeding. Can one creator serve both the native class feature and item example without losing progress, meaning or provenance above the rulebook?

## Evidence

The source implementations are published separately: [neutral runtime #2297](https://github.com/KirkDiggler/rpg-toolkit/pull/2297), [shared rulebook creator #2299](https://github.com/KirkDiggler/rpg-toolkit/pull/2299) and [item consumer #2300](https://github.com/KirkDiggler/rpg-toolkit/pull/2300). The handwritten native Rage condition and private item recipe/vocabulary were removed; both callers now use the registry. These remain development PRs, not released adoption.

Native consumers revealed a value the initial wire assumption missed. The rulebook emitted a source entity ID, but session and protobuf EffectRow did not carry it. [Session #2306](https://github.com/KirkDiggler/rpg-toolkit/pull/2306) adds the direct projection; [proto #422](https://github.com/KirkDiggler/rpg-api-protos/pull/422) adds source_id. Its contract/generation checks pass, but publication still needs operator authorization. An opaque row ID was not repurposed to repair the loss.

The existing character converter also produced no source for the new origin shape and omitted the registry description. [API #1193](https://github.com/KirkDiggler/rpg-api/pull/1193) maps those metadata values and preserves the saved bytes. Its new test crosses real handlers and Redis-backed storage: class activation spends one charge, the first boundary stores progressed state, and an information read does not renew grace. The action carries four distinct rows, only one of which applies to its outgoing damage.

[Web #1398](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1398) no longer requires cached damage_bonus/level. The badge reads saved progress and server text, keeping bindings opaque. Ten focused assertions failed before the reader change; all 52 focused cases passed afterward. Full API and web gates also completed. The [implementation checkpoint](https://github.com/KirkDiggler/rpg-toolkit/blob/design/rulebook-effect-composition/docs/ideas/rulebook-effect-composition/implementation.md) preserves measurements and the earlier timed-out attempts rather than counting them as passes.

No isolated stack has been booted, no browser walk or independent runtime review has happened, and no release tag adoption is claimed. The operator's stack and characters were untouched.

## Interpretation

Sharing the constructor was necessary but not sufficient. The consumers tested whether the agreed facts survived each boundary. Four effect rows did not mean four attack bonuses; an origin was not a stacking address; a binding was not a client-side formula. Those distinctions had to survive ordinary persistence and rendering, not just the rulebook's tests.

The missing wire field changed the execution dependency, not the agreed design or the decision to refuse old Rage saves. This is now a publication gate: generated SDKs must exist before the API and client can carry the source value. It is not another request to approve the architecture.

## Next

With authorization, publish the additive proto contract and adopt its generated SDKs, then boot the exact isolated stack for the early native look. [S12](https://github.com/KirkDiggler/rpg-project/issues/884) owns that work and the later walk/review/release evidence. The current question is still whether the real journey agrees with the source-level proof.

— cross-team agent, on behalf of KirkDiggler
