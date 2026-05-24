# Unified Action System Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move action economy onto the Character in the toolkit so the API delegates instead of hardcoding ability/action lists.

**Architecture:** The Character struct gains an embedded `*ActionEconomy` (nil outside combat). New methods — `StartTurn`, `AvailableAbilities`, `AvailableActions`, `ActivateAbility`, `ExecuteAction`, `EndTurn` — compute class-specific ability/action lists from features, equipment, and combat state. The API deletes `buildAvailableAbilities`/`buildAvailableActions` and delegates to the Character. Proto gets `resource_current`/`resource_max` fields and new enum values.

**Tech Stack:** Go (rpg-toolkit, rpg-api), Protocol Buffers (rpg-api-protos), React/TypeScript (rpg-dnd5e-web)

**Spec:** `rpg-project/docs/superpowers/specs/2026-03-21-unified-action-system-design.md`

---

## File Structure

### rpg-api-protos (Task 1)
- Modify: `dnd5e/api/v1alpha1/enums.proto` — add RAGE, SECOND_WIND, MARTIAL_ARTS_BONUS enum values
- Modify: `dnd5e/api/v1alpha1/encounter.proto` — add resource_current/resource_max to AvailableAbility, remove dodge_active/disengage_active from ActionEconomy

### rpg-toolkit (Tasks 2–6)
- Modify: `rulebooks/dnd5e/features/loader.go` — extend Feature interface with `Ref()` and `Name()` methods
- Modify: `rulebooks/dnd5e/character/data.go` — add ActionEconomyData field to Data struct
- Create: `rulebooks/dnd5e/character/action_economy.go` — StartTurn, AvailableAbilities, AvailableActions, ActivateAbility, ExecuteAction, EndTurn, InCombat, ExitCombat, persistence helpers
- Create: `rulebooks/dnd5e/character/action_economy_types.go` — AvailableAbility, AvailableAction, ActionEconomyData, all Input/Output types, GrantedActionKey constants
- Create: `rulebooks/dnd5e/character/action_economy_test.go` — tests for all action economy methods

### rpg-api (Tasks 7–8)
- Modify: `internal/orchestrators/encounter/orchestrator.go` — delete buildAvailableAbilities/buildAvailableActions, update ActivateCombatAbility/ExecuteAction to delegate to character
- Modify: `internal/handlers/dnd5e/v1alpha1/encounter/converters.go` — update proto converters for new fields
- Modify: `internal/entities/encounter.go` — remove ActionEconomyState (moved to character)

### rpg-dnd5e-web (Task 9 — out of scope for initial PR, tracked separately)
- Render resource_current/resource_max on abilities
- Handle new CombatAbilityId enum values

---

## Task 1: Proto Changes (rpg-api-protos)

**Repo:** `rpg-api-protos`
**Branch from:** `main`
**Files:**
- Modify: `dnd5e/api/v1alpha1/enums.proto:850-865` — extend CombatAbilityId enum
- Modify: `dnd5e/api/v1alpha1/encounter.proto:202-211` — add fields to AvailableAbility
- Modify: `dnd5e/api/v1alpha1/encounter.proto:172-198` — remove dodge/disengage from ActionEconomy

- [ ] **Step 1: Create issue on project board**

Create a GitHub issue: "Proto: Add resource fields to AvailableAbility, extend CombatAbilityId for unified action system"

- [ ] **Step 2: Create feature branch**

```bash
cd /home/kirk/personal/rpg-api-protos
git checkout main && git pull
git checkout -b feat/unified-action-proto
```

- [ ] **Step 3: Extend CombatAbilityId enum**

In `dnd5e/api/v1alpha1/enums.proto`, add after `COMBAT_ABILITY_ID_FLURRY_OF_BLOWS = 11`:

```protobuf
  // Class features that cost action economy
  COMBAT_ABILITY_ID_RAGE = 12;                 // Barbarian: bonus action, consumes rage charge
  COMBAT_ABILITY_ID_SECOND_WIND = 13;          // Fighter: bonus action, consumes second wind use
  COMBAT_ABILITY_ID_MARTIAL_ARTS_BONUS = 14;   // Monk: bonus action, granted after monk weapon/unarmed strike
```

- [ ] **Step 4: Add resource fields to AvailableAbility**

In `dnd5e/api/v1alpha1/encounter.proto`, modify the `AvailableAbility` message to add:

```protobuf
message AvailableAbility {
  CombatAbilityId ability_id = 1;
  string name = 2;
  bool can_use = 3;
  string reason = 4;
  int32 resource_current = 5;  // Current charges (0 = no resource tracking)
  int32 resource_max = 6;      // Max charges (0 = no resource tracking)
}
```

- [ ] **Step 5: Remove dodge_active and disengage_active from ActionEconomy**

In `dnd5e/api/v1alpha1/encounter.proto`, remove fields 9 (`disengage_active`) and 10 (`dodge_active`) from the `ActionEconomy` message. Mark them as reserved:

```protobuf
  reserved 9, 10;
  reserved "disengage_active", "dodge_active";
```

- [ ] **Step 6: Format and validate**

```bash
buf format -w
buf lint
buf breaking --against '.git#branch=main'
```

Note: The breaking change check will flag removed fields. This is intentional — document in PR description.

- [ ] **Step 7: Commit and push**

```bash
git add dnd5e/api/v1alpha1/enums.proto dnd5e/api/v1alpha1/encounter.proto
git commit -m "feat: add resource fields to AvailableAbility, extend CombatAbilityId for unified action system"
git push -u origin feat/unified-action-proto
```

- [ ] **Step 8: Create PR**

PR title: "Proto: Resource fields + class ability enums for unified action system"
Link to spec. Note breaking changes (dodge_active/disengage_active removal).

**Wait for PR merge and CI-generated code before proceeding to Tasks 2+.**

---

## Task 2: Action Economy Types (rpg-toolkit)

**Repo:** `rpg-toolkit`
**Branch from:** `main`
**Files:**
- Create: `rulebooks/dnd5e/character/action_economy_types.go`

This task defines all the types the action economy system needs. No logic yet.

- [ ] **Step 1: Create issue on project board**

Create a GitHub issue: "Toolkit: Unified action system — action economy on Character"

- [ ] **Step 2: Create feature branch**

```bash
cd /home/kirk/personal/rpg-toolkit
git checkout main && git pull
git checkout -b feat/unified-action-economy
```

- [ ] **Step 3: Write the types file**

Create `rulebooks/dnd5e/character/action_economy_types.go`:

```go
package character

import (
	"github.com/KirkDiggler/rpg-toolkit/core"
	coreCombat "github.com/KirkDiggler/rpg-toolkit/core/combat"
)

// AvailableAbility represents something a character can do that consumes
// primary action economy resources (action, bonus action, reaction).
type AvailableAbility struct {
	Ref             *core.Ref            // "dnd5e:combat_abilities:attack"
	Name            string               // "Attack"
	ActionType      coreCombat.ActionType // ActionStandard, ActionBonus, ActionReaction
	CanUse          bool                 // computed from current action economy state
	Reason          string               // why CanUse is false (empty when true)
	ResourceCurrent int                  // current charges (0 if no resource)
	ResourceMax     int                  // max charges (0 if no resource)
}

// AvailableAction represents something a character can do that consumes
// granted capacity (attacks remaining, movement remaining, etc.).
type AvailableAction struct {
	Ref    *core.Ref // "dnd5e:actions:strike"
	Name   string    // "Strike"
	CanUse bool      // computed from granted capacity
	Reason string    // why CanUse is false (empty when true)
}

// StartTurnInput provides input for initializing the action economy at turn start.
type StartTurnInput struct {
	Speed int // character's movement speed (30ft default)
}

// StartTurnOutput contains the initialized action economy state.
type StartTurnOutput struct {
	Abilities []AvailableAbility
	Actions   []AvailableAction
}

// ActivateAbilityInput provides input for activating a combat ability or feature.
type ActivateAbilityInput struct {
	AbilityRef *core.Ref // which ability to activate
}

// ActivateAbilityOutput contains the result of activating an ability.
type ActivateAbilityOutput struct {
	Success         bool
	Error           string
	GrantedCapacity string // "1 attack", "30ft movement", etc.
	Abilities       []AvailableAbility
	Actions         []AvailableAction
}

// ExecuteActionInput provides input for executing an action that consumes capacity.
type ExecuteActionInput struct {
	ActionRef *core.Ref // which action to execute
	TargetID  string    // target entity ID (for strikes)
}

// ExecuteActionOutput contains the result of executing an action.
// Note: actual attack resolution (damage, hit/miss) happens in the API
// orchestrator. The Character only manages action economy feasibility.
type ExecuteActionOutput struct {
	Success   bool
	Error     string
	Abilities []AvailableAbility
	Actions   []AvailableAction
}

// EndTurnInput provides input for ending a turn (currently empty).
type EndTurnInput struct{}

// EndTurnOutput contains the result of ending a turn (currently empty).
type EndTurnOutput struct{}

// ExitCombatInput provides input for exiting combat entirely.
type ExitCombatInput struct{}

// ExitCombatOutput contains the result of exiting combat.
type ExitCombatOutput struct{}

// GrantedActionKey identifies a type of granted capacity in the action economy.
type GrantedActionKey string

const (
	// GrantedAttacks tracks the number of attacks remaining (from Attack ability)
	GrantedAttacks GrantedActionKey = "attacks"

	// GrantedOffHandStrikes tracks off-hand strikes remaining (from two-weapon fighting)
	GrantedOffHandStrikes GrantedActionKey = "off_hand_strikes"

	// GrantedFlurryStrikes tracks flurry strikes remaining (from Flurry of Blows)
	GrantedFlurryStrikes GrantedActionKey = "flurry_strikes"

	// GrantedMartialArtsBonus tracks martial arts bonus strike (from Martial Arts)
	GrantedMartialArtsBonus GrantedActionKey = "martial_arts_bonus"

	// GrantedOffHandAttack tracks off-hand attack ability availability
	GrantedOffHandAttack GrantedActionKey = "off_hand_attack"
)

// ActionEconomyData is the serializable form of the action economy state.
// Lives on Character.Data.ActionEconomy (nil outside combat, omitempty).
type ActionEconomyData struct {
	ActionsRemaining      int                      `json:"actions_remaining"`
	BonusActionsRemaining int                      `json:"bonus_actions_remaining"`
	ReactionsRemaining    int                      `json:"reactions_remaining"`
	MovementRemaining     int                      `json:"movement_remaining"`
	Granted               map[GrantedActionKey]int  `json:"granted,omitempty"`
}
```

- [ ] **Step 4: Verify it compiles**

```bash
cd /home/kirk/personal/rpg-toolkit
go build ./rulebooks/dnd5e/character/...
```

- [ ] **Step 5: Commit**

```bash
git add rulebooks/dnd5e/character/action_economy_types.go
git commit -m "feat: add action economy types for unified action system"
```

---

## Task 3: Action Economy Data Persistence (rpg-toolkit)

**Repo:** `rpg-toolkit` (same branch as Task 2)
**Files:**
- Modify: `rulebooks/dnd5e/character/data.go` — add ActionEconomy field
- Modify: `rulebooks/dnd5e/character/character.go` — add actionEconomy field, InCombat method
- Create: `rulebooks/dnd5e/character/action_economy_test.go` — persistence tests

- [ ] **Step 1: Write persistence test**

Create `rulebooks/dnd5e/character/action_economy_test.go`:

```go
package character

import (
	"context"
	"encoding/json"
	"testing"

	"github.com/KirkDiggler/rpg-toolkit/events"
	"github.com/stretchr/testify/suite"
)

type ActionEconomyTestSuite struct {
	suite.Suite
	ctx context.Context
	bus events.EventBus
}

func (s *ActionEconomyTestSuite) SetupTest() {
	s.ctx = context.Background()
	s.bus = events.NewEventBus()
}

func TestActionEconomyTestSuite(t *testing.T) {
	suite.Run(t, new(ActionEconomyTestSuite))
}

func (s *ActionEconomyTestSuite) TestInCombat_NilActionEconomy() {
	char := &Character{}
	s.False(char.InCombat())
}

func (s *ActionEconomyTestSuite) TestInCombat_WithActionEconomy() {
	char := &Character{
		actionEconomy: &ActionEconomyData{
			ActionsRemaining:      1,
			BonusActionsRemaining: 1,
			ReactionsRemaining:    1,
		},
	}
	s.True(char.InCombat())
}

func (s *ActionEconomyTestSuite) TestToData_IncludesActionEconomy() {
	char := createTestCharacter(s.T())
	char.actionEconomy = &ActionEconomyData{
		ActionsRemaining:      1,
		BonusActionsRemaining: 0,
		ReactionsRemaining:    1,
		MovementRemaining:     15,
		Granted: map[GrantedActionKey]int{
			GrantedAttacks: 1,
		},
	}

	data := char.ToData()
	s.Require().NotNil(data.ActionEconomy)
	s.Equal(1, data.ActionEconomy.ActionsRemaining)
	s.Equal(0, data.ActionEconomy.BonusActionsRemaining)
	s.Equal(15, data.ActionEconomy.MovementRemaining)
	s.Equal(1, data.ActionEconomy.Granted[GrantedAttacks])
}

func (s *ActionEconomyTestSuite) TestToData_NilActionEconomyOmitted() {
	char := createTestCharacter(s.T())
	char.actionEconomy = nil

	data := char.ToData()
	s.Nil(data.ActionEconomy)

	// Verify it marshals without the field
	bytes, err := json.Marshal(data)
	s.Require().NoError(err)
	s.NotContains(string(bytes), "action_economy")
}

func (s *ActionEconomyTestSuite) TestLoadFromData_RestoresActionEconomy() {
	char := createTestCharacter(s.T())
	char.actionEconomy = &ActionEconomyData{
		ActionsRemaining:      0,
		BonusActionsRemaining: 1,
		ReactionsRemaining:    1,
		MovementRemaining:     30,
		Granted: map[GrantedActionKey]int{
			GrantedAttacks: 2,
		},
	}

	// Round-trip through data
	data := char.ToData()
	restored, err := LoadFromData(s.ctx, data, s.bus)
	s.Require().NoError(err)

	s.True(restored.InCombat())
	s.Equal(0, restored.actionEconomy.ActionsRemaining)
	s.Equal(30, restored.actionEconomy.MovementRemaining)
	s.Equal(2, restored.actionEconomy.Granted[GrantedAttacks])
}

func (s *ActionEconomyTestSuite) TestLoadFromData_NilActionEconomy() {
	char := createTestCharacter(s.T())
	char.actionEconomy = nil

	data := char.ToData()
	restored, err := LoadFromData(s.ctx, data, s.bus)
	s.Require().NoError(err)

	s.False(restored.InCombat())
}
```

Note: `createTestCharacter` is a helper — check if one exists in the package already. If not, create a minimal one that builds a valid Character for testing.

- [ ] **Step 2: Run test to verify it fails**

```bash
cd /home/kirk/personal/rpg-toolkit
go test ./rulebooks/dnd5e/character/... -run TestActionEconomyTestSuite -v
```

Expected: compile errors — `actionEconomy` field doesn't exist, `InCombat` not defined.

- [ ] **Step 3: Add actionEconomy field to Character struct**

In `rulebooks/dnd5e/character/character.go`, add after the `dirty bool` field (line ~104):

```go
	// Action economy state (nil outside combat)
	actionEconomy *ActionEconomyData
```

- [ ] **Step 4: Add InCombat method to Character**

In `rulebooks/dnd5e/character/action_economy.go` (create this file):

```go
package character

// InCombat returns true if the character is currently in combat.
// Combat is indicated by the action economy being initialized (non-nil).
func (c *Character) InCombat() bool {
	return c.actionEconomy != nil
}
```

- [ ] **Step 5: Add ActionEconomy field to Data struct**

In `rulebooks/dnd5e/character/data.go`, add after `Conditions` field (line ~79):

```go
	// Action economy state (nil outside combat)
	ActionEconomy *ActionEconomyData `json:"action_economy,omitempty"`
```

- [ ] **Step 6: Update ToData to include action economy**

In `rulebooks/dnd5e/character/character.go`, in the `ToData()` method, add before the return (after conditions serialization, around line 975):

```go
	// Copy action economy state (nil outside combat)
	data.ActionEconomy = c.actionEconomy
```

- [ ] **Step 7: Update LoadFromData to restore action economy**

In `rulebooks/dnd5e/character/data.go`, in `LoadFromData()`, add after setting the `dirty` field and before the feature loading section (around line 145):

```go
	// Restore action economy state (nil outside combat is fine)
	char.actionEconomy = d.ActionEconomy
```

- [ ] **Step 8: Run tests to verify they pass**

```bash
cd /home/kirk/personal/rpg-toolkit
go test ./rulebooks/dnd5e/character/... -run TestActionEconomyTestSuite -v
```

Expected: all 5 tests PASS.

- [ ] **Step 9: Commit**

```bash
git add rulebooks/dnd5e/character/
git commit -m "feat: add ActionEconomy persistence on Character"
```

---

## Task 4: StartTurn and EndTurn (rpg-toolkit)

**Repo:** `rpg-toolkit` (same branch)
**Files:**
- Modify: `rulebooks/dnd5e/character/action_economy.go` — add StartTurn, EndTurn
- Modify: `rulebooks/dnd5e/character/action_economy_test.go` — add tests

- [ ] **Step 1: Write StartTurn test**

Add to `action_economy_test.go`:

```go
func (s *ActionEconomyTestSuite) TestStartTurn() {
	char := createTestCharacter(s.T())

	output, err := char.StartTurn(s.ctx, &StartTurnInput{Speed: 30})
	s.Require().NoError(err)

	s.True(char.InCombat())
	s.Equal(1, char.actionEconomy.ActionsRemaining)
	s.Equal(1, char.actionEconomy.BonusActionsRemaining)
	s.Equal(1, char.actionEconomy.ReactionsRemaining)
	s.Equal(30, char.actionEconomy.MovementRemaining)
	s.NotEmpty(output.Abilities)
	s.NotEmpty(output.Actions) // Move is always available
}

func (s *ActionEconomyTestSuite) TestStartTurn_ResetsFromPreviousTurn() {
	char := createTestCharacter(s.T())

	// Simulate previous turn with spent resources
	char.actionEconomy = &ActionEconomyData{
		ActionsRemaining:      0,
		BonusActionsRemaining: 0,
		ReactionsRemaining:    0,
		MovementRemaining:     0,
		Granted: map[GrantedActionKey]int{
			GrantedAttacks: 1,
		},
	}

	output, err := char.StartTurn(s.ctx, &StartTurnInput{Speed: 30})
	s.Require().NoError(err)

	s.Equal(1, char.actionEconomy.ActionsRemaining)
	s.Equal(1, char.actionEconomy.BonusActionsRemaining)
	s.Equal(1, char.actionEconomy.ReactionsRemaining)
	s.Equal(30, char.actionEconomy.MovementRemaining)
	// Granted capacity should be cleared
	s.Empty(char.actionEconomy.Granted)
	s.NotEmpty(output.Abilities)
}

func (s *ActionEconomyTestSuite) TestEndTurn_ResetsButStaysInCombat() {
	char := createTestCharacter(s.T())
	char.actionEconomy = &ActionEconomyData{
		ActionsRemaining: 0,
		Granted: map[GrantedActionKey]int{
			GrantedAttacks: 1,
		},
	}

	_, err := char.EndTurn(s.ctx, &EndTurnInput{})
	s.Require().NoError(err)
	s.True(char.InCombat()) // Still in combat between turns
	s.Equal(0, char.actionEconomy.ActionsRemaining)
	s.Empty(char.actionEconomy.Granted) // Granted capacity cleared
}

func (s *ActionEconomyTestSuite) TestExitCombat() {
	char := createTestCharacter(s.T())
	char.actionEconomy = &ActionEconomyData{
		ActionsRemaining: 1,
	}

	_, err := char.ExitCombat(s.ctx, &ExitCombatInput{})
	s.Require().NoError(err)
	s.False(char.InCombat()) // No longer in combat
}
```

- [ ] **Step 2: Run tests to verify they fail**

```bash
go test ./rulebooks/dnd5e/character/... -run "TestStartTurn|TestEndTurn" -v
```

- [ ] **Step 3: Implement StartTurn and EndTurn**

Add to `rulebooks/dnd5e/character/action_economy.go`:

```go
import (
	"context"

	"github.com/KirkDiggler/rpg-toolkit/rpgerr"
)

// StartTurn initializes the action economy for a new combat turn.
// This resets all primary resources and sets movement from the character's speed.
func (c *Character) StartTurn(ctx context.Context, input *StartTurnInput) (*StartTurnOutput, error) {
	if input == nil {
		return nil, rpgerr.New(rpgerr.CodeInvalidArgument, "input is required")
	}

	c.actionEconomy = &ActionEconomyData{
		ActionsRemaining:      1,
		BonusActionsRemaining: 1,
		ReactionsRemaining:    1,
		MovementRemaining:     input.Speed,
		Granted:               make(map[GrantedActionKey]int),
	}

	return &StartTurnOutput{
		Abilities: c.AvailableAbilities(),
		Actions:   c.AvailableActions(),
	}, nil
}

// EndTurn resets the action economy for the end of this character's turn.
// The character remains in combat — use ExitCombat to leave combat entirely.
func (c *Character) EndTurn(_ context.Context, _ *EndTurnInput) (*EndTurnOutput, error) {
	if c.actionEconomy != nil {
		// Reset primary resources and clear granted capacity
		c.actionEconomy.ActionsRemaining = 0
		c.actionEconomy.BonusActionsRemaining = 0
		c.actionEconomy.ReactionsRemaining = 0
		c.actionEconomy.MovementRemaining = 0
		c.actionEconomy.Granted = make(map[GrantedActionKey]int)
	}
	return &EndTurnOutput{}, nil
}

// ExitCombat clears the action economy entirely, removing combat state.
// Call this when the encounter ends, not between turns.
func (c *Character) ExitCombat(_ context.Context, _ *ExitCombatInput) (*ExitCombatOutput, error) {
	c.actionEconomy = nil
	return &ExitCombatOutput{}, nil
}
```

- [ ] **Step 4: Add stub AvailableAbilities and AvailableActions**

Add to `rulebooks/dnd5e/character/action_economy.go`:

```go
// AvailableAbilities returns the abilities available to this character.
// Safe to call outside combat — returns empty slice.
func (c *Character) AvailableAbilities() []AvailableAbility {
	if !c.InCombat() {
		return []AvailableAbility{}
	}
	return c.buildAvailableAbilities()
}

// AvailableActions returns the actions available to this character.
// Safe to call outside combat — returns empty slice.
func (c *Character) AvailableActions() []AvailableAction {
	if !c.InCombat() {
		return []AvailableAction{}
	}
	return c.buildAvailableActions()
}

// buildAvailableAbilities computes the full list of abilities from combat abilities and features.
func (c *Character) buildAvailableAbilities() []AvailableAbility {
	var abilities []AvailableAbility

	// Standard combat abilities (Attack, Dash, Dodge, Disengage)
	// CombatAbility interface has Ref(), Name(), ActionType() from base.go
	for _, ca := range c.combatAbilities {
		ability := AvailableAbility{
			Ref:        ca.Ref(),
			Name:       ca.Name(),
			ActionType: ca.ActionType(),
			CanUse:     c.canUseAbilityByActionType(ca.ActionType()),
		}
		if !ability.CanUse {
			ability.Reason = c.actionTypeExhaustedReason(ca.ActionType())
		}
		abilities = append(abilities, ability)
	}

	// Features that cost action economy (Rage, Second Wind, etc.)
	// PREREQUISITE: The d5e Feature interface must be extended with Ref() and Name().
	// See Task 2 Step 3a for this change.
	for _, f := range c.features {
		ability := AvailableAbility{
			Ref:        f.Ref(),
			Name:       f.Name(),
			ActionType: f.ActionType(),
			CanUse:     c.canUseAbilityByActionType(f.ActionType()),
		}
		// Check feature-specific resource availability
		if ability.CanUse {
			if err := f.CanActivate(context.Background(), c, features.FeatureInput{}); err != nil {
				ability.CanUse = false
				ability.Reason = err.Error()
			}
		} else {
			ability.Reason = c.actionTypeExhaustedReason(f.ActionType())
		}

		// Add resource tracking
		ability.ResourceCurrent, ability.ResourceMax = c.featureResourceInfo(f)

		abilities = append(abilities, ability)
	}

	// Equipment-based: Off-Hand Attack (two light weapons equipped)
	if c.hasTwoLightWeapons() {
		ability := AvailableAbility{
			Ref:        refs.CombatAbilities.OffHandAttack(),
			Name:       "Off-Hand Attack",
			ActionType: coreCombat.ActionBonus,
			CanUse:     c.canUseAbilityByActionType(coreCombat.ActionBonus),
		}
		if !ability.CanUse {
			ability.Reason = c.actionTypeExhaustedReason(coreCombat.ActionBonus)
		}
		abilities = append(abilities, ability)
	}

	return abilities
}

// hasTwoLightWeapons checks if the character has two light weapons equipped.
func (c *Character) hasTwoLightWeapons() bool {
	mainHand := c.GetEquippedSlot(SlotMainHand)
	offHand := c.GetEquippedSlot(SlotOffHand)

	mainWeapon := mainHand.AsWeapon()
	offWeapon := offHand.AsWeapon()

	if mainWeapon == nil || offWeapon == nil {
		return false
	}

	return mainWeapon.HasProperty(weapons.PropertyLight) && offWeapon.HasProperty(weapons.PropertyLight)
}

// buildAvailableActions computes actions from granted capacity and permanent actions.
func (c *Character) buildAvailableActions() []AvailableAction {
	var result []AvailableAction

	// Move is always available if movement remains
	result = append(result, AvailableAction{
		Ref:    refs.Actions.Move(),
		Name:   "Move",
		CanUse: c.actionEconomy.MovementRemaining > 0,
		Reason: c.actionReason(c.actionEconomy.MovementRemaining > 0, "no movement remaining"),
	})

	// Strike — available when attacks are granted
	if attacks, ok := c.actionEconomy.Granted[GrantedAttacks]; ok && attacks > 0 {
		result = append(result, AvailableAction{
			Ref:    refs.Actions.Strike(),
			Name:   "Strike",
			CanUse: true,
		})
	}

	// Off-Hand Strike — available when off-hand attacks are granted
	if oha, ok := c.actionEconomy.Granted[GrantedOffHandStrikes]; ok && oha > 0 {
		result = append(result, AvailableAction{
			Ref:    refs.Actions.OffHandStrike(),
			Name:   "Off-Hand Strike",
			CanUse: true,
		})
	}

	// Flurry Strike — available when flurry strikes are granted
	if fs, ok := c.actionEconomy.Granted[GrantedFlurryStrikes]; ok && fs > 0 {
		result = append(result, AvailableAction{
			Ref:    refs.Actions.FlurryStrike(),
			Name:   "Flurry Strike",
			CanUse: true,
		})
	}

	// Martial Arts Bonus Strike (Unarmed Strike) — available when granted
	if mab, ok := c.actionEconomy.Granted[GrantedMartialArtsBonus]; ok && mab > 0 {
		result = append(result, AvailableAction{
			Ref:    refs.Actions.UnarmedStrike(),
			Name:   "Unarmed Strike",
			CanUse: true,
		})
	}

	return result
}

// canUseAbilityByActionType checks if the action economy has the resource for this action type.
func (c *Character) canUseAbilityByActionType(actionType coreCombat.ActionType) bool {
	switch actionType {
	case coreCombat.ActionStandard:
		return c.actionEconomy.ActionsRemaining > 0
	case coreCombat.ActionBonus:
		return c.actionEconomy.BonusActionsRemaining > 0
	case coreCombat.ActionReaction:
		return c.actionEconomy.ReactionsRemaining > 0
	case coreCombat.ActionFree:
		return true
	default:
		return false
	}
}

// actionTypeExhaustedReason returns the reason string for an exhausted action type.
func (c *Character) actionTypeExhaustedReason(actionType coreCombat.ActionType) string {
	switch actionType {
	case coreCombat.ActionStandard:
		return "no actions remaining"
	case coreCombat.ActionBonus:
		return "no bonus actions remaining"
	case coreCombat.ActionReaction:
		return "no reactions remaining"
	default:
		return "unknown action type"
	}
}

// actionReason returns the reason string if the condition is false, empty string if true.
func (c *Character) actionReason(canUse bool, reason string) string {
	if canUse {
		return ""
	}
	return reason
}

// featureResourceInfo returns the current and max resource for a feature.
// Returns (0, 0) for features without resource tracking.
func (c *Character) featureResourceInfo(f features.Feature) (current, max int) {
	ref := f.Ref()
	if ref == nil {
		return 0, 0
	}

	// Map feature ref to resource key.
	// Rage uses resources.RageCharges ("rage_charges").
	// SecondWind uses the ref ID as key ("second_wind") — its RecoverableResource
	// is stored with refs.Features.SecondWind().ID as the key.
	var resourceKey coreResources.ResourceKey
	switch ref.ID {
	case refs.Features.Rage().ID:
		resourceKey = resources.RageCharges
	case refs.Features.SecondWind().ID:
		// SecondWind stores its resource with the feature ref ID as key
		resourceKey = coreResources.ResourceKey(refs.Features.SecondWind().ID)
	default:
		return 0, 0
	}

	// Look up resource on the character
	resource, ok := c.resources[resourceKey]
	if !ok {
		return 0, 0
	}
	return resource.Current(), resource.Maximum()
}
```

Note: This needs imports for `context`, `coreCombat`, `coreResources`, `features`, `refs`, `resources`, `rpgerr`. Add them at the top of the file.

- [ ] **Step 5: Run tests**

```bash
go test ./rulebooks/dnd5e/character/... -run TestActionEconomyTestSuite -v
```

- [ ] **Step 6: Commit**

```bash
git add rulebooks/dnd5e/character/
git commit -m "feat: add StartTurn, EndTurn, AvailableAbilities, AvailableActions"
```

---

## Task 5: ActivateAbility (rpg-toolkit)

**Repo:** `rpg-toolkit` (same branch)
**Files:**
- Modify: `rulebooks/dnd5e/character/action_economy.go`
- Modify: `rulebooks/dnd5e/character/action_economy_test.go`

- [ ] **Step 1: Write ActivateAbility tests**

Add to `action_economy_test.go`. Use helpers to create test characters of specific classes:

```go
func (s *ActionEconomyTestSuite) TestActivateAbility_Attack() {
	char := createTestFighterCharacter(s.T(), s.bus)
	_, err := char.StartTurn(s.ctx, &StartTurnInput{Speed: 30})
	s.Require().NoError(err)

	output, err := char.ActivateAbility(s.ctx, &ActivateAbilityInput{
		AbilityRef: refs.CombatAbilities.Attack(),
	})
	s.Require().NoError(err)
	s.True(output.Success)
	s.Equal("1 attack", output.GrantedCapacity)
	s.Equal(0, char.actionEconomy.ActionsRemaining)
	s.Equal(1, char.actionEconomy.Granted[GrantedAttacks])
}

func (s *ActionEconomyTestSuite) TestActivateAbility_Dash() {
	char := createTestFighterCharacter(s.T(), s.bus)
	_, err := char.StartTurn(s.ctx, &StartTurnInput{Speed: 30})
	s.Require().NoError(err)

	output, err := char.ActivateAbility(s.ctx, &ActivateAbilityInput{
		AbilityRef: refs.CombatAbilities.Dash(),
	})
	s.Require().NoError(err)
	s.True(output.Success)
	s.Equal(60, char.actionEconomy.MovementRemaining) // 30 + 30
}

func (s *ActionEconomyTestSuite) TestActivateAbility_NoActionRemaining() {
	char := createTestFighterCharacter(s.T(), s.bus)
	_, err := char.StartTurn(s.ctx, &StartTurnInput{Speed: 30})
	s.Require().NoError(err)

	// Use the action
	_, err = char.ActivateAbility(s.ctx, &ActivateAbilityInput{
		AbilityRef: refs.CombatAbilities.Attack(),
	})
	s.Require().NoError(err)

	// Try to use another action
	output, err := char.ActivateAbility(s.ctx, &ActivateAbilityInput{
		AbilityRef: refs.CombatAbilities.Dash(),
	})
	s.Require().NoError(err)
	s.False(output.Success)
	s.Contains(output.Error, "no actions remaining")
}

func (s *ActionEconomyTestSuite) TestActivateAbility_NotInCombat() {
	char := createTestFighterCharacter(s.T(), s.bus)

	output, err := char.ActivateAbility(s.ctx, &ActivateAbilityInput{
		AbilityRef: refs.CombatAbilities.Attack(),
	})
	s.Require().NoError(err)
	s.False(output.Success)
	s.Contains(output.Error, "not in combat")
}

func (s *ActionEconomyTestSuite) TestActivateAbility_Rage() {
	char := createTestBarbarianCharacter(s.T(), s.bus)
	_, err := char.StartTurn(s.ctx, &StartTurnInput{Speed: 30})
	s.Require().NoError(err)

	output, err := char.ActivateAbility(s.ctx, &ActivateAbilityInput{
		AbilityRef: refs.Features.Rage(),
	})
	s.Require().NoError(err)
	s.True(output.Success)
	s.Equal(0, char.actionEconomy.BonusActionsRemaining)
}

func (s *ActionEconomyTestSuite) TestActivateAbility_SecondWind() {
	char := createTestFighterCharacter(s.T(), s.bus)
	// Reduce HP to test healing
	char.hitPoints = char.maxHitPoints - 5
	_, err := char.StartTurn(s.ctx, &StartTurnInput{Speed: 30})
	s.Require().NoError(err)

	output, err := char.ActivateAbility(s.ctx, &ActivateAbilityInput{
		AbilityRef: refs.Features.SecondWind(),
	})
	s.Require().NoError(err)
	s.True(output.Success)
	s.Equal(0, char.actionEconomy.BonusActionsRemaining)
}
```

- [ ] **Step 2: Run tests to verify they fail**

```bash
go test ./rulebooks/dnd5e/character/... -run "TestActivateAbility" -v
```

- [ ] **Step 3: Implement ActivateAbility**

Add to `action_economy.go`:

```go
// ActivateAbility activates a combat ability or feature, consuming action economy
// and granting capacity or effects.
// Safe to call outside combat — returns error output, never panics.
func (c *Character) ActivateAbility(ctx context.Context, input *ActivateAbilityInput) (*ActivateAbilityOutput, error) {
	if input == nil || input.AbilityRef == nil {
		return nil, rpgerr.New(rpgerr.CodeInvalidArgument, "input and ability ref are required")
	}

	if !c.InCombat() {
		return &ActivateAbilityOutput{
			Success:   false,
			Error:     "not in combat",
			Abilities: c.AvailableAbilities(),
			Actions:   c.AvailableActions(),
		}, nil
	}

	// Try combat abilities first
	for _, ca := range c.combatAbilities {
		if ca.Ref().Equals(input.AbilityRef) {
			return c.activateCombatAbility(ctx, ca)
		}
	}

	// Try features
	for _, f := range c.features {
		if f.Ref().Equals(input.AbilityRef) {
			return c.activateFeature(ctx, f)
		}
	}

	return &ActivateAbilityOutput{
		Success:   false,
		Error:     fmt.Sprintf("unknown ability: %s", input.AbilityRef.String()),
		Abilities: c.AvailableAbilities(),
		Actions:   c.AvailableActions(),
	}, nil
}

// activateCombatAbility handles activation of standard combat abilities (Attack, Dash, Dodge, Disengage).
// Uses a bridge pattern: creates a temporary *combat.ActionEconomy from ActionEconomyData,
// delegates to the existing CombatAbility.Activate(), then syncs state back.
func (c *Character) activateCombatAbility(ctx context.Context, ca combatabilities.CombatAbility) (*ActivateAbilityOutput, error) {
	// Build the bridge: create toolkit ActionEconomy from our data
	ae := c.toToolkitActionEconomy()

	// Build CombatAbilityInput with everything the ability needs
	input := combatabilities.CombatAbilityInput{
		Bus:           c.bus,
		ActionEconomy: ae,
		ActionHolder:  c,
		Speed:         c.GetSpeed(),
		ExtraAttacks:  0, // TODO: check for Extra Attack feature
	}

	// Check if ability can be activated (uses existing validation)
	if err := ca.CanActivate(ctx, c, input); err != nil {
		return &ActivateAbilityOutput{
			Success:   false,
			Error:     err.Error(),
			Abilities: c.AvailableAbilities(),
			Actions:   c.AvailableActions(),
		}, nil
	}

	// Activate (existing code handles action economy consumption + effects)
	if err := ca.Activate(ctx, c, input); err != nil {
		return &ActivateAbilityOutput{
			Success:   false,
			Error:     err.Error(),
			Abilities: c.AvailableAbilities(),
			Actions:   c.AvailableActions(),
		}, nil
	}

	// Sync state back from toolkit ActionEconomy to our data
	c.fromToolkitActionEconomy(ae)

	// Determine granted capacity description
	grantedCapacity := c.describeGrantedCapacity(ca)

	return &ActivateAbilityOutput{
		Success:         true,
		GrantedCapacity: grantedCapacity,
		Abilities:       c.AvailableAbilities(),
		Actions:         c.AvailableActions(),
	}, nil
}

// toToolkitActionEconomy creates a *combat.ActionEconomy from our ActionEconomyData.
// This bridges our flat data struct to the existing CombatAbility.Activate() interface.
func (c *Character) toToolkitActionEconomy() *combat.ActionEconomy {
	ae := combat.NewActionEconomy()
	ae.ActionsRemaining = c.actionEconomy.ActionsRemaining
	ae.BonusActionsRemaining = c.actionEconomy.BonusActionsRemaining
	ae.ReactionsRemaining = c.actionEconomy.ReactionsRemaining
	ae.MovementRemaining = c.actionEconomy.MovementRemaining
	ae.AttacksRemaining = c.actionEconomy.Granted[GrantedAttacks]
	ae.OffHandAttacksRemaining = c.actionEconomy.Granted[GrantedOffHandStrikes]
	ae.FlurryStrikesRemaining = c.actionEconomy.Granted[GrantedFlurryStrikes]
	return ae
}

// fromToolkitActionEconomy syncs state from *combat.ActionEconomy back to ActionEconomyData.
func (c *Character) fromToolkitActionEconomy(ae *combat.ActionEconomy) {
	c.actionEconomy.ActionsRemaining = ae.ActionsRemaining
	c.actionEconomy.BonusActionsRemaining = ae.BonusActionsRemaining
	c.actionEconomy.ReactionsRemaining = ae.ReactionsRemaining
	c.actionEconomy.MovementRemaining = ae.MovementRemaining
	if c.actionEconomy.Granted == nil {
		c.actionEconomy.Granted = make(map[GrantedActionKey]int)
	}
	c.actionEconomy.Granted[GrantedAttacks] = ae.AttacksRemaining
	c.actionEconomy.Granted[GrantedOffHandStrikes] = ae.OffHandAttacksRemaining
	c.actionEconomy.Granted[GrantedFlurryStrikes] = ae.FlurryStrikesRemaining
}

// describeGrantedCapacity returns a human-readable description of what an ability granted.
func (c *Character) describeGrantedCapacity(ca combatabilities.CombatAbility) string {
	ref := ca.Ref()
	switch {
	case ref.Equals(refs.CombatAbilities.Attack()):
		count := c.actionEconomy.Granted[GrantedAttacks]
		return fmt.Sprintf("%d attack", count)
	case ref.Equals(refs.CombatAbilities.Dash()):
		return fmt.Sprintf("%dft movement", c.GetSpeed())
	case ref.Equals(refs.CombatAbilities.Dodge()):
		return "dodging until next turn"
	case ref.Equals(refs.CombatAbilities.Disengage()):
		return "disengaging until next turn"
	default:
		return ""
	}
}

// activateFeature handles activation of class features (Rage, Second Wind, etc.).
// Features handle their own resource consumption (charges) but do NOT consume action economy.
// The Character consumes the action economy here, then delegates to Feature.Activate()
// for resource consumption and effects.
func (c *Character) activateFeature(ctx context.Context, f features.Feature) (*ActivateAbilityOutput, error) {
	// Check action economy
	if !c.canUseAbilityByActionType(f.ActionType()) {
		return &ActivateAbilityOutput{
			Success:   false,
			Error:     c.actionTypeExhaustedReason(f.ActionType()),
			Abilities: c.AvailableAbilities(),
			Actions:   c.AvailableActions(),
		}, nil
	}

	// Check feature-specific availability (resource charges, etc.)
	if err := f.CanActivate(ctx, c, features.FeatureInput{}); err != nil {
		return &ActivateAbilityOutput{
			Success:   false,
			Error:     err.Error(),
			Abilities: c.AvailableAbilities(),
			Actions:   c.AvailableActions(),
		}, nil
	}

	// Consume action economy (feature does NOT consume action economy — it only consumes charges)
	c.consumeActionType(f.ActionType())

	// Activate the feature (consumes charges, applies conditions via bus)
	// Note: ActionEconomy is nil because the feature doesn't manage action economy —
	// we already consumed it above. Features only need the bus for condition events.
	if err := f.Activate(ctx, c, features.FeatureInput{Bus: c.bus}); err != nil {
		// Rollback action economy consumption
		c.restoreActionType(f.ActionType())
		return &ActivateAbilityOutput{
			Success:   false,
			Error:     err.Error(),
			Abilities: c.AvailableAbilities(),
			Actions:   c.AvailableActions(),
		}, nil
	}

	return &ActivateAbilityOutput{
		Success:   true,
		Abilities: c.AvailableAbilities(),
		Actions:   c.AvailableActions(),
	}, nil
}

// consumeActionType decrements the appropriate action economy counter.
func (c *Character) consumeActionType(actionType coreCombat.ActionType) {
	switch actionType {
	case coreCombat.ActionStandard:
		c.actionEconomy.ActionsRemaining--
	case coreCombat.ActionBonus:
		c.actionEconomy.BonusActionsRemaining--
	case coreCombat.ActionReaction:
		c.actionEconomy.ReactionsRemaining--
	}
}

// restoreActionType increments the appropriate action economy counter (rollback).
func (c *Character) restoreActionType(actionType coreCombat.ActionType) {
	switch actionType {
	case coreCombat.ActionStandard:
		c.actionEconomy.ActionsRemaining++
	case coreCombat.ActionBonus:
		c.actionEconomy.BonusActionsRemaining++
	case coreCombat.ActionReaction:
		c.actionEconomy.ReactionsRemaining++
	}
}

```

- [ ] **Step 4: Extend d5e Feature interface with Ref() and Name()**

The d5e `features.Feature` interface currently only has:
- `core.Action[FeatureInput]` → gives `GetID()`, `GetType()`, `CanActivate()`, `Activate()`
- `ToJSON()`, `ActionType()`

It does NOT have `Ref()` or `Name()` — but all concrete implementations (Rage, SecondWind, etc.) already have these via the mechanics-level Feature interface. Add them to the d5e interface.

In `rulebooks/dnd5e/features/loader.go` (where the Feature interface is defined), change:
```go
type Feature interface {
	core.Action[FeatureInput]
	ToJSON() (json.RawMessage, error)
	ActionType() combat.ActionType
}
```
to:
```go
type Feature interface {
	core.Action[FeatureInput]
	Ref() *core.Ref
	Name() string
	ToJSON() (json.RawMessage, error)
	ActionType() combat.ActionType
}
```

Verify all concrete implementations (Rage, SecondWind, ActionSurge, FlurryOfBlows, etc.) already satisfy these — they inherit from the mechanics Feature interface or implement directly. Run `go build ./rulebooks/dnd5e/features/...` to verify.

Also add `OffHandAttack` ref to `refs/combat_abilities.go`:
```go
var combatAbilityOffHandAttack = &core.Ref{Module: Module, Type: TypeCombatAbilities, ID: "offhand_attack"}

func (n combatAbilitiesNS) OffHandAttack() *core.Ref { return combatAbilityOffHandAttack }
```

- [ ] **Step 5: Create test helper functions**

Create test helpers at the bottom of `action_economy_test.go` or in a `test_helpers_test.go` file. These should create Characters with the right class, combat abilities, features, and resources. Check existing test helpers in the character package first — there may already be patterns to follow.

```go
// createTestFighterCharacter creates a Fighter with combat abilities and Second Wind
func createTestFighterCharacter(t *testing.T, bus events.EventBus) *Character {
	// Build a minimal fighter character with:
	// - combatAbilities: Attack, Dash, Dodge, Disengage
	// - features: SecondWind
	// - resources: second_wind resource
	// Follow the existing patterns in the character package tests
}

// createTestBarbarianCharacter creates a Barbarian with combat abilities and Rage
func createTestBarbarianCharacter(t *testing.T, bus events.EventBus) *Character {
	// Build a minimal barbarian character with:
	// - combatAbilities: Attack, Dash, Dodge, Disengage
	// - features: Rage
	// - resources: rage_charges resource
}
```

- [ ] **Step 6: Run tests**

```bash
go test ./rulebooks/dnd5e/character/... -run "TestActivateAbility" -v
```

- [ ] **Step 7: Run all character tests to check for regressions**

```bash
go test ./rulebooks/dnd5e/character/... -v
```

- [ ] **Step 8: Commit**

```bash
git add rulebooks/dnd5e/character/
git commit -m "feat: add ActivateAbility for combat abilities and features"
```

---

## Task 6: ExecuteAction (rpg-toolkit)

**Repo:** `rpg-toolkit` (same branch)
**Files:**
- Modify: `rulebooks/dnd5e/character/action_economy.go`
- Modify: `rulebooks/dnd5e/character/action_economy_test.go`

- [ ] **Step 1: Write ExecuteAction tests**

```go
func (s *ActionEconomyTestSuite) TestExecuteAction_Strike() {
	char := createTestFighterCharacter(s.T(), s.bus)
	_, err := char.StartTurn(s.ctx, &StartTurnInput{Speed: 30})
	s.Require().NoError(err)

	// Activate Attack first to grant attacks
	_, err = char.ActivateAbility(s.ctx, &ActivateAbilityInput{
		AbilityRef: refs.CombatAbilities.Attack(),
	})
	s.Require().NoError(err)

	// Execute Strike
	output, err := char.ExecuteAction(s.ctx, &ExecuteActionInput{
		ActionRef: refs.Actions.Strike(),
	})
	s.Require().NoError(err)
	s.True(output.Success)
	s.Equal(0, char.actionEconomy.Granted[GrantedAttacks])
}

func (s *ActionEconomyTestSuite) TestExecuteAction_Strike_NoAttacks() {
	char := createTestFighterCharacter(s.T(), s.bus)
	_, err := char.StartTurn(s.ctx, &StartTurnInput{Speed: 30})
	s.Require().NoError(err)

	// Don't activate Attack — no attacks granted
	output, err := char.ExecuteAction(s.ctx, &ExecuteActionInput{
		ActionRef: refs.Actions.Strike(),
	})
	s.Require().NoError(err)
	s.False(output.Success)
	s.Contains(output.Error, "no attacks remaining")
}

func (s *ActionEconomyTestSuite) TestExecuteAction_NotInCombat() {
	char := createTestFighterCharacter(s.T(), s.bus)

	output, err := char.ExecuteAction(s.ctx, &ExecuteActionInput{
		ActionRef: refs.Actions.Strike(),
	})
	s.Require().NoError(err)
	s.False(output.Success)
	s.Contains(output.Error, "not in combat")
}
```

- [ ] **Step 2: Run tests to verify they fail**

```bash
go test ./rulebooks/dnd5e/character/... -run "TestExecuteAction" -v
```

- [ ] **Step 3: Implement ExecuteAction**

Add to `action_economy.go`:

```go
// ExecuteAction executes a combat action that consumes granted capacity.
// Safe to call outside combat — returns error output, never panics.
// Note: Attack resolution (damage, hit/miss) happens in the API orchestrator.
// This method only manages action economy feasibility.
func (c *Character) ExecuteAction(ctx context.Context, input *ExecuteActionInput) (*ExecuteActionOutput, error) {
	if input == nil || input.ActionRef == nil {
		return nil, rpgerr.New(rpgerr.CodeInvalidArgument, "input and action ref are required")
	}

	if !c.InCombat() {
		return &ExecuteActionOutput{
			Success:   false,
			Error:     "not in combat",
			Abilities: c.AvailableAbilities(),
			Actions:   c.AvailableActions(),
		}, nil
	}

	ref := input.ActionRef

	switch {
	case ref.Equals(refs.Actions.Strike()):
		return c.executeStrike(ctx)

	case ref.Equals(refs.Actions.OffHandStrike()):
		return c.executeOffHandStrike(ctx)

	case ref.Equals(refs.Actions.FlurryStrike()):
		return c.executeFlurryStrike(ctx)

	case ref.Equals(refs.Actions.UnarmedStrike()):
		return c.executeUnarmedStrike(ctx)

	default:
		return &ExecuteActionOutput{
			Success:   false,
			Error:     fmt.Sprintf("unknown action: %s", ref.String()),
			Abilities: c.AvailableAbilities(),
			Actions:   c.AvailableActions(),
		}, nil
	}
}

func (c *Character) executeStrike(ctx context.Context) (*ExecuteActionOutput, error) {
	attacks := c.actionEconomy.Granted[GrantedAttacks]
	if attacks <= 0 {
		return &ExecuteActionOutput{
			Success:   false,
			Error:     "no attacks remaining",
			Abilities: c.AvailableAbilities(),
			Actions:   c.AvailableActions(),
		}, nil
	}

	c.actionEconomy.Granted[GrantedAttacks]--

	// Post-strike grants: Character knows its class and equipment, so it computes
	// what abilities/actions become available after a strike.
	c.checkPostStrikeGrants(ctx)

	return &ExecuteActionOutput{
		Success:   true,
		Abilities: c.AvailableAbilities(),
		Actions:   c.AvailableActions(),
	}, nil
}

// checkPostStrikeGrants checks class rules after a main-hand strike and grants
// bonus abilities/actions (Martial Arts Bonus Strike, Off-Hand Attack).
// This keeps game logic in the toolkit, not the API.
func (c *Character) checkPostStrikeGrants(ctx context.Context) {
	// Monk: grant Martial Arts Bonus Strike if attack used monk weapon or unarmed
	if c.classID == classes.Monk {
		// Check if martial arts bonus already granted this turn
		if !c.HasGranted(GrantedMartialArtsBonus) {
			mainHand := c.GetEquippedSlot(SlotMainHand)
			weapon := mainHand.AsWeapon()
			isMonkWeapon := weapon == nil || isMonkWeaponCheck(weapon) // unarmed or monk weapon
			if isMonkWeapon && c.canUseAbilityByActionType(coreCombat.ActionBonus) {
				c.GrantCapacity(GrantedMartialArtsBonus, 1)
			}
		}
	}

	// Two-weapon fighting: grant Off-Hand Attack if two light weapons and bonus action available
	if c.hasTwoLightWeapons() && !c.HasGranted(GrantedOffHandStrikes) {
		if c.canUseAbilityByActionType(coreCombat.ActionBonus) {
			c.GrantCapacity(GrantedOffHandStrikes, 1)
		}
	}
}

func (c *Character) executeOffHandStrike(_ context.Context) (*ExecuteActionOutput, error) {
	oha := c.actionEconomy.Granted[GrantedOffHandStrikes]
	if oha <= 0 {
		return &ExecuteActionOutput{
			Success:   false,
			Error:     "no off-hand attacks remaining",
			Abilities: c.AvailableAbilities(),
			Actions:   c.AvailableActions(),
		}, nil
	}

	c.actionEconomy.Granted[GrantedOffHandStrikes]--

	return &ExecuteActionOutput{
		Success:   true,
		Abilities: c.AvailableAbilities(),
		Actions:   c.AvailableActions(),
	}, nil
}

func (c *Character) executeFlurryStrike(_ context.Context) (*ExecuteActionOutput, error) {
	fs := c.actionEconomy.Granted[GrantedFlurryStrikes]
	if fs <= 0 {
		return &ExecuteActionOutput{
			Success:   false,
			Error:     "no flurry strikes remaining",
			Abilities: c.AvailableAbilities(),
			Actions:   c.AvailableActions(),
		}, nil
	}

	c.actionEconomy.Granted[GrantedFlurryStrikes]--

	return &ExecuteActionOutput{
		Success:   true,
		Abilities: c.AvailableAbilities(),
		Actions:   c.AvailableActions(),
	}, nil
}

func (c *Character) executeUnarmedStrike(_ context.Context) (*ExecuteActionOutput, error) {
	mab := c.actionEconomy.Granted[GrantedMartialArtsBonus]
	if mab <= 0 {
		return &ExecuteActionOutput{
			Success:   false,
			Error:     "no martial arts bonus strikes remaining",
			Abilities: c.AvailableAbilities(),
			Actions:   c.AvailableActions(),
		}, nil
	}

	c.actionEconomy.Granted[GrantedMartialArtsBonus]--

	return &ExecuteActionOutput{
		Success:   true,
		Abilities: c.AvailableAbilities(),
		Actions:   c.AvailableActions(),
	}, nil
}

// GrantCapacity adds granted capacity to the action economy.
// Used by the API after post-strike grant checks (two-weapon fighting, martial arts).
func (c *Character) GrantCapacity(key GrantedActionKey, count int) {
	if c.actionEconomy == nil {
		return
	}
	if c.actionEconomy.Granted == nil {
		c.actionEconomy.Granted = make(map[GrantedActionKey]int)
	}
	c.actionEconomy.Granted[key] = count
}

// HasGranted returns true if the character has any remaining capacity of the given type.
func (c *Character) HasGranted(key GrantedActionKey) bool {
	if c.actionEconomy == nil || c.actionEconomy.Granted == nil {
		return false
	}
	return c.actionEconomy.Granted[key] > 0
}
```

- [ ] **Step 4: Run tests**

```bash
go test ./rulebooks/dnd5e/character/... -run "TestExecuteAction" -v
```

- [ ] **Step 5: Run full test suite**

```bash
cd /home/kirk/personal/rpg-toolkit
go test ./rulebooks/dnd5e/character/... -v
```

- [ ] **Step 6: Run pre-commit**

```bash
cd /home/kirk/personal/rpg-toolkit
make pre-commit
```

- [ ] **Step 7: Commit**

```bash
git add rulebooks/dnd5e/character/
git commit -m "feat: add ExecuteAction and GrantCapacity methods"
```

- [ ] **Step 8: Push and create PR**

```bash
git push -u origin feat/unified-action-economy
```

Create PR linking to the issue. Wait for merge before API changes.

---

## Task 7: API — Update Proto Converters (rpg-api)

**Repo:** `rpg-api`
**Branch from:** `main` (after proto PR merges and `go get` updates)
**Files:**
- Modify: `internal/handlers/dnd5e/v1alpha1/encounter/converters.go`
- Modify: `internal/entities/encounter.go`

**Prerequisite:** Proto PR merged, toolkit PR merged. Run:
```bash
cd /home/kirk/personal/rpg-api
go get github.com/KirkDiggler/rpg-api-protos@latest
go get github.com/KirkDiggler/rpg-toolkit@latest
go mod tidy
```

- [ ] **Step 1: Create issue on project board**

Create a GitHub issue: "API: Delegate action economy to toolkit Character for unified action system"

- [ ] **Step 2: Create feature branch**

```bash
cd /home/kirk/personal/rpg-api
git checkout main && git pull
git checkout -b feat/unified-action-economy
```

- [ ] **Step 3: Update abilityIDToProtoEnum in converters.go**

Add mappings for the new CombatAbilityId values (RAGE, SECOND_WIND, MARTIAL_ARTS_BONUS). The converter needs to map from toolkit `*core.Ref` to proto enum. Add a new function:

```go
// abilityRefToProtoEnum maps a toolkit ability ref to the proto CombatAbilityId enum.
func abilityRefToProtoEnum(ref *core.Ref) pb.CombatAbilityId {
	if ref == nil {
		return pb.CombatAbilityId_COMBAT_ABILITY_ID_UNSPECIFIED
	}

	switch {
	case ref.Equals(refs.CombatAbilities.Attack()):
		return pb.CombatAbilityId_COMBAT_ABILITY_ID_ATTACK
	case ref.Equals(refs.CombatAbilities.Dash()):
		return pb.CombatAbilityId_COMBAT_ABILITY_ID_DASH
	case ref.Equals(refs.CombatAbilities.Dodge()):
		return pb.CombatAbilityId_COMBAT_ABILITY_ID_DODGE
	case ref.Equals(refs.CombatAbilities.Disengage()):
		return pb.CombatAbilityId_COMBAT_ABILITY_ID_DISENGAGE
	case ref.Equals(refs.Features.Rage()):
		return pb.CombatAbilityId_COMBAT_ABILITY_ID_RAGE
	case ref.Equals(refs.Features.SecondWind()):
		return pb.CombatAbilityId_COMBAT_ABILITY_ID_SECOND_WIND
	case ref.Equals(refs.Features.FlurryOfBlows()):
		return pb.CombatAbilityId_COMBAT_ABILITY_ID_FLURRY_OF_BLOWS
	// Martial Arts Bonus is contextual (granted after strike), mapped here too
	default:
		return pb.CombatAbilityId_COMBAT_ABILITY_ID_UNSPECIFIED
	}
}
```

- [ ] **Step 4: Update convertAvailableAbilitiesToProto**

Update to use the new toolkit types and include resource fields:

```go
func convertAvailableAbilitiesToProto(abilities []character.AvailableAbility) []*pb.AvailableAbility {
	result := make([]*pb.AvailableAbility, len(abilities))
	for i, a := range abilities {
		result[i] = &pb.AvailableAbility{
			AbilityId:       abilityRefToProtoEnum(a.Ref),
			Name:            a.Name,
			CanUse:          a.CanUse,
			Reason:          a.Reason,
			ResourceCurrent: int32(a.ResourceCurrent),
			ResourceMax:     int32(a.ResourceMax),
		}
	}
	return result
}
```

- [ ] **Step 5: Update convertAvailableActionsToProto**

Update to use the new toolkit types:

```go
func convertAvailableActionsToProto(actions []character.AvailableAction) []*pb.AvailableAction {
	result := make([]*pb.AvailableAction, len(actions))
	for i, a := range actions {
		result[i] = &pb.AvailableAction{
			ActionId: actionRefToProtoEnum(a.Ref),
			Name:     a.Name,
			CanUse:   a.CanUse,
			Reason:   a.Reason,
		}
	}
	return result
}
```

- [ ] **Step 6: Update convertActionEconomyToProto**

Remove `DodgeActive` and `DisengageActive` fields from the proto conversion.

- [ ] **Step 7: Commit**

```bash
git add internal/handlers/ internal/entities/
git commit -m "feat: update proto converters for toolkit AvailableAbility/AvailableAction types"
```

---

## Task 8: API — Delegate to Toolkit Character (rpg-api)

**Repo:** `rpg-api` (same branch as Task 7)
**Files:**
- Modify: `internal/orchestrators/encounter/orchestrator.go` — the big one

This is the core refactor: replace hardcoded `buildAvailableAbilities`/`buildAvailableActions` with calls to `char.AvailableAbilities()`/`char.AvailableActions()`, and replace the `ActivateCombatAbility` switch-case with `char.ActivateAbility()`.

**Strategy:** Do this incrementally. The Character now handles action economy, so the encounter entity no longer needs `ActionEconomyState`. Instead:
1. On turn start, call `char.StartTurn()` and persist the character
2. On ability activation, call `char.ActivateAbility()` and persist the character
3. On action execution, call `char.ExecuteAction()` and persist the character
4. Delete `buildAvailableAbilities` and `buildAvailableActions`

- [ ] **Step 1: Update ActivateCombatAbility orchestrator**

Replace the switch/case on ability type with:

```go
func (o *Orchestrator) ActivateCombatAbility(ctx context.Context, input *ActivateCombatAbilityInput) (*ActivateCombatAbilityOutput, error) {
	// ... existing validation and encounter loading ...

	// Load character from toolkit
	bus := events.NewEventBus()
	charOutput, err := o.charRepo.Get(ctx, characterrepo.GetInput{ID: input.EntityID})
	// ... error handling ...
	char, err := character.LoadFromData(ctx, charOutput.Character.Data, bus)
	// ... error handling ...

	// Map proto ability ID to toolkit ref
	abilityRef := protoAbilityToRef(input.AbilityID)

	// Delegate to toolkit character
	abilityOutput, err := char.ActivateAbility(ctx, &character.ActivateAbilityInput{
		AbilityRef: abilityRef,
	})
	// ... error handling ...

	if !abilityOutput.Success {
		return &ActivateCombatAbilityOutput{
			Success:            false,
			Error:              abilityOutput.Error,
			AvailableAbilities: convertToolkitAbilities(abilityOutput.Abilities),
			AvailableActions:   convertToolkitActions(abilityOutput.Actions),
		}, nil
	}

	// Persist character with updated action economy
	charData := char.ToData()
	charOutput.Character.Data = charData
	_, err = o.charRepo.Update(ctx, &characterrepo.UpdateInput{Character: charOutput.Character})
	// ... error handling ...

	// Build response
	return &ActivateCombatAbilityOutput{
		Success:            true,
		GrantedCapacity:    abilityOutput.GrantedCapacity,
		AvailableAbilities: convertToolkitAbilities(abilityOutput.Abilities),
		AvailableActions:   convertToolkitActions(abilityOutput.Actions),
		// ... CombatState ...
	}, nil
}
```

Add the helper to convert proto enum to toolkit ref:

```go
func protoAbilityToRef(abilityID pb.CombatAbilityId) *core.Ref {
	switch abilityID {
	case pb.CombatAbilityId_COMBAT_ABILITY_ID_ATTACK:
		return refs.CombatAbilities.Attack()
	case pb.CombatAbilityId_COMBAT_ABILITY_ID_DASH:
		return refs.CombatAbilities.Dash()
	case pb.CombatAbilityId_COMBAT_ABILITY_ID_DODGE:
		return refs.CombatAbilities.Dodge()
	case pb.CombatAbilityId_COMBAT_ABILITY_ID_DISENGAGE:
		return refs.CombatAbilities.Disengage()
	case pb.CombatAbilityId_COMBAT_ABILITY_ID_OFFHAND_ATTACK:
		return refs.CombatAbilities.OffHandAttack() // Added in Task 4 Step 4
	case pb.CombatAbilityId_COMBAT_ABILITY_ID_RAGE:
		return refs.Features.Rage()
	case pb.CombatAbilityId_COMBAT_ABILITY_ID_SECOND_WIND:
		return refs.Features.SecondWind()
	case pb.CombatAbilityId_COMBAT_ABILITY_ID_FLURRY_OF_BLOWS:
		return refs.Features.FlurryOfBlows()
	case pb.CombatAbilityId_COMBAT_ABILITY_ID_MARTIAL_ARTS_BONUS:
		// Map to the Actions ref — Martial Arts Bonus results in an UnarmedStrike action
		return refs.Actions.UnarmedStrike()
	default:
		return nil
	}
}
```

**IMPORTANT:** Before implementing, read the actual ActivateCombatAbility method at `orchestrator.go:4046-4227` to understand all the nuances being replaced. The existing code handles CombatState building, event processing, etc. Preserve those responsibilities — just change where the ability/action lists come from.

- [ ] **Step 2: Update executeStrike and other action methods**

Update `executeStrike` to call `char.ExecuteAction()` to consume the attack instead of `actionEconomy.UseAttack()`. Keep the attack resolution (combat.ResolveAttack) and post-strike grants (CheckAndGrantOffHandStrike/CheckAndGrantMartialArtsBonusStrike). After post-strike grants, update the character's action economy using `char.GrantCapacity()`.

- [ ] **Step 3: Delete buildAvailableAbilities and buildAvailableActions**

Remove these functions from the orchestrator (~lines 2034-2109). All callers should now use `char.AvailableAbilities()` / `char.AvailableActions()` via the toolkit character.

- [ ] **Step 4: Update turn start to call char.StartTurn**

In the `StartTurn` or `AdvanceTurn` flow, call `char.StartTurn()` and persist the character. Remove direct ActionEconomy creation on the encounter.

- [ ] **Step 5: Remove ActionEconomyState from encounter entity**

The `ActionEconomyState` in `internal/entities/encounter.go` is no longer needed — action economy now lives on the character. Remove it from `CombatState` and `EncounterData`. Update the encounter repository's `UpdateInput` to remove the `ActionEconomy` field.

**WARNING:** This is a large change. Multiple methods reference `ActionEconomyState`. Use grep to find all references and update them. Some may need the character's action economy instead.

- [ ] **Step 6: Run tests**

```bash
cd /home/kirk/personal/rpg-api
go test ./... -v
```

Fix any compilation errors or test failures.

- [ ] **Step 7: Run pre-commit**

```bash
make pre-commit
```

- [ ] **Step 8: Commit and push**

```bash
git add .
git commit -m "feat: delegate action economy to toolkit Character, delete hardcoded ability/action lists"
git push -u origin feat/unified-action-economy
```

- [ ] **Step 9: Create PR**

PR title: "API: Unified action system — delegate to toolkit Character"
Link to spec and toolkit PR.

---

## Verification Checklist

After all tasks are complete, verify the success criteria from the spec:

- [ ] 1. A Monk sees "Martial Arts Bonus Strike" (not "Off-Hand Attack") after striking with a monk weapon
- [ ] 2. A Barbarian sees "Rage (2/2)" in their bonus action abilities
- [ ] 3. A Fighter sees "Second Wind (1/1)" in their bonus action abilities
- [ ] 4. A Fighter with two light weapons sees "Off-Hand Attack" in bonus actions
- [ ] 5. The API orchestrator has zero hardcoded ability lists (`grep -r "buildAvailableAbilities" rpg-api/` returns nothing)
- [ ] 6. `buildAvailableAbilities` and `buildAvailableActions` are deleted from the API
- [ ] 7. Action economy persists on the character, not the encounter
- [ ] 8. All methods use Input/Output types

---

## Notes for Implementers

### Dependency Order
Proto → Toolkit → API. Each must merge before the next starts.

### What NOT to Change
- `combat.ResolveAttack` — attack resolution stays in the toolkit, called by the API
- The web rendering of `AvailableAbility[]` / `AvailableAction[]` — minimal changes needed (just new fields)

### What Changes Ownership
- Post-strike grants (`CheckAndGrantOffHandStrike`, `CheckAndGrantMartialArtsBonusStrike`) — the Character now computes these internally via `checkPostStrikeGrants()`. The API no longer calls these directly; it gets updated ability/action lists from `ExecuteActionOutput`.

### Testing Strategy
- Toolkit: Unit tests with testify suites, create test characters of each class
- API: Existing handler/orchestrator tests should be updated to reflect the new flow
- Integration: Manual testing with the web client after all PRs merge

### Off-Hand Attack as Ability
The spec mentions "Off-Hand Attack" appearing in the abilities list for characters with two light weapons. This is equipment-based, not a feature. The `buildAvailableAbilities` method on Character should check `EquipmentSlots` for two light weapons and include it. This is separate from the `CheckAndGrantOffHandStrike` flow (which grants the off-hand **action** after a main-hand strike).

The ability version (`COMBAT_ABILITY_ID_OFFHAND_ATTACK`) consumes a bonus action and grants `GrantedOffHandStrikes`. The action version (`ACTION_ID_OFF_HAND_STRIKE`) consumes from `GrantedOffHandStrikes`. Keep both — the ability is the explicit "I want to off-hand attack" choice, the action is "now execute it."
