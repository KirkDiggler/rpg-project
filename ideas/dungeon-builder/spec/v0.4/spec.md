# Dungeon YAML Spec v0.4

**Status:** PROPOSED v0.4, 2026-08-08. Not yet ratified.
**Normative delta.** RATIFIED v0.3
([`../v0.3/spec.md`](../v0.3/spec.md)) remains the complete baseline. This file
changes only the constructs and rules it names. Unmentioned v0.3 grammar and behavior
remain normative. Context copied here is for reading continuity, not a claim that this
file restates every v0.3 field.

See [`README.md`](README.md) for evidence, reconciliation, and ratification history.

## 1. Three distinct axes

| Axis | Value | Meaning |
|---|---|---|
| Documentation cut | v0.4 | This proposal/review surface; never a YAML field. |
| Dungeon document version | `version: 1` | Unchanged persisted YAML decode version. |
| Canvas floor source | `canvas.floor_source: bounds \| regions` | Canvas topology only; omission resolves to `bounds`. |

There is no `spec:` field. `spec: "0.4"`, `spec: draft`, and other documentation or
workflow markers MUST fail strict decode. Behavior and environment fields are
additive and independent of canvas floor-source choice.

## 2. Additive tranches, projections, and capability probes

v0.4 contains four independently deliverable tranches because each has a complete
source, validation, persistence, authoring projection, and runtime projection below.
A tranche is not supported until all of those parts and its probe pass.

| Tranche | Stable client capability key | Required authoring projection | Required runtime projection |
|---|---|---|---|
| A — region floor | `authoring.floor_source.regions` | resolved `FloorPlan.floor_source`, `floor_cells`, pair-based `edges`, optional-presence `entrance` | snapshot mask plus authorized owning-hex edges |
| B — prop anchor | `authoring.prop.semantic_anchor` | `FloorPlan.placements[]`: `source_path`, `ref`, absolute `at`, authored `anchor`/`adjustment` | authorized `HexRecord.contents[]` placement entry with the same source facts |
| C — monster behavior | `authoring.monster.behavior_config` | each monster placement's `source_path`, typed `source_behavior`, and typed `resolved_behavior` | resolved targeting/config in monster `DataJSON`; no profile |
| D — region lighting | `authoring.environment.region_lighting` | raw root/region lighting plus root/region `resolved_ambient` | optional resolved ambient on each authorized visible/remembered `HexRecord` |

These keys are builder-side names for existing `AuthoringService.PutDungeon`
`validate_only` probes; they do not add a capability RPC or a document field. The
server MUST keep a tranche probe rejected as `unsupported capability` until that
release includes its validation, persistence, authoring projection, **and runtime
projection**; accepting the probe attests the complete tranche, not decode alone. On
connection and explicit refresh, the builder sends one known-good minimal document
per key, with only that tranche added. A key is accepted only when
`PutDungeonResponse.success` is true **and** the returned `FloorPlan` contains the
exact discriminating projection asserted below:

1. **A:** a one-cell `floor_source: regions` canvas returns that one floor cell, six
   envelope pairs, resolved `floor_source: regions`, and absent entrance.
2. **B:** a legacy room-chain fixture with one deterministic structural solid wall
   and one wall-aligned prop returns the exact `source_path`, ref, absolute cell, and
   anchor/adjustment.
3. **C:** a legacy room-chain fixture uses exact-ref targeting defaults, monster
   `place`, and `boss`, with `params: {}` normalized to no source param entries; it
   returns source paths, typed source/resolved behavior, and distinct derived
   targeting for placement and boss. The key proves the shape, not any
   dynamic profile/knob name.
4. **D:** a legacy canvas with `floor_source` omitted (therefore v0.3 bounds floor)
   and no root override contains one region override, its silent child, and an
   unrelated silent top-level region. Projection returns the override for parent/
   child and absent effective ambient for root/unrelated region.

A transport failure, unsuccessful response, missing field, wrong presence, or wrong
value marks the key unsupported. The builder MUST retain the raw response/error for
display. Dynamic profile names and non-empty `params` are additionally tested by the
**exact candidate document**: before every save/run, the builder sends that exact
source with `validate_only=true`. The candidate response, not the representative
probe, is authoritative for registry values.

Unsupported or not-yet-probed semantics hard-stop preview/save/run. The builder MUST
NOT strip a field, substitute bounds floor, flatten an anchor, resolve behavior
client-side, or erase lighting to make a request pass. This intentionally tightens
the older capability-aware stripping experiment: v0.4 is exact-probe-or-stop.

## 3. Shared delta rules

1. Decode remains strict, known-field, and single-document. `version:` remains exactly
   `1`.
2. Coordinates remain pointy-top odd-q offset `[column,row]`. Room `place[].at` and
   `boss.at` remain room-local source coordinates; their projection is absolute.
3. The toolkit owns compile/validation/rules. The API passes provider truth through;
   it does not derive geometry, behavior, anchors, or inheritance. The web authors
   references/intent and renders authorized truth.
4. Field errors introduced by v0.4 MUST populate existing
   `ValidationError.field` with the canonical source path. Authoring-derived records
   call the same value `source_path`. Placement roots are `place[i]`,
   `rooms[r].place[i]`, or `rooms[r].boss`; errors append the full nested suffix
   (for example `rooms[1].place[0].params.flee_threshold`).
5. All writes are atomic. Failure leaves authored content and registry state
   unchanged.

## 4. Tranche A — region-union structural floor

### 4.1 Canvas discriminator and floor modes

```yaml
canvas:
  width: 12
  height: 8
  floor_source: regions
rooms: []
```

`canvas.floor_source` is optional enum `bounds | regions`:

- omitted or `bounds`: the v0.3 full `[0,width) × [0,height)` rectangle;
- `regions`: the canonical deduplicated union of all `regions[].cells`.

Canvas dimensions remain positive workspace/coordinate-legality bounds in either
case. Under `regions`, an in-bounds cell outside the union is void, not root floor.
The v0.3 topology matrix is unchanged: canvas requires explicit `rooms: []`; non-empty
rooms forbid canvas and regions; regions without canvas reject. Room-chain generated
floor is not affected.

### 4.2 Regions and containment

The v0.3 region source shape remains `id`, optional `name`, optional/inheriting
`archetype`, and required absolute `cells`. Tranche D adds optional `lighting`.
`extent`, polygons, authored parent, and authored derived indices remain invalid.

1. Same-region duplicate cells canonicalize. A single empty region is legal and has
   root parent.
2. Between distinct regions, disjoint or strict containment is legal. Equal cell sets
   and partial overlap reject. **Two empty regions are equal sets and therefore
   reject**; the single-empty exception does not make multiple empty regions valid.
3. Parent is the smallest strict non-empty superset, or root. Empty's parent is root.
   Innermost ownership and parent/index are deterministic derived state.
4. Region boundaries remain semantic and never create interior walls. Archetype
   remains an inheriting label and creates no boss/spawn/content rule.
5. Under `bounds`, unpainted floor is root. Under `regions`, unpainted cells are void;
   root is hierarchy-only.

### 4.3 Canonical mask and mechanics

A canvas compile creates one canonical sorted floor mask from bounds or region union.
All mechanical consumers use that compiled mask: validation, pathing, movement,
placement, targeting/reachability, start/party seating, wall support, LoS, reveal,
fog, encounter initialization, and projection. No subsystem re-derives a rectangle
or union for itself.

`FloorPlan.floor_cells` is the complete authoring mask, sorted ascending
`(column,row)`. Runtime exposes only authorized hexes, but every runtime floor hex
comes from the encounter snapshot's same mask.

### 4.4 Envelope edges reuse the existing pair projection

Every hex side with exactly one floor endpoint and one void/off-canvas endpoint is an
implicit generated solid envelope. This includes the outer boundary and an interior
void boundary. It blocks movement and LoS exactly like other canonical solid edges.

`FloorPlan.edges` reuses the existing flat `{from,to,kind}` pair representation:

1. an envelope pair has exactly one endpoint in `FloorPlan.floor_cells`; that endpoint
   owns the envelope edge;
2. the other endpoint is the adjacent odd-q coordinate, whether an in-canvas void or
   an off-canvas coordinate (negative or at/exceeding a bound);
3. endpoint order is nonsemantic; consumers MUST determine ownership by membership in
   `floor_cells`;
4. `kind` is `solid`; the pair is normalized/deduplicated under the existing
   undirected identity rule;
5. runtime attaches the edge only to the owning floor `HexRecord`. There is no void
   `HexRecord` and no duplicate attachment on another hex.

Interior authored `walls:` retains v0.3 source validation: both endpoints are
adjacent floor cells, duplicates reject, door identity is normalized endpoint-based,
and runtime uses `HexRecord.edges`. No authored envelope door exists because a door
requires floor on both sides.

The existing authoring pair projection therefore needs no open wire decision. A
one-cell region floor has six projected pairs; an interior one-cell void has six
owning-floor pairs around it; an off-canvas side uses its actual adjacent coordinate.

### 4.5 Draft validation, entrance presence, and runnable strictness

`PutDungeon(validate_only=true)` accepts every structurally valid mask, including:

- empty floor;
- non-empty floor too small for `PartyCap`;
- disconnected floor components.

It always returns the canonical floor and envelope. `FloorPlan.entrance` has normative
presence:

- with authored `start`, **present** at that cell only when its ordered PartyCap
  seating envelope lies wholly inside the same floor-cell connected component;
  otherwise absent during validate-only;
- with omitted/null `start`, **present** at the first qualifying anchor in ascending
  `(column,row)` order;
- **absent** when no qualifying anchor exists.

A floor component is defined by hex-cell adjacency in the mask, independent of wall
or door state. Every reserved seat MUST be a member of the anchor component. The
existing deterministic PartyCap seat-order rule is reused; no seed or random fallback
may choose another entrance.

A non-validate write/registration and encounter start are strict. They require:

1. non-empty floor;
2. present resolved entrance and complete PartyCap envelope;
3. every floor cell belongs to the entrance component (connected floor for this cut);
4. every placement, authored edge endpoint, start, and anchor support is valid against
   the compiled mask/edges.

Thus disconnected drafts preview but do not run. An entrance present on one island
does not make a second island runnable. Non-validate failure is specific and atomic.
Legacy bounds/room omitted-start behavior remains v0.3-compatible; these lifecycle
rules apply to `floor_source: regions`.

### 4.6 Candidate edits

A region-cell or canvas-bound edit compiles the complete candidate before mutation.
It recomputes mask, envelope, containment, entrance, PartyCap seating, lighting, and
dependent content. A prior derived entrance/seat envelope is not authored content and
is never an edit dependency: discard it, recompute from the candidate, and permit it
to change. Only the freshly resolved candidate entrance/seats are authoritative for
future authored registrations.

A strict write rejects when the candidate fails §4.5 runnable conditions or removed
floor invalidates an explicitly authored `start`, placement/monster/boss, authored
edge, or prop support. No authored content moves, disappears, becomes root content,
or falls back to bounds. If a region edit leaves the union unchanged (for example,
deleting a nested scope whose cells remain in its parent), semantic properties may
inherit outward. If the union changes and all strict candidate conditions pass,
write/registration replaces atomically. Validate-only may return a structurally valid
but non-runnable candidate as §4.5 defines.

### 4.7 Authored content versus encounter persistence

There are two lifecycles, not two competing floor authorities:

- **Authored content load/write:** YAML is the source. `PutDungeon` non-validate and
  startup/content discovery decode and compile the YAML, then atomically replace the
  authored-dungeon registry state only after the full candidate succeeds. No prior
  compiled floor cache is treated as authoring authority.
- **Running encounter snapshot:** encounter creation copies the exact resolved mask,
  canonical edges, entrance, placements, behavior config, and lighting it started
  with into encounter persistence. Encounter reload uses that snapshot and validates
  its internal consistency (edge ownership/deduplication, entrance/seats/entities on
  snapshot floor, authorized records). It MUST NOT compare against or recompile the
  current authored YAML. Later authored edits do not reshape an in-progress
  encounter.

This gives authored reload current source truth and encounter reload stable started
truth without a source-versus-snapshot disagreement policy.

### 4.8 Tranche A projection

Authoring `FloorPlan` MUST carry:

- resolved `floor_source` (`bounds` or `regions`, including resolved omission);
- full canonical `floor_cells`;
- full canonical flat pair `edges`, including envelope pairs;
- optional-presence `entrance` per §4.5;
- v0.3 region source plus derived parent/innermost projection.

Runtime continues fog-authorized `HexRecord`/zone projection. An envelope is attached
only to its authorized owning floor record. Hidden floor cells, edges, and zones are
not globally exposed.

## 5. Tranche B — semantic prop anchors

### 5.1 Canonical source

`anchor` is props-only and may appear on room or canvas `place:`:

```yaml
- ref: 'dnd5e:props:bookcase'
  at: [4, 3]
  blocks_movement: true
  blocks_los: true
  anchor:
    surface: wall
    edge: E
    support: floor
    orientation: into-cell
    adjustment: { along: 0.05, normal: -0.02, vertical: 0.0 }
```

| Field | Required | Values/meaning |
|---|---|---|
| `anchor.surface` | yes | `floor \| wall`; semantic alignment surface |
| `anchor.edge` | wall only | `E \| NE \| NW \| W \| SW \| SE`, side of compiled absolute `at` |
| `anchor.support` | yes | `floor \| wall`; wall-aligned floor prop versus wall mount |
| `anchor.orientation` | yes | floor: six absolute directions; wall: `into-cell \| into-wall` |
| `anchor.adjustment` | no | optional finite-number `along`, `normal`, `vertical`; each omitted component is zero |

There is no authored `span`, placement stable ID, raw transform, or replacement
operation. The chosen canonical cell+wall edge already identifies the wall span.
`anchor` rejects on monster and boss refs. Legacy v0.3 floor-prop `facing` remains
unchanged when `anchor` is absent; combining `anchor` with legacy `facing` or `mount`
rejects.

### 5.2 Coordinate bases

Adjustment is cosmetic, meters, and unambiguous:

- **floor surface:** `normal` is semantic forward selected by the absolute
  orientation; `along` is semantic right when looking forward; `vertical` is world
  up;
- **wall surface:** `normal` is the chosen wall normal in the orientation direction
  (`into-cell` or `into-wall`); `along` is right when looking along that normal;
  `vertical` is world up.

Only finite numbers are server-valid. The spec does not promote #729's fixture bounds
or calibration offsets. Builder UX may offer catalog-informed bounded controls, but
the submitted canonical adjustment is validated only for shape/finiteness and is
never clamped or silently changed by the platform.

### 5.3 Wall support and mechanics

A wall anchor may target only a seed-invariant canonical solid edge known at compile:

- structural room perimeter;
- a Tranche A envelope; or
- an explicit authored `walls:` solid edge.

It MUST reject a door, missing edge, connector opening, and pattern/procedural wall
whose presence varies with encounter seed. Pattern-generated random walls cannot be
stable authored support even if one happened to appear in a preview.

`surface: floor` requires `support: floor` and forbids `edge`. `surface: wall`
requires `edge` and permits `support: floor` (wall-aligned furniture) or
`support: wall` (mount). The source path error names the failed anchor edge.

Authored `blocks_movement` and `blocks_los` remain the sole mechanical prop truth.
Catalog footprint, bounds, pivot, and light behavior are render/fit information only
and MUST NOT alter floor, traversal, LoS, targetability, or fog.

### 5.4 Catalog and client-owned replacement

The synced `rpg-game-assets` catalog owns model pivot normalization, calibrated
scale, attachment baseline, and model-forward correction. The platform does not load,
version, copy, validate, or project that catalog. Authoring and game web builds MUST
consume the same released catalog bundle for preview/runtime parity and resolve the
same authored ref+anchor locally.

Replacement is a client-local whole-document edit. There is no server replacement
RPC, placement stable ID, or preserve/re-resolve policy in the platform. The editor
may build a candidate that changes `ref` and `support` while retaining other authored
intent, then sends the complete document through validate-only and normal write. The
server validates only the submitted result and never sees the old placement as a
replacement instruction.

Raw GLB pivots, matrices, quaternions, world coordinates, temporary IDs, selection,
and uncommitted nudge/snap state remain client-local.

### 5.5 Tranche B projection

Authoring `FloorPlan.placements[]` returns the authored asset `ref`, compiled absolute
`at`, complete semantic `anchor` including adjustment presence/values, and
`source_path`. Runtime extends the authorized prop entry in `HexRecord.contents[]` with authored
`asset_ref`, complete `anchor`, and `source_path`; the existing `entity_id` remains.
`at` is the containing `HexRecord.position` and is not duplicated on the content
entry. Neither projection
contains a resolved matrix, catalog correction, catalog profile, or server-computed
attachment.

The bookcase/torch parity contract is therefore the authored facts plus the same
released web catalog bundle, not an undefined server attachment wire.

## 6. Tranche C — Phase-1 monster behavior

### 6.1 Source and parity

Flat optional `targeting`, `profile`, and `params` are accepted on every monster room
`place`, canvas `place`, and room `boss`. All three reject on prop refs.

```yaml
defaults:
  'dnd5e:monsters:wolf':
    targeting: lowest-health
    profile: pack-hunter
    params: { pursue_distance: 6 }
```

`defaults` keys are exact registered monster refs only. Prop refs, families, aliases,
prefixes, wildcards, and unknown refs reject. A default accepts only the same three
fields. Boss receives full validation, precedence, compile, persistence, and
projection parity with monster `place`.

`targeting` values are exactly `closest | lowest-health | lowest-ac`. `profile` is a
registered toolkit profile name. `params` is a flat map whose YAML values must map to
JSON scalar bool, string, or finite number. Null, list, and map values reject.
Registry metadata decides per-key expected scalar kind, whether a number must be an
integer, and allowed range/set.

### 6.2 Typed scalar-union authoring wire

The authoring proto MUST NOT use an untyped map or `google.protobuf.Value` for
behavior params. It adds this scalar union (message names are normative; field numbers
are assigned additively by the owning proto change):

```proto
message BehaviorParam {
  string key = 1;
  oneof value {
    bool bool_value = 2;
    string string_value = 3;
    double number_value = 4;
  }
}

message SourceBehaviorConfig {
  optional string targeting = 1;
  optional string profile = 2;
  repeated BehaviorParam params = 3;
}

message ResolvedBehaviorConfig {
  string targeting = 1;
  repeated BehaviorParam params = 2;
}
```

`BehaviorParam.key` is non-empty. Within one repeated list, keys are unique and
emitted in ascending key order; duplicates reject rather than last-write-win. A
`number_value` producer MUST emit a finite number. Registry validation then enforces
that key's scalar kind, integer requirement, and allowed range/set.

Authored `params: {}` and omitted `params` both normalize to zero source
`BehaviorParam` entries: there is no semantic or projected presence distinction for
an empty map. Presence remains load-bearing for actual entries and their zero/false/
empty-string scalar values, and for optional source `targeting`/`profile`.

### 6.3 Toolkit registry and two independent merge chains

Profiles contain `MachineConfig` params only. They MUST NOT contain or change
targeting. The toolkit owns dynamic profile names and param key/type/range rules; this
spec freezes the container/merge semantics, not candidate names or knobs.

Targeting and machine config resolve independently:

**Targeting, lowest to highest:**

1. monster constructor targeting;
2. exact-ref default explicit `targeting`;
3. placement/boss explicit `targeting`.

**MachineConfig params, lowest to highest, per key:**

1. monster constructor MachineConfig;
2. exact-ref default profile params;
3. exact-ref default explicit `params`;
4. placement/boss profile params;
5. placement/boss explicit `params`.

Presence is significant: explicit `closest`, zero, false, and empty string are values
subject to registry validation, never “unset.” Unknown profile/key, wrong scalar
kind, non-finite value, non-integer where required, or out-of-range value fails with
the exact `source_path`.

`profile` compiles out. Runtime/encounter persistence contains resolved targeting and
resolved MachineConfig in monster `DataJSON`, never profile name. Authoring source
retains `profile`. A fresh authored-content load recompiles against the current
registry and atomically replaces registered content; an existing encounter reloads
its started `DataJSON` snapshot without consulting current YAML/profile registry.

There is no fallback brain. A monster unable to produce valid resolved `DataJSON`
rejects before registration/encounter entry.

### 6.4 Derived authoring projection and runtime exclusion

Each monster/boss `FloorPlan.placements[]` entry returns:

- `source_path`, exact `ref`, and absolute `at`;
- optional `source_behavior: SourceBehaviorConfig`, containing optional source
  `targeting`/`profile` plus repeated typed source params (absent when the placement
  authors none; explicit `params: {}` alone also normalizes to absent); and
- required `resolved_behavior: ResolvedBehaviorConfig` after both merge chains.

The same contract applies to room `place`, canvas `place`, and `boss`.
`resolved_behavior` never contains `profile`. Its targeting is constructor/default/
placement resolved, and its params are unique sorted `BehaviorParam` entries. Errors
use the same `source_path` plus nested field/key suffix.

Dungeon source and derived authoring projection MUST NOT include current mode,
machine state, knowledge/memory, stimuli, last-seen data, search/lapse counters,
decision rationale, current target/entity ID, path, initiative state, patrol, facing,
vision cones, or other runtime state. Runtime authorization remains ordinary monster
entity/fog authorization; behavior config does not reveal a hidden monster.

## 7. Tranche D — render-only environment lighting

### 7.1 Source, inheritance, and mechanics boundary

```yaml
lighting: { ambient: 0.6 }
regions:
  - id: shrine
    cells: [[3, 3], [3, 4]]
    lighting: { ambient: 0.35 }
```

Document and region `lighting.ambient` are optional finite floats in `[0,1]`. An
authored value is an absolute normalized renderer-intensity override, not a multiplier
or theme-relative delta. Resolution walks innermost region → ancestors → document
root and selects the nearest **authored** override.

If no scope in that chain authors an override, effective ambient is absent. The game
renderer/theme keeps its current baseline; the platform does not invent or publish a
numeric default. This preserves existing surface differences (for example general
versus crypt rendering) instead of falsely collapsing them to one constant. The
toolkit resolves override inheritance as rules provider; the API passes it through.

Lighting is render-only. It MUST NOT change structural floor, wall/LoS tests,
visibility, senses, reveal, fog authorization, target selection, or pathing. A darker
override never hides an otherwise authorized hex and a brighter override never
reveals one.

Room-chain mode accepts document lighting only. Bounds-canvas unpainted cells use the
root override when authored, otherwise no override. Region-floor cells use the nearest
authored value in their innermost region chain, otherwise no override.

### 7.2 Persistence and exact projection

Authored YAML persists only raw lighting presence/value. Authored-content load
recompiles optional effective overrides. A running encounter snapshot persists each
hex's optional effective override (present or absent) so reload remains internally
stable without consulting current authored YAML.

Authoring projection is exact:

- `FloorPlan.lighting` echoes the raw optional root block;
- optional `FloorPlan.resolved_ambient` is present only when the root authors an
  override;
- each `FloorPlan.regions[]` entry echoes raw optional `lighting` and optional
  `resolved_ambient`, present only when its inheritance chain finds an authored
  override.

Runtime `HexRecord.resolved_ambient` is optional. It is present on an authorized
visible or remembered floor record only when that cell has an effective authored
override; otherwise it is absent and the renderer/theme baseline remains in force.
Hidden cells/regions and their lighting remain absent; there is no global runtime
lighting/region map. The Tranche D probe discriminates support by requiring present
`0.35` on the overridden parent/silent child and absence on the unrelated silent
region, so an older server that omits every field does not falsely pass.

## 8. Client-local compilation boundary

`wallLines`, continuous wall footprints, fractional coverage, draft/raw masks, raw
matrices/world coordinates, temporary IDs, UI selection/gizmos, snap previews, and
uncommitted nudges are client-local. A builder may compile them losslessly into
canonical region cells, `walls:`, or anchors before validate-only.

The platform accepts only canonical YAML and MUST NOT normatively cite or implement a
specific TypeScript algorithm. Fractional coverage is never server standability. If
a local construct cannot compile losslessly, the client hard-stops; it does not
approximate or strip.

## 9. Cross-layer acceptance specimens

### 9.1 Irregular floor with interior void and pair envelope

```yaml
version: 1
key: ring-room
name: 'Ring Room'
canvas: { width: 5, height: 5, floor_source: regions }
rooms: []
regions:
  - id: ring
    cells: [[1,1], [1,2], [1,3], [2,1], [2,3], [3,1], [3,2], [3,3]]
```

Validate-only returns exactly eight sorted cells, resolved `regions` source, present
deterministic entrance at the first PartyCap-capable cell, and explicit solid pair
edges around both outer void and `[2,2]`. Every envelope pair has exactly one floor
owner, including off-canvas cases in an equivalent rim fixture. Non-validate writes,
starts, encounter snapshot reload, pathing, targeting, placement, LoS, and fog agree
on the same mask/edge truth.

Removing a cell under dependent content rejects a write atomically, while
validate-only still projects a structurally valid non-runnable candidate when the
only failure is lifecycle seating/connectivity.

### 9.2 Tiny draft and two islands

A two-cell connected `floor_source: regions` mask is structurally valid:
validate-only succeeds with cells/envelope and **absent** `FloorPlan.entrance`;
non-validate rejects insufficient same-component PartyCap seating.

A discriminating two-island mask contains a four-plus-cell PartyCap-capable island A
and a one-cell island B. Validate-only succeeds, projects both islands and both
envelopes, and may project entrance on A because its seats share A. Non-validate
still rejects because B is outside the entrance component. Removing B makes the same
candidate runnable. This proves entrance presence and whole-floor connectedness are
separate gates.

### 9.3 Wall-aligned bookcase edited to wall torch

Initial submitted source:

```yaml
- ref: 'dnd5e:props:bookcase'
  at: [3, 2]
  blocks_movement: true
  blocks_los: true
  anchor:
    { surface: wall, edge: E, support: floor, orientation: into-cell,
      adjustment: { along: 0.05 } }
```

Client-edited complete source:

```yaml
- ref: 'dnd5e:props:torch-ornate'
  at: [3, 2]
  blocks_movement: false
  blocks_los: false
  anchor:
    { surface: wall, edge: E, support: wall, orientation: into-cell,
      adjustment: { along: 0.05 } }
```

There is no replacement call or persisted identity. Each document independently
validates against the same seed-invariant solid edge. Authoring/runtime return the
submitted ref, absolute cell, anchor, adjustment, and source path. Builder and game
consume the same released catalog bundle, so bookcase and torch each receive their
catalog pivot/scale/attachment/model-forward calibration locally. No fixture matrix,
torch height, or provisional #729 constant enters YAML/server projection. The torch's
false/false authored blockers prove catalog footprint/light does not become mechanics.

### 9.4 Behavior defaults, placement override, and boss parity

A toolkit test registry supplies fixture-local profiles/params; those names do not
become production vocabulary. Acceptance proves:

- targeting constructor → exact-ref default → placement/boss explicit;
- config constructor → default profile → default explicit params → placement/boss
  profile → placement/boss explicit params;
- profile params cannot alter targeting;
- explicit `closest`, numeric zero, false, and empty string survive presence-aware
  merge when valid for their keys;
- room place, canvas place, and boss project exact `source_path`, optional typed
  source behavior, and required typed resolved behavior;
- omitted params and `{}` both emit no source entries; real zero/false/empty-string
  entries survive with the correct oneof arm; duplicate keys/invalid scalar forms/
  registry ranges reject at source path;
- prop fields reject, exact-ref defaults do not match another monster, and runtime
  `DataJSON` contains no profile.

### 9.5 Lighting inheritance and fog authorization

With root omitted, parent override `0.35` and silent child resolve the child to
present `0.35`; an unrelated silent top-level region resolves to **absent**. With an
authored root `0.6`, a silent unrelated region resolves to present `0.6`. Authoring
projection distinguishes raw presence from optional effective presence. Authorized
visible/remembered runtime hexes carry the override only where resolved; hidden
region/hex data remains absent and authorized no-override hexes omit the field.
Adversarial tests prove override changes cannot alter LoS, visibility, reveal, fog,
or target selection.

## 10. Scope and ratification

| In v0.4 | Client-local/not a wire construct | Above v0.4 |
|---|---|---|
| semantic region floor, explicit envelope pairs, draft/run lifecycle, deterministic entrance | `wallLines`, coverage masks, draft markers | authored holes primitive, levels, flat-top orientation |
| prop semantic anchor/adjustment and exact source projection | replacement command/ID, raw transforms, catalog calibration data | prefabs/batch layout, arbitrary rotation/coloring |
| targeting/profile/params/defaults shape and compile | behavior calculations | runtime AI state, patrol/vision cones |
| root/region ambient inheritance/projection | renderer calibration | per-source lights/environment gameplay |

The broad authored-playable-scene direction and all wire/lifecycle choices above are
made for this proposal. Exactly two ratification points remain:

1. **Asset calibration release gate:** the `rpg-game-assets` owner must define and
   evidence the multi-asset/multi-wall calibration coverage, adjustment UX bounds,
   and released catalog bundle consumed identically by builder and game. No catalog
   schema or numeric constant is added to the server contract.
2. **Initial behavior registry contents:** the monster-behavior owner must ratify the
   first shipped profile names and param keys/types/integer/ranges. This spec already
   fixes their typed scalar-union wire, validation, merge, persistence, and projection
   semantics;
   vocabulary growth does not require a YAML shape change.

Relevant evidence: rpg-project PR #202 and
[its platform handoff](https://github.com/KirkDiggler/rpg-project/pull/202#issuecomment-5228001955),
rpg-dnd5e-web [#728](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/728) /
[PR #729](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/729), and delivered
v0.3 Wave 0/1 evidence linked from the README.
