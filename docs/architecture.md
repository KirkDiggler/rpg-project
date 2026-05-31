# Architecture

## The Three Layers

```
rpg-dnd5e-web (Renderer)
  - Untrusted client
  - Renders data from API
  - Sends references + intent (never calculations)
  - Uses proto types directly
       |
       | (References + Intent)
       v
rpg-api (Orchestrator)
  - Trusted server, game-agnostic
  - Stores and retrieves state (Redis)
  - Calls toolkit by key, returns results
  - Broadcasts events for multiplayer
  - Layers: Handlers -> Orchestrators -> Repositories
       |
       | (Toolkit function calls)
       v
rpg-toolkit (Rules Engine)
  - Source of truth for D&D 5e mechanics
  - Event-driven combat (chains, topics)
  - Returns rich breakdowns for rendering
  - Layers: Core -> Mechanics -> Tools -> Rulebooks/dnd5e
```

## Data Flow

```
Toolkit entity
    -> ToData()                (flatten to plain *Data)
    -> [JSON in Redis]
    -> LoadFromData(ctx, bus)  (rehydrate LIVE onto the bus — NOT a raw unmarshal)
    -> Toolkit entity
    -> [Converter in Handler]
    -> Proto Type
    -> [gRPC over wire]
    -> Proto Type in React
```

ONE conversion point: Toolkit <-> Proto (in handler converters).
The persist hop has its own contract — see below.

## The Persistence & Hydration Contract (`ToData` / `LoadFromData`)

This is the single most important contract in the platform, and the one most easily missed. The toolkit holds **no storage of its own** — the *only* way game state crosses an RPC boundary is this round-trip:

```
toolkit entity  --ToData()-->         plain *Data (JSON-tagged)  -->  rpg-api persists opaque JSON
rpg-api reads JSON  -->  *Data  --LoadFromData(ctx, bus)-->        LIVE, bus-subscribed toolkit entity
```

**`LoadFromData` is not deserialization.** A plain `json.Unmarshal` gives you a dead struct. `LoadFromData` reconstructs each condition and calls `.Apply(ctx, bus)` — **which subscribes its handlers to the event bus.** It returns an entity that is *already live* and will react to events. Unmarshal it by hand and you get something that looks right and does nothing.

**It cascades.** `Encounter.LoadFromData` creates **one fresh bus**, then hydrates every combatant onto *that* bus (`character.LoadFromData` / `monster.LoadFromData`). One call brings the whole encounter alive, once.

**It is a convention, not an interface — which is exactly why it is missable.** There is no shared `interface { ToData(); LoadFromData() }` to grep for; each stateful type implements the pair by hand with its own `*Data` struct. If you don't already know the pattern by name, nothing in the code points you to it. *That gap caused a real false start: a consumer wrote a plain unmarshal + a per-call re-loader, never learning the contract — and walked straight into #684.*

### Load once = subscribe once (the #684 cure)

A condition's `Apply()` runs **only** during `LoadFromData`. So the encounter is loaded **exactly once per orchestrator method**, and every verb after that reuses the *held* entity — never re-loads.

```
orchestrator.<Verb>(id):

  load(id)                                  # the SINGLE load path
    enc = encounter.LoadFromData(ctx, data, broker)
        |- creates ONE fresh bus
        `- cascade -> character/monster.LoadFromData(...)
              `- condition.Apply(ctx, bus)  # subscribes handlers ONCE
                                            # (a plain unmarshal skips this -> dead struct)

  enc.<Verb>(...)                           # toolkit runs the rules on the live bus
        events fire -> rpg-api translates -> proto events out (to all players)

  data = enc.ToData()                       # flatten live state back to plain Data
  if enc.SyncErr() != nil { ... }           # write-back errors surface here
  repo.Save(json(data))                     # rpg-api persists the opaque blob
```

Re-loading mid-method re-runs `Apply()` on the same long-lived bus -> duplicate subscriptions -> the `"modifier ID already exists"` double-apply class (#684). One `load(id)` makes that **structurally impossible** — which is why the encounter orchestrator has exactly one private `load(id)` per method.

### Write-back is not dirty-gated

`ToData` write-back is deliberately **not** `IsDirty()`-gated: the rulebook dirty flag tracks only HP, not condition-state flips (e.g. `SneakAttack.UsedThisTurn`). The orchestrator checks `enc.SyncErr()` after `ToData()` and before saving.

## Multiplayer Model

Only one player sees the RPC response. Everyone else learns through events.

```
Player A: ActivateFeature("rage") -> gets response with result
Player B: receives FeatureActivatedEvent via stream
Player C: receives FeatureActivatedEvent via stream
```

This is why the event bus matters. The API:
1. Calls toolkit to execute the action
2. Returns response to caller
3. Broadcasts events to all participants

## Combat System

```
Player Action (Attack, ActivateFeature, Move, EndTurn)
    -> API receives RPC
    -> Toolkit resolves (chains fire, events publish)
    -> API stores updated state
    -> API returns result to caller
    -> API broadcasts events to all players
    -> Monster turns execute automatically on EndTurn
    -> Monster results broadcast as events
```

## Multi-Room Dungeons

Rooms have absolute positions in the dungeon. The toolkit handles this:
- Each room knows its offset in dungeon-space
- Entities have positions relative to their room
- Room transitions are logical connections (door, stairs, portal)
- The UI renders rooms at their absolute positions (no client-side offset math)

## Trust Model

| Layer | Trusts | Validates |
|-------|--------|-----------|
| Web | Nothing - sends intent only | Nothing - server decides |
| API | Toolkit for rules | Request shape, auth, existence |
| Toolkit | Its own calculations | Preconditions (can activate? has uses?) |
