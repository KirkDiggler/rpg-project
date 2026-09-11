---
status: DESIGN, proposed 2026-09-11
journey: rpg-project#430 (directed movement, step 2) · rpg-project#243 (cast a spell) · builds on: ../../battlemap/directed-movement/design.md (the directive; Away, the reaction and the provoke are its "harder fields") and ../thunderwave/design.md (the first push, landed 2026-09-11) · supersedes: ../heal-and-half/design.md §2 for this spell (rpg-project#414 keeps Cure Wounds and the record of the 2026-09-09 cut)
law: one spell, whole; the save buys half; the failure buys a flee the target pays for and is struck during; nothing about the world enters resolution, nothing about the rules enters encounter
---

# Dissonant Whispers — half on a save, and the flee

## 0. Why this spell, and what it proves

Kirk, 2026-09-11: *"we are doing one spell at a time here … on whispers it
seems like we could do the whole thing now cuz most of it is known and the
half on save is the new piece."*

Thunderwave proved the shape and the least permissive directive. Whispers is
a single-target save with the directive's three fields Thunderwave could not
exercise, plus the one outcome policy no spell has needed yet:

1. **Half.** A successful save buys half the damage, not nothing. Every
   save-for-half spell after this one is content.
2. **Away.** The route is a search on the field, not a line: the farthest
   reachable standable cell from the caster within the mover's own speed.
3. **The price.** The target pays its reaction before it runs, if it has
   one. A monster has never had one to pay; this wave gives it a meter.
4. **The provoke.** The flee is a walk that provokes, resolved per step by
   the movement machine, and when the reactor is a player the walk waits
   for their answer the way a monster's own walk does.

The walk: the bard stands beside a skeleton, the fighter beside it too.
Whispers. The skeleton fails. It takes 3d6 psychic, its reaction is spent,
and it runs for the far corner, six cells. The bard is asked whether to
swing as it leaves their reach, then the fighter is asked. Both swing. The
skeleton, struck twice, reaches the corner. On its own turn it cannot take
an opportunity attack on anyone, because the reaction is gone. A second
cast on a skeleton that makes its save: half of 3d6, rounded down, and it
stays where it is.

The 2014 and 2024 texts agree on the shape (Wisdom save; 3d6 psychic; on a
failure it must immediately use its reaction, if available, to move as far
as its speed allows away from you, avoiding dangerous terrain; half on a
success). We are not bound to either and this design follows them because
nothing in them is in the way.

## 1. What is true today

Verified 2026-09-11 against rpg-toolkit `origin/main` `9cc74df2` (after the
Thunderwave wave: spatial v0.13.0, encounter v0.73.0, dnd5e v0.155.0,
resolution v0.41.0, session v0.75.0). Two read-only surveys; every claim
below carries its file.

**Dissonant Whispers has no content of any kind.** No constant, no ref, no
data row, no profile. Every hit is a comment naming it as the customer for
machinery that already exists (`combat/actions/move.go:17,61`,
`encounter/directive.go:55,253,310`, `resolution/contest.go:484`,
`resolution/movement.go:76`, `session/mover.go:278`, `session/react.go:297`).
Psychic damage is declared (`damage/damage.go:111`) and Vicious Mockery
already deals it through a save-gated cast.

**The gate can say `Half` and nothing downstream can hear it.**
`saves.Half` validates at the gate (`saves/gate.go:206`) and is refused in
THREE places, not the two #414 counted: `combat/actions/cast.go:224` (the
cast profile), `combat/actions/attack.go:259` (the attack-rider condition),
and `resolution/contest.go:253` `validateConditionGate`, which is misnamed:
`Start` calls it before the machine knows whether a condition exists
(`contest.go:694`), so it gates damage-only contests too.

**The success branch returns before any delivery exists.**
`contestMachine.resolve` (`contest.go:857-859`): `if outcome.Succeeded {
return Done{Outcome: outcome}, nil }`, with the godoc stating it as a rule.
On the failure branch damage is rolled inside `applyPreparedDamage`
(`contest.go:405`), the trace is built FROM the components
(`damageCalculation`, `:573`), and the continuation calls `reportDamage`
then `runFollowUps` (`:913,920`), which is how a concentrating target owes
its Constitution check. Those two have exactly two callers each, both on
failure branches (`contest.go`, `strike.go`).

**The trace has no place for a halved total, and the guard that the trace
explains the number is enforced three times.** `RollCalculation`
(`events/roll_trace.go:115`) is components plus a total;
`ValidateRollCalculation` (`:219-248`) defines the arithmetic as a strictly
additive per-component sum, with Bane's `SubtractDice` (`:111`) the only
operator, at the dice level. The guard: `resolution/contest.go:427`
(`FinalDamage` total must equal the calculation's), `encounter/activation.go:434`
(`Calculation.Total == Requested`), `session/events.go:886-896` (read-back).
The one halving that exists, `DamageComponent.Multiplier` 0.5 for
resistance (`events/events.go:227`), lives at the `FinalDamage` layer and is
never read by the trace, so a multiplier component trips the first guard.
A negative modifier-only component is legal today (`validateRollComponent`,
`:254`) and needs only a source ref and a name.

**The directive is built and refuses exactly what Whispers brings.**
`CastMove{Policy, Cells, Speed, Pays, Provokes}` validates `Speed: true` and
`Pays: PaysReaction` in content already (`combat/actions/move.go:81-108`);
only a policy other than `MoveLine` is refused there. Resolution's
`validateMove` (`contest.go:486-508`) refuses `Pays != PaysNothing` and
`Speed` with its doc naming this spell: *"Dissonant Whispers brings both,
with the spend and the lookup. It deletes these two arms; it does not work
around them."* Session refuses `Speed` a SECOND time
(`session/castmove.go:98-105`) and passes `Budget: push.move.Cells` verbatim
(`:116`). Encounter's `Route` refuses any policy but `MoveLine` with
`ErrUnsupportedPolicy` (`directive.go:135`), and its `routeLine` is
arithmetic along `GetLineOfSight`, not a search.

**Away's parts exist unassembled.** `floodFrom` (`encounter/clocks.go:1326`)
floods `spatial.Field` under the fold with no limit; `spatial.FieldInput.Limit`
exists and no encounter caller uses it (`tools/spatial/field.go:23`);
`nearestStop` (`clocks.go:1367`) picks a standable cell by a goal with the
scan-order tie-break; `Encounter.Distance` is the grid's ruler
(`encounter.go:1069`, cube distance on the hex field).

**Speed is on the roster row session already reads.** `encounter.Member.SpeedFeet`
(`encounter/field.go:1349`), filled at join from the sheet's `GetSpeed()`
(`session/write.go:472`); `encounter.CellsFromFeet` (`encounter/units.go:33`)
is the one place feet become cells, by Kirk's rpg-project#254 ruling.

**A monster has no reaction to spend, by ruling.** `Monster.CanReact()`
returns a hard-coded `true` (`monster/monster.go:198-208`: *"TRUE IS THE
ANSWER, not a placeholder"*); the only subscriber to
`SpendRequestedTopic` is the character keeper (`character/sheet_keeper.go:171`);
`monstertraits/loader.go:450-455` states the asymmetry as a ruling: *"A
monster has no action economy at all, so its UsedThisTurn is the only meter
there is."* That meter belongs to the opportunity-attack condition
(`conditions/opportunity_attack.go:395-421`), resets on the monster's turn
start (`:222-229`), and is persisted on the sheet blob. Nothing outside
that condition can spend a monster's reaction, and nothing can find it spent.

**The active-turn holder can take an opportunity attack.** No gate at any
layer reads whose turn it is: not the condition (`opportunity_attack.go:306-378`),
not the machine (`resolution/movement.go`), not reactor selection
(`session/mover.go:526-579`). Readiness is installed for every member of
every interaction (`resolution/truth.go:114`). So the caster who whispers
at an adjacent creature gets the swing as it leaves their reach, on their
own turn, for their reaction. That is the signature play and it costs no
new code.

**A provoking directed walk by a monster pauses, and `Direct` turns the
pause into an error after the cast is recorded.** When the mover is a
monster, `moverSeam.Move` asks players instead of swinging for them
(`session/mover.go:88`, Kirk's rpg-project#316 rung 3) and returns
`StepPausedError`; `walkPath` returns the pause with the pending cells
(`clocks.go:968-984`); `Direct` refuses with `ErrStepPaused` because *"a
directed move has no turn to hold the rest of the walk on"*
(`directive.go:349-353`), and its doc says *"the day a directive provokes
(Dissonant Whispers), this refusal is the thing that has to be answered."*
In `session/cast.go` that arrives at `:425`, after `RecordCast` at `:402`.
The paused-walk holder that exists is the turn's (`encounter/pause.go`,
`ResumeTurn`), keyed to the active member.

**Two facts the wire already has wrong, named and left.** The movement
beat's cause is written by encounter (`encounter.go:1324`) and dropped by
session's `MovedBody` (`session/types.go:1740`); and the cast beat's
`MoveImposed.MovedCells` is the route's length, written before the walk
(`castmove.go:131`), with `DirectOutput` discarded (`:154`). Both are
rpg-api-protos#329's family and the visualization lane's, not this wave's.

## 2. Ownership

| noun | owner | why |
|---|---|---|
| the profile: 60 ft, one creature, WIS gate, half, 3d6 psychic, the move `{Away, Speed, PaysReaction, Provokes}` | content (`spells/cast.go`) | a spell is its row |
| half: the outcome policy, the halved number, the trace that explains it | resolution's contest | it is what a save buys |
| the halving in the trace | one more component, same grammar | the trace explains the number; the guards stay |
| the price: is a reaction available, spending it | resolution asks the combatant and publishes the spend request; the reactor's keeper debits | economy is the sheet's |
| a monster's reaction meter | the monster's keeper, persisted on its blob | a monster has a sheet blob; the meter goes where `UsedThisTurn` already goes |
| the speed in cells | session reads `SpeedFeet` off the roster, converts with `CellsFromFeet` | the sheet role, the one conversion place |
| the route: Away | encounter (`Route`) | geometry under the fold |
| the walk, each step's reactions, the pause and the resume | encounter (`Direct`, with a held directive) | it owns the walk and the pause |
| asking a player whether to swing | session's mover seam, unchanged | already the one place |

The fold gets no new author. A fleeing creature enters only cells the fold
calls not Blocked and stops only on Standable, exactly as a monster's own
route does.

## 3. Half

**The row says `Half`.** Three refusals are narrowed, not deleted: `Half` is
permitted only when the profile declares damage and delivers no condition
(a halved condition is meaningless and would be an affordance with nothing
behind it). `validateConditionGate` is renamed `validateGate`, since it
gates every contest.

**The success branch delivers.** `resolve` gains one branch: on success
with `OnSuccess == Half`, `applyPreparedDamage` runs with the halving
applied, then the same continuation as the failure branch: `reportDamage`,
`runFollowUps`. A wizard concentrating on their own spell who makes the
save still takes half and still owes the Constitution check. No imposed
condition, no imposed move: the save was made.

**The halving is a component, not a flag.** #414 proposed `Halved bool` on
`RollCalculation`. The survey shows why not: the guard that the trace
explains the number is enforced in three layers, the trace type is
mirrored in `events`, `encounter`, and `session` and on the wire, and the
client renders components. A flag would need every one of those to learn
it, and until the web did, a trace would show dice summing to 14 above a
total of 7. Instead, after the pools are rolled and summed, one more
`DamageComponent` is appended, of the same damage type, with
`Roll.Modifier = floor(sum/2) − sum` and a source naming the spell and
`Label: "halved by a successful save"`. `damageCalculation` builds the
trace from the components, so the trace carries the same component, the
three guards hold by construction, and every renderer that shows Bane's
`−1d4` shows this. Rounding is the tabletop's: half, rounded down; the
component is the rounded half's complement.

What it forecloses: nothing. If a later reader needs to know a roll was
halved as a fact rather than a line item (an Evasion that turns half into
nothing), that is a property of the save outcome and lives on
`ContestOutcome`, not in the arithmetic.

**Thunderwave's row flips.** Its comment promised it: *"the row flips to
`saves.Half` with no code change here the moment half resolves."* The
promise holds only once the contest's gate validator is widened, which
this wave does, so the flip rides here. Struck if Kirk wants Thunderwave to
stay all-or-nothing.

## 4. Away

```go
// encounter
const MoveAway MovePolicy = "away"
```

`Route` for `MoveAway`: flood from the mover with `Limit: Budget`, passable
as `floodFrom` already defines it (edges the canvas allows; cells the fold
does not call Blocked, so an ally may be crossed). Among reached cells that
are Standable and whose `Distance` from the anchor is strictly greater than
the mover's own, choose the greatest `Distance`; ties by shorter walk, then
scan order. The path is `field.PathTo(best)`. If no reached cell is farther
than where the mover stands, the route is empty and `StoppedBy` says so
("nowhere farther from <anchor> within <n> cells"), which is the pinned
case the design of `ErrUnsupportedPolicy` insists must be distinguishable
from a bad policy.

Distance is the ruler's, not the walk's, per directed-movement §3: "away
from you" means far from you. A creature in a dead-end corridor that runs
six cells down it and ends nearer the caster by the crow does not run.

Content's `MoveAway` joins `combat/actions/move.go`; `routePolicy` in
session learns the crossing. Both closed sets grow by one word, in the
places that refuse every other word.

Dangerous terrain: the text says the fleer avoids it. No cell is dangerous
today, so the clause is inert. When burning ground exists, `Away` treats a
cell whose fold carries an on-enter harm as not Standable for a flee.

## 5. The price, and a monster's meter

**Resolution asks and spends.** When a contest with `Pays: PaysReaction`
fails, before the move is described: `CanReact()` on the target. If false,
the move is recorded as not taken (`ImposedMove` with `NotTaken: "no
reaction to spend"`) and session maps it to `ResultMoved{Moved: 0,
StoppedBy: <that>}`, a real outcome, not a missing one. If true, resolution
publishes the same `SpendRequestedEvent` the opportunity attack publishes
(`ActionReaction`, amount 1, attributed to the spell), then describes the
move. Paid first, walked after; a wall two cells in does not refund.

**A monster gets a reaction meter, and it replaces the once-per-turn
flag.** Today the request would pass a monster by, so a monster would flee
for free every round and could still swing an opportunity attack after
fleeing. The primitive the lower layer lacks is one meter per creature per
turn. Kirk, 2026-09-11: *"monsters should have reaction and it should
replace that used once hack."* So:

- The monster's keeper (the loader that persists the monster's blob) holds
  `ReactionSpent bool`, subscribes to `SpendRequestedTopic` for
  `ActionReaction`, clears it on the monster's turn start, and
  `Monster.CanReact()` reads it. A character's meter stays
  `ReactionsRemaining`, already there.
- The opportunity-attack condition's `UsedThisTurn` is deleted, with its
  turn-start reset, its rest reset, and its persistence
  (`carryingFreeReactions`). The condition's gate is `CanReact()` alone,
  which it already asks, and its bill is the spend request it already
  publishes. Protection fighting style reads the same question today and
  needs nothing.
- One meter, one question, one bill, for both kinds of creature. A monster
  that swung on the fighter's turn cannot be made to flee on the bard's;
  one that fled cannot swing; a monster's reaction comes back at the start
  of its own turn, exactly as a character's does.

This reverses one sentence of a recorded ruling (`monstertraits/loader.go:454`,
"a monster has no action economy at all") and none of its reasoning: the
ruling was that a monster's reaction is metered by the reacting condition
because nothing else ever asked. Whispers asks. The meter is the smallest
economy a monster can have, and it is not a `SpendProfile`, not slots, not
the character's ledger.

## 6. The provoke, and the held walk

The flee reaches the mover seam with `Forced: false` (encounter inverts
`Provokes`), so the movement machine seeds no prevention and every reactor
in reach is offered the swing. When the reactor is a player, the seam asks
instead of swinging (rpg-project#316), and today that pause is `Direct`'s
error.

**The answer: a directed walk can be held.** Encounter gains a held
directive beside the held turn: `{Mover, Cause, Remaining []Position,
Provokes, Moved int}`. When `walkPath` returns a pause inside `Direct`, the
remainder is held on the encounter (persisted with the composition, as the
paused turn is), `Direct` returns `DirectOutput{Paused: true, Moved: n}`,
and the same window that holds a monster's paused walk holds this one: the
player answers, and the resume path continues `Direct` with the remainder
instead of `ResumeTurn`. The beats land in walking order with the reaction
attack beats interleaved, exactly as a monster's own paused walk lands
them. A player caster whose own window it is (the bard beside the
skeleton) answers on their own turn; nothing about that is new to the
window.

Why not swing for the player automatically during a forced move: it
forecloses declining, which is the whole reason #316 asks, and it makes the
one directive whose point is being struck on the way out the one walk a
player never gets to choose on.

`Direct` keeps refusing to push the currently paused member; that guard is
about a body a window is measuring reach against, and it holds.

## 7. The cast

```go
DissonantWhispers: {
    name: "Dissonant Whispers",
    cost: slotCost(resources.SpellSlotLevel1),
    build: func(spellSaveDC int) actions.CastProfile {
        return actions.CastProfile{
            RangeFeet:  DissonantWhispersRangeFeet, // 60
            Target:     actions.CastTargetOneCreature,
            MinTargets: 1, MaxTargets: 1,
            Save: &saves.SaveGate{
                Abilities:  []abilities.Ability{abilities.WIS},
                DC:         saves.DCStatic(spellSaveDC),
                OnSuccess:  saves.Half,
                Recurrence: saves.RecurrenceNone,
            },
            Damage: []damage.Damage{{Dice: DissonantWhispersDamage /* 3d6 */, Type: damage.Psychic}},
            Move: &actions.CastMove{
                Policy:   actions.MoveAway,
                Speed:    true,
                Pays:     actions.PaysReaction,
                Provokes: true,
            },
        }
    },
},
```

Order on a failure, as Thunderwave ruled it: damage, then the price, then
the move. A target the damage drops is not asked to pay and does not run.
On a success: half, and nothing else.

Single-target candidates are sight-gated (`session/offers.go`), which is
"a creature you can see". Range is caster-centred. No new target kind.

**The bard's pick** becomes `Options: {Bane, Thunderwave, Dissonant
Whispers}`, `Count: 3`; the count tracks the catalogue
(`requirements.go:435-443`, rpg-toolkit#1661).

## 8. Acceptance contract

Through production entrypoints.

1. **Half, rounded down.** A saved 3d6 of 13 deals 6; the trace shows the
   three dice, a component "halved by a successful save −7", total 6; all
   three guards accept it. A saved 14 deals 7.
2. **Half still breaks concentration.** A target concentrating who makes
   the save takes half and owes a Constitution check at max(10, half/2),
   resolved as a nested follow-up. This is the scene that fails if the
   success branch skips `reportDamage`.
3. **Half is damage-only.** A profile with `Half` and a delivered condition
   is refused at content and at the contest with a reason.
4. **Away picks far by the ruler.** On a field with a corridor that bends
   back toward the caster, the fleer takes the open floor, not the corridor,
   and ends at the reached standable cell of greatest grid distance; ties
   are stable.
5. **Pinned is a real answer.** A fleer with the caster at the mouth of its
   dead end does not move; the result says nowhere farther within its
   speed.
6. **Speed is the budget.** A 30-foot creature routes at most six cells; a
   25-foot creature five. The budget is read from the roster row, not the
   profile.
7. **Pay first.** A target with no reaction left is dealt damage and
   records a move not taken; a target with one is debited before the first
   step; a wall two cells in does not refund.
8. **A monster pays, from one meter.** A monster that fled on the bard's
   turn cannot swing an opportunity attack until its next turn starts; one
   that swung cannot be made to flee; a monster that swung once cannot
   swing again that round, with no `UsedThisTurn` left in the tree to say
   so.
9. **The flee provokes.** A monster fleeing out of the fighter's reach is
   struck by the fighter (automatically, when the fighter is a monster;
   after a window, when a player), and the beats interleave in walking
   order.
10. **The held walk.** A player asked mid-flee answers yes and the walk
    resumes to its end; answers no and it resumes untouched; the cast beat
    was recorded before the walk either way.
11. **The caster's own swing.** The bard adjacent to the target is offered
    the window on their own turn and the strike lands on the fleeing
    creature.
12. **The dropped do not run.** A target reduced to zero by the damage
    pays nothing and stays.
13. **Regression.** Thunderwave still pushes two cells without provoking;
    Bane, Thunderclap, Sacred Flame, Vicious Mockery, True Strike, Blade
    Ward resolve unchanged; the bard's pick is 3 of 3.

## 9. Rejected alternatives

- **`Halved bool` on the calculation** (#414 §2.3). Three mirrors, the wire,
  and the client renderer before a trace stops contradicting its total; the
  component is the same truth in the grammar that exists.
- **A 0.5 damage multiplier.** Lives at the `FinalDamage` layer, unread by
  the trace, trips the first guard, and conflates a save's half with
  resistance, which stacks differently.
- **Half applied per pool.** Rounds each pool separately; the rule halves
  the total.
- **Away by walking distance.** Runs the fleer into the nearest dead end.
- **Swinging for players during a forced move.** Forecloses declining and
  takes the choice away on exactly the walk it matters most.
- **Whispers as an instruction the monster obeys on its own turn.** A real
  option and a real divergence: the target stands beside the squishy until
  its turn and keeps its reaction. That is Command's shape, and the
  monster-with-instructions design decides it on purpose; this spell keeps
  the text's immediate, reaction-priced flee.
- **A monster `SpendProfile`, slots, the character's ledger.** More economy
  than the customer asked for; one meter is the primitive.

## 10. Sequence, one module per PR, merged bottom-up

1. **events / combat / saves** (dnd5e root): `MoveAway`; the three `Half`
   refusals narrowed to damage-only; the halving component's source label.
2. **monster / monstertraits / conditions** (dnd5e root, same PR as 1 or
   its own): the reaction meter, subscription, turn-start clear, `CanReact`
   reads it; the opportunity-attack condition's `UsedThisTurn` and its
   resets and persistence deleted.
3. **encounter**: `MoveAway` in `Route` (flood with `Limit`, farthest by
   ruler); the held directive and its resume; `DirectOutput.Paused`.
4. **resolution**: `validateGate`; the success branch delivering half with
   the component; the price asked and spent before `imposeMove`;
   `ImposedMove.NotTaken`; the two refusal arms in `validateMove` deleted.
5. **session**: the `Speed` refusal deleted; budget from `SpeedFeet` via
   `CellsFromFeet`; `routePolicy` learns Away; the react answer path resumes
   a held directive; `NotTaken` mapped to a zero `ResultMoved`.
6. **spells**: the profile, the constants, the ref; the bard's 3 of 3;
   Thunderwave's row flips to `Half`.
7. **rpg-api**: the sandbox seed's bard knows the spell. No proto change:
   the cast request, `MoveImposed` and the reaction attack beat already
   carry everything the walk shows.
8. **rpg-dnd5e-web**: nothing required; the armed member-target cast and
   the reaction window already exist.

Walked on the local stack from pushed pseudo-versions before anything leaves
draft.

## 11. Where the evidence is thin

- Not run. No production code, no test claimed passing.
- Whether `combat.FinalDamage` accepts a negative modifier component of a
  damage type without complaint was not read; the builder verifies it
  first.
- The resume path for a held directive was sketched from `pause.go` and
  `react.go` by their docs, not traced line by line; how the react answer
  distinguishes a held turn from a held directive is the builder's first
  question.
- Whether the monster's keeper can subscribe on the same bus the condition
  publishes on at `Attach` time was inferred from `carryingFreeReactions`,
  not read.
- rpg-api's sandbox seed was not opened.

— cross-team agent, on behalf of KirkDiggler
