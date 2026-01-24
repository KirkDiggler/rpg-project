# Multi-Room Dungeons: Absolute Positioning

## Status: In Progress (PRs Open)

## The Problem

When a player opens a door to room 2, the original implementation rendered it at 0,0,0 (cube coords) and the UI calculated the offset. This is wrong because:
- Client shouldn't do spatial math
- Different clients could calculate differently
- The toolkit should own positioning (it owns all rules)

## The Solution

Rooms have absolute positions in dungeon-space. The toolkit's dungeon package handles this.

### What Exists

**Toolkit PR #565**: `feat(dungeon): Add dungeon generation to toolkit`
- Dungeon package in rulebook layer
- Rooms get absolute offsets when added to dungeon
- Connection system links rooms logically

**API PR #400**: `feat: Add unified dungeon coordinate system`
- API stores dungeon state with absolute coords
- Returns absolute positions in proto responses
- UI renders directly without offset math

**Web branch**: `feature/cube-coordinates`
- Hex grid uses cube coordinates (q, r, s)
- Renders rooms at their absolute positions

### Architecture

```
Dungeon (toolkit/rulebooks/dnd5e/dungeon)
  |
  +-- Room 1 (offset: 0, 0, 0)
  |     +-- Entities at absolute positions
  |     +-- Walls, doors at absolute positions
  |
  +-- Room 2 (offset: 12, 0, -12)  <- toolkit calculates this
  |     +-- Entities at absolute positions
  |     +-- Connection to Room 1 via door
  |
  +-- Room 3 (offset: 24, 0, -24)
        +-- ...
```

### Data Flow

1. Toolkit generates dungeon with room offsets
2. API stores the dungeon state (rooms + offsets + entities)
3. API returns absolute positions in CombatState proto
4. Web renders rooms at their absolute positions
5. When door opens: API sends new room data with absolute coords
6. Web just adds it to the scene (no math needed)

## What's Left

- [ ] Merge cube-coordinates branches across all repos
- [ ] Merge toolkit #565 (dungeon generation)
- [ ] Merge API #400 (unified coordinates)
- [ ] Test multi-room transition with absolute positioning
- [ ] Verify event broadcasts include absolute coords for room reveals

## Design Decisions

- Cube coordinates (q, r, s) for hex grids - no ambiguity, clean math
- Rooms are rectangular regions in dungeon-space
- Connections are logical, not spatial (ADR-0015)
- Spawn layer handles entity placement after room transition
