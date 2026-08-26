# Shared Table Dice Ritual — Concepts Lab Design

**Status:** Approved conversational design; written review pending

**Tracking:** [rpg-project#289](https://github.com/KirkDiggler/rpg-project/issues/289) · concept design slice [#290](https://github.com/KirkDiggler/rpg-project/issues/290)

**Foundation:** [interactive collectible 3D dice tray](../design.md) · production session combat [rpg-project#270](https://github.com/KirkDiggler/rpg-project/issues/270) · [rpg-dnd5e-web#822](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/822)

## Purpose

Find the physical feel of throwing dice before asking production contracts to support it.

This slice stays entirely in rpg-dnd5e-web Concepts Lab with fixture data. It evolves the literal shared dice components already used by production, but it does not modify the production session route, protos, toolkit, API, persistence, or real multiplayer transport.

The concept must let Kirk throw the dice repeatedly, start with deliberately broad motion candidates, and iterate quickly until the held interaction, release, travel, tumble, impact, rerolls, and pacing feel right.

## North star

Rolling dice is a shared D&D ritual, not a delayed number reveal.

The acting player physically throws one attack group and, on a hit, one damage group. A die contributed by another player uses that contributor's set and joins the same handful. Rules-driven rerolls remain visible but automatic. A local roller/witness fixture proves the shared-table experience without claiming production networking exists.

Fixture data supplies every face and attribution. Gesture changes presentation only.

## Scope

The concept includes:

1. exact held attachment;
2. at least three intentionally distinct throw-feel candidates;
3. one- and multi-die roll groups;
4. fixture-backed player dice sets and mixed contributor styles;
5. one explicit attack throw and one explicit damage throw on a hit;
6. automatic visual rerolls and source-labelled modifier toasts;
7. simulated roller/witness release through the existing presentation-event pattern;
8. keyboard, pointer/touch, reduced-motion, semantic fallback, and responsive review states; and
9. hands-on comparison and iteration evidence.

It does not include a proto or backend contract, production combat wiring, real multiplayer release, authoritative set ownership, or production promotion.

## Existing foundation

### Production components

Web PR #822 promoted the production `CombatExperience` and its real `DiceTrayPresentation` d20 ritual. The actor explicitly presses Roll or grabs/releases a carved d20. Story and semantic verdict remain concealed until release. Reconnect/catch-up settles stale presentations rather than replaying them.

The concept reuses and evolves this shared component chain rather than creating a second renderer:

```text
DiceTrayPresentation
  -> DiceTray3D / roll-group tray
    -> RollGroupGestureController
      -> DiceMotionSolver
        -> AttackDie3D / generic die renderer
```

The production route remains on its current one-d20 inputs during this concept slice.

### Permanent authority boundary

Interactive dice Stones 0 and 1 already established:

- authoritative faces are supplied before presentation;
- `VisualThrowProfile@1` changes only choreography;
- raw pointer samples remain local;
- Roller and Spectator own independent renderers and resources;
- unknown assets and renderer failures fall back truthfully; and
- reduced motion retains explicit input and exact semantic settlement.

This concept preserves those rules.

### Current held-motion defect

The pointer controller correctly records normalized screen samples. The solver then maps them to fixed world X/Z offsets beneath a perspective three-quarter camera. Those world axes are not aligned with the visible tray.

Under the production camera, a pointer at 75% tray width currently projects the die near 57% width. Vertical input can project diagonally. Procedural wobble makes a correctly sampled gesture feel even more disconnected.

This is a coordinate-boundary defect, not a shake-tuning defect.

## Confirmed experience decisions

1. **Exact attachment while held.** The touched point stays beneath the pointer/finger. Position has no spring lag. Weight comes from lift, pose, shadow, and rotation.
2. **Gesture shapes presentation.** Release direction and energy may affect travel, tumble, and impacts. Supplied fixture faces alone determine settlement.
3. **Feel begins wide.** The first candidates should be meaningfully different, not minor slider variations. Kirk selects through repeated play and requests iteration.
4. **Attack and damage are separate throws.** The player throws the attack group once. A hit then asks for one damage-group throw. A miss ends promptly.
5. **No extra reroll input.** Great Weapon Fighting shows original 1s/2s, flashes affected dice, automatically tumbles only those dice, and settles to supplied replacements.
6. **Modifiers are not fabricated dice.** Flat values appear as concise source-labelled toasts after dice settle.
7. **Contributed dice use the contributor's set.** They join the acting player's same handful. This makes help socially visible.
8. **One cohesive set per player initially.** Each fixture player selects one set covering standard die shapes. Custom per-shape sets and unique duplicate collectibles remain future work.
9. **One roller, shared witness.** Only the roller supplies input. A local append-only fixture release starts independent roller and witness presentations. The surface labels this as simulated delivery.
10. **Production remains untouched.** The concept may evolve shared internals behind backward-compatible d20 inputs, but production behavior and data adapters do not change in this slice.

## Concept experience

### Attack group

1. The review scenario supplies an ordered attack group, normally the acting player's d20 plus an optional contributed die such as another player's Bless d4.
2. Each physical die resolves its fixture set from its contributor.
3. Grabbing any die lifts the entire compact group while preserving the touched grab offset.
4. The group follows the pointer/finger exactly in the visible tray plane.
5. Release emits one sanitized group profile into the local fixture host.
6. Roller and witness consume the same profile and supplied faces through independent renderer instances.
7. Dice settle. Any supplied attack reroll/discard sequence plays automatically.
8. The fixture verdict appears.
9. A miss completes. A hit arms the fixture damage group.

### Damage group

1. The tray gathers every supplied damage die in stable order, including base, critical, feature, condition, or contributed dice.
2. The acting player throws the complete mixed group once.
3. Dice settle first on their supplied original faces.
4. Reroll fixtures play automatically in order. In the Great Weapon Fighting scenario, only supplied 1s/2s flash and reroll while unaffected dice remain in place.
5. Supplied signed modifiers appear as source-labelled toasts.
6. The supplied final total lands with the fixture impact.

### Witness and recovery exercises

- The witness never exposes Roll or Grab controls.
- Roller and witness use equal immutable release values but independent scenes, materials, motion, and telemetry.
- Duplicate local release delivery starts no second throw.
- Missing release exercises a bounded neutral auto-release so the fixture never stalls.
- Initial hydration with released fixture history settles immediately instead of replaying stale motion.
- Reduced-motion and forced-provider-failure exercises remain available.

## Fixture contract

The concept owns a provisional component-input vocabulary. It is consumer evidence, not a proto proposal and not production authority.

### Roll scenario

A scenario contains:

- scenario ID and review label;
- roller member and witness member;
- fixture player/set records;
- attack group;
- optional damage group;
- supplied verdict/impact facts; and
- optional review notes that are visibly labelled fixture metadata.

Scenario validation is strict and fails closed before rendering. No fixture parser repairs malformed data.

### Roll group

Each group contains:

- stable key (`attack` or `damage` in this concept);
- ordered physical dice;
- ordered non-die modifiers;
- supplied final total when displayed; and
- supplied verdict/impact labels.

### Physical die

Each die contains:

- stable member ID within the group;
- die kind: d4, d6, d8, d10, d12, or d20;
- original face;
- final face;
- zero or more ordered reroll steps with before, after, reason ref, and display label;
- counted/discarded fixture disposition;
- source ref and display label;
- contributor member; and
- explicit purpose label such as base, critical, feature, condition, or granted.

The renderer never derives member count from notation, invents critical dice, reconstructs rerolls, calculates a total, or guesses a contributor.

A reroll remains history on one physical die. Advantage/disadvantage is represented by multiple physical d20 entries with supplied counted/discarded disposition. This keeps the component shape honest without implementing those production rules.

### Modifier

A modifier contains:

- stable ID;
- signed fixture value or supplied multiplier text;
- source ref and display label;
- optional source member; and
- stable order.

The concept displays these facts. It does not add or apply them.

### Fixture sets

A fixture set maps one stable fixture set ID to visual treatments/presets for supported die kinds. At least two clearly distinct cohesive sets are required so a mixed d20 + contributed d4 visibly proves contributor ownership.

Fixture set IDs and membership are local Concepts Lab data. The surface must not claim they are owned, equipped, persisted, or delivered by production.

Multiple logical dice of one kind may clone the same fixture preset with independent scene instances. They share a style, not collectible identity.

## Shared component architecture

### Roll-group presentation

`DiceTrayPresentation` evolves behind its current one-d20 public behavior into a roll-group-capable presentation boundary. Production's current request adapter continues to project a one-member d20 group unchanged.

The concept uses the group-shaped input directly. It must not fork production components into `src/concepts`.

The shared presentation owns:

- one immutable authoritative fixture request;
- one matching presentation-only release;
- phase progression for attack and damage groups;
- renderer completion/fallback reconciliation;
- automatic reroll phases;
- modifier and impact sequencing; and
- scope reset/discontinuity behavior.

### Exact held mapping

The renderer derives the visible tray plane from the actual camera and viewport. Pointer rays intersect that plane, and the initial grab offset is retained. Group pose is applied by the next animation frame without network or spring-position dependence.

Rotation may be filtered and weighted; anchor position may not lag.

The measurable invariant is: across supported concept viewports and approved camera modes, the projected grabbed point remains within 2 CSS pixels of the pointer after the next rendered frame. Touch keeps the same anchor rule with its larger hit target.

### Group gesture controller

One controller owns:

- per-die projected hit regions;
- whole-group pickup from any member;
- pointer capture and outside release;
- exact held anchor;
- compact held layout;
- local velocity/energy sampling;
- one sanitized release profile;
- cancel/lost-capture/unmount cleanup; and
- exactly-once release commitment.

Raw coordinates, pointer IDs, paths, and sample timing do not enter fixture presentation events or diagnostics.

### Group motion solver

The existing `DiceMotionSolver` boundary remains replaceable. A group-level release profile is combined with each member's stable index to produce deterministic independent member motion.

The solver owns:

- initial release pose;
- travel and tumble;
- member scatter;
- tray contacts/rebounds;
- continuous convergence to supplied faces;
- isolated automatic reroll motion; and
- readable contained resting layout.

Contributor style never changes motion or result.

### Generic die renderer

The current d20 renderer evolves toward a die-kind-neutral renderer behind the same provider/settlement responsibilities:

- strict preset resolution;
- independent scene cloning;
- normalization from asset bounds;
- material treatment;
- supplied result-to-pose settlement;
- observed-result diagnostics where verified metadata permits them; and
- truthful semantic/SVG fallback.

The carved d20 remains the only shape whose exact physical face correctness may be labelled verified in this slice. Other locally available Original shapes may be used as clearly labelled provisional feel assets. Their concept appearance is not production face-map approval.

## Feel candidates

The first pass includes at least three intentionally separated candidates:

### Weighty

- restrained travel;
- slower angular response;
- low, firm rebounds;
- strong pickup/contact shadow and sound cues; and
- a grounded resting cadence.

### Energetic

- faster release response;
- wider scatter;
- more visible tumble;
- lively rebounds; and
- a quicker final cadence.

### Physical

- momentum-forward travel;
- stronger member/tray collision response;
- less obviously authored paths; and
- a test of whether the current deterministic solver can reach the desired physicality.

These names are review handles, not final profile names. Candidate constants remain development fixtures. Kirk may select one, reject all, or ask for a blend. Iteration continues on the concept branch until the feel is approved.

A full rigid-body engine is not assumed. If the physical candidate demonstrates that the current solver cannot reach the approved feel, that finding earns a separate architectural decision; it is not smuggled into this concept.

## Review surface

The existing `?concept=attack-die-3d` route gains a focused roll-group/feel stage rather than another top-level concept renderer.

The stage provides:

- clear candidate selector;
- replay without changing supplied faces;
- roller and witness side by side;
- scenario selector;
- reduced-motion toggle;
- responsive review sizes;
- local simulated-delivery labels;
- provisional/verified asset labels; and
- concise diagnostics separated from the player-facing tray.

Required scenarios:

1. single carved d20;
2. mixed-set d20 + contributed Bless d4;
3. ordinary multi-die damage;
4. critical damage pool;
5. Great Weapon Fighting with at least two affected dice and one unaffected die;
6. duplicate/missing release;
7. reduced motion; and
8. provider/renderer fallback.

The stage optimizes rapid repetition: reset and rethrow are immediate, and changing candidate does not require navigation or a server reset.

## Presentation phases

The concept projects explicit fixture facts through:

1. `attack-armed`
2. `attack-released`
3. `attack-settled-originals`
4. `attack-rerolls` when supplied
5. `attack-verdict`
6. `damage-armed` on a supplied hit
7. `damage-released`
8. `damage-settled-originals`
9. `damage-rerolls` when supplied
10. `damage-modifiers`
11. `impact`
12. `complete`

A miss skips damage. A hit with only flat fixture damage skips physical damage dice but may show modifier/impact facts. Renderer completion acknowledges presentation only; it never changes fixture truth.

## Reroll presentation

For each ordered reroll step:

1. all dice hold their original/current faces;
2. affected dice receive a brief high-contrast flash and semantic cue;
3. only affected dice lift/tumble;
4. they settle on the supplied `after` faces;
5. the reason label appears once for the batch when labels match; and
6. the sequence advances automatically.

The Great Weapon Fighting scenario supplies 1s/2s as affected facts. The client does not implement a `face <= 2` rule; a malformed fixture claiming a different affected face can still be shown as fixture data, while fixture validation separately pins the intended review scenario.

## Modifier presentation

After final dice settle, ordered modifier toasts appear briefly with provider-style fixture labels such as `+3 Strength` or `+2 Rage`. Distinct long-term visual languages are outside this slice.

Toasts do not obscure dice, require input, alter totals, or synthesize missing labels. The final supplied total/impact follows.

## Sound and visual feedback

Feel candidates may vary bounded pickup, tumble, contact, reroll, and final-impact audio. Audio follows presentation contacts and may not drive state transitions. The concept remains understandable muted, and browser autoplay restrictions cannot block rolling.

Held feedback uses lift, contact shadow, subtle pose, cursor change, and optional restrained sound. It does not use a whole-tray grabbed border.

## Accessibility and reduced motion

- Roll remains a first-class button for keyboard, switch, assistive technology, touch, and low-effort play.
- Grabbing is optional and never a skill check.
- Group labels identify contributing players without exposing concealed faces.
- Automatic rerolls do not steal focus or require confirmation.
- Settled semantic output announces original, reroll, replacement, modifier, and final fixture facts once in readable order.
- Reduced motion preserves explicit input, contributor styling, affected-die distinction, final faces, modifiers, and semantics while suppressing travel, tumble, bounce, and camera motion.
- Pointer cancel, lost capture, unmount, scenario change, or candidate change leaves the group armed or exactly-once released—never half committed.
- WebGL, provider, shader, audio, or solver failure falls back semantically and does not stall the scenario.

## Failure behavior

| Failure | Concept behavior |
| --- | --- |
| Malformed fixture | Refuse scenario before mounting the tray |
| Missing physical die facts | Semantic fixture result only; never fabricate dice |
| Unknown contributor/set | Honest default/SVG style with fixture warning |
| Asset/hash/face-map failure | Conceal until release, then truthful semantic/SVG settlement |
| Renderer/solver/WebGL failure | Exactly-once fallback completion; sequence continues |
| Lost capture/cancel | Return to armed unless release already committed |
| Duplicate local release | First valid release wins; no second throw |
| Missing local release | Neutral auto-release after bounded concept grace |
| Hydrated released history | Immediate settled projection; no stale replay |
| Reduced motion | Static held cue and direct exact settlement with full semantics |

## Testing

### Fixture and state

- Strict scenario validation rejects missing IDs, unsupported faces, invalid reroll chains, duplicate die IDs, unknown contributors, and malformed modifier order.
- Attack miss skips damage; hit arms exactly one damage group.
- Rerolls and modifiers advance automatically in supplied order.
- Duplicate requests/events never create duplicate groups, rerolls, toasts, or announcements.

### Interaction

- Projected grabbed-point error remains at most 2 CSS pixels after the next frame across approved viewports/cameras.
- Every member can begin a whole-group grab from its projected hit region.
- Outside release commits once; cancel/lost capture/unmount reset deterministically.
- Keyboard Roll produces a neutral profile through the same release path.
- No serialized held/profile/event/diagnostic object contains raw pointer fields.

### Motion and rendering

- Every candidate consumes identical supplied faces and settles those faces.
- Member variation is deterministic from group profile and stable index.
- Multi-die resting poses remain contained, readable, and non-overlapping.
- Reroll motion affects only supplied members.
- Roller/witness use equal release values and independent renderer resources.
- Unknown/provisional assets are labelled and fall back without false face-correctness claims.

### Accessibility and failure

- Focus, labels, live announcements, reduced motion, and button equivalence pass component tests.
- Forced provider, WebGL, solver, and release failures complete once and never stall.
- The concept remains usable at the existing 1024×768 floor and representative narrower touch layout.

## Human feel gate

Automated tests report attachment accuracy, frames, containment, exact supplied settlement, and failures. They do not declare a candidate enjoyable.

Kirk repeatedly throws:

- the single d20;
- mixed d20 + contributed d4;
- ordinary multi-die damage;
- critical damage; and
- Great Weapon Fighting rerolls.

He evaluates attachment, weight, responsiveness, travel, tumble, impact, reroll clarity, total pacing, and whether mixed dice still read as one handful. The concept remains in iteration until he approves a direction.

The completion evidence records:

- reviewed web commit;
- chosen or blended candidate revision;
- tested scenarios/viewports/input modes;
- measured attachment and renderer facts;
- known provisional asset limitations; and
- Kirk's observed approval or requested next iteration.

## Delivery

This slice uses one rpg-dnd5e-web concept branch from `origin/dev`. It may use locally synced licensed dice assets and fixture manifests. No licensed GLB, private screenshot, or private evidence enters a public repository.

The concept is exercised locally through the existing Concepts route. No proto, toolkit, API, deployment, or production web branch is created. No dependency merge is needed.

If the concept earns production continuation, its actual component-input needs become evidence for a later cross-repository design. That later design decides persisted roll facts, public set projection, real shared release, reconnect, and production promotion. Nothing in this concept pre-approves those contracts.

## Repository changes

### `rpg-dnd5e-web`

Expected concept/shared-component areas:

- `src/components/ui/dice/` for group-capable shared interaction, motion, rendering, fixture-independent event/state boundaries, and tests;
- `src/concepts/attack-die-3d/` for fixture scenarios, candidate controls, simulated delivery, diagnostics, and review composition;
- `docs/how-to/attack-die-3d-concept.md` and Concepts documentation for the reproducible review path; and
- concept evidence documentation without licensed/private artifacts.

Production session-combat adapters and `CombatExperience` behavior remain unchanged.

### `rpg-project`

This document records the approved concept and later gates. The journey remains open after the concept; concept approval is not production delivery.

## Non-goals

- Proto, toolkit, API, deployment, or production session-route changes.
- Real multiplayer release transport.
- Persisted or authoritative equipped-set ownership.
- Production-correct set-wide face maps beyond already verified facts.
- Client calculation of attacks, damage, criticals, rerolls, modifiers, or totals.
- Gesture influencing supplied faces.
- Custom per-shape sets or individually inventoried duplicate dice.
- Dice jail, commerce, trading, or rewards.
- Frame-perfect network synchronization.
- A mandatory rigid-body engine.
- Choosing final motion constants in prose before hands-on iteration.

## Later production questions earned by this concept

The concept must report, not answer prematurely:

1. the minimal physical-die and modifier facts the production event must provide;
2. whether contributor member alone is sufficient for appearance resolution;
3. the shared release values witnesses actually need;
4. the bounded fallback timing that feels non-stalling;
5. which die shapes/assets require production semantic correction;
6. whether deterministic choreography reaches the approved feel or rigid bodies are justified; and
7. the observed performance envelope for realistic group sizes.

— ui-ux agent, on behalf of KirkDiggler
