# Background grants: equipment, gold, and turning on data that's already real

**Parent:** rpg-project#382

**Supersedes (once built):** rpg-toolkit#162 (open since 2025-08-06, correct problem statement,
stale proposed fix), the background-half of rpg-toolkit#615 (which incorrectly claims
`backgrounds/grants.go` already fires during finalization — verified false, zero callers).
rpg-toolkit#661 (BackgroundID dropped on `LoadFromData`) is a different bug, not superseded here.

## 1. What exists today, verified directly against the code

- `backgrounds.Data` (`backgrounds/data.go`) — `SkillCount`, `Skills`, `LanguageCount`, `Feature`
  (a display string only). 17 backgrounds populated.
- `backgrounds.Grant`/`GetGrants(bg)` (`backgrounds/grants.go`) — a **separate, richer, already
  fully populated** structure: real `SkillProficiencies` and `ToolProficiencies` per background
  (Acolyte: Insight+Religion; Criminal/Spy: Deception+Stealth+cards+thieves' tools; all 17 done).
  This data is real and presumed SRD-sourced (not authored in this pass, not being re-audited).
- **Zero callers of `backgrounds.GetGrants` anywhere in the real tree** (grepped the whole repo,
  excluding stale `.claude/worktrees/*` duplicates). Compare: `races.GetGrants` genuinely IS
  called (`draft.go:1018`) and `classes.GetGrants` genuinely IS called
  (`permanent_conditions.go:75`, internally within `classes/grant.go:237`) — background is the
  one of the three that was never wired in.
- Three TODOs sitting in `character/draft.go`, all pointing at the same root gap:
  `compileSkills` (`"Add background skills when we have internal background data"`),
  a validation path (`"Add background skills when background data is implemented"`), and
  `SetBackground` (`"Validate background when we have internal background data"`). None have
  been actioned.
- `EquipmentItem{ID, Quantity}` already exists in two shapes — `classes.EquipmentItem` (the one
  that actually feeds `compileInventory` today) and a different `choices.EquipmentItem{ID,
  Quantity, Detail}` (a UI-display shape for equipment-choice screens). `classes` defines its own
  locally rather than importing a shared type — that's the existing precedent to follow, not a
  gap to fix here.
- `currency` has zero rpg-toolkit-specific dependencies (stdlib only) — `Grant` gaining
  `StartingGold currency.Money` has no import-cycle risk.

## 2. The shape

```go
package backgrounds

// EquipmentItem is one item a background grants, by identity and quantity --
// mirrors classes.EquipmentItem's shape exactly (same convention: each
// package defines its own rather than importing a shared type).
type EquipmentItem struct {
    ID       shared.EquipmentID
    Quantity int
}

type Grant struct {
    SkillProficiencies []skills.Skill
    ToolProficiencies  []proficiencies.Tool
    Languages          []languages.Language

    // New:
    Equipment    []EquipmentItem
    StartingGold currency.Money
}
```

`Grant` is the right home, not `Data` — `Grant` is already "what you get automatically" (its own
doc comment), and equipment/gold are exactly that, same as skills/tools already are.

## 3. Scope — three wiring call-sites, not one

Adding the two fields above and populating 17 real data entries does nothing on its own --
`GetGrants` has to actually be called. Three places in `character/draft.go`:

1. **`compileSkills`** — add a background loop, identical shape to the existing racial loop:
   ```go
   if grant := backgrounds.GetGrants(d.background); grant != nil {
       for _, skill := range grant.SkillProficiencies {
           skillMap[skill] = shared.Proficient
       }
   }
   ```
   (Tool proficiencies land wherever `compileProficiencies` currently assembles
   `[]proficiencies.Tool` — same shape, same function, not yet checked for its exact call site.)
2. **`compileInventory`** — add a background-equipment loop alongside the existing
   `inventoryItemsFromGrant`-style class loop, same `equipment.GetByID` resolution.
3. **A new small step, `compileWallet`** (or wherever `Wallet` gets set during finalize) —
   `Wallet = grant.StartingGold` (as `currency.Money` directly, per §2, this is just an assign).

## 4. Data-sourcing discipline

Existing skill/tool data: trusted as-is, not re-verified in this pass. **New** `Equipment` and
`StartingGold` values per background: sourced and cross-checked against two independent SRD
references before going in, same discipline the item-catalog wave (rpg-toolkit#1524) used — not
typed from memory, not assumed correct by analogy to the skill data sitting next to it.

## 5. Explicitly out of scope

- **Background *features*** (Acolyte's "Shelter of the Faithful" actually doing something
  mechanical) — still just a display string, its own already-flagged future TODO, not touched
  here.
- **Feats** — not applicable; backgrounds don't grant feats in RAW.
- **Packs** — if any background's equipment list happens to include a pack (unconfirmed; PHB
  background equipment is normally itemized directly, packs are usually a class-equipment-choice
  thing, not a background grant) — defer that specific entry until `Unpack` exists. Verify when
  sourcing the real data; may be moot.
- **`backgrounds.Data`/`SetBackground` validation TODO** — a separate, narrower gap (validating a
  background ID is real at selection time), not fixed by this wave.

## 6. Confirmed toolkit-only

All three grant types populate structures that already have working wire paths: skills join the
same `skillMap` race/class already populate (already projected — players already see their
skills today); equipment joins the same `Inventory` list class equipment already populates (same
existing `inventory` wire field); gold sets `Wallet`, which already has a wire path from
`CharacterData.wallet` (field 17, shipped this session). No new protos/api/web work.

## 7. Done when

Every one of the 17 backgrounds, freshly compiled into a character, produces the correct skill
proficiencies, tool proficiencies, equipment, and starting gold — table-driven test per
background, not a handful of spot checks. The three TODOs in `draft.go` are deleted, not just
addressed in spirit. rpg-toolkit#162 and the background-half of #615 close as superseded.
