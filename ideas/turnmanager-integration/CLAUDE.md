# TurnManager Integration

## What This Is
Wire rpg-toolkit's TurnManager through rpg-api to eliminate 75+ boundary-violating switch statements in the encounter orchestrator.

## Origin
Platform audit 2026-03-22 — all 4 specialists independently flagged this as the #1 problem.
- Audit: `docs/teams/platform-audit/2026-03-22/synthesis.md`
- Issue: #4

## Status
- [x] Research (audit findings)
- [ ] Design (superpowers brainstorm -> spec)
- [ ] Implementation plan
- [ ] Implementation

## Key Files to Read
- rpg-toolkit: `rulebooks/dnd5e/combat/turn_manager.go`, `turn_manager_queries.go`
- rpg-api: `internal/orchestrators/encounter/orchestrator.go` (4,946 lines — the god object)
- rpg-api: `internal/orchestrators/encounter/orchestrator.go:4258-4276` (the switch statement)
- rpg-api-protos: `encounter.proto` (AvailableAbility, AvailableAction messages)

## The Boundary Violation
```go
// WRONG — API knows what features do:
func applyFeatureSideEffects(featureID string, ae *entities.ActionEconomyState) {
    switch featureID {
    case refs.Features.FlurryOfBlows().ID:
        ae.GrantFlurryStrikes(2)  // API knows Flurry grants 2 strikes!
    }
}

// RIGHT — API just passes through:
result, err := tm.UseAbility(ctx, &UseAbilityInput{AbilityRef: featureRef})
// Toolkit decides what gets granted, API reads the result
```

## Phases
1. Load TurnManager per character in combat
2. Remove feature logic from API (delete switch statements)
3. Return available actions from TurnManager in all combat responses
4. Remove ActionEconomyState from API entities
