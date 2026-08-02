# Idea: Equipment Data Enrichment

Flow weapon/armor stats from toolkit through protos and API so the UI can display meaningful equipment details during character creation and beyond.

## Milestone
4class-dungeon

## Status
**Plan added 2026-08-01** — Kirk approved landing the concrete-options wave in
two phases rather than as one cross-repo change. **Phase 1 (now):** proto
contract (additive `options` field) + a rich, accessible web picker built
against typed fixtures, run standalone via an equipment preview on port 3002.
No wiring of the picker to production or to any fake/reconstructed
eligibility. **Phase 2 (deferred):** toolkit category-choice expansion
(Monk's full-registry resolution is the sharp-edge test) and the API
translation layer, opening only once Phase 1's gate is clear; web then swaps
the picker's fixture input for the live wire and deletes
`ListEquipmentByType` + the client-side eligibility reconstruction in the same
PR. Dungeon-builder's dev server (port 3001) and its lab API/Redis stack are
untouched by this wave. See `plan.md` for phase gates, compatibility/rollback,
the later local-toolkit-override step, and merge order. No implementation
started.

## Key Decision
Toolkit enriches at source (not API, not UI). Equipment choices carry full weapon/armor stats inline. UI gets a reusable WeaponCard component.

## Repos
rpg-toolkit, rpg-api-protos, rpg-api, rpg-dnd5e-web
