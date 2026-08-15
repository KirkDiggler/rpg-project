# Level-1 Townfolk Four-Class Runtime Roster Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: use `superpowers:subagent-driven-development` or `superpowers:executing-plans` task by task. Use TDD for every behavior change and `superpowers:verification-before-completion` before each push, PR gate, and completion claim.

**Goal:** Replace the four Fantasy Rivals class-body families with the approved Fantasy Kingdom Fighter, Monk, Rogue, and Barbarian while preserving the public class aliases, two-clip runtime behavior, standalone weapon contract, and web renderer.

**Architecture:** A private, repository-shaped stage is the only producer. It consumes four hash-bound Blender checkpoints and private 4096x4096 Kingdom A/B/C atlases, embeds deterministic 2048x2048 runtime derivatives, builds all 36 standing/downed/portrait artifacts, deletes all 12 D artifacts, regenerates metadata, and passes mechanical plus visual review before canonical materialization. The provider release is one verified Git commit. The web then syncs from a detached worktree at the exact merged provider SHA through `RPG_GAME_ASSETS_PATH`; no roster resolver change is planned.

**Tech stack:** Blender 5.0.1, Auto-Rig Pro 3.78.34, Python 3 stdlib and `bpy`, glTF/GLB, existing provider validators and catalog tools, Git/GitHub CLI, React/TypeScript/Vitest, Playwright or Chrome DevTools for real WebGL verification.

**Authority:** [design.md](design.md), [rpg-project#225](https://github.com/KirkDiggler/rpg-project/issues/225), [rpg-project PR #226](https://github.com/KirkDiggler/rpg-project/pull/226), [rpg-game-assets#59](https://github.com/KirkDiggler/rpg-game-assets/issues/59), approved checkpoint [rpg-game-assets#58](https://github.com/KirkDiggler/rpg-game-assets/issues/58).

---

## Global constraints

- Kirk alone approves creative evidence and merges PRs.
- Do not begin provider implementation until PR #226 is merged. Do not merge it on Kirk's behalf.
- Licensed FBXs, atlases, `.blend` files, and GLBs stay under private/local roots or `rpg-game-assets`; never add them to `rpg-project` or tracked `rpg-dnd5e-web` paths.
- Public evidence may contain only rendered PNG/GIF/video, checksums, commands, and provenance receipts.
- Stable A aliases remain `characters/{fighter,monk,rogue,barbarian}.glb`; B/C use `-b`/`-c`. Downed aliases and portraits retain their current spelling.
- The release contains exactly `Idle_Relaxed [2,55]` then `Walk_Forward [2,33]`. Do not call `character_action_transfer.py`; retained Fantasy Rivals idles are intentionally retired.
- Standing models contain no weapon. Preserve standalone weapon GLBs, socket transforms, and `bakedIntoModel: false` byte-for-byte/semantically.
- Private source atlases are exactly 4096x4096. Every staged standing/downed GLB embeds exactly one 2048x2048 image generated in Blender 5.0.1 with `Image.scale(2048, 2048)`; no loose derivative PNG is committed.
- Build the entire A/B/C standing, downed, and portrait family before canonical apply. A failure removes/rejects the stage and leaves canonical files unchanged.
- Every GitHub comment ends `— asset-pipeline agent, on behalf of KirkDiggler`.

## Approved class contract

| Class | Source body | Approved/private checkpoint |
|---|---|---|
| Fighter | `SK_Chr_Peasant_Male_01` | `assets/synty/review/level1-townfolk-58/peasant-male-01-idle-walk-review.blend` |
| Monk | `SK_Chr_Monk_01` | `assets/synty/review/level1-townfolk-58/monk-01-idle-walk-review.blend` |
| Rogue | `SK_Chr_Rider_01` | produced and approved in Task 2 |
| Barbarian | `SK_Chr_Blacksmith_Male_01` | produced and approved in Task 2 |

Pinned checkpoint hashes already approved:

```text
fighter/Peasant  7cb1ddf58ca38e98ba49415d3e22eb5dcd63ea618a6458a5cedbc933cb97c41c
monk             cf3ad1574dff09de73c90d7b8c4be93dfbfa4be32cbeb9140ab687aa444a350e
```

## File map

### `rpg-game-assets` — create

- `scripts/retarget_townfolk_idle_walk_arp.py`
- `scripts/configs/animation/townfolk_idle_walk_arp_profile.json`
- `scripts/test_retarget_townfolk_idle_walk_arp.py`
- `scripts/configs/character-promotion/townfolk-checkpoints-v1.json`
- `scripts/validate_townfolk_release.py`
- `scripts/test_validate_townfolk_release.py`
- `scripts/render_character_portrait.py`
- `scripts/test_render_character_portraits.py`
- `scripts/test_pose_downed.py`
- `harness/models/synty/characters/townfolk-promotion-v1.json` (generated only after validation)

### `rpg-game-assets` — modify

- `scripts/export_checkpoint_character.py`
- `scripts/swap_atlas.py`
- `scripts/pose_downed.py`
- `scripts/render_character_portraits.py`
- `scripts/promote_character_checkpoints.py`
- `scripts/character_promotion_validation.py`
- `scripts/build_mesh_stats.py`
- focused tests beside those scripts
- `harness/models/synty/characters/manifest.json`
- `harness/models/synty/npcs/manifest.json` (remove the stale “candidate Monk class body” comment only)
- `harness/models/synty/characters/portrait-render-v1.json`
- four `harness/models/synty/characters/*-stats.json` files
- `harness/models/synty/mesh-stats.json`
- `harness/animation-qa/report.json`
- `harness/animation-qa/summary.txt`
- `harness/catalogs/synty-complete-inventory.json`
- `README.md`

### `rpg-game-assets` — generated family replacement

- Replace 12 standing GLBs: four classes x A/B/C.
- Replace 12 downed GLBs: four classes x A/B/C.
- Replace 12 portrait PNGs: four classes x A/B/C.
- Delete 12 D artifacts: four standing, four downed, four portraits.
- Delete `harness/models/synty/characters/unarmed-promotion-v1.json` because it describes the retired A/B/C/D Fantasy Rivals release.
- Do not remove unrelated historic files such as `fighter-idle.glb` unless a separate inventory defect proves they are in contract scope.

### `rpg-dnd5e-web`

No renderer/resolver behavior change is expected. After the provider merge, update stale clip-contract documentation and test fixtures in:

- `src/components/hex-grid/ClassCharacterModel.tsx`
- `src/components/hex-grid/classCharacterModels.ts`
- `src/components/hex-grid/classCharacterModels.test.ts`

The narrow public PR must describe the exact two-clip release without changing lookup or playback logic. `scripts/sync-synty-assets.sh` already gained the required explicit source override through merged PR #754. Runtime evidence goes on `evidence/asset-pipeline-wave1` under:

```text
playtest-evidence/asset-pipeline/level1-townfolk-class-roster/<run-id>/
```

---

## Task 1: Merge gate, clean provider worktree, and immutable baseline

**Consumes:** merged design authority and current `rpg-game-assets/origin/main`.

**Produces:** one clean provider branch/worktree, a committed durable private baseline receipt for weapon/socket and pre-release D hashes, Board 19 item moved to In Progress.

- [ ] Verify the design and operational dependencies before creating implementation state:

```bash
test "$(gh pr view 226 -R KirkDiggler/rpg-project --json state --jq .state)" = MERGED
test "$(gh pr view 754 -R KirkDiggler/rpg-dnd5e-web --json state --jq .state)" = MERGED
test "$(gh pr view 60 -R KirkDiggler/rpg-game-assets --json state --jq .state)" = MERGED
```

Expected: three `MERGED` assertions. Stop if any assertion fails.

- [ ] Create the implementation branch from the exact current private provider main:

```bash
ASSET_ROOT=/home/kirk/game-dev/rpg-game-assets
ASSET_WT=/home/kirk/game-dev/rpg-game-assets/.worktrees/59-level1-townfolk-roster
ASSET_BRANCH=asset/59-level1-townfolk-roster
git -C "$ASSET_ROOT" fetch origin main
test ! -e "$ASSET_WT"
git -C "$ASSET_ROOT" worktree add -b "$ASSET_BRANCH" "$ASSET_WT" origin/main
test -z "$(git -C "$ASSET_WT" status --porcelain)"
ASSET_BASELINE="$(git -C "$ASSET_WT" rev-parse HEAD)"
```

Expected: clean worktree at `origin/main`; record `$ASSET_BASELINE` in the eventual PR.

- [ ] Capture immutable pre-release hashes as a durable private receipt and commit it before any producer change:

```bash
cd "$ASSET_WT"
BASELINE_RECEIPT_ROOT="$ASSET_WT/evidence/59-townfolk-promotion/baseline"
mkdir -p "$BASELINE_RECEIPT_ROOT"
sha256sum harness/models/synty/characters/weapons/*.glb \
  > "$BASELINE_RECEIPT_ROOT/weapons.sha256"
find harness/models/synty/characters -maxdepth 2 -type f \
  \( -name '*-d.glb' -o -name '*-downed-d.glb' -o -path '*/portraits/*-d.png' \) \
  -print0 | sort -z | xargs -0 sha256sum \
  > "$BASELINE_RECEIPT_ROOT/d-variants.sha256"
jq '{fighter:.mapping.fighter.weapon,monk:.mapping.monk.weapon,rogue:.mapping.rogue.weapon,barbarian:.mapping.barbarian.weapon}' \
  harness/models/synty/characters/manifest.json \
  > "$BASELINE_RECEIPT_ROOT/weapon-contract.json"
printf '{"schemaVersion":1,"baselineCommit":"%s"}\n' "$ASSET_BASELINE" \
  > "$BASELINE_RECEIPT_ROOT/baseline.json"
python3 scripts/build_synty_complete_inventory.py --check
git add "$BASELINE_RECEIPT_ROOT"
git diff --cached --check
git commit -m "chore: record townfolk promotion baseline (#59)"
```

Expected: four standalone class weapon hashes and exactly 12 D artifacts are committed in the private provider before any asset mutation; `baseline.json` names the pre-receipt `$ASSET_BASELINE`; the current complete inventory verifies. Never recapture this receipt later.

- [ ] Move `rpg-game-assets#59` to Board 19 `Status=In Progress`; preserve `Team=Assets`, `Feature=Class Kits`, and `Action=Build`.

---

## Task 2: Reproducible Kingdom retarget producer and the missing checkpoints

**Files:** create the townfolk retarget script/profile/test listed above. Reuse `scripts/configs/animation/polygon_to_rivals_bones.json`.

**Consumes:** four source FBXs, `A_Idle_Standing_Masc.fbx`, `A_Walk_F_Masc.fbx`, Auto-Rig Pro 3.78.34, and the two issue-58 oracle checkpoints.

**Produces:** reviewed Rider and Blacksmith checkpoints plus reports under private `assets/synty/review/level1-townfolk-59/`; no licensed repository additions.

### 2A. Add pure/profile tests first

- [ ] Port the proven instrumented producer behavior from the issue-58 private verification script into `retarget_townfolk_idle_walk_arp.py`. Keep paths in the external JSON profile, not Python constants.

Required profile shape:

```json
{
  "profileId": "level1-townfolk-idle-walk-arp-v1",
  "blenderVersion": "5.0.1",
  "autoRigProVersion": "3.78.34",
  "actions": [
    {"name": "Idle_Relaxed", "frames": [2, 55]},
    {"name": "Walk_Forward", "frames": [2, 33]}
  ],
  "expectedMappings": 46,
  "expectedLiveConstraints": 47,
  "expectedFinalConstraints": 0,
  "targets": []
}
```

Each target records logical source path, source SHA-256, body name, 55-bone rest hash, geometry/material fingerprint, output name, and whether an approved oracle checkpoint exists.

- [ ] Write failing tests for:
  - exact four target IDs and body names;
  - exact source/idle/walk hashes;
  - `Hips -> Pelvis` among exactly 46 mappings;
  - exactly 47 observed live constraints for each clip;
  - exact two-action names/order/ranges;
  - one `Root` armature, one body mesh, 55 bones, zero final constraints, no weapons/helpers;
  - source/final geometry, topology, UV, weights, shape keys, rest pose, and packed-image preservation;
  - refusal to overwrite an existing output or accept an unpinned input.

Run RED:

```bash
cd "$ASSET_WT"
python3 -B -m unittest -v scripts/test_retarget_townfolk_idle_walk_arp.py
```

Expected: failure because the new producer/profile do not exist.

- [ ] Implement only enough pure/profile behavior to make non-Blender tests pass. Commit code/config/tests before producing binaries:

```bash
python3 -B -m unittest -v scripts/test_retarget_townfolk_idle_walk_arp.py
git add scripts/retarget_townfolk_idle_walk_arp.py \
  scripts/configs/animation/townfolk_idle_walk_arp_profile.json \
  scripts/test_retarget_townfolk_idle_walk_arp.py
git commit -m "feat: add reproducible townfolk ARP retarget recipe"
```

### 2B. Produce Rider and Blacksmith privately

- [ ] Verify exact external inputs:

```bash
GAME_DEV=/home/kirk/game-dev
KINGDOM="$GAME_DEV/assets/synty/polygon-fantasy-kingdom/Source_Files/Characters"
IDLE="$GAME_DEV/assets/synty/animation-base-locomotion/SourceFiles/Animations/Polygon/Masculine/Idle/A_Idle_Standing_Masc.fbx"
WALK="$GAME_DEV/assets/synty/animation-base-locomotion/SourceFiles/Animations/Polygon/Masculine/Locomotion/Walk/A_Walk_F_Masc.fbx"
sha256sum \
  "$KINGDOM/SK_Chr_Peasant_Male_01.fbx" \
  "$KINGDOM/SK_Chr_Monk_01.fbx" \
  "$KINGDOM/SK_Chr_Rider_01.fbx" \
  "$KINGDOM/SK_Chr_Blacksmith_Male_01.fbx" \
  "$IDLE" "$WALK"
```

Expected: every hash exactly matches the committed profile.

- [ ] Prepare the private checkpoint root without changing the approved issue-58 bytes:

```bash
REVIEW59="$GAME_DEV/assets/synty/review/level1-townfolk-59"
CHECKPOINT_ROOT="$REVIEW59/checkpoints"
mkdir -p "$CHECKPOINT_ROOT" "$REVIEW59/reports" "$REVIEW59/evidence"
cp --reflink=auto "$GAME_DEV/assets/synty/review/level1-townfolk-58/peasant-male-01-idle-walk-review.blend" \
  "$CHECKPOINT_ROOT/fighter-peasant-male-01.blend"
cp --reflink=auto "$GAME_DEV/assets/synty/review/level1-townfolk-58/monk-01-idle-walk-review.blend" \
  "$CHECKPOINT_ROOT/monk-01.blend"
sha256sum "$CHECKPOINT_ROOT/fighter-peasant-male-01.blend" "$CHECKPOINT_ROOT/monk-01.blend"
```

Expected: hashes remain `7cb1...c41c` and `cf3a...350e`.

- [ ] Run the repository producer for Rider and Blacksmith into new private outputs:

```bash
for target in rogue-rider-01 barbarian-blacksmith-male-01; do
  blender -b -P scripts/retarget_townfolk_idle_walk_arp.py -- \
    --profile scripts/configs/animation/townfolk_idle_walk_arp_profile.json \
    --target-id "$target" \
    --character-fbx "$KINGDOM/$(jq -r --arg id "$target" '.targets[]|select(.id==$id)|.filename' scripts/configs/animation/townfolk_idle_walk_arp_profile.json)" \
    --idle-fbx "$IDLE" --walk-fbx "$WALK" \
    --bone-map scripts/configs/animation/polygon_to_rivals_bones.json \
    --output-blend "$CHECKPOINT_ROOT/$target.blend" \
    --report-json "$REVIEW59/reports/$target.json"
done
```

Expected per target: 46 mappings, four `CONSTRAINT_PROBE` observations (bind/final for two clips), bind count 47, final count 0, 55 bones, one mesh, exact two actions.

- [ ] Run the Blender-backed integration suite against all four checkpoints:

```bash
TOWNFOLK_ARP_INTEGRATION=1 GAME_DEV_ROOT="$GAME_DEV" \
  python3 -B -m unittest -v scripts/test_retarget_townfolk_idle_walk_arp.py
```

Expected: all configured input, oracle, source-preservation, and final-contract tests pass; no integration skip.

### 2C. Human checkpoint gate

- [ ] Render Rider and Blacksmith with the same fixed ground, front/side/three-quarter/high-oblique cameras and Idle/Walk frame samples used in issue #58. Generate source-vs-checkpoint reports and contact sheets under `$REVIEW59/evidence`.

- [ ] Obtain Kirk's explicit written `CHECKPOINT REVIEW: PASS` on `rpg-game-assets#59` for Rider and Blacksmith locomotion, clothing deformation, silhouette, and ground contact. Static roster approval alone does not satisfy this animation gate.

- [ ] Obtain an independent mechanical review confirming source preservation, 47-live/zero-final constraints, exact clips, and no weapons. Stop if either review fails.

---

## Task 3: Townfolk release config and semantic validators with TDD

**Files:** create `townfolk-checkpoints-v1.json`, `validate_townfolk_release.py`, and its test; extend `character_promotion_validation.py` and tests.

**Consumes:** the four reviewed checkpoint hashes and private Kingdom atlas hashes.

**Produces:** pure validators capable of rejecting an incomplete, over-budget, armed, animated-down, or D-contaminated stage.

- [ ] Write the new portable config with exactly these release semantics:

```json
{
  "schemaVersion": 1,
  "workflowVersion": "townfolk-promotion-v1",
  "metadataPath": "harness/models/synty/characters/townfolk-promotion-v1.json",
  "runtimeAtlasSize": 2048,
  "sourceAtlasSize": 4096,
  "colors": ["a", "b", "c"],
  "actions": [
    {"name": "Idle_Relaxed", "frames": [2, 55]},
    {"name": "Walk_Forward", "frames": [2, 33]}
  ],
  "classes": {
    "fighter": {"body": "SK_Chr_Peasant_Male_01"},
    "monk": {"body": "SK_Chr_Monk_01"},
    "rogue": {"body": "SK_Chr_Rider_01"},
    "barbarian": {"body": "SK_Chr_Blacksmith_Male_01"}
  }
}
```

Fill each class with checkpoint file/hash, source FBX file/hash, A/B/C standing/downed/portrait paths, and A/B/C source atlas file/hash. Do not include a D key or retained-idle list.

- [ ] Write RED tests for these interfaces:

```python
def validate_townfolk_config(config: dict) -> list[str]: ...
def validate_standing(path: Path, spec: dict) -> list[str]: ...
def validate_downed(path: Path, standing: Path, spec: dict) -> list[str]: ...
def validate_family(root: Path, config: dict) -> list[str]: ...
def validate_manifest(manifest: dict, config: dict, baseline_weapons: dict) -> list[str]: ...
def validate_target_budgets(mesh_report: dict, paths: set[str]) -> list[str]: ...
```

Tests must reject:

- any color set other than ordered A/B/C;
- missing/extra/reordered actions or wrong frame bounds;
- non-2048 embedded runtime images, multiple images, external images, or source atlases not 4096;
- changed A/B/C geometry, skin, node, material ownership, action semantics, or Root contract;
- standing weapons/helpers/additional meshes;
- downed skins, animations, armatures, extra meshes, bad Root wrapper, non-finite bounds, or ground offset outside tolerance;
- any of the 12 D filenames or a D reference in staged character manifests/metadata;
- anything other than exactly 12 standing + 12 downed + 12 portrait family artifacts;
- missing portraits, non-512 dimensions, or non-transparent PNGs;
- modified standalone weapon path/socket/`bakedIntoModel` contract;
- target-path mesh budget warnings.

Run RED:

```bash
python3 -B -m unittest -v \
  scripts/test_character_promotion_validation.py \
  scripts/test_validate_townfolk_release.py
```

- [ ] Implement by reusing `read_glb`, `semantic_glb`, `validate_export_structure`, and `validate_variant_parity`; do not write a second GLB parser. Generalize evidence receipt parsing instead of changing the old hard-coded unarmed receipt in place.

- [ ] Extend `build_mesh_stats.py` with a target-path strict gate so unchanged legacy warnings remain visible but cannot mask a townfolk failure. Preserve no-argument output bytes when no target paths are supplied.

Example interface:

```bash
python3 scripts/build_mesh_stats.py --repo-root <stage> --output <stage-report> \
  --strict-path 'characters/fighter*.glb' \
  --strict-path 'characters/monk*.glb' \
  --strict-path 'characters/rogue*.glb' \
  --strict-path 'characters/barbarian*.glb'
```

- [ ] Run GREEN and commit:

```bash
python3 -B -m unittest -v \
  scripts/test_character_promotion_validation.py \
  scripts/test_validate_townfolk_release.py \
  scripts/test_build_mesh_stats.py
git add scripts/configs/character-promotion/townfolk-checkpoints-v1.json \
  scripts/character_promotion_validation.py scripts/test_character_promotion_validation.py \
  scripts/validate_townfolk_release.py scripts/test_validate_townfolk_release.py \
  scripts/build_mesh_stats.py scripts/test_build_mesh_stats.py
git commit -m "feat: validate complete townfolk character releases"
```

---

## Task 4: Build the complete disposable stage

**Files:** modify exporter, atlas swapper, downed producer, portraits, and promoter; add focused tests.

**Consumes:** Task 2 checkpoints and Task 3 config/validators.

**Produces:** one complete, mechanically passing repository-shaped stage; canonical provider files remain byte-identical.

### 4A. Add producer tests before implementation

- [ ] Add `--atlas-size 2048` to `export_checkpoint_character.py`. Test that it finds exactly one body atlas image, scales only the in-memory image, embeds exactly one 2048x2048 PNG, and does not alter geometry/actions.

- [ ] Add `--atlas-size 2048` to `swap_atlas.py`. Test source dimension enforcement, B/C payload replacement, exact non-image semantic parity, and exact action parity.

- [ ] Refactor `pose_downed.py` so argument parsing is inside `main()` and `convert_one(input_path, output_path)` is testable. Test an explicit output path and validate the resulting static Root-wrapped, grounded, animation-free, skin-free GLB.

- [ ] Change `render_character_portraits.py` from “render only if the old portrait has a weapon” to “render every configured townfolk variant.” Add the Blender worker `render_character_portrait.py` for deterministic 512x512 transparent output. Test 12 outputs, input/output hashes, dimensions, alpha, and stable ordering.

- [ ] Extend `promote_character_checkpoints.py` with `stage`, `validate`, `apply`, and `verify-index`. Write RED tests proving:
  - stage starts from a complete canonical `harness/` copy;
  - canonical files do not change during `stage` or `validate`;
  - A uses direct two-clip checkpoint export, never `character_action_transfer.py`;
  - B/C use atlas swap with 2048 embedding;
  - downed/portraits are all regenerated;
  - all 12 D artifacts and old promotion metadata are deleted only in stage;
  - report/catalog/inventory generation happens before the stage is sealed;
  - any failed producer/validator removes the partial stage;
  - injected apply failure restores the pre-apply canonical tree byte-for-byte;
  - `verify-index` rejects missing, extra, wrong-hash, or unstaged deletions.

Run RED:

```bash
python3 -B -m unittest -v \
  scripts/test_promote_character_checkpoints.py \
  scripts/test_swap_atlas.py \
  scripts/test_pose_downed.py \
  scripts/test_render_character_portraits.py \
  scripts/test_promotion_artifacts.py
```

### 4B. Implement the repository-shaped stage

- [ ] Implement this exact stage order:

1. Copy the complete canonical `harness/` to a fresh stage.
2. Validate all checkpoint, source FBX, and source atlas hashes before launching Blender.
3. Export each palette-A checkpoint directly through `export_checkpoint_character.py --clip Idle_Relaxed --clip Walk_Forward --atlas-size 2048`.
4. Produce B/C standing siblings with `swap_atlas.py --atlas-size 2048`.
5. Produce palette-A downed from A standing; produce B/C from the exact A downed structure by atlas swap.
6. Render A/B/C portraits from staged standing GLBs.
7. Delete the 12 D artifacts and `unarmed-promotion-v1.json` from stage.
8. Rewrite only the four class mappings in `characters/manifest.json`: Kingdom source, one idle, exact A/B/C colors, preserved weapon contract.
9. Remove the stale NPC Monk-candidate comment; update README release inventory/provenance.
10. Regenerate four stats sidecars, global mesh stats, animation QA report/summary, portrait report, `townfolk-promotion-v1.json`, and the complete Synty inventory.
11. Verify the safe web catalog remains valid; regenerate it only if its deterministic bytes require it.
12. Run the complete townfolk validator and target-path budget gate.
13. Seal the stage tree digest. Do not touch canonical files.

- [ ] Run all focused producer tests GREEN and commit:

```bash
python3 -B -m unittest -v \
  scripts/test_promote_character_checkpoints.py \
  scripts/test_swap_atlas.py \
  scripts/test_pose_downed.py \
  scripts/test_render_character_portraits.py \
  scripts/test_promotion_artifacts.py \
  scripts/test_animation_qa.py \
  scripts/test_build_web_asset_catalog.py
git add scripts
git commit -m "feat: stage complete A-B-C townfolk character release"
```

- [ ] Push the tested code commits and open a **ready, non-draft** provider PR linked to #59/#225/#226. The PR is allowed to say “release stage and visual gate pending”; it must not imply assets are ready.

### 4C. Build the real stage

```bash
STAGE_ROOT="$ASSET_WT/.stage/59-townfolk"
ATLAS_ROOT="$GAME_DEV/assets/synty/polygon-fantasy-kingdom/Source_Files/Textures"
rm -rf "$STAGE_ROOT"
python3 -B scripts/promote_character_checkpoints.py stage \
  --config scripts/configs/character-promotion/townfolk-checkpoints-v1.json \
  --repo-root "$ASSET_WT" \
  --checkpoint-root "$CHECKPOINT_ROOT" \
  --atlas-root "$ATLAS_ROOT" \
  --stage-root "$STAGE_ROOT" \
  --blender blender
python3 -B scripts/validate_townfolk_release.py \
  --config scripts/configs/character-promotion/townfolk-checkpoints-v1.json \
  --stage-root "$STAGE_ROOT/release"
```

Expected final line:

```text
townfolk release validation: PASS (12 standing, 12 downed, 12 portraits, 0 D; runtime atlases 2048x2048)
```

- [ ] Recheck canonical immutability and baseline weapons using the committed receipt:

```bash
sha256sum --check "$BASELINE_RECEIPT_ROOT/weapons.sha256"
test -z "$(git status --porcelain -- harness README.md)"
```

Expected: all weapons `OK`; no canonical release diff yet.

---

## Task 5: Visual evidence, Kirk approval, and independent stage review

**Consumes:** the sealed Task 4 stage.

**Produces:** a license-safe public evidence commit, Kirk's creative approval, and an independent `EVIDENCE REVIEW: PASS` receipt bound to the stage digest.

- [ ] Create a clean evidence-branch worktree and run-specific directory:

```bash
WEB_ROOT=/home/kirk/game-dev/rpg-dnd5e-web
EVIDENCE_WT=/home/kirk/game-dev/rpg-dnd5e-web/.worktrees/evidence-townfolk-59
git -C "$WEB_ROOT" fetch origin evidence/asset-pipeline-wave1
git -C "$WEB_ROOT" worktree add "$EVIDENCE_WT" origin/evidence/asset-pipeline-wave1
RUN_ID="$(git -C "$ASSET_WT" rev-parse --short HEAD)-$(date -u +%Y%m%d)-townfolk"
EVIDENCE_RUN="$EVIDENCE_WT/playtest-evidence/asset-pipeline/level1-townfolk-class-roster/$RUN_ID"
mkdir -p "$EVIDENCE_RUN"/{animation,palettes,downed,portraits,texture-resolution}
```

- [ ] Render from the exact sealed stage:
  - Fighter/Monk/Rogue/Barbarian palette-A `Idle_Relaxed` and `Walk_Forward` at 0/45/90/180 degrees, with contact frames and high-oblique views;
  - one A/B/C standing sheet (12 models);
  - one A/B/C downed sheet (12 models);
  - one A/B/C portrait sheet (12 portraits);
  - matched 4096-source versus 2048-runtime close-ups for every class/palette;
  - stage manifest and SHA-256 receipt containing no private absolute paths.

Use existing `animation_qa_render.py`, `render_animation_evidence.mjs`, `render_character.py`, and the approved fixed-ground renderer. Do not copy GLBs, atlases, `.blend` files, or private JSON reports into `$EVIDENCE_RUN`.

- [ ] Inspect every artifact directly. Kirk must post written approval for the final four-class A/B/C standing/downed/portrait presentation and the 2048 comparison. The implementer may not self-approve creative quality.

- [ ] Commit and push only rendered/public-safe evidence:

```bash
(
  cd "$EVIDENCE_WT"
  find "${EVIDENCE_RUN#"$EVIDENCE_WT"/}" -type f -print0 | sort -z | xargs -0 sha256sum \
    > "${EVIDENCE_RUN#"$EVIDENCE_WT"/}/evidence.sha256"
  git add "${EVIDENCE_RUN#"$EVIDENCE_WT"/}"
  git diff --cached --check
  git commit -m "evidence: level-1 townfolk runtime roster"
  git push origin HEAD:evidence/asset-pipeline-wave1
)
EVIDENCE_COMMIT="$(git -C "$EVIDENCE_WT" rev-parse HEAD)"
```

- [ ] Dispatch a fresh read-only independent reviewer. Require it to:
  - verify the evidence commit contains no licensed source or converted asset files;
  - inspect all animation, palette, downed, portrait, and resolution-comparison sheets;
  - rerun pure/provider validation against the sealed stage;
  - compare the stage tree digest to the evidence receipt;
  - report blockers by severity.

- [ ] The independent reviewer posts one exact signed receipt on the provider PR:

```text
EVIDENCE REVIEW: PASS
stageTreeSha256: <digest>
evidenceCommit: <sha>
animations: PASS (4 classes; Idle_Relaxed + Walk_Forward; 4 angles)
palettes: PASS (12 standing)
downed: PASS (12 static)
portraits: PASS (12 transparent 512px)
textureResolution: PASS (4096 source compared with 2048 runtime)
licensedFilesPublished: 0
— asset-pipeline agent, on behalf of KirkDiggler
```

Capture the comment URL. Any changed stage byte invalidates this receipt and requires a new evidence run/review.

---

## Task 6: Seal, apply, verify the Git index, and release the provider PR

**Consumes:** exact sealed stage, evidence commit, Kirk approval, and independent review URL.

**Produces:** one canonical provider release commit and a fully reviewed private PR. Kirk remains the merger.

- [ ] Run `validate` to bind evidence and generate final promotion metadata/inventories inside the stage:

```bash
python3 -B scripts/promote_character_checkpoints.py validate \
  --config scripts/configs/character-promotion/townfolk-checkpoints-v1.json \
  --repo-root "$ASSET_WT" \
  --stage-root "$STAGE_ROOT" \
  --baseline-commit "$ASSET_BASELINE" \
  --evidence-commit "$EVIDENCE_COMMIT" \
  --evidence-review-url "$EVIDENCE_REVIEW_URL"
```

Expected: `VALIDATED` metadata with the baseline SHA, committed baseline-receipt path/hash, stage digest, 36 family hashes, checkpoint/source/atlas hashes, report verdicts, evidence SHA/URL, and no recursive self-hash.

- [ ] Apply only a validated stage. The implementation must use same-filesystem candidate/backup directory replacement and restore the baseline on injected failure:

```bash
python3 -B scripts/promote_character_checkpoints.py apply \
  --repo-root "$ASSET_WT" \
  --stage-root "$STAGE_ROOT" \
  --baseline-commit "$ASSET_BASELINE"
```

Expected: canonical `harness/` now matches the validated stage; backup removed only after post-copy digest verification.

- [ ] Stage exactly the validated changed/deleted path set and verify the index:

```bash
git add --pathspec-from-file="$STAGE_ROOT/release/pathspec.nul" --pathspec-file-nul
python3 -B scripts/promote_character_checkpoints.py verify-index \
  --repo-root "$ASSET_WT" --stage-root "$STAGE_ROOT"
git diff --cached --check
git diff --cached --stat
```

Expected: 36 A/B/C family replacements, 12 D deletions, old promotion metadata deletion, intended manifests/reports/catalogs/README changes, and no weapon GLB changes.

- [ ] Run all provider gates from fresh outputs:

```bash
python3 -B -m unittest discover -s scripts -p 'test_*.py' -v
TOWNFOLK_ARP_INTEGRATION=1 GAME_DEV_ROOT="$GAME_DEV" \
  python3 -B -m unittest -v scripts/test_retarget_townfolk_idle_walk_arp.py
python3 -B scripts/validate_townfolk_release.py \
  --config scripts/configs/character-promotion/townfolk-checkpoints-v1.json \
  --stage-root "$ASSET_WT"
python3 -B scripts/build_synty_complete_inventory.py --check
python3 -B scripts/build_web_asset_catalog.py --check
python3 -B scripts/verify_web_asset_stage.py --verify-only
sha256sum --check "$BASELINE_RECEIPT_ROOT/weapons.sha256"
```

Expected: all tests pass, complete inventory/catalog verify, web-stage provider contract verifies, and standalone weapons remain byte-identical.

- [ ] Create the single canonical release commit and push:

```bash
git commit -m "asset: promote level-1 townfolk class roster (#59)"
RELEASE_COMMIT="$(git rev-parse HEAD)"
git push -u origin "$ASSET_BRANCH"
```

- [ ] Update the ready provider PR with baseline/release SHAs, stage digest, exact artifact counts, deleted D list, 2048 decision, evidence commit/review URL, commands, rollback command, and licensing boundary.

- [ ] Request an independent code/release review. Require comparison of `git show --name-status "$RELEASE_COMMIT"` against the sealed inventory and a signed `GATE REVIEW: PASS`. Resolve findings through TDD and regenerate evidence if any released asset byte changes.

- [ ] Stop for Kirk to merge the provider PR. Record the resulting merge commit; never substitute the branch head for the merged SHA.

---

## Task 7: Exact-provider web sync and real four-class runtime gate

**Consumes:** the merged provider commit and merged web sync override.

**Produces:** byte-bound sync proof plus standing, moving/facing, and authentic downed browser evidence for all four classes. No web feature PR unless a real consumer defect is found.

### 7A. Narrow web contract-documentation PR

- [ ] After the provider merge, create a clean web worktree from current `origin/dev`. Update only the three files in the web file map so comments and test fixtures describe the exact `Idle_Relaxed` + `Walk_Forward` contract. Add an exact-real-shape test using `['Idle_Relaxed', 'Walk_Forward']`; do not change URL resolution or playback behavior.

```bash
WEB_DOC_WT=/home/kirk/game-dev/rpg-dnd5e-web/.worktrees/225-townfolk-two-clip-contract
WEB_DOC_BRANCH=docs/225-townfolk-two-clip-contract
git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin dev
git -C /home/kirk/game-dev/rpg-dnd5e-web worktree add -b "$WEB_DOC_BRANCH" "$WEB_DOC_WT" origin/dev
cd "$WEB_DOC_WT"
npx vitest run src/components/hex-grid/classCharacterModels.test.ts
npm run ci-check
git diff --check
git add src/components/hex-grid/ClassCharacterModel.tsx \
  src/components/hex-grid/classCharacterModels.ts \
  src/components/hex-grid/classCharacterModels.test.ts
git commit -m "docs: align class animation contract with townfolk roster (#225)"
git push -u origin "$WEB_DOC_BRANCH"
```

- [ ] Open a narrow ready PR linked to #225 and the merged provider PR. Require green CI and Kirk's merge. Capture the merged web commit as `$WEB_SHA`; no licensed runtime file may be staged.

### 7B. Detached exact-provider sync

- [ ] Resolve and verify both merged SHAs, then create isolated detached worktrees:

```bash
PROVIDER_SHA="$(gh pr view "$ASSET_PR_NUMBER" -R KirkDiggler/rpg-game-assets --json mergeCommit --jq .mergeCommit.oid)"
WEB_SHA="$(gh pr view "$WEB_DOC_PR_NUMBER" -R KirkDiggler/rpg-dnd5e-web --json mergeCommit --jq .mergeCommit.oid)"
test -n "$PROVIDER_SHA" && test -n "$WEB_SHA"
RUN_ROOT="$(mktemp -d /tmp/townfolk-225.XXXXXX)"
git -C /home/kirk/game-dev/rpg-game-assets fetch origin main
git -C /home/kirk/game-dev/rpg-game-assets worktree add --detach "$RUN_ROOT/assets" "$PROVIDER_SHA"
git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin dev
git -C /home/kirk/game-dev/rpg-dnd5e-web worktree add --detach "$RUN_ROOT/web" "$WEB_SHA"
test "$(git -C "$RUN_ROOT/assets" rev-parse HEAD)" = "$PROVIDER_SHA"
test "$(git -C "$RUN_ROOT/web" rev-parse HEAD)" = "$WEB_SHA"
```

- [ ] Install and sync using the explicit detached source:

```bash
cd "$RUN_ROOT/web"
npm ci
RPG_GAME_ASSETS_PATH="$RUN_ROOT/assets" npm run assets:sync
```

- [ ] Prove exact mirroring and D deletion:

```bash
rsync -aicn --delete "$RUN_ROOT/assets/harness/models/synty/" \
  "$RUN_ROOT/web/public/models/synty/" | tee "$RUN_ROOT/mirror.diff"
test ! -s "$RUN_ROOT/mirror.diff"
(cd "$RUN_ROOT/assets/harness/models/synty" && find . -type f -print0 | sort -z | xargs -0 sha256sum) > "$RUN_ROOT/source.sha256"
(cd "$RUN_ROOT/web/public/models/synty" && find . -type f -print0 | sort -z | xargs -0 sha256sum) > "$RUN_ROOT/dest.sha256"
cmp "$RUN_ROOT/source.sha256" "$RUN_ROOT/dest.sha256"
test -z "$(find "$RUN_ROOT/web/public/models/synty/characters" -maxdepth 2 -type f \
  \( -name '*-d.glb' -o -name '*-downed-d.glb' -o -path '*/portraits/*-d.png' \) -print)"
```

Expected: empty rsync diff, identical checksum manifests, zero D artifacts.

### 7C. Automated consumer gates

```bash
npx vitest run \
  scripts/sync-synty-assets.test.ts \
  src/components/hex-grid/classCharacterModels.test.ts \
  src/components/hex-grid/useHexMovePath.test.ts \
  src/components/hex-grid/HexEntity.test.ts \
  src/components/hex-grid/HexEntity.test.tsx \
  src/components/playtest/playtestMapHelpers.test.ts
npm run ci-check
```

Expected: all focused tests and complete web CI pass at the exact merged `$WEB_SHA`. Resolver/playback behavior remains unchanged.

### 7D. Real WebGL verification

- [ ] Pin the fixture producer before seeding. Start/record the local API/Redis service image, then seed from a detached API worktree at current reviewed `origin/main`:

```bash
git -C /home/kirk/game-dev/rpg-api fetch origin main
API_SHA="$(git -C /home/kirk/game-dev/rpg-api rev-parse origin/main)"
git -C /home/kirk/game-dev/rpg-api worktree add --detach "$RUN_ROOT/api" "$API_SHA"
cd "$RUN_ROOT/api"
go run ./cmd/devseed --fixture=equip-demo --encounter-id=townfolk-fighter
go run ./cmd/devseed --fixture=wave-2-monk --encounter-id=townfolk-monk
go run ./cmd/devseed --fixture=wave-1-rogue --encounter-id=townfolk-rogue
go run ./cmd/devseed --fixture=wave-3-barbarian --encounter-id=townfolk-barbarian
```

Record `$API_SHA` and the running API service image/container digest in the runtime receipt; do not seed from a mutable incidental checkout.

Class/player URLs:

```text
http://127.0.0.1:3001/?encounterId=townfolk-fighter&playerId=aldric
http://127.0.0.1:3001/?encounterId=townfolk-monk&playerId=charli
http://127.0.0.1:3001/?encounterId=townfolk-rogue&playerId=alice
http://127.0.0.1:3001/?encounterId=townfolk-barbarian&playerId=bob
```

- [ ] Start the detached web worktree against the local API:

```bash
cd "$RUN_ROOT/web"
VITE_API_HOST=http://localhost:8080 npm run dev -- --host 127.0.0.1
```

- [ ] Use a temporary Playwright driver, Chrome DevTools MCP, or the existing browser harness to capture for each class:
  1. stationary `Idle_Relaxed`;
  2. a genuine multi-hex click path while `Walk_Forward` is visibly active;
  3. at least two travel directions proving facing follows ordered movement;
  4. return to idle after movement;
  5. authentic `unconscious`/downed rendering reached through the turn-based fixture (reseed between destructive runs).

Do not fake the downed state by changing React props or editing the resolver. If combat cannot reach an authentic downed state, stop and file the narrow fixture/tooling gap instead of weakening the gate.

- [ ] Capture network responses, console, page errors, screenshots, and short movement video. Require HTTP 200 for exact standing/downed aliases and zero GLTF/console/page errors.

- [ ] Commit only license-safe runtime evidence and a receipt to `evidence/asset-pipeline-wave1`. The receipt records `$PROVIDER_SHA`, `$WEB_SHA`, `$API_SHA`, API service image digest, sync checksum digest, fixture/player, loaded model URLs, and observed Idle/Walk/facing/downed results.

---

## Task 8: Independent final gate, closure, and rollback receipt

**Consumes:** merged provider SHA, exact sync proof, runtime evidence, provider reviews, and web CI output.

**Produces:** signed final gate, closed #59/#225, Board 19 Done, and an updated active-session handoff.

- [ ] Dispatch a fresh independent final reviewer to verify:
  - provider PR merge SHA equals runtime receipt SHA;
  - 36 A/B/C artifacts exist and all 12 D artifacts are absent;
  - exact two-action standing contract and static downed contract;
  - 2048 runtime images and target budgets;
  - weapons/socket metadata unchanged;
  - public evidence contains no licensed binaries;
  - all four real runtime scenarios visibly pass.

- [ ] Require one signed `FINAL GATE: PASS` comment. Kirk decides whether any visual caveat is acceptable.

- [ ] Record rollback exactly:

```bash
git -C /home/kirk/game-dev/rpg-game-assets revert "$PROVIDER_SHA"
# open/merge a rollback PR, then repeat Task 7 exact-provider sync and runtime smoke
```

Never rewrite history or restore ad hoc backup files after publication.

- [ ] After PASS, close `rpg-game-assets#59` and `rpg-project#225`, move both Board 19 items to Done, and rewrite `rpg-project/sessions/active.md` with provider merge SHA, evidence URLs, test commands, and any residual risks.

---

## Plan audit

- [ ] Design PR #226 is merged before implementation.
- [ ] Peasant/Monk approved hashes remain unchanged; Rider/Blacksmith get explicit Kirk animation approval.
- [ ] The stage contains exactly 12 standing, 12 downed, 12 portraits, and no D.
- [ ] Every standing file has exactly `Idle_Relaxed` then `Walk_Forward`; every downed file is static.
- [ ] All runtime images are 2048x2048 derivatives of hash-bound 4096x4096 A/B/C sources.
- [ ] Canonical provider bytes do not change before stage, visual, and independent review gates pass.
- [ ] Apply failure tests restore canonical bytes; Git index exactly matches the sealed stage.
- [ ] Standalone weapons/socket metadata remain unchanged.
- [ ] The narrow web contract-doc/test PR is merged; comments and real-shape tests describe exactly two clips without renderer behavior changes.
- [ ] Web sync uses `RPG_GAME_ASSETS_PATH` pointed at a detached merged provider SHA and proves byte identity with delete semantics.
- [ ] Runtime evidence records exact merged web, provider, and API fixture SHAs plus the API service image digest.
- [ ] All four classes pass real Idle, Walk, facing, and authentic downed browser verification.
- [ ] No licensed binary enters a public repository.
- [ ] Kirk performs every design, provider, and web PR merge and gives final creative acceptance.
