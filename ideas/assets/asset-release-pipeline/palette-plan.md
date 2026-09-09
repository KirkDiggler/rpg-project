# Palette-aware Asset Review and Promotion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.
**Goal:** Let an operator choose one audited, prebuilt Dark Fortress palette appearance in the Asset Review Lab and promote those exact bytes without changing existing schema-v1/default-C authority.
**Architecture:** Assets emits and validates one source-matched alternatives descriptor while reusing the current material audit, conversion, normalization, and atomic promotion engines. The Web Lab treats physical source identity and selected appearance identity separately, exports strict schema-v2 Ready JSON, and invalidates Ready whenever the selected bytes are no longer the successfully loaded bytes.
**Tech Stack:** Python 3 stdlib/unittest and existing Assets scripts; React 19, TypeScript 5.8, Vitest 4, Testing Library, Three.js GLTFLoader.
**Spec:** `ideas/assets/asset-release-pipeline/design.md` (human approved; planning complete; implementation is not complete)

## Global Constraints

- Implement against Assets `37c13c68b6cfc87ad6684351f934b4ff1fd83515` and Web `d9553c2edda790a67aa33ae7870e8e72408056b6`; re-read repository instructions and current base refs before implementation.
- Keep Dark Fortress `defaultAtlas` at palette C. Do not rewrite canonical `library/PACK/VERSION/glbs`, schema-v1 recipes, existing refs, default-C runtime bytes, or generated thumbnail contracts.
- Offer only configured, generated alternatives whose physical source is independently audited `ready-default` or `ready-explicit`. Never trust the current `packSlug-palette-X` comparison manifest as provider authority.
- Palette substitution changes only the main atlas. Reuse `audit_group`, `resolve_material_overrides`, `replace_material_slots`, `planned_runtime_image_facts`, `inspect_normalized_glb`, and `atomic_apply_targets`; preserve auxiliary base-color/normal/wrap/object/slot bindings exactly.
- One ref publishes one chosen appearance. Do not add arbitrary uploads, runtime palette variants, shader fallback, or multiple color refs.
- A loading, failed, stale, descriptor-drifted, config-drifted, atlas-drifted, or selected-GLB-drifted preview is not Ready. Import may show an old decision but cannot approve different bytes.
- Assets owns descriptor/provider authority and lands to `main` first. Web consumes that merged contract from a single issue branch based on `origin/dev`. Use one writer per worktree and one PR per repository, not one branch per finding.
- Unit tests use synthetic GLBs/configs/cache trees. The final tiny licensed-source acceptance is an explicit human run and is not a unit-test fixture.

---

## File Responsibility Map

### Assets PR — one Assets issue branch from `origin/main`

| File | Responsibility |
| --- | --- |
| `scripts/synty_palette_alternatives.py` | Strict descriptor dataclasses, canonical parsing, containment/hash/config/audit/binding validation, and selected-GLB resolution shared by preparation and promotion. |
| `scripts/build_synty_palette_variants.py` | Generate prebuilt variants through the existing pack runner, then emit source-matched descriptor rows rather than treating suffixed manifests as trusted authority. |
| `scripts/prepare_asset_review.py` | Join verified alternatives to original trusted candidates and copy each selected GLB into ignored Web custody with a content-addressed URL. |
| `scripts/world_asset_review.py` | Parse schema v1 unchanged and schema v2 with required nested `paletteSelection`; retain original source hash separately. |
| `scripts/promote_world_assets.py` | Resolve selected bytes through the shared descriptor validator, normalize them through the existing engine, record both identities, and include live cumulative metadata in the governed transaction. |
| `scripts/world_asset_cumulative_metadata.py` | Deterministically refresh only #117 `providerMetadata.inventory` and `.meshStats` pointers from actual staged files. |
| `scripts/test_{synty_palette_alternatives,build_synty_palette_variants,prepare_asset_review,world_asset_review,promote_world_assets,world_asset_cumulative_metadata}.py` | Synthetic contract, drift, material-binding, resolution, v1 preservation, selected-byte promotion, and cumulative-pointer regressions. |
| `scripts/test_build_web_asset_catalog.py` | Replace moving total count/hash pins with independent tree/recipe/catalog partition checks while preserving historical path seals. |
| `docs/human/asset-ingestion/{asset-review-lab,world-asset-promotion}.md` | Describe the dropdown, schema-v2 identities, and unchanged schema-v1/default behavior. |

### Web PR — one Web issue branch from `origin/dev`, after the Assets contract merges

| File | Responsibility |
| --- | --- |
| `src/dev/asset-review/model.ts` | Strict v1/v2 catalog/progress/provider parsing, selected-appearance identity, import reconciliation, Ready validation, and batch-ID helpers. |
| `src/dev/asset-review/AssetReviewLab.tsx` | Render one Palette dropdown plus editable/Generate batch ID controls; route selected URL/load state through model helpers. |
| `src/dev/asset-review/AssetReviewScene.tsx` | Continue loading exactly one URL and report loading/success/error for that exact URL; no palette/material logic. |
| `src/dev/asset-review/{model,AssetReviewLab}.test.ts{x}` | Schema, stale import, Ready invalidation, load race, dropdown, and batch-ID regressions. |
| `src/dev/asset-review/AssetReviewLab.css` | Small layout rules for the new dropdown and batch controls only. |

## Task 1: Generate and prepare strict source-matched alternatives

**Files:**
- Create: `scripts/synty_palette_alternatives.py`
- Create: `scripts/test_synty_palette_alternatives.py`
- Modify: `scripts/build_synty_palette_variants.py`
- Modify: `scripts/test_build_synty_palette_variants.py`
- Modify: `scripts/prepare_asset_review.py`
- Modify: `scripts/test_prepare_asset_review.py`
**Interfaces:**
- Consumes: `audit_group(source_root: Path, config: Mapping[str, object], group: str) -> MaterialAudit`, `resolve_material_overrides(config: dict[str, object], source_root: Path) -> dict[str, tuple[ResolvedMaterialBinding, ...]]`, existing pack-runner manifests, and original trusted manifest rows.
- Produces: `PaletteAlternative`, `load_palette_alternatives(pack_root: Path, config_path: Path) -> dict[str, tuple[PaletteAlternative, ...]]`, and `resolve_palette_alternative(pack_root: Path, selection: Mapping[str, object], *, pack_slug: str, pack_version: str, source_path: PurePosixPath, source_glb_sha256: str) -> Path`. The review catalog becomes schema 2 when alternatives are present and each candidate has `paletteAlternatives`.
- [ ] **Step 1: Add failing descriptor tests before implementation**
Use a synthetic original trusted row and two generated GLBs. The contract test must prove physical source identity remains original, exact auxiliary bindings survive, and every mutable authority hash is represented:

```python
def test_descriptor_is_source_matched_and_binds_auxiliary_materials(self):
    alternatives = load_palette_alternatives(self.pack_root, self.config_path)
    a = alternatives[self.source_path][0]
    self.assertEqual("polygon-dark-fortress", a.pack_slug)
    self.assertEqual("v3", a.pack_version)
    self.assertEqual(self.original_glb_sha256, a.source_glb_sha256)
    self.assertEqual("A", a.palette)
    self.assertEqual(self.config_sha256, a.pack_config_sha256)
    self.assertEqual(self.atlas_a_sha256, a.atlas.sha256)
    self.assertEqual(self.variant_a_sha256, a.selected_glb.sha256)
    self.assertEqual(
        [{"objectName": "SM_Prop_Brazier_02", "slot": 1,
          "materialName": "Chains_04",
          "baseColor": {"path": "SourceFiles/DarkFortress/Texture/Misc/Chains_01.tga",
                        "sha256": self.chain_sha256},
          "normal": {"path": "SourceFiles/DarkFortress/Texture/Misc/Chains_Normals_01.png",
                     "sha256": self.normal_sha256},
          "wrap": "repeat"}],
        list(a.material_bindings),
    )
```

Also mutate, one subtest at a time: descriptor hash/path, config bytes, palette name, atlas path/hash, source hash/path, selected path/hash, material status, binding texture hash, symlink component, traversal, and duplicate `(sourcePath,palette)`. Each must raise before a candidate is copied. Include a fixture whose derived manifest still says `polygon-dark-fortress-palette-A`; prove that manifest alone is rejected and becomes usable only through the validated source-matched descriptor.
- [ ] **Step 2: Run the focused tests red**

```bash
python3 -m unittest -v \
  scripts.test_synty_palette_alternatives \
  scripts.test_build_synty_palette_variants \
  scripts.test_prepare_asset_review
```

Expected: failure because the shared descriptor loader and `paletteAlternatives` catalog field do not exist.
- [ ] **Step 3: Implement the smallest shared descriptor authority**
Use frozen dataclasses for portable paths/digests. Descriptor files remain at `palette-variants/COMPARISON/PALETTE/palette.json` and contain one sorted `models` row per source. A row records original pack/version/source/default GLB hash, `materialStatus`, exact resolved auxiliary binding manifest values, and selected GLB path/hash/size. The document records `descriptorVersion: 1`, comparison/group/palette, exact config hash, and atlas path/hash.
Before writing, run `audit_group` on the original config and admit only selected jobs with status `ready-default` or `ready-explicit`; obtain binding values from `resolve_material_overrides`. Read and hash the generated GLB from the isolated variant root, but never reinterpret the suffixed generated manifest as the original pack. Canonicalize with existing `canonical_json`.
In preparation, load original candidates first, then call `load_palette_alternatives`; join only on the complete original tuple `(packSlug, packVersion, sourcePath, sourceGlbSha256)`. Copy alternatives with existing `_copy_candidate` (no hard links), use selected-hash filenames, and emit strict local URLs. If no descriptor exists, emit the byte-identical schema-v1 catalog; if any candidate has alternatives, emit schema 2 and an exact `paletteAlternatives` array for every candidate (empty when none).
- [ ] **Step 4: Add preparation regression coverage and run green**

```python
def test_prepare_copies_only_validated_selected_bytes_without_aliasing_cache(self):
    catalog = self._prepare()
    row = next(c for c in catalog["candidates"] if c["source"]["sourcePath"] == self.source_path)
    self.assertEqual(2, catalog["schemaVersion"])
    selected = row["paletteAlternatives"][0]
    copied = self.web / "public" / selected["url"].removeprefix("/")
    self.assertEqual(selected["selectedGlb"]["sha256"], sha256(copied.read_bytes()))
    self.assertNotEqual(copied.stat().st_ino, self.variant_glb.stat().st_ino)
    self.assertEqual(self.original_glb_sha256, row["source"]["glbSha256"])
```

```bash
python3 -m unittest -v \
  scripts.test_synty_palette_alternatives \
  scripts.test_build_synty_palette_variants \
  scripts.test_prepare_asset_review
```

Expected: all pass, including the existing derived-config test that proves `materialOverrides` are retained.
- [ ] **Step 5: Commit the independently reviewable descriptor/preparation change**

```bash
git add scripts/synty_palette_alternatives.py scripts/test_synty_palette_alternatives.py \
  scripts/build_synty_palette_variants.py scripts/test_build_synty_palette_variants.py \
  scripts/prepare_asset_review.py scripts/test_prepare_asset_review.py
git diff --cached --check
git commit -m "feat: prepare audited palette alternatives"
```

## Task 2: Parse, resolve, and promote schema-v2 selections with cumulative authority

**Files:**
- Create: `scripts/world_asset_cumulative_metadata.py`
- Create: `scripts/test_world_asset_cumulative_metadata.py`
- Modify: `scripts/world_asset_review.py`
- Modify: `scripts/test_world_asset_review.py`
- Modify: `scripts/promote_world_assets.py`
- Modify: `scripts/test_promote_world_assets.py`
- Modify: `scripts/test_build_web_asset_catalog.py`
- Modify: `docs/human/asset-ingestion/world-asset-promotion.md`
- Modify: `docs/human/asset-ingestion/asset-review-lab.md`
**Interfaces:**
- Consumes: Task 1 `resolve_palette_alternative(pack_root, selection, *, pack_slug, pack_version, source_path, source_glb_sha256) -> Path`, existing `resolve_world_asset_source`, normalizer/image inspectors, inventory/catalog/stats generators, and atomic apply.
- Produces: `WorldAssetPaletteSelection`, optional `WorldAssetPromotionEntry.palette_selection`, v1/v2 `load_world_asset_batch(path: Path) -> WorldAssetBatch`, and `refresh_live_provider_metadata(release_root: Path, current_bytes: bytes) -> bytes`. Schema-v1 entries still resolve their canonical trusted GLB; schema-v2 entries resolve `selectedGlb` only after the complete descriptor binding validates.
- [ ] **Step 1: Write parser/resolver tests that fail on the current schema-v1-only code**

```python
def test_schema_v2_keeps_original_and_selected_glb_identity_separate(self):
    batch = self.load(provider_document_v2(self.palette_entry))
    entry = batch.entries[0]
    self.assertEqual(self.original_sha, entry.source.glb_sha256)
    self.assertEqual(self.selected_sha, entry.palette_selection.selected_glb.sha256)
    self.assertEqual(self.selected_path, resolve_world_asset_source(entry, self.cache, self.review_config))
def test_schema_v1_dark_fortress_still_resolves_existing_default_c_bytes(self):
    batch = self.load(provider_document(copy.deepcopy(VALID_PROVIDER_ENTRY)))
    self.assertEqual(1, batch.schema_version)
    self.assertIsNone(batch.entries[0].palette_selection)
    self.assertEqual(self.canonical_default_c, resolve_world_asset_source(batch.entries[0], self.cache, self.review_config))
```

Reject mixed/unknown schema keys, v2 entries without `paletteSelection`, and changes to descriptor version/comparison/palette/descriptor hash/config hash/atlas/selected path/hash. Assert canonical bytes for an existing v1 fixture are unchanged.
- [ ] **Step 2: Run provider tests red**

```bash
python3 -m unittest -v scripts.test_world_asset_review scripts.test_promote_world_assets
```

Expected: v2 is rejected because `batch.schemaVersion` currently requires integer 1.
- [ ] **Step 3: Add schema-v2 parsing and selected-byte promotion by delegation**
Add an exact nested dataclass matching the approved JSON. For schema 1, retain the current exact entry keys and return `palette_selection=None`. For schema 2, require `paletteSelection` on every entry and validate exact keys/types before delegating all cache/config/atlas/material/containment/hash checks to Task 1's resolver. Do not copy descriptor validation into `world_asset_review.py` or `promote_world_assets.py`.
Update `_entry_document` and `canonical_recipe_bytes` to serialize the matching version. Update `_source_for_entry` so original manifest trust remains mandatory, then choose the validated selected GLB for normalization. Existing `run_blender_normalizer`, `planned_runtime_image_facts`, `inspect_normalized_glb`, catalog generation, and `atomic_apply_targets` remain unchanged. Add `selectedAppearance` only to schema-v2 receipt entries; retain existing v1 receipt shape and semantics.
- [ ] **Step 4: Make cumulative count/hash movement generated, narrow, and independently checked**
`refresh_live_provider_metadata` must parse #117, replace only `providerMetadata.inventory` and `.meshStats` with path/size/hash/count/tree facts computed from the staged files, preserve every other JSON value, and return canonical bytes. Add `evidence/117-all-race-hair/verification.json` to Task 2's promotion targets. Validation recomputes those facts from files; it must not compare generated output with a second copy generated from itself.
In `test_build_web_asset_catalog.py`, remove moving assertions against literal `3225` and `472607…`, but keep:

```python
def test_world_asset_authority_matches_tree_catalog_and_complete_inventory(self):
    _document, inventory_rows = stage.load_complete_inventory(COMPLETE_INVENTORY)
    actual_tree_rows = stage.inventory(MODELS)
    expected_world_paths = runtime_paths_from_tracked_world_asset_recipes(REPO_ROOT)
    self.assertEqual(actual_tree_rows, inventory_rows)              # filesystem vs inventory
    self.assertEqual(expected_world_paths, _world_asset_runtime_paths(WORLD_ASSET_CATALOG))
    self.assertEqual(expected_world_paths,
                     {r["path"] for r in inventory_rows if r["path"].startswith("world-assets/")})
```

Keep the historical baseline/path-partition hashes and #99/#110/#113 seals unchanged. Existing modular-race and specialist-provider closure tests continue to assert their historical receipt values while reading current inventory/mesh hashes through #117's designated live pointers.
- [ ] **Step 5: Prove selected bytes and live pointers through the full synthetic transaction**
Add a schema-v2 promotion fixture where original and selected GLBs differ. Assert the fake normalizer receives the selected path, output/receipt hashes bind it, auxiliary image count survives, stage validation detects descriptor drift, rollback restores all prior bytes, and v1 promotion produces its previous canonical recipe/default output.

```bash
python3 -m unittest -v \
  scripts.test_world_asset_review \
  scripts.test_promote_world_assets \
  scripts.test_world_asset_cumulative_metadata \
  scripts.test_build_web_asset_catalog \
  scripts.test_modular_race_class_contract \
  scripts.test_specialist_weapon_provider_closure
python3 scripts/build_synty_complete_inventory.py --check
git diff --check
```

Expected: all pass; the tracked count/hash remain unchanged in this implementation-only test run because no real batch is promoted.
- [ ] **Step 6: Document and commit the provider contract**
Document schema 1 as canonical/default and schema 2 as exact selected appearance. Include one concise example with both original `source.glbSha256` and selected `paletteSelection.selectedGlb.sha256`; state that #117's two live pointers are generated transaction targets, not hand-edited historical evidence.

```bash
git add scripts/world_asset_review.py scripts/test_world_asset_review.py \
  scripts/promote_world_assets.py scripts/test_promote_world_assets.py \
  scripts/world_asset_cumulative_metadata.py scripts/test_world_asset_cumulative_metadata.py \
  scripts/test_build_web_asset_catalog.py docs/human/asset-ingestion/world-asset-promotion.md \
  docs/human/asset-ingestion/asset-review-lab.md
git diff --cached --check
git commit -m "feat: promote selected palette bytes"
```

After Task 2, run the Assets repository's full documented unit suite and open one Assets PR. Merge it by human approval before Task 3 begins.

## Task 3: Deliver the single-dropdown Lab and hash-bound exports

**Files:**
- Modify: `src/dev/asset-review/model.ts`
- Modify: `src/dev/asset-review/model.test.ts`
- Modify: `src/dev/asset-review/AssetReviewLab.tsx`
- Modify: `src/dev/asset-review/AssetReviewLab.test.tsx`
- Modify: `src/dev/asset-review/AssetReviewScene.tsx`
- Modify: `src/dev/asset-review/AssetReviewLab.css`
**Interfaces:**
- Consumes: Task 1 schema-v2 candidate `paletteAlternatives` and Task 2 provider `paletteSelection` shape.
- Produces: `appearanceIdentity(entry: AssetReviewEntry) -> string`, `entryPreviewUrl(entry: AssetReviewEntry) -> string`, `selectPaletteAppearance(entry: AssetReviewEntry, comparisonId: string, palette: string) -> AssetReviewEntry`, `recordPreviewLoad(entry: AssetReviewEntry, url: string, status: AssetReviewLoadStatus) -> AssetReviewEntry`, `setBatchId(batch: AssetReviewBatch, value: string) -> AssetReviewBatch`, and `generateBatchId(referencePack: string, uuid?: string, date?: Date) -> string`.
- [ ] **Step 1: Add failing model tests for v1 compatibility and complete v2 identity**

```ts
it('demotes Ready and clears load success when palette or descriptor bytes change', () => {
  const ready = readyPaletteEntry('A');
  const selected = selectPaletteAppearance(ready, 'braziers', 'B');
  expect(selected).toMatchObject({ decision: 'keep', loadedSuccessfully: false });
  expect(appearanceIdentity(selected)).toContain(PALETTE_B_SELECTED_SHA);
  expect(entryPreviewUrl(selected)).toBe(PALETTE_B_URL);
});
it('never restores Ready when imported appearance identity is stale', () => {
  const imported = { ...paletteBatch('A'), entries: [readyPaletteEntry('A')] };
  const current = paletteCatalog({ descriptorSha256: CHANGED_DESCRIPTOR_SHA });
  const merged = mergeCatalogWithReview(current, imported);
  expect(merged.batch.entries[0]?.decision).toBe('keep');
  expect(merged.batch.entries[0]?.loadedSuccessfully).toBe(false);
  expect(merged.staleAppearanceKeys).toHaveLength(1);
});
```

Also cover: strict unknown/missing nested keys, selected path/hash mismatch, stale config/atlas hashes, v1 import/export byte semantics, schema-v2 Ready-only output, original source hash retained, duplicate refs, invalid batch edits, and deterministic UUID injection.
- [ ] **Step 2: Run model tests red**

```bash
npm test -- --run src/dev/asset-review/model.test.ts
```

Expected: failure because palette types/helpers and schema-v2 parsing do not exist.
- [ ] **Step 3: Implement strict model transitions before UI wiring**
Parse schema 1 with the current exact keys. Parse schema 2 with exact `paletteAlternatives`/`paletteSelection` keys and lower-case hashes, normalized cache-relative paths, a selected GLB URL whose 12-character prefix matches the selected hash, and one selected option present byte-for-byte in the current candidate alternatives.
Use physical source key only to locate the same candidate during import; use `appearanceIdentity` to decide whether `decision: ready` and `loadedSuccessfully` may survive. When identities differ, retain useful draft metadata and report the stale appearance, but force Keep/false. `recordPreviewLoad` ignores callbacks for a URL other than `entryPreviewUrl(entry)`; loading and error both clear success and demote Ready. `selectPaletteAppearance` performs the same invalidation even when only descriptor/config/atlas hashes changed.
Keep `ASSET_REVIEW_STORAGE_KEY = 'rpg.asset-review.batch.v1'` so old local progress can be parsed and merged. Schema-v2 serialization includes nested selection and never local URL. `generateBatchId` returns `REFERENCE-world-assets-YYYYMMDD-UUID`, uses `crypto.randomUUID()` and the current date by default, permits both inputs to be injected in tests, and validates through `setBatchId`.
- [ ] **Step 4: Wire one dropdown, batch controls, and exact scene load handling**
Render one `<select aria-label="Palette">` for candidates with alternatives; options are the descriptor's sorted configured palettes. Do not render texture upload, color chips, or additional published refs. Display original and selected SHA-256 separately in Prepared source. Pass `entryPreviewUrl(activeEntry)` to `AssetReviewScene`.
Add `<input aria-label="Batch ID">` and `<button>Generate batch ID</button>` near export actions. Disable exports for invalid batch IDs and show the existing field-error style. Route every scene callback through `recordPreviewLoad`; on `loading`, success from the previous appearance must disappear immediately.
- [ ] **Step 5: Add UI regressions and run green**

```tsx
it('offers one palette control and requires the newly selected GLB to load', async () => {
  await renderPaletteLab();
  expect(screen.getAllByLabelText('Palette')).toHaveLength(1);
  fireEvent.click(screen.getByRole('button', { name: 'Report scene success' }));
  fireEvent.click(screen.getByRole('button', { name: 'Mark Ready' }));
  fireEvent.change(screen.getByLabelText('Palette'), { target: { value: 'B' } });
  expect(screen.getByTestId('current-decision')).toHaveTextContent('Keep');
  expect(screen.getByRole('button', { name: 'Mark Ready' })).toBeDisabled();
  expect(screen.getByTestId('asset-review-scene')).toHaveAttribute('data-url', PALETTE_B_URL);
});
```

Include a late-success test for palette A after B is selected, an error-after-Ready test, import with changed descriptor hash, editable batch ID, generated collision-safe ID, and schema-v2 provider download assertions.

```bash
npm test -- --run src/dev/asset-review/model.test.ts src/dev/asset-review/AssetReviewLab.test.tsx
npm run typecheck
npm run lint -- --quiet
npm run format:check -- src/dev/asset-review
```

Expected: all pass with no generated thumbnail changes.
- [ ] **Step 6: Commit and perform the human palette acceptance before opening the Web PR**

```bash
git add src/dev/asset-review/model.ts src/dev/asset-review/model.test.ts \
  src/dev/asset-review/AssetReviewLab.tsx src/dev/asset-review/AssetReviewLab.test.tsx \
  src/dev/asset-review/AssetReviewScene.tsx src/dev/asset-review/AssetReviewLab.css
git diff --cached --check
git commit -m "feat: review exact palette appearances"
```

Prepare a tiny, unpromoted, trusted Dark Fortress source with A/B/C alternatives. In the loopback Lab verify auxiliary chain color/normal/wrapping and slot assignment remain visible, select a nondefault palette, mark Ready only after its exact URL succeeds, export schema v2, then change palette and confirm immediate Keep. Record hashes and textual visual verdict outside the Web repository; do not commit licensed screenshots or GLBs.

## Validation and Self-review Gate

| Requirement | Primary proof |
| --- | --- |
| Configured/audited/prebuilt only; no suffixed-manifest trust | Task 1 descriptor drift/audit tests |
| Original versus selected identity; one ref | Tasks 2–3 schema/resolution/export tests |
| v1/default-C recipes and bytes preserved | Task 2 v1 canonical and transaction regression |
| Failed/stale loads and palette/hash edits invalidate Ready | Task 3 model/UI tests |
| Editable/generated batch IDs | Task 3 helper/UI tests |
| Auxiliary materials/normal/wrap/slots unchanged | Task 1 binding record plus tiny human GLB proof |
| Existing normalizer/transaction/helpers reused | Task 2 call path and unchanged engine tests |
| Cumulative facts move without rewriting history | Task 2 pointer/partition/#99/#113 tests |
| Thumbnails unchanged | Web diff contains no thumbnail contract or generated thumbnail edits |

Before requesting review, search both plans and diffs for placeholder language; compare every `paletteSelection` property and helper signature between Python, TypeScript, examples, and tests; run local Markdown-link/code-fence validation from the plan index; and inspect `git diff --check` plus `git status --short` in each worktree. Planning completion does not mark implementation complete.
