# Playtest: Round 1 — Monk kills monster in room 1

**Date:** 2026-04-05
**Tracking Issue:** KirkDiggler/rpg-project#8
**Status:** COMPLETE — All PRs merged

## Final Results

| # | Criteria | Result | Notes |
|---|----------|--------|-------|
| 1 | Unarmed strike selectable | PASS | |
| 2 | Displays correctly (not "unknown weapon") | PASS | |
| 3 | Correct damage (1d4 + DEX) | PASS | |
| 4 | Monster HP updates | PASS | Bloodied shown correctly |
| 5 | HP and AC correct | PASS | Fixed: AC GameContext wiring, DEX label |
| 6 | Monster death visible | PASS | Dead monsters stay on board, tilted grey |
| 7 | Combat log shows rolls | PASS | |
| 8 | Monster state persists | PASS | |

## PRs Merged

- rpg-toolkit#607 — DEX label fix in Martial Arts damage breakdown
- rpg-toolkit#609 — AC chain tests proving Unarmored Defense WIS modifier
- rpg-api#454 — Unarmed strike, entity state events, AC GameContext, action reset, HP preservation
- rpg-dnd5e-web#371 — Entity state wiring, player HP, dead monster display

## Bugs Found and Fixed During Round

| # | Layer | Description | Issue | Fix |
|---|-------|-------------|-------|-----|
| 1 | API | No unarmed strike option | #437 | addUnarmedStrikeOption in orchestrator |
| 2 | API | Weapon fallback was Greataxe | #438 | Stale naming, code was correct |
| 3 | Web | Monster HP never updates | #372 | Wire applyEncounterCombatState to RPC responses |
| 4 | Web | Monster death not visible | #373 | Keep dead entities, render tilted/grey |
| 5 | API | AC missing WIS in combat log | #456 | GameContext not wired in resolveMonsterAttack |
| 6 | API | Action economy not reset on turn | — | char.EndTurn() + TurnNumber invalidation |
| 7 | API | Player HP reset to full on attack | — | applyCurrentCombatHP from enc.CharacterHP |
| 8 | API | Player HP not updating after monster attack | — | syncCharacterHPFromMonsterTurns |
| 9 | Toolkit | STR label instead of DEX | #605 | Update component.SourceRef |

## Known Intermittent

"Insufficient action" after EndTurn sometimes occurs but cannot be consistently reproduced. Likely timing between EndTurn response and TurnChange event.

## Observations

- Move shows under available strikes only after clicking Attack button
- Hammer icon in combat log for unarmed strike (cosmetic)
- API is where most bugs live — wiring between toolkit and web

## Key Learnings

- Toolkit is solid — all issues were API wiring or web state reading
- Entity state plumbing is the #1 source of bugs: who updates what, when, from which data
- Copilot catches real issues — added to all worker prompts
- Scenario-based rounds work well for finding integration bugs
