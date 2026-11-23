# Anchored rigid parts — S6 continuation

## Goal, authority and finish line

KirkDiggler approved anchored rigid parts on2026-11-23, resolving [the inner-frame gap](https://github.com/KirkDiggler/rpg-project/issues/1022#issuecomment-7092137564). Extend the existing provider-owned height fitter, not the gameplay or disclosure model. Fixed doors/trim retain their shape; whole caps move rigidly; actual infill declares repeat or stretch. Existing masonry profiles keep their behavior.

Finish: checked provider/consumer contract, source-prepared selected inner panels and four doorway families, native/taller visual proof (including actual full cap preservation and Original atlas sampling), normal sync/gates and accurate draft/review status on #1022. This continuation does not waive the broader S6 native/Play, independent review or released-pin obligations. Large gateway remains parked. No new implementation session or per-task review.

Inspected: Assets `eabb637`; Web `cbcd8b1a`; Project `3b4adce`. Existing worktrees remain `.worktrees/wall-kit-fit`. The old source candidates remain rejected; do not install their top-rail repetition as the new behavior.

## Contract to implement

Add a distinct `heightFit` alternative, leaving `scale`, `fixed` and `repeat-above` unchanged:

```
{ mode: 'anchored', node: string, capNode: string,
  lowerAnchor: string, upperAnchor: string,
  lowerHeight: number, upperHeight: number,
  extension: 'repeat' | 'stretch' }
```

Heights are measured scene-unit distances from the actual asset base. Source markers are geometry-free points: `HeightFit_v1_Lower` is fixed, `HeightFit_v1_Upper` belongs to the cap. Explicit recipe tags `height-anchored-repeat` / `height-anchored-stretch` select exactly one policy. The existing `above` role identifies the genuine infill node, and `cap` identifies one rigid hierarchy. Missing/ambiguous markers, competing policies, wrong ancestry, nonvertical anchors, a marker with geometry, a moving leaf in either controlled part, or a band crossing the protected aperture refuse.

The infill's native geometry spans the anchors. Rigid fixed/cap geometry may overlap that span; bounding boxes no longer dictate its layout. The cap reaches the native top. Fixed geometry retains the asset base. The lower anchor stays fixed; the upper anchor/cap translate by `targetHeight - nativeHeight`. Infill fills the new distance, with one scaled instance for stretch or the existing bounded nearest-row policy for repeat. Native height is the minimum for this new alternative: no unproved shrinking of overlapping architecture. The allocation cap never truncates requested height. Shared mapping retains authored UVs for stretch/atlas content; no UVs may escape a packed atlas by blindly applying masonry continuation.

This is asset presentation metadata. Existing opaque asset refs and wall heights remain the native/wire inputs; no new proto/API/Toolkit fields or gameplay apertures are needed for this height extension. Selectable inner-frame finishes still need their existing atlas-provider/capture integration; the height extension alone does not declare those complete.

## Task A — shared planner and detached assembly

**Owner/files:** Web `src/rendering/assetHeightLayout.ts`, `assetHeightLayout.test.ts`, `assetHeightInstance.ts`, `assetHeightInstance.test.ts`.
**Prerequisite:** approved anchored shape above; existing isolated ownership/64-row machinery.
**Interface/behavior:** add the anchored union alternative and consume exactly its markers/metrics. Reuse the existing owned clone/cap translation/row positioning lifecycle. Validate anchors against loaded geometry before mutation. Preserve legacy inference for old profiles; an anchored source must not be guessed from silhouettes or names of assets.
**Tests:** native3.5 with anchors2/3 and a cap extending down to2.8 reaches6 with fixed leaf/frame unchanged, cap still0.7 high and translated2.5, one stretched infill or bounded repeated rows. Cached source and another instance remain unchanged. Below-native height, reversed/nonfinite metrics, missing/duplicate/geometry-bearing/misparented markers and a leaf under infill/cap refuse. Existing repeat minimum, cap and huge-height cases remain green.
- [ ] Add failing anchored layout/loaded-instance cases.
- [ ] Implement and run `npx vitest run src/rendering/assetHeightLayout.test.ts src/rendering/assetHeightInstance.test.ts`.

## Task B — source measurement and strict catalog seam

**Owner/files:** Assets `scripts/asset_height_contract.py`, `test_asset_height_contract.py`; Web `scripts/generate-world-asset-catalog.mjs`, `generateWorldAssetCatalog.test.ts`, generated `src/generated/worldAssetCatalog.ts` through that generator only.
**Prerequisite:** Task A's consumed shape fixed as above; roles/tags already belong to existing promotion entries.
**Interface/behavior:** derive numeric heights from declared source anchors and actual geometry, never accept guessed catalog numbers. Emit the exact anchored shape. Keep untagged legacy geometry checks strict. Generator checks exact keys, finite ordered heights within native height, distinct names, matching above/cap roles, and repeat/stretch enum; source/loaded geometry validators own real marker existence/placement.
**Tests:** source fixture with full cap overlapping fixed trim succeeds only on explicit anchored declaration; same geometry without it still refuses. Translation of the entire root does not change measurements. Bad tags/markers/ancestry/geometry and moving leaves refuse. Generated catalog accepts the measured shape, rejects extra keys/unknown policy/bad metrics, and exposes matching TS type.
- [ ] Red then green provider and generator cases.
- [ ] Run Assets `PYTHONPATH=scripts python3 -m unittest test_asset_height_contract test_wall_height_band`; Web `npx vitest run scripts/generateWorldAssetCatalog.test.ts`.

## Task C — real model adapter and authoring validation

**Owner/files:** Web `src/components/hex-grid/WorldAssetModel.tsx`, `WorldAssetModel.test.tsx`, `src/rendering/plainWallFit.ts` and its tests if needed, `src/concepts/world-building/wallHeightEdits.test.ts`, `CONTRACT.md`.
**Prerequisite:** Tasks A/B; generated type available before typechecking adapters.
**Behavior:** plain fitting retains native-height UV setup before anchored assembly; chart/profile dimensions reflect the final target; full cap hierarchy remains rigid. Atlas-authored UVs are not treated as masonry charts merely because a model is wall-fitted. Height refusal occurs before the existing authoring transaction/history commit. No per-asset rendering branch or new owner.
**Tests:** actual model component uses anchored assembly rather than returning an unchanged source; two heights isolate cached buffers and protect cap/leaf geometry. Original atlas UVs stay in their authored region. Native minimum refuses before owner commit. Existing masonry Original/selected-chart regressions remain green.
- [ ] Red then green loaded renderer/owner cases.
- [ ] Run the named component/rendering/height-edit tests, typecheck; normal full Web gate once at the PR update, not a repeated campaign on the currently contended host.

## Task D — selected inner source preparation and integration

**Owner:** Assets trusted FBXs/immutable house captures, existing authoring helpers, source recipes under `scripts/configs/world-asset-promotion/`, evidence under `evidence/1022-wall-kit/`; Web's ignored developer fixtures.
**Prerequisite:** Tasks A–C. Source authority remains the original captures/FBXs, never rejected candidates or installed runtime geometry.
**Behavior:** declare whole cap components and anchors; preserve actual rigid door/trim geometry; author genuine plank/plaster infill from the selected family's sources. Keep packed atlas sampling explicit and preserve old registered palette capabilities when changing plain-panel source identity. Refresh affected material bindings through their existing producer, not hand-edited hashes. Begin with plain House01/03 and the four opening/doorway pairs; selected05/06 remain visible coverage, not silently completed. No extra castle-door work.
**Tests/evidence:** exact leaf and whole-cap source signatures/shape, paired opening/assembly frames, native/taller bounds and joins, cap appears once, no repeated horizontal-rail block or plaster striping. Verify Original and supported finish selections separately. Normal producer custody/receipt/index/atomic install and consumer sync/check; installed-data Studio evidence before calling the candidate delivered. Complete source identity/material compatibility guards before promotion.
- [ ] Replace rejected source partition with semantic components/real infill.
- [ ] Restore `pending-tests/test_published_house_height.py` from Assets' local ledger when publishing the accepted data; update assertions for anchored policy and full rigid cap rather than a last sliver.
- [ ] Inspect actual native/taller/rotated views, then bounded publication and provider check.
- [ ] Record exact pins/results/remaining work; retain rejected probe as case evidence, not acceptance.

## Requirement coverage

| Requirement | Tasks | Concrete proof |
|---|---|---|
| Whole rigid trim/caps including overlap | A, B, D | Overlapping cap fixture and actual original cap shape/signature |
| Genuine infill extends, not a horizontal rail | A, D | Declared repeat/stretch behavior; inspected native/taller wood/plaster |
| Existing masonry unchanged | A–C | Existing repeat/cap and Original/chart tests |
| Explicit source authority, no guessing | B, D | Tag/marker/role rejection and source-custody proofs |
| Cached hierarchy and leaf isolation | A, C, D | Two instances; exact leaf signature and rotation |
| Atlas safety and finish compatibility | C, D | UV-domain checks and existing material-binding producer guards |
| Honest delivery | D | Normal sync/Studio evidence, gate/review/pin state, no merge inferred |

## Provider/consumer seams

| Provider | Consumer | Produced / consumed | Dependency / proof |
|---|---|---|---|
| B source measurement | B generator | Same anchored fields and scene units | Source fixture -> strict generator; generated type |
| B generator | A planner / C model | Policy, two markers and measured heights | Unknown/invalid shapes refuse; actual loaded markers checked |
| A planner | A instance assembler | Rows plus cap translation delta | Geometry reaches exact height while cap/leaf dimensions remain fixed |
| D prepared source | A/C real renderer | Named parts/markers, untouched atlas resources | Native/taller installed-data render, not only synthetic fixtures |
| D asset identity | Existing atlas provider | New GLB identity with validated target scopes | Refresh through existing producer; old incompatible binding must not silently pass |

Checked against current code: the old cap translation already uses the whole target-minus-native delta and can be reused; only its bounds-based eligibility must be separated for the new mode. The plain fitter currently continues UVs for every sampled material, so atlas-safe behavior needs an explicit check in C. The source's overlapping solids verifier is already corrected/tested separately. No required new persistence/event field is hidden in these tasks. The held earlier Web gate remains incomplete; it is not completion evidence for this continuation.
