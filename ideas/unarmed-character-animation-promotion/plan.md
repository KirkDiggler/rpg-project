# Unarmed Character Animation Promotion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a reusable private workflow that stages, validates, materializes, index-verifies, and publishes one complete unarmed four-class character release.

**Architecture:** An isolated `rpg-game-assets` worktree begins at a recorded baseline commit. Blender creates the A GLB and transfers retained actions with explicit Blender 5 slots; `swap_atlas.py` generates B/C/D. Validation writes a versioned inventory/hashes file, `apply` materializes only that inventory, and `verify-index` proves the staged Git index matches it before the one release commit.

**Tech Stack:** Python 3 stdlib, Blender 5 `bpy`, existing `swap_atlas.py`, `animation_qa.py`, `build_mesh_stats.py`, Git, and Node `sharp`.

## Global Constraints

- Implement later in one private `rpg-game-assets` issue/Board 19 Team `Assets` item/branch/PR referencing **rpg-project issue #119** and **rpg-project PR #120**.
- Config contains only logical checkpoint/atlas filenames. Require explicit `--checkpoint-dir` and `--atlas-dir` (or equivalent exported variables) before work begins.
- Baseline worktree must be clean and equal `baselineCommit`; pre-commit failures discard/rebuild it. The one release commit is publication. Post-publication rollback is a new PR running `git revert <releaseCommit>`.
- Metadata records `baselineCommit`, `workflowVersion`, config SHA-256, release inventory/hashes, portrait-evidence SHA-256, and gate results. It never records its own unknown release commit SHA; that SHA is recorded in the PR/gate/rollback issue.
- Exact clips, ordered `idleClips`, unarmed `bakedIntoModel: false`, Root transforms, assembly names, no downed edits, standalone weapon retention, mesh/animation gates, and no web product changes are exactly as binding design.md states.
- Public evidence is only `rpg-dnd5e-web` branch `evidence/asset-pipeline-wave1` under `playtest-evidence/asset-pipeline/unarmed-character-animation-promotion/`; it is excluded from private release inventory and rollback.

## File Map

- Create `scripts/configs/character-promotion/unarmed-checkpoints-v1.json`: logical class/checkpoint/atlas/clip/body contract.
- Create `scripts/export_checkpoint_character.py` and `scripts/character_action_transfer.py`: one-mesh export and Blender 5 retained-action transfer.
- Create `scripts/character_promotion_validation.py`: semantic GLB, animation, QA-report, inventory/hash validators.
- Create `scripts/promote_character_checkpoints.py`: `stage`, `validate`, `apply`, `verify-index` CLI.
- Create `scripts/render_character_portraits.py`: versioned 512x512 staged portrait renderer promoted from the external renderer interface.
- Create `scripts/render_character_portrait_contact_sheet.mjs`: deterministic manifest-driven 4x4 labelled portrait sheet using `sharp`.
- Create `scripts/test_character_promotion_validation.py`, `scripts/test_character_action_transfer.py`, `scripts/test_promote_character_checkpoints.py`, `scripts/test_build_mesh_stats.py`, `scripts/test_render_character_portrait_contact_sheet.mjs`.
- Modify `scripts/build_mesh_stats.py`, `scripts/animation_qa.py`, `scripts/test_animation_qa.py`, `README.md`, `harness/models/synty/characters/manifest.json`, generated standing GLBs/conditional portraits/reports.

## CLI Contract

```bash
python3 scripts/promote_character_checkpoints.py stage --config scripts/configs/character-promotion/unarmed-checkpoints-v1.json --repo-root "$PWD" --checkpoint-dir "$SYNTY_APPROVED_CHECKPOINT_DIR" --atlas-dir "$SYNTY_FANTASY_RIVALS_TEXTURE_DIR" --stage-dir tmp/unarmed-v1
node scripts/render_character_portrait_contact_sheet.mjs --manifest tmp/unarmed-v1/release/harness/models/synty/characters/manifest.json --input-root tmp/unarmed-v1/release/harness/models/synty --out /home/kirk/game-dev/rpg-dnd5e-web/playtest-evidence/asset-pipeline/unarmed-character-animation-promotion/portraits-v1.png --sha-out /home/kirk/game-dev/rpg-dnd5e-web/playtest-evidence/asset-pipeline/unarmed-character-animation-promotion/portraits-v1.sha256
python3 scripts/promote_character_checkpoints.py validate --config scripts/configs/character-promotion/unarmed-checkpoints-v1.json --repo-root "$PWD" --stage-dir tmp/unarmed-v1 --baseline-commit "$(git rev-parse HEAD)" --portrait-evidence-sha /home/kirk/game-dev/rpg-dnd5e-web/playtest-evidence/asset-pipeline/unarmed-character-animation-promotion/portraits-v1.sha256
python3 scripts/promote_character_checkpoints.py apply --repo-root "$PWD" --stage-dir tmp/unarmed-v1 --baseline-commit "$(git rev-parse HEAD)"
git add README.md scripts harness/models/synty/characters/manifest.json harness/models/synty/characters/{fighter,barbarian,monk,rogue}.glb harness/models/synty/characters/{fighter,barbarian,monk,rogue}-{b,c,d}.glb harness/models/synty/mesh-stats.json harness/animation-qa
python3 scripts/promote_character_checkpoints.py verify-index --repo-root "$PWD" --stage-dir tmp/unarmed-v1
git diff --cached --check && git diff --cached --stat
git commit -m "asset: promote unarmed character animation checkpoints"
```

`apply` refuses non-clean worktrees, a HEAD different from metadata `baselineCommit`, missing/changed validated release files, or an inventory path outside the repo. It copies each release path to its canonical relative path, writes tracked `harness/models/synty/characters/unarmed-promotion-v1.json`, and changes nothing else. `verify-index` runs `git diff --cached --name-only`, requires its sorted set equal metadata inventory, obtains each staged blob with `git show :<path>`, and compares SHA-256 to metadata. It returns nonzero before commit on any mismatch.

### Task 1: Semantic Validators

**Files:** Create `scripts/character_promotion_validation.py`, `scripts/test_character_promotion_validation.py`.

**Interfaces:** `semantic_glb(gltf, bin) -> dict`, `animation_fingerprint(gltf, bin, animation) -> dict`, `validate_candidate(baseline, candidate, allowed_weapon_nodes) -> list[str]`, `validate_retained_actions(baseline, candidate, names) -> list[str]`, `validate_variant_parity(paths, expected) -> list[str]`.

- [ ] **Step 1: Write red tests.**

```python
def test_candidate_rejects_nonweapon_material_change(self):
    self.assertEqual(v.validate_candidate(self.base, self.changed_material, {"weapon"}), ["material[0] changed"])

def test_retained_fingerprint_uses_target_bone_path_times_values(self):
    self.assertEqual(v.validate_retained_actions(self.base, self.reexported, ["Idle_Drinking"]), [])

def test_variant_parity_rejects_sampler_count_difference(self):
    self.assertEqual(v.validate_variant_parity(self.variants, EXPECTED), ["monk c Walk_Forward sampler count differs from a"])
```

- [ ] **Step 2: Verify red.** Run: `python3 scripts/test_character_promotion_validation.py` Expected: FAIL with missing module.

- [ ] **Step 3: Implement robust semantics.**

```python
def animation_fingerprint(gltf, bin_data, animation):
    nodes = build_nodes(gltf)
    return {animation["name"]: sorted((nodes[ch["target"]["node"]].name, ch["target"]["path"],
        tuple(decode_accessor(gltf, bin_data, animation["samplers"][ch["sampler"]]["input"])),
        tuple(decode_accessor(gltf, bin_data, animation["samplers"][ch["sampler"]]["output"]))) for ch in animation["channels"])}
```

`semantic_glb` resolves node names, children, skins, meshes/primitives/POSITION/index accessors, materials/textures/images, and animations by semantic name, not indices. `validate_candidate` permits only configured weapon subtree mesh/node/material/texture/image removal and animation payload changes; Root, body node, body material/texture image bytes, skin joints, topology, and scene ownership must match. `validate_retained_actions` compares the fingerprints above for retained clips, which survives buffer/accessor reindexing. Parity compares exact clip set plus channel/sampler counts and fingerprints across A/B/C/D.

- [ ] **Step 4: Verify green and commit.** Run: `python3 scripts/test_character_promotion_validation.py` Expected: PASS. Commit `feat: validate semantic character promotion contracts`.

### Task 2: Blender Export, Transfer, and Real Re-export Gate

**Files:** Create `scripts/export_checkpoint_character.py`, `scripts/character_action_transfer.py`, `scripts/test_character_action_transfer.py`.

**Interfaces:** `assign_armature_action(armature, action)`, `copy_named_action(source, target, name)`, `validate_export_scene(armature, body) -> dict`; exporter `--out --body --stats-out --clip`; transfer `--target --baseline --out --body --retain`.

- [ ] **Step 1: Write red real-asset test.**

```python
def test_monk_real_reexport_preserves_retained_semantics(self):
    result = run_transfer("monk", self.checkpoint_dir, self.canonical_root, self.out)
    self.assertEqual(result.returncode, 0, result.stderr)
    self.assertEqual(v.validate_retained_actions(self.baseline, self.out, ["Idle_Meditative", "Idle_Drinking"]), [])
```

- [ ] **Step 2: Verify red.** Run: `python3 scripts/test_character_action_transfer.py --checkpoint-dir "$SYNTY_APPROVED_CHECKPOINT_DIR" --canonical-root harness/models/synty --stage-dir tmp/action-smoke` Expected: FAIL because helpers do not exist.

- [ ] **Step 3: Implement.**

```python
def assign_armature_action(armature, action):
    armature.animation_data_create() if armature.animation_data is None else None
    for track in list(armature.animation_data.nla_tracks): armature.animation_data.nla_tracks.remove(track)
    armature.animation_data.action = action
    assert len(action.slots) == 1
    armature.animation_data.action_slot = action.slots[0]
```

Exporter removes only `Icosphere`, requires exactly one armature and one mesh object named configured body, captures primitive/POSITION/index/material-slot stats, and rejects another mesh. Transfer imports staged target then baseline, copies only two configured actions after deleting name collisions, binds copied actions using the helper, removes baseline actions/armature, and exports checkpoint `Idle_Relaxed`/`Walk_Forward` plus retained actions exactly once.

- [ ] **Step 4: Verify green.** Run prior command. Expected: PASS and four clips. Commit `feat: transfer retained character actions with Blender slots`.

### Task 3: Staged QA and Portrait Producers

**Files:** Modify `scripts/build_mesh_stats.py`, `scripts/animation_qa.py`, `scripts/test_animation_qa.py`; create `scripts/test_build_mesh_stats.py`, `scripts/render_character_portraits.py`, `scripts/render_character_portrait_contact_sheet.mjs`, `scripts/test_render_character_portrait_contact_sheet.mjs`.

- [ ] **Step 1: Write red tests.**

```python
def test_mesh_stats_preserves_repo_relative_paths(self):
    stats.build(repo_root=self.stage_root, output=self.stage_root / "harness/models/synty/mesh-stats.json")
    self.assertEqual(load_asset()["file"], "harness/models/synty/characters/monk.glb")
```

```javascript
assert.equal(await sha256(out), await fs.readFile(shaOut, "utf8").then(s => s.split(" ")[0]));
assert.match(await fs.readFile(out, "utf8"), /fighter a/);
```

- [ ] **Step 2: Verify red.** Run: `python3 scripts/test_build_mesh_stats.py && node scripts/test_render_character_portrait_contact_sheet.mjs` Expected: FAIL because interfaces/producers do not exist.

- [ ] **Step 3: Implement producers and staged interfaces.** Mesh CLI is `python3 scripts/build_mesh_stats.py [--repo-root ROOT] [--output PATH]`; defaults are existing `REPO_ROOT` and `OUT_PATH`, preserving bytes and `harness/models/synty/...` paths. Stage a complete repo-root-shaped tree at `stage/release`, not a harness-only root. Animation QA requires `--paths ... --input-root ROOT`; report paths are relative to ROOT and changed verdicts are parsed as PASS, not CLI exit code.

Portrait renderer CLI is `blender -b -P scripts/render_character_portraits.py -- --input-root ROOT --manifest MANIFEST --classes fighter barbarian monk rogue --out-root OUT`; it resolves manifest colors `a,b,c,d`, renders transparent 512x512 `portraits/<class>{,-b,-c,-d}.png`, and writes `portrait-render-v1.json` with input/output SHA-256s. Contact CLI consumes that manifest plus staged manifest, orders fighter/barbarian/monk/rogue then a/b/c/d, labels `${class} ${color}`, writes one PNG and `<png>.sha256` using `crypto.createHash("sha256")`; install `sharp` in `scripts/node_modules` exactly as existing UI script documents.

- [ ] **Step 4: Verify producers.**

```bash
mkdir -p scripts/node_modules && (cd scripts && npm init -y && npm i sharp)
blender -b -P scripts/render_character_portraits.py -- --input-root tmp/unarmed-v1/release --manifest tmp/unarmed-v1/release/harness/models/synty/characters/manifest.json --classes fighter barbarian monk rogue --out-root tmp/unarmed-v1/release/harness/models/synty/characters
node scripts/render_character_portrait_contact_sheet.mjs --manifest tmp/unarmed-v1/release/harness/models/synty/characters/manifest.json --input-root tmp/unarmed-v1/release/harness/models/synty --out /home/kirk/game-dev/rpg-dnd5e-web/playtest-evidence/asset-pipeline/unarmed-character-animation-promotion/portraits-v1.png --sha-out /home/kirk/game-dev/rpg-dnd5e-web/playtest-evidence/asset-pipeline/unarmed-character-animation-promotion/portraits-v1.sha256
```

Expected: one labelled 4x4 PNG and SHA file. Independent pass statement is exactly: `I viewed portraits-v1.png and confirm no weapon or weapon fragment is visible in any of its 16 labelled frames.` If false for class X, rerun portrait renderer with `--classes X`, rebuild sheet, obtain a new statement, then rerun Task 4 validation. Commit `feat: render reproducible character portrait evidence`.

### Task 4: Stage, Validate, Apply, and Verify Index

**Files:** Create/modify `scripts/promote_character_checkpoints.py`, `scripts/test_promote_character_checkpoints.py`.

- [ ] **Step 1: Write red materialization/index tests.**

```python
def test_apply_and_verify_index_require_exact_validated_inventory(self):
    cli.apply(self.repo, self.stage, self.baseline)
    git(self.repo, "add", *self.metadata["inventory"])
    self.assertEqual(cli.verify_index(self.repo, self.stage), [])
    git(self.repo, "add", "unrelated.txt")
    self.assertEqual(cli.verify_index(self.repo, self.stage), ["staged paths differ from release inventory"])
```

- [ ] **Step 2: Verify red.** Run: `python3 scripts/test_promote_character_checkpoints.py` Expected: FAIL with missing CLI functions.

- [ ] **Step 3: Implement flow.** `stage` builds complete `stage/release` repo-shaped tree and records pre-export fingerprints. Portrait gate runs before `validate`. `validate` performs semantic constrained diff, retained fingerprint, A/B/C/D parity, one-mesh, staged mesh stats `--repo-root stage/release`, parsed changed-path animation PASS gates, then writes tracked inventory metadata. `apply` checks clean/baseline, copies validated paths only. `verify-index` compares staged set and staged blob SHA-256s.

- [ ] **Step 4: Run full flow.** Execute the CLI Contract commands in order, with `git add` before `verify-index`. Expected: all commands exit 0; no canonical edit occurs before `apply`; index exactly matches metadata after `apply`.

- [ ] **Step 5: Real Git rollback integration.**

```python
def test_release_commit_reverts_complete_inventory(self):
    baseline, release = make_commits(self.repo)
    git(self.repo, "revert", "--no-edit", release)
    self.assertEqual(tree_hashes(self.repo), tree_hashes_at(self.repo, baseline))
```

Run: `python3 scripts/test_promote_character_checkpoints.py` Expected: PASS. Commit `feat: materialize and verify character promotion releases`.

### Task 5: Evidence, Release, and Client Gate

- [ ] Run exact multi-angle commands:

```bash
for class in fighter barbarian monk rogue; do for clip in Idle_Relaxed Walk_Forward; do for angle in 0 45 90 180; do out="tmp/unarmed-v1/evidence/${class}-${clip}-${angle}"; blender -b -P scripts/animation_qa_render.py -- --character "tmp/unarmed-v1/release/harness/models/synty/characters/${class}.glb" --clip "$clip" --out-dir "$out" --frames 28 --wrap-frames 2 --width 256 --height 320 --angle "$angle" --delay-cs 4; node scripts/render_animation_evidence.mjs --frames-dir "$out" --out-strip "${out}.png" --out-gif "${out}.gif" --title "${class} ${clip} angle ${angle}" --verdict PASS; done; done; done
```

- [ ] Independent gate reruns Tasks 2-4, views portrait/multi-angle files, validates `git show --name-only <releaseCommit>` against metadata, and posts `GATE REVIEW`.
- [ ] After release commit only, run `npm run assets:sync && npm run test:run && npm run ci-check` in `rpg-dnd5e-web`; verify actual Home → character select → Play → lobby → Start → EncounterView idle → Walk_Forward → idle for all classes. File a separate web issue for any finding.
- [ ] Push one implementation PR citing **rpg-project issue #119** and **rpg-project PR #120**, baseline/release SHA, gate results, and external evidence SHA/links.

## Final Review Checklist

- [ ] Every design acceptance criterion maps to Tasks 1-5 and has a command.
- [ ] No tracked metadata self-references a release SHA; external evidence is excluded from release transaction.
- [ ] `stage -> portrait gate -> validate -> apply -> git add -> verify-index -> one commit` is the only publication order.
- [ ] No incomplete-marker or producer-free contact/evidence requirement remains.
