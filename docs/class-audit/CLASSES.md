# Per-Class Breakdown

> Detailed documentation of each implemented class's features, abilities, and mechanics.

---

## Barbarian

**Hit Die:** d12 | **Primary:** STR | **Saves:** STR, CON  
**Armor:** Light, Medium, Shields | **Weapons:** Simple, Martial

### Level 1 Features

#### Rage (Feature — Activatable)
- **Action Cost:** Bonus Action
- **Resource:** Rage Charges (2 at L1, resets on Long Rest)
- **Duration:** 10 rounds (1 minute), OR ends early if no combat activity
- **Effects when active:**
  - Applies `RagingCondition` → subscribes to damage chain
  - +2 melee damage bonus (scales: +2 at L1-8, +3 at L9-15, +4 at L16+)
  - Resistance to bludgeoning, piercing, slashing damage (half damage)
  - Advantage on STR checks and STR saves (not yet implemented)
- **Auto-end logic:** Tracks `attackedThisTurn` and `tookDamageThisTurn` flags via event subscriptions. If neither flag is set when `TurnEndEvent` fires, rage ends with reason `"no_combat_activity"`. After 10 turns, ends with reason `"duration_expired"`.
- **Implementation:** `rulebooks/dnd5e/features/rage.go` + `conditions/raging.go`
- **Proto:** `FEATURE_ID_RAGE`, `CONDITION_ID_RAGING`

#### Unarmored Defense (Condition — Passive)
- **AC Formula:** 10 + DEX modifier + CON modifier
- **Requirement:** No armor worn (shields OK)
- **Implementation:** `rulebooks/dnd5e/conditions/unarmored_defense.go`
- **Proto:** `CONDITION_ID_UNARMORED_DEFENSE`
- **Note:** Shares implementation with Monk variant (uses config `{"variant": "barbarian"}`)

#### Brutal Critical (Condition — Passive, Higher Levels)
- **Effect:** Roll additional weapon damage dice on critical hits
- **Scaling:** +1 die at L9, +2 at L13, +3 at L17
- **Implementation:** `rulebooks/dnd5e/conditions/brutal_critical.go`
- **Proto:** `CONDITION_ID_BRUTAL_CRITICAL`

#### Reckless Attack (Feature — Not Yet Implemented)
- **Proto:** `FEATURE_ID_RECKLESS_ATTACK`
- **Expected:** Declare before attack → advantage on melee STR attacks this turn, but attacks against you have advantage until next turn

### Barbarian Grant Chain
```
Level 1 Grant:
  Features:   [rage]
  Conditions: [unarmored_defense(barbarian)]
  Armor:      [light, medium, shields]
  Weapons:    [simple, martial]
```

---

## Fighter

**Hit Die:** d10 | **Primary:** STR | **Saves:** STR, CON  
**Armor:** Light, Medium, Heavy, Shields | **Weapons:** Simple, Martial

### Level 1 Features

#### Second Wind (Feature — Activatable)
- **Action Cost:** Bonus Action
- **Resource:** 1 use per Short/Long Rest
- **Effect:** Heal 1d10 + fighter level HP
- **Implementation:** `rulebooks/dnd5e/features/second_wind.go`
- **Proto:** `FEATURE_ID_SECOND_WIND`
- **Events:** Publishes `HealingReceivedEvent` with roll, modifier, and total

#### Fighting Styles (Conditions — Passive)
All fighting styles are implemented as conditions that subscribe to attack/damage/AC event chains:

| Style | Effect | Chain | Test Count |
|-------|--------|-------|-----------|
| **Defense** | +1 AC when wearing armor | AC Chain | 2 tests |
| **Dueling** | +2 damage with one-handed melee (no off-hand weapon) | Damage Chain | 3 tests |
| **Archery** | +2 to ranged attack rolls | Attack Chain | 2 tests |
| **Great Weapon Fighting** | Reroll 1s and 2s on damage dice (two-handed/versatile) | Damage Chain | 2 tests |
| **Two-Weapon Fighting** | Add ability modifier to off-hand damage | Damage Chain | 2 tests |
| **Protection** | Impose disadvantage on attacks vs adjacent ally (requires shield + reaction) | Attack Chain | 1 test |

- **Proto:** `CONDITION_ID_FIGHTING_STYLE_*` (6 enums), `FIGHTING_STYLE_*` enums
- **Edge Cases Tested:**
  - Defense: No bonus without armor
  - Dueling: No bonus with two-handed or dual-wielding
  - Archery: No bonus for melee attacks
  - TWF: No bonus for main-hand attacks
  - Protection: Requires shield, reaction, and adjacent ally

#### Improved Critical (Condition — Passive, Champion Subclass)
- **Effect:** Critical hit on 19-20 (instead of just 20)
- **Implementation:** `rulebooks/dnd5e/conditions/improved_critical.go`
- **Proto:** `CONDITION_ID_IMPROVED_CRITICAL`

#### Action Surge (Feature — Activatable, Level 2+)
- **Action Cost:** Free
- **Effect:** Gain one additional action this turn
- **Resource:** 1 use per Short/Long Rest
- **Implementation:** `rulebooks/dnd5e/features/action_surge.go`
- **Proto:** `FEATURE_ID_ACTION_SURGE`

### Fighter Grant Chain
```
Level 1 Grant:
  Features:   [second_wind]
  Conditions: [] (fighting style is a CHOICE, applied separately)
  Armor:      [light, medium, heavy, shields]
  Weapons:    [simple, martial]
```

---

## Monk

**Hit Die:** d8 | **Primary:** DEX | **Saves:** STR, DEX  
**Armor:** None | **Weapons:** Simple, Shortsword

### Level 1 Features

#### Martial Arts (Condition — Passive)
- **Effects:**
  1. Use DEX instead of STR for unarmed strikes and monk weapons (when DEX > STR)
  2. Unarmed strike damage scales: 1d4 (L1-4), 1d6 (L5-10), 1d8 (L11-16), 1d10 (L17+)
  3. Bonus action unarmed strike after Attack action (Martial Arts Bonus Strike)
- **Monk Weapons:** Shortswords + simple melee weapons without Heavy or Two-Handed
- **Implementation:** `rulebooks/dnd5e/conditions/martial_arts.go`
- **Proto:** `CONDITION_ID_MARTIAL_ARTS`
- **Event Chain:** Subscribes to Damage Chain at modifier stage; swaps ability and rerolls weapon dice

#### Unarmored Defense (Condition — Passive)
- **AC Formula:** 10 + DEX modifier + WIS modifier
- **Requirement:** No armor worn, no shield
- **Implementation:** Same as Barbarian, config `{"variant": "monk"}`
- **Proto:** `CONDITION_ID_UNARMORED_DEFENSE`
- **Known Issue:** Not yet wired to ACChain for dynamic calculation (see Monk integration test TODO)

### Level 2 Features

#### Ki (Resource)
- **Pool:** Ki points = Monk Level (2 at L2, 3 at L3, etc.)
- **Resets on:** Short Rest or Long Rest
- **Proto resource key:** `resources.Ki`

#### Flurry of Blows (Feature — Activatable)
- **Action Cost:** Bonus Action (1 Ki)
- **Effect:** 2 unarmed strikes as bonus action (instead of 1 from Martial Arts)
- **Implementation:** `rulebooks/dnd5e/features/flurry_of_blows.go`
- **Proto:** `FEATURE_ID_FLURRY_OF_BLOWS`, `COMBAT_ABILITY_ID_FLURRY_OF_BLOWS`
- **Status:** Ki consumption tested, actual strike granting partially implemented

#### Patient Defense (Feature — Activatable)
- **Action Cost:** Bonus Action (1 Ki)
- **Effect:** Take Dodge action as bonus action (attacks against you have disadvantage)
- **Implementation:** `rulebooks/dnd5e/features/patient_defense.go`
- **Proto:** `FEATURE_ID_PATIENT_DEFENSE`
- **Status:** Ki consumption tested, Dodge condition not yet wired

#### Step of the Wind (Feature — Activatable)
- **Action Cost:** Bonus Action (1 Ki)
- **Effect:** Dash or Disengage as bonus action + double jump distance
- **Implementation:** `rulebooks/dnd5e/features/step_of_the_wind.go`
- **Proto:** `FEATURE_ID_STEP_OF_THE_WIND`
- **Status:** Ki consumption tested, Dash/Disengage not yet wired

#### Deflect Missiles (Feature — Activatable, Level 3+)
- **Action Cost:** Reaction
- **Effect:** Reduce ranged attack damage by 1d10 + DEX + Monk Level; if reduced to 0, can throw back for 1 Ki
- **Implementation:** `rulebooks/dnd5e/features/deflect_missiles.go`
- **Proto:** `FEATURE_ID_DEFLECT_MISSILES`

#### Unarmored Movement (Condition — Passive, Level 2+)
- **Effect:** +10 ft speed (scales at higher levels)
- **Implementation:** `rulebooks/dnd5e/conditions/unarmored_movement.go`
- **Proto:** `CONDITION_ID_UNARMORED_MOVEMENT`

### Monk Grant Chain
```
Level 1 Grant:
  Features:   []
  Conditions: [unarmored_defense(monk), martial_arts(level=1)]
  Armor:      [] (none)
  Weapons:    [simple, shortsword]
```

---

## Rogue

**Hit Die:** d8 | **Primary:** DEX | **Saves:** DEX, INT  
**Armor:** Light | **Weapons:** Simple, Hand Crossbow, Longsword, Rapier, Shortsword  
**Tools:** Thieves' Tools | **Languages:** Thieves' Cant

### Level 1 Features

#### Sneak Attack (Condition — Passive)
- **Effect:** Extra damage dice on one attack per turn
- **Requirements (any one):**
  1. Attack with advantage, OR
  2. Ally within 5 ft of target (and no disadvantage on attack)
- **Weapon Requirement:** Must use finesse (DEX-based) or ranged weapon
- **Damage Scaling:** ceil(Rogue Level / 2)d6 — 1d6 at L1, 2d6 at L3, 3d6 at L5, etc.
- **Once per turn:** Tracks `usedThisTurn` flag, resets on `TurnEndEvent`
- **Implementation:** `rulebooks/dnd5e/conditions/sneak_attack.go`
- **Proto:** `CONDITION_ID_SNEAK_ATTACK`, `FEATURE_ID_SNEAK_ATTACK`
- **Spatial Awareness:** Uses `gamectx.WithRoom` to check ally adjacency to target

#### Expertise (Choice — Not Yet Implemented as Mechanic)
- **Effect:** Double proficiency bonus on 2 chosen skills
- **Status:** Choice system exists in character creation, but expertise effect on skill checks not tested in combat
- **Proto:** Listed under `ValidationField_EXPERTISE`

#### Thieves' Cant (Language)
- **Effect:** Secret language shared among rogues
- **Granted via:** `getRogueGrants()` includes `languages.ThievesCant`

### Rogue Grant Chain
```
Level 1 Grant:
  Features:   []
  Conditions: [sneak_attack(level=1)]
  Armor:      [light]
  Weapons:    [simple, hand_crossbow, longsword, rapier, shortsword]
  Tools:      [thieves_tools]
  Languages:  [thieves_cant]
```

---

## Conditions Catalog

All conditions implemented in `rulebooks/dnd5e/conditions/`:

| Condition | Type | Subscribes To | Proto Enum |
|-----------|------|---------------|-----------|
| Raging | Combat modifier | Damage Chain, Turn End, Damage Received | `CONDITION_ID_RAGING` |
| Brutal Critical | Combat modifier | Damage Chain (critical hits) | `CONDITION_ID_BRUTAL_CRITICAL` |
| Sneak Attack | Combat modifier | Damage Chain | `CONDITION_ID_SNEAK_ATTACK` |
| Martial Arts | Combat modifier | Damage Chain | `CONDITION_ID_MARTIAL_ARTS` |
| Unarmored Defense | AC modifier | AC Chain | `CONDITION_ID_UNARMORED_DEFENSE` |
| Unarmored Movement | Speed modifier | Movement | `CONDITION_ID_UNARMORED_MOVEMENT` |
| Improved Critical | Attack modifier | Attack Chain | `CONDITION_ID_IMPROVED_CRITICAL` |
| FS: Defense | AC modifier | AC Chain | `CONDITION_ID_FIGHTING_STYLE_DEFENSE` |
| FS: Dueling | Damage modifier | Damage Chain | `CONDITION_ID_FIGHTING_STYLE_DUELING` |
| FS: Archery | Attack modifier | Attack Chain | `CONDITION_ID_FIGHTING_STYLE_ARCHERY` |
| FS: GWF | Damage modifier | Damage Chain | `CONDITION_ID_FIGHTING_STYLE_GREAT_WEAPON_FIGHTING` |
| FS: TWF | Damage modifier | Damage Chain | `CONDITION_ID_FIGHTING_STYLE_TWO_WEAPON_FIGHTING` |
| FS: Protection | Attack modifier | Attack Chain | `CONDITION_ID_FIGHTING_STYLE_PROTECTION` |
| Dodging | Combat state | Attack Chain (incoming) | *(no proto enum yet)* |
| Disengaging | Movement state | Opportunity Attack check | *(no proto enum yet)* |

## Features Catalog

All features implemented in `rulebooks/dnd5e/features/`:

| Feature | Class | Action Cost | Resource | Proto Enum |
|---------|-------|-------------|----------|-----------|
| Rage | Barbarian | Bonus Action | Rage Charges | `FEATURE_ID_RAGE` |
| Second Wind | Fighter | Bonus Action | 1/Short Rest | `FEATURE_ID_SECOND_WIND` |
| Action Surge | Fighter | Free | 1/Short Rest | `FEATURE_ID_ACTION_SURGE` |
| Flurry of Blows | Monk | Bonus Action | 1 Ki | `FEATURE_ID_FLURRY_OF_BLOWS` |
| Patient Defense | Monk | Bonus Action | 1 Ki | `FEATURE_ID_PATIENT_DEFENSE` |
| Step of the Wind | Monk | Bonus Action | 1 Ki | `FEATURE_ID_STEP_OF_THE_WIND` |
| Deflect Missiles | Monk | Reaction | Free (throw=1 Ki) | `FEATURE_ID_DEFLECT_MISSILES` |
