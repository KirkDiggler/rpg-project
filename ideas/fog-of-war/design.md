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
- Exercise a two-room scenario that proves visible, remembered, unseen,
  hidden-world-change, reconnect, and re-sight behavior.
- Define a narrow consumer contract that later platform work can implement
  without making the web calculate LOS.
- Preserve a single rendering seam for visible versus remembered content.
- Make unsafe or incomplete data fail closed, never into live truth.

### Non-goals

- No production fog-of-war implementation or production-default change.
- No toolkit, proto, API, stream, persistence, or encounter-route change in
  this concept.
- No automatic party vision, shared minimap, or out-of-band knowledge sync.
- No client-side LOS, reveal, memory derivation, or inference from world data.
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

The future web concept creates `src/concepts/fog-of-war/` with:

- A `FogOfWarConcept` route entry registered in `src/concepts/ConceptsView.tsx`.
- Authored fixtures for the two rooms, transitions, reconnect snapshot, and
  hidden-world updates.
- A reducer that accepts only authored viewer projections and applies only
  explicit transitions; it never queries map geometry or computes LOS.
- Shared render-state inputs for real floor, wall, door, trap, prop, corpse,
  monster, and player components. Components receive `visible` or `remembered`;
  unseen is represented by omission from their inputs.
- One central crypt-memory material/presentation treatment used by all
  remembered renderers.
- `CONTRACT.md`, initially documenting evidence and candidate gaps rather than
  requesting platform changes.

The concept uses real HexGrid/Synty/components to expose integration seams.
It may use fixture adapters, but it must not replace the map with a visual
showcase mockup or create a parallel game renderer.

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

## Authored scenario and data flow

The fixture is a two-room crypt. Every render input is an explicitly authored
viewer projection containing no unauthorized records. The concept may keep a
separate authored world-truth fixture or scenario panel to compare hidden
changes, but it is never passed to renderers or used by the reducer to derive
visibility or memory. This comparison is not client-side LOS.

1. **Room 1, visible.** The viewer receives Room 1's live floor, walls, a
   closed door, trap, props, corpse, monster, and player. Room 2 has no records
   at all, including its walls and door-side geometry.
2. **Door and reveal.** An authored visible projection opens the Room 1 door
   and reveals the doorway/currently visible Room 2 contents. The reducer does
   not calculate the reveal.
3. **Room 2, Room 1 remembered.** The viewer moves into Room 2. The projection
   marks Room 1's complete last-observed scene remembered and Room 2 live.
4. **Hidden change.** The separate authored world-truth fixture changes Room 1
   while the viewer projection receives no Room 1 update. The remembered Room 1
   reducer state must remain byte-for-byte equivalent in knowledge terms: same
   observed positions and states.
5. **Reconnect.** A reconnect projection restores the viewer's Room 2 visible
   state and Room 1 remembered snapshot. It does not reconstruct memory from
   current hidden world truth.
6. **Return and re-sight.** The viewer returns to Room 1 and receives current
   truth. The stale remembered Room 1 scene is atomically replaced, exposing
   the formerly hidden changes as live state.

The fixture includes a second authored viewer projection where useful to prove
that one player's revelation does not populate another player's map. It does
not add collaboration mechanics.

## Desired future consumer contract

The concept contract is intentionally small and projection-shaped. A future
snapshot supplies a viewer identity and a complete personal scene projection;
transitions supply the same kinds of records scoped to that viewer. Each
renderable record carries:

- a stable record or entity identifier;
- its kind and render payload appropriate to the real component;
- `visible` or `remembered` render state;
- a last-observed position and state for remembered records; and
- optional opaque palette metadata for future selection, with crypt charcoal as
  the required fallback.

`UNSEEN` has no record. A current `VISIBLE` record is authoritative only for
the projection that carries it. A `REMEMBERED` record is a persisted personal
snapshot, not a live entity with a cosmetic flag. The platform owns creation,
replacement, deletion, and persistence of knowledge; the web reducer only
uses supplied records and explicit state transitions.

The production implementation may choose exact proto message names and storage
layout after Kirk approves the concept evidence. It must preserve these consumer
semantics: reconnect includes remembered geometry and entities; entity and
geometry knowledge are scoped together; current visibility is delivered rather
than inferred; and re-sight replaces memory atomically.

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

Visual review is required in the real WebGL concept on desktop and mobile. It
must inspect the complete scenario: Room 1 live, the door reveal, Room 1 crypt
memory from Room 2, hidden mutation isolation, reconnect restoration, and
current-truth replacement on return.

## Acceptance criteria

- `/concepts` exposes **Fog of War** and the two-room scenario runs with real
  game rendering components.
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

Future Intelligence/senses may vary retention or fidelity, and future themes
may select a different remembered palette such as forest sepia. Neither changes
the three knowledge states or grants the web authority to derive them; neither
is in the v1 implementation scope.
