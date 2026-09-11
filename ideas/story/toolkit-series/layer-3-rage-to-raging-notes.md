# Layer 3 working notes: Rage to Raging

**Status:** Wide research ledger for **Rage Is Not Raging**, published 2026-09-04 at https://kirkdiggler.github.io/rpg-toolkit/rage-is-not-raging/. Material outside the chapter cut remains here for later field notes.

Keep material beyond the eventual chapter cut here. Once the narrative boundary is approved, create a smaller focus brief and build the visual draft only from that brief.

## Series handoff

Layer 2, **The Nervous System**, ended with events separating participation from deterministic rule order and with Kirk's confidence in the `.On(bus)` shape:

>```text
AttackChain.On(bus)
```

Kirk's memory of that moment:

> That was the thing that let me go to sleep that night. When I saw it, I knew we were going to be okay.

The published chapter then looked farther ahead to the limit of every synchronous bus:

>> The event bus could ask every rule in memory. It could not wait for a person who was not there yet.

Do not jump directly from this ending to resolution machines. Several important transitions happened first.

## The transformation Kirk remembers

> Early I thought Rage the feature could do the work of a condition.

> We had Rage before having Rage set a Raging condition happened after. That transition will be on this post.

The core distinction:

```text
Rage
  permanent capability on the character
  owns availability and limited uses
  can be activated

Raging
  temporary state created by activation
  owns what remains true while Rage lasts
  contributes modifiers
  listens for attacks, damage, turns, and unconsciousness
  decides when it ends
  removes itself
```

Possible compact law:

> Features activate. Conditions persist.

Possible sharper line:

> Rage is the ability to enter the state. Raging is the state.

The early implementation mixed these responsibilities. Commit `c578c911` contains a 170-line Rage Feature that:

- checks and consumes uses;
- stores the active state and owner;
- subscribes directly to attack and incoming-damage events;
- adds outgoing damage;
- applies physical resistance;
- owns its subscription IDs;
- unsubscribes itself;
- publishes Rage-started and Rage-ended events.

The architecture became cleaner only when activation and ongoing modification became different things.

## Kirk's answers: what mattered

### The compiler forced the design correction

The circular dependency was not incidental cleanup. It drove the solution. Go refused to compile the package cycle, making the missing responsibility boundary impossible to ignore.

Kirk would not continue around that warning. The architecture had said it was incorrect; the boundary had to be fixed first. The calendar pause is not the story. The compiler forcing a conceptual separation is.

### Feature became single-purpose

`Action[T]` supplied the clean contract:

```text
anything can be an Action[T]
an action activates
that is all
```

For this story, avoid expanding into a general action-system history. Its value is that it helped make Feature focused: Rage owns the capability to activate and the resources required to do so. It does not own everything that remains true afterward.

### Rage handing off to Raging was the breakthrough

The meaningful transition is not merely two files or two types. It is a composable handoff:

```text
Rage feature activates
        ↓ requests the state change
Raging condition takes over
        ↓ participates independently
other systems observe the result
```

Separating the verb from the active state makes each side composable. Rage can request Raging without owning its lifecycle. Raging can participate in attacks, damage, turns, and removal without owning activation or limited uses.

### Observability was emotionally meaningful

Seeing the condition affect the combat log mattered at the time.

The player-facing debugging question was concrete:

> Why is this 18 damage? Oh right: dice + Strength + Rage.

And the same desire extended to other autonomous rules:

> Did my Unarmored Defense kick in?

The payoff is broader than `+2`. Rules were firing on their own through subscriptions and chains, but their insides were becoming observable. The result could explain which independent contributions produced it.

Potential chapter law:

> Composable rules still need to explain themselves.

Potential final-panel heading from Kirk's remembered experience:

> Why is this 18 damage?

The exact November 17 integration test provides a source-backed version:

```text
1d12 weapon damage   8
Strength modifier   +3
Rage bonus          +2
                    ──
total damage        13
```

The same test records:

- attack roll `15 + 3 STR + 2 proficiency = 20` against AC 13;
- no Rage bonus when Rage is inactive;
- critical hits double the weapon dice but not the Rage modifier.

Use **Why is this 13 damage?** if the final page presents the historical test as evidence. Preserve **Why is this 18 damage?** in these notes as Kirk's remembered player-facing question, not as the literal archived test result.

Unarmored Defense does **not** fit inside the same historical proof:

- `2025-11-27`: `UnarmoredDefenseCondition` is introduced with direct `CalculateAC` behavior;
- `2025-12-06`: `ACChain` infrastructure appears;
- `2025-12-07`: AC breakdown types and `Character.EffectiveAC()` appear;
- `2025-12-17`: a bug fix records that Unarmored Defense was not being applied to Character AC;
- `2026-01-27`: `UnarmoredDefenseCondition` is wired to `ACChain`.

The question **Did my Unarmored Defense kick in?** captures why observability mattered, but it must not be presented as part of the November Rage proof. Save the actual Unarmored Defense chain for a later chapter.

### A later pattern begins here

`OnConditionApplied` will return much later when direct mutation of GameCtx characters becomes request-shaped events. The recurring move is:

```text
do not reach across the boundary and mutate the object
request that something happen
let the owning side decide how
```

Record this as a future callback, not part of the current chapter. It helps show that the thin event was not a one-off workaround, but the story must not explain architecture that did not exist yet.

## Verified chronology

### 2025-08-16: `core.Action[T]` and the Rage identity crisis

Commit `b553c735` added generic `core.Action[T]` with:

```go
type Action[T any] interface {
    Entity
    CanActivate(ctx context.Context, owner Entity, input T) error
    Activate(ctx context.Context, owner Entity, input T) error
}
```

The source comment used Rage as its first no-input example.

The same day's architecture record said actions were an internal rulebook implementation pattern, not something the game server should manipulate directly. The public surface should remain natural:

```text
feature.Activate(...)
spell.Cast(...)
```

Journey 023 records the problem explicitly as an identity crisis:

```text
rage.go            feature that tracks uses
rage_condition.go  condition that modifies things
```

Its conclusion:

> Features activate, effects modify.

The same exploration also found the Rage self-removal deadlock. PR `#220` and its deferred operations were kept. PR `#221` was discarded. Layer 2 already tells the lock story, so Layer 3 should only repeat it if the responsibility split needs that evidence.

### 2025-08-18 to 2025-08-19: typed and chained topics

Commit `65516107` completed typed topics and chained topics with `.On(bus)`.

Both forms already existed:

```go
attacks := combat.AttackTopic.On(bus)
attacks := combat.AttackChain.On(bus)
```

The historical reaction was to the typed topic form:

> you had me at `attacks := combat.AttackTopic.On(bus)` 🔥

The same design and implementation also included `AttackChain.On(bus)`. Chain events therefore predate the later circular dependency. The circular dependency did not invent chains.

### 2025-08-20: Feature and RagingCondition coexist

Commit `a6a87e09` integrated event-driven features and conditions into Character.

At this stage:

- Rage was a Feature with activation and resource responsibility.
- `RagingCondition` represented the active state.
- Rage directly imported and constructed `RagingCondition`.
- The feature then applied the condition and published a condition-applied event.

The conceptual split existed, but the package boundary was still coupled.

### 2025-08-27: bus migration completes

Commit `2990ef3f` completed the broader event-bus migration.

This is infrastructure adoption, not necessarily a central story beat.

### 2025-09-28: the circular dependency exposes the missing boundary

Journey 044 and commit `c7459da4` record the attempted Rage loading flow:

```text
load Rage from JSON
activate it
publish that a condition was applied
let conditions react through the event system
```

The first event design carried the full Condition object:

```go
type ConditionAppliedEvent struct {
    Target    core.Entity
    Condition *conditions.Condition
}
```

That created the cycle:

```text
dnd5e events
    imports conditions to name Condition

conditions
    imports dnd5e to subscribe to those events
```

Three attempted escapes were named:

1. move event definitions into `conditions`;
2. duplicate event types;
3. create a separate events package without yet understanding where it belonged.

The architectural reframe was:

> Events describe what happened, not what exists.

The September answer was thin occurrence data:

```go
type ConditionAppliedEvent struct {
    Target core.Entity
    Type   ConditionType
    Source string
    Data   any
}
```

Rage could publish who received something, what kind of condition was applied, what caused it, and the values needed to interpret the occurrence.

This was a real conceptual correction, but it was an intermediate design rather than the final handoff. It removed the concrete Condition object from the event while leaving the consuming side and concrete construction path incomplete. November would revisit the same compiler wall and finish the package layering without giving up strong typing.

### The 48-day repository pause

After the September 28 merges, all game repositories show no commits until toolkit work resumes on November 16.

Kirk's remembered interpretation:

- the circular dependency was a sign that the architecture was not correct;
- solving it required dedicated thought;
- AI capabilities were increasing;
- work and family meant "big brain" was not available;
- this pattern is common with difficult side-project problems.

Current publication decision: do not make this shorter pause a chapter beat unless it becomes narratively necessary. Git proves the pause but not a single cause. The circular dependency itself is enough evidence that the design had reached a wall.

Do not confuse this with the documented January through April 2026 gap.

### 2025-11-16: first concrete combat composition

Commit `55f5e0a4` added:

- a Monster entity;
- attack resolution;
- explicit modifier stages;
- a concrete combat use of the chain infrastructure.

This is where the promising August chain API begins to become visible D&D combat rather than only generic infrastructure.

### 2025-11-17: the final package split and handoff

Commit `6f428d47` returned to the circular dependency while implementing Character condition tracking.

Journey 045 names the deeper problem: the root `dnd5e` package was trying to be both foundational, defining events and shared types, and coordinating, importing its child implementations. A package could not be both.

The final structure extracted neutral ground:

```text
dnd5e/events
  event types
  topics
  ConditionType
  ConditionBehavior interface
        ↑
        ├── conditions
        ├── features
        └── character
```

Unlike the September `Data any` interim, the final event remained strongly typed through the small `ConditionBehavior` interface.

The complete handoff became:

```text
Rage Feature
  consumes a use
  creates RagingCondition
  publishes ConditionAppliedEvent
        ↓
Character.onConditionApplied
  verifies the target
  applies RagingCondition
  stores it with the character
        ↓
RagingCondition
  subscribes to the events and chains it owns
```

This is the concrete form of Kirk's remembered breakthrough: Rage hands off to Raging, and Character owns the active-condition collection.

### 2025-11-17: Raging joins the damage chain

Commit `8c1c0606` wired `RagingCondition` into `DamageChain`.

Commit `f5c6f780` completed a combat vertical slice with Rage damage.

Commit `0228d23a` added full Rage plus attack integration coverage.

The conceptual flow becomes:

```text
character has Rage feature
        ↓ activate and hand off
character becomes Raging
        ↓ active condition participates
attack resolves
        ↓
DamageChain gathers contributions
        ↓
Raging contributes +2
```

### 2025-11-19 to 2025-11-26: the architecture becomes visible

Commit `0fd81d75` added damage breakdown tracking.

The later journey record describes the payoff:

> We see the +2 damage bonus appearing in the combat breakdown.

The accompanying goal was:

> make adding a new feature a ~30-minute task, not a multi-day adventure.

This is a strong candidate for the chapter's visual and emotional endpoint. It turns an architectural responsibility split into something a person can inspect.

## What the circular dependency actually unlocked

It did not introduce chain events. Those were already present.

It clarified the boundary between packages in two passes:

```text
BEFORE
root dnd5e defines events and imports child implementations
features and conditions also import root dnd5e
compiler finds a cycle

SEPTEMBER INTERMEDIATE
ConditionAppliedEvent carries Type + Data any
event describes the occurrence without the Condition object
complete consuming handoff is not yet present

NOVEMBER FINAL
neutral dnd5e/events defines topics and ConditionBehavior
features creates RagingCondition and publishes the interface
Character receives, applies, and stores it
conditions subscribe without importing a coordinating root
```

This made it possible for:

- Rage activation to remain a Feature responsibility;
- ongoing Raging behavior to remain a Condition responsibility;
- Character to own its active-condition collection;
- event definitions and the small shared interface to live on neutral ground;
- all dependency arrows to point one way;
- later combat chains to gather the Raging modifier cleanly.

## Candidate chapter spine

This is provisional. Do not storyboard until the boundary is approved.

```text
.On(bus) gives confidence
        ↓
Rage Feature tries to own every ongoing responsibility
        ↓
Rage is not Raging
        ↓
the attempted split creates a compiler-enforced cycle
        ↓
the smaller Action[T] contract was already waiting
        ↓
each responsibility moves to the thing that can own it
        ↓
neutral dnd5e/events makes the typed handoff possible
        ↓
Raging joins DamageChain
        ↓
8 + 3 + 2 becomes visible as 13
```

Recommended six-panel version:

1. **One object doing everything**
   - The early Rage Feature owns uses, activation, subscriptions, modifiers, activity tracking, and its own ending.

2. **Rage is not Raging**
   - D&D uses one word for a capability and an active state.
   - The architecture needs two focused responsibilities.

3. **The compiler agrees**
   - Feature creates Condition.
   - Condition needs D&D events.
   - The coordinating root is also pretending to be the foundation.
   - Go refuses the cycle instead of letting the boundary remain vague.

4. **Give the job to the thing that can own it**
   - Reveal `Action[T]` as the small contract already present historically, not as something invented after the cycle.
   - Rage owns `CanActivate` and `Activate`.
   - Raging owns the temporary state and ongoing behavior.
   - Character owns applying and storing its active conditions.

5. **Give the handoff neutral ground**
   - `dnd5e/events` owns topics and the small `ConditionBehavior` contract.
   - Rage creates and publishes Raging without turning the foundational package into a coordinator.
   - Character receives, applies, and stores Raging.

6. **Why is this 13 damage?**
   - Show only `Barbarian → DamageChain → Raging +2` as combat scaffolding.
   - The source-backed breakdown proves the autonomous rule is observable.

## Candidate titles

Do not select yet.

- **Rage Is Not Raging**
- **Features Activate. Conditions Persist.**
- **One Word, Two Jobs**
- **When the Architecture Became +2**
- **Events Describe What Happened**
- **The First Visible Modifier**

Selected chapter title: **Rage Is Not Raging**.

Use **Features Activate. Conditions Persist.** as one architectural law.

Deeper responsibility law from Kirk:

> Put each responsibility with the thing that can do it well and own the result.

Applied here:

```text
Rage owns activation
Raging owns the active state
Character owns its condition collection
dnd5e/events owns the handoff vocabulary
```

## Possible visual motifs

Continue the D20 field-notes system:

- D20 blue for the new seam;
- restrained rust for the real case intended to break it.

New visual motif for this chapter:

```text
one Rage node
      ↓ split
Rage Feature     Raging Condition
verb             state
```

The circular dependency can draw a rust loop between the package columns. The extracted `dnd5e/events` foundation cuts under that loop as a straight blue dependency layer. A second blue handoff line can carry the small `ConditionBehavior` interface from Rage to Character.

The final `+2` should be visually small but emotionally large: one sourced line appearing in a damage breakdown after all the architecture above it.

## Material to keep out of this chapter

### The September `Data any` intermediate

The intermediate event cleaned up the package dependency but used the modern spelling of `interface{}`. It is omitted from this chapter to keep the Rage-to-Raging handoff readable.

Preserve it for a possible later story about readable code, explicit interfaces, and taking generics or generalized data too far.

### Chain interruption

Kirk remembers interruption becoming a real blocker later. It was never cleanly solved by the early chain architecture and belongs with resolution, at least one or two posts after this chapter.

Do not use later interruption or suspension machinery to explain the 2025 Feature-to-Condition split.

### Resolution machines

Persisted pause, remote human input, rehydration, and exact-step resume arrive much later. Layer 2 already teased this boundary. This chapter should not pay it off prematurely.

### The January through April 2026 gap

The documented long gap has an honest explanation already used in `Where Everything Meets`:

- training for a 108-kilometer ultra with five kilometers of elevation gain;
- weekly running regularly exceeding 100 kilometers;
- work focused on practical AI-agent deployment and developer adoption;
- family and normal life competing for the same attention.

The game did not lose to one thing. It waited while other demanding, interesting things won the calendar.

Keep this for the chapter that actually reaches January 2026.

### The later encounter reset

The 18,000-line encounter application, clean-room rebuild, `play/clock`, `play/intel`, `play/record`, `play/interrupt`, and the resolution-owned interaction bus belong to the August 2026 composable-encounter story.

They are not evidence for what Kirk knew in autumn 2025.

## Decisions established

- The chapter title is **Rage Is Not Raging**.
- The chapter includes the transition from Rage doing condition work to Rage handing off to Raging.
- The circular dependency is the forcing function. The shorter repository pause is omitted.
- `Action[T]` appears after the compiler wall in the narrative as the focused contract that was already waiting historically. It is not presented as a new invention after the cycle or expanded into a general subsystem history.
- The responsibility rule is: put each job with the thing that can do it well and own the result.
- September's thin `Data any` occurrence event remains in the evidence ledger but is omitted from the visual chapter. November's neutral events subpackage and `ConditionBehavior` interface carry the readable typed solution.
- The chapter ends with the source-backed `13` damage breakdown, not merely with `RagingCondition` joining `DamageChain`.
- Chain interruption and persisted suspension remain later.

## Remaining questions

Ask one at a time before choosing the final cut and visual sequence.

1. The final proof is one large, literal damage breakdown. The source-backed November test totals `13`, not the remembered illustrative `18`.
2. Unarmored Defense falls after the November cut and remains later material.
3. Use only enough combat scaffolding to establish `Barbarian attacks Monster → DamageChain → Raging contributes +2 → 8 + 3 + 2 = 13`. Save the wider combat system for another chapter.

## Process guardrail from Layer 2

The Layer 2 drafting pass became rough because the chapter boundary and causal order were still moving while the visual page was being built.

For Layer 3:

1. collect wide notes first;
2. verify chronology against Git;
3. ask Kirk focused memory questions;
4. choose one transformation and one payoff;
5. move later material below the cut;
6. create a compact focus brief;
7. only then build the self-contained local HTML draft.
