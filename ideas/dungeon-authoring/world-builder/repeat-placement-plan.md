# Repeat Placement Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Draw repeated cataloged barricades as ordinary editable scene pieces/group, with correct spacing and one Undo.

**Architecture:** One web-only, integrated UI deliverable. Pure layout returns transforms; a scene operation creates ordinary props/groups; transient viewport gestures preview and commit once through existing room history. No provider work or persistent run type.

**Tech Stack:** TypeScript, React, React Three Fiber/Three.js, Vitest, R3F test renderer, existing browser harness.

**Spec:** [repeat-placement.md](repeat-placement.md), including Kirk's barricade-first amendment. Historical Crypt enrollment paragraphs are NOT implementation requirements.

## Global Constraints

- Owning issue: web#1080, Project 19 UI/UX / The Dungeon / Concept / In Progress; parent journey project#169.
- Code worktree: `/home/kirk/game-dev/rpg-dnd5e-web/.worktrees/1080-repeat-placement`, branch `concept/1080-repeat-placement`, base `e83dd2db3dc5a154ee8f0995673771de2db8f713` (includes #1071 touch-pan).
- First exact ref: `dnd5e:props:dark-fortress:barricade_02`. Existing catalog and runtime bytes only; no generated catalog edits, asset promotion, or dependency upgrades.
- Preserve geometry/render scale. Fix the extra scaling of reported dimensions only. Catalog extents already include runtime scale `0.75`.
- Whole-copy snapping, free world X/Z, y=0. Result: one ordinary prop or, for 2+ copies, an ordinary group. No persisted generator/run metadata.
- Existing validation limits remain: MAX_ITEMS=200, MAX_GROUPS=80, existing workspace extents and JSON limits. Invalid runs are rejected atomically, not truncated.
- No floors, pattern/randomization controls, variant replacement UI, auto-enclosure/corner fitting, backend, RPC, or YAML work.
- No source edits in the retained #1068 tree, primary checkouts, game-dev scripts, or Assets. Do not reset/reseed/rebuild API or Redis.
- Keep localhost:3030 and Kirk's drafts intact. Parent owns the eventual Vite-only source handoff. Worker may use an unused temporary browser-proof port, with an owned server and fresh browser context, then stop only that server.
- First stop is Kirk's hands-on checkpoint. Local commits with normal hooks are allowed; no push/PR, full ci-check, review dispatch, merge, or cleanup before that checkpoint. Later: one complete ci-check at PR boundary, one GLM review, human-visible findings before any fix loop.

## File map

| File | Responsibility |
|---|---|
| `src/components/hex-grid/WorldAssetModel.tsx` + `.test.tsx` | Correct reported bounds; unchanged model rendering |
| `src/concepts/world-building/repeatPlacement.ts` + `.test.ts` (new) | Pure bounded transform layout and immutable scene insertion |
| `src/concepts/world-building/RepeatPlacementPreview.tsx` (new, only if useful) | Existing model leaf rendered as transient non-interactive copies |
| `src/concepts/world-building/WorldBuildingConcept.tsx` + `.test.tsx` | Catalog selection/Repeat controls; single existing room commit |
| `src/concepts/world-building/WorldBuildingViewport.tsx` + `.test.tsx` | Repeat gesture ownership, preview, cancel, release |
| `src/concepts/world-building/worldBuilding.css` | Small control/preview styling; no UI redesign |

`sceneState.ts`, `serialization.ts`, `catalog.ts`, `WorldPropModel.tsx`, `roomDraft.ts`, and `types.ts` are existing owners to reuse. Do not duplicate their constants, validation, renderer selection, or persistence model.

## Task 1: One repeat-placement UI slice through the first look

**Interfaces:**

```ts
import type { IdFactory, WorldPoint, WorldScene, WorldTransform } from './types';

export interface RepeatLayoutInput {
  start: WorldPoint;
  end: WorldPoint;
  step: number;                  // scene units, NOT multiplied by scale again
  originOffset: number;          // distance from leading end to asset origin
  maxCount: number;              // remaining scene capacity, bounded by MAX_ITEMS
}
export interface RepeatLayout {
  transforms: WorldTransform[];
  count: number;
  snappedEnd: WorldPoint;
}
export interface AddRepeatedPropsInput {
  scene: WorldScene;
  assetRef: string;
  transforms: readonly WorldTransform[];
  idFactory: IdFactory;
  label: string;
}
export interface AddRepeatedPropsOutput {
  scene: WorldScene;
  selectedIds: string[];
}
// New module exports:
// layoutRepeatedProps(input: RepeatLayoutInput): RepeatLayout
// addRepeatedProps(input: AddRepeatedPropsInput): AddRepeatedPropsOutput
```

### Step 1 — isolate setup and establish a focused baseline

- [ ] Read the worktree AGENTS, the UI/UX charter, this task and the spec. No child agents.
- [ ] Install this worktree's locked dependencies with `npm ci --ignore-scripts`; do not symlink node_modules. Assert the existing repository hook path is `.husky/_`, run the normal `npm run prepare` to install this worktree's Husky support, and confirm the hook path is still `.husky/_` with an executable `.husky/_/pre-commit`. Do not commit with missing hooks or change hook policy.
- [ ] Copy only ignored runtime `public/models/synty/` and, if required, `public/models/custom-dice/` from the retained #1068 worktree into this worktree. Verify destination ignore rules first; no deletion, sync generator, source-pipeline command, or provider update. Check barricade bytes against the tracked catalog SHA `5602a6f9008a08f0535a9b6470f8b59bcda027e469be1d52b8c2e45e37e91d67`.
- [ ] Run the focused baseline, saving logs outside scanned web Markdown:

```bash
npm run test:run -- src/components/hex-grid/WorldAssetModel.test.tsx src/concepts/world-building/WorldBuildingConcept.test.tsx src/concepts/world-building/WorldBuildingViewport.test.tsx src/concepts/world-building/roomDraft.test.ts
```

Stop with source identity/logs on setup failure; do not switch execution protocols or silently repair unrelated dependencies.

### Step 2 — red/green for trustworthy reported dimensions

- [ ] Replace the test's duplicated incorrect scaling expectation with catalog dimensions directly. Add a SEPARATE zero-yaw geometry test: construct the mock source BoxGeometry with the measured raw dimensions `[0.24207866191864014, 0.2667747139930725, 0.23023077845573425]`. After the actual renderer wrapper transform, Box3 dimensions must equal both catalog dimensions and the reported bounds. Keep the existing authored-yaw=0.7 transform test, but do not compare its rotated world AABB to local dimensions.

```ts
const [width, height, depth] = GENERATED_WORLD_ASSETS[REF]!.boundsMeters;
expect(onBoundsMeasured).toHaveBeenCalledWith({
  minY: 0, maxY: height, width, height, depth,
});
```

Separate zero-yaw test, using the source-sized mock described above:

```tsx
it('reports the actual rendered source geometry size exactly once', async () => {
  const onBoundsMeasured = vi.fn();
  const [width, height, depth] = GENERATED_WORLD_ASSETS[REF]!.boundsMeters;
  const renderer = await ReactThreeTestRenderer.create(
    <WorldAssetModel assetRef={REF} position={[0, 0, 0]} rotationY={0}
      onBoundsMeasured={onBoundsMeasured} />
  );
  const model = renderer.scene.findByProps({ name: 'world-asset-model' });
  model.instance.updateWorldMatrix(true, true);
  const actual = new THREE.Box3().setFromObject(model.instance)
    .getSize(new THREE.Vector3());
  expect(actual.x).toBeCloseTo(width);
  expect(actual.y).toBeCloseTo(height);
  expect(actual.z).toBeCloseTo(depth);
  expect(onBoundsMeasured).toHaveBeenCalledWith({
    minY: 0, maxY: height, width, height, depth,
  });
  await renderer.unmount();
});
```

- [ ] Run WorldAssetModel.test.tsx and observe the expected failure. Then remove SYNTY_SCALE multiplication only in its reported `maxY/width/height/depth`. Keep the rendering group's `scale={SYNTY_SCALE}` and position/yaw unchanged. Run the test green.

### Step 3 — red/green for layout and ordinary scene insertion

- [ ] Write tests for this exact simple case plus reverse/diagonal, one piece, invalid inputs and capacity overflow:

```ts
const layout = layoutRepeatedProps({
  start: { x: 0, z: 0 }, end: { x: 6.2, z: 0 },
  step: 2, originOffset: 1, maxCount: 10,
});
expect(layout.count).toBe(3);
expect(layout.transforms.map(p => p.x)).toEqual([1, 3, 5]);
expect(layout.transforms.every(p => p.y === 0 && p.z === 0)).toBe(true);
expect(layout.snappedEnd).toEqual({ x: 6, z: 0 });
expect(() => layoutRepeatedProps({
  start: { x: 0, z: 0 }, end: { x: 1000, z: 0 },
  step: 2, originOffset: 1, maxCount: 2,
})).toThrow();
```

- [ ] Run new tests red. Implement finite/positive measurement validation and integer capacity validation before allocation. Count is `Math.max(1, Math.round(distance / step))`; for zero drag use +X. Reject non-finite/unsafe counts or counts above capacity. Direction/yaw and origins are:

```ts
const dx = end.x - start.x;
const dz = end.z - start.z;
const distance = Math.hypot(dx, dz);
const ux = distance === 0 ? 1 : dx / distance;
const uz = distance === 0 ? 0 : dz / distance;
const rotationY = Math.atan2(-uz, ux);
// For each integer i from 0 to count-1:
const transform = {
  x: start.x + ux * (originOffset + i * step),
  y: 0,
  z: start.z + uz * (originOffset + i * step),
  rotationY,
};
```

Import existing MAX_ITEMS to bound any externally supplied maxCount; do not allow a caller to allocate an unbounded preview.

- [ ] Implement insertion using existing `addProp`/`groupSelection`; all state is local until the caller commits. Fail on duplicate IDs. For a one-piece result select its ID; for multiple pieces create/select one group ID. Validate the completed scene with the current room workspace before committing. Reuse the current reconciler for room declarations; do not infer gameplay flags from the asset or its dimensions.
- [ ] Tests prove input scene unchanged, fresh IDs/parentId, one-copy/no group, multiple copies/group, failure without a partial scene, and output accepted by existing serialization. Run layout, scene, and serialization-focused checks green.

### Step 4 — add the smallest usable controls and gesture

- [ ] Add a room-mode **Repeat** action for generated catalog entries with valid dimensions (including both barricades), selecting that exact ref and arming Repeat without placing anything. Keep the existing palette drag behavior. A toolbar Repeat button may reuse the selected repeat ref; do not add a separate asset browser.
- [ ] Extend the room tool union with `repeat`; keep ordinary manipulation modes intact. Pass a small optional descriptor/callback through `roomAuthoring`:

```ts
repeat?: { assetRef: string; step: number; originOffset: number; maxCount: number };
onRepeatGesture?: (assetRef: string, transforms: readonly WorldTransform[]) => void;
```

The descriptor uses generated `boundsMeters[0]` directly, half-width origin offset, and MAX_ITEMS minus committed item count. Missing measurements cannot fall back to guessed dimensions. Selecting another asset/tool cancels any old preview.

- [ ] Extend existing floor-gesture capture/cancel discipline. Primary-pointer down records the start and immutable descriptor; move computes bounded transforms; release clears capture/preview and calls the callback exactly once. Ignore other pointer IDs. Escape/right click/pointercancel/lostcapture/tool change/unmount discard the gesture. Do not interfere with #1071 touch pan or existing camera controls.
- [ ] Show actual model copies via `WorldPropModel` (shared GLB cache), with a visible preview cue/count; no per-model interaction handlers or mutation of shared materials. Keep transient models out of committed model-load counters, selection, history and storage. Invalid preview shows an explanation and cannot commit.
- [ ] Concept callback uses `addRepeatedProps`, then its existing `commit` once with the selected result:

```ts
onRepeatGesture: (assetRef, transforms) => {
  try {
    const result = addRepeatedProps({
      scene, assetRef, transforms, idFactory, label: 'Repeated pieces',
    });
    commit(result.scene, result.selectedIds);
  } catch (error) {
    setNotice(error instanceof Error ? error.message : String(error));
  }
}
```

Repeat stays armed for another run. Test that choosing a palette Repeat action does not also trigger its drag/drop/keyboard-placement path.

### Step 5 — focused interaction proofs, then normal local commit

- [ ] Add real R3F floor-handler coverage for capture, move, off-ground release, second-pointer ignore and cancellation. Do not use a mocked button with no production handler as cancellation proof.
- [ ] Concept-level tests use the real callback and history: no preview persistence, one Undo removes the whole group, Redo restores IDs, one-copy output, reload preserves transforms/parentId, and invalid limits leave scene/history/bytes unchanged. Keep existing invalid-load/StrictMode regressions green.
- [ ] Run focused changed/affected tests, scoped ESLint/Prettier, and `npm run typecheck`. No redundant full suite or ci-check at this first-look stage.
- [ ] Self-review the source; commit green changes with normal hooks. Do not push or publish yet.

### Step 6 — real browser first look and handoff

- [ ] If a browser proof needs a server, start a temporary owned Vite process on a verified-unused port using VITE_API_HOST=http://localhost:8110. Do not stop or modify :3030/:8110, API/Redis, the manifest/state file or the old worktree. Use a fresh isolated browser profile; stop only your temporary server afterward.
- [ ] Select Barricade 02 from the actual catalog. Draw 3+ pieces, repeat at another yaw, inspect the real mesh spacing, move/rotate the resulting group, Undo/Redo, then reload. Save screenshots and an interaction receipt. Natural holes/protrusions in a barricade are not a promise of an airtight masonry wall; measure placement correctly and show Kirk its real appearance.
- [ ] Return `ready-for-walk` or `blocked`, exact head, changed files, test/browser evidence, concise walk instructions, and remaining concerns. Parent owns the Vite-only :3030 handoff after checking source readiness; no API rebuild or Redis reset. Retain this worker for bounded feedback on the same slice.

## Parent checkpoints after this task

1. Verify evidence/source; safely repoint only the feature Vite process, keeping localhost:3030. If the environment helper cannot do a web-only source switch, perform a bounded, documented local process/state handoff rather than `up/down` or an API rebuild. Preserve old state for rollback.
2. Kirk walks. No automatic polish/fix/review loop; substantive changes return to him.
3. After the hands-on checkpoint, one final local ci-check, push/draft PR to dev, then one GLM 5.3 Flash review using the canonical pr-review skill. The single review covers this integrated task's spec and quality; no duplicate per-step/final seats.
4. Publish exact-head verdict/dispositions. No automatic merge, branch removal, environment teardown or provider work.
