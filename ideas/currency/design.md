# The `currency` package: `Money` first, nothing else yet

**Parent:** rpg-project#376 · **Origin:** rpg-toolkit#1275 (Vendor and NPC Inventory
Foundation) — this design narrows and
supersedes that issue's `Money`/`Wallet` sections specifically; its `ItemRef`/`StockEntry`/
`Availability` sections are already built and shipped (`rulebooks/dnd5e/npcs`, the `Interact`/
`Trade` waves). **Follows:** rpg-project#369/#370 (`Trade`), which deliberately shipped with no
`Currency` field on `TradeOffer` — this is that field's prerequisite.

## 1. What exists today, verified

- Every priced item already carries a cost: `equipment.Detail.Cost string` (`detail.go`), e.g.
  `"15 gp"`, `"1 sp"`, `"4 cp"`. Checked every `Cost:` literal across
  `rulebooks/dnd5e/weapons`, `armor`, and the rest of the equipment data: **uniformly
  `<int> <denomination>`, single denomination, no compounds** (`"1 gp 5 sp"` does not occur
  anywhere). That's the exact shape `ParseCost` needs to handle — no more, no less.
- Nothing resembling `Money`, a wallet, or currency arithmetic exists anywhere in the toolkit.
  This is genuinely new, not a rename of something half-built.

## 2. Why a package, not a type on its own

`currency` is the domain ("things a vendor can be paid in"); `Money` is its first concrete type
(D&D coinage). If a second currency kind is ever a *real* requirement — not invented here — it
lands as a sibling type in the same package, not a reshape of `Money` into something generic.
Nothing forces every type in the package to share a method set: `Money` gets `ParseCost` and a
denomination `Breakdown` because coinage divides; a hypothetical future type with no
denominations would simply not have those methods. No shared interface is extracted ahead of a
second concrete type existing to extract it *from* — guessing that shape now, with no second
currency to check it against, is the same mistake `Give`/`Receive` avoided by waiting until
buy/sell/barter/gift were concretely on the table together before generalizing.

## 3. The shape — `Money`, this wave's only type

```go
package currency

// Money is a normalized D&D 5e currency amount, stored as copper -- the
// smallest denomination -- so arithmetic never reconciles mixed units.
type Money struct {
    Copper int `json:"copper"`
}

// SRD conversion: 1 pp = 10 gp = 100 sp = 1,000 cp; 1 ep = 5 sp = 50 cp.
func FromCopper(cp int) Money
func FromSilver(sp int) Money
func FromElectrum(ep int) Money
func FromGold(gp int) Money
func FromPlatinum(pp int) Money

// ParseCost parses a single-denomination equipment cost string ("15 gp",
// "4 cp") into Money. Every Cost string in the current equipment data is
// this shape (verified, §1) -- a compound input is refused as a data
// defect, not silently misparsed.
func ParseCost(cost string) (Money, error)

func (m Money) Add(other Money) Money
func (m Money) Sub(other Money) (Money, error) // error rather than negative
func (m Money) CanAfford(cost Money) bool

// Breakdown is the canonical coin-purse split, largest denomination first,
// each remainder carried down -- what a display actually wants (e.g. 1247
// copper -> 1 pp, 2 gp, 4 sp, 7 cp), not five independent "entirely in this
// one unit" conversions rpg-toolkit#1275's original sketch proposed.
type Breakdown struct{ Platinum, Gold, Electrum, Silver, Copper int }
func (m Money) Breakdown() Breakdown
```

Display formatting (spacing, which denominations to show, localization) stays web's job — the
toolkit gives it the split, not a formatted string.

## 4. Scope for this wave — deliberately just this

`Money` ships alone: parse, arithmetic, afford-check, breakdown. Fully standalone, fully testable
with **zero consumers** — nothing references it yet, same as `character.AddInventoryItem` before
`Trade` existed to call it.

**Explicitly not this wave, each its own follow-on:**
- **`Wallet`** — a `Wallet Money` field on `character.Data`, beside `Inventory`, not embedded in
  it. Ruled here, built next: currency needs arithmetic and a single normalized afford-check value
  a stack of items was never built to give you (rpg-toolkit#1275's own words: *"Character gold
  should not be modeled as a regular inventory item for purchasing power"*). Needs `Money` to
  exist first, so it cannot land in the same PR.
- **`TradeOffer.Currency`** — the field `Trade` deliberately left out (rpg-project#370 §3: *"No
  Currency field yet... inventing Money here, unused, would be designing for a requirement this
  slice doesn't have"*). Additive once `Wallet` exists.
- **`Quote`/`Buy`** — rpg-toolkit#1275's pricing/affordability flow. Needs `Wallet` first.
- **Any currency besides `Money`** (tickets, tokens, faction credits) — no concrete vendor needs
  one; see §2.
- **Vendor purse/gold limits** — rpg-toolkit#1275's own non-goal, unchanged.

## 5. The full path beyond this wave

This wave ships `Money` alone (§4). The rest of the road to "a player actually pays for
something" is laid out here so it doesn't need re-deriving later — nothing below is built by
this PR, and each wave is its own design/issue when its turn comes.

**Wave 2 — `PriceOf` + catalog completeness** (toolkit, same module as `Money`, no cross-repo
touch):
- `equipment.PriceOf(id shared.EquipmentID) (currency.Money, error)` — wraps the existing
  `equipment.ResolveEquipmentDetail` + `currency.ParseCost`. Lives in `equipment`, not
  `currency`: `currency` stays dependency-free (a bare string in, `Money` out), `equipment`
  already knows how to resolve full item details and gains a `Money`-returning sibling to
  `ResolveEquipmentDetail`.
- `rulebooks/dnd5e/items.All` is missing entries for most of its own declared adventuring-gear
  constants — `Torch`, `HempenRope`, `Rations`, `Bedroll`, `Waterskin`, and 9 others have `ItemID`
  constants (referenced by starting-pack contents) but no `Name`/`Weight`/`Cost` in `All`, so
  `ResolveEquipmentDetail` returns `nil` for any of them today. **Required, not optional**: every
  character starts with pack-granted gear (a torch, rope, rations from a chosen starting pack)
  that must be sellable, and a pack-granted item carries the exact same `InventoryItemData{Type,
  ID, Quantity}` identity as a bought or looted one (verified — no "came from a pack" flag exists
  anywhere), so pricing must work for it regardless of how it entered inventory. Populate the 14
  missing entries with real PHB cost/weight.

**Wave 3 — `Wallet`** (toolkit; likely also a wire touch, decide when this wave starts):
- `character.Data.Wallet Money`, beside `Inventory`, not embedded in it (rpg-toolkit#1275: *"gold
  should not be modeled as a regular inventory item"* — arithmetic and a single afford-check value
  a stack of items was never built to give).
- **Open question for that wave, not this one**: does a wallet need to be visible to the player
  for this to be a meaningful step (project it wherever character state already reaches the
  client), or can persistence-only ship first with visibility as its own fast-follow? Either
  answer is fine; don't presume it here.

**Wave 4 — `Trade` learns to charge** (the first wave that's genuinely cross-repo again):
- `TradeOffer.Currency` — additive field, finally filled in (rpg-project#370 deliberately shipped
  without it).
- `session.Trade`'s refusal narrows: `Give.Items` stays refused (no item-for-item yet), but
  `Give.Currency` becomes a *new legal case* — check `Wallet.CanAfford`, `Wallet.Sub`, otherwise
  unchanged (decrement stock, add item, exactly as today).
- **rpg-api-protos**: `TradeOffer` gains `currency`; `VendorStockEntry` gains `price` (doesn't
  exist today — web's PR #920 flagged this gap directly).
- **rpg-api**: bump pins, thread the new fields through the existing `Trade`/`Interact`
  converters.
- **rpg-dnd5e-web**: show price per stock row, show the player's own balance (needs Wave 3's
  visibility decision), a real "Buy for 15gp" confirm, balance updates on success.

**Beyond Wave 4, not yet designed:**
- **`Quote`** (multi-item cart) — rpg-toolkit#1275's `Quote{Lines []QuoteLine, Total}` is already
  a multi-line shape; this is where "buy several different items in one transaction" lands,
  generalizing Wave 4's single-item case rather than needing its own separate multi-item project.
- **Selling / barter** — `Give.Items` stays refused through every wave above. Unlocking it means
  resolving what rpg-project#370 left open: does a vendor accept unlisted items? What's the sell
  price (half of `PriceOf`, the usual convention, or something else)? Does a sold item reappear in
  vendor stock? None of that is decided; it's its own design conversation when it's time.

## 6. Done when

`currency.Money` round-trips through JSON, `ParseCost` correctly parses every existing equipment
`Cost` string (a table-driven test over the real data, not a handful of examples), `Add`/`Sub`/
`CanAfford` behave correctly at zero and at the boundary (afford exactly, afford one short), and
`Breakdown` produces the correct coin-purse split for representative amounts including one that
exercises every denomination at once.
