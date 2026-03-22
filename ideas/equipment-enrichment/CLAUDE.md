# Idea: Equipment Data Enrichment

Flow weapon/armor stats from toolkit through protos and API so the UI can display meaningful equipment details during character creation and beyond.

## Milestone
4class-dungeon

## Status
**Design drafted.** Pending review. No implementation started.

## Key Decision
Toolkit enriches at source (not API, not UI). Equipment choices carry full weapon/armor stats inline. UI gets a reusable WeaponCard component.

## Repos
rpg-toolkit, rpg-api-protos, rpg-api, rpg-dnd5e-web
