# Condition Persistence Fix — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Fix condition round-trip serialization so event bus subscriptions survive save/load, and eliminate CharacterHP dual-storage inconsistency.

**Architecture:** Conditions already serialize/deserialize correctly via `loader.go`, and `LoadFromData` already calls `Apply(ctx, bus)` on deserialized conditions (lines 220-226 of `data.go`). The re-subscription gap described in the original audit has been fixed. What remains: (1) no round-trip test proves subscriptions survive save/load, (2) `Remove()` returns on first unsubscribe error leaking remaining subscriptions, (3) `CharacterHP` is dual-stored in both encounter state and character data causing silent desync.

**Tech Stack:** Go, Redis, testify, gomock

**Issue:** https://github.com/KirkDiggler/rpg-project/issues/5
**Audit:** rpg-project/docs/teams/platform-audit/2026-03-22/synthesis.md

---

## Task 1: Add round-trip test for condition persistence (rpg-toolkit)

**Repo:** `rpg-toolkit`
**Branch:** `fix/condition-round-trip-test`
**Files:**
- `rulebooks/dnd5e/conditions/loader_test.go` (add new test to existing `LoaderTestSuite`)

### Steps

- [ ] **1a.** Read existing loader tests to understand current coverage.
  - File: `/home/kirk/personal/rpg-toolkit/rulebooks/dnd5e/conditions/loader_test.go`
  - Current tests verify field values survive serialization but do NOT verify `IsApplied()` or event bus subscriptions after load.

- [ ] **1b.** Add `TestRagingConditionRoundTripWithSubscriptions` to `LoaderTestSuite`.
  - Create a `RagingCondition` with known state (CharacterID, DamageBonus=2, TurnsActive=3).
  - Call `Apply(ctx, bus)` on the original — verify `IsApplied()` is true.
  - Call `ToJSON()` to serialize.
  - Call `LoadJSON()` to deserialize — this gives a new condition instance.
  - Verify the deserialized condition has `IsApplied() == false` (loader does NOT call Apply).
  - Call `Apply(ctx, bus)` on the deserialized condition.
  - Verify `IsApplied() == true`.
  - Publish a `DamageChainEvent` with `AttackerID` matching the condition's `CharacterID`.
  - Verify the damage chain includes the rage damage bonus component — proving the subscription is live.

- [ ] **1c.** Add `TestRagingConditionRoundTripCleanup` to verify `Remove()` works on a re-applied condition.
  - Create condition, Apply, ToJSON, LoadJSON, Apply the loaded copy.
  - Call `Remove(ctx, bus)` on the loaded copy.
  - Verify `IsApplied() == false`.
  - Remove the original too (it is on the same bus).

- [ ] **1d.** Run tests to verify they pass.
  ```bash
  cd /home/kirk/personal/rpg-toolkit && go test ./rulebooks/dnd5e/conditions/ -run TestLoaderTestSuite -v
  ```

- [ ] **1e.** Run pre-commit checks.
  ```bash
  cd /home/kirk/personal/rpg-toolkit && make pre-commit
  ```

---

## Task 2: Add round-trip test at the Character level (rpg-toolkit)

**Repo:** `rpg-toolkit`
**Branch:** same as Task 1 (`fix/condition-round-trip-test`)
**Files:**
- `rulebooks/dnd5e/character/data_test.go` (new or existing test file)

### Steps

- [ ] **2a.** Check what test files exist in the character package.
  ```bash
  ls /home/kirk/personal/rpg-toolkit/rulebooks/dnd5e/character/*test*
  ```

- [ ] **2b.** Add `TestCharacterConditionRoundTrip` to verify the full `Data -> LoadFromData -> conditions applied` path.
  - Build a `character.Data` struct with a serialized Raging condition in `Data.Conditions`.
  - Call `LoadFromData(ctx, data, bus)`.
  - Verify the returned character has one condition and it is applied (`IsApplied() == true`).
  - This test proves the existing fix at line 220 works end-to-end.

- [ ] **2c.** Run tests.
  ```bash
  cd /home/kirk/personal/rpg-toolkit && go test ./rulebooks/dnd5e/character/ -run TestCharacterConditionRoundTrip -v
  ```

- [ ] **2d.** Run pre-commit checks.
  ```bash
  cd /home/kirk/personal/rpg-toolkit && make pre-commit
  ```

---

## Task 3: Fix Remove() to continue on error (rpg-toolkit)

**Repo:** `rpg-toolkit`
**Branch:** `fix/condition-remove-cleanup`
**Files to change (same pattern in all):**
- `rulebooks/dnd5e/conditions/raging.go`
- `rulebooks/dnd5e/conditions/brutal_critical.go`
- `rulebooks/dnd5e/conditions/reckless_attack.go`
- `rulebooks/dnd5e/conditions/unarmored_defense.go`
- `rulebooks/dnd5e/conditions/dodging.go`
- `rulebooks/dnd5e/conditions/disengaging.go`
- `rulebooks/dnd5e/conditions/sneak_attack.go`
- `rulebooks/dnd5e/conditions/martial_arts.go`
- `rulebooks/dnd5e/conditions/improved_critical.go`
- `rulebooks/dnd5e/conditions/fighting_style_*.go` (6 files)

### The Bug

Every condition's `Remove()` follows this pattern:
```go
for _, subID := range r.subscriptionIDs {
    err := bus.Unsubscribe(ctx, subID)
    if err != nil {
        return err  // BUG: remaining subscriptions leak
    }
}
```

If the first unsubscribe fails (stale ID), the rest never get cleaned up.

### Steps

- [ ] **3a.** Add a failing test first: `TestRemoveContinuesOnStaleSubscription` in `raging_test.go`.
  - Apply a RagingCondition (creates 5 subscriptions).
  - Manually corrupt one subscription ID (e.g., replace first ID with `"stale-id"`).
  - Call `Remove(ctx, bus)`.
  - Current behavior: returns error, remaining subscriptions leak.
  - Expected behavior: returns error but still cleans up all valid subscriptions, sets `bus = nil`.

- [ ] **3b.** Fix `Remove()` in `raging.go` to collect errors instead of returning early.
  ```go
  func (r *RagingCondition) Remove(ctx context.Context, bus events.EventBus) error {
      if r.bus == nil {
          return nil
      }

      total := len(r.subscriptionIDs)
      var errs []error
      for _, subID := range r.subscriptionIDs {
          if err := bus.Unsubscribe(ctx, subID); err != nil {
              errs = append(errs, err)
          }
      }

      r.subscriptionIDs = nil
      r.bus = nil

      if len(errs) > 0 {
          return fmt.Errorf("failed to unsubscribe %d/%d subscriptions: %w", len(errs), total, errors.Join(errs...))
      }
      return nil
  }
  ```

- [ ] **3c.** Verify the test passes.
  ```bash
  cd /home/kirk/personal/rpg-toolkit && go test ./rulebooks/dnd5e/conditions/ -run TestRagingConditionTestSuite/TestRemoveContinuesOnStaleSubscription -v
  ```

- [ ] **3d.** Apply the same fix to all other condition files listed above.
  - Each file has the identical `Remove()` pattern — same early-return bug.
  - Use the same error-collection approach.
  - **Note:** Grep for the actual `return err` pattern inside unsubscribe loops rather than blindly applying to every listed file. Some conditions (e.g., `unarmored_movement.go`) have a `Remove()` that simply sets `bus = nil` with no subscription loop — those don't need this fix.

- [ ] **3e.** Run full condition test suite.
  ```bash
  cd /home/kirk/personal/rpg-toolkit && go test ./rulebooks/dnd5e/conditions/ -v
  ```

- [ ] **3f.** Run pre-commit checks.
  ```bash
  cd /home/kirk/personal/rpg-toolkit && make pre-commit
  ```

---

## Task 4: Remove CharacterHP from EncounterData (rpg-api)

**Repo:** `rpg-api`
**Branch:** `fix/remove-character-hp-dual-storage`
**Files:**
- `internal/repositories/encounters/repository.go` — remove `CharacterHP` from `EncounterData`, `CreateEncounterInput`, `UpdateEncounterInput`
- `internal/repositories/encounters/inmemory.go` — remove `CharacterHP` handling
- `internal/orchestrators/encounter/orchestrator.go` — replace `CharacterHP` reads with character repo lookups
- `internal/orchestrators/encounter/monster_turns.go` — replace `CharacterHP` mutation with character data updates
- `internal/orchestrators/encounter/orchestrator_test.go` — update tests
- `internal/orchestrators/encounter/event_publishing_test.go` — update tests

### The Bug

HP is stored in two places:
1. `EncounterData.CharacterHP` map (encounter repo) — used for TPK detection
2. `character.Data.HitPoints` (character repo) — the canonical source

When monster attacks damage a character, `monster_turns.go` updates `CharacterHP` in the encounter AND writes `charData.HitPoints`. If one write fails, they desync silently.

### Steps

- [ ] **4a.** Audit all CharacterHP usage. Grep results show these locations:
  - `orchestrator.go`: Lines 744-745 (init), 849/865 (save), 1758 (save after turn), 2647-2653 (TPK check), 3534/3562/3578 (multiplayer save)
  - `monster_turns.go`: Lines 226-232 (update on hit), 271-273 (sync to char data)
  - `repository.go`: Lines 89, 112, 125, 155, 180-181 (field definition and CRUD)
  - `inmemory.go`: Lines 46, 87, 125 (storage plumbing)

- [ ] **4b.** Refactor `checkAllCharactersDead()` to accept character data instead of `CharacterHP` map.
  - Current: reads `enc.CharacterHP` map.
  - New: load characters from the character repo by encounter's character IDs. The orchestrator already has a character repo reference — use it.

- [ ] **4c.** Refactor `monster_turns.go` to update only character data, not `CharacterHP`.
  - Lines 226-232: instead of `enc.CharacterHP[action.TargetID] = newHP`, update the character's `Data.HitPoints` directly (which already happens at lines 268-273).
  - Remove the `CharacterHP` update — it is redundant.

- [ ] **4d.** Remove `CharacterHP` from all encounter save calls in `orchestrator.go`.
  - Remove from `CreateEncounterInput` and `UpdateEncounterInput` construction.
  - Remove initialization at line 744-745.

- [ ] **4e.** Remove `CharacterHP` field from `EncounterData`, `CreateEncounterInput`, `UpdateEncounterInput` in `repository.go`.

- [ ] **4f.** Remove `CharacterHP` handling from `inmemory.go`.

- [ ] **4g.** Update tests.
  - Remove all `CharacterHP` assertions and setup from `orchestrator_test.go` and `event_publishing_test.go`.
  - Add test: verify character HP is correct after monster damage using character repo lookup.

- [ ] **4h.** Run tests.
  ```bash
  cd /home/kirk/personal/rpg-api && go test ./internal/... -v
  ```

- [ ] **4i.** Run pre-commit checks.
  ```bash
  cd /home/kirk/personal/rpg-api && make pre-commit
  ```

---

## Task 5: Integration test — save/load with active conditions (rpg-toolkit)

**Repo:** `rpg-toolkit`
**Branch:** same as Task 1 (`fix/condition-round-trip-test`)
**Files:**
- `rulebooks/dnd5e/conditions/raging_test.go` (add integration test)

### Steps

- [ ] **5a.** Add `TestRagingConditionSaveLoadDamageBonus` to `RagingConditionTestSuite`.
  - Create character-like setup: event bus, RagingCondition with DamageBonus=2.
  - Apply condition to bus.
  - Fire a `DamageChainEvent` with `AttackerID` matching — verify rage bonus is added.
  - Serialize condition via `ToJSON()`.
  - Remove the original condition (`Remove(ctx, bus)`).
  - Deserialize via `LoadJSON()`.
  - Re-apply the deserialized condition (`Apply(ctx, bus)`).
  - Fire another `DamageChainEvent` — verify rage bonus is STILL added, proving subscriptions survived the round-trip.

- [ ] **5b.** Run the integration test.
  ```bash
  cd /home/kirk/personal/rpg-toolkit && go test ./rulebooks/dnd5e/conditions/ -run TestRagingConditionTestSuite/TestRagingConditionSaveLoadDamageBonus -v
  ```

- [ ] **5c.** Run pre-commit checks.
  ```bash
  cd /home/kirk/personal/rpg-toolkit && make pre-commit
  ```

---

## PR Strategy

| Task | Repo | Branch | PR |
|------|------|--------|----|
| Tasks 1, 2, 5 | rpg-toolkit | `fix/condition-round-trip-test` | Single PR — adds test coverage for existing fix |
| Task 3 | rpg-toolkit | `fix/condition-remove-cleanup` | Separate PR — behavioral change to Remove() |
| Task 4 | rpg-api | `fix/remove-character-hp-dual-storage` | Separate PR — API repo, independent change |

**Order:** Tasks 1/2/5 can merge independently of Task 3 or 4. Task 3 and 4 have no dependency on each other.

---

## Key Files Reference

| File | Repo | Role |
|------|------|------|
| `rulebooks/dnd5e/conditions/loader.go` | rpg-toolkit | Routes JSON to correct condition type by ref |
| `rulebooks/dnd5e/conditions/raging.go` | rpg-toolkit | Model condition: Apply/Remove/ToJSON/loadJSON |
| `rulebooks/dnd5e/character/data.go` | rpg-toolkit | `LoadFromData` — already calls Apply on conditions (line 220) |
| `rulebooks/dnd5e/events/events.go` | rpg-toolkit | `ConditionBehavior` interface definition |
| `internal/repositories/encounters/repository.go` | rpg-api | `EncounterData` with `CharacterHP` field |
| `internal/orchestrators/encounter/orchestrator.go` | rpg-api | Uses `CharacterHP` for TPK detection |
| `internal/orchestrators/encounter/monster_turns.go` | rpg-api | Dual-writes HP to both CharacterHP and character data |
