# OpenCode Team Workflow Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Establish a portable, project-scoped OpenCode team workflow whose canonical role policy lives in `rpg-project`, whose workstation setup lives in `game-dev`, and whose first live proof is recoverable from GitHub alone.

**Architecture:** `rpg-project` first receives provider-neutral role charters and overlays, then project-scoped OpenCode configuration and thin adapters that bind those charters to OpenAI model and permission profiles. `game-dev` consumes the resulting project contract through an exact seven-repository bootstrap list, while a clean-slate Verify issue proves global configuration is never modified. The equipment pilot begins only after that environment proof and the three existing contract decisions are recorded; it is a separate plan, not implementation detail in this document.

**Tech Stack:** Markdown role charters, OpenCode 1.18.4 JSONC and Markdown agents, Bash, `jq`, Git, GitHub CLI, Project 19.

## Global Constraints

- Work only in the repository named by each task; no worktree path is a review surface.
- Start every executable task with one repository issue, one Project 19 item, a fresh branch from `main`, and a ready, non-draft PR; use one issue per PR.
- Project 19 fields are exact: Status `Todo` then `In Progress` then `In Review` then `Done`; Team `UI/UX`, `Platform`, `Assets`, or `Cross-team`; Feature `Party Assembles`, `Class Kits`, `The Dungeon`, `Game Screen`, `Capstone`, `Shelf`, or `Infra`; Kind `Build`, `Fix`, `Verify`, `Learn`, or `Decide`.
- Every GitHub comment and PR body written by a worker ends with `— <role>, on behalf of KirkDiggler`.
- Ready for review is not merge-ready. An independent Sol gate precedes the human merge; Kirk alone merges. Verify auto-deploy completion after every merge where the target repository deploys.
- Project 19 and GitHub issue, PR, branch, and checkpoint comments are the only durable task state. `task_id` is optional live continuity, never recovery state.
- Use OpenCode 1.18.4 mechanics only. Project config deep-merges global config. Use only supported keys such as `model`, `small_model`, `default_agent`, `references`, `mcp`, `agent`, and `permission`; do not introduce `project`, `workspace`, `agents`, `mcps`, `permissions`, `charter`, `overlay`, `contextPath`, or `checkpoints` config keys.
- `opencode.jsonc` contains `$schema: "https://opencode.ai/config.json"`; adapters live at `.opencode/agents/`; every adapter frontmatter uses singular `permission`.
- Every adapter body explicitly requires reading its listed canonical charter and overlay before action. Adapters bind model and permissions only; they do not duplicate charter policy and never use `{file:...}` Markdown splicing.
- Use only OpenAI models named in this plan. Do not configure, invoke, or dispatch an Anthropic, Claude, or Sonnet model.
- Do not add a plugin, daemon, checkpoint JSON schema, duplicate tracker, dry-run rewrite, or test framework.
- The equipment feature and proto implementation are out of scope. This plan records only the post-environment-proof workflow gate and the prerequisite decisions for `rpg-api-protos#187`.

---

## Reviewable Units

| Task | Repository | Dependency | Project 19 fields | Deliverable |
|---|---|---|---|---|
| 1 | `rpg-project` | none | Cross-team / Infra / Build / Todo | Canonical charters, overlays, and roles index |
| 2 | `rpg-project` | Task 1 merged | Cross-team / Infra / Build / Todo | OpenCode config, adapters, workflow skill, red/green verifier |
| 3 | `game-dev` | Task 2 merged | Cross-team / Infra / Build / Todo | Seven-repo portable bootstrap and shell-only tests |
| 4 | `rpg-project` issue only | Task 3 merged | Cross-team / Infra / Verify / Todo | Fresh-machine clean-slate evidence; no code PR |
| 5 | existing `rpg-api-protos#187` plus new planning issue | Task 4 passed | Platform / Class Kits / Decide / Todo | Decision checkpoint, separate equipment plan, kill/replace/gate proof |

## File Structure

### Task 1 canonical role system

- Create: `docs/teams/roles/director/overlays/ui-ux.md` — UI/UX lane-specific director overlay.
- Create: `docs/teams/roles/director/overlays/platform.md` — Platform lane-specific director overlay.
- Create: `docs/teams/roles/director/overlays/assets.md` — Assets lane-specific director overlay.
- Create: `docs/teams/roles/rpg-dnd5e-web-member/overlays/ui-ux.md` — shared web charter's UI/UX ownership seam.
- Create: `docs/teams/roles/rpg-dnd5e-web-member/overlays/assets.md` — shared web charter's Assets ownership seam.
- Create: `docs/teams/roles/rpg-deployment-member/prompt.md` and `context/{active-work,dependencies,discoveries,lessons-learned,patterns}.json` — deployment standing-member charter and minimal persistent context.
- Create: `docs/teams/roles/rpg-game-assets-member/prompt.md` and `context/{active-work,dependencies,discoveries,lessons-learned,patterns}.json` — assets standing-member charter and minimal persistent context.
- Create: `docs/teams/roles/independent-gate/prompt.md` and `context/{active-work,dependencies,discoveries,lessons-learned,patterns}.json` — read-first adversarial gate charter and minimal evidence context.
- Create: `docs/teams/roles/explore/prompt.md` and `context/{active-work,dependencies,discoveries,lessons-learned,patterns}.json` — read-only orientation charter and minimal discovery context.
- Modify: `docs/teams/roles/README.md` — roster, pod ownership, web seam, and OpenCode-adapter relationship.

### Task 2 OpenCode runtime

- Create: `AGENTS.md` — Git symlink to `CLAUDE.md`, not a copied file.
- Create: `opencode.jsonc` — project config, six sibling references, disabled inherited MCPs, portable Chrome configuration, built-in overrides.
- Create: `.opencode/agents/{ui-lead,platform-lead,assets-lead,rpg-toolkit-member,rpg-api-member,rpg-api-protos-member,rpg-deployment-member,rpg-game-assets-member,ui-web-member,assets-web-member,toolkit-fixer,api-fixer,web-fixer,independent-gate,explore,janitor}.md` — exactly the 16 thin adapters in the adapter manifest below.
- Create: `.opencode/skills/project-board-workflow/SKILL.md` — issue/board/PR/checkpoint procedure.
- Create: `scripts/verify-opencode.sh` — no-model-call structural verifier.

### Task 3 portable workspace

- Create: `scripts/workspace-repos.sh` — sole ordered source of seven workspace repository names.
- Create: `tests/bootstrap-contract.sh` — shell contract test run before bootstrap changes.
- Create: `scripts/verify-workspace.sh` — post-bootstrap local verifier.
- Modify: `bootstrap.sh` — source repository list, clone/fetch all seven, and issue a warning-only OpenCode availability check.
- Modify: `.gitignore` — ignore all seven nested clones.
- Modify: `README.md` and `CLAUDE.md` — seven-repository orientation; describe OpenCode as project-scoped and non-mutating.

### Task 4 clean-slate proof

- Create: no repository files.
- Create: one Project 19 Verify issue in `rpg-project`; the issue comment stream is the evidence record.

### Task 5 equipment workflow proof

- Create: no equipment or proto code under this plan.
- Create later: one separate `rpg-project` equipment-plan issue and ready plan PR after the three `rpg-api-protos#187` decisions are resolved.

## Adapter Manifest

Each file below has YAML frontmatter with `description`, `mode`, `model`,
`variant`, and singular `permission` keys. Its body names every canonical file
in that row literally, requires reading those files before any action, states
that it does not restate or override charter policy, and requires a GitHub
checkpoint when blocked, when a handoff is required, and before a dispatched
task ends. Permission maps use YAML nesting, with `"*"` preceding specific
allow or deny patterns because the last matching rule wins.

| Adapter file | Mode / model / variant | Canonical files read before action | Permission binding |
|---|---|---|---|
| `ui-lead.md` | primary / `openai/gpt-5.6-sol-fast` / `xhigh` | `docs/teams/roles/director/prompt.md`; `docs/teams/roles/director/field-notes.md`; `docs/teams/roles/director/overlays/ui-ux.md` | `edit: deny`; `bash: {"*": "deny", "gh *": "allow"}`; `task: {"*": "deny", "ui-web-member": "allow", "web-fixer": "allow", "explore": "allow", "independent-gate": "allow", "janitor": "allow"}` |
| `platform-lead.md` | primary / `openai/gpt-5.6-sol-fast` / `xhigh` | `docs/teams/roles/director/prompt.md`; `docs/teams/roles/director/field-notes.md`; `docs/teams/roles/director/overlays/platform.md` | `edit: deny`; `bash: {"*": "deny", "gh *": "allow"}`; `task: {"*": "deny", "rpg-toolkit-member": "allow", "rpg-api-member": "allow", "rpg-api-protos-member": "allow", "rpg-deployment-member": "allow", "toolkit-fixer": "allow", "api-fixer": "allow", "explore": "allow", "independent-gate": "allow", "janitor": "allow"}` |
| `assets-lead.md` | primary / `openai/gpt-5.6-sol-fast` / `xhigh` | `docs/teams/roles/director/prompt.md`; `docs/teams/roles/director/field-notes.md`; `docs/teams/roles/director/overlays/assets.md` | `edit: deny`; `bash: {"*": "deny", "gh *": "allow"}`; `task: {"*": "deny", "rpg-game-assets-member": "allow", "assets-web-member": "allow", "web-fixer": "allow", "explore": "allow", "independent-gate": "allow", "janitor": "allow"}` |
| `rpg-toolkit-member.md` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/rpg-toolkit-member/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `rpg-api-member.md` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/rpg-api-member/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `rpg-api-protos-member.md` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/rpg-api-protos-member/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `rpg-deployment-member.md` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/rpg-deployment-member/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `rpg-game-assets-member.md` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/rpg-game-assets-member/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `ui-web-member.md` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/rpg-dnd5e-web-member/prompt.md`; `docs/teams/roles/rpg-dnd5e-web-member/overlays/ui-ux.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `assets-web-member.md` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/rpg-dnd5e-web-member/prompt.md`; `docs/teams/roles/rpg-dnd5e-web-member/overlays/assets.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `toolkit-fixer.md` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/toolkit-fixer/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `api-fixer.md` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/api-fixer/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `web-fixer.md` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/web-fixer/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `independent-gate.md` | subagent / `openai/gpt-5.6-sol` / `max` | `docs/teams/roles/independent-gate/prompt.md` | `edit: allow`; `bash: {"*": "allow", "git commit *": "deny", "git push *": "deny", "git merge *": "deny", "gh pr merge *": "deny"}`; `task: deny` |
| `explore.md` | subagent / `openai/gpt-5.6-luna` / `medium` | `docs/teams/roles/explore/prompt.md` | `edit: deny`; `bash: allow`; `task: deny` |
| `janitor.md` | subagent / `openai/gpt-5.6-luna` / `low` | `docs/teams/roles/janitor/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |

The primary agents are the only main-session entry points. Members, fixers, gate, explore, and janitor run underneath them. An active session may resume its `task_id`; replacement after a failure must be reconstructed from the issue, branch, PR, and checkpoint only.

### Task 1: Canonical Provider-Neutral Role System

**Files:**
- Create every Task 1 path listed in **File Structure**.
- Modify: `docs/teams/roles/README.md`.

**Interfaces:**
- Consumes: `docs/teams/roles/director/{prompt,field-notes}.md`; the four existing standing-member charters; existing fixer and janitor charters.
- Produces: canonical, provider-neutral files consumed verbatim by the Task 2 adapter manifest; the role index becomes the roster authority.

- [ ] **Step 1: Create the task record before creating a branch**

Create an `rpg-project` issue titled `OpenCode canonical role system` with the goal, charter list, and the four-question done-gate. Add it to Project 19 and set exactly `Team=Cross-team`, `Feature=Infra`, `Kind=Build`, and `Status=Todo`; then set `Status=In Progress`. Post `WORK SESSION STARTED` with the issue URL, planned branch, and next checkpoint, ending `— cross-team Terra role implementer, on behalf of KirkDiggler`.

- [ ] **Step 2: Create the fresh issue branch**

```bash
git fetch origin
git switch main
git pull --ff-only
git switch -c docs/ROLE_ISSUE-opencode-canonical-roles
```

Expected: `git status --short --branch` reports only `## docs/ROLE_ISSUE-opencode-canonical-roles`, where `ROLE_ISSUE` is the issue number created in Step 1.

- [ ] **Step 3: Write the charter acceptance checks before charter content**

Create `scripts` only if this repository already has a suitable documentation-check location; otherwise run these exact review commands manually and record their output in the issue checkpoint:

```bash
test -f docs/teams/roles/director/overlays/ui-ux.md
test -f docs/teams/roles/director/overlays/platform.md
test -f docs/teams/roles/director/overlays/assets.md
test -f docs/teams/roles/rpg-dnd5e-web-member/overlays/ui-ux.md
test -f docs/teams/roles/rpg-dnd5e-web-member/overlays/assets.md
for role in rpg-deployment-member rpg-game-assets-member independent-gate explore; do
  test -f "docs/teams/roles/$role/prompt.md"
  for context in active-work dependencies discoveries lessons-learned patterns; do
    jq -e '.items == []' "docs/teams/roles/$role/context/$context.json" >/dev/null
  done
done
```

Expected before adding files: failure on the first missing overlay or charter path.

- [ ] **Step 4: Write the three director overlays with these exact responsibilities**

Create the following minimal files. They extend the director charter; they do not repeat its no-hands-on-work policy.

```markdown
<!-- docs/teams/roles/director/overlays/ui-ux.md -->
# Director Overlay: UI/UX

Run the UI/UX lane with its human collaborator. Route screens, HUD, interaction,
accessibility, responsive Discord viewport, and fixture-to-live presentation to
`ui-web-member`; require screenshots or recordings as the evidence artifact.
Escalate missing server facts as Platform work; never authorize client-side game
calculations or state gating. Coordinate shared-web work with the Assets overlay
through linked issues or one explicitly joint issue.
```

```markdown
<!-- docs/teams/roles/director/overlays/platform.md -->
# Director Overlay: Platform

Run the Platform lane with its human collaborator. Sequence contract work before
API consumers and API work before web consumers. Route rules to rpg-toolkit,
shape to rpg-api-protos, orchestration to rpg-api, and release delivery to
rpg-deployment. Require wire-state evidence for platform claims and terminal
deployment verification after applicable human merges.
```

```markdown
<!-- docs/teams/roles/director/overlays/assets.md -->
# Director Overlay: Assets

Run the Assets lane with its human collaborator. Route licensed source, conversion,
contract-tree, manifest, and performance work to rpg-game-assets; route web-side
model loading, environment rendering, animation playback, and 3D evidence to
`assets-web-member`. Require multi-angle evidence and preserve the private asset
license boundary; coordinate shared-web work with the UI/UX overlay.
```

- [ ] **Step 5: Write the shared-web overlays and new standing-member charters**

Create the web overlays with these exact ownership boundaries:

```markdown
<!-- docs/teams/roles/rpg-dnd5e-web-member/overlays/ui-ux.md -->
# UI/UX Web Overlay

Own screens, HUD, UX flows, accessibility, responsive Discord viewports, and
fixture-to-live presentation. Read the shared web charter first. Render server
state and send intent; a missing server fact is an upstream issue, never a local
game calculation. Attach screenshot or recording evidence to the owning issue.
```

```markdown
<!-- docs/teams/roles/rpg-dnd5e-web-member/overlays/assets.md -->
# Assets Web Overlay

Own model loading, props and environment rendering, animation playback, 3D
performance, appearance seams, and multi-angle visual evidence. Read the shared
web charter first. Consume the game-assets contract tree; do not handle raw
licensed source or move asset-pipeline policy into the web repository.
```

Create `rpg-deployment-member/prompt.md` as a first-person standing expert charter that owns `rpg-deployment` auto-deploy pipeline, `nginx-http.conf`, release sequencing, and watching `Deploy RPG Platform` to terminal success, while refusing toolkit/API/proto business logic. Create `rpg-game-assets-member/prompt.md` as a first-person standing expert charter that owns the private Synty license boundary, FBX-to-GLB promotion, `harness/models/synty/`, manifests, shipped-asset budgets, and handoff to `assets-web-member`, while refusing web renderer code. Both charters must include issue-to-merge ownership, living-doc ownership, lane pushback, TDD/repo-local gate discipline, the four-question done-gate, no merge authority, and a permission/auth blocker stop-and-report clause.

- [ ] **Step 6: Write independent-gate and explore charters**

Create `independent-gate/prompt.md` requiring a fresh context and independent worktree, adversarial review of diff, issue, claims, tests, and evidence; permitting reversible mutation experiments; forbidding fixes, commits, pushes, merges, and self-declared merge-ready; requiring findings to route to the original implementer and a fresh regate after remediation. Create `explore/prompt.md` requiring read-only orientation, a concise evidence-backed report, no edit, no decision, no dispatch, and a GitHub-visible blocker report if access prevents the requested check.

- [ ] **Step 7: Create minimal context files using one exact shape**

For each of `rpg-deployment-member`, `rpg-game-assets-member`, `independent-gate`, and `explore`, create the five `context/*.json` files listed above with exactly:

```json
{
  "items": []
}
```

- [ ] **Step 8: Update the roles index**

Update `docs/teams/roles/README.md` to list three director overlays, six standing owners (`rpg-toolkit`, `rpg-api`, `rpg-api-protos`, `rpg-deployment`, `rpg-game-assets`, and shared `rpg-dnd5e-web` split through the two overlays), the three fixers, independent gate, explore, and janitor. State that charters and overlays are provider-neutral canonical policy and that `.opencode/agents/*.md` only bind those documents to runtime model and permission profiles.

- [ ] **Step 9: Run the charter acceptance checks green**

Run the Step 3 command again.

Expected: exit code `0`; every new context file parses as `{"items":[]}` and every required charter/overlay exists.

- [ ] **Step 10: Review, gate, and publish the ready PR**

Run `git diff --check`, inspect `git diff -- docs/teams/roles`, self-review all changed charter boundaries against `ideas/opencode-team-workflow/design.md` sections 2, 4, and 7, then request `independent-gate`. Correct findings through the original Terra worker and request a fresh Sol regate. Push, open a ready PR with `Closes #ROLE_ISSUE`, set Project 19 `Status=In Review`, and post the PR URL, verification, blockers, and explicit next action to the issue. A human merges after the gate; then set `Status=Done` and verify deployment only if this repository later gains an applicable deploy.

- [ ] **Step 11: Commit the reviewable role system**

```bash
git add docs/teams/roles
git commit -m "docs(roles): add provider-neutral OpenCode role system (#ROLE_ISSUE)"
```

Expected: commit succeeds without `--no-verify` and contains only Task 1 paths.

### Task 2: Project-Scoped OpenCode Runtime

**Files:**
- Create every Task 2 path listed in **File Structure**.

**Interfaces:**
- Consumes: the merged Task 1 canonical charters and overlays; OpenCode 1.18.4.
- Produces: `scripts/verify-opencode.sh` as the structural runtime contract; `platform-lead` as the default primary agent; only the 16 adapter names in the manifest.

- [ ] **Step 1: Create the task record before creating a branch**

Create an `rpg-project` issue titled `OpenCode project runtime configuration`, link the merged Task 1 PR, add it to Project 19 with exactly `Team=Cross-team`, `Feature=Infra`, `Kind=Build`, `Status=Todo`, then move it to `In Progress`. Publish the work-session comment and create `feat/RUNTIME_ISSUE-opencode-runtime` from current `main`, where `RUNTIME_ISSUE` is the issue number just created.

- [ ] **Step 2: Write the verifier first**

Create `scripts/verify-opencode.sh`, make it executable, and begin it exactly as follows:

```bash
#!/usr/bin/env bash
set -euo pipefail

fail() { printf 'FAIL: %s\n' "$*" >&2; exit 1; }
require_command() { command -v "$1" >/dev/null 2>&1 || fail "$1 is required"; }
require_file() { [ -f "$1" ] || fail "$1 is missing"; }

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

require_command opencode
require_command jq
require_file opencode.jsonc
[ -L AGENTS.md ] || fail "AGENTS.md must be a symlink"
[ "$(readlink AGENTS.md)" = "CLAUDE.md" ] || fail "AGENTS.md must point to CLAUDE.md"
```

- [ ] **Step 3: Demonstrate the intentional red state**

```bash
./scripts/verify-opencode.sh
```

Expected before creating project configuration: non-zero exit with `FAIL: opencode.jsonc is missing`. Record this exact red output in the runtime issue.

- [ ] **Step 4: Add the root config and symlink**

Create the symlink with `ln -s CLAUDE.md AGENTS.md`. Create `opencode.jsonc` with the following full configuration; it must contain no credentials and must not attempt a Chrome connection:

```jsonc
{
  "$schema": "https://opencode.ai/config.json",
  "model": "openai/gpt-5.6-sol-fast",
  "small_model": "openai/gpt-5.6-luna",
  "default_agent": "platform-lead",
  "references": {
    "../rpg-toolkit": { "description": "Rules engine and rulebook boundary." },
    "../rpg-api": { "description": "Game server and by-key orchestration boundary." },
    "../rpg-api-protos": { "description": "Canonical API contract definitions." },
    "../rpg-dnd5e-web": { "description": "Discord Activity UI that renders data and sends intent." },
    "../rpg-deployment": { "description": "Deployment pipeline and live delivery configuration." },
    "../rpg-game-assets": { "description": "Private game asset library and asset contract tree." }
  },
  "mcp": {
    "hub-tools": { "enabled": false },
    "hub-tools-staging": { "enabled": false },
    "linear": { "enabled": false },
    "goat-drops": { "enabled": false },
    "chrome-devtools": {
      "type": "local",
      "command": ["npx", "-y", "chrome-devtools-mcp@latest", "--browserUrl=http://127.0.0.1:9222", "--no-usage-statistics"],
      "enabled": true
    }
  },
  "agent": {
    "build": { "disable": true },
    "plan": { "disable": true },
    "general": {
      "description": "General implementation worker bound to Terra.",
      "mode": "subagent",
      "model": "openai/gpt-5.6-terra",
      "variant": "high",
      "permission": { "task": "deny" }
    },
    "title": {
      "description": "Hidden title generator bound to Luna.",
      "mode": "primary",
      "model": "openai/gpt-5.6-luna",
      "variant": "low"
    },
    "summary": {
      "description": "Hidden summary generator bound to Luna.",
      "mode": "primary",
      "model": "openai/gpt-5.6-luna",
      "variant": "low"
    },
    "compaction": {
      "description": "Hidden context compactor bound to Luna.",
      "mode": "primary",
      "model": "openai/gpt-5.6-luna",
      "variant": "low"
    }
  }
}
```

- [ ] **Step 5: Create the adapters exactly from the manifest**

Create exactly the 16 files named in **Adapter Manifest**, including the exact mode, model, variant, canonical read list, and permission binding. Do not create an adapter for `general`, `build`, `plan`, `title`, `summary`, or `compaction`: those names are runtime overrides in `opencode.jsonc`. Do not use `.opencode/agent/`, duplicate charter prose, or `{file:...}` in any Markdown adapter.

- [ ] **Step 6: Create the project board workflow skill**

Create `.opencode/skills/project-board-workflow/SKILL.md`:

```markdown
---
name: project-board-workflow
description: Use before starting repository work that needs a Project 19 issue, branch, PR, gate, checkpoint, or merge handoff.
---

# Project 19 Workflow

Before executable work, create or identify one repository issue, add it to
Project 19, set Team, Feature, Kind, and Status, then create one fresh branch
from main. Publish WORK SESSION STARTED on the issue. Keep issue, PR, branch,
and checkpoint comments sufficient for a replacement worker to resume without
session state. Open ready PRs, request an independent gate, route findings to
the original worker, obtain a fresh regate, and leave human merge authority to
Kirk. End every GitHub comment with the active role signature on behalf of
KirkDiggler. After an applicable merge, verify deployment to terminal success
before marking the Project item Done.
```

- [ ] **Step 7: Complete the verifier without any model call**

Append the following complete checks after Step 2's preamble. The verifier inspects resolved configuration and files only; it does not run `opencode run`, does not call a model, and does not require Chrome to accept a connection.

```bash
resolved="$(mktemp)"
trap 'rm -f "$resolved"' EXIT
opencode debug config >"$resolved"

jq -e '.model == "openai/gpt-5.6-sol-fast" and .small_model == "openai/gpt-5.6-luna" and .default_agent == "platform-lead"' "$resolved" >/dev/null || fail "root model profile is wrong"
for repo in rpg-toolkit rpg-api rpg-api-protos rpg-dnd5e-web rpg-deployment rpg-game-assets; do
  jq -e --arg path "../$repo" '.references[$path].description | type == "string" and length > 0' "$resolved" >/dev/null || fail "reference $repo is missing"
done
for server in hub-tools hub-tools-staging linear goat-drops; do
  jq -e --arg server "$server" '.mcp[$server].enabled == false' "$resolved" >/dev/null || fail "MCP $server is not disabled"
done
jq -e '.mcp["chrome-devtools"].enabled == true and .mcp["chrome-devtools"].type == "local" and (.mcp["chrome-devtools"].command | type == "array")' "$resolved" >/dev/null || fail "chrome-devtools configuration is wrong"

agents=(ui-lead platform-lead assets-lead rpg-toolkit-member rpg-api-member rpg-api-protos-member rpg-deployment-member rpg-game-assets-member ui-web-member assets-web-member toolkit-fixer api-fixer web-fixer independent-gate explore janitor)
for agent in "${agents[@]}"; do
  require_file ".opencode/agents/$agent.md"
  opencode debug agent "$agent" >/dev/null || fail "agent $agent does not resolve"
done

for lead in ui-lead platform-lead assets-lead; do
  grep -Fqx 'mode: primary' ".opencode/agents/$lead.md" || fail "$lead mode is wrong"
  grep -Fqx 'model: openai/gpt-5.6-sol-fast' ".opencode/agents/$lead.md" || fail "$lead model is wrong"
  grep -Fqx 'variant: xhigh' ".opencode/agents/$lead.md" || fail "$lead variant is wrong"
  grep -Fqx '  edit: deny' ".opencode/agents/$lead.md" || fail "$lead can edit"
done
for member in rpg-toolkit-member rpg-api-member rpg-api-protos-member rpg-deployment-member rpg-game-assets-member ui-web-member assets-web-member toolkit-fixer api-fixer web-fixer; do
  grep -Fqx 'mode: subagent' ".opencode/agents/$member.md" || fail "$member mode is wrong"
  grep -Fqx 'model: openai/gpt-5.6-terra' ".opencode/agents/$member.md" || fail "$member model is wrong"
  grep -Fqx 'variant: high' ".opencode/agents/$member.md" || fail "$member variant is wrong"
  grep -Fqx '  task: deny' ".opencode/agents/$member.md" || fail "$member can dispatch"
done
grep -Fqx 'model: openai/gpt-5.6-sol' .opencode/agents/independent-gate.md || fail "gate model is wrong"
grep -Fqx 'variant: max' .opencode/agents/independent-gate.md || fail "gate variant is wrong"
grep -Fqx '  task: deny' .opencode/agents/independent-gate.md || fail "gate can dispatch"
grep -Fqx 'model: openai/gpt-5.6-luna' .opencode/agents/explore.md || fail "explore model is wrong"
grep -Fqx 'variant: medium' .opencode/agents/explore.md || fail "explore variant is wrong"
grep -Fqx 'model: openai/gpt-5.6-luna' .opencode/agents/janitor.md || fail "janitor model is wrong"
grep -Fqx 'variant: low' .opencode/agents/janitor.md || fail "janitor variant is wrong"

grep -Fq 'docs/teams/roles/director/prompt.md' .opencode/agents/ui-lead.md || fail "ui lead charter pointer is missing"
grep -Fq 'docs/teams/roles/director/overlays/platform.md' .opencode/agents/platform-lead.md || fail "platform overlay pointer is missing"
grep -Fq 'docs/teams/roles/director/overlays/assets.md' .opencode/agents/assets-lead.md || fail "assets overlay pointer is missing"
grep -Fq 'docs/teams/roles/rpg-dnd5e-web-member/overlays/ui-ux.md' .opencode/agents/ui-web-member.md || fail "UI web overlay pointer is missing"
grep -Fq 'docs/teams/roles/rpg-dnd5e-web-member/overlays/assets.md' .opencode/agents/assets-web-member.md || fail "assets web overlay pointer is missing"
grep -Fq 'git commit *: deny' .opencode/agents/independent-gate.md || fail "gate commit denial is missing"
grep -Fq 'git push *: deny' .opencode/agents/independent-gate.md || fail "gate push denial is missing"
grep -Fq 'gh pr merge *: deny' .opencode/agents/independent-gate.md || fail "gate merge denial is missing"

```

- [ ] **Step 8: Run the required OpenCode validation set**

```bash
./scripts/verify-opencode.sh
opencode debug config
opencode agent list
for agent in ui-lead platform-lead assets-lead rpg-toolkit-member rpg-api-member rpg-api-protos-member rpg-deployment-member rpg-game-assets-member ui-web-member assets-web-member toolkit-fixer api-fixer web-fixer independent-gate explore janitor; do opencode debug agent "$agent"; done
opencode models openai --verbose
opencode debug file read AGENTS.md
opencode mcp list
```

Expected: verifier prints `PASS: OpenCode project configuration verified without a model call`; config shows six references and four disabled MCPs; all 16 adapters resolve with manifest model/mode/variant; OpenAI models list includes the configured model IDs; `AGENTS.md` resolves through the Git symlink; MCP listing contains configured `chrome-devtools` without requiring its port to be live.

- [ ] **Step 9: Commit, gate, and release the runtime PR**

```bash
git add AGENTS.md opencode.jsonc .opencode scripts/verify-opencode.sh
git commit -m "feat(workflow): add project-scoped OpenCode runtime (#RUNTIME_ISSUE)"
```

Run `git diff --check`, request a fresh `independent-gate`, push, open a ready PR with `Closes #RUNTIME_ISSUE`, set Project 19 to `In Review`, and publish all red/green evidence. The human merge follows a passing gate; set the item to `Done` after merge.

### Task 3: Portable Seven-Repository Workspace

**Files:**
- Create: `scripts/workspace-repos.sh`, `tests/bootstrap-contract.sh`, `scripts/verify-workspace.sh` in `game-dev`.
- Modify: `bootstrap.sh`, `.gitignore`, `README.md`, `CLAUDE.md` in `game-dev`.

**Interfaces:**
- Consumes: merged Task 2 `rpg-project` runtime; Bash; `git`; optional `opencode`.
- Produces: `workspace_repos`, a zero-argument Bash function that prints exactly seven repository names in order; bootstrapping and verification consume this function rather than another repository list.

- [ ] **Step 1: Create the task record before creating a branch**

Create a `game-dev` issue titled `Portable seven-repository OpenCode workspace`, add it to Project 19 with exactly `Team=Cross-team`, `Feature=Infra`, `Kind=Build`, `Status=Todo`, then move it to `In Progress`. Post the branch and checkpoint contract, then create `feat/WORKSPACE_ISSUE-seven-repo-workspace` fresh from `main`, where `WORKSPACE_ISSUE` is the issue number just created.

- [ ] **Step 2: Write the bootstrap contract test first**

Create `tests/bootstrap-contract.sh` with this complete test, make it executable, and run it before adding `scripts/workspace-repos.sh`:

```bash
#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
fail() { printf 'FAIL: %s\n' "$*" >&2; exit 1; }

[ -f "$ROOT/scripts/workspace-repos.sh" ] || fail "scripts/workspace-repos.sh is missing"
# shellcheck source=scripts/workspace-repos.sh
source "$ROOT/scripts/workspace-repos.sh"

expected=$'rpg-project\nrpg-toolkit\nrpg-api\nrpg-api-protos\nrpg-dnd5e-web\nrpg-game-assets\nrpg-deployment'
actual="$(workspace_repos)"
[ "$actual" = "$expected" ] || fail "workspace repository order is wrong"

for repo in rpg-project rpg-toolkit rpg-api rpg-api-protos rpg-dnd5e-web rpg-game-assets rpg-deployment; do
  grep -Fqx "/$repo/" "$ROOT/.gitignore" || fail ".gitignore lacks /$repo/"
done

grep -Fq 'source "$ROOT/scripts/workspace-repos.sh"' "$ROOT/bootstrap.sh" || fail "bootstrap does not use workspace-repos.sh"
if grep -Eq 'opencode.*(install|curl|wget|npm|pnpm|apt-get)|\.config/opencode|OPENCODE_CONFIG' "$ROOT/bootstrap.sh"; then
  fail "bootstrap may install or mutate OpenCode global configuration"
fi

bash -n "$ROOT/bootstrap.sh"
bash -n "$ROOT/scripts/workspace-repos.sh"
bash -n "$ROOT/scripts/verify-workspace.sh"
printf 'PASS: bootstrap contract verified\n'
```

- [ ] **Step 3: Demonstrate the intentional red state**

```bash
./tests/bootstrap-contract.sh
```

Expected before the source list exists: non-zero exit with `FAIL: scripts/workspace-repos.sh is missing`.

- [ ] **Step 4: Add the one exact repository source**

Create `scripts/workspace-repos.sh`:

```bash
#!/usr/bin/env bash

workspace_repos() {
  printf '%s\n' \
    rpg-project \
    rpg-toolkit \
    rpg-api \
    rpg-api-protos \
    rpg-dnd5e-web \
    rpg-game-assets \
    rpg-deployment
}
```

- [ ] **Step 5: Replace bootstrap's repository block and add the warning-only check**

Immediately after `cd "$ROOT"` in `bootstrap.sh`, add:

```bash
# shellcheck source=scripts/workspace-repos.sh
source "$ROOT/scripts/workspace-repos.sh"
```

Replace the existing three-item `REPOS=(...)` declaration and loop with:

```bash
for repo in $(workspace_repos); do
  if [ -d "$ROOT/$repo/.git" ]; then
    ok "$repo already cloned — fetching"
    git -C "$ROOT/$repo" fetch --quiet || warn "$repo: fetch failed (check SSH auth)"
  else
    warn "$repo not present — cloning"
    if git clone "git@github.com:KirkDiggler/$repo.git" "$ROOT/$repo"; then
      ok "$repo cloned"
    else
      warn "$repo clone failed — this is almost always SSH auth. Set up your git SSH key (github.com) yourself, then re-run ./bootstrap.sh."
      NEXT_STEPS+=("git clone git@github.com:KirkDiggler/$repo.git $ROOT/$repo  (after SSH auth is set up)")
    fi
  fi
done
```

Before the final `Bootstrap complete` section, add only this availability warning:

```bash
log "Checking OpenCode availability"
if command -v opencode >/dev/null 2>&1; then
  ok "OpenCode available ($(command -v opencode))"
else
  warn "OpenCode is not installed; install it separately if this workspace will use OpenCode."
fi
```

This block must not install OpenCode, write `$HOME/.config/opencode`, set `OPENCODE_CONFIG`, or touch provider credentials.

- [ ] **Step 6: Add the post-bootstrap verifier**

Create `scripts/verify-workspace.sh`:

```bash
#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
fail() { printf 'FAIL: %s\n' "$*" >&2; exit 1; }
# shellcheck source=scripts/workspace-repos.sh
source "$ROOT/scripts/workspace-repos.sh"

command -v opencode >/dev/null 2>&1 || fail "opencode is not available"
for repo in $(workspace_repos); do
  path="$ROOT/$repo"
  [ -d "$path/.git" ] || fail "$repo is not a git clone"
  remote="$(git -C "$path" remote get-url origin)"
  [ "$remote" = "git@github.com:KirkDiggler/$repo.git" ] || fail "$repo origin is $remote"
done

[ -L "$ROOT/rpg-project/AGENTS.md" ] || fail "rpg-project AGENTS.md is not a symlink"
[ "$(readlink "$ROOT/rpg-project/AGENTS.md")" = "CLAUDE.md" ] || fail "rpg-project AGENTS.md points somewhere else"
"$ROOT/rpg-project/scripts/verify-opencode.sh"
printf 'PASS: seven-repository workspace verified\n'
```

- [ ] **Step 7: Update ignores and orientation exactly**

Replace the cloned-repository portion of `.gitignore` with these seven lines:

```gitignore
/rpg-project/
/rpg-toolkit/
/rpg-api/
/rpg-api-protos/
/rpg-dnd5e-web/
/rpg-game-assets/
/rpg-deployment/
```

Update `README.md` and `CLAUDE.md` so every statement that says three cloned game repos instead says the ordered seven-repository workspace. State that `rpg-project` owns canonical roles and project-scoped OpenCode configuration; `game-dev` only bootstraps/checks availability and never installs OpenCode, changes global OpenCode configuration, credentials, or provider settings.

- [ ] **Step 8: Run contract and syntax checks green**

```bash
./tests/bootstrap-contract.sh
bash -n bootstrap.sh
bash -n scripts/workspace-repos.sh
bash -n scripts/verify-workspace.sh
```

Expected: `PASS: bootstrap contract verified` and all three syntax commands return exit code `0`. Do not run the bootstrap during this task as its tool-install behavior is not a unit test.

- [ ] **Step 9: Commit, gate, and release the portability PR**

```bash
git add bootstrap.sh .gitignore README.md CLAUDE.md scripts/workspace-repos.sh scripts/verify-workspace.sh tests/bootstrap-contract.sh
git commit -m "feat(workspace): bootstrap seven project repositories (#WORKSPACE_ISSUE)"
```

Run `git diff --check`; request an independent gate; push a ready PR with `Closes #WORKSPACE_ISSUE`; set Project 19 to `In Review`; publish the red/green transcript. Kirk merges after a passing fresh gate, then mark the item `Done`. This repo has no application deploy claim, so no deployment verification is asserted.

### Task 4: Project 19 Clean-Slate Verification

**Files:**
- Create: no files and no code PR.

**Interfaces:**
- Consumes: merged Tasks 1 through 3; a fresh WSL/Linux path or fresh Linux machine; GitHub issue checkpoint comments.
- Produces: a reproducible evidence record that global OpenCode configuration is unchanged across two bootstrap runs and the seven-repo project configuration works locally.

- [ ] **Step 1: Create the Verify issue before the environment run**

Create an `rpg-project` issue titled `Verify OpenCode clean-slate workspace bootstrap`, add it to Project 19 with exactly `Team=Cross-team`, `Feature=Infra`, `Kind=Verify`, and `Status=Todo`, then move it to `In Progress`. No branch and no code PR are created because this task's explicit deliverable is a clean-machine evidence record. Post `WORK SESSION STARTED`, the exact fresh path or machine identifier, and the expected evidence list.

- [ ] **Step 2: Capture the global-config before snapshot without modifying it**

Run exactly one of the following and paste the command output and chosen state into the issue:

```bash
if [ -e "$HOME/.config/opencode/opencode.jsonc" ]; then sha256sum "$HOME/.config/opencode/opencode.jsonc"; else printf 'ABSENT\n'; fi
```

The absence case is valid. Do not create the path, copy a template into it, authenticate, or alter provider credentials.

- [ ] **Step 3: Bootstrap twice from the clean `game-dev` checkout**

```bash
./bootstrap.sh
./bootstrap.sh
```

Expected: each run attempts or confirms exactly seven ordered clones from `workspace_repos`; the OpenCode section only reports availability or a warning and performs no install or global write. Record clone/fetch results and any SSH/auth blocker in the issue instead of silently retrying it.

- [ ] **Step 4: Verify the local workspace and project runtime**

```bash
./scripts/verify-workspace.sh
cd rpg-project
opencode debug config
opencode agent list
opencode debug file read AGENTS.md
opencode mcp list
```

Expected: `PASS: seven-repository workspace verified`; all seven `.git` directories and GitHub remotes exist; `rpg-project/AGENTS.md` resolves to `CLAUDE.md`; project verification passes; the runtime lists six sibling references and configured adapters. Chrome may be unavailable; the config and MCP listing are the required proof, not a live browser connection.

- [ ] **Step 5: Capture the after snapshot and compare**

Run the same command from Step 2. The before and after states must both be `ABSENT`, or both be the identical SHA-256 digest. Record the two values and the comparison result in the issue.

Expected: exact equality. A changed value is a failed verification: stop, set the Project item to `In Review`, publish the diff path and blocker, and open a linked fix issue rather than modifying the global file.

- [ ] **Step 6: Publish the evidence checkpoint and close only on proof**

Post a structured issue comment containing fresh environment, before/after states, both bootstrap outcomes, ordered repository/remotes proof, `verify-workspace.sh` output, symlink/config/sibling-access proof, and explicit next action. Request an independent Sol gate to review the evidence without altering the environment. After the gate passes and Kirk confirms the record, set the Project item to `Done`. No code PR is created for this Verify task.

### Task 5: Equipment Pilot Entry Gate and Recovery Proof

**Files:**
- Create: no proto, API, toolkit, web, or equipment implementation files in this plan.
- Create later: a separate `rpg-project` equipment-plan issue and ready plan PR after the three decisions below are documented.

**Interfaces:**
- Consumes: passing Task 4 issue evidence; existing `rpg-api-protos#187`; Project 19 durable state.
- Produces: a decision-complete, separately reviewed equipment plan and a recoverability/gate proof route; it deliberately does not prescribe proto fields or implementation.

- [ ] **Step 1: Verify and activate the existing decision task**

Use `rpg-api-protos#187` as the existing backing issue. Verify it is on Project 19 with exactly `Team=Platform`, `Feature=Class Kits`, `Kind=Decide`, and `Status=Todo`, then move it to `In Progress`. The Platform lead posts `WORK SESSION STARTED`, links the passed Task 4 Verify issue, and records that no proto implementation can start yet.

- [ ] **Step 2: Resolve exactly three contract decisions in GitHub**

The Platform lead obtains and records Kirk's decision for each of the following on `rpg-api-protos#187`: slot-selection ownership for ambiguous equip intent; authoritative item-slot compatibility representation; and whether inventory includes equipped items. The checkpoint names the selected answer and rationale for all three. Do not add the minor two-hander-blocked marker as a blocker unless a UI requirement proves it necessary.

- [ ] **Step 3: Create the separate equipment plan review unit**

After all three decisions are visible, create a new `rpg-project` issue titled `Equipment contract implementation plan`, add it to Project 19 with exactly `Team=Platform`, `Feature=Class Kits`, `Kind=Build`, `Status=Todo`, then set `In Progress`. Create a fresh `docs/EQUIPMENT_PLAN_ISSUE-equipment-plan` branch, where `EQUIPMENT_PLAN_ISSUE` is the issue number just created. Write and self-review a separate plan that references the resolved decisions and separates the proto, toolkit, API, and web legs into their own future issues/PRs. Open it as a ready Plan Review PR, gate it with fresh Sol, and require human merge. This step intentionally contains no proto implementation instructions.

- [ ] **Step 4: Prove deliberate worker replacement only after the plan snapshot merges**

The Platform lead launches a Terra implementation worker against `rpg-api-protos#187` only after the separate equipment plan snapshot is merged. Before deliberate termination, the worker must publish a GitHub checkpoint with completed work, commands and results, blockers, branch, current commit, and explicit next action, ending `— rpg-api-protos-member, on behalf of KirkDiggler`. Terminate that worker only after the checkpoint is visible.

- [ ] **Step 5: Replace only from durable state and fresh-gate the result**

Launch a new Terra worker with only the issue URL, branch, PR URL if present, and checkpoint comment. It must reconstruct from those GitHub artifacts rather than an inherited `task_id` or local transcript, finish its one issue/one PR work, and publish its own checkpoint. Launch a fresh Sol `independent-gate` after the PR is ready; the gate may run reversible tests but must not fix, commit, push, or merge. Route every finding to the replacement worker and require a fresh Sol regate after remediation. Kirk performs the human merge and deployment verification only if the repository's release path applies.

## Final Review Checklist

- [ ] Re-read `ideas/opencode-team-workflow/design.md` sections 1 through 10 and record this exact coverage map in the self-review comment: section 1 (control-plane architecture) is Tasks 1 and 2; section 2 (Project 19 contract and artifact lifecycle) is Tasks 1 through 5; section 3 (durable continuity) is Tasks 2, 4, and 5; section 4 (roles and roster) is Task 1; section 5 (model profile) is Task 2; section 6 (configuration and source of truth) is Tasks 2 and 3; section 7 (operational gates) is Tasks 1 through 5; section 8 (side-by-side verification) is Task 4; section 9 (equipment pilot) is Task 5; section 10 (rollout issue) is this Plan Review PR tracking #101.
- [ ] Search the plan and lifecycle corrections for unresolved-marker text, time estimates, and vague-test language; replace every occurrence that would leave an implementer to invent behavior. The post-gate equipment-plan creation statement is valid only because it names its exact prerequisite: three documented `rpg-api-protos#187` decisions after Task 4 passes.
- [ ] Check every adapter name, canonical path, model, variant, mode, and permission against **Adapter Manifest** and `opencode.jsonc`; check that only six sibling references and exactly four disabled inherited MCPs are specified.
- [ ] Confirm all required validation commands appear exactly: `opencode debug config`, `opencode agent list`, `opencode debug agent NAME`, `opencode models openai --verbose`, `opencode debug file read AGENTS.md`, and `opencode mcp list`.
- [ ] Confirm no prohibited config keys, no plugin, no daemon, no checkpoint schema, no duplicate tracker, no dry-run rewrite, no test framework, no Anthropic/Claude/Sonnet model, and no equipment implementation details appear.
- [ ] Run `git diff --check`, inspect `git status --short --branch`, and inspect `git diff -- ideas/opencode-team-workflow/{plan,design,CLAUDE}.md`; fix every discrepancy inline before committing.
- [ ] Commit only `ideas/opencode-team-workflow/plan.md`, `ideas/opencode-team-workflow/design.md`, and `ideas/opencode-team-workflow/CLAUDE.md` for this documentation snapshot; never amend and never use `--no-verify`.

## Plan Review Handoff

Push `docs/101-opencode-team-workflow-plan` and open a ready PR against `main` titled `docs(ideas): OpenCode team workflow implementation plan (#101)`. Use this body structure:

```markdown
## Review phase: Plan Review

## Summary

This plan establishes five independently gateable units: canonical role policy, project-scoped OpenCode runtime, portable seven-repository bootstrap, clean-slate verification, and the decision-gated equipment recovery proof.

## Task boundaries

- Task 1: canonical role system
- Task 2: project OpenCode runtime
- Task 3: portable seven-repository workspace
- Task 4: clean-slate Verify issue
- Task 5: decision-gated equipment plan and recovery proof

## Review focus

- OpenCode 1.18.4 configuration and adapter mechanics
- Charter/overlay ownership and permission boundaries
- Red/green verifier and non-mutating bootstrap contracts
- GitHub-only recovery proof before the equipment pilot

## Verification

- Design coverage, unresolved-marker scan, model/path/config consistency, and `git diff --check` completed

Tracks #101

— cross-team Terra plan writer, on behalf of KirkDiggler
```

Publish an issue #101 checkpoint with the PR URL and `Next action: review the Plan Review PR, then create Task 1's dedicated Project 19 issue after written approval.` End it with `— cross-team Terra plan writer, on behalf of KirkDiggler`. Do not merge the plan PR.
