# UI Testing Plan — Bottom-Up

> Start at the toolkit, fix the foundation, then work up through API → UI.
> Each layer should be solid before the next one relies on it.

---

## The Problem

The audit found a pattern: **every layer trusts the one below it, but nobody verified that trust.**

- Toolkit: ~300+ tests, but Monk Ki features only test resource consumption — not actual effects
- API: Barbarian + Monk have integration tests; **Fighter & Rogue have zero**
- UI: **Zero component tests.** 4,000+ lines of combat/feature rendering untested.

When something breaks at the toolkit level (e.g., Flurry of Blows doesn't actually grant strikes), it cascades silently through API and UI. The player sees a button that does nothing. Nobody knows until playtime.

---

## Phase 1: Toolkit — Complete the Monk Effects

**Goal:** Every Ki feature tests its *actual effect*, not just Ki consumption.

### Current State (Monk Integration Tests)

| Test | Verifies Ki Cost | Verifies Effect |
|------|:---:|:---:|
| Flurry of Blows | ✅ | ❌ "not yet tested" |
| Patient Defense | ✅ | ❌ "not yet tested" |
| Step of the Wind | ✅ | ❌ "not yet tested" |
| Martial Arts DEX | — | ✅ |
| Martial Arts Scaling | — | ✅ |
| Monk Weapon DEX | — | ✅ |
| Unarmored Defense | — | ⚠️ Uses mock AC, TODO for real ACChain |

### Work Items

1. **Flurry of Blows → Verify 2 strikes granted**
   - Feature code *exists* — publishes `ActionGrantedEvent` with 2 `FlurryStrike` actions
   - Integration test needs to: activate feature, verify 2 actions appear, execute them
   - Pattern: subscribe to `ActionGrantedTopic`, count events
   - **~0.5 day**

2. **Patient Defense → Verify Dodge condition applied**
   - Feature publishes `PatientDefenseActivatedEvent`
   - Integration test needs to: activate, verify Dodging condition or disadvantage on incoming attacks
   - **~0.5 day**

3. **Step of the Wind → Verify Dash/Disengage effects**
   - Feature publishes `StepOfTheWindActivatedEvent` with action choice
   - Integration test: activate with "dash", verify movement doubles; activate with "disengage", verify no opportunity attacks
   - **~0.5 day**

4. **Unarmored Defense → Wire to ACChain**
   - `UnarmoredDefenseCondition` exists but isn't connected to the AC calculation chain
   - Integration test: apply condition, call `EffectiveAC()`, verify 10 + DEX + WIS
   - This is referenced in the audit as P1 #6
   - **~0.5 day**

5. **Unarmored Movement → Verify speed bonus**
   - Condition exists, needs integration test verifying actual movement capacity changes
   - **~0.25 day**

**Phase 1 Total: ~2-3 days**

---

## Phase 2: Toolkit — Fighter & Rogue Parity

**Goal:** All four classes have the same integration test coverage.

### Current State

| Class | Toolkit Unit Tests | Toolkit Integration | API Integration |
|-------|:---:|:---:|:---:|
| Barbarian | ✅ | ✅ | ✅ |
| Fighter | ✅ | ✅ | ❌ |
| Monk | ✅ | ⚠️ partial | ✅ (3 tests) |
| Rogue | ✅ | ✅ | ❌ |

Fighter and Rogue toolkit integration tests look complete. The gap is at the API layer (Phase 3).

### Work Items

1. **Reckless Attack implementation** (Barbarian)
   - Proto enum exists, feature not implemented
   - Free action, sets advantage on attacks + grants advantage to attackers
   - **~2 days** (feature code + condition + tests)

**Phase 2 Total: ~2 days**

---

## Phase 3: API — Wire Missing Integration Tests

**Goal:** Every class feature works end-to-end through gRPC.

### Current State

- `barbarian_test.go`: ✅ Rage activation, damage bonus, resistance
- `monk_test.go`: ⚠️ 3 tests (attack, MA die, AC) — Ki features not tested at API level
- `fighter_test.go`: ❌ Does not exist
- `rogue_test.go`: ❌ Does not exist

### Work Items

1. **Fighter API integration tests**
   - `TestSecondWind_ActivateFeature_Healing`
   - `TestFightingStyle_Defense_ACBonus`
   - `TestFightingStyle_Dueling_DamageBonus`
   - `TestActionSurge_GrantsExtraAction`
   - Pattern established by `barbarian_test.go`
   - **~1 day**

2. **Rogue API integration tests**
   - `TestSneakAttack_WithAdvantage_DamageBreakdown`
   - `TestSneakAttack_AllyAdjacent_DamageBreakdown`
   - **~1 day**

3. **Monk API Ki feature tests**
   - `TestFlurryOfBlows_GrantsStrikes`
   - `TestPatientDefense_GrantsDodge`
   - `TestStepOfTheWind_GrantsDash`
   - Depends on Phase 1 (toolkit effects actually working)
   - **~1 day**

4. **Fix non-deterministic tests**
   - Several tests skip when dice miss (especially `TestMartialArts_DamageUsesMADie`)
   - Options: fixed-seed roller in test harness, or `SetDiceOverride` gRPC method
   - **~0.5 day**

**Phase 3 Total: ~3-4 days**

---

## Phase 4: UI — Foundation (Pure Logic)

**Goal:** Test all the TypeScript utility functions that parse/transform data from protos. No React needed.

### What Exists

- 3 test files, all hex math (pure logic, no React)
- Vitest + jsdom configured
- No `@testing-library/react` installed

### Work Items

1. **`conditionData.test.ts`** — Type guards, parsers, Monk scaling functions
   - `parseConditionData()` with valid/invalid/empty bytes
   - `isRagingData()`, `isMartialArtsData()`, etc. — all type guards
   - `getMartialArtsDie()` — level 1→1d4, 5→1d6, 11→1d8, 17→1d10
   - `getUnarmoredMovementBonus()` — level scaling
   - `conditionIdToString()` — enum mapping
   - **~0.5 day**

2. **`featureData.test.ts`** — Parse feature JSON, type guards, usage tracking
   - `parseFeatureData()` with rage data, second wind data, empty
   - `hasUsageData()` — presence check
   - `isRageData()`, `isSecondWindData()`, `isActionSurgeData()`
   - **~0.25 day**

3. **`featureConditionMapping.test.ts`** — Feature↔Condition mapping
   - `isFeatureActiveByCondition(FeatureId.RAGE, conditions)` → true/false
   - `getActiveConditionIds()` — set construction
   - **~0.25 day**

4. **`featureIcons.test.ts`** — Icon and category lookups
   - Verify every ConditionId maps to an icon
   - Verify source string parsing → correct category
   - **~0.25 day**

**Phase 4 Total: ~1-1.5 days**

---

## Phase 5: UI — Component Tests

**Goal:** Test React components in isolation with mock proto data.

### Setup Required

```bash
npm install -D @testing-library/react @testing-library/jest-dom @testing-library/user-event
```

### Architecture Principle: Props-Driven Components

The existing components are already reasonably modular — they take typed props and render. The main testability issue is that some large components (CombatOverlay at 422 lines, CombatHistorySidebar at 479 lines) mix too many concerns.

**Refactoring rule:** If a component does data transformation AND rendering, split it:
- **Hook**: transforms raw proto data → view model
- **Component**: renders view model → DOM

This is already the pattern for `usePlayerTurn`. Extend it.

### Work Items (ordered by risk — most fragile/visible first)

1. **`ConditionBadge.test.tsx`** — Template for all component tests
   - Renders correct icon for each condition type
   - Renders subtitle from parsed data ("+2" for Raging, "1d4" for Martial Arts)
   - Renders correct category class (class/fighting-style/debuff/racial)
   - Tooltip includes rich data
   - **~0.5 day** (includes establishing test patterns and fixtures)

2. **`FeatureActionButton.test.tsx`**
   - Renders feature name and icon
   - Shows usage count "2/3" when feature has usage data
   - Disabled when action economy doesn't allow it
   - Active state when corresponding condition exists
   - Calls onActivate with correct FeatureId
   - **~0.5 day**

3. **`CombatAbilitiesPanel.test.tsx`**
   - Renders all base abilities (Attack, Dash, Dodge, etc.)
   - Disables abilities when action/bonus action used
   - Shows available strikes after Attack activation
   - Respects API-provided availability overrides
   - **~0.5 day**

4. **`FeaturesListPanel.test.tsx`**
   - Separates activatable vs passive features
   - Shows action type labels with correct colors
   - Active indicator for features with active conditions
   - Empty state when no features
   - **~0.25 day**

5. **`FeatureActions.test.tsx`**
   - Groups features by action type (bonus first, then action, free, reaction)
   - Passes correct active state from conditions
   - Read-only mode for unspecified action types
   - **~0.25 day**

6. **`DamageBreakdown.test.tsx`** + **`DamageSourceBadge.test.tsx`**
   - Renders damage components with correct source labels
   - Correct colors per damage type
   - **~0.25 day**

**Phase 5 Total: ~2-3 days**

---

## Phase 6: UI — Refactor Monoliths (Optional/Ongoing)

**Goal:** Break large components into testable pieces.

### Candidates

| Component | Lines | Issue |
|-----------|:---:|-------|
| `CombatHistorySidebar` | 479 | Mixes event parsing + rendering + scrolling logic |
| `CombatOverlay` | 422 | Mixes state management + animation + rendering |
| `ActionPanelV2` | 356 | Mixes hook usage + layout + action dispatch |

### Pattern

```
Before: BigComponent.tsx (400 lines, untestable)

After:
  useBigComponentLogic.ts     → tested with renderHook()
  BigComponentView.tsx         → tested with RTL, receives view model
  BigComponent.tsx             → thin wrapper connecting hook to view
```

**Phase 6: Ongoing, as components get touched**

---

## Dependency Chain

```
Phase 1 (Toolkit: Monk effects)
  ↓
Phase 2 (Toolkit: Reckless Attack)    Phase 4 (UI: Pure logic tests)
  ↓                                      ↓
Phase 3 (API: Integration tests)     Phase 5 (UI: Component tests)
                                         ↓
                                      Phase 6 (UI: Refactor monoliths)
```

Phases 1-3 and 4-5 can run in **parallel** — toolkit/API work doesn't block UI pure logic tests.

---

## Test Fixture Strategy

### Proto Fixtures

Create a shared `src/__fixtures__/` directory with factory functions:

```typescript
// src/__fixtures__/characters.ts
export function createMonkCharacter(overrides?: Partial<Character>): Character
export function createBarbarianCharacter(overrides?: Partial<Character>): Character

// src/__fixtures__/conditions.ts
export function createRagingCondition(overrides?: Partial<Condition>): Condition
export function createMartialArtsCondition(monkLevel?: number): Condition

// src/__fixtures__/features.ts
export function createRageFeature(uses?: number, maxUses?: number): CharacterFeature
export function createFlurryOfBlowsFeature(): CharacterFeature
```

This avoids every test file constructing its own proto objects and keeps fixture data consistent with what the API actually sends.

---

## Success Criteria

| Phase | "Done" means |
|-------|-------------|
| 1 | All Monk Ki features test their actual effects, not just resource consumption |
| 2 | Reckless Attack implemented with full test coverage |
| 3 | All 4 classes have API integration tests; no non-deterministic skips |
| 4 | Every pure utility function in the UI has a test file |
| 5 | Every combat/feature component renders correctly for all 4 class archetypes |
| 6 | No component over 200 lines without extracted hooks/sub-components |

---

## Issue Tracking Suggestion

One GitHub issue per phase, with sub-tasks as checkboxes. Keeps PRs focused:

- `test/monk-toolkit-effects` (Phase 1)
- `feat/reckless-attack` (Phase 2)
- `test/api-integration-all-classes` (Phase 3)
- `test/ui-pure-logic` (Phase 4)
- `test/ui-component-tests` (Phase 5)
