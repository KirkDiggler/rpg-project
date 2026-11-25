# Finishing adoption with the content owner

Written 2026-11-12. [Composable conditions S3 #706](https://github.com/KirkDiggler/rpg-project/issues/706).

## Question

Could the [castle-copy release stop](2026-11-11-when-a-provider-pin-also-selects-content.md) be resolved without downgrading the provider, weakening parity or absorbing another wave's content?

## Evidence

[API #1227](https://github.com/KirkDiggler/rpg-api/pull/1227) merged its canonical captain/vouch castle at `eba2475a`. We merged dev on top of the maintenance branch; only dependency files conflicted. The same released graph that had failed now passed full API tests and ci-check, including `TestCastleCopiesAgree`. Final-head remote test/build gates passed before [API #1225](https://github.com/KirkDiggler/rpg-api/pull/1225) merged at `c6183e14`.

Web preserved the independently shipped studio line-grant/topic-wake work by merging dev on top. Its complete local gate passed 799 test files and 10,325 tests, with six existing skips; remote lint/typecheck, tests, audit and preview passed before [web #1410](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1410) merged at `e00205d8`.

The API now commits effects v0.5.0, rulebook v0.215.0, resolution v0.75.1 and session v0.185.0, preserving encounter v0.198.0 and the current generated SDK. These merges target dev, not production main. The earlier failed probe remains recorded rather than being rewritten into a clean first attempt.

## Interpretation

The parity guard and ownership boundary worked together. The failure named the missing adoption; the owning PR supplied it; the unchanged gate then proved the integrated graph. No new runtime mechanism or compatibility path was required.

## Next

The [release table](https://github.com/KirkDiggler/rpg-toolkit/blob/main/docs/ideas/composable-maintenance/implementation.md#authorized-release-progress) and S3's ship receipt own completion. Keep this journal open. Preserve the live operator stack and the branch references its older development pins still need; archiving receipts and removing finished unserved worktrees must not become an accidental run teardown. Recording/video remains unstarted.
