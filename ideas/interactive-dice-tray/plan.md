# Interactive Collectible 3D Dice Tray Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development to implement this plan task-by-task. Each task is a visible vertical slice and ends with independent review plus a browser checkpoint.

**Goal:** Grow the existing lightning-d20 proof into a production-intent 3D dice visualization through visible outside-in increments: physical drawer placement, explicit Roll, a strict append-only component event contract, grab/release, and two event-fed roller/spectator views. Result 10 remains the fixed fixture truth for this completion slice; collectible variants and full asset-owned 1–20 settlement are deferred follow-ups.

**Architecture:** `DiceTray3DShell` owns the overflow-capable presentation surface inside the physical drawer. `DiceTray3D` owns accessible low-level interaction and composes `AttackDie3D`. `DiceTrayPresentation` is the literal shared production-intent component: it projects a strictly validated append-only request/release event list into `DiceTray3D` and local renderer-observation state. Concepts Lab owns only fixture event production/delivery; future production maps authoritative facts into the same event contract without rewriting these components.

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
- Do not stream pointer movement. Roller and spectator consume one compact, sanitized release event from the same append-only component-input list in the Concepts Lab fixture.
- Presets are stable collectible die identities, not `player`/`monster` booleans. Spectators render the roller's selected preset.
- Only allowlisted preset/model contracts may resolve. No arbitrary asset URL may enter through props or release data.
- A physical 3D result must be mapped to the exact inspected GLB. Unknown preset, digest mismatch, unmapped result, invalid result, load/shader/context failure, or invalid tray cardinality fails closed to the existing semantic SVG surface.
- Known lightning GLB SHA-256: `8a8e50995ee790481e6d1f4b58919f1acee169398acd783af28376464160c1aa`. Any other digest invalidates physical result mappings. This slice uses only the inspected provisional result-10 pose.
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
| 2 | Sample gameplay screen with the current dock/log and one result-10 d20 in a left dice drawer |
| 3 | Player waits for explicit Roll; monster can auto-roll |
| 4 | Single-pane concept now runs through the strict shared event-fed component |
| 5 | Optional grab, shake, and release through the same event request |
| 6 | Roller and spectator instances consume one append-only fixture event list |
| 7 | Final browser matrix, private GIF, documentation, and independent branch review |
| Deferred A | Stormforged and Cryptstone collectible presets |
| Deferred B | Asset-owned authoritative input 1–20 |

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

### Task 2: Put One Working Result-10 d20 in a Left Gameplay Drawer

**Files:**
- Create: `src/components/ui/dice/DiceTray3D.tsx`
- Create: `src/components/ui/dice/DiceTray3D.test.tsx`
- Create: `src/concepts/attack-die-3d/DiceTrayEncounterPreview.tsx`
- Create: `src/concepts/attack-die-3d/DiceTrayEncounterPreview.test.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.tsx`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx`
- Modify: `public/themes/base.css`

**Produces:** a fixture-backed sample gameplay screen using the real current `EncounterDock` at the bottom, its default-open combat log on the right, a neutral map stand-in, and an always-visible dice-only drawer floating on the left. The drawer renders one real result-10 `AttackDie3D`. No Roll/grab interaction, center outcome duplication, production encounter wiring, or drawer-preference control yet.

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

export interface DiceTrayEncounterPreviewProps {
  tray: React.ReactNode;
}
```

- [ ] **Step 1: Write failing tray composition/cardinality tests**

Mock `AttackDie3D`. Assert exactly one d20 passes result, token, phase, reduced motion, scene/sidecar override, and calibration pose unchanged. Assert empty, two-item, non-d20, unknown preset, and result outside 1–20 render semantic fallback copy and never mount `AttackDie3D`. In this slice the only allowlisted preset is `lightning`; no caller-supplied URL exists.

- [ ] **Step 2: Write failing gameplay-composition tests**

Render `DiceTrayEncounterPreview` with a labeled tray stub. Assert:

1. one `data-testid="dice-tray-encounter-preview"` contains a neutral map area;
2. the tray is inside `data-testid="dice-tray-left-drawer"` and the real `EncounterDock` is present;
3. the dock's combat-log toggle is present and the default-open `data-testid="floating-log"` is on the right;
4. the drawer is before the dock in DOM order and is labeled `Always visible · dice only`;
5. the drawer contains no `HIT`, `MISS`, `CRIT`, `damage total`, or modifier equation copy;
6. no production `EncounterView` or `CombatPresentation` component is imported or mounted.

Update the Tray panel test to expect `Gameplay placement checkpoint`, `Result 10 only · no interaction yet`, the encounter preview, left drawer, real dock, and die renderer rather than the old empty copy.

- [ ] **Step 3: Verify RED**

```bash
npm run test:run -- \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/concepts/attack-die-3d/DiceTrayEncounterPreview.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx
```

Expected: FAIL because `DiceTray3D`, the encounter preview, and placed die do not exist.

- [ ] **Step 4: Implement the placement-only tray**

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

Use reduced motion with `phase="settled"` and the existing `PROVISIONAL_RESULT_10_POSE`. Pass the parent concept's already-loaded scene and sidecar into the panel/tray so the gameplay stage does not fetch the GLB twice. `AttackDie3D` remains the real renderer; the SVG `DiceTray` remains its truthful fallback.

- [ ] **Step 5: Compose the sample gameplay screen with current UI**

`DiceTrayEncounterPreview` owns fixture data only. Reuse the real `EncounterDock` with a representative current-turn character, actions, economy, and `CONCEPT_LOG_ENTRIES`; callbacks are inert concept handlers. Layout:

- preview frame: 1024×768 aspect target, responsive `max-width: 100%`, map above dock;
- dock: real component along the bottom, not a visual reimplementation;
- combat log: real dock-owned default-open surface on the right;
- drawer: absolute/floating lower-left above the dock, always open in this concept;
- drawer content: dice only, with short placement-status copy outside the tray surface;
- map: neutral fixture backdrop only; no licensed screenshot or fabricated live encounter state.

Do not import or modify `EncounterView`, `CombatPresentation`, `useBeatSequencer`, or production transport/state code.

- [ ] **Step 6: Implement containment, opened-drawer depth, and responsive styling**

Add `data-testid="dice-tray-3d-renderer"`. The drawer may use a compact visual shell derived from the same 440×360 motion coordinate space, but must remain large enough for readable numerals. At settled result 10, the complete visible die must fit inside `dice-tray-3d-well`. Keep the motion surface `overflow: visible`; the preview frame itself may clip only at the simulated viewport edge, not at the rounded tray.

The always-open surface must read as a physical drawer seen from above and pulled open toward the player. Build a CSS/DOM 2.5D carcass whose dominant plane is a wider foreshortened rolling floor, with a distinct back plane, angled side planes, lower front face, restrained centered handle, shallow left-edge attachment cue, and directional shadow. Do not use nested rounded rectangles or a tall square profile that can read as an upright safe, monitor, or appliance. Decorative planes are `aria-hidden`. Do not transform the Three.js canvas or change renderer camera, scale, lighting, pointer coordinates, result settlement, or fallback. No open/close animation or preference control belongs in this slice.

At narrow review widths, stack the drawer above the dock/map or scale the preview without overlapping the dock/log; do not move the drawer into the center outcome lane. The complete drawer assembly and combat log use the same 10px gap above the dock at desktop/floor widths; decorative faces must be included in that visible bound rather than overflowing below the measured drawer box.

- [ ] **Step 7: Verify GREEN and capture**

```bash
npm run test:run -- \
  src/components/ui/dice/DiceTray3DShell.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/game/EncounterDock.test.tsx \
  src/concepts/attack-die-3d/DiceTrayEncounterPreview.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx
npx prettier --check \
  public/themes/base.css \
  src/components/ui/dice/DiceTray3D.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/concepts/attack-die-3d/DiceTrayEncounterPreview.tsx \
  src/concepts/attack-die-3d/DiceTrayEncounterPreview.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx
npm run typecheck
npm run lint -- --quiet
git diff --check
```

At the real route, browser-verify the current dock remains readable, the combat log stays on the right, the drawer stays left above the dock, center map space remains unobstructed, and the settled die is fully inside the rounded well. Capture:

```text
/home/kirk/game-dev/.verification/interactive-dice-tray/task-2/gameplay-left-drawer-result-10.png
```

Also capture a narrow responsive state under the same directory.

- [ ] **Step 8: Commit**

```bash
git add \
  src/components/ui/dice/DiceTray3D.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/concepts/attack-die-3d/DiceTrayEncounterPreview.tsx \
  src/concepts/attack-die-3d/DiceTrayEncounterPreview.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx \
  public/themes/base.css
git commit -m "feat: preview dice drawer with combat ui (#749)"
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

- reject blank presentation ID and, for this pre-event Task 3 implementation, a non-allowlisted preset; Task 4 explicitly supersedes this constructor rule with bounded non-URL preset syntax while keeping asset resolution allowlisted;
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

### Task 4: State the Outside-In Presentation Event Contract

**Files:**
- Create: `src/components/ui/dice/dicePresentationEvent.ts`
- Create: `src/components/ui/dice/dicePresentationEvent.test.ts`
- Create: `src/components/ui/dice/DiceTrayPresentation.tsx`
- Create: `src/components/ui/dice/DiceTrayPresentation.test.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.test.tsx`
- Modify: `src/components/ui/dice/dicePresentationRelease.ts`
- Modify: `src/components/ui/dice/dicePresentationRelease.test.ts`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx`

**Produces:** the literal shared production-intent event-fed tray component. Concepts Lab supplies fixture events; future production supplies the same contract without rewriting the component.

```ts
export type DicePresentationEvent =
  | {
      schemaVersion: 1;
      type: 'dice-presentation-requested';
      eventId: string;
      presentationId: string;
      roller: { entityId: string; role: 'player' | 'monster' };
      die: {
        kind: 'd20';
        presetId: string;
        authoritativeResult: number;
      };
    }
  | {
      schemaVersion: 1;
      type: 'dice-presentation-released';
      eventId: string;
      presentationId: string;
      release: DicePresentationRelease;
    };
```

`DiceTrayPresentation` consumes an ordered append-only `readonly DicePresentationEvent[]`, a viewer `witnessRole`, and explicitly development-only renderer injections. It renders the existing `DiceTray3D`; it does not duplicate the renderer or drawer. Its only outward interaction is a request to append a validated release event. `presentationId` is the sole external correlation. The shared component allocates a local numeric renderer generation whenever the accepted presentation ID changes; neither events nor production callers supply `DiceTray3DItem.id` or `presentationToken`.

- [ ] **Step 1: Write strict event/parser tests**

Require exact schema/version/type keys; bounded safe IDs; one d20; integer result 1–20; syntactically safe preset IDs that cannot be URLs; strict recursively reconstructed/frozen inbound release parsing; no renderer ID/token, URL, description, outcome, hit, target, damage, or transport field. Task 4 changes both release construction and parsing from registry-membership validation to bounded non-URL preset-ID syntax; only asset resolution remains allowlisted. This lets a newer safe preset request/replay through truthful SVG on an older client without authorizing a model URL.

Unknown keys at any depth and malformed IDs fail closed. The first valid request fixes all authority-bearing facts for a presentation ID; identical request replay is idempotent, conflicting request facts are rejected, release-before-request is ignored, and the first matching release wins solely by presentation ID regardless of variation. The released event's outer `presentationId` must equal `release.presentationId`, and its repeated `release.presetId` must equal the immutable request preset; mismatch is ignored. Later identical or conflicting releases are ignored. Replay requires a new presentation ID.

- [ ] **Step 2: Verify RED and implement the event projector**

```bash
npm run test:run -- \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/dicePresentationEvent.test.ts
```

Provide pure functions that recursively reconstruct/freeze one unknown event and project an ordered event list into the latest valid requested presentation plus at most one matching release. Malformed later events do not poison the last valid projection. Array order is component-input delivery order; do not add wall-clock authority or a network abstraction. Update `dicePresentationReleaseKey` so release cardinality is the presentation ID, not `${presentationId}:${variation}`.

- [ ] **Step 3: Write the shared component tests**

Assert:

1. no valid request renders no tray;
2. request without release renders player+roller armed indefinitely;
3. Roll asks the host to append one frozen release event but does not mutate props or leave armed until delivery;
4. appending that exact event to the mounted armed stream starts rolling;
5. matching renderer telemetry for the captured local generation/result settles the local component;
6. a new presentation ID allocates a new local renderer generation and resets observation;
7. initial hydration containing request+release converges directly to settled/fallback without replay;
8. truncation, reorder, or non-prefix replacement converges instead of animating stale choreography;
9. monster and spectator components never request automatic events; the host owns monster autoplay;
10. malformed/stale/duplicate/conflicting events do not change fixed request facts or active presentation;
11. renderer failure while armed remains concealed, then matching released failure settles truthful SVG without stalling;
12. Roll on an unknown syntactically safe preset requests one event; delivery renders neutral/rolling/settled SVG rather than loading a URL or erasing the result;
13. changing roll status is exposed through a polite live region;
14. the current single-pane Concepts Lab fixture renders `DiceTrayPresentation`, appends its requested release event once, and no longer owns a duplicate phase/telemetry state machine.

- [ ] **Step 4: Implement the literal shared component**

`DiceTrayPresentation` owns only event projection, local renderer generation allocation, delivery-prefix detection, and local renderer-observation/failure phase. It delegates rendering, input, fail-closed behavior, and authority preservation to `DiceTray3D`/`AttackDie3D`. The same component instance shape must be usable by Concepts Lab fixtures and later production callers; do not import from `concepts/`.

Remove monster autoplay production from `DiceTray3D`; `witnessRole` controls view/input only and never grants append authority. Split invalid tray cardinality from unknown safe preset handling: invalid input keeps the existing unavailable copy, while unknown preset uses semantic SVG with armed concealment and post-release result truth. A failure observed before release is remembered but does not reveal; after release it becomes terminal settled fallback.

The requested event carries the already-known result. The released event carries presentation-only variation. Development-only scene/sidecar/calibration injections must be grouped and bound to lightning result 10; a bare calibration pose is not part of the semantic production contract. Production transport/correlation, encounter wiring, and profile ownership remain later adapters and are not represented as fake concept contracts.

Replace the current single-pane concept's direct phase/release orchestration with a minimal append-only fixed-result-10 fixture around `DiceTrayPresentation`. This creates a browser-visible outside-in checkpoint before Task 6 expands the same pattern to two views.

- [ ] **Step 5: Verify and commit**

```bash
npm run test:run -- \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/dicePresentationEvent.test.ts \
  src/components/ui/dice/DiceTrayPresentation.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx
npm run typecheck
npm run lint -- --quiet
```

```bash
git add \
  src/components/ui/dice/dicePresentationEvent.ts \
  src/components/ui/dice/dicePresentationEvent.test.ts \
  src/components/ui/dice/DiceTrayPresentation.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx \
  src/components/ui/dice/DiceTray3D.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/dicePresentationRelease.ts \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx
git commit -m "feat: add event-fed dice presentation contract (#749)"
```

---

### Task 5: Add Optional Grab, Shake, and Release

**Files:**
- Modify: `src/components/ui/dice/dicePresentationRelease.ts`
- Modify: `src/components/ui/dice/dicePresentationRelease.test.ts`
- Modify: `src/components/ui/dice/DiceTray3D.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.test.tsx`
- Modify: `src/components/ui/dice/DiceTrayPresentation.test.tsx`
- Modify: `src/components/ui/dice/attackDieMotion.ts`
- Modify: `src/components/ui/dice/attackDieMotion.test.ts`
- Modify: `public/themes/base.css`

**Produces:** optional pointer/touch ritual that requests one compact release through the shared event contract; pointer movement stays local and affects only decorative motion.

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

Track pointer ID, origin, current point, and accumulated path distance. Use pointer capture on an accessible button labeled `Grab d20`; set `touch-action: none`. Pointer-up builds the same release contract as Roll. `DiceTrayPresentation` wraps that request as the same validated append-only release event used by the button path. Never pass move samples to the host, event list, or spectator.

- [ ] **Step 5: Verify, capture, and commit**

```bash
npm run test:run -- \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/attackDieMotion.test.ts \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx
npm run typecheck
npm run lint -- --quiet
```

Capture mid-drag, outside-travel, and settled states under `task-5/`. Then:

```bash
git add \
  src/components/ui/dice/dicePresentationRelease.ts \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/DiceTray3D.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx \
  src/components/ui/dice/attackDieMotion.ts \
  src/components/ui/dice/attackDieMotion.test.ts \
  public/themes/base.css
git commit -m "feat: add grab and release dice gesture (#749)"
```

---

### Deferred Follow-up A: Add Two Collectible Presets

This is not required for the fixed-result roller/spectator visualization proof. Resume after the shared event-fed experience is accepted or when asset issue #49 supplies promoted preset facts.

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

### Task 6: Prove Event-Fed Roller and Spectator Views

**Files:**
- Create: `src/concepts/attack-die-3d/diceTrayWitnessFixture.ts`
- Create: `src/concepts/attack-die-3d/diceTrayWitnessFixture.test.ts`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTrayEncounterPreview.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTrayEncounterPreview.test.tsx`
- Reuse unchanged: `src/concepts/combat-panel/logFixtures.ts`
- Modify: `public/themes/base.css`

**Produces:** two visible instances of the literal shared `DiceTrayPresentation`, both driven by one append-only fixture event list, plus the real current dock/log fed by structured fixture data.

- [ ] **Step 1: Write the fixture boundary tests**

The concept fixture may own initial events and an append reducer, but no reusable event type, projection, renderer, or tray state machine. Require:

1. one valid fixed-result-10 requested event using the lightning preset;
2. append-only event order and event-ID/release-key deduplication;
3. no fixture field named `description`, `message`, `text`, `url`, `html`, or `transport`;
4. combat history remains `CombatLogEntry[]` structured event data, not authored prose;
5. the current attack log facts use structured roll/bonus/AC/hit/critical and optional damage fields;
6. fixture delivery is deterministic under fake timers and does not generate an authoritative result;
7. player mode starts with request only, while monster mode's single host producer appends one deterministic request/release pair in sequence; rendered consumers never produce monster autoplay.

The existing real `CombatLog` already synthesizes prose late from structured `CombatLogEntry.event` fields. Reuse that contract; do not create another combat-log vocabulary or modify production `CombatLog`, `useCombatLog`, `EncounterDock`, or `EncounterView`.

- [ ] **Step 2: Write shared-witness component tests**

Assert:

1. Roller and Spectator headings render;
2. both are `DiceTrayPresentation` instances receiving the identical immutable event array;
3. both project the same presentation ID, preset, authoritative result, and deeply equal sanitized release values; local renderer generations may differ and object identity is not asserted;
4. spectator has no controls and no event callback authority;
5. pointer movement changes only roller-local grabbed state and appends no event;
6. Roll or pointer release requests one event, the fixture host appends it once, and both instances begin rolling from it;
7. monster mode receives one host-produced shared automatic release event, with neither rendered view requesting it;
8. each instance settles from its own matching renderer telemetry, while stale telemetry cannot settle a newer request;
9. duplicate/stale release events are ignored by the shared projector;
10. visible copy says `Fixture event delivery · shared component contract · no production transport`.

- [ ] **Step 3: Verify RED**

```bash
npm run test:run -- \
  src/concepts/attack-die-3d/diceTrayWitnessFixture.test.ts \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/DiceTrayEncounterPreview.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx
```

- [ ] **Step 4: Implement fixture delivery around shared components**

The concept host stores one append-only event array. In player mode, the roller's validated event request is appended once; the spectator receives no callback. In monster mode, the host fixture producer appends the deterministic request and release in separate ordered deliveries so mounted consumers animate without competing to produce the event. Both shared components consume the same array reference/value and the same parent-loaded provider scene/sidecar without a second GLB fetch. Keep result 10 fixed: this proof is about interaction and witnessing, not face calibration or client randomness.

Keep `DiceTrayEncounterPreview` a fixture composition around the real `EncounterDock` and real log. If it needs a comparison slot, expose generic tray children/labels rather than moving event projection into the concept preview.

- [ ] **Step 5: Add responsive comparison layout**

Use two columns only when two readable 3D tray surfaces fit; otherwise stack. Each pane clearly labels Roller or Spectator and current phase. Preserve the approved physical drawer language, die scale/readability, combat-log default-open behavior, 10px dock rhythm, and narrow no-overlap order. Do not shrink numerals merely to force columns.

- [ ] **Step 6: Verify, capture, and commit**

```bash
npm run test:run -- \
  src/concepts/attack-die-3d/diceTrayWitnessFixture.test.ts \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/DiceTrayEncounterPreview.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/game/EncounterDock.test.tsx \
  src/components/game/CombatLog.test.tsx
npm run typecheck
npm run lint -- --quiet
```

Capture armed, local-grab, shared-rolling, and independently settled paired views under `task-6/` at desktop and narrow widths. Then:

```bash
git add \
  src/concepts/attack-die-3d/diceTrayWitnessFixture.ts \
  src/concepts/attack-die-3d/diceTrayWitnessFixture.test.ts \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/concepts/attack-die-3d/DiceTrayEncounterPreview.tsx \
  src/concepts/attack-die-3d/DiceTrayEncounterPreview.test.tsx \
  public/themes/base.css
git commit -m "feat: compare event-fed roller and spectator (#749)"
```

---

### Deferred Follow-up B: Consume Asset-Owned 1–20 Face Metadata Without Lying

This is not required for the fixed-result-10 roller/spectator visualization proof.

**Blocked until:** `rpg-game-assets#47` supplies the hash-bound lightning-d20 face metadata. Web must not independently reconstruct, label, or publish the asset team's face map.

**Files:**
- Modify: `src/concepts/attack-die-3d/attackDieExperiment.ts`
- Modify: `src/concepts/attack-die-3d/attackDieExperiment.test.ts`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.tsx`
- Modify: `src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx`
- Modify: `src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx`

**Produces:** authoritative result input 1–20 backed only by asset-owned metadata. Wrong, missing, incomplete, or hash-mismatched mappings use SVG.

- [ ] **Step 1: Receive and validate the asset contract**

Use the sidecar delivered through the asset sync boundary. The existing strict contract requires:

- the exact lightning GLB SHA-256;
- the declared right-handed, `+Y`-up, `xyzw` coordinate convention and root correction;
- exactly 20 unique results from 1 through 20;
- one finite normalized settling quaternion per result; and
- candidate/verified state and evidence consistent with the existing schema.

Do not add a web-owned `attackDieProvisionalFaceMap`, source-normal table, manually labeled contact sheet, or duplicate numeric map. If the asset team authors face markers/normals in Blender, their pipeline must emit the sidecar quaternions (or promote the contract schema separately); web consumes the resulting contract.

- [ ] **Step 2: Write provider and fail-closed tests**

Assert that an exact-hash, complete asset sidecar imports all 20 faces into the Concepts Lab. Missing sidecar, invalid cardinality, duplicate result, non-normalized quaternion, invalid state/evidence, or GLB hash mismatch imports no face calibration and forces truthful SVG for unmapped results. Keep the existing concept-only result-10 proof clearly provisional until the asset contract supersedes it.

- [ ] **Step 3: Add authoritative result input**

Add input 1–20 to the Tray stage. Pass the chosen authoritative result unchanged to the roller/spectator presentation. Select its target only from the validated asset sidecar; never derive, rotate, or guess a target in web code. Gesture/release variation remains decorative and cannot alter the selected target.

- [ ] **Step 4: Verify all asset-provided results and fallback**

```bash
npm run test:run -- \
  src/concepts/attack-die-3d/attackDieExperiment.test.ts \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx \
  src/components/ui/dice/attackDieContract.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx
npm run typecheck
npm run lint -- --quiet
```

After the asset team has completed its private face/readability review, browser-run asset-provided results 1→20 in normal and reduced motion. Any wrong physical face is reported back to the asset contract owner rather than patched with a web-local quaternion. Capture web integration evidence privately under `task-7/`; do not duplicate the asset team's calibration evidence.

- [ ] **Step 5: Commit**

```bash
git add \
  src/concepts/attack-die-3d/attackDieExperiment.ts \
  src/concepts/attack-die-3d/attackDieExperiment.test.ts \
  src/concepts/attack-die-3d/AttackDie3DConcept.tsx \
  src/concepts/attack-die-3d/AttackDie3DConcept.test.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.tsx \
  src/concepts/attack-die-3d/DiceTray3DConceptPanel.test.tsx
git commit -m "feat: consume authoritative d20 face metadata (#749)"
```

---

### Task 7: Final Browser Matrix, GIF, Documentation, and Review

**Files:**
- Modify: `docs/how-to/attack-die-3d-concept.md`
- Modify only through test-first fixes: files owned by Tasks 0–6 and matching tests
- Private outputs: `/home/kirk/game-dev/.verification/interactive-dice-tray/task-7/`

**Produces:** repeatable review guide, final private GIF, complete verification evidence, and independent whole-branch review.

- [ ] **Step 1: Document the final Concepts Lab flow**

Document `?concept=attack-die-3d → Tray`, fixed result 10, Player/Monster, Arm, Roll, grab/release, event-fed Roller/Spectator, reduced motion, and forced fallback. Explicitly label:

- provisional lightning model/result-10 pose;
- fixture event delivery through the shared component contract;
- no production transport despite production-intent component inputs;
- one d20 only; and
- no production combat/network/loadout/damage-dice wiring.

- [ ] **Step 2: Run focused and repository suites**

```bash
npm run test:run -- \
  src/components/ui/dice/attackDieContract.test.ts \
  src/components/ui/dice/attackDieMaterial.test.ts \
  src/components/ui/dice/attackDieMotion.test.ts \
  src/components/ui/dice/dicePresentationRelease.test.ts \
  src/components/ui/dice/dicePresentationEvent.test.ts \
  src/components/ui/dice/AttackDie3D.test.tsx \
  src/components/ui/dice/DiceTray3DShell.test.tsx \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx \
  src/components/ui/dice/DiceTray.test.tsx \
  src/concepts/attack-die-3d/diceTrayWitnessFixture.test.ts \
  src/concepts/attack-die-3d/attackDieExperiment.test.ts \
  src/concepts/attack-die-3d/DiceTrayEncounterPreview.test.tsx \
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
- monster emits one shared automatic release event;
- spectator has no controls or event-request authority;
- both literal shared components consume the same append-only fixture events and show fixed result 10;
- pointer movement remains roller-local and never enters the event stream;
- combat log input remains structured `CombatLogEntry` event data with prose generated only by the real renderer;
- reduced motion preserves explicit input and exact result-10 settlement;
- malformed/stale/duplicate presentation events and renderer failure fail closed;
- responsive narrow layout stacks without clipping or overlap.

- [ ] **Step 4: Generate final private GIF**

Capture a player roller/spectator throw at 20 fps for roughly 2.6 seconds and encode:

```text
/home/kirk/game-dev/.verification/interactive-dice-tray/task-7/roller-spectator.gif
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
