# The roll is seen — advantage and disadvantage carry both faces

**Status:** SHIPPED 2026-09-17 (tracking rpg-project#462). Walked by Kirk: the
untrained Intimidate shows two faces and the kept one on every surface. Follows
the front room goblin (`ideas/shenanigans/front-room-goblin.md`), whose
untrained rule shipped applied but invisible.

| Repo | Release |
|---|---|
| rpg-api-protos | v0.1.199 (#344: `KeepRule`, `DiceKeep`, `DiceTrace.keep`, `calculation` on `Intimidated`/`Persuaded`/`DoorChanged`, `Struck` source lists deprecated) |
| rpg-toolkit | rulebooks/dnd5e v0.180.0 (#1806) · encounter v0.88.0 (#1807) · resolution v0.53.0 (#1808) · session v0.95.0 (+ #1810 pin-only, tags in go.mod) (#1809) |
| rpg-api | dev 98b6d753 (#1002) |
| rpg-dnd5e-web | dev 560e4115 (#1110) |

## What the build corrected (kept visible)

- **The build order table below was wrong.** Encounter depends on neither
  resolution nor dnd5e; that is what its mirror types are for. The real graph
  is dnd5e and encounter first, then resolution, then session. Two stale pins
  surfaced on the way (resolution seven encounter minors behind main).
- **The offered die had no granter.** R7 says a contributed die is its
  granter's, but the offer carried only who was *asked*. `Offer.SourceID` was
  added and filled from the granter each condition already records.
- **The advantage input flags are gone.** No non-test caller ever set
  `HasAdvantage`/`HasDisadvantage`; a boolean brings neither a ref nor an
  entity, so R7 could not record it. Advantage arrives only on the chain.
- **Encounter's mirror refuses an anonymous pool after all.** The builder
  reverted that check as "structural by charter, no rulebook to judge from",
  and I accepted it. The independent review overturned both of us: `SourceID`
  presence is data provenance, not 5e eligibility, the mirror already
  enforces provenance on subtractive dice and on keep sources, and session's
  persisted-JSON decoder relies on this validator, so an anonymous pool would
  have survived persistence and reached the log. Fixed on the review round.
- **Calculation carriers are optional on the types, strict at the producer.**
  Intimidate, Persuade and Unlock refuse a nil calculation (`ErrNoCalculation`)
  on both the direct and the resumed-pose path; a verb that rolled nothing
  never calls the rule. rpg-api maps it to Internal: a producer defect, not a
  precondition a player can act on.
- **"Drawn struck through" cannot happen in the log.** The formatter returns a
  plain string three text surfaces print. The line says `kept 18`; the
  discarded die's look belongs to the tray slice. Kirk: the debug log is JSON
  and its formatting does not matter.
- **The old combat log stays untouched.** It renders the dead encounter view's
  v1alpha2 events, which have no calculation. The live debug feed was the
  target, and its trace renderer now prints the keep wherever a trace is
  printed, instead of a second renderer keyed off the beat.
- **Every d20 now names its roller, and the web's "worth attributing" test
  was "names an entity".** Left alone, a check line ends with the roller's own
  name, as saves and concentration lines already did. Not changed in this
  slice; ruling open below.
- **Walk finding that was not a bug.** Kirk expected an arrival on a failed
  check; the front room table brings the bandits only on a failed Persuade
  (the false fact), and both persuades landed. The journals proved the answer
  and arrival path live. The author's table is the tool, and it is
  configurable there.

## Open after shipping

- Whether a die is named on the line only when its entity differs from the
  beat's actor (recommended), so "Bless (from Alice)" shows and your own d20
  does not. Four lines and three expectations in web.
- The tray draws the second physical die from the keep record.
- A builder palette for `intimidate:` and `on:` (carried from the front room).

## Kirk's ruling, 2026-09-17

> "The disadvantage rolls should be a first class thing in our system. Dice
> rolled should know to roll one or two and what the individual results were."

And the standing law from the same wave: everything is visible in the log, the
debug log for sure.

On the panel, the same day, Kirk widened it past the d20:

> "This isn't just about the d20 roll. We may have better insight into the
> damage and other rolls. I know Bardic Inspiration is a d6. Long term, players
> will have the appearance they are rolling all the dice. Knowing the source of
> the dice will be important: if you are using my Bardic Inspiration or
> someone's Bless, you use their dice style from their set. While a d20 function
> is good for this slice, we will want the additions later to fit in with our
> choices here."

## Every die knows whose it is (the invariant this slice must honor)

The trace shape already carries entity provenance: `RollSource.SourceID` is
"the responsible character or entity when this contribution has entity
provenance", and the contributed dice fill it today — Bless's d4 names the
caster, Bardic Inspiration's d6 names the bard, Guidance's d4 names the caster,
Help's advantage names the helper. A tray that draws each die in its owner's
set has the data for every die **except the one rolled most: the d20 is
anonymous.** Attacks source it to the weapon definition, saves to the rule
ref, checks to the approach, and none of them names the roller. A client that
wants the roller's d20 in the roller's style has to guess it from the beat's
actor, which is the client calculating.

The rule, stated so later additions fit:

- **Every dice pool on a calculation names the entity whose rule threw it.**
  The d20 is the roller's (`SourceID` = actor). A contributed die is its
  granter's. A damage die is the wielder's, a monster trait's die the
  monster's. `ValidateRollCalculation` refuses a dice component with an empty
  `SourceID`. A roll with no entity behind it does not exist in this game.
- **Whose die and whose rule are different facts and both are kept.** `Ref`
  and `Name` say the rule (Bless, Longsword, Untrained); `SourceID` says the
  entity. The bard's d6 is `Ref: Bardic Inspiration, SourceID: bard`.
- **The web picks a dice style by `SourceID`, never by the beat's actor.**
  This slice only prints; the tray slice draws. The fact is on the wire from
  this slice on.
- The three Go spellings of the trace (`events` for rules, `encounter` for
  persistence, `session` for the seam) stay. They are the seams doing their
  job, not duplication to clean.

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
indistinguishable from a straight roll. A player who was Helped and rolled
untrained sees a plain d20 and never learns the two rules met.

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
   advantage means two dice. It takes the roller's id and writes it as the
   d20's `SourceID`: the d20 stops being anonymous.
   **`RollD20` is one pool, not the roll.** The roll is the
   `RollCalculation`, which attacks, saves, death saves, damage and healing
   already build and carry to the log today; only checks lack a real one.
   `RollD20` fills its first component (the operation's d20, the contract
   `validateRecordedD20` already checks) and nothing else. Every die added to
   the roll keeps the door it has now: Bless's d4 is described before the roll
   and rolled beside the d20 by `rolls.ResolveContributions`; Bardic
   Inspiration and Guidance freeze the settled calculation, ask the player,
   and land as a new component on resume. Each is its own component with its
   own source. Advantage never touches those pools; `Keep` lives on the d20's
   trace only. A later effect that adds a die adds a component through one of
   those doors; it never changes `RollD20`. (Kirk on the panel: "the RollD20
   function could have additional dice in the case of being blessed or other
   effects." The calculation has them; the roller does not.)
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
1d20 [11] · advantage (Help, from Alice) cancelled by disadvantage (Untrained)
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
  we roll one die and say why. A log that cannot show "Alice's Help was eaten
  by untrained" hides the one thing the player needed to learn.
- **R7 — every dice pool names its entity, and validation refuses one that
  does not.** The d20 gets the roller's id from `rolls.RollD20`. The builder
  of the dnd5e PR lists every `DiceTrace` constructor it touched and which
  entity each now names; a damage or trait die found anonymous is fixed in
  that PR, not left for the tray slice to discover.
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

## Corrected on the panel (kept visible)

- **"Bless cancelled by Untrained" was wrong.** The first draft used Bless as
  the advantage in the cancellation example. Bless adds a d4 to the roll; it
  grants no advantage. Help does. The example and R2 now say Help. The rule
  shape was not affected, the illustration was, and a wrong illustration in a
  design is how a builder ships a wrong test.

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
| toolkit 1 | rulebooks/dnd5e | `DiceKeep`, `KeepRule`, `rolls.RollD20` (writes the roller's `SourceID`), validation incl. anonymous-pool refusal; checks and saves call it; `AbilityCheckResult.Calculation`; every anonymous `DiceTrace` constructor named |
| toolkit 2 | resolution | strike calls `rolls.RollD20`, `rollAttackD20` deleted, `checkCalculationFor` deleted, `StrikeOutcome` sources come from the trace |
| toolkit 3 | encounter | mirror `DiceKeep`; `ResolveCheckOutput.Calculation`; Intimidate/Persuade/Unlock inputs and beat payloads carry it; `validateRecordedD20` checks the keep record |
| toolkit 4 | session | bodies `IntimidatedBody`/`PersuadedBody`/`DoorBody`/`RollWindowOpenedBody` gain `Calculation`; `StruckBody` loses the lists; decoders |
| protos | v0.1.198 | the additive block above |
| api | dev | fill four conversions; drop the deprecated lists |
| web | dev | the log line; debug feed reads `Keep` |

**Unit tests, not walk samples:** the four trace facts above, cancellation,
validation refusals (an anonymous dice pool included), the d20's `SourceID`
being the roller on attack, save and check, and the mapping from
`CheckModifierSource` to `RollSource`. The walk is one click per seam.

**Done when** (the walk): an untrained character's Intimidate shows two d20
faces, the kept one, and the word "Untrained" in the story and debug logs; a
trained character's shows one face and no rule; the paused window shows the
same; a Dodging save shows two faces and "Dodging".

## Next

The world clock advances as the party moves and creatures are driven on it
(front room goblin, Next §2). Then the tray draws the second die.
