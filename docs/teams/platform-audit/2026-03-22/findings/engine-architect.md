# Engine Architect Audit — rpg-toolkit

**Date**: 2026-03-22
**Domain**: rpg-toolkit (rules engine)
**Auditor**: Engine Architect specialist

---

## Architecture Map

### Event Bus Architecture (Core Foundation)

**Structure:**
- `events.EventBus` interface: Single pub/sub mechanism using topics
- `simpleEventBus`: In-memory implementation with sync.RWMutex protecting `map[Topic][]subscription`
- `StagedChain[T]`: Ordered pipeline for modifiers with 5 explicit stages
- Topic-to-Bus connection pattern: `topic.On(bus)` for type-safe subscriptions

**Chain Stages (dnd5e/combat/stages.go):**
```
StageBase       (dice rolls, proficiency, ability mods)
StageFeatures   (rage +2, sneak attack dice, extra attack)
StageConditions (bless/bane, prone, restrained)
StageEquipment  (magic weapon bonuses)
StageFinal      (resistance/vulnerability, damage caps)
```

**Chain Types Identified:**

| Chain | Publisher | Subscribers | Use Case |
|-------|-----------|-------------|----------|
| `AttackChain` | ResolveAttack (attack.go:209) | DodgingCondition, RecklessAttack | Attack roll modifiers (+adv/-disadv) |
| `DamageChain` | ResolveDamage (damage.go:285) | RagingCondition, SneakAttack, FightingStyles | Damage calculation & resistance |
| `ACChain` | (Defined but usage unclear) | UnarmoredDefense (partial - has TODO) | Armor class calculation |
| `MovementChain` | MoveEntity (movement_test.go:173) | UnarmoredMovement | Movement penalties |
| `SavingThrowChain` | (Assumed, needs verification) | DodgingCondition, Barbarian | Saving throw modifiers |
| `DamageReceivedTopic` | (TypedTopic, not chained) | RagingCondition.onDamageReceived | Track damage for rage continuation |
| `TurnStartTopic` | TurnManager.StartTurn | DodgingCondition, conditions with auto-cleanup | Per-turn initialization |
| `TurnEndTopic` | TurnManager.EndTurn | RagingCondition, action economy reset | Per-turn cleanup |
| `ConditionAppliedTopic` | Features/abilities | RagingCondition (checks for unconscious) | Cross-condition logic |
| `RestTopic` | (Assumed, on encounter rest) | RagingCondition, RecoverableResource | Resource recovery |

### Subscription Model

**Two-Tier Subscription:**

1. **TypedTopic** (notification-style)
   ```go
   topic.Subscribe(ctx, func(event EventType) error { ... })
   topic.Publish(ctx, event)
   ```
   Used for: Damage received, turn start/end, conditions applied, rest

2. **ChainedTopic** (pipeline-style)
   ```go
   topic.SubscribeWithChain(ctx, func(event EventType, chain Chain[EventType]) (Chain, error) {
       chain.Add(stage, id, modifier)
       return chain, nil
   })
   ```
   Used for: Attack chain, damage chain, AC chain, movement chain, saving throws

**Cleanup Mechanism:**
- Each subscription gets a unique ID: `"{topic}-{nextID}"`
- Stored in `idToTopic` map for reverse lookup during `Unsubscribe()`
- Conditions track subscription IDs in `subscriptionIDs []string`
- Called during condition `Remove()` -> unsubscribes from all events

### Condition Subscription Pattern

Conditions implement 4-method interface:
1. `IsApplied()` - check if subscribed (tracks via `bus != nil`)
2. `Apply(ctx, bus)` - subscribe to events relevant to this condition
3. `Remove(ctx, bus)` - unsubscribe from all tracked IDs
4. `ToJSON()` - persist state for save/load

---

## Strengths

### 1. Clean Separation of Concerns
- Event bus is abstract and game-agnostic
- Conditions don't know about other conditions
- Each condition owns its own subscription lifecycle
- Damage/Attack/Movement each have isolated chains

### 2. Explicit Stage Ordering
- 5-stage pipeline defined once, used everywhere
- `ModifierStages` array enforces order: Features -> Conditions -> Equipment
- Prevents order-dependent bugs

### 3. Type-Safe Event Handling
- Topics are typed (`TypedTopic[DamageChainEvent]`)
- Handlers are type-checked at compile time
- No magic strings, no `interface{}`-based handler registration

### 4. Comprehensive Integration Tests
- `integration_test.go` covers full combat scenarios
- Tests prove Raging Barbarian vs Dodging Defender works end-to-end

### 5. TurnManager Provides Query API
- `GetAvailableAbilities()` and `GetAvailableActions()` exist and work
- Query respects current action economy state
- Returns both `CanUse` and `Reason` for UI to display

---

## Tech Debt

### CRITICAL — Blocks Evolution

#### 1. Race Condition: Multiple Conditions Modifying Same Chain Stage
**Severity:** BLOCKS-EVOLUTION
**Problem:** When two conditions both add modifiers to the same stage, there's no ordering guarantee between them.
**Evidence:** `StagedChain.Execute()` (events/chain.go:99-120) iterates through modifiers sequentially, but subscription order depends on timing of `Apply()` calls.
**Impact:** With 4+ conditions modifying same stage, damage breakdowns become order-dependent.

#### 2. Condition Persistence Broken on Reload
**Severity:** BLOCKS-EVOLUTION
**Problem:** Conditions use `json.RawMessage` for serialization but don't fully support round-trip deserialization.
**Evidence:** RagingData has JSON marshaling (raging.go:131-143), loadJSON() loads the JSON (raging.go:146-161), BUT no loader that routes from JSON back to RagingCondition on character load. Character.LoadFromJSON() doesn't re-subscribe conditions to bus.
**Impact:** Save character with Raging condition, load it -> condition data exists but not subscribed to event bus. Damage bonus won't apply.

#### 3. Subscription Cleanup Silent Failures
**Severity:** BLOCKS-EVOLUTION
**Problem:** `Unsubscribe()` calls return errors but conditions in Remove() don't propagate or log them properly.
**Evidence:** RagingCondition.Remove() (raging.go:112-128) returns on FIRST error but silently succeeds if ID already gone.

### HIGH — Slows Development

#### 4. GetAbilityInfos/GetActionInfos Are Class-Agnostic
**Severity:** SLOWS-DEVELOPMENT
**Problem:** Character.GetAbilityInfos() returns ALL combatAbilities from static slice populated at Finalize. No dynamic filtering by class.

#### 5. TurnManager Not Exposed via API
**Severity:** SLOWS-DEVELOPMENT
**Problem:** TurnManager exists and works but rpg-api doesn't use it.
**Evidence:** rpg-api-protos #129, rpg-api #404 created but incomplete.

#### 6. ACChain Partially Implemented
**Severity:** SLOWS-DEVELOPMENT
**Problem:** ACChain defined (ac.go:54) but only one condition subscribes (UnarmoredDefense), and that subscription is commented as TODO.

#### 7. No Reaction System
**Severity:** SLOWS-DEVELOPMENT
**Problem:** ActionEconomy tracks `ReactionsRemaining` but no event bus events exist to trigger reactions.

### MEDIUM — Cosmetic

#### 8. Two Context Types With Confusing Names
#### 9. DamageChainEvent and AttackChainEvent Not Symmetric (value vs pointer)
#### 10. Feature Refs vs Condition Refs Config field inconsistency

---

## Evolution Risks

### Adding Spell Concentration — HIGH RISK
- ConcentrationCondition exists but no mutual exclusion enforcement
- No way to enforce "only one concentration active"
- Damage interruption window has race condition potential

### Adding Multiclass — HIGH RISK
- Grant system designed for multiple sources but resource system assumes single class
- GetGrants() only checks single classID
- Two classes granting same condition would break deduplication

### Adding Reactions — CRITICAL RISK
- Event bus is turn-based (no interrupt mechanism)
- Reactions need nested event scopes within attack/damage resolution
- No subscription model for "opportunity windows"

---

## Recommendations (Ordered by Impact)

1. **P0: Unblock Spell Concentration** — Add character.CanConcentrate(), mutual exclusion check (1 day)
2. **P1: Wire TurnManager Through API** — Proto messages + API integration (2 days)
3. **P2: Fix Condition Persistence** — Complete loader, round-trip test (1 day)
4. **P3: Define Reaction Event Model** — AttackDeclaredEvent, AttackRolledEvent, ReactionHandler interface (3 days)
5. **P4: Add Stage Ordering within Stages** — Priority field on modifier registration (1 day)
6. **P5: Complete UnarmoredDefense ACChain** — Wire Apply() to ACChain (0.5 day)
7. **P6: Extract Class-Specific Ability Filtering** — Character.GetAbilitiesForClass() (0.5 day)
