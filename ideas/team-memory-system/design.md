# Team Memory System — Design Spec

## Goal

Give persistent agent team roles and solo sessions accumulated domain knowledge and operational state that survives across sessions, using the existing schema-driven type system and native CLAUDE.md discovery.

## Problem

Agent team members are ephemeral — they start fresh every spawn. Knowledge discovered during work sessions (patterns, lessons learned, architectural decisions) is lost unless manually saved. The platform audit session (2026-03-22) demonstrated this: 5 lessons learned, multiple patterns discovered, none of which would be available to future agents working in the same repos.

## Architecture

Two layers, one schema contract.

### Layer 1: Domain Knowledge (per-repo)

Knowledge discovered about a codebase lives IN that codebase at `.claude/knowledge/`. Every session — team member or solo — picks it up automatically through native CLAUDE.md discovery.

```
rpg-toolkit/
  CLAUDE.md                              <- Hand-curated (human)
  .claude/
    knowledge/
      context/                           <- Auto-loaded on every session
        patterns.json                    <- Active patterns
        lessons-learned.json             <- Active lessons
        decisions.json                   <- Active architectural decisions
      archive/                           <- Read on demand only
        patterns.json                    <- Deprecated patterns
        lessons-learned.json             <- Superseded lessons
        decisions.json                   <- Superseded decisions

rpg-api/
  .claude/
    knowledge/
      context/
        (same structure)
      archive/
        (same structure)

rpg-dnd5e-web/
  .claude/
    knowledge/
      context/
        (same structure)
      archive/
        (same structure)

rpg-api-protos/
  .claude/
    knowledge/
      context/
        (same structure)
      archive/
        (same structure)
```

Each repo's CLAUDE.md gets a line: "Read `.claude/knowledge/context/` for agent-discovered patterns and lessons."

**Why context/archive:** Domain knowledge grows over time. Deprecated patterns and superseded lessons shouldn't consume context tokens on every session. When a pattern is deprecated or a lesson superseded, the agent moves the entry from `context/` to `archive/`. Archive is only read when explicitly investigating historical decisions.

**Why per-repo:** Knowledge about the toolkit's event bus doesn't help someone working on the web client. Scoping to the repo keeps context relevant and tight.

**Why `.claude/knowledge/`:** The `.claude/` directory is already the convention for Claude-specific configuration. Knowledge files are Claude-specific artifacts.

### Layer 2: Role State (central, in rpg-project)

Operational state for persistent team roles lives centrally in rpg-project since it spans repos.

```
rpg-project/docs/teams/
  roles/
    project-manager/
      prompt.md                          <- Spawn instructions for this role
      context/                           <- Auto-loaded on spawn
        active-prs.json                  <- PR tracking across repos
        active-blockers.json             <- Current blockers
        active-work-items.json           <- In-progress tasks
      archive/                           <- Read on demand only
        closed-prs.json
        resolved-blockers.json
        completed-work-items.json

    code-reviewer/
      prompt.md
      context/
        review-patterns.json             <- What to look for
      archive/
        past-reviews.json
```

**Physical separation enforces autoload:** `context/` = always loaded on spawn. `archive/` = only read when explicitly needed. No filtering logic, no wasted tokens on completed items.

## Schema Contract

Extend `rpg-project/schemas/` with two new type definitions. All entries across both layers use the same schema contract.

### New Type: `lesson-learned`

Captures what went wrong or what worked well during a work session.

```json
{
  "type": "lesson-learned",
  "id": "string (kebab-case slug, e.g., 'integration-tests-catch-mock-gaps')",
  "created": "ISO date string",
  "updated": "ISO date string (set when status changes)",
  "status": "active | superseded",
  "summary": "string (one-line description)",
  "detail": "string (full explanation with context)",
  "source": "string (what session/audit/PR discovered this)",
  "domain": "string (toolkit | api | web | protos | cross-repo)",
  "references": ["string (issue URLs, PR URLs, file paths)"]
}
```

**Lifecycle:** Created as `active`. Set to `superseded` when the codebase changes make the lesson no longer applicable (e.g., a workaround lesson is superseded when the root cause is fixed).

### New Type: `pattern`

Captures reusable patterns discovered through work — both what TO do and what NOT to do.

```json
{
  "type": "pattern",
  "id": "string (kebab-case slug, e.g., 'error-collection-remove')",
  "created": "ISO date string",
  "updated": "ISO date string (set when status changes)",
  "status": "active | deprecated",
  "name": "string (pattern name, e.g., 'error-collection Remove()')",
  "description": "string (what the pattern is and when to use it)",
  "example": "string (code example or reference to file:line)",
  "anti_pattern": "string | null (what NOT to do, null if not applicable)",
  "domain": "string (toolkit | api | web | protos | cross-repo)",
  "references": ["string (file paths, PR URLs)"]
}
```

**Lifecycle:** Created as `active`. Set to `deprecated` when the pattern is replaced by a better approach.

### Existing Types Used

Both domain knowledge and role state reuse existing schema types where they fit:

- `decision` — Architectural decisions made during sessions. Used in `.claude/knowledge/decisions.json`. Uses the existing decision schema as-is (no `domain` field needed since the file is already per-repo).
- `task-progress` — For tracking work items (PM role state)
- `blocker` — For tracking blockers (PM role state, or domain knowledge)
- `known-issue` — For known issues discovered during work (domain knowledge)

No new types needed beyond `lesson-learned` and `pattern`.

### File Format

All knowledge and role state files are **bare JSON arrays** — not wrapped objects:

```json
[
  { "type": "pattern", "id": "error-collection-remove", ... },
  { "type": "pattern", "id": "condition-apply-after-load", ... }
]
```

This is simpler than the existing `memories.json` wrapped format (`{"scope": ..., "memories": [...]}`) because knowledge files don't need scope metadata — the file path already implies the scope (repo for domain knowledge, role for state).

Empty files should be initialized as `[]` (empty array).

### Concrete Example

A complete `rpg-toolkit/.claude/knowledge/context/patterns.json` file:

```json
[
  {
    "type": "pattern",
    "id": "error-collection-remove",
    "created": "2026-03-22",
    "updated": "2026-03-22",
    "status": "active",
    "name": "Error-collection Remove() for conditions",
    "description": "Condition Remove() methods must collect all unsubscribe errors instead of returning on the first failure. Early return leaks remaining subscriptions.",
    "example": "See conditions/raging.go Remove() — collects errors in []error slice, nils state unconditionally, returns aggregated error via errors.Join().",
    "anti_pattern": "for _, id := range subscriptionIDs { if err := bus.Unsubscribe(id); err != nil { return err } } — leaks remaining subscriptions on first error",
    "domain": "toolkit",
    "references": ["https://github.com/KirkDiggler/rpg-toolkit/pull/603"]
  },
  {
    "type": "pattern",
    "id": "feature-activation-direct-ref",
    "created": "2026-03-22",
    "updated": "2026-03-22",
    "status": "active",
    "name": "Feature activation uses direct refs, not proto conversion",
    "description": "ActivateFeature must call char.ActivateAbility() with the toolkit feature ref directly. Mapping through proto enums (feature -> CombatAbilityId -> back to ref) loses feature identity — PatientDefense becomes generic Dodge, wrong action cost and no ki spent.",
    "example": "See rpg-api orchestrator.go ActivateFeature — uses featureIDToRef() then char.ActivateAbility(ref) directly.",
    "anti_pattern": "Mapping PatientDefense -> COMBAT_ABILITY_ID_DODGE -> protoAbilityIDToRef -> refs.CombatAbilities.Dodge() — loses the feature, activates wrong ability",
    "domain": "api",
    "references": ["https://github.com/KirkDiggler/rpg-api/pull/447"]
  }
]
```

## Write Rules

### Who writes what

| Writer | Domain Knowledge | Role State |
|--------|-----------------|------------|
| Any agent | Append entries to repo's `.claude/knowledge/` | No |
| Role agent (PM, reviewer) | Append entries to repo's `.claude/knowledge/` | Read/write own `context/` and `archive/` |
| Human (Kirk) | Edit anything | Edit anything |

### How agents write

Agents **append** entries to JSON arrays. They never rewrite files. The schema enforces structure — agents fill in fields according to the type definition.

**Append pattern:**
```
1. Read the file (or create if missing)
2. Parse JSON array
3. Append new entry with all required fields
4. Write file back
```

**No deduplication required at write time.** If noise accumulates, periodic cleanup by PM or human. Start simple, add gates only if needed.

### When agents write

- **Patterns:** When an agent discovers a reusable approach during implementation (e.g., "conditions should use error-collection Remove()")
- **Lessons learned:** When something goes wrong or succeeds in a non-obvious way (e.g., "integration tests catch what unit tests miss")
- **Decisions:** When an architectural choice is made with rationale (e.g., "ActivateFeature calls char.ActivateAbility directly instead of delegating through proto conversion")

Agents should write as they discover — not batch at the end. Knowledge lost to session timeout is knowledge lost forever.

## Agent Lifecycle

### On spawn (persistent role)

1. Read `prompt.md` for role instructions
2. Read everything in role's `context/` directory (operational state)
3. Read `.claude/knowledge/context/` from relevant repo(s) (domain knowledge via CLAUDE.md)
4. Agent has full context — no filtering, no wasted tokens

### During work

1. Append domain knowledge entries to `context/` files as discovered (patterns, lessons, decisions)
2. Update role state as needed (PR status changes, blockers resolved)
3. Move deprecated/superseded entries from `context/` to `archive/` when knowledge is replaced

### On shutdown

1. Move completed items from `context/` to `archive/` (role state only)
2. Domain knowledge stays in `context/` unless superseded/deprecated (then move to `archive/`)

### On spawn (solo session, not a team)

1. CLAUDE.md in the repo references `.claude/knowledge/`
2. Session reads knowledge files as part of normal CLAUDE.md discovery
3. No role state — solo sessions don't have a role
4. Solo sessions CAN write domain knowledge (same append pattern)

## CLAUDE.md Integration

Each repo's CLAUDE.md gets this addition:

```markdown
## Agent-Discovered Knowledge

Read `.claude/knowledge/context/` for patterns, lessons learned, and
decisions discovered by previous work sessions. These complement the
hand-curated conventions above.

When you discover a new pattern or learn something non-obvious during
your work, append it to the appropriate file in `.claude/knowledge/context/`.
When a pattern is deprecated or a lesson superseded, move the entry
from `context/` to `archive/`.
```

This is the only integration point. No special loading logic, no custom tooling. CLAUDE.md already tells agents what to read — we're just telling them to read one more directory.

## Initial Content

### Domain knowledge from platform audit (2026-03-22)

**rpg-toolkit patterns.json:**
- Error-collection Remove() pattern for condition subscriptions
- Condition round-trip: Apply() must be called after LoadFromData deserialization
- 5-stage chain ordering: Features → Conditions → Equipment

**rpg-toolkit lessons-learned.json:**
- Integration tests catch what unit tests miss
- Proto enum mapping loses feature identity — use direct refs
- Spec reviewers can mistake pre-existing code for new additions — check git diff

**rpg-api lessons-learned.json:**
- Boundary violations compound — every switch on game enums requires API changes for new features
- ActionEconomy should be single-sourced from character data, not dual-stored

**rpg-api patterns.json:**
- Feature activation routes through char.ActivateAbility() directly, never through proto conversion
- syncCharActionEconomyToEncounter bridges character state to encounter entity

### Role state for project-manager

**active-prs.json:** Current open PRs across all repos
**active-work-items.json:** Remaining tasks from platform audit plans

## File Structure Summary

```
rpg-project/
  schemas/
    lesson-learned.json              <- NEW type definition
    pattern.json                     <- NEW type definition
    (existing: decision, blocker, known-issue, task-progress, note, test-criteria)
  docs/teams/
    roles/
      project-manager/
        prompt.md                    <- PM spawn instructions
        context/                     <- Auto-loaded operational state
          active-prs.json
          active-blockers.json
          active-work-items.json
        archive/
          closed-prs.json
          resolved-blockers.json
          completed-work-items.json

rpg-toolkit/.claude/knowledge/
  context/                           <- Auto-loaded every session
    patterns.json                    <- Active patterns
    lessons-learned.json             <- Active lessons
    decisions.json                   <- Active decisions
  archive/                           <- Read on demand
    patterns.json                    <- Deprecated/superseded entries

rpg-api/.claude/knowledge/
  context/                           <- (same structure)
  archive/

rpg-dnd5e-web/.claude/knowledge/
  context/                           <- (same structure)
  archive/

rpg-api-protos/.claude/knowledge/
  context/                           <- (same structure)
  archive/
```

## Out of Scope

- **Automated deduplication** — Start without it. Add if noise becomes a problem.
- **Cross-domain knowledge queries** — Agents read their repo's knowledge. Cross-repo patterns go in rpg-project or get duplicated where relevant.
- **Knowledge validation/review gates** — Any agent can write. Add review gates only if quality degrades.
- **UI for browsing knowledge** — Files are the interface. Git history provides audit trail.
- **Migration of existing CLAUDE.md patterns into knowledge files** — CLAUDE.md stays hand-curated. Knowledge files capture NEW discoveries.
