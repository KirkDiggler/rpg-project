# Crypt Monsters Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Privately export and validate all seven approved standing crypt-monster GLBs, then make the existing tomb resolver PR consume the merged asset contract and release through the normal web-main pipeline.

**Architecture:** A fresh private `rpg-game-assets` branch exports each approved local Blender checkpoint through the existing `scripts/export_checkpoint_character.py`; a small data-driven crypt validator proves the checkpoint-to-GLB contract and guards the untouched downed siblings. The existing web PR #594 remains a narrow, deterministic resolver: its hand-maintained table mirrors the private NPC manifest and selects only Soldier01 for skeletons and Knight for skeleton-captains. Asset review and merge are a hard prerequisite for web work; web-main merge is the only normal deployment trigger.

**Tech Stack:** Blender 5.0.1 and Python stdlib GLB validators in private `rpg-game-assets`; glTF/GLB; Vitest, TypeScript, React Three Fiber, Vite, and Playwright/browser verification in `rpg-dnd5e-web`; GitHub Actions Docker publish and `rpg-deployment` workflow dispatch.

## Global Constraints

- The coordinating session is PM/QA/verification only: persistent implementation agents create worktrees, edit, commit, push, open PRs, and post GitHub activity.
- Begin asset implementation from current `origin/main` in a new persistent private `rpg-game-assets` worktree and branch; never reuse merged PR #28 or `/home/kirk/recovery-worktrees/crypt-monsters-assets` for implementation.
- Use `scripts/export_checkpoint_character.py` for every standing export; do not create a second exporter.
- The seven approved checkpoint inputs are local licensed files under `/home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/`; source checkpoints and generated GLBs stay local or in private `rpg-game-assets`, never in `rpg-project` or public `rpg-dnd5e-web`.
- Each standing output contains exactly ordered `Idle_Relaxed` `[2,55]`, then `Walk_Forward` `[2,33]`; no `Idle_Base`, `Take 001`, or other clips are permitted.
- Each output must retain `Root`, exactly one named source body mesh, its `SyntyAtlas` material, unchanged geometry/weights, finite bounds, no constraints or helper/junk nodes, and the character Root transform `[0.01,0.01,0.01]` plus `[0.70710688829422,0,0,0.7071066498756409]` rotation.
- Replace only the seven standing files under `harness/models/synty/npcs/`; all seven `-downed.glb` siblings must have identical SHA-256 values before and after the change.
- Phase 1 runtime selection is only `dnd5e:monsters:skeleton` -> deterministic `skeleton-soldier-01.glb` and `dnd5e:monsters:skeleton-captain` -> `skeleton-knight.glb`; Slave, Ghost01, Ghost02, and Tormented Soul remain unselectable.
- Preserve v1alpha2 `monsterRefId` precedence over v1alpha1 `MonsterType`; preserve the designed #595 downed-fallback behavior rather than expanding it in this PR.
- Do not commit `public/models/synty/**` in the public web repository; `npm run assets:sync` populates its ignored local copy.
- Every GitHub comment ends exactly `— asset-pipeline agent, on behalf of KirkDiggler`.
- No manual deployment dispatch or empty commit is allowed unless the normal web-main run is diagnosed as failed and the human explicitly approves recovery.

---

## Approved Batch Contract

The approved batch manifest is `/home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/manifest.json`, visual approval is `/home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/visual-approval.json` (approved 2026-07-25), and all checkpoints are Blender 5.0.1 scenes with exactly `Root` and the listed source mesh. Both clips have 47 expected live retarget constraints before baking, then zero constraints in the checkpoint/export scene.

| ID                    | Checkpoint                                                                               | SHA-256                                                            | Source body                     | Runtime standing output                             | Existing downed sibling                                    | Source GLB SHA-256                                                 |
| --------------------- | ---------------------------------------------------------------------------------------- | ------------------------------------------------------------------ | ------------------------------- | --------------------------------------------------- | ---------------------------------------------------------- | ------------------------------------------------------------------ |
| `skeleton-soldier-01` | `/home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/skeleton-soldier-01.blend` | `320507646077c49b5d99624958a0e0542467d5434bf523d4568f7f4471eb6b8d` | `Character_Skeleton_Soldier_01` | `harness/models/synty/npcs/skeleton-soldier-01.glb` | `harness/models/synty/npcs/skeleton-soldier-01-downed.glb` | `fc8bbac9a4a41d5f21001cd4ef0228cdb7c6025cbdc8ecfa6fa0808e710254a0` |
| `skeleton-soldier-02` | `/home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/skeleton-soldier-02.blend` | `0b3079d5ba9acdd169e2fc55a1c2a623ec9e04b17bcb74011bfe2beb3c808c33` | `Character_Skeleton_Soldier_02` | `harness/models/synty/npcs/skeleton-soldier-02.glb` | `harness/models/synty/npcs/skeleton-soldier-02-downed.glb` | `fd5653444d0092ebf45ad4f1d1d0a293421d510dced773fe5d5f36b1724c6a34` |
| `skeleton-slave-01`   | `/home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/skeleton-slave-01.blend`   | `0c295fe8c63cd701a380632261129515525b598afa758daf4b5a2571c8d9488a` | `Character_Skeleton_Slave_01`   | `harness/models/synty/npcs/skeleton-slave-01.glb`   | `harness/models/synty/npcs/skeleton-slave-01-downed.glb`   | `ec499e42e69942be8bf25434ac1082271b2160dd47b63c02c29e47c4ad863062` |
| `skeleton-knight`     | `/home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/skeleton-knight.blend`     | `1a7186e48a30dfddec130ff10dc955ab59188aff590165f9e404010c82207ec0` | `Character_Skeleton_Knight`     | `harness/models/synty/npcs/skeleton-knight.glb`     | `harness/models/synty/npcs/skeleton-knight-downed.glb`     | `691453f0dfb2322c2e21ee35cd42b99ab36ac3919f53f4d944ab751aa67dba31` |
| `ghost-01`            | `/home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/ghost-01.blend`            | `8544731f1f14039dcd79c00e2373ea161201a7a962937d1e8a98411d3b20cec3` | `Character_Ghost_01`            | `harness/models/synty/npcs/ghost-01.glb`            | `harness/models/synty/npcs/ghost-01-downed.glb`            | `b62cb86a450e18703a1909e8c6627ea3e9d527dbab166289283612750d561e97` |
| `ghost-02`            | `/home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/ghost-02.blend`            | `94552ca20d07129fa977a2a809b044520fefeae50e87f4eb265a422140c0f31c` | `Character_Ghost_02`            | `harness/models/synty/npcs/ghost-02.glb`            | `harness/models/synty/npcs/ghost-02-downed.glb`            | `e07722db46ee111bbe13ccad4fea76c31714753d1006fc93b2a8feeb903d0b51` |
| `tormented-soul`      | `/home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/tormented-soul.blend`      | `8bfc9c00a055459fa02628960cfec05a1f12b421c21ddfbad186694001a879a5` | `Character_Tormented_Soul`      | `harness/models/synty/npcs/tormented-soul.glb`      | `harness/models/synty/npcs/tormented-soul-downed.glb`      | `a55f4118d59b38a875e8980ad41284a218165bf6ef77a158da0b1cd0c230e7ee` |

All seven use `SyntyAtlas`; raw approved geometry fingerprints are the report values: Soldier01 `3258/6955/25baa13f...`, Soldier02 `5064/10464/bb9e597b...`, Slave `4138/8592/90cee25a...`, Knight `4832/10143/b5cf2bb2...`, Ghost01 `2272/4836/680be0b7...`, Ghost02 `1844/4423/9a6efc27...`, Tormented Soul `1246/2645/73b17788...` (triangles/vertices/SHA-256 fingerprint). The idle source SHA-256 is `76dc31c92fd3ca69f28e92f1f530eac6f60ee71ea0f25d490a633f49d2088dd0`; the walk source SHA-256 is `b2081df84b35040dd5105aaae175e5e84ab0c8c97c1e9a5aee1ab45c1e82e78d`.

## File Map

### Private assets (`rpg-game-assets` fresh worktree)

- Modify: `scripts/character_promotion_validation.py` only if its existing `validate_export_structure()` cannot be composed into the crypt validator without duplicating GLB parsing.
- Create: `scripts/validate_crypt_checkpoint_exports.py` - data-driven command-line validator for the seven exported temporary GLBs, checkpoint export stats, source-body contracts, exact clip order/ranges, finite values, no junk, and downed hashes.
- Create: `scripts/test_validate_crypt_checkpoint_exports.py` - stdlib `unittest` fixtures covering every validator rejection and success path.
- Modify: `harness/models/synty/npcs/manifest.json` - retain every current rules mapping, source, `file`, `downed`, and `rulesRef`; change each seven standing entry’s `pose` and `animationClips` to `Idle_Relaxed` then `Walk_Forward` with exact ranges.
- Modify: `harness/models/synty/npcs/{skeleton-soldier-01,skeleton-soldier-02,skeleton-slave-01,skeleton-knight,ghost-01,ghost-02,tormented-soul}.glb` - only after validation succeeds; private generated outputs.
- Modify: `harness/models/synty/mesh-stats.json` - generated exclusively by `scripts/build_mesh_stats.py`.
- Create: `harness/animation-qa/crypt-monsters-export-report.json` - private generated animation QA report for the seven exported standing files.
- Create: `harness/evidence/crypt-monsters-export/` - private controlled exported-GLB renders, render manifests, and Soldier01 approved-source comparison; do not put any of these files in public repositories.

### Public web (`rpg-dnd5e-web`, existing PR #594 branch after asset merge)

- Modify: `src/components/hex-grid/monsterModels.ts` - retain the existing `resolveMonsterModelUrl(monsterRefId, monsterType, isDowned): string | undefined` interface and narrow table; correct comments from `Idle_Base`/three skeleton looks to the merged two-clip metadata and deterministic Soldier01 candidate.
- Modify: `src/components/hex-grid/monsterModels.test.ts` - retain and update deterministic resolver, precedence, fallback, unselectable roster, and downed-url expectations.
- Modify only if stale assertions require it: `src/components/hex-grid/HexEntity.test.ts` - preserve `shouldTiltDeadOrDowned(isDead, isDowned, hasResolvedModel)` and assert a resolved dead monster uses its authored downed sibling without tilt.
- Modify only if stale prose names old clips: `src/components/hex-grid/HexEntity.tsx` - no behavior expansion; keep `isDead` passed as the monster downed signal and preserve the #595 missing-downed sibling limitation.
- Generated but never committed: `public/models/synty/**` from `npm run assets:sync`.
- Create: `docs/evidence/crypt-monsters-594-reference-tomb.md` - public-safe browser evidence only: route, commit SHAs, asset-sync confirmation, screenshots, test commands, and observed standing/moving/dead results; no licensed source/checkpoint/generated asset content.

### Deployment verification only (`rpg-deployment`)

- Do not modify files. Inspect `.github/workflows/deploy.yml` run dispatched by web `.github/workflows/docker.yml` after #594 merges to `main`.

### Tracking only (`rpg-project`)

- Modify: `ideas/crypt-monsters/plan.md` only in this design PR. The later 15-reference coverage item is a non-blocking tracking task; it creates no toolkit refs, assets, resolver entries, or web coverage test in this plan.

## Interfaces

```python
# scripts/export_checkpoint_character.py (existing; do not replace)
def export_character(out: str, body_name: str, stats_out: str, clips: list[str]) -> dict: ...

# scripts/validate_crypt_checkpoint_exports.py (new only if current validators lack these checks)
def validate_export(
    glb_path: Path, stats_path: Path, body: str, expected_down_hash: str | None
) -> list[str]: ...

# Command interface
python scripts/validate_crypt_checkpoint_exports.py \
  --spec /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/manifest.json \
  --exports-dir /tmp/crypt-monsters-exports \
  --npcs-dir harness/models/synty/npcs \
  --downed-hashes /tmp/crypt-monsters-downed-before.sha256
```

The validator returns process status `0` and prints `crypt export validation: PASS (7 standing, 7 downed unchanged)` only when all contracts pass; otherwise it returns non-zero with one exact failing ID/path/condition per line. It reuses `character_promotion_validation.read_glb`, `semantic_glb`, and `validate_export_structure` rather than parsing GLB container details twice.

```ts
export function resolveMonsterModelUrl(
  monsterRefId: string | undefined,
  monsterType: MonsterType | undefined,
  isDowned: boolean,
): string | undefined;

// Required Phase 1 results
resolveMonsterModelUrl("skeleton", undefined, false);
// '/models/synty/npcs/skeleton-soldier-01.glb'
resolveMonsterModelUrl("skeleton-captain", undefined, true);
// '/models/synty/npcs/skeleton-knight-downed.glb'
resolveMonsterModelUrl("ghost", undefined, false);
// undefined
```

### Task 1: Create the Fresh Private Asset Worktree and Baseline

**Files:**

- Create: a persistent private worktree outside the shared checkout, such as `/home/kirk/recovery-worktrees/rpg-game-assets-128-crypt-monsters`
- No repository file changes.

**Interfaces:**

- Consumes: current `origin/main` in `/home/kirk/game-dev/rpg-game-assets`.
- Produces: branch `asset/128-crypt-monsters-export` based exactly on current `origin/main`; a saved seven-file downed SHA-256 baseline outside the repository.

- [ ] **Step 1: Have the persistent asset agent fetch and establish the new branch from current main**

```bash
git -C /home/kirk/game-dev/rpg-game-assets fetch origin
git -C /home/kirk/game-dev/rpg-game-assets worktree add -b asset/128-crypt-monsters-export /home/kirk/recovery-worktrees/rpg-game-assets-128-crypt-monsters origin/main
git -C /home/kirk/recovery-worktrees/rpg-game-assets-128-crypt-monsters status --short
git -C /home/kirk/recovery-worktrees/rpg-game-assets-128-crypt-monsters merge-base --is-ancestor origin/main HEAD
```

Expected: clean status and exit status `0`; do not use branch `asset/559-undead-roster-promotion` or its recovery worktree.

- [ ] **Step 2: Capture and verify the byte-identity baseline for all downed siblings**

```bash
sha256sum harness/models/synty/npcs/skeleton-soldier-01-downed.glb harness/models/synty/npcs/skeleton-soldier-02-downed.glb harness/models/synty/npcs/skeleton-slave-01-downed.glb harness/models/synty/npcs/skeleton-knight-downed.glb harness/models/synty/npcs/ghost-01-downed.glb harness/models/synty/npcs/ghost-02-downed.glb harness/models/synty/npcs/tormented-soul-downed.glb > /tmp/crypt-monsters-downed-before.sha256
sha256sum --check /tmp/crypt-monsters-downed-before.sha256
```

Expected: seven `: OK` lines. Save this file only in `/tmp` or the agent’s private evidence directory, not in a public repository.

- [ ] **Step 3: Verify every approved checkpoint against the local batch manifest before any export**

```bash
sha256sum /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/skeleton-soldier-01.blend /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/skeleton-soldier-02.blend /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/skeleton-slave-01.blend /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/skeleton-knight.blend /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/ghost-01.blend /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/ghost-02.blend /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/tormented-soul.blend
```

Expected: values exactly match the Approved Batch Contract table. Stop if any value differs; do not export an unapproved checkpoint.

### Task 2: Add the Minimal Crypt Export Validator With TDD

**Files:**

- Create: `scripts/validate_crypt_checkpoint_exports.py`
- Create: `scripts/test_validate_crypt_checkpoint_exports.py`
- Modify only if needed for a reusable pure helper: `scripts/character_promotion_validation.py`

**Interfaces:**

- Consumes: existing `read_glb(path) -> tuple[dict, bytes]`, `semantic_glb(gltf, binary) -> dict`, and `validate_export_structure(candidate, stats) -> dict` from `character_promotion_validation.py`; exporter `*-stats.json`; batch-manifest checkpoint identifiers.
- Produces: `validate_export(glb_path, stats_path, body, expected_down_hash) -> list[str]`, which accepts only one `Root`, one exact body, one `SyntyAtlas`, matching geometry/skin semantics, exact animation array order/ranges, finite POSITION/bounds/animation values, and no helper or non-body mesh nodes.

- [ ] **Step 1: Write failing unit tests for the complete seven-model contract**

```python
def test_accepts_exact_two_clip_contract_and_rejects_junk_clip():
    candidate, stats = exported_fixture(body="Character_Skeleton_Soldier_01")
    candidate["animations"] = [
        animation("Idle_Relaxed", 2.0, 55.0),
        animation("Walk_Forward", 2.0, 33.0),
    ]
    assert validate_export_semantics(candidate, stats, "Character_Skeleton_Soldier_01") == []
    candidate["animations"].append(animation("Take 001", 1.0, 2.0))
    assert "animations must equal ['Idle_Relaxed', 'Walk_Forward']" in validate_export_semantics(candidate, stats, "Character_Skeleton_Soldier_01")

def test_rejects_nonfinite_bounds_extra_mesh_and_wrong_material():
    candidate, stats = exported_fixture(body="Character_Ghost_01")
    candidate["meshes"]["Character_Ghost_01"]["primitives"][0]["material"] = "Other"
    assert "body material names differ" in validate_export_semantics(candidate, stats, "Character_Ghost_01")
    candidate, stats = exported_fixture(body="Character_Ghost_01")
    candidate["meshes"]["Character_Ghost_01"]["primitives"][0]["attributes"]["POSITION"]["values"][0][0] = float("nan")
    assert "non-finite POSITION value" in validate_export_semantics(candidate, stats, "Character_Ghost_01")
    candidate, stats = exported_fixture(body="Character_Ghost_01")
    candidate["nodes"]["Icosphere"] = {"mesh": "Icosphere", "children": []}
    candidate["meshes"]["Icosphere"] = {"primitives": [], "weights": None}
    assert "export must contain exactly one named body mesh" in validate_export_semantics(candidate, stats, "Character_Ghost_01")
```

- [ ] **Step 2: Run the new test file before implementation**

Run: `python -m unittest scripts/test_validate_crypt_checkpoint_exports.py -v`

Expected: FAIL because `validate_export_semantics` and the command module do not exist.

- [ ] **Step 3: Implement the smallest validator around existing semantic helpers**

```python
EXPECTED_CLIPS = [("Idle_Relaxed", 2.0, 55.0), ("Walk_Forward", 2.0, 33.0)]

def validate_export_semantics(candidate, stats, body):
    errors = list(promotion_validation.validate_export_structure(candidate, stats)["errors"])
    animations = candidate.get("animations", {})
    if list(animations) != [name for name, _, _ in EXPECTED_CLIPS]:
        errors.append("animations must equal ['Idle_Relaxed', 'Walk_Forward']")
    # Decode each animation sampler input, require min/max frame values above,
    # and reject non-finite POSITION, bounds, and animation accessor values.
    return errors
```

Do not implement a Blender export operator or duplicate `read_glb`, `semantic_glb`, geometry comparison, or animation reordering. The command loads each exported GLB, its exporter stats JSON, and the known body name, then also compares `sha256sum --check` output for downed files.

- [ ] **Step 4: Run focused validator tests and existing promotion-validator regression tests**

Run: `python -m unittest scripts/test_validate_crypt_checkpoint_exports.py scripts/test_character_promotion_validation.py -v`

Expected: all tests pass; the output names the passing test count and has no traceback.

- [ ] **Step 5: Commit the validator-only change**

```bash
git add scripts/validate_crypt_checkpoint_exports.py scripts/test_validate_crypt_checkpoint_exports.py scripts/character_promotion_validation.py
git commit -m "test: validate crypt checkpoint exports"
```

Expected: one private asset commit containing only validator code/tests and no GLBs.

### Task 3: Export, Validate, and Promote the Seven Standing GLBs

**Files:**

- Modify: the seven standing GLBs listed in the Approved Batch Contract.
- Modify: `harness/models/synty/npcs/manifest.json`
- Modify: `harness/models/synty/mesh-stats.json`
- Create: `harness/animation-qa/crypt-monsters-export-report.json`

**Interfaces:**

- Consumes: Task 2 validator and existing `export_character`; local approved checkpoints; current NPC manifest keys `skeletonSoldier01`, `skeletonSoldier02`, `skeletonSlave01`, `skeletonKnight`, `ghost01`, `ghost02`, `tormentedSoul`.
- Produces: exactly seven replacement standing GLBs; manifest `animationClips: ["Idle_Relaxed", "Walk_Forward"]`, with `pose` describing ranges `Idle_Relaxed [2,55]` and `Walk_Forward [2,33]`; refreshed mesh statistics.

- [ ] **Step 1: Export all seven checkpoints into an untracked temporary directory, never directly over canonical GLBs**

```bash
mkdir -p /tmp/crypt-monsters-exports
blender -b /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/skeleton-soldier-01.blend -P scripts/export_checkpoint_character.py -- --out /tmp/crypt-monsters-exports/skeleton-soldier-01.glb --body Character_Skeleton_Soldier_01 --stats-out /tmp/crypt-monsters-exports/skeleton-soldier-01-stats.json --clip Idle_Relaxed --clip Walk_Forward
blender -b /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/skeleton-soldier-02.blend -P scripts/export_checkpoint_character.py -- --out /tmp/crypt-monsters-exports/skeleton-soldier-02.glb --body Character_Skeleton_Soldier_02 --stats-out /tmp/crypt-monsters-exports/skeleton-soldier-02-stats.json --clip Idle_Relaxed --clip Walk_Forward
blender -b /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/skeleton-slave-01.blend -P scripts/export_checkpoint_character.py -- --out /tmp/crypt-monsters-exports/skeleton-slave-01.glb --body Character_Skeleton_Slave_01 --stats-out /tmp/crypt-monsters-exports/skeleton-slave-01-stats.json --clip Idle_Relaxed --clip Walk_Forward
blender -b /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/skeleton-knight.blend -P scripts/export_checkpoint_character.py -- --out /tmp/crypt-monsters-exports/skeleton-knight.glb --body Character_Skeleton_Knight --stats-out /tmp/crypt-monsters-exports/skeleton-knight-stats.json --clip Idle_Relaxed --clip Walk_Forward
blender -b /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/ghost-01.blend -P scripts/export_checkpoint_character.py -- --out /tmp/crypt-monsters-exports/ghost-01.glb --body Character_Ghost_01 --stats-out /tmp/crypt-monsters-exports/ghost-01-stats.json --clip Idle_Relaxed --clip Walk_Forward
blender -b /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/ghost-02.blend -P scripts/export_checkpoint_character.py -- --out /tmp/crypt-monsters-exports/ghost-02.glb --body Character_Ghost_02 --stats-out /tmp/crypt-monsters-exports/ghost-02-stats.json --clip Idle_Relaxed --clip Walk_Forward
blender -b /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/tormented-soul.blend -P scripts/export_checkpoint_character.py -- --out /tmp/crypt-monsters-exports/tormented-soul.glb --body Character_Tormented_Soul --stats-out /tmp/crypt-monsters-exports/tormented-soul-stats.json --clip Idle_Relaxed --clip Walk_Forward
```

Expected: fourteen files in `/tmp/crypt-monsters-exports`; each exporter invocation completes without `post-export structure invalid`.

- [ ] **Step 2: Run the full re-import/semantic/downed validation before copying any output**

Run: `python scripts/validate_crypt_checkpoint_exports.py --spec /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/manifest.json --exports-dir /tmp/crypt-monsters-exports --npcs-dir harness/models/synty/npcs --downed-hashes /tmp/crypt-monsters-downed-before.sha256`

Expected: `crypt export validation: PASS (7 standing, 7 downed unchanged)`. It verifies `Root` + exact source body, `SyntyAtlas`, root/body relationship, skin/weights and checkpoint geometry, animation names/order/ranges, finite bounds, and absence of constraints/helpers/junk.

- [ ] **Step 3: Copy only validated standing outputs into the private harness**

```bash
cp /tmp/crypt-monsters-exports/skeleton-soldier-01.glb harness/models/synty/npcs/skeleton-soldier-01.glb
cp /tmp/crypt-monsters-exports/skeleton-soldier-02.glb harness/models/synty/npcs/skeleton-soldier-02.glb
cp /tmp/crypt-monsters-exports/skeleton-slave-01.glb harness/models/synty/npcs/skeleton-slave-01.glb
cp /tmp/crypt-monsters-exports/skeleton-knight.glb harness/models/synty/npcs/skeleton-knight.glb
cp /tmp/crypt-monsters-exports/ghost-01.glb harness/models/synty/npcs/ghost-01.glb
cp /tmp/crypt-monsters-exports/ghost-02.glb harness/models/synty/npcs/ghost-02.glb
cp /tmp/crypt-monsters-exports/tormented-soul.glb harness/models/synty/npcs/tormented-soul.glb
sha256sum --check /tmp/crypt-monsters-downed-before.sha256
```

Expected: seven downed `OK` lines. `git diff --name-only` must show only the seven standing GLBs before metadata generation.

- [ ] **Step 4: Update manifest metadata without changing mappings or downed references**

For each of `skeletonSoldier01`, `skeletonSoldier02`, `skeletonSlave01`, `skeletonKnight`, `ghost01`, `ghost02`, and `tormentedSoul`, set:

```json
"pose": "two baked clips: Idle_Relaxed [2,55], then in-place Walk_Forward [2,33] (55-bone rig; exported from approved checkpoint)",
"animationClips": ["Idle_Relaxed", "Walk_Forward"]
```

Keep `rulesRef` values unchanged: Soldiers/Slave remain `dnd5e:monsters:skeleton`, Knight remains `dnd5e:monsters:skeleton-captain`, and Ghost01/Ghost02/Tormented Soul remain explicit `null`. Keep every `file`, `downed`, source name, root-wrapper statement, and `forwardAxis` entry unchanged.

- [ ] **Step 5: Regenerate stats and focused animation QA**

```bash
python scripts/build_mesh_stats.py --repo-root . --output harness/models/synty/mesh-stats.json
python scripts/animation_qa.py --harness-dir harness --repo-root . --only npcs/ --report harness/animation-qa/crypt-monsters-export-report.json --summary
sha256sum --check /tmp/crypt-monsters-downed-before.sha256
```

Expected: mesh-stats prints `Wrote harness/models/synty/mesh-stats.json`; QA has 14 passing standing clips and no errors for the seven standing NPC outputs; downed hashes still all pass. Budget warnings remain warn-only and must be recorded rather than suppressed.

- [ ] **Step 6: Re-run all private automated gates before commit**

Run: `python -m unittest scripts/test_validate_crypt_checkpoint_exports.py scripts/test_character_promotion_validation.py scripts/test_animation_qa.py scripts/test_build_mesh_stats.py -v && python scripts/validate_crypt_checkpoint_exports.py --spec /home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/manifest.json --exports-dir harness/models/synty/npcs --npcs-dir harness/models/synty/npcs --downed-hashes /tmp/crypt-monsters-downed-before.sha256`

Expected: all unit tests pass and final validator prints its seven-standing/seven-downed PASS line. If the validator requires adjacent `*-stats.json`, point it at `/tmp/crypt-monsters-exports` for stats while it reads the promoted GLBs from `harness/models/synty/npcs`.

- [ ] **Step 7: Commit private assets and metadata**

```bash
git add harness/models/synty/npcs/manifest.json harness/models/synty/npcs/skeleton-soldier-01.glb harness/models/synty/npcs/skeleton-soldier-02.glb harness/models/synty/npcs/skeleton-slave-01.glb harness/models/synty/npcs/skeleton-knight.glb harness/models/synty/npcs/ghost-01.glb harness/models/synty/npcs/ghost-02.glb harness/models/synty/npcs/tormented-soul.glb harness/models/synty/mesh-stats.json harness/animation-qa/crypt-monsters-export-report.json
git commit -m "asset: export approved crypt monster locomotion"
```

Expected: no source blend, local report, `/tmp` artifact, or downed GLB is staged.

### Task 4: Create Controlled Private Evidence and Stop at the Asset Merge Gate

**Files:**

- Create: `harness/evidence/crypt-monsters-export/**` in private `rpg-game-assets`.
- No public repository file changes.

**Interfaces:**

- Consumes: Task 3 promoted GLBs, approved checkpoint blends, `scripts/animation_qa.py`, `scripts/render_character.py`, and `scripts/render_animation_evidence.mjs`.
- Produces: reviewable private visual proof for all exported standing GLBs plus a Soldier01 approved-source comparison.

- [ ] **Step 1: Render controlled exported-GLB evidence for each clip**

Render `Idle_Relaxed` at frames 2, 28, and 55 and `Walk_Forward` at frames 2, 17, and 33 from the seven exported GLBs, with fixed camera, lighting, scale, ground, and forward axis. Use `scripts/render_character.py --frame` for stills and `scripts/render_animation_evidence.mjs` for contact strips/GIFs after generating its expected frame manifest. Store output only under `harness/evidence/crypt-monsters-export/`.

Expected: each model visibly has relaxed idle, forward in-place walk, floor contact, +Z facing, intact geometry/weapons, and no T-pose/deformation; Ghost evidence retains its approved authored material treatment.

- [ ] **Step 2: Make a Soldier01 source comparison**

Render the approved source checkpoint `/home/kirk/game-dev/assets/synty/crypt-monsters/review/batch/skeleton-soldier-01.blend` and exported `harness/models/synty/npcs/skeleton-soldier-01.glb` at matching Idle frames 2/28/55 and Walk frames 2/17/33. Place paired files and a short private manifest stating the checkpoint SHA-256 `320507646077c49b5d99624958a0e0542467d5434bf523d4568f7f4471eb6b8d`, output path, and observed agreement.

Expected: the comparison proves the exported runtime asset matches the approved source presentation rather than only proving that the exporter completed.

- [ ] **Step 3: Commit private evidence and open the private asset PR**

```bash
git add harness/evidence/crypt-monsters-export
git commit -m "docs: add crypt monster export evidence"
git push -u origin asset/128-crypt-monsters-export
gh pr create --base main --head asset/128-crypt-monsters-export --title "asset: export approved crypt monster locomotion" --body-file /tmp/crypt-monsters-asset-pr.md
```

Expected: private PR describes the seven exact files, checkpoint/clip contract, downed hash proof, validator/QA commands, and links only permitted private evidence. It explicitly states that all Synty sources/checkpoints/generated GLBs remain private.

- [ ] **Step 4: Human merge approval checkpoint: stop**

Do not update web PR #594, merge the asset PR, or dispatch deployment. Request private asset PR review; wait for explicit human approval and merge. Any PR comment must end with `— asset-pipeline agent, on behalf of KirkDiggler`.

### Task 5: Rebase the Existing Web PR #594 After the Asset Merge

**Files:**

- Modify: `src/components/hex-grid/monsterModels.ts`
- Modify: `src/components/hex-grid/monsterModels.test.ts`
- Modify only if a stale test needs correction: `src/components/hex-grid/HexEntity.test.ts`
- Modify only if stale prose remains: `src/components/hex-grid/HexEntity.tsx`

**Interfaces:**

- Consumes: merged private asset `main`, existing PR #594 branch `feat/559-crypt-monster-models`, and current web `origin/main`.
- Produces: an updated #594 branch whose resolver behavior remains `resolveMonsterModelUrl(string | undefined, MonsterType | undefined, boolean): string | undefined`.

- [ ] **Step 1: Verify the asset merge commit before touching the web branch**

```bash
git -C /home/kirk/game-dev/rpg-game-assets fetch origin
git -C /home/kirk/game-dev/rpg-game-assets log -1 --format='%H %s' origin/main
git -C /home/kirk/game-dev/rpg-game-assets show --name-only --format= origin/main -- harness/models/synty/npcs/manifest.json
```

Expected: `origin/main` contains the asset-PR merge and `animationClips` for each seven manifest entry is exactly `["Idle_Relaxed", "Walk_Forward"]`. Stop if this is not true.

- [ ] **Step 2: Have the persistent web agent update #594 from current web main**

```bash
git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin
git -C /home/kirk/game-dev/rpg-dnd5e-web worktree add /home/kirk/recovery-worktrees/rpg-dnd5e-web-594-crypt-monsters feat/559-crypt-monster-models
git -C /home/kirk/recovery-worktrees/rpg-dnd5e-web-594-crypt-monsters rebase origin/main
```

Expected: branch retains the existing #594 commits atop current `origin/main`; resolve conflicts without changing unrelated current-main behavior.

- [ ] **Step 3: Write failing resolver tests that pin the approved Phase 1 table**

```ts
it("uses Soldier01 deterministically for skeleton and Knight for skeleton-captain", () => {
  expect(resolveMonsterModelUrl("skeleton", undefined, false)).toBe(
    "/models/synty/npcs/skeleton-soldier-01.glb",
  );
  expect(resolveMonsterModelUrl("skeleton-captain", undefined, false)).toBe(
    "/models/synty/npcs/skeleton-knight.glb",
  );
});

it("keeps asset-ready Slave and spirits unselectable in Phase 1", () => {
  expect(
    resolveMonsterModelUrl("skeleton-slave", undefined, false),
  ).toBeUndefined();
  expect(resolveMonsterModelUrl("ghost", undefined, false)).toBeUndefined();
  expect(resolveMonsterModelUrl("specter", undefined, false)).toBeUndefined();
});

it("prefers authoritative monsterRefId even when its enum fallback would map", () => {
  expect(
    resolveMonsterModelUrl("ghost", MonsterType.SKELETON, false),
  ).toBeUndefined();
});
```

- [ ] **Step 4: Run only resolver tests to confirm the stale implementation fails where expected**

Run: `npm run test:run -- src/components/hex-grid/monsterModels.test.ts`

Expected: if the existing PR still permits a stale source assumption, the new test fails; if its behavior is already correct, it passes and only prose/metadata comments need correction. Do not manufacture a behavior change merely to force a red test.

- [ ] **Step 5: Apply the narrow resolver correction and preserve designed fallbacks**

Keep the table exactly:

```ts
const MONSTER_REF_MODELS: Record<string, string[]> = {
  skeleton: ["skeleton-soldier-01.glb"],
  "skeleton-captain": ["skeleton-knight.glb"],
};
```

Keep `MONSTER_TYPE_TO_REF_ID` only for `MonsterType.SKELETON` and `MonsterType.SKELETON_CAPTAIN`; preserve trimmed/lowercased ref IDs, authoritative-ref-wins behavior, `withDownedSuffix`, and undefined fallback to `MediumHumanoid`. Update stale `Idle_Base`, three-look, and pre-export comments to say `Idle_Relaxed [2,55]` then `Walk_Forward [2,33]`. Do not select Slave/Ghost01/Ghost02/Tormented Soul and do not implement #595’s second-tier standing fallback.

- [ ] **Step 6: Extend the dead-model regression only if current test coverage does not already prove it**

```ts
it("does not tilt a dead monster with an authored resolved downed GLB", () => {
  expect(shouldTiltDeadOrDowned(true, false, true)).toBe(false);
});
```

Do not change `HexEntity.tsx` behavior: it must call `resolveMonsterModelUrl(monsterRefId, monsterType, isDead)`, pass `isDead` as `isDownedVariant`, and tilt only fallback `MediumHumanoid` output.

- [ ] **Step 7: Run focused tests and commit the web correction**

Run: `npm run test:run -- src/components/hex-grid/monsterModels.test.ts src/components/hex-grid/HexEntity.test.ts`

Expected: PASS with resolver mapping, authoritative ref precedence, enum fallback when ref absent, unmapped fallback, downed URL, and no-double-tilt cases covered.

```bash
git add src/components/hex-grid/monsterModels.ts src/components/hex-grid/monsterModels.test.ts src/components/hex-grid/HexEntity.tsx src/components/hex-grid/HexEntity.test.ts
git commit -m "fix(monsters): align crypt resolver with approved exports"
```

Stage only files actually modified; do not add ignored `public/models/synty` files.

### Task 6: Sync Assets, Verify the Reference Tomb, and Stop at the Web Merge Gate

**Files:**

- Create: `docs/evidence/crypt-monsters-594-reference-tomb.md`
- Generated only: `public/models/synty/**`

**Interfaces:**

- Consumes: merged `rpg-game-assets/main`, `npm run assets:sync`, current #594 branch, local reference content `/home/kirk/game-dev/dungeon-content/reference-tomb.yaml`.
- Produces: public-safe evidence that the reference tomb renders skeleton standing/moving/dead and skeleton-captain standing/moving/dead with synced runtime assets.

- [ ] **Step 1: Sync from private asset main and prove the files are ignored**

```bash
npm run assets:sync
test -f public/models/synty/npcs/skeleton-soldier-01.glb
test -f public/models/synty/npcs/skeleton-knight.glb
git check-ignore public/models/synty/npcs/skeleton-soldier-01.glb
git status --short --ignored public/models/synty/npcs
```

Expected: sync reports that its destination mirrors private harness assets; both files exist; `git check-ignore` exits `0`; no `public/models/synty` file is staged or committed.

- [ ] **Step 2: Run complete web validation**

```bash
npm run test:run
npm run typecheck
npm run build
npm run ci-check
```

Expected: all commands exit `0`. `npm run ci-check` is mandatory before push; do not use `--no-verify`.

- [ ] **Step 3: Run the route and capture browser evidence**

Start the existing local stack according to the web repository’s current local-dev guide. In the existing browser session, load the reference-tomb content authored at `/home/kirk/game-dev/dungeon-content/reference-tomb.yaml`; then pass the browser’s current URL to `/home/kirk/game-dev/tools/browser/screenshot.mjs` for these captures:

```bash
node /home/kirk/game-dev/tools/browser/screenshot.mjs "$(xclip -o -selection clipboard)" /tmp/crypt-monsters-reference-tomb-standing.png
node /home/kirk/game-dev/tools/browser/screenshot.mjs "$(xclip -o -selection clipboard)" /tmp/crypt-monsters-reference-tomb-moving.png
node /home/kirk/game-dev/tools/browser/screenshot.mjs "$(xclip -o -selection clipboard)" /tmp/crypt-monsters-reference-tomb-dead.png
```

Expected: standing skeleton resolves Soldier01; moving skeleton visibly plays in-place `Walk_Forward` while board movement supplies translation; dead skeleton loads its authored `-downed.glb` and is not additionally tilted; skeleton-captain resolves Knight with the same standing/moving/dead behavior. Confirm `Slave`, `Ghost01`, `Ghost02`, and `Tormented Soul` do not appear as selectable resolver results.

- [ ] **Step 4: Write safe web PR evidence and commit**

The evidence document records web SHA, merged asset SHA, exact commands/results, route parameters, and screenshot paths/observations. It must not embed, upload, or commit licensed source/checkpoint/generated GLB files.

```bash
git add docs/evidence/crypt-monsters-594-reference-tomb.md src/components/hex-grid/monsterModels.ts src/components/hex-grid/monsterModels.test.ts src/components/hex-grid/HexEntity.tsx src/components/hex-grid/HexEntity.test.ts
git commit -m "docs: verify crypt monsters on reference tomb"
git push origin feat/559-crypt-monster-models
```

- [ ] **Step 5: Human merge approval checkpoint: stop**

Update PR #594 with the test and browser evidence, ending any comment `— asset-pipeline agent, on behalf of KirkDiggler`. Stop for review and explicit human merge approval. Do not merge #594, manually dispatch deployment, or create an empty commit.

### Task 7: Verify Normal Release After Human Merges Web PR #594

**Files:**

- No changes.

**Interfaces:**

- Consumes: web `main` merge SHA; Docker workflow `.github/workflows/docker.yml`; deployment workflow `/home/kirk/game-dev/rpg-deployment/.github/workflows/deploy.yml`.
- Produces: evidence that the normal web-main pipeline cloned private assets at or after the asset merge, published the image, automatically dispatched deployment, and served the production tomb route/assets.

- [ ] **Step 1: Verify the web-main Docker run used post-asset-merge private assets**

```bash
WEB_MAIN_RUN_ID="$(gh run list --repo KirkDiggler/rpg-dnd5e-web --workflow docker.yml --branch main --limit 1 --json databaseId --jq '.[0].databaseId')"
gh run view "$WEB_MAIN_RUN_ID" --repo KirkDiggler/rpg-dnd5e-web --log
```

Expected: `Sync Synty assets from private repo` succeeded after `git clone --depth 1` of `KirkDiggler/rpg-game-assets`; inspect the clone revision/log if emitted, or reproduce `git ls-remote` and record that its main SHA is at or after the merged asset PR commit. Build/push must publish the `latest` image.

- [ ] **Step 2: Verify automatic deployment dispatch and production completion**

```bash
DEPLOYMENT_RUN_ID="$(gh run list --repo KirkDiggler/rpg-deployment --workflow deploy.yml --branch main --limit 1 --json databaseId --jq '.[0].databaseId')"
gh run view "$DEPLOYMENT_RUN_ID" --repo KirkDiggler/rpg-deployment --log
```

Expected: the deployment was triggered with source `rpg-dnd5e-web` and the web merge SHA, then completes successfully. This is the automatic dispatch from workflow lines 102-109, not a human-triggered run.

- [ ] **Step 3: Verify production assets and reference-tomb behavior**

Use the production route and browser harness to confirm the expected Soldier01 and Knight loaded assets plus standing/moving/dead behavior. Record HTTP status for `/models/synty/npcs/skeleton-soldier-01.glb`, `/models/synty/npcs/skeleton-knight.glb`, and their `-downed.glb` variants, and capture permitted screenshots.

Expected: production route and assets return successfully and behavior matches Task 6.

- [ ] **Step 4: Diagnose before any recovery action**

If the normal web-main run, image publication, dispatch, or production verification fails, collect the failing run IDs/logs and identify the failure stage. Do not run `workflow_dispatch` and do not make an empty commit. Present the diagnosis and request explicit human approval before either recovery action.

### Task 8: Track the Separate 15-Reference Coverage Initiative Without Implementing It

**Files:**

- No code, asset, or resolver changes in this implementation.

**Interfaces:**

- Consumes: Phase 1 completion evidence and the toolkit’s current 15 monster references.
- Produces: a non-blocking tracking note/issue scope for later explicit mapping-or-gap decisions and a coverage regression test.

- [ ] **Step 1: Create or update the separate tracker only after Phase 1 gates are complete**

The tracker must state: inventory all 15 current toolkit monster references; map each deliberately to an approved asset or record an explicit asset gap; add a coverage regression test in that later work; keep the 13 non-tomb references out of this asset/web delivery.

- [ ] **Step 2: Verify no scope leak**

Run: `git diff --name-only <phase-1-base>...HEAD`

Expected: no new toolkit refs, no Ghost/Slave resolver mapping, no additional GLBs, and no 15-reference coverage test are present in this Phase 1 change set.

## PR and Evidence Gates

1. The persistent asset agent owns implementation and Git flow in the new private asset worktree. Its PR may not merge until all seven checkpoint hashes, export validation, downed hash identity, mesh stats, animation QA, controlled exported-GLB evidence, and Soldier01 approved-source comparison pass review.
2. **Human approval required:** merge the private asset PR. Only after its merge commit is observable on `rpg-game-assets/main` may the persistent web agent update #594.
3. The persistent web agent owns #594 branch updates. It must run resolver tests, full tests, typecheck, build, `npm run ci-check`, asset sync, ignored-file verification, and reference-tomb browser evidence before requesting review.
4. **Human approval required:** merge web PR #594. No deployment action precedes this merge.
5. The web-main merge is the release trigger. Verify Docker cloned private assets at or after the asset merge, image publication completed, automatic deployment dispatch ran, and production tomb route/assets work.
6. Every GitHub comment, including PR updates and review requests, ends `— asset-pipeline agent, on behalf of KirkDiggler`.

## Plan Self-Review

- Spec coverage: Tasks 1-4 cover the fresh private worktree, existing exporter, seven exact standing exports, full structural/animation/downed validation, private visual proof, Soldier comparison, private PR, and asset merge gate. Tasks 5-6 cover the post-merge #594 rebase, narrow deterministic resolver, #595 preservation, TDD, sync, full checks, and tomb behavior. Task 7 covers normal deployment only and approval-gated recovery. Task 8 isolates the non-blocking 15-reference initiative.
- Placeholder scan: no incomplete markers or unspecified asset/model identifiers remain. The release commands obtain their run IDs directly from GitHub; browser capture commands use the current route URL copied by the verifier rather than inventing an unverified route shape.
- Path and type consistency: all exporter invocations use the existing `--out`, `--body`, `--stats-out`, and repeated `--clip` interface; body names, checkpoint paths, runtime paths, manifest keys, clip order/ranges, resolver signature, and web sync path match inspected current code.
- Granularity: each code task begins with a focused test, has a test command and expected result, and ends at an independently reviewable commit or explicit human gate.
