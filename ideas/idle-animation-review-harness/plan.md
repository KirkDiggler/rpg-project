# Idle Animation Review Harness Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.
>
> **Plan location note:** this plan lives beside the approved design at
> `ideas/idle-animation-review-harness/design.md` in `rpg-project`. Implementation
> happens in `rpg-dnd5e-web` under a dedicated implementation issue; this repository
> receives no web implementation files.

**Goal:** Build a Concepts Lab Idle Review bench that hashes and plays each exact
runtime-manifest GLB clip, preserves audit truth across asset changes, and exports
deterministic reviewer evidence without hidden fallbacks.

**Architecture:** `assetIdentity.ts` is the sole fetch-once/Web Crypto seam. The catalog
uses it for the manifest, and one page-lifetime `ExactModelRepository` uses it for GLBs;
the repository owns parsed source resources while stages own only clones/mixers. Pure
state/storage/export functions classify catalog, revalidation, current targets, and
removed audits; the concept owns request identity and shares repository results with the
R3F stage and review form.

**Tech Stack:** React 19, TypeScript 5.8, Vitest 4, Testing Library, React Three Fiber 9,
drei `useAnimations` and `OrbitControls`, Three.js `GLTFLoader`/
`SkeletonUtils.clone`, Web Crypto, browser localStorage, Vite port 3001, and the installed
`playwright` package (standalone Node script, not `@playwright/test`).

**Spec:** `ideas/idle-animation-review-harness/design.md` is approved and controls scope.

**Tracking:** Before the web branch exists, create one `rpg-dnd5e-web` issue linked to
`rpg-project#112` and PR #113; add it to Project 19 with `Team=Assets`, `Feature=Infra`.

## Global Constraints

- The only catalog URL is `/models/synty/characters/manifest.json`. Never embed an
  alternate catalog or substitute a class, model, clip, animation, or pose.
- Mapping entries validate independently. Class key/model are non-empty; `idleClips` are
  ordered non-empty exact strings and exact-string unique. Duplicates invalidate only that
  class with `DUPLICATE_IDLE_CLIP`; no deduplication.
- `TargetTuple` is `readonly [classKey, modelPath, clipName]`; `targetId` is
  `JSON.stringify(tuple)`. Request IDs are monotonic selection instances, never identity.
- `resolveIdleClipName` from `src/components/hex-grid/classCharacterModels.ts` is used only
  to display current production selection; it never selects stage playback.
- `assetIdentity.ts` owns all browser fetch plus SHA-256 work. Catalog and repository consume
  it; no other module calls fetch for manifest/GLB bytes.
- `ExactModelRepository` parses the same hashed GLB bytes once per model URL per page.
  Revalidation and selected-stage loads share it. Missing a requested clip is per-target
  inspection, not a second fetch.
- A persisted tuple absent from the current catalog becomes one
  `CATALOG_TARGET_REMOVED` stale audit, with no GLB request or verdict migration. A renamed
  current tuple is independently unreviewed.
- `revalidating` is transient UI only: no stored review counts, progress is provisional, and
  export is disabled until each persisted current-catalog model settles.
- Current exports have `targets` exactly once in manifest order and `removedTargetAudits`
  separately sorted by target ID. Current counts exclude audits; `removedStaleAuditCount`
  counts audits separately.
- `Fix`/`Reject` require a nonblank actionable note; `Keep` note is optional. Stale reviews
  are reconfirmed only after the new exact target/request reaches ready playing or paused.
- The repository owns parsed source scene geometry/material/texture disposal once at concept
  unmount. A stage stops/uncaches its action/mixer, detaches its SkeletonUtils clone, and
  never disposes clone-shared geometry/material.
- Import `SYNTY_SCALE` from `src/rendering/calibrationConstants.ts`; it is currently `0.75`.
  Game-like fixture: orthographic `[8,10,8]`, zoom `80`, near `0.1`, far `1000`, ambient
  `0.6`, directional `0.8` at `[10,10,5]`, XZ ground `y=0`. It is representative only.
- The asset manifest exists at both
  `/home/kirk/game-dev/rpg-game-assets/harness/models/synty/characters/manifest.json` and
  the synced web public tree. Run `npm run assets:sync` before browser evidence; never add
  ignored `public/models/synty/` GLBs to git.
- Enter Concepts Lab through the current development button `[title="Open Concepts Lab"]`.
  Do not create or claim a `/concepts` route.
- Run `npm run ci-check` before pushing. Never use `--no-verify`; never merge. Every
  GitHub issue/PR/comment body ends `— asset-pipeline agent, on behalf of KirkDiggler`.

## Locked File Structure

| File | Responsibility |
| --- | --- |
| `src/concepts/idle-review/assetIdentity.ts` + test | Sole byte fetch/hash/abort seam. |
| `src/concepts/idle-review/catalog.ts` + test | Manifest loading/validation, canonical targets, catalog diagnostics, display-only production selection. |
| `src/concepts/idle-review/modelRepository.ts` + test | Shared parse-once GLB repository, exact clip inspection, source ownership disposal. |
| `src/concepts/idle-review/reviewState.ts` + test | Typed draft, revalidation partition/classification, progress, deterministic export/serialization/download data. |
| `src/concepts/idle-review/reviewStorage.ts` + test | Validated versioned localStorage envelope and visible storage diagnostic. |
| `src/concepts/idle-review/calibration.ts` + test | Studio and production-derived game-like fixtures. |
| `src/concepts/idle-review/IdleReviewStage.tsx` + test | Clone/action/camera/playback presentation; receives repository asset, never fetches. |
| `src/concepts/idle-review/IdleReviewForm.tsx` + test | Exact-ready verdict controls, diagnostics, current progress, audits/export/reset. |
| `src/concepts/idle-review/IdleAnimationConcept.tsx` + test | Catalog/repository/state orchestration and test dependency construction. |
| `src/concepts/ConceptsView.tsx` + test | `Idle Review` tab registration only. |
| `docs/architecture/components/concepts-route.md` | Current Contents entry for the non-production QA bench. |
| `scripts/verify-idle-review.mjs` | Standalone Playwright evidence flow. |
| `playtest-evidence/idle-review/overview.png`, `game-like.png` | Public evidence screenshots; no GLB files. |

---

### Task 1: Web Issue, Worktree, Shared Asset Identity, And Catalog

**Files:**
- Create: `src/concepts/idle-review/assetIdentity.ts`
- Create: `src/concepts/idle-review/assetIdentity.test.ts`
- Create: `src/concepts/idle-review/catalog.ts`
- Create: `src/concepts/idle-review/catalog.test.ts`

**Interfaces produced:**

```ts
export type AssetIdentityCode = 'FETCH_FAILED' | 'HASH_FAILED' | 'ABORTED';
export type HashedBytesResult =
  | { ok: true; url: string; bytes: ArrayBuffer; sha256: string }
  | { ok: false; url: string; code: AssetIdentityCode; message: string };
export interface AssetIdentityDependencies {
  fetch(input: RequestInfo | URL, init?: RequestInit): Promise<Response>;
  digest(algorithm: AlgorithmIdentifier, data: BufferSource): Promise<ArrayBuffer>;
}
export function sha256Hex(bytes: ArrayBuffer, deps?: Pick<AssetIdentityDependencies, 'digest'>): Promise<string>;
export function fetchHashedBytes(url: string, signal: AbortSignal, deps?: AssetIdentityDependencies): Promise<HashedBytesResult>;

export type CatalogDiagnosticCode = 'MANIFEST_FETCH_FAILED' | 'MANIFEST_HASH_FAILED' | 'MANIFEST_PARSE_FAILED' | 'INVALID_MAPPING_ENTRY' | 'DUPLICATE_IDLE_CLIP';
export interface CatalogDiagnostic { code: CatalogDiagnosticCode; classKey?: string; field?: string; duplicateValue?: string; message: string; }
export type TargetTuple = readonly [string, string, string];
export interface IdleTarget { tuple: TargetTuple; targetId: string; classKey: string; modelPath: string; modelUrl: string; clipName: string; }
export interface ValidatedCatalog { manifestSha256: string; targets: readonly IdleTarget[]; diagnostics: readonly CatalogDiagnostic[]; }
export type CatalogLoadResult = { ok: true; catalog: ValidatedCatalog } | { ok: false; diagnostic: CatalogDiagnostic } | { ok: false; cancelled: true };
export interface CatalogDependencies { fetchBytes: typeof fetchHashedBytes; }
export function loadCatalog(signal: AbortSignal, deps?: CatalogDependencies): Promise<CatalogLoadResult>;
export function validateManifestBytes(bytes: ArrayBuffer, manifestSha256: string): ValidatedCatalog;
export function makeTargetId(tuple: TargetTuple): string;
export function productionSelection(actual: readonly string[]): string | undefined;
```

**Pre-task lifecycle (complete before Task 1 TDD begins; this is not a code task):** Create the
tracking issue, board item, and isolated web worktree. The following commands set the required
`WEB_ISSUE` variable for all task commits.

Run from `/home/kirk/game-dev/rpg-dnd5e-web`:

```bash
git fetch origin --prune
ISSUE_BODY="$(printf '%s\n' '## Goal' '' 'Implement the approved Idle Review Concepts Lab harness tracked by rpg-project#112 and rpg-project PR #113.' '' 'The bench hashes and plays only exact runtime manifest/GLB clips, keeps stale and removed audit truth, and never substitutes a catalog, class, model, clip, animation, or pose.' '' '— asset-pipeline agent, on behalf of KirkDiggler')"
ISSUE_URL="$(gh issue create --repo KirkDiggler/rpg-dnd5e-web --title 'feat: idle animation review harness' --body "$ISSUE_BODY")"
WEB_ISSUE="${ISSUE_URL##*/}"
PROJECT_ID="$(gh project view 19 --owner KirkDiggler --format json | jq -r '.id')"
FIELDS="$(gh project field-list 19 --owner KirkDiggler --format json)"
TEAM_FIELD="$(printf '%s' "$FIELDS" | jq -r '.fields[] | select(.name == "Team") | .id')"
ASSETS="$(printf '%s' "$FIELDS" | jq -r '.fields[] | select(.name == "Team") | .options[] | select(.name == "Assets") | .id')"
FEATURE_FIELD="$(printf '%s' "$FIELDS" | jq -r '.fields[] | select(.name == "Feature") | .id')"
INFRA="$(printf '%s' "$FIELDS" | jq -r '.fields[] | select(.name == "Feature") | .options[] | select(.name == "Infra") | .id')"
STATUS_FIELD="$(printf '%s' "$FIELDS" | jq -r '.fields[] | select(.name == "Status") | .id')"
IN_PROGRESS="$(printf '%s' "$FIELDS" | jq -r '.fields[] | select(.name == "Status") | .options[] | select(.name == "In Progress") | .id')"
ITEM="$(gh project item-add 19 --owner KirkDiggler --url "$ISSUE_URL" --format json | jq -r '.id')"
gh project item-edit --project-id "$PROJECT_ID" --id "$ITEM" --field-id "$TEAM_FIELD" --single-select-option-id "$ASSETS"
gh project item-edit --project-id "$PROJECT_ID" --id "$ITEM" --field-id "$FEATURE_FIELD" --single-select-option-id "$INFRA"
gh project item-edit --project-id "$PROJECT_ID" --id "$ITEM" --field-id "$STATUS_FIELD" --single-select-option-id "$IN_PROGRESS"
git worktree add /tmp/opencode/rpg-dnd5e-web-idle-animation-review -b "feat/${WEB_ISSUE}-idle-animation-review" origin/main
cd /tmp/opencode/rpg-dnd5e-web-idle-animation-review
npm install
npm test -- --run src/concepts/equipment/fixtures.test.ts
```

Expected: Project 19 item is `Team=Assets`, `Feature=Infra`; baseline Vitest exits 0;
the worktree is clean on `feat/${WEB_ISSUE}-idle-animation-review`.

- [ ] **Step 1: Write failing asset/catalog tests**

Create `assetIdentity.test.ts` and `catalog.test.ts`:

```ts
it('returns FETCH_FAILED, HASH_FAILED, and ABORTED distinctly', async () => {
  const abort = new AbortController(); abort.abort();
  await expect(fetchHashedBytes('/x', abort.signal)).resolves.toMatchObject({ ok: false, code: 'ABORTED' });
  const fetchFailed = await fetchHashedBytes('/x', new AbortController().signal, { fetch: vi.fn().mockRejectedValue(new Error('offline')), digest: crypto.subtle.digest.bind(crypto.subtle) });
  expect(fetchFailed).toMatchObject({ ok: false, code: 'FETCH_FAILED' });
  const hashFailed = await fetchHashedBytes('/x', new AbortController().signal, { fetch: vi.fn().mockResolvedValue(new Response(new Uint8Array([1]))), digest: vi.fn().mockRejectedValue(new Error('digest failed')) });
  expect(hashFailed).toMatchObject({ ok: false, code: 'HASH_FAILED' });
});

it('rejects duplicate clips without deriving duplicate UI/progress/export targets', () => {
  const bytes = new TextEncoder().encode(JSON.stringify({ mapping: {
    fighter: { model: 'characters/fighter.glb', idleClips: ['Idle_Relaxed', 'Idle_Relaxed'] },
    monk: { model: 'characters/monk.glb', idleClips: ['Idle_Meditative'] },
  }})).buffer;
  const catalog = validateManifestBytes(bytes, 'manifest');
  expect(catalog.targets.map((target) => target.targetId)).toEqual([JSON.stringify(['monk', 'characters/monk.glb', 'Idle_Meditative'])]);
  expect(catalog.diagnostics).toContainEqual(expect.objectContaining({ code: 'DUPLICATE_IDLE_CLIP', classKey: 'fighter', field: 'idleClips', duplicateValue: 'Idle_Relaxed' }));
});

it('uses the production resolver only for display', () => {
  expect(productionSelection(['Walk', 'Idle_Relaxed'])).toBe('Idle_Relaxed');
});
```

- [ ] **Step 2: Run RED tests**

Run:

```bash
npm test -- --run src/concepts/idle-review/assetIdentity.test.ts src/concepts/idle-review/catalog.test.ts
```

Expected: FAIL with module-not-found for `./assetIdentity` and `./catalog`.

- [ ] **Step 3: Implement asset identity and catalog**

Create `assetIdentity.ts` using this complete error boundary:

```ts
const defaults: AssetIdentityDependencies = { fetch: globalThis.fetch.bind(globalThis), digest: crypto.subtle.digest.bind(crypto.subtle) };
export async function sha256Hex(bytes: ArrayBuffer, deps = defaults): Promise<string> {
  const digest = await deps.digest('SHA-256', bytes);
  return Array.from(new Uint8Array(digest), (value) => value.toString(16).padStart(2, '0')).join('');
}
export async function fetchHashedBytes(url: string, signal: AbortSignal, deps = defaults): Promise<HashedBytesResult> {
  if (signal.aborted) return { ok: false, url, code: 'ABORTED', message: `Aborted ${url}` };
  let response: Response;
  try { response = await deps.fetch(url, { signal }); }
  catch (error) { return { ok: false, url, code: signal.aborted ? 'ABORTED' : 'FETCH_FAILED', message: String(error) }; }
  if (!response.ok) return { ok: false, url, code: 'FETCH_FAILED', message: `${url} returned ${response.status}` };
  let bytes: ArrayBuffer;
  try { bytes = await response.arrayBuffer(); }
  catch (error) { return { ok: false, url, code: signal.aborted ? 'ABORTED' : 'FETCH_FAILED', message: String(error) }; }
  try { return { ok: true, url, bytes, sha256: await sha256Hex(bytes, deps) }; }
  catch (error) { return { ok: false, url, code: 'HASH_FAILED', message: String(error) }; }
}
```

In `catalog.ts`, import `fetchHashedBytes` and the real
`resolveIdleClipName` from `../../components/hex-grid/classCharacterModels`.
`productionSelection(actual)` directly returns `resolveIdleClipName([...actual])`; it is not
injected and no playback code imports it. `loadCatalog` maps `ABORTED` to
`{ ok: false, cancelled: true }`, maps `FETCH_FAILED` to `MANIFEST_FETCH_FAILED` and
`HASH_FAILED` to `MANIFEST_HASH_FAILED`, safely
catches JSON/mapping shape as `MANIFEST_PARSE_FAILED`, and calls pure `validateManifestBytes`. Validation must iterate
mapping entries independently and use `clips.find((clip, index) => clips.indexOf(clip) !== index)`
for exact duplicates. `productionSelection(actual)` is exactly
`resolveIdleClipName([...actual])` and no stage imports it.

- [ ] **Step 4: Run GREEN tests, inspect, stage exact files, and commit**

Run:

```bash
npm test -- --run src/concepts/idle-review/assetIdentity.test.ts src/concepts/idle-review/catalog.test.ts
git status --short
git diff --check
git add src/concepts/idle-review/assetIdentity.ts src/concepts/idle-review/assetIdentity.test.ts src/concepts/idle-review/catalog.ts src/concepts/idle-review/catalog.test.ts
git commit -m "feat: validate idle review runtime catalog (${WEB_ISSUE})"
```

Expected: both focused files pass; status before staging names only the four files; commit succeeds.

### Task 2: Shared Exact Model Repository

**Files:**
- Create: `src/concepts/idle-review/modelRepository.ts`
- Create: `src/concepts/idle-review/modelRepository.test.ts`

**Interfaces produced:**

```ts
export type ModelDiagnosticCode = 'MODEL_FETCH_FAILED' | 'MODEL_HASH_FAILED' | 'MODEL_PARSE_FAILED' | 'MODEL_ABORTED' | 'REQUESTED_CLIP_MISSING' | 'ACTION_FAILED';
import type { GLTF } from 'three/addons/loaders/GLTFLoader.js';
import * as THREE from 'three';
export interface ModelDiagnostic { code: ModelDiagnosticCode; modelUrl: string; message: string; requestedClip?: string; modelSha256?: string; actualClipNames: readonly string[]; }
export interface OwnedModelAsset { modelUrl: string; modelSha256: string; sourceScene: THREE.Group; animations: readonly THREE.AnimationClip[]; actualClipNames: readonly string[]; }
export type ModelAssetResult = { ok: true; asset: OwnedModelAsset } | { ok: false; diagnostic: ModelDiagnostic };
export interface ModelRepositoryDependencies { fetchBytes: typeof fetchHashedBytes; parse(bytes: ArrayBuffer): Promise<GLTF>; disposeSource(scene: THREE.Object3D): void; }
export class ExactModelRepository { get(modelUrl: string): Promise<ModelAssetResult>; dispose(): void; }
export function inspectExactClip(asset: OwnedModelAsset, clipName: string): { ok: true; duration: number } | { ok: false; diagnostic: ModelDiagnostic };
export function createExactModelRepository(overrides?: Partial<ModelRepositoryDependencies>): ExactModelRepository;
```

- [ ] **Step 1: Write failing repository tests**

```ts
it('fetches and parses a model URL once for revalidation and selection', async () => {
  const bytes = new ArrayBuffer(8); const scene = new THREE.Group();
  const fetchBytes = vi.fn().mockResolvedValue({ ok: true, url: '/fighter.glb', bytes, sha256: 'hash' });
  const parse = vi.fn().mockResolvedValue({ scene, animations: [new THREE.AnimationClip('Idle_Relaxed', 2, [])] });
  const repository = new ExactModelRepository({ fetchBytes, parse, disposeSource: vi.fn() });
  await repository.get('/fighter.glb');
  await repository.get('/fighter.glb');
  expect(fetchBytes).toHaveBeenCalledTimes(1); expect(parse).toHaveBeenCalledWith(bytes);
});
it('reports a missing exact clip without reloading the asset', () => {
  const asset = { modelUrl: '/fighter.glb', modelSha256: 'hash', sourceScene: new THREE.Group(), animations: [new THREE.AnimationClip('Idle_Relaxed', 2, [])], actualClipNames: ['Idle_Relaxed'] };
  expect(inspectExactClip(asset, 'Idle_Missing')).toMatchObject({ ok: false, diagnostic: { code: 'REQUESTED_CLIP_MISSING', actualClipNames: ['Idle_Relaxed'] } });
});
it('disposes a late parsed scene exactly once after repository disposal', async () => {
  let resolveParse: (value: GLTF) => void = () => {};
  const disposeSource = vi.fn();
  const repository = new ExactModelRepository({ fetchBytes: vi.fn().mockResolvedValue({ ok: true, url: '/late.glb', bytes: new ArrayBuffer(1), sha256: 'late' }), parse: () => new Promise((resolve) => { resolveParse = resolve; }), disposeSource });
  const pending = repository.get('/late.glb'); repository.dispose();
  resolveParse({ scene: new THREE.Group(), animations: [] } as GLTF);
  await expect(pending).resolves.toMatchObject({ ok: false, diagnostic: { code: 'MODEL_ABORTED' } });
  expect(disposeSource).toHaveBeenCalledTimes(1);
});
```

- [ ] **Step 2: Run RED tests**

Run: `npm test -- --run src/concepts/idle-review/modelRepository.test.ts`
Expected: FAIL with module-not-found for `./modelRepository`.

- [ ] **Step 3: Implement repository ownership**

```ts
export class ExactModelRepository {
  private readonly cache = new Map<string, Promise<ModelAssetResult>>();
  private readonly owned = new Map<string, OwnedModelAsset>();
  private readonly controller = new AbortController();
  private disposed = false;
  constructor(private readonly deps: ModelRepositoryDependencies) {}
  get(modelUrl: string): Promise<ModelAssetResult> {
    if (this.disposed) return Promise.resolve({ ok: false, diagnostic: { code: 'MODEL_ABORTED', modelUrl, message: `Repository disposed before ${modelUrl}`, actualClipNames: [] } });
    const cached = this.cache.get(modelUrl); if (cached) return cached;
    const pending = this.load(modelUrl); this.cache.set(modelUrl, pending); return pending;
  }
  private async load(modelUrl: string): Promise<ModelAssetResult> {
    const fetched = await this.deps.fetchBytes(modelUrl, this.controller.signal);
    if (!fetched.ok) return { ok: false, diagnostic: { code: fetched.code === 'HASH_FAILED' ? 'MODEL_HASH_FAILED' : fetched.code === 'ABORTED' ? 'MODEL_ABORTED' : 'MODEL_FETCH_FAILED', modelUrl, message: fetched.message, actualClipNames: [] } };
    try {
      const gltf = await this.deps.parse(fetched.bytes);
      if (this.disposed) { this.deps.disposeSource(gltf.scene); return { ok: false, diagnostic: { code: 'MODEL_ABORTED', modelUrl, message: `Aborted ${modelUrl}`, actualClipNames: [] } }; }
      const asset = { modelUrl, modelSha256: fetched.sha256, sourceScene: gltf.scene, animations: gltf.animations, actualClipNames: gltf.animations.map((clip) => clip.name) };
      this.owned.set(modelUrl, asset); return { ok: true, asset };
    } catch (error) { return { ok: false, diagnostic: { code: 'MODEL_PARSE_FAILED', modelUrl, message: String(error), actualClipNames: [] } }; }
  }
  dispose() { if (this.disposed) return; this.disposed = true; this.controller.abort(); for (const asset of this.owned.values()) this.deps.disposeSource(asset.sourceScene); this.owned.clear(); this.cache.clear(); }
}
```

`parse` wraps direct `new GLTFLoader().parse(bytes, '', resolve, reject)`. The parser receives
the exact `ArrayBuffer` returned by `fetchHashedBytes`; no object URL is created. `disposeSource`
traverses the repository-owned source scene, disposes each geometry once, gathers materials in
a Set, gathers every material texture property in a Set, then disposes each material and texture
once. Stage clones share these source resources and never dispose geometry/material/texture.

```ts
export function createExactModelRepository(overrides: Partial<ModelRepositoryDependencies> = {}) {
  const disposeSource = (scene: THREE.Object3D) => {
    const geometries = new Set<THREE.BufferGeometry>(); const materials = new Set<THREE.Material>(); const textures = new Set<THREE.Texture>();
    scene.traverse((node) => { if (node instanceof THREE.Mesh) { geometries.add(node.geometry); for (const material of Array.isArray(node.material) ? node.material : [node.material]) materials.add(material); } });
    for (const material of materials) { for (const value of Object.values(material)) if (value instanceof THREE.Texture) textures.add(value); material.dispose(); }
    for (const geometry of geometries) geometry.dispose(); for (const texture of textures) texture.dispose();
  };
  const parse = (bytes: ArrayBuffer) => new Promise<GLTF>((resolve, reject) => new GLTFLoader().parse(bytes, '', resolve, reject));
  return new ExactModelRepository({ fetchBytes: fetchHashedBytes, parse, disposeSource, ...overrides });
}
```

- [ ] **Step 4: Run GREEN tests, inspect, stage exact files, and commit**

Run:

```bash
npm test -- --run src/concepts/idle-review/modelRepository.test.ts
git status --short
git diff --check
git add src/concepts/idle-review/modelRepository.ts src/concepts/idle-review/modelRepository.test.ts
git commit -m "feat: share exact idle review models (${WEB_ISSUE})"
```

Expected: repository tests pass; source disposal occurs once per owned scene.

### Task 3: Typed Draft, Revalidation, Storage, And Export

**Files:**
- Create: `src/concepts/idle-review/reviewState.ts`
- Create: `src/concepts/idle-review/reviewState.test.ts`
- Create: `src/concepts/idle-review/reviewStorage.ts`
- Create: `src/concepts/idle-review/reviewStorage.test.ts`

**Interfaces produced:**

```ts
export type Verdict = 'keep' | 'fix' | 'reject';
export type PersistentOutcome = 'reviewed' | 'unreviewed' | 'blocked' | 'stale';
export type RevalidationPhase = 'idle' | 'revalidating' | 'settled';
export interface StoredReview { targetId: string; tuple: TargetTuple; manifestSha256: string; modelSha256: string; verdict: Verdict; note: string; timestamp: string; }
export interface DraftEnvelope { version: 1; reviews: readonly StoredReview[]; }
export interface StorageDiagnostic { code: 'STORAGE_INVALID'; message: string; }
export interface CurrentTargetOutcome { target: IdleTarget; outcome: PersistentOutcome; modelSha256?: string; actualClipNames: readonly string[]; diagnostic?: ModelDiagnostic; review?: StoredReview; }
export interface RemovedTargetAudit extends StoredReview { outcome: 'stale'; inCurrentCatalog: false; diagnosticCode: 'CATALOG_TARGET_REMOVED'; message: string; }
export interface CurrentProgress { currentTotal: number; currentReviewed: number; currentUnreviewed: number; currentBlocked: number; currentStale: number; keep: number; fix: number; reject: number; removedStaleAuditCount: number; }
export interface ExportTargetRecord { targetId: string; tuple: TargetTuple; inCurrentCatalog: true; assetPath: string; class: string; clip: string; outcome: PersistentOutcome; manifestSha256: string; modelSha256?: string; persistedIdentity?: { manifestSha256: string; modelSha256: string }; diagnostic?: ModelDiagnostic; verdict?: Verdict; note?: string; timestamp?: string; actualClipNames: readonly string[]; }
export interface IdleReviewExport { version: 1; exportedAt: string; manifestSha256: string; targets: readonly ExportTargetRecord[]; removedTargetAudits: readonly RemovedTargetAudit[]; catalogDiagnostics: readonly CatalogDiagnostic[]; summary: CurrentProgress; }
export function partitionStoredReviews(stored: readonly StoredReview[], targets: readonly IdleTarget[]): { current: readonly StoredReview[]; removed: readonly RemovedTargetAudit[] };
export function classifyCurrentTargets(targets: readonly IdleTarget[], stored: readonly StoredReview[], manifestSha256: string, models: ReadonlyMap<string, ModelAssetResult>, stageDiagnostics?: ReadonlyMap<string, ModelDiagnostic>): readonly CurrentTargetOutcome[];
export function validateVerdict(verdict: Verdict, note: string): string | undefined;
export function computeProgress(outcomes: readonly CurrentTargetOutcome[], removed: readonly RemovedTargetAudit[]): CurrentProgress;
export function saveCurrentReview(target: IdleTarget, manifestSha256: string, asset: OwnedModelAsset, verdict: Verdict, note: string, timestamp: string): StoredReview;
export function buildExport(input: { manifestSha256: string; outcomes: readonly CurrentTargetOutcome[]; removedTargetAudits: readonly RemovedTargetAudit[]; catalogDiagnostics: readonly CatalogDiagnostic[]; exportedAt: string }): IdleReviewExport;
export function serializeExport(value: IdleReviewExport): string;
export function triggerJsonDownload(filename: string, text: string): void;
export const REVIEW_STORAGE_KEY: 'idle-review/v1';
export function loadDraft(storage: Storage): { reviews: readonly StoredReview[]; diagnostic?: StorageDiagnostic };
export function saveDraft(storage: Storage, reviews: readonly StoredReview[]): void;
export interface BuildExportInput { manifestSha256: string; outcomes: readonly CurrentTargetOutcome[]; removedTargetAudits: readonly RemovedTargetAudit[]; catalogDiagnostics: readonly CatalogDiagnostic[]; exportedAt: string; }
```

- [ ] **Step 1: Write failing state/storage/export tests**

```ts
it('classifies removed records without model fetch and replacement as unreviewed', () => {
  const oldReview = { targetId: JSON.stringify(['fighter', 'characters/old.glb', 'Idle_Old']), tuple: ['fighter', 'characters/old.glb', 'Idle_Old'] as const, manifestSha256: 'old', modelSha256: 'old-model', verdict: 'fix' as const, note: 'replace motion', timestamp: '2026-07-22T00:00:00.000Z' };
  const replacementTarget = { tuple: ['fighter', 'characters/new.glb', 'Idle_New'] as const, targetId: JSON.stringify(['fighter', 'characters/new.glb', 'Idle_New']), classKey: 'fighter', modelPath: 'characters/new.glb', modelUrl: '/models/synty/characters/new.glb', clipName: 'Idle_New' };
  const partition = partitionStoredReviews([oldReview], [replacementTarget]);
  expect(partition.current).toEqual([]);
  expect(partition.removed).toContainEqual(expect.objectContaining({ inCurrentCatalog: false, diagnosticCode: 'CATALOG_TARGET_REMOVED', verdict: oldReview.verdict }));
  expect(classifyCurrentTargets([replacementTarget], [], 'new-manifest', new Map()).map((entry) => entry.outcome)).toEqual(['unreviewed']);
});
it('restores only matching hashes, makes changed hashes stale, failures blocked, and missing clips target-blocked', () => {
  const target = { tuple: ['fighter', 'characters/fighter.glb', 'Idle_Relaxed'] as const, targetId: JSON.stringify(['fighter', 'characters/fighter.glb', 'Idle_Relaxed']), classKey: 'fighter', modelPath: 'characters/fighter.glb', modelUrl: '/models/synty/characters/fighter.glb', clipName: 'Idle_Relaxed' };
  const review = { targetId: target.targetId, tuple: target.tuple, manifestSha256: 'manifest', modelSha256: 'model', verdict: 'keep' as const, note: '', timestamp: '2026-07-22T00:00:00.000Z' };
  const matchingAsset = { ok: true as const, asset: { modelUrl: target.modelUrl, modelSha256: 'model', sourceScene: new THREE.Group(), animations: [new THREE.AnimationClip('Idle_Relaxed', 1, [])], actualClipNames: ['Idle_Relaxed'] } };
  const failedAsset = { ok: false as const, diagnostic: { code: 'MODEL_FETCH_FAILED' as const, modelUrl: target.modelUrl, message: 'offline', actualClipNames: [] } };
  expect(classifyCurrentTargets([target], [review], 'manifest', new Map([[target.modelUrl, matchingAsset]]) )[0].outcome).toBe('reviewed');
  expect(classifyCurrentTargets([target], [review], 'new-manifest', new Map([[target.modelUrl, matchingAsset]]) )[0].outcome).toBe('stale');
  expect(classifyCurrentTargets([target], [review], 'manifest', new Map([[target.modelUrl, failedAsset]]) )[0].outcome).toBe('blocked');
});
it('rejects malformed stored records explicitly', () => {
  localStorage.setItem(REVIEW_STORAGE_KEY, JSON.stringify({ version: 1, reviews: [{ targetId: 3 }] }));
  expect(loadDraft(localStorage)).toMatchObject({ reviews: [], diagnostic: { code: 'STORAGE_INVALID' } });
});
it('requires actionable notes only for fix and reject verdicts', () => {
  expect(validateVerdict('keep', '')).toBeUndefined();
  expect(validateVerdict('fix', '')).toBe('Fix requires an actionable note');
  expect(validateVerdict('reject', '')).toBe('Reject requires an actionable note');
});
it('blocks a successful model when its requested clip is absent', () => {
  const target = { tuple: ['fighter', 'characters/fighter.glb', 'Idle_Relaxed'] as const, targetId: JSON.stringify(['fighter', 'characters/fighter.glb', 'Idle_Relaxed']), classKey: 'fighter', modelPath: 'characters/fighter.glb', modelUrl: '/models/synty/characters/fighter.glb', clipName: 'Idle_Relaxed' };
  const asset = { ok: true as const, asset: { modelUrl: target.modelUrl, modelSha256: 'model', sourceScene: new THREE.Group(), animations: [new THREE.AnimationClip('Idle_Drinking', 1, [])], actualClipNames: ['Idle_Drinking'] } };
  expect(classifyCurrentTargets([target], [], 'manifest', new Map([[target.modelUrl, asset]]))[0]).toMatchObject({ outcome: 'blocked', diagnostic: { code: 'REQUESTED_CLIP_MISSING' } });
});
it('exports a matching ACTION_FAILED stage target as blocked', () => {
  const target = { tuple: ['fighter', 'characters/fighter.glb', 'Idle_Relaxed'] as const, targetId: JSON.stringify(['fighter', 'characters/fighter.glb', 'Idle_Relaxed']), classKey: 'fighter', modelPath: 'characters/fighter.glb', modelUrl: '/models/synty/characters/fighter.glb', clipName: 'Idle_Relaxed' };
  const asset = { ok: true as const, asset: { modelUrl: target.modelUrl, modelSha256: 'model', sourceScene: new THREE.Group(), animations: [new THREE.AnimationClip('Idle_Relaxed', 1, [])], actualClipNames: ['Idle_Relaxed'] } };
  const diagnostic: ModelDiagnostic = { code: 'ACTION_FAILED', modelUrl: target.modelUrl, modelSha256: 'model', requestedClip: target.clipName, actualClipNames: ['Idle_Relaxed'], message: 'play failed' };
  expect(classifyCurrentTargets([target], [], 'manifest', new Map([[target.modelUrl, asset]]), new Map([[target.targetId, diagnostic]]))[0]).toMatchObject({ outcome: 'blocked', diagnostic });
});
it('exports manifest order deterministically and sorts removed audits', () => {
  const firstTarget = { tuple: ['fighter', 'characters/fighter.glb', 'Idle_Relaxed'] as const, targetId: 'target-a', classKey: 'fighter', modelPath: 'characters/fighter.glb', modelUrl: '/models/synty/characters/fighter.glb', clipName: 'Idle_Relaxed' };
  const secondTarget = { tuple: ['monk', 'characters/monk.glb', 'Idle_Meditative'] as const, targetId: 'target-b', classKey: 'monk', modelPath: 'characters/monk.glb', modelUrl: '/models/synty/characters/monk.glb', clipName: 'Idle_Meditative' };
  const review: StoredReview = { targetId: firstTarget.targetId, tuple: firstTarget.tuple, manifestSha256: 'reviewed-manifest', modelSha256: 'reviewed-model', verdict: 'keep', note: '', timestamp: '2026-07-22T00:00:00.000Z' };
  const outcomes: readonly CurrentTargetOutcome[] = [{ target: firstTarget, outcome: 'reviewed', modelSha256: 'current-model', actualClipNames: ['Idle_Relaxed'], review }, { target: secondTarget, outcome: 'unreviewed', modelSha256: 'second-model', actualClipNames: ['Idle_Meditative'] }];
  const audits: readonly RemovedTargetAudit[] = [{ ...review, targetId: 'removed-z', outcome: 'stale', inCurrentCatalog: false, diagnosticCode: 'CATALOG_TARGET_REMOVED', message: 'removed z' }, { ...review, targetId: 'removed-a', outcome: 'stale', inCurrentCatalog: false, diagnosticCode: 'CATALOG_TARGET_REMOVED', message: 'removed a' }];
  const exported = buildExport({ manifestSha256: 'current-manifest', outcomes, removedTargetAudits: audits, catalogDiagnostics: [], exportedAt: '2026-07-22T12:00:00.000Z' });
  expect(exported.targets.map((target) => target.targetId)).toEqual(['target-a', 'target-b']);
  expect(exported.removedTargetAudits.map((audit) => audit.targetId)).toEqual(['removed-a', 'removed-z']);
  expect(exported.targets[0]).toMatchObject({ manifestSha256: 'current-manifest', modelSha256: 'current-model', verdict: 'keep' });
  expect(exported.targets[0].persistedIdentity).toBeUndefined();
  expect(exported.summary.currentTotal).toBe(2); expect(exported.summary.removedStaleAuditCount).toBe(2);
  expect(exported.exportedAt).toBe('2026-07-22T12:00:00.000Z');
  expect(serializeExport(exported)).toBe(serializeExport(exported));
});
```

- [ ] **Step 2: Run RED tests**

Run: `npm test -- --run src/concepts/idle-review/reviewState.test.ts src/concepts/idle-review/reviewStorage.test.ts`
Expected: FAIL with module-not-found for `./reviewState` and `./reviewStorage`.

- [ ] **Step 3: Implement classification and serialization**

`partitionStoredReviews` compares exact target IDs before repository calls. For every current
target, `classifyCurrentTargets` applies this order: a matching `ACTION_FAILED` stage diagnostic
=> blocked; no stored review => unreviewed; failed repository result => blocked;
`inspectExactClip` failure => blocked; unequal manifest/model hash => stale; equal hashes =>
reviewed. `validateVerdict` returns `Fix requires an actionable
note`/`Reject requires an actionable note` for blank required notes. A reconfirmation creates
a new StoredReview with the active target's current hashes and injected timestamp.

`loadDraft` checks `version === 1`, array shape, each `targetId` string, tuple of exactly
three strings, `targetId === JSON.stringify(tuple)`, SHA strings, verdict union, note string,
and timestamp string. Any invalid envelope or record returns `{ reviews: [], diagnostic }`.

`buildExport` maps every current outcome once in the catalog order supplied by
`classifyCurrentTargets`, using approved `manifestSha256` and available `modelSha256` for the
current identity. A stale current target may add `persistedIdentity` with the stored hashes only
for stale diagnostics/audit support; it never replaces the approved current hash keys. It sorts
only removed audits by `targetId`; `computeProgress` receives only current outcomes plus audit count.

```ts
export function computeProgress(outcomes: readonly CurrentTargetOutcome[], audits: readonly RemovedTargetAudit[]): CurrentProgress {
  const count = (outcome: PersistentOutcome) => outcomes.filter((entry) => entry.outcome === outcome).length;
  const reviewed = outcomes.filter((entry) => entry.outcome === 'reviewed');
  return { currentTotal: outcomes.length, currentReviewed: count('reviewed'), currentUnreviewed: count('unreviewed'), currentBlocked: count('blocked'), currentStale: count('stale'), keep: reviewed.filter((entry) => entry.review?.verdict === 'keep').length, fix: reviewed.filter((entry) => entry.review?.verdict === 'fix').length, reject: reviewed.filter((entry) => entry.review?.verdict === 'reject').length, removedStaleAuditCount: audits.length };
}
export function buildExport(input: BuildExportInput): IdleReviewExport {
  const targets = input.outcomes.map((entry) => ({ targetId: entry.target.targetId, tuple: entry.target.tuple, inCurrentCatalog: true, assetPath: entry.target.modelPath, class: entry.target.classKey, clip: entry.target.clipName, outcome: entry.outcome, manifestSha256: input.manifestSha256, modelSha256: entry.modelSha256, persistedIdentity: entry.outcome === 'stale' && entry.review ? { manifestSha256: entry.review.manifestSha256, modelSha256: entry.review.modelSha256 } : undefined, diagnostic: entry.diagnostic, verdict: entry.review?.verdict, note: entry.review?.note, timestamp: entry.review?.timestamp, actualClipNames: entry.actualClipNames ?? [] }));
  return { version: 1, exportedAt: input.exportedAt, manifestSha256: input.manifestSha256, targets, removedTargetAudits: [...input.removedTargetAudits].sort((a, b) => a.targetId.localeCompare(b.targetId)), catalogDiagnostics: input.catalogDiagnostics, summary: computeProgress(input.outcomes, input.removedTargetAudits) };
}
```

```ts
export function serializeExport(value: IdleReviewExport): string {
  return `${JSON.stringify(value, null, 2)}\n`;
}
export function triggerJsonDownload(filename: string, text: string): void {
  const blob = new Blob([text], { type: 'application/json' });
  const url = URL.createObjectURL(blob); const anchor = document.createElement('a');
  anchor.href = url; anchor.download = filename; document.body.append(anchor); anchor.click();
  setTimeout(() => { anchor.remove(); URL.revokeObjectURL(url); }, 1000);
}
```

- [ ] **Step 4: Run GREEN tests, inspect, stage exact files, and commit**

Run:

```bash
npm test -- --run src/concepts/idle-review/reviewState.test.ts src/concepts/idle-review/reviewStorage.test.ts
git status --short
git diff --check
git add src/concepts/idle-review/reviewState.ts src/concepts/idle-review/reviewState.test.ts src/concepts/idle-review/reviewStorage.ts src/concepts/idle-review/reviewStorage.test.ts
git commit -m "feat: preserve idle review audit state (${WEB_ISSUE})"
```

Expected: tests pass; export order is manifest target order plus target-ID-sorted audits.

### Task 4: Calibration And Safe R3F Stage

**Files:**
- Create: `src/concepts/idle-review/calibration.ts`
- Create: `src/concepts/idle-review/calibration.test.ts`
- Create: `src/concepts/idle-review/IdleReviewStage.tsx`
- Create: `src/concepts/idle-review/IdleReviewStage.test.tsx`

**Interfaces produced:**

```ts
export type VisualContext = 'studio' | 'game-like';
export interface CameraPreset { id: 'front' | 'three-quarter' | 'side' | 'top'; position: [number, number, number]; }
export interface IdleReviewCalibration { id: VisualContext; scale: number; camera: { position: [number, number, number]; zoom: number; near: number; far: number }; groundY: number; ambientIntensity: number; directional: { position: [number, number, number]; intensity: number }; presets: readonly CameraPreset[]; }
export type StageEvent = { kind: 'ready'; targetId: string; requestId: number; modelSha256: string; actualClipNames: readonly string[]; duration: number; status: 'playing' | 'paused' } | { kind: 'error'; targetId: string; requestId: number; diagnostic: ModelDiagnostic };
export interface IdleReviewStageProps { target: IdleTarget; requestId: number; assetResult: ModelAssetResult | undefined; context: VisualContext; onContextChange(context: VisualContext): void; onEvent(event: StageEvent): void; }
export interface IdleReviewSceneProps { target: IdleTarget; requestId: number; assetResult: ModelAssetResult | undefined; context: VisualContext; speed: 0.25 | 0.5 | 1; paused: boolean; restartGeneration: number; presetId: CameraPreset['id']; animationTimeRef: React.RefObject<HTMLOutputElement | null>; onEvent(event: StageEvent): void; }
export function IdleReviewScene(props: IdleReviewSceneProps): React.JSX.Element;
```

- [ ] **Step 1: Write failing calibration, outer-stage, and inner-scene tests**

```tsx
import * as THREE from 'three';
import { act, fireEvent, render, screen } from '@testing-library/react';
import { expect, it, vi } from 'vitest';
it('uses production game-like calibration and never duplicates SYNTY_SCALE', () => {
  expect(GAME_LIKE_CALIBRATION).toMatchObject({ scale: SYNTY_SCALE, camera: { position: [8, 10, 8], zoom: 80, near: 0.1, far: 1000 }, groundY: 0, ambientIntensity: 0.6, directional: { position: [10, 10, 5], intensity: 0.8 } });
});
it('keeps outer controls and output in ordinary DOM while Canvas receives IdleReviewScene', () => {
  render(<IdleReviewStage target={target} requestId={3} assetResult={{ ok: true, asset }} context="studio" onContextChange={vi.fn()} onEvent={vi.fn()} />);
  expect(screen.getByRole('button', { name: 'Game-like' })).toBeInTheDocument();
  expect(screen.getByTestId('animation-time')).toBeInstanceOf(HTMLOutputElement);
  expect(canvasChildren).toEqual([IdleReviewScene]);
});
it('inner scene preserves time for speed/preset, resets for context, and writes the DOM output', () => {
  const output = document.createElement('output'); const ref = { current: output };
  const { rerender } = render(<Canvas><IdleReviewScene target={target} requestId={3} assetResult={{ ok: true, asset }} context="studio" speed={1} paused={false} restartGeneration={0} presetId="front" animationTimeRef={ref} onEvent={onEvent} /></Canvas>);
  fakeAction.time = 1.2; act(() => capturedFrame({ invalidate })); expect(output.value).toBe('1.200');
  rerender(<Canvas><IdleReviewScene target={target} requestId={3} assetResult={{ ok: true, asset }} context="studio" speed={0.5} paused={false} restartGeneration={0} presetId="side" animationTimeRef={ref} onEvent={onEvent} /></Canvas>);
  expect(fakeAction.time).toBe(1.2);
  rerender(<Canvas><IdleReviewScene target={target} requestId={3} assetResult={{ ok: true, asset }} context="game-like" speed={0.5} paused={false} restartGeneration={0} presetId="side" animationTimeRef={ref} onEvent={onEvent} /></Canvas>);
  expect(fakeAction.time).toBe(0); fakeAction.time = 1.4; act(() => capturedFrame({ invalidate })); expect(output.value).toBe('1.400');
});
it('emits ACTION_FAILED for an absent action or a throwing play and never emits ready', () => {
  mockActions({}); renderInner(); expect(onEvent).toHaveBeenLastCalledWith(expect.objectContaining({ kind: 'error', targetId: target.targetId, requestId: 3, diagnostic: expect.objectContaining({ code: 'ACTION_FAILED', modelUrl: target.modelUrl, requestedClip: target.clipName, modelSha256: asset.modelSha256, actualClipNames: asset.actualClipNames }) }));
  mockActions({ [target.clipName]: { ...fakeAction, play: () => { throw new Error('play failed'); } } }); renderInner(); expect(onEvent).not.toHaveBeenCalledWith(expect.objectContaining({ kind: 'ready' }));
});
it('emits synchronized paused and playing events for playback and context controls', () => {
  const onEvent = vi.fn(); render(<IdleReviewStage target={target} requestId={3} assetResult={{ ok: true, asset }} context="studio" onContextChange={vi.fn()} onEvent={onEvent} />);
  fireEvent.click(screen.getByRole('button', { name: 'Pause' })); expect(onEvent).toHaveBeenLastCalledWith(expect.objectContaining({ kind: 'ready', targetId: target.targetId, requestId: 3, status: 'paused' }));
  fireEvent.click(screen.getByRole('button', { name: 'Play' })); expect(onEvent).toHaveBeenLastCalledWith(expect.objectContaining({ kind: 'ready', targetId: target.targetId, requestId: 3, status: 'playing' }));
  fireEvent.click(screen.getByRole('button', { name: 'Pause' }));
  fireEvent.click(screen.getByRole('button', { name: 'Game-like' })); expect(onEvent).toHaveBeenLastCalledWith(expect.objectContaining({ kind: 'ready', targetId: target.targetId, requestId: 3, status: 'playing' }));
});
```

Mock `Canvas` as a component that records its direct child type and renders that child, not a
fragment that bypasses the parent/child relationship. In separate inner-scene tests, mock R3F
hooks, `useAnimations`, and `OrbitControls`; capture `useFrame` in `capturedFrame`, supply
`fakeAction`, `invalidate`, and `mockActions`. This verifies `IdleReviewScene` is the Canvas
descendant and the only hook consumer. Task 5 owns orchestration assertions for `ACTION_FAILED`.

- [ ] **Step 2: Run RED tests**

Run: `npm test -- --run src/concepts/idle-review/calibration.test.ts src/concepts/idle-review/IdleReviewStage.test.tsx`
Expected: FAIL with missing modules/exports and absent `ACTION_FAILED` handling.

- [ ] **Step 3: Implement fixtures, outer DOM stage, and inner R3F scene**

`calibration.ts` imports `SYNTY_SCALE` and exports literal studio/game-like fixtures and front
`[0,2,6]`, three-quarter `[6,4,6]`, side `[6,2,0]`, top `[0,10,0]` presets.

```tsx
export function IdleReviewStage({ target, requestId, assetResult, context, onContextChange, onEvent }: IdleReviewStageProps) {
  const animationTimeRef = useRef<HTMLOutputElement>(null); const [speed, setSpeed] = useState<0.25 | 0.5 | 1>(1);
  const [paused, setPaused] = useState(false); const [restartGeneration, setRestartGeneration] = useState(0); const [presetId, setPresetId] = useState<CameraPreset['id']>('front'); const [diagnostic, setDiagnostic] = useState<ModelDiagnostic>();
  const receive = (event: StageEvent) => { setDiagnostic(event.kind === 'error' ? event.diagnostic : undefined); onEvent(event); };
  const selectContext = (next: VisualContext) => { setPaused(false); onContextChange(next); };
  return <section><div><button onClick={() => selectContext('studio')}>Studio</button><button onClick={() => selectContext('game-like')}>Game-like</button><button onClick={() => setPaused(false)}>Play</button><button onClick={() => setPaused(true)}>Pause</button><button onClick={() => { setPaused(false); setRestartGeneration((value) => value + 1); }}>Restart</button>{([0.25, 0.5, 1] as const).map((value) => <button key={value} onClick={() => setSpeed(value)}>{value}x</button>)}{(['front', 'three-quarter', 'side', 'top'] as const).map((id) => <button key={id} onClick={() => setPresetId(id)}>{id === 'three-quarter' ? 'Three-quarter' : id[0].toUpperCase() + id.slice(1)}</button>)}</div><output data-testid="animation-time" ref={animationTimeRef}>0.000</output>{diagnostic && <p data-testid="stage-diagnostic">{diagnostic.code}: {diagnostic.message}</p>}<ErrorBoundary fallback={<p data-testid="stage-diagnostic">Canvas error</p>}><Canvas><IdleReviewScene target={target} requestId={requestId} assetResult={assetResult} context={context} speed={speed} paused={paused} restartGeneration={restartGeneration} presetId={presetId} animationTimeRef={animationTimeRef} onEvent={receive} /></Canvas></ErrorBoundary></section>;
}
export function IdleReviewScene({ target, requestId, assetResult, context, speed, paused, restartGeneration, presetId, animationTimeRef, onEvent }: IdleReviewSceneProps) {
  const { invalidate, camera } = useThree(); const controls = useRef<OrbitControlsImpl | null>(null); const emit = useEffectEvent(onEvent);
  const asset = assetResult?.ok ? assetResult.asset : undefined; const clone = useMemo(() => asset ? cloneSkeleton(asset.sourceScene) : new THREE.Group(), [asset]); const { actions, mixer } = useAnimations(asset?.animations ?? [], clone); const inspected = useMemo(() => asset ? inspectExactClip(asset, target.clipName) : undefined, [asset, target.clipName]); const action = asset ? actions[target.clipName] : undefined;
  useEffect(() => { const emitActionFailure = (message: string) => emit({ kind: 'error', targetId: target.targetId, requestId, diagnostic: { code: 'ACTION_FAILED', modelUrl: asset?.modelUrl ?? target.modelUrl, requestedClip: target.clipName, modelSha256: asset?.modelSha256, actualClipNames: asset?.actualClipNames ?? [], message } }); if (!asset) { if (assetResult && !assetResult.ok) emit({ kind: 'error', targetId: target.targetId, requestId, diagnostic: assetResult.diagnostic }); return; } if (!inspected) return; if (!inspected.ok) { emit({ kind: 'error', targetId: target.targetId, requestId, diagnostic: inspected.diagnostic }); return; } if (!action) { emitActionFailure(`No action created for ${target.clipName}`); return; } try { action.reset().setLoop(THREE.LoopRepeat, Infinity).play(); action.time = 0; emit({ kind: 'ready', targetId: target.targetId, requestId, modelSha256: asset.modelSha256, actualClipNames: asset.actualClipNames, duration: inspected.duration, status: 'playing' }); } catch (error) { emitActionFailure(String(error)); return; } return () => { action.stop(); mixer.uncacheAction(action.getClip(), clone); mixer.uncacheRoot(clone); clone.removeFromParent(); }; }, [asset, assetResult, action, clone, inspected, mixer, requestId, target.targetId, target.clipName]);
  useEffect(() => { if (action) action.setEffectiveTimeScale(speed); }, [action, speed]); useEffect(() => { if (!asset || !action) return; action.paused = paused; emit({ kind: 'ready', targetId: target.targetId, requestId, modelSha256: asset.modelSha256, actualClipNames: asset.actualClipNames, duration: action.getClip().duration, status: paused ? 'paused' : 'playing' }); }, [asset, action, paused, requestId, target.targetId]);
  useEffect(() => { if (!asset || !action) return; action.time = 0; action.paused = false; action.play(); applyContextCamera(context, camera, controls.current); emit({ kind: 'ready', targetId: target.targetId, requestId, modelSha256: asset.modelSha256, actualClipNames: asset.actualClipNames, duration: action.getClip().duration, status: 'playing' }); invalidate(); }, [asset, action, context, camera, invalidate, requestId, target.targetId]);
  useEffect(() => { if (!action) return; action.time = 0; action.paused = false; action.play(); invalidate(); }, [action, restartGeneration, invalidate]);
  useEffect(() => { applyPreset(presetId, camera, controls.current); }, [presetId, camera]); useFrame(() => { if (action && !action.paused && animationTimeRef.current) { animationTimeRef.current.value = action.time.toFixed(3); invalidate(); } });
  return <><ambientLight /><directionalLight /><mesh rotation-x={-Math.PI / 2} /><primitive object={clone} /><OrbitControls ref={controls} /></>;
}
```

No DOM element is rendered inside `<Canvas>`: the outer component owns all DOM controls, status
diagnostics, and output. The inner scene is the only caller of `useThree`, `useFrame`, and
`useAnimations`, renders all R3F objects, and sends exact ready/error events. Missing exact clips
retain `REQUESTED_CLIP_MISSING`; an absent created action or exception from reset/setLoop/play
emits `ACTION_FAILED` with URL/hash/requested clip in the message/target and actual names.
Because `inspected` is memoized by asset and exact clip, speed, preset, and paused rerenders do
not rerun setup, cleanup, or reset the action.

- [ ] **Step 4: Run GREEN tests, inspect, stage exact files, and commit**

Run:

```bash
npm test -- --run src/concepts/idle-review/calibration.test.ts src/concepts/idle-review/IdleReviewStage.test.tsx
git status --short
git diff --check
git add src/concepts/idle-review/calibration.ts src/concepts/idle-review/calibration.test.ts src/concepts/idle-review/IdleReviewStage.tsx src/concepts/idle-review/IdleReviewStage.test.tsx
git commit -m "feat: render exact idle review stage (${WEB_ISSUE})"
```

Expected: focused tests pass; clone cleanup never disposes shared source resources.

### Task 5: Form, Concept Orchestration, Tab, And Concept Documentation

**Files:**
- Create: `src/concepts/idle-review/IdleReviewForm.tsx` + test
- Create: `src/concepts/idle-review/IdleAnimationConcept.tsx` + test
- Modify: `src/concepts/ConceptsView.tsx`
- Create: `src/concepts/ConceptsView.test.tsx`
- Modify: `docs/architecture/components/concepts-route.md`

**Interfaces consumed:** all Task 1–4 interfaces.

- [ ] **Step 1: Write failing form/concept/tab tests**

```tsx
const readyTarget = { tuple: ['fighter', 'characters/fighter.glb', 'Idle_Relaxed'] as const, targetId: JSON.stringify(['fighter', 'characters/fighter.glb', 'Idle_Relaxed']), classKey: 'fighter', modelPath: 'characters/fighter.glb', modelUrl: '/models/synty/characters/fighter.glb', clipName: 'Idle_Relaxed' };
const monkTarget = { tuple: ['monk', 'characters/monk.glb', 'Idle_Meditative'] as const, targetId: JSON.stringify(['monk', 'characters/monk.glb', 'Idle_Meditative']), classKey: 'monk', modelPath: 'characters/monk.glb', modelUrl: '/models/synty/characters/monk.glb', clipName: 'Idle_Meditative' };
const readyCatalog = { manifestSha256: 'manifest', targets: [readyTarget, monkTarget], diagnostics: [] };
const readyAsset = { modelUrl: readyTarget.modelUrl, modelSha256: 'hash', sourceScene: new THREE.Group(), animations: [new THREE.AnimationClip('Idle_Relaxed', 1, [])], actualClipNames: ['Idle_Relaxed'] };
const readyRepository = { get: vi.fn().mockResolvedValue({ ok: true, asset: readyAsset }), dispose: vi.fn() } as unknown as ExactModelRepository;
const readyDependencies: IdleReviewDependencies = { loadCatalog: vi.fn().mockResolvedValue({ ok: true, catalog: readyCatalog }), createRepository: () => readyRepository, storage: localStorage, now: () => '2026-07-22T00:00:00.000Z' };
it('disables verdict and export until the exact active request is ready and revalidation settles', () => {
  render(<IdleReviewForm activeTarget={readyTarget} activeRequestId={2} stageEvent={{ kind: 'ready', targetId: readyTarget.targetId, requestId: 1, modelSha256: 'hash', actualClipNames: [], duration: 1, status: 'playing' }} exportDisabledReason="Revalidating saved asset identities before export" onSave={vi.fn()} onReset={vi.fn()} onExport={vi.fn()} currentProgress={{ currentTotal: 1, currentReviewed: 0, currentUnreviewed: 1, currentBlocked: 0, currentStale: 0, keep: 0, fix: 0, reject: 0, removedStaleAuditCount: 0 }} removedTargetAudits={[]} />);
  expect(screen.getByRole('button', { name: 'Keep' })).toBeDisabled();
  expect(screen.getByRole('button', { name: 'Export JSON' })).toBeDisabled();
});
it('runs one repository through revalidation and stage selection', async () => {
  render(<IdleAnimationConcept dependencies={readyDependencies} />);
  await waitFor(() => expect(readyRepository.get).toHaveBeenCalledTimes(1));
  await userEvent.selectOptions(screen.getByRole('combobox', { name: 'Clip' }), 'Idle_Relaxed');
  expect(readyRepository.get).toHaveBeenCalledTimes(1);
});
it('rejects stale A-to-B-to-A stage events by request ID', () => {
  const events: StageEvent[] = [];
  let currentOnEvent: ((event: StageEvent) => void) | undefined;
  vi.mocked(IdleReviewStage).mockImplementation(({ onEvent }) => { currentOnEvent = onEvent; return <button onClick={() => events.forEach((event) => currentOnEvent?.(event))}>emit</button>; });
  render(<IdleAnimationConcept dependencies={readyDependencies} />);
  fireEvent.change(screen.getByRole('combobox', { name: 'Class' }), { target: { value: 'monk' } });
  fireEvent.change(screen.getByRole('combobox', { name: 'Class' }), { target: { value: 'fighter' } });
  events.push({ kind: 'ready', targetId: readyTarget.targetId, requestId: 1, modelSha256: 'hash', actualClipNames: ['Idle_Relaxed'], duration: 1, status: 'playing' });
  fireEvent.click(screen.getByRole('button', { name: 'emit' }));
  expect(screen.getByRole('button', { name: 'Keep' })).toBeDisabled();
  events.push({ kind: 'ready', targetId: readyTarget.targetId, requestId: 3, modelSha256: 'hash', actualClipNames: ['Idle_Relaxed'], duration: 1, status: 'playing' });
  fireEvent.click(screen.getByRole('button', { name: 'emit' }));
  expect(screen.getByRole('button', { name: 'Keep' })).toBeEnabled();
  expect(screen.getByRole('status')).toHaveTextContent('Ready: fighter / Idle_Relaxed (playing)');
  expect(screen.getByRole('status')).toHaveTextContent('currentTotal');
});
it('ignores stale ACTION_FAILED but blocks the accepted matching action failure', () => {
  const events: StageEvent[] = []; let currentOnEvent: ((event: StageEvent) => void) | undefined;
  vi.mocked(IdleReviewStage).mockImplementation(({ onEvent }) => { currentOnEvent = onEvent; return <button onClick={() => events.forEach((event) => currentOnEvent?.(event))}>emit action error</button>; });
  render(<IdleAnimationConcept dependencies={readyDependencies} />);
  fireEvent.change(screen.getByRole('combobox', { name: 'Class' }), { target: { value: 'monk' } }); fireEvent.change(screen.getByRole('combobox', { name: 'Class' }), { target: { value: 'fighter' } });
  const error = (requestId: number): StageEvent => ({ kind: 'error', targetId: readyTarget.targetId, requestId, diagnostic: { code: 'ACTION_FAILED', modelUrl: readyTarget.modelUrl, requestedClip: readyTarget.clipName, modelSha256: 'hash', actualClipNames: ['Idle_Relaxed'], message: 'play failed' } });
  events.push(error(1)); fireEvent.click(screen.getByRole('button', { name: 'emit action error' })); expect(screen.getByRole('status')).not.toHaveTextContent('ACTION_FAILED');
  events.push(error(3)); fireEvent.click(screen.getByRole('button', { name: 'emit action error' })); expect(screen.getByRole('status')).toHaveTextContent('ACTION_FAILED'); expect(screen.getByRole('status')).toHaveTextContent('currentBlocked'); expect(screen.getByRole('button', { name: 'Keep' })).toBeDisabled(); expect(screen.getByRole('button', { name: 'Save review' })).toBeDisabled();
});
```

The initial render is Fighter/request 1; the two changes are Monk/request 2 then Fighter/request
3. The mocked Stage captures the current `onEvent` callback, so event 1 is rejected and event 3
is accepted by the request-ID guard.

```tsx
it('exposes stable labeled controls, status diagnostics, progress, and removed audits', () => {
  const removedAudit: RemovedTargetAudit = { targetId: 'removed', tuple: ['fighter', 'characters/old.glb', 'Idle_Old'], manifestSha256: 'old-manifest', modelSha256: 'old-model', verdict: 'fix', note: 'replace it', timestamp: '2026-07-22T00:00:00.000Z', outcome: 'stale', inCurrentCatalog: false, diagnosticCode: 'CATALOG_TARGET_REMOVED', message: 'target removed' };
  render(<IdleReviewForm activeTarget={readyTarget} activeRequestId={3} stageEvent={{ kind: 'error', targetId: readyTarget.targetId, requestId: 3, diagnostic: { code: 'ACTION_FAILED', modelUrl: readyTarget.modelUrl, requestedClip: readyTarget.clipName, modelSha256: 'hash', actualClipNames: ['Idle_Relaxed'], message: 'play failed' } }} exportDisabledReason={undefined} onSave={vi.fn()} onReset={vi.fn()} onExport={vi.fn()} currentProgress={{ currentTotal: 2, currentReviewed: 0, currentUnreviewed: 1, currentBlocked: 1, currentStale: 0, keep: 0, fix: 0, reject: 0, removedStaleAuditCount: 1 }} removedTargetAudits={[removedAudit]} />);
  expect(screen.getByRole('status')).toHaveTextContent('ACTION_FAILED'); expect(screen.getByRole('status')).toHaveTextContent('currentBlocked');
  expect(screen.getByLabelText('Class')).toBeInTheDocument(); expect(screen.getByLabelText('Clip')).toBeInTheDocument(); expect(screen.getByLabelText('Note')).toBeInTheDocument();
  expect(screen.getByRole('button', { name: 'Save review' })).toBeDisabled(); expect(screen.getByRole('button', { name: 'Export JSON' })).toBeInTheDocument(); expect(screen.getByRole('region', { name: 'Removed target audits' })).toHaveTextContent('target removed');
});
```

- [ ] **Step 2: Run RED tests**

Run: `npm test -- --run src/concepts/idle-review/IdleReviewForm.test.tsx src/concepts/idle-review/IdleAnimationConcept.test.tsx src/concepts/ConceptsView.test.tsx`
Expected: FAIL with missing form/concept/tab modules.

- [ ] **Step 3: Implement form and parent-owned repository**

The parent constructs once and cleans once:

```tsx
const repositoryRef = useRef<ExactModelRepository | null>(null);
if (!repositoryRef.current) repositoryRef.current = dependencies.createRepository();
useEffect(() => () => repositoryRef.current?.dispose(), []);
const onStageEvent = useEffectEvent((event: StageEvent) => {
  if (event.targetId !== active?.target.targetId || event.requestId !== active.requestId) return;
  setStageEvent(event);
});
function select(target: IdleTarget) { setActive({ target, requestId: ++requestCounter.current }); setStageEvent(undefined); }
```

`IdleReviewDependencies` is coherent test construction, not a disconnected loader:

```ts
export interface IdleReviewDependencies {
  loadCatalog: typeof loadCatalog;
  createRepository(): ExactModelRepository;
  storage: Storage;
  now(): string;
}
```

Catalog options use raw class keys as `<option value="fighter">Fighter</option>`; title-case
labels are display-only through `classKey[0].toUpperCase() + classKey.slice(1)`. Catalog loading,
`partitionStoredReviews`, and `Promise.all(uniqueCurrentPersistedModelUrls.map((url) => repositoryRef.current!.get(url)))` run before settled
progress/export. Selecting an unreviewed target calls the same `repositoryRef.current!.get(target.modelUrl)`;
pass that cached `ModelAssetResult` to stage; stale
reconfirmation invokes save only after matching ready target/request and writes current
hashes. `IdleReviewStage` receives `assetResult` and its outer DOM controls; the concept forwards
its exact Stage event through `onStageEvent`, and passes a matching `error.diagnostic` keyed by
`targetId` as the optional `stageDiagnostics` map to `classifyCurrentTargets`, so
`ACTION_FAILED` keeps verdict disabled and makes the exported current target blocked. Form renders
requested/actual clip names, duration, model path, hashes, display-only production selection,
`No manifest default declared`, removed
audits, and current-only progress. The sole `role="status"` always renders active
load/diagnostic/readiness text plus labeled `currentTotal`, `currentReviewed`,
`currentUnreviewed`, `currentBlocked`, and `currentStale` counts in the same region. An accepted
`Ready: fighter / Idle_Relaxed (playing)` is additive and never removes those counts, so browser
helpers may wait for `currentUnreviewed` or `currentStale` after selection readiness. Selectors
are labels `Class`, `Clip`, and `Note`; buttons are `Save review` and
`Export JSON`; audits are in `role="region" aria-label="Removed target audits"`. Add
`'idle-review'` tab in ConceptsView, without route changes.
Update only the current contents list in concepts-route documentation.

- [ ] **Step 4: Run GREEN tests, inspect, stage exact files, and commit**

Run:

```bash
npm test -- --run src/concepts/idle-review/IdleReviewForm.test.tsx src/concepts/idle-review/IdleAnimationConcept.test.tsx src/concepts/ConceptsView.test.tsx
git status --short
git diff --check
git add src/concepts/idle-review/IdleReviewForm.tsx src/concepts/idle-review/IdleReviewForm.test.tsx src/concepts/idle-review/IdleAnimationConcept.tsx src/concepts/idle-review/IdleAnimationConcept.test.tsx src/concepts/ConceptsView.tsx src/concepts/ConceptsView.test.tsx docs/architecture/components/concepts-route.md
git commit -m "feat: add idle review concepts bench (${WEB_ISSUE})"
```

Expected: focused suites pass; A→B→A stale events cannot enable verdicts.

### Task 6: Browser Evidence, CI, Ready PR, And Independent Gate

**Files:**
- Create: `scripts/verify-idle-review.mjs`
- Create: `playtest-evidence/idle-review/overview.png`
- Create: `playtest-evidence/idle-review/game-like.png`

- [ ] **Step 1: Implement the standalone browser verification script and executable scenarios**

Create `scripts/verify-idle-review.mjs`. Use `node:assert/strict`, not Playwright `expect`; every
route is installed before `openLab` navigates, and every helper below is called from `main`.

```js
import assert from 'node:assert/strict'; import { mkdir } from 'node:fs/promises'; import { chromium } from 'playwright';
const base = 'http://127.0.0.1:3001/?playerId=idle-review'; const errors = [];
const browser = await chromium.launch(); const page = await browser.newPage({ viewport: { width: 1600, height: 1100 } });
page.on('console', (message) => { if (message.type() === 'error') errors.push(message.text()); }); page.on('pageerror', (error) => errors.push(String(error)));
const targets = [['Fighter', 'Idle_Relaxed'], ['Fighter', 'Idle_Stretch'], ['Fighter', 'Idle_Drinking'], ['Barbarian', 'Idle_Relaxed'], ['Barbarian', 'Idle_ChinScratch'], ['Barbarian', 'Idle_Drinking'], ['Monk', 'Idle_Relaxed'], ['Monk', 'Idle_Meditative'], ['Monk', 'Idle_Drinking'], ['Rogue', 'Idle_Relaxed'], ['Rogue', 'Idle_CheckWatch'], ['Rogue', 'Idle_Drinking']];
async function openLab() { await page.goto(base, { waitUntil: 'networkidle' }); await page.locator('[title="Open Concepts Lab"]').click(); await page.getByRole('button', { name: 'Idle Review' }).click(); }
async function scenario(install = async () => {}, { preserveDraft = false } = {}) { errors.length = 0; await page.unrouteAll(); if (!preserveDraft) { await page.goto(base, { waitUntil: 'networkidle' }); await page.evaluate(() => localStorage.clear()); } await install(); await openLab(); }
async function status() { return (await page.getByRole('status').textContent()) ?? ''; }
async function changedTime() { const first = Number(await page.getByTestId('animation-time').textContent()); await page.waitForFunction((prior) => Number(document.querySelector('[data-testid="animation-time"]')?.textContent) !== prior, first); return [first, Number(await page.getByTestId('animation-time').textContent())]; }
async function chooseTarget(className, clipName) { await page.getByRole('combobox', { name: 'Class' }).selectOption({ label: className }); await page.getByRole('combobox', { name: 'Clip' }).selectOption({ label: clipName }); }
async function waitForReady(classKey, clipName) { const ready = `Ready: ${classKey} / ${clipName} (playing)`; await page.getByRole('status').filter({ hasText: ready }).waitFor(); assert.match(await status(), new RegExp(`Ready: ${classKey} / ${clipName} \\(playing\\)`)); }
async function selectReadyTarget(className, clipName) { await chooseTarget(className, clipName); await waitForReady(className.toLowerCase(), clipName); }
async function saveKeep(className, clipName) { await selectReadyTarget(className, clipName); await page.getByRole('button', { name: 'Keep' }).click(); await page.getByRole('button', { name: 'Save review' }).click(); }
async function assertDownload() { const pending = page.waitForEvent('download'); await page.getByRole('button', { name: 'Export JSON' }).click(); const stream = await (await pending).createReadStream(); const chunks = []; for await (const chunk of stream) chunks.push(chunk); const exported = JSON.parse(Buffer.concat(chunks).toString('utf8')); assert.ok(Array.isArray(exported.targets)); assert.ok(Array.isArray(exported.removedTargetAudits)); assert.ok(Array.isArray(exported.catalogDiagnostics)); assert.equal(typeof exported.summary.currentTotal, 'number'); assert.equal(typeof exported.summary.currentReviewed, 'number'); assert.equal(typeof exported.summary.currentUnreviewed, 'number'); assert.equal(typeof exported.summary.currentBlocked, 'number'); assert.equal(typeof exported.summary.currentStale, 'number'); assert.equal(typeof exported.summary.removedStaleAuditCount, 'number'); for (const target of exported.targets) { assert.equal(typeof target.manifestSha256, 'string'); if (target.modelSha256 !== undefined) assert.equal(typeof target.modelSha256, 'string'); } return exported; }
async function verifyNormalTargetsAndControls() { await scenario(); for (const [className, clipName] of targets) { await selectReadyTarget(className, clipName); const [before, after] = await changedTime(); assert.notEqual(after, before, `${className}/${clipName} did not animate`); } await page.getByRole('button', { name: 'Studio' }).click(); await page.screenshot({ path: 'playtest-evidence/idle-review/overview.png', fullPage: true }); await page.getByRole('button', { name: 'Pause' }).click(); const paused = Number(await page.getByTestId('animation-time').textContent()); for (const name of ['0.25x', '0.5x', '1x', 'Front', 'Three-quarter', 'Side', 'Top']) { await page.getByRole('button', { name }).click(); assert.equal(Number(await page.getByTestId('animation-time').textContent()), paused, `${name} reset paused time`); } await page.getByRole('button', { name: 'Play' }).click(); await changedTime(); await page.getByRole('button', { name: 'Restart' }).click(); const restarted = Number(await page.getByTestId('animation-time').textContent()); assert.ok(restarted >= 0 && restarted < 0.25); await page.getByRole('button', { name: 'Game-like' }).click(); const gameLike = Number(await page.getByTestId('animation-time').textContent()); assert.ok(gameLike >= 0 && gameLike < 0.25); await page.locator('canvas').dragTo(page.locator('canvas'), { sourcePosition: { x: 30, y: 30 }, targetPosition: { x: 90, y: 90 } }); await page.locator('canvas').hover(); await page.mouse.wheel(0, -100); await page.screenshot({ path: 'playtest-evidence/idle-review/game-like.png', fullPage: true }); assert.deepEqual(errors, []); }
async function verifyDuplicateManifest() { await scenario(async () => { await page.route('**/models/synty/characters/manifest.json', async (route) => { const response = await page.request.get(route.request().url()); const manifest = await response.json(); manifest.mapping.fighter.idleClips = ['Idle_Relaxed', 'Idle_Relaxed']; await route.fulfill({ json: manifest }); }); }); assert.match(await status(), /DUPLICATE_IDLE_CLIP/); assert.doesNotMatch(await page.getByRole('combobox', { name: 'Class' }).textContent(), /Fighter/); assert.deepEqual(errors, []); }
async function routeRemovedRename() { await page.route('**/models/synty/characters/manifest.json', async (route) => { const response = await page.request.get(route.request().url()); const manifest = await response.json(); manifest.mapping.fighter.idleClips = manifest.mapping.fighter.idleClips.filter((clip) => clip !== 'Idle_Stretch'); manifest.mapping.fighter.idleClips.push('Idle_Stretch_New'); await route.fulfill({ json: manifest }); }); }
async function verifyRemovedRenameAudit() { await scenario(); await saveKeep('Fighter', 'Idle_Stretch'); await scenario(routeRemovedRename, { preserveDraft: true }); assert.match(await page.getByRole('region', { name: 'Removed target audits' }).textContent(), /Idle_Stretch/); await chooseTarget('Fighter', 'Idle_Stretch_New'); await page.getByRole('status').filter({ hasText: 'currentUnreviewed' }).waitFor(); assert.match(await status(), /unreviewed/i); assert.deepEqual(errors, []); }
async function verifyStaleModelIdentity() { await scenario(); await saveKeep('Fighter', 'Idle_Relaxed'); await scenario(async () => { const alternate = await page.request.get('http://127.0.0.1:3001/models/synty/characters/monk.glb'); const body = await alternate.body(); await page.route('**/models/synty/characters/fighter.glb', (route) => route.fulfill({ body })); }, { preserveDraft: true }); await chooseTarget('Fighter', 'Idle_Relaxed'); await page.getByRole('status').filter({ hasText: 'currentStale' }).waitFor(); assert.match(await status(), /currentStale/); assert.deepEqual(errors, []); }
async function verifyAbortedModel() { await scenario(async () => { await page.route('**/models/synty/characters/fighter.glb', (route) => route.abort('failed')); }); await chooseTarget('Fighter', 'Idle_Relaxed'); await page.getByRole('status').filter({ hasText: 'currentBlocked' }).waitFor(); assert.match(await status(), /MODEL_FETCH_FAILED/); assert.deepEqual(errors.filter((error) => !error.includes('fighter.glb')), []); }
async function verifyRevalidationGateAndExport() { await scenario(); await saveKeep('Fighter', 'Idle_Relaxed'); await scenario(async () => { await page.route('**/models/synty/characters/fighter.glb', async (route) => { await new Promise((resolve) => setTimeout(resolve, 100)); await route.continue(); }); }, { preserveDraft: true }); assert.equal(await page.getByRole('button', { name: 'Export JSON' }).isDisabled(), true); assert.match(await status(), /revalidating/i); await page.waitForFunction(() => ![...document.querySelectorAll('button')].find((button) => button.textContent === 'Export JSON')?.disabled); const exported = await assertDownload(); assert.equal(exported.summary.currentTotal, exported.targets.length); assert.deepEqual(errors, []); }
async function main() { await mkdir('playtest-evidence/idle-review', { recursive: true }); await verifyNormalTargetsAndControls(); await verifyDuplicateManifest(); await verifyRemovedRenameAudit(); await verifyStaleModelIdentity(); await verifyAbortedModel(); await verifyRevalidationGateAndExport(); }
try { await main(); assert.deepEqual(errors, [], `unexpected browser errors: ${errors.join('\n')}`); } finally { await browser.close(); }
```

- [ ] **Step 2: Run the browser script after the task’s focused RED/GREEN suite**

Run: `node scripts/verify-idle-review.mjs`
Expected: every named helper runs; normal targets animate; duplicate, removed/renamed, stale,
aborted, revalidation, download, and controls assertions pass; both screenshots are written.

- [ ] **Step 3: Sync assets, run browser verification, full CI, inspect, stage exact files, and commit**

Terminal one:

```bash
npm run assets:sync
npm run dev -- --host 127.0.0.1
```

Expected: Vite prints `http://127.0.0.1:3001/`.

Terminal two:

```bash
npm test -- --run src/concepts/idle-review/catalog.test.ts src/concepts/idle-review/modelRepository.test.ts src/concepts/idle-review/reviewState.test.ts src/concepts/idle-review/reviewStorage.test.ts src/concepts/idle-review/calibration.test.ts src/concepts/idle-review/IdleReviewStage.test.tsx src/concepts/idle-review/IdleReviewForm.test.tsx src/concepts/idle-review/IdleAnimationConcept.test.tsx src/concepts/ConceptsView.test.tsx
node scripts/verify-idle-review.mjs
npm run ci-check
git status --short
git diff --check
git add scripts/verify-idle-review.mjs playtest-evidence/idle-review/overview.png playtest-evidence/idle-review/game-like.png
git commit -m "test: capture idle review evidence (${WEB_ISSUE})"
```

Expected: focused tests, browser script, and CI exit 0; screenshots exist; no
`public/models/synty/` path appears in status/diff.

- [ ] **Step 4: Push, create ready PR, acknowledge Copilot, request gate, and stop before merge**

```bash
git push -u origin "feat/${WEB_ISSUE}-idle-animation-review"
HEAD_SHA="$(git rev-parse HEAD)"
RAW="https://raw.githubusercontent.com/KirkDiggler/rpg-dnd5e-web/${HEAD_SHA}"
PR_BODY="$(printf '%s\n' "Closes #${WEB_ISSUE}" '' 'Tracks rpg-project#112 and rpg-project PR #113.' '' '## Viewed evidence' '' "![Overview](${RAW}/playtest-evidence/idle-review/overview.png)" '- I viewed the overview evidence: all current manifest targets are selectable in Idle Review.' '' "![Game-like](${RAW}/playtest-evidence/idle-review/game-like.png)" '- I viewed the game-like evidence: the shared scale and representative scene baseline render together.' '' '## Verification' '' '- npm run ci-check' '- npm run assets:sync followed by node scripts/verify-idle-review.mjs' '' '— asset-pipeline agent, on behalf of KirkDiggler')"
PR_URL="$(gh pr create --repo KirkDiggler/rpg-dnd5e-web --base main --head "feat/${WEB_ISSUE}-idle-animation-review" --title 'feat: idle animation review harness' --body "$PR_BODY")"
WEB_PR="${PR_URL##*/}"
gh pr view "$WEB_PR" --repo KirkDiggler/rpg-dnd5e-web --json state,isDraft,baseRefName,files
gh project item-list 19 --owner KirkDiggler --format json --limit 500 | jq -e ".items[] | select(.content.number == ($WEB_ISSUE | tonumber)) | select(.team == \"Assets\" and .feature == \"Infra\" and .status == \"In Progress\")"
IN_REVIEW="$(printf '%s' "$FIELDS" | jq -r '.fields[] | select(.name == "Status") | .options[] | select(.name == "In Review") | .id')"
if [ "$IN_REVIEW" != "null" ]; then gh project item-edit --project-id "$PROJECT_ID" --id "$ITEM" --field-id "$STATUS_FIELD" --single-select-option-id "$IN_REVIEW"; fi
gh project item-list 19 --owner KirkDiggler --format json --limit 500 | jq -e ".items[] | select(.content.number == ($WEB_ISSUE | tonumber)) | select(.team == \"Assets\" and .feature == \"Infra\" and (.status == \"In Review\" or .status == \"In Progress\"))"
```

Expected: PR is open, not draft, targets main, and contains no GLB assets. Wait for CI and
Copilot; every comment reply ends the required signature. Supply design, plan, diff, CI,
browser output, and viewed evidence to an independent agent GATE REVIEW. Fix a gate finding
in a new commit with its focused GREEN test and `npm run ci-check`; report pass evidence to
Kirk. Do not merge the web PR or rpg-project PR #113.

---

## Plan Self-Check

- Coverage: Task 1 establishes issue/worktree and shared byte/catalog contracts; Task 2
  shares parse-once assets; Task 3 covers drafts/revalidation/export; Task 4 covers
  calibration/stage cleanup; Task 5 covers form/orchestration/tab/docs; Task 6 covers real
  assets, failure routes, evidence, CI, PR, Copilot, and gate.
- Types: catalog diagnostics and model diagnostics are distinct unions; every referenced
  export is declared in the task producing it; stage accepts repository results rather than
  disconnected loader hooks.
- TDD: Tasks 1-5 order failing tests, RED commands, implementation, GREEN commands, diff/status
  inspection, exact staging, then a new logical commit. Task 6 is executable browser/CI evidence
  verification; CI precedes push.
- Scope: only Concepts Lab registration changes existing product source. No route, encounter,
  resolver, manifest, backend, production cache, or licensed asset change is authorized.
