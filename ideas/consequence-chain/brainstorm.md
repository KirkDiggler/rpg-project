# Brainstorm: how a carried fact can force a save that costs a check

Working notes, 2026-10-25. Loose on purpose; the design doc comes after a
direction is picked. The README holds the scene and the link table.

## What the probe found (measured, not guessed)

- **A save is already a resolution machine.** `resolution/save.go` rolls a
  saving throw on the shared dice and `save_pose.go` can stop to offer the
  saver a die (inspiration on a save). The reaction window on a save is paid
  for.
- **Vicious Mockery is already a condition.** `conditions/vicious_mockery.go`:
  a save on the bard's turn; on failure the TARGET carries "disadvantage on
  the next attack roll before the end of its next turn", consumed by that
  roll. The heckle is this with "next ability check" in place of "next attack
  roll".
- **Checks already read conditions.** `checks.go` fires the chain through the
  bus so a condition can impose disadvantage on a skill check. Nothing new for
  the performance roll.
- **Facts are per-knower.** `knowsFact(actor, fact)`: a fact is what a
  creature knows, not a world flag. The chef learning `kitchen-botched` is
  the chef's own knowledge; it does not follow the party into another castle
  unless the chef does.
- **Frightened exists only as a ref**, not as a condition in the registry.
- **Monsters reach the rules through host capabilities** (`Striker`, `Mover`,
  `Disengager` in the encounter) and resolution's Driven arm; there is no
  table word today that fires a sheet ability, and no NPC-to-player verb.
- **Templates can add actions at assembly** (rpg-toolkit#2121, shipped
  2026-10-25): content can give the chef an action his base stat block lacks.

So the whole chain is one missing door: **an NPC acting on a player with a
save, landing a condition.** Five ways to cut that door.

## Five shapes

### A. The heckle is a spell

The chef casts Vicious Mockery, generalised: the condition's roll kind becomes
a parameter (next attack / next ability check / next Charisma check).

- For: everything exists; one parameter on one condition.
- Against: a chef casting a cantrip is the fiction leaking through the
  mechanism. Every future taunt, bellow and glare becomes a "spell". Casting
  has its own economy (slots, components) that a heckle does not have.

### B. The heckle is a monster action

A stat-block action, like Frightful Presence: "one creature within 30 ft
that can hear: Wisdom save DC 12 or rattled until the end of its next turn."
Content gives the chef that action through the template (addActions). A new
table word names a sheet action (`act: heckle` with the usual target
selector); a new host capability beside `Striker` runs it through
resolution's Driven arm and the save machine; the failed save lands the
condition through resolution's existing arm.

- For: this is the full-D&D door. Every monster ability that says "make a
  save" (breath, frightful presence, a ghoul's claw) comes through the same
  word, the same capability, the same machine. The chef's agency is on his
  table, rolled like everything else, explained by the record. The action's
  DC and reach live on the sheet where D&D keeps them.
- Against: the largest cut. A table word, a capability, the Driven activation
  of a save-shaped action, and a condition. The chef's Heckle needs an action
  economy outside a fight: he acts at world pace, which the world clock
  already gives NPCs, but the ask-then-act path is written for the fight.

### C. The heckle is a social verb pointed the other way

The player has intimidate/persuade against NPCs (`Speak`, keyed on
`success`/`failure`). Mirror it: an NPC row `intimidate: target` runs the same
contest against a player, and the player's answer could be a retort window.

- For: one social system, both directions; a player could answer back.
- Against: today's social verb is a CHECK by the actor against a disposition,
  not a SAVE by the target, and its outcome is a fact or a disposition, not a
  condition. Making it do both blurs two different rules. The RAW heckle is a
  save, so this is the wrong primitive wearing a familiar name.

### D. No verb: the mark is a condition that triggers itself

Failing the kitchen puts "shamed before the chef" on the character as a
condition. The condition subscribes to the check chain: when a Performance
check begins with the chef in sight, it forces the save and, on failure,
imposes the disadvantage. The chef does nothing.

- For: the smallest engine change; conditions already subscribe to chains and
  checks already read them; no table word, no capability.
- Against: the chef's agency is gone. A kind chef cannot decide not to heckle;
  a soldier chef and a coward chef heckle alike; there is no row to tune and
  no `answered` beat to explain. It hides a decision inside a rule, which is
  the opposite of what the table grammar exists for.

### E. A generic save-or-condition row word

The chef's row says it all: `afflict: { target, save: wis, dc: 12, condition:
rattled }`. No stat block involved; the row IS the ability.

- For: author-tunable in one place, explained by the record, serves
  heckle/taunt/curse with one word.
- Against: it makes the table a second owner of a creature's abilities. D&D
  keeps abilities on the stat block; a row that invents one per line means
  the studio has two places to look for "what can this creature do", and a
  monster's sheet stops being the truth. Tempting, and a band-aid.

## Weighing

| | A spell | B action | C social mirror | D self-trigger | E row word |
|-|---------|----------|-----------------|----------------|------------|
| the chef decides, on his table | yes | yes | yes | **no** | yes |
| abilities stay on the sheet | n/a | **yes** | n/a | n/a | **no** |
| serves the rest of D&D's saves | no | **yes** | no | no | partly |
| player gets a reaction window | yes | **yes** | yes | yes | yes |
| record explains it | yes | yes | yes | **no** | yes |
| size | small | **large** | medium | small | medium |
| fiction leak | **casting** | none | wrong rule | none | none |

**Leaning: B**, built as the smallest cut that keeps it true: one table word
that names a sheet action, one host capability shaped like `Striker`, the
existing save machine and Driven arm, and `rattled` as a condition with the
roll kind as a parameter (Vicious Mockery becomes its "next attack" instance,
or stays as it is beside it). The chef gets Heckle through the template, so
content authors give any NPC a save-shaped action without touching Go.

What B buys next, which is the test of whether it is the primitive: a dragon's
breath, a ghoul's paralysing claw and a banshee's wail are the same word on a
table and the same capability in the host, with only the sheet differing.

## Where the trade-offs bite

- **World pace.** The performance is free roam. B needs the chef's row to fire
  at world pace and the activation to run without a fight. The world clock
  already drives NPC turns outside fights (S1 pursuit, the alarm), so this is
  a check, not a build; it may surface the "when do we return to free roam"
  question already open on #667.
- **Who rolls the save.** The save machine can pose the offer to the saver.
  A heckle is small; should the player be asked at all? Proposed: the machine
  poses only when the saver holds something to spend (inspiration), else it
  rolls through. That is the machine's existing rule for strikes; confirm.
- **Facts versus conditions for the mark.** The fact is the chef's knowledge
  (per-knower) and gates his row. It is not on the player at all, which is
  right: the player carries nothing until the heckle lands. D would put it on
  the player as a condition, which is why D loses the chef's agency.
- **The moment.** `performance-begun` as a fact the chef learns when the
  performance starts is honest (he is there), and it is content. A `scene`
  condition kind would be a second way to say the same thing; not needed.
- **Duration.** The designer's scene says the next performance. Vicious
  Mockery's "next roll of that kind before the end of your next turn" is the
  RAW-shaped reading; "the next Performance check whenever it happens" is a
  longer memory and a different condition. Pick when the condition is
  designed; both are parameters of the same descriptor.

## Questions for Kirk, in the order they matter

1. B as the direction: a save-shaped NPC action fired from a table row through
   a host capability, with abilities staying on the sheet?
2. Does the player get asked on the save (a window) or does it roll through
   unless they hold something to spend?
3. `rattled` ends on the next check of its kind before the end of their next
   turn (RAW shape), or on the next Performance whenever it comes?
