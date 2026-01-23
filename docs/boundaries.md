# Layer Boundaries

What each layer knows and doesn't know. When in doubt, check here.

## rpg-dnd5e-web (Renderer)

### Knows
- Proto message shapes (generated types)
- How to render game state visually
- Discord Activity SDK integration
- User intent (which button was clicked)
- References/keys for game content

### Does NOT Know
- What "rage" does mechanically
- How to calculate damage
- Whether an action is legal
- Dice rolling logic
- Game rules of any kind

### Sends to API
- References: `"dnd5e:features:rage"`, `"dnd5e:weapons:greataxe"`
- Intent: "attack target X", "move to position Y", "activate feature Z"
- IDs: character_id, encounter_id, target_id

### Never Sends
- Calculated damage values
- Modified stats
- Rule interpretations
- "Trust me, this is valid"

---

## rpg-api (Orchestrator)

### Knows
- Feature keys and character IDs
- How to store/retrieve from Redis
- How to call toolkit functions
- How to broadcast events to all players
- Request validation (does this character exist? is it their turn?)

### Does NOT Know
- What rage damage bonus is
- How attack rolls work
- Whether a spell can target that creature
- Any D&D 5e rule

### Does
- Receives RPC -> validates request shape -> calls toolkit -> stores result -> broadcasts events
- Converts toolkit types to proto types (ONE conversion point in handler converters)
- Manages encounter lifecycle (create, turn order, end)

### Never Does
- `if character.HasCondition("raging") { damage += 2 }` (WRONG - that's a rule)
- Dice rolling for game mechanics
- Feature/condition logic
- Any `switch` on feature/condition names

---

## rpg-toolkit (Rules Engine)

### Knows
- All D&D 5e rules
- How features activate and conditions apply
- Damage calculation chains
- Monster behavior and targeting
- Spatial mechanics (hex grids, pathfinding, line of sight)
- Dungeon generation and room connections

### Does NOT Know
- Where data is stored
- How to talk to clients
- What proto messages look like
- User authentication
- Redis, gRPC, Discord

### Returns
- Rich structured results with breakdowns
- Updated game state (conditions applied, resources consumed)
- Events for the bus (what happened, who was affected)

### Never Does
- Database calls
- Network operations
- Proto marshaling
- User-facing error messages

---

## rpg-api-protos (Contracts)

### Is
- The API shape definition
- Source of truth for what goes over the wire
- Generated into Go and TypeScript

### Is NOT
- Business logic
- Validation rules
- Implementation details

### Rules
- Proto changes go to feature branches, CI generates code
- Messages mirror toolkit types where possible
- Rich responses with breakdowns (not just totals)
- Enums for anything the client needs to render differently

---

## The Smell Tests

**"Is this in the right layer?"**

1. Does the API have a `switch` on feature/condition names? -> Move to toolkit
2. Is the client calculating a value? -> Move to server
3. Is the toolkit importing `net/http` or `redis`? -> Move to API
4. Is the API returning just a number without breakdown? -> Add detail
5. Is there a duplicate type definition? -> Use toolkit's canonical type
6. Is the client making a game decision (can I do this)? -> That's server-side validation
