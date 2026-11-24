# Distinct authored appearances — S6 continuation

## Authority and scope

KirkDiggler confirmed in Blender that the offered wood palettes look the same and approved provider-owned per-asset/surface appearance grouping: show one meaningful choice, preserve all existing saved references. This resolves the misleading wood picker, not permission to invent wood finishes or publish the pending asset batch. Parent: [#1022](https://github.com/KirkDiggler/rpg-project/issues/1022).

Goal: Assets supplies verified equivalence for mapped authored colour content; the editor offers one representative per group. Wood has one Original appearance; plaster retains distinct alternatives. Rendering/compatibility/native serialization keep exact material refs. No model-name special case, runtime pixel guessing, new store, API or proto change.

Baseline: Assets07ad37e, Web5a2f73d1, Project1c422c9. Existing feature worktrees and isolated3196 candidate Studio remain the development path; accepted3195 source view is untouched.

## Contract

Schema3 atlas surfaces gain optional `appearanceGroups: { original: boolean, materialRefs: string[] }[]`. Exactly one group includes Original; groups partition the compatible material refs without duplicates. The Original group may have no material refs; other groups cannot be empty. Unknown refs, bad booleans/fields, incomplete coverage and multiple/missing Original groups refuse. Absence preserves the legacy full choice list. Groups never grant compatibility or change renderer resolution.

For the selected `wall-body-atlas-v1` scopes, the producer compares RGBA colour content over a conservative union of triangle UV bounding rectangles, with a texel border, at the declared colour channel. Out-of-range UVs conservatively cover the entire image. Different image dimensions do not collapse. This is authored-colour equivalence, not a promise about every mip/lighting/antialias result. Geometry identity, material resources and producer proof bind the result. Other profiles remain without grouping until explicitly supported; no global atlas deduplication.

The sheet retains `value` as the exact current reference and adds a display-only `displayValue` for the group's representative. Original is preferred for its group; other groups use deterministic material order. A single valid group is shown as read-only Original, not a misleading dropdown. An explicit reset remains available for a saved override. Rendering a sheet, unrelated edits, save/reload and Undo/Redo must not canonicalize aliases. Missing/incompatible saved refs remain explicit errors, not Original.

## Task 1 — provider derivation and verification

**Owner/files:** Assets new `scripts/world_atlas_appearances.py` and focused tests; `world_material_atlas_candidate.py`, `prepare_prop_atlas_candidate.py`, their tests and pipeline documentation.
**Interfaces:** optional groups above; derive from final GLB scope and verified default/compatible palette bytes. Build groups for body-atlas targets; recompute them when verifying bindings with actual model/resource roots. Keep old scopes/compatibility unchanged.
**Tests:** two images differing only outside mapped UVs collapse; mapped colour differences remain separate; same images on a different surface can group differently; Original equivalence and deterministic order; geometry/UV/image mismatch refuses; malformed/incomplete group metadata refuses. Existing single-mesh, glazing, receipt and publication tests remain green.
- [ ] RED/GREEN focused tests and producer implementation.
- [ ] Rebuild the staged material bundle against verified candidate world385470b; only the four panel bindings gain grouping, no images or compatibility refs change.
**Verify:** Assets `PYTHONPATH=scripts python3 -m unittest test_world_atlas_appearances test_world_material_atlas_candidate test_prepare_prop_atlas_candidate test_compound_atlas_scopes test_publish_world_materials_v2`.

## Task 2 — wire/schema and display-only projection

**Owner/files:** Web `src/rendering/worldMaterialTypes.ts`, `worldAtlasCatalogSchema.mjs`, its `.d.mts`, `worldAtlasCatalog.ts`, `scripts/world-material-catalog.mjs`; schema/catalog tests; `appearance/appearanceSheet.ts`, `appearanceTypes.ts`, `StudioAppearancePanel.tsx`, sheet/edit/panel tests.
**Interfaces:** same optional groups copied by generated and dev-preview catalog paths. The pure sheet projects distinct choices and displayValue; raw value and owner patches remain unchanged. The existing reset command still clears only on explicit user action.
**Tests:** grouped wood shows one Original; plaster retains distinct groups; equivalent saved refs stay valid and unchanged under unrelated edits; explicit reset works; unavailable/stale references do not collapse into Original; old catalogs still show all compatible choices. Runtime material resolution continues to accept every compatible alias.
- [ ] RED/GREEN schema and sheet/UI tests.
- [ ] Typecheck and targeted existing appearance/history/catalog regressions.
**Verify:** focused Vitest catalog/schema and `appearanceSheet.test.ts`, `appearanceEdits.test.ts`, `StudioArrangePanel.test.tsx`; ordinary full `npm run ci-check` at the PR update.

## Task 3 — staged Studio and paper trail

**Owner/files:** existing private candidate material bundle and3196 preview worktree; source evidence under Assets `evidence/1022-wall-kit/`; main feature commits/PRs and #1022.
**Behavior/proof:** stage the new metadata through the existing preview producer, no hand-edited runtime catalog. Check actual Studio wood read-only display, plaster choices/visible change, and a saved equivalent ref surviving height Apply/reload; verify explicit reset separately. Preserve accepted height/door behavior. Update the pending publication identity—old catalog6996… does not authorize a revised candidate.
- [ ] Actual UI/visual and saved-reference checks.
- [ ] Commit/push checked code and private evidence; retain publication/merge holds.

## Coverage and seams

| Requirement | Tasks | Proof |
|---|---|---|
| Per-asset/surface equivalence, not image-global | 1 | UV-footprint differences and same-image/different-scope cases |
| One meaningful choice / real alternatives | 2,3 | Wood single display and plaster group options in actual Studio |
| Saved refs unchanged | 2,3 | Raw value retained; unrelated edit/reload preserves alias; explicit reset only |
| No compatibility or rendering rewrite | 1,2 | All compatible refs retained and runtime resolution regression |
| Honest publication identity | 3 | Producer-built catalog/receipt, revised SHA, no fabricated acceptance |

| Provider | Consumer | Contract / availability / proof |
|---|---|---|
| Source GLB + palette resources | Assets grouping producer | Exact bound scope/colour channel and verified image bytes; footprint tests |
| Assets schema3 surface | Web strict reader/generated/dev paths | Optional groups with the same fields/absence rules; malformed-wire tests |
| Parsed surface groups | Appearance sheet | Exact raw ref plus display-only representative; preservation tests |
| Sheet | UI / existing owner edit | Read-only single appearance or distinct select; reset remains explicit |

Checks: the existing edit adapter emits changes only on user intent, so displayValue need not become a persisted field. Both Python and JS schema validators currently reject unknown surface fields and must advance together. The dev staging reader uses the same JS schema, so copying only UI code would be incomplete. No implementation delegation or per-task independent review is needed.
