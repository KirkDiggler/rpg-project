# Loot gains currency (and items) — monster gets the other half of a split character already has

**Parent:** rpg-project#386 · **Initiative:** rpg-project#310 · **Origin:** rpg-project#376

**Scope, settled**: this design covers only §2 below — giving `monster.Data` the fields and
`session.Loot` the mechanism to move them. It deliberately says nothing about how a monster comes
to have loot in the first place (§3/§4) — that's a real, separate, later conversation, not an
open question left dangling inside this one.

## 1. What exists today, verified directly

`encounter.Loot` moves everything a downed member holds to the looter via one dispatch routine,
`transferHoldings` (`encounter/loot.go:196`) — today two kinds, intel and a takeable prop
reference. **Zero currency or item-inventory concept anywhere in `encounter/loot.go`,
`session/loot.go`, or `monster.Data`** (all verified directly, grepped).

**The pattern this should follow already exists for players, just not for monsters.** A player
character already keeps two completely separate constructs: real `Inventory`/`Wallet`
(`character.Data`, session-persisted, catalog-backed, sellable) and, entirely separately,
**holdings/intel** (encounter-internal — a player who searches or is told about a door holds that
knowledge through the same `holdings`/`intel` system a monster's knowledge lives in). Monsters
today only have the second half (they can hold and be looted for intel) — they have no
`Inventory`/`Wallet` at all. This design is filling in the missing half of a split that already
works for players, not inventing a new one.

## 2. Current shape (unsettled) — direct transfer, not a new holding kind

Earlier drafts of this doc modeled currency as a third arm in `transferHoldings` (encounter-layer
plumbing, seeded at monster placement). **Current thinking has moved away from that**, in favor of:

- `monster.Data` gains real `Inventory []InventoryItemData` and `Wallet currency.Money`, the exact
  same fields and shapes `character.Data` already has.
- `session.Loot` does two things in one call: calls `encounter.Loot` for holdings (intel/props,
  completely unchanged), and — separately — checks the target's `monster.Data` for
  `Inventory`/`Wallet` and transfers those directly using the same primitives `Trade` already
  needs: `AddInventoryItem` per item, `Wallet.Add` for gold, saved the same way
  `saveCharacterRecord` already saves a player's sheet.
- This also resolves item-loot (a captain carrying a real, later-sellable sword) for free, using
  the same mechanism as gold — no need to teach the `prop`/holdings model to understand equipment
  identity, which was the awkward part of the earlier draft.
- The P3 privacy law (every `looted` beat looks identical regardless of contents) still holds —
  that law is about the beat's shape, which `session.Loot` controls regardless of which internal
  mechanism did the transferring underneath it.

## 3. What's deliberately not decided here — content, not mechanism

How much gold or which items a given monster carries is authoring/content work, entirely separate
from the mechanism above, same distinction as "17 backgrounds' worth of gold values" being
separate from the wiring that reads them. Monsters have stats, attacks, and CR today — zero loot
data, ever; this is a genuinely new content layer, not a field to populate. The right sequencing,
by analogy to how vendor stock (`npcs`) was built: one hard-set example first (mirroring
`npcs.NewMerchant(nil)`), proving `session.Loot` actually moves it, before building any
`tools/selectables`-driven generated/configured loot-table system. **Its own future design, not
this one.**

## 4. Explicitly out of scope

- **Any pricing/value question** on the loot side — gold is credited directly via `Wallet.Add`, no
  `PriceOf` involved; items are added directly via `AddInventoryItem`. Nothing is being bought or
  sold here (selling a looted item later is just `Sell`, unrelated to this design).
- Vaults, chests, and other non-monster lootable containers — not addressed by this design at all;
  whether they'd reuse `monster.Data`'s new shape, need their own, or something else entirely is a
  separate question this doc doesn't answer.
- **The monster-loot content layer itself** (§3) — real, wanted, explicitly a later conversation.
- **Monster AI consuming its own `Inventory` mid-combat** (a monster using a potion as an action,
  the unused remainder becoming lootable on death) — a genuinely exciting idea raised while
  designing this, but it touches monster combat/`behavior`/`TurnDriver`, not `Loot`. Named here so
  it isn't lost, not decided or scoped.
- **Equipment-driven monster variants** (a ranged vs. melee bandit differing only by what's
  equipped) — same treatment: named, not scoped, not this design's job. Monster combat resolution
  reads a fixed stat block today, not equipped items the way a player's attacks do; teaching it
  otherwise is a separate, bigger conversation.

## 5. Done when (also unsettled — revisit once §2 firms up)

Killing a monster authored with gold and/or items, then looting its body, credits the looter's
`Wallet` and/or adds the items to their `Inventory`, saved durably; a monster authored with
neither changes nothing and produces byte-identical beats to any other empty loot (P3 preserved).
