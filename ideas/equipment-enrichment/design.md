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

**New types in `equipment` package** (see Package Structure section for type definitions):

The `equipment` package already imports `weapons`, `armor`, etc. — so `EquipmentDetail`, `WeaponDetail`, and `ArmorDetail` use proper typed constants (no string workarounds). Tools, packs, and ammunition get base fields only (name, weight, cost) since they don't have type-specific stats that affect gameplay decisions.

**Resolver function** in `equipment/`:

```go
// ResolveEquipmentDetail looks up an equipment ID using the existing GetByID
// pattern and returns a populated detail struct.
// Returns nil if the ID is not found in any registry.
func ResolveEquipmentDetail(id shared.EquipmentID) *EquipmentDetail
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
| **rpg-toolkit** | Add `EquipmentDetail` types to `equipment/`, `ResolveEquipmentDetail()` extending existing `GetByID`, call it in equipment requirement builders and grants |
| **rpg-api-protos** | Add `equipment_detail` field to `EquipmentItem` message |
| **rpg-api** | Map toolkit `Detail` to proto `equipment_detail` in choice handler, including Cost/Weight conversion |
| **rpg-dnd5e-web** | Create `EquipmentCard` component, use in equipment choice selector and category dropdowns |

## Package Structure (No Restructure Needed)

The `equipment` package already exists at `rulebooks/dnd5e/equipment/` and imports `weapons`, `armor`, `tools`, `packs`, and `ammunition` — exactly the dependency direction needed. It has a `GetByID` function that looks up across all registries, which is the foundation for `ResolveEquipmentDetail`.

The `EquipmentDetail`, `WeaponDetail`, and `ArmorDetail` types belong in the `equipment` package where they can use proper typed constants from `weapons` and `armor` packages (no string workarounds needed).

**Updated types using proper package types:**

```go
// In rulebooks/dnd5e/equipment/

type EquipmentDetail struct {
    Name     string                `json:"name"`
    Type     shared.EquipmentType  `json:"type"`
    Weight   float64               `json:"weight"`
    Cost     string                `json:"cost"`
    Weapon   *WeaponDetail         `json:"weapon,omitempty"`
    Armor    *ArmorDetail          `json:"armor,omitempty"`
}

type WeaponDetail struct {
    Category   weapons.WeaponCategory   `json:"category"`
    Damage     string                   `json:"damage"`
    DamageType damage.Type              `json:"damage_type"`
    Properties []weapons.WeaponProperty `json:"properties"`
    Range      *weapons.Range           `json:"range,omitempty"`
}

type ArmorDetail struct {
    Category            armor.ArmorCategory `json:"category"`
    BaseAC              int                 `json:"base_ac"`
    DexBonus            bool                `json:"dex_bonus"`
    MaxDexBonus         *int                `json:"max_dex_bonus,omitempty"`
    StrengthRequirement int                 `json:"strength_requirement,omitempty"`
    StealthDisadvantage bool                `json:"stealth_disadvantage"`
}
```

`ResolveEquipmentDetail` extends the existing `GetByID` pattern — look up the equipment, then populate the detail struct from the resolved item.

## Implementation Order (after restructure)

1. **Protos first** — Add the field, generate code
2. **Toolkit** — Add detail types to `equipment/`, extend `GetByID` into `ResolveEquipmentDetail`, enrich requirement builders
3. **API** — Map the new field with Cost/Weight conversion
4. **UI** — Build EquipmentCard and wire into equipment choices

## Testing

- **Toolkit:** Test `ResolveEquipmentDetail` resolves weapons, armor, tools, packs, and ammunition. Test that it returns nil for unknown IDs. Test that enriched requirements contain populated details.
- **API:** Test the choice mapping produces correct `equipment_detail` proto fields including Cost/Weight conversion.
- **UI:** Verify `EquipmentCard` renders correctly for weapon, armor, and gear variants.
