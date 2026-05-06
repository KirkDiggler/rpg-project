# Encounter v1alpha2 — Orchestrator & Event Streams Design

**Status:** Draft, partial. Captures Phase 2 design decisions from a brainstorming session that took the scenic route. Fresh session recommended for the next pass — see §5.

**Date:** 2026-05-06
**Companion to:** [`design.md`](design.md) (wire contract — shipped as v0.1.93)
**Phase:** 2 of the v1alpha2 migration

---

## 1. Goal & Scope

Cover the implementation architecture in rpg-api that the v1alpha2 wire contract from `design.md` lands on. Kept deliberately separate from `design.md` because that doc is the wire contract; this is the rpg-api orchestration shape.

**In scope (this doc):**
- Package layout (encounter/v2/)
- Orchestrator's stateless per-RPC pattern
- Event streams component shape
- Visibility policy location (rpg-api, not toolkit)
- Boundary rules with toolkit

**Not in scope (this doc, deferred to fresh-session pickup):**
- Path simulation algorithm in `MoveEntity`
- Encounter lifecycle details (create/destroy/idle/persistence)
- Toolkit integration map per RPC (which toolkit modules each RPC calls and in what order)
- Test strategy
- Exact streams API surface (one specific sub-question still open — see §3)

---

## 2. Locked Decisions

These were explicitly confirmed during the design conversation.

### 2.1 Package Layout

```
internal/orchestrators/encounter/
  orchestrator.go       ← legacy v1alpha1, package encounter
  open_door.go          ← legacy
  ...                   ← gets deleted in Phase 5 cutover
  v2/                   ← new v1alpha2 work
    orchestrator.go     ← package encounter (same name, different import path)
    session.go
    ...
```

Both directories use `package encounter`. Imports differentiate by path — go-redis pattern. Same approach for handlers under `internal/handlers/dnd5e/v1alpha2/encounter/`.

Phase 5 cutover deletes everything outside `v2/`. The `v2/` segment lives forever; future v3 sits alongside as `encounter/v3/`. Aligns with `feedback_prefer_breaking_changes` — no parallel-run, clean break.

### 2.2 Stateless Orchestrator — Per-RPC Cycle

The orchestrator does **not** hold long-lived session state. Each RPC is a bounded cycle:

```
1. Load entities from repo
2. Wire entities to a fresh event bus (toolkit's LoadFromData reconstitutes
   conditions as bus subscribers — request-scoped bus)
3. Set up gamectx (toolkit's per-request game context)
4. Process the turn input (RPC params → toolkit calls)
5. Check each entity's Dirty flag
6. Save dirty entities back to repo
7. Publish wire events via the streams component
```

The session struct I sketched earlier (`PerPlayer map[PlayerID]*PlayerView` etc.) is wrong — that's all repo state, not orchestrator state. The repo is the source of truth.

### 2.3 Do Not Change the Core

The toolkit's event bus, conditions/effects, action economy, spatial primitives — those are core. The orchestrator integrates *with* them via the published interfaces (`LoadFromData`, the bus, `ResolveAttack`, `ActionEconomy`). If we discover the toolkit needs something it doesn't have, that's a separate toolkit-side issue, not "modify in place to make the orchestrator's life easier."

This matches `rpg-project/CLAUDE.md`'s boundary rule: "Toolkit implements RULES — returns rich breakdowns" and the toolkit's own philosophy ("Generic Tools, Not Game Rules").

### 2.4 Event Streams Component

A domain-named component owns event distribution. The orchestrator does **not** talk to a pub/sub bus directly; it talks to the streams component. The component is **dumb** about visibility — that's policy, not plumbing.

**Lives in rpg-api initially.** Following the `dungeon` pattern (issue #479): start as an `internal/` component, learn what works, graduate to toolkit later when the API stabilizes and other consumers want it.

**Pluggable backing — same pattern as repos:**

| Impl | Use | Backing |
|---|---|---|
| `InMemoryEncounterStreams` | dev / single-instance prod today | go channels + maps + mutex |
| `RedisEncounterStreams` | future horizontal scale | redis pub/sub on per-encounter topic |
| `MockEncounterStreams` | tests | gomock-generated |

**Key insight from Kirk:** *players-in-the-encounter is gameplay state*, persisted in the repo. It is NOT MMO-style subscription tracking. The streams component handles event delivery to whoever's currently subscribed (gRPC stream open); the gameplay roster lives elsewhere.

### 2.5 Visibility Policy Stays in rpg-api

Visibility (sticky geometry per character, real-time entity LOS, trap illusions) is game logic. It does NOT belong in the streams component, the toolkit, or the bus. It's a pure function the streamer calls per event:

```go
func ResolveEvent(state EncounterState, raw RawEvent, playerID string) (encounterpb.EncounterEvent, bool)
```

Stateless, testable as a pure function. Calls into toolkit's `tools/spatial` for raw LOS primitives but holds the per-player policy itself.

---

## 3. Proposed Shapes (Pending Confirmation)

These were proposed but Kirk hasn't explicitly affirmed. Fresh session should treat as starting points, not commitments.

### 3.1 Streams Component API (proposed)

```go
type EncounterStreams interface {
    OpenEncounter(ctx context.Context, encID string) error
    CloseEncounter(ctx context.Context, encID string) error
    Publish(ctx context.Context, encID string, post Snapshot, events []RawEvent) error
    Subscribe(ctx context.Context, encID, playerID string) (<-chan StreamMessage, error)
}

type StreamMessage struct {
    State Snapshot      // post-RPC encounter state, for the visibility filter
    Raw   RawEvent      // the event itself
}
```

**Open sub-question:** explicit `AddPlayer`/`RemovePlayer` lifecycle methods, or collapse into `Subscribe`? My lean was *collapse* (smaller surface; the only state to track is "who's currently subscribed"); explicit lifecycle is justified only if we want async push (offline notifications, replay queues). Defer to next-session evaluation against actual use cases.

### 3.2 Streamer as Glue (proposed)

Handler stays dumb by delegating to a thin glue function:

```go
streamer.Run(ctx, streams, encID, playerID, send)
```

`streamer.Run` contains the loop: `subscribe → for each msg: filter via view.ResolveEvent → send`. Handler just:

```go
func (h *Handler) StreamEncounter(req, stream) error {
    playerID := authFromCtx(stream.Context())
    return streamer.Run(stream.Context(), h.streams, req.EncounterID, playerID, stream.Send)
}
```

### 3.3 State Piggybacks on Events (proposed)

Streams' `Publish` carries `(post-state, events)` as one tuple per RPC. Subscribers get fresh state with each event delivery — no repo round-trips in the hot path. Filter is the same `view.ResolveEvent` pure function.

**Caveat:** this was my framing, not Kirk's. Fresh session should evaluate alternatives:
- State refetched per event from repo (cheap with in-memory; expensive with persistent)
- State held in subscriber goroutine, updated as events arrive
- Other shapes

---

## 4. Open Questions (Deferred to Fresh Session)

These are real design surface, intentionally not resolved:

### 4.1 Path Simulation in MoveEntity

The largest piece of *new* logic. v1alpha1's MoveCharacter just teleports; v1alpha2 simulates the proposed path step-by-step against the real world (which the client may not fully see — undetected traps, hidden difficult terrain, perception-gated obstacles), produces actual_path with optional `MovementInterruption`. Bus events fire for traps, AoO, etc.

Questions to think through:
- Per-hex simulation loop: who owns it (orchestrator? toolkit's spatial?)
- How traps trigger from movement (entity reaction on bus when a movement-event passes a hex they care about?)
- AoO opportunity check: how is the toolkit's combat module called per-hex?
- What's emitted vs swallowed (do we emit per-hex events, or one EntityMoved with the actual path baked in?)

### 4.2 Encounter Lifecycle

When does an encounter get created? When does it get destroyed (or persist)? How does mode flip work mechanically (server-internal logic emits ModeChanged + InitiativeRolled when an ambush triggers)? What's the "idle" state vs combat state on the server side? How long do encounters live in repo (in-memory only today; persistence is board issue #473)?

### 4.3 Toolkit Integration Map per RPC

For each of the 8 RPCs (CreateEncounter, GetEncounter, StreamEncounter, MoveEntity, Interact, TakeAction, EndTurn, SubmitCheck): which toolkit modules get called and in what order. Mostly pattern-matching against existing v1alpha1 usage, but worth pinning so the orchestrator code is predictable.

Particularly: how `Interact` dispatches based on target entity type (door → toolkit's door handling; chest → loot reveal; mimic → entity transform + mode flip + initiative). The dispatch shape is non-obvious.

### 4.4 Test Strategy

`feedback_integration_tests_first` says build integration tests that prove API flows work before manual playtesting. For Phase 2:
- Unit tests for `view.ResolveEvent` (pure function — easy)
- Unit tests for `streams` impls (gomock for the interface; in-memory impl tested against the contract)
- Integration tests using `bufconn` against the full handler+orchestrator+streams+repo stack
- End-to-end playtest verification (Phase 4 territory, but design tests now)

What's the right granularity of integration tests? Per-RPC happy path + a couple of edge cases each? Or scenario-driven (full mimic-ambush sequence; full cross-room movement with trap stop)?

### 4.5 Toolkit Doc Issues (still queued)

From the design.md follow-ups; not blocking but should land sometime:
1. Restructure `rpg-toolkit/docs/architecture/components/` into per-module directories
2. Write `components/rulebook/action_economy.md` (promote two-level model from `rulebooks/dnd5e/CLAUDE.md`)
3. Document the harness rule: knowledge belongs in `architecture/`, `status.md`, `quality.md` — not CLAUDE.md

---

## 5. Notes for Fresh-Session Pickup

The session that drafted this took a meandering path. The fresh agent should be aware of opinions I accumulated and may want to discard:

### Biases to Examine

1. **"Two buses" framing.** I pushed early for separating toolkit's request-scoped bus from a server-side streaming bus. Kirk's later correction — "this isn't MMO, players-in-encounter is gameplay state" — suggests the streaming layer is even thinner than I was making it. The fresh session should evaluate whether a separate streaming abstraction is needed, or whether the repo+publish model is enough.

2. **Visibility as a first-class component.** I framed `view.ResolveEvent` as a separate component very early. It might still be the right shape, but the fresh session should reach for it only after the orchestrator's per-RPC flow is concrete.

3. **State-piggyback-on-events.** My framing for fresh state in the filter path. Possibly correct, possibly over-engineered. Alternatives in §3.3.

4. **MMO-shaped subscriber thinking.** I kept reaching for subscription bookkeeping (RegisterPlayer, etc.) even when Kirk's framing was simpler. Fresh session should treat "who's online" as the streams component's only concern, and "who's in the encounter" as repo state.

5. **Long-lived session struct.** I sketched a `Session` struct with `PerPlayer map[…]*PlayerView` early. That's wrong — repo is the source of truth, orchestrator is stateless per RPC. Don't reach for it.

### What's Definitely Right

- Layout decision (§2.1) — go-redis pattern, encounter/v2/, package encounter.
- Stateless per-RPC cycle (§2.2) — load → wire → gamectx → process → checkDirty → save.
- "Do not change the core" (§2.3) — toolkit untouched.
- Streams component lives in rpg-api initially (§2.4) — `dungeon` precedent (#479). NOT in toolkit yet.
- Visibility policy in rpg-api (§2.5) — pure function `view.ResolveEvent`.

### Suggested Pickup Order

1. Read `design.md` (wire contract — shipped at v0.1.93)
2. Read `plan.md` (master plan — Phase 1 done, Phases 2-5 written just-in-time)
3. Read this doc (`orchestrator-design.md`) — locked decisions in §2; treat §3 as proposed; §4 is the next design surface
4. Engage Kirk on §4.1 (path simulation) first — most novel piece
5. Then §4.2 (lifecycle), then §4.3 (integration map), then §4.4 (tests)
6. Once §4 is settled, write `plans/02-api-orchestrator.md`
7. Subagent-driven execution against that plan, same as Phase 1

---

## 6. References

- Wire contract: [design.md](design.md)
- Master plan: [plan.md](plan.md)
- Phase 1 plan: [plans/01-protos.md](plans/01-protos.md) (shipped 2026-05-06 → v0.1.93)
- Phase 1 issue: KirkDiggler/rpg-api-protos#151 (closed by PR #150)
- Wave: "alpha2 encounter" on board #11 (Chapter 1: Architecture Honesty)
- Toolkit ActionEconomy: `rpg-toolkit/rulebooks/dnd5e/combat/action_economy.go`
- Toolkit `dungeon` graduation precedent: KirkDiggler/rpg-api#479
