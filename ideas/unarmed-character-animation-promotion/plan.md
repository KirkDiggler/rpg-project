# Unarmed Character Animation Promotion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build and execute a reusable, config-driven private `rpg-game-assets` workflow that promotes a complete, validated four-class set of intentionally unarmed standing character A/B/C/D GLBs from approved checkpoints without changing canonical paths or the web resolver.

**Architecture:** A versioned Blender helper exports an approved checkpoint to an A-stage GLB; a Python promotion CLI reads a batch configuration, preserves the two named non-relaxed idles from the current canonical A model, produces B/C/D through the existing atlas swapper, and stages every generated/revised artifact under one release directory. A pure-stdlib validation module reads GLB JSON/BIN data and blocks promotion until structural, animation, parity, mesh/animation-QA, portrait-review, and evidence contracts pass; the CLI writes a transaction manifest and promotes or rolls back the complete release set only.

**Tech Stack:** Python 3 stdlib, Blender Python (`bpy`), existing `retarget_locomotion_common.py` GLB export behavior, existing `swap_atlas.py`, GLB JSON/BIN parsing modeled on `animation_qa.py`/`build_mesh_stats.py`, Node `sharp` evidence compositor, shell/git, and the existing web asset-sync/client test workflow.

## Global Constraints

- Primary tracked implementation repository: private `/home/kirk/game-dev/rpg-game-assets`; no Synty source, checkpoint `.blend`, FBX, or converted GLB may be placed in a public repository or public evidence location.
- Approved checkpoint sources are exactly `assets/synty/animation-retarget/approved/fighter-unarmed-idle-walk-approved.blend`, `barbarian-unarmed-idle-walk-approved.blend`, `monk-unarmed-idle-walk-approved.blend`, and `rogue-unarmed-idle-walk-approved.blend`.
- Canonical runtime filenames and the web resolver remain unchanged: `characters/fighter.glb`, `characters/barbarian.glb`, `characters/monk.glb`, and `characters/rogue.glb`.
- Standing outputs are intentionally unarmed. Standalone `characters/weapons/<class>-weapon.glb` files and their metadata remain available and untouched; each class weapon `bakedIntoModel` becomes `false`.
- Exact clips: fighter `Idle_Relaxed`, `Idle_Stretch`, `Idle_Drinking`, `Walk_Forward`; barbarian `Idle_Relaxed`, `Idle_ChinScratch`, `Idle_Drinking`, `Walk_Forward`; monk `Idle_Relaxed`, `Idle_Meditative`, `Idle_Drinking`, `Walk_Forward`; rogue `Idle_Relaxed`, `Idle_CheckWatch`, `Idle_Drinking`, `Walk_Forward`.
- Reuse the checkpoint `Idle_Relaxed` and `Walk_Forward`; preserve only the two non-relaxed idles from the current canonical A model, including their existing names and keyframe payloads.
- `idleClips` stays ordered `[Idle_Relaxed, class-distinctive idle, Idle_Drinking]`; `Walk_Forward` is embedded but is not an idle metadata entry.
- Every A/B/C/D output has Root name `Root`, Float32 scale `[0.009999999776482582, 0.009999999776482582, 0.009999999776482582]`, glTF `[x, y, z, w]` rotation `[0.70710688829422, 0, 0, 0.7071066498756409]`, exactly one identity-local-transform child, and assembly names `SK_BR_Character_Slayer_01`, `SK_BR_Character_BarbarianGiant_01`, `SK_Character_Mystic_01`, and `SK_Character_DarkElf_01`, respectively.
- Stage and validate the complete four-class A/B/C/D set before any canonical replacement. A failure preserves the complete canonical set and retains staging for diagnosis; post-promotion rollback restores the entire old A/B/C/D set, affected portraits, manifest/metadata, docs, reports, and evidence together.
- Direct glTF comparison against the pre-promotion canonical A file permits only weapon-node removal and animation payload changes; every other structural difference fails validation.
- `scripts/build_mesh_stats.py` and `PROPOSED_BUDGETS` are authoritative: changed standing models must produce no new warning relative to the captured pre-promotion mesh-stats report. `scripts/animation_qa.py` hard gates must pass; foot-slide remains informational only.
- Evidence is non-licensed screenshots only, published through the existing public `rpg-dnd5e-web` `evidence/asset-pipeline-wave1` branch under `playtest-evidence/asset-pipeline/unarmed-character-animation-promotion/`; no web product branch or tracked web-code change is allowed unless verification finds a separate consumer defect, which gets its own issue.
- Future batches change config/input and run this workflow. A new design is required only when the contract, not merely the class/checkpoint configuration, changes.
- Start implementation only after opening one `rpg-game-assets` issue and Board 19 item, then create one implementing branch/PR referencing `rpg-project#119` and design PR `rpg-project#120`. Do not make implementation commits while publishing this plan.

---

## File Map

### Create in `rpg-game-assets`

- `scripts/export_checkpoint_character.py` — focused Blender-only exporter: opens one approved checkpoint, retains only configured `Idle_Relaxed` and `Walk_Forward` actions, validates the one-armature/no-weapon export scene, and writes the staged A GLB without retargeting.
- `scripts/character_promotion_validation.py` — pure-Python GLB/manifest/report readers and deterministic validators for Root wrapper, assembly ownership, weapon-node absence, clip names/keyframe counts, preserved animation payloads, variant parity, constrained JSON differences, and staged release inventory.
- `scripts/promote_character_checkpoints.py` — reusable orchestration CLI with `stage`, `validate`, `promote`, and `rollback` commands; invokes the versioned helper and existing scripts, manages release directories and hashes, and never writes canonicals before `validate` succeeds.
- `scripts/configs/character-promotion/unarmed-checkpoints-v1.json` — versioned batch contract/configuration for the current four checkpoints, canonical paths, variant atlas paths, expected clips, Root assembly names, and manifest metadata.
- `scripts/test_character_promotion_validation.py` — stdlib `unittest` tests using synthetic GLB JSON/BIN fixtures plus real-path configuration tests; no Blender or licensed data required.
- `scripts/test_promote_character_checkpoints.py` — stdlib `unittest` tests for config loading, command construction, release inventory, validation-before-promotion, transaction journal, and rollback using temporary directories and mocked subprocesses.
- `harness/animation-qa/unarmed-character-animation-promotion-v1.json` — generated staged/release validation report, committed only after the real execution passes.

### Modify in `rpg-game-assets`

- `scripts/animation_qa.py` — add `--paths FILE [FILE ...]` so the authoritative existing analyzer can report the staged A/B/C/D files without scanning canonical `harness/`; preserve existing default catalog behavior.
- `scripts/test_animation_qa.py` — regression tests for `--paths` selection/error behavior.
- `harness/models/synty/characters/manifest.json` — update weapon commentary and class `weapon.bakedIntoModel` values to `false`; retain standalone weapon `file` and socket metadata; retain ordered `idleClips`.
- `harness/models/synty/characters/{fighter,barbarian,monk,rogue}{,-b,-c,-d}.glb` — generated promotion outputs only, never hand-edited.
- `harness/models/synty/characters/portraits/{fighter,barbarian,monk,rogue}{,-b,-c,-d}.png` — replace only for classes whose portrait contact-sheet review detects a weapon; otherwise leave byte-identical.
- `harness/models/synty/mesh-stats.json` — generated by the existing mesh-stats builder after promotion.
- `harness/animation-qa/report.json` and `harness/animation-qa/summary.txt` — generated by the existing animation QA command after promotion.
- `README.md` — replace armed-standing-model and color-variant wording with the intentionally-unarmed contract, standalone weapon status, and the reusable promotion command.

### External, non-product evidence

- `rpg-dnd5e-web` branch `evidence/asset-pipeline-wave1`: create `playtest-evidence/asset-pipeline/unarmed-character-animation-promotion/README.md`, portrait contact sheet, multi-angle `Idle_Relaxed` and `Walk_Forward` screenshots/strips, and actual-client screenshots/video-derived frames. These are evidence-only files, never source/GLB/FBX/.blend assets.

## Configuration And CLI Contract

`unarmed-checkpoints-v1.json` is the input schema. It has `schemaVersion: 1`, `batchId: "unarmed-character-animation-promotion-v1"`, `stagingRoot: "tmp/character-promotion/unarmed-character-animation-promotion-v1"`, `canonicalRoot: "harness/models/synty"`, `mapping: "scripts/configs/animation/polygon_to_rivals_bones.json"`, `atlasRoot: "/home/kirk/game-dev/assets/synty/polygon-fantasy-rivals/PolygonFantasyRivals_Source_Files/Source_Files/Textures"`, and `classes` containing one entry per class:

```json
{
  "schemaVersion": 1,
  "batchId": "unarmed-character-animation-promotion-v1",
  "stagingRoot": "tmp/character-promotion/unarmed-character-animation-promotion-v1",
  "canonicalRoot": "harness/models/synty",
  "mapping": "scripts/configs/animation/polygon_to_rivals_bones.json",
  "atlasRoot": "/home/kirk/game-dev/assets/synty/polygon-fantasy-rivals/PolygonFantasyRivals_Source_Files/Source_Files/Textures",
  "classes": {
    "fighter": {
    "checkpoint": "/home/kirk/game-dev/assets/synty/animation-retarget/approved/fighter-unarmed-idle-walk-approved.blend",
    "canonical": "characters/fighter.glb",
    "assembly": "SK_BR_Character_Slayer_01",
    "rigFamily": "bigrig",
    "checkpointClips": ["Idle_Relaxed", "Walk_Forward"],
    "idleClips": ["Idle_Relaxed", "Idle_Stretch", "Idle_Drinking"],
    "clips": ["Idle_Relaxed", "Idle_Stretch", "Idle_Drinking", "Walk_Forward"],
    "colors": {"a": "FantasyRivals_Texture_01_A.png", "b": "FantasyRivals_Texture_01_B.png", "c": "FantasyRivals_Texture_01_C.png", "d": "FantasyRivals_Texture_01_D.png"}
    },
    "barbarian": {
    "checkpoint": "/home/kirk/game-dev/assets/synty/animation-retarget/approved/barbarian-unarmed-idle-walk-approved.blend",
    "canonical": "characters/barbarian.glb",
    "assembly": "SK_BR_Character_BarbarianGiant_01",
    "rigFamily": "bigrig",
    "checkpointClips": ["Idle_Relaxed", "Walk_Forward"],
    "idleClips": ["Idle_Relaxed", "Idle_ChinScratch", "Idle_Drinking"],
    "clips": ["Idle_Relaxed", "Idle_ChinScratch", "Idle_Drinking", "Walk_Forward"],
    "colors": {"a": "FantasyRivals_Texture_01_A.png", "b": "FantasyRivals_Texture_01_B.png", "c": "FantasyRivals_Texture_01_C.png", "d": "FantasyRivals_Texture_01_D.png"}
    },
    "monk": {
    "checkpoint": "/home/kirk/game-dev/assets/synty/animation-retarget/approved/monk-unarmed-idle-walk-approved.blend",
    "canonical": "characters/monk.glb",
    "assembly": "SK_Character_Mystic_01",
    "rigFamily": "standard",
    "checkpointClips": ["Idle_Relaxed", "Walk_Forward"],
    "idleClips": ["Idle_Relaxed", "Idle_Meditative", "Idle_Drinking"],
    "clips": ["Idle_Relaxed", "Idle_Meditative", "Idle_Drinking", "Walk_Forward"],
    "colors": {"a": "FantasyRivals_Texture_01_A.png", "b": "FantasyRivals_Texture_01_B.png", "c": "FantasyRivals_Texture_01_C.png", "d": "FantasyRivals_Texture_01_D.png"}
    },
    "rogue": {
    "checkpoint": "/home/kirk/game-dev/assets/synty/animation-retarget/approved/rogue-unarmed-idle-walk-approved.blend",
    "canonical": "characters/rogue.glb",
    "assembly": "SK_Character_DarkElf_01",
    "rigFamily": "standard",
    "checkpointClips": ["Idle_Relaxed", "Walk_Forward"],
    "idleClips": ["Idle_Relaxed", "Idle_CheckWatch", "Idle_Drinking"],
    "clips": ["Idle_Relaxed", "Idle_CheckWatch", "Idle_Drinking", "Walk_Forward"],
    "colors": {"a": "FantasyRivals_Texture_01_A.png", "b": "FantasyRivals_Texture_01_B.png", "c": "FantasyRivals_Texture_01_C.png", "d": "FantasyRivals_Texture_01_D.png"}
    }
  }
}
```

The executable interface is:

```bash
python3 scripts/promote_character_checkpoints.py stage --config scripts/configs/character-promotion/unarmed-checkpoints-v1.json
python3 scripts/promote_character_checkpoints.py validate --config scripts/configs/character-promotion/unarmed-checkpoints-v1.json --release tmp/character-promotion/unarmed-character-animation-promotion-v1/release
python3 scripts/promote_character_checkpoints.py promote --config scripts/configs/character-promotion/unarmed-checkpoints-v1.json --release tmp/character-promotion/unarmed-character-animation-promotion-v1/release
python3 scripts/promote_character_checkpoints.py rollback --transaction tmp/character-promotion/unarmed-character-animation-promotion-v1/transaction.json
```

`stage` creates `baseline/`, `release/`, and `evidence/` below `stagingRoot`, copies all affected canonical artifacts to `baseline/`, and writes only below `release/`. It invokes Blender as `blender -b <checkpoint> -P scripts/export_checkpoint_character.py -- --out <release-A> --clip Idle_Relaxed --clip Walk_Forward`, merges the checkpoint actions with preserved non-relaxed actions through the existing `retarget_locomotion_common.py` action/export conventions (not a second retarget implementation), then runs `swap_atlas.py` once per B/C/D atlas. `validate` is read-only after report generation and returns nonzero on any gate. `promote` refuses an absent or hash-mismatched successful validation report; it records SHA-256s of every baseline/release path in `transaction.json`, atomically `os.replace`s each canonical target from a same-directory temporary file, and on any exception restores every target from `baseline/` before returning nonzero. The release is published as one git commit containing all generated assets, reports, docs, metadata, portraits-if-changed, and evidence references; that one commit is the reviewable atomic unit. `rollback` verifies the transaction baseline hashes, restores the whole recorded target inventory, regenerates mesh/QA reports, re-runs sync/client verification, and creates one complete rollback commit. No command silently promotes a subset.

## Tasks

### Task 1: Create the Batch Contract and Pure Validation Foundation

**Files:**
- Create: `scripts/configs/character-promotion/unarmed-checkpoints-v1.json`
- Create: `scripts/character_promotion_validation.py`
- Create: `scripts/test_character_promotion_validation.py`

**Interfaces:**
- Produces `load_config(path: str) -> dict`, `read_glb(path: str) -> tuple[dict, bytes]`, `animation_keyframe_counts(gltf: dict) -> dict[str, int]`, `validate_root(gltf: dict, assembly: str) -> list[str]`, `validate_clip_contract(gltf: dict, expected: list[str]) -> list[str]`, `validate_variant_parity(paths: dict[str, str], expected: list[str]) -> list[str]`, and `validate_no_weapon(gltf: dict) -> list[str]`.
- Consumes no Blender module and no licensed fixture. Tests construct a minimal valid GLB JSON/BIN dictionary in memory.

- [ ] **Step 1: Write failing Root/clip/parity tests.**

```python
class PromotionValidationTests(unittest.TestCase):
    def test_root_requires_exact_wrapper_and_identity_assembly_child(self):
        gltf = minimal_gltf(assembly="SK_Character_Mystic_01")
        self.assertEqual(v.validate_root(gltf, "SK_Character_Mystic_01"), [])
        gltf["nodes"][0]["scale"] = [0.01, 0.01, 0.01]
        self.assertIn("Root scale", v.validate_root(gltf, "SK_Character_Mystic_01")[0])

    def test_variants_require_same_clip_names_and_keyframe_counts(self):
        with tempfile.TemporaryDirectory() as tmp:
            paths = write_variant_glbs(tmp, {"a": 31, "b": 31, "c": 30, "d": 31})
            self.assertEqual(
                v.validate_variant_parity(paths, ["Idle_Relaxed", "Idle_Meditative", "Idle_Drinking", "Walk_Forward"]),
                ["monk c: Walk_Forward keyframe count 30 != a count 31"],
            )
```

- [ ] **Step 2: Run the test to verify it fails.**

Run: `python3 scripts/test_character_promotion_validation.py`

Expected: FAIL with `ModuleNotFoundError: No module named 'character_promotion_validation'`.

- [ ] **Step 3: Implement the minimal stdlib GLB parser and exact validators.**

```python
ROOT_SCALE = [0.009999999776482582] * 3
ROOT_ROTATION = [0.70710688829422, 0, 0, 0.7071066498756409]

def validate_clip_contract(gltf, expected):
    actual = [animation.get("name") for animation in gltf.get("animations", [])]
    return [] if actual == expected else [f"clip names {actual!r} != expected {expected!r}"]

def validate_root(gltf, assembly):
    nodes = gltf["nodes"]
    roots = [node for node in nodes if node.get("name") == "Root"]
    if len(roots) != 1:
        return [f"expected exactly one Root node, found {len(roots)}"]
    root = roots[0]
    errors = []
    if root.get("scale") != ROOT_SCALE:
        errors.append(f"Root scale {root.get('scale')!r} != {ROOT_SCALE!r}")
    if root.get("rotation") != ROOT_ROTATION:
        errors.append(f"Root rotation {root.get('rotation')!r} != {ROOT_ROTATION!r}")
    # Resolve the one child index and require its identity local TRS/name.
    return errors + validate_root_child(nodes, root, assembly)
```

Implement `read_glb` and accessor decoding by extracting the existing zero-dependency logic from `animation_qa.py` into this new module only; do not alter its established public functions. `validate_no_weapon` must reject a mesh/node/material name containing case-insensitive `weapon`, `wep`, `sword`, `axe`, `staff`, `bow`, `dagger`, `shield`, or `quiver`, and reject an extra mesh/body assembly beyond the configured child ownership.

- [ ] **Step 4: Run the focused tests.**

Run: `python3 scripts/test_character_promotion_validation.py`

Expected: PASS with all Root, exact-clip, keyframe-parity, and weapon-name rejection tests green.

- [ ] **Step 5: Commit the contract foundation.**

```bash
git add scripts/configs/character-promotion/unarmed-checkpoints-v1.json scripts/character_promotion_validation.py scripts/test_character_promotion_validation.py
```

### Task 2: Add Constrained-Diff and Preservation Validators

**Files:**
- Modify: `scripts/character_promotion_validation.py`
- Modify: `scripts/test_character_promotion_validation.py`

**Interfaces:**
- Produces `canonicalize_non_animation(gltf: dict) -> dict`, `validate_allowed_a_diff(baseline: str, staged: str) -> list[str]`, and `validate_preserved_actions(baseline: str, staged: str, names: list[str]) -> list[str]`.
- `validate_allowed_a_diff` permits removal only of weapon nodes/meshes/materials/textures/images and changes below `animations`; all Root, non-weapon nodes, mesh primitive topology/accessors, skins, scene ownership, and body material references must match.

- [ ] **Step 1: Write failing constrained-diff tests.**

```python
def test_allowed_diff_accepts_only_weapon_removal_and_animation_payload_change(self):
    baseline, staged = write_pair_with_weapon_removed_and_new_walk(self.tmp)
    self.assertEqual(v.validate_allowed_a_diff(baseline, staged), [])

def test_allowed_diff_rejects_body_mesh_change(self):
    baseline, staged = write_pair_with_weapon_removed_and_new_walk(self.tmp)
    staged_json = v.read_glb(staged)[0]
    staged_json["meshes"][0]["name"] = "ChangedBody"
    rewrite_glb_json(staged, staged_json)
    self.assertEqual(v.validate_allowed_a_diff(baseline, staged), ["non-weapon mesh changed: ChangedBody"])

def test_preserved_actions_require_byte_equal_sampler_accessors(self):
    baseline, staged = write_pair_with_preserved_idle_changed(self.tmp)
    self.assertEqual(v.validate_preserved_actions(baseline, staged, ["Idle_Meditative", "Idle_Drinking"]), ["Idle_Drinking animation payload changed"])
```

- [ ] **Step 2: Run the focused tests to verify red.**

Run: `python3 scripts/test_character_promotion_validation.py PromotionValidationTests.test_allowed_diff_accepts_only_weapon_removal_and_animation_payload_change`

Expected: FAIL with `AttributeError: module 'character_promotion_validation' has no attribute 'validate_allowed_a_diff'`.

- [ ] **Step 3: Implement the comparison with stable object normalization.**

```python
def validate_preserved_actions(baseline_path, staged_path, names):
    before, before_bin = read_glb(baseline_path)
    after, after_bin = read_glb(staged_path)
    errors = []
    for name in names:
        if animation_payload(before, before_bin, name) != animation_payload(after, after_bin, name):
            errors.append(f"{name} animation payload changed")
    return errors
```

Normalize node-indexed references after dropping explicitly identified weapon subtrees, then compare JSON values recursively. Compare preserved animation samplers by decoded time/value tuples, not raw buffer offsets, so exporter buffer layout cannot create a false failure. Require the staged `Idle_Relaxed` and `Walk_Forward` payloads to differ from the baseline when a same-name baseline exists and to exist exactly once.

- [ ] **Step 4: Run all validation tests.**

Run: `python3 scripts/test_character_promotion_validation.py`

Expected: PASS; no test depends on a real Synty file.

- [ ] **Step 5: Commit the semantic-diff gate.**

```bash
git add scripts/character_promotion_validation.py scripts/test_character_promotion_validation.py
```

### Task 3: Add the Focused Checkpoint Export Helper

**Files:**
- Create: `scripts/export_checkpoint_character.py`
- Modify: `scripts/test_character_promotion_validation.py`

**Interfaces:**
- CLI: `blender -b CHECKPOINT.blend -P scripts/export_checkpoint_character.py -- --out PATH --clip Idle_Relaxed --clip Walk_Forward`.
- Produces one unarmed staged A GLB containing exactly the requested checkpoint actions. It fails before export unless there is one armature/body assembly, no weapon-like mesh/node, and every requested action exists exactly once.
- Reuses `retarget_locomotion_common.drop_stale_action`/the existing glTF exporter settings conceptually: no new retarget math, no source FBX import, and `export_anim_single_armature=True` exports only explicitly retained actions.

- [ ] **Step 1: Add a failing config-level test for the required checkpoint action contract.**

```python
def test_config_requests_only_checkpoint_owned_actions(self):
    config = v.load_config("scripts/configs/character-promotion/unarmed-checkpoints-v1.json")
    for entry in config["classes"].values():
        self.assertEqual(entry["checkpointClips"], ["Idle_Relaxed", "Walk_Forward"])
```

- [ ] **Step 2: Run it to verify red.**

Run: `python3 scripts/test_character_promotion_validation.py PromotionValidationTests.test_config_requests_only_checkpoint_owned_actions`

Expected: FAIL because `checkpointClips` is absent from the initial configuration.

- [ ] **Step 3: Add `checkpointClips` and implement Blender export.**

```python
requested = set(args.clip)
actions = {action.name: action for action in bpy.data.actions}
missing = sorted(requested - actions.keys())
if missing:
    raise SystemExit(f"checkpoint missing requested actions: {missing}; available: {sorted(actions)}")
for action in list(bpy.data.actions):
    if action.name not in requested:
        bpy.data.actions.remove(action, do_unlink=True)
bpy.ops.export_scene.gltf(
    filepath=args.out, export_format="GLB", export_yup=True, export_apply=False,
    export_animations=True, export_anim_single_armature=True, export_image_format="AUTO",
)
```

Before this block, remove Blender-importer `Icosphere` artifacts, locate exactly one armature, and invoke local weapon-name checks over objects/materials. Do not change Root transforms or retarget any action. Update the config schema so every class explicitly declares `"checkpointClips": ["Idle_Relaxed", "Walk_Forward"]`.

- [ ] **Step 4: Run tests and a non-writing Blender argument check.**

Run: `python3 scripts/test_character_promotion_validation.py && blender -b /home/kirk/game-dev/assets/synty/animation-retarget/approved/monk-unarmed-idle-walk-approved.blend -P scripts/export_checkpoint_character.py -- --help`

Expected: Python tests PASS; Blender prints `--out` and repeatable `--clip` help, exits 0.

- [ ] **Step 5: Commit the checkpoint exporter.**

```bash
git add scripts/export_checkpoint_character.py scripts/configs/character-promotion/unarmed-checkpoints-v1.json scripts/test_character_promotion_validation.py
```

### Task 4: Make Animation QA Addressable for Staged Releases

**Files:**
- Modify: `scripts/animation_qa.py`
- Modify: `scripts/test_animation_qa.py`

**Interfaces:**
- Adds `--paths PATH [PATH ...]`; when supplied, `build_report` analyzes exactly those files and reports paths relative to the common parent. Existing `--harness-dir`, discovery, reports, threshold semantics, and foot-slide informational status stay unchanged.

- [ ] **Step 1: Write the failing `--paths` test.**

```python
class ExplicitPathTests(unittest.TestCase):
    def test_explicit_paths_skip_catalog_discovery(self):
        with mock.patch.object(qa, "analyze_file", return_value=[]) as analyze:
            report = qa.build_report("ignored", paths=["/tmp/stage/monk.glb"])
        self.assertEqual(report["summary"]["filesWithErrors"], 0)
        analyze.assert_called_once_with("/tmp/stage/monk.glb", "monk.glb")
```

- [ ] **Step 2: Run it to verify red.**

Run: `python3 scripts/test_animation_qa.py ExplicitPathTests.test_explicit_paths_skip_catalog_discovery`

Expected: FAIL with `TypeError: build_report() got an unexpected keyword argument 'paths'`.

- [ ] **Step 3: Implement the narrow optional path flow.**

```python
def build_report(harness_dir, include_loose=False, only=None, paths=None):
    glbs = sorted(paths) if paths is not None else discover_glbs(harness_dir, include_loose=include_loose)
    common_parent = os.path.commonpath(glbs) if paths else harness_dir
    clips, errors = [], []
    for path in glbs:
        rel = os.path.relpath(path, common_parent)
        try:
            clips.extend(analyze_file(path, rel))
        except Exception as exc:
            errors.append({"file": rel, "error": f"{type(exc).__name__}: {exc}"})
```

Add `parser.add_argument("--paths", nargs="+")` and pass `args.paths`; reject simultaneous `--paths` and `--only` with `parser.error` to keep filtering unambiguous.

- [ ] **Step 4: Run regression coverage.**

Run: `python3 scripts/test_animation_qa.py`

Expected: PASS, including the pre-existing exact-once-scale and informational-foot-slide tests.

- [ ] **Step 5: Commit the staging QA interface.**

```bash
git add scripts/animation_qa.py scripts/test_animation_qa.py
```

### Task 5: Implement the Reusable Staging and Validation CLI

**Files:**
- Create: `scripts/promote_character_checkpoints.py`
- Create: `scripts/test_promote_character_checkpoints.py`
- Modify: `scripts/character_promotion_validation.py`

**Interfaces:**
- `stage(config) -> pathlib.Path` writes `baseline/`, `release/harness/models/synty/characters/`, `release/harness/animation-qa/`, `release/README.md`, and `release-manifest.json`; no canonical file is opened for writing.
- `validate(config, release) -> dict` writes `validation.json` with `status: "PASS"` only after every contract gate passes.
- `promote(config, release) -> None` requires a passing validation report with release hashes and writes `transaction.json`; `rollback(transaction) -> None` restores the exact recorded inventory.

- [ ] **Step 1: Write failing stage/promotion guard tests.**

```python
class PromotionCliTests(unittest.TestCase):
    def test_stage_never_targets_canonical_outputs(self):
        with mock.patch("subprocess.run") as run:
            release = cli.stage(self.config)
        for call in run.call_args_list:
            self.assertNotIn("harness/models/synty/characters/monk.glb", " ".join(call.args[0]))
        self.assertTrue((release / "harness/models/synty/characters/monk.glb").exists())

    def test_promote_refuses_missing_or_failed_validation(self):
        with self.assertRaisesRegex(RuntimeError, "validation status must be PASS"):
            cli.promote(self.config, self.release)
```

- [ ] **Step 2: Run the focused tests to verify red.**

Run: `python3 scripts/test_promote_character_checkpoints.py`

Expected: FAIL with `ModuleNotFoundError: No module named 'promote_character_checkpoints'`.

- [ ] **Step 3: Implement config loading, staging, and the complete-set validator.**

```python
def stage(config):
    root = Path(config["stagingRoot"])
    baseline, release = root / "baseline", root / "release"
    copy_release_inventory(config, baseline, source_root=Path(config["canonicalRoot"]))
    for class_name, entry in config["classes"].items():
        a_out = release / "harness/models/synty" / entry["canonical"]
        run_blender_export(entry["checkpoint"], a_out, entry["checkpointClips"])
        merge_preserved_actions(baseline / entry["canonical"], a_out, entry)
        run_atlas_variants(a_out, entry, release)
    write_release_manifest(release, config)
    return release
```

`merge_preserved_actions` must import the checkpoint A output plus the baseline A output into Blender, retain checkpoint `Idle_Relaxed`/`Walk_Forward`, copy the two named baseline actions unchanged, remove all other actions, and export exactly four named actions. Reuse the existing `retarget_locomotion_common` conventions for stale/NLA cleanup and glTF export settings; do not call either retarget wrapper because the approved clips already exist in the checkpoint. `run_atlas_variants` calls existing `swap_atlas.py` for B/C/D using the configured absolute atlas files and writes only beneath `release`.

`validate` must validate every class/color, the exact manifest `idleClips` order, `bakedIntoModel is false`, unchanged standalone weapon paths, A constrained diff/preserved actions, A/B/C/D keyframe parity, all Root/assembly/no-weapon rules, complete release inventory, and report hashes. It runs `python3 scripts/build_mesh_stats.py` against a temporary copied harness rooted in `release/harness`, compares changed standing entries against `baseline/mesh-stats.json` for new `warnings`, runs `python3 scripts/animation_qa.py --paths <all 16 staged GLBs> --report ...`, and fails if errors, WARN verdicts, missing clips, or a non-informational gate failure appears.

- [ ] **Step 4: Run tests to verify green.**

Run: `python3 scripts/test_promote_character_checkpoints.py && python3 scripts/test_character_promotion_validation.py`

Expected: PASS. The tests demonstrate `stage` cannot write canonical paths and `promote` cannot run before a passing report.

- [ ] **Step 5: Commit the staged workflow.**

```bash
git add scripts/promote_character_checkpoints.py scripts/test_promote_character_checkpoints.py scripts/character_promotion_validation.py
```

### Task 6: Add Complete-Set Transactional Promotion and Documentation Contract

**Files:**
- Modify: `scripts/promote_character_checkpoints.py`
- Modify: `scripts/test_promote_character_checkpoints.py`
- Modify: `README.md`
- Modify: `harness/models/synty/characters/manifest.json`

**Interfaces:**
- `release_inventory(config) -> list[PurePosixPath]` enumerates all 16 standing GLBs, manifest, README, mesh stats, animation reports, evidence index, and only those portraits selected by portrait review.
- `promote` creates `transaction.json` before copying, verifies all staged hashes, uses same-directory `.promotion-tmp-<uuid>` files plus `os.replace`, and automatically invokes `restore_transaction` on any error.

- [ ] **Step 1: Write failing transaction and manifest tests.**

```python
def test_failed_mid_promotion_restores_every_prior_target(self):
    before = snapshot_tree(self.canonical)
    with mock.patch("promote_character_checkpoints.os.replace", side_effect=[None, OSError("disk full")]):
        with self.assertRaises(OSError):
            cli.promote(self.config, self.release)
    self.assertEqual(snapshot_tree(self.canonical), before)

def test_manifest_contract_marks_only_runtime_models_unarmed(self):
    manifest = json.loads((self.release / "harness/models/synty/characters/manifest.json").read_text())
    for entry in manifest["mapping"].values():
        self.assertFalse(entry["weapon"]["bakedIntoModel"])
        self.assertTrue(entry["weapon"]["file"].endswith("-weapon.glb"))
```

- [ ] **Step 2: Run it to verify red.**

Run: `python3 scripts/test_promote_character_checkpoints.py PromotionCliTests.test_failed_mid_promotion_restores_every_prior_target`

Expected: FAIL because `promote` has not yet written/restored a transaction.

- [ ] **Step 3: Implement all-or-nothing operational behavior and update staged docs/metadata.**

```python
def promote(config, release):
    validation = load_passing_validation(release)
    transaction = snapshot_transaction(config, release, validation)
    write_json(transaction.path, transaction.to_dict())
    replaced = []
    try:
        for target in transaction.targets:
            atomic_replace_from_staged(target)
            replaced.append(target)
    except Exception:
        restore_transaction(transaction, targets=replaced)
        raise
```

Copy and edit `manifest.json` only in `release`: set all four `weapon.bakedIntoModel` fields to `false`, replace the armed/baked-weapon claims in `_weaponComment` and `_colorsComment`, preserve the `weapon.file` and socket blocks, and preserve every ordered three-item `idleClips`. Update `README.md` to say standing A/B/C/D models are intentionally unarmed, color swaps re-color body-only models, standalone weapons remain future equipment assets, and show the config-driven command. Do not modify downed GLBs. The implementation must name every target in `transaction.json`, including `README.md`, metadata, generated reports, evidence index, and conditionally regenerated portraits, so rollback cannot leave mixed documentation/reports/artifacts.

- [ ] **Step 4: Run all unit tests.**

Run: `python3 scripts/test_promote_character_checkpoints.py && python3 scripts/test_character_promotion_validation.py && python3 scripts/test_animation_qa.py`

Expected: PASS. The injected `os.replace` failure test proves every previously replaced target is restored.

- [ ] **Step 5: Commit the transactional promotion contract.**

```bash
git add scripts/promote_character_checkpoints.py scripts/test_promote_character_checkpoints.py README.md harness/models/synty/characters/manifest.json
```

### Task 7: Execute the Approved Four-Class Batch and Complete Asset Gates

**Files:**
- Modify/generated: all paths in the File Map's `Modify in rpg-game-assets` section that are selected by the staged release.
- Create/generated: private staging below `tmp/character-promotion/unarmed-character-animation-promotion-v1/` (never commit this directory).
- Create external evidence-only files: `rpg-dnd5e-web` `evidence/asset-pipeline-wave1:playtest-evidence/asset-pipeline/unarmed-character-animation-promotion/`.

**Interfaces:**
- Consumes the fully tested CLI/config from Tasks 1-6.
- Produces a PASS `validation.json`, final generated mesh/animation reports, unarmed A/B/C/D GLBs, reviewed portrait result, multi-angle proof, and a complete transaction record.

- [ ] **Step 1: Open implementation tracking before executing.**

Create exactly one `rpg-game-assets` issue and Board 19 item with Team `Assets`, then branch `asset/119-unarmed-character-animation-promotion`; the issue and PR body must reference `rpg-project#119` and `rpg-project#120`. Do not reuse this design PR as the implementation PR.

- [ ] **Step 2: Run all workflow regression tests before touching a checkpoint.**

Run: `python3 scripts/test_character_promotion_validation.py && python3 scripts/test_promote_character_checkpoints.py && python3 scripts/test_animation_qa.py`

Expected: PASS.

- [ ] **Step 3: Stage all four classes, never a single canonical file.**

Run: `python3 scripts/promote_character_checkpoints.py stage --config scripts/configs/character-promotion/unarmed-checkpoints-v1.json`

Expected: exit 0; `tmp/character-promotion/unarmed-character-animation-promotion-v1/release/harness/models/synty/characters/` contains exactly `fighter{,-b,-c,-d}.glb`, `barbarian{,-b,-c,-d}.glb`, `monk{,-b,-c,-d}.glb`, and `rogue{,-b,-c,-d}.glb`; `git diff -- harness/models/synty/characters` is empty.

- [ ] **Step 4: Generate and inspect portraits before promotion.**

Create a 4-by-4 contact sheet from current portrait PNGs with class/color labels. An independent reviewer must view it and record: `I viewed the four-class portrait contact sheet; no weapon or weapon fragment is visible in any frame.` If any portrait fails, use `personal/assets/synty/pipeline/portrait_render.py` only against the staged A/B/C/D GLBs for that class, place regenerated PNGs under the release inventory, recreate the sheet, and obtain a new recorded passing statement. Never publish the GLBs or source assets with the sheet.

- [ ] **Step 5: Run structural, clip, parity, diff, mesh, and animation gates.**

Run: `python3 scripts/promote_character_checkpoints.py validate --config scripts/configs/character-promotion/unarmed-checkpoints-v1.json --release tmp/character-promotion/unarmed-character-animation-promotion-v1/release`

Expected: exit 0 and `validation.json` reports all 16 GLBs with exact Root/assembly values, exactly four expected clips, no weapon, preserved non-relaxed idle payloads, A/B/C/D clip/keyframe parity, and no unauthorized A diff; `animation-qa` has zero errors and every clip verdict is `PASS`; mesh stats has no new warning on the 16 changed standing models relative to baseline.

- [ ] **Step 6: Produce multi-angle anatomical evidence from staged models.**

For every class and each of `Idle_Relaxed` and `Walk_Forward`, invoke `scripts/animation_qa_render.py` at camera angles `0`, `45`, `90`, and `180`, including wrap frames, then invoke `scripts/render_animation_evidence.mjs` for the strips. Review front, side, three-quarter, and back evidence for weapon remnants, skinning, clipping, pose defects, and loop seam. Commit only PNG/GIF screenshots and README/viewed statements to the public evidence branch; no raw GLB, FBX, blend, atlas, or private report leaves `rpg-game-assets`.

- [ ] **Step 7: Obtain the independent asset gate.**

An agent other than the implementer, in a separate worktree, reruns the three Python test commands, runs `validate` from scratch against the staged release, independently views portrait and multi-angle evidence, checks transaction inventory/rollback code, and posts a `GATE REVIEW` verdict with findings or `MERGE-READY`. The reviewer must explicitly state that the four-class complete set, no-weapon contract, Root/clip/parity checks, mesh/animation gates, and evidence were independently verified.

- [ ] **Step 8: Promote only the complete passing release.**

Run: `python3 scripts/promote_character_checkpoints.py promote --config scripts/configs/character-promotion/unarmed-checkpoints-v1.json --release tmp/character-promotion/unarmed-character-animation-promotion-v1/release`

Expected: exit 0; `transaction.json` lists all staged/promoted artifacts and SHA-256s; no canonical subset was replaced. If this exits nonzero, inspect retained staging, verify canonicals match baseline hashes, repair the workflow/input, and restart from stage; do not hand-copy any output.

- [ ] **Step 9: Regenerate final reports and rerun authoritative asset checks.**

Run: `python3 scripts/build_mesh_stats.py && python3 scripts/animation_qa.py --report harness/animation-qa/report.json --summary > harness/animation-qa/summary.txt && python3 scripts/test_character_promotion_validation.py && python3 scripts/test_promote_character_checkpoints.py && python3 scripts/test_animation_qa.py`

Expected: each command exits 0; final report records all changed canonical A/B/C/D models, has no new mesh warning for them, and has PASS hard animation gates.

- [ ] **Step 10: Commit the complete private asset release.**

```bash
git add README.md scripts harness/models/synty/characters/manifest.json harness/models/synty/characters/{fighter,barbarian,monk,rogue}.glb harness/models/synty/characters/{fighter,barbarian,monk,rogue}-{b,c,d}.glb harness/models/synty/mesh-stats.json harness/animation-qa
git add harness/models/synty/characters/portraits  # only if the contact-sheet gate regenerated a portrait
git commit -m "asset: promote unarmed character animation checkpoints"
```

Before committing, inspect `git status --short`, `git diff --cached --stat`, and `git log --oneline -10`; stage no source/checkpoint/staging files. This is the single complete-set promotion commit.

### Task 8: Sync and Verify the Actual Client Without a Web Code Change

**Files:**
- No `rpg-dnd5e-web` product files modified.
- Evidence-only: update the public evidence branch README with commands, client routes, screenshots, and viewed statements.

**Interfaces:**
- Consumes the merged/private `rpg-game-assets` release via existing `npm run assets:sync`.
- Verifies `ClassCharacterModel` resolves the unchanged `characters/<class>.glb` paths and plays idle-to-walk-to-idle from embedded clips.

- [ ] **Step 1: Sync private assets into the web checkout.**

Run in `/home/kirk/game-dev/rpg-dnd5e-web`: `npm run assets:sync`

Expected: exits 0 and prints `public/models/synty/ now mirrors rpg-game-assets:harness/models/synty/`.

- [ ] **Step 2: Run web automated checks.**

Run in `/home/kirk/game-dev/rpg-dnd5e-web`: `npm run test:run && npm run ci-check`

Expected: both exit 0. Do not edit resolver/client code to make this pass.

- [ ] **Step 3: Verify the actual client route.**

Start the normal local stack and Vite client, create or use real Fighter, Barbarian, Monk, and Rogue characters, enter the real Home → character select → Play → lobby → Start → `EncounterView` route, and trigger movement for each. Capture before-move idle, in-motion `Walk_Forward`, and post-arrival idle frames. Confirm the canonical path resolves, transitions occur without a weapon, no console/page errors occur, and the client returns to an idle clip after movement. Record this in the evidence README with a viewed statement.

- [ ] **Step 4: Handle a consumer defect correctly.**

If sync, tests, or actual-client verification reveal a web defect, stop this promotion scope, file one separate `rpg-dnd5e-web` issue with reproduction/evidence, and do not make a web tracked change here. If no defect exists, record that no web product change was made.

- [ ] **Step 5: Push and open/update the implementation PR.**

Run in `rpg-game-assets`: `git push -u origin asset/119-unarmed-character-animation-promotion`

Expected: normal push succeeds. Open one ready-for-review private PR referencing the single implementation issue, `rpg-project#119`, and `rpg-project#120`; include load-bearing workflow/paths, complete-set transaction explanation, mesh/animation output, public evidence links and viewed statements, client verification, and the independent gate comment. End every GitHub comment with `— asset-pipeline agent, on behalf of KirkDiggler`.

## Final Review Checklist

- [ ] The PR changes only private asset workflow/config/generated assets/docs plus public screenshot-only evidence; it contains no Synty source, blend, FBX, atlas, or GLB outside private `rpg-game-assets`.
- [ ] The PR contains exactly one complete canonical promotion set, never a per-class promotion, and standalone weapon GLBs remain untouched.
- [ ] `bakedIntoModel` is `false` for all four classes; top-level/color model paths and downed assets stay unchanged.
- [ ] Every generated report, manifest statement, portrait decision, and evidence link is consistent with intentionally unarmed standing models.
- [ ] Tests and gates have fresh command output, an independent gate review, and actual-client idle-to-walk-to-idle evidence.
- [ ] Future use requires only a new versioned config/input and CLI execution; a design update is required only for a runtime/promotion-contract change.
