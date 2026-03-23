# Monk QA Checklist (Level 1)

Class-specific testing for the Monk at level 1.

## Identity

- **Hit Dice**: d8
- **Primary Ability**: Dexterity + Wisdom
- **Armor**: None
- **Weapons**: Simple, Shortsword
- **Level 1 Features**: Martial Arts (condition), Unarmored Defense (condition)
- **Features Panel**: Empty at level 1 — no activatable features until level 2 (Ki, Flurry of Blows)

## Character Creation

- [x] Monk class selectable
- [x] Monk textures load
- [-] Weapon renders on model (or unarmed appearance) — **BUG: shortsword not rendering**
- [x] Starting HP reflects d8 hit dice + CON modifier

## Martial Arts (Passive Condition — Level 1)

### Monk Weapon Attacks (shortsword)
- [ ] Shortsword attack uses **DEX** modifier (not STR) for attack and damage
- [ ] Proficiency bonus applies to shortsword attack roll
- [ ] Damage is 1d6 + DEX mod (shortsword die, not Martial Arts die)

### Unarmed Strikes
- [ ] Unarmed strike is available as a Strike option — **BUG: no unarmed strike action available**
- [ ] Unarmed strikes use **DEX** modifier (not STR) for attack and damage
- [ ] Martial Arts die applies: 1d4 + DEX mod damage
- [ ] When no weapon equipped, attacking should use unarmed strike — **BUG: falls back to Greataxe**

### Martial Arts Bonus Strike (free bonus action after Attack)
- [ ] After attacking with monk weapon OR unarmed, Monk gets a bonus action unarmed strike (no Ki cost)
- [ ] This is "Martial Arts Bonus Strike", NOT "Off-Hand Strike"
- **BUG: Two-weapon fighting check runs first and grants Off-Hand Strike instead of Martial Arts Bonus Strike**
- **BUG: Off-Hand Strike "select a target" message appears even after clicking an enemy**

## Unarmored Defense (Passive Condition — Level 1)

- [x] AC = 10 + DEX mod + WIS mod (when wearing no armor)
- [x] AC displayed correctly in character panel
- [ ] Does NOT stack with worn armor — unable to verify (starting equipment has no armor option)

## Combat Flow (Level 1 Monk Turn)

1. [ ] Turn starts — 1 action, 1 bonus action, 30ft movement (no Unarmored Movement bonus at L1)
2. [ ] Move to target
3. [ ] Activate Attack (action) — grants 1 strike
4. [ ] Strike with shortsword — uses DEX, 1d6 + DEX damage
5. [ ] Martial Arts Bonus Strike granted (bonus action) — unarmed, 1d4 + DEX damage
6. [ ] End turn — temporary actions clear

**Alternative flow (unarmed only):**
1. [ ] Activate Attack (action) — grants 1 strike
2. [ ] Strike unarmed — uses DEX, 1d4 + DEX damage
3. [ ] Martial Arts Bonus Strike granted (bonus action) — unarmed, 1d4 + DEX damage

## Level 1 Bugs Found

| Bug | Layer | Description |
|-----|-------|-------------|
| Off-Hand Strike instead of Martial Arts Bonus | API | TWF check runs before Martial Arts check in orchestrator; Monk with shortsword gets wrong bonus action |
| No unarmed strike option | API/Web | No way to choose unarmed strike as a standard Strike action |
| Greataxe fallback | API | When no weapon found, API defaults to Greataxe instead of unarmed strike |
| Off-Hand Strike target selection broken | Web | "Select a target" message even after clicking enemy |
| Shortsword not rendering | Web | Equipped shortsword doesn't appear on 3D character model |

## Not Applicable at Level 1

These are level 2+ features — do not test:
- Ki Points (level 2)
- Flurry of Blows (level 2)
- Patient Defense (level 2)
- Step of the Wind (level 2)
- Unarmored Movement (level 2)
- Deflect Missiles (level 3)
