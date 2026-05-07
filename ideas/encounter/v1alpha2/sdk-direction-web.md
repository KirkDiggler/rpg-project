# Encounter SDK Direction — rpg-dnd5e-web side (Wave 2.5 design, slice 2)

**Status:** Design approved 2026-05-07; ready for spec-document-reviewer + writing-plans.

**Companion docs:**
- `sdk-direction.md` — toolkit-side encounter SDK design (full architectural rationale)
- `sdk-direction-rpgapi.md` — rpg-api slice 1 design (the wire shape this slice consumes)
- `plans/03-rpgapi-walking-skeleton.md` — rpg-api slice 1 plan (executed; rpg-api PR #496)
- `rpg-toolkit/docs/architecture/components/encounter.md` — toolkit component doc

**Wave:** [#11 Chapter 1: Architecture Honesty — Wave 2.5: Player events ▶ Movement](https://github.com/users/KirkDiggler/projects/10)

**Tracker issues:**
- [rpg-project#15](https://github.com/KirkDiggler/rpg-project/issues/15) — wave umbrella + sign-off (closes on playtest)
- [rpg-api#494](https://github.com/KirkDiggler/rpg-api/issues/494) — rpg-api wiring (slice 1, complete on PR #496)
- [rpg-dnd5e-web#387](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/387) — this slice's tracker

---

## Wave goal (the playtest sentence)

**Two players in one encounter, one moves, the broker delivers per-player projected events to each — and the web renders the move from each player's perspective with no client-side filtering.**

The wave closes when that scenario is verified on a local two-browser playtest. Web slice 2 is the renderer that closes it.

---

## Sequencing within slice 2

Single PR on rpg-dnd5e-web. Strictly additive — every line of code shipped is new code; no v1alpha1 paths are deleted. Slice 3 (post-wave) is where v1alpha1 movement deletion happens.

**Coordinated sibling work** (lands on rpg-api PR #496's branch via the dead-drop in `sessions/active.md`):
- `internal/repositories/encounters/v2/redis.go` — Redis-backed `Repository` impl
- `cmd/devseed/main.go` — one-shot encounter seeder
- Optional `DEV_SEED_FILE` startup auto-seed
- `cmd/server/server.go` swap `NewInMemory` → `NewRedis`

Both PRs (rpg-api #496 with the additions, rpg-dnd5e-web slice 2) merge together via the local-replace workflow ([rpg-project#16](https://github.com/KirkDiggler/rpg-project/issues/16)). The deployed app never has a window where the web is on v2 but rpg-api isn't.

---

## Architecture

### 1. Coexistence shape — strictly additive

The web is **strictly additive** in slice 2. We add a second gRPC subscription alongside the existing v1alpha1 stream; the v1alpha1 hook stays untouched in code; consumers stop registering the *movement-specific* v1alpha1 callbacks.

```
LobbyView.tsx
  ├── useEncounterStream(...)        // v1alpha1, UNCHANGED — lobby/combat lifecycle
  │     onMovementCompleted: NOT REGISTERED  (v2 owns this)
  │     onRoomRevealed:      NOT REGISTERED  (v2's GeometryRevealed replaces it)
  │     onMonsterTurnCompleted, onAttackResolved, ...: still registered
  │
  └── useEncounterStream2(...)       // v1alpha2, NEW — encounter-scoped events only
        onSnapshotDelivered, onEntityMoved, onGeometryRevealed,
        onEntityAppeared, onEntityDisappeared
```

**Why no feature flag:** the local-replace workflow merges both PRs together. There's no staging window where v2 is wired but rpg-api hasn't shipped it. The flag would be ceremony.

### 2. Mount point

`useEncounterStream2` mounts in `LobbyView.tsx`, sibling to the existing `useEncounterStream`. Same `encounterId` / `playerId` plumbing, both lifecycles tied to the same component lifetime. Reasoning:
- Reuses existing encounter-id source (no new context, no new prop drilling)
- Same mount/unmount semantics as v1 — easier to reason about parity
- Slice 3 cleanup is mechanical: drop the v1 movement callbacks, keep the rest

A `<EncounterStreamProvider>` refactor (own both streams, expose via context) is a sensible later move once the dual-hook pattern shows real friction. Not load-bearing for slice 2.

### 3. Target end-state — drop-in callback-bag replacement

Each future wave migrates more event types from v1alpha1 to v1alpha2. The migration unit is **one event type at a time**: change which hook the consumer registers `on<X>` against, drop the v1 callback definition once nothing registers it, drop the rpg-api emission. Eventually v1alpha1's hook is deleted entirely; v2's is the only one left.

The v2 hook ends up *simpler* than the v1 hook it replaces because the snapshot arrives in-stream rather than via a separate RPC (no buffer-during-sync, no `lastEventId` ULID reconciliation, no `getEncounterHistory` path).

### 4. Proto package bump

`package.json` currently pins `@kirkdiggler/rpg-api-protos#v0.1.86`. Slice 2 bumps to a version that has the v1alpha2 encounter package + `EntityDisappeared.last_known_position`. Lock file regenerated per `rpg-dnd5e-web/CLAUDE.md` proto-update protocol.

### 5. Files touched in slice 2

**New:**
- `src/api/useEncounterStream2.ts` — the v2 hook
- `src/api/encounterStream2Dispatch.ts` — typed dispatch helper (mirrors v1alpha1's pattern, kept separate for testability)
- Tests for both above

**Modified:**
- `src/components/LobbyView.tsx` — mount the v2 hook, wire callbacks
- `src/hooks/useEncounterState.ts` — accept v2 event deltas
- `src/components/encounter/BattleMapPanel.tsx` — render ghost entities (`EntityDisappeared`), consume v2-revealed hexes
- `package.json` + `package-lock.json` — proto bump

**Unchanged in slice 2:**
- `src/api/useEncounterStream.ts` (v1alpha1 hook stays exactly as-is)

---

## Stream lifecycle & snapshot semantics

The v2 hook's lifecycle is structurally simpler than v1alpha1's because the snapshot arrives in-stream rather than via a separate RPC.

### Hook surface

```ts
useEncounterStream2(encounterId: string | null, playerId: string, options: {
  onSnapshotDelivered?: (event: SnapshotDeliveredEvent) => void;
  onEntityMoved?: (event: EntityMovedEvent) => void;
  onGeometryRevealed?: (event: GeometryRevealedEvent) => void;
  onEntityAppeared?: (event: EntityAppearedEvent) => void;
  onEntityDisappeared?: (event: EntityDisappearedEvent) => void;
}): { connectionState: ConnectionState; error: Error | null }

type ConnectionState = 'idle' | 'connecting' | 'connected' | 'disconnected' | 'error';
```

### Lifecycle flow

```
on encounterId change:
  1. abort current subscription (if any)
  2. open StreamEncounter(encounterId, playerId)        // gRPC server-streaming
  3. state := 'connecting'
  4. await first message → MUST be SnapshotDelivered
       - call options.onSnapshotDelivered?.(event)        // no-op in slice 1; payload empty
       - state := 'connected'
  5. loop:
       await next message
       dispatch by event.event.case → typed callback
  6. on stream end / error / abort:
       - if intentional abort (cleanup): return
       - else: state := 'disconnected'; schedule reconnect
```

### Reconnect

Mirror v1alpha1's exponential backoff: 1s initial, 2x multiplier, 30s cap, 10 max attempts. **Share constants by extracting `RECONNECT_CONFIG` from `useEncounterStream.ts:83-88` into `src/api/streamReconnect.ts` and importing from both hooks** — single source of truth, no drift. On reconnect, server sends a fresh `SnapshotDelivered` first, then resumes deltas.

### Snapshot semantics — what `onSnapshotDelivered` does in slice 2

The proto field `SnapshotDelivered.encounter` is **empty** in slice 1 (toolkit `Snapshot` shape doesn't map yet — flagged as a followup). The hook treats `SnapshotDelivered` as a **stream-up sync barrier**:

- Fires every time the stream opens (initial + reconnects).
- Hook acknowledges it by transitioning to `'connected'`.
- Consumer's `onSnapshotDelivered` callback may log it (the rpg-api side stamps `delivered_at` from `h.now()`, useful for stream-health diagnostics).
- Payload is *not* applied as state. Don't wipe local state, don't reset positions. The doc-string on the callback flags this so a future slice doesn't mistake "empty" for "real empty."

When a future slice expands `SnapshotDelivered.encounter`, the same callback gets a real payload and the consumer applies it. Slice 2 wiring doesn't need to anticipate that shape.

**Why this is cleaner than v1alpha1's two-call pattern:** the toolkit broker subscribes the player to the encounter channel *before* sending the snapshot. Any toolkit event firing in the subscribe-then-snapshot window is queued by the broker and arrives after the snapshot, in order. The v2 hook's contract — "you are connected once `SnapshotDelivered` arrives" — preserves that ordering for free. v1alpha1 had to solve this client-side via `lastEventId` ULID comparison; v2 just inherits the broker's guarantee.

### Error discipline

The hook never returns `(undefined, error)` mid-stream. Unknown `event.event.case` → log warning, continue (matches `useEncounterStream.ts:155`). Transport errors → transition to `'disconnected'`, let backoff handle it.

### What this hook does NOT do

- No `getEncounterHistory` analogue — slice 2 has no event-replay-on-reconnect (deferred per the SDK design's "catch-up policy" entry; v1alpha1's history fetch handles whatever the v2 stream missed during disconnect for the v1-owned event types).
- No buffer-during-sync — irrelevant; snapshot arrives in-stream.
- No offline buffering beyond what the broker holds.

---

## Event dispatch & rendering

Switch on `event.event.case` (the proto oneof discriminator). All five event types map to existing rendering primitives or near-trivial extensions of them.

### Per-event behavior

| Event | Hook callback | State mutation | Render result |
|---|---|---|---|
| `SnapshotDelivered` | `onSnapshotDelivered?` | None (payload empty in slice 1; transition to `'connected'`) | Stream-up signal; consumers may log |
| `EntityMoved` | `onEntityMoved` | `applyEntityPositionUpdate(entityId, last(actual_path))` — reuse existing v1 pattern | Entity teleports to final hex (matches v1 behavior) |
| `GeometryRevealed` | `onGeometryRevealed` | Add `hexes[]` to `revealedHexes: Set<HexKey>` on encounter state | Those hexes become rendered/non-fog |
| `EntityAppeared` | `onEntityAppeared` | `applyEntityAppeared({id, kind, position, ...})` — add or revive entity, clear any `ghost` flag | Full-opacity entity at the first-visible hex |
| `EntityDisappeared` | `onEntityDisappeared` | `applyEntityDisappeared(entityId, last_known_position)` — keep entity in store, set `ghost: true`, set `position` to `last_known_position` | Entity rendered at last-known hex, ~40% opacity, desaturated tint |

### `useEncounterState` additions

Three new reducer methods on the existing hook (matches the existing pattern at `src/hooks/useEncounterState.ts:107-144`):

```ts
applyHexRevealed(hexes: HexCoord[]): void
applyEntityAppeared(entity: EntityState): void
applyEntityDisappeared(entityId: string, lastKnown: HexCoord): void
```

State shape gains:
- `revealedHexes: Set<string>` (key = `"q,r,s"`) — separate from existing v1 `revealedRooms`. UI renders a hex as revealed if **either** set covers it (no merge logic; just an `||` at render time).
- Entity record gains `ghost: boolean` (set by `applyEntityDisappeared`, cleared by `applyEntityAppeared`). Don't add timestamp / fade-time fields speculatively — slice 2 doesn't use them, future fade polish adds what it needs.

### Coordinate transform

v1alpha2's `Position` is cube `(x, y, z)` with invariant `x + y + z = 0`. The web's hex math (`src/components/hex-grid/hexMath.ts`) uses `(q, r, s)`. Mapping is 1:1: `q := x, r := y, s := z`. Wrap in helpers (one place to fix if proto field names shift, table-tested round-trip):

```ts
export function protoPositionToHex(pos: Position): HexCoord
export function hexToProtoPosition(h: HexCoord): Position
```

### Animation policy — slice 2 teleports

`applyEntityPositionUpdate` writes the final hex (`actual_path[actual_path.length - 1]`); no per-step tween, no path animation. Matches v1alpha1's existing behavior at `src/hooks/useEncounterState.ts:108-118`. The full `actual_path` array remains on the event object the callback receives — slice 2 just doesn't read past `last(...)`. Don't store `actual_path[]` in `useEncounterState`; future slices that want path-aware features (animation, combat-log breadcrumbs) add storage when they need it.

Animation lands when the new animated character models arrive (separate polish slice, future wave). The proto carries the path; rendering it animated is purely a client-side enhancement.

### Visibility transitions — ghost rendering

`EntityDisappeared` sets `entity.ghost = true` and updates `entity.position` to `last_known_position`. `BattleMapPanel`'s entity rendering reads the flag and applies reduced opacity (~40%) plus a desaturated tint. Existing rendering pipeline (`MediumHumanoid` and family) already supports per-entity material modifications via shader uniforms.

`EntityAppeared` clears the ghost flag and sets `entity.position` to the new visible hex; renders full opacity.

Marker lifetime: cleared on `EntityAppeared` (LoS regained) or encounter end (existing reset path). No TTL, no fade-over-time logic — toolkit's "last-known" semantics is "until something better is known," and we render that literally.

### `LobbyView` wiring

```ts
const v1 = useEncounterStream(encounterId, playerId, {
  // ALL existing callbacks EXCEPT:
  // onMovementCompleted — stop registering (v2 owns this)
  // onRoomRevealed     — stop registering (v2's GeometryRevealed replaces it; v1's was broken per #380 anyway)
  onMonsterTurnCompleted: ...,
  onAttackResolved: ...,
  onCombatStarted: ...,
  // ...etc, unchanged
});

const v2 = useEncounterStream2(encounterId, playerId, {
  onSnapshotDelivered: () => { /* log only */ },
  onEntityMoved: (e) => state.applyEntityPositionUpdate(e.entityId, last(e.actualPath)),
  onGeometryRevealed: (e) => state.applyHexRevealed(e.hexes),
  onEntityAppeared: (e) => state.applyEntityAppeared(e.entity),
  onEntityDisappeared: (e) => state.applyEntityDisappeared(e.entityId, e.lastKnownPosition),
});
```

Both `connectionState` values are exposed for future UI use (e.g., a small "v2 stream: disconnected" banner during reconnect). Slice 2 doesn't surface them.

---

## Test strategy

### Test layers

| Layer | Tool | What it proves |
|---|---|---|
| **Coord transform** | `vitest`, table-driven | `protoPositionToHex` ↔ `hexToProtoPosition` round-trip; cube invariant `x+y+z=0` preserved |
| **`useEncounterState` reducers** | `vitest`, pure functions | `applyHexRevealed` adds without dropping prior reveals; `applyEntityAppeared` clears ghost; `applyEntityDisappeared` keeps entity at `lastKnown` with `ghost:true`; appear→disappear→appear sequences settle |
| **Hook lifecycle** | `vitest` + RTL, fake stream injected | First message must be `SnapshotDelivered` to transition to `'connected'`; `onSnapshotDelivered` fires even with empty payload; subsequent typed events fire matching callbacks; unknown event-cases log + continue (don't tear down stream); `encounterId` change reconnects; abort honored on unmount |
| **Reconnect** | `vitest`, fake stream simulating disconnect | Backoff schedule matches v1 (1s → 30s, max 10); state transitions through `'disconnected'` → `'connecting'` → `'connected'` per reconnect; reconnect re-receives `SnapshotDelivered` and proceeds without wiping state |
| **Component** | `vitest` + RTL, fake stream feeds typed events | `EntityMoved` → entity at last hex of `actual_path`; `EntityAppeared` → full opacity at new hex; `EntityDisappeared` → ghost styling at `last_known_position`; `GeometryRevealed` → those hexes pass visibility check; `appeared → moved → moved → disappeared → appeared` settles cleanly |

The existing v1alpha1 hook (`useEncounterStream.ts`) has no tests of its own — coverage is on the consumer side. Slice 2 fixes that for v2: TDD up front, hook-level tests ship with the hook.

### Fake-stream pattern

A small test helper that returns an async iterator yielding typed events on demand — same shape as the gRPC client returns. Inject in place of `encounterClient.streamEncounter` via a constructor arg or a test-only module override. Reusable for future v2 event-type tests as more events migrate.

### Local end-to-end (slice 2 gate) — Redis-backed repo path

**rpg-api side** (lands on `feat/494-encounter-v2-walking-skeleton` via `sessions/active.md` dead-drop):

- `internal/repositories/encounters/v2/redis.go` — Redis-backed Repository impl
- `cmd/devseed/main.go` — one-shot encounter seeder (2 players, mutual LoS, single room)
- Optional `DEV_SEED_FILE=./seed.json` startup auto-seed
- `cmd/server/server.go` swap `NewInMemory()` → `NewRedis(redisClient, 24*time.Hour)`
- Broker's `InMemoryTransport` stays — single-process, pub/sub buys nothing

**Playtest workflow:**

```bash
# 1. One-time seed (per fresh redis instance)
cd ~/personal/rpg-api && git checkout feat/494-encounter-v2-walking-skeleton
go run ./cmd/devseed > seed.json
redis-cli -x SET enc:v2:dev-encounter EX 86400 < seed.json

# 2. Bring up the stack
redis-server &  # if not already running
AUTH_DEV_MODE=true make run &
scripts/run-local-with-envoy.sh &  # grpc-web bridge

# 3. Web
cd ~/personal/rpg-dnd5e-web
npm i  # picks up the rpg-api-protos bump
npm run dev  # vite on :3001

# 4. Two browsers
# Browser 1: http://localhost:3001?encounterId=dev-encounter&playerId=alice
# Browser 2: http://localhost:3001?encounterId=dev-encounter&playerId=bob
```

Both subscribe to `StreamEncounter`, both receive `SnapshotDelivered`, alice clicks a hex to move, both see `EntityMoved` and the entity teleports to the destination. Wave gate met.

**Debug visibility (the actual win of the Redis path):**
- `redis-cli GET enc:v2:dev-encounter` → live encounter state, including PerceptionView per player
- `redis-cli MONITOR` while playing → every Get/Save round-trip
- Edit the JSON in-place to test edge scenarios — no rebuild required

### Wave gate (rpg-project#15)

Closes when the **two-browser playtest passes**: two players, one encounter, mutual LoS, one moves, both browsers show the move at the correct hex with no `roomId` / `currentRoomId` gating in the v2 codepath. Playtest log goes on issue #15.

If real API gaps surface mid-playtest, dead-drop them on rpg-api PR #496 per `sessions/active.md` protocol. Don't open follow-up PRs — fixes go on the same `feat/494-encounter-v2-walking-skeleton` branch.

### What's NOT a gate

- Asymmetric LoS rendering (verified at the rpg-api bufconn level via `TestMovementSlicePerViewerProjection_AsymmetricLoS`; web component test against fake stream covers it)
- Animation of `actual_path` (deferred)
- Reconnect under real network drop (covered by unit test, not playtest)

---

## Slice 2 acceptance gate

- `npm run ci-check` clean (format + lint + typecheck + build + tests)
- All five test layers green
- Two-browser local playtest passes the wave-2.5 sentence (rpg-project#15 sign-off log)
- v1alpha1 hook unchanged; only consumer registrations dropped for movement-specific callbacks
- Proto package bumped, lock file committed alongside

---

## Followups & out of scope

### Slice 3 (post-wave cleanup, after playtest passes)

- Delete v1alpha1 movement *callbacks* on web: `onMovementCompleted`, the matching `case 'movementCompleted':` branch in `useEncounterStream.ts:127-128`, the `MovementCompletedEvent` import. Optionally `onRoomRevealed` if v2 covers everything (verify in playtest).
- rpg-api stops emitting `MovementCompletedEvent` (and probably `RoomRevealedEvent`).
- Refactor opportunity: extract `<EncounterStreamProvider>` if the dual-hook pattern in `LobbyView` is awkward. Not forced.

### Filed-but-not-blocking issues to open

- **Expand `SnapshotDelivered.encounter`** — toolkit `Snapshot` shape doesn't map to proto yet; v2 sends an empty encounter on stream open. Required before v1alpha1's `getEncounterState` can be retired. Filed against rpg-toolkit + rpg-api-protos.
- **`EncounterStreamProvider` refactor** — defer until v2 has 3+ event types in active use AND dual-hook pattern shows real friction.
- **Animation along `actual_path`** — defer until new animated character models land. Polish slice in a future wave.
- **Reconnect telemetry** — UI surface for stream health (banner, retry indicator). Not load-bearing for any wave goal.

### Open questions answered during design

- **Stream lifecycle / mount point** — sibling hook in `LobbyView` (Architecture §2).
- **Snapshot reconciliation** — v1 owns initial state, v2 owns deltas, empty `SnapshotDelivered` is a sync barrier (Stream lifecycle §Snapshot semantics).
- **Event dispatch shape** — callback-bag with switch-on-oneof, mirroring v1 (Event dispatch §Per-event behavior).
- **Animation hex-by-hex vs teleport** — teleport for slice 2; animation deferred (Event dispatch §Animation policy).
- **v1 vs v2 coexistence** — parallel streams, no flag, v1 movement callbacks just not registered (Architecture §1).
- **Pass-through visibility rendering** — ghost at `last_known_position`, 40% opacity, cleared on reappear (Event dispatch §Visibility transitions).

### Out of scope (slice 2 will not ship)

- `OpenDoor`, attack, condition events on v2 — wave 2.6+
- Real LoS (walls, lighting, darkvision) — toolkit ships Manhattan stub; web renders whatever the toolkit projects
- Multi-process scale-out (Redis pub/sub for the broker) — single-process is fine until production load demands otherwise
- v1alpha1 deletion of *anything* — strictly slice 3
- `CreateEncounter` / `GetEncounter` RPCs — playtest seeds via Redis directly; not slice 2's problem
