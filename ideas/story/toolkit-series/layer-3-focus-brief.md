# Focus brief: Layer 3, Rage Is Not Raging

**Status:** Published 2026-09-04 at https://kirkdiggler.github.io/rpg-toolkit/rage-is-not-raging/. This remains the compact design and accuracy record.

## Deliverable

Create a self-contained, dark-mode visual field note titled:

# Rage Is Not Raging

Published predecessors:

- https://kirkdiggler.github.io/rpg-project/
- https://kirkdiggler.github.io/rpg-toolkit/
- https://kirkdiggler.github.io/rpg-toolkit/the-nervous-system/

The complete local draft was reviewed before publication and remains preserved beside the earlier drafts.

## One transformation

Early Rage tried to own everything that happened after activation. The compiler forced the architecture to separate the capability from the active state.

```text
Rage
  can activate
  spends a use

Raging
  is the temporary state
  changes calculations
  watches its ending

Character
  owns active conditions

dnd5e/events
  owns the handoff vocabulary
```

Core law:

> Put each responsibility with the thing that can do it well and own the result.

Supporting law:

> Features activate. Conditions persist.

The separation makes the pieces composable.

## Handoff and chronology

Layer 2 ended on Kirk's confidence in `.On(bus)`:

> When I saw it, I knew we were going to be okay.

Layer 3 briefly rewinds one day. The overloaded Rage implementation landed before the final `.On(bus)` API and shows why the smaller connection felt like relief.

| Date | Evidence | Meaning here |
|---|---|---|
| 2025-08-16 | `b553c735` | `Action[T]` supplies `CanActivate` and `Activate`; Rage is its first example. |
| 2025-08-17 | `c578c911` | A roughly 170-line Rage Feature owns activation, subscriptions, modifiers, resistance, and ending. |
| 2025-08-18/19 | `65516107` | Typed and chained topics gain `.On(bus)`. Chains already exist before the later compiler cycle. |
| 2025-08-20 | `a6a87e09` | Rage and `RagingCondition` coexist, but their package boundary remains coupled. |
| 2025-09-28 | Journey 044 | Direct dependencies produce a circular import. Go refuses the architecture. |
| 2025-11-16/17 | `55f5e0a4`, `6f428d47` | `dnd5e/events` becomes neutral ground; Character receives, applies, and stores Raging. |
| 2025-11-17 | `8c1c0606`, `0228d23a` | Raging contributes through `DamageChain`; the complete Rage attack is tested. |
| 2025-11-19 | `0fd81d75` | `DamageBreakdown` makes each contribution observable. |

Narrative-order safeguard: Panel 4 reveals `Action[T]` after the compiler wall because its small contract answers the responsibility question. State that it was already present; do not imply it was invented later.

## Approved six panels

### 1. One object doing everything

**Beat:** Rewind one day from `.On(bus)`. The first Rage Feature works hard because it owns too much.

**Visual:** One rust Rage node connected to:

```text
uses · activation · attack subscription · damage bonus
resistance · activity tracking · unsubscribe · ending
```

**Transition:** One D&D word is hiding two kinds of behavior.

### 2. Rage is not Raging

**Beat:** Rage is the capability to enter the state. Raging is what remains true afterward.

**Visual:** Split the overloaded node in D20 blue:

```text
Rage Feature                 Raging Condition
can activate                 is active
spends a use                 changes calculations
starts the handoff           watches its ending
```

**Line:**

> Features activate. Conditions persist.

### 3. The compiler agrees

**Beat:** Separating the concepts is not enough. Their package dependencies still point both ways, and Go refuses the cycle.

**Visual:** Compact rust loop:

```text
features → conditions → D&D event definitions → conditions
```

Put the compiler error in the center. Do not show a wall of source code.

**Meaning:** Kirk would not continue until the architecture stopped declaring contradictory ownership.

### 4. Give the job to the thing that can own it

**Beat:** The small activation contract was already waiting.

**Visual:** Responsibility table:

```text
Rage             CanActivate + Activate
Raging           temporary behavior and ending
Character        active-condition collection
dnd5e/events     shared handoff contract
```

**Line:**

> Put each responsibility with the thing that can do it well and own the result.

### 5. Give the handoff neutral ground

**Beat:** The final solution extracts foundational `dnd5e/events`. Features, conditions, and Character can depend on it without it importing their implementations.

**Visual A:** Replace Panel 3's rust loop with a blue foundation:

```text
              dnd5e/events
       topics · ConditionBehavior
                    ▲
       ┌────────────┼────────────┐
   features     conditions    character
```

**Visual B:** Show the actual handoff:

```text
Rage creates RagingCondition
        ↓ ConditionAppliedEvent
          Condition: ConditionBehavior
Character receives it
        ↓ Apply + store
Raging owns what happens next
```

The event bus is the route, not the owner.

### 6. Why is this 13 damage?

**Beat:** One complete attack makes autonomous behavior observable.

**Visual:** Keep combat scaffolding small:

```text
Barbarian → Monster → DamageChain
```

Let the exact archived breakdown dominate:

```text
1d12 weapon damage   8
Strength modifier   +3
Rage bonus          +2
                    ──
total damage        13
```

Color `Rage +2` D20 blue.

**Closing meaning:**

> Composable rules still need to explain themselves.

Do not force a next-chapter hook until its chronology is researched.

## Page discipline

This chapter must be visibly smaller than Layer 2:

- exactly six numbered panels;
- one main diagram per panel;
- no more than two short paragraphs before each diagram;
- one law or personal line after each diagram;
- no generic subsystem tours;
- no repeated explanation of refs, Bless, the event bus, or the Rage lock;
- let the final breakdown carry the emotional weight.

## Visual system

Continue the D20 Architectural Field Notes style:

- `#11141a` background;
- `#f1f3f5` primary text;
- `#c2c8d0` secondary text;
- `#9da7b4` minimum small-text contrast;
- `#7189d7` for the new seam;
- `#c96d62` for over-owned responsibility and the compiler cycle;
- Source Serif 4 prose;
- IBM Plex Sans Condensed headings;
- IBM Plex Mono only for code, dates, compiler evidence, and breakdowns;
- open ruled sections;
- readable mobile diagrams with swipe cues;
- shared D20 favicon and Apple touch icon.

Visual movement:

```text
one overloaded rust node
        ↓
two focused blue nodes
        ↓
one-way blue foundation
        ↓
one explainable number
```

## Leave outside the cut

Keep these only in `layer-3-rage-to-raging-notes.md`:

- September's temporary `Data any` event;
- the 48-day repository pause;
- broad Monster and combat architecture;
- Unarmored Defense and `ACChain`;
- chain interruption;
- persisted pause and resolution machines;
- the January through April 2026 gap;
- the later encounter rewrite;
- the later request-shaped GameCtx mutation pattern.

## Accuracy checks

- `Action[T]` predates the overloaded Rage implementation and compiler cycle.
- The overloaded Rage implementation predates final `.On(bus)` by one day; label the rewind.
- Chained topics predate the circular dependency.
- Rage and Raging coexist before their package boundary is clean.
- The lasting handoff uses `dnd5e/events` and `ConditionBehavior`, not `Data any`.
- Rage creates Raging; Character applies and stores it.
- The archived damage total is `13`, not the remembered illustrative `18`.
- Unarmored Defense becomes chain-observable later.
