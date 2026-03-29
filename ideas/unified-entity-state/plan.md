# Unified Entity State Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace fragmented entity state across protos, API, and web with a single `EntityState` message and `EncounterState` container, fixing bugs #357, #358, #347.

**Architecture:** Single `EntityState` proto with common fields (HP, position, conditions) + oneof details (character/monster/obstacle). `EncounterState` holds flat entity map + room layouts + combat state. API uses toolkit Load/Fire/Harvest/Save pattern. Web client has one store with snapshot/delta operations.

**Tech Stack:** Protobuf (buf), Go (rpg-api, rpg-toolkit), TypeScript/React (rpg-dnd5e-web), Vitest

**Spec:** `rpg-project/ideas/unified-entity-state/design.md`
**Toolkit cleanup notes:** `rpg-project/ideas/unified-entity-state/toolkit-cleanup.md`

---

## Phase 0: Spike Test (rpg-toolkit)

> Validates the toolkit supports the Load/Fire/Harvest/Save cycle before any other work begins. If this fails, toolkit changes are needed first — see `toolkit-cleanup.md`.

### Task 0.1: Spike Test — Character Damage via Toolkit Cycle

**Files:**
- Create: `rpg-toolkit/rulebooks/dnd5e/combat/spike_entity_cycle_test.go`

- [ ] **Step 1: Write the spike test**

This test validates the full cycle the API will use. Create a test that:
1. Creates a character and monster via their factory/builder functions
2. Wires both to the same event bus
3. Resolves an attack from monster → character via `combat.ResolveAttack`
4. Calls `character.ApplyDamage()` with the result
5. Asserts `character.IsDirty()` is true
6. Asserts `character.ToData().HitPoints` reflects the damage
7. Bonus: drops character to 0 HP and checks if Unconscious condition appears in `ToData().Conditions`

Look at existing combat tests in `rpg-toolkit/rulebooks/dnd5e/combat/` for patterns on how to set up combatants and the bus.

```go
func (s *SpikeEntityCycleSuite) TestCharacterDamageViaToolkitCycle() {
    // 1. Create bus
    // 2. Create character with known HP via factory/builder
    // 3. Create monster with a melee attack via factory
    // 4. Load monster actions + conditions (multi-step pattern)
    // 5. Subscribe both to bus
    // 6. Resolve attack: combat.ResolveAttack(ctx, &AttackInput{...})
    // 7. If hit, call character.ApplyDamage(ctx, &ApplyDamageInput{...})
    // 8. Assert character.IsDirty()
    // 9. data := character.ToData()
    // 10. Assert data.HitPoints < originalHP
}
```

- [ ] **Step 2: Run the spike test**

```bash
cd rpg-toolkit && go test ./rulebooks/dnd5e/combat/ -run TestSpikeEntityCycle -v
```

**If it passes:** The toolkit supports the pattern. Proceed to Phase 1.
**If it fails:** Document what's missing. See `toolkit-cleanup.md` for the fixes needed. Those must be implemented before continuing.

- [ ] **Step 3: Document results**

Update `rpg-project/ideas/unified-entity-state/memories.json` with a `test-criteria` entry recording the spike test outcome.

---

## Phase 1: Proto Changes (rpg-api-protos)

> All proto work in a single PR. This generates new types for both Go and TypeScript.
> **Repo:** `rpg-api-protos` at `/home/kirk/personal/rpg-api-protos`
> **Branch from:** fresh main

### Task 1.1: Add EntityState and Supporting Messages

**Files:**
- Modify: `dnd5e/api/v1alpha1/encounter.proto`

- [ ] **Step 1: Add new messages after existing EntityPlacement (around line 100)**

Add these messages to `encounter.proto`:

```protobuf
message EntityState {
  string entity_id = 1;
  EntityType entity_type = 2;
  string room_id = 3;

  // Spatial
  .api.v1alpha1.Position position = 4;
  EntitySize size = 5;
  bool blocks_movement = 6;
  bool blocks_line_of_sight = 7;

  // Combat
  int32 current_hit_points = 8;
  int32 max_hit_points = 9;
  repeated .dnd5e.api.v1alpha1.Condition active_conditions = 10;
  DeathSaveProgress death_saves = 11;

  // Type-specific rendering data
  oneof details {
    CharacterDetails character_details = 20;
    MonsterDetails monster_details = 21;
    ObstacleDetails obstacle_details = 22;
  }
}

message CharacterDetails {
  string name = 1;
  Race race = 2;
  CharacterClass character_class = 3;
  Appearance appearance = 4;
  EquipmentSlots equipment_slots = 5;
  int32 level = 6;
  int32 armor_class = 7;
}

message MonsterDetails {
  string name = 1;
  MonsterType monster_type = 2;
  int32 armor_class = 3;
}

message ObstacleDetails {
  ObstacleType obstacle_type = 1;
}
```

Note: Use `.api.v1alpha1.Position` since Position is in `room_common.proto`. Reference `Condition` from `common.proto`. Check existing imports at top of encounter.proto.

- [ ] **Step 2: Add CharacterClass enum to enums.proto if not already there**

Check `dnd5e/api/v1alpha1/enums.proto` — if there's already a `Class` enum, rename the field in CharacterDetails to use it. If the enum is literally named `Class`, the proto field should be `CharacterClass character_class` to avoid reserved word issues. Verify the existing enum name and adjust.

- [ ] **Step 3: Run buf lint**

```bash
cd rpg-api-protos && buf lint
```

Fix any lint issues before proceeding.

- [ ] **Step 4: Commit**

```bash
git add -A && git commit -m "feat: add EntityState and detail messages for unified entity state"
```

### Task 1.2: Add EncounterState and RoomLayout Messages

**Files:**
- Modify: `dnd5e/api/v1alpha1/encounter.proto`

- [ ] **Step 1: Add EncounterState and RoomLayout messages**

```protobuf
message RoomLayout {
  string id = 1;
  RoomType type = 2;
  int32 width = 3;
  int32 height = 4;
  .api.v1alpha1.GridType grid_type = 5;
  repeated .api.v1alpha1.Wall walls = 6;
  .api.v1alpha1.Position origin = 7;
}

message EncounterState {
  string encounter_id = 1;
  string dungeon_id = 2;
  map<string, EntityState> entities = 3;
  map<string, RoomLayout> rooms = 4;
  string current_room_id = 5;
  repeated string revealed_room_ids = 6;
  CombatState combat = 7;
  DungeonState dungeon_state = 8;
  int32 rooms_cleared = 9;
  map<string, DoorInfo> doors = 10;
}
```

Note: `GridType` and `Wall` are in `api/v1alpha1/room_common.proto`. Ensure the import exists.

- [ ] **Step 2: Add ActionEconomy to CombatState**

The existing `CombatState` (line 227) has fields 1-7. Add:

```protobuf
message CombatState {
  // ... existing fields 1-7 ...
  ActionEconomy current_turn_economy = 10;
}
```

Use field number 10 to leave room for future fields 8-9.

- [ ] **Step 3: Run buf lint**

```bash
buf lint
```

- [ ] **Step 4: Commit**

```bash
git add -A && git commit -m "feat: add EncounterState, RoomLayout, and ActionEconomy on CombatState"
```

### Task 1.3: Update Snapshot Event Payloads

**Files:**
- Modify: `dnd5e/api/v1alpha1/encounter.proto`

- [ ] **Step 1: Update snapshot events to carry EncounterState**

**Field number strategy:** Use field 10+ for new fields (`encounter_state = 10`). Mark removed old field numbers as `reserved` to prevent reuse. This differs from `design.md` which uses sequential numbers — the plan's approach is safer for proto binary compatibility. Check existing field numbers on each message before editing.

Find and update each snapshot event. Keep existing field numbers where reusing, add new fields for `EncounterState`:

**CombatStartedEvent** — find the existing message. Replace room/party/monsters/doors fields with:
```protobuf
message CombatStartedEvent {
  string dungeon_id = 1;
  EncounterState encounter_state = 10;
  repeated MonsterTurnResult monster_turns = 11;
}
```

**RoomRevealedEvent:**
```protobuf
message RoomRevealedEvent {
  string connection_id = 1;
  EncounterState encounter_state = 10;
}
```

**CombatEndedEvent:**
```protobuf
message CombatEndedEvent {
  EncounterResult result = 1;
  EncounterState encounter_state = 10;
}
```

**CombatResumedEvent:**
```protobuf
message CombatResumedEvent {
  EncounterState encounter_state = 10;
}
```

**PlayerReconnectedEvent:**
```protobuf
message PlayerReconnectedEvent {
  string player_id = 1;
  PartyMember member = 2;
  EncounterState encounter_state = 10;
}
```

**DungeonVictoryEvent:**
```protobuf
message DungeonVictoryEvent {
  string dungeon_id = 1;
  EncounterState encounter_state = 10;
}
```

**DungeonFailureEvent:**
```protobuf
message DungeonFailureEvent {
  string dungeon_id = 1;
  EncounterState encounter_state = 10;
}
```

For each event: remove the old fields that `EncounterState` replaces (room, combat_state, monsters, doors, party). Use `reserved` for removed field numbers to prevent reuse.

- [ ] **Step 2: Update GetEncounterState RPC response**

Find `GetEncounterStateResponse` and replace its fields with:
```protobuf
message GetEncounterStateResponse {
  EncounterState encounter_state = 1;
  string last_event_id = 2;
}
```

- [ ] **Step 3: Run buf lint**

```bash
buf lint
```

- [ ] **Step 4: Commit**

```bash
git add -A && git commit -m "feat: update snapshot events to carry EncounterState"
```

### Task 1.4: Update Delta Event Payloads

**Files:**
- Modify: `dnd5e/api/v1alpha1/encounter.proto`

- [ ] **Step 1: Update delta events to carry EntityState**

**AttackResolvedEvent:**
```protobuf
message AttackResolvedEvent {
  string attacker_id = 1;
  string target_id = 2;
  AttackResult result = 3;
  EntityState updated_attacker = 10;
  EntityState updated_target = 11;
}
```

**MonsterTurnCompletedEvent:**
```protobuf
message MonsterTurnCompletedEvent {
  MonsterTurnResult monster_turn = 1;
  repeated EntityState updated_entities = 10;
  CombatState combat_state = 11;
}
```

**MovementCompletedEvent:**
```protobuf
message MovementCompletedEvent {
  string entity_id = 1;
  repeated .api.v1alpha1.Position path = 2;
  EntityState updated_entity = 10;
  CombatState combat_state = 11;
}
```

**TurnEndedEvent:**
```protobuf
message TurnEndedEvent {
  CombatState combat_state = 10;
  repeated EntityState updated_entities = 11;
}
```

**FeatureActivatedEvent:**
```protobuf
message FeatureActivatedEvent {
  string entity_id = 1;
  string feature_id = 2;
  string message = 3;
  EntityState updated_entity = 10;
}
```

**ActionExecutedEvent:**
```protobuf
message ActionExecutedEvent {
  string entity_id = 1;
  ActionId action_id = 2;
  oneof result {
    AttackResult strike_result = 3;
    MoveResult move_result = 7;
  }
  repeated EntityState updated_entities = 10;
  CombatState combat_state = 11;
  GrantedAction granted_action = 12;
}
```

**CombatAbilityActivatedEvent:**
```protobuf
message CombatAbilityActivatedEvent {
  string entity_id = 1;
  CombatAbilityId ability_id = 2;
  EntityState updated_entity = 10;
  CombatState combat_state = 11;
}
```

**DeathSaveRolledEvent:**
```protobuf
message DeathSaveRolledEvent {
  string character_id = 1;
  int32 roll = 2;
  bool is_success = 3;
  bool is_critical_fail = 4;
  bool is_critical_success = 5;
  EntityState updated_entity = 10;
}
```

**CharacterUnconsciousEvent:**
```protobuf
message CharacterUnconsciousEvent {
  string character_id = 1;
  string source_entity_id = 2;
  EntityState updated_entity = 10;
}
```

**CharacterDiedEvent:**
```protobuf
message CharacterDiedEvent {
  string character_id = 1;
  EntityState updated_entity = 10;
}
```

**CharacterStabilizedEvent:**
```protobuf
message CharacterStabilizedEvent {
  string character_id = 1;
  EntityState updated_entity = 10;
}
```

**RestCompletedEvent:**
```protobuf
message RestCompletedEvent {
  RestType rest_type = 1;
  repeated EntityState updated_entities = 10;
}
```

For each: remove old fields replaced by EntityState (updated_room, updated_characters, updated_target, etc.). Use `reserved` for removed field numbers.

- [ ] **Step 2: Run buf lint**

```bash
buf lint
```

- [ ] **Step 3: Commit**

```bash
git add -A && git commit -m "feat: update delta events to carry EntityState"
```

### Task 1.5: Generate Code and Validate

**Files:**
- Generated: `gen/go/`, `gen/ts/`, `gen/cpp/`

- [ ] **Step 1: Run buf generate**

```bash
buf generate
```

- [ ] **Step 2: Verify Go types compile**

```bash
cd gen/go && go build ./...
```

- [ ] **Step 3: Verify TypeScript types compile**

Check that generated TS files exist in `gen/ts/dnd5e/api/v1alpha1/`. Look for `EntityState`, `EncounterState`, `CharacterDetails`, `MonsterDetails` in the generated encounter_pb files.

- [ ] **Step 4: Run pre-commit**

```bash
cd rpg-api-protos && make pre-commit
```

(If no Makefile, run `buf lint && buf generate` manually)

- [ ] **Step 5: Commit generated code**

```bash
git add -A && git commit -m "chore: regenerate code for unified entity state protos"
```

- [ ] **Step 6: Push and create PR**

```bash
git push -u origin <branch> && gh pr create --title "feat: unified entity state protos" --body "..."
```

---

## Phase 2: API Changes (rpg-api)

> After proto PR merges and new proto version is pulled in.
> **Repo:** `rpg-api` at `/home/kirk/personal/rpg-api`
> **Branch from:** fresh main
> **Depends on:** Phase 1 merged + proto dependency updated

### Task 2.0: Update Proto Dependency

**Files:**
- Modify: `go.mod`

- [ ] **Step 1: Update rpg-api-protos dependency**

```bash
cd rpg-api && go get github.com/KirkDiggler/rpg-api-protos@latest
go mod tidy
```

- [ ] **Step 2: Verify it compiles (expect errors — that's fine)**

```bash
go build ./... 2>&1 | head -50
```

Compilation errors are expected — old event struct fields are gone. This confirms the new protos are in.

- [ ] **Step 3: Commit**

```bash
git add go.mod go.sum && git commit -m "chore: update rpg-api-protos for unified entity state"
```

### Task 2.1: Add EntityStateData and Conversion Functions

**Files:**
- Create: `rpg-api/internal/entities/entity_state.go`
- Create: `rpg-api/internal/entities/entity_state_test.go`

- [ ] **Step 1: Write test for toEntityState conversion**

```go
// entity_state_test.go
func TestToEntityStateFromCharacterData(t *testing.T) {
    // Given a character.Data with known HP, race, class, appearance
    // When toEntityState is called
    // Then EntityState has correct fields populated
    // And character details oneof is set
}

func TestToEntityStateFromMonsterData(t *testing.T) {
    // Given a monster.Data with known HP, type
    // When toEntityState is called
    // Then EntityState has correct fields
    // And monster details oneof is set
}
```

- [ ] **Step 2: Run test to verify it fails**

```bash
go test ./internal/entities/ -run TestToEntityState -v
```

Expected: compilation error (types don't exist yet)

- [ ] **Step 3: Implement EntityStateData and conversion**

```go
// entity_state.go
package entities

type EntityStateData struct {
    EntityID    string
    EntityType  string  // "character", "monster", "obstacle"
    RoomID      string
    ToolkitData interface{}  // character.Data or monster.Data
    Position    spatial.CubeCoord
    Size        int
}

// toEntityState converts stored toolkit data to proto EntityState.
// This is the SINGLE conversion point between toolkit types and proto types.
func ToEntityState(esd *EntityStateData) *pb.EntityState {
    // Type-switch on ToolkitData to populate common fields (HP, conditions)
    // and the oneof details (CharacterDetails vs MonsterDetails)
}

func ToEntityStates(ids []string, entities map[string]*EntityStateData) []*pb.EntityState {
    // Batch conversion for event construction
}
```

- [ ] **Step 4: Run tests**

```bash
go test ./internal/entities/ -run TestToEntityState -v
```

- [ ] **Step 5: Commit**

```bash
git add internal/entities/entity_state*.go && git commit -m "feat: add EntityStateData and toEntityState conversion"
```

### Task 2.2: Add EncounterState Builder

**Files:**
- Create: `rpg-api/internal/entities/encounter_state_builder.go`
- Create: `rpg-api/internal/entities/encounter_state_builder_test.go`

- [ ] **Step 1: Write test for building full EncounterState proto**

```go
func TestBuildEncounterState(t *testing.T) {
    // Given entities map, rooms map, combat state, doors
    // When BuildEncounterState is called
    // Then all fields are populated correctly in the proto
}
```

- [ ] **Step 2: Run test to verify it fails**

```bash
go test ./internal/entities/ -run TestBuildEncounterState -v
```

- [ ] **Step 3: Implement BuildEncounterState**

```go
type BuildEncounterStateInput struct {
    EncounterID    string
    DungeonID      string
    Entities       map[string]*EntityStateData
    Rooms          map[string]*RoomLayoutData
    CurrentRoomID  string
    RevealedRoomIDs []string
    Combat         *CombatState
    DungeonState   string
    RoomsCleared   int
    Doors          map[string]*DoorData
}

func BuildEncounterState(input *BuildEncounterStateInput) *pb.EncounterState {
    // Convert all entities via ToEntityState
    // Convert rooms to proto RoomLayout
    // Convert combat state to proto CombatState
    // Convert doors to proto DoorInfo
}
```

- [ ] **Step 4: Run tests**

```bash
go test ./internal/entities/ -run TestBuildEncounterState -v
```

- [ ] **Step 5: Commit**

```bash
git add internal/entities/encounter_state_builder*.go && git commit -m "feat: add EncounterState builder"
```

### Task 2.3: Refactor EncounterData to Use Unified Entity Map

**Files:**
- Modify: `rpg-api/internal/repositories/encounters/repository.go` (lines 80-101)
- Modify: `rpg-api/internal/repositories/encounters/inmemory.go`

- [ ] **Step 1: Replace fragmented fields with unified entity map**

In `repository.go`, change `EncounterData`:

Remove:
```go
Monsters      []*monster.Data
CharacterHP   map[string]int
RoomData      interface{}
```

Add:
```go
Entities      map[string]*entities.EntityStateData
Rooms         map[string]*entities.RoomLayoutData
```

Keep: `InitiativeData`, `InitiativeRolls`, `ActionEconomy`, `BossMonsterIDs`, `HasBossRoom`, lobby fields.

- [ ] **Step 2: Update SaveInput and UpdateInput structs to match**

- [ ] **Step 3: Update inmemory.go Save/Load to use new fields**

- [ ] **Step 4: Fix compilation errors in repository package**

```bash
go build ./internal/repositories/encounters/...
```

Expect errors in orchestrator (next task). Repository should compile on its own.

- [ ] **Step 5: Commit**

```bash
git add internal/repositories/encounters/ && git commit -m "refactor: replace fragmented entity storage with unified Entities map"
```

### Task 2.4: Refactor Orchestrator — CreateDungeon

**Files:**
- Modify: `rpg-api/internal/orchestrators/encounter/orchestrator.go`

- [ ] **Step 1: Update CreateDungeon to populate Entities map**

Find where `CreateDungeon` currently sets `Monsters`, `CharacterHP`, and `RoomData` (around line 745+). Replace with:

```go
// Build entities map from spawned monsters and party characters
entityMap := make(map[string]*entities.EntityStateData)

// Add monsters
for _, m := range monsterDatas {
    entityMap[m.ID] = &entities.EntityStateData{
        EntityID:    m.ID,
        EntityType:  "monster",
        RoomID:      room.ID,
        ToolkitData: m,
        Position:    /* from room placement */,
        Size:        1,
    }
}

// Add characters
for _, member := range party {
    charData := member.Character.ToData()
    entityMap[charData.ID] = &entities.EntityStateData{
        EntityID:    charData.ID,
        EntityType:  "character",
        RoomID:      room.ID,
        ToolkitData: charData,
        Position:    /* from room placement */,
        Size:        1,
    }
}
```

- [ ] **Step 2: Update CombatStartedEvent construction**

Replace the old event fields with:
```go
event := &entities.CombatStartedEvent{
    DungeonID: dungeonID,
    EncounterState: entities.BuildEncounterState(&entities.BuildEncounterStateInput{
        // ... populate from encounter data
    }),
    MonsterTurns: monsterTurns,
}
```

- [ ] **Step 3: Fix compilation — follow errors until CreateDungeon compiles**

```bash
go build ./internal/orchestrators/encounter/...
```

- [ ] **Step 4: Run existing tests to catch regressions early**

```bash
go test ./internal/orchestrators/encounter/ -run TestCreateDungeon -v 2>&1 | head -50
```

Some tests will fail due to structural changes — that's expected. But running now catches logic bugs introduced in this task vs deferring to Task 2.9.

- [ ] **Step 5: Commit**

```bash
git add internal/orchestrators/encounter/orchestrator.go && git commit -m "refactor: CreateDungeon uses unified entity map"
```

### Task 2.5: Refactor Orchestrator — ResolveAttack

**Files:**
- Modify: `rpg-api/internal/orchestrators/encounter/orchestrator.go`

- [ ] **Step 1: Update ResolveAttack to use Load/Fire/Harvest pattern**

Replace the current flow with:

```go
// 1. Load entities from encounter data
attackerData := enc.Entities[input.AttackerID]
targetData := enc.Entities[input.TargetID]

// 2. Hydrate toolkit objects
bus := events.NewBus()
attacker := /* LoadFromData(attackerData.ToolkitData) + subscribe to bus */
target := /* LoadFromData(targetData.ToolkitData) + subscribe to bus */

// 3. Execute
result, err := combat.ResolveAttack(ctx, &combat.AttackInput{...})

// 4. Apply damage (caller's responsibility per toolkit design)
if result.Hit {
    target.ApplyDamage(ctx, &combat.ApplyDamageInput{...})
}

// 5. Harvest dirty state
if target.IsDirty() {
    enc.Entities[input.TargetID].ToolkitData = target.ToData()
}
if attacker.IsDirty() {
    enc.Entities[input.AttackerID].ToolkitData = attacker.ToData()
}

// 6. Build event with EntityState
event := &entities.AttackResolvedEvent{
    AttackerID:      input.AttackerID,
    TargetID:        input.TargetID,
    Result:          result,
    UpdatedAttacker: entities.ToEntityState(enc.Entities[input.AttackerID]),
    UpdatedTarget:   entities.ToEntityState(enc.Entities[input.TargetID]),
}
```

- [ ] **Step 2: Remove all CharacterHP references from ResolveAttack**

- [ ] **Step 3: Fix compilation**

```bash
go build ./internal/orchestrators/encounter/...
```

- [ ] **Step 4: Run existing attack tests to catch logic bugs early**

```bash
go test ./internal/orchestrators/encounter/ -run TestResolveAttack -v 2>&1 | head -50
```

Structural test failures are expected, but watch for logic errors (e.g., forgetting `ApplyDamage` call, wrong entity ID in harvest).

- [ ] **Step 5: Commit**

```bash
git add internal/orchestrators/encounter/orchestrator.go && git commit -m "refactor: ResolveAttack uses Load/Fire/Harvest pattern"
```

### Task 2.6: Refactor Monster Turns — Remove CharacterHP Math

**Files:**
- Modify: `rpg-api/internal/orchestrators/encounter/monster_turns.go`

- [ ] **Step 1: Replace CharacterHP subtraction with toolkit pattern**

Find lines 226-232 where the API manually does:
```go
currentHP := enc.CharacterHP[action.TargetID]
newHP := currentHP - attackResult.TotalDamage
```

Replace with the same Load/Fire/Harvest pattern: load character from `enc.Entities[targetID].ToolkitData`, call `ApplyDamage()`, harvest via `ToData()`.

- [ ] **Step 2: Update character state loading (lines 271-274)**

Instead of manually setting HP from `CharacterHP` map, read from the entity's `ToolkitData` which is already up to date after harvesting.

- [ ] **Step 3: Update MonsterTurnCompletedEvent construction**

Replace `UpdatedCharacters` field with `UpdatedEntities` carrying `EntityState` entries:
```go
event := &entities.MonsterTurnCompletedEvent{
    MonsterTurn:     turnResult,
    UpdatedEntities: entities.ToEntityStates(affectedIDs, enc.Entities),
    CombatState:     combatState,
}
```

- [ ] **Step 4: Fix compilation**

```bash
go build ./internal/orchestrators/encounter/...
```

- [ ] **Step 5: Run monster turn tests to catch logic bugs early**

```bash
go test ./internal/orchestrators/encounter/ -run TestMonsterTurn -v 2>&1 | head -50
```

This is the most critical refactor — the CharacterHP manual math is being replaced with toolkit bus flow. Watch for damage not being applied or wrong entities being harvested.

- [ ] **Step 6: Commit**

```bash
git add internal/orchestrators/encounter/monster_turns.go && git commit -m "refactor: monster turns use toolkit for damage, remove CharacterHP math"
```

### Task 2.7: Refactor Remaining Event Construction

**Files:**
- Modify: `rpg-api/internal/orchestrators/encounter/orchestrator.go`
- Modify: `rpg-api/internal/entities/encounter_events.go`

- [ ] **Step 1: Update all remaining event struct definitions in encounter_events.go**

Update each event struct to match the new proto payloads. Replace `UpdatedRoom`, `UpdatedCharacters`, `UpdatedTarget` fields with `UpdatedEntities []*pb.EntityState` or `EncounterState *pb.EncounterState`.

- [ ] **Step 2: Update event construction in orchestrator for each event type**

Work through each event in the orchestrator:
- `MovementCompleted` — carry `UpdatedEntity` and `CombatState`
- `TurnEnded` — carry `UpdatedEntities` and `CombatState`
- `FeatureActivated` — carry `UpdatedEntity`
- `ActionExecuted` — carry `UpdatedEntities` and `CombatState`
- `CombatAbilityActivated` — carry `UpdatedEntity` and `CombatState`
- `RoomRevealed` — carry full `EncounterState`
- Death save events — carry `UpdatedEntity`
- `CombatEnded`, `DungeonVictory`, `DungeonFailure` — carry full `EncounterState`
- `CombatResumed`, `PlayerReconnected` — carry full `EncounterState`

- [ ] **Step 3: Fix compilation for the entire orchestrator**

```bash
go build ./internal/orchestrators/encounter/...
```

- [ ] **Step 4: Commit**

```bash
git add internal/orchestrators/encounter/ internal/entities/ && git commit -m "refactor: all events carry EntityState or EncounterState"
```

### Task 2.8: Update Handler/Converter Layer

**Files:**
- Modify: `rpg-api/internal/handlers/dnd5e/v1alpha1/encounter/handler.go`
- Modify: `rpg-api/internal/handlers/dnd5e/v1alpha1/encounter/converters.go`

- [ ] **Step 1: Update proto conversion in converters.go**

Remove/update functions that convert old event shapes to proto. The new events already carry proto-ready `EntityState` and `EncounterState`, so conversion should be simpler.

- [ ] **Step 2: Update handler methods that read from old event fields**

The streaming handler (`StreamEncounterEvents`) and `GetEncounterState` need to use the new proto shapes.

- [ ] **Step 3: Update GetEncounterState to return EncounterState**

```go
func (h *Handler) GetEncounterState(ctx context.Context, req *pb.GetEncounterStateRequest) (*pb.GetEncounterStateResponse, error) {
    // Load encounter data
    // Build EncounterState via entities.BuildEncounterState
    // Return with last_event_id
}
```

- [ ] **Step 4: Fix compilation for entire handler package**

```bash
go build ./internal/handlers/...
```

- [ ] **Step 5: Commit**

```bash
git add internal/handlers/ && git commit -m "refactor: handler layer uses unified entity state protos"
```

### Task 2.9: Fix All Tests

**Files:**
- Modify: test files across `internal/orchestrators/encounter/`, `internal/handlers/`, `internal/repositories/encounters/`

- [ ] **Step 1: Run all tests to see what breaks**

```bash
go test ./... 2>&1 | grep FAIL
```

- [ ] **Step 2: Update test fixtures**

Tests that create `EncounterData` with `Monsters`, `CharacterHP`, `RoomData` need to use the new `Entities` map instead.

- [ ] **Step 3: Update test assertions**

Tests that assert on event fields (`UpdatedRoom`, `UpdatedCharacters`) need to assert on `UpdatedEntities` or `EncounterState` instead.

- [ ] **Step 4: Run full test suite**

```bash
go test ./...
```

- [ ] **Step 5: Run pre-commit**

```bash
make pre-commit
```

- [ ] **Step 6: Commit**

```bash
git add -A && git commit -m "test: update all tests for unified entity state"
```

### Task 2.10: Push and Create PR

- [ ] **Step 1: Run ci-check**

```bash
make ci-check
```

- [ ] **Step 2: Push and create PR**

```bash
git push -u origin <branch>
gh pr create --title "feat: unified entity state - API refactor" --body "..."
```

---

## Phase 3: Web Client Changes (rpg-dnd5e-web)

> Can be developed in parallel with Phase 2 after protos land.
> **Repo:** `rpg-dnd5e-web` at `/home/kirk/personal/rpg-dnd5e-web`
> **Branch from:** fresh main
> **Depends on:** Phase 1 merged + proto package updated

### Task 3.0: Update Proto Package

**Files:**
- Modify: `package.json`

- [ ] **Step 1: Update rpg-api-protos package**

```bash
cd rpg-dnd5e-web && npm install @kirkdiggler/rpg-api-protos@latest
```

- [ ] **Step 2: Verify new types are available**

Check that `EntityState`, `EncounterState`, `CharacterDetails`, `MonsterDetails` exist in the generated TS types.

- [ ] **Step 3: Commit**

```bash
git add package.json package-lock.json && git commit -m "chore: update rpg-api-protos for unified entity state"
```

### Task 3.1: Create useEncounterState Hook

**Files:**
- Create: `src/hooks/useEncounterState.ts`
- Create: `src/hooks/useEncounterState.test.ts`

- [ ] **Step 1: Write tests for the hook**

```typescript
// useEncounterState.test.ts
describe('useEncounterState', () => {
  it('applySnapshot replaces entire state', () => {
    // Given empty state
    // When applySnapshot called with full EncounterState
    // Then all entities, rooms, combat are set
  });

  it('applyEntityUpdates merges by entity ID', () => {
    // Given state with entities A and B
    // When applyEntityUpdates called with updated B
    // Then A is unchanged, B is updated
  });

  it('applyEntityUpdates with new entity adds it', () => {
    // Given state with entity A
    // When applyEntityUpdates called with entity C
    // Then both A and C are in state
  });

  it('applyCombatState updates combat without touching entities', () => {
    // Given state with combat and entities
    // When applyCombatState called
    // Then combat is updated, entities unchanged
  });
});
```

- [ ] **Step 2: Run tests to verify they fail**

```bash
npm run test:run -- --reporter verbose src/hooks/useEncounterState.test.ts
```

- [ ] **Step 3: Implement the hook**

```typescript
// useEncounterState.ts
export interface EncounterStateData {
  encounterId: string;
  dungeonId: string;
  entities: Map<string, EntityState>;
  rooms: Map<string, RoomLayout>;
  currentRoomId: string;
  revealedRoomIds: Set<string>;
  combat: CombatState | null;
  doors: Map<string, DoorInfo>;
  dungeonState: DungeonState;
}

export function useEncounterState() {
  const [state, setState] = useState<EncounterStateData>(createEmptyState());

  const applySnapshot = useCallback((encounterState: EncounterState) => {
    // Convert proto EncounterState to local EncounterStateData
    // Replace entire state
  }, []);

  const applyEntityUpdates = useCallback((updates: EntityState[]) => {
    setState(prev => {
      const newEntities = new Map(prev.entities);
      for (const entity of updates) {
        newEntities.set(entity.entityId, entity);
      }
      return { ...prev, entities: newEntities };
    });
  }, []);

  const applyCombatState = useCallback((combat: CombatState) => {
    setState(prev => ({ ...prev, combat }));
  }, []);

  const reset = useCallback(() => {
    setState(createEmptyState());
  }, []);

  return { state, applySnapshot, applyEntityUpdates, applyCombatState, reset };
}
```

Note: Proto `map` fields generate plain objects in TypeScript. The hook will need to convert `Record<string, EntityState>` from proto into `Map<string, EntityState>` for the local state.

- [ ] **Step 4: Run tests**

```bash
npm run test:run -- --reporter verbose src/hooks/useEncounterState.test.ts
```

- [ ] **Step 5: Commit**

```bash
git add src/hooks/useEncounterState*.ts && git commit -m "feat: add useEncounterState hook with snapshot/delta operations"
```

### Task 3.2: Create Entity Helper Utilities

**Files:**
- Create: `src/utils/entityHelpers.ts`
- Create: `src/utils/entityHelpers.test.ts`

- [ ] **Step 1: Write tests**

```typescript
describe('entityHelpers', () => {
  it('hasCondition returns true when condition present', () => {});
  it('hasCondition returns false when condition absent', () => {});
  it('isDead returns true when DEAD condition present', () => {});
  it('isUnconscious returns true when UNCONSCIOUS condition present', () => {});
  it('getHealthCategory returns correct category based on HP ratio', () => {});
  it('getEntityName returns name from character or monster details', () => {});
});
```

- [ ] **Step 2: Implement helpers**

```typescript
export function hasCondition(entity: EntityState, conditionId: ConditionId): boolean {
  return entity.activeConditions.some(c => c.id === conditionId);
}

export function isDead(entity: EntityState): boolean {
  return hasCondition(entity, ConditionId.DEAD);
}

export function isUnconscious(entity: EntityState): boolean {
  return hasCondition(entity, ConditionId.UNCONSCIOUS);
}

export function getHealthCategory(entity: EntityState): string {
  if (isDead(entity)) return 'dead';
  const ratio = entity.currentHitPoints / entity.maxHitPoints;
  if (ratio >= 1) return 'uninjured';
  if (ratio >= 0.5) return 'injured';
  if (ratio >= 0.25) return 'bloodied';
  return 'near death';
}

export function getEntityName(entity: EntityState): string {
  if (entity.details.case === 'characterDetails') return entity.details.value.name;
  if (entity.details.case === 'monsterDetails') return entity.details.value.name;
  return entity.entityId;
}
```

- [ ] **Step 3: Run tests**

```bash
npm run test:run -- --reporter verbose src/utils/entityHelpers.test.ts
```

- [ ] **Step 4: Commit**

```bash
git add src/utils/entityHelpers*.ts && git commit -m "feat: add entity helper utilities (hasCondition, isDead, getHealthCategory)"
```

### Task 3.3: Rewire LobbyView Event Handlers

**Files:**
- Modify: `src/components/LobbyView.tsx`

This is the biggest task. LobbyView currently has 15+ event handlers each manually syncing 2-4 state containers.

- [ ] **Step 1: Replace fragmented state with useEncounterState**

Remove these useState calls (lines 138-140, 172-174):
```typescript
// REMOVE:
const [combatState, setCombatState] = useState<CombatState | null>(null);
const [monsters, setMonsters] = useState<MonsterCombatState[]>([]);
const [fullCharactersMap, setFullCharactersMap] = useState<Map<string, Character>>(new Map());
```

Add:
```typescript
const { state: encounterState, applySnapshot, applyEntityUpdates, applyCombatState, reset } = useEncounterState();
```

- [ ] **Step 2: Rewrite snapshot event handlers**

For `handleCombatStarted`, `handleRoomRevealed`, `handleStateSync`, `handleCombatResumed`, `handlePlayerReconnected`:

```typescript
const handleCombatStarted = (event: CombatStartedEvent) => {
  applySnapshot(event.encounterState);
  // Handle surprise round monster turns if present
};
```

Each of these collapses from 20-30 lines to 1-3 lines.

- [ ] **Step 3: Rewrite delta event handlers**

For `handleAttackResolved`, `handleMonsterTurnCompleted`, `handleMovementCompleted`, `handleTurnEnded`, `handleFeatureActivated`, `handleActionExecuted`, `handleCombatAbilityActivated`, `handleRestCompleted`, death save events:

**Rule:** For any delta event that includes `combat_state`, always call both `applyEntityUpdates` AND `applyCombatState`. Events with CombatState: MonsterTurnCompleted, MovementCompleted, TurnEnded, ActionExecuted, CombatAbilityActivated.

```typescript
const handleAttackResolved = (event: AttackResolvedEvent) => {
  applyEntityUpdates([event.updatedAttacker, event.updatedTarget].filter(Boolean));
  // Add to combat log
};

const handleMovementCompleted = (event: MovementCompletedEvent) => {
  applyEntityUpdates([event.updatedEntity]);
  applyCombatState(event.combatState);
};
```

- [ ] **Step 4: Remove useDungeonMap for entity tracking**

The `useDungeonMap` hook currently tracks entities. With the unified store, entities come from `encounterState.entities`. The dungeon map hook may still be useful for floor tile generation, but entity tracking moves to `useEncounterState`.

- [ ] **Step 5: Delete mergeCharacterUpdate import and usage**

Remove all calls to `mergeCharacterUpdate` in event handlers. The server now sends complete `EntityState` — no merging needed.

- [ ] **Step 6: Fix TypeScript compilation**

```bash
npx tsc --noEmit
```

Work through all type errors. Components that received `monsters`, `fullCharactersMap` as props now receive data derived from `encounterState.entities`.

- [ ] **Step 7: Commit**

```bash
git add src/components/LobbyView.tsx && git commit -m "refactor: LobbyView uses useEncounterState, remove fragmented state"
```

### Task 3.4: Update BattleMapPanel

**Files:**
- Modify: `src/components/encounter/BattleMapPanel.tsx`

- [ ] **Step 1: Update props to receive entities from encounter state**

Instead of receiving `dungeonMap.entities` + `monsters[]` separately, receive `entities: Map<string, EntityState>` and derive everything from it.

- [ ] **Step 2: Filter dead entities using helper**

```typescript
const aliveEntities = useMemo(() => {
  return Array.from(entities.values()).filter(entity => !isDead(entity));
}, [entities]);
```

- [ ] **Step 3: Map EntityState to HexEntity props**

```typescript
const renderableEntities = useMemo(() => {
  return aliveEntities
    .filter(e => e.roomId === currentRoomId)
    .map(entity => ({
      entityId: entity.entityId,
      name: getEntityName(entity),
      position: entity.position,
      type: entity.entityType === EntityType.CHARACTER ? 'player' : 'monster',
      entity,  // pass full EntityState for HexEntity rendering
    }));
}, [aliveEntities, currentRoomId]);
```

- [ ] **Step 4: Commit**

```bash
git add src/components/encounter/BattleMapPanel.tsx && git commit -m "refactor: BattleMapPanel reads from unified entity state"
```

### Task 3.5: Update HoverInfoPanel

**Files:**
- Modify: `src/components/combat-v2/panels/HoverInfoPanel.tsx`

- [ ] **Step 1: Update props to receive EntityState**

Instead of separate `characters` and `monsters` maps, receive `entities: Map<string, EntityState>`.

- [ ] **Step 2: Update health display to use getHealthCategory**

```typescript
const healthCategory = getHealthCategory(entity);
```

This directly fixes bug #357 — the hover panel now reads HP from the same `EntityState` object that the renderer uses.

- [ ] **Step 3: Commit**

```bash
git add src/components/combat-v2/panels/HoverInfoPanel.tsx && git commit -m "fix: hover panel reads HP from unified entity state (fixes #357)"
```

### Task 3.6: Update Remaining Components

**Files:**
- Modify: `src/components/hex-grid/HexEntity.tsx`
- Modify: `src/components/combat-v2/panels/CombatPanel.tsx`
- Modify: Any other components that read from `monsters[]` or `fullCharactersMap`

- [ ] **Step 1: Update HexEntity to receive EntityState**

Character rendering reads from `entity.details.characterDetails` (appearance, equipment, class). Monster rendering reads from `entity.details.monsterDetails`.

- [ ] **Step 2: Update CombatPanel**

Combat panel reads current entity's details + available abilities from encounter state.

- [ ] **Step 3: Update initiative tracker if it reads HP**

Should read from `encounterState.entities` by entity ID.

- [ ] **Step 4: Fix all remaining TypeScript errors**

```bash
npx tsc --noEmit
```

- [ ] **Step 5: Commit**

```bash
git add src/components/ && git commit -m "refactor: all components read from unified entity state"
```

### Task 3.7: Delete Dead Code

**Files:**
- Delete: `src/utils/characterMerge.ts`
- Delete: `src/utils/characterMerge.test.ts`
- Modify: `src/hooks/useDungeonMap.ts` (remove entity tracking if no longer needed)

- [ ] **Step 1: Delete mergeCharacterUpdate**

```bash
rm src/utils/characterMerge.ts src/utils/characterMerge.test.ts
```

- [ ] **Step 2: Verify no remaining imports**

```bash
grep -r "characterMerge\|mergeCharacterUpdate" src/
```

Should return nothing.

- [ ] **Step 3: Clean up useDungeonMap**

If entity tracking has been fully moved to `useEncounterState`, remove the `entities` field from `DungeonMapState` and the `updateEntities` method. Keep floor tile and wall accumulation if still used for rendering.

- [ ] **Step 4: Commit**

```bash
git add -A && git commit -m "chore: delete mergeCharacterUpdate and dead entity state code"
```

### Task 3.8: Run Full Test Suite and CI

- [ ] **Step 1: Run all tests**

```bash
npm run test:run
```

- [ ] **Step 2: Fix any failing tests**

Update test fixtures that reference old state shapes (`monsters[]`, `fullCharactersMap`).

- [ ] **Step 3: Run ci-check**

```bash
npm run ci-check
```

- [ ] **Step 4: Commit test fixes**

```bash
git add -A && git commit -m "test: update tests for unified entity state"
```

### Task 3.9: Push and Create PR

- [ ] **Step 1: Push and create PR**

```bash
git push -u origin <branch>
gh pr create --title "feat: unified entity state - web client refactor" --body "Fixes #357 #358 #347"
```

---

## Deployment

**In-flight data:** Any in-progress encounters in Redis will be invalid after deploy. The encounter repository is in-memory, so a server restart clears it. Ensure no active game sessions during deployment, or add a version field to EncounterData for graceful rejection of old-format data.

All three PRs (protos, API, web) must merge and deploy together since the proto changes are breaking. Coordinate:

1. Merge proto PR first
2. Update dependencies in API and web
3. Merge API and web PRs
4. Deploy API and web together
