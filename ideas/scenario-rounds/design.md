# Scenario-Based Development Rounds — Design Spec

## Goal

Replace reactive whack-a-mole bug fixing with a structured, scenario-driven development cycle that produces the same quality as the design-then-plan workflow, while keeping Kirk's strategic layer (Claude) free from implementation context consumption.

## Problem

The current approach to getting features working is fragmented:
- A change lands, Kirk playtests, finds multiple issues, we go back and forth fixing them one at a time
- Each fix consumes context window, leaving less room for strategic thinking
- No persistent knowledge carries forward between sessions
- Bugs found in one layer often trace to another, causing mid-session pivots

The design -> plan -> implement cycle produces high-quality code every time, but it's been used for individual features. We need that same discipline applied to getting the whole game experience working.

## Solution: Scenario Rounds

Work is organized into **rounds**, each driven by a concrete player scenario. A round is a complete cycle from planning through implementation to playtest verification. Between rounds, persistent context files carry forward what the team learned.

### Core Concept

Instead of "fix these 12 bugs," the organizing unit is a player experience: "Monk fights in room 1, kills monster, removed from board." The scenario naturally touches every layer (toolkit damage calc, API encounter state, web combat UI) and forces integration quality. Bugs reveal themselves as failures against concrete acceptance criteria, not as an open-ended list.

## Roles

### Strategic Layer (Claude, in conversation with Kirk)

The persistent voice across rounds. Plays the PM role.

**Responsibilities:**
- Co-plan rounds with Kirk based on scenarios
- Assess state by reading team context files and GH Projects
- Break scenarios into acceptance criteria and layer-specific tasks
- Create GH issues, assign to project board
- Write round plans for the Round Lead
- Review round outcomes with Kirk
- Maintain the big picture across rounds

**Rules:**
- Never implements code directly
- Context stays available for strategic conversation
- Reads all team context files at round start to understand accumulated knowledge

### Round Lead (Bug Fix Coordinator role, evolved)

Supervises workers during a round's execution phase. Dispatched by the strategic layer.

**Responsibilities:**
- Receive the round plan with acceptance criteria and assigned tasks
- Spawn and coordinate layer workers
- Route cross-layer discoveries (non-blocking: file issue, keep going; blocking: re-route to correct worker)
- Review worker PRs against acceptance criteria and boundary rules
- Post round summary: what landed, what's blocked, what was discovered
- Trigger Platform Simplifier for post-round review

**Rules:**
- Never implements code
- Never merges PRs (Kirk merges after playtest)
- Can file `bug` issues for discoveries, flags `gap` items to Kirk
- Escalates to Kirk (through round summary) when architectural questions arise

### Platform Simplifier (architectural conscience)

Reviews designs, plans, and round outcomes for boundary violations and architectural drift.

**When active:**
- **Design/plan reviews:** When a new feature or fix is being designed, reviews for boundary rule compliance, unnecessary complexity, and toolkit-vs-API responsibility
- **Post-round review:** After workers complete a round, reads updated context files and looks for cross-cutting patterns, recurring boundary violations, or tech debt being introduced
- **On-demand:** When the strategic layer or Kirk wants an architectural assessment before starting a new scenario area

**Responsibilities:**
- Review designs and plans against the boundary rule (client=references, API=orchestration, toolkit=rules)
- Identify when work is adding complexity that should be solved at the toolkit tools level
- Update shared `patterns.json` and `lessons-learned.json` with findings
- Synthesize cross-layer observations into actionable briefs
- Maintain `previous-briefs.json` to avoid repeating findings

**Rules:**
- Advisory only — flags issues, doesn't implement
- Reports surface friction and architectural drift, not code style
- Findings go into context files, not just conversation

### Layer Workers (Web Fixer, API Fixer, Toolkit Fixer)

Implementation agents, one per layer. Same as bug-fix-team design with one addition: workers use design -> plan -> implement cycle even for bug fixes.

**On spawn:**
- Read role prompt at `docs/teams/roles/<role>/prompt.md`
- Read role context directory at `docs/teams/roles/<role>/context/`
- Read repo's `.claude/knowledge/context/` for domain knowledge
- Read assigned GH issues for the round

**Responsibilities:**
- Implement fixes/features using design -> plan -> implement cycle
- Work in git worktrees for isolation
- Run ci-check / pre-commit before pushing
- File `[discovered]` bug issues for cross-layer problems
- Flag blocking issues to Round Lead
- Update context files with discoveries, patterns, lessons learned

**Rules:**
- One issue, one branch, one PR
- Stay in their layer — cross-layer problems become new issues
- Never use `git commit --no-verify`
- Never merge PRs

## Context Directory Structure

Each team role has persistent state at `rpg-project/docs/teams/roles/<role>/context/`. Files are loaded on spawn and updated during work.

### Per-Role Context Files

| File | Purpose | Who Writes | Lifespan |
|------|---------|------------|----------|
| `active-work.json` | Current round's assigned tasks + status | Round Lead | Per-round, archived on completion |
| `discoveries.json` | Issues found outside this role's scope | Worker | Accumulates, archived when filed as GH issues |
| `patterns.json` | Recurring problems, workarounds, known quirks | Worker + Platform Simplifier | Long-lived |
| `lessons-learned.json` | "This approach works/doesn't work for X" | Worker + Platform Simplifier | Long-lived |
| `dependencies.json` | What this layer needs from other layers | Worker | Updated each round |
| `previous-briefs.json` | Platform Simplifier's past findings (prevents repeats) | Platform Simplifier | Long-lived |

### Context File Schemas

**`active-work.json`** — Array of task entries assigned for this round:
```json
[{
  "id": "string (GH issue number, e.g., 'web#366')",
  "summary": "string (one-line description)",
  "status": "assigned | in-progress | pr-open | blocked",
  "pr": "string | null (PR URL when created)",
  "blocker": "string | null (description if blocked)",
  "round": "string (round tracking issue number)"
}]
```

**`discoveries.json`** — Array of cross-layer issues found during work:
```json
[{
  "id": "string | null (GH issue number once filed, null if pending)",
  "summary": "string (what was found)",
  "found_while": "string (what task was being worked on)",
  "layer": "web | api | toolkit (which layer owns the fix)",
  "blocking": "boolean (does this block current work?)",
  "round": "string (round tracking issue number)"
}]
```

**`dependencies.json`** — Array of cross-layer needs:
```json
[{
  "needs": "string (what this layer needs, e.g., 'encounter state event must include entity HP')",
  "from": "web | api | toolkit (which layer provides it)",
  "status": "needed | available | blocked",
  "related_issue": "string | null (GH issue if one exists)"
}]
```

**`previous-briefs.json`** — Platform Simplifier's past findings:
```json
[{
  "round": "string (round tracking issue number)",
  "date": "ISO date string",
  "findings": ["string (one-line summary of each finding)"],
  "actioned": ["string (findings that led to changes)"]
}]
```

`patterns.json` and `lessons-learned.json` use the schema types defined in the team-memory-system design (`pattern` and `lesson-learned` types).

### Per-Repo Domain Knowledge

Lives at `<repo>/.claude/knowledge/context/` per the team-memory-system design. Contains patterns, lessons, and decisions discovered about that codebase. Auto-loaded via CLAUDE.md on every session.

### What Does NOT Live in Context Files

- Implementation plans (per-feature artifacts in `ideas/`)
- Bug details (GH issues)
- Code patterns or architecture (code, ADRs, CLAUDE.md)
- Ephemeral task state (GH Projects board)

### Archive Rule

When a round completes, the Round Lead moves resolved items from `discoveries.json` and `active-work.json` into `archive/`. Long-lived files (`patterns.json`, `lessons-learned.json`) persist and grow — they are the team's institutional memory.

## GH Projects Integration

Project board #10 remains the source of truth for work tracking.

### Issue Flow Through a Round

```
Backlog -> Round Planned -> In Progress -> In Review -> Playtest -> Done
```

### Labels

| Label | Purpose |
|-------|---------|
| `scenario:<name>` | Groups issues to a round's scenario (e.g., `scenario:monk-room1`) |
| `layer:web` / `layer:api` / `layer:toolkit` | Which worker owns it |
| `blocking` | Worker is stopped, needs cross-layer help |
| `discovered` | Found during a round, not originally planned |
| `gap` | Needs design work, not just a fix |

### Round Tracking Issue

Each round gets a tracking issue titled like "Round 1: Monk kills monster in room 1" with acceptance criteria as a checklist. Individual task issues link back to it. One place to see round progress.

## Round Lifecycle

### Phase 1: Setup (Kirk + Strategic Layer)

1. Kirk describes the scenario ("Monk fights in room 1, kills monster, removed from board")
2. Strategic layer reads all team context files for accumulated knowledge
3. Strategic layer checks GH Projects for open/blocked issues related to the scenario
4. Together, break scenario into acceptance criteria — concrete, testable statements:
   - "Monk can select Flurry of Blows, costs 1 ki point"
   - "Monster HP reaches 0, entity removed from hex grid"
   - "Combat log shows each attack roll and damage"
5. Strategic layer creates GH issues for new work, tags with scenario label, assigns to board
6. Strategic layer writes round plan: which workers, what order, what tasks

### Phase 2: Execution (Round Lead + Workers)

7. Round Lead spawns with its context directory. The round plan is passed as part of the spawn prompt — it includes the round tracking issue number, acceptance criteria, worker assignments, and task issue numbers. The Round Lead reads the full GH issues for details.
8. Workers spawn in parallel (independent tasks) or sequence (dependencies), each in a worktree
9. Each worker loads: role prompt + context directory + assigned GH issues
10. Workers execute using design -> plan -> implement cycle (even for bug fixes)
11. Non-blocking discoveries -> file GH issue with `[discovered]` prefix, keep going
12. Blocking discoveries -> flag to Round Lead, who re-routes or escalates
13. Workers file PRs, run ci-check, update their context files with what they learned

### Phase 3: Review (Round Lead + Platform Simplifier)

14. Round Lead reviews PRs against acceptance criteria and boundary rules
15. Platform Simplifier reads updated context files, checks for cross-cutting patterns and architectural drift
16. Platform Simplifier updates shared patterns and lessons-learned files
17. Round Lead posts round summary: what landed, what's blocked, what was discovered

### Phase 4: Regroup (Kirk + Strategic Layer)

18. Kirk pulls changes, playtests against the acceptance criteria
19. Kirk reports what works and what doesn't
20. Strategic layer reads updated context files + Kirk's findings
21. Together, plan the next round (or continue current scenario if acceptance criteria not met)

## Design Quality Principle

Workers use the full design -> plan -> implement cycle even for bug fixes within a round. A "fix HP display" task still gets a short plan with clear scope before writing code. This is what produces the quality difference between whack-a-mole and structured work.

For small fixes, the "plan" may be 3-5 lines. For larger tasks within a round, it may be a full plan document. The discipline of thinking before coding is the point, not the document length.

## First Round Example

**Scenario:** "Monk fights in room 1, kills monster, removed from board"

**Acceptance criteria:**
- Start encounter with a Monk character and at least one monster
- Monk can use unarmed strike (displays correctly, not "unknown weapon")
- Monk can use Flurry of Blows (costs ki, shows in combat log)
- Monster HP bar updates on each hit
- Monk's HP and AC display correctly
- When monster HP reaches 0, entity is removed from hex grid
- Combat log shows each attack roll, damage, and ki usage

**Likely work breakdown:**
- **Web:** Combat panel reads unified entity state, unarmed strike display fix, HP bar updates, entity removal on death, ki resource display
- **Toolkit:** Verify monk combat chain (unarmed strike damage, Flurry of Blows, ki spending)
- **API:** Verify encounter state events carry correct data for all of the above

**Workers:** Web Fixer (4-5 issues), Toolkit Fixer (verification, fixes if needed), API Fixer (verify event payloads)

## Evolution Toward Autonomy

This system is designed to mature. As context files get richer and team roles get sharper from real rounds:

1. **Early rounds:** Full co-planning with Kirk, detailed acceptance criteria, close review
2. **Mid-term:** Strategic layer can pre-plan rounds based on context files, Kirk reviews and adjusts
3. **Long-term:** Strategic layer runs multiple rounds autonomously against a high-level goal ("make 4-class dungeon playable"), Kirk playtests and provides direction

Autonomy is emergent, not designed. The context directory system and GH Projects tracking make it possible — rich accumulated knowledge means each round starts with better information and needs less human guidance.

## Relationship to Existing Designs

### Team Memory System (`ideas/team-memory-system/`)
Provides the context directory structure, schema types, and domain knowledge pattern. Scenario rounds USE this system — workers read and write context files as designed.

### Bug Fix Team (`ideas/bug-fix-team/`)
Provides the worker roles, worktree isolation, and issue lifecycle. Scenario rounds EVOLVE this — same roles, same isolation, but organized around scenarios instead of bug lists, and with the design-then-plan discipline added.

### Platform Simplifier
Gains a new trigger: design/plan review in addition to post-round synthesis. Becomes the architectural conscience that prevents the tech debt that comes from solving things at the wrong layer.

## Out of Scope

- Automated test runners (workers run tests manually via ci-check)
- Automated deployment (Kirk merges and deploys)
- Cross-scenario planning (handled in Phase 4 regroup conversations)
- Tooling or scripts for managing context files (files are the interface, git history is the audit trail)
