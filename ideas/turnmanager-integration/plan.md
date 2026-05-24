# TurnManager Integration -- Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Remove boundary-violating switch statements from rpg-api by routing all combat actions through toolkit's Character action economy methods.

**Architecture:** The API orchestrator currently hardcodes game rules (feature action costs, side effects) that belong in the toolkit. The toolkit's Character already provides `ActivateAbility()` and `ExecuteAction()` that handle all this correctly. This plan removes the API's parallel implementation and delegates to the toolkit.

**Tech Stack:** Go, gRPC, Redis, testify, gomock

**Issue:** https://github.com/KirkDiggler/rpg-project/issues/4
**Audit:** rpg-project/docs/teams/platform-audit/2026-03-22/synthesis.md

---

## What Already Works (DO NOT TOUCH)

- `ActivateCombatAbility` (orchestrator.go:4114-4217) -- delegates to `char.ActivateAbility()`, converts results via bridge functions
- `orchestrator.go` -- all conversion helpers (`convertCharAbilitiesToEntities`, `convertCharActionsToEntities`, `protoAbilityIDToRef`, `protoActionIDToRef`)
- `loadCharacterForCombat()` -- loads character, calls `StartTurn()` if stale
- `persistCharacterData()` -- saves character data back to repo
- Toolkit `Character.ActivateAbility()` -- handles both combat abilities and features, manages action economy
- Toolkit `Character.ExecuteAction()` -- routes Strike/OffHandStrike/FlurryStrike/UnarmedStrike/Move internally
- Individual `executeStrike()`, `executeFlurryStrike()`, `executeUnarmedStrike()`, `executeMove()` -- these already use `char.ExecuteAction()` for action economy, then do combat resolution. They stay but the outer switch in `ExecuteAction()` gets simplified.

---

## Task 1: Route ActivateFeature through ActivateCombatAbility

**What:** `ActivateFeature` (orchestrator.go:2225-2391) duplicates what `ActivateCombatAbility` already does. It loads the encounter, manually checks action economy via `getFeatureActionCost()`, loads the character without combat state, calls `feature.Activate()` directly, manually consumes action economy, and persists to the encounter's `ActionEconomyState`. Meanwhile, `ActivateCombatAbility` loads the character with `loadCharacterForCombat()` (which calls `StartTurn()`), delegates to `char.ActivateAbility()` (which handles features and action economy internally), and persists via `persistCharacterData()`.

**Why:** The old path bypasses the character's action economy entirely. It uses `entities.ActionEconomyState` on the encounter instead of `character.ActionEconomyData` on the character. Features activated through the old path don't update the character's granted capacity (e.g., FlurryOfBlows granting flurry strikes). The new path handles all of this.

**Key insight:** `protoAbilityIDToRef()` already maps feature proto enums: `COMBAT_ABILITY_ID_RAGE` -> `refs.Features.Rage()`, `COMBAT_ABILITY_ID_FLURRY_OF_BLOWS` -> `refs.Features.FlurryOfBlows()`, etc. The handler already has `ActivateCombatAbility` as a separate RPC. The handler's `ActivateFeature` currently maps `FeatureId` enum -> feature ID string. We need it to instead map `FeatureId` -> `CombatAbilityId` and delegate to `ActivateCombatAbility`.

### Steps

- [ ] **1a. Add `featureIDToCombatAbilityID` mapping in handler converters and use it to route**

  **File:** `/home/kirk/personal/rpg-api/internal/handlers/dnd5e/v1alpha1/encounter/converters.go`

  Add a new function after `featureEnumToID` (line ~855):

  ```go
  // featureIDToCombatAbilityID maps a FeatureId proto enum to the equivalent CombatAbilityId.
  // Used to route legacy ActivateFeature calls through the ActivateCombatAbility path.
  func featureIDToCombatAbilityID(featureID dnd5ev1alpha1.FeatureId) (dnd5ev1alpha1.CombatAbilityId, bool) {
      switch featureID {
      case dnd5ev1alpha1.FeatureId_FEATURE_ID_RAGE:
          return dnd5ev1alpha1.CombatAbilityId_COMBAT_ABILITY_ID_RAGE, true
      case dnd5ev1alpha1.FeatureId_FEATURE_ID_SECOND_WIND:
          return dnd5ev1alpha1.CombatAbilityId_COMBAT_ABILITY_ID_SECOND_WIND, true
      case dnd5ev1alpha1.FeatureId_FEATURE_ID_FLURRY_OF_BLOWS:
          return dnd5ev1alpha1.CombatAbilityId_COMBAT_ABILITY_ID_FLURRY_OF_BLOWS, true
      case dnd5ev1alpha1.FeatureId_FEATURE_ID_PATIENT_DEFENSE:
          return dnd5ev1alpha1.CombatAbilityId_COMBAT_ABILITY_ID_DODGE, true // PatientDefense activates Dodge
      case dnd5ev1alpha1.FeatureId_FEATURE_ID_STEP_OF_THE_WIND:
          return dnd5ev1alpha1.CombatAbilityId_COMBAT_ABILITY_ID_DASH, true // StepOfTheWind activates Dash
      case dnd5ev1alpha1.FeatureId_FEATURE_ID_ACTION_SURGE:
          return dnd5ev1alpha1.CombatAbilityId_COMBAT_ABILITY_ID_ACTION_SURGE, true // Free action
      case dnd5ev1alpha1.FeatureId_FEATURE_ID_MARTIAL_ARTS_BONUS:
          return dnd5ev1alpha1.CombatAbilityId_COMBAT_ABILITY_ID_MARTIAL_ARTS_BONUS, true
      default:
          return dnd5ev1alpha1.CombatAbilityId_COMBAT_ABILITY_ID_UNSPECIFIED, false
      }
  }
  ```

  **File:** `/home/kirk/personal/rpg-api/internal/handlers/dnd5e/v1alpha1/encounter/handler.go`

  Update the `ActivateFeature` handler method to use `featureIDToCombatAbilityID` to route feature RPCs through the `ActivateCombatAbility` orchestrator path. The handler should call `featureIDToCombatAbilityID(req.FeatureId)` and, if a mapping exists, delegate to the orchestrator's `ActivateCombatAbility` method instead of `ActivateFeature`.

- [ ] **1b. Rewrite `ActivateFeature` orchestrator to delegate to `ActivateCombatAbility`**

  **NOTE:** Steps 1b and 1c must be applied together in a single commit -- they won't compile independently since 1b calls `featureIDToRef` and `featureRefToProtoAbilityID` which are defined in 1c.

  **File:** `/home/kirk/personal/rpg-api/internal/orchestrators/encounter/orchestrator.go`

  Replace lines 2225-2391 (the entire `ActivateFeature` method) with:

  ```go
  // ActivateFeature activates a combat feature (e.g., Rage) for a character.
  // Delegates to ActivateCombatAbility which handles action economy through the toolkit.
  func (o *Orchestrator) ActivateFeature(
      ctx context.Context,
      input *ActivateFeatureInput,
  ) (*ActivateFeatureOutput, error) {
      // 1. Validate input
      if input == nil {
          return nil, fmt.Errorf("input is required")
      }
      if input.EncounterID == "" {
          return nil, fmt.Errorf("encounter ID is required")
      }
      if input.CharacterID == "" {
          return nil, fmt.Errorf("character ID is required")
      }
      if input.FeatureID == "" {
          return nil, fmt.Errorf("feature ID is required")
      }

      // 2. Map feature ID to toolkit ref
      abilityRef := featureIDToRef(input.FeatureID)
      if abilityRef == nil {
          return &ActivateFeatureOutput{
              Success: false,
              Message: fmt.Sprintf("unknown feature: %s", input.FeatureID),
          }, nil
      }

      // 3. Delegate to ActivateCombatAbility
      abilityOutput, err := o.ActivateCombatAbility(ctx, &ActivateCombatAbilityInput{
          EncounterID: input.EncounterID,
          EntityID:    input.CharacterID,
          AbilityID:   featureRefToProtoAbilityID(abilityRef),
      })
      if err != nil {
          return nil, err
      }

      // 4. Load character data for the response (old path returned this)
      charOutput, err := o.charRepo.Get(ctx, &charrepo.GetInput{
          ID: input.CharacterID,
      })
      if err != nil {
          return nil, fmt.Errorf("failed to load character data after activation: %w", err)
      }

      // 5. Publish FeatureActivatedEvent for multiplayer broadcast
      if abilityOutput.Success {
          o.eventBus.Publish(ctx, &events.FeatureActivatedEvent{
              EncounterID: input.EncounterID,
              CharacterID: input.CharacterID,
              FeatureID:   input.FeatureID,
          })
      }

      // 6. Convert response to ActivateFeatureOutput format
      message := fmt.Sprintf("%s activated successfully", input.FeatureID)
      if !abilityOutput.Success {
          message = abilityOutput.Error
      }

      return &ActivateFeatureOutput{
          Success:       abilityOutput.Success,
          Message:       message,
          CharacterData: charOutput.Data,
      }, nil
  }
  ```

- [ ] **1c. Add `featureIDToRef` and `featureRefToProtoAbilityID` helpers**

  **File:** `/home/kirk/personal/rpg-api/internal/orchestrators/encounter/orchestrator.go`

  Add near `protoAbilityIDToRef` (around line 2076):

  ```go
  // featureIDToRef maps a feature ID string to a toolkit ref.
  // Used by ActivateFeature to validate the feature ID before delegating.
  func featureIDToRef(featureID string) *core.Ref {
      switch featureID {
      case refs.Features.Rage().ID:
          return refs.Features.Rage()
      case refs.Features.SecondWind().ID:
          return refs.Features.SecondWind()
      case refs.Features.FlurryOfBlows().ID:
          return refs.Features.FlurryOfBlows()
      case refs.Features.PatientDefense().ID:
          return refs.Features.PatientDefense()
      case refs.Features.StepOfTheWind().ID:
          return refs.Features.StepOfTheWind()
      case refs.Features.ActionSurge().ID:
          return refs.Features.ActionSurge()
      default:
          return nil
      }
  }

  // featureRefToProtoAbilityID maps a toolkit feature ref to a proto CombatAbilityId.
  // Reverse of protoAbilityIDToRef for feature refs.
  func featureRefToProtoAbilityID(ref *core.Ref) pb.CombatAbilityId {
      if ref == nil {
          return pb.CombatAbilityId_COMBAT_ABILITY_ID_UNSPECIFIED
      }
      switch ref.ID {
      case refs.Features.Rage().ID:
          return pb.CombatAbilityId_COMBAT_ABILITY_ID_RAGE
      case refs.Features.SecondWind().ID:
          return pb.CombatAbilityId_COMBAT_ABILITY_ID_SECOND_WIND
      case refs.Features.FlurryOfBlows().ID:
          return pb.CombatAbilityId_COMBAT_ABILITY_ID_FLURRY_OF_BLOWS
      case refs.Features.PatientDefense().ID:
          return pb.CombatAbilityId_COMBAT_ABILITY_ID_DODGE // PatientDefense activates Dodge
      case refs.Features.StepOfTheWind().ID:
          return pb.CombatAbilityId_COMBAT_ABILITY_ID_DASH // StepOfTheWind activates Dash
      case refs.Features.ActionSurge().ID:
          return pb.CombatAbilityId_COMBAT_ABILITY_ID_ACTION_SURGE // Free action, no action cost
      case refs.Features.MartialArtsBonus().ID:
          return pb.CombatAbilityId_COMBAT_ABILITY_ID_MARTIAL_ARTS_BONUS
      default:
          return pb.CombatAbilityId_COMBAT_ABILITY_ID_UNSPECIFIED
      }
  }
  ```

- [ ] **1d. Write test: Rage activation through new path**

  **File:** `/home/kirk/personal/rpg-api/internal/orchestrators/encounter/orchestrator_test.go`

  Add a test that verifies ActivateFeature delegates to ActivateCombatAbility. The test should:
  - Set up encounter with initiative data (character's turn)
  - Set up character with Rage feature, action economy initialized via `StartTurn`
  - Mock `charRepo.Get` to return barbarian character data with `ActionEconomyData` matching `computeTurnNumber`
  - Mock `charRepo.Update` to capture persisted character data
  - Call `ActivateFeature` with `FeatureID: refs.Features.Rage().ID`
  - Assert `Success: true`
  - Assert persisted character data has `BonusActionsRemaining` decremented (Rage costs bonus action)

  ```
  cd /home/kirk/personal/rpg-api && go test ./internal/orchestrators/encounter/ -run TestActivateFeatureViaAbilityPath -v
  ```

- [ ] **1e. Verify existing handler tests still pass**

  ```
  cd /home/kirk/personal/rpg-api && go test ./internal/handlers/dnd5e/v1alpha1/encounter/ -run TestActivateFeature -v
  ```

- [ ] **1f. Run pre-commit**

  ```
  cd /home/kirk/personal/rpg-api && make pre-commit
  ```

- [ ] **1g. Commit (steps 1a-1e together)**

  Steps 1b and 1c MUST be in the same commit since 1b calls functions defined in 1c.
  Commit all of 1a through 1e together.

  Message: `refactor: route ActivateFeature through ActivateCombatAbility path`

---

## Task 2: Simplify ExecuteAction switch to use protoActionIDToRef

**What:** `ExecuteAction` (orchestrator.go:4219-4277) has a switch on `pb.ActionId` that routes to separate `executeStrike()`, `executeFlurryStrike()`, etc. methods. Each of those methods already calls `char.ExecuteAction()` internally for action economy. The outer switch is redundant -- `protoActionIDToRef()` already maps every `pb.ActionId` to the correct toolkit ref.

**Why the inner methods stay:** `executeStrike()` does more than just action economy -- it loads the monster, resolves the attack via `combat.ResolveAttack()`, persists monster HP, and builds the response. We cannot replace these methods. We are only simplifying the routing.

**What changes:** Replace the manual switch with `protoActionIDToRef()` lookup, then route to the correct handler based on the ref. This is a small change that removes one layer of proto enum knowledge.

### Steps

- [ ] **2a. Refactor `ExecuteAction` to use `protoActionIDToRef`**

  **File:** `/home/kirk/personal/rpg-api/internal/orchestrators/encounter/orchestrator.go`

  Replace the switch block in `ExecuteAction` (lines 4257-4276) with:

  ```go
  // 4. Map proto action ID to toolkit ref for validation
  actionRef := protoActionIDToRef(input.ActionID)
  if actionRef == nil {
      return nil, fmt.Errorf("unsupported action ID: %s", input.ActionID.String())
  }

  // 5. Route to appropriate handler
  switch input.ActionID {
  case pb.ActionId_ACTION_ID_STRIKE:
      return o.executeStrike(ctx, input, encOutput.Data, actionEconomy, combat.AttackHandMain)
  case pb.ActionId_ACTION_ID_OFF_HAND_STRIKE:
      return o.executeStrike(ctx, input, encOutput.Data, actionEconomy, combat.AttackHandOff)
  case pb.ActionId_ACTION_ID_FLURRY_STRIKE:
      return o.executeFlurryStrike(ctx, input, encOutput.Data, actionEconomy)
  case pb.ActionId_ACTION_ID_UNARMED_STRIKE:
      return o.executeUnarmedStrike(ctx, input, encOutput.Data, actionEconomy)
  case pb.ActionId_ACTION_ID_MOVE:
      return o.executeMove(ctx, input, encOutput.Data, actionEconomy)
  default:
      return nil, fmt.Errorf("action %s not yet implemented", actionRef.ID)
  }
  ```

  **Note:** This is intentionally a small refactor. The inner methods stay because they handle combat resolution, not just action economy. The validation through `protoActionIDToRef` catches unsupported actions earlier with a better error, and positions us for future consolidation of the inner methods.

- [ ] **2b. Remove unused `actionEconomy` loading from ExecuteAction**

  The `actionEconomy` variable loaded at lines 4250-4255 is passed to each execute method but none of them use it (the parameter is named `_` in all of them). Remove the loading:

  ```go
  // Before (lines 4250-4255):
  actionEconomy := encOutput.Data.ActionEconomy
  if actionEconomy == nil {
      actionEconomy = entities.NewActionEconomyState()
  }

  // After: delete these lines entirely
  ```

  **IMPORTANT:** Before passing `nil`, verify ALL execute method signatures (`executeStrike`, `executeFlurryStrike`, `executeUnarmedStrike`, `executeMove`) actually ignore the action economy parameter (named `_`). If any of them use it, that method must be updated first.

  Update the routing calls to pass `nil` for the action economy parameter:

  ```go
  case pb.ActionId_ACTION_ID_STRIKE:
      return o.executeStrike(ctx, input, encOutput.Data, nil, combat.AttackHandMain)
  case pb.ActionId_ACTION_ID_OFF_HAND_STRIKE:
      return o.executeStrike(ctx, input, encOutput.Data, nil, combat.AttackHandOff)
  case pb.ActionId_ACTION_ID_FLURRY_STRIKE:
      return o.executeFlurryStrike(ctx, input, encOutput.Data, nil)
  case pb.ActionId_ACTION_ID_UNARMED_STRIKE:
      return o.executeUnarmedStrike(ctx, input, encOutput.Data, nil)
  case pb.ActionId_ACTION_ID_MOVE:
      return o.executeMove(ctx, input, encOutput.Data, nil)
  ```

- [ ] **2c. Verify existing ExecuteAction tests pass**

  ```
  cd /home/kirk/personal/rpg-api && go test ./internal/orchestrators/encounter/ -run TestExecuteAction -v
  ```

- [ ] **2d. Run pre-commit**

  ```
  cd /home/kirk/personal/rpg-api && make pre-commit
  ```

- [ ] **2e. Commit**

  Message: `refactor: simplify ExecuteAction routing with protoActionIDToRef validation`

---

## Task 3: Delete getFeatureActionCost and applyFeatureSideEffects

**What:** `getFeatureActionCost()` (orchestrator.go:1911-1936) and `applyFeatureSideEffects()` (orchestrator.go:1938-1953) are now dead code since `ActivateFeature` delegates to `ActivateCombatAbility` which uses the toolkit. Also delete the `actionCostType` type and constants (lines 1888-1895).

**Why:** These functions encode game rules (which features cost bonus actions, what side effects FlurryOfBlows has) in the API layer. The toolkit's `Character.activateFeature()` handles all of this through `f.ActionType()` and `f.Activate()`.

### Steps

- [ ] **3a. Verify no remaining callers**

  Search for references to these functions:

  ```
  cd /home/kirk/personal/rpg-api && grep -rn 'getFeatureActionCost\|applyFeatureSideEffects\|actionCostType\|actionCostAction\|actionCostBonusAction\|actionCostReaction\|actionCostNone' internal/
  ```

  After Task 1, the only references should be the definitions themselves. If any callers remain, they must be updated first.

- [ ] **3b. Delete the dead code**

  **File:** `/home/kirk/personal/rpg-api/internal/orchestrators/encounter/orchestrator.go`

  Delete lines 1888-1953 (the `actionCostType` type, constants, `getFeatureActionCost`, and `applyFeatureSideEffects`).

- [ ] **3c. Verify compilation and tests**

  ```
  cd /home/kirk/personal/rpg-api && go build ./... && go test ./internal/orchestrators/encounter/ -v
  ```

- [ ] **3d. Run pre-commit**

  ```
  cd /home/kirk/personal/rpg-api && make pre-commit
  ```

- [ ] **3e. Commit**

  Message: `cleanup: remove dead getFeatureActionCost and applyFeatureSideEffects`

---

## Task 4: Clean up EndTurn action economy reset

**What:** `EndTurn` (orchestrator.go:1754-1755) persists `entities.NewActionEconomyState()` to the encounter. But the character's action economy is now stored on the character data (via `ActionEconomyData`), not the encounter. The `loadCharacterForCombat()` helper detects stale turn numbers and calls `StartTurn()` automatically. So the encounter-level `ActionEconomy` field is only used for backwards compatibility.

**Why:** With the new path, the character's `StartTurn()` resets action economy at the start of each turn. The encounter's `ActionEconomyState` is a parallel struct that can drift out of sync. We should stop writing it from `EndTurn` since `loadCharacterForCombat` handles the reset.

**Caution:** The `ActionEconomy` field is still read in some response builders (e.g., `ActivateCombatAbilityOutput.ActionEconomy`). We cannot remove the field entirely yet. This task only removes the `EndTurn` reset -- the field may be nil or stale, which is acceptable since clients should use `AvailableAbilities`/`AvailableActions` instead.

### Steps

- [ ] **4a. Remove `ActionEconomy` from EndTurn persist call**

  **File:** `/home/kirk/personal/rpg-api/internal/orchestrators/encounter/orchestrator.go`

  Change line 1755 from:
  ```go
  ActionEconomy:     entities.NewActionEconomyState(), // Reset action economy for new turn
  ```
  To: delete the line entirely. The `encounterrepo.UpdateInput` should no longer set `ActionEconomy` in `EndTurn`.

- [ ] **4b. Write test: EndTurn no longer resets encounter ActionEconomy**

  **File:** `/home/kirk/personal/rpg-api/internal/orchestrators/encounter/orchestrator_test.go`

  Add a test or modify an existing `EndTurn` test to verify:
  - The `encounterrepo.UpdateInput` passed to `encRepo.Update` does NOT set `ActionEconomy`
  - Use `gomock.Do` or a custom matcher to inspect the `UpdateInput`

  ```
  cd /home/kirk/personal/rpg-api && go test ./internal/orchestrators/encounter/ -run TestEndTurn -v
  ```

- [ ] **4c. Run pre-commit**

  ```
  cd /home/kirk/personal/rpg-api && make pre-commit
  ```

- [ ] **4d. Commit**

  Message: `refactor: stop resetting encounter ActionEconomy in EndTurn, character owns action economy now`

---

## Task 5: Integration verification

**What:** Run the full test suite and verify nothing is broken across both repos. No new code, just verification.

### Steps

- [ ] **5a. Run full rpg-api test suite**

  ```
  cd /home/kirk/personal/rpg-api && go test ./... -count=1
  ```

- [ ] **5b. Run full rpg-toolkit test suite**

  ```
  cd /home/kirk/personal/rpg-toolkit && go test ./... -count=1
  ```

- [ ] **5c. Run rpg-api CI checks**

  ```
  cd /home/kirk/personal/rpg-api && make ci-check
  ```

- [ ] **5d. Verify combat flow manually by reading tests**

  Confirm test coverage exists for the full flow:
  1. `ActivateCombatAbility(ATTACK)` -> grants attacks on character
  2. `ExecuteAction(STRIKE)` -> consumes attack, resolves combat
  3. `ActivateFeature(RAGE)` -> routes through `ActivateCombatAbility`, consumes bonus action on character
  4. `EndTurn` -> character's next `StartTurn` resets action economy

  If any of these flows lack test coverage, note it but do not add tests in this task -- file a follow-up issue.

- [ ] **5e. Create PR**

  ```
  cd /home/kirk/personal/rpg-api && git push -u origin feat/turnmanager-integration && gh pr create --title "Route all combat actions through toolkit Character action economy" --body "..."
  ```

---

## Key Files Reference

| File | Repo | Role |
|------|------|------|
| `internal/orchestrators/encounter/orchestrator.go` | rpg-api | Main changes: ActivateFeature, ExecuteAction, dead code removal |
| `internal/orchestrators/encounter/service.go` | rpg-api | Input/Output types (no changes expected) |
| `internal/entities/encounter.go` | rpg-api | `ActionEconomyState` -- no changes this PR, deprecate later |
| `internal/handlers/dnd5e/v1alpha1/encounter/handler.go` | rpg-api | Handler for ActivateFeature (no orchestrator changes needed) |
| `internal/handlers/dnd5e/v1alpha1/encounter/converters.go` | rpg-api | `featureIDToCombatAbilityID` helper (used by handler to route feature RPCs) |
| `rulebooks/dnd5e/character/action_economy.go` | rpg-toolkit | Character action economy (no changes) |

## Out of Scope (follow-up issues)

1. **Remove `entities.ActionEconomyState` entirely** -- requires updating all response types and handler conversions to use character's action economy data instead. Large change, separate PR.
2. **Remove `ActivateFeature` from the Service interface** -- requires handler and proto changes. Separate PR after verifying clients use `ActivateCombatAbility`.
3. **Remove `MovementRemaining` from encounter data** -- movement is now tracked on the character's action economy. Requires updating EndTurn and response builders.
4. **Consolidate `executeStrike`/`executeFlurryStrike`/`executeUnarmedStrike`** -- these share 80% of their code. Extract common attack resolution logic. Separate refactor.
