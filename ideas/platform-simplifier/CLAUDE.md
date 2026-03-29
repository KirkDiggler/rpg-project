# Idea: Platform Simplifier Role

A new team role that reads accumulated knowledge and upcoming work across all repos, identifies structural friction, and briefs the team before workers start.

## Milestone
4class-dungeon

## Status
**Design complete.** Ready for plan.

## The Problem

Individual workers see their repo. The PM sees operational status. Nobody sees the structural picture automatically. Cross-cutting cleanup opportunities (like unified-entity-state — 3 bugs sharing a root cause in fragmented state) are only caught after enough pain accumulates and Kirk manually spots the pattern.

## Design

See `design.md` for the full spec. Key points:

- **Reports first** in team startup, before PM and workers
- **Reads** knowledge from all repos, recent git history, project board, PM state, existing ideas
- **Analyzes** via three passes: pattern clustering, friction forecasting, drift detection
- **Produces** session brief (always), knowledge updates (when warranted), idea seeds (high confidence)
- **State** in `docs/teams/roles/platform-simplifier/context/` — previous briefs and active observations

## Related
- rpg-project/ideas/team-memory-system — the knowledge system this role reads and writes
- rpg-project/ideas/unified-entity-state — the canonical example this role would have caught
- docs/teams/roles/ — existing team role definitions

## State
See `memories.json` for structured progress.
