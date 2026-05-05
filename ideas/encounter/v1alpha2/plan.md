# Encounter v1alpha2 — Master Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.
>
> This master plan decomposes the v1alpha2 encounter migration into 5 phases. Each phase has its own detailed plan in `plans/`. Read this document for context, then dispatch the active phase plan.

**Goal:** Replace `dnd5e.api.v1alpha1.EncounterService` with a redesigned `v1alpha2` contract that eliminates room-id leakage, mirrors toolkit primitives, and removes the structural cause of the multi-room bug class. End state: web exclusively speaks v1alpha2; v1alpha1 deleted from rpg-api-protos.

**Architecture:** Three repos, five sequential phases. Protos defined first; rpg-api orchestrator and handlers built against the new contract; web migrated; v1alpha1 deleted as the final phase. Clean break — no parallel-run, no shim layer (per design §6).

**Tech Stack:** buf v2 (protos), Go (rpg-api), TypeScript + React (rpg-dnd5e-web), gRPC + grpc-web (transport), rpg-toolkit (rules engine: spatial, environments, conditions, action economy).

**Spec:** [`../design.md`](../design.md) — read sections 2 (Architecture), 3 (Wire Shapes), 4 (RPCs), 5 (Streaming), 6 (Migration), 9 (Decisions Log) before starting any phase.

**Wave:** "alpha2 encounter" on board #11. Done-when: web exclusively speaks `dnd5e.api.v1alpha2.EncounterService`; `v1alpha1` deleted from `rpg-api-protos`.

---

## Phase Decomposition

```
┌──────────────────────────────────────────────────────────────────────┐
│ Phase 1: Protos                                                      │
│   rpg-api-protos                                                     │
│   Done: v1alpha2 messages + service compile in Go and TS             │
└──────────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌──────────────────────────────────────────────────────────────────────┐
│ Phase 2: API Orchestrator                                            │
│   rpg-api                                                            │
│   Done: Encounter session, Space, visibility, action economy,        │
│         path-correction working under unit + integration tests       │
└──────────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌──────────────────────────────────────────────────────────────────────┐
│ Phase 3: API Handlers                                                │
│   rpg-api                                                            │
│   Done: All 8 v1alpha2 RPCs return real responses + emit events      │
│         Integration tests against in-memory server pass              │
└──────────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌──────────────────────────────────────────────────────────────────────┐
│ Phase 4: Web Migration                                               │
│   rpg-dnd5e-web                                                      │
│   Done: Web exclusively talks v1alpha2 in playtest                   │
│         All roomId/currentRoomId references removed                  │
│         Multi-room playtest verifies cross-room pathing/attack       │
└──────────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌──────────────────────────────────────────────────────────────────────┐
│ Phase 5: Cutover                                                     │
│   all three repos                                                    │
│   Done: v1alpha1 deleted from rpg-api-protos, rpg-api, rpg-dnd5e-web │
│         Wave goal verified: cross-room behaviors work end-to-end     │
└──────────────────────────────────────────────────────────────────────┘
```

Phases are sequential by *completion* — each phase's done-when depends on the prior. **Phase 4 (web) may begin scaffolding work as soon as Phase 1 ships:** type imports, event-handler skeletons, removing roomId references can land in web before any v1alpha2 RPC returns real data. Phase 4 cannot *complete* until Phase 3 is shipping real responses, but it doesn't have to wait to *start*. Other phases are strictly sequential.

## Phase Plans

| # | Phase | Repo | Plan File | Status |
|---|---|---|---|---|
| 1 | Protos | rpg-api-protos | [`plans/01-protos.md`](plans/01-protos.md) | Drafted |
| 2 | API Orchestrator | rpg-api | `plans/02-api-orchestrator.md` | Written after Phase 1 ships |
| 3 | API Handlers | rpg-api | `plans/03-api-handlers.md` | Written after Phase 2 ships |
| 4 | Web Migration | rpg-dnd5e-web | `plans/04-web-migration.md` | Written after Phase 3 ships |
| 5 | Cutover | all three | `plans/05-cutover.md` | Written after Phase 4 ships |

**Why phases are written just-in-time:** Phase 1 will teach us what the proto shapes feel like in code; Phase 2 will teach us where toolkit integration is rough; Phase 3 will teach us the realistic event-stream shape; Phase 4 will teach us what the web actually needs. Writing all 5 plans upfront would lock in guesses. Writing them as we go costs nothing and gives later phases the benefit of earlier learning.

## Per-Phase Done-When Criteria

### Phase 1: Protos — Done-When
- `make lint` passes (no output, exit 0)
- `make compile-go` passes (after the idempotency fix in Phase 1 Task 4: creates `gen/go/go.mod` if missing, runs `go mod tidy`, runs `go build ./...` cleanly; safe to re-run)
- `make compile-ts` passes (`npx tsc --noEmit` against generated TS, exit 0)
- `make test` passes end-to-end (lint + format check + generate + mocks)
- Phase 1 plan's Task 6 consumer-check passes (every public Go type referenced from external program builds)
- PR merged to main on `rpg-api-protos` (verify with `gh pr view <num> --json state` returning `MERGED`)
- CI auto-pushed to `generated` branch and tagged a release (verify with `git ls-remote --tags origin | tail -3`); the resulting pseudo-version is what Phase 2 will `go get`

### Phase 2: API Orchestrator — Done-When
- Encounter session model with mode (FREE_ROAM / TURN_BASED) implemented; mode flips work under unit tests
- Space construction from existing dungeon generation produces flat hex map with absolute coords; toolkit room→space translation tested
- Tiered visibility: explored geometry sticks, entity LOS computed per player per event; both paths covered by tests
- Entity model with oneof typed data (Character, Monster, Trap, Prop, NPC, Obstacle) round-trips through unit tests
- Path correction: orchestrator simulates proposed path against world, emits EntityMoved with MovementInterruption; integration test covers OoM, hidden-trap-stop, blocked-by-entity
- Action economy integration with toolkit's two-level model; AvailableActions computed per turn state
- All exported orchestrator methods have unit tests using gomock (per repo convention)

### Phase 3: API Handlers — Done-When
- All 8 RPCs implemented (`CreateEncounter`, `GetEncounter`, `StreamEncounter`, `MoveEntity`, `Interact`, `TakeAction`, `EndTurn`, `SubmitCheck`) — return real responses, emit real events on stream
- Integration tests using `bufconn` cover: encounter creation, mode flip on mimic interaction, MoveEntity with proposed-path correction, Interact triggering trap, TakeAction in TURN_BASED, EndTurn advancing initiative + round, SubmitCheck against pending prompt with FailedPrecondition when none pending
- Stream lifecycle tested: initial snapshot delivery, sequential events, reconnect with `last_seen_sequence`
- Auth header validation tested: server identifies caller from header, validates authority for the operation (player owns the character being moved, etc.)
- Error taxonomy on the wire: `FailedPrecondition` for mode/state mismatches (`TURN_BASED`-only RPC called in `FREE_ROAM`, `SubmitCheck` with no pending prompt), `PermissionDenied` for authority failures, `NotFound` for missing entities/encounters; documented in `docs/architecture/components/encounter.md` (or equivalent) on rpg-api
- Logging covers RPC entry, mode transitions, event emissions; metrics emit per-RPC counters and stream subscription gauges
- No new lint regressions on the encounter package (per active.md gotcha)

### Phase 4: Web Migration — Done-When
- New `useEncounterV2` hook (or equivalent) replacing existing v1alpha1 hooks
- All call sites that read `roomId` / `currentRoomId` / per-room walls / per-room entities updated to consume flat Space + visible entities + AvailableActions from v1alpha2
- Event handler dispatches keyed off v1alpha2 typed events (DoorOpened, EntityMoved, ModeChanged, etc.)
- Pathing computed against flat space; no per-room translation
- Range overlay + floor rendering work in continuous coords
- Playtest verifies: open door → revealed monster at correct position; cross-room movement; cross-room attack; multi-room rendering
- v1alpha1 hooks remain in place but are not called by any active component (deletion happens in Phase 5)

### Phase 5: Cutover — Done-When
- `rpg-api-protos`: `dnd5e/api/v1alpha1/encounter.proto` deleted; service registration removed; mocks regenerated; PR merged
- `rpg-api`: v1alpha1 encounter handler deleted; v1alpha1-only orchestrator paths deleted; server registration cleaned; tests updated; PR merged
- `rpg-dnd5e-web`: v1alpha1 encounter hooks/clients/events deleted; tests updated; PR merged
- Final playtest verifies wave goal (cross-room behaviors work, no regressions)
- `grep -ri "v1alpha1.EncounterService\|v1alpha1.Encounter" .` returns nothing across all three repos
- Wave on board #11 closed; chapter 1 (board #11) sees its blocking dependency cleared

---

## Cross-Phase Concerns

### Integration test strategy

Phase 3 produces in-memory bufconn integration tests that exercise the full stack: handler → orchestrator → toolkit → events stream. These become the regression baseline that Phases 4 and 5 must not break.

Phase 4 introduces playtest-driven verification (Chrome DevTools MCP, real grpc-web through the web client). This catches anything the bufconn tests miss (serialization quirks, stream reconnect, browser-side state machines).

Phase 5 re-runs the Phase 3 integration tests *and* the Phase 4 playtest after deletion to confirm nothing was depending on v1alpha1 implicitly.

### Feedback memories that govern execution

- `feedback_prefer_breaking_changes` — clean break, no shim layer
- `feedback_test_double_naming` — gomock-generated → "mock"; hand-written → "testItem"/"fakeRepo"
- `feedback_integration_tests_first` — build integration tests before manual playtest
- `feedback_single_pr_per_round` — one PR per logical unit; consolidate same-layer fixes
- `feedback_always_pr` — always push and create PR when finishing a branch; never merge locally
- `feedback_merge_over_rebase` — merge, never rebase
- `feedback_no_logic_in_web` — web renders + calls; never gates on game state

### What will *not* be done in this plan

- Persistence beyond encounter lifetime (encounters remain in-memory)
- Multi-rulebook abstractions (v1alpha2 lives in `dnd5e.api.v1alpha2` only)
- The 3 toolkit doc issues surfaced in design §7 (separate dispatch)
- Bulk-promote remaining ~21 janitor entries (separate dispatch)
- Janitor pass on toolkit docs harness (separate dispatch)

### Risk register

| Risk | Mitigation |
|---|---|
| Phase 2 orchestrator scope blows up | Phase 2 plan written *after* Phase 1 ships; we'll know what's actually hard |
| Web migration in Phase 4 reveals contract gaps | Phase 4 can request small additive changes to v1alpha2 protos; not breaking |
| Stale `fix/cross-room-state-wave2` and `fix/wave2-player-move-render` branches go unused | Decide post-Phase 1 whether to cherry-pick any commits or abandon |
| Encounter package's 65 pre-existing lint findings start being touched | Don't touch them in this work; that's a separate cleanup |

---

## Execution Handoff (Phase 1)

**To start Phase 1 (Protos):**

1. Open `plans/01-protos.md`.
2. Choose execution mode:
   - **Subagent-driven** (recommended) — dispatch a fresh subagent per task, review between tasks
   - **Inline execution** — execute tasks in this session using `superpowers:executing-plans`
3. Create branch `feat/v1alpha2-encounter-protos` on `rpg-api-protos`.
4. Begin with Phase 1 Task 1.

After Phase 1 ships and the proto module is published/tagged, resume here to write Phase 2.
