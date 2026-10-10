# Room surfaces — inspected implementation outline and readiness check

## Goal, authority and scope

Turn the accepted Blender appearance into reusable provider assets/materials and a room-owned Studio/runtime surface path. Authority is [design R1–R6/R11](design.md), operator visual acceptance on [Assets #290](https://github.com/KirkDiggler/rpg-game-assets/issues/290), and [design PR #549](https://github.com/KirkDiggler/rpg-project/pull/549). The [contract proposal](contract-proposal.md) is not yet an accepted schema.

**Not handoff-ready for production implementation.** This document records the source probe, concrete candidate work and coverage check. R7/R8 public contracts and the closed-loop policy R12 still block affected implementation. Do not treat this outline as permission to fill those decisions in a worker. No gameplay-material rules, general arbitrary-mesh deformation, whole-library enrollment, or replacement region model is included.

## Inspected baseline

- Assets: `b23b03de06a9e7b3768df8776a973c9a0d25366c`, current remote main at inspection. Worktree `.worktrees/290-runtime-surfaces`.
- Studio: PR #1243 head `445eb1d517793f96f46e315641fcd9898a9dd669`, inspected in a separate detached `.worktrees/room-surface-contract`. It contains #1245 room-boundary work. The root Web checkout `e22ec3c0` does not contain that owner; using it alone would falsely suggest no regions exist. Current remote dev is `d5103772d86bc1d8601bb7522adbe2c1e1dd55b3`; this probe does not claim the Studio draft is merged there.
- No runtime provider implementation is inspected against current integration pins in this pass. Existing structural ownership is documented by project #527; Web's actual adapter establishes the consumer input. Provider task contracts need their own current-version probe, not the stale local API root.

## Source findings and consequences

| Inspected source/symbol | Finding | Consequence |
|---|---|---|
| Assets `scripts/world_asset_review.py`, `WorldAssetPromotionEntry`, strict recipe keys | Roles and runtime texture policy exist; no surface bindings/material catalog | Additive provider contract must enter source recipes and validation, not generated JSON |
| Assets `scripts/build_world_asset_catalog.py`, `build_world_asset_catalog` | Emits refs, hashes, dimensions, tags and optional roles; bounds include shared 0.75 scale | No existing runtime finish capability; coordinate units need deliberate conversion |
| Assets `scripts/normalize_authored_world_asset.py`, `named_node_world_points` | Hierarchy-preserving normalization and unique node verification exist | Reuse that custody; resolve surface targets against final exported primitives |
| Web `scripts/generate-world-asset-catalog.mjs`, `OPTIONAL_ASSET_KEYS` | Only `roles` accepted as optional | Consumer accept must precede provider emission of additional keys |
| Web `authoringRegions.ts`, `AuthoringRegion`, `BoundaryRun`, `EnclosureWitness` | Scene-owned stable region/label identities and directed source walk | Reuse regions, not notes or gameplay region IDs |
| Web `regionBoundaryGeometry.ts`, `resolveAuthoringRegions` | Pure resolved ring/hex-union or named unresolved result; witness preserves source provenance | Extend transient side-span projection here, not a new material-specific room detector |
| Web `structuralWalls.ts`, `StructuralWall.appearance` | One asset ref plus dimensions/elevation; strict key allowlist | No finish or side-selection semantics currently persisted |
| Web `structuralWallEditing.ts`, `layoutWallPieces` | Cuts openings, repeats and fits pieces per wall; no neighbor-aware miter | A corner-aware fitter is new work; do not mistake legacy overlap-miter rendering for this path |
| Web `StructuralWallSurfaces.tsx` | Shared editor/play surfaces; fits entire assets using catalog bounds | Add bound-surface application after fit, and provider-enrolled body-only corner handling |
| Web `WorldAssetModel.tsx` / `useRememberedModelTint.ts` | Clones object hierarchy; materials/geometry may remain shared; memory overlay snapshots original material references | Clone only owned mutable resources and integrate overlay lifecycle deliberately |
| Web `components/session/structuralLayout.ts` | Reads permitted Atlas records, converts feet once, constructs plain `StructuralWallSurface` | Authoring metadata carriage alone cannot deliver finishes to play |
| Web `StructuralLayoutEnvironment.tsx` | Renders walls and doors independently from permitted records | Do not fetch the full authoring room/walk or require a withheld neighboring wall to fit a known one |

## Actual checks

Assets worktree:

```sh
python3 -m unittest discover -s scripts -p 'test_build_world_asset_catalog.py'
python3 -m unittest discover -s scripts -p 'test_world_asset_review.py'
python3 scripts/build_world_asset_catalog.py --check
```

Result: 6 + 26 tests passed; current catalog byte check passed.

Web worktree:

```sh
npm run test:run -- \
  src/concepts/world-building/regionBoundaryGeometry.test.ts \
  src/concepts/world-building/structuralWallEditing.test.ts \
  src/concepts/world-building/StructuralWallFit.test.tsx \
  src/components/session/structuralLayout.test.ts \
  scripts/generateWorldAssetCatalog.test.ts
```

Result with the Studio worktree's existing dependencies: **117/117 passed, five files**. No dependencies installed or lockfiles changed. Initial reuse of root `node_modules` produced 14 schema-construction failures because that checkout pins proto v0.1.224, while Studio pins v0.1.226. Corrected the isolated worktree's dependency symlink to the Studio dependency tree and reran. This was probe setup error, not a surface feature regression. No full Web gate or new-feature test pass is claimed.

## Conditional work sequence

Develop from the shared renderer's required inputs toward provider declarations. Coordinate Web schema acceptance before provider emission. Publish verified assets before consumer adoption of their exact revision/hash. Integrate gameplay delivery only on released provider bindings, then verify save/play/reload. Production tasks below remain conditional; no merge is authorized.

### A — Export one bounded surface-capable kit and reusable material sets

**Owner:** Assets. **Prerequisite:** R8 binding/material schema agreement; real GLB round-trip of the accepted straight wall/doorway checkpoint. Corner-end capabilities require the separate fit declaration in C, not inferred face directions.

**Existing files:** `scripts/world_asset_review.py`, `scripts/build_world_asset_catalog.py`, `scripts/normalize_authored_world_asset.py`, `scripts/promote_world_assets.py`, `scripts/test_world_asset_review.py`, `scripts/test_build_world_asset_catalog.py`, `scripts/test_promote_world_assets.py`, `docs/human/asset-ingestion/world-asset-promotion.md`.

**Proposed new files:** `scripts/world_asset_surfaces.py`, `scripts/test_world_asset_surfaces.py`, `scripts/build_surface_material_catalog.py`, `scripts/test_build_surface_material_catalog.py`; exact recipe/runtime filenames follow schema agreement and the existing promotion workflow.

**Candidate interface:** `AssetSurfaceBinding` and `SurfaceMaterial` in the proposal. Resolve `(node, material)` against final GLB primitives; refuse zero/ambiguous matches, duplicate claims, non-finite geometry, incompatible declared side or unverified map resources. Missing declarations preserve existing assets byte-for-byte. Image paths remain private runtime-root-relative; no source paths in consumer output.

**Required assertions:** front and back bindings resolve only their wall-body primitives; trim/leaf untouched; duplicate node/material names and overlapping bindings refuse; a re-export changing primitive identity invalidates the receipt; bad digest/path/color-space/normal convention refuses; two finishes reference one geometry file; changing one map cannot publish a half-valid material set.

**Checks:** focused named Python tests, `build_world_asset_catalog.py --check`, complete inventory/staging checks and real Blender→GLB→browser node/material readback. Do not promote ignored proof files as though they already satisfy custody.

### B — Consumer catalog acceptance and instance-safe materials

**Owner:** Web asset/rendering boundary. **Prerequisite:** accepted A schema and generated fixtures; does not require a new room UI initially.

**Existing files:** `scripts/generate-world-asset-catalog.mjs`, `scripts/generateWorldAssetCatalog.test.ts`, `src/components/hex-grid/WorldAssetModel.tsx`, `WorldAssetModel.test.tsx`, `useRememberedModelTint.ts`, `src/concepts/world-building/WorldPropModel.tsx`.

**Proposed new files:** `scripts/generate-surface-material-catalog.mjs`, corresponding generator test; `src/rendering/roomSurfaceMaterials.ts`, `roomSurfaceMaterials.test.ts`. Generated catalog filename follows A's agreed consumer identity.

**Candidate interface:** safe projected surface/material records; per-instance application binds only the provider target, retaining all other materials. Share immutable image cache resources; own changed material/UV/tangent buffers. Missing capability stays authored; declared failure names the surface instead of silently substituting another finish.

**Required assertions:** strict unknown-key/path/schema rejection; one geometry shared across two finishes; front-only assignment cannot recolor back or trim; both normal and color change atomically; invalid/absent bindings; no mutation of cached GLB materials; memory/selection overlays restore the currently selected finish; repeated mount/unmount does not dispose shared images or leak instance resources.

**Checks:** focused generator and `WorldAssetModel` Vitest tests; typecheck; exact private synced material fixture in browser before visibility/persistence claims.

### C — Directed side spans, fitted geometry and mapping

**Owner:** Web shared geometry/rendering. **Prerequisites:** R7/R12 mapping policy and provider fit declaration. The accepted miter appearance does not settle its general schema.

**Existing files:** `src/concepts/world-building/regionBoundaryGeometry.ts`, `regionBoundaryGeometry.test.ts`, `structuralWallEditing.ts`, `structuralWallEditing.test.ts`, `StructuralWallSurfaces.tsx`, `StructuralWallFit.test.tsx`, `structuralWallGeometry.ts`.

**Proposed new files:** `src/rendering/roomSurfaceMapping.ts`, `roomSurfaceMapping.test.ts`, `src/rendering/structuralWallJoins.ts`, `structuralWallJoins.test.ts`.

**Candidate interface:** `WallSideFinishSpan` from the proposal plus an explicitly enrolled fit-capable body contract. Derive final face-path coordinates; keep doorway intervals in mapping distance; apply geometry changes only to cloned enrolled end regions. No mechanical blocker edits or arbitrary mesh inference.

**Required assertions:** one source wall shared by two rooms; T-junctions yield distinct subintervals; opening/closing a leaf does not change mapping; resizing adds repeats, not stretch; placement rotations update normal basis; bound trim excluded; matching inner/outer 90-degree joins; no positive-volume overlap or geometric gaps; source mesh unchanged; unsupported fit/topology names the refusal. Closed-loop test must distinguish R12's selected behavior from silent scale adjustment. Tests for anchor removal, label movement, reversed source direction, unresolved regions and incompatible explicit areas follow the final lifecycle contract.

**Checks:** focused mapping/geometry/fit tests; browser front/back/corner normal lighting renders; a two-room fit test at different lengths/heights and after Undo/reload. Prove the material physical-unit conversion once, with a declared default repeat fixture.

### D — Region finish authoring and persistence

**Owner:** Web's existing room document/session facade. **Prerequisites:** accepted R8 scene/version and R12 anchor lifecycle; A/B/C available for real preview.

**Existing files:** `src/concepts/world-building/authoringRegions.ts`, `regionEdits.ts`, `types.ts`, `serialization.ts`, `roomDraft.ts`, `mapLabelEdits.ts`, `WorldBuildingConcept.tsx`, `studioArrange.ts`, `singleRoomDungeon.ts`; `src/concepts/encounter-studio/studioSession.ts`, `StudioArrangePanel.tsx`, `StudioArrangeFields.tsx`, `EncounterStudioIntegration.test.tsx`; associated codec/region/Arrange tests and `world-building/CONTRACT.md`.

**Candidate behavior:** selected linked room owns finish; whole-document transaction preserves all walls/blockers/policies. No room detection from note labels. Absent setting preserves old appearance; invalid/new-version data is protected rather than stripped. Preview/cancel/no-op creates no history; accepted choice and mapping repair each create one commit. Unresolved region retains intent with explicit feedback.

**Required assertions:** two rooms sharing one wall choose different materials; both 2D/3D views share one selection; rename/move label does not shift texture origin; deleting a region cannot alter another room; old scenes remain unchanged; new settings survive JSON/YAML/Undo/Redo/reload and cannot be downgraded by another editor operation.

**Checks:** targeted region/codec/Arrange tests and native browser walk. Full `npm run ci-check` at the Web PR boundary, not claimed by the source probe.

### E — Permitted play layout and joined delivery (blocked provider brief)

**Owner:** existing encounter compiler/projection, session/API adapters, protos, then Web's `src/components/session/structuralLayout.ts` and `StructuralLayoutEnvironment.tsx` consumers.

**Required output:** current-version outside-in provider brief with exact types/fields, persistence, snapshot/introduction/replacement semantics, unit conversion and disclosure behavior. This task cannot responsibly name final provider changes before that probe. It is explicitly incomplete rather than hiding a cross-repo public-contract decision in a broad "wire it up" task.

**Required tests for that brief:** source→compile→save/load→member snapshot/reveal→shared renderer correspondence; hidden neighboring room does not disclose its name/walk/material assignment or alter the permitted wall's mapping when revealed; known door remains independently drawable; legacy encounters without finish fields retain authored appearance; same room looks the same in Studio and play; only permitted mapping/fit facts cross the boundary. Provider refs are opaque; no shader or licensed-asset interpretation in toolkit/API.

No unrestricted source fetch is an acceptable shortcut. The #1245 opaque source-carriage proof is not evidence that these fields appear in member layout.

## Requirement coverage check

| Requirement | Candidate task/proof | Readiness |
|---|---|---|
| Geometry once, materials reused | A/B: one GLB across two map sets | Schema agreement required |
| Independent shared-wall sides | A/C/D: opposite sides + source subintervals | Existing room owner reusable; side-span API new |
| No stretch / continuous joins | C: fit/UV/normal assertions and room resize | R7/R12 and fit contract required |
| Normal maps and protected trim | A/B/C: actual primitive bindings, tangent basis, frame exclusion | Blender visual proof accepted; browser/export proof still required |
| Door leaf omission independent of finish | A/C: static frame and complete leaf group | Reuse #290 part/fit handoff, not a duplicate role schema |
| History/version/save/reload | D: codec, no-op, legacy and native checks | R8 version/lifecycle required |
| Same result in play without private topology leaks | E then C/D integration | Provider brief and tests missing; completion blocker |
| License/custody | A/B: recipes, receipts, inventory, strict consumer projection | Existing workflow reusable |

## Provider/consumer seam check

| Provider → consumer | Produced vs consumed | Availability | Joined proof |
|---|---|---|---|
| A → B | Surface bindings and material catalog | Both currently absent; exact proposed types must be agreed together | Final GLB primitive readback + consumer generator fixture |
| Existing region geometry → C | Oriented source witness and resolved area → clipped wall-side intervals | Witness/ring exists; side-span result does not | Two-room shared source/T-junction assertions |
| A fit facts → C | Declared deformable ends → visual miter | No existing fit capability established | Exported body prototype and both-side corner render |
| C/D → E | Room intent/resolved visual layout → permitted records | New seam; no provider implementation | Compile/projection/reveal tests at actual pins |
| E → Web runtime adapter → shared surfaces | Opaque refs + permitted mapping/fit values | Current adapter only supplies ordinary appearance | Studio→play correspondence with concealed neighbor |

## Readiness disposition

Reuse existing region, document, structural renderer and promotion owners. Do not create a parallel room model or asset export per finish. The source probe and baseline checks are complete; the production plan is not. Resolve R12 with the operator, finish R7/R8 fit/schema/lifecycle and the current-provider brief, then replace these conditional contracts with runnable tasks and repeat this coverage/seam check before implementation.
