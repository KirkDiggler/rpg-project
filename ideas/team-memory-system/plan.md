# Team Memory System — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Set up schema-driven knowledge files across all repos and role state for the PM, so agents accumulate domain knowledge that persists across sessions.

**Architecture:** Two new schema types (lesson-learned, pattern) added to rpg-project/schemas/. Each repo gets `.claude/knowledge/context/` with empty JSON arrays. CLAUDE.md files updated to reference knowledge directory. PM role gets context/archive directory structure for operational state. Initial knowledge entries seeded from platform audit findings.

**Tech Stack:** JSON, markdown, git

**Spec:** rpg-project/ideas/team-memory-system/design.md

---

## Task 1: Add schema type definitions

**Repo:** rpg-project
**Files:**
- Create: `schemas/lesson-learned.json`
- Create: `schemas/pattern.json`
- Modify: `schemas/README.md`

- [ ] **Step 1: Create lesson-learned.json schema**

  ```json
  {
    "type": "lesson-learned",
    "id": "",
    "created": "",
    "updated": "",
    "status": "active|superseded",
    "summary": "",
    "detail": "",
    "source": "",
    "domain": "",
    "references": []
  }
  ```

  Save to `/home/kirk/personal/rpg-project/schemas/lesson-learned.json`

- [ ] **Step 2: Create pattern.json schema**

  ```json
  {
    "type": "pattern",
    "id": "",
    "created": "",
    "updated": "",
    "status": "active|deprecated",
    "name": "",
    "description": "",
    "example": "",
    "anti_pattern": null,
    "domain": "",
    "references": []
  }
  ```

  Save to `/home/kirk/personal/rpg-project/schemas/pattern.json`

- [ ] **Step 3: Update schemas/README.md**

  Move `Pattern` from "Planned" to "Implemented" table:
  ```
  | Pattern | `pattern.json` | Reusable approaches, best practices, conventions |
  ```

  Add `Lesson Learned` to "Implemented" table:
  ```
  | Lesson Learned | `lesson-learned.json` | What went wrong or worked well across sessions |
  ```

  Remove `Pattern` from "Planned" table. Remove `Expertise` from planned (superseded by `lesson-learned` + `pattern`).

- [ ] **Step 4: Commit**

  ```bash
  cd /home/kirk/personal/rpg-project
  git add schemas/lesson-learned.json schemas/pattern.json schemas/README.md
  git commit -m "feat: add lesson-learned and pattern schema types for team memory system"
  ```

---

## Task 2: Create knowledge directories in rpg-toolkit

**Repo:** rpg-toolkit
**Files:**
- Create: `.claude/knowledge/context/patterns.json`
- Create: `.claude/knowledge/context/lessons-learned.json`
- Create: `.claude/knowledge/context/decisions.json`
- Create: `.claude/knowledge/archive/.gitkeep`
- Modify: `CLAUDE.md`

- [ ] **Step 1: Create directory structure and empty files**

  ```bash
  cd /home/kirk/personal/rpg-toolkit
  mkdir -p .claude/knowledge/context .claude/knowledge/archive
  echo '[]' > .claude/knowledge/context/patterns.json
  echo '[]' > .claude/knowledge/context/lessons-learned.json
  echo '[]' > .claude/knowledge/context/decisions.json
  touch .claude/knowledge/archive/.gitkeep
  ```

- [ ] **Step 2: Add knowledge section to CLAUDE.md**

  Find the end of the main conventions/patterns section in CLAUDE.md and add:

  ```markdown
  ## Agent-Discovered Knowledge

  Read `.claude/knowledge/context/` for patterns, lessons learned, and
  decisions discovered by previous work sessions. These complement the
  hand-curated conventions above.

  When you discover a new pattern or learn something non-obvious during
  your work, append it to the appropriate file in `.claude/knowledge/context/`.
  When a pattern is deprecated or a lesson superseded, move the entry
  from `context/` to `archive/`.

  See `rpg-project/schemas/` for type definitions (lesson-learned, pattern, decision).
  ```

- [ ] **Step 3: Commit**

  ```bash
  cd /home/kirk/personal/rpg-toolkit
  git add .claude/knowledge/ CLAUDE.md
  git commit -m "feat: add agent knowledge directory for team memory system"
  ```

---

## Task 3: Create knowledge directories in rpg-api

**Repo:** rpg-api
**Files:**
- Create: `.claude/knowledge/context/patterns.json`
- Create: `.claude/knowledge/context/lessons-learned.json`
- Create: `.claude/knowledge/context/decisions.json`
- Create: `.claude/knowledge/archive/.gitkeep`
- Modify: `CLAUDE.md`

- [ ] **Step 1: Create directory structure and empty files**

  ```bash
  cd /home/kirk/personal/rpg-api
  mkdir -p .claude/knowledge/context .claude/knowledge/archive
  echo '[]' > .claude/knowledge/context/patterns.json
  echo '[]' > .claude/knowledge/context/lessons-learned.json
  echo '[]' > .claude/knowledge/context/decisions.json
  touch .claude/knowledge/archive/.gitkeep
  ```

- [ ] **Step 2: Add knowledge section to CLAUDE.md**

  Same text as Task 2, Step 2.

- [ ] **Step 3: Commit**

  ```bash
  cd /home/kirk/personal/rpg-api
  git add .claude/knowledge/ CLAUDE.md
  git commit -m "feat: add agent knowledge directory for team memory system"
  ```

---

## Task 4: Create knowledge directories in rpg-dnd5e-web and rpg-api-protos

**Repos:** rpg-dnd5e-web, rpg-api-protos
**Files (each repo):**
- Create: `.claude/knowledge/context/patterns.json`
- Create: `.claude/knowledge/context/lessons-learned.json`
- Create: `.claude/knowledge/context/decisions.json`
- Create: `.claude/knowledge/archive/.gitkeep`
- Modify: `CLAUDE.md`

- [ ] **Step 1: Create structure in rpg-dnd5e-web**

  ```bash
  cd /home/kirk/personal/rpg-dnd5e-web
  mkdir -p .claude/knowledge/context .claude/knowledge/archive
  echo '[]' > .claude/knowledge/context/patterns.json
  echo '[]' > .claude/knowledge/context/lessons-learned.json
  echo '[]' > .claude/knowledge/context/decisions.json
  touch .claude/knowledge/archive/.gitkeep
  ```

- [ ] **Step 2: Add knowledge section to rpg-dnd5e-web CLAUDE.md**

  Same text as Task 2, Step 2.

- [ ] **Step 3: Commit rpg-dnd5e-web**

  ```bash
  cd /home/kirk/personal/rpg-dnd5e-web
  git add .claude/knowledge/ CLAUDE.md
  git commit -m "feat: add agent knowledge directory for team memory system"
  ```

- [ ] **Step 4: Create structure in rpg-api-protos**

  ```bash
  cd /home/kirk/personal/rpg-api-protos
  mkdir -p .claude/knowledge/context .claude/knowledge/archive
  echo '[]' > .claude/knowledge/context/patterns.json
  echo '[]' > .claude/knowledge/context/lessons-learned.json
  echo '[]' > .claude/knowledge/context/decisions.json
  touch .claude/knowledge/archive/.gitkeep
  ```

- [ ] **Step 5: Add knowledge section to rpg-api-protos CLAUDE.md**

  Same text as Task 2, Step 2.

- [ ] **Step 6: Commit rpg-api-protos**

  ```bash
  cd /home/kirk/personal/rpg-api-protos
  git add .claude/knowledge/ CLAUDE.md
  git commit -m "feat: add agent knowledge directory for team memory system"
  ```

---

## Task 5: Seed initial domain knowledge from platform audit

**Repos:** rpg-toolkit, rpg-api
**Files:**
- Modify: `rpg-toolkit/.claude/knowledge/context/patterns.json`
- Modify: `rpg-toolkit/.claude/knowledge/context/lessons-learned.json`
- Modify: `rpg-api/.claude/knowledge/context/patterns.json`
- Modify: `rpg-api/.claude/knowledge/context/lessons-learned.json`

- [ ] **Step 1: Seed rpg-toolkit patterns.json**

  Write to `/home/kirk/personal/rpg-toolkit/.claude/knowledge/context/patterns.json`:

  ```json
  [
    {
      "type": "pattern",
      "id": "error-collection-remove",
      "created": "2026-03-22",
      "updated": "2026-03-22",
      "status": "active",
      "name": "Error-collection Remove() for conditions",
      "description": "Condition Remove() methods must collect all unsubscribe errors instead of returning on the first failure. Early return leaks remaining subscriptions. Collect errors in a []error slice, nil state unconditionally, return aggregated error via errors.Join().",
      "example": "See conditions/raging.go Remove() — collects errors, nils subscriptionIDs and bus, returns fmt.Errorf with errors.Join.",
      "anti_pattern": "for _, id := range subscriptionIDs { if err := bus.Unsubscribe(id); err != nil { return err } } — leaks remaining subscriptions on first error",
      "domain": "toolkit",
      "references": ["https://github.com/KirkDiggler/rpg-toolkit/pull/603"]
    },
    {
      "type": "pattern",
      "id": "condition-apply-after-load",
      "created": "2026-03-22",
      "updated": "2026-03-22",
      "status": "active",
      "name": "Conditions must Apply() after LoadFromData",
      "description": "LoadFromData at data.go:220 calls Apply(ctx, bus) on each deserialized condition. This re-subscribes conditions to the event bus after round-trip serialization. Without Apply, conditions have their data but no live subscriptions.",
      "example": "See character/data.go LoadFromData — iterates conditions, calls Apply(ctx, bus) on each.",
      "anti_pattern": null,
      "domain": "toolkit",
      "references": ["https://github.com/KirkDiggler/rpg-toolkit/pull/602"]
    },
    {
      "type": "pattern",
      "id": "bus-capture-before-remove",
      "created": "2026-03-22",
      "updated": "2026-03-22",
      "status": "active",
      "name": "Capture bus reference before Remove() when publishing after",
      "description": "If a condition needs to publish events after removing itself (e.g., UnconsciousCondition nat-20 path), capture the bus reference before calling Remove(). Remove() nils c.bus, so publishing after Remove() would nil-pointer panic.",
      "example": "See conditions/unconscious.go onTurnStart — captures savedBus := c.bus before c.Remove(), then publishes on savedBus.",
      "anti_pattern": "c.Remove(ctx, bus); bus.Publish(ctx, event) — bus is nil after Remove, panic",
      "domain": "toolkit",
      "references": ["https://github.com/KirkDiggler/rpg-toolkit/pull/601"]
    }
  ]
  ```

- [ ] **Step 2: Seed rpg-toolkit lessons-learned.json**

  Write to `/home/kirk/personal/rpg-toolkit/.claude/knowledge/context/lessons-learned.json`:

  ```json
  [
    {
      "type": "lesson-learned",
      "id": "integration-tests-catch-mock-gaps",
      "created": "2026-03-22",
      "updated": "2026-03-22",
      "status": "active",
      "summary": "Integration tests catch what unit tests miss",
      "detail": "Unit tests with mocks passed but integration tests with real toolkit characters failed during TurnManager API refactor. Mocks don't exercise the full activation chain (ActivateAbility -> feature.Activate -> event bus -> condition.Apply). Always run integration tests before marking API refactors complete.",
      "source": "platform-audit-2026-03-22",
      "domain": "toolkit",
      "references": ["https://github.com/KirkDiggler/rpg-project/issues/4"]
    },
    {
      "type": "lesson-learned",
      "id": "spec-reviewer-check-git-diff",
      "created": "2026-03-22",
      "updated": "2026-03-22",
      "status": "active",
      "summary": "Spec reviewers must check git diff, not just file contents",
      "detail": "A spec reviewer saw all tests in a file and thought they were all newly added. Only the git diff shows what actually changed. Pre-existing code in the file was mistaken for new additions, causing a false spec failure.",
      "source": "platform-audit-2026-03-22",
      "domain": "cross-repo",
      "references": []
    }
  ]
  ```

- [ ] **Step 3: Seed rpg-api patterns.json**

  Write to `/home/kirk/personal/rpg-api/.claude/knowledge/context/patterns.json`:

  ```json
  [
    {
      "type": "pattern",
      "id": "feature-activation-direct-ref",
      "created": "2026-03-22",
      "updated": "2026-03-22",
      "status": "active",
      "name": "Feature activation uses direct refs, not proto conversion",
      "description": "ActivateFeature must call char.ActivateAbility() with the toolkit feature ref directly. Mapping through proto enums (feature -> CombatAbilityId -> back to ref) loses feature identity — PatientDefense becomes generic Dodge, wrong action cost and no ki spent.",
      "example": "See orchestrator.go ActivateFeature — uses featureIDToRef() then char.ActivateAbility(ref) directly.",
      "anti_pattern": "Mapping PatientDefense -> COMBAT_ABILITY_ID_DODGE -> protoAbilityIDToRef -> refs.CombatAbilities.Dodge() — loses the feature, activates wrong ability",
      "domain": "api",
      "references": ["https://github.com/KirkDiggler/rpg-api/pull/447"]
    },
    {
      "type": "pattern",
      "id": "sync-char-economy-to-encounter",
      "created": "2026-03-22",
      "updated": "2026-03-22",
      "status": "active",
      "name": "Sync character action economy back to encounter entity",
      "description": "After persisting character data via persistCharacterData(), call syncCharActionEconomyToEncounter() to bridge the character's toolkit action economy state back to the encounter entity's ActionEconomyState. This keeps GetEncounterState responses accurate.",
      "example": "See orchestrator.go syncCharActionEconomyToEncounter — reads char.GetActionEconomy(), maps to entities.ActionEconomyState, persists via encRepo.Update.",
      "anti_pattern": null,
      "domain": "api",
      "references": ["https://github.com/KirkDiggler/rpg-api/pull/447"]
    }
  ]
  ```

- [ ] **Step 4: Seed rpg-api lessons-learned.json**

  Write to `/home/kirk/personal/rpg-api/.claude/knowledge/context/lessons-learned.json`:

  ```json
  [
    {
      "type": "lesson-learned",
      "id": "boundary-violations-compound",
      "created": "2026-03-22",
      "updated": "2026-03-22",
      "status": "active",
      "summary": "Boundary violations compound — every switch on game enums requires API changes for new features",
      "detail": "The API had 75+ switch statements on feature/condition/ability refs. Every new class feature required API code changes, defeating the boundary architecture. The fix (PR #447) routes through char.ActivateAbility() directly so the toolkit handles all game logic.",
      "source": "platform-audit-2026-03-22",
      "domain": "api",
      "references": ["https://github.com/KirkDiggler/rpg-project/issues/4", "https://github.com/KirkDiggler/rpg-api/pull/447"]
    },
    {
      "type": "lesson-learned",
      "id": "proto-enum-loses-identity",
      "created": "2026-03-22",
      "updated": "2026-03-22",
      "status": "active",
      "summary": "Proto enum round-trip loses feature identity",
      "detail": "Mapping PatientDefense -> COMBAT_ABILITY_ID_DODGE -> protoAbilityIDToRef -> refs.CombatAbilities.Dodge() lost the feature identity. PatientDefense costs bonus action + ki, standard Dodge costs standard action. Features without dedicated proto enums must bypass proto conversion entirely.",
      "source": "platform-audit-2026-03-22",
      "domain": "api",
      "references": ["https://github.com/KirkDiggler/rpg-api/pull/447"]
    }
  ]
  ```

- [ ] **Step 5: Commit rpg-toolkit**

  ```bash
  cd /home/kirk/personal/rpg-toolkit
  git add .claude/knowledge/context/
  git commit -m "feat: seed initial domain knowledge from platform audit"
  ```

- [ ] **Step 6: Commit rpg-api**

  ```bash
  cd /home/kirk/personal/rpg-api
  git add .claude/knowledge/context/
  git commit -m "feat: seed initial domain knowledge from platform audit"
  ```

---

## Task 6: Set up PM role directory

**Repo:** rpg-project
**Files:**
- Create: `docs/teams/roles/project-manager/prompt.md`
- Create: `docs/teams/roles/project-manager/context/active-prs.json`
- Create: `docs/teams/roles/project-manager/context/active-blockers.json`
- Create: `docs/teams/roles/project-manager/context/active-work-items.json`
- Create: `docs/teams/roles/project-manager/archive/.gitkeep`

- [ ] **Step 1: Create directory structure**

  ```bash
  cd /home/kirk/personal/rpg-project
  mkdir -p docs/teams/roles/project-manager/context docs/teams/roles/project-manager/archive
  touch docs/teams/roles/project-manager/archive/.gitkeep
  ```

- [ ] **Step 2: Create PM prompt.md**

  Write to `docs/teams/roles/project-manager/prompt.md`:

  ```markdown
  # Project Manager

  You are the Project Manager for the RPG platform. You track work across
  all repos, maintain PR status, flag blockers, and ensure nothing falls
  through the cracks.

  ## On Startup

  1. Read everything in `context/` (this directory) for current operational state
  2. Read the project board: https://github.com/users/KirkDiggler/projects/10
  3. Check open PRs across repos:
     - `gh pr list` in rpg-toolkit, rpg-api, rpg-dnd5e-web, rpg-api-protos

  ## Your Responsibilities

  - Track PR lifecycle across all repos (open, reviewing, merged, closed)
  - Maintain active blockers and flag when they're resolved
  - Track remaining work items from implementation plans
  - Update context/ files as state changes
  - Move completed items from context/ to archive/ on shutdown
  - Flag unblocked work to the team lead

  ## What You Don't Do

  - You don't implement code
  - You don't make architectural decisions
  - You don't merge PRs (wait for explicit approval)
  - You don't create issues without team lead approval

  ## Repos

  | Repo | Path |
  |------|------|
  | rpg-project | /home/kirk/personal/rpg-project |
  | rpg-toolkit | /home/kirk/personal/rpg-toolkit |
  | rpg-api | /home/kirk/personal/rpg-api |
  | rpg-dnd5e-web | /home/kirk/personal/rpg-dnd5e-web |
  | rpg-api-protos | /home/kirk/personal/rpg-api-protos |

  ## State Files

  Use existing schema types from `rpg-project/schemas/`:
  - `active-prs.json` — entries with type `task-progress`
  - `active-blockers.json` — entries with type `blocker`
  - `active-work-items.json` — entries with type `task-progress`

  All files are bare JSON arrays `[{...}, {...}]`.
  ```

- [ ] **Step 3: Initialize empty state files**

  ```bash
  echo '[]' > docs/teams/roles/project-manager/context/active-prs.json
  echo '[]' > docs/teams/roles/project-manager/context/active-blockers.json
  echo '[]' > docs/teams/roles/project-manager/context/active-work-items.json
  ```

- [ ] **Step 4: Commit**

  ```bash
  cd /home/kirk/personal/rpg-project
  git add docs/teams/roles/
  git commit -m "feat: add project-manager role with context/archive state directories"
  ```

---

## Task 7: Seed PM state from current session

**Repo:** rpg-project
**Files:**
- Modify: `docs/teams/roles/project-manager/context/active-work-items.json`

- [ ] **Step 1: Populate active work items from platform audit plans**

  Write remaining tasks from all 3 plans to `active-work-items.json`. Use `task-progress` schema type. Include:

  - P4 Task 2: Simplify ExecuteAction (rpg-api) — Plan: `rpg-project/ideas/turnmanager-integration/plan.md`, Issue: https://github.com/KirkDiggler/rpg-project/issues/4
  - P4 Task 4: Clean up EndTurn reset (rpg-api) — same plan and issue
  - P4 Task 5: Integration verification + PR (rpg-api) — same plan and issue
  - P5 Task 4: Remove CharacterHP dual storage (rpg-api) — Plan: `rpg-project/ideas/condition-persistence/plan.md`, Issue: https://github.com/KirkDiggler/rpg-project/issues/5
  - P5 Task 5: Integration test save/load (rpg-toolkit) — same plan and issue
  - P6 Task 3: API orchestration for death saves + rest (rpg-api) — Plan: `rpg-project/ideas/death-saves/plan.md`, Issue: https://github.com/KirkDiggler/rpg-project/issues/6
  - P6 Task 4: Web UI for death saves + rest (rpg-dnd5e-web) — same plan and issue

  Each entry should reference the plan file path and GitHub issue.

- [ ] **Step 2: Commit**

  ```bash
  cd /home/kirk/personal/rpg-project
  git add docs/teams/roles/project-manager/context/active-work-items.json
  git commit -m "feat: seed PM state with remaining work from platform audit plans"
  ```

---

## Key Files Reference

| File | Repo | Role |
|------|------|------|
| `schemas/lesson-learned.json` | rpg-project | Type definition |
| `schemas/pattern.json` | rpg-project | Type definition |
| `schemas/README.md` | rpg-project | Schema index |
| `.claude/knowledge/context/*.json` | all repos | Domain knowledge (auto-loaded) |
| `.claude/knowledge/archive/` | all repos | Deprecated/superseded entries |
| `CLAUDE.md` | all repos | References knowledge directory |
| `docs/teams/roles/project-manager/` | rpg-project | PM role state |
