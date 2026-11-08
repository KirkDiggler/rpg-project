# 2026-11-07 — When is the shared Raging cut actually adopted?

Written during the authorized closeout of [Composable conditions #703](https://github.com/KirkDiggler/rpg-project/issues/703), slices S9–S12.

## Question

After the operator's normal-behavior report, the independent round, and the [native damage-sustain correction](2026-11-07-when-native-damage-disagrees-with-the-fixture.md), what evidence turns a working development graph into released adoption?

## Evidence

The operator authorized the inside-out sequence. Each provider merged after a CI-green head verdict; each consumer then adopted the actual generated tag and repeated its gates. The observed releases are mechanics/effects v0.4.0, D&D v0.214.0, resolution v0.74.3 and session v0.170.0. The item example remains intentionally untagged but consumes those released providers.

[API #1193](https://github.com/KirkDiggler/rpg-api/pull/1193) merged to dev at 4b4b081b with no toolkit development pseudo-pins. [Web #1398](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1398) merged to dev at adc9f641. Parallel movement/performance work kept landing during the sequence; catch-up merges preserved it rather than rolling dependencies back. Final consumers retain encounter v0.184.0 and published proto v0.1.260, whose generated commit is d83fdfa5.

The [release record](https://github.com/KirkDiggler/rpg-toolkit/blob/main/docs/ideas/rulebook-effect-composition/implementation.md#released-adoption) carries every PR, merge commit and module tag. The [QA receipt](https://github.com/KirkDiggler/rpg-toolkit/blob/main/docs/ideas/rulebook-effect-composition/qa.md) still distinguishes real two-client/native/castle observations from the positive incoming-damage test-backed branch. Publishing tags did not turn an unobserved live hit into evidence.

## Interpretation

The item example and the real native class caller now converge on a released rulebook creator. That is stronger than another class-shaped fixture, and narrower than a full condition migration or native item inventory. Grants, resource pools, origin, by-ref identity and active-condition lifetime remain separate concerns.

The release also preserved the path that found the mistake: old manually published damage fixtures could pass while the native notification differed. The correction and its focused closure remain part of the explanation, not something removed from a polished success story.

## Next

[S9–S12 delivery records](https://github.com/KirkDiggler/rpg-project/issues/703) carry closure and exact-path cleanup. This journal PR stays open as requested. Broader condition families, designer work, the eventual field note and recording work remain separate; no production promotion or operator-stack rebuild is implied by merging API/web dev.

— cross-team agent, on behalf of KirkDiggler
