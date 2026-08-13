## Completed

Created and locally committed the design-only change for issue #208.

- **Worktree:** `/home/kirk/game-dev/.pi-worktrees/rpg-project-toolkit-contributor-sandbox-design`
- **Branch:** `docs/208-toolkit-contributor-sandbox`
- **Commit:** `ab42e2a22bafb483981ace1baade2d7cc58a5ad5`
- **Commit message:** `docs: design toolkit contributor sandbox (#208)`
- **Diff:** 1 file, 522 insertions
- No `plan.md`, implementation code, push, or PR created.
- Residual questions: none.
- Independent reviewer gate remains pending.

### Checks

- `prettier --check ideas/toolkit-contributor-sandbox/design.md` — exit 0
- `git diff --check HEAD^ HEAD` — exit 0
- `git show --check --oneline --stat HEAD` — exit 0
- Scoped changed-file assertion — exit 0
- No-plan assertion — exit 0
- Clean worktree assertion — exit 0
- No staged files assertion — exit 0
- Primary HEAD/branch/status invariants — exit 0

Initial formatting checks found Prettier/trailing-space issues; these were corrected before commit, and all final checks passed.

## Files Changed

- `ideas/toolkit-contributor-sandbox/design.md` — Added the bounded cross-repository design, daily WSL2 workflow, ownership and control flow, API seeding contract, sandbox lifecycle, safety gates, verification matrix, phases, and non-goals.

## Notes

The primary checkout remained untouched at `c69f71f9e70b5864702845454147cb8ef4f4692b` on `main`. Its pre-existing unrelated modifications and untracked `.pi-subagents/` directory remained exactly unchanged, with no staged files.