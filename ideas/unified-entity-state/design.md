# Unified Entity State — Design Spec

**Date:** 2026-03-28
**Status:** Approved
**Milestone:** 4-class dungeon
**Blocked issues:** rpg-dnd5e-web#358, rpg-dnd5e-web#357, rpg-dnd5e-web#347

## Problem

Entity data is fragmented across multiple state containers at every layer:

**Web client** — LobbyView holds 4+ independent `useState` calls:
- `dungeonMap.entities` — spatial positions (EntityPlacement)
- `monsters[]` — monster HP and type (MonsterCombatState)
- `fullCharactersMap` — character data with visual fields (Character)
- `combatState` — turn order, action economy

Every event handler manually syncs across these. They drift, causing dead monsters still rendering (#358), hover panels showing stale HP (#357), and HP bars not updating after healing (#347).

**API** — `EncounterData` stores entity state in 4 separate structures:
- `Monsters` — `[]*monster.Data`
- `CharacterHP` — `map[string]int` (API manually doing damage math — violates toolkit boundary)
- `RoomData` — `spatial.RoomData` (entity positions)
- `InitiativeData` — `*initiative.TrackerData`

**Events** — Each event type has bespoke fields for entity updates (`updated_room`, `updated_characters`, `monsters`, `target_hp`) with no consistent pattern.

## Solution

A single `EntityState` message that carries everything about an entity (position, HP, conditions, visual data). A single `EncounterState` that holds all entities, rooms, and combat state. Consistent at every layer: proto, API, web client.

## Proto Design

### EntityState

One representation for any entity in the encounter:

```protobuf
message EntityState {
  string entity_id = 1;
  EntityType entity_type = 2;
  string room_id = 3;

  // Spatial
  Position position = 4;
  EntitySize size = 5;
  bool blocks_movement = 6;
  bool blocks_line_of_sight = 7;

  // Combat (shared across all combatant types)
  int32 current_hit_points = 8;
  int32 max_hit_points = 9;
  repeated Condition active_conditions = 10;
  DeathSaveProgress death_saves = 11;

  // Type-specific rendering data
  oneof details {
    CharacterDetails character = 20;
    MonsterDetails monster = 21;
    ObstacleDetails obstacle = 22;
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

### EncounterState

The complete picture of everything happening in the dungeon:

```protobuf
message EncounterState {
  string encounter_id = 1;
  string dungeon_id = 2;

  // All entities across all rooms, keyed by entity_id
  map<string, EntityState> entities = 3;

  // Room layouts (spatial only — tiles, walls, doors, origin)
  map<string, RoomLayout> rooms = 4;
  string current_room_id = 5;
  repeated string revealed_room_ids = 6;

  // Combat
  CombatState combat = 7;

  // Dungeon progress
  DungeonState dungeon_state = 8;
  int32 rooms_cleared = 9;

  // Doors (cross-room connections)
  map<string, DoorInfo> doors = 10;
}

message RoomLayout {
  string id = 1;
  RoomType type = 2;
  int32 width = 3;
  int32 height = 4;
  GridType grid_type = 5;
  repeated Wall walls = 6;
  Position origin = 7;
}
```

### Key decisions

- `CharacterDetails` carries only what rendering needs — not the full character sheet. The `Character` message still exists for character creation/sheet views.
- `EntityState` owns HP for both characters and monsters. No more separate tracking.
- **No boolean flags for entity status.** Dead, unconscious, raging, dodging, etc. are all `Condition` entries in `active_conditions`. The toolkit already models these as conditions. The client derives rendering behavior from conditions (e.g., `hasCondition(entity, DEAD)` to remove from board). No dual representation.
- `RoomLayout` is spatial-only — entities live in the flat `entities` map with a `room_id` field, not nested inside rooms.
- Obstacles are entities too — consistent rendering path.

## Event Design

Events carry `EntityState` messages directly instead of bespoke partial fields.

### Pattern rules

- **Snapshot events** (CombatStarted, RoomRevealed, StateSync) send the full `EncounterState`.
- **Delta events** (Attack, Movement, TurnEnded) send only the affected `EntityState` entries.
- The client applies deltas by replacing entries in its local entities map by ID — one operation, same for every event.
- No more `updated_room` field — if an entity moved, its `EntityState` has the new position. Room layouts are static.

### Complete event catalog

Every event that touches entity or combat state, classified as snapshot or delta:

| Event | Type | Entity payload |
|-------|------|----------------|
| CombatStartedEvent | snapshot | `EncounterState` |
| RoomRevealedEvent | snapshot | `EncounterState` |
| StateSync (GetEncounterState) | snapshot | `EncounterState` |
| CombatResumedEvent | snapshot | `EncounterState` (reconnecting clients rebuild from this) |
| PlayerReconnectedEvent | snapshot | `EncounterState` (so reconnecting player gets full state) |
| AttackResolvedEvent | delta | `updated_attacker`, `updated_target` |
| MonsterTurnCompletedEvent | delta | `repeated updated_entities` |
| MovementCompletedEvent | delta | `updated_entity` |
| TurnEndedEvent | delta | `repeated updated_entities` |
| FeatureActivatedEvent | delta | `updated_entity` |
| ActionExecutedEvent | delta | `repeated updated_entities` |
| CombatAbilityActivatedEvent | delta | `updated_entity` (conditions may change, e.g. Rage) |
| DeathSaveRolledEvent | delta | `updated_entity` (HP, is_unconscious, is_dead may change) |
| CharacterUnconsciousEvent | delta | `updated_entity` |
| CharacterDiedEvent | delta | `updated_entity` |
| CharacterStabilizedEvent | delta | `updated_entity` |
| RestCompletedEvent | delta | `repeated updated_entities` |
| CombatEndedEvent | snapshot | `EncounterState` (final state after combat) |
| DungeonVictoryEvent | snapshot | `EncounterState` |
| DungeonFailureEvent | snapshot | `EncounterState` |
| PlayerJoinedEvent | none | lobby only, no entity state |
| PlayerLeftEvent | none | lobby only |
| CombatPausedEvent | none | informational only |
| PlayerDisconnectedEvent | none | informational only |

### Updated event payloads

```protobuf
// --- Snapshot events ---

message CombatStartedEvent {
  string dungeon_id = 1;
  EncounterState encounter_state = 2;
  repeated MonsterTurnResult monster_turns = 3;  // surprise round pre-actions
}

message RoomRevealedEvent {
  string connection_id = 1;
  EncounterState encounter_state = 2;
}

message CombatEndedEvent {
  EncounterResult result = 1;
  EncounterState encounter_state = 2;  // final state
}

message CombatResumedEvent {
  EncounterState encounter_state = 1;  // full rebuild for reconnected clients
}

message PlayerReconnectedEvent {
  string player_id = 1;
  PartyMember member = 2;
  EncounterState encounter_state = 3;
}

message DungeonVictoryEvent {
  string dungeon_id = 1;
  EncounterState encounter_state = 2;
}

message DungeonFailureEvent {
  string dungeon_id = 1;
  EncounterState encounter_state = 2;
}

// --- Delta events ---

message AttackResolvedEvent {
  string attacker_id = 1;
  string target_id = 2;
  AttackResult result = 3;
  EntityState updated_attacker = 4;
  EntityState updated_target = 5;
}

message MonsterTurnCompletedEvent {
  MonsterTurnResult monster_turn = 1;
  repeated EntityState updated_entities = 2;
  CombatState combat_state = 3;  // turn advances after monster acts
}

message MovementCompletedEvent {
  string entity_id = 1;
  repeated Position path = 2;
  EntityState updated_entity = 3;
  CombatState combat_state = 4;  // movement consumes ActionEconomy.movement_remaining
}

message TurnEndedEvent {
  CombatState combat_state = 1;
  repeated EntityState updated_entities = 2;
}

message FeatureActivatedEvent {
  string entity_id = 1;
  string feature_id = 2;
  string message = 3;
  EntityState updated_entity = 4;
}

message ActionExecutedEvent {
  string entity_id = 1;
  ActionId action_id = 2;
  oneof result {
    AttackResult strike_result = 3;
    MoveResult move_result = 7;
  }
  repeated EntityState updated_entities = 5;
  CombatState combat_state = 6;
  GrantedAction granted_action = 8;  // e.g. off-hand strike after main attack
}

message CombatAbilityActivatedEvent {
  string entity_id = 1;
  CombatAbilityId ability_id = 2;
  EntityState updated_entity = 3;
  CombatState combat_state = 4;
}

message DeathSaveRolledEvent {
  string character_id = 1;
  int32 roll = 2;
  bool is_success = 3;
  bool is_critical_fail = 4;
  bool is_critical_success = 5;
  EntityState updated_entity = 6;
}

message CharacterUnconsciousEvent {
  string character_id = 1;
  string source_entity_id = 2;
  EntityState updated_entity = 3;
}

message CharacterDiedEvent {
  string character_id = 1;
  EntityState updated_entity = 2;
}

message CharacterStabilizedEvent {
  string character_id = 1;
  EntityState updated_entity = 2;
}

message RestCompletedEvent {
  RestType rest_type = 1;
  repeated EntityState updated_entities = 2;
}
```

### What this eliminates

- `mergeCharacterUpdate()` — `EntityState` is always complete, not partial.
- Cross-referencing `monsters[]` with `dungeonMap.entities` — one map.
- Separate HP tracking anywhere — HP is on the `EntityState`.
- `updated_room` on every event — room layouts are static, entity positions are on entities.

## Server-Side Design (rpg-api)

### The foundational pattern

The API must use the toolkit as designed:

1. `LoadFromData()` — hydrate toolkit objects from stored data
2. Attach to event bus
3. Fire the action (attack, move, feature activation)
4. Check dirty flags
5. `ToData()` — serialize back
6. Convert to `EntityState` — persist and publish

**The API never does damage math, HP subtraction, or condition tracking.** That's the toolkit's job via its event-driven internals.

### EncounterData changes

```
Current (API tracks state itself):
  EncounterData.Monsters          []*monster.Data
  EncounterData.CharacterHP       map[string]int      ← violates toolkit boundary
  EncounterData.RoomData          spatial.RoomData

New (API stores toolkit data, doesn't interpret it):
  EncounterData.Entities          map[string]*EntityStateData
  EncounterData.Rooms             map[string]*RoomLayout
  EncounterData.Combat            *CombatState
```

### EntityStateData — the storage type

`EntityStateData` is a Go struct (not a proto message). It holds the toolkit's serialized data alongside metadata the API needs for orchestration:

```go
type EntityStateData struct {
    EntityID   string          // stable entity ID
    EntityType string          // "character", "monster", "obstacle"
    RoomID     string          // which room this entity is in
    ToolkitData interface{}    // character.Data or monster.Data — opaque to the API
    Position   spatial.CubeCoord
    Size       int
}
```

**The conversion boundary:** One function converts from storage to proto:

```go
func toEntityState(esd *EntityStateData) *pb.EntityState
```

This is the **single point** where toolkit data is projected into the proto `EntityState`. It reads HP, conditions, appearance, etc. from the toolkit data to populate the proto fields. The API calls this only when building events or responding to queries — never to make decisions.

### ActionEconomy placement

`ActionEconomy` is per-turn ephemeral state, not per-entity persistent state. It lives on `CombatState`, not on `EntityState`:

```protobuf
message CombatState {
  // ... existing fields (round, turn_order, active_index) ...
  ActionEconomy current_turn_economy = 10;  // movement, actions, bonus remaining
}
```

Delta events that change action economy include `CombatState` (ActionExecutedEvent, CombatAbilityActivatedEvent). MovementCompletedEvent already includes the updated combat state through the turn. The client reads `combat.currentTurnEconomy` for UI display.

### Orchestrator pattern for every combat action

```
// 1. Load
for each involved entity:
    obj := toolkit.LoadFromData(encounter.Entities[id].ToolkitData)
    bus.Attach(obj)

// 2. Execute
result := bus.Fire(action)  // toolkit handles all mutations

// 3. Harvest
for each dirty object:
    data := obj.ToData()
    encounter.Entities[id].ToolkitData = data        // update storage
    encounter.Entities[id].Position = obj.Position()  // sync spatial

// 4. Convert + persist + publish
entityStates := toEntityStates(affectedIDs, encounter.Entities)  // single conversion point
repo.Save(encounter)
publisher.Publish(event with entityStates)
```

### What gets fixed

- `monster_turns.go` manually doing `CharacterHP[id] - damage` — gone. Toolkit character objects handle their own damage via bus.
- Separate `CharacterHP` map — gone. HP lives in the toolkit data inside `EntityState`.
- The API never interprets HP, conditions, or damage — it just loads, fires, harvests, saves.

### What stays the same

- Toolkit boundary rule — toolkit owns rules, API orchestrates.
- `Character` service/repo — full character sheets are still their own thing.
- Redis pub/sub event broadcasting — same mechanism, cleaner payloads.
- Input/Output types on every function.

## Web Client Design (rpg-dnd5e-web)

### Single entity store

One `useEncounterState` hook replaces all fragmented state:

```typescript
interface EncounterState {
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
```

### Two operations cover every event

```typescript
// Snapshot events (CombatStarted, RoomRevealed, StateSync)
function applySnapshot(state: EncounterState): void {
  // Replace entire state
}

// Delta events (AttackResolved, MovementCompleted, TurnEnded, etc.)
function applyEntityUpdates(updates: EntityState[]): void {
  for (const entity of updates) {
    state.entities.set(entity.entityId, entity);
  }
}
```

No merge logic. The server sends complete `EntityState` entries — the client just sets them.

### How the three bugs vanish

- **#358 (dead monster not removed):** Rendering reads `entities` map, checks `hasCondition(entity, DEAD)`. One place, one check.
- **#357 (hover shows uninjured):** Hover panel reads `entities.get(hoveredId).currentHitPoints`. Same object the renderer uses.
- **#347 (HP bar doesn't update):** HP lives on the `EntityState` that React is already watching. Update the entity, UI re-renders.

### Component data flow

```
useEncounterState (holds EncounterState)
  │
  ├── BattleMapPanel
  │     reads entities map → filters by room_id → renders positions
  │     DEAD condition check is on the same object
  │
  ├── HexEntity
  │     reads entity.details.character or entity.details.monster
  │     HP, conditions, appearance — all on one object
  │
  ├── HoverInfoPanel
  │     reads entities.get(hoveredId) — HP, conditions, name, type
  │
  ├── CombatPanel
  │     reads combat state + current entity's details
  │
  └── InitiativeTracker
        reads combat.turnOrder + entities map for HP/status display
```

### What gets deleted

- `monsters` useState array
- `fullCharactersMap` useState
- `mergeCharacterUpdate()` utility
- All per-event manual state sync code in LobbyView
- Dead monster filtering in BattleMapPanel

LobbyView shrinks to: set up the stream, route events to `applySnapshot` or `applyEntityUpdates`, render child components that read from one store.

## Testing Strategy

### Proto layer (rpg-api-protos)

- `EntityState` serializes/deserializes correctly with each `oneof` variant.
- `EncounterState` round-trips through proto encoding.

### API layer (rpg-api)

- **Orchestrator tests:** For each combat action, verify Load → Fire → Harvest → Save cycle produces correct `EntityState` entries.
- **Event construction tests:** Snapshot events carry full `EncounterState`, delta events carry only affected entities.

### Web layer (rpg-dnd5e-web)

- **`useEncounterState` tests:** `applySnapshot` replaces state fully. `applyEntityUpdates` merges by entity ID without losing unaffected entities.
- **Rendering integration:** Entity with `DEAD` condition doesn't render. HP changes propagate to hover panel.

### Bug regression tests

- **#358:** `ActionExecutedEvent` kills monster → `DEAD` condition in `active_conditions` → not in rendered entity list.
- **#357:** `AttackResolvedEvent` with damage → `entities.get(monsterId).currentHitPoints` reflects damage → hover reads same value.
- **#347:** `FeatureActivatedEvent` for Second Wind → entity HP updated → HP bar receives new value.

## Migration & Rollout

**Clean cut, no backwards compatibility layer.** This is a Discord Activity — we control all clients.

### Order of operations

0. **Spike test (rpg-toolkit)** — Verify character-on-bus damage flow works end-to-end. If not, fix toolkit first. See Risk section.
1. **rpg-api-protos** — Add `EntityState`, `EncounterState`, `RoomLayout`, `CharacterDetails`, `MonsterDetails`. Update all event messages per the event catalog. Remove deprecated fields. Single PR.
2. **rpg-api** — Refactor orchestrator to Load → Fire → Harvest → Save pattern. Remove `CharacterHP` map and separate `Monsters` array. Convert event construction to use `EntityState`.
3. **rpg-dnd5e-web** — Replace fragmented state with `useEncounterState`. Update event handlers. Delete merge utilities and per-event sync code.

API and web can be developed in parallel after protos land. All three merge together for coordinated deploy.

### Risk: character-on-bus wiring

The `CharacterHP` manual subtraction in `monster_turns.go` means character objects may not be fully wired to the toolkit event bus for receiving damage today. The entire Load → Fire → Harvest → Save pattern depends on this working.

**Required spike test before implementation begins:**

In rpg-toolkit, write a test that does:
1. `LoadFromData(characterData)` + `LoadFromData(monsterData)` on same bus
2. Fire an attack action from monster → character
3. Verify `character.ToData()` reflects the damage (HP decreased)

If this works, the toolkit already supports the pattern and the API just needs to use it. If it fails, toolkit work is needed first and becomes step 0 in the migration order — contradicting the "no toolkit changes" claim.

This spike is a prerequisite for implementation planning.
