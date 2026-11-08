# 2026-11-08 — What if concentration is one of the effects?

Written during [Composable conditions S3 #706](https://github.com/KirkDiggler/rpg-project/issues/706); detailed proposal: [toolkit #2323](https://github.com/KirkDiggler/rpg-toolkit/pull/2323).

## Question

After [releasing shared Raging](2026-11-07-releasing-the-shared-raging-cut.md), would Bless/Bane simply become more recipes using the existing concentration owner? Kirk's example changed the question: give someone the resistance that Rage uses, but only while someone else concentrates. Concentration must itself be a reusable effect alongside the payload, not a spell wrapper we preserve because it already works.

## Evidence

At toolkit [49e9918b](https://github.com/KirkDiggler/rpg-toolkit/commit/49e9918b), the handwritten concentration condition owns its timer and child addresses and describes saves through FollowUp. Native casting, departure and character views depend on that concrete type. The generic composer gives every output one holder, its amount evaluator returns an integer, and the machine evaluates the first matching transition on a beat. Those are useful first-consumer contracts, not proof of independent pieces reacting across several holders.

The existing generic effects/machine tests, root conditions/effects tests and targeted native concentration/Bless resolution suites passed. No new composed-maintenance behavior was implemented or walked. [The inspection receipt](https://github.com/KirkDiggler/rpg-project/issues/706#issuecomment-6725313075) names the commands and limits.

Kirk selected one authored application with a shared lifetime and linked entity-local attachments rather than a centrally persisted multi-holder runtime. The [detailed walkthrough](https://github.com/KirkDiggler/rpg-toolkit/blob/682680512165ef42f96842462c124bb63ca3a7f3/docs/ideas/composable-maintenance/README.md) is the next proposal, not an approved implementation: local assemblies of the existing machine, a neutral lifetime value, separate concentration/duration/payload meanings, exact links, and scalar-versus-unresolved-dice results.

## Interpretation

The distinction is not just caster versus recipient. We need to separate what an effect contributes, what keeps it alive, and which other effects share its end. Flattening independent components into one first-match transition list would hide another coupling: one part could prevent another from reacting to the same occurrence.

The earlier thought of reusing the concentration owner unchanged would preserve working code but fail the composition test. Its semantics are evidence to retain; its concrete class is not the abstraction to protect. The proposed assembly reuses the existing interpreter rather than introducing another one.

A second boundary is still deliberately visible: native concentration records currently speak in spell identities. The toolkit can prove a non-spell maintenance source without calling it a spell, but a future native item/feature consumer needs that presentation contract before it ships. The proposal does not claim that changing an internal ref makes the whole client path generic.

## Next

Review the complete contracts on [#2323](https://github.com/KirkDiggler/rpg-toolkit/pull/2323), including the new saved-format recommendation and the exact native/toolkit proof boundary. The implementation plan follows agreement; shape approval alone does not settle unseen interfaces. This journal remains open.

— cross-team agent, on behalf of KirkDiggler
