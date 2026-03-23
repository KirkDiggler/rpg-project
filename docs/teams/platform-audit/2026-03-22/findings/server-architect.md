# Server Architect Audit — rpg-api

**Date**: 2026-03-22
**Domain**: rpg-api (game server / data orchestrator)
**Auditor**: Server Architect specialist

---

## 1. Boundary Violations

### 1.1 Action Economy: Hardcoded Ability/Action Lists (CRITICAL)
**Files:** `internal/orchestrators/encounter/orchestrator.go:4114-4217` (ActivateCombatAbility), `orchestrator.go:4219-4277` (ExecuteAction with switch/case)

The ExecuteAction method uses a switch statement on `pb.ActionId` enum values (STRIKE, OFF_HAND_STRIKE, FLURRY_STRIKE, UNARMED_STRIKE, MOVE). This couples the orchestrator to specific action types.

```go
switch input.ActionID {
case pb.ActionId_ACTION_ID_STRIKE:
    return o.executeStrike(ctx, input, encOutput.Data, actionEconomy, combat.AttackHandMain)
case pb.ActionId_ACTION_ID_OFF_HAND_STRIKE:
    return o.executeStrike(ctx, input, encOutput.Data, actionEconomy, combat.AttackHandOff)
case pb.ActionId_ACTION_ID_FLURRY_STRIKE:
    return o.executeFlurryStrike(ctx, input, encOutput.Data, actionEconomy)
}
```

### 1.2 Feature Activation: Missing Class Awareness
**File:** `internal/handlers/dnd5e/v1alpha1/character/handler.go:756` (TODO comment confirms)

No pre-filtering or class-aware validation. Clients must handle toolkit errors; can't pre-populate UI with only valid features per class.

---

## 2. State Management

### 2.1 Action Economy State Flow — GOOD
Clean separation. API correctly loads ActionEconomyState from EncounterData, passes to toolkit, persists updates back. Resets on turn start (line 1755).

### 2.2 Character Data Persistence — GOOD
Uses toolkit's `character.Data.ToJSON()` / `character.LoadFromJSON()`. Called 13 times throughout orchestrator. Canonical types preserved.

### 2.3 Monster Data Consistency — MODERATE CONCERN
Monster state in encounter.Monsters array. Linear search by ID (line 4324). If two strikes happen in parallel, monster.Data could be stale. Mitigated by turn-based sequential design.

### 2.4 Reconnection State Recovery — GOOD (with gaps)
Orchestrator methods exist for PlayerDisconnected/PlayerReconnected. Events published correctly.
**Gap:** Handler.StreamEncounterEvents (line 692) has TODO to call PlayerDisconnected on connection loss. Not wired.

---

## 3. Orchestration Layer

### 3.1 Encounter Orchestrator: God Object
**File:** `internal/orchestrators/encounter/orchestrator.go` (4,946 lines, 43 methods)

Responsibilities that should be split:
- **CombatOrchestrator:** Turn order, active actions, monster turns
- **DungeonOrchestrator:** Room generation, door traversal, exploration
- **LobbyOrchestrator:** Multiplayer state, ready/disconnect
- **CharacterActionOrchestrator:** Attack/move/feature resolution

### 3.2 State Flow Pattern — CORRECT
Handler validates -> Creates service Input -> Orchestrator loads from repos -> Loads toolkit character/monster with bus -> Calls toolkit method -> Persists output -> Returns to handler -> Handler converts to proto.

### 3.3 Event Broadcasting — MODERATE
Events published after state persisted (good). Used for monster turns, disconnection, feature activation.
**Gap:** Attack and Move return responses directly to caller, events for others. Intentional per architecture but needs documentation.

---

## 4. Handler/Converter Consistency

### 4.1 Organization — GOOD
Converters in one place per handler, isolated from business logic, convert toolkit <-> proto types.

### 4.2 TODOs Found
- Line 92: Missing CombatState in Attack response
- Line 356: Room data fetch TODO
- Line 426: Missing UpdatedCombatState
- Line 592-593: Source ref migration

### 4.3 Information Loss — NONE DETECTED
Converters preserve DamageBreakdown, AttackResult, Monster turn results.

---

## 5. Data Layer

### 5.1 Serialization — GOOD
Toolkit types -> JSON via toolkit methods -> Redis -> JSON -> Toolkit types via LoadFromJSON.

### 5.2 Data Consistency Risks

**Risk 1: CharacterHP Dual Storage**
Both EncounterData.CharacterHP map AND character.Data store HP. If one update fails, silent inconsistency.
**Recommendation:** Remove CharacterHP from EncounterData; load from character repos.

**Risk 2: interface{} RoomData**
`RoomData interface{}` throughout codebase. No compile-time type safety. Converters assume spatial.RoomData.
**Files:** repository.go:82, service.go:197, all converters.
**Recommendation:** Replace with `*spatial.RoomData`.

**Risk 3: Initiative Roll Storage**
Rolls grow as monsters die but are never pruned. Over long dungeons, stale data accumulates.

---

## 6. TurnManager Integration Gap

### Current State
- **Toolkit**: TurnManager exists with UseAbility, ExecuteAction, full turn lifecycle
- **API**: Does NOT use it. Calls char.ActivateAbility() and char.ExecuteAction() directly, manages turn order manually, owns ActionEconomyState

### Migration Path
1. Load TurnManager for each character in combat
2. Replace direct char calls with tm.UseAbility() / tm.ExecuteAction()
3. Remove ActionEconomyState from API entities
4. Remove action economy reset logic from EndTurn
**Effort:** 3-5 days. **Benefit:** Single source of truth for action economy.

---

## 7. Technical Debt (Prioritized)

### P0: CRITICAL
1. **Fix interface{} RoomData** (2-4 hours) — Type safety violation
2. **CharacterHP Consistency** (1 day) — Remove from EncounterData
3. **Action Economy Switch Statement** (2-3 days) — Route through toolkit refs

### P1: HIGH
4. **Event Publishing on Connection Loss** (1 day) — Wire gRPC stream teardown
5. **Initiative Roll Cleanup** (half day)
6. **Orchestrator Split** (3-5 days) — Combat/Dungeon/Lobby/CharacterAction

### P2: MEDIUM
7. **Class Filtering for Abilities** (1 day)
8. **Converter TODOs** (1 day)
9. **Integration Test Coverage** (2-3 days)
10. **Deterministic Test Harness** (1 day) — Fixed-seed roller

---

## 8. Fragile Code Paths

1. **OpenDoor Initiative Merge** (lines 2900-2960) — Initiative order manipulation with new monsters. Needs assertion that CurrentIndex < len(Order) after merge.
2. **Monster Turn Execution** (lines 2400-2680) — Complex loop, no transaction boundaries. Low risk due to sequential turns.
3. **Character Reconnection** (lines 3857-3947) — If character data stale, effects incorrect. Should reload from repo, not encounter snapshot.
4. **ActionEconomyState Persistence** (lines 1755, 4187, 4380) — Passed through context; hard to audit what toolkit modified.
