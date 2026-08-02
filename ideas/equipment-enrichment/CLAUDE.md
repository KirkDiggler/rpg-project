# Idea: Equipment Data Enrichment

Flow weapon/armor stats from toolkit through protos and API so the UI can display meaningful equipment details during character creation and beyond.

## Milestone
4class-dungeon

## Status
**Corrected 2026-08-02** — Kirk's scope correction supersedes the original
2026-08-01 phasing. **Phase 1 (now, corrected):** web-only rich rendering of
the *existing production* equipment dropdown, reading fields already present
on `ListEquipmentByType`'s live `Equipment` response — no proto change, no
fixtures/concept lab, no standalone preview (dropped entirely; live data
already exists). Explicitly excludes exact/Monk category eligibility.
rpg-api-protos#204 (the additive-field implementation) is closed without
merge; rpg-api-protos#202, rpg-toolkit#872, rpg-api#755 moved to
Todo/deferred (kept open). **Phase 2 (deferred, unchanged):** toolkit
category-choice expansion (Monk's full-registry resolution is the sharp-edge
test) and the API translation layer that maps a new `options` field, opening
only once Phase 1 (corrected) has shipped; web then deletes
`ListEquipmentByType`'s client-side eligibility reconstruction in the same
PR that starts reading live `options`. See `plan.md` for the corrected phase
gates, compatibility/rollback, and merge order (original Phase 1 kept there
as superseded history). No implementation started on either phase.

## Key Decision
Toolkit enriches at source (not API, not UI). Equipment choices carry full weapon/armor stats inline. UI gets a reusable WeaponCard component.

## Repos
rpg-toolkit, rpg-api-protos, rpg-api, rpg-dnd5e-web
