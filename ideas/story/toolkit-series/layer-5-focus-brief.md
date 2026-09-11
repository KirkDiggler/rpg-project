# Focus brief: Layer 5, A Request Needs an Owner

**Status:** Approved four-panel narrative with a working title. Not final prose. Not published.

## Deliverable

Create a concise, self-contained visual field note titled:

# A Request Needs an Owner

Build as a local `file://` draft. Publish only after Kirk reviews it.

Previous chapter:

- https://kirkdiggler.github.io/rpg-toolkit/when-a-rule-needs-the-room/

## Setting

The rpg-api dungeon prototype was deliberately where the pieces met. Its purpose was to make real interactions work long enough to reveal trustworthy seams. Establish that on the first screen so the final driver pressure reads as the result of successful exploration, not a late defense of complexity.

## One historical transformation

Layer 4 ended with a condition knowing what should change but lacking an owner-controlled write path. Without that boundary, the condition had to know both the result it wanted and how Character represented, validated, and mutated the state correctly.

The request became a boundary contract between **what should happen** and **how the owner applies it**:

```text
rule knows what
  target · requested value · source
        ↓
typed request
  only the facts the owner needs
        ↓
Character owns how
  validate · preserve invariants · mutate
```

The request shape answered:

```text
rule decides something should change
        ↓
publishes one specific event
        ↓
the state owner is subscribed
        ↓
owner modifies itself
```

Supporting examples already existed:

```text
ConditionAppliedEvent → Character applies/stores Condition
HealingReceivedEvent  → Character changes its HP
ActionGrantedEvent     → Character stores temporary Action
```

The new Action model then made things a Character could hold:

```text
Feature    grants
Action     does
Condition  modifies
```

Actions could be permanent or temporarily granted by a Feature. Flurry of Blows became the hardest real test: spend one Ki, create two temporary Flurry Strikes, and ask Character to add both without the Feature mutating Character's collection.

That request model reached the player as current action data. The successful rpg-api dungeon prototype then supplied the concrete driver needed to turn a selected Flurry Strike into a complete interaction.

Core law:

> The rule asks. The owner changes itself.

Core distinction:

> Ownership answers who may change state. Orchestration answers who drives the change to completion.

Closing:

> The request had an owner. The interaction still needed a driver.

Do not compare this architecture with present-day code. Tell only what the system meant at this point in history.

## Historical boundary

| Date | Evidence | Meaning here |
|---|---|---|
| 2025-12-25 | ADR-0026 | API simplicity is a stated goal. Damage still uses direct `ApplyDamage`, not a `DamageRequestedEvent`. |
| 2026-01-02 | ADR-0028, `a4a0e9f7`, `69c11444` | Features grant Actions; Flurry and Off-Hand Strike publish explicit request events. |
| 2026-01-02 | rpg-api `6e1fbfb` | The API marks rulebook logic in its orchestrator as a smell. |
| 2026-01-07 | `7bd794ce`, `fdd27be2` | Character receives granted Actions; the request-oriented attack flow is tested. |
| 2026-01-26 | rpg-api `53186db`, web `a7da7f01` | Current options reach the client as `AvailableActions` and become combat controls. |
| 2026-01-26/27 | four toolkit encounter suites | Barbarian, Monk, Rogue, and Fighter exercise the playable-dungeon direction. Do not claim every level 1–3 feature was complete. |
| 2026-02-09 | rpg-api `1df5555` | A dedicated Flurry execution path shows what the successful prototype must drive. |

Do not include or interpret any project break.

## Approved four panels

### 1. The request answers the previous question

**Beat:** Begin inside the deliberately broad dungeon prototype, then answer Layer 4: how does a rule ask the state owner to change something?

**Visual:** One large owner-directed boundary:

```text
rule knows what
  target · requested value · source
        ↓ typed request
Character owns how
  validate · preserve invariants · mutate
```

Use ConditionApplied, HealingReceived, and ActionGranted as compact supporting examples, not three equal architecture sections.

**Law:**

> The rule asks. The owner changes itself.

### 2. Actions become things a Character can hold

**Beat:** Explain the new first-class Action shape before showing request event internals.

**Visual:** Character's action collection:

```text
Character
  permanent Actions
    Attack
    Move
  temporary Actions
    granted by Features
```

Beside it, show the three historical roles without making them a pipeline:

```text
Features grant
Actions do
Conditions modify
```

Then introduce the typed request envelope an Action can publish when doing something requires a wider interaction.

**Meaning:** A Feature can change what the Character is currently able to do by granting an Action.

### 3. Flurry tests the seam

**Beat:** This is the visual and emotional climax.

**Visual A:**

```text
FlurryOfBlows.Activate
  spend 1 Ki
  create Flurry Strike 1 + 2
        ↓ ActionGrantedEvent ×2
Character.onActionGranted
        ↓
actions: [Flurry Strike 1, Flurry Strike 2]
```

Each temporary Action owns its turn-end cleanup.

**Visual B:** Keep the host/UI payoff in the same panel:

```text
current Flurry capacity
        ↓ AvailableActions
combat panel shows [Flurry Strike]
```

The client is data-driven before the server. rpg-api still constructs much of the availability list at this point.

Use one small evidence strip for the Fighter, Barbarian, Rogue, and Monk playable-dungeon target. It is not a completion claim.

**Line:**

> Feature asks. Character owns the mutation.

### 4. The prototype reveals the next dragon

**Beat:** Selecting Flurry Strike needs a complete interaction, not one local owner mutation. The successful dungeon prototype drives it through an explicit path.

**Visual A:**

```text
player selects Flurry Strike
        ↓ request intent
rpg-api executeFlurryStrike
  load actors
  create bus
  apply conditions
  install context
  resolve strike
  consume capacity
  persist state
  translate result
```

Label the request-to-API connection as a direct API path, not a generic subscriber.

**Visual B:** Widen only enough to show several requests needing their own prototype paths. Add a small evidence label:

```text
historical snapshot
4,443-line encounter orchestrator
54 functions
```

The numbers are evidence of accumulated responsibility, not a verdict against the prototype. It was successfully making the interactions real and teaching where a future seam might belong.

**Question:**

> When has the prototype taught us enough to extract the driver?

**Closing:**

> The request had an owner. The interaction still needed a driver.

Do not show any later extraction or answer.

## Page discipline

- exactly four numbered panels;
- one combined diagram per panel;
- one short lead paragraph before each diagram;
- one closing line after each diagram;
- visible non-footer prose below 650 words;
- Panel 3 dominates visually;
- Panel 4 contains both successful prototype and next horizon;
- metrics remain subordinate;
- no general architecture review;
- no present-day comparison;
- no project break or later solution.

## Visual system

Continue the D20 field-notes design:

- `#11141a` ground;
- `#7189d7` for owner-directed requests;
- `#c96d62` for driver pressure;
- Source Serif 4 prose;
- IBM Plex Sans Condensed headings;
- IBM Plex Mono only for events, dates, and evidence;
- open ruled sections;
- readable mobile labels and swipe cues;
- shared D20 favicon and Apple touch icon.

Visual movement:

```text
one request reaches one owner
        ↓
Actions become things Character can hold
        ↓
one Feature grants two visible strikes
        ↓
the dungeon drives one to completion
        ↓
the future driver seam appears
```

## Accuracy safeguards

- Do not invent `DamageRequestedEvent`.
- Damage uses direct `ApplyDamage` under ADR-0026.
- Condition, healing, and action-grant events have Character-side owners.
- Explicit strike request events exist in January.
- Production has no generic subscriber that completes every strike request.
- rpg-api's dedicated Flurry path lands in February.
- `AvailableActions` is data-shaped for the client, while rpg-api still constructs much of the list.
- Four class suites do not prove every level 1–3 feature was complete.
- The dungeon prototype is introduced in the header as successful, deliberate exploratory work and returns in the final panel.
- Do not include later code or later conclusions.
