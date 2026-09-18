# Project467 — authored doors in the new World Builder

## Scope and decisions

This is a **new World Builder** slice. A scene item's existing `assetRef` and
continuous authored transform are the appearance choice. The goal is to make
that authored door look right. There is no second appearance/style field, no
legacy aperture fit, no silent map widening, and no `DoorSpec`/`AtlasDoorway`
route by default. Existing v2/legacy worlds retain their current renderer and
behavior.

The approved asset is
`dnd5e:env:dark-fortress:wall_door_double_01`. Its private release and Web
catalog already carry it. Its reviewed roles are `Door_Frame`, `Door_Left`,
`Door_Right`, and `Door_Wall_Above` under the normalized parent (human yaw
−180); source GLBs remain untouched. The binding declares intended hinge axis
and open angle, while runtime derives each role's rest transform and pivot from
the loaded GLB hierarchy. No numeric GLB transform is copied into YAML,
proto, or a second registry.

The names below are **proposed contract names**, not existing API fields.

## Exact authored contract (v4 proposal)

Bump the single-room root and `RoomSource` version to 4 because the current
strict v3 decoder rejects unknown gameplay keys. Version 3 remains accepted
unchanged. The v4 document adds an optional `doorBindings` map under
`room.room`, keyed by the stable `scene.items[].id`:

```yaml
version: 4
room:
  version: 4
  # existing implicitRegionId, walkableHexes, propDeclarations, ...
  doorBindings:
    door-west:
      doorId: west-gate
      state: locked
      locked: [{ability: dex, tool: 'dnd5e:item:thieves-tools', dc: 12}]
      concealed: [{ability: perception, dc: 15}]
      passage:
        closed: {width: 1.8, depth: 0.20, offsetX: 0, offsetZ: 0}
        open: null
```

`door-west` must name one live `scene.items[]` prop. `doorId` is a required,
stable, dungeon-local ID and compiles to the existing `<dungeon-key>/<doorId>`
identity. `state` is required and is exactly `open`, `closed`, or `locked`; no
zero-value default is invented. `locked` is absent for `open`/`closed`, and is
required and non-empty for `locked`, using the existing `CheckSpec` approach
shape and validation (`ability` non-empty, `dc >= 1`, optional `tool`).
`concealed` is absent for an ordinary door and, when present, is a non-empty
existing `CheckSpec` list. Existing probe/knowledge semantics apply.

`passage.closed` is required and is the authored owner-local `RoomFootprint`
(shape, bounds, finite-value checks) converted once by the compiler. It blocks
both movement and sight while the state is `closed` or `locked`.
`passage.open` is required and must be `null` in this first slice: `null` means
OPEN has no placed passage obstruction. This is explicit, not an omitted
fallback. A future use case may add a non-null state-specific obstruction, but
it must preserve the existing DoorState meaning that OPEN blocks neither
movement nor sight.

Absent `doorBindings` means no interactive placed doors and exactly the current
room behavior. Unknown item IDs, non-prop items, duplicate `doorId`s, malformed
footprints, duplicate bindings, unknown keys, missing required fields, a lock on
an unlocked state, or a concealed empty list are compile errors with YAML paths.
A visual item may also have an existing `propDeclarations[itemId]`: that is a
separate fixed frame/wall contributor. It is optional because surrounding
walls may already supply it; the author is not forced to duplicate occupancy.
The door binding itself contributes only the changing passage.

## Canonical input and persisted runtime (proposed)

In `rpg-toolkit/rulebooks/dnd5e/encounter` add a door-specific sibling to
`PlacedPropInput`, not a generic FSM:

```go
type PlacedDoorInput struct {
    ID          DoorID                 // compiled dungeon-key/doorId
    SceneItemID string                 // stable visual item id
    Closed      spatial.FootprintPlacement // canonical, movement+LOS blocker
    State       DoorState              // existing open/closed/locked interface
    Concealed   []CheckApproach        // existing knowledge fact
}
```

`CompileSingleRoom` gets `CanonicalPlacedDoors`, using the same single
source-frame→canonical conversion as placed props. `Closed` is copied and
validated at construction. The encounter stores `placedDoors` separately from
static `placed` contributors; state selects whether the closed contributor is
active. OPEN removes it from both movement and sight, while static frame/wall
contributors remain. `DoorState`, `OpenDoor`, `Unlock`, `DoorsFor`, concealment,
door events, and the existing stable ID rules are reused unchanged in meaning.
`DoorInput` remains for adjacent edge portals; it is not adapted to arbitrary
in-room geometry.

Persistence adds the placed-door record beside existing edge-door data (proposed
`PlacedDoorData`: ID, SceneItemID, canonical Closed placement, state/lock/
concealment). `ToData` and `LoadEncounter` reconstruct the same
`PlacedDoorInput`; current state is session truth, not room YAML after start.
Every snapshot performs the same deep-copy/validation boundary as existing
field and door data. A running session is isolated from later author edits.
No close verb is added.

## Smallest wire join and changed obstruction (proposed)

Keep gameplay declarations out of `RoomScenePresentation` and
`room_scene_json`. Extend the SDK `session.Atlas` with a member-visible,
construction-time descriptor and transcribe it in
`rpg-api-protos/dnd5e/api/session/v1alpha1/service.proto` and
`rpg-api/internal/handlers/dnd5e/session/v1alpha1/convert.go` as a proposed
`GetAtlasResponse.room_scene_doors` field:

```proto
message RoomSceneDoorBinding {
  string scene_item_id = 1; // visual stable ID
  string door = 2;          // compiled authoritative door ID
  PlacedFootprint closed = 3; // canonical placement for client preview
}
repeated RoomSceneDoorBinding room_scene_doors = 15;
```

`PlacedFootprint` is proposed wire data (origin, facing, width, depth), all in
the existing canonical session plane; it is gameplay geometry, not GLB pose.
The binding is omitted for a member who does not know a concealed door, just as
that member's doorway knowledge is omitted. `DoorInfo` remains the live state
and lock answer, keyed by `door`; no duplicate state is added to this binding.

`OpenDoor`/`Unlock` already persist state and publish `DoorChanged`; Web's
`useSessionDoors`/event refresh updates the state map. `RoomSceneEnvironment`
receives the binding list, `doors`, and `onDoorClick`; it joins item ID to the
member-visible `DoorInfo`. The client preview index can use the canonical
closed placement while closed/locked and remove it when the event says OPEN.
The server's encounter fold is authoritative for actual movement and sight; a
preview is only an aid. If a preview cannot represent continuous geometry, it
must under-claim rather than block or invent a route.

## Rendering contract

`RoomSceneEnvironment` renders every item at its exact authored transform and
proportions, then applies the named door binding only to the loaded asset. The
frame and `Door_Wall_Above` remain static. The loader verifies the four named
roles, clones the source scene per instance, and derives each leaf's rest local
transform, hinge pivot, and axis from that instance's actual hierarchy. The
reviewed binding supplies the intended axis and open-angle convention; it does
not supply copied rest numbers. OPEN applies isolated rigid rotation to
`Door_Left` and `Door_Right` around their own derived pivots. CLOSED/LOCKED
restores the derived rest pose. No parent non-uniform scale, bounds-inferred
hinge, filename behavior, or local toggle is allowed. Unknown binding/asset is
a named renderer failure, never substitute scenery. Height/UV automation stays
deferred.

## End-to-end proof

1. In the real World Builder, place two copies of the published asset, author
   different transforms, bind each explicitly, and author passage geometry;
   optionally declare fixed frame occupancy only where walls do not already
   supply it. Save, reopen, and verify exact YAML/item IDs/transforms.
2. `Save & Play` → real `PutDungeon` → registry → `CompileSingleRoom` →
   `StartSession` → member `GetAtlas` must reach canonical
   `RoomSceneEnvironment`, not `DungeonShell/AtlasWalls`. Verify the proposed
   wire join names both instances and preserves authored proportions.
3. CLOSED and LOCKED render closed and refuse a server crossing. Real Open or
   Unlock updates every member through the existing event/refetch path; leaves
   swing rigidly, fixed occupancy remains, OPEN passage movement/sight succeeds
   where the server allows it, and preview state changes without localStorage.
4. Reload `ToData`/`LoadEncounter`, repeat state and movement/sight assertions,
   and test unknown bindings/assets with named failures. Browser visual evidence
   is the primary gate; legacy wall-only fixtures are not acceptance.

## Ownership, test scope, and merge dependencies

| Owner | Narrow responsibility | Proof |
|---|---|---|
| Web editor (`rpg-dnd5e-web`) | v4 draft/YAML decode, declaration UI, room-scene door join, GLB role loader, rigid pose | round-trip/validation tests; `RoomSceneEnvironment` tests; real browser Save & Play |
| Toolkit `encounter/dungeonspec` module | decode/validate `doorBindings`, `CanonicalPlacedDoors`, `PlacedDoorInput`, movement/LOS fold | compiler, malformed-source, closed/open, concealment, ToData/Load tests |
| Toolkit `session` module | session verbs/state persistence and `Atlas` descriptor projection | door verb/event/member-knowledge and atlas projection tests |
| `rpg-api-protos` | transcribe proposed binding/footprint field only after SDK shape is proven | buf format/lint/breaking/generate |
| `rpg-api` | registry/authoring unchanged byte flow; SDK↔proto mapping in `convert.go` | registry, GetAtlas, real session integration |
| `rpg-game-assets` | preserve reviewed asset/catalog identity; no gameplay authority | manifest/hash and GLB-role evidence |

Develop outside-in from Web contract to proto and providers; merge the proto
exception first, then toolkit encounter, toolkit session, API, and Web/Assets
consumers. Adopt real provider tags before each consumer merge. The design PR
remains the written contract and must be reviewed before implementation.

## Residual risk

The proposed `room_scene_doors` field and `PlacedFootprint` are new wire/API
surface, and the current client path index does not yet model continuous placed
geometry. Both are deliberately named so they can be rejected or narrowed in
review before code. No gameplay state lives in visual JSON, no GLB numbers are
repeated, and no cross-cell portal assumption is hidden in the contract.
