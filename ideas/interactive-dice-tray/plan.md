# Interactive Collectible 3D Dice Tray Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Extend the existing lightning-d20 proof into a production-intent, Concepts Lab–only 3D dice tray with explicit player input, automatic monster throws, stable collectible-die presets, authoritative 1–20 settlement, and a simulated roller/spectator release.

**Architecture:** `DiceTray3D` owns tray layout and accessible pointer/button interaction; `AttackDie3D` remains the visual renderer and settles only from authoritative results. A small allowlisted preset registry separates collectible die identity from model/material implementation, while a compact presentation-release value lets the Concepts Lab coordinate roller and spectator panes without streaming pointer movement. Production combat wiring, real transport, loadout/ownership projection, persistence, and damage dice remain separate promotion work.

**Tech Stack:** React 19, TypeScript 5.8, Three.js / React Three Fiber, Vitest, Testing Library, Playwright/Chromium, existing Vite Concepts Lab.

**Spec:** `rpg-project/ideas/interactive-dice-tray/design.md`

## Global Constraints

- Work in `/home/kirk/game-dev/rpg-dnd5e-web/.worktrees/attack-die-749` on `feat/749-attack-die-3d-concept`; preserve its current uncommitted, user-approved roll/camera/result-10 work.
- Track implementation through `rpg-dnd5e-web#749` / PR `#750` and link `rpg-project#219`, design PR `rpg-project#220`, `rpg-game-assets#47`, and `rpg-game-assets#49` in durable handoff/evidence.
- Concepts Lab route remains `http://127.0.0.1:3002/?concept=attack-die-3d`.
- Do not wire `CombatPresentation`, `EncounterView`, the production FIFO, or `useBeatSequencer` in this plan. In particular, do not remove the existing production 1.5-second auto-throw timeout here; that is a production-promotion gate.
- Do not change toolkit, API, proto, deployment, account/profile, inventory, or persistence code.
- Do not create or display client-generated authoritative outcomes. Gesture and replay data may change only decorative motion; `authoritativeResult` alone selects the target face.
- Prototype one d20 only. Keep the tray API list-shaped for later groups, but do not render or fabricate damage dice from `EntityDamaged.amount`.
- Player mode waits indefinitely for Roll or grab/release. Monster mode may auto-release. Spectator panes never expose input.
- Do not stream pointer movement. Roller and spectator receive one compact, sanitized release value in the concept simulation.
- Presets are stable die identities, not `player`/`monster` booleans. Spectators render the roller's selected preset.
- Only allowlisted preset/model contracts may resolve. No arbitrary asset URL may enter from props or a release value.
- A mapped physical face must be hash-bound to the exact inspected GLB. An unknown preset, hash mismatch, unmapped result, load/shader/context failure, or invalid result fails closed to the existing semantic SVG surface.
- Current known lightning GLB SHA-256 is `8a8e50995ee790481e6d1f4b58919f1acee169398acd783af28376464160c1aa`; a different digest invalidates the provisional face map.
- Licensed GLBs, source assets, calibration contact sheets, screenshots, and GIFs remain private/untracked. Commit only code, tests, public-safe docs, and numeric contract data.
- Keep the existing production-intent renderer lifecycle and exact-target tolerance (`<= 0.25°`, then hold exact quaternion).
- Reduced motion retains explicit player input and exact settlement while suppressing tumble/effect animation.
- Test-first for every behavior change: observe the new test fail for the intended reason, implement minimally, then rerun focused and protected suites.
- Do not stage `.pi/`, private evidence, or unrelated worktree changes.

## File Structure

### New web files

- `src/components/ui/dice/attackDiePreset.ts` — allowlisted stable preset IDs and model/material/scale contract lookup.
- `src/components/ui/dice/attackDiePreset.test.ts` — preset identity, allowlist, safe-default, and shared-model assertions.
- `src/components/ui/dice/dicePresentationRelease.ts` — compact, sanitized presentation-only release value and gesture quantization.
- `src/components/ui/dice/dicePresentationRelease.test.ts` — clamping, determinism, authority exclusion, and duplicate-key behavior.
- `src/components/ui/dice/DiceTray3D.tsx` — production-intent tray layout, Roll/grab interaction, monster auto-release, and renderer composition.
- `src/components/ui/dice/DiceTray3D.test.tsx` — player/monster/spectator, pointer cancellation, outside release, one-shot commit, fallback, and list-boundary tests.
- `src/concepts/attack-die-3d/attackDieProvisionalFaceMap.ts` — hash-bound, explicitly provisional source-face normals and calibrated 1–20 quaternion tuples.
- `src/concepts/attack-die-3d/attackDieProvisionalFaceMap.test.ts` — completeness, normalization, hash binding, unique results, and +Y face-normal assertions.
- `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx` — Concepts Lab controls and local roller/spectator coordination.
- `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx` — shared preset/result/release and non-stalling concept behavior.

### Modified web files

- `src/components/ui/dice/attackDieVisualConfig.ts` and `.test.ts` — approved 60° camera and review scale reduced from `1.4` to `1.1`.
- `src/components/ui/dice/attackDieMaterial.ts` and `.test.ts` — preset-selected lightning versus provisional crypt treatment.
- `src/components/ui/dice/attackDieMotion.ts` and `.test.ts` — armed/static, rolling, settled, and release-variation motion without changing final pose.
- `src/components/ui/dice/AttackDie3D.tsx` and `.test.tsx` — preset resolution, phase-aware rendering, gesture-derived decorative input, and scale multiplier.
- `src/concepts/attack-die-3d/attackDieExperiment.ts` and `.test.ts` — use the hash-bound provisional map instead of a result-10-only hardcode.
- `src/concepts/attack-die-3d/AttackDie3DConcept.tsx` and `.test.tsx` — add Tray stage and pass provider data into the focused panel.
- `public/themes/base.css` — tray/review surface, grab state, side-by-side layout, responsive sizing, and reduced-motion styles.
- `docs/how-to/attack-die-3d-concept.md` — controls, provisional preset/map status, and explicit production exclusions.

---

### Task 0: Checkpoint the Approved Roll Foundation and Try Scale 1.1

**Files:**
- Modify: `src/components/ui/dice/attackDieVisualConfig.test.ts`
- Modify: `src/components/ui/dice/attackDieVisualConfig.ts`
- Modify: `src/concepts/attack-die-3d/attackDieExperiment.test.ts`
- Checkpoint all currently modified approved prototype files listed by `git status --short`

**Interfaces:**
- Consumes: current local right-to-left trajectory, 440×360 surface, 60° three-quarter camera, geometry-derived result-10 pose, and browser-approved settlement.
- Produces: a clean committed baseline with `ATTACK_DIE_VISUAL_CONFIG.dieScale === 1.1` for later tasks and no loss of the uncommitted approved work.

- [ ] **Step 1: Record and protect the existing dirty baseline**

Run:

```bash
cd /home/kirk/game-dev/rpg-dnd5e-web/.worktrees/attack-die-749
git status --short
git diff --check
npm run test:run -- \
  src/components/ui/dice/attackDieVisualConfig.test.ts \
  src/components/ui/dice/attackDieMotion.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/concepts/attack-die-3d/attackDieExperiment.test.ts \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx
npm run typecheck
npm run lint -- --quiet
```

Expected: the 11 already modified tracked files remain visible; 48 focused tests pass; typecheck, lint, and diff check exit zero. Do not reset or create a new worktree before this checkpoint is committed.

- [ ] **Step 2: Write the failing 1.1-scale assertions**

Change the visual-config expectation to:

```ts
expect(ATTACK_DIE_VISUAL_CONFIG.dieScale).toBe(1.1);
```

Change the experiment-default expectation to:

```ts
dieScale: 1.1,
```

- [ ] **Step 3: Run the focused tests to verify RED**

Run:

```bash
npm run test:run -- \
  src/components/ui/dice/attackDieVisualConfig.test.ts \
  src/concepts/attack-die-3d/attackDieExperiment.test.ts
```

Expected: FAIL because the implementation still reports `1.4`.

- [ ] **Step 4: Apply the minimal visual change**

Set only:

```ts
dieScale: 1.1,
```

in `ATTACK_DIE_VISUAL_CONFIG`. Do not change the approved camera, viewport, trajectory, or result-10 quaternion in this step.

- [ ] **Step 5: Verify the approved foundation and checkpoint it**

Run the Step 1 commands again. Then capture a local, untracked settled screenshot at result 10 and confirm the full die remains readable and inside the review surface.

Commit all approved existing prototype changes plus the scale adjustment:

```bash
git add \
  public/themes/base.css \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/AttackDie3D.tsx \
  src/components/ui/dice/attackDieMotion.test.ts \
  src/components/ui/dice/attackDieMotion.ts \
  src/components/ui/dice/attackDieVisualConfig.test.ts \
  src/components/ui/dice/attackDieVisualConfig.ts \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.tsx \
  src/concepts/attack-die-3d/attackDieExperiment.test.ts \
  src/concepts/attack-die-3d/attackDieExperiment.ts
git commit -m "feat: prototype authoritative lightning d20 roll (#749)"
```

Expected: `.pi/` remains untracked and unstaged.

---

### Task 1: Publish a Hash-Bound Provisional 1–20 Face Map

**Files:**
- Create: `src/concepts/attack-die-3d/attackDieProvisionalFaceMap.ts`
- Create: `src/concepts/attack-die-3d/attackDieProvisionalFaceMap.test.ts`
- Modify: `src/concepts/attack-die-3d/attackDieExperiment.ts`
- Modify: `src/concepts/attack-die-3d/attackDieExperiment.test.ts`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.tsx`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx`

**Interfaces:**
- Consumes: inspected lightning GLB hash and the existing `QuaternionTuple` coordinate contract (`xyzw`, right-handed, +Y up).
- Produces:

```ts
export const PROVISIONAL_LIGHTNING_GLB_SHA256: string;
export const PROVISIONAL_LIGHTNING_FACE_MAP: ReadonlyArray<{
  result: number;
  quaternion: QuaternionTuple;
}>;
export function provisionalLightningFacesForDigest(
  digest: string
): typeof PROVISIONAL_LIGHTNING_FACE_MAP | readonly [];
```

- [ ] **Step 1: Write completeness, geometry, and invalidation tests**

Create tests that assert:

```ts
expect(PROVISIONAL_LIGHTNING_FACE_MAP).toHaveLength(20);
expect(PROVISIONAL_LIGHTNING_FACE_MAP.map((face) => face.result)).toEqual(
  Array.from({ length: 20 }, (_, index) => index + 1)
);
expect(
  provisionalLightningFacesForDigest(PROVISIONAL_LIGHTNING_GLB_SHA256)
).toBe(PROVISIONAL_LIGHTNING_FACE_MAP);
expect(provisionalLightningFacesForDigest('0'.repeat(64))).toEqual([]);
```

For every result, apply the quaternion to that result's inspected source normal and assert it reaches browser +Y:

```ts
const observed = sourceNormal.clone().applyQuaternion(
  new Quaternion(...face.quaternion)
);
expect(observed.x).toBeCloseTo(0, 5);
expect(observed.y).toBeCloseTo(1, 5);
expect(observed.z).toBeCloseTo(0, 5);
expect(Math.hypot(...face.quaternion)).toBeCloseTo(1, 6);
```

Also preserve the existing exact result-10 tuple assertion so the approved pose cannot drift.

- [ ] **Step 2: Run the new test to verify RED**

Run:

```bash
npm run test:run -- src/concepts/attack-die-3d/attackDieProvisionalFaceMap.test.ts
```

Expected: FAIL because the module does not exist.

- [ ] **Step 3: Add the inspected source-normal table and derivation**

Use these browser-coordinate normals, mapped from the inspected numeral-face clusters:

```ts
const SOURCE_NORMAL_BY_RESULT = {
  1: [-0.187602, 0.794679, 0.577313],
  2: [-0.303723, 0.18762, -0.934105],
  3: [0.490818, 0.794853, -0.3568],
  4: [-0.60722, 0.794534, -0.000075],
  5: [0.303631, -0.187494, -0.93416],
  6: [-0.794747, -0.187289, 0.577321],
  7: [0.982442, -0.186565, 0.000928],
  8: [0.187666, -0.794645, 0.57734],
  9: [-0.794663, -0.187615, -0.577332],
  10: [0.491124, 0.794645, 0.356841],
  11: [-0.491125, -0.794651, -0.356829],
  12: [0.794607, 0.187566, -0.577424],
  13: [-0.187459, 0.794728, -0.577293],
  14: [-0.982267, 0.187487, 0.000037],
  15: [0.794651, 0.187536, 0.577373],
  16: [-0.303469, 0.187416, 0.934228],
  17: [0.607728, -0.794146, -0.000299],
  18: [-0.491176, -0.794624, 0.356817],
  19: [0.303439, -0.187461, 0.934229],
  20: [0.187704, -0.794706, -0.577243],
} as const satisfies Record<number, Vector3Tuple>;
```

Build normalized candidate poses by aligning each source normal to +Y, then applying a yaw around +Y. Start from this complete, deterministic review table:

```ts
const INITIAL_READABILITY_YAW_DEGREES = {
  1: 0,
  2: 0,
  3: 0,
  4: 0,
  5: 0,
  6: 0,
  7: 0,
  8: 0,
  9: 0,
  10: 225,
  11: 0,
  12: 0,
  13: 0,
  14: 0,
  15: 0,
  16: 0,
  17: 0,
  18: 0,
  19: 0,
  20: 0,
} as const satisfies Record<number, number>;
```

Result 10's `225` degrees reproduces the approved tuple. Zero is an explicit initial review value, not a readability claim; Step 4 replaces any unreadable value with the reviewed numeric yaw. No result may be absent. Keep this module under `concepts/` and label every export provisional.

- [ ] **Step 4: Calibrate readable yaw without changing which face is uppermost**

Use the existing Calibrate controls and the 60° three-quarter camera for each result 1→20:

1. select the result;
2. confirm the intended numeral is on the upper face;
3. rotate only around world/up presentation orientation as needed to make the numeral readable;
4. record the resulting normalized `xyzw` tuple explicitly in `PROVISIONAL_LIGHTNING_FACE_MAP`;
5. re-run the source-normal-to-+Y assertion.

Keep contact sheets/screenshots under `/tmp` or `.verification`, never Git. A tuple is accepted only if both top-face geometry and human readability agree. Do not infer human approval for the canonical asset contract; label this as concept-only provisional calibration.

- [ ] **Step 5: Bind the concept provider to the exact hash**

Replace the result-10-only hardcode with:

```ts
faces: provisionalLightningFacesForDigest(digest),
```

Only import provisional faces into experiment state when the digest equals `PROVISIONAL_LIGHTNING_GLB_SHA256`. If it differs, pass no calibration pose in Roll/Appearance/Tray stages so `AttackDie3D` uses SVG fallback. The Calibrate stage may still manipulate an explicitly provisional unsaved pose, but it must not call it mapped.

- [ ] **Step 6: Verify GREEN and protected contract tests**

Run:

```bash
npm run test:run -- \
  src/concepts/attack-die-3d/attackDieProvisionalFaceMap.test.ts \
  src/concepts/attack-die-3d/attackDieExperiment.test.ts \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx \
  src/components/ui/dice/attackDieContract.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx
```

Expected: all tests pass; results 1–20 are mapped only for the exact known digest; mismatch exercises fallback.

- [ ] **Step 7: Commit**

```bash
git add \
  src/concepts/attack-die-3d/attackDieProvisionalFaceMap.ts \
  src/concepts/attack-die-3d/attackDieProvisionalFaceMap.test.ts \
  src/concepts/attack-die-3d/attackDieExperiment.ts \
  src/concepts/attack-die-3d/attackDieExperiment.test.ts \
  src/concepts/attack-die-3d/AttackDie3DConcept.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx
git commit -m "feat: map provisional lightning d20 faces (#749)"
```

---

### Task 2: Add Stable Collectible-Die Presets and a Crypt Skin

**Files:**
- Create: `src/components/ui/dice/attackDiePreset.ts`
- Create: `src/components/ui/dice/attackDiePreset.test.ts`
- Modify: `src/components/ui/dice/attackDieMaterial.ts`
- Modify: `src/components/ui/dice/attackDieMaterial.test.ts`
- Modify: `src/components/ui/dice/AttackDie3D.tsx`
- Modify: `src/components/ui/dice/AttackDie3D.test.tsx`

**Interfaces:**
- Consumes: existing allowlisted lightning model/runtime contract and `AttackDieMaterialMode`.
- Produces:

```ts
export type AttackDiePresetId = 'lightning' | 'crypt';
export type AttackDieMaterialTreatment = 'lightning' | 'crypt';
export interface AttackDiePreset {
  id: AttackDiePresetId;
  displayName: string;
  familyId: string;
  dieKind: 'd20';
  modelContractId: 'lightning-d20';
  materialTreatment: AttackDieMaterialTreatment;
  scaleMultiplier: number;
}
export const DEFAULT_ATTACK_DIE_PRESET_ID: AttackDiePresetId;
export function resolveAttackDiePreset(id: string): AttackDiePreset | undefined;
```

`AttackDie3DProps` gains:

```ts
presetId?: string;
```

with `lightning` as the compatibility default.

- [ ] **Step 1: Write failing registry tests**

Assert that:

```ts
expect(resolveAttackDiePreset('lightning')).toMatchObject({
  id: 'lightning',
  modelContractId: 'lightning-d20',
  materialTreatment: 'lightning',
});
expect(resolveAttackDiePreset('crypt')).toMatchObject({
  id: 'crypt',
  modelContractId: 'lightning-d20',
  materialTreatment: 'crypt',
});
expect(resolveAttackDiePreset('https://example.com/die.glb')).toBeUndefined();
```

Also assert both first-slice presets share geometry/face map but have different stable IDs and treatments.

- [ ] **Step 2: Run to verify RED**

```bash
npm run test:run -- src/components/ui/dice/attackDiePreset.test.ts
```

Expected: FAIL because the registry module does not exist.

- [ ] **Step 3: Implement the frozen allowlisted registry**

Create a deeply immutable two-entry registry. Use player-neutral identities:

```ts
lightning: {
  id: 'lightning',
  displayName: 'Stormforged',
  familyId: 'stormforged',
  dieKind: 'd20',
  modelContractId: 'lightning-d20',
  materialTreatment: 'lightning',
  scaleMultiplier: 1,
},
crypt: {
  id: 'crypt',
  displayName: 'Cryptstone',
  familyId: 'cryptstone',
  dieKind: 'd20',
  modelContractId: 'lightning-d20',
  materialTreatment: 'crypt',
  scaleMultiplier: 1,
},
```

These names are provisional Concepts Lab copy, not asset-catalog truth. The registry contains no URL supplied by the caller.

- [ ] **Step 4: Write failing material-treatment tests**

Extend `patchAttackDieMaterials` with a final treatment argument. Use real `MeshStandardMaterial` values and assert:

```ts
const crypt = patchAttackDieMaterials(
  [body, numeral],
  'raw',
  false,
  selectors,
  'crypt'
);
expect((crypt.body as MeshStandardMaterial).color.getHexString()).toBe('24272b');
expect((crypt.numeral as MeshStandardMaterial).color.getHexString()).toBe(
  'd8cfb2'
);
```

Also assert lightning preserves source colors and crypt magical/reduced-motion behavior still obeys the existing shader-time contract.

- [ ] **Step 5: Run material tests to verify RED**

```bash
npm run test:run -- src/components/ui/dice/attackDieMaterial.test.ts
```

Expected: FAIL because treatment is ignored/not accepted.

- [ ] **Step 6: Implement the crypt treatment minimally**

For `crypt`, require color-capable standard materials and apply:

```ts
body.color.set('#24272b');
body.roughness = 0.9;
body.metalness = 0.05;
numeral.color.set('#d8cfb2');
numeral.emissive.set('#4d563c');
numeral.emissiveIntensity = reducedMotion ? 0.08 : 0.12;
```

Keep lightning behavior unchanged. Include treatment and reduced-motion state in `customProgramCacheKey`. Continue cloning/disposal ownership exactly once; add a test proving both treated clones dispose with the token.

- [ ] **Step 7: Resolve presets inside `AttackDie3D`**

Resolve `presetId` at the renderer boundary, pass `materialTreatment` into material patching, and multiply the configured base scale by `scaleMultiplier`. Unknown IDs call `fail('unknown attack die preset', 'unknown-preset')` and render only fallback. Add `'unknown-preset'` to `AttackDieFailureCode`.

The model remains the existing allowlisted runtime in this slice; do not generalize `asset.url` to arbitrary strings.

- [ ] **Step 8: Verify GREEN**

```bash
npm run test:run -- \
  src/components/ui/dice/attackDiePreset.test.ts \
  src/components/ui/dice/attackDieMaterial.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx
npm run typecheck
```

Expected: both presets render from the same model contract with distinct treatments; unknown preset fails closed.

- [ ] **Step 9: Commit**

```bash
git add \
  src/components/ui/dice/attackDiePreset.ts \
  src/components/ui/dice/attackDiePreset.test.ts \
  src/components/ui/dice/attackDieMaterial.ts \
  src/components/ui/dice/attackDieMaterial.test.ts \
  src/components/ui/dice/AttackDie3D.tsx \
  src/components/ui/dice/AttackDie3D.test.tsx
git commit -m "feat: add collectible attack die presets (#749)"
```

---

### Task 3: Make the Renderer Phase-Aware and Gesture-Variable

**Files:**
- Modify: `src/components/ui/dice/attackDieMotion.ts`
- Modify: `src/components/ui/dice/attackDieMotion.test.ts`
- Modify: `src/components/ui/dice/AttackDie3D.tsx`
- Modify: `src/components/ui/dice/AttackDie3D.test.tsx`

**Interfaces:**
- Consumes: `DiceTrayPhase`, exact target quaternion, preset scale, and a sanitized release value from Task 4's declared shape (define the shared type in this task only as the import target; Task 4 implements its constructor).
- Produces:

```ts
export interface AttackDieDecorativeRelease {
  variation: number;
  vector: readonly [number, number];
  shake: number;
}

export function attackDiePoseForPhase(input: {
  phase: DiceTrayPhase;
  elapsedMs: number;
  reducedMotion: boolean;
  current: QuaternionTuple;
  target: QuaternionTuple;
  release?: AttackDieDecorativeRelease;
}): AttackDieMotionFrame & { translation: AttackDieTranslation };
```

`AttackDie3DProps` gains:

```ts
decorativeRelease?: AttackDieDecorativeRelease;
```

- [ ] **Step 1: Write failing phase tests**

Add tests proving:

```ts
expect(attackDiePoseForPhase({ ...base, phase: 'ready' })).toMatchObject({
  observeNow: false,
  exactTargetHeld: false,
  translation: [0, 0, 0],
});
expect(
  attackDiePoseForPhase({ ...base, phase: 'settled' }).quaternion
).toEqual(target);
```

The `ready` quaternion must be a fixed neutral display pose that is not derived from `target`, preventing an armed tray from revealing the authoritative face. `settled` must hold the exact target at the in-tray resting position.

Add a decorative-only assertion:

```ts
expect(midA.translation).not.toEqual(midB.translation);
expect(settledA.translation).toEqual(settledB.translation);
expect(settledA.quaternion).toEqual(target);
expect(settledB.quaternion).toEqual(target);
```

where releases differ but authoritative result/target do not.

- [ ] **Step 2: Run to verify RED**

```bash
npm run test:run -- src/components/ui/dice/attackDieMotion.test.ts
```

Expected: FAIL because phase-aware API does not exist.

- [ ] **Step 3: Implement phase-aware motion**

Use these semantics:

- `hidden`: renderer is not mounted by `AttackDie3D`.
- `entering` / `ready`: fixed neutral quaternion, centered `[0, 0, 0]`, no observation.
- `rolling`: existing 2-second deterministic tumble/convergence and right-to-left translation.
- `settled`: exact target quaternion and exact resting translation.
- `exiting`: exact target held while the tray's CSS exit treatment runs.

Release vector/shake may alter only pre-settle hop, z-depth, and decorative seed. Clamp its contribution even though Task 4 sanitizes it; defense in depth prevents an imported caller from moving the resting pose.

- [ ] **Step 4: Add failing renderer integration tests**

Mock frames and assert:

1. `phase="ready"` copies the same neutral pose for result 1 and result 20;
2. switching to `phase="rolling"` resets elapsed motion once;
3. `phase="settled"` immediately copies exact target and resting position;
4. two release values produce different mid-roll `position.set` calls but identical final target calls;
5. reduced motion remains armed until phase changes, then settles exactly.

- [ ] **Step 5: Run renderer test to verify RED**

```bash
npm run test:run -- src/components/ui/dice/AttackDie3D.test.tsx
```

Expected: FAIL because `RuntimeDie` still rolls regardless of phase.

- [ ] **Step 6: Wire phase through `RuntimeDie`**

Reset its start timestamp and rendered quaternion when entering a new rolling presentation. Do not remount or replace the renderer lock merely because phase changes. Apply `attackDiePoseForPhase` from the frame callback. Preserve token-staleness, exact-target observation, lifecycle release, shader time, and fail-closed behavior.

- [ ] **Step 7: Verify GREEN and lifecycle protection**

```bash
npm run test:run -- \
  src/components/ui/dice/attackDieMotion.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/attackDieRendererLifecycle.test.ts \
  src/components/ui/dice/attackDieRenderGate.test.ts
```

Expected: all pass; armed phase does not reveal or auto-roll; final pose remains exact.

- [ ] **Step 8: Commit**

```bash
git add \
  src/components/ui/dice/attackDieMotion.ts \
  src/components/ui/dice/attackDieMotion.test.ts \
  src/components/ui/dice/AttackDie3D.tsx \
  src/components/ui/dice/AttackDie3D.test.tsx
git commit -m "feat: add armed and settled die phases (#749)"
```

---

### Task 4: Define the Compact Presentation Release Contract

**Files:**
- Create: `src/components/ui/dice/dicePresentationRelease.ts`
- Create: `src/components/ui/dice/dicePresentationRelease.test.ts`

**Interfaces:**
- Consumes: stable `AttackDiePresetId` from Task 2.
- Produces:

```ts
export interface DicePresentationRelease extends AttackDieDecorativeRelease {
  schemaVersion: 1;
  presentationId: string;
  presetId: AttackDiePresetId;
}

export interface DiceGestureSample {
  origin: readonly [number, number];
  current: readonly [number, number];
  distance: number;
}

export function createDicePresentationRelease(input: {
  presentationId: string;
  presetId: string;
  gesture?: DiceGestureSample;
  variation: number;
}): DicePresentationRelease;

export function releaseKey(value: DicePresentationRelease): string;
```

- [ ] **Step 1: Write failing release-contract tests**

Assert exact behavior:

- blank presentation IDs throw;
- unknown preset IDs throw;
- variation is finite, truncated, absolute, and reduced modulo `997`;
- vector components are finite and clamped to `[-1, 1]` after dividing pointer delta by `160` pixels;
- shake is `clamp(distance / 240, 0, 1)`;
- absent gesture produces vector `[0, 0]` and shake `0`;
- repeated input produces deep-equal output;
- the serialized value contains none of `result`, `hit`, `damage`, `target`, or arbitrary URL fields;
- `releaseKey` is `${presentationId}:${variation}`.

- [ ] **Step 2: Run to verify RED**

```bash
npm run test:run -- src/components/ui/dice/dicePresentationRelease.test.ts
```

Expected: FAIL because the module does not exist.

- [ ] **Step 3: Implement strict quantization**

Use a local finite/clamp helper and return an `Object.freeze`d value. Do not include timestamps or randomness in this constructor; the caller supplies a variation counter so roller and spectator can replay the same compact value.

- [ ] **Step 4: Verify GREEN**

```bash
npm run test:run -- \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/attackDieMotion.test.ts
```

Expected: release values are deterministic, bounded, and presentation-only.

- [ ] **Step 5: Commit**

```bash
git add \
  src/components/ui/dice/dicePresentationRelease.ts \
  src/components/ui/dice/dicePresentationRelease.test.ts
git commit -m "feat: define shared dice release presentation (#749)"
```

---

### Task 5: Build the Production-Intent `DiceTray3D` Interaction Component

**Files:**
- Create: `src/components/ui/dice/DiceTray3D.tsx`
- Create: `src/components/ui/dice/DiceTray3D.test.tsx`
- Modify: `public/themes/base.css`

**Interfaces:**
- Consumes: `AttackDie3D`, `AttackDiePresetId`, `DicePresentationRelease`, and existing authoring overrides for the Concepts Lab.
- Produces:

```ts
export type DiceTrayRollerRole = 'player' | 'monster';
export type DiceTrayWitnessRole = 'roller' | 'spectator';

export interface DiceTray3DItem {
  id: string;
  kind: 'd20';
  presetId: string;
  authoritativeResult: number;
  presentationToken: number;
}

export interface DiceTray3DProps {
  presentationId: string;
  rollerRole: DiceTrayRollerRole;
  witnessRole: DiceTrayWitnessRole;
  phase: 'armed' | 'rolling' | 'settled';
  dice: readonly DiceTray3DItem[];
  release?: DicePresentationRelease;
  reducedMotion?: boolean;
  onReleaseRequest?: (release: DicePresentationRelease) => void;
  onTelemetry?: (event: AttackDieTelemetry) => void;
  sceneOverride?: AttackDie3DProps['sceneOverride'];
  sidecarOverride?: AttackDie3DProps['sidecarOverride'];
}
```

The first implementation validates exactly one `kind: 'd20'` item at runtime but keeps the prop list-shaped. Invalid cardinality renders a semantic fallback/status and never invents damage dice.

- [ ] **Step 1: Write player/monster/spectator tests first**

Mock `AttackDie3D` and assert:

1. player + roller + armed shows both `Roll Stormforged d20` and `Grab Stormforged d20`;
2. advancing fake timers by any duration does not call `onReleaseRequest`;
3. clicking Roll calls `onReleaseRequest` exactly once;
4. monster + roller + armed requests one automatic release in an effect;
5. spectator never renders Roll/grab controls and never auto-requests a release;
6. the renderer receives the supplied authoritative result and preset unchanged;
7. two dice or a non-d20 item fail closed rather than rendering a fabricated group.

- [ ] **Step 2: Run to verify RED**

```bash
npm run test:run -- src/components/ui/dice/DiceTray3D.test.tsx
```

Expected: FAIL because the component does not exist.

- [ ] **Step 3: Implement the tray shell and button path**

Render a semantic section with these stable review attributes:

```tsx
<section
  className="dice-tray-3d"
  data-phase={phase}
  data-roller-role={rollerRole}
  data-witness-role={witnessRole}
  data-preset={die.presetId}
>
```

Compose `AttackDie3D` over a distinct rounded `.dice-tray-3d__well`. Keep the 440×360 motion surface and a smaller centered tray rectangle so WebGL motion may cross the rectangle while remaining visible. Pass `phase="ready"` for armed, then rolling/settled directly. The SVG `DiceTray` fallback uses the same semantic phase/result but must show `?` while armed.

- [ ] **Step 4: Write failing pointer tests**

Use `fireEvent.pointerDown/move/up/cancel` with explicit `pointerId`. Assert:

- pointer down captures only in player/roller/armed;
- local pointer moves update `data-grabbed="true"` and a bounded CSS translate but emit no release;
- pointer up outside the tray still emits exactly one release;
- the release contains the selected preset and quantized gesture, not a result;
- a subsequent click/pointer-up cannot emit a second release for the same presentation;
- pointer cancel returns to armed with no release;
- lost capture returns to armed if release did not commit;
- unmount during grab removes local state/listeners;
- reduced motion keeps the same explicit input behavior.

Stub `setPointerCapture`, `hasPointerCapture`, and `releasePointerCapture` in jsdom rather than weakening production code.

- [ ] **Step 5: Run pointer tests to verify RED**

```bash
npm run test:run -- src/components/ui/dice/DiceTray3D.test.tsx
```

Expected: interaction assertions fail because pointer handling is absent.

- [ ] **Step 6: Implement one-shot pointer interaction**

Track only local roller state:

```ts
interface ActiveGrab {
  pointerId: number;
  origin: readonly [number, number];
  current: readonly [number, number];
  distance: number;
}
```

Use pointer capture on the grab control. Accumulate path distance on move. On pointer-up, call `createDicePresentationRelease`, clear local translation, and commit through a ref keyed by `presentationId`. Never expose move samples through props. Pointer-cancel/lost-capture returns to armed without committing. The separate Roll button uses an absent gesture and the same one-shot commit path.

- [ ] **Step 7: Add tray CSS and containment contract**

Add:

- `.dice-tray-3d` as the 440×360 motion surface;
- `.dice-tray-3d__well` as the rounded popup rectangle;
- `.dice-tray-3d__renderer` as the larger overlay layer;
- `.dice-tray-3d__grab-target` with `touch-action: none` and visible keyboard focus;
- grabbed/armed/rolling/settled data-state styles;
- responsive width using `min(100%, 440px)`;
- side-by-side-safe minimums;
- reduced-motion rules that disable transitions but not controls.

Do not set `overflow: hidden` on the motion surface or well; the WebGL canvas layer must be large enough for travel outside the rounded rectangle. The settled model's projected bounds must fit within the well at scale 1.1.

- [ ] **Step 8: Verify GREEN and existing SVG protection**

```bash
npm run test:run -- \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTray.test.tsx \
  src/components/ui/dice/AttackDie3D.test.tsx
npm run typecheck
npm run lint -- --quiet
```

Expected: player waits forever, monster auto-releases once, spectator is read-only, and fallback remains semantic.

- [ ] **Step 9: Commit**

```bash
git add \
  src/components/ui/dice/DiceTray3D.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  public/themes/base.css
git commit -m "feat: add interactive 3d dice tray (#749)"
```

---

### Task 6: Add the Side-by-Side Roller/Spectator Concepts Lab Stage

**Files:**
- Create: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx`
- Create: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.tsx`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx`
- Modify: `public/themes/base.css`

**Interfaces:**
- Consumes: provider scene/sidecar, provisional hash-bound face map, `DiceTray3D`, and compact release values.
- Produces:

```ts
export interface DiceTray3DConceptPanelProps {
  provider?: {
    digest: string;
    sidecar: AttackDieRuntimeSidecar;
    scene: Object3D;
  };
  providerError: string;
}
```

The panel owns only local concept coordination state:

```ts
type ConceptTrayPhase = 'armed' | 'rolling' | 'settled';
```

- [ ] **Step 1: Write failing concept tests**

Assert the panel exposes:

- authoritative result number input constrained to 1–20;
- preset select with Stormforged and Cryptstone;
- roller-mode select with Player and Monster;
- reduced-motion checkbox;
- Arm/reset action;
- Roller and Spectator headings;
- a visible “simulated presentation coordination” warning.

Assert both mocked trays receive the same result and preset, with witness roles `roller` and `spectator`.

- [ ] **Step 2: Run to verify RED**

```bash
npm run test:run -- src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx
```

Expected: FAIL because the panel does not exist.

- [ ] **Step 3: Implement local coordination**

On Arm/reset:

- increment the presentation token;
- clear prior release;
- set phase to armed;
- preserve the selected result/preset/mode.

On the roller tray's `onReleaseRequest`:

- store exactly that compact release;
- change both panes to rolling in the same React update;
- never copy pointer samples or generate a result.

On the roller renderer's first matching `state: 'observed'` telemetry event:

- verify token and requested result match the active presentation;
- set both panes to settled;
- ignore stale spectator/roller callbacks.

For monster mode, the roller tray's automatic release drives the same path. Unknown/missing provider data leaves both panes on semantic fallback and remains non-stalling.

- [ ] **Step 4: Add shared-release behavior tests**

Assert:

1. player remains armed after timers advance;
2. roller release causes both trays to receive the same release object and rolling phase;
3. no pointer-move callback exists between panes;
4. preset switch before arm appears in both panes;
5. result switch before arm appears in both panes;
6. monster mode produces one shared automatic release;
7. stale telemetry cannot settle a newer token;
8. matching observed telemetry settles both;
9. unknown preset/provider mismatch renders fallback without waiting.

- [ ] **Step 5: Integrate a fifth `Tray` stage**

Change:

```ts
const stages = ['Appearance', 'Calibrate', 'Roll', 'Verify', 'Tray'] as const;
```

Render `DiceTray3DConceptPanel` only for the Tray stage and keep calibration/evidence stages intact. Pass the same inspected provider object; do not load a second copy of the GLB in the panel.

Update keyboard tab-wrap tests to cover five stages.

- [ ] **Step 6: Add responsive side-by-side styling**

Use a two-column review grid above a width that fits two compact surfaces and one column below it. Each pane labels role, preset, phase, and authoritative result without claiming real networking. Do not shrink below readable numeral size merely to force two columns.

- [ ] **Step 7: Verify GREEN**

```bash
npm run test:run -- \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx
npm run typecheck
npm run lint -- --quiet
```

Expected: side-by-side concept behavior passes with one shared release and no production transport.

- [ ] **Step 8: Commit**

```bash
git add \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx \
  public/themes/base.css
git commit -m "feat: compare roller and spectator dice trays (#749)"
```

---

### Task 7: Browser-Prove Containment, Presets, Results, and Input Paths

**Files:**
- Modify: `docs/how-to/attack-die-3d-concept.md`
- Modify only if browser evidence reveals a tested defect: files owned by Tasks 1–6 and their matching tests
- Private/untracked output: `/home/kirk/game-dev/.verification/interactive-dice-tray/`

**Interfaces:**
- Consumes: completed Concepts Lab tray stage.
- Produces: repeatable reviewer instructions, private screenshots/GIFs, and an evidence matrix without claiming production transport/performance/asset approval.

- [ ] **Step 1: Add reviewer documentation first**

Document exact route and steps:

```text
?concept=attack-die-3d → Tray
```

Include:

- switch Player/Monster;
- select Stormforged/Cryptstone;
- choose authoritative result 1–20;
- Arm;
- Roll or grab/shake/release;
- compare Roller/Spectator;
- enable reduced motion;
- force fallback;
- explicit labels: provisional face map, provisional skins, simulated coordination, no production combat/network/loadout/damage-dice wiring.

- [ ] **Step 2: Run the complete focused suite**

```bash
npm run test:run -- \
  src/components/ui/dice/attackDieContract.test.ts \
  src/components/ui/dice/attackDieMaterial.test.ts \
  src/components/ui/dice/attackDieMotion.test.ts \
  src/components/ui/dice/attackDiePreset.test.ts \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTray.test.tsx \
  src/concepts/attack-die-3d/attackDieProvisionalFaceMap.test.ts \
  src/concepts/attack-die-3d/attackDieExperiment.test.ts \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx
npm run typecheck
npm run lint -- --quiet
npm run format:check
git diff --check
```

Expected: zero failures/errors. If formatting changes files, run `npm run format`, inspect the diff, and repeat the full command.

- [ ] **Step 3: Browser-check player explicit input**

At the real route:

1. open Tray, Player, Stormforged, result 10;
2. wait at least 5 seconds and confirm both panes remain armed;
3. press Roll and confirm both begin together;
4. confirm both settle on 10;
5. re-arm, drag outside the rounded rectangle, release, and confirm one throw;
6. confirm motion may cross the rectangle but the final rendered die bounds fit inside it.

Capture private screenshots for armed, outside-travel, and settled states.

- [ ] **Step 4: Browser-check presets and monster mode**

1. select Cryptstone and confirm both panes switch to the same dark-stone/bone treatment;
2. select Monster and Arm; confirm one automatic shared throw;
3. confirm spectator has no controls;
4. switch back to Stormforged and verify result does not change merely because preset changed.

- [ ] **Step 5: Browser-check all authoritative results and fallback**

For results 1→20 in both normal and reduced-motion paths:

- confirm telemetry `requestedResult` equals input;
- confirm renderer reaches `state: observed` only when the matching mapped face is uppermost;
- confirm final geometry lies inside the well;
- confirm SVG is used for a deliberately mismatched digest/unknown preset/unmapped fixture;
- record readability findings without labeling them canonical human asset approval.

Any wrong physical face is a release blocker. Add a failing automated regression test before correcting its map or renderer behavior.

- [ ] **Step 6: Generate the private progress GIF**

Capture a Player roller/spectator throw at 20 fps for roughly 2.6 seconds, crop to the concept comparison surface, and encode under:

```text
/home/kirk/game-dev/.verification/interactive-dice-tray/roller-spectator.gif
```

Copy to `~/Downloads` only for sharing. Verify with `file` and `ffprobe`; do not stage it.

- [ ] **Step 7: Run the repository protection suite**

```bash
npm run ci-checks
npm run test:run
```

Expected: formatting, lint, typecheck, build, and all Vitest suites pass. Record the exact counts and any unrelated environmental limitation; do not claim Discord/mobile/low-GPU evidence unless actually run.

- [ ] **Step 8: Verify scope and commit documentation/fixes**

Run:

```bash
git status --short
git diff --check
git diff --name-only origin/feat/749-attack-die-3d-concept...HEAD
```

Confirm there are no modifications under:

```text
src/components/game/combatPresentation/
src/components/game/EncounterView.tsx
```

and no toolkit/API/proto/deployment changes.

If browser review finds a defect, return to the owning task, add the failing regression test there, implement the fix, and use that task's explicit commit command before resuming this task. This keeps the documentation commit deterministic.

Commit only the review guide here:

```bash
git add docs/how-to/attack-die-3d-concept.md
git commit -m "docs: add interactive dice tray review guide (#749)"
```

---

### Task 8: Final Independent Review and PR Handoff

**Files:**
- Modify only in response to accepted review findings: the smallest affected source/test/doc files
- Do not modify: production combat integration files or other repositories

**Interfaces:**
- Consumes: Tasks 0–7 complete and green.
- Produces: reviewed PR #750 update with explicit residual risks and production gates.

- [ ] **Step 1: Request independent spec-compliance review**

Ask a fresh reviewer to compare implementation against:

- `rpg-project/ideas/interactive-dice-tray/design.md`;
- this plan;
- the original attack-die contract in `rpg-project/ideas/attack-die-3d/design.md`.

Require findings to cite file/line and classify blockers versus later production gates. The reviewer must specifically inspect authority separation, player no-timeout behavior, spectator read-only behavior, hash binding, one-shot pointer paths, preset allowlisting, exact settlement, and prohibited production wiring.

- [ ] **Step 2: Request independent code-quality/test review**

A second fresh reviewer checks:

- cleanup/stale callback behavior;
- pointer capture/cancel/lost-capture paths;
- renderer/material disposal;
- unknown preset and invalid cardinality fallback;
- absence of result fields from release data;
- whether tests assert user-visible behavior rather than mocks alone;
- responsive and reduced-motion behavior.

- [ ] **Step 3: Address accepted findings test-first**

For each accepted defect, add/reproduce a failing focused test, implement the minimal fix, rerun the focused suite, and request re-review. Do not accept suggestions that widen into real transport, inventory, damage dice, or production combat wiring; record those as follow-up gates.

- [ ] **Step 4: Run fresh final verification**

```bash
npm run ci-checks
npm run test:run
git diff --check
git status --short
```

Expected: all checks pass and only `.pi/`/private local artifacts remain untracked. Verify every source commit is pushed to `origin/feat/749-attack-die-3d-concept`.

- [ ] **Step 5: Update PR #750 and issue #749**

Post a concise implementation summary with:

- test/build counts and commands;
- private browser/GIF path without uploading licensed imagery to a public repo;
- explicit provisional status of face map and Cryptstone skin;
- explicit statement that roller/spectator coordination is local simulation;
- explicit absence of production combat/network/loadout/damage-dice wiring;
- links to `rpg-project#219` / PR `#220`, assets #47 and #49;
- residual production gates from the design.

End every GitHub comment with:

```text
— asset-pipeline agent, on behalf of KirkDiggler
```

Do not merge. Kirk alone merges.
