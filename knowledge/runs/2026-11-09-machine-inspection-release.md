# Run — 2026-11-09 — machine inspection release and CI stop

## Shipped

- Released machine v0.2.0 at c695af6c and behavior v0.22.0 at08e37981, with actual CI tags, unchanged module-tree comparisons and the consumer's release pin/gates; closed complete S1#899, not the whole inspection story — [release receipt](https://github.com/KirkDiggler/rpg-project/issues/899#issuecomment-6732397206).

## Decided

- Continue the documented inside-out implementation release after the operator read the issue/path and asked to continue; production deployment and broader optimization remain separate — [execution brief](https://github.com/KirkDiggler/rpg-project/issues/898#issuecomment-6732268474).
- Pause at encounter's red CI gate and allow one unchanged-head retry plus an exact local coverage-enabled control; no repeated retry-until-green, timeout increase, test removal or merge bypass — [bounded investigation](https://github.com/KirkDiggler/rpg-toolkit/pull/2326#issuecomment-6732567343).
- Keep the existing CI-budget issue as the owner rather than file a duplicate or silently adopt its sharding proposal; Go's timeout already applies per package, so the root package still needs measured headroom — [budget finding](https://github.com/KirkDiggler/rpg-toolkit/issues/2236#issuecomment-6732741282).

## Found

- Encounter93fdaedf changes only release manifests; full nine-package race/lint pass, but both CI42563030102 attempts time out at600s while a geometry assertion test has run only6–7s. No assertion or race failure is shown; the underlying CI/local speed difference is not proven — [failure evidence](https://github.com/KirkDiggler/rpg-toolkit/pull/2326#issuecomment-6732741028).
- Local root execution is171.341s under race alone and481.304s with uncached race plus atomic coverage. Earlier green CI root was505.202s; completed neighboring packages were also slower on the failing runner. Coverage materially changes the verification workload — [comparison](https://github.com/KirkDiggler/rpg-toolkit/issues/2236#issuecomment-6732741282).
- S2 remains open and encounter is draft; session/API/web are unchanged and unmerged. Resume the authorized release only after the current-head CI gate is resolved — [S2 checkpoint](https://github.com/KirkDiggler/rpg-project/issues/900#issuecomment-6732741544).

## Mistaken

- Treating the first timeout as possibly transient justified one controlled retry, not a diagnosis; the retry failed at the same suite boundary. The release remains blocked rather than interpreting local success as permission to bypass CI — [bounded result](https://github.com/KirkDiggler/rpg-toolkit/pull/2326#issuecomment-6732741028).
