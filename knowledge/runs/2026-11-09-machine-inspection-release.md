# Run — 2026-11-09 — machine inspection release

## Shipped

- Completed the inspection implementation through encounterv0.186.0 (`b1e3f202`), sessionv0.173.0 (`e107bcde`), API dev`cc5e07b4` and web dev`94358893`, on the official proto and released toolkit graph; consumer merge trees equal the tested candidates, with no main/production deployment — [release table](https://github.com/KirkDiggler/rpg-project/issues/898#issuecomment-6752953576).
- Released machine v0.2.0 at c695af6c and behavior v0.22.0 at08e37981, with actual CI tags, unchanged module-tree comparisons and the consumer's release pin/gates; closed complete S1#899, not the whole inspection story — [release receipt](https://github.com/KirkDiggler/rpg-project/issues/899#issuecomment-6732397206).

## Decided

- Continue the documented inside-out implementation release after the operator read the issue/path and asked to continue; production deployment and broader optimization remain separate — [execution brief](https://github.com/KirkDiggler/rpg-project/issues/898#issuecomment-6732268474).
- Pause at encounter's red CI gate and allow one unchanged-head retry plus an exact local coverage-enabled control; no repeated retry-until-green, timeout increase, test removal or merge bypass — [bounded investigation](https://github.com/KirkDiggler/rpg-toolkit/pull/2326#issuecomment-6732567343).
- Keep the existing CI-budget issue as the owner rather than file a duplicate or silently adopt its sharding proposal; Go's timeout already applies per package, so the root package still needs measured headroom — [budget finding](https://github.com/KirkDiggler/rpg-toolkit/issues/2236#issuecomment-6732741282).

## Found

- Encounter93fdaedf changes only release manifests; full nine-package race/lint pass, but both CI42563030102 attempts time out at600s while a geometry assertion test has run only6–7s. No assertion or race failure is shown; the underlying CI/local speed difference is not proven — [failure evidence](https://github.com/KirkDiggler/rpg-toolkit/pull/2326#issuecomment-6732741028).
- Local root execution is171.341s under race alone and481.304s with uncached race plus atomic coverage. Earlier green CI root was505.202s; completed neighboring packages were also slower on the failing runner. Coverage materially changes the verification workload — [comparison](https://github.com/KirkDiggler/rpg-toolkit/issues/2236#issuecomment-6732741282).
- At the93fdaedf stop, S2 remained open and encounter returned to draft; session/API/web were unchanged and unmerged. That gate was not bypassed — [S2 checkpoint](https://github.com/KirkDiggler/rpg-project/issues/900#issuecomment-6732741544).
- A later operator-requested main catch-up reaches115200c9 through publishedf61e4379: complete local gates and all three real CI test shards now pass. At that checkpoint the provider was ready again; consumer release pins and recordv0.3.0-compatible checks were still pending, with the preview unchanged — [gate closure](https://github.com/KirkDiggler/rpg-project/issues/900#issuecomment-6750359071).
- The approved smaller fixtures already supersede the separate test-value cut; PR2337 is closed unmerged rather than maintained as a redundant implementation, preserving its measured audit — [supersession](https://github.com/KirkDiggler/rpg-toolkit/pull/2337#issuecomment-6749652701).

- Final fresh-run verification on the actual fenced/released graph preserves41records across both clients/reconnect/cold-host reload,40published capture payloads, exact9007199254740993 and normal same-character End/relaunch; actual capture rendering is checked — [released-graph receipt](https://github.com/KirkDiggler/rpg-project/issues/900#issuecomment-6752825030).

## Mistaken

- The old owned host's Docker stop did not produce a drain marker and ended137. A fresh external signal was explicitly refused by runc; same-user SIGTERM inside the owned container reached the application and produced exit0, continuation drain and capture-drained status. The old archive stays incomplete and the host issue is separate, not an invented application failure or a hidden pass — [game-dev#127](https://github.com/KirkDiggler/game-dev/issues/127).
- A storage diagnostic initially assumed recordv0.3.0 renamed the JSON audience key; its actual contract keeps `audience` and changes the value to a numeric reference. Reading the published type corrected the control; persisted data and application code were not altered — [qualified verification](https://github.com/KirkDiggler/rpg-project/issues/900#issuecomment-6752825030).
- Treating the first timeout as possibly transient justified one controlled retry, not a diagnosis; the retry failed at the same suite boundary. The release stopped there rather than interpreting local success as permission to bypass CI — [bounded result](https://github.com/KirkDiggler/rpg-toolkit/pull/2326#issuecomment-6732741028).
