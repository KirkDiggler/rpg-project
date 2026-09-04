---
status: DESIGN IN PROGRESS — rulings of 2026-09-04 recorded; §4–§6 open
journey: rpg-project#326 (Living World), slice 2
predecessor: concealed-door/design.md (slice 1, shipped 2026-09-02)
spike: rpg-toolkit examples/world/scenarios/tomb (UC-4)
---

# Recover the artifact — slice 2 of Living World

The scenario, in Kirk's words (2026-09-02): a concealed room, an artifact
placed in it, and a captain who has the map to that room. Two ways to win,
one ending:

1. get in, find the artifact, and leave;
2. kill the captain, learn the room from them, grab the artifact, and leave.

"Filling in a scenario config on the dungeon builder" is the prize. This
document is the design of that form and of the two capabilities it binds.
Capabilities before whole functionality (ruled 2026-09-01) still governs:
each piece below earns its place on its own.

## 0. Vocabulary

- **placement** — one `place:` entry in dungeonspec v2: a ref and a cell.
  Props and monsters are both placements.
- **placement id** — the author's name for one placement, the way a door
  and a region already carry `id:`. Does not exist today.
- **knowledge link** — an author-declared "this placement knows this
  secret" (concealed-door design, named 2026-09-01, deferred until now).
- **intel** — a knowledge fact in the run's journal: `known:door:<id>` or
  `known:region:<id>` with an audience (encounter/conceal.go).
- **loot** — a player verb on a downed member that transfers what the body
  holds to the looter.
- **holding** — what a member carries during a run. Does not exist today.
- **the form** — the scenario's config as the builder renders it: field key,
  label, type, guidance; validated by the scenario package's own `New(cfg)`
  (ruled 2026-09-01).

## 1. Ruled 2026-09-04 (Kirk, in session)

- **R1 — Loot the body.** The intel changes hands by an explicit act on the
  fallen captain. The looter gets it **for free — no check.**
- **R2 — That is the victory path.** Path 2 is loot → open → take → leave.
  No knowledge transfers on the kill itself; a party that kills the captain
  and never loots learns nothing.
- **R3 — Shelved, not built** (Kirk's own list, §9): the intel as a
  parchment item; a knowledge check to read it; handing the item to a
  party member transfers the intel with no check on the document; how one
  player shares intel with another.

## 2. Proposed, awaiting ruling

- **P1 — Intel lives on the placement, not on the scenario.** Slice 1 put
  concealment on the door declaration because it belongs to the door
  wherever the author puts it. The same test puts "knows the vault door"
  on the captain's `place:` entry. Then the scenario form binds only the
  artifact, and nothing in the game needs the word *captain*: the captain
  is a monster who knows a door. (The spike needed a `Captain` field only
  because its `Defeat` verb was scenario-owned; in the game the fall is a
  real event the encounter already notices.)
- **P2 — Placement ids.** `id:` on a `place:` entry, optional, unique
  within the dungeon, refused by name on collision — the third instance of
  the id regions and doors already carry. Required only by whatever binds
  to it (a knowledge link's *subject* needs none; a scenario binding does).
- **P3 — Loot is offered on every body.** The affordance must not say
  which monster carries intel. Every downed member is lootable; a body
  with nothing to give transfers nothing. Same law as search: the answer
  never leaks the question.
- **P4 — Loot is a second writer of the fact search writes.** Success is
  `learnDoor(looter, door, cause="loot")` plus the looter's own
  DOOR_REVEALED beat — the beat slice 1 already sends. Zero new reveal
  shape. The room still hides with its door; it reveals when the door is
  perceived open (slice 1, unchanged).
- **P5 — One mechanism for "who has what".** *Holding* is the noun. The
  captain holds intel; a member who took the artifact holds the artifact;
  a fallen holder's body holds it still, and the same loot verb takes it
  back. The parchment shelf (§9) is this noun with an item in it.

## 3. The authoring model — the file and the form

### 3.1 dungeonspec v2 additions (toolkit `dungeonspec`, carried; validated)

```yaml
place:
  - { id: captain, ref: "dnd5e:monsters:skeleton-captain", at: [23,5],
      targeting: closest, knows: [vault-door] }
  - { id: heirloom, ref: "dnd5e:props:reliquary", at: [30,3],
      blocks_movement: false, blocks_los: false }

scenario:
  recover-the-artifact:
    artifact: heirloom
```

- `id` — P2. Optional. Refused on collision, naming both lines.
- `knows` — a list of door ids (regions later, if a use case arrives).
  Refused by name when the door does not exist. Refused on a prop
  (a prop holds nothing, for `blocks_movement`'s reason). Legal on a
  monster whether or not the door is concealed — knowing an ordinary
  door is inert, not an error.
- `scenario` — one key (the scenario id) mapping to that scenario's
  bindings, **carried opaquely by dungeonspec** and validated by the
  scenario package's `New(cfg)`: dungeonspec checks only that every
  binding names a placement id that exists. Ruled 2026-09-01: "the
  dungeon spec stores `{scenario_id, bindings}` as pure references".

### 3.2 The form

Ruled 2026-09-01 and unchanged: `ListScenarios` returns, per scenario
package, a descriptor (field key, label, type, guidance = the constructor's
refusal text); rpg-api translates verbatim; the builder renders one picker
per field type; submitting validates through `New(cfg)`; descriptor and
`Config` pinned both ways by test.

For this scenario the form has **one field**:

| key | type | guidance (the refusal, verbatim) |
|---|---|---|
| `artifact` | `entity_ref(prop)` | this scenario needs an artifact — which placed thing is the party here to recover |

The `captain` field of the spike's `Config` is **deleted** under P1. The
door's find and open checks stay on the door (slice 1).

### 3.3 Refusals dungeonspec owns (fail closed, name the line)

- a binding names a placement id that does not exist;
- `knows` names a door id that does not exist;
- `knows` on a prop;
- duplicate placement id.

## 4. The loot verb — OPEN in part

Rule half in `encounter` beside `search.go`; entry in `session` beside
`Search`; the same law: keep the secret at the rule half.

```go
type LootInput struct {
    Member MemberID   // who loots
    Target MemberID   // the body
}
type LootOutput struct{} // ack only — see Q1
```

Validation order mirrors search: nil → empty member → closed → not a
member → target not a member → target not down → not adjacent. Refusals
for "not down" and "not adjacent" are ordinary (the body is visible; there
is no secret in whether it is down). Effect: for every holding of the
target, transfer to the looter — today that is intel only: `learnDoor`
with cause `loot`, audience the looter alone, one DOOR_REVEALED beat.

**Q1 (open):** the output. Search returns nothing because *anything* would
say whether there was something to find. Loot's target is a known body,
and "this body carried nothing" is not a secret worth a silent answer —
but a client cannot tell "nothing" from "the beat has not arrived".
Options: ack-only and the beat is the answer (search's shape), or a
`Transferred int` count (says nothing about *what*). Lean: ack-only for
symmetry; revisit when the parchment (§9) makes loot yield an item.

**Q2 (open, plan-level):** the body must be reachable after the fight
dissolves — a removed initiative slot must keep its position. Check in
the plan against `noticeDown`'s Remove consequences.

## 5. The artifact — OPEN (next question for Kirk)

Today a prop is scenery: a ref, a cell, two blocking flags. "Grab the
artifact" needs a verb and a fact. The candidates, cheapest first:

- **A — reach it.** `TriggerReachedPosition` exists. Reaching the artifact's
  cell and then withdrawing wins. No item. *Fails zero-values-tell-the-
  truth:* a member who stood there and left without it "has" it.
- **B — take it (recommended).** A `Take` verb on an adjacent or same-cell
  prop declared takeable; the prop leaves the atlas (a beat every present
  member sees — no secret here); the taker *holds* it (a journal fact,
  `holds:<placement id>`, audience everyone — physical state folds on the
  truth grain, ruled 2026-09-01). A fallen holder's body holds it; loot
  (§4) takes it back — P5's symmetry is the argument for B over a special
  artifact flag.
- **C — inventory.** The character sheet's inventory. Rejected for the
  run: the artifact is run-scoped until the run ends; what "keeping it"
  means across runs is the journal's business (brainstorm §10) and a
  later slice.

Under B: which props are takeable? Proposed: `takeable: true` on the
placement, refused on a monster, defaulting to false — a thing nobody
declared takeable stays scenery. The scenario's `artifact` binding refuses
a placement that is not takeable, in form-filler words.

## 6. The ending — OPEN, with a named promotion

"Both paths end in withdrawal with the artifact" — one ending, not two:
**withdrew while holding the artifact.** Brainstorm §9 ruled: run-ending
is quest v0; "promote the run-goal predicate when the second goal type
arrives, don't invent an engine." This is the second goal type.

**Q3 (open):** where the ending lives. Today `EndingInput{Key, Trigger}`
with three triggers (reached position, member down, external). Options:
(a) a fourth trigger `TriggerWithdrewHolding{Item}` — smallest, and
keeps the endings model the single owner of "the run is over"; (b) the
scenario package's quest predicate (`quest.Flagged`) decides and the
host raises `TriggerExternal` — the spike's shape, two owners of one
answer. Lean: (a), and the scenario package's job shrinks to *declaring*
it; the composer stays out (packages are the units, ruled 2026-09-01).

**Q4 (open):** what the form is for, once P1 lands. With intel on the
placement and loot generic, the scenario carries one binding and one
ending. A `scenario:` block validated by a Go package versus an
`endings:` declaration in dungeonspec is a real fork: the form is the
prize Kirk named, and the package is what makes the descriptor, the
refusals and the pinning test exist; an `endings:` line is cheaper and
has no form. Ruling needed before the plan.

## 7. Ownership

| noun | holder | why |
|---|---|---|
| placement id | dungeonspec (author) | third id after region and door |
| knowledge link `knows` | dungeonspec placement → encounter concealment world at load | like `concealed:` on a door: belongs to the thing, wherever placed |
| `scenario:` bindings | dungeonspec, opaque; scenario package `New(cfg)` validates | ruled 2026-09-01 |
| loot rule half | encounter (`loot.go` beside `search.go`) | the secret is kept where the fact is written |
| loot entry | session (beside `Search`) | the seam; adjacency is the host's truth |
| holdings | encounter world journal facts | run-scoped truth-grain state; the parchment shelf inherits it |
| takeable + `Take` | dungeonspec flag; encounter + session verb | same shape as loot |
| the ending | encounter endings | single owner of "the run is over" (Q3) |
| form descriptor | toolkit scenario package export → rpg-api verbatim → web renders | ruled 2026-09-01 |

Charter checks run: session gains two entry verbs and no rule (holds);
encounter gains two rule halves and one fact kind (holds — it already
owns search and the knowledge facts); dungeonspec gains three carried
fields and refusals it can decide alone (holds — it never resolves
content, C1); rpg-api translates and learns no scenario word (holds).
Singularity: "who has what" answered once, by holdings (P5); "who knows
what" answered once, by the journal (unchanged).

## 8. Acceptance (draft — completes when §4–§6 rule)

| item | proof |
|---|---|
| a party that never loots and never searches finishes the run blind | acceptance scene: kill the captain, never loot, the door stays a wall for everyone (secrecy check, not a win rule — Kirk 2026-09-02) |
| loot on the captain reveals the door to the looter alone | DOOR_REVEALED to one recipient; the other member's atlas unchanged until the door is opened in their presence |
| loot on a body with nothing gives the same bytes as loot on the captain before the reveal beat | P3 |
| take removes the prop for everyone and the taker holds it | atlas beat to all present; `holds` fact |
| withdrawing while holding ends the run once; withdrawing without it does not | Q3 |
| the form refuses a missing or non-takeable artifact in form-filler words | descriptor↔Config pin |

## 9. Shelves — named, empty (Kirk 2026-09-04)

- **The parchment.** Loot yields an *item* that carries the intel instead
  of the intel itself. Needs holdings (§5 B) to exist first; then loot is
  pure item transfer and the intel arrives on reading.
- **Reading is a check.** A knowledge check (Arcana, History, or the
  author's list — multi-approach, as every check is) to get the intel out
  of the parchment. The document's check, distinct from loot's none.
- **Handing an item transfers its intel, no check on the document.** The
  receiver learns what the parchment says by being handed it. Reconcile
  with the 2026-08-30 ruling that social sharing stays out of the game
  ("friends talk over voice — the asymmetry is the fun"): sharing by
  *telling* stays out; sharing by *acting in front of others* is slice 1
  (a knower opens the door, those present see); sharing by *handing a
  thing over* is this shelf. Three different verbs, one of them ours.
- **Player-to-player intel sharing** in general — Kirk's open question.
  No verb until a use case beyond the parchment names one.
- **Loot beyond intel** — coin and gear are rpg-project#310's (wallet,
  shop, prices). Holdings is the seam they will use; nothing stocked.
- **Who sees a body is lootable** — sight-scoped affordances ride the
  sight-scoped-movement follow-up, not this slice.

## Boundaries held

- The room hides with its door; loot reveals the door only (slice 1).
- Search is unchanged. Loot is beside it, never inside it.
- Rooms carry no scenario roles; neither do monsters — `knows` is a fact
  about a monster, not a role.
- No composer adoption; packages are the units (2026-09-01).
- Victory conditions are unchanged by concealment.
