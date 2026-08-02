# Idea: Equipment Data Enrichment

Flow weapon/armor stats from toolkit through protos and API so the UI can display meaningful equipment details during character creation and beyond.

## Milestone
4class-dungeon

## Status
**Design updated 2026-08-01** with the concrete-options architecture decision for
the MEATY equipment wave (category choices resolve to concrete enriched
EquipmentItems at the toolkit, additive proto `options` field, web deletes
ListEquipmentByType + client-side eligibility reconstruction). Pending Kirk's
written design approval on the tracking PR before `plan.md` is added. No
implementation started.

## Key Decision
Toolkit enriches at source (not API, not UI). Equipment choices carry full weapon/armor stats inline. UI gets a reusable WeaponCard component.

## Repos
rpg-toolkit, rpg-api-protos, rpg-api, rpg-dnd5e-web
