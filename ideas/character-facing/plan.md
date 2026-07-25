# Character Facing Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make characters face their direction of travel, by turning heading into a concept the entity owns rather than a hardcoded constant with three ideas fused into it.

**Architecture:** A pure math module (`facing.ts`) computes and eases headings. A hook (`useEntityFacing`) owns an entity's heading in refs and writes `group.rotation.y` imperatively under `frameloop="demand"`. `useHexMovePath` gains a `segmentHeading` reporter and an `onHeading` callback but does **not** own facing — that keeps the seam open for attack-facing later. The single `Math.PI` constant splits into a per-model forward offset and a per-type staging default, calibrated by measurement and gated on visual parity.

**Tech Stack:** TypeScript, React, `@react-three/fiber`, three.js, Vitest, `@testing-library/react`.

**Repo:** all code changes are in `rpg-dnd5e-web`. This plan lives in `rpg-project` per house convention.

**Design:** `ideas/character-facing/design.md` (this directory). Issue: rpg-dnd5e-web#590.

---

## Read before starting

- `src/components/hex-grid/useHexMovePath.ts` — the module doc comment is long and load-bearing. In particular: the caller contract forbids mixing a declarative prop with the imperative writes this hook makes, and the pure-function split (`computeMoveStart`/`advanceFrame`) exists so tests need no WebGL canvas.
- `src/components/hex-grid/useHexMovePath.test.ts` — the `vi.hoisted` + mocked-`useFrame` harness. **Reuse it verbatim; do not invent a new one.** It captures the frame callback the hook registers and lets a test tick frames manually.
- `CLAUDE.md` in `rpg-dnd5e-web` — **run `npm run ci-check` before every push, and never use `--no-verify`.**

**Test commands:**
- Single file: `npx vitest run src/components/hex-grid/facing.test.ts`
- All: `npm run test:run`
- Full gate before pushing: `npm run ci-check`

---

## File structure

| File | Responsibility |
|---|---|
| `src/components/hex-grid/facing.ts` (create) | Pure heading math + the facing constants. No R3F imports. |
| `src/components/hex-grid/facing.test.ts` (create) | Unit tests for the above. No canvas. |
| `src/components/hex-grid/useEntityFacing.ts` (create) | Owns one entity's heading; eases it onto `group.rotation.y`. |
| `src/components/hex-grid/useEntityFacing.test.ts` (create) | Hook tests via the mocked-`useFrame` harness. |
| `src/components/hex-grid/useHexMovePath.ts` (modify) | Adds `segmentHeading` + `onHeading`; takes a caller-owned group ref. |
| `src/components/hex-grid/useHexMovePath.test.ts` (modify) | Adds heading-reporting coverage. |
| `src/components/hex-grid/HexEntity.tsx` (modify) | Owns the group ref, wires the two hooks, passes the split constants. |
| `src/character/creation/AppearanceSelectionModal.tsx` (modify, conditional) | Only if calibration finds `MediumHumanoid` needs a nonzero offset. |

---

## Task 1: Pure heading math (`facing.ts`)

**Files:**
- Create: `src/components/hex-grid/facing.ts`
- Test: `src/components/hex-grid/facing.test.ts`

- [ ] **Step 1: Write the failing tests**

```ts
import { describe, expect, it } from 'vitest';
import { cubeToWorld, HEX_DIRECTIONS } from './hexMath';
import {
  easeHeading,
  headingFromDelta,
  shortestTurn,
  TURN_RATE_RAD_PER_SEC,
} from './facing';

const deg = (r: number) => ((((r * 180) / Math.PI) % 360) + 360) % 360;
const rad = (d: number) => (d * Math.PI) / 180;

describe('headingFromDelta', () => {
  // The six hex neighbours land exactly 60 degrees apart under this
  // cubeToWorld. Verified numerically: E 90, NE 150, NW 210, W 270, SW 330,
  // SE 30. This is the test that pins the atan2(dx, dz) convention -- a
  // Three.js object at rotation.y = t maps its local +Z to (sin t, 0, cos t),
  // so atan2(dx, dz) is that function's exact inverse.
  const EXPECTED_DEG = [90, 150, 210, 270, 330, 30];

  it.each(HEX_DIRECTIONS.map((d, i) => [i, d, EXPECTED_DEG[i]] as const))(
    'direction %i has heading %s deg',
    (_i, dir, expected) => {
      const w = cubeToWorld(dir, 1);
      expect(deg(headingFromDelta(w.x, w.z)!)).toBeCloseTo(expected, 6);
    }
  );

  it('returns undefined for a zero-length delta', () => {
    // The rpg-api#656 degenerate same-hex "move" produces exactly this.
    // atan2(0, 0) would return a misleading 0 (due north); undefined lets
    // the caller hold its current heading instead of snapping.
    expect(headingFromDelta(0, 0)).toBeUndefined();
  });
});

describe('shortestTurn', () => {
  it('takes the short way across the wrap boundary', () => {
    expect(deg(shortestTurn(rad(350), rad(10)))).toBeCloseTo(20, 6);
  });

  it('turns negative when the short way is clockwise', () => {
    expect((shortestTurn(rad(10), rad(350)) * 180) / Math.PI).toBeCloseTo(-20, 6);
  });

  it('is zero when already facing the target', () => {
    expect(shortestTurn(rad(42), rad(42))).toBe(0);
  });

  it('pins the exact-180 tie-break to +PI regardless of input sign', () => {
    // A full reversal (walk east, then west) is a common real path and sits
    // exactly on the boundary where +PI and -PI are equally short. Either is
    // visually fine, but an unpinned tie-break is a coin flip that makes turn
    // direction non-deterministic across runs.
    expect(shortestTurn(0, Math.PI)).toBeCloseTo(Math.PI, 12);
    expect(shortestTurn(Math.PI, 0)).toBeCloseTo(Math.PI, 12);
  });
});

describe('easeHeading', () => {
  it('steps toward the target at no more than turnRate * delta', () => {
    const next = easeHeading(0, Math.PI, 0.1, 8);
    expect(next).toBeCloseTo(0.8, 6);
  });

  it('lands exactly on the target rather than overshooting', () => {
    // Must be === target, not approximately: useEntityFacing stops its
    // invalidate loop on an exact current === target comparison.
    expect(easeHeading(0, 0.5, 1, 8)).toBe(0.5);
  });

  it('is a no-op when already at the target', () => {
    expect(easeHeading(1.25, 1.25, 0.016, 8)).toBe(1.25);
  });

  it('eases the short way across the wrap boundary', () => {
    const next = easeHeading(rad(350), rad(10), 0.01, 8);
    expect(deg(next)).toBeCloseTo(354.58, 1);
  });

  it('resolves a 180 degree reversal inside one hex step', () => {
    // SECONDS_PER_HEX_STEP is 0.45; PI / 8 = 0.393s.
    expect(Math.PI / TURN_RATE_RAD_PER_SEC).toBeLessThan(0.45);
  });
});
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `npx vitest run src/components/hex-grid/facing.test.ts`
Expected: FAIL — cannot resolve `./facing`.

- [ ] **Step 3: Write the implementation**

```ts
/**
 * Pure heading math for character facing (rpg-dnd5e-web#590).
 *
 * No `@react-three/fiber` imports, deliberately — same reasoning as
 * `useHexMovePath.ts`'s `computeMoveStart`/`advanceFrame` split: it lets
 * `facing.test.ts` exercise every rule here directly, with no WebGL canvas
 * and no R3F test renderer.
 *
 * Convention: a "heading" is a world-space Y rotation in radians, in the
 * Three.js sense — an Object3D at `rotation.y = t` maps its local +Z to
 * world `(sin t, 0, cos t)`. `headingFromDelta` is that function's exact
 * inverse. A rig whose forward axis is NOT +Z needs a per-model offset
 * composed on top; that offset is a property of the asset and is kept as a
 * separate term (see MODEL_FORWARD_OFFSET below), never folded in here.
 */

/**
 * World heading for a world-space direction, or `undefined` if the delta has
 * no direction at all.
 *
 * The `undefined` return is load-bearing, not defensive: `useHexMovePath`
 * deliberately supports a degenerate single-point path (rpg-api#656's
 * same-hex "move"), whose segment delta is exactly zero. `Math.atan2(0, 0)`
 * returns 0, which would snap the character to due north for a move that
 * never went anywhere. `undefined` means "no opinion" and lets the caller
 * hold whatever heading it already had.
 */
export function headingFromDelta(dx: number, dz: number): number | undefined {
  if (dx === 0 && dz === 0) return undefined;
  return Math.atan2(dx, dz);
}

const TWO_PI = Math.PI * 2;

/**
 * Signed turn from `from` to `to`, wrapped to (-PI, PI] — i.e. always the
 * short way round. Without this, walking from a heading of 350 degrees to one
 * of 10 degrees would spin 340 degrees the wrong way.
 *
 * The two comparisons are ordered so an exact 180-degree reversal always
 * resolves to +PI regardless of the input's sign. A full reversal (walk east,
 * then west) is a common real path that sits exactly on the boundary where
 * both directions are equally short; leaving it to floating-point sign would
 * make the turn direction differ run to run for no reason.
 */
export function shortestTurn(from: number, to: number): number {
  let d = (to - from) % TWO_PI;
  if (d <= -Math.PI) d += TWO_PI;
  if (d > Math.PI) d -= TWO_PI;
  return d;
}

/**
 * Seconds-to-radians turn rate. Tuned against the hex geometry rather than by
 * eye: the six hex neighbours are exactly 60 degrees apart under
 * `cubeToWorld`, so an ordinary between-hex turn is 1.047 rad (~0.13s here)
 * and a full 180-degree reversal is 0.393s — both inside one
 * `SECONDS_PER_HEX_STEP` (0.45s), so a character finishes turning before it
 * finishes the step it is turning for.
 *
 * Playtest-tuning knob, exactly like `SECONDS_PER_HEX_STEP` is.
 */
export const TURN_RATE_RAD_PER_SEC = 8;

/**
 * Advance `current` toward `target` by at most `turnRate * delta`, never
 * overshooting. Returns `target` itself (===, not approximately) once within
 * one step, because `useEntityFacing` stops its invalidate loop on an exact
 * equality check.
 */
export function easeHeading(
  current: number,
  target: number,
  delta: number,
  turnRate: number = TURN_RATE_RAD_PER_SEC
): number {
  const turn = shortestTurn(current, target);
  if (turn === 0) return current;
  const maxStep = turnRate * delta;
  if (Math.abs(turn) <= maxStep) return target;
  return current + Math.sign(turn) * maxStep;
}
```

- [ ] **Step 4: Run the tests to verify they pass**

Run: `npx vitest run src/components/hex-grid/facing.test.ts`
Expected: PASS, all cases.

- [ ] **Step 5: Commit**

```bash
git add src/components/hex-grid/facing.ts src/components/hex-grid/facing.test.ts
git commit -m "feat(facing)#590: pure heading math (atan2 convention, shortest turn, easing)"
```

---

## Task 2: The heading owner (`useEntityFacing`)

**Files:**
- Create: `src/components/hex-grid/useEntityFacing.ts`
- Test: `src/components/hex-grid/useEntityFacing.test.ts`

- [ ] **Step 1: Write the failing tests**

Copy the mock block from `useHexMovePath.test.ts` verbatim — same `vi.hoisted` shape, same reason (no canvas).

```ts
import { act, renderHook } from '@testing-library/react';
import * as THREE from 'three';
import { beforeEach, describe, expect, it, vi } from 'vitest';

const hoisted = vi.hoisted(() => ({
  frameCallback: undefined as
    | ((state: unknown, delta: number) => void)
    | undefined,
  invalidate: vi.fn(),
}));

vi.mock('@react-three/fiber', () => ({
  useFrame: (cb: (state: unknown, delta: number) => void) => {
    hoisted.frameCallback = cb;
  },
  useThree: () => ({ invalidate: hoisted.invalidate }),
}));

import { useEntityFacing } from './useEntityFacing';

const tick = (delta: number) => act(() => hoisted.frameCallback?.({}, delta));

function setup(initialHeading: number) {
  const group = new THREE.Group();
  const ref = { current: group } as React.RefObject<THREE.Group | null>;
  const hook = renderHook(() => useEntityFacing(ref, initialHeading));
  return { group, hook };
}

beforeEach(() => {
  hoisted.invalidate.mockClear();
});

describe('useEntityFacing', () => {
  it('seeds the initial heading on mount as a snap, not an ease', () => {
    // A character should spawn already facing its staging direction rather
    // than spinning into it over the first few frames.
    const { group } = setup(Math.PI);
    expect(group.rotation.y).toBe(Math.PI);
  });

  it('does not invalidate while settled', () => {
    // frameloop="demand": a perpetual invalidate loop is a real cost. This
    // hook must go quiet once it has nothing to turn.
    const { group } = setup(0);
    hoisted.invalidate.mockClear();
    tick(0.016);
    tick(0.016);
    expect(group.rotation.y).toBe(0);
    expect(hoisted.invalidate).not.toHaveBeenCalled();
  });

  it('eases toward a requested heading over successive frames', () => {
    const { group, hook } = setup(0);
    act(() => hook.result.current.requestHeading(Math.PI / 2));
    tick(0.1); // 8 rad/s * 0.1s = 0.8 rad, short of PI/2 (1.5708)
    expect(group.rotation.y).toBeCloseTo(0.8, 6);
    expect(group.rotation.y).toBeLessThan(Math.PI / 2);
    tick(0.1);
    expect(group.rotation.y).toBeCloseTo(1.5708, 4);
  });

  it('settles exactly on the target and then stops invalidating', () => {
    const { group, hook } = setup(0);
    act(() => hook.result.current.requestHeading(0.5));
    tick(1); // one huge frame: well past the target
    expect(group.rotation.y).toBe(0.5);
    hoisted.invalidate.mockClear();
    tick(0.016);
    expect(hoisted.invalidate).not.toHaveBeenCalled();
  });

  it('takes the short way around the wrap boundary', () => {
    const { group, hook } = setup((350 * Math.PI) / 180);
    act(() => hook.result.current.requestHeading((10 * Math.PI) / 180));
    tick(0.01); // 0.08 rad of a 0.349 rad (+20 deg) turn
    // Must have increased past 350 deg, not swung back down toward 10.
    expect(group.rotation.y).toBeGreaterThan((350 * Math.PI) / 180);
  });

  it('invalidates when a heading is requested, to restart the loop', () => {
    const { hook } = setup(0);
    hoisted.invalidate.mockClear();
    act(() => hook.result.current.requestHeading(1));
    expect(hoisted.invalidate).toHaveBeenCalled();
  });
});
```

- [ ] **Step 2: Run to verify it fails**

Run: `npx vitest run src/components/hex-grid/useEntityFacing.test.ts`
Expected: FAIL — cannot resolve `./useEntityFacing`.

- [ ] **Step 3: Write the implementation**

```ts
/**
 * Owns one entity's world-space heading and eases it onto the group's
 * `rotation.y` (rpg-dnd5e-web#590).
 *
 * This is deliberately NOT part of `useHexMovePath`. #590 noted that facing
 * is not only about movement — an attack wants the attacker turned toward its
 * target, which has nothing to do with a move path — and then proposed two
 * shapes that both made the movement hook the owner of facing. That is
 * structurally the one place attack-facing can never come from. So the
 * movement hook only REPORTS its current segment heading, and this hook owns
 * the heading and accepts requests from any source. Movement is the only
 * source wired today; attack-facing is a second caller of `requestHeading`,
 * not a change to this file.
 *
 * No priority/arbitration between sources: last request wins. With one source
 * that rule is unfalsifiable, so building anything richer now would be
 * designing against an imagined conflict.
 *
 * Ownership contract, shared with `useHexMovePath`: that hook owns the
 * group's `.position` and ONLY `.position`; this hook owns `.rotation.y` and
 * ONLY `.rotation.y`. The two are disjoint, so both can drive the same object
 * every frame without fighting. As with position, the group must NOT also
 * carry a declarative `rotation` prop, or React would re-apply a stale value
 * on renders neither hook caused.
 *
 * Heading here is world-space only. The correction for a rig whose forward
 * axis is not +Z is a separate, composed term applied by the model component
 * itself (see MODEL_FORWARD_OFFSET in `facing.ts`).
 */
import { useFrame, useThree } from '@react-three/fiber';
import { useCallback, useLayoutEffect, useRef } from 'react';
import * as THREE from 'three';
import { easeHeading } from './facing';

export interface UseEntityFacingResult {
  /** Ask the entity to turn to a world-space heading (radians). Safe to call
   * from anywhere — an effect, an event handler, or a frame callback. */
  requestHeading: (radians: number) => void;
}

export function useEntityFacing(
  groupRef: React.RefObject<THREE.Group | null>,
  initialHeading: number
): UseEntityFacingResult {
  const { invalidate } = useThree();
  const currentRef = useRef(initialHeading);
  const targetRef = useRef(initialHeading);
  const seededRef = useRef(false);

  // Layout effect, not a plain effect: the seed must land before the first
  // paint or the entity renders one frame at heading 0 and visibly snaps.
  // Guarded to run once — re-seeding on a later render would yank a
  // mid-turn character back to its spawn pose.
  useLayoutEffect(() => {
    if (seededRef.current || !groupRef.current) return;
    seededRef.current = true;
    currentRef.current = initialHeading;
    targetRef.current = initialHeading;
    groupRef.current.rotation.y = initialHeading;
  }, [groupRef, initialHeading]);

  const requestHeading = useCallback(
    (radians: number) => {
      targetRef.current = radians;
      // Kick the demand-driven frameloop; the useFrame below will not run on
      // its own once things have settled.
      invalidate();
    },
    [invalidate]
  );

  useFrame((_state, delta) => {
    const group = groupRef.current;
    if (!group) return;
    // Exact equality is the settle condition, which is why easeHeading
    // returns `target` itself rather than something merely very close.
    if (currentRef.current === targetRef.current) return;
    currentRef.current = easeHeading(
      currentRef.current,
      targetRef.current,
      delta
    );
    group.rotation.y = currentRef.current;
    invalidate();
  });

  return { requestHeading };
}
```

- [ ] **Step 4: Run to verify it passes**

Run: `npx vitest run src/components/hex-grid/useEntityFacing.test.ts`
Expected: PASS, all cases.

- [ ] **Step 5: Commit**

```bash
git add src/components/hex-grid/useEntityFacing.ts src/components/hex-grid/useEntityFacing.test.ts
git commit -m "feat(facing)#590: useEntityFacing owns heading, eased, source-agnostic"
```

---

## Task 3: `useHexMovePath` reports its segment heading

**Files:**
- Modify: `src/components/hex-grid/useHexMovePath.ts`
- Modify: `src/components/hex-grid/useHexMovePath.test.ts`
- Modify: `src/components/hex-grid/HexEntity.tsx:244-250` (call site only — behaviour-neutral)

**Note:** this task includes a signature change (the hook takes a caller-owned group ref instead of creating and returning one). That is necessary, not cosmetic: `HexEntity` must hand the same ref to both hooks, and if the movement hook creates it, the two hooks are circular at the call site. There is exactly one caller.

- [ ] **Step 1: Write the failing tests** (append to `useHexMovePath.test.ts`)

Add `segmentHeading` to the existing `./useHexMovePath` import block — that block sits *after* the `vi.mock` call on purpose, so the hook picks up the mocked module. Do not move it.

```ts
describe('segmentHeading', () => {
  it('is the heading of the current leg', () => {
    // Due east in world space -> atan2(1, 0) = PI/2.
    const step: StepState = {
      points: [
        { x: 0, z: 0 },
        { x: 1, z: 0 },
      ],
      index: 0,
      elapsed: 0,
    };
    expect(segmentHeading(step)).toBeCloseTo(Math.PI / 2, 6);
  });

  it('is undefined once past the final point', () => {
    const step: StepState = {
      points: [
        { x: 0, z: 0 },
        { x: 1, z: 0 },
      ],
      index: 1,
      elapsed: 0,
    };
    expect(segmentHeading(step)).toBeUndefined();
  });

  it('is undefined for a zero-length leg', () => {
    // rpg-api#656's same-hex move: a real step that goes nowhere. No heading
    // opinion, so the character holds whatever it was facing.
    const step: StepState = {
      points: [
        { x: 2, z: 3 },
        { x: 2, z: 3 },
      ],
      index: 0,
      elapsed: 0,
    };
    expect(segmentHeading(step)).toBeUndefined();
  });
});

describe('useHexMovePath heading reporting', () => {
  it('reports a heading per segment, not per frame', () => {
    const onHeading = vi.fn();
    const group = new THREE.Group();
    const ref = { current: group } as React.RefObject<THREE.Group | null>;
    // Two-leg path: origin -> E -> E again.
    const path: CubeCoord[] = [
      { x: 0, y: 0, z: 0 },
      { x: 1, y: -1, z: 0 },
      { x: 2, y: -2, z: 0 },
    ];

    renderHook(() =>
      useHexMovePath({ x: 2, y: -2, z: 0 }, path, 1, HEX_SIZE, 0, ref, onHeading)
    );

    // One report for the first leg, emitted when the move starts.
    expect(onHeading).toHaveBeenCalledTimes(1);
    expect(onHeading.mock.calls[0][0]).toBeCloseTo(Math.PI / 2, 6);

    // Several frames inside the FIRST leg must not add reports.
    act(() => hoisted.frameCallback?.({}, SECONDS_PER_HEX_STEP / 3));
    act(() => hoisted.frameCallback?.({}, SECONDS_PER_HEX_STEP / 3));
    expect(onHeading).toHaveBeenCalledTimes(1);

    // Crossing into the second leg reports once more.
    act(() => hoisted.frameCallback?.({}, SECONDS_PER_HEX_STEP));
    expect(onHeading).toHaveBeenCalledTimes(2);
    expect(onHeading.mock.calls[1][0]).toBeCloseTo(Math.PI / 2, 6);
  });

  it('reports nothing for a non-genuine move', () => {
    const onHeading = vi.fn();
    const group = new THREE.Group();
    const ref = { current: group } as React.RefObject<THREE.Group | null>;
    // moveSeq undefined = initial mount / reconciliation, never an animation.
    renderHook(() =>
      useHexMovePath({ x: 0, y: 0, z: 0 }, undefined, undefined, HEX_SIZE, 0, ref, onHeading)
    );
    expect(onHeading).not.toHaveBeenCalled();
  });
});
```

- [ ] **Step 2: Run to verify it fails**

Run: `npx vitest run src/components/hex-grid/useHexMovePath.test.ts`
Expected: FAIL — `segmentHeading` is not exported; the hook signature does not accept a ref.

- [ ] **Step 3: Implement**

Add the import and the pure reporter to `useHexMovePath.ts`:

```ts
import { headingFromDelta } from './facing';

/**
 * The world heading of the leg currently being walked, or `undefined` when
 * there is no in-flight leg (already at the final point) or the leg has zero
 * length (rpg-api#656's same-hex move).
 *
 * Pure and stateless, like `computeMoveStart` and `advanceFrame` above, and
 * for the same reason — it is directly unit-testable with no R3F machinery.
 *
 * This hook REPORTS the heading; it does not own facing. See
 * `useEntityFacing.ts` for why that separation matters.
 */
export function segmentHeading(step: StepState): number | undefined {
  if (step.index >= step.points.length - 1) return undefined;
  const a = step.points[step.index];
  const b = step.points[step.index + 1];
  return headingFromDelta(b.x - a.x, b.z - a.z);
}
```

Change the hook signature and body:

```ts
export interface UseHexMovePathResult {
  /** True while stepping through a real move — pass straight through to
   * ClassCharacterModel to pick the walk clip over idle. */
  isMoving: boolean;
}

export function useHexMovePath(
  entityPosition: CubeCoord,
  movePath: CubeCoord[] | undefined,
  moveSeq: number | undefined,
  hexSize: number,
  yOffset: number,
  // Caller-owned as of #590: `useEntityFacing` drives `.rotation.y` on this
  // same object while this hook drives `.position`. If this hook created the
  // ref, the two would be circular at the call site.
  groupRef: React.RefObject<THREE.Group | null>,
  onHeading?: (radians: number) => void
): UseHexMovePathResult {
  // ...existing body, minus `const groupRef = useRef<THREE.Group>(null);`

  // Read through a ref so a caller passing an inline closure cannot re-run
  // the move effect below — its dependency list must track the MOVE, not the
  // identity of a callback.
  //
  // Assigned during render rather than in an effect, deliberately: the move
  // useLayoutEffect below reports the FIRST leg's heading, and a plain
  // useEffect assignment would not have run yet at that point, so the very
  // first leg of every move would report through a stale callback.
  const onHeadingRef = useRef(onHeading);
  onHeadingRef.current = onHeading;
```

In the `useLayoutEffect`, after `stepRef.current = {...}` and `setIsMoving(...)`:

```ts
    const startHeading = segmentHeading(stepRef.current);
    if (startHeading !== undefined) onHeadingRef.current?.(startHeading);
```

In the `useFrame`, inside `if (advanced.stepComplete)` after `s.index += 1; s.elapsed = 0;`:

```ts
      const nextHeading = segmentHeading(s);
      if (nextHeading !== undefined) onHeadingRef.current?.(nextHeading);
```

Return `{ isMoving }` only.

- [ ] **Step 4: Update the one call site** (`HexEntity.tsx`, currently lines 244-250)

```ts
  const movingGroupRef = useRef<THREE.Group>(null);
  const { isMoving } = useHexMovePath(
    position,
    movePath,
    moveSeq,
    hexSize,
    CHARACTER_Y_OFFSET,
    movingGroupRef
  );
```

Heading is not wired yet — that is Task 5. This step keeps the tree compiling and behaviour identical.

- [ ] **Step 5: Run the full suite**

Run: `npm run test:run`
Expected: PASS.

**Every pre-existing `useHexMovePath` behavioural assertion must still hold** — this task adds reporting, it does not alter movement. But the existing hook-level tests call the old 5-argument signature and read `result.current.groupRef`, so their call sites need the caller-owned ref threaded through and the `groupRef` read replaced with the locally-created one. That is a mechanical edit to the setup, not a change to what any test asserts. If you find yourself weakening an assertion to make it pass, stop — that means the refactor changed movement behaviour, which it must not.

- [ ] **Step 6: Commit**

```bash
git add src/components/hex-grid/useHexMovePath.ts src/components/hex-grid/useHexMovePath.test.ts src/components/hex-grid/HexEntity.tsx
git commit -m "feat(facing)#590: useHexMovePath reports segment heading; caller owns group ref"
```

---

## Task 4: Calibrate and split the constant

**This is the load-bearing task. Do not skip the measurement and infer the values instead.**

`Math.PI` today has two different terms fused into it — a model-forward correction and a staging pose. Splitting one `Math.PI` into two composed terms can silently cancel to `2*PI` (no rotation) or double. The split must be measured.

**Files:**
- Modify: `src/components/hex-grid/facing.ts` (add constants)
- Modify: `src/components/hex-grid/HexEntity.tsx:382,433`
- Modify (conditional): `src/character/creation/AppearanceSelectionModal.tsx:64`

- [ ] **Step 1: Capture the baseline, before changing anything**

Use the deterministic playtest harness (`src/components/playtest/SyntyShowcase.tsx` / `SyntyRoomDemo.tsx`) rather than a live encounter — no server, no seed variance, fixed camera. Capture a shot showing a player-class model and a monster.

```bash
node tools/browser/screenshot.mjs <harness-url> /tmp/facing/00-baseline.png
```

Keep the camera azimuth fixed and identical for every shot in this task.

- [ ] **Step 2: Measure each model family's forward axis**

Temporarily set the rendered rotation to 0 for each family in turn, capture, and record which world direction the rig actually points. Do this for **four** cases, not two:

1. `ClassCharacterModel` — standing class GLB
2. `ClassCharacterModel` — **downed variant**
3. `MediumHumanoid` — player variant
4. `MediumHumanoid` — monster variant (`variant="goblin"`)

**Do not assume the downed variants inherit the standing offset.** rpg-dnd5e-web#512 is exactly this failure mode: the downed GLBs shipped without the standing models' `Root` wrapper node (0.01 scale + Z-up→Y-up) and rendered invisible in production, and copy-based QA missed it twice. A variant that diverges on root-node convention is the same thing that changes an apparent forward axis. If the downed offset differs, it needs its own constant.

- [ ] **Step 3: Add the constants**

Append to `facing.ts`, with the measured values and a comment recording how they were measured:

```ts
/**
 * Per-model-family correction for a rig whose forward axis is not +Z. This is
 * a property of the ASSET, not of the entity using it — a monster rendered on
 * a Synty GLB needs the Synty offset, not a monster-flavoured one.
 *
 * MEASURED, not derived (rpg-dnd5e-web#590): before this split, one hardcoded
 * `Math.PI` carried both this correction and the staging default below, so the
 * two terms could not be recovered by reading the old constants — a naive
 * split cancels to 2*PI. Values captured from the playtest harness at a fixed
 * camera azimuth; see ideas/character-facing/plan.md Task 4.
 */
export const SYNTY_GLB_FORWARD_OFFSET = /* measured */ 0;
export const MEDIUM_HUMANOID_FORWARD_OFFSET = /* measured */ 0;

/**
 * Spawn pose. NOT a model correction — this is staging: players face up-board
 * and monsters face down-board so an encounter reads as two sides squaring
 * off. Preserved deliberately (Kirk, 2026-07-24); entities turn out of it as
 * soon as they move.
 */
export const DEFAULT_HEADING_BY_TYPE: Record<string, number> = {
  player: /* measured */ Math.PI,
  monster: /* measured */ 0,
};
```

- [ ] **Step 4: Replace the two hardcoded call sites**

`HexEntity.tsx:382` (`MediumHumanoid`) and `:433` (`ClassCharacterModel`) each take their family's constant. The `facingRotation` prop keeps its name but its documented meaning narrows to "correct this rig's forward axis"; update the prop doc comments in `ClassCharacterModel.tsx:65-67` and `MediumHumanoid.tsx:73`, which currently describe the old players-face-camera convention.

Note the `ErrorBoundary` fallback path is correct by construction once the offset travels with the model: `mediumHumanoidElement` is built with the `MediumHumanoid` constant and the class model with the Synty one, so a GLB that fails to load swaps the offset along with the model. This fires on any missing or unsynced GLB, so include it in Step 6.

- [ ] **Step 5: Check `AppearanceSelectionModal.tsx:64`**

It passes `facingRotation={0}`. If `MEDIUM_HUMANOID_FORWARD_OFFSET` measured nonzero, that character preview will now be turned relative to its own camera — pass the constant there too. If it measured zero, leave it and note why in the commit.

- [ ] **Step 6: The visual-parity gate**

Recapture the exact shot from Step 1, plus one per case in Step 2.

```bash
node tools/browser/screenshot.mjs <harness-url> /tmp/facing/01-after-split.png
```

**The board must look the same as the baseline.** This task changes how the rotation is *spelled*, not what it *is*. A visible difference here means the split cancelled or doubled — fix it before going near Task 5. Verify all four model cases plus the appearance modal.

- [ ] **Step 7: Run the gate and commit**

```bash
npm run ci-check
git add -A
git commit -m "refactor(facing)#590: split facingRotation into measured model offset + staging default"
```

---

## Task 5: Wire movement to facing

**Files:**
- Modify: `src/components/hex-grid/HexEntity.tsx`

- [ ] **Step 1: Wire the two hooks**

```ts
  const movingGroupRef = useRef<THREE.Group>(null);
  const { requestHeading } = useEntityFacing(
    movingGroupRef,
    DEFAULT_HEADING_BY_TYPE[type] ?? 0
  );
  const { isMoving } = useHexMovePath(
    position,
    movePath,
    moveSeq,
    hexSize,
    CHARACTER_Y_OFFSET,
    movingGroupRef,
    requestHeading
  );
```

`type` is `'player' | 'monster' | 'obstacle'`; obstacles fall through to `0` and never move, so the `?? 0` is the honest default rather than a guard.

- [ ] **Step 2: Confirm the outer group has no declarative rotation**

`HexEntity.tsx:408` is `<group ref={movingGroupRef} {...interactionProps}>` — no `rotation` prop, which is required. Do not add one. The dead/downed tilt group at `:414` stays where it is: nesting Z-tilt inside Y-heading is what makes a corpse tilt relative to its own facing rather than to the world.

- [ ] **Step 3: Run the full suite**

Run: `npm run test:run`
Expected: PASS.

- [ ] **Step 4: Capture movement evidence**

Screenshots (or a short capture) of a character walking each of the six hex directions, confirming it is turned correctly for each leg and eases rather than pops between legs. Also confirm idle holds the last heading rather than snapping back on move completion.

- [ ] **Step 5: Gate and commit**

```bash
npm run ci-check
git add -A
git commit -m "feat(facing)#590: characters face their direction of travel"
```

---

## Task 6: PR

- [ ] **Step 1: Push and open the PR**

Body must include: the before/after parity evidence from Task 4 Step 6, the six-direction evidence from Task 5 Step 4, the measured constant values with how they were measured, and `Closes #590`.

Note in the PR that the pacing/skating gap is **not** fixed here and is now more visible: hex steps translate at ~3.85 m/s (`SECONDS_PER_HEX_STEP` 0.45, √3 world units between adjacent hex centres) against a `Walk_Forward` stride authored for ~1.55 m/s. It is a tuning constant and gets its own issue.

End the PR body and any issue comments with:

```
— asset-pipeline agent, on behalf of KirkDiggler
```

- [ ] **Step 2: Check for review comments**

```bash
gh pr view <PR> --comments
gh api repos/KirkDiggler/rpg-dnd5e-web/pulls/<PR>/comments
```

---

## Out of scope — do not build these

- **Attack-facing.** The seam is `requestHeading`. Nothing calls it but movement.
- **Spawn-facing toward the opposing side.** Staging stays a fixed per-type default.
- **Heading priority/arbitration.** Last request wins.
- **The pacing/skating fix.** Separate issue.
- **Any proto change.** Facing is client-owned; 5e has no facing rules.
