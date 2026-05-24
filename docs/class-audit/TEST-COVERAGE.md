# Test Coverage Analysis

> What's tested where, what's missing, and where the confidence gaps are.

---

## Layer 1: rpg-toolkit (Rules Engine)

### Integration Tests — `rulebooks/dnd5e/integration/`

These are the crown jewels — full encounter simulations that test class features end-to-end through the event chain system.

#### `barbarian_encounter_test.go` (BarbarianEncounterSuite)
| Subtest | What It Verifies |
|---------|-----------------|
| `TestRage_ActivationAndDamageBonus` | Rage consumes charge, applies condition, +2 damage in attack |
| `TestRage_ContinuesWhenBarbarianHitsEnemy` | Rage persists when attack lands, no `ConditionRemovedEvent` |
| `TestRage_ContinuesWhenBarbarianTakesDamage` | Rage persists when `DamageReceivedEvent` fires |
| `TestRage_EndsWithNoCombatActivity` | Rage ends with reason `"no_combat_activity"` when idle |
| `TestRage_EndsAfter10Turns` | Rage ends with reason `"duration_expired"` after 10 rounds |
| `TestRage_ResistanceHalvesPhysicalDamage` | 10 slashing → 5 actual damage via `DealDamage` |
| `TestUnarmoredDefense_ACCalculation` | AC = 10 + DEX(+2) + CON(+3) = 15 |
| `TestEncounter_MultiTurnCombat` | Full scenario: activate rage, attack, goblin dies, rage fades |

**Total: 8 test methods × subtests = ~16 subtests**

#### `fighter_encounter_test.go` (FighterEncounterSuite)
| Subtest | What It Verifies |
|---------|-----------------|
| `TestSecondWind_HealsCharacter` | Publishes `HealingReceivedEvent`, verifies 1d10 + level |
| `TestSecondWind_OncePerShortRest` | Second use fails with "no second wind uses remaining" |
| `TestSecondWind_ResetsOnShortRest` | `RestEvent(ShortRest)` restores availability |
| `TestSecondWind_ScalesWithLevel` | Level 5 → modifier = 5 in healing event |
| `TestFightingStyleDefense_AddsACWithArmor` | AC 16 → 17 when `HasArmor: true` |
| `TestFightingStyleDefense_NoBonus_WithoutArmor` | AC unchanged when `HasArmor: false` |
| `TestFightingStyleDueling_AddsDamage` | +2 flat bonus component with rapier (one-handed melee) |
| `TestFightingStyleDueling_NoBonus_TwoHanded` | No bonus with greatsword |
| `TestFightingStyleDueling_NoBonus_DualWielding` | No bonus when wielding two shortswords |
| `TestFightingStyleArchery_AddsAttackBonus` | Attack bonus 5 → 7 for ranged |
| `TestFightingStyleArchery_NoBonus_Melee` | Attack bonus stays 5 for melee |
| `TestFightingStyleGWF_RerollsLowDice` | Dice [1,2] → rerolled to [5,4] |
| `TestFightingStyleGWF_KeepsHighRolls` | Dice [3,5] → unchanged |
| `TestFightingStyleTWF_AddsAbilityModToOffHand` | +3 (STR) bonus added to off-hand component |
| `TestFightingStyleTWF_NoBonus_MainHand` | No TWF bonus for main-hand attack |
| `TestFightingStyleProtection_ImposesDisadvantage` | Disadvantage source added when ally adjacent + shield |

**Total: 16 test methods × subtests = ~32 subtests**

#### `monk_encounter_test.go` (MonkEncounterSuite)
| Subtest | What It Verifies |
|---------|-----------------|
| `TestMartialArts_DEXForUnarmedStrikes` | Ability bonus swapped from 0 (STR) to 3 (DEX) |
| `TestMartialArts_UnarmedDamageScaling` | WeaponDamage updated to "1d4", dice rerolled |
| `TestMartialArts_MonkWeaponWithDEX` | Shortsword keeps 1d6, DEX used for ability bonus |
| `TestUnarmoredDefense_ExpectedAC` | Mock AC = 16 (10 + DEX 3 + WIS 3) |
| `TestKi_InitialPoints` | Level 2 monk has 2 Ki |
| `TestFlurryOfBlows_KiConsumption` | Ki: 2 → 1 after use |
| `TestPatientDefense_KiConsumption` | Ki: 2 → 1 after use |
| `TestStepOfTheWind_KiConsumption` | Ki: 2 → 1 after use |

**Total: 8 test methods × subtests = ~16 subtests**

**⚠ Note:** Ki feature tests only verify resource consumption, not the actual effects (2 strikes for Flurry, Dodge for Patient Defense, Dash/Disengage for Step). These are marked as TODOs in the test file.

#### `rogue_encounter_test.go` (RogueEncounterSuite)
| Subtest | What It Verifies |
|---------|-----------------|
| `TestSneakAttack_WithAdvantage_AddsDamage` | 1d6 sneak attack dice added when `HasAdvantage: true` |
| `TestSneakAttack_WithAllyAdjacent_AddsDamage` | Sneak attack triggers via spatial ally check |
| `TestSneakAttack_NoAdvantageNoAlly_NoDamage` | No extra component when no advantage, ally far away |
| `TestSneakAttack_OncePerTurn` | First attack gets sneak, second doesn't |
| `TestSneakAttack_ResetsOnTurnEnd` | After `TurnEndEvent`, sneak attack available again |
| `TestSneakAttack_RequiresFinesseOrRanged` | STR-based attack doesn't trigger sneak attack |
| `TestSneakAttack_ScalesWithLevel` | Level 3 → 2d6 sneak attack dice |

**Total: 7 test methods × subtests = ~14 subtests**

### Unit Tests — `rulebooks/dnd5e/conditions/`

Each condition has its own `*_test.go` file. Key counts:

| File | Test Count | What's Covered |
|------|-----------|---------------|
| `raging_test.go` | ~12 | Damage chain, resistance, duration, combat tracking |
| `sneak_attack_test.go` | ~10 | Advantage/ally triggers, once per turn, finesse req |
| `martial_arts_test.go` | ~8 | DEX swap, die scaling, monk weapon detection |
| `fighting_style_dueling_test.go` | ~6 | One-handed melee check, dual-wield exclusion |
| `fighting_style_defense_test.go` | ~4 | Armor requirement, AC chain integration |
| `fighting_style_archery_test.go` | ~4 | Ranged-only check |
| `fighting_style_gwf_test.go` | ~6 | Reroll logic, two-handed requirement |
| `fighting_style_twf_test.go` | ~4 | Off-hand only, ability mod addition |
| `fighting_style_protection_test.go` | ~4 | Shield/reaction/adjacency checks |
| `brutal_critical_test.go` | ~4 | Extra dice on crits |
| `improved_critical_test.go` | ~4 | Threshold 19 check |
| `unarmored_defense_test.go` | ~4 | Barbarian/Monk variant formulas |
| `unarmored_movement_test.go` | ~3 | Speed bonus application |
| `dodging_test.go` | ~3 | Disadvantage on incoming attacks |
| `disengaging_test.go` | ~3 | OA prevention |
| `factory_test.go` | ~4 | Condition creation from ref strings |
| `loader_test.go` | ~4 | JSON deserialization |

### Unit Tests — `rulebooks/dnd5e/features/`

| File | Test Count | What's Covered |
|------|-----------|---------------|
| `rage_test.go` | ~10 | Activation, resource consumption, condition apply/remove |
| `second_wind_test.go` | ~8 | Healing, once per rest, reset, level scaling |
| `action_surge_test.go` | ~6 | Action economy grant, once per rest |
| `flurry_of_blows_test.go` | ~6 | Ki cost, strike granting |
| `patient_defense_test.go` | ~6 | Ki cost, dodge application |
| `step_of_the_wind_test.go` | ~6 | Ki cost, dash/disengage choice |
| `deflect_missiles_test.go` | ~6 | Damage reduction, throw-back |
| `loader_test.go` | ~4 | Feature creation from ref strings |

---

## Layer 2: rpg-api (Game Server)

### API Integration Tests — `internal/integration/encounter/`

These use real Redis (via testcontainers), in-process gRPC (bufconn), and proto-generated clients.

#### `barbarian_test.go` (BarbarianIntegrationSuite)
| Test | What It Verifies |
|------|-----------------|
| `TestRage_ActivateFeature_Success` | Full flow: create char → create encounter → start combat → activate rage via gRPC → verify RAGING condition |
| `TestRage_DamageBonus_AppearsInAttackResult` | Rage → attack → damage breakdown includes rage +2 component |
| `TestRage_Resistance_AppliedWhenTakingDamage` | Rage → end turn → monster attacks → check 0.5x multiplier *(may skip if monster misses)* |
| `TestRage_EndsOnTurnEnd_NoCombatActivity` | Rage → end turn without attacking → verify RAGING removed |
| `TestBarbarianUnarmoredDefense_ACCalculation` | Create barbarian → check AC = 10 + DEX mod + CON mod |

#### `monk_test.go` (MonkIntegrationSuite)
| Test | What It Verifies |
|------|-----------------|
| `TestMartialArts_CanAttackInCombat` | Create monk → start combat → attack with monk weapon → verify success |
| `TestMartialArts_DamageUsesMADie` | *(Skipped: needs mocked roller for deterministic verification)* |
| `TestUnarmoredDefense_ACCalculation` | Create monk → check AC = 10 + DEX mod + WIS mod |

#### `character_test.go`
- General character creation flow tests (not class-specific)

#### `smoke_test.go`
- Basic encounter creation/start smoke test

### Handler Tests — `internal/handlers/dnd5e/v1alpha1/`

| File | What's Covered |
|------|---------------|
| `character/handler_test.go` | CreateDraft, UpdateName, UpdateRace, UpdateClass, FinalizeDraft |
| `character/converters_test.go` | Proto ↔ domain conversions for character data |
| `encounter/handler_test.go` | CreateEncounter, StartCombat, Attack, ActivateFeature, EndTurn |
| `encounter/converters_test.go` | Proto ↔ domain conversions for encounter state |

### Orchestrator Tests — `internal/orchestrators/`

| File | What's Covered |
|------|---------------|
| `encounter/orchestrator_test.go` | Full orchestration: turn management, attack resolution, feature activation |
| `encounter/monster_turns_test.go` | Monster AI action selection and execution |
| `encounter/event_publishing_test.go` | Event stream publishing for client updates |
| `character/orchestrator_test.go` | Character CRUD, draft management |

---

## Layer 3: rpg-dnd5e-web (React UI)

### Existing Tests (3 files, all infrastructure-level)

| File | Tests | What's Covered |
|------|-------|---------------|
| `useHexInteraction.test.ts` | ~15 | World-to-hex conversion, click detection, bounds checking |
| `useMovementRange.test.ts` | ~10 | Movement range calculation, path preview |
| `useDungeonMap.test.ts` | ~5 | Dungeon map state management |

### What's NOT Tested (0 tests)

| Component | Lines | Why It Matters |
|-----------|-------|---------------|
| `FeatureActions.tsx` | 203 | Feature activation buttons (Rage, Second Wind, etc.) |
| `FeatureActionButton.tsx` | 194 | Individual activation button with resource display |
| `FeatureBadge.tsx` | 125 | Feature icon/name display |
| `FeaturesPanel.tsx` | 54 | Feature list rendering |
| `ConditionBadge.tsx` | 140 | Active condition display (Raging, etc.) |
| `ConditionsDisplay.tsx` | 32 | Condition list rendering |
| `CombatOverlay.tsx` | 422 | Main combat UI (attack results, damage breakdown) |
| `DamageBreakdown.tsx` | 46 | Damage component rendering (weapon + rage + ability) |
| `DamageSourceBadge.tsx` | 126 | Source badge with proto enum → display name |

**Total untested UI code related to class features/combat: ~1,342 lines**

---

## Coverage Gaps Summary

### Critical Gaps (High Impact)

1. **No Fighter API integration tests** — Second Wind and all Fighting Styles only tested at toolkit level
2. **No Rogue API integration tests** — Sneak Attack only tested at toolkit level
3. **Monk Ki features only test resource consumption** — Flurry of Blows, Patient Defense, Step of the Wind don't test actual effects
4. **Zero UI tests for combat/features** — Feature activation, condition display, damage breakdown all untested
5. **No Expertise mechanic tests** — Rogue's signature skill feature completely untested

### Moderate Gaps (Medium Impact)

6. **Monk Unarmored Defense not wired to ACChain** — Integration tests use mock AC
7. **Reckless Attack not implemented** — Feature enum exists but no toolkit code
8. **API resistance test is non-deterministic** — Skips when monster misses (needs mocked roller)
9. **No API tests for Rage duration expiry** (10 round limit)
10. **No tests for condition persistence** across save/load cycles

### Low Priority Gaps

11. **Brutal Critical integration tests** — Unit tests exist, no encounter-level test
12. **Improved Critical integration tests** — Unit tests exist, no encounter-level test  
13. **Dodging/Disengaging conditions** — Unit tests exist, no integration with combat abilities
14. **Action Surge integration** — Unit test exists, no multi-action combat scenario
