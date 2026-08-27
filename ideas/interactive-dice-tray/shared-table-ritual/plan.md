# Shared Table Dice Feel Lab Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver web issue #826 as a fixture-only Concepts Lab stage where Kirk can compare broad single- and multi-die throw feels with exact held attachment, mixed contributor styles, automatic rerolls, and independent roller/witness presentations.

**Architecture:** Keep the exact-key production `DicePresentationEvent` contract, session adapters, and current one-d20 behavior unchanged, while extracting shared interaction and runtime-mesh internals beneath the existing `DiceTrayPresentation → DiceTray3D → AttackDie3D` chain. The d20 compatibility adapters use those internals in legacy-normalized mode; the group adapter uses the same interaction surface/controller/runtime mesh with actual camera-plane projection and group motion, so Concepts Lab does not become a second renderer. A later production promotion may switch the compatibility adapter only after Kirk approves the concept feel.

**Tech Stack:** React 19, TypeScript, Three.js 0.181, React Three Fiber 9, Vitest 4, React Testing Library, existing GLTF/runtime dice provider, CSS in `public/themes/base.css`.

**Spec:** `ideas/interactive-dice-tray/shared-table-ritual/design.md`

**Tracking:** `KirkDiggler/rpg-dnd5e-web#826`, parent journey `KirkDiggler/rpg-project#289`, design PR `KirkDiggler/rpg-project#291`.

## Global Constraints

- Work only in `rpg-dnd5e-web`; no proto, toolkit, API, deployment, or production session-route changes.
- Branch from latest `origin/dev`, not local `dev`; use one branch `concept/826-shared-table-dice` and one PR targeting `dev`.
- Do not modify `src/components/session/**`, `src/api/**`, or production session-combat adapters.
- Preserve existing `DicePresentationEvent`, `DicePresentationRelease`, `DiceTrayPresentationProps`, `DiceTray3DProps`, and current one-d20 behavior. The d20 path must use extracted shared renderer/interaction internals in a compatibility mode; exact camera-plane attachment remains concept-only until production promotion.
- Do not add group fields to `dicePresentationEvent.ts` or candidate/set/result facts to `VisualThrowProfileV1`.
- Fixture faces, rerolls, dispositions, modifiers, totals, and contributors are supplied facts; the client never calculates or infers them.
- Raw pointer coordinates, pointer IDs, paths, and sample timing never enter serialized events, profiles, diagnostics, or evidence.
- Held grabbed-point error is at most 2 CSS pixels after the next rendered frame in browser evidence.
- Non-d20 Original assets are explicitly `provisional-concept`; only the existing carved d20 path may claim verified physical face correctness.
- Do not add a rigid-body dependency. The Physical candidate stays deterministic behind the new group solver.
- Local licensed assets may be synced for review but remain ignored/uncommitted. No private screenshots or licensed bytes enter the public repo.
- TDD is mandatory: every task begins with focused RED, records the expected failure, implements the minimum GREEN, runs focused regression, and commits.
- Run `npm run ci-check` before every push; never use `--no-verify`.

## Execution preflight

Run before Task 1:

```bash
cd /home/kirk/game-dev/rpg-dnd5e-web
git fetch origin
git worktree add /home/kirk/.pi/worktrees/rpg-dnd5e-web/826-shared-table-dice \
  -b concept/826-shared-table-dice origin/dev
cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/826-shared-table-dice
npm install
RPG_GAME_ASSETS_PATH=/home/kirk/game-dev/rpg-game-assets \
  ASSETS_SYNC_SKIP_UPDATE=1 npm run assets:sync
npm run test:run -- \
  src/components/ui/dice/DiceTrayPresentation.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/rollGroupGestureController.test.ts \
  src/components/ui/dice/choreographedDiceMotion.test.ts \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx
```

Expected baseline: all listed files pass on `origin/dev`. The asset sync changes only ignored `public/models/**` files.

## File structure

### Shared group contract and state

- Create `src/components/ui/dice/diceRollGroup.ts` — exact structural types, parser, face/reroll validation, deep immutable snapshots.
- Create `src/components/ui/dice/diceRollGroup.test.ts` — malformed input and no-client-calculation tests.
- Create `src/components/ui/dice/diceRollGroupEvent.ts` — component-local append-only request/release events and first-valid projection.
- Create `src/components/ui/dice/diceRollGroupEvent.test.ts` — duplicate/conflict/hydration/hostile-input tests.
- Create `src/components/ui/dice/rollGroupPresentationState.ts` — one-group phase reducer for originals, rerolls, modifiers, and completion.
- Create `src/components/ui/dice/rollGroupPresentationState.test.ts` — ordered/idempotent state transition tests.

### Interaction and motion

- Create `src/components/ui/dice/trayPlaneProjection.ts` — actual-camera CSS-screen ↔ tray-plane projection.
- Create `src/components/ui/dice/trayPlaneProjection.test.ts` — deterministic perspective/top-camera round-trip tests.
- Create `src/components/ui/dice/anchoredRollGroupGestureController.ts` — shared one-/multi-member hit testing, grab-offset retention, pointer capture, held state, sanitized release.
- Create `src/components/ui/dice/anchoredRollGroupGestureController.test.ts` — exact attachment, overlap precedence, cleanup, and denied-field tests.
- Create `src/components/ui/dice/TrayPlaneProjectionBridge.tsx` — R3F camera/viewport bridge used by the exact group mode.
- Create `src/components/ui/dice/DiceTrayInteractionSurface.tsx` — one pointer/capture surface used by `DiceTray3D` and `RollGroupTray3D`, with explicit `legacy-normalized` and `tray-plane` projection modes.
- Modify `src/components/ui/dice/DiceTray3D.tsx` and `DiceTray3D.test.tsx` — retain public props and exact existing motion output while using the shared interaction surface in `legacy-normalized` mode.
- Create `src/components/ui/dice/rollGroupLayout.ts` — compact held/resting layouts and projected member hit regions.
- Create `src/components/ui/dice/rollGroupLayout.test.ts` — containment/non-overlap/stable-order tests.
- Create `src/components/ui/dice/rollGroupMotionSolver.ts` — Weighty/Energetic/Physical profiles and deterministic per-member/reroll poses.
- Create `src/components/ui/dice/rollGroupMotionSolver.test.ts` — candidate separation, determinism, settlement, and isolated-reroll tests.

### Provisional multi-shape renderer

- Create `src/components/ui/dice/conceptDiceRuntimeProvider.ts` and `conceptDiceRuntimeProvider.test.ts` — separate provisional-only manifest/model/hash/binding loader, request ownership, and cache for Original carved d4/d6/d8/d10/d12.
- Leave `src/components/ui/dice/diceRuntimeProvider.ts` unchanged; d20 group members call its existing verified API.
- Modify `src/components/ui/dice/diceSettlementObservation.ts` — add generic preset-result observation while retaining `observeUpwardResult` d20 behavior.
- Modify `src/components/ui/dice/diceSettlementObservation.test.ts` — d4/d6 structural observations plus unchanged d20 gates.
- Create `src/components/ui/dice/RuntimeDiceMesh.tsx` and `RuntimeDiceMesh.test.tsx` — generic runtime scene/material/pose child extracted from `AttackDie3D` and used by both paths.
- Modify `src/components/ui/dice/AttackDie3D.tsx` and `AttackDie3D.test.tsx` — preserve the d20 adapter/telemetry while delegating runtime mesh work to `RuntimeDiceMesh`.
- Create `src/components/ui/dice/RollGroupDie3D.tsx` — thin R3F member adapter over `RuntimeDiceMesh` for provisional/verified group members.
- Create `src/components/ui/dice/RollGroupDie3D.test.tsx` — loading/failure/provisional labels/independent clone and shared-runtime behavior.
- Create `src/components/ui/dice/SemanticRollGroup.tsx` — truthful kind/face fallback for every supported die.
- Create `src/components/ui/dice/SemanticRollGroup.test.tsx` — concealed/released/reroll/final semantics.

### Shared group presentation

- Create `src/components/ui/dice/RollGroupTray3D.tsx` — one Canvas, member dice, pointer bridge, exact projection, group controls, and fallback.
- Create `src/components/ui/dice/RollGroupTray3D.test.tsx` — roller/spectator controls, whole-group grab, candidate solver wiring, reduced motion, failure.
- Create `src/components/ui/dice/RollGroupPresentation.tsx` — internal append-only group lifecycle, release requests, automatic reroll/modifier phases, completion.
- Create `src/components/ui/dice/RollGroupPresentation.test.tsx` — independent witnesses, duplicate/missing release, hydration, rerolls, modifiers, announcements.
- Modify `src/components/ui/dice/DiceTrayPresentation.tsx` and `DiceTrayPresentation.test.tsx` — preserve `DiceTrayPresentationProps` and legacy behavior; add a discriminated `mode: 'roll-group'` overload that delegates to the internal group presentation so Concepts Lab mounts the literal production boundary.

### Concept-only fixtures and UI

- Create `src/concepts/attack-die-3d/sharedTableDiceFixtures.ts` — strict scenario/set records and required fixtures.
- Create `src/concepts/attack-die-3d/sharedTableDiceFixtures.test.ts` — all scenario facts/labels/provisional boundaries.
- Create `src/concepts/attack-die-3d/sharedTableDiceState.ts` — attack→verdict→damage→impact coordinator.
- Create `src/concepts/attack-die-3d/sharedTableDiceState.test.ts` — miss/hit/damage exactly-once flow.
- Create `src/concepts/attack-die-3d/sharedTableDiceDelivery.ts` — local append-only roller/witness host and neutral missing-release scheduler.
- Create `src/concepts/attack-die-3d/sharedTableDiceDelivery.test.ts` — first release, duplicate, missing, cleanup, StrictMode tests.
- Create `src/concepts/attack-die-3d/SharedTableDiceStage.tsx` — candidate/scenario controls, replay, two witnesses, status/diagnostics.
- Create `src/concepts/attack-die-3d/SharedTableDiceStage.test.tsx` — all required visible states and accessibility.
- Create `src/concepts/attack-die-3d/CONTRACT.md` — fixture evidence and later production questions, explicitly not a backend request.
- Create `src/concepts/attack-die-3d/sharedTableDiceBoundary.test.ts` — enforce fixture-host versus shared-presentation imports and production isolation.
- Modify `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx` — add a `Single d20 proof` / `Shared table feel` substage without changing Stone 1.
- Modify `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx` — existing Stone 1 assertions plus new substage mount.
- Modify `public/themes/base.css` — scoped `.shared-table-dice-*` layout and responsive styles only.
- Modify `docs/how-to/attack-die-3d-concept.md` — exact URL, scenarios, provisional labels, and review procedure.
- Create `docs/evidence/826-shared-table-dice/README.md` — reproducible commands, measured facts, private-artifact policy, and Kirk feel gate status.

---

### Task 1: Strict roll-group contract

**Files:**
- Create: `src/components/ui/dice/diceRollGroup.ts`
- Create: `src/components/ui/dice/diceRollGroup.test.ts`

**Interfaces:**
- Consumes: `isDicePresentationIdentifier()` and `isDicePresetIdentifier()` from `dicePresentationRelease.ts`.
- Produces:

```ts
export type DiceRollGroupKey = 'attack' | 'damage';
export type DiceKind = 'd4' | 'd6' | 'd8' | 'd10' | 'd12' | 'd20';
export type DiceRollDisposition = 'counted' | 'discarded';
export type DiceRollPurpose =
  | 'base'
  | 'critical'
  | 'feature'
  | 'condition'
  | 'granted';

export interface DiceRollRerollStep {
  readonly before: number;
  readonly after: number;
  readonly reasonRef: string;
  readonly displayLabel: string;
}

export interface DiceRollGroupDie {
  readonly id: string;
  readonly kind: DiceKind;
  readonly presetId: string;
  readonly setId: string;
  readonly originalFace: number;
  readonly finalFace: number;
  readonly rerolls: readonly DiceRollRerollStep[];
  readonly disposition: DiceRollDisposition;
  readonly sourceRef: string;
  readonly sourceLabel: string;
  readonly contributorMemberId: string;
  readonly purpose: DiceRollPurpose;
}

interface DiceRollModifierBase {
  readonly id: string;
  readonly sourceRef: string;
  readonly displayLabel: string;
  readonly sourceMemberId?: string;
  readonly order: number;
}

export type DiceRollModifier = DiceRollModifierBase &
  (
    | { readonly value: number; readonly text?: never }
    | { readonly value?: never; readonly text: string }
  );

export interface DiceRollGroupInput {
  readonly key: DiceRollGroupKey;
  readonly dice: readonly DiceRollGroupDie[];
  readonly modifiers: readonly DiceRollModifier[];
  readonly suppliedFinalTotal?: number;
  readonly verdictLabel?: string;
  readonly impactLabel?: string;
}

export function parseDiceRollGroupInput(
  value: unknown
): DiceRollGroupInput | undefined;
```

- [ ] **Step 1: Write RED parser tests**

Add table tests for exact keys, face ranges by kind, duplicate die IDs, malformed preset/set/member/source IDs, group cardinality, modifier IDs/order/forms, and deep immutability. Attack groups require at least one physical die. Damage groups may have zero physical dice only when they carry at least one valid modifier or a non-empty impact label. Modifiers require exactly one of finite signed `value` or bounded non-empty `text`; `displayLabel` is non-empty; `order` is unique and contiguous from zero. Include this no-calculation discriminator:

```ts
it('accepts a supplied total that is not recomputed from dice or modifiers', () => {
  const parsed = parseDiceRollGroupInput({
    key: 'damage',
    dice: [die({ originalFace: 1, finalFace: 6 })],
    modifiers: [modifier({ value: 3, order: 0 })],
    suppliedFinalTotal: 999,
  });
  expect(parsed?.suppliedFinalTotal).toBe(999);
});
```

Add a generic reroll-chain test: original `1`, steps `1→5`, final `5` passes; `2→5` against original `1` fails. Do not inspect `reasonRef` to implement Great Weapon Fighting.

- [ ] **Step 2: Run RED**

```bash
npm run test:run -- src/components/ui/dice/diceRollGroup.test.ts
```

Expected: FAIL because `diceRollGroup.ts` does not exist.

- [ ] **Step 3: Implement the strict parser**

Use exact own-key snapshots at every object boundary. Validate identifiers as bounded inert strings; reject URLs, slashes, traversal, symbol keys, accessors that throw, non-finite numbers, unsupported enum values, empty attack groups, and empty damage groups with neither modifiers nor impact. Validate the exclusive modifier form and contiguous order without summing it. Validate each reroll chain mechanically:

```ts
let currentFace = die.originalFace;
for (const step of die.rerolls) {
  if (step.before !== currentFace || !faceFits(kind, step.after)) return undefined;
  currentFace = step.after;
}
if (currentFace !== die.finalFace) return undefined;
```

Deep-copy/freeze every tuple, record, and array. Never retain caller-owned data.

- [ ] **Step 4: Run GREEN and existing schema guards**

```bash
npm run test:run -- \
  src/components/ui/dice/diceRollGroup.test.ts \
  src/components/ui/dice/dicePresentationEvent.test.ts \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/visualThrowProfile.test.ts
```

Expected: PASS; existing exact-key production schemas remain unchanged.

- [ ] **Step 5: Commit**

```bash
git add src/components/ui/dice/diceRollGroup.ts \
  src/components/ui/dice/diceRollGroup.test.ts
git commit -m "feat(dice): add strict roll-group component contract (#826)"
```

### Task 2: Append-only group events and one-group lifecycle

**Files:**
- Create: `src/components/ui/dice/diceRollGroupEvent.ts`
- Create: `src/components/ui/dice/diceRollGroupEvent.test.ts`
- Create: `src/components/ui/dice/rollGroupPresentationState.ts`
- Create: `src/components/ui/dice/rollGroupPresentationState.test.ts`

**Interfaces:**
- Consumes: `DiceRollGroupInput`, `DiceRollGroupKey`, `parseDiceRollGroupInput`, `VisualThrowProfileV1`, `parseVisualThrowProfile`.
- Produces:

```ts
export interface DiceRollGroupRequestedEvent {
  readonly schemaVersion: 1;
  readonly type: 'dice-roll-group-requested';
  readonly eventId: string;
  readonly presentationId: string;
  readonly roller: Readonly<{
    memberId: string;
    role: 'player' | 'monster';
  }>;
  readonly group: DiceRollGroupInput;
}

export interface DiceRollGroupRelease {
  readonly schemaVersion: 1;
  readonly presentationId: string;
  readonly groupKey: DiceRollGroupKey;
  readonly throwProfile: VisualThrowProfileV1;
}

export interface DiceRollGroupReleasedEvent {
  readonly schemaVersion: 1;
  readonly type: 'dice-roll-group-released';
  readonly eventId: string;
  readonly presentationId: string;
  readonly release: DiceRollGroupRelease;
}

export type DiceRollGroupEvent =
  | DiceRollGroupRequestedEvent
  | DiceRollGroupReleasedEvent;

export function parseDiceRollGroupEvent(
  value: unknown
): DiceRollGroupEvent | undefined;
export function projectDiceRollGroupEvents(
  values: readonly unknown[]
): Readonly<{
  request?: DiceRollGroupRequestedEvent;
  release?: DiceRollGroupReleasedEvent;
  acceptedEvents: readonly DiceRollGroupEvent[];
}>;
```

```ts
export type RollGroupPresentationPhase =
  | 'armed'
  | 'rolling-originals'
  | 'settled-originals'
  | 'reroll-flash'
  | 'rerolling'
  | 'modifiers'
  | 'complete';

export interface RollGroupPresentationState {
  readonly phase: RollGroupPresentationPhase;
  readonly rerollIndex: number;
  readonly modifierIndex: number;
  readonly hydrated: boolean;
}

export type RollGroupPresentationAction =
  | { readonly type: 'release-delivered' }
  | { readonly type: 'originals-settled' }
  | { readonly type: 'reroll-flash-complete' }
  | { readonly type: 'reroll-settled' }
  | { readonly type: 'modifier-shown' }
  | { readonly type: 'hydrate-released-history' };

export function createRollGroupPresentationState(input: {
  readonly released: boolean;
  readonly hydrated: boolean;
  readonly rerollCount: number;
  readonly modifierCount: number;
}): RollGroupPresentationState;
export function reduceRollGroupPresentation(
  state: RollGroupPresentationState,
  action: RollGroupPresentationAction,
  counts: Readonly<{ rerollCount: number; modifierCount: number }>
): RollGroupPresentationState;
```

- [ ] **Step 1: Write RED event tests**

Test strict exact-key parsing, request-before-release, first immutable request wins, equal duplicate ignored, conflicting duplicate ignored, first compatible release wins, group-key mismatch refused, malformed profile refused, release containing `result`, `pointerId`, `damage`, or `presetId` refused, and all accepted outputs deeply frozen.

- [ ] **Step 2: Write RED lifecycle tests**

Pin these paths:

```text
armed -> rolling-originals -> settled-originals -> complete
armed -> rolling-originals -> settled-originals -> reroll-flash
      -> rerolling -> modifiers -> complete
hydrate released history -> complete
```

Assert duplicate/out-of-order actions return the same state object and cannot replay rerolls or modifiers.

- [ ] **Step 3: Run RED**

```bash
npm run test:run -- \
  src/components/ui/dice/diceRollGroupEvent.test.ts \
  src/components/ui/dice/rollGroupPresentationState.test.ts
```

Expected: FAIL with missing modules.

- [ ] **Step 4: Implement parsers/projection/reducer**

Mirror the proven defensive snapshot style in `dicePresentationEvent.ts`, but keep the files/types separate. Build lifecycle transitions only from supplied counts; never inspect face values or modifier arithmetic.

- [ ] **Step 5: Run GREEN plus production event regression**

```bash
npm run test:run -- \
  src/components/ui/dice/diceRollGroupEvent.test.ts \
  src/components/ui/dice/rollGroupPresentationState.test.ts \
  src/components/ui/dice/dicePresentationEvent.test.ts \
  src/components/ui/dice/DiceTrayPresentation.test.tsx
```

Expected: PASS.

- [ ] **Step 6: Commit**

```bash
git add src/components/ui/dice/diceRollGroupEvent.ts \
  src/components/ui/dice/diceRollGroupEvent.test.ts \
  src/components/ui/dice/rollGroupPresentationState.ts \
  src/components/ui/dice/rollGroupPresentationState.test.ts
git commit -m "feat(dice): add append-only roll-group lifecycle (#826)"
```

### Task 3: Camera-plane exact attachment and whole-group gesture

**Files:**
- Create: `src/components/ui/dice/trayPlaneProjection.ts`
- Create: `src/components/ui/dice/trayPlaneProjection.test.ts`
- Create: `src/components/ui/dice/anchoredRollGroupGestureController.ts`
- Create: `src/components/ui/dice/anchoredRollGroupGestureController.test.ts`
- Create: `src/components/ui/dice/TrayPlaneProjectionBridge.tsx`
- Create: `src/components/ui/dice/DiceTrayInteractionSurface.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.test.tsx`

**Interfaces:**
- Consumes: Three.js `Camera`, `Plane`, `Raycaster`, `Vector3`; existing `ClientBounds`, `PointerCaptureOwner`, `RollGroupPointerSample`, `DiceTray3DProps`; `VisualThrowProfileV1`.
- The production d20 path consumes `DiceTrayInteractionSurface` in `legacy-normalized` mode and remains pixel/motion compatible. `RollGroupTray3D` later consumes the same surface/controller in `tray-plane` mode with `TrayPlaneProjectionBridge`.
- Produces:

```ts
export type TrayPlanePoint = readonly [number, number];

export interface TrayPlaneProjection {
  readonly screenToPlane: (
    clientX: number,
    clientY: number
  ) => TrayPlanePoint | undefined;
  readonly planeToScreen: (
    point: TrayPlanePoint
  ) => readonly [number, number] | undefined;
  readonly planeToNormalized: (
    point: TrayPlanePoint
  ) => readonly [number, number] | undefined;
}

export function createTrayPlaneProjection(input: {
  readonly camera: Camera;
  readonly viewport: ClientBounds;
  readonly origin: readonly [number, number, number];
  readonly xAxis: readonly [number, number, number];
  readonly yAxis: readonly [number, number, number];
  readonly width: number;
  readonly height: number;
}): TrayPlaneProjection | undefined;
```

```ts
export interface AnchoredHeldRollGroupState {
  readonly anchor: TrayPlanePoint;
  readonly normalizedPosition: readonly [number, number];
  readonly normalizedTilt: readonly [number, number];
  readonly shakeEnergy: number;
  readonly wobblePhase: number;
  readonly grabbedDieId: string;
}

export interface AnchoredRollGroupGestureController {
  begin(input: {
    readonly sample: RollGroupPointerSample;
    readonly captureTarget: PointerCaptureOwner;
    readonly projection: TrayPlaneProjection;
    readonly hitRegions: readonly Readonly<{
      dieId: string;
      bounds: ClientBounds;
      memberAnchor: TrayPlanePoint;
      stableIndex: number;
    }>[];
    readonly hitPaddingPx: number;
    readonly motionSeed: number;
  }): AnchoredHeldRollGroupState | undefined;
  move(sample: RollGroupPointerSample): AnchoredHeldRollGroupState | undefined;
  release(sample: RollGroupPointerSample): VisualThrowProfileV1 | undefined;
  cancel(pointerId: number): boolean;
  reset(): void;
  held(): AnchoredHeldRollGroupState | undefined;
}
```

- [ ] **Step 1: Write RED projection tests**

Build deterministic perspective and top cameras using the current visual config. For center, quarter, and edge plane points, assert `planeToScreen(screenToPlane(x,y))` differs from input by at most `0.000001` CSS pixels in pure math. Reject zero viewport, non-unit/parallel axes, behind-camera intersections, and non-finite data.

Add a regression proving fixed X/Z mapping fails under the three-quarter camera while ray-plane mapping passes.

- [ ] **Step 2: Write RED gesture tests**

Pin exact grab-offset behavior: grabbing a die 7 plane units right/3 up from its own keyed member anchor and moving the pointer preserves that offset. Test every member hit region, 14px mouse/24px touch padding, pointer capture ownership, outside move/release, wrong pointer IDs, cancel/lost capture/reset, non-finite samples, exactly-once profile, and deep serialization denial. When padded regions overlap, choose a member containing the point in its unpadded region first; otherwise choose the nearest projected center; break exact ties by `stableIndex`:

```ts
expect(JSON.stringify({ held, profile })).not.toMatch(
  /pointerId|clientX|clientY|timeMs|path|samples/
);
```

- [ ] **Step 3: Run RED**

```bash
npm run test:run -- \
  src/components/ui/dice/trayPlaneProjection.test.ts \
  src/components/ui/dice/anchoredRollGroupGestureController.test.ts
```

Expected: FAIL with missing modules.

- [ ] **Step 4: Implement projection and controller**

Use `Raycaster.setFromCamera()` and `Ray.intersectPlane()` for screen→world. Convert intersections to the provided orthonormal basis. For world→screen, rebuild world position from basis and call `Vector3.project(camera)`. `TrayPlaneProjectionBridge` publishes that actual camera/viewport projection to `DiceTrayInteractionSurface` in `tray-plane` mode. In `legacy-normalized` mode the same surface/controller uses the existing DOM-bounds normalization and produces the same `HeldRollGroupState` consumed by `ChoreographedSolverV1`. Snapshot the projection and keyed hit regions at pointer-down; preserve the initial pointer-minus-selected-member anchor in exact mode. Reuse existing velocity/energy/profile rules only after projection.

- [ ] **Step 5: Run GREEN plus old controller regression**

```bash
npm run test:run -- \
  src/components/ui/dice/trayPlaneProjection.test.ts \
  src/components/ui/dice/anchoredRollGroupGestureController.test.ts \
  src/components/ui/dice/rollGroupGestureController.test.ts \
  src/components/ui/dice/choreographedDiceMotion.test.ts \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx
```

Expected: PASS; existing production request/release/held-motion/settlement output is unchanged, while the shared tray-plane mode passes exact anchor tests.

- [ ] **Step 6: Commit**

```bash
git add src/components/ui/dice/trayPlaneProjection.ts \
  src/components/ui/dice/trayPlaneProjection.test.ts \
  src/components/ui/dice/anchoredRollGroupGestureController.ts \
  src/components/ui/dice/anchoredRollGroupGestureController.test.ts \
  src/components/ui/dice/TrayPlaneProjectionBridge.tsx \
  src/components/ui/dice/DiceTrayInteractionSurface.tsx \
  src/components/ui/dice/DiceTray3D.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx
git commit -m "refactor(dice): share anchored tray interaction modes (#826)"
```

### Task 4: Stable group layouts and broad feel solvers

**Files:**
- Create: `src/components/ui/dice/rollGroupLayout.ts`
- Create: `src/components/ui/dice/rollGroupLayout.test.ts`
- Create: `src/components/ui/dice/rollGroupMotionSolver.ts`
- Create: `src/components/ui/dice/rollGroupMotionSolver.test.ts`

**Interfaces:**
- Consumes: `DiceRollGroupDie`, `DiceMotionPose`, `QuaternionTuple`, `VisualThrowProfileV1`, `AnchoredHeldRollGroupState`.
- Produces:

```ts
export interface RollGroupMemberLayout {
  readonly dieId: string;
  readonly center: readonly [number, number];
  readonly radius: number;
}

export function layoutHeldRollGroup(
  dice: readonly Pick<DiceRollGroupDie, 'id' | 'kind'>[]
): readonly RollGroupMemberLayout[];
export function layoutRestingRollGroup(
  dice: readonly Pick<DiceRollGroupDie, 'id' | 'kind'>[],
  motionSeed: number
): readonly RollGroupMemberLayout[];
```

```ts
export type RollGroupFeelCandidateId =
  | 'weighty'
  | 'energetic'
  | 'physical';

export interface RollGroupFeelProfile {
  readonly id: RollGroupFeelCandidateId;
  readonly displayName: string;
  readonly durationMs: number;
  readonly travel: number;
  readonly tumble: number;
  readonly rebound: number;
  readonly scatter: number;
  readonly rerollDurationMs: number;
  readonly flashDurationMs: number;
  readonly modifierDurationMs: number;
}

export const ROLL_GROUP_FEEL_PROFILES: Readonly<
  Record<RollGroupFeelCandidateId, RollGroupFeelProfile>
>;

export type RollGroupMotionPhase =
  | 'held'
  | 'rolling-originals'
  | 'settled-originals'
  | 'rerolling'
  | 'settled-final';

export function solveRollGroupMemberMotion(input: {
  readonly profile: RollGroupFeelProfile;
  readonly phase: RollGroupMotionPhase;
  readonly elapsedMs: number;
  readonly reducedMotion: boolean;
  readonly target: QuaternionTuple;
  readonly throwProfile: VisualThrowProfileV1;
  readonly memberIndex: number;
  readonly memberCount: number;
  readonly held?: AnchoredHeldRollGroupState;
  readonly affectedByCurrentReroll: boolean;
  readonly heldLayout: RollGroupMemberLayout;
  readonly restingLayout: RollGroupMemberLayout;
}): DiceMotionPose;
```

- [ ] **Step 1: Write RED layout tests**

For 1, 2, 3, 4, and 8 members, assert stable input order, finite centers, radius-aware non-overlap, compact held bounds, contained resting bounds, and deterministic seeded output. Include repeated d6 members to prove logical clones do not need collectible identity.

- [ ] **Step 2: Write RED candidate tests**

Assert:

- same input/profile/index returns byte-equal poses;
- all candidates settle the supplied quaternion exactly;
- Weighty, Energetic, and Physical differ materially at 25%, 50%, and 75% elapsed time;
- reduced motion settles directly without travel/tumble;
- only `affectedByCurrentReroll=true` members move during reroll;
- unaffected dice hold their exact current pose;
- invalid members/times/profiles return failed poses; and
- current `ChoreographedSolverV1` still emits its exact existing one-d20 values.

- [ ] **Step 3: Run RED**

```bash
npm run test:run -- \
  src/components/ui/dice/rollGroupLayout.test.ts \
  src/components/ui/dice/rollGroupMotionSolver.test.ts
```

Expected: FAIL with missing modules.

- [ ] **Step 4: Implement minimal deterministic layouts/solvers**

Use fixed, bounded profile records with intentionally separated values; do not expose sliders yet. Derive member phase from `motionSeed` plus index, and continuously converge to the supplied target before `durationMs`. Implement Physical as deterministic collision-like deflection/rebound; do not add Rapier or another physics dependency.

- [ ] **Step 5: Run GREEN and current solver regression**

```bash
npm run test:run -- \
  src/components/ui/dice/rollGroupLayout.test.ts \
  src/components/ui/dice/rollGroupMotionSolver.test.ts \
  src/components/ui/dice/choreographedDiceMotion.test.ts
```

Expected: PASS.

- [ ] **Step 6: Commit**

```bash
git add src/components/ui/dice/rollGroupLayout.ts \
  src/components/ui/dice/rollGroupLayout.test.ts \
  src/components/ui/dice/rollGroupMotionSolver.ts \
  src/components/ui/dice/rollGroupMotionSolver.test.ts
git commit -m "feat(dice): add broad deterministic group feels (#826)"
```

### Task 5: Explicit provisional multi-shape runtime renderer

**Files:**
- Create: `src/components/ui/dice/conceptDiceRuntimeProvider.ts`
- Create: `src/components/ui/dice/conceptDiceRuntimeProvider.test.ts`
- Regression only: `src/components/ui/dice/diceRuntimeProvider.test.ts`
- Modify: `src/components/ui/dice/diceSettlementObservation.ts`
- Modify: `src/components/ui/dice/diceSettlementObservation.test.ts`
- Create: `src/components/ui/dice/RuntimeDiceMesh.tsx`
- Create: `src/components/ui/dice/RuntimeDiceMesh.test.tsx`
- Modify: `src/components/ui/dice/AttackDie3D.tsx`
- Modify: `src/components/ui/dice/AttackDie3D.test.tsx`
- Create: `src/components/ui/dice/RollGroupDie3D.tsx`
- Create: `src/components/ui/dice/RollGroupDie3D.test.tsx`
- Create: `src/components/ui/dice/SemanticRollGroup.tsx`
- Create: `src/components/ui/dice/SemanticRollGroup.test.tsx`

**Interfaces:**
- Consumes: strict runtime manifest, `prepareMaterialFreeCarvedScene`, `runtimeDiceNormalization`, `resolveRuntimeDiceSettlement`, `DiceMaterialTreatment`, group motion poses.
- Produces:

```ts
export type DiceRuntimeAssurance = 'verified-production' | 'provisional-concept';

export interface ConceptDiceRuntimePresetSnapshot {
  readonly status: 'idle' | 'loading' | 'ready' | 'failed';
  readonly assurance: 'provisional-concept';
  readonly preset?: DiceRuntimePreset;
  readonly scene?: Object3D;
  readonly binding?: RuntimeMeshBinding;
  readonly failureReason?: string;
}

export function getConceptDiceRuntimePresetSnapshot(
  presetId: string
): ConceptDiceRuntimePresetSnapshot;
export function preloadConceptDiceRuntimePreset(
  presetId: string
): Promise<void>;
```

```ts
export interface RollGroupDie3DProps {
  readonly die: DiceRollGroupDie;
  readonly displayedFace: number;
  readonly presentationToken: number;
  readonly pose: DiceMotionPose;
  readonly treatment: DiceMaterialTreatment;
  readonly onReady?: (input: Readonly<{
    dieId: string;
    assurance: DiceRuntimeAssurance;
    runtimeSourceId: number;
    runtimeCloneId: number;
  }>) => void;
  readonly onFailure?: (dieId: string, reason: string) => void;
}
```

- [ ] **Step 1: Write RED provider tests**

Pin that existing `preloadDiceRuntimePreset('dice.original.carved.d6')` still fails as not allowlisted and the production provider file remains byte-unchanged. The separate `conceptDiceRuntimeProvider.ts` owns its own manifest promise, request owners, snapshots, cache keys, and cache. It strictly parses the existing manifest, selects only single-mesh/single-mesh-triangle-group `dice.original.carved.{d4,d6,d8,d10,d12}`, verifies model size/SHA-256 and exact glTF object-node→mesh-definition binding, and labels every result provisional. It deliberately does not call production `loadPresetEntry` or d20-only `validateFaceWitnessGeometry`; it does not claim semantic face correctness. The group requests d20 only through existing `getDiceRuntimePresetSnapshot/preloadDiceRuntimePreset`. Reject d20, percentile d10, painted families, unsafe IDs, bad hash/binding, and mismatched preset kind in the concept loader.

- [ ] **Step 2: Write RED generic observation tests**

Add `observeUpwardPresetResult(entries, supportedResults, quaternion)` for d4/d6/d8/d10/d12. Preserve `observeUpwardResult(entries, quaternion)` as the strict 1–20 wrapper with unchanged error behavior and tests.

- [ ] **Step 3: Write RED renderer/fallback tests**

Mock ready snapshots and assert:

- each kind validates `displayedFace` through the preset's supported result list;
- two dice share immutable provider source but receive distinct prepared scene clone IDs;
- fixture treatments apply without mutating provider scene/materials;
- missing/unmapped/provisional failure calls `onFailure` once;
- simultaneous production d20 plus concept d6 loads keep separate request/cache ownership and cannot downgrade or alias d20 assurance;
- `AttackDie3D` and `RollGroupDie3D` both mount the extracted `RuntimeDiceMesh`, while production telemetry/context lifecycle remains unchanged; and
- semantic fallback renders `d4 ?` while concealed, then `d4 3`, reroll `1 → 4`, and final labels without drawing a d20 polygon.

- [ ] **Step 4: Run RED**

```bash
npm run test:run -- \
  src/components/ui/dice/conceptDiceRuntimeProvider.test.ts \
  src/components/ui/dice/diceRuntimeProvider.test.ts \
  src/components/ui/dice/diceSettlementObservation.test.ts \
  src/components/ui/dice/RuntimeDiceMesh.test.tsx \
  src/components/ui/dice/RollGroupDie3D.test.tsx \
  src/components/ui/dice/SemanticRollGroup.test.tsx
```

Expected: new tests FAIL.

- [ ] **Step 5: Implement concept API and R3F child**

Implement the separate concept loader described above without importing private production provider loaders or mutating production caches. Local duplication of fetch/hash/binding mechanics is accepted here to preserve the verified d20 boundary; `CONTRACT.md` records this as concept evidence, not a production provider design. Extract scene preparation, immutable source ownership, clone/material lifecycle, pose application, and cleanup from `AttackDie3D` into `RuntimeDiceMesh`; make `AttackDie3D` use that component without changing its Canvas, telemetry, observation, fallback, or public props. `RollGroupDie3D` mounts the same `RuntimeDiceMesh` as a child of the group Canvas. It calls the unchanged verified provider for d20 and the separate provisional concept provider for d4/d6/d8/d10/d12.

- [ ] **Step 6: Run GREEN plus exact d20 regressions**

```bash
npm run test:run -- \
  src/components/ui/dice/conceptDiceRuntimeProvider.test.ts \
  src/components/ui/dice/diceRuntimeProvider.test.ts \
  src/components/ui/dice/diceSettlementObservation.test.ts \
  src/components/ui/dice/diceSettlementResolver.test.ts \
  src/components/ui/dice/materialFreeCarvedMesh.test.ts \
  src/components/ui/dice/RuntimeDiceMesh.test.tsx \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/RollGroupDie3D.test.tsx \
  src/components/ui/dice/SemanticRollGroup.test.tsx
git diff --exit-code origin/dev -- src/components/ui/dice/diceRuntimeProvider.ts
```

Expected: PASS; production provider bytes and d20 semantics unchanged.

- [ ] **Step 7: Commit**

```bash
git add src/components/ui/dice/conceptDiceRuntimeProvider.ts \
  src/components/ui/dice/conceptDiceRuntimeProvider.test.ts \
  src/components/ui/dice/diceSettlementObservation.ts \
  src/components/ui/dice/diceSettlementObservation.test.ts \
  src/components/ui/dice/RuntimeDiceMesh.tsx \
  src/components/ui/dice/RuntimeDiceMesh.test.tsx \
  src/components/ui/dice/AttackDie3D.tsx \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/RollGroupDie3D.tsx \
  src/components/ui/dice/RollGroupDie3D.test.tsx \
  src/components/ui/dice/SemanticRollGroup.tsx \
  src/components/ui/dice/SemanticRollGroup.test.tsx
git commit -m "feat(dice): render provisional carved roll groups (#826)"
```

### Task 6: Shared group tray, witnesses, rerolls, and modifiers

**Files:**
- Create: `src/components/ui/dice/RollGroupTray3D.tsx`
- Create: `src/components/ui/dice/RollGroupTray3D.test.tsx`
- Create: `src/components/ui/dice/RollGroupPresentation.tsx`
- Create: `src/components/ui/dice/RollGroupPresentation.test.tsx`
- Modify: `src/components/ui/dice/DiceTrayPresentation.tsx`
- Modify: `src/components/ui/dice/DiceTrayPresentation.test.tsx`

**Interfaces:**
- Consumes: Tasks 1–5 types/parsers/reducer/projection/controller/layout/solver/renderer/fallback.
- Produces:

```ts
export interface RollGroupDieAppearance {
  readonly dieId: string;
  readonly treatment: DiceMaterialTreatment;
}

export interface RollGroupAttachmentDiagnostic {
  readonly presentationId: string;
  readonly groupKey: DiceRollGroupKey;
  readonly witnessRole: 'roller' | 'spectator';
  readonly rendererGeneration: number;
  readonly dieId: string;
  readonly projectedAnchor: readonly [number, number];
  readonly heldPoseApplied: boolean;
  readonly frameSequence: number;
}

export interface DiceRollGroupPresentationProps {
  readonly mode: 'roll-group';
  readonly label: string;
  readonly events: readonly DiceRollGroupEvent[];
  readonly witnessRole: 'roller' | 'spectator';
  readonly feel: RollGroupFeelCandidateId;
  readonly appearances: readonly RollGroupDieAppearance[];
  readonly onReleaseRequest?: (event: DiceRollGroupReleasedEvent) => void;
  readonly onMount?: (mount: Readonly<{
    presentationId: string;
    groupKey: DiceRollGroupKey;
    witnessRole: 'roller' | 'spectator';
    rendererGeneration: number;
  }>) => void;
  readonly onComplete?: (completion: Readonly<{
    presentationId: string;
    groupKey: DiceRollGroupKey;
    witnessRole: 'roller' | 'spectator';
    rendererGeneration: number;
    renderer: '3d' | 'semantic';
  }>) => void;
  readonly onAttachmentDiagnostic?: (
    diagnostic: RollGroupAttachmentDiagnostic
  ) => void;
  readonly reducedMotion?: boolean;
  readonly forceFailure?: 'provider' | 'webgl' | 'solver';
  readonly onDiagnostic?: (diagnostic: Readonly<{
    presentationId: string;
    groupKey: DiceRollGroupKey;
    witnessRole: 'roller' | 'spectator';
    rendererGeneration: number;
    feel: RollGroupFeelCandidateId;
    releaseAccepted: boolean;
    originalsSettled: boolean;
    rerollsCompleted: number;
    modifiersCompleted: number;
    fallback: boolean;
  }>) => void;
}
```

`DiceTrayPresentation` keeps its existing legacy function signature and adds an overload accepting `DiceRollGroupPresentationProps` only when `mode: 'roll-group'` is present. Concept callers import `DiceTrayPresentation`, never the internal `RollGroupPresentation` directly.

- [ ] **Step 1: Write RED tray tests**

Mock Canvas/die children and prove one Canvas owns all members, every member supplies a hit region, grabbing any member picks up the entire group, explicit Roll creates the neutral group profile, spectator exposes no controls, candidate selection reaches every member solver, reduced motion suppresses travel, and one member failure activates one semantic group fallback without stalling.

- [ ] **Step 2: Write RED presentation tests**

Use two independent `DiceTrayPresentation` group-overload instances and assert:

- roller request alone remains armed indefinitely in normal mode;
- release callback requests append but phase changes only when event appears;
- roller/witness consume equal frozen event/profile values with distinct renderer generations/source clones;
- each witness emits one unconditional `onMount` before ready/failure and one matching `onComplete` after 3D or semantic completion;
- provider, WebGL, and solver failure all prove mount-before-semantic-complete without a boundary/provider diagnostic;
- duplicate/conflicting releases do not replay;
- initial released history completes immediately;
- the component never synthesizes a missing release; it consumes only host-appended events;
- `onMount` fires unconditionally once renderer generation is allocated, before provider/Canvas readiness, for roller and spectator independently;
- provider/WebGL/solver failure still emits `onMount` before semantic `onComplete`;
- `onComplete` fires exactly once per witness/presentation generation after 3D or semantic completion and carries presentation ID, group key, witness role, and renderer generation needed by the attack→damage barrier;
- each member hit target exposes only stable `data-roll-group-die-id`, `data-renderer-generation`, and `data-witness-role` selectors;
- attachment diagnostics identify that same die/generation and carry the projected actual grabbed surface point after the member's rendered transform—not merely its center—plus held/frame facts but no pointer sample;
- original faces settle before reroll flash;
- only fixture-supplied reroll die IDs move;
- matching reason labels batch once;
- modifiers appear in supplied order and final total is rendered verbatim; and
- semantic live output announces each phase once.

- [ ] **Step 3: Run RED**

```bash
npm run test:run -- \
  src/components/ui/dice/RollGroupTray3D.test.tsx \
  src/components/ui/dice/RollGroupPresentation.test.tsx
```

Expected: FAIL with missing components.

- [ ] **Step 4: Implement tray/presentation**

Use one R3F Canvas with the same `TrayPlaneProjectionBridge`, `DiceTrayInteractionSurface`, anchored controller, and `RuntimeDiceMesh` extracted under the production d20 chain. Apply held updates on the next R3F frame rather than waiting for React layout. Add a `DiceTrayPresentation` function overload that dispatches `mode: 'roll-group'` to the internal component while routing legacy props through the unchanged implementation. The presentation never owns missing-release timing; it consumes append-only host events. Allocate renderer generation and fence one unconditional `onMount` before any provider/Canvas work. Fence `onComplete`, attachment diagnostics, reroll flash/motion, and modifier timers by presentation ID, witness role, and renderer generation under StrictMode. `onComplete` fires only after renderer or semantic fallback completion; the component itself does not advance the attack→damage coordinator. Compute `projectedAnchor` by transforming the retained grabbed local point through the rendered member pose and actual camera projection on that frame; emit only when its die ID and renderer generation still match the active capture.

- [ ] **Step 5: Run GREEN and production tray regression**

```bash
npm run test:run -- \
  src/components/ui/dice/RollGroupTray3D.test.tsx \
  src/components/ui/dice/RollGroupPresentation.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx \
  src/components/session/combat-experience/CombatExperience.test.tsx
```

Expected: PASS.

- [ ] **Step 6: Commit**

```bash
git add src/components/ui/dice/RollGroupTray3D.tsx \
  src/components/ui/dice/RollGroupTray3D.test.tsx \
  src/components/ui/dice/RollGroupPresentation.tsx \
  src/components/ui/dice/RollGroupPresentation.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx
git commit -m "feat(dice): present shared multi-die roll groups (#826)"
```

### Task 7: Strict scenarios and attack→damage concept coordinator

**Files:**
- Create: `src/concepts/attack-die-3d/sharedTableDiceFixtures.ts`
- Create: `src/concepts/attack-die-3d/sharedTableDiceFixtures.test.ts`
- Create: `src/concepts/attack-die-3d/sharedTableDiceState.ts`
- Create: `src/concepts/attack-die-3d/sharedTableDiceState.test.ts`
- Create: `src/concepts/attack-die-3d/sharedTableDiceDelivery.ts`
- Create: `src/concepts/attack-die-3d/sharedTableDiceDelivery.test.ts`

**Interfaces:**
- Consumes: group contract/events and `createNeutralVisualThrowProfile`.
- Produces:

```ts
export type SharedTableDiceScenarioId =
  | 'single-d20'
  | 'bless-mixed-attack'
  | 'ordinary-damage'
  | 'critical-damage'
  | 'great-weapon-fighting'
  | 'duplicate-release'
  | 'missing-release'
  | 'reduced-motion'
  | 'provider-failure';

export interface SharedTableDicePlayerFixture {
  readonly memberId: string;
  readonly name: string;
  readonly setId: string;
}

export interface SharedTableDiceSetFixture {
  readonly id: string;
  readonly displayName: string;
  readonly treatment: DiceMaterialTreatment;
  readonly presetByKind: Readonly<Record<DiceKind, string>>;
}

export interface SharedTableDiceScenario {
  readonly id: SharedTableDiceScenarioId;
  readonly label: string;
  readonly rollerMemberId: string;
  readonly witnessMemberId: string;
  readonly players: readonly SharedTableDicePlayerFixture[];
  readonly sets: readonly SharedTableDiceSetFixture[];
  readonly attack: DiceRollGroupInput;
  readonly damage?: DiceRollGroupInput;
  readonly hit: boolean;
  readonly impactLabel?: string;
  readonly exercise?: 'duplicate-release' | 'missing-release' | 'provider-failure';
}

export const SHARED_TABLE_DICE_SCENARIOS: Readonly<
  Record<SharedTableDiceScenarioId, SharedTableDiceScenario>
>;
```

```ts
export type SharedTableDicePhase =
  | 'attack'
  | 'attack-verdict'
  | 'damage'
  | 'impact'
  | 'complete';

export interface SharedTableDiceActivePresentation {
  readonly presentationId: string;
  readonly groupKey: DiceRollGroupKey;
  readonly generations: Readonly<{
    roller?: number;
    spectator?: number;
  }>;
  readonly completed: Readonly<{
    roller: boolean;
    spectator: boolean;
  }>;
}

export interface SharedTableDiceState {
  readonly scenarioId: SharedTableDiceScenarioId;
  readonly phase: SharedTableDicePhase;
  readonly activePresentation?: SharedTableDiceActivePresentation;
}

export function reduceSharedTableDice(
  state: SharedTableDiceState,
  action:
    | {
        readonly type: 'presentation-mounted';
        readonly presentationId: string;
        readonly groupKey: DiceRollGroupKey;
        readonly witnessRole: 'roller' | 'spectator';
        readonly rendererGeneration: number;
      }
    | {
        readonly type: 'group-complete';
        readonly presentationId: string;
        readonly groupKey: DiceRollGroupKey;
        readonly witnessRole: 'roller' | 'spectator';
        readonly rendererGeneration: number;
      }
    | { readonly type: 'verdict-complete' }
    | { readonly type: 'impact-complete' },
  scenario: SharedTableDiceScenario
): SharedTableDiceState;

export interface SharedTableDiceDeliveryHost {
  readonly events: () => readonly DiceRollGroupEvent[];
  readonly append: (event: DiceRollGroupEvent) => boolean;
  readonly scheduleMissingRelease: (input: Readonly<{
    presentationId: string;
    groupKey: DiceRollGroupKey;
    presetSeed: number;
    graceMs: 3_000;
  }>) => () => void;
  readonly reset: () => void;
}

export function createSharedTableDiceDeliveryHost(
  onChange: (events: readonly DiceRollGroupEvent[]) => void
): SharedTableDiceDeliveryHost;
```

- [ ] **Step 1: Write RED fixture tests**

Pin exact facts:

- `single-d20`: one actor d20;
- `bless-mixed-attack`: actor d20 plus contributor d4 with different set/treatment;
- `ordinary-damage`: at least two dice;
- `critical-damage`: explicit extra critical-purpose dice;
- `great-weapon-fighting`: three damage dice, two reroll chains `1→5` and `2→4`, one unaffected `6`;
- failure/reduced/duplicate scenarios reuse exact supplied facts without mutation.

Validate player/set uniqueness, roller/witness membership, contributor→player→set consistency, preset kind mapping, and visible `Fixture / simulated` labels. Never sum fixture totals.

- [ ] **Step 2: Write RED coordinator/delivery tests**

Assert miss path `attack → attack-verdict → complete`; hit path `attack → attack-verdict → damage → impact → complete`. `presentation-mounted` records the active presentation ID/group plus independent roller and spectator generations; it is accepted before any provider readiness. `group-complete` marks only the matching witness generation complete. Advance only after both roller and spectator mounted and completed; stale generation, wrong role/presentation/group, completion before that witness mounted, semantic fallback without prior mount, and duplicate matching completion are ignored/idempotent. Delivery is the sole missing-release owner: tests pin first append, equal/conflicting duplicate, one scheduled neutral release after `3_000ms`, timer cleanup on real release/scenario/replay/unmount, and StrictMode one-shot behavior. `RollGroupPresentation` never schedules or appends this fallback.

- [ ] **Step 3: Run RED**

```bash
npm run test:run -- \
  src/concepts/attack-die-3d/sharedTableDiceFixtures.test.ts \
  src/concepts/attack-die-3d/sharedTableDiceState.test.ts \
  src/concepts/attack-die-3d/sharedTableDiceDelivery.test.ts
```

Expected: FAIL with missing modules.

- [ ] **Step 4: Implement fixtures/reducer/delivery host**

Use two material treatments over strict Original carved preset IDs to prove cohesive player sets without claiming ownership. Freeze every fixture. Keep the 3,000ms grace in concept delivery only and expose it in diagnostics for feel review. The stage dispatches `presentation-mounted` directly from each group's unconditional `onMount` callback, never from an optional provider/boundary diagnostic. It forwards each `onComplete` as `group-complete` with exact presentation ID, group key, witness role, and generation. Attack advances to verdict only after both roller and spectator complete; a miss completes after verdict; a hit mounts exactly one damage presentation with two new active generations; damage advances to impact only after both complete. Semantic fallback participates in the same barrier because mount is unconditional.

- [ ] **Step 5: Run GREEN**

```bash
npm run test:run -- \
  src/concepts/attack-die-3d/sharedTableDiceFixtures.test.ts \
  src/concepts/attack-die-3d/sharedTableDiceState.test.ts \
  src/concepts/attack-die-3d/sharedTableDiceDelivery.test.ts
```

Expected: PASS.

- [ ] **Step 6: Commit**

```bash
git add src/concepts/attack-die-3d/sharedTableDiceFixtures.ts \
  src/concepts/attack-die-3d/sharedTableDiceFixtures.test.ts \
  src/concepts/attack-die-3d/sharedTableDiceState.ts \
  src/concepts/attack-die-3d/sharedTableDiceState.test.ts \
  src/concepts/attack-die-3d/sharedTableDiceDelivery.ts \
  src/concepts/attack-die-3d/sharedTableDiceDelivery.test.ts
git commit -m "concept(dice): add shared-table roll scenarios (#826)"
```

### Task 8: Playable feel stage, responsive UI, and documentation

**Files:**
- Create: `src/concepts/attack-die-3d/SharedTableDiceStage.tsx`
- Create: `src/concepts/attack-die-3d/SharedTableDiceStage.test.tsx`
- Create: `src/concepts/attack-die-3d/sharedTableDiceEvidence.ts`
- Create: `src/concepts/attack-die-3d/sharedTableDiceEvidence.test.ts`
- Create: `src/concepts/attack-die-3d/CONTRACT.md`
- Create: `src/concepts/attack-die-3d/sharedTableDiceBoundary.test.ts`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx`
- Modify: `public/themes/base.css`
- Modify: `docs/how-to/attack-die-3d-concept.md`
- Create: `docs/evidence/826-shared-table-dice/README.md`

**Interfaces:**
- Consumes: Tasks 1–7.
- Produces: `SharedTableDiceStage` mounted under the existing Tray stage and reproducible URL `/?concept=attack-die-3d&attackDieStage=tray`.

- [ ] **Step 1: Write RED stage tests**

Test keyboard-accessible candidate radios for Weighty/Energetic/Physical; scenario select with every required ID; Replay; reduced-motion toggle; Roller and Witness regions; `Fixture data`, `Simulated delivery`, and `Provisional non-d20 assets` labels; spectator no controls; contributor/set names; reroll cue; modifier toast; supplied final total; fallback status; and scenario/candidate changes resetting old timers/events. `sharedTableDiceEvidence.test.ts` pins a separate `window.__sharedTableDiceEvidence` bridge containing only revision, presentation/group/witness/generation/die identity, projected rendered anchor, held-pose boolean, and frame sequence—never pointer samples, results, damage, URLs, or renderer resources.

- [ ] **Step 2: Add RED responsive/source guards**

Extend `DiceTray3DConceptPanel.test.tsx` to prove existing Stone 1 pane still mounts unchanged and the new substage is explicit. In `sharedTableDiceBoundary.test.ts`, assert fixture truth/scheduling files may import shared dice value types but shared `src/components/ui/dice` files never import concept fixtures, and no new concept file imports `src/components/session`, `src/api`, generated protos, or network clients. Add source assertions that new CSS selectors are scoped under `.shared-table-dice-stage`.

- [ ] **Step 3: Run RED**

```bash
npm run test:run -- \
  src/concepts/attack-die-3d/SharedTableDiceStage.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx
```

Expected: FAIL because stage/substage does not exist.

- [ ] **Step 4: Implement stage and scoped CSS**

Use a compact control header and one gameplay-scale tray area, not nested cards. Keep reset/replay reachable without scrolling. Wire group `onComplete` callbacks to the exact active presentation/group IDs in `sharedTableDiceState`; do not infer completion from timers. Publish generation-fenced attachment diagnostics through the separate monotonic `window.__sharedTableDiceEvidence` bridge and clear it on scenario/replay/unmount. At ≤1024px stack controls above the two witness panes; at narrow touch width show one pane at a time with accessible Roller/Witness tabs. Do not change existing global grab-target selector values.

- [ ] **Step 5: Write review docs/evidence template**

Document exact launch:

```bash
npm run dev -- --host 127.0.0.1 --port 3010
# open http://127.0.0.1:3010/?concept=attack-die-3d&attackDieStage=tray
```

Write `CONTRACT.md` in the same evidence-not-asks form as `src/concepts/combat-pacing/CONTRACT.md`: fixture roll truth, local release scheduling, contributor sets, and non-d20 assurance are concept evidence; no Platform issue or production contract is claimed. The evidence README records commit, candidate/scenario matrix, attachment measurement, console/page errors, provisional asset caveat, private screenshot hashes if captured, and `Kirk feel gate: pending live review`. It must not claim approval before Kirk throws it.

- [ ] **Step 6: Run GREEN plus complete concept/dice gate**

```bash
npm run test:run -- \
  src/components/ui/dice \
  src/concepts/attack-die-3d \
  src/components/session/combat-experience
```

Expected: PASS.

- [ ] **Step 7: Commit**

```bash
git add src/concepts/attack-die-3d/SharedTableDiceStage.tsx \
  src/concepts/attack-die-3d/SharedTableDiceStage.test.tsx \
  src/concepts/attack-die-3d/sharedTableDiceEvidence.ts \
  src/concepts/attack-die-3d/sharedTableDiceEvidence.test.ts \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/CONTRACT.md \
  src/concepts/attack-die-3d/sharedTableDiceBoundary.test.ts \
  public/themes/base.css \
  docs/how-to/attack-die-3d-concept.md \
  docs/evidence/826-shared-table-dice/README.md
git commit -m "concept(dice): add shared table feel lab (#826)"
```

### Task 9: Browser measurement, regression, and review-ready PR

**Files:**
- Create: `scripts/attack-die/measure-shared-table-attachment.mjs`
- Modify only if evidence facts require correction: `docs/evidence/826-shared-table-dice/README.md`
- No production source changes.

**Interfaces:**
- Consumes: complete concept at Tasks 1–8.
- Produces: exact-head automated/browser evidence and PR for #826; human feel gate remains pending until Kirk returns.

- [ ] **Step 1: Run formatting and focused tests**

```bash
npm run format
npm run test:run -- \
  src/components/ui/dice \
  src/concepts/attack-die-3d \
  src/components/session/combat-experience
git diff --check
```

Expected: PASS.

- [ ] **Step 2: Start the exact worktree concept**

```bash
npm run dev -- --host 127.0.0.1 --port 3010
```

Use the browser harness against:

```text
http://127.0.0.1:3010/?concept=attack-die-3d&attackDieStage=tray
```

At 1280×800, 1024×768, and a narrow touch viewport, exercise all candidates and required scenarios. Record zero page/console errors or list exact failures.

- [ ] **Step 3: Measure real attachment**

Create `scripts/attack-die/measure-shared-table-attachment.mjs`. It accepts `<url> <output-json>`, opens system Chromium through Playwright, enumerates `[data-witness-role="roller"][data-roll-group-die-id][data-renderer-generation]`, and pointer-downs at both center and an off-center point inside each member. For each move through center/quarter/edge tray samples, wait two `requestAnimationFrame` callbacks (the requirement is satisfied on the first; the second makes observation stable). Read `window.__sharedTableDiceEvidence` only when its die ID and renderer generation equal the selected DOM target, and compare its `projectedAnchor`—the actual rendered grabbed surface point—to coordinates owned only by the probe. Require max Euclidean error ≤2 CSS pixels, exit nonzero on missing/stale generation or error, and write only aggregate/sample error—not pointer histories—to output. Run:

```bash
node scripts/attack-die/measure-shared-table-attachment.mjs \
  'http://127.0.0.1:3010/?concept=attack-die-3d&attackDieStage=tray' \
  /tmp/shared-table-dice-attachment.json
node --check scripts/attack-die/measure-shared-table-attachment.mjs
```

Expected: exit 0 and JSON `maximumErrorCssPx <= 2`.

- [ ] **Step 4: Verify production isolation**

```bash
git diff --name-only origin/dev...HEAD -- \
  src/components/session src/api src/App.tsx
git diff origin/dev...HEAD -- \
  src/components/ui/dice/dicePresentationEvent.ts \
  src/components/ui/dice/dicePresentationRelease.ts \
  src/components/ui/dice/visualThrowProfile.ts
```

Expected: no production/session/API/App paths and no existing production event/release/profile schema diffs.

- [ ] **Step 5: Run full gates**

```bash
npm run test:run
npm run ci-check
npm run build
git diff --check
```

Expected: all pass; only the existing expected >500KB Vite chunk warning is allowed.

- [ ] **Step 6: Update evidence with exact facts and commit**

```bash
git add scripts/attack-die/measure-shared-table-attachment.mjs \
  docs/evidence/826-shared-table-dice/README.md
git commit -m "test(dice): record shared table concept evidence (#826)"
```

The measurement script is always committed. Do not create any additional empty evidence commit.

- [ ] **Step 7: Run pre-push gate and push**

```bash
npm run ci-check
git push -u origin concept/826-shared-table-dice
```

Expected: pre-push hooks and push pass without bypass.

- [ ] **Step 8: Open the web PR**

Target `dev`, link `Closes #826`, parent journey #289, and design PR #291. Include exact test totals, browser viewports, 2px attachment result, provisional non-d20 caveat, private artifact policy, production-isolation diff, and signature:

```text
— ui-ux agent, on behalf of KirkDiggler
```

Request code review, but leave `Kirk feel gate: pending` until Kirk returns and throws every named scenario.

## Plan self-review checklist

Completed 2026-08-26 against every section of the approved design:

- [x] Every design requirement maps to Tasks 1–9.
- [x] No task modifies protos, toolkit, API, deployment, or production session files.
- [x] Existing exact-key d20 event/release/profile schemas remain untouched.
- [x] Type names/signatures are consistent across tasks.
- [x] Every implementation instruction names exact behavior, interfaces, commands, and expected output.
- [x] Every task has RED, expected failure, minimal GREEN, focused regression, and commit.
- [x] Human feel approval remains a live gate rather than an automated claim.
