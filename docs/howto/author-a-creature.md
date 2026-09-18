# Author a creature

A creature decides what to do by rolling on a table you write. This page is the
whole grammar for that table: where it goes, what keys it takes, what an entry
may say, how temperament loads the die, and when a creature gets to act.

Everything here is YAML in a dungeon file, and everything a creature does comes
back in the run log with its arithmetic showing. If you write something the
grammar does not have, the file is refused when it loads, by name, with the line
number. You find out on the form rather than at the table.

## What a creature's table is

A creature rolls on a weighted table, and four things load that die:

1. **the rulebook's default table for its kind** — why a creature you say
   nothing about still fights;
2. **your orders** — the `on:` block below;
3. **its temperament** — `temper:`, a set of multipliers and nothing else;
4. **what it has seen and suffered** — who hit it, who frightened it, who it can
   see right now. You never write this. Your entries read it through `when:`.

One entry fires per roll. The creature's line and its one action come from that
entry, and the roll is on the log with every candidate the creature could have
picked instead.

## Where you write it

`on:` goes on a placement, or on a faction so every member of it inherits the
same orders.

```yaml
factions:
  - id: goblins
    on:
      intimidated:
        - { weight: 70, say: "Fine! FINE. The cellar door is behind the barrels.", fact: goblin-cowed }
        - { weight: 30, say: "Boss! BOSS!", flee: {} }
```

Three layers supply tables — the rulebook's default, then the faction's, then
the placement's — and for each key **the nearest layer that names it wins
wholesale**. A placement that writes `time:` replaces the faction's `time:`
entirely; a faction that writes `time:` replaces the rulebook's. Entry lists are
never merged, so you never have to work out what got added to what.

That is a real cost and it is worth seeing once. The bandits in the front room
need one standing order — walk to the front room when there is nobody to fight:

```yaml
on:
  time:
    - { when: { enemy: none }, toward: { at: [3, 3] } }
```

Written like that, those bandits walk west beautifully, form a fight the moment
one of them sees a player, and then stand in it. Their `time` key replaced the
rulebook's, and the rulebook's is where "swing at what is in reach" lived. An
author who wants both writes both. The finished front room below is what both
looks like.

## The keys

Five triggers, and the difference between them is when the roll happens.

| key | fires when |
| --- | --- |
| `intimidated` | a threat against this creature lands |
| `intimidate_failed` | a threat against it misses |
| `persuaded` | an appeal to it lands |
| `persuade_failed` | an appeal to it misses |
| `time` | the creature has time: its turn in a fight, or a round of the world |

The four social keys fire inside the verb, against this creature, the moment the
check is settled. `time` is the one that fires without anybody talking to it.

Failure has a table of its own on purpose. A failed appeal that teaches the
party a lie is what makes attempting worse than not attempting.

A key you do not write falls through to the next layer out. A key nobody writes
at any layer means the creature does nothing about that outcome and no beat is
written at all — absent means absent, not "the creature shrugged".

Anything else under `on:` is refused:

```
"attacked" is not a trigger this build rolls: they are intimidated,
intimidate_failed, persuaded, persuade_failed, time
```

## An entry

An entry is `{ weight, say, when, <one word> }`.

**`weight`** is a relative share. The engine sums the eligible weights and rolls
one die that size, so `3` and `1` mean what `75` and `25` mean. Omitted is 1, so
a table whose entries all omit it is an even split and a single entry is the
certainty it looks like. A weight below 1 is refused — it is a row that can
never fire.

**`say`** is the creature's line, carried word for word onto the beat. Nothing
composes it, edits it, or adds to it.

**One word, or none.** Two words in one entry is refused so ordering never has
to be guessed. An entry with no word is legal only if it has a line to say; one
that neither speaks nor acts is a row written for no reason.

| word | what happens | legal on |
| --- | --- | --- |
| `fact: <id>` | everyone who was there learns the fact | the four social keys |
| `flee: {}` | the creature holds `fled` against whoever spoke to it, at now. It does **not** step here | the four social keys |
| `hold: {}` | the creature does nothing with its time | `time` |
| `attack: <selector>` | strike the selected creature | `time` |
| `toward: <selector>` | walk toward it, or onto an authored cell | `time` |
| `away: <selector>` | walk away from it | `time` |

`flee` not stepping is deliberate and it is what makes a frightened creature
readable. It lands the memory of having been made to run; the verb pays its
round, and the creature's own `time` table does the running on the party's
steps, which is why you watch it go instead of finding it gone.

**Selectors** name what a word acts on.

| selector | who |
| --- | --- |
| `enemy` | the nearest creature it is opposed to and can see, else the nearest one it remembers |
| `attacker` | whoever last landed an attack on it |
| `actor` | whoever did the deed this entry's `when` names |
| `{ at: [col, row] }` | an authored cell, on `toward` only |

A creature is never handed a target it is not opposed to. That is what keeps a
neutral goblin standing in its doorway rather than advancing on the party the
first time it has time.

An authored cell is walked **onto**, not up to, so pick a cell nothing stands
on. A creature anchor is stood beside.

**`when:`** is a condition on what this creature holds, and an entry whose
condition does not hold is not on the table for that roll — absent, not weighted
zero, so the log's candidate list is the honest account of what it could have
done. `when:` is a `time` word: a social key is already the condition.

| `when` | holds when |
| --- | --- |
| `{ enemy: reach }` | a creature it is opposed to is within its reach |
| `{ enemy: seen }` | one is in sight and **none** is in reach |
| `{ enemy: remembered }` | none is in sight, but it remembers where one was |
| `{ enemy: none }` | none of the above |
| `{ attacked: { within: N } }` | it was attacked at most N rounds ago |
| `{ intimidated: { within: N } }` | it was threatened successfully at most N rounds ago |
| `{ persuaded: { within: N } }` | it was talked round at most N rounds ago |
| `{ fled: { within: N } }` | it was made to run at most N rounds ago |

The four `enemy:` bands are exclusive: exactly one of them holds at any moment,
so you write one entry per band and know which fires. `seen` is the gap `toward`
is for — a swing at somebody out of reach is a wasted turn, so the swing belongs
under `reach`.

A deed span is counted from 1 and names one condition. Two conditions in one
`when` is refused rather than read as "and": you meant something, and guessing
which of two readings you meant is what a small vocabulary exists to avoid.

### What gets refused, in the words you will see

Every one of these is what the loader prints. They name the file line.

A word under a key it is not legal on:

```
place[1].on.intimidated[0].attack: `attack` is what a creature does with time,
and `intimidated` is an outcome (line 12)
```

Two words in one entry:

```
place[1].on.intimidated[0]: an entry does one thing: `fact` and `flee` in the
same entry is 2 (line 9)
```

Others you will meet, each naming the line you wrote it on:

```
`fact` answers a social verdict, and `time` is not one
`intimidated` is already the condition — a `when` under it asks when a thing that just happened happened
`actor` is the actor of the deed this entry's `when` names, and this entry names no deed
a cell is somewhere to walk toward, and `away` acts on a creature
a weight of 0 can never be rolled: omit it for 1, or give it a share
this walks to [40,40], which is not floor
`enemy: nearby` is not a condition this build reads: they are reach, seen, remembered, none
a `when` is one condition, and this names 2
a span of 0 rounds is counted from 1
this entry does nothing and says nothing
this names a trigger and lists nothing that happens on it
```

## The rulebook's default

This is the table every creature answers with when you have written nothing.
Knowing it is most of knowing what your own orders will cost you.

```yaml
time:
  - { when: { fled: { within: 3 } },     away: actor,      weight: 3 }
  - { when: { attacked: { within: 3 } }, attack: attacker, weight: 3 }
  - { when: { enemy: reach },            attack: enemy }
  - { when: { enemy: seen },             toward: enemy }
  - { when: { enemy: remembered },       toward: enemy }
  - { when: { enemy: none },             hold: {} }
```

Line by line:

- **`fled`, three rounds, weight 3.** A creature that was frightened runs on its
  own time for three rounds. This is why a goblin you cow needs no table of its
  own to bolt: the social entry lands the memory and this line does the running.
- **`attacked`, three rounds, weight 3.** It comes for whoever hit it, and keeps
  coming for three rounds.
- Those two sit **above** the bands, and heavy, because they are memories rather
  than situations. A creature that was just struck should mostly answer that,
  even with somebody else closer.
- **`reach` → attack.** The band where a swing can land.
- **`seen` → toward.** In sight, out of reach: it has to cross the room first.
  A table whose only sighted answer was `attack` would leave a creature standing
  where it spotted you, swinging at nothing.
- **`remembered` → toward.** It walks to where it last saw you.
- **`none` → hold.** Every entry is conditional, `hold` included. An
  unconditional `hold` is eligible on every roll, so it competes with the attack
  and the walk and a creature in reach stands there half its turns. A table with
  no eligible entry is already a hold and says so in the log, so the condition
  costs nothing and stops `hold` costing the creature its turn.

## Temperament

Four goblins, one table, four behaviours. `temper:` is a set of multipliers on
the table's words and **nothing else**. It adds no entries where a table is
silent, it has no triggers, and it remembers nothing.

```yaml
place:
  - id: front-goblin
    ref: "dnd5e:monsters:goblin"
    faction: goblins
    at: [4, 3]
    temper: coward
```

A word on a placement; a **mix** on a faction, dealt once per member when it
comes into the world:

```yaml
factions:
  - id: goblins
    temper: { coward: 2, soldier: 1, aggressive: 1 }
```

Each deal is its own roll with the faction as the die's owner, and each is a
beat, so the table watches which goblin came out the coward before anybody has
said a word to it. A placement that names its own word wins and the mix is not
dealt for it — you already answered the question the mix exists to ask.

The three words, as percentages on each table word. 100 leaves an authored
weight as written.

| `temper` | attack | toward | away | flee | hold |
| --- | --- | --- | --- | --- | --- |
| `soldier` (and absent) | 100 | 100 | 100 | 100 | 100 |
| `coward` | 50 | 50 | 300 | 300 | 100 |
| `aggressive` | 300 | 300 | 25 | 25 | 50 |

A coward still holds as readily as anyone: cowardice is about direction, not
idleness. `fact` is never multiplied — learning something is not a thing a
creature chooses.

Here is what that does to the goblins' authored 70 cower / 30 flee, and it is
the whole argument for temperament. For a **soldier**, 70 against 30: it runs
three times in ten. For a **coward**, `flee` is trebled, so 7000 against 9000
out of 16000: it runs 56% of the time. For the **aggressive** one, `flee` is
quartered, so 7000 against 750 out of 7750: it runs 10% of the time.

One threat, one line, one table, and one goblin bolts while the next stands
there saying the same words.

Only those three words exist. A fourth is a design decision, not a line somebody
adds:

```
place[1].temper: "brave" is not a temperament this build ships: they are
soldier, coward, aggressive
```

A share of 0 in a mix is refused too — `a share of 0 can never be dealt — give
"coward" a share of at least 1`.

## Time

The world's clock moves because the party acts. Nothing runs on a timer.

| what a player does | what it costs the world |
| --- | --- |
| walking | one round per **pace** — every six cells for a 30-foot mover |
| any action: talking, searching, unlocking, looting, casting | one round |
| standing still | nothing |
| a round of a fight wrapping | one round, for every creature in that fight |

The clock takes the **highest** of these, not the sum, so four players walking
six cells together is one round rather than four.

When a round passes, the world thinks: every creature standing outside a fight
gets one turn's worth of doing, rolls its `time` table, and spends it. Then
sight is refreshed, and a creature that walked into somebody's view joins a
fight the ordinary way. A fight round counts as a round passing, so creatures
outside a fight close one round at a time while it runs.

What this means for your standing orders: a creature that arrives two rooms away
with `{ when: { enemy: none }, toward: { at: [3, 3] } }` walks while the party
talks, walks while the party walks, walks while somebody else fights, and stops
walking the moment it has an enemy to answer instead. A party that sits still
sees nothing move.

## Reading the log

Every one of these is a real line from a walk of the front room.

A temperament dealt at the door — the mix, the roll, the word:

```
{"beat": "tempered", "faction": "goblins", "member": "front-goblin-2", "of": 4, "roll": 2, "temper": "coward"}
```

A roll on a table, with its whole arithmetic:

```
{"beat": "answered", "key": "intimidated", "creature": "front-goblin-2", "temper": "coward",
 "candidates": [{"entry": 0, "weight": 70, "percent": 100, "loaded": 7000},
                {"entry": 1, "weight": 30, "percent": 300, "loaded": 9000}],
 "of": 16000, "roll": 11988, "entry": 1, "word": "flee", "say": "Boss! BOSS!"}
```

Read it as: `weight × percent = loaded`. `of` is the sum of the loaded weights
of every **eligible** entry — entries whose `when` did not hold are not in the
list at all. `roll` is the face. `entry` is the row of your table that fired,
counting from 0. Here the coward's trebled `flee` beat the cower it was written
three times less likely than.

The same line for the aggressive goblin, same file, same table, reads
`{"entry": 1, "weight": 30, "percent": 25, "loaded": 750}` out of `7750`.

A round passing, and a creature acting on it:

```
{"beat": "tick", "tick": 9}
{"beat": "answered", "key": "time", "creature": "bandit-1", "temper": "aggressive",
 "candidates": [{"entry": 5, "weight": 1, "percent": 300, "loaded": 300}],
 "of": 300, "roll": 180, "entry": 5, "word": "toward", "selector": "at"}
{"beat": "moved", "cause": "encounter:table:toward", "member": "bandit-1", "position": {"x": 14, "y": 3}}
{"beat": "moved", "cause": "encounter:table:toward", "member": "bandit-1", "position": {"x": 13, "y": 3}}
```

One candidate, because the bandit could see nobody it was opposed to and only
the standing order's band held. The `cause` on every step names the word that
caused it.

**If a creature does nothing, the log says why.**

```
{"beat": "stayed", "cause": "encounter:table:away", "member": "front-goblin", "why": "nowhere farther from (4, 2) within 6 cells"}
```

That goblin picked `away: actor` and had run out of room. It is not a creature
ignoring your orders; it is a creature that had nowhere left to go.

## The front room, complete

This is the shipped reference file, with the floor, walls and doors left out.
Everything below is the creature authoring.

```yaml
factions:
  # FOUR GOBLINS, ONE TABLE, FOUR BEHAVIOURS. The mix is a die thrown once per
  # goblin as it enters the world. Weighted toward coward on purpose, because
  # the difference between a coward and the aggressive one off this same table
  # is the thing worth watching.
  - id: goblins
    temper: { coward: 2, soldier: 1, aggressive: 1 }
    # The orders live on the faction rather than on one placement precisely so
    # all four share them. A table on one placement cannot be shared.
    #
    # NO `time` KEY HERE, DELIBERATELY. A goblin given time falls through to
    # the rulebook's default, which is what makes a neutral one stand still
    # (nothing it is opposed to is in sight, so no entry holds) and what makes
    # a cowed one keep running (`fled: { within: 3 }` walks it away for three
    # rounds of the party's steps). Writing `time:` here would cost both.
    on:
      # A LANDED THREAT, 70/30. Seven times in ten it folds and teaches the
      # party the fact; three times in ten it bolts — and it is 30 only for a
      # soldier. The 30 is what makes the threat a gamble rather than a button:
      # the goblin you frightened away is a goblin you can no longer talk to.
      intimidated:
        - { weight: 70, say: "Fine! FINE. The cellar door is behind the barrels. Just don't.", fact: goblin-cowed }
        - { weight: 30, say: "Boss! BOSS!", flee: {} }

      # A MISSED THREAT. One entry, so it always fires: the goblin is not
      # impressed and says so, and nothing else happens.
      intimidate_failed:
        - { say: "Big talk, for someone standing in my doorway." }

      # A LANDED APPEAL. It tells the truth and teaches nothing. Talking
      # somebody round is not frightening them, so this is not the cowed fact.
      persuaded:
        - { say: "Bandits took the cellar. Go left at the rope, and mind the third step." }

      # A FAILED APPEAL, AND THE TRAP. The goblin lies, and the lie is a fact
      # like any other — an id and nothing else. The bandits' `arrives` below
      # reads it. Nothing anywhere marks a fact as false, so a lie looks exactly
      # like the truth to whoever was told it.
      persuade_failed:
        - { weight: 100, say: "Cellar's empty, friend. Nothing down there but rats. Straight on through.", fact: cellar-is-clear }

  # The bandits are dealt from a mix weighted the other way. The one coward in
  # four is the point: the same orders produce a bandit that holds back, and
  # nobody authored it.
  - id: bandits
    temper: { coward: 1, soldier: 2, aggressive: 1 }

# The goblins are NEUTRAL, and that line is load-bearing. Without it they are
# hostile, a fight forms on frame one, and there is no front room to talk in.
dispositions:
  - { between: [goblins, party], stance: neutral }
  - { between: [bandits, party], stance: hostile }

place:
  # The one the party meets. The priced checks say this goblin is harder to
  # lean on than its stat block suggests and easier to talk to — a thing you
  # can now say, and the engine hears.
  - id: front-goblin
    ref: "dnd5e:monsters:goblin"
    faction: goblins
    at: [4, 3]
    targeting: closest
    actions: ["dnd5e:weapons:scimitar"]
    intimidate: [{ ability: intimidation, dc: 12 }]
    persuade: [{ ability: persuasion, dc: 10 }]

  # THE OTHER THREE, DELIBERATELY PLAIN. No `on:`, no `temper:`, no priced
  # check. They are the control: every difference the table shows between these
  # four is the die. A `temper:` written on any of them would be the author
  # answering the question the faction's mix exists to ask.
  - id: front-goblin-2
    ref: "dnd5e:monsters:goblin"
    faction: goblins
    at: [4, 1]
  - id: front-goblin-3
    ref: "dnd5e:monsters:goblin"
    faction: goblins
    at: [4, 5]
  - id: front-goblin-4
    ref: "dnd5e:monsters:goblin"
    faction: goblins
    at: [5, 6]

  # THE BANDITS, TWO ROOMS AWAY AND NOT YET ANYWHERE. Until the lie is known by
  # somebody, these are on no map and in no roster. A failed appeal mints the
  # fact and they walk in.
  #
  # THE FIRST FIVE LINES ARE THE RULEBOOK'S DEFAULT, COPIED, AND THE COPY IS
  # THE POINT. Writing `time:` replaces the default's `time:` wholesale. Without
  # the copy these two would walk west beautifully and then stand in the fight
  # they formed, because nothing on their table would have anything to say about
  # an enemy they could see. An author who wants both writes both.
  #
  # NO `hold` LINE, and that is not an omission: the standing order takes the
  # `enemy: none` band the default's `hold` had. A bandit with nobody in sight
  # walks rather than waits, which is the only thing these two do differently
  # from every other creature in the game.
  #
  # AN AUTHORED CELL IS WALKED ONTO, so [3, 3] is chosen clear: [4, 3] is the
  # goblin's and [1, 3] is where the party comes in.
  - id: bandit-1
    ref: "dnd5e:monsters:bandit"
    faction: bandits
    at: [16, 3]
    targeting: closest
    actions: ["dnd5e:weapons:scimitar", "dnd5e:weapons:light-crossbow"]
    arrives: { fact: cellar-is-clear }
    on:
      time:
        - { when: { fled: { within: 3 } },     away: actor,      weight: 3 }
        - { when: { attacked: { within: 3 } }, attack: attacker, weight: 3 }
        - { when: { enemy: reach },            attack: enemy }
        - { when: { enemy: seen },             toward: enemy }
        - { when: { enemy: remembered },       toward: enemy }
        - { when: { enemy: none },             toward: { at: [3, 3] } }
  - id: bandit-2
    ref: "dnd5e:monsters:bandit"
    faction: bandits
    at: [16, 5]
    targeting: closest
    actions: ["dnd5e:weapons:light-crossbow"]
    arrives: { fact: cellar-is-clear }
    on:
      time:
        - { when: { fled: { within: 3 } },     away: actor,      weight: 3 }
        - { when: { attacked: { within: 3 } }, attack: attacker, weight: 3 }
        - { when: { enemy: reach },            attack: enemy }
        - { when: { enemy: seen },             toward: enemy }
        - { when: { enemy: remembered },       toward: enemy }
        - { when: { enemy: none },             toward: { at: [3, 3] } }
```

Copying the default's five lines to keep a standing order is the honest cost of
the layering, and it is recorded rather than solved: the day the rulebook tunes
its default, a copy does not move with it. That is a question to settle after
more of these have been written, not a reason to leave the bandits unable to
fight.

## Words that are not here yet

These are named, designed, and refused by name so you do not sit waiting for
something that is not there. Each refusal tells you where the word went.

- **`alarm`** — a creature that raises the camp. Refused: `` `alarm` is designed
  as the Alarm slice and not built yet``.
- **`lure`** — a creature that draws you somewhere. Refused as designed and not
  built. The front room's trap does not need it: a fact carrying bad directions
  does the same work with words that exist.
- **`pretend`** — a creature that lies about what it is, for Insight to catch.
  Refused as designed and not built.
- **`patrol`** — a route walked rather than a destination. Refused as designed
  and not built. A standing `toward: { at: … }` is the one-destination version.
- **Walking through a door to look.** A creature whose `toward: enemy` reaches
  the cell it remembers and finds nobody there holds. It does not push on to
  look. That is a word, and it does not exist yet.
- **Two words in one entry.** Refused, and two entries do not express it either.
  A list under one entry is the shape it would arrive in when something needs
  it.
- **A fact that turns two factions hostile.** Facts end a hostility today; none
  starts one, and no fact makes a neutral pair friendlier. The next slice is
  where that changes.
