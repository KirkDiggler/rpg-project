# Studio appearance — execution brief

Work: [Web #1402](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1402), linked from [Project #867](https://github.com/KirkDiggler/rpg-project/issues/867). Design: [design.md](design.md). Explanation: [README.md](README.md).

## Outcome and scope

A common appearance editor for selected walls, NPCs and props, populated from verified Assets manifest data. Retain actual color, normal and emissive capabilities rather than assume every asset has every map. The first milestone is recognizable common controls, independent placements, consistent Apply/Cancel/Undo/Redo and native save/play meaning. Existing wall packs include real paired normal maps.

The operator approved the ownership boundary and priority: common controls, then mixed wall/door sections, then richer independent material editing. The tree-containing castle YAML is needed only for that composition edit; more courtyard assets may arrive independently.

Not included: general-purpose form engine, new asset registry/browser, new ingestion pipeline, new NPC mechanics or gameplay lighting, invented wall sides, operator-stack deployment, or making the old ten-PR merge authorization apply to this new line of work.

## Finish line

For the design pass: source-grounded ownership/walkthrough, explicit provider and owner contracts, disposition of the discovered masonry capture gap, and a checked implementation plan linked to this issue. A diagram alone is not implementation readiness.

For the eventual feature: imported choices drive shared controls; wall/NPC/prop changes stay instance-owned; valid channel state is clear; missing references retain intent; atomic history and native/play persistence are verified; existing normal-bearing packs are not downgraded. Applicable resource checks, automated tests, one focused visual verification and independent feature review precede release/pin integration. Merge and operator deployment remain separately authorized.

## Authority and stops

Ordinary inspection, design documentation and publishing this design PR are within the continued task. Follow the accepted shape without reopening it. R9 is a real scope boundary: existing masonry controls are deliberately Studio-only, so including them in played output adds captured material/chart facts rather than a UI-only refactor. Resolve that choice before deriving its implementation tasks. The detailed provider schema and any added capture contract then need the design's ownership/refusal checks; do not implement an assumed wire shape.

## Progress / resume

- [x] Reconcile source snapshots and existing publication/resource ownership.
- [x] Trace NPC complete looks, atlas overrides, masonry normals and current owner transactions.
- [x] Run focused baseline checks: five files, 83 tests passed; no new feature tests or browser proof claimed.
- [x] Write design and walkthrough; identify the concrete masonry play boundary.
- [ ] Operator resolves R9: include existing masonry packs in captured play now (recommended), or retain a clearly Studio-only lane in the first common-controls delivery.
- [ ] Complete the concrete schema/capture design for that scope and check one value source-to-play.
- [ ] Derive the implementation plan and its named tests; create delivery sub-issues only when real implementation slices are known.
- [ ] Execute the agreed plan through its required evidence and PR gates; do not infer merge/deployment authority.

Worktree: `rpg-project/.worktrees/studio-1402`, branch `design/1402-studio-appearance`, from main `b2a2dda`. Product source inspection is read-only in the existing courtyard worktrees: Web `9809f6ce` (tree equal to merged `3be3c439`), Assets `6345db54`. No Web/Assets source edits, dependency installation, new runtime, or stack restart is part of this design pass.

Baseline receipt: `/tmp/rpg-1402-design-baseline-tests.log`. Exact files/observations and the narrowed earlier normal-support statement are recorded in `knowledge/runs/2026-11-10-studio-appearance-design.md` and the linked PR/issue case file.
