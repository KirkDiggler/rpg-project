# Recommendations

> Action items for improving test coverage, documentation, and reliability.

---

## P0: Critical — Fix Before Next Playtest

### 1. Add Fighter and Rogue API Integration Tests

**Why:** These classes have solid toolkit tests but zero API integration coverage. If the API wiring breaks, playtesting will catch it — the hard way.

**Scope:**
- `fighter_test.go` in `rpg-api/internal/integration/encounter/`:
  - `TestSecondWind_ActivateFeature_Success` — Activate via gRPC, verify healing
  - `TestFightingStyle_Defense_ACBonus` — Verify AC reflects style
  - `TestFightingStyle_Dueling_DamageBonus` — Verify +2 in breakdown
- `rogue_test.go` in `rpg-api/internal/integration/encounter/`:
  - `TestSneakAttack_WithAdvantage_DamageBreakdown` — Verify sneak dice in breakdown
  - `TestSneakAttack_AllyAdjacent_DamageBreakdown` — Spatial trigger

**Effort:** ~1 day. Pattern is established from barbarian_test.go.

### 2. Wire Monk Ki Features Through to Effects

**Why:** Integration tests for Flurry of Blows, Patient Defense, and Step of the Wind only test Ki consumption — they don't verify the actual effects (extra strikes, Dodge condition, movement changes).

**Scope:**
- In toolkit integration tests, replace `// TODO` comments with real feature activation
- Verify Flurry grants 2 unarmed strike capacity
- Verify Patient Defense applies Dodging condition
- Verify Step of the Wind grants movement capacity

**Effort:** ~2 days. Feature code exists, needs wiring + test completion.

### 3. Implement Reckless Attack

**Why:** Proto enum `FEATURE_ID_RECKLESS_ATTACK` exists. It's a core Barbarian feature that players expect.

**Scope:**
- Implement as a free action feature that sets `HasAdvantage: true` on attack chain
- Add condition that grants advantage on incoming attacks until next turn
- Add toolkit unit + integration tests
- Wire through API

**Effort:** ~2 days following existing patterns.

---

## P1: High — Next Sprint

### 4. Add UI Component Tests for Feature/Combat Display

**Why:** 1,342 lines of untested React code handling the most visible part of the game — combat results and feature activation. When proto enums change, these break silently.

**Scope (recommended framework: Vitest + React Testing Library):**
- `FeatureActionButton.test.tsx` — Renders correct icon/label, handles click, shows resource count
- `ConditionBadge.test.tsx` — Renders correct condition name/icon for each `ConditionId`
- `DamageBreakdown.test.tsx` — Renders components with source labels
- `CombatOverlay.test.tsx` — Shows hit/miss, damage total, breakdown

**Effort:** ~2-3 days. Start with simple rendering tests.

### 5. Make API Integration Tests Deterministic

**Why:** Several tests skip when dice rolls cause misses. This makes CI unreliable.

**Options:**
- Inject a fixed-seed roller via test harness config
- Add `SetDiceOverride` gRPC method for test mode
- Mock at the orchestrator level (easier but less realistic)

**Effort:** ~1 day for the inject approach.

### 6. Wire Monk Unarmored Defense to AC Chain

**Why:** The integration test has a TODO noting this isn't wired. AC is hardcoded in mock.

**Scope:**
- Connect `UnarmoredDefenseCondition` to `combat.ACChain` event
- Update integration tests to use real `EffectiveAC()` calculation
- Verify both Monk (DEX+WIS) and Barbarian (DEX+CON) variants

**Effort:** ~0.5 day. Pattern established by Fighting Style: Defense.

---

## P2: Medium — Backlog

### 7. Add Condition Persistence Tests

**Why:** Conditions are loaded from character data and re-applied each session. No tests verify they survive save/load correctly.

**Scope:**
- Test that `condition.ToJSON()` → `condition.Load()` round-trips correctly
- Verify conditions re-subscribe to event bus after reload
- Test feature `IsDirty()`/`MarkClean()` cycle

### 8. Add Expertise Mechanic

**Why:** The Rogue's Expertise is defined in the choice system but the actual double-proficiency effect on skill checks isn't implemented or tested.

**Scope:**
- Implement expertise as a modifier on skill check events
- Add to Rogue grant system
- Test in character creation and skill check scenarios

### 9. Document Event Chain Architecture

**Why:** The event chain system (attack chain → damage chain → AC chain) is the backbone of the entire combat system, but there's no high-level documentation explaining how conditions hook into it.

**Deliverable:** An architecture doc in `rpg-toolkit/docs/` explaining:
- What chains exist and their stages
- How conditions subscribe at different stages
- The publish → chain → execute flow
- How to add a new condition

### 10. Add Death Saves Integration

**Why:** There's a design doc in `rpg-project/ideas/death-saves/` but the feature needs integration testing across all four classes.

---

## P3: Stretch — Future

### 11. Add Paladin/Cleric Grants

**Why:** `GetGrants()` returns nil for all caster classes. When these classes are added to the game, they need the same grant → condition → feature pipeline.

**Priority order:** Paladin (Divine Smite already has proto enum), then Cleric (healing).

### 12. Multiclass Support

**Why:** Not needed for the dungeon crawler MVP, but the grant system is designed to support it. Would need testing for grant merging from multiple classes.

### 13. UI Storybook for Combat Components

**Why:** With complex combat state (conditions, features, damage breakdowns), having a Storybook with all visual states documented would prevent regressions.

---

## README Proposal

Each repo should have a README that explains its role in the architecture:

### rpg-toolkit/README.md
```
# rpg-toolkit — D&D 5e Rules Engine

This is where the rules live. If Rage gives +2 damage, this package implements that.

## Architecture
- `rulebooks/dnd5e/` — D&D 5e implementation
  - `classes/` — Class data and grant declarations
  - `conditions/` — Passive effects (event chain subscribers)
  - `features/` — Activatable abilities
  - `combat/` — Attack resolution, damage calculation, AC
  - `integration/` — Full encounter tests per class

## The Boundary Rule
The toolkit IMPLEMENTS rules. It doesn't know about gRPC, Redis, or Discord.
API sends toolkit refs → toolkit resolves behavior → API sends results to client.
```

### rpg-api/README.md
```
# rpg-api — Game Server

Orchestrates the game. Maps proto enums to toolkit calls.

## Key Directories
- `internal/handlers/` — gRPC handler implementations
- `internal/orchestrators/` — Business logic (encounter, character)
- `internal/integration/` — Full-stack tests with real Redis

## The Boundary Rule
The API ORCHESTRATES by key. It knows feature IDs, not feature behavior.
```

### rpg-dnd5e-web/README.md
```
# rpg-dnd5e-web — Discord Activity UI

React Three Fiber game client running as a Discord Activity.

## Key Directories
- `src/components/features/` — Feature/condition display
- `src/components/combat/` — Combat overlay, damage breakdown
- `src/components/hex-grid/` — 3D hex grid rendering

## The Boundary Rule
The client sends REFERENCES (proto enums). It never implements game rules.
```
