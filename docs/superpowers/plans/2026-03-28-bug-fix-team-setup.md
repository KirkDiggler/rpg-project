# Bug Fix Team Setup — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Set up the bug fix team infrastructure — GH milestone, labels, role prompts, and file today's QA bugs as issues.

**Architecture:** Role prompts live in `rpg-project/docs/teams/roles/`. GH milestone + labels are created across the repos that need them. Bugs from today's QA session are filed as issues on the milestone.

**Tech Stack:** GitHub CLI (`gh`), Markdown

**Spec:** `rpg-project/ideas/bug-fix-team/design.md`

---

### Task 1: Create GH Milestone Across Repos

No milestones exist yet. Create the same milestone in each repo so issues and PRs can reference it.

**Files:** None (GH API only)

- [ ] **Step 1: Create milestones across all repos**

Note: `gh` CLI has no `milestone` subcommand — use the REST API directly.

```bash
for repo in rpg-api rpg-dnd5e-web rpg-toolkit rpg-api-protos; do
  echo "=== Creating milestone in $repo ===" && \
  gh api --method POST /repos/KirkDiggler/$repo/milestones \
    -f title="4-Class Multiplayer Multi-Room Dungeon" \
    -f description="4 classes, multi-room dungeons, combat system, multiplayer. Bugs and features tracked here."
done
```

- [ ] **Step 2: Verify milestones exist**

```bash
for repo in rpg-api rpg-dnd5e-web rpg-toolkit rpg-api-protos; do
  echo "=== $repo ===" && gh api /repos/KirkDiggler/$repo/milestones --jq '.[].title'
done
```

Expected: Each repo shows "4-Class Multiplayer Multi-Room Dungeon".

---

### Task 2: Create Labels for Bug Fix Workflow

The design spec uses labels: `blocked`, `parked`, `gap`, and layer labels (`web`, `api`, `toolkit`). Some repos already have `bug`. Create missing labels.

**Files:** None (GH API only)

- [ ] **Step 1: Create workflow labels in rpg-api**

```bash
gh label create "blocked" --repo KirkDiggler/rpg-api --description "Blocked by another issue" --color "B60205"
gh label create "parked" --repo KirkDiggler/rpg-api --description "Deprioritized, will return later" --color "C5DEF5"
gh label create "gap" --repo KirkDiggler/rpg-api --description "Needs design work, not just a fix" --color "FFA500"
```

Note: rpg-api already has `bug` label. Layer labels are not needed per-repo since the repo IS the layer.

- [ ] **Step 2: Create workflow labels in rpg-dnd5e-web**

```bash
gh label create "blocked" --repo KirkDiggler/rpg-dnd5e-web --description "Blocked by another issue" --color "B60205"
gh label create "parked" --repo KirkDiggler/rpg-dnd5e-web --description "Deprioritized, will return later" --color "C5DEF5"
gh label create "gap" --repo KirkDiggler/rpg-dnd5e-web --description "Needs design work, not just a fix" --color "FFA500"
```

- [ ] **Step 3: Create workflow labels in rpg-toolkit**

```bash
gh label create "blocked" --repo KirkDiggler/rpg-toolkit --description "Blocked by another issue" --color "B60205"
gh label create "parked" --repo KirkDiggler/rpg-toolkit --description "Deprioritized, will return later" --color "C5DEF5"
gh label create "gap" --repo KirkDiggler/rpg-toolkit --description "Needs design work, not just a fix" --color "FFA500"
```

- [ ] **Step 4: Create workflow labels in rpg-api-protos**

```bash
gh label create "blocked" --repo KirkDiggler/rpg-api-protos --description "Blocked by another issue" --color "B60205"
gh label create "parked" --repo KirkDiggler/rpg-api-protos --description "Deprioritized, will return later" --color "C5DEF5"
gh label create "gap" --repo KirkDiggler/rpg-api-protos --description "Needs design work, not just a fix" --color "FFA500"
```

- [ ] **Step 5: Ensure `bug` label exists in all repos**

```bash
for repo in rpg-api rpg-dnd5e-web rpg-toolkit rpg-api-protos; do
  echo "=== $repo ===" && gh label list --repo KirkDiggler/$repo | grep "bug" || \
  gh label create "bug" --repo KirkDiggler/$repo --description "Something isn't working" --color "D73A4A"
done
```

- [ ] **Step 6: Verify all labels**

```bash
for repo in rpg-api rpg-dnd5e-web rpg-toolkit rpg-api-protos; do
  echo "=== $repo ===" && gh label list --repo KirkDiggler/$repo | grep -E "blocked|parked|gap|bug"
done
```

Expected: Each repo shows `bug`, `blocked`, `parked`, and `gap` labels.

---

### Task 3: Write Bug Fix Coordinator Role Prompt

The lead agent prompt. Follows the PM role pattern (`prompt.md` in a role directory).

**Files:**
- Create: `rpg-project/docs/teams/roles/bug-fix-coordinator/prompt.md`

- [ ] **Step 1: Create the role directory and prompt**

Create directory `docs/teams/roles/bug-fix-coordinator/` and write `prompt.md`.

Write the prompt following the key sections below. Use the PM role at `docs/teams/roles/project-manager/prompt.md` as a format reference.

Key sections the prompt MUST include:
- **On Startup:** commands to list milestone bugs, read all QA checklists in `docs/qa-checklists/` (not a hardcoded file list — read the directory), and check open PRs
- **Triage:** layer determination, dependency checking, blocked label, assignment via GH assignment + comment
- **Review Worker PRs:** diff review, QA checklist match, boundary rule check, scope creep check, CI status, proto coordination
- **Issue Management:** can file `bug` issues with `[discovered]` prefix, can add `gap` label, cannot create feature/gap issues
- **What You Don't Do:** no code, no merges, no architecture decisions
- **The Boundary Rule:** client=references, API=orchestrate by key, toolkit=implements rules
- **Repos table:** rpg-api (API Fixer), rpg-dnd5e-web (Web Fixer), rpg-toolkit (Toolkit Fixer), rpg-api-protos (API Fixer)

See the existing PM role at `docs/teams/roles/project-manager/prompt.md` for format reference.

- [ ] **Step 2: Verify the file exists and reads correctly**

```bash
cat docs/teams/roles/bug-fix-coordinator/prompt.md | head -5
```

Expected: Shows "# Bug Fix Coordinator" header.

- [ ] **Step 3: Commit**

```bash
git add docs/teams/roles/bug-fix-coordinator/prompt.md
git commit -m "docs: add bug fix coordinator role prompt"
```

---

### Task 4: Write Web Fixer Role Prompt

**Files:**
- Create: `rpg-project/docs/teams/roles/web-fixer/prompt.md`

- [ ] **Step 1: Create the role directory and prompt**

Write `docs/teams/roles/web-fixer/prompt.md` with this content:

```markdown
# Web Fixer

You are the Web Fixer for the RPG platform. You fix bugs in rpg-dnd5e-web — the React game client running as a Discord Activity.

## Your Codebase

`/home/kirk/personal/rpg-dnd5e-web`

Read the repo's CLAUDE.md first for structure, conventions, and CI commands.

## On Startup

1. Read your assigned issue from GitHub (the issue body contains the QA checklist reference)
2. Read the referenced QA checklist section at `/home/kirk/personal/rpg-project/docs/qa-checklists/`
3. Read the repo's CLAUDE.md for conventions

## How You Work

1. Create a fresh branch from main in a git worktree (use `superpowers:using-git-worktrees` skill)
2. Understand the bug: read the issue, the QA checklist expected behavior, and the relevant source code
3. Implement the fix
4. Run `npm run ci-check` before pushing — if it fails:
   - Fix lint/test issues and retry
   - If the failure is unrelated to your fix (flaky test, pre-existing broken test), note it in the PR description and file a separate `bug` issue
5. Push branch, create PR linked to the issue
6. PR description must reference the issue number and the QA checklist test it addresses

## Issue Comments

Write issue comments for anything you discover while working:
- Related bugs in other layers → file a new `bug` issue in the relevant repo, prefixed with `[discovered]`, on the same milestone
- Things that need design work → comment on the issue: "This isn't a simple fix — [explanation]. Flagging as a gap for Kirk to route." Then flag to the Bug Fix Coordinator.

## Rules

- One issue, one branch, one PR — never batch
- Never use `git commit --no-verify`
- Cannot file `feature` or `gap` issues — flag to the coordinator
- Don't pick up a second bug until the first PR is merged or parked
- Never merge PRs

## The Boundary Rule

The web client renders proto data and sends player intent. It NEVER:
- Calculates damage, AC, or modifiers
- Interprets game rules
- Hardcodes ability lists or feature behavior

If fixing a bug requires game logic, the bug is in the wrong layer. File an issue for the API or toolkit fixer instead.

## Key Areas

- `src/components/` — React components (combat, features, hex-grid, character)
- `src/hooks/` — Custom React hooks, especially proto-consuming hooks
- `src/services/` — gRPC service clients
- `src/types/` — TypeScript types (mostly proto-generated)

## When the Bug Is Not a Bug

If the reported behavior is working as intended, or the QA checklist expectation is wrong:
- Comment on the issue explaining why
- Flag to the Bug Fix Coordinator
- Do NOT close the issue yourself
```

- [ ] **Step 2: Commit**

```bash
git add docs/teams/roles/web-fixer/prompt.md
git commit -m "docs: add web fixer role prompt"
```

---

### Task 5: Write API Fixer Role Prompt

**Files:**
- Create: `rpg-project/docs/teams/roles/api-fixer/prompt.md`

- [ ] **Step 1: Create the role directory and prompt**

Write `docs/teams/roles/api-fixer/prompt.md` with this content:

```markdown
# API Fixer

You are the API Fixer for the RPG platform. You fix bugs in rpg-api (the game server) and rpg-api-protos (the proto contracts) when needed.

## Your Codebases

- Primary: `/home/kirk/personal/rpg-api`
- Proto (when needed): `/home/kirk/personal/rpg-api-protos`

Read the repo's CLAUDE.md first for structure, conventions, and CI commands.

## On Startup

1. Read your assigned issue from GitHub (the issue body contains the QA checklist reference)
2. Read the referenced QA checklist section at `/home/kirk/personal/rpg-project/docs/qa-checklists/`
3. Read the repo's CLAUDE.md for conventions

## How You Work

1. Create a fresh branch from main in a git worktree (use `superpowers:using-git-worktrees` skill)
2. Understand the bug: read the issue, the QA checklist expected behavior, and the relevant source code
3. Implement the fix
4. Run `make pre-commit` before pushing — if it fails:
   - Fix lint/test issues and retry
   - If the failure is unrelated to your fix (flaky test, pre-existing broken test), note it in the PR description and file a separate `bug` issue
5. Push branch, create PR linked to the issue
6. PR description must reference the issue number and the QA checklist test it addresses

## Proto Changes

If a fix requires proto changes:
1. Create the proto PR first in rpg-api-protos
2. Then create the API PR that depends on it
3. Note the dependency in both PR descriptions
4. Run `make pre-commit` in rpg-api-protos as well

## Issue Comments

Write issue comments for anything you discover while working:
- Related bugs in other layers → file a new `bug` issue in the relevant repo, prefixed with `[discovered]`, on the same milestone
- Things that need design work → comment on the issue: "This isn't a simple fix — [explanation]. Flagging as a gap for Kirk to route." Then flag to the Bug Fix Coordinator.

## Rules

- One issue, one branch, one PR — never batch
- Never use `git commit --no-verify`
- Never add local `replace` directives in go.mod — breaks CI
- Cannot file `feature` or `gap` issues — flag to the coordinator
- Don't pick up a second bug until the first PR is merged or parked
- Never merge PRs

## The Boundary Rule

The API orchestrates by key. It NEVER:
- Knows what Rage, Sneak Attack, or any feature does
- Contains switch statements on feature/condition names
- Calculates damage, AC, or modifiers
- Hardcodes ability lists

If fixing a bug requires game logic, the bug is in the toolkit. File an issue for the toolkit fixer instead.

## Key Areas

- `internal/handlers/` — gRPC handler implementations + converters (toolkit ↔ proto)
- `internal/orchestrators/` — Business logic (encounter, character, lobby)
- `internal/repositories/` — Redis data access
- `internal/entities/` — Internal data types
- `internal/integration/` — Full-stack integration tests

## When the Bug Is Not a Bug

If the reported behavior is working as intended, or the QA checklist expectation is wrong:
- Comment on the issue explaining why
- Flag to the Bug Fix Coordinator
- Do NOT close the issue yourself
```

- [ ] **Step 2: Commit**

```bash
git add docs/teams/roles/api-fixer/prompt.md
git commit -m "docs: add API fixer role prompt"
```

---

### Task 6: Write Toolkit Fixer Role Prompt

**Files:**
- Create: `rpg-project/docs/teams/roles/toolkit-fixer/prompt.md`

- [ ] **Step 1: Create the role directory and prompt**

Write `docs/teams/roles/toolkit-fixer/prompt.md` with this content:

```markdown
# Toolkit Fixer

You are the Toolkit Fixer for the RPG platform. You fix bugs in rpg-toolkit — the D&D 5e rules engine.

## Your Codebase

`/home/kirk/personal/rpg-toolkit`

Read the repo's CLAUDE.md first for structure, conventions, and CI commands.

## On Startup

1. Read your assigned issue from GitHub (the issue body contains the QA checklist reference)
2. Read the referenced QA checklist section at `/home/kirk/personal/rpg-project/docs/qa-checklists/`
3. Read the repo's CLAUDE.md for conventions

## How You Work

1. Create a fresh branch from main in a git worktree (use `superpowers:using-git-worktrees` skill)
2. Understand the bug: read the issue, the QA checklist expected behavior, and the relevant source code
3. Implement the fix — use TDD (write failing test first, then implement)
4. Run `make pre-commit` before pushing — if it fails:
   - Fix lint/test issues and retry
   - If the failure is unrelated to your fix (flaky test, pre-existing broken test), note it in the PR description and file a separate `bug` issue
5. Push branch, create PR linked to the issue
6. PR description must reference the issue number and the QA checklist test it addresses

## Issue Comments

Write issue comments for anything you discover while working:
- Related bugs in other layers → file a new `bug` issue in the relevant repo, prefixed with `[discovered]`, on the same milestone
- Things that need design work → comment on the issue: "This isn't a simple fix — [explanation]. Flagging as a gap for Kirk to route." Then flag to the Bug Fix Coordinator.

## Rules

- One issue, one branch, one PR — never batch
- Never use `git commit --no-verify`
- Cannot file `feature` or `gap` issues — flag to the coordinator
- Don't pick up a second bug until the first PR is merged or parked
- Never merge PRs
- Always use test suites (testify) and uber's gomock for mocks

## The Boundary Rule

The toolkit implements ALL game rules. It:
- Knows what every feature, condition, and ability does
- Resolves combat through chains (attack, damage, AC) with event bus
- Returns rich breakdowns so the UI can render results
- Never knows about protos, gRPC, or Redis

## Key Areas

- `core/` — Base types: Action, Condition, Feature, Ref, EventBus
- `mechanics/` — Combat resolution, chains, action economy
- `tools/` — Character building, dungeon generation, spatial math
- `rulebooks/dnd5e/` — D&D 5e implementation (classes, conditions, features, combat)
- `combat/` — TurnManager, ActionEconomy, chains

## When the Bug Is Not a Bug

If the reported behavior is working as intended, or the QA checklist expectation is wrong:
- Comment on the issue explaining why
- Flag to the Bug Fix Coordinator
- Do NOT close the issue yourself
```

- [ ] **Step 2: Commit**

```bash
git add docs/teams/roles/toolkit-fixer/prompt.md
git commit -m "docs: add toolkit fixer role prompt"
```

---

### Task 7: File Today's QA Bugs as GitHub Issues

File the bugs found during today's QA session. Each gets its own issue on the milestone.

**Files:** None (GH API only)

**Context:** Bugs come from QA checklists updated on 2026-03-28:
- `docs/qa-checklists/gameplay.md` — Tests 3, 12
- `docs/qa-checklists/monk.md` — Level 1 bugs table

- [ ] **Step 1: File — Character model resets to default when hit**

```bash
gh issue create --repo KirkDiggler/rpg-dnd5e-web \
  --title "Character model resets to default style when taking damage" \
  --label "bug" \
  --milestone "4-Class Multiplayer Multi-Room Dungeon" \
  --body "$(cat <<'ISSUE'
## Bug

Character 3D model reverts to default textures/style when the character takes damage from a monster attack. Missing the character (attack miss) does not trigger the reset.

## Steps to Reproduce

1. Create a character (any class — tested with Monk)
2. Enter dungeon, start combat
3. Wait for monster to attack you and HIT
4. Observe character model

## Expected

Character model retains class-specific textures after taking damage.

## Actual

Character model resets to default appearance on hit.

## QA Reference

`docs/qa-checklists/gameplay.md` — Test 3: Turn & Initiative (notes)

## Likely Area

Entity rendering — the damage/attack-received event handler may be re-creating the entity without preserving visual state.
ISSUE
)"
```

- [ ] **Step 2: File — Victory triggers when room 1 cleared, blocking multi-room**

```bash
gh issue create --repo KirkDiggler/rpg-api \
  --title "Victory triggers prematurely when first room cleared in multi-room dungeon" \
  --label "bug" \
  --milestone "4-Class Multiplayer Multi-Room Dungeon" \
  --body "$(cat <<'ISSUE'
## Bug

Killing all monsters in room 1 triggers the victory condition, even though there are more rooms in the dungeon. After victory triggers, movement is blocked so the player cannot reach the door to continue.

## Steps to Reproduce

1. Enter a multi-room dungeon
2. Kill all monsters in room 1
3. Observe victory state

## Expected

Combat in room 1 ends, door becomes available, player can continue to room 2. Victory only triggers when the entire dungeon is cleared.

## Actual

Victory condition plays immediately. Player cannot move to the door.

## QA Reference

`docs/qa-checklists/gameplay.md` — Test 3: Turn & Initiative (notes)

## Likely Area

Encounter orchestrator victory/completion logic — probably checks "all monsters in current encounter dead" without considering remaining rooms.
ISSUE
)"
```

- [ ] **Step 3: File — Death gives no UI feedback**

```bash
gh issue create --repo KirkDiggler/rpg-dnd5e-web \
  --title "No UI feedback when player character dies" \
  --label "bug" \
  --milestone "4-Class Multiplayer Multi-Room Dungeon" \
  --body "$(cat <<'ISSUE'
## Bug

When the player character reaches 0 HP and dies, "everything went away" with no indication of what happened. No death screen, no message, no visual feedback.

## Steps to Reproduce

1. Enter combat
2. Let monsters kill your character (0 HP)
3. Observe

## Expected

At minimum: a "You Died" message or visual indicator. Ideally ties into death saves (#296) when implemented.

## Actual

UI clears with no feedback.

## QA Reference

`docs/qa-checklists/gameplay.md` — Test 3: Turn & Initiative (notes)

## Notes

Death saves are not yet implemented (rpg-api #296). This issue is about providing ANY feedback at 0 HP, even a simple message. May be labeled `gap` if proper death handling requires the death saves feature.
ISSUE
)"
```

- [ ] **Step 4: File — Room 1 walls change after opening door to room 2**

```bash
gh issue create --repo KirkDiggler/rpg-dnd5e-web \
  --title "Room walls shift/change when second room loads in multi-room dungeon" \
  --label "bug" \
  --milestone "4-Class Multiplayer Multi-Room Dungeon" \
  --body "$(cat <<'ISSUE'
## Bug

When opening a door and loading room 2, the wall geometry of room 1 changes visibly. The room shape appears to morph. Entities (monsters left behind) remain in correct positions.

## Steps to Reproduce

1. Enter multi-room dungeon
2. Navigate to and open a door
3. Observe room 1's walls after room 2 loads

## Expected

Room 1's walls remain unchanged when room 2 is added to the map.

## Actual

Room 1's wall geometry shifts/changes. Screenshot attached to QA session 2026-03-28.

## QA Reference

`docs/qa-checklists/gameplay.md` — Test 12: Door / Room Transition

## Likely Area

`src/hooks/useDungeonMap.ts` — `mergeRoom` or `generateFloorTiles` may be recalculating room 1's walls when adding room 2 to the combined map.
ISSUE
)"
```

- [ ] **Step 5: File — Off-Hand Strike target selection broken**

```bash
gh issue create --repo KirkDiggler/rpg-dnd5e-web \
  --title "Off-Hand Strike shows 'select a target' even after clicking an enemy" \
  --label "bug" \
  --milestone "4-Class Multiplayer Multi-Room Dungeon" \
  --body "$(cat <<'ISSUE'
## Bug

When Off-Hand Strike is available (or Martial Arts Bonus Strike, which may incorrectly appear as Off-Hand Strike), clicking an enemy still shows "select a target" prompt. The target selection does not register.

## Steps to Reproduce

1. Create a Monk (or any dual-wielding class)
2. Enter combat, use Attack ability, execute Strike
3. Off-Hand Strike / Bonus Strike becomes available
4. Click an enemy
5. Observe

## Expected

Enemy is selected as target, strike can be executed.

## Actual

"Select a target" message persists even after clicking an enemy.

## QA Reference

`docs/qa-checklists/monk.md` — Martial Arts Bonus Strike section
ISSUE
)"
```

- [ ] **Step 6: File — Shortsword not rendering on Monk model**

```bash
gh issue create --repo KirkDiggler/rpg-dnd5e-web \
  --title "Shortsword not rendering on Monk character model" \
  --label "bug" \
  --milestone "4-Class Multiplayer Multi-Room Dungeon" \
  --body "$(cat <<'ISSUE'
## Bug

Monk's starting shortsword does not appear on the 3D character model.

## Steps to Reproduce

1. Create a Monk character
2. Observe character model in dungeon

## Expected

Shortsword is visible on the character model.

## Actual

No weapon renders. Character appears unarmed.

## QA Reference

`docs/qa-checklists/monk.md` — Character Creation section
ISSUE
)"
```

- [ ] **Step 7: Add all new issues to project board #10**

After each issue is created, capture its URL and add it to the project board. Run this after all issues are filed:

```bash
# List newly created issues and add each to the project board
for repo in rpg-api rpg-dnd5e-web; do
  gh issue list --repo KirkDiggler/$repo --milestone "4-Class Multiplayer Multi-Room Dungeon" --label "bug" --state open --json url --jq '.[].url' | while read url; do
    echo "Adding $url to project board" && gh project item-add 10 --owner KirkDiggler --url "$url"
  done
done
```

- [ ] **Step 8: Verify all issues filed, on milestone, and on board**

```bash
echo "=== rpg-api ===" && gh issue list --repo KirkDiggler/rpg-api --milestone "4-Class Multiplayer Multi-Room Dungeon" --label "bug" --state open
echo "=== rpg-dnd5e-web ===" && gh issue list --repo KirkDiggler/rpg-dnd5e-web --milestone "4-Class Multiplayer Multi-Room Dungeon" --label "bug" --state open
echo "=== project board ===" && gh project item-list 10 --owner KirkDiggler --limit 20
```

Expected: 1 issue in rpg-api, 5 issues in rpg-dnd5e-web, all on the milestone and visible on the board.

**Note:** This task files the 6 bugs found during today's Monk QA session (2026-03-28). The fighter, barbarian, and rogue checklists may contain additional unfiled bugs from prior sessions — those should be filed in a follow-up pass or by the Bug Fix Coordinator during its first triage.

- [ ] **Step 9: Commit any checklist updates**

If the QA checklists were updated with issue numbers, commit them:

```bash
cd /home/kirk/personal/rpg-project
git add docs/qa-checklists/
git commit -m "docs: link QA checklist bugs to GitHub issues"
```

---

### Task 8: Update Idea Status

**Files:**
- Modify: `rpg-project/ideas/bug-fix-team/CLAUDE.md`
- Modify: `rpg-project/ideas/bug-fix-team/memories.json`

- [ ] **Step 1: Update CLAUDE.md status**

Change the Status section from "Design complete" to:

```markdown
## Status
**Setup complete.** Milestone created, labels created, role prompts written, QA bugs filed. Ready for first session.
```

- [ ] **Step 2: Update memories.json**

Add a new task-progress entry for setup completion. Update the tp-design todo list to mark completed items.

- [ ] **Step 3: Commit**

```bash
git add ideas/bug-fix-team/
git commit -m "docs: mark bug fix team setup as complete"
```
