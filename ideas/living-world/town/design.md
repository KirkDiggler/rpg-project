---
status: DESIGN, proposed 2026-09-22 (Kirk + Fable dream session). NOT SCHEDULED, and §2's choice is OPEN — Kirk decides which reading the first town is.
journey: rpg-project#326 (Living World) — brainstorm §7 (village square is a room), §11 (the town is a lobby)
siblings: ../campaign-journal/design.md (what a town gate reads) · ../item-ledger/design.md (what the merchant sells)
law: dialogue is not a system — it is Persuade and Intimidate rows with `say:` and `fact:` outcomes
---

# The town — a room, or a lobby

## 0. Kirk's ask (2026-09-22)

In his words: a non-combat area to explore. Visit the merchant. Talk to the
wiseman for quests. Use things taken from the dungeon as prerequisites — return
the heirloom to the king who is being attacked by goblins.

That is four capabilities: a place with no fight in it, a shop, a quest source,
and a gate that reads what happened in a previous run. The fourth is the one
that does not exist today, and ../campaign-journal/design.md is its design.

## 1. Dialogue is not a system

Ruled 2026-09-22, before either reading, because it settles most of the scope:
**a conversation is Persuade and Intimidate rows.** A creature's `on:` table
already carries `say:` verbatim and can land a `fact:` on every witness. That is
an offer, a line, and a consequence — which is what a dialogue tree is, minus
the tree. Nobody builds a tree; the branch is the roll and the fact.

The direction this needs was already ruled in
../../dungeon-authoring/world-builder/v4-gameplay-shape.md R5: both verbs are
offered OUT OF COMBAT ONLY, against a non-hostile creature, and the options
offered are the ones CONFIGURED on that creature. A town is where that direction
stops being a note and becomes the whole interaction model.

## 2. The two readings

These genuinely disagree, and the disagreement is in the record, not invented
here.

### Reading A — the town is a room

Brainstorm §7's shelf: "village square is a room, the road is a corridor;
travel reuses graph, walk, and door-distance noise." Under this reading a town
is a v4 single-room document with no hostile disposition. The king, the merchant
and the wiseman are factions; `on:` tables answer Persuade instead of Intimidate;
gates read the campaign grain; the merchant's stock is items owned by
`world:merchant-01`.

**What it forecloses:** the town inherits everything a room is — a hex grid, a
walk loop, line of sight, an atlas, a turn clock that has to be kept switched
off. A town of forty NPCs is forty placements on a grid, and a town that wants
to be a menu cannot become one without leaving the dialect. It also means every
later question about towns ("can two parties be in it at once?") is asked
against encounter machinery built for one party in one instanced run.

### Reading B — the town is a lobby

Brainstorm §11, the same week the guild model was ruled: "Discord itself
supplies voice/presence/community, so the town is a lobby with bodies and a
quest board, not a second simulation." Under this reading a town is not a
document at all — it is a shared hub where parties form, claim from a board,
dispatch, and return.

**What it forecloses:** there is nothing to explore. "A non-combat area to
explore" is the first line of Kirk's ask, and a quest board is not an area. It
also means every noun in the ask — walking up to the merchant, standing in front
of the king — needs a second implementation that is not the one the game already
has, and the streamer authoring a town gets a form, not the World Builder.

### Recommended: A, for the first town

Not because §11 is wrong about scale — a thousand-seat guild town is a lobby,
and §11 is the right answer for that — but because the first town is the
streamer's town, and reading A reuses every shipped noun. Nothing new is
invented to get the four capabilities: factions, dispositions, `on:` tables,
`say:`, `fact:`, `arrives`, `exits`, `endings` and `scenarios` are all already in
the dialect.

The two readings are also not exclusive in the long run: a lobby can dispatch
into a town room the way it dispatches into a dungeon. Starting with A costs
nothing that B needs later, and B first costs the thing Kirk asked for first.

**This is an open ruling. Kirk decides.**

## 3. The file (reading A)

A v4 single-room document. Everything here compiles today except the two lines
marked NEW.

```yaml
version: 4
key: kingsreach-square
play: { void: open, lighting: bright, standing: ground }

factions:
  - { id: crown,    mind: king }
  - { id: market,   mind: merchant }
  - { id: scholars, mind: wiseman }

dispositions:
  - { between: [crown, party],    stance: neutral }
  - { between: [market, party],   stance: neutral }
  - { between: [scholars, party], stance: neutral }

monsterBindings:
  king:
    persuade: [{ ability: persuasion, dc: 10 }]
    on:
      persuaded:
        - say: "You found it. The house of Aldric stands again — name your reward."
          fact: quest-king
          when: { fact: heirloom-taken }       # NEW — see §5
        - say: "Goblins at the north gate and you bring me words. Come back with the heirloom."
      persuade_failed:
        - say: "The audience is over."
  wiseman:
    persuade: [{ ability: persuasion, dc: 12 }]
    on:
      persuaded:
        - say: "The tomb below the old road. That is where it went."
          fact: knows-tomb
  merchant:
    persuade: [{ ability: persuasion, dc: 15 }]
    on:
      persuaded:
        - say: "For you — and only you — the good steel."
          fact: market-favor

exits:
  - { id: north-gate, cell: { q: 9, r: 2 } }

endings:
  - { id: dispatched, when: { fact: knows-tomb } }

scenarios:
  town: { board: wiseman }                      # NEW — see §5
```

What happens. The party walks into a square where nobody is hostile, so no
fight forms and every creature reads `enemy: none` however crowded the square
is — the existing neutral stance, doing exactly what it does today. Persuade is
offered against each of the three because each configured it. Lean on the
wiseman and he tells you where the tomb is, which lands `knows-tomb` on every
witness and fires the `dispatched` ending. Come back having taken the heirloom
and the king's first row is live, because the campaign journal seeded
`heirloom-taken` into this run (../campaign-journal/design.md §4); come back
without it and his second row is all he has.

## 4. What is reused

Everything in §3 except two lines:

| noun | where it already is |
|---|---|
| a room with nobody hostile | `dispositions[].stance: neutral` — a neutral creature runs its `time` table and reads `enemy: none` |
| three NPCs with their own voices | `factions[]` + `monsterBindings.<id>.on` |
| the wiseman's quest | a `fact:` on a `persuaded` row, landed on every witness |
| the gate on a previous run | `{ fact: … }`, seeded from the campaign grain |
| the way out | `exits[]` |
| the town ending | `endings[].when` — the same predicate grammar as every other sink |
| the merchant's stock | ledger rows owned by `world:merchant-01` (../item-ledger/design.md §2) |

## 5. What is new

Three things, and each is small:

1. **`when:` on a social row (NEW).** Today `when:` is `time`-ONLY and is its
   own grammar — a social key IS the condition, and the code says so by name.
   The king's conditional answer needs a social row to be judged against a
   `PredicateSpec`, which is a second grammar on that key. **This directly
   contradicts the shipped refusal** and is the one real dialect change the town
   asks for. The alternative that needs nothing new: two kings, one arriving on
   `arrives: { fact: heirloom-taken }` and one on its absence — which the
   grammar cannot express either, since there is no negation. Named as an open
   ruling.
2. **Out-of-combat Persuade (R5's direction).** Session prices and offers
   Persuade on both clocks today and derives a DC from passive Insight when
   none is authored. R5 ruled the offer becomes out-of-combat-only, against a
   non-hostile creature, showing the configured options. A town is the first
   place that matters.
3. **A `town` scenario binding (NEW).** Reading A needs a scenario that means
   "this run has no victory condition" — a room whose ending is walking out.
   `scenarios:` is carried opaquely by the dialect and validated only as
   references, so this costs the dialect nothing and costs the scenario package
   one small registration.

Plus the two designs this document leans on and does not contain: the campaign
journal's seeding, and the item ledger's owner rewrite behind Trade.

## 6. The merchant is where two shipped shapes collide

Worth naming before anyone briefs it. Trade's counterparty must be a
`KindWorld` member with the vendor capability. A `KindWorld` member **has no
`on:` table, no faction, and is not a target through any hostile door** — it is
what a door and a shopkeeper have in common, and the encounter says so
deliberately. So under reading A the merchant cannot be both:

- as a monster placement, it has a faction and a table, so Persuade reaches it —
  and Trade does not;
- as a `KindWorld` member, Trade reaches it — and it has no table to answer
  with.

One of the two has to move, and which one is not this document's to decide. The
smaller-looking change is to let Trade take a neutral monster placement as a
counterparty, since "non-hostile" is now a stance the run folds rather than a
member kind. Named as an open ruling.

## 7. Open rulings (Kirk)

1. **Reading A or reading B for the first town** (§2). Recommended A; §11 says B.
2. **`when:` on a social row** (§5.1) — allow the second grammar on that key, or
   find the answer inside the existing one.
3. **The merchant's kind** (§6) — teach Trade about neutral monsters, or give
   `KindWorld` a table.
4. **Whether a town run ends at all.** §3 ends it on a fact, which makes leaving
   town a run ending like any other. A town that never ends is a different
   lifetime and the session has never held one.
5. **Whether the king's goblins are a second document or this one.** "The king
   who is being attacked by goblins" is a fight, and this square has no hostile
   side. Whether the attack is a separate run dispatched from here, or an
   `arrives` predicate in this file, is undecided and changes what a town is.
