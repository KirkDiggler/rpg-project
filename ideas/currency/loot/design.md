# Loot gains currency — a body can hold gold, same verb, one new arm

**Parent:** rpg-project#386 · **Initiative:** rpg-project#310 (buy-only vendor initiative — this closes the "loot containers" line it
named and explicitly deferred). **Origin:** rpg-project#376.

## 1. What exists today, verified directly

`encounter.Loot` moves everything a downed member holds to the looter via one dispatch routine,
`transferHoldings` (`encounter/loot.go:196`):

```go
for _, item := range e.holdings.holdingsOf(from) {
    switch {
    case item.record != "":  // intel
        ...
    case item.prop != "":    // a holdable prop
        ...
    }
}
```

Two holding kinds exist today (intel, prop); currency would be a third arm in this same switch,
not a new verb, not a new dispatch mechanism — the design already anticipated this ("today that is
intel only," per the original acting-on-ghosts design). **Zero currency anywhere in either
`encounter/loot.go` or `session/loot.go`** (verified) — this is genuinely new, same as `Money`
itself was.

**`monster.Data` has no currency field at all** (verified, grepped) — how much gold a monster
carries doesn't exist as data anywhere yet.

## 2. The real shape — two layers, same split as `Interact`/`Trade`

This isn't "add one field, done" — `encounter` and `session` split responsibility here exactly
the way they already do for `Interact`: `encounter` answers reach/identity and moves
encounter-internal facts; `session` resolves content that touches session-persisted state
(`character.Data`) and saves it.

**`encounter` layer:**
- The holding struct gains a currency field (alongside the existing `record`/`prop` fields) —
  a monster's holdings, seeded at placement/spawn from `monster.Data`'s new gold value.
- `transferHoldings` gains the third switch arm: move the currency holding from body to looter,
  same as intel/prop already do, within encounter's own holdings tracking.
- `encounter.Loot`'s response needs to report *that* currency moved (not necessarily how much, to
  keep design P3's "the answer never leaks the question" law — a body with gold and a body
  without produce visually different beats only insofar as the looter's own wallet changed, not
  via anything richer in the shared `looted` beat).

**`session` layer — the genuinely new part:**
- `session.Loot` today (`session/loot.go`, 122 lines) is a thin seam wrapper — call
  `encounter.Loot`, return. It has no reason to touch `character.Data` today because intel and
  props are encounter-internal facts.
- Currency is the **first holding kind that needs a session-layer follow-up**: after
  `encounter.Loot` confirms a currency transfer, `session.Loot` must fetch the looter's
  `character.Data`, `Wallet.Add` the amount, and save the character record — the same
  `saveCharacterRecord` helper `Trade`/`write.go` already share, not a new persistence path.
- This makes `session.Loot` grow the same shape `session.Trade`/`session.Interact` already have
  (encounter answers the world question, session resolves and persists the content question) —
  it just hasn't needed to before now.

## 3. What's deliberately not decided here — content, not mechanism

**How much gold a given monster carries is a monster-authoring/content decision**, not a mechanism
decision, same distinction as "17 backgrounds' worth of gold values" being separate from the
wiring that reads them. This design adds the *field* (`monster.Data`'s new gold value) and the
*mechanism* (encounter holding + session credit) — it does not decide what a skeleton vs. a bandit
carries. That's balance/content work for whoever authors monsters, entirely independent of this.

## 4. Explicitly out of scope

- **Loot of items** (a body's equipped weapon becoming takeable) — a `prop`-kind holding already
  covers a body carrying a takeable object; whether combat loot commonly includes weapons/armor is
  a monster-authoring question, same as gold amounts, not this design's concern.
- **Any pricing/value question** — gold looted is just `currency.Money`, added directly via
  `Wallet.Add`. No `PriceOf` involved; nothing is being bought or sold here.

## 5. Done when

Killing a monster authored with a gold holding and looting its body credits the looter's `Wallet`
by exactly that amount, saved durably; looting a monster authored with none changes nothing about
the looter's wallet and produces byte-identical beats to any other empty loot (P3 preserved). A
monster authored with BOTH intel and gold transfers both through the same call, no special
ordering required.
