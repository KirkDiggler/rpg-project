# Vocabulary

Consistent terminology across all repos and sessions.

## Repos

| Term | Repo | Meaning |
|------|------|---------|
| Game server | rpg-api | The data orchestrator |
| Toolkit | rpg-toolkit | The rules engine (all layers) |
| Rulebook | rpg-toolkit/rulebooks/dnd5e | D&D 5e specific rules |
| The web | rpg-dnd5e-web | React UI / Discord Activity |
| Protos | rpg-api-protos | API contract definitions |

## Architecture

| Term | Meaning |
|------|---------|
| Handler | gRPC endpoint implementation (API layer) |
| Orchestrator | Business logic that calls toolkit (API layer) |
| Repository | Data persistence with Input/Output types (API layer) |
| Core | Foundation interfaces: Entity, Action, Chain, Effect (Toolkit) |
| Mechanics | Game-agnostic systems: conditions, resources, proficiency (Toolkit) |
| Tools | Infrastructure: spatial, spawn, environments (Toolkit) |
| Rulebook | Game-specific rules: D&D 5e combat, classes, features (Toolkit) |

## Game Mechanics

| Term | Meaning | Example |
|------|---------|---------|
| Feature | A core.Action that can be activated | Rage, Second Wind, Action Surge |
| Condition | An applied effect that listens on the bus | Raging, Prone, Blessed |
| Activate | Player triggers action, costs resources | Rage.Activate() consumes a use |
| Apply | Subscribe to event bus for passive effects | RagingCondition.Apply() listens for damage |
| Chain | Staged modifier pipeline | Damage chain: base -> features -> conditions -> equipment -> final |
| Reference | A key identifying game content | "dnd5e:features:rage", "dnd5e:weapons:greataxe" |
| Breakdown | Detailed result for rendering decisions | [{source: "rage", amount: 2}, {source: "strength", amount: 3}] |

## Events & Multiplayer

| Term | Meaning |
|------|---------|
| Event Bus | Pub/sub system for game state changes |
| Topic | Typed event channel (AttackTopic, ConditionAppliedTopic) |
| Chained Topic | Topic that feeds into a processing chain |
| Broadcast | Sending event to all participants (not just the acting player) |
| Stream | gRPC server-stream for real-time event delivery to clients |

## Combat

| Term | Meaning |
|------|---------|
| Action Economy | Actions, bonus actions, reactions available per turn |
| Initiative | Turn order determined by d20 + DEX |
| Monster Turn | Automated monster actions on EndTurn (movement, attacks) |
| Dungeon | Multi-room encounter with absolute positioning |
| Room | Single combat space with its own grid |
| Connection | Logical link between rooms (door, stairs, portal) |

## Development

| Term | Meaning |
|------|---------|
| Input/Output types | Every function takes *XInput, returns *XOutput |
| Outside-in | Start at handler, define interfaces, implement inward |
| Pre-commit | `make pre-commit` - must pass before committing |
| CI-check | `make ci-check` / `npm run ci-check` - must pass before pushing |
| Fresh branch | Always start from latest main, never reuse old branches |
