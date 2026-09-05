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

## 5. Done when

`currency.Money` round-trips through JSON, `ParseCost` correctly parses every existing equipment
`Cost` string (a table-driven test over the real data, not a handful of examples), `Add`/`Sub`/
`CanAfford` behave correctly at zero and at the boundary (afford exactly, afford one short), and
`Breakdown` produces the correct coin-purse split for representative amounts including one that
exercises every denomination at once.
