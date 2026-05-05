# Encounter v1alpha2 — Design

**Status:** Draft (brainstorm complete, awaiting spec review)
**Date:** 2026-05-05
**Authors:** Kirk + Claude (brainstorming session)
**Supersedes:** `dnd5e.api.v1alpha1.EncounterService` (full replacement, clean break)

---

## 1. Goal & Motivation

### Why v1alpha2

v1alpha1 has accumulated a class of bugs that are not patchable in place — they all trace to a contract that leaks server-side concepts (rooms, room-local coordinates, currentRoomId focus) onto the wire, which forces the web client into game-logic territory it shouldn't occupy.

The pattern across the last several weeks of debugging:

- Walls stored in room-local coords, web has to translate per room
- `currentRoomId` singular focus drops entities outside the active room
- "Open door reveals new room" requires the web to splice rooms together
- Floor tiles, range overlays, pathing all need to know which room they're in
- `MoveCharacter` cross-room handler has dropped fields we keep finding (`FinalPosition`, `StopReason`, …)
- Web has to know roomId to filter entities, gate interactions, render correctly

Each individual fix has been small. The aggregate cost is large because the contract itself encourages these bugs — every new feature touches the room-id seam, and the seam is wider than it should be.

The goal of v1alpha2 is not to fix the symptoms. It's to redesign the contract so the symptoms can't recur.

### What v1alpha1 got wrong (the root)

> The web should not know about rooms.

Rooms are a server-side authoring concept (the dungeon generator places them, the toolkit's `tools/spatial` thinks in terms of them, the toolkit's `tools/environments` orchestrates them). They should not survive to the wire. The web needs hexes, walls, entities, visibility — not a graph of rooms with their own coordinate systems.

### Design principles for v1alpha2

1. **The web is a renderer.** It draws what the server says exists. It does not maintain game state, gate interactions on game state, or compute walkability.
2. **The server owns visibility.** What the player can see is computed server-side per player; the wire never carries information the player isn't entitled to. (No DevTools cheating.)
3. **One coord space.** Dungeon-absolute hex coordinates everywhere on the wire. Room-local coords are a server-internal implementation detail.
4. **One verb per concept.** `MoveEntity` (any entity), `Interact` (any target). No `MovePlayer` + `MoveMonster` when the payload is the same.
5. **Events are the story.** Typed events with semantic verbs carry world-state changes. RPC responses are minimal acks plus caller-private follow-ups (skill check prompts, dialogue choices).
6. **Mirror the toolkit, don't reinvent it.** The toolkit has thought hard about action economy, conditions, spatial, environments. The wire reflects those decisions rather than introducing parallel abstractions.

---

## 2. Architecture

### Service decomposition

**One service:** `dnd5e.api.v1alpha2.EncounterService`.

Earlier brainstorming considered splitting into a `SpaceService` (geometry primitive) and an `EncounterService` (turn-based overlay). We rejected the split: Space is a *data type*, not a service. Everything happens inside an Encounter — a turn-based combat is an Encounter, a town is an Encounter (in `FREE_ROAM` mode), an exploration session is an Encounter. There is no "between encounters" state for an online player.

### Encounter as session, mode as policy

An Encounter is the *session* the player is currently inside. Its `mode` determines the rule-set in effect:

- `FREE_ROAM` — no initiative, no action budget, free movement and interaction
- `TURN_BASED` — initiative order, action economy, turn boundaries

Mode transitions happen *within* a single encounter (the encounter persists across the transition). A party walks into a tavern (`FREE_ROAM`) → opens a chest that turns out to be a mimic → server flips mode to `TURN_BASED` → combat resolves → mode flips back to `FREE_ROAM`. Same encounter, same space, same stream throughout.

This is a significant departure from v1alpha1, which thinks of an encounter as a short-lived combat session. Implication: the server always has *some* encounter active for an online player. Idle/town encounters need cheap in-memory persistence (same model as today, garbage-collected when the player leaves).

### Space as data

`Space` is a message type, not a service. It carries:

- `Hex[]` — explored geometry (sticky per character, persists across encounters in a campaign)
- `Wall[]` — explored walls
- `Entity[]` — currently visible entities (real-time LOS filtered server-side)
- `Zone[]` — optional metadata regions (lighting, music, scripted triggers, named rooms for ambient cues)

The map is **continuous**. Rooms abut directly with shared walls; doors are passages cut in those shared walls. Opening a door doesn't append a new room — it removes barriers and expands the player's visible region. Intentional empty space (chasms, voids) is filled in with hexes whose terrain is non-walkable.

Server-side, the toolkit still thinks in rooms (generation, room-type behaviors, traps/puzzle/story rooms). That stays inside the orchestrator. The wire never sees roomId.

### Tiered visibility

The visibility model has two layers:

| Layer | Rule | Wire behavior |
|---|---|---|
| Geometry (hexes, walls) | **Sticky.** Once a player has revealed a hex, it stays in their map permanently. | Server emits `GeometryRevealed` events as new regions are explored. Persists per character across the campaign. |
| Entities + statuses | **Real-time LOS.** Visible only while in line-of-sight. | Server emits `EntityAppeared` / `EntityDisappeared` as LOS changes. Status effects are filtered by entity visibility. |

Hidden traps and illusions are modeled as **entity-overrides**: the trap entity reports `appears_as: floor` to players who haven't detected it. Geometry stays truthful; the deception lives in the entity layer. A perception success can flip the appearance for one player without altering what others see.

### Entity model

```proto
message Entity {
  string id = 1;
  Position position = 2;
  EntityType type = 3;
  string display_name = 4;

  optional HitPoints hp = 5;            // present only on destructibles
  repeated StatusEffect status_effects = 6;  // visible only

  oneof data {
    CharacterData character = 10;
    MonsterData   monster   = 11;
    ObstacleData  obstacle  = 12;
    TrapData      trap      = 13;
    PropData      prop      = 14;
    NpcData       npc       = 15;
  }
}
```

The common shape (id, position, type, display_name, optional hp, status_effects) is what every entity carries because every entity is **destructible-or-not** and **has-or-doesn't-have visible statuses**. The oneof carries the type-specific rich data.

Type-specific data messages identify their toolkit content via `Ref`:

```proto
message MonsterData {
  Ref monster_ref = 1;          // {module:"dnd5e", type:"monster", id:"goblin"}
  // monster-specific runtime state: CR-derived stats, abilities, behavior tag
}

message CharacterData {
  Ref class_ref = 1;            // {module:"dnd5e", type:"class", id:"fighter"}
  Ref race_ref = 2;
  // character-specific state
}

message TrapData {
  Ref trap_ref = 1;             // {module:"dnd5e", type:"trap", id:"pit-trap"}
  TrapState state = 2;          // ARMED, TRIGGERED, DISARMED
  EntityType appears_as = 3;    // illusion: shows as FLOOR/PROP to undetected players
}
```

`StatusEffect` is the wire-shape of the toolkit's `core.Effect` (called `Condition` in the dnd5e rulebook, where they're event-bus subscribers). The wire carries only the *display* payload — name, icon, optional duration hint, and the `Ref` of the source effect for tooltip drill-down. The reactive mechanism stays server-side.

```proto
message StatusEffect {
  Ref source = 1;               // {module:"dnd5e", type:"condition", id:"poisoned"}
  string display_name = 2;
  string icon_hint = 3;         // UI category — actual asset path resolved web-side
  optional int32 duration_rounds = 4;
}
```

### Layer correspondence

| Layer | Concept | What it carries |
|---|---|---|
| Toolkit | `core.Effect` | Subscriber on event bus (mechanism) |
| Rulebook (dnd5e) | `Condition` | 5e player-facing concept (Frightened, Poisoned, ...) |
| Wire | `StatusEffect` | Display payload (name, icon, duration hint) |

Each layer is the thinnest version of the concept it needs.

---

## 3. Wire Shapes

### Ref — typed reference to toolkit content

```proto
message Ref {
  string module = 1;   // "dnd5e", future: "artificer", "homebrew-x"
  string type = 2;     // "monster", "feature", "condition", "action", "spell", "item"
  string id = 3;       // "goblin", "rage", "dodging", "attack"
}
```

**Anything that identifies a typed piece of toolkit content is a `Ref`, not a string.** The `module/type/id` triple disambiguates across rulebooks and avoids id collisions when modules grow. Strings are reserved for opaque runtime instance ids (`encounter_id`, `entity_id`) and display payloads (`display_name`, narration text).

The `type` field is **open-ended**, not enum-constrained. Examples currently in use:

| `type` | Example `id`s |
|---|---|
| `monster` | `goblin`, `dragon-young-red` |
| `class` | `fighter`, `wizard` |
| `race` | `human`, `dwarf-mountain` |
| `feature` | `rage`, `second-wind` |
| `condition` | `poisoned`, `prone` |
| `action` | `attack`, `dash`, `disengage` |
| `spell` | `fireball`, `mage-armor` |
| `item` | `longsword`, `thieves-tools` |
| `damage` | `slashing`, `fire`, `radiant` |
| `trap`, `obstacle`, `prop`, `npc` | as appropriate |

New types are added by the toolkit as it grows. The wire never validates the `type` value — it's purely identifying.

Rule of thumb:

| Field looks like... | Type |
|---|---|
| `encounter_id`, `entity_id`, `door_entity_id` | `string` (runtime instance id) |
| `action_ref`, `movement_source`, `monster_ref`, `feature_ref` | `Ref` (toolkit-defined content) |
| `display_name`, `description`, `unavailable_reason` | `string` (UI text) |
| `terrain`, `mode`, `wall_kind` | `enum` (fixed value set) |

### Position

```proto
message Position {
  // Hex cube coordinates. Invariant: x + y + z == 0.
  int32 x = 1;
  int32 y = 2;
  int32 z = 3;
}
```

Dungeon-absolute. There is no other coord space on the wire.

### Encounter

```proto
message Encounter {
  string id = 1;
  EncounterMode mode = 2;
  Space space = 3;
  optional TurnState turn_state = 4;  // present only when mode == TURN_BASED
  // session-level metadata (campaign, party, etc.)
}

enum EncounterMode {
  ENCOUNTER_MODE_UNSPECIFIED = 0;
  ENCOUNTER_MODE_FREE_ROAM = 1;
  ENCOUNTER_MODE_TURN_BASED = 2;
}
```

### Space

```proto
message Space {
  repeated Hex hexes = 1;       // explored, sticky
  repeated Wall walls = 2;      // explored
  repeated Entity entities = 3; // currently visible
  repeated Zone zones = 4;      // optional metadata
}

message Hex {
  Position position = 1;
  TerrainType terrain = 2;      // FLOOR, ROUGH, DIFFICULT, VOID, WATER, ...
  string zone_id = 3;           // optional zone membership
}

message Wall {
  Position from = 1;
  Position to = 2;
  WallKind kind = 3;            // SOLID, DOOR_CLOSED, DOOR_OPEN, WINDOW, ...
}

message Zone {
  string id = 1;
  string name = 2;              // narration ("Throne Room", "Tavern Common Room")
  // ambient hooks: lighting profile, music cue, encounter triggers
}
```

Walkability is *not* a wire field. Server evaluates `terrain × entity movement modes` at MoveEntity time and returns a definitive answer.

### TurnState (present only in TURN_BASED)

```proto
message TurnState {
  repeated string initiative_order = 1;   // entity ids in order
  string active_entity_id = 2;
  int32 round = 3;                        // 1-based, increments when order wraps
  ActionEconomy economy = 4;
  repeated AvailableAction available_actions = 5;
}
```

### ActionEconomy

Mirrors the toolkit's two-level model from `rulebooks/dnd5e/combat/action_economy.go`.

```proto
message ActionEconomy {
  // Primary resources — what you spend
  int32 actions_remaining = 1;
  int32 bonus_actions_remaining = 2;
  int32 reactions_remaining = 3;
  int32 movement_remaining = 4;          // in space units (hexes or feet, TBD)

  // Capacity — what abilities have granted you
  // Keys are toolkit-defined: "attacks", "off_hand_attacks", "flurry_strikes"
  map<string, int32> capacities = 10;
}
```

The capacity map is the rulebook-specific bag. New abilities (Action Surge variants, future class features) add new keys without churning the proto.

### AvailableAction

```proto
message AvailableAction {
  Ref ref = 1;                     // {module:"dnd5e", type:"action", id:"attack"}
  string display_name = 2;
  bool available = 3;
  string unavailable_reason = 4;   // user-facing: "no movement", "out of range", "already used"
  // optional: icon hint, hotkey hint, category
}
```

Web renders the list. It does not compute availability — the server evaluates "can this character take this action right now" against the toolkit and answers definitively.

---

## 4. RPCs

### Service surface

```proto
service EncounterService {
  // Lifecycle
  rpc CreateEncounter(CreateEncounterRequest) returns (CreateEncounterResponse);
  rpc GetEncounter(GetEncounterRequest) returns (GetEncounterResponse);

  // Stream — primary state delivery channel
  rpc StreamEncounter(StreamEncounterRequest) returns (stream EncounterEvent);

  // World actions — emit events on the stream
  rpc MoveEntity(MoveEntityRequest) returns (MoveEntityResponse);
  rpc Interact(InteractRequest) returns (InteractResponse);

  // Turn-based only — guarded server-side by encounter mode
  rpc TakeAction(TakeActionRequest) returns (TakeActionResponse);
  rpc EndTurn(EndTurnRequest) returns (EndTurnResponse);
  rpc SubmitCheck(SubmitCheckRequest) returns (SubmitCheckResponse);  // skill check follow-up
}
```

**InputRequired correlation.** A player can have at most one pending prompt at a time (turn-based and free-roam alike — the UI surfaces one dialog, the player can't fire two RPCs concurrently). The server tracks `pending_prompt_per_player` as part of encounter state. `SubmitCheck` implicitly resolves that prompt; if none is pending, the server returns `FailedPrecondition`. No correlation id on the wire — the oneof in `InputRequired` tells the client what UI to render; the server's pending-state tracking handles demux.

### MoveEntity

Targets *any* entity. Authority comes from the auth header (which player) plus server-side validation (do they have the strength / spell / proficiency to move this thing).

```proto
message MoveEntityRequest {
  string encounter_id = 1;
  string entity_id = 2;                 // what's being moved
  Position destination = 3;
  repeated Position proposed_path = 4;  // client's chosen path through the hex grid
  optional Ref movement_source = 5;     // spell, ability, item — what's powering this
}

message MoveEntityResponse {
  // Empty ack. World changes flow as events. The server may modify the path
  // (trap intercepts, OoM, opportunity attack, hidden difficult terrain) —
  // events report the actual outcome, not the proposal.
}
```

**Path correction is fundamental.** Client sends the path it *thinks* it can take. Server simulates step-by-step against the real world (which the client may not fully see — undetected traps, hidden difficult terrain, perception-gated obstacles) and emits events for what actually happened. The proposed path may be cut short by an `EntityMoved` with a `MovementInterruption`, or extended by additional events as triggers fire.

Player moves their character → authority because they own it. Player pushes a box → authority via Strength check or spell. Server-driven monster turns are *not* RPC calls — they're server-internal logic emitting events directly.

### Interact

The honest verb. Same call shape, completely different consequences depending on target — the entity decides what happens.

```proto
message InteractRequest {
  string encounter_id = 1;
  string target_entity_id = 2;
  optional string interaction_kind = 3;  // optional disambiguation: "open", "examine", "loot"
}

message InteractResponse {
  // World changes flow as events. This response carries only follow-ups
  // private to the caller (skill check prompts, dialogue choices).
  optional InputRequired input_required = 1;
}

message InputRequired {
  oneof kind {
    SkillCheckPrompt skill_check = 1;
    DialogueChoice dialogue = 2;
    TargetSelect target = 3;
  }
}
```

Outcomes and the events they trigger:

| Target state | Events emitted | Caller response |
|---|---|---|
| Door (closed, unlocked) | `DoorOpened`, `GeometryRevealed`, `EntityAppeared`* | empty |
| Door (locked) | none yet | `InputRequired{skill_check: {dc, ability, tool}}` |
| Chest (normal) | `LootRevealed`, `EntityRemoved`(chest) | empty |
| Chest (mimic!) | `EntityRemoved`(chest), `EntityAppeared`(mimic), `ModeChanged`(TURN_BASED), `InitiativeRolled`, `TurnStarted` | empty |
| Trap (undetected) | `TrapTriggered`, status events | empty |
| Trap (detected) | none yet | `InputRequired{skill_check: {disarm DC}}` |
| NPC | `DialogueStarted` | `InputRequired{dialogue: {options}}` |
| Lever | `StateChanged` | empty |

\* if entities in the now-revealed region come into LOS

### TakeAction

Used in `TURN_BASED` for ability/action invocation. The action ref identifies what's being done; the toolkit resolves the rules.

```proto
message TakeActionRequest {
  string encounter_id = 1;
  string actor_entity_id = 2;
  Ref action_ref = 3;              // {module:"dnd5e", type:"action", id:"attack"}
  ActionTarget target = 4;         // entity, position, area, self
  // optional ability-specific parameters (spell level, ki spend, etc.)
}
```

### EndTurn

```proto
message EndTurnRequest {
  string encounter_id = 1;
  string entity_id = 2;
}
```

Server validates active_entity_id matches, advances initiative, increments round if order wrapped, emits `TurnEnded` + `TurnStarted` events.

---

## 5. Streaming

### Event shape: typed union with semantic verbs

```proto
message EncounterEvent {
  int64 sequence = 1;                      // monotonic per encounter
  google.protobuf.Timestamp timestamp = 2;

  oneof event {
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
```

### Decision rule for adding a new event

> Events are unique because each one's payload is unique.

If a new world-state change has the same payload as an existing event, reuse it with a wider type. Don't add `WindowOpened` if its payload is identical to `DoorOpened` — generalize to `BarrierOpened` if needed, or add `WindowOpened` only if its payload genuinely differs.

This keeps the event taxonomy honest and the web's switch statement bounded.

### What events carry

Each event payload carries the semantic verb *and* the explicit state mutation it represents. The web does not have to compute diffs against a previous snapshot; the event tells it what changed.

Example:

```proto
message DoorOpened {
  string door_entity_id = 1;
  repeated Hex revealed_hexes = 2;
  repeated Wall revealed_walls = 3;
  repeated Wall removed_walls = 4;
}

message EntityMoved {
  string entity_id = 1;
  Position from = 2;
  Position to = 3;                            // actual final position (may differ from proposed destination)
  repeated Position actual_path = 4;          // what actually happened, hex by hex, for animation
  optional Ref movement_source = 5;           // spell/ability/item that powered the move
  optional MovementInterruption interruption = 6;  // present when path was cut short
}

message MovementInterruption {
  enum Reason {
    REASON_UNSPECIFIED = 0;
    REASON_OUT_OF_MOVEMENT = 1;
    REASON_TRAP_TRIGGERED = 2;
    REASON_OPPORTUNITY_ATTACK = 3;
    REASON_DIFFICULT_TERRAIN = 4;             // hidden until perceived
    REASON_BLOCKED_BY_ENTITY = 5;
    REASON_BLOCKED_BY_TERRAIN = 6;            // unwalkable hex on path
  }
  Reason reason = 1;
  string description = 2;                     // human-readable narration
  optional Ref triggered_by = 3;              // which trap, feature, ability caused it
}
```

### Stream lifecycle

- Client connects via `StreamEncounter(encounter_id, player_id)`
- Server sends initial **snapshot** as the first event (`SnapshotDelivered{ encounter: ... }`) so the client has a complete starting state
- Subsequent events are deltas
- On reconnect, client sends `last_seen_sequence` and server replays missed events; if the gap is too large, server sends a fresh snapshot

---

## 6. Migration

Clean break. No proxy layer, no dual-write.

### Phases

1. **Define v1alpha2 protos** in `rpg-api-protos`. Generate Go and TypeScript bindings.
2. **Bootstrap handler.** `dnd5e.api.v1alpha2.EncounterService` with `codes.Unimplemented` returns. Outside-in development.
3. **Build orchestrator.** Reuse what fits from v1 orchestration; write new where needed. This is where the toolkit's `tools/spatial`, `tools/environments`, action economy, and condition/effect machinery integrate cleanly instead of being patched against a bad contract.
4. **Migrate web** off v1alpha1 onto v1alpha2 endpoints. Web is the canary — when the web flow works end-to-end on v2, v1 has no remaining consumers.
5. **Delete v1alpha1.** Handler, orchestrator paths that only existed for v1, web v1alpha1 hooks, the protos themselves. Aim is zero v1alpha1 references in the codebase after cutover.

### Why no parallel-run

The half-state (both versions running, web speaking v1, server-side gradually migrating) is exactly the pattern that has caused the bugs we're fixing. Each partial-migration shim is a place for the contract to drift. A clean break is shorter-lived pain for a meaningful structural win. (See `feedback_prefer_breaking_changes`.)

### Risk: web work

The web changes are not trivial — every component that touches roomId, currentRoomId, per-room coord translation, or the existing event taxonomy needs to be updated. The good news: most of those touchpoints exist precisely because v1alpha1 forced them. They go away in v2, not get rewritten.

---

## 7. Open Follow-ups

These are not blockers for v1alpha2 — they're work items surfaced during the brainstorm that should be tracked but don't need to be resolved before drafting the implementation plan.

### Toolkit documentation harness

The toolkit has a documentation harness:

- `docs/status.md` — current health, active work, paused items, per-subsystem confidence
- `docs/quality.md` — A-D scorecard with rationale per module
- `docs/architecture/overview.md` — layer rules, module map, boundaries
- `docs/architecture/components/<module>/` — per-module **directory** with focused per-concept docs (one concept per file)
- `docs/adr/` — architectural decisions
- `docs/journey/` — exploration narratives

**The rule:** everything we know about how the toolkit works lives in the harness, not scattered across `CLAUDE.md` files. CLAUDE.md is for agent-facing development guidelines (workflow, conventions); architecture knowledge belongs in `architecture/`, status in `status.md`, quality in `quality.md`. When a new pattern is discovered, it gets documented in the harness — not buried in CLAUDE.md where consumers can't find it.

**Component docs are directories, not monoliths.** A module with multiple distinct concepts (e.g., dnd5e rulebook has action economy, conditions, AC chain, attack resolution; spatial has grids, rooms, orchestrator, connections) gets a folder where each concept is its own file:

```
docs/architecture/components/
  rulebook/
    action_economy.md      ← the two-level model lives here
    conditions.md
    ac_chain.md
    attack_resolution.md
  spatial/
    grids.md
    rooms.md
    orchestrator.md
    connections.md
  environments/
    ...
```

This makes each concept findable on its own and prevents single-file blobs that grow until no one reads them.

**Issues to file in `rpg-toolkit` (paired):**

1. **"Restructure `docs/architecture/components/` into per-module directories."** Migrate `rulebook-dnd5e.md`, `tools-spatial.md`, `tools-environments.md` etc. into `rulebook/`, `spatial/`, `environments/` directories. Split content along concept boundaries. Smaller modules (core, dice, refs) can stay single-file for now.
2. **"Document the two-level action economy at `docs/architecture/components/rulebook/action_economy.md`."** Promote the explanation from `rulebooks/dnd5e/CLAUDE.md`. First concrete instance of the directory-based pattern. (Depends on #1.)

**Companion issue:** "Add a `docs/CLAUDE.md` (or amend root CLAUDE.md) declaring the harness rule explicitly — architecture/components for inner workings, status.md for current state, quality.md for module scorecards. Knowledge belongs in the harness, in the right slot."

Suspected to be a broader pattern. A janitor pass on toolkit docs would surface other concepts that live only in CLAUDE.md / ADRs and should be promoted into the harness with the new structure.

### Action history (toolkit-side)

`rpg-toolkit/docs/ideas/action-economy-history/` is in-flight. It tracks *what was taken* (Disengage vs Dash) so other rules can query "did they disengage this turn?" Server-side concern only — the wire doesn't need history. v1alpha2 unaffected.

### Walkability

`Hex.terrain` is on the wire. Walkability is server-evaluated against the entity's movement modes. Per-movement-mode walkability rules (flying over VOID, swimming through WATER) are deferred — the server can answer "can this entity step here" with whatever rule shape the toolkit ends up with.

### Movement units

`movement_remaining` in `ActionEconomy` is in "space units" — TBD whether hexes or feet. The toolkit currently uses feet (D&D-native). Web rendering uses hexes. Pick one or carry both. Decision deferred to implementation.

### Geometry persistence backing store

Per-character explored geometry persisting across the campaign needs a persistence layer. Encounters are currently in-memory only in rpg-api. Out of scope for v1alpha2 protos; in scope for the implementation plan.

### Snapshot vs delta detail

The stream lifecycle section says "if the gap is too large, server sends a fresh snapshot." What threshold, what mechanics — implementation detail.

---

## 8. What's NOT in scope for v1alpha2

Listed explicitly so the design doesn't drift:

- **Rulebook abstractions.** v1alpha2 lives in `dnd5e.api.v1alpha2`. Multi-rulebook support is a future v2/v3 concern.
- **Persistence beyond encounter lifetime.** Encounters remain in-memory. Cross-session character state, campaign saves, etc. are separate concerns.
- **Multiplayer auth model.** Identity is via auth header (existing). Permissions and roles are existing infrastructure.
- **Specific spell/ability implementations.** Those live in the toolkit and are referenced by `ref` strings on the wire.

---

## 9. Decisions Log

Decisions made during the brainstorm, recorded so future contributors can see the *why*:

| Decision | Rejected alternatives | Why |
|---|---|---|
| One `EncounterService` | Split Space + Encounter services | Encounter is always primary; Space is data, not a service. Free roam is just a mode. |
| Encounter persists across mode transitions | New encounter per mode | Continuity matters; combat starting mid-exploration shouldn't tear down the world. |
| Flat hex map with optional zones | Structured rooms; flat-only | Web doesn't need rooms; zones cover legitimate zonal needs (lighting, music, narration) without forcing room structure. |
| Tiered visibility (sticky geometry, real-time entities) | Full-state on wire; client-side fog | Cheating-resistant; matches D&D mental model; smaller payloads. |
| Common entity shape (hp, status_effects) + oneof | Per-type entity messages | Conditions and HP are fundamental, not combat-specific. Toolkit's `core.Effect` model affirms. |
| `MoveEntity` for any entity, authority-gated | Separate `MovePlayer`, `MoveMonster` | Same payload → same event/RPC. Authority is a property of the request context. |
| `Interact` with slim response, events tell the story | Discriminated outcome union in response | Response is for the caller only; world changes flow as events that everyone sees. |
| Typed event union with semantic verbs | Generic delta envelope | Animations, sounds, narration key off event type. Generic deltas lose semantic clarity. |
| Action economy mirrors toolkit two-level model | Generic `map<string,int>` budget | Toolkit's primary-vs-capacity split is the right model; capacity map handles rulebook specifics. |
| `AvailableActions` server-pushed list | Web computes availability | Web must not know game rules. Server answers definitively. |
| Continuous map, no gaps between rooms | Rooms with corridors | Map continuity simplifies the wire — opening a door is "barriers removed + visibility expanded," not "new room appended." |
| Clean-break migration | Parallel-run / shim layer | Half-state is the bug source. Cut clean. |
| Client sends proposed path; server corrects via events | Client computes final outcome; server validates only | World contains things the client can't see (undetected traps, hidden terrain). Server is authoritative on what *actually* happens. |
| `Ref{module, type, id}` for typed content; strings only for instance ids | Everything as string | Refs disambiguate across rulebooks, prevent id collisions, and make wire-side intent explicit. Strings are reserved for runtime instance ids and UI text. |
