---
status: DESIGN, proposed 2026-09-22 (Kirk + Fable dream session). NOT SCHEDULED. Every ruling below is Kirk's, cited as "ruled 2026-09-22"; everything undecided is under §9.
journey: rpg-project#326 (Living World) — brainstorm §10 (the journal), §11 (guild worlds)
siblings: ../item-ledger/design.md (the other ledger) · ../town/design.md (its first reader)
law: the die is not the branch source — the persisted consequence is. This is the third grain of the fact system that already exists, not a new system.
---

# The campaign journal — the fact that outlives the run

## 0. Purpose

Intel and dispositions already make every roll a branch: the letter you found
turns the camp, the goblin you leaned on tells you where the vault is. Ruled
2026-09-22: **that machinery is already the branching engine, and the gap is
that nothing outlives the run.** Every fact the run wrote dies with the run, so
the world never learns.

This document adds one grain to the fact model and one law, and nothing else.
The forcing case, in Kirk's words: recover the heirloom from the tomb, then
return it to the king who is being attacked by goblins.

**The heirloom is a fact, not an item** (ruled 2026-09-22). Everyone present at
the run's ending learns it. Facts copy freely, are per-observer, and are never
consumed. Turning it in to the king writes a SECOND fact — `heirloom-returned`
for that character — and the first stays true forever, because it happened. The
item side of the world is a separate ledger, for the economy only:
../item-ledger/design.md.

## 1. The three grains

The fact system has two grains today
(../../dungeon-authoring/world-builder/minds-factions-intel.md, "two grains,
one spelling"). This adds the third.

| grain | who knows it | read by |
|---|---|---|
| **mind** | a faction, through its mind member | `dispositions[].until` |
| **run** | the run's journal — anyone in the run | `arrives`, `endings[].when` |
| **campaign** | a character, across runs; and a faction, across runs | town gates, quest availability, word-spread seeds |

One spelling serves all three: `{ fact: <id> }`. The grain is decided by the
sink, exactly as it is today — `until` asks the mind, `arrives` asks the run.
The campaign grain reaches an author through §4's seeding, so no author ever
writes a new predicate word.

This IS brainstorm §10's journal: append-only, attributed facts; the present
derived by fold; absence means it never happened. Nothing here reverses that
section; §5 reconciles it with §11's re-scoping.

## 2. The file

Nothing in this section is a new authoring key. Both documents below are v4
single-room documents as they compile today.

**Run one — the tomb.** The heirloom is a prop, the ending is the shipped
scenario binding, and the run journal carries `taken:heirloom` when someone
pockets it (../item-ledger/design.md §3 owns the Take verb that writes it):

```yaml
propBindings:
  heirloom: { holdable: true }
exits:
  - { id: entrance, cell: { q: 1, r: 3 } }
endings:
  - { id: recovered, when: { fact: taken:heirloom } }
scenarios:
  recover-the-artifact: { artifact: heirloom, exit: entrance }
```

**Run two — the town.** The king's herald is absent until the party has the
heirloom. `arrives` is a `PredicateSpec` sink today, and `{ fact: … }` on it
asks the run's journal — which §4 has seeded from the campaign grain:

```yaml
factions:
  - { id: crown, mind: king }
dispositions:
  - { between: [crown, party], stance: neutral }
monsterBindings:
  herald:
    arrives: { fact: heirloom-taken }
  king:
    persuade: [{ ability: persuasion, dc: 10 }]
    on:
      persuaded:
        - say: "The house of Aldric stands again. Name your reward."
          fact: heirloom-returned
endings:
  - { id: crown-repaid, when: { fact: heirloom-returned } }
```

`heirloom-returned` is written by the king's own table and lands on every
witness of the Persuade, which is what makes it a fact about the characters who
were there rather than about the party abstractly.

**What is NOT in the file: promotion.** No `promotes:` key, no reward block on
an ending. Promotion is a law (§3), not an authoring choice — an author who
could pick which facts survive would be authoring memory, and memory is a
consequence of who was present.

## 3. Promotion — the one new law

**Ruled 2026-09-22: promotion happens once, at `ended`.** Two halves, and the
second is the load-bearing one.

1. **Each character present at the ending** gets the run-journal facts *in their
   audience* promoted to the campaign grain, attributed to that character, the
   party, the run, and the ending key. A character who disconnected before the
   ending promotes nothing; a character who was there but never in a fact's
   audience does not carry that fact out.
2. **Each SURVIVING faction mind** carries the facts it knows into that
   faction's campaign row. A mind that fell promotes nothing — consistent with
   the no-succession ruling, "accidental succession is still succession".

Half (b) is why this is worth building. Without it the world only ever learns
what players did; with it, the goblin who fled remembers the party. It is the
per-observer testimony rule applied one grain up: the run wrote testimony per
observer, and the ending carries each observer's testimony forward under that
observer's name.

Two consequences worth stating plainly:

- **Nothing is promoted mid-run.** A run with no ending promotes nothing at all.
  Fail closed: the absence of an ending is the absence of a consequence, not a
  partial one.
- **Nothing is ever un-promoted.** No retraction, no decay, no forgetting. "The
  village is burned" is never stored; it folds from "bandits razed the village,
  run 7" and "village rebuilt, run 11" — brainstorm §10's law, unchanged.

The ending key comes along because an ending already IS a content string
(../../run-ending/design.md §6: keys stay content strings, no won/lost enum on
the wire). The row carries that string verbatim; nothing interprets it. This
also stocks that design's own shelf, "post-run persistence — nothing else
writes anywhere yet".

## 4. How a predicate reaches the campaign grain

An author writes `{ fact: heirloom-taken }` in a town document. That predicate
is judged against the RUN's journal, and the fact was written in a different
run. Two ways to connect them: a new predicate word (`{ knew: … }` — a fifth
form in a closed grammar, a second spelling of "is this true", a new refusal in
every sink), or **seeding** — at run start the run's journal is populated from
the campaign journal for the characters present.

**Recommended: seeding.** The run reads campaign facts as if they had happened
in this run, so no new word exists, no sink grows a case, and `arrives`,
`endings[].when` and `scenarios` all reach the campaign grain on the day seeding
lands without a line of change in any of them.

Seeded facts land on the **truth grain**, so `arrives` and `endings[].when` see
them — the intent — while `dispositions[].until` does not, because `until` asks
the faction's MIND and a mind learns only the three ways it learns today. A
party walking in with a campaign fact does not telepathically inform the camp.
That falls out of the existing grain split with no rule of its own.

**What seeding forecloses, named:** a fact that should have been run-only but
was promoted now holds forever, in every later run the character enters, and
nothing can take it back. The mitigation is attribution, not deletion — every
seeded fact carries the character, party, run and ending it came from, so a
predicate that needs to distinguish "in this run" from "ever" can be added later
by reading attribution, without changing the spelling for the ordinary case.
Until such a predicate exists, an author who wants a run-only fact must not let
a character be present at an ending while holding it, which is a real constraint
and is stated here so it is not discovered in a walk.

## 5. Attribution and folds

A campaign row is the fact plus who it belongs to:

```
{ fact, character, party, run, ending, at }      # a character's row
{ fact, faction, site, run, at }                 # a surviving mind's row
```

Brainstorm §10 ruled the journal campaign-scoped, one per persistent party.
§11 re-scoped it the same day: **ONE guild journal**, facts attributed to party
and players. §11 wins, and the reconciliation is that neither "per party" nor
"per character" is a store — **both are folds by attribution**:

| you want | you fold |
|---|---|
| what this character knows, across every party they have played in | rows where `character = X` |
| what this party has done | rows where `party = P` |
| what the guild's world has come to | every row |
| what this faction remembers of the party | rows where `faction = F` |

"Per character across parties" is the fold §11 asked for, and attribution from
birth is why it is free. There is exactly one store.

**A faction's campaign identity is not settled.** A faction id is document-local
(`goblins` in one file is not `goblins` in another). The proposal is the pair
`{site key, faction id}`, stable because a site key already names a document.
Nothing else here depends on which way it goes; §9 carries it.

## 6. Ownership

Ruled 2026-09-22: **rpg-toolkit stores nothing.**

| thing | owner |
|---|---|
| the rule — who gets which facts at an ending | rpg-toolkit, where endings fire. It computes the promotion set from the audiences it already tracks and hands it out. |
| the store — campaign rows, forever | rpg-api persistence |
| the door | the session SDK: one read ("facts for audience X") and one write ("promote these rows") |

rpg-api stays dumb: it persists what it is handed and hands back what it holds.
It never decides who learned anything, because the audience fold is the
toolkit's and lives where the fact was written.

## 7. What this does NOT build

Named so nobody plans on it:

- **No quest tree.** Availability is a predicate over facts; the tree is a
  visualization of the predicate graph, not an engine noun (brainstorm §10).
- **No reputation number.** A number is a fold somebody already computed and
  threw the evidence away. Rows fold; the fold is not stored.
- **No world clock.** Postponed by ruling and still postponed. This journal
  accepts facts from any writer, so a clock arrives later as one more writer.
- **No mind succession.** A fallen mind promotes nothing, by §3.
- **No forgetting, decay, retraction or cap.** Append-only.
- **No new authoring key**, and no second spelling of `{ fact: … }`.
- **No item.** Items are ../item-ledger/design.md, and the two laws in its §5
  are what keep the ledgers apart.

## 8. Shelves (named, empty)

- **Word spreads.** The hold-out shelf (../hold-out/design.md §11) becomes a
  scheduled fact on THIS grain: a run ending `withdrawn` with a live scout who
  saw the party seeds "the king is under goblin attack" a few runs later. The
  mechanism is the arrival primitive at campaign clock instead of round clock.
  Named, not designed; it wants the world-clock question answered first.
- **World goals** (brainstorm §11). Quest machinery at guild scope: predicates
  over guild rows, many parties contributing, folded, plus a wall-clock
  deadline. The fold is §5's; the deadline is the clock we have not built.
- **Fact identifier vocabulary** — wants the typed-ref work. Facts are plain
  strings today and stay plain strings here.

## 9. Open rulings (Kirk)

1. **Seeding grain.** §4 seeds the union of the present characters' campaign
   facts into one run journal, so one character's history unlocks content for
   the whole party. The alternative is per-character seeding with per-observer
   gates, which costs a per-observer predicate evaluation everywhere. Union is
   recommended — they walked in together — but it is Kirk's call.
2. **A faction's campaign identity** (§5): `{site key, faction id}`, or
   something else.
3. **What a party is, across runs.** Attribution names a party; nothing in this
   design says whether a party is durable, ad hoc per run, or a guild roster
   slice. The fold works either way; the row needs the answer.
4. **Whether a wipe ending promotes.** §3 says the ending decides and a run with
   no ending promotes nothing. Whether the party-wipe ending should promote what
   the survivors saw — there are none — or promote the factions' half only, is
   undecided.
5. **Run-only facts** (§4's foreclosure). Whether that constraint is acceptable
   for the first town, or whether attribution-reading predicates are needed
   sooner.

## 10. Seven principles

- **Ownership before mechanism.** One holder per noun: the rule is the
  toolkit's because it owns audiences, the store is rpg-api's, the door is the
  session SDK's, the vocabulary stays the author's.
- **Zero values tell the truth.** No campaign row means it never happened — the
  run journal's own sentence, one grain up. No ending means no promotion. An
  empty faction row means that faction learned nothing, not that it forgot.
- **Fail closed, loudly.** A fallen mind promotes nothing rather than guessing a
  successor. A character absent at the ending promotes nothing rather than
  inheriting the party's. Promotion never happens partially mid-run, so a
  disconnect cannot half-write history.
