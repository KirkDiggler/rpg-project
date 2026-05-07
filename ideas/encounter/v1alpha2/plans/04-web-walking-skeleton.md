# rpg-dnd5e-web Walking Skeleton — Wave 2.5 Slice 2 Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Render v1alpha2 movement events end-to-end on the web. Subscribe to `StreamEncounter` from rpg-api PR #496, dispatch typed events (`SnapshotDelivered`, `EntityMoved`, `GeometryRevealed`, `EntityAppeared`, `EntityDisappeared`), update local state, render. v1alpha1 remains the source of truth for everything except movement.

**Architecture:** Strictly additive. New `useEncounterStream2` hook mounts as a sibling of the existing `useEncounterStream` in `LobbyView.tsx`. New reducer methods on `useEncounterState`. Drop only the `onMovementCompleted` and `onRoomRevealed` registrations on the v1 hook — v1 hook code is unchanged. No feature flag (local-replace workflow merges both PRs together).

**Tech Stack:** TypeScript, React 18, Connect-RPC (`@bufbuild/protobuf`), Vitest, React Testing Library, `@kirkdiggler/rpg-api-protos` (v1alpha2 + EntityDisappeared.last_known_position).

**Spec:** `rpg-project/ideas/encounter/v1alpha2/sdk-direction-web.md` (read first; this plan does not repeat the architectural reasoning).

**Coordinated rpg-api scope** (lands on `feat/494-encounter-v2-walking-skeleton` branch via dead-drop in `sessions/active.md`):
- Redis-backed v2 Repository (`internal/repositories/encounters/v2/redis.go`)
- `cmd/devseed/main.go` one-shot seeder
- Optional `DEV_SEED_FILE` startup auto-seed
- `cmd/server/server.go` swap `NewInMemory` → `NewRedis`

**Issues:** rpg-dnd5e-web#387, rpg-project#15, rpg-api#494. Followups to file: expand `SnapshotDelivered.encounter` (toolkit + protos), animation polish (post-models), `EncounterStreamProvider` refactor (when it earns its keep).

---

## File Structure

| File | Purpose | Status |
|------|---------|--------|
| `package.json` | Bump `@kirkdiggler/rpg-api-protos` to a version with v1alpha2 + EntityDisappeared.last_known_position | Modify |
| `package-lock.json` | Regenerate per CLAUDE.md proto-update protocol | Modify |
| `src/api/streamReconnect.ts` | Shared `RECONNECT_CONFIG` constants used by both v1 and v2 hooks | Create |
| `src/api/useEncounterStream.ts` | Replace local `RECONNECT_CONFIG` with import from `streamReconnect.ts` | Modify |
| `src/api/useEncounterStream2.ts` | The v2 hook — subscribes to `StreamEncounter`, dispatches typed events | Create |
| `src/api/useEncounterStream2.test.ts` | Hook lifecycle + reconnect tests with fake stream | Create |
| `src/api/encounterStream2Dispatch.ts` | Pure-function dispatch helper (switch on oneof case) | Create |
| `src/api/encounterStream2Dispatch.test.ts` | Per-event-case tests + unknown-case warning test | Create |
| `src/api/fakeEncounterStream2.ts` | Test helper: async iterator yielding typed events on demand | Create |
| `src/utils/hexCoord.ts` | `protoPositionToHex` + `hexToProtoPosition` helpers | Create |
| `src/utils/hexCoord.test.ts` | Round-trip + cube invariant tests | Create |
| `src/hooks/useEncounterState.ts` | Add `revealedHexes`, ghost flag handling, three new reducers | Modify |
| `src/hooks/useEncounterState.test.ts` | Tests for new reducer methods + transition sequences | Modify |
| `src/components/LobbyView.tsx:1163-1186` | Mount `useEncounterStream2`; drop `onMovementCompleted` + `onRoomRevealed` from v1 registration | Modify |
| `src/components/encounter/BattleMapPanel.tsx` | Read `entity.ghost` flag; layer `revealedHexes` with existing reveal logic | Modify |
| `src/components/encounter/BattleMapPanel.test.tsx` | Component integration test feeding fake-stream events end-to-end | Create |

**Type extension pattern (locked):**
- `EntityState` in v1alpha1 is a TS type intersection (`Message<"..."> & {...}`). Extend it locally as `EntityState & { ghost?: boolean }` — proto fields stay intact, ghost is a local-only field.
- `LocalEncounterState.entities` becomes `Map<string, EntityState & { ghost?: boolean }>`. Existing consumers that read proto fields are unaffected; new code reads `.ghost`.

**Coordinate transform note:**
- v1alpha2 uses cube `(x, y, z)` with `x + y + z = 0`. The web's `hexMath.ts` uses `(q, r, s)` (also cube). Mapping is 1:1: `q := x, r := y, s := z`. Wrap in helpers so a future proto field rename is one-place fix.

---

## Task 1: Bump proto package to v1alpha2 version

**Why first:** Every subsequent task imports v1alpha2 types. No v2 imports compile until this lands.

**Files:**
- Modify: `package.json`
- Modify: `package-lock.json`

- [ ] **Step 1.1: Find the right proto package version**

The rpg-api Go side already pulls v1alpha2 from a specific generated pseudo-version (per `sessions/active.md`: `v0.0.0-20260507051118-0443dec90664`, generated branch commit `0443dec90664`). The web TypeScript side installs from GitHub by tag. Find the matching tag:

Run:
```bash
gh release list -R KirkDiggler/rpg-api-protos --limit 10
```

Look for the latest tag whose `generated` branch commit corresponds to (or post-dates) `0443dec90664`. If no tagged release exists yet, pin directly to the commit:
```
"@kirkdiggler/rpg-api-protos": "github:KirkDiggler/rpg-api-protos#<commit-sha-from-generated-branch>"
```

- [ ] **Step 1.2: Verify the v1alpha2 encounter package exists in the chosen version**

Run:
```bash
gh api repos/KirkDiggler/rpg-api-protos/contents/gen/ts/dnd5e/api/v1alpha2/encounter?ref=<chosen-tag-or-sha>
```

Expected: a directory listing including `encounter_pb.ts`. If 404, the version is too old; pick a newer one.

- [ ] **Step 1.3: Update package.json + lock file**

Per `rpg-dnd5e-web/CLAUDE.md` proto-update protocol:
```bash
npm i --save github:KirkDiggler/rpg-api-protos#<chosen-tag-or-sha>
```

If npm doesn't pick up the new version (lock file caching):
```bash
rm -rf node_modules package-lock.json && npm install
```

- [ ] **Step 1.4: Verify imports compile**

Create a throwaway file `src/__proto-check.ts`:
```ts
import type {
  StreamEncounterRequestSchema,
  EncounterEvent,
} from '@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha2/encounter/encounter_pb';
console.log({ StreamEncounterRequestSchema });
```

Run:
```bash
npx tsc --noEmit src/__proto-check.ts
```

Expected: no errors. Then delete the file.

- [ ] **Step 1.5: Run full ci-check**

```bash
npm run ci-check
```

Expected: PASS. (No code uses v1alpha2 yet, so this should still build clean.)

- [ ] **Step 1.6: Commit**

```bash
git add package.json package-lock.json
git commit -m "chore(deps): bump rpg-api-protos to version with v1alpha2 encounter

Picks up v1alpha2 encounter package + EntityDisappeared.last_known_position
field. Required for wave 2.5 slice 2 (web-side StreamEncounter rendering).

No code changes — types only become consumable in subsequent commits."
```

---

## Task 2: Coordinate transform helpers

**Files:**
- Create: `src/utils/hexCoord.ts`
- Create: `src/utils/hexCoord.test.ts`

- [ ] **Step 2.1: Write the failing tests**

Create `src/utils/hexCoord.test.ts`:
```ts
import { describe, expect, it } from 'vitest';
import { hexToProtoPosition, protoPositionToHex } from './hexCoord';

// HexCoord uses (q, r, s); Position uses (x, y, z). Both cube; 1:1 mapping.

describe('protoPositionToHex', () => {
  it('maps cube position (1, -1, 0) to hex (1, -1, 0)', () => {
    const pos = { x: 1, y: -1, z: 0 };
    expect(protoPositionToHex(pos)).toEqual({ q: 1, r: -1, s: 0 });
  });

  it('preserves the cube invariant x + y + z = 0', () => {
    const cases = [
      { x: 0, y: 0, z: 0 },
      { x: 2, y: -1, z: -1 },
      { x: -3, y: 5, z: -2 },
    ];
    for (const pos of cases) {
      const hex = protoPositionToHex(pos);
      expect(hex.q + hex.r + hex.s).toBe(0);
    }
  });
});

describe('hexToProtoPosition', () => {
  it('round-trips through protoPositionToHex', () => {
    const cases = [
      { q: 0, r: 0, s: 0 },
      { q: 1, r: -1, s: 0 },
      { q: -2, r: 1, s: 1 },
      { q: 5, r: -3, s: -2 },
    ];
    for (const hex of cases) {
      const pos = hexToProtoPosition(hex);
      expect(protoPositionToHex(pos)).toEqual(hex);
    }
  });
});
```

- [ ] **Step 2.2: Run tests, expect them to fail**

```bash
npx vitest run src/utils/hexCoord.test.ts
```

Expected: FAIL with "Cannot find module './hexCoord'".

- [ ] **Step 2.3: Write the implementation**

Create `src/utils/hexCoord.ts`:
```ts
/**
 * Cube hex coordinates (q, r, s) used by src/components/hex-grid/hexMath.ts.
 * The cube invariant q + r + s = 0 holds for valid hexes.
 */
export interface HexCoord {
  q: number;
  r: number;
  s: number;
}

/**
 * v1alpha2's Position is cube (x, y, z) with the same invariant
 * (x + y + z = 0). Mapping is direct: q := x, r := y, s := z.
 *
 * Wrapped in this helper so a future proto field rename is a one-place fix.
 */
export interface ProtoPosition {
  x: number;
  y: number;
  z: number;
}

export function protoPositionToHex(pos: ProtoPosition): HexCoord {
  return { q: pos.x, r: pos.y, s: pos.z };
}

export function hexToProtoPosition(hex: HexCoord): ProtoPosition {
  return { x: hex.q, y: hex.r, z: hex.s };
}

/**
 * Stable string key for a hex coord — usable in Set<string> / Map<string, T>.
 */
export function hexKey(hex: HexCoord): string {
  return `${hex.q},${hex.r},${hex.s}`;
}
```

- [ ] **Step 2.4: Run tests, expect PASS**

```bash
npx vitest run src/utils/hexCoord.test.ts
```

Expected: PASS — all tests green.

- [ ] **Step 2.5: Commit**

```bash
git add src/utils/hexCoord.ts src/utils/hexCoord.test.ts
git commit -m "feat(utils): add hex coordinate transform helpers for v1alpha2

protoPositionToHex / hexToProtoPosition wrap the 1:1 cube mapping
between v1alpha2 Position(x,y,z) and the web's HexCoord(q,r,s).
hexKey produces a stable string for Set/Map keys.

Single point of change if proto field names ever shift."
```

---

## Task 3: Extract shared `RECONNECT_CONFIG`

**Why:** The spec requires the v2 hook to mirror v1's reconnect schedule exactly. Sharing the constants prevents drift.

**Files:**
- Create: `src/api/streamReconnect.ts`
- Modify: `src/api/useEncounterStream.ts` (replace local const, import from new file)

- [ ] **Step 3.1: Create the shared module**

Create `src/api/streamReconnect.ts`:
```ts
/**
 * Reconnect schedule shared by useEncounterStream (v1alpha1) and
 * useEncounterStream2 (v1alpha2). Single source of truth — preventing
 * drift between the two hooks.
 *
 * Schedule: 1s initial, 2x backoff multiplier, 30s cap, 10 max attempts.
 * Total backoff window if all attempts fail: ~5 minutes.
 */
export const RECONNECT_CONFIG = {
  initialDelayMs: 1000,
  maxDelayMs: 30000,
  backoffMultiplier: 2,
  maxAttempts: 10,
} as const;

export type ReconnectConfig = typeof RECONNECT_CONFIG;
```

- [ ] **Step 3.2: Update `useEncounterStream.ts` to import from the new module**

Edit `src/api/useEncounterStream.ts:83-88` (the existing local `RECONNECT_CONFIG`):

Before:
```ts
const RECONNECT_CONFIG = {
  initialDelayMs: 1000, // Start with 1 second
  maxDelayMs: 30000, // Cap at 30 seconds
  backoffMultiplier: 2, // Double each attempt
  maxAttempts: 10, // Give up after 10 attempts
};
```

After: delete the local const entirely. Add to imports near the top of the file:
```ts
import { RECONNECT_CONFIG } from './streamReconnect';
```

- [ ] **Step 3.3: Verify v1 hook still compiles + works**

```bash
npm run typecheck
npm run test src/api/
```

Expected: PASS. The v1 hook's behavior is unchanged — same constants, just imported.

- [ ] **Step 3.4: Run full ci-check**

```bash
npm run ci-check
```

Expected: PASS.

- [ ] **Step 3.5: Commit**

```bash
git add src/api/streamReconnect.ts src/api/useEncounterStream.ts
git commit -m "refactor(api): extract RECONNECT_CONFIG to shared module

Pulls the inline RECONNECT_CONFIG out of useEncounterStream.ts:83-88
into src/api/streamReconnect.ts so the upcoming useEncounterStream2
can share the schedule. Single source of truth — no drift between
the two hooks.

Behavior unchanged."
```

---

## Task 4: `useEncounterState` reducer additions

**Files:**
- Modify: `src/hooks/useEncounterState.ts`
- Modify: `src/hooks/useEncounterState.test.ts`

- [ ] **Step 4.1: Write failing tests for the new reducers**

Edit `src/hooks/useEncounterState.test.ts` to add a new `describe` block:

```ts
import { hexKey } from '../utils/hexCoord';
// ... existing imports

describe('v1alpha2 reducer additions', () => {
  describe('applyHexRevealed', () => {
    it('adds hexes to revealedHexes without dropping existing reveals', () => {
      const prev = createEmptyEncounterState();
      const after1 = applyHexRevealed(prev, [{ q: 0, r: 0, s: 0 }]);
      expect(after1.revealedHexes.has('0,0,0')).toBe(true);

      const after2 = applyHexRevealed(after1, [{ q: 1, r: -1, s: 0 }]);
      expect(after2.revealedHexes.has('0,0,0')).toBe(true);
      expect(after2.revealedHexes.has('1,-1,0')).toBe(true);
    });

    it('is idempotent on duplicate hexes', () => {
      const prev = applyHexRevealed(createEmptyEncounterState(), [
        { q: 2, r: -1, s: -1 },
      ]);
      const after = applyHexRevealed(prev, [{ q: 2, r: -1, s: -1 }]);
      expect(after.revealedHexes.size).toBe(1);
    });
  });

  describe('applyEntityAppeared', () => {
    it('adds a new entity at first-visible position with ghost cleared', () => {
      const prev = createEmptyEncounterState();
      const entity = makeTestEntity('goblin-1', { x: 3, y: -2, z: -1 });
      const after = applyEntityAppeared(prev, entity);
      const stored = after.entities.get('goblin-1');
      expect(stored).toBeDefined();
      expect(stored?.ghost).toBeFalsy();
    });

    it('clears the ghost flag on a previously-disappeared entity', () => {
      const entity = makeTestEntity('alice', { x: 0, y: 0, z: 0 });
      const withEntity = applyEntityUpdates(createEmptyEncounterState(), [
        entity,
      ]);
      const ghosted = applyEntityDisappeared(withEntity, 'alice', {
        q: 1,
        r: -1,
        s: 0,
      });
      expect(ghosted.entities.get('alice')?.ghost).toBe(true);

      const reappeared = applyEntityAppeared(
        ghosted,
        makeTestEntity('alice', { x: 5, y: -3, z: -2 })
      );
      expect(reappeared.entities.get('alice')?.ghost).toBeFalsy();
    });
  });

  describe('applyEntityDisappeared', () => {
    it('keeps entity in store, sets ghost=true, updates position to last_known', () => {
      const entity = makeTestEntity('bob', { x: 0, y: 0, z: 0 });
      const prev = applyEntityUpdates(createEmptyEncounterState(), [entity]);
      const after = applyEntityDisappeared(prev, 'bob', {
        q: 4,
        r: -2,
        s: -2,
      });
      const stored = after.entities.get('bob');
      expect(stored?.ghost).toBe(true);
      expect(stored?.position).toEqual({ x: 4, y: -2, z: -2 });
    });

    it('is a no-op if the entity is not in state (defensive)', () => {
      const prev = createEmptyEncounterState();
      const after = applyEntityDisappeared(prev, 'unknown', {
        q: 0,
        r: 0,
        s: 0,
      });
      expect(after.entities.size).toBe(0);
    });
  });

  describe('appear/disappear sequences', () => {
    it('survives appeared → moved → disappeared → appeared cleanly', () => {
      let state = createEmptyEncounterState();
      state = applyEntityAppeared(
        state,
        makeTestEntity('mover', { x: 0, y: 0, z: 0 })
      );
      expect(state.entities.get('mover')?.ghost).toBeFalsy();

      state = applyEntityPositionUpdate(state, 'mover', {
        x: 1,
        y: -1,
        z: 0,
      });
      expect(state.entities.get('mover')?.position).toEqual({
        x: 1,
        y: -1,
        z: 0,
      });

      state = applyEntityDisappeared(state, 'mover', { q: 2, r: -1, s: -1 });
      expect(state.entities.get('mover')?.ghost).toBe(true);
      expect(state.entities.get('mover')?.position).toEqual({
        x: 2,
        y: -1,
        z: -1,
      });

      state = applyEntityAppeared(
        state,
        makeTestEntity('mover', { x: 5, y: -3, z: -2 })
      );
      expect(state.entities.get('mover')?.ghost).toBeFalsy();
      expect(state.entities.get('mover')?.position).toEqual({
        x: 5,
        y: -3,
        z: -2,
      });
    });
  });
});

// Helper — same shape used elsewhere in this file's tests; copy the existing
// makeTestEntity if it exists, or write a minimal one:
function makeTestEntity(id: string, pos: Position): EntityState {
  return {
    entityId: id,
    position: pos,
    // ... whatever minimal proto fields the existing tests use
  } as EntityState;
}
```

- [ ] **Step 4.2: Run tests, expect them to fail**

```bash
npx vitest run src/hooks/useEncounterState.test.ts
```

Expected: FAIL — `applyHexRevealed`, `applyEntityAppeared`, `applyEntityDisappeared` not exported.

- [ ] **Step 4.3: Add the state shape change**

Edit `src/hooks/useEncounterState.ts`. Update `LocalEncounterState`:

```ts
export interface LocalEncounterState {
  encounterId: string;
  dungeonId: string;
  /** All entities, keyed by entity ID. v2 may set entity.ghost on LoS loss. */
  entities: Map<string, EntityState & { ghost?: boolean }>;
  rooms: Map<string, RoomLayout>;
  currentRoomId: string;
  revealedRoomIds: string[];
  /** v1alpha2-revealed hexes (per-hex granularity). UI renders revealed if either revealedHexes covers it OR the room is in revealedRoomIds. */
  revealedHexes: Set<string>;
  combat: CombatState | null;
  doors: Map<string, DoorInfo>;
  dungeonState: DungeonState;
  roomsCleared: number;
}
```

Update `createEmptyEncounterState` to initialize `revealedHexes: new Set()`.

Update `applySnapshotToState` to preserve any existing `revealedHexes` (snapshots from v1 don't carry v2 reveal data — pass through `prev.revealedHexes` if applicable, OR initialize empty since snapshot always wipes; verify pattern with the existing `applySnapshotToState` semantics — current code at line 61-76 builds a fresh state, so initialize to `new Set()`).

- [ ] **Step 4.4: Add the three new reducer functions**

After the existing `mergeEntityPosition` function, add:

```ts
import { hexKey, type HexCoord, hexToProtoPosition } from '../utils/hexCoord';

/**
 * Add hexes to the v1alpha2 revealedHexes set.
 * Idempotent — duplicates collapse via Set semantics.
 * Exported for testing.
 */
export function applyHexRevealed(
  prev: LocalEncounterState,
  hexes: HexCoord[]
): LocalEncounterState {
  const next = new Set(prev.revealedHexes);
  for (const h of hexes) {
    next.add(hexKey(h));
  }
  return { ...prev, revealedHexes: next };
}

/**
 * Add or revive an entity in local state. Clears any ghost flag.
 * Used for v1alpha2 EntityAppeared events.
 * Exported for testing.
 */
export function applyEntityAppeared(
  prev: LocalEncounterState,
  entity: EntityState
): LocalEncounterState {
  const newEntities = new Map(prev.entities);
  // Clone-and-clear-ghost — proto fields preserved, ghost reset
  newEntities.set(entity.entityId, { ...entity, ghost: false });
  return { ...prev, entities: newEntities };
}

/**
 * Mark an entity as ghosted at its last-known position. Entity stays in
 * the store; render code applies reduced opacity / desaturated tint.
 * No-op if the entity is not present.
 * Exported for testing.
 */
export function applyEntityDisappeared(
  prev: LocalEncounterState,
  entityId: string,
  lastKnown: HexCoord
): LocalEncounterState {
  const existing = prev.entities.get(entityId);
  if (!existing) return prev;
  const newEntities = new Map(prev.entities);
  newEntities.set(entityId, {
    ...existing,
    ghost: true,
    position: hexToProtoPosition(lastKnown),
  });
  return { ...prev, entities: newEntities };
}
```

- [ ] **Step 4.5: Add the methods to `UseEncounterStateResult` and the hook**

Update the interface:
```ts
export interface UseEncounterStateResult {
  state: LocalEncounterState;
  applySnapshot: (proto: EncounterStateData) => void;
  applyEntityUpdates: (updates: EntityState[]) => void;
  applyEntityPositionUpdate: (entityId: string, position: Position) => void;
  applyCombatState: (combat: CombatState) => void;
  reset: () => void;
  // v1alpha2 additions
  applyHexRevealed: (hexes: HexCoord[]) => void;
  applyEntityAppeared: (entity: EntityState) => void;
  applyEntityDisappeared: (entityId: string, lastKnown: HexCoord) => void;
}
```

Add three new `useCallback` blocks inside `useEncounterState`:
```ts
const applyHexRevealedCb = useCallback((hexes: HexCoord[]) => {
  setState((prev) => applyHexRevealed(prev, hexes));
}, []);

const applyEntityAppearedCb = useCallback((entity: EntityState) => {
  setState((prev) => applyEntityAppeared(prev, entity));
}, []);

const applyEntityDisappearedCb = useCallback(
  (entityId: string, lastKnown: HexCoord) => {
    setState((prev) => applyEntityDisappeared(prev, entityId, lastKnown));
  },
  []
);
```

Add them to the returned object alongside the existing methods.

- [ ] **Step 4.6: Run tests, expect them to PASS**

```bash
npx vitest run src/hooks/useEncounterState.test.ts
```

Expected: PASS — all new tests + all existing tests green.

- [ ] **Step 4.7: Run typecheck**

```bash
npm run typecheck
```

Expected: PASS.

- [ ] **Step 4.8: Commit**

```bash
git add src/hooks/useEncounterState.ts src/hooks/useEncounterState.test.ts
git commit -m "feat(useEncounterState): add v1alpha2 reducers + revealedHexes + ghost flag

Three new reducer methods on the unified entity store:
- applyHexRevealed (per-hex reveal, additive Set, idempotent)
- applyEntityAppeared (add/revive entity, clear ghost)
- applyEntityDisappeared (mark ghost, update to last_known position;
  no-op if entity absent)

State shape gains revealedHexes: Set<string> (key = 'q,r,s').
EntityState extended locally with optional ghost field via type
intersection — proto fields preserved, ghost is local-only.

UI render rule: hex is visible if revealedRoomIds covers it OR
revealedHexes contains it (||, no merge logic).

Tests cover: idempotent reveal, ghost transitions, appear→move→
disappear→appear sequence, defensive no-op on unknown entity."
```

---

## Task 5: Dispatch helper

**Files:**
- Create: `src/api/encounterStream2Dispatch.ts`
- Create: `src/api/encounterStream2Dispatch.test.ts`

- [ ] **Step 5.1: Confirm v1alpha2 EncounterEvent oneof case names**

The proto's `EncounterEvent` carries a oneof with one case per event type. Check the generated TS for the exact case names:

```bash
grep -A40 "export type EncounterEvent " node_modules/@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha2/encounter/encounter_pb.ts
```

Expected case names (verify against actual output): `snapshotDelivered`, `entityMoved`, `geometryRevealed`, `entityAppeared`, `entityDisappeared`. The string discriminator on the oneof is `event.event.case`.

If the generated names differ (e.g. `snapshot_delivered` vs `snapshotDelivered`), use what the generated TS actually exports — adjust the rest of this task accordingly.

- [ ] **Step 5.2: Write failing tests**

Create `src/api/encounterStream2Dispatch.test.ts`:
```ts
import { describe, expect, it, vi } from 'vitest';
import type { EncounterEvent } from '@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha2/encounter/encounter_pb';
import { dispatchEncounterStream2Event, type EncounterStream2Options } from './encounterStream2Dispatch';

function makeEvent<K extends string, V>(caseName: K, value: V): EncounterEvent {
  return {
    event: { case: caseName, value },
    // Other EncounterEvent fields (eventId, deliveredAt) — minimal shape;
    // tests are checking dispatch only, not full proto validity
  } as unknown as EncounterEvent;
}

describe('dispatchEncounterStream2Event', () => {
  it('routes snapshotDelivered to onSnapshotDelivered', () => {
    const onSnapshotDelivered = vi.fn();
    const options: EncounterStream2Options = { onSnapshotDelivered };
    const event = makeEvent('snapshotDelivered', { encounter: undefined });
    dispatchEncounterStream2Event(event, options);
    expect(onSnapshotDelivered).toHaveBeenCalledTimes(1);
  });

  it('routes entityMoved to onEntityMoved', () => {
    const onEntityMoved = vi.fn();
    const options: EncounterStream2Options = { onEntityMoved };
    const event = makeEvent('entityMoved', {
      entityId: 'a',
      actualPath: [{ x: 0, y: 0, z: 0 }],
    });
    dispatchEncounterStream2Event(event, options);
    expect(onEntityMoved).toHaveBeenCalledTimes(1);
  });

  it('routes geometryRevealed to onGeometryRevealed', () => {
    const onGeometryRevealed = vi.fn();
    const options: EncounterStream2Options = { onGeometryRevealed };
    const event = makeEvent('geometryRevealed', { hexes: [] });
    dispatchEncounterStream2Event(event, options);
    expect(onGeometryRevealed).toHaveBeenCalledTimes(1);
  });

  it('routes entityAppeared to onEntityAppeared', () => {
    const onEntityAppeared = vi.fn();
    const options: EncounterStream2Options = { onEntityAppeared };
    const event = makeEvent('entityAppeared', {
      entity: { entityId: 'g', position: { x: 1, y: -1, z: 0 } },
    });
    dispatchEncounterStream2Event(event, options);
    expect(onEntityAppeared).toHaveBeenCalledTimes(1);
  });

  it('routes entityDisappeared to onEntityDisappeared', () => {
    const onEntityDisappeared = vi.fn();
    const options: EncounterStream2Options = { onEntityDisappeared };
    const event = makeEvent('entityDisappeared', {
      entityId: 'g',
      lastKnownPosition: { x: 2, y: -2, z: 0 },
    });
    dispatchEncounterStream2Event(event, options);
    expect(onEntityDisappeared).toHaveBeenCalledTimes(1);
  });

  it('logs a warning for unknown event cases without throwing', () => {
    const warn = vi.spyOn(console, 'warn').mockImplementation(() => {});
    const event = makeEvent('someUnknownCase' as 'snapshotDelivered', {});
    expect(() => dispatchEncounterStream2Event(event, {})).not.toThrow();
    expect(warn).toHaveBeenCalled();
    warn.mockRestore();
  });

  it('is a no-op when no callback is registered for the event type', () => {
    const event = makeEvent('entityMoved', {
      entityId: 'x',
      actualPath: [],
    });
    expect(() => dispatchEncounterStream2Event(event, {})).not.toThrow();
  });
});
```

- [ ] **Step 5.3: Run tests, expect them to fail**

```bash
npx vitest run src/api/encounterStream2Dispatch.test.ts
```

Expected: FAIL — module doesn't exist.

- [ ] **Step 5.4: Write the implementation**

Create `src/api/encounterStream2Dispatch.ts`:
```ts
import type {
  EncounterEvent,
  EntityAppearedEvent,
  EntityDisappearedEvent,
  EntityMovedEvent,
  GeometryRevealedEvent,
  SnapshotDeliveredEvent,
} from '@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha2/encounter/encounter_pb';

/**
 * Per-event-type callbacks for the v1alpha2 encounter stream.
 * Mirrors the v1 hook's options shape (one optional callback per event type).
 *
 * NOTE: onSnapshotDelivered is called every time the stream opens (initial
 * connect + every reconnect). The payload's `encounter` field is empty in
 * slice 1 — DO NOT apply it as state. Treat as a stream-up sync barrier.
 */
export interface EncounterStream2Options {
  onSnapshotDelivered?: (event: SnapshotDeliveredEvent) => void;
  onEntityMoved?: (event: EntityMovedEvent) => void;
  onGeometryRevealed?: (event: GeometryRevealedEvent) => void;
  onEntityAppeared?: (event: EntityAppearedEvent) => void;
  onEntityDisappeared?: (event: EntityDisappearedEvent) => void;
}

/**
 * Dispatches a typed v1alpha2 EncounterEvent to the appropriate callback.
 * Pure function; no side effects beyond callback invocation + a console.warn
 * on unknown event cases (gap to file as a toolkit/proto issue).
 */
export function dispatchEncounterStream2Event(
  event: EncounterEvent,
  options: EncounterStream2Options
): void {
  const payload = event.event;
  switch (payload.case) {
    case 'snapshotDelivered':
      options.onSnapshotDelivered?.(payload.value);
      break;
    case 'entityMoved':
      options.onEntityMoved?.(payload.value);
      break;
    case 'geometryRevealed':
      options.onGeometryRevealed?.(payload.value);
      break;
    case 'entityAppeared':
      options.onEntityAppeared?.(payload.value);
      break;
    case 'entityDisappeared':
      options.onEntityDisappeared?.(payload.value);
      break;
    default:
      // Unknown event type — toolkit/proto gap. Log + continue so the stream
      // doesn't tear down. File as an issue if seen in playtest.
      console.warn(
        '[useEncounterStream2] unknown event case:',
        (payload as { case?: string }).case
      );
  }
}
```

- [ ] **Step 5.5: Run tests, expect PASS**

```bash
npx vitest run src/api/encounterStream2Dispatch.test.ts
```

Expected: PASS.

- [ ] **Step 5.6: Commit**

```bash
git add src/api/encounterStream2Dispatch.ts src/api/encounterStream2Dispatch.test.ts
git commit -m "feat(api): add v1alpha2 encounter event dispatch helper

Pure switch on event.event.case → typed callbacks for the five v1alpha2
encounter event types: SnapshotDelivered, EntityMoved, GeometryRevealed,
EntityAppeared, EntityDisappeared.

Unknown cases log a warning and continue (don't tear down the stream).
Doc-string on onSnapshotDelivered flags the empty-payload-in-slice-1
contract so consumers don't accidentally apply empty as state.

Tests cover all five cases + unknown + no-callback-registered."
```

---

## Task 6: `useEncounterStream2` hook

**Files:**
- Create: `src/api/fakeEncounterStream2.ts` (test helper)
- Create: `src/api/useEncounterStream2.ts`
- Create: `src/api/useEncounterStream2.test.ts`

- [ ] **Step 6.1: Build the fake-stream test helper**

Create `src/api/fakeEncounterStream2.ts`:
```ts
import type { EncounterEvent } from '@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha2/encounter/encounter_pb';

/**
 * Test helper: an async iterable that yields events from a controllable queue.
 * Use to simulate a server-streaming gRPC response in tests without real network.
 *
 * Pattern:
 *   const stream = createFakeStream();
 *   stream.push(makeEvent('snapshotDelivered', {}));
 *   stream.push(makeEvent('entityMoved', { ... }));
 *   stream.close();   // signals end-of-stream
 *   stream.error(new Error('boom'));   // simulates transport error
 */
export interface FakeStream {
  iterator: AsyncIterable<EncounterEvent>;
  push: (event: EncounterEvent) => void;
  close: () => void;
  error: (err: Error) => void;
}

export function createFakeStream(): FakeStream {
  const queue: EncounterEvent[] = [];
  let closed = false;
  let pendingError: Error | null = null;
  let resolve: ((v: void) => void) | null = null;

  const wakeup = () => {
    if (resolve) {
      const r = resolve;
      resolve = null;
      r();
    }
  };

  const iterator: AsyncIterable<EncounterEvent> = {
    [Symbol.asyncIterator]() {
      return {
        async next(): Promise<IteratorResult<EncounterEvent>> {
          while (queue.length === 0 && !closed && !pendingError) {
            await new Promise<void>((r) => {
              resolve = r;
            });
          }
          if (pendingError) {
            const e = pendingError;
            pendingError = null;
            throw e;
          }
          if (queue.length > 0) {
            return { value: queue.shift()!, done: false };
          }
          return { value: undefined, done: true };
        },
      };
    },
  };

  return {
    iterator,
    push(event) {
      queue.push(event);
      wakeup();
    },
    close() {
      closed = true;
      wakeup();
    },
    error(err) {
      pendingError = err;
      wakeup();
    },
  };
}
```

- [ ] **Step 6.2: Write failing tests for the hook**

Create `src/api/useEncounterStream2.test.ts`:
```ts
import { act, renderHook, waitFor } from '@testing-library/react';
import { describe, expect, it, vi } from 'vitest';
import type { EncounterEvent } from '@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha2/encounter/encounter_pb';
import { createFakeStream } from './fakeEncounterStream2';
import { useEncounterStream2 } from './useEncounterStream2';

function makeEvent(caseName: string, value: unknown): EncounterEvent {
  return { event: { case: caseName, value } } as unknown as EncounterEvent;
}

// Mock the gRPC client so the hook's call resolves to our fake stream
vi.mock('./client', () => {
  const fake = createFakeStream();
  return {
    encounterClientV2: {
      streamEncounter: vi.fn(() => fake.iterator),
    },
    __fake: fake, // expose for tests to drive
  };
});

describe('useEncounterStream2', () => {
  it('transitions to connected after the first SnapshotDelivered', async () => {
    const onSnapshotDelivered = vi.fn();
    const { result } = renderHook(() =>
      useEncounterStream2('enc-1', 'alice', { onSnapshotDelivered })
    );

    // Connecting initially
    expect(result.current.connectionState).toBe('connecting');

    // Push the snapshot
    const { __fake } = await import('./client');
    act(() => {
      __fake.push(makeEvent('snapshotDelivered', { encounter: undefined }));
    });

    await waitFor(() => {
      expect(result.current.connectionState).toBe('connected');
    });
    expect(onSnapshotDelivered).toHaveBeenCalledTimes(1);
  });

  it('dispatches subsequent events through their typed callbacks', async () => {
    const onEntityMoved = vi.fn();
    const { result } = renderHook(() =>
      useEncounterStream2('enc-1', 'alice', { onEntityMoved })
    );

    const { __fake } = await import('./client');
    act(() => {
      __fake.push(makeEvent('snapshotDelivered', {}));
    });
    await waitFor(() =>
      expect(result.current.connectionState).toBe('connected')
    );

    act(() => {
      __fake.push(
        makeEvent('entityMoved', {
          entityId: 'alice',
          actualPath: [{ x: 0, y: 0, z: 0 }],
        })
      );
    });
    await waitFor(() => {
      expect(onEntityMoved).toHaveBeenCalledTimes(1);
    });
  });

  it('does not transition to connected before SnapshotDelivered arrives', async () => {
    const { result } = renderHook(() =>
      useEncounterStream2('enc-1', 'alice', {})
    );
    // Wait a tick — should still be connecting
    await new Promise((r) => setTimeout(r, 10));
    expect(result.current.connectionState).toBe('connecting');
  });

  it('logs and continues on unknown event cases (no tear-down)', async () => {
    const warn = vi.spyOn(console, 'warn').mockImplementation(() => {});
    const onEntityMoved = vi.fn();
    const { result } = renderHook(() =>
      useEncounterStream2('enc-1', 'alice', { onEntityMoved })
    );

    const { __fake } = await import('./client');
    act(() => {
      __fake.push(makeEvent('snapshotDelivered', {}));
      __fake.push(makeEvent('totallyUnknownCase', {}));
      __fake.push(
        makeEvent('entityMoved', { entityId: 'a', actualPath: [] })
      );
    });

    await waitFor(() => {
      expect(onEntityMoved).toHaveBeenCalledTimes(1);
      expect(result.current.connectionState).toBe('connected');
    });
    expect(warn).toHaveBeenCalled();
    warn.mockRestore();
  });

  it('aborts cleanly on unmount', async () => {
    const { unmount, result } = renderHook(() =>
      useEncounterStream2('enc-1', 'alice', {})
    );
    expect(result.current.connectionState).toBe('connecting');
    unmount();
    // No assertions on stream content; passing the test = no unhandled rejection
  });

  it('handles encounterId === null by staying idle', () => {
    const { result } = renderHook(() => useEncounterStream2(null, 'alice', {}));
    expect(result.current.connectionState).toBe('idle');
  });

  // Reconnect tests use fake timers — verify backoff schedule
  it('reconnects with exponential backoff on stream error', async () => {
    vi.useFakeTimers();
    const { result } = renderHook(() =>
      useEncounterStream2('enc-1', 'alice', {})
    );
    const { __fake } = await import('./client');

    act(() => {
      __fake.push(makeEvent('snapshotDelivered', {}));
    });
    await vi.waitFor(() =>
      expect(result.current.connectionState).toBe('connected')
    );

    act(() => {
      __fake.error(new Error('connection lost'));
    });
    await vi.waitFor(() =>
      expect(result.current.connectionState).toBe('disconnected')
    );

    // Advance timers by initialDelayMs (1000)
    act(() => {
      vi.advanceTimersByTime(1000);
    });
    await vi.waitFor(() =>
      expect(result.current.connectionState).toBe('connecting')
    );

    vi.useRealTimers();
  });
});
```

- [ ] **Step 6.3: Run tests, expect them to fail**

```bash
npx vitest run src/api/useEncounterStream2.test.ts
```

Expected: FAIL — `useEncounterStream2` not exported.

- [ ] **Step 6.4: Add the v2 client**

Verify `src/api/client.ts` has (or add) a `encounterClientV2` for the v1alpha2 service. Pattern matches the existing `encounterClient`:

```ts
import { createClient } from '@connectrpc/connect';
import { EncounterService as EncounterServiceV2 } from '@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha2/encounter/encounter_pb';

export const encounterClientV2 = createClient(EncounterServiceV2, transport);
```

(The exact import path / service identifier depends on the generated TS — verify against the proto package once Task 1's bump lands.)

- [ ] **Step 6.5: Write the hook implementation**

Create `src/api/useEncounterStream2.ts`:
```ts
import { create } from '@bufbuild/protobuf';
import {
  StreamEncounterRequestSchema,
  type EncounterEvent,
} from '@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha2/encounter/encounter_pb';
import { useEffect, useRef, useState } from 'react';
import { encounterClientV2 } from './client';
import {
  dispatchEncounterStream2Event,
  type EncounterStream2Options,
} from './encounterStream2Dispatch';
import { RECONNECT_CONFIG } from './streamReconnect';

type ConnectionState =
  | 'idle'
  | 'connecting'
  | 'connected'
  | 'disconnected'
  | 'error';

interface UseEncounterStream2Result {
  connectionState: ConnectionState;
  error: Error | null;
}

/**
 * Subscribes to the v1alpha2 StreamEncounter RPC. Sibling of useEncounterStream
 * (v1alpha1) — runs in parallel for slice 2; v1 still owns lobby/combat events.
 *
 * Lifecycle:
 *   1. encounterId set → open stream, state='connecting'
 *   2. First message MUST be SnapshotDelivered → state='connected', fire callback
 *   3. Subsequent events dispatched via encounterStream2Dispatch
 *   4. Stream end / error → state='disconnected' → exponential backoff reconnect
 *
 * Slice 1 contract: SnapshotDelivered.encounter is empty. The hook acknowledges
 * the message (transitions to 'connected') but does NOT apply payload as state.
 * Treat as a stream-up sync barrier.
 *
 * Reconnect: shared RECONNECT_CONFIG with v1 hook (1s → 30s, 10 attempts).
 */
export function useEncounterStream2(
  encounterId: string | null,
  playerId: string,
  options: EncounterStream2Options
): UseEncounterStream2Result {
  const [connectionState, setConnectionState] =
    useState<ConnectionState>('idle');
  const [error, setError] = useState<Error | null>(null);

  const optionsRef = useRef(options);
  optionsRef.current = options;

  const retryCountRef = useRef(0);
  const retryTimeoutRef = useRef<NodeJS.Timeout | undefined>(undefined);
  const abortControllerRef = useRef<AbortController | undefined>(undefined);

  useEffect(() => {
    if (!encounterId) {
      setConnectionState('idle');
      return;
    }

    const connect = async () => {
      setConnectionState('connecting');
      setError(null);
      abortControllerRef.current = new AbortController();

      try {
        const request = create(StreamEncounterRequestSchema, {
          encounterId,
          playerId,
        });
        const stream = encounterClientV2.streamEncounter(request, {
          signal: abortControllerRef.current.signal,
        });

        let sawFirstSnapshot = false;
        for await (const event of stream as AsyncIterable<EncounterEvent>) {
          if (!sawFirstSnapshot) {
            // First message must be SnapshotDelivered (broker contract).
            // Dispatch it as normal — the dispatcher routes to onSnapshotDelivered.
            // Then transition to 'connected'.
            dispatchEncounterStream2Event(event, optionsRef.current);
            sawFirstSnapshot = true;
            setConnectionState('connected');
            retryCountRef.current = 0;
            continue;
          }
          dispatchEncounterStream2Event(event, optionsRef.current);
        }

        // Stream closed by server — schedule reconnect
        scheduleReconnect();
      } catch (err) {
        if (abortControllerRef.current?.signal.aborted) {
          return; // intentional abort, not an error
        }
        console.error('[useEncounterStream2] stream error:', err);
        scheduleReconnect();
      }
    };

    const scheduleReconnect = () => {
      if (retryCountRef.current >= RECONNECT_CONFIG.maxAttempts) {
        setConnectionState('error');
        setError(new Error('Max reconnection attempts reached'));
        return;
      }
      setConnectionState('disconnected');
      const delay = Math.min(
        RECONNECT_CONFIG.initialDelayMs *
          Math.pow(RECONNECT_CONFIG.backoffMultiplier, retryCountRef.current),
        RECONNECT_CONFIG.maxDelayMs
      );
      retryCountRef.current++;
      retryTimeoutRef.current = setTimeout(connect, delay);
    };

    connect();

    return () => {
      abortControllerRef.current?.abort();
      clearTimeout(retryTimeoutRef.current);
    };
  }, [encounterId, playerId]);

  return { connectionState, error };
}
```

- [ ] **Step 6.6: Run tests, expect them to PASS**

```bash
npx vitest run src/api/useEncounterStream2.test.ts
```

Expected: PASS — all hook tests green.

- [ ] **Step 6.7: Run typecheck + ci-check**

```bash
npm run typecheck
npm run ci-check
```

Expected: PASS.

- [ ] **Step 6.8: Commit**

```bash
git add src/api/fakeEncounterStream2.ts src/api/useEncounterStream2.ts \
        src/api/useEncounterStream2.test.ts src/api/client.ts
git commit -m "feat(api): add useEncounterStream2 for v1alpha2 StreamEncounter

New hook subscribes to the v1alpha2 encounter stream and dispatches
typed events via encounterStream2Dispatch. Sibling to useEncounterStream
(v1alpha1) — runs in parallel; v1 still owns lobby/combat lifecycle.

Lifecycle:
- First message MUST be SnapshotDelivered → transition to 'connected'
- Empty SnapshotDelivered.encounter (slice 1) is acknowledged but not
  applied as state (stream-up sync barrier)
- Subsequent events dispatched to per-type callbacks
- Stream error → exponential backoff (shared RECONNECT_CONFIG with v1)

Tests cover: connect-on-snapshot, event dispatch, unknown-case continue,
unmount abort, idle on null encounterId, reconnect schedule."
```

---

## Task 7: Wire `LobbyView` — mount v2 hook, drop v1 movement callbacks

**Files:**
- Modify: `src/components/LobbyView.tsx:1163-1186` (region around the existing `useEncounterStream` call)

- [ ] **Step 7.1: Locate the existing v1 hook registration**

Open `src/components/LobbyView.tsx` and find the `useEncounterStream(encounterId, playerId, {...})` call at line 1163. Note which callbacks are currently passed (line 1168 `onRoomRevealed`, line 1176 `onMovementCompleted` — these are the two we drop).

- [ ] **Step 7.2: Drop v1 movement-related registrations**

Remove (or comment out with a TODO referencing slice 3) the `onRoomRevealed` and `onMovementCompleted` properties from the v1 hook's options. The handler functions (`handleRoomRevealed`, `handleMovementCompleted`) and the v1 hook code itself stay — only the registrations go.

- [ ] **Step 7.3: Add the v2 hook alongside**

After the existing `useEncounterStream(...)` call, add:

```tsx
useEncounterStream2(encounterId, playerId, {
  onSnapshotDelivered: (event) => {
    // v1alpha2 stream barrier — payload empty in slice 1; just log
    console.log('[v2] snapshot delivered, deliveredAt:', event.deliveredAt);
  },
  onEntityMoved: (event) => {
    // Teleport-to-final per spec; ignore intermediate path hexes in slice 2
    const last = event.actualPath[event.actualPath.length - 1];
    if (!last) return;
    encounterState.applyEntityPositionUpdate(event.entityId, last);
  },
  onGeometryRevealed: (event) => {
    encounterState.applyHexRevealed(
      event.hexes.map(protoPositionToHex)
    );
  },
  onEntityAppeared: (event) => {
    if (!event.entity) return;
    encounterState.applyEntityAppeared(event.entity);
  },
  onEntityDisappeared: (event) => {
    if (!event.lastKnownPosition) return;
    encounterState.applyEntityDisappeared(
      event.entityId,
      protoPositionToHex(event.lastKnownPosition)
    );
  },
});
```

Add imports near the top of the file:
```ts
import { useEncounterStream2 } from '../api/useEncounterStream2';
import { protoPositionToHex } from '../utils/hexCoord';
```

- [ ] **Step 7.4: Verify no regression on v1 callbacks**

Run the existing LobbyView-related tests:
```bash
npx vitest run src/hooks/useEncounterState.test.ts src/hooks/useDungeonMap.test.ts
```

Expected: PASS — no existing test should break. (LobbyView itself doesn't have a unit test today; integration test in Task 9 covers it.)

- [ ] **Step 7.5: Run full ci-check**

```bash
npm run ci-check
```

Expected: PASS — types compile, build succeeds, all tests green.

- [ ] **Step 7.6: Commit**

```bash
git add src/components/LobbyView.tsx
git commit -m "feat(LobbyView): mount useEncounterStream2; drop v1 movement callbacks

- Register useEncounterStream2 alongside the existing v1 hook
- Wire EntityMoved → applyEntityPositionUpdate (teleport to last hex)
- Wire GeometryRevealed → applyHexRevealed
- Wire EntityAppeared/Disappeared → respective new reducers
- onSnapshotDelivered logs only (empty payload in slice 1)
- Drop onMovementCompleted + onRoomRevealed registrations from v1 hook
  (v2 owns these now). v1 hook code is unchanged.

Slice 3 will delete the now-orphaned handleMovementCompleted /
handleRoomRevealed functions and the v1 callback definitions."
```

---

## Task 8: `BattleMapPanel` — ghost rendering + revealedHexes layering

**Files:**
- Modify: `src/components/encounter/BattleMapPanel.tsx`

- [ ] **Step 8.1: Identify the entity render path**

Open `BattleMapPanel.tsx`. Find where entities from `encounterEntities` are rendered to the hex grid (the per-entity render component, likely `MediumHumanoid` or a wrapper). Note the prop that controls material/opacity (the existing pattern uses shader uniforms — `MediumHumanoid.tsx:271-275` shows the `useFrame` time uniform pattern).

- [ ] **Step 8.2: Add a ghost prop / styling**

Read each entity's `.ghost` flag (now optional on the local extended type). Pass to the entity render component:

```tsx
{Array.from(encounterEntities.values()).map((entity) => (
  <EntityRenderer
    key={entity.entityId}
    entity={entity}
    ghost={entity.ghost ?? false}
  />
))}
```

In the entity renderer, when `ghost` is true, apply reduced opacity (~0.4) and a desaturated tint. The exact mechanism depends on the existing rendering pipeline:
- If the entity is rendered via `MediumHumanoid` with shader uniforms, add a `ghostMode: boolean` uniform that the existing `AdvancedCharacterShader` switches on (or piggyback on an existing uniform if appropriate).
- If a simpler `<mesh>` with `<meshStandardMaterial>` is in use, set `transparent` + `opacity={ghost ? 0.4 : 1}` and a `color={ghost ? grayTint : normalTint}`.

Pick whichever path requires the least invasive change. **Verify with a brief read of `MediumHumanoid.tsx` and `AdvancedCharacterShader.ts` before writing the code** — slice 2's discipline is "smallest change, no asset additions."

- [ ] **Step 8.3: Layer revealedHexes with revealedRoomIds in fog/visibility logic**

Find the existing fog-of-war / hex visibility check (likely in a render-time predicate that consumes `revealedRoomIds`). Update to also consult `revealedHexes`:

```ts
function isHexRevealed(hex: HexCoord, state: LocalEncounterState): boolean {
  if (state.revealedHexes.has(hexKey(hex))) return true;
  // Existing: check if hex is in a revealed room
  return state.revealedRoomIds.includes(roomIdForHex(hex));
}
```

(Adjust to fit whatever the existing reveal predicate looks like; the shape may differ. The OR is the only addition.)

- [ ] **Step 8.4: Manual smoke check**

Run `npm run dev` and load the existing app (no v2 stream connected — slice 1 hasn't merged yet). Verify the v1 path still renders the battle map without regression. No `entity.ghost` will be set (since v1 doesn't set it), so the rendering should look identical.

- [ ] **Step 8.5: Run ci-check**

```bash
npm run ci-check
```

Expected: PASS.

- [ ] **Step 8.6: Commit**

```bash
git add src/components/encounter/BattleMapPanel.tsx \
        src/components/hex-grid/  # if shader / renderer files modified
git commit -m "feat(BattleMapPanel): render ghost entities + v2 revealedHexes

- Read entity.ghost flag; apply ~40% opacity + desaturated tint
  via existing shader/material pipeline
- Layer revealedHexes (v1alpha2 per-hex reveal) on top of existing
  revealedRoomIds — hex is visible if either covers it
- No new components, no new asset additions

Slice 2 verification path: component test in next commit feeds fake
stream events, asserts ghost rendering at last_known_position."
```

---

## Task 9: Component integration test — fake stream end-to-end

**Files:**
- Create: `src/components/encounter/BattleMapPanel.test.tsx`

This task validates the full slice 2 dispatch chain through a real React render: fake stream → hook → reducer → component. It's the closest thing to the playtest that doesn't need a backend.

- [ ] **Step 9.1: Write the integration test**

Create `src/components/encounter/BattleMapPanel.test.tsx`:
```tsx
import { act, render, waitFor } from '@testing-library/react';
import { describe, expect, it, vi } from 'vitest';
import type { EncounterEvent } from '@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/v1alpha2/encounter/encounter_pb';
import { createFakeStream } from '../../api/fakeEncounterStream2';

// NOTE: this test mounts a thin wrapper around BattleMapPanel that
// owns the v2 hook + state, mirroring LobbyView's wiring. The wrapper
// is test-only — see harness below.

function makeEvent(caseName: string, value: unknown): EncounterEvent {
  return { event: { case: caseName, value } } as unknown as EncounterEvent;
}

vi.mock('../../api/client', () => {
  const fake = createFakeStream();
  return {
    encounterClientV2: { streamEncounter: vi.fn(() => fake.iterator) },
    __fake: fake,
  };
});

// Test wrapper that mirrors LobbyView's v2 wiring
function TestHarness({ encounterId }: { encounterId: string }) {
  // Use the real hooks the way LobbyView does
  // (Implementation: copy the LobbyView wiring snippet into this harness)
  // ... renders <BattleMapPanel encounterEntities={state.entities} ... />
  return null; // placeholder — flesh out per actual hook wiring
}

describe('BattleMapPanel — v1alpha2 integration', () => {
  it('renders entity at the last hex of EntityMoved.actual_path', async () => {
    const { getByTestId } = render(<TestHarness encounterId="enc-1" />);
    const { __fake } = await import('../../api/client');

    act(() => {
      __fake.push(makeEvent('snapshotDelivered', {}));
    });

    // Seed an entity via v1alpha1 path (or via test setup)
    // Then push EntityMoved
    act(() => {
      __fake.push(
        makeEvent('entityMoved', {
          entityId: 'alice',
          actualPath: [
            { x: 0, y: 0, z: 0 },
            { x: 1, y: -1, z: 0 },
            { x: 2, y: -2, z: 0 },
          ],
        })
      );
    });

    await waitFor(() => {
      const aliceEl = getByTestId('entity-alice');
      // Assert the rendered position matches { x: 2, y: -2, z: 0 }
      // (specific assertion depends on how BattleMapPanel exposes positions
      // — data-attribute on the entity DOM element is the cleanest path)
      expect(aliceEl).toHaveAttribute('data-q', '2');
      expect(aliceEl).toHaveAttribute('data-r', '-2');
    });
  });

  it('renders entity as ghost at last_known_position on EntityDisappeared', async () => {
    const { getByTestId } = render(<TestHarness encounterId="enc-1" />);
    const { __fake } = await import('../../api/client');

    // Seed entity, then push appear/disappear sequence
    act(() => {
      __fake.push(makeEvent('snapshotDelivered', {}));
      __fake.push(
        makeEvent('entityAppeared', {
          entity: {
            entityId: 'goblin',
            position: { x: 1, y: -1, z: 0 },
          },
        })
      );
      __fake.push(
        makeEvent('entityDisappeared', {
          entityId: 'goblin',
          lastKnownPosition: { x: 3, y: -2, z: -1 },
        })
      );
    });

    await waitFor(() => {
      const goblinEl = getByTestId('entity-goblin');
      expect(goblinEl).toHaveAttribute('data-ghost', 'true');
      expect(goblinEl).toHaveAttribute('data-q', '3');
    });
  });

  it('reveals hexes from GeometryRevealed', async () => {
    const { queryByTestId } = render(<TestHarness encounterId="enc-1" />);
    const { __fake } = await import('../../api/client');

    act(() => {
      __fake.push(makeEvent('snapshotDelivered', {}));
      __fake.push(
        makeEvent('geometryRevealed', {
          hexes: [
            { x: 5, y: -3, z: -2 },
            { x: 6, y: -3, z: -3 },
          ],
        })
      );
    });

    await waitFor(() => {
      // Hex at (5, -3, -2) should now be marked revealed (no fog overlay)
      expect(queryByTestId('hex-fog-5,-3,-2')).toBeNull();
      expect(queryByTestId('hex-fog-6,-3,-3')).toBeNull();
    });
  });

  it('appear → move → disappear → appear sequence settles cleanly', async () => {
    const { getByTestId } = render(<TestHarness encounterId="enc-1" />);
    const { __fake } = await import('../../api/client');

    act(() => {
      __fake.push(makeEvent('snapshotDelivered', {}));
      __fake.push(
        makeEvent('entityAppeared', {
          entity: { entityId: 'mover', position: { x: 0, y: 0, z: 0 } },
        })
      );
    });
    await waitFor(() => {
      expect(getByTestId('entity-mover')).toHaveAttribute('data-ghost', 'false');
    });

    act(() => {
      __fake.push(
        makeEvent('entityMoved', {
          entityId: 'mover',
          actualPath: [
            { x: 0, y: 0, z: 0 },
            { x: 1, y: -1, z: 0 },
          ],
        })
      );
    });
    await waitFor(() => {
      expect(getByTestId('entity-mover')).toHaveAttribute('data-q', '1');
    });

    act(() => {
      __fake.push(
        makeEvent('entityDisappeared', {
          entityId: 'mover',
          lastKnownPosition: { x: 2, y: -1, z: -1 },
        })
      );
    });
    await waitFor(() => {
      expect(getByTestId('entity-mover')).toHaveAttribute('data-ghost', 'true');
      expect(getByTestId('entity-mover')).toHaveAttribute('data-q', '2');
    });

    act(() => {
      __fake.push(
        makeEvent('entityAppeared', {
          entity: { entityId: 'mover', position: { x: 5, y: -3, z: -2 } },
        })
      );
    });
    await waitFor(() => {
      expect(getByTestId('entity-mover')).toHaveAttribute('data-ghost', 'false');
      expect(getByTestId('entity-mover')).toHaveAttribute('data-q', '5');
    });
  });
});
```

- [ ] **Step 9.2: Wire test-only data attributes**

For the assertions above to work, `BattleMapPanel`'s entity render path needs `data-testid`, `data-q`, `data-r`, `data-ghost` attributes. Add them in Task 8 (or here if you didn't earlier). Pattern:

```tsx
<div
  data-testid={`entity-${entity.entityId}`}
  data-q={entity.position.x}
  data-r={entity.position.y}
  data-ghost={String(entity.ghost ?? false)}
>
  <EntityRenderer ... />
</div>
```

Same approach for hex fog: `data-testid={hex-fog-${q},${r},${s}}` rendered only when the hex is NOT revealed.

- [ ] **Step 9.3: Run tests, expect PASS**

```bash
npx vitest run src/components/encounter/BattleMapPanel.test.tsx
```

Expected: PASS — all four scenarios green.

- [ ] **Step 9.4: Run full test suite + ci-check**

```bash
npm run ci-check
```

Expected: PASS.

- [ ] **Step 9.5: Commit**

```bash
git add src/components/encounter/BattleMapPanel.test.tsx \
        src/components/encounter/BattleMapPanel.tsx
git commit -m "test(BattleMapPanel): integration test for v1alpha2 event dispatch

End-to-end test through fake stream → hook → reducer → component:
- EntityMoved teleports entity to last hex of actual_path
- EntityDisappeared renders entity as ghost at last_known_position
- GeometryRevealed clears fog on the revealed hexes
- appear → move → disappear → appear sequence settles cleanly

Closest thing to the playtest gate without a backend. Adds
data-testid / data-q / data-r / data-ghost attributes to the
entity render for assertion."
```

---

## Task 10: Local end-to-end smoke + playtest gate

**Prerequisites** (rpg-api session must have completed first):
- rpg-api PR #496 branch has the four dead-dropped additions:
  - `internal/repositories/encounters/v2/redis.go`
  - `cmd/devseed/main.go`
  - Optional `DEV_SEED_FILE` startup hook
  - `cmd/server/server.go` swap to `NewRedis`

If those aren't yet present, **stop and ping the API session** per the `sessions/active.md` handoff protocol before proceeding.

- [ ] **Step 10.1: Bring up rpg-api with Redis-backed v2**

In one terminal:
```bash
cd ~/personal/rpg-api && git checkout feat/494-encounter-v2-walking-skeleton
go build ./...   # confirm clean build
redis-cli ping   # verify redis is up; if not: redis-server &

# One-time seed
go run ./cmd/devseed > seed.json
redis-cli -x SET enc:v2:dev-encounter EX 86400 < seed.json

# Verify the seed
redis-cli GET enc:v2:dev-encounter | head -c 200
# Expected: a JSON blob containing the encounter shape

# Start the API
AUTH_DEV_MODE=true make run &
scripts/run-local-with-envoy.sh &
```

- [ ] **Step 10.2: Bring up the web**

In another terminal:
```bash
cd ~/personal/rpg-dnd5e-web
npm run dev
# Vite serves on http://localhost:3001
```

- [ ] **Step 10.3: Single-browser smoke (gate before two-browser)**

Open `http://localhost:3001?encounterId=dev-encounter&playerId=alice` in one browser. Open browser devtools console. Verify:

- v2 stream connects (look for `[v2] snapshot delivered, deliveredAt: ...` log)
- `connectionState` is `'connected'` (inspect via React DevTools or add a temporary status overlay)
- Battle map renders the seeded entities at expected positions
- Click a hex to move alice
- Movement event arrives via v2 stream — entity teleports to the new hex
- `redis-cli GET enc:v2:dev-encounter` shows alice's updated position

If any of the above fails, follow the dead-drop protocol in `sessions/active.md` to file the gap on rpg-api PR #496.

- [ ] **Step 10.4: Two-browser playtest (the wave gate)**

- Browser 1: `http://localhost:3001?encounterId=dev-encounter&playerId=alice`
- Browser 2: `http://localhost:3001?encounterId=dev-encounter&playerId=bob`

Wait for both to connect. Move alice. Verify:
- Browser 1: alice teleports to clicked hex
- Browser 2: alice teleports to the same hex (broker fanned out the EntityMoved event)
- No client-side filtering occurred (no `roomId` / `currentRoomId` checks gating the render)
- `redis-cli MONITOR` shows v2 Get/Save round-trips

Move bob. Same verification in reverse.

- [ ] **Step 10.5: Document the playtest result**

Post a comment on `KirkDiggler/rpg-project#15` with:
- Date / time
- What was tested (the wave-2.5 sentence)
- What worked
- Any gaps that surfaced (and links to filed issues / PR commits)
- Sign-off statement: wave goal verified, ready to merge both PRs

- [ ] **Step 10.6: Mark slice 2 acceptance**

If all the above passes:
- Web slice 2 PR is ready for merge
- rpg-api PR #496 (with the seeded-Redis additions) is ready for merge
- Both merge together (local-replace workflow per rpg-project#16)
- rpg-project#15 closes after both merge
- Update `sessions/active.md`: remove the `## Pending API addition` section (its purpose is done)

If gaps surfaced that prevent the wave gate, declare a checkpoint per `feedback_checkpoint_over_rabbit_hole`: file issues for adjacent bugs, commit what works, and re-evaluate scope.

---

## Slice 2 Acceptance Gate (matches spec)

- ✅ `npm run ci-check` clean (format + lint + typecheck + build + tests)
- ✅ All component-level tests green (Tasks 2, 4, 5, 6, 9)
- ✅ Two-browser local playtest passes the wave-2.5 sentence (Task 10.4)
- ✅ v1alpha1 hook code unchanged; only consumer registrations dropped for movement-specific callbacks (Task 7)
- ✅ Proto package bumped, lock file committed (Task 1)
- ✅ rpg-project#15 sign-off log posted (Task 10.5)

When all six are checked, the slice is done.
