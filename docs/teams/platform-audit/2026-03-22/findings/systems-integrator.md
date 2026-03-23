# Systems Integrator Audit — Boundaries & Protos

**Date**: 2026-03-22
**Domain**: rpg-api-protos contracts, cross-layer boundaries, data flow
**Auditor**: Systems Integrator specialist

---

## 1. Contract Gaps — Proto vs Toolkit Reality

### Gap 1.1: Equipment Enrichment (READY)
Proto field added (`EquipmentItem.equipment_detail`), toolkit hasn't implemented `ResolveEquipmentDetail()` yet.

### Gap 1.2: Available Actions Query (PARTIALLY DONE)
Proto messages `AvailableAbility` and `AvailableAction` defined. Included in `ActivateCombatAbilityResponse` and `ExecuteActionResponse`. No dedicated query RPC. Web must derive availability from `ActionEconomy` fields.
**Issues:** rpg-api-protos#129, rpg-api#404

### Gap 1.3: Death Saves System (MISSING)
No proto support. Design complete. Missing: `CONDITION_ID_UNCONSCIOUS` enum, death save tracking, `CharacterDiedEvent`/`CharacterStabilizedEvent`, death save roll RPC.
**Ref:** rpg-project/ideas/death-saves/design.md, issue #296

### Gap 1.4: Rest System (MISSING)
No proto support. Design complete. Toolkit has `RestTopic` and `RecoverableResource`. Missing: `ShortRest`/`LongRest` RPC, `RestEvent`.
**Ref:** rpg-project/ideas/rest-system/design.md, issue #294

### Gap 1.5: Reaction System (PARTIALLY DESIGNED)
`ActionEconomy.reaction_available` field exists. No interaction flow. No RPC for reaction windows.

---

## 2. Critical Boundary Violations

### Violation 2.1: Feature-Specific Logic in API Orchestrators — CRITICAL

**75+ switch statements** on feature/condition/ability refs in API layer:

```go
// getFeatureActionCost() — API decides action costs
// applyFeatureSideEffects() — API grants strikes for Flurry, Dodge for Patient Defense
```

**Why this violates boundaries:**
- API making game decisions (granting strikes, dodging)
- Rules should live in toolkit
- New class features require API code changes
- Duplicates logic toolkit already does in `Activate()`

**Count:** 75 individual switch cases across orchestrators
**Severity:** Will break when adding any new feature behavior

### Violation 2.2: API Building Availability Lists Instead of Querying Toolkit
From action system audit: `buildAvailableAbilities()` — HARDCODED list of 5 abilities, same for all classes. Toolkit has `TurnManager.GetAvailableAbilities()` but API doesn't call it.

---

## 3. Data Flow Analysis

### Flow 3.1: Combat Action — Mostly Working
Single player gets RPC response. Events broadcast to all players via StreamEncounterEvents.
**Gap:** No dedicated event type for ability activation.

### Flow 3.2: Movement — Dual RPC Confusion
Both deprecated `MoveCharacter` and new `ExecuteAction(Move)` exist.

### Flow 3.3: Reconnection — INCOMPLETE
No event log replay. No death save state recovery. `GetCombatState` RPC exists but doesn't cover all state.

---

## 4. Boundary Rule Compliance

- **API Switch Statements:** VIOLATION — 75+ cases on game-specific enums
- **Client Interpreting Game Data:** OK — render-only (falls back to hardcoded list but that's API's fault)
- **Proto Message Shapes:** Mostly OK — well-designed `AvailableAbility`/`AvailableAction` with `can_use` + `reason`. Gaps for upcoming features.

---

## 5. Upcoming Feature Stress Test

### Death Saves — NOT READY
Needs: `CONDITION_ID_UNCONSCIOUS`, `DeathSaveProgress` message, `DeathSaveRolledEvent`/`CharacterDiedEvent`/`CharacterStabilizedEvent`. Toolkit auto-rolls on TurnStartEvent per design. Straightforward addition.

### Rest System — NOT READY
Needs: `ShortRest`/`LongRest` RPCs, `RestCompletedEvent`. Toolkit already has `RestTopic` and `RecoverableResource`. Web shows "Rest Available" button between rooms.

### Equipment Enrichment — READY (proto field exists)
Needs: Toolkit `ResolveEquipmentDetail()`, API mapping, Web `EquipmentCard` component.

### Reactions — NEEDS DESIGN
Needs: Event types for reaction windows (`AttackDeclaredEvent`, `AttackRolledEvent`, `MovementStepEvent`), `ReactWithFeature` RPC, async multiplayer messaging. High effort.

---

## 6. Breaking Changes Needed

1. **Remove Feature Logic from API** (2-3 days) — Move to toolkit, API reads result only
2. **Add Available Actions to All Responses** (1 day) — Additive, won't break web
3. **Support Rest and Death Saves** (1 week) — Proto + API + web

---

## 7. Recommendations — Ordered by Impact

1. **Stop API from Switching on Feature IDs** (CRITICAL, 2-3 days) — Unblocks adding new class features without API changes
2. **Expose TurnManager via RPC** (1 day) — Web renders correct actions per class
3. **Implement Equipment Enrichment End-to-End** (3 days) — Meaningful equipment choices at creation
4. **Prepare Proto for Death Saves and Rest** (1 day protos + 2 days impl) — Multi-room dungeon realism
5. **Design Reaction System** (2 weeks, defer) — Shield, Opportunity Attacks, Deflect Missiles

---

## Contract Health Scorecard

| Category | Score | Notes |
|----------|-------|-------|
| Service Definition | 7/10 | Core RPCs defined, gaps in rest/death/reactions |
| Message Richness | 6/10 | Breakdowns present, missing death saves and reaction data |
| Enum Coverage | 5/10 | Features/conditions defined, missing Unconscious, RestType |
| Event System | 7/10 | Works for combat, needs rest/death/reaction events |
| Boundary Compliance | 2/10 | CRITICAL: 75+ switches on game enums in API |
| Field Completeness | 7/10 | Most fields present, gaps for upcoming features |

**Overall:** Functionally sufficient for basic combat, but serious boundary violations and gaps for upcoming features.
