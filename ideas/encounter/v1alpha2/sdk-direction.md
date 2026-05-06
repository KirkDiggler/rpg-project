# Encounter SDK Direction — Brainstorm Snapshot

Date: 2026-05-06
Status: **design settled — ready to promote to a formal spec.** All design questions resolved. Items still needing enumeration before implementation are in "What this brainstorm did not settle" at the end.
Relationship to siblings:
- `design.md` — the v1alpha2 contract (shipped Phase 1 protos at v0.1.93). This brainstorm doesn't change the proto contract; it changes who emits/consumes it.
- `orchestrator-design.md` — Phase 2 design as drafted in the previous session. **This brainstorm supersedes it.** Same v1alpha2 wave, different framing of Phase 2: encounter lifecycle and per-player event distribution move into a new toolkit SDK; rpg-api orchestrator becomes a thin shim.

---

## The shift in framing

Today: rpg-api's orchestrator imports toolkit primitives directly — `events.Bus`, character methods, spatial queries — and stitches them together per RPC. The orchestrator both knows the wire protocol AND knows enough rules-shape to glue toolkit pieces. Every new mechanic touches it.

Proposal: insert an **encounter SDK layer** in rpg-toolkit between the orchestrator and the toolkit primitives. The orchestrator becomes a thin shim — auth, persistence, gRPC plumbing, proto↔domain shape mapping. The SDK owns lifecycle, action dispatch, per-player event projection, and event distribution.

**Goal:** simplify the game-server orchestrator. Get consistent event streams out of every action. Make new game servers (or future games) a small lift, because the SDK encapsulates "how to run an encounter."

---

## Architectural decisions

### 1. Encounter is fully transient (data-in / data-out)

Same pattern as `Character`/`BasicEnvironment`/spatial `Room`. Per turn:

```
Load EncounterData from redis → sdk.LoadFromData(data, broker) →
  encounter.<verb>(...) → ToData() → save to redis → return ack
```

The encounter object exists only for the duration of one action. No long-lived in-memory game state in rpg-api.

### 2. Two distinct event systems, parallel taxonomies

- **`events.Bus`** (toolkit's existing in-process synchronous chain) — stays internal to the SDK, used during action resolution to compose modifier chains (Monk WIS-AC, condition stacks, etc.). Fine-grained, mutate-during-dispatch (`PreAttackRolled`, `DamageRolled`, `ConditionApplied`). **Unchanged.** Not the player-facing stream.
- **The Broker + EncounterEvent taxonomy** (new) — process-scoped game-server component, handles per-player gRPC streaming. Coarse-grained, immutable post-resolution narration (`MoveEvent`, `AttackEvent`, `DoorOpenedEvent`, `ModeChangedEvent`...).

Same world-action vocabulary; different shapes for different jobs. They live in separate packages.

### 3. EncounterEvents span all encounter modes

An encounter is a *scene* (a hex map with entities and state). Encounter mode (`COMBAT`, `EXPLORATION`/free-roam, `SOCIAL`) defines whether turn order is enforced and which verbs are available — but the event taxonomy is mode-spanning.

- Mode-spanning events: `MoveEvent`, `DoorOpenedEvent`, `InteractEvent`, `RevealedEvent`.
- Mode-specific events: `AttackEvent` and the action-economy events tend to be combat-mode.
- Mode-transition events: `ModeChangedEvent`, `InitiativeStartedEvent`, `EncounterEndedEvent`.

`encounter/` (not `combat/`) is the right package scope precisely because free-roam events need the same broker plumbing as combat events.

### 4. Per-player visibility is enforced server-side; client is dumb pixels

Video game rule #1: never trust the client. Events that a player should not see never reach their stream. The encounter (which has full state) computes per-player projections; the broker fans them out by audience. Players who can't perceive an event simply don't receive it.

Door opens → Bob (across the room) gets `DoorOpenedEvent` with a few revealed squares. Alice (at the door) gets one with deep reveal into the room. Carol (no LoS) receives nothing. One typed event published; broker delivers per audience.

### 5. Server holds complexity

Per-player **PerceptionView** is persisted on the encounter (`ToData`). Server emits *deltas* against that view. Reconnect/late-join sends the persisted view as a snapshot — the transport doesn't need history retention.

### 6. Broker takes a Transport at construction

The broker is the SDK's pub/sub facade. It owns active in-process subscriptions and routes events. Underneath, it publishes/subscribes through a pluggable **Transport** — a thin interface (`Publish(channel, payload)` / `Subscribe(channel) → events`) that wraps Redis pubsub, Kafka, or an in-memory map. The broker doesn't know which.

### 7. Events are typed concretes under a sealed interface

Following the AWS v2 SDK `AttributeValue` pattern: a sealed `EncounterEvent` interface with an unexported marker method, and one concrete struct per world-action. No generic envelope, no opaque payload. Type-safe, sum-typed, compile-time bounded. (Detailed in the events section below.)

### 8. Visibility model starts simple, grows under one interface

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
    // 1. Auth: this player, this encounter, their turn (when in combat mode)
    // 2. Load
    data := s.encounters.Get(req.EncounterID)
    encounter := sdk.LoadFromData(data, s.broker)

    // 3. Dispatch — orchestrator already knows action type from proto.
    //    Handler also adapts proto → toolkit types here (proto.Weapon → toolkit.Ref, etc.)
    switch a := req.Action.(type) {
    case *MoveAction:
        encounter.Move(req.PlayerID, hexesFromProto(a.Path))
    case *AttackAction:
        encounter.Attack(req.PlayerID, entityFromProto(a.Target), refFromProto(a.Weapon))
    case *ActivateFeatureAction:
        encounter.ActivateFeature(req.PlayerID, refFromProto(a.FeatureRef), a.Args)
    case *UseAction:
        encounter.UseAction(req.PlayerID, refFromProto(a.ActionRef), a.Args)
    }
    // ↑ during those methods, the encounter computes per-player projections
    //   and publishes typed EncounterEvents through s.broker

    // 4. Save and ack
    s.encounters.Put(req.EncounterID, encounter.ToData())
    return &Resp{Status: "ok"}, nil
}
```

The handler does proto ↔ toolkit shape mapping (mechanical, no rules logic). Otherwise: auth, load, dispatch, save, ack.

### Read path — `StreamEncounter` handler

```go
func (s *Server) StreamEncounter(req, stream) error {
    // 1. Auth: player is in encounter
    data := s.encounters.Get(req.EncounterID)

    // 2. Snapshot from persisted PerceptionView (player's current truth)
    encounter := sdk.LoadFromData(data, s.broker)
    stream.Send(snapshotToProto(encounter.SnapshotFor(req.PlayerID)))

    // 3. Subscribe via broker; type-switch each event to its proto shape
    sub := s.broker.Subscribe(req.EncounterID, req.PlayerID)
    defer sub.Close()
    for evt := range sub.Events() {
        stream.Send(eventToProto(evt, req.PlayerID))   // proto adapter at the boundary
    }
    return nil
}

func eventToProto(evt events.EncounterEvent, p PlayerID) *v1alpha2.EncounterEvent {
    switch e := evt.(type) {
    case *events.MoveEvent:           return toMoveProto(e, p)
    case *events.AttackEvent:         return toAttackProto(e, p)
    case *events.DoorOpenedEvent:     return toDoorOpenedProto(e, p)
    case *events.ModeChangedEvent:    return toModeChangedProto(e, p)
    // ... one case per concrete event
    }
}
```

Handler adapter pulls each player's slice from the typed event's `PerPlayer` map and shapes a v1alpha2 proto for the wire.

---

## Events: sealed `EncounterEvent`, typed concretes

Lives in `rpg-toolkit/encounter/events/`.

```go
package events

// Sealed: only types in this package can implement, because the marker is unexported
// and external packages cannot define a method with this fully-qualified name.
type EncounterEvent interface {
    isEncounterEvent()
    EncounterID() EncounterID
    Sequence()    uint64
    Audience()    AudienceSet  // who can perceive — broker uses this for fanout
}

type AudienceSet []PlayerID  // slice is enough; broker just iterates

// One concrete type per world-action
type MoveEvent struct {
    encID     EncounterID
    seq       uint64
    Mover     EntityID
    Path      []Hex
    PerPlayer map[PlayerID]MovePlayerSlice
}
func (*MoveEvent) isEncounterEvent()             {}
func (e *MoveEvent) EncounterID() EncounterID    { return e.encID }
func (e *MoveEvent) Sequence() uint64            { return e.seq }
func (e *MoveEvent) Audience() AudienceSet       { return audienceFrom(e.PerPlayer) }

type MovePlayerSlice struct {
    Revealed     HexSet
    Hidden       HexSet
    SeenSegments []Hex   // which parts of the path this player saw
}

type AttackEvent struct {
    encID     EncounterID
    seq       uint64
    Attacker  EntityID
    Target    EntityID
    Weapon    Ref
    Result    AttackResult
    PerPlayer map[PlayerID]AttackPlayerSlice
}
func (*AttackEvent) isEncounterEvent()           {}
func (e *AttackEvent) EncounterID() EncounterID  { return e.encID }
func (e *AttackEvent) Sequence() uint64          { return e.seq }
func (e *AttackEvent) Audience() AudienceSet     { return audienceFrom(e.PerPlayer) }

type AttackPlayerSlice struct {
    Visible      bool          // does this player see the attack at all?
    DamageHidden bool          // do they see numbers, or just "the goblin took damage"?
    // type-specific extras
}

type DoorOpenedEvent struct { /* same shape */ }
type ModeChangedEvent struct { /* From, To, Initiative */ }

// ...one concrete per verb / world-action
```

**Why each concrete defines its own marker:**
A `BaseEvent` struct embedded into each concrete would be less boilerplate but **breaks the seal** — external packages could embed `BaseEvent` and inadvertently satisfy `EncounterEvent`. AWS v2 SDK takes the same call: each concrete defines its own `is<X>` marker. The four-method-per-type boilerplate is the cost of the seal. Worth it.

**Future extraction of a generic `Event`.**
Today `EncounterEvent` is the only flavor. Later, if we want the broker to carry non-encounter events (lobby messages, system notifications), we can extract:

```go
// Future
type Event interface {
    isEvent()
    ChannelKey() string
    Audience()    AudienceSet
}

type EncounterEvent interface {
    Event
    EncounterID() EncounterID
}
```

Existing concretes get a one-liner `func (*MoveEvent) isEvent() {}` added — additive, no consumer breakage. Go's structural interface satisfaction makes layering free to defer.

---

## SDK package surface

`rpg-toolkit/encounter/` (top-level package).

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

The encounter SDK *uses* `game.Context` internally (chain handlers query it), but doesn't share a package with it. `game.Context` stays in `rpg-toolkit/game/` as the chain-handler query facade.

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

// Called by encounter inside verb methods. Accepts any sealed EncounterEvent.
func (b *Broker) Publish(evt events.EncounterEvent) error

// Called by gRPC StreamEncounter handler.
func (b *Broker) Subscribe(encID EncounterID, playerID PlayerID) Subscription
```

Channel keying inside transport: `enc:<encID>` (one channel per encounter). Per-player audience filtering happens broker-side.

## How Broker routes — internal logic

```go
// Long-running goroutine started at NewBroker; one per encounter the broker is aware of
func (b *Broker) listen(encID EncounterID) {
    payloads, _, _ := b.transport.Subscribe("enc:" + string(encID))
    for bytes := range payloads {
        evt := decode(bytes)                         // sealed EncounterEvent (via type discriminator in wire format)
        for _, playerID := range evt.Audience() {    // broker is type-agnostic — never type-switches
            for _, sub := range b.subscribersFor(encID, playerID) {
                sub.Send(evt)                        // ship the typed event; consumer type-switches
            }
        }
    }
}
```

The broker NEVER type-switches events — it routes by `EncounterID()` and `Audience()` only. Type-switching only happens at the gRPC handler boundary, mapping each concrete event to its proto.

**Self-receipt is a non-issue:** this process publishes to transport → transport broadcasts (incl. back to this process) → broker receives same as remote events → fans out. Single code path. No local-shortcut, no dedupe.

**Wire encoding:** `RedisTransport` (and any other Transport impl) uses an internal codec with a type discriminator field — JSON like `{"_type":"MoveEvent", ...}` or protobuf-internal. That's an internal codec concern; it doesn't pollute the event types themselves.

## Inside a verb — where projection happens

```go
func (e *Encounter) Move(playerID PlayerID, path []Hex) error {
    // 1. Validate (turn ordering when in combat mode, action economy, path legality)
    // 2. Apply state mutation (positions, action economy spend)

    // 3. Compute per-player projections — encounter has full state
    //    (perception views, walls, senses, conditions)
    perPlayer := map[PlayerID]events.MovePlayerSlice{}
    for _, viewer := range e.players {
        slice := perception.ProjectMove(playerID, path, viewer.View, e.spatial)
        if slice != nil {
            perPlayer[viewer.ID] = *slice
            viewer.View.ApplyMove(*slice)            // update what they now know
        }
    }

    // 4. Publish typed event
    return e.broker.Publish(events.NewMoveEvent(e.id, e.nextSeq(), playerID, path, perPlayer))
}
```

`perception.ProjectMove` (and friends) live in their own package — `rpg-toolkit/perception/`, pure functions, testable in isolation. Independent of the SDK and broker.

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
evt := <-sub.Events()
move, ok := evt.(*events.MoveEvent)                       // type-assert to concrete
require.True(t, ok)
require.Contains(t, move.PerPlayer, PlayerID("alice"))
```

No infrastructure required for unit tests.

---

## What it gives the orchestrator

- `TakeAction` handler: auth, load, proto→toolkit map, dispatch, save, ack.
- `StreamEncounter` handler: auth, snapshot, subscribe, type-switch-to-proto, forward.
- No event-shaping. No visibility decisions. No knowledge of rules.

The boundary rule (`API never knows what "rage" does`) holds — and tightens. The orchestrator no longer constructs events or makes visibility decisions; it dispatches verbs and adapts shapes.

---

## Constraints from existing toolkit state

(From the harness-doc Explore pass on `rpg-toolkit/docs/`.)

- **`ToData/LoadFromData` is the canonical persistence pattern** in toolkit — Character, Draft, BasicEnvironment, spatial Room all use it. We're following the established shape.
- **No "Encounter" aggregate exists yet** — this fills a real gap, no name collision risk.
- **`game.Context` exists** in `rpg-toolkit/game/` as the chain-handler query facade. Different audience from this SDK; unchanged by this work.
- **Events package rewrite (issue #617)** left some mechanics modules with stale `replace` directives that can't build. Likely needs cleanup before this lands or as part of it.
- **No visibility/perception model exists** anywhere in toolkit. We're introducing a new subdomain (`rpg-toolkit/perception/`).
- **Dungeon is moving from `rulebooks/dnd5e/dungeon/` to `tools/dungeon/`.** Coordinate if SDK depends on dungeon logic.

---

## Decisions settled

- **Action surface**: verb methods on `Encounter` (`Move`, `Attack`, `ActivateFeature`, `UseAction`, `Interact`, `SubmitCheck`, `EndTurn`). Orchestrator switches on action type from proto. (Was Q1.)
- **Package location**: top-level `rpg-toolkit/encounter/` for SDK + events + broker. `game.Context` stays in `rpg-toolkit/game/` as the chain-handler query facade. Different audiences, different lifecycles, different packages. (Was Q2.)
- **Recipient identity**: `PlayerID` Day 1; introduce `RecipientID` alias when GM seats / NPC observers land. (Was Q3.)
- **Event taxonomy**: sealed `EncounterEvent` interface (AWS v2 SDK pattern), one concrete type per world-action. No generic envelope.
- **Event scope**: encounter events are mode-spanning (combat, free-roam, social) — not combat-only. `encounter/` package name reflects this.
- **Bus vs Broker relationship**: parallel taxonomies. Bus events stay fine-grained for chain composition; Broker events are coarse post-resolution narration. Different shapes, different jobs.
- **`Sequence`**: assigned by the encounter at publish time (encounter is the source of truth for ordering). On the interface for cross-cutting access (logging, tracing, dead-letter handling).
- **Generic `Event` interface extraction**: deferred. Stay collapsed under `EncounterEvent` until a non-encounter use case shows up. Go interface satisfaction makes future extraction additive (one-liner `isEvent()` per concrete).
- **Proto adapter location**: at the gRPC handler boundary. Handler does proto ↔ toolkit shape mapping (mechanical, no rules logic). Toolkit doesn't import proto packages.

---

## Pickup for next session

1. This brainstorm supersedes `orchestrator-design.md` Phase 2. Note that explicitly when promoting (or update orchestrator-design.md with a pointer to this file).
2. Promote to formal design doc — new sibling, e.g. `design-encounter-sdk.md` or `design-v2.md`.
3. Spec review loop.
4. Implementation plan (see writing-plans skill).
5. First implementation slice — most likely: define `EncounterEvent` interface + first 3-4 concrete event types + `Broker` skeleton with `InMemoryTransport`. Get the subscribe/publish loop working end-to-end before wiring rules.

## What this brainstorm did *not* settle (need enumeration before/during spec)

- **Concrete `v1alpha1` → `v1alpha2` cutover sequencing** — orchestrator migration order, deprecation, rollout.
- **Full contents of `EncounterData`** — beyond `PerceptionView`, the encounter persists: entity list (positions, HP, conditions), action economy state, turn state, **monster AI / behavior state (last damager, marked, threat tables, target memory)**, mode state, encounter map / spatial state. Enumerate when writing the spec.
- **Full `EncounterEvent` catalog** — enumerate the concrete types. Combat: Move, Attack, ActivateFeature, UseAction, Interact, ConditionApplied, ConditionRemoved, EndTurn, InitiativeRolled. Free-roam: Move, OpenDoor, Search, Reveal. Mode-transition: ModeChanged, EncounterStarted, EncounterEnded. Plus per-event `PerPlayer` slice shapes.
- **AI decision logic location** — the SDK doesn't *run* AI; it calls into rules code (`mechanics/`, `behavior/`, or per-rulebook). Needs to land somewhere; out of scope for this SDK design but part of the larger picture.
- **Backend choice for prod** — Redis pubsub assumed, but Kafka may matter once playtests scale; not a Day-1 decision.
- **Issue #617 interaction** — events package rewrite left stale `replace` directives in mechanics modules. Does it block this, or do we land alongside?
- **Failure semantics** — what happens if the transport fails mid-action? Action succeeds, event lost? Action rolls back? Probably: action succeeds, event delivery best-effort, reconnect-via-snapshot is the safety net. Document.
- **Multi-process scale-out** — Redis pubsub fans out cleanly; broker self-receipt is uniform. But concurrent-write lock semantics on the same encounter need a thought.
- **Snapshot completeness** — what `SnapshotFor(playerID)` actually serializes. Full `PerceptionView`, plus current entity positions visible to that player, plus turn state. Enumerate.
- **Wire codec for Transport** — JSON with type discriminator vs protobuf-internal vs gob. Affects portability of Redis channel data and ability to inspect via redis-cli. Probably JSON for debuggability; revisit if size matters.
