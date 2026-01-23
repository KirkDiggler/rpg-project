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

### Turn End Cleanup ✅ (PR #568, merged)
Character.Cleanup() removes temporary actions. Character.onActionGranted calls Apply().

### GetTotalSpeed(ctx)
**Problem:** Only base speed from race exists. Condition bonuses (Unarmored Movement) not factored in.

**Impact:** Dash ability uses base speed only. Can be addressed via MovementChain instead.

### Help/Hide/Ready Abilities
**Status:** Not implemented. Can be separate issues when needed.

## Attack Resolution (#505)

`combat.ResolveAttack()` is **fully implemented** in `combat/attack.go`:
- Validates input, looks up combatants from context
- Calculates attack bonus (ability mod + proficiency)
- Fires AttackChain → conditions modify (advantage/disadvantage, critical threshold)
- Rolls d20 with advantage/disadvantage handling
- Determines hit/miss/crit (natural 20/1 rules)
- On hit: rolls damage, fires DamageChain → conditions add bonuses/resistance
- Publishes DamageReceivedEvent

**Remaining #505 work (when needed):**
- Reaction windows (AttackDeclaredEvent for Shield, AttackRolledEvent for Sentinel)
- Attack of Opportunity (MovementStep triggers)

## Gate: Integration Tests Before API Layer ✅ (Passing)

`character/integration_test.go` now demonstrates:

| Scenario | Status | What it proves |
|----------|--------|----------------|
| Action economy flow | ✅ | Attack ability → Strike → OffHandStrike |
| Extra Attack variants | ✅ | Fighter levels 1-20 |
| Two-weapon fighting | ✅ | Light weapon validation, grant flow |
| Event ordering | ✅ | Events published in correct sequence |
| **Raging Barbarian vs Dodging Defender** | ✅ | AttackChain + DamageChain with real conditions |
| **Disadvantage causes miss** | ✅ | DodgingCondition imposes disadvantage |
| **Normal attack (no conditions)** | ✅ | Baseline without modifiers |
| **Critical hit** | ✅ | Natural 20 doubles dice, rage bonus |
| **Turn end cleanup** | ✅ | Temporary actions removed on Cleanup() |

**Still needed before full API integration:**
- Movement with MovementChain (speed bonuses from conditions)
- Full round with multiple characters (turn order)

**The rule:** If it works in the integration test with printed output, it's ready for the API layer.
