# Idea: Rest System

Resource recovery between encounters. Long rest (automatic at dungeon start) restores everything. Short rest (between rooms) lets players spend hit dice.

## Milestone
4class-dungeon

## Status
In Progress (Issue #294). Nothing blocking architecturally - event-driven resource system already built.

## Key Files (across repos)
- rpg-toolkit: `RecoverableResource`, `RestTopic` (already implemented)
- rpg-api: Issue #294

## State
See `design.md` for the implementation plan and `memories.json` for structured progress.
