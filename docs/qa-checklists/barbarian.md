# Barbarian QA Checklist (Level 1)

Class-specific testing for the Barbarian at level 1.

## Identity

- **Hit Dice**: d12
- **Primary Ability**: Strength
- **Armor**: Light, Medium, Shields (no heavy)
- **Weapons**: Simple, Martial
- **Level 1 Features**: Rage (activatable), Unarmored Defense (passive condition)

## Character Creation

- [x] Barbarian class selectable
- [x] Barbarian textures load
- **BUG:** Weapon does not render on model — had greataxe and great club selected but no weapon shows in dungeon
- [x] Starting HP reflects d12 hit dice + CON modifier

## Rage (Activatable Feature — Level 1)

- [x] Appears in features panel as a badge
- **BUG:** No n/n usage counter displayed (should show 2/2, 1/2, 0/2)
- [x] Clickable — listed under features, activates on click
- **BUG:** Not labeled as bonus action in the UI
- [x] "Raging" condition badge appears after activation (turns green with checkmark)
- [?] Usage decrements — cannot verify without counter display
- [x] Cannot Rage when no uses remain (0/2) — confirmed
- [ ] Grayed out / disabled when no bonus action available — not tested

### Rage Effects (while Raging)
- [x] **+2 melee damage** on every melee strike — visible in damage breakdown
- [x] Rage bonus shown as separate component in combat log
- [ ] Rage does NOT apply to ranged attacks — not tested (no ranged weapon)
- [x] Rage persists across multiple turns
- [x] **Resistance to B/P/S damage** — combat log shows halved damage. **BUG:** No UI indicator that resistance is active

### Rage Duration
- [ ] Rage lasts up to 10 rounds — not tested to full duration
- [x] Rage ends if you don't attack or take damage for a turn
- **BUG:** Badge still shows green/checked after Rage expires — should clear
- [x] Can re-activate Rage after it expires (consumes another charge)
- **BUG:** No charge visibility — can't see how many rages remain

## Unarmored Defense (Passive Condition — Level 1)

- [x] AC = 10 + DEX mod + CON mod — **Verified correct**: DEX 13 (+1) + CON 16 (+3) = AC 14
  - Note: STR is +3 but Unarmored Defense uses DEX, not STR
- [x] AC displayed correctly in character panel (shows 14)
- [ ] Shield still adds +2 to Unarmored Defense AC — not tested
- [ ] Does NOT stack with worn armor — cannot test (no armor available in starting equipment)

## Combat Flow (Level 1 Barbarian Turn)

1. [x] Turn starts — 1 action, 1 bonus action, 30ft movement
2. [x] Activate Rage — Raging badge appears, charge consumed (but can't see charge count)
3. [x] Activate Attack (action) — grants 1 strike
4. [x] Strike target — damage includes rage bonus (+2), shown in breakdown
5. [x] Move if movement remains
6. [x] End turn — rage persists

## Level 1 Bugs Found

| Bug | Layer | Description |
|-----|-------|-------------|
| Weapon not rendering | Web | Greataxe/great club selected but not visible on character model |
| No rage charge counter | Web | Feature badge doesn't show n/n usage (e.g., 2/2) |
| Rage not labeled as bonus action | Web | Feature button doesn't indicate it costs a bonus action |
| Rage badge persists after expiry | Web | Green checkmark stays on badge after Rage ends from inactivity |
| Resistance not indicated | Web | B/P/S resistance works (combat log shows it) but no visual indicator on character |
| No LongRest on dungeon start | API | Entering new dungeon doesn't reset rage charges — **FIX IN PROGRESS** (branch fix/playtest-bugs-435, issue #435) |

## Not Applicable at Level 1

These are level 2+ features — do not test:
- Reckless Attack (level 2)
- Danger Sense (level 2)
- Primal Path / Subclass (level 3)
- Extra Attack (level 5)
