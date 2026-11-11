# When does a dependency pin also select a castle?

Written 2026-11-11. [Authorized merge-out and stop receipt](https://github.com/KirkDiggler/rpg-project/issues/706#issuecomment-6809478590).

## Question

Can the verified maintenance wave finish by changing dependency versions alone when another wave has advanced the shared encounter provider?

## Evidence

Effects v0.5.0, rulebook v0.215.0, resolution v0.75.1 and session v0.185.0 were merged and tagged after their gates. The item examples adopted those providers too. Merge-on-top preserved the separately shipped recognition and captain/vouch work, including encounter v0.198.0.

The final API pin probe changed only `go.mod` and `go.sum`. `make test` failed `TestCastleCopiesAgree` at `internal/dungeons/castle_copies_test.go:41`: the test reads the castle from the pinned encounter module and compares it with the shipped API copy. The provider contains the captain/vouch changes; API dev does not yet. Open [API #1227](https://github.com/KirkDiggler/rpg-api/pull/1227) owns that adoption under [Gate S5b #929](https://github.com/KirkDiggler/rpg-project/issues/929).

We stopped before API/web merge, retained the failed pin patch and log, restored the probe files and posted the dependency on both PRs. No castle was silently copied, test weakened or provider downgraded. The operator's running environment was not changed.

A separate release mistake cost time: the resolution wait guessed a minor tag, while its unchanged PR title correctly produced v0.75.1. The tagger had succeeded in 14 seconds. Looking up tags by their merged commit, rather than a predicted number, recovered the actual release without retitling or hand-tagging.

## Interpretation

A provider pin selects more than a Go interface when the consumer promises to ship the same authored content its provider tests. The castle-copy check caught that additional contract. This is a cross-wave adoption dependency, not evidence that maintenance needs different ownership or a compatibility branch.

The right response is coordination at the owning PR, not absorbing its gameplay content into an unrelated release because the file is easy to copy.

## Next

After #1227 lands under its own gates or its owner coordinates the handoff, merge dev on top, retain the castle copy, repeat the released-pin/API gates and finish the already-authorized wave. [S3 #706](https://github.com/KirkDiggler/rpg-project/issues/706) owns that state. This journal remains open.
