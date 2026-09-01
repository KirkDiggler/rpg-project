# Character Customization Concept Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Deliver a durable provider-backed Character Customization Concept that proves or rejects separately loaded skinned scalp/facial-hair accessories and instance-local runtime surface controls on an animated Dwarf Fighter.

**Architecture:** A private provider publishes one Concept-only hairless body, six independently loadable skinned accessories, and an exact manifest. Shared Three.js units rebind an accessory to the body’s existing skeleton and clone a PBR material per instance; the fixture-backed web Concept drives those units without persistence or production-route wiring.

**Tech Stack:** Python 3 stdlib, Blender 5.x Python API, deterministic GLB/JSON promotion, React 19, TypeScript 5.8, Three.js 0.181, React Three Fiber 9, Drei 10, Vitest 4, React Testing Library, Playwright/browser evidence.

**Spec:** `ideas/characters/customization/design.md`

## Global Constraints

- Parent Journey: `rpg-project#338`; design PR: `rpg-project#339`.
- Provider slice: `rpg-game-assets#107`, fresh worktree from latest `origin/main`.
- Web slice: `rpg-dnd5e-web#877`, fresh worktree from latest `origin/dev`.
- Develop provider candidate and web consumer together; on success merge provider first, then sync and verify the exact merged provider bytes before web merge.
- Existing production GLBs, `modular-fantasy-hero-v1`, `[1.08, 0.78, 1.08]`, provider arm correction, `Idle_Relaxed`, `Walk_Forward`, canonical root, and modular weapon sockets remain unchanged.
- Scalp and facial hair select independently; `default`, explicit `none`, and exact style refs are distinct.
- One arbitrary sRGB base color applies to both slots. Roughness and metalness are Concept-only experimental values in `[0, 1]`.
- No proto, API, toolkit, Redis, draft, roster, character-creation, finalized-character editing, or production session-route change.
- No licensed source FBX/texture, private absolute path, or synced GLB is committed to the public web repository.
- Provider failure leaves every tracked runtime output unchanged. Accessory failure leaves the body rendered and reports an explicit status.
- Do not retain a duplicate accessory armature or run an accessory animation mixer in the browser.
- Every implementation PR gets one fresh independent final review; publish the reviewed head and verdict on the PR.

## File map

### `rpg-game-assets#107`

- `scripts/configs/character-customization-concept-v1.json` — exact source, body, style, output, and surface declaration.
- `scripts/character_customization_concept_contract.py` — pure schema, staging, report, and manifest contract.
- `scripts/build_character_customization_concept.py` — Blender body/accessory exporter and structural readback.
- `scripts/promote_character_customization_concept.py` — atomic stage/build/validate/apply/check command.
- `scripts/render_character_customization_concept.py` — deterministic provider contact sheets.
- `scripts/test_character_customization_concept_contract.py` — pure contract refusal/manifest tests.
- `scripts/test_build_character_customization_concept.py` — Blender-independent GLB/report validation tests plus opt-in real build test.
- `scripts/test_promote_character_customization_concept.py` — staging, rollback, preservation, and check tests.
- `scripts/test_render_character_customization_concept.py` — evidence input/output contract tests.
- `harness/models/synty/concepts/character-customization/**` — one body, six accessories, generated manifest.
- `harness/models/synty/mesh-stats.json` and `harness/catalogs/synty-complete-inventory.json` — regenerated complete inventories.
- `evidence/107-character-customization-concept/**` — provider evidence and verification receipt.
- `README.md` — supported Concept runtime root and boundaries.

### `rpg-dnd5e-web#877`

- `src/components/hex-grid/runtimeSurfaceTreatment.ts` — validate and clone instance-local PBR treatment.
- `src/components/hex-grid/runtimeSurfaceTreatment.test.ts` — color/PBR/refusal/isolation/disposal tests.
- `src/components/hex-grid/skinnedAccessory.ts` — pure exact-skeleton rebinding algorithm and result types.
- `src/components/hex-grid/skinnedAccessory.test.ts` — synthetic skeleton success/refusal tests.
- `src/components/hex-grid/SkinnedAccessoryAttachment.tsx` — Drei/R3F loader, mount lifecycle, and status reporting.
- `src/components/hex-grid/SkinnedAccessoryAttachment.test.tsx` — mount/change/unmount behavior.
- `src/components/hex-grid/ClassCharacterModel.tsx` and `.test.tsx` — optional shared accessory seam; existing callers unchanged.
- `src/concepts/character-customization/characterCustomizationAssets.ts` — provider-backed Concept options and defaults.
- `src/concepts/character-customization/characterCustomizationExperiment.ts` and `.test.ts` — provisional fixture, presets, observations, coverage, verdict.
- `src/concepts/character-customization/CharacterCustomizationPreview.tsx` — two-instance real renderer and cameras.
- `src/concepts/character-customization/CharacterCustomizationConcept.tsx` and `.test.tsx` — controls, inspector, and registration coverage.
- `src/concepts/character-customization/CONTRACT.md` — observed evidence and accepted/rejected boundary.
- `src/concepts/ConceptsView.tsx` — `character-customization` deep-link registration.
- `scripts/characterCustomizationConceptPublication.test.ts` — exact provider/evidence publication receipt.
- `docs/evidence/877-character-customization-concept/**` — browser proof and exact receipt.
- `docs/architecture/components/concepts-route.md`, `docs/how-to/concepts-route.md`, and `src/concepts/README.md` — durable Concept index.

---

### Task 1: Provider recipe and pure contract

**Files:**
- Create: `scripts/configs/character-customization-concept-v1.json`
- Create: `scripts/character_customization_concept_contract.py`
- Create: `scripts/test_character_customization_concept_contract.py`

**Interfaces:**
- Consumes: `modular_race_class_contract.load_recipe`, the approved `dwarf:fighter` combination, and an explicit private source root.
- Produces:
  - `load_concept_recipe(path: Path) -> dict[str, object]`
  - `stage_concept_inputs(source_root: Path, recipe: dict[str, object], stage_root: Path) -> StagedConceptInputs`
  - `validate_build_report(report: object, declaration: dict[str, object]) -> dict[str, object]`
  - `build_manifest(recipe: dict[str, object], reports: dict[str, dict[str, object]]) -> dict[str, object]`
  - `manifest_bytes(document: dict[str, object]) -> bytes`

- [x] **Step 1: Create the provider issue worktree**

```bash
git -C /home/kirk/game-dev/rpg-game-assets fetch origin
git -C /home/kirk/game-dev/rpg-game-assets worktree add \
  -b learn/107-character-customization-concept \
  /home/kirk/.pi/worktrees/rpg-game-assets/107-character-customization-concept \
  origin/main
```

Expected: clean branch named for `rpg-game-assets#107`; do not modify the active off-hand worktree.

- [x] **Step 2: Write failing schema tests**

```python
class ConceptRecipeTests(unittest.TestCase):
    def test_loads_exact_dwarf_fighter_candidate_contract(self):
        recipe = load_concept_recipe(CONFIG)
        self.assertEqual("dwarf:fighter", recipe["bodyCombination"])
        self.assertEqual(
            ["Chr_Hair_04", "Chr_Hair_08", "Chr_Hair_16"],
            [row["sourceMesh"] for row in recipe["slots"]["scalp"]["options"]],
        )
        self.assertEqual(
            ["Chr_FacialHair_Male_01", "Chr_FacialHair_Male_02", "Chr_FacialHair_Male_03"],
            [row["sourceMesh"] for row in recipe["slots"]["facial-hair"]["options"]],
        )
        self.assertEqual("modular-fantasy-hero:hair:04", recipe["slots"]["scalp"]["defaultStyleRef"])
        self.assertEqual("modular-fantasy-hero:facial-hair:02", recipe["slots"]["facial-hair"]["defaultStyleRef"])

    def test_rejects_none_as_a_provider_style_ref(self):
        document = json.loads(CONFIG.read_text())
        document["slots"]["scalp"]["options"][0]["styleRef"] = "none"
        with self.assertRaisesRegex(ContractError, "reserved style ref"):
            validate_recipe_document(document)
```

- [x] **Step 3: Run the focused tests and observe the missing module failure**

Run:

```bash
python3 -m unittest scripts.test_character_customization_concept_contract -v
```

Expected: FAIL because `character_customization_concept_contract` and its config do not exist.

- [x] **Step 4: Add the exact config**

Use these stable public refs and runtime paths:

```json
{
  "schemaVersion": 1,
  "workflowVersion": "character-customization-concept-v1",
  "baseRecipe": "scripts/configs/character-promotion/modular-race-class-v1.json",
  "bodyCombination": "dwarf:fighter",
  "outputRoot": "harness/models/synty/concepts/character-customization",
  "manifest": "harness/models/synty/concepts/character-customization/manifest.json",
  "body": {
    "excludedMeshes": ["Chr_Hair_04", "Chr_FacialHair_Male_02"],
    "output": "harness/models/synty/concepts/character-customization/dwarf-fighter-body.glb"
  },
  "surface": {
    "mode": "uniform-pbr-v1",
    "defaultBaseColorSrgb": "#5A3825",
    "defaultRoughness": 0.72,
    "defaultMetalness": 0.0
  },
  "slots": {
    "scalp": {
      "defaultStyleRef": "modular-fantasy-hero:hair:04",
      "options": [
        {"styleRef": "modular-fantasy-hero:hair:04", "label": "Hair 04", "sourceMesh": "Chr_Hair_04", "output": "harness/models/synty/concepts/character-customization/scalp/hair-04.glb"},
        {"styleRef": "modular-fantasy-hero:hair:08", "label": "Hair 08", "sourceMesh": "Chr_Hair_08", "output": "harness/models/synty/concepts/character-customization/scalp/hair-08.glb"},
        {"styleRef": "modular-fantasy-hero:hair:16", "label": "Hair 16", "sourceMesh": "Chr_Hair_16", "output": "harness/models/synty/concepts/character-customization/scalp/hair-16.glb"}
      ]
    },
    "facial-hair": {
      "defaultStyleRef": "modular-fantasy-hero:facial-hair:02",
      "options": [
        {"styleRef": "modular-fantasy-hero:facial-hair:01", "label": "Facial Hair 01", "sourceMesh": "Chr_FacialHair_Male_01", "output": "harness/models/synty/concepts/character-customization/facial-hair/facial-hair-01.glb"},
        {"styleRef": "modular-fantasy-hero:facial-hair:02", "label": "Facial Hair 02", "sourceMesh": "Chr_FacialHair_Male_02", "output": "harness/models/synty/concepts/character-customization/facial-hair/facial-hair-02.glb"},
        {"styleRef": "modular-fantasy-hero:facial-hair:03", "label": "Facial Hair 03", "sourceMesh": "Chr_FacialHair_Male_03", "output": "harness/models/synty/concepts/character-customization/facial-hair/facial-hair-03.glb"}
      ]
    }
  }
}
```

- [x] **Step 5: Implement strict pure validation and staging**

Use dataclasses and exact-key validation:

```python
@dataclass(frozen=True)
class StagedConceptInputs:
    recipe: dict[str, object]
    base_recipe: dict[str, object]
    source_fbx: Path
    atlas: Path
    animation_source: Path
    output_root: Path

RESERVED_STYLE_REFS = {"", "default", "none"}
SLOTS = ("scalp", "facial-hair")
SURFACE_MODE = "uniform-pbr-v1"
```

Reject unknown/missing keys, duplicate refs/paths/source meshes, non-portable paths, wrong Dwarf combination/proportions, absent defaults, unsupported surface mode, and any hash mismatch while staging. Reuse the base recipe’s pinned archive/member/atlas/animation facts instead of duplicating hashes.

- [x] **Step 6: Write manifest tests before manifest implementation**

```python
def test_manifest_binds_every_output_to_report_hash_and_weighted_bones(self):
    manifest = build_manifest(self.recipe, self.reports)
    self.assertEqual("modular-fantasy-hero-v1", manifest["rigFamily"])
    self.assertEqual(["Idle_Relaxed", "Walk_Forward"], manifest["body"]["animations"])
    for slot in ("scalp", "facial-hair"):
        for option in manifest["slots"][slot]["options"]:
            self.assertRegex(option["sha256"], r"^[0-9a-f]{64}$")
            self.assertEqual(63, option["skeleton"]["boneCount"])
            self.assertGreater(len(option["skeleton"]["weightedBones"]), 0)
            self.assertEqual([], option["animations"])
```

- [x] **Step 7: Implement deterministic manifest assembly and run tests**

`manifest_bytes` must use sorted keys, two-space indentation, UTF-8, and a trailing newline. Reports must bind exact output path, hash, mesh, skeleton order, weighted bones, inverse-bind digest, material mode, animation list, and body socket profile.

Run:

```bash
python3 -m unittest scripts.test_character_customization_concept_contract -v
git diff --check
```

Expected: PASS.

- [x] **Step 8: Commit the pure contract**

```bash
git add scripts/configs/character-customization-concept-v1.json \
  scripts/character_customization_concept_contract.py \
  scripts/test_character_customization_concept_contract.py
git commit -m "learn: define hair accessory concept contract (#107)"
```

---

### Task 2: Deterministic Blender body and accessory exporter

**Files:**
- Create: `scripts/build_character_customization_concept.py`
- Create: `scripts/test_build_character_customization_concept.py`
- Modify only if required for import safety: `scripts/build_modular_race_class.py`

**Interfaces:**
- Consumes: `StagedConceptInputs`, the base Dwarf Fighter declaration, and existing modular builder low-level functions.
- Produces:
  - `build_body(staged: StagedConceptInputs, output: Path, report: Path) -> dict[str, object]`
  - `build_accessory(staged: StagedConceptInputs, declaration: dict[str, object], output: Path, report: Path) -> dict[str, object]`
  - Blender CLI accepts `--recipe`, `--source-root`, `--asset`, `--output`, and `--report`; `--asset` is either literal `body` or one style ref declared in the recipe.

- [x] **Step 1: Write failing report-validation tests**

```python
def test_body_report_requires_hairless_animated_dwarf(self):
    report = load_fixture("body-report.json")
    self.assertEqual(12, report["meshes"]["count"])
    self.assertNotIn("Chr_Hair_04", report["meshes"]["names"])
    self.assertNotIn("Chr_FacialHair_Male_02", report["meshes"]["names"])
    self.assertEqual(["Idle_Relaxed", "Walk_Forward"], report["animations"])
    self.assertEqual([1.08, 0.78, 1.08], report["proportions"])


def test_accessory_report_requires_one_mesh_full_shared_skeleton_and_no_clips(self):
    report = load_fixture("hair-08-report.json")
    self.assertEqual(["Chr_Hair_08"], report["meshes"]["names"])
    self.assertEqual(63, report["skeleton"]["boneCount"])
    self.assertEqual([], report["animations"])
    self.assertEqual("uniform-pbr-v1", report["material"]["mode"])
    self.assertEqual(0, report["material"]["textureCount"])
```

- [x] **Step 2: Run focused tests and observe failure**

```bash
python3 -m unittest scripts.test_build_character_customization_concept -v
```

Expected: FAIL because exporter/report fixtures do not exist.

- [x] **Step 3: Implement the body build using existing modular primitives**

The body path must execute this exact sequence:

```python
reset_scene()
target_added = import_fbx(staged.source_fbx)
body_mesh_names = [
    name for name in base_combination["meshes"]
    if name not in recipe["body"]["excludedMeshes"]
]
target_armature, meshes = collect_target(target_added, body_mesh_names)
bake_proportions_into_rig_and_meshes(target_armature, meshes, [1.08, 0.78, 1.08])
bake_tpose_arm_chain_length(target_armature, meshes, [1.08, 0.78, 1.08])
bind_single_atlas(meshes, staged.atlas)
# Transfer exactly Idle_Relaxed and Walk_Forward using the existing mapping/rest-correction helpers.
export_selected(output, target_armature, meshes, ["Idle_Relaxed", "Walk_Forward"])
```

Do not alter production `build_combination` behavior. If Blender-import guards currently make the low-level module unimportable, move only the `bpy`-dependent main invocation behind its existing runtime check and prove all modular tests remain unchanged.

- [x] **Step 4: Implement accessory export against the identical transformed armature**

```python
reset_scene()
added = import_fbx(staged.source_fbx)
armature, meshes = collect_target(added, [declaration["sourceMesh"]])
bake_proportions_into_rig_and_meshes(armature, meshes, [1.08, 0.78, 1.08])
bind_uniform_pbr_material(
    meshes[0],
    name="CustomizationSurface",
    base_color=(1.0, 1.0, 1.0, 1.0),
    roughness=0.72,
    metalness=0.0,
)
clear_nla(armature)
export_selected(output, armature, meshes, [])
```

`bind_uniform_pbr_material` must clear inherited slots and assign one opaque node-based Principled BSDF material with no image texture. Preserve geometry, vertex groups, full armature order, and inverse binds.

- [x] **Step 5: Add structural GLB readback**

Implement pure GLB inspection that proves:

```python
{
    "schemaVersion": 1,
    "assetKind": "body" | "accessory",
    "output": "portable/path.glb",
    "sha256": "...",
    "rigFamily": "modular-fantasy-hero-v1",
    "proportions": [1.08, 0.78, 1.08],
    "meshes": {"count": 1, "names": ["Chr_Hair_08"]},
    "skeleton": {
        "boneCount": 63,
        "boneNames": ["Root", "..."],
        "weightedBones": ["Head", "..."],
        "inverseBindSha256": "..."
    },
    "material": {
        "mode": "uniform-pbr-v1",
        "count": 1,
        "textureCount": 0,
        "opaque": true
    },
    "animations": []
}
```

The actual bone list comes from readback and is hash-bound; do not invent or truncate it in the report.

- [x] **Step 6: Run one real body and one real accessory build**

```bash
blender --background --python-exit-code 1 \
  --python scripts/build_character_customization_concept.py -- \
  --recipe scripts/configs/character-customization-concept-v1.json \
  --source-root /home/kirk/Downloads/synty \
  --asset body \
  --output .stage/107-customization/body/dwarf-fighter-body.glb \
  --report .stage/107-customization/body/build-report.json

blender --background --python-exit-code 1 \
  --python scripts/build_character_customization_concept.py -- \
  --recipe scripts/configs/character-customization-concept-v1.json \
  --source-root /home/kirk/Downloads/synty \
  --asset modular-fantasy-hero:hair:08 \
  --output .stage/107-customization/hair-08/hair-08.glb \
  --report .stage/107-customization/hair-08/build-report.json
```

Expected: body has 12 meshes/two clips; accessory has one mesh/zero clips/63 ordered bones/one untextured opaque PBR material.

- [x] **Step 7: Add clean-rebuild equality and run focused tests**

Build Hair 08 twice into sibling stage directories and assert SHA-256 equality for GLB and canonical report. Run:

```bash
python3 -m unittest \
  scripts.test_character_customization_concept_contract \
  scripts.test_build_character_customization_concept -v
```

Expected: PASS.

- [x] **Step 8: Commit the builder**

```bash
git add scripts/build_character_customization_concept.py \
  scripts/test_build_character_customization_concept.py \
  scripts/build_modular_race_class.py
git commit -m "learn: export skinned customization accessories (#107)"
```

Omit `scripts/build_modular_race_class.py` from `git add` when no import-safety edit was needed.

---

### Task 3: Atomic provider promotion and evidence

**Files:**
- Create: `scripts/promote_character_customization_concept.py`
- Create: `scripts/test_promote_character_customization_concept.py`
- Create: `scripts/render_character_customization_concept.py`
- Create: `scripts/test_render_character_customization_concept.py`
- Modify: `scripts/build_synty_complete_inventory.py`
- Modify: `scripts/build_mesh_stats.py`
- Modify: relevant inventory/catalog tests only where the new Concept path must be classified
- Modify: `README.md`
- Create after real build: `harness/models/synty/concepts/character-customization/**`
- Create after real build: `evidence/107-character-customization-concept/**`
- Regenerate: `harness/models/synty/mesh-stats.json`
- Regenerate: `harness/catalogs/synty-complete-inventory.json`

**Interfaces:**
- Consumes: all seven validated build reports and outputs.
- Produces:
  - CLI phases `--build`, `--validate`, `--apply`, and `--check`
  - one deterministic manifest and all-or-nothing runtime installation
  - provider verification receipt and contact sheets

- [x] **Step 1: Write failing atomic-promotion tests**

```python
def test_validate_refuses_one_missing_accessory(self):
    stage = self.complete_stage()
    (stage / "facial-hair-03" / "facial-hair-03.glb").unlink()
    with self.assertRaisesRegex(ValueError, "facial-hair:03"):
        validate_stage(stage, self.recipe)


def test_apply_rolls_back_all_outputs_when_install_fails(self):
    before = snapshot_tree(self.runtime_root)
    with mock.patch("os.replace", side_effect=OSError("injected")):
        with self.assertRaisesRegex(OSError, "injected"):
            apply_release(self.complete_stage(), self.repo_root)
    self.assertEqual(before, snapshot_tree(self.runtime_root))
```

Also snapshot every pre-existing tracked `.glb` path/hash and assert promotion changes none of them.

- [x] **Step 2: Run focused tests and observe failure**

```bash
python3 -m unittest scripts.test_promote_character_customization_concept -v
```

Expected: FAIL because promotion code does not exist.

- [x] **Step 3: Implement build/validate/apply/check**

`--build` invokes Blender with `--python-exit-code 1` once per declared asset into the lexical `.stage/107-customization` directory. `--validate` requires all reports, exact output/report hashes, one 63-bone armature per export, unique skin-joint names, per-name inverse-bind compatibility between every accessory joint and the body, body-only clips, accessory-only material mode, and portable paths. `--apply` prepares temp siblings and atomically installs all seven GLBs plus manifest, restoring all previous files on any exception. `--check` byte-compares tracked files against a clean rebuilt release.

- [x] **Step 4: Add inventory/mesh-stat classification tests**

```python
def test_concept_accessories_are_counted_as_skinned_character_parts(self):
    stats = stats_for_glb(HAIR_08, "concepts/character-customization/scalp/hair-08.glb")
    self.assertEqual("character-accessory", stats["assetClass"])
    self.assertEqual(0, stats["animationClipCount"])


def test_complete_inventory_includes_every_concept_runtime_file(self):
    inventory = json.loads(INVENTORY.read_text())
    paths = {row["path"] for row in inventory["files"]}
    self.assertIn("models/synty/concepts/character-customization/manifest.json", paths)
    self.assertIn("models/synty/concepts/character-customization/scalp/hair-08.glb", paths)
```

- [x] **Step 5: Build and validate the full real candidate set**

```bash
python3 scripts/promote_character_customization_concept.py \
  --recipe scripts/configs/character-customization-concept-v1.json \
  --source-root /home/kirk/Downloads/synty \
  --stage-root .stage/107-customization \
  --build --validate
```

Expected: seven deterministic outputs and one canonical staged manifest; no tracked runtime file changed yet.

- [x] **Step 6: Implement and render provider evidence**

`render_character_customization_concept.py` must create:

- `dwarf-scalp-candidates-idle-walk.png`
- `dwarf-facial-hair-candidates-idle-walk.png`
- `dwarf-hairless-body-socket-witness.png`

The script imports the staged body/accessories, drives the body’s sampled idle/walk frames, and renders candidate rows. These sheets establish candidate identity and gross fit; browser rebinding remains authoritative for runtime success.

- [x] **Step 7: Apply, regenerate, and verify provider state**

```bash
python3 scripts/promote_character_customization_concept.py \
  --recipe scripts/configs/character-customization-concept-v1.json \
  --source-root /home/kirk/Downloads/synty \
  --stage-root .stage/107-customization \
  --apply
python3 scripts/build_mesh_stats.py
python3 scripts/build_synty_complete_inventory.py
python3 scripts/build_web_asset_catalog.py --check
python3 scripts/build_synty_complete_inventory.py --check
python3 scripts/promote_character_customization_concept.py \
  --recipe scripts/configs/character-customization-concept-v1.json \
  --source-root /home/kirk/Downloads/synty \
  --stage-root .stage/107-clean-check \
  --check
```

- [x] **Step 8: Run provider gates**

```bash
python3 -m unittest discover -s scripts -p 'test_*.py'
python3 -m compileall -q scripts
git diff --check
```

Expected: all tests pass; only established environment-gated skips; every pre-existing GLB hash unchanged; no private absolute path in tracked files.

- [x] **Step 9: Commit the complete provider candidate**

```bash
git add README.md scripts harness evidence/107-character-customization-concept
git commit -m "learn: publish Dwarf customization concept assets (#107)"
```

Do not open or merge the provider PR until Task 6 proves the browser consumer locally.

---

### Task 4: Pure skeleton rebinding and surface treatment

**Files:**
- Create: `src/components/hex-grid/skinnedAccessory.ts`
- Create: `src/components/hex-grid/skinnedAccessory.test.ts`
- Create: `src/components/hex-grid/runtimeSurfaceTreatment.ts`
- Create: `src/components/hex-grid/runtimeSurfaceTreatment.test.ts`

**Interfaces:**
- Produces:

```typescript
export interface RuntimeSurfaceTreatment {
  readonly baseColorSrgb: `#${string}`;
  readonly roughness: number;
  readonly metalness: number;
}

export type SkinnedAccessoryBindResult =
  | {
      readonly ok: true;
      readonly mesh: THREE.SkinnedMesh;
      readonly mappedBoneNames: readonly string[];
      readonly bodyRootBoneUuid: string;
      readonly mappedBoneUuids: readonly string[];
      readonly ownsSkeletonWrapper: boolean;
    }
  | {
      readonly ok: false;
      readonly code:
        | 'body-skeleton-count'
        | 'accessory-mesh-count'
        | 'duplicate-body-bone'
        | 'missing-body-bone'
        | 'inverse-bind-mismatch'
        | 'bind-matrix-mismatch';
      readonly message: string;
      readonly missingBoneNames: readonly string[];
    };

export function bindSkinnedAccessory(
  bodyRoot: THREE.Object3D,
  accessoryRoot: THREE.Object3D,
  tolerance?: number
): SkinnedAccessoryBindResult;

export function applyRuntimeSurfaceTreatment(
  mesh: THREE.SkinnedMesh,
  treatment: RuntimeSurfaceTreatment
): readonly THREE.Material[];
```

- [x] **Step 1: Create the web issue worktree and sync the local provider candidate**

```bash
git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin
git -C /home/kirk/game-dev/rpg-dnd5e-web worktree add \
  -b concept/877-character-customization \
  /home/kirk/.pi/worktrees/rpg-dnd5e-web/877-character-customization \
  origin/dev
RPG_GAME_ASSETS_PATH=/home/kirk/.pi/worktrees/rpg-game-assets/107-character-customization-concept \
  npm --prefix /home/kirk/.pi/worktrees/rpg-dnd5e-web/877-character-customization run assets:sync
```

Expected: `public/models/synty/` remains ignored with zero tracked files.

- [x] **Step 2: Write synthetic-skeleton failing tests**

Construct a body with one `THREE.Skeleton([Root, Spine, Head], inverses)` shared by two body meshes and an accessory skin using cloned `Root` and `Head` bones/inverses. Assert:

```typescript
const result = bindSkinnedAccessory(bodyRoot, accessoryRoot);
expect(result.ok).toBe(true);
if (result.ok) {
  expect(result.mesh.skeleton.bones).toEqual([bodyRootBone, bodyHeadBone]);
  expect(result.mesh.skeleton.bones).not.toContain(accessoryRootBone);
  expect(result.mappedBoneUuids).toEqual([
    bodyRootBone.uuid,
    bodyHeadBone.uuid,
  ]);
  expect(accessoryRoot.children).not.toContain(result.mesh);
}
```

Add one full-order case that reuses the exact body `Skeleton` object and one subset case that creates an owned `Skeleton` wrapper over body `Bone` objects. Add discriminating tests for duplicate body bone names, two accessory skinned meshes, missing bones, per-name inverse-bind delta above `1e-5`, and bind-matrix mismatch. No test may merely restate Three.js serialization.

- [x] **Step 3: Run focused tests and observe missing functions**

```bash
npm run test:run -- src/components/hex-grid/skinnedAccessory.test.ts src/components/hex-grid/runtimeSurfaceTreatment.test.ts
```

Expected: FAIL because modules do not exist.

- [x] **Step 4: Implement exact skeleton reuse**

`bindSkinnedAccessory` must:

1. collect one authoritative body skeleton from body `SkinnedMesh` nodes and build a unique body-bone map by exact name;
2. require exactly one accessory `SkinnedMesh`;
3. map every accessory skin joint to the body `Bone` with the same name;
4. compare each mapped bone's inverse-bind matrix by name and compare `bindMatrix`, with max absolute delta `<= 1e-5`;
5. reuse the exact body `Skeleton` object when names/order cover it exactly; otherwise create an owned `THREE.Skeleton` wrapper containing only mapped body `Bone` objects and the compatible source inverses;
6. detach the accessory mesh from its cloned source hierarchy, bind it to that skeleton, and return the detached mesh only, so no accessory bones enter the mounted scene; and
7. report wrapper ownership so lifecycle cleanup disposes only an owned wrapper's bone texture, never the body skeleton.

Never map by array index without comparing names and bind facts.

- [x] **Step 5: Implement treatment validation and material cloning**

Accept only `/^#[0-9A-F]{6}$/i`, finite roughness/metalness within `[0, 1]`, and `THREE.MeshStandardMaterial` sources. Clone every material, set `color`, `roughness`, and `metalness`, and set `needsUpdate = true`. Return created materials so the owner can dispose them; never dispose shared geometry, source materials, or the body skeleton.

- [x] **Step 6: Prove instance isolation and cleanup**

```typescript
const firstCreated = applyRuntimeSurfaceTreatment(firstMesh, RED_LEATHER);
const secondCreated = applyRuntimeSurfaceTreatment(secondMesh, BLOND_HAIR);
expect((firstMesh.material as THREE.MeshStandardMaterial).color.getHexString()).toBe('6b3f26');
expect((secondMesh.material as THREE.MeshStandardMaterial).color.getHexString()).toBe('d8b36a');
expect(sourceMaterial.color.getHexString()).toBe('ffffff');
firstCreated.forEach((material) => material.dispose());
expect(disposeSpy).toHaveBeenCalledTimes(firstCreated.length);
```

- [x] **Step 7: Run focused tests and commit**

```bash
npm run test:run -- src/components/hex-grid/skinnedAccessory.test.ts src/components/hex-grid/runtimeSurfaceTreatment.test.ts
npm run typecheck
git add src/components/hex-grid/skinnedAccessory* src/components/hex-grid/runtimeSurfaceTreatment*
git commit -m "concept: prove exact skinned accessory rebinding (#877)"
```

---

### Task 5: R3F attachment lifecycle and real character seam

**Files:**
- Create: `src/components/hex-grid/SkinnedAccessoryAttachment.tsx`
- Create: `src/components/hex-grid/SkinnedAccessoryAttachment.test.tsx`
- Modify: `src/components/hex-grid/ClassCharacterModel.tsx`
- Modify: `src/components/hex-grid/ClassCharacterModel.test.tsx`

**Interfaces:**
- Produces:

```typescript
export interface SkinnedAccessoryPresentation {
  readonly slot: 'scalp' | 'facial-hair';
  readonly styleRef: string;
  readonly url: string;
  readonly treatment: RuntimeSurfaceTreatment;
}

export type SkinnedAccessoryStatus =
  | { readonly code: 'none'; readonly slot: SkinnedAccessoryPresentation['slot'] }
  | { readonly code: 'loading'; readonly slot: SkinnedAccessoryPresentation['slot']; readonly styleRef: string }
  | { readonly code: 'attached'; readonly slot: SkinnedAccessoryPresentation['slot']; readonly styleRef: string; readonly url: string; readonly bodyRootBoneUuid: string; readonly mappedBoneNames: readonly string[]; readonly mappedBoneUuids: readonly string[] }
  | { readonly code: 'rejected'; readonly slot: SkinnedAccessoryPresentation['slot']; readonly styleRef: string; readonly url: string; readonly message: string };
```

`ClassCharacterModelProps` gains optional `accessories?: readonly SkinnedAccessoryPresentation[]` and `onAccessoryStatus?: (status: SkinnedAccessoryStatus) => void`.

- [x] **Step 1: Write failing component tests**

Mock `useGLTF` with synthetic body/accessory scenes. Assert:

- no accessory loader mounts when `accessories` is absent;
- one exact presentation emits `loading` then `attached`;
- style URL change removes the old mesh/material before mounting the new one;
- a rejected bind leaves the body primitive mounted and emits `rejected`;
- unmount removes the accessory mesh and disposes only cloned materials; and
- existing idle/walk/main-hand tests pass unchanged.

- [x] **Step 2: Run tests and observe missing component/props**

```bash
npm run test:run -- \
  src/components/hex-grid/SkinnedAccessoryAttachment.test.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx
```

Expected: FAIL on missing attachment component and props.

- [x] **Step 3: Implement the loader lifecycle**

`SkinnedAccessoryAttachment` uses `useGLTF(url)`, `SkeletonUtils.clone(scene)`, `bindSkinnedAccessory`, and `applyRuntimeSurfaceTreatment`. In one effect it adds the successfully rebound mesh to `characterRoot`; cleanup removes it, disposes only created materials, and disposes the rebound skeleton only when `ownsSkeletonWrapper` is true. Call R3F `invalidate()` after mount, treatment change, rejection, and cleanup.

Key the component by `${slot}|${styleRef}|${url}` so a style change cannot retain stale state. Report status only when it belongs to the current presentation identity.

- [x] **Step 4: Extend the real renderer without changing default behavior**

After the existing body `<primitive>` and before attachment slots, render:

```tsx
{accessories?.map((accessory) => (
  <SkinnedAccessoryAttachment
    key={`${accessory.slot}|${accessory.styleRef}|${accessory.url}`}
    characterRoot={cloned}
    presentation={accessory}
    onStatus={onAccessoryStatus}
  />
))}
```

No production caller supplies these props in this Journey.

- [x] **Step 5: Run renderer regression tests and commit**

```bash
npm run test:run -- \
  src/components/hex-grid/SkinnedAccessoryAttachment.test.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx \
  src/components/hex-grid/HexEntity.test.tsx
npm run typecheck
git add src/components/hex-grid/ClassCharacterModel* src/components/hex-grid/SkinnedAccessoryAttachment*
git commit -m "concept: mount skinned accessories on character models (#877)"
```

---

### Task 6: Durable Character Customization Concept

**Files:**
- Create: `src/concepts/character-customization/characterCustomizationAssets.ts`
- Create: `src/concepts/character-customization/characterCustomizationExperiment.ts`
- Create: `src/concepts/character-customization/characterCustomizationExperiment.test.ts`
- Create: `src/concepts/character-customization/CharacterCustomizationPreview.tsx`
- Create: `src/concepts/character-customization/CharacterCustomizationConcept.tsx`
- Create: `src/concepts/character-customization/CharacterCustomizationConcept.test.tsx`
- Create: `src/concepts/character-customization/CONTRACT.md`
- Modify: `src/concepts/ConceptsView.tsx`
- Modify: `src/concepts/README.md`
- Modify: `docs/architecture/components/concepts-route.md`
- Modify: `docs/how-to/concepts-route.md`

**Interfaces:**
- Consumes: exact provider URLs/style refs, optional `ClassCharacterModel.accessories`, and canonical weapon presentation.
- Produces:

```typescript
export type StyleSelection = 'default' | 'none' | string;
export type CustomizationMotion = 'idle' | 'walk';
export type CustomizationView = 'close' | 'orbit' | 'play';

export interface CharacterCustomizationFixture {
  readonly scalp: StyleSelection;
  readonly facialHair: StyleSelection;
  readonly treatment: RuntimeSurfaceTreatment;
  readonly motion: CustomizationMotion;
  readonly view: CustomizationView;
  readonly showWeaponWitness: boolean;
}
```

- [x] **Step 1: Write failing pure fixture/coverage tests**

Pin these presets:

```typescript
export const SURFACE_PRESETS = {
  hair: { baseColorSrgb: '#5A3825', roughness: 0.72, metalness: 0 },
  clothLike: { baseColorSrgb: '#5B6B8C', roughness: 0.95, metalness: 0 },
  leatherLike: { baseColorSrgb: '#6B3F26', roughness: 0.7, metalness: 0 },
  metalLike: { baseColorSrgb: '#9CA3AF', roughness: 0.25, metalness: 1 },
} as const;
```

Test default resolution, explicit `none`, unknown ref refusal, one shared treatment object for both attached slots, and verdict coverage requiring every style/none state, both motions, all three views, all four presets, one simultaneous non-default scalp+facial pair, and positive reference-twin isolation.

- [x] **Step 2: Run pure tests and observe failure**

```bash
npm run test:run -- src/concepts/character-customization/characterCustomizationExperiment.test.ts
```

Expected: FAIL because experiment/assets modules do not exist.

- [x] **Step 3: Implement exact provider-backed assets and pure resolution**

Use URL prefix `/models/synty/concepts/character-customization/`. `default` resolves to Hair 04 and Facial Hair 02. `none` returns no presentation. Unknown strings return an explicit unmapped resolution shown by the inspector; never silently choose default.

- [x] **Step 4: Write failing Concept interaction tests**

Assert the page:

- registers under `?concept=character-customization` with label `Character Customization`;
- exposes scalp/facial controls independently;
- includes `Default` and `None` for each;
- passes one changed color to both active accessory presentations;
- exposes roughness and metalness sliders with `[0, 1]` bounds;
- switches idle/walk and close/orbit/tactical views;
- shows exact style refs, URLs, statuses, mapped/missing bones, body root-bone identity, mapped bone identities, asset sizes, and fixture JSON;
- renders a controlled customized character and an untouched reference twin; and
- disables `Record Concept verdict` until complete positive coverage exists.

- [x] **Step 5: Implement the two-instance preview**

Follow `WeaponAttachmentPreview` camera conventions. The controlled body uses the Concept hairless Dwarf URL and selected accessories. The reference twin uses the same body with default Hair 04/Facial Hair 02 and immutable default treatment. Position the pair so close/orbit/tactical cameras keep both visible; the close view may focus the controlled head while retaining a labelled reference inset.

Use `HexColorPicker` from the already-installed `react-colorful`; no dependency change.

- [x] **Step 6: Implement diagnostics and observation recording**

Record an observation only after current scalp/facial statuses match the current fixture identities and the R3F scene commits. The inspector must label outputs `NON-PRODUCTION CONCEPT EVIDENCE`. It reports zero mounted accessory armatures by counting only mounted body bones and bound mesh status—not by guessing from URL success.

- [x] **Step 7: Register/document the Concept and run focused tests**

```bash
npm run test:run -- \
  src/concepts/character-customization/characterCustomizationExperiment.test.ts \
  src/concepts/character-customization/CharacterCustomizationConcept.test.tsx \
  src/components/hex-grid/SkinnedAccessoryAttachment.test.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx
npm run typecheck
```

Expected: PASS.

- [x] **Step 8: Run the local browser Learn before any provider merge**

```bash
VITE_API_HOST=http://localhost:8080 npm run dev -- --port 3014 --strictPort
```

Open `http://localhost:3014/?concept=character-customization`. Exercise every style, both `none` states, combined non-default hair/facial hair, arbitrary black/blond/red colors, all four PBR presets, idle/walk, all views, reference isolation, and weapon witness. Capture console/request output and decide whether exact-skeleton rebinding passes the design’s success gate.

- [x] **Step 9: Commit only after the mechanism passes locally**

```bash
git add src/components src/concepts docs/architecture/components/concepts-route.md docs/how-to/concepts-route.md
git commit -m "concept: add character customization lab (#877)"
```

If it fails, stop without this commit/PR, preserve measured evidence in `rpg-project#338`, and amend the design toward bundled selectable nodes.

---

### Task 7: Provider-first publication and exact web receipt

**Files:**
- Provider: PR/evidence files from Tasks 1–3
- Web create: `scripts/characterCustomizationConceptPublication.test.ts`
- Web create: `docs/evidence/877-character-customization-concept/README.md`
- Web create: `docs/evidence/877-character-customization-concept/receipt.json`
- Web create: accepted browser screenshots
- Web modify if exact merged paths/hashes changed: `src/concepts/character-customization/characterCustomizationAssets.ts`

**Interfaces:**
- Consumes: successful local browser verdict.
- Produces: exact provider merge lock and publication evidence.

- [x] **Step 1: Open the provider PR and run one independent final review**

Before push:

```bash
python3 -m unittest discover -s scripts -p 'test_*.py'
python3 -m compileall -q scripts
git diff --check
```

Open the PR for `rpg-game-assets#107`; publish one current-head independent verdict with Critical/Important/Minor counts and provider verification evidence.

- [x] **Step 2: Merge provider, record its exact merge, and recreate an exact provider view**

```bash
PROVIDER_PR=$(gh pr list --repo KirkDiggler/rpg-game-assets \
  --head learn/107-character-customization-concept \
  --json number --jq '.[0].number')
test -n "$PROVIDER_PR"
PROVIDER_MERGE_SHA=$(gh pr view "$PROVIDER_PR" \
  --repo KirkDiggler/rpg-game-assets \
  --json state,mergeCommit \
  --jq 'select(.state=="MERGED") | .mergeCommit.oid')
test "${#PROVIDER_MERGE_SHA}" -eq 40
PROVIDER_SHORT=${PROVIDER_MERGE_SHA:0:8}
PROVIDER_VIEW=/home/kirk/.pi/worktrees/rpg-game-assets/provider-$PROVIDER_SHORT
git -C /home/kirk/game-dev/rpg-game-assets fetch origin
git -C /home/kirk/game-dev/rpg-game-assets worktree add --detach \
  "$PROVIDER_VIEW" "$PROVIDER_MERGE_SHA"
```

Record the full merge SHA in the web receipt and run provider `--check` in `$PROVIDER_VIEW` with the explicit private source root.

- [x] **Step 3: Sync exact merged provider bytes into the web worktree**

```bash
RPG_GAME_ASSETS_PATH="$PROVIDER_VIEW" npm run assets:sync
git ls-files public/models/synty | wc -l
```

Expected: sync succeeds; tracked Synty file count is `0`.

- [x] **Step 4: Write the failing publication receipt test**

```typescript
it('pins the exact merged customization provider and seven outputs', () => {
  const receipt = loadReceipt();
  expect(receipt.providerCommit).toMatch(/^[0-9a-f]{40}$/);
  expect(receipt.providerManifestSha256).toMatch(/^[0-9a-f]{64}$/);
  expect(receipt.body.rigFamily).toBe('modular-fantasy-hero-v1');
  expect(receipt.body.animations).toEqual(['Idle_Relaxed', 'Walk_Forward']);
  expect(receipt.slots.scalp.options).toHaveLength(3);
  expect(receipt.slots.facialHair.options).toHaveLength(3);
  expect(receipt.productionGlbsChanged).toEqual([]);
});
```

Also assert every public path is under `/models/synty/concepts/character-customization/`, every hash is exact, no private path appears, and `.gitignore` still ignores `public/models/synty/`.

- [x] **Step 5: Generate the exact receipt from merged provider facts**

The receipt records provider commit, manifest/inventory hashes, seven path/hash/size rows, source mesh/style refs, shared bone-order/inverse-bind facts, default refs, surface mode, existing-production preservation digest, and browser observation summary. It records no timestamp-dependent hash input.

- [x] **Step 6: Repeat the full browser matrix against merged bytes**

Use `?concept=character-customization` on the web branch after exact sync. Capture at minimum:

- default Dwarf close view;
- alternate scalp + default beard in walk;
- default scalp + alternate facial hair in walk;
- combined alternates with arbitrary color;
- scalp none / facial none;
- cloth-like, leather-like, and metal-like comparison;
- controlled instance beside unchanged reference twin; and
- canonical weapon witness.

Proof windows must have zero unexpected console errors, page errors, or request failures; all seven GLBs must return HTTP 200 with exact provider hashes.

- [x] **Step 7: Commit exact publication evidence**

```bash
npm run test:run -- scripts/characterCustomizationConceptPublication.test.ts
npm run format
git add scripts/characterCustomizationConceptPublication.test.ts \
  docs/evidence/877-character-customization-concept \
  src/concepts/character-customization/characterCustomizationAssets.ts
git commit -m "docs: record customization concept evidence (#877)"
```

---

### Task 8: Full gates, review, and durable verdict

**Files:**
- Modify: `src/concepts/character-customization/CONTRACT.md`
- Modify: `docs/evidence/877-character-customization-concept/README.md`
- Modify in design branch: `ideas/characters/customization/design.md`
- Modify in design branch: `ideas/characters/customization/plan.md` checkbox state only after execution

**Interfaces:**
- Produces: one reviewed web PR, one observed Concept verdict, and an updated canonical design record.

- [x] **Step 1: Write the observed `CONTRACT.md` verdict**

Record exact provider/web heads, accepted and rejected candidates, bind status, bone/inverse-bind facts, accessory sizes, material behavior, clipping observations, cache/reference isolation, animation/socket results, browser URL, and Kirk’s words. Keep future proto fields explicitly provisional.

- [x] **Step 2: Run the focused web suite**

```bash
npm run test:run -- \
  src/components/hex-grid/skinnedAccessory.test.ts \
  src/components/hex-grid/runtimeSurfaceTreatment.test.ts \
  src/components/hex-grid/SkinnedAccessoryAttachment.test.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx \
  src/components/hex-grid/HexEntity.test.tsx \
  src/concepts/character-customization/characterCustomizationExperiment.test.ts \
  src/concepts/character-customization/CharacterCustomizationConcept.test.tsx \
  scripts/characterCustomizationConceptPublication.test.ts
```

Expected: PASS.

- [x] **Step 3: Run full web CI**

```bash
npm run ci-check
git diff --check
git ls-files public/models/synty | wc -l
```

Expected: format, lint, typecheck, build, and full tests pass; tracked Synty count `0`.

- [x] **Step 4: Open the web PR and run one independent final review**

Publish a current-head verdict comment naming review scope and all verification evidence. Address every valid finding on the same branch, rerun affected focused tests and `npm run ci-check`, and publish an updated verdict if the head changes materially.

- [x] **Step 5: Have Kirk walk the durable Concept**

Serve the exact reviewed head and open `?concept=character-customization`. Record whether separate skinned accessories and runtime PBR treatment are accepted as the production direction. This human visual verdict—not unit tests alone—decides the Learn.

- [x] **Step 6: Merge web after provider and verify exact merges/checks**

Confirm the provider merge is an ancestor of the receipt facts, the web PR is merged into `dev`, and all required jobs succeeded on the final head. Close `rpg-game-assets#107` and `rpg-dnd5e-web#877` as completed only after their exact merges are verified.

- [ ] **Step 7: Update and finish the canonical design record**

In the `rpg-project#339` worktree, append the exact implementation record and Kirk verdict to `design.md`; mark completed plan checkboxes; run:

```bash
git diff --check
python3 - <<'PY'
from pathlib import Path
text = "\n".join(path.read_text() for path in Path("ideas/characters/customization").glob("*.md"))
forbidden = ["TO" + "DO", "T" + "BD", "FIX" + "ME", "PLACE" + "HOLDER"]
assert not any(word in text for word in forbidden)
PY
```

Commit and push:

```bash
git add ideas/characters/customization/design.md ideas/characters/customization/plan.md
git commit -m "docs: record character customization Concept verdict (#338)"
git push
```

Merge design PR #339 only after the implementation outcome is known. Close Journey #338 when the durable success Concept is merged or the bounded failed Learn is fully recorded. A successful outcome starts a separate production customization journey whose first delivery layer is the evidence-derived proto contract.
