---
name: The mad king — castle use case (UC-5)
status: DRAFT, noodled in session 2026-10-13 (Kirk + cross-team); rulings in §6 made in that session; nothing scheduled, nothing built
purpose: the next living-world use case after the bandit camp, hostage population, weekend goal and tomb — one castle, many ways in, many endings, authored entirely in the studio by a UGC author
related: ../use-cases.md (UC-1..4) · ../brainstorm.md §13–14 · ../../shenanigans/README.md · ../../shenanigans/front-room-goblin.md · ../dialogue/social-offer-from-the-npc.md · ../disposition/both-ways.md · ../../creature-table/design.md · rpg-toolkit#1020 (stealth against perception)
---

# The mad king — castle use case

The bandit camp found the kernel: entities, edges, audience-scoped facts,
folds, predicates, verbs. The castle is the next size up, and it is written
the way UC-1 was: declared content as data, player actions as generic verbs,
every route asserted end to end. The standing acceptance criterion holds —
**zero castle-specific code**. If a route needs the word "troupe" or "king"
in Go, the primitive is wrong, and finding that is the point.

What has changed since the camp: the content is not a Go package, it is a
studio document. A streamer authors this castle, shares it, and another
table plays it. Every primitive below must therefore end as a field on a
form, not a line in a test. That is the boundary law from brainstorm §20
closing its own loop.

## 0. The scenario, in Kirk's words (2026-10-13)

> our use case right now is to kill the mad king. we will be in a castle
> encounter using our new studio. the idea is you can simply fight your way
> in, sneak in as a performer, or simply sneak and stealth in fighting only
> what notices you. on the table is possibly getting some guards to change
> sides on their king — they know he is really bad guy underneath kind of
> thing.

> the kitchen staff could be an in if we finished other encounters in a
> certain way.

> we had a blanket intimidate and persuasion but we plan on its home to be
> on npcs that are not hostile. i think the options will be authored for
> npcs. I am setting up the encounter, how can we get the guards to switch?
> interacting with them.

> I agree alarm and I also think a hearing channel for intel would also be
> impactful. … patrol was the thing I was thinking of adding to the tables.

> we can end the king encounter however we want. we are building use cases
> to show things working.

## 1. The castle, as structure

Three factions and a kitchen. The split is what makes every route
authorable with one `until` per pair, which is the law a pair turns at
most once by authoring (both-ways R1).

| faction | who | mind | stance toward the party |
|---|---|---|---|
| `garrison` | the gate captain, gate guards, hall guards | `captain` | neutral `until: { fact: party-recognized }` → hostile |
| `loyalists` | the king, his personal guard, the chamberlain | `king` | hostile, always (an attack from a turned garrison re-turns nothing; they were never neutral) |
| `kitchen` | the cook and two scullions | `cook` | neutral, no `until` |
| `garrison` ↔ `loyalists` | | | neutral `until: { fact: guards-turned }` → hostile |

Places: the gate, the yard, the great hall, the kitchen (its own door to
the yard), the chapel, the inner keep with patrolling loyalists, the throne
room. One dungeon, one run. The kitchen staff being "an in" means a fact
learned in the kitchen gates an option at the gate; that works inside one
run today and brings no journal.

**Why the disguise gets you in, not to the king.** The troupe check is on
the gate captain and it keeps the garrison neutral. It does not reach the
loyalists, because a fact lands on its witnesses and the loyalists' mind is
the king. A performer who wants the throne room still fights, sneaks, or
turns the garrison and lets it storm the keep. That is the audience law
doing exactly what it should, and it is the honest shape of a disguise that
fools the door and not the court. "Word spreads" stays shelved.

## 2. The file (studio document, v4 dialect, keys marked)

Keys marked `TODAY` compile on main. Keys marked `NEW` are this use case's
primitives (§4). Presentation keys omitted.

```yaml
version: 4
key: the-mad-king

factions:                                                     # TODAY
  - { id: garrison,  mind: captain, temper: { soldier: 2, coward: 1 } }
  - { id: loyalists, mind: king,    temper: aggressive }
  - { id: kitchen,   mind: cook }

dispositions:                                                 # TODAY
  - { between: [garrison, party],     stance: neutral, until: { fact: party-recognized } }
  - { between: [loyalists, party],    stance: hostile }
  - { between: [garrison, loyalists], stance: neutral, until: { fact: guards-turned } }
  - { between: [kitchen, party],      stance: neutral }

intel:                                                        # TODAY
  - { id: cooks-ledger, reveals: { fact: king-burned-village } }

tables:                                                       # TODAY (named root tables)
  gate-patrol:
    time:
      - { when: { enemy: none },                       toward: { path: [[2,3],[6,3],[6,8],[2,8]] } }   # NEW: path
      - { when: { heard: { kind: clang, within: 2 } }, toward: enemy }                                  # NEW: heard
      - { when: { enemy: seen }, say: "To arms! Intruders in the yard!", fact: alarm-raised }           # NEW: fact on time
      - { when: { enemy: reach },                      attack: enemy }
  keep-patrol:
    time:
      - { when: { enemy: none },                       toward: { path: [[12,4],[18,4],[18,10]] } }
      - { when: { heard: { kind: voice, within: 3 } }, toward: enemy }
      - { when: { heard: { kind: boom,  within: 3 } }, toward: { at: [20,12] } }      # fall back to the throne room
      - { when: { fact: alarm-raised },                toward: { at: [20,12] } }       # NEW: when fact
      - { when: { enemy: seen }, say: "Protect the king!", fact: alarm-raised }
      - { when: { enemy: reach },                      attack: enemy }
  storm-the-keep:
    time:
      - { when: { fact: guards-turned }, when_not_enemy_seen: true, toward: { at: [20,12] } }   # see §5, open
      - { when: { enemy: reach }, attack: enemy }
      - { when: { enemy: seen },  toward: enemy }

monsters:                                                     # TODAY
  - { id: captain,    ref: "dnd5e:monsters:guard-captain", at: [4,2],   faction: garrison }
  - { id: gate-1,     ref: "dnd5e:monsters:guard",         at: [2,3],   faction: garrison }
  - { id: gate-2,     ref: "dnd5e:monsters:guard",         at: [6,3],   faction: garrison }
  - { id: hall-1,     ref: "dnd5e:monsters:guard",         at: [10,6],  faction: garrison }
  - { id: cook,       ref: "dnd5e:monsters:commoner",      at: [3,10],  faction: kitchen }
  - { id: keep-1,     ref: "dnd5e:monsters:knight",        at: [12,4],  faction: loyalists }
  - { id: keep-2,     ref: "dnd5e:monsters:knight",        at: [18,10], faction: loyalists }
  - { id: king,       ref: "dnd5e:monsters:mad-king",      at: [21,12], faction: loyalists }
  - { id: reserve-1,  ref: "dnd5e:monsters:knight",        at: [16,2],  faction: loyalists, arrives: { fact: alarm-raised } }
  - { id: reserve-2,  ref: "dnd5e:monsters:knight",        at: [16,2],  faction: loyalists, arrives: { fact: alarm-raised } }

monsterBindings:
  captain:
    table: storm-the-keep                                     # TODAY
    options:                                                  # NEW
      - id: troupe
        say: "We're the troupe for tonight's feast."
        requires: { fact: feast-tonight }
        check: [{ ability: performance, dc: 13 }, { ability: deception, dc: 15 }]
        success: [{ say: "Through the side gate, then. Don't dawdle." }]
        failure: [{ say: "Troupe? There's no— GUARDS!", fact: party-recognized }]
      - id: stand-aside
        say: "Stand aside or bleed."
        check: [{ ability: intimidation, dc: 14 }]
        success:
          - { weight: 3, say: "…go on, then. I saw nothing.", fact: gate-cowed }
          - { weight: 1, say: "Big words. GUARDS!", fact: party-recognized }
        failure: [{ say: "You'll bleed first.", fact: party-recognized }]
      - id: turn
        say: "He burned Ashford. Your own village. You know what he is."
        requires: { fact: king-burned-village }
        check: [{ ability: persuasion, dc: 15 }]
        success: [{ say: "…Gods forgive me. The garrison is yours. We take the keep.", fact: guards-turned }]
        failure: [{ say: "Lies. Get out before I change my mind." }]
      - id: hold-the-castle
        say: "Hold the gate. Let no one in or out until it's done."
        requires: { fact: guards-turned }
        check: [{ ability: persuasion, dc: 10 }]
        success: [{ say: "Done. Go finish it.", fact: throne-taken }]
        failure: [{ say: "We'll hold what we can." }]
  gate-1: { table: gate-patrol }
  gate-2: { table: gate-patrol }
  keep-1: { table: keep-patrol }
  keep-2: { table: keep-patrol }
  cook:
    options:                                                  # NEW
      - id: gossip
        say: "Busy night?"
        check: [{ ability: persuasion, dc: 10 }]
        success: [{ say: "Feast for the whole court, and him sitting there grinning after what he did to Ashford.", fact: feast-tonight }]
        failure: [{ say: "Out of my kitchen." }]
      - id: ashford
        say: "What did he do to Ashford?"
        requires: { fact: feast-tonight }
        check: [{ ability: persuasion, dc: 12 }]
        success: [{ say: "Burned it. Every house. The captain's mother was in one.", fact: king-burned-village }]
        failure: [{ say: "I've said too much already." }]
      - id: bribe
        say: "Ten gold says you never saw us."
        requires: { holds: purse }
        check: [{ ability: persuasion, dc: 8 }]
        success: [{ say: "Saw who?", fact: feast-tonight }]
        failure: [{ say: "Keep your coin." }]

propBindings:
  altar:                                                      # options on a prop: NEW
    options:
      - id: examine
        say: "Examine the altar."
        check: [{ ability: religion, dc: 12 }]
        success: [{ say: "The sigils are the Pale One's. The king keeps a shrine to it.", fact: king-is-cultist }]
        failure: [{ say: "Old stone, old candles." }]
  ledger: { holdable: true, holds: [cooks-ledger] }           # TODAY

endings:                                                      # TODAY
  - { id: king-slain,   when: { down: king } }
  - { id: king-deposed, when: { fact: throne-taken } }
```

Two reads of the file worth stating. The `ashford` option and the ledger
are two writers of one fact, exactly as UC-4's fight and search were:
persuade the cook, or find and hold the ledger. And `king-is-cultist` is
written by nothing but the altar and read by nothing yet; it is the hook a
second author reaches for when they add a `requires` to the captain, and it
shows a knowledge check landing with the checker as audience.

## 3. Routes and assertions

Every route is universally attemptable. The residue of a route is a fact in
somebody's head, or the absence of one; nothing records "how you got in".

| route | verbs composed | facts written (audience) | asserted outcome |
|---|---|---|---|
| **Front door** | attack a gate guard | none | aggression law turns garrison ↔ party hostile; fight forms at the gate; `alarm-raised` from the first guard with a sighting; reserves arrive; loyalists in the keep walk to the throne room on the fact |
| **Troupe** | gossip with the cook, speak `troupe` at the gate | `feast-tonight` (party), nothing on success | garrison stays neutral; the party walks to the hall; journal shows no attack facts before the keep |
| **Troupe, blown** | same, failing the check | `party-recognized` (gate witnesses) | garrison turns hostile from a fact its mind witnessed; the fight forms by the stance site, not by sight refresh |
| **Kitchen door** | enter by the yard's kitchen door, sneak past the keep patrol | none | passive Stealth beats the knight's passive Perception on first contact: no sighting lands; the party reaches the throne room with one fight |
| **Kitchen door, heard** | same, but the fighter forces the inner door | a `crash` report (hearers within 2 hops) | the knight's `heard` row walks it to the door; its `enemy: seen` row shouts; `alarm-raised` lands on hearers and reserves arrive |
| **Turn the garrison** | cook `gossip` → `ashford`, captain `turn` | `feast-tonight`, `king-burned-village`, `guards-turned` (witnesses incl. the captain, who is the mind) | garrison ↔ loyalists turns hostile; the captain's `storm-the-keep` row walks the garrison to the throne room; a three-sided fight forms in which the party and the garrison both target loyalists and neither targets the other |
| **Deposed** | turn the garrison, then `hold-the-castle` | `throne-taken` | ending `king-deposed` fires with the king standing; method-indifference at castle scale |
| **Slain** | any route, king to 0 hp | none | ending `king-slain` |
| **Scout** | talk to a gate guard instead of the captain | `guards-turned` never written: the guard carries no `turn` option, and even a fact landed on him is not on the mind | nothing turns; correct, and the studio says why |

Every route: facts are attributed and audience-scoped in the run's journal;
dispositions are folds, nothing stored; every pick and every check is a seen
roll with the creature or character as the die's entity.

## 4. Primitives exercised, with state

| primitive | state | notes |
|---|---|---|
| Factions, dispositions, `until` both ways, the aggression law | SHIPPED | both-ways R1–R5 |
| Faction mind as the grain for `fact` untils | SHIPPED | hold-out |
| `arrives: { fact }`, `endings` on the predicate grammar | SHIPPED | v4 gameplay shape |
| Creature tables: layers, `time`, enemy bands, deed windows, temper, `toward: { at }` | SHIPPED | creature-table design |
| NPC stat blocks from a base monster (`guard`, `commoner`, `knight` as human-based blocks) | SHIPPED | stat-block initiative |
| Localized fights: only the engaged pair enters | SHIPPED | `trigger.go` |
| **Options on a binding** — player line, approach list, `requires`, success and failure answer tables | **NEW, this use case** | §6 R1–R3 |
| **`requires` as the predicate grammar's next consumer**, with a `holds` form | NEW | the fifth consumer after `until`, `arrives`, `endings`, and the hold-out's `round` |
| **Hearing**: a report of `{ kind, position, when }`; loudness per deed kind; DC rises per door hop; passive Perception answers | NEW | §6 R4–R5 |
| **`when: { heard: { kind, within } }`** on a table row | NEW | the deed condition with a kind |
| **`fact` legal on a `time` row** (audience: hearers) | NEW | this is the alarm; no `alarm` word |
| **`when: { fact }`** on a table row | NEW | the predicate grammar entering the table |
| **`toward: { path: [...] }`** with the next waypoint derived from position | NEW | patrol holds no state |
| Passive Stealth at first contact | OPEN rpg-toolkit#1020 | the "player gets the drop" case that is classified and does nothing |
| Players hear too: a per-viewer beat when passive beats the DC | NEW, small | perception law on a second sense |

Not brought, by name: `allied` (a three-sided fight where the party and the
garrison both target loyalists is enough; allied waits for a use case where
"fights for you" differs from "fights the same enemy"); per-character stance;
any monster-initiated check (hearing and noticing are passive on the monster
side, on purpose); the campaign journal (the castle is one run); an authored
conversation tree; word spreads; the challenge (§5, on the list).

## 5. What this surfaces (findings to file, not to fix here)

- **`requires` is one predicate.** No `and`. The castle lives within it by
  chaining facts (the `ashford` line requires `feast-tonight`), which is the
  same move the journal ruling made for quest trees. Record; do not widen.
- **The challenge: a neutral guard approaches for questioning (on the
  list, Kirk 2026-10-13).** A garrison guard who watches the party climb
  the kitchen wall at midnight does nothing today, because the enemy bands
  are opposed-only and a sighting is not a fact. The shape it wants, each
  piece one word: a `stranger: seen` band (a non-opposed party member in
  sight), an `approach` word that walks to them, and the exchange opening
  from the guard's side — his `say` poses the question and the party's
  options are the answers, with failure writing `party-recognized`. "Some
  guards do nothing and others follow up" is the table and the temper doing
  their job: `stranger: seen` rolls two `hold` against one `approach`, and a
  lazy temper tilts it. Not brought here; the castle is authored so it does
  not need it. It is the first use case for a creature opening an exchange.
- **The faction mind can be more than one knower (Kirk 2026-10-13).** Not
  any member — the audience grain exists so one lieutenant can see through
  a disguise while the camp stays fooled — but `mind: [captain, lieutenant]`,
  any of whom knowing is enough. A list on the existing field, the same
  fold. A quorum or a declared derivation beyond that waits for its use
  case; nobody reaches for a `quorum` field.
- **Word spreads is creatures talking, now that hearing exists.** The
  shelved version was a scheduled fact reaching the mind N rounds later.
  With hearing, a guard who holds a fact has a `time` row that says it
  aloud and writes the same fact to hearers; the captain learns it when
  that guard walks within earshot. The author decides who gossips and a
  thick door stops it. The one thing to design before using it is the
  loop: a creature that knows the fact would repeat it every round. A
  `within` window on the row or a once-per-knower rule on `fact` are the
  two cheap answers.
- **Rows keep wanting a second condition.** Three instances now: storm the
  keep yielding to `enemy: seen`; the challenge not re-challenging a troupe
  it already admitted (`stranger: seen` and not `fact: troupe-admitted`);
  the talker not repeating itself. By the second-instance law the grammar
  earns the answer, and it should be one answer for all three.
- **"Storm the keep" wants two conditions.** The row `when: { fact:
  guards-turned }` should yield to `enemy: seen`; today a `when` is one
  condition and rows are rolled, not ordered. The sketch above shows the
  wart (`when_not_enemy_seen`) rather than hiding it. Two honest answers:
  row precedence by order, or `when` as a small list. See the last finding.
- **The studio dropped the check routes.** `intimidate:` and `persuade:`
  bindings are read from files but not editable, and exits, endings and
  concealments live only in the older editors. Options are where that form
  comes back, and it is where a UGC author spends most of their time.
- **Loudness vocabulary needs a home.** `clang`, `boom`, `voice`, `crash`,
  `growl` are rulebook content stamped on deeds. The castle uses four.

## 6. Rulings made in session (2026-10-13, Kirk)

- **R1 — options are flat, facts gate what shows up.** An author tree is
  something to work toward, not the start.
- **R2 — the answer table lives on the option.** Intimidate and Persuade
  become options whose check lists Intimidation or Persuasion; the
  verb-keyed table triggers (`intimidated`, `persuade_failed`, …) retire;
  one `Speak` verb with an option id replaces the two RPCs. A creature with
  no options cannot be talked to (social-offer R1 stands).
- **R3 — the deed is the skill.** A successful Intimidation check lands
  `intimidated` on witnesses whatever the line said; Religion and Insight
  land no deed. The untrained disadvantage rule stays attached to the skill.
- **R4 — a heard report carries position and kind, never identity.** The
  kind is a sealed word (`clang`, `boom`, `voice`, `crash`, `growl`, one per
  use case).
- **R5 — hearing is passive.** The sound's DC rises per door hop; the
  listener's passive Perception answers. No monster rolls.
- **R6 — `fact` becomes legal on a `time` row and `when: { fact }` joins the
  table conditions.** Alarm is a loud fact; no `alarm` word.
- **R7 — patrol is `toward: { path }`** with the next waypoint derived, so
  the table keeps holding no state.
- **Direction, not ruled:** social options' home is non-hostile creatures.
  Whether a hostile creature in a fight still offers an Intimidation option
  (front-room R4 left it) is a question for the walk, not this doc.

## 7. Done when

- The file above compiles in both dialects with `New()` refusing each
  missing field in form-filler words (a `requires` naming a fact nothing
  writes is refused by name, like a disposition naming a missing faction).
- Every row of §3 is an encounter-level scenario test; the toolkit's
  `examples/` or the reference content carries the castle as the sixth
  reference dungeon.
- Kirk authors it in the studio, shares the document, and walks each route
  on the local stack: the troupe in, the troupe blown, the kitchen wall, a
  forced door bringing a knight, the captain turning and the garrison
  marching, the king deposed with his heart still beating.

— cross-team agent, on behalf of KirkDiggler
