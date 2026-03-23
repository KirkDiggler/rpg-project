# Fighter QA Checklist (Level 1)

Class-specific testing for the Fighter at level 1.

## Identity

- **Hit Dice**: d10
- **Primary Ability**: Strength
- **Armor**: Light, Medium, Heavy, Shields
- **Weapons**: Simple, Martial
- **Level 1 Features**: Second Wind (activatable), Fighting Style (passive condition)

## Character Creation

- [x] Fighter class selectable
- [x] Fighter textures load (heavy armor look)
- **BUG:** Weapon does not render on model (cross-class issue #337)
- [ ] Shield renders if equipped — not tested
- [x] Starting HP reflects d10 hit dice + CON modifier (12 = 10 + 2 CON mod)
- [x] Fighting Style chosen during class selection (Two-Weapon Fighting confirmed)
- [x] Starting equipment slots populated: chain-mail, shortsword (main), scimitar (off-hand)

## Second Wind (Activatable Feature — Level 1)

- [x] Appears in features panel — clickable, activates
- **BUG:** No icon displayed (should be 💚) — same as Rage, generic badge
- **BUG:** No usage counter (n/n) — same issue as Rage (#338)
- **BUG:** Not labeled as bonus action (#339)
- [x] Heals HP — healing works when activated
- [x] HP bar updates after healing
- [ ] Cannot heal above max HP — not tested
- [x] Cannot activate again after use (0/1) — but no gauge shown
- [ ] Grayed out / disabled when no bonus action available — not tested

## Fighting Style: Two-Weapon Fighting (Passive Condition — Level 1)

- [x] Condition applied: `fighting_style_two_weapon_fighting` confirmed in character data
- [ ] Adds ability modifier to off-hand damage — **cannot verify, Off-Hand Strike target selection is broken (#323)**
- **BUG:** No UI indicator showing which Fighting Style is active or what it does

## Two-Weapon Fighting Flow

- [x] After main-hand Strike, Off-Hand Strike appears as bonus action
- **BUG:** Cannot select target for Off-Hand Strike — "select a target" message even after clicking enemy (#323)
- [ ] Off-hand damage includes ability modifier (TWF style bonus) — blocked by above bug

## AC Calculation

- **BUG:** AC shows 12 but chain mail is equipped (should be AC 16). Equipment slots have `armor: chain-mail` but AC calculation appears to ignore equipped armor. DEX 14 (+2) means naked AC would be 12 = 10 + 2, which is what's showing.

## Combat Flow (Level 1 Fighter Turn)

1. [x] Turn starts — 1 action, 1 bonus action, 30ft movement
2. [x] Activate Attack (action) — grants 1 strike
3. [x] Strike target — attack roll resolves
4. [x] Off-Hand Strike granted as bonus action after main-hand Strike
5. **BLOCKED:** Cannot complete Off-Hand Strike (#323)
6. [x] Second Wind works — heals HP
7. [x] End turn — temporary actions clear

## Level 1 Bugs Found

| Bug | Layer | Issue | Description |
|-----|-------|-------|-------------|
| Weapon not rendering | Web | #337 | Shortsword/scimitar not visible on model |
| No feature usage counter | Web | #338 | Second Wind doesn't show 1/1 or 0/1 |
| No action type label | Web | #339 | Second Wind not labeled as bonus action |
| Off-Hand Strike broken | Web | #323 | Cannot select target — blocks TWF testing |
| AC ignores equipped armor | API | NEW | Chain mail equipped but AC = 12 (naked AC), should be 16 |
| Fighting Style invisible | Web | — | No UI shows which style is active |
| No LongRest on dungeon start | API | #435 | Second Wind uses not restored — fix in progress |

## Not Applicable at Level 1

These are level 2+ features — do not test:
- Action Surge (level 2)
- Extra Attack (level 5)
- Martial Archetype / Subclass (level 3)
