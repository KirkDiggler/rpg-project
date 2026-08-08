# Visual Anchor Metadata — Implementation Plan

**Status:** planning revision proposed for Kirk review after the eight design decisions
in [`design.md`](design.md) were approved at PR #205 HEAD `067fbc9`. This plan is
not approved and is not an implementation-start signal.

**Goal:** ship one digest-bound visual-calibration release for the small bookcase and
ornate torch, then make Dungeon Builder and the actual Game select and place those
exact variants through one pure selector/resolver with identical matrices and no
runtime geometry measurement.

**Scope:** two future implementation issues/PRs—one in `rpg-game-assets`, one in
`rpg-dnd5e-web`—plus coordination with the already-proposed Dungeon YAML v0.4
Tranche B contract in rpg-project PR #203 / issue #206. No issue is created until
Kirk approves this plan.

**Design:** [`ideas/visual-anchor-metadata/design.md`](design.md), approved decisions
recorded on [rpg-project PR #205](https://github.com/KirkDiggler/rpg-project/pull/205).
The idea PR stays open through delivery and is not merged merely because this plan
is coherent.

## Non-negotiable boundaries

- Raw licensed inputs remain immutable and private. Generated defects are fixed
  reproducibly in `rpg-game-assets`, never hidden by web metadata.
- V1 enrolls only `SM_Prop_Bookcase_Small_01` and
  `SM_Prop_Torch_Ornate_01`.
- Dungeon YAML uses PR #203's proposed `at` plus
  `anchor.surface/edge/support/orientation/adjustment`; this plan adds no YAML,
  proto, API, or toolkit field.
- Platform code never loads or validates the visual catalog. Server adjustment
  validity remains semantic-shape + finite-number only.
- `rpg-game-assets#43` stays separate. The downed fighter is a post-fix verification
  through the existing character resolver, never a catalog entry or web offset.
- No runtime `Box3`/GLB measurement, asset-name branch, duplicated selector, or
  second transform path enters Builder/Game.
- No implementation issue, branch, code, or tag is created from this plan before
  plan approval.

## Verified current seams (planning baseline)

Validated without editing the owning repositories:

| Repo/ref | Current fact | Plan consequence |
| --- | --- | --- |
| `rpg-game-assets origin/main@d22c53f` | `library/prop-role-map.json` is the hand-maintained prop source; `scripts/build_prop_manifest.py` measures promoted GLBs and writes `harness/models/synty/props/manifest.json`. | New calibration declarations join the existing generated inventory; raw pack manifests remain untouched. |
| same | Generated prop manifest has global `calibration.SYNTY_SCALE=0.75`; it has no stable default id, safe visual projection, per-entry total scale, or asset digest. | Provider work must add these explicitly rather than infer them in web. |
| same | No repository tags and no repository CI workflow currently exist; Python script tests use stdlib `unittest`. | Release/tag and deterministic generator gates must be explicit in the provider PR/checkpoint. |
| `rpg-dnd5e-web origin/dev@4b85bb7` | `src/components/hex-grid/propManifest.ts` is a hand-maintained mirror; `resolvePropVariant` returns `PROP_KEYS[key]?.[0]`. It contains three rug `renderScale: 2` overrides. | Enrolled defaults become id-based/order-independent; non-enrolled props stay legacy. #624 remains the broader mirror owner. |
| same | `PropModel.tsx` applies `position`, `rotationY`, and `SYNTY_SCALE * renderScale`; character wrappers independently apply `SYNTY_SCALE`. | Enrolled matrix rendering needs a compile-time-distinct path that cannot double-apply position/rotation/scale. Characters remain legacy. |
| same | Builder `src/author/preview3d/DungeonPreview3D.tsx` resolves a variant directly, then passes position/rotation to `PropModel`. | Builder must adapt v0.4 semantics, then call the shared selector/resolver. |
| same | Actual Game path reaches props through `EncounterMap` → `HexGrid` → `HexEntity`; `HexEntity` wraps `PropModel` in an outer world-position group and falls back to a capsule. | Enrolled Game placement must bypass both current transforms while preserving the generic fallback. |
| same | `scripts/sync-synty-assets.sh` pulls the adjacent asset checkout's latest branch then `rsync --delete`; `.github/workflows/docker.yml` separately shallow-clones asset default `main` and copies files without catalog/digest verification. | Local and Docker staging must share one pinned, atomic, verified path. |
| same | Learn-only modules include `PropCompositionConcept*`, `propCompositionExperiment*`, `AssetAnchorLabConcept*`, `AssetAnchorLabPreview*`, and `assetAnchorExperiment*`. | Production must not keep duplicate correction tables/math; retain evidence and only the diagnostic value that consumes shared production contracts. |
| PR #203 `c37a1e1` | V0.4 already proposes `at` and nested `anchor {surface,edge,support,orientation,adjustment {along,normal,vertical}}`; it rejects parallel `facing`/`mount`. | The remaining contract work is the executable semantic adapter, not new YAML. |

**Native gates:** web uses `npm run ci-check` before every push. Asset scripts use
focused `python3 -m unittest ...`, direct generator/verifier runs, and deterministic
clean-diff checks; there is no honest aggregate asset CI command today.

## Work-item and PR structure (created only after plan approval)

### Future issue A — rpg-game-assets provider

**Suggested title:** `Generate digest-bound visual calibration catalog for bookcase and torch`

- Repo/base: `rpg-game-assets`, fresh branch from `origin/main`.
- Board 19: Feature=`The Dungeon`, Team=`Assets`, Kind=`Build`.
- One issue, one branch, one ready PR. No public raw assets.
- Owns source declarations, schema/generator/verifier, initial enrollment, private
  asset evidence, and immutable provider release tag.
- Does **not** touch web code or #43.

### Future issue W — rpg-dnd5e-web consumer

**Suggested title:** `Consume shared visual anchors in Builder and Game`

- Repo/base: `rpg-dnd5e-web`, fresh branch from `origin/dev`.
- Board 19: Feature=`The Dungeon`, Team=`Assets`, Kind=`Build`.
- One issue, one branch, one ready PR for the complete web wave: contract fixture,
  ingestion, selector/resolver, Builder, Game, evidence, and cleanup.
- Links web #624 but does not close it unless #624's full broader acceptance is
  independently met.
- No proto/API/toolkit change. It consumes Tranche B projection supplied under
  #203/#206 when that external contract is available.

### Existing owners that remain separate

- rpg-project #203/#206 owns v0.4 ratification/tracking and any later platform
  tranche issues. This plan supplies adapter semantics only.
- rpg-game-assets #43 owns downed re-export centering.
- web #624 owns general manifest-mirror elimination beyond the two enrolled props.
- rpg-project #204 / PR #205 remains the cross-repo tracking/review surface.

## Dependency graph

```text
Approved design (#205)                         DONE
        |
        v
Plan review on same PR                         CURRENT
        |
        v
PR #203/#206 records and ratifies exact Tranche B adapter boundary
(no new YAML; finite-only platform validation; external visual authority)
        |
        +-----------------------------+
        |                             |
        v                             v
W branch: consumer-first pure      A branch: producer catalog,
contract/tests using fixtures      actual GLBs, verifier
        |                             |
        +-------------+---------------+
                      v
A PR merges to main; immutable provider release tag is created
                      |
                      v
W branch replaces fixtures with pinned released catalog/GLBs
                      |
          #203 Tranche B runtime projection available
                      |
                      v
Builder + actual Game parity/evidence on W branch
                      |
                      v
A independent gate -> W independent gate -> Kirk visual acceptance
                      |
                      v
Implementation PRs may merge; #205 remains open through final evidence/#43 verify
```

The external Tranche B wire/projection is a dependency for actual Game parity. If
#203 changes the agreed boundary, stop and return to Kirk/design review; do not invent
a client approximation or silently add proto/API/toolkit work here.

## Develop outside-in; merge inside-out

### Develop outside-in

1. **Consumer semantics first:** on the single web branch, encode the approved
   selector/resolver and PR #203 adapter against license-safe fixtures. The consumer
   names the exact safe-catalog shape and failure outcomes.
2. **Provider implements that contract:** asset generator proves the catalog against
   the actual promoted bookcase/torch GLBs.
3. **Consumer integrates the real provider release:** replace fixtures with the
   generated released artifact, then wire Builder and Game.
4. **Runtime projection closes the loop:** use the actual #203 Tranche B projection,
   never a Concepts-only imitation, for Game evidence.

The web PR may open ready only when it consumes a real provider release; fixture-only
work is an internal checkpoint, not merge-ready state.

### Merge inside-out / provider first

1. Merge future asset PR A to `rpg-game-assets/main` after its independent gate.
2. Kirk creates the immutable provider release tag at that exact merged commit.
3. Web W pins that tag/catalog revision, passes atomic staging and actual parity.
4. Merge W to `rpg-dnd5e-web/dev` using the repo's normal feature-PR squash flow.
5. Leave #205 open until cross-repo evidence and post-#43 verification are recorded.
   PR #203 may remain open for its other v0.4 tranches.

There is no web-first merge: a public consumer cannot honestly pin an unpublished
private provider output.

# Phase 0 — v0.4 ratification coordination

Before either implementation issue is cut:

- [ ] PR #203 states that no YAML field beyond its current semantic anchor shape is
  required by Assets.
- [ ] Its normative table matches the approved adapter:
  - floor/floor → floor-contact at cell floor center;
  - wall/floor → floor-contact at structural edge/floor datum;
  - wall/wall → wall-attachment at structural edge/floor datum;
  - floor/wall → reject.
- [ ] Structural room-inward/tangent remains fixed; `into-cell|into-wall` rotates the
  model 0|180° without flipping adjustment signs.
- [ ] Server validation is semantic shape + finite numbers only; no catalog limit,
  hash, scale, pivot, or correction enters server validity/projection.
- [ ] #206 records the still-unproven production evidence gates rather than claiming
  the Learn already passed them.
- [ ] Kirk ratifies Tranche B semantics (or explicitly says implementation may start
  before the broader v0.4 PR completes).

**Stop condition:** any request for server catalog resolution, raw transform fields,
new YAML, or asset-dependent gameplay validation returns to design review.

# Phase A — rpg-game-assets provider issue/PR

## A1. Source declaration and schema

**Create:**

- `library/visual-anchor-calibrations.json`
- `scripts/test_build_web_asset_catalog.py`

**Read/modify:**

- `library/prop-role-map.json`
- `scripts/build_prop_manifest.py` only if a small reusable inventory helper is
  needed; do not mix visual calibration into gameplay role semantics.
- `harness/models/synty/props/manifest.json` remains generated inventory.

The declaration is private producer input. It names:

- schema version;
- semantic family and stable `defaultVariantId`;
- concrete variant id + promoted file;
- accepted GLB digest;
- sole `totalScale` (`0.75` for both initial variants unless producer evidence
  disproves it);
- evidenced `sourceForwardYawRad` (including evidence-backed zero);
- tagged floor-contact/wall-attachment model point; and
- optional Builder UX bound hints, explicitly non-normative for YAML validity.

Tests fail for duplicate ids, default missing/foreign to family, nonpositive scale,
unproven/malformed yaw, invalid tag/finite values, stale accepted digest, and a
floor-contact vertical correction.

**Judgment:** use ids independent of path, recommended
`synty:props:<source-piece-name>`. The filename may move; the stable id does not.
Changing an id is a content migration, not a rename.

## A2. Deterministic safe projection

**Create:**

- `scripts/build_web_asset_catalog.py`
- `harness/catalogs/synty-web-assets.json` (generated, safe allowlist)

The generator joins declarations to the existing generated prop inventory and the
actual promoted files. It emits only:

- schema/catalog revision and producer release id;
- families + explicit default ids;
- stable id/ref/path/digest/totalScale/yaw/tagged point/companions;
- optional UX hints.

It excludes private source paths, pack notes, geometry, materials, textures,
footprint/LoS/gameplay facts, and measurement dumps. Two consecutive runs must be
byte-identical and leave `git diff` clean.

`catalogRevision` is a digest of normalized consumer entries. Every model/companion
digest is computed from the actual promoted GLB. A deliberate byte mutation, order
mutation, missing default, or stale declaration must turn the test/generator red.

## A3. Atomic bundle verifier

**Create:**

- `scripts/verify_web_asset_bundle.py`
- `scripts/test_verify_web_asset_bundle.py`

Given a staged root + catalog, the verifier checks schema, catalog revision, producer
release id, paths, exact staged file SHA-256 values, companions, and allowlisted
fields. It fails closed and never repairs/copies files.

Tests stage old-catalog/new-GLB and new-catalog/old-GLB mixtures, missing or
unexpected catalog references, corrupted bytes, unsupported schema, and wrong
producer release. Unrelated non-enrolled files in the broader staged harness remain
allowed. Every mutation must return nonzero.

## A4. Initial enrollment and calibration evidence

Enroll exactly:

| Family/default | Concrete file | Calibration |
| --- | --- | --- |
| `dnd5e:props:bookcase` | `props/SM_Prop_Bookcase_Small_01.glb` | `floor-contact` measured X/Z point; total scale 0.75; evidenced yaw |
| `dnd5e:props:torch-ornate` | `props/SM_Prop_Torch_Ornate_01.glb` | `wall-attachment` measured X/Y/Z point; total scale 0.75; evidenced yaw |

- [ ] Prove each model point from build-time inspection of the exact promoted digest.
- [ ] Prove source yaw is a coherent asset convention on the exact output and
  relevant siblings, not a Root/axis repair.
- [ ] Record numeric measurement provenance and license-safe multi-angle screenshots.
- [ ] Confirm `_Bookcase_01/_02`, `_Torch_Ornate_02`, rugs, walls, and characters are
  not enrolled by family association.
- [ ] Confirm no raw licensed file/source path appears in public-safe output/evidence.

## A5. Provider gate and release handoff

Run at exact PR head:

```bash
python3 -m unittest scripts.test_build_web_asset_catalog \
  scripts.test_verify_web_asset_bundle
python3 scripts/build_web_asset_catalog.py
python3 scripts/build_web_asset_catalog.py
git diff --check
git status --short
```

Also run any touched existing manifest tests and the verifier against the real staged
bundle. The PR checkpoint records commands, catalog revision, both GLB digests, and
proposed provider release id.

### Version/tag handoff

No asset/web tags exist today. Recommended first immutable provider release:
`web-assets/v1.0.0` (annotated tag) on the merged asset PR commit. The catalog can
name that release id before merge; the release gate verifies the tag resolves to the
commit containing byte-identical catalog/assets. Tags are never moved; changed
content gets `web-assets/v1.0.1`, while schema-breaking semantics get a new catalog
schema major/release major.

This avoids a self-referential "catalog embeds its own future commit SHA" problem.
If Kirk rejects introducing tags, the fallback is an out-of-band pinned merge SHA
stored in the web provenance wrapper—not a hand-edited provider digest. Decide before
A implementation begins.

# Phase W — rpg-dnd5e-web consumer issue/PR

One web branch carries W1–W8. Do not split integration findings into stacked PRs.

## W1. Consumer-first catalog and pure contracts

**Create (recommended paths):**

- `src/rendering/visual-assets/catalog.ts`
- `src/rendering/visual-assets/selectVisualVariant.ts`
- `src/rendering/visual-assets/resolveVisualPlacement.ts`
- `src/rendering/visual-assets/semanticAnchorAdapter.ts`
- colocated `*.test.ts`
- test-only safe catalog fixtures; no fake production generated file

Cover:

- stable default selection independent of entry order;
- legacy semantic-ref selection;
- future explicit id known/removed/foreign behavior without default substitution;
- hard missing/foreign default;
- exact floor/floor, wall/floor, wall/wall matrices and floor/wall rejection;
- all six floor orientations and wall edges;
- fixed structural inward sign under `into-cell|into-wall`;
- no-anchor identity, model-point mismatch, missing-wall fallback, invalid catalog,
  digest failure, and model-load failure outcome;
- replacement refresh of the new family's default facts.

The modules have no React/R3F/loader/model-name imports. Mutation tests must prove
order, sign, transform order, and fallback assertions discriminate.

## W2. Generated artifact ingestion and legacy crosswalk

**Generated provider copy:** `src/generated/synty-web-assets.json`.

**Modify:**

- `scripts/sync-synty-assets.sh`
- `.github/workflows/docker.yml`
- `src/components/hex-grid/propManifest.ts` and its tests only at the enrollment
  crosswalk seam

Use one shared staging script/path locally and in Docker:

1. resolve the pinned immutable provider release;
2. clone/fetch into a temporary checkout without pulling/mutating the developer's
   shared adjacent checkout;
3. stage catalog + referenced GLBs/companions;
4. verify catalog bytes/revision and every staged digest;
5. atomically replace the destination only on success;
6. preserve the previous good bundle on failure.

The current Docker "clone latest main + cp" and local "pull + rsync" divergence is
removed for this bundle.

Partition by enrollment:

- enrolled bookcase/torch families use generated selector/matrix only;
- all other props use current `propManifest.ts`/`PropModel` unchanged;
- a guard rejects overlap/double authority;
- current rug effective scale 1.5 and character scale 0.75 remain unchanged;
- enrolled total scale applies exactly once.

Post a signed cross-link to web #624. Do not claim the broader mirror issue closed.

## W3. PR #203 semantic adapter

`semanticAnchorAdapter.ts` consumes a minimal semantic interface matching ratified
#203 fields. Builder source and Game projection adapters translate into it; neither
contains placement math.

Tests pin:

| Semantic combination | Resolver registration |
| --- | --- |
| floor/floor | floor-contact; cell floor target; along/right, normal/forward, vertical/up |
| wall/floor | floor-contact; structural edge floor target; tangent/up/inward |
| wall/wall | wall-attachment; structural edge floor target; tangent/up/inward |
| floor/wall or invalid/missing support | deterministic rejection before resolver |

Catalog UX bounds may shape controls/warnings only. Any finite server-valid submitted
adjustment must pass through byte-for-byte and Game applies it exactly; no clamp.

If Tranche B generated types/projection are unavailable, keep only pure fixture tests
and report the external blocker. Do not invent proto types, parse an unratified JSON
shape, or merge a Concepts-only substitute.

## W4. Sole-transform model seam

**Modify/refactor:**

- `src/components/hex-grid/PropModel.tsx`
- `src/components/hex-grid/PropModel.test.tsx`

Recommended shape: preserve `PropModel` as the legacy position/rotation/scale API for
non-enrolled assets, extract shared load/clone/tint/companion rendering, and add a
compile-time-distinct enrolled component/API that accepts one resolved `Matrix4`.

- Set/apply the matrix once on the enrolled root.
- Do not pass position, rotation, `SYNTY_SCALE`, or `renderScale` to that path.
- Parent and companions share the identical matrix.
- Preserve URL-keyed GLTF cache/material lifetime and remembered tint.
- Test a nontrivial scale+yaw+anchor+wall adjustment and assert exact world matrix.
- A deliberate extra outer scale/rotation/position must make tests fail.

## W5. Builder integration and replacement

**Modify:**

- `src/author/preview3d/DungeonPreview3D.tsx` and tests
- the ratified DungeonDoc/placement parser/types owned by the #203 consumer seam
- Builder inspector/replacement reducer at its existing owner once Tranche B lands

Replace the enrolled path's direct `resolvePropVariant` + position/rotation props
with:

```text
persisted anchor -> semantic adapter -> shared selector -> shared resolver
                   -> sole matrix component
```

Legacy/unanchored props remain byte-identical.

Replacement is client-local whole-document editing:

- preserve `at`, edge/orientation, and compatible along/normal only when the action
  says so;
- author the new ref/support/vertical/blockers explicitly;
- select the new family's stable default and refresh scale/yaw/model point/companions;
- never preserve an old matrix or old asset facts;
- validate the complete candidate through #203's normal validate-only flow.

Tests cover bookcase wall/floor → torch wall/wall, failed target selection leaving the
old document unchanged, and no stable placement ID/server replacement command.

## W6. Actual Game integration

**Modify at the real path:**

- `src/components/game/EncounterMap.tsx` or its existing renderable-entity adapter
- `src/components/hex-grid/HexGrid.tsx`
- `src/components/hex-grid/HexEntity.tsx` and tests

Consume the authorized Tranche B runtime projection. Carry semantic anchor data to
the same adapter/selector/resolver; do not rederive from model identity or nearest
wall.

For enrolled props, remove/bypass `HexEntity`'s outer world-position group and
`PropModel` transforms; render the sole matrix. Preserve capsule/loading/error
fallback at the logical owning target and the existing remembered/interaction
behavior.

Current `WALL_ADJACENT_PROP_KEYS` contains only legacy wall-banner behavior. Do not
add bookcase/torch. Enrolled semantic anchors never use nearest-wall heuristics.

Game tests use the exact same catalog fixture and persisted anchor input as Builder
and assert matrix byte/numeric equality, not merely similar screenshots.

## W7. Production parity and performance evidence

Capture one evidence ledger keyed by:

- provider release tag;
- `catalogRevision`;
- concrete stable default id;
- each GLB digest;
- persisted semantic anchor input; and
- resolved 4×4 matrix.

Required paired Builder/Game proof:

- [ ] floor/floor sanity case;
- [ ] corner-pivot small bookcase on wall/floor;
- [ ] ornate torch on wall/wall;
- [ ] bookcase → torch whole-document replacement;
- [ ] E/NE/NW/W/SW/SE wall edges with fixed inward/tangent signs;
- [ ] interior-authored solid wall and generated-envelope wall;
- [ ] `into-cell` and `into-wall` without adjustment-sign reversal;
- [ ] no-anchor, registration mismatch, missing wall context, corrupted catalog,
  and model-load fallback;
- [ ] paired representative Discord viewport screenshots with explicit viewed
  statements and numeric matrices from the same release/hash.

The two enrolled models are pivot-distinct on purpose: bookcase visible-center
floor point versus torch back attachment point. Placeholder boxes or Learn-only
fixture constants cannot pass.

Performance gates:

- no runtime `Box3`, GLB traversal, or per-frame matrix recomputation;
- O(1) family/id lookup and constant-time resolution on placement changes;
- no duplicate GLB/material/companion allocation beyond existing caching;
- catalog bundle size recorded;
- `DevPerfProbe`/current real-route baseline compared before/after on the same scene;
- unexpected draw-call, memory, or frame-time regression blocks acceptance.

Run focused tests during work and `npm run ci-check` at every pushed checkpoint and
at exact final head.

## W8. Experiment cleanup and documentation honesty

After production parity is captured:

- retain `docs/evidence/prop-composition-728*` and
  `docs/evidence/asset-anchor-lab-731*` as immutable Learn history;
- remove fixture-local correction/selection/transform constants from
  `propCompositionExperiment*` and `assetAnchorExperiment*`;
- either make retained Concepts diagnostics call the shared production
  selector/resolver, or remove their route/code if they add no calibration QA value;
- keep runtime bounds visualization isolated to the Learn/calibration route and add a
  source/build guard proving production Builder/Game do not import it;
- update architecture/how-to docs and stale `propManifest.ts` comments only where the
  enrolled/legacy partition changes their truth;
- do not broaden cleanup into class/wall/all-manifest migration (#624).

**Recommendation:** keep the Anchor Lab raw-measurement visualization as an isolated
calibration tool, but make its calibrated answer consume the released catalog/shared
resolver. Remove the one-off Prop Composition route once production replacement
screenshots supersede its executable value. This preserves useful QA without two
sources of transform truth.

# Post-fix downed verification (separate owner)

After rpg-game-assets #43 merges:

- sync the exact corrected asset revision through the existing character path;
- toggle standing/downed fighter on one logical hex in the Anchor Lab and actual
  Game;
- require raw downed centering with no safe-catalog character entry, no
  `variantId` anchor, and no web translation;
- record player + NPC evidence required by #43 separately.

#43 is not folded into A or W and does not block bookcase/torch implementation PR
review. PR #205 remains open until the post-fix verification is recorded; if #43 is
still open, report that explicit external tracking gate rather than weakening it.

# Review, independent gates, and human acceptance

## Asset PR gate

A fresh-context Assets reviewer verifies exact head:

- deterministic generation and mutation red/green proof;
- actual promoted digest/point/yaw/scale evidence;
- stable defaults and safe allowlist;
- staged bundle corruption rejection;
- licensing boundary and no raw public payload;
- only two enrolled variants.

Every finding is answered on the PR with the required signature. Kirk alone merges
and creates the release tag.

## Web PR gate

Before independent review:

- `npm run ci-check` passes on exact pushed head;
- implementer self-review confirms one selector/resolver and no double transform;
- every Copilot thread is individually reconciled if Copilot reviews the PR;
- paired actual Builder/Game evidence is attached with hashes/matrices.

A fresh independent product gate then adversarially checks semantic mapping, matrix
order/sign, legacy partition, corrupted staging, fallback parity, actual-game path,
performance, and screenshot claims. Deliberate mutations of default order, wall
normal, orientation flip, extra outer scale, and digest verification must turn the
relevant tests red.

Kirk performs creative visual acceptance on the pinned provider release. A final
independent re-gate runs after any visual correction. No agent claims approval,
merge-readiness, or completion before those gates.

# Rollback

- Provider tags are immutable. A bad release is not retagged; publish a new patch
  release after correction.
- Atomic staging leaves the prior catalog+GLBs untouched on verification failure.
- Web keeps explicit enrollment partitioning. A rollback can un-enroll the two
  families and restore their legacy render path without changing YAML or gameplay.
- Revert the web consumer commit/PR as one unit; never keep new catalog selection
  with old component transforms or vice versa.
- Persisted semantic anchors remain valid through rollback; legacy rendering may be
  visually less precise but no matrix/calibration data needs migration.
- If #203 changes field semantics, stop the web rollout and return to design instead
  of translating old documents heuristically.

# Real plan judgments for Kirk

| Judgment | Recommendation | Consequence |
| --- | --- | --- |
| Provider release identity | Introduce immutable annotated `web-assets/v1.0.0` tag, then patch tags for content changes. | Clean provider-first pin and avoids self-referential embedded commit SHA; adds the asset repo's first tag convention. |
| Stable variant id spelling | Use `synty:props:<source-piece-name>` independent of path. | Rename/move safe; changing id later is explicit migration. |
| Generator shape | New safe projection generator joined to existing prop inventory, not more fields in `build_prop_manifest.py` role semantics. | Keeps gameplay role catalog and visual consumer contract separate; one extra deterministic script. |
| Enrolled model component | Compile-time-distinct matrix component over shared loading internals. | Makes double transforms harder than a runtime boolean; legacy path remains during #624 transition. |
| UX bounds | Catalog hints inform Builder controls/evidence only; server accepts every finite semantic adjustment exactly. | #206 can ratify without catalog coupling; unusual finite values remain author responsibility and Game parity obligation. |
| Concept cleanup | Keep isolated Anchor Lab raw diagnostics, route calibrated output through production resolver; remove one-off Prop Composition code after parity. | Retains calibration value without duplicate production truth. |
| #43 timing | Do not block A/W implementation PRs; block final #205 tracking completion on post-fix verification. | Separate defect owner stays honest while anchor delivery can proceed. |

If Kirk chooses no provider tags, resolve the alternative pin/provenance mechanism
before implementation issues are cut; do not leave version handoff implicit.

# Definition of done

The visual-anchor delivery is done only when all are true:

1. PR #203/#206 semantic adapter boundary is ratified and any required external
   Tranche B projection is actually available; no catalog logic entered platform.
2. Asset provider PR is merged to main, independently gated, tagged immutably, and
   its catalog/GLBs pass real staged digest verification.
3. Exactly two concrete variants are enrolled with explicit stable family defaults,
   total scale, evidenced yaw, digest-bound tagged points, and license-safe output.
4. Web consumes that exact provider release; local and Docker staging share atomic
   verification and preserve the previous bundle on failure.
5. Builder and actual Game use one selector, one semantic adapter, one placement
   resolver, and one matrix-only model path for enrolled props; legacy props remain
   unchanged and no double transform exists.
6. Replacement refreshes the new family's intrinsic facts while preserving only
   explicitly compatible semantic authored intent.
7. Paired evidence proves two pivot-distinct assets, all six wall edges, both wall
   sources, both wall orientations, replacement, deterministic fallbacks, and equal
   matrices from identical catalog/producer/GLB hashes.
8. Performance, drift, licensing, full web CI, independent gates, and Kirk visual
   acceptance pass at exact heads.
9. Experiment-only duplicate transform constants are removed or routed through the
   production resolver; Learn evidence remains intact.
10. #43 post-fix standing/downed verification is recorded with no web/catalog
    correction.
11. Both implementation issues/PRs carry signed final checkpoints and honest board
    state. Kirk alone merges.
12. PR #205 remains open through all delivery evidence and only then becomes eligible
    for final tracking merge. No plan approval is inferred from this revision.
