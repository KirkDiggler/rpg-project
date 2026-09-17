# The roll is seen — advantage and disadvantage carry both faces

**Status:** DESIGN, panel-back (tracking rpg-project#462). Follows the front room
goblin (`ideas/shenanigans/front-room-goblin.md`), whose untrained rule shipped
applied but invisible.

## Kirk's ruling, 2026-09-17

> "The disadvantage rolls should be a first class thing in our system. Dice
> rolled should know to roll one or two and what the individual results were."

And the standing law from the same wave: everything is visible in the log, the
debug log for sure.

## The break this closes

Three machines roll a d20. Two of them keep both faces; one throws the second
away; none of them says **why** a face was kept.

| Machine | Rolls the pair | Keeps both faces in the trace | Says who granted or imposed it | Records a cancellation |
|---|---|---|---|---|
| attack (`resolution/strike.go` `rollAttackD20`) | yes | yes | parallel lists on `StrikeOutcome.Folded`, refs only, attacks only | no |
| save (`saves/saves.go` `rollD20`) | yes | yes | `SavingThrowResult.AdvantageSources`, dropped at the encounter seam | no |
| check (`checks/checks.go`) | yes | **no** — returns one `Roll int` | `AbilityCheckResult.DisadvantageSources`, dropped at `session/conceal.go` | no |

The check machine's trace is faked downstream. `resolution/check.go`'s own
comment says it: *"OriginalRolls/FinalRolls both carry that one settled face
rather than the pair the rules package actually rolled."* Session then drops
even that at `encounter.ResolveCheckOutput{Beaten, Applied, Total}`, so
`Intimidated`, `Persuaded` and `DoorChanged` reach the wire as
`{dc, total, beaten}` and the log prints one number.

Cancellation is unrepresentable everywhere. All three compute
`effective := has && !hasOther`, roll one die, and the trace is
indistinguishable from a straight roll. A player with Bless who rolled untrained
sees a plain d20 and never learns the two rules met.

**Why this is a primitive, not a patch.** Every d20 the game will ever roll —
attack, save, check, initiative, death save, contest — takes advantage and
disadvantage from a growing list of sources (Elven Accuracy rolls three and
keeps one; the Lucky feat rerolls; Bardic Inspiration adds a die). If the
record of *what was thrown and why one face counted* lives in three
hand-written switches, the fourth machine will write a fourth, and the log will
lie in a fourth way. One d20 roller, one trace shape, one keep record.

## What this slice adds (the tool)

1. **One d20 roller.** `rolls.RollD20` in `rulebooks/dnd5e/rolls` (the
   package that already resolves `DiceContribution` pools) replaces
   `saves.rollD20`, `resolution.rollAttackD20` and the inline switch in
   `checks.MakeAbilityCheck`. Input: the roller, the granted sources, the
   imposed sources. Output: the kept face and a `DiceTrace` with every face,
   the kept index, and the keep record below. It is the only place that knows
   advantage means two dice.
2. **The keep record on the trace.** `DiceTrace` gains `Keep *DiceKeep`, the
   sibling of `Rerolls`: `Rerolls` explains why `FinalRolls` differ from
   `OriginalRolls`; `Keep` explains why `KeptIndices` is what it is.
3. **The check machine returns what it rolled.** `AbilityCheckResult` gains
   `Calculation`, built in the rules package the way saves already do. The
   fake in `resolution/check.go` is deleted.
4. **The seam carries it.** `encounter.ResolveCheckOutput` gains
   `Calculation`; the Intimidate, Persuade and Unlock beats carry it; the
   paused check window carries it. Session bodies, wire, api and web follow.

## The shape

### `DiceKeep` (events package, rule-owned; mirrored in encounter and session)

```go
// DiceKeep records the rule that decided KeptIndices, and who brought it.
// Nil when nothing touched the pool: a straight roll keeps every face.
type DiceKeep struct {
	// Rule names the keep rule that was applied, or "cancelled" when granted
	// and imposed sources met and the pool was rolled straight.
	Rule    KeepRule     // "advantage" | "disadvantage" | "cancelled"
	Granted []RollSource // sources that granted advantage
	Imposed []RollSource // sources that imposed disadvantage
}
```

Facts the shape states, and the tests that pin them:

- **Straight roll → `Keep == nil`, notation `1d20`, no kept indices.** Zero
  value tells the truth: nobody touched the pool.
- **Advantage → `2d20`, `KeptIndices` has one entry, the kept face is the
  max, `Granted` non-empty, `Imposed` empty.** Disadvantage is the mirror with
  min.
- **Cancelled → `1d20`, no kept indices, both lists non-empty.** The trace
  looks like a straight roll *except* the record says why. This is the case
  the log has never been able to show.
- `ValidateRollCalculation` refuses every other combination (a `Keep` with
  advantage on a one-die pool, a kept face that is not the max, a cancelled
  record with one list empty). Fail closed: a builder that fills the record by
  hand and gets it wrong is refused at the seam, not rendered wrong.

`RollSource` is the existing sourced-fact type (`Ref, Name, Label, SourceID`).
`CheckModifierSource` and `SaveModifierSource` map onto it one to one
(`SourceRef→Ref`, `Name→Name`, `SourceType→Label`, `EntityID→SourceID`).
The untrained rule already publishes `Name: "Untrained"` with the rules ref, so
the word the log prints comes down from the server. The web never invents it.

### The parallel lists go

`Struck.advantage_sources` / `disadvantage_sources` (`AttackModifierSource`,
refs and ids only) are the older, narrower spelling of what `Keep` carries.
Keeping both would let the two disagree. Toolkit: `StruckBody` loses them
(no backcompat baggage). Wire: `[deprecated = true]`, never filled. Web: the
`[adv: …]` debug rendering reads `Keep` instead.

### Protos (additive)

```proto
message DiceKeep {
  KeepRule rule = 1;
  repeated RollSource granted = 2;
  repeated RollSource imposed = 3;
}
enum KeepRule { KEEP_RULE_UNSPECIFIED = 0; KEEP_RULE_ADVANTAGE = 1;
                KEEP_RULE_DISADVANTAGE = 2; KEEP_RULE_CANCELLED = 3; }

// DiceTrace
DiceKeep keep = 8;
// Intimidated, Persuaded
RollCalculation calculation = 6;
// DoorChanged
RollCalculation calculation = 7;
// Struck
repeated AttackModifierSource advantage_sources = 10 [deprecated = true];
repeated AttackModifierSource disadvantage_sources = 11 [deprecated = true];
```

`RollWindowOpened.calculation = 5` already exists on the wire and has never
been filled, because `session.RollWindowOpenedBody` has no field for it. This
slice gives the body the field and fills it: the paused Intimidate window is
where an untrained roll is *first* seen, and today it shows one face.

`Saved` already carries a calculation; it gains the keep record through the
trace with no wire change. `UnlockResponse` and `IntimidateResponse` do not
gain it — the outcome lives only in the event (Search precedent).

### Web

The log line, story and debug, reads the trace and names the rule:

```
2d20 [7, 18] kept 7 · disadvantage: Untrained
2d20 [7, 18] kept 18 · advantage: Reckless Attack
1d20 [11] · advantage (Bless) cancelled by disadvantage (Untrained)
```

The discarded face is drawn struck through. `formatDice`'s
`(kept indices [1])` text is replaced, not kept beside. Drawing two physical
d20s in the tray is the follow-up: the multi-die group shape already exists in
`src/components/ui/dice/diceRollGroup.ts` with `counted | discarded`, and the
adapter from a live `RollCalculation` to it is its own web slice.

## Rulings taken in this note (break them if wrong)

- **R1 — the keep record lives on the trace, not beside it.** A parallel list
  can disagree with the dice it describes. `Keep` beside `KeptIndices` cannot.
- **R2 — cancellation is a recorded rule, not an absence.** RAW rolls one die;
  we roll one die and say why. A log that cannot show "your Bless was eaten by
  untrained" hides the one thing the player needed to learn.
- **R3 — checks build their calculation in the rules package, like saves.**
  Attacks build theirs in resolution because offers rewrite it after the roll.
  Checks pose too (Guidance), and the posed die is appended in
  `check_pose.go` exactly as today. Two of three machines now agree; the third
  is left named, not fixed.
- **R4 — Unlock's `DoorChanged` is a check beat and carries the calculation.**
  "Any future check beat" in #462 includes the one that already exists.
  Search reveals a door per viewer and appends no numbers; it stays out.
- **R5 — the paused window is in scope.** Same trace, one more carrier; the
  proto field is already there.
- **R6 — the tray's second physical die is the follow-up, not this slice.**
  #462's done-when is the log.

## Not in this slice

- Elven Accuracy (three dice, keep one) and reroll feats. The shape holds
  (`3d20`, one kept index, `Rule: advantage`; rerolls already have a home).
  No source brings them yet.
- Initiative and death saves keep their straight `1d20` traces. When a source
  grants advantage on them, they call `rolls.RollD20` like everyone else.
- The `approachModifier` blindness to `Untrained` named in `check.go`
  (picks an untrained +3 over a trained +2). No shipped check lists both.
- A "what I know" projection. Unchanged from the front room ruling.

## Build order

Design panel-back → protos (Kirk merges) → four toolkit PRs on
pseudo-versions → api → web, walked once, merged inside-out.

| PR | Module | What |
|---|---|---|
| toolkit 1 | rulebooks/dnd5e | `DiceKeep`, `KeepRule`, `rolls.RollD20`, validation; checks and saves call it; `AbilityCheckResult.Calculation` |
| toolkit 2 | resolution | strike calls `rolls.RollD20`, `rollAttackD20` deleted, `checkCalculationFor` deleted, `StrikeOutcome` sources come from the trace |
| toolkit 3 | encounter | mirror `DiceKeep`; `ResolveCheckOutput.Calculation`; Intimidate/Persuade/Unlock inputs and beat payloads carry it; `validateRecordedD20` checks the keep record |
| toolkit 4 | session | bodies `IntimidatedBody`/`PersuadedBody`/`DoorBody`/`RollWindowOpenedBody` gain `Calculation`; `StruckBody` loses the lists; decoders |
| protos | v0.1.198 | the additive block above |
| api | dev | fill four conversions; drop the deprecated lists |
| web | dev | the log line; debug feed reads `Keep` |

**Unit tests, not walk samples:** the four trace facts above, cancellation,
validation refusals, and the mapping from `CheckModifierSource` to
`RollSource`. The walk is one click per seam.

**Done when** (the walk): an untrained character's Intimidate shows two d20
faces, the kept one, and the word "Untrained" in the story and debug logs; a
trained character's shows one face and no rule; the paused window shows the
same; a Dodging save shows two faces and "Dodging".

## Next

The world clock advances as the party moves and creatures are driven on it
(front room goblin, Next §2). Then the tray draws the second die.
