# Encounter SDK Direction — rpg-api side (Wave 2.5 design)

**Status:** Design approved 2026-05-06; ready for plan-document-reviewer + writing-plans.

**Companion docs:**
- `sdk-direction.md` — encounter SDK design covering both sides of the boundary
- `plans/02-walking-skeleton.md` — toolkit slice 1 (shipped, PR #623 merged)
- `rpg-toolkit/docs/architecture/components/encounter.md` — toolkit component doc

**Wave:** [#11 Chapter 1: Architecture Honesty — Wave 2.5: Player events ▶ Movement](https://github.com/users/KirkDiggler/projects/10)

**Tracker issues:**
- [rpg-project#15](https://github.com/KirkDiggler/rpg-project/issues/15) — wave umbrella + sign-off (closes on playtest)
- [rpg-toolkit#628](https://github.com/KirkDiggler/rpg-toolkit/issues/628) — confirm SDK sufficient for integration
- [rpg-toolkit#629](https://github.com/KirkDiggler/rpg-toolkit/issues/629) — known gap: emit EntityAppeared / EntityDisappeared on LoS crossings (filed during this design; out of slice 1 scope)
- [rpg-api#494](https://github.com/KirkDiggler/rpg-api/issues/494) — rpg-api wiring (this doc's primary subject)
- [rpg-dnd5e-web#387](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/387) — web rendering side (slice 2)

---

## Wave goal (the playtest sentence)

**Two players in one encounter, one moves, the broker delivers per-player projected events to each — and the web renders the move from each player's perspective with no client-side filtering.**

The wave closes when that scenario is verified on a local two-browser playtest. Not when PRs merge.

---

## Sequencing

**Option chosen: API-as-one-PR, then web; outside-in within each branch.**

- **Slice 1 — rpg-api #494:** single branch. First commits scaffold the wire (v1alpha2 service registered with `codes.Unimplemented` stubs) and a failing bufconn integration test. Subsequent commits fill in the broker, handlers, repo, translator. Branch ships when the bufconn test passes. Web stays on v1alpha1 movement.
- **Slice 2 — rpg-dnd5e-web #387:** subscribe to v1alpha2 `StreamEncounter`, dispatch typed events, render. Component test against a fake stream.
- **Sign-off — #15:** local two-browser playtest. Closes the wave + the four issues.

Outside-in is encoded as the *commit pattern* within each branch, not as an extra no-behavior PR (Option 3 was considered and rejected as ceremony).

**Locked decisions on sequencing** (deviations from issue #494 text):

1. **v1alpha1 movement stays alive in slice 1.** Issue #494 said "delete the old path." We do not. Parallel-run during slice 1 protects against a bad assumption surfacing in playtest. v1alpha1 movement is deleted in **slice 3** only after the v1alpha2 path is observed working in playtest. Other v1alpha1 verbs (Attack, OpenDoor, EndTurn, etc.) stay alive regardless — out of wave scope.
2. **Toolkit gaps use local `replace` directives during dev**, stripped before any rpg-api PR. Same dance as `feedback_worktrees`.
3. **bufconn integration test = slice 1 gate. Two-browser playtest = wave gate.**

---

## Architecture

### 1. Server wiring (additive, no v1alpha1 disturbance)

```
cmd/server/server.go (existing v1alpha1 wiring UNTOUCHED)
  + transport := encounter.NewInMemoryTransport()
  + broker    := encounter.NewBroker(transport)         // process-scoped singleton
  + encDataRepo := encountersv2.NewInMemory()           // *encounter.Data store
  + v2Handler   := encounterv2handler.New(...)
  + encounterv2pb.RegisterEncounterServiceServer(srv, v2Handler)
  + healthServer.SetServingStatus("dnd5e.api.v1alpha2.encounter.EncounterService", SERVING)
```

**Nothing is shared between v1alpha1 and v2.** Different services, different event paths (Redis pub/sub vs. in-memory broker), different repos. gRPC dispatches by service name; no collision.

**Single broker** because `InMemoryTransport` has no cross-process semantics; one instance handles all encounters via per-`(encID, playerID)` subscriptions. Multi-process (Redis transport) is deferred per the SDK spec.

### 2. Package layout (internal naming decoupled from wire version)

```
internal/handlers/dnd5e/v2/encounter/      # package encounter (mirrors v1alpha1 layout)
  handler.go
  handler_test.go
  translate.go
  translate_test.go

internal/repositories/encounters/v2/        # package encounters (matches existing plural)
  repository.go
  in_memory.go
  in_memory_test.go
```

Internal `v2` is the second generation of the implementation; the directory does not move when the wire graduates from v1alpha2 → v1beta → v1. Same play go-redis runs.

### 3. Repository

```go
package encounters

import (
    "context"
    "errors"
    "github.com/KirkDiggler/rpg-toolkit/encounter"
)

var ErrNotFound = errors.New("encounter not found")

type Repository interface {
    Get(ctx context.Context, id string) (*encounter.Data, error)  // ErrNotFound if missing
    Save(ctx context.Context, data *encounter.Data) error
}

func NewInMemory() Repository { ... }  // sync.Mutex-guarded map
```

`Save` and `Get` round-trip through the toolkit's `ToData`/`LoadFromData` (deep copy, not map alias) — proves serialization survives every storage operation.

No `Seed()` helper for tests; integration tests construct an `*encounter.Data` and call `Save` like any other caller.

### 4. Handler shape — slice 1 RPCs

```go
type Handler struct {
    encounterv2pb.UnimplementedEncounterServiceServer  // every other RPC = Unimplemented
    broker  *encounter.Broker
    encRepo encountersv2.Repository
    clock   clockwork.Clock
}
```

**Slice 1 implements only `MoveEntity` and `StreamEncounter`.** Everything else (CreateEncounter, GetEncounter, TakeAction, EndTurn, Interact, SubmitCheck) returns `codes.Unimplemented` via the embedded server. Future slices fill them in. CreateEncounter is not implemented in slice 1 because the integration test seeds `*encounter.Data` directly via the repo — the wave goal does not require RPC-level encounter creation.

**MoveEntity flow** (load → verb → save → ack):
```
1. playerID := auth.GetPlayerID(ctx); reject if empty → codes.Unauthenticated
2. data := encRepo.Get(ctx, req.EncounterId)         // ErrNotFound → codes.NotFound
3. enc  := encounter.LoadFromData(data, h.broker)
4. path := protoPositionsToHexes(req.ProposedPath)
5. err  := enc.Move(core.PlayerID(playerID), path)   // toolkit validates + broker emits
6. err  := encRepo.Save(ctx, enc.ToData())
7. return &MoveEntityResponse{}                      // empty ack by design
```

`req.EntityId` is validated against the player's controlled entity (looked up via the encounter's player registry); reject mismatches in slice 1. NPC/GM control revisits this in a later slice.

**StreamEncounter flow** (subscribe → snapshot → loop):
```
1. playerID := auth.GetPlayerID(ctx); reject if empty
2. sub, err := h.broker.Subscribe(encID, playerID)
3. defer sub.Close()
4. data := encRepo.Get(ctx, encID)
   enc  := encounter.LoadFromData(data, h.broker)
   snap := enc.SnapshotFor(playerID)
   stream.Send(translateSnapshot(snap, h.clock.Now()))   // SnapshotDelivered first
5. loop:
     select case <-ctx.Done(): return
            case evt := <-sub.Events():
                out, err := translateEvent(evt, playerID, h.clock.Now())
                // error switch (see translator section below)
                stream.Send(out)
```

Subscribe-before-snapshot ordering ensures any event firing between the snapshot read and the subscription becoming live is captured. The toolkit broker delivers from the moment of `Subscribe`.

**Critical: no orchestrator layer in v2.** The toolkit IS the business logic. Handler talks directly to `encounter.Encounter` and `Broker`.

```
v1alpha1: Handler → Orchestrator → entities + Publisher
   v2:    Handler → Toolkit Encounter SDK + Broker
```

The handler is *thinner* in v2 — RPC-translation + load/save bookkeeping only. Business logic moves out of rpg-api into rpg-toolkit. This is the load-bearing architectural shift; it is the whole point of Chapter 1: Architecture Honesty.

### 5. Proto translator (toolkit event → v1alpha2 wire)

Pure functions, same package as the handler (`translate.go`).

```go
var (
    ErrViewerSawNothing  = errors.New("viewer's PerPlayer slice is empty; no event for this stream")
    ErrUnknownEventType  = errors.New("translator has no mapping for this toolkit event")
)

func translateEvent(
    evt events.EncounterEvent,
    viewerID core.PlayerID,
    now time.Time,
) (*encounterv2pb.EncounterEvent, error)
```

**Per-viewer signature** because the toolkit's `MoveEvent.PerPlayer` carries every viewer's slice; the translator picks `evt.PerPlayer[viewerID]` to populate `EntityMoved.actual_path`. Translation is per-stream, not global.

**Wall-clock injected.** Toolkit events carry `seq` but not timestamp (toolkit is rules engine, not clock owner). The proto envelope wants both. Handler injects `clockwork.Clock`; translator stamps `now` per call.

**Slice 1 mapping table:**

| Toolkit event | Proto envelope `oneof event` | Notes |
|---|---|---|
| `*events.MoveEvent` | `entity_moved` | `actual_path = PerPlayer[viewerID].SeenSegments`; `from = actual_path[0]`; `to = actual_path[len-1]`. Per-viewer reality. |
| `*events.HexRevealedEvent` | `geometry_revealed` | `hexes = PerPlayer[viewerID].Hexes`. Walls deferred (movement-stub LoS doesn't emit any). |
| Synthetic `SnapshotDelivered` | `snapshot_delivered` | Built from `enc.SnapshotFor(viewerID)` at stream open — not from broker. |

Other toolkit event types → return `(nil, ErrUnknownEventType)`. New events get added to the table as future slices land.

**Stream-loop error discipline:**
```go
out, err := translateEvent(evt, viewerID, h.clock.Now())
switch {
case errors.Is(err, ErrViewerSawNothing):
    continue  // expected; viewer was on the audience set but saw nothing visible
case errors.Is(err, ErrUnknownEventType):
    h.log.Warn("translator gap", "type", fmt.Sprintf("%T", evt))
    continue  // gap to file; don't tear down the stream
case err != nil:
    return err  // genuine bug; stream ends, client reconnects
}
stream.Send(out)
```

Typed errors are the communication channel. Callers `errors.Is`-check the expected ones; everything else escalates. Honors the project rule: never return `(nil, nil)`.

**Coordinate translation:** `core.Hex` (Q,R,S cube) ↔ proto `Position` (Q,R,S cube). 1:1 helpers, table-tested.

### 6. Test strategy

| Layer | Tool | What it proves |
|---|---|---|
| **Translator unit** | `translate_test.go`, table-driven, `proto.Equal` | Wire-fidelity contract. **Target: 100% line coverage on `translate.go`.** |
| **Handler unit** | testify suite, real broker + real `InMemoryTransport` + real repo + fake clock | RPC behavior end-to-end within one process. |
| **Integration (bufconn)** | extends existing `internal/integration/harness/harness.go` with v2 client + broker handle | The wave-2.5 acceptance shape: two players, one moves, both subscribe, assert per-stream delivery. **Slice 1 gate.** |

**No broker mock generated.** The broker is a small, well-tested toolkit type — mocking would cost more friction than it pays. The mockable boundary, if and when one is ever needed, is `Transport` (interface), not `Broker` (concrete). Slice 1 has zero handler-test mocks.

**Auth in tests:** reuse whatever helper the v1alpha1 handler tests use to inject a playerID into the request context. If no helper exists, add a tiny one in the auth package.

**Integration test placement:** `internal/integration/encounter_v2_test.go`, same dir as existing integration tests. No subdir; nothing about v2 needs isolation.

**Integration test outline (the slice 1 gate):**
```go
func (s *EncounterV2IntegrationSuite) TestMovementSliceTwoPlayers() {
    data := buildTwoPlayerEncounterData(s.T())   // both players in mutual LoS
    s.Require().NoError(s.srv.EncRepoV2.Save(ctx, data))

    streamA, _ := s.srv.EncounterClientV2.StreamEncounter(ctxAsPlayer(ctx, "A"), &pb.StreamEncounterRequest{EncounterId: data.ID})
    streamB, _ := s.srv.EncounterClientV2.StreamEncounter(ctxAsPlayer(ctx, "B"), &pb.StreamEncounterRequest{EncounterId: data.ID})

    snapA := mustRecv(streamA); s.Require().NotNil(snapA.GetSnapshotDelivered())
    snapB := mustRecv(streamB); s.Require().NotNil(snapB.GetSnapshotDelivered())

    _, err := s.srv.EncounterClientV2.MoveEntity(ctxAsPlayer(ctx, "A"), &pb.MoveEntityRequest{
        EncounterId: data.ID, EntityId: "A", ProposedPath: pathThreeHexes,
    })
    s.Require().NoError(err)

    movA := mustRecv(streamA); assertEntityMoved(s, movA, "A", pathThreeHexes)
    movB := mustRecv(streamB); assertEntityMoved(s, movB, "A", pathThreeHexes)
}
```

---

## Slice 1 commit shape

Outside-in encoded in commit ordering. Six commits, one PR.

```
1. scaffold: register v1alpha2.EncounterServiceServer with codes.Unimplemented stubs
   + harness extension (EncounterClientV2, broker handle)
   + bufconn integration test (failing / t.Skip with TODO)

2. feat: encounters/v2 in-memory repository
   + ToData/LoadFromData round-trip tested

3. feat: proto translator (translate.go) for MoveEvent + HexRevealedEvent + Snapshot
   + table-driven tests, 100% coverage on translate.go
   + ErrViewerSawNothing / ErrUnknownEventType sentinels

4. feat: StreamEncounter handler — broker subscribe, snapshot-then-deltas
   + handler unit tests for subscribe/snapshot/teardown
   → integration test partially passes (streams open, snapshot delivered)

5. feat: MoveEntity handler — load → verb → save
   + handler unit tests for happy path + auth/notfound errors
   → integration test fully passes (the slice-1 gate)

6. docs: ADR/journey note + status.md update
```

Reviewers can read commit-by-commit. CI is green throughout (failing test gets `t.Skip` until commit 5 enables it).

---

## Toolkit gap workflow

If integration surfaces an SDK gap (per [#628](https://github.com/KirkDiggler/rpg-toolkit/issues/628)):

1. Open the gap as a new toolkit issue. Add to board #11 wave 2.5.
2. Local: `replace github.com/KirkDiggler/rpg-toolkit => ../rpg-toolkit` in rpg-api `go.mod`. Toolkit branch holds the fix locally.
3. Iterate end-to-end against the local toolkit.
4. **Before opening any rpg-api PR:** push the toolkit branch, open + merge the toolkit PR, tag a new toolkit version, `go get -u` in rpg-api, **strip the replace directive**.
5. rpg-api PR ships against a published toolkit version per workspace `CLAUDE.md`.

The known LoS-loss gap ([#629](https://github.com/KirkDiggler/rpg-toolkit/issues/629)) is filed proactively — not blocking slice 1 because the wave goal scenario is two players in one room with mutual LoS. If a different playtest fixture exposes it, run the workflow above.

---

## Slice 1 acceptance gate

- `make ci-check` clean.
- All three test layers green; integration test (the wave-acceptance shape) passes.
- Local two-browser smoke: web (still on v1alpha1) — no regressions on existing movement.
- All toolkit gaps either zero OR filed + acknowledged on board.

---

## Slice 2 + wave close (preview)

- **Slice 2 (rpg-dnd5e-web #387):** subscribe to v1alpha2 `StreamEncounter`, dispatch typed events, render. Component test against fake stream. Local two-browser playtest with rpg-api-from-slice-1.
- **Wave sign-off:** playtest log on issue #15 → close #15, #494, #387, #628 (or close #628 with no work if no gaps surfaced).
- **Slice 3 (post-wave cleanup):** delete v1alpha1 `MoveCharacter` + `EventTypeMovementCompleted` orchestrator emission. Other v1alpha1 verbs stay alive.

---

## Open SDK questions answered during design

These were the open questions in `next-session.md`. Resolutions:

- **Single rpg-api server struct vs component-style?** — Component-style. The existing `cmd/server/server.go` uses functional composition; the v2 handler is one more component instantiated and registered alongside.
- **v1alpha1 cutover (dual-mode / flagged / v1alpha2-only)?** — Dual-mode via parallel service registration. Web migrates one slice at a time; v1alpha1 movement deletion is a deliberate slice-3 step gated on playtest verification.
- **Proto adapter location?** — Hand-written, same package as the handler (`translate.go`). Pure functions, table-tested.
- **Test strategy (mocks vs real broker + InMemoryTransport)?** — Real broker, no broker mock. The mockable boundary is `Transport`, not `Broker`. Translator stays pure; handler tests use real components.
- **Snapshot behavior on connect vs reconnect?** — Same path. Every `Subscribe` triggers a `SnapshotFor(playerID)` build and a synthetic `SnapshotDelivered` as the first message; deltas follow.

---

## Out of scope (forward pointers)

- `OpenDoor` verb on rpg-api side — same shape as `MoveEntity`, future slice.
- Combat verbs, action economy, conditions, monsters — Wave 2.5 is movement only.
- Real LoS (walls, lighting, darkvision) — toolkit ships Manhattan stub.
- Redis transport — `InMemoryTransport` until we go multi-process.
- Event-replay catch-up (`EventLog` interface alongside `Transport`).
- Out-of-character chronicle (additive — second broker channel).
- `RecipientID` generalization for GMs/spectators (PlayerID-only Day 1).
