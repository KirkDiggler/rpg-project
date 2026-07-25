# Fog of War Concept Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver a fixture-driven `/concepts` Fog of War two-room crypt that renders viewer-scoped visible and remembered knowledge with real HexGrid/Synty components, without changing production encounter behavior or cross-repo contracts.

**Architecture:** The web adds one small shared scene-knowledge presentation seam to the existing mixed HexGrid scene: known geometry stays in the normal floor/wall collections while remembered-key membership and per-entity state select an inert crypt-charcoal treatment. A typed concept fixture owns complete viewer projections, a separate world-truth inspector fixture, and a reducer limited to atomic projection replacement; the renderer never receives world truth or computes LOS.

**Tech Stack:** TypeScript 5.8, React 19, React Three Fiber 9, Three.js 0.181, `@react-three/drei`, Vitest 4, `@testing-library/react`, and `@react-three/test-renderer`. No dependency additions.

## Global Constraints

- This plan is canonical at `rpg-project/ideas/fog-of-war/plan.md`, adjacent to the approved `design.md`; implementation occurs only in `rpg-dnd5e-web`.
- Execute four sequential web issues/PRs. Before each task: create one `rpg-dnd5e-web` issue, add it to Board 19, set Feature=`The Dungeon`, set Team=`Assets` for Tasks 1-2 (shared 3D renderer seam) and Team=`UI/UX` for Tasks 3-4 (fixture and concept presentation), then create a fresh worktree/branch from latest `origin/main`. Use the issue number returned by `gh issue create`; never invent one.
- Each GitHub issue body or comment ends exactly `— asset-pipeline agent, on behalf of KirkDiggler`.
- Each task is one issue and one PR. Run `npm run ci-check` before its push; never use `--no-verify`. This planning operation creates no web issues, branches, pushes, or PRs.
- Scope is the concept plus the smallest additive shared renderer support. Do not modify `EncounterView.tsx`, `useEncounterState.ts`, production stream handling, toolkit, API, or protos. Omitted new props preserve production behavior byte-for-byte in intent.
- `UNSEEN` is omission, never a state value, dark substitute, inferred floor, wall, entity, or interaction target.
- Define `SceneKnowledgeState = 'visible' | 'remembered'`. Optional `knowledgeState` defaults to `'visible'` so existing production callers remain unchanged. Preserve `isGhost`: remembered is separate from pale-cyan ghosting and never changes current production ghost semantics.
- Use the single mixed geometry scene: all known floor tiles remain in `floorTiles`; all known walls remain in `walls`. `rememberedFloorHexKeys?: ReadonlySet<string>` and `rememberedWallHexKeys?: ReadonlySet<string>` only select presentation and interaction filtering. Do not split wall lists, mutate `Wall`, or introduce a geometry wrapper type.
- Centralize the initial memory presentation in `src/components/hex-grid/sceneKnowledge.ts`: `CRYPT_MEMORY_COLOR = new THREE.Color('#465366')`, `CRYPT_MEMORY_OPACITY = 1.0`, `CRYPT_MEMORY_EMISSIVE = new THREE.Color('#111923')`, and `CRYPT_MEMORY_EMISSIVE_INTENSITY = 0.08`. Remembered materials are opaque (`transparent = false`, `depthWrite = true`); cool charcoal color/emissive supplies separation without spectral transparency. Memory wins over selection, hover, theme, ghost, door, and wall highlights.
- Material safety is mandatory. `useGLTF` and `useTexture` assets are URL-cached: clone per-instance materials, support `Material | Material[]`, restore original cached materials before reuse, dispose only materials created by that instance/effect, and never dispose cached geometry, cached textures, or original materials. Primitive/loading fallbacks must visibly stay remembered too.
- The Canvas uses `frameloop="demand"`: remembered class models resolve no clip and never self-invalidate. Do not add movement/facing hook enable flags unless direct inspection during execution proves clearing inputs insufficient.
- Remembered content renders but is inert. Build pathing, occupancy, interaction, turn-order, self-indicator, hover, and door handlers from visible-only records/geometry. Remembered entities never block based on stale positions. Remembered floors cannot be hovered/clicked/pathed. Remembered doors have no handlers, cursor change, or propagation stop. `TurnOrderOverlay` receives no remembered IDs.
- `showFrontierGroundHints?: boolean` defaults to `true`; the concept passes `false` so hints never expose unseen-adjacent ground.
- The concept projection is a typed, complete authored scene projection with floor/wall/entity records carrying real `AbsoluteFloorTile`, generated `Wall`, and shared `HexGridEntity` payloads plus stable id, `roomId`, and state. The adapter emits exact HexGrid props and omits unseen records. `currentRoomId` is camera/context metadata only; never use it to filter Room 1 remembered content from Room 2 live content.
- Reducer actions are only `hydrate` and atomic `replaceProjection`. It does not accept world truth and has no reveal, door-open, LOS, or inferred-memory action. Invalid records fail closed by omission; malformed remembered records never become visible.
- Do not build Intelligence, senses, retention, palette-selection, or production persistence systems. `CONTRACT.md` is evidence-only, not a platform request.
- Visual evidence uses the real development Concepts Lab, not standalone HTML. Task 4 adds a dev-only query seam: `?concept=fog-of-war&fogStep=<step-id>` opens Concepts Lab and selects an authored Fog step; normal navigation is unchanged. The exact screenshot form is `node /home/kirk/game-dev/tools/browser/screenshot.mjs <url> <output.png> 5000 <width> <height>`.

---

## File Map

| File | Responsibility |
| --- | --- |
| `src/components/hex-grid/sceneKnowledge.ts` | Shared `SceneKnowledgeState`, crypt constants, `isRemembered`, key/segment/fitting membership helpers, and safe per-instance material tint cloning. |
| `src/components/hex-grid/HexGrid.tsx` | Exports `HexGridEntity`; accepts remembered key sets and frontier flag; keeps mixed rendering while deriving visible-only interaction/path/occupancy/turn-order inputs. |
| `src/components/hex-grid/{SyntyHexFloor,ShadedHexFloor}.tsx` | Applies crypt-charcoal per remembered floor tile; shaded instancing retains map iteration order for color indexing. |
| `src/components/hex-grid/{SyntyHexWall,ShadedHexWall}.tsx` | Applies remembered wall/door/frame/end/fitting treatment and removes remembered door event handlers. |
| `src/components/hex-grid/{HexEntity,ClassCharacterModel,MediumHumanoid,PropModel}.tsx` | Renders remembered entities frozen and inert through real asset and fallback paths. |
| `src/components/playtest/playtestMapHelpers.ts` | Aliases `RenderableEntity` to the exported shared HexGrid entity contract. |
| `src/concepts/fog-of-war/{fixtures,reducer,adapter}.ts` | Typed viewer projections, isolated world truth, authored control steps, fail-closed reducer, and exact HexGrid input adapter. |
| `src/concepts/fog-of-war/FogOfWarConcept.tsx` | Responsive control/inspector page mounting a real crypt `HexGrid`. |
| `src/concepts/fog-of-war/{fixtures,reducer,adapter,FogOfWarConcept}.test.ts[x]` | Pure contract, reducer, adapter, and page behavior coverage. |
| `src/{App.tsx,concepts/ConceptsView.tsx,concepts/README.md}` | Dev-only query selection, Fog-of-war registration, and concepts-wide executable-contract policy. |
| `src/concepts/fog-of-war/CONTRACT.md` | Evidence and candidate gaps only; no platform request. |

## Task 1: Shared knowledge contract and inert mixed geometry

**Issue/branch setup:** Create the issue first with Feature=`The Dungeon`, Team=`Assets`, then use its returned number in the branch/worktree name. Example commands use `$ISSUE`, set from `gh issue create` output.

**Files:**
- Create: `src/components/hex-grid/sceneKnowledge.ts`
- Create: `src/components/hex-grid/sceneKnowledge.test.ts`
- Modify: `src/components/hex-grid/HexGrid.tsx`
- Modify: `src/components/hex-grid/SyntyHexFloor.tsx`
- Modify: `src/components/hex-grid/ShadedHexFloor.tsx`
- Modify: `src/components/hex-grid/SyntyHexWall.tsx`
- Modify: `src/components/hex-grid/ShadedHexWall.tsx`
- Modify: `src/components/hex-grid/SyntyHexFloor.test.tsx`
- Modify: `src/components/hex-grid/SyntyHexWall.test.tsx`
- Modify: `src/components/hex-grid/ShadedHexWall.test.tsx`
- Modify: `src/components/hex-grid/HexGrid.movement.test.ts`

**Interfaces:**
- Consumes: `AbsoluteFloorTile`, generated v1alpha2 `Wall`, `wallKey`, `doorHexKinds`, `frontierGroundHintHexes`, Synty segment keys (`wallHex->neighborHex`), and fitting keys (`hexA|hexB|hexC`).
- Produces: `SceneKnowledgeState`, `HexGridEntity`, `rememberedSegment`, `rememberedFitting`, optional HexGrid remembered key sets, and `showFrontierGroundHints` defaulting to `true`.

- [ ] **Step 1: Create the issue and isolated task worktree.**

```bash
ISSUE_URL=$(gh issue create --repo KirkDiggler/rpg-dnd5e-web --title "Fog of War: remembered geometry seam" --body $'Add shared remembered-scene presentation and inert mixed geometry for the approved Fog of War concept. No stream or production encounter-state changes.\n\n— asset-pipeline agent, on behalf of KirkDiggler')
ISSUE=${ISSUE_URL##*/}
gh project item-add 19 --owner KirkDiggler --url "$ISSUE_URL"
# Set Feature=The Dungeon and Team=Assets using field/option IDs read from Board 19 at execution time.
git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin main
git -C /home/kirk/game-dev/rpg-dnd5e-web worktree add "/tmp/opencode/web-$ISSUE-fog-geometry" -b "feat/$ISSUE-fog-geometry" origin/main
npm --prefix "/tmp/opencode/web-$ISSUE-fog-geometry" install
```

Expected: the board item carries the returned issue number and requested fields; the new worktree is clean on `feat/$ISSUE-fog-geometry`.

- [ ] **Step 2: Write failing shared-contract tests.**

Create `sceneKnowledge.test.ts` with tests for visible-by-default, remembered-only state, a segment whose key begins with a remembered wall hex, a fitting whose `|`-joined touching keys include one remembered wall key, and `cloneCryptMaterials` scalar/array safety:

```ts
import { describe, expect, it } from 'vitest';
import * as THREE from 'three';
import {
  cloneCryptMaterials,
  isRemembered,
  rememberedFitting,
  rememberedSegment,
} from './sceneKnowledge';

describe('scene knowledge decisions', () => {
  it('defaults omitted state to visible and recognizes remembered only', () => {
    expect(isRemembered(undefined)).toBe(false);
    expect(isRemembered('visible')).toBe(false);
    expect(isRemembered('remembered')).toBe(true);
  });
  it('recognizes remembered segment and fitting keys', () => {
    const remembered = new Set(['1,-1,0']);
    expect(rememberedSegment('1,-1,0->2,-2,0', remembered)).toBe(true);
    expect(rememberedFitting('0,0,0|1,-1,0|1,0,-1', remembered)).toBe(true);
  });
  it('clones scalar and array materials without mutating originals', () => {
    const scalar = new THREE.MeshStandardMaterial({ color: '#ffffff' });
    const array = [new THREE.MeshStandardMaterial({ color: '#ffffff' }), new THREE.MeshBasicMaterial({ color: '#ffffff' })];
    const scalarClone = cloneCryptMaterials(scalar) as THREE.MeshStandardMaterial;
    const arrayClone = cloneCryptMaterials(array) as THREE.Material[];
    expect(scalarClone).not.toBe(scalar);
    expect(Array.isArray(arrayClone)).toBe(true);
    expect(arrayClone[0]).not.toBe(array[0]);
    expect(scalar.color.getHexString()).toBe('ffffff');
    expect(scalarClone.color.getHexString()).toBe('465366');
    expect(scalarClone.transparent).toBe(false);
    expect(scalarClone.depthWrite).toBe(true);
    expect(scalarClone.opacity).toBe(1);
    expect(arrayClone[0]!.transparent).toBe(false);
    expect(arrayClone[0]!.depthWrite).toBe(true);
    expect(arrayClone[0]!.opacity).toBe(1);
  });
});
```

- [ ] **Step 3: Run the focused test and confirm the missing-module failure.**

Run: `npm test -- --run src/components/hex-grid/sceneKnowledge.test.ts`

Expected: FAIL because `./sceneKnowledge` does not exist.

- [ ] **Step 4: Add the shared type, concrete crypt constants, and pure membership helpers.**

Create `sceneKnowledge.ts`:

```ts
import * as THREE from 'three';

export type SceneKnowledgeState = 'visible' | 'remembered';
export const CRYPT_MEMORY_COLOR = new THREE.Color('#465366');
export const CRYPT_MEMORY_EMISSIVE = new THREE.Color('#111923');
export const CRYPT_MEMORY_OPACITY = 1.0;
export const CRYPT_MEMORY_EMISSIVE_INTENSITY = 0.08;

export function isRemembered(state?: SceneKnowledgeState): boolean {
  return state === 'remembered';
}
export function rememberedSegment(key: string, keys?: ReadonlySet<string>): boolean {
  return keys?.has(key.split('->')[0]!) ?? false;
}
export function rememberedFitting(key: string, keys?: ReadonlySet<string>): boolean {
  return key.split('|').some((hexKey) => keys?.has(hexKey) ?? false);
}
export function cloneCryptMaterials(
  original: THREE.Material | THREE.Material[]
): THREE.Material | THREE.Material[] {
  const source = Array.isArray(original) ? original : [original];
  const clones = source.map((material) => {
    const clone = material.clone();
    if ('color' in clone && clone.color instanceof THREE.Color) clone.color.multiply(CRYPT_MEMORY_COLOR);
    if (clone instanceof THREE.MeshStandardMaterial) {
      clone.emissive.copy(CRYPT_MEMORY_EMISSIVE);
      clone.emissiveIntensity = CRYPT_MEMORY_EMISSIVE_INTENSITY;
    }
    clone.transparent = false;
    clone.opacity = CRYPT_MEMORY_OPACITY;
    clone.depthWrite = true;
    return clone;
  });
  return Array.isArray(original) ? clones : clones[0]!;
}
```

Implementers must use an effect around `cloneCryptMaterials`: snapshot each clone's original material, assign only cloned materials while remembered, restore the original reference on cleanup/state change, flatten a scalar result to `[clone]` and an array result to `clone` for disposal, and dispose exactly those created clones. The helper itself never disposes input material.

- [ ] **Step 5: Make HexGrid’s entity and geometry seam additive.**

Replace the inline `HexGridProps['entities']` shape with this exported contract at the existing `HexGridProps` location, then add the three optional props:

```ts
export interface HexGridEntity {
  entityId: string;
  name: string;
  position: { x: number; y: number; z: number };
  type: 'player' | 'monster' | 'obstacle';
  isDead?: boolean;
  isGhost?: boolean;
  classRefId?: string;
  isDowned?: boolean;
  obstacleType?: ObstacleType;
  propRefId?: string;
  movePath?: { x: number; y: number; z: number }[];
  moveSeq?: number;
  knowledgeState?: SceneKnowledgeState;
}

export interface HexGridProps {
  floorTiles: Map<string, AbsoluteFloorTile>;
  rememberedFloorHexKeys?: ReadonlySet<string>;
  entities: HexGridEntity[];
  walls?: Wall[];
  rememberedWallHexKeys?: ReadonlySet<string>;
  showFrontierGroundHints?: boolean;
  // Retain every existing prop unchanged.
}
```

Inside `Scene`, derive `visibleEntities`, `visibleFloorTiles`, and `visibleWalls` with `knowledgeState !== 'remembered'` and the respective remembered-key set. Use only those visible collections for `isHexBlocked`, `entitiesMap`, `currentEntityPosition`, `doorKinds`, `useHexInteraction`, movement range, `doorPositions`, `wallPositions`, entity click resolution, and self indicator. Keep full `floorTiles`, `walls`, and `entities` for rendering. Calculate frontier hints only when `showFrontierGroundHints !== false`; otherwise use `[]`. Pass the remembered sets to both floor and wall renderers. Filter `turnOrder` in `HexGrid` against `props.entities` so remembered IDs are absent before `TurnOrderOverlay` receives them.

- [ ] **Step 6: Add remembered floor precedence in both renderers.**

Add `rememberedFloorHexKeys?: ReadonlySet<string>` to `SyntyHexFloorProps` and `ShadedHexFloorProps`. In Synty, calculate `isRemembered = rememberedFloorHexKeys?.has(key) ?? false` and select an opaque memory `meshBasicMaterial` with `color={CRYPT_MEMORY_COLOR}`, `opacity={1}`, `transparent={false}`, `depthWrite`, and `toneMapped={false}` before the existing crypt-theme branch. In shaded, preserve map iteration order: its creation effect and color effect iterate `for (const [key, tile] of floorTiles)` in identical order; in the color effect choose memory color before selected, hovered, door, wall, or base variation.

Add an R3F test assertion that a remembered Synty tile uses `#465366` even when `spaceTheme="crypt"`, and a unit-level color-decision test exported from `sceneKnowledge.ts` if extracting the shaded priority makes it testable. Do not reorder `floorTiles`, because instance color index is coupled to map iteration order.

- [ ] **Step 7: Add remembered wall and door behavior without splitting wall lists.**

Add `rememberedWallHexKeys?: ReadonlySet<string>` to both wall props. In `SyntyHexWall`, compute `isRemembered` for each segment via `rememberedSegment(key, rememberedWallHexKeys)`, and for each vertex fitting via `rememberedFitting(key, rememberedWallHexKeys)`. Extend `GlbInstance` with `remembered?: boolean`; for remembered wall segments, end caps, door frames, door leaves, and fittings it uses `cloneCryptMaterials` so every cloned material is charcoal, opaque, and depth-writing. Non-remembered theme/locked-door tint behavior remains unchanged. For a remembered door render a plain `<group>` with no `onClick`, `onPointerOver`, or `onPointerOut`; do not stop propagation or alter the cursor. In `ShadedHexWall`, choose the opaque memory color before `getKindColor`, and attach no door handlers when its `wall.from` key belongs to `rememberedWallHexKeys`.

Add tests that a remembered closed door has no event target in both renderers and that a Synty fitting key such as `0,0,0|1,-1,0|1,0,-1` becomes tinted when `1,-1,0` is remembered. The full wall list remains mounted so segment and fitting construction see all neighbors.

- [ ] **Step 8: Verify geometry interaction isolation and focused suites.**

Extend `HexGrid.movement.test.ts` so a remembered closed door is absent from the visible `doorKinds` passed to pathing, while a visible closed door still blocks. Verify the chosen API explicitly: `isHexBlocked` consumes the visible floor set, visible entity list, and visible door map rather than inspecting remembered state itself.

Run:

```bash
npm test -- --run src/components/hex-grid/sceneKnowledge.test.ts src/components/hex-grid/SyntyHexFloor.test.tsx src/components/hex-grid/SyntyHexWall.test.tsx src/components/hex-grid/ShadedHexWall.test.tsx src/components/hex-grid/HexGrid.movement.test.ts
```

Expected: PASS; omitted props retain the existing theme, wall, door, and pathing test behavior.

- [ ] **Step 9: Review, commit, and open the task PR only after CI.**

```bash
git status --short
git diff --check
npm run ci-check
git add src/components/hex-grid/sceneKnowledge.ts src/components/hex-grid/sceneKnowledge.test.ts src/components/hex-grid/HexGrid.tsx src/components/hex-grid/SyntyHexFloor.tsx src/components/hex-grid/ShadedHexFloor.tsx src/components/hex-grid/SyntyHexWall.tsx src/components/hex-grid/ShadedHexWall.tsx src/components/hex-grid/SyntyHexFloor.test.tsx src/components/hex-grid/SyntyHexWall.test.tsx src/components/hex-grid/ShadedHexWall.test.tsx src/components/hex-grid/HexGrid.movement.test.ts
git commit -m "feat: add remembered geometry rendering"
```

Expected: only Task 1 files are staged; `npm run ci-check` passes before the task branch is pushed and its one PR is opened.

## Task 2: Frozen remembered entities and visible-only turn state

**Issue/branch setup:** After Task 1 merges, create a new Board 19 issue with Feature=`The Dungeon`, Team=`Assets`; start a fresh worktree from then-current `origin/main`.

**Files:**
- Modify: `src/components/hex-grid/HexGrid.tsx`
- Modify: `src/components/hex-grid/HexEntity.tsx`
- Modify: `src/components/hex-grid/ClassCharacterModel.tsx`
- Modify: `src/components/hex-grid/MediumHumanoid.tsx`
- Modify: `src/components/hex-grid/PropModel.tsx`
- Modify: `src/components/playtest/playtestMapHelpers.ts`
- Create: `src/components/hex-grid/HexEntity.test.tsx`
- Create: `src/components/hex-grid/ClassCharacterModel.test.tsx`
- Create: `src/components/hex-grid/PropModel.test.tsx`
- Modify: `src/components/hex-grid/TurnOrderOverlay.test.tsx`

**Interfaces:**
- Consumes: Task 1 `SceneKnowledgeState`, `CRYPT_MEMORY_*`, `cloneCryptMaterials`, and exported `HexGridEntity`.
- Produces: `HexEntityProps.knowledgeState?: SceneKnowledgeState`, `ClassCharacterModelProps.remembered?: boolean`, `MediumHumanoidProps.remembered?: boolean`, `PropModelProps.remembered?: boolean`, and `RenderableEntity = HexGridEntity`.

- [ ] **Step 1: Create the issue and fresh worktree from latest main.**

Create the issue titled `Fog of War: frozen remembered entity rendering`, place it on Board 19 with Feature=`The Dungeon` and Team=`Assets`, fetch `origin/main`, create `/tmp/opencode/web-$ISSUE-fog-entities` on `feat/$ISSUE-fog-entities`, install dependencies, and run `npm test -- --run src/components/hex-grid/TurnOrderOverlay.test.tsx` to establish the focused baseline.

- [ ] **Step 2: Write failing entity inertness tests.**

Create `HexEntity.test.tsx` with a remembered player and remembered obstacle. Assert the R3F tree has no `onClick`/`onPointerOver` function on their hit groups, no selected material behavior, and that the remembered player passes neither movement path nor sequence into its model branch. Also assert an ordinary production ghost still mounts its existing stop-propagating no-op click handler. Add a `HexGrid` pure test where `['remembered-first', 'visible-active']` has `activeIndex: 1`; after filtering remembered IDs the order is `['visible-active']` and the recomputed active index is `0`.

```ts
expect(screenedEntityProps.knowledgeState).toBe('remembered');
expect(screenedEntityProps.isSelected).toBe(false);
expect(screenedEntityProps.movePath).toBeUndefined();
expect(screenedEntityProps.moveSeq).toBeUndefined();
```

- [ ] **Step 3: Run the focused test and confirm failure.**

Run: `npm test -- --run src/components/hex-grid/HexEntity.test.tsx`

Expected: FAIL because `knowledgeState` is not accepted and remembered renderer behavior does not exist.

- [ ] **Step 4: Make HexEntity derive remembered/inert rendering state.**

Add the optional prop and derive the state once:

```ts
export interface HexEntityProps {
  // Existing props unchanged.
  knowledgeState?: SceneKnowledgeState;
}

const remembered = knowledgeState === 'remembered';
const isInert = isDead || isGhost || remembered;
const renderedSelected = !remembered && !isDead && isSelected;
const renderedMovePath = remembered ? undefined : movePath;
const renderedMoveSeq = remembered ? undefined : moveSeq;
```

Branch pointer props by `remembered` first: remembered entities receive no pointer handlers at all; non-remembered dead/ghost entities keep the current stop-propagating no-op click handler; non-remembered live entities keep their existing click/hover/cursor behavior. Pass `renderedSelected`, `remembered`, `renderedMovePath`, and `renderedMoveSeq` into the class, medium, prop, loading placeholder, and primitive fallback paths. Keep `isGhost` unchanged for non-remembered entities. In `HexGrid`, pass `isSelected={false}` and no move inputs for remembered entities, suppress `SelfIndicatorRing` when `myEntity.knowledgeState === 'remembered'`, and recompute turn state as follows:

```ts
const originalActiveEntityId = combatState?.turnOrder?.[combatState.activeIndex]?.entityId;
const rememberedEntityIds = new Set(
  props.entities.filter((entity) => entity.knowledgeState === 'remembered').map((entity) => entity.entityId)
);
const turnOrder = (combatState?.turnOrder ?? [])
  .filter((entry) => !rememberedEntityIds.has(entry.entityId))
  .map((entry) => ({ entityId: entry.entityId, entityType: entry.entityType, initiative: entry.initiative }));
const activeIndex = originalActiveEntityId
  ? turnOrder.findIndex((entry) => entry.entityId === originalActiveEntityId)
  : -1;
```

- [ ] **Step 5: Freeze class models without demand-frame work.**

Extend `ClassCharacterModelProps` and its animation/material branch:

```ts
export interface ClassCharacterModelProps {
  url: string;
  isSelected?: boolean;
  isGhost?: boolean;
  remembered?: boolean;
  facingRotation?: number;
  isMoving?: boolean;
}

const resolvedClipName = remembered
  ? undefined
  : isMoving
    ? (resolveWalkClipName(names) ?? resolveIdleClipName(names))
    : resolveIdleClipName(names);

useFrame((state) => {
  if (!remembered && resolvedClipName) state.invalidate();
});
```

In the existing material effect, remembered has highest priority: clone every original material through `cloneCryptMaterials`, assign clones preserving scalar/array material shape, and dispose only those clones in cleanup. Do not set ghost opacity or selection emissive when remembered. Add a test with mocked `useGLTF`/`useAnimations` that remembered resolves no action and does not call the demand-frame invalidation path.

- [ ] **Step 6: Apply remembered presentation to MediumHumanoid, prop, and fallbacks.**

`MediumHumanoid` gains `remembered?: boolean`; it passes `selected: 0`, `ghostAmount: 0`, `showOutline: false`, and the opaque crypt palette into both textured and solid parts. Add `remembered` to `SolidCharacterPart` and `TexturedCharacterPart`; remembered materials use central opaque color/depth-write settings and their `useFrame` calls do not invalidate. `PropModel` gains `remembered?: boolean`, snapshots original materials from its clone, assigns `cloneCryptMaterials` only while remembered, restores originals, and disposes only created clones. The capsule fallback and `LoadingPlaceholder` take `remembered` and use an opaque crypt-charcoal `meshStandardMaterial` with `transparent={false}`, `opacity={1}`, and `depthWrite`; they must never fall back to live blue/red/purple colors.

Run:

```bash
npm test -- --run src/components/hex-grid/HexEntity.test.tsx src/components/hex-grid/ClassCharacterModel.test.tsx src/components/hex-grid/PropModel.test.tsx
```

Expected: PASS; live selection and ghost tests remain distinct from remembered tests.

- [ ] **Step 7: Unify the playtest entity type and verify turn-order filtering.**

Replace the duplicated interface in `playtestMapHelpers.ts` with:

```ts
import type { HexGridEntity } from '../hex-grid/HexGrid';
export type RenderableEntity = HexGridEntity;
```

Keep `buildRenderableEntities` output behavior unchanged, which means omitted `knowledgeState` remains visible. Add a `TurnOrderOverlay`-adjacent test through the new HexGrid turn-order helper showing remembered IDs are absent; do not change `TurnOrderOverlay`’s public props.

- [ ] **Step 8: Run focused suites and commit the entity slice.**

```bash
npm test -- --run src/components/hex-grid/HexEntity.test.tsx src/components/hex-grid/ClassCharacterModel.test.tsx src/components/hex-grid/PropModel.test.tsx src/components/hex-grid/TurnOrderOverlay.test.tsx src/components/playtest/playtestMapHelpers.test.ts
git status --short
git diff --check
npm run ci-check
git add src/components/hex-grid/HexGrid.tsx src/components/hex-grid/HexEntity.tsx src/components/hex-grid/ClassCharacterModel.tsx src/components/hex-grid/MediumHumanoid.tsx src/components/hex-grid/PropModel.tsx src/components/playtest/playtestMapHelpers.ts src/components/hex-grid/HexEntity.test.tsx src/components/hex-grid/ClassCharacterModel.test.tsx src/components/hex-grid/PropModel.test.tsx src/components/hex-grid/TurnOrderOverlay.test.tsx
git commit -m "feat: render remembered entities inertly"
```

Expected: focused tests and CI pass; commit contains only Task 2 files before push/PR.

## Task 3: Typed viewer projections and fail-closed reducer

**Issue/branch setup:** After Task 2 merges, create a Board 19 issue with Feature=`The Dungeon`, Team=`UI/UX`; begin a fresh worktree from latest `origin/main`.

**Files:**
- Create: `src/concepts/fog-of-war/fixtures.ts`
- Create: `src/concepts/fog-of-war/reducer.ts`
- Create: `src/concepts/fog-of-war/adapter.ts`
- Create: `src/concepts/fog-of-war/fixtures.test.ts`
- Create: `src/concepts/fog-of-war/reducer.test.ts`
- Create: `src/concepts/fog-of-war/adapter.test.ts`

**Interfaces:**
- Consumes: `AbsoluteFloorTile`, generated `Wall`, Task 1 `SceneKnowledgeState`, and Task 1 `HexGridEntity`.
- Produces: `ViewerSceneProjection`, `WorldTruthFixture`, discriminated `FogConceptStep`, `FogOfWarState`, `fogOfWarReducer`, `sanitizeProjection`, and `toHexGridProps`.

- [ ] **Step 1: Create the issue/worktree and write the failing projection contract tests.**

Create the UI/UX issue titled `Fog of War: typed viewer projection fixtures`, add it to Board 19, create its fresh worktree, then write `fixtures.test.ts` expecting seven named control steps: projection steps `room1-visible`, `door-reveal`, `room2-with-room1-memory`, `reconnect`, `resight-room1`, and `second-viewer`; plus the sole world-truth step `hidden-change-inspector`. Assert that no viewer projection record uses an unseen state, that `hidden-change-inspector` has no `projection` property, and that the second viewer’s Room 2 records are absent before its own authored reveal.

Use this discriminated shape in the test import:

```ts
export type ViewerRecord =
  | { kind: 'floor'; id: string; roomId: string; knowledgeState: SceneKnowledgeState; tile: AbsoluteFloorTile }
  | { kind: 'wall'; id: string; roomId: string; knowledgeState: SceneKnowledgeState; wall: Wall }
  | { kind: 'entity'; id: string; roomId: string; knowledgeState: SceneKnowledgeState; entity: HexGridEntity };
```

- [ ] **Step 2: Run fixture tests and confirm the missing-export failure.**

Run: `npm test -- --run src/concepts/fog-of-war/fixtures.test.ts`

Expected: FAIL because the fixture module and typed scenarios do not exist.

- [ ] **Step 3: Create typed fixtures and strictly isolate world truth.**

Create `fixtures.ts` with `ViewerSceneProjection { viewerId: string; currentRoomId: string; records: ViewerRecord[] }` and a separate `WorldTruthRecord` union with the same real floor/wall/entity payloads but no `knowledgeState`, then `WorldTruthFixture { rooms: Record<string, { label: string; records: WorldTruthRecord[] }> }`. Define the exact control model:

```ts
export type FogConceptStep =
  | { kind: 'projection'; id: string; label: string; action: 'hydrate' | 'replaceProjection'; projection: ViewerSceneProjection }
  | { kind: 'world-truth'; id: 'hidden-change-inspector'; label: string; worldTruth: WorldTruthFixture };
```

Export `FOG_CONCEPT_STEPS: FogConceptStep[]`. `room1-visible` uses `action: 'hydrate'`; door/reveal, Room 2 memory, reconnect, re-sight, and second viewer use `action: 'replaceProjection'`; `hidden-change-inspector` is the only `kind: 'world-truth'` entry. `WorldTruthFixture` is exported only for inspector display and fixture comparison; it is not accepted by reducer or adapter types. Construct all maps with real `AbsoluteFloorTile`, generated `Wall`, and `HexGridEntity` values. The `room2-with-room1-memory` projection contains Room 1 records with `'remembered'` and Room 2 records with `'visible'` together. The hidden-change fixture changes only `WORLD_TRUTH_AFTER_HIDDEN_CHANGE`; it has no viewer projection or reducer action.

- [ ] **Step 4: Write reducer tests before the reducer.**

Create `reducer.test.ts`:

```ts
import { describe, expect, it } from 'vitest';
import { ROOM2_WITH_ROOM1_MEMORY, RESIGHT_ROOM1 } from './fixtures';
import { fogOfWarReducer, initialFogOfWarState } from './reducer';

describe('fogOfWarReducer', () => {
  it('does not change memory when hidden world truth changes outside a dispatch', () => {
    const state = fogOfWarReducer(initialFogOfWarState, { type: 'hydrate', projection: ROOM2_WITH_ROOM1_MEMORY });
    expect(state.projection).toEqual(ROOM2_WITH_ROOM1_MEMORY);
  });
  it('atomically replaces remembered Room 1 with re-sighted current truth', () => {
    const remembered = fogOfWarReducer(initialFogOfWarState, { type: 'hydrate', projection: ROOM2_WITH_ROOM1_MEMORY });
    const next = fogOfWarReducer(remembered, { type: 'replaceProjection', projection: RESIGHT_ROOM1 });
    expect(next.projection).toEqual(RESIGHT_ROOM1);
    expect(next.projection.records.some((r) => r.roomId === 'room-1' && r.knowledgeState === 'remembered')).toBe(false);
  });
  it('returns the same state reference for malformed top-level projection', () => {
    const state = fogOfWarReducer(initialFogOfWarState, { type: 'hydrate', projection: ROOM2_WITH_ROOM1_MEMORY });
    const malformed = { ...ROOM2_WITH_ROOM1_MEMORY, viewerId: '', records: [] } as unknown as ViewerSceneProjection;
    expect(fogOfWarReducer(state, { type: 'replaceProjection', projection: malformed })).toBe(state);
  });
  it('omits malformed remembered records rather than coercing them live', () => {
    const malformed = { ...ROOM2_WITH_ROOM1_MEMORY, records: [...ROOM2_WITH_ROOM1_MEMORY.records, { kind: 'entity', id: '', roomId: 'room-1', knowledgeState: 'remembered', entity: {} }] } as unknown as ViewerSceneProjection;
    const next = fogOfWarReducer(initialFogOfWarState, { type: 'hydrate', projection: malformed });
    expect(next.projection?.records.some((record) => record.id === '')).toBe(false);
  });
});
```

- [ ] **Step 5: Implement the two-action fail-closed reducer.**

Create `reducer.ts`:

```ts
export interface FogOfWarState { projection: ViewerSceneProjection | null; }
export const initialFogOfWarState: FogOfWarState = { projection: null };
export type FogOfWarAction =
  | { type: 'hydrate'; projection: ViewerSceneProjection }
  | { type: 'replaceProjection'; projection: ViewerSceneProjection };

function isValidViewerRecord(record: ViewerRecord): boolean {
  if (!record.id || !record.roomId || (record.knowledgeState !== 'visible' && record.knowledgeState !== 'remembered')) return false;
  if (record.kind === 'floor') {
    const { x, y, z } = record.tile;
    return Number.isFinite(x) && Number.isFinite(y) && Number.isFinite(z) && x + y + z === 0;
  }
  if (record.kind === 'wall') return record.wall.from !== undefined && record.wall.to !== undefined;
  const { entity } = record;
  if (!entity || !entity.entityId || !entity.name || !entity.position) return false;
  return Number.isFinite(entity.position.x) && Number.isFinite(entity.position.y) && Number.isFinite(entity.position.z);
}
export function sanitizeProjection(
  projection: ViewerSceneProjection
): ViewerSceneProjection | null {
  if (!projection.viewerId || !projection.currentRoomId || !Array.isArray(projection.records)) {
    return null;
  }
  return {
    viewerId: projection.viewerId,
    currentRoomId: projection.currentRoomId,
    records: projection.records.filter(isValidViewerRecord),
  };
}
export function fogOfWarReducer(state: FogOfWarState, action: FogOfWarAction): FogOfWarState {
  const projection = sanitizeProjection(action.projection);
  return projection === null ? state : { ...state, projection };
}
```

`sanitizeProjection(projection: ViewerSceneProjection): ViewerSceneProjection | null` returns `null` when top-level `viewerId`/`currentRoomId` is empty or `records` is not an array. For a valid top level it returns a new projection whose records pass all checks: non-empty `id`/`roomId`; floor coordinates are finite and satisfy `x + y + z === 0`; wall has `from` and `to`; entity has non-empty id/name and finite position; `knowledgeState` is exactly `'visible'` or `'remembered'`. Invalid nested records are omitted. It never converts a missing/invalid state to visible; malformed remembered records are omitted. `fogOfWarReducer` preserves the existing state reference on `null`. There are no other action variants, and no reducer parameter accepts `WorldTruthFixture`.

- [ ] **Step 6: Write adapter tests, then implement exact HexGrid inputs.**

Create `adapter.test.ts` asserting that `toHexGridProps(ROOM2_WITH_ROOM1_MEMORY)` returns both rooms in `floorTiles`, Room 1 keys in `rememberedFloorHexKeys`/`rememberedWallHexKeys`, remembered entities with `knowledgeState: 'remembered'`, no unseen record, and `showFrontierGroundHints: false`.

Implement:

```ts
export interface FogHexGridProps {
  floorTiles: Map<string, AbsoluteFloorTile>;
  rememberedFloorHexKeys: ReadonlySet<string>;
  walls: Wall[];
  rememberedWallHexKeys: ReadonlySet<string>;
  entities: HexGridEntity[];
  showFrontierGroundHints: false;
}
export function toHexGridProps(projection: ViewerSceneProjection): FogHexGridProps {
  const floorTiles = new Map<string, AbsoluteFloorTile>();
  const rememberedFloorHexKeys = new Set<string>();
  const walls: Wall[] = [];
  const rememberedWallHexKeys = new Set<string>();
  const entities: HexGridEntity[] = [];
  for (const record of projection.records) {
    if (record.kind === 'floor') {
      const key = `${record.tile.x},${record.tile.y},${record.tile.z}`;
      floorTiles.set(key, record.tile);
      if (record.knowledgeState === 'remembered') rememberedFloorHexKeys.add(key);
    } else if (record.kind === 'wall') {
      walls.push(record.wall);
      if (record.knowledgeState === 'remembered' && record.wall.from) {
        rememberedWallHexKeys.add(`${record.wall.from.x},${record.wall.from.y},${record.wall.from.z}`);
      }
    } else {
      entities.push({ ...record.entity, knowledgeState: record.knowledgeState });
    }
  }
  return { floorTiles, rememberedFloorHexKeys, walls, rememberedWallHexKeys, entities, showFrontierGroundHints: false };
}
```

The explicit `record.kind` branch uses only sanitized records, adds full known geometry to the normal map/array, adds remembered record coordinate keys to the matching set, and never filters records by `currentRoomId`. For wall membership use `wall.from` as the wall-hex key, matching door and Synty segment ownership. It returns no combat callbacks, no door callback, and no production stream data.

- [ ] **Step 7: Run the contract test suite and commit Task 3.**

```bash
npm test -- --run src/concepts/fog-of-war/fixtures.test.ts src/concepts/fog-of-war/reducer.test.ts src/concepts/fog-of-war/adapter.test.ts
git status --short
git diff --check
npm run ci-check
git add src/concepts/fog-of-war/fixtures.ts src/concepts/fog-of-war/reducer.ts src/concepts/fog-of-war/adapter.ts src/concepts/fog-of-war/fixtures.test.ts src/concepts/fog-of-war/reducer.test.ts src/concepts/fog-of-war/adapter.test.ts
git commit -m "feat: add fog of war viewer fixtures"
```

Expected: tests prove no leaks, hidden mutation isolation, atomic re-sight, and the exact mixed-scene adapter output; CI passes before push/PR.

## Task 4: Fog of War concept page, documentation, and visual evidence

**Issue/branch setup:** After Task 3 merges, create a Board 19 issue with Feature=`The Dungeon`, Team=`UI/UX`; use a fresh worktree from latest `origin/main`.

**Files:**
- Create: `src/concepts/fog-of-war/FogOfWarConcept.tsx`
- Create: `src/concepts/fog-of-war/FogOfWarConcept.test.tsx`
- Create: `src/concepts/fog-of-war/CONTRACT.md`
- Create: `src/concepts/README.md`
- Modify: `src/App.tsx`
- Create: `src/App.test.tsx`
- Modify: `src/concepts/ConceptsView.tsx`
- Create: `src/concepts/ConceptsView.test.tsx`
- Modify: `docs/how-to/concepts-route.md`

**Interfaces:**
- Consumes: Task 3 `FOG_CONCEPT_STEPS`, `FogConceptStep`, `WorldTruthFixture`, `fogOfWarReducer`, and `toHexGridProps`; Task 1/2 `HexGrid` presentation seam.
- Produces: a registered `/concepts` Fog of War page that dispatches only projection control steps, exposes clear fixture evidence, and supports a dev-only evidence query seam.

- [ ] **Step 1: Create the issue/worktree and write failing page-navigation tests.**

Create the UI/UX issue titled `Fog of War: executable two-room concept`, add it to Board 19, create the fresh worktree, then create `FogOfWarConcept.test.tsx` with mocked `HexGrid`. Assert the default is `room1-visible`; clicking `room2-with-room1-memory` shows labels `Viewer projection` and `World truth inspector`, passes both Room 1 and Room 2 tiles to the mock, passes `showFrontierGroundHints={false}`, and reports remembered record count. Capture the mock's serialized viewer props, select `hidden-change-inspector`, and assert the serialized viewer props are unchanged while the world-truth inspector changes. Create `src/concepts/ConceptsView.test.tsx`; render `ConceptsView`, click `Fog of War`, and assert the mocked `FogOfWarConcept` branch renders. Add an `App` test at `src/App.test.tsx` proving development `?concept=fog-of-war&fogStep=resight-room1` opens Concepts Lab, while no query preserves normal initial view.

- [ ] **Step 2: Run the page test and confirm failure.**

Run: `npm test -- --run src/concepts/fog-of-war/FogOfWarConcept.test.tsx`

Expected: FAIL because the page and ConceptsView registration do not exist.

- [ ] **Step 3: Build the responsive real-HexGrid concept page.**

Implement a reducer-driven component:

```tsx
export function FogOfWarConcept({ initialStepId }: { initialStepId?: string }) {
  const initialStep = FOG_CONCEPT_STEPS.find((step) => step.kind === 'projection' && step.id === initialStepId) ?? FOG_CONCEPT_STEPS[0]!;
  const initialProjection = initialStep.kind === 'projection' ? initialStep.projection : ROOM1_VISIBLE;
  const [stepId, setStepId] = useState(initialStep.id);
  const [worldTruth, setWorldTruth] = useState(WORLD_TRUTH_INITIAL);
  const [state, dispatch] = useReducer(
    fogOfWarReducer,
    initialProjection,
    (projection) => fogOfWarReducer(initialFogOfWarState, { type: 'hydrate', projection })
  );
  const selectStep = (nextId: string) => {
    const next = FOG_CONCEPT_STEPS.find((step) => step.id === nextId) ?? FOG_CONCEPT_STEPS[0]!;
    setStepId(next.id);
    if (next.kind === 'world-truth') {
      setWorldTruth(next.worldTruth);
      return;
    }
    dispatch({ type: next.action, projection: next.projection });
  };
  const grid = state.projection ? toHexGridProps(state.projection) : null;
  return grid ? <HexGrid {...grid} syntyDungeon spaceTheme="crypt" isPlayerTurn={false} combatState={null} /> : null;
}
```

`FogOfWarConcept` accepts `initialStepId?: string`; it is used only by the dev query seam and tests. `selectStep` dispatches only `kind: 'projection'` steps. `hidden-change-inspector` updates the separate `worldTruth` state and leaves the viewer projection reference/adapter output unchanged. In `App.tsx`, only when `import.meta.env.DEV` and `new URLSearchParams(window.location.search).get('concept') === 'fog-of-war'`, initialize `currentView` to Concepts Lab. In `ConceptsView.tsx`, parse `fogStep` only in development, initialize the active page to `'fog-of-war'` only for that same concept query, and pass `initialStepId` to `FogOfWarConcept`; normal in-app navigation and all non-development behavior are unchanged. Provide named controls for the full authored sequence: Room 1 visible, Door/reveal, Room 2 with Room 1 remembered, Hidden change inspector, Reconnect, Re-sight Room 1, and Second viewer. The layout uses a responsive grid (`grid-template-columns: repeat(auto-fit, minmax(18rem, 1fr))`): the WebGL panel has `minHeight: 32rem` desktop and `minHeight: 24rem` at narrow width; inspectors use accessible headings and code-like record counts. Clearly label that viewer projection is renderer input and world truth is comparison-only.

- [ ] **Step 4: Register the concept and write the concepts policy.**

In `ConceptsView.tsx`, import `FogOfWarConcept`, add `'fog-of-war'` to `ConceptPage`, add `{ id: 'fog-of-war', label: 'Fog of War' }` to `CONCEPT_PAGES`, and render `<FogOfWarConcept />` in the active branch. Create `src/concepts/README.md` stating exactly that concepts use real game components with representative fixtures; are executable outside-in contract experiments rather than visual showcase mocks; fixtures describe desired consumer data; `CONTRACT.md` holds evidence/candidate gaps; Kirk reviews before gaps become cross-repo requests; and approved concepts drive toolkit/proto/API/production-web work from a known consumer contract.

- [ ] **Step 5: Add an evidence-only contract log and route documentation.**

Create `src/concepts/fog-of-war/CONTRACT.md` with numbered evidence, not requests: additive-only `GeometryRevealed`; API `visibleNow` not delivered as current geometry; unconditional snapshot wall/door leakage; entity-only ghosting and no remembered reconnect entities; and toolkit `View`’s reserved `KnownEntities`. State that the fixtures intentionally model desired consumer data, no platform issue is created by this concept, and Kirk review precedes any request. Add a Fog of War row to `docs/how-to/concepts-route.md` saying it is a two-room viewer-projection contract bench using real HexGrid/Synty rendering and documents the development-only query seam for reproducible captures.

- [ ] **Step 6: Run page and fixture tests, then perform desktop/mobile WebGL review.**

```bash
npm test -- --run src/App.test.tsx src/concepts/ConceptsView.test.tsx src/concepts/fog-of-war/FogOfWarConcept.test.tsx src/concepts/fog-of-war/fixtures.test.ts src/concepts/fog-of-war/reducer.test.ts src/concepts/fog-of-war/adapter.test.ts
npm run dev -- --host 127.0.0.1
```

With the server running, capture the query-selected real Concepts Lab states with the existing harness:

```bash
node /home/kirk/game-dev/tools/browser/screenshot.mjs "http://127.0.0.1:5173/?concept=fog-of-war&fogStep=room2-with-room1-memory" /tmp/fog-room2-memory-desktop.png 5000 1440 900
node /home/kirk/game-dev/tools/browser/screenshot.mjs "http://127.0.0.1:5173/?concept=fog-of-war&fogStep=room2-with-room1-memory" /tmp/fog-room2-memory-mobile.png 5000 390 844
node /home/kirk/game-dev/tools/browser/screenshot.mjs "http://127.0.0.1:5173/?concept=fog-of-war&fogStep=resight-room1" /tmp/fog-resight-desktop.png 5000 1440 900
node /home/kirk/game-dev/tools/browser/screenshot.mjs "http://127.0.0.1:5173/?concept=fog-of-war&fogStep=resight-room1" /tmp/fog-resight-mobile.png 5000 390 844
```

Inspect: Room 1 has no pre-observation content initially; remembered Room 1 and visible Room 2 render together; remembered floor/wall/door/entity is opaque crypt-charcoal with no hover/cursor/action affordance; hidden change modifies only the world-truth inspector; reconnect restores remembered content; re-sight removes stale remembered presentation and shows current truth; second viewer remains isolated; no frontier hint appears beyond unknown geometry. Attach these images to the Task 4 PR; do not commit them.

- [ ] **Step 7: Run full CI, review evidence, and commit Task 4.**

```bash
npm run ci-check
git status --short
git diff --check
git add src/App.tsx src/App.test.tsx src/concepts/fog-of-war/FogOfWarConcept.tsx src/concepts/fog-of-war/FogOfWarConcept.test.tsx src/concepts/fog-of-war/CONTRACT.md src/concepts/README.md src/concepts/ConceptsView.tsx src/concepts/ConceptsView.test.tsx docs/how-to/concepts-route.md
git commit -m "feat: add fog of war concept"
```

Expected: full CI passes, the commit contains only Task 4 files, and the visual evidence is attached to the implementation PR rather than added as an unrelated repository artifact.

## Final Acceptance Sweep

- [ ] Confirm all four implementation PRs were issue-first, Board-19 classified, independently reviewed, and merged in order from latest `main`.
- [ ] Confirm omitted `knowledgeState`, remembered key sets, and `showFrontierGroundHints` leave production defaults unchanged; no production encounter stream/reducer file changed.
- [ ] Confirm unseen is absent from fixtures, adapter output, floor/wall/entity renderer inputs, and visual captures.
- [ ] Confirm mixed Room 1 remembered and Room 2 visible geometry render concurrently; no `currentRoomId` filtering removes remembered Room 1.
- [ ] Confirm remembered material precedence, opaque `transparent=false`/`depthWrite=true` treatment, safe GLTF/texture cache handling, material-array restoration, disposal ownership, no remembered animation or demand invalidation, and stable shaded instanced-floor ordering.
- [ ] Confirm remembered Synty segments use the portion before `->`, fittings inspect all `|`-joined keys, and remembered door/frame/end/fitting pieces have crypt treatment with no handlers.
- [ ] Confirm remembered entities never enter pathing, occupancy, hover, selection, targeting, self indicator, or turn order; existing ghost behavior remains pale-cyan and separate.
- [ ] Confirm reducer accepts only full hydrate/replace projections, preserves the current state reference for invalid top-level projections, sanitizes malformed nested records by omission, has no world-truth input, and never derives LOS, reveal, memory, or hidden mutations.
- [ ] Confirm desktop and mobile real-WebGL evidence covers Room 1 visible, door/reveal, Room 2 plus Room 1 memory, hidden-change isolation, reconnect, re-sight, and second-viewer isolation.
- [ ] Confirm `src/concepts/README.md` and evidence-only fog `CONTRACT.md` state the approved concept workflow and have not created a platform request.

## Out of Scope Follow-ups

The approved concept is the consumer contract only. Separate, later issue-first work may: teach toolkit rules and encounter-data serialization to retain complete per-viewer knowledge; add proto snapshot/transition shapes; have API durably store and project only authorized current/remembered records; remove unconditional wall/door leakage; restore remembered knowledge on reconnect; and promote the proven web seam into the production encounter route. Intelligence, senses, memory fidelity/retention, and alternative palettes such as forest sepia remain future design work and do not change this plan's two-state renderer seam.
