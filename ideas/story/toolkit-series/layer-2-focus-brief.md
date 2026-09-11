# Focus brief: Layer 2, The Nervous System

**Status:** Published 2026-09-03 at https://kirkdiggler.github.io/rpg-toolkit/the-nervous-system/. This remains the compact design and chronology record; the reviewed page is the final prose.

## Deliverable

Create a dark-mode, visual-first architecture page titled:

# The Nervous System

This is a detailed GitHub Pages chapter, not a LinkedIn post. A later short LinkedIn introduction will link to it.

The page belongs after the live Layer 1 chapter:

- Why/story: https://kirkdiggler.github.io/rpg-project/
- Layer 1: https://kirkdiggler.github.io/rpg-toolkit/

Layer 1 ends with the `ToData` / `LoadFromData` breakthrough and this hook:

> Well, Rage is part of that data, right?

Layer 2 must explain how polymorphic behavior is restored and how independent game rules participate without combat knowing their names.

## Personal pattern that should organize the page

> I do not trust an abstraction because it handles the example that inspired it. I trust it more after giving it the next real thing I think might break it, and sometimes the next next thing. I keep going until the lines connecting them no longer fit in my head.

The threshold is not line count. It is when the relationship graph exceeds working memory. The architecture must then make connections explicit, local, ordered, or representable as data.

Recurring sequence:

```text
smallest useful seam
        ↓
hardest real example available
        ↓
next real example after that
        ↓
hidden assumptions become visible
        ↓
seam survives or moves
```

Series examples:

- Configuration-driven choices → Monk.
- Polymorphic loader → external Artificer module.
- Conditions → Bless.
- Self-managing condition → Rage ending itself.
- Resolution → pause for a human in another process.

Visual motif: D20 blue marks a new seam; restrained rust marks the real case intended to break it.

## Verified chronology

### 2025-07-02: the first bus

The initial bus already supported:

- string event types;
- `Subscribe` and `SubscribeFunc`;
- numeric handler priorities;
- synchronous `Publish`;
- source, target, and timestamp on each event;
- a generic context map;
- modifiers carrying source, type, value, and priority;
- copying the subscriber list before executing handlers, so the registry lock was not held over user code.

Conceptual shape:

```text
Event
  type: string
  source: Entity
  target: Entity
  timestamp
  Context
    map[string]any
    Modifier[]
```

The first bus was more capable than memory suggested. It was also becoming a dynamic language of strings, maps, runtime assertions, and priority numbers.

### 2025-07-03 to 2025-07-05: conditions and relationships

The bus gained cancellation and richer typed getters over its generic context.

Conditions became entities with:

- Apply/Remove lifecycle;
- their own subscription tracking;
- persistence intent;
- functions attached to events.

A D&D-specific `ConcentrationManager` was generalized into `RelationshipManager` with relationship kinds:

- concentration;
- aura;
- channeled;
- maintained;
- linked;
- dependent.

This was an attempt to make relationships between a source and several conditions explicit.

### 2025-07-29: the external-module question

Typed constants made official content safe but appeared closed to new content. The Artificer exploration asked whether a package could bring a new class, spells, feats, items, and behavior.

Kirk's example address:

```text
homebrew:classes:artificer
```

The external Go package still has to be imported or registered. A string cannot load code that was never linked. Once registered, its ref can route data to the package that owns the loader and return a value satisfying the shared interface.

### 2025-08-11: `core.Ref` and `ToJSON` converge

At 15:13, `core.Ref` moved into Core with three parts:

```text
module : type : value
```

Its original source comment explicitly mentioned external modules and Artificer.

Later that night, complete `ToJSON` and loader examples appeared. The pattern hardened over the following days:

```text
opaque JSON
      ↓ peek ref
choose owning module
      ↓
module chooses loader
      ↓
loader owns JSON shape
      ↓
concrete object satisfying shared interface
```

Law:

> The JSON carries an address, not an implementation.

`ToData` restores a known aggregate shape such as Character. `ToJSON` plus a ref lets polymorphic features and conditions name the package that knows how to restore them.

### 2025-08-12 to 2025-08-18: bus redesigns

The August 12 ref-based rewrite deleted thousands of lines but introduced reflection, pointer-based ref matching, and a regression: it held a read lock while invoking handlers.

Rage later tried to unsubscribe while handling its own end event. Unsubscribe needed the write lock, producing the self-removal deadlock.

The first repair was `DeferredAction`: handlers returned unsubscribe/publish intent, and the bus performed it after dispatch. Depth protection also guarded recursive event cascades.

The August 18 typed-topic design simplified again:

```go
attacks := AttackTopic.On(bus)
attacks.Subscribe(ctx, handler)
```

It removed the Event interface, visible topic strings, reflection from the public path, deferred actions, and the extra context machinery. Static typed topics met dynamic runtime-loaded effects.

## Bless: visual and emotional center

The first condition architecture immediately invited the hardest real example Kirk could think of: Bless.

Bless needs:

- one caster;
- several grantees;
- a Blessed condition on each target;
- a fresh d4 on every qualifying attack or save;
- one concentration relationship connecting the caster to all granted conditions;
- damage to the caster causing a Constitution saving throw;
- a failed save breaking concentration;
- breaking concentration removing every Blessed condition created by that cast.

### What the archived demo actually proved

The demo:

1. created Blessed conditions for Fighter and Rogue;
2. applied each condition;
3. each condition subscribed a function to `before_attack`;
4. the function filtered by attacker, rolled a fresh d4, and appended a modifier;
5. `RelationshipManager` grouped both conditions under the Cleric's concentration;
6. `BreakAllRelationships(cleric)` removed both conditions and their subscriptions;
7. later attacks received no d4.

That was real and exciting decoupling.

### What the demo did not prove

The concentration failure was scripted:

```text
print "cleric takes damage and fails concentration save"
subtract HP
BreakAllRelationships(cleric)
```

The saving throw and player decision were not executed by the architecture.

A later concentration implementation listened for damage and published a `concentration.check` event, but no subscriber completed the save. The bus could synchronously invoke another handler. It could not suspend the interaction, leave the process, wait for a player, and resume later.

Honest line:

> The test passed, and for a while that felt like the same thing as the architecture working.

Optional voice:

> It gave us false hope. But rebellions are built on hope.

## Events and chains

Events answer:

> Who wants to react to this moment?

They do not answer:

> In what deterministic rules order should all those reactions change one calculation?

The mutable-context and priority era created:

- registration order becoming rules order;
- magic priority numbers;
- base values and bonuses losing provenance;
- effects mutating shared event state unpredictably.

Turn-based play provides a natural boundary: gather all contributions first, then execute once.

The Chain pattern kept discovery through the bus while moving calculation into a rulebook-ordered structure:

```text
base → features → conditions → equipment → final
```

Subscribers contribute named transformation functions to stages. The publisher executes the assembled chain. The result can explain what changed it and why.

Strong distinction:

> Events solved who gets to react. Chains solved how all those reactions become one deterministic result.

## Hard limit to preserve

Every historical bus version remained synchronous and process-local.

Nested publication is still one call stack. Deferred operations happen after a dispatch in the same process. Chained topics gather live Go closures from current subscriptions.

None can represent:

```text
pause now
persist unfinished interaction
return through RPC
wait minutes or hours
receive player input in another process
rehydrate
resume from the exact step
```

That capability arrived much later with resolution machines, where a suspended interaction is data rather than a parked call stack.

Closing line:

> The event bus could ask every rule in memory. It could not wait for a person who was not there yet.

Do not explain resolution machines deeply here. Save them for the composable encounter/resolution chapter.

## Recommended visual page sequence

1. **Rage is part of the data**
   - `ToData` versus polymorphic `ToJSON`.
   - Ref as loader address.

2. **The loader existed. Naturally, Artificer**
   - External package registers/imports behavior.
   - Ref routes to loader; loader returns shared interface.

3. **What conditions were supposed to buy**
   - Behavior belongs to the condition.
   - Combat does not switch on named conditions.

4. **The surprisingly capable first bus**
   - Event, context, modifiers, priority, cancellation.
   - Strings/maps/runtime typing shown honestly.

5. **Conditions existed. Naturally, Bless**
   - Fresh d4 functions on several grantees.
   - One caster and concentration relationship.

6. **The relationship graph leaves working memory**
   - Caster → concentration → N Blessed targets → attacks/saves/removal.
   - The demo's manual failed-save shortcut is shown explicitly.

7. **Rage finds the lock regression**
   - August rewrite holds read lock over handler.
   - Rage self-removal requests write lock.
   - Deferred intent, then eventual subscriber snapshot.

8. **`.On(bus)` and typed topics**
   - Static topics; dynamic loaded effects.
   - Explicit connections without visible strings.

9. **Chains replace priority arithmetic**
   - Discover contributors, place named functions into stages, execute once.

10. **The process boundary**
    - Synchronous bus versus persisted suspension.
    - Close on the absent human.

Ten panels may be too many. During storyboard design, compress to 7–8 without losing Bless or the process-boundary ending.

## Visual system

Use the approved D20 Architectural Field Notes style:

- background `#11141a`;
- primary text `#f1f3f5`;
- secondary text `#c2c8d0`;
- smallest text no dimmer than `#9da7b4`;
- D20 blue `#7189d7` for the new seam;
- rust `#c96d62` for the real case/pressure;
- Source Serif 4 prose;
- IBM Plex Sans Condensed headings;
- IBM Plex Mono only for code, refs, dates, and evidence;
- open ruled sections, no nested rounded cards;
- mobile swipe cues and readable diagram labels;
- shared D20 favicon remains a TODO.

## Publication boundary

- Published only after Kirk reviewed the complete local draft.
- The current toolkit root remains Layer 1 until a navigation/overview page becomes worthwhile.
- `/living-world/` remains accessible only by direct URL and is not linked.
- Layer 1 and Layer 2 now link to each other; Layer 2 also links back to `Where Everything Meets`.
- The shared D20 SVG favicon and Apple touch icon are live on the toolkit and origin-story sites.
