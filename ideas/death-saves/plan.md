# Death Saves & Rest System — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Add proto contracts, toolkit conditions, API orchestration, and web UI for death saves and rest system — enabling multi-room dungeons where characters can die and heal.

**Architecture:** Toolkit already has death save mechanics and rest methods. This plan adds the missing UnconsciousCondition, proto contracts (enums, messages, RPCs, events), API orchestration layer, and web components. Proto changes land first, then each downstream repo consumes them.

**Tech Stack:** Go, protobuf/buf, gRPC, Redis, React/TypeScript, testify, gomock

**Issue:** https://github.com/KirkDiggler/rpg-project/issues/6
**Audit:** rpg-project/docs/teams/platform-audit/2026-03-22/synthesis.md
**Design Docs:** rpg-project/ideas/death-saves/design.md, rpg-project/ideas/rest-system/design.md

---

## Task 1: Add death save and rest proto contracts (rpg-api-protos)

**Repo:** `/home/kirk/personal/rpg-api-protos`
**Branch:** `feat/death-saves-rest-protos`

### 1.1 Setup

- [ ] `cd /home/kirk/personal/rpg-api-protos && git checkout main && git pull origin main`
- [ ] `git checkout -b feat/death-saves-rest-protos`

### 1.2 Add CONDITION_ID_UNCONSCIOUS to ConditionId enum

**File:** `dnd5e/api/v1alpha1/enums.proto`

- [ ] Add `CONDITION_ID_UNCONSCIOUS = 15;` after `CONDITION_ID_UNARMORED_MOVEMENT = 14;`
- [ ] Add `CONDITION_ID_RECKLESS_ATTACK = 16;` and `CONDITION_ID_DODGING = 17;` and `CONDITION_ID_DISENGAGING = 18;` — these exist in toolkit but are missing from proto (needed for completeness)

### 1.3 Add RestType enum

**File:** `dnd5e/api/v1alpha1/enums.proto`

- [ ] Add new enum after `ActionId` (the last enum in enums.proto). Note: `EncounterEndReason` is in encounter.proto, not enums.proto:
  ```protobuf
  // RestType identifies the type of rest taken between encounters
  enum RestType {
    REST_TYPE_UNSPECIFIED = 0;
    REST_TYPE_SHORT = 1;
    REST_TYPE_LONG = 2;
  }
  ```

### 1.4 Add DeathSaveProgress message

**File:** `dnd5e/api/v1alpha1/encounter.proto`

- [ ] Add message in the combat state section (after `CombatState`):
  ```protobuf
  // DeathSaveProgress tracks death save successes and failures for a character at 0 HP
  message DeathSaveProgress {
    int32 success_count = 1;
    int32 failure_count = 2;
    bool stabilized = 3;
    bool dead = 4;
  }
  ```

### 1.5 Add CharacterCombatState message

**File:** `dnd5e/api/v1alpha1/encounter.proto`

- [ ] Add message to track per-character combat state (death saves, conditions):
  ```protobuf
  // CharacterCombatState tracks combat-specific state for a player character
  // Entity positions are in Room.entities, this provides combat stats
  message CharacterCombatState {
    string character_id = 1;
    int32 current_hit_points = 2;
    int32 max_hit_points = 3;
    DeathSaveProgress death_saves = 4;
    bool is_unconscious = 5;
    bool is_dead = 6;
  }
  ```
- [ ] Add `repeated CharacterCombatState characters = 12;` to `GetEncounterStateResponse` (after `dungeon_id`). Note: fields 1-11 are already taken (`last_event_id = 9`, `doors = 10`, `dungeon_id = 11`), so use field 12.

### 1.6 Add death save and rest event messages

**File:** `dnd5e/api/v1alpha1/encounter.proto`

- [ ] Add death save event messages in the combat events section:
  ```protobuf
  // DeathSaveRolledEvent is sent when a character rolls a death saving throw
  message DeathSaveRolledEvent {
    string character_id = 1;
    int32 roll = 2;
    bool is_success = 3;
    bool is_critical_fail = 4;
    bool is_critical_success = 5;
    DeathSaveProgress progress = 6;
    // If nat 20: character regains consciousness at 1 HP
    bool regained_consciousness = 7;
    int32 hp_restored = 8;
  }

  // CharacterDiedEvent is sent when a character accumulates 3 death save failures
  message CharacterDiedEvent {
    string character_id = 1;
    string character_name = 2;
  }

  // CharacterStabilizedEvent is sent when a character accumulates 3 death save successes
  message CharacterStabilizedEvent {
    string character_id = 1;
    string character_name = 2;
  }

  // CharacterUnconsciousEvent is sent when a character drops to 0 HP
  message CharacterUnconsciousEvent {
    string character_id = 1;
    string character_name = 2;
    string source_entity_id = 3;
  }
  ```

- [ ] Add rest event messages:
  ```protobuf
  // RestCompletedEvent is sent when the party completes a rest between rooms
  message RestCompletedEvent {
    RestType rest_type = 1;
    repeated string character_ids = 2;
    // Per-character HP after rest
    repeated CharacterCombatState updated_characters = 3;
  }
  ```

### 1.7 Wire events into EncounterEvent oneof

**File:** `dnd5e/api/v1alpha1/encounter.proto`

- [ ] Add to the `EncounterEvent.event` oneof (death save events in 50-59 range):
  ```protobuf
  // Death save events
  DeathSaveRolledEvent death_save_rolled = 50;
  CharacterDiedEvent character_died = 51;
  CharacterStabilizedEvent character_stabilized = 52;
  CharacterUnconsciousEvent character_unconscious = 53;

  // Rest events
  RestCompletedEvent rest_completed = 55;
  ```

### 1.8 Add Rest RPCs

**File:** `dnd5e/api/v1alpha1/encounter.proto`

- [ ] Add rest request/response messages:
  ```protobuf
  // ShortRestRequest initiates a short rest for the party between rooms
  message ShortRestRequest {
    string dungeon_id = 1;
    // Character IDs spending hit dice (with how many)
    repeated HitDiceSpend hit_dice_spends = 2;
  }

  // HitDiceSpend specifies how many hit dice a character spends during short rest
  message HitDiceSpend {
    string character_id = 1;
    int32 dice_count = 2;
  }

  // ShortRestResponse returns updated character state after short rest
  message ShortRestResponse {
    bool success = 1;
    string error = 2;
    repeated CharacterCombatState updated_characters = 3;
  }

  // LongRestRequest initiates a long rest for the party
  message LongRestRequest {
    string dungeon_id = 1;
  }

  // LongRestResponse returns updated character state after long rest
  message LongRestResponse {
    bool success = 1;
    string error = 2;
    repeated CharacterCombatState updated_characters = 3;
  }
  ```

- [ ] Add RPCs to `EncounterService`:
  ```protobuf
  // === Rest ===

  // ShortRest performs a short rest for the party between rooms
  rpc ShortRest(ShortRestRequest) returns (ShortRestResponse);

  // LongRest performs a long rest for the party
  rpc LongRest(LongRestRequest) returns (LongRestResponse);
  ```

### 1.9 Format, lint, and push

- [ ] `buf format -w`
- [ ] `buf lint`
- [ ] `buf generate` (verify no errors)
- [ ] `git add dnd5e/api/v1alpha1/enums.proto dnd5e/api/v1alpha1/encounter.proto`
- [ ] `git commit -m "feat: add death save, unconscious, and rest proto contracts"`
- [ ] `git push -u origin feat/death-saves-rest-protos`
- [ ] `gh pr create --title "feat: add death save and rest proto contracts" --body "Adds CONDITION_ID_UNCONSCIOUS, DeathSaveProgress, CharacterCombatState, death save events, rest RPCs, and RestType enum for issue #6"`

---

## Task 2: Create UnconsciousCondition in toolkit (rpg-toolkit)

**Repo:** `/home/kirk/personal/rpg-toolkit`
**Branch:** `feat/unconscious-condition`
**Depends on:** Task 1 merged (for proto enum, but toolkit doesn't import protos — can proceed independently)

### 2.1 Setup

- [ ] `cd /home/kirk/personal/rpg-toolkit && git checkout main && git pull origin main`
- [ ] `git checkout -b feat/unconscious-condition`

### 2.2 Add `IsCritical` field to `DamageReceivedEvent`

**File:** `rulebooks/dnd5e/events/events.go`

- [ ] Add `IsCritical bool` field to the existing `DamageReceivedEvent` struct (after `DamageType`). Per RAW, a critical hit on an unconscious character causes 2 death save failures instead of 1. Without this field, the UnconsciousCondition cannot distinguish critical hits from normal hits.

### 2.3 Add death save event types and topics

**File:** `rulebooks/dnd5e/events/events.go`

- [ ] Add `DeathSaveRolledEvent` struct:
  ```go
  // DeathSaveRolledEvent is published when a death save is rolled for an unconscious character
  type DeathSaveRolledEvent struct {
      CharacterID           string
      Roll                  int
      IsSuccess             bool
      IsCriticalFail        bool
      IsCriticalSuccess     bool
      Successes             int
      Failures              int
      Stabilized            bool
      Dead                  bool
      RegainedConsciousness bool
      HPRestored            int
  }
  ```
- [ ] Add `CharacterDiedEvent` struct:
  ```go
  // CharacterDiedEvent is published when a character accumulates 3 death save failures
  type CharacterDiedEvent struct {
      CharacterID string
  }
  ```
- [ ] Add `CharacterStabilizedEvent` struct:
  ```go
  // CharacterStabilizedEvent is published when a character accumulates 3 death save successes
  type CharacterStabilizedEvent struct {
      CharacterID string
  }
  ```
- [ ] Add topics:
  ```go
  DeathSaveRolledTopic = events.DefineTypedTopic[DeathSaveRolledEvent]("dnd5e.death_save.rolled")
  CharacterDiedTopic = events.DefineTypedTopic[CharacterDiedEvent]("dnd5e.death_save.died")
  CharacterStabilizedTopic = events.DefineTypedTopic[CharacterStabilizedEvent]("dnd5e.death_save.stabilized")
  ```

### 2.4 Write UnconsciousCondition tests first (TDD)

**File:** `rulebooks/dnd5e/conditions/unconscious_test.go`

- [ ] Create test suite `UnconsciousConditionTestSuite` with mock event bus and mock roller
- [ ] Test: `TestApply_SubscribesToEvents` — verifies subscriptions to TurnStartTopic, DamageReceivedTopic, HealingReceivedTopic
- [ ] Test: `TestApply_AlreadyApplied_ReturnsError` — calling Apply twice errors
- [ ] Test: `TestRemove_Unsubscribes` — verifies all subscriptions are removed
- [ ] Test: `TestOnTurnStart_RollsDeathSave` — on turn start for this character, calls `MakeDeathSave`, publishes `DeathSaveRolledEvent`
- [ ] Test: `TestOnTurnStart_IgnoresOtherCharacters` — other character turn starts are ignored
- [ ] Test: `TestOnTurnStart_CriticalFail_TwoFailures` — roll 1 adds 2 failures
- [ ] Test: `TestOnTurnStart_CriticalSuccess_RegainsConsciousness` — roll 20 removes condition, publishes healing
- [ ] Test: `TestOnTurnStart_ThreeFailures_Dies` — publishes `CharacterDiedEvent`
- [ ] Test: `TestOnTurnStart_ThreeSuccesses_Stabilizes` — publishes `CharacterStabilizedEvent`
- [ ] Test: `TestOnDamageReceived_AddsFailure` — damage while unconscious adds 1 failure
- [ ] Test: `TestOnDamageReceived_CriticalHit_AddsTwoFailures` — critical adds 2
- [ ] Test: `TestOnDamageReceived_IgnoresOtherCharacters` — other character damage ignored
- [ ] Test: `TestOnHealingReceived_RemovesCondition` — healing above 0 removes unconscious
- [ ] Test: `TestOnHealingReceived_IgnoresOtherCharacters` — other character healing ignored
- [ ] Test: `TestToJSON_RoundTrip` — serialize and deserialize preserves state
- [ ] Test: `TestIsApplied_ReflectsBusState` — true when applied, false after remove

### 2.5 Implement UnconsciousCondition

**File:** `rulebooks/dnd5e/conditions/unconscious.go`

- [ ] Define `UnconsciousData` struct (JSON serialization):
  ```go
  type UnconsciousData struct {
      Ref         *core.Ref `json:"ref"`
      CharacterID string    `json:"character_id"`
      Successes   int       `json:"successes"`
      Failures    int       `json:"failures"`
      Stabilized  bool      `json:"stabilized"`
      Dead        bool      `json:"dead"`
  }
  ```
- [ ] Define `UnconsciousCondition` struct:
  ```go
  type UnconsciousCondition struct {
      CharacterID     string
      Roller          dice.Roller
      deathSaveState  *saves.DeathSaveState
      subscriptionIDs []string
      bus             events.EventBus
  }
  ```
- [ ] Implement `IsApplied() bool` — returns `r.bus != nil`
- [ ] Implement `Apply(ctx, bus) error`:
  - Subscribe to `TurnStartTopic` (auto-roll death save on this character's turn)
  - Subscribe to `DamageReceivedTopic` (auto-failure on damage)
  - Subscribe to `HealingReceivedTopic` (wake up if healed)
  - Store subscription IDs for cleanup
- [ ] Implement `Remove(ctx, bus) error` — unsubscribe all, nil out bus
- [ ] Implement `onTurnStart` handler:
  - Guard: skip if `event.CharacterID != c.CharacterID`
  - Guard: skip if already stabilized or dead
  - Call `saves.MakeDeathSave(ctx, &saves.DeathSaveInput{Roller: c.Roller, State: c.deathSaveState})`
  - Update internal state from result
  - Publish `DeathSaveRolledEvent` to `DeathSaveRolledTopic`
  - If dead: publish `CharacterDiedEvent` to `CharacterDiedTopic`
  - If stabilized: publish `CharacterStabilizedEvent` to `CharacterStabilizedTopic`
  - If nat 20: publish `HealingReceivedEvent` to `HealingReceivedTopic` (1 HP), then remove self
- [ ] Implement `onDamageReceived` handler:
  - Guard: skip if `event.TargetID != c.CharacterID`
  - Guard: skip if already stabilized or dead
  - Call `saves.TakeDamageWhileUnconscious(ctx, &saves.DamageWhileUnconsciousInput{State: c.deathSaveState, IsCritical: event.IsCritical})`
  - Uses `IsCritical` field added to `DamageReceivedEvent` in step 2.2
  - Update internal state
  - Publish `DeathSaveRolledEvent` (with roll=0 to indicate damage, not a roll)
  - If dead: publish `CharacterDiedEvent`
- [ ] Implement `onHealingReceived` handler:
  - Guard: skip if `event.TargetID != c.CharacterID`
  - Guard: skip if dead
  - Reset death save state (successes=0, failures=0, stabilized=false)
  - Remove self (call `Remove`)
- [ ] Implement `ToJSON() (json.RawMessage, error)` — marshal `UnconsciousData`
- [ ] Implement `loadJSON(data json.RawMessage) error` — unmarshal and populate fields
- [ ] Add `var _ dnd5eEvents.ConditionBehavior = (*UnconsciousCondition)(nil)` compile check

### 2.6 Register in loader

**File:** `rulebooks/dnd5e/conditions/loader.go`

- [ ] Add case to `LoadJSON` switch:
  ```go
  case refs.Conditions.Unconscious().ID:
      uc := &UnconsciousCondition{}
      if err := uc.loadJSON(data); err != nil {
          return nil, rpgerr.Wrap(err, "failed to load unconscious condition")
      }
      return uc, nil
  ```

### 2.7 Run tests and push

- [ ] `cd /home/kirk/personal/rpg-toolkit && go test ./rulebooks/dnd5e/conditions/... -v -run TestUnconsciousSuite`
- [ ] `make pre-commit`
- [ ] `git add rulebooks/dnd5e/conditions/unconscious.go rulebooks/dnd5e/conditions/unconscious_test.go rulebooks/dnd5e/conditions/loader.go rulebooks/dnd5e/events/events.go`
- [ ] `git commit -m "feat: add UnconsciousCondition with death save automation"`
- [ ] `git push -u origin feat/unconscious-condition`
- [ ] `gh pr create --title "feat: add UnconsciousCondition with death save automation" --body "Implements UnconsciousCondition that auto-rolls death saves on turn start, handles damage while unconscious, and wakes up on healing. Uses existing saves.MakeDeathSave and saves.TakeDamageWhileUnconscious. For issue #6"`

---

## Task 3: Add rest and death save API orchestration (rpg-api)

**Repo:** `/home/kirk/personal/rpg-api`
**Branch:** `feat/death-saves-rest-api`
**Depends on:** Task 1 merged (proto contracts), Task 2 merged (UnconsciousCondition)

### 3.1 Setup

- [ ] `cd /home/kirk/personal/rpg-api && git checkout main && git pull origin main`
- [ ] `git checkout -b feat/death-saves-rest-api`
- [ ] `go get github.com/KirkDiggler/rpg-api-protos@latest` (pick up new proto gen)
- [ ] `go get github.com/KirkDiggler/rpg-toolkit@latest` (pick up UnconsciousCondition)
- [ ] `go mod tidy`

### 3.2 Handle dropping to 0 HP — apply UnconsciousCondition

Damage is applied to characters via the event-driven flow: `ResolveAttack` (line ~280) publishes `DamageReceivedEvent`, which character handlers process. The key functions where strike damage is resolved and HP checked are:
- `executeStrike` (line ~4280) — handles STRIKE and OFF_HAND_STRIKE actions
- `executeFlurryStrike` (line ~4444) — handles Flurry of Blows unarmed strikes
- `resolveMonsterAttack` (line ~291 in `monster_turns.go`) — monster attacks against characters

After damage resolution in each of these, check if the character dropped to 0 HP:

- [ ] In each damage resolution path listed above, after damage is applied, check if `character.CurrentHP() <= 0`
- [ ] If so, create `UnconsciousCondition{CharacterID: characterID}` and publish `ConditionAppliedEvent` to apply it
- [ ] Broadcast `CharacterUnconsciousEvent` via encounter event stream
- [ ] Write test: `TestDamageReducesToZero_AppliesUnconsciousCondition`

### 3.3 Handle death save event forwarding

The UnconsciousCondition publishes toolkit events. The API needs to forward these as proto encounter events.

- [ ] Subscribe to `DeathSaveRolledTopic` on the event bus
- [ ] On death save rolled: broadcast `DeathSaveRolledEvent` proto to all connected clients via event stream
- [ ] Subscribe to `CharacterDiedTopic` on the event bus
- [ ] On character died: broadcast `CharacterDiedEvent` proto, update encounter state
- [ ] Subscribe to `CharacterStabilizedTopic` on the event bus
- [ ] On character stabilized: broadcast `CharacterStabilizedEvent` proto
- [ ] Write test: `TestDeathSaveRolled_BroadcastsToClients`
- [ ] Write test: `TestCharacterDied_BroadcastsAndUpdatesState`
- [ ] Write test: `TestCharacterStabilized_BroadcastsToClients`

### 3.4 Handle healing waking unconscious characters

- [ ] When healing brings a character above 0 HP, the UnconsciousCondition auto-removes itself via `onHealingReceived`
- [ ] Verify the existing healing flow publishes `HealingReceivedEvent` so the condition reacts
- [ ] Write test: `TestHealing_WakesUnconsciousCharacter`

### 3.5 Implement ShortRest RPC

- [ ] Create handler for `ShortRest` RPC in encounter service
- [ ] Validate: dungeon exists, combat is not active (between rooms)
- [ ] For each `HitDiceSpend`: call toolkit's `character.ShortRest()` with the dice count
- [ ] Persist updated character state
- [ ] Broadcast `RestCompletedEvent` to event stream
- [ ] Return `ShortRestResponse` with updated `CharacterCombatState` for each character
- [ ] Write test: `TestShortRest_SpendsHitDice_RestoresHP`
- [ ] Write test: `TestShortRest_DuringCombat_ReturnsError`

### 3.6 Implement LongRest RPC

- [ ] Create handler for `LongRest` RPC in encounter service
- [ ] Validate: dungeon exists, combat is not active
- [ ] For each character: call toolkit's `character.LongRest()`
- [ ] Persist updated character state
- [ ] Broadcast `RestCompletedEvent` to event stream
- [ ] Return `LongRestResponse` with updated `CharacterCombatState`
- [ ] Write test: `TestLongRest_RestoresFullHP_ResetsResources`
- [ ] Write test: `TestLongRest_DuringCombat_ReturnsError`

### 3.7 Update GetEncounterState to include CharacterCombatState

- [ ] In `GetEncounterState` handler, populate `CharacterCombatState` for each character
- [ ] Include death save progress if unconscious: read from the `UnconsciousCondition`'s internal state via `character.GetConditions()` — iterate conditions, find the `UnconsciousCondition`, and extract its `UnconsciousData` (successes, failures, stabilized, dead) to populate `DeathSaveProgress`
- [ ] Write test: `TestGetEncounterState_IncludesCharacterCombatState`

### 3.8 Run tests and push

- [ ] `make pre-commit`
- [ ] `make ci-check`
- [ ] `git add .` (review staged files first)
- [ ] `git commit -m "feat: add death save orchestration and rest RPCs"`
- [ ] `git push -u origin feat/death-saves-rest-api`
- [ ] `gh pr create --title "feat: add death save orchestration and rest RPCs" --body "Handles dropping to 0 HP (applies UnconsciousCondition), forwards death save events to clients, implements ShortRest/LongRest RPCs. For issue #6"`

---

## Task 4: Web UI for death saves and rest (rpg-dnd5e-web)

**Repo:** `/home/kirk/personal/rpg-dnd5e-web`
**Branch:** `feat/death-saves-rest-ui`
**Depends on:** Task 1 merged (proto types), Task 3 merged (API endpoints)

### 4.1 Setup

- [ ] `cd /home/kirk/personal/rpg-dnd5e-web && git checkout main && git pull origin main`
- [ ] `git checkout -b feat/death-saves-rest-ui`
- [ ] `npm install` (pick up latest generated proto types)

### 4.2 Add death save pip tracker component

- [ ] Create `DeathSaveTracker` component showing 3 success pips and 3 failure pips
- [ ] Props: `successCount: number`, `failureCount: number`, `stabilized: boolean`, `dead: boolean`
- [ ] Filled pip for each success/failure, empty for remaining
- [ ] Show "Stabilized" or "Dead" label when resolved
- [ ] Style: green pips for success, red for failure, grey for empty

### 4.3 Integrate death save tracker into character card

- [ ] Find the character card/status component used during combat
- [ ] Conditionally render `DeathSaveTracker` when character is unconscious (`is_unconscious: true` from `CharacterCombatState`)
- [ ] Hide normal HP bar when unconscious, show death save pips instead

### 4.4 Handle death save events in event stream

- [ ] Add handler for `death_save_rolled` event type in the event stream processor
- [ ] Show toast/notification: "Character rolled X on death save" with success/failure color
- [ ] On critical success (nat 20): show prominent notification "Character regains consciousness!"
- [ ] Add handler for `character_died` event — show death notification, grey out character
- [ ] Add handler for `character_stabilized` event — show stabilized notification
- [ ] Add handler for `character_unconscious` event — show unconscious notification, update character state

### 4.5 Add rest button between rooms

- [ ] Find the room transition / door UI (between encounters)
- [ ] Add "Short Rest" and "Long Rest" buttons when combat is not active
- [ ] Short Rest: show hit dice spending UI (how many hit dice to spend per character)
- [ ] Long Rest: simple confirmation dialog
- [ ] Call `ShortRest` or `LongRest` RPC on button click
- [ ] Update character HP display from response
- [ ] Handle `rest_completed` event in stream to update all clients

### 4.6 Test and push

- [ ] `npm run ci-check`
- [ ] `git add .` (review staged files)
- [ ] `git commit -m "feat: add death save tracker and rest UI"`
- [ ] `git push -u origin feat/death-saves-rest-ui`
- [ ] `gh pr create --title "feat: add death save tracker and rest UI" --body "Adds DeathSaveTracker pip component, death save event handling in event stream, and short/long rest buttons between rooms. For issue #6"`

---

## Implementation Order Summary

```
Task 1 (protos)  ──→  Task 3 (API) ──→  Task 4 (Web)
                  ↗
Task 2 (toolkit) ─┘
```

Tasks 1 and 2 can run in parallel (toolkit does not import protos). Tasks 3 and 4 depend on both. Task 4 depends on Task 3.

## Key Files Reference

| What | Where |
|------|-------|
| Proto enums | `/home/kirk/personal/rpg-api-protos/dnd5e/api/v1alpha1/enums.proto` |
| Proto encounter | `/home/kirk/personal/rpg-api-protos/dnd5e/api/v1alpha1/encounter.proto` |
| Toolkit death saves | `/home/kirk/personal/rpg-toolkit/rulebooks/dnd5e/saves/death_saves.go` |
| Toolkit events | `/home/kirk/personal/rpg-toolkit/rulebooks/dnd5e/events/events.go` |
| Toolkit conditions | `/home/kirk/personal/rpg-toolkit/rulebooks/dnd5e/conditions/` |
| Toolkit condition loader | `/home/kirk/personal/rpg-toolkit/rulebooks/dnd5e/conditions/loader.go` |
| Toolkit refs (unconscious) | `/home/kirk/personal/rpg-toolkit/rulebooks/dnd5e/refs/conditions.go` |
| Existing condition model (Raging) | `/home/kirk/personal/rpg-toolkit/rulebooks/dnd5e/conditions/raging.go` |
| Proto CLAUDE.md | `/home/kirk/personal/rpg-api-protos/CLAUDE.md` |
