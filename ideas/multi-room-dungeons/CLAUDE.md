# Idea: Multi-Room Dungeons with Absolute Positioning

Rooms have absolute positions in dungeon-space. The toolkit calculates offsets. API stores them. UI renders directly without math.

## Milestone
4class-dungeon

## Status
In Progress. Toolkit PR #565 (dungeon generation) and API PR #400 (unified coordinates) open.

## Key Files (across repos)
- rpg-toolkit: PR #565, `rulebooks/dnd5e/dungeon/`
- rpg-api: PR #400
- rpg-dnd5e-web: `feature/cube-coordinates` branch

## State
See `design.md` for architecture and `memories.json` for structured progress.
