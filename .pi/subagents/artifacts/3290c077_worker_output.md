## Completed

Applied the independent-review corrections to PR [#217](https://github.com/KirkDiggler/rpg-project/pull/217).

### Changed sections

- **Established facts** — added immutable `rpg-game-assets` PR #46, commit, merge commit, and GLB SHA-256 references without claiming provenance.
- **Decisions / Component boundaries / Existing presentation / Data flow** — made `EncounterView` the FIFO owner, preserved existing sequencer callbacks, locked renderer mode per token, and defined stale-work cancellation.
- **Face-map and asset contract** — froze `<= 0.25°` settle observation followed by exact target-quaternion hold.
- **Failure behavior** — specified immediate whole-beat SVG fallback without readiness waits.
- **Performance behavior** — added named matrix, paired same-build protocol, 20 warm samples, 8-second post-unmount window, and explicit regression budgets.
- **Validation and evidence** — expanded the immutable readability tuple.
- **Testing/playtest and graduation** — added lifecycle, tolerance, invalidation, and performance gates.

## Files Changed

- `ideas/attack-die-3d/design.md`

No tests or `plan.md` were added.

## Commit and PR

- Commit: `427c3da60db15b1a84ed45d2bb353076666e0ded`
- Branch: `idea/216-attack-die-3d`
- PR #217 head: exact same SHA
- PR state: `OPEN`, ready, base `main`, head `idea/216-attack-die-3d`
- Merge state: `CLEAN`, `MERGEABLE`

## Validation

All final checks exited `0`:

- Markdownlint: zero errors
- `git diff --check HEAD^..HEAD`
- Placeholder scan
- Contradiction/scope/ambiguity structural self-review
- Current-commit one-file scope check
- GitHub link resolution
- Asset commit/hash verification
- Local/upstream SHA comparison
- Tracked/index cleanliness check

Linked-worktree final status, including untracked files:

```text
## idea/216-attack-die-3d...origin/idea/216-attack-die-3d
?? .pi/
```

Tracked files and index are clean. The pre-existing untracked `.pi/` directory was not modified or committed.

## Residual Risks

- Kirk’s written-spec approval and the independent reviewer gate remain pending.
- Asset provenance/licensing remains unresolved.
- Face calibration and human readability evidence remain future implementation work.
- Mobile/low-GPU profile availability can block promotion.
- Performance budgets remain unmeasured until concept implementation.