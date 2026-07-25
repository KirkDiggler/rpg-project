# Character Facing — heading as a first-class concept (v1)

## Status: Design — approved in session by Kirk 2026-07-24. Closes the design half of rpg-dnd5e-web#590; implementation plan follows on this PR.

North star: **a character faces where it is going, and facing is a concept the game owns rather than a constant three unrelated ideas are hiding inside.**

Driving acceptance case: a character walks a multi-hex path in any of the six hex
directions and is turned the right way for every leg of it, easing between legs
rather than popping — and, with movement disabled, the board still looks exactly
like it does today.

v1 wires exactly one heading source (movement). Attack-facing and
spawn-facing-toward-enemies get a seam, not an implementation.

## Context — what exists today (verified 2026-07-24 against `origin/main`, all repos current)

- `facingRotation` is a hardcoded constant at both call sites: `HexEntity.tsx:382`
  (`type === 'player' ? Math.PI : 0`, the `MediumHumanoid` branch) and
  `HexEntity.tsx:433` (`Math.PI`, the `ClassCharacterModel` branch). It is applied
  as a Y-rotation in `ClassCharacterModel.tsx:198` and `MediumHumanoid.tsx:582`.
  Nothing derives it from anything. Characters never turn.
- `useHexMovePath` already interpolates world-space `points` and tracks
  `stepRef = {points, index, elapsed}`, and already mutates the group's position
  every frame in `useFrame`. The heading for the current leg is
  `points[index + 1] - points[index]` — the data is present and unused.
- The hook's pure core (`computeMoveStart`, `advanceFrame`) is deliberately split
  out with no `@react-three/fiber` imports so `useHexMovePath.test.ts` can drive it
  without a canvas. That file's `vi.hoisted` + mocked-`useFrame` harness (the
  rpg-dnd5e-web#551 pattern) captures the frame callback and ticks it manually.
- **There is no `facing` or `heading` field anywhere in the encounter protos.** The
  only near-hit is `hex_orientation` (pointy-top vs flat-top; unrelated).
- The Canvas runs `frameloop="demand"`. Position is driven through refs
  specifically to avoid re-rendering per frame; `useHexMovePath`'s caller contract
  explicitly forbids also passing a declarative `position` prop on the same object,
  because React would re-apply a stale value on renders the hook did not cause.

### Correction to rpg-dnd5e-web#590's premise: the camera orbits

The issue describes `Math.PI` as a camera-relative convention ("the Synty models'
forward axis faces the camera at rest") to be composed with, not replaced. It is not
camera-relative. `useCameraControls.ts:64` starts azimuth at `Math.PI / 4` and
derives the camera position from it spherically (`:83-89`); `:150` rotates it on
drag; `:231,236,278,282` rotate it on Q/E. `Math.PI` is a fixed **world** rotation —
already ~45° off from pointing at the camera at the default azimuth, and off by an
arbitrary amount after one keypress.

There is therefore no camera-facing invariant to preserve. This makes the design
simpler than the issue assumes, not harder.

## Decisions (resolved in the 2026-07-24 brainstorm)

1. **Facing is client-owned; no proto change.** 5e has no facing rules, so this is
   pure presentation. Every client derives the same heading from the same
   `EntityMoved.actualPath`, so clients agree without syncing anything. The only
   divergence is a client joining mid-encounter with no move history — cosmetic, and
   it self-heals on that entity's next move.
2. **Imperative, not declarative.** The issue's option `(2)` (hook returns a heading,
   `HexEntity` passes it down) would `setState` once per hex step, adding a React
   re-render per hex to a tree built on refs precisely to avoid that. Imperative also
   makes "idle holds the last heading" free — nothing resets it — where the
   declarative form has to actively remember it.
3. **The movement hook reports its heading; it does not own facing.** #590 correctly
   notes "facing is not only about movement — attacking wants the attacker turned
   toward the target," then proposes two shapes that both make the movement hook the
   owner. That is structurally the one place attack-facing can never come from. A
   separate owner accepts heading *requests* from any source.
4. **No priority system. Last request wins.** v1 has one source, so precedence is
   unfalsifiable design. It gets built when a second source exists and the conflict
   is real.
5. **The single constant is split into two named terms.** Model-forward offset (an
   asset property) and staging default (a spawn pose) are different things that
   happen to both equal `Math.PI` for players today.
6. **Today's staging is preserved** (Kirk): players face up-board, monsters face
   down-board. Monsters get facing in this slice — they already flow through
   `useHexMovePath`, so it is nearly free, and a goblin walking backwards is as
   visible as a fighter doing it.

## Components

### `src/components/hex-grid/facing.ts` — pure math (new)

No `@react-three/fiber` imports, matching the `computeMoveStart`/`advanceFrame`
precedent that makes the movement hook testable without a canvas.

```ts
/** World heading (radians) for a world-space delta, or undefined if degenerate. */
export function headingFromDelta(dx: number, dz: number): number | undefined;

/** Signed turn from `from` to `to`, wrapped to (-PI, PI] — never the long way. */
export function shortestTurn(from: number, to: number): number;

/** Step `current` toward `target` by at most turnRate*delta; never overshoots. */
export function easeHeading(
  current: number, target: number, delta: number, turnRate: number
): number;

export const TURN_RATE_RAD_PER_SEC = 8;
```

`headingFromDelta` is `Math.atan2(dx, dz)`. A Three.js object at `rotation.y = θ`
maps its local **+Z** to world `(sin θ, 0, cos θ)`, so `atan2(dx, dz)` is that
function's exact inverse. If a given rig's forward is **−Z** rather than +Z, that
discrepancy *is* the per-model offset — which is exactly why the offset stays a
separate composed term instead of being folded in here.

A zero-length delta returns `undefined` rather than `atan2(0, 0)`'s misleading `0`.
This matters: `useHexMovePath` deliberately supports a degenerate single-point path
(the rpg-api#656 same-hex "move"), and `undefined` makes that a no-request — the
character holds its heading instead of snapping to due-north.

**Turn rate.** With this `cubeToWorld`, the six `HEX_DIRECTIONS` produce world
headings 60° apart — E 90°, NE 150°, NW 210°, W 270°, SW 330°, SE 30°. So at 8 rad/s
an ordinary between-hex turn (60°, 1.047 rad) resolves in ~0.13s and a full 180°
reversal in ~0.39s, both inside one `SECONDS_PER_HEX_STEP` (0.45s). This is a
playtest-tuning knob and is flagged in code as one, exactly like
`SECONDS_PER_HEX_STEP` is.

### `src/components/hex-grid/useEntityFacing.ts` — the heading owner (new)

```ts
export function useEntityFacing(
  groupRef: React.RefObject<THREE.Group | null>,
  initialHeading: number
): { requestHeading: (radians: number) => void };
```

- Holds `current` and `target` in refs. **No React state** — no re-renders.
- Seeds `rotation.y = initialHeading` in a `useLayoutEffect` guarded to mount only,
  as a **snap, not an ease**: a character should spawn facing its staging direction,
  not spin into it on the first frame. Layout-effect timing (not `useEffect`) so
  there is no one-frame flash at heading 0.
- `useFrame` eases `current` toward `target` and writes `rotation.y`, calling
  `invalidate()` only while actually turning. **Once settled it stops invalidating** —
  under `frameloop="demand"` a perpetual invalidate loop is a real cost, and this
  file follows the same discipline `ClassCharacterModel` uses (its heartbeat
  self-sustains only while a clip is playing).
- `requestHeading(r)` sets `target` and calls `invalidate()` to restart the loop.

This is the seam. Attack-facing later is a second caller of `requestHeading`, not a
change to any of this.

### `src/components/hex-grid/useHexMovePath.ts` — reports, does not own (changed)

- New pure export `segmentHeading(step: StepState): number | undefined` — the heading
  of `points[index] → points[index+1]`, or `undefined` when there is no in-flight leg
  or the leg is zero-length. Testable directly, like its two siblings.
- New optional `onHeading?: (radians: number) => void`, invoked **on move start and
  on each segment advance — never per frame.**
- **Signature change:** the hook accepts the group ref rather than creating and
  returning it. Two peer hooks now write to one object, so the caller owning it is
  the honest shape; keeping creation inside the movement hook would make the two
  hooks circular at the call site. This is a targeted change to code being worked in,
  with one caller.

### `HexEntity.tsx` — wiring (changed)

```ts
const movingGroupRef = useRef<THREE.Group>(null);
const { requestHeading } = useEntityFacing(movingGroupRef, DEFAULT_HEADING_BY_TYPE[type]);
const { isMoving } = useHexMovePath(/* …, */ movingGroupRef, requestHeading);
```

**Ownership split, to be documented in both hooks:** the movement hook owns
`.position` and only `.position`; the facing hook owns `.rotation.y` and only
`.rotation.y`. Disjoint, so they cannot fight.

The outer group carries **no declarative `rotation` prop** — the same rule its
`position` already follows, and for the same reason. The inner model keeps its
declarative `rotation={[0, modelForwardOffset, 0]}`; that is a constant, so React
re-applying it on any render is harmless.

The dead/downed tilt group (`HexEntity.tsx:414`) nests underneath and composes
correctly: Y-heading outside, Z-tilt inside, so a corpse tilts relative to its own
facing rather than to the world.

### Splitting the constant

- **Model-forward offset** — named per-family constants
  (`SYNTY_GLB_FORWARD_OFFSET`, `MEDIUM_HUMANOID_FORWARD_OFFSET`), passed as the
  narrowed `facingRotation` prop, which now means only "correct this rig's forward
  axis."
- **Staging default** — `DEFAULT_HEADING_BY_TYPE`, seeded as `initialHeading`.
  Preserves today's players-up-board / monsters-down-board look.

## The calibration step — the load-bearing risk

`Math.PI` currently has both terms fused into it. **The split cannot be derived by
reading the current constants**; naively splitting one `Math.PI` into two composed
terms can silently cancel to `2π` (no rotation) or double. It has to be measured:

1. Temporarily render each model family at offset 0 and heading 0.
2. Screenshot from a known, fixed camera azimuth via `tools/browser/screenshot.mjs`.
3. Read off which world direction each rig's forward actually points.
4. Derive `*_FORWARD_OFFSET` per family, then derive `DEFAULT_HEADING_BY_TYPE` to
   reproduce today's on-screen look exactly.
5. **Visual-parity gate: with movement disabled, the board must look like it does
   today.** Only then wire the movement source.

Step 5 before any behavior change is what keeps this from shipping everyone
backwards. This is the QA-evidence half of the work, not a footnote to it.

## Data flow

```
EntityMoved.actualPath
  → useHexMovePath: builds world points, advances index in useFrame
  → segmentHeading(step)                       [pure, per segment change]
  → onHeading(radians)
  → useEntityFacing.requestHeading             [sets target, kicks invalidate]
  → useFrame: easeHeading(current → target)    [pure]
  → group.rotation.y                           [world heading]
      ∘ model's declarative facingRotation     [rig forward-axis correction]
```

Heading persists after a move completes (`current === target`, nothing resets it),
so idle holds the last heading. A superseded or non-genuine move clears the movement
hook's step state and snaps position; facing simply holds — cosmetically correct.

## Testing

- **`facing.test.ts`** (pure, no canvas): `headingFromDelta` against all six
  `HEX_DIRECTIONS` (30/90/150/210/270/330°) and `undefined` on a zero delta;
  `shortestTurn` wrap — 350°→10° must turn **+20°, not −340°**; `easeHeading` clamps
  at the target without overshoot, and is a no-op when already there.
- **`useEntityFacing.test.ts`** (existing `vi.hoisted` mocked-`useFrame` harness):
  seeds the initial heading on mount without easing; eases toward a requested
  heading over ticks; takes the short way around the wrap; **stops calling
  `invalidate` once settled.**
- **`useHexMovePath.test.ts`** (extended): `segmentHeading` unit cases including the
  degenerate single-point path; hook-level assertion that `onHeading` fires **once
  per segment, not per frame**, with the correct heading per leg.
- **Visual evidence:** the parity gate above, then screenshots of a character walking
  each of the six hex directions.

## Not in v1 (reserved seats, deliberately absent)

- **Attack-facing** — a second `requestHeading` caller. The seam exists; nothing calls it.
- **Spawn-facing toward the opposing side** — staging stays the fixed per-type default.
- **Heading priority/arbitration** — last request wins until a real conflict exists.
- **Pacing / skating** — hex steps translate at ~3.85 m/s (`SECONDS_PER_HEX_STEP`
  0.45, `HEX_SIZE` 1.0, √3 world units between adjacent centers) while
  `Walk_Forward`'s stride is authored for ~1.55 m/s. A tuning constant, not an asset
  defect, and its own issue. **Correct facing will make this more visible, not less**
  — a legible walk cycle pointed the right way and still sliding.
- **Any proto change.**

## Risks

1. **Offset/staging cancellation** — the fused-`Math.PI` hazard above. Mitigated by
   the calibration procedure and the visual-parity gate.
2. **`AppearanceSelectionModal.tsx:64`** passes `facingRotation={0}`. If calibration
   finds `MediumHumanoid` needs a nonzero offset, that character-preview turns
   sideways unless it receives the same constant. Explicit check, not an assumption.
3. **Two hooks, one object** — mitigated by the disjoint-property split, documented
   in both hooks.
4. **Monsters on `MediumHumanoid`, players on Synty GLBs.** The current
   `type === 'player' ? Math.PI : 0` conflates entity type with model family. Once a
   monster renders as a Synty GLB it needs the Synty offset, not the monster staging
   default. The split fixes this by construction; the test matrix should still cover
   a monster on each model family.

## Open questions

None blocking. Both calls raised in the brainstorm were resolved by Kirk: keep
today's staging, and include monsters in this slice.
