# Clean Shared Dungeon Dice Throws Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the production legacy dice drawer with one DOM launch tile, one world-dice controller, and one `SessionCanvas` rigid-body presentation shared through the existing multiplayer throw-plan service.

**Architecture:** Authoritative combat creates a result-bearing but concealed `WorldDiceRequest`. The actor's DOM launch tile owns input, a framework-free raw Rapier planner creates the existing bounded `DiceThrowPlan`, and actor/witness clients feed accepted plans into the same visible Rapier layer inside the existing `SessionCanvas`. Only a rendered settled face or classified semantic fallback opens the combat reveal gate; off-table attempts remain concealed and re-arm the actor.

**Tech Stack:** React 19, TypeScript 5.8, Three.js 0.181, React Three Fiber 9, `@react-three/rapier` 2.2.0, `@dimforge/rapier3d-compat` 0.19.2, Connect-ES, proto release `v0.1.145`, Vitest 4, React Testing Library, Playwright.

**Spec:** `ideas/interactive-dice-tray/shared-dungeon-throws/design.md`

**Tracking:** Parent journey `KirkDiggler/rpg-project#289`; design slice `KirkDiggler/rpg-project#303`; design PR `KirkDiggler/rpg-project#304`; API issue/PR `KirkDiggler/rpg-api#852/#853`; web issue `KirkDiggler/rpg-dnd5e-web#837`.

## Global Constraints

- The replacement web branch starts from freshly fetched `origin/dev` and targets `dev`.
- Preserve the discarded `feat/837-shared-dungeon-dice` worktree only as local evidence. Do not cherry-pick, copy, or import its production coordinator, adapters, physics files, settlement glue, or timers.
- Concepts Lab files remain untouched and working. Production imports nothing from `src/concepts/**`.
- Production mounts no `DiceDrawer`, `DiceTrayPresentation`, tray Canvas, or second WebGL renderer.
- `SessionCanvas` is the one production renderer and the only visual Canvas. Raw Rapier pre-simulation is short-lived computation and is disposed before visible playback.
- The game server remains a validated relay and never gains a simulation engine in this wave.
- Normal actor release-to-visible playback targets less than 250 ms; measure it at the local-feel gate without masking it in this slice.
- The toolkit, `SessionService`, Attack RPC/result authority, and toolkit Story are unchanged.
- Product scope is one body: die ID `attack-d20`, shape `D20`, preset `dice.original.carved.d20`.
- The existing transport remains group-shaped: 1–20 bodies, at most 480 steps, 128 contacts, 256 checkpoint body states, 32 attempts, and 64 KiB encoded size.
- Plans carry no result, hit/miss/critical, damage, target HP, attack reference, arbitrary asset URL, raw pointer sample, or prose.
- Logical colliders are floor cells, authored walls, shut/locked doors, and active dice. Props, items, characters, and monsters are absent.
- Physical worlds derive one aggregate floor from logical floor cells while retaining individual walls and shut doors. Logical floor cells remain the off-table support mask and fingerprint input.
- A settled terminal performs one authoritative-face correction. Do not first snap through the terminal quaternion.
- Reveal result, damage, Story/log, and downed presentation only after the corrected face has rendered and the body is fixed, or after the failed body has been removed and semantic fallback is accepted.
- Off-table never reveals. It increments the presentation attempt and restores only the actor's tile; witnesses receive noninteractive retry status.
- No generic timer may reveal while a body is playing or an off-table retry is active.
- Reduced motion and physical failure retain an explicit semantic completion control.
- TDD is mandatory. Every code task records focused RED, minimum GREEN, focused regression, and one commit.
- Run `npm run ci-check` before every web push and never use `--no-verify`.
- Kirk's human approval is required in the production route at Task 8 and Task 10. Do not continue past either human gate without approval.

---

## File Structure

### Authority and production composition

- Modify `src/components/session/combat-experience/presentation.ts` — concealed attack records, actor/witness world-dice request selector, settlement fact, and reveal projection.
- Modify `src/components/session/combat-experience/useCombatPresentation.ts` — expose `worldDiceRequest` and `settleWorldDice`; remove legacy release callbacks/events.
- Modify `src/components/session/combat-experience/useSessionCombatExperience.ts` — pass the world-dice authority seam through the production controller.
- Modify `src/components/session/combat-experience/types.ts` — remove tray event/callback props and define the generic world-dice overlay/status seam.
- Modify `src/components/session/combat-experience/CombatExperience.tsx` — remove the legacy drawer and settlement-timer gate; render the supplied world-dice control.
- Delete `src/components/session/combat-experience/DiceDrawer.tsx`.
- Delete `src/components/session/combat-experience/diceDrawerVisibility.ts`.
- Delete `src/components/session/combat-experience/useDiceSettleGate.ts`.
- Delete or replace their production-only tests.

### Clean world-dice feature

Create `src/components/session/world-dice/`:

- `types.ts` — immutable request, held state, launch, terminal, progress, and controller output types.
- `physicsSchema.ts` — fixed `RAPIER_DUNGEON_D20_V1` constants and carved-d20 convex hull.
- `plan.ts` — strict immutable plan domain plus proto conversion.
- `worldSnapshot.ts` — logical floor/wall/door/body records, bounded IDs, aggregate physical floor, origin, support predicate, and fingerprint.
- `planner.ts` — dynamically loaded raw Rapier pre-simulation.
- `rapierLoader.ts` — coalesced raw Rapier initialization.
- `playback.ts` — pure fixed-step checkpoint/terminal controller.
- `WorldDiceSceneLayer.tsx` — held/planning/playback/fixed-beat world body inside `SessionCanvas`.
- `gesture.ts` — pure pointer-to-world carry, lift, velocity, release, and cancellation state.
- `controllerState.ts` — pure actor/witness state machine.
- `WorldDiceLaunchTile.tsx` — DOM pointer owner, Roll/fallback controls, retry copy, and accessibility.
- `useWorldDiceController.ts` — effects joining authority, planner, transport, and visible scene commands.
- Matching focused `*.test.ts` / `*.test.tsx` files for every unit.

### Transport and integration

- Modify `src/api/client.ts` — add `SessionPresentationService` client and redact its debug payloads.
- Create `src/api/useSessionDiceThrows.ts` and test — publish plus live-only stream with scope fencing.
- Modify `src/components/session/SessionEncounterView.tsx` and tests — instantiate the controller above sibling map/control, pass scene projection and presentation layer, and settle authority.
- Modify `src/components/session/SessionCanvas.tsx` and tests — expose stable projection ownership and retain the generic presentation layer.
- Modify `src/components/session/combat-experience/CombatExperience.module.css` — compact tile, invisible capture-owner, retry, witness, and failure states.
- Create `src/components/session/world-dice/productionBoundary.test.ts` — no production concept/legacy tray imports and one Canvas.

### Evidence and docs

- Create `scripts/attack-die/capture-shared-dungeon-throw-evidence.mjs` — public-safe two-context diagnostics and screenshots.
- Create `docs/evidence/837-shared-dungeon-throws/README.md` — exact human procedure and evidence manifest.
- Create `docs/architecture/components/session-world-dice.md` — authority/controller/transport/render boundaries.
- Modify `docs/status.md` and `docs/quality.md` only after both human gates pass.

---

### Task 1: Reconcile the provider and establish a clean web worktree

**Files:**
- No product files.
- API worktree: `/home/kirk/.pi/worktrees/rpg-api/852-shared-dice-presentation`
- Discarded web worktree: `/home/kirk/.pi/worktrees/rpg-dnd5e-web/837-shared-dungeon-dice`
- New web worktree: `/home/kirk/.pi/worktrees/rpg-dnd5e-web/837-world-dice-clean`

**Interfaces:**
- Consumes: proto release `v0.1.145`; API PR #853; web issue #837.
- Produces: one clean `feat/837-world-dice-clean` branch based exactly on current `origin/dev`, plus a reconciled API PR head.

- [ ] **Step 1: Invoke worktree setup law**

Read and follow `superpowers:using-git-worktrees` before creating the replacement worktree. Verify the discarded worktree is not modified, staged, committed, or used as a copy source during setup.

- [ ] **Step 2: Verify and publish the rebased API provider**

```bash
cd /home/kirk/.pi/worktrees/rpg-api/852-shared-dice-presentation
git status --short --branch
make ci-check
git fetch origin
git push --force-with-lease origin feat/852-shared-dice-presentation

gh pr view 853 -R KirkDiggler/rpg-api \
  --json baseRefName,headRefName,headRefOid,state,statusCheckRollup
```

Expected: clean worktree; `make ci-check` passes; PR base is `dev`; GitHub head equals the local rebased commit. If the PR has unanswered review threads, stop and use `superpowers:receiving-code-review` before changing API code.

- [ ] **Step 3: Preserve and clearly label the discarded local branch**

```bash
cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/837-shared-dungeon-dice
git branch -m archive/837-shared-dungeon-dice-discarded
git status --short --branch
```

Expected: the uncommitted experiment remains present and the branch name cannot be mistaken for the replacement delivery branch. Do not clean, commit, or push it.

- [ ] **Step 4: Create the replacement from the remote base**

```bash
git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin --prune
git -C /home/kirk/game-dev/rpg-dnd5e-web worktree add \
  /home/kirk/.pi/worktrees/rpg-dnd5e-web/837-world-dice-clean \
  -b feat/837-world-dice-clean origin/dev

cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/837-world-dice-clean
git merge-base --is-ancestor origin/dev HEAD
test "$(git rev-parse HEAD)" = "$(git rev-parse origin/dev)"
```

Expected: both assertions pass.

- [ ] **Step 5: Install assets/dependencies and prove the untouched baseline**

```bash
cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/837-world-dice-clean
npm install
RPG_GAME_ASSETS_PATH=/home/kirk/game-dev/rpg-game-assets \
  ASSETS_SYNC_SKIP_UPDATE=1 npm run assets:sync
npm run ci-check
```

Expected: baseline CI passes before feature edits. Record exact test counts and commit SHA in execution progress; stop and reconcile any baseline failure rather than attributing it to dice work.

---

### Task 2: Replace legacy tray release authority with a world-settlement seam

**Files:**
- Modify: `src/components/session/combat-experience/presentation.ts`
- Modify: `src/components/session/combat-experience/presentation.test.ts`
- Modify: `src/components/session/combat-experience/useCombatPresentation.ts`
- Modify: `src/components/session/combat-experience/useCombatPresentation.test.tsx`
- Modify: `src/components/session/combat-experience/useSessionCombatExperience.ts`
- Modify: `src/components/session/combat-experience/useSessionCombatExperience.test.tsx`
- Modify: `src/components/session/combat-experience/types.ts`
- Modify: `src/components/session/combat-experience/CombatExperience.tsx`
- Modify: `src/components/session/combat-experience/CombatExperience.test.tsx`
- Delete: `src/components/session/combat-experience/DiceDrawer.tsx`
- Delete: `src/components/session/combat-experience/DiceDrawerCollapse.test.tsx`
- Delete: `src/components/session/combat-experience/diceDrawerVisibility.ts`
- Delete: `src/components/session/combat-experience/diceDrawerVisibility.test.ts`
- Delete: `src/components/session/combat-experience/useDiceSettleGate.ts`
- Delete: `src/components/session/combat-experience/useDiceSettleGate.test.tsx`

**Interfaces:**
- Produces:

```ts
export type WorldDiceAuthorityRequest = Readonly<{
  presentationKey: string;
  mode: 'physical' | 'semantic';
  role: 'actor' | 'witness';
  session: string;
  presentationId?: string;
  authoritySeq: bigint;
  roller: string;
  authoritativeFace: number;
  presetId: 'dice.original.carved.d20';
}>;

export interface WorldDiceSettledFact {
  readonly type: 'world-dice-settled';
  readonly presentationKey: string;
  readonly kind: 'physical' | 'semantic' | 'failure';
}

export function selectWorldDiceRequest(
  state: CombatPresentationState
): WorldDiceAuthorityRequest | undefined;
```

`UseCombatPresentationResult` produces `worldDiceRequest` and `settleWorldDice(fact)`; it no longer produces `diceEvents`, `onDiceReleaseRequest`, or `onSemanticReleaseRequest`.

- [ ] **Step 1: Write RED authority tests**

Add focused cases with these exact assertions:

```ts
it('keeps a player attack concealed until matching world settlement', () => {
  const armed = reduceCombatPresentation(base, playerAttackEvent('s', 42n));
  const request = selectWorldDiceRequest(armed);
  expect(request).toMatchObject({
    role: 'actor',
    authoritySeq: 42n,
    authoritativeFace: 17,
  });
  expect(selectVisibleResult(armed)).toBeUndefined();

  const settled = reduceCombatPresentation(armed, {
    type: 'world-dice-settled',
    presentationKey: request!.presentationKey,
    kind: 'physical',
  });
  expect(selectVisibleResult(settled)?.d20).toBe(17);
});

it('does not expose an off-table transition to the authority reducer', () => {
  expectTypeOf<CombatPresentationFact>().not.toMatchTypeOf<{
    type: 'world-dice-off-table';
  }>();
});
```

Also pin witness player attacks as waiting, monster/catch-up as automatic, unsafe presentation IDs as semantic, newer conflict fail-closed, downed target unresolved until settlement, duplicate settlement idempotent, and foreign/stale key ignored.

- [ ] **Step 2: Run RED**

```bash
npm run test:run -- \
  src/components/session/combat-experience/presentation.test.ts \
  src/components/session/combat-experience/useCombatPresentation.test.tsx
```

Expected: FAIL because `WorldDiceAuthorityRequest`, `world-dice-settled`, and the selector do not exist.

- [ ] **Step 3: Implement the pure settlement boundary**

Replace record settlement values with:

```ts
export type CombatDiceSettlement =
  | 'unresolved'
  | 'actor-armed'
  | 'witness-waiting'
  | 'settled'
  | 'auto';
```

Player-owned live attacks become `actor-armed`; other live player attacks become `witness-waiting`; monsters, catch-up, and truthful conflict degradation become `auto`. `isVisible` returns true only for `settled` or `auto` records with an accepted typed event. Remove release profiles/events from combat state; leave legacy dice modules untouched for Concepts Lab.

- [ ] **Step 4: Remove the production tray and timer gate**

Remove `DiceDrawer` and `useDiceSettleGate` from `CombatExperience`. Add one generic control slot:

```ts
interface CombatExperienceBaseProps {
  // existing fields unchanged
  worldDiceControl?: ReactNode;
}
```

Render `{worldDiceControl}` immediately after the map container so the later DOM tile can overlay the map without owning another Canvas. Result, toast, Story, and downed reveal now consume the already-settlement-gated projection directly.

- [ ] **Step 5: Run GREEN and production regressions**

```bash
npm run test:run -- \
  src/components/session/combat-experience/presentation.test.ts \
  src/components/session/combat-experience/useCombatPresentation.test.tsx \
  src/components/session/combat-experience/useSessionCombatExperience.test.tsx \
  src/components/session/combat-experience/CombatExperience.test.tsx \
  src/components/session/downedReveal.test.ts
npm run typecheck
```

Expected: PASS; no production test renders `session-combat-dice-drawer`; Concepts Lab tests remain untouched.

- [ ] **Step 6: Commit**

```bash
git add src/components/session/combat-experience src/components/session/downedReveal.test.ts
git commit -m "refactor(combat): replace tray release with world settlement (#837)"
```

---

### Task 3: Define immutable plans and the canonical logical/physical world

**Files:**
- Modify: `package.json`
- Modify: `package-lock.json`
- Create: `src/components/session/world-dice/types.ts`
- Create: `src/components/session/world-dice/physicsSchema.ts`
- Create: `src/components/session/world-dice/physicsSchema.test.ts`
- Create: `src/components/session/world-dice/plan.ts`
- Create: `src/components/session/world-dice/plan.test.ts`
- Create: `src/components/session/world-dice/worldSnapshot.ts`
- Create: `src/components/session/world-dice/worldSnapshot.test.ts`

**Interfaces:**
- Produces:

```ts
export interface WorldDiceSnapshot {
  readonly colliders: readonly WorldDiceLogicalCollider[];
  readonly bodies: readonly WorldDiceBodyDescriptor[];
}

export function buildWorldDiceSnapshot(input: Readonly<{
  scene: Scene3D;
  doors: ReadonlyMap<string, DoorInfo>;
  bodies: readonly WorldDiceBodyDescriptor[];
}>): WorldDiceSnapshot;

export function physicalCollidersForWorldDice(
  snapshot: WorldDiceSnapshot
): readonly WorldDicePhysicalCollider[];

export function isWorldDiceSupported(
  snapshot: WorldDiceSnapshot,
  position: Readonly<{ x: number; y: number; z: number }>
): boolean;

export function chooseWorldDiceOrigin(
  snapshot: WorldDiceSnapshot,
  playerPosition: CubeCoord,
  occupiedFloorKeys: ReadonlySet<string>
): readonly [number, number];

export async function fingerprintWorldDiceSnapshot(
  snapshot: WorldDiceSnapshot
): Promise<Uint8Array>;

export function parseWorldDicePlan(value: unknown): WorldDicePlan | undefined;
export function worldDicePlanFromProto(value: DiceThrowPlan): WorldDicePlan | undefined;
export function worldDiceDraftToProto(value: WorldDiceDraft): DiceThrowDraft;
```

- [ ] **Step 1: Pin exact provider dependencies**

```bash
npm i --save github:KirkDiggler/rpg-api-protos#v0.1.145
npm i --save @dimforge/rapier3d-compat@0.19.2
npm ls @dimforge/rapier3d-compat
rg 'rpg-api-protos' package.json package-lock.json
```

Expected: direct Rapier ownership is exactly `0.19.2`; generated package resolves the `v0.1.145` source commit.

- [ ] **Step 2: Write RED parser/schema tests**

Use valid one- and two-body fixtures, then hostile table cases for non-finite numbers, quaternion error above `0.0001`, duplicate bodies, unsupported shapes, wrong fingerprint length, attempts outside `1..32`, more than 128 contacts, same-step canonical secondary ordering, missing terminals, contact at/after involved terminal, forbidden keys, and caller mutation.

```ts
const parsed = parseWorldDicePlan(validPlan());
expect(parsed).toBeDefined();
expect(Object.isFrozen(parsed)).toBe(true);
expect(Object.isFrozen(parsed!.bodies)).toBe(true);
expect(JSON.stringify(parsed)).not.toMatch(
  /authoritativeFace|result|damage|targetHp|pointer|clientX|samples/
);
```

- [ ] **Step 3: Write RED snapshot tests**

Pin insertion-order independence, open-door omission, shut/locked inclusion, prop/entity omission, deterministic bounded wall hashes, collision detection for duplicate derived IDs, logical floor count, one aggregate physical floor, and one fixed SHA-256 fingerprint.

```ts
expect(snapshot.colliders.filter((c) => c.kind === 'floor')).toHaveLength(319);
expect(physicalCollidersForWorldDice(snapshot).filter((c) => c.kind === 'floor'))
  .toHaveLength(1);
expect(physicalCollidersForWorldDice(snapshot)).toHaveLength(31);
```

Use the live Reference Tomb fixture values only where the fixture contract guarantees 319/27/3; keep smaller geometry fixtures for arithmetic tests.

- [ ] **Step 4: Run RED**

```bash
npm run test:run -- \
  src/components/session/world-dice/physicsSchema.test.ts \
  src/components/session/world-dice/plan.test.ts \
  src/components/session/world-dice/worldSnapshot.test.ts
```

Expected: FAIL because the feature directory does not exist.

- [ ] **Step 5: Implement strict immutable contracts**

Use one frozen schema record, a sync 64-bit FNV-1a geometry hash for bounded wall IDs, explicit UTF-8 canonical fingerprint records, deep copies of every accepted proto/slice/byte field, and exact list/set validation. Reject rather than clamp malformed plans.

Physical floor aggregation computes X/Z bounds from logical floor boxes while retaining the common floor Y/height. `isWorldDiceSupported` checks individual logical floor records plus the fixed vertical margins; it never uses the aggregate floor for off-table truth.

- [ ] **Step 6: Run GREEN and commit**

```bash
npm run test:run -- src/components/session/world-dice/{physicsSchema,plan,worldSnapshot}.test.ts
npm run typecheck
git add package.json package-lock.json src/components/session/world-dice
git commit -m "feat(dice): define clean dungeon throw contracts (#837)"
```

---

### Task 4: Generate bounded plans with disposable raw Rapier

**Files:**
- Create: `src/components/session/world-dice/rapierLoader.ts`
- Create: `src/components/session/world-dice/rapierLoader.test.ts`
- Create: `src/components/session/world-dice/planner.ts`
- Create: `src/components/session/world-dice/planner.test.ts`

**Interfaces:**
- Produces:

```ts
export type WorldDicePlanGeneration =
  | Readonly<{ kind: 'plan'; draft: WorldDiceDraft }>
  | Readonly<{ kind: 'fallback'; reason: WorldDicePlanningFailure }>;

export async function generateWorldDicePlan(input: Readonly<{
  presentationId: string;
  authoritySeq: bigint;
  attempt: number;
  snapshot: WorldDiceSnapshot;
  bodies: readonly WorldDiceBodyInitial[];
}>): Promise<WorldDicePlanGeneration>;
```

`WorldDicePlanningFailure` is a closed union: `unsupported | invalid-input | collider | limits | incomplete-terminal | cancelled`.

- [ ] **Step 1: Write RED loader and planner tests**

Pin one import/init for concurrent callers, fresh worlds per call, disposal after success/failure, one wall contact, one shut-door contact, open-door pass, off-table, low-energy settled, two-d20 collision with exactly two involved post-states, same-step canonical ordering, mixed terminals, deterministic output, and fallback at all plan bounds.

```ts
const [first, second] = await Promise.all([
  generateWorldDicePlan(input),
  generateWorldDicePlan(input),
]);
expect(first).toEqual(second);
expect(testRapierInitCount()).toBe(1);
expect(testLiveWorldCount()).toBe(0);
```

- [ ] **Step 2: Run RED**

```bash
npm run test:run -- \
  src/components/session/world-dice/rapierLoader.test.ts \
  src/components/session/world-dice/planner.test.ts
```

Expected: FAIL with missing modules.

- [ ] **Step 3: Implement the raw fixed-step planner**

Dynamically import and coalesce `@dimforge/rapier3d-compat` initialization. Create static bodies from `physicalCollidersForWorldDice`, dynamic convex-hull d20 bodies from the shared schema, and one collision-event queue. After each `world.step`:

1. drain started collision transitions;
2. canonically order same-step meaningful contacts;
3. snapshot only involved body states;
4. classify/remove logical off-table bodies;
5. count consecutive in-bounds low-energy steps; and
6. stop only when every body has one terminal.

Wrap `world.free()` and queue cleanup in `finally`. A face/result is not accepted by the function signature.

- [ ] **Step 4: Run GREEN and repeat leak checks**

```bash
npm run test:run -- src/components/session/world-dice/{rapierLoader,planner}.test.ts
for run in 1 2 3 4 5; do
  npm run test:run -- src/components/session/world-dice/planner.test.ts || exit 1
done
```

Expected: deterministic PASS five times; live-world test counter returns zero after every test.

- [ ] **Step 5: Commit**

```bash
git add src/components/session/world-dice
git commit -m "feat(dice): pre-simulate bounded dungeon throws (#837)"
```

---

### Task 5: Build one held/playback/fixed world layer in SessionCanvas

**Files:**
- Create: `src/components/session/world-dice/playback.ts`
- Create: `src/components/session/world-dice/playback.test.ts`
- Create: `src/components/session/world-dice/WorldDiceSceneLayer.tsx`
- Create: `src/components/session/world-dice/WorldDiceSceneLayer.test.tsx`
- Modify: `src/components/session/SessionCanvas.tsx`
- Modify: `src/components/session/SessionCanvas.test.tsx`

**Interfaces:**
- Consumes: `WorldDiceSnapshot`, `WorldDicePlan`, verified runtime preset provider, and current `SessionCanvas.presentationLayer` seam.
- Produces:

```ts
export type WorldDiceSceneCommand =
  | Readonly<{ kind: 'projection' }>
  | Readonly<{ kind: 'held'; body: WorldDiceHeldBody }>
  | Readonly<{ kind: 'planning'; body: WorldDiceHeldBody }>
  | Readonly<{
      kind: 'playback';
      plan: WorldDicePlan;
      authoritativeFaces: Readonly<Record<string, number>>;
    }>
  | Readonly<{ kind: 'fixed-beat'; body: WorldDiceFixedBody }>;

export interface WorldDiceSceneLayerProps {
  readonly command: WorldDiceSceneCommand;
  readonly snapshot: WorldDiceSnapshot;
  readonly projectionRef: MutableRefObject<TrayPlaneProjection | undefined>;
  readonly reducedMotion: boolean;
  readonly onReady: () => void;
  readonly onProgress: (step: number) => void;
  readonly onContact: (event: WorldDiceObservedContact) => void;
  readonly onTerminal: (event: WorldDiceObservedTerminal) => void;
  readonly onFailure: (reason: WorldDicePlaybackFailure) => void;
}
```

- [ ] **Step 1: Write RED pure playback tests**

Pin monotonic fixed steps, one-time checkpoint application, static one-body and dice-pair two-body correction, stale generation fencing, off-table removal, all-body completion, and typed failure when adapters are missing or throw.

- [ ] **Step 2: Write RED R3F layer tests**

Mock Rapier/runtime mesh and prove:

- projection-only command populates the actual camera-plane bridge before handoff and mounts no body;
- one `Physics` world and one aggregate floor once a body is held or played;
- no prop/entity colliders;
- one carved mesh and kinematic body while held/planning;
- dynamic playback from accepted initial state;
- terminal position/zero velocity without terminal quaternion snap;
- one 320 ms current-to-authoritative target slerp;
- final body fixed;
- one rendered frame after final target before terminal callback;
- progress emitted from physics steps;
- no terminal callback after failure/unmount; and
- reduced motion emits direct semantic completion without travel.

```ts
expect(body.setRotation).not.toHaveBeenCalledWith(
  plan.terminal.dice[0]!.state.rotation,
  expect.anything()
);
expect(onTerminal).toHaveBeenCalledTimes(1);
expect(body.setBodyType).toHaveBeenLastCalledWith('fixed', true);
```

- [ ] **Step 3: Run RED**

```bash
npm run test:run -- \
  src/components/session/world-dice/playback.test.ts \
  src/components/session/world-dice/WorldDiceSceneLayer.test.tsx \
  src/components/session/SessionCanvas.test.tsx
```

Expected: FAIL with missing playback/layer.

- [ ] **Step 4: Implement one visible world lifecycle**

Use the existing verified carved-d20 runtime provider and settlement map from `src/components/ui/dice/`; do not import the concept's `PhysicsDieBody`. Mount `Physics` with fixed `1/60`, schema solver values, `colliders={false}`, and the derived physical colliders.

Held/planning uses a kinematic body and current pointer-owned pose. Playback switches that same feature boundary to dynamic plan state. At settled terminal preserve current rotation, reconcile position, zero velocities, set kinematic, slerp once to the authoritative target, set fixed, invalidate a final frame, and then emit terminal. Off-table disables/removes the body immediately.

- [ ] **Step 5: Run GREEN and commit**

```bash
npm run test:run -- \
  src/components/session/world-dice/playback.test.ts \
  src/components/session/world-dice/WorldDiceSceneLayer.test.tsx \
  src/components/session/SessionCanvas.test.tsx
npm run typecheck
git add src/components/session/world-dice src/components/session/SessionCanvas*
git commit -m "feat(dice): render one world dice lifecycle (#837)"
```

---

### Task 6: Add the strict publish/live-stream adapter

**Files:**
- Modify: `src/api/client.ts`
- Modify: `src/api/client.test.ts`
- Modify: `src/api/streamLogging.ts`
- Modify: `src/api/streamLogging.test.ts`
- Create: `src/api/useSessionDiceThrows.ts`
- Create: `src/api/useSessionDiceThrows.test.ts`

**Interfaces:**
- Produces:

```ts
export interface SessionDiceThrowTransport {
  publish(draft: WorldDiceDraft): Promise<WorldDicePlan>;
  readonly streamState: 'connecting' | 'live' | 'reconnecting' | 'stopped';
}

export function useSessionDiceThrows(input: Readonly<{
  session: string;
  member: string;
  onPlan: (plan: WorldDicePlan) => void;
}>): SessionDiceThrowTransport;
```

- [ ] **Step 1: Write RED transport tests**

Mock the generated Connect client and pin authenticated client construction, exact `{session, member, draft}` publish conversion, strict accepted response parse, live delivery, malformed drop, bounded reconnect, one active stream under StrictMode, scope reset, stale callback fencing, and AbortController cleanup.

Add logger tests proving presentation requests/responses/stream items log only:

```ts
{
  presentationId,
  attempt,
  bodyCount,
  contactCount,
  terminalCount
}
```

and never vectors, quaternions, velocities, or raw encoded payloads.

- [ ] **Step 2: Run RED**

```bash
npm run test:run -- \
  src/api/useSessionDiceThrows.test.ts \
  src/api/client.test.ts \
  src/api/streamLogging.test.ts
```

Expected: FAIL because the presentation client/hook and redaction are absent.

- [ ] **Step 3: Implement the live-only adapter**

Add `SessionPresentationService` to the shared authenticated transport. Follow `useSessionEventStream` generation/cancellation conventions but omit Story replay, sequence-gap repair, and Get calls. Parse every publish/stream plan through `worldDicePlanFromProto`; malformed data is dropped with one sanitized development warning.

Reconnect only while scope is mounted, with the existing bounded stream backoff pattern. `publish` accepts only a plan matching the requested session/member-bound roller and draft identity.

- [ ] **Step 4: Run GREEN and commit**

```bash
npm run test:run -- \
  src/api/useSessionDiceThrows.test.ts \
  src/api/client.test.ts \
  src/api/streamLogging.test.ts
npm run typecheck
git add src/api
git commit -m "feat(dice): consume shared throw transport (#837)"
```

---

### Task 7: Build the pure controller, gesture, and DOM launch tile

**Files:**
- Create: `src/components/session/world-dice/gesture.ts`
- Create: `src/components/session/world-dice/gesture.test.ts`
- Create: `src/components/session/world-dice/controllerState.ts`
- Create: `src/components/session/world-dice/controllerState.test.ts`
- Create: `src/components/session/world-dice/WorldDiceLaunchTile.tsx`
- Create: `src/components/session/world-dice/WorldDiceLaunchTile.test.tsx`
- Modify: `src/components/session/combat-experience/CombatExperience.module.css`

**Interfaces:**
- Produces:

```ts
export type WorldDiceControllerState =
  | Readonly<{ phase: 'idle' }>
  | Readonly<{ phase: 'armed'; request: WorldDiceAuthorityRequest; attempt: number }>
  | Readonly<{ phase: 'held'; request: WorldDiceAuthorityRequest; attempt: number; held: WorldDiceHeldBody }>
  | Readonly<{ phase: 'planning'; request: WorldDiceAuthorityRequest; attempt: number; held: WorldDiceHeldBody }>
  | Readonly<{ phase: 'waiting-plan'; request: WorldDiceAuthorityRequest; attempt: number }>
  | Readonly<{ phase: 'playing'; request: WorldDiceAuthorityRequest; attempt: number; plan: WorldDicePlan }>
  | Readonly<{ phase: 'retry'; request: WorldDiceAuthorityRequest; attempt: number }>
  | Readonly<{ phase: 'removing-failure'; request: WorldDiceAuthorityRequest; reason: string }>
  | Readonly<{ phase: 'fixed-beat'; request: WorldDiceAuthorityRequest; untilMs: number }>
  | Readonly<{ phase: 'settled' }>;

export interface WorldDiceLaunchTileProps {
  readonly mode: 'armed' | 'retry' | 'fallback';
  readonly rollerName: string;
  readonly projectionRef: MutableRefObject<TrayPlaneProjection | undefined>;
  readonly snapshot: WorldDiceSnapshot;
  readonly onHeldChange: (held: WorldDiceHeldBody | undefined) => void;
  readonly onLaunch: (launch: WorldDiceLaunch) => void;
  readonly onCancel: () => void;
  readonly onNeutralRoll: () => void;
  readonly onSemanticSettle: () => void;
}
```

- [ ] **Step 1: Write RED gesture tests**

Pin die-target-only begin, pointer identity, pointer capture ownership, valid-floor handoff, invalid release restoration, map-event suppression, left X/Z carry, right+left X/Z freeze and bounded lift, right release resume, filtered bounded velocity, one release, button-mask loss, cancel, and no raw pointer fields in `WorldDiceLaunch`.

- [ ] **Step 2: Write RED state tests**

Pin actor and witness transitions, response/echo dedupe, full identity matching, attempts 1–32, off-table to retry without settlement, attempt-32 fallback, settled to fixed beat, failure-removal before semantic settlement, progress heartbeat, stale plan/callback ignore, and scope reset.

- [ ] **Step 3: Write RED tile tests**

Use real pointer events and prove the tile:

- is visible only for actor armed/retry/fallback modes;
- calls `setPointerCapture` before begin;
- remains mounted but visually hidden after handoff until up/cancel;
- calls `preventDefault` and `stopPropagation` for the active pointer only;
- suppresses context menu only while active;
- offers `Roll d20` and `Reveal result` with keyboard focus; and
- announces `Off the table — throw again` through polite status.

- [ ] **Step 4: Run RED**

```bash
npm run test:run -- \
  src/components/session/world-dice/gesture.test.ts \
  src/components/session/world-dice/controllerState.test.ts \
  src/components/session/world-dice/WorldDiceLaunchTile.test.tsx
```

Expected: FAIL with missing modules.

- [ ] **Step 5: Implement pure ownership and tile behavior**

Keep mutable browser pointer ownership inside the tile/gesture adapter and immutable product transitions in `controllerState.ts`. Stable callbacks read the current integration through refs so coordinator rerenders cannot trigger cleanup and release pointer capture between pointer-down and first move.

The tile is compact and absent from idle layout. Its capture-owner class uses opacity/pointer target behavior without `display:none` or unmounting until release/cancel.

- [ ] **Step 6: Run GREEN and commit**

```bash
npm run test:run -- src/components/session/world-dice/{gesture,controllerState,WorldDiceLaunchTile}.test*
npm run typecheck
git add src/components/session/world-dice src/components/session/combat-experience/CombatExperience.module.css
git commit -m "feat(dice): add direct world throw controls (#837)"
```

---

### Task 8: Integrate the actor path and stop for Kirk's local-feel gate

**Files:**
- Create: `src/components/session/world-dice/useWorldDiceController.ts`
- Create: `src/components/session/world-dice/useWorldDiceController.test.tsx`
- Modify: `src/components/session/SessionEncounterView.tsx`
- Modify: `src/components/session/SessionEncounterView.test.tsx`
- Modify: `src/components/session/combat-experience/CombatExperience.tsx`
- Modify: `src/components/session/combat-experience/CombatExperience.test.tsx`
- Create: `src/components/session/world-dice/productionBoundary.test.ts`

**Interfaces:**
- Produces:

```ts
export interface UseWorldDiceControllerResult {
  readonly control: ReactNode;
  readonly sceneLayer: ReactNode;
  readonly projectionRef: MutableRefObject<TrayPlaneProjection | undefined>;
  readonly status: 'idle' | 'armed' | 'held' | 'planning' | 'playing' | 'retry' | 'fallback';
}

export function useWorldDiceController(input: Readonly<{
  request: WorldDiceAuthorityRequest | undefined;
  scene: Scene3D | undefined;
  playerPosition: CubeCoord | undefined;
  doors: ReadonlyMap<string, DoorInfo>;
  occupiedFloorKeys: ReadonlySet<string>;
  reducedMotion: boolean;
  rollerName: string;
  transport: SessionDiceThrowTransport;
  onSettled: (fact: WorldDiceSettledFact) => void;
}>): UseWorldDiceControllerResult;
```

At this task's human gate, only the actor consumes its accepted publish response. Witness stream plans are collected/fenced but not mounted until Task 9.

- [ ] **Step 1: Write RED actor orchestration tests**

Mock planner/transport/layer callbacks. Pin armed tile, held world command, frozen planning command, exactly one publish, accepted response identity check, response/echo dedupe, actor playback, progress heartbeat, settled/fixed/drawn completion, 600 ms fixed result beat, semantic mode, reduced motion, cancellation, and every unmount/scope cleanup.

- [ ] **Step 2: Write RED production composition tests**

Render `SessionEncounterView` and prove:

```ts
expect(screen.queryByTestId('session-combat-dice-drawer')).not.toBeInTheDocument();
expect(screen.queryByRole('button', { name: /roll d20/i })).not.toBeInTheDocument();

acceptPlayerAttack();
expect(await screen.findByRole('button', { name: /roll d20/i })).toBeVisible();
expect(sessionCanvasProps.presentationLayer).toBeDefined();
```

Also assert no tile for monsters/witnesses/catch-up, map handlers remain unchanged outside active capture, `SessionCanvas` receives one presentation layer, and production source has no `src/concepts`, `DiceTrayPresentation`, or `DiceDrawer` import.

- [ ] **Step 3: Run RED**

```bash
npm run test:run -- \
  src/components/session/world-dice/useWorldDiceController.test.tsx \
  src/components/session/world-dice/productionBoundary.test.ts \
  src/components/session/SessionEncounterView.test.tsx \
  src/components/session/combat-experience/CombatExperience.test.tsx
```

Expected: FAIL because production orchestration is not connected.

- [ ] **Step 4: Implement actor-only accepted-plan playback**

Instantiate the transport and controller in `SessionEncounterScope`, above the sibling `CombatExperience` and `SessionCanvas`. Pass `controller.control` into `CombatExperience.worldDiceControl`; pass `controller.sceneLayer` and `projectionRef` into `SessionCanvas` through its generic presentation/projection seam.

On pointer/neutral release, build one snapshot, generate one draft, publish once, freeze held pose, and consume the accepted response. Do not accept witness stream plans yet. A publish error enters `removing-failure`, unmounts the body, and only then exposes semantic settlement.

- [ ] **Step 5: Run focused GREEN and full actor regressions**

```bash
npm run test:run -- \
  src/components/session/world-dice \
  src/components/session/SessionEncounterView.test.tsx \
  src/components/session/SessionCanvas.test.tsx \
  src/components/session/combat-experience \
  src/components/session/downedReveal.test.ts
npm run typecheck
```

Expected: PASS.

- [ ] **Step 6: Start the integrated local stack at the exact branch heads**

Build/run API PR #853 through the local deployment overlay and start this web worktree on a dedicated port. Verify reflection lists `dnd5e.api.session.presentation.v1alpha1.SessionPresentationService`, then record URLs and SHAs in the execution report.

- [ ] **Step 7: STOP — Kirk local-feel gate**

Ask Kirk to drive one authenticated production player. Do not proceed to Task 9 until Kirk verifies:

1. no old tray at startup;
2. tile only after a player-owned attack arms;
3. pickup and drag remain attached over the map without movement highlights;
4. left+right lift works;
5. wall/shut-door bounce feels connected;
6. off-table returns the tile with result/log/damage still hidden;
7. retry settles with one face correction and no trailing rolls; and
8. damage/log reveal follows the visibly fixed die.

Record Kirk's words and any defects. A rejected feel gate stays on this task/branch and returns to focused RED tests; it is not deferred to multiplayer.

- [ ] **Step 8: Commit the approved actor slice**

```bash
git add src/components/session src/components/ui/dice package.json package-lock.json
git commit -m "feat(session): throw the attack die into the dungeon (#837)"
```

---

### Task 9: Add witness playback, concealed retry, and classified failure

**Files:**
- Modify: `src/components/session/world-dice/controllerState.ts`
- Modify: `src/components/session/world-dice/controllerState.test.ts`
- Modify: `src/components/session/world-dice/useWorldDiceController.ts`
- Modify: `src/components/session/world-dice/useWorldDiceController.test.tsx`
- Modify: `src/components/session/world-dice/WorldDiceLaunchTile.tsx`
- Modify: `src/components/session/SessionEncounterView.tsx`
- Modify: `src/components/session/SessionEncounterView.test.tsx`
- Modify: `src/components/session/combat-experience/CombatExperience.module.css`

**Interfaces:**
- Extends the Task 8 controller so both roles consume matching streamed plans. Produces one noninteractive witness status and one failure-removal handshake.

- [ ] **Step 1: Write RED multiplayer transition tests**

Pin:

- plan-before-event buffer bounded to 3 seconds;
- event-before-plan witness fallback bounded to 10 seconds;
- actor response/echo exactly once;
- witness stream plan exactly once;
- full correlation and fingerprint checks;
- one door refresh plus 500 ms grace on fingerprint mismatch;
- shared off-table to actor retry/witness waiting status;
- attempts 1–31 accepted and attempt 32 exposes semantic control;
- newer presentation cannot preempt active player throw;
- stale attempt/renderer/session callbacks ignored;
- reconnect does not replay; and
- catch-up/monster remains automatic.

- [ ] **Step 2: Write RED failure and reveal-order tests**

Use fake timers and an unmount observer to prove:

```ts
failPlayback('stalled');
expect(onSettled).not.toHaveBeenCalled();
expect(worldBody).toBeUnmounted();
completeFailureRemoval();
expect(fallbackControl).toBeVisible();
```

Pin a progress watchdog that classifies failure only after 2 seconds with no advancing playback step. Receiving progress resets it. The watchdog removes/fixes the body and never reveals directly. No watchdog runs in `retry` or while the actor retains a valid fallback control.

- [ ] **Step 3: Run RED**

```bash
npm run test:run -- \
  src/components/session/world-dice/controllerState.test.ts \
  src/components/session/world-dice/useWorldDiceController.test.tsx \
  src/components/session/SessionEncounterView.test.tsx
```

Expected: FAIL because witness stream plans and retry/failure states are not connected.

- [ ] **Step 4: Implement one shared plan-consumption path**

Route publish responses and stream plans through one `acceptPlan(plan, source)` callback. Validate complete authority identity, expected attempt, schema, body contract, and current snapshot fingerprint before dispatching `plan-accepted`. Actor and witness both create the same `WorldDiceSceneCommand.playback` from that accepted plan.

On off-table, actor dispatches `retry` and renders the tile; witness dispatches `waiting-plan` and renders polite text `<roller> is throwing again`. Neither calls `onSettled`.

On true failure, first transition to `removing-failure`, remove the scene layer, acknowledge removal on the next committed render, and then expose/call semantic completion as specified for actor/witness/reduced motion.

- [ ] **Step 5: Run GREEN plus timing regressions**

```bash
npm run test:run -- \
  src/components/session/world-dice \
  src/api/useSessionDiceThrows.test.ts \
  src/components/session/SessionEncounterView.test.tsx \
  src/components/session/combat-experience \
  src/components/session/downedReveal.test.ts
npm run typecheck
```

Expected: PASS; fake-timer suite has no pending timers after unmount.

- [ ] **Step 6: Commit**

```bash
git add src/components/session src/api
git commit -m "feat(dice): share retries and settlement with witnesses (#837)"
```

---

### Task 10: Complete evidence, human multiplayer acceptance, CI, and review

**Files:**
- Create: `scripts/attack-die/capture-shared-dungeon-throw-evidence.mjs`
- Create: `docs/evidence/837-shared-dungeon-throws/README.md`
- Create: `docs/architecture/components/session-world-dice.md`
- Modify: `docs/status.md`
- Modify: `docs/quality.md`
- Modify tests found by final regression only when they reveal a real product defect.

**Interfaces:**
- Produces: reproducible public-safe evidence, approved two-player behavior, one web PR to `dev`, answered review, and green required checks.

- [ ] **Step 1: Add RED evidence manifest validation**

The evidence script must reject a run unless each scenario has both independent context IDs and these sanitized fields:

```ts
{
  presentationId,
  attempt,
  colliderFingerprint,
  plannedContacts: [{ kind, colliderId, step }],
  observedContacts: [{ kind, colliderId, step }],
  terminal,
  fallback,
  authoritativeFace
}
```

It must reject raw pointer fields, vectors, quaternions, velocities, auth headers, URLs containing tokens, licensed model bytes, or private asset paths.

- [ ] **Step 2: Run the complete automated gate before asking Kirk**

```bash
npm run format
npm run lint
npm run typecheck
npm run build
npm run test:run
npm run ci-check
git diff --check
```

Expected: all pass with exact counts recorded. Inspect the production build graph and source boundary test to confirm one Canvas and no production concept/legacy tray import.

- [ ] **Step 3: STOP — Kirk integrated two-player gate**

Bring Kirk into two independently authenticated production windows. Kirk drives the actor; the implementer records only sanitized evidence. Verify in order:

1. one release produces exactly one API `PublishDiceThrow`;
2. actor and witness each show one world throw;
3. both record the same authored wall contact;
4. both record the same shut-door contact;
5. after opening that door through the real verb, both omit it and pass through;
6. off-table removes both world dice, reveals nothing, restores only actor control, and shows witness retry status;
7. retry settles to the same authoritative face with one correction;
8. damage, log, downed reveal, and toast occur after the fixed face;
9. Roll and reduced-motion controls complete truthfully;
10. interrupted stream uses bounded fallback without stale replay;
11. weapons remain visible and camera/map interactions remain unchanged outside active capture; and
12. browser consoles contain zero page errors or raw plan payloads.

Do not declare the feature complete until Kirk explicitly approves this gate.

- [ ] **Step 4: Fix rejected-gate findings through TDD**

For each observed defect, add one focused failing regression test in the owning module, run it RED, implement the minimum fix, run focused GREEN, then repeat Step 2 and Step 3. Do not patch behavior directly from console guesses.

- [ ] **Step 5: Write evidence and architecture docs**

Document one renderer/controller, authority/reveal boundary, logical-versus-physical colliders, transport live-only behavior, off-table concealment, classified failure removal, exact test commands/counts, branch SHAs, and Kirk's approval. Do not claim broader damage-dice, mobile, or server-simulation readiness.

- [ ] **Step 6: Commit and push**

```bash
git add src package.json package-lock.json scripts docs
git commit -m "docs(dice): record shared dungeon throw evidence (#837)"
npm run ci-check
git push -u origin feat/837-world-dice-clean
```

- [ ] **Step 7: Open the web PR to `dev`**

Use the UI/UX Team signature with authenticated login `KirkDiggler`. Link journey #289, design #303/PR #304, proto #257/tag `v0.1.145`, API #852/PR #853, both human gates, and exact CI/evidence artifacts. Request exactly one Copilot review round and do not report the PR ready in the same turn.

- [ ] **Step 8: Receive and close the review round**

Invoke `superpowers:receiving-code-review`. Read all findings before fixing any. Apply valid fixes on the same branch with focused RED/GREEN tests; reply to every thread with the exact commit or technical decline. Never re-request Copilot.

Verify thread closure:

```bash
R=repos/KirkDiggler/rpg-dnd5e-web/pulls/$WEB_PR/comments
echo "open threads: $(gh api "$R" --jq '[.[]|select(.in_reply_to_id==null)]|length')"
echo "with reply: $(gh api "$R" --jq '[.[]|select(.in_reply_to_id!=null)]|length')"
```

Expected: every top-level finding has at least one reply.

- [ ] **Step 9: Final verification before completion claim**

Invoke `superpowers:verification-before-completion`, rerun `npm run ci-check`, verify GitHub required checks, read back PR base/head, and confirm the local tree is clean. Keep API PR #853 ahead of the web merge and keep design PR #304 open until the provider/API/web merge order is complete.

---

## Merge order

Kirk alone merges, in this order:

```text
rpg-api-protos #257 (already merged, v0.1.145)
  -> rpg-api #853 to dev
  -> rpg-dnd5e-web clean #837 PR to dev
  -> rpg-project #304 to main
```

The web PR must pin the published proto artifact and consume an API head that passed the integrated two-browser gate. The design record merges last.
