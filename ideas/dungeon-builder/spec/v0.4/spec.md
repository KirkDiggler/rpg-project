# Dungeon YAML Spec v0.4

**Status:** PROPOSED v0.4, 2026-08-08. Not yet ratified.
**Normative.** This is the proposed next contract cut over RATIFIED v0.3
([`../v0.3/spec.md`](../v0.3/spec.md)). It is restated as a standalone contract:
an implementer MUST NOT need to diff v0.3 to discover v0.4 behavior. See
[`README.md`](README.md) for evidence, rationale, reconciliation, and remaining
ratification points.

## 1. What “v0.4” means

Three different axes MUST remain distinct:

| Axis | Value in this document | Purpose |
|---|---|---|
| Documentation spec level | v0.4 | Names this proposed contract and review cut; it is not a YAML field. |
| Dungeon document version | `version: 1` | Gates the persisted YAML document shape; unchanged from v0.3. |
| Canvas structural-floor source | `canvas.floor_source: bounds \| regions` | Selects canvas topology only; omission preserves the legacy bounds rectangle. |

There is no `spec:` YAML field. A server MUST reject it as unknown. A builder MUST
NOT send `spec:`, `spec: draft`, or another documentation-level marker. “Draft” is
client workflow state, not dungeon content (§12).

v0.4 is one broad authored-scene cut split into independently implementable
tranches. A provider MAY deliver the tranches separately, but it MUST reject an
unsupported canonical field rather than accept and discard it.

| Tranche | Canonical source | Provider responsibilities | Independent of |
|---|---|---|---|
| A — topology | `canvas.floor_source`, `regions[].cells`, `walls`, `start` | canonical floor mask, envelope, validation, persistence/reload, projection, mechanics/fog | B, C, D |
| B — prop composition | prop `anchor` | semantic anchor validation, asset-catalog resolution, canonical placement persistence/projection | A's `regions` mode (works on legacy bounds and room chains) |
| C — monster behavior | `defaults`, placement/boss `targeting`/`profile`/`params` | registry validation, presence-aware compile, runtime config materialization | A, B, D and floor-source choice |
| D — environment | document/region `lighting` | provider-resolved inheritance, persistence, authoring projection, fog-authorized runtime projection | B, C; document lighting also works without regions |

Tranche ordering does not create semantic coupling. In particular, behavior fields
are legal on every supported monster placement location whether canvas floor comes
from `bounds`, `regions`, or a room chain.

## 2. Ground rules and rejection behavior

1. Decode is strict and single-document. Unknown fields MUST fail the document;
   multi-document YAML MUST be rejected.
2. `version:` MUST equal `1`. Any other value MUST be rejected.
3. Canonical cell coordinates are absolute pointy-top odd-q offset
   `[column,row]`, unless a room-scoped `at` is explicitly called room-local.
4. The toolkit/provider owns validation, compilation, derived geometry, rule
   registries, and runtime materialization. The API orchestrates and projects
   provider results; it MUST NOT independently derive floor, anchors, inheritance,
   behavior, or placement legality. The web sends canonical authored intent and
   renders authorized provider truth; it MUST NOT send rule calculations.
5. An unsupported canonical semantic MUST hard-stop save/run. No layer MAY make a
   document appear compatible by stripping `canvas.floor_source`, `regions`,
   `anchor`, monster behavior, or `lighting`, or by replacing the canonical floor
   with canvas bounds. Client-local compilation is permitted only where §12 says so.
6. Validation errors MUST identify the source field path and offending value. An
   edit/write is atomic: a failure leaves the prior persisted dungeon unchanged.
7. Presence is data. Omitted values, explicit zero/false values, and explicitly
   supplied enum values MUST remain distinguishable wherever this specification
   defines inheritance or override behavior.

## 3. Document metadata and topology modes

```yaml
version: 1
key: shrine-hall
name: 'The Shrine Hall'
theme: crypt
height: 8
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `version` | int | yes | exactly `1` |
| `key` | string | yes | `[a-z0-9-]+`; MUST match the request key |
| `name` | string | yes | non-empty |
| `theme` | string | no | cosmetic label |
| `height` | int | decode-optional | room-chain mode requires `>= 4`; canvas mode ignores it because `canvas.height` is authoritative |

A document uses exactly one topology mode:

- **room-chain mode:** non-empty `rooms:` and no `canvas:`;
- **canvas mode:** `canvas:` present and `rooms: []`.

Canvas with non-empty rooms, canvas with omitted `rooms`, or empty/omitted rooms
without canvas MUST reject. `regions:` requires canvas mode; room-chain plus regions
MUST reject.

### 3.1 Room chain, connectors, and room content

```yaml
rooms:
  - id: antechamber
    archetype: entrance
    width: 6
    place:
      - { ref: 'dnd5e:props:brazier', at: [1, 1] }
  - id: vault
    archetype: boss
    width: 7
    boss: { ref: 'dnd5e:monsters:skeleton-captain', at: [4, 3] }
connectors:
  - { from: antechamber, to: vault }
```

1. `rooms:` MUST contain at least two entries. Room start columns are derived as
   `start_column[i] = start_column[i-1] + width[i-1] + 1`; the extra column is the
   connector gap.
2. Exactly one room MUST have `archetype: boss` and declare `boss:`. No other room
   may declare `boss:`. Region archetypes never participate in this rule.
3. `connectors:` MUST contain `len(rooms)-1` entries in room order. Each entry's
   `from`/`to` MUST name the corresponding adjacent room pair. Optional legacy
   `locked: {dc, ability}` retains v0.3 behavior.
4. Room-scoped `place[].at` and `boss.at` are room-local and compile to absolute
   cells by adding `start_column`. Their row MUST NOT be the room door row.
5. `obstacles[]` remains `{ref, count}` with `count >= 1`.
6. Room-chain generated perimeter/connector geometry and omitted-start behavior are
   unchanged by Tranche A. Tranches B–D add only their stated fields.

### 3.2 Canvas and the floor-source discriminator

```yaml
canvas:
  width: 12
  height: 8
  floor_source: regions
rooms: []
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `canvas.width` | positive int | yes | columns `[0,width)` |
| `canvas.height` | positive int | yes | rows `[0,height)` |
| `canvas.floor_source` | enum | no | `bounds \| regions`; omission means `bounds` |

`floor_source` is semantic and canvas-local. It is not a document version and is
not affected by the documentation label “v0.4.”

- **`bounds` or omitted (legacy):** structural floor is every in-bounds cell,
  `[0,width) × [0,height)`. Regions are semantic scopes over that rectangle;
  unpainted cells belong to root. Existing v0.3 documents therefore keep their
  meaning without migration.
- **`regions`:** structural floor is the canonical union of all
  `regions[].cells`. Canvas dimensions are workspace/coordinate-legality bounds
  only. An in-bounds cell outside the union is void, not root floor.

In canvas mode the entire room-only validation cluster MUST be skipped together:
document `height >= 4`, minimum rooms, connector count, boss cardinality, boss
axis, the room M1 monster/boss restriction, and boss-ref validation. Top-level
`place:` is accepted only in canvas mode; top-level `boss:` does not exist.

## 4. Tranche A — canonical topology

### 4.1 Regions and structural floor

```yaml
regions:
  - id: crypt
    name: 'Outer Crypt'
    archetype: chamber
    cells: [[1, 1], [1, 2], [2, 1], [2, 2]]
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `id` | string | yes | unique in document |
| `name` | string | no | display label; omitted uses `id` |
| `archetype` | string | no | `entrance \| chamber \| corridor \| boss`; explicit empty/unknown rejects |
| `cells` | `[[int,int]]` | yes | absolute, in canvas bounds; duplicates canonicalize; empty/disconnected legal |
| `lighting` | object | no | Tranche D (§10) |

`extent`, `parent`, polygons, and authored derived indices are not accepted region
shape. Region source cells are canonicalized to unique ascending `(column,row)`
pairs.

Containment remains semantic:

1. Disjoint sets are siblings. A strict subset is a child of the smallest strict
   superset. Equal or partial overlap rejects because no unique innermost owner
   exists. Empty regions are root children and own no cell.
2. Region boundaries do not create walls. An interior transition is open unless a
   canonical `walls:` edge independently blocks it.
3. Each property inherits independently, innermost to root. `archetype` is only a
   label: zero, one, or many regions of every archetype are runnable and it creates
   no boss entity, placement rule, or spawn behavior.
4. Under `floor_source: bounds`, unpainted floor belongs to root. Under
   `floor_source: regions`, unpainted cells are void and root is hierarchy-only.
5. Parent relations and the innermost cell index are derived, never authored.
   Persistence retains irreducible region facts and recomputes these relations.

### 4.2 One authoritative floor mask

For canvas mode the provider MUST compile exactly one canonical structural-floor
mask:

- from all bounded cells for `bounds`/omission; or
- from the deduplicated region-cell union for `regions`.

The compiled `FloorPlan` floor-cell set is the sole mechanical floor truth. The
provider MUST persist that exact canonical mask with the compiled dungeon and
reload it without substituting canvas dimensions or independently reinterpreting
regions. Recompilation deterministically replaces the mask atomically from source.
On reload the provider MUST verify the persisted mask against a fresh deterministic
source derivation and fail on disagreement rather than select whichever copy is
convenient.

Every downstream use MUST query this same mask: static validation, persisted-load
validation, pathfinding, movement, start/party seating, prop/monster/boss placement,
wall endpoint/support checks, range and target reachability, line of sight, fog
reveal/occlusion, encounter initialization, and authoring/runtime projection. A
second rectangle-derived or region-derived floor helper in one of those paths is a
contract violation even when it agrees on a fixture.

### 4.3 Envelope and canonical edges

Every side with structural floor on exactly one side and void/off-canvas on the
other is an implicit generated solid **envelope edge**. This includes outer
perimeters and the boundary of an interior void. The envelope blocks movement and
line of sight and participates in prop-anchor support checks.

Envelope edges MUST be explicitly projected by the provider in both authoring
`FloorPlan` and fog-authorized runtime `HexRecord.edges`. Clients MUST NOT derive
an envelope from floor membership. The canonical edge representation MUST identify
the owning floor cell and side/adjacent coordinate even where the other side is
void. Shared interior edges remain normalized and deduplicated.

Authored source remains edge-native:

```yaml
walls:
  - { from: [3, 2], to: [4, 2], kind: solid }
  - { from: [5, 2], to: [6, 2], kind: door }
```

1. `from` and `to` are distinct, hex-adjacent structural-floor cells. Identity is
   the normalized undirected pair; duplicate/reversed duplicates reject.
2. `kind` is `solid | door`. A lock, if present, is legal only on a door and has one
   or more `{ability, dc}` options with `dc` in `1..30`.
3. Authored edges may replace a colliding generated non-connector interior edge;
   connector collisions reject. An envelope side is not authorable through the
   two-floor-endpoint grammar because its other side is void.
4. Door identity derives from dungeon key plus normalized physical edge, never a
   region ID. Doors open between two floor cells; there is no door through an
   envelope into void.
5. Runtime wall truth remains canonical per-hex edges. A flat parallel wall list or
   client-derived wall truth MUST NOT be introduced.

### 4.4 Validate-only drafts, runnable floor, and edits

A `PutDungeon(validate_only=true)` request with `floor_source: regions` MAY have an
empty union. It returns a valid empty `FloorPlan` with no entrance and reports
ordinary field-path errors for any `start`, placement, or authored edge aimed at
non-floor. This exception exists only for the live authoring loop.

A non-validate write, encounter initialization, or other runnable compile MUST
reject an empty structural floor with `canvas.floor_source: regions has no runnable
floor`. It MUST NOT persist or run an empty dungeon. Empty individual regions
remain legal when the overall union is non-empty.

A region-cell edit is applied as a candidate document:

1. recompute the candidate union, envelope, containment, environment, start, and all
   dependent validation;
2. if removed floor would orphan `start`, an edge endpoint, a prop/monster/boss,
   an anchor support edge, party seating, or another floor-dependent fact, reject
   the whole edit with every relevant field path (or a deterministic first error);
3. never delete/move content, reinterpret it as root content, or fall back to the
   canvas rectangle;
4. if the union changes without invalidating content, accept atomically and project
   the changed mask/envelope; if the union is unchanged (for example deleting a
   nested region fully covered by its parent), semantic properties may inherit from
   the next enclosing scope.

Growing canvas bounds is accepted if all other rules hold. Shrinking below any
region cell or dependent content rejects specifically and atomically.

### 4.5 Start on an irregular floor

```yaml
start: [1, 2] # absolute, optional/null
```

An authored start MUST be in the canonical floor mask and satisfy the existing
ordered `PartyCap` reservation rules before blockers are generated. The existing
ordered PartyCap reservation (normal default `PartyCap = 4`) is rooted at that anchor and reserved for every seed; insufficient seats produce a
requested-versus-available error, never partial seating or a new anchor.

For `floor_source: regions`, omitted/null `start` is deterministic and seed-free:
consider floor cells in ascending `(column,row)` order and select the first cell for
which the toolkit's canonical ordered `PartyCap` envelope succeeds. If none
succeeds, a runnable compile rejects clearly. Validate-only on an empty union leaves
entrance absent. `FloorPlan.entrance`, compiled persistence, reload, and the first
real player position MUST agree.

Legacy `bounds` canvas and room-chain omitted-start behavior remains v0.3-compatible.

## 5. Common placements

A `place:` entry has `ref` and `at`. `blocks_movement` and `blocks_los` are optional
props-only fields and MUST reject on monster refs. Room-scoped `at` is local before
compile; top-level canvas `at` is absolute. Every compiled placement stores an
absolute canonical floor cell.

The v0.3 `facing: E|NE|NW|W|SW|SE` field remains accepted only on a floor prop when
`anchor` is absent. It is a legacy shorthand for a floor-surface anchor with the
same orientation (§6). `mount: floor` MUST remain accepted as the legacy no-op/default when `anchor` is
absent; `mount: wall` MUST reject with guidance to use `anchor`. `facing`/`mount` and
`anchor` MUST NOT be combined. Monster/boss placement fields are specified in §8.

No top-level `boss:` exists. A room `boss:` gets full monster-behavior parity but
retains room-chain boss-cardinality rules.

## 6. Tranche B — semantic prop composition

### 6.1 Canonical encoding

`anchor` is accepted only on a prop `place:` entry:

```yaml
place:
  - ref: 'dnd5e:props:bookcase'
    at: [4, 3]
    anchor:
      surface: wall
      edge: E
      span: center
      support: floor
      orientation: into-cell
      adjustment: { along: 0.05, normal: -0.02, vertical: 0.0 }
```

| Field | Type | Required | Semantics |
|---|---|---|---|
| `at` | cell | yes | canonical support/owning floor cell |
| `anchor.surface` | enum | yes | `floor \| wall`; semantic alignment surface |
| `anchor.edge` | direction | wall only | `E \| NE \| NW \| W \| SW \| SE` side of `at` |
| `anchor.span` | enum | wall only | `center` in v0.4; center of the named edge span |
| `anchor.support` | enum | yes | `floor \| wall`; what physically supports this placement |
| `anchor.orientation` | enum | yes | floor surface: six absolute directions; wall surface: `into-cell \| into-wall` |
| `anchor.adjustment.along` | finite float | no | meters in anchor-local tangent/right basis; default `0` |
| `anchor.adjustment.normal` | finite float | no | meters from the anchor surface toward semantic front; default `0` |
| `anchor.adjustment.vertical` | finite float | no | meters above resolved attachment baseline; default `0` |

Rules:

1. `surface: floor` requires `support: floor` and forbids `edge`/`span`.
   `surface: wall` requires a canonical solid edge at `at`+`edge`, requires
   `span: center`, and permits `support: floor` (wall-aligned bookcase) or
   `support: wall` (wall-mounted torch). A door edge is not mountable.
2. Wall `into-cell` means semantic model front points from the named wall into the
   owning floor cell; `into-wall` is the reverse. Floor orientations use the same
   pointy-top direction vocabulary as legacy `facing`. The asset's raw model-forward
   axis is never assumed to equal this semantic direction.
3. Adjustment is cosmetic and anchor-local. It MUST NOT affect the owning cell,
   collision, movement, LoS, targetability, range, fog, or wall identity. The asset
   catalog supplies allowed bounds for the selected asset/attachment; an absent
   bound means only exact zero is accepted. Out-of-range replacement rejects rather
   than clamps or drops authored adjustment.
4. The canonical source and persistence retain `at` plus the semantic `anchor`
   fields and their presence. Compiled placement retains the resolved catalog
   profile/facts needed to render. A raw matrix or world coordinate is transient
   client output, not compiled persistence or wire truth, and MUST NOT replace the
   semantic source.
5. `anchor` on a monster or `boss`, unknown surface/support/orientation values,
   missing wall edge/span, incompatible asset attachment, and non-finite adjustments
   reject at the field path.

### 6.2 Asset catalog ownership and replacement

The authoritative asset catalog owns, per asset and supported attachment profile:

- model/variant reference and geometry;
- raw pivot normalization and calibrated render scale;
- attachment baseline (including vertical and wall-clearance offsets);
- semantic model-forward correction;
- supported surface/support combinations and adjustment bounds;
- footprint and cosmetic/light behavior.

A placement owns only canonical cell plus authored surface/edge/span/support/
orientation intent and optional local adjustment. Compile resolves catalog facts;
a renderer MUST NOT invent a second correction table.

Replacement is an atomic semantic document edit. It preserves `at`, wall edge/span,
orientation intent, and adjustment, while changing `ref` and, when the author chose
a different physical relationship, `support`. It then re-resolves every catalog fact
for the new asset. It MUST NOT preserve an old asset's matrix, pivot correction,
scale, attachment height, wall clearance, or model-forward correction. If the new
asset cannot satisfy the resulting semantic anchor/adjustment, replacement rejects
without mutating source.

The bookcase-to-torch example therefore changes `support: floor` to
`support: wall` while preserving the wall edge/span, `into-cell`, and local
adjustment. The catalog — not this spec — supplies both assets' distinct baselines.

The exact catalog home, calibration workflow, and per-asset values remain
ratification/implementation work (§14). Fixture measurements from web PR #729 MUST
NOT be copied as production constants.

## 7. Prop authoring projection

Authoring `FloorPlan` MUST echo canonical placement cell and semantic anchor fields
and also expose the provider-resolved catalog profile/facts or an explicit unresolved
catalog error. Runtime placement projection MUST carry the same resolved local facts
(or an immutable catalog profile reference/version) needed for all clients to render
the same result. It MUST NOT carry a raw matrix or world coordinate. Clients derive
the transient world transform from canonical hex geometry plus provider facts and
MUST NOT recalibrate pivot/scale/forward corrections from raw GLBs independently.

Runtime projections MAY omit editor-only semantic metadata not needed to render, but
persistence and authoring reload MUST retain it losslessly. Fog/visibility rules for
whether a prop is exposed remain unchanged.

## 8. Tranche C — Phase-1 monster behavior

### 8.1 Source shape and placement parity

The following fields are flat optional siblings of `ref`/`at` on every monster
`place:` entry (room-scoped or top-level canvas) and on room `boss:`:

```yaml
- ref: 'dnd5e:monsters:wolf'
  at: [5, 3]
  targeting: closest
  profile: cautious
  params:
    flee_threshold: 0
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `targeting` | enum | no | `closest \| lowest-health \| lowest-ac` |
| `profile` | string | no | registered toolkit profile name |
| `params` | map string → scalar | no | every key/type/range validated by toolkit registry; null/object/list values reject |

All three fields MUST reject on prop refs with a monster-only field-path error. They
have identical acceptance and compile behavior for room `place`, canvas `place`, and
`boss`; only the surrounding topology/boss rules differ.

Dungeon-wide exact-ref defaults use:

```yaml
defaults:
  'dnd5e:monsters:wolf':
    targeting: lowest-health
    profile: pack-hunter
    params:
      pursue_distance: 6
```

`defaults` keys MUST be exact registered `dnd5e:monsters:*` refs. Prop refs,
prefixes, patterns, tags, families, aliases, and unknown refs reject. A default
object accepts only `targeting`, `profile`, and `params`. Defaults apply only when
the placement's exact ref matches.

### 8.2 Registry ownership and compile

The toolkit owns a dynamic behavior registry containing:

- the currently supported profile names and each profile's optional targeting plus
  parameter bundle; and
- parameter keys with scalar type, allowed range/set, and compile mapping.

This spec freezes the shape, not candidate profile names or knob names. Names shown
in examples are illustrative and are not required registry entries. Unknown profile,
unknown parameter, wrong scalar type, non-finite number, or out-of-range value MUST
fail validation using the registry active for that compile. The client may discover
registry metadata for UI, but it MUST NOT define or calculate behavior.

Compilation is presence-aware and applies layers in exactly this lowest-to-highest
order:

1. monster constructor configuration;
2. exact-ref default `profile` bundle;
3. exact-ref default explicit `params`, then explicit `targeting`;
4. placement `profile` bundle;
5. placement explicit `params`, then explicit `targeting`.

A later present value wins per key. Explicit zero/false and explicit
`targeting: closest` are values, not absence. A placement profile may therefore
replace a value from the defaults layer; placement explicit values replace both.
Omitted values leave the lower layer intact.

`profile` MUST compile out. The spawn/runtime monster persists only resolved
`targeting` and the validated machine configuration/params in `DataJSON`; runtime
code MUST NOT look up an authored profile name. Authoring source retains the profile
for editing, while compiled persistence retains the resolved configuration so reload
does not change merely because a later toolkit release changes registry contents.
A deliberate source recompile uses the then-active registry and replaces compiled
state atomically.

An absent behavior block preserves constructor behavior exactly. There is no fallback
brain: a monster that cannot produce valid resolved `DataJSON` rejects before it
enters an encounter.

### 8.3 Explicit runtime exclusion

Phase 1 authoring includes only `targeting`, `profile`, `params`, and exact-ref
`defaults`. It MUST NOT persist or decode current mode, state-machine state,
knowledge/memory, perceived stimuli, last-seen data, search/lapse counters,
decision rationale, current target/entity ID, path, initiative state, patrol,
facing/vision cones, or any other runtime observation. Runtime state remains owned by
the toolkit encounter/monster persistence path, never dungeon source or defaults.

## 9. Behavior persistence and projection

Authoring projection MUST echo the canonical source behavior fields with presence
preserved and SHOULD expose the resolved configuration for preview/debug as
provider-owned derived data. It MUST NOT claim a profile survives into runtime.
Runtime monster projection continues through resolved `DataJSON` and ordinary
fog/entity authorization; dungeon behavior config does not grant a client hidden
monster or decision state.

Behavior fields are additive and topology-neutral. The same defaults/override
specimens MUST produce the same resolved config in room-chain, legacy bounds canvas,
and region-floor canvas modes.

## 10. Tranche D — environment and lighting

```yaml
lighting:
  ambient: 0.8
regions:
  - id: shrine
    cells: [[3, 3], [3, 4]]
    lighting: { ambient: 0.35 }
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `lighting.ambient` | float | no | finite, `0..1`; document/root declaration |
| `regions[].lighting.ambient` | float | no | finite, `0..1`; inheriting scope declaration |

The toolkit environment provider resolves lighting. A region silent on a property
inherits the first present value walking innermost → outward → document root. If the
document root is also absent, the provider supplies its named baseline; a renderer
MUST NOT choose a local fallback. Each property resolves independently. v0.4 has
only `ambient`; per-source lighting is out of scope.

In room-chain mode, document lighting applies to all floor because rooms do not gain
a lighting field. In either canvas floor-source mode, region inheritance follows the
same containment graph; under `regions`, every floor cell has at least one declared
region owner, while under `bounds` unpainted root cells resolve directly from root.

Persistence MUST retain authored lighting presence/value and the provider identity/
version plus resolved root/region environment snapshot used by the compiled dungeon.
Reload uses that snapshot, not a renderer default and not whatever baseline a newer
provider happens to have. A deliberate source recompile may resolve against the
current provider and atomically replace the snapshot.

Authoring `FloorPlan` MUST expose authored presence and provider-resolved ambient for
root and every region. Runtime MUST expose only the resolved ambient needed for a
fog-authorized visible/remembered cell (directly on the hex or through its authorized
zone chain). It MUST NOT send a global hidden region/environment map or reveal a
hidden region merely because it overrides lighting.

## 11. Cross-layer persistence and downgrade contract

| Fact | Authored persistence | Compiled persistence | Authoring projection | Runtime projection |
|---|---|---|---|---|
| Canvas bounds/floor source | exact, presence preserved | source discriminator + canonical floor mask | bounds, discriminator, explicit cells | fog-authorized floor cells |
| Regions | canonical source cells/properties | derived containment/index as rebuildable cache only | source + derived parent/resolved props | authorized innermost zone + needed ancestors |
| Envelope/walls | authored walls only | canonical generated+authored edges | explicit canonical edges | authorized `HexRecord.edges` |
| Prop composition | cell + semantic anchor | catalog resolution + source link | semantic + resolved attachment | resolved attachment when prop authorized |
| Monster behavior | defaults and placement fields | resolved targeting/config; no profile | source + optional resolved preview | ordinary authorized monster state; no source profile |
| Lighting | authored presence/value | provider/version + resolved snapshot | authored + resolved root/regions | resolved value only where fog-authorized |

Source and compiled forms serve different purposes; neither authorizes duplicate
mechanical derivations. A reload validates their required agreement and fails loudly
on corrupt/missing load-bearing compiled truth.

A client/provider capability mismatch is not a migration path. If a target cannot
accept any canonical semantic used by the document, validation/save/run stops. It
MUST NOT silently send a v0.3 rectangle, flatten a semantic anchor to coordinates,
materialize a profile name client-side, or erase a lighting override.

## 12. Client-local authoring concerns

The following are authoring-tool implementation state, never platform YAML/proto/API
constructs:

- `wallLines`, continuous wall footprints, fractional coverage, and draft masks;
- raw matrices/quaternions, raw GLB pivots, world coordinates, and renderer-axis
  corrections;
- uncommitted UI nudge state, gizmo state, selection/hover state, snap previews,
  temporary IDs, and fixture slot IDs;
- a documentation-spec badge, `draft` marker, target-dialect version, or raw
  authoring controls.

An authoring client may compile these into canonical `walls`, region cells, and prop
anchors before a request. That compilation is client-owned and non-normative to the
platform: the platform accepts and validates only the canonical result and MUST NOT
cite or depend on a particular TypeScript module/algorithm. Fractional coverage is
not a server traversability value. If a client-local construct cannot be represented
losslessly as canonical floor/edges/placements, the client MUST hard-stop and show
the gap; it MUST NOT approximate or strip it.

A UI nudge becomes canonical only when the author commits it as bounded
`anchor.adjustment`; until then it remains client-local.

## 13. Cross-layer acceptance specimens

### 13.1 Irregular floor with an interior void

```yaml
version: 1
key: ring-room
name: 'Ring Room'
canvas: { width: 5, height: 5, floor_source: regions }
rooms: []
regions:
  - id: ring
    archetype: chamber
    cells:
      - [1, 1]
      - [1, 2]
      - [1, 3]
      - [2, 1]
      - [2, 3] # [2,2] deliberately void
      - [3, 1]
      - [3, 2]
      - [3, 3]
```

Acceptance:

- validate-only compiles exactly those eight sorted cells; with no blockers, the
  runnable fixture resolves omitted start deterministically to `[1,1]` and reserves
  the PartyCap envelope;
- `[2,2]` and every other unclaimed in-bounds cell fail placement/path/target-floor
  checks exactly like off-canvas void;
- explicit envelope edges surround both the outside perimeter and the `[2,2]` void
  in authoring/runtime truth; pathing, LoS, fog, persistence, and reload agree;
- deleting a cell under dependent content rejects atomically rather than moving that
  content to root or widening floor to bounds;
- the same source with all region cells removed passes validate-only with no entrance
  but fails non-validate write/run.

### 13.2 Wall-aligned bookcase → wall-mounted torch

Before:

```yaml
- ref: 'dnd5e:props:bookcase'
  at: [3, 2]
  anchor:
    { surface: wall, edge: E, span: center, support: floor,
      orientation: into-cell, adjustment: { along: 0.05 } }
```

After one semantic replace action:

```yaml
- ref: 'dnd5e:props:torch-ornate'
  at: [3, 2]
  anchor:
    { surface: wall, edge: E, span: center, support: wall,
      orientation: into-cell, adjustment: { along: 0.05 } }
```

Acceptance: cell, edge/span, orientation, and committed local adjustment persist;
support changes by authored replacement intent; catalog pivot/scale/attachment/
model-forward facts refresh; neighbors do not change; no bookcase matrix or
fixture-local torch height survives; persistence/reload and every renderer produce
the provider-resolved torch attachment. An incompatible catalog/bound rejects the
whole replacement.

### 13.3 Behavior defaults and overrides

```yaml
defaults:
  'dnd5e:monsters:wolf':
    targeting: lowest-health
    profile: pack-hunter
    params: { pursue_distance: 6 }
place:
  - { ref: 'dnd5e:monsters:wolf', at: [1, 1] }
  - ref: 'dnd5e:monsters:wolf'
    at: [1, 2]
    profile: cautious
    params: { pursue_distance: 0 }
    targeting: closest
  - { ref: 'dnd5e:monsters:skeleton', at: [1, 3], targeting: lowest-ac }
```

The acceptance harness MUST register fixture-local profiles `pack-hunter`/`cautious`
and parameter `pursue_distance` with discriminating values/ranges. That fixture
registration makes the YAML executable without freezing those names into the
production registry. Acceptance:

- wolf 1 resolves constructor → default profile → explicit default param/targeting;
- wolf 2 then applies placement profile and explicit zero/`closest`, proving
  presence-aware precedence;
- skeleton receives no wolf defaults because matching is exact-ref only;
- the same entries compile identically in room `place`, canvas `place`, and `boss`
  locations (subject only to their surrounding topology rules); props reject all
  three behavior fields; persisted runtime config contains no profile name.

### 13.4 Lighting inheritance and fog

A document root `ambient: 0.8`, parent region override `0.35`, and silent child MUST
resolve child to `0.35`; an unrelated silent top-level region resolves to `0.8`.
With root omitted, all silent scopes resolve the provider baseline and persist the
provider/version snapshot. Authoring projection shows all resolved values; a runtime
viewer sees values only for authorized cells/zone ancestors, never the hidden
lighting override as a map leak.

## 14. Ratification points and above-v0.4 scope

The broad authored-playable-scene direction is approved. These concrete integration
points remain for ratification before the header changes to RATIFIED:

1. **Envelope wire representation:** confirm the published authoring edge projection
   can express one-sided floor/void edges; add an additive canonical side form if it
   cannot. Client derivation is not an alternative.
2. **Prop catalog authority and calibration:** choose the owning repository/API,
   provider versioning, attachment-profile schema, and multi-asset/multi-wall
   calibration gate. Do not import PR #729's provisional numeric corrections.
3. **Adjustment bounds:** confirm catalog-resolved per-attachment bounds (with zero-only
   when absent) rather than freezing the fixture's ± values.
4. **Registry discovery/provenance:** name how builders query the toolkit behavior
   profile/knob registry and how compiled persistence records its provider version;
   profile/knob vocabulary itself remains dynamic.
5. **Environment provider/projection:** name the absent-root baseline provider and the
   additive authoring/runtime fields carrying resolved ambient; renderer-local default
   is not acceptable.
6. **Tranche capability rollout:** name the capability response used by a builder to
   enable each independent tranche. Regardless of mechanism, unsupported semantics
   hard-stop and are never stripped.

Explicitly above v0.4: holes as an authored primitive (irregular void is omission
from region union), authored `end`, flat-top orientation, per-source lights,
behavior runtime state/patrol/vision, prop prefabs/batch layouts/coloring, arbitrary
rotation, raw transforms, multi-level geometry, lock behavior, and a top-level boss.

Relevant trackers/evidence: rpg-project #175–#192, #200–#203; monster behavior PR
#202 and its platform handoff comment; rpg-dnd5e-web #728/PR #729.
