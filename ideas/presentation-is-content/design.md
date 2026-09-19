# Presentation is content — the encounter stops carrying the World Builder's scene

**Status:** RULED 2026-09-19 (rpg-project#479): Kirk took R1–R6 as written, with one long-term note: "long term these scenes will have revisions that are immutable. for our current stage your call is fine." So serve-by-key is the current stage; an immutable scene revision pinned on the session is the registry's future, never the encounter's. Lane: toolkit (platform). Building in the R6 order.

**Where it came from (Kirk, 2026-09-19):** "so a downstream consumer is shaping the internal of
our encounter? … we are meant to be composable but it feels like we are making another
application again." And the ruling that follows: "before we build on this let's get it cleaned
up. yesterday was when we were coming up with ideas … now we should clean up."

## The one sentence

The World Builder's scene is content, served by dungeon key; the engine reads the three numbers
per prop it needs to place a footprint and carries nothing else, and there is one authored
gameplay grammar under two geometry dialects instead of a second dialect adapted into the first.

## What is true today, measured

Every claim below was verified against rpg-toolkit `origin/main` (58b30731, encounter v0.93.0),
rpg-api `origin/dev`, rpg-api-protos `origin/main` and rpg-dnd5e-web `origin/dev` on 2026-09-19.

### The presentation's path through the engine

| Step | Where |
|---|---|
| The builder's draft (`rpg-room-authoring-draft`, kind at `roomDraft.ts:15`) is wrapped as the single-room document | web `singleRoomDungeon.ts:53` |
| dungeonspec decodes it with a full shape walk over frame, workspace, items, groups, transforms, lights | `single_room_decode.go:383-478` |
| … and copies the whole thing into `FieldInput.RoomScene` | `single_room_compile.go:33-35`, `presentationCopy` at `:158-175` |
| `compileField` validates it with bounds **mirrored from the editor's TypeScript** and snapshots it on the field | `compilefield.go:236-250`, `:341`; `room_scene_validate.go:19-20`: "SCALAR bounds are the editor's own (world-building serialization.ts / roomDraft.ts at height head)" |
| It is written into **every** encounter record under `room_scene` | `data.go:240`, `:1871`; read back `:3052` |
| … which rpg-api marshals whole into Redis on **every save** | `redis_repos.go:107-114` |
| It is projected on `Atlas.RoomScene` | `atlas.go:93`, `:327` |
| The session SDK re-validates it and marshals it to a JSON string | `session/convert.go:87-98`, `types.go:208` |
| rpg-api copies the string onto the wire | `session/v1alpha1/convert.go:1677` → `GetAtlasResponse.room_scene_json = 14` (`service.proto:1357`) |
| The web decodes it from the atlas and renders it | `roomSceneJson.ts:129`, `atlasToScene3D.ts:307-321`, `DungeonEnvironment.tsx:50-62`, `RoomSceneEnvironment.tsx:188-196` |

**What the engine reads from all of that for play: three numbers per prop.** The one adapter,
`placedPropFrom` (`single_room_placement.go:99-110`), reads `item.Transform.X`, `.Z` and
`.RotationY` keyed by item id, joins them to the gameplay block's own `RoomFootprint` and two
blocking booleans, and produces a `PlacedPropInput`. The decoder also reads
`workspace.hexRadius` to bound the walkable cells (`single_room_decode.go:631`). Nothing in play
reads an asset ref, a label, a parent id, a height scale, a point light's color, intensity or
range, or the frame's axis words. The engine judges all of them anyway.

### What it costs

| Cost | Measure |
|---|---|
| Engine source that carries or judges a document it does not read | 513 lines (`room_scene.go` 137, `room_scene_validate.go` 376) |
| Engine tests keeping that document consistent with the editor | 870 lines (`room_scene_persistence_test.go` 748, `room_scene_internal_test.go` 122) |
| Carriers, one per layer, all for the same bytes | `FieldData.RoomScene`, `Atlas.RoomScene`, `session.Atlas.RoomSceneJSON`, proto `room_scene_json` |
| The record | every live encounter in Redis holds the full authored visual scene and rewrites it on every save |
| The second dialect | toolkit#1834 had to build the single-room document **as a v2 `Spec`** (`single_room_site.go:180`) to reuse the gameplay validators, and its review named the transcription of `monstersOf` into `CompileSingleRoom` as the drift risk |
| Lockstep | the editor's scalar bounds must change in Go and TypeScript together, or "a room the editor accepts" is refused at compile |

### How it got here

Lane #1753 (four milestones, `rpg-project/ideas/dungeon-authoring/world-builder/*-plan.md`) set
the goal as *"compile the complete approved room into the existing encounter, with faithful
persistence and atlas presentation"* (provider plan :5) and *"carry the complete canonical room
presentation over the existing authoring/session atlas response"* (proto plan :5). Losslessness
was the requirement, and the encounter was chosen as the carrier because it was the one thing
the play view already fetched. The choice was made to get the World Builder's room onto the
table, and it did. It is now the shape everything new is being poured into: the site keys
(#1826, shipped as an adapter) and the authored doors (rpg-project#468 assumes *"the canonical
room-scene branch"* on the atlas is how play reaches a door's appearance).

## The principle it breaks

**Ownership before mechanism.** Ask who owns each noun:

| Noun | Owner | Where it lives today |
|---|---|---|
| What a room looks like (assets, transforms, lights, labels) | the **World Builder**, as content | the encounter record, four carriers deep |
| Where a prop stands and what it blocks | the **engine** (`PlacedPropInput`, the field) | the engine (right) |
| The editor's scalar bounds and workspace presets | the **editor** | `room_scene_validate.go`, mirrored |
| The authored file, by key | the **content registry** (`internal/dungeons/registry.go`, "never re-marshals a file: GetDungeon hands back exactly the bytes that were Put") | the registry (right), **and** the encounter |
| The authored gameplay grammar (factions, dispositions, answers, temper, actions, arrivals) | **dungeonspec** | validators bound to the v2 `Spec`; the site document adapts into it |

A downstream consumer's document inside the engine is the thing the toolkit's composability
rule exists to prevent: the encounter should be usable by a host that has no Three.js scene at
all, and today it validates one on every compile.

## The shape

### 1. The scene is served by key, not by encounter

The registry already holds the verbatim file per key and `AuthoringService.GetDungeon(key)`
already returns it, ungated (`get_dungeon.go:14-27`; proto doc: *"Ungated: reading content
mutates nothing"*). The web already has the codec for that file (`decodeSingleRoomDungeon`) and
already calls `GetDungeon` from three places. What the play view lacks is only the **key**:
`dungeonKey` exists on the launch request and nowhere after (`useStartLobbyEncounter.ts:16`).

So: the session record carries the dungeon key it was launched from, `GetAtlasResponse` gains
`dungeon_key`, and the play view fetches the file by key and reads the scene out of it with its
own codec. The api stays dumb: one string field, no scene RPC, no scene DTO. The scene the
player sees and the field the engine compiled come from the same bytes, because the registry
compiled that entry from them.

**What this forecloses, honestly (and Kirk's answer):** a running session no longer carries its own visuals. If the
file under a key is re-Put while a session is live, the geometry stays what was compiled at
launch and the scene changes underneath it. Pre-v1 that is acceptable and visible. When it
stops being acceptable, the fix is the registry's (a content hash on the entry, pinned on the
session), never the encounter's. Kirk, ruling R1: "long term these scenes will have revisions that are immutable" — the session will one day pin a revision, not a mutable key; that revision is content's noun.

### 2. The engine carries nothing it does not read

Delete from `rulebooks/dnd5e/encounter`: `RoomScenePresentation` and its types,
`ValidateRoomScene` and the mirrored bounds, `FieldInput.RoomScene`, `field.roomScene`,
`FieldData.RoomScene` (`room_scene`), `Atlas.RoomScene`, `copyRoomScene`, the three concealed-
structure refusals that exist only because a scene is present, and both test files. Keep
`PlacedPropInput` and everything the field does with a footprint: that is engine.

From `rulebooks/dnd5e/session`: `Atlas.RoomSceneJSON` and its validation.
From protos: `room_scene_json` deprecated, never removed in place (protos: deprecate, don't
break). From rpg-api: the one converter line; `dungeonstest/workshop.go` stops building a
toolkit presentation type. From the web: `roomSceneJson.ts` and the atlas branch in
`atlasToScene3D.ts`; `DungeonEnvironment` takes the scene from the document instead.

**Loaded records:** a record saved with `room_scene` loads with the key ignored. No migration.

### 3. dungeonspec lowers the draft; it does not carry it

The single-room decoder reads, from `room.scene`, exactly what `placedPropFrom` needs: for each
item id, `transform.x`, `transform.z`, `transform.rotationY`; and from `workspace`, `hexRadius`.
It stops walking frame words, lights, labels, groups, parents, height scales. Strictness over
the presentation belongs to the codec that owns it (the web's, which already validates it and
refuses what it does not know). Strictness over the gameplay keys stays exactly as it is:
`KnownFields(true)` and the shape walk on `room.room`, `play`, the root, and every site key.

One open cost here: `KnownFields(true)` over the typed decode means the scene's fields still
need Go struct fields to land in, or the decode must read the scene as a `yaml.Node` and pick
the three numbers out of it. The second is the honest one (the engine does not model the scene);
it is also where the decoder's mirrored bounds go away for good. R3 below.

### 4. One gameplay grammar, two geometry dialects

The adapter #1834 built (`siteSpec(s)` → a v2 `Spec` → the v2 validators) is evidence that the
gameplay grammar is already independent of geometry. Make that structural: the types and
validators for `factions`, `dispositions`, answers (`on:`, `when`, words, selectors), `temper`,
`actions`, `arrives`, intel and scenarios become functions over their own types, taking a path
prefix and a membership index, with no `*Spec` receiver. The v2 dialect (regions, walls, doors,
offset cells, orientation) and the site dialect (axial cells, placed footprints) each lower
their geometry to `FieldInput` and call the same grammar for the rest. `CompileSingleRoom` stops
transcribing `monstersOf`. The two threads deferred from #1834's review land here: the faction-
level `at:` pin, and the orders helper (which becomes the shared grammar itself).

**Not in this design:** retiring v2. All five shipped reference dungeons and the front room are
v2, and the World Builder cannot yet author what they do (regions, doors, arrivals, intel,
scenarios). v2 stays authorable until the builder can, and nothing here builds a converter.

### 5. The frame question stays refused

`at:` inside a site document stays refused with the shipped sentence. The site dialect's own
frame (axial `{q, r}`) is the obvious answer when the sites layer gives a creature somewhere to
walk to; deciding it now would be building for a use case not yet brought.

## Ownership after

| Noun | Owner | Lives in |
|---|---|---|
| The room's appearance | the World Builder | the authored file, served by key from the registry |
| A prop's stance and what it blocks | the engine | `PlacedPropInput` → the field |
| The three numbers that connect the two | dungeonspec's lowering | `placedPropFrom`, reading the scene as a node |
| The authored gameplay grammar | dungeonspec, one package-level grammar | shared by both dialects |
| Which dungeon a session is playing | the session record | `dungeon_key` on the record and on `GetAtlasResponse` |
| Whether a scene is well-formed | the web's codec | `singleRoomDungeon.ts` / `roomDraft.ts`, the only validators |

## Rulings needed

- **R1. Serve by key.** The play view fetches the file by `dungeon_key` through the existing
  ungated `GetDungeon` and reads the scene with its own codec; no scene RPC, no scene DTO.
  (Alternative, not recommended: a `GetDungeonScene` RPC returning presentation JSON, which
  keeps a scene shape on the api's surface.)
- **R2. The engine drops the presentation entirely**, including `FieldData.room_scene`; loaded
  records ignore the key. (Alternative: keep an opaque `json.RawMessage` pass-through on the
  record. Not recommended: it is the same coupling with the type erased.)
- **R3. The lowering reads the scene as a node**, picking `transform.{x,z,rotationY}` per item
  id and `workspace.hexRadius`, and validates nothing else about it. (Alternative: keep typed
  scene structs in dungeonspec with `KnownFields` off for that subtree.)
- **R4. The gameplay grammar becomes package-level functions over its own types**, shared by
  both dialects; the #1834 adapter is deleted in that PR.
- **R5. v2 stays authorable**; no converter; the site dialect is the World Builder's.
- **R6. Order.** Presentation out first (protos → toolkit encounter → session → api → web, one
  wave, walked on the workshop room and one v2 dungeon), then the grammar split as its own
  toolkit wave. Doors (#468) and further site keys wait behind both.

## Slices

| # | Repo / module | What | Proof |
|---|---|---|---|
| 1 | rpg-api-protos | `GetAtlasResponse.dungeon_key`; `room_scene_json` marked deprecated | buf lint/breaking; generate compiles |
| 2 | rpg-toolkit `rulebooks/dnd5e/encounter` | delete the presentation (§2); dungeonspec lowering reads the node (§3) | the workshop fixture compiles to the same `PlacedPropInput`s; a v3 record with `room_scene` loads; committed pictures unchanged except the dropped key |
| 3 | rpg-toolkit `rulebooks/dnd5e/session` | drop `RoomSceneJSON`; `Atlas.DungeonKey` | unit |
| 4 | rpg-api | session record carries the key; converter fills `dungeon_key`, drops the scene; `workshop.go` builds no toolkit presentation | handler tests; a Redis encounter blob contains no `room_scene` |
| 5 | rpg-dnd5e-web | play view fetches by key and renders from the document; delete `roomSceneJson.ts` | the workshop room screenshot before and after is the same; a v2 dungeon renders as today |
| 6 | rpg-toolkit encounter (second wave) | the grammar split (§4), the two deferred pins | both dialects' committed pictures unchanged; `TestTheTwoDialectsCompileTheSameOrders` deleted because there is one path |

**The walk (Kirk):** launch the workshop single room, see the same room; open the debug feed
and confirm the atlas carries a key and no scene; launch the front room (v2), see it unchanged.
Read one Redis encounter blob and confirm no `room_scene`.

## Done when

- No type in `rulebooks/dnd5e/encounter` names an asset, a transform, a light or a workspace.
- `git grep room_scene rulebooks/dnd5e` is empty; the proto field carries `[deprecated = true]`.
- The workshop room renders from `GetDungeon(key)` and looks the same as before.
- `single_room_site.go`'s `siteSpec` adapter is gone and both dialects call one grammar.
- rpg-project#468 is re-based on "the scene comes from the document, door state from the atlas,
  joined by item id" before any door work starts.
