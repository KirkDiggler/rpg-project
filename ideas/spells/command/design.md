---
status: DESIGN, approved 2026-09-12 (Kirk ruled §7; first slice = Approach, Flee, Grovel)
journey: rpg-project#430 (directed movement, step 4) · rpg-project#243 (cast a spell) · builds on: ../../battlemap/directed-movement/design.md (the directive) and ../dissonant-whispers/design.md (Away, the held walk, the monster reaction meter; landed 2026-09-12)
law: the word is content; the compulsion is a condition on the target; the compelled turn is DRIVEN by the engine, not obeyed by a brain; a player and a monster are compelled through the one seam built for a member nobody is playing
---

# Command — the compelled turn

## 0. Why this spell, and what it proves

Kirk, 2026-09-12: *"command never hits the monster brain, turn driver takes
the turn for the monster or at least does something before they can. way
better than what I was thinking."* And the frame around it: *"getting a
foundation of monster behavior in place will enable the collaborator to
build robust systems … but our current task is to implement the command
spell options."*

Command is the fourth directed-movement customer and the first effect that
lands on a **future turn** rather than inside the cast. Every shipped
directive fires while the caster is still acting; Command stores a word on
the target and the target's own next turn is spent on it. That is the
"compelled turn" the directed-movement design (§10 step 4) said needed a
ruling before Command could be built.

The ruling this design proposes and builds on: **the engine drives the
compelled turn.** Nobody plays it. Not the human behind a commanded player,
and not the brain behind a commanded monster. The seam that already exists
for "a member nobody is playing right now" takes the turn, and the brain is
never consulted. Consequences:

1. **One mechanism for both kinds.** A commanded fighter and a commanded
   ghoul cross the same code.
2. **No brain can get it wrong.** Correctness does not depend on any
   present or future `TurnDriver` honoring an instruction.
3. **The behavior foundation is untouched.** The collaborator's lane
   inherits one fact (a compulsion is a condition the driver enforces) and
   one tool (an intent that asks the encounter to route), and nothing else
   moves under them.

What it proves for the engine, in order: a cast that carries a chosen
option; a condition that expires at the end of the **target's** next turn;
a participation answer that hands a player's turn to the driver; a turn
intent that asks the encounter for a route instead of handing it a path
(the same intent monster flee will use); `Toward` as a route policy.

The walk: the bard faces a ghoul two rooms away with a skeleton between
them. *Command: Approach.* The ghoul fails its Wisdom save. Nothing happens
yet; the log says the ghoul is commanded. Initiative reaches the ghoul. It
does not get its turn: the engine walks it toward the bard by the shortest
path, as far as its speed allows, provoking as it passes the fighter, who is
asked whether to swing. It stops beside the bard and its turn ends. Next
round it acts normally. Then *Command: Grovel* on the skeleton: it fails,
its turn arrives, it falls prone and passes. Then *Command: Flee* on the fighter's own
player, cast by an enemy in a later slice, and the fighter's turn is
walked away from the caster without the human being asked anything.

## 1. What is true today

Survey of `rpg-toolkit origin/main` 2026-09-12 (post Dissonant Whispers,
`rulebooks/dnd5e v0.157.0`, `encounter v0.74.0`, `resolution v0.42.0`,
`session v0.76.0`). Full report: the survey behind this doc.

**Command is a name and nothing else.** `spells/types.go:113` declares the
constant, `refs/spells.go:83` the ref, and the domain cleric grants it at
`character/choices/subclass_modifications.go:257`. No `SpellData` row, no
`castContent` profile, no condition, no test. By the directed-movement
design's own rule a spell with no profile does not exist. The
"monster-with-instructions design" the Whispers doc (§9) deferred to was
never written on any branch.

**Nothing turn-time exists.** Every directive fires inside the caster's
`Cast` (`encounter/directive.go:38-42`: a directive "charges no turn
budget, because it is not the mover's turn being spent"). Grep for
`Compel|Obey|Instruction|Directive|Command` across the five modules finds
only the activation-time family, `MoveStep.Forced` ("a fact about the step,
not an instruction", `encounter/turndriver.go:~570`), and comments naming
"a listener that treats a notification as an instruction" as the defect.

**A `TurnStart` subscriber cannot touch the turn.** `Announce` returns only
`error`, and an error aborts the whole verb (`encounter/turndriver.go:
~450-458`). `TurnStartEvent` is two scalars (`events/events.go:609-620`).
There is no turn-started story beat, only `"turn-ended"`.

**Turns are skipped by one seam, for both kinds.** `TurnParticipation` is
`Wait | AutoPass | Remove` (`encounter/participation.go:17-30`). Session
computes it from life state alone (`session/participation.go:160-171` over
`combat.ParticipationFor`). `driveTurnsWithParticipation`
(`encounter/clocks.go:321-409`) auto-passes an `AutoPass` member of either
kind, drives a `Wait` monster through the `TurnDriver`, and **stops** at a
`Wait` player (`clocks.go:403-405`). `Stunned`, `Paralyzed`,
`Incapacitated`: not in the tree.

**The driver seam is documented as kind-neutral.** `behavior/basic.go:5-13`:
`TurnDriver` implementations are for "ANY member nobody is playing right now
— monsters today, but … a disconnected player auto-passing until they
reconnect … land on the identical question." Nothing drives a player today.

**The brain is thirty lines and sees only data.** `behavior.Basic.Act`
(`basic.go:37-68`) reads `encounter.MonsterView` (`Self, Position, Actions,
Targeting, Seen, Remembered, Budget, Round`; no HP, AC, conditions, or
instruction of any kind) and returns a sealed `TurnIntent`: `Pass{}`,
`Attack{Target, Action}`, `Move{Path}` (`encounter/turndriver.go`, whose doc
says a fourth case "should probably earn its own ADR"). It has no encounter
handle, so **it cannot call `Route`**. The directed-movement design's §8
monster-flee bullet ("behavior asks `Route{Away …}`") is therefore not
buildable as written; see §5.

**The call chain** (`clocks.go:297 driveMonsterTurns → :321
driveTurnsWithParticipation → :~470 driveOneMonsterTurn → e.turnDriver.Act`)
passes through session's `turnDriverSeam` (`session/turndriver.go:203-232`),
which projects the view, calls the host's driver, and projects the intent
back. `driveOneMonsterTurn` loops build-view → `Act` → execute until `Pass`
or the budget is spent; a `Move` is executed per cell through `stepTo`, which
provokes through the `Mover` seam and **pauses** for a player's reaction
window (`ErrStepPaused`, resumed by `ResumeTurn`).

**Conditions land on a monster the way they land on a character.**
`monster/data.go:41-42` carries `Conditions []json.RawMessage`; the keeper
subscribes `ConditionApplied/Removed/StateChanged` (`monster/load.go:
~215-233`) plus, since Whispers, `SpendRequested` and `TurnStart`
(`load.go:238-245`). A cast's `Effects` row applies a condition to the
target through `resolution` (`contest.go:372`); Bane already does this to
monsters.

**Expiry counts the owner's turn ends on the condition.** No `CastProfile`
duration exists except `Concentration.TurnEnds` (`combat/actions/cast.go:
136-152`). Blade Ward's `{"turn_ends":2}` (`spells/cast.go:~40-52`,
`conditions/blade_ward.go:86,138,243`) is the pattern: the condition counts
`TurnEndEvent`s whose `SubjectID` is its owner. "The end of the target's
next turn" is that pattern with a count of one, and the survey found no
counterexample that needed anything else.

**The cast already carries a choice.** `CastInput.DeclarationID`
(`session/cast.go:38-47`) "is also WHICH SPELL this casts: one verb compiles
one offer per castable known entry, so the selector names the row rather
than the verb." `Afford` compiles the declarations; the client echoes one.
`CastRequest` (`service.proto:579-601`) carries `declaration_id`, `targets`,
and the aimed `cell`; nothing else.

**Route policies are closed at `Line | Away`.** `combat/actions/move.go:
22-35`; `encounter.Route`'s switch refuses anything else with
`ErrUnsupportedPolicy` (`directive.go:146`). The `MoveAway` doc notes
"'toward' is still waiting for Thorn Whip." `MovePays` is closed at
`nothing | reaction`.

**Prone exists** (`conditions/prone.go`, factory + loader rows). **Monsters
hold no equipment**: `HasShieldEquipped()` is a hard `false`
(`monster/monster.go:~199-201`), actions are definitions, not held items.
**No creature type** field exists on `monster.Data`; "undead" appears only
in constructor comments and the Undead Fortitude trait. **No language
model** exists anywhere. **Every monster in the tomb is undead** (skeleton,
zombie, skeleton captain, ghoul: `monster/monsters/registry.go`).

## 2. Ownership

| noun | owner | why there |
|---|---|---|
| the word, its list, its declaration label | `spells` (content) | the option is content the same way the damage die is; a declaration per word |
| `Commanded` condition (`word`, `caster`, one turn end) | `conditions` (rules) | a fact on the target with a clock, persisted with the sheet like every condition |
| what a word means (route policy, prone, pass) | `resolution` | the compulsion's consequences are rules; resolution DESCRIBES them as imposed effects, exactly as a cast's are |
| `TurnParticipationDriven` | `encounter` | the clock consequence of "nobody plays this turn"; the encounter owns no reason |
| deciding a member is compelled | `session` | it already turns rulebook facts into participation; it reads the sheet the encounter cannot |
| the compelled driver (compulsion first, host brain second) | `session` | the seam is session's; the host's brain stays the host's |
| `Routed` intent, `Toward` policy, walking a routed turn | `encounter` | geometry and the clock; the intent says WHAT, the encounter finds WHERE |
| the sandbox bard's fourth spell | `rpg-api` | seed |

Singularity: one compulsion per creature at a time (a second Command
replaces the first: the newer word wins, the condition is replaced not
stacked, and the log says so). One participation answer per member per
assessment, as today. One `Routed` walk per compelled turn, then the turn
ends.

## 3. The word is a declaration

`Afford` compiles **one declaration per word** for a known Command:
`Command: Approach`, `Command: Flee`, `Command: Grovel`.
Each carries the same range, target rule, save, and cost, and a distinct
`option` the profile knows. The client draws them as it draws any castable
row and echoes the chosen `declaration_id`. **No proto change. No new cast
input field.** `CastInput.DeclarationID`'s own doc is the argument: the
selector names the row, and a row is now allowed to be a spell plus an
option.

Content shape, in `combat/actions`:

```go
// CastProfile gains one field.
Options []CastOption `json:"options,omitempty"`

type CastOption struct {
    ID    string `json:"id"`    // "approach" | "flee" | "grovel" | "halt"
    Label string `json:"label"` // "Approach"
}
```

A profile with `Options` compiles one declaration per option; a profile
without compiles one, as today. `Validate` refuses an empty `ID`, a
duplicate `ID`, and `Options` on a profile that declares no consumer of the
option (today: an `Effects` row whose `Parameters` template names it; see
§4). The chosen option reaches the effect's parameters as `{"word": "flee"}`.

Zero values tell the truth: a profile with no `Options` has no option, and
every existing profile is unchanged.

## 4. The compulsion is a condition

`conditions.Commanded`:

```go
type CommandedData struct {
    MemberID string `json:"member_id"`
    CasterID string `json:"caster_id"` // the anchor Approach and Flee measure from
    Word     string `json:"word"`
    TurnEnds int    `json:"turn_ends"` // 1; counted on the OWNER's turn ends, Blade Ward's pattern
}
```

Applied by the cast's `Effects` row on a failed save (`Recipient: target`,
`Ref: refs.Conditions.Commanded()`, `Parameters` templated with the chosen
word and the caster). On a made save nothing is applied; Command has no
damage, so `OnSuccess: Negated` is the whole success branch.

It subscribes to `TurnEndTopic` and removes itself after the first turn end
whose `SubjectID` is its owner. That is "the end of the target's next turn"
without a new duration mechanism: the target's next turn end is the first
one it will see. If the target is downed before its turn, the turn is
auto-passed by life state, a `TurnEndEvent` is still announced
(`clocks.go:~424 autoPassTurn`), and the condition expires unused. Nothing
is left behind.

Replacement: applying a second `Commanded` to a member that holds one
removes the first (`ConditionRemoved`, reason "replaced") and applies the
second. Two words on one creature is not a state this design allows.

It contributes nothing to any roll. Its one reader is §5.

## 5. The compelled turn

### 5.1 Participation: `Driven`

`encounter.TurnParticipation` gains a fourth value:

```go
// TurnParticipationDriven retains the initiative slot and hands the turn
// to the TurnDriver whoever the member is. A player in Driven is not
// waited for; the composition acts for them exactly as it acts for a
// monster. The rulebook owns the reason (a compulsion); the encounter
// owns only that nobody is asked.
TurnParticipationDriven TurnParticipation = "driven"
```

`driveTurnsWithParticipation` treats `Driven` as it treats a `Wait`
monster: build the view, ask the driver, execute, regardless of `Kind`.
A `Driven` player is never returned to the human: every player verb
already refuses a non-active member, and the clock never rests on a
`Driven` slot, so no verb of theirs can land.

Session decides `Driven` in `participationNow`: a member whose sheet holds
`Commanded` and whose life state would otherwise be `Wait` is `Driven`.
`AutoPass` and `Remove` outrank it (a dying commanded fighter still dies on
schedule). The assessment is re-read at the top of every drive loop, as it
is today, so a Command cast between assessments is seen the moment the
clock reaches its target.

### 5.2 The driver: compulsion first, brain second

Session's `turnDriverSeam` becomes two layers. The outer, the **compelled
driver**, holds a lookup from `MemberID` to the member's `Commanded`
condition (the same sheet map participation reads). `Act(view)`:

1. If `view.Self` holds no `Commanded`: delegate to the host's driver,
   unchanged. A monster's brain is consulted exactly as today.
2. Otherwise ask `resolution.Obey(ObeyInput{Member, Word, CasterID})` for
   the compulsion's imposed effects (§5.3) and translate them into ONE
   terminal intent (§5.4). The host's driver is **not called**. For a
   player there is no host driver to call, and that is the point.

The compelled driver is stateless. Every word produces one terminal intent,
so `Act` is asked once per compelled turn.

### 5.3 Resolution describes the word

`resolution.Obey` is a sibling of the cast's outcome production and
produces the same vocabulary, `[]ImposedEffect`, so the words are rules and
session applies them the way it already applies a cast's:

| word | imposed effects |
|---|---|
| Approach | `ImposedMove{Policy: Toward, AnchorID: caster, Budget: the turn, Provokes: true}` |
| Flee | `ImposedMove{Policy: Away, AnchorID: caster, Budget: the turn, Provokes: true}` |
| Grovel | `ImposedCondition{Prone}` on the owner |

Every word ends the turn after its effects; that is not a fourth effect,
it is the intent's terminal nature (§5.4). "Budget: the turn" is the third
`MoveBudget` spelling, `Turn bool`, mutually exclusive with `Cells` and
`Speed`: the mover's remaining movement this turn, which the encounter
already tracks. `Pays` stays `PaysNothing`: the turn IS the price, so there
is no `PaysTurn` value and no new arm in `payForMove`.

`Obey` runs on the bus with the cast so `ImposedCondition{Prone}` is applied
by the same `imposeCondition` a cast uses, with `Commanded` as its source.
The compelled creature falls prone before its turn ends, as the text says.

### 5.4 The `Routed` intent, and it is terminal

`encounter.TurnIntent` gains its fourth case, and this is the ADR the
sealed vocabulary asked for:

```go
// Routed asks the encounter to find the path and walk it, then end the
// turn. Where Move hands the encounter a path the driver already knows,
// Routed hands it a POLICY and an ANCHOR: "toward that member, as far as
// this turn's movement reaches." The encounter routes with the same Route
// every directive uses, walks the cells one at a time through the same
// step every Move uses (so the walk provokes, pauses for a player's
// window, and resumes by ResumeTurn), and ends the turn when the walk
// stops for any reason.
//
// Terminal because its customers are: a commanded creature's turn IS the
// walk, and a fleeing monster that stops running has nothing else to do
// with the turn. A driver that wants to walk and then act hands a path to
// Move, as today.
type Routed struct {
    Policy MovePolicy // Toward | Away
    Anchor MemberID
}
```

Execution in `driveOneMonsterTurn`: `Route{Mover, Policy, Anchor, Budget:
remaining movement in cells}`; walk the path per cell through the existing
step (turn budget charged, `MoveStep.Forced` set with the cause so the OA
fold reads a forced step exactly as a directed walk's); on `ErrStepPaused`
the turn is held and `ResumeTurn` finishes the walk and the turn, exactly as
a monster's own paused walk today; on exhaustion or `StoppedBy`, end the
turn with the `"turn-ended"` beat. Moved beats carry `cause:
dnd5e:conditions:commanded`.

`Toward` is new in `encounter.Route`: shortest walking path to the cell
adjacent to the anchor, stopping there ("ending its turn if it moves within
5 feet of you"); if no path exists, walk the reachable cell with the least
ruler distance to the anchor, and if that is the mover's own cell, the walk
is empty with `StoppedBy "no path toward …"`. `Away` is reused unchanged.

Grovel needs no route: the compelled driver returns `Pass{}` after
`Obey`'s effects are applied. Halt, when added, is the same `Pass{}` with
no effects.

**Why this intent is the foundation piece.** The brain cannot call `Route`
(§1). A monster that decides to flee needs exactly `Routed{Away, threat}`,
and a monster that decides to close needs `Routed{Toward, target}` with
nothing else in the turn. This is the one tool Command leaves for the
behavior lane, and it is left because Command needs it, not because flee
might.

### 5.5 What the client sees

Nothing new is required. A compelled turn produces the beats that already
exist: `"moved"` with a cause, `"window_opened"` when a player is asked to
swing, `"turn-ended"`. The cast itself produces `"cast"` and `"saved"`. A
turn-started beat is not added here; the Whispers walk's perception finding
(toolkit#1670) is Kirk's lane and this design does not touch testimony.

The web needs the declaration rows to show their labels, which they already
do for every castable entry. If the client hides or collapses same-spell
rows, that is the one client change and it is cosmetic.

## 6. The cast

```go
Command: {
    name: "Command",
    cost: slotCost(resources.SpellSlotLevel1),
    build: func(spellSaveDC int) actions.CastProfile {
        return actions.CastProfile{
            RangeFeet:  CommandRangeFeet, // 60
            Target:     actions.CastTargetOneCreature,
            MinTargets: 1,
            MaxTargets: 1,
            Save: &saves.SaveGate{
                Abilities:  []abilities.Ability{abilities.WIS},
                DC:         saves.DCStatic(spellSaveDC),
                OnSuccess:  saves.Negated,
                Recurrence: saves.RecurrenceNone,
            },
            // Three words in the first slice (§7.4). Halt and Drop are
            // rows here when their day comes; undead and language are
            // divergences recorded in §7.1 and §7.2.
            Options: []actions.CastOption{
                {ID: "approach", Label: "Approach"},
                {ID: "flee", Label: "Flee"},
                {ID: "grovel", Label: "Grovel"},
            },
            Effects: []actions.CastEffect{{
                Recipient:  actions.CastRecipientTarget,
                Ref:        refs.Conditions.Commanded(),
                Parameters: commandedParameters, // {"word": <option>, "turn_ends": 1}; caster filled by resolution
            }},
        }
    },
},
```

A `SpellData` row (the bard requirement test reads `Level`). The bard's
level-1 pick becomes 4 of `{Bane, Thunderwave, DissonantWhispers, Command}`
(the count tracks the catalogue, rpg-toolkit#1661). The domain cleric grant
at `subclass_modifications.go:257` now points at a spell that exists.

## 7. Rulings (Kirk, 2026-09-12)

Three places the text and the engine part. Each ruled the same morning.

**7.1 Undead.** Letter: "the spell has no effect if the target is undead."
Engine: no creature type exists on `monster.Data`, and every tomb monster
is undead, so honoring the letter makes Command unwalkable. **Ruled:
diverge.** Command works on anything with a Wisdom save. Kirk: *"agree,
when we get the goblins in we can circle back."* Shelf: a creature-type
field on the monster definition and an immunity gate on the save, stocked
when goblins arrive and the tomb has something the clause can spare.

**7.2 Language.** Letter: "if it doesn't understand your language."
Engine: no language exists. **Ruled: defer, leave a note.** The note is
this paragraph and the comment on the profile; no shelf is carved because
nothing else reads a language.

**7.3 Drop.** Letter: "the target drops whatever it is holding and then
ends its turn." Engine: monsters hold nothing, a character's `UnequipItem`
moves an item to inventory not the floor; on every sandbox creature Drop
would equal Halt. **Ruled: defer, leave a note.** Kirk's forward note, kept
here because it moves an initiative: *"I have been thinking about posing
the monsters with weapons so I can see if the skele has a bow or sword, so
this pressure could move that initiative up. we will need drop separate
from equip but I would want that when I could see it. suppose I also need
props for the weapons that could be holdable and deliver the weapon to
their hand."* So Drop waits on three things, in order: a monster that
visibly HOLDS a weapon (the posing initiative), a holdable weapon prop
delivered to the hand, and a drop primitive distinct from equip that puts
the item on the floor. When the first lands, Drop is the use case that
brings the third.

**7.4 The first slice.** Kirk: *"I think flee, approach and setting them
prone is a good first slice at it."* The profile ships **Approach, Flee,
Grovel**. Halt is not in the slice; it is the one word with no effect and
no route (a `Pass`), so it is a one-row addition to `Options` whenever
wanted, and it is left out so the slice proves the three mechanisms the
words exercise (Toward, Away, an imposed condition) and no filler.

Confirmed rather than asked: the compelled turn is fully engine-driven for
players and monsters alike (Kirk, reading the shape back: *"way better than
what I was thinking"*). The human behind a commanded player is not asked
where to flee; the engine picks the cell, as it does for Whispers.

## 8. Acceptance contract

1. `Afford` for a bard who knows Command lists three declarations, one per
   word, each castable, each with the spell's range and save.
2. A cast with the Approach declaration on a ghoul that fails: the ghoul
   holds `Commanded{word: approach, caster: bard}`; the log shows the cast
   and the failed save; the ghoul has not moved.
3. On a made save nothing is applied and the log shows the save.
4. When initiative reaches the commanded ghoul, its brain is not called
   (the test driver records zero calls), it walks the shortest path toward
   the bard as far as its movement allows, stops adjacent, and its turn
   ends. Moved beats carry the cause.
5. The walk provokes: a fighter it passes gets a window; the turn holds on
   the window and `ResumeTurn` finishes the walk and ends the turn.
6. Flee walks the farthest reachable cell by ruler from the caster within
   the turn's movement, then ends the turn. A pinned target ends its turn
   where it stands with the reason recorded.
7. Grovel: the target is prone (source `Commanded`) and its turn ends.
8. The condition is gone after the compelled turn's end, and the creature
   acts normally on its following turn.
9. A commanded **player** is driven identically: their client gets no
    turn, their verbs would be refused, the beats read the same.
10. A commanded creature that is downed before its turn auto-passes by
    life state and the condition expires unused.
11. A second Command on a creature already commanded replaces the word.
12. Every existing profile compiles exactly the declarations it compiled
    before; `Options` absent means one declaration.
13. Thunderwave's and Whispers' directed walks are byte-for-byte the same
    beats as before (`Routed` and `Toward` change nothing on the cast path).

## 9. Rejected alternatives

- **Command as an instruction the brain obeys.** Every future brain must
  honor it; a player needs a second path anyway; two mechanisms for one
  spell. This design's whole thesis is the opposite.
- **A prologue from a `TurnStart` subscriber.** The bus cannot touch the
  turn, by design (§1). Making it able to would put a rules listener in
  charge of the clock.
- **The compelled turn as a directed walk (`Direct`) at the crossing, then
  `AutoPass`.** `Direct` charges no turn budget and needs no active turn;
  here the turn IS the budget and the mover IS active. The driven-turn path
  already walks, provokes, pauses and resumes for a member nobody plays.
  `Direct` is for the caster's turn; this is the target's.
- **A `PaysTurn` price.** The turn is not a price paid before a walk; it is
  the budget the walk spends. `Budget: Turn` says it; `MovePays` stays
  closed.
- **The word as a new cast input and proto field.** The declaration already
  carries a choice; a second channel for the same decision is the
  disagreement `CastInput`'s doc warns against.
- **A `Compelled` participation that still consults the brain.** Then the
  brain must know to return the compelled intent, which is the first bullet
  again.
- **Precomputing "toward" and "away" paths on `MonsterView`.** The view
  would carry geometry for words it does not know were spoken, for every
  member, every turn. `Routed` asks for exactly one route when one is
  needed.
- **Honoring undead immunity now.** Unwalkable in the only sandbox; see
  §7.1.
- **Expiry anchored to a round or the caster's clock.** Blade Ward's
  comment records the round trap; the caster's clock is Bane's because Bane
  is the caster's concentration. Command's clock is the target's, and the
  owner's turn end is the one boundary this stack already counts.

## 10. Sequence, one module per PR, merged bottom-up

1. **dnd5e root** (`combat/actions`, `conditions`, `refs`, `spells`):
   `CastOption` + `Options` + validation; `Commanded` condition with
   loader, factory, display, replacement; `Toward` in `MovePolicy`; `Turn`
   budget on `CastMove`; the Command profile, constants, data row; the
   bard's 4 of 4; the cleric grant now resolves.
2. **encounter**: `TurnParticipationDriven`; `Routed` intent; `Toward` in
   `Route`; walking a routed turn to its end, with hold and resume.
3. **resolution**: `Obey`; `Turn` on `MoveDirective`; `Toward` accepted
   where `Away` is.
4. **session**: `Afford` compiles one declaration per option and threads
   the word into the effect's parameters; `participationNow` answers
   `Driven`; the compelled driver wraps the host's; `Routed` and `Toward`
   cross the seam.
5. **rpg-api**: the sandbox bard knows Command. No proto change.
6. **rpg-dnd5e-web**: nothing required; cosmetic grouping of same-spell
   rows if wanted.

## 11. Where the evidence is thin

- The PHB text is recalled, not cited; the words and their clauses are
  from memory and labelled so in the survey.
- `driveOneMonsterTurn`'s intent-execution switch was read through its doc
  and callers, not line by line; the `Routed` execution in §5.4 assumes the
  per-cell step it calls for `Move` is reusable with a cause.
- How `Afford` compiles declarations and labels them was read from
  `CastInput`'s doc, not from `afford.go`; §3 assumes one row per castable
  entry can become one row per option without a shape change on the offer.
- Whether the web collapses or lists same-spell declarations is unsurveyed.
- The `Effects` parameter templating (a per-cast value reaching a
  condition's parameters) has no precedent; every shipped `Parameters` is a
  literal. §3's validation rule is the design's, not the code's.
