# The creature's table — one authored surface for what a creature does

**Status:** SHIPPED 2026-09-18 (tracking rpg-project#466; RULED R1–R8 by Kirk; walked by a scripted pass Kirk could not take himself, proven from the record; review round skipped by Kirk: "we can merge away").

| Repo | Release |
|---|---|
| rpg-api-protos | v0.1.200 (#346: `AnswerKey`, `AnswerCandidate`, `Answered.key/candidates/temper`, `Temper` enum with `TEMPER_NONE`, `Tempered` beat with `faction`) · v0.1.202 (#347: `Stayed` beat) |
| rpg-toolkit | mind/behavior v0.5.0 (#1819, the table primitive) · rulebooks/dnd5e v0.182.0 (#1815) · encounter v0.90.0 (#1816) · resolution v0.54.1 (#1821) · session v0.97.0 (#1818) · `rulebooks/dnd5e/behavior` deleted (#1820) · hook fix #1822 |
| rpg-api | dev de83ffd6 (#1005) |
| rpg-dnd5e-web | dev 20330c9a (#1123); story rendering of `time` picks and `tempered` is web#1122; picker/header/raw-id web#1128 |
| rpg-project | author's guide `docs/howto/author-a-creature.md` (#474); corrections #469, #472, #473 |

Supersedes the mind ladder and the decider as the way a creature decides.

**Where it came from (Kirk, 2026-09-18, thinking out loud, then adopted):** "the answer table
can be where choices come from … I liked the mind behavior to give different monster types
different behaviors … maybe we had 4 goblins all with the same answer table. maybe the answer
table was the instructions given to the group but we got a coward, couple soldier types and
an aggressive type. all goblins, all the same table, but would act different."

**Lane:** toolkit. One small additive proto PR (the pick's arithmetic on the beat). api and web
repin only.

## The one sentence

A creature decides by rolling on an authored weighted table, and four things load that die:
the rulebook's default for its kind, the author's orders, its own temperament, and what it has
seen and suffered.

## Why now, and why this and not the mind

Three things claim to say what a creature does today, verified against rpg-toolkit
origin/main on 2026-09-17:

| Surface | What it is | Who can change it | State |
|---|---|---|---|
| The mind ladder (`mind/behavior`, `rulebooks/dnd5e/behavior`) | presets in Go (retaliator, berserker, coward) with feel numbers, five rungs, verbs `Pass Attack Toward Away`, a `Mind` interface `Judge/Name/Rank/Keep` | us, in Go | drives fights only, through `TurnDriver` |
| The decider (`encounter.Decider`, `Pump`, `Snapshot`, `Intent`) | an older world-roam seam: a cell and holdings in, `MoveTo`/`Hold` out | us | wired to nothing; session never registers one; `Pump` is called by a workbench |
| The answer table (`on:` in dungeonspec, `Answered` beat) | per verb, per outcome, weighted entries of a sealed vocabulary; the world rolls | the author | shipped, walked: "it feels like D&D" |

The table is the only one that is a tool to tell stories, which is the product. The mind is a
black box with three words on the lid and a streamer cannot open it. Pre-release, no game runs
on the mind, and its useful parts are mechanisms the table's words can drive. So the mind and
the decider are retired, not layered under.

What the mind got right is kept: **the outcome of anything is testimony the creature holds,
never a flag** (the shenanigans law), and **who a creature believes is where** comes from
perception, per observer. The table reads those. It does not replace them.

## The shape

### 1. One table per creature, in layers (R3, R4)

A table is a map from **trigger** to a list of weighted **entries**. Three layers supply
tables, and for each trigger key **the nearest layer that names it wins wholesale**. There is
no merging of entry lists, so an author never has to reason about what was added to what.

1. **The rulebook's default table for the monster's kind.** Content, not Go: a thug's table,
   a goblin's table, shipped with the rulebook. This is why a placement with no `on:` still
   fights.
2. **The author's orders**, on a faction (inherited by every placement in it) or on a
   placement (nearest). Today's `on:` block, unchanged in spelling.
3. Nothing else. Temperament (§3) is not a layer of entries; it loads the die.

### 2. Triggers, entries, words, selectors (R6)

Every vocabulary here is **sealed and grows one word per use case**, the law the shenanigans
README already keeps. What is listed is what the front room, the shipped shenanigans, and the
three shipped movement spells need. Nothing is listed ahead of a use.

**Triggers.** Two kinds, and the difference is when the roll happens.

| Trigger | Fires when | Kind |
|---|---|---|
| `intimidated`, `intimidate_failed`, `persuaded`, `persuade_failed` | the social verb resolves against this creature | event: the pick executes now, inside the verb (shipped) |
| `time` | the creature has time: its turn in a fight, or a round of the world (§5) | time: the pick is one turn's worth of doing |

No `attacked` event trigger: what a creature does about being attacked is decided when it
next has time, which is what the retaliator already did through its grudge. No `arrived`: an
arrival that should go somewhere is a `time` entry with a `when` (below), a standing order,
not a memory.

**Entries.** `{ weight, say, when, <one word> }`. `weight` omitted is 1. `say` is carried
verbatim. **`when` is a condition on what this creature holds**, and an entry whose `when`
does not hold is not on the table for this roll. Conditions:

| `when` | Holds when |
|---|---|
| `enemy: reach` | an opposed member is within this creature's reach (the same reach `attack` tests) |
| `enemy: seen` | an opposed member is in sight and **none is in reach** |
| `enemy: remembered` | none in sight, but one is held from an earlier sighting |
| `enemy: none` | none of the above |

The four `enemy:` bands are **exclusive by definition**, so an author never writes two entries that
both hold for one situation without meaning to. (Corrected during the build; see below.)
| `<deed>: { within: N }` for `attacked`, `intimidated`, `persuaded`, `fled` | this creature holds that deed against itself, landed at most N rounds ago (the clock's unit, §5). This is the preset's "patience", moved from Go into the author's sight |

**Words.** One per entry.

| Word | What happens | Legal on |
|---|---|---|
| `fact: id` | witnesses learn the fact (shipped) | event |
| `flee` | this creature holds `fled` from the actor, at now. It does **not** step here: the verb pays a round (§5) and the creature's `time` table, reading `fled: {within: N}`, does the running. The shipped one-shot `fleeFrom` walk goes | event |
| `hold` | pass | time |
| `attack: <selector>` | strike the selected member; refused off the turn clock, since an enemy in reach is a fight sight already formed | time |
| `toward: <selector>` | walk toward the selected member's believed position, or an authored cell, the turn's movement | time |
| `away: <selector>` | walk away from it, the turn's movement (the coward's run; Command's and Dissonant Whispers' compelled walks keep their own engine-driven `Routed` path and do not pass through the table) | time |

Named and **not built**: `alarm`, `lure`, `pretend`, `patrol`. They keep their refusal text.

**Selectors.** `enemy` (nearest opposed member in sight, else the nearest remembered),
`attacker` (who last landed `attack` on me), `actor` (the actor of the deed the `when`
named, so `fled` pairs with `away: actor`), `at: [col, row]` (an authored cell, for
`toward` only). Opposition is the stance graph's answer (`opposed(a, b)`), projected onto
what the creature sees and remembers. **A mind is never handed a target it is not opposed
to**, which is what keeps the neutral goblin from advancing on the party the first time it
has time.

**The rulebook's default thug**, as content, to show the shape:

```yaml
on:
  time:
    - { when: { attacked: { within: 3 } }, attack: attacker, weight: 3 }
    - { when: { enemy: reach },            attack: enemy }
    - { when: { enemy: seen },             toward: enemy }
    - { when: { enemy: remembered },       toward: enemy }
    - { when: { enemy: none },             hold: {} }
```

**The front room, rewritten on this shape.** The goblin keeps the shipped four social keys.
The bandits, so the arrived thug walks to the front room without a rumour nobody carried:

```yaml
place:
  - id: thug-1
    ref: thug
    faction: bandits
    at: [12, 4]
    arrives: { fact: cellar-is-clear }
    on:
      time:
        - { when: { enemy: none }, toward: { at: [3, 4] } }   # the front room
```

Its `time` key replaces the default's wholesale, so this thug does not fight from the
table. That is the author's cost of overriding a key, and it is visible: the entries the
default had for `enemy: seen` are not there. (A fight still forms on sight; inside it the
compelled and default machinery of the fight applies.) An author who wants both writes both.

### 3. Temperament loads the die (R5)

Four goblins, one table, four behaviours. A temperament is **a weight profile and nothing
else**: a sealed word naming a map from table word to multiplier, applied to every weighted
pick this creature makes.

| `temper` | attack | toward | away | flee | hold |
|---|---|---|---|---|---|
| `soldier` (and absent) | 1 | 1 | 1 | 1 | 1 |
| `coward` | ½ | ½ | 3 | 3 | 1 |
| `aggressive` | 3 | 3 | ¼ | ¼ | ½ |

Numbers are content, in the rulebook beside the default tables, and a walk tunes them. The
goblin's authored `intimidated: [70 cower, 30 flee]` becomes 70 against 90 for a coward
(runs 56%) and 70 against 7.5 for the aggressive one (runs 10%). A heavily loaded die makes a
goblin act like a thug, with the same orders.

Authored per placement (`temper: coward`) or **dealt from a mix on the faction**:
`temper: { coward: 1, soldier: 2, aggressive: 1 }`. The deal happens at spawn, through the
shared dice with the faction as the die's entity, and the result is a beat, so the streamer
sees which goblin came out the coward.

**What temperament may not do, ruled on Kirk's "I go with your lean":** it does not add
entries where a table is silent, it has no triggers, and it holds no memory. Silence in the
orders means the rulebook default speaks, and the multiplier applies to that default's
entries, which already contain a way out. The moment temperament grows a trigger or a span, it
is the mind again under a new name.

### 4. Experience is the fourth layer, and it is already built

Two soldiers with identical orders and temperament still differ because one watched the
fighter drop its friend, and only its `attacked: {within: 3}` holds. What a creature has
seen (sightings, memories), suffered (deeds against it, each with an `At`), and been told
(rumours, later) is the holdings perception already keeps per observer. The table reads them
through `when`. No new memory is built; the deed-and-clock work is what made this cheap.

### 5. Time: the world clock advances because the party acts (R1, R2)

The `time` trigger is the world clock slice, absorbed. Today the clock exists, one per
encounter, and **nothing a player does advances it**: Move, Search, Unlock, Intimidate and
Persuade only stamp its reading; it moves on a fight round wrapping and inside the unwired
`Pump`. (The goblin design's "Search and Unlock already spend there" was false.)

The unit is **one round**, which is what the tick already means to every stamp and every
span. A world-clock verb pays its time **after its outcome lands**:

| Verb on the world clock | Displacement | Driver |
|---|---|---|
| Move | one per pace: every `SpeedFeet / 5` cells the mover walks; the remainder carries on the member, persisted | the mover |
| Intimidate, Persuade, Search, Unlock, Interact, Loot, Cast, anything the turn clock prices as an action | one | the actor |
| a fight round wrapping | one **per bubble member** | each bubble member |

**The driver is always the member, never `"world"`.** The clock accrues by driver as
max, not sum: four players walking six cells together is one round, not four. The two shipped
sites use the literal `"world"`, which would make a fight the front runner forever (after ten
rounds of fighting, a player's first walk would raise nothing until they had walked ten
paces). Both change.

**Standing still is free**, said plainly. A party that talks to the goblin and waits sees
nothing move. That is the roguelike clock `play/clock` chose ("advances only because players
act") and the encounter's C5 (no goroutines, no timers). A wait or rest verb reports
displacement like any other, later, when a use case brings it.

**When time passes, the world thinks**, inside the same call that raised the high-water,
after the verb's own beats and before its sight refresh: every standing monster on the world
clock with budget is given, per unit, one turn's worth (`AttacksLeft: 0`, `MovementFeet:
SpeedFeet`), rolls its `time` table, executes, spends one. Then the verb's one sight refresh
runs, and a creature that walked into an enemy's sight joins a fight by the shipped path. A
fight round wrapping is such a raise, so **creatures outside a fight close one round at a
time while it runs**, which is the primitive Alarm needs.

**Rejected shortcut:** one unit per Move verb regardless of length. The client's path length
would set the world's speed.

### 6. The roll is seen (R7)

A pick is a roll through the shared dice with **the creature as the die's entity** (every
die knows whose it is). The beat carries the eligible entries, each with authored weight,
temperament factor and product, the total, the face, and the entry that fired. `time` picks
ride the same beat as social answers. Full data down the log until v1; the story log shows the
line and the act. This is the one additive proto change: the factors on the answer beat, and
`time` as a trigger the beat can name.

### 7. What goes, so the path forward is clear (R8)

Kirk: "when we do get started we will want to make sure to clean out the dead ends and make
the path forward clear." Deleted in the wave, not deprecated, not left beside:

| Dead end | Replaced by |
|---|---|
| `encounter.Decider`, `Snapshot`, `Intent`, `IntentMoveTo`, `IntentHold`, `JoinInput.Decider`, `MemberInput.Decider`, the `deciders` map, `Pump`, `PumpInput/Output`, the workbench `pump` command, README §"What Pump guarantees you" | the `time` trigger on the world clock (§5) |
| `mind/behavior`: the ladder, `Decide`, `Mind{Judge,Name,Rank,Keep}`, `Verb`, the `Game` | **the table itself, in the same module** (RULED 2026-09-18, below): `Table`, `Layer`, `When`, `Temper`, `Pick`, the deed words. The encounter supplies `Facts` and executes words |
| `rulebooks/dnd5e/behavior`: `Basic`, `Minded`, `Retaliator`, `Grudge`, `Fear`, `Excuse`, the preset words, `mindFor` | the rulebook's default tables (content) and `temper` profiles |
| `MemberInput.Mind` / `MonsterView.Mind` and the `mind` word on monster sheets | `temper` on the placement or dealt from the faction |
| `encounter.fleeFrom` and `answerCauseFlee` | `flee` lands `fled`; the `time` table runs |
| `dungeonspec.laterWords["tell"]` stays; `alarm`, `lure`, `pretend` stay refused | unchanged |
| ADR-0043 §"What already exists" (two seams) | amended: one seam, `TurnDriver` renamed `Driver`, consulted on both clocks; a note that the table is the driver's policy |

Kept, because they are mechanisms the words drive: `TurnDriver` as the seam (one
implementation, the table), `Striker`, `Routed` and the compelled-turn driver, `Route`/`Direct`
and the `MoveAway`/`Toward` policies, perception holdings and `Report`, deeds with `At`, the
stance graph, arrivals, the Announcer.

## What the build corrected (kept visible)

- **`enemy: seen` could not close.** As first written, `seen → attack` and `remembered → toward` were the only movement entries, and `attack` out of reach is a pass by the driver's own law (it does not turn one word into another). A creature that saw the party across a room stood still forever; the raider camp's chief never left his door. Found by the session builder against the hold-out tests. Fixed as a vocabulary ruling, not content: `enemy:` gains `reach`, and the bands are exclusive. The shipped default is reach → attack, seen → toward, remembered → toward, with `fled` and `attacked` above them.
- **The table is a mind primitive, not an encounter feature** (RULED by Kirk 2026-09-18). The wave first built the evaluator inside `rulebooks/dnd5e/encounter` because `on:` lived there, and then hollowed `mind/behavior` on "no reader". Wrong home: weighted triggers, `when` over holdings, a temperament profile and a seen pick know nothing of D&D. Kirk: "that it is not dnd specific makes it a welcome addition to the mind package which can be used in any rulebook and is good validation of its composability." Moved in-wave: `mind/behavior` holds `Table`, `Layer`, `When`, `Selector`, `Temper`, `Facts`, `Pick` and the deed vocabulary; the encounter builds `Facts` from its view, executes the words, and owns the dialect, persistence and beats.
- **An unconditional entry competes on every roll.** The first default (and this doc's own thug example) ended in `{ hold: {} }` with no `when`, so it was eligible beside `attack` and a skeleton in reach did nothing half its turns. Found by the api builder on the real stack. Every entry in the shipped default now carries a `when`; the engine already answers a silent table with hold and says so in the beat.
- **A placement's key replaces the default's key wholesale, and that cost is real for an author.** The bandits' one-line standing order removed their fighting entries. Written as the design says (both, on the placement); whether a placement's entries should merge in above the default's is a question for Kirk after the walk.
- **The rulebook's default ships one generic table**, not one per kind. The design's "a thug's table, a goblin's table" is the door (`table.Default(ref)`), and only the generic stands behind it until a kind needs its own.
- **The default's `when:` words are the author's past tense** (`attacked`, `intimidated`, `persuaded`, `fled`); the encounter maps them onto its deed kinds at compile in one place. The first build validated against the deed kinds and refused the design's own example.
- **The default source is headerless.** `CompileTable` takes the contents of an `on:` mapping; the first content shipped the header.
- **`rulebooks/dnd5e/resolution` is a fifth toolkit PR.** It imported `Decider`. The wave plan missed it.
- **The roller is a construction-time capability** on the encounter, optional and refused at the roll, because a fight round raises the clock inside `EndTurn`, which takes no die.
- **A world round refreshes sight itself.** Five verbs that pay a round deliberately refresh no sight; once a round can move somebody inside them, that stopped being safe.
- **An authored cell is walked onto**, not up to. A member anchor is stood beside.
- **Open word, not built:** a creature whose `toward: enemy` reaches the remembered cell and finds nobody holds. "Walk through that door and look" is a later word.
- **On the wire**, temperament is an enum with an explicit `TEMPER_NONE` (zero stays "the producer failed"), and the dealt-temperament beat names the faction whose die was thrown.

## Ownership

| Noun | Charter | Holder |
|---|---|---|
| What a creature does | its table, layered rulebook → faction → placement | dungeonspec compiles; encounter holds the compiled table per member |
| The roll | the shared dice, entity = the creature | encounter, through the Roller |
| Temperament | a weight profile by word | rulebook content; the placement or the faction's mix names one |
| What a creature knows | holdings, per observer | mind/perception (unchanged) |
| Whom it opposes | the stance graph | encounter, projected onto the view |
| Time | `play/clock.Tick`, one per encounter | encounter, advanced by verbs at their outcome |

## The seven principles, run

- **Ownership before mechanism.** The author owns policy; the engine owns rolls, paths,
  sight and time. Session adds no logic.
- **Zero values tell the truth.** No `temper` is a soldier; no `on:` is the rulebook default;
  `weight` absent is 1; an entry whose `when` fails is absent from the roll, not weighted 0.
- **Fail closed loudly.** `attack` off the turn clock refuses; an unknown word, selector or
  `when` refuses at compile; a table with no eligible entry on `time` is `hold`, and says so
  in the beat.
- **Assume it already exists.** The table, weights, `Answered`, holdings with `At`, the stance
  graph, pathing, `Tick` budgets and `Ready` all exist. The slice wires them.
- **A gap is not a license to widen.** No `attacked` event, no `arrived`, no rumour, no
  patrol, no wait verb, no fourth temperament, no entry merging across layers.
- **Cite the text, not the number.** ADR-0043's reason for two seams was `Decider`'s thin
  vocabulary; that reason has lapsed and the ADR says so, rather than being routed around.
- **Delete, don't strand.** §7.

## Rulings

- **R1** Time passes only because the party acts: a Move pays one round per pace, an action
  pays one round, standing still is free. *(Recommended yes.)*
- **R2** The unit is the round and every advance names its member as the driver. *(Yes.)*
- **R3** The table is the one surface for what a creature does; the mind ladder, the presets,
  the decider and `Pump` are deleted (§7). *(Yes. Kirk 2026-09-18: "this is the right
  direction".)*
- **R4** Three layers, nearest key wins wholesale: rulebook default → faction → placement.
  *(Yes.)*
- **R5** Temperament is a weight profile only: no entries, no triggers, no memory; authored
  per placement or dealt from a faction mix through the shared dice. *(Yes. Kirk: "i go with
  your lean".)*
- **R6** The vocabulary sealed as in §2: triggers `intimidated intimidate_failed persuaded
  persuade_failed time`; conditions `enemy: seen|remembered|none`, `<deed>: {within}`;
  words `fact flee hold attack toward away`; selectors `enemy attacker actor at`. *(Yes.)*
- **R7** Every pick is a roll with the creature as the die's entity, shown with its arithmetic
  on the beat. *(Yes.)*
- **R8** The deletions in §7 land in the same wave, with ADR-0043 amended. *(Yes.)*

## Build order, once ruled

1. rpg-project: this doc merged; tracking issue.
2. protos: factors on the answer beat, `time` nameable on it. Kirk merges.
3. rpg-toolkit, one module per PR, on pseudo-versions, deletions inside the PR that
   replaces each thing:
   - `rulebooks/dnd5e/encounter` (with dungeonspec): table compile (`time`, `when`, words,
     selectors, `temper`, faction mix); table evaluation and the seen roll; `time` on the
     world clock (pace, action cost, drivers by member, the world thinks at every raise incl.
     the round site); `Opposed` on the view; `Decider`/`Pump`/`fleeFrom` deleted; ADR-0043
     amended; workbench `pump` removed.
   - `rulebooks/dnd5e` content: default tables per shipped monster; the three `temper`
     profiles.
   - `rulebooks/dnd5e/behavior`: becomes the one `Driver` that evaluates the table; presets
     deleted. `mind/behavior`: deleted (or reduced to what perception needs; the builder
     reports which).
   - `rulebooks/dnd5e/session`: repin; `Mind` on spawn replaced by `temper`; the dealt
     temperament beat.
4. api and web: repin; the content YAMLs (front room, reference dungeons) move `mind` words
   to `temper` and gain the bandits' `time` entry.
5. Kirk walks the front room: four goblins dealt from a mix, same orders, one runs and one
   does not; the party walks and the world moves with it; a cowed goblin keeps running for
   three rounds of the party's steps; the arrived thug walks to the front room and a fight
   forms when it sees the party; during that fight a second thug closes one round at a time.
6. Merge inside-out.

## Done when

- A placement with no `on:` fights from the rulebook's default table; a faction's `on:`
  is inherited; a placement's key replaces the faction's key wholesale.
- The same table under `coward` and `aggressive` gives measurably different pick
  distributions in a unit test, and the beat shows the factor.
- A dealt mix produces a beat naming each goblin's temperament.
- Walking six cells advances the clock one round; four members walking together advance it
  once; a fight round advances it once for creatures outside the fight.
- A creature with `fled: {within: 3}` in its default table keeps running on the party's
  steps and stops on the fourth round.
- A creature with `enemy: remembered` walks toward where it last saw the party.
- A neutral creature with nothing opposed in sight holds.
- `Decider`, `Pump`, `fleeFrom`, the presets and the ladder have no references; ADR-0043
  says why.
- Every claim above is a unit test; the walk proves wiring.
