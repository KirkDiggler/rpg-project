# Single-room play toolkit provider implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Compile the complete approved room into the existing encounter, with faithful persistence and atlas presentation.

**Architecture:** Keep source scene data inline. Convert its declared frame once into encounter/spatial facts. Preserve legacy v2 and use the existing `Compiled` result, not a second game runtime.

**Tech Stack:** Go1.24.1 module, yaml.v3, testify suites, released spatial geometry.

**Spec:** [single-room-play.md](single-room-play.md), approved by Kirk.

## Global constraints

- Work only in nearest module `rulebooks/dnd5e/encounter`, including dungeonspec, on the existing #1753 line. Other modules are read-only. One coherent provider PR; no automatic merge/tag.
- All Go commands below run **from that module directory**, with `GOWORK=off`. No root-level cross-module test/tidy command or committed local replace/go.work.
- Spatial v0.14.0/v0.15.0 already supply placed footprints, traces and sight lanes. Pin released v0.15.0 when the integration needs it. A genuinely missing primitive is a reported prerequisite, not authority to edit another module.
- Complete actual RoomDraft/WorldScene shape, hex actor/start cells, existing monster defaults, document-local portability. No invented stamp/selection schema, duplicate authored poses, room-as-prop anchor or per-hex visual entities.
- New format v3 is distinct from legacy v2 and local/snapshot envelope versions. Explicit false is not missing. No source-only/zero-field value may be called a successful `Compiled` result.
- No doors, cover, height physics, pickup/drop, concealment/intel or multi-room feature work. Preserve existing legacy behavior; new v3 cannot silently bypass observer projection.
- Encounter/compiler cannot import the root rulebook to resolve monster sheets. It checks reference syntax/placement and forwards refs; the existing SDK content loader resolves definitions. Play readiness must resolve those refs before session creation in the later host-seam work.

## Current interfaces and corrections

At toolkit `3a9dbf6e5125e443d8f761c6c2f536556e0bbe8c`:

```go
func Decode(raw []byte) (*Spec, error)       // legacy syntax decode
func Compile(spec *Spec) (Compiled, error) // validated playable v2 field
func Load(raw []byte) (Compiled, error)      // existing host entry
// Compiled already has Field, PartyStart []Seat, StartFacing, Monsters,
// Intel, Factions, Dispositions, Scenarios and Endings. There is no Seats field.
```

Keep `Decode`/`Compile` v2 signatures unchanged. Add the following typed v3
entries; only milestone3 wires `Load`'s version dispatch:

```go
type SingleRoomDecodeInput struct { Source []byte }
type SingleRoomDecodeResult struct { Spec *SingleRoomSpec }
func DecodeSingleRoom(in SingleRoomDecodeInput) (*SingleRoomDecodeResult, error)

type CompileSingleRoomInput struct { Spec *SingleRoomSpec }
func CompileSingleRoom(in CompileSingleRoomInput) (Compiled, error)
```

The earlier draft wrongly mixed root-module commands, possible spatial edits,
`Compiled.Seats`, and a source-only compiled stub. Those directions are withdrawn.

## Task 1: Typed source and strict lossless decode — first executable milestone

**Files:** create encounter `room_scene.go`, `room_scene_test.go`; dungeonspec
`single_room.go`, `single_room_decode.go`, `single_room_decode_test.go`, and
`testdata/world-builder-v3.yaml`. Keep responsibilities separate from the large
legacy files. Do not wire FieldInput, Load, or runtime compilation yet.

**Produces:** encounter-owned typed presentation data and `DecodeSingleRoom`.
The field schema is exactly the approved spec; use JSON/YAML tags with its
actual camelCase source names. Proposed type relationships:

```go
// Each declaration below is a new type, not an existing API.
type SingleRoomSpec struct {
    Version int                 `yaml:"version" json:"version"`
    Key     string              `yaml:"key" json:"key"`
    Play    SingleRoomPlay      `yaml:"play" json:"play"`
    Room    RoomSource          `yaml:"room" json:"room"`
}
type SingleRoomPlay struct {
    Void     string `yaml:"void" json:"void"`
    Lighting string `yaml:"lighting" json:"lighting"`
    Standing string `yaml:"standing" json:"standing"`
}
type RoomSource struct {
    Version         int                          `yaml:"version" json:"version"`
    ID              string                       `yaml:"id" json:"id"`
    Name            string                       `yaml:"name" json:"name"`
    CoordinateFrame encounter.RoomSceneFrame     `yaml:"coordinateFrame" json:"coordinateFrame"`
    Workspace       encounter.RoomSceneWorkspace `yaml:"workspace" json:"workspace"`
    Scene           encounter.RoomVisualScene    `yaml:"scene" json:"scene"`
    Gameplay        RoomGameplaySource           `yaml:"room" json:"room"`
}
```

`RoomSceneFrame` names the five existing coordinateFrame fields;
`RoomSceneWorkspace` holds `HexRadius`/`HorizontalLimit`;
`RoomVisualScene` holds version, id, name, items and groups. Items/groups use the
existing complete shape: kind, ID/label, asset ref on items, X/Y/Z/RotationY
transform, optional parentId, supportId, heightScale and owner pointLight.
Light is enabled/offset/color/intensity/range, **not** a new RGB/light array.
`RoomScenePresentation` contains its version plus frame/workspace/scene, never
monster markers. Use float64 and preserve optionality.

`RoomGameplaySource` contains implicitRegionId, walkableHexes,
propDeclarations, arrangementDeclarations, optional PartyStart and Monsters.
`RoomCell` has integral Q/R; monster source has ID/Ref/Cell only. Declarations
have footprint width/depth/offsetX/offsetZ and presence-aware required movement/
LOS booleans (`*bool` in source decoding or equivalent explicit node presence).
Template declaration keys are not live scene IDs. Do not default missing required
transform coordinates or flags to zero/false while decoding.

- [ ] Copy the approved YAML example verbatim into the fixture. Add a testify
  `SingleRoomSourceSuite`, loading the fixture in SetupTest. Add these load-bearing
  assertions before implementation:

```go
out, err := DecodeSingleRoom(SingleRoomDecodeInput{Source: s.raw})
s.Require().NoError(err)
s.Require().NotNil(out.Spec)
s.Equal(-2.25, out.Spec.Room.Scene.Items[0].Transform.X)
s.Equal(1.5, *out.Spec.Room.Scene.Items[0].HeightScale)
s.False(*out.Spec.Room.Gameplay.PropDeclarations["table"].BlocksLineOfSight)
s.Equal("table", out.Spec.Room.Scene.Items[1].SupportID)
s.Equal("furniture", out.Spec.Room.Scene.Items[1].ParentID)
```

  Add JSON/YAML round trips preserving graph, light, source frame and false flags.
  Table-test unknown nested/top-level keys, second YAML document, duplicate keys,
  missing required coordinate/flag, nonfinite number, invalid height range,
  fractional actor cell, duplicate/cyclic/dangling IDs, invalid ref grammar and
  unsupported versions. Incomplete start/floor readiness belongs to compilation,
  not a fabricated playable result; source decoding must not mutate input bytes.
- [ ] Run `GOWORK=off go test ./dungeonspec -run TestSingleRoomSourceSuite -count=1`;
  expect missing new decoder/types before implementation, then specific refusals.
- [ ] Implement strict source decoding with existing ErrBadSpec/ValidationError
  path reporting, known-field enforcement, one-document semantics and graph/scalar
  validation. Required presence must be checked before zero-value conversion.
- [ ] Run the focused suite, full module tests and required module lint/format/tidy.
  Verify legacy Load still refuses this v3 source (runtime support is not wired).
  Commit the meaningful source checkpoint normally; first working push opens a
  **Draft** PR explicitly listing the remaining three milestones.

## Task 2: Shared placed-footprint facts

**Files:** new `placed_props.go`/tests; modify existing `compilefield.go`,
`cellfacts.go`, `canvas.go`, `step.go`, `clocks.go` only at shared fact/traversal
seams; module go.mod/go.sum pin spatial v0.15.0. No spatial source changes.

**Consumes:** source types and released geometry. **Produces:** proposed
`PlacedPropInput {ID string; Placement spatial.FootprintPlacement;
BlocksMovement, BlocksLineOfSight bool}` on FieldInput, and one field-owned
contributor set read by standing, crossing and sight. Presentation remains a
separate carried value; legacy Props are not repurposed as anchors.

- [ ] Red tests: exact unit/yaw/offset/dimension mapping, centre standing,
  thin interior crossing with both centres clear, independent flags, overlap,
  geometry outside floor mask and unchanged legacy wall/door/prop behavior.
  `GOWORK=off go test . -run TestPlacedPropsSuite -count=1`.
- [ ] Use these existing operations rather than raster/LOS copies:

```go
standing, err := spatial.TraceFootprint(spatial.FootprintTraceInput{
    Placement: placement, From: centre, To: centre,
}) // BlocksMovement && standing.Contact
crossing, err := spatial.TraceFootprint(spatial.FootprintTraceInput{
    Placement: placement, From: fromCentre, To: toCentre,
}) // BlocksMovement && crossing.Interior; propagate errors
```

  Feed existing SightLanes from the same contributor set. Integrate CellAt and
  traversal so Route, Step, Join and Canvas cannot disagree. Frame conversion is
  the exact approved `k=FeetPerCell/sqrt(3)`, negative yaw degrees and D/W swap.
  No new path search, mesh inference, observer bypass or percentage-policy work.
- [ ] Green: focused suite plus existing cellfacts/route/step/sight suites and
  module gates before commit. If the released geometry cannot express a required
  query, report the exact limitation instead of changing another module.

## Task 3: Complete v3 compilation and host dispatch

**Files:** new `dungeonspec/single_room_compile.go`/tests, change `compile.go`'s
Load version dispatch only after the new compile path is complete.

**Consumes:** source decode and FieldInput placed contributors.
**Produces:** existing `Compiled.Field`, **PartyStart**, and Monsters.

- [ ] Red: load the fixture through the existing public entry:

```go
compiled, err := Load(s.raw)
s.Require().NoError(err)
s.Require().NotEmpty(compiled.PartyStart)
s.Require().Len(compiled.Monsters, 1)
s.Equal("dnd5e:monsters:skeleton", compiled.Monsters[0].Ref)
s.Empty(compiled.Monsters[0].Actions) // definition defaults, not new loadout
```

  Add blocked/off-floor starts and monsters, occupied spawn cells, stable ID
  namespace, negative axial coordinates and deterministic nearest free seats.
  Run `GOWORK=off go test ./dungeonspec -run TestSingleRoomCompileSuite -count=1`.
- [ ] Compile one implicit bright region and transparent void, copy presentation,
  build placed contributors, validate final geometry, then derive seats excluding
  blocked/occupied cells. Read actual seatsOf/HexCellAt frame conventions; v3
  input is axial, never pass it through legacy offset conversion by accident.
  Preserve v2 `Load`/`Decode`/`Compile` behavior. Validate reference grammar here;
  leave monster definition loading to SDK. Never return partially ready Compiled.
- [ ] Green: v3 suite plus legacy dungeonspec fixtures/module gates; commit.

## Task 4: Persistence, atlas projection and downstream handoff

**Files:** `data.go`, `atlas.go`, `projection.go`, new focused scene/contributor
round-trip tests and module docs. FieldData/Atlas additions use the same types
introduced above, not parallel wrappers with divergent defaults.

- [ ] Red tests create an encounter through existing package test helpers with
  compiled FieldInput, capture its atlas and representative CellAt/Route/Canvas
  results, call ToData, reload with the required existing supplied capabilities,
  and compare all results. Include source graph/lights/height/floats; independent
  overlap contributors; stale/unsupported saved data; and no mutable aliases.
  Run `GOWORK=off go test . -run TestRoomScenePersistenceSuite -count=1`.
- [ ] Persist canonical inputs and presentation, rebuild derived indexes on load,
  and carry validated presentation on Atlas/AtlasFor. No author monster markers
  in presentation; preserve member-scoped actor knowledge. The new dialect is
  single, unconcealed-region content; do not silently leak a new full-scene payload
  into legacy concealed scenarios. Unsupported combinations are explicit errors.
- [ ] Verify removing one contributor in a newly compiled source retains the other
  (editor edits affect future launches, not a new live-DM protocol). Run
  `GOWORK=off go test ./...`, required lint/format/tidy and module race checks;
  commit and update Draft evidence. One independent PR review when complete.

## Completion and release boundary

Provider acceptance: strict lossless source decode, a complete v3 compiled field,
shared movement/sight facts, valid placements/seats, deep-copy/persistence fidelity,
and correct atlas/member projection, with legacy tests retained. It does not prove
a browser or resolve a monster sheet by importing the root rulebook.

Protos next expose the approved versioned room_scene_json presentation on atlas;
API/session consumers preserve it and preflight definition resolution/capacity;
web adds setup markers, source codecs and shared rendering/Play. Actual character
and monster acting in this same room is final wave acceptance. Follow owning
release rules and real tags; no hand tags, premature merge to unblock work, or
assumption that updating a sibling checkout changes a pinned consumer.
