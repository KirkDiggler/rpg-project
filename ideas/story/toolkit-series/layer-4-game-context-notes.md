# Layer 4 working notes: when a rule needs the room

**Status:** Wide research ledger for **When a Rule Needs the Room**, published 2026-09-08 at https://kirkdiggler.github.io/rpg-toolkit/when-a-rule-needs-the-room/. Material outside the chapter cut remains here for later field notes.

Keep later write/request architecture here after the visual chapter cut is chosen. Build a compact focus brief before building HTML.

## Series handoff

Layer 3, **Rage Is Not Raging**, ended with a rule whose hidden contribution could explain itself:

```text
weapon die    8
Strength     +3
Rage         +2
             ──
total        13
```

Raging can contribute because it knows its own state and hears the current damage chain.

The next real rules need information they do not own.

Kirk's line:

> Conditions work great until they need another player or the room.

## The chapter's main story

`gamectx` was a major breakthrough, not supporting plumbing.

The need grew through real D&D rules:

```text
Dueling
  What is in my main hand?
  Is a second weapon in my other hand?
  Is the off-hand item a shield?

Sneak Attack
  Where is my target?
  Who is next to them?
  Is that creature my ally?

Protection
  Is my ally within five feet?
  Am I carrying a shield?
  Is my reaction available?
```

These questions do not belong in every event. They describe runtime state surrounding one publish.

Kirk's remembered breakthrough:

> This is just runtime context for a given publish, which is exactly what `context.Context` in Go is built for.

Core distinction:

```text
Event
  what happened

Context
  what is true around this request right now
```

Possible law:

> Events describe what happened. Context answers what is true right now.

## Verified chronology

### 2025-11-26: one rule becomes a workflow

Journey 046 opens after the Rage proof:

> We've proven the event bus architecture works with the Rage feature.

Its stated goal:

> Make adding a new feature a ~30-minute task, not a multi-day adventure.

The toolkit quickly adds more Barbarian, Fighter, and Monk rules. Do not turn this chapter into a feature inventory. Its importance is that each real rule asks the toolkit for a reusable seam, and then Dueling asks for something different: outside knowledge.

A compact opening progression may be worth showing:

```text
Rage
  proves damage modifiers can compose

Second Wind · 2025-11-30
  introduces a visible healing flow through HealingReceivedTopic
  exposes the need for healing modifiers
  HealChain follows on 2025-12-05

Unarmored Defense · 2025-11-27
  exposes the need for armor-class calculation
  ACChain follows on 2025-12-06
  the condition is not actually wired into ACChain until later
```

Accuracy rule: do not say Second Wind's first implementation used HealChain. It published `HealingReceivedEvent`. Do not say the first Unarmored Defense implementation already participated in ACChain. The rules exposed those reusable calculation seams before their complete integration.

### 2025-12-04: Dueling asks about both hands

Commit `f03cf0f3` added the first `gamectx` package.

Commit `6e7468bc` implemented Dueling with it.

Dueling requires:

```text
one one-handed melee weapon
no second weapon
a shield is allowed
```

The `DamageChain` event had the attacker ID and damage components. It did not have the complete equipment state, and adding equipment to the event would solve one rule by burdening every publisher.

The first `CharacterRegistry` was deliberately read-shaped:

```go
type CharacterRegistry interface {
    GetCharacter(id string) interface{}
}
```

The first implementation was not yet elegant: `interface{}` plus a type assertion to `CharacterWeapons`. Its architectural purpose was more important than its polish.

### 2025-12-05: gamectx moves into the rulebook

The package moved under `rulebooks/dnd5e/gamectx`.

The layer model:

```text
rpg-api
  loads real state
  implements/provides registries
  wraps them into request context

D&D rulebook
  defines the questions conditions may ask
  owns Dueling, Rage, and other rules

core/events
  remain generic infrastructure
```

The host provides data. The rulebook defines how rules query it.

### Four rejected homes for runtime knowledge

Journey 048 records four approaches before `context.Context` clicked.

#### 1. Frankenstein events

Put weapons, armor, allies, levels, and every future fact onto `DamageChainEvent`.

Rejected because every new rule grows the event and events stop describing what happened.

#### 2. Resolver functions on the chain

Give the chain callbacks such as `GetWeapons` and `GetArmor`.

Rejected because the toolkit cannot depend on or conjure the game server that implements them.

#### 3. Every condition caches state

Have Dueling subscribe to equipment changes and maintain `hasOneHandedOnly` itself.

Rejected because copied state can become stale and every simple query creates more subscriptions.

#### 4. Put registries on the bus

Let the event bus become pub/sub plus queryable game-state storage.

Rejected because event routing and runtime state access are different responsibilities.

### The `context.Context` reframe

Go already had the transport for request-scoped values flowing through a call tree.

The pattern became:

```text
host loads state
      ↓
wraps read registries into context.Context
      ↓
publishes/resolves one interaction
      ↓
condition asks the question it owns
      ↓
context disappears with the request
```

Conditions declare requirements and fail early when required context is absent.

Dueling initially checked self data when it was applied. The later rules made the self/world distinction more visible:

```text
self data
  my weapons
  my armor
  my ability scores
  often read at Apply time

world data
  target position
  nearby entities
  teams and alliances
  read when the event occurs
```

### 2025-12-15: Sneak Attack needs the room and teams

Commit `a428813e` added Room and ability-score access to `gamectx`.

Commit `63b323d0` added team relationships and the positional Sneak Attack check.

The rule can now ask:

```text
Did I have advantage?
        or
Is one of my allies within five feet of the target?
```

The condition queries Room for positions and Teams for relationships. The attack event remains focused.

### 2025-12-16: Protection needs several views at once

Commit `9369d362` implemented Protection.

It consults:

- Room for the Fighter and target positions;
- CharacterRegistry for the Fighter's shield;
- action economy for reaction availability;
- the attack event for who is attacking whom.

Protection shows the full value of the breakthrough. A rule can compose several narrow views without asking the attack event to carry the whole world.

Potential visual center:

```text
                one attack event
                       │
          ┌────────────┼────────────┐
          │            │            │
       Dueling    Sneak Attack   Protection
       hands       room+teams    room+shield+reaction
```

Each rule asks only the question it owns.

## The overstep near the end

The read model solved a great deal. The problem began when access to live runtime objects was treated as permission to mutate them.

Kirk remembers:

- `ApplyDamage` mutating a combatant obtained through context;
- the combatant marking itself dirty;
- an expected end-of-interaction save back to the database.

### 2025-12-25: ApplyDamage crosses the line

Commit `a0fecce5` added a `Combatant` interface with `ApplyDamage`.

Commit `857773bc` added the two-phase damage flow:

```text
resolve modifiers
      ↓
target.ApplyDamage(...)
      ↓
publish what happened
```

The method directly mutates Character or Monster HP. Its source comment says the caller is responsible for persisting the updated state.

### 2026-01-03: dirty tracking formalizes the write contract

Commit `7c6af9d5` added:

```go
IsDirty() bool
MarkClean()
```

`ApplyDamage` marks the combatant dirty.

The commit message states the intended pattern directly:

> API loads combatants into gamectx, toolkit mutates them during combat, and API saves dirty ones afterward.

This is the chapter's final warning, not its main architecture.

### 2026-01-04: intended, not fully wired

The contemporary API architecture record says:

> gamectx not fully utilized. ADR-0026 intended for combatants to be loaded into gamectx, mutated during combat, then saved. Currently API manages saving separately.

Its intended pseudocode loops through context combatants, saves those where `IsDirty()` is true, and calls `MarkClean()`.

Accuracy rule: do not claim an automatic dirty-object save sweep was already running in the API. The mutation and dirty contract shipped. The generic post-resolution save loop remained intended architecture.

## Approved ending weight

The chapter is about the `gamectx` unlock. Dirty tracking appears only near the end as the next boundary.

Full final-panel concept:

```text
THE READ BREAKTHROUGH

condition asks
  what is equipped?
  who is nearby?
  which side are they on?
  is a reaction available?

context answers with request-scoped runtime state

──────────────────── boundary ────────────────────

THE OVERSTEP

API loads live Combatants
        ↓
places them in gamectx
        ↓
toolkit calls ApplyDamage
        ↓
Character / Monster mutates HP
        ↓
object marks itself dirty
        ↓
intended: host saves dirty objects afterward
```

Evidence label:

```text
Dirty tracking shipped.
The automatic dirty-object save sweep was still intended,
not the API's completed implementation.
```

The important gap is not merely database plumbing. `gamectx` solved read access but supplied no owner-controlled path for a condition to return a requested state change.

End the panel with:

```text
condition knows what should change
        ↓
        ?
        ↓
Character owns the state and persistence shape
```

Closing question:

> A condition could know what should change. How could it ask the Character that owned the state to change it?

Potential supporting line:

> `gamectx` was the right place to read the world. Mutation was the overstep.

The next chapter can answer with conditions publishing modification requests that the owning Character subscribes to. Hint at that direction, but do not explain or diagram the solution here.

## Candidate chapter spine

Provisional, pending title and exact panel count.

```text
Rage proves a local condition can participate
        ↓
Dueling needs equipment outside the event
        ↓
Sneak Attack and Protection need other people and the room
        ↓
the event threatens to become a kitchen sink
        ↓
context.Context is already the request-scoped answer
        ↓
gamectx lets each condition ask only what it owns
        ↓
read access expands into live mutation
        ↓
how do we save this?
```

Possible six-panel compression:

1. **One real rule asks for the next seam**
   - Rage proves damage composition.
   - Second Wind exposes healing flow.
   - Unarmored Defense exposes armor-class calculation.
   - The pattern is becoming repeatable rather than remaining one successful Rage example.

2. **Conditions work until they need the room**
   - Dueling first asks about both hands.
   - Sneak Attack and Protection later widen the question to other people and positions.

3. **The event is not the world**
   - A Frankenstein event accumulates weapons, armor, positions, teams, and action economy.

4. **Four wrong homes for the same knowledge**
   - event, chain callbacks, cached condition state, and bus registries.

5. **This is runtime context**
   - Go's `context.Context` already carries request-scoped values through one publish.

   - The breakthrough diagram also shows each rule asking its own question: Dueling reads hands; Sneak Attack reads room+teams; Protection composes room+shield+reaction.

6. **Reading worked. How do we write this back?**
   - live Combatant → ApplyDamage → dirty;
   - intended host save loop;
   - no clean path from a condition's decision back to the Character that owns the state;
   - reading was the breakthrough, direct mutation was the overstep.

## Candidate titles

- **When a Rule Needs the Room**
- **The Event Is Not the World**
- **What Is True Right Now?**
- **Conditions Need a World**
- **Context Is Not Permission**

Selected working title: **When a Rule Needs the Room**. It names the real pressure case without making the whole chapter sound like a Go API tutorial.

Reserve **Context Is Not Permission** for the final mutation warning rather than the chapter title.

Potential panel laws:

> Events describe what happened. Context answers what is true right now.

> Each rule asks only the question it owns.

> Access is not ownership.

## Material for later chapters

### Request-shaped writes

Later architecture stops handing mutation authority through context. Rules publish/request a change; the owning sheet or keeper validates and performs it.

This is the answer to the current chapter's closing problem and belongs in the next post.

### `gamectx` simplification and retirement

Much later, conditions read their own sheet and a narrow cast instead of several broad registries. Writes become requests by construction. Preserve that full redesign for its chronological chapter.

### Chain interruption and resolution machines

Still later. Do not pull persisted pause, player input, or request steps into this chapter.

### The January through April gap

The chapter can end before the documented long gap. Its known life/work/running explanation remains later material.

### Broad combat and UI work

The tactical voxel grid, combat log UI, automated Monster turns, action economy, and class implementation sprint are context, not the main story here.

## Accuracy safeguards

- `gamectx` begins with Dueling on December 4.
- The initial CharacterRegistry uses `interface{}` and is refined later; do not present the first API as perfectly typed.
- Sneak Attack gains Room and Teams context on December 15.
- Protection composes Room, CharacterRegistry, shield state, and action economy on December 16.
- `ApplyDamage` appears December 25.
- dirty tracking appears January 3.
- the automatic dirty save sweep is intended but not yet implemented in the API's January 4 architecture record.
- context read access is the breakthrough; mutation is the overstep.
