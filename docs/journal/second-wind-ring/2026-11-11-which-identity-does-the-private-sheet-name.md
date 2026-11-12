# Which identity does the private sheet name?

Written 2026-11-11. [Composable conditions S3 #706](https://github.com/KirkDiggler/rpg-project/issues/706), [source-identity finding](https://github.com/KirkDiggler/rpg-toolkit/pull/2329#discussion_r4542938014).

## Question

Does separating origin, replacement address and invocation in the runtime also keep them separate when the player asks who supplied an effect? The [previous entry](2026-11-11-following-pending-delivery-to-the-private-sheet.md) followed the maintained payload into the private sheet. A readable effect name was not enough to establish a correct source name.

## Evidence

The bounded joined review found one Important defect: the private SourceMember projection used the condition's address source. A coordinator uses its application invocation there. That identity is valid for replacement/custody, but the rail cannot resolve it as a creature's name.

The root regression reproduced both forms of confusion: a coordinator invocation and a recipient's distinct replacement qualifier. The actual v2 API acceptance also failed with the invocation in place of the caster. Root `87602d8f` reads immutable origin for the linked metadata protocol; API `14a3edec` passes it through; web `87f50a35` proves the supplied caster resolves without parsing or fallback. [Focused independent closure](https://github.com/KirkDiggler/rpg-toolkit/pull/2329#pullrequestreview-5865232723) marked the finding Addressed. It did not restart unaffected review scopes.

The subsequent [bounded native walk](https://github.com/KirkDiggler/rpg-toolkit/blob/design/bless-bane-composition/docs/ideas/composable-maintenance/qa.md) used two fresh, normally acquired characters and two isolated browser contexts. The live inspector displayed **Source: QA Mercy**. Both clients retained Bless after reload. A naturally failed Bane delivery later contributed `5 − 1 = 4` to a guard's save; the positive Bless term remained independent. Replacing one caster's Bless with existing Guidance removed the old recipients while leaving the other caster's Bane. Departure, group short rest and end-of-run cleanup removed the applicable links without refunding spell payment.

The captain's castle attack missed. No positive caster damage or concentration failure was manufactured; the named resolver tests remain the evidence for those branches. A local QA driver failed to serialize an optional field after an attack had completed. We recovered that single attack from persisted story rather than retrying it for a cleaner recording. Native RPC/stream observations, mounted UI observations and test fixtures are distinct evidence, even when they agree.

## Interpretation

The runtime separation survived, but an older projection still assigned the wrong meaning to a familiar field. This is a consumer-boundary defect, not a reason to merge origin and invocation or teach the UI to decode opaque IDs. The correction belongs where the source fact is selected; the API and rail should remain boring readers.

The walk also distinguishes completeness of an implementation from readiness to release it. The reviewed and walked cut can be sound while advancing bases create dependency conflicts and remote CI remains queued. Neither a clean review nor a screenshot resolves that later integration boundary.

## Next

[S3's checklist](https://github.com/KirkDiggler/rpg-project/issues/706) owns release preparation: preserve parallel changes during merge-on-top catch-up, obtain current-head remote gates, and adopt CI-produced tags inside-out only with explicit merge authority. This journal remains open; no new architectural ruling or release permission follows from the walk.
