# Original Dice Runtime Integration — Stone 0 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the merged Concepts Lab's provisional Lightning provider with the asset-owned Original carved d20 runtime contract, preserving the authoritative presentation boundary while enabling exact 1–20 physical settlement.

**Architecture:** `rpg-game-assets` first generates a deterministic consumer-safe runtime manifest beside promoted custom-dice GLBs. After Kirk merges that provider PR, `rpg-dnd5e-web` syncs and strictly validates the private runtime root, hashes the selected GLB before parsing, adapts the material-free carved mesh through asset-owned triangle groups, and supplies the validated provider behind the unchanged `DiceTrayPresentation → DiceTray3D` authority boundary.

**Tech Stack:** Python 3 asset validators/generators, private GLB runtime assets, POSIX shell/rsync, React 19, TypeScript 5.8, Three.js / React Three Fiber, Vitest, Testing Library, Playwright/Chromium, Vite Concepts Lab.

**Spec:** `rpg-project/ideas/interactive-dice-tray/design.md`

## Global Constraints

- Provider issue: `rpg-game-assets#53`; consumer issue: `rpg-dnd5e-web#751`; initiative: `rpg-project#219`; design PR: `rpg-project#220`.
- Work lands inside-out: assets PR first, Kirk merge, then web branch from fresh `origin/dev`. Kirk alone merges both PRs.
- Use one branch per owning repo for Stone 0. Do not split follow-up findings into stacked PRs; keep same-repo Stone 0 fixes on that repo's branch.
- `library/custom-dice/` is authoring authority only. No consumer may read it at runtime.
- Approved private runtime roots are `harness/models/synty/` and `harness/models/custom-dice/`; do not move custom dice into the misleading Synty namespace.
- Generated runtime manifest path is `harness/models/custom-dice/dice-tray-presets.json`.
- Runtime preset IDs and model paths resolve only from the strict asset manifest. Caller props/events never provide an asset URL.
- Stone 0 selects only `dice.original.carved.d20`. The painted-number family from assets PR #52 remains a later preset despite being present in the generated runtime manifest.
- Exact Original carved d20 GLB SHA-256 is `87bf2d0535023e69c968fb9878ba4ad990df4eeec4b503ebb0e917419c47a77e`; size is 491,312 bytes. A mismatch fails closed before GLTF parsing.
- All result quaternions, selectors, authored bounds, coordinate facts, and body/numeral triangle groups remain asset-owned. Web code contains no copied settlement map or model-specific exception table.
- The web owns only presentation normalization. Center on `(bboxMin + bboxMax) / 2`, set `targetMaxExtent = 0.55` world units, and compute `uniformScale = 0.55 / max(dimensions)`. Original d20 dimensions `[10,10,10]` therefore yield `0.055`. Keep that extent unchanged across the approved 1440/1241/1240/760 review widths because the 356px drawer is unchanged; settled projected geometry must retain at least 8 CSS px clearance from every well edge. Do not reuse Lightning group scale `1.1` as an Original asset fact.
- Preserve `DiceTrayPresentation`, `presentationId` as sole external correlation, immutable first-request/first-release authority, Roller/Spectator restrictions, independent renderer generations/telemetry, structured combat logs, responsive drawer layout, and truthful SVG fallback.
- Stone 0 does not implement held pointer-follow, shake energy, `VisualThrowProfile@1`, personalized release choreography, multi-die groups, rigid-body physics, production transport, profile/loadout ownership, or damage-die authority.
- No Canvas or renderer CSS transforms. Three.js object transforms only.
- Private GLBs, private runtime manifests, screenshots, GIFs, JSON evidence, and frame sequences stay ignored/untracked in the public web repository.
- Test-first for every behavior change: capture RED for the intended missing behavior, implement minimally, rerun focused and protected suites, then commit.
- Every implementation task ends with an independent review. Asset and web PRs are ready, never draft. GitHub comments use `— asset-pipeline agent, on behalf of KirkDiggler`.
- Final sign-off requires exact-commit browser evidence on the real Concepts Lab route; CI green alone is insufficient.

---

### Task 1: Publish the custom-dice consumer runtime contract (`rpg-game-assets#53`)

**Files:**
- Create: `scripts/build_dice_runtime_manifest.py`
- Create: `scripts/test_build_dice_runtime_manifest.py`
- Modify: `scripts/validate_dice_tray_presets.py`
- Modify: `scripts/test_validate_dice_tray_presets.py`
- Modify: `library/custom-dice/dice-tray-presets.json`
- Generate: `harness/models/custom-dice/dice-tray-presets.json`
- Modify: `README.md`
- Modify: `library/custom-dice/README.md`

**Interfaces:**
- Consumes: authoring contract `library/custom-dice/dice-tray-presets.json`; exact promoted GLBs under `harness/models/custom-dice/`; carved `*.face-tags.authority.json` triangle groups from assets PR #51.
- Produces: deterministic `dice-runtime-presets@1` JSON consumed by the web; `build_runtime_manifest(repo_root: Path) -> dict`; `canonical_json(value: dict) -> bytes`; CLI write and `--check` modes.

The generated contract conforms to these exact field types:

```ts
type LowercaseSha256 = string; // validator requires /^[0-9a-f]{64}$/
type QuaternionXyzw = readonly [number, number, number, number];

type RuntimeSelectors =
  | {
      readonly kind: 'single-mesh';
      readonly objectNode: string;
      readonly meshDefinition: string;
    }
  | {
      readonly kind: 'multi-node';
      readonly rootObjectNode: string;
      readonly shellObjectNode: string;
      readonly numeralObjectNodeCount: number;
    };

type RuntimeGeometry =
  | {
      readonly kind: 'single-mesh-triangle-groups';
      readonly totalTriangles: number;
      readonly bodyTriangleIndices: readonly number[];
      readonly numeralTriangleIndices: readonly number[];
    }
  | { readonly kind: 'multi-node' };

interface RuntimeBounds {
  readonly bboxMin: readonly [number, number, number];
  readonly bboxMax: readonly [number, number, number];
  readonly dimensions: readonly [number, number, number];
}

interface RuntimeMeshFacts {
  readonly primitiveCount: number;
  readonly triangles: number;
  readonly materials: number;
  readonly textures: number;
}

interface DiceRuntimePresetRecord {
  readonly presetId: string;
  readonly displayName: string;
  readonly familyId: string;
  readonly dieKind: 'd20' | 'd12' | 'd10-percentile' | 'd10' | 'd8' | 'd6' | 'd4';
  readonly model: {
    readonly path: string;
    readonly sha256: LowercaseSha256;
    readonly sizeBytes: number;
    readonly selectors: RuntimeSelectors;
    readonly bounds: RuntimeBounds;
    readonly meshFacts: RuntimeMeshFacts;
    readonly geometry: RuntimeGeometry;
  };
  readonly faceSettlementMap: {
    readonly supportedResults: readonly number[];
    readonly entries: Readonly<
      Record<string, { readonly faceIndex: number; readonly quaternion: QuaternionXyzw }>
    >;
  };
}

interface DiceRuntimeManifestRecord {
  readonly $schemaVersion: 1;
  readonly contract: 'dice-runtime-presets';
  readonly generatedBy: 'build_dice_runtime_manifest@1.0.0';
  readonly sourceManifestSha256: LowercaseSha256;
  readonly runtimeRoot: 'harness/models/custom-dice';
  readonly coordinateContract: {
    readonly assetUpAxis: 'Y-up glTF';
    readonly assetUnits: 'glTF scene units';
    readonly quaternionConvention: 'x,y,z,w';
    readonly settlementMapMeaning: string;
  };
  readonly presets: readonly DiceRuntimePresetRecord[];
}
```

The real `dice.original.carved.d20` record must carry:

```json
{
  "presetId": "dice.original.carved.d20",
  "displayName": "Original Carved D20",
  "familyId": "dice.original.carved",
  "dieKind": "d20",
  "modelPath": "original-set/Original_D20_Source.glb",
  "runtimeSha256": "87bf2d0535023e69c968fb9878ba4ad990df4eeec4b503ebb0e917419c47a77e",
  "sizeBytes": 491312,
  "objectNode": "Original_D20_Source_NO_MATERIALS",
  "meshDefinition": "Original_D20_Source_NO_MATERIALS_mesh",
  "boundsMin": [-5.0, -5.0, -5.0],
  "boundsMax": [5.0, 5.0, 5.0],
  "primitiveCount": 1,
  "totalTriangles": 10482,
  "bodyTriangleCount": 3563,
  "numeralTriangleCount": 6919
}
```

Those values populate the typed nested fields above; the JSON fragment is a review checklist, not an alternative schema. The face map has supported results `[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20]` and one entry for each. For painted-number presets, the generated manifest carries their existing `multi-node` selector contract, but Stone 0 web support remains allowlisted to carved d20.

- [ ] **Step 1: Create the fresh assets worktree**

```bash
cd /home/kirk/game-dev/rpg-game-assets
git fetch origin
git worktree add .worktrees/53-dice-runtime-contract -b asset/53-dice-runtime-contract origin/main
cd .worktrees/53-dice-runtime-contract
```

Expected: branch starts from the current `origin/main` containing PRs #50–#52; no files are staged; parent checkout's `.pi/` and `.claude/worktrees/` are untouched.

- [ ] **Step 2: Write failing generator tests**

In `scripts/test_build_dice_runtime_manifest.py`, create temporary authoring/runtime fixtures and assert:

```python
runtime = build_runtime_manifest(repo_root)
assert runtime["$schemaVersion"] == 1
assert runtime["contract"] == "dice-runtime-presets"
assert runtime["runtimeRoot"] == "harness/models/custom-dice"
assert [p["presetId"] for p in runtime["presets"]] == sorted(
    p["presetId"] for p in runtime["presets"]
)
assert canonical_json(runtime).endswith(b"\n")
```

Also assert the projected JSON contains none of:

```python
for forbidden in (
    "library/", ".blend", "evidence/", "sourceBlend", "sourceNode",
    "taggedBlend", "authorityPath", "provenance"
):
    assert forbidden not in canonical_json(runtime).decode()
```

Add independent cases for:

- output byte determinism under repeated generation;
- `--check` rejecting stale/missing output;
- promoted path traversal or any path outside `harness/models/custom-dice/`;
- source/promoted GLB hash mismatch;
- authoring `sizeBytes` mismatch, source/promoted byte-size mismatch, and emitted size not derived from promoted bytes;
- missing promoted GLB;
- selector/root/mesh mismatch against GLB JSON;
- bounds mismatch against accessor bounds;
- carved triangle overlap, gap, duplicate, negative, out-of-range, wrong total, or wrong bound hash;
- supported-result duplicate/gap/extra/out-of-range key;
- non-finite or non-unit quaternion;
- a complete map whose hash differs from the emitted model hash; and
- an authoring-only field leaking into output; and
- stale cross-field family metadata, including the current carved d6 `trayFaceMapStatus`, disagreeing with its complete preset map.

- [ ] **Step 3: Run RED**

```bash
python3 scripts/test_build_dice_runtime_manifest.py
```

Expected: FAIL because `build_dice_runtime_manifest.py` and its interfaces do not exist.

- [ ] **Step 4: Strengthen authoring validation before projection**

Modify `validate_dice_tray_presets.py` so every tray-ready preset requires:

- promoted path inside `harness/models/custom-dice/`;
- byte-identical source/promoted GLB hashes;
- authoring `sizeBytes` equal to both source and promoted byte lengths, while emitted `sizeBytes` is derived from promoted bytes;
- explicit selectors and bounds matching promoted bytes;
- unique supported results exactly matching the die kind;
- decimal-string result keys matching supported results;
- finite normalized `xyzw` quaternions;
- complete-map hash matching runtime hash; and
- a valid geometry contract.

Add an explicit authoring link from each carved preset to its `*.face-tags.authority.json`; validate that authority file's `triangleGroups.runtimeSha256` matches the runtime GLB and that the two triangle arrays form one disjoint complete partition.

Expand `test_validate_dice_tray_presets.py` with mutation tests for each new failure class before changing validator behavior; run once to see the expected failures, then implement until green.

- [ ] **Step 5: Implement deterministic runtime projection**

Implement four public module functions with these signatures and responsibilities:

- `build_runtime_manifest(repo_root: Path) -> dict[str, object]` validates authoring/provider inputs and returns the consumer projection.
- `canonical_json(value: dict[str, object]) -> bytes` returns deterministic UTF-8 JSON with two-space indentation and one trailing newline.
- `write_runtime_manifest(repo_root: Path, output: Path) -> None` writes through a sibling temporary file and atomically replaces the output.
- `check_runtime_manifest(repo_root: Path, output: Path) -> None` byte-compares generated content and raises a nonzero CLI failure without writing.

CLI behavior:

```bash
python3 scripts/build_dice_runtime_manifest.py
python3 scripts/build_dice_runtime_manifest.py --check
```

The write command atomically replaces `harness/models/custom-dice/dice-tray-presets.json`. `--check` generates in memory and byte-compares without changing files. JSON uses UTF-8, two-space indentation, deterministic insertion order, sorted presets, numerically ordered result entries, and one trailing newline.

- [ ] **Step 6: Generate and inspect the real runtime contract**

```bash
python3 scripts/build_dice_runtime_manifest.py
python3 scripts/build_dice_runtime_manifest.py --check
python3 scripts/validate_dice_tray_presets.py
python3 scripts/test_validate_dice_tray_presets.py
python3 scripts/test_build_dice_runtime_manifest.py
```

Expected:

- all commands exit zero;
- runtime manifest contains every promoted/review-approved/complete Original preset;
- `dice.original.carved.d20` has exactly 491,312 promoted bytes, 20 results, and triangle counts 3,563 body + 6,919 numeral = 10,482;
- runtime manifest contains no authoring/library/evidence path; and
- no GLB bytes changed.

- [ ] **Step 7: Correct the repository contract documentation**

Update `README.md` to declare:

```text
harness/models/synty/       # licensed Synty runtime root
harness/models/custom-dice/ # private custom-dice runtime root
```

Update `library/custom-dice/README.md` to distinguish the authoring manifest from generated `harness/models/custom-dice/dice-tray-presets.json`, document generator/check commands, and name `rpg-dnd5e-web#751` as the consumer handoff. Do not claim custom dice are part of the Synty inventory.

- [ ] **Step 8: Verify, commit, push, and open the ready provider PR**

```bash
python3 scripts/validate_dice_tray_presets.py
python3 scripts/test_validate_dice_tray_presets.py
python3 scripts/test_build_dice_runtime_manifest.py
python3 scripts/build_dice_runtime_manifest.py --check
git diff --check
git status --short
git add \
  README.md \
  library/custom-dice/README.md \
  library/custom-dice/dice-tray-presets.json \
  harness/models/custom-dice/dice-tray-presets.json \
  scripts/build_dice_runtime_manifest.py \
  scripts/test_build_dice_runtime_manifest.py \
  scripts/validate_dice_tray_presets.py \
  scripts/test_validate_dice_tray_presets.py
git commit -m "feat: publish custom dice runtime contract (#53)"
git push -u origin asset/53-dice-runtime-contract
```

Open one ready PR to `main` with `Closes #53`, load-bearing paths, exact commands/results, manifest/GLB hashes, and the signed handoff. Commission an independent assets gate. **Stop until Kirk merges this provider PR.**

---

### Task 2: Extend the private web asset boundary (`rpg-dnd5e-web#751`)

**Files:**
- Create: `scripts/sync-game-assets.sh`
- Create: `scripts/sync-game-assets.test.ts`
- Modify: `scripts/sync-synty-assets.sh` (compatibility shim only)
- Modify: `package.json`
- Modify: `.gitignore`
- Modify: `scripts/attack-die/build-frozen.sh`
- Modify: `scripts/attack-die/serve-frozen.mjs`
- Test: `scripts/attack-die/evidenceProtocol.test.ts`

**Interfaces:**
- Consumes: merged assets runtime roots and generated manifest from Task 1.
- Produces: `npm run assets:sync` mirroring Synty and custom-dice roots; environment-testable shell interface using `RPG_GAME_ASSETS_DIR`, `RPG_WEB_ROOT`, and `ASSETS_SYNC_SKIP_UPDATE=1`; frozen server mount `/models/custom-dice/**`.

- [ ] **Step 1: Create the fresh web worktree after provider merge**

```bash
cd /home/kirk/game-dev/rpg-dnd5e-web
git fetch origin
git worktree add .worktrees/751-original-dice-runtime -b feat/751-original-dice-runtime origin/dev
cd .worktrees/751-original-dice-runtime
```

Expected: base includes merged PR #750 and no local held-motion experiment commits.

- [ ] **Step 2: Write failing sync-boundary tests**

Create temporary assets/web roots in `sync-game-assets.test.ts`, invoke the shell script with:

```ts
{
  RPG_GAME_ASSETS_DIR: assetsRoot,
  RPG_WEB_ROOT: webRoot,
  ASSETS_SYNC_SKIP_UPDATE: '1',
}
```

Assert:

- Synty files mirror to `public/models/synty/`;
- custom-dice GLBs and `dice-tray-presets.json` mirror to `public/models/custom-dice/`;
- stale destination files are deleted independently per root;
- `library/`, `.blend`, and `evidence/` files never copy;
- missing either approved source root exits nonzero without partially deleting the other destination; and
- both destination roots match `.gitignore`.

Run:

```bash
npm run test:run -- scripts/sync-game-assets.test.ts
```

Expected: FAIL because the new script and second runtime root do not exist.

- [ ] **Step 3: Implement the two-root sync**

`scripts/sync-game-assets.sh` resolves or updates the private checkout, validates both source roots before mutation, then runs separate `rsync -a --delete` operations. `scripts/sync-synty-assets.sh` becomes a compatibility shim executing the new script. Update `package.json`:

```json
"assets:sync": "sh scripts/sync-game-assets.sh"
```

Ignore:

```gitignore
public/models/synty/
public/models/custom-dice/
```

- [ ] **Step 4: Extend frozen-provider safety**

`build-frozen.sh` must temporarily move both ignored provider roots out of `public/`, restore both through the existing signal-safe trap, and prove neither private tree entered `dist`.

`serve-frozen.mjs` must accept separate read-only roots and serve:

```text
/models/synty/**       -> --synty-root
/models/custom-dice/** -> --custom-dice-root
```

Reuse the existing canonical-path/traversal/symlink containment rules. Add evidence protocol tests for the second mount and for traversal/symlink rejection.

- [ ] **Step 5: Verify and commit the boundary**

```bash
npm run test:run -- \
  scripts/sync-game-assets.test.ts \
  scripts/attack-die/evidenceProtocol.test.ts \
  scripts/attack-die/frozenBuildManifest.test.ts
npm run typecheck
npx eslint scripts/sync-game-assets.test.ts scripts/attack-die/evidenceProtocol.test.ts
sh -n scripts/sync-game-assets.sh scripts/sync-synty-assets.sh scripts/attack-die/build-frozen.sh
npx prettier --check \
  scripts/sync-game-assets.test.ts \
  scripts/attack-die/serve-frozen.mjs \
  package.json
git diff --check
git add scripts package.json .gitignore
git commit -m "feat: sync custom dice runtime assets (#751)"
```

Expected: private provider directories remain ignored and unstaged.

---

### Task 3: Strictly validate and coalesce the Original runtime provider

**Files:**
- Create: `src/components/ui/dice/diceRuntimeManifest.ts`
- Create: `src/components/ui/dice/diceRuntimeManifest.test.ts`
- Create: `src/components/ui/dice/diceRuntimeProvider.ts`
- Create: `src/components/ui/dice/diceRuntimeProvider.test.ts`
- Modify: `src/components/ui/dice/dicePresentationRelease.ts`
- Modify: `src/components/ui/dice/dicePresentationRelease.test.ts`
- Modify: `src/components/ui/dice/dicePresentationEvent.ts`
- Modify: `src/components/ui/dice/dicePresentationEvent.test.ts`
- Modify: `src/components/ui/dice/attackDieRuntime.ts`
- Test: `src/components/ui/dice/attackDieRuntime.test.ts`

**Interfaces:**
- Consumes: `/models/custom-dice/dice-tray-presets.json`; allowlisted preset ID `dice.original.carved.d20`.
- Produces: `parseDiceRuntimeManifest(value: unknown): DiceRuntimeManifestResult`; `preloadDiceRuntimePreset(presetId: string): Promise<void>`; `getDiceRuntimePresetSnapshot(presetId: string): DiceRuntimePresetSnapshot`; one immutable validated contract plus parsed source scene per preset.

Runtime types:

```ts
type RuntimeDiceSelectors =
  | {
      readonly kind: 'single-mesh';
      readonly objectNode: string;
      readonly meshDefinition: string;
    }
  | {
      readonly kind: 'multi-node';
      readonly rootObjectNode: string;
      readonly shellObjectNode: string;
      readonly numeralObjectNodeCount: number;
    };

interface CarvedTriangleGroupGeometry {
  readonly kind: 'single-mesh-triangle-groups';
  readonly totalTriangles: number;
  readonly bodyTriangleIndices: readonly number[];
  readonly numeralTriangleIndices: readonly number[];
}

interface MultiNodeGeometry {
  readonly kind: 'multi-node';
}

interface DiceSettlementFace {
  readonly faceIndex: number;
  readonly quaternion: readonly [number, number, number, number];
}

type RuntimeDieKind =
  | 'd20'
  | 'd12'
  | 'd10-percentile'
  | 'd10'
  | 'd8'
  | 'd6'
  | 'd4';

interface DiceRuntimePreset {
  readonly presetId: string;
  readonly displayName: string;
  readonly familyId: string;
  readonly dieKind: RuntimeDieKind;
  readonly model: {
    readonly path: string;
    readonly sha256: string;
    readonly sizeBytes: number;
    readonly selectors: RuntimeDiceSelectors;
    readonly bounds: {
      readonly bboxMin: readonly [number, number, number];
      readonly bboxMax: readonly [number, number, number];
      readonly dimensions: readonly [number, number, number];
    };
    readonly meshFacts: {
      readonly primitiveCount: number;
      readonly triangles: number;
      readonly materials: number;
      readonly textures: number;
    };
    readonly geometry: CarvedTriangleGroupGeometry | MultiNodeGeometry;
  };
  readonly faceSettlementMap: {
    readonly supportedResults: readonly number[];
    readonly entries: Readonly<Record<string, DiceSettlementFace>>;
  };
}

interface DiceRuntimeManifest {
  readonly $schemaVersion: 1;
  readonly contract: 'dice-runtime-presets';
  readonly generatedBy: 'build_dice_runtime_manifest@1.0.0';
  readonly sourceManifestSha256: string;
  readonly runtimeRoot: 'harness/models/custom-dice';
  readonly coordinateContract: {
    readonly assetUpAxis: 'Y-up glTF';
    readonly assetUnits: 'glTF scene units';
    readonly quaternionConvention: 'x,y,z,w';
    readonly settlementMapMeaning: string;
  };
  readonly presets: readonly DiceRuntimePreset[];
}

type DiceRuntimeManifestResult =
  | { readonly ok: true; readonly manifest: DiceRuntimeManifest }
  | { readonly ok: false; readonly reason: string };

interface RuntimeMeshBinding {
  readonly objectNode: string;
  readonly meshDefinition: string;
  readonly meshDefinitionIndex: number;
}

interface DiceRuntimePresetSnapshot {
  readonly status: 'idle' | 'loading' | 'ready' | 'failed';
  readonly preset?: DiceRuntimePreset;
  readonly scene?: Object3D;
  readonly binding?: RuntimeMeshBinding;
  readonly failureReason?: string;
}
```

- [ ] **Step 1: Extend the bounded preset identifier with a failing contract test**

Update release/event tests first. The identifier grammar is total length `1–64`, one to eight dot-separated segments, and every segment matches `/^[a-z][a-z0-9-]{0,31}$/`. Require both `lightning` and `dice.original.carved.d20` to pass. Require leading/trailing dots, empty segments, `..`, `/`, `\\`, `:`, `%`, URL-shaped text, traversal text, uppercase, an over-32-character segment, and total length over 64 to fail in release construction and inbound event parsing.

Run:

```bash
npm run test:run -- \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/dicePresentationEvent.test.ts
```

Expected: the new Original ID assertions fail because merged `PRESET_IDENTIFIER` admits only one undotted segment. Implement one shared identifier predicate; do not special-case the Original ID.

- [ ] **Step 2: Write strict runtime-manifest parser tests**

Start from a minimal valid synthetic d20 fixture and mutate one property per test. Require exact keys, bounded safe IDs, runtime-relative non-URL model path, lowercase 64-hex hash, exact byte size, finite bounds, a geometry discriminator, unique decimal result keys, and finite normalized quaternions. Recursively reconstruct/freeze accepted values. Validate the exact result set by kind: d20 `1–20`, d12 `1–12`, d10 `1–10`, d10-percentile `[0, 10, 20, 30, 40, 50, 60, 70, 80, 90]`, d8 `1–8`, d6 `1–6`, and d4 `1–4`. The provider later narrows the selected record to carved d20.

Reject unknown keys, accessors/getters/proxies that throw, path traversal, absolute/URL paths, duplicate results, geometry partition overlap/gap/out-of-range, wrong triangle total, unknown schema/contract, wrong die kind, authoring fields, and an incomplete map.

- [ ] **Step 3: Run runtime parser RED and implement**

```bash
npm run test:run -- src/components/ui/dice/diceRuntimeManifest.test.ts
```

Expected: FAIL because the parser does not exist. Implement the pure parser without importing Three.js or performing I/O; rerun to GREEN.

- [ ] **Step 4: Write provider loading/coalescing and handoff tests**

Use a valid fixture with every top-level and nested field required by Task 1, serialize it with the same canonical ordering, and prove Task 3 accepts those exact bytes without dropping `generatedBy`, `runtimeRoot`, or `coordinateContract`. Mock fetch, digest, and GLTF parsing. Assert:

1. two concurrent consumers trigger one manifest request, one GLB request, one digest, and one parse;
2. selection is by allowlisted preset ID only, never caller URL;
3. response failure, malformed manifest, missing preset, byte-size mismatch, digest mismatch, or invalid map yields one shared failed snapshot;
4. `GLTFLoader.parse` is never called before successful size/hash/contract validation;
5. a valid exact hash parses once and exposes one source scene/contract to both witnesses;
6. provider parsing validates through `gltf.parser.json` that the declared `objectNode` references the declared `meshDefinition`, then exposes a prepared binding keyed by the Object3D node name rather than trying to find a mesh-definition name in `scene`;
7. a fixture whose glTF node name and mesh-definition name differ still binds correctly, while a wrong node→mesh index fails; and
8. reset hooks exist only for tests.

- [ ] **Step 5: Implement the coalesced provider and loader binding**

Fetch order is strict:

```text
manifest bytes → strict JSON parse → select allowlisted preset
→ GLB response/bytes → size check → SHA-256 check → GLTF parse
```

Cache manifest ownership once and preset ownership by `presetId + sha256`. Keep the `GLTF` parser result long enough to validate the glTF node→mesh-definition binding, then store the source scene plus an immutable binding `{ objectNode, meshDefinition, meshDefinitionIndex }` in the ready snapshot. Failed ownership remains failed for the mounted lifecycle; do not race retries from paired witnesses. Keep the legacy Lightning loader available only to historical non-Tray concept stages until final cleanup proves it unused.

- [ ] **Step 6: Verify and commit provider loading**

```bash
npm run test:run -- \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/dicePresentationEvent.test.ts \
  src/components/ui/dice/diceRuntimeManifest.test.ts \
  src/components/ui/dice/diceRuntimeProvider.test.ts \
  src/components/ui/dice/attackDieRuntime.test.ts
npm run typecheck
npx eslint src/components/ui/dice/diceRuntimeManifest.ts \
  src/components/ui/dice/diceRuntimeProvider.ts
npx prettier --check src/components/ui/dice/diceRuntimeManifest* \
  src/components/ui/dice/diceRuntimeProvider*
git diff --check
git add src/components/ui/dice
git commit -m "feat: validate Original dice runtime provider (#751)"
```

---

### Task 4: Render the material-free carved mesh from asset-owned geometry groups

**Files:**
- Create: `src/components/ui/dice/materialFreeCarvedMesh.ts`
- Create: `src/components/ui/dice/materialFreeCarvedMesh.test.ts`
- Modify: `src/components/ui/dice/attackDieMaterial.ts`
- Modify: `src/components/ui/dice/attackDieMaterial.test.ts`
- Modify: `src/components/ui/dice/AttackDie3D.tsx`
- Modify: `src/components/ui/dice/AttackDie3D.test.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.test.tsx`
- Modify: `src/components/ui/dice/DiceTrayPresentation.tsx`
- Modify: `src/components/ui/dice/DiceTrayPresentation.test.tsx`

**Interfaces:**
- Consumes: validated `DiceRuntimePreset`, parsed source scene, validated `RuntimeMeshBinding`, authoritative result, existing phase/release/reduced-motion inputs.
- Produces: `prepareMaterialFreeCarvedScene(scene, preset, binding, treatment): PreparedDiceScene`; exact result target from `preset.faceSettlementMap.entries[String(result)].quaternion`; existing `AttackDieTelemetry` semantics.

```ts
interface DiceMaterialTreatment {
  readonly bodyColor: string;
  readonly numeralColor: string;
  readonly roughness: number;
  readonly metalness: number;
}

interface PreparedDiceScene {
  readonly scene: Object3D;
  readonly dispose: () => void;
}
```

`prepareMaterialFreeCarvedScene` accepts `(scene: Object3D, preset: DiceRuntimePreset, binding: RuntimeMeshBinding, treatment: DiceMaterialTreatment)` and returns `PreparedDiceScene`.

- [ ] **Step 1: Write geometry discrimination tests**

Build a small indexed `BufferGeometry` fixture. Assert preparation:

- finds exactly one scene Object3D named by `binding.objectNode` and requires it to be the Mesh selected by the provider-validated node→mesh-definition binding;
- never attempts `scene.getObjectByName(binding.meshDefinition)`, because GLTFLoader names the Object3D from the glTF node rather than the mesh definition;
- verifies indexed triangle count against `totalTriangles`;
- copies source triangle triples in body-then-numeral order into a cloned index buffer;
- creates exactly two contiguous geometry groups with material indices 0 and 1;
- never mutates source scene/geometry/materials;
- assigns owned runtime body and numeral materials; and
- disposes owned clones/materials exactly once.

Reject missing/duplicate node, non-mesh selector, non-indexed geometry, count mismatch, invalid group partition, and unexpected child mesh.

- [ ] **Step 2: Run geometry RED and implement**

```bash
npm run test:run -- \
  src/components/ui/dice/materialFreeCarvedMesh.test.ts \
  src/components/ui/dice/attackDieMaterial.test.ts
```

Expected: FAIL because carved scene preparation does not exist.

Implement one-time scene preparation. Triangle-group entries identify triangle ordinals, not vertex indices; copy each referenced source index triple into the reordered index array. Two geometry groups produce two draw calls without thousands of per-triangle groups.

- [ ] **Step 3: Write renderer authority tests**

Assert Original d20:

- receives target only from the validated map for authoritative results 1–20;
- ignores decorative release variation/vector/shake when selecting target;
- preserves exact-target hold and `<= 0.25°` observation semantics;
- recenters from the validated bounds midpoint and computes exactly `0.55 / max(dimensions)`; the real Original d20 yields `0.055`, remains unchanged at approved responsive widths, and preserves at least 8 CSS px settled clearance from every well edge;
- shares provider identity but owns independent scene clones, renderer generations, telemetry, Canvas, and disposal;
- reduced motion still requires explicit release and settles exactly;
- provider/map/geometry failure remains concealed while armed and converges to semantic SVG after release; and
- unknown safe presets never become model URLs; and
- accessible settled status is driven by matching renderer telemetry (`observed` 3D versus `failed`/fallback), never by checking whether `presetId === 'lightning'`.

- [ ] **Step 4: Adapt `AttackDie3D` behind its stable caller boundary**

Add a discriminated internal provider path for `dice-runtime-preset` while retaining the historical Lightning development injection only where old concept tabs still require it. Do not change `DiceTrayPresentation` event authority. `DiceTray3D` allowlists `dice.original.carved.d20` to the new provider and routes unsupported presets to SVG.

Replace `DiceTrayPresentation`'s literal-Lightning status inference with committed local renderer observation: matching `renderer: '3d', state: 'observed', exactTargetHeld: true` records 3D settlement; matching failed/fallback completion records semantic fallback. Stale telemetry remains ignored. This changes announcement truth only, not request/release authority.

Apply movement only to Three.js groups. Do not add Canvas/renderer CSS transforms or held-motion behavior.

- [ ] **Step 5: Verify and commit rendering**

```bash
npm run test:run -- \
  src/components/ui/dice/materialFreeCarvedMesh.test.ts \
  src/components/ui/dice/attackDieMaterial.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx
npm run typecheck
npx eslint src/components/ui/dice
npx prettier --check src/components/ui/dice/materialFreeCarvedMesh* \
  src/components/ui/dice/attackDieMaterial* \
  src/components/ui/dice/AttackDie3D* \
  src/components/ui/dice/DiceTray3D*
git diff --check
git add src/components/ui/dice
git commit -m "feat: render Original carved d20 contract (#751)"
```

---

### Task 5: Switch the Tray stage, prove results 1–20, and finalize Stone 0

**Files:**
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.tsx`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx`
- Modify: `src/concepts/attack-die-3d/diceTrayWitnessFixture.ts`
- Modify: `src/concepts/attack-die-3d/diceTrayWitnessFixture.test.ts`
- Create: `scripts/attack-die/stone0TrayEvidenceProtocol.ts`
- Create: `scripts/attack-die/stone0TrayEvidenceProtocol.test.ts`
- Create: `scripts/attack-die/capture-stone0-tray-evidence.mjs`
- Modify: `package.json`
- Modify: `docs/how-to/attack-die-3d-concept.md`
- Private outputs: `/home/kirk/game-dev/.verification/interactive-dice-tray/stone-0/`

**Interfaces:**
- Consumes: exact merged provider contract and web runtime from Tasks 2–4.
- Produces: browser-visible Original carved d20 Tray stage, explicit authoritative result fixture control 1–20, exact-commit evidence, one ready web PR closing #751.

- [ ] **Step 1: Write the failing concept/provider tests**

Assert:

- while provider state is pending, no drawer/Canvas/result mounts and a result-free polite loading status is visible;
- terminal provider failure mounts the shared presentation with no Canvas, preserves the Roller Roll control and Spectator non-authority, conceals the armed result as `?`, and settles to truthful SVG only after the existing matching release event;
- Roller and Spectator receive the same immutable provider contract/source scene and one shared event array, while renderer generations and clones remain distinct;
- requested preset is `dice.original.carved.d20`, never `lightning`;
- authoritative fixture input accepts integers 1–20 and changes request identity before delivery;
- each result selects its exact asset entry;
- Roll/host Monster release behavior remains unchanged;
- malformed/missing provider reaches terminal fail-closed presentation rather than remaining in the pending gate; and
- non-Tray historical Lightning tooling remains clearly provisional and does not supply the Tray provider.

- [ ] **Step 2: Run concept RED and implement the provider gate**

```bash
npm run test:run -- \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/diceTrayWitnessFixture.test.ts \
  src/components/ui/dice/DiceTrayPresentation.test.tsx
```

Expected: FAIL because Tray still requests the Lightning preset/fixed provisional mapping.

Switch only the Tray provider/fixture to Original carved d20. The parent gate distinguishes `loading` from terminal `failed`: only loading withholds the presentation; failed supplies the failure reason to the existing local renderer-failure lifecycle so armed truth remains concealed and release can converge to SVG. Keep Concepts delivery labeled fixture-only with no production transport/profile ownership claim.

Write `stone0TrayEvidenceProtocol.test.ts` before its implementation. Require exact source SHA, build-manifest identity, full provider-manifest and GLB hashes, request/transfer counts, one result fact for each integer 1–20, pending/terminal-failure/reduced-motion/responsive scenario records, deterministic filenames, empty validation failures, and rejection of missing/duplicate/malformed scenario facts. Implement the pure protocol and Playwright driver only after RED.

- [ ] **Step 3: Run focused and full repository gates**

```bash
npm run test:run -- \
  src/components/ui/dice/diceRuntimeManifest.test.ts \
  src/components/ui/dice/diceRuntimeProvider.test.ts \
  src/components/ui/dice/materialFreeCarvedMesh.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/diceTrayWitnessFixture.test.ts
npm run test:run
npm run typecheck
npm run lint
npm run build
git diff --check
git status --short
```

Expected: all gates pass; only ignored provider/evidence files remain untracked or ignored; no private byte is staged.

- [ ] **Step 4: Update documentation and commit the exact candidate**

Update the how-to guide with Original provider path/full hash, generated runtime manifest, 1–20 fixture review, pending-versus-terminal provider behavior, failure exercises, explicit private-evidence limits, and the statement that tactile motion remains Stone 1.

```bash
git add \
  src/concepts/attack-die-3d \
  scripts/attack-die/stone0TrayEvidenceProtocol.ts \
  scripts/attack-die/stone0TrayEvidenceProtocol.test.ts \
  scripts/attack-die/capture-stone0-tray-evidence.mjs \
  package.json \
  docs/how-to/attack-die-3d-concept.md
git commit -m "feat: prove Original d20 settlement in Concepts Lab (#751)"
```

- [ ] **Step 5: Build and serve exact-commit private evidence**

Use the corrected frozen-build path with private custom-dice mounted separately:

```bash
SHA=$(git rev-parse HEAD)
OUT=/home/kirk/game-dev/.verification/interactive-dice-tray/stone-0/$SHA
BUILD_MANIFEST="$OUT/build-manifest.json"
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
node scripts/attack-die/capture-stone0-tray-evidence.mjs \
  --url 'http://127.0.0.1:3003/?concept=attack-die-3d' \
  --out "$OUT" \
  --build-manifest "$BUILD_MANIFEST" \
  --source-sha "$SHA"
kill "$PREVIEW_PID"
trap - EXIT
```

The capture driver owns a deterministic local API fixture, fresh browser contexts per scenario, explicit response mutation for malformed-manifest/hash cases, and real `WEBGL_lose_context` for context loss. It writes `browser-evidence.json`, `network.json`, `console.json`, and deterministic screenshots named by scenario/result/viewport. The evidence protocol rejects any source/build mismatch or incomplete matrix.

On `?concept=attack-die-3d → Tray`, verify:

1. one runtime manifest request, one Original D20 GLB request/transfer, and no duplicate Tray provider load;
2. exact GLB digest `87bf2d0535023e69c968fb9878ba4ad990df4eeec4b503ebb0e917419c47a77e` before Canvas readiness;
3. Roller and Spectator share provider/event values and own distinct WebGL contexts/clones/telemetry;
4. each result 1→20 reaches and holds the asset-owned target within `0.25°`;
5. result target is invariant under Roll versus host release and decorative variation;
6. Player remains armed indefinitely; Monster release remains host-owned;
7. reduced motion requires explicit input and settles exactly without tumble;
8. missing manifest, malformed manifest, GLB hash mismatch, invalid geometry partition, unmapped result, WebGL creation failure, real context loss, and shader failure converge truthfully;
9. desktop `1440×1080`, boundary `1241×900`/`1240×900`, and narrow `760×900` preserve the accepted drawer/log/dock layout and containment; and
10. startup/scenario console and page errors are empty except explicitly documented unrelated API noise.

Store screenshots, JSON, hashes, and logs privately under the exact-SHA output directory. If evidence finds a defect, add a discriminating test, commit the fix, and rerun the entire exact-SHA capture from a new empty output directory. Do not claim formal performance graduation; the paired performance script's final-context caveat remains separate work.

- [ ] **Step 6: Push, open ready PR, reconcile review, and gate**

```bash
git push -u origin feat/751-original-dice-runtime
```

Open one ready PR to `dev` with `Closes #751`, dependency on merged assets #53 PR, exact runtime hashes, focused/full/static commands, private evidence paths/hashes, and no production transport or tactile-motion claim.

Reconcile every Copilot thread individually. Commission an independent fresh-context gate that reruns the full suite, audits strict manifest/hash-before-parse/authority/fallback claims, and independently views browser evidence. Post a signed `GATE REVIEW` verdict. Leave merge to Kirk.

---

## Stone 0 completion and handoff

Stone 0 is complete only after Kirk merges both owning-repo PRs and the exact merged web route still renders the Original carved d20 truthfully. Then:

- record provider/web merge SHAs and evidence on `rpg-project#219` and PR #220;
- update Project 19 items #53 and #751 to Done;
- run the two Stone 0 retro questions from the design;
- rewrite `rpg-project/sessions/active.md` with solid facts and remaining risks; and
- begin a new Stone 1 plan/issue for tactile roll-group gesture and `VisualThrowProfile@1`, without reusing the rejected local rail-motion commits.
