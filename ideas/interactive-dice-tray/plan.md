# Original Carved D20 Semantic Recovery Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Correct the asset-owned Original carved d20 result identities and numeral triangle roles, then make the web independently observe the upward result and recapture readable exact-SHA Stone 0 evidence.

**Architecture:** Recovery remains inside-out. `rpg-game-assets` derives a versioned semantic witness from direct carved `D20_Result_##` tags, binds it to exact GLB triangles, corrects the unchanged model's settlement/partition metadata, and publishes `dice-runtime-presets@2`. Only after Kirk merges that provider PR does `rpg-dnd5e-web` consume the v2 witness, independently resolve the upward face from the rendered pose, and replace the circular evidence oracle with geometry-backed telemetry plus readable all-result close-ups.

**Tech Stack:** Python 3, Blender 5.x read-only tag extraction, private GLB geometry, JSON authority/runtime contracts, React 19, TypeScript 5.8, Three.js / React Three Fiber, Vitest, Testing Library, Playwright/Chromium, Vite Concepts Lab.

**Spec:** `rpg-project/ideas/interactive-dice-tray/design.md` at `b0e388dcb6563cf9c8bdb1a4a318cd676dd306d5`

## Global Constraints

- Tracking: `rpg-game-assets#57`, `rpg-dnd5e-web#751`, web PR `#752`, initiative `rpg-project#219`, design PR `#220`.
- Assets base is merged provider commit `8c32071e935df9ec60f68de820c0188d1ece0f87`; web recovery continues exact reviewed branch `asset/751-original-d20-runtime` from `b5045811e769bcd686186c930252a4c6f95d44b1` after the correcting assets PR merges.
- Work lands assets first. Kirk alone merges the assets correction and web PR. Do not start the web contract change against an unmerged provider or merge either PR automatically.
- Keep the exact GLB unchanged: 491,312 bytes, SHA-256 `87bf2d0535023e69c968fb9878ba4ad990df4eeec4b503ebb0e917419c47a77e`, 10,482 indexed triangles, one material-free mesh.
- Correct carved result identity comes only from direct `D20_Result_##` tags in `Original_D20_Source.face-tags.blend`/authority. Painted D20 labels may support plane-shape comparison but never carved result order.
- Correct d20 roles are exactly 2,684 body triangles and 7,798 numeral recess/cutwall triangles. Every set must be complete, disjoint, in range, and bound to the exact GLB.
- The correcting consumer contract is `$schemaVersion: 2`, `contract: "dice-runtime-presets"`, `generatedBy: "build_dice_runtime_manifest@2.0.0"`. V2 face entries require a runtime-geometry witness; strict web parsing rejects v1 rather than guessing.
- `library/custom-dice/` remains authoring-only. Consumers read only `harness/models/custom-dice/`; public web provider bytes remain ignored/untracked.
- Web contains no corrective result permutation, copied quaternion table, model URL, hash exception, or carved label table.
- `DiceTrayPresentation` remains the authority boundary; `presentationId` remains the sole external correlation. Gesture decoration never changes the authoritative result.
- Roller and Spectator retain independent Canvas, renderer, scene clone, generation, resources, and telemetry while consuming the same immutable provider/event facts.
- Existing target-hold angular telemetry remains useful but cannot establish face identity. 3D evidence passes only when geometry-backed `observedUpwardResult === requestedResult` for both witnesses.
- Canvas visibility is named `canvasVisible`; it must never be labeled carved-numeral correctness. Full-page screenshots and hashes alone are archival evidence, not semantic approval.
- Keep tactile held/shake motion, `VisualThrowProfile@1`, multi-die groups, rigid-body physics, production transport/loadout ownership, and performance graduation out of this recovery.
- Test-first every production behavior: write a discriminating failing test, run RED, implement minimally, rerun GREEN, then protected suites.
- Private evidence remains under `/home/kirk/game-dev/.verification/interactive-dice-tray/stone-0/`; do not stage GLBs, manifests, screenshots, JSON, logs, or frames in the public web repository.
- GitHub comments end with `— asset-pipeline agent, on behalf of KirkDiggler`.

## Current invalidated state

- Web PR #752 is open but its merge-ready verdict is withdrawn.
- Current provider request→actual upward numeral is:

```text
1→1, 2→2, 3→5, 4→6, 5→3, 6→4, 7→8, 8→7, 9→9,
10→11, 11→12, 12→10, 13→14, 14→13, 15→18, 16→17,
17→16, 18→15, 19→19, 20→20
```

- The prior 3,563/6,919 partition includes 879 cutwall triangles in body.
- Exact package `.../b5045811e769bcd686186c930252a4c6f95d44b1/` proved conformance to the supplied tuple, not carved face identity, and may not be cited as passing Stone 0 evidence.

---

### Task 1: Publish direct carved face witnesses (`rpg-game-assets#57`)

**Files:**
- Create: `scripts/dice_semantic_contract.py`
- Create: `scripts/test_dice_semantic_contract.py`
- Create: `scripts/export_carved_d20_semantics.py`
- Modify: `scripts/build_dice_tray_face_maps.py`
- Modify: `scripts/build_dice_runtime_manifest.py`
- Modify: `scripts/test_build_dice_runtime_manifest.py`
- Modify: `scripts/validate_dice_tray_presets.py`
- Modify: `scripts/test_validate_dice_tray_presets.py`
- Modify: `library/custom-dice/original-set/Original_D20_Source.face-tags.authority.json`
- Modify: `library/custom-dice/dice-tray-presets.json`
- Generate: `harness/models/custom-dice/dice-tray-presets.json`

**Interfaces:**
- Consumes: direct Blender material tags, exact authoring/runtime GLB bytes, current carved authority JSON.
- Produces: `extract_glb_triangles(path: Path) -> list[Triangle]`; `validate_carved_semantics(preset: dict, authority: dict, triangles: list[Triangle]) -> list[str]`; deterministic Blender export for direct-tag runtime ordinals; v2 runtime entries.

The v2 consumer entry is exact:

```ts
interface RuntimeFaceWitnessV2 {
  readonly kind: 'runtime-face-triangles';
  readonly faceNormal: readonly [number, number, number];
  readonly triangleIndices: readonly number[];
  readonly triangleSignatureSha256: string;
}

interface DiceSettlementFaceV2 {
  readonly faceIndex: number;
  readonly quaternion: readonly [number, number, number, number];
  readonly witness: RuntimeFaceWitnessV2;
}
```

`triangleIndices` contains the nonempty outer-plane triangles directly tagged for that carved result, is a subset of the body partition, and is disjoint from every other result witness. `triangleSignatureSha256` hashes canonical sorted vertex-coordinate signatures for those exact runtime ordinals; it is not a hash of labels or array positions.

- [ ] **Step 1: Create the isolated assets worktree**

```bash
cd /home/kirk/game-dev/rpg-game-assets
git fetch origin
test "$(git rev-parse origin/main)" = 8c32071e935df9ec60f68de820c0188d1ece0f87
git worktree add .worktrees/57-original-d20-semantic-recovery \
  -b asset/57-original-d20-semantic-recovery origin/main
cd .worktrees/57-original-d20-semantic-recovery
git status --short --branch
```

Expected: clean tracked tree; parent checkout's `.pi/` and `.claude/worktrees/` remain untouched.

- [ ] **Step 2: Record baseline failures without changing source**

```bash
sha256sum \
  library/custom-dice/original-set/Original_D20_Source.glb \
  harness/models/custom-dice/original-set/Original_D20_Source.glb
python3 scripts/build_dice_runtime_manifest.py --check
python3 scripts/validate_dice_tray_presets.py
```

Expected: both GLBs hash to `87bf2d...a77e`; `--check` exposes the stale source-manifest binding; structural validation does not detect the semantic face permutation.

- [ ] **Step 3: Write semantic RED tests**

In `scripts/test_dice_semantic_contract.py`, use one synthetic tetrahedral fixture for unit behavior and the real d20 for integration:

```python
def test_real_carved_d20_entries_match_direct_tags(self):
    preset, authority, triangles = load_real_d20_inputs(self.repo_root)
    self.assertEqual([], validate_carved_semantics(preset, authority, triangles))


def test_permuted_settlement_is_rejected(self):
    preset, authority, triangles = synthetic_direct_tag_fixture()
    preset["faceSettlementMap"]["entries"]["1"]["quaternion"] = \
        preset["faceSettlementMap"]["entries"]["2"]["quaternion"]
    self.assertIn(
        "result 1 witness does not settle alone to world up",
        validate_carved_semantics(preset, authority, triangles),
    )
```

Also cover duplicate/missing result tags, an unmatched runtime triangle signature, non-bijective face indices/normals, witness indices outside the exact mesh, witness indices crossing results, non-unit normals/quaternions, wrong result label, and any non-target face tying the target at world-up tolerance.

Extend generator/parser tests so v1 output, missing witness keys, empty witness arrays, duplicate witness ordinals, non-body witness ordinals, wrong witness digest, and stale `sourceManifestSha256` fail.

- [ ] **Step 4: Run RED**

```bash
python3 scripts/test_dice_semantic_contract.py
python3 scripts/test_build_dice_runtime_manifest.py
python3 scripts/test_validate_dice_tray_presets.py
```

Expected: semantic tests fail because direct carved settlement witnesses do not exist and the real map is permuted; v2 projection tests fail against the v1 generator.

- [ ] **Step 5: Implement pure semantic validation**

`dice_semantic_contract.py` parses indexed GLB positions/indices without Blender, canonicalizes each triangle as three lexicographically sorted rounded glTF-space vertices, computes outward normals, and validates exported direct-tag witnesses.

For every result `r`:

```python
rotated = quaternion_rotate(entry["quaternion"], entry["witness"]["faceNormal"])
if vector_distance(rotated, [0.0, 1.0, 0.0]) > 1e-6:
    errors.append(f"result {r} witness does not settle alone to world up")
```

Rotate all other result normals by the same quaternion and require their world Y to be `< 0.8`; require the target world Y to be `> 0.999999`. Recompute witness signatures from exact runtime triangles and compare bytes.

- [ ] **Step 6: Export the direct carved authority read-only**

`export_carved_d20_semantics.py` runs under Blender without saving either `.blend` or GLB. It:

1. opens `Original_D20_Source.face-tags.blend` and object `Original_D20_FACE_TAGGED`;
2. converts Blender `(x,y,z)` to glTF `(x,z,-y)`;
3. triangulates material-tagged polygons in memory;
4. joins all 10,482 tagged/untagged triangles to runtime GLB ordinals by canonical vertex-coordinate signature;
5. derives each `D20_Result_##` outer plane, normal, body witness ordinals, face index, and settle quaternion; and
6. atomically writes only the JSON authority export requested by `--out`.

Run:

```bash
blender --background --python scripts/export_carved_d20_semantics.py -- \
  --blend library/custom-dice/original-set/Original_D20_Source.face-tags.blend \
  --glb library/custom-dice/original-set/Original_D20_Source.glb \
  --out /tmp/Original_D20_Source.face-tags.authority.generated.json
```

Expected: 10,482/10,482 coordinate signatures match; results 1–20 are unique; no source binary is written. Compare and deliberately promote the generated semantic sections into the tracked authority JSON.

- [ ] **Step 7: Stop using painted result order for carved d20**

Change `build_dice_tray_face_maps.py` so `dice.original.carved.d20` reads `settlementEntries` from the direct carved authority. Remove `sourceNode`/`original-painted-number-d20-decal-node` from carved entries. Preserve painted D20 derivation only for `dice.original.painted-numbers.d20`.

The corrected carved mapping is generated from tags and must resolve to:

```text
new result <- old plane entry:
1<-1, 2<-2, 3<-5, 4<-6, 5<-3, 6<-4, 7<-8, 8<-7, 9<-9,
10<-12, 11<-10, 12<-11, 13<-14, 14<-13, 15<-18, 16<-17,
17<-16, 18<-15, 19<-19, 20<-20
```

This table is a review oracle, not production input.

- [ ] **Step 8: Emit and validate runtime contract v2**

Update `build_dice_runtime_manifest.py` to emit schema/generator v2 and each entry's direct runtime witness. Update authoring/runtime validators for exact-key reconstruction and semantic validation. Regenerate:

```bash
python3 scripts/build_dice_runtime_manifest.py
python3 scripts/build_dice_runtime_manifest.py --check
python3 scripts/validate_dice_tray_presets.py
python3 scripts/test_dice_semantic_contract.py
python3 scripts/test_validate_dice_tray_presets.py
python3 scripts/test_build_dice_runtime_manifest.py
```

Expected: all pass; runtime source hash equals current authoring bytes; all 20 target normals settle alone to up; GLB hashes unchanged.

- [ ] **Step 9: Commit Task 1**

```bash
git diff --check
git status --short
git diff --name-only -- '*.glb' '*.blend' | test ! -s /dev/stdin
git add scripts library/custom-dice harness/models/custom-dice/dice-tray-presets.json
git commit -m "fix: bind carved d20 results to direct face tags (#57)"
```

Commission an independent task review before Task 2.

---

### Task 2: Correct numeral roles and publish assets recovery

**Files:**
- Modify: `scripts/derive_body_numeral_triangle_groups.py`
- Modify: `scripts/dice_semantic_contract.py`
- Modify: `scripts/test_dice_semantic_contract.py`
- Create: `scripts/render_carved_d20_face_audit.py`
- Modify: `library/custom-dice/original-set/Original_D20_Source.face-tags.authority.json`
- Modify: `library/custom-dice/dice-tray-presets.json`
- Generate: `harness/models/custom-dice/dice-tray-presets.json`
- Create: `evidence/57-original-d20-semantic-recovery/carved-d20-all-results-contact-sheet.png`

**Interfaces:**
- Consumes: Task 1 direct-tag/runtime ordinal bridge.
- Produces: exact semantic partition, readable 20-face private review sheet, ready assets PR.

- [ ] **Step 1: Write triangle-role RED tests**

```python
def test_real_d20_partition_matches_direct_tag_roles(self):
    semantic = load_real_d20_semantics(self.repo_root)
    self.assertEqual(2684, len(semantic.expected_body))
    self.assertEqual(7798, len(semantic.expected_numeral))
    self.assertEqual(semantic.expected_body, semantic.published_body)
    self.assertEqual(semantic.expected_numeral, semantic.published_numeral)
```

Add a synthetic regression where 272 cutwall-only normal clusters each have a local maximum; require every cutwall to remain numeral. Add mutations for one cutwall moved to body, one outer-plane triangle moved to numeral, gaps, overlap, and wrong direct-tag bridge.

- [ ] **Step 2: Run RED**

```bash
python3 scripts/test_dice_semantic_contract.py
```

Expected: real d20 fails with 3,563/6,919 and identifies 879 false-body cutwalls.

- [ ] **Step 3: Implement direct semantic roles**

For a direct-tag authority, body is exactly the union of each result's outer-plane witness triangles. Numeral is every other runtime triangle, including lower parallel recess floors and all untagged cutwalls. Do not select a body triangle merely because it is the maximum of a cutwall-only normal cluster.

`derive_body_numeral_triangle_groups.py` must:

- use the direct witness path for d20;
- validate rather than overwrite direct semantic exports;
- leave other carved dice byte-identical in this issue; and
- explicitly label their legacy cluster-derived roles as outside #57's audited scope.

- [ ] **Step 4: Regenerate and prove exact role counts**

```bash
python3 scripts/derive_body_numeral_triangle_groups.py
python3 scripts/build_dice_runtime_manifest.py
python3 scripts/build_dice_runtime_manifest.py --check
python3 scripts/validate_dice_tray_presets.py
python3 scripts/test_dice_semantic_contract.py
python3 scripts/test_validate_dice_tray_presets.py
python3 scripts/test_build_dice_runtime_manifest.py
```

Expected: d20 2,684 + 7,798 = 10,482; false-role count 0; other promoted GLB bytes and non-d20 role arrays unchanged.

- [ ] **Step 5: Generate readable all-result review evidence**

`render_carved_d20_face_audit.py` consumes the generated runtime contract, assigns the exact two runtime materials, renders each corrected result from the agreed overhead three-quarter camera at a minimum 320×320 panel, labels panels outside the die image, and builds a 4×5 contact sheet.

```bash
blender --background --python scripts/render_carved_d20_face_audit.py -- \
  --manifest harness/models/custom-dice/dice-tray-presets.json \
  --preset dice.original.carved.d20 \
  --out evidence/57-original-d20-semantic-recovery
```

Expected: 20 large panels, one requested result per panel, complete cream numeral recess/cutwalls, no label overlay on geometry. Record output SHA-256 and inspect every panel manually.

- [ ] **Step 6: Run the full assets gate and independent review**

```bash
python3 scripts/validate_dice_tray_presets.py
python3 scripts/test_validate_dice_tray_presets.py
python3 scripts/test_build_dice_runtime_manifest.py
python3 scripts/test_dice_semantic_contract.py
python3 scripts/build_dice_runtime_manifest.py --check
python3 -m py_compile \
  scripts/dice_semantic_contract.py \
  scripts/export_carved_d20_semantics.py \
  scripts/render_carved_d20_face_audit.py
git diff --check
git diff --name-only -- '*.glb' '*.blend' | test ! -s /dev/stdin
git status --short
```

Independent reviewer must recompute all 20 upward results and 2,684/7,798 roles from bytes, inspect the contact sheet, and report zero blocker/high/medium findings.

- [ ] **Step 7: Commit, push, and open the ready assets PR**

```bash
git add scripts library/custom-dice harness/models/custom-dice/dice-tray-presets.json \
  evidence/57-original-d20-semantic-recovery
git commit -m "fix: color every carved d20 numeral triangle (#57)"
git push -u origin asset/57-original-d20-semantic-recovery
```

Open one ready PR to `main` with `Closes #57`, exact old/new manifest hashes, unchanged GLB hash, semantic command output, contact-sheet hash/path, and signed gate. Move #57 to In Review. **Stop for Kirk's contact-sheet approval and merge.**

---

### Task 3: Consume and validate v2 face witnesses (`rpg-dnd5e-web#751`)

**Prerequisite:** Kirk has merged the #57 assets PR and Project 19 marks #57 Done.

**Files:**
- Modify: `src/components/ui/dice/diceRuntimeManifest.ts`
- Modify: `src/components/ui/dice/diceRuntimeManifest.test.ts`
- Modify: `src/components/ui/dice/diceRuntimeProvider.ts`
- Modify: `src/components/ui/dice/diceRuntimeProvider.test.ts`
- Modify: `src/components/ui/dice/diceRuntimeTestFixtures.ts`
- Ignored sync: `public/models/custom-dice/**`

**Interfaces:**
- Consumes: merged `dice-runtime-presets@2` and exact corrected provider bytes.
- Produces: immutable `RuntimeFaceWitnessV2`; provider-validated witness normals/ordinals bound to parsed geometry.

- [ ] **Step 1: Refresh only the private provider boundary**

```bash
cd /home/kirk/game-dev/rpg-dnd5e-web/.worktrees/751-original-d20-runtime
git fetch origin
test "$(git rev-parse HEAD)" = b5045811e769bcd686186c930252a4c6f95d44b1
npm run assets:sync
sha256sum public/models/custom-dice/dice-tray-presets.json \
  public/models/custom-dice/original-set/Original_D20_Source.glb
git status --short --ignored | grep '!! public/models/custom-dice/'
```

Expected: corrected manifest hash differs; GLB remains `87bf2d...a77e`; provider stays ignored.

- [ ] **Step 2: Write strict v2 parser RED tests**

Require exact v2 schema/generator and recursively frozen witness values. Reject v1, unknown keys, empty/duplicate/out-of-range witness indices, non-unit/non-finite normals, wrong digest grammar, witness overlap, witness indices outside body, incomplete result set, and any face index reused by two results.

```bash
npm run test:run -- src/components/ui/dice/diceRuntimeManifest.test.ts
```

Expected: corrected real provider and v2 fixture fail against the v1 parser.

- [ ] **Step 3: Implement v2 parsing**

Add `RuntimeFaceWitnessV2` and `DiceSettlementFaceV2` exactly as Task 1 defines. Reconstruct/freeze every accepted array/object and keep hostile getter/proxy handling fail-closed. Do not accept optional witnesses or infer normals from quaternions.

- [ ] **Step 4: Write provider geometry RED tests**

Provider preparation must recompute each witness digest from parsed GLB index/position bytes and verify every witness triangle's outward normal agrees with `faceNormal` within dot `>= 0.999999`. Mutations of one index, one vertex coordinate, one digest, body membership, or face normal fail before renderer readiness.

```bash
npm run test:run -- src/components/ui/dice/diceRuntimeProvider.test.ts
```

Expected: fails because provider does not inspect witness semantics.

- [ ] **Step 5: Implement hash-before-parse witness validation**

Preserve load order:

```text
manifest strict v2 parse → allowlisted preset → GLB size/SHA
→ GLTF parse and node→mesh binding → witness geometry validation → ready
```

Expose only the immutable validated preset/scene/binding. A witness failure is terminal provider failure and follows existing armed concealment/release-to-SVG semantics.

- [ ] **Step 6: Verify and commit Task 3**

```bash
npm run test:run -- \
  src/components/ui/dice/diceRuntimeManifest.test.ts \
  src/components/ui/dice/diceRuntimeProvider.test.ts \
  src/components/ui/dice/materialFreeCarvedMesh.test.ts
npm run typecheck
npx eslint src/components/ui/dice/diceRuntimeManifest.ts \
  src/components/ui/dice/diceRuntimeProvider.ts
npx prettier --check src/components/ui/dice/diceRuntimeManifest* \
  src/components/ui/dice/diceRuntimeProvider* \
  src/components/ui/dice/diceRuntimeTestFixtures.ts
git diff --check
git add src/components/ui/dice
git commit -m "fix: validate carved d20 face witnesses (#751)"
```

Commission independent task review.

---

### Task 4: Observe the actual upward result in the renderer

**Files:**
- Create: `src/components/ui/dice/diceSettlementObservation.ts`
- Create: `src/components/ui/dice/diceSettlementObservation.test.ts`
- Modify: `src/components/ui/dice/AttackDie3D.tsx`
- Modify: `src/components/ui/dice/AttackDie3D.test.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.test.tsx`
- Modify: `src/components/ui/dice/DiceTrayPresentation.test.tsx`

**Interfaces:**

```ts
interface UpwardResultObservation {
  readonly result: number;
  readonly upDot: number;
  readonly runnerUpDot: number;
  readonly margin: number;
}

function observeUpwardResult(
  entries: Readonly<Record<string, DiceSettlementFaceV2>>,
  worldQuaternion: readonly [number, number, number, number]
): UpwardResultObservation;
```

`AttackDieTelemetry` adds `observedUpwardResult`, `observedUpDot`, and `observedUpMargin` on observed frames.

- [ ] **Step 1: Write pure observation RED tests**

Test all 20 corrected real entries. For each requested result, apply its quaternion and require:

```ts
expect(observation.result).toBe(requestedResult);
expect(observation.upDot).toBeGreaterThan(0.999999);
expect(observation.margin).toBeGreaterThan(0.2);
```

Add the exact historical permutation: pair result 3's direct witness with the old result-3 quaternion and require observation result 5, proving the test fails the previously accepted provider. Reject ties, invalid quaternions, nonfinite values, and an incomplete entry set.

- [ ] **Step 2: Run RED and implement the pure resolver**

```bash
npm run test:run -- src/components/ui/dice/diceSettlementObservation.test.ts
```

Expected: FAIL because the independent resolver does not exist. Implement quaternion-vector rotation, sort by transformed Y, and return target/runner-up facts without consulting `mappedTarget` or requested result.

- [ ] **Step 3: Write renderer RED tests**

Require exact-target hold **and** independently observed face equality before `state: 'observed'`. A synthetically permuted target must emit controlled failure/fallback rather than semantic success. Roller and Spectator must independently report the same upward result from separate frames/clones.

- [ ] **Step 4: Integrate observation into actual world-pose telemetry**

Use the final rendered group quaternion after motion convergence. Do not derive observation from the requested result or copy the target tuple into the observation. Keep existing angular error as a separate diagnostic.

Observed success requires:

```ts
angularErrorDegrees <= 0.25 &&
observation.result === requestedResult &&
observation.upDot > 0.999999 &&
observation.margin > 0.2
```

Failure preserves explicit release and truthful SVG convergence; it never substitutes another physical result.

- [ ] **Step 5: Verify and commit Task 4**

```bash
npm run test:run -- \
  src/components/ui/dice/diceSettlementObservation.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx
npm run typecheck
npx eslint src/components/ui/dice
npx prettier --check src/components/ui/dice/diceSettlementObservation* \
  src/components/ui/dice/AttackDie3D*
git diff --check
git add src/components/ui/dice
git commit -m "fix: observe the upward carved result (#751)"
```

Commission independent task review.

---

### Task 5: Replace the circular Stone 0 evidence oracle

**Files:**
- Modify: `scripts/attack-die/stone0TrayEvidenceProtocol.ts`
- Modify: `scripts/attack-die/stone0TrayEvidenceProtocol.test.ts`
- Modify: `scripts/attack-die/capture-stone0-tray-evidence.mjs`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx`
- Modify: `docs/how-to/attack-die-3d-concept.md`
- Private archive/output: `/home/kirk/game-dev/.verification/interactive-dice-tray/stone-0/`

**Interfaces:**
- Consumes: corrected provider, independent renderer telemetry, real Tray route.
- Produces: v2 result facts, readable close-ups, exact-SHA package that cannot pass the historic permutation.

Each result fact requires both witnesses:

```ts
interface Stone0ResultWitnessV2 {
  readonly requestedResult: number;
  readonly mappedTarget: readonly [number, number, number, number];
  readonly observedUpwardResult: number;
  readonly observedUpDot: number;
  readonly observedUpMargin: number;
  readonly canvasVisible: boolean;
  readonly exactTargetHeld: boolean;
  readonly numeralTriangleCount: 7798;
}
```

- [ ] **Step 1: Invalidate the old private package explicitly**

Rename the old exact-SHA directory to a rejection archive suffixed `-REJECTED-semantic-oracle`, rename its `PASS` marker to `INVALIDATED-PASS.txt`, and add `FAILED.txt` explaining that 15/20 asset labels and 879 triangle roles were wrong. Preserve every old artifact for diagnosis; never cite its package hash as acceptance.

- [ ] **Step 2: Write protocol RED tests**

Require schema v2, exact corrected provider manifest hash, results 1–20, both witnesses' independent upward facts, 2,684/7,798 roles, and 40 close-up screenshots. Reject:

- requested result differing from observed result;
- target hold passing while upward result fails;
- literal/hard-coded observed values;
- missing/duplicate close-ups;
- `carvedVisible` or `carvedResult` fields;
- close-up dimensions below 220×220 physical pixels;
- old 3,563/6,919 counts; and
- any package carrying both PASS and FAILED markers.

```bash
npm run test:run -- scripts/attack-die/stone0TrayEvidenceProtocol.test.ts
```

Expected: current protocol fails because it equates canvas/target visibility with carved correctness.

- [ ] **Step 3: Implement truthful evidence vocabulary**

Rename canvas-only facts to `canvasVisible`. Read `observedUpwardResult`, dot, and margin from matching renderer telemetry. No evidence-driver constant may claim a carved result.

Capture per result:

- existing full-page 1440 layout screenshot;
- Roller well close-up at browser device scale factor 3; and
- Spectator well close-up at device scale factor 3.

Each close-up must be at least 220×220 physical pixels, include the entire die/well, and remain bound in `package-manifest.json`. Retain 1241/1240/760 responsive, reduced-motion, provider failure, WebGL/context-loss, and shader scenarios.

- [ ] **Step 4: Add the historical permutation regression**

Feed the old provider map into the production protocol/renderer fixture and require failure containing:

```text
requested result 3 observed upward result 5
```

This test must fail if observation is again derived from `mappedTarget` equality.

- [ ] **Step 5: Run focused and full gates**

```bash
npm run test:run -- \
  scripts/attack-die/stone0TrayEvidenceProtocol.test.ts \
  src/components/ui/dice/diceRuntimeManifest.test.ts \
  src/components/ui/dice/diceRuntimeProvider.test.ts \
  src/components/ui/dice/diceSettlementObservation.test.ts \
  src/components/ui/dice/materialFreeCarvedMesh.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx
npm run test:run
npm run typecheck
npm run lint
npm run build
git diff --check
git status --short
```

Expected: all pass; private provider/evidence remains ignored; no GLB or private manifest is staged.

- [ ] **Step 6: Commit exact web recovery candidate**

```bash
git add src scripts/attack-die docs/how-to/attack-die-3d-concept.md
git commit -m "test: prove carved d20 face identity in browser (#751)"
SHA=$(git rev-parse HEAD)
test "$SHA" != b5045811e769bcd686186c930252a4c6f95d44b1
```

- [ ] **Step 7: Capture a fresh exact-SHA package**

```bash
OUT=/home/kirk/game-dev/.verification/interactive-dice-tray/stone-0/$SHA
BUILD_MANIFEST="$OUT/build-manifest.json"
test ! -e "$OUT"
mkdir -p "$OUT"
VITE_ATTACK_DIE_WEB_COMMIT="$SHA" \
  npm run attack-die:freeze-build -- --out "$BUILD_MANIFEST"
node scripts/attack-die/serve-frozen.mjs \
  --dist dist \
  --build-manifest "$BUILD_MANIFEST" \
  --synty-root public/models/synty \
  --custom-dice-root public/models/custom-dice \
  --host 127.0.0.1 \
  --port 3003 >"$OUT/preview.log" 2>&1 &
PREVIEW_PID=$!
trap 'kill "$PREVIEW_PID" 2>/dev/null || true' EXIT
for attempt in $(seq 1 80); do
  curl -fsS 'http://127.0.0.1:3003/?concept=attack-die-3d' >/dev/null && break
  sleep 0.25
done
PLAYWRIGHT_CHROMIUM_EXECUTABLE=/usr/bin/google-chrome \
node scripts/attack-die/capture-stone0-tray-evidence.mjs \
  --url 'http://127.0.0.1:3003/?concept=attack-die-3d' \
  --out "$OUT" \
  --build-manifest "$BUILD_MANIFEST" \
  --source-sha "$SHA"
kill "$PREVIEW_PID"
trap - EXIT
test -f "$OUT/PASS"
test ! -f "$OUT/FAILED.txt"
```

- [ ] **Step 8: Human and independent evidence gate**

Kirk reviews all 20 Roller close-ups; independent reviewer checks all 40 close-ups, recomputes package hashes/protocol, and confirms:

- visible numeral = requested result for 1–20;
- numeral treatment is complete and readable;
- Roller/Spectator agree while owning independent renderers;
- geometry-backed observations match requested results;
- no blocker/high/medium finding remains.

Any failure gets a discriminating test, new commit, and entirely new exact-SHA directory.

---

### Task 6: Restore PR #752 only after all recovery gates pass

**Files:**
- No new product files unless review finds a defect.
- Update external SDD ledger and GitHub checkpoints only.

- [ ] **Step 1: Run exact-head normal pre-push hook and push**

Temporarily move harness-only `.pi/` and `.superpowers/` outside the web worktree so formatting checks do not scan them; restore both afterward. Push normally with hooks enabled.

```bash
git push origin asset/751-original-d20-runtime
test "$(git rev-parse HEAD)" = \
  "$(git rev-parse origin/asset/751-original-d20-runtime)"
```

Expected: full pre-push gate passes; only untracked `.pi/` remains.

- [ ] **Step 2: Reconcile remote review and CI**

Require GitHub Test, Lint and Type Check, Security Audit, and Deploy Preview green. Reconcile every Copilot thread individually. Commission a fresh whole-branch review from merged foundation `1322dc46b21ee0e7e4e87891b84f74a686f64232` through exact new head.

- [ ] **Step 3: Post the replacement signed gate**

The new GATE REVIEW must explicitly supersede the withdrawn b504 gate and cite:

- merged assets correction commit/PR;
- corrected provider manifest/source hashes;
- unchanged GLB hash;
- 20/20 independent upward observations for both witnesses;
- 2,684/7,798 exact roles;
- close-up count and package-manifest hash;
- full test totals and GitHub checks; and
- residual physical-device/performance limitations.

- [ ] **Step 4: Move Project 19 to In Review and stop**

Move assets #57 to Done only after Kirk merged it. Move web #751 to In Review only after the replacement gate is posted. Update project #219/PR #220. **Do not merge PR #752. Stop for Kirk's final live visual and merge decision.**

## Post-merge completion

After Kirk merges PR #752, verify the exact merged route against the corrected provider, record provider/web merge SHAs and evidence hashes, move #751 to Done, answer both Stone 0 retro questions, and rewrite `rpg-project/sessions/active.md`. Create a separate Stone 1 issue/plan; do not reuse rejected held-motion commits.
