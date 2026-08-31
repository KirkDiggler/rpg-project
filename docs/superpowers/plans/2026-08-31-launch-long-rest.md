# Launch Long Rest Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Start every new dungeon run from the persisted result of the character's real 2014 LongRest path, including all resources and explicit condition lifetimes.

**Architecture:** rpg-toolkit's attached Character.LongRest remains the sole rules mechanism. Every condition opts into removal, retained-state reset, or no change through its own subscriptions; rpg-api only loads, attaches, invokes, serializes, persists, and then seats characters.

**Tech Stack:** Go 1.24, toolkit typed event bus, testify, miniredis, gRPC integration tests.

**Spec:** `docs/superpowers/specs/2026-08-31-run-readiness-and-activation-story-design.md`

## Global Constraints

- Launch invokes a normal 2014 long rest and tops off HP; reconnect never rests.
- Feature-owned resources must recover through RestEvent, not feature-ref switches.
- Temporary conditions own their removal; Character.LongRest must not switch over condition refs.
- Passive conditions remain; Sneak Attack and Opportunity Attack meters reset.
- Hit dice recover half maximum (minimum one), not the old arcade full refill.
- Persisted spell-slot used counts reset to zero.
- Retire the parallel RestoreForLaunch behavior; no API-side game rules.

---

### Task 1: Complete Character.LongRest on a persisted attached sheet

**Files:**
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/character.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/long_rest_test.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/activation_persists_test.go`
- Delete after consumers migrate: `rpg-toolkit/rulebooks/dnd5e/character/arcade_recovery.go`
- Delete after consumers migrate: `rpg-toolkit/rulebooks/dnd5e/character/arcade_recovery_test.go`

**Interfaces:**
- Consumes: `Load(ctx, *Data)`, `Attach(ctx, *Character, events.EventBus)`, `Character.LongRest(ctx)`.
- Produces: `ToData()` containing full HP, cleared death saves, normal resource recovery, feature-owned recovery, and unused spell slots.

- [ ] **Step 1: Add a failing attached round-trip test**

Build a valid persisted Fighter with Second Wind at `uses:0,max_uses:1`, HP below max, death-save state, a spent short-rest resource, spent hit dice, and spell slots with `Used > 0`. Load and attach it, call LongRest, and serialize:

```go
char, err := Load(ctx, data)
require.NoError(t, err)
bus := events.NewEventBus()
require.NoError(t, Attach(ctx, char, bus))
t.Cleanup(func() { require.NoError(t, char.Cleanup(ctx)) })

require.NoError(t, char.LongRest(ctx))
got := char.ToData()
require.Equal(t, got.MaxHitPoints, got.HitPoints)
if got.DeathSaveState != nil {
    require.Zero(t, got.DeathSaveState.Successes)
    require.Zero(t, got.DeathSaveState.Failures)
}
require.Equal(t, 0, got.SpellSlots[1].Used)

var secondWind features.SecondWindData
require.NoError(t, json.Unmarshal(featureByRef(t, got.Features, refs.Features.SecondWind()), &secondWind))
require.Equal(t, secondWind.MaxUses, secondWind.Uses)
```

Add this test helper in the same file so the lookup is ref-based rather than positional:

```go
func featureByRef(t *testing.T, blobs []json.RawMessage, want *core.Ref) json.RawMessage {
    t.Helper()
    for _, raw := range blobs {
        var envelope struct { Ref core.Ref `json:"ref"` }
        require.NoError(t, json.Unmarshal(raw, &envelope))
        if envelope.Ref.Equals(want) { return raw }
    }
    t.Fatalf("feature %s not found", want.String())
    return nil
}
```

Assert a level-four character with zero hit dice recovers exactly two, not four.

- [ ] **Step 2: Run the test and verify RED**

```bash
cd rpg-toolkit/rulebooks/dnd5e
go test ./character -run 'TestLongRest|Test.*Activation.*Persist' -count=1
```

Expected: spell-slot assertion fails; Second Wind passes only when the feature was attached correctly.

- [ ] **Step 3: Reset spell slots in Character.LongRest**

After normal resource recovery and before publishing RestEvent:

```go
for level, slots := range c.spellSlots {
    if slots.Used == 0 {
        continue
    }
    slots.Used = 0
    c.spellSlots[level] = slots
}
```

Keep HP maximum, death-save clearing, short/long resource refill, and half-hit-die recovery in Character.LongRest. Do not add feature-specific logic.

- [ ] **Step 4: Verify the focused character suite**

```bash
go test ./character -run 'TestLongRest|Test.*Activation.*Persist' -count=1
```

Expected: PASS.

- [ ] **Step 5: Commit the complete runtime rest**

```bash
git add rulebooks/dnd5e/character/character.go \
  rulebooks/dnd5e/character/long_rest_test.go \
  rulebooks/dnd5e/character/activation_persists_test.go
git commit -m "feat: complete persisted long-rest recovery"
```

Do not delete RestoreForLaunch until the API consumer no longer calls it; delete it in the final toolkit cleanup commit after Task 5's branch proves migration.

### Task 2: Make every temporary condition end itself on long rest

**Files:**
- Create: `rpg-toolkit/rulebooks/dnd5e/conditions/rest.go`
- Create: `rpg-toolkit/rulebooks/dnd5e/conditions/rest_test.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/reckless_attack.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/dodging.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/disengaging.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/hidden.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/helped.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/prone.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/unconscious.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/shield_spell.go`
- Verify existing: `rpg-toolkit/rulebooks/dnd5e/conditions/raging.go`

**Interfaces:**
- Consumes: `dnd5eEvents.RestTopic`, `ConditionRemovedTopic`.
- Produces: a reusable opt-in subscription helper; no central condition-ref removal switch.

- [ ] **Step 1: Write failing removal tests**

For each temporary condition, attach it to a bus, subscribe to ConditionRemovedTopic, publish a short rest as a negative control and a long rest as the trigger:

```go
require.NoError(t, condition.Apply(ctx, bus))
require.NoError(t, dnd5eEvents.RestTopic.On(bus).Publish(ctx, dnd5eEvents.RestEvent{
    RestType: coreResources.ResetShortRest, CharacterID: memberID,
}))
require.Empty(t, removed, "short-rest negative control")
require.NoError(t, dnd5eEvents.RestTopic.On(bus).Publish(ctx, dnd5eEvents.RestEvent{
    RestType: coreResources.ResetLongRest, CharacterID: memberID,
}))
require.Equal(t, condition.Ref().String(), removed.ConditionRef)
require.Equal(t, "long rest", removed.Reason)
```

Use table subtests for Reckless Attack, Dodging, Disengaging, Hidden, Helped, Prone, Unconscious, and Shield. Keep Rage's existing any-rest behavior as its own rule and positive control.

- [ ] **Step 2: Run tests and verify RED**

```bash
cd rpg-toolkit/rulebooks/dnd5e
go test ./conditions -run 'Test.*LongRest|TestRaging.*Rest' -count=1
```

Expected: every new temporary-condition case except Rage fails to publish removal.

- [ ] **Step 3: Add the opt-in removal helper**

Implement in `rest.go`:

```go
func subscribeRemoveOnLongRest(
    ctx context.Context,
    bus events.EventBus,
    memberID string,
    ref *core.Ref,
) (string, error) {
    return dnd5eEvents.RestTopic.On(bus).Subscribe(ctx,
        func(ctx context.Context, event dnd5eEvents.RestEvent) error {
            if event.CharacterID != memberID || event.RestType != coreResources.ResetLongRest {
                return nil
            }
            return dnd5eEvents.ConditionRemovedTopic.On(bus).Publish(ctx,
                dnd5eEvents.ConditionRemovedEvent{
                    MemberID: memberID, ConditionRef: ref.String(), Reason: "long rest",
                })
        })
}
```

Each listed condition explicitly calls this helper from Apply, appends the returned ID to its own subscription list, and includes it in existing rollback/unsubscribe behavior.

- [ ] **Step 4: Run focused and full condition tests**

```bash
go test ./conditions -run 'Test.*LongRest|TestRaging.*Rest' -count=1
go test ./conditions -count=1
```

Expected: PASS with no leaked or duplicate subscriptions.

- [ ] **Step 5: Commit temporary-condition lifetimes**

```bash
git add rulebooks/dnd5e/conditions
git commit -m "feat: end temporary conditions on long rest"
```

### Task 3: Reset retained meters and enforce registry completeness

**Files:**
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/loader.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/loader_test.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/sneak_attack.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/sneak_attack_test.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/opportunity_attack.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/opportunity_attack_meter_test.go`
- Create: `rpg-toolkit/rulebooks/dnd5e/conditions/long_rest_registry_test.go`

**Interfaces:**
- Produces: unexported `conditionLoaders` registry keyed by canonical ref string and a complete long-rest expectation table.

- [ ] **Step 1: Add failing retained-meter tests**

Load Sneak Attack and Opportunity Attack with `used_this_turn:true`, attach, publish a long rest for the owner, and assert serialized state is false. Also assert passive conditions such as Unarmored Defense and Two-Weapon Fighting remain present.

- [ ] **Step 2: Add a registry completeness test**

Define `longRestCases` with exactly one entry for every loader key and assert set equality:

```go
require.ElementsMatch(t,
    slices.Collect(maps.Keys(conditionLoaders)),
    slices.Collect(maps.Keys(longRestCases)),
)
```

Each case declares `retain`, `reset`, or `remove`; the behavioral subtest runs `Load -> Apply -> RestEvent -> ToJSON` and asserts the declared post-state. Include Shield and the feature-ref-shaped Sneak Attack key.

- [ ] **Step 3: Run tests and verify RED**

```bash
cd rpg-toolkit/rulebooks/dnd5e
go test ./conditions -run 'TestLongRestRegistry|Test.*Meter.*LongRest' -count=1
```

Expected: meters remain used and `conditionLoaders` does not yet exist.

- [ ] **Step 4: Refactor LoadJSON dispatch into one registry**

Mechanically move each existing switch arm into:

```go
type conditionLoader func(json.RawMessage) (dnd5eEvents.ConditionBehavior, error)
var conditionLoaders = map[string]conditionLoader{
    refs.Conditions.Raging().String(): loadRaging,
    // every current loader arm, including refs.Features.SneakAttack and refs.Spells.Shield
}
```

Peek the complete `core.Ref`, look up by `peek.Ref.String()`, and preserve existing wrapped errors. This is dispatch consolidation only; no loading behavior changes.

- [ ] **Step 5: Subscribe retained meters to long rest**

Sneak Attack and Opportunity Attack subscribe to RestTopic. For matching long rests, set `UsedThisTurn=false` only when true and publish ConditionStateChangedTopic with their owner/ref so the keeper marks the sheet dirty. Include subscription rollback and removal.

- [ ] **Step 6: Verify all conditions**

```bash
go test ./conditions -count=1
go test ./character -run TestLongRest -count=1
```

Expected: PASS; every registered condition has an explicit rule.

- [ ] **Step 7: Commit registry and meter behavior**

```bash
git add rulebooks/dnd5e/conditions
git commit -m "test: enforce long-rest behavior for every condition"
```

### Task 4: Verify and publish the toolkit root provider

**Files:**
- Modify if invalidated: `rpg-toolkit/docs/status.md`
- Delete: `rpg-toolkit/rulebooks/dnd5e/character/arcade_recovery.go`
- Delete: `rpg-toolkit/rulebooks/dnd5e/character/arcade_recovery_test.go`

**Interfaces:**
- Produces: published `rulebooks/dnd5e` tag with complete LongRest and no parallel arcade reset.

- [ ] **Step 1: Confirm no non-test consumer needs RestoreForLaunch after the coordinated API branch is ready**

```bash
rg -n 'RestoreForLaunch' /home/kirk/game-dev --glob '*.go'
```

Expected before API migration: only the known API launch call plus toolkit tests/docs. Coordinate deletion and API pin so no merged consumer is broken.

- [ ] **Step 2: Delete the parallel helper and update stale docs**

Remove RestoreForLaunch and its tests. Update toolkit status/ADR references to state launch hosts invoke attached Character.LongRest.

- [ ] **Step 3: Run full toolkit verification**

```bash
cd rpg-toolkit/rulebooks/dnd5e
find character conditions -name '*.go' -print0 | xargs -0 gofmt -w
go test ./... -count=1
go vet ./...
cd ../../..
make lint-all
make pre-commit
```

Expected: all scoped module checks pass. Record any established unrelated root coverage-parser failure verbatim.

- [ ] **Step 4: Commit, open PR, and wait for human merge/tag**

```bash
git add rulebooks/dnd5e docs/status.md
git commit -m "refactor: make long rest the only launch recovery path"
```

After merge, record the exact root module tag.

### Task 5: Invoke LongRest from rpg-api launch orchestration

**Files:**
- Modify: `rpg-api/go.mod`
- Modify: `rpg-api/go.sum`
- Modify: `rpg-api/internal/orchestrators/lobby/start_encounter_session_stack.go`
- Modify: `rpg-api/internal/orchestrators/lobby/start_encounter_session_stack_test.go`
- Modify: `rpg-api/internal/orchestrators/lobby/abandon_encounter_test.go`
- Modify if invalidated: `rpg-api/docs/status.md`

**Interfaces:**
- Consumes: published `character.Load`, `Attach`, `LongRest`, `ToData` behavior.
- Produces: rested persisted records before SessionManager.StartSession.

- [ ] **Step 1: Add failing real-launch acceptance**

Seed valid Fighter and Barbarian records. Fighter has spent Second Wind, HP, hit dice, spell slots, and a temporary condition; Barbarian has spent Rage Charges and Raging. Start the lobby encounter and assert the repository post-state:

```go
s.Equal(fighter.MaxHitPoints, gotFighter.Data.HitPoints)
s.Equal(1, secondWindUses(t, gotFighter.Data.Features))
s.Equal(0, gotFighter.Data.SpellSlots[1].Used)
s.Equal(expectedHalfRecovered, gotFighter.Data.Resources[resources.HitDice].Current)
s.NotContains(conditionRefs(gotFighter.Data), refs.Conditions.Dodging().String())
s.Equal(2, gotBarbarian.Data.Resources[resources.RageCharges].Current)
s.NotContains(conditionRefs(gotBarbarian.Data), refs.Conditions.Raging().String())
s.Contains(conditionRefs(gotBarbarian.Data), refs.Conditions.UnarmoredDefense().String())
```

Also retain the “failure before session creation” test with one malformed party member.

- [ ] **Step 2: Run and verify RED on the current API pin**

```bash
cd rpg-api
go test ./internal/orchestrators/lobby -run 'TestSessionStackSuite/TestStartEncounter_.*Rest' -count=1
```

Expected: Second Wind/condition/half-hit-die assertions fail under RestoreForLaunch.

- [ ] **Step 3: Pin the published toolkit root tag**

Use the exact Task 4 tag and run `go mod tidy`; no replace remains.

- [ ] **Step 4: Replace persisted-field reset with attached LongRest**

Add a private orchestration helper:

```go
func restForLaunch(ctx context.Context, entity *entities.Character) (*entities.Character, error) {
    live, err := tkchar.Load(ctx, entity.Data)
    if err != nil { return nil, fmt.Errorf("load for launch rest: %w", err) }
    bus := events.NewEventBus()
    if err := tkchar.Attach(ctx, live, bus); err != nil {
        return nil, fmt.Errorf("attach for launch rest: %w", err)
    }
    if err := live.LongRest(ctx); err != nil {
        _ = live.Cleanup(ctx)
        return nil, fmt.Errorf("long rest for launch: %w", err)
    }
    rested := live.ToData()
    if err := live.Cleanup(ctx); err != nil {
        return nil, fmt.Errorf("clean up launch rest: %w", err)
    }
    return &entities.Character{Data: rested, Appearance: entity.Appearance}, nil
}
```

Preflight every party member through this helper into memory, then persist all rested entities, then start/session-seat exactly as today. API code never reads feature or condition refs.

- [ ] **Step 5: Verify launch and full API**

```bash
go test ./internal/orchestrators/lobby -count=1
go test -short ./... -count=1
./scripts/verify-release-pin.sh
make pre-commit
```

Expected: PASS.

- [ ] **Step 6: Commit and open the API PR**

```bash
git add go.mod go.sum internal/orchestrators/lobby docs/status.md
git commit -m "feat: long rest characters before dungeon launch"
```

### Task 6: Live run-start acceptance

**Files:**
- No production files unless evidence exposes a defect.

- [ ] **Step 1: Build exact API head in isolated lab1**

Use the local runbook and a dedicated identity. Do not restart or repoint the shared primary API.

- [ ] **Step 2: Create spent persisted state through real gameplay**

Use Second Wind and Rage, take damage, and leave temporary statuses. End/abandon the run through the normal route.

- [ ] **Step 3: Start a new Reference Tomb run**

Verify owner status shows full HP and restored feature charges; permanent class/fighting conditions remain and temporary conditions are absent.

- [ ] **Step 4: Record evidence**

Post exact heads, toolkit tag, API CI, repository assertions, live screenshots/transcript, and human verdict. The isolated lab is stopped after verification; shared primary remains untouched.
