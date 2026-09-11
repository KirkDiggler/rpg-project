# Layer 1 starting point: before the architecture existed

**Status:** Raw source notes for the first rpg-toolkit architecture page. Not approved prose. Not published.

## The opening worth preserving

The first thing designed for the rules engine was its storage.

That is funny now because `rpg-toolkit` does not persist anything.

The initial assumption was that repositories would be injected into rulebook or toolkit modules. A room generator would receive a `RoomRepository`. Generic storage adapters would make Redis, Mongo, memory, or files interchangeable. The toolkit might provide standard CRUD and individual systems could extend it.

None of the current boundaries existed yet. It was genuinely unknown how the pieces would hook together.

## What the record actually shows

### 2025-06-25: the repository begins with one README

The first commit contained only `README.md`. It described an architecture that did not exist yet:

```text
core/
  entities/
  state/
  events/
  storage/

systems/
  combat/
  inventory/
  progression/
  world/
  dialogue/
  quests/

games/
  d20/
    dnd5e/
    pathfinder/
  pbta/
  fate/
```

The project was still described as TypeScript-first. There was no Go code, no `core` module, no event bus, and no implemented rulebook.

The broad dream was already present: shared RPG pieces underneath game-specific implementations, usable from a Discord bot, web app, Unity game, or something else. The seams were only folder names.

### 2025-06-26: storage consumes the design

The next day added and replaced six different storage documents:

- persistence-pattern comparisons;
- storage middleware;
- two Go persistence proposals;
- a repository pattern;
- a Redis implementation;
- minimal and simplified repository variants.

The working idea became:

```go
type RoomRepository interface {
    Save(ctx context.Context, room *Room) error
    FindByID(ctx context.Context, id string) (*Room, error)
}

roomRepo := storage.NewStandardRepo[*Room](redisAdapter, "rooms")

generator := rooms.NewRoomGenerator(rooms.GeneratorConfig{
    RoomRepo: roomRepo,
})
```

Storage adapters, generic repositories, and repository injection looked like the foundation. The intended rule was that the backing datastore should not matter. A module would declare the access pattern it needed, such as `Save` and `FindByID`, and any Redis, Mongo, memory, or file implementation satisfying that interface could be injected.

### 2025-07-03: the uncertainty is finally written honestly

The first Architectural Dragons note asks:

> Where does game state live? Who owns the truth?

It admits:

> We've been glossing over this. Need storage interfaces before we go much further.

It also says game-specific mechanics such as advantage and disadvantage should be injected from rulebook modules, with core providing hooks rather than implementations.

So the rulebook idea existed. Shared RPG infrastructure existed as an intention. But there was still no stable answer for:

- who owned state;
- who persisted it;
- what core meant;
- what belonged to a rulebook;
- whether generic systems interpreted game data;
- how runtime behavior came back after loading data.

## The actual starting pain: character creation in the Discord bot

The practical problem predated the toolkit.

The Discord bot tried to drive character creation from `dnd5eapi.co` data. The D&D reference database used by the bot was Mongo and held character-creation data alongside other D&D content. The bot read from that database; it was not saving player choices into Mongo.

The source choices array contained polymorphic and nested shapes:

- choose skills from a list;
- choose equipment bundles;
- choose from a category;
- choose an option that contains another choice;
- carry item counts;
- distinguish automatic grants from player decisions.

The problem was not whether Mongo could store those shapes. It already did. The problem was what every consumer had to know after reading them.

The server still needed to understand:

- what kind of choice this was;
- which options were legal;
- where a choice came from;
- what to remove if race or class changed;
- what the selected data meant to the character.

The dream was pure data-driven character creation. The result was a generic choices array whose meaning leaked into every consumer.

## The first freedom

The first real architectural freedom came from allowing the D&D rulebook to code what a class was and what choices it required.

A Fighter could declare its grants and ask for its own skills, fighting style, and equipment choices. The server no longer had to interpret every possible shape from an external API. It could ask the rulebook for requirements, submit selections, and receive validation.

This initially looked less generic. It was more extensible because meaning had moved to the package that owned it.

Working rulebook-specific behavior was easier to split apart later than generic abstractions were to combine into a real game.

## The mechanics middle layer

An early answer was a generic `mechanics/` layer: conditions, effects, features, proficiency, resources, and spells.

The hope was that shared RPG behavior could be solved once beneath every rulebook. Some primitives earned a shared home. Other modules became abstractions built before a second rulebook proved the common shape. The current D&D behavior increasingly lives in `rulebooks/dnd5e`, while several old `mechanics/*` surfaces became internal, stale, or unused by the host.

The durable lesson is not "mechanics was wrong." It is:

> Start with the rulebook that knows the meaning. Extract a shared mechanism after another real use proves the seam.

Things inside a working rulebook are easier to break out than speculative generic pieces are to assemble into one.

## The strings-to-types pendulum

The early generic designs relied heavily on strings, maps, `any`, and runtime assertions. They were flexible but fragile.

The reaction was to type everything:

- domain-specific string types;
- constants for every identity;
- generic interfaces;
- many small packages;
- compile-time contracts at every seam.

Some of that was load-bearing. Some was an overcorrection.

The eventual compromise was not a return to magic strings. It was to use structured refs and typed contracts where data crosses a boundary, while allowing rulebook-owned behavior to stay concrete.

```text
dnd5e:classes:fighter
dnd5e:features:rage
dnd5e:conditions:raging
```

A host can carry a ref without understanding the implementation behind it.

## Actions were another seam that moved

`Action` changed meaning repeatedly:

```text
executable behavior everywhere
    ↓
internal rulebook implementation pattern
    ↓
first-class activatable object with subscriptions
    ↓
inert data describing something an actor may attempt
```

The important point for Layer 1 is not every version of the interface. It is that the architecture kept moving behavior toward the package that owned the rule and moving transportable state toward plain data.

The clean current vocabulary was discovered through several reasonable models being replaced.

## The ToData breakthrough

The hard question was not merely "how do we save JSON?"

It was:

> How does a rules engine save an object whose important part is behavior?

Functions cannot be stored in Mongo or Redis. Teaching the server how to reconstruct every class, feature, and condition would spread the rulebook into the host.

The pattern that simplified the system was:

```text
saved Data
    ↓ LoadFromData
live rulebook object with behavior
    ↓ ToData
saved Data
```

For a character or encounter, the data shape is typed. For polymorphic features and conditions, the host stores opaque JSON containing a structured ref. The rulebook peeks at the ref and routes the remaining data to the implementation that owns it.

The server can store Rage without knowing what Rage does.

`ToData` also composes. An aggregate asks its children for data, and loading walks the same tree in reverse:

```text
Encounter.ToData
  Character.ToData
    Feature.ToJSON
    Condition.ToJSON
    Resources
    Equipment
    Choices
```

There is no central serializer that understands every rule.

## Repositories left, then returned at the door

Repository injection did not move directly from Core to session.

Without an SDK layer that owned a complete operation, repositories inside toolkit modules created more lifecycle wiring than they removed. The first simplification was to kick repositories out of the toolkit. `rpg-api` became responsible for loading characters, calling the rule, deciding what changed, and saving the result.

`IsDirty()` emerged in that period for a good reason. As character data became more nuanced, the host needed a reliable signal that a live rulebook object had changed persisted state.

The signal helped, but the orchestration grew complicated. `rpg-api` had to know when to load, which bus to use, when a condition might have changed state, which participant was dirty, and what to save. The host sat a little too far from the action. The hydration history eventually produced double subscriptions and scattered cleanup/write-back paths because multiple host call sites reconstructed the same behavior.

The session SDK supplied the missing layer. It brought the lifecycle close to the verbs:

```text
incoming request data
        ↓
session verb
  load → act → save → return
        ↓
host repository implementations
```

`rulebooks/dnd5e/session` now declares the repository interfaces it calls. `rpg-api` implements them and injects them when constructing `session.Manager`. The SDK owns when to use the repositories; the host still owns Redis and every other database decision.

Both movements removed complexity:

1. Removing repositories from toolkit internals kept storage technology out of rules and mechanics.
2. Returning repository contracts at the SDK door removed load/save choreography from `rpg-api`.

`IsDirty()` survives inside the rulebook/resolution path as an honest change signal. What disappeared from the host was responsibility for arranging the entire lifecycle around it.

## The current shape this eventually produced

```text
HOST
  owns databases, transport, accounts, and sessions
  implements and injects repository interfaces
  stores typed data and opaque rulebook JSON
  sends IDs and refs

SESSION SDK DOOR
  declares the key-value access patterns it calls
  accepts host implementations once at construction
  loads, acts, saves, and drops runtime objects per verb

RULEBOOK
  owns classes, choices, features, conditions, and game rules
  reconstructs behavior from data

CORE
  owns only the smallest vocabulary honestly shared across games
  knows no D&D and owns no persistence
```

The repository-injection idea from the first week survived after its scope narrowed. It was not the foundation beneath every toolkit module. It became the outer session door: the SDK declares the access patterns it needs, and the host implements and injects them.

The toolkit became simpler when it stopped owning storage technology, made every rulebook object responsible for crossing its own data/behavior boundary, and kept repository ports only where the host enters.

## The question that should end Layer 1

At the end of this page, the reader should understand:

- why pure external data was not enough;
- why rulebooks own meaning;
- why the host stores data but never behavior;
- how `ToData` and `LoadFromData` reconstitute live rules;
- why a condition can be saved without the server understanding it.

Then the unresolved question is earned:

> The rulebook can load a condition's behavior back into memory. Combat still cannot switch on every possible condition. How does that behavior participate without the rest of the engine learning its name?

Layer 2 answers with the event bus and the Chain pattern.

## Visual correction for the storyboard

The character-creation panel must show a read path, not a save path:

```text
Mongo D&D reference database
  classes · races · equipment · polymorphic choice[]
                    ↓ read
                Discord bot
                    ↓ must interpret every shape
           character-creation flow
```

The repository-injection panel is a later and separate architectural response:

```text
module declares an access pattern
            ↓
injected repository interface
            ↓
Redis | Mongo | memory | files
```

Its promise was datastore independence: the backing technology did not matter as long as it supported the access pattern the module actually needed. That promise survives in the current `rulebooks/dnd5e/session` package. Session declares `SessionRepository`, `EncounterRepository`, and `CharacterRepository`; `rpg-api` implements them and injects them into `session.NewManager`. The original idea was not removed. It moved to the door.

## Possible title and opening

### Title

**How Do You Save a Rule?**

### Opening candidate

> The first thing I designed for the rules engine was its storage. That is funny now, because `rpg-toolkit` does not persist anything.
>
> I thought repositories would be injected into the rulebook. I had diagrams for generic CRUD, Redis adapters, Mongo adapters, and rooms that saved themselves through repository interfaces. I did not yet have a `core` package, a working rulebook, or a clear answer for what owned the truth.
>
> The architecture began as boxes with names. The seams came later.
