---
status: DESIGN, proposed 2026-09-22 (Kirk + Fable dream session). NOT SCHEDULED. Rulings are Kirk's, cited as "ruled 2026-09-22"; open questions are under §8.
journey: rpg-project#326 (Living World) — pays off the Take shelf, ../hold-out/design.md §11
siblings: ../campaign-journal/design.md (the story ledger, and this is NOT it) · ../town/design.md (the merchant)
law: only the game creates an item instance; everyone else rewrites its owner
---

# The item ledger — one place a duplication bug can hide

## 0. Purpose

**Items are a separate ledger from facts, and it exists for the economy only**
(ruled 2026-09-22). Gold, gear, stock, the thing the merchant sells you and the
thing you sell back. It is not where story outcomes live — those are facts, and
facts are ../campaign-journal/design.md.

The reason for a ledger rather than a field on a character: **dup-proofing**. If
the only way an item comes into existence is one call in one place, there is one
place a duplication bug can hide. Today there is no runtime "add an item to
inventory" path at all (`InventoryItemData` is produced at draft-compile time
only, ../../npcs/trade/design.md §1), which means the path does not yet exist to
get wrong — this is the design of it before it is built in three places at once.

Kirk's precedent, in his words: a prior game bit-shifted a character guid to
change the scope of an inventory. This design names the bits.

## 1. The instance

```
{ id, ref, owner: { scope, id }, quantity }
```

- **`id`** — the instance's own identity, minted once, never reused. Two
  longswords are two rows.
- **`ref`** — what it is: an equipment ref the rulebook already knows
  (`dnd5e:weapons:longsword`). The ledger never learns what a longsword does.
- **`owner`** — a COMPOSITE KEY, `{scope, id}`. See §2.
- **`quantity`** — a stack is one row, because equipment data already stacks
  (`TradeItem.Quantity` exists for exactly this).

Nothing else. No location, no container, no equipped flag — those are questions
about a character's sheet or a room's floor, and they have their own owners.

## 2. The three scopes

`owner.scope` is closed, and each value answers a question the others cannot:

| scope | `owner.id` | what it means |
|---|---|---|
| `character` | a character id | the character owns it wherever they go — the gear on their sheet |
| `party:character` | a character id, within one party | a character's holdings inside ONE party. A player may belong to many parties, and what they carry in one is not what they carry in another |
| `world` | an NPC, merchant, king, or a site | stock, a vendor's shelf, the king's treasury, the contents of a chest nobody has opened |

`party:character` is the scope that earns the composite key. A player with three
characters across two parties has a row-space with no collisions and no id
arithmetic, and the fold "what does this character have, in this party" is a
filter rather than a mask. This is the bit-shift, written as data.

**Inventory moves OUT of the character module.** A character's gear becomes a
fold over ledger rows with `scope: character` and this character's id. Nothing
is "deleted from the character to be created in the ledger" — the ledger becomes
the one place the rows were always going to live, and the character's inventory
becomes a view of it. That is a real migration and it is named here, not hidden.

## 3. Who creates, who rewrites, who consumes

**Only the game creates an instance** (ruled 2026-09-22). Players and verbs only
rewrite the owner. Three creators, and no fourth:

| creation | when |
|---|---|
| **Take** | a player pockets an authored prop that has an item identity. The prop's `item:` key (../hold-out/design.md §11's shelf, shape (a)) is what says a prop is pocketable; absent means hold-only, which is the zero value that keeps an heirloom chest in two hands |
| **an ending's reward** | content at a run's ending mints what it said it would |
| **content at load** | a merchant's authored stock, a chest's contents — minted when the document is loaded, owned by `world:<id>` from the first instant |

**Rewriting the owner** is what every verb does:

| verb | rewrite |
|---|---|
| **Trade — buy** | `world:merchant-01` → `party:character:<buyer>` |
| **Trade — sell** | `party:character:<seller>` → `world:merchant-01` (which is why buyback is free: sold stock is indistinguishable from authored stock, ../../currency/sell/design.md §2) |
| **turn-in** | `party:character:<X>` → `world:<king>`, or the row is deleted |

**A turn-in consumes.** Hand the heirloom to the king and the instance is gone
from the party — owner rewritten to `world`, or removed outright. That is the
difference between an item and a fact in one sentence: **you can only turn the
heirloom in once, and you will always have returned it.** The item is consumed;
the fact `heirloom-returned` is written and stays true forever.

## 4. Hold, Take, Trade — three verbs, three lifetimes

`hold.go` already draws the line, and this design does not move it:

- **Hold** is a pair of hands for the length of the run. It writes a run-journal
  fact (`holds:prop:<id>`), the prop leaves the atlas for everyone, and nothing
  reaches any sheet. Unchanged.
- **Take** pockets an item. It writes a `taken:<prop>` fact on the run journal
  AND mints a ledger row owned by the taker. Two writes, two ledgers, one verb —
  and §5's laws are what keep them from reaching into each other.
- **Trade** moves an existing row between owners. ../../npcs/trade/design.md's
  `Give`/`Receive` shape is already exactly "rewrite the owner of these rows",
  and the currency side (../../currency/design.md) is the same rewrite on a
  `Money` amount instead of a row.

The two breaks the Take shelf asked to be ruled are answered here: the
withdrawal ending names a HELD item, and a pocketed win leaves with the
character while the run's win does not. **Both dissolve** once the heirloom's
story outcome is a fact rather than an item — the ending reads
`{ fact: taken:heirloom }`, which is true whether the thing is in a hand or a
pocket, and the campaign keeps the FACT while the ledger row is just loot. The
shelf anticipated ruling these as item questions; they turned out to be fact
questions.

## 5. The two laws that keep the ledgers apart

Ruled 2026-09-22, and they are the whole reason two ledgers are safe:

1. **A gate never reads an item. It reads a fact.** Take writes
   `taken:<prop>`; the gate — an `arrives`, an `endings[].when`, a town's
   condition — reads that fact. Nothing anywhere asks "does this character
   possess instance 4a7f". If it did, then losing, selling, or dropping loot
   would silently rewrite the story, and a merchant would be a plot device.
2. **A fact never spawns an item.** Only content spawns items, at a Take or at
   an ending. A fact arriving in the journal mints nothing. If it did, a fact
   that copies freely — which every fact does, by design — would copy items with
   it, and the dup bug would be in the knowledge system where nobody would look
   for it.

Together they mean the two stores can be wrong independently without corrupting
each other, and each is debuggable alone.

## 6. Relationship to what exists

- **../hold-out/design.md §11, the Take shelf** — this is that shelf stocked.
  Its shape (a) `place[].item:` is §3's creator; its (b) `taken:<prop>` fact and
  the prop leaving the atlas are unchanged; its (c) "a runtime item-enters-
  inventory path the character does not have today, which Trade's receive side
  needs too" IS this ledger, built once for both as the shelf demanded.
- **../../npcs/trade/design.md** — Trade is a rewrite, not a transfer of
  ownership between two hand-maintained lists. Its `Give.Items` /
  `Receive.Items` shape survives untouched; what changes underneath is that both
  sides read and write one store.
- **../../currency/design.md and ../../currency/sell/design.md** — `Money` is
  not a ledger row (a coin purse is a scalar, not an instance), but a wallet has
  the same owner key: `{scope, id}` → `Money`. Sell's `AddToVendorStock` becomes
  a plain owner rewrite, which is a smaller mechanism than the one that design
  had to invent without a ledger to lean on.
- **../recover-the-artifact/design.md §5** — its rejection of the character
  sheet ("the artifact is run-scoped until the run ends; what keeping it means
  across runs is the journal's business") stands and is now answered: the
  artifact's meaning is the journal's, and its loot value is the ledger's.

## 7. Ownership

Ruled 2026-09-22, the same split as the campaign journal:

| thing | owner |
|---|---|
| the rule — what a verb is allowed to rewrite, and the refusals | rpg-toolkit, where the verb lives (Take in encounter, Trade in session) |
| the store — instances, forever | rpg-api persistence |
| the door | the session SDK: one read ("items for owner X") and one write ("set owner") |

rpg-api never decides that a trade is legal. It is handed a rewrite and it
applies it. The minting call is the one place the id is generated, and it is the
one place to look when two longswords become three.

## 8. Explicitly NOT the story ledger

Stated as its own section because the failure mode is real and quiet: the moment
a quest reads "does the party possess the heirloom" instead of "did the party
take the heirloom", the economy becomes load-bearing for the story and every
sale is a plot hole. §5's first law forbids it. If a use case seems to need it,
the answer is a fact written at the moment the item changed hands — which is
what a turn-in already does.

## 9. Open rulings (Kirk)

1. **Where a character's gear lives after the move** (§2). The migration out of
   the character module is named but not sequenced, and the character's compiled
   draft inventory is its current source.
2. **Whether `party:character` is one scope or two keys.** Written here as one
   scope with a character id, which assumes a row knows its party. The
   alternative is `scope: character` plus a separate party column.
3. **Whether a turn-in rewrites to `world` or deletes.** §3 allows both; the
   difference is whether the king can be robbed later.
4. **Stacks versus instances for ordinary goods.** A quantity column is written
   in §1, which means fifty arrows are one row; whether a named or magical item
   must be its own row is undecided.
5. **Whether a wallet is a ledger row or its own store** (§6).
