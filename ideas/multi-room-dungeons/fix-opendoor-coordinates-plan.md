# Fix OpenDoor Room-Local Positions and Non-Unique Entity IDs — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix OpenDoor so revealed room entities have dungeon-absolute positions and unique IDs across rooms.

**Architecture:** Three targeted fixes in the API layer. Entity IDs get room-scoped prefixes at the orchestrator. Entity positions get origin-shifted at the handler (mirroring the existing wall shift). Wall shift is verified working.

**Tech Stack:** Go, gRPC, gomock, testify suites

**Spec:** `rpg-project/ideas/multi-room-dungeons/fix-opendoor-coordinates-design.md`

**Issue:** [rpg-api#441](https://github.com/KirkDiggler/rpg-api/issues/441)

---

### Task 1: Make entity IDs unique per dungeon — orchestrator

**Files:**
- Modify: `internal/orchestrators/encounter/orchestrator.go:1064-1097` (`placeMonsters`)
- Modify: `internal/orchestrators/encounter/orchestrator.go:2777-2800` (OpenDoor monster creation)
- Test: `internal/orchestrators/encounter/open_door_test.go`

The `placeMonsters` function and the OpenDoor monster loop both generate IDs as `fmt.Sprintf("monster-%s", placement.ID)`. Since `placement.ID` is room-scoped (e.g., `melee-0`), the same ID appears in every room. Both need the room ID baked in.

- [ ] **Step 1: Write failing test — unique monster IDs across rooms**

Add a test in `open_door_test.go` that opens a door and asserts the revealed room's monster IDs include the room ID (e.g., `monster-room2-melee-0`), and that they differ from room 1's monster IDs.

- [ ] **Step 2: Run test to verify it fails**

Run: `cd /home/kirk/personal/rpg-api && go test ./internal/orchestrators/encounter/ -run TestOpenDoor -v`
Expected: FAIL — IDs are `monster-melee-0` without room prefix.

- [ ] **Step 3: Update `placeMonsters` to include room ID**

In `orchestrator.go`, change `placeMonsters` signature to accept `roomID string`:

```go
func (o *Orchestrator) placeMonsters(roomData *spatial.RoomData, room *dungeon.Room, roomID string) []*monster.Data {
```

Update both ID generation lines (1066 and 1097):
```go
// Was: fmt.Sprintf("monster-%s", placement.ID)
monsterIDs = append(monsterIDs, fmt.Sprintf("monster-%s-%s", roomID, placement.ID))
// ...
monsterID := fmt.Sprintf("monster-%s-%s", roomID, placement.ID)
```

Update all callers of `placeMonsters` (lines 736 and 3366) to pass `room.ID`:
```go
monsters := o.placeMonsters(roomData, startRoom, startRoom.ID)
```

- [ ] **Step 4: Update OpenDoor monster loop (line 2779)**

In the OpenDoor flow, update the monster ID generation:
```go
// Was: monsterID := fmt.Sprintf("monster-%s", placement.ID)
monsterID := fmt.Sprintf("monster-%s-%s", revealedRoomID, placement.ID)
```

This affects lines 2779, 2785, and the downstream uses of `monsterID` in that loop.

- [ ] **Step 5: Run tests to verify they pass**

Run: `cd /home/kirk/personal/rpg-api && go test ./internal/orchestrators/encounter/ -run TestOpenDoor -v`
Expected: PASS — monster IDs now include room ID.

- [ ] **Step 6: Run all orchestrator tests to check for regressions**

Run: `cd /home/kirk/personal/rpg-api && go test ./internal/orchestrators/encounter/ -v -count=1`
Expected: All tests pass. Some existing tests may need updating if they assert on specific monster ID strings.

- [ ] **Step 7: Commit**

```bash
git add internal/orchestrators/encounter/orchestrator.go internal/orchestrators/encounter/open_door_test.go
git commit -m "fix: make monster IDs unique per dungeon by including room ID (#441)"
```

---

### Task 2: Shift entity positions by room origin — handler

**Files:**
- Modify: `internal/handlers/dnd5e/v1alpha1/encounter/converters.go:32-50` (add `shiftEntitiesByOrigin` near `shiftWallsByOrigin`)
- Modify: `internal/handlers/dnd5e/v1alpha1/encounter/handler.go:163-171` (call it in OpenDoor)
- Test: `internal/handlers/dnd5e/v1alpha1/encounter/converters_test.go`

The handler already calls `shiftWallsByOrigin` to offset wall coordinates. Entity positions need the same treatment.

- [ ] **Step 1: Write failing test — entity positions shifted by origin**

Add a test in `converters_test.go` for `shiftEntitiesByOrigin`:

```go
func (s *ConvertersTestSuite) TestShiftEntitiesByOrigin() {
    entities := map[string]*dnd5ev1alpha1.EntityPlacement{
        "monster-room2-melee-0": {
            EntityId: "monster-room2-melee-0",
            Position: &apiv1alpha1.Position{X: 3, Y: -5, Z: 2},
        },
    }
    origin := &apiv1alpha1.Position{X: 0, Y: 17, Z: -17}

    shiftEntitiesByOrigin(entities, origin)

    s.Equal(float64(3), entities["monster-room2-melee-0"].Position.X)
    s.Equal(float64(12), entities["monster-room2-melee-0"].Position.Y)
    s.Equal(float64(-15), entities["monster-room2-melee-0"].Position.Z)
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd /home/kirk/personal/rpg-api && go test ./internal/handlers/dnd5e/v1alpha1/encounter/ -run TestShiftEntitiesByOrigin -v`
Expected: FAIL — function does not exist.

- [ ] **Step 3: Implement `shiftEntitiesByOrigin`**

Add to `converters.go` right after `shiftWallsByOrigin` (after line 50):

```go
// shiftEntitiesByOrigin shifts entity positions by the room origin offset.
// Entities are stored in room-local coordinates; this converts them to dungeon-absolute.
func shiftEntitiesByOrigin(entities map[string]*dnd5ev1alpha1.EntityPlacement, origin *apiv1alpha1.Position) {
	if origin == nil || entities == nil {
		return
	}
	for _, e := range entities {
		if e.Position != nil {
			e.Position.X += origin.X
			e.Position.Y += origin.Y
			e.Position.Z += origin.Z
		}
	}
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cd /home/kirk/personal/rpg-api && go test ./internal/handlers/dnd5e/v1alpha1/encounter/ -run TestShiftEntitiesByOrigin -v`
Expected: PASS

- [ ] **Step 5: Wire into OpenDoor handler**

In `handler.go`, add the call after `shiftWallsByOrigin` (after line 170):

```go
		// Shift walls from room-local to dungeon-absolute coordinates
		shiftWallsByOrigin(openDoorRoom.Walls, openDoorRoom.Origin)
		// Shift entity positions from room-local to dungeon-absolute coordinates
		shiftEntitiesByOrigin(openDoorRoom.Entities, openDoorRoom.Origin)
```

- [ ] **Step 6: Add nil-safety tests**

Test that `shiftEntitiesByOrigin` handles nil origin, nil entities, and entities with nil positions without panicking.

- [ ] **Step 7: Run all handler tests**

Run: `cd /home/kirk/personal/rpg-api && go test ./internal/handlers/dnd5e/v1alpha1/encounter/ -v -count=1`
Expected: All tests pass.

- [ ] **Step 8: Commit**

```bash
git add internal/handlers/dnd5e/v1alpha1/encounter/converters.go internal/handlers/dnd5e/v1alpha1/encounter/converters_test.go internal/handlers/dnd5e/v1alpha1/encounter/handler.go
git commit -m "fix: shift entity positions by room origin in OpenDoor response (#441)"
```

---

### Task 3: Verify wall shift works and run pre-commit

**Files:**
- Read: `internal/handlers/dnd5e/v1alpha1/encounter/converters.go` (`shiftWallsByOrigin`)
- Test: `internal/handlers/dnd5e/v1alpha1/encounter/converters_test.go`

The `shiftWallsByOrigin` function exists and is called. The console errors in the screenshot show wall key collisions (`wall-15--11--4-11--4--7`), which will resolve naturally once wall coordinates are absolute (the client derives keys from coordinates). Verify with a test.

- [ ] **Step 1: Write test — wall shift produces correct absolute coordinates**

Add a test in `converters_test.go` that creates walls with room-local coordinates, calls `shiftWallsByOrigin` with an origin, and asserts the coordinates are absolute. If a test already exists, verify it covers this case.

- [ ] **Step 2: Run test to verify it passes**

Run: `cd /home/kirk/personal/rpg-api && go test ./internal/handlers/dnd5e/v1alpha1/encounter/ -run TestShiftWalls -v`
Expected: PASS — the function already works correctly.

- [ ] **Step 3: Run pre-commit**

Run: `cd /home/kirk/personal/rpg-api && make pre-commit`
Expected: All checks pass (fmt, lint, test).

- [ ] **Step 4: Commit any test additions**

```bash
git add internal/handlers/dnd5e/v1alpha1/encounter/converters_test.go
git commit -m "test: add wall shift verification for OpenDoor coordinate fix (#441)"
```

---

### Task 4: Final verification

- [ ] **Step 1: Run full test suite**

Run: `cd /home/kirk/personal/rpg-api && go test ./... -count=1`
Expected: All tests pass.

- [ ] **Step 2: Run ci-check**

Run: `cd /home/kirk/personal/rpg-api && make ci-check`
Expected: All checks pass.

- [ ] **Step 3: Push and create PR**

```bash
git push origin fix/opendoor-coordinates-441
gh pr create --title "fix: OpenDoor returns dungeon-absolute positions and unique entity IDs" --body "Fixes #441"
```
