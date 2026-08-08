# Visual Anchor Metadata — canonical asset calibration and one shared resolver

**Status:** Design proposed for Kirk review. This is the first design-only revision for
[rpg-project#204](https://github.com/KirkDiggler/rpg-project/issues/204). No
`plan.md` or implementation issue exists yet.

North star: **a model's intrinsic visual registration is produced once with the
asset, while every scene says where that registered model belongs. Builder and game
must reach the same transform through the same pure resolver.**

The initial proof is intentionally narrow:

- center the concrete `SM_Prop_Bookcase_Small_01` bookcase variant on its authored
  span using intrinsic metadata;
- register the concrete `SM_Prop_Torch_Ornate_01` variant to an authored wall plane
  using intrinsic metadata;
- do **not** make the fixture's torch height or Kirk's `-0.20m` trim intrinsic; and
- after [rpg-game-assets#43](https://github.com/KirkDiggler/rpg-game-assets/issues/43),
  verify the fighter's standing/downed variants on one logical hex with no downed
  anchor correction in the web.

This document decides the production boundary and contract. It deliberately does
not sequence implementation work.

## Evidence and the boundary it earned

The accepted Learn slices are
[rpg-dnd5e-web#728/#729](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/729)
and
[#731/#732](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/732).
They used actual synced GLBs, all six canonical asset facings, and the shared tactical Play camera; they did not exercise all six authored wall-edge orientations.
The production design inherits these observed facts:

| Evidence | Production conclusion |
| --- | --- |
| The small bookcase's rendered bounds center is `(+0.927,+1.257,+0.330)m` from its raw origin after canonical `0.75` render scale. Its authored hex/span was already correct. | `(+0.927,0,+0.330)m` is an intrinsic floor-contact point for **that concrete variant**. Align that point to the authored floor/span target; do not move the logical hex. |
| The ornate torch is centered in X/Z but needs its model-owned back surface registered to a wall. The lab's nominal-plane result used a rendered back point near `(0,-0.032,-0.076)m`. | The back attachment point is intrinsic. The wall target, height, clearance, and trim are placement inputs. |
| A `1.15m` visible torch mount line worked in one fixture; Kirk also preferred a signed fixture Z trim of `-0.20m`. | Both remain provisional scene choices. Neither number appears as a catalog default. |
| The fighter's downed bounds are entirely displaced on negative local Z while standing is centered. | This is a reproducible export defect owned by `rpg-game-assets#43`, not a legitimate downed web anchor. |
| Visible spill and the current bookcase `footprintHexes: 2` did not agree cleanly. | Visual registration says nothing about gameplay occupancy. The footprint judgment remains separate. |

The earlier composition Learn also proved the replacement rule: preserve stable
placement identity, span/wall intent, facing, and authored adjustment; resolve the
new concrete variant and refresh its intrinsic asset facts. Preserving an old
variant's complete XYZ matrix made a wall torch inherit a floor bookcase's
assumptions and is rejected.

## Locked ownership model

There are exactly two transform owners and one separate gameplay owner.

### 1. Asset-owned: intrinsic visual calibration

`rpg-game-assets` owns facts that remain true anywhere the same generated model is
rendered:

- the distributable model identity and path;
- canonical visual scale;
- correction from the source model's forward axis into the canonical `+Z`-forward
  frame;
- a measured floor-contact or wall-attachment point for a concrete generated
  variant; and
- companion meshes that inherit the same resolved transform.

Raw licensed GLBs and all other source Synty/licensed files are immutable inputs.
Calibration never patches them in place. Generated/promoted outputs may change only
through the reproducible asset pipeline.

Wrong root wrappers, axes, scale, grounding, or generated-pose centering are not
"calibration." They are re-export defects. They remain asset-pipeline fixes even
when a runtime translation would look correct.

### 2. Placement-owned: authored scene intent

A placement owns where and how an asset is used:

- logical owning hex, wall, span, or scene anchor;
- facing or the wall's room-inward normal;
- mount height;
- wall clearance/embed;
- bounded local nudge; and
- stable identity across replace.

These values survive replacement where their semantics still apply. The new
variant's asset calibration never survives by copying the old variant's matrix; it
is re-resolved.

### 3. Gameplay-owned: occupancy and rules

Occupied hexes, collision, movement blocking, pathfinding, LoS, cover, and a true
multi-hex footprint are gameplay/authoring facts. They do not enter the visual
resolver and cannot be derived from visible bounds or visual-anchor metadata.

An asset catalog may continue to expose measured dimensions for asset QA, but those
measurements are not gameplay authority. This design does not ratify the existing
bookcase `footprintHexes: 2`, change any proto/toolkit rule, or create a client-side
placement-legality rule.

## Canonical coordinate and transform contract

All consumer-visible translations use **meters** in a right-handed Three.js-style
frame:

- `+X`: model right;
- `+Y`: up;
- `+Z`: model forward, and for a wall-mounted item, away from the wall into the
  room after facing is resolved.

Vectors are homogeneous **column vectors**, and matrices multiply them on the left.
For angle `θ` in radians, positive yaw follows Three.js's right-handed Y rotation:

```text
R_y(θ) = | cosθ  0  sinθ  0 |
         |   0   1    0   0 |
         |-sinθ  0  cosθ  0 |
         |   0   0    0   1 |
```

Thus `R_y(θ)` maps local `+Z` to world `(sinθ,0,cosθ)`. A world facing vector
`f=(fx,0,fz)` therefore has `θ=atan2(fx,fz)`. All facing and inward vectors are
finite and normalized before resolution; a zero-length direction is invalid.

The catalog's model points are expressed in the **canonical rendered local frame**:
after the variant's total GLB-to-rendered-meter scale and producer-confirmed source
yaw, but before intrinsic anchor translation, placement facing, or world placement.
This is why the bookcase point is `0.927m`, not an unscaled accessor coordinate.

A calibration is bound to the tuple:

```text
(distributable GLB digest, total scale, source-forward yaw, concrete variant id)
```

Changing any member invalidates the measured anchor until the producer explicitly
reconfirms or recalibrates it.

### Source-forward yaw is narrow asset normalization

`sourceForwardYawRad` is required on every enrolled safe-catalog entry, including an
explicit evidence-backed `0`. It is radians in the `R_y` sign convention above and
may express only a legitimate source-family convention proven against the exact
promoted output and representative siblings. It cannot contain placement facing,
wall orientation, staging pose, camera compensation, or a scene-specific rotation.

A malformed promoted Root/axis is a re-export defect. The producer rejects a yaw
entry if the evidence does not distinguish a coherent source convention from a
broken export. A model-specific yaw guessed in the web is forbidden.

### Tagged anchor semantics — no generic offset bucket

A concrete variant has either no intrinsic anchor or exactly one tagged anchor:

```text
none
  Correct-origin/legacy behavior. The canonical model origin is used.

floor-contact
  modelPointM: { x, z }
  The declared horizontal model point is aligned to the placement's floor/span
  target. The generated model must already be grounded at Y=0; this tag cannot
  carry a Y correction and therefore cannot hide a grounding defect.

wall-attachment
  modelPointM: { x, y, z }
  The declared point on the model's attachment/back surface is aligned to an
  authored wall target at the placement-owned mount height and clearance.
  The point describes model geometry; it does not choose the target height.
```

For matrix use, `floor-contact {x,z}` expands to `a=(x,0,z)` and
`wall-attachment` uses its full `a=(x,y,z)`. No anchor uses `a=(0,0,0)`.
The tag determines required context; it is not an advisory label.

For the initial proof, the expected calibrated facts are:

| Concrete variant | Tag | Intrinsic model point | Explicit exclusions |
| --- | --- | --- | --- |
| `SM_Prop_Bookcase_Small_01` | `floor-contact` | `{x:+0.927, z:+0.330}m` | no wall trim; no footprint change |
| `SM_Prop_Torch_Ornate_01` | `wall-attachment` | measured back/attachment point, approximately `{x:0, y:-0.032, z:-0.076}m`, producer-confirmed against the generated GLB | no default mount height; no `-0.20m` trim |
| `fighter-downed` | none and **not enrolled in the v1 catalog** | none | #43 must correct the generated asset; no diagnostic lab translation ships |

The torch's point explains the Learn arithmetic without stealing placement intent:
aligning its `y≈-0.032m` model point to a scene-selected `1.15m` target produces an
origin translation near `+1.182m`; aligning its `z≈-0.076m` back point to the
fixture's nominal `-0.866m` plane produces an origin translation near `-0.790m`.
Only the model point is intrinsic. Both target values belong to the placement/wall
context.

## Canonical metadata and generated catalog

### Current scale/catalog reality and the chosen authority

Today there is no single scale authority:

- `harness/models/synty/props/manifest.json` records a global
  `calibration.SYNTY_SCALE = 0.75` but not a total scale per concrete prop;
- the public web's hand-maintained `propManifest.ts` adds `renderScale = 2` for
  rug variants, so their effective scale is `0.75 × 2 = 1.5`; and
- character wrappers independently apply `SYNTY_SCALE = 0.75` outside the prop
  manifest.

The future authority for every **enrolled** concrete variant is one safe-catalog
field, `totalScale`, a dimensionless uniform GLB-local-unit to rendered-meter
multiplier. It already includes all global and per-variant visual scale factors.
The placement matrix applies it exactly once. Existing global constants and web
`renderScale` overrides are migrated/cross-checked when a variant enrolls and are
disabled for that resolved-matrix path. A component may not multiply another scale
on top.

V1 enrolls only the prop variants needed by the proof:

- `SM_Prop_Bookcase_Small_01` under `dnd5e:props:bookcase`; and
- `SM_Prop_Torch_Ornate_01` under `dnd5e:props:torch-ornate`.

All non-enrolled props remain on the existing `propManifest.ts`/`PropModel` path
during transition. A single shared adapter partitions by catalog enrollment: an
enrolled family must use selector + placement matrix, and a non-enrolled family must
use the legacy resolver. A build guard rejects overlap or a second scale/transform
application for an enrolled variant. Thus two populations coexist temporarily, but
no concrete asset has two authorities or two placement paths.

[rpg-dnd5e-web#624](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/624)
remains the owner of eliminating the broader prop/class/wall hand-maintained mirrors.
This design supplies its safe-catalog seam but does not migrate rugs, walls,
characters, or the rest of the prop catalog. The post-#43 downed verification uses
the existing character resolver/wrapper and its current `0.75` scale; v1 creates no
character safe-catalog entry.

### Private producer sources

The following responsibilities are distinct:

1. `library/visual-anchor-calibrations.json` in private `rpg-game-assets` is the
   reviewed declaration of intrinsic calibration intent. It is keyed by stable
   concrete variant id and records tag/model point, evidence-backed source yaw,
   measurement provenance, and the exact GLB/total-scale digest it was accepted
   against.
2. Existing generated inventory such as
   `harness/models/synty/props/manifest.json` establishes promoted GLBs, semantic
   refs, paths, companions, and the current `0.75` calibration input. Any existing
   web-only scale override must be migrated into or explicitly reconciled by the
   producer before that variant can enroll.
3. A deterministic rpg-game-assets projection produces
   `harness/catalogs/synty-web-assets.json`. **This generated file is the canonical
   consumer contract for enrolled variants.** It joins promoted inventory,
   selection defaults, total scale, and approved anchors, and refuses stale or
   invalid declarations.

The reviewed declarations are inputs, not browser data. The generated catalog is
canonical because it proves them against the actual promoted output and flattens all
inheritance before distribution.

### License-safe public projection

The safe catalog contains only what selector and renderer need:

```text
schemaVersion
catalogRevision                 # digest of normalized consumer entries
producerRevision                # exact rpg-game-assets revision
families by semantic reference:
  defaultVariantId              # explicit stable default
  enrolled variant ids
entries by stable variant id:
  semantic reference key
  concrete variant id
  distributable relative model path
  staged-asset digest
  totalScale                    # sole dimensionless scale authority
  sourceForwardYawRad
  optional tagged visual anchor
  optional anchor-kind-local Builder UX limit hints
  optional companion variant/path references
```

It excludes raw source paths, licensed source files, vertex/index data, embedded
textures/materials, conversion details, private pack inventory, review notes,
measurement captures, and gameplay footprint/collision semantics.

The byte-identical safe projection is checked into public web as
`src/generated/synty-web-assets.json` through the authenticated sync/bake boundary.
A static build imports it; the browser does not fetch the private repository or
measure a GLB. Numeric calibration, model URLs, digests, and screenshots are
license-safe evidence. Raw licensed GLBs remain private; shipped GLBs continue to
enter builds through the approved private asset bake rather than public git.

### Two separate executable contracts

Selection and placement are distinct pure contracts:

```text
selectVisualVariant(catalog, semanticRef, explicitVariantId?)
  -> selected(entry) | selectionFailure(reason)

resolveVisualPlacement(selectedEntry, registration)
  -> resolved(matrix) | placementFallback(matrix, reason)
```

Neither function loads a model. Both builder and game call the same selector before
the same placement resolver. Keeping them separate makes a missing default a catalog
selection failure rather than a mysterious transform fallback.

#### V1 selection policy: semantic ref to stable producer default

V1 takes the no-new-proto path: existing semantic refs remain the persisted/wire
identity. Each enrolled family declares an explicit stable `defaultVariantId`.
The shared selector uses it; array order has no meaning.

- Reordering catalog entries is behaviorally inert.
- A legacy semantic-ref-only placement selects the family's current declared
  default.
- Replacement to a new semantic family resolves that family's declared default and
  refreshes path/scale/yaw/anchor/companions **atomically** with the semantic ref.
  If the new family/default cannot select, replacement is rejected and the old
  placement remains unchanged.
- A future author-selected visual variant must persist an explicit `variantId`
  through a separately approved authoring/persistence contract. It is not smuggled
  into local component state in v1.
- When that future input exists, a known explicit id must belong to the named family
  and wins over its default. An unknown/removed explicit id never silently falls
  back to a new default, because that would rewrite saved author intent; selection
  returns `unknown-explicit-variant` and uses the shared missing-model fallback.
- A known family with a missing/non-member default is an invalid catalog and a hard
  producer/sync/build failure, not ordinary runtime absence.

Anchors remain per concrete entry. No family-level anchor inheritance is permitted.
`SM_Prop_Bookcase_Small_01` cannot calibrate `_01`/`_02` siblings merely because all
share `dnd5e:props:bookcase`; `SM_Prop_Torch_Ornate_02` remains legacy/non-enrolled
until separately measured. Producer-side deduplication may reuse one reviewed
record, but the safe catalog is flattened.

A correct, intentional standing/downed/color/pose variant may eventually have its
own anchor only when repeated evidence proves a legitimate difference. Broken
output is repaired first.

### End-to-end data flow

```text
PRIVATE rpg-game-assets producer
  immutable inputs + promoted GLBs + inventory + reviewed declarations/defaults
                    |
                    | deterministic validation/projection
                    v
canonical safe catalog + exact producer revision + staged GLBs
                    |
                    | atomic authenticated sync/bake; verify every staged digest
                    v
public checked-in catalog + runtime GLBs from that same producer revision
                    |
                    | static import
                    v
ONE pure selectVisualVariant
                    |
                    v
ONE pure resolveVisualPlacement
                    |
             +------+------+
             |             |
             v             v
  builder/author view   live game renderer
             |             |
             +------ same selected entry and matrix ------+
```

| Variant | Producer/catalog fact | Selector result | Placement input | Both consumers observe |
| --- | --- | --- | --- | --- |
| Small bookcase | explicit bookcase default -> concrete id; digest, total scale, yaw, floor point | stable default independent of array order | authored span/floor target, facing, nudge | visible center registered; logical hex/footprint unchanged |
| Ornate torch 01 | explicit torch default -> concrete id; digest, total scale, yaw, back point | stable default independent of array order | wall plane/inward, scene height, clearance/nudge | back point registered; height/trim stay caller data |
| Downed fighter | no v1 catalog entry | existing character resolver, outside selector | ordinary entity placement after #43 | corrected raw output shares standing origin; no resolver exception |

## One pure shared placement resolver

The resolver has no React, R3F, loader, GLB, repository, hex-pathfinding,
semantic-ref selection, or asset-identity knowledge. It consumes one selected valid
entry and one generic registration produced by the authored-semantics adapter. It
returns the one model matrix and a reason.

```text
resolveVisualPlacement(entry, registration)
  -> { matrix, outcome: applied | no-anchor | registration-mismatch |
                        missing-wall-context }
```

The shared selector runs first; the placement resolver never chooses a variant.
Builder and Game adapt the persisted semantic placement through the exact same
adapter and resolver.

### Matrix definitions

`T(q)` translates by 3-vector `q`. `S(s)` is uniform scale by sole
`totalScale=s`. `ψ=sourceForwardYawRad`. Vertices are homogeneous column vectors.
An intrinsic model point `a` is already in canonical rendered meters after
`R_y(ψ)S(s)`, so `T(-a)` sits to the left of yaw/scale.

#### Floor surface + floor support

The adapter supplies:

```text
p_c: world center of authored `at` on the floor datum
φ: yaw selected by six-direction `anchor.orientation`
d_f=(adjustment.along, adjustment.vertical, adjustment.normal)
```

Here `along` is faced local right, `normal` is faced local forward, and `vertical`
is world up. For matching `floor-contact a=(ax,0,az)`:

```text
M_floor_floor(a) = T(p_c) · R_y(φ) · T(d_f) · T(-a)
                 · R_y(ψ) · S(s)
```

`p_c.y` is authored floor elevation, never a measured model bound. Positive normal
moves forward; positive along moves right; positive vertical moves up.

#### Canonical structural wall frame

Both valid wall-surface combinations use the chosen edge's seed-invariant structural
frame. Let `p_e` be that edge midpoint at the owning cell's floor datum. Let
`i=(ix,0,iz)` be the finite normalized **structural room-inward** direction and
`u=(0,1,0)`. Define `t=u×i=(iz,0,-ix)`; then `t×u=i`. The right-handed basis is:

```text
B(i) = | tx  0  ix  0 |
       |  0  1   0  0 |
       | tz  0  iz  0 |
       |  0  0   0  1 |
```

For persisted adjustment:

```text
d_w=(adjustment.along, adjustment.vertical, adjustment.normal)
```

`along` is positive structural tangent `t`; `vertical` is positive world up;
`normal` is **always positive structural room-inward `i`**, regardless of model
orientation. This gives wall clearance/embed one stable sign.

`anchor.orientation` changes model-facing only:

```text
δ = 0   for into-cell
δ = π   for into-wall
```

`R_y(δ)` is inside the adjustment frame, so `into-wall` turns the model 180° around
the registered point without reversing along/normal/vertical signs.

#### Wall surface + floor support (bookcase shape)

This combination requires a `floor-contact` model point. The support target is the
wall-edge midpoint on the floor datum; the prop remains floor-supported while using
the wall frame:

```text
M_wall_floor(a_floor) = T(p_e) · B(i) · T(d_w) · R_y(δ)
                      · T(-a_floor) · R_y(ψ) · S(s)
```

With omitted adjustment, a calibrated bookcase's declared floor point lands at the
edge span/floor datum. Normal adjustment moves it into/out of the room without
changing its intrinsic visible-center correction.

#### Wall surface + wall support (torch shape)

This combination requires a `wall-attachment` point `a_wall=(ax,ay,az)`:

```text
M_wall_wall(a_wall) = T(p_e) · B(i) · T(d_w) · R_y(δ)
                    · T(-a_wall) · R_y(ψ) · S(s)
```

For wall support, `adjustment.vertical` is the semantic target height of the declared
attachment point above the edge's floor datum. It is authored placement intent, not
an asset baseline. `adjustment.normal` is authored clearance relative to the
structural wall plane. Omission means zero; the catalog supplies neither value.

The resolver never measures a wall GLB or chooses a visible-face plane. A scene that
wants a particular height, clearance, or embed authors the adjustment.

### Exact identity-anchor fallbacks

Identity means `a=(0,0,0)`; it retains valid total scale, producer yaw, semantic
orientation, and authored adjustment:

```text
M_floor_floor_identity = T(p_c) · R_y(φ) · T(d_f) · R_y(ψ) · S(s)
M_wall_floor_identity  = T(p_e) · B(i) · T(d_w) · R_y(δ) · R_y(ψ) · S(s)
M_wall_wall_identity   = T(p_e) · B(i) · T(d_w) · R_y(δ) · R_y(ψ) · S(s)
```

- A valid selected entry with no anchor uses its registration's identity matrix and
  returns `no-anchor`.
- A model-point tag incompatible with the valid semantic combination uses that
  combination's identity matrix and returns `registration-mismatch`; it never
  reinterprets a point.
- If a wall edge cannot produce finite `p_e` and normalized `i`, the adapter uses the
  required owning-`at` floor fallback with the corresponding floor identity matrix
  and returns `missing-wall-context`. Platform validation should already reject the
  invalid semantic support; this runtime fallback is deterministic defense.

Fallback is not calibration evidence and cannot be saved back as metadata.

### Sole-transform invariant

The returned matrix is the **only** transform applied to the GLB primitive and every
companion. The component disables/omits current outer `position`, `rotationY`,
`SYNTY_SCALE`, and `renderScale` for an enrolled entry. Companions receive the
identical matrix. Any additional component position/rotation/scale is a parity and
scale failure.

### Replacement behavior

Replacement is a client-local whole-document edit, matching proposed v0.4. It keeps
semantic `at`, edge/orientation/support intent, and compatible local adjustment only
when the author chooses to preserve them; it changes `ref`, `surface/support`, and
vertical/other adjustment as authored. On the submitted complete document, the
selector resolves the new family's stable default and refreshes path, total scale,
yaw, model point, and companions. No old resolved XYZ matrix or old asset fact is
preserved, and there is no server replacement command/stable placement identity.

## Generation, atomic delivery, drift, and version behavior

### Deterministic producer gates

A valid producer run guarantees:

- canonical ordering/numeric formatting is byte-identical on two consecutive runs;
- refs, variant ids, defaults, paths, and companions are unique and internally
  consistent;
- every family default exists and belongs to that family;
- `totalScale` is finite/positive and reconciles all current global/per-variant
  scale inputs exactly once;
- source yaw is finite radians and carries exact-output/sibling evidence;
- anchor values are finite meters, within sanity bounds, valid for their tag, and
  checked against build-time bounds of the exact promoted output;
- floor-contact outputs are already grounded and cannot carry vertical correction;
- every entry binds asset digest + total scale + yaw + concrete id; changing one
  without explicit recalibration/affirmation fails stale;
- only the two v1 prop variants/families are enrolled; and
- normalized safe output contains only the public allowlist.

GLB inspection is allowed **only in the private producer/gate**. It produces
metadata; it is never a browser dependency.

### Catalog and runtime GLBs are one atomic release unit

The authenticated sync/deployment bake stages, in a temporary destination, all of:

1. the safe catalog;
2. the exact enrolled GLBs and companions it names; and
3. the producer revision recorded by the catalog.

Before publish, it hashes the **actual staged runtime files** and requires every hash
to equal the catalog entry, requires the checked-in public catalog bytes/revision to
match the producer projection, and requires the staged asset checkout to equal
`producerRevision`. Only the verified set replaces the prior runtime set. Failure
leaves the prior catalog+GLBs intact; a new catalog can never publish alongside old
GLBs, or vice versa.

Unsupported schema, invalid entries/numbers/defaults, duplicate identity, total-scale
conflict, unproven yaw, producer-revision mismatch, safe-catalog drift, and staged
asset digest mismatch are **hard producer/sync/build failures**. They do not become
identity-anchor fallbacks and cannot publish. If corrupted catalog data somehow
reaches a browser, selection fails closed for the whole enrolled catalog because
its path/scale/yaw cannot be trusted.

### Exhaustive selection/resolution/render outcomes

Builder and game share this outcome policy as well as the pure functions:

| Condition | Deterministic result |
| --- | --- |
| Semantic ref belongs to a non-enrolled v1 family | Use the one legacy adapter/path. It may not consult the enrolled matrix path. |
| Enrolled family + valid declared default, no explicit id (v1/legacy semantic ref) | Selector returns exactly `defaultVariantId`, independent of array order. |
| Future explicit id exists and belongs to family | Selector returns that exact entry; default is ignored. |
| Future explicit id is unknown, removed, or belongs to another family | `unknown-explicit-variant`; do **not** substitute default. Render the shared missing-asset visual at the logical placement. |
| Enrolled family default missing, unknown, or from another family | Hard invalid-catalog producer/sync/build failure; no publish. If encountered at runtime, catalog fails closed and renders the shared missing-asset visual, not an untrusted matrix. |
| Valid selected entry + matching anchor/registration | Use exact `M_floor_floor`, `M_wall_floor`, or `M_wall_wall`. |
| Valid selected entry + no anchor | Use the selected semantic combination's exact identity matrix; outcome `no-anchor`. |
| Anchor tag and supplied complete registration kind disagree | Use that supplied registration's exact identity matrix; outcome `registration-mismatch`. |
| Wall intent lacks finite edge point or normalized structural inward | Use owning-`at` `M_floor_floor_identity`; outcome `missing-wall-context`. |
| Unsupported schema, invalid entry, producer/digest mismatch | Hard failure before publish. Runtime corruption fails closed: no catalog entry/matrix is trusted; shared missing-asset visual only. |
| Selected path passes catalog gates but the GLB later fails HTTP/decode/load | Do not try another variant and do not change placement. Both consumers render the same shared missing-asset visual at the logical/fallback floor frame and report `model-load-failure`; retry/cache policy remains loader-owned. |
| Replacement target family/default cannot select | Reject replacement atomically; old semantic ref, selected entry, and rendered matrix remain unchanged. |
| Anchor is removed in a valid new catalog revision | The still-selected entry deterministically uses its identity matrix and reports `no-anchor`; no stale web copy survives. |

The shared missing-asset visual is a generic non-GLB placeholder at the logical
owning floor/span target. It applies no catalog scale/yaw/anchor and has no gameplay
meaning. This replaces divergent builder/game guesses; it is not a second model
resolver.

### Versioning and visual changes

- `schemaVersion` is an integer contract major. Tagged-union meaning, axes, units,
  scale semantics, selector semantics, or matrix order changes require a new major.
- `catalogRevision` is a deterministic content digest. Entry/default/path/scale/yaw/
  anchor changes update it.
- Anchor/default/scale changes are visible content changes, not schema changes. They
  require producer evidence, selector/resolver parity tests, and renewed visual
  acceptance for affected variants.
- V1 documents persist semantic refs, not selected matrices. They deterministically
  take the catalog's stable default on each revision. Future explicit variant ids,
  once separately approved, never silently retarget when removed.
- Scene documents store placement intent, not resolved matrices. A new valid catalog
  re-resolves that intent. Pixel-stable replay may record catalog revision as
  provenance later, but v1 does not fork behavior by revision.

## Five-way routing table

| Finding | Diagnostic test | Owner and artifact | Must not become |
| --- | --- | --- | --- |
| **Re-export defect** | Root/wrapper, axis, scale, grounding, or generated pose is malformed; a canonical sibling/variant proves the output should share one origin. | `rpg-game-assets` reproducible export/conversion. Current exemplar: #43 downed family. | Web offset, per-variant anchor, scene nudge. |
| **Stable asset anchor** | Correct generated asset has a coherent pivot but one measured registration point is intrinsic across scenes, cameras, and all six facings. | Private calibration declaration -> generated rpg-game-assets safe catalog. Bookcase visible center and torch back registration are initial exemplars. | Placement tweak or gameplay footprint. |
| **Legitimate per-variant anchor** | Each variant is a correct intentional output, but its stable intrinsic point genuinely differs; repeated evidence proves the difference is not an export defect. | Separate concrete catalog entries, each bound to its own digest/evidence. | Family-level inherited offset or excuse not to repair export. |
| **True multi-hex footprint** | Rules/authoring say the object physically occupies several cells independent of camera and visible pivot. | Gameplay/placement contract in its owning toolkit/API/authoring design. | Visual-anchor translation or bounds-derived client rule. |
| **Scene-specific nudge** | The desired change varies with wall, room, composition, or art direction: mount height, clearance/embed, local trim. | Authored placement. Torch height and Kirk's `-0.20m` fixture trim are current exemplars. | Global asset default. |

When classification is ambiguous, no metadata ships. Record the ambiguity and run
the experiment that distinguishes the rows.

## Dungeon YAML v0.4 Assets handoff

Direct answer for
[rpg-project#206](https://github.com/KirkDiggler/rpg-project/issues/206) / PR #203:
**no additional YAML fields are required for visual-anchor positioning if the
currently proposed v0.4 semantic placement fields are ratified.** The remaining
Assets work is the exact shared adapter above plus production evidence, not another
YAML object.

### Exact current-proposal field audit

| Candidate from discussion | Actual current status at PR #203 HEAD `c37a1e1` | Assets answer |
| --- | --- | --- |
| `at` | Existing v0.3 absolute owning cell | Reuse unchanged. |
| `facing` | Existing v0.3 floor-prop field; proposed v0.4 explicitly rejects combining it with `anchor` | Do not create a parallel facing field. `anchor.orientation` owns anchor facing. |
| `mount` | Decode-known but rejected in v0.3; proposed v0.4 rejects combining it with `anchor` | Do not ratify it for this slice. `anchor.support` carries floor-vs-wall support. |
| `height` | Not a placement schema field | Do not add it. For wall support, `anchor.adjustment.vertical` is the semantic attachment-point height above floor datum. |
| `rotate_degrees` | V0.3 calls it experiment-only, not a dialect candidate; PR #203 does not propose it | Do not add it for anchor positioning. Six-direction/into-cell/into-wall orientation is sufficient for v1. |
| `offset` | Not proposed | Do not add it. Proposed `anchor.adjustment {along,normal,vertical}` is already the single anchor-local translation. |
| `anchor.surface/edge/support/orientation/adjustment` | Already proposed by PR #203 | Ratify these names/shape; #205 supplies their exact adapter semantics. |

### Executable mapping from proposed YAML to the external resolver

The server validates only the semantic combinations and finite adjustment numbers,
persists them exactly, and projects them exactly. It does not load a catalog and
catalog data cannot change YAML validity.

| `surface` | `support` | Other required shape | Required external model point | Target datum and exact adjustment mapping | Result |
| --- | --- | --- | --- | --- | --- |
| `floor` | `floor` | no `edge`; `orientation=E|NE|NW|W|SW|SE` | `floor-contact` | `at` cell center on floor; `along→faced right`, `normal→faced forward`, `vertical→world up` | `M_floor_floor` |
| `floor` | `wall` | invalid | none | rejected at `anchor.support` | no resolver call |
| `wall` | `floor` | seed-invariant solid `edge`; `orientation=into-cell|into-wall` | `floor-contact` | edge midpoint on floor; `along→structural tangent`, `normal→structural room-inward`, `vertical→world up` | `M_wall_floor` (bookcase shape) |
| `wall` | `wall` | seed-invariant solid `edge`; `orientation=into-cell|into-wall` | `wall-attachment` | edge midpoint/floor datum; same axes; `vertical` is attachment-point height above floor and `normal` is structural-plane clearance | `M_wall_wall` (torch shape) |

A floor surface with any edge, a wall surface without an edge, a door/missing/
procedural support edge, an unsupported orientation vocabulary, a monster/boss
anchor, or non-finite adjustment is rejected deterministically at its source path.
A catalog model-point mismatch is a client visual diagnostic/fallback; it cannot
turn a semantically valid YAML document into a server validation failure.

For wall cases, the structural room-inward vector and tangent are fixed by
`at+edge`. `orientation: into-cell|into-wall` rotates the **model** by `0|180°`
around its registered point; it does not flip the persisted adjustment basis.
Positive `adjustment.normal` always means structural room-inward, so Builder and
Game cannot disagree about clearance sign.

### Actual proposed syntax: bookcase and torch

```yaml
# floor-supported bookcase aligned to a wall
- ref: 'dnd5e:props:bookcase'
  at: [3, 2]
  blocks_movement: true
  blocks_los: true
  anchor:
    surface: wall
    edge: E
    support: floor
    orientation: into-cell
    adjustment: { along: 0.05, normal: -0.02, vertical: 0.0 }

# wall-supported replacement authored as a complete new document value
- ref: 'dnd5e:props:torch-ornate'
  at: [3, 2]
  blocks_movement: false
  blocks_los: false
  anchor:
    surface: wall
    edge: E
    support: wall
    orientation: into-cell
    adjustment: { along: 0.05, normal: -0.02, vertical: 1.15 }
```

`1.15` is shown only as the Learn fixture's explicit scene-authored mount choice; it
is not a catalog baseline, schema default, or generally accepted production value.
Omitting a component means zero, exactly as PR #203 proposes. Replacement preserves
`at`, edge/orientation intent, and compatible along/normal adjustment only because
the client author chose to carry them; it changes ref/support/vertical and submits a
complete candidate. The resolver reloads the new stable default variant's intrinsic
facts. V1 requires no authored `variant_id`.

### Adjustment enforcement boundary

Proposed v0.4 is correct to validate adjustment for **shape and finiteness only**,
never clamp or silently change it, persist exact presence/values, and make Game apply
those exact values. Catalog-informed bounds are Builder UX and release-evidence
criteria only; they are not normative server/YAML validity and no catalog dependency
enters the platform validator.

#729's experimental along `±0.25m` / normal `±0.20m`, #732's `±0.25m` fine
trim, and Kirk's fixture `-0.20m` remain evidence/UX observations, not schema bounds
or hard-coded defaults. Builder may warn or bound its controls based on the released
catalog, but the submitted finite value remains exact and the platform never clamps.

### No raw transform or calibration in YAML

YAML contains only ref, `at`, authored blocker facts, and the proposed semantic
`anchor` fields above. It never contains concrete variant id in v1, matrix, world
XYZ, quaternion/Euler, scale, source yaw, pivot/model point, GLB path/digest,
catalog owner/hash, or wall-GLB measurement. Builder and Game adapt the same
persisted semantics against the same external visual-calibration release; release
provenance belongs to acceptance evidence, not the YAML model.

Repository ownership, catalog generation, and atomic distribution remain
non-normative #204/#205 production architecture. The Dungeon spec only depends on
an external calibration authority and Builder/Game parity.

### Evidence ledger — do not overstate the Learn

| Claim | Evidence state |
| --- | --- |
| Actual corner-pivot bookcase and wall torch are materially different anchor cases | **Locked Learn:** web #728/#729 and #731/#732. |
| Client replacement can preserve semantic span intent while refreshing asset facts | **Locked concept evidence:** #729; not production persistence/release proof. |
| Actual-GLB inspection, owning hex, shared Play camera, six asset facings | **Locked Learn:** #731/#732; asset facings are not all six wall edges. |
| Downed fighter is export defect, not variant anchor | **Locked classification:** #731/#732; correction remains rpg-game-assets#43. |
| All six wall edges, interior-authored + generated-envelope walls | **Unproven production gate.** Must exercise exact adapter bases/signs. |
| Paired Builder/Game output from one external calibration release/hash | **Unproven production gate.** Concepts Lab screenshots are not this proof. |
| Catalog-informed UX bounds across assets/walls | **Unproven release-evidence gate.** It must not gate YAML validity. |

Required production evidence records external release/catalog revision, selected
stable default, GLB digest, persisted semantic anchor input, and resolved matrix next
to paired Builder/Game screenshots. It covers floor/floor, wall/floor bookcase,
wall/wall torch, all six wall edges, both structural support sources, replacement,
and fallbacks. Cameras may differ; selected entry and matrix may not.

Relevant lineage:
[#204](https://github.com/KirkDiggler/rpg-project/issues/204) /
[PR #205](https://github.com/KirkDiggler/rpg-project/pull/205),
[web #728](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/728) /
[PR #729](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/729),
[web #731](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/731) /
[PR #732](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/732), and
[rpg-game-assets#43](https://github.com/KirkDiggler/rpg-game-assets/issues/43).

## Validation and evidence gates

### Producer / catalog / delivery gates

1. Schema, explicit family defaults, unique stable identities, positive total
   scales, evidenced yaw, finite tag values, path/companion resolution, and digest
   binding pass.
2. Two clean generator runs are byte-identical.
3. Build-time GLB measurement proves each model point against the exact promoted
   output. Deliberate digest, default, scale, yaw, and anchor mutations each fail.
4. Atomic staging proves the checked-in catalog, producer revision, staged runtime
   GLBs, and every per-entry digest are one release unit; old/new mixing fails.
5. Current-seam reconciliation proves enrolled bookcase/torch total scale is the
   intended `0.75`, proves the existing component outer scale is disabled on the
   matrix path, and proves a synthetic `renderScale=2` overlap would fail rather
   than double-scale. Legacy rug/character behavior remains unchanged.
6. Re-export defects emit no anchor workaround. #43 is accepted only when raw
   standing/downed placement shares the token origin through the existing character
   resolver.
7. The safe projection is exactly the public allowlist and contains no raw licensed
   payload. Evidence uses numeric summaries and license-safe screenshots/contact
   sheets only.

### Selector / resolver / consumer gates

1. Pure selector tests cover reorder independence, semantic-ref default selection,
   family membership, unknown/removed explicit id without default substitution,
   missing/foreign default hard failure, and atomic replacement rejection/refresh.
2. Column-vector golden matrices cover floor/floor, wall/floor, and wall/wall;
   both model-point tags; nonzero total scale/yaw; all six floor orientations and
   wall edges; fixed structural normal under `into-cell|into-wall`; persisted
   adjustment axes; and exact identity matrices.
3. Wrong registration kind and missing wall context discriminate against mutations
   and produce their documented exact identity/fallback matrices. Invalid catalog,
   schema, or digest never reaches the resolver as ordinary `no-anchor`.
4. Builder and game feed the same semantic ref to the same selector and the same
   selected entry/registration to the same resolver, then assert identical matrices
   and identical failure visuals. Orbit/Play/game may change cameras only.
5. Component tests prove the returned matrix is the sole primitive/companion
   transform: no outer position, rotation, `SYNTY_SCALE`, or `renderScale` remains
   for enrolled entries.
6. Source/build checks reject `Box3`, geometry traversal, loader inspection,
   semantic/model-name conditionals, or a second selector/transform implementation
   in runtime paths.
7. Actual synced-GLB evidence records one released catalog/producer/GLB hash and
   paired Builder/Game matrices/screenshots for bookcase and torch at a representative
   Discord viewport. The torch covers all six wall-edge orientations and both
   interior-authored and generated-envelope walls; multi-angle viewed statements
   keep height/clearance visibly placement data.
8. Catalog-informed adjustment bounds are verified across multiple assets/walls as
   Builder UX and release evidence only. Server tests prove every finite persisted
   adjustment is preserved/applied exactly and never catalog-clamped; the experimental
   fixture numbers receive no default status.
9. After #43, raw standing/downed fighter toggling keeps one logical hex and visual
   origin with no character catalog entry or web correction. This verifies #43; it
   is not a third anchor rollout.

## Performance characteristics

Family/default and concrete-id selection are O(1); placement is fixed-cost
vector/matrix arithmetic. There is no runtime network request for the catalog, GLB
measurement, bounds calculation, scene traversal, raycast, or model clone. Both pure
contracts may be memoized by `(catalogRevision, semanticRef, explicitVariantId)` and
`(catalogRevision, variantId, registration)` respectively.

The catalog adds constant metadata per enrolled variant and one static table. It
does not duplicate geometry, textures, materials, or companions. A scene pays one
selection/transform resolution per changed placement, not per frame. Catalog byte
size, selector/resolver calls, and missing-asset outcomes join existing performance
evidence; unexpected growth is review-visible.

## Explicit non-goals

- No `plan.md`, implementation issue, code, migration, or batch calibration in this
  design revision.
- No modification of raw licensed GLBs or public redistribution of licensed source
  assets.
- No web correction for #43 or any other re-export defect.
- No runtime bounding-box/wall measurement or automatic "smart centering."
- No model-specific branch in builder, game, model components, selector, or
  placement resolver.
- No generic/unbounded transform editor.
- No proto/API/toolkit change in v1. Author-selected variants require a later,
  separately approved persistence contract; local-only selection is not shipped.
- No pathfinding, collision, LoS, cover, or gameplay-footprint change, and no
  decision that the bookcase truly occupies one or two hexes.
- No global torch mount height and no promotion of the fixture's `1.15m` target or
  Kirk's `-0.20m` trim.
- No calibration/enrollment for `SM_Prop_Bookcase_01`, `_02`,
  `SM_Prop_Torch_Ornate_02`, rugs, walls, characters, or the broader catalog.
- No absorption of #624's broader mirror-generation scope, #469/#523's umbrellas,
  or #43's export work.

## Acceptance invariants

The design and later delivery are acceptable only while all remain true:

1. Raw licensed inputs are immutable; generated defects are repaired reproducibly in
   `rpg-game-assets`.
2. Canonical calibration/default/total-scale metadata is generated by
   `rpg-game-assets`, bound to exact concrete output and producer revision.
3. Catalog plus runtime GLBs publish atomically only after staged-file digest
   verification; the public artifact is a license-safe projection, not another
   hand-maintained source.
4. Builder and game use the same pure selector followed by the same pure placement
   resolver and identical outcome policy.
5. Concrete selection is independent of catalog order. Legacy semantic refs use the
   explicit producer default; removed explicit future intent never silently does.
6. The resolved matrix is the sole primitive/companion transform and total scale is
   applied exactly once.
7. Neither pure contract has model identities, gameplay rules, or runtime geometry
   measurement.
8. A visual anchor changes only the render matrix; logical hexes and occupancy are
   byte-for-byte unchanged.
9. Placement owns mount height, wall clearance, and nudge. Replacement preserves
   intent and atomically refreshes the new family's stable default asset facts.
10. Ordinary no-anchor/context mismatch uses the exact documented identity matrix;
    invalid schema/catalog/digest never degrades into ordinary no-anchor.
11. Dungeon YAML persists only ref/`at`, blockers, and semantic anchor surface/
    edge/support/orientation/adjustment; no matrix/world XYZ/quaternion or asset
    calibration fact enters the document.
12. YAML adjustment validity depends only on semantic shape and finite numbers.
    Catalog bounds may guide Builder UX/evidence but never alter, clamp, or reject the
    exact persisted value; Learn fixture numbers are not defaults.
13. V1 enrolls only small bookcase and ornate torch 01; downed fighter is existing-
    character-path #43 verification only.
14. All six wall-edge orientations, both interior and generated-envelope walls, and
    paired Builder/Game evidence from one released hash are proven before acceptance.

## Judgments for Kirk's review

The proposed architecture takes concrete defaults; these are the remaining Kirk review calls. Each has
a recommended default and an explicit consequence:

| Judgment | Recommended default | Consequence |
| --- | --- | --- |
| Concrete selection in v1 | **Use producer-declared stable `defaultVariantId` from the existing semantic ref; no proto change.** | Reorder-safe and minimal now. Authors cannot choose `_02` until a separately approved explicit-variant persistence contract exists. |
| Public identity | **Use stable variant ids, with paths as mutable data and digests as revision binding.** | Adds one durable id layer; prevents path rename or array order from moving calibration. |
| Anchor representation | **Use tagged intrinsic model points, not stored correction vectors.** | Resolver derives signs uniformly and reviewers can distinguish floor contact from wall attachment; generic offset convenience is rejected. |
| Scale authority | **Make per-entry `totalScale` the sole authority for enrolled variants; leave non-enrolled legacy assets partitioned until #624 migration.** | Prevents double scaling and avoids broad v1 migration; temporary coexistence needs a strict enrollment/overlap gate. |
| Wall target | **Use authored structural wall plane plus explicit placement clearance. Never measure the selected wall GLB at runtime.** | Builder/game parity survives wall-model changes; authors own visible-face/embed judgment. |
| Adjustment UX limits | **Use reviewed per-variant/per-kind hints backed by multi-wall extrema evidence; do not generalize #729/#732 fixture bounds or make hints server validity.** | Builder controls can stay useful without introducing a catalog dependency into YAML validation; any finite submitted value persists/applies exactly. |
| Catalog change behavior | **Treat default/anchor/scale edits as content-digest changes requiring renewed visual evidence; reserve schema major for semantic/axis/unit/order changes.** | Existing semantic-ref scenes re-resolve to corrected asset truth; pixel-stable historical replay would need recorded catalog provenance later. |
| Floor vertical correction | **Forbid Y in v1 floor anchors.** | Grounding errors cannot be hidden; legitimate future vertical attachment semantics require an explicit tagged-contract review. |

The design recommends all defaults above. Approval approves the boundary, not an
implementation sequence. Per cross-repo workflow, a later plan belongs on this same
idea PR only after design approval, and the idea PR remains open through delivery.
