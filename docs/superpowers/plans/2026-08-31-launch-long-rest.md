# First-Admission Long Rest Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Persist a normal 2014 LongRest before a character's first-ever Join to a dungeon session, without exposing D&D runtime lifecycle to rpg-api.

**Architecture:** `character.Character.LongRest` remains the rule owner. `resolution.LongRest` is the data-in/data-out interaction boundary that owns the transient bus and attached sheet. Toolkit Session `Join` calls it only when the member is absent from the encounter's persisted `EverMembers`, persists the returned `character.Data`, then commits placement. `rpg-api` only removes its obsolete arcade-reset loop and pins published toolkit modules.

**Tech Stack:** Go 1.24, toolkit typed event bus, resolution surface/cast, session repository seam, testify, miniredis, gRPC integration tests.

**Spec:** `docs/superpowers/specs/2026-08-31-run-readiness-and-activation-story-design.md`

## Global Constraints

- A normal 2014 long rest occurs only before a character's first-ever Join to a session; reconnect and exit/rejoin do not rest again.
- `EverMembers` is the persisted admission record; do not add a caller-authored `rest` flag.
- `character.Character.LongRest` owns HP, death saves, character resources, hit-die arithmetic, spell slots, and RestEvent publication.
- Feature-owned resources recover through RestEvent, never feature-ref switches.
- Temporary conditions own removal; passive conditions remain; Sneak Attack and Opportunity Attack meters reset.
- Resolution owns the transient bus and runtime character. Session and rpg-api exchange data/IDs only.
- Session persists the rested `character.Data` through `CharacterRepository` before committing the joined encounter.
- `rpg-api` contains no LongRest, feature, condition, hit-die, spell-slot, or event-bus behavior.
- Retire `RestoreForLaunch`; do not replace it with another API-side helper.
- Publish inside-out with one PR/tag per toolkit module: `rulebooks/dnd5e` → `resolution` → `session` → thin rpg-api consumer.
- Runtime-carried effect cleanup on reused buses is separately tracked by rpg-toolkit#1372 and does not expand this slice.

---

### Task 1: Complete Character.LongRest on a persisted attached sheet

**PR/module:** `rpg-toolkit` — `rulebooks/dnd5e`

**Files:**
- Modify: `rulebooks/dnd5e/character/character.go`
- Modify: `rulebooks/dnd5e/character/load.go`
- Modify: `rulebooks/dnd5e/character/sheet_keeper.go`
- Modify: `rulebooks/dnd5e/character/long_rest_test.go`
- Modify: `rulebooks/dnd5e/character/attach_rollback_test.go`
- Modify: `rulebooks/dnd5e/character/load_test.go`
- Modify: `rulebooks/dnd5e/character/sheet_keeper_test.go`

**Interfaces:**
- Consumes: `character.Load`, `character.Attach`, `Character.LongRest`, `Character.ToData`.
- Produces: a strictly loaded attached character whose normal LongRest persists every implemented recovery outcome.

- [ ] **Step 1: Write the attached persisted round-trip test**

Seed a level-four Fighter with HP below 36, death saves, `SecondWindData{Uses:0, MaxUses:1}`, a 0/2 short-rest pool, 0/4 hit dice, and first-level slots with `Used:2`. Run strict `Load → Attach → LongRest → ToData` and assert independent literals:

```go
require.Equal(t, 36, got.HitPoints)
require.Equal(t, 36, got.MaxHitPoints)
require.Zero(t, got.DeathSaveState.Successes)
require.Zero(t, got.DeathSaveState.Failures)
require.Equal(t, 2, got.Resources[resources.HitDice].Current)
require.Equal(t, 2, got.Resources[shortRestPool].Current)
require.Equal(t, 0, got.SpellSlots[1].Used)
require.Equal(t, 1, secondWindUses(t, got.Features))
```

The test must use a ref-based feature lookup and real `events.NewEventBus`.

- [ ] **Step 2: Verify the real RED sequence**

```bash
cd rpg-toolkit/rulebooks/dnd5e
go test ./character -run 'TestLongRest|Test.*Activation.*Persist' -count=1
```

Expected current defects, in order as earlier ones are corrected: attached hit dice double-recover to four; persisted Second Wind remains zero because load did not attach feature lifecycle; spell slots remain used.

- [ ] **Step 3: Make recovery single-owned and attach feature lifecycle generically**

Character-owned pools remain inert during `Load/Attach`; `LongRest` and `ShortRest` recover them directly. Features that implement the existing Apply/Remove lifecycle attach generically through `BusForEffect(feature.Ref())`; no feature-ref switch is allowed. Strict failure rolls back, lenient failure drops only the failed feature, and lifecycle teardown is symmetric.

Legacy `LoadResourceData` reconstructs inert resource values and retains its signature for source compatibility; raw RestEvent does not recover those character-owned pools.

- [ ] **Step 4: Reset persisted spell slots before RestEvent**

```go
for level, slots := range c.spellSlots {
    if slots.Used == 0 {
        continue
    }
    slots.Used = 0
    c.spellSlots[level] = slots
}
```

Call `poolChanged` before publishing the owner-scoped RestEvent.

- [ ] **Step 5: Verify character behavior and lifecycle**

```bash
go test ./character -run 'TestLongRest|Test.*Activation.*Persist|TestAttach|TestPureLoad|TestSheetKeeper|TestCharacterResource' -count=1
go test ./character -count=1
golangci-lint run ./character/...
```

Mutation checks must kill reintroduced character-resource RestTopic subscription, omitted feature attachment, and omitted spell-slot reset.

- [ ] **Step 6: Commit**

```bash
git add rulebooks/dnd5e/character
git commit -m "feat: complete persisted long-rest recovery"
```

### Task 2: Make temporary conditions end themselves on long rest

**PR/module:** same `rulebooks/dnd5e` provider PR

**Files:**
- Create: `rulebooks/dnd5e/conditions/rest.go`
- Create: `rulebooks/dnd5e/conditions/rest_test.go`
- Modify: `reckless_attack.go`, `dodging.go`, `disengaging.go`, `hidden.go`, `helped.go`, `prone.go`, `unconscious.go`, `shield_spell.go`
- Verify unchanged rule: `raging.go`

**Interfaces:**
- Consumes: RestTopic and ConditionRemovedTopic.
- Produces: explicit owner-scoped opt-in removal plus self-unsubscription for every temporary condition.

- [ ] **Step 1: Add the failing real-bus table**

Use explicit canonical expected refs for Reckless Attack, Dodging, Disengaging, Hidden, Helped, Prone, Unconscious, and Shield. Every case proves wrong-owner and short-rest negative controls, one exact long-rest removal fact with reason `"long rest"`, `IsApplied()==false` afterward, and no second removal on another long rest. Keep Rage as a separate any-rest control.

- [ ] **Step 2: Verify RED**

```bash
go test ./conditions -run 'Test.*LongRest|TestRaging.*Rest' -count=1
```

- [ ] **Step 3: Add one opt-in helper**

```go
type removeCondition func(context.Context, events.EventBus) error

func subscribeRemoveOnLongRest(
    ctx context.Context,
    bus events.EventBus,
    memberID string,
    ref *core.Ref,
    remove removeCondition,
) (string, error)
```

The callback ignores other owners/non-long rests, publishes the standard removal fact first, returns immediately on publish error, and then invokes the owning condition's `Remove(ctx, bus)`. Each condition passes its own Remove method, tracks the returned subscription ID, and includes it in Apply rollback and normal removal.

- [ ] **Step 4: Verify conditions**

```bash
go test ./conditions -run 'Test.*LongRest|TestRaging.*Rest' -count=1
go test ./conditions -count=1
golangci-lint run ./conditions/...
go vet ./conditions/...
```

Mutation checks must kill omission of any helper call, owner/rest-type guards, publish-before-remove ordering, and self-removal.

- [ ] **Step 5: Commit**

```bash
git add rulebooks/dnd5e/conditions
git commit -m "feat: end temporary conditions on long rest"
```

### Task 3: Reset retained meters and prove every loader entry

**PR/module:** same `rulebooks/dnd5e` provider PR

**Files:**
- Modify: `rulebooks/dnd5e/conditions/loader.go`
- Modify: `rulebooks/dnd5e/conditions/loader_test.go`
- Modify: `rulebooks/dnd5e/conditions/sneak_attack.go`
- Modify: `rulebooks/dnd5e/conditions/sneak_attack_test.go`
- Modify: `rulebooks/dnd5e/conditions/opportunity_attack.go`
- Modify: `rulebooks/dnd5e/conditions/opportunity_attack_meter_test.go`
- Create: `rulebooks/dnd5e/conditions/long_rest_registry_test.go`
- Create: `rulebooks/dnd5e/character/long_rest_conditions_integration_test.go`

**Interfaces:**
- Produces: canonical full-ref `conditionLoaders`, an independent 22-entry expectation matrix, and persisted all-condition LongRest proof.

- [ ] **Step 1: Add meter RED tests**

Load Sneak Attack and Opportunity Attack with `used_this_turn:true`, Apply to a real bus, publish owner long rest, and assert serialized false plus one standard ConditionStateChanged fact. Wrong owner, short rest, and already-false cases remain unchanged and publish nothing.

- [ ] **Step 2: Add the independent exhaustive matrix**

Declare exactly 22 explicit fixtures/outcomes: 11 retain unchanged, 2 reset, 9 remove. Assert set equality with production loader keys, while keeping independent loader/ref-contract tests so a paired omission cannot pass.

- [ ] **Step 3: Refactor loader dispatch mechanically**

```go
type conditionLoader func(json.RawMessage) (dnd5eEvents.ConditionBehavior, error)

var conditionLoaders = map[string]conditionLoader{
    refs.Conditions.Raging().String(): loadRaging,
    // every current canonical ref, including Features.SneakAttack and Spells.Shield
}
```

Peek the complete ref and route on `peek.Ref.String()`. Preserve each current constructor/loadJSON function and wrapped error meaning. A wrong module/type with a familiar ID must not route.

- [ ] **Step 4: Subscribe both retained meters**

For matching owner long rest only, true becomes false and publishes the condition's normal state-changed fact. False remains false without dirtying the sheet. Include Apply rollback and Remove cleanup.

- [ ] **Step 5: Prove the persisted character path**

For all 22 fixtures run strict `Character.Load → Attach → LongRest → ToData`. Assert retained JSON equivalence, both seeded meters persisted false, and all temporary refs absent. The matrix must not rely on a source-text assertion or production fixture registry.

- [ ] **Step 6: Verify and commit**

```bash
go test ./conditions -count=1
go test ./character -run 'TestLongRest' -count=1
go test ./... -count=1
go vet ./...
golangci-lint run ./conditions/... ./character/...
git add rulebooks/dnd5e/conditions rulebooks/dnd5e/character/long_rest_conditions_integration_test.go
git commit -m "test: enforce long-rest behavior for every condition"
```

### Task 4: Publish the root D&D rules provider

**PR/module:** `rpg-toolkit` — `rulebooks/dnd5e`

**Files:**
- Delete: `rulebooks/dnd5e/character/arcade_recovery.go`
- Delete: `rulebooks/dnd5e/character/arcade_recovery_test.go`
- Modify: `rpg-toolkit/docs/status.md` only if current claims are invalidated

- [ ] **Step 1: Delete the parallel arcade implementation**

Existing rpg-api remains safe on its old pinned root module until Task 7. Remove `RestoreForLaunch` and its tests from the new provider release; do not preserve a compatibility wrapper or duplicate reset representation.

- [ ] **Step 2: Verify scope and known residual**

Confirm no new production code addresses runtime-carried reused-bus cleanup; that gap is rpg-toolkit#1372. Confirm the complete provider diff remains root dnd5e rules/tests only.

- [ ] **Step 3: Run provider gates**

```bash
cd rpg-toolkit/rulebooks/dnd5e
find character conditions -name '*.go' -print0 | xargs -0 gofmt -w
go test ./... -count=1
go vet ./...
golangci-lint run ./character/... ./conditions/...
cd ../../..
make lint-all
make pre-commit
```

Record established unrelated root failures verbatim; do not repair them in this PR.

- [ ] **Step 4: Independent reviews and PR**

Run internal Terra and public GLM review against the launch-rest scope. Open one PR for the root module, `Closes #1365`, wait for human merge, and record the CI-minted `rulebooks/dnd5e/v…` tag.

### Task 5: Add resolution.LongRest as the data boundary

**PR/module:** `rpg-toolkit` — `rulebooks/dnd5e/resolution`

**Files:**
- Create: `rulebooks/dnd5e/resolution/long_rest.go`
- Create: `rulebooks/dnd5e/resolution/long_rest_test.go`
- Modify: `rulebooks/dnd5e/resolution/go.mod`
- Modify: `rulebooks/dnd5e/resolution/go.sum`

**Interfaces:**
- Consumes: published root `character.Data` and `Character.LongRest` behavior.
- Produces:

```go
type LongRestInput struct {
    Character *character.Data
}

type LongRestOutput struct {
    Character *character.Data
}

func LongRest(context.Context, *LongRestInput) (*LongRestOutput, error)
```

- [ ] **Step 1: Create a new module issue/worktree from current origin/main**

One issue, branch, PR, and eventual resolution tag. Pin the exact Task 4 root tag; local overrides are allowed only during development and never committed.

- [ ] **Step 2: Write failing data-boundary tests**

Use real spent Fighter/Barbarian records. Assert nil input/data rejection, strict malformed-effect rejection, complete recovery, and output data independent from input. Add an internal `longRestOn` test over a held surface proving all subscriptions are torn down on success and every error path.

- [ ] **Step 3: Implement through resolution's one attachment door**

`LongRest` creates a new surface over `events.NewEventBus`. The unexported implementation validates one strict participant, calls `attachAll` with `DropUnreadable:false`, reads the character from the cast, invokes `Character.LongRest`, captures `ToData` before teardown, and joins operation/teardown errors using the module's existing error vocabulary. It does not call `Character.Cleanup`, expose a bus, return a runtime character, or duplicate any rest rule.

- [ ] **Step 4: Verify and publish**

```bash
cd rpg-toolkit/rulebooks/dnd5e/resolution
gofmt -w long_rest.go long_rest_test.go
go test ./... -count=1
go vet ./...
golangci-lint run ./...
```

Commit, independently review, open one resolution PR, wait for human merge, and record the CI-minted `rulebooks/dnd5e/resolution/v…` tag.

### Task 6: Rest and persist only first-ever Session Join

**PR/module:** `rpg-toolkit` — `rulebooks/dnd5e/session`

**Files:**
- Modify: `rulebooks/dnd5e/session/write.go`
- Modify: `rulebooks/dnd5e/session/write_test.go`
- Modify: `rulebooks/dnd5e/session/conditions_test.go`
- Create: `rulebooks/dnd5e/session/join_long_rest_test.go`
- Modify: `rulebooks/dnd5e/session/go.mod`
- Modify: `rulebooks/dnd5e/session/go.sum`
- Modify if invalidated: `rulebooks/dnd5e/session/README.md`, `doc.go`

**Interfaces:**
- Consumes: published `resolution.LongRest` and `CharacterRepository.SaveCharacter`.
- Produces: first-ever Join that persists a rested record and reports the write.

- [ ] **Step 1: Create the session issue/worktree and pin resolution/root tags**

Use a local override only while the provider is unpublished. The committed module graph contains exact published tags and no replace/go.work.

- [ ] **Step 2: Write failing Join persistence tests**

Seed a session with empty `EverMembers` and a spent character. Join and assert the repository contains full HP, cleared death saves, half-restored hit dice, unused spell slots, restored feature resource, retained passive conditions, and removed temporary conditions. Assert `Saved.Written` contains `character:<id>`.

Add separate tests:

- current-member duplicate Join is refused before rest/save;
- Exit followed by Join sees the member in persisted `EverMembers` and does not rest/save the deliberately re-spent record;
- a genuinely new late member rests;
- resolution rest failure or placement failure writes neither character nor encounter;
- character save failure leaves encounter unchanged and reports failure;
- character save success followed by encounter save failure reports the character as written and leaves that rested record durable.

- [ ] **Step 3: Detect first admission from persisted encounter data**

Before `encounter.Join` mutates `EverMembers`, inspect `scope.enc.ToData().EverMembers` for `in.Member`. Do not add a caller flag or second session lifecycle state.

- [ ] **Step 4: Resolve, project, place, then persist**

For a first-ever member only:

```go
rested, err := resolution.LongRest(ctx, &resolution.LongRestInput{Character: record})
```

Project `rested.Character` through the existing projection path, then perform every current placement/discovery pre-commit check. Only after those succeed, save the character with explicit report vocabulary:

```go
if err := m.characters.SaveCharacter(ctx, rested.Character); err != nil {
    report := SaveReport{Failed: []string{"character:" + in.Member}}
    return nil, &SaveError{Report: report, Err: fmt.Errorf("saving character: %w", err)}
}
scope.written = append(scope.written, "character:"+in.Member)
```

Then run the existing commit so an encounter failure includes the already-written character in SaveReport. Non-first Join uses the original record and performs no character save.

- [ ] **Step 5: Verify and publish**

```bash
cd rpg-toolkit/rulebooks/dnd5e/session
gofmt -w write.go write_test.go conditions_test.go join_long_rest_test.go
go test ./... -count=1
go vet ./...
golangci-lint run ./...
```

Commit, independently review, open one session PR, wait for human merge, and record the CI-minted `rulebooks/dnd5e/session/v…` tag.

### Task 7: Remove the API arcade reset and consume toolkit Join

**PR/repo:** `rpg-api` against `origin/dev`

**Files:**
- Modify: `go.mod`, `go.sum`
- Modify: `internal/orchestrators/lobby/start_encounter_session_stack.go`
- Modify: `internal/orchestrators/lobby/start_encounter_session_stack_test.go`
- Modify: `internal/orchestrators/lobby/abandon_encounter_test.go` only if assertions name old reset behavior
- Modify: `docs/architecture/components/lobby-service.md`, `docs/status.md`

- [ ] **Step 1: Create the API issue/worktree and add the failing acceptance**

Seed spent Fighter and Barbarian records, call the existing Lobby StartEncounter route, and assert the character repository after seating contains the complete normal-rest outcomes. The test proves the provider through the API route; it does not assert an API helper or mock LongRest call.

- [ ] **Step 2: Pin all published provider tags**

Pin the exact root, resolution, and session tags from Tasks 4–6. Run `go mod tidy`; no local replace survives.

- [ ] **Step 3: Delete API launch recovery behavior**

Remove the entire `RestoreForLaunch` load/mutate/persist block and its `tkchar` import. Keep the existing `StartSession → Join → Spawn` sequence unchanged; Join now owns first admission. Add no replacement helper, feature/condition inspection, bus, or rest flag.

- [ ] **Step 4: Verify the thin consumer**

```bash
cd rpg-api
go test ./internal/orchestrators/lobby -count=1
go test -short ./... -count=1
./scripts/verify-release-pin.sh
make pre-commit
```

The API diff must be dependency pins, deletion of obsolete behavior, direct integration evidence, and truthful docs only.

- [ ] **Step 5: Review and open the API PR**

Publish the exact-head independent review verdict. The PR targets `dev`, uses the repository's required merge style, and closes its own child issue.

### Task 8: Live first-admission acceptance

**Files:** no production files unless evidence exposes a launch-rest defect.

- [ ] **Step 1: Build exact API head in isolated lab1**

Use a dedicated identity and the local runbook. Do not restart or repoint the shared primary API.

- [ ] **Step 2: Create spent persisted state through gameplay**

Spend HP, Second Wind, Rage, hit dice/spell slots where available, and leave temporary conditions. End or abandon the run through the normal route.

- [ ] **Step 3: Start a new Reference Tomb run**

Verify the first Join persists full HP, normal half-hit-die recovery, restored feature/character resources, unused spell slots, retained passives, and removed temporary conditions before the owner status renders.

- [ ] **Step 4: Prove no repeated rest**

Reconnect without Join and verify no recovery reruns. In a controlled integration fixture, Exit/rejoin an ever-member after re-spending state and prove Join does not rest again.

- [ ] **Step 5: Record evidence**

Post exact heads/tags, toolkit and API CI, repository assertions, live transcript/screenshots, and human verdict to the provider/consumer issues and parent #341. Stop the isolated lab; leave shared primary untouched.
