# Studio appearance — shipped handoff

Delivery slice: [Courtyard S5 #919](https://github.com/KirkDiggler/rpg-project/issues/919). Broader follow-up: [Web #1402](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1402). Design/walkthrough: [design.md](design.md), [README.md](README.md). The implementation plan is archived; do not restart it.

## Delivered

One manifest-driven Appearance panel for walls, NPCs and props, using typed owner adapters and existing Apply/Cancel/history/native persistence. Valid choices come from Assets metadata, not per-model hard-coded lists. Existing normal-bearing masonry faces now reach captured play and reload. Unsupported independent channels are not invented; unbound region defaults remain unapplied. NPC rules, blockers, openings, door state and gameplay lighting remain separate.

Both operator looks are accepted: panel comment6775787958, played wall comment6808054615 on #919. One bounded independent review found the full authoring Atlas missing face choices; F1 was fixed with a two-reader/alias-isolation regression and full gates. No repeated art review is required.

## Released chain

| Layer     | PR    | Merge / version                                                                                    |
| --------- | ----- | -------------------------------------------------------------------------------------------------- |
| Assets    | #345  | `32d11dc19c2783f5c2cda711c61c933b265d8759`                                                         |
| Encounter | #2356 | `1fedb460e8de0820b7192c6e48112f8d4c26c677` → v0.204.0                                              |
| Session   | #2357 | `d7069af90a0bc78facfc130af522967b5b19ceff` → v0.189.0                                              |
| API dev   | #1226 | `212b38f27f659a09acc1f229b16740a30f7b1171`; encounter v0.204.0 / newer compatible session v0.191.0 |
| Web dev   | #1404 | `a09b555e0ebc106877a0262209d5a942e2ceec10`; TSv0.1.270, actual merged material32d11dc              |

Go generated source is `4f63b5b97fa8`; Protos#427 was operator-merged/generated earlier. No pseudo-version remains in the final direct encounter/session pins. The merged Web tree equals gated head1f0270e2; final local gate10,406passed/five skipped and exact-head remote checks pass. API full local/current-head remote checks pass. No manual tags, replacements, force pushes or protection overrides.

The release paused when updated toolkit versions exposed missing upstream API speech/castle adoption. API#1228/#1229/#1231 resolved those failures; no gameplay repair was smuggled into this work and no check was waived.

## Evidence and remaining work

Private accepted panel and native masonry run artifacts are committed in Assets `evidence/919-studio-appearance/`, including screenshots, JSON and recorded drivers. The actual run shows native selection → Apply → Save a copy → Play → reload, captured face refs and the expected rendered normal map, with matching fetched hashes and no candidate requests/page exceptions. The separate API empty-provider/Redis test proves cold reload after source mutation. No broad combat or performance claim.

- Toolkit#2382 tracks the older full-Atlas MaterialOverrides omission; it is separate from the fixed S5 FaceFinishes finding.
- Web#1402/#1268 retains richer independent channel work; the first common-control milestone is shipped.
- Next product direction is mixed wall/door sections #906, under its own concrete design. The tree-containing composition #895 stays separate; no tree placement is invented.
- Project design/evidence PR#915 is not one of the five authorized implementation merges. Keep its worktree until its own publication decision.

## Close-out and authority

Operator authorized this five-PR chain; it is complete. **No operator-stack deployment or dev→main release occurred.** Operator3030/8115 and Lab3180 remain untouched.

Close-out removes only `local/studio-919` (3184/8126) and this wave's exact source/review/walk worktrees after preserving the evidence. Old courtyard worktrees/environments belong to earlier work and are not swept. No new merge/deployment authority is implied for the remaining design or future slices.

Continue the operator-approved cost cadence: one bounded useful checkpoint, concise records and focused checks; required full gates at their boundaries; human judgments used early. Do not reopen accepted looks, repeat completed investigations or silently add migrations.
