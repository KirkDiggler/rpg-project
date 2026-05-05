# Phase 1 — Protos (rpg-api-protos)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Define the complete `dnd5e.api.v1alpha2` proto package — messages, enums, service, and request/response types — so that Phase 2 can begin building handlers and orchestrators against it.

**Architecture:** Three new proto files under `dnd5e/api/v1alpha2/encounter/`:
- `types.proto` — all data shapes (Ref, Position, HitPoints, enums, Hex, Wall, Zone, StatusEffect, Space, Entity + typed data, Encounter, TurnState, ActionEconomy, AvailableAction, MovementInterruption, InputRequired)
- `events.proto` — `EncounterEvent` envelope + 23 event payloads
- `service.proto` — `EncounterService` definition + RPC request/response messages

This subdirectory layout (`encounter/` as feature folder, with `types`/`events`/`service` files) sets a convention for future v2 services. Three-file split avoids the encounter↔events circular import that would otherwise break `buf lint`.

buf v2 toolchain. Generates Go (under `gen/go/dnd5e/api/v1alpha2/encounter/`) and TypeScript (under `gen/ts/dnd5e/api/v1alpha2/encounter/`). Mocks generated for the gRPC service.

**Tech Stack:** Protocol Buffers v3, buf v2, Go (codegen), TypeScript (codegen), gomock (service mocks).

**Spec:** [`../../design.md`](../../design.md) — sections 3 (Wire Shapes), 4 (RPCs), 5 (Streaming) are normative.

**Repo:** `rpg-api-protos` at `/home/kirk/personal/rpg-api-protos`.

**Branch:** `feat/v1alpha2-encounter-protos` (cut from `main`).

**Conventions** (from `rpg-api-protos/CLAUDE.md`):
- Always `buf format -w` before commit
- Feature branch only — never push to `generated`
- **`gen/` is gitignored.** Feature-branch commits include only `.proto` files and any Makefile/source changes. CI publishes generated code to the `generated` branch on merge to `main`. Do not `git add gen/`.
- snake_case fields, PascalCase messages, UPPER_SNAKE_CASE enum values (with type prefix)
- `option go_package = "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/v1alpha2/encounter;encounterpb";` on every file
- `option java_package = "com.kirkdiggler.rpg.api.dnd5e.v1alpha2.encounter";`

**Granularity note:** Tasks below are **phase-aligned**, not 2–5-minute TDD micro-steps. Protos are spec, not behavior; per-message commits would inflate review surface without improving safety. Each task ends with a passing `make test` / `make compile-go` / `make compile-ts` and a single commit per logical chunk.

---

## File Map

```
dnd5e/api/v1alpha2/encounter/
  types.proto       ← Task 1 (large; all data shapes)
  events.proto      ← Task 2 (depends on types.proto)
  service.proto     ← Task 3 (depends on types.proto + events.proto)
```

```
Makefile                 ← Task 4: add v1alpha2 mock entry + make compile-go idempotent
```

---

## Task 1: types.proto — all data shapes

**Files:**
- Create: `dnd5e/api/v1alpha2/encounter/types.proto`

This is the largest task; produces ~25 messages and 5 enums in a single file. No imports from other v1alpha2 files (this is the leaf node of the v2 import graph).

- [ ] **Step 1: Create branch from latest main**

```bash
cd /home/kirk/personal/rpg-api-protos
git checkout main
git pull origin main
git checkout -b feat/v1alpha2-encounter-protos
mkdir -p dnd5e/api/v1alpha2/encounter
```

- [ ] **Step 2: Write `types.proto`**

```protobuf
syntax = "proto3";

package dnd5e.api.v1alpha2.encounter;

option go_package = "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/v1alpha2/encounter;encounterpb";
option java_multiple_files = true;
option java_package = "com.kirkdiggler.rpg.api.dnd5e.v1alpha2.encounter";

// =============================================================================
// Common primitives
// =============================================================================

// Ref is a typed reference to a piece of toolkit content.
// module/type/id triple disambiguates across rulebooks and prevents id collisions.
// Used everywhere the wire identifies a typed thing (action, monster, feature, condition,
// damage type, item, ...). NOT used for runtime instance ids (encounter_id, entity_id) —
// those stay strings. The `type` field is open-ended; toolkit grows the vocabulary.
message Ref {
  string module = 1;  // "dnd5e", future: "artificer", "homebrew-x"
  string type = 2;    // "monster", "feature", "condition", "action", "spell", "item", "damage", ...
  string id = 3;      // "goblin", "rage", "dodging", "attack"
}

// Position uses hex cube coordinates with the invariant x + y + z == 0.
// Coordinates are dungeon-absolute on the wire — no room-local coords ever.
message Position {
  int32 x = 1;
  int32 y = 2;
  int32 z = 3;
}

// HitPoints is the destructibility shape carried on Entity when the entity has HP.
// Non-destructible entities omit the optional Entity.hp field entirely.
message HitPoints {
  int32 current = 1;
  int32 max = 2;
  int32 temp = 3;
}

// =============================================================================
// Enums
// =============================================================================

// EncounterMode determines the rule-set in effect for the encounter.
// Mode flips happen within a single encounter; the encounter persists across the transition.
enum EncounterMode {
  ENCOUNTER_MODE_UNSPECIFIED = 0;
  ENCOUNTER_MODE_FREE_ROAM = 1;
  ENCOUNTER_MODE_TURN_BASED = 2;
}

// EntityType is the discriminator for what kind of thing an Entity is.
// Used for client-side dispatch (rendering, default interactions). Toolkit content
// identification still uses Ref inside the type-specific data messages.
enum EntityType {
  ENTITY_TYPE_UNSPECIFIED = 0;
  ENTITY_TYPE_CHARACTER = 1;
  ENTITY_TYPE_MONSTER = 2;
  ENTITY_TYPE_OBSTACLE = 3;
  ENTITY_TYPE_TRAP = 4;
  ENTITY_TYPE_PROP = 5;
  ENTITY_TYPE_NPC = 6;
}

// TerrainType describes per-hex movement properties.
// Walkability per entity (e.g. flying over VOID, swimmer through WATER) is server-evaluated.
enum TerrainType {
  TERRAIN_TYPE_UNSPECIFIED = 0;
  TERRAIN_TYPE_FLOOR = 1;
  TERRAIN_TYPE_ROUGH = 2;
  TERRAIN_TYPE_DIFFICULT = 3;
  TERRAIN_TYPE_VOID = 4;
  TERRAIN_TYPE_WATER = 5;
}

// WallKind describes what a wall segment is.
// DOOR_CLOSED transitions to DOOR_OPEN via the DoorOpened event; the segment stays
// in the wall list with a different kind so closing it is a state change, not a removal.
enum WallKind {
  WALL_KIND_UNSPECIFIED = 0;
  WALL_KIND_SOLID = 1;
  WALL_KIND_DOOR_CLOSED = 2;
  WALL_KIND_DOOR_OPEN = 3;
  WALL_KIND_WINDOW = 4;
}

// TrapState is carried on TrapData entities so visible state matches what the player
// should see.
enum TrapState {
  TRAP_STATE_UNSPECIFIED = 0;
  TRAP_STATE_ARMED = 1;
  TRAP_STATE_TRIGGERED = 2;
  TRAP_STATE_DISARMED = 3;
}

// =============================================================================
// Space — flat hex map with optional zones
// =============================================================================

// Hex is one cell of the player's view of the map.
// Sticky once revealed: stays in Space.hexes for the rest of the campaign for that character.
message Hex {
  Position position = 1;
  TerrainType terrain = 2;
  string zone_id = 3;  // optional zone membership ("" if none)
}

// Wall is a barrier between two adjacent hexes.
// Doors are walls with a DOOR_* kind; opening a door changes the kind, not the wall's
// existence.
message Wall {
  Position from = 1;
  Position to = 2;
  WallKind kind = 3;
}

// Zone is optional metadata attached to a region of hexes.
// Used for ambient cues (lighting, music), narration moments ("you entered the throne
// room"), and scripted triggers. Zones do NOT structure the map for movement or rendering
// — that's all flat-hex.
message Zone {
  string id = 1;
  string name = 2;
  // Future ambient hooks (lighting profile ref, music cue ref, trigger refs) will be
  // added with new field numbers as needed.
}

// StatusEffect is the wire-shape of toolkit's core.Effect (called Condition in the dnd5e
// rulebook). Carries display payload only; the reactive mechanism stays server-side.
message StatusEffect {
  Ref source = 1;                  // {module:"dnd5e", type:"condition", id:"poisoned"}
  string display_name = 2;
  string icon_hint = 3;            // UI category — actual asset path resolved web-side
  optional int32 duration_rounds = 4;
}

// Space is the player's current view of the world.
// Continuous flat hex map — no rooms on the wire. Zones are optional metadata regions.
// Visibility:
//   - hexes and walls are sticky (explored geometry persists per character across the
//     campaign)
//   - entities are real-time LOS (server filters per player per event)
message Space {
  repeated Hex hexes = 1;          // explored, sticky
  repeated Wall walls = 2;         // explored
  repeated Entity entities = 3;    // currently visible (LOS-filtered)
  repeated Zone zones = 4;         // optional metadata
}

// =============================================================================
// Entity — common shape + typed data oneof
// =============================================================================

// Entity is anything that can occupy a position in the space.
// Common shape (id, position, type, display_name, optional hp, status_effects) is what
// every entity carries because every entity is destructible-or-not and has-or-doesn't-have
// visible statuses. The oneof carries the type-specific rich data.
message Entity {
  string id = 1;                              // runtime instance id; not a Ref
  Position position = 2;
  EntityType type = 3;
  string display_name = 4;
  optional HitPoints hp = 5;                  // present only on destructibles
  repeated StatusEffect status_effects = 6;   // visible only

  oneof data {
    CharacterData character = 10;
    MonsterData monster = 11;
    ObstacleData obstacle = 12;
    TrapData trap = 13;
    PropData prop = 14;
    NpcData npc = 15;
  }
}

// CharacterData — player-controlled character.
message CharacterData {
  Ref class_ref = 1;        // {module:"dnd5e", type:"class", id:"fighter"}
  Ref race_ref = 2;
  string player_id = 3;     // who controls this character
  // Additional fields (level, AC, equipment) added in later phases as orchestrator needs.
}

// MonsterData — server-controlled monster.
message MonsterData {
  Ref monster_ref = 1;      // {module:"dnd5e", type:"monster", id:"goblin"}
}

// ObstacleData — boulder, table, statue, etc.
message ObstacleData {
  Ref obstacle_ref = 1;
  bool blocks_movement = 2;
  bool blocks_line_of_sight = 3;
}

// TrapData — pit traps, dart traps, magic traps.
// appears_as is the illusion mechanic: undetected traps report a different EntityType
// to players who haven't perceived them. Server filters per player.
message TrapData {
  Ref trap_ref = 1;         // {module:"dnd5e", type:"trap", id:"pit-trap"}
  TrapState state = 2;
  EntityType appears_as = 3;  // FLOOR/PROP for undetected; same as `type` once perceived
}

// PropData — chest, lever, lootable, decoration.
message PropData {
  Ref prop_ref = 1;
  bool interactable = 2;
}

// NpcData — non-hostile, non-monster characters (shopkeeper, quest-giver).
message NpcData {
  Ref npc_ref = 1;
  bool hostile = 2;         // can flip during encounter (e.g. betrayal)
}

// =============================================================================
// Encounter — session, mode, turn state, action economy
// =============================================================================

// Encounter is the session the player is currently inside.
// One encounter persists across mode flips — combat starting mid-exploration does not tear
// down the world; it just changes the rule-set.
message Encounter {
  string id = 1;                     // runtime instance id
  EncounterMode mode = 2;
  Space space = 3;
  optional TurnState turn_state = 4; // present only when mode == TURN_BASED
}

// TurnState is the action-economy and initiative payload, present only in TURN_BASED mode.
message TurnState {
  repeated string initiative_order = 1;  // entity ids in order of who acts when
  string active_entity_id = 2;
  int32 round = 3;                       // 1-based; increments when initiative_order wraps
  ActionEconomy economy = 4;
  repeated AvailableAction available_actions = 5;
}

// ActionEconomy mirrors the toolkit's two-level model from
// rulebooks/dnd5e/combat/action_economy.go.
//
// Primary resources (action, bonus_action, reaction, movement) are universal-ish across
// turn-based rulebooks. Capacity is the rulebook-specific bag — toolkit-defined keys like
// "attacks", "off_hand_attacks", "flurry_strikes". New capacity types add new keys without
// churning the proto.
message ActionEconomy {
  int32 actions_remaining = 1;
  int32 bonus_actions_remaining = 2;
  int32 reactions_remaining = 3;
  int32 movement_remaining = 4;          // in feet (toolkit-native unit)

  // Keys are toolkit-defined: "attacks", "off_hand_attacks", "flurry_strikes"
  map<string, int32> capacities = 10;
}

// AvailableAction is one entry in the server-pushed list of actions the active entity can
// take right now. Web does not compute availability — it renders the list and disables
// buttons whose `available` is false (showing `unavailable_reason` as tooltip).
message AvailableAction {
  Ref ref = 1;                           // {module:"dnd5e", type:"action", id:"attack"}
  string display_name = 2;
  bool available = 3;
  string unavailable_reason = 4;         // user-facing: "no movement", "out of range", ...
}

// MovementInterruption is attached to an EntityMoved event when the path was cut short.
// Used so the client can animate the partial movement and surface narration.
message MovementInterruption {
  enum Reason {
    REASON_UNSPECIFIED = 0;
    REASON_OUT_OF_MOVEMENT = 1;
    REASON_TRAP_TRIGGERED = 2;
    REASON_OPPORTUNITY_ATTACK = 3;
    REASON_DIFFICULT_TERRAIN = 4;       // hidden until perceived
    REASON_BLOCKED_BY_ENTITY = 5;
    REASON_BLOCKED_BY_TERRAIN = 6;      // unwalkable hex on path
  }
  Reason reason = 1;
  string description = 2;
  optional Ref triggered_by = 3;        // which trap, feature, ability caused it
}

// =============================================================================
// Caller-private follow-up payloads
// =============================================================================

// InputRequired is the caller-private follow-up payload returned in InteractResponse or
// TakeActionResponse when the operation needs more input from the requester (skill check,
// dialogue choice). Server tracks the pending prompt as encounter state; SubmitCheck
// implicitly resolves it. No correlation id needed on the wire.
message InputRequired {
  oneof kind {
    SkillCheckPrompt skill_check = 1;
    DialogueChoice dialogue = 2;
    TargetSelect target_select = 3;
  }
}

message SkillCheckPrompt {
  int32 dc = 1;
  string ability = 2;             // 5e three-letter codes: "STR", "DEX", "INT", ...
  optional Ref tool = 3;          // {module:"dnd5e", type:"item", id:"thieves-tools"}
}

message DialogueChoice {
  repeated DialogueOption options = 1;
}

message DialogueOption {
  string id = 1;                  // option id the client echoes back via Interact again
  string text = 2;
}

message TargetSelect {
  repeated string candidate_entity_ids = 1;
}
```

- [ ] **Step 3: Format**

```bash
buf format -w dnd5e/api/v1alpha2/encounter/types.proto
```

- [ ] **Step 4: Lint**

```bash
buf lint
```

Expected: passes.

- [ ] **Step 5: Generate**

```bash
buf generate
```

Expected: produces `gen/go/dnd5e/api/v1alpha2/encounter/types.pb.go` and `gen/ts/dnd5e/api/v1alpha2/encounter/types_pb.ts`. Verify both exist:

```bash
ls gen/go/dnd5e/api/v1alpha2/encounter/types.pb.go
ls gen/ts/dnd5e/api/v1alpha2/encounter/types_pb.ts
```

- [ ] **Step 6: Compile-check Go**

```bash
make compile-go
```

Expected: clean build (creates `gen/go/go.mod` if missing, runs `go build ./...`).

- [ ] **Step 7: Compile-check TypeScript**

```bash
make compile-ts
```

Expected: `npx tsc --noEmit` exits 0.

- [ ] **Step 8: Commit**

```bash
git add dnd5e/api/v1alpha2/encounter/types.proto
git commit -m "feat(v1alpha2): add encounter/types.proto

Establishes dnd5e.api.v1alpha2.encounter package with all data shapes:
- Common primitives (Ref, Position, HitPoints)
- Enums (EncounterMode, EntityType, TerrainType, WallKind, TrapState)
- Space (Hex, Wall, Zone, StatusEffect, Space)
- Entity with typed data oneof (Character/Monster/Obstacle/Trap/Prop/NPC)
- Encounter, TurnState, ActionEconomy (mirrors toolkit two-level model),
  AvailableAction, MovementInterruption
- InputRequired for caller-private follow-ups (SkillCheckPrompt,
  DialogueChoice, TargetSelect)"
```

---

## Task 2: events.proto — EncounterEvent envelope + 23 payloads

**Files:**
- Create: `dnd5e/api/v1alpha2/encounter/events.proto`

- [ ] **Step 1: Write `events.proto`**

```protobuf
syntax = "proto3";

package dnd5e.api.v1alpha2.encounter;

import "google/protobuf/timestamp.proto";

import "dnd5e/api/v1alpha2/encounter/types.proto";

option go_package = "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/v1alpha2/encounter;encounterpb";
option java_multiple_files = true;
option java_package = "com.kirkdiggler.rpg.api.dnd5e.v1alpha2.encounter";

// EncounterEvent is the unit of state delivery on the stream.
// Decision rule for adding events: events are unique because each one's payload is unique.
// Same payload → same event with a wider type, not a new event.
message EncounterEvent {
  int64 sequence = 1;                      // monotonic per encounter
  google.protobuf.Timestamp timestamp = 2;

  oneof event {
    // Lifecycle
    SnapshotDelivered snapshot_delivered = 5;

    // Geometry & visibility
    GeometryRevealed geometry_revealed = 10;
    EntityAppeared entity_appeared = 11;
    EntityDisappeared entity_disappeared = 12;

    // Entity state
    EntityMoved entity_moved = 20;
    EntityDamaged entity_damaged = 21;
    EntityHealed entity_healed = 22;
    EntityDied entity_died = 23;
    EntityRemoved entity_removed = 24;
    StatusApplied status_applied = 25;
    StatusRemoved status_removed = 26;

    // World interaction
    DoorOpened door_opened = 30;
    DoorClosed door_closed = 31;
    LootRevealed loot_revealed = 32;
    TrapTriggered trap_triggered = 33;
    StateChanged state_changed = 34;       // lever toggle, generic state mutation

    // Encounter mode + turn structure
    ModeChanged mode_changed = 40;
    InitiativeRolled initiative_rolled = 41;
    TurnStarted turn_started = 42;
    TurnEnded turn_ended = 43;
    EncounterEnded encounter_ended = 44;

    // Dialogue
    DialogueStarted dialogue_started = 50;
    DialogueEnded dialogue_ended = 51;
  }
}

// SnapshotDelivered is the first event on a new stream subscription. Carries the player's
// complete view so the client can render without needing to request anything else.
message SnapshotDelivered {
  Encounter encounter = 1;
}

// GeometryRevealed adds hexes/walls to the player's sticky map. Emitted when a door opens,
// when the player moves into a previously-unseen area, or when a Reveal effect is cast.
message GeometryRevealed {
  repeated Hex hexes = 1;
  repeated Wall walls = 2;
}

message EntityAppeared {
  Entity entity = 1;          // came into LOS
  string reason = 2;          // optional: "entered LOS", "spell reveal", "perception success"
}

message EntityDisappeared {
  string entity_id = 1;
  string reason = 2;          // "left LOS", "invisibility cast"
}

message EntityMoved {
  string entity_id = 1;
  Position from = 2;
  Position to = 3;                              // actual final position
  repeated Position actual_path = 4;            // hex by hex, for animation
  optional Ref movement_source = 5;
  optional MovementInterruption interruption = 6;
}

message EntityDamaged {
  string entity_id = 1;
  int32 amount = 2;
  Ref damage_type = 3;        // {module:"dnd5e", type:"damage", id:"slashing"}
  HitPoints hp_after = 4;
  optional string source_entity_id = 5;
}

message EntityHealed {
  string entity_id = 1;
  int32 amount = 2;
  HitPoints hp_after = 3;
  optional string source_entity_id = 4;
}

message EntityDied {
  string entity_id = 1;
  optional string killer_entity_id = 2;
}

message EntityRemoved {
  string entity_id = 1;
  string reason = 2;          // "destroyed", "fled", "transformed"
}

message StatusApplied {
  string entity_id = 1;
  StatusEffect status = 2;
  optional string source_entity_id = 3;
}

message StatusRemoved {
  string entity_id = 1;
  Ref status_source = 2;      // matches StatusEffect.source from earlier StatusApplied
}

message DoorOpened {
  string door_entity_id = 1;
  repeated Hex revealed_hexes = 2;       // full hex (terrain + zone) for newly-visible cells
  repeated Wall revealed_walls = 3;
  repeated Wall removed_walls = 4;       // walls no longer present (door itself transitions
                                         // via DOOR_CLOSED → DOOR_OPEN; this is for any
                                         // genuinely-removed barriers from the open action)
}

message DoorClosed {
  string door_entity_id = 1;
}

message LootRevealed {
  string container_entity_id = 1;
  repeated Ref items = 2;     // {module:"dnd5e", type:"item", id:"longsword"}
}

message TrapTriggered {
  string trap_entity_id = 1;
  string victim_entity_id = 2;
  // Damage events emitted separately; this is the narration moment.
}

message StateChanged {
  string entity_id = 1;
  string state_key = 2;       // entity-specific: "lever_position", "door_locked"
  string state_value = 3;
}

message ModeChanged {
  EncounterMode from = 1;
  EncounterMode to = 2;
  string reason = 3;          // narration: "ambush", "combat ended", "negotiated peace"
}

message InitiativeRolled {
  repeated string order = 1;
}

message TurnStarted {
  string entity_id = 1;
  int32 round = 2;
}

message TurnEnded {
  string entity_id = 1;
}

message EncounterEnded {
  string reason = 1;
}

message DialogueStarted {
  string with_entity_id = 1;
}

message DialogueEnded {
  string with_entity_id = 1;
}
```

- [ ] **Step 2: Format + lint + generate + compile-check Go + TS**

```bash
buf format -w dnd5e/api/v1alpha2/encounter/events.proto
buf lint
buf generate
make compile-go
make compile-ts
```

All five must pass.

- [ ] **Step 3: Commit**

```bash
git add dnd5e/api/v1alpha2/encounter/events.proto
git commit -m "feat(v1alpha2): add encounter/events.proto

EncounterEvent typed union with 23 payloads grouped by category:
- Lifecycle (SnapshotDelivered)
- Geometry & visibility (GeometryRevealed, EntityAppeared/Disappeared)
- Entity state (EntityMoved/Damaged/Healed/Died/Removed, StatusApplied/Removed)
- World interaction (DoorOpened/Closed, LootRevealed, TrapTriggered, StateChanged)
- Mode + turn structure (ModeChanged, InitiativeRolled, TurnStarted/Ended,
  EncounterEnded)
- Dialogue (DialogueStarted/Ended)

Decision rule: events are unique because payloads are unique. Same payload
→ reuse with wider type, never a new event."
```

---

## Task 3: service.proto — EncounterService + RPC request/response messages

**Files:**
- Create: `dnd5e/api/v1alpha2/encounter/service.proto`

- [ ] **Step 1: Write `service.proto`**

```protobuf
syntax = "proto3";

package dnd5e.api.v1alpha2.encounter;

import "dnd5e/api/v1alpha2/encounter/events.proto";
import "dnd5e/api/v1alpha2/encounter/types.proto";

option go_package = "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/v1alpha2/encounter;encounterpb";
option java_multiple_files = true;
option java_package = "com.kirkdiggler.rpg.api.dnd5e.v1alpha2.encounter";

// =============================================================================
// RPC request / response messages
// =============================================================================

message CreateEncounterRequest {
  string campaign_id = 1;
  EncounterMode initial_mode = 2;
  // Future: party composition, dungeon ref, scenario ref
}

message CreateEncounterResponse {
  Encounter encounter = 1;
}

message GetEncounterRequest {
  string encounter_id = 1;
}

message GetEncounterResponse {
  Encounter encounter = 1;
}

message StreamEncounterRequest {
  string encounter_id = 1;
  string player_id = 2;
  optional int64 last_seen_sequence = 3;  // for reconnect/replay
}

message MoveEntityRequest {
  string encounter_id = 1;
  string entity_id = 2;                  // what's being moved
  Position destination = 3;
  repeated Position proposed_path = 4;   // client's chosen path through the hex grid
  optional Ref movement_source = 5;      // spell, ability, item powering this
}

message MoveEntityResponse {
  // Empty ack. World changes flow as events. The server may modify the path
  // (trap intercepts, OoM, opportunity attack, hidden difficult terrain) — events
  // report the actual outcome, not the proposal.
}

message InteractRequest {
  string encounter_id = 1;
  string target_entity_id = 2;
  optional string interaction_kind = 3;  // optional: "open", "examine", "loot"
}

message InteractResponse {
  // World changes flow as events. This response carries only follow-ups
  // private to the caller (skill check prompts, dialogue choices).
  optional InputRequired input_required = 1;
}

message TakeActionRequest {
  string encounter_id = 1;
  string actor_entity_id = 2;
  Ref action_ref = 3;                    // {module:"dnd5e", type:"action", id:"attack"}
  ActionTarget target = 4;
  // Future: spell level, ki spend, etc.
}

message TakeActionResponse {
  optional InputRequired input_required = 1;
}

// ActionTarget is the union of how an action can be aimed.
message ActionTarget {
  oneof kind {
    string entity_id = 1;
    Position position = 2;
    AreaTarget area = 3;
    SelfTarget self = 4;
  }
}

message AreaTarget {
  Position center = 1;
  // Future: shape (cone, line, sphere, cube), size, orientation
}

message SelfTarget {}

message EndTurnRequest {
  string encounter_id = 1;
  string entity_id = 2;
}

message EndTurnResponse {}

// SubmitCheckRequest resolves the caller's currently-pending InputRequired prompt.
// No correlation id: server tracks pending_prompt_per_player. If none is pending,
// server returns FailedPrecondition.
message SubmitCheckRequest {
  string encounter_id = 1;
  string entity_id = 2;
  int32 roll = 3;          // raw d20 result the client rolled
  // Future: optional advantage/disadvantage, modifier breakdown
}

message SubmitCheckResponse {
  bool success = 1;
  int32 total = 2;
}

// =============================================================================
// Service
// =============================================================================

service EncounterService {
  // Lifecycle
  rpc CreateEncounter(CreateEncounterRequest) returns (CreateEncounterResponse);
  rpc GetEncounter(GetEncounterRequest) returns (GetEncounterResponse);

  // Stream — primary state delivery channel.
  // Server sends an initial snapshot as the first event, then deltas. Reconnects pass
  // last_seen_sequence to resume; if gap is too large, server sends a fresh snapshot.
  rpc StreamEncounter(StreamEncounterRequest) returns (stream EncounterEvent);

  // World actions — emit events on the stream. Caller-private follow-ups (skill check,
  // dialogue) come back in the response's InputRequired.
  rpc MoveEntity(MoveEntityRequest) returns (MoveEntityResponse);
  rpc Interact(InteractRequest) returns (InteractResponse);

  // Turn-based only — server returns FailedPrecondition if mode != TURN_BASED.
  rpc TakeAction(TakeActionRequest) returns (TakeActionResponse);
  rpc EndTurn(EndTurnRequest) returns (EndTurnResponse);

  // Resolves the caller's pending InputRequired prompt.
  // Returns FailedPrecondition if no prompt is pending.
  rpc SubmitCheck(SubmitCheckRequest) returns (SubmitCheckResponse);
}
```

- [ ] **Step 2: Format + lint + generate + compile-check**

```bash
buf format -w dnd5e/api/v1alpha2/encounter/service.proto
buf lint
buf generate
make compile-go
make compile-ts
```

All five must pass. `buf generate` should now produce `gen/go/dnd5e/api/v1alpha2/encounter/service.pb.go` AND `service_grpc.pb.go` (the gRPC service stubs).

- [ ] **Step 3: Commit**

```bash
git add dnd5e/api/v1alpha2/encounter/service.proto
git commit -m "feat(v1alpha2): add encounter/service.proto

EncounterService with 8 RPCs:
  - Lifecycle: CreateEncounter, GetEncounter
  - Stream: StreamEncounter (server stream of EncounterEvent)
  - World actions: MoveEntity, Interact
  - Turn-based: TakeAction, EndTurn
  - Prompt resolution: SubmitCheck (resolves pending InputRequired)

Plus all RPC request/response message types and ActionTarget union."
```

---

## Task 4: Update Makefile — v1alpha2 mocks + idempotent compile-go

**Files:**
- Modify: `Makefile` — add v1alpha2/encounter mock entry; make `compile-go` idempotent

This task does two things:
1. Wire mock generation for the new v1alpha2 encounter service
2. Fix the existing `compile-go` target to be safe to re-run (it currently runs `go mod init` unconditionally, which fails on second invocation when `gen/go/go.mod` already exists)

- [ ] **Step 1: Read the current `mocks` and `compile-go` targets**

```bash
grep -A 30 '^mocks:' Makefile
grep -A 3 '^compile-go:' Makefile
```

- [ ] **Step 2: Add v1alpha2/encounter mock entry**

In the `mocks:` target, after the existing v1alpha1 encounter mock line, add:

```makefile
	mkdir -p gen/go/dnd5e/api/v1alpha2/encounter/mocks
	mockgen -source=gen/go/dnd5e/api/v1alpha2/encounter/service_grpc.pb.go -destination=gen/go/dnd5e/api/v1alpha2/encounter/mocks/encounter_service.go -package=mocks
```

- [ ] **Step 3: Make `compile-go` idempotent**

Replace the existing `compile-go` target body:

```makefile
compile-go: ## Test Go compilation
	cd gen/go && go mod init github.com/KirkDiggler/rpg-api-protos/gen/go && go mod tidy && go build ./...
```

with:

```makefile
compile-go: ## Test Go compilation
	cd gen/go && { [ -f go.mod ] || go mod init github.com/KirkDiggler/rpg-api-protos/gen/go; }
	cd gen/go && go mod tidy && go build ./...
```

The `{ … }` brace group binds the `||` tightly to the `[ -f go.mod ]` test (so a hypothetical `cd gen/go` failure doesn't fall through to running `go mod init` in the repo root). The guard skips `go mod init` when `go.mod` already exists; `go mod tidy && go build ./...` always runs. After this fix `make compile-go` is safe to re-run.

- [ ] **Step 4: Run `make mocks`**

```bash
make mocks
```

Expected: produces `gen/go/dnd5e/api/v1alpha2/encounter/mocks/encounter_service.go`. Verify file exists and is non-empty:

```bash
test -s gen/go/dnd5e/api/v1alpha2/encounter/mocks/encounter_service.go && echo "ok"
```

- [ ] **Step 5: Verify idempotency of compile-go**

```bash
make compile-go
make compile-go  # second run should succeed, not fail with "go.mod already exists"
```

Both must succeed.

- [ ] **Step 6: Run full `make test`**

```bash
make test
```

Expected: lint + format check + generate + mocks all pass.

- [ ] **Step 7: Commit**

> **Note on `gen/`:** `.gitignore` lists `gen/` so generated code is never committed on feature branches. CI publishes generated artifacts to the `generated` branch on merge to `main`. Feature branches commit only `.proto`, `Makefile`, and any other source-side changes.

```bash
git add Makefile
git commit -m "feat(v1alpha2): generate gRPC service mocks for testing

Adds mock generation for v1alpha2 encounter service. Also makes the
compile-go target idempotent (skip 'go mod init' when go.mod exists)
so it can be re-run safely during local verification."
```

---

## Task 5: Consumer compile check — verify every public type is reachable

**Files:**
- (no permanent file changes — uses `make compile-go` to bootstrap a real consumer go.mod, then a throwaway file)

- [ ] **Step 1: Ensure `gen/go/go.mod` exists**

```bash
make compile-go
```

After Task 4's Makefile fix, this target is idempotent: if `gen/go/go.mod` doesn't exist, it runs `go mod init` once; otherwise that step is skipped. Either way, `go mod tidy && go build ./...` always runs.

- [ ] **Step 2: Write a throwaway consumer file inside gen/go**

```bash
cat > gen/go/consumer_check_v1alpha2.go <<'EOF'
//go:build ignore

// consumer_check_v1alpha2 verifies that every public type in
// dnd5e.api.v1alpha2.encounter can be referenced. NOT committed; throwaway compile check.
package main

import (
	"fmt"

	v2 "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/v1alpha2/encounter"
	"google.golang.org/grpc"
)

func main() {
	// Common
	_ = &v2.Ref{Module: "dnd5e", Type: "monster", Id: "goblin"}
	_ = &v2.Position{X: 0, Y: 0, Z: 0}
	_ = &v2.HitPoints{Current: 10, Max: 10, Temp: 0}

	// Enums
	_ = v2.EncounterMode_ENCOUNTER_MODE_FREE_ROAM
	_ = v2.EntityType_ENTITY_TYPE_CHARACTER
	_ = v2.TerrainType_TERRAIN_TYPE_FLOOR
	_ = v2.WallKind_WALL_KIND_DOOR_CLOSED
	_ = v2.TrapState_TRAP_STATE_ARMED

	// Space
	_ = &v2.Hex{}
	_ = &v2.Wall{}
	_ = &v2.Zone{}
	_ = &v2.StatusEffect{}
	_ = &v2.Space{}

	// Entity + typed data
	_ = &v2.Entity{}
	_ = &v2.CharacterData{}
	_ = &v2.MonsterData{}
	_ = &v2.ObstacleData{}
	_ = &v2.TrapData{}
	_ = &v2.PropData{}
	_ = &v2.NpcData{}

	// Encounter messages
	_ = &v2.Encounter{}
	_ = &v2.TurnState{}
	_ = &v2.ActionEconomy{}
	_ = &v2.AvailableAction{}
	_ = &v2.MovementInterruption{}
	_ = &v2.InputRequired{}
	_ = &v2.SkillCheckPrompt{}
	_ = &v2.DialogueChoice{}
	_ = &v2.DialogueOption{}
	_ = &v2.TargetSelect{}

	// RPC request/response
	_ = &v2.CreateEncounterRequest{}
	_ = &v2.CreateEncounterResponse{}
	_ = &v2.GetEncounterRequest{}
	_ = &v2.GetEncounterResponse{}
	_ = &v2.StreamEncounterRequest{}
	_ = &v2.MoveEntityRequest{}
	_ = &v2.MoveEntityResponse{}
	_ = &v2.InteractRequest{}
	_ = &v2.InteractResponse{}
	_ = &v2.TakeActionRequest{}
	_ = &v2.TakeActionResponse{}
	_ = &v2.ActionTarget{}
	_ = &v2.AreaTarget{}
	_ = &v2.SelfTarget{}
	_ = &v2.EndTurnRequest{}
	_ = &v2.EndTurnResponse{}
	_ = &v2.SubmitCheckRequest{}
	_ = &v2.SubmitCheckResponse{}

	// Events
	_ = &v2.EncounterEvent{}
	_ = &v2.SnapshotDelivered{}
	_ = &v2.GeometryRevealed{}
	_ = &v2.EntityAppeared{}
	_ = &v2.EntityDisappeared{}
	_ = &v2.EntityMoved{}
	_ = &v2.EntityDamaged{}
	_ = &v2.EntityHealed{}
	_ = &v2.EntityDied{}
	_ = &v2.EntityRemoved{}
	_ = &v2.StatusApplied{}
	_ = &v2.StatusRemoved{}
	_ = &v2.DoorOpened{}
	_ = &v2.DoorClosed{}
	_ = &v2.LootRevealed{}
	_ = &v2.TrapTriggered{}
	_ = &v2.StateChanged{}
	_ = &v2.ModeChanged{}
	_ = &v2.InitiativeRolled{}
	_ = &v2.TurnStarted{}
	_ = &v2.TurnEnded{}
	_ = &v2.EncounterEnded{}
	_ = &v2.DialogueStarted{}
	_ = &v2.DialogueEnded{}

	// Service interfaces — confirms gRPC plugin produced server + client stubs
	var _ v2.EncounterServiceClient = (v2.EncounterServiceClient)(nil)
	var _ v2.EncounterServiceServer = (v2.EncounterServiceServer)(nil)
	_ = v2.NewEncounterServiceClient((*grpc.ClientConn)(nil))

	fmt.Println("v1alpha2 consumer check OK")
}
EOF
```

The `//go:build ignore` tag keeps this file out of the regular package build; we run it explicitly.

- [ ] **Step 3: Run the consumer check**

```bash
cd gen/go
go mod tidy   # pull in google.golang.org/grpc transitively used by the throwaway file
go run consumer_check_v1alpha2.go
cd /home/kirk/personal/rpg-api-protos
```

Expected: `v1alpha2 consumer check OK` printed. Any compile error means a public type is missing or named differently than expected.

- [ ] **Step 4: Clean up the throwaway file**

```bash
rm gen/go/consumer_check_v1alpha2.go
```

- [ ] **Step 5: Re-run `make compile-go` and `make compile-ts` to confirm no residue**

```bash
make compile-go
make compile-ts
```

Both should still pass cleanly.

(No commit needed — verification only.)

---

## Task 6: PR + merge

- [ ] **Step 1: Run `make test` one final time**

```bash
cd /home/kirk/personal/rpg-api-protos
make test
make compile-go
make compile-ts
```

All four targets must pass.

- [ ] **Step 2: Push the branch**

```bash
git push -u origin feat/v1alpha2-encounter-protos
```

- [ ] **Step 3: Open a PR**

```bash
gh pr create --title "feat: add dnd5e.api.v1alpha2 encounter contract" --body "$(cat <<'EOF'
## Summary

- Adds `dnd5e.api.v1alpha2.encounter` proto package with the redesigned encounter contract per [v1alpha2 design](../../../rpg-project/ideas/encounter/v1alpha2/design.md).
- Three new proto files in `dnd5e/api/v1alpha2/encounter/`: `types`, `events`, `service` (subdirectory layout sets convention for future v2 services).
- `EncounterService` with 8 RPCs: `CreateEncounter`, `GetEncounter`, `StreamEncounter`, `MoveEntity`, `Interact`, `TakeAction`, `EndTurn`, `SubmitCheck`.
- 23 typed event payloads in `EncounterEvent`'s oneof.
- Two-level action economy mirrors toolkit `rulebooks/dnd5e/combat/action_economy.go`.
- Refs (`module/type/id`) for all typed toolkit content; strings reserved for instance ids and UI text.
- `InputRequired` correlation handled via server-side pending-prompt tracking; `SubmitCheck` returns `FailedPrecondition` when no prompt is pending.
- v1alpha1 encounter contract is **untouched** — Phase 5 (cutover) deletes it after web migrates in Phase 4.

## Test plan
- [ ] `make test` passes locally (lint + format + generate + mocks)
- [ ] `make compile-go` — `gen/go/` builds cleanly
- [ ] `make compile-ts` — `npx tsc --noEmit` passes
- [ ] Consumer compile check (Phase 1 plan Task 5) — every public type reachable from external Go program
- [ ] CI green (buf lint, buf breaking-vs-main on the new file set, generate)

🤖 Generated with [Claude Code](https://claude.com/claude-code)
EOF
)"
```

- [ ] **Step 4: Wait for CI + Copilot review**

Per `feedback_copilot_review_timing`, sleep 6–10 min before first poll.

```bash
gh pr checks
gh pr view --comments
```

Resolve any review comments per `feedback_copilot_review`. Fix or threaded why-not on every comment before asking Kirk to merge.

- [ ] **Step 5: Ask Kirk to merge**

Per repo convention, never merge PRs without explicit user approval.

- [ ] **Step 6: After merge — verify CI auto-published a release**

```bash
git checkout main
git pull
git ls-remote --tags origin | tail -3
```

CI should have pushed to the `generated` branch and tagged a release. The resulting Go module pseudo-version is what Phase 2 (rpg-api) will consume via `go get -u github.com/KirkDiggler/rpg-api-protos/gen/go@latest`.

---

## Phase 1 Done-When (recap)

- [ ] `make lint` passes (no output, exit 0)
- [ ] `make compile-go` passes
- [ ] `make compile-ts` passes (`npx tsc --noEmit`, exit 0)
- [ ] `make test` passes (lint + format check + generate + mocks)
- [ ] Task 5 consumer compile check passes (every public Go type referenced from external program builds)
- [ ] PR merged to `main` on `rpg-api-protos` (`gh pr view <num> --json state` returns `MERGED`)
- [ ] CI auto-pushed to `generated` branch and tagged a release (`git ls-remote --tags origin | tail -3`)

When all boxes are checked, return to `../plan.md` and trigger writing of `plans/02-api-orchestrator.md` informed by what Phase 1 actually produced.
