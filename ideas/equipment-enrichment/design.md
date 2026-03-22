# Equipment Data Enrichment

**Date:** 2026-03-21
**Status:** Draft
**Scope:** Flow weapon/equipment stats from toolkit through the pipeline so character creation shows meaningful equipment details

## Problem

When choosing starting equipment during character creation, players see item names ("Shortsword", "Chain mail") without any stats. A player has no way to compare a shortsword vs a handaxe without external knowledge. The toolkit has all this data (damage dice, damage type, properties, weight, category) and the protos have messages to carry it (`Equipment`, `WeaponData`, `ArmorData`) — but the equipment *choice* types don't include it.

This applies to both concrete items in equipment bundles AND the category-based dropdowns (e.g., "choose a martial weapon") where comparing options is the whole point.

## Goals

1. Enrich equipment choices with full weapon/armor stats at the toolkit level
2. Flow that data through protos and API to the UI
3. Build a reusable `EquipmentCard` component in the UI that handles weapons, armor, and gear
4. Start with character creation, designed so the same component works in inventory/combat/tooltips later

## Non-Goals

- No new weapon or armor items
- No changes to how equipment choices work mechanically (same choose-from-bundles pattern)
- No inventory or combat UI changes in this pass
- No class-specific guidance text (that was the concepts prototype scope)

## The Boundary Rule

```
Toolkit: resolves equipment IDs to full stats → returns enriched equipment choices
API:     passes through enriched data → no equipment knowledge needed
UI:      renders what it receives → reusable EquipmentCard component
```

## Design

### Layer 1: Toolkit — Enrich EquipmentItem

**Scope:** Both `EquipmentItem` types need enrichment:
- `character/choices/requirements.go` — items in equipment choice bundles
- `classes/grants.go` — items granted automatically (not chosen)

Both reference equipment by ID. Both should carry resolved stats so the UI can display them.

**New types in `shared/` package** (reusable across choices and grants):

```go
type EquipmentDetail struct {
    Name     string         `json:"name"`
    Type     EquipmentType  `json:"type"`     // weapon, armor, tool, pack, ammunition
    Weight   float64        `json:"weight"`
    Cost     string         `json:"cost"`     // "25 gp" format

    // Only one of these is set, based on Type
    Weapon   *WeaponDetail  `json:"weapon,omitempty"`
    Armor    *ArmorDetail   `json:"armor,omitempty"`
    // Tools, packs, and ammunition get base fields only (name, weight, cost).
    // They don't have type-specific stats that affect gameplay decisions.
}

type WeaponDetail struct {
    Category    string   `json:"category"`    // "simple-melee", "martial-ranged", etc.
    Damage      string   `json:"damage"`      // "1d8"
    DamageType  string   `json:"damage_type"` // "slashing", "piercing", "bludgeoning"
    Properties  []string `json:"properties"`  // ["finesse", "light", "versatile"]
    NormalRange int      `json:"normal_range,omitempty"` // feet, for thrown/ranged
    LongRange   int      `json:"long_range,omitempty"`   // feet, for thrown/ranged
}

type ArmorDetail struct {
    Category            string `json:"category"`              // "light", "medium", "heavy", "shield"
    BaseAC              int    `json:"base_ac"`
    DexBonus            bool   `json:"dex_bonus"`
    MaxDexBonus         *int   `json:"max_dex_bonus,omitempty"`
    StrengthRequirement int    `json:"strength_requirement,omitempty"`
    StealthDisadvantage bool   `json:"stealth_disadvantage"`
}
```

**Note on string types:** `WeaponDetail` and `ArmorDetail` use plain strings for category, damage type, and properties instead of the typed constants from the `weapons`/`armor` packages. This avoids import cycles since `shared` is imported by those packages, not the other way around. The string values match the existing constants (e.g., `weapons.CategorySimpleMelee` = `"simple-melee"`).

**Resolver function** in `shared/`:

```go
// ResolveEquipmentDetail looks up an equipment ID across weapon, armor, tool,
// pack, and ammunition registries and returns the populated detail.
// Returns nil if the ID is not found in any registry.
func ResolveEquipmentDetail(id EquipmentID) *EquipmentDetail
```

**Where enrichment happens:** After building the equipment requirements/grants, call `ResolveEquipmentDetail` for each item. This happens once when building class data — the results are part of the static class definition, not computed per API request.

**Tools, packs, and ammunition:** These get `EquipmentDetail` with `Name`, `Type`, `Weight`, and `Cost` populated, but no type-specific sub-struct. A dungeoneer's pack doesn't have stats to compare the way weapons do — the name and type are sufficient for display.

### Layer 2: Protos — Add Equipment to EquipmentItem

The proto `EquipmentItem` message in `choices.proto` currently has:

```protobuf
message EquipmentItem {
    string selection_id = 1;
    int32 quantity = 2;
    oneof type_hint { ... }
}
```

**Proposed:** Add an optional `Equipment` field:

```protobuf
message EquipmentItem {
    string selection_id = 1;
    int32 quantity = 2;
    oneof type_hint {
        Weapon weapon = 3;
        Armor armor = 4;
        Tool tool = 5;
        Pack pack = 6;
        Ammunition ammunition = 7;
    }
    Equipment equipment_detail = 8;  // Resolved equipment stats (optional)
}
```

The existing `Equipment` message in `character.proto` already has `WeaponData`, `ArmorData`, and `GearData` with all the fields we need. No new proto messages required — we reuse what exists.

**Field mapping notes** (toolkit → proto):
- `WeaponDetail.NormalRange`/`LongRange` → `WeaponData.normal_range`/`long_range` + `range` set to `"melee"` or `"ranged"` based on whether range values are present
- `EquipmentDetail.Cost` string → `Cost` message: parse "25 gp" into `{quantity: 25, unit: "gp"}`
- `EquipmentDetail.Weight` float64 → `Weight` message: `{quantity: int(weight), unit: "lb"}`

These conversions live in the API's choice mapping layer, which already handles toolkit→proto type conversion.

### Layer 3: API — Pass Through

The API handler that maps toolkit choice types to proto messages needs to map the new `Detail` field to the `equipment_detail` proto field. The conversion logic for `Cost` and `Weight` is non-trivial but mechanical — the API gains no new knowledge about what weapons are good or what stats mean.

### Layer 4: UI — Reusable EquipmentCard Component

Create an `EquipmentCard` component in `src/components/` that takes `Equipment` proto data and renders contextually based on type:

**For weapons:**
- Name prominently
- Damage — dice + type (e.g., "1d6 slashing")
- Properties — as tags (Finesse, Light, Versatile, etc.)
- Category — simple/martial, melee/ranged
- Range — for thrown/ranged weapons
- Weight — in lbs

**For armor:**
- Name prominently
- AC — base value with dex bonus note
- Category — light/medium/heavy/shield
- Stealth disadvantage flag
- Strength requirement (if any)
- Weight — in lbs

**For tools/packs/ammunition:**
- Name and type
- Weight

The component accepts the proto `Equipment` type directly so it works anywhere the proto is available (character creation, inventory, combat tooltips).

**Usage in equipment choices:** The `EquipmentChoiceSelector` component checks if `equipment_detail` is present on each `EquipmentItem`. If present, render with `EquipmentCard`. If not (backward compat), fall back to item name string.

**Category choice dropdowns:** When a player picks from a category (e.g., "choose a martial weapon"), the dropdown options should also show `EquipmentCard` for each weapon. This requires the API to resolve available weapons for each category and include their stats. This is already partially handled — the UI calls a weapon list endpoint for category choices — the data just needs the `Equipment` detail attached.

## Repos Affected

| Repo | Changes |
|------|---------|
| **rpg-toolkit** | Add `EquipmentDetail` types to `shared/`, `ResolveEquipmentDetail()` helper, call it in equipment requirement builders and grants |
| **rpg-api-protos** | Add `equipment_detail` field to `EquipmentItem` message |
| **rpg-api** | Map toolkit `Detail` to proto `equipment_detail` in choice handler, including Cost/Weight conversion |
| **rpg-dnd5e-web** | Create `EquipmentCard` component, use in equipment choice selector and category dropdowns |

## Prerequisite: Package Reorganization

**Before implementing enrichment, the toolkit needs a package restructure.**

The current layout has `weapons/`, `armor/`, and shared types in `shared/`. This creates import cycles when trying to build `EquipmentDetail` — `shared` can't import `weapons`/`armor` types, and using strings to avoid cycles is a code smell.

**The fix:** Create a proper `equipment` package that is the parent concept. Weapons, armor, tools, packs, and ammunition are all equipment. The `equipment` package should own the `EquipmentDetail` types and resolver, importing from `weapons`/`armor` as needed. Consider whether `weapons`/`armor` should become sub-packages of `equipment` (e.g., `equipment/weapons`, `equipment/armor`) or stay as peers that `equipment` imports.

**Prompt for the restructure session:**

> The rpg-toolkit has weapon and armor data in separate packages (`rulebooks/dnd5e/weapons/`, `rulebooks/dnd5e/armor/`) with shared types in `shared/`. We need to add an `EquipmentDetail` type that references weapon/armor types (WeaponCategory, WeaponProperty, DamageType, ArmorCategory, etc.), but `shared` can't import from `weapons`/`armor` without creating import cycles.
>
> Equipment is a first-class D&D concept — weapons, armor, tools, packs, and ammunition are all equipment. We need a proper `equipment` package that owns this hierarchy. Evaluate whether `weapons`/`armor` should become sub-packages of `equipment` or stay as peers. The goal is clean imports where `EquipmentDetail` can use proper typed constants, not strings.
>
> Check `rpg-project/ideas/equipment-enrichment/design.md` for the full enrichment design that depends on this restructure.
>
> Key files to review:
> - `rulebooks/dnd5e/weapons/types.go` — WeaponProperty, WeaponCategory, Weapon struct
> - `rulebooks/dnd5e/armor/armor.go` — ArmorCategory, Armor struct
> - `rulebooks/dnd5e/shared/equipment.go` — EquipmentID, EquipmentType, EquipmentCategory
> - `rulebooks/dnd5e/character/choices/requirements.go` — where equipment choices are built

## Implementation Order (after restructure)

1. **Protos first** — Add the field, generate code
2. **Toolkit** — Add types to equipment package, resolver, enrich requirement builders
3. **API** — Map the new field with Cost/Weight conversion
4. **UI** — Build EquipmentCard and wire into equipment choices

## Testing

- **Toolkit:** Test `ResolveEquipmentDetail` resolves weapons, armor, tools, packs, and ammunition. Test that it returns nil for unknown IDs. Test that enriched requirements contain populated details.
- **API:** Test the choice mapping produces correct `equipment_detail` proto fields including Cost/Weight conversion.
- **UI:** Verify `EquipmentCard` renders correctly for weapon, armor, and gear variants.
