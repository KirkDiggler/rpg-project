# Multi-Contributor Workspace Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let each authenticated collaborator direct Project 19 work, resume local state across agent runtimes, and receive the correct Team, repository, and module guidance before editing.

**Architecture:** Project 19 remains shared truth, ignored `rpg-project/active.md` carries local continuity, and Git proves local implementation state. The rollout is additive before destructive: add Team charters and local-state contracts in rpg-project, switch game-dev and its Pi harness to them, add the toolkit module guard, then remove the legacy role/session state and run a real Codex pilot.

**Tech Stack:** Markdown and Git symlinks, GitHub Projects v2 and `gh`, Bash contract tests, TypeScript/Node test runner, Pi extension APIs, Go multi-module repository CI

**Spec:** `ideas/team-workflow/multi-contributor-workspace/design.md`

## Global Constraints

- Project 19 remains the only durable task board: https://github.com/users/KirkDiggler/projects/19.
- The authenticated operator comes from `gh api user`; Project owner `KirkDiggler` is not operator identity.
- Human direction remains authoritative; startup or dispatch must not select work automatically.
- Shared state lives on Project 19/issues/PRs, local continuity lives in ignored `rpg-project/active.md`, and Git wins every local-state disagreement.
- The only assumed automatic instruction load is `game-dev/AGENTS.md`; it must explicitly point to rpg-project, the selected repository, and the nearest module.
- Standing charters align with Project 19 Team: Platform, UI/UX, Assets, Monster AI, Cross-team.
- Signatures derive from Team: `platform`, `ui-ux`, `assets`, `monster-ai`, `cross-team`.
- The canonical `.agents/skills/` catalog starts with no `SKILL.md`; runtime-specific legacy skills are not imported.
- One rpg-toolkit implementation PR changes at most one nearest-`go.mod` module plus directly supporting root docs/checks.
- Provider modules merge and publish before consumer modules pin them; committed local `replace` and `go.work` state is forbidden.
- No tracked state or useful context is deleted until its Project/Git/docs disposition is recorded.
- Every repository change gets its own Project 19 slice issue and ready PR. PR #238 remains open through the pilot and carries plan amendments.
- Before the dynamic signature implementation merges, use the currently required repository signature. After it merges, use the Team-derived signature.

---

## File and slice map

| Slice | Repository/branch | Main responsibility |
|---|---|---|
| Existing board-doc amendment | rpg-project PR #230 branch | Move the open Project 19 idea under `ideas/team-workflow/` and repair links |
| Foundation | rpg-project fresh slice | Add local continuity, Team charters, clean skill namespace, and additive verification |
| Bootloader | game-dev fresh slice | Add root AGENTS and runtime-neutral startup; make Claude memory opt-in |
| Pi Team migration | game-dev fresh slice after Bootloader | Route by Team + repository, use Area/assignees/operator, and sign dynamically |
| Toolkit guard | rpg-toolkit fresh slice | Add AGENTS links, concise module law, and one-module-per-PR CI guard |
| Legacy cleanup | rpg-project fresh slice after Pi Team migration | Retire shared session/progress state and repoint runtime adapters |
| Acceptance | Project 19 checkpoints | Prove a clean Codex startup on the active contributor journey and run the retro |

### Stable interfaces between slices

- Team charter paths:
  - `docs/teams/roles/platform/prompt.md`
  - `docs/teams/roles/ui-ux/prompt.md`
  - `docs/teams/roles/assets/prompt.md`
  - `docs/teams/roles/monster-ai/prompt.md`
  - `docs/teams/roles/cross-team/prompt.md`
- Local continuity path: ignored `rpg-project/active.md`; template `rpg-project/docs/templates/local-active.md`.
- Shared skill catalog pointer: `rpg-project/.agents/skills/README.md`; no approved skill file in the pilot.
- Runtime Team role IDs: `platform`, `ui-ux`, `assets`, `monster-ai`, `cross-team`.
- Team signature labels are identical to those role IDs.
- Repository selection is independent of Team role selection.
- rpg-toolkit module scope check consumes NUL-delimited changed paths and exits non-zero when more than one nearest `go.mod` root appears.

---

### Task 1: Move the open Project 19 journey idea into the Team Workflow domain

**Issue/PR:** Existing rpg-project issue #229 and PR #230. Do not create another issue.

**Files:**
- Move: `ideas/project-19-journeys/brainstorm.md` → `ideas/team-workflow/project-19-journeys/brainstorm.md`
- Move: `ideas/project-19-journeys/design.md` → `ideas/team-workflow/project-19-journeys/design.md`
- Move: `ideas/project-19-journeys/plan.md` → `ideas/team-workflow/project-19-journeys/plan.md`
- Modify: `CLAUDE.md`
- Modify through GitHub API: rpg-project issue #229 and PR #230 bodies if they contain old path links

**Interfaces:**
- Consumes: open PR #230 head and its existing approved board design.
- Produces: the canonical `ideas/team-workflow/project-19-journeys/design.md` path referenced by PR #238.

- [ ] **Step 1: Verify the existing PR branch and refuse an unexpected worktree**

Run:

```bash
gh pr view 230 --repo KirkDiggler/rpg-project --json state,isDraft,headRefName,headRefOid,baseRefName
git -C /home/kirk/game-dev/.worktrees/rpg-project-229 status --short --branch
```

Expected: PR is open/non-draft, head is `docs/229-project-19-journeys`, base is `main`, and the worktree has no unrelated changes. If the known worktree no longer matches the live PR head, create a fresh worktree from the live head; never reset it.

- [ ] **Step 2: Write and run the pre-move assertion**

Run:

```bash
test -d /home/kirk/game-dev/.worktrees/rpg-project-229/ideas/project-19-journeys
test ! -e /home/kirk/game-dev/.worktrees/rpg-project-229/ideas/team-workflow/project-19-journeys
```

Expected: both commands pass, proving the move has not already happened.

- [ ] **Step 3: Move the directory and repair tracked references**

Run in the PR #230 worktree:

```bash
git mv ideas/project-19-journeys ideas/team-workflow/project-19-journeys
rg -l 'ideas/project-19-journeys' . --glob '!ideas/team-workflow/multi-contributor-workspace/**'
```

Replace every reported tracked reference with `ideas/team-workflow/project-19-journeys`. Do not rewrite GitHub comment history; update only mutable issue/PR bodies that still advertise the old path.

- [ ] **Step 4: Verify the move and path closure**

Run:

```bash
test ! -e ideas/project-19-journeys
test -f ideas/team-workflow/project-19-journeys/brainstorm.md
test -f ideas/team-workflow/project-19-journeys/design.md
test -f ideas/team-workflow/project-19-journeys/plan.md
! rg -n 'ideas/project-19-journeys' .
git diff --check
```

Expected: all pass and the diff is a three-file rename plus repaired pointers.

- [ ] **Step 5: Commit, push, and read back PR #230**

```bash
git add CLAUDE.md ideas/team-workflow/project-19-journeys
git commit -m 'docs: group Project 19 journey design under team workflow'
git push
gh pr view 230 --repo KirkDiggler/rpg-project --json headRefOid,files,url
```

Expected: the live head matches the pushed commit and files show the new domain path.

- [ ] **Step 6: Human ratification gate**

Post a signed checkpoint on #229 describing the move and the existing pilot evidence. Stop until the human director decides whether PR #230 is ready to merge. After a human merge, verify `origin/main` contains `ideas/team-workflow/project-19-journeys/design.md` before Task 2 branches.

---

### Task 2: Add the rpg-project multi-contributor foundation without deleting compatibility files

**Issue/PR:** Create one `Build` slice under journey #236, Team `Cross-team`, Area `Infra`, Initiative `Four-player Level-3 Dungeon`. Branch from fresh `origin/main` only after Task 1's merge gate.

**Files:**
- Create: `.gitignore`
- Create: `docs/templates/local-active.md`
- Create: `.agents/skills/README.md`
- Create: `docs/teams/roles/platform/prompt.md`
- Create: `docs/teams/roles/ui-ux/prompt.md`
- Create: `docs/teams/roles/assets/prompt.md`
- Create: `docs/teams/roles/monster-ai/prompt.md`
- Create: `docs/teams/roles/cross-team/prompt.md`
- Create: `scripts/verify-team-workflow.sh`
- Modify: `CLAUDE.md`
- Modify: `docs/teams/roles/README.md`
- Modify: `docs/teams/roles/working-agreements.md`
- Retain unchanged for compatibility: `sessions/active.md`, legacy role prompts/contexts, `.opencode/agents/**`, `.opencode/skills/**`

**Interfaces:**
- Consumes: Project 19 journey model on main.
- Produces: stable Team charter paths and local-state/template paths consumed by game-dev Task 4.

- [ ] **Step 1: Create the slice and isolated branch**

The issue body must use the PIH dispatch marker and name this exact acceptance: five Team charters, ignored local active file, empty canonical skills catalog, no removal of legacy runtime inputs, and deterministic verifier evidence. Add it beneath #236, then derive the branch by concatenating `feat/`, the new issue number, and `-team-workflow-foundation`.

- [ ] **Step 2: Write the failing verifier first**

Create `scripts/verify-team-workflow.sh` with these checks:

```bash
#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
fail() { printf 'FAIL: %s\n' "$*" >&2; exit 1; }

[ -L "$ROOT/AGENTS.md" ] || fail "AGENTS.md must remain a symlink"
[ "$(readlink "$ROOT/AGENTS.md")" = "CLAUDE.md" ] || fail "AGENTS.md must point to CLAUDE.md"
git -C "$ROOT" check-ignore -q --no-index active.md || fail "active.md must be ignored"

for heading in 'Operator:' '## User direction' '## Current focus' '## Local work' '## Last observed' '## Next' '## Open questions'; do
  grep -Fq "$heading" "$ROOT/docs/templates/local-active.md" || fail "local active template lacks $heading"
done

for team in platform ui-ux assets monster-ai cross-team; do
  prompt="$ROOT/docs/teams/roles/$team/prompt.md"
  [ -f "$prompt" ] || fail "missing Team charter $team"
  grep -Fq "— $team agent, on behalf of <github-login>" "$prompt" || fail "wrong signature contract for $team"
done

[ -f "$ROOT/.agents/skills/README.md" ] || fail "clean skill catalog README is missing"
if find "$ROOT/.agents/skills" -mindepth 2 -name SKILL.md -print -quit | grep -q .; then
  fail "pilot skill catalog must contain no SKILL.md"
fi

for pointer in 'gh api user' 'rpg-project/active.md' 'docs/teams/roles/' '.agents/skills/' "owning repository's AGENTS.md"; do
  grep -Fq "$pointer" "$ROOT/CLAUDE.md" || fail "CLAUDE.md lacks startup pointer $pointer"
done

printf 'PASS: additive multi-contributor foundation verified\n'
```

Make it executable and run it.

Expected: FAIL first because `.gitignore`, template, Team charters, and skills README do not exist.

- [ ] **Step 3: Add the ignored local continuity contract**

Create `.gitignore` with the root-anchored rule and rationale:

```gitignore
# Per-operator local continuity. Shared work remains on Project 19.
/active.md
```

Create `docs/templates/local-active.md` using the exact headings and field semantics from design §5. Its `User direction` block must say that the quote is verbatim, `Local work` must label SHA/status as observations, and `Open questions` must say they are not decisions.

Run:

```bash
git check-ignore -v --no-index active.md
```

Expected: `.gitignore` reports `/active.md`.

- [ ] **Step 4: Add the five Team charters**

Each prompt uses frontmatter `name` equal to its path slug and a description equal to the outcome-lens sentence below, followed by this fixed section order: `Outcome lens`, `Cross-repository responsibilities`, `Refuse and escalate`, `Completion evidence`, `Required load chain`, `Signature`.

Use the following exact identities and boundaries:

| Path | Outcome lens and frontmatter description | Must refuse | Signature |
|---|---|---|---|
| `platform/prompt.md` | toolkit/API/proto/deployment/workspace architecture that keeps rules in toolkit and hosts thin | rules in API, wire invention by hosts, global tool configuration | `— platform agent, on behalf of <github-login>` |
| `ui-ux/prompt.md` | screens, HUD, interaction, accessibility, and presentation | client-side game calculations or legality gates | `— ui-ux agent, on behalf of <github-login>` |
| `assets/prompt.md` | licensed ingestion, manifests, model loading, rendering, animation, and visual evidence | licensed source leakage or backend rule ownership | `— assets agent, on behalf of <github-login>` |
| `monster-ai/prompt.md` | intentional monster decisions expressed through toolkit-owned behavior contracts | AI rules in API/web or hidden non-composable turn scripts | `— monster-ai agent, on behalf of <github-login>` |
| `cross-team/prompt.md` | initiative seams, integration, coordination, and end-to-end verification | silently absorbing another Team's outcome or deciding for another director | `— cross-team agent, on behalf of <github-login>` |

Every charter must say that repository/module AGENTS—not the Team charter—owns technical commands and invariants.

- [ ] **Step 5: Establish the empty canonical skills namespace**

Create `.agents/skills/README.md` stating:

- shared repeatable procedures belong here only after individual review;
- the pilot contains no approved skills;
- `.opencode/skills` and `.claude/skills` are legacy candidates, not inputs;
- AGENTS invariants and live state must never be copied into a skill; and
- repository-only procedures belong in that repository's `.agents/skills/`.

Do not create any `SKILL.md`.

- [ ] **Step 6: Rewrite shared pointers additively**

Update `CLAUDE.md` top/startup and Project Board sections so a reader:

1. derives the operator with `gh api user --jq .login`;
2. treats Project 19 assignment as shared work;
3. reads ignored `rpg-project/active.md` if present;
4. lets the human choose focus;
5. reads the selected repository's root AGENTS and nearest scoped instructions; and
6. checks `.agents/skills/` only for a matching approved skill.

Rewrite `docs/teams/roles/README.md` around the five Team charters. Mark the old repository/fixer/support directories as temporary compatibility inputs pending the game-dev runtime migration; do not call them standing ownership.

Update `working-agreements.md` to remove `sessions/active.md` from new dispatch briefs and replace fixed Kirk signatures with the Team-derived `<github-login>` contract. Keep the paid-for seam, evidence, shell, and test rules.

- [ ] **Step 7: Run the verifier green and prove compatibility remains**

```bash
./scripts/verify-team-workflow.sh
./scripts/verify-opencode.sh
test -f sessions/active.md
test -f docs/teams/roles/rpg-toolkit-member/prompt.md
git diff --check
```

Expected: both verifiers pass, and legacy files still exist for current adapters.

- [ ] **Step 8: Commit and publish the foundation PR**

```bash
git add .gitignore .agents/skills/README.md CLAUDE.md docs/templates/local-active.md docs/teams/roles scripts/verify-team-workflow.sh
git commit -m 'feat: add multi-contributor team foundation'
git push -u origin "$(git branch --show-current)"
```

Open a ready PR closing its slice issue. Post a `cross-team` signed checkpoint. After human merge, verify all five Team prompts and the skills README from `origin/main` before Task 3.

---

### Task 3: Make game-dev AGENTS the runtime-neutral startup bootloader

**Issue/PR:** New game-dev `Build` slice under #236, Team `Platform`, Area `Infra`.

**Files:**
- Create symlink: `AGENTS.md` → `CLAUDE.md`
- Create: `scripts/install-claude-memory.sh`
- Create: `tests/contributor-startup-contract.sh`
- Modify: `CLAUDE.md`
- Modify: `README.md`
- Modify: `bootstrap.sh`
- Modify: `scripts/verify-workspace.sh`
- Modify: `tests/bootstrap-contract.sh`

**Interfaces:**
- Consumes: Team charters/template/skills README from Task 2 on rpg-project main.
- Produces: the only assumed automatic GPT instruction and an explicit load chain independent of private runtime memory.

- [ ] **Step 1: Write failing bootloader assertions**

Extend `tests/bootstrap-contract.sh` and create `tests/contributor-startup-contract.sh` to assert:

```bash
[ -L "$ROOT/AGENTS.md" ]
[ "$(readlink "$ROOT/AGENTS.md")" = "CLAUDE.md" ]
grep -Fq 'gh api user --jq .login' "$ROOT/CLAUDE.md"
grep -Fq 'rpg-project/AGENTS.md' "$ROOT/CLAUDE.md"
grep -Fq 'rpg-project/active.md' "$ROOT/CLAUDE.md"
grep -Fq "selected repository's AGENTS.md" "$ROOT/CLAUDE.md"
grep -Fq 'rpg-project/.agents/skills/' "$ROOT/CLAUDE.md"
grep -Fq 'operator differs' "$ROOT/CLAUDE.md"
grep -Fq 'Project 19 unavailable' "$ROOT/CLAUDE.md"
grep -Fq 'Git wins' "$ROOT/CLAUDE.md"
grep -Fq 'GAME_DEV_INSTALL_CLAUDE_MEMORY' "$ROOT/bootstrap.sh"
```

Also fail if `bootstrap.sh` copies `memory/*.md` outside the exact opt-in branch.

Run:

```bash
bash tests/contributor-startup-contract.sh
bash tests/bootstrap-contract.sh
```

Expected: FAIL because root AGENTS and the opt-in installer do not exist.

- [ ] **Step 2: Add the root symlink and rewrite the bootloader**

Create the link with:

```bash
ln -s CLAUDE.md AGENTS.md
```

Rewrite the beginning of `CLAUDE.md` as the explicit startup sequence:

```text
1. gh api user --jq .login
2. read rpg-project/AGENTS.md
3. read rpg-project/active.md when it exists
4. query Project 19 assignments for that login
5. ask the human director to choose focus
6. read the selected repository's AGENTS.md and nearest scoped AGENTS/README
7. load a matching rpg-project/.agents/skills skill only if an approved SKILL.md exists
```

Preserve portable workspace layout, licensed Synty boundaries, bootstrap ownership, and the game-dev-vs-product routing rule. Remove claims that Kirk is the only operator or that Claude memory is required. State failure behavior explicitly: if the active-file operator differs, preserve and stop; if Project 19 is unavailable, label it unavailable; if prose and Git disagree, Git wins; if several assignments exist, show all and let the human choose.

- [ ] **Step 3: Extract Claude memory installation behind explicit opt-in**

Move the current Section 5 copy/backup logic from `bootstrap.sh` into executable `scripts/install-claude-memory.sh`. Keep its path-derived project key and backup-before-overwrite behavior.

Replace the unconditional bootstrap section with:

```bash
log "Checking optional Claude memory overlay"
if [ "${GAME_DEV_INSTALL_CLAUDE_MEMORY:-0}" = "1" ]; then
  "$ROOT/scripts/install-claude-memory.sh"
  ok "optional Claude memory overlay installed"
else
  skip "Claude memory overlay not requested (set GAME_DEV_INSTALL_CLAUDE_MEMORY=1 to opt in)"
fi
```

The installer must describe memory as a personal convenience, not shared policy. It must not alter Pi, Codex, OpenCode, credentials, trust, or global provider configuration.

- [ ] **Step 4: Update workspace verification and README**

`scripts/verify-workspace.sh` must verify game-dev and rpg-project AGENTS links, all five Team charter files, the local-active template, and `.agents/skills/README.md`. It must not require any `SKILL.md`.

README Quickstart must state that bootstrap is complete without a memory install and that every contributor starts in game-dev, where AGENTS directs them to Project 19 and rpg-project.

- [ ] **Step 5: Run deterministic checks**

```bash
bash -n bootstrap.sh
bash -n scripts/install-claude-memory.sh
bash tests/contributor-startup-contract.sh
bash tests/bootstrap-contract.sh
bash scripts/verify-workspace.sh
```

Expected: all pass. Run bootstrap once without the environment variable and verify output says memory was not requested; do not run the opt-in installer against another user's home as a test.

- [ ] **Step 6: Commit and publish**

```bash
git add AGENTS.md CLAUDE.md README.md bootstrap.sh scripts/install-claude-memory.sh scripts/verify-workspace.sh tests/bootstrap-contract.sh tests/contributor-startup-contract.sh
git commit -m 'feat: add runtime-neutral contributor bootloader'
git push -u origin "$(git branch --show-current)"
```

Open a ready PR closing the slice. After human merge, run `./bootstrap.sh` from the primary game-dev root and verify the new bootloader without installing memory.

---

### Task 4: Migrate the Pi team harness from repository roles to Team roles and authenticated operators

**Issue/PR:** New game-dev `Build` slice under #236, Team `Platform`, Area `Infra`. Start only after Task 3 merges.

**Files:**
- Modify: `.pi/extensions/pi-team-harness/src/roles.ts`
- Modify: `.pi/extensions/pi-team-harness/src/repository-map.ts`
- Modify: `.pi/extensions/pi-team-harness/src/github.ts`
- Modify: `.pi/extensions/pi-team-harness/src/status.ts`
- Modify: `.pi/extensions/pi-team-harness/src/guided-dispatch.ts`
- Modify: `.pi/extensions/pi-team-harness/src/dispatch.ts`
- Modify: `.pi/extensions/pi-team-harness/src/charters.ts`
- Modify: `.pi/extensions/pi-team-harness/src/supervisor.ts`
- Modify: `.pi/extensions/pi-team-harness/src/checkpoint.ts`
- Modify: `.pi/extensions/pi-team-harness/src/rpc-worker.ts`
- Modify: `.pi/extensions/pi-team-harness/index.ts`
- Modify: `.pi/extensions/pi-team-harness/test/roles.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/repository-map.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/github.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/status.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/guided-dispatch.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/dispatch.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/charters.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/checkpoint.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/checkpoint-lifecycle.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/supervisor.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/index-dispatch-start.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/rpc-worker.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/activity-overlay.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/dirty-primary-worktree.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/inbox.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/integration-base.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/pull-request-publication.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/recovery.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/self-route.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/settlement-delivery.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/settlement-report.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/team-overlay.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/worktree.test.ts`
- Modify: `.pi/extensions/pi-team-harness/test/fixtures/github.json`
- Modify: `.pi/extensions/pi-team-harness/test/fixtures/team-status.json`
- Modify: `.pi/extensions/pi-team-harness/test/verifier.test.ts`
- Modify: `README.md`
- Modify: `scripts/verify-pi-team-harness.mjs`

**Interfaces:**
- Consumes: five Team charter paths from rpg-project main and the operator's live `gh` login.
- Produces:

```ts
export type Team = "Platform" | "UI/UX" | "Assets" | "Monster AI" | "Cross-team";
export type TeamRoleId = "platform" | "ui-ux" | "assets" | "monster-ai" | "cross-team";
export interface TeamRoleAdapter {
  readonly id: TeamRoleId;
  readonly team: Team;
  readonly charterPaths: readonly [string];
}
export function teamRoleFor(team: Team): TeamRoleAdapter;
export function repositoryDescriptorForRepository(repository: string): RepositoryDescriptor | undefined;
export function resolveRepositoryDescriptor(runtimeRoot: string, repository: string): ResolvedRepositoryDescriptor;
export interface Project19Fields { status?: string; team?: string; area?: string; kind?: string }
export interface GitHubTaskRow {
  readonly issueNumber: number;
  readonly title: string;
  readonly issueUrl: string;
  readonly status: string;
  readonly team: string;
  readonly area: string;
  readonly kind: string;
  readonly assignees: readonly string[];
  readonly checkpoint?: string;
  readonly blocker?: string;
  readonly pullRequestUrl?: string;
  readonly reviewPhase?: string;
  readonly gateStatus?: string;
  readonly nextReviewer?: string;
  readonly recovery?: string;
  readonly linkedTeams?: readonly string[];
  readonly linkedUrls?: readonly string[];
}
```

`ValidDispatch`, `ActiveWorker`, `SupervisorInspection`, and `RpcWorkerRequest` each gain an exact `readonly operatorLogin: string`; child process environment gains `PI_TEAM_HARNESS_OPERATOR_LOGIN`.

- [ ] **Step 1: Replace role-route tests with Team + repository tests (red)**

Update `roles.test.ts`, `repository-map.test.ts`, `guided-dispatch.test.ts`, and `dispatch.test.ts` to require:

- exactly five Team role IDs and exact charter paths;
- Monster AI support;
- one Team role usable across several valid repositories;
- repository resolution by issue repository, not role ID;
- game-dev allowed only for Platform + Area Infra;
- web and API routes use `origin/dev` → `dev`; and
- guided dispatch derives one Team role from the Project Team with no selector between repository-member roles.

Run:

```bash
./.pi/extensions/pi-team-harness/node_modules/.bin/tsx --test --test-concurrency=1 .pi/extensions/pi-team-harness/test/roles.test.ts .pi/extensions/pi-team-harness/test/repository-map.test.ts .pi/extensions/pi-team-harness/test/guided-dispatch.test.ts .pi/extensions/pi-team-harness/test/dispatch.test.ts
```

Expected: FAIL against repository-role adapters and `Feature` naming.

- [ ] **Step 2: Implement Team roles and independent repository routing**

Replace `ROLE_ADAPTERS` with the five Team adapters. Remove `RoleKind`, repository-member/fixer IDs, overlays, and `GUIDED_ROLE_ROUTES`.

Change repository resolution to use `identity.repository`. In child mode, derive the descriptor from `PI_TEAM_HARNESS_OWNING_ISSUE_URL`'s repository rather than `PI_TEAM_HARNESS_ROLE_ID`.

Update integration types:

```ts
export type IntegrationBaseRef = "origin/main" | "origin/dev";
export type PullRequestTarget = "main" | "dev";
```

Keep closed owner/origin/worktree mappings unchanged.

Run the four tests again; expected PASS.

- [ ] **Step 3: Write Area, assignee, and mine-view tests (red)**

Update `github.test.ts` and `status.test.ts` to require:

- Project field identity name `Area` at the existing field ID;
- project rows retain normalized `assignees: string[]`;
- `GhCliTeamStatusClient.getAuthenticatedLogin()` parses exactly one safe login from `gh api user --jq .login`;
- `TeamStatusScope` includes `mine`;
- mine view filters rows by exact case-insensitive login and does not apply a WIP limit; and
- missing/failed identity read yields unavailable status, not another user's rows.

Run:

```bash
./.pi/extensions/pi-team-harness/node_modules/.bin/tsx --test --test-concurrency=1 .pi/extensions/pi-team-harness/test/github.test.ts .pi/extensions/pi-team-harness/test/status.test.ts
```

Expected: FAIL because the current adapter uses `feature` and discards assignees.

- [ ] **Step 4: Implement Area, operator, and mine status**

Rename `feature` properties and aliases to `area` across GitHub parsing, snapshots, status rows, safety rendering, dispatch snapshots, and tests. Change `PROJECT_19_IDENTITY.fields.feature` to `fields.area` with name `Area` and the unchanged ID. Remove the synthetic `GitHubTaskRow.role` field and the missing-role escalation; Team is the standing role source.

Add authenticated login validation using GitHub's login character/length rules and no fallback. Extend the injected GitHub reader with `getAuthenticatedLogin(): Promise<string>`. `validateDispatch` must perform that read before worktree mutation, put the exact login on `ValidDispatch`, and carry it through supervisor inspection and `RpcWorkerRequest` into `PI_TEAM_HARNESS_OPERATOR_LOGIN`. Make bare `/team-status` and startup inbox refresh use `mine`; retain explicit `managed`, `team`, and `project` views.

Run the two targeted tests; expected PASS.

- [ ] **Step 5: Write dynamic checkpoint signature tests (red)**

Change `CheckpointInput` tests to require `operatorLogin` and exact endings:

```text
— platform agent, on behalf of dammitbilly0ne
— ui-ux agent, on behalf of KirkDiggler
— assets agent, on behalf of KirkDiggler
— monster-ai agent, on behalf of dammitbilly0ne
— cross-team agent, on behalf of KirkDiggler
```

Add rejection tests for role/login controls, embedded newlines, mismatched Team role IDs, and the legacy fixed `on behalf of KirkDiggler` validator.

Run:

```bash
./.pi/extensions/pi-team-harness/node_modules/.bin/tsx --test --test-concurrency=1 .pi/extensions/pi-team-harness/test/checkpoint.test.ts .pi/extensions/pi-team-harness/test/checkpoint-lifecycle.test.ts
```

Expected: FAIL against the fixed signature renderer.

- [ ] **Step 6: Implement authenticated checkpoint signing**

Add `operatorLogin` to `CheckpointInput`; render:

```ts
lines.push("", `— ${role} agent, on behalf of ${operatorLogin}`);
```

Validate the signature against the exact expected role/login passed to `validateSignedCheckpointBody`. In child tool registration, read the authenticated login through the injected `GhCommandRunner`, validate it, and require it to equal `PI_TEAM_HARNESS_OPERATOR_LOGIN` before rendering or publishing. Do not accept operator login from model tool parameters or continue after an auth identity switch.

Run checkpoint tests; expected PASS.

- [ ] **Step 7: Remove role context loading and make worker startup explicit (red then green)**

Update `charters.test.ts`, `supervisor.test.ts`, and `index-dispatch-start.test.ts` to require:

- only one Team charter file is loaded;
- no `context/` tree is discovered or serialized;
- the worker prompt names its Team, authenticated operator, owning repository, rpg-project AGENTS path, repository AGENTS path, issue, branch, and integration target; and
- worker prompts state that the runtime starts in the issue worktree and must read root/scoped AGENTS before the task brief.

Delete `LoadedRoleKnowledge.contexts`, `contextDirectories`, `trackedContextPaths`, JSON context parsing, and `ValidDispatch.contexts`. Keep the existing tracked-blob/no-follow/clean-policy trust checks for Team prompts.

Run:

```bash
./.pi/extensions/pi-team-harness/node_modules/.bin/tsx --test --test-concurrency=1 .pi/extensions/pi-team-harness/test/charters.test.ts .pi/extensions/pi-team-harness/test/supervisor.test.ts .pi/extensions/pi-team-harness/test/index-dispatch-start.test.ts
```

Expected: PASS after context removal and prompt rewrite.

- [ ] **Step 8: Reconcile every old field/role/base reference**

Run:

```bash
rg -n 'feature|Feature|rpg-toolkit-member|rpg-api-member|game-dev-member|director-assets|toolkit-fixer|origin/development|pullRequestTarget: "development"|on behalf of KirkDiggler' .pi/extensions/pi-team-harness README.md scripts/verify-pi-team-harness.mjs
```

Every production match must be removed or be an explicit historical test fixture asserting refusal. Update README's guided dispatch, repository routes, status, canonical trust, and signature sections to the new vocabulary.

- [ ] **Step 9: Run full harness verification**

```bash
npm --prefix .pi/extensions/pi-team-harness test
npm --prefix .pi/extensions/pi-team-harness run typecheck
node scripts/verify-pi-team-harness.mjs
bash tests/bootstrap-contract.sh
bash scripts/verify-workspace.sh
```

Expected: all pass with no model call. Also run one RPC preview for a Platform/Infra game-dev issue and one Monster AI toolkit fixture; verify the preview shows Team role + repository route and performs no mutation.

- [ ] **Step 10: Commit and publish**

Use coherent commits for Team routing, actor/status/signatures, and context removal; publish one ready PR closing the slice. Post a `platform` signed checkpoint after the dynamic signer is live. After human merge, verify the exact game-dev main source through `node scripts/verify-pi-team-harness.mjs` before Task 5.

---

### Task 5: Make rpg-toolkit module isolation unavoidable and enforceable

**Issue/PR:** New rpg-toolkit `Build` slice under #236, Team `Platform`, Area `Infra`.

**Files:**
- Create symlink: `AGENTS.md` → `CLAUDE.md`
- Create symlinks: `rulebooks/dnd5e/AGENTS.md`, `rulebooks/dnd5e/character/AGENTS.md`, `tools/spatial/AGENTS.md` → sibling `CLAUDE.md`
- Create: `scripts/check-module-scope.sh`
- Create: `tests/module-scope-contract.sh`
- Modify: `CLAUDE.md`
- Modify: `Makefile`
- Modify: `.github/workflows/ci-optimized.yml`

**Interfaces:**
- Consumes: repository/module load chain from game-dev.
- Produces: `scripts/check-module-scope.sh --stdin0`, which maps each changed path to its deepest enclosing tracked `go.mod`, prints the selected module or docs-only result, and exits 1 for multiple modules.

- [ ] **Step 1: Write the module-scope contract test first**

Create executable `tests/module-scope-contract.sh` with this complete harness:

```bash
#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SCOPE="$ROOT/scripts/check-module-scope.sh"
fail() { printf 'FAIL: %s\n' "$*" >&2; exit 1; }

expect_pass() {
  local label=$1
  shift
  local output
  output="$(printf '%s\0' "$@" | "$SCOPE" --stdin0)" || fail "$label unexpectedly failed"
  printf 'PASS: %s — %s\n' "$label" "$output"
}

expect_fail() {
  local label=$1
  shift
  local output
  if output="$(printf '%s\0' "$@" | "$SCOPE" --stdin0 2>&1)"; then
    fail "$label unexpectedly passed: $output"
  fi
  grep -Fq 'rulebooks/dnd5e' <<<"$output" || fail "$label omitted provider module"
  grep -Fq 'rulebooks/dnd5e/resolution' <<<"$output" || fail "$label omitted resolution module"
  printf 'PASS: %s refused both modules\n' "$label"
}

expect_invalid() {
  local output code
  set +e
  output="$(printf '%s\0' '../outside.go' | "$SCOPE" --stdin0 2>&1)"
  code=$?
  set -e
  [ "$code" -eq 2 ] || fail "unsafe path returned $code instead of 2"
  grep -Fq 'invalid changed path' <<<"$output" || fail "unsafe path lacked bounded diagnostic"
  printf 'PASS: unsafe path refused\n'
}

expect_pass docs-only docs/adr/DECISIONS.md CLAUDE.md
expect_pass resolution-only rulebooks/dnd5e/resolution/strike.go rulebooks/dnd5e/resolution/go.mod
expect_pass provider-plus-root rulebooks/dnd5e/damage/damage.go docs/adr/DECISIONS.md scripts/check-no-legacy-attack.sh
expect_fail provider-and-resolution rulebooks/dnd5e/damage/damage.go rulebooks/dnd5e/resolution/strike.go
expect_invalid
```

The failing assertion requires diagnostics naming both module roots.

Run:

```bash
bash tests/module-scope-contract.sh
```

Expected: FAIL because the scope script does not exist.

- [ ] **Step 2: Implement deepest-module detection**

Create `scripts/check-module-scope.sh` with:

```bash
#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mode="${1:-}"
[ "$mode" = "--stdin0" ] || { echo 'usage: check-module-scope.sh --stdin0' >&2; exit 2; }
refuse() { printf 'invalid changed path: %s\n' "$1" >&2; exit 2; }

modules=()
while IFS= read -r -d '' changed; do
  [ -n "$changed" ] || continue
  case "$changed" in
    /*|../*|*/../*|*/..|*//*) refuse "$changed" ;;
  esac
  cursor="$(dirname "$changed")"
  while [ "$cursor" != "." ] && [ "$cursor" != "/" ]; do
    if [ -f "$repo_root/$cursor/go.mod" ]; then
      modules+=("$cursor")
      break
    fi
    cursor="$(dirname "$cursor")"
  done
done

mapfile -t unique < <(printf '%s\n' "${modules[@]:-}" | sed '/^$/d' | sort -u)
case "${#unique[@]}" in
  0) echo 'module scope: docs/repository-only' ;;
  1) echo "module scope: ${unique[0]}" ;;
  *) printf 'multiple versioned modules changed:\n' >&2; printf '  %s\n' "${unique[@]}" >&2; exit 1 ;;
esac
```

Use filesystem `go.mod` boundaries from the checked-out commit. Reject absolute paths, `..` traversal, empty path components, and paths outside the repository before module lookup.

- [ ] **Step 3: Run red/green fixture proof**

```bash
bash tests/module-scope-contract.sh
printf '%s\0' rulebooks/dnd5e/damage/damage.go rulebooks/dnd5e/resolution/strike.go | ./scripts/check-module-scope.sh --stdin0
```

Expected: contract PASS; deliberate command exits 1 and names both modules.

- [ ] **Step 4: Put module law first and add AGENTS links**

Rewrite the top of `CLAUDE.md` in this order:

1. multi-module definition: nearest `go.mod` owns the release;
2. one versioned module per implementation PR;
3. one in-flight PR per module;
4. provider merge/tag before consumer pin;
5. local `replace`/`go.work` allowed only uncommitted;
6. exact module-local verification commands;
7. layer/semantic-owner guidance and progressive docs pointers.

Remove slash-command claims, personal paths, old completed-module/issue inventories, and stale counts. Preserve the current play/composition/host-seam laws, ADR/DECISIONS pointer, TDD, and no-rules-in-host boundaries.

Create all four symlinks and verify each with `readlink`.

- [ ] **Step 5: Wire the guard into local and CI checks**

Add Make target:

```make
module-scope-test:
	@bash tests/module-scope-contract.sh
```

In `.github/workflows/ci-optimized.yml`, add a pull-request-only step in the `changes` job after checkout:

```yaml
- name: Enforce one versioned module per PR
  if: github.event_name == 'pull_request'
  run: |
    git diff --name-only -z "origin/${{ github.base_ref }}...HEAD" |
      ./scripts/check-module-scope.sh --stdin0
    bash tests/module-scope-contract.sh
```

Keep `fetch-depth: 0`. Do not add the guard to main pushes, where a merge commit may legitimately aggregate already-reviewed module PRs over time.

- [ ] **Step 6: Verify repository guidance and CI syntax**

```bash
bash tests/module-scope-contract.sh
bash -n scripts/check-module-scope.sh
make module-scope-test
for path in AGENTS.md rulebooks/dnd5e/AGENTS.md rulebooks/dnd5e/character/AGENTS.md tools/spatial/AGENTS.md; do test -L "$path"; test "$(readlink "$path")" = CLAUDE.md; done
! rg -n '/home/(kirk|frank)|/bugfix|/feature|Completed Modules|Pending Work' CLAUDE.md
git diff --check
```

Validate workflow YAML with the repository's available action/yaml checker. Run the test suite only for the scripts/docs slice; no module Go source changes are expected.

- [ ] **Step 7: Commit and publish**

```bash
git add AGENTS.md CLAUDE.md Makefile .github/workflows/ci-optimized.yml scripts/check-module-scope.sh tests/module-scope-contract.sh rulebooks/dnd5e/AGENTS.md rulebooks/dnd5e/character/AGENTS.md tools/spatial/AGENTS.md
git commit -m 'feat: enforce toolkit module-scoped pull requests'
git push -u origin "$(git branch --show-current)"
```

Open a ready PR closing the slice. In the PR evidence, cite #1144 as the red fixture shape and #1146/#1148 as the corrected provider/consumer sequence. After merge, verify the CI guard on one docs-only PR or fixture and one deliberate two-module test input.

---

### Task 6: Retire legacy shared progress and repository-role policy

**Issue/PR:** New rpg-project `Build` slice under #236, Team `Cross-team`, Area `Infra`. Start only after Tasks 4 and 5 are merged and verified.

**Files:**
- Create: `docs/teams/legacy-state-migration.md`
- Modify: `CLAUDE.md`
- Modify: `docs/teams/roles/README.md`
- Modify: `docs/teams/roles/working-agreements.md`
- Modify: `.opencode/agents/*.md`
- Modify: `scripts/verify-opencode.sh`
- Modify: `scripts/verify-team-workflow.sh`
- Delete: `sessions/active.md`
- Delete: legacy canonical role directories under `docs/teams/roles/` except `platform`, `ui-ux`, `assets`, `monster-ai`, `cross-team`
- Delete after individual rejection: `.opencode/skills/project-board-workflow/SKILL.md`
- Do not modify: runtime-specific skills in other repositories

**Interfaces:**
- Consumes: game-dev Team runtime and toolkit AGENTS/module guard already on main.
- Produces: only five canonical standing Team charters; no tracked shared session/progress/context authority.

- [ ] **Step 1: Capture a complete pre-delete manifest**

Generate the exact source inventory:

```bash
git ls-tree -r --name-only HEAD sessions docs/teams/roles .opencode/skills | sort > /tmp/legacy-team-state-paths.txt
```

Create `docs/teams/legacy-state-migration.md` with one row per removed source file and columns:

```text
Source | Classification | Canonical destination/evidence | Action
```

Allowed classifications are exactly: `shared-live`, `local-only`, `repository-law`, `architecture`, `historical-rationale`, `actionable-debt`, `personal`, `stale-duplicate`.

For every row, cite a concrete Project issue/PR, Team/repository doc, ADR/design path, or `git history only`. A row may not use an unspecified deferred-review disposition or omit its destination.

- [ ] **Step 2: Reconcile the latest tracked session file before removal**

For each `sessions/active.md` section:

- current journey/slice/status → verify/update Project 19;
- accepted decision or blocker → verify/update owning issue/PR;
- machine-local path/state → place only in the current operator's ignored `active.md` when still useful;
- historical narrative → cite Git history in the manifest.

Post shared updates before deleting the file. Record every destination URL in the manifest. Then remove `sessions/active.md` with `git rm`.

- [ ] **Step 3: Classify and remove role contexts/prompts**

Use the manifest to promote current reusable repository laws into the owning repo's AGENTS/ADR/README before deletion. Open an owning Project issue for live debt that lacks one. Do not copy stale progress into Team charters.

Remove every legacy canonical role directory after its files have manifest rows. Final `docs/teams/roles/` children must be:

```text
README.md
working-agreements.md
platform/prompt.md
ui-ux/prompt.md
assets/prompt.md
monster-ai/prompt.md
cross-team/prompt.md
```

- [ ] **Step 4: Repoint OpenCode runtime profiles without importing legacy policy**

Keep model/permission profiles when useful, but change their canonical pointers by Team:

| Runtime adapter | Team prompt |
|---|---|
| `platform-lead`, `rpg-toolkit-member`, `rpg-api-member`, `rpg-api-protos-member`, `rpg-deployment-member`, `game-dev-member`, `toolkit-fixer`, `api-fixer` | `docs/teams/roles/platform/prompt.md` |
| `ui-lead`, `ui-web-member`, `web-fixer` | `docs/teams/roles/ui-ux/prompt.md` |
| `assets-lead`, `assets-web-member`, `rpg-game-assets-member` | `docs/teams/roles/assets/prompt.md` |
| `independent-gate`, `explore`, `janitor` | `docs/teams/roles/cross-team/prompt.md` |

Add a `monster-ai` runtime adapter pointing only to `docs/teams/roles/monster-ai/prompt.md`. Adapter files must say they are execution profiles, not standing roles. Remove references to context directories, director overlays, and repo-member prompts.

- [ ] **Step 5: Reject rather than migrate the stale board skill**

The existing `.opencode/skills/project-board-workflow/SKILL.md` uses `Feature`, fixed Kirk authority/signatures, and shared-session recovery. Record those concrete rejection reasons in the migration manifest, then remove it. Do not create a canonical replacement skill; Project 19 invariants remain in AGENTS/working agreements.

Leave `rpg-api/.claude/skills/rpg-api-development` unchanged and explicitly out of scope; it is not exposed by the new canonical catalog.

- [ ] **Step 6: Rewrite verifiers for the clean end state**

`scripts/verify-team-workflow.sh` must now fail on:

```bash
[ -e sessions/active.md ]
find docs/teams/roles -type d -name context -print -quit | grep -q .
find .agents/skills -name SKILL.md -print -quit | grep -q .
test -e .opencode/skills/project-board-workflow/SKILL.md
rg -n 'sessions/active.md|docs/teams/roles/(director|rpg-|.*fixer|explore|janitor|project-manager)' CLAUDE.md docs/teams .opencode/agents scripts/verify-opencode.sh
```

Invert each expression into a clear failure message. Update `verify-opencode.sh` to assert the Team prompt mapping above, the Monster AI adapter, and no dependency on a board skill.

- [ ] **Step 7: Run the clean-state verification**

```bash
./scripts/verify-team-workflow.sh
./scripts/verify-opencode.sh
git diff --check
! rg -n 'sessions/active.md|on behalf of KirkDiggler|Feature' CLAUDE.md docs/teams/roles .opencode/agents
```

Expected: all pass. `on behalf of KirkDiggler` may remain only in historical GitHub text outside these canonical paths, never in active policy.

- [ ] **Step 8: Commit and publish**

Commit the migration manifest, promoted canonical facts, adapter updates, and deletions together so no commit leaves unresolved role pointers. Open a ready PR closing the slice, with the manifest as the primary review surface. After human merge, run game-dev's `scripts/verify-workspace.sh` against synchronized rpg-project main.

---

### Task 7: Run the real Codex contributor pilot and the journey retro

**Issue/PR:** Use journey #236 for the integrated Verify checkpoint. Do not create a repository diff solely for the walkthrough. Coordinate with the assignee of Composable Attack Damage journey #232 and its actual next slice.

**Evidence locations:**
- Project 19 journey #236 learning log/checkpoint
- The selected real slice issue under #232
- PR #238 plan/design amendments when the pilot changes the contract
- Each collaborator's ignored `rpg-project/active.md` locally

**Interfaces:**
- Consumes: merged Tasks 2–6.
- Produces: evidence for design §16 and a retro decision on propagation/checkpoint promotion.

- [ ] **Step 1: Synchronize the collaborator workspace without touching local work**

From the collaborator's game-dev root, fetch only; do not reset or clean. Verify:

```bash
gh api user --jq .login
git fetch origin
git -C rpg-project fetch origin
git -C rpg-toolkit fetch origin
```

Expected login: `dammitbilly0ne` for the named pilot. If another collaborator performs the pilot, record that login rather than impersonating Dammit.

- [ ] **Step 2: Establish local continuity from real current work**

Have the human state one future-facing direction in their own words. The coordinating agent writes it verbatim to ignored `rpg-project/active.md` and records the real journey, slice, PR, worktree, branch, observed HEAD, upstream state, and dirty summary.

Verify:

```bash
git -C rpg-project check-ignore -v active.md
git -C rpg-project status --short -- active.md
```

Expected: ignored by `/active.md` and absent from tracked status.

- [ ] **Step 3: Start a fresh Codex session at game-dev root**

Do not preload Claude memory or any legacy `.claude/skills`/`.opencode/skills`. Ask only for orientation and resumption. Capture evidence that the session:

1. loaded game-dev AGENTS;
2. read rpg-project AGENTS;
3. derived the actual `gh` login;
4. found assigned journey #232 and its current slice/PR;
5. read local user direction;
6. inspected the referenced Git work without mutation;
7. read rpg-toolkit root and nearest module AGENTS; and
8. stated one-module-per-PR/provider-first rules before proposing edits.

The evidence must distinguish automatic root load from explicit subsequent reads.

- [ ] **Step 4: Exercise stale/local disagreement safely**

Change only an observed SHA in `active.md` to the immediately previous known SHA while leaving Git untouched. Ask the same Codex session to re-orient. It must report that Git wins and update the observation only after the human confirms. Restore the accurate local note; do not reset any repository.

- [ ] **Step 5: Publish a Team-derived shared checkpoint when collaboration requires it**

If the real slice reaches a blocker, review transition, decision, or handoff during the pilot, publish through the owning runtime using the active Team signature and authenticated login. Do not publish a checkpoint merely to satisfy this plan; if no collaboration boundary occurs, record that as retro evidence.

- [ ] **Step 6: Run deterministic workspace verification on the collaborator machine**

```bash
bash game-dev/tests/contributor-startup-contract.sh
bash game-dev/tests/bootstrap-contract.sh
bash game-dev/scripts/verify-workspace.sh
bash rpg-toolkit/tests/module-scope-contract.sh
```

Expected: all pass. Record exact command output and environment/runtime names without publishing secrets or licensed asset paths.

- [ ] **Step 7: Run the retro against the acceptance questions**

Record answers on #236:

- Did the collaborator find assigned work without requesting assignment?
- Did the root bootloader cause every required explicit read?
- Did Team role and repository law stay distinct?
- Did local continuity add value beyond Project 19 and Git?
- Which local fact, if any, needed promotion to GitHub?
- Did the module guard prevent or clarify a cross-module PR?
- Should AGENTS links propagate to the remaining repositories now?
- Did any rejected legacy skill prove necessary?

Every answer must cite observed evidence. A “not exercised” result remains open rather than being rounded up to success.

- [ ] **Step 8: Amend PR #238 and close the journey only on proof**

If the pilot changes the contract, amend brainstorm/design/plan on PR #238 before implementation PRs ratify. Mark journey #236 Done only when every design §16 item has evidence. Otherwise leave it In Progress with the next concrete proof. The human director decides whether PR #238 merges alongside the final implementation state.

---

## Plan self-review checklist

- [ ] Every design section has an implementing task or explicit pilot evidence.
- [ ] Project 19 shared state, local continuity, and Git authority remain separate in every task.
- [ ] Compatibility is additive before old role/session inputs are removed.
- [ ] Team roles and repository routing are separate interfaces in the Pi harness.
- [ ] The canonical skill catalog stays empty; the stale OpenCode skill is rejected, not moved.
- [ ] Toolkit multi-module failure is proven red before the CI guard is trusted green.
- [ ] No task asks an agent to reset, clean, stash, force-push, merge, or impersonate another operator.
- [ ] Each repository diff has one slice issue, branch, ready PR, deterministic checks, and signed checkpoint.
