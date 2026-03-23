# Platform Audit Synthesis — 2026-03-22

## Audit Team

| Specialist | Domain | Report |
|---|---|---|
| Engine Architect | rpg-toolkit (rules engine) | [findings/engine-architect.md](findings/engine-architect.md) |
| Server Architect | rpg-api (game server) | [findings/server-architect.md](findings/server-architect.md) |
| Client Architect | rpg-dnd5e-web (React UI) | [findings/client-architect.md](findings/client-architect.md) |
| Systems Integrator | rpg-api-protos + boundaries | [findings/systems-integrator.md](findings/systems-integrator.md) |

---

## The #1 Problem All Specialists Found Independently

**The API is doing the toolkit's job.** 75+ switch statements on feature/condition IDs in the encounter orchestrator. Every specialist flagged this from their angle:

- **Engine Architect**: "TurnManager exists and works, API doesn't use it"
- **Server Architect**: "Action economy switch statement hardcoded to proto ActionId enums"
- **Client Architect**: "Falls back to hardcoded list if API doesn't provide availability"
- **Systems Integrator**: "Boundary compliance score: 2/10 — 75+ switches on game enums"

This is the single change that unblocks the most future work. Every new class feature currently requires API code changes, which defeats the entire boundary architecture.

---

## Cross-Cutting Themes

### Theme 1: TurnManager Is the Answer Nobody's Using

Toolkit built TurnManager with `GetAvailableAbilities()`, `GetAvailableActions()`, `UseAbility()`, `ExecuteAction()`. It's the clean boundary the architecture promises. But the API bypasses it entirely, hardcoding feature side effects in `applyFeatureSideEffects()` and action costs in `getFeatureActionCost()`.

**Fix this and you fix**: boundary violations, hardcoded ability lists, client fallback hacks, and the "add a class = change 3 repos" problem.

### Theme 2: Condition Persistence Is Broken End-to-End

- **Engine**: Conditions serialize to JSON but don't re-wire subscriptions on reload
- **Server**: CharacterHP stored in two places (encounter state AND character repo) — silent inconsistency
- **Systems**: Reconnection has no documented state recovery story

A player disconnecting mid-rage and rejoining would lose the damage bonus. This will bite during multi-room dungeons.

### Theme 3: Missing Proto Contracts Block the Next Features

All four specialists confirmed: **death saves, rest system, and reactions have no proto support**.

| Feature | Toolkit Ready? | Proto Ready? | API Ready? | Web Ready? |
|---------|---------------|-------------|-----------|-----------|
| Death Saves | Condition pattern exists | No messages/events | No RPCs | No UI |
| Rest System | RestTopic + RecoverableResource | No RPCs/events | No orchestrator | No UI |
| Equipment Enrichment | Needs ResolveEquipmentDetail() | Field added | Mapping ready | Needs component |
| Reactions | No event model | reaction_available field only | Nothing | Nothing |

### Theme 4: The Orchestrator Is a God Object

4,946 lines, 43 methods. Combat, dungeon generation, lobby management, door transitions, event publishing — all in one file. Server Architect recommends splitting into Combat/Dungeon/Lobby/CharacterAction orchestrators.

---

## If We Could Only Do 3 Things

### 1. Wire TurnManager Through API (3-5 days)

- Remove `applyFeatureSideEffects()` and `getFeatureActionCost()` from API
- API calls `tm.UseAbility()` / `tm.ExecuteAction()` and reads the result
- Return `AvailableAbilities`/`AvailableActions` in all combat responses
- **Unblocks**: Adding new classes without API changes, correct client rendering

### 2. Fix Condition Persistence + State Consistency (2 days)

- Complete condition loader (factory from JSON -> re-subscribe to bus)
- Remove CharacterHP from EncounterData (load from character repo)
- Add round-trip test: serialize -> deserialize -> verify subscriptions
- **Unblocks**: Save/load, reconnection, multi-room dungeons

### 3. Add Death Saves + Rest Proto Contracts (1 day protos, 2 days implementation)

- Add unconscious condition, death save messages, rest RPCs to protos
- **Unblocks**: Multi-room dungeons where characters can die and heal

---

## Sequencing: What Unblocks What

```
[1] TurnManager Integration ──> [2] Condition Persistence Fix
         |                              |
         |──> New class features        |──> Reconnection reliability
         |    (no API changes)          |    (state preserved)
         |                              |
         v                              v
[3] Death Saves + Rest Protos ──> [4] Multi-room dungeon feature completeness
                                         |
                                         v
                                  [5] Reaction System (future)
```

---

## What's Well-Designed and Should Be Preserved

- **Event bus + staged chain architecture** — elegant, extensible, type-safe
- **Load -> Execute -> Persist pattern** in API — consistent across all actions
- **Converter organization** — one place per handler, no information loss
- **Proto Available Ability/Action messages** — already designed with `can_use` + `reason`
- **5-stage modifier pipeline** — explicit ordering prevents subtle bugs

---

## Evolution Paths

**Near-term** (enables 4-class dungeon): TurnManager integration + condition persistence + rest/death protos

**Mid-term** (enables casters): Spell concentration (mutual exclusion on character), spell slot resource tracking, ACChain completion

**Long-term** (enables full D&D combat): Reaction event model (interrupt-style events in the bus), multiclass support (multi-source grants, shared resource pools)

---

## All Tech Debt Items

### Critical (Blocks Evolution)

| Item | Repo | Source | Severity |
|------|------|--------|----------|
| 75+ switch statements on game enums in API | rpg-api | Systems Integrator, Server Architect | CRITICAL |
| Condition persistence broken on reload | rpg-toolkit | Engine Architect | CRITICAL |
| CharacterHP stored in two places | rpg-api | Server Architect | CRITICAL |
| `interface{}` RoomData type | rpg-api | Server Architect | CRITICAL |
| No death save proto support | rpg-api-protos | Systems Integrator | CRITICAL |
| No rest system proto support | rpg-api-protos | Systems Integrator | CRITICAL |

### High (Slows Development)

| Item | Repo | Source | Severity |
|------|------|--------|----------|
| TurnManager not exposed via API | rpg-api | Engine Architect, Server Architect | HIGH |
| GetAbilityInfos not class-aware | rpg-toolkit | Engine Architect | HIGH |
| ACChain partially implemented | rpg-toolkit | Engine Architect | HIGH |
| No reaction event model | rpg-toolkit | Engine Architect | HIGH |
| Event publishing on connection loss not wired | rpg-api | Server Architect | HIGH |
| Encounter orchestrator 4,946 lines | rpg-api | Server Architect | HIGH |
| Client hardcoded ability fallbacks | rpg-dnd5e-web | Client Architect | HIGH |
| Flaky tests from non-deterministic dice | rpg-api | Server Architect | HIGH |

### Medium (Cosmetic/Maintainability)

| Item | Repo | Source | Severity |
|------|------|--------|----------|
| Two context types with confusing names | rpg-toolkit | Engine Architect | MEDIUM |
| DamageChainEvent vs AttackChainEvent pointer inconsistency | rpg-toolkit | Engine Architect | MEDIUM |
| Initiative roll data never pruned | rpg-api | Server Architect | MEDIUM |
| Converter TODOs (missing fields in responses) | rpg-api | Server Architect | MEDIUM |
| Stage ordering within stages undefined | rpg-toolkit | Engine Architect | MEDIUM |
