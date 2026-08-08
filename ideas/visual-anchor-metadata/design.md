# Visual Anchor Metadata — canonical calibration with ratified world offset

**Status:** Course-corrected design proposed for renewed Kirk review on
[rpg-project PR #205](https://github.com/KirkDiggler/rpg-project/pull/205).
The earlier semantic-anchor/YAML mapping was based on superseded PR #203 HEAD
`c37a1e1` and is withdrawn. Ratified Dungeon YAML v0.4 at PR #203 HEAD `40a3938`
is the contract this revision consumes.

North star: **the asset owns its intrinsic registration; authored YAML owns only the
logical placement and an optional finite world-axis translation; Builder and Game
compose both through one pure resolver.**

V1 production proof remains deliberately narrow:

- `SM_Prop_Bookcase_Small_01`: intrinsic visible-center calibration;
- `SM_Prop_Torch_Ornate_01`: intrinsic attachment-point calibration;
- both selected as stable catalog defaults from their existing semantic refs;
- Builder and actual Game apply the same `at`/existing `facing`/ratified `offset`
  and catalog entry to produce the same matrix; and
- no downed-character web correction. rpg-game-assets #43 remains an independent
  re-export defect and is not a completion dependency for this design.

No implementation issue/code is created by this revision.

## What the Learn locked

Accepted Learn evidence:

- [web #728/#729](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/729)
  proved that the actual corner-pivot bookcase needs intrinsic centering, replacement
  must refresh asset facts, and raw matrices are the wrong persistence unit.
- [web #731/#732](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/732)
  measured actual synced GLBs, distinguished intrinsic correction from scene nudge,
  exercised six asset facings in the Concepts Lab, and classified downed displacement
  as an export defect.
- The Learn renderer reported the bookcase center as approximately
  `(+0.927,+1.257,+0.330)` **Learn scene units** after its legacy 0.75 scale; the
  horizontal point belongs to the asset. Learn scene units are an experiment-local
  measurement label, not an assertion that the values already equal ratified
  game-world units.
- The Learn renderer reported the torch back/attachment point as approximately
  `(0,-0.032,-0.076)` Learn scene units; its target location/height remains authored
  scene choice.
- Kirk's `-0.20` Learn-scene-unit fixture trim and the Learn control bounds are
  scene/UX evidence, not catalog defaults or server validation. Production promotion
  must prove and record the Learn-scene-unit → canonical-game-world-unit conversion
  (including an evidenced identity conversion if that is the result) before using any
  numeric fact.

The Learn did **not** prove a persistent wall edge/support model, all six authored
wall-edge registrations, or Builder/Game parity from one released catalog. Ratified
v0.4 deliberately does not add those semantics.

## Ratified Dungeon YAML v0.4 boundary

### Exact field audit at PR #203 `40a3938`

| Field | Ratified state | This design's use |
| --- | --- | --- |
| `ref` | Existing placement identity | Shared pure selector resolves the family's stable catalog default. |
| `at` | Existing logical owning cell; compiled absolute in projection | Supplies canonical world placement origin `p`. |
| `facing` | Existing v0.3 rule unchanged: accepted only where that baseline permits it (non-monster floor/default-mount props); six values | Supplies placement yaw `φ` only when valid/present. This design does not broaden applicability. |
| `mount` | Existing v0.3 decode-known/rejected rule remains unchanged | Not used to infer wall support or catalog registration. |
| `offset` | Ratified v0.4 optional exact triple `[x,y,z]`; finite game-world components relative to canonical origin; facing does not rotate it | Supplies world translation `o`. Omission is zero without authored presence. |
| `anchor`, `surface`, `support`, `edge`, `orientation`, `adjustment`, `height`, `rotate_degrees`, `variant_id` | Not ratified v0.4 placement fields | Not introduced by this design. |

Direct answer: **v0.4 needs no additional YAML beyond ratified `offset`.**

### A world translation is not a raw full transform

Allowed persisted render intent:

```yaml
- ref: 'dnd5e:props:bookcase'
  at: [3, 2]
  facing: E
  offset: [0.05, 0.0, 0.0]
```

`offset` is exactly a finite world-axis delta in game-world units. It is not rotated
by facing. The compiler/API preserve it verbatim; it remains mechanically inert.

Forbidden YAML remains:

- matrix, quaternion, Euler/raw rotation, or scale;
- pivot correction, intrinsic model point, source-forward yaw, GLB path/digest, or
  catalog identity/revision;
- semantic wall edge/support/attachment data; and
- renderer-derived bounds or wall-GLB facts.

The distinction is load-bearing: v0.4 pragmatically transports one translation
vector without turning YAML into a serialized renderer transform.

## Locked ownership

### rpg-game-assets owns intrinsic visual calibration

Raw licensed inputs are immutable. `rpg-game-assets` owns reproducible promoted GLBs
and generated metadata for:

- stable semantic-family default and concrete variant id;
- exact distributable path/digest;
- sole `totalScale` conversion from GLB-local units to canonical game-world units;
- evidenced source-forward yaw convention;
- tagged intrinsic `floor-contact` or `wall-attachment` model point expressed after
  scale in those same canonical game-world units; and
- required initial Builder UX hint ranges/steps/evidence in those same canonical
  game-world units for local calibration controls.

Wrong Root/axis/scale/grounding/generated-pose centering remains re-export work. A
yaw or model point cannot launder a malformed export.

### YAML/platform owns verbatim placement transport

Toolkit validates exact shape/finiteness and compiles `offset`. Protos carry optional
presence and exact three components on authoring and authorized runtime placement.
API persists/projects without interpreting. No platform layer loads the visual
catalog, derives offsets, clamps values, or changes gameplay from them.

### Web owns presentation and optional authoring conversion

Builder may present convenient asset/anchor-local controls. Before persistence it
must deterministically convert the chosen local delta to the ratified world axes and
write only `offset`. The exact persisted triple is previewed and Game applies it
verbatim.

For a local right/up/forward control vector `ℓ=(r,u,f)`, whose components are
canonical game-world units, and existing placement yaw `φ`, the Builder conversion is:

```text
o = xyz( R_y(φ) · [r,u,f,0] )
```

This conversion occurs only in Builder authoring UX. Once persisted, `o` is world
axis data and neither Game nor resolver rotates it again. A wall-oriented local UX
may use a transient selected wall frame, but that wall relationship is not persisted;
changing geometry does not reattach/recompute the offset.

Rich semantic wall/support/anchor persistence is explicitly **future contract work**,
not hidden inside v0.4.

### Gameplay remains separate

Offset/model calibration cannot change owning cell, collision, blockers, pathing,
range, targeting, LoS, fog, visibility, or interaction identity. Authored blocker
fields remain sole mechanics. A visually multi-cell object does not become a
multi-hex gameplay footprint through this resolver.

## Canonical catalog and variant selection

### Current reality

- Private generated prop manifest records global `SYNTY_SCALE=0.75`.
- Public `propManifest.ts` is hand-maintained, selects index zero, and adds rug
  `renderScale=2` (effective 1.5).
- Character wrappers independently apply 0.75.

For each **enrolled** concrete variant, safe catalog `totalScale` becomes the sole
scale authority. Enrolled component rendering disables all outer/global/renderScale
multipliers. Non-enrolled props remain on the legacy path; no concrete asset may
have two authorities. Web #624 remains the broader mirror owner.

### Exact immutable ids

V1 grammar:

```text
^synty:props:[a-z0-9]+(?:-[a-z0-9]+)*$
```

Exact literals:

```text
synty:props:sm-prop-bookcase-small-01
synty:props:sm-prop-torch-ornate-01
```

A producer registrar rejects duplicate ids across primary entries and companions,
duplicate paths with conflicting ids, a family default outside that family, and any
id mutation without explicit migration. Paths may move; ids do not silently change.

### Separate pure selector

```text
selectVisualVariant(catalog, semanticRef, explicitVariantId?)
  -> selected(entry) | selectionFailure(reason)
```

V1 persists no explicit variant id. Each enrolled family declares
`defaultVariantId`; selector order is irrelevant.

- semantic-ref-only placement selects the declared default;
- future explicit id requires a separate approved persistence contract;
- future unknown/removed/foreign explicit id never falls back silently;
- missing/foreign default is hard invalid catalog; and
- replacement selects the new ref's stable default and reloads every intrinsic fact.

Initial defaults are the two exact literals above. Sibling bookcase/torch variants,
rugs, walls, and characters remain non-enrolled.

The separate placement API accepts optional calibration so generic placement transport
is not coupled to catalog selection:

```text
resolveVisualPlacement(entryOrNone, canonicalOrigin, facingYaw, worldOffset)
  -> matrix + diagnostics
```

Builder and Game call this same pure function. Non-prop/legacy callers pass no entry;
only a successfully selected enrolled prop supplies catalog calibration.

## Units, axes, and exact transform

Vectors are homogeneous columns in a right-handed Three.js-style frame:

- `+X` world/model right;
- `+Y` up;
- `+Z` forward;
- positive yaw `θ` maps local `+Z` toward `+X`:

```text
R_y(θ) = | cosθ  0  sinθ  0 |
         |   0   1    0   0 |
         |-sinθ  0  cosθ  0 |
         |   0   0    0   1 |
```

The named production length unit is the **canonical game-world unit** used by
ratified `p` and `offset`. Catalog `totalScale=s` converts one GLB-local length unit to
canonical game-world units. After `R_y(ψ)·S(s)`, tagged point `a`, every hint
min/max/step, `p`, `o`, and every resolver translation are all expressed in that same
unit. Angles are dimensionless radians. No value labeled only as a Learn scene unit
may enter this equation until G proves its conversion.

Let:

- `q` = a GLB vertex in GLB-local units;
- `p` = current canonical world origin (compiled `at` initially; the current canonical
  entity origin after movement/re-placement), in game-world units;
- `o=(ox,oy,oz)` = persisted v0.4 world-axis offset, or zero, in game-world units;
- `φ` = existing valid facing yaw, or the existing renderer's canonical default when
  facing is absent;
- `a` = selected entry's tagged point after `R_y(ψ)·S(s)`, in game-world units;
- `ψ` = dimensionless source-forward yaw; and
- `S(s)` = the GLB-local-unit → game-world-unit uniform conversion.

Every valid placement kind uses exactly one generic world placement frame:

```text
P = T(p) · T(o) · R_y(φ)
```

Non-enrolled props, monsters, bosses, and other valid placements receive `P` exactly
once; their existing model-internal calibration/scale remains below that group. An
enrolled prop additionally receives intrinsic calibration:

```text
C(a) = T(-a) · R_y(ψ) · S(s)
M(a) = P · C(a)
worldVertex[game-world unit] = M(a) · q[GLB-local unit]
```

Dimensional check: `S(s)` converts `q` to game-world length; rotations preserve that
unit; `a`, `p`, and `o` are game-world translations. No meter/world conversion is
implicit.

Consequences:

1. source scale/yaw normalizes enrolled geometry into canonical game-world units;
2. intrinsic point moves to canonical local placement origin;
3. existing facing rotates model + intrinsic correction;
4. persisted world offset translates **outside** facing and is never reinterpreted;
5. canonical origin locates the placement and changes on movement/re-placement while
   the same world-axis `o` follows it; and
6. catalog lookup/calibration occurs only for enrolled props—generic offset is not an
   asset-catalog feature.

Both tags use `C(a)` in v1. `floor-contact` and `wall-attachment` describe which
intrinsic model point was calibrated and which Builder local UX is appropriate; v0.4
does not persist a different registration target. V1 `floor-contact` requires
`a_y=0` in canonical game-world units: bookcase calibration uses only its horizontal
visible-center facts, and a nonzero grounding correction is an asset re-export defect.
A torch reaches a visual wall location only through its authored world offset and
facing, not a persisted wall relationship.

Identity-anchor fallback is exact for a selected enrolled entry:

```text
C_identity = R_y(ψ) · S(s)
M_identity = P · C_identity
```

`P` is the sole world placement transform for every placement. For enrolled props,
`M` is the sole group transform for primitive/companions: no enclosing position,
rotation, `SYNTY_SCALE`, or `renderScale` applies again. Non-enrolled children retain
their deliberately partitioned existing internal scale but never receive `p`, `o`, or
`φ` a second time.

## Source yaw boundary

`sourceForwardYawRad` is required for enrolled entries, including evidenced zero. It
is radians in the sign above and may express only a coherent source convention proven
against the exact promoted output and relevant siblings. It cannot contain placement
facing, camera compensation, staging pose, offset conversion, or scene rotation.
Malformed promoted Root/axis is re-exported.

## Safe catalog generation and release

### Private producer sources

- `library/visual-anchor-calibrations.json`: reviewed declarations, exact ids/defaults,
  accepted digest, total scale, yaw evidence, tagged point, and required initial UX
  hints/evidence.
- existing `harness/models/synty/props/manifest.json`: generated promoted inventory.
- `scripts/build_web_asset_catalog.py`: deterministic join/projection.
- `harness/catalogs/synty-web-assets.json`: canonical safe generated catalog.

Safe output contains schema/catalog revision, fixed `lengthUnit: game-world`,
families/default ids, exact entry ids, paths/digests/total scale/yaw/points/companions,
and authoring UX hints. A unit/axis/scale semantic change requires a schema major. It
excludes
raw source paths/files, geometry/material/texture payload, private pack notes,
measurement dumps, gameplay footprint, and LoS facts.

Any default/id/path/digest/scale/yaw/point/hint/companion edit changes the catalog
content digest. Default, scale, yaw, or point edits require renewed per-entry numeric +
visual evidence before release. Unit/axis/matrix-order meaning changes require a schema
major rather than a content-only revision.

### Provenance choice and split

A provider git revision is release provenance, not self-authored catalog content.
Recommended v1: after G merges, W commits a small provider lock containing only:

- exact merged G commit SHA;
- safe catalog content digest;
- generator/tool identity and version; and
- repository/schema identity needed to verify those facts.

The lock contains no web SHA and no mutable branch name. No tracked G catalog/wrapper
may contain its own not-yet-known post-merge SHA. CI/build checks verify the lock, then
record the actual `github.sha`/web HEAD plus verified lock hash/provider SHA only in a
non-committed evidence artifact and deployment/image labels. This split avoids both
self-reference and a falsely tracked build identity.

Alternative only by explicit Kirk call: create an annotated `web-assets/v1.0.0` tag,
protect `web-assets/v*` from update/deletion via a no-bypass ruleset, and put its tag
object id plus `git rev-parse web-assets/v1.0.0^{}` peeled commit in the W lock. An
unprotected or moveable tag is not acceptable provenance.

### Complete atomic stage

Local sync and Docker build stage as one unit from the exact provider revision:

- the **complete legacy** `harness/models/synty/` tree;
- the safe catalog;
- the verified tracked W provider lock; and
- non-committed digest/build verification results.

The staged catalog is byte-identical to the checked public artifact; enrolled GLB
hashes match; the complete tree replaces the prior public tree only after success.
Failure retains the entire previous catalog+asset tree. Current "pull latest + rsync"
and "clone latest main + copy" paths cannot remain for release evidence.

Unsupported schema, invalid id/default/number/yaw/point, catalog drift, provenance
mismatch, and GLB digest mismatch are hard producer/stage/build failures. They do not
become visual identity fallbacks.

## Required initial Builder UX hints

Initial safe entries must ship reviewed finite local-control hints for both assets;
they are not optional for the proof release. Hints name axes, step, and recommended
inclusive UI range. They are evidence-backed authoring guidance only:

- Builder may bound/warn its controls;
- server accepts any finite `offset` and never reads hints;
- persisted world triple is never clamped; and
- Game applies it exactly.

The Learn's ±0.25/±0.20 and Kirk's -0.20 are candidate evidence, not automatically
accepted production numbers. Provider release evidence must justify the actual hints
across multiple placements/views. If that evidence cannot support hints, return to
Kirk for an explicit design amendment; do not silently omit them.

## Deterministic outcomes and rollback-safe fallbacks

| Condition | Shared result |
| --- | --- |
| Non-enrolled family | Legacy selector/model path, but ratified world offset still applies outside its current model transform. |
| Valid enrolled entry + point | Use `M(a)`. |
| Valid entry with no point | Use `M_identity`, report `no-anchor`; never generate metadata from fallback. |
| Missing/foreign default, unsupported schema, invalid entry, provenance/digest mismatch | Hard generation/stage/build failure; do not publish. Runtime corruption fails closed to generic missing-asset visual at `p+o`. |
| Model HTTP/decode/load failure | Generic shared fallback at exact `p+o`; do not select another variant or drop offset. |
| Replacement target cannot select | Client rejects edited preview/save; prior complete document remains. |
| Valid ref edit | New stable default/intrinsic facts load; client explicitly retains, changes, or removes world offset. No implicit policy. |

Once `offset` is persisted, rollback may not remove parser/transport/projection or
silently ignore it. Catalog enrollment can roll back to legacy rendering, but both
Builder and Game must continue applying the exact world triple. Platform schema is
additive/durable; rollback preserves unknown optional presence through stored source
and encounter snapshots rather than destructively stripping it.

## Replacement under offset-only v0.4

Replacement is client-local complete-document editing, exactly as ratified:

```yaml
# initial
- ref: 'dnd5e:props:bookcase'
  at: [3, 2]
  facing: E
  offset: [0.05, 0.0, 0.0]

# edited candidate
- ref: 'dnd5e:props:torch-ornate'
  at: [3, 2]
  facing: E
  offset: [0.05, 0.0, 0.2]
```

The client may retain/change/remove offset explicitly. New ref selects the new stable
default and refreshes path/scale/yaw/point/companions. There is no server replacement
command, stable placement id, implicit mount-height policy, or preserved matrix.

Without semantic wall/support persistence, replacement cannot promise "keep attached
to this wall" if geometry changes. That richer intent is deferred above v0.4.

## Public/private and licensing boundary

Raw Synty source and promoted GLBs never enter public git. The private authenticated
stage may ship GLBs inside the built game. Public-safe JSON contains only approved
identity/path/render numbers/digests/hints. Screenshots/contact sheets and numeric
matrices are license-safe evidence. An allowlist test rejects source paths, pack
notes, geometry, materials, textures, and raw measurement payload.

Public fixture/schema/matrix checks run automatically without secrets. A licensed
exact-head run is a distinct protected trust gate: Kirk manually approves a reviewed,
same-repository, exact-head job through a protected GitHub Environment; forks and
Dependabot cannot receive the credential. The credential is short-lived, read-only,
least-privileged, and never survives private checkout. Logs/URLs are scrubbed and an
explicit artifact/cache allowlist forbids GLBs, tokens, and private checkout content.
The plan defines the executable gate.

## Performance contract

Named, measurable methods and budgets for the initial two-asset scene:

1. **Pure resolver microbenchmark:** use the repository CI Node version and production
   resolver build on one pinned Linux runner. One sample times a batch of 1,000 resolves
   cycling the fixed six-facing × three-offset case table with `performance.now()`, then
   divides elapsed time by 1,000. Each fresh Node process runs 10 untimed warmup batches
   then 100 timed samples. Sort the 100 per-resolve samples and select nearest-rank p95
   (`ceil(0.95*n)-1` zero-based). Run three fresh processes in the same job/runner and
   take the median of their three p95 values. Required result: ≤ `0.05 ms/resolve`.
2. **Idle recomputation guard:** instrument selector/resolver; after state settles,
   300 `requestAnimationFrame` ticks produce **zero** additional resolves.
3. **Real-route `DevPerfProbe`:** compare exact `origin/dev` base and W head sequentially
   in three paired cycles on the same job/runner, browser version, provider revision,
   fixed route/camera, and already-loaded bookcase/torch. After model-ready plus 120
   warmup frames, sample the probe's existing 8,000 ms window. Aggregate frame time as
   the median of the three reported per-run `frameTimeMs.p95` values. Head must be no
   more than `max(1.0 ms, 5% of base p95)` above base. In **every pair**, existing
   `rendererInfo` deltas must be exactly +0 for `calls.max`, `triangles.max`,
   `geometries`, `textures`, and `programs`. No unsupported GPU-byte claim is made.
4. **Catalog budget:** initial safe catalog + tracked provider lock ≤ 8 KiB
   uncompressed.

Any exceeded budget blocks acceptance or requires a named Kirk-approved exception
with the measured evidence; no "looks fine" waiver.

## Achievable production evidence

Required paired Builder/actual-Game evidence uses identical:

- provider peeled commit provenance;
- catalog revision and GLB digests;
- semantic ref/default variant;
- compiled absolute `at`;
- applicable existing `facing`; and
- exact persisted world `offset`.

Prove:

- bookcase intrinsic centering and torch intrinsic point on two pivot-distinct GLBs;
- omission, explicit `[0,0,0]`, and positive/negative X/Y/Z offsets;
- generic offset exactly once for a representative room prop, canvas prop, room
  monster, and room boss in Builder and actual Game; only enrolled bookcase/torch
  additionally receive catalog calibration;
- VISIBLE-over-REMEMBERED, reconnect snapshot hydration, live update/resight/vacate,
  and movement/re-placement behavior without stale or duplicate offset;
- all six **existing facing** values where baseline rules permit them;
- facing changes model orientation but does not rotate offset;
- bookcase→torch edit with retained, changed, and removed offset;
- Builder/Game numeric matrix equality and paired screenshots at representative
  Discord viewport;
- interior-wall and generated-envelope scenes only as visual authoring examples—the
  contract does not claim semantic support/reattachment; and
- deterministic no-anchor, invalid catalog/provenance, and model-load fallback.

Concepts Lab evidence is input, not production parity. No requirement may claim six
wall edges/support types because v0.4 does not transport them.

## External ratified-offset implementation chain

There are no downstream implementation issues yet. After renewed design/plan approval,
#203/#206 must cut owning items for:

1. **rpg-toolkit / dungeonspec:** decode exactly three finite values, preserve optional
   presence, compile/copy without reinterpretation, strict field-path errors.
2. **rpg-api-protos:** optional exact triple in authoring placement projection and
   authorized runtime placement. Preserve omission versus authored zero. No matrix or
   catalog fields.
3. **rpg-api:** persist/reload authored source, copy toolkit result into authoring and
   authorized runtime projections, preserve encounter snapshots, no calculations.
4. **rpg-dnd5e-web:** author/validate/save offset in Builder and apply projected exact
   offset in actual Game, in addition to visual catalog integration.

The concrete current seams are toolkit `encounter/dungeonspec/{spec,decode,validate,
compile}.go`, protos `dnd5e/api/authoring/v1alpha1/service.proto` plus the authorized
v1alpha2 encounter placement type selected by #203's implementation plan, API
`internal/orchestrators/authoring` + dungeon registry/snapshot and v2 encounter
projection, and web state/render ingestion through `src/hooks/useEncounterState.ts`,
`src/components/game/EncounterView.tsx`, and
`src/components/playtest/playtestMapHelpers.ts` before the Builder/Game renderer paths
documented in the plan.

Develop outside-in from web projection requirements; merge provider-first
(toolkit/protos → API → web). No web-only fixture may be reported as actual Game
support.

## Experiment cleanup

Learn evidence documents/screens remain. Production code must not import runtime
measurement tables. Retained Anchor Lab diagnostics may visualize raw bounds but its
calibrated answer must use released selector/resolver. Remove one-off Prop Composition
correction/selection math after production replacement evidence. A build-graph guard
keeps Learn measurement modules out of Builder/Game.

## Explicit non-goals

- No semantic anchor/surface/support/edge persistence in v0.4.
- No automatic wall attachment, wall-GLB measurement, or scene reattachment.
- No proto/API/toolkit catalog logic.
- No authored variant choice in v1.
- No runtime GLB measurement or model-specific component branch.
- No gameplay footprint/collision/pathing/LoS change.
- No batch enrollment, general #624 completion, raw asset edit, or public GLB.
- No web offset for #43 and no requirement that #43 close before #205 completes.

## Acceptance invariants

1. Ratified `offset` is preserved verbatim and applied once in world axes; facing never
   rotates it.
2. No raw full transform or intrinsic catalog fact enters YAML/protos/API.
3. `totalScale`, post-scale intrinsic point/hints, `p`, `o`, and resolver translations
   share the named canonical game-world unit; Learn scene-unit facts require evidenced
   conversion before promotion.
4. Stable defaults are order-independent and exact ids pass the global registrar.
5. Builder local controls deterministically compile to the displayed persisted world
   triple; Game applies that triple exactly once to every valid placement kind. Only
   enrolled props receive catalog calibration.
6. One selector and one matrix resolver feed Builder/Game; `P` is the sole world
   placement transform and enrolled `M` is the sole primitive/companion group transform.
7. Complete legacy tree + safe catalog stage atomically from the tracked provider lock;
   actual web HEAD belongs only to non-committed build evidence/labels.
8. Invalid catalog/provenance/digest cannot publish; ordinary load/no-anchor fallback
   retains `p+o`.
9. Offset remains cosmetic and mechanically inert.
10. Initial UX hints are evidence-backed and present, but never server validity.
11. Paired evidence is achievable under offset-only v0.4 and does not claim semantic
    walls/support.
12. #43 remains separate/nonblocking and can never justify a web correction.

## Genuine Kirk judgments for renewed approval

| Judgment | Recommendation | Consequence |
| --- | --- | --- |
| Provider provenance | **Pin exact merged G commit SHA + catalog digest + tool identity in a tracked W provider lock with no web SHA.** | Actual web HEAD is non-committed CI/image evidence. If Kirk prefers tags, require protected annotated tag + tag object and peeled commit; unprotected tag is rejected. |
| Exact immutable ids | **Approve the two literals and grammar above.** | Stable across path moves; later spelling changes require migration. |
| UX hints | **Require hints/evidence for both initial entries.** | Builder has bounded useful controls without server coupling; missing evidence returns to design instead of shipping unbounded guesses. |
| Wall authoring | **Accept v0.4 offset-only limitation and defer semantic support/edge persistence.** | Torch/bookcase can be positioned, but cannot automatically remain attached when wall geometry changes. |
| Performance budgets | **Adopt the four quantitative gates above.** | Objective regression bar; measured exception requires Kirk, not agent rationalization. |
| #43 | **Remove it from #205 completion gating; keep only the no-web-offset invariant/cross-link.** | Unrelated export work cannot strand bookcase/torch delivery. |

This revised design requires renewed Kirk approval before the plan is treated as
current.
