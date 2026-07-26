# Fog of War Production-Promotion Design

## Status, purpose, and scope

This is the approved high-level production design for the Fog of War concept
promoted under project #130. It fixes production semantics, ownership, delivery
waves, and acceptance gates. Exact proto field layout and per-repository task
details belong in the later implementation plan. This is deliberate scope, not
an unresolved decision.

The concept remains historical executable evidence. It is an outside-in
consumer contract, not a rules simulator or visual mock: fixtures state the
exact snapshots and transitions the game client wants to receive, and pass
through the same reducer, adapter, and renderer seam that production will use.
At promotion, the fixture source is replaced by the encounter stream; the
consumer path stays the same. Proven fixture needs drive proto and RPC
contracts; the API then projects and delivers those contracts; the toolkit
supplies and persists the authoritative information.

This is a process note for future concepts and production promotion. It does
not request a rebuild of the current concept.

This document supersedes the concept plan's production-target treatment of the
pale-cyan entity ghost path. The concept plan's default-compatible `isGhost`
behavior remains valid for its original non-production concept slice while
production wiring is incomplete.

> **Partly superseded — read `design.md` §"The event layer" first.**
>
> The event set named throughout this document — `GeometryAppeared`,
> `GeometryDisappeared`, additive `GeometryRevealed`, and separate removal
> transitions — has been replaced by a single per-viewer `HexKnowledgeChanged`
> carrying hex records with a `VISIBLE | REMEMBERED | GONE` state, hex-attached
> edges, and total contents. Reveal, appear, disappear, and removal are states
> of one record rather than four messages, and the seam speaks hex rather than
> geometry, because walls and doors are already hex edges on the wire.
>
> Everything else here still holds: the three knowledge states, viewer scoping,
> hidden mutations leaving stale memory, witnessed removals updating it
> immediately, no client-side LOS or world-truth lookup, the wall/door
> projection leak, the event-before-persist ordering hazard, the wave split,
> and the verification gates. What changed is the shape of the messages, not
> the semantics they must carry.
>
> Backward compatibility with `GeometryRevealed` is no longer a requirement.
> Nothing here is playable yet, and one way through is preferred to a correct
> path plus a fallback.

## Production Semantics

Production uses three personal knowledge states: unseen, currently visible,
and remembered. Unseen is omission. Currently visible records carry current
authorized truth. Remembered records are frozen last-authorized observations;
they are not live records with a cosmetic flag.

Remembered rendering replaces the user-facing pale-cyan ghost path in
production. These are not parallel user-facing states: an entity or geometry
record is either currently visible, remembered, or omitted. Existing renderer
default compatibility stays intact until the production stream is wired.

`GeometryRevealed` retains its strictly additive meaning: first discovery and
personal knowledge. It does not set or imply current visibility.
`EntityAppeared` and `EntityDisappeared` retain their meanings for current LOS
entry and loss.

Production adds distinct geometry-current-visibility events:

- `GeometryAppeared`: known geometry became currently visible or was
  re-sighted. It carries current, viewer-authorized geometry records.
- `GeometryDisappeared`: known geometry left current visibility. It becomes
  remembered; it is not deleted.

The geometry events must support freezing and atomically refreshing complete
authorized floors, walls, and doors without a client LOS calculation or any
client world-truth lookup. `GeometryRevealed` must not be overloaded to do
this work.

Viewer-authorized entity and geometry removals require distinct removal
transitions or tombstones. Their exact proto names and fields are
implementation-plan scope. `EntityDisappeared` and `GeometryDisappeared`
always mean visible to remembered and never delete knowledge. An authorized
removal deletes or replaces the affected remembered record immediately; a
hidden removal emits no removal transition to that viewer and preserves stale
memory. Existing globally broadcast `EntityRemoved` semantics must be
reconciled with this viewer-scoped contract rather than blindly reused, because
they could leak a hidden deletion.

A future visibility modifier, such as Darkness, calls generic visibility
reconciliation. That reconciliation naturally emits applicable entity and
geometry transitions. Implementing Darkness itself is out of scope.

Hidden changes and removals leave stale memory until a re-sight or another
authorized observation. Witnessed removals update memory immediately. Unseen
records are omitted.

## Current Evidence And Gaps

Existing entity appeared/disappeared behavior covers several movement paths and
per-viewer last-known positions. Toolkit coverage is incomplete for viewer
movement discovering stationary players, door-open entity scans, some
spawn/add paths, and generic visibility-modifier hooks.

Geometry revelation is sticky, and no geometry appeared/disappeared path
exists. The API computes current `visibleNow` internally but does not deliver
geometry-loss or current-geometry classification. Live `GeometryRevealed`
carries hexes but not walls. Snapshot projection leaks whole-room walls and
doors. Remembered dynamic entities are not persisted or restored on reconnect.

Existing tracking is:

- Toolkit #761.
- API #647 and #648, which are related work to reuse rather than duplicate.
- Web #604, #605, and #606.
- Project #130.

## Delivery Approach

One cross-repository production-promotion initiative delivers two sequential
waves. Work is issue-first and follows cross-repository dependency order. This
document creates no issues.

### Wave A: Live Knowledge

The toolkit owns a central visibility reconciler. After every
visibility-affecting mutation, it compares before and after visibility per
viewer and emits transitions. Wave A completes current action coverage for
target movement, viewer movement, pass-through movement, doors, and spawn/add
paths, with an extensible hook for future visibility effects.

The toolkit reuses `EntityAppeared` and `EntityDisappeared` and adds geometry
appeared/disappeared domain events, plus viewer-authorized entity and geometry
removal transitions. It persists complete per-viewer last-observed geometry
records, including floor, wall, and door state. Existing sticky `RevealedHexes`
is the seed for this knowledge but is insufficient by itself. Protos add
geometry-visibility and removal contracts without overloading
`GeometryRevealed`. Their exact field encoding is an implementation-plan
decision, but they must carry enough complete, viewer-authorized geometry
records to freeze and atomically refresh floors, walls, and doors without
client LOS or world-truth lookup.

The API translates those per-viewer events and authorized removal tombstones,
removes unconditional wall and door leakage, and prevents event-before-persist
enrichment races. Events must therefore be self-contained or emitted only after
persistence makes their authoritative data available.

The web hydrates or adds `GeometryRevealed` records as known baseline geometry,
remembered until an authoritative current transition says otherwise.
`GeometryAppeared` alone promotes or refreshes geometry to current `visible`.
Entity and geometry disappearance map to `knowledgeState='remembered'`;
authorized removal transitions delete or replace the affected remembered
record. Entity appearance maps to current `visible`. The web retires pale-cyan
ghost presentation on the production route and consumes the contract through
the existing renderer seam, without client inference.

Wave A intentionally does not complete Fog of War. Dynamic entity memory is
not yet durable: on reconnect, hidden remembered dynamic entities may reset.
Geometry knowledge is durable in Wave A: persisted complete per-viewer
last-observed floor, wall, and door records hydrate as remembered, then
authoritative current `GeometryAppeared` replay promotes only records currently
visible. The client must never infer that all known geometry is visible or look
up current world data to fill the durable records.

### Wave B: Durable Knowledge

The toolkit populates and persists complete per-viewer known dynamic entity
snapshots. Protos add a complete viewer-scoped visible/remembered full-scene
snapshot projection. The API stores, projects, and restores that unified
snapshot on reconnect. The web atomically hydrates it, eliminating the Wave A
dynamic-memory reset.

## Contract And Data Flow

The authoritative path is toolkit visibility and knowledge rules, toolkit
encounter data persistence, API projection and delivery, proto/RPC transition
or snapshot contracts, then the existing web reducer/adapter/renderer seam.
The web renders supplied viewer knowledge only.

Required event behavior is:

| Situation | Required viewer result |
| --- | --- |
| First reveal | `GeometryRevealed` establishes known baseline geometry only, initially remembered; `GeometryAppeared` is also received and alone establishes currently visible authorized geometry. Visible entities arrive through `EntityAppeared`. |
| Loss of sight | `GeometryDisappeared` makes known geometry remembered; `EntityDisappeared` makes the entity's last authorized observation remembered. |
| Re-sight | `GeometryAppeared` atomically replaces remembered geometry with current authorized records; `EntityAppeared` updates entities to visible current truth. |
| Hidden mutation | No transition for that hidden record; stale remembered state remains unchanged. |
| Witnessed removal | A viewer-authorized entity or geometry removal transition/tombstone is delivered immediately and deletes or replaces the remembered record. |
| Hidden removal | No removal transition is delivered to that viewer; stale memory persists until a later authorized observation establishes the removal. |
| Reconnect in Wave A | Persisted complete last-observed floor, wall, and door records hydrate remembered; current `GeometryAppeared` replay promotes only records currently visible. Hidden remembered dynamic entities may be absent. |
| Reconnect in Wave B | Complete viewer-scoped visible/remembered snapshot atomically restores entity and geometry knowledge. |

Unknown or malformed records fail closed by omission. Duplicate transitions are
idempotent. Out-of-order or missing transitions recover through an
authoritative snapshot; the client never reconstructs LOS or world truth.

Renderer seams may merge while preserving default compatibility. The production
switch happens only after every Wave A layer is ready.

## Issue Package

The initiative has a parent production-promotion tracking issue under or linked
to project #130.

Wave A child issues, in dependency order:

1. Toolkit reconciliation and event completeness.
2. Proto geometry-visibility contracts.
3. API projection, translation, wall filtering, and race-safe integration.
4. Web production ghost retirement and event consumption.
5. Cross-repository Wave A acceptance.

Wave B child issues, in dependency order:

1. Toolkit durable knowledge.
2. Proto snapshot projection.
3. API persistence and reconnect projection.
4. Web atomic hydration.
5. Final cross-repository acceptance.

API #647 and #648 are evaluated and reused as related work within this package,
not recreated as duplicate work.

## Verification Gates

Wave A requires all of the following:

- Toolkit multi-viewer transition matrices for target movement, viewer
  movement, pass-through, doors, spawn/add, and the generic sight-change hook.
- Toolkit and API coverage for viewer-authorized entity and geometry removals,
  including hidden removal preserving stale memory.
- Proto compatibility checks.
- API multi-viewer integration coverage proving no hidden wall or door leakage.
- Web coverage proving fixture-event and real-stream paths produce identical
  renderer inputs.
- Production-route browser acceptance repeating the two-room concept sequence.

Wave B additionally requires reconnect refresh tests that prove atomic
restoration of complete visible and remembered viewer knowledge.

No wave is accepted merely because a renderer can display remembered content.
Wave A acceptance does not claim overall Fog of War completion.

## Non-Goals

- The actual Darkness spell.
- Intelligence or senses.
- Party or shared vision.
- Memory decay or fidelity policies.
- Alternate palettes.
- Collaboration mechanics.
- Client-side LOS or client reconstruction of world truth.
