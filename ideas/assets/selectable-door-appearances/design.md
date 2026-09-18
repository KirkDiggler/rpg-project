# Project467 — authored doors in the new World Builder

## Scope (corrected)

This is a **new World Builder** slice. The author chooses an existing scene
item's `assetRef` and its continuous placement; that is the appearance choice.
The goal is to make that authored door look right. Do not add a second style
system, fit to the old aperture, widen a legacy map, or route this through
`DoorSpec`/`AtlasDoorway` by default. Existing v2/legacy worlds retain their
current renderer and behavior.

The approved example is
`dnd5e:env:dark-fortress:wall_door_double_01`. The private Assets release and
Web catalog already carry it. Its reviewed roles are `Door_Frame`, `Door_Left`,
`Door_Right`, and `Door_Wall_Above`, under the normalized parent (human yaw
−180); original GLBs remain untouched. The role binding is explicit. At load
time derive rest transforms and leaf pivots from the actual GLB. The reviewed
binding declares the intended hinge axis/open angle; geometry alone cannot tell
us that intent. Do not duplicate numeric transforms in YAML or a proto.

## Current chain and ownership

* **Authoring/editor:** `rpg-dnd5e-web/src/concepts/world-building/`
  (`WorldBuildingConcept.tsx`, `roomDraft.ts`, `serialization.ts`,
  `singleRoomDungeon.ts`). A room `WorldProp` already persists `id`, `assetRef`,
  and authored `transform`; `RoomPropDeclaration` currently persists one
  explicit owner-local footprint plus independent movement/LOS flags. The UI's
  “Movement & sight declaration” is an authored declaration, not mesh bounds.
* **Save:** `useRoomPublishing.ts` emits the exact v3 YAML; `PutDungeon`'s
  `validate_only` path is the compiler proof, and `FileRegistry` stores the
  accepted bytes verbatim (`rpg-api/internal/dungeons/`,
  `internal/handlers/dnd5e/authoring/v1alpha1/`). Local storage is a draft,
  never shared gameplay state.
* **Compile:** `rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec/` decodes
  `RoomSource`, `CanonicalPlacedProps` converts declared scene items into
  canonical `PlacedPropInput`, and `CompileSingleRoom` creates the field's
  room-scene presentation plus gameplay geometry. `PlacedPropInput` is explicit
  geometry, but is immutable after construction and has no door identity/state.
* **Session/persistence:** `rpg-api/internal/sessionworld/` and the registry
  start the SDK session. `encounter.RoomScenePresentation` is copied through
  `Field.RoomScene`, `ToData`/`LoadEncounter`, `Atlas`, and `AtlasFor`; the
  canonical scene is visual presentation, while field folds own movement/sight.
* **Play/render:** `GetAtlasResponse.room_scene_json` is decoded by
  `src/components/session/roomSceneJson.ts`. At Web
  `11755d8ae0362130efc4d69bef0e1a5d95249c15`,
  `GameView → SessionEncounterView → SessionCanvas → DungeonEnvironment` sends
  a room scene to `RoomSceneEnvironment` and bypasses `DungeonShell/AtlasWalls`.
  `RoomSceneEnvironment.tsx` currently renders each item through
  `WorldPropModel` at its source pose and renders the workspace floor.

## Minimal gameplay contract

A door is an explicit binding from one stable scene-item ID to one authoritative
interactive door identity. It is not inferred from `assetRef`, labels, GLB
bounds, adjacency, or local storage. The room author must author separately:

1. the fixed frame/wall occupancy (which remains blocked in every door state); and
2. the passage obstruction (movement/LOS geometry for CLOSED/LOCKED, and an
   explicitly authored open passage for OPEN).

The current single `RoomPropDeclaration` cannot express that split: its one
static footprint can never become passable. The missing primitive is therefore
a **stateful placed obstruction bound to an item**, with explicit closed/open
geometry and a stable door ID. The encounter composition must fold that
obstruction into its existing movement, sight, persistence, and snapshot paths;
no client-side bounds test or `localStorage` toggle is a substitute. The
compiler owns the source-frame→canonical conversion once; neither API nor Web
recomputes it.

Reuse the genuine door machinery where it fits: the existing `DoorState`
(open/closed/locked), lock/concealment knowledge, `OpenDoor`/`Unlock`, member
scoping, `DoorChanged`/reveal events, and `Doors` state read. Existing
`DoorInput`/`AtlasDoorway` remains valid for a real adjacent cross-cell portal;
it is **not** the geometry contract for an arbitrary in-room placed door. The
new placed-door primitive must expose the same state/verb seam without
pretending every authored frame is a portal edge. A Close verb remains deferred;
do not add one unless implementation evidence makes it a separate decision.

The exact source/proto names for the binding are intentionally not invented
here. The consumer owning the room interaction must define the narrow shape:
scene item ID, authoritative door ID, explicit closed/open obstruction facts,
and any existing lock/concealment data. Do not put gameplay declarations into
the presentation-only `RoomScenePresentation` or add an appearance field to
legacy atlas doorways. If a separate atlas/session descriptor is required to
join item ID to member-scoped `DoorInfo`, it must be designed and transcribed
from the proven SDK contract before API/Web implementation.

## Saved, played, and observed state

World Builder edits the scene item, fixed occupancy, and passage declaration as
one room-draft history transaction. Save/reload round-trips them in v3 YAML;
`PutDungeon` validates and `FileRegistry` preserves exact bytes. Compilation
creates visual scene data and authoritative static/stateful geometry. A newly
started session snapshots both; changing the published room later cannot mutate
that running session. `ToData`/`LoadEncounter` preserves the binding, geometry,
and current door state by stable identity.

On play, `GetAtlas` carries the canonical visual scene and the authoritative
mechanical map. `GetDoors` remains the member-scoped live state and stream beats
remain the refresh path. `SessionEncounterView.handleDoorClick` continues to
call existing Open/Unlock; the canonical room branch must pass that intent and
state into `RoomSceneEnvironment`, not invent a local toggle. The server's
movement/sight answer remains authoritative even where the current atlas path
cannot preview a continuous placed footprint.

## Visual binding and motion

`RoomSceneEnvironment` must render the authored item at its exact transform and
proportions, then bind only the named door parts. Frame and wall-above stay
static. Each leaf is an isolated instance with a rigid pivot derived from its
loaded GLB rest hierarchy; open motion rotates that leaf around its own pivot.
Never apply non-uniform parent scaling to a moving assembly, infer a hinge from
bounds, or silently narrow/widen the authored opening. Keep the existing asset
catalog and failure markers: an unknown binding is a named refusal, not a
replacement model. Height/UV automation remains deferred.

## Early proof and acceptance

1. Author a room in the real World Builder: place the published double-door
   asset, set its visual pose, and author distinct fixed-frame and passage
   occupancy. Save, reload, reopen, and verify exact authored pose/asset ID.
2. Publish with `Save & Play`; start a real session, verify the saved room
   reaches `RoomSceneEnvironment` (not the legacy shell), and verify two
   independent door instances remain isolated.
3. With authoritative CLOSED and LOCKED states, leaves are closed, frame/wall
   occupancy remains, and a server movement attempt through the passage is
   refused. Open/unlock through the real handler; state events/refetch update
   every member, leaves swing rigidly, fixed occupancy remains, and passage
   movement succeeds where the server allows it.
4. Reload persisted session data and repeat state/render assertions. Test
   missing/invalid binding and missing asset with visible named failure. Use
   visual screenshots/browser evidence as the primary gate; unit tests alone or
   an old `AtlasWalls`/`SyntyHexWall` fixture do not prove this slice.

## Narrow unresolved decisions

* The room compiler/encounter owner must name the stateful placed-obstruction
  primitive and its exact open/closed geometry representation.
* The SDK consumer and proto owner must choose the smallest authoritative
  carriage joining scene-item ID to member-visible door ID/state; no parallel
  Web-only mapping or appearance field is approved.
* Assets/Web must approve the actual GLB-derived role/pivot binding and a
  visually verified open-angle convention. No manual transform registry,
  manual motion claim, map widening, Close verb, or height/UV system is
  authorized by this design.
