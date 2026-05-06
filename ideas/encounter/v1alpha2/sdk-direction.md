# Encounter SDK Direction — Brainstorm Snapshot

Date: 2026-05-05
Status: **brainstorming, in-progress** — not a spec yet. Open questions still on the table at the end.
Relationship to siblings:
- `design.md` — the v1alpha2 contract (shipped Phase 1 protos at v0.1.93). This brainstorm doesn't change the proto contract; it changes who emits/consumes it.
- `orchestrator-design.md` — Phase 2 design as drafted in the previous session. **This brainstorm is a re-framing of that Phase 2 work.** Instead of growing more orchestrator code in rpg-api, the orchestrator gets thinner and the new responsibility (encounter lifecycle, action dispatch, event projection) moves into a new SDK layer in rpg-toolkit. If this direction holds, `orchestrator-design.md` §2/§3 get superseded; §4 open questions get re-asked from this seam.

---

## The shift in framing

Today: rpg-api's orchestrator imports toolkit primitives directly — `events.Bus`, character methods, spatial queries — and stitches them together per RPC. The orchestrator both knows the wire protocol AND knows enough rules-shape to glue toolkit pieces. Every new mechanic touches it.

Proposal: insert an **encounter SDK layer** in rpg-toolkit between the orchestrator and the toolkit primitives. The orchestrator becomes a thin shim — auth, persistence, gRPC plumbing. The SDK owns lifecycle, action dispatch, per-player event projection, and event distribution.

**Goal:** simplify the game-server orchestrator. Get consistent event streams out of every action. Make new game servers (or future games) a small lift, because the SDK encapsulates "how to run an encounter."

---

## Architectural decisions made so far

### 1. Encounter is fully transient (data-in / data-out)

Same pattern as `Character`/`BasicEnvironment`/spatial `Room`. Per turn:

```
Load EncounterData from redis → sdk.LoadFromData(data, broker) →
  encounter.<verb>(...) → ToData() → save to redis → return ack
```

The encounter object exists only for the duration of one action. No long-lived in-memory game state in rpg-api.

### 2. Two distinct event systems

- **`events.Bus`** (toolkit's existing in-process synchronous chain) — stays internal to the SDK, used during action resolution to compose modifier chains (Monk WIS-AC, condition stacks, etc.). **Unchanged.** Not the player-facing stream.
- **The Broker** (new) — process-scoped, owned by the game server. Handles per-player gRPC streaming of game events. Transport-agnostic.

### 3. Per-player visibility is enforced server-side; client is dumb pixels

Video game rule #1: never trust the client. Events that a player should not see never reach their stream. The encounter (which has full state) computes per-player projections; the broker fans them out. Players who can't perceive an event simply don't receive it.

Door opens → Bob (across the room) gets `Revealed{ squares: [a few near him] }`. Alice (at the door) gets `Revealed{ squares: [deep into the room] }`. Carol (no LoS) gets nothing. One raw event published; three different deliveries (or two, plus one drop).

### 4. Server holds complexity

Per-player **PerceptionView** is persisted on the encounter (`ToData`). Server emits *deltas* against that view. Reconnect/late-join sends the persisted view as a snapshot — the transport doesn't need history retention.

### 5. Broker takes a Transport at construction

The broker is the SDK's pub/sub facade. It owns active subscriptions in this process and routes events. Underneath, it publishes/subscribes through a pluggable **Transport** — a thin interface (`Publish(channel, payload)` / `Subscribe(channel) → events`) that wraps Redis pubsub, Kafka, or an in-memory map. The broker doesn't know which.

### 6. Visibility model starts simple, grows under one interface

Day 1: ad-hoc projection inside encounter verbs. Persistent perception state is a `PerceptionView` per player on the encounter (cumulative reveals, known entities, active senses, conditions). When rules force richer modeling (long-term awareness, illusions, etc.) it grows under the same `Project` signature.

Visibility/perception is **net-new** in toolkit. No prior model exists. This is a real subdomain we're introducing.

---

## End-to-end shape

### Game server startup

```go
transport := redis.NewTransport(redisClient)   // anything that satisfies the Transport interface
broker := sdk.NewBroker(transport)             // process-scoped, long-lived
// broker is held in the server struct, injected into all handlers
```

### Write path — `TakeAction` handler in rpg-api

```go
func (s *Server) TakeAction(ctx, req) (*Resp, error) {
    // 1. Auth: this player, this encounter, their turn
    // 2. Load
    data := s.encounters.Get(req.EncounterID)
    encounter := sdk.LoadFromData(data, s.broker)

    // 3. Dispatch — orchestrator already knows action type from proto
    switch a := req.Action.(type) {
    case *MoveAction:             encounter.Move(req.PlayerID, a.Path)
    case *AttackAction:           encounter.Attack(req.PlayerID, a.Target, a.Weapon)
    case *ActivateFeatureAction:  encounter.ActivateFeature(req.PlayerID, a.FeatureRef, a.Args)
    case *UseAction:              encounter.UseAction(req.PlayerID, a.ActionRef, a.Args)
    }
    // ↑ during those methods, the encounter computes per-player projections
    //   and publishes ONE raw event with per-player slices to s.broker

    // 4. Save and ack
    s.encounters.Put(req.EncounterID, encounter.ToData())
    return &Resp{Status: "ok"}, nil
}
```

Roughly 15 lines. No event-shaping. No visibility decisions. No turn-state checks beyond auth.

### Read path — `StreamEncounter` handler

```go
func (s *Server) StreamEncounter(req, stream) error {
    // 1. Auth: player is in encounter
    data := s.encounters.Get(req.EncounterID)

    // 2. Snapshot from persisted PerceptionView (player's current truth)
    encounter := sdk.LoadFromData(data, s.broker)
    stream.Send(encounter.SnapshotFor(req.PlayerID))

    // 3. Subscribe via broker; forward per-player projected events
    sub := s.broker.Subscribe(req.EncounterID, req.PlayerID)
    defer sub.Close()
    for evt := range sub.Events() {
        stream.Send(evt)
    }
    return nil
}
```

Roughly 10 lines. Encounter loaded once for the snapshot, then dropped. Broker subscription holds the long-lived per-player channel.

---

## SDK package surface

Package location TBD — either new top-level `rpg-toolkit/encounter/`, or grow inside the existing `rpg-toolkit/game/` package. See open questions.

```go
// Construction
func New(cfg *Config, b *Broker) *Encounter
func LoadFromData(data *Data, b *Broker) (*Encounter, error)

// Setup (used during New)
func (e *Encounter) AddPlayer(p PlayerInput) error
func (e *Encounter) AddMonster(m MonsterInput) error

// Action verbs — orchestrator dispatches to these
func (e *Encounter) Move(playerID PlayerID, path []Hex) error
func (e *Encounter) Attack(playerID PlayerID, target EntityID, weapon Ref) error
func (e *Encounter) ActivateFeature(playerID PlayerID, feat Ref, args FeatureArgs) error
func (e *Encounter) UseAction(playerID PlayerID, action Ref, args ActionArgs) error
func (e *Encounter) Interact(playerID PlayerID, target EntityID) error
func (e *Encounter) SubmitCheck(playerID PlayerID, checkID CheckID, roll Roll) error
func (e *Encounter) EndTurn(playerID PlayerID) error

// View / persistence
func (e *Encounter) SnapshotFor(playerID PlayerID) Snapshot
func (e *Encounter) ToData() *Data
```

Note: **no `Subscribe` on Encounter.** Subscribe is a Broker concern (long-lived, process-scoped); Encounter is transient and mutates+publishes only.

## Transport — the pluggable pub/sub interface

```go
type Transport interface {
    Publish(channel string, payload []byte) error
    Subscribe(channel string) (events <-chan []byte, close func() error, err error)
}

// Implementations:
//   RedisTransport     — wraps redis pubsub (prod default)
//   InMemoryTransport  — map of channels (tests)
//   KafkaTransport     — later if needed
```

Transport is dumb bytes. Doesn't know about encounters, players, or events.

## Broker — process-scoped, owns subscriptions, routes via Transport

```go
type Broker struct {
    transport Transport
    // (encID, playerID) → []Subscription   in-process registry
}

func NewBroker(t Transport) *Broker

// Called by encounter inside verb methods. ONE raw event per action; per-player slices embedded.
func (b *Broker) Publish(raw RawEvent) error

// Called by gRPC StreamEncounter handler. Per-player filtered/extracted stream.
func (b *Broker) Subscribe(encID EncounterID, playerID PlayerID) Subscription
```

Channel keying inside transport: `enc:<encID>` (one channel per encounter). Per-player extraction is broker-side, not transport-side.

## How Broker routes — internal logic

```go
// Long-running goroutine started at NewBroker; one per encounter the broker is aware of
func (b *Broker) listen(encID EncounterID) {
    payloads, _, _ := b.transport.Subscribe("enc:" + encID)
    for bytes := range payloads {
        raw := decode(bytes)
        for _, sub := range b.subscribersFor(encID) {
            slice, ok := raw.PerPlayer[sub.PlayerID]
            if !ok { continue }                  // not visible to this player → drop
            playerEvt := toPlayerEvent(raw.Type, raw.Base, slice)   // mutate a little
            sub.Send(playerEvt)
        }
    }
}
```

**Self-receipt is a non-issue:** this process publishes to transport → transport broadcasts (including back to this process) → broker receives same as remote events → fans out. Single code path. No local-shortcut, no dedupe.

## RawEvent shape

```go
type RawEvent struct {
    EncounterID EncounterID
    Type        EventType                            // e.g., "MoveOccurred", "DoorOpened"
    Base        any                                  // type-specific common payload (proto.Message)
    PerPlayer   map[PlayerID]PlayerSlice             // who-can-see + their slice
    Sequence    uint64                                // monotonic per encounter for client ordering
}

type PlayerSlice struct {
    Revealed   HexSet                                 // newly visible squares this event
    Hidden     HexSet                                 // newly hidden (e.g., light went out)
    Entities   []EntityRef                            // who is now visible / no longer visible
    // type-specific extras as needed
}
```

A player whose ID isn't in `PerPlayer` simply doesn't perceive this event.

## Inside a verb — where projection actually happens

```go
func (e *Encounter) Move(playerID PlayerID, path []Hex) error {
    // 1. Validate (turn, action economy, path legality)
    // 2. Apply state mutation (positions, action economy spend)

    // 3. Compute per-player projections (encounter has full state — perception views, walls, senses)
    perPlayer := map[PlayerID]PlayerSlice{}
    for _, viewer := range e.players {
        slice := perception.Project(MoveOccurred{playerID, path}, viewer.View, e.spatial)
        if slice != nil {
            perPlayer[viewer.ID] = *slice
            viewer.View.Apply(*slice)              // update what they now know
        }
    }

    // 4. Publish ONE raw event
    return e.broker.Publish(RawEvent{
        EncounterID: e.id,
        Type:        "MoveOccurred",
        Base:        MoveBase{Mover: playerID, Path: path},
        PerPlayer:   perPlayer,
        Sequence:    e.nextSeq(),
    })
}
```

`perception.Project` is the new subdomain — its own package (`rpg-toolkit/perception/`), pure function, testable in isolation. Independent of the SDK and broker.

## Persisted state — what `PerceptionView` holds

```go
type PerceptionView struct {
    PlayerID       PlayerID
    RevealedHexes  HexSet                              // cumulative reveal
    KnownEntities  map[EntityID]EntityKnowledge        // last-seen pos, anonymous-vs-identified
    ActiveSenses   []Sense                              // darkvision range, blindsight
    Conditions     []Ref                                // blinded, etc.
}
```

Per-player view rides in `EncounterData`. `LoadFromData` rehydrates them; `ToData` serializes them. Reconnect resyncs from the view, no event replay needed.

## Test seam

```go
broker := sdk.NewBroker(InMemoryTransport{})              // map of channels, no infra
enc, _ := encounter.LoadFromData(testData, broker)
sub := broker.Subscribe(testData.ID, "alice")
enc.Move("bob", path)
evt := <-sub.Events()                                      // assert visibility/content for alice
```

No infrastructure required for unit tests.

---

## What it gives the orchestrator

- `TakeAction` handler ≈ 15 lines: auth, load, dispatch, save, ack
- `StreamEncounter` handler ≈ 10 lines: auth, snapshot, subscribe, forward
- No event-shaping. No visibility decisions. No knowledge of rules.

The boundary rule (`API never knows what "rage" does`) holds — and tightens. The orchestrator no longer constructs events or makes visibility decisions; it dispatches verbs and forwards what comes out the broker.

---

## Constraints from existing toolkit state

(From the harness-doc Explore pass on `rpg-toolkit/docs/`.)

- **`ToData/LoadFromData` is the canonical persistence pattern** in toolkit — Character, Draft, BasicEnvironment, spatial Room all use it. We're following the established shape, not inventing one.
- **No "Encounter" aggregate exists yet** — this fills a real gap, no name collision risk.
- **`game.Context` exists** in `rpg-toolkit/game/` as a small facade (just `context.go`). The encounter SDK could grow there or stand on its own.
- **Events package rewrite (issue #617)** left some mechanics modules with stale `replace` directives that can't build. Likely needs cleanup before this lands or as part of it.
- **No visibility/perception model exists** anywhere in toolkit. We're introducing a new subdomain (`rpg-toolkit/perception/`).
- **Dungeon is moving from `rulebooks/dnd5e/dungeon/` to `tools/dungeon/`.** Coordinate if SDK depends on dungeon logic.

---

## Open questions (still to settle before writing the spec)

### Q1 — `RawEvent.Base` and `PlayerSlice` shape: toolkit-domain types or proto types?

Toolkit historically does not import proto packages (boundary cleanliness). Proposed: domain types in toolkit; rpg-api adapts to `v1alpha2.EncounterEvent` at the wire boundary. Costs an adapter layer; keeps toolkit free of proto coupling.

Trade-off: 1:1 field mapping is mechanical work but stable. The shape of `v1alpha2` events is already settled and not expected to drift much, so the adapter is small.

**Lean: domain types in toolkit. Confirm.**

### Q2 — Where does the action catalog live?

Two shapes for the action surface:

- **A) Verb methods** (drawn above) — `encounter.Move(...)`, `encounter.Attack(...)`, etc. Orchestrator switches on action type from the proto. Adding a new action type touches the SDK signature *and* the orchestrator switch.
- **B) Sealed-sum dispatch** — `encounter.Apply(playerID, ActionDescriptor)` where `ActionDescriptor` is a Go sum type. SDK owns the dispatch table. New actions land in toolkit only; orchestrator doesn't change.

**Lean: A initially** — orchestrator already knows action type from proto, verb methods read clearly. Revisit if catalog grows past ~10 verbs.

### Q3 — Package location

- **`rpg-toolkit/encounter/`** (new top-level) — discoverable, signals importance.
- **Grow `rpg-toolkit/game/`** (existing, currently just `Context`) — "how you run a game" is conceptually what `game/` is about; would house `Encounter`, `PerceptionView`, `Broker`.

**Lean: grow `game/`.** It pulls more weight that way and matches the conceptual scope.

### Q4 — Recipient identity beyond PlayerID

`RawEvent.PerPlayer` is keyed by `PlayerID` today. Real games have GMs, possibly NPC-as-spectator, possibly observers. Probably want a `RecipientID` alias or interface so the SDK is agnostic about who counts as a recipient. Day 1 = just PlayerIDs; the type can grow.

**Lean: ship Day 1 with `PlayerID`; introduce `RecipientID` when GM seats land.** Not a blocker.

---

## Pickup for next session

1. Settle Q1–Q4 above.
2. Decide whether this brainstorm replaces `orchestrator-design.md` Phase 2 or sits alongside it. (My read: replaces — same wave, different framing.)
3. Write the formal design doc (probably `design-v2.md` here, or a new sibling).
4. Spec review loop.
5. Implementation plan.

## What this brainstorm did *not* settle

- Concrete v1alpha1 → v1alpha2 cutover sequencing (orchestrator migration order, deprecation).
- Specific contents of `EncounterData` beyond `PerceptionView` (entity list, action economy, turn state — straightforward, but enumerate).
- Backend choice for prod (Redis pubsub assumed, but Kafka may matter once playtests scale; not a day-1 decision).
- How the existing `events.Bus` mechanics module rewrite (#617) interacts with this — does it block, or do we land alongside?
- **Failure semantics**: what happens if the transport fails mid-action? Action succeeds, event lost? Action rolls back? Probably: action succeeds, event delivery is best-effort with reconnect-via-snapshot as the safety net. But document.
- **Multi-process scale-out**: does anything need to change if rpg-api runs N replicas? Mostly no — Redis pubsub fans out across processes; broker self-receipt is uniform. But concurrent-write lock semantics on the same encounter need a thought.
- **Snapshot completeness**: what does `SnapshotFor(playerID)` actually serialize? The full PerceptionView, plus current entity positions visible to that player, plus turn state? Enumerate when writing the spec.
