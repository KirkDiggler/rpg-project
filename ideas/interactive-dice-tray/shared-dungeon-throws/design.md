# Shared Dungeon Dice Throws — Production Design

**Status:** Revised design approved in chat by Kirk 2026-08-28; written review pending

**Tracking:** [rpg-project#289](https://github.com/KirkDiggler/rpg-project/issues/289) · production design slice [rpg-project#303](https://github.com/KirkDiggler/rpg-project/issues/303) · design PR [rpg-project#304](https://github.com/KirkDiggler/rpg-project/pull/304)

**Foundation:** [interactive collectible 3D dice tray](../design.md) · [shared table dice ritual](../shared-table-ritual/design.md) · rigid-body concept [rpg-dnd5e-web#830](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/830)

## Purpose

Make an attack d20 feel like a physical object the active player carries into the real dungeon and throws, while every connected player witnesses the same conversationally meaningful event.

The authoritative attack result remains unchanged. Gesture, rigid-body physics, contacts, off-table retries, and face correction are presentation only. Clients agree on the same authored wall or shut door contacts and the same settled/off-table terminal outcome; they do not need pixel-identical trajectories.

This revision replaces the rejected production integration attempted on `rpg-dnd5e-web` issue #837. That integration proved the transport and physics but grafted them onto the legacy production dice drawer, creating two renderers and overlapping release, settlement, telemetry, and reveal lifecycles. The replacement starts from current `origin/dev`, imports no code from the discarded web branch, and rebuilds the approved concept behavior behind one production controller and one WebGL renderer.

## Product outcome

With two authenticated players on the production session route:

1. no tray or dice panel is visible while no player-owned attack roll is armed;
2. an active player's attack presents a small temporary DOM launch tile over the map;
3. left-drag carries the die from that tile into the real dungeon;
4. left remains held while right+vertical movement raises or lowers it;
5. releasing left creates one bounded throw plan and starts one world throw on each client;
6. both clients witness the same named wall/shut-door contacts and settled/off-table outcome;
7. an off-table attempt keeps result, damage, and combat log concealed, returns the actor's launch tile with an explicit retry message, and shows witnesses a noninteractive retry status;
8. a settled attempt corrects once to the already-authoritative face, visibly freezes, and only then reveals result, damage, and combat log; and
9. Roll, reduced-motion, and semantic fallback paths remain fully supported.

Physical dexterity never changes gameplay. An off-table retry uses the same authoritative result; it never requests or invents another roll.

## Scope

This wave includes:

- one attack d20 using verified preset `dice.original.carved.d20`;
- a DOM-only launch tile and neutral Roll control;
- direct handoff into the real `SessionCanvas`;
- pointer capture, map-event isolation, horizontal carry, and two-button lift;
- fixed-step Rapier pre-simulation and visible playback;
- floor, authored-wall, shut/locked-door, and active-dice collision truth;
- the existing group-shaped publish/live-stream contract;
- actor and witness playback through one world-dice controller;
- off-table presentation attempts;
- one late authoritative-face correction;
- truthful reduced-motion and failure fallback; and
- local-feel and two-browser human approval gates.

It does not include:

- the legacy production `DiceDrawer`, `DiceTrayPresentation`, or a tray Canvas;
- deleting or rewriting Concepts Lab history;
- damage or contributed dice;
- equipped dice-set projection;
- server-side physics;
- exact trajectory streaming;
- result-dependent physics generation;
- tray art, audio, haptics, or release-pause masking;
- props, items, characters, or monsters as colliders; or
- broad mobile/performance graduation beyond the load-bearing fixes named here.

## Decisions retained from the approved design

The clean rebuild does not reopen the multiplayer contract.

1. **Client-generated full plan.** At release the roller pre-simulates the fixed Rapier world before visible playback.
2. **Server relay, not simulation.** The game server authenticates, validates, deduplicates, and broadcasts the plan without running physics or touching toolkit Story.
3. **Conversational identity, not frame identity.** Sparse wall, shut-door, die-to-die, and terminal records coordinate meaningful outcomes while local simulation owns intermediate pixels.
4. **Separate presentation service.** Decorative plans stay outside the toolkit-mirroring `SessionService` and authoritative Story sequence.
5. **Group-shaped foundation.** The contract supports up to 20 bodies and per-body terminals even though this product adapter supplies one d20.
6. **Snapshot collision truth.** Door and world membership are frozen per attempt and protected by a canonical fingerprint.
7. **Physics never carries authority.** Plans contain no result, hit/miss/critical, damage, target HP, or arbitrary asset selector.
8. **Fail open truthfully.** Unsupported or unavailable presentation never stalls authoritative combat indefinitely.
9. **Server simulation remains deferred.** It earns a new design only if matching schema, snapshot, plan, and checkpoints repeatedly fail the two-browser conversational gate.

## Clean production architecture

```text
authoritative attack presentation
             |
             v
      WorldDiceController
       /       |        \
launch tile  planner   transport
       \       |        /
        accepted DiceThrowPlan
                 |
                 v
   DungeonDicePlaybackLayer
       inside SessionCanvas
                 |
       settled / off-table / failure
                 |
                 v
         combat reveal gate
```

### One production renderer

`SessionCanvas` remains the only production WebGL Canvas. The carved 3D die exists only inside that Canvas. The DOM launch tile is not a renderer and owns no animated 3D die.

Raw Rapier pre-simulation is a short-lived computation world, not a second visual renderer or presentation lifecycle. It produces an immutable plan and is disposed before visible playback begins.

The Concepts Lab retains `DiceTrayPresentation`, its historical tray renderer, and the rigid-body proof. Production imports nothing from `src/concepts/**`, and the clean rebuild does not refactor the concept to consume production code.

### WorldDiceLaunchTile

The actor-only launch tile:

- is absent while idle;
- appears when a locally owned authoritative attack d20 arms;
- offers a visible die target plus an accessible Roll control;
- owns pointer capture from pointer-down through release/cancel, including movement outside its visible bounds;
- remains invisibly mounted as capture owner after world handoff;
- stops active gesture events before they reach map hover, movement, camera, or targeting interactions;
- hides its visible token when the real 3D die mounts under the pointer;
- returns with `Off the table — throw again` after an off-table terminal; and
- contains no Canvas, Rapier world, settlement observer, or result state.

Pointer-down before valid handoff, cancellation, lost capture, or release outside valid floor returns the token without publishing.

### WorldDiceController

One controller owns the complete production presentation lifecycle:

```text
idle
  -> armed
  -> held
  -> planning
  -> waiting-for-accepted-plan
  -> playing
  -> retry | settled | fallback
```

It owns:

- session, presentation ID, authority sequence, roller, and attempt;
- actor versus witness role;
- the launch tile command;
- held world state;
- immutable collision snapshot;
- plan generation and publication;
- accepted response/stream deduplication;
- visible playback command;
- off-table retry and settled completion;
- reveal-gate completion; and
- cancellation of every stale pointer, promise, timer, subscription callback, plan, and physics generation.

It does not render UI, calculate gameplay results, or translate physics into the legacy tray-release event.

### DungeonDicePlanner

The planner is framework-free and consumes only immutable inputs:

- presentation identity and attempt;
- canonical world snapshot;
- stable body list; and
- bounded initial rigid-body state derived from the gesture or neutral Roll profile.

It dynamically loads raw Rapier, steps at 60 Hz, records sparse meaningful contacts and one terminal per body, returns a strict draft or a typed fallback, and disposes the world in every terminal path.

A result or face target is never an input.

### SessionDiceThrowTransport

The transport is a thin adapter over the shipped presentation contract. It:

- publishes a draft for the authenticated session member;
- consumes the live-only stream;
- strictly parses immutable plans;
- deduplicates the actor's publish response and stream echo; and
- fences reconnect and prior-scope callbacks.

It knows nothing about launch UI, combat reveal, physics bodies, or Story.

### DungeonDicePlaybackLayer

The playback layer mounts inside the existing generic `SessionCanvas.presentationLayer` seam. Actor and witness use the same implementation and inputs.

It owns:

- one visible Rapier world;
- canonical physical colliders;
- one verified carved mesh per active body;
- fixed-step plan advancement;
- sparse contact corrections;
- per-body terminal handling; and
- one authoritative-face correction after a settled terminal.

It emits sanitized contacts, terminal completion, readiness, progress, and typed failure. It never emits gameplay results.

### Combat reveal gate

The authoritative attack response/event remains the only source of attack result and Story truth. Presentation may conceal that truth, but physics never modifies it.

For a player-owned physical roll, the gate opens only after:

- a settled terminal reaches the matching controller scope;
- the authoritative face correction has completed;
- the corrected face has been observed as drawn; and
- the body is fixed rather than still simulating.

The gate may also open through an explicit semantic control or a classified presentation failure after the failed physical body is removed. It does not open on first release or first accepted plan.

Game state, action economy, query refreshes, and server authority remain immediate. Only the paced result, matching damage toast, and corresponding combat-log/Story presentation wait.

## Experience flows

### Actor pointer throw

1. A locally owned authoritative attack presentation enters `armed`.
2. The launch tile appears; the result remains concealed indefinitely while the actor has a valid control.
3. Pointer-down on the die target captures the pointer and starts local carry.
4. Crossing a valid visible dungeon floor point mounts the carved body in `SessionCanvas` under the pointer and hides the tile token.
5. Left-drag controls X/Z through the actual session camera projection.
6. While left remains held, right+vertical movement freezes X/Z and changes bounded height.
7. Releasing right resumes horizontal carry. Releasing left commits one attempt.
8. The held world body freezes while the planner and publish request complete. The target budget for this pause is under 250 ms; masking animation is deferred polish.
9. The controller accepts either the publish response or matching stream echo exactly once.
10. Actor and witnesses start the accepted plan through the same playback layer.

### Actor Roll control

Roll creates a neutral bounded launch from the nearest valid unoccupied floor origin and follows the same planner, publish, acceptance, playback, terminal, and reveal path. It is not a legacy semantic release shortcut.

If physical presentation cannot be constructed, the same tile changes to an explicit semantic fallback control rather than mounting a second renderer.

### Witness

A witness never receives held pointer motion and never gets an interactive launch tile.

A matching authoritative attack causes the witness controller to wait for a matching plan. On acceptance it validates correlation and collider fingerprint, then mounts the same playback layer. After an off-table terminal it displays a small noninteractive status such as `<roller> is throwing again` while awaiting the next attempt.

### Settled

The planner records an in-bounds low-energy terminal. Visible playback reaches that terminal, reconciles terminal position and zero velocity without first snapping through an unnecessary pre-simulated terminal quaternion, then performs one correction to the locally known authoritative face.

After the corrected face has rendered, the body becomes fixed, the reveal gate opens, and the frozen die remains for a short result beat before unmounting. There must be no trailing physical rolls or additional quaternion correction after damage/log reveal.

### Off-table

The planner records `OFF_TABLE` when a body leaves the logical floor-support mask or crosses the shared vertical threshold. At that terminal each client removes the body.

For this one-d20 wave:

- result, damage, and combat log remain concealed;
- the actor increments `attempt` and receives the retry launch tile;
- witnesses receive only retry status;
- the same authoritative face remains the eventual target; and
- no generic reveal timer runs while the retry remains active.

Attempts are bounded by the existing contract at 32. At the bound, the actor receives an explicit semantic settlement control rather than attempt 33.

## Collision world and performance

### Canonical logical snapshot

The web builds one immutable logical `DungeonDiceWorldSnapshot` per attempt. It contains sorted records for:

- every floor support cell;
- every authored wall run;
- every shut or locked door gap; and
- every active die body.

Open doors are omitted. Door membership is frozen for the attempt. Props, equipment, loose items, characters, monsters, and decorative meshes are absent.

Stable IDs are deterministic and bounded. Oversized authored wall keys are represented by deterministic geometry-bound hashes rather than copied directly into the wire contract.

The fingerprint canonically includes physics schema, sorted logical colliders, material/body facts, and stable body descriptors. Every client recomputes the 32-byte SHA-256 value.

### Physical floor optimization

Logical floor cells remain separate for fingerprinting and exact off-table support checks, but raw and visible Rapier worlds do not create one rigid collider per cell. They deterministically derive one aggregate physical floor from the logical mask while retaining individual wall and shut-door colliders.

This avoids the rejected integration's 319-floor-collider runtime cost. Crossing outside the logical support mask still produces `OFF_TABLE`, even while the aggregate physical floor prevents expensive per-cell contact work during ordinary rolling.

A future exact edge mesh may replace the aggregate floor under a new physics schema if visible edge behavior earns that complexity.

### Fixed schema

`RAPIER_DUNGEON_D20_V1` retains the shipped limits:

- 60 Hz fixed steps;
- at most 480 steps;
- at most 20 bodies;
- at most 128 meaningful contacts;
- at most 256 checkpoint body states;
- at most 32 attempts;
- at most 64 KiB encoded plan size; and
- D20-only physical bodies under this schema.

A missing terminal at step 480 is a planning failure, not an invented settlement.

### Contact coordination

Plans record wall, shut-door, and die-to-die contact transitions with complete post-step state for only the involved body set. Same-step contacts are legal and use deterministic UTF-8 secondary ordering. Floor contacts remain local physics.

Visible playback applies checkpoints after the named fixed step. Clients therefore describe the same meaningful contact even when insignificant intermediate motion diverges.

## Existing wire and game-server boundary

Proto release `v0.1.145` provides `dnd5e.api.session.presentation.v1alpha1.SessionPresentationService`:

- `PublishDiceThrow`
- `StreamDiceThrows`

The group-shaped plan carries schema version, server-bound session/roller identity, presentation identity, authority sequence, attempt, physics schema, collider fingerprint, initial body states, sparse contact checkpoints, and per-body terminals.

It carries no result, hit/miss/critical, damage, HP, target, attack reference, preset ownership, arbitrary URL, raw pointer samples, or prose.

The game server implementation under rpg-api issue #852 / PR #853 owns:

- authentication;
- member ownership and session seating;
- strict structural and numeric validation;
- server-bound roller identity;
- Redis first-valid deduplication;
- cross-instance Pub/Sub delivery; and
- cancellation/resource cleanup.

It never imports Rapier, builds colliders, reads attack outcomes, changes toolkit Story, or simulates physics. Redis delivery is live-only and transient. Missing presentation plans fall back; they are not reconstructed from Story.

## Correlation and synchronization

A client accepts a plan only when all of these match locally accepted authoritative truth:

- session;
- presentation ID;
- authority sequence;
- roller;
- expected attempt;
- known physics schema;
- body contract; and
- collider fingerprint.

The actor may accept its publish response or stream echo; equal identity is consumed once. Witnesses accept the stream. A short receipt-relative synchronization buffer is allowed, and the release-to-visible target remains under 250 ms on the actor's normal path.

Plan-before-event is bounded and buffered. Event-before-plan is bounded for witnesses and then semantically settles. Catch-up/hydrated history settles immediately and never replays a stale physical throw.

## Failure behavior

| Failure | Behavior |
| --- | --- |
| Release before valid handoff | Return token; publish nothing |
| Pointer cancel/lost capture | Remove held body, restore tile, publish nothing |
| Plan generation fails or lacks terminal | Remove/freeze physical body; actor receives explicit semantic fallback |
| Publish fails | Stop physical attempt and settle truthfully; never begin a private actor-only throw |
| Plan arrives before event | Buffer briefly by full identity, then drop |
| Witness has event but no plan | Bounded semantic settlement |
| Unknown schema/body contract | Refuse playback; semantic settlement |
| Fingerprint mismatch | Refresh live doors once, wait briefly, then semantic settlement |
| Unknown contact collider | Treat as plan/fingerprint failure; never invent a collider |
| Provider/WebGL/Rapier failure | Unmount failed body before semantic settlement |
| Playback stops progressing | Classify as playback failure, unmount/freeze first, then reveal truthfully |
| Active off-table retry | No reveal timer; actor retains retry/fallback controls |
| Stream reconnect | No replay; current presentation remains fenced or falls back |
| Reduced motion | Explicit control followed by direct authoritative settlement |
| Scope/session/member change | Dispose pointer, timers, plans, worlds, bodies, and stale callbacks |

A safety watchdog may classify a genuinely stalled playback, but it cannot merely reveal while an active die remains rolling. Failure settlement first removes or fixes the physical presentation; damage/log reveal follows.

## Accessibility

- The Roll control is equivalent to pointer throwing for authority, transport, and settlement.
- No accessibility path requires two simultaneous pointer buttons.
- Keyboard, switch, touch, reduced-motion, and provider-failure users retain an explicit completion control.
- Retry and witness status use polite semantic announcements.
- Pointer gesture ownership suppresses accidental map movement without disabling ordinary map controls outside the active gesture.
- Reduced motion performs direct authoritative settlement rather than travel or repeated correction.

## Testing

### Unit and component coverage

TDD covers:

- absence of production `DiceDrawer`, `DiceTrayPresentation`, and a second Canvas;
- no dice UI while idle;
- actor-only launch tile and witness-only retry status;
- pointer capture through outside movement, map-event isolation, cancellation, and cleanup;
- direct handoff, horizontal carry, two-button lift, and neutral Roll;
- immutable plan parsing and forbidden-field rejection;
- canonical snapshot/fingerprint and bounded wall IDs;
- aggregate physical floor plus logical per-cell support mask;
- open-door omission and shut/locked-door inclusion;
- plan generation, same-step contacts, terminals, bounds, and disposal;
- actor response/echo deduplication and witness stream acceptance;
- stale session/presentation/attempt/renderer fencing;
- off-table concealment and attempts 1–32;
- one authoritative-face correction and drawn-settlement gate;
- no reveal while a physical body remains active;
- reduced-motion, catch-up, monster, missing-plan, transport, provider, and playback failures; and
- a production boundary guard forbidding `src/concepts/**` and legacy tray imports.

Existing combat, camera, movement-budget, downed-reveal, equipment, weapon, Story, provider, and session-route tests remain green.

### Human approval gates

Automated equality and screenshots do not approve feel. Kirk is explicitly looped in at two implementation checkpoints.

#### Gate 1 — local production feel, before multiplayer wiring

On the real production route with one player, Kirk verifies:

- no old tray is visible at startup;
- the launch tile appears only for an armed player-owned roll;
- pickup, carry, map isolation, lift, and release feel connected;
- wall/door collisions feel physical;
- off-table return clearly asks for another throw;
- settled correction is one clean beat with no trailing rolls; and
- damage/log reveal follows visible settlement.

Implementation stops at this gate until Kirk approves the local ritual.

#### Gate 2 — integrated two-player acceptance

With two independent authenticated players, Kirk verifies:

- exactly one plan is published for one release;
- exactly one world throw appears per client;
- both clients report the same meaningful wall/shut-door contacts and terminal;
- an open door is omitted and does not create an invisible hit;
- off-table keeps both clients concealed, returns only the actor's control, and gives the witness retry status;
- a retry settles to the same authoritative face on both clients;
- reduced-motion and missing-plan paths complete truthfully;
- reconnect does not replay stale dice; and
- no page errors, duplicate renderer, weapon regression, camera reset, or Story/result disagreement occurs.

The web slice is not complete until Kirk approves this walk.

## Delivery

### Repository state

- `rpg-api-protos` PR #257 is merged and published as `v0.1.145`.
- `rpg-api` issue #852 / PR #853 retains the presentation transport and targets `dev`.
- The discarded web branch `feat/837-shared-dungeon-dice` remains local evidence only and will not be cherry-picked.
- The replacement web branch reuses issue #837, starts from freshly fetched `origin/dev`, and opens one PR to `dev`.
- This design PR stays open and merges last.

### Clean web implementation rule

The replacement may read the approved concept and current `origin/dev`, but it does not copy or cherry-pick code from the discarded production integration. In particular it does not preserve that branch's coordinator, tray adapters, settlement glue, or reveal timers.

The concepts remain historical working artifacts. Clean production code is allowed to reimplement a proven behavior behind appropriately named production interfaces; production never imports the concept.

### Development order

```text
revised design approval
  -> fresh web branch from origin/dev
  -> remove legacy tray from production composition
  -> local one-controller ritual
  -> Kirk local-feel gate
  -> existing multiplayer transport integration
  -> Kirk two-player gate
  -> full web CI and one review round
  -> provider/API/web merge order
  -> design-record merge
```

The API branch is reconciled and published before the web consumer merges. Normal repository gates and one answered Copilot review round apply.

## Deferred work and escalation triggers

### Server simulation

Do not add it now. Revisit only if two-browser evidence repeatedly shows different meaningful contacts or terminal outcomes despite matching schema, fingerprint, plan, and checkpoint application, or if dice position becomes gameplay-authoritative.

The comparison at that point is server pre-simulation plus sparse plans versus trajectory/keyframe streaming. Do not jump directly to per-frame network physics.

### Damage and contributed dice

The existing service remains group-shaped. A later slice supplies authoritative per-die/subset facts, verified non-d20 asset/collider contracts, and retry policy for subsets. This wave does not infer those rules.

### Polish

Launch-tile art, pause-masking thrower animation, face-steering improvements, audio, haptics, exact dungeon-edge collision mesh, and broad Discord/mobile performance budgets are separate slices. The clean boundary must leave room for them without introducing another renderer or lifecycle.

## Non-goals

- Gesture influencing a game result.
- Server-side physics.
- Pixel-identical trajectories.
- Persisted or replayed presentation plans.
- Decorative events in toolkit Story.
- Legacy tray compatibility in production.
- Damage, critical, reroll, modifier, or contributed-die presentation.
- Equipped collectible-set ownership.
- Arbitrary plan-selected assets.
- Streaming held pointer motion.
- Rewriting or deleting Concepts Lab history.

— cross-team agent, on behalf of KirkDiggler
