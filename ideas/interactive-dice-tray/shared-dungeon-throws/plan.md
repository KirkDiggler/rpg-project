# Incremental Dungeon Dice Checkpoint Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans task-by-task. Stop at the named Kirk gate.

**Goal:** Prove the approved rigid-body ritual in production, then add local planning, unary publication, and visual-only witness delivery as separately approved seams.

**Architecture:** A fresh branch from latest `origin/dev` removes the production drawer and mounts one concept-shaped Rapier body inside the existing `SessionCanvas`. The local authoritative attack remains concealed until that body visibly settles; off-table returns the same presentation to the DOM tile. No pre-simulation, throw plan, publish, stream, witness, or Redis code is imported or called.

**Tech Stack:** React 19, TypeScript, Three.js/R3F, `@react-three/rapier`, verified carved-d20 runtime provider, Vitest, one isolated owned browser.

**Spec:** `ideas/interactive-dice-tray/shared-dungeon-throws/design.md` — “Actor-only production checkpoint.”

## Global constraints

- Reuse web issue #837 but create fresh local branch `feat/837-actor-dice-checkpoint` from freshly fetched `origin/dev`.
- Preserve current `feat/837-world-dice-clean`, its local rejected archives, and all SDD artifacts. Do not reset, cherry-pick, copy, or continue them.
- Read the approved concept as behavioral reference; production imports nothing from `src/concepts/**`.
- Concepts remain untouched.
- One existing `SessionCanvas` is the only Canvas/WebGL renderer.
- Actor-only checkpoint imports/calls no `useSessionDiceThrows`, presentation proto service, planner, plan parser, fingerprint, checkpoint, Redis, or witness code.
- One live body owns handoff → held → release → settlement/off-table.
- Use the concept values verbatim: radius `0.275`; die friction/restitution `0.72/0.48`; damping `0.22/0.16`; floor `0.9/0.25`; wall/door restitution `0.55`; natural assist thresholds `0.28/1.1`; forced assist at 3 seconds; correction 320 ms.
- The direct pointer gesture matches the concept: capture on the DOM token, valid-floor handoff, left X/Z carry, left+right vertical lift with frozen X/Z, filtered release profile, cancellation, and off-table return.
- Attack authority remains server-owned. Physics never determines a result.
- Actor result/damage/log/downed reveal waits for physical settlement or explicit semantic fallback. Off-table never reveals.
- Monster attacks, catch-up history, and nonlocal player attacks remain automatic/read-only and never wait for this actor control.
- No human test until focused/full automated gates and one owned isolated browser proof pass.
- Owned browser/backend processes are recorded and cleaned; never mutate the shared `rpg-dev` stack or user lobbies.
- This checkpoint creates no web PR and does not merge. After Kirk's verdict, stop and design the next seam.

---

### Task 1: Preserve current work and create the fresh actor-only worktree

**Produces:** `/home/kirk/.pi/worktrees/rpg-dnd5e-web/837-actor-dice-checkpoint` on `feat/837-actor-dice-checkpoint` at exact latest `origin/dev`.

- [ ] Read `superpowers:using-git-worktrees` and web `CLAUDE.md`.
- [ ] Verify the current persistent-body worktree is unchanged and record its branch/HEAD/status; do not touch it.
- [ ] Fetch web origin and create the new worktree/branch from `origin/dev`.
- [ ] Install dependencies, sync approved assets, and run untouched `npm run ci-check`.
- [ ] Record baseline SHA/test counts. Stop on any baseline failure.

No product commit.

---

### Task 2: Remove the legacy production tray and add settlement-only local authority

**Files:**
- Modify `src/components/session/combat-experience/{presentation,useCombatPresentation,useSessionCombatExperience,types,CombatExperience}.ts*`
- Delete production `DiceDrawer`, drawer visibility, and timer settle-gate files/tests.
- Modify production/session tests only; do not alter Concepts source.

**Produces:**

```ts
export interface LocalWorldDieRequest {
  readonly presentationKey: string;
  readonly role: 'actor' | 'witness';
  readonly session: string;
  readonly authoritySeq: bigint;
  readonly roller: string;
  readonly authoritativeFace: number;
  readonly presetId: 'dice.original.carved.d20';
}

export function settleLocalWorldDie(input: Readonly<{
  presentationKey: string;
  kind: 'physical' | 'semantic' | 'failure';
}>): void;
```

- [ ] Write RED reducer/hook/component tests: local player attack arms and remains concealed; matching settlement reveals; off-table has no authority fact; monsters/catch-up/nonlocal player auto-settle; conflicts fail closed; no drawer/second Canvas.
- [ ] Implement the minimum immutable authority seam. Do not define plan/transport types.
- [ ] Render one generic `localWorldDieControl?: ReactNode` overlay slot outside the map's isolated stacking context.
- [ ] Run focused combat/session/downed tests and typecheck.
- [ ] Commit `refactor(combat): replace tray with local world settlement (#837)`.

---

### Task 3: Port the concept-shaped live actor body into SessionCanvas

**Files:**
- Create `src/components/session/local-world-die/LocalWorldDieLayer.tsx` and tests.
- Create `localWorldDieGesture.ts` and tests.
- Create `useLocalWorldDie.ts` and tests.
- Create `LocalWorldDieTile.tsx` and tests.
- Create small collider/support helpers and tests under the same folder.
- Modify `SessionEncounterView.tsx`/test, `SessionCanvas.tsx`/test, and combat CSS.

**Interfaces:**

```ts
export type LocalWorldDieCommand =
  | Readonly<{ kind: 'held'; position: readonly [number, number]; height: number }>
  | Readonly<{ kind: 'released'; position: readonly [number, number]; height: number; profile: VisualThrowProfileV1 }>
  | Readonly<{ kind: 'reset' }>;

export interface LocalWorldDieTerminal {
  readonly kind: 'settled' | 'off-table' | 'failure';
}
```

**Implementation shape:**

- One production component owns pointer/held/release state, following `DungeonFloorPhysicsSpike` directly.
- One `Physics` world and one `RigidBody`/verified `RuntimeDiceMesh` mount after valid handoff and remain through terminal.
- Release changes that same body kinematic → dynamic immediately. There is no planner/publish pause.
- Floor cells, authored walls, and current shut/locked doors collide; props/entities/items do not.
- Natural low-energy or three-second assist corrects the same body once to the authoritative face, fixes it, waits for a drawn frame, then emits settled.
- Off-table uses the concept floor/vertical predicate, removes/reset once, restores tile, and emits no combat settlement.
- Explicit Roll uses the concept neutral profile through the same live body path.
- Failure removes body first, then presents explicit Reveal result fallback.

Steps:

- [ ] Write RED gesture tests copied as behavior—not imports—from the approved concept cases.
- [ ] Write RED scene tests for one body handle/mount, collider selection, direct dynamic release, low-energy/3s assist, one correction, drawn settlement, off-table reset, cleanup, StrictMode, and provider failure.
- [ ] Write RED controller/composition tests for actor-only tile, witness absence, concealed retry, settlement reveal, and one Canvas/no concept/network imports.
- [ ] Implement the minimum concept-shaped path. Avoid a generalized plan/controller state machine.
- [ ] Run all local-world-die, combat, SessionCanvas, provider/runtime, and SessionEncounter tests; then full tests, typecheck, lint, format, and build.
- [ ] Commit `feat(session): throw the local attack die in the dungeon (#837)`.

---

### Task 4: Owned local proof and mandatory Kirk gate

**Evidence requirements before Kirk:**

- one isolated backend project on an unused nonshared port serving the normal production session route; fixtures may diagnose failures but cannot satisfy final proof;
- one owned browser/profile/PID with guaranteed cleanup;
- no headless/browser/test CPU contamination;
- topmost pickup target and actual pointer capture;
- reliable 10/10 pickup/handoff/release cycles;
- one body mount/handle per attempt;
- wall and shut-door collisions;
- off-table return with result/log/damage concealed;
- next attempt reliable and free of stale callbacks;
- one correction, fixed/drawn face, then reveal;
- Roll and semantic fallback;
- zero page/console errors; and
- full CI green.

- [ ] Add a small safe evidence protocol containing only attempt, readiness, mount count, terminal/removal reason, and elapsed spans—no pointer coordinates, transforms, velocities, result, auth, or raw payload.
- [ ] Run the owned proof for at least 10 actor attempts and clean every process/container/profile.
- [ ] Run `npm run ci-check` and `git diff --check`.
- [ ] Commit evidence/docs only after proof passes.
- [ ] **STOP and show Kirk the evidence before starting a URL.**
- [ ] Kirk verifies pickup, carry, lift, release, collision feel, off-table retry, settlement correction, reveal order, and absence of the old tray.
- [ ] If rejected, return to the single owning Task 3 behavior through systematic debugging/TDD. Do not add networking.
- [x] If approved, record the verdict and stop the branch. Kirk approved the actor ritual; each later seam below received separate approval.

---

## Post-gate amendment

Tasks 5–7 supersede only the actor-only exclusions on pre-simulation, presentation proto imports, unary publication, and live stream consumption. All authority, one-Canvas, persistent-body, collider, privacy, no-Concept-import, no-web-PR, and short-human-gate constraints remain in force.

### Task 5: Local planning and unary publication checkpoints — complete

- [x] Add Direct/Planned A/B without networking; coordinate terminal type/step only.
- [x] Obtain Kirk approval of Planned feel.
- [x] Pin presentation proto `v0.1.145` and publish one-d20 drafts through `PublishDiceThrow`.
- [x] Wait for the server-bound unary response before local planned playback.
- [x] Keep stream and witnesses disconnected.
- [x] Obtain Kirk approval: approximately 24 ms cold and 10 ms warm publication latency.
- [x] Run full verification and commit web checkpoints `354872d` and `10a0a7d`.

### Task 6: Admit live witness plans without changing combat presentation

**Files:**
- Create focused stream/admission helpers and tests under `src/components/session/local-world-die/`.
- Modify `src/api/client.ts` only if the existing generated stream client needs a narrow adapter.

**Produces:**

```ts
interface LocalWorldDieWitnessPlan {
  readonly presentationId: string;
  readonly authoritySeq: bigint;
  readonly roller: string;
  readonly attempt: number;
  readonly initialState: LocalWorldDieRigidBodyState;
  readonly terminal: LocalWorldDiePlanTerminal;
}
```

- [ ] Write RED tests proving a live plan is accepted only for the current nonlocal authoritative player presentation with matching session, presentation ID, sequence, roller, expected attempt, schema, one-d20 body, and 32-byte local fingerprint.
- [ ] Write RED tests proving actor echo, stale attempts, malformed bodies, unknown schema, mismatch, stream failure, and cancellation produce no witness command and no authority effect.
- [ ] Implement one cancellable live-only subscription with immutable strict admission and equal-identity deduplication.
- [ ] Run focused tests and typecheck; commit the independently reviewable transport/admission seam.

### Task 7: Render one noninteractive witness body

**Files:**
- Modify `src/components/session/local-world-die/LocalWorldDieLayer.tsx` and focused tests.
- Modify `src/components/session/SessionEncounterView.tsx` and focused tests.
- Modify the smallest combat overlay surface only if a noninteractive status is needed.

- [ ] Write RED tests for witness spawn from the accepted initial state, local dynamic playback, terminal type/step application, fixed settled beat/removal, off-table removal, scope cancellation, and no tile/pointer ownership.
- [ ] Add a witness playback command to the existing world-die layer; do not create another Canvas, Physics owner, controller, or renderer.
- [ ] Compose admitted witness plans only for the current nonlocal player presentation. Leave Story/result/damage/log timing unchanged.
- [ ] Verify the actor Direct, Planned, and Published modes are behaviorally unchanged.
- [ ] Run focused/full tests, typecheck, format, lint, build, and `git diff --check`; commit the witness-render seam.

### Task 8: Two-browser visual-only Kirk gate

- [ ] Open two independent authenticated clients against the current session and healthy presentation service.
- [ ] Prove one Published actor throw yields exactly one actor body and one noninteractive witness body.
- [ ] Confirm actor stream echo does not restart or duplicate actor playback.
- [ ] Confirm stale/mismatched plans and reconnect do not replay a die.
- [ ] Record delivery-to-first-frame and terminal type/step only; do not log raw plans, transforms, velocities, auth, or result.
- [ ] Ask Kirk to judge visual delivery/replay only, explicitly noting that witness Story timing and meaningful contact checkpoints remain out of scope.
- [ ] Stop after Kirk's verdict. Do not add suspense gating, checkpoints, retry status, push, or open a web PR without the next explicit approval.
