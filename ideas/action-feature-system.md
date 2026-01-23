# Actions and Features: The Activate/Apply Model

## Status: Active Design - Partially Implemented

## The Insight

Everything a player "does" in combat is a `core.Action[T]`. But they come in different flavors:

### Activated Actions (Player triggers, immediate effect)
- **Rage**: Consumes a use -> publishes ConditionAppliedEvent -> condition takes over
- **Second Wind**: Consumes a use -> rolls healing -> publishes HealingReceivedEvent
- **Action Surge**: Consumes a use -> directly grants an extra action

### Applied Effects (Subscribe to bus, passive/ongoing)
- **RagingCondition**: Listens for damage chains -> adds rage bonus at StageFeatures
- **Prone**: Listens for attack rolls -> grants advantage to melee attackers
- **Blessed**: Listens for attack/save chains -> adds 1d4

### The Spectrum

```
Pure Activate          Both                    Pure Apply
(immediate, done)     (activate creates        (passive, ongoing)
                       an applied effect)
|                     |                        |
Second Wind           Rage                     Pack Tactics
Action Surge          Bless (spell)            Undead Fortitude
                      Flurry of Blows?
```

## The Open Question: Combat Actions as Features

Strikes and special attacks are technically "activated" too:
- **Basic Attack**: Activate with weapon ref + target -> resolve through chain
- **Offhand Strike (Two-Weapon Fighting)**: Activate with offhand weapon -> bonus action attack
- **Flurry of Blows**: Activate (costs Ki) -> grants 2 unarmed bonus action attacks

Are these "features" or "actions"? The current model:
- Features have a `FeatureInput` and are stored on the character
- Strikes/attacks have an `AttackInput` and are resolved through combat

**Possible unification**: Both are `core.Action[T]` but with different input types. The API just calls `Activate(key, input)` and the toolkit figures out what kind of action it is.

## What's Implemented

| Feature | Activate | Apply/Condition | Resource | Chain Integration |
|---------|----------|-----------------|----------|-------------------|
| Rage | Yes | Yes (RagingCondition) | Long rest | Damage chain |
| Second Wind | Yes | Resource only | Short rest | None |
| Action Surge | Yes | Resource only | Short rest | None |
| Flurry of Blows | Not yet | Not yet | Ki (short rest) | Would grant attacks |
| Offhand Strike | Not yet | Not yet | None (bonus action) | Would be attack |
| Sneak Attack | Passive | Event listener | None | Damage chain |

## Design Principles

1. **core.Action[T] is the universal interface** - everything activatable implements it
2. **CanActivate checks preconditions** - resources, action economy, valid state
3. **Activate executes and publishes** - never both modifies AND subscribes
4. **Conditions do the subscribing** - features create conditions, conditions listen
5. **Chains order the modifiers** - base -> features -> conditions -> equipment -> final
6. **Resources recover via events** - subscribe to RestTopic, auto-restore

## Next Steps

- Implement Flurry of Blows (Monk) - tests the "activate grants attacks" pattern
- Implement Offhand Strike - tests the "bonus action attack" pattern
- Consider whether basic attacks should go through the same Activate path
- Consider ActivateFeature vs ActivateAction as separate RPCs or unified
