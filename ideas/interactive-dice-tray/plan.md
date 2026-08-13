# Interactive Collectible 3D Dice Tray Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development to implement this plan task-by-task. Each task is a visible vertical slice and ends with independent review plus a browser checkpoint.

**Goal:** Grow the existing lightning-d20 proof into a production-intent, Concepts Lab–only 3D dice tray through visible increments: empty tray, one placed die, explicit Roll, grab/release, collectible presets, roller/spectator witnessing, then complete authoritative 1–20 settlement.

**Architecture:** `DiceTray3DShell` owns the rounded tray and overflow-capable presentation surface. `DiceTray3D` adds accessible interaction and composes `AttackDie3D`, which remains the authoritative-result visual renderer. An allowlisted preset registry separates collectible identity from model/material implementation, and a compact presentation-only release value coordinates roller/spectator concept panes without streaming pointer movement.

**Tech Stack:** React 19, TypeScript 5.8, Three.js / React Three Fiber, Vitest, Testing Library, Playwright/Chromium, Vite Concepts Lab.

**Spec:** `rpg-project/ideas/interactive-dice-tray/design.md`

## Global Constraints

- Work in `/home/kirk/game-dev/rpg-dnd5e-web/.worktrees/attack-die-749` on `feat/749-attack-die-3d-concept`; preserve its current uncommitted, user-approved trajectory, camera, result-10 pose, and sizing work.
- Track implementation through `rpg-dnd5e-web#749` / PR `#750`; reference `rpg-project#219`, design PR `rpg-project#220`, `rpg-game-assets#47`, and `rpg-game-assets#49` in final handoff.
- Concepts Lab route remains `http://127.0.0.1:3002/?concept=attack-die-3d`.
- Every task must leave a visible, browser-reviewable state. After task review passes, capture a private screenshot under `/home/kirk/game-dev/.verification/interactive-dice-tray/task-N/` and report the path before moving on.
- Do not wire `CombatPresentation`, `EncounterView`, the production FIFO, or `useBeatSequencer`. Do not remove the current production 1.5-second auto-throw timeout in this concept plan; removing it is a production-promotion gate.
- Do not change toolkit, API, proto, deployment, account/profile, inventory, ownership, purchasing, loadout, or persistence code.
- Prototype one d20 only. Keep the tray boundary list-shaped for later groups, but never render or fabricate damage dice from `EntityDamaged.amount`.
- Player mode waits indefinitely for Roll or grab/release. Monster mode may auto-release. Spectator panes expose no roll input.
- Gesture and replay data are decorative only. `authoritativeResult` alone selects the target face; no client path generates, changes, rerolls, biases, clamps, or interprets it.
- Do not stream pointer movement. Roller and spectator receive one compact, sanitized release value in the Concepts Lab simulation.
- Presets are stable collectible die identities, not `player`/`monster` booleans. Spectators render the roller's selected preset.
- Only allowlisted preset/model contracts may resolve. No arbitrary asset URL may enter through props or release data.
- A physical 3D result must be mapped to the exact inspected GLB. Unknown preset, digest mismatch, unmapped result, invalid result, load/shader/context failure, or invalid tray cardinality fails closed to the existing semantic SVG surface.
- Known lightning GLB SHA-256: `8a8e50995ee790481e6d1f4b58919f1acee169398acd783af28376464160c1aa`. Any other digest invalidates the provisional face map.
- Licensed GLBs, source assets, calibration contact sheets, screenshots, and GIFs stay private/untracked. Commit only code, tests, public-safe docs, and numeric contract data.
- Preserve the renderer lifecycle and settlement contract: quaternion error `<= 0.25°`, then copy and hold the exact target.
- Reduced motion retains explicit player input and exact settlement while suppressing tumble/effect animation.
- Test-first for every behavior change: observe RED for the intended reason, implement minimally, then rerun focused and protected tests.
- Use one writer subagent at a time. A fresh reviewer gates every task before the next writer starts.
- Never stage `.pi/`, private evidence, or unrelated changes. Kirk alone merges.

## Visible Delivery Order

| Task | Browser-visible checkpoint |
|---|---|
| 0 | Existing approved roll, now smaller at scale 1.1 |
| 1 | Empty rounded tray/drawer in a new Tray stage |
| 2 | One result-10 lightning d20 placed and settling inside the tray |
| 3 | Player waits for explicit Roll; monster can auto-roll |
| 4 | Optional grab, shake, and release |
| 5 | Stormforged and Cryptstone collectible presets |
| 6 | Roller and spectator panes sharing one release |
| 7 | Authoritative input 1–20 with complete provisional physical settlement |
| 8 | Final browser matrix, private GIF, documentation, and independent branch review |

---

### Task 0: Checkpoint the Approved Roll and Try Scale 1.1

**Files:**
- Modify: `src/components/ui/dice/attackDieVisualConfig.test.ts`
- Modify: `src/components/ui/dice/attackDieVisualConfig.ts`
- Modify: `src/concepts/attack-die-3d/attackDieExperiment.test.ts`
- Checkpoint all currently modified approved prototype files shown by `git status --short`

**Produces:** committed baseline with the current right-to-left roll, 440×360 surface, 60° three-quarter camera, result-10 top face, and `ATTACK_DIE_VISUAL_CONFIG.dieScale === 1.1`.

- [ ] **Step 1: Verify and record the dirty baseline**

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

Expected: the current 11 tracked modifications remain; 48 focused tests pass; static checks exit zero. Do not reset, stash, or create a replacement worktree.

- [ ] **Step 2: Write the failing scale assertions**

```ts
expect(ATTACK_DIE_VISUAL_CONFIG.dieScale).toBe(1.1);
```

and in the experiment defaults:

```ts
dieScale: 1.1,
```

- [ ] **Step 3: Verify RED**

```bash
npm run test:run -- \
  src/components/ui/dice/attackDieVisualConfig.test.ts \
  src/concepts/attack-die-3d/attackDieExperiment.test.ts
```

Expected: FAIL because implementation still reports `1.4`.

- [ ] **Step 4: Apply only the approved scale experiment**

```ts
dieScale: 1.1,
```

Do not alter camera, viewport, trajectory, or result-10 quaternion.

- [ ] **Step 5: Verify GREEN, capture the browser state, and commit**

Repeat Step 1. Capture result 10 settled at the real route to:

```text
/home/kirk/game-dev/.verification/interactive-dice-tray/task-0/result-10-scale-1.1.png
```

Confirm the complete die is visible. Then:

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

Expected: `.pi/` stays untracked and unstaged.

---

### Task 1: Show an Empty Tray/Drawer First

**Files:**
- Create: `src/components/ui/dice/DiceTray3DShell.tsx`
- Create: `src/components/ui/dice/DiceTray3DShell.test.tsx`
- Create: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx`
- Create: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.tsx`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx`
- Modify: `public/themes/base.css`

**Produces:** a reusable rounded tray shell and fifth Concepts Lab tab that visibly renders an empty, non-interactive tray.

```ts
export interface DiceTray3DShellProps {
  label: string;
  phase: 'empty' | 'armed' | 'rolling' | 'settled';
  children?: React.ReactNode;
  controls?: React.ReactNode;
  className?: string;
}
```

- [ ] **Step 1: Write shell and concept tests**

Assert:

```ts
render(<DiceTray3DShell label="Player attack tray" phase="empty" />);
expect(screen.getByRole('region', { name: 'Player attack tray' })).toBeTruthy();
expect(screen.getByText('Your d20 will appear here')).toBeTruthy();
```

With children, assert the empty copy disappears and children render inside `data-testid="dice-tray-3d-motion-surface"`, while `data-testid="dice-tray-3d-well"` remains a distinct rounded boundary.

In `AttackDie3DConcept`, assert five keyboard-operable tabs:

```ts
['Appearance', 'Calibrate', 'Roll', 'Verify', 'Tray']
```

and Tray renders copy containing `Empty tray checkpoint` and `No interaction yet`.

- [ ] **Step 2: Verify RED**

```bash
npm run test:run -- \
  src/components/ui/dice/DiceTray3DShell.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx
```

Expected: FAIL because shell/panel/Tray stage do not exist.

- [ ] **Step 3: Implement the shell and empty panel**

The shell renders:

```tsx
<section
  role="region"
  aria-label={label}
  className="dice-tray-3d-shell"
  data-phase={phase}
>
  <div className="dice-tray-3d-shell__well" data-testid="dice-tray-3d-well" />
  <div
    className="dice-tray-3d-shell__motion-surface"
    data-testid="dice-tray-3d-motion-surface"
  >
    {children ?? <p>Your d20 will appear here</p>}
  </div>
  {controls && <div className="dice-tray-3d-shell__controls">{controls}</div>}
</section>
```

Add `Tray` to the existing tab array and keep arrow-key wrapping correct across five tabs. The panel passes no child yet and labels the checkpoint honestly.

- [ ] **Step 4: Add the tray geometry CSS**

- motion surface: `width: min(100%, 440px); height: 360px; position: relative; overflow: visible`;
- well: centered rounded rectangle, visibly distinct from the outer review surface;
- the well must not clip future throw motion;
- responsive width remains readable below 700px;
- visible focus styles are preserved for future controls.

- [ ] **Step 5: Verify GREEN and capture the empty tray**

```bash
npm run test:run -- \
  src/components/ui/dice/DiceTray3DShell.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx
npm run typecheck
npm run lint -- --quiet
git diff --check
```

Capture:

```text
/home/kirk/game-dev/.verification/interactive-dice-tray/task-1/empty-tray.png
```

- [ ] **Step 6: Commit**

```bash
git add \
  src/components/ui/dice/DiceTray3DShell.tsx \
  src/components/ui/dice/DiceTray3DShell.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx \
  public/themes/base.css
git commit -m "feat: add empty 3d dice tray concept (#749)"
```

---

### Task 2: Place One Working Result-10 d20 in the Tray

**Files:**
- Create: `src/components/ui/dice/DiceTray3D.tsx`
- Create: `src/components/ui/dice/DiceTray3D.test.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx`
- Modify: `public/themes/base.css`

**Produces:** one list-shaped tray item rendered through the real `AttackDie3D`, fixed to the currently approved result-10 visual proof. No Roll or grab interaction yet.

```ts
export interface DiceTray3DItem {
  id: string;
  kind: 'd20';
  presetId: string;
  authoritativeResult: number;
  presentationToken: number;
}

export interface DiceTray3DProps {
  label: string;
  phase: 'rolling' | 'settled';
  dice: readonly DiceTray3DItem[];
  reducedMotion?: boolean;
  sceneOverride?: AttackDie3DProps['sceneOverride'];
  sidecarOverride?: AttackDie3DProps['sidecarOverride'];
  calibrationPose?: QuaternionTuple;
}
```

- [ ] **Step 1: Write failing composition/cardinality tests**

Mock `AttackDie3D`. Assert exactly one d20 passes `authoritativeResult`, token, and overrides unchanged. Assert empty, two-item, non-d20, and result outside 1–20 render semantic fallback copy and never mount `AttackDie3D`.

- [ ] **Step 2: Verify RED**

```bash
npm run test:run -- src/components/ui/dice/DiceTray3D.test.tsx
```

Expected: FAIL because `DiceTray3D` does not exist.

- [ ] **Step 3: Implement placement-only composition**

Compose `AttackDie3D` inside `DiceTray3DShell`. For this task only, the concept supplies:

```ts
{
  id: 'attack',
  kind: 'd20',
  presetId: 'lightning',
  authoritativeResult: 10,
  presentationToken: token,
}
```

Use reduced motion for the initial settled placement and the existing geometry-derived result-10 pose. Label the panel `Placement checkpoint · result 10 only · no interaction yet`.

- [ ] **Step 4: Write and implement containment styling**

Add `data-testid="dice-tray-3d-renderer"`. At settled result 10, the projected die bounds must be fully inside `dice-tray-3d-well`; the larger 440×360 motion surface remains available for later overflow travel. Do not hide overflow.

- [ ] **Step 5: Verify GREEN and capture**

```bash
npm run test:run -- \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx
npm run typecheck
npm run lint -- --quiet
```

Browser-measure both bounding rectangles and assert the settled die's visible bounds are within the well. Capture:

```text
/home/kirk/game-dev/.verification/interactive-dice-tray/task-2/result-10-in-tray.png
```

- [ ] **Step 6: Commit**

```bash
git add \
  src/components/ui/dice/DiceTray3D.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  public/themes/base.css
git commit -m "feat: place lightning d20 in the tray (#749)"
```

---

### Task 3: Add Explicit Roll and Monster Auto-Roll

**Files:**
- Create: `src/components/ui/dice/dicePresentationRelease.ts`
- Create: `src/components/ui/dice/dicePresentationRelease.test.ts`
- Modify: `src/components/ui/dice/attackDieMotion.ts`
- Modify: `src/components/ui/dice/attackDieMotion.test.ts`
- Modify: `src/components/ui/dice/AttackDie3D.tsx`
- Modify: `src/components/ui/dice/AttackDie3D.test.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.test.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx`

**Produces:** armed player tray that never auto-rolls, accessible Roll button, monster auto-roll, and phase-aware renderer that hides the result while armed.

```ts
export interface AttackDieDecorativeRelease {
  variation: number;
  vector: readonly [number, number];
  shake: number;
}

export interface DicePresentationRelease extends AttackDieDecorativeRelease {
  schemaVersion: 1;
  presentationId: string;
  presetId: string;
}
```

`DiceTray3DProps` gains:

```ts
rollerRole: 'player' | 'monster';
witnessRole: 'roller' | 'spectator';
phase: 'armed' | 'rolling' | 'settled';
release?: DicePresentationRelease;
onReleaseRequest?: (value: DicePresentationRelease) => void;
```

- [ ] **Step 1: Write release-contract tests**

`createDicePresentationRelease` must:

- reject blank presentation ID and non-allowlisted preset;
- normalize finite variation to `Math.abs(Math.trunc(value)) % 997`;
- use `[0, 0]` and shake `0` for button release;
- return a frozen object;
- serialize with none of `result`, `hit`, `damage`, `target`, or URL;
- key releases as `${presentationId}:${variation}`.

- [ ] **Step 2: Verify RED and implement the compact button release**

```bash
npm run test:run -- src/components/ui/dice/dicePresentationRelease.test.ts
```

Implement the minimal deterministic constructor; rerun to GREEN.

- [ ] **Step 3: Write phase-aware renderer tests**

Assert:

- `phase="ready"` uses the same neutral quaternion for authoritative results 1 and 20 and emits no observation;
- `phase="rolling"` resets elapsed roll once and follows the existing trajectory;
- `phase="settled"` copies exact target/resting position immediately;
- reduced motion stays neutral while armed, then settles exactly when released.

- [ ] **Step 4: Verify RED and implement phase-aware motion**

Add:

```ts
export function attackDiePoseForPhase(input: {
  phase: DiceTrayPhase;
  elapsedMs: number;
  reducedMotion: boolean;
  current: QuaternionTuple;
  target: QuaternionTuple;
  release?: AttackDieDecorativeRelease;
}): AttackDieMotionFrame & { translation: AttackDieTranslation };
```

Semantics:

- `entering`/`ready`: fixed neutral pose at center, no target reveal;
- `rolling`: current tumble/convergence;
- `settled`/`exiting`: exact target at left resting position;
- `hidden`: renderer not mounted.

Wire `phase` and `decorativeRelease` through `AttackDie3D` without changing renderer-lock/token lifecycle.

- [ ] **Step 5: Write player/monster/spectator tests**

Assert:

1. player+roller+armed shows `Roll d20`;
2. advancing fake timers by one hour emits no release;
3. Roll emits exactly once;
4. monster+roller+armed emits one automatic release in an effect;
5. spectator never renders controls and never emits;
6. supplied result remains unchanged through release.

- [ ] **Step 6: Implement the interaction state**

Use one commit ref keyed by `presentationId`. Button and monster effect call the same one-shot release function. The concept controls phase: Arm → shared release → rolling → matching observed telemetry → settled. Keep this single-pane at this task; spectator pairing comes in Task 6.

- [ ] **Step 7: Verify, capture, and commit**

```bash
npm run test:run -- \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/attackDieMotion.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx
npm run typecheck
npm run lint -- --quiet
```

Capture armed and rolling states under `task-3/`. Then:

```bash
git add \
  src/components/ui/dice/dicePresentationRelease.ts \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/attackDieMotion.ts \
  src/components/ui/dice/attackDieMotion.test.ts \
  src/components/ui/dice/AttackDie3D.tsx \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/DiceTray3D.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx
git commit -m "feat: add explicit player dice roll (#749)"
```

---

### Task 4: Add Optional Grab, Shake, and Release

**Files:**
- Modify: `src/components/ui/dice/dicePresentationRelease.ts`
- Modify: `src/components/ui/dice/dicePresentationRelease.test.ts`
- Modify: `src/components/ui/dice/DiceTray3D.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.test.tsx`
- Modify: `src/components/ui/dice/attackDieMotion.ts`
- Modify: `src/components/ui/dice/attackDieMotion.test.ts`
- Modify: `public/themes/base.css`

**Produces:** optional pointer/touch ritual that emits one compact release; pointer movement stays local and affects only decorative motion.

```ts
export interface DiceGestureSample {
  origin: readonly [number, number];
  current: readonly [number, number];
  distance: number;
}
```

Quantization:

```ts
vector: [clamp(dx / 160, -1, 1), clamp(dy / 160, -1, 1)]
shake: clamp(distance / 240, 0, 1)
```

- [ ] **Step 1: Write quantization and authority tests**

Assert finite clamping, absent gesture defaults, deterministic equality, and no outcome fields. Two releases may change mid-roll translation/quaternion but must finish at identical target/resting pose.

- [ ] **Step 2: Verify RED and implement quantization**

```bash
npm run test:run -- \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/attackDieMotion.test.ts
```

Implement, then rerun to GREEN.

- [ ] **Step 3: Write pointer lifecycle tests**

Using explicit `pointerId`, assert:

- only player+roller+armed can capture;
- pointer move changes local `data-grabbed="true"` presentation but emits nothing;
- pointer-up outside the tray still emits once;
- duplicate pointer-up/click cannot emit again;
- pointer-cancel and lost capture return to armed without release;
- unmount during grab cleans up;
- reduced motion preserves explicit input.

Stub `setPointerCapture`, `hasPointerCapture`, and `releasePointerCapture` in jsdom.

- [ ] **Step 4: Implement one-shot pointer handling**

Track pointer ID, origin, current point, and accumulated path distance. Use pointer capture on an accessible button labeled `Grab d20`; set `touch-action: none`. Pointer-up builds the same release contract as Roll. Never pass move samples to parent/spectator.

- [ ] **Step 5: Verify, capture, and commit**

```bash
npm run test:run -- \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/attackDieMotion.test.ts \
  src/components/ui/dice/DiceTray3D.test.tsx
npm run typecheck
npm run lint -- --quiet
```

Capture mid-drag, outside-travel, and settled states under `task-4/`. Then:

```bash
git add \
  src/components/ui/dice/dicePresentationRelease.ts \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/DiceTray3D.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/attackDieMotion.ts \
  src/components/ui/dice/attackDieMotion.test.ts \
  public/themes/base.css
git commit -m "feat: add grab and release dice gesture (#749)"
```

---

### Task 5: Add Two Collectible Presets

**Files:**
- Create: `src/components/ui/dice/attackDiePreset.ts`
- Create: `src/components/ui/dice/attackDiePreset.test.ts`
- Modify: `src/components/ui/dice/attackDieMaterial.ts`
- Modify: `src/components/ui/dice/attackDieMaterial.test.ts`
- Modify: `src/components/ui/dice/AttackDie3D.tsx`
- Modify: `src/components/ui/dice/AttackDie3D.test.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx`

**Produces:** allowlisted Stormforged and Cryptstone identities, sharing geometry/face map but using distinct treatments.

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
```

- [ ] **Step 1: Write registry tests**

Require:

```ts
lightning: {
  id: 'lightning',
  displayName: 'Stormforged',
  familyId: 'stormforged',
  modelContractId: 'lightning-d20',
  materialTreatment: 'lightning',
  scaleMultiplier: 1,
}
crypt: {
  id: 'crypt',
  displayName: 'Cryptstone',
  familyId: 'cryptstone',
  modelContractId: 'lightning-d20',
  materialTreatment: 'crypt',
  scaleMultiplier: 1,
}
```

Unknown IDs and URL-shaped values resolve `undefined`.

- [ ] **Step 2: Verify RED and implement the frozen allowlist**

```bash
npm run test:run -- src/components/ui/dice/attackDiePreset.test.ts
```

Implement without accepting caller URLs.

- [ ] **Step 3: Write treatment tests**

For Cryptstone, assert cloned standard materials use:

```ts
body.color.set('#24272b');
body.roughness = 0.9;
body.metalness = 0.05;
numeral.color.set('#d8cfb2');
numeral.emissive.set('#4d563c');
numeral.emissiveIntensity = reducedMotion ? 0.08 : 0.12;
```

Stormforged preserves current source treatment. Both dispose owned clones once and include treatment/reduced state in shader cache key.

- [ ] **Step 4: Verify RED and implement treatment**

```bash
npm run test:run -- src/components/ui/dice/attackDieMaterial.test.ts
```

Wire preset resolution inside `AttackDie3D`. Unknown preset emits failure code `'unknown-preset'` and uses fallback.

- [ ] **Step 5: Add concept selector and shared result invariance test**

Switching preset changes both display name and material treatment but does not change `authoritativeResult`, target quaternion, or release semantics. Label both presets provisional.

- [ ] **Step 6: Verify, capture, and commit**

```bash
npm run test:run -- \
  src/components/ui/dice/attackDiePreset.test.ts \
  src/components/ui/dice/attackDieMaterial.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx
npm run typecheck
npm run lint -- --quiet
```

Capture both settled looks under `task-5/`. Then:

```bash
git add \
  src/components/ui/dice/attackDiePreset.ts \
  src/components/ui/dice/attackDiePreset.test.ts \
  src/components/ui/dice/attackDieMaterial.ts \
  src/components/ui/dice/attackDieMaterial.test.ts \
  src/components/ui/dice/AttackDie3D.tsx \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/DiceTray3D.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx
git commit -m "feat: add collectible dice presets (#749)"
```

---

### Task 6: Show Roller and Spectator Side by Side

**Files:**
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.test.tsx`
- Modify: `public/themes/base.css`

**Produces:** two visible panes that show the roller's same preset and begin from one simulated compact release; pointer movement remains local.

- [ ] **Step 1: Write shared-witness tests**

Assert:

1. Roller and Spectator headings render;
2. both trays receive identical preset/result/token;
3. spectator receives `witnessRole="spectator"` and no controls;
4. pointer move changes only roller local state;
5. roller release causes both trays to receive the same frozen release object and rolling phase;
6. monster mode causes one shared automatic release;
7. stale telemetry cannot settle a newer token;
8. matching observed telemetry settles both;
9. duplicate release key is ignored;
10. concept copy says `Simulated presentation coordination · no production networking`.

- [ ] **Step 2: Verify RED**

```bash
npm run test:run -- \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx
```

- [ ] **Step 3: Implement local shared release**

Both panes receive the same provider scene/sidecar without a second fetch. Roller `onReleaseRequest` stores one release and changes both phases in one React update. Only matching roller telemetry settles the pair. This is local concept state, not a network abstraction.

- [ ] **Step 4: Add responsive comparison layout**

Use two columns only when two readable tray surfaces fit; otherwise stack. Each pane labels role, preset, phase, and authoritative result. Do not shrink numerals to force columns.

- [ ] **Step 5: Verify, capture, and commit**

```bash
npm run test:run -- \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx
npm run typecheck
npm run lint -- --quiet
```

Capture armed and rolling paired views under `task-6/`. Then:

```bash
git add \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/components/ui/dice/DiceTray3D.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  public/themes/base.css
git commit -m "feat: compare shared roller and spectator throws (#749)"
```

---

### Task 7: Allow Authoritative Result 1–20 Without Lying

**Files:**
- Create: `src/concepts/attack-die-3d/attackDieProvisionalFaceMap.ts`
- Create: `src/concepts/attack-die-3d/attackDieProvisionalFaceMap.test.ts`
- Modify: `src/concepts/attack-die-3d/attackDieExperiment.ts`
- Modify: `src/concepts/attack-die-3d/attackDieExperiment.test.ts`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.tsx`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx`

**Produces:** exact-digest provisional 1–20 map and result input. Wrong/missing mappings use SVG.

```ts
export const PROVISIONAL_LIGHTNING_GLB_SHA256 =
  '8a8e50995ee790481e6d1f4b58919f1acee169398acd783af28376464160c1aa';
export const PROVISIONAL_LIGHTNING_FACE_MAP: ReadonlyArray<{
  result: number;
  quaternion: QuaternionTuple;
}>;
export function provisionalLightningFacesForDigest(
  digest: string
): typeof PROVISIONAL_LIGHTNING_FACE_MAP | readonly [];
```

- [ ] **Step 1: Write completeness/hash/geometry tests**

Require 20 sorted unique results, normalized quaternions, exact hash binding, mismatch `[]`, and result 10's existing approved tuple. For every result, apply its quaternion to the inspected source normal and assert browser +Y within five decimals.

Use these inspected browser-coordinate source normals:

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

- [ ] **Step 2: Verify RED and build deterministic candidate poses**

Align each normal to +Y, then yaw around +Y. Start with yaw `225` for result 10 and `0` for the other explicit entries. Zero is a review start, not a readability claim.

```bash
npm run test:run -- src/concepts/attack-die-3d/attackDieProvisionalFaceMap.test.ts
```

- [ ] **Step 3: Calibrate readability result by result**

For each result 1→20 at the 60° three-quarter camera:

1. verify the intended numeral is physically uppermost;
2. adjust only yaw around +Y until readable;
3. store the final normalized tuple explicitly;
4. rerun the +Y geometry assertion;
5. keep contact sheets/screenshots private.

The map remains concept-only provisional and is not asset-team human approval.

- [ ] **Step 4: Bind provider import to the exact digest**

Only exact digest imports all provisional faces. Digest mismatch passes no mapped calibration pose outside Calibrate, forcing SVG. Add authoritative input 1–20 to the Tray stage and mirror it to both panes.

- [ ] **Step 5: Verify all results and fallback**

```bash
npm run test:run -- \
  src/concepts/attack-die-3d/attackDieProvisionalFaceMap.test.ts \
  src/concepts/attack-die-3d/attackDieExperiment.test.ts \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/components/ui/dice/attackDieContract.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx
npm run typecheck
npm run lint -- --quiet
```

Browser-run results 1→20 in normal and reduced motion. Any wrong physical face is a blocker and requires a failing regression test before correction. Capture a 20-face private contact sheet under `task-7/`.

- [ ] **Step 6: Commit**

```bash
git add \
  src/concepts/attack-die-3d/attackDieProvisionalFaceMap.ts \
  src/concepts/attack-die-3d/attackDieProvisionalFaceMap.test.ts \
  src/concepts/attack-die-3d/attackDieExperiment.ts \
  src/concepts/attack-die-3d/attackDieExperiment.test.ts \
  src/concepts/attack-die-3d/AttackDie3DConcept.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx
git commit -m "feat: settle authoritative d20 results in tray (#749)"
```

---

### Task 8: Final Browser Matrix, GIF, Documentation, and Review

**Files:**
- Modify: `docs/how-to/attack-die-3d-concept.md`
- Modify only through test-first fixes: files owned by Tasks 0–7 and matching tests
- Private outputs: `/home/kirk/game-dev/.verification/interactive-dice-tray/task-8/`

**Produces:** repeatable review guide, final private GIF, complete verification evidence, and independent whole-branch review.

- [ ] **Step 1: Document the final Concepts Lab flow**

Document `?concept=attack-die-3d → Tray`, Player/Monster, preset, result, Arm, Roll, grab/release, Roller/Spectator, reduced motion, and forced fallback. Explicitly label:

- provisional skins and face map;
- simulated coordination;
- one d20 only;
- no production combat/network/loadout/damage-dice wiring.

- [ ] **Step 2: Run focused and repository suites**

```bash
npm run test:run -- \
  src/components/ui/dice/attackDieContract.test.ts \
  src/components/ui/dice/attackDieMaterial.test.ts \
  src/components/ui/dice/attackDieMotion.test.ts \
  src/components/ui/dice/attackDiePreset.test.ts \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/DiceTray3DShell.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTray.test.tsx \
  src/concepts/attack-die-3d/attackDieProvisionalFaceMap.test.ts \
  src/concepts/attack-die-3d/attackDieExperiment.test.ts \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx
npm run ci-checks
npm run test:run
git diff --check
```

- [ ] **Step 3: Browser-check the final matrix**

Verify:

- player stays armed for at least 10 seconds;
- Roll and outside release each commit once;
- cancel/lost capture do not throw;
- monster emits one shared automatic release;
- spectator has no controls;
- both panes show roller preset and same authoritative result;
- Stormforged/Cryptstone do not alter result;
- results 1–20 settle correctly and inside the well;
- reduced motion preserves explicit input/exact face;
- unknown preset/digest mismatch/unmapped/renderer failure show SVG;
- responsive narrow layout stacks without clipping.

- [ ] **Step 4: Generate final private GIF**

Capture a player roller/spectator throw at 20 fps for roughly 2.6 seconds and encode:

```text
/home/kirk/game-dev/.verification/interactive-dice-tray/task-8/roller-spectator.gif
```

Copy to `~/Downloads` only for sharing. Verify with `file`, `ffprobe`, and SHA-256; never stage it.

- [ ] **Step 5: Scope audit and documentation commit**

```bash
git status --short
git diff --check
git diff --name-only origin/feat/749-attack-die-3d-concept...HEAD
```

Confirm no changes under `src/components/game/combatPresentation/`, `EncounterView.tsx`, or other repositories. If browser review finds a defect, return to the owning task's tests and commit command before resuming.

```bash
git add docs/how-to/attack-die-3d-concept.md
git commit -m "docs: add interactive dice tray review guide (#749)"
```

- [ ] **Step 6: Independent final review**

Use fresh reviewers for:

1. spec/authority/scope compliance;
2. correctness, cleanup, pointer lifecycle, disposal, and tests;
3. user-flow/accessibility/responsive behavior.

One writer applies accepted fixes test-first; one scoped re-review checks the fix diff. Rerun `npm run ci-checks`, `npm run test:run`, `git diff --check`, and `git status --short`.

- [ ] **Step 7: Push and update PR/issue without merging**

Push only after checks/reviews pass. Update PR #750 and issue #749 with commands/counts, private artifact paths, explicit provisional/simulated status, absence of production wiring, and remaining production gates. End comments:

```text
— asset-pipeline agent, on behalf of KirkDiggler
```

Kirk alone merges.
