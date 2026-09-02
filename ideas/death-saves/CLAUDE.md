# Idea: Death Saves

A Dying player remains in initiative and receives one explicit, selector-bearing
Death Save verb on their turn. The player throws the same shared physical d20
used by Attack; whole-party Story and continuation remain hidden until an
in-bounds settlement.

## Milestone

Four-player Level-3 Dungeon

## Status

Approved design; implementation not started.

## Current truth

- `brainstorm.md` — verified session-stack census and Kirk's 2026-09-02 rulings.
- `design.md` — approved architecture and proof contract.
- `archive/2026-01-23-condition-design.md` — superseded automatic-condition design.
- `archive/2026-01-23-memories.json` — superseded structured progress snapshot.

The archived design assigned zero-HP detection to rpg-api and auto-rolled from
`UnconsciousCondition` on TurnStart. Do not implement that shape. Toolkit owns
life-state and Death Save rules; encounter owns participation/clocks; session
publishes the explicit verb; rpg-api maps; the web renders provider facts and
the shared die.

No character-module extraction is assumed or active in this design.
