# Persistent-Body Shared Dungeon Dice Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Preserve the shipped authority/plan/transport foundation while rebuilding production world dice around one persistent attempt-scoped Rapier body that matches the approved concept and survives pickup, planning, playback, settlement, and retry without a mount gap.

**Architecture:** A complete attempt scope freezes snapshot/fingerprint and creates one hidden kinematic body inside the existing `SessionCanvas`. Pickup becomes interactive only after projection, verified runtime, Physics world, and that body are ready. The same body is revealed during carry, frozen during raw planning/publish, switched to accepted-plan dynamic playback, and then fixed or removed; every callback carries the originating scope. Multiplayer still relays immutable plans and sparse checkpoints through the existing API without server simulation.

**Tech Stack:** React 19, TypeScript 5.8, Three.js 0.181, React Three Fiber 9, `@react-three/rapier` 2.2.0, `@dimforge/rapier3d-compat` 0.19.2, Connect-ES, proto `v0.1.145`, Vitest 4, React Testing Library, Playwright/CDP, Docker Compose.

**Spec:** `ideas/interactive-dice-tray/shared-dungeon-throws/design.md`

**Tracking:** `rpg-project#289/#303`, design PR #304, API #852/PR #853, web #837.

## Starting state

- API PR #853 is rebased/pushed at `4c6b5cada2f53d9425553e63ba015e1d04fad584`, targets `dev`, and passed `make ci-check`.
- Proto #257 is merged as `v0.1.145` with generated commit `4d2ba6a8a1d919284665a7c299f87c8f0a3ddbd6`.
- Web worktree `/home/kirk/.pi/worktrees/rpg-dnd5e-web/837-world-dice-clean` is on `feat/837-world-dice-clean` at committed head `f21178a41bc1c6a7f82a5489b20d9720849b192d` plus rejected, uncommitted Task 8 integration bytes.
- Keep these reviewed web foundations: combat settlement-only authority, strict plan/proto adapters, canonical logical snapshot/fingerprint, aggregate physical floor derivation, disposable raw Rapier loader/planner shape, pure sparse playback controller, live-only transport, one `SessionCanvas`, and legacy-tray removal.
- Rewrite the fixed schema/launch mapping, scene/body lifecycle, controller scope/readiness, and production actor integration.

## Global constraints

- Web work targets latest `origin/dev` ancestry and one PR to `dev`; do not rebase during this continuation unless `origin/dev` changes before push.
- Archive rejected uncommitted Task 8 bytes on a local-only branch before resetting the feature worktree. Never push or cherry-pick that archive.
- Concepts under `src/concepts/attack-die-3d/**` remain untouched and working. Production imports no concept module.
- `SessionCanvas` is the only production Canvas/WebGL renderer.
- One full attempt scope contains session, presentation ID, authority sequence, roller, attempt, snapshot fingerprint, render generation, and die ID.
- Actor pickup requires projection + verified runtime + Physics world + scoped body readiness. Projection-only readiness is insufficient.
- One actor body handle/mount persists through prewarm, held, planning, accepted playback, correction, and fixed beat. Retry begins only after prior removal acknowledgement and gets a new render generation/body.
- Equivalent scene/query object rerenders never replace an active attempt snapshot or clear readiness.
- Every scene callback carries its originating full scope; no callback infers ownership from current state.
- Intentional off-table, settled cleanup, semantic failure, provider/runtime failure, and scope cancellation have distinct sanitized removal reasons.
- Physics schema values match the approved concept: radius `0.275`; die friction/restitution `0.72/0.48`; damping `0.22/0.16`; floor `0.9/0.25`; walls/doors restitution `0.55`; low energy `<0.28/<1.1`; bounded assist at step 180; correction 320 ms.
- Neutral Roll produces linear velocity `{x:0,y:0.8,z:0}` and angular velocity `{x:0,y:0,z:0}`.
- Gesture release mapping remains `horizontal=0.5+speed*7.5`, `vertical=0.8+speed*1.5`, `angular=speed*18+shake*1.5`; raw samples remain local.
- Server remains a validator/relay. Do not add server physics, trajectory streaming, toolkit Story writes, or result fields to plans.
- Off-table never reveals. Result/damage/log/downed reveal opens only after corrected rendered settlement or explicit semantic failure after body removal.
- Human testing is not debugging. Do not request Kirk's gate until owned lifecycle, isolated backend, clean-browser, and performance evidence pass.
- No test or browser process may remain after its step; record PID/process cleanup.
- TDD, `npm run ci-check` before push, and no `--no-verify` remain mandatory.

---

## File structure

### Keep with focused amendments

- `src/components/session/combat-experience/{presentation,useCombatPresentation,useSessionCombatExperience}.ts`
- `src/components/session/world-dice/{types,plan,worldSnapshot,rapierLoader,playback}.ts`
- `src/api/{client,streamLogging,useSessionDiceThrows}.ts`
- `src/components/session/SessionCanvas.tsx`

### Rewrite

- `src/components/session/world-dice/physicsSchema.ts`
- `src/components/session/world-dice/planner.ts`
- `src/components/session/world-dice/WorldDiceSceneLayer.tsx`
- `src/components/session/world-dice/controllerState.ts`
- `src/components/session/world-dice/useWorldDiceController.ts`
- `src/components/session/world-dice/WorldDiceLaunchTile.tsx`
- `src/components/session/world-dice/gesture.ts`
- `src/components/session/SessionEncounterView.tsx`
- `src/components/session/combat-experience/{CombatExperience.tsx,types.ts,CombatExperience.module.css}`

### Create

- `src/components/session/world-dice/launchProfile.ts`
- `src/components/session/world-dice/lifecycleDiagnostics.ts`
- `src/components/session/world-dice/persistentBodyBoundary.test.tsx`
- `scripts/attack-die/sharedDungeonThrowEvidenceProtocol.ts`
- `scripts/attack-die/sharedDungeonThrowEvidenceProtocol.test.ts`
- `scripts/attack-die/capture-shared-dungeon-throw-evidence.mjs`
- `docs/architecture/components/session-world-dice.md`
- `docs/evidence/837-shared-dungeon-throws/README.md`

### Delete when reference scan proves unused

- `src/components/session/combat-experience/storyReveal.ts` and its test.
- Remaining ignored legacy dice props/callback compatibility in `combat-experience/types.ts` and concept fixtures that no longer represent production behavior. Do not delete shared `src/components/ui/dice/DiceTrayPresentation*`; Concepts still own it.

---

### Task 1: Preserve rejected Task 8 and restore the clean committed foundation

**Files:** No product edits on the feature branch.

**Produces:** local archive branch `archive/837-task8-rejected-persistent-body`; clean feature branch at `f21178a`; fresh SDD ledger for this amended plan.

- [ ] **Step 1: Preserve the current SDD workspace**

Rename `/home/kirk/game-dev/.superpowers/sdd/plan` to `/home/kirk/game-dev/.superpowers/sdd/plan-pre-persistent-body-2026-08-29`. Create a fresh workspace/ledger whose first line names this plan path and record the prior archive path plus amendment commit.

- [ ] **Step 2: Save a binary patch and inventory**

```bash
cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/837-world-dice-clean
git diff --binary > /home/kirk/game-dev/.superpowers/sdd/plan/rejected-task8.patch
git status --porcelain=v1 > /home/kirk/game-dev/.superpowers/sdd/plan/rejected-task8-status.txt
```

Expected: patch is non-empty and status lists every rejected Task 8 file.

- [ ] **Step 3: Commit rejected bytes on a local-only archive branch**

```bash
git switch -c archive/837-task8-rejected-persistent-body
git add -A
git commit -m "chore: preserve rejected persistent-body attempt (#837)"
ARCHIVE_SHA=$(git rev-parse HEAD)
git switch feat/837-world-dice-clean
```

Do not push the archive. Expected: feature branch returns to `f21178a...` with a clean tree; archive commit retains exact prior bytes.

- [ ] **Step 4: Verify baseline and no owned runtime remains**

```bash
test "$(git rev-parse HEAD)" = f21178a41bc1c6a7f82a5489b20d9720849b192d
git status --short
! pgrep -af '[s]hared-dice-cdp-profile|837-world-dice-clean.*[v]ite'
npm run test:run -- src/components/session/world-dice src/components/session/combat-experience
npm run typecheck
```

Expected: clean branch, focused baseline green, port 3010 stopped.

---

### Task 2: Reconcile fixed physics and launch profiles with the concept

**Files:**
- Modify `physicsSchema.ts`, `physicsSchema.test.ts`
- Modify `worldSnapshot.ts`, `worldSnapshot.test.ts`
- Modify `planner.ts`, `planner.test.ts`
- Modify `gesture.ts`, `gesture.test.ts`
- Create `launchProfile.ts`, `launchProfile.test.ts`

**Produces:**

```ts
export interface WorldDiceReleaseProfile {
  readonly direction: readonly [number, number];
  readonly speed: number;
  readonly shake: number;
  readonly spinBias: number;
}
export function worldDiceLaunchFromProfile(
  position: WorldDiceVector3,
  profile: WorldDiceReleaseProfile
): WorldDiceBodyInitial;
export function neutralWorldDiceLaunch(position: WorldDiceVector3): WorldDiceBodyInitial;
```

- [ ] **Step 1: Write RED concept-parity tests**

Pin every exact schema value from Global constraints, neutral vectors, gesture direction/speed/spin mapping, low-energy thresholds, step-180 in-bounds assist terminal, floor/wall/door material selection, and fingerprint change. Tests contain expected values from the approved spec and import no concept file.

- [ ] **Step 2: Verify RED**

```bash
npm run test:run -- src/components/session/world-dice/{physicsSchema,launchProfile,worldSnapshot,planner,gesture}.test.ts
```

Expected: failures show current radius/material/damping/threshold/neutral drift and missing launch helper.

- [ ] **Step 3: Implement one schema/profile source**

Move all die/static material facts and release mapping into `physicsSchema.ts`/`launchProfile.ts`. Gesture emits a sanitized profile-derived body at release. Neutral control calls `neutralWorldDiceLaunch`; no controller hard-coded velocities remain.

Planner classifies in-bounds settled at concept low energy or step 180, still rejects no terminal at 480, and preserves contact-before-terminal validation. Update canonical fingerprint fixture because fixed schema facts changed.

- [ ] **Step 4: Run GREEN and repeated real planner proof**

```bash
npm run test:run -- src/components/session/world-dice/{physicsSchema,launchProfile,plan,worldSnapshot,rapierLoader,planner,gesture}.test.ts
for run in $(seq 1 10); do npm run test:run -- src/components/session/world-dice/planner.test.ts || exit 1; done
npm run typecheck
```

Record planner duration distribution under low worker count, terminal kind/step, contacts, and disposal. Target plan p95 <250 ms.

- [ ] **Step 5: Commit**

```bash
git add src/components/session/world-dice
git commit -m "fix(dice): align dungeon physics with approved concept (#837)"
```

---

### Task 3: Rewrite the scene layer around one persistent scoped body

**Files:**
- Rewrite `WorldDiceSceneLayer.tsx` and test
- Modify `playback.ts`/test only if adapter scope must be explicit
- Create `lifecycleDiagnostics.ts`/test
- Create `persistentBodyBoundary.test.tsx`
- Modify `SessionCanvas.test.tsx` only for integration seam proof

**Produces:**

```ts
export interface WorldDiceAttemptScope {
  readonly key: string;
  readonly session: string;
  readonly presentationId: string;
  readonly authoritySeq: bigint;
  readonly roller: string;
  readonly attempt: number;
  readonly fingerprintHex: string;
  readonly renderGeneration: number;
  readonly dieId: 'attack-d20';
}

export type WorldDiceSceneCommand =
  | Readonly<{ kind: 'prewarm'; scope: WorldDiceAttemptScope; reset: WorldDiceBodyInitial }>
  | Readonly<{ kind: 'held'; scope: WorldDiceAttemptScope; body: WorldDiceHeldBody }>
  | Readonly<{ kind: 'planning'; scope: WorldDiceAttemptScope; body: WorldDiceHeldBody }>
  | Readonly<{ kind: 'playback'; scope: WorldDiceAttemptScope; plan: WorldDicePlan; authoritativeFace: number }>
  | Readonly<{ kind: 'fixed'; scope: WorldDiceAttemptScope }>;

export interface WorldDiceReadiness {
  readonly projection: boolean;
  readonly runtime: boolean;
  readonly world: boolean;
  readonly body: boolean;
}
```

All callbacks receive `scope` explicitly.

- [ ] **Step 1: Write lifecycle RED tests**

Cover delayed projection, delayed runtime source, equivalent snapshot rerender, prewarm readiness, one body handle/mount across every actor phase, no body gap, prop-driven kinematic→dynamic→fixed transitions, correction on same handle, exactly one removal reason, retry new generation, and delayed old callbacks ignored.

The boundary test records mount/unmount/handle identities from a behavior-complete adapter, not only React nodes.

- [ ] **Step 2: Verify RED**

```bash
npm run test:run -- src/components/session/world-dice/{WorldDiceSceneLayer,persistentBodyBoundary,lifecycleDiagnostics,playback}.test*
```

Expected: current source-gated world unmount/readiness split fails.

- [ ] **Step 3: Implement persistent world/body ownership**

Mount projection bridge and one Physics world for `prewarm`. Once provider source exists, mount one hidden kinematic body and retain it by full scope/die key. Full readiness fires only when all four booleans are true. Handoff changes visibility/pose, not mount identity. Planning freezes; playback applies accepted initial state to the same handle and starts fixed stepping; settlement corrects/fixes the same handle.

Scope cancellation/off-table/failure disables then unmounts once, emits reason, and acknowledges removal. Never conditionally erase a handed-off body because runtime/source props rerender.

- [ ] **Step 4: Run GREEN and scheduling proof**

```bash
npm run test:run -- src/components/session/world-dice/{WorldDiceSceneLayer,persistentBodyBoundary,lifecycleDiagnostics,playback}.test*
npm run test:run -- src/components/session/SessionCanvas.test.tsx src/components/ui/dice/{RuntimeDiceMesh,dungeonDiceRuntime,diceSettlementResolver}.test*
npm run typecheck
```

Assert prewarm/held/planning/fixed schedule zero continuous frame callbacks; playback/correction schedule one controlled callback and stop at terminal.

- [ ] **Step 5: Commit**

```bash
git add src/components/session/world-dice src/components/session/SessionCanvas.test.tsx
git commit -m "feat(dice): keep one body through each dungeon throw (#837)"
```

---

### Task 4: Rewrite controller, readiness, retry, and tile around the full attempt scope

**Files:**
- Rewrite `controllerState.ts`/test
- Rewrite `useWorldDiceController.ts`/test
- Modify `WorldDiceLaunchTile.tsx`/test
- Modify `gesture.ts`/test as required by frozen handoff snapshot
- Modify `types.ts`

**Produces:** one pure state machine with phases `idle | prewarming | armed | held | planning | waiting-plan | playing | removing | retry | fixed | fallback | settled` and explicit full scope on every physical phase.

- [ ] **Step 1: Write RED scope/readiness tests**

Pin one frozen snapshot per attempt, equivalent object churn ignored, full readiness required for pickup, Roll queued during prewarm, direct gesture blocked before readiness, snapshot frozen at handoff, one planner/publish operation, equal response/echo idempotent, conflict fail-removes, terminal/removal ordering, retry only after removal, new generation/body, stale callbacks ignored in every phase, witness no interactive fallback, and scope reset cleanup.

- [ ] **Step 2: Verify RED**

```bash
npm run test:run -- src/components/session/world-dice/{controllerState,useWorldDiceController,WorldDiceLaunchTile,gesture}.test*
```

- [ ] **Step 3: Implement one scope owner**

Create the attempt snapshot/fingerprint/scope once when authority arms and prerequisites exist. Emit `prewarm` immediately. The tile shows preparation plus active Roll; pickup target appears only at full readiness. A pre-ready Roll records queued neutral intent and launches once body-ready. All scene callbacks compare their supplied scope to state scope; no callback reads current scope to invent provenance.

Off-table transitions to removing, waits acknowledgement, increments attempt/generation, freezes a new current snapshot, and prewarms before retry interaction. Failure removes first and only then exposes actor fallback/auto-settles witness.

- [ ] **Step 4: Run GREEN and full feature regression**

```bash
npm run test:run -- src/components/session/world-dice
npm run test:run -- src/components/session/combat-experience src/components/session/SessionCanvas.test.tsx
npm run typecheck
```

- [ ] **Step 5: Commit**

```bash
git add src/components/session/world-dice
git commit -m "feat(dice): scope dungeon throw ownership end to end (#837)"
```

---

### Task 5: Integrate the actor path and produce owned evidence before human testing

**Files:**
- Modify `SessionEncounterView.tsx`/test
- Modify `CombatExperience.tsx`/test, `types.ts`, CSS
- Delete dead production-only legacy helper after reference scan
- Modify/create `productionBoundary.test.ts`
- Create evidence protocol/script/docs skeleton

- [ ] **Step 1: Write RED production composition tests**

Prove one controller/transport, prewarm before pickup, control topmost in game-frame overlay, one presentation layer, no legacy drawer/concept/second Canvas, actor-only control, monster/catch-up auto behavior, weapons/camera/map preserved, and settlement-only reveal.

- [ ] **Step 2: Implement production composition**

Wire one controller above sibling map/control. Use last-good scene facts to create a scope, but never replace an active attempt snapshot on refetch. Remove stale compatibility props/dead `storyReveal` only after `rg` proves no production/reference use. Add sanitized lifecycle evidence sink guarded for development/evidence mode.

- [ ] **Step 3: Run automated gates**

```bash
npm run test:run -- src/components/session/world-dice src/components/session/SessionEncounterView.test.tsx src/components/session/combat-experience src/components/session/SessionCanvas.test.tsx
npm run test:run
npm run typecheck
npm run lint
npm run format:check
npm run build
```

- [ ] **Step 4: Run isolated backend, never shared `rpg-dev`**

```bash
cd /home/kirk/game-dev/rpg-deployment
RPG_API_HOST_PORT=8082 RPG_API_IMAGE=rpg-api:task8-4c6b5ca \
  docker compose -p rpg-dice-837 \
  -f docker-compose.local-dev.yml -f docker-compose.local-api-src.yml up -d
```

Start web on 3010 with `VITE_API_HOST=http://127.0.0.1:8082` and **without** `VITE_DEV_PLAYER_ID`. Seed/create the sandbox lobby only in this isolated compose project.

- [ ] **Step 5: Owned clean-browser proof**

Start one headless/system browser with a unique profile and recorded PID. Prove through the production route:

- topmost pickup hit target;
- full readiness before pickup;
- one body mount/handle through handoff, planning, accepted playback, correction, fixed terminal;
- one plan/publish;
- off-table removal reason and concealed retry;
- attempt 2 new generation/body and zero stale callbacks;
- damage/log after fixed face;
- planner and accepted-plan-to-frame spans meet targets; and
- no console/page/raw-payload errors.

Cleanly terminate browser/profile and isolated compose project. Verify no matching process/container remains.

- [ ] **Step 6: Commit evidence-ready actor path**

```bash
git add src scripts/attack-die docs/architecture/components/session-world-dice.md docs/evidence/837-shared-dungeon-throws/README.md
git commit -m "feat(session): integrate persistent dungeon dice actor flow (#837)"
```

Task review must be clean before Task 6.

---

### Task 6: Mandatory Kirk local-feel gate

**Files:** No code unless a rejected observation first receives a focused RED regression.

- [ ] **Step 1: Present owned evidence before URL**

Show exact branch/API SHAs, isolated stack identity, lifecycle trace summary, performance spans, process cleanup, and test counts. Start the isolated stack/browser-facing web only after Kirk agrees to test.

- [ ] **Step 2: STOP for Kirk**

Kirk verifies no old tray; honest preparation; reliable first pickup; carry/lift/release feel; no body disappearance; wall/shut-door bounce; off-table concealed retry; attempt-2 reliability; one correction; fixed-face then damage/log.

- [ ] **Step 3: Handle verdict**

If rejected, stop and return to the owning prior task through systematic debugging/TDD. Do not proceed to witnesses. If approved, record Kirk's words in evidence and continue.

---

### Task 7: Add witness stream playback without creating a second lifecycle

**Files:**
- Modify controller/state/tests
- Modify `SessionEncounterView.tsx`/test
- Modify evidence protocol/script

- [ ] **Step 1: Write RED witness tests**

Pin plan-before-event buffer, event-before-plan bounded fallback, stream/response dedupe, complete correlation/fingerprint, one door refresh/grace, witness prewarm only after accepted plan, same persistent playback path, off-table noninteractive retry status, stale reconnect fencing, and catch-up no replay.

- [ ] **Step 2: Implement one `acceptPlan` path**

Actor response/echo and witness stream call one strict callback. Witness begins at accepted playback scope (no held phases) but uses the same scene body/playback/terminal/correction/removal implementation. Failure auto-settles witness only after removal.

- [ ] **Step 3: Verify and commit**

```bash
npm run test:run -- src/api/useSessionDiceThrows.test.ts src/components/session/world-dice src/components/session/SessionEncounterView.test.tsx
npm run test:run
npm run typecheck
npm run lint
npm run format:check
git add src scripts/attack-die
git commit -m "feat(dice): share persistent dungeon throws with witnesses (#837)"
```

---

### Task 8: Two-player gate, docs, CI, PR, and review

- [ ] **Step 1: Isolated two-player owned proof**

Use isolated `rpg-dice-837` backend and two independent contexts. Capture one plan/publish, one body per context, same wall/shut-door contacts, open-door omission, off-table concealment/retry, same settled face, fallback, reconnect/no replay, and clean process teardown.

- [ ] **Step 2: Full verification**

```bash
npm run ci-check
git diff --check
git status --short --branch
```

- [ ] **Step 3: STOP for Kirk integrated gate**

Kirk drives two authenticated players only after owned proof passes. Record approval or return to TDD; never debug by repeated human retries.

- [ ] **Step 4: Final docs and commit**

Complete architecture/evidence/status/quality with exact artifacts and no overclaims. Commit `docs(dice): record persistent shared throw evidence (#837)`.

- [ ] **Step 5: Push/open PR to `dev`**

Run `npm run ci-check`, push `feat/837-world-dice-clean`, open one PR linked to #837/#303/#289 and API #853, use the UI/UX signature for `KirkDiggler`, request exactly one Copilot round, and do not report ready in the same turn.

- [ ] **Step 6: Review and completion verification**

Use `superpowers:receiving-code-review`, answer every thread, never re-request, rerun CI, then use `superpowers:verification-before-completion` and final whole-branch review. Merge order remains API #853 → web → design PR #304; Kirk alone merges.
