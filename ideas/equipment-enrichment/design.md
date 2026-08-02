# Equipment Data Enrichment

**Date:** 2026-03-21 (updated 2026-08-01 — concrete-options architecture decision; corrected 2026-08-02 — scope narrowed to existing live data, see correction note below)
**Status:** Design updated — current pass narrowed to web-only rich rendering from existing live data; concrete-options/toolkit/API architecture below remains the deferred future decision
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

## 2026-08-02 correction: current dropdown-formatting pass needs none of this

**Kirk's correction, propagated across tracking:** the immediate work is rich
content inside the *existing production* equipment dropdown, and it needs **no
new contract**. `ListEquipmentByTypeResponse.equipment` (rpg-api-protos
`character.proto` / `equipment_types.proto`, `message Equipment`) already
returns full `Equipment` per item today — `WeaponData`/`ArmorData`, cost,
weight, description — for every category-dropdown call the web already makes.
The web can render rich `EquipmentCard`-shaped content directly from that
existing live response; there is no wire-shape gap to fill for this pass, and
no concept/fixtures lab or standalone preview is needed since the real data
is already available live. Accordingly:

- **rpg-api-protos#204** (the additive `EquipmentCategoryChoice.options`
  field implementing the decision below) is **closed without merge** —
  premature for this pass, not wrong as a future decision.
- **rpg-api-protos#202**, **rpg-toolkit#872**, and **rpg-api#755** move to
  Todo/deferred on Project 19. They stay open — they are the tracking
  issues for the *exact, toolkit-resolved* concrete-eligibility leg (Monk's
  full `simple-melee`/`simple-ranged` set, special exclusions) that remains a
  real future need, just not part of formatting the current dropdown.
- **rpg-dnd5e-web#668** is rescoped to the immediate production rich-dropdown
  formatting pass against existing `ListEquipmentByType` data, explicitly
  **excluding** Monk/exact category eligibility.
- The Phase 1 fixtures/standalone-preview plan in `plan.md` (port 3002,
  typed-fixture picker) is superseded for this reason — see `plan.md` for the
  corrected phasing.

The **2026-08-01 update below is unchanged as the future architecture
decision** for exact toolkit-resolved category eligibility (the Monk sharp
edge) — it is deferred, not reversed. This correction only concerns *when*
that wave lands relative to the current dropdown-formatting work, and
clarifies that the current work does not require it at all.

## 2026-08-01 update: the MEATY wave — category choices resolve to concrete enriched options

The design above (2026-03-21) covers enrichment of *concrete* equipment items —
named gear the character definitely gets. It did not fully settle the harder half
of the problem: **category choices** ("choose a martial weapon", "choose two
simple weapons") were left with the API calling a separate weapon-list endpoint
and the UI reconstructing which items are actually eligible for a given category.
That reconstruction is a boundary violation — the UI ends up doing rules-shaped
work (which weapon IDs count as "simple melee," which are excluded by a
class-specific carve-out) that only the toolkit should know. This section is the
verified architecture decision for that wave.

### Decision

**The toolkit resolves each `EquipmentCategoryChoice` to the concrete, eligible,
enriched `EquipmentItem`s at the source — not the API, not the web.**

- For every category choice attached to a class/background grant, the toolkit
  expands the category into its full, concrete membership using the same
  eligibility rules it already enforces when *validating* a chosen item — so
  there is exactly one place (the toolkit) that knows what belongs in "simple
  melee" or "martial ranged."
- **Monk is the sharp edge that proves the rule isn't a shortcut.** Monk's
  weapon proficiency is the full actual `simple-melee` + `simple-ranged` sets,
  not a hand-picked shortlist — the expansion must walk the real weapon
  registries the same way the validator does, or the two will drift apart the
  next time a weapon is added.
- **Existing special exclusions carry over unchanged.** Any class/category
  carve-out that already excludes specific weapon IDs from an otherwise-open
  category (e.g. a category that is "martial weapons except heavy ones a small
  race can't use") applies during expansion, not as a UI-side filter
  afterward. The expansion is the validator's own eligibility function reused
  for enumeration, so exclusions can't fall out of sync with what a chosen item
  is actually validated against.
- Each resolved option is a fully enriched `EquipmentItem` (`EquipmentDetail` /
  `WeaponDetail` / `ArmorDetail` populated per the Layer 1 design above) — a
  category choice's options are indistinguishable, data-shape-wise, from a
  concrete grant's item once they reach the wire.

### Proto: additive, repeated concrete options

The proto contract carries the resolved membership as a `repeated` field of
concrete, enriched options on the category-choice message — additive only, no
breaking change to the existing `EquipmentItem`/`equipment_detail` shape landed
in the 2026-03-21 design:

```protobuf
message EquipmentCategoryChoice {
  string selection_id = 1;
  string category = 2;       // existing: human-readable category label
  int32 choose_count = 3;    // existing: how many the player picks
  repeated EquipmentItem options = 4;  // NEW: concrete, enriched, eligible options
}
```

`options` is additive on a message that already exists on the wire — clients that
don't read it are unaffected; clients that do read it get the full resolved list
directly, with `equipment_detail` already populated per option.

### API: translate only

The API's choice-mapping layer maps the toolkit's resolved category-choice
membership straight into `options`, using the exact same `EquipmentItem` mapping
(including Cost/Weight conversion) already built for concrete grants. No new
rules knowledge, no new eligibility logic — the API is a pass-through of what the
toolkit already decided.

### Web: delete the reconstruction, render the rich options directly

This wave **removes** the web's client-side category→type eligibility
reconstruction and its `ListEquipmentByType` call entirely. Once `options`
carries concrete, enriched items, the web has no remaining reason to ask "which
weapon IDs count as martial" — it renders `options` with the same `EquipmentCard`
the concrete-grant path already uses (Layer 4 above) and lets the player pick
`choose_count` of them. This is the payoff of the boundary decision: a whole
class of client-side rules logic disappears rather than needing to be kept in
sync with the toolkit going forward.

### Alternatives rejected

- **Keep `ListEquipmentByType` as a separate lookup, just enrich its response.**
  Rejected: this keeps two independent code paths (concrete grants vs. category
  browsing) that both need to agree on eligibility and enrichment, and keeps the
  "which type does this category mean" mapping duplicated across API and web.
  The category choice already has a natural single owner (the toolkit, at
  resolution time) — a second lookup endpoint is redundant with `options`.
- **Send category + type-hint, let the API resolve eligible IDs.** Rejected:
  this just moves the reconstruction from web to API, and the API still isn't
  the one that owns weapon-eligibility rules (Monk's set, exclusions) — it would
  have to import toolkit-shaped knowledge to do it, which is exactly the
  boundary violation being fixed. Resolution has to happen where validation
  already happens: the toolkit.
- **Enumerate categories as an enum + let the client hardcode membership.**
  Rejected outright — this is the status quo bug (a hardcoded/derived client
  eligibility list silently drifting from the real validator), not a fix.

### Migration

- Additive proto field — no version bump required beyond the normal additive
  process; existing `EquipmentCategoryChoice` consumers on old API versions are
  unaffected (empty `options`).
- API and web migrate together: the web PR that starts reading `options` is the
  same PR that deletes `ListEquipmentByType` and the client-side eligibility
  reconstruction — no dark period where both paths are live and could disagree.
- No data migration — this is a request/response shape change, nothing stored.

### Tests

- **Toolkit:** category-choice expansion returns the exact membership the
  validator accepts for that category, for every class that has a category
  choice — asserted by generating the category's expansion and confirming each
  member independently passes the validator, and that nothing the validator
  accepts is missing from the expansion. Monk's `simple-melee` + `simple-ranged`
  expansion specifically asserted against the full weapon registry count (not a
  hand-picked subset) so a future added weapon is caught by a size assertion,
  not silently missed. Existing special-exclusion cases get a test asserting the
  excluded ID appears in neither the expansion nor a successful validation.
- **API:** choice-mapping test asserting `options` on the wire matches the
  toolkit's expansion 1:1, including enrichment fields (reuses the existing
  concrete-item mapping test fixtures where possible rather than duplicating
  Cost/Weight conversion assertions).
- **Web:** component test rendering a category choice from fixture `options`
  data (no `ListEquipmentByType` call in the test — confirms the deletion),
  asserting `EquipmentCard` renders per option and selection respects
  `choose_count`.

### Merge order (inside-out, per rpg-project's wave-shape rule)

1. **Toolkit** — implement category-choice expansion, land and tag.
2. **Protos** — land the additive `options` field (can happen in parallel with
   toolkit implementation since it's additive and doesn't depend on the
   toolkit's internals, but the API bump depends on both being real).
3. **API** — bump to the toolkit's real published tag, map `options` onto the
   wire, **remove any local `replace` directive** pointing at a toolkit
   worktree before this merges (see Development note below).
4. **Web** — consume `options`, delete `ListEquipmentByType` + the client-side
   eligibility reconstruction. Lands last because it depends on the proto
   contract already being live on the API it's pointed at.

This is the standard toolkit → api → web merge order; only the require-protos-
first nuance is new here, because this wave (unlike a pure toolkit/api change)
has a real wire-shape addition consumed by two downstream layers.

### Development note — local toolkit override, never committed

During development, rpg-api's worktree for this wave points its toolkit module
at a local toolkit worktree via the documented local-override mechanism
(`rpg-api/docs/how-to/local-toolkit-override.md`) so the API side can iterate
against unpublished toolkit changes. **This override must never be committed** —
it is stripped (real tag substituted) as part of the API PR that actually merges,
per the merge order above. Only ever override the one `rpg-toolkit` module for
this wave; needing more is the signal the wave was sliced too thin (working-
agreements.md `Unmerged provider work`).
