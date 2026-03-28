# Idea: Bug Fix Team

Coordinated multi-agent team for fixing QA bugs across layers (web, API, toolkit). Lead triages and reviews, layer-aligned workers implement in worktrees, GH milestone is the shared state.

## Milestone
4class-dungeon

## Status
**Design complete.** Role prompts and milestone setup not yet implemented.

## Team Structure

| Role | Repos | Purpose |
|------|-------|---------|
| Bug Fix Coordinator (lead) | all (read-only) | Triage, assign, review PRs |
| Web Fixer | rpg-dnd5e-web | Fix web/UI bugs |
| API Fixer | rpg-api, rpg-api-protos | Fix server/orchestrator bugs |
| Toolkit Fixer | rpg-toolkit | Fix rules engine bugs (as needed) |

## Key Design Decisions

1. GH milestone as shared state (no local context files)
2. Lead reviews before Kirk sees PRs
3. Workers stay in their layer — cross-layer issues become new bugs
4. Workers can file `bug` issues, flag `gap` items to Kirk
5. Worktree isolation so Kirk can keep testing locally

## Next Steps

1. Write role prompts in `docs/teams/roles/`
2. Create GH milestone (or use existing 4-class dungeon milestone)
3. File today's QA bugs as issues
4. Run first session

## Related
- `docs/teams/roles/` — where role prompts live
- `docs/teams/platform-audit-prompts.md` — prior team pattern
- `docs/qa-checklists/` — bug source material

## State
See `memories.json` for structured progress and decisions.
