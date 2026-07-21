# Item Icon Coverage — Canonical Weapon/Armor Registry Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.
>
> **Plan location note:** this plan lives beside the approved design at
> `ideas/equipment/item-icons/design.md` in `rpg-project` (Kirk's explicit
> preference for this idea, per the Cross-Repo Design Workflow in
> `rpg-project/CLAUDE.md`), not under `docs/superpowers/plans/`.
> Implementation itself happens in **`rpg-dnd5e-web`**, on its own branch,
> against **rpg-dnd5e-web#576**.

**Goal:** Add a web-side, exhaustive canonical weapon (38) + armor (13) id
-> icon lookup (`src/utils/itemIcons.ts`) and wire it into `EquipmentSlots`/
`InventoryLight` so the live equipment popover shows an icon for every
canonical item even though the server's `icon_key` is empty today, while
preserving every existing accessibility/interaction contract.

**Architecture:** One new pure module (`getItemIconUrl`) implements a
3-step resolution precedence — non-empty wire `iconKey` (via the existing
`resolveIconUrl`) wins, else a literal 38+13 canonical map, else a small
non-exhaustive supplemental map, else `undefined` (text-only, never a
broken `<img>`). `EquipmentSlots.tsx`/`InventoryLight.tsx` each swap their
one `resolveIconUrl(item.iconKey)` call site for
`getItemIconUrl(item.ref, item.iconKey)` — no other change to either
component's rendering, disabled/`busy`, or intent-emission logic.

**Tech Stack:** TypeScript 5.8 (strict mode, `satisfies` for compile-time
exhaustiveness), Vitest + `@testing-library/react`, React 19 function
components, no new runtime dependencies.

## Global Constraints

- `npm run ci-check` MUST pass before every push (format, lint, typecheck,
  build, tests) — non-negotiable per `rpg-dnd5e-web/CLAUDE.md`.
- Never use `git commit --no-verify` or `git push --no-verify`.
- `ref.id` is used verbatim in every lookup — **no normalization**. Wire
  canonical ids are exact lowercase kebab-case (e.g. `war-pick`,
  `hand-crossbow`).
- `quality` (`IconQuality`) is recorded metadata only — **not rendered** in
  v1; no component reads `.quality`.
- `ITEM_ICONS`/`SUPPLEMENTAL_ITEM_ICONS` entry `path` values are
  **manifest-relative** (e.g. `icons/weapons/ICON_SM_Wep_Sword_02_Clean.png`)
  — `resolveIconUrl` (already in `equipmentTypes.ts`) is what prefixes
  `/models/synty/ui/library/`. `getItemIconUrl` must reuse `resolveIconUrl`
  for that prefixing, not re-hardcode the base path.
- The exhaustiveness test iterates the **literal** `CANONICAL_WEAPON_IDS`/
  `CANONICAL_ARMOR_IDS` arrays, never `Object.keys(ITEM_ICONS)` — a
  silently-missing key must fail loudly.
- `ITEM_ICONS`'s internal literal must be checked with
  `satisfies Record<CanonicalItemId, ItemIconEntry>` so a missing OR
  misspelled canonical key is a compile error; the exported `ITEM_ICONS`
  itself is widened to `Record<string, ItemIconEntry>` so callers can
  safely index by an arbitrary `ref.id` (guarded by a truthiness check at
  the call site, since `noUncheckedIndexedAccess` is off in this repo's
  `tsconfig.app.json`).
- Do **not** add the fixture-only ids `dagger-1`, `dagger-2`, or `torch`
  (from `src/concepts/equipment/fixtures.ts`) to
  `SUPPLEMENTAL_ITEM_ICONS` — their fixture data already carries a
  non-empty `iconKey`, so step 1 of the precedence (wire wins) already
  resolves them; they are not live wire canonical ids.
- Components import the resolver from `../../../utils/itemIcons` (three
  levels up from `src/components/game/equipment/`), never `../../utils`.
- Preserve exactly, in both components: decorative `alt=""` on every icon
  `<img>`, the enclosing `<button>`'s `title`/`aria-label` carrying the
  real item name, the existing `onError` handler that hides the image on
  load failure, and every existing click/disabled/`busy` interaction.
- One issue per PR, branch from latest `origin/main`, no direct commits to
  `main`. This is **player-facing web behavior** (rpg-project's
  `ideas/opencode-team-workflow/design.md` §4/§9) — it gets normal PR
  review plus **one independent Sol gate** before Kirk merges; Kirk alone
  decides whether to merge.
- Any GitHub issue/PR comment this work produces ends with
  `— asset-pipeline agent, on behalf of KirkDiggler` (repo convention, see
  `rpg-project`'s `asset-initiative-tracking` memory).

## File Responsibilities

| File | Responsibility |
| --- | --- |
| `src/utils/itemIcons.ts` (new) | Canonical weapon/armor icon registry: `IconQuality`, `ItemIconEntry`, `CANONICAL_WEAPON_IDS`/`CANONICAL_ARMOR_IDS` literal arrays, `CanonicalItemId` derived type, the exhaustive `ITEM_ICONS` map, the small non-exhaustive `SUPPLEMENTAL_ITEM_ICONS` map, and `getItemIconUrl` (the one function implementing the 3-step precedence). |
| `src/utils/itemIcons.test.ts` (new) | Exhaustiveness (38/13/51 counts, no overlap), entry validity (non-empty path, valid quality), tier-representative resolution, wire-override, supplemental resolution, unknown-id -> `undefined`. |
| `src/components/game/equipment/EquipmentSlots.tsx` (modify) | Swap its one icon call site from `resolveIconUrl(item.iconKey)` to `getItemIconUrl(item.ref, item.iconKey)`. No other change. |
| `src/components/game/equipment/EquipmentSlots.test.tsx` (modify) | Replace the now-invalid "empty icon_key" test (it used the canonical id `longsword`, which will now resolve) with an unknown-id version; add canonical-resolution and `onError`-fallback coverage. |
| `src/components/game/equipment/InventoryLight.tsx` (modify) | Same swap as `EquipmentSlots.tsx`, at its one icon call site. |
| `src/components/game/equipment/InventoryLight.test.tsx` (modify) | Add canonical-resolution coverage for a carried item (`greatsword`, empty `iconKey`). |
| `docs/architecture/components/equipment.md` (modify, after behavior lands) | Replace the stale "no item-id-to-icon mapping table lives in the web" scope-decision note with the new registry description; list `itemIcons.test.ts` under Tests. |

---

## Task 1: Canonical registry + resolver (`src/utils/itemIcons.ts`)

**Files:**
- Create: `src/utils/itemIcons.test.ts`
- Create: `src/utils/itemIcons.ts`

**Interfaces:**
- Consumes: `resolveIconUrl(iconKey: string): string | undefined` and the
  `/models/synty/ui/library` base it applies — both already exported from
  `src/components/game/equipment/equipmentTypes.ts` (unchanged).
- Produces (consumed by Task 2): `IconQuality`, `ItemIconEntry`,
  `CANONICAL_WEAPON_IDS`, `CANONICAL_ARMOR_IDS`, `CanonicalItemId`,
  `ITEM_ICONS: Record<string, ItemIconEntry>`,
  `SUPPLEMENTAL_ITEM_ICONS: Record<string, ItemIconEntry>`,
  `getItemIconUrl(ref: { id: string }, iconKey: string): string | undefined`
  — all exported from `src/utils/itemIcons.ts`.

- [ ] **Step 1: Set up the isolated workspace (folded into this task — no standalone setup task)**

Create a project-local worktree from latest `origin/main`, tied to
rpg-dnd5e-web#576 (this repo's `.gitignore` already has `.worktrees/` at
line 32, so it's the correct project-local location):

```bash
cd /home/kirk/game-dev/rpg-dnd5e-web
git fetch origin
git worktree add .worktrees/576-item-icons -b feat/576-item-icons origin/main
cd .worktrees/576-item-icons
npm install
```

Expected: `npm install` completes without error; `git status` in the new
worktree reports `On branch feat/576-item-icons`, clean.

Verify the baseline is green before writing anything:

```bash
npm run test:run
```

Expected: the full existing suite passes (0 failures) — this is the clean
baseline the rest of this task's steps diff against.

- [ ] **Step 2: Write the failing test file**

Create `src/utils/itemIcons.test.ts`:

```ts
import { describe, expect, it } from 'vitest';
import {
  CANONICAL_ARMOR_IDS,
  CANONICAL_WEAPON_IDS,
  getItemIconUrl,
  ITEM_ICONS,
  SUPPLEMENTAL_ITEM_ICONS,
} from './itemIcons';
import type { IconQuality } from './itemIcons';

const VALID_QUALITIES: IconQuality[] = ['dedicated', 'approximate', 'generic'];

describe('itemIcons', () => {
  describe('canonical id lists', () => {
    it('has exactly 38 canonical weapon ids', () => {
      expect(CANONICAL_WEAPON_IDS).toHaveLength(38);
    });

    it('has exactly 13 canonical armor ids', () => {
      expect(CANONICAL_ARMOR_IDS).toHaveLength(13);
    });

    it('has no id shared between the weapon and armor lists', () => {
      const weaponSet: Set<string> = new Set(CANONICAL_WEAPON_IDS);
      const overlap = CANONICAL_ARMOR_IDS.filter((id) => weaponSet.has(id));
      expect(overlap).toEqual([]);
    });
  });

  describe('ITEM_ICONS exhaustiveness', () => {
    it('has an entry for every canonical weapon id', () => {
      for (const id of CANONICAL_WEAPON_IDS) {
        expect(
          ITEM_ICONS[id],
          `missing ITEM_ICONS entry for "${id}"`
        ).toBeDefined();
      }
    });

    it('has an entry for every canonical armor id', () => {
      for (const id of CANONICAL_ARMOR_IDS) {
        expect(
          ITEM_ICONS[id],
          `missing ITEM_ICONS entry for "${id}"`
        ).toBeDefined();
      }
    });

    it('has exactly 51 entries — no extra ids beyond the 38+13 canonical set', () => {
      expect(Object.keys(ITEM_ICONS)).toHaveLength(38 + 13);
    });
  });

  describe('entry validity', () => {
    it('every ITEM_ICONS entry has a non-empty path and a valid quality', () => {
      for (const [id, entry] of Object.entries(ITEM_ICONS)) {
        expect(entry.path.length, `${id} has an empty path`).toBeGreaterThan(
          0
        );
        expect(
          VALID_QUALITIES,
          `${id} has an invalid quality "${entry.quality}"`
        ).toContain(entry.quality);
      }
    });

    it('every SUPPLEMENTAL_ITEM_ICONS entry has a non-empty path and a valid quality', () => {
      for (const [id, entry] of Object.entries(SUPPLEMENTAL_ITEM_ICONS)) {
        expect(entry.path.length, `${id} has an empty path`).toBeGreaterThan(
          0
        );
        expect(
          VALID_QUALITIES,
          `${id} has an invalid quality "${entry.quality}"`
        ).toContain(entry.quality);
      }
    });
  });

  describe('getItemIconUrl resolution', () => {
    it('resolves a dedicated weapon (shortsword) by canonical id with an empty icon_key', () => {
      expect(getItemIconUrl({ id: 'shortsword' }, '')).toBe(
        '/models/synty/ui/library/icons/weapons/ICON_SM_Wep_Sword_02_Clean.png'
      );
    });

    it('resolves a dedicated weapon (greataxe) by canonical id with an empty icon_key', () => {
      expect(getItemIconUrl({ id: 'greataxe' }, '')).toBe(
        '/models/synty/ui/library/icons/inventory/ICON_FantasyWarrior_Inventory_Axes01_Clean.png'
      );
    });

    it('resolves a dedicated weapon (quarterstaff) by canonical id with an empty icon_key', () => {
      expect(getItemIconUrl({ id: 'quarterstaff' }, '')).toBe(
        '/models/synty/ui/library/icons/weapons/ICON_SM_Wep_Staff_01_Clean.png'
      );
    });

    it('resolves an approximate weapon (greatclub) by canonical id', () => {
      expect(getItemIconUrl({ id: 'greatclub' }, '')).toBe(
        '/models/synty/ui/library/icons/weapons/ICON_SM_Wep_Staff_02_Clean.png'
      );
    });

    it('resolves an approximate weapon (heavy-crossbow) by canonical id', () => {
      expect(getItemIconUrl({ id: 'heavy-crossbow' }, '')).toBe(
        '/models/synty/ui/library/icons/inventory/ICON_FantasyWarrior_Inventory_Bows01_Clean.png'
      );
    });

    it('resolves a generic weapon (club) by canonical id', () => {
      expect(getItemIconUrl({ id: 'club' }, '')).toBe(
        '/models/synty/ui/library/icons/inventory/ICON_FantasyWarrior_Inventory_Items01_Clean.png'
      );
    });

    it('resolves a generic weapon (unarmed-strike) by canonical id', () => {
      expect(getItemIconUrl({ id: 'unarmed-strike' }, '')).toBe(
        '/models/synty/ui/library/icons/inventory/ICON_FantasyWarrior_Inventory_Items01_Clean.png'
      );
    });

    it('resolves the dedicated armor id (shield)', () => {
      expect(getItemIconUrl({ id: 'shield' }, '')).toBe(
        '/models/synty/ui/library/icons/weapons/ICON_SM_Wep_Shield_01_Clean.png'
      );
    });

    it('resolves a generic body-armor id (leather)', () => {
      expect(getItemIconUrl({ id: 'leather' }, '')).toBe(
        '/models/synty/ui/library/icons/inventory/ICON_FantasyWarrior_Inventory_Armor01_Clean.png'
      );
    });

    it('wire icon_key wins over a canonical id match', () => {
      expect(
        getItemIconUrl({ id: 'longsword' }, 'icons/weapons/custom-override.png')
      ).toBe('/models/synty/ui/library/icons/weapons/custom-override.png');
    });

    it('resolves a supplemental id (bolts-20) not in the canonical map', () => {
      expect(getItemIconUrl({ id: 'bolts-20' }, '')).toBe(
        '/models/synty/ui/library/icons/inventory/ICON_FantasyWarrior_Inventory_Arrows01_Clean.png'
      );
    });

    it('resolves a supplemental id (dungeoneer-pack) not in the canonical map', () => {
      expect(getItemIconUrl({ id: 'dungeoneer-pack' }, '')).toBe(
        '/models/synty/ui/library/icons/inventory/ICON_FantasyWarrior_Inventory_Backpack01_Clean.png'
      );
    });

    it('returns undefined for an unknown homebrew id with an empty icon_key', () => {
      expect(getItemIconUrl({ id: 'homebrew-relic' }, '')).toBeUndefined();
    });
  });
});
```

- [ ] **Step 3: Run the test to verify it fails**

Run: `npm run test:run -- src/utils/itemIcons.test.ts`

Expected: FAIL — Vitest cannot resolve the import; the whole file errors
before any test runs, e.g. `Error: Failed to resolve import "./itemIcons"
from "src/utils/itemIcons.test.ts". Does the file exist?`

- [ ] **Step 4: Implement the registry + resolver**

Create `src/utils/itemIcons.ts`:

```ts
/**
 * Canonical weapon/armor icon lookup for the equipment popover
 * (`EquipmentSlots.tsx`/`InventoryLight.tsx`, rpg-dnd5e-web#576), following
 * the same web-owned-map + text-fallback precedent as
 * `src/utils/actionIcons.ts` (#497) and `src/utils/conditionIcons.ts`
 * (#473). Server-authored `Item.iconKey` is always empty today
 * (rpg-api#680 scope-decision) — this registry closes that gap for the
 * 38 `Weapon` + 13 `Armor` enum ids without any server/toolkit change.
 * See rpg-project's `ideas/equipment/item-icons/design.md` for the full
 * design (coverage boundary, resolution precedence, tier rationale).
 *
 * Resolution precedence, implemented by `getItemIconUrl`:
 *   1. Wire wins — a non-empty `iconKey` resolves via the existing
 *      `resolveIconUrl` (equipmentTypes.ts), unconditionally.
 *   2. Canonical — `ref.id` looked up in `ITEM_ICONS` (exhaustive over
 *      the 38 weapon + 13 armor canonical ids).
 *   3. Supplemental — `ref.id` looked up in `SUPPLEMENTAL_ITEM_ICONS`
 *      (non-exhaustive extra map for ammo/pack ids with no wire enum).
 *   4. Text-only — `undefined`. Callers already render text-only for an
 *      `undefined` icon URL (see `resolveIconUrl`'s empty-key case) —
 *      never a broken `<img>`.
 *
 * `ref.id` is used verbatim, no normalization — the wire's canonical ids
 * are exact lowercase kebab-case (`types.proto`'s `Ref` doc comment gives
 * `{module:"dnd5e", type:"item", id:"longsword"}`).
 */

import { resolveIconUrl } from '../components/game/equipment/equipmentTypes';

/** Icon-tier metadata — recorded for a future asset-batch pass, not
 *  consumed by rendering logic in v1 (design doc "Tier assignments"). */
export type IconQuality = 'dedicated' | 'approximate' | 'generic';

export interface ItemIconEntry {
  /** Manifest-relative path under the Synty UI library base, e.g.
   *  "icons/weapons/ICON_SM_Wep_Sword_02_Clean.png" — the same base
   *  `resolveIconUrl` resolves an `iconKey` against. */
  path: string;
  quality: IconQuality;
}

/** The 38 canonical `Weapon` enum ids (dnd5e/api/v1alpha2/weapons/
 *  weapons.proto), excluding WEAPON_UNSPECIFIED and the simple/martial/
 *  any-weapon category placeholders (not concrete weapons — excluded from
 *  the generated enum too). Alphabetical; this exact literal array is what
 *  the exhaustiveness test iterates — NOT `Object.keys(ITEM_ICONS)` — so a
 *  silently-missing key fails loudly instead of the test just seeing a
 *  smaller-but-still-"complete" map. */
export const CANONICAL_WEAPON_IDS = [
  'battleaxe',
  'blowgun',
  'club',
  'dagger',
  'dart',
  'flail',
  'glaive',
  'greataxe',
  'greatclub',
  'greatsword',
  'halberd',
  'hand-crossbow',
  'handaxe',
  'heavy-crossbow',
  'javelin',
  'lance',
  'light-crossbow',
  'light-hammer',
  'longbow',
  'longsword',
  'mace',
  'maul',
  'morningstar',
  'net',
  'pike',
  'quarterstaff',
  'rapier',
  'scimitar',
  'shortbow',
  'shortsword',
  'sickle',
  'sling',
  'spear',
  'trident',
  'unarmed-strike',
  'war-pick',
  'warhammer',
  'whip',
] as const;

/** The 13 canonical `Armor` enum ids (dnd5e/api/v1alpha2/armor/armor.proto),
 *  excluding ARMOR_UNSPECIFIED. Alphabetical, same exhaustiveness-test role
 *  as `CANONICAL_WEAPON_IDS`. */
export const CANONICAL_ARMOR_IDS = [
  'breastplate',
  'chain-mail',
  'chain-shirt',
  'half-plate',
  'hide',
  'leather',
  'padded',
  'plate',
  'ring-mail',
  'scale-mail',
  'shield',
  'splint',
  'studded-leather',
] as const;

/** Union of every canonical weapon + armor id. `CANONICAL_ITEM_ICONS`
 *  below is `satisfies`-checked against `Record<CanonicalItemId,
 *  ItemIconEntry>`, so adding a new proto enum value here without a
 *  registry entry (or misspelling one) is a compile error, not a silent
 *  gap. */
export type CanonicalItemId =
  | (typeof CANONICAL_WEAPON_IDS)[number]
  | (typeof CANONICAL_ARMOR_IDS)[number];

/** Internal literal, `satisfies`-checked against `Record<CanonicalItemId,
 *  ItemIconEntry>` so TypeScript rejects a missing OR misspelled canonical
 *  key at compile time (an extra key not in `CanonicalItemId` is also
 *  rejected — this must be exactly the 38+13 ids, no more, no less).
 *  Exported below widened to `Record<string, ItemIconEntry>` so callers
 *  can safely index by an arbitrary `ref.id` without a TS error; the
 *  runtime lookup in `getItemIconUrl` still guards with a truthiness
 *  check rather than trusting the widened type. */
const CANONICAL_ITEM_ICONS = {
  battleaxe: {
    path: 'icons/weapons/ICON_SM_Wep_Axe_03_Clean.png',
    quality: 'dedicated',
  },
  blowgun: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Items01_Clean.png',
    quality: 'generic',
  },
  club: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Items01_Clean.png',
    quality: 'generic',
  },
  dagger: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Daggers01_Clean.png',
    quality: 'dedicated',
  },
  dart: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Arrows01_Clean.png',
    quality: 'approximate',
  },
  flail: {
    path: 'icons/weapons/ICON_SM_Wep_Mace_05_Clean.png',
    quality: 'approximate',
  },
  glaive: {
    path: 'icons/weapons/ICON_SM_Wep_Spear_06_Clean.png',
    quality: 'dedicated',
  },
  greataxe: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Axes01_Clean.png',
    quality: 'dedicated',
  },
  greatclub: {
    path: 'icons/weapons/ICON_SM_Wep_Staff_02_Clean.png',
    quality: 'approximate',
  },
  greatsword: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Swords01_Clean.png',
    quality: 'dedicated',
  },
  halberd: {
    path: 'icons/weapons/ICON_SM_Wep_Spear_08_Clean.png',
    quality: 'dedicated',
  },
  'hand-crossbow': {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Bows01_Clean.png',
    quality: 'approximate',
  },
  handaxe: {
    path: 'icons/weapons/ICON_SM_Wep_Axe_04_Clean.png',
    quality: 'dedicated',
  },
  'heavy-crossbow': {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Bows01_Clean.png',
    quality: 'approximate',
  },
  javelin: {
    path: 'icons/weapons/ICON_SM_Wep_Spear_02_Clean.png',
    quality: 'dedicated',
  },
  lance: {
    path: 'icons/weapons/ICON_SM_Wep_Spear_09_Clean.png',
    quality: 'dedicated',
  },
  'light-crossbow': {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Bows01_Clean.png',
    quality: 'approximate',
  },
  'light-hammer': {
    path: 'icons/weapons/ICON_SM_Wep_Hammer_01_Clean.png',
    quality: 'dedicated',
  },
  longbow: {
    path: 'icons/weapons/ICON_SM_Prop_Bow_02_Clean.png',
    quality: 'dedicated',
  },
  longsword: {
    path: 'icons/weapons/ICON_SM_Wep_Sword_02_Clean.png',
    quality: 'dedicated',
  },
  mace: {
    path: 'icons/weapons/ICON_SM_Wep_Mace_01_Clean.png',
    quality: 'dedicated',
  },
  maul: {
    path: 'icons/weapons/ICON_SM_Wep_Hammer_04_Clean.png',
    quality: 'dedicated',
  },
  morningstar: {
    path: 'icons/weapons/ICON_SM_Wep_Mace_03_Clean.png',
    quality: 'dedicated',
  },
  net: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Items01_Clean.png',
    quality: 'generic',
  },
  pike: {
    path: 'icons/weapons/ICON_SM_Wep_Spear_04_Clean.png',
    quality: 'dedicated',
  },
  quarterstaff: {
    path: 'icons/weapons/ICON_SM_Wep_Staff_01_Clean.png',
    quality: 'dedicated',
  },
  rapier: {
    path: 'icons/weapons/ICON_SM_Wep_Sword_01_Clean.png',
    quality: 'dedicated',
  },
  scimitar: {
    path: 'icons/weapons/ICON_SM_Wep_Sword_03_Clean.png',
    quality: 'dedicated',
  },
  shortbow: {
    path: 'icons/weapons/ICON_SM_Prop_Bow_01_Clean.png',
    quality: 'dedicated',
  },
  shortsword: {
    path: 'icons/weapons/ICON_SM_Wep_Sword_02_Clean.png',
    quality: 'dedicated',
  },
  sickle: {
    path: 'icons/weapons/ICON_SM_Wep_Knife_03_Clean.png',
    quality: 'approximate',
  },
  sling: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Items01_Clean.png',
    quality: 'generic',
  },
  spear: {
    path: 'icons/weapons/ICON_SM_Wep_Spear_01_Clean.png',
    quality: 'dedicated',
  },
  trident: {
    path: 'icons/weapons/ICON_SM_Wep_Spear_11_Clean.png',
    quality: 'dedicated',
  },
  'unarmed-strike': {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Items01_Clean.png',
    quality: 'generic',
  },
  'war-pick': {
    path: 'icons/weapons/ICON_SM_Wep_IcePick_01_Clean.png',
    quality: 'dedicated',
  },
  warhammer: {
    path: 'icons/weapons/ICON_SM_Wep_Hammer_02_Clean.png',
    quality: 'dedicated',
  },
  whip: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Items01_Clean.png',
    quality: 'generic',
  },
  breastplate: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Armor01_Clean.png',
    quality: 'generic',
  },
  'chain-mail': {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Armor01_Clean.png',
    quality: 'generic',
  },
  'chain-shirt': {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Armor01_Clean.png',
    quality: 'generic',
  },
  'half-plate': {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Armor01_Clean.png',
    quality: 'generic',
  },
  hide: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Armor01_Clean.png',
    quality: 'generic',
  },
  leather: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Armor01_Clean.png',
    quality: 'generic',
  },
  padded: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Armor01_Clean.png',
    quality: 'generic',
  },
  plate: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Armor01_Clean.png',
    quality: 'generic',
  },
  'ring-mail': {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Armor01_Clean.png',
    quality: 'generic',
  },
  'scale-mail': {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Armor01_Clean.png',
    quality: 'generic',
  },
  shield: {
    path: 'icons/weapons/ICON_SM_Wep_Shield_01_Clean.png',
    quality: 'dedicated',
  },
  splint: {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Armor01_Clean.png',
    quality: 'generic',
  },
  'studded-leather': {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Armor01_Clean.png',
    quality: 'generic',
  },
} satisfies Record<CanonicalItemId, ItemIconEntry>;

/** Exhaustive over the 38 weapon + 13 armor canonical ids (51 total) — see
 *  `CANONICAL_ITEM_ICONS`'s `satisfies` check above for the compile-time
 *  guarantee. Widened to `Record<string, ItemIconEntry>` so
 *  `getItemIconUrl` can index by an arbitrary `ref.id` string. */
export const ITEM_ICONS: Record<string, ItemIconEntry> = CANONICAL_ITEM_ICONS;

/** Small, intentionally non-exhaustive. Non-enum gear (ammo, packs, tools)
 *  that happens to have a reasonable icon and a real toolkit id — never
 *  checked for completeness, absence here is normal and expected. Do not
 *  add fixture-only ids here (e.g. `dagger-1`/`dagger-2`/`torch` from
 *  `src/concepts/equipment/fixtures.ts`) — those already carry a
 *  non-empty `iconKey` in their fixture data, so step 1 of the precedence
 *  (wire wins) already resolves them; they are not live wire canonical
 *  ids. */
export const SUPPLEMENTAL_ITEM_ICONS: Record<string, ItemIconEntry> = {
  'bolts-20': {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Arrows01_Clean.png',
    quality: 'generic',
  },
  'bolts-50': {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Arrows01_Clean.png',
    quality: 'generic',
  },
  'dungeoneer-pack': {
    path: 'icons/inventory/ICON_FantasyWarrior_Inventory_Backpack01_Clean.png',
    quality: 'generic',
  },
};

/**
 * Resolve the icon URL for an owned item, implementing the 3-step
 * precedence (wire -> canonical -> supplemental -> text-only). `ref`
 * carries `id` only (module/type unused here — id is unique within the
 * weapon/armor/item id space this registry covers). `ref.id` is used
 * verbatim, no normalization.
 *
 * @example
 * ```typescript
 * getItemIconUrl({ id: 'longsword' }, '');
 * // '/models/synty/ui/library/icons/weapons/ICON_SM_Wep_Sword_02_Clean.png'
 * getItemIconUrl({ id: 'longsword' }, 'icons/weapons/custom.png');
 * // '/models/synty/ui/library/icons/weapons/custom.png' — wire wins
 * getItemIconUrl({ id: 'homebrew-relic' }, '');
 * // undefined — unknown id, caller falls back to text-only
 * ```
 */
export function getItemIconUrl(
  ref: { id: string },
  iconKey: string
): string | undefined {
  const wireUrl = resolveIconUrl(iconKey);
  if (wireUrl) return wireUrl;

  const canonicalEntry = ITEM_ICONS[ref.id];
  if (canonicalEntry) return resolveIconUrl(canonicalEntry.path);

  const supplementalEntry = SUPPLEMENTAL_ITEM_ICONS[ref.id];
  if (supplementalEntry) return resolveIconUrl(supplementalEntry.path);

  return undefined;
}
```

- [ ] **Step 5: Run the test to verify it passes**

Run: `npm run test:run -- src/utils/itemIcons.test.ts`

Expected: PASS — `Tests  21 passed (21)`.

- [ ] **Step 6: Typecheck in isolation**

Run: `npm run typecheck`

Expected: no errors. This is the step that actually exercises the
`satisfies Record<CanonicalItemId, ItemIconEntry>` compile-time
exhaustiveness check — if you deliberately delete one canonical entry
(e.g. comment out `whip:`) and rerun, `npm run typecheck` must fail with a
"Property 'whip' is missing" style error; put the entry back before
continuing.

- [ ] **Step 7: Commit**

```bash
git add src/utils/itemIcons.ts src/utils/itemIcons.test.ts
git commit -m "feat(equipment): canonical weapon/armor icon registry + resolver (#576)"
```

---

## Task 2: Wire the resolver into the two components

**Files:**
- Modify: `src/components/game/equipment/EquipmentSlots.tsx`
- Modify: `src/components/game/equipment/EquipmentSlots.test.tsx`
- Modify: `src/components/game/equipment/InventoryLight.tsx`
- Modify: `src/components/game/equipment/InventoryLight.test.tsx`

**Interfaces:**
- Consumes: `getItemIconUrl(ref: { id: string }, iconKey: string): string |
  undefined` from `../../../utils/itemIcons` (Task 1).
- Produces: no new exports — both components' public props
  (`EquipmentSlotsProps`/`InventoryLightProps`) are unchanged.

- [ ] **Step 1: Replace `EquipmentSlots.test.tsx` with the updated suite**

The current file's fourth test ("skips the `<img>` entirely for an empty
icon_key") uses the canonical id `longsword`, which will resolve to a
real icon once Task 2 lands — that assertion becomes false, not a
regression. Replace the whole file with:

```tsx
import { fireEvent, render, screen } from '@testing-library/react';
import { describe, expect, it, vi } from 'vitest';
import { EquipmentSlots } from './EquipmentSlots';
import type { EquippedMap, ItemLike, SlotDefLike } from './equipmentTypes';

const SLOTS: SlotDefLike[] = [
  { key: 'main_hand', displayLabel: 'Main hand', accepts: ['weapon'] },
  { key: 'off_hand', displayLabel: 'Off hand', accepts: ['weapon', 'shield'] },
  { key: 'armor', displayLabel: 'Armor', accepts: ['armor'] },
];

const ITEMS: ItemLike[] = [
  {
    ref: { module: 'dnd5e', type: 'item', id: 'longsword' },
    name: 'Longsword',
    statLine: '1d8 slashing · versatile',
    iconKey: 'icons/weapons/longsword.png',
    kind: 'weapon',
    slotKeys: ['main_hand', 'off_hand'],
  },
];

describe('EquipmentSlots', () => {
  it('renders an empty hint for a socket with no Ref in `equipped`', () => {
    render(
      <EquipmentSlots
        slots={SLOTS}
        equipped={{}}
        items={ITEMS}
        onIntent={vi.fn()}
      />
    );
    const socket = screen.getByTestId('equip-socket-main_hand');
    expect(socket.textContent).toContain('— empty —');
    expect((socket as HTMLButtonElement).disabled).toBe(true);
  });

  it('renders the equipped item looked up by Ref.id from `items`', () => {
    const equipped: EquippedMap = {
      main_hand: { module: 'dnd5e', type: 'item', id: 'longsword' },
    };
    render(
      <EquipmentSlots
        slots={SLOTS}
        equipped={equipped}
        items={ITEMS}
        onIntent={vi.fn()}
      />
    );
    const socket = screen.getByTestId('equip-socket-main_hand');
    expect(socket.textContent).toContain('Longsword');
    expect(socket.textContent).toContain('1d8 slashing · versatile');
    expect((socket as HTMLButtonElement).disabled).toBe(false);
  });

  it('emits an UnequipItem intent when an occupied socket is clicked', () => {
    const onIntent = vi.fn();
    const equipped: EquippedMap = {
      main_hand: { module: 'dnd5e', type: 'item', id: 'longsword' },
    };
    render(
      <EquipmentSlots
        slots={SLOTS}
        equipped={equipped}
        items={ITEMS}
        onIntent={onIntent}
      />
    );
    fireEvent.click(screen.getByTestId('equip-socket-main_hand'));
    expect(onIntent).toHaveBeenCalledWith({
      kind: 'UnequipItem',
      slotKey: 'main_hand',
    });
  });

  it('skips the <img> entirely for an unknown id with an empty icon_key (never a broken image, rpg-dnd5e-web#576)', () => {
    const unknownItems: ItemLike[] = [
      {
        ...ITEMS[0],
        ref: { module: 'dnd5e', type: 'item', id: 'homebrew-relic' },
        iconKey: '',
      },
    ];
    const equipped: EquippedMap = {
      main_hand: { module: 'dnd5e', type: 'item', id: 'homebrew-relic' },
    };
    render(
      <EquipmentSlots
        slots={SLOTS}
        equipped={equipped}
        items={unknownItems}
        onIntent={vi.fn()}
      />
    );
    expect(
      screen.getByTestId('equip-socket-main_hand').querySelector('img')
    ).toBeNull();
  });

  it('resolves a canonical icon for a known id even when icon_key is empty (rpg-dnd5e-web#576)', () => {
    const canonicalItems: ItemLike[] = [{ ...ITEMS[0], iconKey: '' }];
    const equipped: EquippedMap = {
      main_hand: { module: 'dnd5e', type: 'item', id: 'longsword' },
    };
    render(
      <EquipmentSlots
        slots={SLOTS}
        equipped={equipped}
        items={canonicalItems}
        onIntent={vi.fn()}
      />
    );
    const socket = screen.getByTestId('equip-socket-main_hand');
    const img = socket.querySelector('img');
    expect(img).not.toBeNull();
    expect(img?.getAttribute('src')).toBe(
      '/models/synty/ui/library/icons/weapons/ICON_SM_Wep_Sword_02_Clean.png'
    );
    expect(img?.getAttribute('alt')).toBe('');
    expect(socket.getAttribute('title')).toContain('Longsword');
    expect(socket.getAttribute('aria-label')).toContain('Longsword');
  });

  it('hides the icon on image load failure while the item name stays visible (onError fallback)', () => {
    const equipped: EquippedMap = {
      main_hand: { module: 'dnd5e', type: 'item', id: 'longsword' },
    };
    render(
      <EquipmentSlots
        slots={SLOTS}
        equipped={equipped}
        items={ITEMS}
        onIntent={vi.fn()}
      />
    );
    const socket = screen.getByTestId('equip-socket-main_hand');
    const img = socket.querySelector('img') as HTMLImageElement;
    fireEvent.error(img);
    expect(img.style.display).toBe('none');
    expect(socket.textContent).toContain('Longsword');
  });

  it('disables every socket while `busy`', () => {
    const equipped: EquippedMap = {
      main_hand: { module: 'dnd5e', type: 'item', id: 'longsword' },
    };
    render(
      <EquipmentSlots
        slots={SLOTS}
        equipped={equipped}
        items={ITEMS}
        onIntent={vi.fn()}
        busy
      />
    );
    expect(
      (screen.getByTestId('equip-socket-main_hand') as HTMLButtonElement)
        .disabled
    ).toBe(true);
  });
});
```

- [ ] **Step 2: Replace `InventoryLight.test.tsx` with the updated suite**

Replace the whole file with:

```tsx
import { fireEvent, render, screen } from '@testing-library/react';
import { describe, expect, it, vi } from 'vitest';
import { InventoryLight } from './InventoryLight';
import type { EquippedMap, ItemLike, SlotDefLike } from './equipmentTypes';
import { refKey } from './equipmentTypes';

const SLOTS: SlotDefLike[] = [
  { key: 'main_hand', displayLabel: 'Main hand', accepts: ['weapon'] },
  { key: 'off_hand', displayLabel: 'Off hand', accepts: ['weapon', 'shield'] },
  { key: 'armor', displayLabel: 'Armor', accepts: ['armor'] },
];

/** Matches InventoryLight's `data-testid={inv-${refKey(item.ref)}}`. */
const invTestId = (id: string) =>
  `inv-${refKey({ module: 'dnd5e', type: 'item', id })}`;

const ITEMS: ItemLike[] = [
  {
    ref: { module: 'dnd5e', type: 'item', id: 'longsword' },
    name: 'Longsword',
    statLine: '1d8 slashing · versatile',
    iconKey: '',
    kind: 'weapon',
    slotKeys: ['main_hand', 'off_hand'],
  },
  {
    ref: { module: 'dnd5e', type: 'item', id: 'greatsword' },
    name: 'Greatsword',
    statLine: '2d6 slashing · two-handed',
    iconKey: '',
    kind: 'weapon',
    slotKeys: ['main_hand'],
  },
  {
    ref: { module: 'dnd5e', type: 'item', id: 'torch' },
    name: 'Torch',
    statLine: 'light, 20 ft radius',
    iconKey: '',
    kind: 'gear',
    slotKeys: [],
  },
];

describe('InventoryLight', () => {
  it('lists only items NOT referenced by `equipped`', () => {
    const equipped: EquippedMap = {
      main_hand: { module: 'dnd5e', type: 'item', id: 'longsword' },
    };
    render(
      <InventoryLight
        slots={SLOTS}
        equipped={equipped}
        items={ITEMS}
        onIntent={vi.fn()}
      />
    );
    expect(screen.queryByTestId(invTestId('longsword'))).toBeNull();
    expect(screen.getByTestId(invTestId('greatsword'))).toBeTruthy();
    expect(screen.getByTestId(invTestId('torch'))).toBeTruthy();
  });

  it('shows the empty-carried message when everything is equipped', () => {
    const equipped: EquippedMap = {
      main_hand: { module: 'dnd5e', type: 'item', id: 'longsword' },
      off_hand: { module: 'dnd5e', type: 'item', id: 'greatsword' },
    };
    render(
      <InventoryLight
        slots={SLOTS}
        equipped={equipped}
        items={ITEMS.slice(0, 2)}
        onIntent={vi.fn()}
      />
    );
    expect(screen.getByText('Nothing carried.')).toBeTruthy();
  });

  it('renders slotless gear unclickable, with a "gear" badge', () => {
    render(
      <InventoryLight
        slots={SLOTS}
        equipped={{}}
        items={ITEMS}
        onIntent={vi.fn()}
      />
    );
    const torchRow = screen.getByTestId(invTestId('torch'));
    expect((torchRow as HTMLButtonElement).disabled).toBe(true);
    expect(torchRow.textContent).toContain('gear');
  });

  it('emits an EquipItem intent targeting the first compatible slot on click', () => {
    const onIntent = vi.fn();
    render(
      <InventoryLight
        slots={SLOTS}
        equipped={{}}
        items={ITEMS}
        onIntent={onIntent}
      />
    );
    fireEvent.click(screen.getByTestId(invTestId('longsword')));
    expect(onIntent).toHaveBeenCalledWith({
      kind: 'EquipItem',
      ref: { module: 'dnd5e', type: 'item', id: 'longsword' },
      slotKey: 'main_hand',
    });
  });

  it('targets the first compatible slot as a swap when nothing is empty', () => {
    const onIntent = vi.fn();
    const equipped: EquippedMap = {
      main_hand: { module: 'dnd5e', type: 'item', id: 'longsword' },
      off_hand: { module: 'dnd5e', type: 'item', id: 'longsword' },
    };
    render(
      <InventoryLight
        slots={SLOTS}
        equipped={equipped}
        items={ITEMS}
        onIntent={onIntent}
      />
    );
    fireEvent.click(screen.getByTestId(invTestId('greatsword')));
    expect(onIntent).toHaveBeenCalledWith({
      kind: 'EquipItem',
      ref: { module: 'dnd5e', type: 'item', id: 'greatsword' },
      slotKey: 'main_hand',
    });
  });

  it('resolves a canonical icon for a carried item when icon_key is empty (rpg-dnd5e-web#576)', () => {
    const equipped: EquippedMap = {
      main_hand: { module: 'dnd5e', type: 'item', id: 'longsword' },
    };
    render(
      <InventoryLight
        slots={SLOTS}
        equipped={equipped}
        items={ITEMS}
        onIntent={vi.fn()}
      />
    );
    const row = screen.getByTestId(invTestId('greatsword'));
    const img = row.querySelector('img');
    expect(img).not.toBeNull();
    expect(img?.getAttribute('src')).toBe(
      '/models/synty/ui/library/icons/inventory/ICON_FantasyWarrior_Inventory_Swords01_Clean.png'
    );
    expect(img?.getAttribute('alt')).toBe('');
    expect(row.getAttribute('title')).toContain('Greatsword');
    expect(row.getAttribute('aria-label')).toContain('Greatsword');
  });

  it('disables every row while `busy`', () => {
    render(
      <InventoryLight
        slots={SLOTS}
        equipped={{}}
        items={ITEMS}
        onIntent={vi.fn()}
        busy
      />
    );
    expect(
      (screen.getByTestId(invTestId('longsword')) as HTMLButtonElement)
        .disabled
    ).toBe(true);
  });
});
```

- [ ] **Step 3: Run both test files to verify they fail exactly where expected**

Run: `npm run test:run -- src/components/game/equipment/EquipmentSlots.test.tsx src/components/game/equipment/InventoryLight.test.tsx`

Expected: `Test Files  2 failed (2)`, `Tests  2 failed | 12 passed (14)` —
specifically:
- `EquipmentSlots.test.tsx`: only "resolves a canonical icon for a known
  id even when icon_key is empty" fails (`expected null not to be null` —
  the current component still calls `resolveIconUrl('')`, which is
  `undefined` regardless of `ref.id`). The other 6 tests pass unchanged,
  including the new "unknown id" and `onError` tests — neither depends on
  the component swap, since an unmapped id and a non-empty wire `iconKey`
  both behave identically before and after Task 2.
- `InventoryLight.test.tsx`: only "resolves a canonical icon for a carried
  item when icon_key is empty" fails, same reason. The other 6 pass.

- [ ] **Step 4: Swap the icon call site in `EquipmentSlots.tsx`**

In `src/components/game/equipment/EquipmentSlots.tsx`, replace the import
line:

```ts
import { refKey, resolveIconUrl } from './equipmentTypes';
```

with:

```ts
import { refKey } from './equipmentTypes';
import { getItemIconUrl } from '../../../utils/itemIcons';
```

and replace the icon-resolution line:

```ts
const iconUrl = item ? resolveIconUrl(item.iconKey) : undefined;
```

with:

```ts
const iconUrl = item ? getItemIconUrl(item.ref, item.iconKey) : undefined;
```

No other line in this file changes — the `<img>` JSX, `onError` handler,
`title`/`aria-label`, and `disabled`/`busy` logic are untouched.

- [ ] **Step 5: Swap the icon call site in `InventoryLight.tsx`**

In `src/components/game/equipment/InventoryLight.tsx`, replace the import
line:

```ts
import { refKey, resolveIconUrl, targetSlotFor } from './equipmentTypes';
```

with:

```ts
import { refKey, targetSlotFor } from './equipmentTypes';
import { getItemIconUrl } from '../../../utils/itemIcons';
```

and replace the icon-resolution line:

```ts
const iconUrl = resolveIconUrl(item.iconKey);
```

with:

```ts
const iconUrl = getItemIconUrl(item.ref, item.iconKey);
```

No other line in this file changes.

- [ ] **Step 6: Run both test files to verify they pass**

Run: `npm run test:run -- src/components/game/equipment/EquipmentSlots.test.tsx src/components/game/equipment/InventoryLight.test.tsx`

Expected: PASS — `Test Files  2 passed (2)`, `Tests  14 passed (14)`.

- [ ] **Step 7: Typecheck**

Run: `npm run typecheck`

Expected: no errors — confirms `equipmentTypes.ts` no longer needs to
export `resolveIconUrl` to these two components (it's still exported for
`itemIcons.ts` and `src/concepts/equipment/fixtures.ts`, both unaffected)
and that the new `../../../utils/itemIcons` import path resolves from both
component files.

- [ ] **Step 8: Commit**

```bash
git add src/components/game/equipment/EquipmentSlots.tsx \
  src/components/game/equipment/EquipmentSlots.test.tsx \
  src/components/game/equipment/InventoryLight.tsx \
  src/components/game/equipment/InventoryLight.test.tsx
git commit -m "feat(equipment): resolve canonical item icons in EquipmentSlots/InventoryLight (#576)"
```

---

## Task 3: Docs, full CI, visual evidence, and PR flow

**Files:**
- Modify: `docs/architecture/components/equipment.md`

**Interfaces:**
- Consumes: nothing new — this task verifies and documents Tasks 1-2's
  already-landed behavior; no further code changes.

- [ ] **Step 1: Update the stale "no icon mapping table" scope decision**

In `docs/architecture/components/equipment.md`, under `## Scope
decisions`, replace this bullet:

```markdown
- **Icons degrade gracefully, by necessity.** The server sends `icon_key`
  empty today (rpg-api#680 Scope-decision: no toolkit/asset-manifest
  source exists yet for a bare sprite key) — verified against a live
  server during the #571 playtest. Every icon render is conditional
  (`resolveIconUrl` returns `undefined` for an empty key) plus an
  `onError` fallback for a key that resolves but 404s. No item-id-to-icon
  mapping table lives in the web; when the manifest gap closes, the
  server sending real keys is the only change needed.
```

with:

```markdown
- **Icons degrade gracefully, by necessity.** The server sends `icon_key`
  empty today (rpg-api#680 Scope-decision: no toolkit/asset-manifest
  source exists yet for a bare sprite key) — verified against a live
  server during the #571 playtest. `src/utils/itemIcons.ts` (rpg-dnd5e-web
  #576) closes most of that gap web-side: `getItemIconUrl(ref, iconKey)`
  resolves a non-empty wire `iconKey` first (unchanged `resolveIconUrl`
  behavior — a future manifest key always wins), else looks `ref.id` up
  in the exhaustive 38-weapon+13-armor `ITEM_ICONS` map, then the small
  non-exhaustive `SUPPLEMENTAL_ITEM_ICONS` map (ammo/pack ids with no wire
  enum), else returns `undefined`. Every icon render stays conditional on
  that `undefined` (never a broken `<img>`) plus the existing `onError`
  fallback for a path that resolves but 404s. See rpg-project's
  `ideas/equipment/item-icons/design.md` for the full coverage boundary
  and tier rationale (dedicated/approximate/generic — recorded metadata,
  not rendered in v1).
```

Also update the frontmatter `updated:` field at the top of the file to
today's date (the date this step is actually executed).

- [ ] **Step 2: Add `itemIcons.test.ts` to the Tests section**

Under `## Tests`, add a bullet (keep the existing bullets unchanged):

```markdown
- `src/utils/itemIcons.test.ts` — exhaustiveness over the 38+13 canonical
  ids, entry validity, tier-representative resolution, wire override,
  supplemental resolution, unknown-id fallback.
```

- [ ] **Step 3: Run the full CI check**

Run: `npm run ci-check`

Expected: all steps pass — `✓ Format check passed`, lint clean, typecheck
clean, build succeeds, full test suite green (no regressions in any file
outside the four touched in Tasks 1-2).

- [ ] **Step 4: Sync local assets before visual verification**

A fresh worktree does not carry the gitignored `public/models/synty/`
tree populated by earlier syncs. `scripts/sync-synty-assets.sh` derives
its private-asset checkout location as `$WEB_ROOT/../rpg-game-assets` —
inside this worktree, `$WEB_ROOT` is
`.../rpg-dnd5e-web/.worktrees/576-item-icons`, so that resolves to
`.../rpg-dnd5e-web/.worktrees/rpg-game-assets`, a location that does not
exist yet, not the real checkout already sitting at
`/home/kirk/game-dev/rpg-game-assets`. Left alone, `npm run assets:sync`
would clone a second, redundant copy of the private repo into
`.worktrees/`. From the worktree root, symlink around that instead:

```bash
[ -e ../rpg-game-assets ] || ln -s /home/kirk/game-dev/rpg-game-assets ../rpg-game-assets
```

Expected: no output (the `[ -e ... ] ||` guard makes this idempotent — a
rerun after the symlink already exists is a silent no-op, not a
duplicate-link error). This makes the sync script reuse the existing
private checkout instead of cloning a duplicate one; it does not copy or
duplicate any Synty asset data itself.

Now sync:

```bash
npm run assets:sync
```

Expected: clones/pulls `rpg-game-assets` (now resolved via the symlink)
and copies `harness/models/synty/` into `public/models/synty/`. Confirm
the specific file this registry's dedicated-shortsword/longsword entries
reference is present:

```bash
test -f public/models/synty/ui/library/icons/weapons/ICON_SM_Wep_Sword_02_Clean.png
```

Expected: exit status `0` (no output either way — `test` is silent on
success; run `echo $?` immediately after if you want to see the `0`
explicitly, but do not add an `&& echo` to the command itself).

- [ ] **Step 5: Supporting visual check — plain browser, `/concepts` equipment bench**

This is a fast, backend-free sanity check, not the required evidence —
Step 6 is mandatory and is what actually verifies the shipped behavior.
Start the dev server:

```bash
npm run dev
```

Using the chrome-devtools MCP tools (or equivalent), navigate to
`http://localhost:5173/concepts`, open the equipment bench, and capture a
screenshot. Confirm: Sir Aldric's longsword/greatsword/shield/chain-mail/
handaxe all show icons (dedicated/generic tiers, previously blank), and
Remy's dagger rows (fixture-only ids, unaffected by this change) still
show their existing fixture icon.

If no icons appear here, that is a real regression in this change — stop
and debug before proceeding to Step 6; do not attribute it to the known
production-sync discrepancy (that discrepancy is specifically about the
Discord Activity proxy path, checked separately in Step 7, not this
same-origin dev-server check).

- [ ] **Step 6: MANDATORY visual verification — the real local game route**

`/concepts` (Step 5) is fixture-driven and never exercises the actual
wire path (`CharacterData` -> `characterEquipmentFrom` ->
`EquipmentPopover`, see `docs/architecture/components/equipment.md`'s
"Live wiring" section). This step is the one that actually proves the
shipped behavior and is **not optional** — it must be completed and pass
before this PR is opened or submitted to gate.

Per `rpg-api`'s `docs/how-to/run-locally.md`, the server needs
`AUTH_DEV_MODE=true` (`make run` does not set this — it is a plain
`go run cmd/server/*.go server` with no dev-auth env var, so a web client
sending the `Authorization: Dev <playerId>` header would be rejected).
The web client also talks gRPC-Web over HTTP, not raw gRPC — `rpg-api`'s
`docker-compose.yml` documents an `envoy` service that proxies HTTP 8080
(what TypeScript clients call) to gRPC 50051 (what the bare Go server
listens on); starting only the server on 50051 leaves the web client with
nothing to connect to on 8080.

Start the stack, in order, from an `rpg-api` checkout:

```bash
cd /home/kirk/game-dev/rpg-api

# 1. Redis — idempotent: start the container if it already exists
#    (stopped) from a prior run, else create it fresh, per
#    docs/how-to/run-locally.md.
docker start rpg-redis 2>/dev/null || docker run -d --name rpg-redis -p 6379:6379 redis:alpine
```

```bash
# 2. Envoy — gRPC-Web proxy, HTTP 8080 -> gRPC 50051 (host networking,
#    per docker-compose.yml's own comment on the envoy service).
cd /home/kirk/game-dev/rpg-api
docker compose up -d envoy
```

```bash
# 3. The rpg-api gRPC server itself, in its own dedicated FOREGROUND
#    terminal (not backgrounded — leave it running and watch its logs
#    while testing) — AUTH_DEV_MODE=true is required, exactly per
#    docs/how-to/run-locally.md:
cd /home/kirk/game-dev/rpg-api
AUTH_DEV_MODE=true go run ./cmd/server server
```

Expected: the server logs it's listening on `:50051` and stays running in
this terminal (Ctrl+C stops it — leave it up for the rest of this step).

```bash
# 4. Seed the equip-demo fixture (rpg-dnd5e-web#571's aldric: a fighter
#    with longsword+shield+chain-mail equipped, a spare greatsword
#    carried) into the default "dev-encounter" encounter, from a
#    separate terminal, same rpg-api checkout:
cd /home/kirk/game-dev/rpg-api
go run ./cmd/devseed --fixture=equip-demo
```

Expected: the seed command exits 0 with no error; it writes to the
Redis instance from step 1 (`localhost:6379`, matching the devseed's
default).

In the `rpg-dnd5e-web` worktree, create `.env.local` (already gitignored
— `.gitignore` lines 25-27 — never commit it) with exactly:

```
VITE_API_HOST=http://localhost:8080
VITE_DEV_PLAYER_ID=aldric
```

Then start (or restart, if it was already running from Step 5, since
Vite only reads `.env.local` at startup) the dev server:

```bash
npm run dev
```

Navigate to exactly:

```
http://localhost:5173/?encounterId=dev-encounter&playerId=aldric
```

Both the `encounterId` and `playerId` query params are required here,
and they are **not** interchangeable with the `.env.local` values above:
`PlaytestHarness.tsx` reads `playerId` strictly from
`window.location.search` (`new URLSearchParams(...).get('playerId')`) and
derives `entityId` from it directly (`` `char-${playerId}` `` — matching
the devseed's `entityAldric = "char-aldric"` exactly for `playerId=aldric`).
Omitting `&playerId=aldric` from the URL makes the harness render its own
guard — "Error: playerId is required — add ?playerId=alice to the URL" —
regardless of `.env.local`. `useDevPlayerIdAuth(playerId)` then syncs that
URL value into the gRPC auth store before any request fires. `.env.local`'s
`VITE_DEV_PLAYER_ID` is a separate, harness-independent fallback that
`src/api/auth.ts`'s general dev-auth interceptor uses for OTHER routes
when no URL override exists at all — it does not satisfy or bypass
`PlaytestHarness`'s own explicit query-param requirement, so it is kept
here only for consistency with the rest of a normal local-dev setup, not
because this specific harness route needs it.

Confirm the live encounter loads as `aldric`. Open the equipment popover
(the chestplate chip on `EncounterDock`) and, using the chrome-devtools
MCP tools (or equivalent), capture a screenshot showing: `EquipmentSlots`
rendering icons for the equipped longsword/shield/chain-mail, and
`InventoryLight` rendering an icon for the carried greatsword — all
sourced through the real wire path, not fixtures. If a pre-change
baseline is convenient (e.g. a second worktree checked out at
`origin/main` running the same seeded encounter), capture a "before"
screenshot showing no icons for comparison; a "post-change only"
screenshot proving icons now render is sufficient evidence on its own if
a separate before-checkout isn't convenient.

**If this local stack cannot be started or the seeded encounter cannot be
reached in this environment** (missing Docker access, `rpg-api` checkout
unavailable, network policy, etc.): stop here. Report a verification
blocker describing exactly what failed and at which command above. Do
**not** claim this task's product work complete, and do **not** proceed
to open the PR or submit anything to the Step 11 Sol gate on the strength
of Step 5's `/concepts` check alone — that check is supporting evidence
only, not a substitute for this step.

- [ ] **Step 7: Visual verification — inside the Discord Activity, separately**

Per `docs/architecture/components/discord.md`, the Discord Activity
sandbox routes every API/asset call through `/.proxy` under
`discordsays.com` — a different request path than the same-origin checks
in Steps 5-6. Launch the activity through Discord exactly as done for
prior verified work in this repo, open the equipment popover on a
character with equipped canonical items, and capture a screenshot.

Two outcomes, each with its own PR-body command in Step 9 below:

- **Outcome A — both surfaces show icons:** proceed to Step 8.
- **Outcome B — Step 6's real local game route shows icons but the
  Discord Activity does not:** this confirms the proxy/path-mapping gap
  the design doc already flagged as unconfirmed. File a **separate**
  issue (exact command in Step 9's Outcome B) against the Discord
  activity/proxy setup describing the discrepancy. Do **not** expand
  #576's scope to fix it, and do not block this PR on it.

- [ ] **Step 8: Commit the docs change**

```bash
git add docs/architecture/components/equipment.md
git commit -m "docs(equipment): note the canonical item-icon lookup (#576)"
```

- [ ] **Step 9: Push and open the PR**

Push first (shared by both outcomes):

```bash
git push -u origin feat/576-item-icons
```

**If Step 7's Outcome A** (both surfaces passed):

```bash
gh pr create --repo KirkDiggler/rpg-dnd5e-web \
  --base main --head feat/576-item-icons \
  --title "feat(equipment): canonical weapon/armor item icons (#576)" \
  --body "Closes #576.

Adds src/utils/itemIcons.ts — an exhaustive 38-weapon+13-armor canonical icon registry plus getItemIconUrl(ref, iconKey), implementing the wire > canonical > supplemental > text-only precedence — and wires it into EquipmentSlots.tsx/InventoryLight.tsx, replacing their direct resolveIconUrl(item.iconKey) call with getItemIconUrl(item.ref, item.iconKey). No other rendering/interaction/accessibility behavior changes.

Design: rpg-project#111 (ideas/equipment/item-icons/design.md + plan.md).

Verification: full local ci-check green. Visual verification passed on the real local game route (rpg-api devseed equip-demo fixture, dev-encounter, aldric) and inside the Discord Activity — no proxy/path discrepancy found for this change. Screenshots will be attached in a signed PR comment immediately after this PR opens.

— asset-pipeline agent, on behalf of KirkDiggler"
```

**If Step 7's Outcome B** (Discord-only proxy gap): first file the
separate issue and capture its URL, then reference that URL in the PR
body:

```bash
PROXY_ISSUE_URL=$(gh issue create --repo KirkDiggler/rpg-dnd5e-web \
  --title "Discord Activity proxy does not serve canonical item icons that resolve correctly in plain browser (#576 follow-up)" \
  --body "While verifying rpg-dnd5e-web#576 (canonical weapon/armor item icons), the same equipment popover rendered icons correctly on the real local game route in a plain browser (http://localhost:5173/?encounterId=dev-encounter&playerId=aldric, same-origin, no proxy) but did not render them inside the Discord Activity (discordsays.com sandbox, requests routed through /.proxy — see docs/architecture/components/discord.md). This points at a proxy/path-mapping gap for /models/synty/ui/library/** asset requests, not a lookup-correctness bug in itemIcons.ts. Filed separately per #576's scope decision — not folded into or blocking that PR.

— asset-pipeline agent, on behalf of KirkDiggler")
echo "$PROXY_ISSUE_URL"

# rpg-project's board rules (CLAUDE.md: "No issue without a board entry")
# require every new issue land on the same project #576 already tracks
# ("The Dungeon Run", board 19):
gh project item-add 19 --owner KirkDiggler --url "$PROXY_ISSUE_URL"

gh pr create --repo KirkDiggler/rpg-dnd5e-web \
  --base main --head feat/576-item-icons \
  --title "feat(equipment): canonical weapon/armor item icons (#576)" \
  --body "Closes #576.

Adds src/utils/itemIcons.ts — an exhaustive 38-weapon+13-armor canonical icon registry plus getItemIconUrl(ref, iconKey), implementing the wire > canonical > supplemental > text-only precedence — and wires it into EquipmentSlots.tsx/InventoryLight.tsx, replacing their direct resolveIconUrl(item.iconKey) call with getItemIconUrl(item.ref, item.iconKey). No other rendering/interaction/accessibility behavior changes.

Design: rpg-project#111 (ideas/equipment/item-icons/design.md + plan.md).

Verification: full local ci-check green. Visual verification passed on the real local game route (rpg-api devseed equip-demo fixture, dev-encounter, aldric) in a plain browser. Icons did not render inside the Discord Activity — filed separately as $PROXY_ISSUE_URL, added to board 19 (proxy/path-mapping gap, not a lookup-correctness bug; not blocking this PR per #576's scope decision). Screenshots (real-game-route pass and Discord-Activity fail) will be attached in a signed PR comment immediately after this PR opens.

— asset-pipeline agent, on behalf of KirkDiggler"
```

Expected (either outcome): PR opens against `main`, CI runs green
(mirrors the local `ci-check` from Step 3).

- [ ] **Step 10: Attach the evidence screenshots, then independently verify each one**

Do not invent or paste a screenshot URL into the PR body — GitHub only
produces a real one once an image is actually uploaded through its UI.

1. Open the new PR in a browser (the URL `gh pr create` printed).
2. Add a new PR comment. Attach every screenshot captured above —
   Step 6's mandatory real-local-game-route screenshot at minimum, plus
   Step 5's supporting `/concepts` screenshot and Step 7's Discord
   Activity screenshot (pass or fail-state, whichever occurred) — via
   GitHub's comment image-attachment control (drag-and-drop or the
   attach-files button; GitHub hosts each as a
   `user-images.githubusercontent.com` (or `github.com/user-attachments`)
   URL once uploaded).
3. End the comment with a one-line caption per attachment identifying
   which step/surface it's evidence for, then the signature footnote:
   `— asset-pipeline agent, on behalf of KirkDiggler`.
4. Post the comment.
5. **Independently verify:** reload the PR page (a fresh page load, not
   the same in-memory view used to compose the comment) and open each
   attached image individually. Confirm each one actually renders the
   claimed content — do not just trust that the upload succeeded.
6. Record a written viewed-statement for each attachment, either as a
   follow-up line in the same comment or a second comment, e.g.: "Viewed
   equip-demo-local.png: aldric's equipment popover on
   `?encounterId=dev-encounter&playerId=aldric`, longsword/shield/chain-mail
   icons all rendering via the real wire path." One such statement per
   attachment.

- [ ] **Step 11: Address automated review, then the independent Sol gate**

Per the repo's standing review process:

1. Acknowledge every Copilot review comment individually (not in bulk) —
   address as needed, reply to each thread.
2. Once code review is clear, an **independent** reviewer (never the
   implementer, in its own fresh worktree — not `.worktrees/576-item-icons`)
   performs the adversarial Sol gate: reruns the full suite, audits the
   38/13 exhaustiveness and path claims against the actual file contents,
   independently opens and views each screenshot attached in Step 10
   (not just reads the viewed-statements), and posts a GATE REVIEW
   comment (`MERGE-READY` or findings-before-merge).
3. If the gate reports findings, this task's original implementer
   remediates them and the same gate reviewer performs a focused recheck
   — do not restart a full audit unless remediation materially rewrites
   scope.
4. **Kirk merges.** No agent merges this PR. Once merged, `#576` closes
   automatically (via the `Closes #576` PR body).

- [ ] **Step 12: Do not merge rpg-project PR #111 yet**

Per `ideas/equipment/item-icons/design.md`'s process note and
`rpg-project/CLAUDE.md`'s Cross-Repo Design Workflow, PR #111 (this
design+plan) stays **open** as the tracking surface until the
`rpg-dnd5e-web` PR from Step 9 is merged. Once it is, comment on PR #111
linking the merged `rpg-dnd5e-web` PR and let Kirk decide when to merge
#111 — do not merge it as part of this task.
