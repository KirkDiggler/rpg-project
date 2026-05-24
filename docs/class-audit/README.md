# D&D 5e Dungeon Crawler — Class Audit

> **Date:** 2026-02-07  
> **Scope:** Four implemented player classes (Barbarian, Fighter, Monk, Rogue)  
> **Repos audited:** rpg-toolkit, rpg-api, rpg-dnd5e-web, rpg-api-protos

## Architecture Boundary Rule

```
Client sends REFERENCES → API orchestrates by KEY → Toolkit implements RULES
```

- **rpg-dnd5e-web** (React) — Sends proto enum references (e.g., `FEATURE_ID_RAGE`)
- **rpg-api** (Go/gRPC) — Maps keys to toolkit calls, owns persistence/streaming
- **rpg-toolkit** (Go) — Implements all D&D 5e rules via event-driven condition/feature system
- **rpg-api-protos** — Protobuf contracts; defines enums for classes, features, conditions

## The Four Implemented Classes

| Class | Hit Die | Primary Ability | Key Mechanic | Grants System |
|-------|---------|----------------|--------------|---------------|
| **Barbarian** | d12 | STR | Rage + Unarmored Defense | ✅ `getBarbarianGrants()` |
| **Fighter** | d10 | STR | Fighting Styles + Second Wind | ✅ `getFighterGrants()` |
| **Monk** | d8 | DEX | Martial Arts + Ki | ✅ `getMonkGrants()` |
| **Rogue** | d8 | DEX | Sneak Attack + Expertise | ✅ `getRogueGrants()` |

All other classes (Bard, Cleric, Druid, Paladin, Ranger, Sorcerer, Warlock, Wizard) have `ClassData` entries and proto enums but **no grants, no feature implementations, and no integration tests**.

## Feature Matrix — Test Coverage

### Legend
- ✅ = Tested with real assertions
- 🔸 = Partially tested (Ki consumption only, mock-based)
- ❌ = Not tested
- ➖ = N/A

| Feature | Toolkit Unit | Toolkit Integration | API Integration | UI Tests |
|---------|:---:|:---:|:---:|:---:|
| **BARBARIAN** | | | | |
| Rage activation | ✅ | ✅ (6 subtests) | ✅ (activate via gRPC) | ❌ |
| Rage +2 damage | ✅ | ✅ | ✅ (breakdown) | ❌ |
| Rage B/P/S resistance | ✅ | ✅ | ✅ (resistance test) | ❌ |
| Rage ends (no combat) | ✅ | ✅ | ✅ (turn end test) | ❌ |
| Rage ends (10 rounds) | ✅ | ✅ | ❌ | ❌ |
| Rage resource tracking | ✅ | ✅ | ✅ (implicit) | ❌ |
| Unarmored Defense (CON) | ✅ | ✅ | ✅ (AC calculation) | ❌ |
| Brutal Critical | ✅ (unit) | ❌ | ❌ | ❌ |
| **FIGHTER** | | | | |
| Second Wind healing | ✅ | ✅ (4 subtests) | ❌ | ❌ |
| Second Wind once/rest | ✅ | ✅ | ❌ | ❌ |
| Second Wind level scaling | ✅ | ✅ | ❌ | ❌ |
| Fighting Style: Defense | ✅ | ✅ (2 subtests) | ❌ | ❌ |
| Fighting Style: Dueling | ✅ | ✅ (3 subtests) | ❌ | ❌ |
| Fighting Style: Archery | ✅ | ✅ (2 subtests) | ❌ | ❌ |
| Fighting Style: GWF | ✅ | ✅ (2 subtests) | ❌ | ❌ |
| Fighting Style: TWF | ✅ | ✅ (2 subtests) | ❌ | ❌ |
| Fighting Style: Protection | ✅ | ✅ (1 subtest) | ❌ | ❌ |
| Improved Critical | ✅ (unit) | ❌ | ❌ | ❌ |
| Action Surge | ✅ (unit) | ❌ | ❌ | ❌ |
| **MONK** | | | | |
| Martial Arts DEX swap | ✅ | ✅ (3 subtests) | ❌ | ❌ |
| Martial Arts 1d4 scaling | ✅ | ✅ | ❌ | ❌ |
| Unarmored Defense (WIS) | 🔸 (mock AC) | 🔸 | ✅ (AC test) | ❌ |
| Ki resource | 🔸 | 🔸 (manual) | ❌ | ❌ |
| Flurry of Blows | 🔸 | 🔸 (Ki only) | ❌ | ❌ |
| Patient Defense | 🔸 | 🔸 (Ki only) | ❌ | ❌ |
| Step of the Wind | 🔸 | 🔸 (Ki only) | ❌ | ❌ |
| Deflect Missiles | ✅ (unit) | ❌ | ❌ | ❌ |
| Unarmored Movement | ✅ (unit) | ❌ | ❌ | ❌ |
| **ROGUE** | | | | |
| Sneak Attack w/ advantage | ✅ | ✅ | ❌ | ❌ |
| Sneak Attack w/ ally | ✅ | ✅ | ❌ | ❌ |
| Sneak Attack denied | ✅ | ✅ | ❌ | ❌ |
| Sneak Attack once/turn | ✅ | ✅ | ❌ | ❌ |
| Sneak Attack reset | ✅ | ✅ | ❌ | ❌ |
| Sneak Attack finesse req | ✅ | ✅ | ❌ | ❌ |
| Sneak Attack level scaling | ✅ | ✅ | ❌ | ❌ |
| Expertise | ❌ | ❌ | ❌ | ❌ |

### Test Count Summary

| Layer | Files | Test Functions/Subtests |
|-------|-------|----------------------|
| **Toolkit — conditions** (unit) | 17 test files | ~150 test funcs |
| **Toolkit — features** (unit) | 8 test files | ~78 test funcs |
| **Toolkit — integration** | 4 class suites | ~78 subtests |
| **Toolkit — combat** (unit) | 12 test files | ~60+ test funcs |
| **API — integration** | 4 encounter test files | ~27 test funcs |
| **API — handler/orchestrator** | 15+ test files | ~50+ test funcs |
| **UI — src tests** | 3 test files | ~30 tests (hex grid, dungeon map) |
| **UI — feature/combat** | 0 test files | 0 tests |

## Key Files

- [CLASSES.md](./CLASSES.md) — Per-class breakdown of features, abilities, and mechanics
- [TEST-COVERAGE.md](./TEST-COVERAGE.md) — Detailed test coverage analysis with file paths
- [RECOMMENDATIONS.md](./RECOMMENDATIONS.md) — Action items for improving coverage and docs

## Design Philosophy: Why This Architecture?

### Conditions vs Features

The toolkit distinguishes between two types of class mechanics:

**Conditions** (passive, always-on event subscribers):
- Hook into the event bus and modify attack/damage/AC chains
- Examples: Raging (+2 damage, B/P/S resistance), Martial Arts (DEX swap), Sneak Attack (extra dice), all Fighting Styles
- Loaded from character data, applied via `condition.Apply(ctx, bus)`
- Persist as long as they're active

**Features** (activatable abilities):
- Require player input to trigger
- Consume resources (rage charges, Ki, Second Wind uses)
- Examples: Rage (activation), Second Wind, Flurry of Blows, Action Surge
- Loaded from character data, activated via `feature.Activate(ctx, owner, input)`

This split maps cleanly to the UI: Conditions show as status badges, Features show as action buttons.

### The Grant System

Classes declare what they give at each level via `GetGrants()`:
```go
Grant{
    Level: 1,
    Features:   []FeatureRef{{Ref: "dnd5e:features:rage"}},
    Conditions: []ConditionRef{{Ref: "dnd5e:conditions:unarmored_defense"}},
    ...
}
```

The API layer reads these refs and instantiates toolkit implementations. The UI never sees implementation details — only keys and display data.
