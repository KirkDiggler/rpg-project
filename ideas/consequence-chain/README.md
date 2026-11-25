# Consequence chain: a fact you carry forces a save, a failed save costs a check

Status: idea, pre-architecture. Kirk, 2026-10-25, from the game designer.
Castle story rpg-project#639.

## The scene

The chef asks for help in the kitchen. You run a series of checks. Fail them,
and you carry that. Later, at the performance, the chef heckles you from the
crowd: a saving throw. Fail it, and you perform at disadvantage.

## The chain, link by link

| link | what it is | exists today |
|------|-----------|--------------|
| 1. a series of checks | skill checks the kitchen scene asks for | checks exist (`rulebooks/dnd5e/checks`); a scripted series is content |
| 2. failing leaves a mark | a `fact` the witnesses learn: `kitchen-botched` | `fact:` is a row output every witness learns; `when: { fact: … }` reads it |
| 3. the mark is a trigger later | the chef's table has a row keyed on the fact AND the moment | rows are rolled under `when` lists; "the moment" is one more fact (`performance-begun`) set when the performance starts |
| 4. the NPC acts on you | a verb the chef performs against a player: heckle | social verbs run player → NPC (`Speak`, intimidate/persuade keyed `success`/`failure`). NPC → player is the gap |
| 5. the verb forces a save | a Wisdom or Charisma saving throw with a DC | modifiers exist on the character; a general forced-save resolution outside death saves is thin |
| 6. a failed save lands a condition | `rattled`: disadvantage on the next Charisma (Performance) check, or on checks for N rounds | conditions as effects with disadvantage exist (checks, attacks, sequence); a new condition descriptor is content plus a descriptor |
| 7. the check reads the condition | the performance check rolls at disadvantage | checks already read conditions for disadvantage |

Five of seven links exist. The two that do not are the same primitive seen
from two sides: **an NPC verb against a player whose resolution is a save**.

## The shape that looks elegant

- The heckle is a table row, not a script: `when: [{ fact: kitchen-botched },
  { fact: performance-begun }] → heckle: target` on the chef's table, rolled
  like every other row. A kinder chef has a lower weight; a soldier never
  heckles. The record explains why it fired (S6).
- `heckle` is one of a small family of **NPC-to-player social verbs** whose
  resolution is a saving throw (heckle, taunt, intimidate-back). The verb
  names the ability and the DC source; the save rolls on the shared dice; the
  outcome lands as a condition through resolution's existing arm, never a
  sheet write (the driven-monster rule).
- `rattled` is a condition descriptor like any other: what it modifies
  (disadvantage on Charisma checks), how it ends (the next such check, or N
  rounds), how it reads in the UI. The performance check needs nothing new.
- The kitchen series is content: a prop (the chef) with speech options whose
  success/failure rows set facts. The only engine piece is link 4/5.

## Open

- Who makes the save: the heckled player as a reaction window (they could
  spend inspiration), or an automatic roll on the shared dice with the
  result delivered. The reaction-window machinery exists; the question is
  whether a heckle is worth a window.
- Whether `rattled` ends on the next check or after a duration; the designer's
  scene says the next performance.
- Whether facts should carry a scope (this run, this scene) so `kitchen-botched`
  does not follow the party into the next castle. Today facts are world facts
  for the encounter.
- Where the NPC-to-player verb's DC comes from: the NPC's stat block (a
  heckler's Charisma) or the row.
