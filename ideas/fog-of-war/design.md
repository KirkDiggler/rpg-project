# Fog of War: viewer-scoped knowledge

## Status and scope

Approved concept design for rpg-project#130. The first delivery is one
fixture-driven web concept under `/concepts` -> **Fog of War**. It uses real
HexGrid, Synty assets, and game components, but it is not production fog of
war and does not change the toolkit, protos, API, or production encounter
route.

The concept is an executable outside-in consumer-contract experiment. Its
fixtures state the data the web needs; they do not pretend that the current
platform supplies that data. `src/concepts/fog-of-war/CONTRACT.md` records
survey evidence and candidate platform gaps after the concept is reviewed.
Kirk reviews that record before any candidate becomes a cross-repo request.

### Revision: the concept is playable, and the event layer is the deliverable

The original concept stepped a viewer through an authored six-step sequence.
That is not enough to design a wire contract. A scripted list only proves the
cases someone thought to script, and it never produces the transitions a
player generates by wandering — the ones that expose whether a message shape
actually holds up.

This revision makes the concept **playable on a single viewer**. The player
moves freely through an authored map and watches fog open and close. Nothing
is smoke and mirrors: the concept defines a real event layer, and the rendered
scene is built only from events that layer emits. What the concept proves by
being played is what the protos then encode.

Single viewer is deliberate, not a simplification to be undone later.
Per-viewer routing already exists end to end — `Broker.Subscribe(encID,
playerID)` delivers only events whose `Audience` contains that player, and
`TranslateEvent(evt, viewer, now)` already translates per viewer. A contract
that serves one viewer correctly serves all of them once the toolkit
populates each viewer's slice.

## Context

This is a multiplayer D&D Discord Activity. Visibility and visibility events
belong to one viewer, not the party. Players do not automatically share what
they can see: communicating truthfully, incompletely, or deceptively is
gameplay.

The web renders server-authored knowledge and sends intent. It must not
calculate line of sight, visibility, or memory transitions. The correct future
production boundary is:

1. The toolkit owns visibility and knowledge rules, maintains the per-viewer
   `View`/encounter-data shape, and serializes it as encounter data.
2. The API durably stores that encounter data and projects the viewer's
   knowledge.
3. Protos carry snapshot and transition data.
4. The web renders the received state only.

## Goals and non-goals

### Goals

- Establish the player experience for individual, durable dungeon knowledge.
- Let one viewer move freely through an authored map and prove visible,
  remembered, unseen, hidden-world-change, reconnect, and re-sight behavior by
  being played, not by stepping a script.
- Define the event layer — message shape, payload, and transition semantics —
  concretely enough that the proto work is transcription rather than
  invention, and without making the web calculate LOS.
- Preserve a single rendering seam for visible versus remembered content.
- Make unsafe or incomplete data fail closed, never into live truth.

### Non-goals

- No production fog-of-war implementation or production-default change.
- No toolkit, proto, API, stream, persistence, or encounter-route change in
  this concept.
- No automatic party vision, shared minimap, or out-of-band knowledge sync.
- No client-side LOS, reveal, memory derivation, or inference from world data.
  The concept's authority does compute line of sight, because something must
  stand in for the server — but it sits on the far side of the event boundary
  and is enforced as such by test. No consumer-side module may compute or
  import it. "The client" means the consumer half.
- No Intelligence, sense, retention, fidelity, or theme system. The contract
  leaves an opaque optional presentation selector only; v1 uses crypt charcoal.
- No attempt to turn remembered things into interactable gameplay objects.

## Experience model

Every renderable scene record is in exactly one personal knowledge state:

| State | Meaning | Rendering and behavior |
| --- | --- | --- |
| `UNSEEN` | The viewer has never observed it. | It is absent. Render no floor, walls, doors, traps, props, corpses, monsters, players, labels, or substitute silhouette. |
| `VISIBLE` | The viewer currently observes it. | Render current server truth normally. Live content may animate, label, select, target, and accept interactions according to existing gameplay rules. |
| `REMEMBERED` | The viewer observed it earlier but does not currently observe it. | Render the entire last-observed scene snapshot, frozen and inert. It persists across dungeon movement and reconnect. |

Memory is personal. A remembered record contains what that viewer last saw:
floor, walls, doors, traps, props, corpses, monsters, and players, with each
object's last-observed position and state. A hidden change never mutates it.
When the viewer re-sights content, the incoming current truth atomically
replaces that remembered record. The replacement is not a merge and does not
preserve stale fields.

## Visual treatment

Remembered content uses the **crypt** treatment centrally, not separate
per-component approximations:

- Frozen: no animation, idle loop, movement interpolation, or hover response.
- Desaturated charcoal at low contrast, with subtle cool separation so floor,
  structure, and recognizable objects remain legible.
- No labels, HP, selection, targeting, interaction affordance, or action
  metadata is exposed.
- A remembered closed door, trap, corpse, monster, or player remains the last
  observed door, trap, corpse, monster, or player; it is not anonymized.

Visible content retains the normal production visual treatment. Unseen content
has no rendering path. If an asset is unavailable, its fallback preserves the
state distinction: live fallback remains live, remembered fallback stays crypt
charcoal and inert, and unseen remains absent. An unsupported future palette selector
falls back to crypt charcoal and cannot alter knowledge or interaction state.

## Concept architecture

The concept creates `src/concepts/fog-of-war/` in two halves separated by a
hard boundary: an **authority** standing in for the server, and a **consumer**
standing in for the game client. Events are the only thing that crosses.

The authority holds the authored world truth and a deliberately crude line of
sight. It is the only code in the concept permitted to read world truth. Given
a move intent it reconciles what the viewer can now see and emits the events
defined below. It stands in for work the toolkit and API will own; it is not a
second game engine. It exists to produce the event stream, and it must not
grow rules beyond what producing that stream requires.

The consumer is the path production keeps:

- A reducer holding one viewer's knowledge keyed by hex, applying events. It
  never reads world truth, never computes LOS, and takes no input but events.
- An adapter turning reducer state into `HexGrid` props.
- Real `HexGrid`/Synty/game components — the renderer seam merged in
  rpg-dnd5e-web#602. Components receive `visible` or `remembered`; unseen is
  represented by omission from their inputs.
- One central crypt-memory material/presentation treatment used by all
  remembered renderers.

Alongside both: a `FogOfWarConcept` route entry registered in
`src/concepts/ConceptsView.tsx`, and `CONTRACT.md`, initially documenting
evidence and candidate gaps rather than requesting platform changes.

The boundary is the point of the exercise. World truth lives on one side,
events cross, and the consumer can only ever know what a record told it. A
test asserts that no consumer-side module imports from the authority half, so
client-side LOS cannot arrive later as a convenience.

The concept uses real HexGrid/Synty/components to expose integration seams.
It may use fixture adapters, but it must not replace the map with a visual
showcase mockup or create a parallel game renderer.

### Movement is parked

The authority moves things between hexes; it does not attempt convincing
motion. Smooth movement is a large problem with its own owners, and reaching
for it here would swallow the concept. `EntityMoved` already carries
`actual_path` for whoever takes that on later. The fog contract states where
things are, discretely, and the record shape below is checked only for not
foreclosing interpolation — not for enabling it.

### Concepts README requirement

This concept implementation also adds `src/concepts/README.md`. It applies to
all concepts, not only fog of war, and states that concepts:

- use real game components with representative fixtures;
- are executable outside-in contract experiments, not visual showcase mocks;
- author fixtures as desired consumer data;
- use `CONTRACT.md` for evidence and candidate platform gaps;
- require Kirk's review before gaps become cross-repo requests; and
- let approved concepts drive toolkit, proto, API, and production-web work from
  a known consumer contract.

## Authored world and play loop

The authored world is a two-room crypt held by the authority. The consumer
never receives it. A scenario panel may display world truth beside the
rendered view so a reviewer can see what the player is *not* being told;
that panel reads from the authority and never feeds the reducer, and the
comparison is a review aid, not client-side LOS.

The play loop is: the player clicks a hex, the authority reconciles
visibility, events are emitted, the reducer applies them, the scene
re-renders. Every case below is reachable by playing rather than by
advancing a script.

1. **Standing in Room 1.** Records for what the viewer can see arrive
   `VISIBLE`. Room 2 has no records at all — no floor, no walls, no
   door-side geometry.
2. **Opening the door.** New hex records arrive for what opening it exposes.
   The reducer calculates no reveal.
3. **Walking into Room 2.** Room 1's records arrive `REMEMBERED` as the
   viewer loses sight of them; Room 2's arrive `VISIBLE`.
4. **Hidden change.** The authority mutates Room 1 while the viewer is away
   and emits nothing to that viewer. Remembered Room 1 must be unchanged in
   knowledge terms: same observed positions and states.
5. **Reconnect.** A fresh subscription restores Room 2 visible and Room 1
   remembered. Memory is not reconstructed from current world truth.
6. **Walking back.** Room 1 records arrive `VISIBLE` with current truth and
   atomically replace the stale remembered scene, exposing what changed.

Two cases the play loop must also reach, because they are what makes the
record shape earn itself:

7. **A monster crossing the viewer's sight.** It walks a path partly inside
   the viewer's vision. While visible, its records move hex to hex. When it
   steps out of sight, the last hex the viewer saw it on freezes
   `REMEMBERED` with the monster still placed on it, and the hexes it
   continues through send nothing. The frozen monster is what the viewer
   believes until contradicted.
8. **Walking up to the frozen monster.** That hex arrives `VISIBLE` with
   `contents: []`. The remembered monster disappears — not because a
   "forget" message arrived, but because the current record is total and
   says the hex is empty.

Case 8 is the load-bearing one. It is the reason a visible record must state
its full contents rather than only its additions.

## The event layer

The hex is the unit of truth. One event carries one viewer's slice:

```
HexKnowledgeChanged {              // one viewer's slice
  hexes:    [HexRecord]
  entities: [Entity]               // authorized disclosure, this viewer only
}

HexRecord {
  position: Position
  state:    VISIBLE | REMEMBERED
  terrain:  TerrainType
  zone_id:  string
  edges:    [Wall]                 // existing Wall shape: from, to, kind, id
  contents: [Placement]            // [] means provably empty
}

Placement { entity_id, facing }
```

Two collections, delivered together so they cannot disagree. Hexes say
**where**; entities say **what**.

### The record is an observation

A `HexRecord` is what one viewer saw at one moment. An `Entity` is what the
thing currently is, as disclosed to that viewer. Memory freezes records, not
entities.

That rule decides where a field belongs. Facing is the worked example: a
viewer sees a goblin facing north and loses sight of it; the goblin later
turns south and another viewer sees that. The first viewer's memory must
still say north. If facing lived on the entity, the second viewer's
observation would silently rewrite the first viewer's memory — one object,
two memories that must stay independent. So facing is carried on the
placement, inside the record, where it freezes with everything else the
viewer observed.

Anything that must stay frozen in memory belongs on the record. Anything that
should reflect current disclosure belongs on the entity.

### Why the hex, and why hexes rather than "geometry"

Walls and doors are already edges between two hexes on the wire —
`Wall{from, to, kind, id}`, with doors as a wall *kind* rather than a
separate list. An abstract geometry record would flatten that back into a
bag of walls the client must re-associate with hexes. A hex-keyed record is
self-describing at exactly the seam the client consumes, and the renderer
already agrees: `HexGrid` remembers walls by hex key, not by wall id.

`Geometry` remains the right parent concept in the toolkit, with hex as one
layout family and other layouts possible later. At this seam the concept
speaks hex.

### Why a visible record is total

A `VISIBLE` record states everything on its hex, including `contents: []`.
Empty is a positive fact, not missing data. If a record could omit contents,
the client would have to decide whether an absent monster means "gone" or
"not included in this message" — and every way to decide that is either
inference or dependence on a second message that may not arrive. Making the
visible record total removes the question.

This is why re-sight needs no merge rule: an arriving `VISIBLE` record
replaces the remembered one wholesale. The door someone opened behind the
viewer's back, the monster that left, the trap that was not there before —
all corrected by one message, because appear is the new truth.

### Nothing is ever deleted

There is no removal transition and no tombstone, because a total record
leaves nothing for one to do. Ask what a removal would remove:

- A monster that left while the viewer watched — the hex arrives `VISIBLE`
  with `contents: []`. Handled.
- A wall demolished while the viewer watched — the record carries `edges` and
  replaces wholesale. Handled.
- A monster deleted from the world while the viewer was away — memory keeps
  it. That is the required behavior, not a defect needing a tombstone.
- The hex itself ceasing to exist — not a thing.

So a record's state is `VISIBLE` or `REMEMBERED`, and knowledge only ever
grows or gets replaced. Deletion is the one operation a later observation
cannot correct, which is reason enough not to have it when nothing needs it.
Unreferenced entity records may linger in a viewer's disclosed set; they are
vocabulary for memories, harmless, and finite.

### Remembered records carry their observation

A `REMEMBERED` record carries the full frozen observation rather than
instructing the client to freeze what it holds. The server retains each
viewer's last observation anyway — reconnect requires it — so sending it
costs nothing and removes a client behavior.

The payoff is that live transitions and reconnect hydration become the same
code path instead of two. The client has exactly one behavior: merge records
by hex key. It never freezes, never deletes, and never decides anything.

### Why entities are a separate collection

A placement carries `entity_id` and `facing`. The entity it references
carries what the server is willing to disclose to *this* viewer, and only
that. Two consequences fall out:

- Perception is the same mechanism, not a special case. A viewer who passes
  the check gets a trap placement on that hex plus a trap entity to resolve
  it against; a viewer who fails gets `contents: []` and no trap entity at
  all. Neither viewer's client contains logic about traps.
- Disclosure is a server decision per field. HP is simply a field the server
  may decline to populate — not a policy the client is trusted to enforce.

Fail-closed becomes one rule: a placement whose `entity_id` is not in the
viewer's known entity set is dropped.

### What this replaces

Backward compatibility buys nothing here — none of this is playable yet, and
one way through beats a right way plus a fallback. So:

- `GeometryRevealed` is replaced. Its additive sticky hex/wall payload cannot
  express a visible/remembered partition, which is the whole feature.
- `EntityAppeared` and `EntityDisappeared` are replaced. A hex record already
  states presence and absence.
- `EntityDisappeared.last_known_position` is retired. It exists today
  specifically so a client can "freeze marker at last-seen hex without
  client-side game-state tracking" — the hex record *is* the last-seen
  position.
- `DoorOpened`'s `revealed_hexes` / `revealed_walls` / `removed_walls`
  payload is retired. A door opening is new hex records arriving. That leaves
  `DoorOpened` a pure notification for sound and narration, which is what it
  should have been.

- The viewer-authorized removal transitions and tombstones named in
  `production-design.md` are never built. A total record leaves them nothing
  to do — see "Nothing is ever deleted" above. This supersedes one of that
  document's stated fixed semantics, not merely its event names.

`EntityMoved` stays: it describes how something travelled, which is
presentation, not knowledge.

### Required behavior

| Situation | Viewer receives |
| --- | --- |
| First sight | Records `VISIBLE` with full truth. No separate reveal step. |
| Loss of sight | Records `REMEMBERED` carrying the frozen observation. |
| Re-sight | Records `VISIBLE` with current truth, replacing memory wholesale. |
| Hidden mutation | Nothing. Stale memory persists by construction. |
| Movement in view | Source hex `contents: []`, destination hex holding the entity. |
| Witnessed removal | A `VISIBLE` record without the removed thing. No tombstone. |
| Hidden removal | Nothing. Memory stays stale until an authorized observation. |
| Reconnect | The viewer's complete known set: `VISIBLE` and `REMEMBERED` records together, same code path as any other event. |

`UNSEEN` has no record. A `VISIBLE` record is authoritative only for the slice
carrying it. A `REMEMBERED` record is a personal snapshot, not a live record
with a cosmetic flag. Duplicate records are idempotent — applying the same
record twice is a merge by hex key onto the same value. The platform owns
creation, replacement, deletion, and persistence of knowledge; the reducer
only applies supplied records.

Exact proto field numbering and message naming follow repository convention
and are settled when the protos are written. The semantics above are what the
concept proves and what the protos must preserve.

## Verified current contract gaps

The concept does not fix these. They are verified reasons it must remain
fixture-driven until reviewed:

1. `rpg-api-protos/dnd5e/api/v1alpha2/encounter/events.proto:109-114` defines
   `GeometryRevealed` as additive sticky hex/wall data; it has no current-visible
   geometry projection or memory replacement semantics.
2. `rpg-api/internal/handlers/dnd5e/v2/encounter/project.go:133-139` computes
   `visibleNow`, but the returned `Encounter` at `:208-218` does not expose it
   as geometry state. `wallsToProto` at `:260-289` and `doorWallsToProto` at
   `:292-310` intentionally project all walls and doors regardless of
   `RevealedHexes` or `visibleNow`, leaking unobserved geometry on snapshots.
3. `rpg-api-protos/dnd5e/api/v1alpha2/encounter/events.proto:121-130` and
   `rpg-api/internal/handlers/dnd5e/v2/encounter/translate.go:486-505` support
   a per-viewer `last_known_position` on `EntityDisappeared`. The current web
   reducer turns this into a retained ghost at
   `rpg-dnd5e-web/src/hooks/useEncounterState.ts:761-785`, and the stream
   handler applies it at `src/components/game/EncounterView.tsx:402-408`.
   That is entity-only ghosting, not a complete inert remembered scene.
4. Reconnect snapshots currently include only currently visible players and
   monsters (`rpg-api/internal/handlers/dnd5e/v2/encounter/project.go:141-187`).
   They cannot restore remembered entities. Static obstacles are sticky only
   when their hex was revealed (`:189-205`), still without remembered state.
5. `rpg-toolkit/encounter/perception/view.go:5-22` defines the per-player
   `RevealedHexes` encounter-data shape and reserves `KnownEntities`, but the
   latter is explicitly unused shape stability. `Encounter.ToData` serializes
   that shape for its caller to save (`rpg-toolkit/encounter/encounter.go:566-574`);
   rpg-api durably persists the returned encounter data. The current seam
   therefore does not retain a complete knowledge snapshot.
6. `rpg-api-protos/dnd5e/api/v1alpha2/encounter/types.proto:184-195` runs two
   different visibility models side by side, and says so: `Space` documents
   that "hexes and walls are sticky (explored geometry persists per character
   across the campaign)" while "entities are real-time LOS (server filters per
   player per event)". Sticky geometry is drawn as though currently observed,
   so the wire has no state between explored and gone. That gap — not a
   rendering shortfall — is why fog cannot be expressed today.
7. `rpg-toolkit/encounter/events/hex_revealed.go:23-32` already carries a
   per-viewer `PerPlayer map[PlayerID]HexRevealedSlice` whose `Audience()` is
   derived from its keys, and `HexRevealedSlice` reserves an unused
   `Entities []EntityVisibility` field "for shape stability so future slices
   can add entity-visibility accumulation without a JSON migration". The
   per-viewer split this design needs is the established pattern, and room
   for entity knowledge was deliberately left open.

## Fail-closed behavior

- Unknown, unseen, or absent records are omitted. No default floor, wall,
  entity, live state, or inferred visibility may be rendered.
- A malformed remembered record is discarded or rendered only as an inert,
  non-live fallback; it never becomes a visible record through coercion.
- Remembered entries stay inert even if an incoming or stale payload contains
  action metadata. Interaction, targeting, selection, hover labels, and HP are
  suppressed before component dispatch.
- Hidden updates that lack an explicit current-visible/re-sight transition do
  not modify remembered state.
- Unsupported palette values use crypt charcoal without changing knowledge,
  state, or access to interactions.
- Missing assets use state-preserving fallbacks; they cannot collapse
  remembered and visible rendering into the same affordance.

## Verification

The concept implementation must include reducer and fixture tests for:

- no geometry or entity leak before observation, including walls, doors, traps,
  floors, props, corpses, monsters, and players;
- unseen-mutation isolation while the viewer is away;
- atomic re-sight refresh from remembered state to current truth;
- reconnect restoration of remembered and visible knowledge;
- remembered interaction, targeting, selection, labels, HP, hover, and
  animation suppression;
- unknown/malformed records and unsupported palettes failing closed;
- state-preserving asset fallback; and
- `ConceptsView` navigation wiring for Fog of War.

The event layer adds these, which the older scripted concept could not reach:

- **The boundary holds.** No consumer-side module imports from the authority
  half. This is the test that keeps client-side LOS out permanently.
- **A visible record is total.** A hex holding a remembered monster, on
  receiving a `VISIBLE` record with `contents: []`, renders empty — case 8.
- **Placements fail closed.** A placement referencing an entity absent from
  the viewer's entity set is dropped, not rendered as an unknown shape.
- **Records are idempotent.** Applying the same record twice leaves reducer
  state identical.
- **The reducer has no other input.** Its state is a pure function of the
  events applied, asserted by replaying a recorded session against a fresh
  reducer and comparing state.

Visual review is required in the real WebGL concept on desktop and mobile. It
must inspect the complete scenario by playing it: Room 1 live, the door
reveal, Room 1 crypt memory from Room 2, hidden mutation isolation, reconnect
restoration, current-truth replacement on return, and the monster that walks
out of sight, freezes, and vanishes on approach.

## Acceptance criteria

- `/concepts` exposes **Fog of War**, and a single viewer can be walked freely
  around the authored map with real game rendering components, watching fog
  open and close.
- The rendered scene is built only from emitted events. Nothing reaches the
  renderer by a path that bypasses the event layer.
- No unseen fixture content reaches component inputs or the rendered scene.
- Remembered Room 1 remains a recognizable but crypt-treated, inert last view
  through movement and reconnect.
- Hidden Room 1 changes remain invisible to the viewer until the authored
  re-sight projection arrives, then replace the remembered scene atomically.
- The implementation contains the concepts-wide README and a fog-of-war
  `CONTRACT.md` that records evidence/candidates without filing platform asks.
- Production defaults remain unchanged.
- Reducer/fixture tests and desktop/mobile WebGL review pass the verification
  cases above.

## Production follow-ups

After concept review, separate issue-first work may implement the approved
consumer contract in dependency order:

1. Toolkit: own the per-player current-visibility and scene/entity-knowledge
   rules; maintain and serialize the `View`/encounter-data shape; enforce the
   re-sight and unseen-isolation rules.
2. Protos: add snapshot and transition shapes for viewer-scoped current and
   remembered geometry/entities without overloading additive reveal events.
3. API: durably store the toolkit's encounter data; project only
   viewer-authorized current and remembered records, remove whole-room
   wall/door leakage, and restore knowledge on reconnect.
4. Production web: consume that contract on the encounter route using the
   proven concept rendering seam.

### Ordering, and what this revision supersedes

That numbering is dependency order for *landing* the work, not the order in
which the contract gets decided. The contract is decided outside-in: the
concept proves the event layer, the protos transcribe it, the API translates
and projects it, and the toolkit supplies the authority behind it. Working
this way means corrections happen while they are still cheap — a shape that
turns out wrong costs a fixture edit rather than four merged PRs.

The pipeline the production work plugs into already exists and does not need
inventing: encounters publish through `Broker`, stream handlers subscribe
per player, and `TranslateEvent(evt, viewer, now)` converts domain events to
protos for one viewer. Fog is new event types flowing through it.

This revision supersedes two things in `production-design.md`:

- Its `GeometryAppeared` / `GeometryDisappeared` naming and its four-event
  split of reveal, appear, disappear, and removal. The concept collapses these
  into `HexKnowledgeChanged` with a per-record state, and speaks hex rather
  than geometry at this seam.
- Its treatment of `GeometryRevealed` as retained and additive. It is
  replaced, not preserved; backward compatibility is not a constraint while
  nothing is playable.

The Wave A issues filed under rpg-project#147 (rpg-toolkit#850, #851,
rpg-api-protos#197, rpg-api#724, #725, rpg-dnd5e-web#609, rpg-project#148)
were written against the superseded four-event split and need updating before
they are worked. Their goals, viewer-scoping requirements, and no-client-LOS
constraints all survive; the event set named in them does not.

Future Intelligence/senses may vary retention or fidelity, and future themes
may select a different remembered palette such as forest sepia. Neither changes
the three knowledge states or grants the web authority to derive them; neither
is in the v1 implementation scope.
