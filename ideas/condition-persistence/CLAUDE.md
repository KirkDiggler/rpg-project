# Condition Persistence Fix

## What This Is
Fix broken condition serialization round-trip and CharacterHP dual-storage inconsistency.

## Origin
Platform audit 2026-03-22 — Engine Architect and Server Architect both flagged independently.
- Audit: `docs/teams/platform-audit/2026-03-22/synthesis.md`
- Issue: #5

## Status
- [x] Research (audit findings)
- [ ] Design (superpowers brainstorm -> spec)
- [ ] Implementation plan
- [ ] Implementation

## The Problem
1. **Conditions don't re-wire on load**: Save character with Raging -> load -> data exists but event bus subscriptions lost -> damage bonus doesn't apply
2. **CharacterHP in two places**: EncounterData.CharacterHP AND character.Data both store HP. If one fails to persist, silent inconsistency.
3. **Subscription cleanup silent failures**: Unsubscribe returns on first error, silently succeeds for missing IDs

## Key Files to Read
- rpg-toolkit: `rulebooks/dnd5e/conditions/raging.go` (ToJSON/loadJSON pattern)
- rpg-toolkit: `rulebooks/dnd5e/features/loader.go` (existing feature loader — model for condition loader)
- rpg-api: `internal/repositories/encounters/repository.go:89` (CharacterHP in EncounterData)
- rpg-toolkit: `events/bus.go` (Unsubscribe behavior)

## Phases
1. Complete condition loader (factory: JSON -> peek Ref -> instantiate + Apply)
2. Character.LoadFromJSON re-subscribes conditions to bus
3. Remove CharacterHP from EncounterData (single source of truth in character repo)
4. Improve subscription cleanup logging
