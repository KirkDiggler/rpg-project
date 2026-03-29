# Bug Fix Team Design

## Problem

QA playtesting reveals bugs across multiple layers (web, API, toolkit) that need to be fixed in parallel without stepping on each other or the running local dev environment. Currently, fixing bugs is serial — one conversation, one repo at a time. The platform audit team proved that multi-agent coordination works for read-only analysis, but we don't have a pattern for coordinated implementation work.

## Solution

A lead-plus-workers team structure where a Bug Fix Coordinator triages and reviews, layer-aligned workers implement fixes in worktrees, and GitHub milestone + issues serve as the shared coordination state. No local context files — the milestone IS the state.

## Team Structure

### Lead: Bug Fix Coordinator

Coordinates the bug fix session. Never writes code.

**On startup:**
- Read GH milestone issues (open, labeled `bug`)
- Read QA checklists at `rpg-project/docs/qa-checklists/`
- Check open PRs across repos

**Responsibilities:**
- Triage bugs by layer (web / api / toolkit)
- Check dependencies between bugs (does X need fixing before Y?)
- Assign issues to the right worker agent
- Review worker PRs against QA checklist expectations and boundary rules
- Flag approved PRs to Kirk for testing and merge
- File `bug` issues autonomously when discovered during review
- Flag `gap` items (needs design work, not just a fix) to Kirk with explanation

**Rules:**
- Never implements code
- Never merges PRs
- Cannot create `feature` or `gap` issues — only flags them
- May add a `gap` label to an existing bug issue to flag it for Kirk (this is labeling, not filing)

### Workers: Web Fixer, API Fixer, Toolkit Fixer

Layer-aligned implementation agents. Each owns one repo boundary.

| Role | Repos | When Active |
|------|-------|-------------|
| Web Fixer | rpg-dnd5e-web | When web bugs exist |
| API Fixer | rpg-api, rpg-api-protos | When API bugs exist. If a fix requires proto changes, create the proto PR first, then the API PR that depends on it. Note the dependency in both PR descriptions. |
| Toolkit Fixer | rpg-toolkit | When toolkit bugs exist (may not be needed every session) |

**On startup:**
- Read assigned issue from GH (the issue body contains the QA checklist reference)
- Read the referenced QA checklist section for expected behavior
- Read their repo's CLAUDE.md for conventions

**Responsibilities:**
- Implement the fix on a fresh branch from main
- Run `make pre-commit` (Go) or `npm run ci-check` (web) before pushing
- If CI fails: fix lint/test issues and retry. If the failure is unrelated to the fix (flaky test, pre-existing broken test), note it in the PR description and file a separate `bug` issue for the flaky test.
- Create PR linked to the issue, referencing QA checklist test
- Write notes as issue comments for anything discovered
- File new `bug` issues in other repos for cross-layer breakage found

**Rules:**
- One issue, one branch, one PR — never batch
- Work in git worktrees (use `superpowers:using-git-worktrees` skill for setup) to avoid stomping on Kirk's local branches
- Cannot file `feature` or `gap` issues — flag to lead
- Don't pick up a second bug until the first PR is merged or parked
- Never use `git commit --no-verify`

**When the bug is not a bug:**
- If the reported behavior is working as intended, or the QA checklist expectation is wrong, comment on the issue explaining why and flag to lead. Lead confirms and closes, or escalates to Kirk.

## Workflow

### 1. Startup — Lead reads the battlefield

```
Lead reads:
  - GH milestone issues (open, labeled `bug`)
  - QA checklists for current findings
  - Open PRs across repos (anything in flight?)
```

### 2. Triage — Lead assigns work

```
For each open bug issue:
  - Determine which layer owns it (web / api / toolkit)
  - Check dependencies (does bug X need to be fixed before bug Y?)
  - If blocked by another bug: add `blocked` label + comment explaining the dependency
  - Assign the GH issue to the worker and comment: "Assigned to [Web/API/Toolkit] Fixer"
```

Assignment means: GH issue assignment + a comment. The comment is the signal — workers check for issues assigned to their role name.

### 3. Workers execute

```
Worker:
  - Reads the assigned issue
  - Reads relevant QA checklist section for expected behavior
  - Creates branch from fresh main (in a worktree)
  - Implements fix
  - Runs pre-commit / ci-check
  - Pushes branch, creates PR linked to the issue
  - Writes issue comments for notes/discoveries
  - Files new bug issues for cross-layer problems found
```

### 4. Lead reviews each PR

```
Lead:
  - Reads the PR diff and the linked issue
  - Reviews against QA checklist: does the fix address the expected behavior?
  - Checks boundary rule: no game logic in API, no calculations in web
  - Checks for scope creep: did the worker change things unrelated to the bug?
  - Runs or verifies tests pass (check CI status on the PR)
  - If proto changes: verifies API and proto PRs are coordinated
  - Approves or sends back with specific comments
  - When approved, flags Kirk: "PR #X ready for testing"
```

### 5. Kirk tests and merges

```
Kirk:
  - Pulls the branch, runs the app, verifies against QA checklist
  - Merges or requests changes
  - Lead picks up the next bug
```

## Issue Lifecycle

### Filing bugs onto the milestone

After a QA session, bugs get filed as GH issues with:
- **Title:** clear description of the broken behavior
- **Body:** steps to reproduce, expected vs actual, QA checklist reference
- **Labels:** `bug` + layer label (`web`, `api`, `toolkit`)
- **Milestone:** 4-Class Multiplayer Multi-Room Dungeon (existing milestone)
- **Board:** added to project board #10

### Issue states through the fix cycle

```
Open (unassigned)
  → Lead assigns to a worker (GH assignment + comment)
  → Worker creates branch + PR (PR linked to issue)
  → Lead reviews PR
  → Lead flags Kirk: "ready for testing"
  → Kirk tests + merges → issue closes via PR

Blocked:
  → Lead adds `blocked` label + comment explaining dependency
  → When dependency is resolved, lead removes `blocked` label and assigns

Parked:
  → Kirk requests changes or deprioritizes
  → Lead adds `parked` label
  → Worker moves to next bug
  → Parked bugs rejoin the queue when Kirk unparks them

Not a bug:
  → Worker comments explaining why
  → Lead confirms and closes, or escalates to Kirk
```

### Discovery issues (found while fixing something else)

Worker files a new `bug` issue with:
- Title prefixed with `[discovered]` so it's easy to spot
- Body explains what they were fixing when they found it
- Labels: `bug` + layer
- Milestone: same milestone
- Unassigned — lead picks it up in next triage pass

### Gap items (needs design, not just a fix)

Worker or lead writes an issue comment on the original bug:

> "This isn't a simple fix — [explanation]. Flagging as a gap for Kirk to route."

Lead adds a `gap` label. Kirk decides whether it joins an existing workstream or becomes a new design effort in `ideas/`.

## Role Prompt Locations

```
rpg-project/docs/teams/roles/
  bug-fix-coordinator/
    prompt.md           -- Lead agent prompt
  web-fixer/
    prompt.md           -- Web worker prompt
  api-fixer/
    prompt.md           -- API worker prompt
  toolkit-fixer/
    prompt.md           -- Toolkit worker prompt
```

## Key Design Decisions

1. **GH milestone as shared state** — no local context files to manage. Issues and PR comments are the coordination layer. Durable, visible, no write contention.
2. **Lead reviews before Kirk sees PRs** — catches quality issues, boundary violations, and 5e rules bugs early. Proven pattern from the platform audit review cycles.
3. **Workers stay in their layer** — boundary rule enforced by role. Cross-layer problems become new issues for the other agent, not scope creep.
4. **Discovery issues keep the pipeline fed** — workers can file `bug` issues but not `feature` or `gap` issues. Prevents scope creep while capturing real problems found during work.
5. **Worktree isolation** — workers don't touch Kirk's running branches. He can test the app while agents work.

## Session Boundaries

Kirk kicks off a bug fix session by spawning the lead. The lead triages, dispatches workers, and reviews. The session ends when:
- All assigned bugs have PRs reviewed and flagged to Kirk, OR
- Kirk ends the session

Between sessions, the GH milestone is the state. A new session's lead reads the milestone and picks up where the last left off — no local files to sync.

## Milestone Scope

All bugs go on the current active milestone (currently "4-Class Multiplayer Multi-Room Dungeon"). If a bug is clearly unrelated to the milestone (e.g., a lobby regression from months ago), the lead flags it to Kirk for routing rather than filing it on the wrong milestone.

## What This Does NOT Cover

- Architectural refactoring (use the platform audit team for that)
- New feature design (use brainstorming → writing-plans flow)
- Multi-agent implementation of planned features (use subagent-driven-development)
- This team fixes what's broken. Not what's missing.
