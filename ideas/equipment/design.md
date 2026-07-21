# Equipment on the Wire (v1alpha2)

**Date:** 2026-07-20 (design) — 2026-07-21 (T1/T2/T3 shipped)
**Status:** Core slice merged across protos + toolkit + rpg-api. Real signoff (web data-source
swap + playtest over the live wire) is still ahead — see Signoff below.
**Scope:** Serve the approved equipment concept end-to-end: v1alpha2 wire + toolkit rules + rpg-api
orchestration, so the built-and-waiting web popover (rpg-dnd5e-web#557) wires up via a
data-source swap.

Not to be confused with `ideas/equipment-enrichment/` — that's an older (2026-03-21), separate
scope: stat display on character-creation equipment *choices* (the `EquipmentCard` component).
This doc is the in-combat/sheet equipped-slots + inventory system.

## Source of truth (already approved by Kirk 2026-07-20)

- **rpg-api-protos#187** — the wire request (this is the contract).
- **rpg-dnd5e-web#557** (draft) — fixture-first bench + `src/concepts/equipment/CONTRACT.md` + 9 behavioral tests in `fixtures.test.ts`. The fixture reducer's occupancy semantics ARE the server acceptance spec.
- Coordinates with **rpg-project#94** (rules-side equip design, still capture-only) and subsumes the CharacterData portion of **rpg-api-protos#183**.

## Boundary reading (the honesty this slice enforces)

```
web      sends References, renders server-composed display fields — computes nothing
rpg-api  orchestrates by key: calls toolkit equip rules, passes composed fields through
toolkit  owns the rules: equip/unequip effects, AC recompute, stat_line + AC note, occupancy
```

Three boundary decisions pinned here (were unhomed before this slice):

1. **`stat_line` and the AC `note` compose in the TOOLKIT**, as a display projection over the existing
   `EffectiveAC() *combat.ACBreakdown` and `equipment.ResolveEquipmentDetail()`. rpg-api passes them
   through verbatim; the web renders them verbatim. CONTRACT §4/§5: "the web must never assemble this
   from rules data… the note is a server string." (server = the rules engine authors the string.)

2. **Two-handed occupancy is a RULE in `Character.EquipItem`** (toolkit), not presentation: equipping a
   two-handed weapon clears/blocks `off_hand`; equipping into an occupied slot returns the occupant to
   inventory. Acceptance = the 9 behavioral tests from web#557 `fixtures.test.ts`, ported to Go.

3. **Surface placement (v1alpha2):**
   - `CharacterData` (encounter HUD snapshot) gains `equipped`, `inventory`, `slots`, real AC + note.
   - Equip/Unequip **intent RPCs** live on a **new `dnd5e/api/v1alpha2/character` service** (out-of-combat
     sheet), returning the recomputed character. Encounter-scoped equip (action-economy cost) is deferred
     to rpg-project#94 — the RPCs are character-scoped for now.

## Why Option B (not the full #94 pass, not the minimal lift)

Grounded current-state (verified across 3 repos, as of the 2026-07-20 design pass):

- **Reuse (already built):** toolkit `Character.EquipItem/UnequipItem/EquipmentSlots`;
  `EffectiveAC() *combat.ACBreakdown` computing AC from worn armor+shield+DEX through a staged modifier
  chain; `ResolveEquipmentDetail()` structured stats; weapon/armor registries. protos already model
  `Equipment/WeaponData/ArmorData` (v1alpha1).
- **The real gap:** none of it is on the **v1alpha2** wire (still v1alpha1, string-ids + enum slots);
  `stat_line` has no composer; `ACBreakdown` has no note field and **rpg-api never calls `EffectiveAC`**
  (it ships the stored int); **two-handed occupancy exists nowhere**.

So the honest slice is: build the v1alpha2 surface, add the one missing rule (occupancy) + two display
projections (stat_line, AC note) in the toolkit, and re-point rpg-api at the toolkit's rules engine.
That's "lean = what we need, done properly" — not the full #94 pipeline, not a lift that ships wrong AC.

## Non-goals (deferred, tracked under rpg-project#94)

Encumbrance / weight effects · quantity / stacking · proficiency effects on equip · creation-time
loadout redesign · in-combat action-economy cost of equipping · drag-and-drop (presentation polish).

## What shipped

- **T1 protos — [rpg-api-protos#188](https://github.com/KirkDiggler/rpg-api-protos/pull/188)** (merged 2026-07-21, closes #187): v1alpha2 `CharacterData.equipped/inventory/slots/armor_class_detail/main_hand_damage`,
  new `Item`/`SlotDef`/`ArmorClassDisplay` messages, new `dnd5e.api.v1alpha2.character.CharacterService`
  (`EquipItem`/`UnequipItem`, character-scoped, out-of-combat sheet). Ref-based, slots-as-data,
  keys-not-enums, as designed. v1alpha1's string-id + enum-slot equipment surface deprecated in
  place, not deleted.
  - A same-PR detour: an adversarial gate surfaced that `Ref` (the generic module/type/id envelope)
    only lived inside `dnd5e.api.v1alpha2.encounter`, forcing the new `character` package to import
    `encounter` just to name a `Ref`. A relocation to a shared `api.v1alpha2.core` package was tried,
    then **reverted** — the web imports `RefSchema` directly from the generated `encounter/types_pb`
    (`useSetReactionReady.ts`), so a hard move would have broken the web's build, not just the wire.
    Deferred to **rpg-api-protos#189** as a future coordinated protos+web migration. See
    `ideas/typed-ref-vocabulary/design.md` for the fuller foundation question this opened up.
- **T2 toolkit — [rpg-toolkit#812](https://github.com/KirkDiggler/rpg-toolkit/pull/812)** (merged
  2026-07-21, closes #811): two-handed occupancy in `Character.EquipItem` (the 9 behavioral tests
  from web#557's fixture reducer, ported to Go, TDD); the `stat_line` and AC `note` display
  projections over `EffectiveAC()` and `ResolveEquipmentDetail()`.
- **T3 rpg-api — [rpg-api#682](https://github.com/KirkDiggler/rpg-api/pull/682)** (merged 2026-07-21,
  closes #680): implements the v1alpha2 character service; equip/unequip call the toolkit's
  `Character.EquipItem` (rules, not a bare `Set`); AC now comes from `EffectiveAC()`, replacing the
  previously-shipped stored int; populates `CharacterData` equipment/inventory/slots + composed
  fields; pushes the recomputed snapshot.

All three merged same-day (2026-07-21), T1+T2 in parallel as planned, T3 integrating both via a
local `replace` during development, bumped to real versions before its own PR opened.

## Deferred / tracked separately

- **In-combat live-push of equipment changes** — [rpg-api#681](https://github.com/KirkDiggler/rpg-api/issues/681):
  the character-scoped `EquipItem`/`UnequipItem` RPCs return the recomputed character, but an
  equip change made mid-encounter doesn't yet push onto open `StreamEncounter` viewers (an
  `entity-data-changed`-style event). Needed before equipping is usable *during* combat, not just
  between encounters.
- **Combat/seat AC single-source** — [rpg-api#684](https://github.com/KirkDiggler/rpg-api/issues/684):
  `Entity.armor_class` on the combat/seat path should read `EffectiveAC()` directly rather than a
  separately-maintained stored int, closing the class of drift `armor_class_detail` vs
  `Entity.armor_class` syncing was papering over on the character-service path.
- **Typed `Item` rebase + slots/kinds enums** — see `ideas/typed-ref-vocabulary/design.md`. `Item.ref`
  (currently a generic `Ref`), `Item.kind`, and `SlotDef.key`/`slot_keys` (currently open strings) are
  candidates to become typed enums once the generator foundation (protos#190/#191) has a use for them.
  Deliberately not done in this slice — see that doc for why.
- **The web data-source swap + playtest** — the actual signoff bar (below), still ahead as of this
  writing.

## Acceptance / signoff

- 9 occupancy behaviors green in Go (T2) — **done**, merged in rpg-toolkit#812.
- rpg-api serves real AC (matches `EffectiveAC`, not stored int) + composed `stat_line` for the fighter/
  barbarian/rogue casts from the concept — **done**, merged in rpg-api#682.
- **Signoff bar = observable via the web popover over the real wire (data-source swap), evidence on
  the PR. This has NOT happened yet** — rpg-dnd5e-web#557's popover is built against fixtures; wiring
  it to the real `CharacterService`/`CharacterData` is the remaining step before this slice is
  actually done from a player's perspective, not just merged.
- Nothing merged without Kirk; all three PRs were draft-first with Copilot + adversarial gate before
  merge, as planned.
