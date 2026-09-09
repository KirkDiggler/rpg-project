# Palette-aware Resumable Asset Releases Delivery Index

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver the approved design through one palette-aware review/promotion plan and one subsequent resumable-runner plan.

**Architecture:** Assets first establishes strict selected-appearance authority and promotion; Web then consumes that contract in the Lab. A later Assets issue adds the local release runner around those merged engines and the existing Web sync/check path.

**Tech Stack:** Python 3/unittest, React/TypeScript/Vitest, Git/GitHub CLI, Node/npm/Playwright.

**Spec:** `ideas/assets/asset-release-pipeline/design.md` for design #412, tracked in draft PR #413 (human approved; planning complete; implementation is not complete)

## Global Constraints

- Implementation source was planned against pinned Assets `37c13c68b6cfc87ad6684351f934b4ff1fd83515` and Web `d9553c2edda790a67aa33ae7870e8e72408056b6`; implementation must refresh repository instructions and bases before editing.
- One writer per worktree, one issue/branch/PR per repository slice. Assets feature changes land `main` before compatible Web changes land `dev`; the runner uses its own later Assets issue.
- Reuse existing audit, conversion, normalization, transaction, inventory, sync, and exact-ref engines. Do not add hosted services, daemons, automatic merges, cache relocation, thumbnail generation, or NPC/rules/multihex/composition scope.
- Design is human approved and planning is complete. Neither this index nor either detailed plan claims implementation complete.

---

## Delivery Status and Task Map

| Slice | Status | Tasks | Repository/landing order |
| --- | --- | --- | --- |
| [Palette-aware review and promotion](palette-plan.md) | Planned; implementation not started | 1. source-matched descriptors/preparation; 2. schema-v2 promotion/cumulative authority; 3. Lab dropdown/export | Tasks 1–2: one Assets PR → `main`; Task 3: one Web PR → `dev` |
| [Resumable local release runner](release-plan.md) | Planned; implementation not started | 1. workspace/state/commands; 2. stage/warnings/recovery; 3. GitHub/Provider gate; 4. exact merge/Web gate | One later Assets issue/branch/PR → `main`; tiny batch acceptance then creates one Provider PR and one Web PR |

## Execution Sequence

1. Implement and human-merge Palette Tasks 1–2 in one Assets worktree/PR.
2. Implement and human-merge Palette Task 3 in one Web worktree/PR based on current `origin/dev`.
3. Create a new Assets issue/worktree for all four runner tasks; do not split review findings into branches.
4. Run the tiny real acceptance. Pause for human Provider merge, resume from the exact merge, then pause for human Web merge.
5. Record final exact commits/PRs/warnings/browser result in the local generated summary and GitHub surfaces. Keep this design PR open until implementation is actually complete.

## Planning Validation

- Detailed plans contain exact file responsibility maps, consumes/produces signatures, red/green regression snippets, implementation boundaries, validation, commits, and self-review coverage tables.
- Relative links: [design](design.md), [palette plan](palette-plan.md), [release plan](release-plan.md).
- Source-map input used during planning: `existing-pipeline-seams.md` from project #412 planning artifacts.
