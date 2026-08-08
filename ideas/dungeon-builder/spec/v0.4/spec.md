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

## 2. Additive transport shapes and capability behavior

v0.4 adds four separable shapes. Each section defines decode/validation,
preservation, authoring projection, runtime transport, and mechanical boundary.

| Tranche | Authored shape | Authoring/runtime transport |
|---|---|---|
| A — region floor | `canvas.floor_source: regions` + `regions[].cells` | resolved source, full `FloorPlan.floor_cells`/pair `edges`/optional entrance; encounter snapshot uses the same mask/edges |
| B — placement offset | optional `offset: [x,y,z]` | preserved on `FloorPlan.placements[]` and authorized runtime placement |
| C — monster config | optional `target`, `profile`, `params` + exact-ref defaults | authored values compile into provider/spawn config; props reject |
| D — lighting | optional root/region `lighting.ambient` | raw declarations plus optional inherited override; authorized runtime hex override |

Capability detection uses the normal authoring path, not a new capability system. The
builder sends the **exact candidate document** with
`AuthoringService.PutDungeon(validate_only=true)`. Unsupported fields or values fail
loudly through the ordinary response and source-path errors. A successful response
returns the projection defined by the relevant section.

Unsupported or not-yet-accepted semantics hard-stop preview/save/run. No layer may
strip fields, substitute bounds floor, resolve opaque behavior in the client, or erase
an offset/lighting override to make a request pass. Client-local conveniences may
compile losslessly to canonical fields only as §8 permits.

## 3. Shared delta rules

1. Decode remains strict, known-field, and single-document. `version:` remains exactly
   `1`.
2. Coordinates remain pointy-top odd-q offset `[column,row]`. Room `place[].at` and
   `boss.at` remain room-local source coordinates; their projection is absolute.
3. The compiler validates and derives the canonical result. The API carries request
   and result fields without reinterpreting them. The web authors values and renders
   authorized projection.
4. Field errors introduced by v0.4 MUST populate existing
   `ValidationError.field` with the canonical source path. Authoring-derived records
   call the same value `source_path`. Placement roots are `place[i]`,
   `rooms[r].place[i]`, or `rooms[r].boss`; errors append the full nested suffix
   (for example `rooms[1].place[0].params.example_key`).
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
placement, target reachability, start/party seating, LoS, reveal,
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
4. every placement, authored edge endpoint, and start is valid against the compiled
   mask/edges.

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
floor invalidates an explicitly authored `start`, placement/monster/boss, or authored
edge. No authored content moves, disappears, becomes root content,
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

## 5. Tranche B — placement `offset`

### 5.1 Shape and applicability

Any room-scoped or canvas top-level `place:` entry, and room `boss:`, may carry:

```yaml
offset: [0.05, 0.0, 0.2]
```

`offset` is exactly three finite numbers `[x,y,z]`. Components are game-world units on
the game's existing world axes and are relative to the placement's canonical compiled
origin. The triple is world-axis-aligned; facing does not rotate or reinterpret it.
Omission means `[0,0,0]` without creating an authored value.

Existing `ref`, `at`, `facing`, `mount`, blocker, placement-location, and boss rules
remain unchanged and apply independently. `offset` does not make an otherwise invalid
`facing` or `mount` combination valid.

### 5.2 Preservation, projection, and mechanics

The compiler/server MUST preserve an authored triple verbatim through source
persistence, authored-content reload, and encounter creation. It performs no pivot,
attachment, snapping, bounding, or asset-specific correction.

Authoring `FloorPlan.placements[]` returns `source_path`, `ref`, compiled absolute
`at`, applicable existing facing/mount fields, and optional authored `offset`.
Runtime returns the same optional triple on the authorized placement in
`HexRecord.contents[]`. Builder and game apply that identical triple to the canonical
placement origin.

`offset` is cosmetic. It MUST NOT change the owning cell, structural floor, collision,
`blocks_movement`, `blocks_los`, pathing, range, targetability, LoS, visibility, fog,
or interaction identity. Authored blocker fields remain the sole prop mechanics.

A client changing `ref` submits a complete edited document. It may retain, change, or
remove `offset` explicitly. There is no server replacement command or stable placement ID. Asset-specific render
behavior, raw matrices/quaternions, and uncommitted nudge state remain client-local.

## 6. Tranche C — provider-defined monster config transport

### 6.1 Source and applicability

Every monster room `place`, canvas `place`, and room `boss` accepts flat optional:

```yaml
target: wounded
profile: cautious
params:
  retreat_threshold: 0
  announce: false
```

| Field | Shape |
|---|---|
| `target` | non-empty string |
| `profile` | non-empty string |
| `params` | map of non-empty string key to JSON scalar bool, string, or finite number |

Null, list, and object/map param values reject. Empty-string **param values** are valid
and preserved; empty target/profile and empty param keys reject. `params: {}` and
omitted params both contribute no keys. Props reject `target`, `profile`, and `params`
at their source paths. Boss has full parity with monster `place`.

Top-level exact-ref defaults use the same shape:

```yaml
defaults:
  'dnd5e:monsters:wolf':
    target: wounded
    profile: pack
    params: { pursue: 6 }
```

Default keys are exact monster refs. Prop refs, families, aliases, prefixes, wildcards,
and unknown refs reject.

Target strings, profile strings, param keys, and their accepted values are opaque
provider-defined vocabulary. v0.4 does not enumerate or interpret them. The compiler
passes authored config to the applicable provider/spawn-config validation and reports
any rejection at the exact source path. The API is a messenger and performs no
behavior validation or calculation.

### 6.2 Deterministic transport merge

Config merges in three layers, lowest to highest:

1. constructor/base config;
2. exact-ref dungeon default;
3. placement or boss config.

For `target` and `profile`, a later present non-empty string replaces the earlier
value. `params` merges key-wise; later present keys replace earlier keys. Omitted
fields/keys leave the earlier layer unchanged. Explicit numeric zero, `false`, and
empty-string param values are present values and MUST survive compile/persistence.
An empty params map clears nothing because it contributes no keys.

The merged opaque config is passed into compile/spawn data and encounter persistence.
Profile and target remain authored opaque values; v0.4 does not require a resolved
behavior preview or invent a separate typed authoring proto union.

### 6.3 Preservation and projection

Each monster/boss `FloorPlan.placements[]` entry echoes:

- `source_path`, exact `ref`, and compiled absolute `at`;
- optional authored `target` and `profile`; and
- authored `params` with keys/JSON-scalar values preserved, including zero/false/
  empty string.

The compiled spawn config carries the three-layer merged `target`, `profile`, and
key-wise params to the monster provider/DataJSON path. Provider validation errors use
`ValidationError.field` with the placement/default source path and nested key suffix.
No behavior fields are silently removed.

Dungeon YAML MUST NOT contain mutable runtime state: current mode, machine state,
knowledge/memory, stimuli, last-seen data, search/lapse counters, decision rationale,
current target entity ID, path, initiative state, patrol state, or other observations.

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
versus crypt rendering) instead of falsely collapsing them to one constant. The compiler resolves authored override inheritance; the API passes the fields
through without reinterpretation.

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
canonical region cells, `walls:`, or placement `offset` before validate-only.

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
starts, encounter snapshot reload, pathing, target selection, placement, LoS, and fog agree
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

### 9.3 Placement offset survives a ref edit

Initial submitted source:

```yaml
- ref: 'dnd5e:props:bookcase'
  at: [3, 2]
  facing: E
  blocks_movement: true
  blocks_los: true
  offset: [0.05, 0.0, 0.0]
```

Client-edited complete source:

```yaml
- ref: 'dnd5e:props:torch-ornate'
  at: [3, 2]
  facing: E
  blocks_movement: false
  blocks_los: false
  offset: [0.05, 0.0, 0.2]
```

There is no replacement call or persisted placement identity. Each document validates
independently. Authoring/runtime return the submitted ref, absolute cell, facing,
blockers, source path, and exact optional triple. The builder/game apply the same
world-axis triple to the canonical origin. The torch's false/false authored blockers
prove offset/model appearance does not become mechanics. A third candidate that
retains `[0.05,0,0]` or omits offset is equally legal: ref changes have no implicit
offset policy.

### 9.4 Behavior transport defaults, placement override, and boss parity

A provider-backed fixture accepts opaque values `target: wounded`, `profile: cautious`,
and fixture params. Acceptance proves:

- constructor/base → exact-ref default → placement/boss order;
- target/profile later-present replacement and params key-wise replacement;
- explicit numeric zero, false, and empty-string param values survive;
- omitted params and `{}` contribute no keys and clear nothing;
- room place, canvas place, and boss echo exact authored fields/source paths and pass
  merged config to their spawn paths;
- props reject, exact-ref defaults do not match another monster, and provider value
  rejection is reported at the exact source path;
- no runtime state appears in YAML.

### 9.5 Lighting inheritance and fog authorization

With root omitted, parent override `0.35` and silent child resolve the child to
present `0.35`; an unrelated silent top-level region resolves to **absent**. With an
authored root `0.6`, a silent unrelated region resolves to present `0.6`. Authoring
projection distinguishes raw presence from optional effective presence. Authorized
visible/remembered runtime hexes carry the override only where resolved; hidden
region/hex data remains absent and authorized no-override hexes omit the field.
Adversarial tests prove override changes cannot alter LoS, visibility, reveal, fog,
or target selection.

## 10. Scope and review state

| In v0.4 | Client-local/not a wire construct | Above v0.4 |
|---|---|---|
| semantic region floor, explicit envelope pairs, draft/run lifecycle, deterministic entrance | `wallLines`, coverage masks, draft markers | authored holes primitive, levels, flat-top orientation |
| optional verbatim placement `offset` | ref-replacement UI, raw transforms, uncommitted nudge state | prefabs/batch layout, arbitrary rotation/coloring |
| opaque target/profile/params transport + exact-ref defaults | behavior calculations and runtime state | authored patrol/vision routes |
| optional root/region ambient override inheritance/projection | renderer baseline/theme selection | per-source lights/environment gameplay |

This document remains **PROPOSED** until a bounded consistency review confirms the
simplified transport shapes and topology rules. There are no remaining non-shape governance gates in v0.4.

**Genuine open spec-shape questions:** none currently identified. Review may still
find a concrete ambiguity; if so, resolve it in this PR before ratification. Do not
flip the status merely because the prior artificial gates were removed.

Relevant evidence: rpg-project PR #202 and
[its platform handoff](https://github.com/KirkDiggler/rpg-project/pull/202#issuecomment-5228001955),
rpg-dnd5e-web [#728](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/728) /
[PR #729](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/729), and delivered
v0.3 Wave 0/1 evidence linked from the README.
