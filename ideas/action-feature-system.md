# Action System: Two-Level Action Economy

## Status: Core Implemented, Gaps Being Filled (#546)

## The Architecture (Resolved)

Combat uses a **two-level action economy**:

### Level 1: Combat Abilities (spend action economy → grant capacity)
- **Attack**: Spends 1 action → grants N attacks (1 + ExtraAttacks)
- **Dash**: Spends 1 action → grants extra movement equal to speed
- **Dodge**: Spends 1 action → applies DodgingCondition
- **Disengage**: Spends 1 action → applies DisengagingCondition

### Level 2: Actions (consume capacity → do things)
- **Strike**: Consumes 1 attack → resolves through AttackChain
- **Move**: Consumes movement → changes position
- **FlurryStrike**: Consumes 1 flurry strike → unarmed bonus attack
- **OffHandStrike**: Consumes 1 off-hand attack → light weapon bonus attack

### Features (activate → spend resources → grant conditions/actions)
- **Rage**: Consumes a use → publishes ConditionAppliedEvent → RagingCondition listens
- **Second Wind**: Consumes a use → rolls healing → publishes HealingReceivedEvent
- **Flurry of Blows**: Consumes Ki → grants FlurryStrike actions

### Conditions (subscribe to bus → passive modifiers)
- **RagingCondition**: Damage chain bonus, advantage on STR saves
- **DodgingCondition**: Disadvantage on incoming attacks, advantage on DEX saves
- **DisengagingCondition**: Prevents attacks of opportunity
- **UnarmoredDefense**: Modifies AC calculation
- **SneakAttack**: Adds dice on eligible attacks

## Key Design Decisions

1. **Abilities consume economy, grant capacity** - Clear two-level split
2. **Actions consume capacity, do work** - Strike uses an attack, Move uses movement
3. **Features are player-triggered** - Cost resources, immediate or grant conditions
4. **Conditions are passive listeners** - Subscribe to chains, modify events
5. **Chains order modifiers** - base → features → conditions → equipment → final
6. **Resources recover via RestTopic** - Subscribe to events, auto-restore

## What's Implemented

### Combat Abilities
| Ability | Implemented | Tested | Notes |
|---------|-------------|--------|-------|
| Attack | ✅ | ✅ | Grants 1+ExtraAttacks |
| Dash | ✅ | ✅ | Grants speed as extra movement |
| Dodge | ✅ | ✅ | Applies DodgingCondition |
| Disengage | ✅ | ✅ | Applies DisengagingCondition |
| Help | ❌ | - | Grants advantage to ally |
| Hide | ❌ | - | Stealth check → Hidden condition |

### Actions
| Action | Implemented | Tested | Notes |
|--------|-------------|--------|-------|
| Strike | ✅ | ✅ | Consumes attack, fires AttackChain |
| Move | ✅ | ✅ | Consumes movement, fires MovementChain |
| FlurryStrike | ✅ | ✅ | Consumes flurry, unarmed attack |
| OffHandStrike | ✅ | ✅ | Consumes off-hand, light weapon |

### Character Methods for Combat
| Method | Implemented | PR |
|--------|-------------|-----|
| GetSpeed() | ✅ | #567 |
| GetExtraAttacksCount() | ✅ | #567 |
| GetTotalSpeed(ctx) | ❌ | - |

### Conditions
| Condition | Implemented | PR |
|-----------|-------------|-----|
| DodgingCondition | ✅ | #566 |
| DisengagingCondition | ✅ | #563 |
| RagingCondition | ✅ | Earlier |
| UnarmoredDefense | ✅ | Earlier |
| SneakAttack | ✅ | Earlier |
| BrutalCritical | ✅ | Earlier |
| MartialArts | ✅ | Earlier |
| UnarmoredMovement | ✅ | Earlier |
| ImprovedCritical | ✅ | Earlier |
| Fighting Styles (6) | ✅ | Earlier |

## Current Gaps (#546)

### Turn End Cleanup (Next Up)
**Problem:** Temporary actions (FlurryStrike, OffHandStrike) don't auto-remove when:
1. Uses are exhausted (`UsesRemaining() == 0`)
2. Turn ends

**Impact:** Actions accumulate on character, state corruption.

**Solution options:**
- Actions self-remove when exhausted (publish `ActionRemovedEvent`)
- Character listens to `TurnEndEvent` and removes temporary actions
- Or both (exhaustion + turn boundary)

### GetTotalSpeed(ctx)
**Problem:** Only base speed from race exists. Condition bonuses (Unarmored Movement) not factored in.

**Impact:** Dash ability uses base speed only. Can be addressed via MovementChain instead.

### Help/Hide/Ready Abilities
**Status:** Not implemented. Can be separate issues when needed.

## What's Next: Attack Resolution (#505)

Once turn cleanup is solid, the next major piece is `combat.ResolveAttack()` - the three-phase attack model with reaction windows (ADR-0027). This makes the combat loop actually resolve hits and damage.

See issue #505 for the full plan.

## Gate: Integration Tests Before API Layer

Before moving any of this up to the game server, we need integration tests in the toolkit that prove the full combat flow end-to-end. Pattern: test suite with printed output showing the sequence of events.

**What the integration test should demonstrate:**
1. Character with features/conditions applied to event bus
2. Turn start → action economy reset
3. Activate Attack ability → grants attacks
4. Strike action → fires through AttackChain → conditions modify (advantage/disadvantage)
5. Resolve hit/miss → damage chain → conditions modify (rage bonus, sneak attack)
6. Movement with MovementChain (speed bonuses from conditions)
7. Turn end → temporary actions removed, conditions expire
8. Full round with multiple characters

Existing: `character/integration_test.go` covers parts of this. Will need expansion once attack resolution lands.

**The rule:** If it works in the integration test with printed output, it's ready for the API layer. If not, we're not done in the toolkit.
