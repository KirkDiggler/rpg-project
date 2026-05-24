# Idea: Scenario-Based Development Rounds

## What This Is

A structured system for organizing game development work around player scenarios instead of individual bug lists. Combines the quality of the design -> plan -> implement cycle with persistent team knowledge and parallel execution.

## Origin

Session 2026-04-04. The whack-a-mole approach to bug fixing was consuming context and producing lower quality than the design -> plan cycle. Kirk wanted to leverage the team roles system to organize work while keeping the strategic conversation layer free from implementation details.

## Key Insight

The organizing unit is a **player scenario** ("Monk kills monster in room 1"), not a bug list. Scenarios naturally touch every layer and force integration quality. Bugs reveal themselves as acceptance criteria failures.

## Status

Design complete. Ready for spec review, then implementation planning.

## Builds On

- `../team-memory-system/` — Context directory structure and domain knowledge
- `../bug-fix-team/` — Worker roles, worktree isolation, issue lifecycle
- `docs/teams/roles/platform-simplifier/` — Architectural review role

## Roles

| Role | Purpose |
|------|---------|
| Strategic Layer (Claude + Kirk) | Co-plan rounds, maintain big picture, PM duties |
| Round Lead | Supervise workers, route discoveries, review PRs |
| Platform Simplifier | Architectural conscience — reviews designs, plans, round outcomes |
| Web/API/Toolkit Fixers | Layer-aligned implementation in worktrees |

## Round Lifecycle

Setup (Kirk + Claude) -> Execution (Lead + Workers) -> Review (Lead + Simplifier) -> Regroup (Kirk + Claude)
