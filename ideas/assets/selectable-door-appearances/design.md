# Project467 — authored doors in the World Builder

## Scope correction — earlier proposal below is not approved for implementation

Kirk clarified that the target is the NEW World Builder, where visual placement
and occupied space are explicitly authored. The goal is to make the door look
right, not constrain it to the older atlas renderer's one-unit doorway gap.

Verified at Web11755d8: `DungeonEnvironment` selects `RoomSceneEnvironment` when
`scene.roomScene` is present and bypasses `DungeonShell`/`AtlasWalls`. The saved
scene item already carries `assetRef` and placement. Therefore the earlier
proposal to add an appearance field through the old DoorSpec/AtlasDoorway path
must not drive this implementation without tracing the canonical room-scene
interaction and occupancy seams first.

Revised direction: preserve intentionally authored visual proportions/placement;
author the opening and blocking space to match; bind the placed item's door
parts to authoritative interactive state. Keep fixed frame/wall occupancy
separate from the passage obstruction changed by opening. Never derive gameplay
blocking solely from mesh bounds or fake shared door state in local storage.

Next design pass must trace World Builder saved room-scene/gameplay data through
play/session rendering and identify the minimal item-to-interaction binding.
It must establish which existing door-state machinery can genuinely be reused.
No product implementation has started. Retain the old proposal below as the
record of the assumption being corrected, not as an implementation specification.

## Earlier proposal — superseded target assumption

Add an optional, per-door opaque `appearance` reference to the authored dungeon
file. This is a selectable appearance, not a global replacement: absence keeps
today's frame/leaf renderer and existing default available. It changes no door
identity, state, walkability, concealment, or Open/Unlock behavior. Close remains
out of scope; this slice presents closed/locked and open poses.

The reference is transport metadata. Toolkit/API must not know GLB node names,
materials, pivots, or motion. Assets/Web owns the binding from the stable
appearance reference to a reviewed assembly (runtime file, named roles, rest
transforms, hinge axes/angles). Unknown or invalid references fail visibly in the
renderer and fall back to the old default; they never silently become scenery or
an unrelated appearance. A missing reference is not an error.

## Verified live chain and ownership

At Web `11755d8ae0362130efc4d69bef0e1a5d95249c15`, the running route is:
`GameView` → `SessionEncounterView` → `SessionCanvas` →
`DungeonEnvironment` → `DungeonShell` → `AtlasWalls`. `GameView` explicitly
calls the old `EncounterView` wire legacy; a legacy-only test is not acceptance.
The canonical room-scene branch intentionally bypasses `DungeonShell`; the
integration fixture for this slice must use the atlas branch and reach
`AtlasWalls`.

- Web authoring owns `src/author/dungeonYaml.ts` (`DoorDoc`, strict parse,
deterministic emit, `addDoor`/`updateDoor`) and `src/author/Inspector.tsx`
(`DoorPanel`). `DungeonBuilder.tsx` turns the document into YAML, debounces
`PutDungeon{validate_only}`, and saves the exact YAML. `draftStorage.ts` is
only a best-effort local draft, never shared persistence. The existing picker
pattern is preferred; add a door-appearance picker, not a material editor.
- Toolkit dungeonspec owns the authored `DoorSpec` and compilation. In the
current API pin (`rulebooks/dnd5e/encounter` v0.88.0), `DoorSpec`,
`encounter.DoorInput`, `doorRecord`, and `AtlasDoorway` have no appearance.
Add the neutral optional value through this existing compile/atlas path; do not
put asset knowledge in `dungeonspec` or API.
- API `internal/sessionworld/sessionworld.go` compiles content through toolkit;
`internal/dungeons/registry.go` stores YAML verbatim and the compiled atlas;
`internal/handlers/dnd5e/authoring/v1alpha1/put_dungeon.go` and
`internal/handlers/dnd5e/session/v1alpha1/convert.go` are the conversion seams.
- Proto `dnd5e/api/session/v1alpha1/types.proto` currently has only
`AtlasDoorway.connection/from/to`; add the opaque optional appearance there.
`DoorInfo` remains identity/state/lock only. `DoorChanged` remains state-only;
`DoorRevealed.doorways` must carry the same static appearance metadata.
- Web `useSessionDoors(sessionId, member)` is member-scoped and refetched on
door events. `SessionEncounterView` joins `DoorInfo.door` to
`AtlasDoorway.connection`, then passes both unchanged to `SessionCanvas`.
`handleDoorClick` calls the existing Open/Unlock RPCs and ignores OPEN; no
Close RPC is invented. `AtlasWalls` is the only live target for this slice.

Thus this is broader interface work (authoring YAML → toolkit neutral carry →
proto/API atlas → Web binding), not a Web-only or Assets-only implementation.
The outside-in wave starts with the authored consumer contract and proto, then
API/toolkit transport and finally Web/Assets rendering; merge inside-out after
provider contracts are available.

## Contract and fallback

`doors[i].appearance` is absent or a non-empty opaque reference. The authoring
editor writes it verbatim and reloads it through `GetDungeon`; `PutDungeon`
validation and the compiled `GetAtlas` answer are the same source of truth.
The selected value is joined by door ID, not by position or array index.
Server/session state continues to be keyed by the authored compiled door ID
(`dungeon-key/door-id`). A concealed door's appearance remains hidden until its
normal atlas/door-reveal knowledge arrives.

Web's Assets-owned binding declares, per reference, frame/static roles, leaf
roles, each leaf's rest transform and hinge axis/pivot, closed pose, open angle,
and compatible fit policy. No filename heuristic or duplicate numeric transform
registry is allowed. Cached source scenes/materials are cloned per instance;
one door's fit or tint cannot mutate another.

## Fit and pose

Measured current geometry and route facts are load-bearing:
`atlasWallRuns.ts`'s `GAP_LENGTH` is `DOOR_FRAME_CALIBRATED_WIDTH = 1.0`.
The candidate stable asset is `dnd5e:env:dark-fortress:wall_door_double_01`;
its reviewed provider catalog records bounds about 1.875 × 2.255 × 0.264 m,
and the GLB's named roles are `Door_Frame`, `Door_Left`, `Door_Right`, and
`Door_Wall_Above` under its calibration parent. The two leaf hinge positions
are x=.25 and x=2.25 in the authored geometry: after the renderer's shared
scale .75, hinge span is 1.5 while the mechanical gap is 1.0. Native-size fit
is therefore not evidence of correctness.

The assembly must first derive a closed-pose fit from the reviewed role bounds
and both hinge positions. Keep the frame/header static; place each leaf below
its own fixed hinge root, then apply a rigid Y rotation around that root for
OPEN. Fit translation/scaling belongs below the moving hinge, never on a
non-uniform parent that would deform the swing. Preserve authored height for
this slice: no adjustable header, wall-height/UV system, or vertical-lift
motion. Reuse existing measured fit primitives only after they are generalized
to named multi-leaf roles; do not copy constants from the candidate GLB into a
registry or proto.

Human choice remains explicit: preserve the authoritative 1.0 gap and fit the
candidate's leaf roles to it (recommended, protecting movement/LOS geometry),
or widen the rendered gap to the measured 1.5 hinge span (which changes wall
run geometry and needs a separate acceptance). Do not choose silently in code.

## Delivery and proof

1. Web authoring adds the optional field, picker, strict round-trip tests, and
editor preview; proto adds only the opaque atlas metadata; toolkit compiles and
projects it without interpreting it; API maps it in both PutDungeon/GetAtlas
and DoorRevealed paths.
2. Assets promotes a portable role binding for the stable reference and Web
syncs the exact provider revision. Web adds the atlas-branch assembly and
state-driven rigid leaf presentation, retaining the legacy default/fallback.
3. Test at the real route: two independent doors (old/new styles), persisted
selection through save, reload, and a fresh session; IDs/clicks remain distinct;
server-driven CLOSED, LOCKED, and OPEN states render correctly; unknown and
missing references diagnose/fallback safely; closed geometry fits the 1.0
aperture and OPEN leaves rotate rigidly from measured hinges. Include a browser
proof with old/new side by side, not a local-only toggle demo. Keep legacy
`EncounterView` fixtures separate and non-accepting.

### Open decisions before implementation

- Human chooses 1.0-gap fit (recommended) versus a deliberate 1.5-gap geometry
  wave; this design does not authorize widening the atlas gap.
- Human/Assets approves the final role binding and open-angle convention after
  visual evidence; the existing 90-degree legacy guess is not reused blindly.
- Proto/API/toolkit owners confirm the exact optional field name/presence rules
  and release sequencing; no consumer should invent a parallel local override.
