# Monster loot content — the layer #386/#387 deliberately parked

**Parent:** rpg-project#388 · **Initiative:** rpg-project#386 · **Depends on:** #387 (`monster.Data` gaining `Inventory`/`Wallet`,
`session.Loot` moving them — this design assumes those fields exist, doesn't touch the mechanism)

## 1. Sanity check before proposing anything — is there already a smarter path?

Checked before assuming "mirror the vendor package" was right:

- **`tools/selectables` is the correct tool, not a reach.** Its own package doc names *"treasure
  generation"* as an explicit goal. Verified zero current consumers anywhere in `rulebooks/dnd5e`
  or `monster` — it's built and ready, not reinvented here.
- **Mirroring `npcs` as a whole *package* is the wrong shape, though.** `npcs` earned its own
  package because it introduced a genuinely new member kind (`KindWorld`). Monster loot introduces
  no new kind — `monster.Data` already exists, confirmed as a pure stat block today (identity,
  HP/AC/abilities, speed, senses, actions) with nothing loot-shaped to conflict with. This content
  layer belongs *inside* `monster`, not as a sibling package. The **pattern** worth mirroring from
  `npcs` (hard-set first, generated/configured later) still holds — the package boundary doesn't.

## 2. The shape — same sequencing as vendor content, one layer later

**Step 1 — one hard-set example**, mirroring `npcs.NewMerchant(nil)`'s role: a helper (e.g.
`monster.WithLoot(data *Data, loot LootConfig) *Data`, or a config option on however monsters are
already constructed) that sets a fixed `Inventory`/`Wallet` on a specific monster instance. No
`selectables` yet. Proves `session.Loot` (from #387) actually moves real content end to end,
exactly the same proof-of-mechanism role the demo vendor played for `Interact`/`Trade`.

**Step 2 — category/range generation, `selectables`-driven**: a weighted table producing a gold
amount within a range, or selecting from a small item pool — the *"monster drops a random gp
amount within a range"* case named as the simplest real generation target. This is where
`selectables.NewBasicTable[...]` actually gets its first caller in this rulebook.

**Step 3 — full authored/configured loot tables** (per-monster-type tables, category expansion
the way vendor stock eventually wants "one of each martial weapon") — not designed here, named so
it isn't invented from scratch later without this context.

## 3. What this does NOT decide

- Whether every monster type needs authored loot, or whether unauthored monsters simply have
  `nil` `Inventory`/`Wallet` (no loot) — a content/game-design call, not answered here.
- Anything from #387's own out-of-scope list (monster AI consuming inventory mid-combat,
  equipment-driven monster variants) — unrelated to content authoring, still parked there.

## 4. Done when

One specific monster, authored with Step 1's hard-set helper, dies and is looted through
`session.Loot` (#387), landing real items/gold in the looter's `Inventory`/`Wallet` — the same
"prove the mechanism with one hardcoded case" bar the demo vendor cleared for `Trade`.
