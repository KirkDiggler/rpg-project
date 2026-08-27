# Authored Crypt Lighting Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Render region-authored crypt darkness and eight placed light-source props identically in the dungeon builder and playable session, without changing mechanics or the device-stable floor material.

**Architecture:** Normalize atlas region lighting and placed source props into immutable `DungeonLightingFacts`, resolve one focus-aware `DungeonLightingPlan`, and render it through a shared `DungeonEnvironment`. Real point lights illuminate lit 3D materials; the same active source descriptions deterministically color the unlit continuous-UV floor after its per-region base exposure is applied.

**Tech Stack:** React 19, TypeScript, Three.js, React Three Fiber, `@react-three/test-renderer`, Vitest, Testing Library, Vite.

**Spec:** `ideas/dungeon-builder/dungeon-lighting.md`

## Global Constraints

- Execution starts only after Kirk supplies one owning `rpg-dnd5e-web` issue assigned to Team Assets; worker sessions do not read or mutate Project 19.
- Create the implementation branch from fresh `origin/dev`, never local `dev`, and target the PR at `dev`.
- One web issue, one branch, one implementation PR; do not merge it.
- Visuals only: no fog, LOS, darkvision, reveal, movement, targeting, or encounter-rule changes.
- Exactly eight source refs emit; region intensity never scales a source's output.
- Preserve `MeshBasicMaterial`, `toneMapped={false}`, `DUNGEON_SURFACE_Y`, continuous absolute-world UVs, and the current shell/profile fallback.
- Real point lights are capped at 12 nearest the current focus with stable ties; floor pools use exactly that same active set and stay inside the source region.
- No shadows, inferred door lights, authored source parameters, flicker, VFX, audio, carried lights, provider work, or wire changes.
- Use `openai-codex/gpt-5.6-luna` or stronger for any delegated implementation/review work; never mini.
- Run focused tests during tasks and one full `npm run ci-check` before the first push.
- Request exactly one Copilot review when the PR opens; answer every finding and never re-request.
- Kirk alone gives visual approval and performs merges.

## Execution preflight

The executor receives the issue number as `WEB_ISSUE` from Kirk or the board-management session, then uses the worktree skill:

```bash
test -n "$WEB_ISSUE"
cd /home/kirk/game-dev/rpg-dnd5e-web
git fetch origin --prune
git worktree add "/home/kirk/.pi/worktrees/rpg-dnd5e-web/${WEB_ISSUE}-dungeon-lighting" \
  -b "feat/${WEB_ISSUE}-dungeon-lighting" origin/dev
cd "/home/kirk/.pi/worktrees/rpg-dnd5e-web/${WEB_ISSUE}-dungeon-lighting"
npm install
npx vitest run \
  src/author/paletteData.test.ts \
  src/components/hex-grid/syntyHexFloorHelpers.test.ts \
  src/components/hex-grid/SyntyHexFloor.test.tsx \
  src/components/session/atlasToScene3D.test.ts \
  src/components/session/DungeonSceneLights.test.tsx \
  src/author/preview3d/DungeonPreview3D.render.test.tsx
```

Expected baseline: all named files pass. Stop and investigate any baseline failure before changing code.

## File responsibility map

- `src/rendering/dungeonLightSources.ts` — immutable eight-ref visual source vocabulary and calibrated presets; no React or atlas adaptation.
- `src/rendering/dungeonLighting.ts` — pure atlas-lighting validation, source placement, fallback, focus culling, and complete plan resolution.
- `src/components/hex-grid/syntyHexFloorHelpers.ts` — device-stable base exposure and floor-pool color math.
- `src/components/hex-grid/SyntyHexFloor.tsx` — per-cell unlit floor material/UV leaf; consumes resolved floor maps without deciding lighting semantics.
- `src/components/session/DungeonSceneLights.tsx` — renders one already-resolved light plan; no atlas interpretation.
- `src/components/session/DungeonEnvironment.tsx` — shared builder/game composition of lights, shell, and atlas props.
- `src/components/session/atlasToScene3D.ts` — converts wire positions/regions/props once and attaches immutable lighting facts to `Scene3D`.
- `src/author/fixtures/cryptLightingShowcase.ts` — deterministic visual fixture only; never a second compiler or runtime rule source.

## Performance boundary

At most 12 real point lights mount for one focus. Selection uses squared X/Z distance and stable ties; no per-frame allocation mutates scene facts. The same selected set drives floor pools, and tests include a 13-source discriminator so the cap cannot silently disappear.

## Spec coverage index

- Low crypt fill, mixed-region validation, atomic fallback, and the point-light budget: Task 2.
- Regional floor exposure, source pools, unlit material, and continuous UV preservation: Task 3.
- Authored source rendering plus shared builder/game `DungeonEnvironment`: Task 4.
- Exact eight-source manifest and legacy-authority removal: Task 1.
- Integrated visual approval on the reference-tomb geometry: Task 5.
- Complete verification, one Copilot round, PR publication, and stop-for-Kirk merge: Task 6.
- Every deferred/non-goal item is repeated in Global Constraints and is excluded from all task file lists.

---

### Task 1: Establish one light-source manifest

**Files:**
- Create: `src/rendering/dungeonLightSources.ts`
- Create: `src/rendering/dungeonLightSources.test.ts`
- Modify: `src/author/paletteData.ts`
- Modify: `src/author/paletteData.test.ts`
- Modify: `src/components/playtest/playtestMapHelpers.ts`
- Modify: `src/components/playtest/playtestMapHelpers.test.ts`

**Interfaces:**
- Produces:

```ts
export interface DungeonLightSourceSpec {
  readonly color: string;
  readonly intensity: number;
  readonly distance: number;
  readonly height: number;
  readonly floorPoolStrength: number;
}

export const DUNGEON_LIGHT_SOURCE_SPECS: ReadonlyMap<
  string,
  DungeonLightSourceSpec
>;
export const DUNGEON_LIGHT_SOURCE_REFS: readonly string[];
export function dungeonLightSourceSpec(
  ref: string
): DungeonLightSourceSpec | undefined;
export function isDungeonLightSourceRef(ref: string): boolean;
```

- Consumers: palette categorization in this task; atlas lighting facts in Task 2; legacy mood-light projection retained by `playtestMapHelpers.ts`.

- [ ] **Step 1: Write failing manifest tests**

Create `dungeonLightSources.test.ts` with an exact key assertion and semantic assertions:

```ts
const EXPECTED = [
  'dnd5e:props:brazier',
  'dnd5e:props:candle-stand',
  'dnd5e:props:candles',
  'dnd5e:props:glowing-orb',
  'dnd5e:props:lantern',
  'dnd5e:props:rune-marker',
  'dnd5e:props:rune-pillar',
  'dnd5e:props:torch-ornate',
] as const;

expect([...DUNGEON_LIGHT_SOURCE_REFS].sort()).toEqual(EXPECTED);
expect(isDungeonLightSourceRef('dnd5e:props:torch')).toBe(false);
expect(isDungeonLightSourceRef('dnd5e:props:stone-lantern')).toBe(false);
for (const ref of EXPECTED) {
  const spec = dungeonLightSourceSpec(ref);
  expect(spec).toBeDefined();
  expect(spec?.intensity).toBeGreaterThan(0);
  expect(spec?.distance).toBeGreaterThan(0);
  expect(spec?.height).toBeGreaterThanOrEqual(0);
  expect(spec?.floorPoolStrength).toBeGreaterThan(0);
  expect(spec?.floorPoolStrength).toBeLessThanOrEqual(1);
  expect(Object.isFrozen(spec)).toBe(true);
}
```

- [ ] **Step 2: Make palette tests demand the shared authority**

Replace `paletteData.test.ts`'s local eight-ref array with `DUNGEON_LIGHT_SOURCE_REFS`, and add a source check against `paletteData.ts`:

```ts
for (const ref of DUNGEON_LIGHT_SOURCE_REFS) {
  expect(categoryForProp(ref)).toBe('lighting');
}
expect(readFileSync('src/author/paletteData.ts', 'utf8')).not.toContain(
  'LIGHTING_PROP_KEYS'
);
```

- [ ] **Step 3: Run tests to verify RED**

Run:

```bash
npx vitest run src/rendering/dungeonLightSources.test.ts src/author/paletteData.test.ts
```

Expected: FAIL because `dungeonLightSources.ts` and its exports do not exist.

- [ ] **Step 4: Implement the immutable source manifest**

Use the approved warm `#ff9d52` and cool `#3d84dc` families with these initial calibration values:

```ts
const specs = new Map<string, DungeonLightSourceSpec>([
  ['dnd5e:props:brazier',       { color: '#ff9d52', intensity: 2.8, distance: 5.5, height: 0.9,  floorPoolStrength: 1.0 }],
  ['dnd5e:props:torch-ornate',  { color: '#ff9d52', intensity: 1.6, distance: 3.6, height: 1.4,  floorPoolStrength: 0.85 }],
  ['dnd5e:props:candle-stand',  { color: '#ff9d52', intensity: 1.4, distance: 3.2, height: 1.0,  floorPoolStrength: 0.70 }],
  ['dnd5e:props:lantern',       { color: '#ff9d52', intensity: 1.3, distance: 3.0, height: 0.65, floorPoolStrength: 0.65 }],
  ['dnd5e:props:candles',       { color: '#ff9d52', intensity: 1.1, distance: 2.6, height: 0.35, floorPoolStrength: 0.50 }],
  ['dnd5e:props:glowing-orb',   { color: '#3d84dc', intensity: 2.0, distance: 4.5, height: 1.2,  floorPoolStrength: 0.90 }],
  ['dnd5e:props:rune-pillar',   { color: '#3d84dc', intensity: 0.9, distance: 2.6, height: 1.2,  floorPoolStrength: 0.65 }],
  ['dnd5e:props:rune-marker',   { color: '#3d84dc', intensity: 0.7, distance: 2.2, height: 0.15, floorPoolStrength: 0.45 }],
]);
```

Freeze each value before placing it in the exported map. Export a frozen key array. Lookups must use `Map.get`, never a plain-object index.

- [ ] **Step 5: Derive palette category from the manifest**

Remove `LIGHTING_PROP_KEYS` from `paletteData.ts` and implement:

```ts
export function categoryForProp(ref: string) {
  return isDungeonLightSourceRef(ref) ? 'lighting' : 'obstacles-props';
}
```

Update its comments to name `dungeonLightSources.ts` as authority and remove the stale `walkLighting.ts` claim; that file no longer exists.

- [ ] **Step 6: Route the live legacy prop-light helper through the manifest**

Delete `MOOD_LIGHT_SPEC_BY_PROP_REF` from `playtestMapHelpers.ts`. In `buildCryptMoodLights`, convert the legacy short id to the canonical ref and copy the shared values:

```ts
const spec = prop.propRefId
  ? dungeonLightSourceSpec(`dnd5e:props:${prop.propRefId}`)
  : undefined;
if (!spec) continue;
lights.push({
  position: [world.x, MOOD_LIGHT_HEIGHT, world.z],
  color: spec.color,
  intensity: spec.intensity,
  distance: spec.distance,
});
```

Keep the old route's separately inferred door lights unchanged; the new shared environment must not import or use them. Update legacy tests so candles expect warm `#ff9d52` and every manifest ref projects the shared values.

- [ ] **Step 7: Run focused tests GREEN**

Run:

```bash
npx vitest run \
  src/rendering/dungeonLightSources.test.ts \
  src/author/paletteData.test.ts \
  src/components/playtest/playtestMapHelpers.test.ts
```

Expected: PASS.

- [ ] **Step 8: Commit Task 1**

```bash
git add src/rendering/dungeonLightSources.ts \
  src/rendering/dungeonLightSources.test.ts \
  src/author/paletteData.ts src/author/paletteData.test.ts \
  src/components/playtest/playtestMapHelpers.ts \
  src/components/playtest/playtestMapHelpers.test.ts
git commit -m "feat(rendering): define dungeon light sources"
```

---

### Task 2: Preserve atlas lighting and resolve one complete plan

**Files:**
- Create: `src/rendering/dungeonLighting.ts`
- Create: `src/rendering/dungeonLighting.test.ts`
- Modify: `src/components/session/atlasToScene3D.ts`
- Modify: `src/components/session/atlasToScene3D.test.ts`
- Modify: `src/components/session/DungeonShell.test.tsx`
- Modify: `src/components/session/DungeonShell.provider.test.tsx`
- Modify: `src/components/session/SessionCanvas.test.tsx`

**Interfaces:**
- Consumes: `dungeonLightSourceSpec(ref)` from Task 1; normalized cube-cell keys and `propWorldPosition` from `atlasToScene3D.ts`.
- Produces:

```ts
export type DungeonLightingFallbackReason =
  | 'no-regions'
  | 'unknown-archetype'
  | 'mixed-archetypes'
  | 'invalid-intensity'
  | 'conflicting-region-cells'
  | 'unowned-floor-cells'
  | 'source-outside-region';

export interface DungeonLightingRegionInput {
  readonly id: string;
  readonly archetype: string;
  readonly intensity: number;
  readonly cellKeys: readonly string[];
}

export interface DungeonLightingSourceInput {
  readonly key: string;
  readonly ref: string;
  readonly cellKey: string;
  readonly groundedPosition: readonly [number, number, number];
}

export interface DungeonLightSource {
  readonly key: string;
  readonly ref: string;
  readonly regionId: string;
  readonly cellKey: string;
  readonly position: readonly [number, number, number];
  readonly spec: DungeonLightSourceSpec;
}

export interface DungeonPointLight {
  readonly key: string;
  readonly position: readonly [number, number, number];
  readonly color: string;
  readonly intensity: number;
  readonly distance: number;
}

export interface DungeonFloorPool extends FloorPoolLight {
  readonly floorPoolStrength: number;
}

export interface DungeonLightingFacts {
  readonly mode: 'crypt' | 'legacy';
  readonly fallbackReason: DungeonLightingFallbackReason | null;
  readonly regionByCell: ReadonlyMap<string, string>;
  readonly intensityByCell: ReadonlyMap<string, number>;
  readonly sources: readonly DungeonLightSource[];
}

export interface DungeonLightingPlan {
  readonly mode: 'crypt' | 'legacy';
  readonly ambientIntensity: number;
  readonly directionalIntensity: number;
  readonly directionalPosition: readonly [number, number, number];
  readonly pointLights: readonly DungeonPointLight[];
  readonly floorExposureByCell: ReadonlyMap<string, number>;
  readonly floorPoolsByCell: ReadonlyMap<string, readonly DungeonFloorPool[]>;
  readonly diagnostics: readonly string[];
}

export function buildDungeonLightingFacts(
  floorCellKeys: readonly string[],
  regions: readonly DungeonLightingRegionInput[],
  sources: readonly DungeonLightingSourceInput[]
): DungeonLightingFacts;

export function resolveDungeonLighting(
  facts: DungeonLightingFacts,
  focus: Readonly<{ x: number; z: number }>
): DungeonLightingPlan;
```

`Scene3D` gains required `lighting: DungeonLightingFacts` while retaining `archetypes` for shell resolution.

- [ ] **Step 1: Write failing pure resolver tests**

Cover these discriminators in `dungeonLighting.test.ts`:

```ts
expect(validFacts.intensityByCell.get('0,0,0')).toBe(0.6);
expect(validFacts.intensityByCell.get('1,-1,0')).toBe(0.15);
expect(sourceAtZero.spec.intensity).toBe(sourceAtOne.spec.intensity);
expect(resolveDungeonLighting(validFacts, { x: 0, z: 0 }).pointLights)
  .toHaveLength(1);
expect(invalidFacts.fallbackReason).toBe('conflicting-region-cells');
expect(resolveDungeonLighting(invalidFacts, { x: 0, z: 0 })).toMatchObject({
  mode: 'legacy',
  ambientIntensity: 0.6,
  directionalIntensity: 0.8,
  pointLights: [],
});
```

Create 13 equidistant/nearby source inputs and assert exactly 12 survive, ordered by squared distance then stable `cellKey|ref|key` tie-break. Assert only the selected 12 appear in every `floorPoolsByCell` value.

- [ ] **Step 2: Run pure tests RED**

```bash
npx vitest run src/rendering/dungeonLighting.test.ts
```

Expected: FAIL because the module does not exist.

- [ ] **Step 3: Implement fact validation and atomic legacy fallback**

Use exact legacy constants `0.6`, `0.8`, `[10,20,10]`. Use initial crypt fill `0.08`, `0.05`, `[10,20,10]`; Task 5 may change only the two crypt intensity constants after visual judgment.

Validation order is deterministic:

1. zero regions -> `no-regions`;
2. trimmed archetypes must all equal `crypt`; unknown -> `unknown-archetype`, more than one distinct known word -> `mixed-archetypes`;
3. every intensity must be finite and inside `[0,1]` -> otherwise `invalid-intensity`;
4. duplicate cell ownership -> `conflicting-region-cells`;
5. every floor cell must have region ownership -> otherwise `unowned-floor-cells`;
6. every recognized source must belong to exactly one region -> otherwise `source-outside-region`.

An invalid fact set returns empty maps/sources and one fallback reason. Do not clamp invalid wire input into validity. Its resolved legacy plan carries exactly one builder diagnostic, `Legacy lighting: <fallback-reason>`.

For valid sources, copy the manifest entry into `DungeonLightSource.spec` and derive its final point position as:

```ts
[
  groundedPosition[0],
  groundedPosition[1] + DUNGEON_SURFACE_Y + spec.height,
  groundedPosition[2],
]
```

The source spec is copied unchanged; region intensity is never read while building source output.

- [ ] **Step 4: Implement nearest-12 planning and region-clipped pools**

Export `DUNGEON_POINT_LIGHT_BUDGET = 12`. Sort a copy of sources by squared X/Z distance to focus and stable source key; never mutate facts. Build one `DungeonFloorPool` per selected source carrying `color`, `intensity`, `distance`, and `floorPoolStrength`. Add it only to cells whose `regionByCell` matches that source's region.

When more than 12 sources exist, add exactly one diagnostic:

```text
12 of N placed light sources active near this view
```

- [ ] **Step 5: Make `buildScene3D` normalize wire facts once**

In `atlasToScene3D.ts`, after props are built, map every `floorTiles` key, map each atlas region through the existing `positionToCube` and `coordToKey`, and map each positioned prop through `propWorldPosition`; let the manifest decide whether it becomes a recognized source. Call `buildDungeonLightingFacts(floorCellKeys, regions, sources)` and return it as `scene.lighting`.

Use a stable source key containing ref, cell key, and atlas prop index so duplicate visual refs cannot collide.

- [ ] **Step 6: Extend atlas scene tests**

Add a two-region fixture with intensities `0.6` and `0.15`, a raised/offset brazier, and an ordinary pillar. Assert:

```ts
expect(scene.lighting.mode).toBe('crypt');
expect(scene.lighting.intensityByCell).toEqual(
  new Map([
    ['0,0,0', 0.6],
    ['1,-1,0', 0.15],
  ])
);
expect(scene.lighting.sources).toHaveLength(1);
expect(scene.lighting.sources[0]?.ref).toBe('dnd5e:props:brazier');
```

Also assert the source's X/Z includes authored offset and Y includes authored elevation but not source height twice. Update direct `Scene3D` constructors with an explicit legacy fact fixture rather than weakening `Scene3D.lighting` to optional.

- [ ] **Step 7: Run Task 2 tests GREEN**

```bash
npx vitest run \
  src/rendering/dungeonLighting.test.ts \
  src/components/session/atlasToScene3D.test.ts \
  src/author/preview3d/DungeonPreview3D.test.ts
npm run typecheck
```

Expected: focused tests and TypeScript pass.

- [ ] **Step 8: Commit Task 2**

```bash
git add src/rendering/dungeonLighting.ts \
  src/rendering/dungeonLighting.test.ts \
  src/components/session/atlasToScene3D.ts \
  src/components/session/atlasToScene3D.test.ts \
  src/components/session/DungeonShell.test.tsx \
  src/components/session/DungeonShell.provider.test.tsx \
  src/components/session/SessionCanvas.test.tsx \
  src/author/preview3d/DungeonPreview3D.test.ts
git commit -m "feat(rendering): resolve authored dungeon lighting"
```

Before committing, inspect `git diff --cached --name-only`; every staged session test change must be limited to supplying the new required `lighting` facts.

---

### Task 3: Compose regional exposure and source pools on the unlit floor

**Files:**
- Modify: `src/components/hex-grid/syntyHexFloorHelpers.ts`
- Modify: `src/components/hex-grid/syntyHexFloorHelpers.test.ts`
- Modify: `src/components/hex-grid/SyntyHexFloor.tsx`
- Modify: `src/components/hex-grid/SyntyHexFloor.test.tsx`
- Modify: `src/components/session/DungeonShell.tsx`
- Modify: `src/components/session/DungeonShell.test.tsx`
- Modify: `src/components/session/DungeonShell.provider.test.tsx` only if its direct props require the new optional lighting input

**Interfaces:**
- Consumes: `DungeonLightingPlan.floorExposureByCell` and `.floorPoolsByCell` from Task 2.
- Produces:

```ts
export const CRYPT_DARK_FLOOR_TINT: THREE.Color;
export function cryptFloorBaseColor(intensity: number): THREE.Color;

export interface FloorPoolLight {
  readonly position: readonly [number, number, number];
  readonly color: string;
  readonly intensity?: number;
  readonly distance: number;
  readonly floorPoolStrength?: number;
}

export interface DungeonFloorLighting {
  readonly exposureByCell: ReadonlyMap<string, number>;
  readonly poolsByCell: ReadonlyMap<string, readonly FloorPoolLight[]>;
}
```

`SyntyHexFloorProps` gains optional `floorLighting?: DungeonFloorLighting`. Existing `poolLights` remains for the legacy look-lab callers.

- [ ] **Step 1: Write failing floor-color tests**

In `syntyHexFloorHelpers.test.ts`, assert:

```ts
expect(cryptFloorBaseColor(0).getHexString()).toBe('101318');
expect(isCloseTo(cryptFloorBaseColor(1), BASE)).toBe(true);
const half = cryptFloorBaseColor(0.5);
expect(half.r).toBeGreaterThan(CRYPT_DARK_FLOOR_TINT.r);
expect(half.r).toBeLessThan(BASE.r);
```

Extend pool tests so `floorPoolStrength: 0.5` produces less movement from base than `1`, and non-positive strength contributes nothing.

- [ ] **Step 2: Write failing `SyntyHexFloor` regional wiring tests**

Render two adjacent profile tiles with exposure `0` and `1`. Assert the first uses `#101318`, the second uses the existing crypt tint, both materials are `MeshBasicMaterial`, and both have `toneMapped === false`.

Add a warm pool only to the intensity-0 tile and assert:

- its color moves toward warm;
- the adjacent tile remains exactly its intensity-1 base;
- the profile UV arrays before/after lighting are equal; and
- remembered floor color still takes precedence over exposure and pools.

- [ ] **Step 3: Run floor tests RED**

```bash
npx vitest run \
  src/components/hex-grid/syntyHexFloorHelpers.test.ts \
  src/components/hex-grid/SyntyHexFloor.test.tsx
```

Expected: FAIL because regional floor APIs are absent.

- [ ] **Step 4: Implement deterministic base exposure**

Set `CRYPT_DARK_FLOOR_TINT` to `new THREE.Color('#101318')`. Implement `cryptFloorBaseColor` as a non-mutating clamped lerp from dark to the existing `CRYPT_FLOOR_TINT`:

```ts
return CRYPT_DARK_FLOOR_TINT.clone().lerp(
  CRYPT_FLOOR_TINT,
  Math.max(0, Math.min(1, intensity))
);
```

Export or relocate `CRYPT_FLOOR_TINT` so the helper owns the color math without duplicating RGB values.

Multiply each pool's quadratic distance weight by `floorPoolStrength ?? 1`. Keep `MAX_FLOOR_POOL_BLEND = 0.55` and weighted color averaging unchanged.

- [ ] **Step 5: Wire per-cell composition without touching UV generation**

For each tile key in `SyntyHexFloor`:

```ts
const intensity = floorLighting?.exposureByCell.get(key);
const base = intensity === undefined
  ? CRYPT_FLOOR_TINT
  : cryptFloorBaseColor(intensity);
const lights = floorLighting?.poolsByCell.get(key) ?? poolLights ?? [];
```

Pass `base` and `lights` to the tile. Keep remembered rendering first, lit-surface A/B behavior unchanged, profile UV calculations untouched, and non-crypt rendering byte-behavior identical.

- [ ] **Step 6: Pass floor lighting through both shell paths**

Add optional `floorLighting` to `DungeonShellProps`, `LegacyShell`, and `ProfileResources`; pass it to `SyntyHexFloor` in both branches. This preserves the shell's existing atomic resource fallback while allowing the resolved lighting plan to survive either floor resource path.

- [ ] **Step 7: Run floor and shell tests GREEN**

```bash
npx vitest run \
  src/components/hex-grid/syntyHexFloorHelpers.test.ts \
  src/components/hex-grid/SyntyHexFloor.test.tsx \
  src/components/session/DungeonShell.test.tsx \
  src/components/session/DungeonShell.provider.test.tsx
```

Expected: PASS.

- [ ] **Step 8: Commit Task 3**

```bash
git add src/components/hex-grid/syntyHexFloorHelpers.ts \
  src/components/hex-grid/syntyHexFloorHelpers.test.ts \
  src/components/hex-grid/SyntyHexFloor.tsx \
  src/components/hex-grid/SyntyHexFloor.test.tsx \
  src/components/session/DungeonShell.tsx \
  src/components/session/DungeonShell.test.tsx \
  src/components/session/DungeonShell.provider.test.tsx
git commit -m "feat(rendering): shade crypt floors by region and sources"
```

---

### Task 4: Share one environment renderer between builder and game

**Files:**
- Create: `src/components/session/DungeonEnvironment.tsx`
- Create: `src/components/session/DungeonEnvironment.test.tsx`
- Modify: `src/components/session/DungeonSceneLights.tsx`
- Modify: `src/components/session/DungeonSceneLights.test.tsx`
- Modify: `src/components/session/SessionCanvas.tsx`
- Modify: `src/components/session/SessionCanvas.test.tsx`
- Modify: `src/author/preview3d/DungeonPreview3D.tsx`
- Modify: `src/author/preview3d/DungeonPreview3D.test.ts`
- Modify: `src/author/preview3d/DungeonPreview3D.render.test.tsx`

**Interfaces:**
- Consumes: `Scene3D.lighting`, `resolveDungeonLighting`, `DungeonShell.floorLighting`, and existing `AtlasPropModel`.
- Produces:

```ts
export interface DungeonEnvironmentProps {
  readonly scene: Scene3D;
  readonly focus: Readonly<{ x: number; z: number }>;
  readonly hexSize: number;
  readonly doors?: ReadonlyMap<string, DoorInfo>;
  readonly onDoorClick?: (door: string) => void;
  readonly onShellFallbackReason?: (reason: ShellFallbackReason | null) => void;
  readonly onLightingDiagnostics?: (messages: readonly string[]) => void;
}

export function DungeonEnvironment(
  props: DungeonEnvironmentProps
): React.ReactElement;

export interface DungeonSceneLightsProps {
  readonly plan?: DungeonLightingPlan;
}
```

Omitted `DungeonSceneLights.plan` renders the exact legacy constants for isolated legacy callers/tests.

- [ ] **Step 1: Write failing light-renderer tests**

Extend `DungeonSceneLights.test.tsx` with a crypt plan containing two point lights. Assert one ambient, one directional, two point lights, `decay={2}`, and exact position/color/intensity/distance. Keep the existing omitted-plan legacy assertions.

- [ ] **Step 2: Write failing `DungeonEnvironment` tests**

Build a one-region crypt scene with one brazier and assert the component renders:

- one shell;
- one prop model;
- low crypt ambient/directional;
- one point light;
- floor-lighting maps passed into the shell; and
- an empty diagnostic callback.

Build a legacy scene and assert static `0.6/0.8`, zero point lights, and unchanged floor props. Build a 13-source scene and assert the callback receives the budget diagnostic.

- [ ] **Step 3: Run component tests RED**

```bash
npx vitest run \
  src/components/session/DungeonSceneLights.test.tsx \
  src/components/session/DungeonEnvironment.test.tsx
```

Expected: FAIL because the plan prop and environment component do not exist.

- [ ] **Step 4: Implement plan-driven `DungeonSceneLights`**

Render plan values when supplied and the exported current constants otherwise:

```tsx
{plan.pointLights.map((light) => (
  <pointLight
    key={light.key}
    position={light.position}
    color={light.color}
    intensity={light.intensity}
    distance={light.distance}
    decay={2}
  />
))}
```

Do not enable `castShadow`.

- [ ] **Step 5: Implement `DungeonEnvironment`**

Resolve the plan with `useMemo([scene.lighting, focus.x, focus.z])`. Report diagnostics with an effect that compares stable plan output, then render exactly:

```tsx
<>
  <DungeonSceneLights plan={plan} />
  <DungeonShell
    scene={scene}
    doors={doors}
    onDoorClick={onDoorClick}
    onFallbackReason={onShellFallbackReason}
    floorLighting={{
      exposureByCell: plan.floorExposureByCell,
      poolsByCell: plan.floorPoolsByCell,
    }}
  />
  {scene.props.map((prop, index) => (
    <AtlasPropModel
      key={`${prop.ref}-${coordToKey(prop.position)}-${index}`}
      prop={prop}
      hexSize={hexSize}
      orientation="pointy"
    />
  ))}
</>
```

Memoize the `floorLighting` wrapper so it does not churn the floor subtree every render.

- [ ] **Step 6: Replace duplicated builder/game composition**

In `SessionCanvas.tsx`, replace `DungeonSceneLights`, `DungeonShell`, and the atlas-prop map with one `DungeonEnvironment`. Pass focus from the existing `target` world position, doors, and door click.

In `DungeonPreview3D.tsx`, replace the same three leaves with one `DungeonEnvironment`. Pass the existing preview `target` X/Z, shell fallback callback, and a new lighting diagnostic callback. Render a compact builder-only banner for non-empty diagnostics; game supplies no callback.

Do not move monsters, characters, path markers, interaction overlays, cameras, or controls into the environment.

- [ ] **Step 7: Update parity/source-contract tests**

Change static source assertions to require exactly one `<DungeonEnvironment>` and no direct `<DungeonSceneLights>`, `<DungeonShell>`, or `<AtlasPropModel>` in either caller.

Update rendered tests to assert:

- reference tomb gets one low crypt ambient/directional pair;
- its existing entrance brazier creates one point light;
- preview and game use equal plans at equal focus;
- shell fallback banner still works; and
- a budget diagnostic is visible only in builder.

- [ ] **Step 8: Run Task 4 tests GREEN**

```bash
npx vitest run \
  src/components/session/DungeonSceneLights.test.tsx \
  src/components/session/DungeonEnvironment.test.tsx \
  src/components/session/SessionCanvas.test.tsx \
  src/author/preview3d/DungeonPreview3D.test.ts \
  src/author/preview3d/DungeonPreview3D.render.test.tsx
```

Expected: PASS.

- [ ] **Step 9: Commit Task 4**

```bash
git add src/components/session/DungeonEnvironment.tsx \
  src/components/session/DungeonEnvironment.test.tsx \
  src/components/session/DungeonSceneLights.tsx \
  src/components/session/DungeonSceneLights.test.tsx \
  src/components/session/SessionCanvas.tsx \
  src/components/session/SessionCanvas.test.tsx \
  src/author/preview3d/DungeonPreview3D.tsx \
  src/author/preview3d/DungeonPreview3D.test.ts \
  src/author/preview3d/DungeonPreview3D.render.test.tsx
git commit -m "feat(rendering): share authored dungeon lighting"
```

---

### Task 5: Add the lighting fixture and calibrate the integrated crypt

**Files:**
- Create: `src/author/fixtures/cryptLightingShowcase.ts`
- Create: `src/author/fixtures/cryptLightingShowcase.test.ts`
- Modify: `src/author/DungeonBuilderSandbox.tsx`
- Modify: `src/author/fixtures/cryptPropShowcase.test.ts`
- Modify only after Kirk's visual verdict: numeric values in `src/rendering/dungeonLightSources.ts` and the two crypt fill constants in `src/rendering/dungeonLighting.ts`
- Create after approval: `docs/evidence/190-dungeon-lighting/README.md`
- Create after approval: `docs/evidence/190-dungeon-lighting/builder.png`
- Create after approval: `docs/evidence/190-dungeon-lighting/game.png`
- Create after approval: `docs/evidence/190-dungeon-lighting/source-close.png`

**Interfaces:**
- Produces: `cryptLightingShowcaseDoc(): DungeonDoc` and sandbox selector `?authorFixture=crypt-lighting`.
- Consumes: the reference tomb geometry and approved eight source refs; no private asset paths or light parameters enter YAML.

- [ ] **Step 1: Write the failing fixture test**

Clone the reference tomb document rather than re-deriving walls/doors/cells. The test must assert:

```ts
const doc = cryptLightingShowcaseDoc();
expect(doc.regions.map((r) => r.lighting.intensity)).toEqual([0.6, 0.4, 0.15]);
expect(doc.walls).toEqual(referenceTombDoc().walls);
expect(doc.doors).toEqual(referenceTombDoc().doors);
expect(doc.place.filter((p) => isDungeonLightSourceRef(p.ref)).map((p) => p.ref))
  .toEqual([
    'dnd5e:props:lantern',
    'dnd5e:props:torch-ornate',
    'dnd5e:props:glowing-orb',
    'dnd5e:props:rune-marker',
  ]);
```

Place the lantern in entrance, ornate torch in hall, and orb plus rune marker far enough apart in the tomb that part of the `0.15` region remains outside every source radius.

- [ ] **Step 2: Run fixture test RED**

```bash
npx vitest run src/author/fixtures/cryptLightingShowcase.test.ts
```

Expected: FAIL because the fixture does not exist.

- [ ] **Step 3: Implement fixture and query selection**

Create the fixture by cloning `referenceTombDoc()`, changing `key` to `crypt-lighting-showcase`, changing `name` to `Crypt Lighting Showcase`, removing the original brazier/pillar placements, and adding the four source props at real cells with useful facings. Add `authorFixture=crypt-lighting` to `sandboxDocForSearch`; preserve `crypt-props` and default behavior exactly.

- [ ] **Step 4: Run all lighting-focused tests**

```bash
npx vitest run \
  src/rendering/dungeonLightSources.test.ts \
  src/rendering/dungeonLighting.test.ts \
  src/author/paletteData.test.ts \
  src/author/fixtures/cryptLightingShowcase.test.ts \
  src/components/hex-grid/syntyHexFloorHelpers.test.ts \
  src/components/hex-grid/SyntyHexFloor.test.tsx \
  src/components/session/atlasToScene3D.test.ts \
  src/components/session/DungeonSceneLights.test.tsx \
  src/components/session/DungeonEnvironment.test.tsx \
  src/components/session/DungeonShell.test.tsx \
  src/components/session/SessionCanvas.test.tsx \
  src/author/preview3d/DungeonPreview3D.test.ts \
  src/author/preview3d/DungeonPreview3D.render.test.tsx
```

Expected: PASS.

- [ ] **Step 5: Commit the fixture before visual tuning**

```bash
git add src/author/fixtures/cryptLightingShowcase.ts \
  src/author/fixtures/cryptLightingShowcase.test.ts \
  src/author/DungeonBuilderSandbox.tsx \
  src/author/fixtures/cryptPropShowcase.test.ts
git commit -m "test(author): add crypt lighting showcase"
```

- [ ] **Step 6: Start one branch Vite and inspect the builder**

```bash
npm run dev -- --host 127.0.0.1 --port 3013 --strictPort
```

Open:

```text
http://127.0.0.1:3013/?concept=dungeon-builder&authorFixture=crypt-lighting
```

Inspect all three regions and the close source view. Verify the `0.15` tomb is near-black away from sources, warm and cool sources illuminate both floor and nearby 3D pieces, no permanent per-hex checkerboard returns, and no obvious source floats above/below its prop.

- [ ] **Step 7: Exercise the playable route with the same authored YAML**

Use the fixture's emitted YAML in the real `/author` route against the local stack, save it under `crypt-lighting-showcase`, and use Save & Play. Do not hand-edit the atlas or game route. Position the player/camera at the same focus used for the builder comparison.

- [ ] **Step 8: Present one visual gate to Kirk**

Capture temporary candidate frames outside Git first:

```bash
node /home/kirk/game-dev/tools/browser/screenshot.mjs \
  'http://127.0.0.1:3013/?concept=dungeon-builder&authorFixture=crypt-lighting' \
  /tmp/crypt-lighting-builder.png
```

Also capture the playable session and a close source view through the same browser harness/manual camera state. Present the three frames together. Kirk judges darkness, source reach, warm/cool color, floating origins, floor readability, wall/prop response, and builder/game parity.

- [ ] **Step 9: Apply at most one focused calibration round**

If Kirk requests adjustment, change only:

- the two crypt fill intensities;
- source manifest color/intensity/distance/height/floor strength; or
- the near-black floor endpoint.

Do not add a new feature, source ref, query dial, material family, or renderer path. Update exact-value tests, rerun the focused command from Step 4, and recapture the three frames. Stop after Kirk approves or explicitly opens a separate follow-up.

- [ ] **Step 10: Commit approved evidence and exact calibration**

Write `README.md` with branch head, fixture key, source refs, exact approved constants, commands, image SHA-256 values, and Kirk's verdict. Then:

```bash
sha256sum docs/evidence/190-dungeon-lighting/*.png
git add src/rendering/dungeonLightSources.ts \
  src/rendering/dungeonLightSources.test.ts \
  src/rendering/dungeonLighting.ts \
  src/rendering/dungeonLighting.test.ts \
  src/components/hex-grid/syntyHexFloorHelpers.ts \
  src/components/hex-grid/syntyHexFloorHelpers.test.ts \
  docs/evidence/190-dungeon-lighting
git commit -m "docs(rendering): record approved crypt lighting"
```

If initial values are approved unchanged, the commit contains evidence only; do not manufacture a code diff.

---

### Task 6: Verify once, publish, and close the review round

**Files:**
- Modify only for valid final findings: files already listed in Tasks 1–5
- Update PR body with evidence links and exact test result

**Interfaces:**
- Produces: one reviewed, green web PR for Kirk; no merge and no board mutation.

- [ ] **Step 1: Run the complete local gate once before push**

```bash
npm run ci-check
```

Expected: formatting, lint, typecheck, build, production-bundle guards, and complete Vitest suite all pass. Record exact file/test counts from output. Fix any failure, rerun this full command, and use only the final successful run as evidence.

- [ ] **Step 2: Verify scope and repository hygiene**

```bash
git diff --check origin/dev...HEAD
git status --short
git diff --name-only origin/dev...HEAD
git grep -n 'castShadow\|lighting.*darkvision\|lighting.*line.of.sight' -- src/rendering src/components/session src/components/hex-grid || true
git status --short -- public
```

Expected: clean diff check; only intended source/tests/evidence changed; `git status -- public` is empty; no licensed GLB/texture/private manifest added; no mechanics or shadow behavior introduced.

- [ ] **Step 3: Push after the full gate**

```bash
git push -u origin "feat/${WEB_ISSUE}-dungeon-lighting"
```

The mandatory pre-push hook must run; never use `--no-verify`.

- [ ] **Step 4: Open one PR against `dev`**

The PR body must include:

- `Closes #${WEB_ISSUE}` as completion intent;
- parent `KirkDiggler/rpg-project#190` and design PR #299;
- exact behavior and deferred scope;
- focused and full test counts;
- builder/game/close evidence links and hashes;
- fallback and 12-light-budget behavior; and
- `— assets agent, on behalf of KirkDiggler`.

Do not mutate Project 19. Because `dev` is not the default branch, the board-management session owns any manual issue closure after merge.

- [ ] **Step 5: Request exactly one Copilot review and wait for it**

Use the repository's documented `requestReviews` GraphQL mutation with bot id `BOT_kgDOCnlnWA`, verify by reading the PR node back, and do not report readiness in the same turn. Poll with a bounded wait rather than repeated rapid API calls.

- [ ] **Step 6: Adjudicate every Copilot finding**

Read all findings before changing code. For every top-level inline finding:

- reproduce and fix valid defects on the same branch;
- check the same defect shape elsewhere;
- decline invalid findings with technical evidence;
- reply with the exact fixing commit or reason; and
- never request another review.

Run affected focused tests after each fix. If any production code changes after the full gate, rerun `npm run ci-check` once before the final push.

- [ ] **Step 7: Fresh final verification**

```bash
git status --short --branch
git rev-parse HEAD
git ls-remote origin "refs/heads/feat/${WEB_ISSUE}-dungeon-lighting"
gh pr view "$PR_NUMBER" -R KirkDiggler/rpg-dnd5e-web \
  --json state,mergeable,headRefOid,statusCheckRollup,url
```

Count top-level Copilot review comments and replies with the documented REST commands; every finding must have a response. Report the reviewed head, test counts, evidence hashes, and remaining deferred work. Stop for Kirk's merge.
