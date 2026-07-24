# Unarmed Character Animation Promotion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build one private, inventory-controlled promotion workflow for the four approved unarmed character checkpoints, then prove the released assets in the web client.

**Architecture:** `stage` creates a complete disposable release tree and never changes canonical assets. The sole public-evidence producer commits a fresh, run-specific evidence directory before an independent review. Only `validate` may create release metadata; only a PASS validation may be applied to a clean, isolated asset worktree and committed once.

**Tech Stack:** Python 3 stdlib, Blender 5 `bpy`, existing `swap_atlas.py`, `animation_qa.py`, `build_mesh_stats.py`, Git, GitHub CLI, Node ESM with `sharp` and `omggif`.

## Global Constraints

- Track the work at https://github.com/KirkDiggler/rpg-project/issues/119 and review this design/plan at https://github.com/KirkDiggler/rpg-project/pull/120. Commit `82d4fc5` references issue #119.
- Product changes are private `rpg-game-assets` content only. Public evidence is committed only on `rpg-dnd5e-web` branch `evidence/asset-pipeline-wave1` under `playtest-evidence/asset-pipeline/unarmed-character-animation-promotion/`; it contains no licensed GLB, texture, source blend, or private report.
- The four canonical A files are `characters/fighter.glb`, `characters/barbarian.glb`, `characters/monk.glb`, and `characters/rogue.glb`; their B/C/D files use `-b`, `-c`, and `-d`. No downed file, resolver, or web product code changes.
- Each A/B/C/D GLB has exactly the class's three ordered idle clips plus `Walk_Forward`, with the retained idles semantically identical to baseline. `idleClips` is `Idle_Relaxed`, class-distinctive idle, `Idle_Drinking`; `Walk_Forward` is not an idle entry. Set every class weapon `bakedIntoModel` to `false` while retaining its standalone weapon file, socket fields, and weapons catalog.
- Root is exactly `Root`, scale `[0.009999999776482582,0.009999999776482582,0.009999999776482582]`, rotation `[0.70710688829422,0,0,0.7071066498756409]`, with one identity-local child owning the source assembly. The assembly/body is `SK_BR_Character_Slayer_01`, `SK_BR_Character_BarbarianGiant_01`, `SK_Character_Mystic_01`, or `SK_Character_DarkElf_01` for fighter, barbarian, monk, or rogue respectively.
- Config names only logical source files; invocation requires `--checkpoint-root` and `--atlas-root`. The complete portable config is below.
- Semantic validation uses decoded glTF semantics, not indices: constrained baseline diff, interpolation-aware action fingerprints, exact action set, A/B/C/D parity, one body mesh/no extra mesh, Root contract, baseline topology/material ownership, manifest/docs gate, and authoritative mesh/animation reports. Name checks alone never prove weapon absence.
- `build_mesh_stats.py` must support a staged `--repo-root` while preserving the existing no-argument output bytes and `harness/models/synty/...` report paths. Animation QA accepts `--paths` and `--input-root`, writes paths relative to the input root, and parses changed-model PASS verdicts rather than treating exit status as the verdict.
- Metadata records baseline SHA, workflow/config SHA-256, evidence commit/SHA/review URL/ID/verdict, reports, gate results, and sorted release inventory hashes. It excludes `metadataPath` from its inventory to prevent recursive self-hashing. A release commit SHA is recorded externally in the asset PR/final gate/rollback issue, never in metadata.
- Baseline and asset-release worktrees are clean and at metadata `baselineCommit`. Before the release commit, failure discards the isolated worktree and staging tree; no backup is a rollback. After publication, rollback is a new PR that sets `RELEASE_COMMIT` to the published SHA, runs `git revert "$RELEASE_COMMIT"`, then asset sync and client verification.
- Every GitHub review or gate comment ends exactly `— asset-pipeline agent, on behalf of KirkDiggler`.

## File Map

- Create `scripts/configs/character-promotion/unarmed-checkpoints-v1.json`: portable class, clip, Root, canonical, atlas, and portrait contract.
- Create `scripts/character_promotion_validation.py`: glTF semantics, manifests/docs, report, evidence-review, inventory, and index validation primitives.
- Create `scripts/export_checkpoint_character.py` and `scripts/character_action_transfer.py`: Blender 5 export and retained-action transfer.
- Create `scripts/promote_character_checkpoints.py`: `stage`, `validate`, `apply`, and `verify-index`; `validate` is the sole metadata producer.
- Create `scripts/render_character_portraits.py` and `scripts/render_character_portrait_contact_sheet.mjs`: staged-only portrait evidence producers.
- Reuse unchanged `scripts/animation_qa_render.py` and `scripts/render_animation_evidence.mjs`: their established multi-angle CLI and GIF validation behavior is the Task 4 animation evidence producer contract; do not replace either script.
- Modify `scripts/build_mesh_stats.py`, `scripts/animation_qa.py`, `README.md`, and `harness/models/synty/characters/manifest.json`; add focused tests beside the corresponding scripts.

## Portable Config

```json
{
  "schemaVersion": 1,
  "workflowVersion": "unarmed-promotion-v1",
  "metadataPath": "harness/models/synty/characters/unarmed-promotion-v1.json",
  "root": {"name": "Root", "scale": [0.009999999776482582, 0.009999999776482582, 0.009999999776482582], "rotation": [0.70710688829422, 0, 0, 0.7071066498756409]},
  "atlases": {"b": "FantasyRivals_Texture_01_B.png", "c": "FantasyRivals_Texture_01_C.png", "d": "FantasyRivals_Texture_01_D.png"},
  "classes": {
    "fighter": {"checkpoint": "fighter-unarmed-idle-walk-approved.blend", "canonical": {"a": "characters/fighter.glb", "b": "characters/fighter-b.glb", "c": "characters/fighter-c.glb", "d": "characters/fighter-d.glb"}, "body": "SK_BR_Character_Slayer_01", "assembly": "SK_BR_Character_Slayer_01", "weaponNodes": ["SM_Wep_Slayer_01"], "authoritative": ["Idle_Relaxed", "Walk_Forward"], "retained": ["Idle_Stretch", "Idle_Drinking"], "final": ["Idle_Relaxed", "Idle_Stretch", "Idle_Drinking", "Walk_Forward"], "portraits": {"a": "characters/portraits/fighter.png", "b": "characters/portraits/fighter-b.png", "c": "characters/portraits/fighter-c.png", "d": "characters/portraits/fighter-d.png"}},
    "barbarian": {"checkpoint": "barbarian-unarmed-idle-walk-approved.blend", "canonical": {"a": "characters/barbarian.glb", "b": "characters/barbarian-b.glb", "c": "characters/barbarian-c.glb", "d": "characters/barbarian-d.glb"}, "body": "SK_BR_Character_BarbarianGiant_01", "assembly": "SK_BR_Character_BarbarianGiant_01", "weaponNodes": ["SM_Wep_BarbarianGiant_01"], "authoritative": ["Idle_Relaxed", "Walk_Forward"], "retained": ["Idle_ChinScratch", "Idle_Drinking"], "final": ["Idle_Relaxed", "Idle_ChinScratch", "Idle_Drinking", "Walk_Forward"], "portraits": {"a": "characters/portraits/barbarian.png", "b": "characters/portraits/barbarian-b.png", "c": "characters/portraits/barbarian-c.png", "d": "characters/portraits/barbarian-d.png"}},
    "monk": {"checkpoint": "monk-unarmed-idle-walk-approved.blend", "canonical": {"a": "characters/monk.glb", "b": "characters/monk-b.glb", "c": "characters/monk-c.glb", "d": "characters/monk-d.glb"}, "body": "SK_Character_Mystic_01", "assembly": "SK_Character_Mystic_01", "weaponNodes": ["SM_Wep_Mystic_01"], "authoritative": ["Idle_Relaxed", "Walk_Forward"], "retained": ["Idle_Meditative", "Idle_Drinking"], "final": ["Idle_Relaxed", "Idle_Meditative", "Idle_Drinking", "Walk_Forward"], "portraits": {"a": "characters/portraits/monk.png", "b": "characters/portraits/monk-b.png", "c": "characters/portraits/monk-c.png", "d": "characters/portraits/monk-d.png"}},
    "rogue": {"checkpoint": "rogue-unarmed-idle-walk-approved.blend", "canonical": {"a": "characters/rogue.glb", "b": "characters/rogue-b.glb", "c": "characters/rogue-c.glb", "d": "characters/rogue-d.glb"}, "body": "SK_Character_DarkElf_01", "assembly": "SK_Character_DarkElf_01", "weaponNodes": ["SM_Wep_DarkElf_01"], "authoritative": ["Idle_Relaxed", "Walk_Forward"], "retained": ["Idle_CheckWatch", "Idle_Drinking"], "final": ["Idle_Relaxed", "Idle_CheckWatch", "Idle_Drinking", "Walk_Forward"], "portraits": {"a": "characters/portraits/rogue.png", "b": "characters/portraits/rogue-b.png", "c": "characters/portraits/rogue-c.png", "d": "characters/portraits/rogue-d.png"}}
  }
}
```

## Exact CLI Journey

Run these commands from the private `rpg-game-assets` worktree unless a command explicitly uses `$EVIDENCE_WT`. This is the only release order: evidence worktree setup, `stage`, evidence production/public commit, independent review URL, `validate`, `apply`, inventory-driven add, `verify-index`, one release commit, then PR/client verification.

```bash
EVIDENCE_WT=/tmp/opencode/evidence-unarmed
git -C /home/kirk/game-dev/rpg-dnd5e-web worktree add "$EVIDENCE_WT" evidence/asset-pipeline-wave1
test "$(git -C "$EVIDENCE_WT" branch --show-current)" = evidence/asset-pipeline-wave1
test -z "$(git -C "$EVIDENCE_WT" status --porcelain)"

python3 scripts/promote_character_checkpoints.py stage --config scripts/configs/character-promotion/unarmed-checkpoints-v1.json --repo-root "$PWD" --checkpoint-root "$SYNTY_APPROVED_CHECKPOINT_DIR" --atlas-root "$SYNTY_FANTASY_RIVALS_TEXTURE_DIR" --stage-root tmp/unarmed-v1

npm install --prefix scripts --no-save --no-package-lock sharp omggif
RUN_ID="$(git rev-parse --short HEAD)-unarmed-v1"
EVIDENCE_RUN="$EVIDENCE_WT/playtest-evidence/asset-pipeline/unarmed-character-animation-promotion/$RUN_ID"
test ! -e "$EVIDENCE_RUN" && mkdir -p "$EVIDENCE_RUN/animation"
blender -b -P scripts/render_character_portraits.py -- --input-root tmp/unarmed-v1/release --manifest tmp/unarmed-v1/release/harness/models/synty/characters/manifest.json --classes fighter barbarian monk rogue --out-root tmp/unarmed-v1/release/harness/models/synty/characters
node scripts/render_character_portrait_contact_sheet.mjs --manifest tmp/unarmed-v1/release/harness/models/synty/characters/manifest.json --input-root tmp/unarmed-v1/release/harness/models/synty --out "$EVIDENCE_RUN/portraits.png" --sha-out "$EVIDENCE_RUN/portraits.sha256"
for class in fighter barbarian monk rogue; do for clip in Idle_Relaxed Walk_Forward; do for angle in 0 45 90 180; do out="tmp/unarmed-v1/evidence/${class}-${clip}-${angle}"; blender -b -P scripts/animation_qa_render.py -- --character "tmp/unarmed-v1/release/harness/models/synty/characters/${class}.glb" --clip "$clip" --out-dir "$out" --frames 28 --wrap-frames 2 --width 256 --height 320 --angle "$angle" --delay-cs 4; node scripts/render_animation_evidence.mjs --frames-dir "$out" --out-strip "${out}.png" --out-gif "${out}.gif" --title "$class $clip angle $angle" --verdict PASS; done; done; done
cp tmp/unarmed-v1/evidence/*.{png,gif} "$EVIDENCE_RUN/animation/"
test "$(find "$EVIDENCE_RUN/animation" -name '*.png' | wc -l)" -eq 32
test "$(find "$EVIDENCE_RUN/animation" -name '*.gif' | wc -l)" -eq 32
(cd "$EVIDENCE_WT" && sha256sum "${EVIDENCE_RUN#"$EVIDENCE_WT"}"/portraits.png "${EVIDENCE_RUN#"$EVIDENCE_WT"}"/animation/* > "${EVIDENCE_RUN#"$EVIDENCE_WT"}"/evidence.sha256 && git add "${EVIDENCE_RUN#"$EVIDENCE_WT"}" && git commit -m "evidence: unarmed character animation views" && git push origin evidence/asset-pipeline-wave1)
EVIDENCE_COMMIT="$(git -C "$EVIDENCE_WT" rev-parse HEAD)"

# Task 1 creates the ready, non-draft private asset PR and exports these identifiers.
test -n "$ASSET_PR_URL" && test -n "$ASSET_PR_NUMBER"
test "$(gh pr view "$ASSET_PR_NUMBER" --repo KirkDiggler/rpg-game-assets --json isDraft --jq .isDraft)" = false
gh pr comment "$ASSET_PR_NUMBER" --repo KirkDiggler/rpg-game-assets --body "$(printf 'EVIDENCE REVIEW\nevidenceCommit: %s\nportraits: PASS (16 viewed)\nanimationAnatomy: PASS (64 artifacts viewed; fighter,barbarian,monk,rogue; Idle_Relaxed,Walk_Forward; 0,45,90,180)\n— asset-pipeline agent, on behalf of KirkDiggler' "$EVIDENCE_COMMIT")"
EVIDENCE_REVIEW_URL="$(gh api "repos/KirkDiggler/rpg-game-assets/issues/$ASSET_PR_NUMBER/comments" | jq -er --arg sha "$EVIDENCE_COMMIT" 'map(select(.body == ("EVIDENCE REVIEW\\nevidenceCommit: " + $sha + "\\nportraits: PASS (16 viewed)\\nanimationAnatomy: PASS (64 artifacts viewed; fighter,barbarian,monk,rogue; Idle_Relaxed,Walk_Forward; 0,45,90,180)\\n— asset-pipeline agent, on behalf of KirkDiggler"))) | if length == 1 then .[0].html_url else error("expected exactly one matching EVIDENCE REVIEW") end')"
python3 scripts/promote_character_checkpoints.py validate --config scripts/configs/character-promotion/unarmed-checkpoints-v1.json --repo-root "$PWD" --stage-root tmp/unarmed-v1 --baseline-commit "$(git rev-parse HEAD)" --portrait-evidence-sha "$EVIDENCE_RUN/portraits.sha256" --evidence-commit "$EVIDENCE_COMMIT" --evidence-review-url "$EVIDENCE_REVIEW_URL"
python3 scripts/promote_character_checkpoints.py apply --repo-root "$PWD" --stage-root tmp/unarmed-v1 --baseline-commit "$(git rev-parse HEAD)"
git add --pathspec-from-file=tmp/unarmed-v1/release/pathspec.nul --pathspec-file-nul
python3 scripts/promote_character_checkpoints.py verify-index --repo-root "$PWD" --stage-root tmp/unarmed-v1
git diff --cached --check && git diff --cached --stat
git commit -m "asset: promote unarmed character animation checkpoints"
```

Task 1 creates the ready, non-draft private asset PR as the required review surface; Task 6 updates it with the release commit. The independent reviewer, not the implementer, runs the two review commands above after viewing every artifact. The URL is rejected unless GitHub returns exactly that review comment. No command in Tasks 1-4 invokes `validate`, `apply`, `verify-index`, or creates a release commit.

### Task 1: Config And Pure Semantic Validators

**Files:** Create the config, `scripts/character_promotion_validation.py`, and `scripts/test_character_promotion_validation.py`.

**Interfaces:** Produces `semantic_glb(gltf, bin_data) -> dict`, `animation_fingerprint(gltf, bin_data, animation) -> dict`, `validate_candidate(baseline, candidate, weapon_nodes) -> list[str]`, `validate_retained_actions(baseline, candidate, names) -> list[str]`, `validate_variant_parity(paths, expected) -> list[str]`, `validate_manifest_docs(baseline_manifest, candidate_manifest, baseline_readme, candidate_readme, config) -> list[str]`, and `validate_evidence_review(evidence_commit, comment_url) -> dict`. Task 5 consumes these pure validators.

- [ ] At Task 1 start, create the separate private asset implementation issue, add it to Board 19, set `Team=Assets` and `Feature=Class Kits`, and create the clean isolated worktree from latest `main`:

```bash
ASSET_REPO=KirkDiggler/rpg-game-assets
ASSET_ISSUE_URL="$(gh issue create --repo "$ASSET_REPO" --title "asset: promote unarmed character animation checkpoints" --body "Implements https://github.com/KirkDiggler/rpg-project/issues/119 using the approved design at https://github.com/KirkDiggler/rpg-project/pull/120.\n\n— asset-pipeline agent, on behalf of KirkDiggler")"
ASSET_ISSUE_NUMBER="${ASSET_ISSUE_URL##*/}"
PROJECT_ID="$(gh api graphql -f query='query { user(login: "KirkDiggler") { projectV2(number: 19) { id } } }' --jq '.data.user.projectV2.id')"
PROJECT_ITEM_ID="$(gh project item-add 19 --owner KirkDiggler --url "$ASSET_ISSUE_URL" --format json --jq .id)"
TEAM_FIELD_ID="$(gh project field-list 19 --owner KirkDiggler --format json | jq -er '.fields[] | select(.name == "Team") | .id')"
TEAM_OPTION_ID="$(gh project field-list 19 --owner KirkDiggler --format json | jq -er '.fields[] | select(.name == "Team") | .options[] | select(.name == "Assets") | .id')"
FEATURE_FIELD_ID="$(gh project field-list 19 --owner KirkDiggler --format json | jq -er '.fields[] | select(.name == "Feature") | .id')"
FEATURE_OPTION_ID="$(gh project field-list 19 --owner KirkDiggler --format json | jq -er '.fields[] | select(.name == "Feature") | .options[] | select(.name == "Class Kits") | .id')"
gh project item-edit --id "$PROJECT_ITEM_ID" --project-id "$PROJECT_ID" --field-id "$TEAM_FIELD_ID" --single-select-option-id "$TEAM_OPTION_ID"
gh project item-edit --id "$PROJECT_ITEM_ID" --project-id "$PROJECT_ID" --field-id "$FEATURE_FIELD_ID" --single-select-option-id "$FEATURE_OPTION_ID"
ASSET_ROOT=/home/kirk/game-dev/rpg-game-assets
ASSET_BRANCH=asset/119-unarmed-character-animation-promotion
ASSET_WT=/tmp/opencode/rpg-game-assets-119-unarmed-character-animation-promotion
git -C "$ASSET_ROOT" fetch origin main
ASSET_BASELINE="$(git -C "$ASSET_ROOT" rev-parse origin/main)"
test ! -e "$ASSET_WT"
git -C "$ASSET_ROOT" worktree add -b "$ASSET_BRANCH" "$ASSET_WT" "$ASSET_BASELINE"
test "$(git -C "$ASSET_WT" rev-parse HEAD)" = "$ASSET_BASELINE"
test -z "$(git -C "$ASSET_WT" status --porcelain)"
```

- [ ] Expected outputs are an issue URL under `https://github.com/KirkDiggler/rpg-game-assets/issues/`, a Board 19 item with `Team` set to `Assets` and `Feature` set to `Class Kits`, and a clean `$ASSET_WT` on `$ASSET_BRANCH` at `$ASSET_BASELINE`.
- [ ] Write tests that reject material/topology/Root/mesh changes, an interpolation change, a retained action key/value change, variant clip/count differences, a `bakedIntoModel: true` manifest, stale baked-weapon documentation, and malformed reviewed GitHub JSON.
- [ ] Run `python3 scripts/test_character_promotion_validation.py`; expect failure before implementations exist.
- [ ] Implement semantic node/skin/mesh/material/image/accessor decoding, constrained weapon-subtree removal, interpolation-aware target-bone fingerprints, A/B/C/D parity, exact manifest/docs rules, report-path/verdict checks, inventory SHA-256 checks, and fixture-backed review parsing. The real review lookup parses the comment ID from `EVIDENCE_REVIEW_URL`, calls `gh api repos/KirkDiggler/rpg-game-assets/issues/comments/$COMMENT_ID`, and requires the exact evidence SHA, `portraits: PASS (16 viewed)`, `animationAnatomy: PASS (64 artifacts viewed; fighter,barbarian,monk,rogue; Idle_Relaxed,Walk_Forward; 0,45,90,180)`, and signature.
- [ ] Run `python3 scripts/test_character_promotion_validation.py`; expect PASS. Commit the first tested workflow/config change, push it, and open the required ready non-draft PR with live evidence-pending checklist status:

```bash
git -C "$ASSET_WT" add scripts/configs/character-promotion/unarmed-checkpoints-v1.json scripts/character_promotion_validation.py scripts/test_character_promotion_validation.py
git -C "$ASSET_WT" commit -m "feat: validate semantic character promotion contracts"
git -C "$ASSET_WT" push -u origin "$ASSET_BRANCH"
ASSET_PR_URL="$(gh pr create --repo "$ASSET_REPO" --head "$ASSET_BRANCH" --base main --title "asset: promote unarmed character animation checkpoints" --body "Implements $ASSET_ISSUE_URL for https://github.com/KirkDiggler/rpg-project/issues/119 and https://github.com/KirkDiggler/rpg-project/pull/120.\n\nChecklist: Task 1 PASS; Task 2 pending; Task 3 pending; Task 4 evidence pending; Task 5 release pending; Task 6 client gate pending.\n\n— asset-pipeline agent, on behalf of KirkDiggler")"
ASSET_PR_NUMBER="${ASSET_PR_URL##*/}"
test "$(gh pr view "$ASSET_PR_NUMBER" --repo "$ASSET_REPO" --json isDraft --jq .isDraft)" = false
```

- [ ] Expected outputs are a pushed Task 1 commit, a ready non-draft `$ASSET_PR_URL`, and its captured `$ASSET_PR_NUMBER`; Task 4 uses both values for the independent `EVIDENCE REVIEW` comment.

### Task 2: Blender 5 Retained-Action Transfer And Unarmed Export

**Files:** Create `scripts/export_checkpoint_character.py`, `scripts/character_action_transfer.py`, and `scripts/test_character_action_transfer.py`.

**Interfaces:** Produces `assign_armature_action(armature, action)`, `copy_named_action(source, target, name)`, `validate_export_scene(armature, body) -> dict`, exporter `--out --body --stats-out --clip`, and transfer `--target --baseline --out --body --retain`. Task 3 calls these through `stage`.

- [ ] Write a Blender-backed monk re-export test that transfers `Idle_Meditative` and `Idle_Drinking`, then asks Task 1's validator to prove retained semantics and exactly four final clips.
- [ ] Run `python3 scripts/test_character_action_transfer.py --checkpoint-dir "$SYNTY_APPROVED_CHECKPOINT_DIR" --canonical-root harness/models/synty --stage-dir tmp/action-smoke`; expect failure before scripts exist.
- [ ] Implement Blender 5 slot binding: clear NLA tracks, require exactly one action slot, set `armature.animation_data.action` and `action_slot`. Export only the configured target armature/body, remove only `Icosphere`, reject every additional mesh, capture source topology/material stats, import the baseline only to copy configured retained actions, remove imported baseline objects/actions, and export `Idle_Relaxed`, `Walk_Forward`, and retained clips exactly once.
- [ ] Re-run the command; expect PASS and the four configured clips. Commit `feat: transfer retained character actions with Blender slots`.

### Task 3: Full Staged Release Producer

**Files:** Create `scripts/promote_character_checkpoints.py`, `scripts/render_character_portraits.py`, and tests; modify `scripts/build_mesh_stats.py`, `scripts/animation_qa.py`, and their tests.

**Interfaces:** Consumes Tasks 1-2 and source roots. Produces only `stage/release`: all four staged A/B/C/D GLBs; staged manifest/docs/weapon metadata; conditional regenerated A/B/C/D portraits and `portrait-render-v1.json`; staged mesh-stat and animation-QA reports. It does not validate the complete release, materialize canonical files, or commit.

- [ ] Write tests for `stage`: it calls export/transfer for A, uses `swap_atlas.py` for B/C/D without hand-maintained geometry/actions, keeps staged files repo-relative, generates mesh/animation reports, and regenerates all four portraits for a class only when inspection flags a weapon in its current portrait.
- [ ] Write builder tests that assert unchanged no-argument output bytes and staged `--repo-root stage/release --output stage/release/harness/models/synty/mesh-stats.json` output names `harness/models/synty/characters/monk.glb`; write QA tests for relative paths and parsed PASS verdicts.
- [ ] Run the focused tests; expect failure before stage/report interfaces exist.
- [ ] Implement `stage` to build the entire disposable release tree before any evidence command, mutate the staged manifest/docs only, run report producers against that tree, and preserve retained/non-goal assets. Implement portrait rendering as transparent 512x512 A/B/C/D output with input/output hashes. Keep `sharp`/`omggif` ESM imports installed through `npm install --prefix scripts --no-save --no-package-lock sharp omggif`.
- [ ] Run `python3 scripts/test_promote_character_checkpoints.py stage && python3 scripts/test_build_mesh_stats.py && python3 scripts/test_animation_qa.py`; expect PASS. Commit `feat: stage complete unarmed character release`.

### Task 4: Public Evidence And Independent EVIDENCE REVIEW

**Files:** Create `scripts/render_character_portrait_contact_sheet.mjs` and its tests. Reuse unchanged `scripts/animation_qa_render.py` and `scripts/render_animation_evidence.mjs`; no interface change is required, and their existing CLI/GIF validation behavior remains authoritative.

**Interfaces:** Consumes the complete Task 3 stage. Produces the single public `$EVIDENCE_RUN`, `portraits.png`, `portraits.sha256`, exactly 32 PNG and 32 GIF multi-angle artifacts, `evidence.sha256`, pushed `$EVIDENCE_COMMIT`, and `$EVIDENCE_REVIEW_URL`. Task 5 consumes all six. This is the only multi-angle producer.

- [ ] Write tests that verify contact-sheet ordering fighter/barbarian/monk/rogue then a/b/c/d, labels `${class} ${color}`, SHA-256 contents, and one PNG/GIF pair per class, `Idle_Relaxed`/`Walk_Forward`, and angle `0`/`45`/`90`/`180`.
- [ ] Run `node scripts/test_render_character_portrait_contact_sheet.mjs`; expect failure before producers exist.
- [ ] Implement the deterministic ESM contact sheet only. Reuse the existing animation render and GIF assembly scripts unchanged, then execute every concrete producer command in **Exact CLI Journey** beginning with `npm install --prefix scripts --no-save --no-package-lock sharp omggif` and ending with `git push origin evidence/asset-pipeline-wave1`; do not alter staged/canonical assets.
- [ ] Confirm the fresh directory contains the portrait sheet, its SHA, 32 PNGs, 32 GIFs, and `evidence.sha256`; capture `EVIDENCE_COMMIT` only after the evidence push.
- [ ] An independent reviewer views the sheet and all 64 animation artifacts, then posts the required comment on the ready non-draft `$ASSET_PR_URL` created in Task 1 and provides its URL.

- [ ] The independent reviewer runs the `gh pr comment` command in **Exact CLI Journey**, which interpolates the captured `$EVIDENCE_COMMIT`; then record its URL and ID with the following `gh api` command.

- [ ] Record the returned comment URL and ID. If portraits fail, regenerate only the affected class's staged A/B/C/D portraits, rebuild the sheet, repeat all Task 4 producers, commit a new fresh evidence run, and obtain a new review before continuing. Commit `feat: publish reproducible unarmed evidence`.

### Task 5: Validate, Apply, And Release

**Files:** Modify `scripts/promote_character_checkpoints.py` and `scripts/test_promote_character_checkpoints.py`.

**Interfaces:** Consumes Task 3's stage and Task 4's evidence commit/SHA/review URL. `validate` produces metadata, validated inventory, and no canonical changes. A PASS `validate` enables `apply`; `apply` produces canonical materialization and `pathspec.nul`; `verify-index` consumes the Git index and metadata. Task 6 consumes the one release commit.

- [ ] Write red tests proving `validate` refuses missing/stale evidence or a GitHub comment mismatch; `apply` refuses absent metadata/inventory, dirty or wrong-baseline worktrees, and changed staged files; conditional portraits are copied only if inventoried; and `verify-index` rejects any extra/missing path or staged blob SHA mismatch.
- [ ] Run `python3 scripts/test_promote_character_checkpoints.py`; expect failure before flow functions exist.
- [ ] Implement `validate` to fetch and verify the real review comment, run every Task 1 semantic/report/manifest/docs gate against all staged A/B/C/D files, then write metadata and sorted inventory hashes excluding metadata. Implement `apply` to require PASS metadata and exact baseline, copy only inventory paths, write metadata, and emit NUL-delimited sorted `inventory.paths UNION {metadataPath}`. Implement `verify-index` to require exactly that staged set and compare `git show :<path>` SHA-256s to inventory while separately validating metadata.
- [ ] Execute the `validate`, `apply`, inventory-driven `git add`, and `verify-index` commands in **Exact CLI Journey**, in that order. Expect `validate` before `apply`, no canonical edit before apply, and a verified index matching metadata.
- [ ] Add a real Git integration test: create baseline/release commits, set `RELEASE_COMMIT` to the release SHA, run `git revert --no-edit "$RELEASE_COMMIT"`, and assert the complete tracked inventory tree hashes equal the baseline. Document the durable rollback process: new rollback PR, `git revert "$RELEASE_COMMIT"`, asset sync, and client verification.
- [ ] Run `python3 scripts/test_promote_character_checkpoints.py && git diff --cached --check`; expect PASS and no whitespace errors. Create exactly one release commit: `asset: promote unarmed character animation checkpoints`.

### Task 6: Asset PR, Web Client Verification, And Independent Final Gate

**Files:** No new evidence producers. Update only the private asset PR description and, if operationally required, an external final-gate record.

**Interfaces:** Consumes the Task 5 release commit, its metadata/inventory, and the Task 4 public evidence/review. Produces the asset PR, synced web verification evidence, and independent final-gate result. It never creates pre-release visual evidence.

- [ ] Push the single release commit on the private `rpg-game-assets` branch and update the existing ready non-draft asset PR with issue #119, PR #120, baseline/release SHAs, metadata inventory/hash result, evidence commit/SHA/review URL, licensing boundary, and rollback command.
- [ ] Only after the release commit, run `npm run assets:sync && npm run test:run && npm run ci-check` in `rpg-dnd5e-web`; expect all commands to exit 0. Do not change the resolver. File any consumer defect as a separate web issue.
- [ ] Independently verify the actual client for fighter, barbarian, monk, and rogue: Home, character select, Play, lobby, Start, EncounterView, `Idle_Relaxed`, `Walk_Forward`, then idle; confirm the promoted canonical paths are loaded.
- [ ] Set `RELEASE_COMMIT="$(git rev-parse HEAD)"`, dispatch an independent final reviewer to compare `git show --name-only "$RELEASE_COMMIT"` with metadata inventory, confirm the Task 4 evidence/review, inspect the client result, and post a signed `GATE REVIEW` on the asset PR. Require PASS before merge.

## Plan Audit

- [ ] `stage` occurs only in Task 3 and before every evidence producer.
- [ ] All portrait and multi-angle producer commands occur only in Task 4; Task 6 consumes their published commit and never regenerates them.
- [ ] Complete release validation occurs only in Task 5, after Task 4 evidence commit and independent comment URL; no Task 1-4 command validates, applies, verifies index, or commits a release.
- [ ] `apply` consumes PASS metadata and inventory from `validate`; inventory-driven `git add` precedes `verify-index`; one release commit follows it.
- [ ] Every task declares what it consumes and produces; the config, semantic validators, manifest/docs gate, mesh-stat compatibility, dynamic portraits, ESM dependencies, licensing, evidence hashes/provenance, and revert process are covered without duplicate producers.
