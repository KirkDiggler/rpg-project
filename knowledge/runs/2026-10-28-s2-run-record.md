# Run — 2026-10-28 — Team memory S2 run record

Slice [rpg-project#692](https://github.com/KirkDiggler/rpg-project/issues/692) of story [rpg-project#676](https://github.com/KirkDiggler/rpg-project/issues/676). Base: `rpg-project` `main` at [`39cae207`](https://github.com/KirkDiggler/rpg-project/commit/39cae207).

## Shipped

- `knowledge/runs/TEMPLATE.md` with four sections and one worked example line each, and `knowledge/README.md` for the workspace level — this PR.
- The run record written into three skills: build-slice (the shape and where it lives), pr-review (check it exists, four sections, links resolve), merge-out (a PR without one is not merge-ready) — this PR.
- One pointer line in `CLAUDE.md`'s table — this PR.

## Decided

- The record lives at the lowest directory containing everything the PR changed, so a docs-only PR spanning levels uses the repo root, because rule 13 places knowledge at the lowest binding level and a run binds what it touched — [design rule 2](https://github.com/KirkDiggler/rpg-project/blob/main/ideas/team-memory/design.md#standing-rules).
- The record may carry commit pins and past tense while the `knowledge/` files beside it may not, because the record is the case file and the doc-genre check exists to keep case-file material out of loaded docs — [rpg-project CLAUDE.md, "When you hit a gap"](https://github.com/KirkDiggler/rpg-project/blob/main/CLAUDE.md#when-you-hit-a-gap).
- A missing record is an Important review finding and a decided line without a because is Minor, because the record is the only evidence the ingest reads and a because is what the shift question checks against — [design rule 13 on PR #696](https://github.com/KirkDiggler/rpg-project/pull/696).
- The skills gain one paragraph each and no new approval step, because the slice issue forbids one — [rpg-project#692](https://github.com/KirkDiggler/rpg-project/issues/692).

## Found

- The build-slice skill names `Closes <repo>#<n>` for the slice a PR finishes while the story skill says merging the last PR does not close the slice; the two read against each other and the slice issue, not this PR, is where that is settled — [build-slice L88-90](https://github.com/KirkDiggler/rpg-project/blob/main/.agents/skills/build-slice/SKILL.md#L88-L90), [story L71-72](https://github.com/KirkDiggler/rpg-project/blob/main/.agents/skills/story/SKILL.md#L71-L72).
- The S1 PR rpg-toolkit#2136 already carries a record in this shape, written before the template existed; the template copies its section order — [rpg-toolkit#2136](https://github.com/KirkDiggler/rpg-toolkit/pull/2136).

## Mistaken

- none observed.
