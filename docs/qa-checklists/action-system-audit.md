# Action System Audit (2026-03-21)

Notes from playtest weekend audit of the action economy system across all layers.

## Architecture: Two-Level System

```
Level 1: Abilities (consume primary resources)
  Attack → consumes action → grants AttacksRemaining
  Dash → consumes action → grants extra movement
  Dodge → consumes action → sets DodgeActive
  Disengage → consumes action → sets DisengageActive
  Off-Hand Attack → consumes bonus action → grants OffHandAttacksRemaining

Level 2: Actions (consume granted capacity)
  Strike → consumes AttacksRemaining
  Off-Hand Strike → consumes OffHandAttacksRemaining
  Flurry Strike → consumes FlurryStrikesRemaining
  Unarmed Strike → consumes bonus action (Martial Arts)
  Move → consumes MovementRemaining
```

## Current State by Layer

### Toolkit (source of truth for rules)
- `ActionEconomy` struct in `combat/action_economy.go`
- CombatAbility refs in `refs/combat_abilities.go` (Attack, Dash, Dodge, Disengage, Help, Hide, Ready)
- Action refs in `refs/actions.go` (Move, Strike, OffHandStrike, FlurryStrike, UnarmedStrike)
- Features implement `core.Action[T]` interface (Rage, Second Wind, Flurry of Blows)
- **Gap:** No "what can this character do" query — toolkit has the pieces but no orchestrator for available abilities

### API (data orchestrator)
- `ActionEconomyState` in `entities/encounter.go` — mirrors toolkit struct
- `buildAvailableAbilities()` — **HARDCODED list of 5 abilities, same for all classes**
- `buildAvailableActions()` — **HARDCODED list of 4 actions, same for all classes**
- `ActivateCombatAbility()` — switch/case on CombatAbilityId enum
- `ExecuteAction()` — switch/case on ActionId enum
- **Gap:** No class awareness. No feature-to-ability bridge.

### Protos (contracts)
- `CombatAbilityId` enum: ATTACK(1), DASH(2), DODGE(3), DISENGAGE(4), HELP(5), HIDE(6), READY(7), OFFHAND_ATTACK(10), FLURRY_OF_BLOWS(11)
- `ActionId` enum: MOVE(1), STRIKE(2), OFF_HAND_STRIKE(3), FLURRY_STRIKE(4), UNARMED_STRIKE(5)
- `AvailableAbility` message: ability_id, name, can_use, reason
- `AvailableAction` message: action_id, name, can_use, reason
- Both returned in ActivateCombatAbilityResponse and ExecuteActionResponse

### Web (renders what API tells it)
- `CombatAbilitiesPanel.tsx` — renders available abilities as buttons grouped by action type
- Falls back to hardcoded list if API doesn't provide availability
- Calls `ActivateCombatAbility` RPC on button click
- Calls `ExecuteAction` RPC for strikes/movement

## Key Gaps

1. **buildAvailableAbilities is hardcoded** — same 5 abilities for every class
2. **No class filtering** — Monks see Off-Hand Attack, Fighters don't see Martial Arts
3. **Features not integrated** — Rage/Second Wind are in a separate panel, not in the action economy flow
4. **FLURRY_OF_BLOWS ability** — proto defined but not in orchestrator switch
5. **HELP, HIDE, READY** — proto defined but not implemented
6. **Extra Attack** — hardcoded to 1, not class/level-dependent
7. **No reaction system** — reactions exist in action economy but no RPC for them

## Design Direction

The toolkit should own "what can this character do on their turn" — the API just passes the character and gets back the ability list. The action economy should be integrated with class data so abilities are dynamic per character.
