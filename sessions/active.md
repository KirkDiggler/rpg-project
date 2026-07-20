# Active handoff — 2026-07-20 (~04:45): PLATFORM session pause-and-restart

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.
> This rewrite covers the PLATFORM lane only (toolkit / api / protos / deploy). The
> game-UX (web#525) and asset (web#523) lanes run their own parallel sessions; their
> state is theirs — coordinate via board #19, respect the renderable seam.

Kirk restarted the platform session (new CC version, dontAsk permission mode). This
handoff is the pause point. Board #19 is the queue; issue-first, one issue per PR,
Opus gate posted on the PR before any merge-ready call.

## Now (platform)

**Wave: The Dungeon — Slice 2 (two-chamber dungeon end to end), rpg-project#96
(https://github.com/KirkDiggler/rpg-project/issues/96).** Defined, boarded (The
Dungeon leg), playtest bar stated on the umbrella. Slice issues: toolkit#804 →
protos#185 → api#676 → web#526. First two were dispatched this session:

- **protos#186 (https://github.com/KirkDiggler/rpg-api-protos/pull/186)** — Wall.id,
  DONE pending merge: CI green, gate PASS posted on the PR (agent review = review of
  record, Copilot doesn't cover protos), undrafted. **In Kirk's merge queue.**
- **toolkit#804** — two-chamber generator was IN FLIGHT at pause. Implementer was
  ordered to checkpoint: commit+push branch `feat/wave2-slice2-generator` (worktree
  `rpg-toolkit/.claude/worktrees/slice2-generator`). Check the branch on origin for
  the checkpoint state; resume by dispatching a fresh implementer onto that branch
  (verify its claims against the actual diff first — don't trust the WIP commit
  message). Its scope includes the #791-gate sweeps (reload-blocking integration
  test, door+wall dedup note).

## Kirk's merge queue at pause (all gate-passed, CI green — local settings deny agent `gh pr merge`, deliberate)

1. toolkit#803 — End error-string reword (gate PASS:
   https://github.com/KirkDiggler/rpg-toolkit/pull/803#issuecomment-5018530252)
2. protos#186 — Wall.id (gate PASS:
   https://github.com/KirkDiggler/rpg-api-protos/pull/186#issuecomment-5018701315)
3. This handoff PR.

## Solid (verified this session — do not re-derive)

- **api#675 SHIPPED, deploy chain verified end-to-end**: merged 03:14Z (1d6b26f),
  main Build Docker Image green INCLUDING push + Trigger-deployment steps,
  rpg-deployment "Deploy RPG Platform" success 03:17:57Z. Prod runs the pocket
  build. The TWO main commits before it had FAILED Docker builds — api#674 (decouple
  tests from publish) keeps earning priority.
- **Pocket-bounce live-in-Discord confirm NOT yet done** (deliberate, not
  forgotten): deploy chain verified + pre-merge director-viewed live evidence on
  api#675. The live confirm = Kirk in Discord runs two-goblin fight (TURN_BASED →
  FREE_ROAM breather → fresh pocket → ModeEnded). Expect web#516 (stale economy bar
  in FREE_ROAM) to show — it's user-visible now, quick-win queue.
- **web#520 merged by Kirk** (UX lane's item); web main build green after.
- **toolkit#805 filed+boarded** (docs: cross-module = merge-commit rule into
  toolkit CLAUDE.md) — executes an already-logged decision; retro action item.
- **Worktree inventory complete** (read-only agent sweep): ~60 worktrees across 5
  repos, vast majority merged+clean = safe sweep candidates. FLAGS: rpg-api
  `rage-bump` (on main, no PR, dirty — abandoned?); web `investigate-downed-510`
  (no PR — check findings written up before delete); rpg-project `handoff-platform`
  LOCKED (a parallel session's seat — DO NOT TOUCH); web `rpg-game-assets` (asset
  lane's intentional staging — exempt); several worktrees live OUTSIDE
  .claude/worktrees (rpg-api-worktrees/, old job tmp dirs) — a convention-scoped
  cleanup script misses them. Open/unmerged branches to triage individually:
  toolkit#744, toolkit#731(closed-unmerged), rpg-project#89, #88(closed-unmerged).

## The retro (NOT yet run — participatory, with Kirk; it closes the night wave)

Material all prepped: (1) deploy coupling api#674 with tonight's fresh evidence;
(2) gate-visibility convention — every gate keeps catching things, PR-comment
format now has two exemplars (#803, #186); (3) cross-module merge rule →
toolkit#805; (4) worktree cleanup per inventory above; (5) NEW process findings
this session: chrome-devtools MCP tools do NOT propagate to dispatched subagents
(playtest driving can't be delegated in bg-session shape — needs a decided
mechanism), and agents SYSTEMATICALLY idle without delivering reports (3 of 4
tonight; one-turn re-ask works every time — consider a prompt-pattern or hook
fix); (6) ledger sweep to close the wave: web#516, web Leave-button wiring,
api#667, api#674, toolkit#799/#800, web#521.

## Open questions

- Pocket bounce live-in-Discord: expected to pass (deploy verified, pre-merge live
  evidence) but NOT yet observed post-deploy — confirm with Kirk, don't log as done.
- toolkit#804 checkpoint quality: whatever the WIP branch claims, verify against
  the diff before resuming.

## Next (in order)

1. Kirk: merge queue above → restart done.
2. Retro with Kirk (agenda above).
3. Resume toolkit#804 from the checkpoint branch; gate; merge-commit if
   cross-module.
4. api#676 (needs #186 merged + #804 delivered): door projection, entrance spawn
   (roomCenterHex dies), per-chamber seeding. Trap: room.GetEntitiesInRange is
   blind to players.
5. web#526 (needs #676): door click → Interact. Coordinate with UX lane on board.
6. Slice-2 playtest vs the bar on #96; then Slice 3 (locked boss door) per
   wave2-design §Slice 3.

## Decision log (this session, 2026-07-20, all visible on board/PRs)

- Slice 2 defined as the platform lane's first wave: umbrella rpg-project#96 +
  4 slice issues, boarded The Dungeon leg (Kirk's kickoff order).
- toolkit#803 + protos#186 gates run and PASSED; gate comments on both PRs; merges
  queued to Kirk (agent-merge deny stands, deliberate).
- toolkit#805 filed executing the merge-commit-rule decision.
- Slice-2 front legs dispatched pre-retro (retro material is process-level, forks
  LOCKED — parallelism judged safe; retro validates).
- Pocket-bounce: local-bypass plan approved by Kirk mid-session, then obsoleted by
  the MCP-subagent finding; settled as deploy-chain-verified + live confirm with
  Kirk post-restart.

## Pointers

- Wave-2 design (forks LOCKED): ideas/the-dungeon/wave2-design.md — §Slice 2/3.
- Board #19: https://github.com/users/KirkDiggler/projects/19 (field IDs in
  director memory + this file's git history).
- Wave-level dungeon umbrella: rpg-api#648. Slice-2 wave umbrella: rpg-project#96.
- Deploy: rpg-deployment; nginx-http.conf is the LIVE config; verify GHCR moved +
  "Deploy RPG Platform" ran before believing a deploy changed anything.
- Open PRs at pause: toolkit#803, protos#186 (both merge-ready), plus whatever
  toolkit#804's checkpoint produced. Dependabot debt unchanged
  (web#314/#254/#253/#252/#251/#205, stale web#38).
