# Shared Dungeon Dice Throws — Production Design

**Status:** Draft for written review; proposal approved by Kirk 2026-08-27

**Tracking:** [rpg-project#289](https://github.com/KirkDiggler/rpg-project/issues/289) · production design slice [rpg-project#303](https://github.com/KirkDiggler/rpg-project/issues/303)

**Foundation:** [interactive collectible 3D dice tray](../design.md) · [shared table dice ritual](../shared-table-ritual/design.md) · production session combat [rpg-dnd5e-web#822](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/822) · rigid-body concept [rpg-dnd5e-web#830](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/830)

## Purpose

Promote the approved rigid-body attack-die experience into the real session route and let the whole table witness the same meaningful throw.

The acting player carries the verified Original carved d20 from the existing dice drawer into the actual dungeon, controls its position and lift, and releases it into real rigid-body collisions. Every connected player sees the same conversationally important events: the same authored wall or shut door is hit, and the die either settles on the dungeon or rolls off the table. Minor differences in spin, contact point, and bounce height are acceptable.

The authoritative attack result remains unchanged. Physics, gesture, shared release, collision checkpoints, off-table attempts, and face correction are presentation only.

## Product outcome

With two authenticated players in the production session route:

1. the active player attacks and receives one armed carved d20 in the existing drawer;
2. left-drag carries it onto the real dungeon floor;
3. left remains held while right+vertical movement raises or lowers it;
4. releasing left throws it;
5. both players see the same named wall/door contacts and the same settled/off-table terminal outcome;
6. an off-table throw returns both presentations to the tray for another physical attempt without changing the attack result; and
7. the first accepted throw plan crosses the existing Story/result reveal gate; and
8. a valid physical attempt settles to the already-authoritative d20 face.

A player using the Roll button, reduced motion, or semantic fallback remains fully supported. Physical dexterity never changes gameplay.

## Scope

This production wave includes:

- one attack d20;
- the verified default preset `dice.original.carved.d20`;
- the existing production dice drawer;
- tray-to-dungeon handoff;
- two-button lift;
- fixed-step Rapier simulation against floor cells, authored walls, shut/locked doors, and active dice bodies;
- a client-generated throw plan with sparse contact checkpoints;
- a separate presentation-only publish/stream service;
- shared roller/witness playback;
- off-table presentation attempts;
- late authoritative-face assist;
- reduced motion and semantic fallback; and
- two-browser production-route evidence.

It does not include damage dice, contributed dice, equipped-set projection, server-side physics, exact trajectory streaming, tray art polish, face-assist polish, or a dedicated performance pass.

## Decision summary

1. **Build multiplayer coordination with production physics.** Actor-only physics would teach the wrong lifecycle because witnesses would still see a different neutral roll.
2. **Target conversational identity, not frame identity.** Everyone must agree on wall/door contacts and off-table versus settled. Pixel-identical paths are not required.
3. **Pre-simulate on the roller.** The roller runs the same fixed-step Rapier world faster than real time at release and publishes a compact plan before visible playback. Contact facts therefore arrive before the bounce rather than too late to coordinate it.
4. **The server relays but does not simulate.** It validates, deduplicates, and broadcasts untrusted presentation plans. A light server simulation is deferred until client evidence proves it earns its cost.
5. **Keep presentation off the toolkit event spine.** `SessionService` mirrors the toolkit session SDK field-for-field. Decorative physics gets a separate service, sequence, and reconnect policy.
6. **Preserve current reveal authority.** The first accepted release plan crosses the existing reveal gate. If that physical attempt later rolls off the table, Story remains revealed and the next throw is a presentation-only attempt.
7. **Snapshot collision truth per attempt.** A door opening during or after a throw does not rewrite that throw's world. The next attempt uses the latest door state.
8. **Fail open to truthful settlement.** Missing transport, mismatched colliders, unsupported physics, provider failure, reconnect, or reduced motion never stalls the authoritative combat presentation.

## Current reality

### Production combat

`rpg-dnd5e-web#822` ships the production `CombatExperience` on the session route. The acting player explicitly presses Roll or grabs/releases one carved d20. Story and the semantic verdict remain concealed until release. The actor's Attack response may arrive before the matching typed stream event; the existing presentation reducer already supports release while waiting for that event. Witnesses currently create a deterministic neutral release from their authoritative event and auto-settle independently.

The release profile is local presentation data. It is not delivered to teammates.

### Shared dice components

The durable chain is:

```text
DiceTrayPresentation
  -> DiceTray3D / RuntimeDiceMesh
    -> gesture controller
      -> DiceMotionSolver
        -> verified runtime provider and semantic fallback
```

The shared-table concept added exact camera-plane attachment, roll-group boundaries, independent roller/witness renderers, and provisional multi-die presentation without changing production inputs. The current production d20 request/release schemas remain strict and outcome-safe.

### Rigid-body proof

`rpg-dnd5e-web#830` added a concept-only Rapier proof using the actual carved d20 and the real `SessionCanvas`. It proved:

- convex-hull dice collision;
- gravity, friction, restitution, damping, and spin;
- floor-cell, authored-wall, and shut-door proxy colliders;
- tray-to-dungeon handoff;
- left+right lift;
- off-floor return to tray; and
- late authoritative-face correction.

Kirk approved real rigid-body physics as the production direction. The concept architecture is intentionally not shippable unchanged: Rapier is eagerly imported, concept controls own production-like state, the launch tray is a plain proof, and collision/release facts are not shared.

### Existing server delivery

The authoritative session stream is a per-recipient projection of toolkit events. `GetStory` is its replay source. The game server's current broker is best-effort and in-process; authoritative gaps are repaired from toolkit Story.

A dice throw plan is different:

- it is one shared presentation occurrence rather than per-recipient game truth;
- it has no rulebook source;
- missing it requires settlement fallback, not Story replay; and
- adding it to toolkit sequence numbers would make decorative transport look authoritative.

The production presentation channel therefore has its own service and no replay obligation.

## Experience flow

### Actor: pointer throw

1. A locally owned authoritative attack presentation enters `armed`.
2. The existing drawer shows the verified carved d20 and waits indefinitely for explicit actor input.
3. Pointer-down must begin on the die's projected hit region. The drawer surface captures the pointer.
4. While the pointer remains inside the drawer, the normal tray-held pose remains visible.
5. When the pointer exits over a valid visible dungeon floor point, the tray renderer hides and the same preset mounts in the production `SessionCanvas` beneath the pointer.
6. Left-drag controls dungeon X/Z using the actual session camera and a horizontal world plane.
7. While left remains held, holding right freezes X/Z and maps vertical pointer movement to bounded height.
8. Releasing right resumes X/Z carry. Releasing left commits the attempt.
9. Release state is converted to a bounded rigid-body launch, simulated offscreen, and published as one throw plan.
10. The held die remains frozen for the short publish/synchronization interval; it never begins a private visible throw that witnesses cannot join.
11. The actor consumes the accepted response or matching stream echo exactly once and starts visible playback.

Releasing or cancelling before valid dungeon handoff returns the die to the drawer and publishes nothing.

### Actor: Roll button

The existing Roll button remains first-class. It creates a neutral launch from the nearest unoccupied valid floor cell, at the default lift, through the same pre-simulation, publish, playback, and settlement path. It does not bypass shared coordination.

If world-space physics cannot load, the button preserves the current semantic release control. A neutral presentation plan may still be published when plan generation is available without WebGL; otherwise witnesses use bounded missing-plan fallback.

### Witness

A witness never receives held pointer motion and never controls the actor's die.

When a matching plan arrives, the witness:

1. confirms the matching authoritative attack request is locally accepted;
2. confirms the roller and attack sequence match;
3. confirms its collider fingerprint matches the plan;
4. waits the synchronization buffer; and
5. mounts the die at the plan's release state and runs visible playback.

The witness therefore sees the throw from release onward, not the actor carrying and lifting it.

### Contact playback

Every client steps the same body at 60 Hz. At each planned wall/door checkpoint, the client applies the recorded post-contact rigid-body state after that physics step. With matching simulations this assignment is imperceptible because values already agree. Under small cross-browser divergence it is a sparse correction at the moment of impact, ensuring the subsequent bounce belongs to the same collider.

Only wall and shut-door contacts are social milestones in v1. Floor contacts remain ordinary local physics. The full physical path is not transmitted.

### Settled terminal

A `SETTLED` plan ends at the recorded low-energy in-bounds body state. Each client then performs the existing late authoritative-face assist against the result already supplied by its authoritative attack event. The body freezes only after correction completes.

The 320 ms correction is accepted temporary polish debt. Improving it through trajectory search, torque steering, or less visible correction is a later slice.

### Off-table terminal

An `OFF_TABLE` plan visibly leaves valid floor support or falls beneath the approved threshold. At the recorded terminal step every client removes the world body and returns the die to its drawer.

The actor's presentation re-arms as `attempt + 1`. Witnesses wait for that next plan. The server never rerolls and the web never asks it to; the same authoritative face remains the eventual target.

Story is revealed by the first accepted release plan and remains revealed across off-table attempts. This keeps the current release boundary and makes the repeated throw explicitly presentation-only.

After 32 off-table attempts, the physical retry control yields to direct semantic settlement. This is a safety bound, not a dexterity challenge; semantic settlement is also always available through failure and reduced-motion paths.

## Collision world

### One canonical snapshot builder

The web owns one pure `DungeonDiceWorldSnapshot` builder. Both hidden pre-simulation and visible R3F/Rapier playback consume its immutable output. Neither rebuilds collision decisions independently.

The snapshot contains sorted records for:

- one floor box per dungeon floor cell;
- one box per authored wall run;
- one box per shut or locked door gap;
- active dice bodies participating in the same presentation; and
- material constants, body dimensions, gravity, timestep, and out-of-bounds thresholds fixed by the physics schema.

Stable collider IDs are:

```text
floor:<cube-key>
wall:<authored-wall-run-key>
door:<connection-id>
dice:<presentation-id>:<die-id>
```

The initial production product has one active attack d20, so dice-to-dice collision is exercised through component/physics tests rather than a player-facing multi-die group. The collision world and body filtering do not need replacement when authoritative damage groups arrive.

### Deliberately absent colliders

The following never enter the snapshot in this wave:

- props;
- equipment or loose items;
- local or remote characters;
- monsters; and
- decorative dungeon meshes that are not authored wall/door boundaries.

The rendered asset is not the collision authority. Atlas floor cells, authored wall runs, and live door state are.

### Door snapshot

Door state is frozen when an attempt is generated. Open doors are omitted; shut and locked doors are colliders. A subsequent door event changes the next attempt only.

This avoids one client's mid-flight door refresh changing a trajectory another client already planned. Turn sequencing makes such overlap unlikely, but the contract remains explicit.

### Collider fingerprint

The snapshot is canonically encoded from physics schema, sorted collider records, dimensions, transforms, material constants, and active dice initial records, then SHA-256 hashed. The plan carries exactly 32 fingerprint bytes.

Clients recompute rather than trusting the publisher. A mismatch triggers one live-door refresh and a 500 ms bounded wait. If the fingerprint still differs, the client refuses physical playback and uses truthful settlement fallback.

## Client-generated throw plan

### Fixed physics schema

The first schema is `RAPIER_DUNGEON_D20_V1`. It fixes:

- 60 Hz timestep;
- gravity;
- d20 convex-hull geometry and scale;
- friction, restitution, damping, CCD, and sleep settings;
- collider dimensions/materials;
- held-height and release-velocity bounds;
- low-energy settlement test;
- off-table thresholds;
- maximum 480 steps (8 seconds); and
- maximum 16 wall/door checkpoints.

A change to any of those facts creates a new schema value. Clients do not silently approximate unknown schemas.

### Plan generation

At pointer release, the roller:

1. freezes the world snapshot;
2. converts the sanitized gesture plus world release pose into a rigid-body initial state;
3. creates an isolated raw Rapier world from the snapshot;
4. steps at exactly 60 Hz;
5. records each first wall/door contact transition and the body's post-step state;
6. stops at in-bounds low energy, off-table detection, or step 480;
7. maps step-480 non-settlement to truthful direct settlement rather than fabricating a terminal physics claim; and
8. disposes the isolated world.

A planned face correction is not simulated and no result enters the plan generator.

### Contact correction

A checkpoint records the complete post-contact body state:

- position;
- normalized quaternion;
- linear velocity; and
- angular velocity.

Visible playback applies that state at the checkpoint step. The collider ID is also emitted to local diagnostics and evidence. Player-facing UI need not narrate technical IDs.

### Terminal state

A terminal records kind, step, and body state. `SETTLED` requires low energy and valid floor support. `OFF_TABLE` requires the shared out-of-bounds predicate. Exhausting 480 steps is neither and falls back; it is not mislabeled settled.

## Wire design

### Package and service

Add a separate proto package:

```text
dnd5e/api/session/presentation/v1alpha1/service.proto
package dnd5e.api.session.presentation.v1alpha1
service SessionPresentationService
```

This package has its own version clock. It does not add fields or RPCs to the toolkit-mirroring `dnd5e.api.session.v1alpha1.SessionService`.

### Service shape

```proto
service SessionPresentationService {
  rpc PublishDiceThrow(PublishDiceThrowRequest)
      returns (PublishDiceThrowResponse);
  rpc StreamDiceThrows(StreamDiceThrowsRequest)
      returns (stream DiceThrowPlan);
}
```

`PublishDiceThrowRequest` carries `session`, `member`, and a `DiceThrowDraft`. The game server authenticates the player, verifies ownership and seating, and constructs the accepted `DiceThrowPlan` with `session` and `roller` bound server-side. The client cannot claim another roller.

`StreamDiceThrowsRequest` carries `session` and the subscribing member. The server applies the same ownership and seated-session gates as production session reads/streams.

### Core messages

The proto schema uses the following message structure and field numbers:

```proto
message DiceThrowPlan {
  uint32 schema_version = 1;
  string session = 2;
  string presentation_id = 3;
  uint64 authority_seq = 4;
  string roller = 5;
  uint32 attempt = 6;
  DicePhysicsSchema physics_schema = 7;
  bytes collider_fingerprint = 8;
  RigidBodyState initial = 9;
  repeated ContactCheckpoint contacts = 10;
  ThrowTerminal terminal = 11;
}

message DiceThrowDraft {
  uint32 schema_version = 1;
  string presentation_id = 2;
  uint64 authority_seq = 3;
  uint32 attempt = 4;
  DicePhysicsSchema physics_schema = 5;
  bytes collider_fingerprint = 6;
  RigidBodyState initial = 7;
  repeated ContactCheckpoint contacts = 8;
  ThrowTerminal terminal = 9;
}

message RigidBodyState {
  Vector3 position = 1;
  Quaternion rotation = 2;
  Vector3 linear_velocity = 3;
  Vector3 angular_velocity = 4;
}

message ContactCheckpoint {
  uint32 step = 1;
  DiceContactKind kind = 2;
  string collider_id = 3;
  RigidBodyState after = 4;
}

message ThrowTerminal {
  uint32 step = 1;
  DiceTerminalKind kind = 2;
  RigidBodyState state = 3;
}
```

The package owns its small `Vector3` and `Quaternion` presentation vocabulary rather than importing a game-position type. These values are inert rigid-body presentation state, not hex positions or rules coordinates.

### Explicit absences

No message carries:

- d20 result;
- hit, miss, or critical;
- attack total or target defense;
- damage or target HP;
- attack/declaration reference;
- preset ownership;
- arbitrary asset URL;
- raw pointer coordinates, IDs, samples, or timing; or
- authored prose.

`presentation_id` and `authority_seq` correlate to local authoritative truth but do not establish it.

## Game-server design

### Responsibilities

The game server owns:

- gRPC translation;
- authentication;
- member ownership and session seating;
- strict structural and numeric validation;
- validation of the requested session and server binding of the roller;
- first-valid deduplication;
- transient cross-instance delivery; and
- bounded logging/diagnostics.

It does not import Rapier, build colliders, inspect attack outcomes, select faces, calculate motion, or write toolkit Story.

### Validation bounds

The server rejects rather than clamps:

- unknown schema or physics enum values;
- missing/unsafe session, member, presentation, or collider IDs;
- attempt outside `1..32`;
- fingerprint length other than 32 bytes;
- missing, non-finite, or non-normalized body state;
- position components outside `[-4096, 4096]`;
- linear speed above 64 world units/second;
- angular speed above 128 radians/second;
- terminal step outside `1..480`;
- more than 16 contacts;
- non-increasing contact steps;
- contact at or after terminal step;
- floor/dice contact kinds in v1 milestones;
- quaternion norm error greater than `0.0001`; and
- payloads above the generated message-size bound.

`presentation_id` uses the existing `[A-Za-z0-9][A-Za-z0-9:_-]{0,127}` contract. A contact collider ID is kind-prefixed, 1–256 printable ASCII characters, and contains no whitespace or control characters. Repeated contact with the same collider is legal when its later step is strictly greater. The game server does not claim the wall exists; each receiver proves that through its matching fingerprint and local snapshot.

### Authorization

Publish requires all three:

1. authenticated player;
2. requested member is a player character controlled by that player; and
3. that member appears in the launch-written session roster.

Stream requires the same ownership and seating. The implementation reuses or extracts the current session ownership/seating logic rather than creating a weaker parallel check.

A recipient only renders plans matching locally accepted authoritative attack presentations. This final client gate is intentional: a decorative publisher cannot manufacture game fiction even when it is a valid session member.

### Redis delivery

Use Redis Pub/Sub rather than an in-process-only broker:

- channel: a safely encoded session presentation channel;
- dedupe key: `(session, presentation_id, attempt)`;
- value: deterministic accepted-plan bytes;
- dedupe TTL: two minutes;
- first `SET NX` wins;
- byte-identical retry returns the accepted plan idempotently;
- conflicting retry returns `AlreadyExists`; and
- accepted plan publishes once after the dedupe write.

The unary response returns the accepted plan so the roller can proceed if its own stream echo is delayed. Stream echoes and responses are deduplicated by full plan identity in the web.

Redis state is transient coordination, not a replay API. `StreamDiceThrows` is live-only. A server or client reconnect does not request old plans.

### Backpressure and failure

A slow or disconnected presentation subscriber never blocks publication. Redis Pub/Sub has no delivery guarantee; that is acceptable because the authoritative event and web fallback remain sufficient.

Publish failure returns an ordinary transport error. It does not alter the Attack response or toolkit state.

## Web architecture

### Production coordinator

A `useDungeonDiceCoordinator` hook lives at `SessionEncounterView`, above the sibling map and drawer. It owns:

- active authoritative dice presentation identity;
- local roller versus witness role;
- attempt number;
- drawer/world handoff state;
- immutable collider snapshot;
- hidden plan generation;
- publish/stream reconciliation;
- visible physics command;
- terminal/off-table transitions; and
- scope fencing by session, presentation, attempt, and renderer generation.

`CombatExperience` remains responsible for layout and accessible controls. `SessionCanvas` remains responsible for the dungeon scene and accepts the production presentation layer through its existing presentation-only seam.

### Shared production physics

Extract concept physics into shared, tested units under `src/components/ui/dice/`:

- dungeon dice collider snapshot/fingerprint;
- fixed physics schema constants;
- gesture-to-launch conversion;
- raw Rapier pre-simulation and plan construction;
- plan parsing/validation;
- plan playback/reconciliation;
- d20 rigid body/verified runtime mesh; and
- off-table/settlement predicates.

The concept stage imports these shared units afterward. Production never imports `src/concepts/**`.

### Lazy loading

Rapier and the production physics layer load through a dynamic boundary only when an attack d20 presentation arms or a plan arrives. The world and raw pre-simulation resources are disposed at presentation reset/unmount.

This is an architectural shipping requirement because the concept's eager import added approximately 0.85 MB gzip. It is not the deferred performance pass: no frame budget or broad optimization is declared here.

### Existing presentation reducer

The authoritative reducer remains the source of request/result/Story truth. It gains presentation-plan facts without accepting physics as authority:

- actor response can arm before the stream event;
- first accepted plan performs the same local-release transition as today;
- witness event waits for a matching plan rather than immediately inventing a neutral release;
- plan-before-event buffers for at most 3 seconds, then drops;
- event-before-plan waits up to 10 seconds, then neutral-settles;
- catch-up/hydration settles immediately and never replays stale plans;
- off-table attempts do not hide already-revealed Story; and
- conflicts still suppress singular dice UI rather than choosing physics over authority.

The game state, raw Debug, query refreshes, and action availability remain immediate exactly as today.

### Start synchronization

Clients start visible playback 150 ms after accepting a matching plan. The delay is relative to receipt, not synchronized wall time. The roller accepts either the publish response or stream echo; witnesses accept the stream plan.

This yields small network-dependent start skew while keeping contact step identity. Clock synchronization and frame-perfect starts are not required.

### Semantic and reduced-motion behavior

- Reduced motion keeps explicit Roll/release but moves directly to authoritative settlement; it may announce planned wall/off-table facts semantically without rendering travel.
- WebGL/provider failure keeps the existing concealed-before-release and truthful-after-release behavior.
- Rapier generation failure falls back to the ordinary neutral/semantic release path.
- A physically capable witness may still run a neutral throw when the roller uses semantic fallback; the result remains local authoritative truth.
- No accessibility path requires two simultaneous pointer buttons. The Roll button is the equivalent input for keyboard, switch, touch, and low-effort play.

## Failure and discontinuity behavior

| Failure | Behavior |
| --- | --- |
| Release before dungeon handoff | Return to drawer; publish nothing |
| Plan generation exceeds 480 steps | Do not label physics terminal; release through truthful direct fallback |
| Publish fails | After 750 ms, actor proceeds locally; witnesses use missing-plan fallback |
| Plan arrives before authoritative event | Buffer by session/presentation for at most 3 seconds |
| Authoritative event has no plan | Witness neutral-settles after 10 seconds |
| Duplicate equal publish | Return first accepted plan idempotently |
| Conflicting same-attempt publish | First accepted plan wins; conflict rejected/ignored |
| Unknown physics schema | Refuse physical playback; settle truthfully |
| Collider fingerprint mismatch | Refetch doors once, wait 500 ms, then fallback |
| Unknown contact collider | Treat as fingerprint/plan mismatch; never invent collider |
| Visible simulation diverges | Apply sparse post-contact and terminal states at planned steps |
| Off-table | All clients return to tray; actor may publish next attempt |
| Attempt exceeds 32 | Direct semantic settlement; no deadlock |
| Stream disconnect/reconnect | No replay; current authoritative presentation settles/falls back |
| Hydrated released history | Immediate settled projection; no stale throw |
| Provider/WebGL/Rapier failure | Existing semantic result and completion path |
| Reduced motion | Direct settlement with explicit input and semantic contact/terminal cue |
| Scope/session/member change | Dispose physics and fence every stale callback/plan |

## Security and trust boundary

The throw plan is untrusted decorative input.

The server validates identity, membership, shape, size, and numeric bounds. Receivers validate schema, immutable parse, authoritative correlation, roller identity, attempt order, and collider fingerprint. The runtime resolves only the verified default d20 preset; the plan cannot select an asset.

A malicious seated player can at worst request a bounded decorative path for their own matching presentation. They cannot:

- change any roll or rule outcome;
- create a locally accepted attack presentation without the authoritative event;
- act as another member;
- stream arbitrary asset URLs;
- force unbounded simulation or payload growth;
- write toolkit Story; or
- stall another client's combat state beyond bounded presentation fallback.

Raw gesture samples remain local and never enter the plan.

## Testing

### Proto contract

Normal Buf format, lint, breaking, and generation gates are sufficient for protobuf mechanics. Contract review additionally verifies:

- the separate package/service boundary;
- validated session and server-bound roller fields;
- bounded presentation vocabulary;
- no outcome/result/damage fields; and
- explicit live-only stream semantics.

No bespoke test re-proves generated serialization.

### Game server

Focused handler/repository/integration tests prove:

- unauthenticated, foreign-member, and unseated callers are refused before publish/subscribe;
- the requested session is validated against seating and the roller is derived from the owned member;
- every structural/numeric bound fails closed;
- equal retry is idempotent and conflicting retry loses;
- two authenticated subscribers receive byte-equal plans;
- two game-server instances sharing Redis deliver one plan across processes;
- disconnected/slow subscribers do not block publish;
- Redis/delivery failure does not call or mutate the toolkit manager;
- no Story entry or authoritative session sequence is created; and
- context cancellation closes subscriptions/resources.

### Web units and components

TDD covers:

- deterministic sorted collider snapshots and fingerprints;
- shut/locked versus open door inclusion;
- omission of props/entities/items;
- finite gesture-to-launch conversion and lift bounds;
- fixed-step plan generation and disposal;
- same input producing the same contact IDs, steps, terminal, and checkpoint states;
- wall, door, settled, off-table, timeout, and 480-step fallback scenarios;
- contact/terminal reconciliation;
- strict plan parsing and immutable snapshots;
- plan/event orderings, duplicates, attempt progression, and scope fences;
- exact tray-to-world pointer handoff;
- left+right lift and release/cancel behavior;
- Roll-button equivalence;
- actor/witness control separation;
- reduced motion and semantic fallback;
- lazy physics loading; and
- a boundary guard that production imports no concept module.

Existing d20 authority, face-map, provider, renderer, Story, combat-response/event, reconnect, and semantic fallback suites remain green.

### Two-browser production gate

Use two independent authenticated players on the real session route. Capture private/uncommitted visual evidence and public-safe diagnostics.

Required observations:

1. **Shared wall:** release toward an authored wall; both clients record the same `wall:<key>` checkpoint and visibly bounce there.
2. **Shared shut door:** release toward a shut or locked door; both record the same `door:<connection>` checkpoint and visibly bounce.
3. **Open door:** open it through the production verb and repeat; both fingerprints omit that door and neither bounces on an invisible leaf.
4. **Off-table:** throw beyond valid floor; both show `OFF_TABLE`, return to the drawer, and wait for attempt two.
5. **Valid retry:** attempt two settles; both present the same authoritative face.
6. **Missing plan:** interrupt the presentation stream; the authoritative combat presentation still completes through bounded fallback.
7. **Reconnect:** reconnect after release; no stale throw replays.
8. **Accessibility:** Roll button and reduced-motion paths complete without pointer gesture.
9. **Runtime health:** zero page errors, fatal errors, or result/Story disagreement.

The diagnostic record includes plan identity, attempt, collider fingerprint, planned contact IDs/steps, locally observed contact IDs/steps, terminal kind, authoritative result, and fallback status. It never includes raw pointer data.

### Human approval gate

Kirk performs repeated wall, door, open-door, off-table, and valid-settlement throws with two browsers. The gate asks:

- Does carrying and lift still feel physically connected?
- Do both people naturally describe the same bounce?
- Does the synchronization delay feel like one shared throw rather than lag?
- Does an off-table return feel understandable rather than punitive?
- Does late face correction remain acceptable for this production pass?

Automated equality cannot approve feel.

## Delivery and repository ownership

### `rpg-project`

Owns this design, the implementation plan, Project 19 slice hierarchy, decisions, and integrated approval record. The design PR stays open through implementation and merges last.

### `rpg-api-protos`

Owns the new presentation package, publish/stream service, strict message comments, and generated contract publication.

### `rpg-api`

Owns presentation authz, session validation, server-bound roller identity, Redis dedupe/Pub/Sub, live streaming, diagnostics, and cross-instance tests. It does not import or change toolkit dice/session rules.

### `rpg-dnd5e-web`

Owns the coordinator, transport hook, tray/world interaction, snapshot/fingerprint, hidden simulation, visible playback, reconciliation, fallback, accessibility, lazy loading, and production evidence.

### `rpg-toolkit`

No changes. The authoritative attack event remains the sole result source.

## Development and merge order

Develop outside-in against the approved consumer behavior, using local generated packages/overrides where necessary:

```text
web integration contract/proof -> proto -> game server -> final web integration
```

Use one branch per owning repository for the wave. Publication merges providers before consumers:

```text
rpg-api-protos -> rpg-api -> rpg-dnd5e-web -> rpg-project design record
```

Every implementation PR references parent journey #289 and design slice #303. Kirk walks the integrated branches before any consumer merge.

## Deferred work and escalation triggers

### Server simulation

Do not add it now. It earns a design when two-browser evidence repeatedly fails one of these despite matching plan/schema/fingerprint:

- clients hit different stable colliders;
- contact correction is visibly disruptive;
- clients disagree on off-table versus settled before terminal reconciliation; or
- future gameplay makes dice position authoritative.

The first escalation should compare server pre-simulation plus the same sparse plan against full trajectory/keyframe streaming. It should not jump directly to per-frame network physics.

### Damage and contributed dice

Remain under journey #289. They require production per-die/reroll/contributor facts and verified set-wide assets. This d20 service/plan may later grow a versioned multi-body schema; v1 does not reserve fields speculatively.

### Equipped set projection

The default verified carved d20 is fixed for this wave. Witness collectible identity still requires an authoritative equipped-set projection and safe default policy.

### Polish and performance

Tray art, hidden face steering, trajectory search, audio/haptics, render scheduling, realistic group-size performance budgets, and broad Discord/mobile profiling remain separate slices. Lazy loading and disposal are included here because the current eager concept architecture is not a valid production boundary.

## Non-goals

- Gesture influencing a roll or any rule outcome.
- Server-side physics.
- Exact contact points, spin, or pixel-identical paths.
- Persisted/replayed presentation plans.
- Putting decorative events in toolkit Story.
- Damage, critical, reroll, modifier, or contributed-die presentation.
- Equipped collectible-set ownership.
- Props, items, characters, or monsters as colliders.
- Streaming held pointer motion.
- Arbitrary model/preset selection from a plan.
- Tray art or final face-correction polish.
- A dedicated performance budget/pass.

— cross-team agent, on behalf of KirkDiggler
