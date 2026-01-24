# Idea: Two-Level Action Economy

Core combat system architecture. Abilities spend economy to grant capacity, Actions consume capacity to do work. Features activate with resource costs. Conditions listen passively on the event bus.

## Milestone
4class-dungeon

## Status
Toolkit complete. API PR #403 ready to merge (CI passing). **Next: wire up rpg-dnd5e-web.**

## Next Session Start Here

1. Merge rpg-api PR #403 (ActivateCombatAbility + ExecuteAction RPCs)
2. Upgrade web protos: `npm i --save github:KirkDiggler/rpg-api-protos#v0.1.80`
3. Create hooks + update ActionPanel to use two-level flow
4. See `memories.json` note `afs-next-session` for detailed steps

## Key Files (across repos)
- rpg-toolkit: `character/abilities/`, `character/actions/`, `character/features/`, `character/conditions/`
- rpg-toolkit: `character/integration_test.go`
- rpg-api: PR #403 (ActivateCombatAbility + ExecuteAction handlers/orchestrator)
- rpg-dnd5e-web: `src/components/combat-v2/panels/ActionPanel.tsx`, `src/api/encounterHooks.ts`
- rpg-api-protos: `ActivateCombatAbilityRequest/Response`, `ExecuteActionRequest/Response`

## State
See `design.md` for the full architecture and `memories.json` for structured progress.
