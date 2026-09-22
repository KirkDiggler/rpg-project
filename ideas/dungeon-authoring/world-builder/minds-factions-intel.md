# Minds, factions, dispositions, intel: what an author has today, and how our scenarios are built from it

**Status:** REFERENCE, written 2026-09-22 against rpg-toolkit main `1b9696f9`
(encounter v0.99.0, session v0.101.0) from a file-and-line inventory. It is a
map of what EXISTS, written because "we have been changing so much that I
have lost track" (Kirk). Every claim names the code that makes it true; when
the code moves, this doc is wrong and should be fixed, not trusted. Nothing
here is a proposal.

## The model, in one screen

**A monster has no mind object.** The mind ladder, the presets (Basic, Minded,
Retaliator, Grudge, Fear), the Decider and the Pump were DELETED in the
creature's-table wave (`ideas/creature-table/design.md`, R1–R8). Kirk's reason,
kept in the code: "the mind was a black box with three words on the lid and a
streamer could not open it." What a creature has instead is exactly three things:

| Thing | What it is | Where it comes from |
|---|---|---|
| a **table** | weighted answers per trigger: what it does when it has time, and what it says or does when leaned on | authored `on:` blocks, layered |
| a **temper** | one word that loads the die: `soldier`, `coward`, `aggressive` | authored, or dealt from a faction mix |
| **holdings** | what it has seen, what has been done to it, what intel it carries | never authored; the run writes them |

**One evaluator** (`behavior.Pick`, `mind/behavior/table.go`) rolls the table.
Every roll writes an `answered` beat with every candidate's weight, so the
streamer can open the box.

**Four layers, nearest wins wholesale, key by key:**

1. the rulebook's one generic `time` table (behind every monster ref);
2. the faction's `on:` and `temper` (root `factions[]`);
3. the creature's own `monsterBindings.<id>` (`on`, `temper`, `actions`,
   `holds`, `intimidate`, `persuade`, `arrives`);
4. experience: deeds and sightings, read per pick, never stored as state.

"Wholesale" means: a creature that authors `on: { time: [...] }` REPLACES the
faction's whole `time` list, it does not add to it. The cost is visible in the
file on purpose.

**The site scope belongs to no creature:** root `factions[]`, `dispositions[]`,
`intel[]`, `concealments{}`, `exits[]`, `endings[]`, `scenarios{}`.

## The keys, and what the engine does with each

### `monsters[]` — identity
`{ id, ref, cell: {q,r}, faction }`. `faction` is the one thing a binding
cannot override (four goblins in one faction must not need four orders blocks).
Empty faction = the reserved `monsters` side. `party` is refused as a faction.

### `monsterBindings.<id>` — orders
Every key optional; the id must name a live monster.

- **`on:`** — five trigger keys and no more: `intimidated`,
  `intimidate_failed`, `persuaded`, `persuade_failed`, `time`. An entry is
  `{ weight, say, when, <one outcome word> }`.
  - Outcome words: `fact: <id>` and `flee: {}` on the social keys;
    `hold: {}`, `attack: <sel>`, `toward: <sel>`, `away: <sel>` on `time`.
    Exactly one per entry, or none (a bare `say`).
  - `when:` is `time`-only and is ITS OWN grammar (see "Two grammars"):
    `{ enemy: reach | seen | remembered | none }` or
    `{ attacked | intimidated | persuaded | fled: { within: N } }`.
  - Selectors: `enemy`, `attacker`, `actor`. (`{at: [col,row]}` is refused in
    the single-room dialect until the sites layer.)
  - Words designed and refused by name: `alarm`, `lure`, `pretend`, `patrol`,
    `tell`.
  - **`flee: {}` moves nobody.** It lands a `fled` deed; the running is the
    creature's own `time` row `{ when: { fled: { within: 3 } }, away: actor }`.
    A table with no `fled` row does not run. Visibly the author's choice.
- **`temper`** — `soldier` (100 across the board), `coward` (attack/toward 50,
  away/flee 300), `aggressive` (attack/toward 300, away/flee 25, hold 50).
  A percent multiplier per word and NOTHING else: no entries, no triggers, no
  memory. An unknown word refuses the spawn.
- **`actions`** — full weapon refs. Non-empty REPLACES the stat block's list;
  the ORDER is the instruction (first action in reach is used).
- **`holds`** — intel record ids the creature carries from spawn. Holding is
  NOT knowing: nothing is revealed until the record changes hands.
- **`intimidate` / `persuade`** — `[{ ability, dc }]`, the approach grammar.
  Absent = the rulebook derives the DC from the monster's passive Insight.
  Authored list wins whole. `[]` is refused.
- **`arrives`** — a predicate; until it holds the creature is ABSENT (no
  roster, no clock, no atlas), then placed at its cell with an `arrived` beat.

### `factions[]` — the sides
`{ id, mind, temper, on }`.

- **`mind: <monster id>`** means "the faction knows what this member knows".
  It is a KNOWLEDGE HUB, not a commander: it gives no orders, picks no targets,
  is consulted by nothing at turn time. It is read in exactly two places: the
  stance fold and the flip graph. A faction of one gets its member as mind
  automatically; a faction of many with no mind cannot learn, and the file
  says so. A mind that falls is not succeeded: the faction stops learning
  (ruled, "accidental succession is still succession").
- **`temper`** — a word, or a MIX `{ coward: 2, soldier: 1, aggressive: 1 }`
  dealt once at the door through the shared dice, with a `tempered` beat so
  the streamer sees which goblin came out the coward.
- **`on`** — the same grammar as a creature's, inherited by every member
  (layer 2).

### `dispositions[]` — who fights whom
`{ between: [a, b], stance, until }`. Stances: `hostile`, `neutral`, `allied`.
Defaults when nothing is declared: a faction is allied with itself, `party`
is hostile to every faction, every other pair is neutral.

**What a stance does:** hostile = fights form, the member blocks movement,
it is a valid `enemy` target. Neutral = none of those; the creature still
runs its `time` table but reads `enemy: none` however crowded the room, so
it holds. Allied is authorable but today indistinguishable from neutral.

**`until` takes ONLY `{ fact: <id> }`** in this build, and a hostile pair
that turns becomes NEUTRAL, never allied. `{round}`, `{down}` and `{stance}`
on an `until` are refused with one sentence, in the file and again in the run.

### `intel[]` — records and facts
`{ id, reveals: { fact: <word> } | { concealment: <id> } }`.

- A **fact is declared by mention**: a record's `reveals.fact` and a
  disposition's `until.fact` just agree on the word. Facts are not prefixed;
  they belong to the story.
- A record is carried by `monsterBindings.<id>.holds` or
  `propBindings.<id>.holds`. Intel COPIES: a scroll handed on teaches the
  next holder too.
- **Three ways a fact is learned:** loot a body or pick up a prop (free, no
  check); a creature's own `on:` entry with `fact:` teaches every WITNESS of
  the social verb; PRESENCE TRANSFER, a holder standing in the same region
  as a faction's mind teaches that mind (and the reverse).
- **Two grains, one spelling:** on a disposition's `until` the fact must be
  known by the faction's MIND (the audience grain); on `arrives` and
  `endings[].when` it need only exist in the run's journal (the truth grain).
  This is why reading the letter to the scout flips nothing.

### The predicate grammar (one key, closed)
`{ round: N }` (a fight's own clock; never holds outside a fight),
`{ down: <monster id> }`, `{ fact: <word> }`,
`{ stance: { between: [a,b], is: hostile|neutral|allied } }`.

Sinks: `dispositions[].until` (fact only), `monsterBindings.<id>.arrives`,
`propBindings.<id>.arrives`, `endings[].when`. Arrivals always fire before
endings at the same event, so `ended` stays the story's last word.

### Two grammars, do not conflate
`PredicateSpec` above feeds `until` / `arrives` / `when` on an ending.
An `on:` entry's `when:` is `WhenSpec`: enemy bands and deed spans. They are
sealed separately. The v4 gameplay-shape doc's "sinks" table lists them
together; that was loose prose, and this doc is the correction.

### Intimidate and Persuade today
One shared body. The session prices the verb, derives or takes the DC, rolls
(no proficiency = disadvantage, our divergence from RAW). The encounter is
TOLD beaten. Audience = witnesses from the actor's cell; the target must be
one. A beat goes to every witness landed or not. On a hit: a deed on every
witness, then the target's table rolls under `intimidated` / `persuaded`
(or the `_failed` twin), which may teach a `fact` to every witness and/or
land `fled`. **Nothing sets a flee flag or a stance.** The verb lands
testimony; what it is worth is the author's table.

### How a monster remembers
Three per-observer stores, none of them live state: DEEDS done TO it (read by
`within: N` and the `attacker`/`actor` selectors), SIGHT testimony (`enemy:`
bands, remembered positions), and FACTS in the journal. No forgetting, no
decay, no cap beyond the author's `within` window.

## The scenarios, built from those keys

Each is a shipped v4 fixture in dungeonspec testdata; the YAML below is the
load-bearing part only.

### 1. The front-room goblin: lean on it and see what it is made of
`world-builder-v4-front-room.yaml`. Everything is the faction's; the creature
adds only its price.

```yaml
factions:
  - id: goblins
    temper: { coward: 2, soldier: 1, aggressive: 1 }     # dealt at the door, `tempered` beat
    on:
      intimidated:       [{ weight: 70, say: "Fine! FINE.", fact: goblin-cowed },
                          { weight: 30, say: "Boss! BOSS!", flee: {} }]
      intimidate_failed: [{ say: "You? Threaten ME?" }]
      persuaded:         [{ say: "...fine. But you never saw me.", fact: goblin-cowed }]
      time:              [{ when: { fled: { within: 3 } }, away: actor, weight: 3 },
                          { when: { enemy: reach }, attack: enemy },
                          { when: { enemy: seen },  toward: enemy },
                          { when: { enemy: none },  hold: {} }]
dispositions:
  - { between: [goblins, party], stance: hostile, until: { fact: goblin-cowed } }
monsterBindings:
  front-goblin: { intimidate: [{ ability: intimidation, dc: 12 }] }
```
What happens: Intimidate beats 12 → the goblin's table rolls under
`intimidated`, loaded by the temper it drew. 70 percent it says "Fine" and
teaches `goblin-cowed` to every witness; the faction of one has that goblin
as its mind, so the pair turns NEUTRAL and the fight dissolves. 30 percent it
lands `fled` and spends three rounds walking away, because its `time` table
says so. A coward triples the `flee` row before the roll.

### 2. The hold-out: a letter turns the raiders
`world-builder-v4-raider-camp.yaml`, `…-raider-letter.yaml`.

```yaml
factions:     [{ id: raiders, mind: chief }]
dispositions: [{ between: [raiders, party], stance: hostile, until: { fact: saved-wiseman } }]
intel:        [{ id: wisemans-letter, reveals: { fact: saved-wiseman } }]
monsterBindings:
  chief:     { holds: [wisemans-letter] }                 # camp version: loot it off him
  messenger: { holds: [wisemans-letter], arrives: { round: 6 } }   # or it walks in
propBindings:
  letter:    { holdable: true, holds: [wisemans-letter] } # letter version: it lies on the floor
scenarios:   { hold-out: { convince: raiders } }
```
What happens: the record has to REACH THE MIND. Pick the letter up: you know
the fact, the raiders do not. Stand in the chief's region holding it:
presence transfer teaches the chief, the pair turns neutral, a `stance` beat
goes to everyone, fights with no sides left dissolve, and the `hold-out`
ending (sugar for `stance … is: neutral`) fires. Reading it to the scout does
nothing; the scout is not the camp's mind. Kill the chief first and the
faction can never learn: no succession, by ruling.

### 3. The clock: reinforcements when the chief falls, the letter on round 6
`world-builder-v4-raider-camp.yaml`.
```yaml
monsterBindings:
  reinforcement-1: { arrives: { down: chief } }
  reinforcement-2: { arrives: { down: chief } }
  reinforcement-3: { arrives: { down: chief } }
  messenger:       { arrives: { round: 6 } }
endings: [{ id: held-out, when: { round: 6 } }]
```
Until then they are absent, not hidden. A ring (A waits on B, B on A) and a
creature waiting on its own fall are refused at the author's path.

### 4. Recover the artifact: the heirloom and the way out
`world-builder-v4-tomb-heirloom.yaml`. No factions, no intel: this is the
control. `propBindings.heirloom: { holdable: true }`, `exits: [{ id: entrance,
cell: <partyStart> }]`, `scenarios: { recover-the-artifact: { artifact:
heirloom, exit: entrance } }`. The run ends when someone Leaves at the
entrance holding the heirloom.

### 5. The secret vault: a concealment, found three ways
`world-builder-v4-tomb-vault.yaml` (engine landed 2026-09-22, rpg-project#490).
```yaml
concealments:
  vault:
    checks: [{ ability: perception, dc: 15 }, { ability: investigation, dc: 13 }]
    cells:  [...]                      # the vault's floor, still in walkableHexes
    props:  [vault-door, heirloom]
intel:
  - { id: vault-map,  reveals: { concealment: vault } }
  - { id: hall-notes, reveals: { concealment: vault } }
monsterBindings: { captain: { holds: [vault-map] } }
propBindings:    { hall-scroll: { holdable: true, holds: [hall-notes] } }
```
Search from a cell touching it rolls the checks; loot the captain or pick up
the scroll and the map reveals it; being shoved through the wall reveals it.
An unaware player sees wall and cannot walk through it. Whoever perceives the
door open, or someone standing inside, learns it. `notice` is carried and
unread until slice 2.

### 6. The coward's rout, without a single new key
Give a faction `temper: coward` and the default `time` table. Intimidate one:
`flee` is loaded 300 percent, it lands `fled`, and `away: actor` at weight 3
dominates its next three picks. Every step is in the `answered` beat with the
die that chose it.

## What does NOT exist, so nobody plans on it

| Wanted | Status |
|---|---|
| Dialogue, choices, offers from a creature | nothing; only `say:` on a table row (R5 direction in v4-gameplay-shape.md, not designed) |
| Allied flip | ruled out: a side stops being hostile, never joins the party |
| Word spreads (a non-mind member learns, the mind N turns later) | hold-out shelf §11 |
| Mind succession | shelved by ruling |
| Directed dispositions (A hostile to B, not B to A) | pair is unordered on purpose |
| Per-character stance (Deceive, disguise, Take command) | not designed |
| `until` on round / down / stance | refused by name |
| Regroup, Alarm, Lure, Pretend, Patrol | designed words, refused by name |
| An `attacked` TRIGGER key | deliberately absent: `attacked: {within: N}` on `time` is the answer |
| Entry-list MERGING across layers | ruled out; nearest wins wholesale |
| A fourth temper, or a temper with triggers/memory | R5 of the creature's table |
| Kind-specific default tables | the door `table.Default(ref)` exists; one generic table stands behind it |
| A monster rolling a check (Insight vs a disguise) | inexpressible today |
| `count` on a placement | hold-out shelf |
| `notice` passive tell | carried, unread; slice 2 of #490 |
| Web pass-through of the v4 root keys; placed props on the wire | rpg-dnd5e-web#1171; rpg-api-protos#351 |

## Where the rulings live
`ideas/creature-table/design.md` (the table, tempers, layers) ·
`ideas/shenanigans/` (Intimidate, the front-room goblin, the verb ladder) ·
`ideas/living-world/hold-out/design.md` (factions, dispositions, mind,
predicates, reserve; §11 shelf) · `ideas/living-world/intel-record/design.md`
(records) · `ideas/monster-intel/design.md` (what a monster knows) ·
`ideas/dungeon-authoring/world-builder/v4-gameplay-shape.md` (the v4 keys) ·
`ideas/dungeon-authoring/world-builder/concealing-shape.md` (concealments).
