# The record train

For [rpg-toolkit#2002](https://github.com/KirkDiggler/rpg-toolkit/issues/2002),
a follow-up of [rpg-toolkit#1965](https://github.com/KirkDiggler/rpg-toolkit/issues/1965).
The law is [design.md](design.md); the slices are [slices.md](slices.md).

A landing writes every sheet before it tells a single beat, so the standing
consult always answers from the end state of the whole output. When a landing
tells several units, such as a multiattack's swings, a strike and its
retaliation, or the reactions to one step, each unit must reach the story
before anyone asks who is standing. **`Encounter.RecordTrain`** takes the
ordered units, appends every beat of every unit, and then runs the standing
consult once. A unit is either an outcome or an activation, and each carries
its own concentration. A single outcome is a train of one: `Record` is
deleted, and the append-and-notice body is the only path. A swing inside a
multiattack names its sequence on its beat. Session's record functions build
one train per landing and call it once. `land`'s order does not move.

## Walk one thing through

A goblin boss multiattacks a cleric at 12 hit points who holds Fog Cloud. Each
scimitar swing hits for 8. A fighter stands out of reach.

1. Resolution swings twice. The first swing breaks the cleric's concentration
   and the second drops her. Resolution's output carries a sequence with two
   steps. The first step carries its own check and break.
2. The landing writes the cleric's sheet at 0, then calls the record step.
3. The record step builds two units. Each unit is a struck outcome naming the
   Multiattack as its sequence. The first unit carries the concentration. The
   step calls `RecordTrain` once.
4. The encounter prepares both units before it appends anything. It appends
   struck, saved, concentration ended, then struck. It lands each swing's
   attack deed behind that swing's own beats. Then it asks who is standing
   once. The cleric is down, so `down` lands last, behind the blow that caused
   it.
5. If the cleric was the last party member standing, the party-defeated ending
   lands after `down`. Nothing is refused, because the encounter closes after
   the last beat.

## Pointers

| Concern | Where (rpg-toolkit `rulebooks/dnd5e/`) |
|---|---|
| The train verb and the one append path | `encounter/outcome.go` `RecordTrain`, `appendTrainAndNotice` |
| What a unit prepares | `encounter/outcome.go` `prepareRecord`, `encounter/activation.go` `prepareActivation` |
| The standing consult | `encounter/standing.go` `noticeDown` |
| A transaction noticing once after all its beats | `encounter/cast.go` `RecordCast` |
| The record functions that build a train | `session/landings.go` `recordAttack`, `stepRecord` |
| The landing order | `session/land.go` `land` |
| The swing event bodies | `session/types.go` `StruckBody`, `MissedBody`, `WardedBody` |
