# Shared Dungeon Dice Throws Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ship one verified carved attack d20 that the actor carries from the production drawer into the real dungeon and that every session member sees hit the same authored walls/shut doors and reach the same settled/off-table terminal outcome.

**Architecture:** The web builds one immutable collision snapshot, runs a hidden fixed-step Rapier pre-simulation, and publishes a bounded group-shaped throw plan through a separate presentation service. The game server validates/authenticates/deduplicates and relays the plan through Redis Pub/Sub without simulating or touching toolkit Story; every web client runs local Rapier playback and applies sparse involved-body checkpoints. The product adapter supplies one d20, while transport/physics boundaries remain list-shaped and prove die-to-die behavior in tests.

**Tech Stack:** Proto3 + Buf, Go 1.25 + gRPC + go-redis, React 19 + TypeScript 5.8 + Connect-ES, Three.js 0.181, React Three Fiber 9, `@react-three/rapier` 2.2.0, `@dimforge/rapier3d-compat` 0.19.2, Vitest 4, Playwright.

**Spec:** `ideas/interactive-dice-tray/shared-dungeon-throws/design.md`

**Tracking:** Parent journey `KirkDiggler/rpg-project#289`; approved production design slice `KirkDiggler/rpg-project#303`; design PR `KirkDiggler/rpg-project#304`.

## Global Constraints

- Create one implementation issue and one branch per owning repository before code: `rpg-api-protos` from `origin/main`, `rpg-api` from `origin/dev`, and `rpg-dnd5e-web` from `origin/dev`.
- Product scope is one attack body with die ID `attack-d20`, shape `D20`, and verified preset `dice.original.carved.d20`.
- Plan/domain/service boundaries accept 1–20 stable bodies; `RAPIER_DUNGEON_D20_V1` accepts only `D20` bodies and the product adapter supplies exactly one.
- Physics is fixed at 60 Hz, at most 480 steps, 128 contacts, 256 checkpoint body states, 32 attempts, and 64 KiB encoded plan size.
- Position components stay within `[-4096,4096]`, linear speed magnitude at most 64, angular speed magnitude at most 128, and quaternion norm error at most `0.0001`.
- A plan carries no die result, disposition, hit/miss/critical, attack total, target defense, damage, HP, declaration/attack ref, preset ownership, asset URL, raw pointer coordinate/ID/path/timing, or prose.
- `SessionService`, toolkit Story, authoritative event sequence, `rpg-toolkit`, Attack request/response, and result authority remain unchanged.
- Publish and stream both require authenticated ownership of the named player member and membership in the launch-written session roster; session is validated and roller is server-bound from member.
- Redis Pub/Sub is live-only. A two-minute `SET NX` value provides cross-instance first-valid deduplication; there is no replay RPC.
- Floor cells, authored wall runs, shut/locked doors, and active dice are the only colliders. Props, items, characters, and monsters are absent.
- Open/shut door membership is frozen per attempt and included in the SHA-256 collision fingerprint.
- Actor playback begins only after accepted publish response or stream echo. Plan-before-event buffers 3 seconds; event-before-plan falls back after 10 seconds; start buffer is 150 ms; collider refresh grace is 500 ms; publish-local fallback is 750 ms.
- First accepted plan crosses the existing Story reveal gate. Off-table attempts never reroll authority or re-conceal Story.
- Reduced motion, provider/WebGL/Rapier failure, reconnect, stale history, unknown schema, fingerprint mismatch, and transport loss always complete truthfully.
- Production code imports nothing from `src/concepts/**`; concept spikes consume extracted shared production units afterward.
- Rapier/raw physics is dynamically imported, loading is coalesced, and every world/subscription/timer is disposed on scope reset.
- TDD is required except hand-written protobuf mechanics tests, which project policy forbids. Every code task starts with focused RED, records the expected failure, implements minimum GREEN, runs focused regressions, and commits.
- Run `buf format/lint/breaking/generate` for protos, `make pre-commit && make ci-check` for API, and `npm run ci-check` for web before every push. Never use `--no-verify`.
- Request exactly one Copilot round for each feature PR, read/reply to every inline thread, and never re-request.
- Publish/merge in this order: protos → API → web → rpg-project design record. Kirk performs the integrated two-browser walk before either consumer merges and Kirk alone merges PRs.

---

## File Structure

### `rpg-api-protos`

- Create `dnd5e/api/session/presentation/v1alpha1/service.proto` — group plan enums/messages and publish/live-stream service.
- Create `docs/architecture/components/session-presentation-service.md` — authority, lifecycle, validation, and consumers.
- Modify `docs/architecture/overview.md`, `docs/architecture/data-model.md`, `docs/status.md`, `docs/quality.md` — register the new in-flight consumed service honestly.

### `rpg-api`

- Create `internal/handlers/dnd5e/sessionaccess/access.go` and tests — shared authenticated owner+seated gates.
- Modify `internal/handlers/dnd5e/session/v1alpha1/handler.go`, `get_doors.go`, and ownership tests — delegate existing gates to the shared helper without behavior change.
- Create `internal/orchestrators/sessionpresentation/{types,validation,service,orchestrator}.go` and tests/mock — proto-free plan domain and publish/subscribe service.
- Create `internal/repositories/sessionpresentation/{repository,redis}.go` and tests — Lua first-valid dedupe + Redis Pub/Sub subscription.
- Create `internal/handlers/dnd5e/sessionpresentation/v1alpha1/{handler,convert,publish_dice_throw,stream_dice_throws}.go` and tests — wire translation and status mapping.
- Modify `cmd/server/server.go` and `internal/integration/harness/harness.go` — construct/register service and health/test clients.
- Create `internal/integration/sessionpresentation/shared_throw_test.go` — cross-instance two-subscriber proof.
- Create `docs/architecture/components/session-presentation.md`; modify `docs/architecture/overview.md`, `docs/status.md`, and `docs/quality.md`.
- Modify `go.mod` and `go.sum` only for the published generated proto pin.

### `rpg-dnd5e-web`

- Modify `package.json` and `package-lock.json` — published proto pin and direct `@dimforge/rapier3d-compat@0.19.2` ownership.
- Modify `src/api/client.ts`; create `src/api/useSessionDiceThrows.ts` and tests — Connect client, publish, and live-only stream.
- Create `src/components/ui/dice/dungeonDicePhysicsSchema.ts` and tests — fixed constants and d20 hull.
- Move/evolve concept helpers into `src/components/ui/dice/dungeonDiceWorld.ts` and tests — collider records, stable IDs, fingerprint, origin, and bounds.
- Create `src/components/ui/dice/dungeonDicePlan.ts` and tests — immutable domain parser/proto adapters.
- Create `src/components/ui/dice/dungeonDicePlanGenerator.ts` and tests — raw Rapier pre-simulation.
- Create `src/components/ui/dice/dungeonDicePlayback.ts` and tests — step/checkpoint/terminal reconciliation model.
- Create `src/components/ui/dice/DungeonDicePhysicsLayer.tsx` and tests — visible multi-body R3F/Rapier layer and face assist.
- Create `src/components/ui/dice/dungeonDiceGestureBridge.ts` and tests — tray hit ownership, map handoff, two-button lift, and launch input.
- Create `src/components/session/combat-experience/dungeonDiceCoordinatorState.ts` and tests — actor/witness attempt and timeout reducer.
- Create `src/components/session/combat-experience/useDungeonDiceCoordinator.ts` and tests — authority+transport+physics orchestration.
- Modify `DiceTrayInteractionSurface.tsx`, `DiceTray3D.tsx`, `DiceTrayPresentation.tsx`, `DiceDrawer.tsx`, `CombatExperience.tsx`, and their tests — optional production world bridge without legacy behavior changes.
- Modify `SessionCanvas.tsx`, `SessionEncounterView.tsx`, and tests — actual-camera projection and production presentation layer.
- Modify `presentation.ts`, `useCombatPresentation.ts`, `types.ts`, and tests — shared-plan release fact and witness wait/fallback.
- Refactor `src/concepts/attack-die-3d/{PhysicsTraySpike,DungeonFloorPhysicsSpike,dungeonDiceColliders,dungeonDiceInteraction}*` to import shared units.
- Create `src/components/ui/dice/dungeonDiceProductionBoundary.test.ts` — no production concept imports/eager Rapier entry.
- Create `scripts/attack-die/capture-shared-dungeon-throw-evidence.mjs` and `docs/evidence/${WEB_ISSUE}-shared-dungeon-throws/README.md`; modify `docs/how-to/attack-die-3d-concept.md`, `docs/status.md`, and `docs/quality.md`.

---

### Task 1: Create the three implementation slices and isolated worktrees

**Files:**
- No product files.
- Project 19: three direct child issues of `rpg-project#289`.

**Interfaces:**
- Produces: exact issue numbers exported as `PROTO_ISSUE`, `API_ISSUE`, `WEB_ISSUE`; one worktree/branch per repository.

- [ ] **Step 1: Create issue bodies with the responsible Team signatures**

Create `/tmp/shared-dice-protos-issue.md` and `/tmp/shared-dice-api-issue.md` with this outcome/non-goal shape and the Platform signature:

```markdown
## Parent journey

KirkDiggler/rpg-project#289 · approved design KirkDiggler/rpg-project#303 / PR #304.

## Outcome

Deliver this repository's owned production seam for shared group-shaped dungeon dice throw plans, preserving authoritative combat truth.

## Done when

- The approved design's repository-specific contract and tests are complete.
- The integrated API+web branch walk proves shared wall, shut-door, open-door, off-table, retry, fallback, and reconnect behavior.
- The PR has one answered Copilot round and all local/GitHub gates pass.

## Non-goals

- Server-side physics, exact trajectories, toolkit Story changes, damage presentation, collectible ownership, or gesture authority.

— platform agent, on behalf of KirkDiggler
```

Create `/tmp/shared-dice-web-issue.md` with the same parent/Done-when and this outcome/signature:

```markdown
## Outcome

Promote the verified carved attack d20 into the real SessionCanvas with tray handoff, two-button lift, real collisions, shared group-plan playback, and truthful fallback.

— ui-ux agent, on behalf of KirkDiggler
```

- [ ] **Step 2: Create, parent, and board the issues**

```bash
PROTO_URL=$(gh issue create -R KirkDiggler/rpg-api-protos \
  --title 'feat: add shared dice presentation contract' \
  --body-file /tmp/shared-dice-protos-issue.md --assignee KirkDiggler)
API_URL=$(gh issue create -R KirkDiggler/rpg-api \
  --title 'feat: relay shared dungeon dice throws' \
  --body-file /tmp/shared-dice-api-issue.md --assignee KirkDiggler)
WEB_URL=$(gh issue create -R KirkDiggler/rpg-dnd5e-web \
  --title 'feat: play shared dice throws in the dungeon' \
  --body-file /tmp/shared-dice-web-issue.md --assignee KirkDiggler)
export PROTO_ISSUE=${PROTO_URL##*/} API_ISSUE=${API_URL##*/} WEB_ISSUE=${WEB_URL##*/}
```

Use `addSubIssue` to parent each under `rpg-project#289`. Add each to Project 19 with Initiative **Four-player Level-3 Dungeon**, Area **Game Screen**, Kind **Build**, Status **In Progress**; Team **Platform** for proto/API and **UI/UX** for web. Verify all fields by reading Project 19 back; do not trust mutation responses.

- [ ] **Step 3: Create worktrees from remote bases**

```bash
git -C /home/kirk/game-dev/rpg-api-protos fetch origin
git -C /home/kirk/game-dev/rpg-api fetch origin
git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin

git -C /home/kirk/game-dev/rpg-api-protos worktree add \
  /home/kirk/.pi/worktrees/rpg-api-protos/${PROTO_ISSUE}-shared-dice-presentation \
  -b feat/${PROTO_ISSUE}-shared-dice-presentation origin/main

git -C /home/kirk/game-dev/rpg-api worktree add \
  /home/kirk/.pi/worktrees/rpg-api/${API_ISSUE}-shared-dice-presentation \
  -b feat/${API_ISSUE}-shared-dice-presentation origin/dev

git -C /home/kirk/game-dev/rpg-dnd5e-web worktree add \
  /home/kirk/.pi/worktrees/rpg-dnd5e-web/${WEB_ISSUE}-shared-dungeon-dice \
  -b feat/${WEB_ISSUE}-shared-dungeon-dice origin/dev
```

- [ ] **Step 4: Install and verify clean baselines**

```bash
cd /home/kirk/.pi/worktrees/rpg-api-protos/${PROTO_ISSUE}-shared-dice-presentation
buf lint && buf format --diff --exit-code

cd /home/kirk/.pi/worktrees/rpg-api/${API_ISSUE}-shared-dice-presentation
go mod download && make ci-check

cd /home/kirk/.pi/worktrees/rpg-dnd5e-web/${WEB_ISSUE}-shared-dungeon-dice
npm install
RPG_GAME_ASSETS_PATH=/home/kirk/game-dev/rpg-game-assets \
  ASSETS_SYNC_SKIP_UPDATE=1 npm run assets:sync
npm run ci-check
```

Expected: all three baselines pass. Stop and reconcile any pre-existing failure before implementation.

---

### Task 2: Add the group-shaped presentation proto

**Files:**
- Create: `dnd5e/api/session/presentation/v1alpha1/service.proto`
- Create: `docs/architecture/components/session-presentation-service.md`
- Modify: `docs/architecture/overview.md`
- Modify: `docs/architecture/data-model.md`
- Modify: `docs/status.md`
- Modify: `docs/quality.md`

**Interfaces:**
- Produces: `SessionPresentationService`, `PublishDiceThrow`, `StreamDiceThrows`, and the exact enums/messages consumed by API/web.

- [ ] **Step 1: Write the service contract**

Use package `dnd5e.api.session.presentation.v1alpha1` and Go package suffix `sessionpresentationpb`. Define the exact approved enums and messages:

```proto
syntax = "proto3";

package dnd5e.api.session.presentation.v1alpha1;

option go_package = "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/session/presentation/v1alpha1;sessionpresentationpb";
option java_multiple_files = true;
option java_package = "com.kirkdiggler.rpg.api.dnd5e.session.presentation.v1alpha1";

enum DicePhysicsSchema {
  DICE_PHYSICS_SCHEMA_UNSPECIFIED = 0;
  DICE_PHYSICS_SCHEMA_RAPIER_DUNGEON_D20_V1 = 1;
}
enum DiceShape {
  DICE_SHAPE_UNSPECIFIED = 0;
  DICE_SHAPE_D4 = 1;
  DICE_SHAPE_D6 = 2;
  DICE_SHAPE_D8 = 3;
  DICE_SHAPE_D10 = 4;
  DICE_SHAPE_D12 = 5;
  DICE_SHAPE_D20 = 6;
}
enum DiceStaticContactKind {
  DICE_STATIC_CONTACT_KIND_UNSPECIFIED = 0;
  DICE_STATIC_CONTACT_KIND_WALL = 1;
  DICE_STATIC_CONTACT_KIND_DOOR = 2;
}
enum DiceTerminalKind {
  DICE_TERMINAL_KIND_UNSPECIFIED = 0;
  DICE_TERMINAL_KIND_SETTLED = 1;
  DICE_TERMINAL_KIND_OFF_TABLE = 2;
}
message Vector3 { float x = 1; float y = 2; float z = 3; }
message Quaternion { float x = 1; float y = 2; float z = 3; float w = 4; }
message RigidBodyState {
  Vector3 position = 1;
  Quaternion rotation = 2;
  Vector3 linear_velocity = 3;
  Vector3 angular_velocity = 4;
}
message DiceBodyInitial {
  string die_id = 1;
  DiceShape shape = 2;
  RigidBodyState state = 3;
}
message StaticColliderContact {
  DiceStaticContactKind kind = 1;
  string collider_id = 2;
}
message DiceBodyCheckpoint { string die_id = 1; RigidBodyState state = 2; }
message ContactCheckpoint {
  uint32 step = 1;
  string primary_die_id = 2;
  oneof target {
    StaticColliderContact static_collider = 3;
    string other_die_id = 4;
  }
  repeated DiceBodyCheckpoint after = 5;
}
message DiceBodyTerminal {
  string die_id = 1;
  uint32 step = 2;
  DiceTerminalKind kind = 3;
  RigidBodyState state = 4;
}
message ThrowTerminal { repeated DiceBodyTerminal dice = 1; }
message DiceThrowDraft {
  uint32 schema_version = 1;
  string presentation_id = 2;
  uint64 authority_seq = 3;
  uint32 attempt = 4;
  DicePhysicsSchema physics_schema = 5;
  bytes collider_fingerprint = 6;
  repeated DiceBodyInitial bodies = 7;
  repeated ContactCheckpoint contacts = 8;
  ThrowTerminal terminal = 9;
}
message DiceThrowPlan {
  uint32 schema_version = 1;
  string session = 2;
  string presentation_id = 3;
  uint64 authority_seq = 4;
  string roller = 5;
  uint32 attempt = 6;
  DicePhysicsSchema physics_schema = 7;
  bytes collider_fingerprint = 8;
  repeated DiceBodyInitial bodies = 9;
  repeated ContactCheckpoint contacts = 10;
  ThrowTerminal terminal = 11;
}
message PublishDiceThrowRequest { string session = 1; string member = 2; DiceThrowDraft draft = 3; }
message PublishDiceThrowResponse { DiceThrowPlan plan = 1; }
message StreamDiceThrowsRequest { string session = 1; string member = 2; }
service SessionPresentationService {
  rpc PublishDiceThrow(PublishDiceThrowRequest) returns (PublishDiceThrowResponse);
  rpc StreamDiceThrows(StreamDiceThrowsRequest) returns (stream DiceThrowPlan);
}
```

Add comments from the approved design directly above every enum/message/RPC, especially live-only/no-replay, server-bound roller, no outcome authority, fixed bounds, and future-shape additive schema semantics.

- [ ] **Step 2: Run contract gates**

```bash
buf format -w
buf lint
buf format --diff --exit-code
buf breaking --against 'https://github.com/KirkDiggler/rpg-api-protos.git#branch=main'
buf generate
make mocks
```

Expected: all pass; breaking is clean because this is additive. Do not add hand-written protobuf serialization tests.

- [ ] **Step 3: Update living contract docs**

Document the service as **in flight with both consumers assigned**, not as an unused service. Record the exact package, two RPCs, live-only Redis-backed intended host, group bounds, authority absences, and consumer issues. Add one quality row whose current evidence is contract generation only; do not claim runtime confidence before API/web land.

- [ ] **Step 4: Commit and push**

```bash
git add dnd5e/api/session/presentation/v1alpha1/service.proto docs

git commit -m "feat: add shared dice presentation contract (#${PROTO_ISSUE})"
buf lint && buf format --diff --exit-code && \
  buf breaking --against 'https://github.com/KirkDiggler/rpg-api-protos.git#branch=main' && \
  buf generate
git push -u origin feat/${PROTO_ISSUE}-shared-dice-presentation
```

- [ ] **Step 5: Open/review/publish the proto PR**

Open one PR to `main`, link `rpg-project#289/#303`, request exactly one Copilot round, answer every thread, and rerun the contract gates. Kirk merges this provider PR. Verify the generated-branch commit and new root tag by read-back before consumers pin it:

```bash
gh release list -R KirkDiggler/rpg-api-protos --limit 3
git ls-remote origin refs/heads/generated
```

Record the exact artifacts for later tasks:

```bash
PROTO_ROOT_TAG=$(gh release list -R KirkDiggler/rpg-api-protos --limit 1 --json tagName --jq '.[0].tagName')
PROTO_GENERATED_COMMIT=$(git ls-remote origin refs/heads/generated | cut -f1)
printf '%s\n' "$PROTO_ROOT_TAG" > /tmp/shared-dice-proto-tag
printf '%s\n' "$PROTO_GENERATED_COMMIT" > /tmp/shared-dice-proto-generated-commit
```

Verify the root tag contains the merged proto PR and generated commit was created by that merge, then record both in this plan's execution progress and both consumer PR bodies.

---

### Task 3: Extract one reusable session ownership/seating gate in the game server

**Files:**
- Create: `internal/handlers/dnd5e/sessionaccess/access.go`
- Create: `internal/handlers/dnd5e/sessionaccess/access_test.go`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/handler.go`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/get_doors.go`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/ownership_test.go`

**Interfaces:**
- Produces:

```go
type Access struct { /* character + roster repositories */ }
func New(characters characterrepo.Repository, roster rosterrepo.Repository) (*Access, error)
func (a *Access) CallerActingAs(ctx context.Context, member string) error
func (a *Access) CallerSeated(ctx context.Context, session string) error
func (a *Access) CallerMemberSeated(ctx context.Context, session, member string) error
```

- [ ] **Step 1: Write RED access tests**

Add table tests proving unauthenticated, empty session/member, missing character, foreign owner, missing roster, member absent from roster, monster row, and successful owned player seat. The combined gate must perform no downstream service call on refusal.

```go
func TestCallerMemberSeated_OwnedPlayerInRosterPasses(t *testing.T) {
    access := newAccessFixture(t, "player-1", "fighter", []roster.Member{{ID: "fighter", Kind: roster.KindPlayer}})
    err := access.CallerMemberSeated(auth.WithPlayerID(context.Background(), "player-1"), "session-1", "fighter")
    require.NoError(t, err)
}
```

- [ ] **Step 2: Run RED**

```bash
go test ./internal/handlers/dnd5e/sessionaccess ./internal/handlers/dnd5e/session/v1alpha1
```

Expected: FAIL because `sessionaccess` does not exist.

- [ ] **Step 3: Implement and delegate existing session gates**

Move the authenticated-player, ownership, and roster-seat mechanics into `sessionaccess.Access`. Keep current gRPC status codes/text. Construct one `Access` in the existing session handler and make `callerActingAs`/`callerSeated` thin delegates so every existing handler test remains a behavior pin.

- [ ] **Step 4: Run GREEN and ownership regression**

```bash
go test ./internal/handlers/dnd5e/sessionaccess ./internal/handlers/dnd5e/session/v1alpha1 -run 'Access|Ownership|Seated|EveryMemberTakingVerb'
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add internal/handlers/dnd5e/sessionaccess internal/handlers/dnd5e/session/v1alpha1
git commit -m "refactor(session): share member presentation access gates (#${API_ISSUE})"
```

---

### Task 4: Define and validate the proto-free presentation domain

**Files:**
- Create: `internal/orchestrators/sessionpresentation/types.go`
- Create: `internal/orchestrators/sessionpresentation/validation.go`
- Create: `internal/orchestrators/sessionpresentation/validation_test.go`
- Create: `internal/orchestrators/sessionpresentation/service.go`
- Generate: `internal/orchestrators/sessionpresentation/mock/mock_service.go`

**Interfaces:**
- Produces proto-free `Vector3`, `Quaternion`, `RigidBodyState`, `BodyInitial`, `ContactCheckpoint`, `BodyTerminal`, `Draft`, `Plan`; `ValidateDraft`; and:

```go
type PublishInput struct { Session, Member string; Draft Draft }
type PublishOutput struct { Plan Plan }
type SubscribeInput struct { Session, Member string }
type Subscription interface { Plans() <-chan Plan; Close() error }
type Service interface {
    Publish(context.Context, *PublishInput) (*PublishOutput, error)
    Subscribe(context.Context, *SubscribeInput) (Subscription, error)
}
```

- [ ] **Step 1: Write RED validator tables**

Cover every approved bound, including 20 bodies, D20-only first schema, duplicate IDs, normalized quaternions, vector/speed limits, 128 contacts, 256 checkpoint states, exact static/die involved-body lists, per-body terminal completeness, contact-before-terminal, attempt 1–32, 32-byte fingerprint, and deterministic JSON payload no larger than 64 KiB.

```go
func TestValidateDraft_RejectsDieCheckpointWithoutBothBodies(t *testing.T) {
    draft := validTwoBodyDraft()
    draft.Contacts[0].After = draft.Contacts[0].After[:1]
    require.ErrorIs(t, ValidateDraft(&draft), ErrInvalidPlan)
}
```

- [ ] **Step 2: Run RED**

```bash
go test ./internal/orchestrators/sessionpresentation
```

Expected: FAIL with missing package/types.

- [ ] **Step 3: Implement immutable validation**

Validate exact enum values and copy every accepted slice/byte field before returning a normalized draft. Do not clamp. Use `math.IsNaN/IsInf`, magnitude checks, `abs(norm-1) <= 0.0001`, and encoded-size check after normalization. Define package errors `ErrInvalidPlan`, `ErrConflict`, and `ErrClosed`.

- [ ] **Step 4: Generate mock and run GREEN**

```bash
go generate ./internal/orchestrators/sessionpresentation
go test ./internal/orchestrators/sessionpresentation
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add internal/orchestrators/sessionpresentation
git commit -m "feat(session-presentation): validate shared throw plans (#${API_ISSUE})"
```

---

### Task 5: Build Redis first-valid dedupe and cross-instance Pub/Sub

**Files:**
- Create: `internal/repositories/sessionpresentation/repository.go`
- Create: `internal/repositories/sessionpresentation/redis.go`
- Create: `internal/repositories/sessionpresentation/redis_test.go`
- Create: `internal/orchestrators/sessionpresentation/orchestrator.go`
- Create: `internal/orchestrators/sessionpresentation/orchestrator_test.go`

**Interfaces:**
- Repository consumes opaque deterministic plan bytes keyed by session/presentation/attempt and produces live subscription payloads.
- `sessionpresentation.New(repository)` implements the Task 4 `Service`.

- [ ] **Step 1: Write RED repository tests**

Use miniredis and two Redis clients. Prove first publish reaches both subscriptions, equal retry returns the existing bytes without republish, conflict returns `ErrConflict`, different attempt publishes, cancellation closes, and two repository instances communicate.

- [ ] **Step 2: Run RED**

```bash
go test ./internal/repositories/sessionpresentation ./internal/orchestrators/sessionpresentation
```

Expected: FAIL because repository/orchestrator implementations do not exist.

- [ ] **Step 3: Implement atomic Lua accept+publish**

Use SHA-256 of session for safe channel/key suffixes and this atomic shape:

```lua
local existing = redis.call('GET', KEYS[1])
if existing then
  if existing == ARGV[1] then return 1 end
  return -1
end
redis.call('SET', KEYS[1], ARGV[1], 'PX', ARGV[2])
redis.call('PUBLISH', KEYS[2], ARGV[1])
return 2
```

Interpret `2` as first accepted/published, `1` as equal idempotent retry, and `-1` as conflict. TTL is exactly two minutes. Subscription owns `*redis.PubSub`, copies payload bytes, and closes on context or explicit `Close`.

- [ ] **Step 4: Implement service serialization/server binding**

`Publish` validates and normalizes the draft, constructs `Plan{Session: input.Session, Roller: input.Member}`, JSON-encodes the proto-free struct deterministically, delegates to Redis, and returns the accepted decoded plan. `Subscribe` decodes and revalidates every payload; malformed Pub/Sub payloads are logged/dropped without closing the stream.

- [ ] **Step 5: Run GREEN and commit**

```bash
go test ./internal/repositories/sessionpresentation ./internal/orchestrators/sessionpresentation -race
git add internal/repositories/sessionpresentation internal/orchestrators/sessionpresentation
git commit -m "feat(session-presentation): relay plans through Redis (#${API_ISSUE})"
```

---

### Task 6: Add the presentation gRPC handler and conversion

**Files:**
- Create: `internal/handlers/dnd5e/sessionpresentation/v1alpha1/handler.go`
- Create: `internal/handlers/dnd5e/sessionpresentation/v1alpha1/convert.go`
- Create: `internal/handlers/dnd5e/sessionpresentation/v1alpha1/publish_dice_throw.go`
- Create: `internal/handlers/dnd5e/sessionpresentation/v1alpha1/stream_dice_throws.go`
- Create matching `*_test.go` files.

**Interfaces:**
- Consumes: generated `sessionpresentationpb`, shared `sessionaccess.Access`, and Task 4 `sessionpresentation.Service`.
- Produces: registered server implementation with strict proto↔domain translation.

- [ ] **Step 1: Pin the published generated proto**

```bash
GOPROXY=direct go get github.com/KirkDiggler/rpg-api-protos/gen/go@generated
go mod tidy
grep rpg-api-protos go.mod
```

Verify the resolved pseudo-version points at Task 2's generated commit; do not commit a local `replace`.

- [ ] **Step 2: Write RED handler tests**

Mock `Service` and prove auth/access happens before service calls, server ignores any attempted roller/session fields because draft has none, conversion preserves ordered bodies/contacts/terminals, validation errors map `InvalidArgument`, conflict maps `AlreadyExists`, repository failure maps `Internal`, stream cancellation returns cleanly, and send failure returns the transport error.

- [ ] **Step 3: Implement outside-in handler**

```go
type HandlerConfig struct {
    Service sessionpresentation.Service
    Access  *sessionaccess.Access
}
type Handler struct {
    sessionpresentationpb.UnimplementedSessionPresentationServiceServer
    service sessionpresentation.Service
    access  *sessionaccess.Access
}
```

Both RPCs call `Access.CallerMemberSeated(ctx, session, member)` before service work. `PublishDiceThrow` converts draft, calls exactly one `Publish`, and returns its accepted plan. `StreamDiceThrows` subscribes once and forwards until context/channel closure.

- [ ] **Step 4: Run GREEN**

```bash
go test ./internal/handlers/dnd5e/sessionpresentation/v1alpha1 ./internal/handlers/dnd5e/sessionaccess
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add go.mod go.sum internal/handlers/dnd5e/sessionpresentation
git commit -m "feat(session-presentation): expose shared throw RPCs (#${API_ISSUE})"
```

---

### Task 7: Wire and integration-test the game-server service

**Files:**
- Modify: `cmd/server/server.go`
- Modify: `internal/integration/harness/harness.go`
- Create: `internal/integration/sessionpresentation/shared_throw_test.go`
- Create: `docs/architecture/components/session-presentation.md`
- Modify: `docs/architecture/overview.md`
- Modify: `docs/status.md`
- Modify: `docs/quality.md`

**Interfaces:**
- Produces: production and test registration; cross-instance acceptance proof.

- [ ] **Step 1: Write RED harness/integration test**

Start two test servers against one Redis container, seat Alice/Bob through server A's real lobby launch path, open one presentation stream through each server, publish Alice's valid one-body plan through server A, and assert byte-equal plans arrive through both server A and server B. Assert no session Story sequence changes before/after publish. The harness must use Redis-backed character and roster repositories shared by both servers so server B proves the same production ownership/seating gate rather than a test-only bypass.

- [ ] **Step 2: Run RED**

```bash
go test ./internal/integration/sessionpresentation -run TestSharedThrowCrossesServerInstances -v
```

Expected: FAIL because service is not registered/wired.

- [ ] **Step 3: Wire production and harness**

Construct one Redis presentation repository/orchestrator, one shared `sessionaccess.Access`, and the new handler beside `SessionService`. Register `SessionPresentationService` and health name `dnd5e.api.session.presentation.v1alpha1.SessionPresentationService`. In `TestServer`, add `SessionPresentationClient` and `RosterRepo`; replace the inline in-memory lobby roster with `rosterrepo.NewRedis(ts.redisClient, 24*time.Hour)` and pass that same repository to lobby launch and presentation access. Close subscriptions through context/server shutdown.

- [ ] **Step 4: Run GREEN plus full API gates**

```bash
go test ./internal/integration/sessionpresentation -race -count=3
make pre-commit
make ci-check
git diff --check
```

Expected: all pass.

- [ ] **Step 5: Update docs, commit, push, and open PR**

Document exact auth, Redis live-only behavior, no Story/toolkit calls, limits, health registration, and current confidence. Then:

```bash
git add cmd/server internal docs go.mod go.sum
git commit -m "feat(session-presentation): broadcast shared dungeon throws (#${API_ISSUE})"
make ci-check
git push -u origin feat/${API_ISSUE}-shared-dice-presentation
```

Open one PR to `dev`, link journey/design/proto PR, request one Copilot round, answer every thread, and leave it open for integrated web evidence.

---

### Task 8: Add immutable web plan types and canonical collision snapshots

**Files:**
- Modify: `package.json`
- Modify: `package-lock.json`
- Create: `src/components/ui/dice/dungeonDicePhysicsSchema.ts`
- Create: `src/components/ui/dice/dungeonDicePhysicsSchema.test.ts`
- Create: `src/components/ui/dice/dungeonDicePlan.ts`
- Create: `src/components/ui/dice/dungeonDicePlan.test.ts`
- Create: `src/components/ui/dice/dungeonDiceWorld.ts`
- Create: `src/components/ui/dice/dungeonDiceWorld.test.ts`

**Interfaces:**
- Produces:

```ts
export interface DungeonDicePlan { /* immutable approved group shape */ }
export function parseDungeonDicePlan(value: unknown): DungeonDicePlan | undefined;
export function planFromProto(value: DiceThrowPlan): DungeonDicePlan | undefined;
export function draftToProto(value: DungeonDiceDraft): DiceThrowDraft;
export function buildDungeonDiceWorldSnapshot(input: SnapshotInput): DungeonDiceWorldSnapshot;
export async function fingerprintDungeonDiceWorld(snapshot: DungeonDiceWorldSnapshot): Promise<Uint8Array>;
```

- [ ] **Step 1: Install exact dependencies**

```bash
PROTO_ROOT_TAG=$(cat /tmp/shared-dice-proto-tag)
PROTO_GENERATED_COMMIT=$(cat /tmp/shared-dice-proto-generated-commit)
npm i --save "github:KirkDiggler/rpg-api-protos#${PROTO_ROOT_TAG}"
npm i --save @dimforge/rapier3d-compat@0.19.2
rg "$PROTO_GENERATED_COMMIT" package-lock.json
```

Verify `package-lock.json` resolves the recorded generated source commit and one Rapier 0.19.2 copy.

- [ ] **Step 2: Write RED strict parser/schema tests**

Port every server bound into hostile-input table tests. Include deep freeze/caller mutation, exact body/terminal sets, die-to-die involved states, all-d20 first schema, and no forbidden authority/raw-pointer keys.

```ts
expect(JSON.stringify(parsed)).not.toMatch(/result|damage|target|pointer|clientX|samples/);
expect(Object.isFrozen(parsed?.bodies)).toBe(true);
```

- [ ] **Step 3: Write RED snapshot/fingerprint tests**

Build fixture scenes proving deterministic sort independent of map insertion order, stable floor/wall/door IDs, open-door omission, shut/locked inclusion, prop/entity omission, body descriptor inclusion, two-body distinct IDs, and one fixed expected SHA-256 hex.

- [ ] **Step 4: Implement minimum immutable domain/snapshot**

Move the useful logic from concept `dungeonDiceColliders.ts`/`dungeonDiceInteraction.ts` rather than importing it. Define one schema constants record consumed later by raw and visible worlds. Canonical fingerprint bytes use explicit UTF-8 records with fixed decimal/field ordering; never `JSON.stringify(Map)`.

- [ ] **Step 5: Run GREEN and commit**

```bash
npm run test:run -- \
  src/components/ui/dice/dungeonDicePhysicsSchema.test.ts \
  src/components/ui/dice/dungeonDicePlan.test.ts \
  src/components/ui/dice/dungeonDiceWorld.test.ts

git add package.json package-lock.json src/components/ui/dice/dungeonDice*
git commit -m "feat(dice): define shared dungeon throw plans (#${WEB_ISSUE})"
```

---

### Task 9: Generate deterministic multi-body plans with raw Rapier

**Files:**
- Create: `src/components/ui/dice/rapierLoader.ts`
- Create: `src/components/ui/dice/rapierLoader.test.ts`
- Create: `src/components/ui/dice/dungeonDicePlanGenerator.ts`
- Create: `src/components/ui/dice/dungeonDicePlanGenerator.test.ts`

**Interfaces:**
- Produces:

```ts
export type DungeonDicePlanGeneration =
  | Readonly<{ kind: 'plan'; draft: DungeonDiceDraft }>
  | Readonly<{ kind: 'fallback'; reason: string }>;
export async function generateDungeonDiceThrowPlan(input: Readonly<{
  presentationId: string;
  authoritySeq: bigint;
  attempt: number;
  snapshot: DungeonDiceWorldSnapshot;
  bodies: readonly DungeonDiceBodyInitial[];
}>): Promise<DungeonDicePlanGeneration>;
```

- [ ] **Step 1: Write RED loader/generator tests**

Prove import/init coalescing, disposal, one d20 wall contact, shut-door contact, open-door pass, off-table, low-energy settled, two d20 die-to-die checkpoint with exactly two `after` states, mixed terminals, deterministic byte-equal output, and fallback at 480/129 contacts/257 states.

- [ ] **Step 2: Run RED**

```bash
npm run test:run -- \
  src/components/ui/dice/rapierLoader.test.ts \
  src/components/ui/dice/dungeonDicePlanGenerator.test.ts
```

Expected: FAIL with missing modules.

- [ ] **Step 3: Implement raw fixed-step world**

Dynamically import `@dimforge/rapier3d-compat`, coalesce `init()`, construct fixed cuboids and convex-hull d20 bodies from the shared schema, enable collision events, and map collider handles to stable IDs/body IDs. Drain started wall/door/die contacts after each `world.step(eventQueue)`, snapshot involved post-step states, remove off-table bodies, and stop only when every body terminals or a bound fails.

- [ ] **Step 4: Run GREEN and leak regression**

```bash
npm run test:run -- src/components/ui/dice/dungeonDicePlanGenerator.test.ts --reporter=verbose
for run in 1 2 3 4 5; do
  npm run test:run -- src/components/ui/dice/dungeonDicePlanGenerator.test.ts || exit 1
done
```

Expected: deterministic pass and no retained worlds/callbacks.

- [ ] **Step 5: Commit**

```bash
git add src/components/ui/dice/rapierLoader* src/components/ui/dice/dungeonDicePlanGenerator*
git commit -m "feat(dice): pre-simulate shared rigid-body throws (#${WEB_ISSUE})"
```

---

### Task 10: Consume the presentation publish/live stream in the web

**Files:**
- Modify: `src/api/client.ts`
- Create: `src/api/useSessionDiceThrows.ts`
- Create: `src/api/useSessionDiceThrows.test.ts`

**Interfaces:**
- Produces:

```ts
export const sessionPresentationClient: Client<typeof SessionPresentationService>;
export interface UseSessionDiceThrowsResult {
  publish(draft: DungeonDiceDraft): Promise<DungeonDicePlan>;
  state: 'connecting' | 'live' | 'reconnecting' | 'stopped';
}
export function useSessionDiceThrows(input: {
  session: string;
  member: string;
  onPlan(plan: DungeonDicePlan): void;
}): UseSessionDiceThrowsResult;
```

- [ ] **Step 1: Write RED client/hook tests**

Mock the generated client. Pin auth client export, publish conversion/strict response parse, live plan delivery, malformed drop, reconnect with bounded backoff, scope reset, StrictMode single active stream, late prior-scope fencing, and clean AbortController cancellation. No replay/Get call exists.

- [ ] **Step 2: Run RED**

```bash
npm run test:run -- src/api/useSessionDiceThrows.test.ts src/api/client.test.ts
```

Expected: FAIL with missing client/hook.

- [ ] **Step 3: Implement live-only transport**

Follow `useSessionEventStream` lifecycle fencing but omit sequence/gap/GetStory machinery. Log only sanitized presentation/attempt/body/contact counts in development. Publish sends `{session, member, draft}` and accepts only a matching server-bound plan.

- [ ] **Step 4: Run GREEN**

```bash
npm run test:run -- src/api/useSessionDiceThrows.test.ts src/api/client.test.ts src/api/streamLogging.test.ts
```

- [ ] **Step 5: Commit**

```bash
git add src/api/client.ts src/api/useSessionDiceThrows*
git commit -m "feat(dice): connect shared throw transport (#${WEB_ISSUE})"
```

---

### Task 11: Add the actor/witness coordinator state machine

**Files:**
- Create: `src/components/session/combat-experience/dungeonDiceCoordinatorState.ts`
- Create: `src/components/session/combat-experience/dungeonDiceCoordinatorState.test.ts`
- Modify: `src/components/session/combat-experience/presentation.ts`
- Modify: `src/components/session/combat-experience/presentation.test.ts`
- Modify: `src/components/session/combat-experience/useCombatPresentation.ts`
- Modify: `src/components/session/combat-experience/useCombatPresentation.test.tsx`
- Modify: `src/components/session/combat-experience/types.ts`

**Interfaces:**
- Produces a pure coordinator reducer and `UseCombatPresentationResult.acceptSharedDicePlan(plan)`.

- [ ] **Step 1: Write RED coordinator transition tests**

Pin actor path `armed-tray → held-tray → held-world → planning → waiting-start → playing → settled`; off-table returns to `armed-tray` with incremented attempt and revealed Story; witness path waits for plan; plan-before-event 3-second buffer; event-before-plan 10-second fallback; publish failure 750 ms; duplicate/conflict/late/fallback plans ignored; an off-table terminal on attempt 32 completes semantically instead of arming attempt 33; scope changes dispose.

- [ ] **Step 2: Write RED authority reducer tests**

Prove first accepted actor plan creates the existing local release event, matching witness plan creates one neutral internal release, physics never supplies result, off-table does not create a second Story reveal, catch-up settles immediately, and current response/event conflict behavior remains fail-closed.

- [ ] **Step 3: Run RED**

```bash
npm run test:run -- \
  src/components/session/combat-experience/dungeonDiceCoordinatorState.test.ts \
  src/components/session/combat-experience/presentation.test.ts \
  src/components/session/combat-experience/useCombatPresentation.test.tsx
```

- [ ] **Step 4: Implement minimum state/reducer integration**

Add a `shared-plan-release` fact containing only presentation ID/authority seq/roller/attempt. Convert it to the same strict `DicePresentationReleasedEvent` used today, using the actor's local profile when present and deterministic neutral profile for witnesses. Keep full plan state outside `CombatPresentationState`.

- [ ] **Step 5: Run GREEN and commit**

```bash
npm run test:run -- src/components/session/combat-experience/{dungeonDiceCoordinatorState,presentation,useCombatPresentation}*.test*
git add src/components/session/combat-experience
git commit -m "feat(combat): coordinate shared dice attempts (#${WEB_ISSUE})"
```

---

### Task 12: Play plans in the production SessionCanvas

**Files:**
- Create: `src/components/ui/dice/dungeonDicePlayback.ts`
- Create: `src/components/ui/dice/dungeonDicePlayback.test.ts`
- Create: `src/components/ui/dice/DungeonDicePhysicsLayer.tsx`
- Create: `src/components/ui/dice/DungeonDicePhysicsLayer.test.tsx`
- Modify: `src/components/session/SessionCanvas.tsx`
- Modify: `src/components/session/SessionCanvas.test.tsx`

**Interfaces:**
- Produces:

```ts
export interface DungeonDicePhysicsLayerProps {
  plan: DungeonDicePlan;
  authoritativeFaces: Readonly<Record<string, number>>;
  snapshot: DungeonDiceWorldSnapshot;
  projectionRef: MutableRefObject<TrayPlaneProjection | undefined>;
  reducedMotion: boolean;
  onContact(event: DungeonDiceObservedContact): void;
  onTerminal(event: DungeonDiceObservedTerminal): void;
  onFailure(reason: string): void;
}
```

- [ ] **Step 1: Write RED pure playback tests**

Given observed states, prove step monotonicity, one-time checkpoint application, static one-body correction, die pair two-body correction, per-body terminal removal/freeze, mixed terminal completion, stale generation fencing, and exact sanitized diagnostics.

- [ ] **Step 2: Write RED R3F component tests**

Mock/load Rapier and runtime provider to prove one Physics world, static snapshot colliders, one verified runtime mesh per body, no prop/entity colliders, fixed `1/60`, checkpoint state application, off-table callback, settled face-assist completion, and reduced-motion direct completion.

- [ ] **Step 3: Run RED**

```bash
npm run test:run -- \
  src/components/ui/dice/dungeonDicePlayback.test.ts \
  src/components/ui/dice/DungeonDicePhysicsLayer.test.tsx \
  src/components/session/SessionCanvas.test.tsx
```

- [ ] **Step 4: Implement lazy visible layer**

Use `Physics timeStep={1/60}` and `useAfterPhysicsStep` for plan step ownership. At each contact set translation/rotation/linvel/angvel for exactly `after`. At settled terminal perform the extracted 320 ms target slerp from the verified face map; at off-table remove/notify. Mount `TrayPlaneProjectionBridge` from the actual SessionCanvas camera. Export the layer through a dynamic module boundary; `SessionCanvas` retains only generic `presentationLayer` ownership.

- [ ] **Step 5: Run GREEN and commit**

```bash
npm run test:run -- \
  src/components/ui/dice/dungeonDicePlayback.test.ts \
  src/components/ui/dice/DungeonDicePhysicsLayer.test.tsx \
  src/components/session/SessionCanvas.test.tsx

git add src/components/ui/dice/dungeonDicePlayback* \
  src/components/ui/dice/DungeonDicePhysicsLayer* src/components/session/SessionCanvas*
git commit -m "feat(dice): play shared throws in SessionCanvas (#${WEB_ISSUE})"
```

---

### Task 13: Bridge the existing drawer gesture into world-space carry

**Files:**
- Create: `src/components/ui/dice/dungeonDiceGestureBridge.ts`
- Create: `src/components/ui/dice/dungeonDiceGestureBridge.test.ts`
- Modify: `src/components/ui/dice/DiceTrayInteractionSurface.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.tsx`
- Modify: `src/components/ui/dice/DiceTray3D.test.tsx`
- Modify: `src/components/ui/dice/DiceTrayPresentation.tsx`
- Modify: `src/components/ui/dice/DiceTrayPresentation.test.tsx`

**Interfaces:**
- Produces:

```ts
export interface DungeonDiceWorldGestureBridge {
  begin(sample: RollGroupPointerSample, capture: PointerCaptureOwner): boolean;
  move(sample: RollGroupPointerSample, buttons: number): DungeonDiceHeldWorldState | undefined;
  release(sample: RollGroupPointerSample): DungeonDiceLaunchInput | undefined;
  cancel(pointerId: number): void;
  neutralLaunch(): DungeonDiceLaunchInput;
}
```

- [ ] **Step 1: Write RED gesture tests**

Prove begin only after existing d20 hit acceptance, tray-held movement stays local, first valid map projection performs one handoff, invalid map/release returns tray, left controls X/Z, right+left freezes X/Z and adjusts bounded height, right release resumes X/Z, left release commits once, outside capture works, context menu is suppressed only while active, and serialized launch contains no raw pointer keys.

- [ ] **Step 2: Run RED**

```bash
npm run test:run -- \
  src/components/ui/dice/dungeonDiceGestureBridge.test.ts \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx
```

- [ ] **Step 3: Implement optional bridge without legacy change**

`DiceTrayInteractionSurface` first performs its existing hit/controller begin. When an optional world bridge accepts that grab, it continues pointer capture but routes move/release/cancel to the bridge and does not call the legacy release callback. Without the prop, byte-for-behavior legacy choreography remains. `DiceTray3D` hides its die only after bridge state becomes world-held; before that it renders the actual carved die.

- [ ] **Step 4: Run GREEN plus exact attachment regressions**

```bash
npm run test:run -- \
  src/components/ui/dice/dungeonDiceGestureBridge.test.ts \
  src/components/ui/dice/DiceTray3D.test.tsx \
  src/components/ui/dice/DiceTrayPresentation.test.tsx \
  src/components/ui/dice/anchoredRollGroupGestureController.test.ts \
  src/components/ui/dice/trayPlaneProjection.test.ts
```

- [ ] **Step 5: Commit**

```bash
git add src/components/ui/dice
git commit -m "feat(dice): hand drawer gestures into the dungeon (#${WEB_ISSUE})"
```

---

### Task 14: Integrate transport, drawer, map, fallback, and concept reuse

**Files:**
- Create: `src/components/session/combat-experience/useDungeonDiceCoordinator.ts`
- Create: `src/components/session/combat-experience/useDungeonDiceCoordinator.test.tsx`
- Modify: `src/components/session/combat-experience/DiceDrawer.tsx`
- Modify: `src/components/session/combat-experience/CombatExperience.tsx`
- Modify: `src/components/session/combat-experience/CombatExperience.test.tsx`
- Modify: `src/components/session/combat-experience/types.ts`
- Modify: `src/components/session/SessionEncounterView.tsx`
- Modify: `src/components/session/SessionEncounterView.test.tsx`
- Refactor: `src/concepts/attack-die-3d/PhysicsTraySpike.tsx`
- Refactor: `src/concepts/attack-die-3d/DungeonFloorPhysicsSpike.tsx`
- Refactor: `src/concepts/attack-die-3d/dungeonDiceColliders.ts`
- Refactor: `src/concepts/attack-die-3d/dungeonDiceInteraction.ts`
- Create: `src/components/ui/dice/dungeonDiceProductionBoundary.test.ts`
- Modify: `src/components/session/combat-experience/CombatExperience.module.css`

**Interfaces:**
- Consumes Tasks 8–13 and wires one production d20 body/result.
- Produces complete production route behavior and sanitized diagnostics.

- [ ] **Step 1: Write RED coordinator integration tests**

Mock plan generation/publish/stream/physics. Prove actor plan publish freezes held state, accepted response starts after 150 ms, stream echo dedupes, witness matching plan starts, fingerprint mismatch refetches doors once then falls back after 500 ms, publish failure starts local after 750 ms, off-table returns both drawer states and increments attempts 1–31, valid retry settles, attempt-32 off-table semantic-settles without publishing attempt 33, reconnect/history never replays, and scope changes dispose every timer/world/subscription.

- [ ] **Step 2: Write RED production composition/boundary tests**

Render `SessionEncounterView` and assert the drawer receives the bridge only for authoritative roller; spectators have no gesture callbacks; SessionCanvas receives the lazy presentation layer; props/entities remain ordinary render children; Roll button calls neutral plan path; reduced motion completes; concept modules import shared units; production source contains no `src/concepts` import; initial bundle source has no eager `@react-three/rapier` entry from the production route.

- [ ] **Step 3: Run RED**

```bash
npm run test:run -- \
  src/components/session/combat-experience/useDungeonDiceCoordinator.test.tsx \
  src/components/session/combat-experience/CombatExperience.test.tsx \
  src/components/session/SessionEncounterView.test.tsx \
  src/components/ui/dice/dungeonDiceProductionBoundary.test.ts
```

- [ ] **Step 4: Implement orchestration and fallback**

Instantiate `useDungeonDiceCoordinator` beside `useSessionCombatExperience` with last-good scene, live doors, refetchDoors, current dice request/events/result, witness role, and transport. Feed bridge/status into `CombatExperience`; feed projection/lazy physics layer into `SessionCanvas.presentationLayer`. During world carry/playback, drawer remains visible with clear status and no duplicate die. Keep semantic release button available on failure. Refactor concept spikes to shared snapshot/body/predicate implementations while retaining their development controls.

- [ ] **Step 5: Run focused GREEN and commit**

```bash
npm run test:run -- \
  src/components/session/combat-experience \
  src/components/session/SessionEncounterView.test.tsx \
  src/components/session/SessionCanvas.test.tsx \
  src/components/ui/dice/dungeonDiceProductionBoundary.test.ts \
  src/concepts/attack-die-3d/dungeonDiceColliders.test.ts \
  src/concepts/attack-die-3d/dungeonDiceInteraction.test.ts

git add src/components src/concepts public/themes/base.css
git commit -m "feat(session): share physical dungeon dice throws (#${WEB_ISSUE})"
```

---

### Task 15: Complete web docs, automated gates, and reproducible evidence tooling

**Files:**
- Create: `scripts/attack-die/capture-shared-dungeon-throw-evidence.mjs`
- Create: `docs/evidence/${WEB_ISSUE}-shared-dungeon-throws/README.md`
- Create: `docs/architecture/components/session-dice-presentation.md`
- Modify: `docs/how-to/attack-die-3d-concept.md`
- Modify: `docs/status.md`
- Modify: `docs/quality.md`

**Interfaces:**
- Produces: public-safe diagnostic/evidence procedure; no licensed/private artifacts committed.

- [ ] **Step 1: Write the evidence script assertions**

Drive deterministic development hooks for wall, shut-door, open-door, off-table, valid retry, missing stream, reconnect, Roll button, and reduced motion. For two panes/contexts assert equal plan ID/attempt/fingerprint/planned contact IDs+steps/terminal/authoritative face, zero raw pointer fields, and zero page/console/request failures. Save screenshots/video only to ignored `/tmp` or private evidence directory.

- [ ] **Step 2: Run focused and full web gates**

```bash
npm run format
npm run test:run -- \
  src/components/ui/dice/dungeonDicePhysicsSchema.test.ts \
  src/components/ui/dice/dungeonDicePlan.test.ts \
  src/components/ui/dice/dungeonDiceWorld.test.ts \
  src/components/ui/dice/dungeonDicePlanGenerator.test.ts \
  src/components/ui/dice/dungeonDicePlayback.test.ts \
  src/components/ui/dice/DungeonDicePhysicsLayer.test.tsx \
  src/components/ui/dice/dungeonDiceGestureBridge.test.ts \
  src/components/session/combat-experience/dungeonDiceCoordinatorState.test.ts \
  src/components/session/combat-experience/useDungeonDiceCoordinator.test.tsx
npm run ci-check
git diff --check
```

Expected: all pass. Record exact test counts and build chunk observations without declaring a performance budget.

- [ ] **Step 3: Update docs honestly**

Document product route, schema/transport ownership, failure/reconnect, default d20-only product, group-shaped future seam, dynamic loading, public-safe diagnostics, and what remains unverified until two authenticated browsers. Name the concept's eager architecture as retired for production, not deleted history.

- [ ] **Step 4: Commit, push, and open web PR**

```bash
git add package.json package-lock.json src scripts docs public/themes/base.css
git commit -m "docs(dice): record shared dungeon throw verification (#${WEB_ISSUE})"
npm run ci-check
git push -u origin feat/${WEB_ISSUE}-shared-dungeon-dice
```

Open one PR to `dev`, link journey/design/proto/API PRs, request one Copilot round, answer every thread, and keep it open for the integrated walk.

---

### Task 16: Run the real two-browser integrated gate

**Files:**
- Amend only failing owning-repository branches discovered by integration.
- Update API/web PR bodies and web evidence README with verified facts.

**Interfaces:**
- Consumes: published proto, API branch image, web branch Vite build, two independent auth contexts.
- Produces: Kirk-reviewable shared throw evidence.

- [ ] **Step 1: Build the API branch image and run the local stack**

```bash
API_WT=/home/kirk/.pi/worktrees/rpg-api/${API_ISSUE}-shared-dice-presentation
WEB_WT=/home/kirk/.pi/worktrees/rpg-dnd5e-web/${WEB_ISSUE}-shared-dungeon-dice

docker build -t rpg-api:local "$API_WT"
docker compose \
  -f /home/kirk/game-dev/rpg-deployment/docker-compose.local-dev.yml \
  -f /home/kirk/game-dev/rpg-deployment/docker-compose.local-api-src.yml \
  up -d rpg-api

cd "$WEB_WT"
npm run dev -- --host 127.0.0.1 --port 3010
```

Verify API health/reflection includes `SessionPresentationService` before browser work.

- [ ] **Step 2: Seat two independent authenticated players**

Use the production lobby/session path with `toolkit-sandbox-fighter` and `toolkit-sandbox-barbarian` in separate browser contexts. Do not reuse cookies/local storage. Start the reference tomb and reach a player attack declaration.

- [ ] **Step 3: Execute all required shared throws**

Observe and record:

1. authored wall: equal `wall:${wallRunKey}` checkpoint and visible bounce;
2. shut/locked door: equal `door:${connectionId}` checkpoint and visible bounce;
3. open that door through the real verb: matching new fingerprint and no door checkpoint;
4. off-table: both visibly leave, return to tray, and await attempt 2;
5. attempt 2 valid settlement: equal authoritative face;
6. presentation stream interruption: bounded truthful fallback with combat still usable;
7. reconnect after release: no stale replay;
8. Roll button and reduced-motion completion;
9. zero page/console/request failures except the intentionally interrupted presentation stream.

- [ ] **Step 4: Run the automated evidence script and fresh full gates**

```bash
cd "$WEB_WT"
node scripts/attack-die/capture-shared-dungeon-throw-evidence.mjs \
  --base-url http://127.0.0.1:3010 \
  --out /tmp/shared-dungeon-throw-evidence
npm run ci-check

cd "$API_WT"
make ci-check
```

Compare roller/witness sanitized manifests. Both must name equal contacts and terminals.

- [ ] **Step 5: Present exact heads to Kirk for the feel gate**

Report API/web commit IDs, URLs, test output, plan/contact evidence, known face-assist/tray-art debt, and private screenshot path. Kirk repeats wall/door/off-table throws and rules on connected feel, same-bounce conversation, synchronization delay, retry clarity, and face assist. Do not merge consumers before this approval.

---

### Task 17: Close review rounds and publish inside-out

**Files:**
- PR/issue/Project 19 metadata and the existing design/plan status lines.

**Interfaces:**
- Produces: reviewed/merged provider→consumer chain and final journey record.

- [ ] **Step 1: Close every Copilot round**

For each implementation PR, count top-level inline findings and replies using the repository pull-comments endpoint. Apply valid findings on the same branch, check the same shape elsewhere, reply in every thread with commit/test evidence, decline invalid findings technically, and never re-request.

- [ ] **Step 2: Re-run final gates on exact PR heads**

```bash
# protos
buf lint && buf format --diff --exit-code && \
  buf breaking --against 'https://github.com/KirkDiggler/rpg-api-protos.git#branch=main' && buf generate
# api
make ci-check
# web
npm run ci-check
```

Read GitHub checks back and require green exact heads. Run `git diff --check` in all worktrees.

- [ ] **Step 3: Kirk merges inside-out**

Kirk merges proto if not already published, then API to `dev`, then web to `dev`. Feature PRs into `dev` squash. Verify resulting merge commits/tags and refresh the local stack from pure `dev` after each consumer merge.

- [ ] **Step 4: Update the design record and Project 19**

On `design/303-shared-dungeon-dice`, amend design/plan with exact issue/PR numbers, provider tag/generated commit, API/web merge commits, evidence path, Kirk verdict, and any earned server-simulation trigger. Mark implementation issues Done and #303 In Review/Done only when Git, PRs, board, and evidence agree.

- [ ] **Step 5: Merge the rpg-project tracking PR last and clean worktrees**

Kirk merges PR #304 after implementation is complete. Then remove only this wave's clean worktrees/branches and temporary evidence/server process. Preserve unrelated worktrees, ports, containers, and operator continuity files.
