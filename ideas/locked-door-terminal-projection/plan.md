# Locked Door Terminal Projection Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Project the toolkit-owned locked crypt boss door onto the encounter wire while characterizing the existing unlock and terminal encounter behavior.

**Architecture:** `rpg-api` will consume published `encounter/v0.40.0`, whose `CryptDungeonParams` already supplies the boss lock configuration. `doorWallsToProto` remains a pure `DoorData` projection, selecting only the proto wall kind while preserving the existing ID and passage edge. Existing toolkit `AttemptUnlock`/`SubmitCheck` and `EncounterEnded` paths are tested through the API without new rules or terminal detection.

**Tech Stack:** Go 1.24, gRPC/protobuf, `rpg-toolkit/encounter`, Testify suites, shared Redis testcontainers.

## Global Constraints

- Start isolated branch `feat/690-locked-door-terminal-projection` from API `origin/main` `4c740e8`.
- Depend only on published `github.com/KirkDiggler/rpg-toolkit/encounter v0.40.0`; no `replace` or generated source changes.
- `CryptDungeonParams` is canonical: entrance connector is plain; boss connector is locked with DC 12, DEX, and no tool. API must not configure those fields.
- Wall truth table: `Open` -> `WALL_KIND_DOOR_OPEN`; else `Locked` -> `WALL_KIND_DOOR_LOCKED`; else `WALL_KIND_DOOR_CLOSED`.
- Preserve existing door ID, From, and To projection exactly.
- Reuse existing `Interact -> AttemptUnlock -> InputRequired -> SubmitCheck` and toolkit terminal paths. Do not count hostiles, synthesize reasons, or add defeat detection.
- No generation, placement, obstacles, theme, zone, proto source, or web/UI changes.
- Run targeted shared-Redis integration suites only locally; do not run the full integration package locally.
- Record RED, GREEN, mutation discrimination, non-Docker checks, main lint-debt parity, CI container count, and CI suite evidence in the API PR.

---

### Task 1: Create the isolated API baseline and consume the published toolkit tag

**Files:**
- Modify: `go.mod`
- Modify: `go.sum`
- Modify: `.claude/progress.json`
- Modify: `.claude/tests.json`

**Interfaces:**
- Consumes: published `github.com/KirkDiggler/rpg-toolkit/encounter@v0.40.0`, tag commit `29760ef2bcaf08fb0336a90a917f0b43b14f265b`.
- Produces: an isolated, clean API branch able to compile `tkenc.CryptDungeonParams` with canonical lock state.

- [ ] **Step 1: Create the branch from the fetched API baseline**

Run: `git worktree add -b feat/690-locked-door-terminal-projection /home/kirk/game-dev/.claude/worktrees/rpg-api-690-locked-door-terminal-projection origin/main`

Expected: worktree HEAD is `4c740e8`; `git status --short` is empty after removing branch-local `.claude` tracking artifacts.

- [ ] **Step 2: Add a failing compile-level dependency expectation**

In the existing crypt construction test, assert that the production StartEncounter data exposes the toolkit-owned boss lock values. This must fail while the API remains on `encounter/v0.39.0` because that version's `CryptDungeonParams` has no lock configuration.

```go
boss := data.Doors["crypt-door-corridor-boss"]
s.Require().True(boss.Locked)
s.Require().Equal(12, boss.LockDC)
s.Require().Equal("dex", boss.LockAbility)
s.Require().Empty(boss.LockTool)
```

- [ ] **Step 3: Run the focused RED test**

Run: `go test ./internal/orchestrators/lobby -run TestLobbySuite -count=1`

Expected: lock assertion fails against the old dependency; do not change API lock configuration to fix it.

- [ ] **Step 4: Bump only the encounter module**

Run: `GOPROXY=direct go get github.com/KirkDiggler/rpg-toolkit/encounter@v0.40.0 && go mod tidy`

Confirm `go.mod` contains exactly `github.com/KirkDiggler/rpg-toolkit/encounter v0.40.0`, no local `replace`, and `go list -m -json github.com/KirkDiggler/rpg-toolkit/encounter@v0.40.0` resolves tag commit `29760ef2bcaf08fb0336a90a917f0b43b14f265b`.

- [ ] **Step 5: Run the dependency GREEN test**

Run: `go test ./internal/orchestrators/lobby -run TestLobbySuite -count=1`

Expected: PASS and production crypt data proves entrance plain plus boss locked/DC12/dex/no-tool.

- [ ] **Step 6: Record branch progress and commit the dependency proof**

Update the existing `.claude/progress.json` and `.claude/tests.json` only with #690's dependency/tag and focused test result. Commit only `go.mod`, `go.sum`, the changed lock-characterization test, and those targeted artifacts with `chore(deps): adopt encounter v0.40.0 for crypt boss lock`.

### Task 2: Add the pure locked-door projection and truth-table tests

**Files:**
- Modify: `internal/handlers/dnd5e/v2/encounter/project.go:doorWallsToProto`
- Modify: `internal/handlers/dnd5e/v2/encounter/project_test.go:TestProjectFor_DoorWall_ProjectsIdKindAndPassageEdge`

**Interfaces:**
- Consumes: `*tkenc.DoorData{Open, Locked, ID, Position}` and existing `doorPassageNeighbor`.
- Produces: `WALL_KIND_DOOR_OPEN`, `WALL_KIND_DOOR_LOCKED`, or `WALL_KIND_DOOR_CLOSED` without mutating data.

- [ ] **Step 1: Extend the existing door projector test with the complete truth table**

Use three table cases built from the same door geometry and assert `Id`, `From`, and `To` are identical in every case.

```go
tests := []struct {
    name string
    open bool
    locked bool
    want encounterv2pb.WallKind
}{
    {name: "locked closed", locked: true, want: encounterv2pb.WallKind_WALL_KIND_DOOR_LOCKED},
    {name: "unlocked closed", want: encounterv2pb.WallKind_WALL_KIND_DOOR_CLOSED},
    {name: "open wins over stale locked", open: true, locked: true, want: encounterv2pb.WallKind_WALL_KIND_DOOR_OPEN},
}
```

- [ ] **Step 2: Run the focused projector test for RED**

Run: `go test ./internal/handlers/dnd5e/v2/encounter -run 'TestProjectSuite/TestProjectFor_DoorWall_ProjectsIdKindAndPassageEdge' -count=1`

Expected: locked-closed case reports `WALL_KIND_DOOR_CLOSED`, failing the new assertion.

- [ ] **Step 3: Implement the minimal projector precedence**

Replace the current two-state selection with:

```go
kind := encounterv2pb.WallKind_WALL_KIND_DOOR_CLOSED
switch {
case door.Open:
    kind = encounterv2pb.WallKind_WALL_KIND_DOOR_OPEN
case door.Locked:
    kind = encounterv2pb.WallKind_WALL_KIND_DOOR_LOCKED
}
```

Do not edit `doorPassageNeighbor`, IDs, door data, interaction orchestration, or terminal code.

- [ ] **Step 4: Run GREEN and mutation discrimination**

Run the focused test from Step 2 and expect PASS. Temporarily mutate `case door.Locked` to the closed kind, run the same test and confirm it fails, then restore the locked kind and rerun to PASS. Do not commit the mutation.

- [ ] **Step 5: Commit the pure projection slice**

Run `gofmt -w internal/handlers/dnd5e/v2/encounter/project.go internal/handlers/dnd5e/v2/encounter/project_test.go`, then commit these two files with `feat(encounter): project locked door walls`.

### Task 3: Prove the production crypt and existing unlock flow through real RPCs

**Files:**
- Modify: `internal/integration/dungeon_crypt_test.go`

**Interfaces:**
- Consumes: real `LobbyService.StartEncounter`, `EncounterService.Interact`, `EncounterService.SubmitCheck`, shared harness Redis, and toolkit-issued prompt data.
- Produces: regression coverage for canonical production door state, failed check persistence, successful unlock/open persistence, and projected wall/event output.

- [ ] **Step 1: Add the failing real-RPC lock projection test**

Extend `DungeonCryptSuite` to start the compact crypt and identify doors by their known IDs, not map order. Assert the entrance wall is `DOOR_CLOSED`; boss wall is `DOOR_LOCKED`; both retain their IDs and non-degenerate existing edges.

- [ ] **Step 2: Add failing failed/successful check characterizations**

Drive the existing RPC path against the boss ID. Capture the returned skill-check prompt, submit a deterministic failing roll, reload and assert `{Locked:true, Open:false}` plus locked snapshot kind. Retry with a deterministic passing roll, consume live stream events, reload and assert `{Locked:false, Open:true}`, `DOOR_OPEN`, and the existing `DoorOpened` event. Do not inspect or compute DC, ability modifier, or tool bonus in API production code.

- [ ] **Step 3: Run the targeted suite for RED**

Run: `go test -race ./internal/integration -run '^TestDungeonCryptSuite$' -count=1`

Expected before Task 2 is applied: lock wall kind assertion fails. Expected before new assertions are implemented: missing test behavior is unproven.

- [ ] **Step 4: Run GREEN against shared Redis**

Run the same command after Tasks 1-2. Confirm the suite leases the shared Redis container and does not launch per-test containers.

- [ ] **Step 5: Commit the crypt integration gate**

Commit `internal/integration/dungeon_crypt_test.go` with `test(integration): cover locked crypt boss door flow`.

### Task 4: Characterize terminal projection, persistence, and reconnect without new terminal logic

**Files:**
- Modify: `internal/integration/encounter_v2_test.go`
- Modify: `internal/orchestrators/lobby/get_my_active_lobby_test.go`
- Modify: `docs/architecture/components/encounter.md`
- Modify: `docs/architecture/components/lobby-service.md`
- Modify: `docs/status.md`
- Modify: `.claude/progress.json`
- Modify: `.claude/tests.json`

**Interfaces:**
- Consumes: existing toolkit `EncounterEndedEvent` reasons, persisted `core.ModeEnded`, API `translateEncounterEndedEvent`, and `GetMyActiveLobby` liveness lookup.
- Produces: regression evidence that victory and TPK remain toolkit-owned and terminal reconnects are empty rather than resumable.

- [ ] **Step 1: Add failing/absent characterization assertions for terminal reasons**

In the existing real-RPC all-hostiles test, assert the already-translated `EncounterEnded.Reason` equals `tkenc.EncounterEndedReasonAllHostilesDefeated`, the repository snapshot is `ModeEnded`, and a new `GetEncounter` projection is terminal. Add a focused TPK fixture using existing combat/death helpers; assert its stream reason equals `tkenc.EncounterEndedReasonTPK`, hostiles may remain, and no API-side reason synthesis occurs.

- [ ] **Step 2: Add reconnect liveness assertions**

In `get_my_active_lobby_test.go`, characterize both victory-ended and TPK-ended persisted encounters via the existing `GetMyActiveLobby` method. Each must return an empty output because `ModeEnded` is terminal; do not write lobby status transitions.

- [ ] **Step 3: Run focused RED checks where assertions are new**

Run: `go test ./internal/integration -run '^TestEncounterV2' -count=1`

Run: `go test ./internal/orchestrators/lobby -run 'TestLobbySuite/TestGetMyActiveLobby' -count=1`

Expected: new characterization tests initially fail until their test fixtures/assertions accurately exercise the existing behavior; fix fixtures only, never add terminal production logic.

- [ ] **Step 4: Run focused GREEN checks**

Repeat the two commands from Step 3. Expected: PASS, proving terminal reasons were already translated/persisted and terminal lobbies already refuse resume.

- [ ] **Step 5: Update documentation and artifacts**

Update the wall projection section to document the exact three-state precedence and clarify terminal handling as existing toolkit canonical behavior. Update lobby docs/status/progress/tests with evidence, dependency tag, and explicit non-scope: no API lock or terminal rules.

- [ ] **Step 6: Commit characterization and documentation**

Commit the terminal tests/docs/artifacts with `test(encounter): characterize terminal outcomes and reconnect`.

### Task 5: Verify targeted quality, CI, and publish the unmerged PR

**Files:**
- Modify: none unless checks expose a #690 regression

**Interfaces:**
- Consumes: all prior commits and the GitHub Actions workflow.
- Produces: PR linked to #690 with evidence and no merge.

- [ ] **Step 1: Run non-Docker quality checks**

Run: `go build ./...`, `go vet ./...`, `go test ./internal/handlers/dnd5e/v2/encounter ./internal/orchestrators/lobby`, `go test -race ./internal/handlers/dnd5e/v2/encounter ./internal/orchestrators/lobby`, `gofmt -d $(git diff --name-only origin/main -- '*.go')`, `go mod tidy`, and `golangci-lint run ./...`.

Expected: build/vet/unit/race/fmt/tidy pass; lint has no new findings compared with `origin/main`'s known debt count.

- [ ] **Step 2: Run only targeted shared-Redis suites**

Run: `go test -race ./internal/integration -run '^(TestDungeonCryptSuite|TestEncounterV2)' -count=1`.

Expected: PASS. Do not invoke `go test ./internal/integration/...` or `go test ./...` locally.

- [ ] **Step 3: Inspect current-head CI container and suite evidence**

Push the branch, open a PR linked with `Fixes #690`, and inspect the `Test and Coverage` job logs. Confirm the current head runs all suites and starts exactly three containers, as #700's shared-fixture contract specifies.

- [ ] **Step 4: Handle automated review**

Fetch PR reviews and inline comments with `gh pr view --comments` and `gh api repos/KirkDiggler/rpg-api/pulls/$(gh pr view --json number --jq .number)/comments`. Address valid findings with focused commits and rerun affected checks. End any GitHub comment with `— asset-pipeline agent, on behalf of KirkDiggler`.

- [ ] **Step 5: Publish final evidence without merging**

Post a PR summary covering tag proof, pure projector truth table, toolkit-owned crypt lock parameters, reused unlock path, terminal characterization, RED/GREEN/mutation checks, targeted Redis results, full non-Docker checks, lint-debt parity, and CI's three-container/all-suite evidence. Do not merge the PR.
