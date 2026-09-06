# Bulk World-Asset Review and Promotion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:subagent-driven-development` or `superpowers:executing-plans` to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. Delegation must remain bounded and coordinator-controlled, with one writer per worktree and explicit checkpoints between task batches.

**Goal:** Process all configured Dark Fortress world-building sources under palette C, review cached GLBs in scalable browser batches, and publish explicitly Ready selections into the World Builder through a deterministic private provider.

**Architecture:** `rpg-game-assets` owns material preflight, trusted/review discovery outputs, source-glob queue preparation, strict provider recipes, Blender normalization, and atomic publication. `rpg-dnd5e-web` owns the loopback-only Asset Review Lab and consumes a generated license-safe catalog in the existing world-building concept. Review refs are mechanically derived as `dnd5e:{props|items|weapons|env}:dark-fortress:{lowercase_source_suffix}`; Toolkit/gameplay mappings remain separate.

**Tech Stack:** Python 3.14 stdlib, Pillow, Blender 5.x Python, JSON, React 19, TypeScript 5.8, React Three Fiber, Three.js, Vite, Vitest, Testing Library, existing Synty pack and static-release tooling.

**Spec:** `ideas/assets/world-asset-review/design.md`

## Global Constraints

- Bounded subagents are allowed where isolation improves correctness, especially across Python and TypeScript contexts. The coordinator retains control, uses one writer per worktree, and does not launch unattended autonomous review/fix loops.
- Integration is PR-only; never merge locally or push directly to protected branches.
- Create one implementation issue and one issue branch per owning repository for this wave.
- Assets branches start from fresh `origin/main`; Web branches start from fresh `origin/dev`.
- Always use isolated worktrees. The user has made this the standing preference.
- Finish and merge `rpg-game-assets#160` before creating the new Assets implementation branch.
- Do not merge the Web consumer before its generated catalog is bound to a merged provider commit.
- Dark Fortress palette C is the canonical main atlas for this workflow.
- Unknown custom materials never fall back to C and never become Ready.
- Raw archives remain immutable, local, and never automatically deleted.
- Discovery GLBs and individual render views remain ignored; tracked sheets and portable manifests remain private browsing evidence.
- No licensed GLB may be tracked by the public `rpg-dnd5e-web` repository.
- Review refs are exact four-part visual identities. They do not define Toolkit behavior.
- Category is exactly one of `props`, `items`, `weapons`, or `env`.
- `items` means inventory candidate and `weapons` means equipment candidate; neither grants gameplay behavior.
- Ready is explicit human approval. Valid fields alone never transition an entry to Ready.
- Provider export contains Ready entries only.
- Provider publication is stage-first and batch-atomic.
- Existing prop, weapon, environment, and composition refs/providers remain valid and unchanged.
- Unsupported exact refs render empty and report a diagnostic rather than substituting another visual.
- Preserve unrelated `rpg-game-assets/placed/`, local Polygon Dungeon output/config, `rpg-project/active.md`, and Web `src/generated/characterCustomizationCatalog.ts`.
- Copilot remains disabled. Substantive PRs receive one fresh independent final review at their final heads, with the verdict published on each PR.

## Repository and branch preparation

Before Task 1 implementation begins:

1. Complete Task 0 on the existing `feat/159-brazier-material-proof` branch and merge Assets PR #160 through GitHub.
2. Update canonical Assets `main` with `git fetch origin` and a fast-forward only update; do not touch the untracked Polygon Dungeon files.
3. Create one boarded Assets implementation issue under rpg-project#394 and one boarded Web implementation issue under the same design slice. Capture their returned numeric issue IDs in `ASSETS_ISSUE` and `WEB_ISSUE`.
4. Create external isolated worktrees because the repository-local `.worktrees/` roots may contain active or unignored work:

```bash
test "$ASSETS_ISSUE" -gt 0
test "$WEB_ISSUE" -gt 0

git -C "$HOME/game-dev/rpg-game-assets" fetch origin --prune
git -C "$HOME/game-dev/rpg-game-assets" worktree add \
  "$HOME/.pi/worktrees/rpg-game-assets/${ASSETS_ISSUE}-world-asset-review" \
  -b "feat/${ASSETS_ISSUE}-world-asset-review" origin/main

git -C "$HOME/game-dev/rpg-dnd5e-web" fetch origin --prune
git -C "$HOME/game-dev/rpg-dnd5e-web" worktree add \
  "$HOME/.pi/worktrees/rpg-dnd5e-web/${WEB_ISSUE}-asset-review" \
  -b "feat/${WEB_ISSUE}-asset-review" origin/dev
```

5. Establish clean baselines before editing:

```bash
cd "$HOME/.pi/worktrees/rpg-game-assets/${ASSETS_ISSUE}-world-asset-review"
python3 -m unittest discover -s scripts -p 'test_*.py'
python3 scripts/build_synty_complete_inventory.py --check

cd "$HOME/.pi/worktrees/rpg-dnd5e-web/${WEB_ISSUE}-asset-review"
npm ci
npm run ci-check
```

If either baseline fails, stop and record the exact failure instead of attributing it to this journey.

6. Do not begin the Web catalog-consumer task until web#935's world-building concept files are present on `origin/dev`. If they are still unmerged, keep the Asset Review Lab work on the issue branch and wait rather than stacking across a branch another operator controls.

## File structure

### `rpg-game-assets`

- `scripts/synty_material_preflight.py` — parse vendor material declarations and classify configured conversion jobs.
- `scripts/test_synty_material_preflight.py` — synthetic parser/classification/report tests.
- `scripts/fbx_to_neutral_review_glb.py` — Blender-only geometry preview exporter for blocked material/FX sources.
- `scripts/test_fbx_to_neutral_review_glb.py` — Blender acceptance for neutral output and exclusion from trusted manifests.
- `scripts/synty_library_transaction.py` — stage, validate, and atomically replace complete discovery surfaces.
- `scripts/test_synty_library_transaction.py` — stage purity, inventory, rollback, and drift tests.
- `scripts/build_synty_pack_library.py` — add material-aware stage/apply/check orchestration while preserving existing ordinary runs.
- `scripts/test_build_synty_pack_library.py` — command construction and policy tests.
- `scripts/configs/synty-packs/polygon-dark-fortress.json` — palette C, all world groups, material declaration policy, ref prefixes, and optional suffix overrides.
- `scripts/configs/world-asset-review/polygon-dark-fortress.json` — review-only pack identity, category-prefix suggestions, and exact suffix overrides; deliberately separate from conversion hash authority.
- `scripts/world_asset_review.py` — category/ref derivation and strict review/provider schema types shared by Assets tools.
- `scripts/test_world_asset_review.py` — ref, collision, state-independent provider recipe tests.
- `scripts/prepare_asset_review.py` — source-glob selection, hash verification, reflink/copy, and atomic ignored Web catalog preparation.
- `scripts/test_prepare_asset_review.py` — path, glob, hash, destination, disk, and idempotency tests.
- `scripts/normalize_world_asset_for_runtime.py` — calibrated static GLB normalization preserving reviewed multi-material content.
- `scripts/test_normalize_world_asset_for_runtime.py` — synthetic one- and multi-material Blender acceptance.
- `scripts/build_world_asset_catalog.py` — deterministic provider and Web-safe manifest projection.
- `scripts/test_build_world_asset_catalog.py` — strict allowlist, ref/path/hash, and determinism tests.
- `scripts/promote_world_assets.py` — stage, validate, apply, check orchestration.
- `scripts/test_promote_world_assets.py` — source binding, atomicity, rollback, and drift tests.
- `docs/human/asset-ingestion/material-preflight.md` — material inspection lesson.
- `docs/human/asset-ingestion/asset-review-lab.md` — batch review lesson.
- `docs/human/asset-ingestion/world-asset-promotion.md` — provider custody lesson.
- `library/polygon-dark-fortress/v3/material-preflight.json` — deterministic portable audit.
- `library/polygon-dark-fortress/v3/material-preflight.md` — human summary.
- `harness/models/synty/world-assets/{props,items,weapons,env}/dark-fortress/` — promoted private GLBs.
- `harness/catalogs/synty-world-assets.json` — generated private consumer catalog.
- `scripts/configs/world-asset-promotion/dark-fortress-world-assets-v1.json` — first canonical Ready recipe; later batches use their reviewed batch IDs.
- `evidence/world-asset-promotions/dark-fortress-world-assets-v1/receipt.json` — first deterministic provider receipt.

### `rpg-dnd5e-web`

- `src/dev/asset-review/model.ts` — strict catalog/draft/provider types, transitions, validation, filtering, and serialization.
- `src/dev/asset-review/model.test.ts` — pure state and schema tests.
- `src/dev/asset-review/route.ts` — loopback/development/query gate.
- `src/dev/asset-review/route.test.ts` — route matrix.
- `src/dev/asset-review/disposeObjectResources.ts` — review-specific geometry/material/texture cleanup.
- `src/dev/asset-review/disposeObjectResources.test.ts` — disposal ownership tests.
- `src/dev/asset-review/AssetReviewScene.tsx` — one-model-at-a-time GLTFLoader scene.
- `src/dev/asset-review/AssetReviewLab.tsx` — candidate drawer, calibration sheet, decisions, import/export, autosave.
- `src/dev/asset-review/AssetReviewLab.css` — review layout and status styling.
- `src/dev/asset-review/AssetReviewLab.test.tsx` — navigation, state, filtering, and export tests.
- `src/App.tsx` and `src/App.test.tsx` — lazy local-only route.
- `scripts/generate-world-asset-catalog.mjs` — private provider catalog to committed TypeScript projection.
- `scripts/generateWorldAssetCatalog.test.ts` — generator/hash/path tests.
- `scripts/sync-game-assets.sh` — invoke the generator after private synchronization.
- `package.json` — `world-assets:sync` and `world-assets:check` commands.
- `src/generated/worldAssetCatalog.ts` — generated license-safe visual metadata.
- `src/components/hex-grid/WorldAssetModel.tsx` — generic exact-ref visual resolver/renderer.
- `src/components/hex-grid/WorldAssetModel.test.tsx` — exact resolution and unsupported-ref tests.
- `src/concepts/world-building/catalog.ts` — merge legacy and generated world-building entries.
- `src/concepts/world-building/serialization.ts` — validate against the combined catalog.
- `src/concepts/world-building/WorldBuildingViewport.tsx` — render new exact visuals through `WorldAssetModel`.
- Focused existing world-building tests — generated palette, drag/drop, save/reopen, and unsupported refs.

---

### Task 0: Finish the palette-comparison prerequisite on Assets PR #160

**Repository:** `rpg-game-assets`

**Worktree:** `/home/kirk/game-dev/rpg-game-assets/.worktrees/159-brazier-material-proof`

**Files:**
- Modify: `scripts/build_synty_palette_variants.py`
- Modify: `scripts/test_build_synty_palette_variants.py`
- Modify: `docs/human/asset-ingestion/pack-library.md`
- Modify: `docs/superpowers/plans/2026-09-06-brazier-material-proof.md`
- Move: `library/polygon-dark-fortress/v3/palette-variants/{A,B,C}/**`
- To: `library/polygon-dark-fortress/v3/palette-variants/braziers/{A,B,C}/**`

**Interfaces:**
- Consumes: required `--comparison-id braziers` in addition to the existing source root, pack root, group, match, and palettes.
- Produces: `palette-variants/{comparison-id}/{palette}/` with a descriptor containing the exact comparison ID.

- [ ] **Step 1: Remove the obsolete uncommitted per-source-atlas plan extension**

Read the worktree diff and remove only the appended `sourceAtlasOverrides` task from `docs/superpowers/plans/2026-09-06-brazier-material-proof.md`. Preserve every committed plan section and every other file.

Run:

```bash
cd /home/kirk/game-dev/rpg-game-assets/.worktrees/159-brazier-material-proof
git diff -- docs/superpowers/plans/2026-09-06-brazier-material-proof.md
git status --short
```

Expected: the stale uncommitted task is identified before removal; no unrelated path is changed.

- [ ] **Step 2: Write failing comparison-namespace tests**

Add tests that construct two commands and assert their roots cannot collide:

```python
self.assertEqual(
    Path("/pack/palette-variants/braziers/A"),
    variant_output_root(Path("/pack"), "braziers", "A"),
)
self.assertEqual(
    Path("/pack/palette-variants/plushies/A"),
    variant_output_root(Path("/pack"), "plushies", "A"),
)
with self.assertRaisesRegex(ValueError, "comparison ID"):
    safe_comparison_id("../braziers")
```

Assert that `pipeline_command()` includes the comparison ID in its pack root and that the generated `palette.json` contains `"comparisonId": "braziers"`.

- [ ] **Step 3: Run the focused test and verify failure**

Run:

```bash
python3 scripts/test_build_synty_palette_variants.py
```

Expected: FAIL because `variant_output_root` and `safe_comparison_id` do not exist and output currently stops at `palette-variants/A`.

- [ ] **Step 4: Implement required comparison namespacing**

Add:

```python
def safe_comparison_id(value: str) -> str:
    if not value or not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9_-]*", value):
        raise ValueError("comparison ID must be filename-safe")
    return value


def variant_output_root(pack_root: Path, comparison_id: str, palette: str) -> Path:
    return Path(pack_root) / "palette-variants" / safe_comparison_id(comparison_id) / palette
```

Add required CLI option `--comparison-id`. Route both the pack-runner destination and descriptor destination through `variant_output_root()`. Include `comparisonId` in the descriptor.

- [ ] **Step 5: Migrate the tracked brazier comparison**

Move the existing A/B/C descriptors, sheet manifests, and sheets beneath `palette-variants/braziers/`. Regenerate them with:

```bash
python3 scripts/build_synty_palette_variants.py \
  --source-root "$HOME/Downloads/synty/POLYGON_Dark_Fortress_SourceFiles_v3" \
  --pack-root library/polygon-dark-fortress/v3 \
  --config scripts/configs/synty-packs/polygon-dark-fortress.json \
  --group props \
  --comparison-id braziers \
  --match 'SourceFiles/DarkFortress/FBX/SM_Prop_Brazier_*.fbx' \
  --palette A --palette B --palette C \
  --resume
```

Expected: 7 models × 3 palettes, three descriptors under `palette-variants/braziers/`, and no tracked top-level `palette-variants/{A,B,C}` files.

- [ ] **Step 6: Update the learning command and run all Assets gates**

Document why the comparison ID is custody rather than presentation. Run:

```bash
python3 scripts/test_build_synty_palette_variants.py
python3 -m unittest discover -s scripts -p 'test_*.py'
python3 scripts/build_synty_complete_inventory.py --check
git diff --check
git status --short
```

Expected: PASS; the complete inventory check passes, and only intended #160 paths are modified.

- [ ] **Step 7: Commit, push, review, and merge #160**

```bash
git add scripts/build_synty_palette_variants.py \
  scripts/test_build_synty_palette_variants.py \
  docs/human/asset-ingestion/pack-library.md \
  docs/superpowers/plans/2026-09-06-brazier-material-proof.md \
  library/polygon-dark-fortress/v3/palette-variants
git commit -m "fix: namespace Synty palette comparisons"
git push
```

Run one fresh independent final review on the pushed head and publish its verdict on PR #160. Merge only after the review is closed and Kirk accepts the current tracked comparison evidence.

---

### Task 1: Parse and classify Synty material declarations

**Repository:** `rpg-game-assets`

**Files:**
- Create: `scripts/synty_material_preflight.py`
- Create: `scripts/test_synty_material_preflight.py`
- Modify: `scripts/convert_synty_pack.py`
- Modify: `scripts/test_convert_synty_pack.py`
- Modify: `scripts/configs/synty-packs/polygon-dark-fortress.json`

**Interfaces:**
- Consumes: configured `materialPreflight.declarationPath`, `defaultMaterialNames`, conversion jobs, and resolved `materialOverrides`.
- Produces: `parse_material_declarations(path: Path) -> tuple[PrefabMaterials, ...]`, `audit_group(source_root: Path, config: Mapping[str, object], group: str) -> MaterialAudit`, and canonical JSON/Markdown report bytes.

- [ ] **Step 1: Write a synthetic vendor-material fixture**

Use `TemporaryDirectory` and write:

```text
-------------------
Folder Name: Props
-------------------
Prefab Name: SM_Prop_Table_01
    Mesh Name: SM_Prop_Table_01
        Slot: PolygonDarkFortress_Mat_01_A (Uses custom shader)

Prefab Name: SM_Prop_Brazier_02
    Mesh Name: SM_Prop_Brazier_02
        Slot: PolygonDarkFortress_Mat_01_A (Uses custom shader)
        Slot: Chains_04 (Uses custom shader)

Prefab Name: SM_Prop_Bottle_01
    Mesh Name: SM_Prop_Bottle_01
        Slot: PolygonDarkFortress_Mat_01_A (Uses custom shader)
        Slot: Glass_02 (Uses custom shader)
```

Create corresponding empty FBX fixture files and config jobs.

- [ ] **Step 2: Write failing parser and audit tests**

Assert exact ordered parsing and these statuses:

```python
self.assertEqual("ready-default", rows["SM_Prop_Table_01"].status)
self.assertEqual("ready-explicit", rows["SM_Prop_Brazier_02"].status)
self.assertEqual("review-required", rows["SM_Prop_Bottle_01"].status)
self.assertEqual("Glass_02", rows["SM_Prop_Bottle_01"].blocking_slots[0].material_name)
```

Also assert rejection of duplicate prefab names, slots before a mesh, meshes before a prefab, empty declarations, missing selected prefabs, mismatched override material names, out-of-range slots, and overrides targeting undeclared objects.

- [ ] **Step 3: Run the test and verify failure**

Run:

```bash
python3 scripts/test_synty_material_preflight.py
```

Expected: FAIL because `synty_material_preflight` does not exist.

- [ ] **Step 4: Implement immutable declaration and audit types**

Define:

```python
@dataclass(frozen=True)
class DeclaredSlot:
    index: int
    material_name: str

@dataclass(frozen=True)
class DeclaredMesh:
    name: str
    slots: tuple[DeclaredSlot, ...]

@dataclass(frozen=True)
class PrefabMaterials:
    folder: str
    prefab_name: str
    meshes: tuple[DeclaredMesh, ...]

@dataclass(frozen=True)
class AuditedSlot:
    index: int
    material_name: str
    status: Literal["default-atlas", "explicit", "review-required", "unsupported"]
    reason: str

@dataclass(frozen=True)
class AuditedMesh:
    name: str
    slots: tuple[AuditedSlot, ...]

@dataclass(frozen=True)
class AuditedSource:
    source_path: PurePosixPath
    group: str
    status: Literal["ready-default", "ready-explicit", "review-required", "unsupported"]
    meshes: tuple[AuditedMesh, ...]
    reasons: tuple[str, ...]

@dataclass(frozen=True)
class MaterialAudit:
    pack_slug: str
    group: str
    sources: tuple[AuditedSource, ...]
```

Parse slot index from declaration order within each mesh. Match a direct conversion job to the declaration whose `Prefab Name` equals `job.review_relative.stem`. Treat only exact configured `defaultMaterialNames` as atlas-compatible. Resolve non-default slots only through an override matching source path, mesh/object name, index, and declared material name.

- [ ] **Step 5: Add strict pack-config fields**

Extend `load_config()` allowed keys with `materialPreflight` and validate this shape:

```json
{
  "declarationPath": "SourceFiles/DarkFortress/MaterialList_PolygonDarkFortress.txt",
  "defaultMaterialNames": [
    "PolygonDarkFortress_Mat_01_A",
    "PolygonDarkFortress_Mat_01_B",
    "PolygonDarkFortress_Mat_01_C"
  ]
}
```

Paths must be normalized relative POSIX paths. Names must be non-empty and unique. `defaultAtlas` remains required to equal one named atlas variant.

- [ ] **Step 6: Implement deterministic reports**

Export canonical JSON with each source, mesh, slot, classification, and reason. Export Markdown with count summary followed by unresolved material families and exact sources. Sort every collection by source path, object name, and slot index.

- [ ] **Step 7: Run focused tests and commit**

Run:

```bash
python3 scripts/test_synty_material_preflight.py
python3 scripts/test_convert_synty_pack.py
python3 -m py_compile scripts/synty_material_preflight.py scripts/convert_synty_pack.py
git diff --check
```

Expected: PASS.

Commit:

```bash
git add scripts/synty_material_preflight.py \
  scripts/test_synty_material_preflight.py \
  scripts/convert_synty_pack.py \
  scripts/test_convert_synty_pack.py \
  scripts/configs/synty-packs/polygon-dark-fortress.json
git commit -m "feat: audit Synty materials before conversion"
```

---

### Task 2: Add all Dark Fortress world groups and honest neutral review output

**Repository:** `rpg-game-assets`

**Files:**
- Modify: `scripts/configs/synty-packs/polygon-dark-fortress.json`
- Create: `scripts/fbx_to_neutral_review_glb.py`
- Create: `scripts/test_fbx_to_neutral_review_glb.py`
- Modify: `scripts/convert_synty_pack.py`
- Modify: `scripts/test_convert_synty_pack.py`
- Modify: `scripts/render_glb_previews.py`
- Modify: `scripts/build_preview_sheets.py`
- Modify: `scripts/test_build_preview_sheets.py`

**Interfaces:**
- Consumes: MaterialAudit from Task 1.
- Produces: trusted conversion manifests containing only ready sources and review-only manifests containing neutral or static-review sources with blocking labels.

- [ ] **Step 1: Write failing group-count and palette tests**

Build a synthetic config and assert configured patterns map as follows:

```python
expected = {
    "environment": ("SM_Env_*.fbx", "SM_Bld_*.fbx", "SM_Veh_*.fbx"),
    "props": ("SM_Prop_*.fbx",),
    "items": ("SM_Chr_Attach_*.fbx",),
    "weapons": ("SM_Wep_*.fbx",),
    "effects": ("FX_*.fbx",),
}
```

Assert the Dark Fortress config's `defaultAtlas` ends with `PolygonDarkFortress_Texture_01_C.png`. Assert a `review-required` job never enters a trusted manifest.

- [ ] **Step 2: Add the complete Dark Fortress group configuration**

Use these exact include patterns under `SourceFiles/DarkFortress/FBX/`:

```json
{
  "environment": {
    "include": ["SourceFiles/DarkFortress/FBX/SM_Env_*.fbx", "SourceFiles/DarkFortress/FBX/SM_Bld_*.fbx", "SourceFiles/DarkFortress/FBX/SM_Veh_*.fbx"],
    "exclude": ["SourceFiles/DarkFortress/FBX/Collision/*.fbx"]
  },
  "props": {
    "include": ["SourceFiles/DarkFortress/FBX/SM_Prop_*.fbx"],
    "exclude": ["SourceFiles/DarkFortress/FBX/Collision/*.fbx"]
  },
  "items": {
    "include": ["SourceFiles/DarkFortress/FBX/SM_Chr_Attach_*.fbx"],
    "exclude": []
  },
  "weapons": {
    "include": ["SourceFiles/DarkFortress/FBX/SM_Wep_*.fbx"],
    "exclude": []
  },
  "effects": {
    "include": ["SourceFiles/DarkFortress/FBX/FX_*.fbx"],
    "exclude": []
  }
}
```

Preserve each group's optional preview yaw. Add a pure private acceptance assertion that the real source root plans 429 environment + 352 props + 21 items + 17 weapons + 6 effects = 825 review identities.

- [ ] **Step 3: Write the failing neutral-Blender acceptance test**

Generate a tiny synthetic FBX or Blender scene fixture with two source materials. Run the proposed exporter and assert:

- output is a GLB with mesh geometry;
- output has exactly one opaque neutral material;
- output has no source textures, animation, camera, or light;
- bounds remain equal to the imported source bounds within `1e-5`; and
- its manifest status is `material-review-required`, never trusted.

Run:

```bash
blender --background --factory-startup --python scripts/test_fbx_to_neutral_review_glb.py
```

Expected: FAIL because the neutral exporter does not exist.

- [ ] **Step 4: Implement the neutral exporter**

The Blender entry point must:

```python
NEUTRAL_RGBA = (0.38, 0.40, 0.44, 1.0)
```

Import one direct FBX in factory mode, exclude non-renderable collision/helper objects using the existing preview identity rule, clear material slots while preserving polygon slot validity, assign one Principled BSDF neutral material, remove animation/camera/light data, and export one embedded static GLB. It must never write into the trusted `glbs/` root.

- [ ] **Step 5: Partition trusted and review-only conversion**

Add a material-policy input with exact values:

```python
MaterialPolicy = Literal["legacy", "review-all"]
```

`legacy` preserves current behavior for existing packs. `review-all` requires material preflight and routes:

- `ready-default` and `ready-explicit` through ordinary conversion;
- `review-required` through the neutral exporter;
- `unsupported` into the report without fabricated output; and
- every `effects` source through the review-only static/neutral boundary.

Trusted manifests include `materialStatus`. Review manifests include `reviewStatus`, `readyEligible: false`, and exact reasons.

- [ ] **Step 6: Label review previews and sheets**

Carry `readyEligible` and reasons into preview manifests. Render review-only GLBs against the same opposing camera recipe, but add a visible header/badge in generated sheets:

```text
MATERIAL REVIEW REQUIRED
FX / STATIC PREVIEW ONLY
```

Trusted sheets contain neither label. Filename-family grouping must recognize `SM_Chr_Attach` and `FX` in addition to existing `SM_Item`, `SM_Wep`, `SM_Prop`, `SM_Bld`, and `SM_Env` families.

- [ ] **Step 7: Run focused tests and commit**

Run:

```bash
python3 scripts/test_convert_synty_pack.py
python3 scripts/test_build_preview_sheets.py
blender --background --factory-startup --python scripts/test_fbx_to_neutral_review_glb.py
git diff --check
```

Expected: PASS.

Commit:

```bash
git add scripts/configs/synty-packs/polygon-dark-fortress.json \
  scripts/fbx_to_neutral_review_glb.py \
  scripts/test_fbx_to_neutral_review_glb.py \
  scripts/convert_synty_pack.py \
  scripts/test_convert_synty_pack.py \
  scripts/render_glb_previews.py \
  scripts/build_preview_sheets.py \
  scripts/test_build_preview_sheets.py
git commit -m "feat: prepare all Dark Fortress world assets for review"
```

---

### Task 3: Make complete pack-library publication stage-first and atomic

**Repository:** `rpg-game-assets`

**Files:**
- Create: `scripts/synty_library_transaction.py`
- Create: `scripts/test_synty_library_transaction.py`
- Modify: `scripts/build_synty_pack_library.py`
- Modify: `scripts/test_build_synty_pack_library.py`
- Modify: `.gitignore`

**Interfaces:**
- Consumes: all configured groups, material policy, source root, pack root, and a disjoint stage root.
- Produces: `stage_library()`, `validate_library_stage()`, `apply_library_stage()`, and `check_library()` over trusted GLBs, review-only GLBs, previews, sheets, material reports, and manifests.

- [ ] **Step 1: Write failing transaction tests**

Use temporary roots and injected conversion/render/sheet runners. Assert:

- stage changes no canonical file;
- stage requires a path disjoint from both source and pack roots;
- validation requires every configured group and expected manifest;
- a missing model, changed hash, stale sheet, or extra output fails;
- apply replaces the complete discovery surface;
- an injected mid-apply failure restores every prior canonical target; and
- check detects drift without rewriting.

- [ ] **Step 2: Run the tests and verify failure**

Run:

```bash
python3 scripts/test_synty_library_transaction.py
python3 scripts/test_build_synty_pack_library.py
```

Expected: FAIL because the transaction module and material-aware commands do not exist.

- [ ] **Step 3: Implement a complete expected-output inventory**

Represent every target relative to the pack root:

```python
@dataclass(frozen=True)
class LibraryStage:
    release_root: Path
    targets: tuple[PurePosixPath, ...]
```

Targets include trusted `glbs/`, review-only `.material-review-cache/`, disposable `.preview-cache/`, tracked `sheets/`, every group sheet manifest, and material-preflight JSON/Markdown. Validate no symlink component and no unlisted file beneath governed roots.

- [ ] **Step 4: Add explicit commands without breaking current calls**

Preserve the existing ordinary invocation as `legacy` behavior. Add subcommands:

```text
stage     --stage-root PATH --material-policy review-all
validate  --stage-root PATH
apply     --stage-root PATH
check
```

All commands require `--source-root`, `--pack-root`, `--config`, and either `--all-groups` or repeated `--group`. `apply` repeats validation before atomic replacement.

- [ ] **Step 5: Add disk preflight**

Estimate trusted output from current matching manifest sizes when available; estimate unknown conversions from selected source bytes multiplied by a documented conservative factor of `2`. Add expected preview PNG and neutral-review allowance. Require free bytes to be at least estimated bytes plus 10%. Print the estimate and observed free bytes before Blender starts.

Tests inject disk usage and assert exact refusal when `free < ceil(estimate * 1.10)`.

- [ ] **Step 6: Run synthetic transaction gates and commit**

Run:

```bash
python3 scripts/test_synty_library_transaction.py
python3 scripts/test_build_synty_pack_library.py
python3 -m py_compile scripts/synty_library_transaction.py scripts/build_synty_pack_library.py
git diff --check
```

Expected: PASS.

Commit:

```bash
git add .gitignore \
  scripts/synty_library_transaction.py \
  scripts/test_synty_library_transaction.py \
  scripts/build_synty_pack_library.py \
  scripts/test_build_synty_pack_library.py
git commit -m "feat: publish Synty discovery libraries atomically"
```

- [ ] **Step 7: Run and verify the private Dark Fortress C build**

From the canonical Assets checkout containing the existing ignored cache, use a disposable stage:

```bash
python3 scripts/build_synty_pack_library.py stage \
  --source-root "$HOME/Downloads/synty/POLYGON_Dark_Fortress_SourceFiles_v3" \
  --pack-root library/polygon-dark-fortress/v3 \
  --config scripts/configs/synty-packs/polygon-dark-fortress.json \
  --all-groups \
  --material-policy review-all \
  --stage-root /tmp/dark-fortress-c-library

python3 scripts/build_synty_pack_library.py validate \
  --source-root "$HOME/Downloads/synty/POLYGON_Dark_Fortress_SourceFiles_v3" \
  --pack-root library/polygon-dark-fortress/v3 \
  --config scripts/configs/synty-packs/polygon-dark-fortress.json \
  --all-groups \
  --stage-root /tmp/dark-fortress-c-library
```

Inspect the material summary and representative trusted/review sheets. Then apply and check:

```bash
python3 scripts/build_synty_pack_library.py apply \
  --source-root "$HOME/Downloads/synty/POLYGON_Dark_Fortress_SourceFiles_v3" \
  --pack-root library/polygon-dark-fortress/v3 \
  --config scripts/configs/synty-packs/polygon-dark-fortress.json \
  --all-groups \
  --stage-root /tmp/dark-fortress-c-library

python3 scripts/build_synty_pack_library.py check \
  --source-root "$HOME/Downloads/synty/POLYGON_Dark_Fortress_SourceFiles_v3" \
  --pack-root library/polygon-dark-fortress/v3 \
  --config scripts/configs/synty-packs/polygon-dark-fortress.json \
  --all-groups
```

Expected: exactly 825 source identities are accounted for across trusted, material-review, unsupported, and FX review statuses; no unresolved source appears in a trusted manifest; every trusted manifest records the C atlas hash.

- [ ] **Step 8: Commit tracked reports and sheets**

Review `git status` and ensure ignored GLBs/views are absent. Commit only config, reports, portable manifests, and trusted/review sheets:

```bash
git add scripts/configs/synty-packs/polygon-dark-fortress.json \
  library/polygon-dark-fortress/v3/material-preflight.json \
  library/polygon-dark-fortress/v3/material-preflight.md \
  library/polygon-dark-fortress/v3/sheet-manifest-*.json \
  library/polygon-dark-fortress/v3/sheets
git commit -m "asset: rebuild Dark Fortress discovery under palette C"
```

---

### Task 4: Define world-asset review identity and bulk preparation

**Repository:** `rpg-game-assets`

**Files:**
- Create: `scripts/configs/world-asset-review/polygon-dark-fortress.json`
- Create: `scripts/world_asset_review.py`
- Create: `scripts/test_world_asset_review.py`
- Create: `scripts/prepare_asset_review.py`
- Create: `scripts/test_prepare_asset_review.py`

**Interfaces:**
- Consumes: repeated source globs, trusted/review manifests, and a strict review config resolved by source pack slug.
- Produces: strict candidate catalog schema v1, `derive_ref(source_path, category, config)`, `select_review_candidates()`, and ignored `public/models/synty/asset-review/` content.

- [ ] **Step 1: Write failing ref-derivation tests**

Assert:

```python
self.assertEqual(
    "dnd5e:props:dark-fortress:brazier_01",
    derive_visual_ref("SM_Prop_Brazier_01.fbx", "props", config),
)
self.assertEqual(
    "dnd5e:items:dark-fortress:rope_01",
    derive_visual_ref("SM_Prop_Rope_01.fbx", "items", config),
)
self.assertEqual(
    "dnd5e:weapons:dark-fortress:sword_02",
    derive_visual_ref("SM_Wep_Sword_02.fbx", "weapons", config),
)
self.assertEqual(
    "dnd5e:env:dark-fortress:tower_03",
    derive_visual_ref("SM_Bld_Tower_03.fbx", "env", config),
)
```

Assert rejection of unknown category, wrong prefix, empty suffix, unsafe characters, unknown pack key, and duplicate derived refs. Assert an exact source-keyed `refSuffixOverrides` row resolves a real collision.

- [ ] **Step 2: Write failing bulk-selection and preparation tests**

Create trusted and review manifest fixtures. Assert repeated `--source-match` patterns are normalized, deduplicated, source-path sorted, and reject traversal, backslashes, absolute patterns, and empty result.

Assert:

- actual cached hash must match manifest;
- eligible candidates require three finite positive GLB dimensions; Ready-ineligible material/FX review candidates may preserve finite non-negative dimensions when at least one axis is positive;
- a trusted candidate within the provider texture budget has `readyEligible: true`;
- a material/FX candidate has `readyEligible: false` plus reasons;
- a material-trusted candidate whose planned post-ceiling images exceed 4.5 MiB remains `reviewStatus: trusted` but has `readyEligible: false` plus the exact provider-budget reason;
- destination must be ignored and contain no tracked file;
- destination model is not a hard link to the source (`st_ino` differs on the same device);
- reflink failure falls back to verified `shutil.copy2`;
- insufficient disk writes neither model nor catalog;
- re-running identical input is idempotent; and
- catalog replacement is atomic.

- [ ] **Step 3: Run tests and verify failure**

Run:

```bash
python3 scripts/test_world_asset_review.py
python3 scripts/test_prepare_asset_review.py
```

Expected: FAIL because both modules are missing.

- [ ] **Step 4: Implement category and source-prefix configuration**

Add this separate reviewed config at `scripts/configs/world-asset-review/polygon-dark-fortress.json`; do not add review-only keys to the conversion pack config because its complete hash binds generated GLB manifests:

```json
{
  "schemaVersion": 1,
  "packSlug": "polygon-dark-fortress",
  "referencePack": "dark-fortress",
  "reviewCategories": {
    "SM_Prop_": "props",
    "SM_Wep_": "weapons",
    "SM_Chr_Attach_": "items",
    "SM_Env_": "env",
    "SM_Bld_": "env",
    "SM_Veh_": "env",
    "FX_": "env"
  },
  "refSuffixOverrides": {}
}
```

Require exact config keys, schema version 1, and `packSlug` equality with the selected pack root. Resolve this file automatically from the pack slug so the preparation CLI needs no additional human argument. Sort prefixes longest-first so `SM_Chr_Attach_` cannot be consumed by a shorter future prefix. Lowercase the suffix and preserve underscores. Store the configured `referencePack`, a humanized display suggestion, measured GLB dimensions, and browsing-family guess separately from the ref.

- [ ] **Step 5: Implement strict catalog preparation**

Use this candidate shape:

```json
{
  "schemaVersion": 1,
  "candidates": [
    {
      "source": {
        "packSlug": "polygon-dark-fortress",
        "packVersion": "v3",
        "sourcePath": "SourceFiles/DarkFortress/FBX/SM_Prop_Brazier_01.fbx",
        "glbSha256": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      },
      "url": "/models/synty/asset-review/aaaaaaaaaaaa-SM_Prop_Brazier_01.glb",
      "sourceFamily": "props",
      "suggestedCategory": "props",
      "suggestedDisplayName": "Brazier 01",
      "browsingFamily": "brazier",
      "referencePack": "dark-fortress",
      "refSuffix": "brazier_01",
      "dimensionsMeters": [1.0, 2.0, 1.0],
      "readyEligible": true,
      "reviewStatus": "trusted",
      "reasons": []
    }
  ]
}
```

Use `cp --reflink=always --` when available; on failure, remove any partial destination and use `shutil.copy2`. Hash the destination afterward. Reject equal source/destination inode identity. Measure dimensions from GLB POSITION accessor bounds using the existing pure-Python convention. Require all axes finite and non-negative with at least one positive axis for Ready-ineligible review rows; eligible rows require all three axes positive. Never clamp or fabricate a zero axis. For material-trusted rows, call the shared pure `planned_runtime_image_facts()` contract from Task 7; if its aggregate exceeds 4.5 MiB, preserve `reviewStatus: trusted` but set `readyEligible: false` and append its exact provider-budget reason. Write JSON through a temporary sibling plus `os.replace`.

- [ ] **Step 6: Add CLI and ignore boundary**

Add `prepare` and `reset` subcommands. `prepare` accepts required `--pack-root`, repeatable `--source-match`, `--web-root`, optional `--port`, and `--reset`. Print counts by trusted/material/FX status and the exact loopback URL:

```text
http://127.0.0.1:5173/?assetReview=1
```

The existing Web rule `public/models/synty/` already covers `public/models/synty/asset-review/`; preparation must prove that effective boundary with `git check-ignore` before writing and must reject any Web checkout where the destination is tracked or not ignored. This Assets task does not edit the Web repository.

- [ ] **Step 7: Run focused tests and commit**

Run:

```bash
python3 scripts/test_world_asset_review.py
python3 scripts/test_prepare_asset_review.py
python3 -m py_compile scripts/world_asset_review.py scripts/prepare_asset_review.py
git diff --check
```

Expected: PASS.

Commit:

```bash
git add scripts/configs/world-asset-review/polygon-dark-fortress.json \
  scripts/world_asset_review.py \
  scripts/test_world_asset_review.py \
  scripts/prepare_asset_review.py \
  scripts/test_prepare_asset_review.py
git commit -m "feat: prepare bulk world-asset review queues"
```

---

### Task 5: Build the pure Web review-state model

**Repository:** `rpg-dnd5e-web`

**Files:**
- Create: `src/dev/asset-review/model.ts`
- Create: `src/dev/asset-review/model.test.ts`

**Interfaces:**
- Consumes: Assets candidate catalog schema v1 and optional review-progress JSON schema v1.
- Produces: strict parsing, catalog merge, decision transitions, validation, filtering, review serialization, and Ready-only provider serialization.

- [ ] **Step 1: Write failing strict-catalog tests**

Define fixture source hash as `'a'.repeat(64)` and assert:

- exact candidate keys parse;
- unknown/missing keys fail;
- URL must match `/models/synty/asset-review/{content-addressed-name}.glb` and reject slashes or unsafe filename characters inside the final segment;
- categories are limited to Props/Items/Weapons/Environment values;
- `readyEligible` must be boolean;
- trusted candidates may be ineligible only when they carry a non-empty provider-preflight reason;
- material/FX candidates must be ineligible with a reason; and
- source hash must be lowercase SHA-256;
- eligible candidate dimensions must have three positive finite axes; and
- ineligible candidate dimensions may contain zero axes but must be finite, non-negative, and positive on at least one axis.

- [ ] **Step 2: Write failing review-transition tests**

Define:

```ts
export type ReviewDecision =
  | 'undecided'
  | 'keep'
  | 'ready'
  | 'skip'
  | 'defer';

export type WorldAssetCategory = 'props' | 'items' | 'weapons' | 'env';
export type ReviewStatus = 'trusted' | 'material-review' | 'fx-review';
export type FieldErrors = Record<string, string>;
```

Assert:

```ts
expect(markKeep(entry).decision).toBe('keep');
expect(markReady(validKeptEntry).decision).toBe('ready');
expect(() => markReady(materialBlockedEntry)).toThrow(/not eligible/i);
expect(updateCategory(readyEntry, 'items').decision).toBe('keep');
expect(updateCalibration(readyEntry, { scale: 1.2 }).decision).toBe('keep');
```

Assert Ready requires non-empty display name, derived-ref equality, finite scale in `(0, 100]`, normalized yaw `[-180, 180)`, horizontal offsets within `[-0.5, 0.5]`, vertical offset within `[-0.1, 0.1]`, positive measured bounds whose scaled runtime axes do not exceed 20 metres, and a successful-load marker.

- [ ] **Step 3: Write failing merge/filter/export tests**

Assert:

- stable identity is pack slug + version + source path + GLB hash;
- exact matches preserve draft decisions;
- changed hashes return as Undecided;
- stale imported entries are reported and not silently inserted;
- search covers source path, display name, ref, and browsing family;
- category/source/status/family filters compose;
- filtered next/previous uses deterministic source order;
- review JSON includes all candidates and decisions but no URL;
- provider JSON contains Ready only; and
- provider export refuses zero Ready entries.

- [ ] **Step 4: Run tests and verify failure**

Run:

```bash
npm test -- --run src/dev/asset-review/model.test.ts
```

Expected: FAIL because `model.ts` does not exist.

- [ ] **Step 5: Implement focused types and functions**

Export these interfaces/functions with exact names:

```ts
export interface AssetReviewSource {
  packSlug: string;
  packVersion: string;
  sourcePath: string;
  glbSha256: string;
}

export interface AssetReviewCandidate {
  source: AssetReviewSource;
  url: string;
  sourceFamily: string;
  suggestedCategory: WorldAssetCategory;
  suggestedDisplayName: string;
  browsingFamily: string;
  referencePack: string;
  refSuffix: string;
  dimensionsMeters: [number, number, number];
  readyEligible: boolean;
  reviewStatus: ReviewStatus;
  reasons: string[];
}

export interface AssetReviewCatalog {
  schemaVersion: 1;
  candidates: AssetReviewCandidate[];
}

export interface AssetReviewEntry {
  source: AssetReviewSource;
  url: string;
  sourceFamily: string;
  browsingFamily: string;
  referencePack: string;
  refSuffix: string;
  dimensionsMeters: [number, number, number];
  readyEligible: boolean;
  reviewStatus: ReviewStatus;
  reasons: string[];
  decision: ReviewDecision;
  loadedSuccessfully: boolean;
  displayName: string;
  category: WorldAssetCategory;
  ref: string;
  calibration: {
    scale: number;
    yawDegrees: number;
    fineOffsetMeters: [number, number, number];
  };
  tags: string[];
  supportsDecoration: boolean;
  notes: string;
  deferReason: string;
}

export interface AssetReviewBatch {
  schemaVersion: 1;
  batchId: string;
  entries: AssetReviewEntry[];
}

export interface MergeResult {
  batch: AssetReviewBatch;
  staleSourceKeys: string[];
}

export interface ProviderFieldPatch {
  displayName?: string;
  category?: WorldAssetCategory;
  calibration?: Partial<AssetReviewEntry['calibration']>;
  tags?: string[];
  supportsDecoration?: boolean;
  notes?: string;
}

export interface ReviewFilter {
  search: string;
  category?: WorldAssetCategory;
  sourceFamily?: string;
  browsingFamily?: string;
  decision?: ReviewDecision | 'needs-details';
  reviewStatus?: ReviewStatus;
}

export function parseAssetReviewCatalog(value: unknown): AssetReviewCatalog;
export function mergeCatalogWithReview(catalog: AssetReviewCatalog, review?: AssetReviewBatch): MergeResult;
export function visualRef(entry: Pick<AssetReviewEntry, 'category' | 'referencePack' | 'refSuffix'>): string;
export function validateReady(entry: AssetReviewEntry): FieldErrors;
export function transitionDecision(entry: AssetReviewEntry, decision: ReviewDecision): AssetReviewEntry;
export function updateProviderFields(entry: AssetReviewEntry, patch: ProviderFieldPatch): AssetReviewEntry;
export function filterReviewEntries(entries: AssetReviewEntry[], filter: ReviewFilter): AssetReviewEntry[];
export function serializeReviewProgress(batch: AssetReviewBatch): string;
export function serializeReadyProviderBatch(batch: AssetReviewBatch): string;
```

Provider-field updates demote Ready before returning. `transitionDecision(entry, 'ready')` calls `validateReady(entry)` and throws on any error.

- [ ] **Step 6: Run focused tests and commit**

Run:

```bash
npm test -- --run src/dev/asset-review/model.test.ts
npm run typecheck
npx prettier --check src/dev/asset-review/model.ts src/dev/asset-review/model.test.ts
git diff --check
```

Expected: PASS.

Commit:

```bash
git add src/dev/asset-review/model.ts src/dev/asset-review/model.test.ts
git commit -m "feat: model world-asset review decisions"
```

---

### Task 6: Build the loopback Asset Review Lab and bounded loader lifecycle

**Repository:** `rpg-dnd5e-web`

**Files:**
- Create: `src/dev/asset-review/route.ts`
- Create: `src/dev/asset-review/route.test.ts`
- Create: `src/dev/asset-review/disposeObjectResources.ts`
- Create: `src/dev/asset-review/disposeObjectResources.test.ts`
- Create: `src/dev/asset-review/AssetReviewScene.tsx`
- Create: `src/dev/asset-review/AssetReviewLab.tsx`
- Create: `src/dev/asset-review/AssetReviewLab.css`
- Create: `src/dev/asset-review/AssetReviewLab.test.tsx`
- Modify: `src/App.tsx`
- Modify: `src/App.test.tsx`

**Interfaces:**
- Consumes: `/models/synty/asset-review/catalog.json` and Task 5 model functions.
- Produces: `?assetReview=1` local route, visual review controls, autosaved review state, and downloadable review/provider JSON.

- [ ] **Step 1: Write the failing route matrix**

Assert:

```ts
it.each([
  ['development', '127.0.0.1', '?assetReview=1', true],
  ['development', 'localhost', '?assetReview=1', true],
  ['development', '::1', '?assetReview=1', true],
  ['development', '[::1]', '?assetReview=1', true],
  ['production', '127.0.0.1', '?assetReview=1', false],
  ['development', 'review.example.test', '?assetReview=1', false],
  ['development', '127.0.0.1', '?assetReview=0', false],
])('%s %s %s', (mode, hostname, search, expected) => {
  expect(isAssetReviewRoute(mode, hostname, search)).toBe(expected);
});
```

Also retain existing `?propCalibration=1` route behavior unchanged.

- [ ] **Step 2: Write failing disposal tests**

Construct a Three.js hierarchy sharing one geometry, material, and texture across two meshes. Assert `disposeObjectResources(root)` disposes each unique owned resource exactly once. Assert it does not dispose caller-supplied shared floor/fighter resources because they are outside the candidate root.

- [ ] **Step 3: Write failing Lab interaction tests**

Mock `AssetReviewScene` and catalog fetch. Assert:

- deterministic first candidate and progress counts;
- drawer search for `brazier` limits the list and next/previous operates inside it;
- category radio updates the read-only ref;
- K marks Keep;
- S and D advance;
- shortcuts do nothing while focus is in input, textarea, select, or contenteditable;
- Keep can remain incomplete;
- Keep/Needs Details finds incomplete keepers;
- Mark Ready is disabled until fields validate and the scene reports load success;
- material/FX candidates never enable Ready;
- editing a Ready field demotes it to Keep;
- local storage receives review progress;
- malformed import preserves current state;
- Review export contains all decisions; and
- Provider export contains Ready entries only.

- [ ] **Step 4: Run tests and verify failure**

Run:

```bash
npm test -- --run \
  src/dev/asset-review/route.test.ts \
  src/dev/asset-review/disposeObjectResources.test.ts \
  src/dev/asset-review/AssetReviewLab.test.tsx \
  src/App.test.tsx
```

Expected: FAIL because the Asset Review Lab modules do not exist.

- [ ] **Step 5: Implement the route and lazy App boundary**

Export:

```ts
const LOOPBACK_HOSTS = new Set(['127.0.0.1', 'localhost', '::1', '[::1]']);

export function isAssetReviewRoute(
  mode: string,
  hostname: string,
  search: string
): boolean {
  return (
    mode === 'development' &&
    LOOPBACK_HOSTS.has(hostname) &&
    new URLSearchParams(search).get('assetReview') === '1'
  );
}
```

Use a development-conditional lazy import in `App.tsx`. Do not add Home, Concepts, or product navigation.

- [ ] **Step 6: Implement one-model-at-a-time loading**

Use `GLTFLoader` directly inside an effect rather than `useGLTF`'s global cache. On URL change:

1. set the candidate state to loading;
2. load one GLB;
3. attach its scene only if the effect remains current;
4. report success/failure to the Lab;
5. dispose and detach the prior candidate root during cleanup; and
6. ignore stale load callbacks after cleanup.

Reuse the delivered floor, fighter, camera, bounds, raw overlay, and calibration conventions without changing `src/dev/prop-calibration/` behavior.

- [ ] **Step 7: Implement drawer and metadata workflow**

Provide All/Undecided/Keep/Needs Details/Ready/Skip/Defer tabs, category/family/source/status filters, text search, direct row selection, and filtered previous/next. Show exact source path, hash, material status, blocking reasons, derived ref, calibration, tags, an explicit Supports decorations checkbox, notes, and defer reason.

Ready requires a deliberate button click. Display names/category/calibration/tags/supportsDecoration/notes update one candidate without advancing. `supportsDecoration` defaults false and is never inferred from a source name. Skip/Defer advance; Keep does not force completion.

Store review JSON under:

```ts
export const ASSET_REVIEW_STORAGE_KEY = 'rpg.asset-review.batch.v1';
```

- [ ] **Step 8: Run focused and app gates**

Run:

```bash
npm test -- --run \
  src/dev/asset-review/model.test.ts \
  src/dev/asset-review/route.test.ts \
  src/dev/asset-review/disposeObjectResources.test.ts \
  src/dev/asset-review/AssetReviewLab.test.tsx \
  src/App.test.tsx
npm run typecheck
npm run lint
npm run format:check
npm run build
git diff --check
```

Expected: PASS.

- [ ] **Step 9: Commit**

```bash
git add src/App.tsx src/App.test.tsx src/dev/asset-review
git commit -m "feat: review world assets in bulk"
```

---

### Task 7: Define and normalize generic world-asset provider recipes

**Repository:** `rpg-game-assets`

**Files:**
- Extend: `scripts/world_asset_review.py`
- Extend: `scripts/test_world_asset_review.py`
- Create: `scripts/normalize_world_asset_for_runtime.py`
- Create: `scripts/test_normalize_world_asset_for_runtime.py`
- Modify: `scripts/normalize_prop_for_runtime.py`
- Modify: `scripts/test_normalize_prop_for_runtime.py`

**Interfaces:**
- Consumes: Ready-only schema-v1 JSON and trusted discovery GLBs.
- Produces: `load_world_asset_batch()`, `validate_world_asset_batch()`, `runtime_path_for_world_asset()`, and Blender-normalized one- or multi-material static GLBs.

- [ ] **Step 1: Write failing provider-recipe tests**

Use a valid row with hash `'a' * 64`. Assert:

```python
self.assertEqual(
    PurePosixPath("harness/models/synty/world-assets/props/dark-fortress/brazier_01.glb"),
    runtime_path_for_world_asset("dnd5e:props:dark-fortress:brazier_01"),
)
```

Assert strict rejection of unknown keys, category outside the four values, ref/category disagreement, pack key other than configured `dark-fortress`, suffix disagreement with source/config override, unsafe/traversing source, malformed hash, duplicate ref, duplicate output path, non-finite calibration, scale outside `(0, 100]`, yaw outside `[-180, 180)`, horizontal offsets outside `[-0.5, 0.5]`, and vertical offset outside `[-0.1, 0.1]`.

- [ ] **Step 2: Write failing multi-material normalization tests**

Construct two synthetic GLBs:

1. one mesh, one material, one embedded 2048×2048 atlas;
2. one mesh with two primitives/materials, embedded main atlas, auxiliary color texture, and auxiliary normal texture.

Run Blender normalization with nontrivial scale/yaw/offset. Assert:

- source mesh/material/image counts are preserved;
- primitive material indices remain preserved;
- each embedded image remains embedded;
- texture/color versus normal usage remains unchanged;
- images over the configured per-image limit are reduced without upscaling smaller images;
- decoded texture limits are enforced and reported by image;
- output nodes are identity transforms;
- bounds are finite, centered, and grounded at the requested offset; and
- skin, animation, camera, light, external URI, morph, and unsupported object inputs fail.

- [ ] **Step 3: Run tests and verify failure**

Run:

```bash
python3 scripts/test_world_asset_review.py
blender --background --factory-startup --python scripts/test_normalize_world_asset_for_runtime.py
```

Expected: FAIL because generic provider parsing and world-asset normalization do not exist.

- [ ] **Step 4: Generalize static texture inspection without weakening prop checks**

Extract shared GLB parsing, finite geometry, identity transforms, bounds, and embedded-image inspection from `normalize_prop_for_runtime.py` into functions usable by both entry points. Keep the existing prop wrapper's exact one-image requirement and 4.5 MiB decoded cap unchanged.

Extract `planned_runtime_image_facts(path: Path) -> tuple[dict[str, int | float | str], ...]` as a Blender-free contract that applies the same per-image 1024×1024 downscale calculation and 4.5 MiB aggregate check used by normalization. The world-asset wrapper consumes that exact plan, reports each image's dimensions/decoded bytes, applies the ceiling without upscaling, and preserves the existing 20-metre maximum runtime axis. It never drops an auxiliary image or rewires a material to the main atlas. Review preparation consumes the same pure plan so Ready eligibility cannot disagree with provider normalization.

- [ ] **Step 5: Implement strict recipe types and path derivation**

Define immutable `WorldAssetSource`, `WorldAssetCalibration`, `WorldAssetPromotionEntry`, and `WorldAssetBatch` dataclasses, including an explicit boolean `supports_decoration`. `validate_world_asset_batch(batch)` validates portable structure, refs, transforms, and uniqueness without source I/O. Provider source resolution loads the reviewed pack config and separately recomputes the expected source suffix/category/ref before Blender runs. Derive runtime paths only from validated ref segments.

- [ ] **Step 6: Run focused regression gates and commit**

Run:

```bash
python3 scripts/test_world_asset_review.py
python3 scripts/test_normalize_prop_for_runtime.py
blender --background --factory-startup --python scripts/test_normalize_world_asset_for_runtime.py
python3 -m py_compile \
  scripts/world_asset_review.py \
  scripts/normalize_prop_for_runtime.py \
  scripts/normalize_world_asset_for_runtime.py
git diff --check
```

Expected: PASS, including every existing floor-prop normalizer test.

Commit:

```bash
git add scripts/world_asset_review.py \
  scripts/test_world_asset_review.py \
  scripts/normalize_prop_for_runtime.py \
  scripts/test_normalize_prop_for_runtime.py \
  scripts/normalize_world_asset_for_runtime.py \
  scripts/test_normalize_world_asset_for_runtime.py
git commit -m "feat: validate and normalize world-asset recipes"
```

---

### Task 8: Publish generic world assets atomically

**Repository:** `rpg-game-assets`

**Files:**
- Create: `scripts/build_world_asset_catalog.py`
- Create: `scripts/test_build_world_asset_catalog.py`
- Create: `scripts/promote_world_assets.py`
- Create: `scripts/test_promote_world_assets.py`
- Modify: `scripts/build_mesh_stats.py`
- Modify: `scripts/build_synty_complete_inventory.py` only if its existing recursive inventory omits the new root

**Interfaces:**
- Consumes: validated WorldAssetBatch and canonical trusted discovery manifests.
- Produces: staged/promoted GLBs, tracked recipe, `harness/catalogs/synty-world-assets.json`, mesh stats, complete inventory, and deterministic receipt.

- [ ] **Step 1: Write failing catalog tests**

Build a temporary promoted root and assert canonical catalog entries include exactly:

```json
{
  "ref": "dnd5e:props:dark-fortress:brazier_01",
  "displayName": "Brazier 01",
  "category": "props",
  "file": "world-assets/props/dark-fortress/brazier_01.glb",
  "glbSha256": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
  "sizeBytes": 1234,
  "boundsMeters": [1.0, 2.0, 1.0],
  "tags": ["dark-fortress"],
  "supportsDecoration": false
}
```

The test fixture may use a deterministic synthetic GLB/hash pair rather than the literal example hash/size above. Assert deterministic ref order, duplicate/path/hash rejection, strict public-safe field allowlist, and absence of source paths, machine paths, review decisions, defer reasons, and gameplay behavior.

- [ ] **Step 2: Write failing provider transaction tests**

Inject a normalizer and temporary roots. Assert:

- `stage` mutates no canonical path;
- source resolution requires trusted manifest status and exact GLB hash;
- material-review and FX rows fail;
- every Ready entry produces one runtime GLB;
- stage regenerates catalog, mesh stats, inventory, tracked recipe, and receipt;
- `validate` compares exact staged bytes;
- `apply` restores all prior targets on injected failure;
- a ref owned by another recipe fails;
- changing an existing ref requires editing its owning recipe; and
- `check` detects GLB, recipe, catalog, receipt, stats, or inventory drift.

- [ ] **Step 3: Run tests and verify failure**

Run:

```bash
python3 scripts/test_build_world_asset_catalog.py
python3 scripts/test_promote_world_assets.py
```

Expected: FAIL because provider modules do not exist.

- [ ] **Step 4: Implement deterministic catalog generation**

Implement this exact catalog algorithm:

Import `sha256_file` from `index_synty_archives`, import `load_world_asset_batch`, `runtime_path_for_world_asset`, and `validate_world_asset_batch` from `world_asset_review`, and define `_require` plus `world_asset_bounds_meters` in this focused catalog module. `world_asset_bounds_meters` reads all POSITION accessor minima/maxima, computes positive axis lengths, and multiplies each by the shared runtime scale `0.75`. Then implement:

```python
def build_world_asset_catalog(
    provider_root: Path,
    recipe_root: Path,
) -> dict[str, object]:
    rows: list[dict[str, object]] = []
    recipes: list[dict[str, str]] = []
    refs: set[str] = set()
    files: set[str] = set()
    for recipe_path in sorted(Path(recipe_root).glob("*.json")):
        if recipe_path.name == "template.json":
            continue
        batch = load_world_asset_batch(recipe_path)
        validate_world_asset_batch(batch)
        recipes.append({"batchId": batch.batch_id, "sha256": sha256_file(recipe_path)})
        for entry in batch.entries:
            runtime_path = runtime_path_for_world_asset(entry.ref)
            runtime_file = runtime_path.relative_to("harness/models/synty").as_posix()
            _require(entry.ref not in refs, f"duplicate world-asset ref: {entry.ref}")
            _require(runtime_file not in files, f"duplicate world-asset file: {runtime_file}")
            output = Path(provider_root) / runtime_path
            _require(output.is_file() and not output.is_symlink(), f"missing world asset: {entry.ref}")
            rows.append({
                "ref": entry.ref,
                "displayName": entry.display_name,
                "category": entry.category,
                "file": runtime_file,
                "glbSha256": sha256_file(output),
                "sizeBytes": output.stat().st_size,
                "boundsMeters": list(world_asset_bounds_meters(output)),
                "tags": list(entry.tags),
                "supportsDecoration": entry.supports_decoration,
            })
            refs.add(entry.ref)
            files.add(runtime_file)
    return {
        "schemaVersion": 1,
        "generatedBy": "build_world_asset_catalog@1.0.0",
        "recipes": sorted(recipes, key=lambda row: row["batchId"]),
        "assets": sorted(rows, key=lambda row: str(row["ref"])),
    }


def catalog_bytes(document: Mapping[str, object]) -> bytes:
    return (json.dumps(document, indent=2, sort_keys=True, ensure_ascii=True) + "\n").encode("utf-8")
```

The implementation also validates each promoted file's GLB structure through `world_asset_bounds_meters`. Public-safe recipe hashes bind the generated catalog to its tracked human selections; the provider receipt binds full source-to-output provenance.

- [ ] **Step 5: Implement stage/validate/apply/check**

Follow `promote_props.py` and `static_release_transaction.py`, but govern only these targets:

```text
harness/models/synty/world-assets/                 # recursively governed category/pack/suffix GLBs
scripts/configs/world-asset-promotion/              # all non-template tracked batch recipes
harness/catalogs/synty-world-assets.json
harness/models/synty/mesh-stats.json
harness/catalogs/synty-complete-inventory.json
evidence/world-asset-promotions/                    # one receipt directory per tracked batch ID
```

The receipt binds batch/recipe hash, source/output hashes, bounds, per-image texture facts, catalog hash, inventory hash, tool versions, and exact targets without timestamps or absolute paths.

- [ ] **Step 6: Run provider gates and commit**

Run:

```bash
python3 scripts/test_build_world_asset_catalog.py
python3 scripts/test_promote_world_assets.py
python3 scripts/test_build_mesh_stats.py
python3 scripts/build_synty_complete_inventory.py --check
python3 -m unittest discover -s scripts -p 'test_*.py'
git diff --check
```

Expected: PASS.

Commit:

```bash
git add scripts/build_world_asset_catalog.py \
  scripts/test_build_world_asset_catalog.py \
  scripts/promote_world_assets.py \
  scripts/test_promote_world_assets.py \
  scripts/build_mesh_stats.py \
  scripts/build_synty_complete_inventory.py
git commit -m "feat: publish selected world assets atomically"
```

---

### Task 9: Generate and consume the world-building visual catalog

**Repository:** `rpg-dnd5e-web`

**Prerequisite:** web#935's world-building concept is merged into `origin/dev`, and the private provider branch from Task 8 has a clean committed test batch for local pre-merge visual verification.

**Files:**
- Create: `scripts/generate-world-asset-catalog.mjs`
- Create: `scripts/generateWorldAssetCatalog.test.ts`
- Modify: `scripts/sync-game-assets.sh`
- Modify: `package.json`
- Generate: `src/generated/worldAssetCatalog.ts`
- Create: `src/components/hex-grid/WorldAssetModel.tsx`
- Create: `src/components/hex-grid/WorldAssetModel.test.tsx`
- Modify: `src/concepts/world-building/catalog.ts`
- Modify: `src/concepts/world-building/serialization.ts`
- Modify: `src/concepts/world-building/serialization.test.ts`
- Modify: `src/concepts/world-building/WorldBuildingViewport.tsx`
- Modify: `src/concepts/world-building/WorldBuildingViewport.test.tsx`
- Modify: `src/concepts/world-building/WorldBuildingConcept.tsx`
- Modify: `src/concepts/world-building/WorldBuildingConcept.test.tsx`

**Interfaces:**
- Consumes: synced ignored GLBs and `harness/catalogs/synty-world-assets.json` from a clean exact Assets revision.
- Produces: committed generated TypeScript, exact world-asset resolution, and World Builder palette entries.

- [ ] **Step 1: Write failing generator tests**

Create temporary provider and Web trees. Assert generation:

- rejects dirty/uncommitted provider input using the existing sync cleanliness convention;
- parses only schema-v1 provider catalog fields;
- verifies each synchronized GLB size and hash;
- rejects traversal, symlink, duplicate ref, duplicate runtime URL, malformed bounds, and unknown category;
- sorts exact refs deterministically;
- writes no absolute/provider path or source identity; and
- `--check` detects stale generated bytes without rewriting.

- [ ] **Step 2: Write failing resolver and world-building tests**

Use one generated fixture entry and assert:

```ts
expect(resolveWorldAsset('dnd5e:props:dark-fortress:brazier_01')).toEqual({
  ref: 'dnd5e:props:dark-fortress:brazier_01',
  displayName: 'Brazier 01',
  category: 'props',
  url: '/models/synty/world-assets/props/dark-fortress/brazier_01.glb',
  boundsMeters: [1, 2, 1],
  tags: ['dark-fortress'],
  supportsDecoration: false,
});
expect(resolveWorldAsset('dnd5e:props:dark-fortress:missing')).toBeUndefined();
```

Assert the combined world-building catalog includes the generated entry once, search finds it, drag/drop accepts it, scene JSON round-trips the exact ref, and an unknown exact ref is rejected without substitution.

- [ ] **Step 3: Run focused tests and verify failure**

Run:

```bash
npm test -- --run \
  scripts/generateWorldAssetCatalog.test.ts \
  src/components/hex-grid/WorldAssetModel.test.tsx \
  src/concepts/world-building/serialization.test.ts \
  src/concepts/world-building/WorldBuildingViewport.test.tsx \
  src/concepts/world-building/WorldBuildingConcept.test.tsx
```

Expected: FAIL because generated world-asset catalog support does not exist.

- [ ] **Step 4: Implement generator and scripts**

Generate provider binding plus assets:

```ts
export const GENERATED_WORLD_ASSET_PROVIDER = Object.freeze({
  commit: '0000000000000000000000000000000000000000',
  catalogSha256: '0000000000000000000000000000000000000000000000000000000000000000',
  generatedBy: 'build_world_asset_catalog@1.0.0',
  recipes: [] as ReadonlyArray<Readonly<{ batchId: string; sha256: string }>>,
});

export interface GeneratedWorldAsset {
  ref: string;
  displayName: string;
  category: 'props' | 'items' | 'weapons' | 'env';
  url: string;
  glbSha256: string;
  sizeBytes: number;
  boundsMeters: [number, number, number];
  tags: string[];
  supportsDecoration: boolean;
}

export const GENERATED_WORLD_ASSETS: Readonly<Record<string, GeneratedWorldAsset>> = Object.freeze({});
```

The all-zero commit/hash values above are test-fixture examples only. The generator always emits the exact clean provider Git commit and actual catalog SHA-256; it refuses a dirty provider. The empty asset object is deterministic when the provider catalog contains zero entries; after the first promotion the generator emits concrete ref-keyed entries. Add:

```json
{
  "world-assets:sync": "bash scripts/sync-game-assets.sh --world-assets",
  "world-assets:check": "bash scripts/sync-game-assets.sh --world-assets --check"
}
```

Integrate with existing argument parsing rather than duplicating the complete asset mirror.

- [ ] **Step 5: Implement generic exact-ref rendering**

`WorldAssetModel` resolves only generated exact refs. It uses the same GLB loading, error reporting, shared Synty scale convention, and provider-baked transform assumptions as `PropModel`, without copying gameplay role/default-family behavior into the generic resolver.

Update the world-building catalog to merge legacy entries and generated entries. Copy each generated entry's explicit `supportsDecoration` value into the authoring catalog; never infer it from labels or dimensions. Validate scene refs against the combined map. Route generated entries to `WorldAssetModel` and legacy refs to existing `PropModel`.

- [ ] **Step 6: Test the unmerged provider branch in the real concept**

From a clean Web issue worktree, set `ASSETS_PROVIDER_WORKTREE` to the clean Assets implementation worktree created during repository preparation and prove it is clean:

```bash
test -d "$ASSETS_PROVIDER_WORKTREE/.git" || test -f "$ASSETS_PROVIDER_WORKTREE/.git"
test -z "$(git -C "$ASSETS_PROVIDER_WORKTREE" status --porcelain)"
RPG_GAME_ASSETS_PATH="$ASSETS_PROVIDER_WORKTREE" npm run world-assets:sync
npm run dev -- --host 127.0.0.1 --port 5173
```

Open the world-building concept. Verify a Ready sample entry:

- appears once with its exact label/ref;
- can be searched and dragged;
- renders at reviewed scale/yaw/grounding;
- can be grouped with existing table/candle/book assets;
- survives scene export/import and browser reload; and
- reports no browser, WebGL, or asset-load error.

If presentation is wrong, return to Asset Review Lab, demote/edit/reapprove, rebuild the provider stage, and repeat sync. Do not add a Web-side repair transform.

- [ ] **Step 7: Run complete Web gates and commit**

Run:

```bash
RPG_GAME_ASSETS_PATH="$ASSETS_PROVIDER_WORKTREE" npm run world-assets:check
npm test -- --run \
  scripts/generateWorldAssetCatalog.test.ts \
  src/components/hex-grid/WorldAssetModel.test.tsx \
  src/concepts/world-building/serialization.test.ts \
  src/concepts/world-building/WorldBuildingViewport.test.tsx \
  src/concepts/world-building/WorldBuildingConcept.test.tsx
npm run ci-check
git diff --check
```

Expected: PASS.

Commit:

```bash
git add package.json \
  scripts/sync-game-assets.sh \
  scripts/generate-world-asset-catalog.mjs \
  scripts/generateWorldAssetCatalog.test.ts \
  src/generated/worldAssetCatalog.ts \
  src/components/hex-grid/WorldAssetModel.tsx \
  src/components/hex-grid/WorldAssetModel.test.tsx \
  src/concepts/world-building
git commit -m "feat: add selected world assets to the builder"
```

---

### Task 10: Complete the learning guides and promote Kirk's first Ready batch

**Repository:** `rpg-game-assets`

**Files:**
- Create: `docs/human/asset-ingestion/material-preflight.md`
- Create: `docs/human/asset-ingestion/asset-review-lab.md`
- Create: `docs/human/asset-ingestion/world-asset-promotion.md`
- Modify: `docs/human/asset-ingestion/README.md`
- Modify: `docs/human/asset-ingestion/pack-library.md`
- Modify: `docs/human/asset-ingestion/troubleshooting.md`
- Produce: `scripts/configs/world-asset-promotion/dark-fortress-world-assets-v1.json`
- Produce: `evidence/world-asset-promotions/dark-fortress-world-assets-v1/receipt.json`
- Produce: selected GLBs and regenerated provider metadata from Task 8

**Interfaces:**
- Consumes: delivered Assets commands plus Kirk's Ready-only JSON export.
- Produces: repeatable human lessons and one clean committed provider batch suitable for pre-merge Web verification.

- [ ] **Step 1: Write the material-preflight lesson**

Teach material slots and face assignments; UVs and plausible/rainbow wrong-atlas failures; default versus auxiliary color/normal textures; reading both preflight reports; inspecting one blocked source in Blender; identifying color, normal, repeat/clamp, transparency, and unsupported shader behavior; adding one explicit mapping; and rebuilding the affected group safely.

Include exact Dark Fortress commands, failure tables, and Make it work / Explain it checks.

- [ ] **Step 2: Write the Asset Review Lab lesson**

Document one broad source glob and one narrow family glob, what `.fbx` identity selects versus which cached `.glb` loads, all five review states, category/ref derivation, Supports decorations, search/filter/keyboard navigation, autosave, review-progress export, stale hash behavior, and Ready-only export.

Include the exact route:

```text
http://127.0.0.1:5173/?assetReview=1
```

- [ ] **Step 3: Write the provider-promotion lesson**

Teach recipe validation, stage, exact-stage validation, apply, check, provider PR, pre-merge Web sync, merged-provider rebinding, and World Builder verification. Explicitly distinguish visual category and Supports decorations from Toolkit/gameplay mapping.

- [ ] **Step 4: Run documentation and command-surface checks**

Run:

```bash
python3 -m py_compile \
  scripts/synty_material_preflight.py \
  scripts/prepare_asset_review.py \
  scripts/promote_world_assets.py
python3 scripts/prepare_asset_review.py --help
python3 scripts/promote_world_assets.py --help
git diff --check
```

Expected: PASS; every documented top-level command and option exists.

- [ ] **Step 5: Commit the learning guides**

```bash
git add docs/human/asset-ingestion/material-preflight.md \
  docs/human/asset-ingestion/asset-review-lab.md \
  docs/human/asset-ingestion/world-asset-promotion.md \
  docs/human/asset-ingestion/README.md \
  docs/human/asset-ingestion/pack-library.md \
  docs/human/asset-ingestion/troubleshooting.md
git commit -m "docs: teach bulk world-asset review and promotion"
```

- [ ] **Step 6: Run Kirk's first real selection batch**

Prepare a manageable material-ready family batch rather than all 825 at once. Kirk reviews candidates, marks some Keep, reloads, filters Keep/Needs Details, finishes at least one candidate, sets Supports decorations where appropriate, marks it Ready, and exports both review-progress and provider JSON.

Record the source glob, candidate count, state counts, exact Ready refs, source hashes, calibration values, material status, and Kirk's visual verdict.

- [ ] **Step 7: Stage, inspect, and apply the real provider batch**

Use:

```bash
python3 scripts/promote_world_assets.py stage \
  --repo-root "$PWD" \
  --cache-root "$HOME/game-dev/rpg-game-assets" \
  --config "$HOME/Downloads/dark-fortress-world-assets-v1.json" \
  --stage-root /tmp/dark-fortress-world-assets-v1

python3 scripts/promote_world_assets.py validate \
  --repo-root "$PWD" \
  --cache-root "$HOME/game-dev/rpg-game-assets" \
  --config "$HOME/Downloads/dark-fortress-world-assets-v1.json" \
  --stage-root /tmp/dark-fortress-world-assets-v1

python3 scripts/promote_world_assets.py apply \
  --repo-root "$PWD" \
  --cache-root "$HOME/game-dev/rpg-game-assets" \
  --config "$HOME/Downloads/dark-fortress-world-assets-v1.json" \
  --stage-root /tmp/dark-fortress-world-assets-v1

python3 scripts/promote_world_assets.py check \
  --repo-root "$PWD" \
  --cache-root "$HOME/game-dev/rpg-game-assets" \
  --config scripts/configs/world-asset-promotion/dark-fortress-world-assets-v1.json
```

Expected: PASS; every Ready row has one normalized provider GLB and receipt entry.

- [ ] **Step 8: Commit the complete provider unit**

```bash
git add scripts/configs/world-asset-promotion/dark-fortress-world-assets-v1.json \
  evidence/world-asset-promotions/dark-fortress-world-assets-v1 \
  harness/models/synty/world-assets \
  harness/catalogs/synty-world-assets.json \
  harness/models/synty/mesh-stats.json \
  harness/catalogs/synty-complete-inventory.json
git commit -m "asset: promote selected Dark Fortress world assets"
git status --short
```

Expected: the Assets issue branch is clean.

- [ ] **Step 9: Run Assets final gates and open the implementation PR**

```bash
python3 -m unittest discover -s scripts -p 'test_*.py'
python3 scripts/build_synty_complete_inventory.py --check
python3 scripts/promote_world_assets.py check \
  --repo-root "$PWD" \
  --cache-root "$HOME/game-dev/rpg-game-assets" \
  --config scripts/configs/world-asset-promotion/dark-fortress-world-assets-v1.json
git diff --check
git status --short
git push -u origin "$(git branch --show-current)"
```

Expected: PASS and clean status. Open one PR to Assets `main`, but do not merge until Task 11's pre-merge visual gate passes and the final independent review is published.

---

### Task 11: Bind merged provider authority and complete the real World Builder gate

**Repository:** `rpg-dnd5e-web`

**Files:**
- Regenerate: `src/generated/worldAssetCatalog.ts`
- Modify only if the real gate finds a tested contract defect: files already owned by Tasks 5, 6, and 9

**Interfaces:**
- Consumes: the clean Assets provider branch for the pre-merge gate, then a clean detached worktree at merged Assets `origin/main`.
- Produces: exact merged-provider metadata and browser evidence that Ready assets are usable in the real world-building concept.

- [ ] **Step 1: Run the pre-merge provider visual gate**

Set `ASSETS_PROVIDER_WORKTREE` to the clean Assets implementation worktree and run:

```bash
test -z "$(git -C "$ASSETS_PROVIDER_WORKTREE" status --porcelain)"
RPG_GAME_ASSETS_PATH="$ASSETS_PROVIDER_WORKTREE" npm run world-assets:sync
RPG_GAME_ASSETS_PATH="$ASSETS_PROVIDER_WORKTREE" npm run world-assets:check
npm run dev -- --host 127.0.0.1 --port 5173
```

Kirk verifies each first-batch Ready asset can be searched, placed, moved, grouped, saved, reopened, and rendered without WebGL or asset-load errors. A support surface with `supportsDecoration: true` must accept an existing candle/book placement. Wrong provider transforms return to Task 10; do not add Web repair transforms.

- [ ] **Step 2: Finish and merge the Assets implementation PR**

Run one fresh independent review on the final Assets head, address valid findings on the same branch, rerun affected gates, and publish the final-head verdict. Merge through GitHub only after Kirk's visual verdict.

- [ ] **Step 3: Create a clean merged-provider verification worktree**

```bash
git -C "$HOME/game-dev/rpg-game-assets" fetch origin --prune
git -C "$HOME/game-dev/rpg-game-assets" worktree remove --force /tmp/rpg-game-assets-world-provider-main 2>/dev/null || true
git -C "$HOME/game-dev/rpg-game-assets" worktree add --detach \
  /tmp/rpg-game-assets-world-provider-main origin/main

test -z "$(git -C /tmp/rpg-game-assets-world-provider-main status --porcelain)"
```

Expected: detached clean worktree at the exact merged provider commit.

- [ ] **Step 4: Rebind Web metadata to merged provider authority**

```bash
RPG_GAME_ASSETS_PATH=/tmp/rpg-game-assets-world-provider-main npm run world-assets:sync
RPG_GAME_ASSETS_PATH=/tmp/rpg-game-assets-world-provider-main npm run world-assets:check
git add src/generated/worldAssetCatalog.ts
git commit -m "chore: bind merged world-asset provider"
```

If sync also updates a tracked provider receipt/lock file defined by the generator, include that exact generated file in the same commit. Do not stage ignored `public/models/synty/` GLBs.

- [ ] **Step 5: Run complete Web gates**

```bash
RPG_GAME_ASSETS_PATH=/tmp/rpg-game-assets-world-provider-main npm run world-assets:check
npm test -- --run \
  scripts/generateWorldAssetCatalog.test.ts \
  src/components/hex-grid/WorldAssetModel.test.tsx \
  src/concepts/world-building/serialization.test.ts \
  src/concepts/world-building/WorldBuildingViewport.test.tsx \
  src/concepts/world-building/WorldBuildingConcept.test.tsx
npm run ci-check
git diff --check
git status --short
```

Expected: PASS and clean status.

- [ ] **Step 6: Review and publish the Web implementation PR**

Push the one Web issue branch and open a PR to `dev`. Run one fresh independent review on the final head, address valid findings on the same branch, rerun affected gates, and publish the final-head verdict. Merge through GitHub only after Kirk confirms the merged-provider assets remain correct in the real World Builder.

- [ ] **Step 7: Close the design tracking loop**

Update rpg-project PR #395 with the merged Assets/Web PRs, exact provider/Web commits, test evidence, first-batch refs, and Kirk's visual verdict. Merge #395 only after implementation completes, then close #394 and update Journey #365 without overwriting another operator's active file.
