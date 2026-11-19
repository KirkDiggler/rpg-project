# Sell — the mirror of Buy, no new verb

**Parent:** rpg-project#384 · **Origin:** rpg-project#376 · **Supersedes** the "Selling / barter" stub in `ideas/currency/design.md`
§5, which deliberately left this undesigned until it was its turn.

## 1. The core mechanism — one refusal lifted, nothing new invented

`session.Trade` already refuses any non-empty `Give.Items` (`ErrGiveNotSupported`). Sell lifts that
refusal for the mirror case and adds the symmetric validation/effect — no new verb, no new RPC,
no new proto message. `Give`/`Receive` already say everything a direction needs; sell is just the
other populated side of the same shape `Trade` was built with from the start.

**Validation** (one direction per call, still no barter this wave):
- `Give.Items` exactly one entry — the actor must actually **possess** it (an own-inventory check,
  the real difference from buy's vendor-stock check).
- `Receive.Currency` must exactly match the server-computed price (`equipment.PriceOf`, 1:1 — same
  law as buy, direction-flipped: a client asserting an inflated expected payout is the same risk
  class as asserting a discounted payment, so it gets the same `ErrWrongPrice` treatment).
- `Receive.Items` and `Give.Currency` both stay empty.

**Effect:**
1. Remove the item from the actor's own inventory — a new small primitive,
   `RemoveInventoryItem` (mirror of `AddInventoryItem`, doesn't exist yet).
2. Credit the actor's `Wallet.Add`.
3. The vendor's payout is checked against its own `Wallet` first — see §3.
4. The item goes into the vendor's stock — see §2. Not discarded.

**Beat:** `sold` (from the actor's perspective). `OutcomeSold` already exists in `encounter/
outcome.go`, built ahead of this design, with its own doc comment stating exactly why: *"'Alice
sold her longsword' is a different statement from 'Alice traded for one,' so it gets its own kind
rather than reusing `OutcomeTraded` with a flipped flag."*

**Rename `OutcomeTraded`→`OutcomeBought` while touching this area, for consistency the current
code doesn't quite have.** Today buy uses the generic `"traded"` and only sell got a specific
word — an artifact of buy shipping first, before the "different statement" reasoning existed to
apply consistently. The same reasoning that earned `sold` its own kind applies equally to buy:
"Alice bought a longsword" is a different, truer statement than "Alice traded for one." Low cost
to fix now (this project's own stance: pre-pre-alpha, breaking an outcome-kind string from
tonight is not a gate) and it's already the area of code this wave touches.

**`Bartered` is reserved, not built.** Barter (item-for-item) isn't in scope this wave (`Give.Items`
stays refused when `Receive.Items` is also populated), but when it lands, it earns its own kind
for the same reason — not because "traded" reads badly for it (it doesn't, of the three it fits
best), but for symmetry: every real transaction shape gets an honest word, none defaults to the
generic one. Explicitly NOT `Bartered`-for-NPC vs. some other name for player-to-player — a
player counterparty needs a consent mechanism and is almost certainly its own verb entirely, not
a naming variant of this one (see rpg-project#369/#370's original scoping). That verb, whenever
designed, earns its own outcome kind from its own design — not decided here, in the abstract,
ahead of it existing.

So: three real outcome kinds once this wave and the rename land — `Bought`, `Sold`, and
`Bartered` reserved for later. No case defaults to a bare `Traded` anymore.

## 2. Sold items become real vendor stock — this makes buyback free

Rather than vanishing, a sold item runs through a new mutation (mirror of the existing
`DecrementVendorStock`): **`AddToVendorStock`** — increment an existing `StockEntryData` if the
vendor already stocks that `Type`+`ID`, or append a new one (`StockModeLimited`, quantity = what
was sold) if it doesn't.

Once that's true, **buyback is not a new mechanic** — it's just `Trade` buying, against a stock
entry indistinguishable in the data model from authored stock. Same "generalize once, get the
related behavior for free" pattern as `Give`/`Receive` itself.

**UI differentiation marker**: `StockEntryData` gains `Source: authored | player_sold` (or a
simpler `PlayerSold bool`). Toolkit's job is only to carry the tag through faithfully — the actual
badge/color/pip treatment is entirely web's call, not decided here.

**Explicitly deferred, not decided against** (the author's own retraction, worth keeping visible
rather than losing): filtering "important" items a character shouldn't be able to sell (e.g. a
cleric's holy symbol). Real idea, not this wave — can layer on top of the mechanism above without
changing its shape.

## 3. The vendor gets an optional wallet — real enforcement, not a decorative field

`rpg-toolkit#1275` already named "vendor purse/gold limits" as an anticipated future feature —
this isn't invented here, it's the first wave that actually needs an answer for it, since Sell is
the first thing that moves gold *out of* a vendor.

```go
// npc.Data or npcs.VendorData
Wallet *currency.Money  // nil = unlimited buying power; set = enforced
```

Sell's validation: if the vendor's `Wallet` is set, check `CanAfford`/`Sub` the same way a
player's does, refuse (`ErrInsufficientFunds`, same sentinel buy already uses) if it can't cover
the payout; if `nil`, skip the check — unlimited. **Both branches are real and tested** — a vendor
with a set, exhausted purse refusing a sell is a genuine test case, even though every currently
authored vendor is `nil`. This is the difference between this and the tickets/tokens mistake
caught earlier: the check code exists and is exercised now; content simply doesn't set a limit
yet. Whoever eventually builds NPC content authoring (rpg-api#903 Phase 2) gets a real, working
knob for free, not a redesign.

## 4. Cross-repo — checked directly, this is NOT toolkit-only

Unlike `Money`/`Wallet`, `Sell` touches all four repos:

- **rpg-toolkit** — §1-3 above.
- **rpg-api-protos** — smaller than expected: `TradeOffer.items` (field 1) **already exists on
  the wire** — it was always structurally there for `Give` too, only ever server-refused. Zero new
  fields needed for the core transaction. The one real addition: `VendorStockEntry` gains a new
  `player_sold`/`source` field to carry the tag from §2 — doesn't exist today, checked directly.
- **rpg-api** — bump pins, thread the new tag through the `VendorStockEntry` converter, add an
  error-table row for a genuinely new sentinel: none of Buy's existing errors (`ErrOutOfStock`,
  `ErrWrongPrice`, `ErrInsufficientFunds`) cover "you don't possess this item to sell." The
  vendor-side insufficient-funds case reuses the *existing* `ErrInsufficientFunds` mapping —
  nothing new there.
- **rpg-dnd5e-web** — the real surprise: **no existing UI lets a player pick an item from their
  own inventory and sell it** — everything built so far (`InventoryLight`, `EquipmentPopover`) is
  equip/buy-oriented. This is real, non-trivial new UI, arguably bigger than the toolkit mechanism
  itself. Plus rendering the player-sold tag visually on vendor stock rows.

## 5. What's still open, deliberately not solved here

- **Price preview for the client.** Buy got this for free (`VendorStockEntry.price` already
  exists). Sell has no equivalent — nothing on the wire lets a client show "sell for 7gp" before
  confirming. Small, separate, web/protos-facing decision (likely: extend the character-data
  inventory projection with a computed sell price per item, mirroring `VendorStockEntry.price`) —
  doesn't block anything in this design.
- **Item-protection filtering** (§2).

## 6. Done when

A player can sell an item they possess for its exact 1:1 price, see it land in their `Wallet`, see
it appear in the vendor's stock (tagged `player_sold`), and buy it back through the ordinary buy
path with no special-casing. A vendor with a set, exhausted `Wallet` correctly refuses a sell it
can't afford; a vendor with `nil` (every current vendor) never hits that check at all.
