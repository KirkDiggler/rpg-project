# Rest System

## Status: In Progress (Issue #294)

## The Need

Characters need to start dungeons at full capacity. Between rooms, they may want to short rest. The rest system resets resources (HP, feature uses, hit dice).

## Design

### Long Rest (Before Dungeon Start)

Automatic. When dungeon starts, all characters get a long rest:
- HP restored to maximum
- All hit dice recovered
- All feature uses reset (Rage, Action Surge, Second Wind)
- All spell slots recovered
- All conditions removed

### Short Rest (Between Rooms - Stretch)

Player-initiated between rooms:
- Spend hit dice to recover HP (roll hit die + CON mod per die spent)
- Short-rest features reset (Second Wind, Action Surge, Ki)
- Long-rest features do NOT reset (Rage uses)

## How It Works with Events

The toolkit already has `RestTopic` and `RecoverableResource`:

```go
// Features already subscribe to RestTopic
resource.Apply(bus)  // subscribes to rest events

// When rest happens:
bus.Publish(RestEvent{
    CharacterID: "char-1",
    Type: RestTypeLong,  // or RestTypeShort
})

// Each resource checks if it should reset
func (r *RecoverableResource) onRest(ctx, event RestEvent) {
    if event.CharacterID != r.ownerID { return }
    if event.Type matches r.resetType {
        r.current = r.max
    }
}
```

## Implementation Plan

### API Side (rpg-api #294)
1. Add `LongRest` to encounter orchestrator
2. Call it in `CreateDungeon` before combat starts
3. Toolkit handles the actual reset logic
4. Persist updated character state

### Toolkit Side
- Already has RecoverableResource with RestTopic subscription
- May need a top-level `Rest(character, restType)` function
- Should publish RestEvent for each character
- Each resource reacts independently

## What's Blocking

Nothing architecturally. The event-driven resource system is already built. This is primarily an API orchestration task: call the right toolkit function at dungeon start.

## Test Cases

- Character at half HP gets full HP after long rest
- Rage with 0 uses gets all uses back after long rest
- Second Wind with 0 uses gets use back after short rest
- Rage does NOT reset on short rest
- Hit dice restored to max on long rest
- Short rest: spending hit dice heals correct amount
