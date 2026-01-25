# Idea: Two-Level Action Economy

Core combat system architecture. Abilities spend economy to grant capacity, Actions consume capacity to do work. Features activate with resource costs. Conditions listen passively on the event bus.

## Milestone
4class-dungeon

## Status
**TurnManager merged (PR #573) with integration tests (PR #575)** - Ready for API integration.

## What TurnManager Provides

```go
// Create per turn with all dependencies
tm, _ := combat.NewTurnManager(&combat.NewTurnManagerInput{
    Character:  character,   // The combatant whose turn it is
    Combatants: lookup,      // All combatants for target resolution
    Room:       room,        // Spatial room for movement
    EventBus:   bus,         // For turn lifecycle events
    Roller:     roller,      // Dice roller (optional)
})

// Lifecycle
tm.StartTurn(ctx)          // Sets movement, publishes TurnStartEvent
tm.EndTurn(ctx)            // Publishes TurnEndEvent, cleanup

// Abilities (consume economy, grant capacity)
tm.UseAbility(ctx, &UseAbilityInput{AbilityRef: refs.CombatAbilities.Attack()})
tm.UseAbility(ctx, &UseAbilityInput{AbilityRef: refs.CombatAbilities.Dash()})

// Actions (consume capacity, do work)
tm.Strike(ctx, &StrikeInput{TargetID: "goblin-1", Weapon: sword})
tm.Move(ctx, &MoveInput{Path: path})
tm.OffHandStrike(ctx, &OffHandStrikeInput{TargetID: "goblin-1", Weapon: dagger})
tm.FlurryStrike(ctx, &FlurryStrikeInput{TargetID: "goblin-1", Weapon: unarmed})

// Queries for UI
tm.GetAvailableAbilities(ctx)  // []AvailableAbility with CanUse, Reason
tm.GetAvailableActions(ctx)    // []AvailableAction with CapacityType
tm.GetEconomy()                // Current ActionEconomy state
```

## Next Steps

1. **API**: Thin orchestrator that creates TurnManager, calls methods, persists state
2. **API**: Handler converts proto enums → `*core.Ref` at boundary
3. **Web**: Wire up after API ready

## Testing Coverage

**Unit Tests** (`turn_manager_test.go`):
- Construction validation
- Turn lifecycle (StartTurn, EndTurn, events)
- Full attack turn flow
- Movement and insufficient movement
- Dash extends movement
- Economy exhaustion
- Strike requires attack ability first
- Disengage prevents OA
- GetAvailableAbilities/Actions queries

**Integration Tests** (`integration_test.go`):
- Barbarian rage damage bonus
- Rage resistance to B/P/S
- Critical hits with rage
- Second Wind healing
- Archery fighting style +2
- Great Weapon Fighting rerolls

**TurnManager Integration Tests** (PR #575):
- Full fighter turn: Attack ability → 2 strikes → movement
- Availability queries reflect economy state
- Dash doubles movement
- Dodge consumes action and publishes event
- Strike requires attack capacity
- Move validates path

## Key Files
- `combat/turn_manager.go` - Main orchestrator
- `combat/turn_manager_actions.go` - Strike, Move, OffHandStrike, FlurryStrike
- `combat/turn_manager_abilities.go` - UseAbility
- `combat/turn_manager_queries.go` - GetAvailableAbilities, GetAvailableActions
- `combat/action_economy.go` - ActionEconomy type

## Related Issues
- #571: Activate() function (CLOSED - implemented via TurnManager)
- #572: Execute() function (OPEN - relates to API integration)
- #574: TurnManager integration tests (PR #575)
- #546: Complete ability/action system

## State
See `memories.json` for structured progress and decisions.
