# Modular Elf Fighter Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ship one fixed-look, animated Modular Fantasy Hero Elf Fighter on the production session route from authoritative public race and class refs, while preserving class/downed/placeholder fallbacks and a future atlas-replacement seam.

**Architecture:** The private provider builds `elf:fighter` from a hash-bound modular recipe into one normalized GLB and generated manifest. The public web compiles that reviewed manifest entry into a typed race+class resolver, threads the roster's existing `raceRef` to local and peer entities, and applies a rig-family socket override without changing equipment authority. Provider work is issue `rpg-game-assets#80` from `origin/main`; web work is issue `rpg-dnd5e-web#849` from `origin/dev`.

**Tech Stack:** Python 3.11+ standard library, Blender 5.0.1 Python API, glTF/GLB, existing provider validation/atlas/inventory tools, React 19, TypeScript, React Three Fiber, Three.js, Vitest, Playwright/browser evidence.

**Spec:** `ideas/characters/modular-race-class/design.md`

## Global Constraints

- Journey: `KirkDiggler/rpg-project#320`; design authority: `rpg-project#321` / PR `#322`.
- Provider issue: `KirkDiggler/rpg-game-assets#80`; base new work on fresh `origin/main`.
- Web issue: `KirkDiggler/rpg-dnd5e-web#849`; base new work on fresh `origin/dev`.
- One branch per owning repository for the wave; provider publishes before web can merge.
- Initial exact key is normalized `elf:fighter`; only its standing model is modular.
- A downed Elf Fighter resolves to the existing Townfolk Fighter downed GLB.
- Standing output has one 63-bone rig, the 13 declared meshes, one embedded 1024x1024 PNG, and exactly `Idle_Relaxed` plus `Walk_Forward`.
- Default atlas is `PolygonFantasyHero_Texture_01_A.png`; alternate B is evidence only and is never promoted.
- `Customization` remains empty. No proto, API, toolkit, shader customization, runtime modular assembly, portrait, or modular downed output.
- Auto Rig Pro is not installed or required.
- Unknown/missing exact combinations fall back to class; missing/unloadable class models fall back to `MediumHumanoid`.
- One accepted socket profile serves the entire `modular-fantasy-hero-v1` rig family. No item-, race-, or class-specific correction.
- Raw archives, FBXs, textures, private absolute paths, and staged bytes never enter `rpg-project` or `rpg-dnd5e-web`.
- Public web commits code, hashes, and public-safe evidence only; synced GLBs remain ignored.
- Feature PRs receive exactly one Copilot review request; every finding is answered before readiness is reported.

## File map

### Provider (`rpg-game-assets#80`)

| File | Responsibility |
| --- | --- |
| `scripts/configs/character-promotion/modular-race-class-v1.json` | Hash-bound source, atlas, animation, output, and `elf:fighter` mesh recipe |
| `scripts/configs/animation/townfolk-to-modular-fantasy-hero-v1.json` | Explicit 48-bone source→target animation map |
| `scripts/modular_race_class_contract.py` | Pure recipe validation, safe selective ZIP staging, manifest generation, and non-image semantic comparison |
| `scripts/build_modular_race_class.py` | Blender-only assembly, action transfer, export, structural readback, socket calibration, and focused rendering |
| `scripts/promote_modular_race_class.py` | Normal-Python orchestration for validate/build/apply/check and generated provider updates |
| `scripts/test_modular_race_class_contract.py` | Fixture ZIP, hash/path safety, recipe, manifest, and atlas-semantic tests |
| `scripts/test_promote_modular_race_class.py` | CLI orchestration, refusal, and generated-output tests with subprocesses mocked |
| `harness/models/synty/characters/race-class/elf-fighter.glb` | Promoted private standing runtime asset |
| `harness/models/synty/characters/race-class/manifest.json` | Generated exact-key model/rig/atlas/socket runtime authority |
| `harness/models/synty/mesh-stats.json` | Refreshed generated mesh profile |
| `harness/catalogs/synty-complete-inventory.json` | Refreshed exact provider inventory |
| `evidence/80-modular-elf-fighter/README.md` | Private commands, visual verdict, limitations, and source boundary |
| `evidence/80-modular-elf-fighter/verification.json` | Structural, semantic, animation, atlas, socket, bounds, and output hashes |
| `evidence/80-modular-elf-fighter/*.png` | Idle/walk, atlas, and representative main-hand contact sheets |
| `README.md` | Race/class provider layout and repeatable command |

### Web (`rpg-dnd5e-web#849`)

| File | Responsibility |
| --- | --- |
| `src/components/hex-grid/classCharacterModels.ts` | Typed exact race+class→class→undefined model resolution with rig family |
| `src/components/hex-grid/classCharacterModels.test.ts` | Exact, normalization, downed, class fallback, and unknown-ref tests |
| `src/components/hex-grid/HexEntity.tsx` | Consume race+class resolution and select the model-family socket override |
| `src/components/hex-grid/ClassCharacterModel.tsx` | Apply an optional socket override without changing weapon identity |
| `src/components/hex-grid/ClassCharacterModel.test.tsx` | Socket override/remount and Townfolk-default regression tests |
| `src/components/hex-grid/mainHandWeapons.ts` | Provider-accepted modular socket constant and rig-family lookup |
| `src/components/hex-grid/mainHandWeapons.test.ts` | Exact provider socket values and unchanged Townfolk roster behavior |
| `src/components/session/SessionCanvas.tsx` | Carry local and visible-peer public race refs into `HexEntity` |
| `src/components/session/SessionCanvas.test.tsx` | Local/peer Elf Fighter and missing-roster fallback rendering tests |
| `src/components/session/SessionEncounterView.tsx` | Read the local public `raceRef` beside `classRef` and pass it to Canvas |
| `src/components/session/SessionEncounterView.test.tsx` | Public roster race authority and private-sheet non-authority tests |
| `docs/evidence/849-modular-elf-fighter/README.md` | Public-safe provider identity, commands, visual verdict, and fallback scope |
| `docs/evidence/849-modular-elf-fighter/receipt.json` | Exact merged provider commit, manifest/output hashes, and browser facts |
| `docs/evidence/849-modular-elf-fighter/*.png` | Production route/contact sheets with no licensed source bytes |

---

### Task 1: Provider recipe and safe source staging

**Files:**
- Create: `scripts/configs/character-promotion/modular-race-class-v1.json`
- Create: `scripts/modular_race_class_contract.py`
- Create: `scripts/test_modular_race_class_contract.py`

**Interfaces:**
- Produces `ContractError`, `load_recipe(path: Path) -> dict[str, object]`, `combination_for(recipe, key) -> dict[str, object]`, `stage_inputs(source_root, recipe, combination_key, stage_root) -> StagedInputs`, and `manifest_bytes(document) -> bytes`.
- `StagedInputs` exposes `archive`, `combined_fbx`, `default_atlas`, and `alternate_atlas` as stage-relative `Path` values.
- Task 2 consumes the validated recipe and staged files; no later task reads the source archive directly.

- [ ] **Step 1: Create the isolated provider worktree and mark issue #80 In Progress**

```bash
provider_repo=$HOME/game-dev/rpg-game-assets
git -C "$provider_repo" fetch origin
worktree=$HOME/.pi/worktrees/rpg-game-assets/80-modular-elf-fighter
git -C "$provider_repo" worktree add "$worktree" -b asset/80-modular-elf-fighter origin/main
cd "$worktree"
gh issue view 80 --repo KirkDiggler/rpg-game-assets --json state,url,projectItems
gh project item-edit \
  --id PVTI_lAHOAASbwc4Bcj4vzg4f5Rc \
  --project-id PVT_kwHOAASbwc4Bcj4v \
  --field-id PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM \
  --single-select-option-id a434eab1
python3 -m unittest discover -s scripts -p 'test_*.py'
```

Expected: branch starts at `origin/main`; issue #80 is open and tracked; established provider tests pass before changes.

- [ ] **Step 2: Write failing contract and staging tests**

Create fixture ZIPs entirely under `tempfile.TemporaryDirectory()` and pin these behaviors:

```python
class ModularRaceClassContractTests(unittest.TestCase):
    def test_loads_exact_elf_fighter_recipe(self):
        recipe = load_recipe(self.recipe_path)
        combo = combination_for(recipe, " ELF : FIGHTER ")
        self.assertEqual("elf", combo["race"])
        self.assertEqual("fighter", combo["class"])
        self.assertEqual(13, len(combo["meshes"]))
        self.assertEqual(
            "harness/models/synty/characters/race-class/elf-fighter.glb",
            combo["output"],
        )

    def test_stages_only_hash_verified_declared_members(self):
        staged = stage_inputs(self.source_root, load_recipe(self.recipe_path), "elf:fighter", self.stage)
        self.assertEqual(b"combined-fbx", staged.combined_fbx.read_bytes())
        self.assertEqual(b"atlas-a", staged.default_atlas.read_bytes())
        self.assertEqual(b"atlas-b", staged.alternate_atlas.read_bytes())
        self.assertEqual(
            sorted(["combined.fbx", "default.png", "alternate.png"]),
            sorted(path.name for path in self.stage.iterdir()),
        )

    def test_refuses_archive_or_member_hash_drift_without_replacing_stage(self):
        sentinel = self.stage / "sentinel"
        self.stage.mkdir()
        sentinel.write_text("keep")
        self.recipe["sourceArchive"]["sha256"] = "0" * 64
        with self.assertRaisesRegex(ContractError, "archive sha256"):
            stage_inputs(self.source_root, self.recipe, "elf:fighter", self.stage)
        self.assertEqual("keep", sentinel.read_text())
```

Also test unsafe archive member paths, absolute output paths, duplicate meshes, an unknown combination, wrong atlas dimensions, missing actions, unsupported rig family, unknown recipe keys, and source-root text absent from `manifest_bytes`.

- [ ] **Step 3: Run focused tests and verify RED**

Run:

```bash
python3 -m unittest scripts.test_modular_race_class_contract -v
```

Expected: FAIL with `ModuleNotFoundError: No module named 'modular_race_class_contract'`.

- [ ] **Step 4: Add the exact production recipe**

Create `scripts/configs/character-promotion/modular-race-class-v1.json` with these exact immutable inputs and selected meshes:

```json
{
  "schemaVersion": 1,
  "workflowVersion": "modular-race-class-v1",
  "rigFamily": "modular-fantasy-hero-v1",
  "sourceArchive": {
    "file": "POLYGON_Modular_Fantasy_Hero_SourceFiles_v2.zip",
    "sha256": "9ca5e73b4a41f513c07201858e94080dd5fb9d1c8e687d4ed42c014376fbb17e"
  },
  "combinedFbx": {
    "member": "Source_Files/FBX/ModularCharactersFixedScale.fbx",
    "sha256": "616858fd02f1f27a8492c38a5a319a69f30f1617af8cd12afa44ec467b76a7f2"
  },
  "atlases": {
    "default": {
      "key": "01-a",
      "member": "Source_Files/Textures/PolygonFantasyHero_Texture_01_A.png",
      "sha256": "7f84972790e530f8d83b378eb95f3151e2664c7b4ac23b1d125a55e1efcecd62",
      "dimensions": [1024, 1024]
    },
    "alternateEvidence": {
      "key": "01-b",
      "member": "Source_Files/Textures/PolygonFantasyHero_Texture_01_B.png",
      "sha256": "cb296fdc43a0051caa5e44209c40d7edae5e05900319eea8038bd55b8ca4fc4c",
      "dimensions": [1024, 1024]
    }
  },
  "animationSource": {
    "path": "harness/models/synty/characters/fighter.glb",
    "sha256": "cb3ad6ff4169bc4befd901e4f23f618498c3f9f895dfeaf4664d0131d20980ae",
    "actions": ["Idle_Relaxed", "Walk_Forward"]
  },
  "outputRoot": "harness/models/synty/characters/race-class",
  "manifest": "harness/models/synty/characters/race-class/manifest.json",
  "combinations": {
    "elf:fighter": {
      "race": "elf",
      "class": "fighter",
      "meshes": [
        "Chr_Head_Male_00",
        "Chr_Ear_Ear_03",
        "Chr_Hair_01",
        "Chr_Torso_Male_16",
        "Chr_Hips_Male_16",
        "Chr_ArmUpperLeft_Male_16",
        "Chr_ArmUpperRight_Male_16",
        "Chr_ArmLowerLeft_Male_16",
        "Chr_ArmLowerRight_Male_16",
        "Chr_HandLeft_Male_16",
        "Chr_HandRight_Male_16",
        "Chr_LegLeft_Male_16",
        "Chr_LegRight_Male_16"
      ],
      "defaultPalette": "01-a",
      "output": "harness/models/synty/characters/race-class/elf-fighter.glb"
    }
  }
}
```

- [ ] **Step 5: Implement strict recipe validation and atomic selective staging**

Core shapes:

```python
class ContractError(ValueError):
    pass

@dataclass(frozen=True)
class StagedInputs:
    archive: Path
    combined_fbx: Path
    default_atlas: Path
    alternate_atlas: Path


def combination_for(recipe: dict[str, object], key: str) -> dict[str, object]:
    normalized = ":".join(part.strip().lower() for part in key.split(":"))
    if normalized.count(":") != 1:
        raise ContractError(f"invalid race:class key: {key!r}")
    try:
        return recipe["combinations"][normalized]
    except KeyError as exc:
        raise ContractError(f"unknown race:class combination: {normalized}") from exc
```

`stage_inputs` must hash the archive before opening it, read exact member names rather than search by basename, verify each member hash and PNG dimensions, write into a temporary sibling directory, and replace the requested stage only after all checks pass. Reject symlinks, traversal, absolute paths, decorated schemas, and stage roots inside `harness/`, `library/`, or `evidence/`.

- [ ] **Step 6: Run focused and full pure-Python tests**

```bash
python3 -m unittest scripts.test_modular_race_class_contract -v
python3 -m unittest discover -s scripts -p 'test_*.py'
git diff --check
```

Expected: focused tests pass; full suite retains only established skips.

- [ ] **Step 7: Commit the provider contract**

```bash
git add scripts/configs/character-promotion/modular-race-class-v1.json \
        scripts/modular_race_class_contract.py \
        scripts/test_modular_race_class_contract.py
git commit -m 'feat: define modular race-class provider recipe (#80)'
```

---

### Task 2: Blender assembly and reusable animation transfer

**Files:**
- Create: `scripts/configs/animation/townfolk-to-modular-fantasy-hero-v1.json`
- Create: `scripts/build_modular_race_class.py`
- Create: `scripts/test_promote_modular_race_class.py`
- Modify: `scripts/modular_race_class_contract.py`

**Interfaces:**
- Consumes Task 1's recipe and `StagedInputs`.
- Produces `build_combination(recipe_path, mapping_path, source_root, combination_key, baseline, output, report) -> dict[str, object]` inside Blender.
- The report has exact keys `combination`, `armature`, `meshes`, `animations`, `texture`, `boundsMeters`, `geometry`, `rootProfile`, and `warnings`.
- Task 3 consumes the output GLB and report; Task 4 consumes the built rig and action names for socket evidence.

- [ ] **Step 1: Write failing mapping and CLI-orchestration tests**

Pin the explicit interface without requiring Blender in the ordinary test suite:

```python
class ModularPromotionTests(unittest.TestCase):
    def test_mapping_has_the_exact_48_townfolk_to_modular_pairs(self):
        mapping = load_animation_mapping(self.mapping_path)
        self.assertEqual(48, len(mapping["bones"]))
        self.assertEqual("Hips", mapping["bones"]["Pelvis"])
        self.assertEqual("Hand_R", mapping["bones"]["Hand_R"])
        self.assertEqual("Thumb_01.001", mapping["bones"]["thumb_01_r"])
        self.assertEqual("Toes_R", mapping["bones"]["toes_r"])

    def test_build_command_uses_only_declared_staged_inputs_and_baseline(self):
        command = blender_build_command(
            blender=Path("blender"),
            script=Path("scripts/build_modular_race_class.py"),
            recipe=self.recipe_path,
            mapping=self.mapping_path,
            source_root=self.source_root,
            combination="elf:fighter",
            baseline=Path("harness/models/synty/characters/fighter.glb"),
            output=Path(".stage/modular-race-class/elf-fighter.glb"),
            report=Path(".stage/modular-race-class/build-report.json"),
        )
        self.assertNotIn(str(Path.home()), " ".join(map(str, command)))
        self.assertEqual("elf:fighter", command[command.index("--combination") + 1])
```

Also test duplicate target bones, missing `Pelvis`/`Hips`, wrong baseline hash, non-Blender importability of the orchestration module, and subprocess failure output propagation.

- [ ] **Step 2: Run focused tests and verify RED**

```bash
python3 -m unittest scripts.test_promote_modular_race_class -v
```

Expected: FAIL because the mapping and builder command do not exist.

- [ ] **Step 3: Add the explicit 48-bone map**

Create `scripts/configs/animation/townfolk-to-modular-fantasy-hero-v1.json` with `schemaVersion: 1`, source `townfolk-v1`, target `modular-fantasy-hero-v1`, root mapping `Pelvis -> Hips`, and these exact pairs:

```json
{
  "Foot_L":"Ankle_L","Foot_R":"Ankle_R","Hand_L":"Hand_L","Hand_R":"Hand_R",
  "Pelvis":"Hips","Thigh_L":"UpperLeg_L","Thigh_R":"UpperLeg_R",
  "UpperArm_L":"Shoulder_L","UpperArm_R":"Shoulder_R","ball_l":"Ball_L","ball_r":"Ball_R",
  "calf_l":"LowerLeg_L","calf_r":"LowerLeg_R","clavicle_l":"Clavicle_L","clavicle_r":"Clavicle_R",
  "eyebrows":"Eyebrows","eyes":"Eyes","finger_01_l":"Finger_01","finger_01_r":"Finger_01.001",
  "finger_02_l":"Finger_02","finger_02_r":"Finger_02.001","finger_03_l":"Finger_03",
  "finger_03_r":"Finger_03.001","finger_04_l":"Finger_04","finger_04_r":"Finger_04.001",
  "head":"Head","indexFinger_01_l":"IndexFinger_01","indexFinger_01_r":"IndexFinger_01.001",
  "indexFinger_02_l":"IndexFinger_02","indexFinger_02_r":"IndexFinger_02.001",
  "indexFinger_03_l":"IndexFinger_03","indexFinger_03_r":"IndexFinger_03.001",
  "indexFinger_04_l":"IndexFinger_04","indexFinger_04_r":"IndexFinger_04.001",
  "lowerarm_l":"Elbow_L","lowerarm_r":"Elbow_R","neck_01":"Neck",
  "spine_01":"Spine_01","spine_02":"Spine_02","spine_03":"Spine_03",
  "thumb_01_l":"Thumb_01","thumb_01_r":"Thumb_01.001","thumb_02_l":"Thumb_02",
  "thumb_02_r":"Thumb_02.001","thumb_03_l":"Thumb_03","thumb_03_r":"Thumb_03.001",
  "toes_l":"Toes_L","toes_r":"Toes_R"
}
```

- [ ] **Step 4: Implement Blender assembly and action transfer**

`build_modular_race_class.py` must:

1. reset to an empty scene;
2. call Task 1 staging;
3. import the fixed-scale combined FBX;
4. require one 63-bone armature and all 13 declared mesh names;
5. delete every undeclared mesh and importer `Icosphere`;
6. bind the default atlas to one Principled material;
7. import the hash-pinned Fighter GLB and require exactly the two declared actions;
8. compute each rest correction as `target_rest.inverted() @ source_rest`;
9. sample every source keyframe time and bake `correction.inverted() @ source_quaternion @ correction` onto the mapped target bone;
10. rotate/scale `Pelvis.location` into `Hips.location` using the measured head-to-foot ratio;
11. remove the source armature/actions completely;
12. export only target armature plus declared meshes; and
13. re-import the GLB and write the structural report.

The key bake loop is:

```python
for frame in keyframe_times(source_action):
    scene.frame_set(int(math.floor(frame)), subframe=frame - math.floor(frame))
    view_layer.update()
    for source_name, target_name in bone_map.items():
        source_quaternion = source_armature.pose.bones[source_name].matrix_basis.to_quaternion()
        correction = corrections[source_name]
        target_bone = target_armature.pose.bones[target_name]
        target_bone.rotation_mode = "QUATERNION"
        target_bone.rotation_quaternion = correction.inverted() @ source_quaternion @ correction
        target_bone.keyframe_insert(data_path="rotation_quaternion", frame=frame)
```

Do not copy source IK helpers or invent animation on modular attachment bones; attachment bones inherit from their mapped parents.

- [ ] **Step 5: Run the real build and verify structural GREEN**

```bash
: "${SYNTY_SUBSCRIPTION_ROOT:?set this to the private Synty subscription directory}"
rm -rf .stage/modular-race-class
blender --background --python scripts/build_modular_race_class.py -- \
  --recipe scripts/configs/character-promotion/modular-race-class-v1.json \
  --mapping scripts/configs/animation/townfolk-to-modular-fantasy-hero-v1.json \
  --source-root "$SYNTY_SUBSCRIPTION_ROOT" \
  --combination elf:fighter \
  --baseline harness/models/synty/characters/fighter.glb \
  --output .stage/modular-race-class/elf-fighter.glb \
  --report .stage/modular-race-class/build-report.json
jq '{combination,armature,meshes,animations,texture,boundsMeters,geometry,warnings}' \
  .stage/modular-race-class/build-report.json
```

Expected: `armature.boneCount == 63`, 13 exact meshes, animations exactly `Idle_Relaxed` and `Walk_Forward`, one 1024² texture, `geometry.triangleCount == 3750`, no `Icosphere`, and no warning that invalidates promotion.

- [ ] **Step 6: Render sampled idle/walk frames and inspect them**

Use the existing animation QA renderer against the built GLB:

```bash
for clip in Idle_Relaxed Walk_Forward; do
  blender --background --python scripts/animation_qa_render.py -- \
    --character .stage/modular-race-class/elf-fighter.glb \
    --clip "$clip" \
    --out-dir ".stage/modular-race-class/$clip" \
    --frames 12 --wrap-frames 2
done
```

Expected: arms remain down/natural, feet alternate through walk, no mesh gaps, no T-pose contamination, and the wrap frames do not expose a new discontinuity.

- [ ] **Step 7: Run focused tests and commit the builder**

```bash
python3 -m unittest scripts.test_promote_modular_race_class -v
git diff --check
git add scripts/configs/animation/townfolk-to-modular-fantasy-hero-v1.json \
        scripts/build_modular_race_class.py \
        scripts/test_promote_modular_race_class.py \
        scripts/modular_race_class_contract.py
git commit -m 'feat: assemble animated modular Elf Fighter (#80)'
```

---

### Task 3: Atlas proof, generated manifest, and promotion orchestration

**Files:**
- Create: `scripts/promote_modular_race_class.py`
- Modify: `scripts/modular_race_class_contract.py`
- Modify: `scripts/test_modular_race_class_contract.py`
- Modify: `scripts/test_promote_modular_race_class.py`
- Create during apply: `harness/models/synty/characters/race-class/elf-fighter.glb`
- Create during apply: `harness/models/synty/characters/race-class/manifest.json`

**Interfaces:**
- Produces CLI modes `validate`, `build`, `apply`, and `check`.
- Produces `build_manifest(recipe, combination, report, output_sha256, socket_profile) -> dict[str, object]` and `non_image_semantic_sha256(glb_path) -> str`.
- Task 4 supplies the accepted socket profile before `apply`; Task 5 treats `check` as the provider release gate.

- [ ] **Step 1: Write failing manifest, atlas, and CLI mode tests**

```python
def test_manifest_records_exact_key_rig_atlas_clips_and_output(self):
    manifest = build_manifest(self.recipe, self.combo, self.report, "a" * 64, self.socket)
    self.assertEqual(["elf:fighter"], list(manifest["combinations"]))
    entry = manifest["combinations"]["elf:fighter"]
    self.assertEqual("modular-fantasy-hero-v1", entry["rigFamily"])
    self.assertEqual("01-a", entry["defaultPalette"])
    self.assertEqual([1024, 1024], entry["atlasDimensions"])
    self.assertEqual(["Idle_Relaxed", "Walk_Forward"], entry["animations"])
    self.assertEqual("characters/race-class/elf-fighter.glb", entry["model"])


def test_alternate_atlas_changes_image_digest_but_not_non_image_semantics(self):
    replace_embedded_atlas(self.default_glb, self.alternate_glb, self.atlas_b)
    self.assertNotEqual(sha256_file(self.default_glb), sha256_file(self.alternate_glb))
    self.assertEqual(
        non_image_semantic_sha256(self.default_glb),
        non_image_semantic_sha256(self.alternate_glb),
    )
```

Also test deterministic manifest bytes, no absolute path, one embedded image only, refusal to `apply` without an accepted socket report, check-mode byte comparison, and preservation of a prior runtime output on failed build.

- [ ] **Step 2: Run focused tests and verify RED**

```bash
python3 -m unittest scripts.test_modular_race_class_contract scripts.test_promote_modular_race_class -v
```

Expected: FAIL because manifest/semantic/CLI functions are missing.

- [ ] **Step 3: Implement deterministic manifest and atlas semantic comparison**

Use the existing `swap_atlas.replace_embedded_atlas` for the temporary B proof. `non_image_semantic_sha256` must remove `images`, normalize the image `bufferView`, rebuild the remaining buffer views into a contiguous non-image binary stream, rewrite their offsets against that stream, normalize `buffers[0].byteLength`, and hash the resulting canonical JSON plus rebuilt binary. This makes different PNG lengths comparable while still detecting any changed node, skin, accessor, animation, or non-image byte; do not treat a Blender re-export as equivalent.

Manifest top-level keys are exactly:

```python
{
    "schemaVersion": 1,
    "workflowVersion": "modular-race-class-v1",
    "socketProfiles": {socket_profile["id"]: socket_profile["socket"]},
    "combinations": {"elf:fighter": combination_entry},
}
```

`manifest_bytes` uses sorted keys, two-space indentation, ASCII output, `allow_nan=False`, and one trailing newline.

- [ ] **Step 4: Implement validate/build/apply/check orchestration**

`build` invokes Blender into `.stage/modular-race-class`, writes the alternate-atlas evidence there, and never touches `harness/`. `apply` requires successful report/socket/semantic checks, copies the accepted default GLB to a temporary sibling, writes the manifest to a temporary sibling, then replaces those two files. `check` rebuilds in a fresh temporary directory and compares generated runtime/manifest semantics against tracked outputs.

- [ ] **Step 5: Run focused tests and commit orchestration**

```bash
python3 -m unittest scripts.test_modular_race_class_contract scripts.test_promote_modular_race_class -v
git diff --check
git add scripts/promote_modular_race_class.py \
        scripts/modular_race_class_contract.py \
        scripts/test_modular_race_class_contract.py \
        scripts/test_promote_modular_race_class.py
git commit -m 'feat: validate modular race-class promotion (#80)'
```

---

### Task 4: One rig-family socket, visual evidence, and provider outputs

**Files:**
- Modify: `scripts/build_modular_race_class.py`
- Modify: `scripts/promote_modular_race_class.py`
- Modify: `scripts/test_promote_modular_race_class.py`
- Create: `evidence/80-modular-elf-fighter/README.md`
- Create: `evidence/80-modular-elf-fighter/verification.json`
- Create: `evidence/80-modular-elf-fighter/elf-fighter-idle-walk.png`
- Create: `evidence/80-modular-elf-fighter/elf-fighter-atlas-a-b.png`
- Create: `evidence/80-modular-elf-fighter/elf-fighter-longsword.png`
- Modify: `README.md`
- Modify generated: `harness/models/synty/mesh-stats.json`
- Modify generated: `harness/catalogs/synty-complete-inventory.json`

**Interfaces:**
- Produces one accepted socket object `{id, socket:{bone,boneUnitMeters,positionMeters,rotationQuaternion,scale}}` in `verification.json` and the generated runtime manifest.
- Web Task 8 copies this exact profile by value and locks it in a unit test.

- [ ] **Step 1: Add a failing socket-report test**

```python
def test_socket_report_accepts_exactly_one_rig_family_profile(self):
    report = load_socket_report(self.socket_report_path)
    self.assertEqual("modular-fantasy-hero-v1", report["rigFamily"])
    self.assertIn(report["acceptedProfile"]["id"], {
        "townfolk-main-hand-v1",
        "modular-fantasy-hero-main-hand-v1",
    })
    socket = report["acceptedProfile"]["socket"]
    self.assertEqual("Hand_R", socket["bone"])
    self.assertEqual(0.01, socket["boneUnitMeters"])
    self.assertTrue(validate_socket(socket))
    self.assertEqual(["Idle_Relaxed", "Walk_Forward"], report["clips"])
```

Require one representative weapon (`longsword.glb`), sampled frame facts for both clips, finite matrices, and no per-item correction map.

- [ ] **Step 2: Run focused tests and verify RED**

```bash
python3 -m unittest scripts.test_promote_modular_race_class -v
```

Expected: FAIL because socket reporting is missing.

- [ ] **Step 3: Measure Townfolk reuse before calibrating anything**

Run socket mode with the current accepted profile from `harness/models/synty/weapons/manifest.json`:

```bash
blender --background --python scripts/build_modular_race_class.py -- \
  --socket-report \
  --character .stage/modular-race-class/elf-fighter.glb \
  --townfolk-character harness/models/synty/characters/fighter.glb \
  --weapon harness/models/synty/weapons/longsword.glb \
  --weapon-manifest harness/models/synty/weapons/manifest.json \
  --out-dir .stage/modular-race-class/socket
```

The script computes the desired body-space weapon matrix from Townfolk and compares applying `townfolk-main-hand-v1` to the modular hand across both clips. It renders frames 2/mid/end for idle and walk.

If the matrices and renders pass, accept `townfolk-main-hand-v1`. If not, derive one modular profile at idle frame 2 with:

```python
modular_socket_matrix = modular_hand_matrix.inverted() @ desired_weapon_matrix
```

Decompose it into position, normalized quaternion, and uniform scale, rerun every sampled frame, and accept `modular-fantasy-hero-main-hand-v1` only when the weapon remains rigidly gripped and correctly oriented through both clips. This evidence-driven branch is the design's one allowed socket decision; neither branch creates per-item data.

- [ ] **Step 4: Apply the accepted provider output and regenerate catalogs**

```bash
python3 scripts/promote_modular_race_class.py validate \
  --recipe scripts/configs/character-promotion/modular-race-class-v1.json \
  --source-root "$SYNTY_SUBSCRIPTION_ROOT"
python3 scripts/promote_modular_race_class.py build \
  --recipe scripts/configs/character-promotion/modular-race-class-v1.json \
  --mapping scripts/configs/animation/townfolk-to-modular-fantasy-hero-v1.json \
  --source-root "$SYNTY_SUBSCRIPTION_ROOT" \
  --stage-root .stage/modular-race-class
python3 scripts/promote_modular_race_class.py apply \
  --recipe scripts/configs/character-promotion/modular-race-class-v1.json \
  --stage-root .stage/modular-race-class
python3 scripts/build_mesh_stats.py
python3 scripts/build_synty_complete_inventory.py
python3 scripts/promote_modular_race_class.py check \
  --recipe scripts/configs/character-promotion/modular-race-class-v1.json \
  --mapping scripts/configs/animation/townfolk-to-modular-fantasy-hero-v1.json \
  --source-root "$SYNTY_SUBSCRIPTION_ROOT"
```

Expected: only default Elf Fighter GLB and generated manifest enter `harness`; alternate atlas and raw source remain under ignored `.stage`.

- [ ] **Step 5: Write concise evidence and provider documentation**

`verification.json` records archive/member/animation/output/manifest hashes, structural readback, non-image atlas semantic hash, socket profile and sampled matrices, and exact evidence paths. The README records Kirk's eventual visual verdict verbatim and states that color selection, downed/portrait, additional combinations, combat animation, and Auto Rig Pro are deferred.

- [ ] **Step 6: Run the provider release gate**

```bash
python3 -m unittest scripts.test_modular_race_class_contract -v
python3 -m unittest scripts.test_promote_modular_race_class -v
python3 -m unittest discover -s scripts -p 'test_*.py'
python3 scripts/promote_modular_race_class.py check \
  --recipe scripts/configs/character-promotion/modular-race-class-v1.json \
  --mapping scripts/configs/animation/townfolk-to-modular-fantasy-hero-v1.json \
  --source-root "$SYNTY_SUBSCRIPTION_ROOT"
python3 scripts/build_synty_complete_inventory.py --check
git diff --check
if rg -n '/home/|/tmp/|Downloads/synty' \
  scripts/configs/character-promotion/modular-race-class-v1.json \
  harness/models/synty/characters/race-class \
  evidence/80-modular-elf-fighter README.md; then exit 1; fi
if git status --porcelain | grep -E '\.(fbx|blend|zip|unitypackage)$'; then exit 1; fi
```

Expected: all gates pass; no private absolute path or raw source is tracked.

- [ ] **Step 7: Commit the promoted provider release**

```bash
git add README.md \
        scripts/build_modular_race_class.py \
        scripts/promote_modular_race_class.py \
        scripts/test_promote_modular_race_class.py \
        harness/models/synty/characters/race-class \
        harness/models/synty/mesh-stats.json \
        harness/catalogs/synty-complete-inventory.json \
        evidence/80-modular-elf-fighter
git commit -m 'asset: promote modular Elf Fighter (#80)'
```

---

### Task 5: Publish and close the provider review round

**Files:**
- No new product files; valid review fixes stay on `asset/80-modular-elf-fighter`.

**Interfaces:**
- Produces the provider PR URL, final branch commit, generated manifest SHA-256, output SHA-256, and accepted socket profile consumed by web Tasks 6–9.

- [ ] **Step 1: Review the whole provider branch once**

Review `origin/main...HEAD` for source-path leakage, source/member hash enforcement, exact mesh selection, action transfer, root/forward scale, one-image atlas semantics, socket-family scope, generated inventory, and absence of runtime customization. Fix only valid findings, then rerun Task 4 Step 6 once.

- [ ] **Step 2: Push and create the provider PR**

```bash
manifest=harness/models/synty/characters/race-class/manifest.json
model=harness/models/synty/characters/race-class/elf-fighter.glb
manifest_sha=$(sha256sum "$manifest" | cut -d' ' -f1)
model_sha=$(sha256sum "$model" | cut -d' ' -f1)
socket_id=$(jq -r '.combinations["elf:fighter"].socketProfile' "$manifest")
cat > /tmp/rpg-game-assets-80-pr.md <<EOF
## Summary

- generate the fixed-look animated Modular Fantasy Hero Elf Fighter from one hash-bound recipe
- promote one embedded 1024² default atlas and prove alternate-atlas replacement preserves non-image semantics
- publish one accepted ${socket_id} rig-family socket profile

## Provider identity

- Manifest SHA-256: ${manifest_sha}
- Elf Fighter SHA-256: ${model_sha}
- Runtime path: harness/models/synty/characters/race-class/elf-fighter.glb

## Verification

- focused modular contract/promotion tests
- full provider Python suite
- deterministic promotion check
- complete-inventory check
- sampled idle/walk, atlas, and longsword evidence under evidence/80-modular-elf-fighter/

## Boundaries

No raw source, selectable customization, runtime assembly, modular downed/portrait, additional combinations, or Auto-Rig Pro dependency.

Closes #80
Design: KirkDiggler/rpg-project#321
Journey: KirkDiggler/rpg-project#320

— assets agent, on behalf of KirkDiggler
EOF
git push -u origin asset/80-modular-elf-fighter
pr_url=$(gh pr create \
  --repo KirkDiggler/rpg-game-assets \
  --base main \
  --head asset/80-modular-elf-fighter \
  --title 'asset: generate fixed-look modular Elf Fighter' \
  --body-file /tmp/rpg-game-assets-80-pr.md)
printf '%s\n' "$pr_url"
```

- [ ] **Step 3: Request exactly one Copilot review and verify by readback**

```bash
pr_number=${pr_url##*/}
pr_id=$(gh pr view "$pr_number" --repo KirkDiggler/rpg-game-assets --json id --jq .id)
gh api graphql \
  -f query='mutation($id:ID!){requestReviews(input:{pullRequestId:$id,botIds:["BOT_kgDOCnlnWA"]}){pullRequest{id}}}' \
  -f id="$pr_id" >/dev/null
gh api graphql \
  -f query='query($id:ID!){node(id:$id){... on PullRequest{reviewRequests(first:100){nodes{requestedReviewer{... on Bot{id login}}}}}}}' \
  -f id="$pr_id" \
  --jq '.data.node.reviewRequests.nodes[].requestedReviewer | select(.id=="BOT_kgDOCnlnWA")'
```

Expected: readback prints the Copilot bot id/login. Do not report readiness in the same turn as the request.

- [ ] **Step 4: Answer every review thread and rerun the release gate**

```bash
R=repos/KirkDiggler/rpg-game-assets/pulls/${pr_url##*/}/comments
echo "open threads:  $(gh api "$R" --jq '[.[]|select(.in_reply_to_id==null)]|length')"
echo "with a reply:  $(gh api "$R" --jq '[.[]|select(.in_reply_to_id!=null)]|length')"
```

Apply valid findings on the same branch, reply with commit ids, explain declines, never re-request Copilot, and require equal counts before reporting ready.

- [ ] **Step 5: Record the provider handoff without inventing a merge commit**

Report the PR URL, current head commit, manifest/output hashes, and accepted socket profile. If Kirk has not merged it, state that web development may use the explicit provider worktree but final web receipt/merge remains blocked on the real provider merge commit.

---

### Task 6: Typed composite player-model resolution

**Files:**
- Modify: `src/components/hex-grid/classCharacterModels.ts`
- Modify: `src/components/hex-grid/classCharacterModels.test.ts`

**Interfaces:**
- Produces `CharacterRigFamily`, `PlayerCharacterModelResolution`, and `resolvePlayerCharacterModel(raceRefId, classRefId, isDowned)`.
- Keeps `resolveClassCharacterModelUrl` as the existing class-only wrapper for non-production callers while `HexEntity` moves to the typed resolver.
- Tasks 7 and 8 consume `resolution.url` and `resolution.rigFamily`.

- [ ] **Step 1: Create the isolated web worktree and mark issue #849 In Progress**

```bash
web_repo=$HOME/game-dev/rpg-dnd5e-web
git -C "$web_repo" fetch origin
worktree=$HOME/.pi/worktrees/rpg-dnd5e-web/849-modular-elf-fighter
git -C "$web_repo" worktree add "$worktree" -b feat/849-race-class-models origin/dev
cd "$worktree"
gh issue view 849 --repo KirkDiggler/rpg-dnd5e-web --json state,url,projectItems
gh project item-edit \
  --id PVTI_lAHOAASbwc4Bcj4vzg4f5Rg \
  --project-id PVT_kwHOAASbwc4Bcj4v \
  --field-id PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM \
  --single-select-option-id a434eab1
npm run test:run -- src/components/hex-grid/classCharacterModels.test.ts
```

Expected: branch starts at `origin/dev`; issue #849 is open/tracked; baseline resolver tests pass.

- [ ] **Step 2: Replace class-only expectations with failing typed-resolution tests**

```typescript
it('resolves the exact standing Elf Fighter before class fallback', () => {
  expect(resolvePlayerCharacterModel('elf', 'fighter', false)).toEqual({
    url: '/models/synty/characters/race-class/elf-fighter.glb',
    rigFamily: 'modular-fantasy-hero-v1',
    source: 'race-class',
  });
});

it('uses the Townfolk Fighter downed fallback for a downed Elf Fighter', () => {
  expect(resolvePlayerCharacterModel('elf', 'fighter', true)).toEqual({
    url: '/models/synty/characters/fighter-downed.glb',
    rigFamily: 'townfolk-v1',
    source: 'class',
  });
});

it('uses class fallback for combinations not yet promoted', () => {
  expect(resolvePlayerCharacterModel('dwarf', 'fighter', false)).toEqual({
    url: '/models/synty/characters/fighter.glb',
    rigFamily: 'townfolk-v1',
    source: 'class',
  });
});
```

Also pin trimmed/case-insensitive exact refs, missing race with known class, unknown class, and empty refs.

- [ ] **Step 3: Run focused tests and verify RED**

```bash
npm run test:run -- src/components/hex-grid/classCharacterModels.test.ts
```

Expected: FAIL because `resolvePlayerCharacterModel` and types do not exist.

- [ ] **Step 4: Implement the typed resolver**

```typescript
export type CharacterRigFamily =
  | 'townfolk-v1'
  | 'modular-fantasy-hero-v1';

export interface PlayerCharacterModelResolution {
  url: string;
  rigFamily: CharacterRigFamily;
  source: 'race-class' | 'class';
}

const RACE_CLASS_CHARACTER_MODELS = {
  'elf:fighter': {
    model: 'race-class/elf-fighter.glb',
    rigFamily: 'modular-fantasy-hero-v1',
  },
} as const;
```

Normalize refs independently. Skip exact entries when `isDowned`; then call the existing class lookup and return `townfolk-v1`. Return `undefined` only when class lookup also fails.

- [ ] **Step 5: Run focused tests and commit**

```bash
npm run test:run -- src/components/hex-grid/classCharacterModels.test.ts
npm run typecheck
git add src/components/hex-grid/classCharacterModels.ts \
        src/components/hex-grid/classCharacterModels.test.ts
git commit -m 'feat: resolve exact race-class character models (#849)'
```

---

### Task 7: Thread public race identity through local and peer rendering

**Files:**
- Modify: `src/components/session/SessionEncounterView.tsx`
- Modify: `src/components/session/SessionEncounterView.test.tsx`
- Modify: `src/components/session/SessionCanvas.tsx`
- Modify: `src/components/session/SessionCanvas.test.tsx`
- Modify: `src/components/hex-grid/HexEntity.tsx`

**Interfaces:**
- Adds `raceRefId?: string` beside `classRefId` on `SessionCanvasProps` and `HexEntityProps`.
- Local source is `ownRoster.raceRef`; peer source is `roster.get(member.subject)?.raceRef`.
- `HexEntity` consumes both through Task 6's resolver.

- [ ] **Step 1: Add failing local public-authority tests**

Extend the existing `uses public roster identity/body` test:

```typescript
expect(hoisted.lastCanvasProps.current?.characterName).toBe('Aldric');
expect(hoisted.lastCanvasProps.current?.classRefId).toBe('fighter');
expect(hoisted.lastCanvasProps.current?.raceRefId).toBe('elf');
```

Make the private `CharacterData` fixture disagree with both (`wizard`, `human`) so the test proves private data does not choose the model.

- [ ] **Step 2: Add failing visible-peer exact-resolution and missing-row tests**

In `SessionCanvas.test.tsx`, add a local `SessionScene` case with `raceRefId="elf"` and `classRefId="fighter"` and assert the mocked mesh URL contains `/race-class/elf-fighter.glb`. Change Bob's roster race to `elf` and assert the same exact URL for the visible peer. Keep the missing-roster test and assert neither exact nor class URL mounts. Preserve the existing model-load ErrorBoundary test and rerun it with the exact Elf Fighter URL to prove a failed exact asset still mounts `MediumHumanoid`.

- [ ] **Step 3: Run focused tests and verify RED**

```bash
npm run test:run -- \
  src/components/session/SessionEncounterView.test.tsx \
  src/components/session/SessionCanvas.test.tsx
```

Expected: FAIL because race is not passed and peer resolution still uses class only.

- [ ] **Step 4: Implement the public race data flow**

In `SessionEncounterView`:

```typescript
const ownRoster = roster.get(member);
const characterName = ownRoster?.name || 'You';
const classRefId = ownRoster?.classRef || undefined;
const raceRefId = ownRoster?.raceRef || undefined;
```

Pass `raceRefId` into `SessionCanvas`. In `SessionCanvas`, pass local `raceRefId` and peer `roster?.get(member.subject)?.raceRef || undefined` into `HexEntity`. In `HexEntity`, replace class URL resolution with `resolvePlayerCharacterModel(raceRefId, classRefId, isDowned)` and use its URL. Preserve monster resolution unchanged.

- [ ] **Step 5: Run focused tests, typecheck, and commit**

```bash
npm run test:run -- \
  src/components/hex-grid/classCharacterModels.test.ts \
  src/components/session/SessionEncounterView.test.tsx \
  src/components/session/SessionCanvas.test.tsx
npm run typecheck
git add src/components/session/SessionEncounterView.tsx \
        src/components/session/SessionEncounterView.test.tsx \
        src/components/session/SessionCanvas.tsx \
        src/components/session/SessionCanvas.test.tsx \
        src/components/hex-grid/HexEntity.tsx
git commit -m 'feat: render public race and class identity (#849)'
```

---

### Task 8: Apply exactly one modular rig-family socket override

**Files:**
- Modify: `src/components/hex-grid/mainHandWeapons.ts`
- Modify: `src/components/hex-grid/mainHandWeapons.test.ts`
- Modify: `src/components/hex-grid/ClassCharacterModel.tsx`
- Modify: `src/components/hex-grid/ClassCharacterModel.test.tsx`
- Modify: `src/components/hex-grid/HexEntity.tsx`

**Interfaces:**
- Produces `mainHandSocketForRigFamily(rigFamily) -> MainHandSocket`.
- `ClassCharacterModelProps` gains `mainHandSocketOverride?: MainHandSocket`.
- Consumes the exact provider socket object from Task 5; Townfolk behavior remains object-identical to `TOWNFOLK_MAIN_HAND_SOCKET`.

- [ ] **Step 1: Add failing socket-family tests from the provider manifest**

After the provider branch emits its accepted manifest, copy its socket object byte-for-byte into `MODULAR_FANTASY_HERO_MAIN_HAND_SOCKET`. Add tests that parse the provider manifest from the explicit local provider worktree during development and deep-equal the TypeScript constant, then pin:

```typescript
expect(mainHandSocketForRigFamily('townfolk-v1')).toBe(
  TOWNFOLK_MAIN_HAND_SOCKET
);
expect(mainHandSocketForRigFamily('modular-fantasy-hero-v1')).toBe(
  MODULAR_FANTASY_HERO_MAIN_HAND_SOCKET
);
```

The committed unit test embeds the exact accepted values so CI needs no private provider checkout.

- [ ] **Step 2: Add a failing component override test**

Render `ClassCharacterModel` with a Townfolk socket in `mainHandPresentation` and a distinct valid `mainHandSocketOverride`. Assert the attached clone's local position/quaternion/scale equal the override, while the original presentation object remains unchanged.

- [ ] **Step 3: Run focused tests and verify RED**

```bash
npm run test:run -- \
  src/components/hex-grid/mainHandWeapons.test.ts \
  src/components/hex-grid/ClassCharacterModel.test.tsx
```

Expected: FAIL because the rig-family lookup and override prop do not exist.

- [ ] **Step 4: Implement immutable socket override composition**

In `ClassCharacterModel`:

```typescript
const effectiveMainHandPresentation = useMemo(
  () =>
    mainHandPresentation && mainHandSocketOverride
      ? { ...mainHandPresentation, socket: mainHandSocketOverride }
      : mainHandPresentation,
  [mainHandPresentation, mainHandSocketOverride]
);
```

Pass the effective presentation to `MainHandAttachmentSlot`. In `HexEntity`, derive the override from `playerModelResolution?.rigFamily`; class/downed fallback naturally selects Townfolk.

- [ ] **Step 5: Run focused tests, typecheck, and commit**

```bash
npm run test:run -- \
  src/components/hex-grid/classCharacterModels.test.ts \
  src/components/hex-grid/mainHandWeapons.test.ts \
  src/components/hex-grid/ClassCharacterModel.test.tsx \
  src/components/session/SessionCanvas.test.tsx
npm run typecheck
git add src/components/hex-grid/mainHandWeapons.ts \
        src/components/hex-grid/mainHandWeapons.test.ts \
        src/components/hex-grid/ClassCharacterModel.tsx \
        src/components/hex-grid/ClassCharacterModel.test.tsx \
        src/components/hex-grid/HexEntity.tsx
git commit -m 'feat: apply modular rig hand socket (#849)'
```

---

### Task 9: Sync exact provider bytes and prove the production route

**Files:**
- Create: `docs/evidence/849-modular-elf-fighter/README.md`
- Create: `docs/evidence/849-modular-elf-fighter/receipt.json`
- Create: `docs/evidence/849-modular-elf-fighter/*.png`
- Synced but ignored: `public/models/synty/characters/race-class/elf-fighter.glb`
- Synced but ignored: `public/models/synty/characters/race-class/manifest.json`

**Interfaces:**
- Produces exact merged provider commit/manifest/output hashes and visual/browser facts used in the web PR and final journey walk.

- [ ] **Step 1: Develop against the explicit provider worktree without inventing a merge hash**

Before provider merge:

```bash
RPG_GAME_ASSETS_PATH=$HOME/.pi/worktrees/rpg-game-assets/80-modular-elf-fighter \
  npm run assets:sync
sha256sum public/models/synty/characters/race-class/{manifest.json,elf-fighter.glb}
```

Do not commit a receipt naming a provider merge until GitHub proves PR #80 merged. The code/tests may proceed against these exact branch bytes.

- [ ] **Step 2: Add an asset-load smoke and verify exact synced paths**

Run the focused R3F tests, then serve a production-development build and request both the class fallback and exact model paths. Require HTTP 200 for the exact GLB and no console/page errors.

- [ ] **Step 3: Run the real route on a parallel lab**

Use a unique dev identity and lab endpoint so concurrent weapon work remains untouched:

```bash
cd "$HOME/game-dev/rpg-deployment"
docker compose -f docker-compose.local-dev.yml \
               -f docker-compose.local-lab.yml up -d rpg-api-lab2 envoy-lab2
cd "$HOME/.pi/worktrees/rpg-dnd5e-web/849-modular-elf-fighter"
VITE_API_HOST=http://localhost:8082 \
VITE_DEV_PLAYER_ID=modular-elf-fighter-849 \
  npm run dev -- --port 3011
```

Create/select an Elf Fighter through the real character flow, enter a session, and capture:

1. stationary `Idle_Relaxed`;
2. real hex movement using `Walk_Forward`;
3. authoritative main-hand weapon attached;
4. downed fixture/path resolving current Fighter downed; and
5. a non-promoted race+class retaining class fallback.

Use `domcontentloaded`, not `networkidle`, for browser capture because the session stream remains open.

- [ ] **Step 4: After provider merge, sync and record exact authority**

```bash
provider_repo=$HOME/game-dev/rpg-game-assets
git -C "$provider_repo" fetch origin
provider_pr=$(gh pr list --repo KirkDiggler/rpg-game-assets \
  --head asset/80-modular-elf-fighter --state merged --json number --jq '.[0].number')
test -n "$provider_pr"
provider_commit=$(gh pr view "$provider_pr" --repo KirkDiggler/rpg-game-assets \
  --json state,mergeCommit --jq 'select(.state=="MERGED") | .mergeCommit.oid')
test -n "$provider_commit"
git -C "$provider_repo" merge-base --is-ancestor "$provider_commit" origin/main
provider_stage=$(mktemp -d)
git -C "$provider_repo" worktree add --detach "$provider_stage" "$provider_commit"
trap 'git -C "$provider_repo" worktree remove --force "$provider_stage"' EXIT
RPG_GAME_ASSETS_PATH="$provider_stage" npm run assets:sync
manifest_sha=$(sha256sum public/models/synty/characters/race-class/manifest.json | cut -d' ' -f1)
model_sha=$(sha256sum public/models/synty/characters/race-class/elf-fighter.glb | cut -d' ' -f1)
printf '%s %s %s\n' "$provider_commit" "$manifest_sha" "$model_sha"
```

Generate `receipt.json` from the merged bytes and captured browser facts with this exact portable shape:

```bash
socket_profile=$(jq -r '.combinations["elf:fighter"].socketProfile' \
  public/models/synty/characters/race-class/manifest.json)
export PROVIDER_COMMIT="$provider_commit" PROVIDER_MANIFEST_SHA="$manifest_sha" \
  ELF_FIGHTER_SHA="$model_sha" SOCKET_PROFILE="$socket_profile"
python3 - <<'PY'
import json, os, re
from pathlib import Path

if not re.fullmatch(r"[0-9a-f]{40}", os.environ["PROVIDER_COMMIT"]):
    raise SystemExit("invalid provider merge commit")
for name in ("PROVIDER_MANIFEST_SHA", "ELF_FIGHTER_SHA"):
    if not re.fullmatch(r"[0-9a-f]{64}", os.environ[name]):
        raise SystemExit(f"invalid SHA-256 in {name}")
value = {
    "schemaVersion": 1,
    "providerCommit": os.environ["PROVIDER_COMMIT"],
    "providerManifestSha256": os.environ["PROVIDER_MANIFEST_SHA"],
    "elfFighterSha256": os.environ["ELF_FIGHTER_SHA"],
    "socketProfile": os.environ["SOCKET_PROFILE"],
    "modelUrl": "/models/synty/characters/race-class/elf-fighter.glb",
    "animations": ["Idle_Relaxed", "Walk_Forward"],
    "browser": {
        "modelRequestCount": 1,
        "modelResponseStatus": 200,
        "consoleErrors": 0,
        "pageErrors": 0,
        "unexpectedRequestFailures": 0,
    },
}
path = Path("docs/evidence/849-modular-elf-fighter/receipt.json")
path.parent.mkdir(parents=True, exist_ok=True)
path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")
PY
```

Reject the receipt if the socket id differs from the merged provider manifest or if captured browser counts differ from the values above; it contains no absolute local path.

- [ ] **Step 5: Run the web release gate**

```bash
npm run test:run
npm run ci-check
git diff --check
if rg -n '/home/|/tmp/|Downloads/synty' docs/evidence/849-modular-elf-fighter; then exit 1; fi
if git status --porcelain | grep -E '\.(glb|fbx|blend|zip|unitypackage)$'; then exit 1; fi
```

Expected: all tests/checks pass and no licensed binary is tracked.

- [ ] **Step 6: Commit public-safe evidence**

```bash
git add docs/evidence/849-modular-elf-fighter
git commit -m 'docs(evidence): prove modular Elf Fighter route (#849)'
```

---

### Task 10: Publish web, complete reviews, and close the design record last

**Files:**
- Valid review fixes remain on `feat/849-race-class-models`.
- Modify after both implementations: `ideas/characters/modular-race-class/design.md` only to add final implementation pointers if the approved contract did not change.

**Interfaces:**
- Produces the merge-ready web PR and final A evidence; journey #320 remains open for B.

- [ ] **Step 1: Review the whole web branch once**

Review `origin/dev...HEAD` for public roster authority, exact normalized lookup, downed/class/placeholder fallbacks, no client rule inference, socket-family scope, unchanged Townfolk weapon behavior, ignored asset boundary, and test/evidence honesty. Apply valid fixes and rerun Task 9 Step 5 once.

- [ ] **Step 2: Push and create the web PR**

```bash
provider_commit=$(jq -r .providerCommit docs/evidence/849-modular-elf-fighter/receipt.json)
manifest_sha=$(jq -r .providerManifestSha256 docs/evidence/849-modular-elf-fighter/receipt.json)
model_sha=$(jq -r .elfFighterSha256 docs/evidence/849-modular-elf-fighter/receipt.json)
cat > /tmp/rpg-dnd5e-web-849-pr.md <<EOF
## Summary

- resolve standing player models by exact public race + class before class fallback
- render the provider Elf Fighter for local and roster-visible players
- apply the provider-approved modular rig socket while preserving Townfolk defaults

## Provider identity

- Provider merge: ${provider_commit}
- Manifest SHA-256: ${manifest_sha}
- Elf Fighter SHA-256: ${model_sha}

## Verification

- focused resolver, roster, model, and socket tests
- npm run test:run
- npm run ci-check
- real session-route idle, walk, authoritative main-hand, downed fallback, and class fallback evidence

## Deferred

Selectable customization, runtime assembly/shaders, modular downed/portrait, additional combinations, and Auto-Rig Pro.

Closes #849
Provider: KirkDiggler/rpg-game-assets#80
Design: KirkDiggler/rpg-project#321
Journey: KirkDiggler/rpg-project#320

— assets agent, on behalf of KirkDiggler
EOF
git push -u origin feat/849-race-class-models
pr_url=$(gh pr create \
  --repo KirkDiggler/rpg-dnd5e-web \
  --base dev \
  --head feat/849-race-class-models \
  --title 'feat: resolve player models by public race and class' \
  --body-file /tmp/rpg-dnd5e-web-849-pr.md)
printf '%s\n' "$pr_url"
```

- [ ] **Step 3: Request exactly one Copilot review and close every thread**

```bash
pr_number=${pr_url##*/}
pr_id=$(gh pr view "$pr_number" --repo KirkDiggler/rpg-dnd5e-web --json id --jq .id)
gh api graphql \
  -f query='mutation($id:ID!){requestReviews(input:{pullRequestId:$id,botIds:["BOT_kgDOCnlnWA"]}){pullRequest{id}}}' \
  -f id="$pr_id" >/dev/null
gh api graphql \
  -f query='query($id:ID!){node(id:$id){... on PullRequest{reviewRequests(first:100){nodes{requestedReviewer{... on Bot{id login}}}}}}}' \
  -f id="$pr_id" \
  --jq '.data.node.reviewRequests.nodes[].requestedReviewer | select(.id=="BOT_kgDOCnlnWA")'
R=repos/KirkDiggler/rpg-dnd5e-web/pulls/$pr_number/comments
echo "open threads:  $(gh api "$R" --jq '[.[]|select(.in_reply_to_id==null)]|length')"
echo "with a reply:  $(gh api "$R" --jq '[.[]|select(.in_reply_to_id!=null)]|length')"
```

Expected: bot request appears in readback. Do not report readiness in that turn. Apply/answer every finding on the same branch, never re-request, and require equal root/reply counts before readiness.

- [ ] **Step 4: Obtain Kirk's integrated A verdict**

Kirk walks the real Elf Fighter route and judges class silhouette, ears/head, idle, walk, weapon grip, scale, and fallback behavior. Record the verdict verbatim in web evidence and provider evidence if it changes the provider acceptance statement.

- [ ] **Step 5: Merge inside-out and update the design tracking surface**

Kirk merges provider PR first, then web PR to `dev`. Verify both GitHub merge commits. Update design PR #322 with links, commits, manifest/output hashes, socket id, and integrated verdict. Merge design PR #322 last; close design issue #321. Keep journey #320 open and create the next provider Build slice for the three remaining Elf class recipes only after A is accepted.

- [ ] **Step 6: Remove only proven-merged worktrees**

After merge readback:

```bash
git -C "$HOME/game-dev/rpg-game-assets" worktree remove \
  "$HOME/.pi/worktrees/rpg-game-assets/80-modular-elf-fighter"
git -C "$HOME/game-dev/rpg-dnd5e-web" worktree remove \
  "$HOME/.pi/worktrees/rpg-dnd5e-web/849-modular-elf-fighter"
```

Do not remove the design worktree until PR #322 is also proven merged. Do not touch the separate #73 race-catalog or #846 weapon worktrees.
