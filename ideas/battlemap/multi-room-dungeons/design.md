# Multi-Room Dungeons: Absolute Positioning

## Status: In Progress — API is the blocker

## The Problem

When a player opens a door to room 2, each room needs to render at its correct position in dungeon-space. The client should not calculate offsets — the toolkit owns spatial rules.

## The Solution

Rooms have absolute positions in dungeon-space. The toolkit's environment package calculates room origins via BFS during spatial placement.

## What Exists (Code Audit 2026-02-06)

### Toolkit ✅ Complete
- `RoomNode.Position` — absolute cube coordinates per room
- `ToAbsolute()` / `ToLocal()` — coordinate conversion helpers
- `roomPositions` map in `BasicEnvironment` — tracks all room origins
- `localToAbsolute()` — converts entity positions during persistence
- BFS spatial placement in `GraphGenerator` — calculates room positions from door alignment

### Proto ✅ Complete
- `Room.origin` field (field 9, type `Position`) added in PR #126
- Generated Go code includes `Room.Origin *v1alpha1.Position`
- API's `go.mod` already points to the generated commit with this field

### API ❌ The Gap
The toolkit calculates room positions but the API never passes them through:

**`convertRoomDataToProto()`** builds proto Room with:
- Id, Type, Width, Height, GridType, HexOrientation, Entities
- Does NOT set `Origin` or `Walls`

**`convertToDungeonEntity()`** stores:
- Rooms map, Connections, StartRoomID, BossRoomID, state
- Does NOT store room origins/positions

**`OpenDoor()`** returns:
- `RoomOffset: nil // TODO: Calculate offset for grid merge`

**`convertToRoomData()`** creates `spatial.RoomData` with:
- ID, Type, Width, Height, GridType, CubeEntities
- No origin concept

### UI ❌ Blocked on API
- `InstancedHexTiles` renders tiles in a loop: `for z in 0..gridHeight, x in 0..gridWidth`
- All tiles start from (0,0,0) — single room only
- No multi-room state accumulation
- Camera fixed at position `[8, 10, 8]`

## Data Flow (current vs target)

### Current (broken)
```
Toolkit RoomNode.Position ✅
  → dungeon.Room (no origin) ❌
    → entities.Dungeon (no origins) ❌
      → spatial.RoomData (no origin) ❌
        → proto Room (Origin: nil) ❌
          → UI renders at (0,0) ❌
```

### Target
```
Toolkit RoomNode.Position ✅
  → dungeon.Room.Origin ← ADD
    → entities.Dungeon.RoomOrigins ← ADD
      → orchestrator passes origin ← THREAD
        → proto Room.Origin set ← SET
          → UI accumulates + renders absolute ← BUILD
```

## Implementation Phases

### Phase 1: API — Thread Room Origin (rpg-api #401)

**Step 1a: Add origin to dungeon component Room**
```go
// internal/components/dungeon/types.go
type Room struct {
    ID       string
    Shape    *Shape
    // ... existing fields ...
    Origin   Position  // NEW: absolute position in dungeon-space
}
```

**Step 1b: Calculate room positions during generation**

Two options:

**Option A (recommended):** Calculate positions in the dungeon generator after rooms and connections are built. Use connection directions + room dimensions to compute BFS offsets.

**Option B:** Use toolkit's `GraphGenerator` directly. More correct but requires wiring toolkit environment creation into the dungeon component.

The calculation:
- Room 0 (start): origin = (0, 0, 0)
- Room N: origin = previous_room.origin + offset based on connection direction and room dimensions
- For hex grids: offset depends on which wall the door is on (north/south/east/west)

**Step 1c: Store origins on dungeon entity**
```go
// internal/entities/dungeon.go
type Dungeon struct {
    // ... existing fields ...
    RoomOrigins map[string]Position `json:"room_origins"` // room ID -> absolute origin
}
```

**Step 1d: Populate proto Room.origin**
```go
// converters.go - update convertRoomDataToProto to accept origin
func convertRoomDataToProto(roomData interface{}, origin *Position) *dnd5ev1alpha1.Room {
    room := &dnd5ev1alpha1.Room{
        // ... existing fields ...
        Origin: &apiv1alpha1.Position{
            X: int32(origin.X),
            Y: int32(origin.Y),
            Z: int32(origin.Z),
        },
    }
    return room
}
```

**Step 1e: Thread through StartCombat and OpenDoor**
- StartCombat: first room origin is (0,0,0)
- OpenDoor: look up revealed room's origin from `dungeon.RoomOrigins`

### Phase 2: UI — Accumulate Rooms (rpg-dnd5e-web #311)

```typescript
interface DungeonMapState {
  floorTiles: CubeCoord[]           // All tile positions across rooms
  entities: Map<string, EntityData>  // All entities keyed by ID
  walls: WallData[]                  // All walls across rooms
  doors: Map<string, DoorData>       // All doors
  revealedRoomIds: Set<string>       // Tracking
}
```

- On `CombatStartedEvent`: init with first room tiles at origin (0,0,0)
- On `RoomRevealedEvent`: merge new tiles at `room.origin` offset

### Phase 3: UI — Render Absolute (rpg-dnd5e-web #312)

Refactor `InstancedHexTiles`:
- Old: `gridWidth, gridHeight` props → loop from (0,0)
- New: `floorTiles: CubeCoord[]` prop → render at absolute positions

### Phase 4: UI — Camera Follow (rpg-dnd5e-web #313)

- Track active character position
- Smooth lerp camera to character on turn start
- Option C from issue: manual pan + auto-center button

### Phase 5: UI — Room Navigation (rpg-dnd5e-web #310)

With contiguous map rendering, room "switching" becomes camera panning. Add:
- Mini-map showing revealed rooms (#266)
- "Center on character" button
- Room indicators showing monster presence

## Design Decisions

| Decision | Choice | Reasoning |
|----------|--------|-----------|
| Coordinate system | Cube (q,r,s) | No ambiguity, constraint q+r+s=0, industry standard |
| Position ownership | Toolkit | Toolkit owns all spatial rules. No math in client. |
| Storage | Origins per room on dungeon entity | Simple, queryable, doesn't require toolkit reconstruction |
| Proto field | Room.origin (field 9) | Already exists and generated |
| Room position calc | BFS from start room using door directions | Matches toolkit's approach, deterministic |

## Previous Attempt

**API PR #400** — Implemented unified coordinates. Closed without merge.
- Approach was sound (BFS, absolute coords)
- Too large a change in one PR
- This plan breaks into 5 testable phases

## References
- rpg-api#426 — Tracking issue
- rpg-api#399, #401, #402 — API issues
- rpg-dnd5e-web#310-313, #266 — UI issues
- Proto PR #126 — Room.origin field
- API PR #400 — Previous attempt (closed)
