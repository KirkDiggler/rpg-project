# Idea: Unified Entity State

The web client tracks entity data across multiple fragmented state containers (dungeonMap.entities, monsters[], fullCharactersMap, combatState). Every combat event update has to sync across all of them, and they drift — causing bugs like dead monsters still rendering, hover panels showing stale HP, and character models resetting.

## Milestone
4class-dungeon

## Status
**Design approved.** See `design.md` for full spec. Ready for implementation planning.

## The Problem

Today's bug fixes exposed the pattern repeatedly:
- Monster rendered from `dungeonMap.entities`, HP tracked in `monsters[]` → dead monster not removed
- Hover panel reads from `monsters[]`, HP only updated via stream events → always shows "uninjured"
- Character visual state in `fullCharactersMap`, combat events overwrite with partial data → model resets

Each fix was a point solution (merge utilities, render-layer filtering, extra state sync). A unified entity store keyed by ID — holding visual, combat, spatial, and HP data together — would eliminate the entire class of sync bugs.

## Questions to Explore
- Single store vs. derived views from one source of truth?
- How does this interact with the proto event model? Events send partial updates — does the store handle merging?
- Should the API send complete entity snapshots instead of partial updates?
- How does this relate to the multi-room spatial tracking work (rpg-api#397)?
- Performance: would re-rendering on every entity change be too expensive?

## Related Issues
- rpg-dnd5e-web#351 — character model resets (mergeCharacterUpdate workaround)
- rpg-dnd5e-web#357 — hover always uninjured (fragmented HP state)
- rpg-dnd5e-web#358 — dead monster not removed (HP in different container than render)
- rpg-api#397 — multi-room unified spatial tracking (server-side equivalent)

## State
See `memories.json` for structured progress.
