# Rogue QA Checklist (Level 1)

Class-specific testing for the Rogue at level 1.

## Identity

- **Hit Dice**: d8
- **Primary Ability**: Dexterity
- **Armor**: Light only
- **Weapons**: Simple, Hand Crossbow, Longsword, Rapier, Shortsword
- **Level 1 Features**: Sneak Attack (passive condition), Expertise (skill bonus), Thieves' Cant (language)
- **Features Panel**: Empty at level 1 — Sneak Attack is a condition (passive), not an activatable feature. Cunning Action is level 2.

## BLOCKER: Cannot Create Rogue (#343)

Character creation fails with:
```
Failed to finalize character:
[unknown] draft is incomplete - missing: [class selection or class choices].
Class validation: Choose 2 skills or thieves' tools for expertise:
Must choose 2 skills or tools for expertise
```

**Cause:** No expertise selection UI in character creation. Toolkit requires 2 expertise choices but web has no step for this.

**All testing below is blocked until this is fixed.**

---

## Character Creation

- [ ] Rogue class selectable
- [ ] Rogue textures load
- [ ] Weapon renders on model (rapier/shortsword typical)
- [ ] Starting HP reflects d8 hit dice + CON modifier
- [ ] Expertise: choose 2 skills to double proficiency bonus — **NO UI FOR THIS**
- [ ] 4 skill proficiencies selectable

## Sneak Attack (Passive Condition — Level 1)

### Eligibility (must meet ALL of these)
- [ ] Must use a **finesse** (rapier, shortsword, dagger) or **ranged** weapon
- [ ] Must have **advantage** on the attack, OR an **ally within 5ft** of the target
- [ ] Triggers only **once per turn**

### Damage
- [ ] Extra 1d6 damage at level 1
- [ ] Sneak Attack damage shown as separate component in combat log
- [ ] Does NOT trigger if conditions aren't met

## Combat Flow (Level 1 Rogue Turn)

**All blocked — cannot create character.**

## Level 1 Bugs Found

| Bug | Layer | Issue | Description |
|-----|-------|-------|-------------|
| Cannot create Rogue | Web | #343 | No expertise selection UI — blocks all Rogue testing |

## Not Applicable at Level 1

These are level 2+ features — do not test:
- Cunning Action (level 2)
- Roguish Archetype / Subclass (level 3)
- Uncanny Dodge (level 5)
