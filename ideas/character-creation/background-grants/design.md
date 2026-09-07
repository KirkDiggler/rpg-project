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

## 6. Confirmed toolkit-only — for the *fixed* grants only (PR 1 + PR 2)

All three fixed-grant types populate structures that already have working wire paths: skills
join the same `skillMap` race/class already populate (already projected — players already see
their skills today); equipment joins the same `Inventory` list class equipment already populates
(same existing `inventory` wire field); gold sets `Wallet`, which already has a wire path from
`CharacterData.wallet` (field 17, shipped this session). No new protos/api/web work for this part.

**This does not hold for PR 3.** Toolkit's own investigation (rpg-toolkit#1554 comments) found six
backgrounds carry choice-shaped content (an equipment and/or tool-proficiency pick — Entertainer,
Folk Hero, Guild Artisan/Merchant, Outlander, Noble/Knight, Soldier, Charlatan) that was never
modeled at all, not partially wired. Fixing that properly means PR 3 gives `BackgroundChoices` two
new fields, `Equipment []EquipmentChoiceSelection` and `Tools []shared.SelectionID` (mirroring
`ClassChoices`) — and that surface **does** cross the toolkit boundary. See §8.

## 7. Done when

Every one of the 17 backgrounds, freshly compiled into a character, produces the correct skill
proficiencies, tool proficiencies, equipment, and starting gold — table-driven test per
background, not a handful of spot checks. The three TODOs in `draft.go` are deleted, not just
addressed in spirit. rpg-toolkit#162 and the background-half of #615 close as superseded.

## 8. Cross-repo impact of PR 3 (the six background choices) — not urgent, written down before it's discovered cold

PR 3 hasn't landed (blocked on rpg-toolkit#1555, then PR 1, then PR 2 — see that issue for
sequencing). Nothing here needs action yet. This section exists so the protos/api/web sessions
don't each re-derive it independently once PR 3 does land, and so nobody hits it as a surprise.

**rpg-api-protos — `BackgroundInfo` has no choice-discovery field.** `RaceInfo` and `ClassInfo`
(`character_pb.ts`) both already carry a generic `choices: Choice[]` field so a client can discover
what to render, using the existing `ChoiceCategory`/`Choice`/`ChoiceSubmission`/`ChoiceData` shapes
(category + source, oneof selection payload) already defined in `choices.proto` and already used
for class choices today. `BackgroundInfo` has **none of that** — only old fixed scalars
(`skill_proficiencies: Skill[]`, `tool_proficiencies: Tool[]`, `additional_languages: int32`,
etc.), nothing shaped like a player-facing choice. For the client to show real Equipment/Tools
picks for the six backgrounds above, something has to describe what the options *are*, not just
accept a submission. Needed: add `choices: Choice[]` to `BackgroundInfo`, mirroring Race/Class —
this is new proto surface, not just a codegen bump.

**rpg-api — `UpdateBackground` handler silently drops Equipment/Tools submissions.** Verified
directly (`internal/handlers/dnd5e/v1alpha1/character/handler.go:384-424`): the handler loops
`req.BackgroundChoices` and only has a branch for `CHOICE_CATEGORY_LANGUAGES`, building
`toolkitchar.BackgroundChoices` from that alone. `CHOICE_CATEGORY_EQUIPMENT`/`_TOOLS` entries pass
through the loop and are dropped with no error — a silent no-op, not a rejection. The wire already
supports both categories (`choices.proto`'s `ChoiceCategory` enum has had `EQUIPMENT=1`/`TOOLS=3`
all along); this is purely a missing handler branch, waiting on toolkit's PR 3 to give
`BackgroundChoices` the `Equipment`/`Tools` fields to convert into.

**rpg-dnd5e-web — zero background-choice UI exists today, but the reusable piece already does.**
`SkillsBackgroundSection.tsx`'s `handleBackgroundSelect` unconditionally calls
`setBackground(background, [])` — its own comment says "Backgrounds don't have choices - they
provide fixed proficiencies," which was true until PR 3. `ChoiceRenderer.tsx` already renders
EQUIPMENT/TOOLS/SKILLS/LANGUAGES generically for class/race choices today, so once
`BackgroundInfo.choices` exists (protos, above), background selection should be able to reuse that
component rather than building new choice UI from scratch.

**Sequencing implied by the above:** toolkit PR 3 → rpg-api-protos (`BackgroundInfo.choices`) →
rpg-api (handler branches + response population) → rpg-dnd5e-web (wire `SkillsBackgroundSection`
to `ChoiceRenderer` instead of the hardcoded empty submission). Each step is genuinely blocked on
the one before it — there's nothing for protos/api/web to build against until toolkit's
`BackgroundChoices.Equipment`/`.Tools` fields exist.
