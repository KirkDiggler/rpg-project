# Death Saves

## Status: Todo (Issue #296)

## The Need

When a character hits 0 HP, they don't die immediately. They fall unconscious and make death saving throws each turn. This adds drama and gives teammates a chance to help.

## D&D 5e Rules

1. At 0 HP: character falls unconscious (Unconscious condition)
2. On their turn: roll d20 (no modifiers)
   - 10+ = success
   - 9 or below = failure
   - Natural 20 = regain 1 HP, conscious again
   - Natural 1 = 2 failures
3. 3 successes = stabilized (unconscious but not dying)
4. 3 failures = dead
5. Taking damage while at 0 HP = automatic failure
6. Taking a critical hit while at 0 HP = 2 failures
7. Healing while at 0 HP = conscious at healed amount

## Design Thoughts

### As a Condition
Death saves fit the condition pattern perfectly:
- `UnconsciousCondition.Apply(bus)` subscribes to:
  - TurnStartTopic: prompt for death save roll
  - DamageReceivedTopic: auto-fail on hit
  - HealingReceivedTopic: wake up
- Tracks successes/failures internally
- Publishes `CharacterDiedEvent` or `CharacterStabilizedEvent`

### What the API Does
- Detect 0 HP after damage resolution
- Apply Unconscious condition via toolkit
- On character's turn: toolkit rolls death save automatically
- Return result in turn events
- If healed: toolkit removes condition, character regains consciousness

### What the UI Shows
- Unconscious indicator on character
- Death save tracker (successes/failures as pips)
- Dramatic roll animation
- "STABILIZED" or "DEAD" resolution

## Dependencies

- Healing system (for revival) - partially exists via HealingReceivedEvent
- Unconscious condition - needs implementation
- Character death state - what happens after?

## Open Questions

- Do we allow players to manually roll death saves (add tension) or auto-roll?
- What happens when a character dies? Remove from encounter? Spectator mode?
- Can other players use actions to stabilize (Medicine check)?
- Should monsters target unconscious players? (Cruel but RAW-legal)
