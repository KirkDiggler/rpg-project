# Calibrated Floor-Prop Promotion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans` to implement this plan task-by-task. Journey #365 forbids subagent-driven development, spawned subagents, and autonomous review/fix loops.

**Goal:** Let Kirk select 10–20 cached prop GLBs, calibrate them locally beside a fighter through an in-game property sheet, export one batch recipe, and publish the batch through deterministic provider and web-consumer scripts.

**Architecture:** `rpg-game-assets` remains the private authority for source resolution, calibration recipes, Blender baking, promoted GLBs, and generated portable metadata. `rpg-dnd5e-web` provides a loopback-only development lab and consumes a generated license-safe catalog; Builder and gameplay share its resolver. Promotion is stage-first and batch-atomic, while unsupported exact refs remain unresolved rather than receiving a substitute.

**Tech Stack:** Python 3.14, Blender 5.x Python, JSON, React 19, TypeScript 5.8, React Three Fiber, Three.js, Vite, Vitest, Testing Library, existing static release transaction tooling.

**Spec:** `ideas/assets/asset-library-ingestion/design.md`

## Global Constraints

- Work inline. Do not spawn subagents or autonomous review/fix loops.
- Integration is PR-only; never merge locally or push directly to `main`.
- Raw licensed archives remain immutable, local, and never automatically deleted.
- `.glb-cache/`, `.preview-cache/`, calibration GLBs, and synchronized web GLBs remain ignored.
- No licensed GLB may be tracked by the public `rpg-dnd5e-web` repository.
- Version 1 accepts only static, rigid, one-GLB, floor-standing props.
- Runtime geometry uses the existing shared `SYNTY_SCALE`; per-prop scale, base yaw, centering, and grounding are baked by the provider.
- Three-part refs are family/default aliases. Four-or-more-part refs identify exactly one GLB.
- Exact refs never randomize, substitute, or fall back.
- Movement and line-of-sight behavior are explicit booleans, never inferred from ref spelling.
- No daemon, database, generalized transaction framework, lock manager, or production asset service.
- Preserve unrelated `rpg-game-assets/placed/` and `rpg-project/active.md`.

## Delivered prerequisites

Archive indexing, pack conversion, ignored GLB caches, cached opposing renders, and tracked sheets are already delivered through the Fantasy Kingdom and Dark Fortress ingestion PRs. `rpg-game-assets#139` is the paired-sheet gate: merge it before implementation so the contact sheets and `.preview-cache/` policy match the spec. This plan does not rebuild those delivered tools.

## Repository and delivery order

1. Merge the approved design PR `rpg-project#367` and paired-sheet PR `rpg-game-assets#139`.
2. Create one linked implementation issue in `rpg-game-assets` for Tasks 1, 2, 5, and 6.
3. Create one linked implementation issue in `rpg-dnd5e-web` for Tasks 3, 4, and 7.
4. Create isolated worktrees through the repository's normal issue/worktree process.
5. Land the local lab before Kirk's calibration checkpoint.
6. Land the private provider batch before final web consumption.
7. Repeat exact-ref Save & Play verification against the merged provider revision.

## File structure

### `rpg-game-assets`

- `scripts/prop_promotion_recipe.py` — strict portable recipe types, parsing, validation, ref/path derivation.
- `scripts/test_prop_promotion_recipe.py` — pure recipe and default-ref contract tests.
- `scripts/configs/prop-promotion/template.json` — one intentionally incomplete starter row.
- `scripts/prepare_prop_calibration.py` — resolve a selected source through a cache manifest and write the ignored web calibration catalog.
- `scripts/test_prepare_prop_calibration.py` — source/hash/path/ignore-boundary tests.
- `scripts/normalize_prop_for_runtime.py` — Blender entry point that bakes static prop geometry into runtime coordinates.
- `scripts/test_normalize_prop_for_runtime.py` — synthetic-GLB Blender acceptance test.
- `scripts/promote_props.py` — stage, validate, apply, check, and single-ref validation orchestration.
- `scripts/test_promote_props.py` — stage purity, atomic apply, drift, and failure tests.
- `scripts/build_prop_web_catalog.py` — deterministic compact provider-to-web projection.
- `scripts/test_build_prop_web_catalog.py` — exact/default alias and allowlist tests.
- `scripts/build_prop_manifest.py` — merge legacy role-map entries with tracked promotion recipes.
- `harness/catalogs/synty-props-web.json` — generated license-safe catalog.

### `rpg-dnd5e-web`

- `src/dev/prop-calibration/route.ts` — development + loopback + query gate.
- `src/dev/prop-calibration/route.test.ts` — complete route-gate matrix.
- `src/dev/prop-calibration/model.ts` — catalog/recipe types, strict parsing, validation, reducer, import/export.
- `src/dev/prop-calibration/model.test.ts` — pure state and JSON round-trip tests.
- `src/dev/prop-calibration/PropCalibrationScene.tsx` — fighter/hex/raw/calibrated Three.js scene.
- `src/dev/prop-calibration/PropCalibrationLab.tsx` — candidate navigation, property sheet, local drafts, import/export.
- `src/dev/prop-calibration/PropCalibrationLab.test.tsx` — form/control/export behavior.
- `scripts/generate-prop-catalog.mjs` — provider JSON to committed TypeScript projection.
- `scripts/generatePropCatalog.test.ts` — deterministic generation and invalid-input tests.
- `src/components/hex-grid/generatedPropCatalog.ts` — generated license-safe data.
- `src/components/hex-grid/propManifest.ts` — typed resolver facade over generated data.
- `src/author/paletteData.ts` — exact selectable refs and explicit behavior without family-alias duplicates.
- `src/author/DungeonBuilder.tsx` and focused tests — placement defaults copied from generated explicit behavior.
- `src/App.tsx` and `src/App.test.tsx` — lazy local-only full-window route.
- `package.json` — `props:sync` command.

---

### Task 1: Define the portable prop-promotion recipe

**Repository:** `rpg-game-assets`

**Files:**
- Create: `scripts/prop_promotion_recipe.py`
- Create: `scripts/test_prop_promotion_recipe.py`
- Create: `scripts/configs/prop-promotion/template.json`

**Interfaces:**
- Consumes: exported calibration JSON matching schema version `1`.
- Produces: `load_prop_batch(path: Path) -> PropBatch`, `validate_prop_batch(batch: PropBatch, existing_family_defaults: Mapping[str, str]) -> None`, and `runtime_path_for_ref(ref: str) -> PurePosixPath`.

- [ ] **Step 1: Write strict failing recipe tests**

Create table-driven `unittest` coverage for:

```python
VALID_ENTRY = {
    "source": {
        "packSlug": "polygon-dark-fortress",
        "packVersion": "v3",
        "sourcePath": "SourceFiles/DarkFortress/FBX/SM_Prop_Alchemy_Tool_04.fbx",
        "glbSha256": "a5e6e9fe78f4a42226362a434e62b53625d5d779b5309d95de441f70a91054ba",
    },
    "displayName": "Alchemy Tool",
    "familyRef": "dnd5e:props:alchemy-tool",
    "ref": "dnd5e:props:alchemy-tool:04",
    "defaultForFamily": True,
    "calibration": {
        "scale": 1.0,
        "yawDegrees": 0.0,
        "fineOffsetMeters": [0.0, 0.0, 0.0],
    },
    "placement": "floor",
    "role": "decor",
    "themes": ["crypt"],
    "blocksMovement": False,
    "blocksLoS": False,
    "notes": "",
}
```

Assert:

```python
self.assertEqual(
    PurePosixPath("harness/models/synty/props/alchemy-tool--04.glb"),
    runtime_path_for_ref("dnd5e:props:alchemy-tool:04"),
)
```

Also assert rejection of unknown keys, empty strings, absolute/traversing source paths, malformed hashes, duplicate exact refs, non-three-part family refs, exact refs with fewer than four parts, exact refs outside their family, multiple/missing defaults for a newly declared family, non-boolean behavior fields, roles outside `obstacle|cover|decor`, placement other than `floor`, scale outside `(0, 100]`, non-finite calibration, yaw normalization failure, horizontal offsets outside `[-0.5, 0.5]`, vertical offsets outside `[-0.1, 0.1]`, and runtime-path collisions.

- [ ] **Step 2: Run the tests and verify the expected import failure**

Run:

```bash
python3 scripts/test_prop_promotion_recipe.py
```

Expected: FAIL because `prop_promotion_recipe` does not exist.

- [ ] **Step 3: Implement immutable recipe types and validation**

Define:

```python
@dataclass(frozen=True)
class PropSource:
    pack_slug: str
    pack_version: str
    source_path: PurePosixPath
    glb_sha256: str

@dataclass(frozen=True)
class PropCalibration:
    scale: float
    yaw_degrees: float
    fine_offset_meters: tuple[float, float, float]

@dataclass(frozen=True)
class PropPromotionEntry:
    source: PropSource
    display_name: str
    family_ref: str
    ref: str
    default_for_family: bool
    calibration: PropCalibration
    placement: Literal["floor"]
    role: Literal["obstacle", "cover", "decor"]
    themes: tuple[str, ...]
    blocks_movement: bool
    blocks_los: bool
    notes: str

@dataclass(frozen=True)
class PropBatch:
    schema_version: int
    batch_id: str
    entries: tuple[PropPromotionEntry, ...]
```

Use exact-key validation before construction. Normalize yaw with:

```python
def normalize_yaw(value: float) -> float:
    normalized = ((value + 180.0) % 360.0) - 180.0
    return 0.0 if normalized == -0.0 else normalized
```

Derive output paths solely from exact-ref tail segments joined by `--`.

- [ ] **Step 4: Add the starter template**

The tracked template contains one row with all property names, empty identity strings, `scale: 1.0`, zero calibration offsets, `placement: "floor"`, `role: "decor"`, empty themes, and explicit false behavior booleans. Its header comment field states that it is intentionally incomplete and must not pass provider validation.

- [ ] **Step 5: Run focused tests**

Run:

```bash
python3 scripts/test_prop_promotion_recipe.py
python3 -m py_compile scripts/prop_promotion_recipe.py
```

Expected: PASS.

- [ ] **Step 6: Commit**

```bash
git add scripts/prop_promotion_recipe.py scripts/test_prop_promotion_recipe.py scripts/configs/prop-promotion/template.json
git commit -m "asset: define prop promotion batch recipes"
```

---

### Task 2: Prepare selected cached GLBs for local calibration

**Repository:** `rpg-game-assets`

**Files:**
- Create: `scripts/prepare_prop_calibration.py`
- Create: `scripts/test_prepare_prop_calibration.py`
- Modify: `.gitignore`
- Modify: `library/source-catalog/README.md`

**Interfaces:**
- Consumes: `--pack-root`, `--source-path`, `--web-root`, and optional `--port`.
- Produces: ignored `public/models/synty/prop-calibration/catalog.json`, one content-addressed GLB copy, and a loopback URL.

- [ ] **Step 1: Write failing preparation tests**

Build temporary asset/web repositories. The fixture manifest contains one model row matching `VALID_ENTRY.source`. Assert that:

```python
result = prepare_candidate(
    pack_root=pack_root,
    source_path=PurePosixPath("SourceFiles/DarkFortress/FBX/SM_Prop_Alchemy_Tool_04.fbx"),
    web_root=web_root,
    port=5173,
)
self.assertEqual(
    "http://127.0.0.1:5173/?propCalibration=1",
    result.url,
)
self.assertEqual(1, len(json.loads(result.catalog_path.read_text())["candidates"]))
```

Verify exact copied bytes and content-addressed filename. Assert refusal for unknown/duplicate conflicting source paths, stale output hashes, missing cache files, symlinked sources, `..`, absolute paths, wrong web package name, tracked calibration destination, and destinations outside the web root. Re-adding the identical candidate is idempotent.

- [ ] **Step 2: Run the test and verify failure**

```bash
python3 scripts/test_prepare_prop_calibration.py
```

Expected: FAIL because `prepare_prop_calibration` does not exist.

- [ ] **Step 3: Implement `add` and `reset` commands**

Use this catalog shape:

```json
{
  "schemaVersion": 1,
  "candidates": [
    {
      "source": {
        "packSlug": "polygon-dark-fortress",
        "packVersion": "v3",
        "sourcePath": "SourceFiles/DarkFortress/FBX/SM_Prop_Alchemy_Tool_04.fbx",
        "glbSha256": "a5e6e9fe78f4a42226362a434e62b53625d5d779b5309d95de441f70a91054ba"
      },
      "url": "/models/synty/prop-calibration/a5e6e9fe78f4-SM_Prop_Alchemy_Tool_04.glb"
    }
  ]
}
```

Resolve source FBX names through `.glb-cache/manifest-props.json`, then resolve its `outputPath` under the same `.glb-cache`. Derive `packSlug` from the pack directory's parent name and `packVersion` from the pack directory name; load `scripts/configs/synty-packs/{pack_slug}.json` and require its declared `packSlug` to match before accepting either value. Hash the actual GLB before copying. Write catalog bytes atomically through a temporary sibling file and `os.replace`.

`reset` removes only `public/models/synty/prop-calibration/` after proving the resolved target remains under the supplied web root.

- [ ] **Step 4: Enforce the ignore boundary**

Add the explicit defensive pattern:

```gitignore
public/models/synty/prop-calibration/
```

The command runs `git check-ignore --quiet` against the destination before copying and refuses if Git would track it.

- [ ] **Step 5: Document the exact beginner commands**

Document one concrete Dark Fortress command and explain that `.glb-cache` is a script-managed local warehouse; the collaborator supplies the contact-sheet source path, not a cache path.

- [ ] **Step 6: Verify and commit**

```bash
python3 scripts/test_prepare_prop_calibration.py
python3 -m py_compile scripts/prepare_prop_calibration.py
git diff --check
git add .gitignore library/source-catalog/README.md scripts/prepare_prop_calibration.py scripts/test_prepare_prop_calibration.py
git commit -m "asset: prepare cached props for local calibration"
```

---

### Task 3: Add a loopback-only development route

**Repository:** `rpg-dnd5e-web`

**Files:**
- Create: `src/dev/prop-calibration/route.ts`
- Create: `src/dev/prop-calibration/route.test.ts`
- Create: `src/dev/prop-calibration/PropCalibrationLab.tsx` with a temporary route-visible shell
- Modify: `src/App.tsx`
- Modify: `src/App.test.tsx`

**Interfaces:**
- Produces: `isPropCalibrationRoute(mode: string, hostname: string, search: string): boolean` and a lazy full-window route.

- [ ] **Step 1: Write the failing route matrix**

```ts
it.each([
  ['development', '127.0.0.1', '?propCalibration=1', true],
  ['development', 'localhost', '?propCalibration=1', true],
  ['development', '::1', '?propCalibration=1', true],
  ['development', '[::1]', '?propCalibration=1', true],
  ['production', '127.0.0.1', '?propCalibration=1', false],
  ['development', 'dev.example.test', '?propCalibration=1', false],
  ['development', '127.0.0.1', '', false],
  ['development', '127.0.0.1', '?propCalibration=0', false],
])('%s %s %s', (mode, hostname, search, expected) => {
  expect(isPropCalibrationRoute(mode, hostname, search)).toBe(expected);
});
```

Add an App test proving production with the query does not mount the lab and development loopback does.

- [ ] **Step 2: Run focused tests and verify failure**

```bash
npm test -- --run src/dev/prop-calibration/route.test.ts src/App.test.tsx
```

Expected: FAIL on the missing route module/surface.

- [ ] **Step 3: Implement the pure gate**

```ts
const LOOPBACK_HOSTS = new Set(['127.0.0.1', 'localhost', '::1', '[::1]']);

export function isPropCalibrationRoute(
  mode: string,
  hostname: string,
  search: string
): boolean {
  if (mode !== 'development' || !LOOPBACK_HOSTS.has(hostname)) return false;
  return new URLSearchParams(search).get('propCalibration') === '1';
}
```

- [ ] **Step 4: Mount through a development-conditional lazy import**

In `App.tsx`, follow the existing toolkit-sandbox pattern:

```ts
const LazyPropCalibrationLab =
  import.meta.env.MODE === 'development'
    ? lazy(() =>
        import('./dev/prop-calibration/PropCalibrationLab').then(
          ({ PropCalibrationLab }) => ({ default: PropCalibrationLab })
        )
      )
    : null;
```

Compute the route once. Before ordinary application routing, return a full-window `Suspense` surface only when the gate is true and the lazy component exists. Do not add an `AppView`, Concepts entry, Home link, or author navigation entry.

- [ ] **Step 5: Verify production refusal and commit**

```bash
npm test -- --run src/dev/prop-calibration/route.test.ts src/App.test.tsx
npm run typecheck
npm run build
git add src/App.tsx src/App.test.tsx src/dev/prop-calibration
git commit -m "dev: add loopback-only prop calibration route"
```

Expected: all pass; the production test refuses the route.

---

### Task 4: Build the property-sheet state and local calibration UI

**Repository:** `rpg-dnd5e-web`

**Files:**
- Create: `src/dev/prop-calibration/model.ts`
- Create: `src/dev/prop-calibration/model.test.ts`
- Create: `src/dev/prop-calibration/PropCalibrationScene.tsx`
- Modify: `src/dev/prop-calibration/PropCalibrationLab.tsx`
- Create: `src/dev/prop-calibration/PropCalibrationLab.test.tsx`

**Interfaces:**
- Consumes: `/models/synty/prop-calibration/catalog.json` from Task 2.
- Produces: schema-v1 batch JSON accepted by Task 1 and stored under local key `rpg.prop-calibration.batch.v1`.

- [ ] **Step 1: Write failing pure-model tests**

Define expected types in the tests:

```ts
export interface CalibrationEntry {
  source: {
    packSlug: string;
    packVersion: string;
    sourcePath: string;
    glbSha256: string;
  };
  url: string;
  displayName: string;
  familyRef: string;
  ref: string;
  defaultForFamily: boolean;
  calibration: {
    scale: number;
    yawDegrees: number;
    fineOffsetMeters: [number, number, number];
  };
  placement: 'floor';
  role: 'obstacle' | 'cover' | 'decor';
  themes: string[];
  blocksMovement: boolean;
  blocksLoS: boolean;
  notes: string;
}
```

Assert strict catalog parsing, candidate-to-incomplete-row construction, merging a newly prepared candidate into an existing local draft without resetting completed rows, reducer field updates, yaw normalization, scale/offset bounds, exact/family ref relation, provider-readiness errors by field, local-storage round trip, draft export, provider-ready export refusal while incomplete, and exact JSON round trip when complete.

- [ ] **Step 2: Run pure tests and verify failure**

```bash
npm test -- --run src/dev/prop-calibration/model.test.ts
```

Expected: FAIL because `model.ts` does not exist.

- [ ] **Step 3: Implement the pure state boundary**

Export:

```ts
export const PROP_CALIBRATION_STORAGE_KEY = 'rpg.prop-calibration.batch.v1';
export function parseCalibrationCatalog(value: unknown): CalibrationCatalog;
export function entryFromCandidate(candidate: CalibrationCandidate): CalibrationEntry;
export function validateCalibrationEntry(entry: CalibrationEntry): FieldErrors;
export function isProviderReady(batch: CalibrationBatch): boolean;
export function serializeCalibrationBatch(batch: CalibrationBatch): string;
export function parseCalibrationBatch(text: string): CalibrationBatch;
export function propCalibrationReducer(
  state: PropCalibrationState,
  action: PropCalibrationAction
): PropCalibrationState;
```

Keep parsing strict and dependency-free. `url` is local calibration input and must be omitted from exported provider entries.

- [ ] **Step 4: Write failing component tests**

Mock `PropCalibrationScene` and `fetch`. Verify:

- catalog candidates appear in source order;
- source/hash/bounds fields are read-only;
- changing scale, yaw, role, themes, behavior flags, names, and refs updates the scene props and property sheet;
- previous/next preserves each row;
- local storage receives drafts;
- Import replaces state only after strict parsing;
- Export Draft downloads incomplete JSON;
- Export Provider Batch remains disabled until every row is complete;
- exported provider JSON contains no URL, absolute path, Blob URL, or localhost string.

- [ ] **Step 5: Implement the Three.js scene**

Reuse existing hex/floor/camera primitives and the canonical fighter resolver. Load the selected candidate with `useGLTF(entry.url)`. Render:

```tsx
<group rotation={[0, THREE.MathUtils.degToRad(entry.calibration.yawDegrees), 0]}>
  <group
    position={resolvedPreviewOffset}
    scale={SYNTY_SCALE * entry.calibration.scale}
  >
    <primitive object={candidateClone} />
  </group>
</group>
```

Measure candidate bounds from the loaded object. Derive center/floor alignment, then apply the fine offset. Show raw/calibrated overlay, fighter, owning hex, orbit/play camera toggle, and compass-facing controls. Never load from a filesystem URL.

- [ ] **Step 6: Implement the property sheet and export/import**

Use accessible labels and numeric inputs alongside sliders. Download with a Blob and temporary object URL, revoking it immediately after click. Use the batch ID directly as the filename, for example `first-floor-props-v1.json`. Autosave only validated draft structure; malformed stored JSON is ignored with a visible diagnostic.

- [ ] **Step 7: Verify and commit**

```bash
npm test -- --run src/dev/prop-calibration/model.test.ts src/dev/prop-calibration/PropCalibrationLab.test.tsx src/dev/prop-calibration/route.test.ts src/App.test.tsx
npm run typecheck
npm run build
git add src/dev/prop-calibration src/App.tsx src/App.test.tsx
git commit -m "dev: calibrate prop batches beside a player model"
```

---

### Human checkpoint: Calibrate the first batch

This checkpoint is not delegated.

1. Kirk chooses 10–20 static floor props from the paired sheets.
2. Run Task 2's `add` command once per source path.
3. Start the normal Vite development server.
4. Open the printed loopback URL.
5. Complete every property sheet and inspect every compass facing plus play camera.
6. Export `first-floor-props-v1.json` as provider-ready.
7. Re-import the exported file and confirm all values and candidates are unchanged.
8. Preserve the export until Task 6 records its canonicalized copy.

Do not continue to provider `apply` without Kirk's visual acceptance of the calibrated batch.

---

### Task 5: Bake one calibrated static GLB through Blender

**Repository:** `rpg-game-assets`

**Files:**
- Create: `scripts/normalize_prop_for_runtime.py`
- Create: `scripts/test_normalize_prop_for_runtime.py`

**Interfaces:**
- Consumes: input GLB, output GLB, scale, yaw degrees, and fine offset metres.
- Produces: one static GLB with embedded textures, identity mesh-node transforms, centered horizontal bounds, and grounded vertical bounds.

- [ ] **Step 1: Write the failing synthetic Blender acceptance**

The test creates an offset rectangular mesh and packed 4×4 texture in a temporary source GLB, invokes Blender in factory mode with the normalizer, then parses the result. Assert:

- no skins, animations, cameras, or punctual lights;
- exactly the source mesh/material/image content expected;
- image remains embedded;
- node transforms are absent or identity;
- source dimensions multiplied by requested scale and rotated axes match expected dimensions within `1e-5`;
- X/Z bounds center equals requested fine X/Z within `0.01m` after shared-scale conversion;
- minimum Y is within `0.01m` of requested fine Y;
- output hash is stable across two identical runs.

- [ ] **Step 2: Run the Blender test and verify failure**

```bash
python3 scripts/test_normalize_prop_for_runtime.py
```

Expected: FAIL because the normalizer does not exist.

- [ ] **Step 3: Implement static mesh baking**

After importing the source GLB, reject every non-mesh runtime payload except empty transform nodes. For each mesh object, preserve its material assignments and evaluate vertices through its original world matrix.

Build a base matrix:

```python
base = Matrix.Rotation(math.radians(yaw_degrees), 4, "Y") @ Matrix.Scale(scale, 4)
```

Measure every `base @ object.matrix_world @ vertex.co`, derive aggregate center X/Z and minimum Y, then build:

```python
translation = Matrix.Translation((
    -center_x + fine_x,
    -minimum_y + fine_y,
    -center_z + fine_z,
))
final = translation @ base @ object.matrix_world
```

Copy each mesh datablock, apply `final` directly to its vertices through `mesh.transform(final)`, clear parentage, and set object matrices to identity. Export selected mesh objects as GLB with images embedded and animations disabled.

- [ ] **Step 4: Add strict post-export inspection**

Parse the output JSON chunk and reject absent POSITION bounds, non-finite accessors/transforms, skins, animations, cameras, lights, external URIs, final axes over `20m` after shared runtime scale, and floor error over `0.01m`.

- [ ] **Step 5: Verify and commit**

```bash
python3 scripts/test_normalize_prop_for_runtime.py
python3 -m py_compile scripts/normalize_prop_for_runtime.py
git add scripts/normalize_prop_for_runtime.py scripts/test_normalize_prop_for_runtime.py
git commit -m "asset: bake calibrated static prop geometry"
```

---

### Task 6: Stage and atomically publish a prop batch

**Repository:** `rpg-game-assets`

**Files:**
- Create: `scripts/promote_props.py`
- Create: `scripts/test_promote_props.py`
- Create: `scripts/build_prop_web_catalog.py`
- Create: `scripts/test_build_prop_web_catalog.py`
- Modify: `scripts/build_prop_manifest.py`
- Generate: `harness/catalogs/synty-props-web.json`

**Interfaces:**
- Consumes: Task 1 recipe and Task 5 Blender normalizer.
- Produces: `stage`, `validate`, `apply`, `check`, `validate --only`, promoted GLBs, combined prop manifest, compact web catalog, mesh stats, inventory, and deterministic receipt.

- [ ] **Step 1: Write failing staged-provider tests**

Use a temporary repository with one legacy prop and two recipe entries. Inject a fake normalizer that copies deterministic fixture GLBs. Assert:

```python
stage = stage_batch(repo_root, recipe_path, stage_root, normalize_runner=fake_runner)
self.assertFalse((repo_root / "harness/models/synty/props/alchemy-tool--04.glb").exists())
self.assertTrue((stage.release_root / "harness/models/synty/props/alchemy-tool--04.glb").is_file())
```

Also assert:

- source lookup uses `sourcePath` and verifies `glbSha256`;
- `--only` validates one row but cannot apply a partial batch;
- stage contains the canonicalized recipe at `scripts/configs/prop-promotion/{batch.batch_id}.json`;
- role/behavior/theme data enters the combined generated manifest;
- generated targets include GLBs, recipe, prop manifest, web catalog, mesh stats, inventory, and receipt;
- stage writes its deterministic receipt at `evidence/prop-promotions/{batch.batch_id}/receipt.json`;
- stage failure leaves canonical bytes unchanged;
- injected `atomic_apply_targets` failure restores every pre-state byte;
- check passes after apply and fails after changing one GLB, recipe field, manifest, or catalog byte.

- [ ] **Step 2: Write failing web-catalog tests**

Define compact output:

```json
{
  "schemaVersion": 1,
  "generatedBy": "build_prop_web_catalog@1.0.0",
  "refs": {
    "dnd5e:props:alchemy-tool:04": {
      "familyRef": "dnd5e:props:alchemy-tool",
      "displayName": "Alchemy Tool",
      "file": "props/alchemy-tool--04.glb",
      "sizeBytes": 12,
      "sha256": "dbc666adb19aa053ab772b04dba60248d8a75462b2a50595e345c2d837a4aad0",
      "role": "decor",
      "footprintHexes": 1,
      "blocksMovement": false,
      "blocksLoS": false
    }
  },
  "familyDefaults": {
    "dnd5e:props:alchemy-tool": "dnd5e:props:alchemy-tool:04"
  },
  "paletteRefs": ["dnd5e:props:alchemy-tool:04"]
}
```

Assert deterministic sorted bytes, all legacy keys retained, exact refs present once, default aliases target real exact refs, aliases absent from `paletteRefs`, runtime size/hash bound to the promoted GLB, paths confined to `props/*.glb`, and output containing no notes, source paths, source hashes, absolute paths, timestamps, usernames, or vendor archive names.

- [ ] **Step 3: Run focused tests and verify failure**

```bash
python3 scripts/test_promote_props.py
python3 scripts/test_build_prop_web_catalog.py
```

Expected: FAIL on missing modules/interfaces.

- [ ] **Step 4: Make `build_prop_manifest.py` stageable**

Refactor without changing baseline bytes and add `--check` byte comparison for the canonical output. The exact callable interface is `build_prop_manifest(repo_root: Path, legacy_role_map: Path, recipe_dir: Path, props_dir: Path) -> dict[str, object]`.

Keep a CLI that writes the canonical default path. Merge legacy rows first, then tracked recipe rows. Recipe rows supply exact ref, family metadata, explicit `blocksMovement`, and explicit `blocksLoS`; legacy rows retain their current measured behavior. Reject any conflicting ref/file/default declaration.

Before adding a recipe fixture, run the refactored generator and assert byte equality with current `harness/models/synty/props/manifest.json`.

- [ ] **Step 5: Implement deterministic compact catalog generation**

Project only consumer-safe fields. Legacy three-part keys remain selectable refs. New exact refs enter `refs` and `paletteRefs`; their three-part family alias enters only `familyDefaults`. Sort maps and lists deterministically and emit no timestamp.

- [ ] **Step 6: Implement provider commands**

Follow `promote_off_hand.py`'s proven boundaries. Export these exact interfaces:

- `stage_batch(repo_root: Path, cache_root: Path, recipe_path: Path, stage_root: Path, *, normalize_runner: NormalizeRunner = run_blender_normalizer) -> StageResult`;
- `validate_stage(repo_root: Path, cache_root: Path, recipe_path: Path, stage_root: Path) -> None`;
- `apply_stage(repo_root: Path, cache_root: Path, recipe_path: Path, stage_root: Path) -> None`; and
- `check_batch(repo_root: Path, cache_root: Path, recipe_path: Path) -> None`.

Construct a merged staged `harness/models/synty` tree, run `build_mesh_stats.build(repo_root=stage_root, output=stage_root / "harness/models/synty/mesh-stats.json")`, and invoke `build_synty_complete_inventory.main` with staged source/output paths. Validate the staged release twice before calling existing `atomic_apply_targets` with an exact sorted target tuple.

Resolve ignored source caches under `cache_root`, which defaults to `repo_root` but may point at the canonical checkout when execution occurs in a managed worktree. Never record that local root in the recipe or receipt.

Write a deterministic receipt at `evidence/prop-promotions/{batch.batch_id}/receipt.json` containing batch ID, recipe hash, ordered source/output hashes, measured bounds, generated catalog hash, inventory hash, and tool versions. Do not record timestamps or machine paths.

- [ ] **Step 7: Verify baseline and provider behavior**

```bash
python3 scripts/build_prop_manifest.py --check
python3 scripts/test_build_prop_web_catalog.py
python3 scripts/test_promote_props.py
python3 scripts/build_synty_complete_inventory.py --check
python3 scripts/build_mesh_stats.py --strict-path 'props/*.glb'
git diff --check
```

Expected: all pass before applying the human batch.

- [ ] **Step 8: Stage the human-approved batch**

```bash
python3 scripts/promote_props.py stage \
  --repo-root "$PWD" \
  --cache-root /home/kirk/game-dev/rpg-game-assets \
  --config "$HOME/Downloads/first-floor-props-v1.json" \
  --stage-root /tmp/rpg-props-first-floor-v1
python3 scripts/promote_props.py validate \
  --repo-root "$PWD" \
  --cache-root /home/kirk/game-dev/rpg-game-assets \
  --config "$HOME/Downloads/first-floor-props-v1.json" \
  --stage-root /tmp/rpg-props-first-floor-v1
```

Open the staged GLBs in the local calibration/game surface and stop for Kirk's batch visual verdict.

- [ ] **Step 9: Apply, check, and commit only after approval**

```bash
python3 scripts/promote_props.py apply \
  --repo-root "$PWD" \
  --cache-root /home/kirk/game-dev/rpg-game-assets \
  --config "$HOME/Downloads/first-floor-props-v1.json" \
  --stage-root /tmp/rpg-props-first-floor-v1
python3 scripts/promote_props.py check \
  --repo-root "$PWD" \
  --cache-root /home/kirk/game-dev/rpg-game-assets \
  --config scripts/configs/prop-promotion/first-floor-props-v1.json
python3 scripts/build_synty_complete_inventory.py --check
git status --short
git add -- \
  scripts/configs/prop-promotion/first-floor-props-v1.json \
  harness/models/synty/props \
  harness/models/synty/mesh-stats.json \
  harness/catalogs/synty-props-web.json \
  harness/catalogs/synty-complete-inventory.json \
  evidence/prop-promotions/first-floor-props-v1/receipt.json
git commit -m "asset: promote first calibrated floor props"
```

Confirm no `.glb-cache/`, `.preview-cache/`, or calibration path is staged. Push and open a reviewed provider PR; do not merge it locally.

---

### Task 7: Generate exact-ref web resolver and Builder entries

**Repository:** `rpg-dnd5e-web`

**Files:**
- Create: `scripts/generate-prop-catalog.mjs`
- Create: `scripts/generatePropCatalog.test.ts`
- Generate: `src/components/hex-grid/generatedPropCatalog.ts`
- Modify: `src/components/hex-grid/propManifest.ts`
- Modify: `src/components/hex-grid/propManifest.test.ts`
- Modify: `src/author/paletteData.ts`
- Modify: `src/author/paletteData.test.ts`
- Modify: `src/author/DungeonBuilder.tsx`
- Modify: `src/author/DungeonBuilder.test.tsx`
- Modify: `src/components/session/AtlasPropModel.tsx`
- Modify: `package.json`

**Interfaces:**
- Consumes: merged provider `harness/catalogs/synty-props-web.json` and synchronized `public/models/synty/props/*.glb`.
- Produces: generated TypeScript data, exact/default resolver behavior, and non-duplicated palette entries.

- [ ] **Step 1: Write failing generator tests**

Copy a compact provider fixture into a temporary assets root, writing the fixture runtime file as the 12 bytes `promoted-glb` so its expected SHA-256 is `dbc666adb19aa053ab772b04dba60248d8a75462b2a50595e345c2d837a4aad0`. Assert that two runs produce byte-identical TypeScript and that generated data contains:

```ts
export const GENERATED_PROP_REFS = { /* exact and legacy refs */ } as const;
export const GENERATED_PROP_FAMILY_DEFAULTS = {
  'dnd5e:props:alchemy-tool': 'dnd5e:props:alchemy-tool:04',
} as const;
export const GENERATED_PROP_PALETTE_REFS = [
  'dnd5e:props:alchemy-tool:04',
] as const;
```

Reject missing GLBs, hash/path disagreement when catalog inventory binding is present, aliases absent from refs, aliases in palette refs, unknown fields, absolute paths, and output outside `src/components/hex-grid/generatedPropCatalog.ts`.

- [ ] **Step 2: Write failing resolver/palette tests**

Assert:

```ts
expect(resolvePropVariant('dnd5e:props:alchemy-tool:04')?.file)
  .toBe('props/alchemy-tool--04.glb');
expect(resolvePropVariant('dnd5e:props:alchemy-tool')?.file)
  .toBe('props/alchemy-tool--04.glb');
expect(resolvePropVariant('dnd5e:props:alchemy-tool:99')).toBeUndefined();
expect(PALETTE_PROPS.map((p) => p.ref)).toContain('dnd5e:props:alchemy-tool:04');
expect(PALETTE_PROPS.map((p) => p.ref)).not.toContain('dnd5e:props:alchemy-tool');
expect(PALETTE_PROPS.find((p) => p.ref.endsWith(':04'))).toMatchObject({
  blocksMovement: false,
  blocksLoS: false,
});
```

Add a focused `DungeonBuilder` assertion that placing the exact ref copies these two explicit values rather than re-deriving them from role.

Update `AtlasPropModel`'s unsupported exact-ref behavior test to expect no misleading primitive substitute for a syntactically exact prop ref. Preserve legacy fallback behavior only for old non-prop entity signals where the existing primitive genuinely represents that signal.

- [ ] **Step 3: Run focused tests and verify failure**

```bash
npm test -- --run scripts/generatePropCatalog.test.ts src/components/hex-grid/propManifest.test.ts src/author/paletteData.test.ts
```

Expected: FAIL because generation/exact alias resolution is absent.

- [ ] **Step 4: Implement the generator and facade**

The generator reads provider JSON from `RPG_GAME_ASSETS_DIR`, validates it, and writes one deterministic TypeScript file. Replace the hand-copied `PROP_KEYS` body with a typed projection from generated data while preserving public `PropVariant`, `resolvePropVariant`, and `resolvePropModelUrl` interfaces.

Resolver order is exact then family default:

```ts
export function resolvePropVariant(ref: string | undefined): PropVariant | undefined {
  if (!ref) return undefined;
  const direct = GENERATED_PROP_REFS[ref as keyof typeof GENERATED_PROP_REFS];
  if (direct) return direct;
  const exact = GENERATED_PROP_FAMILY_DEFAULTS[
    ref as keyof typeof GENERATED_PROP_FAMILY_DEFAULTS
  ];
  return exact
    ? GENERATED_PROP_REFS[exact as keyof typeof GENERATED_PROP_REFS]
    : undefined;
}
```

Do not truncate or reconstruct multi-part refs with `split(':').pop()` except for display abbreviations.

- [ ] **Step 5: Generate palette entries without aliases**

Build `PALETTE_PROPS` from `GENERATED_PROP_PALETTE_REFS`, not `Object.keys(PROP_KEYS)`. Use provider display names for labels, include explicit `blocksMovement` and `blocksLoS`, and retain short abbreviations only as the compact board glyph. Update `DungeonBuilder`'s `PROP_DEFAULTS` to copy those booleans directly instead of deriving them from role.

- [ ] **Step 6: Add the consumer command**

Add:

```json
"props:sync": "npm run assets:sync && node scripts/generate-prop-catalog.mjs"
```

The command refuses a dirty generated file only in `--check` mode; ordinary mode writes deterministic output for review. It never stages or commits synchronized GLBs.

- [ ] **Step 7: Run against the merged provider and commit**

After the provider PR merges:

```bash
RPG_GAME_ASSETS_DIR=/home/kirk/game-dev/rpg-game-assets npm run props:sync
npm test -- --run scripts/generatePropCatalog.test.ts src/components/hex-grid/propManifest.test.ts src/author/paletteData.test.ts src/author/DungeonBuilder.test.tsx src/components/session/AtlasPropModel.test.tsx
npm run typecheck
npm run build
test -z "$(git ls-files 'public/models/synty/**')"
git diff --check
git add package.json scripts/generate-prop-catalog.mjs scripts/generatePropCatalog.test.ts src/components/hex-grid src/author/paletteData.ts src/author/paletteData.test.ts
git commit -m "feat: consume calibrated exact-ref props"
```

Push and open a reviewed web PR; do not merge locally.

---

### Task 8: Prove exact refs through Builder and gameplay

**Repositories:** merged `rpg-game-assets`, web implementation worktree, running local API/toolkit stack.

**Files:**
- Modify: focused existing tests only where the live round trip exposes a missing assertion.
- Create: `docs/evidence/calibrated-props-v1/README.md`
- Create: license-safe screenshots under the same evidence directory.

**Interfaces:**
- Consumes: merged provider revision and generated web resolver.
- Produces: exact-ref persistence/rendering evidence and final PR gate.

- [ ] **Step 1: Add an automated multi-part ref round-trip fixture**

Use one actual promoted exact ref in `dungeonYaml.test.ts` and the existing authoring/session conversion tests. Assert the entire ref string survives parse, emit, placement update, save payload, API persistence/projection, and atlas-to-scene mapping. Do not assert only the final segment.

- [ ] **Step 2: Run focused cross-boundary tests**

```bash
npm test -- --run src/author/dungeonYaml.test.ts src/author/preview3d/DungeonPreview3D.test.ts src/components/session/atlasToScene3D.test.ts
```

Run the owning focused Go tests in `rpg-toolkit` and `rpg-api` for prop persistence/projection. If any seam invokes the current exactly-three-part `core.Ref.ParseString`, stop and implement or merge the separately reviewed multi-part Ref support before continuing; do not encode the variant into one flattened segment.

- [ ] **Step 3: Perform the local visual workflow**

1. Start the normal local API and web stack.
2. Open Dungeon Builder.
3. Confirm every promoted exact ref appears once, grouped by family.
4. Place at least one obstacle, one cover prop, and one decor prop.
5. Exercise compass facing and within-cell offset.
6. Save, reopen, and confirm exact refs/behavior are unchanged.
7. Save & Play and confirm the same GLBs, scales, grounding, and facings appear.
8. Confirm unsupported exact ref test content renders empty with a visible diagnostic rather than another family member.

- [ ] **Step 4: Capture license-safe evidence**

Record provider commit, provider catalog hash, generated TypeScript hash, selected refs, commands, and Kirk's verdict. Screenshots may show shipped game assets but must not contain source paths, local absolute paths, Blender source, cache listings, or unpromoted licensed models.

- [ ] **Step 5: Run final repository verification**

In `rpg-game-assets`:

```bash
python3 scripts/test_prop_promotion_recipe.py
python3 scripts/test_prepare_prop_calibration.py
python3 scripts/test_normalize_prop_for_runtime.py
python3 scripts/test_build_prop_web_catalog.py
python3 scripts/test_promote_props.py
python3 scripts/build_synty_complete_inventory.py --check
git status --short
```

In `rpg-dnd5e-web`:

```bash
npm test -- --run src/dev/prop-calibration scripts/generatePropCatalog.test.ts src/components/hex-grid/propManifest.test.ts src/author/paletteData.test.ts src/author/dungeonYaml.test.ts
npm run ci-check
test -z "$(git ls-files 'public/models/synty/**')"
git status --short
```

Expected: all checks pass; only intended tracked changes exist; synchronized/calibration GLBs remain ignored.

- [ ] **Step 6: Commit evidence and stop for merge decision**

```bash
git add docs/evidence src scripts package.json
git commit -m "test: verify calibrated props through save and play"
```

Update both PR bodies with exact commits, catalog/inventory hashes, test output, visual evidence, residual limitations, and the authenticated Team signature. Leave both PRs for Kirk's merge decision.
