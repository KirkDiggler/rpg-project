# Memory Types

Structured memory schemas used at any scope (project, milestone, idea). Each type has a defined JSON schema. Empty fields are shown intentionally - they tell agents what's trackable.

## Implemented

| Type | File | Use Case |
|------|------|----------|
| Task Progress | `task-progress.json` | Track what's done, in progress, blocked, todo |
| Decision | `decision.json` | Choices made with reasoning and alternatives |
| Blocker | `blocker.json` | What's preventing progress and why |
| Test Criteria | `test-criteria.json` | Validation requirements and pass/fail state |
| Known Issue | `known-issue.json` | Documented problems with impact and workarounds |
| Note | `note.json` | Context, observations, general information |
| Pattern | `pattern.json` | Reusable approaches, best practices, conventions |
| Lesson Learned | `lesson-learned.json` | What went wrong or worked well across sessions |

## Planned (implement when use case arises)

| Type | Purpose |
|------|---------|
| Follow Up | Action items to revisit at a specific time or trigger |
| Announcement | Team communications, status updates |

## Usage

Each scope (project, milestone, idea) has a `memories.json` containing an array of typed entries. All entries share common fields (`type`, `id`, `created`, `status`) plus type-specific fields defined in their schema.

```json
{
  "scope": "milestone",
  "name": "4class-dungeon",
  "memories": [
    {"type": "task-progress", ...},
    {"type": "decision", ...},
    {"type": "blocker", ...}
  ]
}
```
