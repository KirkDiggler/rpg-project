# Item Icon Coverage — Canonical Weapon/Armor Registry Design

**Date:** 2026-07-21
**Status:** Design — pending Kirk review. Plan follows in this same PR after design approval.
**Scope:** Web-side lookup that maps canonical weapon/armor ids to already-deployed sprite icons
in the live equipment popover — a presentation-only gap-closer, no server/toolkit/protos change.
**Implementation repo:** rpg-dnd5e-web ([rpg-dnd5e-web#576](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/576))
**Relates to:** `ideas/equipment/design.md` (the equipment-on-the-wire slice this popover renders
data for) and `ideas/typed-ref-vocabulary/design.md` (source of the 38/13 canonical enum counts
this design's coverage boundary is pinned to).

## Context

The live equipment popover (`EquipmentSlots`/`InventoryLight`, promoted for rpg-dnd5e-web#571)
renders every item text-only today. `Item.icon_key` on the wire is always empty (rpg-api#680
scope-decision: no toolkit/asset-manifest source exists yet) and `resolveIconUrl`
(`equipmentTypes.ts`) correctly returns `undefined` for an empty key, so nothing breaks — it just
never shows an icon.

rpg-dnd5e-web#576 asks for a **web-side lookup** that closes this gap without platform work,
following the precedent already shipped twice in that repo: `src/utils/actionIcons.ts` (#497) and
`src/utils/conditionIcons.ts` (#473) both resolve a server-authored id through a web-owned map and
fall back to text/emoji for anything unmapped — never a broken `<img>`. `itemIcons.ts` is the same
shape for items. This supersedes rpg-api#691 (server-populated `icon_key`, closed); the wire field
stays as-is, available later for module/homebrew content that ships its own icon reference.

The sprites this lookup points at are already deployed — `public/models/synty/ui/library/**` in
rpg-dnd5e-web, synced from `rpg-game-assets` by that repo's `docker.yml` on every build (license
permits shipping in builds, forbids public-repo redistribution of source assets). This is a
lookup-only change: no new art, no server change, no rules change.

Icons are pure presentation. The lookup never decides what an item does or gates whether it can be
equipped — that boundary is unconditional (see rpg-dnd5e-web's CLAUDE.md: "renders what the API
sends; it never calculates game rules").

## Canonical coverage boundary

The toolkit registry (`rpg-toolkit/rulebooks/dnd5e/weapons`, `.../dnd5e/armor`) is the source of
truth. `rpg-api-protos` generates the wire enums 1:1 from it via `make refgen` (generated files are
headed "DO NOT EDIT — Regenerate with: make refgen"), per `ideas/typed-ref-vocabulary/design.md`.
The enum's inline comments carry the canonical kebab-case ids this lookup keys on — e.g.
`WEAPON_WAR_PICK = 36; // war-pick` in `dnd5e/api/v1alpha2/weapons/weapons.proto`.

**v1 coverage = every concrete enum value, excluding `*_UNSPECIFIED`:**

- 38 `Weapon` values (`dnd5e/api/v1alpha2/weapons/weapons.proto`)
- 13 `Armor` values (`dnd5e/api/v1alpha2/armor/armor.proto`)

Category placeholders (`simple-weapon`, `martial-weapon`, `any-weapon`) are choice-requirement
consts, not concrete weapons — the proto generator already excludes them from the enum ("aren't in
the registry"), and this lookup excludes them the same way.

**Explicitly out of the canonical boundary:** non-enum gear — ammunition (crossbow bolts), packs
(dungeoneer's pack), tools, and anything else with no wire enum today. These may get a handful of
entries in a clearly separate, non-exhaustive **supplemental map** (see below) but are never
counted toward, or tested against, the 38+13 exhaustiveness requirement.

### Canonical weapon IDs (38)

| id             | tier        | id             | tier        |
| -------------- | ----------- | -------------- | ----------- |
| battleaxe      | dedicated   | longbow        | dedicated   |
| blowgun        | generic     | longsword      | dedicated   |
| club           | generic     | mace           | dedicated   |
| dagger         | dedicated   | maul           | dedicated   |
| dart           | approximate | morningstar    | dedicated   |
| flail          | approximate | net            | generic     |
| glaive         | dedicated   | pike           | dedicated   |
| greataxe       | dedicated   | quarterstaff   | dedicated   |
| greatclub      | approximate | rapier         | dedicated   |
| greatsword     | dedicated   | scimitar       | dedicated   |
| halberd        | dedicated   | shortbow       | dedicated   |
| hand-crossbow  | approximate | shortsword     | dedicated   |
| handaxe        | dedicated   | sickle         | approximate |
| heavy-crossbow | approximate | sling          | generic     |
| javelin        | dedicated   | spear          | dedicated   |
| lance          | dedicated   | trident        | dedicated   |
| light-crossbow | approximate | unarmed-strike | generic     |
| light-hammer   | dedicated   | war-pick       | dedicated   |
|                |             | warhammer      | dedicated   |
|                |             | whip           | generic     |

(38 rows total: 25 dedicated, 7 approximate, 6 generic.)

### Canonical armor IDs (13)

| id              | tier      |
| --------------- | --------- |
| breastplate     | generic   |
| chain-mail      | generic   |
| chain-shirt     | generic   |
| half-plate      | generic   |
| hide            | generic   |
| leather         | generic   |
| padded          | generic   |
| plate           | generic   |
| ring-mail       | generic   |
| scale-mail      | generic   |
| shield          | dedicated |
| splint          | generic   |
| studded-leather | generic   |

(13 rows total: 1 dedicated, 12 generic — no armor id is "approximate"; non-shield armor has no
per-weight sprite in the pack at all, so it's generic rather than a near-miss.)

## Resolution precedence

One function implements the full precedence so callers make a single call instead of re-deriving
the order:

1. **Wire wins.** If `item.iconKey` is non-empty, resolve it directly (existing `resolveIconUrl`
   behavior) — a future manifest key always overrides the static registry below.
2. **Canonical, then supplemental.** Else look up `item.ref.id` in `ITEM_ICONS` (the exhaustive
   38+13 map), then in `SUPPLEMENTAL_ITEM_ICONS` (the small, non-exhaustive extra map).
3. **Text-only.** Else return `undefined`. Truly unknown ids — homebrew, module content, anything
   not in either map — render text-only, exactly like `actionIcons.ts`/`conditionIcons.ts` already
   do for their unmapped keys. Never a placeholder/warning icon, never a broken `<img>`.

This is a **presentation-only** decision tree: it never influences whether an item can be equipped,
what it does, or any other rules outcome.

## Registry shape (`src/utils/itemIcons.ts` in rpg-dnd5e-web)

```ts
export type IconQuality = 'dedicated' | 'approximate' | 'generic';

export interface ItemIconEntry {
  /** Relative path under the Synty UI library base, e.g.
   *  "icons/weapons/ICON_SM_Wep_Sword_02_Clean.png" — same base
   *  `resolveIconUrl` already resolves against. */
  path: string;
  quality: IconQuality;
}

/** Exhaustive over the 38 weapon + 13 armor canonical ids. No canonical id
 *  omitted — enforced by a test that iterates the literal id list above,
 *  not `Object.keys(ITEM_ICONS)` (so a silently-missing key fails loudly
 *  instead of the test just seeing a smaller-but-still-"complete" map). */
export const ITEM_ICONS: Record<string, ItemIconEntry>;

/** Small, intentionally non-exhaustive. Non-enum gear (ammo, packs, tools,
 *  fixture-only flavor items like `torch`) that happens to have a
 *  reasonable icon. Never checked for completeness — absence here is
 *  normal and expected. */
export const SUPPLEMENTAL_ITEM_ICONS: Record<string, ItemIconEntry>;

/** Implements the 3-step precedence above. `ref` carries `id` (and
 *  `module`/`type`, unused here — id is unique within the weapon/armor/
 *  item id space this registry covers). */
export function getItemIconUrl(
  ref: { id: string },
  iconKey: string
): string | undefined;
```

`getItemIconUrl` is the new call site for `EquipmentSlots.tsx` and `InventoryLight.tsx`, replacing
their current direct `resolveIconUrl(item.iconKey)` call. `resolveIconUrl` itself is unchanged — it
stays the pure "manifest key -> URL, or undefined" helper; `getItemIconUrl` composes it with the
new registry lookup as step 1 of the precedence.

`quality` is recorded metadata, not consumed by rendering logic in v1 — a `dedicated` and a
`generic` entry render through the identical `<img>` + fallback path. It exists so a later
asset-batch pass can enumerate every `approximate`/`generic` entry (e.g. "give armor real
per-weight icons") without re-deriving the matrix from the #576 issue thread.

**Validated assumption:** the real wire's `Ref.id` is the bare canonical id with no instance
suffix — `types.proto`'s own doc comment for `Item.ref` gives the example
`{module:"dnd5e", type:"item", id:"longsword"}`, and `Item` has no quantity field. (The concept
fixtures' `dagger-1`/`dagger-2` ids are a fixture-only convenience for demoing two independently-
equippable carried daggers in `EquippedMap`'s single-ref-per-slot shape — not a wire convention the
lookup needs to normalize.) This is consistent with `ideas/typed-ref-vocabulary/design.md`, which
records that `Item.ref` stays a generic `Ref` (module/type/id) for now rather than a typed
Weapon/Armor enum — the typed-`Item` rebase is tracked separately there and does not block this
lookup, which keys on the string id either way.

## Tier assignments

Verified against `rpg-game-assets` `origin/main` by the #576 issue comment (spot-checked
`ui/library/icons/weapons/` + `icons/inventory/`).

### Dedicated (matching sprite category exists)

| category         | canonical ids                                                |
| ---------------- | ------------------------------------------------------------ |
| Swords           | shortsword, longsword, greatsword, rapier, scimitar          |
| Axes             | handaxe, battleaxe, greataxe                                 |
| Dagger           | dagger                                                       |
| Mace/Morningstar | mace, morningstar                                            |
| Hammers          | light-hammer, warhammer, maul                                |
| Polearms/Spears  | spear, javelin, pike, glaive, halberd, lance, trident        |
| Staff            | quarterstaff                                                 |
| Bows             | shortbow, longbow                                            |
| War pick         | war-pick (the pack's "IcePick" sprite — genuinely apt shape) |
| Shield (armor)   | shield                                                       |

### Approximate (nearest existing category, no exact match)

`greatclub`, `sickle`, `flail`, `dart`, `hand-crossbow`, `light-crossbow`, `heavy-crossbow`.

### Generic (no plausible weapon-shaped sprite — neutral inventory icon)

`whip`, `sling`, `net`, `blowgun`, `club`, `unarmed-strike`.

`club` and `unarmed-strike` were not called out in the #576 issue comment's category accounting
(it enumerated `whip`/`sling`/`net`/`blowgun` as the "no plausible sprite" set) but both are
required canonical ids in the 38-entry enum and need a registry entry regardless. All six generic
ids use a **neutral generic inventory/items sprite**
(`icons/inventory/ICON_FantasyWarrior_Inventory_Items01_Clean.png`) — not a warning/error icon. The
tooltip and accessible name still show the real item name (see "Integration points" below), so a
generic icon never reads as "this is broken" — it reads as "a plain object icon labeled Club."

### Armor

`shield` is dedicated (shares the weapon-tier shield sprite). The other 12 armor ids share one
generic body-armor icon for v1: `icons/inventory/ICON_FantasyWarrior_Inventory_Armor01_Clean.png` —
the same path `fixtures.ts` already uses for `chain-mail`/`leather`. Per-weight armor icons are a
future asset-batch ask (#576 comment), not v1 scope.

**Reuse note for whoever implements:** eight canonical ids already have a path chosen in
`src/concepts/equipment/fixtures.ts` under that exact `ref.id` (`longsword`, `greatsword`,
`shield`, `chain-mail`, `handaxe`, `greataxe`, `shortsword`, `leather`) — reuse those exact paths in
`ITEM_ICONS` rather than re-picking a different numbered variant, so the concept bench and the live
registry agree pixel-for-pixel on items they share. The fixture's `dagger-1`/`dagger-2` items point
at `icons/inventory/ICON_FantasyWarrior_Inventory_Daggers01_Clean.png` — a reasonable pick for the
canonical `dagger` entry too, but note their `ref.id` is the fixture-only `dagger-1`/`dagger-2`, not
the canonical `dagger` (see the validated-assumption note above), so this is a "same sprite,
different id" reuse rather than a direct id match.

## Integration points (preserve existing behavior)

`EquipmentSlots.tsx` and `InventoryLight.tsx` already:

- Put the item's name and stat line into the enclosing `<button>`'s `aria-label` and `title` (e.g.
  `"${item.name} — ${item.statLine} (click to unequip)"`). This design does not change that text —
  every icon tier (dedicated/approximate/generic) sits behind the same accessible name.
- Render the `<img>` as decorative (`alt=""`), correct because the name is already conveyed by the
  enclosing button — no change needed here either.
- Hide the image on load failure (`onError` sets `display: none`) while the name/stat-line spans
  keep rendering — the existing graceful-degradation path. This design doesn't add new failure
  handling; a registry entry pointing at a since-renamed/missing file degrades exactly like an
  empty `resolveIconUrl` does today.
- Fall back to text-only for any item whose `getItemIconUrl` returns `undefined` — already tested
  for the empty-`iconKey` case in #575; this design extends the same code path to cover "known id,
  but not in either icon map" and "wire id present but unmapped" as the same outcome.

The only call-site change either component needs is swapping `resolveIconUrl(item.iconKey)` for
`getItemIconUrl(item.ref, item.iconKey)`.

## Asset serving verification (tracked separately from lookup correctness)

The #576 issue's first comment recorded a real discrepancy: the sprite files are confirmed present
in the deployed image (rpg-game-assets `origin/main` since 07-19; `docker.yml` logged "Synced 2061
Synty asset file(s)" on the last three prod builds), but Kirk did not observe icons in production.
Root cause is undetermined — a leading suspect is the Discord Activity proxy's path mapping
differing from a same-origin plain-browser request, but that is not confirmed.

This design's correctness does not depend on that being resolved first — `itemIcons.ts`'s unit
tests (pure string/lookup assertions) and a plain-browser check are sufficient to verify the lookup
itself. But "verify icons render" as a manual/QA step must check **both surfaces separately** and
report which one(s) fail, because they exercise different request paths:

1. **Plain browser tab** against the deployed host, hard-refreshed, on the `/concepts` equipment
   bench (same-origin request, no proxy).
2. **Inside the actual Discord Activity** (requests proxy through Discord's activity iframe
   mapping).

If (1) passes and (2) fails, that confirms a proxy/path-mapping gap and should be filed as its own
issue against the Discord activity setup — not folded into or blocking this lookup work, and not
treated as evidence the lookup itself is wrong.

## Tests

- **Exhaustiveness:** iterate the literal 38-weapon-id + 13-armor-id lists (mirroring the proto
  enums, not `Object.keys(ITEM_ICONS)`) and assert every one has an `ITEM_ICONS` entry — a missing
  id fails loudly.
- **Entry validity:** every entry in both `ITEM_ICONS` and `SUPPLEMENTAL_ITEM_ICONS` has a
  non-empty `path` and a `quality` in `'dedicated' | 'approximate' | 'generic'`.
- **Representative resolution, by tier:** `shortsword` and `greataxe` (dedicated), `greatclub`
  (approximate), `quarterstaff` (dedicated), `club` and `unarmed-strike` (generic) — each asserts
  the exact resolved path and quality.
- **Wire override:** an item with a non-empty `iconKey` resolves to that path even when `ref.id`
  also has a canonical entry (wire wins).
- **Supplemental:** an id present only in `SUPPLEMENTAL_ITEM_ICONS` resolves; an id in neither map
  (e.g. a homebrew/module id) returns `undefined`.
- **Unknown id:** a fully unmapped id returns `undefined` — text-only, never a broken image.
- **Component behavior unchanged:** existing `EquipmentSlots.test.tsx`/`InventoryLight.test.tsx`
  assertions (click emits the correct intent, disabled while `busy`, `aria-label`/`title` content)
  continue to pass with only the icon-source call swapped — the DOM/interaction contract does not
  change.

## Non-goals

- Bespoke new art or sprite commissioning (all icons come from the already-deployed library).
- Server-populated `icon_key` (rpg-api#691 — closed, superseded by this web-side lookup).
- Any game-rules behavior — this is presentation only.
- Drag/drop for equip/unequip interactions.
- A complete icon set for all gear (potions, tools, packs, ammunition) — only the 38+13
  enum-backed weapon/armor ids are exhaustive; non-enum gear gets, at most, a few opportunistic
  supplemental entries.

## Files (for whoever implements, all paths relative to rpg-dnd5e-web)

| File                                               | Change                                                      |
| --------------------------------------------------- | ------------------------------------------------------------ |
| `src/utils/itemIcons.ts` (new)                     | `ITEM_ICONS`, `SUPPLEMENTAL_ITEM_ICONS`, `getItemIconUrl`   |
| `src/utils/itemIcons.test.ts` (new)                | Tests per the "Tests" section above                         |
| `src/components/game/equipment/EquipmentSlots.tsx` | Swap `resolveIconUrl(item.iconKey)` → `getItemIconUrl(...)` |
| `src/components/game/equipment/InventoryLight.tsx` | Same swap                                                   |
| `docs/architecture/components/equipment.md`        | Note the new lookup + precedence once implemented           |

## Process note

Per the cross-repo design workflow (see rpg-project's `CLAUDE.md`), this design was mistakenly
committed directly to `rpg-dnd5e-web` `main` (local commit `20f1bda`) instead of landing here first.
It is reproduced here verbatim in substance as the canonical design; the `rpg-dnd5e-web` commit is
being moved to a local backup branch and local `main` reset to `origin/main` as part of the same
correction. Implementation happens in `rpg-dnd5e-web` against issue #576 once this design is
approved; this rpg-project PR gets the plan added after approval and stays open until that
implementation is complete.
