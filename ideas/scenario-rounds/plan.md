# Scenario-Based Development Rounds — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Set up the infrastructure for scenario-based development rounds — context directories, updated role prompts, GH labels, and a round template — so the first round can be kicked off immediately after.

**Architecture:** File-based context system in rpg-project/docs/teams/roles/ with JSON context files per role, updated prompts that instruct agents to load context on spawn, and GH labels across 4 repos for scenario tracking.

**Tech Stack:** JSON files, Markdown prompts, GitHub CLI (gh), shell commands

---

### Task 1: Create Context Directories for Worker Roles

The 3 worker roles (web-fixer, api-fixer, toolkit-fixer) and bug-fix-coordinator need context/ and archive/ directories with initialized JSON files per the design spec.

**Files:**
- Create: `docs/teams/roles/web-fixer/context/active-work.json`
- Create: `docs/teams/roles/web-fixer/context/discoveries.json`
- Create: `docs/teams/roles/web-fixer/context/patterns.json`
- Create: `docs/teams/roles/web-fixer/context/lessons-learned.json`
- Create: `docs/teams/roles/web-fixer/context/dependencies.json`
- Create: `docs/teams/roles/web-fixer/archive/.gitkeep`
- Create: same structure for `api-fixer/`, `toolkit-fixer/`, `bug-fix-coordinator/`

- [ ] **Step 1: Create context directories and empty JSON arrays for all 4 roles**

For each of `web-fixer`, `api-fixer`, `toolkit-fixer`, `bug-fix-coordinator`:

```bash
cd /home/kirk/personal/rpg-project
for role in web-fixer api-fixer toolkit-fixer bug-fix-coordinator; do
  mkdir -p docs/teams/roles/$role/context
  mkdir -p docs/teams/roles/$role/archive
  for file in active-work.json discoveries.json patterns.json lessons-learned.json dependencies.json; do
    echo '[]' > docs/teams/roles/$role/context/$file
  done
  touch docs/teams/roles/$role/archive/.gitkeep
done
```

- [ ] **Step 2: Verify directory structure**

```bash
find docs/teams/roles/*/context -type f | sort
find docs/teams/roles/*/archive -type f | sort
```

Expected: 5 JSON files per role in context/, .gitkeep in each archive/

- [ ] **Step 3: Commit**

```bash
git add docs/teams/roles/*/context/ docs/teams/roles/*/archive/
git commit -m "feat: add context directories for scenario round roles"
```

---

### Task 2: Create Platform Simplifier Context Files

Platform Simplifier already has `context/` with `active-observations.json` and `previous-briefs.json`. Add the missing files from the design spec. Note: the Simplifier is advisory, not a worker — it does not get `active-work.json`, `discoveries.json`, or `dependencies.json` (those are worker-only files).

**Files:**
- Create: `docs/teams/roles/platform-simplifier/context/patterns.json`
- Create: `docs/teams/roles/platform-simplifier/context/lessons-learned.json`

- [ ] **Step 1: Add missing context files**

```bash
cd /home/kirk/personal/rpg-project
echo '[]' > docs/teams/roles/platform-simplifier/context/patterns.json
echo '[]' > docs/teams/roles/platform-simplifier/context/lessons-learned.json
```

- [ ] **Step 2: Commit**

```bash
git add docs/teams/roles/platform-simplifier/context/
git commit -m "feat: add patterns and lessons-learned to platform simplifier context"
```

---

### Task 3: Update Round Lead (Bug Fix Coordinator) Prompt

Evolve the bug-fix-coordinator prompt from milestone-based bug triage to scenario-round supervision. The prompt needs to handle receiving a round plan, spawning workers, routing discoveries, and posting round summaries.

**Files:**
- Modify: `docs/teams/roles/bug-fix-coordinator/prompt.md`

- [ ] **Step 1: Read current prompt**

Read `docs/teams/roles/bug-fix-coordinator/prompt.md` to understand existing structure.

- [ ] **Step 2: Rewrite prompt for scenario rounds**

Replace the content with the evolved Round Lead prompt:

```markdown
# Round Lead (Bug Fix Coordinator)

You are the Round Lead for the RPG platform. You supervise workers during a scenario round's execution phase, review PRs, and route cross-layer discoveries.

## On Startup

1. Read your context directory at `docs/teams/roles/bug-fix-coordinator/context/` — load all files
2. Read the round plan provided in your spawn prompt. It contains:
   - Round tracking issue number
   - Acceptance criteria
   - Worker assignments (which worker gets which GH issues)
   - Task issue numbers
3. Read the full GH issues for each assigned task:
   ```
   gh issue view <number> --repo KirkDiggler/<repo>
   ```
4. Read ALL QA checklist files in `/home/kirk/personal/rpg-project/docs/qa-checklists/` — discover files dynamically
5. Check for open PRs across repos:
   ```
   for repo in rpg-api rpg-dnd5e-web rpg-toolkit rpg-api-protos; do
     echo "=== KirkDiggler/$repo ===" && gh pr list --repo KirkDiggler/$repo --state open
   done
   ```

## Round Execution

### 1. Dispatch Workers

Spawn workers based on the round plan. Each worker gets:
- Their assigned GH issue numbers
- The round tracking issue number
- The acceptance criteria relevant to their tasks

Spawn independent workers in parallel. If tasks have dependencies (e.g., API must land before web can use it), spawn sequentially.

Workers work in git worktrees — they won't interfere with each other or Kirk's branches.

### 2. Monitor and Route

While workers execute:
- **Non-blocking discoveries:** Worker files a `[discovered]` GH issue, tags the layer, keeps going. You verify the issue is filed correctly.
- **Blocking discoveries:** Worker flags to you. Determine which layer owns the fix:
  - If another worker is active for that layer: route the issue to them
  - If no worker for that layer: escalate in the round summary for Kirk
- Update `active-work.json` in the worker's context directory as status changes

### 3. Review PRs

For each worker PR:
- Read the PR diff and the linked issue
- Review against acceptance criteria: does the fix address what the scenario requires?
- Check boundary rule: no game logic in API, no calculations in web
- Check for scope creep: did the worker change things unrelated to their task?
- Verify CI passes (check PR status checks)
- If proto changes: verify API and proto PRs are coordinated
- Approve or send back with specific comments
- When approved, comment: "Ready for testing @KirkDiggler"

### 4. Post-Round Summary

After all workers complete (or are blocked), post a round summary as a comment on the round tracking issue:

```markdown
## Round Summary

### Completed
- [ PR link ] — what it does

### Blocked
- [ issue link ] — why it's blocked

### Discovered
- [ issue link ] — found during work, filed for next round

### Context Updates
- Key patterns or lessons workers added to context files

### Recommendation
What should the next round focus on, based on what was learned
```

### 5. Trigger Platform Simplifier

After posting the summary, note that the Platform Simplifier should review:
- Updated context files from all workers
- PRs for architectural drift or boundary violations
- Cross-cutting patterns worth capturing

## Issue Management

- File `bug` issues autonomously when you discover something broken during review
- Prefix discovery issues with `[discovered]`
- May add a `gap` label to flag items needing design work
- Cannot create `feature` or `gap` issues — flag them to Kirk with explanation

## What You Don't Do

- Don't implement code
- Don't merge PRs (Kirk merges after playtest)
- Don't make architectural decisions
- Don't create feature or gap issues

## The Boundary Rule

```
Client sends REFERENCES     -> never calculations
API orchestrates by KEY      -> never knows what "rage" does
Toolkit implements RULES     -> returns rich breakdowns
```

If a worker's PR violates this, send it back.

## Repos

| Repo | Path | Worker |
|------|------|--------|
| rpg-api | /home/kirk/personal/rpg-api | API Fixer |
| rpg-dnd5e-web | /home/kirk/personal/rpg-dnd5e-web | Web Fixer |
| rpg-toolkit | /home/kirk/personal/rpg-toolkit | Toolkit Fixer |
| rpg-api-protos | /home/kirk/personal/rpg-api-protos | API Fixer |
```

- [ ] **Step 3: Commit**

```bash
git add docs/teams/roles/bug-fix-coordinator/prompt.md
git commit -m "feat: evolve bug-fix-coordinator prompt to round lead for scenario rounds"
```

---

### Task 4: Update Worker Prompts to Load Context

Add context directory loading instructions to each worker's prompt. Workers need to read their context files on startup and write back discoveries and lessons during work.

**Files:**
- Modify: `docs/teams/roles/web-fixer/prompt.md`
- Modify: `docs/teams/roles/api-fixer/prompt.md`
- Modify: `docs/teams/roles/toolkit-fixer/prompt.md`

- [ ] **Step 1: Read current worker prompts**

Read all three worker prompt files.

- [ ] **Step 2: Add context loading to On Startup section of each worker**

Add after the existing startup steps in each prompt:

```markdown
## Context Loading

Before starting work, read your persistent context:

1. Read all files in your context directory: `docs/teams/roles/<role>/context/`
   - `patterns.json` — known patterns and quirks for your layer
   - `lessons-learned.json` — what worked and didn't in previous rounds
   - `dependencies.json` — cross-layer needs
   - `active-work.json` — your assigned tasks for this round
   - `discoveries.json` — pending cross-layer issues
2. Read your repo's domain knowledge: `<repo>/.claude/knowledge/context/` (if it exists)

## Context Writing

During and after your work:

- **Discoveries:** When you find a bug outside your layer, add an entry to `discoveries.json` AND file a `[discovered]` GH issue. The JSON entry links to the issue once filed.
- **Patterns:** When you learn something reusable about your codebase (a quirk, a workaround, a reliable approach), add it to `patterns.json` using the `pattern` schema type.
- **Lessons:** When something goes wrong or succeeds non-obviously, add it to `lessons-learned.json` using the `lesson-learned` schema type.
- **Dependencies:** When your work needs something from another layer, add it to `dependencies.json`.
```

Replace `<role>` and `<repo>` with the specific values for each worker:
- web-fixer: role=web-fixer, repo=rpg-dnd5e-web
- api-fixer: role=api-fixer, repo=rpg-api
- toolkit-fixer: role=toolkit-fixer, repo=rpg-toolkit

- [ ] **Step 3: Commit**

```bash
git add docs/teams/roles/web-fixer/prompt.md docs/teams/roles/api-fixer/prompt.md docs/teams/roles/toolkit-fixer/prompt.md
git commit -m "feat: add context loading and writing instructions to worker prompts"
```

---

### Task 5: Update Platform Simplifier Prompt for Design/Plan Reviews

Add the design/plan review trigger to the Platform Simplifier's prompt. Currently it only does post-audit synthesis. The scenario rounds design adds review of designs and plans for boundary violations.

**Files:**
- Modify: `docs/teams/roles/platform-simplifier/prompt.md`

- [ ] **Step 1: Read current prompt**

Read `docs/teams/roles/platform-simplifier/prompt.md`.

- [ ] **Step 2: Add design/plan review section**

Add a new section after the existing responsibilities:

```markdown
## Design and Plan Reviews

When dispatched to review a design or plan document:

1. Read the document at the provided path
2. Read the project architecture docs:
   - `rpg-project/docs/architecture.md`
   - `rpg-project/docs/boundaries.md`
3. Check each proposed change against:
   - **Boundary rule:** Is game logic staying in the toolkit? Is the API only orchestrating? Is the client only rendering?
   - **Toolkit-first:** Could this be solved as a toolkit tool instead of hardcoded in the API or web?
   - **Unnecessary complexity:** Are conversion layers, dual representations, or speculative abstractions being introduced?
   - **Modular design:** Will this work for all classes, or is it hardcoded for one?
4. Report findings as:
   - **BOUNDARY VIOLATION:** Game logic in the wrong layer
   - **COMPLEXITY:** Unnecessary abstraction or conversion
   - **TOOLKIT OPPORTUNITY:** Could be a reusable tool instead of per-layer implementation
   - **OK:** No issues found

Write findings to `context/previous-briefs.json` to avoid repeating them in future reviews.
```

- [ ] **Step 3: Commit**

```bash
git add docs/teams/roles/platform-simplifier/prompt.md
git commit -m "feat: add design/plan review capability to platform simplifier"
```

---

### Task 6: Create GH Labels Across Repos

Create the scenario-round labels across all 4 repos. These are: `layer:web`, `layer:api`, `layer:toolkit`, `blocking`, `discovered`, `gap`.

Note: `scenario:<name>` labels are created per-round, not upfront.

**Files:** None (GH API only)

- [ ] **Step 1: Create labels in all repos**

```bash
for repo in rpg-api rpg-dnd5e-web rpg-toolkit rpg-api-protos; do
  echo "=== Creating labels in KirkDiggler/$repo ==="
  gh label create "layer:web" --repo KirkDiggler/$repo --color "1d76db" --description "Owned by web fixer" --force
  gh label create "layer:api" --repo KirkDiggler/$repo --color "0e8a16" --description "Owned by API fixer" --force
  gh label create "layer:toolkit" --repo KirkDiggler/$repo --color "d93f0b" --description "Owned by toolkit fixer" --force
  gh label create "blocking" --repo KirkDiggler/$repo --color "b60205" --description "Worker blocked, needs cross-layer help" --force
  gh label create "discovered" --repo KirkDiggler/$repo --color "fbca04" --description "Found during a round, not originally planned" --force
  gh label create "gap" --repo KirkDiggler/$repo --color "d4c5f9" --description "Needs design work, not just a fix" --force
done
```

- [ ] **Step 2: Verify labels exist**

```bash
for repo in rpg-api rpg-dnd5e-web rpg-toolkit rpg-api-protos; do
  echo "=== KirkDiggler/$repo ===" && gh label list --repo KirkDiggler/$repo | grep -E "layer:|blocking|discovered|gap"
done
```

Expected: 6 labels in each repo.

---

### Task 7: Create Round Template

Create a round tracking issue template that the strategic layer uses when kicking off each round. This lives in rpg-project as a reference, not as a GH issue template.

**Files:**
- Create: `docs/teams/round-template.md`

- [ ] **Step 1: Write the round template**

```markdown
# Round Template

Use this template when creating the tracking issue for a new round.

## Issue Title

`Round N: <scenario description>`

## Issue Body

```markdown
## Scenario
<One sentence describing the player experience being verified>

## Acceptance Criteria
- [ ] <Concrete, testable statement 1>
- [ ] <Concrete, testable statement 2>
- [ ] ...

## Worker Assignments

### Web Fixer
- [ ] #<issue> — <summary>

### API Fixer
- [ ] #<issue> — <summary>

### Toolkit Fixer
- [ ] #<issue> — <summary>

## Labels
Apply `scenario:<name>` label to this issue and all task issues.

## Notes
<Any context from previous rounds, known dependencies, or things to watch for>
```

## After Creating

1. Apply the `scenario:<name>` label to the tracking issue
2. Apply `scenario:<name>` + `layer:<x>` labels to each task issue
3. Add all issues to project board #10
4. Pass the tracking issue number, acceptance criteria, and assignments to the Round Lead spawn prompt
```

- [ ] **Step 2: Commit**

```bash
git add docs/teams/round-template.md
git commit -m "feat: add round tracking issue template for scenario rounds"
```

---

### Task 8: Remove Project Manager Role (Folded into Strategic Layer)

Per the design, the PM role is folded into the strategic layer (Claude in conversation with Kirk). Remove the project-manager role directory or mark it as deprecated. The context files (active-prs.json, active-work-items.json, active-blockers.json) move to being maintained by the strategic layer directly through GH Projects, not a separate role.

**Files:**
- Modify: `docs/teams/roles/project-manager/prompt.md`

- [ ] **Step 1: Read current PM prompt and context**

Read `docs/teams/roles/project-manager/prompt.md` and list context files to understand what's there.

- [ ] **Step 2: Archive the PM role**

Add a deprecation notice to the top of the PM prompt:

```markdown
> **DEPRECATED:** This role has been folded into the Strategic Layer (Claude in conversation with Kirk) as part of the scenario-based development rounds system. See `ideas/scenario-rounds/design.md`. The PM's tracking responsibilities are now handled through GH Projects board #10 and the round tracking issues.
```

Do NOT delete the directory — the context files may have useful historical data.

- [ ] **Step 3: Commit**

```bash
git add docs/teams/roles/project-manager/prompt.md
git commit -m "feat: deprecate project-manager role, folded into strategic layer"
```

---

### Task 9: Final Verification

- [ ] **Step 1: Verify all context directories exist with correct files**

```bash
cd /home/kirk/personal/rpg-project
echo "=== Context files ==="
find docs/teams/roles/*/context -type f | sort
echo ""
echo "=== Archive dirs ==="
find docs/teams/roles/*/archive -type f | sort
```

Expected: 5-6 JSON files per worker role, platform simplifier has its existing files plus new ones, all archive/ dirs have .gitkeep.

- [ ] **Step 2: Verify all prompts are updated**

Read each prompt.md and confirm:
- bug-fix-coordinator: Has round plan loading, worker dispatch, round summary sections
- web-fixer/api-fixer/toolkit-fixer: Has Context Loading and Context Writing sections
- platform-simplifier: Has Design and Plan Reviews section
- project-manager: Has deprecation notice

- [ ] **Step 3: Verify GH labels**

```bash
for repo in rpg-api rpg-dnd5e-web rpg-toolkit rpg-api-protos; do
  echo "=== KirkDiggler/$repo ===" && gh label list --repo KirkDiggler/$repo | grep -E "layer:|blocking|discovered|gap"
done
```

- [ ] **Step 4: Verify round template exists**

```bash
cat docs/teams/round-template.md
```
