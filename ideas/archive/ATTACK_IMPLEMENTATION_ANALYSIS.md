# Character Attack Implementation Analysis

## Executive Summary

After examining the current architecture, I've identified that while the event bus infrastructure is well-designed to support character attacks with conditions and modifiers, the actual attack implementation in the Character struct is just a placeholder. The good news is that all the foundational pieces are in place - we just need to connect them properly.

## Current State Analysis

### 1. Character Implementation Status

The `Character` struct in `rpg-toolkit/rulebooks/dnd5e/character/character.go` has:

**What EXISTS:**
- Basic character structure with all necessary fields
- Placeholder `Attack()` method that returns empty results
- Conditions and effects storage mechanisms
- Equipment tracking
- Ability scores and proficiency bonus calculation
- `LoadCharacterFromContext()` pattern already implemented (lines 414-429)

**What's MISSING:**
- Actual attack roll calculation logic
- Integration with the dice roller
- Event bus integration for attack modifiers
- Weapon proficiency checking
- Attack bonus calculation (STR/DEX + proficiency)
- Critical hit detection

### 2. Event Bus Architecture

The event bus is FULLY READY for attack implementation:

**Strengths:**
- Complete event types for attack sequence (`EventOnAttackRoll`, `EventOnDamageRoll`, etc.)
- Modifier system supporting both flat bonuses and dice rolls
- Priority-based handler execution
- Context system for passing attack metadata
- Example implementations showing Bless spell integration

**Key Event Flow:**
```
1. EventBeforeAttackRoll → Prepare attack
2. EventOnAttackRoll → Apply modifiers (proficiency, bless, etc.)
3. EventAfterAttackRoll → Determine hit/miss
4. EventOnDamageRoll → Calculate damage with modifiers
5. EventBeforeTakeDamage → Apply resistances/vulnerabilities
```

### 3. LoadFromContext Pattern Analysis

The spatial tool demonstrates a mature implementation:

**Pattern Benefits:**
- Clean separation between data and behavior
- Event bus integration from initialization
- Self-contained data structures
- Consistent loading interface

**Character Already Has This!**
The character already implements `LoadCharacterFromContext()` at line 414, but it's not fully utilizing the event bus yet. The TODO comment at line 422 indicates event emission is planned but not implemented.

### 4. Modifier System Analysis

The modifier system is sophisticated and ready:

**Capabilities:**
- `RawValue`: Flat numeric bonuses (+2 from Rage)
- `DiceValue`: Dice-based modifiers (1d4 from Bless)
- Priority system for modifier ordering
- Conditional modifiers (only apply in certain situations)
- Duration tracking for temporary effects

**Example from Bless:**
```go
e.Context().AddModifier(events.NewModifier(
    "bless",
    events.ModifierSaveBonus,
    events.NewDiceValue(1, 4, "bless"),
    50,
))
```

## Architecture Assessment

### Will Current Architecture Support Character Attacks?

**YES**, the current architecture is well-suited for character attacks with conditions/modifiers. Here's why:

1. **Event Bus Integration**: The event system already defines all necessary attack events and has a proven modifier collection pattern

2. **Condition System**: Characters already store conditions and effects, just need to emit events when they change

3. **Dice Integration**: The dice roller exists and just needs to be wired into the attack logic

4. **Context Pattern**: Already implemented for characters, providing clean integration point

## What Needs to Be Added

### 1. Core Attack Implementation

```go
// In character.go
func (c *Character) Attack(ctx context.Context, weapon Weapon, target Target, eventBus events.EventBus) AttackResult {
    // Create attack event
    attackEvent := events.NewGameEvent(events.EventOnAttackRoll, c, target)
    
    // Add weapon and attack context
    attackEvent.Context().Set("weapon", weapon.Name)
    attackEvent.Context().Set("weapon_type", weapon.Properties) // for finesse weapons
    
    // Determine ability modifier (STR or DEX for finesse)
    abilityMod := c.getAttackAbilityModifier(weapon)
    attackEvent.Context().Set("base_modifier", abilityMod)
    
    // Add proficiency if proficient
    if c.isProficientWith(weapon) {
        attackEvent.Context().Set("proficiency_bonus", c.proficiencyBonus)
    }
    
    // Check conditions that affect attacks
    if c.HasCondition(conditions.Blessed) {
        attackEvent.Context().Set("has_bless", true)
    }
    
    // Publish event to collect modifiers
    eventBus.Publish(ctx, attackEvent)
    
    // Calculate total modifier
    totalMod := abilityMod
    if prof, ok := attackEvent.Context().GetInt("proficiency_bonus"); ok {
        totalMod += prof
    }
    
    // Add collected modifiers
    for _, mod := range attackEvent.Context().Modifiers() {
        if mod.Type() == events.ModifierAttackBonus {
            totalMod += mod.ModifierValue().GetValue()
        }
    }
    
    // Roll the attack
    roll, _ := dice.DefaultRoller.Roll(20)
    total := roll + totalMod
    
    // Check for critical
    isCrit := roll == 20
    isFumble := roll == 1
    
    // Determine hit
    hit := !isFumble && (isCrit || total >= target.AC())
    
    return AttackResult{
        Roll:     roll,
        Modifier: totalMod,
        Total:    total,
        Hit:      hit,
        Critical: isCrit,
        Fumble:   isFumble,
    }
}
```

### 2. Condition Registration on Event Bus

```go
// Register condition handlers when character is loaded
func (c *Character) RegisterConditionHandlers(eventBus events.EventBus) {
    // Bless condition
    if c.HasCondition(conditions.Blessed) {
        eventBus.SubscribeFunc(events.EventOnAttackRoll, 50, func(ctx context.Context, e events.Event) error {
            if e.Source().GetID() == c.id {
                e.Context().AddModifier(events.NewModifier(
                    "bless",
                    events.ModifierAttackBonus,
                    events.NewDiceValue(1, 4, "bless"),
                    50,
                ))
            }
            return nil
        })
    }
    
    // Other conditions...
}
```

### 3. Enhanced LoadFromContext

```go
func LoadCharacterFromContext(ctx context.Context, gameCtx game.Context[Data],
    raceData *race.Data, classData *class.Data, backgroundData *shared.Background) (*Character, error) {
    
    char, err := LoadCharacterFromData(gameCtx.Data(), raceData, classData, backgroundData)
    if err != nil {
        return nil, err
    }
    
    // Register with event bus
    if gameCtx.EventBus() != nil {
        // Register condition handlers
        char.RegisterConditionHandlers(gameCtx.EventBus())
        
        // Emit loaded event
        event := events.NewGameEvent("character.loaded", char, nil)
        gameCtx.EventBus().Publish(ctx, event)
    }
    
    return char, nil
}
```

### 4. Attack Result Structure Enhancement

```go
type AttackResult struct {
    Roll         int
    NaturalRoll  int      // The d20 roll before modifiers
    Modifier     int      // Total modifiers applied
    Total        int      // Roll + Modifier
    Hit          bool
    Critical     bool
    Fumble       bool
    Advantage    bool     // Was rolled with advantage
    Disadvantage bool     // Was rolled with disadvantage
    Modifiers    []events.Modifier // All modifiers that were applied
}
```

## Missing Core Functionality

### 1. Weapon Proficiency System
- Need to check if character is proficient with weapon type
- Store weapon proficiencies in character data
- Apply proficiency bonus only when proficient

### 2. Attack Ability Determination
- STR for melee weapons
- DEX for ranged weapons
- DEX for finesse weapons (player choice)
- Spellcasting ability for spell attacks

### 3. Advantage/Disadvantage System
- Roll 2d20, take higher (advantage) or lower (disadvantage)
- Conditions that grant advantage/disadvantage
- Can't stack - they cancel out

### 4. Critical Hit System
- Natural 20 always hits
- Doubles damage dice (not modifiers)
- Some features expand crit range (Champion fighter: 19-20)

## Architectural Concerns

### 1. Event Bus Lifecycle
**Concern**: Who owns the event bus and when are handlers registered?
**Solution**: Pass event bus through context, register handlers on character load

### 2. Modifier Stacking
**Concern**: How do we handle multiple modifiers of the same type?
**Solution**: Use priority system and modifier types to control stacking rules

### 3. Persistent vs Temporary Effects
**Concern**: Conditions need to persist between sessions
**Solution**: Store condition data, re-register handlers on load

## Recommendations

### Immediate Actions (Phase 1)
1. Implement basic attack calculation in Character
2. Wire in dice roller for attack rolls
3. Add weapon proficiency checking
4. Calculate proper ability modifiers

### Event Integration (Phase 2)
1. Publish attack events during Attack() method
2. Register basic condition handlers (Bless, Bane, etc.)
3. Implement advantage/disadvantage through events
4. Add damage calculation with modifiers

### Advanced Features (Phase 3)
1. Spell attacks using same system
2. Area of effect attacks
3. Reaction system (opportunity attacks)
4. Exhaustive condition implementations

## Conclusion

The current architecture is MORE than capable of supporting character attacks with conditions and modifiers. The event bus system is particularly well-designed for this purpose. The main work needed is:

1. **Connecting existing pieces**: The dice roller, event bus, and character are all ready - they just need to be wired together
2. **Implementing attack logic**: The placeholder needs to become real calculation
3. **Registering condition handlers**: Conditions need to register their modifiers with the event bus

The LoadFromContext pattern is already in place for characters and works well. The spatial tool's implementation shows this pattern is mature and effective.

**Assessment: GREEN LIGHT** - The architecture will support character attacks excellently. No major refactoring needed, just implementation of the missing pieces.