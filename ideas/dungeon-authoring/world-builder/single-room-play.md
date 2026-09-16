# One playable World Builder room

**Approved by Kirk in conversation.** Implementation follows the scoped plan;
merge, environment changes and cleanup still need their own authority. This narrows the
approved [architecture](design.md) to one real playable room. Kirk approved
existing monster weapon/mind defaults and visibly hex-snapped monster/start
markers; scenery stays freely placed. Doors are parked while he makes assets.
Multi-room traversal, cover, concealment/intel, height physics, pickup/drop,
loadout/AI editors and region-management UI are not prerequisites.

## 1. The user loop

Build and paint the room; choose an existing monster from the existing palette;
place it on a hex; place a party-start marker; validate, save, and Play through
the existing character/lobby flow. Monster markers are authoring previews, not
props. The game spawns real sheets using each definition's weapons and AI mind.
`FactionSpec.Mind` names a knowledge-bearing member and is **not** that AI mind.

The proof must play **this authored room**, not a substitute legacy YAML demo.
A failed validation or launch leaves the authoring draft intact and displays
its errors; it never falls back to a different dungeon.

## 2. Portable source, not a second scene system

Extend the existing local **RoomDraft to v3** with `room.partyStart` (optional
while editing) and `room.monsters` (initially empty). Each monster is
`{id, ref, cell:{q,r}}`; start is `{q,r}`. Cells are integral axial addresses;
there is no authored creature yaw, continuous spawn position, weapon override
or mind override. IDs are stable; refs use the existing grammar, for example
`dnd5e:monsters:skeleton`. Party seats are **derived**, not a second authored list.

Separately, canonical **dungeon YAML v3** is a `SingleRoomSpec` with exactly
`version`, `key`, `play`, and `room`. `room` embeds that complete RoomDraft v3.
Proposed `play` fields are `void`, `lighting`, and `standing`; this first slice
supports the explicit values below. The toolkit's existing `dungeonspec`
package owns strict decoding and validation, not a new authoring service.

```yaml
version: 3
key: workshop-room
play: {void: transparent, lighting: bright, standing: centre-covered}
room:
  version: 3
  id: room-1
  name: Workshop
  coordinateFrame: {horizontalPlane: world-xz, verticalAxis: world-y-up,
    distanceUnit: world-scene-unit, hexRadius: 1,
    footprintFrame: owner-local-xz}
  workspace: {hexRadius: 6, horizontalLimit: 12}
  scene:
    version: 1
    id: scene-1
    name: Workshop
    items:
      - id: table
        kind: prop
        assetRef: dnd5e:props:torture-table
        label: Table
        transform: {x: -2.25, y: 0, z: 1.3, rotationY: 0.37}
        heightScale: 1.5
        parentId: furniture
      - id: candles
        kind: prop
        assetRef: dnd5e:props:candles
        label: Candles
        transform: {x: -2.1, y: 1.2, z: 1.25, rotationY: 0.37}
        parentId: furniture
        supportId: table
        pointLight: {enabled: true, offset: {x: 0, y: 0.5, z: 0},
          color: '#ff9d52', intensity: 1.1, range: 2.6}
    groups:
      - {id: furniture, kind: group, label: Furniture,
         transform: {x: -2.175, y: 0.6, z: 1.275, rotationY: 0.37}}
  room:
    implicitRegionId: room-1-region
    walkableHexes: [{q: 0, r: 0}, {q: 1, r: 0}, {q: 2, r: 0},
      {q: 0, r: 1}, {q: -1, r: 1}, {q: -1, r: 0},
      {q: 0, r: -1}, {q: 1, r: -1}]
    propDeclarations:
      table:
        blocksMovement: true
        blocksLineOfSight: false
        footprint: {width: 1.2, depth: 0.5, offsetX: 0.1, offsetZ: -0.2}
    arrangementDeclarations: {}
    partyStart: {q: 0, r: 0}
    monsters:
      - {id: skeleton-a, ref: 'dnd5e:monsters:skeleton', cell: {q: 2, r: 0}}
```

These scene fields retain their existing meanings. `parentId` names a group;
`supportId` names a prop. Item/group transforms are already world-posed; group
poses do not apply another render transform. Point lights stay on their owner,
with the existing boolean, hex-color and numeric fields. Missing `heightScale`
means normal height; its accepted range stays 0.25–4. A declaration is keyed by
an existing **prop** ID and supplies both booleans plus its local rectangle;
absence means visual dressing, not inferred blocking. Explicit false stays false.
Grouping does not combine independent gameplay contributors.

`workspace` retains the actual radius/limit preset. `arrangementDeclarations`
retains the existing arrangement-ID → template-prop-ID → declaration map; those
are template IDs, not requirements that unused templates become live actors.
There are no newly invented selection, stamp, support-array or light-array
schemas. Used library pieces are already copied into the inline scene. External
model bytes remain references; a composition ID is never the only room copy.
This preserves the approved portability decision and corrects the earlier
scratch draft's ref-only suggestion and invented scene fields.

Local draft v3, saved room-document envelope v2, and dungeon YAML v3 are distinct
formats. The new local key is `rpg.concepts.world-building.room-draft.v3`;
read v2 by explicit lossless upgrade (no monsters/start), preserving its bytes.
An invalid v3 record must not silently fall back to v2. New saved room envelopes
contain RoomDraft v3; the current envelope v1 remains readable through upgrade.
Legacy dungeon v2 stays supported separately. Older readers must refuse new
formats instead of resaving away their meaning. No saved-game migration.

## 3. Source coordinates versus engine geometry

The source scene keeps its **declared portable authoring frame**, avoiding a
second scene format and a second authored pose. It is not the runtime geometry
contract. One toolkit compiler adapter converts its X/Z plane into spatial XY
**feet**; encounter queries use only the resulting canonical geometry.

For pointy hexes of editor circumradius 1:

```text
editor centre = (sqrt(3)*(q+r/2), 1.5*r)
k = 5/sqrt(3) feet per editor unit
spatial origin = (transform.x*k, transform.z*k)
facing degrees = -transform.rotationY*180/pi
Box.D = footprint.width*k; Box.W = footprint.depth*k
LocalOffset = (footprint.offsetX*k, footprint.offsetZ*k)
```

Five feet is the **across-flats/adjacent-centre width**, not the radius.
`(1,0)` maps to `(5,0)` feet; `(0,1)` to `(2.5,5*sqrt(3)/2)` feet. Spatial D
lies along Facing, hence the deliberate width/depth-name swap. Positive Three
Y yaw turns +X toward -Z, hence the sign change. No eight-direction quantization.

Source y, group poses and light offsets/ranges remain source-unit presentation
data in the retained snapshot; whenever expressed in canonical feet they also
multiply by k. Height is **not** exempt from distance conversion. Height scale,
intensity and color do not scale. Preserve doubles; do not narrow through
legacy float32 pose fields. The renderer uses the retained frame/scene directly,
or its exact inverse adapter—not an additional calibration or group transform.

Use `FootprintPlacement`, stationary `TraceFootprint.Contact` for centre-covered
standing, interior-crossing traces for thin movement barriers, and the existing
`SightLanes` evaluator supplied with footprint-aware obstruction facts. Keep the
source footprints available for sight outside the painted mask. No mesh-derived
rules or new geometry algorithm in the API/client. Visual height affects none
of these 2D declarations.

## 4. Provider and transport additions

All names in this section are **proposed additions**:

- **Encounter** owns typed `RoomScenePresentation` (source frame, workspace,
  complete visual scene) beside its existing field/atlas presentation facts,
  and `PlacedPropInput` (ID, canonical `FootprintPlacement`, movement/sight flags).
  Dungeonspec's `RoomSource` uses that same scene type. No duplicate pose DTOs or
  arbitrary property bags. The existing world/composition store remains generic.
- **Dungeonspec** dispatches YAML v3 into `SingleRoomSpec`; compiles one implicit
  region from painted cells, explicit transparent void/bright lighting, and
  placed contributors. Extend the existing compiled Dungeon output with this
  field; retain its existing party-start/seats and monster-spawn outputs. Do not
  introduce a separate playable-room runtime.
- **FieldInput, FieldData and Atlas** gain the optional room presentation and
  placed-contributor fields needed for construction, persistence and projection.
  Rebuild standing, crossing and sight facts from the same persisted inputs;
  removal must retain overlapping contributors. Old cell props remain the v2
  path, not fake anchors for v3 visuals.
- **Protos** add `GetAtlasResponse.room_scene_json`: a versioned encoding of
  canonical `RoomScenePresentation`, strictly decoded at both ends, preserving
  doubles and the full visual graph. It contains no author monster/start markers
  or gameplay-state substitute. Existing atlas cell/boundary channels plus the
  session's movement/sight answers remain mechanical truth; extend diagnostic
  projection only where required to show the same compiled contributors.
- **API** converts that atlas field, persists complete v3 YAML through existing
  Put/Get/List, and reuses the registry and lobby launch. The session SDK remains
  responsible for Join/Spawn and rule execution. No new RPC or API-side rules.
- **Web** reuses existing palette refs and visual leaves, adds snapped setup
  markers, v3 encode/decode and validate/save/Play. A v3 game scene renders the
  retained underlay/workspace and authored pieces, without editor guides,
  duplicate legacy prop proxies or autogenerated perimeter walls. Real actors
  come only from member-scoped session state; preserve its visibility handling.

Use final standability/occupancy—not floor membership alone—for monster cells,
party start and existing deterministic seat allocation. Monster refs resolve
before readiness; launch checks capacity for the actual ready party before
creating a partial game. Published edits affect future launches only: each run
persists its compiled presentation and mechanics, including on reload.

## 5. Delivery and acceptance

Develop from the editor contract; merge providers before consumers: toolkit
source/compiler/encounter integration → atlas proto generation → thin API pin
and adapters → editor/game renderer. Exact issue-sized steps follow approval.

Use one room, an existing monster, a party start, a raised grouped table with
supported lit decor, and a thin blocker. Prove source export/reopen without a
Redis composition; free negative/fractional poses and asymmetric rotated local
footprints; actual monster/default behavior; one real character entering the
same authored room; blocked movement/LOS and explicit false flags; overlap-safe
removal; and reload of the same running snapshot. Validate errors name the
editable item/cell, leave the draft intact and refuse launch. Asset failures
remain explicit, never substituted models. No door or exhaustive-polish gate.

### Inspected anchors

Web `types.ts`, `roomDraft.ts`, `sceneState.rotateTransform`, `hexMath.cubeToWorld`
and `CompositionModel` at height head `3438f64b`; toolkit
`spatial/embedding.go`, `placed_footprint.go`, encounter `FieldInput/FieldData/Atlas`,
`dungeonspec` and session `SpawnInput` at inspected checkout `c8dd8bd8`;
API authoring/lobby launch at `f5ecaad6`; authoring/atlas protos at `a490b6a`.
Refresh provider refs before implementation planning; these are evidence
snapshots, not claims about the newest releases.
