# Milestone: 4-Class Multiplayer Multi-Room Dungeon

## Project Board
https://github.com/users/KirkDiggler/projects/10

## Status: 28/30 Issues Complete (93%)

## Remaining Work

### In Progress
- **rpg-api #294**: Rest System - Long rest before dungeon start
  - Toolkit has RecoverableResource + RestTopic already
  - API needs to call rest at dungeon start
  - See: ideas/rest-system.md

### Todo
- **rpg-api #296**: Death Saves - 0 HP unconscious and saving throws
  - Condition-based design (UnconsciousCondition)
  - See: ideas/death-saves.md

## Open PRs (Need Merge/Review)

### rpg-toolkit
| PR | Title | What It Enables |
|----|-------|-----------------|
| #565 | Dungeon generation | Multi-room with absolute coords |
| #563 | Disengaging condition | Opportunity attacks / retreat |
| #264 | Domain-specific ref packages | Type-safe feature/condition refs |

### rpg-api
| PR | Title | What It Enables |
|----|-------|-----------------|
| #403 | Two-level action economy RPCs | Proper action/bonus/reaction tracking |
| #400 | Unified dungeon coordinate system | Multi-room rendering |

### rpg-api-protos
| PR | Title | What It Enables |
|----|-------|-----------------|
| #85 | Deprecate EQUIPMENT_SLOT_GLOVES | Cleanup |

## What's Working (Done)

### Combat Core
- Attack resolution with damage breakdowns
- Damage chains with feature/condition modifiers
- Critical hits with dice doubling
- Monster turns (movement, targeting, attacks)
- Action economy (action, bonus action, reaction)
- Initiative and turn order

### Characters (4 Classes)
- Fighter: Second Wind, Action Surge, Fighting Styles
- Barbarian: Rage (activate + condition)
- Rogue: Sneak Attack, Expertise
- Monk: (base implementation)

### Multiplayer
- Event-driven state broadcast
- Lobby system with ready-up
- Multi-character combat
- Stream-based event delivery

### Spatial
- Hex grid with cube coordinates
- Pathfinding around walls
- Line of sight
- Monster spawn placement (not on walls)
- Room generation with entity placement

### UI
- Character creation flow
- Combat panel with action buttons
- Hex grid rendering
- Damage source display
- Equipment management

## What's Next After This Milestone

Potential next priorities (not committed):
- Multi-room dungeon transitions (PRs need to merge first)
- Short rest between rooms
- More monster variety
- Spell casting system
- Level advancement
