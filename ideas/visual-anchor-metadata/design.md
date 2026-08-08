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
They used actual synced GLBs, all six facings, and the shared tactical Play camera.
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

## Canonical coordinate contract

All consumer-visible translations use **meters** in a right-handed Three.js-style
frame:

- `+X`: model right;
- `+Y`: up;
- `+Z`: model forward, and for a wall-mounted item, away from the wall into the
  room after facing is resolved.

World rendering stays on the XZ floor plane with `+Y` up. A heading of zero maps
local `+Z` to world `+Z`; positive Y rotation follows Three.js's existing
convention. The six pointy-top hex facings remain E, NE, NW, W, SW, SE as already
produced by the shared world/hex math.

The catalog's model points are expressed in the **canonical rendered local frame**:
after the variant's asset scale and source-forward yaw have been applied, but before
intrinsic anchor translation, placement facing, or world placement. This is why the
bookcase point is `0.927m`, not an unscaled accessor coordinate. It also makes the
unit independent of a particular source pack's FBX/GLB unit convention.

A calibration is bound to the tuple:

```text
(distributable GLB digest, canonical scale, source-forward yaw, concrete variant id)
```

Changing any member invalidates the measured anchor until the producer explicitly
reconfirms or recalibrates it.

### Tagged anchor semantics — no generic offset bucket

A concrete variant has either no intrinsic anchor or exactly one tagged anchor:

```text
none
  Legacy/correct-origin behavior. The canonical model origin is used.

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

The tag is semantic and determines required context. A floor anchor cannot silently
consume a wall plane. A wall anchor without wall context does not guess one.

For the initial proof, the expected calibrated facts are:

| Concrete variant | Tag | Intrinsic model point | Explicit exclusions |
| --- | --- | --- | --- |
| `SM_Prop_Bookcase_Small_01` | `floor-contact` | `{x:+0.927, z:+0.330}m` | no wall trim; no footprint change |
| `SM_Prop_Torch_Ornate_01` | `wall-attachment` | measured back/attachment point, approximately `{x:0, y:-0.032, z:-0.076}m`, producer-confirmed against the generated GLB | no default mount height; no `-0.20m` trim |
| `fighter-downed` | none | none | #43 must correct the generated asset; no diagnostic lab translation ships |

The torch's point explains the Learn arithmetic without stealing placement intent:
aligning its `y≈-0.032m` model point to a scene-selected `1.15m` target produces an
origin translation near `+1.182m`; aligning its `z≈-0.076m` back point to the
fixture's nominal `-0.866m` plane produces an origin translation near `-0.790m`.
Only the model point is intrinsic. Both target values belong to the placement/wall
context.

## Canonical metadata and generated catalog

### Private producer sources

The following responsibilities are distinct:

1. `library/visual-anchor-calibrations.json` in private `rpg-game-assets` is the
   reviewed, hand-maintained declaration of intrinsic calibration intent. It is
   keyed by stable concrete variant id, not merely by a semantic prop reference.
   It records the tagged model point, source-forward yaw, measurement provenance,
   and the GLB/render-calibration digest it was accepted against.
2. Existing generated inventory such as
   `harness/models/synty/props/manifest.json` continues to establish which promoted
   GLB, reference key, concrete variant, path, render scale, and companions exist.
   Its current role/theme/measurement concerns remain owned by that pipeline.
3. A deterministic rpg-game-assets projection produces
   `harness/catalogs/synty-web-assets.json`. **This generated file is the canonical
   consumer contract.** It joins the promoted inventory with approved anchor
   declarations and refuses stale or invalid declarations.

The hand-maintained declaration is an input, not something the browser consumes.
The generated catalog is canonical because it proves that declaration against the
actual promoted output and flattens all inheritance before distribution.

### License-safe public projection

The safe catalog contains only what a web renderer needs:

```text
schemaVersion
catalogRevision                 # digest of normalized consumer entries
producerRevision                # rpg-game-assets source revision/provenance
entries by stable variant id:
  semantic reference key
  concrete variant id
  distributable relative model path
  asset digest
  canonical scale
  source-forward yaw
  optional tagged visual anchor
  optional companion variant/path references
```

It excludes raw source paths, licensed source files, vertex/index data, embedded
textures/materials, conversion workspace details, private pack inventory, review
notes, and measurement captures. It also excludes gameplay footprint/collision
semantics from the visual-resolver input.

The byte-identical safe projection is checked into the public web repository as
`src/generated/synty-web-assets.json` through the established authenticated
asset-sync/bake boundary. A static build imports it; the browser does not fetch the
private repository or measure a GLB to complete the catalog. Numeric calibration,
model URLs, digests, and screenshots are license-safe distributable evidence; raw
FBX/GLB source content remains private. Shipped GLBs continue to enter game builds
through the already-approved private asset bake rather than public git.

### End-to-end data flow

```text
PRIVATE rpg-game-assets producer
  immutable licensed inputs + reproducible promoted GLBs
  + generated prop inventory
  + reviewed visual-anchor declarations
                    |
                    | validate exact asset/render digests; deterministic projection
                    v
PRIVATE canonical generated safe catalog
  harness/catalogs/synty-web-assets.json
                    |
                    | authenticated sync/bake; byte-identical safe copy only
                    v
PUBLIC, checked-in web consumer artifact
  src/generated/synty-web-assets.json
                    |
                    | static import
                    v
ONE pure resolveVisualPlacement(entry, registration)
                    |
             +------+------+
             |             |
             v             v
  builder/author view   live game renderer
  (floor/wall adapter)  (floor/wall adapter)
             |             |
             +------ same resolved matrix ------+
```

The initial variants traverse that flow as follows:

| Variant | Producer fact | Safe catalog entry | Generic resolver input | Both consumers observe |
| --- | --- | --- | --- | --- |
| Small bookcase | exact GLB/render digest plus measured floor-contact X/Z point | concrete `variantId`, path/scale/yaw, `floor-contact` point | authored span/floor target, facing, placement nudge | visible center registered to the span; logical hex and footprint unchanged |
| Ornate torch 01 | exact digest plus measured back/attachment point | concrete `variantId`, path/scale/yaw, `wall-attachment` point | authored wall plane/inward normal, scene mount height, clearance/nudge | back point registered to the wall; height/trim remain the caller's data |
| Downed fighter | no anchor declaration | no downed anchor | ordinary entity placement after #43 | raw corrected output shares standing origin; no resolver exception |

This uses the seam identified by
[rpg-dnd5e-web#624](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/624), but
does not absorb #624's broader class/wall/all-manifest migration. #624 remains the
owner of general mirror elimination. This design only requires that visual-anchor
consumers receive one generated catalog and that drift fail loudly.

### Identity and variants

A semantic reference such as `dnd5e:props:bookcase` selects a family. It does not
own one shared anchor. Each distributable model has a stable `variantId`; selection
returns a concrete catalog entry, and the anchor travels with that entry.

Consequences:

- reordering a family's variants cannot detach an anchor from its model;
- `SM_Prop_Bookcase_Small_01` may be calibrated without silently assigning its
  corner-pivot correction to `SM_Prop_Bookcase_01` or `_02`;
- `SM_Prop_Torch_Ornate_02` remains uncalibrated until separately measured;
- companions inherit their parent's final matrix, as today, unless a companion is
  promoted into an independently placeable concrete variant; and
- a standing/downed or color/pose variant may have its own anchor only when the
  variant is a correct, intentional output and repeated evidence proves the visual
  registration legitimately differs. A broken generated variant is fixed first.

No implicit family-level anchor inheritance is permitted. Explicit deduplication in
the producer may point several validated entries at one named calibration record,
but the safe catalog is flattened so the consumer never performs inheritance.

## One pure shared resolver

The resolver has no React, R3F, loader, GLB, asset-repository, hex-pathfinding, or
asset-identity knowledge. It consumes one already-selected catalog entry and one
generic placement frame and returns a transform plus a diagnostic outcome.

Conceptually:

```text
resolveVisualPlacement(entry, registration) -> {
  matrix,
  outcome: applied | no-anchor | missing-context | invalid-entry
}

registration =
  floor {
    worldTargetM,
    facingYRad,
    localNudgeM: { right, up, forward }
  }
  wall {
    wallPlanePointM,
    roomInwardXZ,
    mountHeightM,
    clearanceM,              # positive into room
    localNudgeM: { tangent, up, inward }
  }
```

Builder/authoring preview and live game import this exact resolver. Their only job is
to adapt their existing authored hex/span/wall state into a generic floor or wall
frame. They do not duplicate transform arithmetic. The model component receives
the resolved matrix and renders it; it does not test for `bookcase`, `torch`,
`downed`, or any other model identity.

### Transform composition order

For a raw model vertex `v`, the resolver produces:

```text
world(v) = T(target + authoredLocalDisplacement)
         · R(canonical placement facing)
         · T(-intrinsic model point)
         · R(source-forward yaw)
         · S(canonical asset scale)
         · v
```

Applied to the model from right to left, the order is:

1. **Asset scale** converts source geometry into the canonical rendered-meter
   frame.
2. **Asset forward normalization** makes the concrete variant `+Z` forward.
3. **Intrinsic anchor normalization** moves the declared model point to the
   canonical registration origin. Because it happens before placement facing, the
   same correction rotates correctly through all six facings.
4. **Canonical facing / wall registration** maps `+Z` to the placement's facing or
   the wall's room-inward normal and maps `+X` to the corresponding right/tangent
   axis.
5. **Placement-owned mount height, wall clearance, and local nudge** move the
   registration target in that already-oriented frame.
6. **World placement** locates the frame at the authored hex/span/wall plane.

For a floor registration, the target is the authored floor/span point. For a wall
registration, the target starts on the authored wall plane; mount height moves it
along `+Y`, and positive clearance moves it along the room-inward normal. A signed
local nudge remains an explicit authored adjustment around that target.

The wall plane is authored structural geometry (normally the canonical edge plane),
not a plane measured from the loaded wall GLB. If a particular scene wants the
object embedded, cleared to a visible wall face, or trimmed by Kirk's provisional
amount, it expresses that as placement clearance/nudge. The resolver never chooses
nominal-vs-visible wall policy and never measures wall thickness.

A caller may not supply a conflicting explicit facing and wall inward normal. Wall
registration derives facing from the wall frame; contradictory context is invalid
rather than resolved by precedence.

### Replacement behavior

Replacement preserves:

- placement identity;
- logical owning hex/wall/span;
- the floor-or-wall registration intent;
- authored facing when floor-registered;
- authored mount height, clearance, and local nudge where semantically applicable.

Replacement refreshes:

- semantic ref and concrete variant selection;
- path, scale, source-forward yaw, companions; and
- the new variant's tagged intrinsic anchor.

If replacement changes registration kind (for example floor bookcase to wall torch),
the authoring operation must make that semantic change explicit. It may preserve the
span and applicable nudge components, but it cannot silently reinterpret an old XYZ
matrix.

## Generation, drift, and version behavior

### Deterministic generation and drift detection

A valid producer run has these properties:

- normalized ordering and numeric formatting produce byte-identical output on two
  consecutive runs;
- every reference, concrete variant id, model path, and companion is unique and
  resolves to a promoted distributable file;
- every anchor number is finite, meter-valued, within declared sanity bounds, and
  valid for its tag;
- floor-contact entries are already grounded and cannot carry vertical correction;
- wall-attachment points are checked against the promoted model's build-time bounds;
- the entry's asset/render-calibration digest matches the exact GLB, scale, and yaw
  measured during acceptance;
- changing a GLB, scale, yaw, or variant identity without affirming/removing its
  calibration fails generation as stale;
- the committed generated catalog is clean after regeneration; and
- the web's checked-in safe catalog has the same normalized bytes/catalog revision
  as the private canonical projection whenever the authenticated asset checkout is
  available (local sync and deployment bake).

GLB inspection is allowed **only in the private producer/gate**. It is the evidence
that produces metadata, not a runtime dependency.

### Fallbacks

Fallback is deliberately boring and visible:

| Condition | Resolver behavior |
| --- | --- |
| Valid entry, valid matching context, anchor present | Apply the tagged anchor. |
| Valid entry, no anchor | Use identity intrinsic anchor; still apply facing and placement inputs; return `no-anchor`. |
| Wall anchor without wall context, or floor/wall kind mismatch | Do not guess or measure; use identity intrinsic anchor with the supplied placement frame where possible; return `missing-context`. |
| Unknown/unsupported schema or invalid numeric entry | Catalog validation fails before release. If malformed data nevertheless reaches the resolver, use identity intrinsic anchor and return `invalid-entry`; never crash or invent a correction. |
| Model path/load failure | Existing model/fallback rendering policy owns it; anchor resolution does not hide the load failure. |

Absence is backward compatible: an uncalibrated asset renders as it did before this
contract. Fallback is not evidence that the asset is correct, and no fallback result
may be written back as a new calibration.

### Versioning and visual changes

- `schemaVersion` is an integer contract major. A consumer supports an explicit set
  and refuses generation/build for an unsupported value. Tagged-union meaning,
  coordinate axes, units, or transform order changes require a new schema major.
- `catalogRevision` is a content digest, not hand-maintained semver. Adding/removing
  entries or changing scale/yaw/anchor/path changes it deterministically.
- A catalog content change does not require a schema bump, but an anchor change is a
  visible behavior change. It requires producer evidence, resolver parity tests, and
  renewed visual acceptance for the affected variants.
- Old scene documents remain readable because they store placement intent, not a
  copied resolved matrix. On a catalog revision, they deterministically re-resolve
  against the new intrinsic asset truth. A replay/export that needs pixel-stable
  history may record the catalog revision as provenance, but the initial game and
  builder do not fork runtime behavior by revision.
- Removing an anchor returns that variant to the explicit `no-anchor` fallback; it
  never preserves a stale web-side copy.

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

## Validation and evidence gates

### Producer / catalog gates

1. Schema validation, unique stable identities, finite values, path/companion
   resolution, and digest binding pass.
2. Two clean generator runs are byte-identical.
3. Build-time GLB measurement proves each declared model point against the exact
   promoted output. A deliberate digest/anchor mutation makes the gate fail.
4. Re-export defects have no emitted visual-anchor workaround. The #43 output is
   accepted only when raw standing/downed placement shares the token origin.
5. The safe public projection contains no raw asset/source payload and is exactly
   the approved field allowlist.
6. Review evidence uses numeric summaries and license-safe screenshots/contact
   sheets, never public raw licensed assets.

### Resolver / consumer gates

1. Pure arithmetic tests cover both anchor tags, scale/yaw/anchor/facing/nudge
   order, six facings, positive/negative clearance, and replacement refresh.
2. Missing anchor, wrong context, invalid values, and unsupported schema each
   discriminate against a deliberately broken mutation and take the documented
   fallback.
3. Builder and game feed the same selected variant + registration input to the
   same resolver and assert identical matrices. Orbit/Play/game may use different
   cameras; they may not use different model transforms.
4. Source/build checks reject `Box3`, geometry traversal, loader inspection, or
   model-name conditionals in the runtime resolver/component path.
5. Actual synced-GLB evidence shows the initial bookcase and torch in Orbit, shared
   Play, and live game at a representative Discord viewport, with all six facings
   and multi-angle viewed statements. The torch evidence labels height/clearance as
   placement data.
6. After #43, actual raw standing/downed fighter toggling keeps one logical hex and
   one visual origin with no catalog/web downed correction. This verifies #43; it
   is not a third anchor rollout.

## Performance characteristics

Catalog lookup is O(1) by concrete variant id. Resolution is fixed-cost vector/matrix
arithmetic: no network request, GLB load, bounds calculation, scene traversal,
raycast, or model clone. It is referentially transparent and may be memoized by
`(catalogRevision, variantId, registration inputs)`.

The catalog adds a small constant amount of numeric metadata per calibrated variant
and one statically imported identity/path table. It does not duplicate geometry,
textures, materials, or companion instances. Rendering and GLB caching remain
unchanged. A large scene therefore pays the same model-load/instance costs as today
plus one constant-time transform resolution per changed placement, not per frame.
Catalog byte size and resolver invocation count belong in the existing asset/runtime
performance evidence, with unexpected growth called out in review.

## Explicit non-goals

- No `plan.md`, implementation issue, code, migration, or batch calibration in this
  design revision.
- No modification of raw licensed GLBs or public redistribution of licensed source
  assets.
- No web correction for #43 or any other re-export defect.
- No runtime bounding-box measurement or automatic "smart centering."
- No model-specific branch in builder, game, model components, or resolver.
- No generic/unbounded transform editor.
- No proto, API, toolkit, pathfinding, collision, LoS, cover, or gameplay-footprint
  change.
- No decision that the bookcase truly occupies one or two hexes.
- No global torch mount height and no promotion of the fixture's `1.15m` target or
  Kirk's `-0.20m` trim.
- No calibration for `SM_Prop_Bookcase_01`, `_02`,
  `SM_Prop_Torch_Ornate_02`, or the broader catalog merely because they share a
  semantic reference.
- No absorption of #624's broader class/wall manifest-generation scope, #469/#523's
  asset/prop umbrellas, or #43's export work.

## Acceptance invariants

The design and later delivery are acceptable only while all of these remain true:

1. Raw licensed inputs are immutable; generated defects are repaired reproducibly in
   `rpg-game-assets`.
2. The canonical intrinsic calibration is generated rpg-game-assets metadata bound
   to an exact concrete asset revision.
3. The public artifact is a license-safe projection, not a second hand-maintained
   catalog.
4. Builder and game consume the same catalog entry and the same pure resolver.
5. The resolver has no asset identities, gameplay rules, or runtime geometry
   measurement.
6. Applying a visual anchor changes only the render matrix; logical hexes and
   occupancy are byte-for-byte unchanged.
7. Placement owns mount height, wall clearance, and local nudge. Replacement
   preserves intent and refreshes intrinsic variant facts.
8. Missing metadata falls back to the old origin behavior and reports why; it never
   guesses.
9. Bookcase calibration is limited to the measured small variant; torch calibration
   is wall registration only; downed fighter is #43 verification only.
10. All six facings and builder/Play/game parity are proven on actual synced assets
    before the initial proof is accepted.

## Judgments for Kirk's review

The design takes positions rather than leaving the boundary vague. The genuine
review calls are:

1. **Stable public variant ids rather than file paths as identity.** Paths remain
   data and may move; anchors stay bound to an explicit variant id plus asset
   digest. This adds one durable identity layer but prevents rename/order drift.
2. **Tagged model points rather than stored correction vectors.** Reviewers see
   what point is being registered; the resolver derives the negative translation.
   This avoids the sign confusion already exposed by the fixture's nominal wall
   plane versus `-0.20m` trim.
3. **A wall target is authored structural geometry.** The asset supplies its back
   point; the placement supplies nominal plane, height, and clearance. The resolver
   does not measure the selected wall GLB's visible face. This preserves one
   transform path across different wall models but makes clearance an explicit
   authoring responsibility.
4. **Anchor edits are visual content changes, not schema versions.** They change a
   content digest and require fresh evidence/visual acceptance, while old scene
   documents re-resolve automatically. A schema major changes only when semantics,
   units, axes, or composition order change.
5. **V1 floor anchors cannot correct Y.** Grounding defects must be re-exported.
   Wall attachment points may name Y because placement still chooses the target
   mount height; the model point does not choose it.

Approval of these judgments approves the design boundary, not an implementation
sequence. Per the cross-repo workflow, the later plan belongs on this same idea PR
only after design approval, and the idea PR remains open through delivery.
