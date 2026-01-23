# Milestone: 4-Class Multiplayer Multi-Room Dungeon

## Project Board
https://github.com/users/KirkDiggler/projects/10

## Status: ~137 Done | 2 In Progress | ~22 Todo

*Last updated: 2026-01-23*

## Recently Completed (This Session)

- **rpg-toolkit #546 (partial)**: Turn end cleanup - Character.Cleanup() removes temporary actions (PR #568, merged)
- **rpg-toolkit #546 (partial)**: GetSpeed + GetExtraAttacksCount (PR #567, merged)
- **rpg-toolkit #505 (discovery)**: ResolveAttack already fully implemented in combat/attack.go
- **Integration tests**: Attack resolution with conditions (AttackChain + DamageChain), critical hits, turn lifecycle

## In Progress

- **rpg-toolkit #505**: Movement integration tests (PR #570)
- **rpg-api #294**: Rest System - Long rest before dungeon start
- **rpg-api #399**: Unified Dungeon Coordinate System

## Current Focus: Integration Test Gate (Nearly Complete)

All major combat pipelines proven end-to-end with real Conditions:
- AttackChain + DamageChain (advantage/disadvantage, rage bonus, crits)
- MovementChain (DisengagingCondition prevents OA, Dash doubles movement)
- Turn lifecycle (temporary action cleanup)

**Done:** Attack resolution, movement, turn cleanup
**Last piece:** Full-round multi-character test (turn order)
**Then:** Ready for API layer integration

See `ideas/action-feature-system.md` for full status.

## Todo - Grouped by Theme

### Multi-Room Dungeon (Core Goal)
- rpg-toolkit #535: Track entities at Environment level with dungeon-absolute coords
- rpg-toolkit #541: Room assembly with absolute coordinate conversion
- rpg-toolkit #542: Door as entity type with state
- rpg-toolkit #543: Collision detection with absolute coordinates
- rpg-toolkit #544: Spatial queries with absolute coordinates
- rpg-toolkit #545: Dungeon generator with theme and encounter integration
- rpg-api #401: Populate Room.origin and convert walls to absolute coordinates
- rpg-api #402: Simplify dungeon orchestrator to use toolkit absolute coordinates
- rpg-api #393: Multi-room encounter management and monster pursuit
- rpg-dnd5e-web #310: Room switching UI for multi-room encounters
- rpg-dnd5e-web #311: Accumulate revealed rooms into single dungeon map state
- rpg-dnd5e-web #312: Render dungeon map using Room.origin for floor tile positioning
- rpg-dnd5e-web #313: Camera follows player through multi-room dungeon

### Combat System
- rpg-api #296: Death Saves - 0 HP unconscious and saving throws
- rpg-api #363: Character alive validation before allowing combat actions
- rpg-toolkit #505: Attack resolution and reactions (ADR-0027)
- rpg-toolkit #546: Complete ability/action system (turn cleanup, Help/Hide, GetTotalSpeed)
- rpg-api #385: Refactor orchestrator to use gamectx combatant pattern (ADR-0026)

### Quality & Polish
- rpg-api #369: Multiplayer - Combat events not published to all stream subscribers
- rpg-api #383: Pathfinding validation and fallbacks
- rpg-toolkit #476: Grant tests for Fighter, Barbarian, Monk
- rpg-toolkit #490: Audit all starter monsters for correct actions/traits
- rpg-dnd5e-web #258: Feature & Condition card components with rich runtime data
- rpg-dnd5e-web #266: Progress Indicator - Room counter and mini-map

## Open PRs

### rpg-toolkit
| PR | Title | Related Issues |
|----|-------|----------------|
| #565 | Dungeon generation | #545, #564 |

### rpg-api
| PR | Title | Related Issues |
|----|-------|----------------|
| #403 | Two-level action economy RPCs | #546 |
| #400 | Unified dungeon coordinate system | #399, #401 |
