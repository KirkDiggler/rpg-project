# Who owns a half-delivered cast?

Written: 2026-11-09. [Composable maintenance](https://github.com/KirkDiggler/rpg-project/issues/706); [R11 decision](https://github.com/KirkDiggler/rpg-toolkit/pull/2323#issuecomment-6752508135).

## Question

Can one composed application remain honest when casting pauses halfway through its initial recipients? The approved scope includes existing native continuation behavior, not only a cast that finishes in one call. The earlier [application-identity investigation](2026-11-08-which-application-did-that-removal-end.md) separated generations from addresses; identity alone does not say when membership becomes durable.

## Evidence

- Root `8d37e5fd` replaced the combined cast declaration with maintenance parts after `94aac8d9` retired the old concentration runtime. All ten current concentration profiles preserve durations/grace. Full root race/vet/lint passed, and the import seam remained acyclic. Final comment cleanup `4a73a227` passed those gates again. [Root PR](https://github.com/KirkDiggler/rpg-toolkit/pull/2329).
- Real linked creation, delivery and reload now prove a non-spell source granting another creature physical resistance, with and without concentration. Ending that ward leaves an independent Raging instance and its resistance intact. The proof also exposed arbitrary content being written into `SpellRef`; end facts now carry generic source identity. These are toolkit tests, not browser or native item acceptance. [Proof](https://github.com/KirkDiggler/rpg-toolkit/blob/8d37e5fd2acd3815d0b71550950c2289d5aabdec/rulebooks/dnd5e/conditions/maintenance_composition_test.go).
- Native resolution `01841a3e` characterizes a two-target Bane cast against the existing released runtime: target zero fails and its returned dirty monster sheet carries Bane; target one pauses to decide about Resistance. The cast has no completed reported outcome yet, but the earlier board change is already durable. Focused test and full resolution race/vet/lint pass. [Characterization PR](https://github.com/KirkDiggler/rpg-toolkit/pull/2344).
- The new binding refuses an unsealed recipient at a successful save boundary. The approved cut also excludes adding members after sealing. A stable application ID in the frozen continuation cannot make that intermediate state valid. [Boundary check](https://github.com/KirkDiggler/rpg-toolkit/blob/8d37e5fd2acd3815d0b71550950c2289d5aabdec/rulebooks/dnd5e/conditions/application_delivery.go#L93-L118).

## Interpretation

The checked plan missed a lifecycle state at the provider/consumer join. Local tests correctly prove sealed applications; the native cast correctly preserves effects delivered before a pause. Neither fact proves they compose.

A resumable initial-delivery phase could keep the paid coordinator and actual deliveries valid through the pause, then seal permanently when the original cast finishes. That is the recommendation, not an approved decision. It would also make the new concentration claim explicit during the paid pause rather than installing it only after the final target. The alternative is to delay shared effects until every target has answered, changing the observed timing of existing effects. Skipping validation or restoring the old runtime is not an alternative within the agreement.

The earlier docs-only merge-ready verdict therefore no longer describes the known state of the design. R1–R10 otherwise remain settled; the affected implementation is paused for R11 rather than silently weakened.

## Next

Resolve [R11](https://github.com/KirkDiggler/rpg-toolkit/pull/2323#issuecomment-6752508135), then amend only the affected lifecycle/data/continuation contracts and execute their tests. Native integration, early look and QA remain unrun. No merge or stack startup occurred. This investigation and journal PR #863 remain open.
