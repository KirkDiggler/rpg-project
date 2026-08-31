# Owner-Authoritative Off-Hand Presentation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Show the acting character's existing authoritative shield or supported off-hand weapon beneath `Hand_L`, without changing equipment gameplay or existing main-hand presentation.

**Architecture:** A decision-first provider Learn selects one canonical Shield, measures two rig-family sockets, and separates shared-socket motion from item-local normalization. A dedicated strict off-hand release promotes Shield plus evidence-proven Handaxe/Sickle left-hand variants, reuses canonical Dagger/Shortsword bytes, and preserves every existing character/main-hand weapon hash. The web projects only this reviewed provider catalog from owner-private `equipped.off_hand` through a generic bone-attachment core.

**Tech Stack:** Python 3, `unittest`, Blender 5 background Python, glTF 2.0/GLB, Pillow, React 19, TypeScript, Three.js/R3F/drei, Vitest, Playwright, Git/GitHub CLI.

**Spec:** `ideas/characters/off-hand-presentation/design.md`

## Global Constraints

- This is presentation-only Assets work: no API, proto, toolkit, inventory, equipment-rule, armor-class, damage, persistence, or fixture code changes.
- Exact authority is existing owner-private `CharacterData.equipped.off_hand`; peer equipment remains unprojected.
- Shield and off-hand weapons use `Hand_L`; main-hand weapons and existing `Hand_R` contracts remain unchanged.
- Exactly one reusable left-hand socket is allowed for `townfolk-v1` and one for `modular-fantasy-hero-v1`; no item, class, or race correction table.
- The provider catalog contains exact Shield, Dagger, Shortsword, Handaxe, and Sickle refs. Dagger/Shortsword reuse canonical bytes; Handaxe/Sickle receive deterministic left-hand variants under `harness/models/synty/off-hand/`.
- Strict generated GLBs: glTF 2.0, identity static root, true meters, POSITION/NORMAL/TEXCOORD_0/TANGENT, one source-compatible 1024² atlas, base RGBA8 ≤ 4.5 MiB, no skin/animation/camera/light.
- Provider releases require clean tracked HEAD, sealed hash-bound inputs, validate-before-apply, rollback, transactional apply, and clean-head byte-for-byte rebuild.
- Licensed source paths, provenance, derivatives, and GLBs remain private to `rpg-game-assets`; web tracks no licensed GLBs.
- Use isolated worktrees, TDD, PR-only integration, fresh independent review, and publish the current-head verdict directly on every substantive PR.
- Copilot remains disabled until Kirk explicitly re-enables it.
- Human visual gates are blocking: do not invent Kirk's shield choice, canonical dimensions, grip transform, or socket values.

---

## Delivery gates

This plan has three sequential merge gates:

1. **Learn gate:** `rpg-game-assets#102` merges accepted Shield, exact two-profile socket receipts, Dagger/Shortsword reuse evidence, and Handaxe/Sickle variant transforms without changing runtime outputs.
2. **Provider gate:** a newly filed provider Build issue consumes the exact merged Learn receipt and merges the strict off-hand catalog/GLBs.
3. **Web gate:** a newly filed web issue consumes the exact merged provider commit and proves owner-authoritative Shield and two-weapon presentation.

Do not file the Build issue before the Learn verdict. Do not file the web issue before the provider merge. A later phase reads exact values from the prior merged receipt; it never substitutes values written in this plan.

---

### Task 1: Deterministic Shield Candidate Inventory

**Files:**
- Create: `rpg-game-assets/evidence/102-off-hand-shield-candidates/candidate-query.json`
- Create: `rpg-game-assets/evidence/102-off-hand-shield-candidates/source-inventory.json`
- Create: `rpg-game-assets/evidence/102-off-hand-shield-candidates/review-config.json`
- Create: `rpg-game-assets/scripts/test_off_hand_shield_candidate_review.py`
- Reuse: `rpg-game-assets/scripts/index_weapon_source_archives.py`
- Reuse: `rpg-game-assets/scripts/weapon_candidate_review.py`

**Interfaces:**
- Consumes: licensed archive root from `RPG_SYNTY_ARCHIVE_ROOT`; exact ref `dnd5e:item:shield`.
- Produces: schema-v2 candidate inventory and review config with all candidates initially `pending`, exact source/atlas hashes, and no absolute paths.

- [ ] **Step 1: Create the provider worktree and record baseline hashes**

```bash
repo=/home/kirk/game-dev/rpg-game-assets
wt=/home/kirk/.pi/worktrees/rpg-game-assets/102-off-hand-shield-candidates
git -C "$repo" fetch origin main --prune
git -C "$repo" worktree add -b learn/102-off-hand-shield-candidates "$wt" origin/main
cd "$wt"
git status --short --branch
sha256sum harness/models/synty/weapons/*.glb \
  harness/models/synty/characters/*.glb \
  harness/models/synty/characters/race-class/*.glb \
  > /tmp/rpg102-runtime-baseline.sha256
python3 -m unittest discover -s scripts -p 'test_*.py'
```

Expected: clean worktree; baseline suite passes with only established private-source skips.

- [ ] **Step 2: Write the failing candidate-contract test**

```python
# scripts/test_off_hand_shield_candidate_review.py
class OffHandShieldCandidateReviewTests(unittest.TestCase):
    def test_exact_shield_group_is_inventory_bound_and_unselected(self) -> None:
        query = json.loads((EVIDENCE / "candidate-query.json").read_text())
        review = json.loads((EVIDENCE / "review-config.json").read_text())
        self.assertEqual(["dnd5e:item:shield"], [row["ref"] for row in query["refs"]])
        self.assertEqual(["dnd5e:item:shield"], [row["ref"] for row in review["groups"]])
        self.assertGreaterEqual(len(review["checkpoints"]), 2)
        self.assertTrue(all(row["verdict"] == "pending" for row in review["checkpoints"]))
        self.assertTrue(all(row["displayGroup"] == "dnd5e:item:shield" for row in review["checkpoints"]))
```

- [ ] **Step 3: Run the test to verify RED**

Run:

```bash
python3 -m unittest scripts.test_off_hand_shield_candidate_review -v
```

Expected: FAIL because the evidence files do not exist.

- [ ] **Step 4: Create the exact query and generate inventory**

Use a schema-v2 query whose single ref searches shield/buckler/kite/tower/round names and excludes icons, sprites, UI, collisions, LOD-only files, characters, and animation files:

```json
{
  "schemaVersion": 2,
  "refs": [{
    "ref": "dnd5e:item:shield",
    "patterns": ["shield", "buckler", "kite", "tower"],
    "excludePatterns": ["icon", "sprite", "ui", "collision", "lod[1-9]", "character", "animation"],
    "excludePathPatterns": ["/UI/", "/Icons/", "/Animations/"]
  }],
  "supportMembers": [{
    "archive": "synty/POLYGON_Fantasy_Kingdom_SourceFiles_v5.zip",
    "member": "Source_Files/Textures/PolygonFantasyKingdom_Texture_01_A.png",
    "refs": ["dnd5e:item:shield"]
  }]
}
```

After the first broad inventory identifies candidate archives, add the exact source-compatible atlas member for every represented candidate pack, remove support entries for packs with no retained candidate, and regenerate inventory. The final test must assert every review checkpoint's `archive` has one hash-bound atlas support member.

Run:

```bash
mkdir -p evidence/102-off-hand-shield-candidates
python3 scripts/index_weapon_source_archives.py \
  --archives-root "$RPG_SYNTY_ARCHIVE_ROOT" \
  --recursive \
  --query evidence/102-off-hand-shield-candidates/candidate-query.json \
  --output evidence/102-off-hand-shield-candidates/source-inventory.json
```

- [ ] **Step 5: Convert complete candidates to private checkpoints**

For every complete source candidate, convert source bytes without semantic relabeling, record its exact source-compatible atlas, and populate the existing schema-v2 checkpoint fields. Set:

```json
{
  "ref": "dnd5e:item:shield",
  "displayGroup": "dnd5e:item:shield",
  "checkpointKind": "static-candidate",
  "promotionEligible": true,
  "verdict": "pending"
}
```

Reject rigged-only/incomplete sources from promotion eligibility rather than hiding their limitations.

- [ ] **Step 6: Run pure and private validation**

```bash
python3 -m unittest scripts.test_off_hand_shield_candidate_review -v
RPG_GAME_ASSETS_SOURCE_WORKSPACE=/home/kirk/game-dev \
  python3 -m unittest scripts.test_off_hand_shield_candidate_review -v
python3 scripts/index_weapon_source_archives.py \
  --archives-root "$RPG_SYNTY_ARCHIVE_ROOT" \
  --recursive \
  --query evidence/102-off-hand-shield-candidates/candidate-query.json \
  --output evidence/102-off-hand-shield-candidates/source-inventory.json \
  --check
```

Expected: PASS and byte-stable inventory regeneration.

- [ ] **Step 7: Commit inventory closure**

```bash
git add evidence/102-off-hand-shield-candidates scripts/test_off_hand_shield_candidate_review.py
git commit -m "learn: inventory off-hand shield candidates"
```

---

### Task 2: Make Candidate Evidence Bone-Neutral

**Files:**
- Modify: `rpg-game-assets/scripts/render_weapon_candidate_evidence.py`
- Modify: `rpg-game-assets/scripts/build_weapon_candidate_review_scene.py`
- Modify: `rpg-game-assets/scripts/test_render_weapon_candidate_evidence.py`
- Modify: `rpg-game-assets/scripts/test_build_weapon_candidate_review_scene.py`

**Interfaces:**
- Consumes: legacy receipts containing `sharedSocket`, or the new calibration receipt containing `socketProfiles`, selected by optional `--socket-profile`.
- Produces: evidence titles, scene metadata, and labels derived from the selected profile's actual bone; existing calls without `--socket-profile` remain unchanged.

- [ ] **Step 1: Write failing title/scene tests**

```python
def test_grip_title_names_actual_receipt_bone(self) -> None:
    self.assertEqual(
        "Off-hand Shield grip review — Fighter only; authored Blender Hand_L socket receipt",
        grip_sheet_title(self.plan, "Hand_L", "Off-hand Shield"),
    )

def test_held_scene_title_names_configured_bone(self) -> None:
    scene = held_scene_title("Shield", "Hand_L")
    self.assertIn("Hand_L socket", scene)
    self.assertNotIn("Hand_R", scene)


def test_named_socket_profile_selects_exact_left_hand_profile(self) -> None:
    bone, socket = read_authored_socket(self.multi_profile_receipt, "townfolk-off-hand-v1")
    self.assertEqual("Hand_L", bone)
    self.assertEqual(self.expected_authored_socket, socket)
```

- [ ] **Step 2: Run focused tests to verify RED**

```bash
python3 -m unittest \
  scripts.test_render_weapon_candidate_evidence \
  scripts.test_build_weapon_candidate_review_scene -v
```

Expected: FAIL because titles are hardcoded to `Hand_R`.

- [ ] **Step 3: Parameterize labels without changing transforms**

Implement:

```python
def grip_sheet_title(plan: dict[str, object], bone_name: str, title: str | None = None) -> str:
    prefix = title or str(plan["title"])
    return f"{prefix} grip review — Fighter only; authored Blender {bone_name} socket receipt"


def held_scene_title(group_title: str, bone_name: str) -> str:
    return f"{group_title} Held — Fighter + current preview transforms + authored Blender {bone_name} socket"
```

Add optional `--socket-profile` to both Blender CLIs. Implement profile selection while preserving legacy receipts:

```python
def read_authored_socket(receipt_path: Path, profile_id: str | None = None):
    receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
    if profile_id is None:
        selected = receipt["sharedSocket"]
    else:
        matches = [row for row in receipt["socketProfiles"] if row["id"] == profile_id]
        if len(matches) != 1:
            raise ValueError(f"socket profile must resolve exactly once: {profile_id}")
        selected = matches[0]
    return selected["bone"], selected["authoredBlenderBoneLocal"]
```

Read `bone_name` once and pass it into title construction. Do not alter `socket_matrix()`, `preview_matrix()`, or the existing main-hand coordinate math.

- [ ] **Step 4: Run focused and regression tests**

```bash
python3 -m unittest \
  scripts.test_render_weapon_candidate_evidence \
  scripts.test_build_weapon_candidate_review_scene \
  scripts.test_named_starter_weapon_roster \
  scripts.test_heavy_polearm_candidate_evidence -v
```

Expected: PASS; existing main-hand evidence plans still report `Hand_R`.

- [ ] **Step 5: Commit the renderer generalization**

```bash
git add scripts/render_weapon_candidate_evidence.py \
  scripts/build_weapon_candidate_review_scene.py \
  scripts/test_render_weapon_candidate_evidence.py \
  scripts/test_build_weapon_candidate_review_scene.py
git commit -m "refactor: derive candidate grip bone from receipt"
```

---

### Task 3: Measure and Validate the Two `Hand_L` Rig Profiles

**Files:**
- Create: `rpg-game-assets/scripts/measure_hand_socket_contract.py`
- Create: `rpg-game-assets/scripts/off_hand_socket_authority.py`
- Create: `rpg-game-assets/scripts/test_measure_hand_socket_contract.py`
- Create: `rpg-game-assets/scripts/test_off_hand_socket_authority.py`
- Create: `rpg-game-assets/evidence/102-off-hand-shield-candidates/socket-measurements.json`
- Test fixture only: `rpg-game-assets/scripts/fixtures/off-hand-socket-authority-valid.json`
- Test fixture only: `rpg-game-assets/scripts/fixtures/off-hand-socket-measurements-valid.json`

**Interfaces:**
- Produces `measure_hand_socket_contract.build_plan(repo_root) -> tuple[ModelProbe, ...]` covering 4 Townfolk and 28 modular standing models.
- Produces `off_hand_socket_authority.load_socket_authority(path, repo_root) -> OffHandSocketAuthority` with exact `townfolk-off-hand-v1` and `modular-fantasy-hero-off-hand-v1` profiles.

- [ ] **Step 1: Write failing plan and authority tests**

```python
def test_plan_covers_all_current_models(self) -> None:
    plan = build_plan(ROOT)
    self.assertEqual(32, len(plan))
    self.assertEqual(4, sum(row.rig_family == "townfolk-v1" for row in plan))
    self.assertEqual(28, sum(row.rig_family == "modular-fantasy-hero-v1" for row in plan))
    self.assertTrue(all(row.bone == "Hand_L" for row in plan))


def test_authority_requires_exact_two_profiles(self) -> None:
    authority = load_socket_authority(FIXTURE, ROOT)
    self.assertEqual(
        ("townfolk-off-hand-v1", "modular-fantasy-hero-off-hand-v1"),
        tuple(profile.id for profile in authority.profiles),
    )
    self.assertTrue(all(profile.bone == "Hand_L" for profile in authority.profiles))
```

- [ ] **Step 2: Run the tests to verify RED**

```bash
python3 -m unittest \
  scripts.test_measure_hand_socket_contract \
  scripts.test_off_hand_socket_authority -v
```

Expected: import/file-not-found failures.

- [ ] **Step 3: Implement exact model-plan discovery**

Define:

```python
@dataclass(frozen=True)
class ModelProbe:
    rig_family: str
    model_path: PurePosixPath
    model_sha256: str
    bone: str
    actions: tuple[str, ...]
    sample_frames: tuple[int, ...]
```

Townfolk paths are the four canonical class aliases. Modular paths and hashes come from `harness/models/synty/characters/race-class/manifest.json`; reject any combination count other than 28 and any unexpected rig family.

- [ ] **Step 4: Implement Blender measurement output**

The Blender CLI imports each exact model, samples `Idle_Relaxed` and `Walk_Forward`, records armature world, `Hand_L` rest, and posed-world matrices, and emits canonical JSON. It measures rig compatibility only; it does not invent socket calibration. The CLI must accept:

```text
--bone Hand_L
--output evidence/102-off-hand-shield-candidates/socket-measurements.json
```

The pure authority test uses `scripts/fixtures/off-hand-socket-authority-valid.json` with explicit nonidentity finite fixture values. Production calibration is created from Blender visual work in Task 4 and validated against these measurements.

- [ ] **Step 5: Implement strict authority validation**

Define:

```python
@dataclass(frozen=True)
class SocketProfile:
    id: str
    rig_family: str
    bone: str
    bone_unit_meters: float
    runtime_position_meters: tuple[float, float, float]
    runtime_rotation_xyzw: tuple[float, float, float, float]
    runtime_scale: float
    authored_position_meters: tuple[float, float, float]
    authored_rotation_xyzw: tuple[float, float, float, float]
    authored_scale: float


@dataclass(frozen=True)
class OffHandSocketAuthority:
    measurements_path: Path
    measurements_sha256: str
    profiles: tuple[SocketProfile, SocketProfile]
```

Validation requires finite values, positive scale/unit size, nonzero quaternion, `Hand_L`, exact profile IDs, exact 32 model hashes, required actions, and matrix-delta records for both rig families. Reject absolute paths and duplicate model/profile entries.

- [ ] **Step 6: Run unit tests and generate first measurement**

```bash
python3 -m unittest \
  scripts.test_measure_hand_socket_contract \
  scripts.test_off_hand_socket_authority -v
blender --background --python scripts/measure_hand_socket_contract.py -- \
  --bone Hand_L \
  --output evidence/102-off-hand-shield-candidates/socket-measurements.json
```

Expected: 32 exact model records and no missing `Hand_L`/clip.

- [ ] **Step 7: Commit the measurement contract**

```bash
git add scripts/measure_hand_socket_contract.py \
  scripts/off_hand_socket_authority.py \
  scripts/test_measure_hand_socket_contract.py \
  scripts/test_off_hand_socket_authority.py \
  scripts/fixtures/off-hand-socket-authority-valid.json \
  scripts/fixtures/off-hand-socket-measurements-valid.json \
  evidence/102-off-hand-shield-candidates/socket-measurements.json
git commit -m "learn: measure shared left-hand rig contracts"
```

---

### Task 4: Kirk Visual Gate and Learn Closure

**Files:**
- Create: `rpg-game-assets/evidence/102-off-hand-shield-candidates/README.md`
- Create: `rpg-game-assets/evidence/102-off-hand-shield-candidates/receipt.json`
- Create: `rpg-game-assets/evidence/102-off-hand-shield-candidates/shield-source-contact-sheet.png`
- Create: `rpg-game-assets/evidence/102-off-hand-shield-candidates/shield-left-hand-contact-sheet.png`
- Modify: `rpg-game-assets/evidence/102-off-hand-shield-candidates/review-config.json`
- Create: `rpg-game-assets/evidence/102-off-hand-shield-candidates/socket-calibration.json`
- Create: `rpg-game-assets/evidence/102-off-hand-shield-candidates/left-hand-variant-authority.json`
- Create: `rpg-game-assets/evidence/102-off-hand-shield-candidates/off-hand-control-contact-sheet.png`
- Modify: `rpg-game-assets/evidence/102-off-hand-shield-candidates/socket-measurements.json`
- Create: `rpg-game-assets/scripts/test_off_hand_shield_candidate_closure.py`
- Create: `rpg-game-assets/scripts/test_off_hand_shield_candidate_evidence.py`

**Interfaces:**
- Produces the merged Learn authority consumed by the Build phase: exactly one accepted Shield, exact Shield normalization/atlas authority, two calibrated socket profiles, Dagger/Shortsword reuse proof, Handaxe/Sickle variant transforms, complete runtime hash baseline, and Kirk's verdict.

- [ ] **Step 1: Calibrate provisional sockets with existing normalized controls**

In Blender, import exact Townfolk Fighter and one modular Fighter plus canonical Dagger and Shortsword. Place those assets at identity under `Hand_L`, calibrate one transform per rig family, and write exact `authoredBlenderBoneLocal` plus converted `runtimeThree` values to `socket-calibration.json`. Keep item roots observable: a user adjustment on an item root is provider normalization evidence, never a socket update. Validate the file through `load_socket_authority()` and record its SHA-256 before candidate rendering. Do not copy/mirror `Hand_R` values without exported-GLB verification.

- [ ] **Step 2: Render pending source and left-hand evidence**

```bash
blender --background --python scripts/render_weapon_candidate_evidence.py -- \
  --config evidence/102-off-hand-shield-candidates/review-config.json \
  --character harness/models/synty/characters/fighter.glb \
  --socket-receipt evidence/102-off-hand-shield-candidates/socket-calibration.json \
  --socket-profile townfolk-off-hand-v1 \
  --source-out evidence/102-off-hand-shield-candidates/shield-source-contact-sheet.png \
  --grip-out evidence/102-off-hand-shield-candidates/shield-left-hand-contact-sheet.png \
  --workspace-root /home/kirk/game-dev

blender --background --python scripts/build_weapon_candidate_review_scene.py -- \
  --config evidence/102-off-hand-shield-candidates/review-config.json \
  --character harness/models/synty/characters/fighter.glb \
  --socket-receipt evidence/102-off-hand-shield-candidates/socket-calibration.json \
  --socket-profile townfolk-off-hand-v1 \
  --out /tmp/rpg102-shield-review.blend \
  --workspace-root /home/kirk/game-dev
```

- [ ] **Step 3: Stop for Kirk's visual decision**

Present source and held views with honest labels: source-faithful/derivative, full dimensions, source pack compatibility, and silhouette class. Do not set an accepted verdict until Kirk names the candidate and canonical scale.

Expected human output: one selected Shield or an explicit rejection/recovery direction.

- [ ] **Step 4: Apply the decision and finalize both sockets**

Set exactly one checkpoint to `accepted`; set all others to `rejected` with reasons. Update the selected entry's exact:

```text
bakedUniformScale
gripTranslationMeters
axisNormalizationQuaternionXyzw
finalDimensionsMeters
runtimeAtlasStrategy
requireTangents=true
```

Calibrate `townfolk-off-hand-v1` and `modular-fantasy-hero-off-hand-v1` using accepted Shield plus Dagger/Shortsword identity controls. Record exact Handaxe/Sickle item-root matrices separately in `left-hand-variant-authority.json`, decomposed into deterministic rigid `bake-static-transform-v1` inputs. Record Dagger/Shortsword as `reuse-canonical-v1`. Discard the review-only War Pick adjustment. Regenerate socket measurements and contact sheets from the final values.

- [ ] **Step 5: Write failing closure tests before the final receipt**

```python
def test_one_human_selection_and_two_socket_profiles_are_bound(self) -> None:
    receipt = json.loads((EVIDENCE / "receipt.json").read_text())
    self.assertEqual("dnd5e:item:shield", receipt["acceptedCandidate"]["ref"])
    self.assertEqual(1, len([row for row in receipt["candidates"] if row["verdict"] == "accepted"]))
    self.assertEqual(
        ["townfolk-off-hand-v1", "modular-fantasy-hero-off-hand-v1"],
        [row["id"] for row in receipt["socketProfiles"]],
    )
    self.assertTrue(all(row["bone"] == "Hand_L" for row in receipt["socketProfiles"]))
    self.assertEqual(4, receipt["rigCoverage"]["townfolkModels"])
    self.assertEqual(28, receipt["rigCoverage"]["modularModels"])
    self.assertEqual(
        ["dnd5e:item:dagger", "dnd5e:item:shortsword"],
        receipt["offHandWeapons"]["reusedCanonicalRefs"],
    )
    self.assertEqual(
        ["dnd5e:item:handaxe", "dnd5e:item:sickle"],
        [row["ref"] for row in receipt["offHandWeapons"]["variantTransforms"]],
    )
```

- [ ] **Step 6: Run closure tests to verify RED**

```bash
python3 -m unittest \
  scripts.test_off_hand_shield_candidate_closure \
  scripts.test_off_hand_shield_candidate_evidence -v
```

Expected: FAIL until receipt/docs and final hashes are complete.

- [ ] **Step 7: Generate the canonical Learn receipt and public-safe docs**

The receipt binds every tracked authority/evidence hash, current provider commit, all existing character and weapon GLB hashes, selected Shield source/normalization/atlas, both coordinate frames, Dagger/Shortsword canonical hashes, Handaxe/Sickle source hashes and reviewed rigid matrices, model coverage, PNG dimensions/decoded hashes, rejected War Pick adjustment, and Kirk's exact verdict. It contains no `/home/`, `/tmp/`, archive provenance disclosure, tokens, or private IDs.

- [ ] **Step 8: Verify no runtime output changed**

```bash
sha256sum -c /tmp/rpg102-runtime-baseline.sha256
python3 -m unittest \
  scripts.test_off_hand_shield_candidate_review \
  scripts.test_off_hand_shield_candidate_closure \
  scripts.test_off_hand_shield_candidate_evidence \
  scripts.test_off_hand_socket_authority -v
python3 -m unittest discover -s scripts -p 'test_*.py'
git diff --check origin/main...HEAD
```

Expected: all baseline hashes match; full suite passes with established skips only.

- [ ] **Step 9: Commit, review, publish verdict, and merge Learn**

```bash
git add evidence/102-off-hand-shield-candidates scripts/test_off_hand_shield_candidate_*.py
git commit -m "learn: select shield and left-hand presentation authority"
git push -u origin learn/102-off-hand-shield-candidates
```

Open the PR for #102. Run a fresh independent whole-branch review, fix findings through TDD, rerun full verification, and publish the final reviewed-head verdict directly on the PR before asking Kirk to merge.

---

### Task 5: Off-Hand Release Authority and Manifest

**Files:**
- Create: `rpg-game-assets/scripts/configs/off-hand-promotion/v1.json`
- Create: `rpg-game-assets/scripts/off_hand_authority.py`
- Create: `rpg-game-assets/scripts/off_hand_manifest.py`
- Create: `rpg-game-assets/scripts/test_off_hand_authority.py`
- Create: `rpg-game-assets/scripts/test_off_hand_manifest.py`
- Consume: `rpg-game-assets/evidence/102-off-hand-shield-candidates/receipt.json`

**Interfaces:**
- Produces `load_off_hand_release_authority(config_path, repo_root, workspace_root, provider_oid="HEAD") -> OffHandReleaseAuthority`.
- Produces `build_off_hand_manifest(authority, output_root) -> dict[str, object]` with exact Shield/Dagger/Shortsword/Handaxe/Sickle items and two socket profiles.

- [ ] **Step 1: After Learn merges, file and link the provider Build issue**

Title: `asset: promote off-hand presentation provider`.

The issue body copies the exact Learn merge commit, accepted Shield/atlas hashes, two socket profiles, Dagger/Shortsword reuse hashes, Handaxe/Sickle rigid-transform authority, existing runtime hash baseline, and Kirk verdict. Add it as a sub-issue of #334 and create a clean worktree from the merged provider `main`.

- [ ] **Step 2: Write failing authority tests**

```python
def test_authority_consumes_exact_merged_learn_receipt(self) -> None:
    authority = load_off_hand_release_authority(CONFIG, ROOT, WORKSPACE)
    self.assertEqual(
        ("dnd5e:item:shield", "dnd5e:item:dagger", "dnd5e:item:shortsword", "dnd5e:item:handaxe", "dnd5e:item:sickle"),
        tuple(item.ref for item in authority.items),
    )
    self.assertEqual(
        ("dnd5e:item:shield", "dnd5e:item:handaxe", "dnd5e:item:sickle"),
        tuple(item.ref for item in authority.generated_items),
    )
    self.assertEqual(
        ("townfolk-off-hand-v1", "modular-fantasy-hero-off-hand-v1"),
        tuple(profile.id for profile in authority.socket_profiles),
    )
    self.assertTrue(authority.require_tangents)
```

- [ ] **Step 3: Run authority tests to verify RED**

```bash
python3 -m unittest scripts.test_off_hand_authority scripts.test_off_hand_manifest -v
```

Expected: missing-module failures.

- [ ] **Step 4: Define the minimal config**

```json
{
  "schemaVersion": 1,
  "workflowVersion": "off-hand-provider-v1",
  "learnReceiptPath": "evidence/102-off-hand-shield-candidates/receipt.json",
  "outputRoot": "harness/models/synty/off-hand",
  "manifestPath": "harness/models/synty/off-hand/manifest.json",
  "runtimeAtlasSize": 1024,
  "budgetDecodedMB": 4.5
}
```

Do not duplicate source paths, transforms, socket values, or hashes in config; load them from the hash-bound merged Learn receipt.

- [ ] **Step 5: Implement strict authority types and loading**

```python
@dataclass(frozen=True)
class OffHandItemAuthority:
    ref: str
    mode: Literal["generate", "reuse"]
    source_path: Path
    source_sha256: str
    output_path: PurePosixPath
    transform: CompiledTransform | None
    runtime_atlas_size: int
    budget_decoded_mb: float
    require_tangents: bool


@dataclass(frozen=True)
class OffHandReleaseAuthority:
    provider_oid: str
    repo_root: Path
    workspace_root: Path
    learn_receipt_path: Path
    learn_receipt_sha256: str
    output_root: PurePosixPath
    manifest_path: PurePosixPath
    items: tuple[OffHandItemAuthority, ...]
    generated_items: tuple[OffHandItemAuthority, ...]
    socket_profiles: tuple[SocketProfile, SocketProfile]
```

Reuse `compile_axis_grip_transform()` and hash/path safety from `weapon_authority`; reject dirty/untracked authority, duplicate JSON keys, unsafe paths, wrong exact item order, stale Shield/source/reuse hashes, missing transform evidence or Kirk verdict, or anything other than two exact `Hand_L` profiles. Handaxe/Sickle sources are the commit-bound canonical main-hand GLBs; their transforms compile from Learn matrices. Dagger/Shortsword must have `transform is None` and reuse their canonical paths directly.

- [ ] **Step 6: Implement deterministic off-hand manifest generation**

Build the document directly from validated authority and measured staged facts:

```python
return {
    "schemaVersion": 1,
    "generator": {"id": "off-hand-provider", "version": "1.0.0"},
    "authorities": {
        "learnReceipt": {
            "path": authority.learn_receipt_path.relative_to(authority.repo_root).as_posix(),
            "sha256": authority.learn_receipt_sha256,
        }
    },
    "socketProfiles": {
        profile.id: socket_manifest_entry(profile)
        for profile in authority.socket_profiles
    },
    "items": {
        item.ref: off_hand_manifest_entry(item, measured_facts[item.ref], authority)
        for item in authority.items
    },
}
```

`socket_manifest_entry` and `off_hand_manifest_entry` are pure functions with direct tests for complete facts, generated-versus-reused mode, provider-normalization evidence, canonical ordering, and `runtimeCorrectionRequired: false`.

- [ ] **Step 7: Run tests and commit authority**

```bash
python3 -m unittest scripts.test_off_hand_authority scripts.test_off_hand_manifest -v
git add scripts/configs/off-hand-promotion/v1.json scripts/off_hand_authority.py \
  scripts/off_hand_manifest.py scripts/test_off_hand_authority.py scripts/test_off_hand_manifest.py
git commit -m "asset: define strict off-hand release authority"
```

---

### Task 6: Transactional Off-Hand Stage and Apply

**Files:**
- Create: `rpg-game-assets/scripts/static_release_transaction.py`
- Create: `rpg-game-assets/scripts/test_static_release_transaction.py`
- Create: `rpg-game-assets/scripts/promote_off_hand.py`
- Create: `rpg-game-assets/scripts/test_promote_off_hand.py`
- Modify: `rpg-game-assets/scripts/build_mesh_stats.py`
- Modify: `rpg-game-assets/scripts/build_web_asset_catalog.py`
- Modify: `rpg-game-assets/scripts/test_build_web_asset_catalog.py`

**Interfaces:**
- Produces `promote_off_hand.py stage|validate|apply|check` with the same clean-head/seal/rollback lifecycle as `promote_weapons.py`.
- Produces generic `atomic_apply_targets(repo_root, staged_root, targets, inject_failure_after=None)` with byte rollback.

- [ ] **Step 1: Write failing transaction tests**

```python
def test_atomic_apply_rolls_back_existing_and_new_targets(self) -> None:
    before = {"existing.glb": b"old"}
    staged = {"existing.glb": b"new", "new-manifest.json": b"{}\n"}
    with self.assertRaisesRegex(RuntimeError, "injected"):
        atomic_apply_targets(self.repo, self.stage, tuple(staged), inject_failure_after="existing.glb")
    self.assertEqual(b"old", (self.repo / "existing.glb").read_bytes())
    self.assertFalse((self.repo / "new-manifest.json").exists())
```

- [ ] **Step 2: Run transaction tests to verify RED**

```bash
python3 -m unittest scripts.test_static_release_transaction -v
```

Expected: missing module/function.

- [ ] **Step 3: Implement the generic atomic target transaction**

Move no weapon semantics. Implement only portable target validation, snapshot, fsync/replace, injected-failure support, reverse-order rollback, and cleanup. Reuse the function from `promote_off_hand.py`; leave `promote_weapons.py` unchanged in this slice unless a separate behavior-preserving extraction proves necessary.

- [ ] **Step 4: Write failing off-hand-stage tests**

```python
def test_stage_contains_exact_six_generated_targets(self) -> None:
    seal = stage_release(self.args)
    self.assertEqual(
        {
            "harness/models/synty/off-hand/shield.glb",
            "harness/models/synty/off-hand/handaxe.glb",
            "harness/models/synty/off-hand/sickle.glb",
            "harness/models/synty/off-hand/manifest.json",
            "harness/models/synty/mesh-stats.json",
            "harness/catalogs/synty-complete-inventory.json",
        },
        {row["path"] for row in seal["targetRecords"]},
    )


def test_validation_requires_strict_static_semantics(self) -> None:
    for name in ("shield.glb", "handaxe.glb", "sickle.glb"):
        gltf, _ = read_glb(self.stage / "release/harness/models/synty/off-hand" / name)
        for mesh in gltf["meshes"]:
            for primitive in mesh["primitives"]:
                self.assertTrue({"POSITION", "NORMAL", "TEXCOORD_0", "TANGENT"} <= set(primitive["attributes"]))
```


- [ ] **Step 5: Run off-hand-stage tests to verify RED**

```bash
python3 -m unittest scripts.test_promote_off_hand -v
```

Expected: missing stage implementation.

- [ ] **Step 6: Implement off-hand normalization/staging**

Use existing tested primitives:

```python
from weapon_glb import (
    ensure_tangent_semantics,
    inspect_weapon,
    normalize_weapon_glb,
    validate_atlas_rewrite,
    validate_geometry_rewrite,
)
```

The names remain historical; behavior is valid for strict static equipment. Stage in this order:

1. verify clean tracked HEAD and commit-bound inputs;
2. normalize selected Shield source using its Learn transform;
3. transform canonical Handaxe/Sickle GLBs using the reviewed rigid variant transforms while preserving their embedded atlases;
4. add tangents only if absent;
5. rewrite/resize only the Shield's source-compatible atlas to 1024²;
6. validate all three generated identity-root outputs and exact Dagger/Shortsword reuse hashes;
7. build the five-item off-hand manifest from measured staged/reused bytes;
8. regenerate mesh stats and complete inventory;
9. write a canonical seal with provider OID, input records, prestate, target records, and tree SHA;
10. validate before any canonical apply.

- [ ] **Step 7: Extend runtime catalog generation for `off-hand/`**

Add `harness/models/synty/off-hand/manifest.json` as a supported runtime root without changing main-hand weapon entries. Tests assert the exact five-item off-hand order, three generated paths, Dagger/Shortsword reuse paths, and unchanged 27 main-hand weapon hashes.

- [ ] **Step 8: Run release-tooling and weapon-regression tests**

```bash
python3 -m unittest \
  scripts.test_static_release_transaction \
  scripts.test_promote_off_hand \
  scripts.test_build_web_asset_catalog \
  scripts.test_promote_weapons -v
```

Expected: PASS, including rollback injection and unchanged weapon-pipeline behavior.

- [ ] **Step 9: Commit release tooling before sealing any stage**

```bash
git add \
  scripts/static_release_transaction.py \
  scripts/test_static_release_transaction.py \
  scripts/promote_off_hand.py \
  scripts/test_promote_off_hand.py \
  scripts/build_mesh_stats.py \
  scripts/build_web_asset_catalog.py \
  scripts/test_build_web_asset_catalog.py
git commit -m "asset: add transactional off-hand release pipeline"
test -z "$(git status --porcelain -uall)"
```

Do not stage, apply, or create canonical off-hand outputs until the evidence tooling in Task 7 is also committed and the tracked provider HEAD is clean.

---

### Task 7: Provider Evidence, Clean Rebuild, and Merge Gate

**Files:**
- Create: `rpg-game-assets/scripts/render_promoted_off_hand_evidence.py`
- Create: `rpg-game-assets/scripts/off_hand_release_evidence.py`
- Create: `rpg-game-assets/scripts/test_off_hand_release_evidence.py`
- Create: `rpg-game-assets/scripts/test_off_hand_provider.py`
- Create: `rpg-game-assets/evidence/334-off-hand-provider/README.md`
- Create: `rpg-game-assets/evidence/334-off-hand-provider/receipt.json`
- Create: `rpg-game-assets/evidence/334-off-hand-provider/off-hand-provider-contact-sheet.png`
- Create: `rpg-game-assets/harness/models/synty/off-hand/shield.glb`
- Create: `rpg-game-assets/harness/models/synty/off-hand/handaxe.glb`
- Create: `rpg-game-assets/harness/models/synty/off-hand/sickle.glb`
- Create: `rpg-game-assets/harness/models/synty/off-hand/manifest.json`
- Modify: `rpg-game-assets/harness/models/synty/mesh-stats.json`
- Modify: `rpg-game-assets/harness/catalogs/synty-complete-inventory.json`

**Interfaces:**
- Consumes only validated sealed stage bytes plus commit-bound character/weapon controls.
- Produces immutable visual/release evidence and complete hash-preservation proof.

- [ ] **Step 1: Write failing release-evidence tests**

```python
def test_receipt_binds_off_hand_items_two_sockets_and_existing_hashes(self) -> None:
    receipt = build_receipt(self.validated_fixture_stage)
    self.assertEqual(
        ["dnd5e:item:shield", "dnd5e:item:dagger", "dnd5e:item:shortsword", "dnd5e:item:handaxe", "dnd5e:item:sickle"],
        [row["ref"] for row in receipt["offHandItems"]],
    )
    self.assertEqual(["townfolk-off-hand-v1", "modular-fantasy-hero-off-hand-v1"], receipt["socketProfiles"])
    self.assertEqual(27, len(receipt["existingWeaponBaseline"]))
    self.assertEqual(32, len(receipt["existingCharacterBaseline"]))
```

- [ ] **Step 2: Run the test to verify RED**

```bash
python3 -m unittest scripts.test_off_hand_release_evidence -v
```

- [ ] **Step 3: Implement sealed-stage evidence tooling**

The renderer accepts only an external validated stage and produces:

- Shield source-faithful full view;
- Human Fighter Shield-only Idle/Walk;
- Human Fighter Longsword + Shield Idle/Walk;
- Human Fighter Shortsword + Dagger Idle/Walk;
- one modular Fighter for the same three states; and
- close hand/grip views for Shield, reused Dagger/Shortsword, and variant Handaxe/Sickle controls.

It refuses canonical provider outputs, stale seal hashes, wrong model/weapon controls, symlinks, PNG metadata, or writes outside its explicit output path. `off_hand_release_evidence.py` records provider commit, seal/tree hashes, Learn receipt hash, all five catalog item hashes/modes, strict generated GLB facts, both socket profiles, all 27 main-hand weapon hashes, all 4 Townfolk + 28 modular character hashes, atlas/base/mip facts, PNG byte/decoded hashes, and Kirk's provider-sheet verdict.

- [ ] **Step 4: Run unit tests and commit evidence tooling**

```bash
python3 -m unittest \
  scripts.test_off_hand_release_evidence \
  scripts.test_render_promoted_weapon_evidence \
  scripts.test_weapon_release_evidence -v
git add \
  scripts/render_promoted_off_hand_evidence.py \
  scripts/off_hand_release_evidence.py \
  scripts/test_off_hand_release_evidence.py
git commit -m "asset: add sealed off-hand release evidence"
test -z "$(git status --porcelain -uall)"
```

- [ ] **Step 5: Build and validate the clean sealed stage**

```bash
stage=/tmp/rpg334-off-hand-stage
rm -rf "$stage"
python3 scripts/promote_off_hand.py stage \
  --config scripts/configs/off-hand-promotion/v1.json \
  --workspace-root /home/kirk/game-dev \
  --stage-root "$stage"
python3 scripts/promote_off_hand.py validate \
  --config scripts/configs/off-hand-promotion/v1.json \
  --stage-root "$stage"
```

Expected: the seal binds the clean tracked HEAD containing all release/evidence code and contains exactly the six canonical target records.

- [ ] **Step 6: Write the production closure test and verify RED**

```python
def test_canonical_receipt_matches_applied_outputs(self) -> None:
    receipt = json.loads((EVIDENCE / "receipt.json").read_text())
    self.assertEqual(
        ["dnd5e:item:shield", "dnd5e:item:dagger", "dnd5e:item:shortsword", "dnd5e:item:handaxe", "dnd5e:item:sickle"],
        [row["ref"] for row in receipt["offHandItems"]],
    )
    for name in ("shield.glb", "handaxe.glb", "sickle.glb"):
        row = next(item for item in receipt["offHandItems"] if item.get("generatedPath", "").endswith(name))
        self.assertEqual(row["sha256"], sha256(ROOT / "harness/models/synty/off-hand" / name))
    self.assertEqual(27, len(receipt["existingWeaponBaseline"]))
    self.assertEqual(32, len(receipt["existingCharacterBaseline"]))
```

Run:

```bash
python3 -m unittest scripts.test_off_hand_provider -v
```

Expected: FAIL because canonical receipt/output files are not applied.

- [ ] **Step 7: Render and generate evidence from immutable stage bytes**

```bash
mkdir -p evidence/334-off-hand-provider
blender --background --python scripts/render_promoted_off_hand_evidence.py -- \
  --stage-release-root "$stage/release" \
  --output-sheet evidence/334-off-hand-provider/off-hand-provider-contact-sheet.png
python3 scripts/off_hand_release_evidence.py build \
  --stage-root "$stage" \
  --evidence-root evidence/334-off-hand-provider
```

- [ ] **Step 8: Stop for Kirk's sealed provider visual approval**

Do not apply if Kirk rejects scale, facing, grip, or silhouette. Correct provider normalization/socket authority, commit those corrections, delete the old stage, and regenerate the entire sealed evidence chain from the new clean HEAD.

- [ ] **Step 9: Apply only approved bytes and commit outputs/evidence**

```bash
python3 scripts/promote_off_hand.py apply --stage-root "$stage"
python3 -m unittest scripts.test_off_hand_provider -v
git add \
  scripts/test_off_hand_provider.py \
  harness/models/synty/off-hand/shield.glb \
  harness/models/synty/off-hand/handaxe.glb \
  harness/models/synty/off-hand/sickle.glb \
  harness/models/synty/off-hand/manifest.json \
  harness/models/synty/mesh-stats.json \
  harness/catalogs/synty-complete-inventory.json \
  evidence/334-off-hand-provider
git commit -m "asset: promote owner off-hand presentation provider"
python3 scripts/promote_off_hand.py check \
  --config scripts/configs/off-hand-promotion/v1.json \
  --workspace-root /home/kirk/game-dev
```

- [ ] **Step 10: Run clean-head rebuild comparison and full verification**

```bash
python3 -m unittest discover -s scripts -p 'test_*.py'
git diff --check origin/main...HEAD

git worktree add --detach /tmp/rpg334-provider-rebuild HEAD
cd /tmp/rpg334-provider-rebuild
python3 scripts/promote_off_hand.py stage \
  --config scripts/configs/off-hand-promotion/v1.json \
  --workspace-root /home/kirk/game-dev \
  --stage-root /tmp/rpg334-off-hand-clean-rebuild
python3 scripts/promote_off_hand.py validate \
  --config scripts/configs/off-hand-promotion/v1.json \
  --stage-root /tmp/rpg334-off-hand-clean-rebuild
python3 scripts/promote_off_hand.py check \
  --config scripts/configs/off-hand-promotion/v1.json \
  --workspace-root /home/kirk/game-dev
```

Compare all release targets byte-for-byte to the reviewed sealed tree, then remove the detached rebuild worktree.

- [ ] **Step 11: Review, publish verdict, and merge**

```bash
git push -u origin HEAD
```

Open the Build PR. Run fresh independent whole-branch review, resolve findings through TDD, rerun exact-head verification, publish the verdict on the PR, and merge before creating web work.

---

### Task 8: Generic Bone-Attachment Core Without Main-Hand Drift

**Files:**
- Create: `rpg-dnd5e-web/src/components/hex-grid/boneAttachment.ts`
- Create: `rpg-dnd5e-web/src/components/hex-grid/boneAttachment.test.ts`
- Create: `rpg-dnd5e-web/src/components/hex-grid/BoneAttachmentSlot.tsx`
- Create: `rpg-dnd5e-web/src/components/hex-grid/BoneAttachmentSlot.test.tsx`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/mainHandPresentation.ts`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/MainHandAttachment.tsx`
- Test: `rpg-dnd5e-web/src/components/hex-grid/MainHandAttachment.test.tsx`
- Test: `rpg-dnd5e-web/src/components/hex-grid/mainHandPresentation.test.ts`

**Interfaces:**
- Produces `attachBoneObject(characterRoot, assetRoot, presentation) -> BoneAttachmentResult`.
- Produces `<BoneAttachmentSlot characterRoot presentation onStatus />`.
- Existing main-hand exports and status shape remain source/behavior compatible.

- [ ] **Step 1: After provider merge, file/link the web issue and sync exact assets**

Create a web issue titled `feat: show owner-authoritative off-hand equipment`, add it beneath #334, create a clean `dev` worktree, and run baseline CI.

Sync from the exact merged provider commit:

```bash
RPG_GAME_ASSETS_PATH=/path/to/exact/provider/worktree \
ASSETS_SYNC_SKIP_UPDATE=1 \
RPG_WEB_ROOT="$PWD" \
  sh scripts/sync-game-assets.sh
```

Verify the ignored shield/manifest bytes against provider hashes before tests or capture.

- [ ] **Step 2: Write failing generic attachment tests**

```ts
it('attaches an asset beneath the configured bone with unit compensation', () => {
  const result = attachBoneObject(root, asset, {
    ref: 'dnd5e:item:shield',
    assetUrl: '/models/synty/off-hand/shield.glb',
    socket: {
      bone: 'Hand_L',
      boneUnitMeters: 0.01,
      positionMeters: [0.1, 0.2, 0.3],
      rotationQuaternion: [0, 0, 0, 1],
      scale: 1,
    },
  });
  expect(result.status.code).toBe('attached');
  expect(asset.parent).toBe(handL);
  expect(asset.position.toArray()).toEqual([10, 20, 30]);
  expect(asset.scale.toArray()).toEqual([100, 100, 100]);
});
```

Also test invalid socket, missing bone, detach idempotence, cached-scene cloning, loading, load failure, stale keyed replacement, and independent slot failure.

- [ ] **Step 3: Run tests to verify RED**

```bash
npx vitest run \
  src/components/hex-grid/boneAttachment.test.ts \
  src/components/hex-grid/BoneAttachmentSlot.test.tsx
```

- [ ] **Step 4: Implement pure generic types/functions**

```ts
export interface HandSocket {
  bone: string;
  boneUnitMeters: number;
  positionMeters: readonly [number, number, number];
  rotationQuaternion: readonly [number, number, number, number];
  scale: number;
}

export interface BonePresentation {
  ref: string;
  assetUrl: string;
  socket: HandSocket;
}
```

Keep the same finite/quaternion/unit checks and scene mutation order as current main hand.

- [ ] **Step 5: Implement generic React loading/error slot**

Clone the URL-cached scene per mounted item, disable attachment raycasts, attach through `attachBoneObject`, detach on replacement/unmount, and report generic `assetUrl` status. Key callers by `ref|assetUrl`.

- [ ] **Step 6: Adapt main hand through wrappers**

`mainHandPresentation.ts` maps `weaponUrl` to generic `assetUrl` and maps generic status back to the current `weaponUrl` status field. `MainHandAttachmentSlot` delegates to `BoneAttachmentSlot`. Do not change current public prop names or resolver output.

- [ ] **Step 7: Run generic and complete main-hand regression tests**

```bash
npx vitest run \
  src/components/hex-grid/boneAttachment.test.ts \
  src/components/hex-grid/BoneAttachmentSlot.test.tsx \
  src/components/hex-grid/mainHandPresentation.test.ts \
  src/components/hex-grid/MainHandAttachment.test.tsx \
  src/components/hex-grid/mainHandWeapons.test.ts \
  src/components/hex-grid/ClassCharacterModel.test.tsx
```

- [ ] **Step 8: Commit the behavior-preserving core**

```bash
git add src/components/hex-grid/boneAttachment* \
  src/components/hex-grid/BoneAttachmentSlot* \
  src/components/hex-grid/mainHandPresentation.ts \
  src/components/hex-grid/MainHandAttachment.tsx
git commit -m "refactor: share rigid hand attachment core"
```

---

### Task 9: Exact Off-Hand Resolver and Rig Sockets

**Files:**
- Create: `rpg-dnd5e-web/src/components/hex-grid/offHandEquipment.ts`
- Create: `rpg-dnd5e-web/src/components/hex-grid/offHandEquipment.test.ts`
- Create: `rpg-dnd5e-web/src/components/hex-grid/OffHandAttachment.tsx`
- Create: `rpg-dnd5e-web/src/components/hex-grid/OffHandAttachment.test.tsx`
- Reuse: `rpg-dnd5e-web/src/components/hex-grid/mainHandWeapons.ts`

**Interfaces:**
- Produces `resolveOffHandPresentation(equipped) -> OffHandPresentationResolution`.
- Produces `offHandSocketForRigFamily(rigFamily) -> HandSocket`.
- Produces `<OffHandAttachmentSlot />` using the generic core.

- [ ] **Step 1: Write failing exact-resolution tests**

```ts
it('maps only exact refs in the reviewed off-hand provider catalog', () => {
  expect(resolveOffHandPresentation(equipped('item', 'shield'))).toMatchObject({
    code: 'mapped',
    presentation: {
      ref: 'dnd5e:item:shield',
      assetUrl: '/models/synty/off-hand/shield.glb',
      assetKind: 'shield',
    },
  });
  expect(resolveOffHandPresentation(equipped('item', 'dagger'))).toMatchObject({
    code: 'mapped',
    presentation: {
      ref: 'dnd5e:item:dagger',
      assetUrl: '/models/synty/weapons/dagger.glb',
      assetKind: 'weapon',
    },
  });
  expect(resolveOffHandPresentation(equipped('item', 'handaxe'))).toMatchObject({
    code: 'mapped',
    presentation: {
      ref: 'dnd5e:item:handaxe',
      assetUrl: '/models/synty/off-hand/handaxe.glb',
      assetKind: 'weapon',
    },
  });
  expect(resolveOffHandPresentation(equipped('item', 'sickle'))).toMatchObject({
    code: 'mapped',
    presentation: {
      ref: 'dnd5e:item:sickle',
      assetUrl: '/models/synty/off-hand/sickle.glb',
      assetKind: 'weapon',
    },
  });
  expect(resolveOffHandPresentation(equipped('item', 'war-pick'))).toEqual({
    code: 'unmapped-ref', ref: 'dnd5e:item:war-pick',
  });
  expect(resolveOffHandPresentation(equipped('item', 'unknown'))).toEqual({
    code: 'unmapped-ref', ref: 'dnd5e:item:unknown',
  });
  expect(resolveOffHandPresentation(equipped('weapons', 'dagger'))).toEqual({
    code: 'unmapped-ref', ref: 'dnd5e:weapons:dagger',
  });
});
```

Also assert missing `off_hand` is empty, exact Shortsword reuses `/models/synty/weapons/shortsword.glb`, catalog order is Shield/Dagger/Shortsword/Handaxe/Sickle, every other main-hand ref remains unsupported, and no handedness/property table or transform catalog exists.

- [ ] **Step 2: Run tests to verify RED**

```bash
npx vitest run src/components/hex-grid/offHandEquipment.test.ts
```

- [ ] **Step 3: Implement exact definitions and socket constants**

Copy the two exact `runtimeThree` profiles and five exact item definitions by value from the merged off-hand provider manifest, with provider commit/manifest hash comments. Define:

```ts
export interface OffHandPresentation extends BonePresentation {
  assetKind: 'shield' | 'weapon';
}
```

Build a five-item presentation-support lookup from the exact provider manifest. Dagger/Shortsword URLs point to canonical weapon files; Handaxe/Sickle URLs point to generated off-hand files. Do not infer support from weapon properties or the main-hand catalog.

- [ ] **Step 4: Implement the semantic off-hand wrapper**

`OffHandAttachmentSlot` delegates to `BoneAttachmentSlot` and reports `empty-off-hand`, `loading`, `attached`, `asset-load-failed`, `missing-bone`, or `invalid-socket`. It never falls back to `CharacterShield`.

- [ ] **Step 5: Run focused tests and commit**

```bash
npx vitest run \
  src/components/hex-grid/offHandEquipment.test.ts \
  src/components/hex-grid/OffHandAttachment.test.tsx \
  src/components/hex-grid/mainHandWeapons.test.ts
git add src/components/hex-grid/offHandEquipment* src/components/hex-grid/OffHandAttachment*
git commit -m "feat: resolve exact owner off-hand presentation"
```

---

### Task 10: Owner-Only Production Routing

**Files:**
- Modify: `rpg-dnd5e-web/src/components/session/SessionEncounterView.tsx`
- Modify: `rpg-dnd5e-web/src/components/session/SessionEncounterView.test.tsx`
- Modify: `rpg-dnd5e-web/src/components/session/SessionCanvas.tsx`
- Modify: `rpg-dnd5e-web/src/components/session/SessionCanvas.test.tsx`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/HexEntity.tsx`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/HexEntity.test.tsx`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/ClassCharacterModel.tsx`
- Modify: `rpg-dnd5e-web/src/components/hex-grid/ClassCharacterModel.test.tsx`

**Interfaces:**
- Adds optional `offHandPresentation` through the existing owner route.
- Adds `offHandSocketOverride` and `onOffHandStatus` to `ClassCharacterModel`.

- [ ] **Step 1: Write failing owner-routing tests**

```ts
it('projects owner off_hand only to the acting player entity', () => {
  renderSession({
    ownerCharacterId: 'owner',
    ownerEquipped: { off_hand: itemRef('shield') },
    peerCharacterId: 'peer',
  });
  expect(ownerEntityProps().offHandPresentation?.ref).toBe('dnd5e:item:shield');
  expect(peerEntityProps().offHandPresentation).toBeUndefined();
});
```

Add propagation tests at every component boundary and assert unknown refs remain empty while main hand is unchanged.

- [ ] **Step 2: Run route tests to verify RED**

```bash
npx vitest run \
  src/components/session/SessionEncounterView.test.tsx \
  src/components/session/SessionCanvas.test.tsx \
  src/components/hex-grid/HexEntity.test.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx
```

- [ ] **Step 3: Implement one-way owner projection**

In `SessionEncounterView`, call `resolveOffHandPresentation(ownerCharacterData?.equipped ?? {})` beside the existing main resolver. Pass only its `.presentation`; do not inspect AC, inventory kind, slot legality, or peer data.

- [ ] **Step 4: Mount independent left attachment in `ClassCharacterModel`**

Select `offHandSocketForRigFamily(playerModelResolution.rigFamily)` in `HexEntity`. In `ClassCharacterModel`, apply the override by memo and render:

```tsx
<OffHandAttachmentSlot
  key={effectiveOffHandPresentation
    ? `${effectiveOffHandPresentation.ref}|${effectiveOffHandPresentation.assetUrl}`
    : 'empty-off-hand'}
  characterRoot={cloned}
  presentation={effectiveOffHandPresentation}
  onStatus={onOffHandStatus}
/>
```

Keep main and off hand as siblings so either can fail independently.

- [ ] **Step 5: Run complete route regression tests**

```bash
npx vitest run \
  src/components/session/SessionEncounterView.test.tsx \
  src/components/session/SessionCanvas.test.tsx \
  src/components/hex-grid/HexEntity.test.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx \
  src/components/hex-grid/mainHandWeapons.test.ts \
  src/components/hex-grid/MainHandAttachment.test.tsx
```

- [ ] **Step 6: Commit production routing**

```bash
git add \
  src/components/session/SessionEncounterView.tsx \
  src/components/session/SessionEncounterView.test.tsx \
  src/components/session/SessionCanvas.tsx \
  src/components/session/SessionCanvas.test.tsx \
  src/components/hex-grid/HexEntity.tsx \
  src/components/hex-grid/HexEntity.test.tsx \
  src/components/hex-grid/ClassCharacterModel.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx
git commit -m "feat: route owner off-hand presentation"
```

---

### Task 11: Production-Backed Off-Hand Concept

**Files:**
- Create: `rpg-dnd5e-web/src/concepts/off-hand-attachment/OffHandAttachmentConcept.tsx`
- Create: `rpg-dnd5e-web/src/concepts/off-hand-attachment/OffHandAttachmentConcept.test.tsx`
- Create: `rpg-dnd5e-web/src/concepts/off-hand-attachment/offHandAttachmentExperiment.ts`
- Create: `rpg-dnd5e-web/src/concepts/off-hand-attachment/offHandAttachmentExperiment.test.ts`
- Create: `rpg-dnd5e-web/src/concepts/off-hand-attachment/CONTRACT.md`
- Modify: `rpg-dnd5e-web/src/concepts/ConceptsView.tsx`
- Modify: `rpg-dnd5e-web/src/App.tsx`

**Interfaces:**
- Produces deep-linkable `?concept=off-hand-attachment` using the production resolvers and `ClassCharacterModel`.
- Produces exact fixtures `empty`, `shield-only`, `longsword-shield`, and `shortsword-dagger`.

- [ ] **Step 1: Write failing Concept contract tests**

```ts
expect(OFF_HAND_FIXTURES.map(row => row.id)).toEqual([
  'empty',
  'shield-only',
  'longsword-shield',
  'shortsword-dagger',
]);
expect(screen.getByTestId('off-hand-coverage').textContent).toContain('states 4/4');
expect(screen.queryByRole('slider')).toBeNull();
expect(screen.queryByRole('spinbutton')).toBeNull();
```

- [ ] **Step 2: Run Concept tests to verify RED**

```bash
npx vitest run \
  src/concepts/off-hand-attachment/offHandAttachmentExperiment.test.ts \
  src/concepts/off-hand-attachment/OffHandAttachmentConcept.test.tsx
```

- [ ] **Step 3: Implement focused production-backed fixtures and UI**

Fixtures contain only wire-shaped `equipped` maps. The Concept resolves both slots through production resolvers and mounts the production `ClassCharacterModel`. Controls select state, class/race model, Idle/Walk, orbit/close/play view, and facing. Diagnostics show exact refs, URLs, rig family, socket IDs, bone lookup, and attachment status; there are no transform controls.

- [ ] **Step 4: Register the deep link and document the contract**

Register `off-hand-attachment` through the existing Concepts route. `CONTRACT.md` records exact-provider ownership, four fixture states, rig/motion/view coverage, error accounting, and the prohibition on transform controls or gameplay rules.

- [ ] **Step 5: Run focused and integration tests**

```bash
npx vitest run \
  src/concepts/off-hand-attachment/offHandAttachmentExperiment.test.ts \
  src/concepts/off-hand-attachment/OffHandAttachmentConcept.test.tsx \
  src/components/hex-grid/ClassCharacterModel.test.tsx
```

- [ ] **Step 6: Commit the Concept**

```bash
git add \
  src/concepts/off-hand-attachment \
  src/concepts/ConceptsView.tsx \
  src/App.tsx
git commit -m "feat: add off-hand attachment concept"
```

---

### Task 12: Browser and Normal-Game Evidence

**Files:**
- Create: `rpg-dnd5e-web/docs/evidence/334-off-hand-presentation/README.md`
- Create: `rpg-dnd5e-web/docs/evidence/334-off-hand-presentation/receipt.json`
- Create: `rpg-dnd5e-web/docs/evidence/334-off-hand-presentation/off-hand-townfolk-contact-sheet.png`
- Create: `rpg-dnd5e-web/docs/evidence/334-off-hand-presentation/off-hand-modular-contact-sheet.png`
- Create: `rpg-dnd5e-web/docs/evidence/334-off-hand-presentation/off-hand-fighter-walk-contact-sheet.png`
- Create: `rpg-dnd5e-web/docs/evidence/334-off-hand-presentation/authoritative-shield-dual-wield-contact-sheet.png`
- Modify: `rpg-dnd5e-web/docs/architecture/components/equipment.md`
- Modify: `rpg-dnd5e-web/docs/architecture/components/hex-grid.md`

**Interfaces:**
- Produces exact-head visual/authority evidence with provider hashes, HTTP status, owner-state parity, and public-safety integrity.

- [ ] **Step 1: Build and serve the exact code head**

```bash
npm run build -- --mode development
npm run preview -- --host 127.0.0.1 --port 3028
```

Record web code head, provider merge, shield/manifest hashes, Chromium version, and Playwright version before capture.

- [ ] **Step 2: Capture Concept matrices**

Capture exact HTTP 200/hash-verified states:

- four Human/Townfolk classes × Shield-only, Longsword+Shield, Shortsword+Dagger in Idle;
- seven modular Fighter races × the same three states in Idle;
- Human Fighter plus one modular Fighter × all three states in Walk;
- empty off hand control;
- zero unexpected console errors, page errors, or request failures;
- expected background stream teardown aborts counted separately.

- [ ] **Step 3: Capture normal-UI authority sequence**

Use the existing normal character/equipment UI only:

1. record exact initial owner equipment, AC, HP, speed, and main-hand damage;
2. Equip Shield into `off_hand`; record RPC HTTP 200 and immediate visible Shield;
3. open a fresh browser context; record authoritative Shield restoration;
4. Unequip Shield; record HTTP 200 and immediate removal;
5. equip a server-valid Shortsword main hand + Dagger off hand pairing;
6. open a fresh context; record both exact assets restored;
7. remove both through normal UI;
8. open a fresh context; prove exact initial owner state restored.

Do not patch Redis, fixture data, responses, or client state. AC is evidence only.

- [ ] **Step 4: Build contact sheets and receipt**

Receipt binds raw observations, exact provider bytes, image SHA/size/dimensions/decoded RGB hash, expected versus unexpected errors, exact initial/final state, and redaction counts. PNGs must contain no text metadata, absolute paths, private IDs, or trailing bytes.

- [ ] **Step 5: Run evidence integrity and license-boundary checks**

```bash
python3 - <<'PY'
import hashlib, json
from pathlib import Path
from PIL import Image
root = Path('docs/evidence/334-off-hand-presentation')
r = json.loads((root / 'receipt.json').read_text())
assert r['authority']['initialOwnerState'] == r['authority']['finalOwnerState']
assert r['errors']['unexpectedConsoleErrors'] == 0
assert r['errors']['pageErrors'] == 0
assert r['errors']['unexpectedRequestFailures'] == 0
for row in r['sheets']:
    p = root / row['name']
    assert hashlib.sha256(p.read_bytes()).hexdigest() == row['sha256']
    with Image.open(p) as image:
        image.load()
        assert image.format == 'PNG'
        assert not image.info
git_text = '\n'.join((root / n).read_text() for n in ('README.md', 'receipt.json'))
assert '/home/' not in git_text and '/tmp/' not in git_text
PY

test "$(git ls-files 'public/models/**/*.glb' | wc -l)" -eq 0
git diff --check origin/dev...HEAD
```

- [ ] **Step 6: Commit evidence/docs**

Bind evidence to the code commit, then ensure only `docs/evidence/334-off-hand-presentation/*` changes after it.

```bash
git add docs/architecture/components/equipment.md docs/architecture/components/hex-grid.md
git add -f docs/evidence/334-off-hand-presentation
git commit -m "docs: record owner off-hand presentation evidence"
```

---

### Task 13: Exact-Head Verification, Review, Merge, and Journey Closure

**Files:**
- Update after merge: `rpg-project/active.md` (local continuity only)
- Comment/close: provider Learn issue, provider Build issue, web issue, Journey #334
- Update: Project 19 item statuses

**Interfaces:**
- Produces merged provider/web commits, published durable verdicts, closed #334, and an explicit return to #302.

- [ ] **Step 1: Run final exact-head provider verification**

```bash
python3 -m unittest discover -s scripts -p 'test_*.py'
python3 scripts/promote_off_hand.py check \
  --config scripts/configs/off-hand-promotion/v1.json \
  --workspace-root /home/kirk/game-dev
git diff --check origin/main...HEAD
test -z "$(git status --porcelain -uall)"
```

- [ ] **Step 2: Run final exact-head web verification**

```bash
npm run ci-check
npm test -- --run
git diff --check origin/dev...HEAD
test -z "$(git status --porcelain -uall)"
```

- [ ] **Step 3: Run fresh independent whole-branch reviews**

Each reviewer checks exact merged dependencies, authority boundaries, two socket profiles, no correction table, strict GLB/media hashes, owner-only routing, unchanged main hand, legacy shield non-fallback, restoration, no tracked licensed GLBs, and current CI. Resolve every Critical/Important finding and disposition every Minor.

- [ ] **Step 4: Publish current-head verdicts directly on PRs**

Each comment includes:

```text
Reviewed head
Readiness
Critical / Important / Minor counts
Scope reviewed
Verification evidence
Prior-finding disposition
```

If a reviewed head changes materially, rerun/rebind review and publish an updated verdict.

- [ ] **Step 5: Merge inside-out and verify exact commits**

Order:

1. Learn PR;
2. provider Build PR;
3. web PR.

After each merge, query GitHub for `mergeCommit.oid`, close the owning issue, move its Project item to Done, and remove only its clean merged worktree/local branch.

- [ ] **Step 6: Close Journey #334 and resume #302**

Post the Learn/provider/web merge commits, Shield/manifest hashes, socket IDs, preservation counts, Concept matrix counts, normal-game sequence, exact restoration, test totals, and published verdicts to #334. Close/move it Done only when all done-when conditions pass.

Update #302: off-hand foundation is complete; resume the approved Glaive, Lance, Scimitar, and Trident Learn slice with full-size Lance under the unchanged `Hand_R` contract.

- [ ] **Step 7: Stop services and clean temporary evidence**

Stop only the preview/browser processes started for this Journey, remove `/tmp/rpg102-*` and `/tmp/rpg334-*`, preserve normal game services, and confirm no private IDs or licensed files entered public history.
