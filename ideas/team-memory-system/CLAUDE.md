# Idea: Team Memory System

## What This Is
A file-system-based memory system for agent team members that provides persistent, structured context across sessions. Team members are ephemeral (fresh each spawn) but their roles aren't — this system gives each role continuity.

## Origin
Platform audit session 2026-03-22. Running 4-specialist audit + implementation team revealed the need for:
- Lessons learned that persist across sessions
- Active work tracking (PRs, blockers) that auto-loads on spawn
- Archive of resolved items that doesn't bloat context

## Key Design Principles

### Physical Separation Over Filtering
Don't load all entries and filter — filtered entries still consume context tokens. Instead, use directory structure to enforce what gets loaded:

```
docs/teams/<role>/
  prompt.md              <- Spawn instructions
  context/               <- Auto-loaded into prompt on spawn
    active-prs.json      <- Current PR status
    active-blockers.json <- Current blockers
    lessons-learned.json <- Always relevant
    patterns.json        <- Reusable patterns
  archive/               <- Only read on demand
    closed-prs.json
    resolved-blockers.json
    completed-work.json
  memories.json          <- Index of what's where
```

### Agent Lifecycle
1. Spawn with prompt.md
2. Prompt says: "Read everything in context/"
3. Agent has instant context — no filtering, no wasted tokens
4. Agent works, updates context/ files
5. On shutdown: move completed items from context/ to archive/

### Schema-Driven Types
Each entry has a type, and types define what fields exist. Similar to rpg-project's existing memories.json pattern.

New schema types (added to rpg-project/schemas/):
- `lesson-learned` — What went wrong or worked well. Domain knowledge, always in context/.
- `pattern` — Reusable patterns discovered. Domain knowledge, always in context/.

Existing schema types reused:
- `decision` — Architectural choices. Domain knowledge (decisions.json) or role state.
- `blocker` — Current blockers. Role state (context/ while active, archive/ when resolved).
- `task-progress` — Tracked work. Role state (PM tracks PRs and work items).
- `known-issue` — Known issues. Domain knowledge.

### Lessons From This Session (2026-03-22)
These should be the first entries in context/lessons-learned.json:

1. **Spec reviewers can check wrong worktree** — Worktree agents create isolated copies. Reviewers must find the correct worktree by branch name, not assume the first directory.

2. **Proto enum mapping loses feature identity** — Mapping PatientDefense→DODGE at handler level caused protoAbilityIDToRef to return wrong ref. Features without their own CombatAbilityId must go through ActivateFeature directly.

3. **Integration tests catch what unit tests miss** — Unit tests with mocks passed but integration tests with real toolkit characters failed. Always run integration tests before marking API refactors complete.

4. **Spec reviewers can mistake pre-existing code for new additions** — When reviewing test files, reviewer saw all tests in file and thought they were all new. Always check git diff, not just file contents.

5. **Plan review catches real bugs** — Proto field number conflict, incomplete feature mappings, missing event publishing — all caught before implementation started.

## Status
Idea stage. Needs design.md with schema definitions and spawn prompt integration patterns.

## Platform Simplifier Role
See `platform-simplifier-design.md` for the design spec. Role implemented at `docs/teams/roles/platform-simplifier/`. Plan at `platform-simplifier-plan.md`.

## Related
- Existing rpg-project memory system: `schemas/`, `memories.json` at each scope
- Team prompts: `docs/teams/platform-audit-prompts.md`
- Project manager role: draft alongside this system
- Unified entity state idea: `../unified-entity-state/` — the canonical example the simplifier would have caught
