# Fog of War Concept Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver a playable single-viewer `/concepts` Fog of War crypt that defines the hex knowledge event layer — the draft proto contract — with real HexGrid/Synty components, without changing production encounter behavior or cross-repo contracts.

**Architecture:** Two halves separated by an event boundary. An **authority** holds authored world truth and a crude line of sight, reconciles visibility after each move intent, and emits per-viewer `HexKnowledgeChanged` events; it stands in for work the toolkit and API will own. A **consumer** — reducer keyed by hex, adapter to HexGrid props, real renderer — applies only those events and has no other input. Walking the map is what proves the contract; the rendered scene is built solely from emitted events, never from world truth.

**Tech Stack:** TypeScript 5.8, React 19, React Three Fiber 9, Three.js 0.181, `@react-three/drei`, Vitest 4, `@testing-library/react`, and `@react-three/test-renderer`. No dependency additions.

## Global Constraints

- This plan is canonical at `rpg-project/ideas/fog-of-war/plan.md`, adjacent to the approved `design.md`; implementation occurs only in `rpg-dnd5e-web`.
- **Tasks 1-2 are one issue and one PR each; Tasks 3-5 share one branch and one PR.** The split is not arbitrary: Tasks 1-2 modify shared production components (`HexGrid`, `HexEntity`, the Synty renderers), which are reusable on their own and correctly merge independently — Task 1 already did, as #602. Tasks 3-5 are entirely `src/concepts/fog-of-war/`. That is one concept, inert in pieces, and merging a third of it delivers nothing.
- The deeper reason is that the concept's job is to prove the contract wrong. If the event layer merges and then playing the concept shows a record shape is mistaken, the correction becomes a follow-up PR against merged history instead of an edit to an unmerged branch — precisely the downstream cost that working outside-in exists to avoid. Keep corrections as edits.
- The concept branch is `feat/605-fog-events`, PR rpg-dnd5e-web#611. Tasks 4 and 5 land as further commits on it. It merges once, when the concept runs.
- Before each task: create or re-scope its `rpg-dnd5e-web` issue, add it to Board 19, set Feature=`The Dungeon`, set Team=`Assets` for Task 2 and Team=`UI/UX` for Tasks 3-5. Use the issue number returned by `gh issue create`; never invent one. Issues #605 and #606 already exist and are re-scoped rather than duplicated. The concept PR closes all three concept issues when it merges.
- **A seam is a review point, not a merge point.** Tasks 3, 4, and 5 each end by pushing and stopping for review. The event layer is what the protos transcribe, so it is read before an authority exists to feed it, and the authority is read before the page makes it pretty. Do not merge to satisfy a seam.
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
- The event layer is defined in `design.md` §"The event layer" and is the draft proto contract. One per-viewer `HexKnowledgeChanged` carries `hexes: [HexRecord]` and `entities: [FogEntity]`. A `HexRecord` is `position`, `state` (`VISIBLE | REMEMBERED`), `terrain`, `zoneId`, `edges: [WallLike]`, `contents: [Placement]`. A `Placement` is `entityId` + `facing`. Do not add fields the concept does not consume.
- A `VISIBLE` record is **total**: `contents: []` positively means empty. Re-sight replaces the held record wholesale — never a merge, never a field-level update. This is what deletes a remembered occupant, so it is not an optimization to soften later.
- **A record is an observation; an entity is current disclosure.** Anything that must stay frozen in a viewer's memory belongs on the record — facing above all, because two viewers who saw the same entity face different ways must keep different memories of it. Anything reflecting what the thing currently is belongs on the entity. Do not move `facing` onto `FogEntity`.
- **Nothing is ever deleted.** There is no `GONE` state, no removal transition, and no tombstone. A witnessed removal is a `VISIBLE` record that no longer lists the thing; a hidden removal is the absence of any record. Knowledge only grows or gets replaced. Do not add a delete path "for completeness" — deletion is the one operation a later observation cannot correct.
- **A `REMEMBERED` record carries its full frozen observation** rather than instructing the client to freeze what it holds. Live transitions and reconnect hydration are therefore the same code path, and the client's only behavior is merge-by-hex-key.
- The reducer's only input is events. It has no world-truth parameter, no LOS, no reveal or door-open action, and no derivation. Knowledge is a map keyed by hex. Applying the same record twice must leave state identical. A placement whose `entityId` is not in the viewer's disclosed entity set is dropped.
- Only the authority reads world truth, and no consumer-side module may import from `authority/`. This is enforced by `boundary.test.ts`, not by convention. The authority exists to produce the event stream; if it grows rules beyond that, stop and re-read `design.md` §"Concept architecture".
- Movement animation is out of scope. The authority moves entities between hexes discretely. Do not add interpolation, easing, or path tweening to reach a nicer-looking result.
- Type `*Like` interfaces against the generated v1alpha2 messages field-for-field rather than importing generated classes, following `src/concepts/combat-pacing/fixtures.ts`. Document every deliberate divergence at its field.
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
| `src/concepts/fog-of-war/events.ts` | The event layer: `HexKnowledgeChanged`, `HexRecord`, `Placement`, `FogEntity`, `hexKey`. The draft proto contract. |
| `src/concepts/fog-of-war/reducer.ts` | One viewer's knowledge keyed by hex. Applies events; no other input. |
| `src/concepts/fog-of-war/adapter.ts` | Reducer state to exact HexGrid props; unseen omitted. |
| `src/concepts/fog-of-war/authority/{world,los,authority}.ts` | Authored world truth, crude hex LOS, and visibility reconciliation emitting per-viewer events. The only code permitted to read world truth. |
| `src/concepts/fog-of-war/boundary.test.ts` | Asserts no consumer module imports from `authority/`. Keeps client-side LOS out permanently. |
| `src/concepts/fog-of-war/FogOfWarConcept.tsx` | Responsive playable page mounting a real crypt `HexGrid`; move intents in, events out. |
| `src/concepts/fog-of-war/{events,reducer,adapter,FogOfWarConcept}.test.ts[x]` | Contract, reducer, adapter, and page behavior coverage. |
| `src/{App.tsx,concepts/ConceptsView.tsx,concepts/README.md}` | Dev-only query selection, Fog-of-war registration, and concepts-wide executable-contract policy. |
| `src/concepts/fog-of-war/CONTRACT.md` | Evidence and candidate gaps only; no platform request. |

## Task 1: Shared knowledge contract and inert mixed geometry — ✅ SHIPPED

**Delivered by rpg-dnd5e-web#601 / PR #602** (`b835c29`, merged 2026-07-25).
`src/components/hex-grid/sceneKnowledge.ts` exists with `SceneKnowledgeState`,
the `CRYPT_MEMORY_*` constants, `isRemembered`, `rememberedSegment`,
`rememberedFitting`, and `cloneCryptMaterials`; `HexGrid` accepts
`rememberedFloorHexKeys`, `rememberedWallHexKeys`, and `knowledgeState` on
entities, and remembered wall runs render in crypt memory.

The steps below are retained as the record of what was built. Do not re-execute
them. The remaining work starts at Task 2.

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

## Task 3: Event layer types, hex reducer, and adapter

This is the contract seam. It is pure TypeScript with no rendering, no world
truth, and no authority — the consumer half of the concept, built and reviewed
before anything exists to produce its input. Hand-authored events stand in for
the authority here, which is what makes the reducer's isolation testable.

**Issue/branch setup:** Update `rpg-dnd5e-web#605` ("Fog of War: typed viewer
projection fixtures") rather than filing a duplicate — its scope is replaced by
the event layer, so rewrite the body against `design.md` §"The event layer" and
retitle it `Fog of War: hex knowledge event layer and reducer`. Confirm Board 19
carries Feature=`The Dungeon`, Team=`UI/UX`. Start a fresh worktree from latest
`origin/main`.

**Files:**
- Create: `src/concepts/fog-of-war/events.ts`
- Create: `src/concepts/fog-of-war/events.test.ts`
- Create: `src/concepts/fog-of-war/reducer.ts`
- Create: `src/concepts/fog-of-war/reducer.test.ts`
- Create: `src/concepts/fog-of-war/adapter.ts`
- Create: `src/concepts/fog-of-war/adapter.test.ts`

**Interfaces:**
- Consumes: `AbsoluteFloorTile` (`@/hooks/dungeonMapGeometry`), exported
  `HexGridEntity` (`src/components/hex-grid/HexGrid.tsx`), `SceneKnowledgeState`
  (`src/components/hex-grid/sceneKnowledge.ts`).
- Produces: `HexKnowledgeChanged`, `HexRecord`, `Placement`, `FogEntity`,
  `fogReducer`, `emptyKnowledge`, `toHexGridProps`.

Follow the `combat-pacing`/`equipment` precedent: declare `*Like` interfaces
matching the generated v1alpha2 messages field-for-field rather than importing
generated classes, and document every deliberate divergence at its field. See
`src/concepts/combat-pacing/fixtures.ts`'s file header for the established
rationale.

- [ ] **Step 1: Update the issue and create the worktree.**

```bash
gh issue edit 605 --repo KirkDiggler/rpg-dnd5e-web \
  --title "Fog of War: hex knowledge event layer and reducer"
# Rewrite the body against design.md §"The event layer"; end it exactly:
#   — asset-pipeline agent, on behalf of KirkDiggler
git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin main
git -C /home/kirk/game-dev/rpg-dnd5e-web worktree add \
  "/tmp/opencode/web-605-fog-events" -b "feat/605-fog-events" origin/main
npm --prefix "/tmp/opencode/web-605-fog-events" install
```

Expected: issue 605 retitled and re-scoped; clean worktree on `feat/605-fog-events`.

- [ ] **Step 2: Write the failing reducer tests.**

Create `reducer.test.ts`. These eight cases are the contract; each maps to a row
of `design.md` §"Required behavior".

```ts
import { describe, expect, it } from 'vitest';
import { emptyKnowledge, fogReducer } from './reducer';
import type { HexKnowledgeChanged } from './events';

const at = (q: number, r: number) => ({ x: q, y: r, z: -q - r });
const goblin = { entityId: 'goblin-1', name: 'Goblin', type: 'monster' as const };

const visible = (q: number, r: number, contents = [] as { entityId: string; facing: number }[]) => ({
  position: at(q, r),
  state: 'VISIBLE' as const,
  terrain: 0,
  zoneId: '',
  edges: [],
  contents,
});

describe('fog reducer', () => {
  it('first sight adds a visible hex', () => {
    const next = fogReducer(emptyKnowledge(), { hexes: [visible(0, 0)], entities: [] });
    expect(next.hexes.get('0,0,0')?.state).toBe('VISIBLE');
  });

  it('a remembered record carries its own frozen observation', () => {
    const seen = fogReducer(emptyKnowledge(), {
      hexes: [visible(0, 0, [{ entityId: 'goblin-1', facing: 0 }])],
      entities: [goblin],
    });
    const lost = fogReducer(seen, {
      hexes: [{ ...visible(0, 0, [{ entityId: 'goblin-1', facing: 0 }]), state: 'REMEMBERED' }],
      entities: [],
    });
    // The server sends what the viewer observed; the client never freezes.
    expect(lost.hexes.get('0,0,0')?.state).toBe('REMEMBERED');
    expect(lost.hexes.get('0,0,0')?.contents).toEqual([{ entityId: 'goblin-1', facing: 0 }]);
    expect(lost.entities.get('goblin-1')).toBeDefined();
  });

  it('re-sight replaces memory wholesale, deleting a remembered occupant', () => {
    // design.md case 8 — the load-bearing one.
    const remembered = fogReducer(emptyKnowledge(), {
      hexes: [{ ...visible(0, 0, [{ entityId: 'goblin-1', facing: 0 }]), state: 'REMEMBERED' }],
      entities: [goblin],
    });
    const resighted = fogReducer(remembered, { hexes: [visible(0, 0)], entities: [] });
    expect(resighted.hexes.get('0,0,0')?.state).toBe('VISIBLE');
    expect(resighted.hexes.get('0,0,0')?.contents).toEqual([]);
  });

  it('a hidden mutation changes nothing', () => {
    const before = fogReducer(emptyKnowledge(), {
      hexes: [{ ...visible(1, 0), state: 'REMEMBERED' }],
      entities: [],
    });
    const after = fogReducer(before, { hexes: [], entities: [] });
    expect(after.hexes.get('1,0,-1')).toEqual(before.hexes.get('1,0,-1'));
  });

  it('freezes the facing that was observed, not one seen later', () => {
    // This viewer saw the goblin facing 0 and lost sight. The goblin later
    // turned and another viewer saw that. Facing lives on the placement, so
    // no other viewer's sighting can rewrite this viewer's memory.
    const remembered = fogReducer(emptyKnowledge(), {
      hexes: [{ ...visible(0, 0, [{ entityId: 'goblin-1', facing: 0 }]), state: 'REMEMBERED' }],
      entities: [goblin],
    });
    // The goblin is re-disclosed to this viewer, but no record arrives for the
    // remembered hex — nothing about the memory may move.
    const later = fogReducer(remembered, { hexes: [], entities: [goblin] });
    expect(later.hexes.get('0,0,0')?.contents).toEqual([{ entityId: 'goblin-1', facing: 0 }]);
  });

  it('applying the same record twice is idempotent', () => {
    const event: HexKnowledgeChanged = { hexes: [visible(0, 0)], entities: [] };
    const once = fogReducer(emptyKnowledge(), event);
    expect(fogReducer(once, event)).toEqual(once);
  });

  it('drops a placement whose entity is not disclosed', () => {
    const next = fogReducer(emptyKnowledge(), {
      hexes: [visible(0, 0, [{ entityId: 'never-disclosed', facing: 0 }])],
      entities: [],
    });
    expect(next.hexes.get('0,0,0')?.contents).toEqual([]);
  });

  it('is a pure function of the events applied', () => {
    const events: HexKnowledgeChanged[] = [
      { hexes: [visible(0, 0, [{ entityId: 'goblin-1', facing: 2 }])], entities: [goblin] },
      { hexes: [{ ...visible(0, 0), state: 'REMEMBERED' }], entities: [] },
      { hexes: [visible(0, 0)], entities: [] },
    ];
    const replay = events.reduce(fogReducer, emptyKnowledge());
    const again = events.reduce(fogReducer, emptyKnowledge());
    expect(replay).toEqual(again);
  });
});
```

- [ ] **Step 3: Run the tests and confirm they fail.**

Run: `npm test -- --run src/concepts/fog-of-war/reducer.test.ts`
Expected: FAIL — `./reducer` and `./events` do not resolve.

- [ ] **Step 4: Write `events.ts`.**

```ts
/**
 * Fog of War event layer (rpg-project ideas/fog-of-war/design.md §"The event
 * layer"). These types are the draft proto contract: what the concept proves
 * by being played is what rpg-api-protos then encodes.
 *
 * `PositionLike` and `WallLike` match the generated v1alpha2 `Position` and
 * `Wall` messages field-for-field rather than importing the generated classes
 * — same rationale as combat-pacing's fixtures.ts.
 */

export interface PositionLike { x: number; y: number; z: number }

/** Matches v1alpha2 Wall field-for-field. Doors are a DOOR_* `kind`, not a
 * separate list — the wire has no separate door collection. */
export interface WallLike {
  from: PositionLike;
  to: PositionLike;
  kind: number;
  id?: string;
}

/** A hex is VISIBLE (current authorized truth) or REMEMBERED (a frozen last
 * observation, carried in full). UNSEEN is omission — never a value. There is
 * deliberately no removal state: a witnessed removal is a VISIBLE record that
 * no longer lists the thing, and a hidden removal is no record at all. */
export type HexState = 'VISIBLE' | 'REMEMBERED';

/** What occupies a hex. Resolves against the event's `entities` collection. */
export interface Placement {
  entityId: string;
  /** Hex-direction index 0-5. Carried on the placement, NOT on the entity:
   * a record is an observation, so two viewers who saw the same goblin face
   * different ways must keep different frozen memories of it. Moving this to
   * FogEntity would let one viewer's sighting rewrite another's memory. */
  facing: number;
}

/** One hex's complete authorized truth for one viewer, as observed at one
 * moment. A VISIBLE record is TOTAL: `contents: []` positively means empty,
 * never "omitted". A REMEMBERED record carries the frozen observation in
 * full, so hydration and live transitions share one code path. */
export interface HexRecord {
  position: PositionLike;
  state: HexState;
  terrain: number;
  zoneId: string;
  edges: WallLike[];
  contents: Placement[];
}

/** Everything the server chose to disclose about an entity to this viewer.
 * Fields the server withholds are simply absent — disclosure is a server
 * decision, never client policy. */
export interface FogEntity {
  entityId: string;
  name: string;
  type: 'player' | 'monster' | 'obstacle';
  classRefId?: string;
  monsterRefId?: string;
  obstacleType?: number;
  propRefId?: string;
}

/** One viewer's slice. Hexes say where; entities say what. Delivered together
 * so they cannot disagree. */
export interface HexKnowledgeChanged {
  hexes: HexRecord[];
  entities: FogEntity[];
}

export const hexKey = (p: PositionLike): string => `${p.x},${p.y},${p.z}`;
```

- [ ] **Step 5: Write `reducer.ts`.**

Knowledge is a map keyed by hex. Applying an event is a merge; there is no
other input and no derivation.

```ts
import type { FogEntity, HexKnowledgeChanged, HexRecord } from './events';
import { hexKey } from './events';

export interface FogKnowledge {
  hexes: ReadonlyMap<string, HexRecord>;
  entities: ReadonlyMap<string, FogEntity>;
}

export const emptyKnowledge = (): FogKnowledge => ({
  hexes: new Map(),
  entities: new Map(),
});

export function fogReducer(state: FogKnowledge, event: HexKnowledgeChanged): FogKnowledge {
  const entities = new Map(state.entities);
  for (const entity of event.entities ?? []) entities.set(entity.entityId, entity);

  const hexes = new Map(state.hexes);
  for (const record of event.hexes ?? []) {
    // Every record replaces wholesale — never a merge, never a delete. This is
    // the reducer's only behavior. Placements referencing an entity this
    // viewer has not been told about are dropped (fail closed).
    hexes.set(hexKey(record.position), {
      ...record,
      contents: record.contents.filter((p) => entities.has(p.entityId)),
    });
  }
  return { hexes, entities };
}
```

- [ ] **Step 6: Run the reducer tests and confirm they pass.**

Run: `npm test -- --run src/concepts/fog-of-war/reducer.test.ts`
Expected: PASS, 8 tests.

- [ ] **Step 7: Write the adapter and its failing test, then implement.**

`adapter.ts` turns `FogKnowledge` into exact `HexGrid` props. Remembered hexes
populate `rememberedFloorHexKeys`/`rememberedWallHexKeys`; contents become
`HexGridEntity[]` with `knowledgeState` set from their hex's state. Unseen is
omission — no record, no prop entry.

Write `adapter.test.ts` first, asserting: a remembered hex's key appears in
both remembered key sets; its occupant is emitted with
`knowledgeState: 'remembered'`; a visible hex's occupant has
`knowledgeState: 'visible'`; and no key appears in `floorTiles` that has no
record. Run it, confirm failure, then implement `toHexGridProps`.

- [ ] **Step 8: Full check and commit.**

```bash
npm run ci-check
git add src/concepts/fog-of-war/
git commit -m "feat(fog)#605: hex knowledge event layer, reducer, and adapter"
```

Expected: CI green; commit contains only Task 3 files. Open the PR and stop —
this is a review seam. The event layer is what the protos transcribe, so it
gets read before an authority exists to feed it.

## Task 4: Fixture authority — world truth, line of sight, intent to events

The server stand-in. It is the only code in the concept permitted to read world
truth, and it sits on the far side of the event boundary.

**Issue/branch setup:** File a new `rpg-dnd5e-web` issue titled
`Fog of War: fixture authority and visibility reconciliation`, Board 19,
Feature=`The Dungeon`, Team=`UI/UX`. **Continue on `feat/605-fog-events`** —
this is a further commit on the concept branch, not a new branch or PR.

**Files:**
- Create: `src/concepts/fog-of-war/authority/world.ts`
- Create: `src/concepts/fog-of-war/authority/los.ts`
- Create: `src/concepts/fog-of-war/authority/authority.ts`
- Create: `src/concepts/fog-of-war/authority/authority.test.ts`
- Create: `src/concepts/fog-of-war/boundary.test.ts`

**Interfaces:**
- Consumes: nothing from the consumer half.
- Produces: `createAuthority(world)` exposing `subscribe()` and
  `moveViewer(coord)`, both returning `HexKnowledgeChanged` events.

- [ ] **Step 1: Create the issue and worktree.** Same shape as Task 3 Step 1;
  branch `feat/$ISSUE-fog-authority`.

- [ ] **Step 2: Write the failing authority tests.**

These are the cases the play loop must reach. Cases 7 and 8 from `design.md`
are why the record shape exists, so they are tested at the authority, not only
at the reducer.

```ts
describe('fixture authority', () => {
  it('emits VISIBLE records for what the viewer can see and nothing else', () => {
    const authority = createAuthority(twoRoomCrypt());
    const event = authority.subscribe();
    expect(event.hexes.every((h) => h.state === 'VISIBLE')).toBe(true);
    // Room 2 is not mentioned at all — unseen is omission, not an empty record.
    expect(event.hexes.some((h) => inRoom2(h.position))).toBe(false);
  });

  it('emits REMEMBERED for hexes the viewer just lost sight of', () => {
    const authority = createAuthority(twoRoomCrypt());
    authority.subscribe();
    const event = authority.moveViewer(intoRoom2);
    expect(event.hexes.filter((h) => inRoom1(h.position)).every((h) => h.state === 'REMEMBERED')).toBe(true);
  });

  it('emits nothing to a viewer for a change they cannot see', () => {
    const authority = createAuthority(twoRoomCrypt());
    authority.subscribe();
    authority.moveViewer(intoRoom2);
    const event = authority.mutateHidden(() => openRoom1Chest());
    expect(event.hexes).toEqual([]);
  });

  it('freezes a monster on the last hex the viewer saw it (design.md case 7)', () => {
    const authority = createAuthority(twoRoomCrypt());
    authority.subscribe();
    const crossing = authority.moveMonster('goblin-1', pathThroughSight);
    const lastSeen = crossing.at(-1)!;
    expect(lastSeen.hexes.some((h) => h.state === 'REMEMBERED'
      && h.contents.some((p) => p.entityId === 'goblin-1'))).toBe(true);
  });

  it('re-sighting the frozen hex reports it empty (design.md case 8)', () => {
    const authority = createAuthority(twoRoomCrypt());
    authority.subscribe();
    authority.moveMonster('goblin-1', pathThroughSight);
    const event = authority.moveViewer(besideTheFrozenHex);
    const record = event.hexes.find((h) => sameHex(h.position, frozenHex))!;
    expect(record.state).toBe('VISIBLE');
    expect(record.contents).toEqual([]);
  });
});
```

- [ ] **Step 3: Run and confirm failure.**

Run: `npm test -- --run src/concepts/fog-of-war/authority/authority.test.ts`
Expected: FAIL — `createAuthority` does not exist.

- [ ] **Step 4: Implement `world.ts` and `los.ts`.**

`world.ts` holds the authored two-room crypt: hexes with terrain, walls as
`WallLike` edges with doors as a `kind`, entity roster, and entity positions.
`los.ts` is a deliberately crude hex line-of-sight — walk the hex line from
viewer to target and block on any solid wall edge crossed. It does not need to
be correct D&D; it needs to be *decidable and stable* so the emitted stream is
reproducible.

Keep this small. It stands in for work the toolkit will own. If it starts
growing rules beyond producing the event stream, that is the signal to stop —
see `design.md` §"Concept architecture".

- [ ] **Step 5: Implement `authority.ts`.**

Hold the previous visible set per viewer. On any mutation: recompute the
visible set, then diff.

- Newly visible or still visible → a `VISIBLE` record built from world truth,
  with `contents` listing every entity standing there and an `entities` entry
  for each disclosed entity.
- Newly not visible, previously visible → a `REMEMBERED` record.
- Not visible and not previously visible → nothing at all.

The diff is the whole design: what the viewer cannot see produces no message,
so stale memory persists without the client doing anything.

- [ ] **Step 6: Write the boundary test.**

This is the test that keeps client-side LOS out permanently.

```ts
import { readdirSync, readFileSync } from 'node:fs';
import { join } from 'node:path';
import { describe, expect, it } from 'vitest';

const CONSUMER = ['events.ts', 'reducer.ts', 'adapter.ts', 'FogOfWarConcept.tsx'];

describe('fog concept boundary', () => {
  it('no consumer module imports from the authority half', () => {
    for (const file of CONSUMER) {
      const source = readFileSync(join(__dirname, file), 'utf8');
      expect(source, `${file} must not import world truth`).not.toMatch(/from\s+['"].*authority/);
    }
  });
});
```

- [ ] **Step 7: Run everything and commit.**

```bash
npm test -- --run src/concepts/fog-of-war/
npm run ci-check
git add src/concepts/fog-of-war/
git commit -m "feat(fog)#$ISSUE: fixture authority, crude LOS, and the event boundary"
```

Expected: CI green. Push to `feat/605-fog-events` and stop — second review
seam. Do not merge; the concept is not runnable yet.

## Task 5: Playable concept page, documentation, and visual evidence

**Issue/branch setup:** Update `rpg-dnd5e-web#606` ("Fog of War: executable
two-room concept") — its scope becomes the playable page. Board 19,
Feature=`The Dungeon`, Team=`UI/UX`. **Continue on `feat/605-fog-events`.**
When this task is done the concept runs, so update PR #611's title and body to
describe the whole concept, close #605/#606/the authority issue, and merge.

**Files:**
- Create: `src/concepts/fog-of-war/FogOfWarConcept.tsx`
- Create: `src/concepts/fog-of-war/FogOfWarConcept.test.tsx`
- Create: `src/concepts/fog-of-war/CONTRACT.md`
- Create: `src/concepts/README.md`
- Modify: `src/concepts/ConceptsView.tsx`
- Modify: `src/App.tsx`

- [ ] **Step 1: Update issue 606 and create the worktree.**

- [ ] **Step 2: Write the failing page test.**

Assert the page wires authority → reducer → adapter → `HexGrid` and nothing
else: clicking a hex calls `authority.moveViewer`, the returned event is passed
to `fogReducer`, and `HexGrid` receives only `toHexGridProps` output. Assert
the page holds no state derived from world truth.

- [ ] **Step 3: Build the page.**

A real crypt `HexGrid` plus a control strip: move by clicking a hex, open the
door, step the monster along its path, trigger a hidden mutation, and force a
reconnect (discard reducer state, re-subscribe). A side inspector may display
world truth for review — it reads from the authority and never feeds the
reducer. Label it clearly as a review aid.

Pass `showFrontierGroundHints={false}` so hints never expose unseen-adjacent
ground.

- [ ] **Step 4: Register the concept and the dev query seam.**

Add the Fog of War entry to `ConceptsView.tsx`. Add the dev-only
`?concept=fog-of-war` seam in `App.tsx` per the Task 4 precedent in this plan's
earlier revision; normal navigation is unchanged.

- [ ] **Step 5: Write `src/concepts/README.md` and `CONTRACT.md`.**

The README states the concepts-wide policy from `design.md`. `CONTRACT.md` is
evidence-only: what the event layer needed, what the current wire carries, and
candidate gaps. It is not a platform request — Kirk reviews it before any
candidate becomes a cross-repo ask.

- [ ] **Step 6: Capture visual evidence by playing it.**

```bash
node /home/kirk/game-dev/tools/browser/screenshot.mjs \
  "http://localhost:5173/?concept=fog-of-war" <output.png> 5000 <width> <height>
```

Capture desktop and mobile for: Room 1 visible with Room 2 absent; the door
opened; Room 2 visible with Room 1 in crypt memory; a hidden mutation leaving
memory unchanged; reconnect; re-sight replacing memory; and the monster
sequence — visible crossing, frozen after it leaves sight, gone on approach.

The monster sequence is the money shot. It is the one a reviewer can judge
without reading any code.

- [ ] **Step 7: Full check, commit, PR.**

```bash
npm run ci-check
git add src/concepts/
git commit -m "feat(fog)#606: playable Fog of War concept, docs, and evidence"
```

Expected: CI green; evidence attached to the PR.

## Final Acceptance Sweep

- [ ] Confirm the shared-component PRs (Tasks 1-2) were issue-first, Board-19 classified, independently reviewed, and merged in order from latest `main` — Task 1 already merged as PR #602.
- [ ] Confirm the concept landed as one PR (#611) covering Tasks 3-5, reviewed at each seam and merged once it ran, rather than as three merges of inert parts.
- [ ] Confirm the rendered scene is built only from emitted events — no path reaches the renderer that bypasses the event layer.
- [ ] Confirm `boundary.test.ts` passes and no consumer module imports from `authority/`.
- [ ] Confirm a `VISIBLE` record with `contents: []` deletes a remembered occupant, proven both at the reducer and by playing it.
- [ ] Confirm omitted `knowledgeState`, remembered key sets, and `showFrontierGroundHints` leave production defaults unchanged; no production encounter stream/reducer file changed.
- [ ] Confirm unseen is absent from fixtures, adapter output, floor/wall/entity renderer inputs, and visual captures.
- [ ] Confirm mixed Room 1 remembered and Room 2 visible geometry render concurrently; no `currentRoomId` filtering removes remembered Room 1.
- [ ] Confirm remembered material precedence, opaque `transparent=false`/`depthWrite=true` treatment, safe GLTF/texture cache handling, material-array restoration, disposal ownership, no remembered animation or demand invalidation, and stable shaded instanced-floor ordering.
- [ ] Confirm remembered Synty segments use the portion before `->`, fittings inspect all `|`-joined keys, and remembered door/frame/end/fitting pieces have crypt treatment with no handlers.
- [ ] Confirm remembered entities never enter pathing, occupancy, hover, selection, targeting, self indicator, or turn order; existing ghost behavior remains pale-cyan and separate.
- [ ] Confirm the reducer takes events as its only input, is idempotent on repeated records, drops placements referencing undisclosed entities, and never derives LOS, reveal, memory, or hidden mutations.
- [ ] Confirm no delete path exists: no `GONE` state, no removal transition, no tombstone, and no reducer branch that removes a hex record.
- [ ] Confirm `facing` is carried on `Placement` and not on `FogEntity`, and that a re-disclosed entity cannot alter a remembered facing.
- [ ] Confirm replaying a recorded session against a fresh reducer reproduces identical state — the reducer is a pure function of its events.
- [ ] Confirm desktop and mobile real-WebGL evidence covers Room 1 visible with Room 2 absent, door/reveal, Room 2 plus Room 1 memory, hidden-change isolation, reconnect, re-sight, and the monster sequence: visible crossing, frozen after leaving sight, gone on approach.
- [ ] Confirm `src/concepts/README.md` and evidence-only fog `CONTRACT.md` state the approved concept workflow and have not created a platform request.

## Out of Scope Follow-ups

The approved concept is the consumer contract only. Separate, later issue-first work may: teach toolkit rules and encounter-data serialization to retain complete per-viewer knowledge; add proto snapshot/transition shapes; have API durably store and project only authorized current/remembered records; remove unconditional wall/door leakage; restore remembered knowledge on reconnect; and promote the proven web seam into the production encounter route. Intelligence, senses, memory fidelity/retention, and alternative palettes such as forest sepia remain future design work and do not change this plan's two-state renderer seam.
