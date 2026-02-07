# Idea: Multi-Room Dungeons with Absolute Positioning

Rooms have absolute positions in dungeon-space. The toolkit calculates offsets. API stores them. UI renders directly without math.

## Milestone
4class-dungeon

## Status
In Progress. Toolkit dungeon generation merged. API needs to thread Room.origin through. UI blocked on API.

## Key Files (across repos)

### Toolkit (✅ ready)
- `tools/environments/graph_generator.go` — `RoomNode.Position`, `ToAbsolute()`, `ToLocal()`
- `tools/environments/environment_persistence.go` — `roomPositions` map, local→absolute conversion
- `tools/spatial/data.go` — `RoomData` struct (no origin field yet)

### Proto (✅ ready)
- `dnd5e/api/v1alpha1/encounter.proto` — `Room.origin` (field 9, type Position)
- Generated Go code at commit `86f5e4e` on `generated` branch

### API (❌ gap — Phase 1)
- `internal/components/dungeon/types.go` — `Room` struct needs Origin
- `internal/entities/dungeon.go` — needs `RoomOrigins` map
- `internal/orchestrators/encounter/orchestrator.go` — `convertToDungeonEntity()`, `convertToRoomData()`, `OpenDoor()`
- `internal/handlers/dnd5e/v1alpha1/encounter/converters.go` — `convertRoomDataToProto()` needs to set Origin

### UI (❌ blocked on API)
- `src/components/hex-grid/InstancedHexTiles.tsx` — renders from (0,0) with gridWidth×gridHeight
- `src/components/hex-grid/HexGrid.tsx` — passes single room dimensions
- `src/components/encounter/BattleMapPanel.tsx` — single room context
- `src/api/useEncounterStream.ts` — event handling for room reveals

## Tracking
- rpg-api#426 — Implementation plan and tracking issue
- rpg-api#401 — Phase 1: Populate Room.origin
- rpg-dnd5e-web#311 — Phase 2: Accumulate rooms
- rpg-dnd5e-web#312 — Phase 3: Render absolute positions
- rpg-dnd5e-web#313 — Phase 4: Camera follow
- rpg-dnd5e-web#310 — Phase 5: Room navigation

## State
See `design.md` for architecture and `memories.json` for structured progress.
