# Trade — one symmetric exchange verb, proven first through a vendor

**Parent:** rpg-project#369 · **Journey:** rpg-project#311 (World NPC Foundation) ·
rpg-api#903 (wires the vendor end to end)
**Builds on:** `Interact` + `WorldNPCDescriptor` (rpg-toolkit#1404/#1434/#1444/#1447, merged;
rpg-api#903 Phase 1, merged; web #920, merged) — a player can already reach a placed vendor and
see its stock. This design adds the verb that moves an item.

## 1. What exists today, verified against `main` in each repo

- **rpg-toolkit**: `session.Interact` returns a `WorldNPCDescriptor` including `Inventory
  []npcs.StockEntryView`, resolved fresh from `npc.Data.Inventory` on every call
  (`session/interact.go`). Read-only — nothing decrements stock or touches a character's
  inventory.
- **rpg-toolkit/character**: no runtime "add an item to inventory" path exists.
  `InventoryItemData` is only ever produced at draft-compile time (`draft.go:compileInventory`).
- **rpg-toolkit/npcs**: `VendorInventory` is a read-only projection — built fresh from
  `VendorInventoryData` on every call, no mutation method (`vendor_inventory.go`).
- **rpg-api / protos / web**: full wire support for `Interact` (`MEMBER_KIND_WORLD`,
  `InteractRequest/Response`, `VendorStockEntry`) and a rendered, clickable vendor
  (`VendorPopover`, web PR #920). No purchase/transfer action anywhere in any of the four repos.
- **rpg-toolkit#1275** ("Vendor and NPC Inventory Foundation") already designed a `Buy` flow with
  `Money`/`Wallet`/`Quote` — none of it built. This design is a narrower first step: the same
  eventual destination, reached without inventing `Money` before it's needed.

## 2. Why the verb is `Trade`, not `Take`

`Take` is already spoken for. rpg-toolkit#1495 (Living World, "recover the artifact",
`design/recover-the-artifact` branch) is landing an `encounter.Take(TakeInput{Member, Target
PropID, Range})` right now — pick up an authored floor prop, get a generic `holds:<id>` **fact**
used only to check an ending trigger. It never touches equipment, never touches a character's
inventory, and the prop vanishes from the atlas for everyone. That is a different noun (an
authored prop) and a different result (an engine-internal fact, not usable gear) from what a
vendor purchase needs, so reusing the name or the type would be wrong on both ends, not just
inconvenient.

That design's own §4.1 states the law this doc also follows: *"A verb is named by what the
record will say... Each verb owns its target kind, its refusal order and its beat, exactly as
Search, OpenDoor, Unlock and Interact do today."* `Trade` is this feature's own verb, honoring
that law rather than reusing a name that means something else next door.

`Trade` also reads correctly for every shape this exchange can take — buying, selling, bartering,
gifting — which `Take` (unidirectional by name) does not.

## 3. The shape

```go
// TradeOffer is one side of an exchange: the items changing hands in that
// direction. Empty is legal — a Give of no items and a Receive of one item
// IS a gift; nothing marks it as one.
//
// No Currency field yet. Adding one is an additive change (the same
// convention npc.Data.Inventory and MEMBER_KIND_WORLD already use in this
// codebase) once rpg-toolkit#1275's Money type is ruled — inventing Money
// here, unused, would be designing for a requirement this slice doesn't have.
type TradeOffer struct {
    Items []TradeItem
}

type TradeItem struct {
    Type     shared.EquipmentType
    ID       string
    Quantity int
}

type TradeInput struct {
    Session string
    Actor   string // the initiating player
    Target  string // the counterparty — a KindWorld member, this slice
    Range   int     // as Interact's: zero means adjacent
    Give    TradeOffer // what Actor hands over
    Receive TradeOffer // what Actor gets
}
```

No `Direction` enum. `Give`/`Receive` already say everything a direction field would, without a
special case for gifting:

| Scenario | Give | Receive |
|---|---|---|
| Buy | currency (future) | items |
| Sell | items | currency (future) |
| Barter | items | items |
| Gift, either way | empty | items — or reversed |
| **This slice's only case** | empty | one item, N quantity |

**Refused: both sides empty.** A trade that moves nothing on either side is a caller defect, not
a very small gift — the same convention `encounter.Interact` already applies to a negative
`Range` (a degenerate input is reported, not silently accepted).

## 4. Scope for this wave — deliberately smaller than the shape above

The type is bidirectional so the wire contract doesn't need to change when the next wave lands.
The **behavior** this wave ships is one-directional only:

- **`Give.Items` MUST be empty.** A non-empty `Give` is refused (a new sentinel, e.g.
  `ErrGiveNotSupported`) — not silently ignored. Accepting arbitrary items into a vendor's stock
  raises real unresolved questions (does an unlisted item create a new stock row? what happens to
  it once pricing exists?) that belong to the sell/barter wave, not this one.
- **`Receive.Items` MUST contain exactly one entry, any quantity up to what's in stock.**
  `TradeItem.Quantity` already exists (a stack has to carry one) and vendor stock already tracks
  a quantity to check against, so "N available" costs nothing over "1 available" — the same
  comparison with a variable. What's refused is **multiple distinct items in one call** (a torch
  and a rope together): that needs validating every line before committing any of them, which is
  real added complexity this wave doesn't need to take on.

  This isn't a permanent limit being deferred awkwardly — it's sequenced correctly.
  rpg-toolkit#1275's `Quote` is already shaped as multiple lines (`Lines []QuoteLine`) with a
  `Total`; a quote **is** a cart. Buying several different items in one transaction is what the
  Quote wave generalizes to N lines as part of adding pricing, not a separate multi-item project
  bolted onto `Trade` beforehand.
- **Counterparty MUST be `KindWorld` with `CapabilityVendor`.** Player-to-player trade is
  explicitly out of scope for this wave — not because the shape doesn't fit (it does: same
  `TradeInput`, a player `Target` instead of an NPC), but because a player counterparty needs
  *consent* a vendor doesn't. A vendor has no agency; validating against its stock/policy is a
  single-call decision. Pulling items out of another player's inventory unilaterally is not — that
  needs a pending-offer/accept-decline mechanic, which is its own piece of work layered on this
  same `TradeInput` shape later, not a free consequence of it.
- **No server-side quote/propose step.** The client already has the vendor's stock from a prior
  `Interact` call; a "Buy Longsword?" confirmation is a client-side dialog before firing one
  `Trade` call. A real quote step earns its cost once there's a price to preview.
- **Client-facing label: "Buy."** The wire/verb name stays `Trade` (that's what the record says);
  the button says what the player understands, even before currency is real.

## 5. Cross-repo plan

### rpg-toolkit (`main`)

1. `character`: a runtime inventory-add helper, e.g.
   `AddInventoryItem(data *character.Data, item InventoryItemData) error` — merges into an
   existing stack by type/id, appends a new one otherwise. New code path; nothing like it exists
   today.
2. `npcs`: a mutation on `VendorInventoryData`, decrementing one `StockModeLimited` row by one
   (error — `ErrOutOfStock` or similar — if missing or already at zero), leaving
   `StockModeUnlimited` rows untouched.
3. `session`: new `Trade` verb (`session/trade.go`, shape mirrors `interact.go`): `openForWrite`
   → confirm reach/visibility → refuse non-empty `Give` → decrement the target's stored vendor
   inventory → `AddInventoryItem` on the actor's `character.Data` → `saveCharacterRecord` (already
   shared by write.go/move.go) → append a `traded` beat → `commit`.

   **Open question for review:** does `Trade` call `encounter.Interact` itself for the
   reach/visibility check (accepting two beats per purchase — `interacted` then `traded`, each a
   true, separate statement) or does the reach/visibility check need factoring into a private
   helper both verbs share, so `Trade` doesn't require a prior `Interact` call and doesn't emit an
   `interacted` beat it didn't ask for? Recommendation: reuse `encounter.Interact` as-is and accept
   two beats — smallest diff, and the Living World `Loot`/`Take` verbs each already run their own
   full validation independently rather than depending on a prior call, so this isn't a new
   pattern.

### rpg-api-protos (`main`)

- `Trade` RPC on `SessionService`, beside `Interact` (`service.proto` ~line 1115).
- `TradeRequest{session, actor, target, give, receive}` / `TradeResponse{descriptor, seq, saved,
  delivery}` — `give`/`receive` as `repeated TradeItem`, `TradeItem{type, id, quantity}`. Reuses
  the `WorldNPCDescriptor` message already defined for `InteractResponse` so the client can
  refresh stock display from the same shape it already knows how to render.

### rpg-api (`dev`)

- `internal/handlers/dnd5e/session/v1alpha1/trade.go`, same thin translation shape as
  `interact.go` (`callerActingAs` → one `manager.Trade()` call → translate output).
- `Manager` interface gains `Trade`.
- `convert.go`: `TradeItem` converters (reuse the existing `EquipmentType` conversions already
  used for `VendorStockEntry`).
- `errors.go`: rows for the new toolkit sentinels (`ErrGiveNotSupported`, `ErrOutOfStock`, or
  whatever review settles on), `sentinelCount` bump — the test suite hard-fails without this, per
  #903's own confirmed gap list.

### rpg-dnd5e-web (`dev`)

- `useSessionTrade` hook, mirroring `useSessionInteract`.
- `VendorPopover`: a "Buy" button per stock row, client-side confirm dialog, disabled once a
  limited row's displayed quantity hits zero (the popover already refreshes from `Interact`'s
  response shape today; `Trade`'s response should let it refresh the same way without a second
  `Interact` round-trip).
- **Open question, not blocking**: no surface in the web currently shows a player's own inventory
  live during a session. This wave may ship with only a success/failure toast — worth confirming
  that's acceptable, or whether a minimal inventory readout is needed to see the trade land.

## 6. Explicitly out of scope

- Currency, wallets, pricing, quotes (rpg-toolkit#1275, unstarted beyond this design's stated
  non-interference with it).
- Selling (player → vendor), barter, and vendor stock accepting unlisted items.
- Player-to-player trade and the consent/accept-decline mechanic it needs.
- NPC content authoring — replacing the hardcoded demo vendor with an authored one
  (rpg-api#903 Phase 2). Unrelated to this design; can land before, after, or alongside it.
- Anything from rpg-toolkit#1495 (Living World `Take`/`Loot`) — a different feature, mentioned
  here only to explain the naming choice.

## 7. Done when

A player who has already `Interact`-ed with the demo vendor can click "Buy" on the longsword row,
confirm, and the longsword appears in their character's inventory over the real `Trade` RPC — no
mocks, no money, one unit, and the vendor's displayed stock reflects the decrement on the next
`Interact`/`Trade` response.
