---
status: DRAFT for Kirk's return, 2026-09-04 — the configuration shape of a real scenario, seam by seam
journey: rpg-project#326; brainstorm: ../disposition/brainstorm.md; tool 1: ../intel-record/design.md (building)
north star: "we are here to build tools that can be used to tell stories" — the scenario is the test case, the seams are the product
---

# The hold-out — a real scenario, configured

Kirk, leaving for a run: "I would like us to work on configuring a real
scenario. intel is an input into it. we need to assign the value to
something, set the intel of that to something. the configuration shape of
this is as important as proving our intel system works (we already know
that) — it is really the seams and how we inject that data into our
scenario."

So this page is the SHAPE first: the file a DM writes, the form the
designer shows, and the seam each line crosses to reach the run. The
scenario is the hold-out from the disposition brainstorm: the goblin camp is
hostile until its chief learns the party saved the Wiseman; a messenger's
letter carries that fact; hold out until it arrives, get it to the chief.

## 1. The file — everything the DM writes

```yaml
factions:
  - id: goblins
    mind: chief                      # the one whose knowledge is the camp's

dispositions:
  - between: [goblins, party]        # `party` is the standing name for the players' side
    stance: hostile
    until: saved-wiseman             # a FACT; when the faction knows it, the stance flips

intel:
  - id: wisemans-letter
    reveals: { fact: saved-wiseman } # the second `reveals` key, after `door`

place:
  - { id: chief, ref: "dnd5e:monsters:goblin-boss", at: [12,4], faction: goblins }
  - { id: scout, ref: "dnd5e:monsters:goblin",      at: [4,2],  faction: goblins }
  - { id: letter, ref: "dnd5e:props:scroll", at: [1,3], holdable: true,
      holds: [wisemans-letter],
      arrives: { turn: 6 } }         # the messenger; the CLOCK tool's first instance

exits:
  - { id: front-gate, at: [1,3] }

scenarios:
  hold-out:
    convince: goblins                # entity_ref(faction)
```

Read it as three facts and their joins:

- **"assign the value to something"** — `intel[].reveals` gives a record its
  value: a door (tool 1, shipped) or a fact (new key, this slice).
- **"set the intel of that to something"** — `place[].holds` puts the record
  on a thing: a monster or a prop (R6). Here the letter, which arrives.
- **the join** — `dispositions[].until` names the SAME fact id the record
  reveals. That one string is the whole seam between intel and disposition;
  the scenario never mentions the letter.

## 2. What the scenario owns (and does not)

The form for `hold-out` has ONE field: `convince: entity_ref(faction)`.
Its ending is "the faction bound in `convince` is no longer hostile to the
party". Everything else — who the chief is, what flips them, what carries
the fact, when it arrives — is the DM's structure, authored with general
tools, and would be the same structure under a different scenario (rooms
carry no roles; intel belongs to no scenario, R5). The scenario's
description tells the DM how to use them:

> "In this scenario the party must turn a hostile faction. Give the faction
> a mind, set its disposition to hostile until a fact, and place intel that
> reveals that fact somewhere the party can reach — on a messenger, in a
> chest, on a body."

`New(cfg, compiled)` refuses, in form-filler words: no faction bound; the
bound faction has no `mind`; no disposition between it and the party carries
an `until`; no intel record reveals that fact (a hold-out nobody can win).

## 3. The seams — how each line reaches the run

| line in the file | dungeonspec (declares, validates) | Compiled → sessionworld (injects) | encounter (runs) | who reads it in play |
|---|---|---|---|---|
| `factions[].id`, `place[].faction` | ids unique; every faction referenced exists; `party` reserved | `SetupInput.Factions`; each `MemberInput.Faction` | the graph's nodes; `IsHostile(a,b)` asks the graph (rung 1, already) | fight formation, targeting |
| `factions[].mind` | names a monster in that faction | `Faction.Mind` | the presence fold: the faction knows what its mind knows | disposition predicate |
| `dispositions[]` | both sides exist; `until` is a fact id some record reveals (warn, not refuse? — see §6) | `SetupInput.Dispositions` seeds graph edges with an `until` | the STANCE WRITER (rung 2): when the mind's facts contain `until`, rewrite the edge; if a fight is formed between the two, dissolve it | fight formation; behavior (Billy) |
| `intel[].reveals.fact` | fact ids are plain strings, declared here | intel table rides `Compiled.Field` whole (already) | on transfer, a `fact` reveal writes `<fact>` into the receiver's journal with the receiver as audience (the same path `door` uses to write `known:door`) | the disposition predicate; later, quest predicates |
| `place[].holds` on a prop | R6 | `PropInput.Holds` | Hold applies reveals to the holder; presence: a holder in the mind's region teaches the mind | — |
| `place[].arrives.turn` | ≥ 1; the cell is floor | `PropInput.Arrives` / `MemberInput.Arrives` | the arrival scheduler on the turn clock: the placement is absent until turn N, then placed with a beat ("a messenger arrives") | the client draws it when it exists |
| `scenarios.hold-out.convince` | the id is a faction | `scenarios.New` → `Declared{Endings: [TriggerStance{Between: [goblins, party], Stance: friendly}]}` | a fourth trigger, fired by the stance writer | the `ended` beat |

Two seams are new mechanisms; everything else is a field on an existing one:

1. **The stance writer** (rung 2 of integration.md): a predicate over one
   member's facts rewriting a graph edge, plus "a flip dissolves a formed
   fight between those factions". Lives in encounter beside `noticeDown`.
2. **The arrival scheduler**: placements with a turn. Lives in encounter's
   clock; the first instance of the CLOCK tool.

## 4. The cut

- **Step 2 — disposition without the clock.** Factions, `mind`,
  dispositions with `until`, `reveals: {fact}`, presence transfer, the
  stance writer, the fight dissolving, `TriggerStance`, the one-field form.
  The letter simply lies at the gate from turn one. Walkable: hold the
  letter, walk it to the chief, the camp turns mid-fight.
- **Step 3 — the clock.** `arrives: {turn}` and the arrival beat; the same
  walk with the letter arriving at turn 6. The hostage's "turns until
  turned" is its second instance.

## 5. Wire

`PutDungeon` verbatim: no change for the file. New on the session wire:
`GetAtlasResponse` (or roster) needs each member's faction so the client can
colour sides; a `stance` beat `{between, stance}` so the table hears the camp
turn; `arrived` beat for the scheduler (step 3). `ListScenarios` unchanged
in shape (`entity_ref(faction)` is one more kind).

## 6. Open rulings (for Kirk's return)

- **Does a flip dissolve a formed fight?** Proposed yes (brainstorm).
- **`until` naming a fact no record reveals** — refuse at compile (a
  hold-out nobody can win) or allow (the DM may reveal it another way
  later)? Proposed: the SCENARIO refuses it (its quest is unwinnable); the
  dungeon alone allows it (pre-release: allow and show the cost).
- **Presence grain** — "in the mind's region" (ruled by presence) vs
  adjacent vs seen. Proposed: region, the same yardstick Search uses.
- **Where `party` lives** — a reserved faction name for the players' side,
  or a declared faction the DM must write? Proposed reserved: it exists in
  every dungeon whether written or not.
- **The `stance` vocabulary** — `hostile | neutral | friendly` closed enum
  in the file (three words the graph already distinguishes), open later.
