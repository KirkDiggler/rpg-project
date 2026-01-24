# Idea: Death Saves

When a character hits 0 HP, they fall unconscious and make death saving throws. Fits the Condition pattern perfectly - UnconsciousCondition subscribes to turn start, damage, and healing events.

## Milestone
4class-dungeon

## Status
Todo (Issue #296). Design complete, implementation not started.

## Key Files (across repos)
- rpg-toolkit: Condition pattern in `character/conditions/`
- rpg-api: Issue #296, Issue #363 (alive validation)

## State
See `design.md` for the rules and `memories.json` for structured progress.
