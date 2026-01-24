# Idea: Two-Level Action Economy

Core combat system architecture. Abilities spend economy to grant capacity, Actions consume capacity to do work. Features activate with resource costs. Conditions listen passively on the event bus.

## Milestone
4class-dungeon

## Status
Core implemented. Gaps being filled (#546). Integration test gate passing.

## Key Files (across repos)
- rpg-toolkit: `character/abilities/`, `character/actions/`, `character/features/`, `character/conditions/`
- rpg-toolkit: `character/integration_test.go`
- rpg-api: PR #403 (two-level action economy RPCs)

## State
See `design.md` for the full architecture and `memories.json` for structured progress.
