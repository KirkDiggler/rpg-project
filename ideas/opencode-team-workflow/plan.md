# OpenCode Team Workflow Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Establish a portable, project-scoped OpenCode team workflow whose canonical role policy lives in `rpg-project`, whose workstation setup lives in `game-dev`, and whose setup evidence and product-PR review paths are recoverable from GitHub alone.

**Architecture:** `rpg-project` first receives provider-neutral role charters and overlays, then project-scoped OpenCode configuration and thin adapters that bind those charters to OpenAI model and permission profiles. `game-dev` consumes the resulting project contract through an exact seven-repository bootstrap list, while an issue-only Verify task proves bootstrap and verification preserve an initialized global configuration baseline. Setup units rely on deterministic checks, self-review, ready review surfaces where applicable, and Kirk's merge or confirmation; only actual product-behavior PRs receive normal review plus one independent Sol gate.

**Tech Stack:** Markdown role charters, OpenCode 1.18.4 JSONC and Markdown agents, Bash, `jq`, Git, GitHub CLI, Project 19.

## Global Constraints

- Work only in the repository named by each task; no worktree path is a review surface.
- Start every executable task with one repository issue and one Project 19 item. Every task that produces a repository diff also starts with a fresh branch from `main` and a ready, non-draft PR; use one issue per PR. Task 4 is the explicit issue-only Verify exception: it creates no branch or PR but remains backed by its issue and Project item.
- Project 19 fields are exact: Status `Todo` then `In Progress` then `In Review` then `Done`, including Task 4's issue-only lifecycle; Team `UI/UX`, `Platform`, `Assets`, or `Cross-team`; Feature `Party Assembles`, `Class Kits`, `The Dungeon`, `Game Screen`, `Capstone`, `Shelf`, or `Infra`; Kind `Build`, `Fix`, `Verify`, `Learn`, or `Decide`.
- Every GitHub comment and PR body written by a worker ends with `— <role>, on behalf of KirkDiggler`.
- Setup lifecycle is exact: create files -> run deterministic red then green checks -> self-review -> commit -> push and open a ready PR -> Kirk reviews and decides whether to merge. A setup-ready PR is not self-declared merge-ready, receives no independent adversarial gate, and Kirk alone merges. Task 4 is the issue-only Verify exception: publish deterministic evidence, self-review it, move the item to `In Review`, and wait for Kirk's confirmation before `Done`.
- Product-behavior PR lifecycle is exact: normal review -> one independent Sol gate. Product behavior means game rules, proto/API contracts, player-facing web behavior, deployment/runtime behavior, or asset-pipeline output consumed by the game. Do not stack a separate task-review or gate layer over the same PR. If the gate reports findings, the original implementer remediates and the same reviewer performs a focused recheck of the reported findings and changed surfaces. Start a new unrestricted audit only after a material scope rewrite. A gate verdict never authorizes a merge: Kirk alone decides and merges. Verify auto-deploy completion after every applicable merge.
- Project 19 and GitHub issue, PR, branch, and checkpoint comments are the only durable task state. `task_id` is optional live continuity, never recovery state.
- Use OpenCode 1.18.4 mechanics only. Project config deep-merges global config. Use only supported keys such as `enabled_providers`, `plugin`, `model`, `small_model`, `default_agent`, `references`, `mcp`, `agent`, and `permission`; do not introduce `project`, `workspace`, `agents`, `mcps`, `permissions`, `charter`, `overlay`, `contextPath`, or `checkpoints` config keys.
- The initial profile permits only OpenAI: `enabled_providers` is exactly `["openai"]`. The project config declares `"plugin": ["superpowers@git+https://github.com/obra/superpowers.git"]` so a clean machine does not depend on Kirk's global configuration. If global and project configuration declare that identical spec, OpenCode resolves one plugin; on a clean machine OpenCode may populate its own cache, but bootstrap never edits global configuration.
- Each `references` entry is keyed by its repository alias and has both an explicit relative `path` and a non-empty `description`; paths are exactly `../rpg-toolkit`, `../rpg-api`, `../rpg-api-protos`, `../rpg-dnd5e-web`, `../rpg-deployment`, and `../rpg-game-assets`.
- `opencode.jsonc` contains `$schema: "https://opencode.ai/config.json"`; adapters live at `.opencode/agents/`; every adapter frontmatter uses singular `permission`.
- Every adapter body explicitly requires reading its listed canonical charter and overlay before action. Adapters bind model and permissions only; they do not duplicate charter policy and never use `{file:...}` Markdown splicing.
- Use only OpenAI models named in this plan. Do not configure, invoke, or dispatch an Anthropic, Claude, or Sonnet model.
- OpenAI authentication is machine-local. Only Kirk performs the interactive `/connect` flow; bootstrap, structural verification, and evidence-only Verify work never authenticate, write credentials, or modify global OpenCode configuration. A product Sol gate uses existing local authentication or stops and publishes an auth blocker.
- Do not add a plugin beyond the exact project-scoped Superpowers spec, daemon, checkpoint JSON schema, duplicate tracker, dry-run rewrite, or test framework.
- This plan does not prescribe equipment implementation. It records decision ratification, documentation, evidence, recovery routing, and the dependency order for the already-open product PRs `rpg-api-protos#188`, `rpg-toolkit#812`, and `rpg-api#682`.

---

## Reviewable Units

| Task | Repository | Dependency | Project 19 fields | Deliverable |
|---|---|---|---|---|
| 1 | `rpg-project` | none | Cross-team / Infra / Build / Todo | Canonical charters, overlays, and roles index |
| 2 | `rpg-project` | Task 1 merged | Cross-team / Infra / Build / Todo | OpenCode config, adapters, workflow skill, red/green verifier |
| 3 | `game-dev` | Task 2 merged | Cross-team / Infra / Build / Todo | Seven-repo portable bootstrap and shell-only tests |
| 4 | `rpg-project` issue only | Task 3 merged | Cross-team / Infra / Verify / Todo | Fresh-machine clean-slate evidence; no code PR |
| 5 | existing `rpg-api-protos#187/#188` plus a documentation issue | Task 4 evidence confirmed by Kirk | `#187`: Platform / Game Screen / Decide / Todo -> In Progress; `#811` and `#680`: Platform / Game Screen / Build / In Progress; documentation issue: Platform / Game Screen / Build / Todo -> In Progress | Decision ratification, documentation, and recovery evidence route the existing product chain; #188, #812, and #682 retain their product review paths |

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
- Modify: `docs/teams/roles/director/prompt.md` — preserve director coordination authority while reserving every merge decision and action to Kirk.
- Modify: `docs/teams/roles/janitor/prompt.md` — Project 19, issue/branch/ready-PR, and Kirk-only merge policy for Janitor repository diffs.
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
- Create: one separate `rpg-project` equipment documentation issue and ready review PR after Kirk's written ratification records the three `#187` contract decisions. It retrospectively documents the adopted `#187/#188`, `#811/#812`, and `#680/#682` chain, the ratified contract/scope, and the missing `ideas/equipment/design.md` reference.

## Adapter Manifest

Each file below uses this exact Markdown structure, substituting only the
literal per-adapter values in the manifest table. The `description` is exactly
`OpenCode runtime adapter for <adapter name>; canonical policy remains in the
listed role documents.` where `<adapter name>` is the filename without `.md`.
The body does not add role policy beyond the following contract:

```markdown
---
description: OpenCode runtime adapter for <adapter name>; canonical policy remains in the listed role documents.
mode: <manifest mode>
model: <manifest model>
variant: <manifest variant>
permission:
  <manifest permission map, preserving the listed order>
---

# <adapter name>

Before any action, read these canonical files:
- `<literal canonical path 1>`
- `<literal canonical path 2, if any>`

The canonical files above are the complete role policy. This adapter only binds
that policy to this runtime model and permission profile; it does not restate
or override the policy. Publish a signed GitHub checkpoint when blocked, when
a handoff is required, and before this dispatched task ends.
```

The manifest table supplies every literal substitution above; implementers may
not use "similar to", copy another adapter, use `{file:...}` splicing, or omit
a listed canonical path. Permission maps use YAML nesting. `"*"` must precede
more-specific patterns because the last matching OpenCode rule wins.

| Adapter file | Exact description | Mode / model / variant | Canonical files read before action | Permission binding |
|---|---|---|---|---|
| `ui-lead.md` | `OpenCode runtime adapter for ui-lead; canonical policy remains in the listed role documents.` | primary / `openai/gpt-5.6-sol-fast` / `xhigh` | `docs/teams/roles/director/prompt.md`; `docs/teams/roles/director/field-notes.md`; `docs/teams/roles/director/overlays/ui-ux.md` | `edit: deny`; `bash: {"*": "deny", "gh *": "allow", "gh pr merge*": "deny", "gh * pr merge*": "deny"}`; `task: {"*": "deny", "ui-web-member": "allow", "web-fixer": "allow", "explore": "allow", "independent-gate": "allow", "janitor": "allow"}` |
| `platform-lead.md` | `OpenCode runtime adapter for platform-lead; canonical policy remains in the listed role documents.` | primary / `openai/gpt-5.6-sol-fast` / `xhigh` | `docs/teams/roles/director/prompt.md`; `docs/teams/roles/director/field-notes.md`; `docs/teams/roles/director/overlays/platform.md` | `edit: deny`; `bash: {"*": "deny", "gh *": "allow", "gh pr merge*": "deny", "gh * pr merge*": "deny"}`; `task: {"*": "deny", "rpg-toolkit-member": "allow", "rpg-api-member": "allow", "rpg-api-protos-member": "allow", "rpg-deployment-member": "allow", "toolkit-fixer": "allow", "api-fixer": "allow", "explore": "allow", "independent-gate": "allow", "janitor": "allow"}` |
| `assets-lead.md` | `OpenCode runtime adapter for assets-lead; canonical policy remains in the listed role documents.` | primary / `openai/gpt-5.6-sol-fast` / `xhigh` | `docs/teams/roles/director/prompt.md`; `docs/teams/roles/director/field-notes.md`; `docs/teams/roles/director/overlays/assets.md` | `edit: deny`; `bash: {"*": "deny", "gh *": "allow", "gh pr merge*": "deny", "gh * pr merge*": "deny"}`; `task: {"*": "deny", "rpg-game-assets-member": "allow", "assets-web-member": "allow", "web-fixer": "allow", "explore": "allow", "independent-gate": "allow", "janitor": "allow"}` |
| `rpg-toolkit-member.md` | `OpenCode runtime adapter for rpg-toolkit-member; canonical policy remains in the listed role documents.` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/rpg-toolkit-member/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `rpg-api-member.md` | `OpenCode runtime adapter for rpg-api-member; canonical policy remains in the listed role documents.` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/rpg-api-member/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `rpg-api-protos-member.md` | `OpenCode runtime adapter for rpg-api-protos-member; canonical policy remains in the listed role documents.` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/rpg-api-protos-member/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `rpg-deployment-member.md` | `OpenCode runtime adapter for rpg-deployment-member; canonical policy remains in the listed role documents.` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/rpg-deployment-member/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `rpg-game-assets-member.md` | `OpenCode runtime adapter for rpg-game-assets-member; canonical policy remains in the listed role documents.` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/rpg-game-assets-member/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `ui-web-member.md` | `OpenCode runtime adapter for ui-web-member; canonical policy remains in the listed role documents.` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/rpg-dnd5e-web-member/prompt.md`; `docs/teams/roles/rpg-dnd5e-web-member/overlays/ui-ux.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `assets-web-member.md` | `OpenCode runtime adapter for assets-web-member; canonical policy remains in the listed role documents.` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/rpg-dnd5e-web-member/prompt.md`; `docs/teams/roles/rpg-dnd5e-web-member/overlays/assets.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `toolkit-fixer.md` | `OpenCode runtime adapter for toolkit-fixer; canonical policy remains in the listed role documents.` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/toolkit-fixer/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `api-fixer.md` | `OpenCode runtime adapter for api-fixer; canonical policy remains in the listed role documents.` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/api-fixer/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `web-fixer.md` | `OpenCode runtime adapter for web-fixer; canonical policy remains in the listed role documents.` | subagent / `openai/gpt-5.6-terra` / `high` | `docs/teams/roles/web-fixer/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |
| `independent-gate.md` | `OpenCode runtime adapter for independent-gate; canonical policy remains in the listed role documents.` | subagent / `openai/gpt-5.6-sol` / `max` | `docs/teams/roles/independent-gate/prompt.md` | `edit: allow`; `bash: {"*": "allow", "git *commit*": "deny", "git *push*": "deny", "git *merge*": "deny", "gh pr merge*": "deny", "gh * pr merge*": "deny"}`; `task: deny` |
| `explore.md` | `OpenCode runtime adapter for explore; canonical policy remains in the listed role documents.` | subagent / `openai/gpt-5.6-luna` / `medium` | `docs/teams/roles/explore/prompt.md` | `edit: deny`; `bash: allow`; `task: deny` |
| `janitor.md` | `OpenCode runtime adapter for janitor; canonical policy remains in the listed role documents.` | subagent / `openai/gpt-5.6-luna` / `low` | `docs/teams/roles/janitor/prompt.md` | `edit: allow`; `bash: allow`; `task: deny` |

The primary agents are the only main-session entry points. Members, fixers, gate, explore, and janitor run underneath them. An active session may resume its `task_id`; replacement after a failure must be reconstructed from the issue, branch, PR, and checkpoint only. The independent-gate adapter is dispatched only for product-behavior PRs. The lead and gate denial patterns block ordinary direct and repo-flagged `gh ... pr merge` forms, plus ordinary `git` commit, push, and merge forms for the gate. They are not a security sandbox against shell wrapping or `gh api`; the lead overlays and independent-gate charter remain the authoritative Kirk-only merge boundary.

### Task 1: Canonical Provider-Neutral Role System

**Files:**
- Create every Task 1 path listed in **File Structure**.
- Modify: `docs/teams/roles/director/prompt.md`, `docs/teams/roles/janitor/prompt.md`, `docs/teams/roles/README.md`.

**Interfaces:**
- Consumes: `docs/teams/roles/director/{prompt,field-notes}.md`; the four existing standing-member charters; existing fixer and janitor charters.
- Produces: canonical, provider-neutral files consumed verbatim by the Task 2 adapter manifest; the role index becomes the roster authority; lane overlays and the Janitor charter leave merge authority only with Kirk.

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

Run this exact shell acceptance block manually before creating any Task 1 files
for the required RED result, then run the identical block after creating them
for the required GREEN result. Do not create a script, test framework, or any
other test artifact for this task. The `bash -euo pipefail -c` wrapper makes
every missing file or failed assertion terminate the check. Record both outputs
in the issue checkpoint:

```bash
bash -euo pipefail -c '
  test -f docs/teams/roles/director/overlays/ui-ux.md
  test -f docs/teams/roles/director/overlays/platform.md
  test -f docs/teams/roles/director/overlays/assets.md
  test -f docs/teams/roles/rpg-dnd5e-web-member/overlays/ui-ux.md
  test -f docs/teams/roles/rpg-dnd5e-web-member/overlays/assets.md
  for role in rpg-deployment-member rpg-game-assets-member independent-gate explore; do
    prompt="docs/teams/roles/$role/prompt.md"
    test -f "$prompt"
    case "$role" in
      rpg-deployment-member) description="Provider-neutral standing member for rpg-deployment." ;;
      rpg-game-assets-member) description="Provider-neutral standing member for rpg-game-assets." ;;
      independent-gate) description="Provider-neutral independent adversarial gate reviewer." ;;
      explore) description="Provider-neutral read-only orientation role." ;;
    esac
    {
      IFS= read -r first
      IFS= read -r name
      IFS= read -r actual_description
      IFS= read -r fourth
    } < "$prompt"
    [ "$first" = "---" ]
    [ "$name" = "name: $role" ]
    [ "$actual_description" = "description: $description" ]
    [ "$fourth" = "---" ]
    for context in active-work dependencies discoveries lessons-learned patterns; do
      jq -e ".items == []" "docs/teams/roles/$role/context/$context.json" >/dev/null
    done
  done
  grep -Fqx "| Board hygiene | \`gh project ...\` (Project 19) | Stale issues, field consistency |" docs/teams/roles/janitor/prompt.md
  grep -Fqx "I never commit directly on main. Every repository diff has one backing issue, one Project 19 item, a fresh branch from main, deterministic checks, self-review, and a ready PR; Kirk alone decides whether to merge." docs/teams/roles/janitor/prompt.md
  grep -Fqx -- "- You drive coordination and verification; Kirk alone decides and merges PRs. Log every decision as a visible, observable fact: board fields, PR comments, and \`sessions/active.md\`. A decision is not done until it is on the board AND the PRs — not just the narrative. Propagate every decision everywhere it is visible, in the same beat." docs/teams/roles/director/prompt.md
'
```

Expected before adding files: failure on the first missing overlay or charter path.

- [ ] **Step 4: Prove the acceptance block discriminates a partial failure**

After all new Task 1 files exist but before accepting GREEN, temporarily rename
`docs/teams/roles/director/overlays/ui-ux.md` to a same-directory `.bak` name,
run the fail-fast prefix used by Step 3, and restore the exact filename:

```bash
mv docs/teams/roles/director/overlays/ui-ux.md docs/teams/roles/director/overlays/ui-ux.md.bak
if bash -euo pipefail -c 'test -f docs/teams/roles/director/overlays/ui-ux.md'; then
  printf 'FAIL: partial missing overlay unexpectedly passed\n' >&2
  exit 1
fi
mv docs/teams/roles/director/overlays/ui-ux.md.bak docs/teams/roles/director/overlays/ui-ux.md
```

Expected: the negative probe exits non-zero before restoration; the restored
file lets the complete Step 3 check reach its later assertions.

- [ ] **Step 5: Reconcile the director charter and write the three director overlays**

In `docs/teams/roles/director/prompt.md`, replace the first bullet under
`## Autonomy + visible facts (the deal with Kirk)` with this exact text. This
updates the canonical director policy before the three adapters inherit it:

```markdown
- You drive coordination and verification; Kirk alone decides and merges PRs. Log every decision as a visible, observable fact: board fields, PR comments, and `sessions/active.md`. A decision is not done until it is on the board AND the PRs — not just the narrative. Propagate every decision everywhere it is visible, in the same beat.
```

Create the following minimal overlays. They extend the director charter; they do not repeat its no-hands-on-work policy.

```markdown
<!-- docs/teams/roles/director/overlays/ui-ux.md -->
# Director Overlay: UI/UX

Run the UI/UX lane with its human collaborator. Route screens, HUD, interaction,
accessibility, responsive Discord viewport, and fixture-to-live presentation to
`ui-web-member`; require screenshots or recordings as the evidence artifact.
Escalate missing server facts as Platform work; never authorize client-side game
calculations or state gating. Coordinate shared-web work with the Assets overlay
through linked issues or one explicitly joint issue. Kirk alone decides and
merges.
```

```markdown
<!-- docs/teams/roles/director/overlays/platform.md -->
# Director Overlay: Platform

Run the Platform lane with its human collaborator. Sequence contract work before
API consumers and API work before web consumers. Route rules to rpg-toolkit,
shape to rpg-api-protos, orchestration to rpg-api, and release delivery to
rpg-deployment. Require wire-state evidence for platform claims and terminal
deployment verification after applicable human merges. Kirk alone decides and
merges.
```

```markdown
<!-- docs/teams/roles/director/overlays/assets.md -->
# Director Overlay: Assets

Run the Assets lane with its human collaborator. Route licensed source, conversion,
contract-tree, manifest, and performance work to rpg-game-assets; route web-side
model loading, environment rendering, animation playback, and 3D evidence to
`assets-web-member`. Require multi-angle evidence and preserve the private asset
license boundary; coordinate shared-web work with the UI/UX overlay. Kirk alone
decides and merges.
```

- [ ] **Step 6: Write the shared-web overlays and new standing-member charters**

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

Create the two standing charters with these exact bodies. They are canonical,
provider-neutral policy; do not put model or runtime-permission language in
either file:

```markdown
<!-- docs/teams/roles/rpg-deployment-member/prompt.md -->
---
name: rpg-deployment-member
description: Provider-neutral standing member for rpg-deployment.
---

# rpg-deployment Standing Member

## Identity and boundary

I own `rpg-deployment` from issue through human merge and delivery verification.
I own the auto-deploy pipeline, `nginx-http.conf`, release sequencing, and
watching `Deploy RPG Platform` to terminal success. I do not implement
toolkit, API, or proto business logic; I name the owning repository and open
or link the upstream issue when a deployment brief crosses that boundary.

## Operating contract

I start from one backing issue and Project 19 entry, a fresh branch from main,
and a visible WORK SESSION STARTED checkpoint. I keep issue, branch, PR, test
results, blockers, and next action sufficient for a replacement worker to
resume from GitHub alone. I use TDD or the repository's closest existing test
discipline, run the repository-local gates, never use `--no-verify`, and keep
`docs/status.md` and `docs/quality.md` honest in the PR that changes their
claims. I publish a signed checkpoint when blocked, handing off, and before a
dispatched task ends.

I refuse lane violations, including a request to repair application behavior
in deployment configuration. I do not make product decisions, declare my own
work merge-ready, or merge. Kirk alone makes final decisions and merges.
After an applicable human merge I watch `Deploy RPG Platform` to terminal
success; merge is not shipped. A permission prompt or authentication block
means stop and report the exact blocker on GitHub rather than retrying silently.

## Done-gate

Before claiming completion I answer: Goal: does observable delivery match the
issue goal? Pattern: did the change follow this repository's established
patterns? Test: did the real repository and deployment path prove it, rather
than a fixture bypass? Pushback: did the brief conflict with this charter or a
platform boundary? State the answer visibly.
```

```markdown
<!-- docs/teams/roles/rpg-game-assets-member/prompt.md -->
---
name: rpg-game-assets-member
description: Provider-neutral standing member for rpg-game-assets.
---

# rpg-game-assets Standing Member

## Identity and boundary

I own `rpg-game-assets` from issue through human merge and the asset-contract
handoff. I protect the private Synty license boundary; I own FBX-to-GLB
promotion, `harness/models/synty/`, manifests, shipped-asset budgets, and the
handoff to `assets-web-member`. I never commit raw licensed source or promoted
GLBs to a public repository. I do not implement web renderer code; I identify
the `assets-web-member` seam and open or link the correct web issue instead.

## Operating contract

I start from one backing issue and Project 19 entry, a fresh branch from main,
and a visible WORK SESSION STARTED checkpoint. I keep issue, branch, PR, test
results, blockers, and next action sufficient for a replacement worker to
resume from GitHub alone. I use TDD or the repository's closest existing test
discipline, run the repository-local gates, never use `--no-verify`, and keep
`docs/status.md` and `docs/quality.md` honest in the PR that changes their
claims. I publish a signed checkpoint when blocked, handing off, and before a
dispatched task ends.

I refuse lane violations, including requests to put private source assets or
renderer behavior in the wrong repository. I do not make product decisions,
declare my own work merge-ready, or merge. Kirk alone makes final decisions
and merges. A permission prompt or authentication block means stop and report
the exact blocker on GitHub rather than retrying silently.

## Done-gate

Before claiming completion I answer: Goal: does the asset deliverable match
the issue goal? Pattern: did the work preserve the contract tree and existing
pipeline? Test: did the real promotion or consuming path prove it, rather than
a fixture bypass? Pushback: did the brief conflict with this charter, the
license boundary, or the web handoff? State the answer visibly.
```

- [ ] **Step 7: Write independent-gate and explore charters**

Create these exact provider-neutral charters:

```markdown
<!-- docs/teams/roles/independent-gate/prompt.md -->
---
name: independent-gate
description: Provider-neutral independent adversarial gate reviewer.
---

# Independent Gate

## Purpose and applicability

I am a fresh-context, adversarial reviewer for a product-behavior PR only:
game rules, proto/API contracts, player-facing web behavior, deployment/runtime
behavior, or asset-pipeline output consumed by the game. Workflow setup
(documentation, configuration, charters, adapters, bootstrap/verifier scripts,
and evidence-only Verify work) does not dispatch me. Before reviewing a product
PR, I read the backing issue, PR diff, implementation claims, tests, evidence,
and this charter. I check the four-question done-gate: observable goal,
repository pattern, real-path test evidence, and unresolved boundary pushback.

## Authority and limits

I may run read-only checks and reversible mutation experiments that test a
claim, restoring the checkout before reporting. I do not fix, edit canonical
work, commit, push, merge, or alter Project fields. I may publish the verdict
`MERGE-READY` only when no Critical or Important findings remain; that verdict
does not authorize a merge or make the human merge decision. Shell permission
rules are defense in depth only; this charter is the authoritative
no-fix/no-canonical-edit/no-commit/no-push/no-merge boundary. If a permission
or authentication prompt blocks review, I stop and publish the exact blocker.

## Outcome

I publish a signed GitHub checkpoint with evidence, commands, residual risk,
and an explicit next action. When Critical or Important findings remain, I
publish those findings ordered by severity and return them to the original
implementer. The same independent reviewer then performs a focused recheck of
the reported findings and changed surfaces after remediation. I publish
`MERGE-READY` only when no Critical or Important findings remain. A new,
unrestricted independent audit occurs only after a material scope rewrite.
Kirk alone decides and merges.
```

```markdown
<!-- docs/teams/roles/explore/prompt.md -->
---
name: explore
description: Provider-neutral read-only orientation role.
---

# Explore

## Purpose and limits

I provide read-only orientation before a lead or member acts. I inspect the
requested repository, issue, PR, design, plan, and evidence, then return a
concise, evidence-backed report with exact paths, commands, and uncertainty.
I do not edit files, make product or architectural decisions, dispatch work,
create branches, commit, push, merge, or claim completion.

## Outcome

I distinguish verified facts from open questions and name the owning lane for
each discovered seam. If access, a permission prompt, or authentication
prevents the requested check, I stop and publish a signed GitHub-visible
blocker containing the failed check and the exact human next action. My report
is input to the owner; it never replaces the issue, PR, or Project 19 record.
```

- [ ] **Step 8: Create minimal context files using one exact shape**

For each of `rpg-deployment-member`, `rpg-game-assets-member`, `independent-gate`, and `explore`, create the five `context/*.json` files listed above with exactly:

```json
{
  "items": []
}
```

- [ ] **Step 9: Reconcile the Janitor charter and update the roles index**

In `docs/teams/roles/janitor/prompt.md`, replace the Project #10 Board hygiene
row, the Soft Mode direct-main exception, and the committing contract with the
following exact policy. Retain the Janitor's curatorial boundaries, promotion
path, and prohibition on merge or design decisions.

```markdown
| Board hygiene | `gh project ...` (Project 19) | Stale issues, field consistency |

## Repository-diff lifecycle

I never commit directly on main. Every repository diff has one backing issue,
one Project 19 item, a fresh branch from main, deterministic checks,
self-review, and a ready PR; Kirk alone decides whether to merge. Janitor
markdown and context-file work is workflow setup, so it receives no independent
Sol gate. If a Janitor task changes product behavior, I stop and route it to the
owning product role and its normal review plus one independent Sol gate.

## Committing Your Work

I leave no dirty working tree at the end of a dispatch. I stage exactly the
intended files, never use `git add -A` or `--no-verify`, and use
`chore(janitor): <short description>`. I never merge a PR or modify another
role's `prompt.md` without orchestrator approval.
```

Update `docs/teams/roles/README.md` to list three director overlays, six standing owners (`rpg-toolkit`, `rpg-api`, `rpg-api-protos`, `rpg-deployment`, `rpg-game-assets`, and shared `rpg-dnd5e-web` split through the two overlays), the three fixers, independent gate, explore, and janitor. State that charters and overlays are provider-neutral canonical policy, that the independent gate applies only to product-behavior PRs, and that `.opencode/agents/*.md` only bind those documents to runtime model and permission profiles.

- [ ] **Step 10: Run the charter acceptance checks green**

Run the Step 3 command again.

Expected: exit code `0`; every new context file parses as `{"items":[]}`, every required charter/overlay exists, and each new charter has its exact provider-neutral `name` and `description` frontmatter.

- [ ] **Step 11: Review and commit the reviewable role system**

Run `git diff --check`, inspect `git diff -- docs/teams/roles`, and self-review
all changed charter boundaries against `ideas/opencode-team-workflow/design.md`
sections 2, 4, 7, and 11. Then commit:

```bash
git add docs/teams/roles
git commit -m "docs(roles): add provider-neutral OpenCode role system (#ROLE_ISSUE)"
```

Expected: commit succeeds without `--no-verify` and contains only Task 1 paths.

- [ ] **Step 12: Push, self-review, and hand off the ready setup PR**

Push the committed branch, open a ready PR with `Closes #ROLE_ISSUE`, set
Project 19 `Status=In Review`, and post the PR URL, Step 3 RED/GREEN output,
Step 4 partial-state negative result, self-review, blockers, and explicit next
action. Do not call it merge-ready or request an adversarial gate: this is setup
under design §11. Kirk reviews and decides whether to merge. After Kirk's merge,
set `Status=Done`; this repository has no applicable deployment claim.

### Task 2: Project-Scoped OpenCode Runtime

**Files:**
- Create every Task 2 path listed in **File Structure**.

**Interfaces:**
- Consumes: the merged Task 1 canonical charters and overlays; OpenCode 1.18.4.
- Produces: `scripts/verify-opencode.sh` as the structural runtime contract; `platform-lead` as the default primary agent; only the 16 adapter names in the manifest; product-only gate routing.

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
  "enabled_providers": ["openai"],
  "plugin": ["superpowers@git+https://github.com/obra/superpowers.git"],
  "model": "openai/gpt-5.6-sol-fast",
  "small_model": "openai/gpt-5.6-luna",
  "default_agent": "platform-lead",
  "references": {
    "rpg-toolkit": { "path": "../rpg-toolkit", "description": "Rules engine and rulebook boundary." },
    "rpg-api": { "path": "../rpg-api", "description": "Game server and by-key orchestration boundary." },
    "rpg-api-protos": { "path": "../rpg-api-protos", "description": "Canonical API contract definitions." },
    "rpg-dnd5e-web": { "path": "../rpg-dnd5e-web", "description": "Discord Activity UI that renders data and sends intent." },
    "rpg-deployment": { "path": "../rpg-deployment", "description": "Deployment pipeline and live delivery configuration." },
    "rpg-game-assets": { "path": "../rpg-game-assets", "description": "Private game asset library and asset contract tree." }
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
      "model": "openai/gpt-5.6-luna",
      "variant": "low"
    },
    "summary": {
      "model": "openai/gpt-5.6-luna",
      "variant": "low"
    },
    "compaction": {
      "model": "openai/gpt-5.6-luna",
      "variant": "low"
    }
  }
}
```

- [ ] **Step 5: Create the adapters exactly from the manifest**

Create exactly the 16 files named in **Adapter Manifest** from its exact
frontmatter/body template, including the literal description, mode, model,
variant, canonical read list, and permission binding for each row. For
`independent-gate.md`, write this exact ordered `bash` map after the broad
allow rule:

```yaml
bash:
  "*": allow
  "git *commit*": deny
  "git *push*": deny
  "git *merge*": deny
  "gh pr merge*": deny
  "gh * pr merge*": deny
```

Do not create an adapter for `general`, `build`, `plan`, `title`, `summary`, or
`compaction`: those names are runtime overrides in `opencode.jsonc`. Do not
use `.opencode/agent/`, duplicate charter prose, or `{file:...}` in any
Markdown adapter.

- [ ] **Step 6: Create the project board workflow skill**

Create `.opencode/skills/project-board-workflow/SKILL.md`:

```markdown
---
name: project-board-workflow
description: Use before starting repository work that needs a Project 19 issue, branch, PR, checkpoint, product gate, or merge handoff.
---

# Project 19 Workflow

Before executable work, create or identify one repository issue, add it to
Project 19, set Team, Feature, Kind, and Status, then create one fresh branch
from main when the work produces a repository diff. Publish WORK SESSION
STARTED on the issue. Keep issue, PR, branch, and checkpoint comments sufficient
for a replacement worker to resume without session state. For workflow setup,
run deterministic red then green checks, self-review, commit, push and open a
ready PR, then hand off Kirk's review and merge decision. For product behavior
(rules, proto/API contracts, player-facing web behavior, deployment/runtime, or
consumed asset output), add normal review plus exactly one independent Sol gate.
Route findings to the original implementer; the same reviewer performs the
focused recheck after remediation. Start a new unrestricted audit only after a
material scope rewrite. End every GitHub comment with the active role signature
on behalf of KirkDiggler. After an applicable merge, verify deployment to
terminal success before marking the Project item Done.
```

- [ ] **Step 7: Complete the verifier without any model call**

Append the following complete checks after Step 2's preamble. The verifier inspects resolved configuration and files only; it does not run `opencode run`, does not call a model, and does not require Chrome to accept a connection.

```bash
tmpfiles=()
cleanup() { [ "${#tmpfiles[@]}" -eq 0 ] || rm -f "${tmpfiles[@]}"; }
trap cleanup EXIT

resolved="$(mktemp)"
tmpfiles+=("$resolved")
opencode debug config >"$resolved"

jq -e '
  .model == "openai/gpt-5.6-sol-fast" and
  .small_model == "openai/gpt-5.6-luna" and
  .default_agent == "platform-lead" and
  .enabled_providers == ["openai"]
' "$resolved" >/dev/null || fail "root model profile is wrong"
jq -e '
  (.agent.build.disable == true) and
  (.agent.plan.disable == true) and
  (.agent.general.mode == "subagent") and
  (.agent.general.model == "openai/gpt-5.6-terra") and
  (.agent.general.variant == "high") and
  (.agent.title | (keys | sort) == ["model", "variant"]) and
  (.agent.summary | (keys | sort) == ["model", "variant"]) and
  (.agent.compaction | (keys | sort) == ["model", "variant"])
' opencode.jsonc >/dev/null || fail "runtime overrides are wrong"
superpowers_plugin="superpowers@git+https://github.com/obra/superpowers.git"
jq -e --arg plugin "$superpowers_plugin" '.plugin == [$plugin]' opencode.jsonc >/dev/null || fail "raw project config must declare the exact Superpowers plugin spec"
jq -e --arg plugin "$superpowers_plugin" '([.plugin[]? | select(. == $plugin)] | length) == 1' "$resolved" >/dev/null || fail "resolved config must contain the exact Superpowers plugin spec once"

general_json="$(mktemp)"
tmpfiles+=("$general_json")
opencode debug agent general >"$general_json" || fail "general agent does not resolve"
jq -e '
  .mode == "subagent" and
  .model.providerID == "openai" and
  .model.modelID == "gpt-5.6-terra" and
  .variant == "high"
' "$general_json" >/dev/null || fail "resolved general profile is wrong"

references=(rpg-toolkit rpg-api rpg-api-protos rpg-dnd5e-web rpg-deployment rpg-game-assets)
jq -e --argjson aliases '["rpg-toolkit","rpg-api","rpg-api-protos","rpg-dnd5e-web","rpg-deployment","rpg-game-assets"]' '
  . as $config |
  ($config.references | keys | sort) == ($aliases | sort) and
  all($aliases[] as $alias | ($config.references[$alias].path == ("../" + $alias)) and
                            ($config.references[$alias].description | type == "string" and length > 0))
' "$resolved" >/dev/null || fail "references must use aliases with exact paths and descriptions"
for server in hub-tools hub-tools-staging linear goat-drops; do
  jq -e --arg server "$server" '.mcp[$server].enabled == false' "$resolved" >/dev/null || fail "MCP $server is not disabled"
done
jq -e '.mcp["chrome-devtools"].enabled == true and .mcp["chrome-devtools"].type == "local" and .mcp["chrome-devtools"].command == ["npx", "-y", "chrome-devtools-mcp@latest", "--browserUrl=http://127.0.0.1:9222", "--no-usage-statistics"]' "$resolved" >/dev/null || fail "chrome-devtools configuration is wrong"

assert_permission_rule() {
  local file="$1" permission="$2" action="$3" pattern="$4"
  jq -e --arg permission "$permission" --arg action "$action" --arg pattern "$pattern" '
    any(.permission[]; .permission == $permission and .action == $action and .pattern == $pattern)
  ' "$file" >/dev/null || fail "missing resolved $permission $action $pattern rule"
}

bash_action() {
  local file="$1" command="$2" action pattern resolved=""
  while IFS=$'\t' read -r action pattern; do
    case "$command" in
      $pattern) resolved="$action" ;;
    esac
  done < <(jq -r '.permission[] | select(.permission == "bash") | [.action, .pattern] | @tsv' "$file")
  printf '%s\n' "$resolved"
}

agents=(ui-lead platform-lead assets-lead rpg-toolkit-member rpg-api-member rpg-api-protos-member rpg-deployment-member rpg-game-assets-member ui-web-member assets-web-member toolkit-fixer api-fixer web-fixer independent-gate explore janitor)
declare -A expected_mode expected_model expected_variant
for agent in ui-lead platform-lead assets-lead; do
  expected_mode[$agent]=primary
  expected_model[$agent]=gpt-5.6-sol-fast
  expected_variant[$agent]=xhigh
done
for agent in rpg-toolkit-member rpg-api-member rpg-api-protos-member rpg-deployment-member rpg-game-assets-member ui-web-member assets-web-member toolkit-fixer api-fixer web-fixer; do
  expected_mode[$agent]=subagent
  expected_model[$agent]=gpt-5.6-terra
  expected_variant[$agent]=high
done
expected_mode[independent-gate]=subagent
expected_model[independent-gate]=gpt-5.6-sol
expected_variant[independent-gate]=max
expected_mode[explore]=subagent
expected_model[explore]=gpt-5.6-luna
expected_variant[explore]=medium
expected_mode[janitor]=subagent
expected_model[janitor]=gpt-5.6-luna
expected_variant[janitor]=low

for agent in "${agents[@]}"; do
  source=".opencode/agents/$agent.md"
  require_file "$source"
  agent_json="$(mktemp)"
  tmpfiles+=("$agent_json")
  opencode debug agent "$agent" >"$agent_json" || fail "agent $agent does not resolve"
  jq -e --arg mode "${expected_mode[$agent]}" --arg model "${expected_model[$agent]}" --arg variant "${expected_variant[$agent]}" '
    .mode == $mode and
    .model.providerID == "openai" and
    .model.modelID == $model and
    .variant == $variant
  ' "$agent_json" >/dev/null || fail "resolved adapter profile is wrong for $agent"

  case "$agent" in
    ui-lead) paths=(docs/teams/roles/director/prompt.md docs/teams/roles/director/field-notes.md docs/teams/roles/director/overlays/ui-ux.md) ;;
    platform-lead) paths=(docs/teams/roles/director/prompt.md docs/teams/roles/director/field-notes.md docs/teams/roles/director/overlays/platform.md) ;;
    assets-lead) paths=(docs/teams/roles/director/prompt.md docs/teams/roles/director/field-notes.md docs/teams/roles/director/overlays/assets.md) ;;
    rpg-toolkit-member) paths=(docs/teams/roles/rpg-toolkit-member/prompt.md) ;;
    rpg-api-member) paths=(docs/teams/roles/rpg-api-member/prompt.md) ;;
    rpg-api-protos-member) paths=(docs/teams/roles/rpg-api-protos-member/prompt.md) ;;
    rpg-deployment-member) paths=(docs/teams/roles/rpg-deployment-member/prompt.md) ;;
    rpg-game-assets-member) paths=(docs/teams/roles/rpg-game-assets-member/prompt.md) ;;
    ui-web-member) paths=(docs/teams/roles/rpg-dnd5e-web-member/prompt.md docs/teams/roles/rpg-dnd5e-web-member/overlays/ui-ux.md) ;;
    assets-web-member) paths=(docs/teams/roles/rpg-dnd5e-web-member/prompt.md docs/teams/roles/rpg-dnd5e-web-member/overlays/assets.md) ;;
    toolkit-fixer) paths=(docs/teams/roles/toolkit-fixer/prompt.md) ;;
    api-fixer) paths=(docs/teams/roles/api-fixer/prompt.md) ;;
    web-fixer) paths=(docs/teams/roles/web-fixer/prompt.md) ;;
    independent-gate) paths=(docs/teams/roles/independent-gate/prompt.md) ;;
    explore) paths=(docs/teams/roles/explore/prompt.md) ;;
    janitor) paths=(docs/teams/roles/janitor/prompt.md) ;;
  esac
  for path in "${paths[@]}"; do
    grep -Fqx -- "- \`$path\`" "$source" || fail "literal canonical pointer $path is missing from $agent"
  done

  if [[ "$agent" == ui-lead || "$agent" == platform-lead || "$agent" == assets-lead ]]; then
    assert_permission_rule "$agent_json" edit deny "*"
    assert_permission_rule "$agent_json" bash deny "*"
    assert_permission_rule "$agent_json" bash allow "gh *"
    assert_permission_rule "$agent_json" bash deny "gh pr merge*"
    assert_permission_rule "$agent_json" bash deny "gh * pr merge*"
    jq -e '
      [.permission | to_entries[] | select(.value.permission == "bash") | {index: .key, action: .value.action, pattern: .value.pattern}] as $rules |
      ([ $rules[] | select(.action == "deny" and .pattern == "*") | .index ] | max) as $broad_deny |
      ([ $rules[] | select(.action == "allow" and .pattern == "gh *") | .index ] | max) as $gh_allow |
      any($rules[]; .action == "allow" and .pattern == "gh *" and .index > $broad_deny) and
      any($rules[]; .action == "deny" and .pattern == "gh pr merge*" and .index > $gh_allow) and
      any($rules[]; .action == "deny" and .pattern == "gh * pr merge*" and .index > $gh_allow)
    ' "$agent_json" >/dev/null || fail "lead gh merge deny must follow broad gh allow"
    [ "$(bash_action "$agent_json" "gh issue view 101")" = allow ] || fail "lead gh coordination must remain allowed"
    [ "$(bash_action "$agent_json" "gh pr merge 103")" = deny ] || fail "lead gh pr merge must resolve to the trailing deny"
    [ "$(bash_action "$agent_json" "gh -R KirkDiggler/rpg-project pr merge 103")" = deny ] || fail "lead repo-flag gh pr merge must resolve to the trailing deny"
    case "$agent" in
      ui-lead) task_allowlist=(ui-web-member web-fixer explore independent-gate janitor) ;;
      platform-lead) task_allowlist=(rpg-toolkit-member rpg-api-member rpg-api-protos-member rpg-deployment-member toolkit-fixer api-fixer explore independent-gate janitor) ;;
      assets-lead) task_allowlist=(rpg-game-assets-member assets-web-member web-fixer explore independent-gate janitor) ;;
    esac
    task_allowlist_json="$(printf '%s\n' "${task_allowlist[@]}" | jq -R . | jq -s .)"
    jq -e --argjson allowlist "$task_allowlist_json" '
      [.permission[] | select(.permission == "task") | {action: .action, pattern: .pattern}] ==
      ([{action: "deny", pattern: "*"}] + [$allowlist[] | {action: "allow", pattern: .}])
    ' "$agent_json" >/dev/null || fail "lead task permissions must be broad deny followed by the exact manifest allowlist"
  else
    assert_permission_rule "$agent_json" task deny "*"
  fi
done

gate_json="$(mktemp)"
tmpfiles+=("$gate_json")
opencode debug agent independent-gate >"$gate_json"
assert_permission_rule "$gate_json" edit allow "*"
assert_permission_rule "$gate_json" task deny "*"
assert_permission_rule "$gate_json" bash allow "*"
for pattern in 'git *commit*' 'git *push*' 'git *merge*' 'gh pr merge*' 'gh * pr merge*'; do
  assert_permission_rule "$gate_json" bash deny "$pattern"
  jq -e --arg pattern "$pattern" '
    [.permission | to_entries[] | select(.value.permission == "bash") | {index: .key, action: .value.action, pattern: .value.pattern}] as $rules |
    ([ $rules[] | select(.action == "allow" and .pattern == "*") | .index ] | max) as $broad_allow |
    any($rules[]; .action == "deny" and .pattern == $pattern and .index > $broad_allow)
  ' "$gate_json" >/dev/null || fail "gate deny $pattern must follow broad allow"
done
[ "$(bash_action "$gate_json" "gh -R KirkDiggler/rpg-project pr merge 103")" = deny ] || fail "gate repo-flag gh pr merge must resolve to the trailing deny"

explore_json="$(mktemp)"
tmpfiles+=("$explore_json")
opencode debug agent explore >"$explore_json"
assert_permission_rule "$explore_json" edit deny "*"

printf "PASS: OpenCode project configuration verified without a model call\n"
```

- [ ] **Step 8: Run the required OpenCode validation set**

```bash
./scripts/verify-opencode.sh
opencode debug config
opencode agent list
for agent in ui-lead platform-lead assets-lead rpg-toolkit-member rpg-api-member rpg-api-protos-member rpg-deployment-member rpg-game-assets-member ui-web-member assets-web-member toolkit-fixer api-fixer web-fixer independent-gate explore janitor; do opencode debug agent "$agent"; done
opencode models openai --verbose
opencode debug file read AGENTS.md
opencode mcp list || printf 'WARN: Chrome may be disconnected at http://127.0.0.1:9222; resolved config is the required proof\n'
```

Expected: verifier prints `PASS: OpenCode project configuration verified without a model call`; config shows six references and four disabled MCPs; all 16 adapters resolve with manifest model/mode/variant; OpenAI models list includes the configured model IDs; `AGENTS.md` resolves through the Git symlink; MCP listing contains configured `chrome-devtools` without requiring its port to be live.

- [ ] **Step 9: Commit the runtime PR**

Run `git diff --check`, then commit:

```bash
git add AGENTS.md opencode.jsonc .opencode scripts/verify-opencode.sh
git commit -m "feat(workflow): add project-scoped OpenCode runtime (#RUNTIME_ISSUE)"
```

Expected: commit succeeds without `--no-verify`.

- [ ] **Step 10: Push and open the ready runtime PR**

Push the committed branch, open a ready PR with `Closes #RUNTIME_ISSUE`, set
Project 19 to `In Review`, and publish all red/green evidence. Do not call the
PR merge-ready.

- [ ] **Step 11: Self-review and hand off the runtime setup PR**

Inspect the ready PR against design sections 4, 5, 6, 7, and 11; rerun
`./scripts/verify-opencode.sh` and `git diff --check`; then post the exact
verification output, self-review, blockers, and next action. Do not request an
independent gate: configuration, adapters, workflow skills, and verifier scripts
are setup under §11. Kirk reviews and decides whether to merge; set the item to
`Done` only after that merge.

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

Update `README.md` and `CLAUDE.md` so every statement that says three cloned game repos instead says the ordered seven-repository workspace. Explicitly remove the stale `settings.json` claim: bootstrap synchronizes only workspace memory, not OpenCode settings. State that `rpg-project` owns canonical roles and project-scoped OpenCode configuration; `game-dev` only bootstraps/checks availability and never installs OpenCode, changes global OpenCode configuration, credentials, or provider settings.

- [ ] **Step 8: Run contract and syntax checks green**

```bash
./tests/bootstrap-contract.sh
bash -n bootstrap.sh
bash -n scripts/workspace-repos.sh
bash -n scripts/verify-workspace.sh
```

Expected: `PASS: bootstrap contract verified` and all three syntax commands return exit code `0`. Do not run the bootstrap during this task as its tool-install behavior is not a unit test.

- [ ] **Step 9: Commit the portability PR**

Run `git diff --check`, then commit:

```bash
git add bootstrap.sh .gitignore README.md CLAUDE.md scripts/workspace-repos.sh scripts/verify-workspace.sh tests/bootstrap-contract.sh
git commit -m "feat(workspace): bootstrap seven project repositories (#WORKSPACE_ISSUE)"
```

Expected: commit succeeds without `--no-verify`.

- [ ] **Step 10: Push and open the ready portability PR**

Push the committed branch, open a ready PR with `Closes #WORKSPACE_ISSUE`, set
Project 19 to `In Review`, and publish the red/green transcript. Do not call
the PR merge-ready.

- [ ] **Step 11: Self-review and hand off the portability setup PR**

Inspect the ready PR against design sections 1, 6, 7, and 11; rerun the contract
and syntax checks plus `git diff --check`; then post the exact verification
output, self-review, blockers, and next action. Do not request an independent
gate: bootstrap scripts and workspace documentation are setup under §11. Kirk
reviews and decides whether to merge, then marks the item `Done`. This repo has
no application deploy claim, so no deployment verification is asserted.

### Task 4: Project 19 Clean-Slate Verification

**Files:**
- Create: no files and no code PR.

**Interfaces:**
- Consumes: merged Tasks 1 through 3; a fresh WSL/Linux path or fresh Linux machine; GitHub issue checkpoint comments.
- Produces: a reproducible evidence record that all three OpenCode 1.18.4 global loader inputs are unchanged after initialized-baseline verification and the seven-repo project configuration works locally.

- [ ] **Step 1: Create the Verify issue before the environment run**

Create an `rpg-project` issue titled `Verify OpenCode clean-slate workspace bootstrap`, add it to Project 19 with exactly `Team=Cross-team`, `Feature=Infra`, `Kind=Verify`, and `Status=Todo`, then move it to `In Progress`. This is the explicit issue-only Verify exception to the branch/PR rule: no branch and no code PR are created, but the issue and Project item remain the durable task record. Post `WORK SESSION STARTED`, the exact fresh path or machine identifier, and the expected evidence list.

- [ ] **Step 2: Run the first bootstrap, initialize OpenCode once, and capture the baseline**

In one shell that remains open through Step 6, run the first bootstrap from the
fresh `game-dev` checkout. It must only clone or fetch repositories and report
OpenCode availability; it must not invoke OpenCode or write global configuration.

```bash
set -euo pipefail
./bootstrap.sh
```

Then initialize OpenCode's normal first-run state exactly once before taking the
baseline. OpenCode 1.18.4 loads `config.json`, `opencode.json`, and
`opencode.jsonc`; when all are absent, its first command may create one. This
intentional initialization is outside bootstrap and prevents that normal
first-run write from contaminating the no-overwrite comparison.

```bash
cd rpg-project
opencode debug config >/dev/null
cd ..

snapshot_global_opencode_config() {
  for config in "$HOME/.config/opencode/config.json" "$HOME/.config/opencode/opencode.json" "$HOME/.config/opencode/opencode.jsonc"; do
    if [ -e "$config" ]; then
      sha256sum -- "$config"
    else
      printf 'ABSENT %s\n' "$config"
    fi
  done
}

before_snapshot="$(mktemp "${TMPDIR:-/tmp}/opencode-global-config-before.XXXXXX")"
snapshot_global_opencode_config | tee "$before_snapshot"
printf 'Before snapshot evidence: %s\n' "$before_snapshot"
```

Paste the emitted three-line snapshot and evidence-file path into the issue.
Any still-absent entry is valid. Do not manually create an entry file or its
parent path, copy a template into global configuration, authenticate, or alter
provider credentials.

- [ ] **Step 3: Run the second bootstrap from the initialized baseline**

```bash
./bootstrap.sh
```

Expected: the two bootstrap runs together attempt or confirm exactly seven
ordered clones from `workspace_repos`; the OpenCode section only reports
availability or a warning and performs no install or global write. Record
clone/fetch results and any SSH/auth blocker in the issue instead of silently
retrying it.

- [ ] **Step 4: Verify the local workspace and project runtime**

```bash
./scripts/verify-workspace.sh
cd rpg-project
opencode debug config
opencode agent list
opencode debug file read AGENTS.md
opencode mcp list || printf 'WARN: Chrome may be disconnected at http://127.0.0.1:9222; resolved project config remains the required proof\n'
```

Expected: `PASS: seven-repository workspace verified`; all seven `.git` directories and GitHub remotes exist; `rpg-project/AGENTS.md` resolves to `CLAUDE.md`; project verification passes; the runtime lists six sibling references and configured adapters. Chrome may be unavailable or `opencode mcp list` may exit non-zero; the resolved enabled/type/command configuration is the required proof, not a live browser connection.

- [ ] **Step 5: Capture the after snapshot and compare**

In the same shell, run this exact block. It saves a second evidence file outside
the repository and compares the complete ordered snapshots with `diff -u`:

```bash
after_snapshot="$(mktemp "${TMPDIR:-/tmp}/opencode-global-config-after.XXXXXX")"
snapshot_global_opencode_config | tee "$after_snapshot"
printf 'After snapshot evidence: %s\n' "$after_snapshot"
diff -u "$before_snapshot" "$after_snapshot"
```

Expected: `diff -u` emits no output and exits zero. The before and after records
must have identical lines for all three loader input paths: `ABSENT <path>` for
each missing file, or the same SHA-256 plus path for each present file. Record both
snapshots, both evidence-file paths, and the comparison exit status in the
issue. A `diff -u` difference or error is a failed verification: stop, set the
Project item to `In Review`, publish the diff and blocker, and open a linked fix
issue rather than modifying global configuration.

This proof is deliberately limited to the OpenCode 1.18.4 global loader inputs
`$HOME/.config/opencode/config.json`, `$HOME/.config/opencode/opencode.json`,
and `$HOME/.config/opencode/opencode.jsonc`. Normal cache or package population
caused by the project-scoped Superpowers plugin is outside this scope and is not
a bootstrap settings overwrite.

- [ ] **Step 6: Publish the evidence checkpoint, review, and close only on proof**

Post a structured issue comment containing the fresh environment, initialized
baseline, before/after states, both bootstrap outcomes, ordered
repository/remotes proof, `verify-workspace.sh` output, symlink/config/sibling-
access proof, self-review, and explicit next action. After successful evidence,
move the Project item to `In Review` and ask Kirk to confirm the record. Do not
request an independent Sol gate: this issue-only evidence task is setup under
§11. If evidence fails, keep the item `In Review`, publish the diff and blocker,
and open a linked fix issue. Only Kirk's confirmation of successful evidence
moves it to `Done`. No code PR is created for this Verify task.

### Task 5: Equipment Pilot Documentation and Recovery Proof

**Files:**
- Create: no proto, API, toolkit, web, or equipment implementation files in this task.
- Create: `ideas/equipment/design.md` in a separate `rpg-project` documentation issue and ready review PR after Kirk ratifies the three `#187` decisions.

**Interfaces:**
- Consumes: Task 4 evidence confirmed by Kirk; existing `rpg-api-protos#187/#188`, `rpg-toolkit#811/#812`, and `rpg-api#680/#682`; Project 19 durable state.
- Produces: a Kirk-ratified decision record, a separately reviewed documentation snapshot, and a GitHub-only recovery evidence route. It does not prescribe product implementation; the existing product PRs retain their own review and gate paths.

- [ ] **Step 1: Verify and adopt the existing implementation chain**

Implementation is already in flight and must be adopted, not discarded or
restarted: `rpg-api-protos#187/#188` is the single-repository protos leg,
`rpg-toolkit#811/#812` is its downstream rules leg, and `rpg-api#680/#682` is
the delivery leg. Re-query their issue, PR, CI, and Project state before each
execution beat. Use `rpg-api-protos#187` as the existing backing issue. Verify
and preserve its live routing: `Team=Platform`, `Feature=Game Screen`, and
`Kind=Decide`. Verify `Status=Todo`, then move only Status from `Todo` to `In
Progress`; do not re-triage Team or Feature. The Platform lead posts `WORK
SESSION STARTED`, links the Task 4 Verify issue confirmed by Kirk, and records
adoption of the existing `#188` branch/PR.

For the existing downstream issues, make their durable Project 19 triage exact:
`rpg-toolkit#811` and `rpg-api#680` are each `Team=Platform`, `Feature=Game
Screen`, `Kind=Build`, `Status=In Progress`. Their open draft PRs, `#812` and
`#682`, remain their one-issue/one-PR units; this plan does not create
replacement issues or branches.

- [ ] **Step 2: Obtain the written three-decision ratification**

`#issuecomment-5027810963` is a recommendation, not a final human decision.
Before documenting or merging product behavior, Kirk posts a written `#187`
decision comment that ratifies or explicitly changes each answer:

1. `EquipItem.slot_key` is player-provided and required for an explicit target.
2. `Item.slot_keys` is authoritative for compatibility; `SlotDef.accepts` is
   display/filter data.
3. `inventory` includes equipped items.

The Platform lead records the ratification URL and confirms whether `#188`
matches it. Do not treat the implementation branch as decision authority. The
minor two-hander-blocked marker remains non-blocking unless a player-facing UI
requirement makes it necessary. If Kirk changes an answer, route the discrepancy
to the original `#188` implementer before product review; do not create a
replacement issue or branch.

- [ ] **Step 3: Create and prove the documentation setup snapshot**

Create a new `rpg-project` issue titled `Equipment documentation and
implementation plan`, add it to Project 19 with exactly `Team=Platform`,
`Feature=Game Screen`, `Kind=Build`, `Status=Todo`, then set `In Progress`.
Create a fresh `docs/EQUIPMENT_PLAN_ISSUE-equipment-plan` branch, where
`EQUIPMENT_PLAN_ISSUE` is the issue number just created. First run this RED
check, which must fail because the missing design document is absent:

```bash
test -f ideas/equipment/design.md
```

Create `ideas/equipment/design.md` with exactly these headings and durable
pointers, without proto field definitions, toolkit logic, API implementation,
or web implementation instructions:

```markdown
# Equipment Slice Design

## Decision Record

Kirk's written `rpg-api-protos#187` ratification: <ratification comment URL>.
The recommendation context is `#issuecomment-5027810963`.

## Adopted Product Chain

`rpg-api-protos#187/#188` is the contract leg. `rpg-toolkit#811/#812` depends
on the merged contract. `rpg-api#680/#682` depends on both prior legs and bumps
their dependency versions before CI can pass. Each is one issue and one PR.

## Review Boundary

This document, its issue, and its ready PR are workflow setup: deterministic
checks, self-review, and Kirk's merge apply. The product PRs #188, #812, and
#682 each receive normal review plus exactly one independent Sol gate.
```

Run this GREEN check before committing:

```bash
bash -euo pipefail -c '
  test -f ideas/equipment/design.md
  grep -Fqx "# Equipment Slice Design" ideas/equipment/design.md
  grep -Fqx "## Decision Record" ideas/equipment/design.md
  grep -Fqx "## Adopted Product Chain" ideas/equipment/design.md
  grep -Fqx "## Review Boundary" ideas/equipment/design.md
  grep -Fq "<ratification comment URL>" ideas/equipment/design.md && {
    printf "FAIL: replace the ratification URL before commit\n" >&2
    exit 1
  }
  grep -Fq "rpg-api-protos#187/#188" ideas/equipment/design.md
  grep -Fq "rpg-toolkit#811/#812" ideas/equipment/design.md
  grep -Fq "rpg-api#680/#682" ideas/equipment/design.md
'
```

Expected: RED exits non-zero before creation; GREEN exits zero only after the
real ratification URL and every chain/dependency boundary is present. Self-
review, commit, push, and open a ready documentation PR. Do not request Sol:
this documentation review is setup under §11. Kirk reviews and decides whether
to merge.

- [ ] **Step 4: Prove deliberate worker replacement on the adopted protos branch**

After Tasks 1 through 3 are merged, Task 4 is confirmed by Kirk, the decision
ratification is visible, and the documentation snapshot is merged, the Platform
lead starts a Terra worker on the existing `rpg-api-protos#187/#188` branch/PR.
The worker adopts rather than restarts the branch, records a complete GitHub
checkpoint with completed work, commands and results, blockers, branch, current
commit, and explicit next action, ending `— rpg-api-protos-member, on behalf of
KirkDiggler`, and is deliberately terminated only after that checkpoint is
visible. Before the recovery exercise, Kirk visibly completes the machine-local
interactive `/connect` flow for OpenAI. No worker authenticates, writes
credentials, or edits global OpenCode configuration; missing local auth is a
GitHub-visible human blocker.

- [ ] **Step 5: Replace from durable state, then route the product PRs**

Launch a new Terra worker with only the `#187` issue URL, `#188` branch and PR
URL, and the first worker's checkpoint comment. It must reconstruct from those
GitHub artifacts rather than an inherited `task_id` or local transcript, carry
the existing `#188` through normal review and publish its own checkpoint. The
replacement proof is evidence-only setup; it does not add a separate gate.

`#188`, `#812`, and `#682` are product-behavior PRs, not setup. Respect their
dependency order: merge the protos contract before the toolkit rules leg, then
the API delivery leg after its dependency versions are bumped. On each product
PR's final ready head, run normal review plus exactly one independent Sol gate.
The reviewer reads the product diff, claims, tests, and evidence; does not fix,
commit, push, merge, or alter Project fields. Findings return to the original
implementer, and that same reviewer performs the focused recheck of reported
findings and changed surfaces. Commission a new unrestricted audit only if
remediation materially rewrites scope. Kirk alone decides and merges. Perform
deployment verification only where the merged repository has an applicable
release path.

`#811/#812` and `#680/#682` remain existing downstream legs outside this
single-PR recovery experiment. Preserve their dependency and merge order:
the adopted protos `#187/#188` contract precedes the toolkit `#811/#812` rules
leg, and both precede the API `#680/#682` delivery leg whose dependency versions
must be bumped before it can pass CI. Each remains a separate one-issue/one-PR
unit with normal review, exactly one independent Sol gate, same-reviewer focused
recheck after findings, and Kirk-only merge authority; neither disappears or
restarts because the protos leg hosts the replacement proof.

## Final Review Checklist

- [ ] Re-read `ideas/opencode-team-workflow/design.md` sections 1 through 11 and record this exact coverage map in the self-review comment: section 1 (control-plane architecture) is Tasks 1 and 2; section 2 (Project 19 contract and artifact lifecycle) is Tasks 1 through 5; section 3 (durable continuity) is Tasks 2, 4, and 5; section 4 (roles and roster) is Task 1; section 5 (model profile) is Task 2; section 6 (configuration and source of truth) is Tasks 2 and 3; section 7 (operational gates) is Tasks 1 through 5; section 8 (side-by-side verification) is Tasks 4 and 5; section 9 (equipment pilot) is Task 5; section 10 (rollout issue) is this Plan Review PR tracking #101; section 11 (approved gate policy) is the global lifecycle plus every Task 1 through 5 handoff.
- [ ] Search the plan and lifecycle corrections for unresolved-marker text, time estimates, and vague-test language; replace every occurrence that would leave an implementer to invent behavior. Task 5 requires Task 4 evidence confirmed by Kirk, a written `#187` ratification, exact documentation RED/GREEN checks, and adoption of the existing `#187/#188`, `#811/#812`, and `#680/#682` chain.
- [ ] Check every adapter name, literal canonical path pointer, model, variant, mode, and permission against **Adapter Manifest** and `opencode.jsonc`; check exact OpenAI model IDs, six alias-keyed sibling references with only `{path,description}`, all 16 adapters, exactly four disabled inherited work MCPs, and hidden `title`/`summary`/`compaction` overrides limited to model and variant.
- [ ] Confirm all required validation commands appear exactly: `opencode debug config`, `opencode agent list`, `opencode debug agent NAME`, `opencode models openai --verbose`, `opencode debug file read AGENTS.md`, and `opencode mcp list`.
- [ ] Confirm `enabled_providers` is only OpenAI; the raw and resolved configuration each contain the exact project-scoped Superpowers plugin spec; no prohibited config keys, unexpected plugin, daemon, checkpoint schema, duplicate tracker, dry-run rewrite, new test framework, Anthropic/Claude/Sonnet model, or equipment implementation details appear. Confirm all shell snippets parse as Bash where applicable, including the verifier cleanup, the jq blocks, Task 1's fail-fast partial-state probe, Task 4's deterministic three-entry initialized-baseline snapshot, and Task 5's fail-fast documentation GREEN check.
- [ ] Search for placeholders, `similar to`, unresolved markers, impossible lifecycle order, stale `settings.json` claims, and vague acceptance instructions. Confirm Tasks 1 through 3 use create -> red/green -> self-review -> commit -> push/ready PR -> Kirk's review and merge decision. Confirm Task 4 is issue-only evidence and Task 5's documentation/recovery work follows the same setup route.
- [ ] Confirm only product PRs #188, #812, and #682 receive normal review plus exactly one independent Sol gate. Confirm findings return to the original implementer and the same reviewer performs a focused recheck; a new unrestricted audit occurs only after a material scope rewrite. Confirm no setup task, documentation PR, configuration/role/adapter/bootstrap/verifier PR, evidence-only Verify issue, or PR #103 receives an independent adversarial gate.
- [ ] Confirm Task 4 initializes OpenCode once before its baseline; saves before and after evidence outside the repository; compares `config.json`, `opencode.json`, and `opencode.jsonc` records with `diff -u`; does not manually create entry files or touch credentials; and treats Superpowers cache/package population as outside the global-settings proof.
- [ ] Confirm the exact issue-only Task 4 lifecycle: its backing issue and Project item move `Todo -> In Progress -> In Review`; it creates no branch or PR; successful evidence moves it to `In Review` for Kirk's confirmation; failure keeps it `In Review` with a linked fix issue; only Kirk's confirmation moves it to `Done`.
- [ ] Run `git diff --check`, inspect `git status --short --branch`, and inspect `git diff -- ideas/opencode-team-workflow/plan.md`; fix every discrepancy inline before committing.
- [ ] Commit only the intended changed `ideas/opencode-team-workflow` documentation files for this remediation snapshot; never amend and never use `--no-verify`.

## Plan Review Handoff

Push `docs/101-opencode-team-workflow-plan` and open a ready PR against `main` titled `docs(ideas): OpenCode team workflow implementation plan (#101)`. Use this body structure:

```markdown
## Review phase: Plan Review

## Summary

This plan establishes five coherent units: canonical role policy, project-scoped OpenCode runtime, portable seven-repository bootstrap, clean-slate verification, and equipment decision/documentation/recovery routing.

## Task boundaries

- Task 1: canonical role system
- Task 2: project OpenCode runtime
- Task 3: portable seven-repository workspace
- Task 4: clean-slate Verify issue
- Task 5: equipment decision, documentation, and recovery routing

## Review focus

- OpenCode 1.18.4 configuration and adapter mechanics
- Charter/overlay ownership and permission boundaries
- Red/green verifier and non-mutating bootstrap contracts
- Lean §11 setup policy and product-only Sol gate routing
- GitHub-only recovery proof and product dependency order

## Verification

- Design §11 coverage, unresolved-marker scan, model/path/config consistency, deterministic shell/config probes, and `git diff --check` completed

Tracks #101

— cross-team Terra plan writer, on behalf of KirkDiggler
```

Publish an issue #101 checkpoint with the PR URL and `Next action: Kirk reviews the Plan Review PR and decides whether to merge; no further setup gate.` End it with `— cross-team Terra plan writer, on behalf of KirkDiggler`. Do not merge the plan PR.
