# Toolkit Cleanup Notes for Unified Entity State

**Date:** 2026-03-28
**Context:** During the toolkit review for the unified-entity-state design, several areas were identified where the toolkit could be improved to better support the Load → Execute → ApplyDamage → Harvest → Save pattern the API needs.

## 1. ApplyDamage should be bus-driven, not caller-invoked

**Current:** `combat.ResolveAttack()` publishes `DamageReceivedEvent` and chain events, but does NOT call `ApplyDamage()` on the target. The caller (rpg-api orchestrator) is responsible for calling `target.ApplyDamage()` after resolution.

**Problem:** This forces the API to understand damage application — a rules concern. The API has to extract damage from the result and call `ApplyDamage()` explicitly. This is how the `CharacterHP` manual subtraction shortcut happened in the first place.

**Ideal:** `ResolveAttack()` (or the event bus flow) should call `ApplyDamage()` on the target as part of resolution. The API fires the action and harvests dirty objects — it never touches HP directly. Entities subscribe to `DamageReceivedEvent` and apply damage to themselves.

**Investigation:** Check if Character and Monster already subscribe to `DamageReceivedEvent` in their `subscribeToEvents()`. If not, adding that subscription would close the loop. If they do, figure out why the API still calls `ApplyDamage()` separately.

## 2. Unconscious condition at 0 HP — is it automatic?

**Current:** Unknown whether dropping to 0 HP automatically applies the Unconscious condition through the bus, or if the API/caller must handle this.

**Why it matters:** The unified EntityState design uses conditions exclusively (no `is_dead`/`is_unconscious` booleans). If the toolkit doesn't auto-apply Unconscious at 0 HP, the API would need to do it — violating the boundary rule.

**Investigation:**
- Check if `ApplyDamage()` or `DamageReceivedEvent` handling triggers Unconscious condition when HP hits 0
- Check if there's a death/unconscious listener on the bus
- If not, this needs to be added: when an entity's HP drops to 0, the toolkit should apply the Unconscious condition (for characters) or Dead condition (for monsters, since monsters die at 0 HP in 5e)

**Spike test to run:**
```go
// Load character + monster on same bus
// Resolve attack, apply damage that drops character to 0 HP
// Check character.ToData().Conditions for Unconscious
// Check monster.ToData().Conditions for Dead (if monster at 0 HP)
```

## 3. Monster LoadFromData is multi-step

**Current:** `monster.LoadFromData()` creates the monster but actions and conditions must be loaded separately via `LoadMonsterActions()` and `LoadMonsterConditions()` due to import cycle avoidance.

**Problem:** Easy to forget a step. The API orchestrator has to know about this internal toolkit constraint.

**Possible improvement:** A single `monster.FullLoadFromData(data, bus)` function that handles all three steps, living in a package that can import both monster and the action/condition loaders. Or a builder pattern that makes the multi-step nature explicit and hard to get wrong.

**Priority:** Low — this is ergonomic, not correctness. The API can work with the multi-step pattern.

## 4. Condition JSON serialization boundary

**Current:** Conditions persist as `[]json.RawMessage` in both `character.Data.Conditions` and `monster.Data.Conditions`. Each condition type implements `ToJSON()` and has a loader that peeks at the `Ref` field to route deserialization.

**Not a bug**, but the API's `toEntityState()` conversion will need to deserialize this JSON to project conditions into proto `Condition` messages. Consider whether the toolkit should provide a helper:

```go
// Something like:
func ConditionSummary(raw json.RawMessage) (*ConditionInfo, error)
// Returns: ID, Name, Source, Duration, IsActive — without full deserialization
```

This would let the API project conditions into protos without needing to import every condition type's internal structure.

**Priority:** Medium — without this, the API's conversion function needs to know how to read every condition type's JSON format, which couples it to toolkit internals.

## 5. ActionEconomy nil-outside-combat pattern

**Current:** `character.Data.ActionEconomy` is `*ActionEconomyData` and is nil when not in combat (omitempty in JSON).

**Not a problem**, just noting: the API currently tracks action economy in its own `ActionEconomyState` on `EncounterData`. The unified design moves this to `CombatState.current_turn_economy` as a proto projection. The toolkit's per-character tracking in Data is preserved in the stored `ToolkitData` — so `StartTurn()` / `EndTurn()` on the character object still work correctly through the Load/Save cycle.

No toolkit changes needed here, just awareness for the API refactor.

## Summary

| Item | Type | Priority | Blocks unified-entity-state? |
|------|------|----------|------------------------------|
| ApplyDamage bus-driven | Enhancement | High | Yes — without it, API still does damage application |
| Unconscious at 0 HP | Investigation + possible fix | High | Yes — conditions-only design depends on it |
| Monster FullLoadFromData | Ergonomic | Low | No — multi-step works |
| ConditionSummary helper | New function | Medium | No — API can parse JSON, just ugly |
| ActionEconomy nil pattern | Awareness | None | No changes needed |

Items 1 and 2 should be investigated first via the spike test defined in `design.md`. If they work already, no toolkit changes needed. If not, they become step 0 in the migration.
