# Layer 5 working notes: requests need owners

**Status:** Wide research ledger for the next architecture field note. Working title: **A Request Needs an Owner**. Six-panel request-to-prototype cut approved for local drafting; not published.

Follow the established process:

1. collect wide notes;
2. verify history against Git;
3. ask Kirk focused memory questions;
4. choose one transformation and endpoint;
5. move later material below the cut;
6. create a compact focus brief;
7. only then build local HTML.

## Series handoff

Layer 4, **When a Rule Needs the Room**, ended with:

> A condition knew what should change. How could it ask the Character that owned the state to change it?

`gamectx` solved request-scoped reads. It did not provide an owner-controlled write path.

Without a request boundary, a condition had to know two different things:

```text
what result it wanted
how Character represented and safely mutated that result
```

The request separated those responsibilities. It became a boundary contract carrying only target, requested value, source, and other facts the owner needed. Character remained responsible for validation, invariants, and mutation.

The next pattern was request-shaped events:

```text
rule decides a change should happen
        ↓
publishes one specific event
        ↓
the object that owns the state is subscribed
        ↓
owner validates and modifies itself
```

This solved direct cross-object mutation inside the rulebook. It also created a new integration burden: someone had to create the bus, load every owner, attach every subscriber, route every request, and persist the results.

That responsibility increasingly landed in rpg-api.

## Kirk's remembered shape

- Conditions could request that a modification happen.
- Character and other state owners subscribed to unique event types and modified themselves.
- The request pattern solved ownership.
- Each request added setup and orchestration complexity.
- Much of that complexity landed in rpg-api.
- That was a smell because implementing rpg-toolkit should make server setup simple.
- The rpg-api encounter area was where this complexity accumulated.
- The later extraction of Encounter began as an attempt to isolate the system and discover its real seams.

Current chapter direction:

> Moving to requests solved who owns a change. The rpg-api dungeon prototype then became the place where a real composition could teach us who should drive those requests.

Possible law:

> A request can give a change an owner without giving the interaction a driver.

Possible API principle:

> The server should eventually load, call, and save. The prototype first had to teach us what the call actually was.

## Kirk's answer: the prototype was a seam laboratory

The rpg-api encounter/dungeon prototype was intentionally built to learn the boundary. Its growing responsibility surface is evidence of successful exploration, not proof that the prototype was a mistake.

By this period the cross-repository prototype could display a tactical grid, move and attack, show combat feedback, run Monster turns, track turn order and action economy, activate features, show conditions, and move through dungeon spaces.

That working composition made architectural questions concrete. A request event with no driver was no longer an abstract API problem; it had to become a playable action in a running host.

The recurring judgment call was:

> When do we know enough to extract the logic the prototype currently owns?

Extract too early and the toolkit predicts a generic seam from one example. Extract too late and every new request teaches the host another rulebook detail. The prototype's job was to accumulate enough real cases that the boundary could be discovered rather than imagined.

Opening decision: establish the rpg-api dungeon prototype on the first screen as the deliberate place where real interactions were composed to reveal seams. The final driver panel should feel like a return to that premise, not the first defense of the prototype.

Potential law:

> You cannot extract a seam before the prototype has taught you where it is.

Potential ending:

> The prototype was not failing. It was becoming specific enough to teach us what to extract.

Do not include or infer a project break in this chapter. Its timing and meaning are outside the current evidence boundary and would distract from the request/prototype story.

## Important terminology distinction

The request pattern did not arrive as one clean subsystem with one naming convention.

Several existing events already behaved like owner-directed requests:

```text
ConditionAppliedEvent
  Feature asks Character to apply and store a condition.

HealingReceivedEvent
  Second Wind asks Character to change its HP.

ActionGrantedEvent
  Feature or granter asks Character to store a temporary action.
```

January added explicitly named request events:

```text
FlurryStrikeRequestedEvent
OffHandStrikeRequestedEvent
```

Do not claim a `DamageRequestedEvent` existed at this point.

ADR-0026 deliberately chose a different damage shape:

```text
DealDamage
  resolves modifiers
  calls target.ApplyDamage directly
  then publishes DamageReceivedEvent as notification
```

Conditions that deal damage were expected to call the shared `DealDamage` flow. `DamageReceivedEvent` was informational after application, not the owner-directed mutation request Kirk now remembers in generalized form.

The broad memory is directionally right: events were being used to decouple decisions from state owners. The exact event names and mutation paths differed.

## Verified chronology

### 2025-12-25: ADR-0026 tries to keep the API simple

Commit `857773bc` implemented the two-phase damage flow and added ADR-0026.

The ADR states the problem directly:

- API tracked HP separately from Character data;
- player and Monster damage persisted differently;
- conditions could not easily deal damage;
- API was loading, calculating, applying, and persisting.

Its intended boundary:

```text
API
  load entities
  install runtime context
  call toolkit
  persist results

Toolkit
  resolve modifiers
  apply damage
  publish notification
```

ADR-0026 was Proposed, not Accepted.

Its positive consequence claimed:

> API stays simple: Load entities, call toolkit function, persist results.

That promise is a useful benchmark for what happened next.

### 2026-01-02: typed Actions and explicit request events

ADR-0028 defined three domain types:

```text
Feature    grants actions or conditions
Action     something a player does
Condition  passive state that modifies
```

Its short law:

> Features grant. Actions do. Conditions modify.

Flurry of Blows granted temporary `FlurryStrike` actions.

Two-Weapon Fighting granted an `OffHandStrike` action.

The actions published explicit requests:

```text
FlurryStrike.Activate
  → FlurryStrikeRequestedEvent

OffHandStrike.Activate
  → OffHandStrikeRequestedEvent
```

The comments and tests said the game server would consume those requests and execute the actual strikes.

`ActionGrantedEvent` had a clearer owner inside the rulebook:

```text
feature/granter publishes ActionGrantedEvent
        ↓
Character.onActionGranted receives it
        ↓
Character stores the temporary Action
```

This is a strong visual example of the request pattern working as intended.

The concrete Flurry of Blows flow is the leading climax candidate:

```text
FlurryOfBlows.Activate
  spends Ki
  creates two temporary FlurryStrike actions
  applies each action to the bus for turn-end cleanup
  publishes ActionGrantedEvent twice
        ↓
Character.onActionGranted
  verifies the event is for this Character
  accepts the Action interface
  appends each strike to []Action
        ↓
two FlurryStrike actions now exist without the Feature mutating Character directly
```

This demonstrates the ownership win. Feature asks; Character owns the mutation.

Using a granted strike reveals the next dragon without solving it:

```text
FlurryStrike.Activate
  consumes strike capacity
  publishes FlurryStrikeRequestedEvent
        ↓
        ?
        ↓
actual attack resolution
```

The chapter can climax on the two actions appearing, then end on the unanswered driver rather than treating the unanswered request as the main failure.

### 2026-01-02: the API smell is named the same day

rpg-api commit `6e1fbfb` is titled:

> Remove game logic from orchestrator, add smell docs

The removed API code inspected weapon slots, shields, and weapon properties to decide whether an off-hand strike should be granted.

The commit states:

> rpg-api stores data. rpg-toolkit handles rules.

And:

> If you find yourself adding game logic in rpg-api, STOP. This is a smell that the toolkit is missing something.

The proposed simpler call was toolkit-owned and Character-aware rather than API-owned rule logic.

This commit is direct evidence of the desired product boundary: a server implementing rpg-toolkit should not need to reproduce D&D rules.

### 2026-01-04: a request exists with nobody listening

The contemporary attack-flow record says:

> Off-hand strike action publishes event nobody handles.

The API ignored the request path and called `ResolveAttack` directly.

This is an important failure shape:

```text
Action publishes a request
        ↓
no production subscriber owns execution
        ↓
API uses a parallel direct path
```

The abstraction worked in toolkit tests while the real host still needed separate orchestration.

Do not overstate this as every request being broken. `ActionGrantedEvent`, `ConditionAppliedEvent`, and `HealingReceivedEvent` had Character-side subscribers. The gap appears when a request needs orchestration beyond one already-loaded state owner.

### 2026-01-07 to 2026-01-10: the action surface expands

The toolkit added:

- action economy;
- Attack, Dash, Dodge, and Disengage abilities;
- Strike and Move actions;
- event-based action granting;
- full attack-flow integration tests;
- saving-throw and movement chains;
- opportunity-attack triggering.

rpg-api added:

- `ActivateCombatAbility` and `ExecuteAction` orchestration;
- handlers for the new verbs;
- toolkit-to-API mapping;
- persistence and action-economy updates.

ADR-0029 said, as a neutral consequence:

> Resolution logic in game server.

That line may be the architectural tension in one sentence. Rule contribution lived in the toolkit, while driving the interaction still lived in the host.

### January 10 snapshot: the host is no longer small

At the January 10 repository state, `internal/orchestrators/encounter/orchestrator.go` was approximately:

```text
4,443 lines
54 functions
15 rpg-toolkit imports
7 LoadFromData call sites
5 NewEventBus call sites
28 repository Save/Update call sites
```

Treat these as historical shape evidence, not a code-quality score or a verdict that the prototype failed. The prototype was successfully gathering real responsibilities in one runnable place. The emerging question was which responsibilities had become stable enough to extract:

```text
which entities to load
which bus to create
which conditions must subscribe
which runtime context to install
which request path to drive
which rulebook result to translate
which objects to persist
```

The toolkit knew the rules. The API prototype was still learning how a complete game made each rule happen.

### 2026-01-26: the request model reaches the combat UI

rpg-api commit `53186db` exposed `AvailableAbilities` and `AvailableActions` in combat responses.

The web commit `a7da7f01` built a two-level, data-driven combat panel from those responses. The UI could render currently available actions, including temporary `Flurry Strike`, rather than relying only on a fixed class-specific button layout.

This is a strong visual climax candidate:

```text
Feature publishes ActionGrantedEvent
        ↓
Character owns the temporary Action
        ↓
API returns AvailableActions
        ↓
web renders what the player can do now
```

Accuracy caveat: rpg-api still constructed much of the available-ability/action list itself at this point. The response was data-shaped for the client, but the toolkit Character was not yet the sole authority for building that menu.

### 2026-01-26 to 2026-01-27: four martial classes get encounter proofs

The toolkit added dedicated encounter suites for:

- Barbarian: Rage activation, duration, resistance, Unarmored Defense, multi-turn combat;
- Monk: Martial Arts, Ki, Flurry of Blows, Patient Defense, Step of the Wind;
- Rogue: Sneak Attack eligibility, once-per-turn state, scaling;
- Fighter: Second Wind and fighting-style behavior including Protection and Two-Weapon Fighting.

These tests support the December milestone goal: a playable dungeon for Fighter, Barbarian, Rogue, and Monk at levels 1-3. Do not claim every level 1-3 feature was complete merely because these four suites existed.

### 2026-02-09: executing Flurry shows the driver cost

rpg-api commit `1df5555` added a dedicated `executeFlurryStrike` path. It loaded Character and Monster, created the bus, installed conditions and runtime lookup, selected an unarmed weapon, resolved the attack, consumed Flurry capacity, persisted state, and rebuilt the response.

The request model had made the action composable. The prototype still needed a large, action-specific driver to make it real.

### March: later solution, outside the likely cut

Toolkit commit `79d770c8` moved StartTurn, EndTurn, AvailableAbilities, AvailableActions, ActivateAbility, and ExecuteAction onto Character.

rpg-api commit `f4c6284` then delegated action economy to Character and deleted its own availability builders and post-strike grant logic.

This is likely evidence for the next chapter about extracting stable responsibilities from the successful prototype. Do not use the March solution to explain the January request-model win.

## Journeys and ADRs around the transition

### Journey documents

- Journey 048 (`2025-12-04`): `gamectx` breakthrough.
- Journey 049 (`2026-05-04`): grep-driven rpg-api/toolkit usage audit.
- Journey 050 (`2026-05-31`): Encounter hydration cascade and duplicate-subscription cure.

The likely request/prototype chapter ends before Journeys 049 and 050. Preserve them as later evidence rather than pulling their conclusions backward.

### ADRs

- ADR-0026: Damage Application via Event Chain, Proposed.
- ADR-0028: Typed Activatable Actions, later superseded by ADR-0045.
- ADR-0029: Chain-Based Combat Modifiers, Proposed.
- ADR-0030: Encounter Owns Combatant Hydration, Accepted on May 31.

ADR-0027 also documented attack resolution and reactions in this period and should be read before deciding whether reactions belong inside the eventual cut.

## The API complexity signal

The complexity was a smell in the precise sense Kirk used at the time: evidence that the toolkit might be missing a stable helper or seam. It was not evidence that a working prototype should never contain orchestration while the boundary remained unknown.

The request pattern introduces two different owner cases.

### Owner already loaded and subscribed

```text
ConditionAppliedEvent → Character applies/stores condition
HealingReceivedEvent  → Character changes HP
ActionGrantedEvent     → Character stores temporary action
```

This is clean because the request and owner meet on the same bus.

### Request needs an interaction driver

```text
OffHandStrikeRequestedEvent
FlurryStrikeRequestedEvent
Move / opportunity attacks
saving throws
reactions
```

These require more than changing one object's local state:

- choose or load targets;
- resolve an attack;
- consume action economy;
- persist multiple participants;
- emit host-facing events;
- possibly wait for input.

The API became the de facto driver because no toolkit layer owned the whole interaction.

Potential core distinction:

> Ownership answers who may change state. Orchestration answers who drives the change to completion.

Request events improved ownership while exposing missing orchestration. The dungeon prototype gave that orchestration somewhere real to run while the seam was still being learned.

## Candidate story spine

Provisional. Keep wide until Kirk chooses the cut.

```text
context solved reading
        ↓
owner-directed events solve local writes
        ↓
Feature grants Action through a request Character hears
        ↓
Action requests a strike
        ↓
the dungeon prototype supplies the real driver
        ↓
API loads, wires, drives, translates, and saves
        ↓
each working case reveals another responsibility
        ↓
the API names rulebook knowledge as a smell
        ↓
when have we learned enough to extract the seam?
```

Possible six-panel version:

1. **A condition asks its owner**
   - `ConditionAppliedEvent`, `HealingReceivedEvent`, and `ActionGrantedEvent` demonstrate local owner-controlled change.

2. **The pattern becomes explicit**
   - Flurry Strike and Off-Hand Strike publish uniquely typed request events.

3. **A request needs someone listening**
   - toolkit tests have subscribers;
   - the production Off-Hand request has none;
   - API calls a parallel direct path.

4. **The driver falls into the API**
   - load, bus, context, resolve, translate, persist.

5. **The request becomes a visible action**
   - API returns `AvailableActions`;
   - the web renders the current action menu;
   - temporary Flurry Strikes become something the player can see.

6. **The next request reveals the dragon**
   - activating a strike still needs a prototype-specific driver;
   - rpg-api loads, wires, resolves, persists, and translates;
   - the request model wins, while the future extraction question becomes visible.

Possible climax:

> The Character could own a temporary action, and the client could render what the player was allowed to do now.

Possible ending:

> We had taught rules how to ask. The prototype was still teaching us what it took to answer.

Alternative ending:

> The request had an owner. The interaction still needed a driver.

## Candidate titles

Do not choose yet.

- **A Request Needs an Owner**
- **The Prototype Has to Learn First**
- **The Server Should Become Boring**
- **Who Drives the Request?**
- **The Toolkit Knew the Rules**
- **When the Plumbing Reaches the Host**
- **The Missing Driver**

Likely title direction depends on the chapter endpoint:

- request pattern as center → **A Request Needs an Owner**;
- prototype learning as center → **The Prototype Has to Learn First**;
- future server simplicity as center → **The Server Should Become Boring**;
- missing orchestration as center → **Who Drives the Request?**

## Visual possibilities

Continue D20 blue for the new seam and rust for the next real case.

### Local request that works

```text
Flurry of Blows
      ↓ ActionGrantedEvent
Character
      ↓ owns []Action
FlurryStrike appears
```

### Request becomes visible

```text
Flurry of Blows
      ↓ ActionGrantedEvent ×2
Character owns []Action
      ↓ AvailableActions
combat UI shows Flurry Strike
```

### Request without a generic driver

```text
FlurryStrikeRequestedEvent
      ↓
rpg-api executeFlurryStrike
      ↓
load · wire · resolve · persist · translate
```

### API accumulation

A central rpg-api column gains one responsibility per request:

```text
load
create bus
hydrate subscriptions
install gamectx
drive request
translate result
save state
publish host event
```

Do not use the raw 4,443-line metric as spectacle. Use it as an evidence label beneath the responsibility graph.

## Material beyond the likely cut

### May 4 audit

Journey 049 re-derives the toolkit/API boundary from actual imports. It finds:

- rpg-api creates fresh buses per interaction;
- Character is the most heavily imported toolkit package;
- gamectx is an important integration shim;
- several documented toolkit modules have zero direct API imports;
- the belief that Features and Conditions are both Actions is only partially true.

This may be the next chapter's opening evidence rather than part of the request-events chapter.

### The rpg-api encounter and isolation work

The large encounter orchestrator lived in rpg-api and was a successful dungeon prototype. Its job was to compose enough real gameplay to make the seams visible.

The later toolkit `encounter` module began as an attempt to isolate behavior and test candidate seams, not as a fully formed final architecture. Preserve that extraction work for the chapter after the request/prototype story.

### Duplicate subscriptions and hydration cascade

Journey 050 records the later concrete failure:

```text
host reloads Character twice on one bus
        ↓
conditions subscribe twice
        ↓
Sneak Attack adds the same modifier ID twice
        ↓
ErrDuplicateID
```

ADR-0030 moves hydration ownership into Encounter:

```text
one load
one subscribe
held runtime entities
ToData cascade back out
```

This is later evidence of the host-setup problem, but likely beyond the next chapter cut.

## Accuracy safeguards

- Do not invent a `DamageRequestedEvent` in January.
- ADR-0026 says `DealDamage` applies damage directly, then notifies.
- `ConditionAppliedEvent`, `HealingReceivedEvent`, and `ActionGrantedEvent` already have Character-side subscribers.
- `FlurryStrikeRequestedEvent` and `OffHandStrikeRequestedEvent` are explicit request events.
- The January 4 record says the production off-hand request has no handler.
- rpg-api itself names game logic in its orchestrator as a smell on January 2.
- The API orchestrator metrics describe historical responsibility shape, not moral failure.
- The rpg-api encounter predates and motivates later toolkit Encounter isolation.
- Journey 049 and ADR-0030 are later; do not pull their solutions backward.
- Do not compare this historical architecture with present-day code.

## Decisions established

- Working title: **A Request Needs an Owner**.
- Use four large story panels rather than six implementation checkpoints.
- Open by answering Layer 4 with owner-directed request events.
- Explain Actions as new things Character can hold before explaining strike-request details.
- Flurry of Blows granting two temporary Actions is the architectural climax.
- The current-action UI payoff stays in the Flurry panel.
- The successful dungeon prototype drives the real strike in the final panel.
- The 4,443-line/54-function snapshot is subordinate evidence, not the story.
- End with the future driver seam visible but unanswered.
- Exclude project-break interpretation and all present-day comparisons.
