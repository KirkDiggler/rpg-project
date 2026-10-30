# The studio draws a patrol route and gives an NPC an extra action

**Scene:** the Encounter Studio on :3030: Site → Import YAML → `the-mad-king.yaml` → Replace draft; then Tables (routes) and Cast (actions).  
**Status:** in studio. Merged to web `dev`; the live play steps (the guard walks a new loop, the captain throws the added javelin) are not yet walked on :3030.  
**Story:** [rpg-project#714](https://github.com/KirkDiggler/rpg-project/issues/714) S6 ([#720](https://github.com/KirkDiggler/rpg-project/issues/720)) and S7 ([#721](https://github.com/KirkDiggler/rpg-project/issues/721))  
**Roadmap:** [rpg-project#714](https://github.com/KirkDiggler/rpg-project/issues/714)

## Idea

The keep's brief asks that "patrols visit authored locations through the
actual connected dungeon" and for "item-granted actions usable by NPCs"
([rpg-project#702](https://github.com/KirkDiggler/rpg-project/issues/702)).
The engine reads `toward: { at }` and `toward: { path }`, and applies
`addActions` and `dropActions`, but the studio offered no picker for either:
every patrol in the keep's file was a hand edit, and no form authored an
extra action.

## Decision

- No ruling: both slice issues say "no decision needed" ([#720](https://github.com/KirkDiggler/rpg-project/issues/720), [#721](https://github.com/KirkDiggler/rpg-project/issues/721)).
- The player half, an item that hands a sheet an activation, is the Grant story and not this card ([#721](https://github.com/KirkDiggler/rpg-project/issues/721)).
- Knowledge line: none.

## Implementation

- rpg-dnd5e-web · [#1354](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1354) · "a cell" and "a route" on `toward`, picked on the Layout with numbered waypoints, per-waypoint Wait and Facing; the keep's routes round-trip byte for byte; the engine's retrace refusal lands at the row. · [run record](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/dev/knowledge/runs/2026-10-30-franks-list-s6-routes.md)
- rpg-dnd5e-web · [#1353](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1353) · Add action and Drop action on a stat block and on a placement, staged in one transaction; the Read back says what the creature is left fighting with. · [run record](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/dev/knowledge/runs/2026-10-30-franks-list-s7-actions.md)
- Not built: an unknown weapon id or an absent drop is refused only at launch, not at validate time ([rpg-toolkit#2168](https://github.com/KirkDiggler/rpg-toolkit/issues/2168)). The toolkit keep's header still calls the patrol and march rows "Hand edit".

## Verify

- Tables → gate-patrol → the `patrol` line shows its three waypoints as chips, not a sentence saying "no picker".
- Tables → a `toward` row → Edit route → click cells on the Layout → the chips rebuild from the clicks; Download YAML shows `path:` with `q`, `r`, `wait`.
- Click the same cell twice and Save & Play → refused with the engine's retrace sentence, jumping to the row.
- Cast → Advanced → Guard Captain → Add action `dnd5e:weapons:javelin` → listed after the longsword; the Read back names it.
- Save & Play and let the captain act → he throws a javelin, and the Debug record shows the action.
