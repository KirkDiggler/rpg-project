# Run — 2026-11-06 — live behavior consumer planning

Planning for [S1 adoption #866](https://github.com/KirkDiggler/rpg-project/issues/866), story [#859](https://github.com/KirkDiggler/rpg-project/issues/859). Project base is the unmerged documentation line at [`5e1eb58d486933fe12cc058eae1141602d630448`](https://github.com/KirkDiggler/rpg-project/commit/5e1eb58d486933fe12cc058eae1141602d630448).

## Shipped

- Prepared a proposed consumer contract, walkthrough, conditional task plan and investigation entry; no runtime/source field, installation, deployment or implementation-ready approval is claimed — [planning PR #877](https://github.com/KirkDiggler/rpg-project/pull/877).

- Revised the consumer plan/design for the accepted review direction, retained independent per-concern machines, and removed new editor panels from the first delivery; runtime implementation has not started — [accepted review](https://github.com/KirkDiggler/rpg-project/pull/877#pullrequestreview-5805884858).

- Added a focused source → instance contract: four separate identities, existing producer API handoff, source-preserving values, actor-local attachment uniqueness, refusal matrix and named future assertions; no runtime or selection-lane policy was implemented — [consumer plan](https://github.com/KirkDiggler/rpg-project/pull/877).

## Decided

- Reuse #866's existing live-journey delivery issue rather than invent a duplicate slice; proposed PR boundaries follow the actual behavior, encounter, session, API and web owners — [existing acceptance checklist](https://github.com/KirkDiggler/rpg-project/issues/866).
- Recommend one composer for an opted-in actor with an explicit imported creature-state instance/body-state source, because two independently acting runners require another arbitration contract; R4 was initially an operator decision, not inferred from R1–R3 — [original R4 checklist](https://github.com/KirkDiggler/rpg-project/pull/877#issuecomment-6693233263).
- Preserve Driver consultation before ordinary composer choice, reusing the existing shared Roller, because bypassing the wrapper would bypass compulsion rather than merely remove a redundant roll — [current compelled wrapper](https://github.com/KirkDiggler/rpg-toolkit/blob/9922e218d7942a8cf048af46cc2090f2e77d4fd1/rulebooks/dnd5e/session/compelled.go).

- KirkDiggler adopted the review on 2026-11-06: canonical runtime with input normalization, typed Delegated, distinct opportunity beats and YAML/Studio preservation first; this changes the plan's target, not the independence of distinct behavior machines — [ruling receipt](https://github.com/KirkDiggler/rpg-project/pull/877#issuecomment-6694313452).
- Retargeted #877 to main before the still-open #868 can merge/delete its branch; the prerequisite's commits remain visible in the main comparison until it lands — [planning PR](https://github.com/KirkDiggler/rpg-project/pull/877).

## Found

- Existing creature state is the source for blocking/posts, `Routed` turn-end also includes blocked/partial walks, and held-turn data has no behavior-completion claim — [current encounter seams](https://github.com/KirkDiggler/rpg-toolkit/tree/9922e218d7942a8cf048af46cc2090f2e77d4fd1/rulebooks/dnd5e/encounter).
- Heard wake inputs omit kind, barred is synchronous while other inputs wait for the raise, and first-per-beat listener indexing cannot safely consume concatenated role scopes — [wake input owner](https://github.com/KirkDiggler/rpg-toolkit/blob/9922e218d7942a8cf048af46cc2090f2e77d4fd1/rulebooks/dnd5e/encounter/wakebeats.go).
- Five selected root entrypoints, including the held suite and body/post/barred/dwell controls, pass unchanged in 0.030s; this is not a full-module or new-consumer result — [baseline revision](https://github.com/KirkDiggler/rpg-toolkit/commit/9922e218d7942a8cf048af46cc2090f2e77d4fd1).
- Studio preserves unknown root fields and re-splices original source; that does not itself prove editing or consumption of a new behavior field — [current codecs](https://github.com/KirkDiggler/rpg-dnd5e-web/tree/2150256f9e067c285c7723daf12f4ec54a03bc77/src/concepts/world-building).

## Mistaken

- The progress message prematurely announced starting the provider task while source/selection-lane contracts still need specification; no runtime files were edited, and the revised plan records that boundary instead of presenting the review adoption as completed implementation planning — [current plan record](https://github.com/KirkDiggler/rpg-project/pull/877).

- The early task draft did not explicitly preserve the compulsion wrapper before composer selection; tracing the real session capability path exposed it, and the conditional plan now names typed delegation and the no-brain/no-roll compulsion test — [current capability composition](https://github.com/KirkDiggler/rpg-toolkit/blob/9922e218d7942a8cf048af46cc2090f2e77d4fd1/rulebooks/dnd5e/session/capabilities.go).
