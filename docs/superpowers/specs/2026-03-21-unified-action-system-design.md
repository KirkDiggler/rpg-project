# Unified Action System Design

## Problem

The current action economy system has hardcoded ability lists (`buildAvailableAbilities`, `buildAvailableActions`) in the API orchestrator that return the same 5 abilities and 4 actions for every class. Monks see "Off-Hand Attack" when they should see "Martial Arts Bonus Strike." Fighters see the same options as Barbarians. Features like Rage and Second Wind exist in a separate panel with no integration into the action economy flow. The API assembles these lists — violating the boundary rule that the toolkit implements rules.

## Solution

Move action economy onto the Character in the toolkit. The character knows its class, features, conditions, and level — so it computes what abilities are available. The API loads the character, calls methods, and forwards results. No game logic in the API.

## Architecture

### Core Model

The `Character` struct gains an embedded `ActionEconomy` that is:
- **nil outside combat** — character creation, lobby, between dungeons
- **Initialized on turn start** — `char.StartTurn(ctx, input)` populates it from class data
- **Queried by the API** — `char.AvailableAbilities()`, `char.AvailableActions()`
- **Mutated through methods** — `char.ActivateAbility(ctx, input)` consumes resources, grants capacity, updates available options
- **Serialized with the character** — `char.ToData()` includes action economy state, `LoadFromData()` restores it

### Safe Access Pattern

The Character provides `InCombat() bool` for branching, and all action economy methods are safe to call outside combat — they return empty slices or errors, never panic:

```go
// Branching when needed:
if char.InCombat() {
    // include combat state in response
}

// Always safe to call — no nil checks needed:
abilities := char.AvailableAbilities() // empty slice if not in combat
result, err := char.ActivateAbility(ctx, input) // error: "not in combat"
```

Internally, `InCombat()` checks if the action economy is initialized. If the definition of "in combat" evolves, the method handles it. API code never checks for nil directly.

### Types

```go
// AvailableAbility represents something a character can do that consumes
// primary action economy resources (action, bonus action, reaction).
type AvailableAbility struct {
    Ref             core.Ref   // "dnd5e:combat_abilities:attack"
    Name            string     // "Attack"
    ActionType      ActionType // ActionTypeStandard, ActionTypeBonus, ActionTypeReaction
    CanUse          bool       // computed from current action economy state
    Reason          string     // why CanUse is false (empty when true)
    ResourceCurrent int        // current charges (0 if no resource)
    ResourceMax     int        // max charges (0 if no resource)
}

// AvailableAction represents something a character can do that consumes
// granted capacity (attacks remaining, movement remaining, etc.).
type AvailableAction struct {
    Ref    core.Ref // "dnd5e:actions:strike"
    Name   string   // "Strike"
    CanUse bool     // computed from granted capacity
    Reason string   // why CanUse is false (empty when true)
}
```

### Input/Output Types

Every method follows the project's Input/Output pattern:

```go
type StartTurnInput struct {
    Speed int // character's movement speed (30ft default)
}

type StartTurnOutput struct {
    Abilities []AvailableAbility
    Actions   []AvailableAction
}

type ActivateAbilityInput struct {
    AbilityRef core.Ref // which ability to activate
}

type ActivateAbilityOutput struct {
    Success          bool
    Error            string
    GrantedCapacity  string // "1 attack", "30ft movement", etc.
    Abilities        []AvailableAbility
    Actions          []AvailableAction
}

type ExecuteActionInput struct {
    ActionRef core.Ref    // which action to execute
    TargetID  string      // target entity ID (for strikes)
}

// Note: ExecuteAction returns the updated ability/action lists.
// The actual attack resolution (damage, hit/miss) happens in the API
// orchestrator which calls combat.ResolveAttack with the target entity.
// The Character doesn't need to know about other entities — it only
// manages action economy feasibility. The API handles targeting.

type ExecuteActionOutput struct {
    Success   bool
    Error     string
    Abilities []AvailableAbility
    Actions   []AvailableAction
}

type EndTurnInput struct{}

type EndTurnOutput struct{}
```

### Separation of Concerns: Action Economy vs. Attack Resolution

The Character's action economy methods handle **feasibility** — can this action be taken? What capacity does it consume? What's available after?

The API orchestrator handles **execution** — loading the target entity, calling `combat.ResolveAttack`, applying damage. This keeps the Character from needing to know about other entities in the encounter.

Flow for a Strike:
1. API calls `char.ExecuteAction(ctx, &ExecuteActionInput{ActionRef: refs.Actions.Strike()})` — checks AttacksRemaining, consumes one, checks for post-strike grants (Martial Arts Bonus Strike, Off-Hand Attack)
2. If success, API loads the target monster and calls `combat.ResolveAttack(ctx, attackInput)` — this is existing toolkit code
3. API persists results and sends updated ability/action lists to client

### What AvailableAbilities Returns

Computed from the character's class, features, conditions, equipment, and current action economy state.

**Standard action abilities (everyone gets these):**
- Attack — consumes action, grants AttacksRemaining (count from Extra Attack if applicable)
- Dash — consumes action, doubles remaining movement
- Dodge — consumes action, applies Dodging condition
- Disengage — consumes action, applies Disengaging condition

**Bonus action abilities (class/feature-dependent):**

These are computed by iterating the character's features and checking which ones have an action economy cost. The Character iterates both its standard combat abilities (which every character has) and its features (which are class-specific). Features that implement action economy cost are included in the list. This is the unification mechanism — no new adapter needed. The Character just builds `AvailableAbility` structs from both sources:

- Standard abilities → always present, `CanUse` based on action economy
- Features with action type → included if the character has them, `CanUse` based on action economy + resource availability

Level 1 examples:
- Rage — Barbarian feature, ActionTypeBonus, ResourceCurrent/Max from rage charges
- Second Wind — Fighter feature, ActionTypeBonus, ResourceCurrent/Max from uses
- Off-Hand Attack — any class with two light weapons equipped, ActionTypeBonus (computed from equipment slots, not a feature)

Contextual abilities (appear after specific triggers):
- Martial Arts Bonus Strike — Monk, appears after striking with monk weapon or unarmed, ActionTypeBonus

Level 2+ examples (out of scope for implementation, but the pattern supports them):
- Flurry of Blows — Monk, ActionTypeBonus, ResourceCurrent/Max from ki
- Action Surge — Fighter, special: grants an extra standard action (modeled as ActionTypeBonus that calls `GrantExtraAction()` on the action economy)
- Cunning Action — Rogue, ActionTypeBonus (Dash/Disengage/Hide as bonus)

**Passive conditions do NOT appear here.** Unarmored Defense, Martial Arts die upgrade, Fighting Styles — these are condition badges, not action choices.

### What AvailableActions Returns

After activating an ability that grants capacity:

- Strike — when `AttacksRemaining > 0`
- Off-Hand Strike — when `OffHandAttacksRemaining > 0`
- Flurry Strike — when `FlurryStrikesRemaining > 0`
- Unarmed Strike — when granted by Martial Arts bonus
- Move — when `MovementRemaining > 0` (always available, movement is free)

### Post-Strike Grants

After `ExecuteAction` resolves a Strike, the Character internally checks class rules and updates its ability/action lists. The updated lists are returned in `ExecuteActionOutput`:

- Monk strikes with monk weapon → "Martial Arts Bonus Strike" added to abilities (ActionTypeBonus)
- Character with two light weapons and main-hand strike → "Off-Hand Attack" added to abilities (ActionTypeBonus)

The API does not need to check class rules — it just forwards the `ExecuteActionOutput.Abilities` to the client.

### Dodge and Disengage State

Currently `DodgeActive` and `DisengageActive` are tracked as booleans on the API's `ActionEconomyState`. In the toolkit, these are conditions (Dodging, Disengaging) applied to the character. The spec keeps them as **conditions** (the existing pattern). When Dodge is activated, the character applies the Dodging condition — the action economy doesn't need dedicated fields for them. The condition system already handles turn-end cleanup.

## API Flow

The API's role is reduced to: load character, call methods, save character, forward results.

```
// Turn start
char, _ := character.LoadFromData(ctx, data, bus)
startOutput := char.StartTurn(ctx, &StartTurnInput{Speed: 30})
// save char, send startOutput.Abilities + startOutput.Actions to client

// Player activates an ability (Attack, Dash, Rage, etc.)
abilityOutput, _ := char.ActivateAbility(ctx, &ActivateAbilityInput{
    AbilityRef: ref,
})
// save char, send abilityOutput.Abilities + abilityOutput.Actions

// Player executes an action (Strike — consumes capacity)
actionOutput, _ := char.ExecuteAction(ctx, &ExecuteActionInput{
    ActionRef: refs.Actions.Strike(),
})
// if success, API resolves the attack against the target (combat.ResolveAttack)
// save char, send actionOutput + attack result

// Player ends turn
char.EndTurn(ctx, &EndTurnInput{})
// save char, advance to next entity
```

The orchestrator's `buildAvailableAbilities()` and `buildAvailableActions()` are **deleted**. The `ActivateCombatAbility` switch/case is **replaced** by `char.ActivateAbility(ctx, input)` which routes internally based on the ref.

## Persistence

### ActionEconomyData (serializable)

```go
type GrantedActionKey string

const (
    GrantedAttacks          GrantedActionKey = "attacks"
    GrantedOffHandStrikes   GrantedActionKey = "off_hand_strikes"
    GrantedFlurryStrikes    GrantedActionKey = "flurry_strikes"
    GrantedMartialArtsBonus GrantedActionKey = "martial_arts_bonus"
    GrantedOffHandAttack    GrantedActionKey = "off_hand_attack"
)

type ActionEconomyData struct {
    ActionsRemaining      int                      `json:"actions_remaining"`
    BonusActionsRemaining int                      `json:"bonus_actions_remaining"`
    ReactionsRemaining    int                      `json:"reactions_remaining"`
    MovementRemaining     int                      `json:"movement_remaining"`
    Granted               map[GrantedActionKey]int  `json:"granted,omitempty"`
}
```

Primary resources (actions, bonus actions, reactions, movement) stay as named fields — every character always has them. Granted capacity is a single map with typed keys. Adding a new grant type (e.g., Monk bonus strikes at higher levels) is just a new constant — no struct changes.

```go
// Check, use, grant — one pattern for everything
char.HasGranted(GrantedAttacks)        // Granted["attacks"] > 0
char.UseGranted(GrantedAttacks)        // Granted["attacks"]--
char.Grant(GrantedAttacks, 2)          // Granted["attacks"] = 2
```
```

- Lives on `Character.Data.ActionEconomy` (omitempty — nil outside combat)
- `ToData()` flattens the smart ActionEconomy to this struct
- `LoadFromData()` rehydrates with character reference, can compute `AvailableAbilities()` immediately
- No character reference in the data — just resource state
- `DodgeActive` and `DisengageActive` are NOT here — they are conditions on the character (existing pattern)

### Backward Compatibility

- `LoadFromData` treats missing/nil `ActionEconomy` as "not in combat" — the character works fine without it
- Existing character data without the `ActionEconomy` field loads normally
- Mid-turn encounters restart at turn start (action economy resets), which is acceptable

### Migration

- `ActionEconomyState` moves OFF the encounter entity and INTO the character data
- Action economy resets every turn — no data migration needed
- Stop reading from encounter, start reading from character
- The encounter entity no longer tracks per-character action economy

## Proto Changes

### AvailableAbility — add resource fields

```protobuf
message AvailableAbility {
  CombatAbilityId ability_id = 1;  // KEEP as enum — extend with new values
  string name = 2;
  bool can_use = 3;
  string reason = 4;
  int32 resource_current = 5;  // NEW: e.g., 2 for Rage charges (0 = no resource)
  int32 resource_max = 6;      // NEW: e.g., 2 for Rage charges (0 = no resource)
}
```

### CombatAbilityId — extend enum for features

```protobuf
enum CombatAbilityId {
  // ... existing values ...
  COMBAT_ABILITY_ID_RAGE = 12;
  COMBAT_ABILITY_ID_SECOND_WIND = 13;
  COMBAT_ABILITY_ID_MARTIAL_ARTS_BONUS = 14;
}
```

This keeps the proto pattern (enums for known ability types). The handler maps toolkit `core.Ref` to proto enum at the boundary. New abilities get new enum values — this is the standard protobuf extension pattern.

### Other proto messages — no changes

- `AvailableAction` — no changes
- `ActionEconomy` — remove `disengage_active` and `dodge_active` (now conditions)
- `ActivateCombatAbilityResponse` — no changes
- `ExecuteActionResponse` — no changes

## Web Impact

Minimal — the web already renders `AvailableAbility[]` and `AvailableAction[]` from API responses. Changes:
- Render `resource_current`/`resource_max` on abilities that have them (e.g., "Rage (2/2)")
- Remove the hardcoded fallback ability list (since the API now provides accurate, class-specific lists)
- Handle new `CombatAbilityId` enum values for display (icon, color)

## Scope

### In Scope
- Toolkit: ActionEconomy on Character with `StartTurn`, `AvailableAbilities`, `AvailableActions`, `ActivateAbility`, `ExecuteAction`, `EndTurn`, `ToData`/`LoadFromData`
- Toolkit: Standard abilities (Attack, Dash, Dodge, Disengage) computed for all classes
- Toolkit: Level 1 class-specific abilities: Rage (Barbarian), Second Wind (Fighter), Martial Arts Bonus Strike (Monk), Off-Hand Attack (equipment-based)
- API: Remove `buildAvailableAbilities`, `buildAvailableActions`, simplify `ActivateCombatAbility` to delegate to toolkit
- API: Move action economy from encounter entity to character data
- Proto: Add resource_current/resource_max to AvailableAbility, extend CombatAbilityId enum

### Out of Scope (pattern supports, implementation is follow-up)
- Reaction system
- Help/Hide/Ready abilities
- Level 2+ features (Flurry of Blows, Cunning Action, Action Surge, Patient Defense, Step of the Wind)
- Web component redesign

## Success Criteria

1. A Monk sees "Martial Arts Bonus Strike" (not "Off-Hand Attack") after striking with a monk weapon
2. A Barbarian sees "Rage (2/2)" in their bonus action abilities
3. A Fighter sees "Second Wind (1/1)" in their bonus action abilities
4. A Fighter with two light weapons sees "Off-Hand Attack" in bonus actions
5. The API orchestrator has zero hardcoded ability lists
6. `buildAvailableAbilities` and `buildAvailableActions` are deleted from the API
7. Action economy persists on the character, not the encounter
8. All methods use Input/Output types
