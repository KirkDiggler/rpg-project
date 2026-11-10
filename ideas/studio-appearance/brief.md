# Studio appearance — execution brief

Work: [Web #1402](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1402), linked from [Project #867](https://github.com/KirkDiggler/rpg-project/issues/867). Design: [design.md](design.md). Explanation: [README.md](README.md).

## Outcome and scope

A common appearance editor for selected walls, NPCs and props, populated from verified Assets manifest data. Retain actual color, normal and emissive capabilities rather than assume every asset has every map. The first milestone is recognizable common controls, independent placements, consistent Apply/Cancel/Undo/Redo and native save/play meaning. Existing wall packs include real paired normal maps.

The operator approved the ownership boundary and priority: common controls, then mixed wall/door sections, then richer independent material editing. The tree-containing castle YAML is needed only for that composition edit; more courtyard assets may arrive independently.

Not included: general-purpose form engine, new asset registry/browser, new ingestion pipeline, new NPC mechanics or gameplay lighting, invented wall sides, operator-stack deployment, or making the old ten-PR merge authorization apply to this new line of work.

## Finish line

The source-grounded design and [checked plan](plan.md) include the approved masonry lane. Implement that plan directly; the diagram alone was not the finish line.

For the eventual feature: imported choices drive shared controls; wall/NPC/prop changes stay instance-owned; valid channel state is clear; missing references retain intent; atomic history and native/play persistence are verified; existing normal-bearing packs are not downgraded. Applicable resource checks, automated tests, one focused visual verification and independent feature review precede release/pin integration. Merge and operator deployment remain separately authorized.

## Authority and stops

R9 is approved: include existing normal-bearing masonry wall finishes in captured play. Continue with concrete contracts, the checked plan and implementation without another blanket approval. Preserve the design's ownership/refusal checks; return only genuinely new architectural decisions. The new wave has no merge or operator-stack deployment authorization.

## Progress / resume

- [x] Reconcile source snapshots and existing publication/resource ownership.
- [x] Trace NPC complete looks, atlas overrides, masonry normals and current owner transactions.
- [x] Run focused baseline checks: five files, 83 tests passed; no new feature tests or browser proof claimed.
- [x] Write design and walkthrough; identify the concrete masonry play boundary.
- [x] Operator resolved R9: include existing masonry packs in captured play now — [agreement](https://github.com/KirkDiggler/rpg-project/pull/915#issuecomment-6758627344).
- [x] Complete the concrete schema/capture contract: schema3 atlas compatibility plus per-wall face choices with existing local mapping, no speculative regional chart payload.
- [x] Derive [plan.md](plan.md), its named tests, requirement coverage and provider/consumer seam checks. This is one deliverable under Web#1402; tasks follow release-unit/test boundaries, not one issue per branch.
- [ ] Execute the agreed plan through its required evidence and PR gates; do not infer merge/deployment authority.

Worktrees: Project `.worktrees/studio-1402` on `design/1402-studio-appearance` from main `b2a2dda`; Web, Assets, toolkit encounter, API and Protos each use their own `.worktrees/studio-1402` on `feat/1402-studio-appearance`. Toolkit session has `.worktrees/studio-1402-session` on `feat/1402-studio-appearance-sdk`. Baselines: Web `3be3c439`, Assets `6345db54`, toolkit `ba6d40f2`, API `b4af0f1d`, Protos `45a0fb36`. No new runtime or stack restart. Initial inspection used the equivalent earlier courtyard snapshots; plan from the fresh owning worktrees.

Baseline receipt: `/tmp/rpg-1402-design-baseline-tests.log`. Exact files/observations and the narrowed earlier normal-support statement are recorded in `knowledge/runs/2026-11-10-studio-appearance-design.md` and the linked PR/issue case file.
