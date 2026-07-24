# Unarmed Character Animation Promotion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a reusable, config-driven private `rpg-game-assets` workflow that validates and publishes a complete unarmed four-class A/B/C/D character release in one Git commit.

**Architecture:** Work is built and validated only in an isolated `rpg-game-assets` worktree rooted at a recorded baseline SHA. Blender helpers export checkpoints and transfer only configured baseline idles using Blender 5 action slots; pure-Python validators and existing QA tools gate the staged harness. The one release commit is the sole publication transaction, and a later full `git revert` of that commit is the sole rollback mechanism.

**Tech Stack:** Python 3 stdlib, Blender 5 `bpy`, existing `swap_atlas.py`, `animation_qa.py`, `build_mesh_stats.py`, GLB JSON/BIN parsing, Git, Node `sharp`/`omggif`, and existing web asset-sync/client checks.

## Global Constraints

- Implementation happens later in one private `rpg-game-assets` issue, Board 19 Team `Assets` item, branch, and PR; all reference `rpg-project#119` and design PR `rpg-project#120`.
- The isolated implementation worktree starts at an explicitly recorded `baselineCommit`; nothing is synced, pushed, or consumed before the one complete release commit is created.
- A pre-commit crash/failure means discard and recreate the isolated worktree at `baselineCommit`; never attempt recovery from temporary backups or per-file replacement.
- A post-publication regression uses a new issue/PR containing `git revert <releaseCommit>`; that one new commit restores every tracked release artifact, then asset sync and actual-client verification are rerun.
- Do not claim filesystem-level multi-file atomicity or use `os.replace` for promotion/rollback.
- Versioned config contains logical identifiers only. Required machine inputs are `SYNTY_APPROVED_CHECKPOINT_DIR` and `SYNTY_FANTASY_RIVALS_TEXTURE_DIR`, or equivalent explicit CLI options; validate both before Blender starts.
- Checkpoint identifiers are `fighter-unarmed-idle-walk-approved.blend`, `barbarian-unarmed-idle-walk-approved.blend`, `monk-unarmed-idle-walk-approved.blend`, and `rogue-unarmed-idle-walk-approved.blend`.
- Exact clips: fighter `Idle_Relaxed`, `Idle_Stretch`, `Idle_Drinking`, `Walk_Forward`; barbarian `Idle_Relaxed`, `Idle_ChinScratch`, `Idle_Drinking`, `Walk_Forward`; monk `Idle_Relaxed`, `Idle_Meditative`, `Idle_Drinking`, `Walk_Forward`; rogue `Idle_Relaxed`, `Idle_CheckWatch`, `Idle_Drinking`, `Walk_Forward`.
- Checkpoint-owned clips are only `Idle_Relaxed` and `Walk_Forward`; retain only each class's distinctive idle and `Idle_Drinking` from canonical A, with unchanged decoded animation payloads.
- `idleClips` remains ordered `[Idle_Relaxed, distinctive, Idle_Drinking]`; `Walk_Forward` is embedded but not idle metadata. `weapon.bakedIntoModel` becomes `false`; standalone weapon files and metadata remain untouched.
- Every A/B/C/D output has Root name `Root`, Float32 scale `[0.009999999776482582, 0.009999999776482582, 0.009999999776482582]`, glTF `[x, y, z, w]` rotation `[0.70710688829422, 0, 0, 0.7071066498756409]`, and exactly one identity-local-transform child. Expected assemblies are fighter `SK_BR_Character_Slayer_01`, barbarian `SK_BR_Character_BarbarianGiant_01`, monk `SK_Character_Mystic_01`, rogue `SK_Character_DarkElf_01`.
- Pre-export Blender validation requires exactly one target armature and one configured body mesh object, no other mesh object. Post-export GLB validation requires exactly one mesh-bearing node/mesh with the configured body name and captured topology/material stats; names supplement, never prove, no-weapon status.
- `PROPOSED_BUDGETS` and hard `animation_qa` gates are authoritative. Foot-slide is informational; the workflow parses changed-path report verdicts and cannot treat exit code 0 as proof.
- Public screenshot evidence is only `rpg-dnd5e-web` `evidence/asset-pipeline-wave1:playtest-evidence/asset-pipeline/unarmed-character-animation-promotion/`; it is external review evidence, not a release-inventory item or rollback target. Never publish Synty GLB, blend, FBX, or atlas files.
- No `rpg-dnd5e-web` product change is in scope. A consumer finding gets a separate issue.

---

## File Map

### Create in `rpg-game-assets`

- `scripts/configs/character-promotion/unarmed-checkpoints-v1.json` - portable batch contract with class data, logical checkpoint/atlas filenames, clips, body names, and Root assembly names.
- `scripts/character_action_transfer.py` - Blender 5 action-slot helpers and CLI that opens a checkpoint, imports canonical A, copies the two retained actions onto the checkpoint armature, and exports four clips.
- `scripts/export_checkpoint_character.py` - Blender checkpoint exporter that captures body stats and enforces the one-armature/one-mesh scene contract.
- `scripts/character_promotion_validation.py` - pure-Python GLB/config/report validators.
- `scripts/promote_character_checkpoints.py` - stage/validate/release-metadata CLI; it does not copy into canonical paths or perform rollback.
- `scripts/test_character_promotion_validation.py` - stdlib unit fixtures for config, Root, mesh-bearing-node, animation payload, and report parsing.
- `scripts/test_character_action_transfer.py` - real Blender subprocess integration smoke test using one checkpoint/canonical pair plus helper-level tests.
- `scripts/test_promote_character_checkpoints.py` - temporary real Git repository integration test for one release commit and durable `git revert` rollback.

### Modify in `rpg-game-assets`

- `scripts/build_mesh_stats.py` - backward-compatible staged root/output arguments and tests in new `scripts/test_build_mesh_stats.py`.
- `scripts/animation_qa.py` and `scripts/test_animation_qa.py` - explicit `--input-root`/`--paths`/`--report` behavior with no inferred common path.
- `README.md`, `harness/models/synty/characters/manifest.json`, all 16 standing class GLBs, conditionally regenerated portraits, `harness/models/synty/mesh-stats.json`, `harness/animation-qa/report.json`, and `harness/animation-qa/summary.txt` - only in the one final release commit.

## Portable Config And CLI

The config has no machine path:

```json
{
  "schemaVersion": 1,
  "batchId": "unarmed-character-animation-promotion-v1",
  "classes": {
    "fighter": {"checkpoint": "fighter-unarmed-idle-walk-approved.blend", "canonical": "characters/fighter.glb", "body": "SK_BR_Character_Slayer_01", "assembly": "SK_BR_Character_Slayer_01", "retainedClips": ["Idle_Stretch", "Idle_Drinking"], "idleClips": ["Idle_Relaxed", "Idle_Stretch", "Idle_Drinking"], "clips": ["Idle_Relaxed", "Idle_Stretch", "Idle_Drinking", "Walk_Forward"]},
    "barbarian": {"checkpoint": "barbarian-unarmed-idle-walk-approved.blend", "canonical": "characters/barbarian.glb", "body": "SK_BR_Character_BarbarianGiant_01", "assembly": "SK_BR_Character_BarbarianGiant_01", "retainedClips": ["Idle_ChinScratch", "Idle_Drinking"], "idleClips": ["Idle_Relaxed", "Idle_ChinScratch", "Idle_Drinking"], "clips": ["Idle_Relaxed", "Idle_ChinScratch", "Idle_Drinking", "Walk_Forward"]},
    "monk": {"checkpoint": "monk-unarmed-idle-walk-approved.blend", "canonical": "characters/monk.glb", "body": "SK_Character_Mystic_01", "assembly": "SK_Character_Mystic_01", "retainedClips": ["Idle_Meditative", "Idle_Drinking"], "idleClips": ["Idle_Relaxed", "Idle_Meditative", "Idle_Drinking"], "clips": ["Idle_Relaxed", "Idle_Meditative", "Idle_Drinking", "Walk_Forward"]},
    "rogue": {"checkpoint": "rogue-unarmed-idle-walk-approved.blend", "canonical": "characters/rogue.glb", "body": "SK_Character_DarkElf_01", "assembly": "SK_Character_DarkElf_01", "retainedClips": ["Idle_CheckWatch", "Idle_Drinking"], "idleClips": ["Idle_Relaxed", "Idle_CheckWatch", "Idle_Drinking"], "clips": ["Idle_Relaxed", "Idle_CheckWatch", "Idle_Drinking", "Walk_Forward"]}
  },
  "atlases": {"b": "FantasyRivals_Texture_01_B.png", "c": "FantasyRivals_Texture_01_C.png", "d": "FantasyRivals_Texture_01_D.png"}
}
```

```bash
export SYNTY_APPROVED_CHECKPOINT_DIR=/home/kirk/game-dev/assets/synty/animation-retarget/approved
export SYNTY_FANTASY_RIVALS_TEXTURE_DIR=/home/kirk/game-dev/assets/synty/polygon-fantasy-rivals/PolygonFantasyRivals_Source_Files/Source_Files/Textures
python3 scripts/promote_character_checkpoints.py stage --config scripts/configs/character-promotion/unarmed-checkpoints-v1.json --worktree-root "$PWD" --checkpoint-dir "$SYNTY_APPROVED_CHECKPOINT_DIR" --atlas-dir "$SYNTY_FANTASY_RIVALS_TEXTURE_DIR" --stage-dir tmp/unarmed-v1
python3 scripts/promote_character_checkpoints.py validate --config scripts/configs/character-promotion/unarmed-checkpoints-v1.json --stage-dir tmp/unarmed-v1 --baseline-commit "$(git rev-parse HEAD)"
```

`stage` rejects a missing/non-directory input root, a missing checkpoint, or a missing B/C/D atlas before creating output. It writes only under `--stage-dir`. `validate` writes `release-metadata.json` with `baselineCommit`, config SHA-256, staged artifact SHA-256s, and QA-report hashes. The implementing engineer stages all listed tracked release files in the isolated worktree and creates one release commit; the CLI never executes `git commit`, push, sync, or rollback.

## Tasks

### Task 1: Portable Contract and Structural Validators

**Files:**
- Create: `scripts/configs/character-promotion/unarmed-checkpoints-v1.json`
- Create: `scripts/character_promotion_validation.py`
- Create: `scripts/test_character_promotion_validation.py`

**Interfaces:** `load_config(path) -> dict`, `resolve_inputs(config, checkpoint_dir, atlas_dir) -> dict`, `validate_root(gltf, assembly) -> list[str]`, `validate_mesh_contract(gltf, body, stats) -> list[str]`, and `validate_clip_contract(gltf, expected) -> list[str]`.

- [ ] **Step 1: Write failing portable-input and mesh-contract tests.**

```python
def test_resolve_inputs_rejects_missing_logical_checkpoint(self):
    with tempfile.TemporaryDirectory() as root:
        with self.assertRaisesRegex(ValueError, "fighter.*checkpoint"):
            v.resolve_inputs(CONFIG, root, root)

def test_mesh_contract_rejects_second_mesh_bearing_node(self):
    gltf = minimal_gltf(body="SK_Character_Mystic_01")
    gltf["nodes"].append({"name": "hidden", "mesh": 0})
    self.assertEqual(v.validate_mesh_contract(gltf, "SK_Character_Mystic_01", BODY_STATS), ["expected one mesh-bearing node, found 2"])
```

- [ ] **Step 2: Verify red.**

Run: `python3 scripts/test_character_promotion_validation.py`

Expected: FAIL with `ModuleNotFoundError: No module named 'character_promotion_validation'`.

- [ ] **Step 3: Implement config/input and GLB checks.**

```python
def resolve_inputs(config, checkpoint_dir, atlas_dir):
    roots = {"checkpoint": Path(checkpoint_dir), "atlas": Path(atlas_dir)}
    if not all(path.is_dir() for path in roots.values()):
        raise ValueError("--checkpoint-dir and --atlas-dir must name existing directories")
    for name, entry in config["classes"].items():
        path = roots["checkpoint"] / entry["checkpoint"]
        if not path.is_file():
            raise ValueError(f"{name}: checkpoint missing: {path}")
    return roots

def validate_mesh_contract(gltf, body, stats):
    nodes = [node for node in gltf["nodes"] if "mesh" in node]
    if len(nodes) != 1:
        return [f"expected one mesh-bearing node, found {len(nodes)}"]
    node = nodes[0]
    mesh = gltf["meshes"][node["mesh"]]
    if node.get("name") != body or mesh.get("name") != body:
        return [f"mesh-bearing node/mesh must both be {body!r}"]
    return compare_mesh_stats(mesh, gltf, stats)
```

Use the current `animation_qa.py` GLB JSON/BIN parser pattern. `compare_mesh_stats` compares primitive count, each primitive index/POSITION accessor count, and ordered material indices captured by Blender; no weapon-name heuristic is an acceptance condition.

- [ ] **Step 4: Verify green and commit.**

Run: `python3 scripts/test_character_promotion_validation.py`

Expected: PASS.

```bash
git add scripts/configs/character-promotion/unarmed-checkpoints-v1.json scripts/character_promotion_validation.py scripts/test_character_promotion_validation.py
```

### Task 2: Blender Export and Action-Slot Transfer

**Files:**
- Create: `scripts/export_checkpoint_character.py`
- Create: `scripts/character_action_transfer.py`
- Create: `scripts/test_character_action_transfer.py`

**Interfaces:**
- Export CLI: `blender -b CHECKPOINT -P scripts/export_checkpoint_character.py -- --out OUT --body BODY --clip Idle_Relaxed --clip Walk_Forward --stats-out STATS`.
- Transfer CLI: `blender -b -P scripts/character_action_transfer.py -- --target TARGET --baseline BASE --out OUT --body BODY --retain IDLE --retain IDLE`.
- Helpers: `assign_armature_action(armature, action) -> None`, `copy_named_action(source_action, target_armature, name) -> bpy.types.Action`, `validate_export_scene(armature, body) -> dict`, and `export_actions(armature, names, out) -> None`.

- [ ] **Step 1: Write failing action-slot and real smoke tests.**

```python
def test_assign_armature_action_sets_exactly_one_slot(self):
    result = run_blender("tests/blender/action_slot_fixture.py")
    self.assertIn("slot=ActionSlot", result.stdout)

def test_monk_checkpoint_plus_canonical_exports_four_expected_actions(self):
    result = run_blender_transfer("monk", self.stage)
    self.assertEqual(result.returncode, 0, result.stderr)
    self.assertEqual(read_animation_names(self.stage / "monk.glb"), ["Idle_Relaxed", "Idle_Meditative", "Idle_Drinking", "Walk_Forward"])
```

- [ ] **Step 2: Verify red.**

Run: `python3 scripts/test_character_action_transfer.py`

Expected: FAIL because neither Blender helper exists.

- [ ] **Step 3: Implement exact Blender 5 action-slot behavior.**

```python
def assign_armature_action(armature, action):
    if armature.animation_data is None:
        armature.animation_data_create()
    for track in list(armature.animation_data.nla_tracks):
        armature.animation_data.nla_tracks.remove(track)
    armature.animation_data.action = action
    assert len(action.slots) == 1, f"{action.name}: expected one action slot, got {len(action.slots)}"
    armature.animation_data.action_slot = action.slots[0]

def copy_named_action(source_action, target_armature, name):
    copied = source_action.copy()
    copied.name = name
    copied.use_fake_user = True
    assign_armature_action(target_armature, copied)
    target_armature.animation_data.action = None
    return copied
```

`validate_export_scene` removes only importer-created `Icosphere`, then requires one armature and one mesh object named `--body`, with no other mesh objects. It captures `primitiveCount`, `positionCounts`, `indexCounts`, and `materialSlots` to `--stats-out`. The transfer script imports staged checkpoint `--target`, records its `Idle_Relaxed`/`Walk_Forward` actions, imports baseline canonical A, finds the source armature, copies exactly the `--retain` actions with `copy_named_action`, deletes baseline armature and every nonselected action, resolves name collisions by deleting a preexisting action before copying, then exports exactly `[Idle_Relaxed, retained[0], retained[1], Walk_Forward]` with `export_anim_single_armature=True`. It asserts checkpoint actions are present before baseline import and their decoded post-export payloads are the authoritative ones.

- [ ] **Step 4: Run real integration smoke, not mocks.**

Run: `python3 scripts/test_character_action_transfer.py --checkpoint-dir "$SYNTY_APPROVED_CHECKPOINT_DIR" --canonical-root harness/models/synty --stage-dir tmp/action-transfer-smoke`

Expected: PASS; it runs Blender on monk checkpoint + `harness/models/synty/characters/monk.glb`, produces four clips, records one mesh, and retains identical decoded `Idle_Meditative`/`Idle_Drinking` payloads.

- [ ] **Step 5: Commit.**

```bash
git add scripts/export_checkpoint_character.py scripts/character_action_transfer.py scripts/test_character_action_transfer.py
```

### Task 3: Staged Mesh and Animation QA Interfaces

**Files:**
- Modify: `scripts/build_mesh_stats.py`
- Create: `scripts/test_build_mesh_stats.py`
- Modify: `scripts/animation_qa.py`
- Modify: `scripts/test_animation_qa.py`

**Interfaces:**
- Mesh CLI: `python3 scripts/build_mesh_stats.py [--harness-dir HARNESS] [--out OUT] [--strict] [--print-derivation]`; defaults remain existing `harness` and `harness/models/synty/mesh-stats.json` behavior.
- Animation CLI: `python3 scripts/animation_qa.py --paths PATH [PATH ...] --input-root ROOT --report OUT`; `--input-root` is required with `--paths`, and report names are `relpath(path, input_root)`.

- [ ] **Step 1: Write failing staged-root and single-path tests.**

```python
def test_mesh_stats_writes_only_requested_staged_output(self):
    out = self.tmp / "stage/harness/models/synty/mesh-stats.json"
    stats.build(harness_dir=self.stage_harness, out_path=out)
    self.assertTrue(out.is_file())
    self.assertFalse(self.canonical_harness.joinpath("models/synty/mesh-stats.json").exists())

def test_animation_paths_use_explicit_input_root_for_one_file(self):
    report = qa.build_report(paths=["/stage/monk.glb"], input_root="/stage")
    self.assertEqual(report["clips"][0]["file"], "monk.glb")
```

- [ ] **Step 2: Verify red.**

Run: `python3 scripts/test_build_mesh_stats.py && python3 scripts/test_animation_qa.py ExplicitPathTests.test_animation_paths_use_explicit_input_root_for_one_file`

Expected: FAIL because `build` has no `harness_dir`/`out_path` arguments and `build_report` has no explicit input root.

- [ ] **Step 3: Implement backward-compatible arguments and report parser.**

```python
def build(strict=False, show_derivation=False, harness_dir=HARNESS_DIR, out_path=None):
    harness_synty_dir = os.path.join(os.path.abspath(harness_dir), "models", "synty")
    out_path = out_path or os.path.join(harness_synty_dir, "mesh-stats.json")
    for root, _dirs, files in os.walk(harness_synty_dir):
        rel_to_synty = os.path.relpath(abs_path, harness_synty_dir)
        rel_to_repo = os.path.relpath(abs_path, os.path.abspath(harness_dir))
    with open(out_path, "w") as f:
        json.dump(manifest, f, indent=2)
```

Wire argparse `--harness-dir` defaulting to `os.path.join(REPO_ROOT, "harness")` and `--out` defaulting to existing `OUT_PATH`; preserve `python3 scripts/build_mesh_stats.py` output/path. For animation QA, reject `--paths` without `--input-root`, reject a path outside the root, and use `os.path.relpath(path, input_root)` without `commonpath`.

- [ ] **Step 4: Verify staged tools and hard-gate parsing.**

Run: `python3 scripts/test_build_mesh_stats.py && python3 scripts/test_animation_qa.py`

Expected: PASS.

Add `parse_hard_gates(report, changed_relpaths) -> list[str]` to `character_promotion_validation.py`; it returns an error for any changed path absent from the report, `report["errors"]`, or any changed clip whose `verdict != "PASS"`. It deliberately ignores `footSlide.exceedsInformationalThreshold`.

- [ ] **Step 5: Commit.**

```bash
git add scripts/build_mesh_stats.py scripts/test_build_mesh_stats.py scripts/animation_qa.py scripts/test_animation_qa.py scripts/character_promotion_validation.py scripts/test_character_promotion_validation.py
```

### Task 4: Stage, Validate, and Prove the Real Complete Set

**Files:**
- Create: `scripts/promote_character_checkpoints.py`
- Modify: `scripts/test_promote_character_checkpoints.py`

**Interfaces:** `stage(config, worktree_root, checkpoint_dir, atlas_dir, stage_dir) -> Path`, `validate(config, stage_dir, baseline_commit) -> dict`, `release_inventory(config, portrait_paths) -> list[str]`.

- [ ] **Step 1: Write failing complete-inventory/metadata tests.**

```python
def test_release_metadata_records_baseline_and_excludes_external_evidence(self):
    metadata = cli.validate(CONFIG, self.stage, baseline_commit="a" * 40)
    self.assertEqual(metadata["baselineCommit"], "a" * 40)
    self.assertNotIn("playtest-evidence", json.dumps(metadata["inventory"]))
    self.assertEqual(len([p for p in metadata["inventory"] if p.endswith(".glb")]), 16)
```

- [ ] **Step 2: Verify red.**

Run: `python3 scripts/test_promote_character_checkpoints.py`

Expected: FAIL with `ModuleNotFoundError: No module named 'promote_character_checkpoints'`.

- [ ] **Step 3: Implement staging and validation only.**

```python
def stage(config, worktree_root, checkpoint_dir, atlas_dir, stage_dir):
    roots = v.resolve_inputs(config, checkpoint_dir, atlas_dir)
    release = Path(stage_dir) / "release"
    for class_name, entry in config["classes"].items():
        a = release / "harness/models/synty" / entry["canonical"]
        run_checkpoint_export(roots["checkpoint"] / entry["checkpoint"], a, entry)
        run_action_transfer(a, Path(worktree_root) / "harness/models/synty" / entry["canonical"], a, entry)
        run_swap_atlas_variants(a, roots["atlas"], release, entry)
    return release
```

Copy the current harness into `release/harness` before writing changed files so `build_mesh_stats.py --harness-dir release/harness --out release/harness/models/synty/mesh-stats.json` sees a complete staged harness. Invoke `animation_qa.py --paths` with all 16 staged GLBs, `--input-root release/harness/models/synty`, and `--report release/harness/animation-qa/report.json`; parse its report with `parse_hard_gates`. Run real `swap_atlas.py` on staged A copies. Compare staged mesh warnings against baseline `harness/models/synty/mesh-stats.json` only for changed 16 paths. Write release metadata, but do not write a canonical file and do not call Git.

- [ ] **Step 4: Execute the full real four-class integration gate.**

Run: `python3 scripts/promote_character_checkpoints.py stage --config scripts/configs/character-promotion/unarmed-checkpoints-v1.json --worktree-root "$PWD" --checkpoint-dir "$SYNTY_APPROVED_CHECKPOINT_DIR" --atlas-dir "$SYNTY_FANTASY_RIVALS_TEXTURE_DIR" --stage-dir tmp/unarmed-v1 && python3 scripts/promote_character_checkpoints.py validate --config scripts/configs/character-promotion/unarmed-checkpoints-v1.json --stage-dir tmp/unarmed-v1 --baseline-commit "$(git rev-parse HEAD)"`

Expected: exit 0; metadata lists exactly the 16 GLBs, manifest/docs/reports, and only regenerated portraits; every class/color passes one-mesh, Root, exact clip, retained payload, constrained-diff, variant parity, mesh, and parsed animation hard gates.

- [ ] **Step 5: Commit.**

```bash
git add scripts/promote_character_checkpoints.py scripts/test_promote_character_checkpoints.py
```

### Task 5: One-Commit Publication and Durable Git-Revert Rollback

**Files:**
- Modify: `scripts/promote_character_checkpoints.py`
- Modify: `scripts/test_promote_character_checkpoints.py`
- Modify: `README.md`
- Modify: `harness/models/synty/characters/manifest.json`

**Interfaces:** `verify_release_inventory(worktree_root, metadata) -> None` and `write_release_metadata(stage_dir, baseline_commit) -> Path`; neither function moves files or reverts Git.

- [ ] **Step 1: Write a temporary-real-Git test.**

```python
def test_one_release_commit_is_reverted_as_a_complete_unit(self):
    repo = init_real_git_repo(self.tmp)
    baseline = commit(repo, {"a.glb": b"old-a", "manifest.json": b"old"}, "baseline")
    release = commit(repo, {"a.glb": b"new-a", "b.glb": b"new-b", "manifest.json": b"new", "release-metadata.json": json.dumps({"baselineCommit": baseline}).encode()}, "release")
    subprocess.run(["git", "revert", "--no-edit", release], cwd=repo, check=True)
    self.assertEqual((repo / "a.glb").read_bytes(), b"old-a")
    self.assertFalse((repo / "b.glb").exists())
    self.assertEqual(json.loads((repo / "release-metadata.json").read_text())["baselineCommit"], baseline)
```

- [ ] **Step 2: Verify red.**

Run: `python3 scripts/test_promote_character_checkpoints.py PublicationTests.test_one_release_commit_is_reverted_as_a_complete_unit`

Expected: FAIL because temporary-repository helpers and release metadata do not exist.

- [ ] **Step 3: Implement commit-boundary workflow documentation.**

```python
def verify_release_inventory(worktree_root, metadata):
    root = Path(worktree_root)
    for rel, digest in metadata["artifacts"].items():
        if sha256_file(root / rel) != digest:
            raise RuntimeError(f"release artifact hash mismatch: {rel}")
```

The CLI writes `release-metadata.json` into the staged release with `baselineCommit` before commit. After the one release commit is created, the executor records its SHA as `releaseCommit` in the PR body, gate review, and rollback issue; metadata does not self-reference an unknown future commit. README must state: pre-commit abort discards worktree; post-publication rollback is `git revert <releaseCommit>` in a new issue/PR; public evidence is excluded.

- [ ] **Step 4: Verify real Git integration and documentation.**

Run: `python3 scripts/test_promote_character_checkpoints.py && git diff -- README.md harness/models/synty/characters/manifest.json`

Expected: test PASS; documentation contains no filesystem-atomicity claim, no `os.replace`, and no temporary-backup rollback claim.

- [ ] **Step 5: Commit.**

```bash
git add scripts/promote_character_checkpoints.py scripts/test_promote_character_checkpoints.py README.md harness/models/synty/characters/manifest.json
```

### Task 6: Exact Evidence, Independent Gate, and Release Execution

**Files:**
- Generated private release files listed in File Map.
- External only: `rpg-dnd5e-web` `evidence/asset-pipeline-wave1:playtest-evidence/asset-pipeline/unarmed-character-animation-promotion/`.

- [ ] **Step 1: Create implementation tracking and isolated worktree.**

Create one private `rpg-game-assets` issue/Board item and branch `asset/119-unarmed-character-animation-promotion` from its recorded baseline SHA; reference `rpg-project#119` and `rpg-project#120` in the issue and PR. Do not reuse either project-tracking item as implementation work.

- [ ] **Step 2: Run unit, Blender smoke, and full staged integration gates.**

Run: `python3 scripts/test_character_promotion_validation.py && python3 scripts/test_build_mesh_stats.py && python3 scripts/test_animation_qa.py && python3 scripts/test_character_action_transfer.py --checkpoint-dir "$SYNTY_APPROVED_CHECKPOINT_DIR" --canonical-root harness/models/synty --stage-dir tmp/action-transfer-smoke && python3 scripts/test_promote_character_checkpoints.py`

Expected: PASS. Then run Task 4's full four-class stage/validate command.

- [ ] **Step 3: Produce exact multi-angle animation evidence.**

```bash
for class in fighter barbarian monk rogue; do
  for clip in Idle_Relaxed Walk_Forward; do
    for angle in 0 45 90 180; do
      out="tmp/unarmed-v1/evidence/${class}-${clip}-${angle}"
      blender -b -P scripts/animation_qa_render.py -- --character "tmp/unarmed-v1/release/harness/models/synty/characters/${class}.glb" --clip "$clip" --out-dir "$out" --frames 28 --wrap-frames 2 --width 256 --height 320 --angle "$angle" --delay-cs 4
      node scripts/render_animation_evidence.mjs --frames-dir "$out" --out-strip "tmp/unarmed-v1/evidence/${class}-${clip}-${angle}.png" --out-gif "tmp/unarmed-v1/evidence/${class}-${clip}-${angle}.gif" --title "${class} ${clip} angle ${angle}" --verdict PASS
    done
  done
done
```

Expected: 32 PNG strips and 32 decodable GIFs. Create the 4x4 portrait contact sheet, obtain the independent recorded weapon-free viewed statement, and publish only screenshots/GIFs plus SHA-256 manifest to the external evidence path. These files are not copied into release metadata or the private release commit.

- [ ] **Step 4: Publish one complete private release commit.**

Run: `git status --short && git diff --cached --stat && git diff --cached --check && git add README.md scripts harness/models/synty/characters/manifest.json harness/models/synty/characters/{fighter,barbarian,monk,rogue}.glb harness/models/synty/characters/{fighter,barbarian,monk,rogue}-{b,c,d}.glb harness/models/synty/mesh-stats.json harness/animation-qa`

Expected: staging contains every release-metadata inventory artifact and no checkpoint, FBX, atlas, temporary stage, or public evidence asset. Commit once:

```bash
git commit -m "asset: promote unarmed character animation checkpoints"
```

The commit is the only publication boundary. A pre-commit failure discards/rebuilds the worktree; post-commit rollback is only `git revert <releaseCommit>` in a new issue/PR.

- [ ] **Step 5: Independent gate, sync, and actual-client verification.**

An independent agent in another worktree reruns the complete command from Step 2, inspects the staged GLBs/reports/evidence, verifies the release inventory against `git show --name-only <releaseCommit>`, and posts `GATE REVIEW`. After that, run in `rpg-dnd5e-web`: `npm run assets:sync && npm run test:run && npm run ci-check`. Verify Home → character select → Play → lobby → Start → `EncounterView` for all four classes shows idle → `Walk_Forward` → idle using unchanged canonical paths and no weapon. A client defect gets its own issue; no web code change is made here.

- [ ] **Step 6: Push one implementation PR.**

Run: `git push -u origin asset/119-unarmed-character-animation-promotion`

Expected: normal push succeeds. Open one ready private PR for the one issue, cite `rpg-project#119`/`rpg-project#120`, baseline/release SHAs, complete-set report results, external evidence hashes/links, sync/client proof, and independent gate. End GitHub comments with `— asset-pipeline agent, on behalf of KirkDiggler`.

## Final Review Checklist

- [ ] No plan/design text claims per-file filesystem atomicity, temporary-backup rollback, or an external evidence item in the private release inventory.
- [ ] The staged tools use explicit roots/output paths and preserve their existing default CLI behavior.
- [ ] Blender integration proves action-slot transfer and `swap_atlas.py` against real local assets before the four-class batch.
- [ ] The full staged release and temporary real-Git revert tests pass; every changed animation report entry is explicitly `PASS`.
- [ ] The one future implementation issue/PR references `rpg-project#119` and `rpg-project#120`; future batches only change config/input unless this contract changes.
