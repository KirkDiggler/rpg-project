# Fix OpenDoor Room-Local Positions and Non-Unique Entity IDs

**Issue:** [rpg-api#441](https://github.com/KirkDiggler/rpg-api/issues/441)
**Date:** 2026-03-21

## Problem

When `OpenDoor` returns a revealed room, three things are wrong:

1. **Entity positions are room-local** — not offset by `Room.origin`
2. **Entity IDs collide across rooms** — `monster-melee-0` in room 2 overwrites room 1's entry
3. **Wall positions may be room-local** — needs verification (`shiftWallsByOrigin` exists but may not have been deployed when the QA capture was taken)

## Design

### Fix 1: Shift entity positions by room origin

**Where:** `internal/handlers/dnd5e/v1alpha1/encounter/converters.go`

Add `shiftEntitiesByOrigin(room.Entities, room.Origin)` that mirrors the existing `shiftWallsByOrigin` pattern. For each entity placement, add the room origin to its position coordinates.

**Called from:** `handler.go` OpenDoor method, immediately after the existing `shiftWallsByOrigin` call (~line 170).

Also apply the same shift in any other code path that returns revealed room entities (e.g., `convertCombatStateToProto` if it includes entity positions from newly revealed rooms).

### Fix 2: Make entity IDs unique per dungeon via room-scoped prefix

**Where:** `internal/orchestrators/encounter/orchestrator.go` (~line 1066)

Change:
```go
monsterID := fmt.Sprintf("monster-%s", placement.ID)
```

To:
```go
monsterID := fmt.Sprintf("monster-%s-%s", roomID, placement.ID)
```

This produces IDs like `monster-room2-melee-0` instead of `monster-melee-0`. The room ID is baked in at spawn time and stays with the monster — if it moves to another room, its ID doesn't change.

The same change applies everywhere `fmt.Sprintf("monster-%s", placement.ID)` appears in the orchestrator (both the monsterIDs slice build and the CubeEntities placement loop).

### Fix 3: Verify wall shift

**Where:** `internal/handlers/dnd5e/v1alpha1/encounter/converters.go`

`shiftWallsByOrigin` already exists and is called in the OpenDoor handler. Verify via test that wall coordinates are correctly shifted. If the QA capture predates the wall fix, close this item. If walls are still room-local, debug the existing shift function.

## Files Changed

| File | Change |
|------|--------|
| `internal/handlers/dnd5e/v1alpha1/encounter/converters.go` | Add `shiftEntitiesByOrigin` function |
| `internal/handlers/dnd5e/v1alpha1/encounter/handler.go` | Call `shiftEntitiesByOrigin` in OpenDoor |
| `internal/orchestrators/encounter/orchestrator.go` | Room-prefix entity IDs at spawn |
| `internal/handlers/dnd5e/v1alpha1/encounter/converters_test.go` | Tests for entity shift |
| `internal/orchestrators/encounter/orchestrator_test.go` | Tests for unique IDs |

## Boundary Rule Compliance

- **API orchestrates** — ID uniqueness and coordinate transforms happen here
- **Toolkit returns room-local data** — unchanged, correctly scoped
- **Client receives absolute coordinates** — no conversion logic needed on frontend

## Testing

- Unit test: `shiftEntitiesByOrigin` correctly adds origin to all entity positions
- Unit test: Monster IDs include room ID and don't collide across rooms
- Integration: OpenDoor response has absolute entity positions and unique IDs
