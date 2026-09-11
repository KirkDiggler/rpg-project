# Focus brief: Layer 4, When a Rule Needs the Room

**Status:** Published 2026-09-08 at https://kirkdiggler.github.io/rpg-toolkit/when-a-rule-needs-the-room/. This remains the compact design and accuracy record.

## Deliverable

Create a self-contained, dark-mode visual field note titled:

# When a Rule Needs the Room

The complete local `file://` draft was reviewed before publication and remains preserved beside the earlier drafts.

Published predecessors:

- https://kirkdiggler.github.io/rpg-project/
- https://kirkdiggler.github.io/rpg-toolkit/
- https://kirkdiggler.github.io/rpg-toolkit/the-nervous-system/
- https://kirkdiggler.github.io/rpg-toolkit/rage-is-not-raging/

## One transformation

Rage proved a condition could contribute using its own state and the event in front of it. The next rules needed facts they did not own.

> Conditions work great until they need another player or the room.

```text
Dueling
  what is in my hands?

Sneak Attack
  who is beside my target?
  are they my ally?

Protection
  is my ally nearby?
  do I have a shield?
  is my reaction available?
```

Putting every answer on every event would turn events into descriptions of the whole world. The breakthrough was recognizing that these were request-scoped runtime questions, exactly the kind of values Go's `context.Context` carries through one call tree.

Core law:

> Events describe what happened. Context answers what is true right now.

The chapter celebrates that read model. It ends when a condition can decide something should change but has no owner-controlled path for writing that decision back.

## Verified chronology

| Date | Evidence | Meaning here |
|---|---|---|
| 2025-11-26 | Journey 046 | Rage validates the feature workflow; goal becomes a new feature in roughly 30 minutes. |
| 2025-11-27 | `d8b195a3` | Unarmored Defense exposes the need for armor-class calculation. |
| 2025-11-30 | `24859b13` | Second Wind creates a visible healing flow through `HealingReceivedTopic`. |
| 2025-12-04 | `f03cf0f3`, `6e7468bc` | `gamectx` appears because Dueling needs equipped-weapon state absent from `DamageChain`. |
| 2025-12-05 | `0f3daeba` | `HealChain` follows Second Wind's initial event-based healing flow. |
| 2025-12-06 | `1d26b0c5` | `ACChain` follows the Unarmored Defense need. |
| 2025-12-15 | `a428813e`, `63b323d0` | Room and Teams context let Sneak Attack ask about nearby allies. |
| 2025-12-16 | `9369d362` | Protection combines room, shield, and reaction information. |
| 2025-12-25 | `a0fecce5`, `857773bc` | `ApplyDamage` mutates a live Combatant available through runtime context. |
| 2026-01-03 | `7c6af9d5` | `IsDirty` and `MarkClean` formalize the intended mutation/persistence contract. |
| 2026-01-04 | contemporary API record | Automatic dirty-object persistence is still intended, not fully wired. |

## Approved six panels

### 1. One real rule asks for the next seam

**Beat:** The Rage proof becomes a repeatable way to discover shared infrastructure.

**Visual:** Three compact rows:

```text
Rage                 damage composition
Second Wind           healing flow → HealChain follows
Unarmored Defense     armor-class need → ACChain follows
```

Do not imply Second Wind initially used `HealChain`; it published `HealingReceivedEvent`. Do not imply Unarmored Defense was already wired into `ACChain`.

**Transition:** Dueling does not need another calculation chain. It needs information outside the event.

### 2. Conditions work until they need the room

**Beat:** Dueling first asks about both hands. Sneak Attack and Protection widen the radius to another player and the room.

**Visual:** Expand outward:

```text
my hands
   ↓
my target
   ↓
nearby creatures
   ↓
teams + positions + reaction state
```

Use the real questions:

- Dueling: one one-handed melee weapon, no second weapon; shield allowed.
- Sneak Attack: advantage or an ally within five feet of the target.
- Protection: nearby ally, equipped shield, available reaction.

### 3. The event is not the world

**Beat:** The tempting answer is to add every fact to `DamageChainEvent`.

**Visual:** Begin with a small event and let rust fields accumulate:

```text
attacker ID · damage components
+ weapons
+ armor
+ nearby allies
+ positions
+ teams
+ level
+ action economy
```

**Line:**

> Every new condition cannot make every publisher describe more of the world.

### 4. Four wrong homes for the same knowledge

**Beat:** Show the real exploration without turning it into four essays.

**Visual:** Four rejected routes around one center question, **Where does runtime knowledge live?**

```text
on every event             events become kitchen sinks
resolver functions         who provides the game server?
inside every condition     copied state becomes stale
inside the event bus       routing and storage become one job
```

Rust marks each failure; keep the language readable rather than code-heavy.

### 5. This is runtime context

**Beat:** The realization is simple: runtime state for one publish is request-scoped context.

**Visual A:** Layer ownership:

```text
rpg-api
  loads real state
  provides registries
        ↓ context.Context
D&D rulebook
  defines the questions
  conditions query what they need
        ↓
core/events
  remain generic
```

**Visual B:** Three narrow reads:

```text
Dueling       CharacterRegistry → hands
Sneak Attack  Room + Teams → nearby ally
Protection    Room + CharacterRegistry → position + shield + reaction
```

Each rule asks only the question it owns. Missing required context fails early.

**Law:**

> Events describe what happened. Context answers what is true right now.

### 6. Reading worked. How do we write this back?

**Beat:** A condition can now read enough context to decide that state should change. There is no owner-controlled write path for returning that decision to Character or Monster.

**Visual:** Preserve the attempted path:

```text
API loads live Combatants
        ↓
places them in runtime context
        ↓
toolkit calls ApplyDamage
        ↓
live Character / Monster mutates HP
        ↓
object marks itself dirty
        ↓
host must discover and save it later
```

Visually divide **read access solved** in blue from **direct mutation** in rust.

End the diagram on the missing seam:

```text
condition knows what should change
        ↓
        ?
        ↓
Character owns the state and persistence shape
```

Evidence label:

```text
ApplyDamage and dirty tracking shipped.
The generic dirty-object save sweep was still intended,
not the API's completed implementation.
```

Closing question:

> A condition could know what should change. How could it ask the Character that owned the state to change it?

This may hint that the eventual answer is a request the owner subscribes to, but it must not explain or diagram request-shaped events yet.

## Page discipline

- exactly six numbered panels;
- one main diagram per panel;
- no more than two short paragraphs before a diagram;
- one law or personal line after a diagram;
- gamectx is the visual and emotional center;
- Panel 6 is a warning, not equal weight with the breakthrough;
- no broad feature, combat, or UI inventory;
- no next architecture explained before its time.

## Visual system

Continue the D20 field-notes design:

- `#11141a` background;
- `#f1f3f5` primary text;
- `#c2c8d0` secondary text;
- `#9da7b4` minimum small-text contrast;
- `#7189d7` for new seams and reads;
- `#c96d62` for kitchen-sink pressure and mutation;
- Source Serif 4 prose;
- IBM Plex Sans Condensed headings;
- IBM Plex Mono only for code, dates, and evidence;
- open ruled sections;
- readable labels and swipe cues on mobile;
- shared D20 favicon and Apple touch icon.

Visual movement:

```text
one local condition
        ↓
an expanding circle of questions
        ↓
a bloated event
        ↓
request-scoped read views
        ↓
one missing path back to the state owner
```

## Leave outside the cut

Keep these only in `layer-4-game-context-notes.md`:

- full class-feature inventory;
- detailed HealChain and ACChain implementation;
- Unarmored Defense's later chain wiring;
- broad tactical UI and Monster automation;
- automatic dirty-save behavior presented as shipped;
- the January through April gap;
- the implementation of request-shaped write events;
- resolution machines;
- later gamectx simplification and retirement.

## Accuracy checks

- Second Wind initially uses `HealingReceivedTopic`; `HealChain` follows five days later.
- Unarmored Defense exposes the AC need; complete chain wiring is later.
- Dueling creates the first `gamectx` pressure on December 4.
- Sneak Attack adds Room and Teams on December 15.
- Protection composes position, shield, and reaction reads on December 16.
- `ApplyDamage` appears December 25.
- dirty tracking appears January 3.
- the January 4 record says the generic API dirty-save sweep is not yet implemented.
- read access is the breakthrough; mutation is the overstep.
