# The front room goblin — an authored reaction table

**Status:** RULED 2026-09-17 (rpg-project#457). Rulings R1–R4 closed by Kirk on the PR; tracking issue follows.

## The scenario, in Kirk's words

> "Goblin in the front room is not hostile to the party. Could be intimidated or persuaded. Intimidation can be configured to either flee or help. Thugs and bandits are in the next rooms and are hostile. The author can say what would happen, and what we want to work towards is making the outcome based on a roll. Intimidation succeeds and the goblin has a 30% chance or so to run. The setup for the author is what I am most interested in. A successful intimidation's bad outcome could be running for the alarm. A failed persuasion? Maybe they give them bad information or lead them into a trap. The authoring ability is our focus."

> "Intimidation is not really a combat skill. In combat the goblin might be afraid and run anyway."

> "Thinking of D&D rules, everyone can intimidate, but they should be rolling with disadvantage without the skill. We want proper rules in. This is new stuff, so we do not need to rush off making things work. We should be deliberate about what we are adding, to make the choices a player takes impactful."

> "This is also a place where the dungeon author can put text the goblin would say for each outcome."

> "The graphs and disposition should allow a non-hostile. It does expose that we want to share that info with the players. Maybe they get a colored circle at the feet. That color could tell the disposition of the NPC. A perceptive person would see a monster pretending to be intimidated; the accurate disposition color shows to them."

## What this slice adds (the tool, not the feature)

1. **A reaction table the author writes on a placement.** Per verb, per outcome, a weighted list of what the creature does and what it says. The world rolls which entry fires.
2. **The untrained rule.** Every character may attempt a skill verb; a character without the skill rolls at disadvantage. That is the choice-at-creation becoming a choice-at-the-table.
3. **Persuade**, Intimidate's twin on the same machine.
4. **A believed stance on the sighting.** Each player sees a ring under a creature colored by what *that player* believes its stance to be. The truth stays with the world.

Everything else the scenario wants (alarm, lure, a creature acting outside a fight) is named below with its state, so the table's vocabulary grows a word per slice instead of being invented whole.

## What already exists (verified 2026-09-17 against origin)

| Need | State | Where |
|---|---|---|
| A goblin that is not hostile | **SHIPPED.** Author `factions:` + `dispositions: [{between: [goblins, party], stance: neutral}]` and place with `faction: goblins`. No fight forms; the goblin stands. Stance is derived from the graph per pair, never stored. | `encounter/field.go` Stance, `encounter/disposition.go`, `dungeonspec/factions.go`, lobby forwards `Faction` at Spawn |
| Thugs two rooms away, hostile | **SHIPPED.** One field, every placement spawned at launch; a fight is a bubble that forms on sight + hostility. `arrives:` holds a reserve. | `encounter/trigger.go` classify, `start_encounter_session_stack.go` |
| "Help" (the goblin stops being an enemy) | **SHIPPED as a flip.** `on: { intimidated: { fact: … } }` teaches witnesses a fact; a disposition `until: { fact: … }` flips the pair to neutral or allied. An allied creature is not a target. It does not fight *for* the party (minds hunt players only). | `encounter/intimidate.go`, hold-out dispositions |
| The check, the deed, the beat | **SHIPPED** with Intimidate. Roll seen; deed on witnesses; fact on witnesses. | [intimidate.md](intimidate.md) |
| Advantage and disadvantage on a check, per source, in the log | **SHIPPED.** The check result records which source granted or imposed it. | `resolution/check.go`, `checks.AbilityCheckResult` |
| A ring under a token colored by faction | **SHIPPED**, but from the roster: the same color for every viewer. | `rpg-dnd5e-web` `factionColor.ts`, `SessionCanvas.tsx`, `HexEntity.tsx` |
| Author text a creature says | **NOTHING.** No content field anywhere carries creature speech. | — |
| A mind that reads stance when it picks a target | **NOTHING.** Minds target "any standing player I can see". Neutrality holds only because a neutral creature is never driven. | `behavior/basic.go` closest |
| A creature acting outside a fight | **NOTHING.** Monsters are driven only inside a bubble. A neutral goblin cannot flee, run for help, or lead anyone anywhere. | `session/clocks.go` driveMonsterTurns |
| A social verb offered outside a fight | **UNVERIFIED, probably NOTHING.** Afford blocks Intimidate as "not your turn"; the front room has no turn. | `session/afford.go` |

The last three rows are the primitives this scenario pays for. They are the same three Regroup and Alarm were going to need, so nothing here is spent twice.

## The authoring shape

On the goblin's placement. Keys are verbs and outcomes so the next verb adds a key, not a field (the `OnSpec` comment already asks for this).

```yaml
place:
  - id: front-goblin
    ref: goblin
    faction: goblins
    at: [3, 4]
    intimidate: [{ ability: intimidation, dc: 12 }]
    persuade:   [{ ability: persuasion,   dc: 10 }]
    on:
      intimidated:
        - { weight: 70, say: "Fine, fine! The cellar door is behind the barrels.", fact: goblin-cowed }
        - { weight: 30, say: "Boss! BOSS!",                                        flee: {} }
      intimidate_failed:
        - { weight: 100, say: "Big talk for someone standing in my doorway.",     alarm: { toward: bandit-hall } }
      persuaded:
        - { weight: 100, say: "Bandits took the cellar. Go left at the rope.",    fact: bandits-in-cellar }
      persuade_failed:
        - { weight: 60, say: "Nothing down there, friend. Go right.",             tell: { fact: cellar-empty, true: false } }
        - { weight: 40, say: "Follow me, I know a way round.",                    lure: { to: pit-room } }

factions: [{ id: goblins }, { id: bandits }]
dispositions:
  - { between: [goblins, party], stance: neutral, until: { fact: goblin-cowed }, then: allied }
  - { between: [bandits, party], stance: hostile }
```

Rules of the table:

- **One key per outcome, one entry fires.** Weights are relative, summed by the engine; an author can write 3 and 1 or 75 and 25. A single entry with no weight is 100.
- **Every entry is one word from the outcome vocabulary plus `say`.** Two words in one entry is an error at validation, so an author never has to guess ordering. Want two things to happen? Two entries do not do that either; a later slice may add a list under one entry when a use case pays for it.
- **`say` is text, carried verbatim on the beat.** The engine never composes it. Absent means the creature says nothing.
- **`fact` teaches the witnesses**, as Intimidate does today. **`tell` teaches the party a fact and may mark it false**, which the intel log already allows: testimony is per observer and can be wrong. A false fact is how bad directions and a bluff get into a run.
- **Failure has a table too.** A failed Intimidate that raises the alarm makes attempting worse than not attempting. That is deliberate, and it is what gives the untrained rule teeth.

### The outcome vocabulary, with state

| Word | What the creature does | State | Slice |
|---|---|---|---|
| `fact` | witnesses learn an authored fact; dispositions may flip on it | SHIPPED | this |
| `say` | the beat carries the author's line | new, small | this |
| `flee` | the creature's mind receives fear (the coward's onset), never a flag | mind exists; needs a creature that acts outside a fight | this, if "acts outside a fight" lands here; else Regroup |
| `tell` | the *party* learns a fact, possibly false | intel holds lies already; "learn from a creature" is new | this |
| `alarm` | runs to named allies and lands a rumour | designed as slice 3, not built | Alarm |
| `lure` | leads the party toward a place | nothing exists | later |
| `pretend` | the creature shows one stance and holds another | see the ring, below | Insight slice |

## The two rolls

1. **The player's check.** Unchanged from Intimidate: `resolution.MakeCheck`, best listed approach, pose window for Bardic Inspiration, the beat carries roll, total, DC, beaten.
2. **The world's reaction roll.** The engine picks one entry from the table for the outcome that occurred. The pick is a roll through the shared dice path so it is reproducible and, if ruled visible, shown.

**R1, RULED 2026-09-17 (Kirk): "everything visible in the log now, probably not story but the debug log for sure."** The reaction beat carries the die, the weights' total, the entry that fired and the creature's line. Web's story rendering shows the outcome and the line; the debug log shows the roll. Full data down the log until v1, as every beat does.

## The untrained rule

**The letter.** 2014 rules put no penalty on an untrained ability check: d20 plus the ability modifier, and proficiency adds its bonus.

**The divergence, ruled 2026-09-17.** A character without proficiency in the skill a verb uses rolls at disadvantage. Against the goblin's DC 9 with +0 Charisma: plain roll about 60%, disadvantage about 36%, proficient at +2 about 70%. Taking Intimidation at creation now matters at the table.

**Where it lives.** The character supplies the fact (proficiency level in the skill; today only the total is exposed, so a small read is added). Resolution applies the rule as a named source on the roll, so the log can say "untrained imposed disadvantage" the way it says "Raging granted advantage." Session and web change nothing. Rules stay in resolution; the sheet only answers questions about itself.

**Scope, R2 RULED 2026-09-17 (Kirk):** skill verbs only (Intimidate, Persuade, later Deceive). "The disadvantage is to make taking Intimidation worth something." The doc that adds a verb says it takes the rule.

**It must be one removable piece.** Kirk: "I am aware I am diverging from RAW. Ideally flipping that to RAW would be an easy refactor." So the rule is ONE named source in resolution (`untrained`), applied in one place, with one test that asserts it and one that asserts its absence when removed. Returning to the letter is deleting that source, not hunting for a special case in the modifier math. A build that spreads the rule across the sheet, the check and the offer has built it wrong.

**A wrinkle for later, named now:** a check with several listed approaches picks the character's best by modifier. Once untrained means disadvantage, an untrained +3 is worse than a trained +2 and "best" has to know it. No shipped check lists both a trained and an untrained approach for one character today; the first one that does brings the comparison.

**This revises a law.** The README said "nothing is gated; everything is a check; every character may attempt every verb on every monster." The attempt is still open to everyone. The roll is no longer the same for everyone. The README carries the correction.

## Persuade

Intimidate's twin: same check machine, same deed shape (`DeedPersuade`), same table keys (`persuaded`, `persuade_failed`), skill Persuasion, DC authored or derived from the creature's passive Insight, priced as an action. The mind learns "this one is not my enemy"; the retaliator's excuse and the coward's fear do not read it, and the stand-down the README names is exactly the "creature acts outside a fight" primitive below.

Two verbs is where a hand-listed verb becomes a cost: session names Intimidate in seven places and web in six (rpg-dnd5e-web#1104). Persuade's build pays that down, or files why not.

## Outside a fight

The front room has no fight. That is the whole point, and it is where the engine is thinnest:

- **Offering the verb.** Afford compiles rows on the turn clock and blocks "not your turn." A social verb must be offered on the world clock too, priced there in whatever the world clock prices (Search and Unlock already spend there, so there is a precedent to read).
- **Driving the creature.** `flee`, `alarm` and `lure` all need the goblin to move when no bubble exists. Today monsters act only inside a bubble. This is the primitive Regroup ("toward my friends") and Alarm ("run to the next room") were already going to need. **R3, RULED 2026-09-17 (Kirk): this slice brings it.** "This will be the first getting verbs outside combat." The front room goblin ships with `flee` able to fire, which means a creature acts on the world clock and a social verb is offered there.
- **Minds must read stance.** A neutral goblin that gets a turn must not attack. Target selection reads "opposed," which the encounter already answers per pair. Small, and load-bearing the moment any neutral creature acts.

## The ring: what a player believes about a creature

The faction ring exists and is truth: one color per faction from the roster, identical for every viewer. Kirk's ask is different and it is the perception law applied to stance:

- **A believed stance rides the per-viewer sighting**, next to name and kind, not the roster. The proto's own doc on sightings says the server must not state a fact a viewer's stale view could be wrong about, "because the fact is exactly what an illusion has to be able to lie about." A stance is such a fact.
- **The truth is what the creature shows, by default.** With no deception in play, every viewer's believed stance equals the derived stance, and the ring matches today's faction color.
- **`pretend` is where they diverge.** An authored outcome where the creature shows one stance (cowed, neutral) and holds another (hostile). A viewer's belief is what the creature shows, unless their Insight beats the creature's Deception, in which case they see the truth. Passive Insight first, the same way passive Perception feeds sightings; an active read is a later verb for the Insight skill.
- **The ring is presentation only**, as the existing one says on its own prop: it decides nothing about who may be attacked.

This gives Insight its use case beside Intimidation and Persuasion, and it is the first place a player's skill changes what they *see* rather than what they *do*.

## What this makes possible next

- **Deceive / disguise** gets its per-character stance from `pretend` turned around: the party pretending to the creature.
- **Alarm** becomes a table word once slice 3 lands; the author writes `alarm: { toward: … }` and nothing else changes.
- **Word spreads** stays shelved, still. A `fact` on witnesses plus a disposition is enough for a camp to turn; who else hears is a later verb.
- **Author-written speech** on every beat that has a creature in it, once it exists on this one.

## Rulings

All four ruled by Kirk on 2026-09-17, on rpg-project#457:

- **R1** the reaction roll is in the beat and the debug log; the story log shows the outcome and the line.
- **R2** untrained disadvantage on skill verbs only, built as one removable source so returning to RAW is a deletion.
- **R3** this slice brings "a creature acts outside a fight" and the social verb on the world clock. First verbs outside combat.
- **R4** an allied goblin stops being a target. Fighting for the party is a later wave.

## Build order, once ruled

1. rpg-project: this doc merged; tracking issue with the table shape and the vocabulary's shipped words.
2. protos: `Persuade` RPC and verb; `Reacted` beat `{creature, outcome, entry, say}`; `stance` on the sighting. Kirk merges.
3. toolkit, one module per PR, on pseudo-versions: dungeonspec table + validation; encounter reaction pick + `DeedPersuade` + believed stance on the view; resolution untrained source + proficiency read on character; behavior reads stance; session world-clock offer + Persuade + creature drive outside a bubble.
4. api and web on pseudo-versions; web reads the ring from the sighting and drops a hand list.
5. Kirk walks the front room: persuade, intimidate, fail both, watch the goblin bolt for the bandits, see the ring change for one character and not another.
6. Merge inside-out.
