# Stone 1 Tactile Roll-Group Choreography Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the current one-d20 roll group fun to grab, shake, and release while preserving the server-selected result and independently verified asset-owned settlement.

**Architecture:** `RollGroupGestureController` owns local hit testing, pointer capture, filtered samples, held state, and terminal cleanup. It emits one strict `VisualThrowProfile@1`; `DicePresentationReleasedEvent` shares that immutable profile, while `ChoreographedSolverV1` independently maps profile + authoritative asset target + elapsed time to Three.js poses for Roller and Spectator. `DiceTrayPresentation`, `presentationId`, provider validation, final upward-result observation, and truthful SVG fallback remain authoritative and unchanged in role.

**Tech Stack:** React 19, TypeScript, Vitest/Testing Library, Three.js, React Three Fiber, Vite, Playwright/Chromium evidence scripts.

**Spec:** `ideas/interactive-dice-tray/design.md` (“Stone 1 — tactile held and personalized release choreography” and “Permanent solver boundary”)

**Tracking:** [rpg-dnd5e-web#755](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/755), [rpg-project#219](https://github.com/KirkDiggler/rpg-project/issues/219)

## Global Constraints

- Start a fresh isolated web worktree from current `origin/dev`; the approved Stone 0 base is merge `8bc2a273dba4d018f9d9c6829ca043b7cdafa60e`.
- Never cherry-pick, copy, or use rejected held-motion commits `75c56871da792262cd93a4f181284bca48c87faa` or `de2377e2d0f8b1bc2923f4c1d831ee113a443f5d` as implementation inputs.
- The gesture changes **how** dice roll, never **what** they roll. No client result generation, rerolling, bias, clamping, concealment, or reinterpretation is allowed.
- `DiceTrayPresentation` remains literal authority; `presentationId` remains the sole external correlation.
- The current d20 is a one-member roll group. Do not add multi-die, lasso, or partial-group behavior.
- Raw pointer coordinates, path history, and sample timestamps remain controller-local and must not enter events, telemetry, logs, evidence JSON, URLs, persistence, or transport.
- `VisualThrowProfile@1` contains exactly schema version, normalized release position, normalized release direction, normalized speed, shake energy, spin bias, and deterministic motion seed.
- Button/keyboard Roll uses a neutral profile. Monster release profiles remain host-produced.
- Roller and Spectator share immutable provider/event/profile facts but independently own Canvas, WebGL context, runtime clone, generation, resources, motion state, rendered world pose, upward observation, and telemetry.
- Movement applies only to Three.js die/shadow groups. Canvas and renderer CSS transforms remain forbidden.
- Final 3D success still requires exact target hold, angular error `<= 0.25°`, `observedUpwardResult === requestedResult`, `upDot > 0.999999`, and margin `> 0.2`.
- Reduced motion uses a single static lifted held cue, then reaches the authoritative target without tumble, shake, bounce, or scatter.
- Every failure or lifecycle interruption fails closed through the merged SVG fallback and cleans up pointer capture/held state exactly once.
- Consumers read private assets only through ignored `public/models/custom-dice/` synchronized from `rpg-game-assets/harness/models/custom-dice/`; never read `library/custom-dice/`.
- Preserve the exact Original carved d20: 491,312 bytes; SHA-256 `87bf2d0535023e69c968fb9878ba4ad990df4eeec4b503ebb0e917419c47a77e`; 2,684 body + 7,798 numeral triangles.
- Private GLBs, runtime manifests, screenshots, telemetry JSON, logs, hashes, frames, and evidence packages remain ignored/untracked.
- Kirk alone decides whether to merge. Agents stop after implementation, independent review, evidence, push, PR gate, and In Review status.

---

## File structure

**Create in `rpg-dnd5e-web`:**

- `src/components/ui/dice/visualThrowProfile.ts` — strict profile type, producer, neutral constructor, parser, and deep freezing.
- `src/components/ui/dice/visualThrowProfile.test.ts` — exact-key, bounds, normalization, immutability, and denied-field tests.
- `src/components/ui/dice/rollGroupGestureController.ts` — local state machine for hit testing, pointer capture, filtering, held state, release profile, and cleanup.
- `src/components/ui/dice/rollGroupGestureController.test.ts` — pointer ownership, filtering, energy, release, cancel, and hostile-input tests.
- `src/components/ui/dice/diceSettlementResolver.ts` — validates the ready provider preset/result identity and resolves the asset-owned runtime target without web-authored face facts.
- `src/components/ui/dice/diceSettlementResolver.test.ts` — preset/result mismatch, missing entry, quaternion, and immutable resolution tests.
- `src/components/ui/dice/diceMotionSolver.ts` — solver interface, one-member descriptor, pose/shadow types, quaternion utilities, and shared constants.
- `src/components/ui/dice/choreographedDiceMotion.ts` — pure deterministic `ChoreographedSolverV1`.
- `src/components/ui/dice/choreographedDiceMotion.test.ts` — held/release personality, determinism, continuity, reduced motion, and exact-target tests.
- `scripts/attack-die/stone1TrayEvidenceProtocol.ts` — strict Stone 1 semantic/package validator.
- `scripts/attack-die/stone1TrayEvidenceProtocol.test.ts` — mutation and resource-bound validation tests.
- `scripts/attack-die/capture-stone1-tray-evidence.mjs` — exact-SHA frozen-build Chromium capture.
- `scripts/attack-die/pngEvidenceValidation.ts` and test — reusable sequential, CRC/inflate/filter-validating PNG inspection with aggregate-byte preflight extracted without changing Stone 0 semantics.

**Modify in `rpg-dnd5e-web`:**

- `src/components/ui/dice/dicePresentationRelease.ts` and test — replace the provisional variation/vector/shake payload with release schema v2 carrying `VisualThrowProfile@1`.
- `src/components/ui/dice/dicePresentationEvent.ts` and test — continue strict event projection while accepting only release schema v2/profile v1.
- `src/components/ui/dice/DiceTray3D.tsx` and test — integrate the controller, bounded die hit target, held state, neutral Roll path, and terminal cleanup.
- `src/components/ui/dice/DiceTrayPresentation.tsx` and test — generate deterministic seeds, wrap exactly one profile in the release event, and preserve reconciliation/fallback semantics.
- `src/components/ui/dice/AttackDie3D.tsx` and test — consume held/profile inputs, call the solver, render the owned shadow, and retain independent final observation.
- `src/components/ui/dice/attackDieMotion.ts` and test — remove after all imports/tests migrate to the explicit solver files.
- `src/concepts/attack-die-3d/diceTrayWitnessFixture.ts` and test — produce host-owned neutral Monster profiles.
- `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx` and test — expose safe Stone 1 evidence facts and prove local-before-release/shared-after-release behavior.
- `src/concepts/attack-die-3d/AttackDie3DConcept.tsx` and test — expose the Stone 1 browser bridge without raw pointer data.
- `public/themes/base.css` — replace the whole-tray grab target/border with a die-sized hit region, grab cursor, visible focus, and no CSS motion transform.
- `scripts/attack-die/stone0TrayEvidenceProtocol.ts` and test — import extracted generic PNG validation without changing accepted Stone 0 semantics.
- `package.json` — add the Stone 1 evidence command.
- `docs/how-to/attack-die-3d-concept.md` — document the profile/solver boundary, scenarios, commands, and limitations.

---

### Task 1: Define and validate `VisualThrowProfile@1`

**Files:**
- Create: `src/components/ui/dice/visualThrowProfile.ts`
- Create: `src/components/ui/dice/visualThrowProfile.test.ts`

**Interfaces:**
- Produces:

```ts
export interface VisualThrowProfileV1 {
  readonly schemaVersion: 1;
  readonly releasePosition: readonly [number, number]; // each 0..1
  readonly releaseDirection: readonly [number, number]; // zero or unit length
  readonly releaseSpeed: number; // 0..1
  readonly shakeEnergy: number; // 0..1
  readonly spinBias: number; // -1..1
  readonly motionSeed: number; // uint32
}

export interface VisualThrowProfileInput {
  releasePosition: readonly [number, number];
  releaseDirection: readonly [number, number];
  releaseSpeed: number;
  shakeEnergy: number;
  spinBias: number;
  motionSeed: number;
}

export function createVisualThrowProfile(
  input: VisualThrowProfileInput
): VisualThrowProfileV1;
export function createNeutralVisualThrowProfile(
  motionSeed: number
): VisualThrowProfileV1;
export function parseVisualThrowProfile(
  value: unknown
): VisualThrowProfileV1 | undefined;
```

- Consumers: Tasks 2–6.

- [ ] **Step 1: Write strict producer/parser tests**

Cover these exact cases:

```ts
const neutral = createNeutralVisualThrowProfile(0x1_0000_0001);
expect(neutral).toEqual({
  schemaVersion: 1,
  releasePosition: [0.5, 0.5],
  releaseDirection: [0, 0],
  releaseSpeed: 0,
  shakeEnergy: 0,
  spinBias: 0,
  motionSeed: 1,
});
expect(Object.isFrozen(neutral)).toBe(true);
expect(Object.isFrozen(neutral.releasePosition)).toBe(true);
expect(Object.isFrozen(neutral.releaseDirection)).toBe(true);
expect(parseVisualThrowProfile(neutral)).toEqual(neutral);
```

Also require:

- producer clamps finite position/speed/energy/bias values to their documented ranges;
- producer normalizes every nonzero direction and canonicalizes near-zero direction to `[0, 0]` with speed `0`;
- producer maps finite integer seeds through unsigned `>>> 0` and rejects non-finite values;
- parser accepts only the seven exact keys and rejects arrays, accessors that throw, sparse/extra-key tuples, NaN/Infinity, non-integer/out-of-range seeds, non-unit nonzero directions outside `1e-6`, and out-of-range scalars;
- recursive denied keys (`result`, `target`, `damage`, `url`, `transport`, `presentationId`, `pointer`, `coordinate`, `path`, `timestamp`, `sample`) do not appear in the serialized profile;
- parsed tuples and object are fresh deeply frozen snapshots.

- [ ] **Step 2: Run the tests and verify RED**

Run:

```bash
npm run test:run -- src/components/ui/dice/visualThrowProfile.test.ts
```

Expected: FAIL because `visualThrowProfile.ts` does not exist.

- [ ] **Step 3: Implement exact-key snapshotting, producer normalization, and parser rejection**

Use these constants and canonicalization rules:

```ts
const PROFILE_KEYS = [
  'schemaVersion',
  'releasePosition',
  'releaseDirection',
  'releaseSpeed',
  'shakeEnergy',
  'spinBias',
  'motionSeed',
] as const;
const DIRECTION_EPSILON = 1e-9;
const UNIT_TOLERANCE = 1e-6;
const clamp = (value: number, min: number, max: number) =>
  Math.min(max, Math.max(min, value));
```

`createVisualThrowProfile` must first reject every non-finite input, clamp bounded scalars, normalize a nonzero direction using `Math.hypot`, force speed to zero for a canonical zero direction, freeze each tuple, then freeze the exact object. `parseVisualThrowProfile` must snapshot exact own keys before reading values, validate without clamping, and construct a fresh deeply frozen profile.

- [ ] **Step 4: Run focused tests and typecheck**

```bash
npm run test:run -- src/components/ui/dice/visualThrowProfile.test.ts
npm run typecheck
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/components/ui/dice/visualThrowProfile.ts \
  src/components/ui/dice/visualThrowProfile.test.ts
git commit -m "feat: define sanitized visual throw profiles (#755)"
```

---

### Task 2: Build the local roll-group gesture controller

**Files:**
- Create: `src/components/ui/dice/rollGroupGestureController.ts`
- Create: `src/components/ui/dice/rollGroupGestureController.test.ts`

**Interfaces:**
- Consumes: `createVisualThrowProfile` from Task 1.
- Produces:

```ts
export interface ClientBounds {
  readonly left: number;
  readonly top: number;
  readonly width: number;
  readonly height: number;
}
export interface RollGroupPointerSample {
  readonly pointerId: number;
  readonly clientX: number;
  readonly clientY: number;
  readonly timeMs: number;
}
export interface PointerCaptureOwner {
  setPointerCapture(pointerId: number): void;
  hasPointerCapture(pointerId: number): boolean;
  releasePointerCapture(pointerId: number): void;
}
export interface HeldRollGroupState {
  readonly normalizedPosition: readonly [number, number];
  readonly normalizedTilt: readonly [number, number];
  readonly shakeEnergy: number;
  readonly wobblePhase: number;
}
export interface RollGroupGestureStart {
  readonly sample: RollGroupPointerSample;
  readonly captureTarget: PointerCaptureOwner;
  readonly trayBounds: ClientBounds;
  readonly hitBounds: ClientBounds;
  readonly hitPaddingPx: number;
  readonly motionSeed: number;
}
export interface RollGroupGestureController {
  begin(input: RollGroupGestureStart): HeldRollGroupState | undefined;
  move(sample: RollGroupPointerSample): HeldRollGroupState | undefined;
  release(sample: RollGroupPointerSample): VisualThrowProfileV1 | undefined;
  cancel(pointerId: number): boolean;
  reset(): void;
  held(): HeldRollGroupState | undefined;
}
export function createRollGroupGestureController(): RollGroupGestureController;
```

- Consumers: Task 4 `DiceTray3D`; Task 3 solver consumes only `HeldRollGroupState`.

- [ ] **Step 1: Write controller state-machine tests**

Build a fake `PointerCaptureOwner` recording set/has/release calls. Test:

1. start outside expanded hit bounds returns `undefined` and never captures;
2. mouse padding 14 px and touch padding 24 px are selected by the caller and honored exactly;
3. capture is accepted only when `hasPointerCapture(pointerId)` returns true after `setPointerCapture`;
4. accepted begin returns centered normalized position for a centered point and zero tilt/energy;
5. matching moves update normalized position, filtered velocity/tilt, wobble phase, and monotonically bounded shake energy;
6. repeated back-and-forth movement has greater energy than one straight segment;
7. wrong-pointer move/release is ignored without mutating state;
8. matching release returns a deeply frozen valid profile and clears/releases capture once;
9. cancel, reset, lost-capture simulation, identity interruption, and capture API exceptions leave no held state and never throw;
10. output/profile serialization contains no client coordinates, time, path, or pointer ID.

Use deterministic samples:

```ts
const start = { pointerId: 7, clientX: 100, clientY: 100, timeMs: 0 };
controller.begin({
  sample: start,
  captureTarget,
  trayBounds: { left: 0, top: 0, width: 200, height: 160 },
  hitBounds: { left: 60, top: 40, width: 80, height: 80 },
  hitPaddingPx: 14,
  motionSeed: 0x1234,
});
controller.move({ pointerId: 7, clientX: 132, clientY: 84, timeMs: 16 });
const profile = controller.release({
  pointerId: 7,
  clientX: 148,
  clientY: 92,
  timeMs: 32,
});
expect(parseVisualThrowProfile(profile)).toEqual(profile);
```

- [ ] **Step 2: Run tests and verify RED**

```bash
npm run test:run -- src/components/ui/dice/rollGroupGestureController.test.ts
```

Expected: FAIL because the controller module does not exist.

- [ ] **Step 3: Implement the bounded filter and state machine**

Use exact math so tests and both browsers agree:

```ts
const dtSeconds = clamp((next.timeMs - previous.timeMs) / 1000, 1 / 240, 0.1);
const delta = [nextX - previousX, nextY - previousY] as const;
const instantVelocity = [
  clamp(delta[0] / dtSeconds, -3, 3),
  clamp(delta[1] / dtSeconds, -3, 3),
] as const;
const filteredVelocity = [
  previousVelocity[0] * 0.65 + instantVelocity[0] * 0.35,
  previousVelocity[1] * 0.65 + instantVelocity[1] * 0.35,
] as const;
const segmentDistance = Math.hypot(...delta);
const shakeEnergy = clamp(previousEnergy + segmentDistance * 0.9, 0, 1);
const normalizedTilt = [
  clamp(-filteredVelocity[1] / 2.4, -1, 1),
  clamp(filteredVelocity[0] / 2.4, -1, 1),
] as const;
const wobblePhase = (previousWobblePhase + segmentDistance * 3) % 1;
```

Accumulate signed turn from consecutive normalized deltas with the 2D cross product. On release, derive direction from filtered velocity (canonical zero below speed `0.02`), `releaseSpeed = clamp(speed / 2.4, 0, 1)`, and `spinBias = clamp(accumulatedTurn * 6 + directionX * 0.2, -1, 1)`. Normalize point coordinates to tray bounds and clamp each to `0..1`.

The controller must hold raw samples only inside its closure. Freeze every returned held tuple/object. Clear internal state before attempting capture release so reentrant/throwing capture APIs cannot duplicate a terminal release.

- [ ] **Step 4: Run focused tests and typecheck**

```bash
npm run test:run -- \
  src/components/ui/dice/visualThrowProfile.test.ts \
  src/components/ui/dice/rollGroupGestureController.test.ts
npm run typecheck
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/components/ui/dice/rollGroupGestureController.ts \
  src/components/ui/dice/rollGroupGestureController.test.ts
git commit -m "feat: control local roll-group gestures (#755)"
```

---

### Task 3: Introduce `DiceMotionSolver` and `ChoreographedSolverV1`

**Files:**
- Create: `src/components/ui/dice/diceSettlementResolver.ts`
- Create: `src/components/ui/dice/diceSettlementResolver.test.ts`
- Create: `src/components/ui/dice/diceMotionSolver.ts`
- Create: `src/components/ui/dice/choreographedDiceMotion.ts`
- Create: `src/components/ui/dice/choreographedDiceMotion.test.ts`

**Interfaces:**
- Consumes: `VisualThrowProfileV1`, `HeldRollGroupState`, `DiceTrayPhase`, `QuaternionTuple`, and validated `DiceRuntimePreset` values from `DiceRuntimeProvider`.
- Produces:

```ts
export interface DiceSettlementResolution {
  readonly presetId: string;
  readonly authoritativeResult: number;
  readonly target: QuaternionTuple;
  readonly entry: DiceSettlementEntryV2;
}
export function resolveRuntimeDiceSettlement(input: {
  readonly preset: DiceRuntimePreset;
  readonly expectedPresetId: string;
  readonly authoritativeResult: unknown;
}): DiceSettlementResolution | undefined;

export interface DiceMotionMemberDescriptor {
  readonly memberIndex: number; // Stone 1: exactly 0
  readonly memberCount: number; // Stone 1: exactly 1
}
export type DiceTranslation = readonly [number, number, number];
export interface DiceShadowPose {
  readonly translation: DiceTranslation;
  readonly scale: number;
  readonly opacity: number;
}
export interface DiceMotionPose {
  readonly quaternion: QuaternionTuple;
  readonly translation: DiceTranslation;
  readonly shadow: DiceShadowPose;
  readonly observeNow: boolean;
  readonly exactTargetHeld: boolean;
  readonly failed: boolean;
}
export interface DiceMotionSolverInput {
  readonly phase: DiceTrayPhase;
  readonly elapsedMs: number;
  readonly reducedMotion: boolean;
  readonly target: QuaternionTuple;
  readonly throwProfile: VisualThrowProfileV1;
  readonly member: DiceMotionMemberDescriptor;
  readonly held?: HeldRollGroupState;
}
export interface DiceMotionSolver {
  readonly revision: 'choreographed-v1';
  solve(input: DiceMotionSolverInput): DiceMotionPose;
}
export const ChoreographedSolverV1: DiceMotionSolver;
export function angularDistanceDegrees(
  first: QuaternionTuple,
  second: QuaternionTuple
): number;
```

- Consumers: Task 4 `AttackDie3D`; Task 5 evidence telemetry.

- [ ] **Step 1: Write settlement-resolver and pure solver tests**

For `resolveRuntimeDiceSettlement`, require a resolution only when the ready provider preset ID exactly matches `expectedPresetId`, the authoritative result is an integer in the preset-supported domain, the matching entry exists, and its quaternion is finite/unit. The returned target tuple/resolution must be a fresh frozen snapshot; wrong preset, missing/extra result, malformed quaternion, and mismatched supported-results/entry facts return `undefined`. The resolver must never accept an expected model path/hash/quaternion argument or contain an Original-d20 exception.

For `ChoreographedSolverV1`, require:

- Stone 1 accepts only `{ memberIndex: 0, memberCount: 1 }`; invalid descriptors fail without NaN and future multi-member scatter remains Stone 2;
- same input at elapsed `0, 120, 600, 1199, 1200, 1500, 1899, 1900, 2200` returns deep-equal poses across repeated calls;
- no output depends on frame count, previous pose, `Date.now`, randomness, or browser state;
- two profiles alter mid-flight translation/quaternion/shadow but have exactly the same target at 1900 ms;
- rolling starts from the profile release position with the constant lift and converges continuously to `[-0.23, 0, 0]`;
- the tumble pose at 1200 ms is exactly the start pose for 1200–1900 ms spherical convergence;
- at `elapsedMs >= 1900`, quaternion is the exact target tuple, `observeNow=true`, `exactTargetHeld=true`, `failed=false`;
- settled/exiting always hold the exact target and resting translation;
- armed/ready with held state follows normalized tray-plane position, lift, bounded tilt/wobble, and shadow; ready without held state is neutral;
- reduced-motion held state is one centered static lift regardless of samples; reduced rolling reaches exact target on the first positive elapsed frame;
- malformed non-finite inputs return `failed=true` and never leak NaN into a pose.

- [ ] **Step 2: Run tests and verify RED**

```bash
npm run test:run -- \
  src/components/ui/dice/diceSettlementResolver.test.ts \
  src/components/ui/dice/choreographedDiceMotion.test.ts
```

Expected: FAIL because resolver/solver modules do not exist.

- [ ] **Step 3: Implement provider-backed settlement resolution and deterministic pose formulas**

`resolveRuntimeDiceSettlement` must consume the already strict/frozen provider preset, compare request preset/result facts, verify the selected entry/quaternion, and return an immutable copy. It may not accept or derive a web-local model hash, model path, result permutation, label table, or target quaternion.

Use solver constants:

```ts
export const ROLL_DURATION_MS = 1900;
export const CONVERGENCE_START_MS = 1200;
export const HOLD_LIFT = 0.16;
export const RESTING_TRANSLATION = [-0.23, 0, 0] as const;
export const NEUTRAL_QUATERNION = [0.31, -0.47, 0.19, 0.805] as const;
```

For rolling translation, use only elapsed time/profile:

```ts
const progress = clamp(elapsedMs / ROLL_DURATION_MS, 0, 1);
const eased = 1 - Math.pow(1 - progress, 3);
const envelope = Math.sin(Math.PI * progress);
const releaseX = (profile.releasePosition[0] - 0.5) * 0.8;
const releaseZ = (0.5 - profile.releasePosition[1]) * 0.55;
const arc = envelope *
  (0.08 + profile.releaseSpeed * 0.18 + profile.shakeEnergy * 0.07);
const bounce = Math.abs(
  Math.sin(progress * Math.PI * (2 + profile.shakeEnergy * 3) + seedPhase)
) * profile.shakeEnergy * 0.035 * envelope;
const translation = [
  lerp(releaseX, -0.23, eased) +
    profile.releaseDirection[0] * profile.releaseSpeed * 0.16 * envelope,
  HOLD_LIFT * (1 - progress) + arc + bounce,
  lerp(releaseZ, 0, eased) -
    profile.releaseDirection[1] * profile.releaseSpeed * 0.14 * envelope,
] as const;
```

Derive `seedPhase` from the uint32 seed, speed, energy, and bias. Before 1200 ms, compute a normalized deterministic tumble quaternion from bounded sine/cosine terms. At/after 1200 ms, slerp from the exact `tumbleQuaternionAt(1200, profile)` to target using `(elapsedMs - 1200) / 700`; return the target tuple verbatim at/after 1900 ms. Reject every member descriptor except the Stone 1 one-member value before calculating a pose; do not implement scatter offsets in this task.

Held translation maps normalized X/Y into the same X/Z tray plane, with Y=`HOLD_LIFT`; held quaternion composes bounded X/Z tilt and a wobble term from `wobblePhase * 2π * shakeEnergy`. Shadow sits on the tray plane under current X/Z, scales from `0.82..1.12`, and fades from `0.34..0.14` as lift grows. Do not animate DOM/Canvas transforms.

- [ ] **Step 4: Run tests, lint the new files, and typecheck**

```bash
npm run test:run -- \
  src/components/ui/dice/diceSettlementResolver.test.ts \
  src/components/ui/dice/choreographedDiceMotion.test.ts
npm run lint -- \
  src/components/ui/dice/diceSettlementResolver.ts \
  src/components/ui/dice/diceSettlementResolver.test.ts \
  src/components/ui/dice/diceMotionSolver.ts \
  src/components/ui/dice/choreographedDiceMotion.ts \
  src/components/ui/dice/choreographedDiceMotion.test.ts
npm run typecheck
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/components/ui/dice/diceSettlementResolver.ts \
  src/components/ui/dice/diceSettlementResolver.test.ts \
  src/components/ui/dice/diceMotionSolver.ts \
  src/components/ui/dice/choreographedDiceMotion.ts \
  src/components/ui/dice/choreographedDiceMotion.test.ts
git commit -m "feat: add deterministic dice choreography solver (#755)"
```

---

### Task 4: Integrate held motion and profile-bearing release events

**Files:**
- Modify: `src/components/ui/dice/dicePresentationRelease.ts`
- Modify: `src/components/ui/dice/dicePresentationRelease.test.ts`
- Modify: `src/components/ui/dice/dicePresentationEvent.ts`
- Modify: `src/components/ui/dice/dicePresentationEvent.test.ts`
- Modify: `src/components/ui/dice/DiceTray3D.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.test.tsx`
- Modify: `src/components/ui/dice/DiceTrayPresentation.tsx`
- Modify: `src/components/ui/dice/DiceTrayPresentation.test.tsx`
- Modify: `src/components/ui/dice/AttackDie3D.tsx`
- Modify: `src/components/ui/dice/AttackDie3D.test.tsx`
- Modify: `public/themes/base.css`
- Delete after migration: `src/components/ui/dice/attackDieMotion.ts`
- Delete after migration: `src/components/ui/dice/attackDieMotion.test.ts`

**Interfaces:**
- Consumes: Tasks 1–3.
- Produces:

```ts
export interface DicePresentationRelease {
  readonly schemaVersion: 2;
  readonly presentationId: string;
  readonly presetId: string;
  readonly throwProfile: VisualThrowProfileV1;
}

export function createDicePresentationRelease(input: {
  presentationId: string;
  presetId: string;
  throwProfile: VisualThrowProfileV1;
}): DicePresentationRelease;
```

`DicePresentationReleasedEvent` remains event schema version 1; only its nested release advances to schema version 2.

`DiceTray3DProps.onReleaseRequest` becomes:

```ts
onReleaseRequest?: (throwProfile?: VisualThrowProfileV1) => void;
```

`AttackDie3DProps` replaces `decorativeRelease` with:

```ts
throwProfile?: VisualThrowProfileV1;
heldRollGroup?: HeldRollGroupState;
```

- [ ] **Step 1: Rewrite release/event tests first**

Require exact release keys `schemaVersion`, `presentationId`, `presetId`, `throwProfile`; release schema must equal 2 and nested profile schema 1. Reject the former `variation/vector/shake` shape, mixed shapes, extra keys, mismatched presentation/preset IDs, malformed profile tuples, and denied raw gesture fields at any depth.

Projection tests must continue proving first valid request/release wins, conflicting duplicates fail closed, delivery arrays remain immutable facts, and `presentationId` is the only release correlation key.

- [ ] **Step 2: Write integration tests before changing components**

In `DiceTray3D.test.tsx` and `AttackDie3D.test.tsx`, add failing tests that prove:

- the grab target is die-sized rather than renderer-sized and pointer-down outside the controller hit bounds does not grab;
- accepted pointer-down obtains capture and changes only Roller local held state;
- move updates `heldRollGroup` passed to the actual `AttackDie3D`, while no release event exists;
- release sends one valid frozen profile, clears held state, and duplicate pointer-up/click does nothing;
- cancel, lost capture, identity/phase change, provider failure, and unmount clear held state/capture;
- keyboard activation calls `onReleaseRequest(undefined)` so the authority boundary creates a neutral profile;
- the actual Three.js die group receives solver quaternion/position and an independently owned shadow mesh; Canvas/renderer style has no transform;
- final observation still derives from the rendered group world quaternion and retains all Stone 0 thresholds.

In `DiceTrayPresentation.test.tsx`, add a test asserting:

```ts
const event = onReleaseRequest.mock.calls[0][0];
expect(event.release.schemaVersion).toBe(2);
expect(event.release.throwProfile).toEqual(profileFromController);
expect(event.presentationId).toBe(request.presentationId);
expect(event.release.presentationId).toBe(request.presentationId);
expect(event.release.throwProfile).not.toHaveProperty('result');
```

- [ ] **Step 3: Run the focused suite and verify RED**

```bash
npm run test:run -- \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/dicePresentationEvent.test.ts \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx \
  src/components/ui/dice/AttackDie3D.test.tsx
```

Expected: FAIL on the old release shape and missing held/profile props.

- [ ] **Step 4: Advance the nested release contract and preserve authority reconciliation**

Change `createDicePresentationRelease` to snapshot/freeze the parsed profile instead of deriving presentation facts from raw gesture data. In `DiceTrayPresentation`, retain `presentationHash` and build exactly one release:

```ts
const throwProfile = requestedProfile ??
  createNeutralVisualThrowProfile(presentationHash(presentationId));
const next = Object.freeze({
  schemaVersion: 1 as const,
  type: 'dice-presentation-released' as const,
  eventId: releaseEventId(presentationId),
  presentationId,
  release: createDicePresentationRelease({
    presentationId,
    presetId,
    throwProfile,
  }),
});
```

Do not alter lifecycle projection, renderer generation, result, provider, fallback, or settlement acceptance logic.

- [ ] **Step 5: Replace inline gesture bookkeeping with the controller**

`DiceTray3D` must create one controller per mounted presentation, snapshot `getBoundingClientRect()` values into plain finite bounds on pointer-down, choose padding `24` for `pointerType === 'touch'` and `14` otherwise, and pass `presentationHash(presentationId)` as motion seed from `DiceTrayPresentation` through a new `motionSeed` prop.

On accepted begin/move, set local `heldRollGroup`. On release, clear local state before calling `onReleaseRequest(profile)`. On cancel/lost capture/identity change/phase change/failure/unmount, call controller cancellation/reset and clear local state without emitting a release.

Keep pointer/held state out of event arrays and boundary diagnostics. Do not log raw events.

- [ ] **Step 6: Replace renderer motion with the solver and owned shadow**

In `AttackDie3D`, resolve runtime targets only through `resolveRuntimeDiceSettlement` using the ready provider preset, requested preset ID, and authoritative result; keep the Lightning calibration path explicitly development-only. Create a neutral profile only for standalone/development calls lacking a release profile. `RuntimeDie.useFrame` calls `ChoreographedSolverV1.solve` using current phase, elapsed time, reduced-motion preference, resolved authoritative target, profile, `{ memberIndex: 0, memberCount: 1 }`, and held state. Remove previous-frame quaternion from solver input.

Apply returned pose only to Three.js objects:

```ts
selectedGroup.quaternion.set(...pose.quaternion);
selectedGroup.position.set(...pose.translation);
shadowGroup.position.set(...pose.shadow.translation);
shadowGroup.scale.setScalar(pose.shadow.scale);
shadowMaterial.opacity = pose.shadow.opacity;
```

Render the shadow as a sibling owned mesh on the tray plane with a circle geometry and transparent material, and dispose it through the existing Canvas/resource lifecycle. Final observation continues to call `selectedGroup.getWorldQuaternion(...)` after applying the exact target.

Delete `attackDieMotion.ts` and its old test only after `rg 'attackDieMotion|AttackDieDecorativeRelease|DiceGestureSample' src` returns no production matches.

- [ ] **Step 7: Replace the whole-tray visual target**

Use a centered die-sized target with forgiving padding and no grabbed border:

```css
.dice-tray-3d-renderer > .dice-tray-3d-grab-target {
  position: absolute;
  top: 50%;
  left: 50%;
  z-index: 2;
  width: min(58%, 156px);
  aspect-ratio: 1;
  margin: 0;
  padding: 0;
  border: 0;
  border-radius: 50%;
  background: transparent;
  box-shadow: none;
  transform: translate(-50%, -50%);
  touch-action: none;
  cursor: grab;
}
.dice-tray-3d-renderer > .dice-tray-3d-grab-target[data-grabbed='true'] {
  box-shadow: none;
  cursor: grabbing;
}
.dice-tray-3d-renderer > .dice-tray-3d-grab-target:focus-visible {
  outline: 3px solid var(--accent-primary);
  outline-offset: 6px;
}
```

The target transform positions only the invisible hit area; it must never animate and must not move the Canvas or die. Add a CSS guard test that rejects `transform`, `translate`, or animation on `.attack-die-3d__canvas`, `.attack-die-3d`, and `.dice-tray-3d-renderer`.

- [ ] **Step 8: Run focused tests, formatting, lint, and typecheck**

```bash
npm run test:run -- \
  src/components/ui/dice/visualThrowProfile.test.ts \
  src/components/ui/dice/rollGroupGestureController.test.ts \
  src/components/ui/dice/diceSettlementResolver.test.ts \
  src/components/ui/dice/choreographedDiceMotion.test.ts \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/dicePresentationEvent.test.ts \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx \
  src/components/ui/dice/AttackDie3D.test.tsx
npm run format:check
npm run lint
npm run typecheck
```

Expected: PASS.

- [ ] **Step 9: Commit**

```bash
git add src/components/ui/dice public/themes/base.css
git commit -m "feat: make the d20 roll group tactile (#755)"
```

---

### Task 5: Prove paired replay, reduced motion, and lifecycle hardening

**Files:**
- Modify: `src/concepts/attack-die-3d/diceTrayWitnessFixture.ts`
- Modify: `src/concepts/attack-die-3d/diceTrayWitnessFixture.test.ts`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.tsx`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx`
- Modify: `src/components/ui/dice/AttackDie3D.tsx`
- Modify: `src/components/ui/dice/AttackDie3D.test.tsx`
- Modify: `src/components/ui/dice/DiceTrayPresentation.tsx`
- Modify: `src/components/ui/dice/DiceTrayPresentation.test.tsx`

**Interfaces:**
- Consumes: integrated controller/profile/solver from Task 4.
- Produces safe browser evidence facts:

```ts
interface Stone1WitnessMotionFact {
  readonly motionRevision: 'choreographed-v1';
  readonly throwProfile?: VisualThrowProfileV1;
  readonly requestedResult: number;
  readonly observedUpwardResult?: number;
  readonly exactTargetHeld: boolean;
  readonly contextId?: number;
  readonly cloneId?: number;
}
```

No held/raw pointer state is published through the shared event or telemetry boundary.

- [ ] **Step 1: Update fixture tests before fixture code**

Require Monster auto-release at exactly 250 ms to carry a host-created neutral profile seeded from its presentation identity. Player release remains Roller-requested. Recursively inspect every fixture event and reject raw-coordinate/history/time/result-target additions inside `throwProfile`.

- [ ] **Step 2: Add paired behavior tests**

Extend the existing paired concept test to prove this order:

1. request event array/provider are shared;
2. Roller pointer-down/move produces visible local held state while Spectator remains armed/unmoved;
3. no event, profile, phase change, or Spectator telemetry occurs before release;
4. pointer-up appends exactly one release event;
5. Roller and Spectator receive profiles that are deep-equal, strictly parsed, and deeply frozen;
6. their solver outputs at fixed elapsed samples are deep-equal while runtime contexts/clones remain distinct;
7. both independently observe the unchanged authoritative result with Stone 0 thresholds;
8. duplicate release, remount, StrictMode effects, late old-generation callbacks, and delivery discontinuity cannot append/replay a second release.

Add separate tests for:

- quick down/up with no move;
- repeated shake and outside-capture release;
- pointer cancel and lost capture (no release);
- reduced motion static held cue and exact settlement;
- renderer/provider failure during hold clears local state and later reveals only truthful SVG after valid release semantics;
- keyboard Roll emits a neutral profile and visible focus remains on the explicit control;
- Monster never constructs a local gesture controller or emits duplicate autoplay from Roller/Spectator components.

- [ ] **Step 3: Run tests and verify RED**

```bash
npm run test:run -- \
  src/concepts/attack-die-3d/diceTrayWitnessFixture.test.ts \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx
```

Expected: FAIL until fixture profiles, safe bridge facts, and lifecycle fences are integrated.

- [ ] **Step 4: Produce host neutral profiles and safe final telemetry**

Change the fixture host to call `createNeutralVisualThrowProfile(presentationHash(...))` and wrap it with release schema 2. At final 3D observation, include only `motionRevision` and the deeply frozen `throwProfile` in telemetry; do not publish held samples.

Fence callbacks by presentation token/generation and callback identity before updating the Concepts evidence bridge. Preserve the current disposed-witness protections.

- [ ] **Step 5: Add the Stone 1 browser bridge**

Expose `window.__stone1TrayEvidence` only from the Concepts Lab with:

- current request identity/result/preset;
- shared event-array/provider IDs;
- Roller/Spectator renderer context, runtime source/clone, final telemetry, and release profile;
- local booleans `rollerGrabbed` and `spectatorGrabbed` read from current UI state;
- release count and lifecycle phase.

Do not include pointer coordinates, pointer ID/type, sample timestamps, path length/history, DOMRect values, or pre-release velocity/tilt.

- [ ] **Step 6: Run focused and full component suites**

```bash
npm run test:run -- \
  src/components/ui/dice \
  src/concepts/attack-die-3d
npm run format:check
npm run lint
npm run typecheck
```

Expected: PASS.

- [ ] **Step 7: Commit**

```bash
git add src/components/ui/dice \
  src/concepts/attack-die-3d
git commit -m "test: prove shared tactile dice choreography (#755)"
```

---

### Task 6: Capture exact-SHA Stone 1 evidence and complete the gate

**Files:**
- Create: `scripts/attack-die/pngEvidenceValidation.ts`
- Create: `scripts/attack-die/pngEvidenceValidation.test.ts`
- Create: `scripts/attack-die/stone1TrayEvidenceProtocol.ts`
- Create: `scripts/attack-die/stone1TrayEvidenceProtocol.test.ts`
- Create: `scripts/attack-die/capture-stone1-tray-evidence.mjs`
- Modify: `scripts/attack-die/stone0TrayEvidenceProtocol.ts`
- Modify: `scripts/attack-die/stone0TrayEvidenceProtocol.test.ts`
- Modify: `package.json`
- Modify: `docs/how-to/attack-die-3d-concept.md`

**Interfaces:**
- Consumes: `window.__stone1TrayEvidence`, frozen-build manifest, corrected runtime provider, and the accepted Stone 0 PNG/package checks.
- Produces an ignored package under `.verification/interactive-dice-tray/stone-1/<exact-web-sha>/` with strict manifest, browser facts, network/console logs, screenshots, hashes, and failure marker semantics.

- [ ] **Step 1: Extract generic sequential PNG inspection with unchanged Stone 0 behavior**

Move the generic PNG signature/chunk framing/CRC/IHDR/IDAT/IEND/inflate/filter logic out of `stone0TrayEvidenceProtocol.ts` into `pngEvidenceValidation.ts`. The API must preflight aggregate decoded bytes, inspect each PNG exactly once sequentially, retain only dimensions/scalars, and reject truncated data, invalid CRC, duplicate/missing critical chunks, trailing bytes, inflate-size mismatch, unsupported filter, or declared resource excess.

Run the existing Stone 0 mutation suite before and after extraction and require identical outcomes.

- [ ] **Step 2: Write the Stone 1 evidence protocol tests**

Use these exact scenario IDs:

```ts
export const STONE1_SCENARIO_IDS = [
  'held-desktop',
  'held-outside-capture',
  'quick-release',
  'repeated-shake',
  'keyboard-neutral',
  'paired-shared-release',
  'reduced-motion-held',
  'responsive-narrow',
  'pointer-cancel',
  'lost-pointer-capture',
  'provider-failure',
  'context-loss',
] as const;
```

The validator must reject packages unless all of these are true:

- source SHA is the exact captured git head and equals the frozen build manifest source SHA;
- build hash, build-manifest hash, provider manifest/source hashes, GLB size/hash, preset ID, and 2,684/7,798 roles match captured bytes;
- authoritative result remains 10 for all success scenarios;
- before release only Roller is grabbed and no release/profile exists;
- after release exactly one event carries release schema 2/profile schema 1;
- Roller/Spectator profiles are deep-equal and contain only allowed keys/bounds;
- profile/object tuples were observed frozen in the live page;
- contexts and clones are distinct, source/provider/events are shared, and final world-pose observations independently match result 10 with all Stone 0 thresholds;
- reduced motion has a static lifted held cue and no tumble/shake/bounce samples;
- cancel/lost-capture emit no release and clear grabbed state;
- provider/context failures converge to SVG without stale held/profile telemetry;
- every required screenshot has the declared viewport/device scale, full PNG validation, readable nontransparent content, and SHA-256 binding;
- validation uses aggregate-byte preflight and sequential decode with peak RSS below 512 MiB;
- rejected captures contain `FAILED.txt`; recaptured superseded packages retain `INVALIDATED-PASS.txt` and cannot satisfy acceptance.

Mutation tests must independently flip every boolean/threshold/profile field, add every denied key, equate context/clone IDs, alter result/target observation, corrupt each PNG stage, exceed resource bounds, and mismatch source/build/provider hashes.

- [ ] **Step 3: Run protocol tests and verify RED**

```bash
npm run test:run -- \
  scripts/attack-die/pngEvidenceValidation.test.ts \
  scripts/attack-die/stone0TrayEvidenceProtocol.test.ts \
  scripts/attack-die/stone1TrayEvidenceProtocol.test.ts
```

Expected: FAIL until extraction and Stone 1 validator exist.

- [ ] **Step 4: Implement the exact-SHA browser capture**

`capture-stone1-tray-evidence.mjs` must:

1. require a clean tracked tree and record exact `git rev-parse HEAD`;
2. sync the corrected private provider through repository-local same-filesystem atomic temporary paths;
3. freeze/hash the build and verify source/build/provider bindings before launch;
4. use a dedicated port and register signal/exception cleanup before spawning the server;
5. capture the twelve scenarios at desktop 1440×1080 and narrow 760×900 where applicable;
6. drive real pointer capture, outside release, quick release, repeated shake, keyboard activation, cancel, and lost-capture paths;
7. sample the live bridge before release, during held state, after shared release, and after both independent final observations;
8. record network requests/transfers, browser console/page errors, Canvas count, context/source/clone/provider/event IDs, profile freeze/key facts, final settlement facts, and screenshot hashes;
9. close pages, contexts, browser, server, and port on success/failure/signal;
10. validate the completed package before printing PASS.

Add:

```json
"attack-die:stone1-evidence": "node scripts/attack-die/capture-stone1-tray-evidence.mjs"
```

- [ ] **Step 5: Run protocol, focused, full, and build gates**

```bash
npm run test:run -- \
  scripts/attack-die/pngEvidenceValidation.test.ts \
  scripts/attack-die/stone0TrayEvidenceProtocol.test.ts \
  scripts/attack-die/stone1TrayEvidenceProtocol.test.ts
npm run test:run
npm run format:check
npm run lint
npm run typecheck
npm run build
bash scripts/ci-check.sh
```

Expected: all commands PASS. Record the exact file/test totals rather than copying Stone 0 totals.

- [ ] **Step 6: Capture and independently inspect evidence**

```bash
npm run attack-die:stone1-evidence
```

Expected: one accepted package under `.verification/interactive-dice-tray/stone-1/<HEAD>/`, twelve passing scenarios, no unexpected browser/network errors, valid package manifest, and peak validation RSS below 512 MiB.

Create readable contact sheets for held/release/settled Roller and Spectator views. Obtain independent human review of tactile clarity, die-sized hit cue, outside-capture behavior, paired personality, narrow layout, and reduced motion. A machine-valid package without readable human approval is not acceptance evidence.

- [ ] **Step 7: Document behavior and explicit limitations**

Update `docs/how-to/attack-die-3d-concept.md` with profile fields/bounds, controller/solver ownership, neutral/Monster paths, exact evidence command, fallback semantics, and the following ungraduated areas: production transport/reconnect, equipped preset projection, physical touch hardware, real Discord Activity, low-GPU/mobile coverage, multi-die groups, rigid-body settlement, and formal paired performance while the final-context flaw remains.

- [ ] **Step 8: Verify no private/generated artifacts are tracked**

```bash
git status --short
if git ls-files public/models .verification | grep -q .; then
  echo 'private/generated evidence is tracked' >&2
  exit 1
fi
git diff --check
if git grep -n 'library/custom-dice' -- src; then
  echo 'production source reads authoring authority' >&2
  exit 1
fi
if git diff 8bc2a273dba4d018f9d9c6829ca043b7cdafa60e...HEAD -- src public/themes \
  | grep -E '75c56871da792262|de2377e2d0f8b1bc'; then
  echo 'rejected commit identifiers entered implementation' >&2
  exit 1
fi
```

Expected: only intended source/docs are tracked; no private providers/evidence; no production read from authoring authority; no rejected implementation identifiers. Independent review still compares behavior against the prohibited commits rather than treating this identifier check as proof of originality.

- [ ] **Step 9: Commit evidence tooling/docs**

```bash
git add scripts/attack-die package.json docs/how-to/attack-die-3d-concept.md
git commit -m "test: gate Stone 1 tactile dice evidence (#755)"
```

- [ ] **Step 10: Run independent review and final exact-head gate**

Dispatch a fresh spec reviewer over `8bc2a273..HEAD`, then a fresh code/evidence reviewer. Address every Blocker/High/Medium finding with a new focused commit and rerun affected gates. Final merge-ready criteria are:

- 0 Blocker / 0 High / 0 Medium review findings;
- exact-head focused/full suites, formatting, lint, typecheck, build, private-asset guards, and normal pre-push hook pass;
- exact-head evidence package accepted and human-reviewed;
- GitHub checks green;
- residual hardware/transport/performance limitations explicitly recorded.

Push branch `feature/755-stone1-tactile-roll-groups`, open/update a PR against `dev`, post a signed gate comment, and move web #755 to **In Review**. **Do not merge; stop for Kirk.**

## Execution record — 2026-08-15

All six tasks completed through web PR [#756](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/756). Final exact reviewed head: `fe19dc7fff00890d6e7fec18ad8a03b982ee6b28`.

- Final review: 0 Blocker / 0 High / 0 Medium; one nonblocking Low static assertion weakness remains covered by behavioral terminal-publication tests.
- Full suite: 192 files / 3,394 tests; format, lint, typecheck, build, CI, pre-push, private guards, and all GitHub checks passed.
- Exact evidence: 12/12 scenarios, 18 protocol PNGs, 12 contexts, PASS-only marker; package-manifest SHA-256 `947ee1c884d698c295588f01deb9a5fd9d19fee29bcf8b4066c51e35f2711e54`.
- Final privacy review removed reconstructible held-pose history from the browser bridge; only generation-fenced monotonic booleans and sanitized final facts leave the renderer.
- Kirk inspected and approved the live exact-head interaction. Web #755 and Project 19 are In Review. PR #756 remains unmerged for Kirk's decision.
