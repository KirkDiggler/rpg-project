# Authored doors in the single-room dialect — scene from the document, state from the atlas, joined by item id

**Status:** PROPOSED 2026-09-19. Re-base of rpg-project#468 (merged doc `ideas/assets/selectable-door-appearances/design.md`, parent #467 still open) on the seam rpg-project#479 shipped. Builds on the grammar split (#484, building). Lane: toolkit (platform) for the engine slice; the web slice is a wire-anchored brief for Kirk's lane.

**Where it came from (Kirk, 2026-09-19):** "once we have our door assets configured properly we
will be back in multi room authoring" and, today, "We have the roles needed for doors in our new
door in web now too."

## The one sentence

A door the World Builder places is the engine's existing edge door: the document says which item
is a door and whether it starts closed, locked, or concealed; the dialect lowers the item's pose
to the hex edge it stands on; the atlas and `GetDoors` carry what they carry today; the web joins
state to the item by id and swings the leaf the asset's roles already describe.

## What is true today, measured

Verified 2026-09-19 against rpg-toolkit `origin/main` e94237d5 (encounter v0.94.1), rpg-dnd5e-web
`origin/dev`, rpg-api-protos `origin/main`, rpg-game-assets `origin/main`.

### What #468 assumed that #479 removed

| #468 said | What shipped instead |
|---|---|
| the atlas reaches `RoomSceneEnvironment` through the canonical room-scene branch (`:150-153`) | the atlas carries `dungeon_key = 15`; the web re-fetches the document (`useDungeonScene.ts`, `SessionEncounterView.tsx:207-210`) |
| `repeated RoomSceneDoorBinding room_scene_doors = 15` on `GetAtlasResponse` (`:96-124`) | tag 15 is `dungeon_key` (`session/v1alpha1/service.proto:1374`); the SDK `Atlas` carries `DungeonKey`, no scene (`session/types.go:205-229`) |
| bump root, room draft, and envelope to v4 (`:23-25`) | root v4 already accepted and shared with the site keys; **the room draft stays 3**, "root 4 with room 3 is the only combination the web can currently produce" (`single_room_decode.go:520-536`; web `roomDraft.ts:126,165`) |
| `PlacedDoorInput`, `CanonicalPlacedDoors`, `placedDoors` stored beside `placed` (`:63-88`) | the single-room `FieldInput` has no `Walls` and no `Doors` (`single_room_compile.go:65-75`); `door` appears in `single_room*.go` twice, both prose |
| a `PlacedFootprint` wire message for the door (`:100-118`) | props already carry `spatial.FootprintPlacement` and the atlas exposes it (`atlas.go:236-247`); a second footprint message is duplicate surface |
| the loader verifies four roles and derives hinges from a door binding (`:131-134`) | shipped, from the asset catalog, not from YAML: `WorldAssetModel.tsx:136-190` (`resolveRoles`, swing about derived hinge); roles are `frame \| leaf \| above` with a `door` group key (`worldAssetCatalog.ts:53-55`), not the node names |
| `RoomSceneEnvironment` takes `doors` and `onDoorClick` (`:138-140`) | its props are exactly `{ presentation }` (`RoomSceneEnvironment.tsx:11-13`); the canonical branch returns before the legacy door renderer (`DungeonEnvironment.tsx:168-175`) and renders zero doors |

**What #468 said that shipped code ratified:** the binding is keyed by the stable scene-item id.
`single_room.go:118` names `doorBindings` as "the THIRD declaration kind on a placed thing …
keyed the same way: by the id of the thing it is about."

### The door the engine already has

| Layer | Shape | Where |
|---|---|---|
| authored (v2) | `DoorSpec{ID, At PositionSpec, Locked CheckSpec, Closed bool, Concealed CheckSpec}`; `At` is a side midpoint with exactly one wall through it | `spec.go:714-745` |
| compiled | `DoorInput{ID, Edge DoorEdge, State DoorState, Concealed}`; identity `<key>/<id>` | `compile.go:318-328, 379-391`; `door.go:231-263` |
| runtime | sealed `DoorState`: open / closed / locked(Lock); registered on the spatial canvas; both edge endpoints must be floor and adjacent | `door.go:172-203`; `compilefield.go:667-711` |
| atlas | `AtlasDoorway{Door, From, To}`, two cells and an id, **state deliberately excluded** ("what state that door is in is `Encounter.Doors`' business") | `atlas.go:55-59, 271-285` |
| live state | `DoorInfo{door, state, lock}` from `GetDoors`, patched by DOOR / DOOR_REVEALED beats; a concealed unfound door is absent from both lists | `service.proto:1185-1197, 1393-1451`; web `useSessionDoors.ts` |

**The single-room dialect has no door and no wall.** A v2 door is "a position on a wall"; the
single-room room has no walls, so the v2 shape is not unused here, it is inapplicable. What the
dialect does have is the prop precedent: `PropDeclarations` keyed by item id, joined to the
item's `transform.x/z/rotationY` by `placedPropFrom` (`single_room_placement.go:73-118`), item id
carried verbatim to `PlacedPropInput.ID` and `AtlasPlacedProp.ID`; duplicate ids a hard defect
(`single_room_lowering.go:193-201`).

### The web's door, measured

- A placed door is a plain `WorldProp{id, assetRef, transform, …}` (`world-building/types.ts:29-43`).
  There is no door field on the item; door-ness is the asset's `roles`.
- The renderer that swings a leaf exists and is already on the canonical path:
  `RoomSceneEnvironment` → `WorldPropModel` → `WorldAssetModel` (`resolveRoles` at `:136-186`).
  What it lacks is a state input.
- **The generated catalog on `origin/dev` has zero `roles` entries** (`grep -c 'roles:'` over
  `src/generated/worldAssetCatalog.ts` = 0). rpg-game-assets authored them for
  `dnd5e:env:dark-fortress:wall_door_double_01` (commit 58b7b9e: frame → `Door_Frame`, leaf ×2
  group `gate`, above → `Door_Wall_Above`); the web's catalog is pinned to an older assets
  commit. Tracked as rpg-dnd5e-web#1124. Kirk reports the roles present in his World Builder;
  the regenerated catalog is the precondition on `dev`.
- Legacy v2 doors render as a gap cut in a wall run and a leaf whose only state read is
  `doors.get(connection)?.state !== OPEN` (`AtlasWalls.tsx:239-250`). That renderer is not
  reached on the canonical branch and is not extended here.

## The principle

**Ownership before mechanism.** The engine owns door state and the edge it sits on. The
document owns which item is a door and its starting state. The dialect owns how a pose becomes
an edge, exactly as it owns how a pose becomes a footprint. The asset owns how a door looks and
which part swings. The web owns the join and the click. Nothing new is owned by anyone; #468's
new carrier, new footprint message, and new placed-door store each gave an existing owner's
noun a second home.

## The shape

### 1. The document: `doorBindings`, the third declaration kind

```yaml
room:
  room:
    propDeclarations: { crate-1: { footprint: …, blocksMovement: true } }
    doorBindings:
      gate-1: { closed: true }
      gate-2: { closed: true, locked: { dc: 15 } }
```

`doorBindings[<itemId>]` carries v2's `DoorSpec` keys with `at` removed: `closed`, `locked`,
`concealed`. Same types (`CheckSpec`), same nil-vs-empty rule, same sentences. This is the
door's *state grammar*; after #484 it is validated by the shared grammar under both dialects,
and `at` is the geometry the dialect supplies. Root stays v4; the room draft stays 3.

An item is one declaration kind: an id in both `propDeclarations` and `doorBindings` is refused
at its path. A binding naming no live item, or an item with no authored transform, refuses by
the prop path's own sentences. `arrangementDeclarations` gets no door form in this slice: a door
stamped inside an arrangement is refused by name (#468 `:59-62` named the remap; the prop path
remaps through `roomDraft.ts:371-372`, and a door does the same when a use brings it).

### 2. The lowering: a pose becomes an edge

`placedDoorFrom(id, binding, pose)` lowers `transform.x/z/rotationY` to a `DoorEdge`: the hex
edge nearest the door's origin, crossed along the door's facing, whose two cells are both in the
room's walkable set. It produces `DoorInput{ID: <key>/<itemId>, Edge, State, Concealed}` and
the compiler sets `FieldInput.Doors`, which the field already validates
(`compilefield.go:667-711`). Refusals by name: the door stands on no edge (origin too far from
any edge midpoint, threshold the dialect owns, in the same feet-per-cell frame as
`placedPropFrom`), or the edge's far cell is not floor ("a door needs floor on both sides").
The door has no footprint and no `Placed` entry: closed-blocks-movement-and-sight is the door
primitive's own behaviour, unchanged.

### 3. Identity and the join

`DoorID` is minted exactly as v2 mints it: `<dungeon key>/<item id>`. `AtlasDoorway`,
`DoorInfo`, `OpenDoor`, `Unlock`, the DOOR beats: unchanged. No proto change. The web derives
the door id from the item id and the dungeon key it already fetched by, and looks `DoorInfo` up
by it. `AtlasPlacedProp` is the precedent: item id in, item id out.

### 4. The web slice (wire-anchored, Kirk's lane)

`RoomSceneEnvironment` gains the `doors` map and `onDoorClick` it was designed to take. For each
item, if `doors.get(`${key}/${item.id}`)` exists, `WorldPropModel` passes `open = state === OPEN`
to `WorldAssetModel`, which swings the `leaf` roles it already resolved; a click on a door item
calls `onDoorClick` with that id. A concealed unfound door has no `DoorInfo` and renders as its
asset, closed, unclickable: the same absence the legacy renderer shows. Precondition:
rpg-dnd5e-web#1124, the regenerated catalog with `roles`.

### 5. Not in this design

Multi-room. In one room both cells of every door edge are inside the room; the door blocks and
opens but leads nowhere new. When the sites layer joins rooms, the door edge is the seam and its
two cells are in two rooms; that is the sites wave and it changes nothing here. Also not here:
exits through a door, endings, a door on a room boundary, and any World Builder UI beyond the
join.

## Ownership after

| Noun | Owner | Where |
|---|---|---|
| Which item is a door and its starting state | the document | `doorBindings` |
| closed / locked / concealed legality | the grammar (post-#484) | shared with v2 `DoorSpec` |
| Pose → edge | the single-room dialect | `placedDoorFrom` beside `placedPropFrom` |
| Door state at play, the edge, blocking | the engine | `door.go`, unchanged |
| Door id | the engine's existing minting | `<key>/<itemId>` |
| How a door looks and which part swings | the asset's roles | rpg-game-assets catalog → `WorldAssetModel` |
| The join and the click | the web | `RoomSceneEnvironment` |

## Rulings needed

- **R1.** A single-room door is the existing edge door. No new engine primitive, no new
  proto, no new atlas field, no new footprint message.
- **R2.** `doorBindings[<itemId>]{closed, locked, concealed}` inside root v4; room draft stays
  3; the state keys are v2's `DoorSpec` minus `at`, validated once by the shared grammar.
- **R3.** The dialect lowers pose to edge (`placedDoorFrom`); no edge or no floor across it
  refuses by name. A door has no footprint and no prop declaration.
- **R4.** `DoorID` = `<key>/<itemId>`, v2's minting; the web derives it, nothing carries it twice.
- **R5.** A door inside an arrangement is refused in this slice.
- **R6.** The web slice is Kirk's lane; rpg-dnd5e-web#1124 is its precondition and the assets
  lane's issue, not ours.

## Slices

1. **encounter** (toolkit, one module, after #484 merges): `doorBindings` decode + grammar
   validation + `placedDoorFrom` + `FieldInput.Doors`; the v4 fixture gains a door; a golden
   picture shows the doorway. Session and rpg-api need no change; the door reaches `GetDoors`
   through the seam they already have. Ready PR, minor bump (new authored key).
2. **web** (Kirk's lane, brief on request): join + click + roles precondition.

## Done when

- The v4 fixture with a door compiles to a picture whose atlas has one `AtlasDoorway` with the
  door's two cells and id `<key>/<itemId>`; every other golden byte-identical.
- Refusals pinned by path and sentence: unknown item, no transform, prop-and-door, no edge, no
  floor across, arrangement door.
- `grep -rn 'PlacedDoorInput\|placedDoors\|room_scene_doors' rpg-toolkit rpg-api-protos` empty.
- On the local stack: put the fixture, start a session, `GetDoors` lists the door closed,
  `OpenDoor` opens it, the DOOR beat arrives. One click per seam.

## Next

The sites layer: rooms joined at door edges, `at:` given its frame, exits and endings across
rooms. #467 closes when slices 1 and 2 land.
