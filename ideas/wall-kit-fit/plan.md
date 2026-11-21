# Consistent wall/doorway fitting — implementation plan

**Goal:** matching finishes and explicit physical heights across the wall/door kit, using one declared fitting contract rather than per-asset renderer guesses.
**Authority/spec:** [Courtyard S6 #1022](https://github.com/KirkDiggler/rpg-project/issues/1022); operator agreement in the linked brief. General once-placed feature sections remain the larger goal, not claimed complete here.
**Architecture:** provider-owned named parts and surface bindings; one height-layout/assembly path shared by placed props, structural walls and doorways. Preserve fixed features, reuse the existing upper-row repetition, and keep mechanical aperture/state authority unchanged.
**Baselines:** Assets `6c74fd4`, Web `363f3b62`, Project `e5c7281`. Worktrees: `.worktrees/wall-kit-fit`; direct implementation, no delegated investigation.
**Finish line:** configured inventory, working Studio height/finish checkpoint, history/native/play integration, bounded review and ordinary repository gates. Merge/deployment remains separately authorized. No broad asset-suite campaign.

**One-wall continuation:** [section ownership, authoring and captured delivery plan](sections-plan.md), following the operator's acceptance of texture alignment and rejection of three independently editable walls as the final interaction. Its backend/native capture work remains required.

## Contracts and constraints

- Public catalog `heightFit` is a closed union: `{mode:'scale'}`, `{mode:'fixed'}`, or `{mode:'repeat-above', node:string, baseHeight:number, rowHeight:number, capNode?:string, capHeight?:number}`. The optional cap pair declares one fixed top translated once; the minimum is fixed base plus cap height. Dimensions are in the same rendered units as catalog `boundsMeters`; `baseHeight` is measured from the asset base. They are measured/validated against declared parts, not entered as a second geometry authority.
- Existing `frame`, `leaf`, `above` roles remain the authored part declarations; `cap` declares the optional fixed top. The repeat part is above the fixed region and spans the remaining native height. Door leaves never belong to it. Generic scalable bodies and fixed props use the same profile/consumer contract without becoming doors.
- `planAssetHeight({nativeHeight,targetHeight,profile,maxRows?})` returns an exact height, whole-instance Y scale and repeat-row offsets/scales. A repeat profile leaves fixed geometry at scale 1, preserves the existing nearest-whole-row policy and distributes its residual across the rows. Invalid heights/profiles refuse. Preserve the existing bounded row-pool policy: the cap limits clone allocation, not target height; spread the residual over the rows actually created. Never silently shorten the geometry or stretch the fixed opening.
- Structural `appearance.height` remains the authored target. A doorway inherits its owning wall's explicit height. Existing prop `heightScale` can be converted once to a target height at the adapter; no new native/wire field is presumed necessary.
- Same compatible finish means shared physical U/V scale and phase. All declared masonry parts on a face receive it, including repeated upper rows. Leaf/trim materials stay separate. Timber/atlas surfaces are not silently reclassified as masonry.
- Source preparation uses the existing trusted source/capture and promotion mechanisms. Never use installed output as replacement source or weaken custody checks. Licensed geometry and screenshots stay private.
- Preserve old document intent and operator :3030/:8115, Lab :3180 and accepted preview :3187. The screenshot's exact scene remains unreproduced until its YAML is supplied; this does not prevent fixing the shared contract.

## Task 1 — shared height layout and isolated assembly

**Owner:** Web rendering. **Prerequisite:** agreed contract above.
**Files:** create `src/rendering/assetHeightLayout.ts`, `assetHeightLayout.test.ts`; then integrate in `src/components/hex-grid/WorldAssetModel.tsx` with `WorldAssetModel.test.tsx` and the loaded-instance fit tests.
**Interface:** the closed height profile and `planAssetHeight` above; immutable planning output, no scene/global mutation.
**Behavior/proofs:** differently sized scalable sources reach the same target; repeat-above grows only the upper rows; exactly at the fixed-region minimum has no upper rows; below minimum refuses; invalid/non-finite dimensions refuse. Targets requiring more than the row pool still reach the exact top using the bounded pool, with fixed geometry unscaled. Input remains unchanged and repeated planning is identical.

- [x] Add named height-layout cases and observe failure before the module exists.
- [x] Implement the pure planner and run `npx vitest run src/rendering/assetHeightLayout.test.ts`.
- [x] Replace the existing above-row count/residual calculations with this planner; keep scene attachment/disposal in the existing commit lifecycle, never during render. Other adapters remain Task 3.
- [x] Add loaded-geometry assertions: two instances at different heights cannot mutate each other or the cached source; fixed leaf transforms stay unchanged; shrinking after growing retires the old owned hierarchy.

## Task 2 — provider configuration and asset inventory

**Owner:** private Assets. **Prerequisite:** profile consumed by Task 1 is fixed.
**Files:** `scripts/world_asset_review.py`, `scripts/promote_world_assets.py`, `scripts/build_world_asset_catalog.py`, `scripts/world_asset_surfaces.py`, `scripts/doorway_asset_contract.py`, `scripts/world_asset_receipts.py`, `scripts/doorway_kit_config.py`, `scripts/blender_export_doorway_kit.py`, `scripts/blender_export_wall_corner_candidate.py`, `scripts/prepare_doorway_kit.py`; new `scripts/asset_height_contract.py`, `scripts/blender_height_parts.py` and focused tests; owning recipes under `scripts/configs/world-asset-promotion/`; generated catalog/receipts through existing producers.
**Interface:** catalog `heightFit`, existing part roles and per-part surface bindings. New verifier resolves declared nodes uniquely and derives repeat base/row height from transformed geometry. Leaf-bearing assets without a repeat region are explicitly fixed until authored correctly, not stretched by default.
**Inventory:** [72 wall/door/window/gate/arch-related entries](inventory.md) and their owning recipes/current declarations are recorded. This is coverage tracking, not a runtime name-based eligibility rule. Include the raw castle walls, Fit01, fixed-feature/window walls, house/interior walls, authored openings/doorways and the older Double Door; retain standalone leaves as fixed parts rather than promoting them to complete wall pieces. The current generic picker admits every non-leaf asset; names are for the human inventory, never runtime eligibility inference.

- [x] Add fixtures proving missing/ambiguous repeat nodes, intersecting moving leaves, invalid row dimensions and inconsistent native height are refused.
- [x] Extend the existing catalog contract and verify measured profiles from final bytes. Current candidate declarations produce 279 scale, 23 fixed and 12 repeat-above profiles across the same 314 refs; this is not yet completed part preparation.
- [ ] Prepare named fixed/repeatable parts from trusted sources for the inventory. **First pair prepared:** original Castle Wall 01 and Single Doorway01, using the existing immutable source checkpoint. The broader inventory is not complete.
- [ ] Enroll all compatible masonry targets, including upper rows; verify their declared face/material ownership without assuming every face is one whole rectangular body.
- [ ] Regenerate only the affected publication units through existing custody mechanisms; retain immutable source evidence. Run focused profile/catalog/surface checks, not the known broad suite.

**Completion evidence:** inventory rows point at source identity, declaration and observed height/finish result. Missing source or genuinely incompatible geometry is an explicit blocker, not an invisible deferral.

## Task 3 — one consumer path and explicit authoring height

**Owner:** Web. **Prerequisites:** Tasks 1–2; actual pushed provider pin for integration.
**Files:** `scripts/generate-world-asset-catalog.mjs`, generator tests and `src/generated/worldAssetCatalog.ts`; `src/rendering/plainWallFit.ts`, `wallOpeningFit.ts`, `wallSurfaceCharts.ts`; `src/components/hex-grid/WorldAssetModel.tsx`; `src/concepts/world-building/WorldPropModel.tsx`, `StructuralWallSurfaces.tsx`, `structuralWallEditing.ts`, `StructuralWallPanel.tsx`, `doorwayPlacement.ts`, `doorFrameCapture.ts`; existing corresponding tests.
**Interfaces:** generator copies the verified height profile; callers supply target height, not competing parent/model Y multipliers. Existing aperture/width/depth fit and miter anchors still own their separate axes. Surface chart application uses each fitted/repeated instance's final coordinates.

- [x] Carry the provider profile through the strict generator, normal pinned sync and loaded upper-row validation; reject disagreement with the loaded repeat geometry.
- [x] Add regressions that actual wall/doorway instances with the same target have the same top/base, and the opening/leaf does not scale vertically with the wall.
- [x] Complete the shared adapter mechanisms: generated props, structural walls and doorway assemblies use `assembleAssetHeightInstance` with explicit `targetHeight`. Legacy static props now use the same scale planner against complete loaded bounds, including companions, while percentage-only calls preserve their old behavior. Leafless opening fits support body anchoring without door semantics. Actual per-asset part/material readiness remains Task 2, not implied by an adapter.
- [x] Apply matching finish to every declared masonry region on each face; cover repeated rows and independent door frame phases, not only a single `.find(side)` target. Original height continuation uses the actual declared source chart; unsupported new section/finish combinations refuse before commit.
- [x] Preserve `appearance.height` when changing asset, expose Wall height in the existing inspector, and report minimum-height refusals before committing an invalid edit.
- [x] Verify history/native round-trip and independent captured play for the delivered one-wall/window/door checkpoint. No source-scene fetch or parent lookup is added; broader asset data coverage remains separate.

## Task 4 — integrated checkpoint and delivery

- [ ] Normal provider sync/check; one isolated Studio checkpoint at two explicit heights with Castle Wall 01, Fit01, the Single Doorway and the older Double Door.
- [ ] Inspect matching finishes and protected openings during normal editing/playtesting; use the operator's exported castle when available, without overwriting it.
- [ ] Complete the inventory's remaining configured entries, not just the demonstration pair.
- [ ] Run ordinary PR-boundary gates once per completed update, publish the bounded review and address concrete findings. Keep the asset broad-suite waiver intact.
- [ ] Record exact provider pins, checks, visual evidence, remaining blockers and PR state on #1022. Do not imply implementation approval is merge/deployment authority.

## First-pair checkpoint

- Assets data pin `a5ebe4e`; Web `33158b28`. Draft PRs: Assets #348, Web #1465.
- Actual Studio: `http://127.0.0.1:3190/_job_1022_start.html`, detached preview tree. This is local authoring, not a new multiplayer-play claim or operator acceptance.
- Real loaded wall/doorway heights: 3.75 and 6. Both leaf heights stay 1.7588006258; selected gray finish chart errors are below 5.4e-8. Real mouse/inspector edits refuse 1 without changing the geometry, accept 5.25, and Undo restores 6.
- Full Web gate: 11292 passed / 30 skipped. Focused provider checks: 59 Python tests plus three Blender geometry tests. No broad asset-suite campaign.
- Ruling: prepare new height revisions entirely off-scene, then expand placement-local node/surface aliases before applying charts/materials. Saved surface identities remain unchanged; cached resources remain immutable.
- Ruling: preserve Castle Wall 01's original material/UVs and ref. Its `wall-body-fit` capability does not silently opt existing documents into separate `wall-miter-90` corner behavior. The old owner transfers only that ref; all remaining historical source evidence stays unchanged.
- Remaining explicit checks include Original/default upper-row charts, non-door opening/body-anchor rendering, the wider inventory, native/play integration and one final bounded review. The explicit-gray first-pair checkpoint is not proof of those cases.

## Feature-batch source ruling

The pictured walls include shaped top caps. Repeating their whole upper halves would duplicate those decorations or produce mismatched cross-sections. Keep a `cap` once at the top, repeat only the recipe-selected middle band, and preserve the lower feature. This is the same generic height contract, not a per-model renderer branch.

Nine existing refs are prepared from hash-pinned original FBXs through the registered material importer: Castle Wall 02–05, Archway01, Door Big01, Drain01, Window03 and Window Fancy01. Each band has matching cross-sections through its length and continuous wall backing; through-openings and tapered bands refuse. A recessed facade is allowed to have its backing behind its decorative projections, rather than being assumed solid at every depth. The fixed cap joins the same captured cut and translates without scaling.

The source-face proof initially lost barycentric precision on a long, thin Wall04 triangle because `mathutils.dot` accumulates float32. Double-precision scalar products fix that counterexample without relaxing the existing tolerance. The prepared exports/captures retain source geometry, UVs, material resources and source identities. Facade masonry is enrolled; edge/ornament surfaces retain their authored materials, as with the existing protected trim contract.

This batch remains candidate work and does not turn window/arch geometry into gameplay passages. The subsequent [section continuation](sections-plan.md) implements one-wall placement and native/captured delivery; its focused editor and Play checkpoints are accepted. Whole-inventory preparation is still open.

## Original-height and adapter checkpoint

Web `486ed93d` fixes the measured Original UV mismatch: the plain fitter was stretching fixed-region UVs before upper-row assembly, and copied bands reset their chart. Fixed parts now retain native-height mapping; only explicitly enrolled moving masonry continues its authored paired-channel derivatives over actual row/cap positions. Unbound trim/atlas channels stay authored. No default material or universal repeat size is guessed. Actual Window03/Wall04 checks at 3.75/6 preserve fixed UV values and continue moving charts within 5.3e-7; selected-finish regressions remain green.

The same checkpoint adds full-body anchoring for measured leafless opening assets, keeping their full frame span rather than treating the aperture as their width. They remain visual-only sections. Nine such existing entries can now use the adapter; unmapped materials/fixed native heights remain explicit capabilities/refusals, not invented masonry or repeat parts. The owner rejects new incompatible section/finish edits without history writes and preserves unchanged imported intent.

Legacy static props continue their declared proportional behavior; an explicit target uses the shared scale profile after the complete loaded assembly (including companions) is measured. It neither invents protected parts nor changes old percentage/anchor behavior. A capped generated asset's proportional minimum includes base plus cap. RED/GREEN cases measure actual geometry and source isolation.

Full Web gate passed: 11345 tests /30 skipped, 884 files passed/1 skipped, format/lint/types/build/production guards. Accepted :3191/:3192 editor origins now serve detached d4fcddc5 with their same local drafts; :3190 and the :3193/:8133 Play stack remain unchanged. Developer-only probes use :3194. No new asset bytes/pins, API fields, gameplay rules or operator deployment.

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

**Checkpoint finding:** the existing renderer capped clones but calculated residual against an uncapped row count, so a fixture reporting height 120 rendered only 66. Its residual scaling also shifted a course's lower face when its pivot was centred. The integrated regression now reaches the requested top, keeps the leaf unchanged and anchors courses at their lower face. This is not yet a diagnosis of the operator's screenshot. The plan's initial row-overflow refusal was corrected to preserve the base code's documented bounded-pool policy without shortening valid heights.

**Open implementation checks:** exact scene reproduction needs the operator's YAML; source availability and per-asset part authoring must be established in the inventory. If inspection shows native/wire facts are insufficient, return that specific architectural gap rather than smuggling new authority into the renderer. These checks do not authorize narrowing the promised inventory.
