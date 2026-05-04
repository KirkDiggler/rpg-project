---
name: Coordinate Types — Implementation Plan
description: Concrete commits, file paths, audit checklist, and acceptance criteria for issue #471
updated: 2026-05-02
status: draft
confidence: medium — design locked; toolkit-type-reuse question open
---

# Plan: Coordinate Types refactor

Implements `rpg-project/ideas/coordinate-types/design.md`.
Tracked as: **rpg-api issue #471**.
Supersedes: paused PRs #459, #461, #463, #466, #467, #468.

## Pre-requisites — these land first

This refactor consumes a new proto shape, so cross-repo work happens in order:

1. **rpg-api-protos issue #139** — `buf breaking` becomes blocking
   - 1-line change: `continue-on-error: true` → `false` in `.github/workflows/ci.yml`
   - Small PR, no implementation risk
2. **rpg-api-protos new PR** — `apiv1alpha1.Position` shape change, **in-place edit of v1alpha1**
   - `double x = 1; double y = 2; double z = 3` → `int32 x = 1; int32 y = 2; int32 z = 3`
   - In-place per project versioning policy (`project_proto_versioning_policy`): alpha and beta packages explicitly allow breaking changes. v1+ would require a per-service version bump, but v1alpha1 doesn't.
   - `buf breaking` flags this (now blocking after #139); we explicitly approve the intentional break.
   - Publish new SDK (`@generated` branch)
3. **rpg-api dependency bump** — `GOPROXY=direct go get github.com/KirkDiggler/rpg-api-protos/gen/go@generated` on the issue #471 branch

After these three, the coord-types refactor PR can land.

> **Fallback if cross-repo coordination stalls:** Land the rpg-api refactor first with `int(protoPos.X)` casts at the handler boundary; clean up the proto signature in a follow-up PR. The casts are lossless for integer-valued floats, and the float→int squeeze is the only "drift" — nothing else changes. We accept this only if pre-req sequencing slips; the preferred path is in-order.

## Single PR. Three internal phases. One commit per phase.

Branch from latest `main`. No reuse of #468's branch — clean slate.

### Phase 1 — Define types and `Module` (commit 1)

Goal: introduce the new types in the `dungeon` package, with the canonical `Module` as the only public bridge between coord spaces. **No call sites changed.** Old `Position` types still in place. Adds compile, tests pass.

**New files in `rpg-api/internal/components/dungeon/`:**
- `coords.go` — `LocalPosition`, `AbsolutePosition` types + constructors that enforce the cube invariant (`X+Y+Z=0`, derive `Y` from `X` and `Z`)
- `module.go` — `Module` struct, `Data` struct (serializable shape), `LoadFromData`, `ToData`, `LocalToAbsolute`, `AbsoluteToLocal`
- `coords_test.go` — constructor invariant enforcement, accessor sanity
- `module_test.go` — load/save round-trip, transform correctness, missing-room error

**Edits:**
- `types.go` — leave existing `Position` in place. Add a doc comment pointing readers at `coords.go` for new code.

**Acceptance for commit 1:**
- `go test ./internal/components/dungeon/...` passes including the new tests
- `make pre-commit` clean
- `git grep 'dungeon\.Position\|entities\.Position'` returns the same usage count as before this commit (we haven't migrated anything)

### Phase 2 — Audit and migrate room-local construction (commit 2)

Goal: every place that constructs a position **inside a room** now produces honest cube using `LocalPosition` (with `.Z` set, `.Y` derived). The compiler catches anyone who tries to pass a `LocalPosition` where a `Position` is expected.

**Audit list (verify each by grep before claiming complete):**

Inside the dungeon component:
- [ ] `internal/components/dungeon/generator.go` — monster placement, obstacle placement, exit selection
- [ ] `internal/components/dungeon/toolkit/perimeter.go` — wall segment construction
- [ ] `internal/components/dungeon/toolkit/coords.go` — `offsetToCube` returns local now
- [ ] `internal/components/dungeon/toolkit/validation.go` — `getZoneCenter`, `hasPathToEdge`, `intersectsWall` etc. (all `dungeon.Position` callers)
- [ ] `internal/components/dungeon/types.go` — any internal struct fields that should be `LocalPosition`
- [ ] `internal/components/dungeon/integration_test.go` and `room_origin_test.go` — test fixtures
- [ ] `internal/components/dungeon/types_test.go` — test fixtures

Outside the dungeon component but still constructing room-local:
- [ ] `internal/orchestrators/encounter/dungeon_mapper.go` — anywhere it builds positions before a room is placed (skim for usage; may have none)

**Acceptance for commit 2:**
- All call sites that build positions inside a room use `LocalPosition`
- `make pre-commit` clean
- `git grep` confirms no `dungeon.Position{` constructor calls remain inside the dungeon package
- Tests still pass — `Module` isn't being called from production code yet, but its type signatures match

### Phase 3 — Migrate transform sites, remove old types (commit 3)

Goal: every place that crosses local→absolute calls `Module.LocalToAbsolute`. Hand-rolled cube math is removed. Old types deleted.

**Transform sites (the 5 from PR #468 plus the handler-side fans):**

Orchestrator:
- [ ] `orchestrators/encounter/orchestrator.go` — `mergeNewRoomMonsters` (the canonical case from PR #468)
- [ ] `orchestrators/encounter/orchestrator.go` — `addMonstersToEntityMap`
- [ ] `orchestrators/encounter/orchestrator.go` — `buildRoomLayoutProto` (wall position translation)
- [ ] `orchestrators/encounter/orchestrator.go` — `getCurrentRoomOrigin` returns `AbsolutePosition`
- [ ] `orchestrators/encounter/orchestrator.go` — Module is constructed/loaded once per encounter and held in orchestrator state (or rebuilt per call from `EncounterData` — TBD by Phase 1's Module shape)

Handlers:
- [ ] `handlers/dnd5e/v1alpha1/encounter/handler.go` — ~9 `shiftRoomToAbsolute(...)` callers all replaced by `module.LocalToAbsolute(...)`
- [ ] `handlers/dnd5e/v1alpha1/encounter/converters.go` — `dungeonPositionToProto` updated to take `AbsolutePosition`; if proto is now `int32`, this is a 3-field copy with no cast
- [ ] `handlers/dnd5e/v1alpha1/encounter/handler.go` — remove `shiftRoomToAbsolute` helper entirely
- [ ] `handlers/dnd5e/v1alpha1/encounter/converters_test.go` — update any test that referenced `shiftRoomToAbsolute`

Then **delete:**
- [ ] `dungeon.Position` — type removed from `types.go`
- [ ] `entities.Position` — type removed from `entities/encounter.go`
- [ ] All `int(...)` / `float64(...)` casts that existed only to bridge the int/float64 mismatch

**Acceptance for commit 3 (and the PR):**
- `git grep -E 'dungeon\.Position\b|entities\.Position\b'` returns **no usages** (allowing for `dungeon.AbsolutePosition` and `dungeon.LocalPosition`)
- `git grep -E 'shiftRoomToAbsolute|cubeY := -cubeX'` returns **no usages**
- All Round 2 #468 sites pass through `Module.LocalToAbsolute` exactly once
- `make pre-commit && make ci-check` clean
- The integration test (`internal/integration/encounter/open_door_test.go`) still passes — RoomRevealed event delivers all rooms with correct absolute positions
- Coverage of `coords.go` and `module.go` ≥ 80%

## Live testing protocol — only after Phase 3 lands

Per `feedback_prefer_breaking_changes` and the design constraint: **no live testing until the old types are gone**. Half-migrated state hides bugs in the seam. The PR is "ready to test" once Phase 3 acceptance criteria are met.

Then:
1. Run the integration suite: `make integration-test` (Docker required; uses testcontainers)
2. Manual playtest: 4-class dungeon, multi-room movement, OpenDoor flow, monster spawn placement
3. Verify the 5 Round 2 fix scenarios visually in the web client (Chrome DevTools MCP attached)

## Open consideration — toolkit type reuse

The rpg-api-development skill says "Use toolkit types directly. Don't create API-specific entity types." rpg-toolkit's `tools/spatial` has `CubeCoordinate` (or similar — verify in toolkit's own code).

**Question:** Should `LocalPosition` and `AbsolutePosition` be type aliases / struct-wrappers around `spatial.CubeCoordinate`, or independent local types?

- **Pro alias:** one source of cube primitive truth; aligns with skill guidance; smoother graduation when dungeon component eventually moves to rpg-toolkit (the types stay).
- **Pro independent:** dungeon component is currently in rpg-api; aliasing introduces a toolkit dependency at the type level (vs. usage level only). The local-vs-absolute semantic is something toolkit's `CubeCoordinate` doesn't carry, so we'd be wrapping anyway.

**Recommendation:** Defer to Phase 1 review. Define as independent local types in commit 1; if `spatial.CubeCoordinate` is a clean fit, refactor to wrap/alias before commit 2. If not, ship as independent and revisit when dungeon graduates. Either way, `Module.LocalToAbsolute` remains the single bridge.

## What this PR does NOT do (deferred)

- **Does not move proto out of orchestrator.** That's issue #472. The orchestrator's `pb.` references to `pb.RoomLayout`, `pb.EntityState`, etc. are not touched here. The cleanup of those uses the new types' clean shape but is a separate refactor.
- **Does not migrate the dungeon component to toolkit.** That's issue #479. This refactor makes the move easier (the new types graduate cleanly) but keeps the component in rpg-api for now.
- **Does not change persistence.** Module's `LoadFromData`/`ToData` integrate with the existing in-memory dungeon repo. Redis persistence is issue #473.
- **Does not address `RoomData` as `interface{}`** in `EncounterData`. The Module shape resolves it conceptually (Module's `Data` is the concrete type), but the EncounterData refactor that consumes it is its own commit/PR (could be late Phase 3 or a follow-up — judgment call once we see the shape).

## Acceptance criteria — PR ready to merge

- [ ] All three phase commits land in order with their individual acceptance met
- [ ] `git grep` audit returns no traces of old types
- [ ] Integration test passes
- [ ] Live playtest verifies multi-room dungeon flow visually
- [ ] No `--no-verify` in commit history
- [ ] Copilot review addressed (per `feedback_copilot_review`)
- [ ] PR description links: design.md, this plan.md, issue #471, supersession of paused #459-#468
- [ ] Round 2 paused PRs (#459, #461, #463, #466, #467, #468) closed as superseded after merge

## Citations

- Design: `rpg-project/ideas/coordinate-types/design.md`
- Tracking: rpg-api issue #471
- rpg-api architecture overview: `rpg-api/docs/architecture/overview.md` (PR #470, merged 2026-05-02)
- Skill guidance: `rpg-api/.claude/skills/rpg-api-development`
- Audit source: `rpg-project/docs/teams/roles/rpg-api-member/context/discoveries.json` (esp. `disc-009` three Position types, `disc-006` RoomData as `interface{}`)
- Project preferences: `feedback_prefer_breaking_changes`, `feedback_verify_before_asserting`, `feedback_single_pr_per_round`
