## Files Retrieved

None. Inspection used Git metadata only; no file contents were read or dumped.

## Review Findings

- **Blocker — `/home/kirk/game-dev/rpg-project`:** primary checkout is dirty and should not be used for attack-die edits.
- **Warning — `docs/teams/roles/rpg-toolkit-member/context/active-work.json`:** one unstaged tracked modification.
- **Warning — `.pi-subagents/` and `.pi/subagents/`:** 39 untracked runtime artifacts.
- **Info — `/home/kirk/game-dev/rpg-project/.worktrees`:** exists, is ignored by `.git/info/exclude:8:/.worktrees/`, and is the safest isolation location.
- **Info:** no staged changes were observed.

## Repository State

- Top level: `/home/kirk/game-dev/rpg-project`
- Branch: `main`, tracking `origin/main`, ahead/behind `+0/-0`
- HEAD: `9aae0194259f4002231c6323e13f88e44b39cdb3`
- Local `origin/main`: present at the same SHA
- Git dir: `.git` → `/home/kirk/game-dev/rpg-project/.git`
- Common dir: `.git` → `/home/kirk/game-dev/rpg-project/.git`
- `.git` is a directory, confirming this is the primary checkout.
- Submodule guard: clear
  - No superproject
  - No gitlink entries
  - No tracked `.gitmodules`

### Exact dirty paths

Tracked, unstaged:

- `docs/teams/roles/rpg-toolkit-member/context/active-work.json`

Untracked:

- `.pi-subagents/artifacts/16239661_delegate_0_input.md`
- `.pi-subagents/artifacts/16239661_delegate_0_meta.json`
- `.pi-subagents/artifacts/16239661_delegate_0_output.md`
- `.pi-subagents/artifacts/16239661_delegate_0_transcript.jsonl`
- `.pi-subagents/artifacts/23c4bbca_scout_0_input.md`
- `.pi-subagents/artifacts/23c4bbca_scout_0_meta.json`
- `.pi-subagents/artifacts/23c4bbca_scout_0_output.md`
- `.pi-subagents/artifacts/23c4bbca_scout_0_transcript.jsonl`
- `.pi-subagents/artifacts/3a3b7950_delegate_0_input.md`
- `.pi-subagents/artifacts/3a3b7950_delegate_0_meta.json`
- `.pi-subagents/artifacts/3a3b7950_delegate_0_output.md`
- `.pi-subagents/artifacts/3a3b7950_delegate_0_transcript.jsonl`
- `.pi-subagents/artifacts/4891ec74_delegate_input.md`
- `.pi-subagents/artifacts/4891ec74_delegate_meta.json`
- `.pi-subagents/artifacts/4891ec74_delegate_output.md`
- `.pi-subagents/artifacts/4891ec74_delegate_transcript.jsonl`
- `.pi-subagents/artifacts/50a8b71c_delegate_0_input.md`
- `.pi-subagents/artifacts/50a8b71c_delegate_0_meta.json`
- `.pi-subagents/artifacts/50a8b71c_delegate_0_output.md`
- `.pi-subagents/artifacts/50a8b71c_delegate_0_transcript.jsonl`
- `.pi-subagents/artifacts/5ab7e1d5_delegate_0_input.md`
- `.pi-subagents/artifacts/5ab7e1d5_delegate_0_meta.json`
- `.pi-subagents/artifacts/5ab7e1d5_delegate_0_output.md`
- `.pi-subagents/artifacts/5ab7e1d5_delegate_0_transcript.jsonl`
- `.pi-subagents/artifacts/a0ba5923_delegate_0_input.md`
- `.pi-subagents/artifacts/a0ba5923_delegate_0_meta.json`
- `.pi-subagents/artifacts/a0ba5923_delegate_0_output.md`
- `.pi-subagents/artifacts/a0ba5923_delegate_0_transcript.jsonl`
- `.pi-subagents/artifacts/c40c5acf_worker_0_input.md`
- `.pi-subagents/artifacts/c40c5acf_worker_0_meta.json`
- `.pi-subagents/artifacts/c40c5acf_worker_0_output.md`
- `.pi-subagents/artifacts/c40c5acf_worker_0_transcript.jsonl`
- `.pi-subagents/artifacts/dd419acb_scout_0_input.md`
- `.pi-subagents/artifacts/dd419acb_scout_0_meta.json`
- `.pi-subagents/artifacts/dd419acb_scout_0_output.md`
- `.pi-subagents/artifacts/dd419acb_scout_0_transcript.jsonl`
- `.pi/subagents/artifacts/c646fa83_scout_0_input.md`
- `.pi/subagents/artifacts/c646fa83_scout_0_transcript.jsonl`
- `.pi/subagents/missions/19cacc64-80d3-4174-8088-4da303647f23.json`

## Worktree Locations

- `.worktrees`: present directory; ignored by repository-local exclude.
- `worktrees`: absent; not ignored.
- Proposed `.worktrees/attack-die-3d`: absent and not registered.
- `ideas/attack-die-3d`: absent from local and remote-tracking refs.
- No local or remote-tracking branch containing both “attack” and “die” was found.

## Linked Worktrees

Clean unless marked dirty:

- `/home/kirk/game-dev/rpg-project` — `main` — **dirty: 1 tracked, 39 untracked**
- `/home/kirk/.claude/jobs/1c129c0a/tmp/wt-spec-v04` — `spec/v0.4-proposal`
- `/home/kirk/.claude/jobs/defe6c51/tmp/rp-doc` — `docs/wave-shape-rules`
- `/home/kirk/game-dev/.claude/worktrees/dungeon-authoring-static-delta` — `idea/dungeon-authoring-plan` — **dirty: 9 untracked**
- `/home/kirk/game-dev/.claude/worktrees/dungeon-walls-two-tier-sync` — `docs/dungeon-walls-two-tier-fog-sync`
- `/home/kirk/game-dev/.claude/worktrees/idea-dungeon-walls` — `idea/dungeon-walls`
- `/home/kirk/game-dev/.claude/worktrees/parallel-lab-howto-docs` — `docs/parallel-lab-api-howto`
- `/home/kirk/game-dev/.pi-worktrees/rpg-project-135` — `design/135-pi-team-harness`
- `/home/kirk/game-dev/.pi-worktrees/rpg-project-139` — `feat/139-pi-team-harness-skeleton`
- `/home/kirk/game-dev/.pi-worktrees/rpg-project-141` — `feat/141-pi-github-dispatch-contract`
- `/home/kirk/game-dev/.pi-worktrees/rpg-project-144` — `feat/144-pi-rpc-worker-supervisor` — **dirty: 4 tracked, 10 untracked**
- `/home/kirk/game-dev/.pi-worktrees/rpg-project-145` — `chore/145-remove-pi-runtime-copy`
- `/home/kirk/game-dev/.pi-worktrees/rpg-project-145-baseline` — detached
- `/home/kirk/game-dev/.pi-worktrees/rpg-project-150` — `design/150-pi-live-session-adjustments`
- `/home/kirk/game-dev/.pi-worktrees/rpg-project-171` — `fix/171-orphaned-checkout-recovery`
- `/home/kirk/game-dev/.pi-worktrees/rpg-project-182` — `docs/182-scoped-director-delegation`
- `/home/kirk/game-dev/.pi-worktrees/rpg-project-198` — `docs/198-toolkit-role-lifecycle`
- `/home/kirk/game-dev/.pi-worktrees/rpg-project-native-ubuntu-toolkit-sandbox` — `docs/211-fix-plan-blob-rehydration` — **dirty: 135 untracked**
- `/home/kirk/game-dev/.pi-worktrees/rpg-project-toolkit-contributor-sandbox-design` — `docs/208-toolkit-contributor-sandbox` — **dirty: 52 untracked**
- `/home/kirk/game-dev/.worktrees/rpg-project-204-visual-anchor-metadata` — `design/204-visual-anchor-metadata`
- `/home/kirk/game-dev/.worktrees/rpg-project-monster-ai` — `feat/201-monster-ai-brainstorm`
- `/home/kirk/game-dev/rpg-project/.claude/worktrees/assets-context-docs` — `docs/assets-member-context` — **dirty: 1 tracked**
- `/home/kirk/game-dev/rpg-project/.claude/worktrees/character-facing` — `idea/character-facing`
- `/home/kirk/game-dev/rpg-project/.claude/worktrees/dev-branch-merge-rule` — `docs/dev-branch-merge-rule`
- `/home/kirk/game-dev/rpg-project/.claude/worktrees/docs-fix-merge-method-enforceable` — `docs/fix-merge-method-enforceable`
- `/home/kirk/game-dev/rpg-project/.claude/worktrees/dungeon-visual-fidelity` — `docs/dungeon-visual-fidelity`
- `/home/kirk/game-dev/rpg-project/.claude/worktrees/fog-event-layer` — `docs/fix-cwd-guidance` — **dirty: 1 untracked**
- `/home/kirk/game-dev/rpg-project/.claude/worktrees/handoff` — `docs/handoff-2026-07-24`
- `/home/kirk/game-dev/rpg-project/.claude/worktrees/local-dev-stack-on-dev-image` — `docs/local-dev-stack-on-dev-image`
- `/home/kirk/game-dev/rpg-project/.claude/worktrees/pacing-promotion-plan` — `ideas/combat-pacing-promotion-plan`
- `/home/kirk/game-dev/rpg-project/.claude/worktrees/retro-01` — `docs/retro-crypt-monsters`
- `/home/kirk/game-dev/rpg-project/.claude/worktrees/three-lanes` — `docs/three-lane-ownership`
- `/home/kirk/game-dev/rpg-project/.claude/worktrees/unit-spec-v03` — `spec/v0.3-ratified`
- `/home/kirk/game-dev/rpg-project/.worktrees/114-animation-factory` — `idea/114-animation-factory` — **dirty: 1 untracked**
- `/home/kirk/game-dev/rpg-project/.worktrees/180-boss-region-clarification` — `docs/180-boss-region-clarification`
- `/home/kirk/game-dev/rpg-project/.worktrees/dungeon-builder-design` — `feat/169-dungeon-builder-design`
- `/home/kirk/game-dev/rpg-project/.worktrees/howto-localdev` — `docs/local-dev-howto`
- `/home/kirk/game-dev/rpg-project/.worktrees/main` — detached
- `/home/kirk/recovery-worktrees/rpg-project-128-crypt-monsters` — `design/128-crypt-monsters`
- `/home/kirk/recovery-worktrees/rpg-project-assets-blender-tooling` — `docs/138-assets-blender-tooling`

## Recommended Non-Destructive Route

An authorized writer should create a **new linked worktree**, without touching or cleaning the primary checkout:

```bash
git -C /home/kirk/game-dev/rpg-project worktree add --no-track \
  -b ideas/attack-die-3d \
  /home/kirk/game-dev/rpg-project/.worktrees/attack-die-3d \
  refs/remotes/origin/main
```

Then write only inside `/home/kirk/game-dev/rpg-project/.worktrees/attack-die-3d`.

This uses the already-local `origin/main`, avoids fetching, preserves every dirty primary-checkout path, uses an ignored established worktree directory, and does not reuse another task’s worktree. The command was **recommended only, not executed**.