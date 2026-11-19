# Consistent wall/doorway fitting — implementation plan

**Goal:** matching finishes and explicit physical heights across the wall/door kit, using one declared fitting contract rather than per-asset renderer guesses.
**Authority/spec:** [Courtyard S6 #1022](https://github.com/KirkDiggler/rpg-project/issues/1022); operator agreement in the linked brief. General once-placed feature sections remain the larger goal, not claimed complete here.
**Architecture:** provider-owned named parts and surface bindings; one height-layout/assembly path shared by placed props, structural walls and doorways. Preserve fixed features, reuse the existing upper-row repetition, and keep mechanical aperture/state authority unchanged.
**Baselines:** Assets `6c74fd4`, Web `363f3b62`, Project `e5c7281`. Worktrees: `.worktrees/wall-kit-fit`; direct implementation, no delegated investigation.
**Finish line:** configured inventory, working Studio height/finish checkpoint, history/native/play integration, bounded review and ordinary repository gates. Merge/deployment remains separately authorized. No broad asset-suite campaign.

## Contracts and constraints

- Public catalog `heightFit` is a closed union: `{mode:'scale'}`, `{mode:'fixed'}`, or `{mode:'repeat-above', node:string, baseHeight:number, rowHeight:number}`. Dimensions are in the same rendered units as catalog `boundsMeters`; `baseHeight` is measured from the asset base. They are measured/validated against declared parts, not entered as a second geometry authority.
- Existing `frame`, `leaf`, `above` roles remain the authored part declarations. The repeat part is above the fixed region and spans the remaining native height. Door leaves never belong to it. Generic scalable bodies and fixed props use the same profile/consumer contract without becoming doors.
- `planAssetHeight({nativeHeight,targetHeight,profile,maxRows?})` returns an exact height, whole-instance Y scale and repeat-row offsets/scales. A repeat profile leaves fixed geometry at scale 1, preserves the existing nearest-whole-row policy and distributes its residual across the rows. Invalid heights/profiles and allocation overflow refuse; no silent cap or height clamp.
- Structural `appearance.height` remains the authored target. A doorway inherits its owning wall's explicit height. Existing prop `heightScale` can be converted once to a target height at the adapter; no new native/wire field is presumed necessary.
- Same compatible finish means shared physical U/V scale and phase. All declared masonry parts on a face receive it, including repeated upper rows. Leaf/trim materials stay separate. Timber/atlas surfaces are not silently reclassified as masonry.
- Source preparation uses the existing trusted source/capture and promotion mechanisms. Never use installed output as replacement source or weaken custody checks. Licensed geometry and screenshots stay private.
- Preserve old document intent and operator :3030/:8115, Lab :3180 and accepted preview :3187. The screenshot's exact scene remains unreproduced until its YAML is supplied; this does not prevent fixing the shared contract.

## Task 1 — shared height layout and isolated assembly

**Owner:** Web rendering. **Prerequisite:** agreed contract above.
**Files:** create `src/rendering/assetHeightLayout.ts`, `assetHeightLayout.test.ts`; then integrate in `src/components/hex-grid/WorldAssetModel.tsx` with `WorldAssetModel.test.tsx` and the loaded-instance fit tests.
**Interface:** the closed height profile and `planAssetHeight` above; immutable planning output, no scene/global mutation.
**Behavior/proofs:** differently sized scalable sources reach the same target; repeat-above grows only the upper rows; exactly at the fixed-region minimum has no upper rows; below minimum refuses; invalid/non-finite dimensions and more than the row cap refuse before allocation. Input remains unchanged and repeated planning is identical.

- [ ] Add named height-layout cases and observe failure before the module exists.
- [ ] Implement the pure planner and run `npx vitest run src/rendering/assetHeightLayout.test.ts`.
- [ ] Replace the scattered row-count/residual calculations with this planner; keep scene attachment/disposal in the existing commit lifecycle, never during render.
- [ ] Add loaded-geometry assertions: two instances at different heights cannot mutate each other or the cached source; fixed leaf transforms stay unchanged; shrinking after growing removes extra rows.

## Task 2 — provider configuration and asset inventory

**Owner:** private Assets. **Prerequisite:** profile consumed by Task 1 is fixed.
**Files:** `scripts/world_asset_review.py`, `scripts/promote_world_assets.py`, `scripts/build_world_asset_catalog.py`, `scripts/world_asset_surfaces.py`, `scripts/doorway_asset_contract.py`; new `scripts/asset_height_contract.py` and focused tests; owning recipes under `scripts/configs/world-asset-promotion/`; generated catalog/receipts through existing producers.
**Interface:** catalog `heightFit`, existing part roles and per-part surface bindings. New verifier resolves declared nodes uniquely and derives repeat base/row height from transformed geometry. Leaf-bearing assets without a repeat region are explicitly fixed until authored correctly, not stretched by default.
**Inventory:** enumerate the currently exposed wall/door-related entries and their owning recipes/source captures in a linked case record. Include the raw castle walls, Fit01, fixed-feature/window walls, house/interior walls, authored openings/doorways and the older Double Door; retain standalone leaves as fixed parts rather than promoting them to complete wall pieces. The current generic picker admits every non-leaf asset; names are for the human inventory, never runtime eligibility inference.

- [ ] Add fixtures proving missing/ambiguous repeat nodes, intersecting moving leaves, invalid row dimensions and inconsistent native height are refused.
- [ ] Extend the existing catalog/recipe contract and verify measured profiles from final bytes.
- [ ] Prepare named fixed/repeatable parts from trusted sources for the inventory. Start the visible checkpoint with Castle Wall 01 and Single Doorway01, but do not call the pass complete with only those two configured.
- [ ] Enroll all compatible masonry targets, including upper rows; verify their declared face/material ownership without assuming every face is one whole rectangular body.
- [ ] Regenerate only the affected publication units through existing custody mechanisms; retain immutable source evidence. Run focused profile/catalog/surface checks, not the known broad suite.

**Completion evidence:** inventory rows point at source identity, declaration and observed height/finish result. Missing source or genuinely incompatible geometry is an explicit blocker, not an invisible deferral.

## Task 3 — one consumer path and explicit authoring height

**Owner:** Web. **Prerequisites:** Tasks 1–2; actual pushed provider pin for integration.
**Files:** `scripts/generate-world-asset-catalog.mjs`, generator tests and `src/generated/worldAssetCatalog.ts`; `src/rendering/plainWallFit.ts`, `wallOpeningFit.ts`, `wallSurfaceCharts.ts`; `src/components/hex-grid/WorldAssetModel.tsx`; `src/concepts/world-building/WorldPropModel.tsx`, `StructuralWallSurfaces.tsx`, `structuralWallEditing.ts`, `StructuralWallPanel.tsx`, `doorwayPlacement.ts`, `doorFrameCapture.ts`; existing corresponding tests.
**Interfaces:** generator copies the verified height profile; callers supply target height, not competing parent/model Y multipliers. Existing aperture/width/depth fit and miter anchors still own their separate axes. Surface chart application uses each fitted/repeated instance's final coordinates.

- [ ] Add regressions that actual wall/doorway instances with the same target have the same top/base, and the opening/leaf does not scale vertically with the wall.
- [ ] Route structural walls, doorway assemblies and the prop adapter into the same height assembly path; remove the older duplicated height calculations after parity is proven.
- [ ] Apply matching finish to every declared masonry region on each face; cover repeated rows and independent door frame phases, not only a single `.find(side)` target.
- [ ] Preserve `appearance.height` when changing asset, expose the target clearly in the existing inspector, and report minimum-height refusals before committing an invalid edit.
- [ ] Verify history/native round-trip and independent captured play. No source-scene fetch or parent lookup is added.

## Task 4 — integrated checkpoint and delivery

- [ ] Normal provider sync/check; one isolated Studio checkpoint at two explicit heights with Castle Wall 01, Fit01, the Single Doorway and the older Double Door.
- [ ] Inspect matching finishes and protected openings during normal editing/playtesting; use the operator's exported castle when available, without overwriting it.
- [ ] Complete the inventory's remaining configured entries, not just the demonstration pair.
- [ ] Run ordinary PR-boundary gates once per completed update, publish the bounded review and address concrete findings. Keep the asset broad-suite waiver intact.
- [ ] Record exact provider pins, checks, visual evidence, remaining blockers and PR state on #1022. Do not imply implementation approval is merge/deployment authority.

## Visible plan checks

| Requirement | Tasks | Concrete proof |
|---|---|---|
| Explicit equal height across source sizes | 1, 3 | Equal measured final bounds at two target heights |
| Protected opening/leaf, repeat only upper masonry | 1–3 | Unchanged fixed transforms; expected repeat rows; below-minimum refusal |
| Matching finishes/phase | 2–4 | Every declared side target uses the same chart; real joined wall/door view |
| Common configuration, not per-name guesses | 1–3 | Closed profile emitted by provider and consumed by all three adapters |
| Whole covered kit, not only Fit01/Single01 | 2, 4 | Explicit asset/source/result inventory |
| History/native/independent play | 3–4 | Owner transaction, round-trip and captured-door tests plus normal play |

| Provider | Consumer | Contract / availability | Proof |
|---|---|---|---|
| Existing source/capture pipeline + Task 2 | Catalog generator | Named parts + measured `heightFit`; pushed provider commit | Profile fixtures and ordinary sync/check |
| Task 1 | Shared model / structural / prop adapters | Exact target-height layout, typed refusal | Pure layout and actual instance bounds tests |
| Task 2 surface bindings | Shared wall charts / independent door render | Per-part side targets, existing material ref/phase | Matching chart coordinates at joins and repeated rows |
| Existing native/captured height and face data | Task 3 | Reused fields; no protocol addition currently needed | Round-trip and independent-disclosure regressions |

**Open implementation checks:** exact scene reproduction needs the operator's YAML; source availability and per-asset part authoring must be established in the inventory. If inspection shows native/wire facts are insufficient, return that specific architectural gap rather than smuggling new authority into the renderer. These checks do not authorize narrowing the promised inventory.
