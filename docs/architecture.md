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
Toolkit Type (character.Data, combat.AttackResult)
    -> [JSON in Redis]
    -> Toolkit Type
    -> [Converter in Handler]
    -> Proto Type
    -> [gRPC over wire]
    -> Proto Type in React
```

ONE conversion point: Toolkit <-> Proto (in handler converters).

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
