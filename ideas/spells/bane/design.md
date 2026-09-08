# Bane — one declaration, three targets, and a die the machine rolls

**Date:** 2026-09-08
**Status:** Design proposed. Slice four of the bard/spells initiative. **Open rulings R1–R7
below need Kirk before any issue is filed.**
**Umbrella:** `ideas/spells/` — levels 1–3, and the choices at 3 made real.
**Brainstorm:** rpg-project#391 (`ideas/spells/brainstorm.md`) §6.2 — the row **Fan-out**,
marked *missing*, and the line in §6.3 that says the held die is *"a condition on the bus"*.
**Slice three:** rpg-project#407 (`ideas/spells/concentration/design.md`), shipped 2026-09-08
as dnd5e v0.149.0 / encounter v0.68.0 / resolution v0.37.0 / session v0.69.0, protos v0.1.178,
rpg-api#951, web#1000.

Kirk's rulings that chose this slice:

- **Bane, not Thunderwave.** Kirk: *"no bane sounds way better."* Thunderwave is three new
  shapes stacked — an area selector, half-damage-on-success, and forced movement — and this
  stack has just spent a slice learning what happens when three arrive together. Bane is the
  same fan-out with the simplest selector: a range check and a chosen list.
- **Lay the cleric's foundation while building the bard.** Bane is a bard spell, so the slice
  is bard progress; it is also Bless's mirror image, so the cleric's headline spell arrives
  next for almost nothing.
- **Spatial waits for the slice that needs it.** Bane names creatures. Nothing here queries a
  shape, and `tools/spatial` gains no caller. Area templates arrive with Thunderwave.

---

## What this slice is

**Bane (2014):** 1st-level enchantment, range 30 feet, concentration up to 1 minute. The caster
chooses **up to three creatures within range**. Each makes a **Charisma saving throw**. On a
failed save, the creature **subtracts a d4 from every attack roll and every saving throw** it
makes for the duration.

Three things the stack has never done, and one it has done only in tests:

| | Shape | State today |
|---|---|---|
| 1 | **Fan-out** — one declaration, N targets, N outcomes, one record | **missing**; every machine input holds one target |
| 2 | **A described die** — an effect says *"subtract 1d4"* and the machine rolls it | **missing**; both pre-roll chains carry settled integers |
| 3 | **Save-then-effect, per target** — CHA save, negate on success | **exists**, shipped with Vicious Mockery |
| 4 | **N children under one concentration owner** | **exists, unexercised** — two children appear in unit tests only, never from a spell |

Everything in row 3 and row 4 is a machine doing what it already does with one more input. Rows
1 and 2 are the design.

---

## Shape 1 — Fan-out: the target is a list, everywhere

Today a cast has exactly two parties, and the code says so on purpose:

> `CastOutcome.TargetID` — *"CasterID and TargetID are the cast's two parties. TargetID is EMPTY
> for a self-targeted profile, which is the one spelling: a caster repeated into both fields
> would be a second way to say the same thing."*
> — `rulebooks/dnd5e/resolution/action.go:130-136`

That sentence is the reason fan-out cannot be bolted on. `CastTargetRule` is
`self | one_creature` (`combat/actions/cast.go:20-29`); `RecordCastInput` carries one `Target`,
one `Save`, and one `Results` list (`encounter/cast.go:194-224`). A second, parallel
"multi-target" spelling beside the singular one would be exactly the two-ways-to-say-it that
this type's own doc refuses.

**So the singular becomes the list, and a one-target cast is a list of one.**

```
                    one declaration
                          │
       CastProfile{ Target: up_to_n, MaxTargets: 3, Save: CHA gate, ... }
                          │
        preflight: every named target distinct, in range, in the interaction
                          │           (refused BEFORE the door — see R5)
                          ▼
        ┌───────── per target, in the order the caster named them ─────────┐
        │   Request(contest)  →  save rolled  →  negate or deliver         │
        └──────────────────────────────────────────────────────────────────┘
                          │
             CastOutcome{ CasterID, Targets: [ {TargetID, Save, Applied}, … ] }
                          │
             RecordCastInput{ Actor, Spell, Targets: [ … ] }
                          │
        one cast beat naming N targets, then per target: saved beat, then results
```

**Why the loop is the contest again and not a new machine.** The brainstorm called fan-out a new
machine, and it is the smallest one on the list: a loop over a target source that yields
`Request(sub-machine)` once per target and collects the outcomes. Movement already does exactly
this — `Request` per trigger inside one machine (`resolution/movement.go:313`). The cast machine
gains the loop; the contest it requests is untouched.

**Names this proposes** (`resolution`):

```go
// CastTargetOutcome is what one cast did to ONE creature it named.
type CastTargetOutcome struct {
    TargetID string
    Save     *ContestOutcome  // nil when the profile carried no gate
    Applied  []ImposedEffect  // empty when the save was made
}

type CastOutcome struct {
    Spell     core.Ref
    CasterID  string
    Targets   []CastTargetOutcome  // replaces TargetID, Save, Applied
    FollowUps []FollowUpOutcome
}
```

Inside the toolkit this is a free refactor: rpg-api pins tags, so a shape change costs nothing
until it is adopted. The call sites that move are True Strike's and Vicious Mockery's, which
become one-element lists and read the same.

**The fork, named and rejected.** The band-aid is a second outcome type for multi-target casts,
leaving `CastOutcome` alone. It is smaller today and it forks every consumer of a cast — the
record, the wire, the client — for a difference the player never sees, which is the fork this
outcome type was sealed to prevent. The primitive is one list.

---

## Shape 2 — The described die, and why it is not a number

Bane subtracts a d4. Bless adds one. Neither is a choice, neither is spent, and both land on a
roll **somebody else** makes.

Both pre-roll chains are live and conditions already subscribe to them:

| Chain | Carries | Live subscribers |
|---|---|---|
| `AttackChain` | advantage/disadvantage sources, `AttackBonus int`, crit threshold | Improved Critical, Helped, Pack Tactics |
| `SavingThrowChain` | advantage/disadvantage sources, `BonusSources []SaveBonusSource{Bonus int}` | Dodging |

**The two chains disagree about sourcing, and that is the first thing to fix.** A save's bonus
is a *list of sources* — `BonusSources []SaveBonusSource{Name, SourceRef, EntityID, Bonus}`
(`events/events.go:436-449`) — so a save can say *who* changed it. An attack's bonus is a bare
scalar, `AttackBonus int` (`events/events.go:311`), and the codebase has already been annoyed by
this: `conditions/inspired.go:44-54` names that scalar as the thing it refused to write into.
Bane needs to say "− 3, Bane" on an attack roll, and today the attack chain has nowhere to say
the second half.

**And a condition could roll the die itself.** This is the real fork, not a strawman:
`conditions/roller_binding.go:31` `RollerBinder.BindRoller` exists and three conditions use it
(`sneak_attack.go:239`, `brutal_critical.go:200`,
`fighting_style_great_weapon_fighting.go:192`). So the small version of Bane is available today:
the condition binds a roller, rolls a d4 in its own chain handler, and appends
`SaveBonusSource{Bonus: -face}` on the save side and `AttackBonus -= face` on the attack side.
**The save half of Bane would need no root change at all.**

**The fork, stated plainly.**

- **The special case above:** Bane rolls its own d4 and hands over a number. Ships sooner. The
  attack roll shows a total nobody can explain, because the scalar carries no source. Every
  future die-lending effect — Bless, Guidance, Cutting Words, Bardic Inspiration's automatic
  cousins — repeats the roll-and-flatten, and each one has to remember not to cache its face.
- **The primitive below:** the chains learn to carry a **die notation**, and the machine that
  rolls the d20 rolls it too. One type serves both chains, both signs, and every later effect.
  Freshness stops being a discipline: a subscriber that cannot roll cannot cache a stale face.

*Recommend the primitive*, and the codebase has already made this exact call once, in the one
place it faced the question:

> *"`Die` is the notation of what would be rolled and added — "1d6". A notation rather than a
> number because nothing has been rolled: whoever takes the offer rolls it, with their own
> roller."*
> — `rulebooks/dnd5e/events/offer.go:34-38`

`Offer` is the **post-roll, spendable, chooseable** lane, built for Bardic Inspiration, and its
own doc draws the line at exactly the case Bane sits on the other side of: *"A subscriber to
[AttackChain] writes a number into a roll nobody has seen yet"* (`offer.go:12-15`). Bane belongs
in that pre-roll lane, and what the lane is missing is the notation.

**One type, both chains:**

```go
// DieSource is a die an effect DESCRIBES and the rolling machine ROLLS.
//
// A notation rather than a number, for the same reason an Offer carries one:
// nothing has been rolled. The machine that rolls the d20 rolls this too, in
// the same breath, and reports the face — so a fresh die every roll is
// structural rather than a thing each subscriber has to remember.
type DieSource struct {
    Name      string     // "Bane" — display name, authored, never derived
    SourceRef *core.Ref  // dnd5e:conditions:baned
    EntityID  string     // who imposed it — the caster
    Die       string     // "1d4"
    Subtract  bool       // Bane subtracts; the zero value adds, which is Bless
}

// DieResult is that die after the machine rolled it, reported so the client can
// show the d4 that changed the number.
type DieResult struct {
    Name      string
    SourceRef *core.Ref
    Die       string
    Face      int
    Subtract  bool
}
```

`AttackChainEvent` and `SavingThrowChainEvent` each gain `DieSources []DieSource`; the attack
fold (`resolution/strike.go:362`) and `saves.MakeSavingThrow` (`saves/saves.go:160-216`) each
roll them with the roller they already hold, fold the faces into the total they already compute,
and report `DieResults` on their result.

**The subscription itself has a shipped precedent.** A condition that sits on one creature and
reaches another creature's roll is `conditions/vicious_mockery.go:188-209`: it lives on the
target, filters on `AttackerID`, and appends to the attack chain at `combat.StageConditions`.
Bane's condition is that subscription with a die instead of disadvantage, and without the
self-consume.

**Bless is then this design with one bool flipped and the save removed.** That is the whole
reason Bane goes first.

---

## Shape 3 and 4 — what is already built

**The save.** `CastProfile.Save` is a `saves.SaveGate`, negate-on-success only, contested before
anything is delivered — shipped with Vicious Mockery. Bane's gate is CHA against the caster's
spell save DC, which is computed (`character/character.go:1641`) and already wired into cast
definitions (`session/casts.go:104`). Nothing new.

**The concentration.** `CastProfile.Concentration{TurnEnds: 10}` puts the concentrating
condition on the caster, and every `baned` condition placed becomes a child address under it.
When the bard's concentration breaks, all three come off together. Slice three built this and
tested **two** children by hand (`conditions/concentrating_test.go:174-175`, `:262-263`); the
only production caller adds one child per delivered condition from a single-target cast
(`resolution/action.go:264`), so today the ceiling is one. Bane is the first spell that produces
three, which makes this slice the multi-child owner's first real exercise.

**Range.** `CastProfile.RangeFeet: 30` exists, and distance is the grid's own
(`encounter/encounter.go:1038`). No new geometry.

---

## What the streamer sees

The story, not the log. Six cards for a three-target cast where one holds:

```
  the bard casts Bane on the skeleton, the goblin and the wolf
  the skeleton fights the words off          d20 15 + 2 = 17 against DC 13 · Saved
  Bane takes hold of the goblin              d20 4 + 1 = 5 against DC 13 · Failed
  Bane takes hold of the wolf                d20 8 + 0 = 8 against DC 13 · Failed
  ...
  the goblin swings at the bard              d20 14 + 4 − 3 (Bane) = 15  · misses
```

That last line is the whole point of shape 2, and it is why the die's face travels to the
client rather than being folded away into a bonus nobody can see. It is the same thing Kirk
asked for on the concentration design: *the d4 is in your hand.*

---

## Module cut

| # | Module | What it adds | Depends on |
|---|---|---|---|
| 1 | **protos** (additive) | `repeated string targets` on `CastRequest` and on the cast beat body, singular `target` `[deprecated = true]` in both; a `RollModifier{name, ref, die, face, subtract}` message, `repeated` on the saved beat and the attack beat | — |
| 2 | **toolkit root** | `DieSource` / `DieResult`; `DieSources` on both chain events; the fold + roll in `saves.MakeSavingThrow`; `CastTargetUpToN` + `CastProfile.MaxTargets`; `conditions.NewBanedCondition`; `refs.Conditions.Baned()`; Bane content | — |
| 3 | **toolkit resolution** | the fan-out loop in the cast machine; `CastTargetOutcome`; the attack-roll die fold in `strike.go` | 2 |
| 4 | **toolkit encounter** | `RecordCastInput.Targets`; one cast beat naming N, then per-target saved + result beats; roll modifiers on the beats | 2 |
| 5 | **toolkit session** | the per-target outcomes and the modifier traces onto the wire | 3, 4 |
| 6 | **rpg-api** | pass-through; the declaration carries a target list | 1, 5 |
| 7 | **rpg-dnd5e-web** | the target picker takes up to three; the story cards above; the die on the roll line | 1, 6 |

Modules 2 is one PR, 3 and 4 are parallel, 5 gates on both. Same train as slice three.

## Done-when Kirk walks it

1. A level-1 bard casts Bane and names three creatures; the picker refuses a fourth and refuses
   anything past 30 feet.
2. Three saves are rolled and shown, each with its own roll, total and DC.
3. A creature that saved is unaffected and says so on a card.
4. An affected creature's next attack roll visibly shows `− N (Bane)` and the d4's face.
5. An affected creature's next saving throw shows the same.
6. Breaking the bard's concentration removes Bane from **all** affected creatures in one beat
   run, and their rolls go back to normal.

---

## Open rulings

**R1 — one cast beat naming N targets, or N cast beats?**
*Recommend one.* The declaration happened once and the player made one choice; N cast beats
would make the record say the bard cast Bane three times. The per-target detail lives in the
saved and result beats that follow.

**R2 — is a made save a beat?**
*Recommend yes.* "The skeleton fights the words off" is story, and a target that silently
disappears from the record reads as a bug. It costs one saved beat with `succeeded: true`,
which the wire already carries.

**R3 — `Subtract bool`, or a signed notation like `"-1d4"`?**
*Recommend the bool.* A signed notation has to be parsed to be understood, and the zero value
of the bool is *adds*, which is Bless — the common case says nothing and the unusual one
declares itself.

**R4 — `MaxTargets` on the profile, or fixed in content?**
*Recommend the profile.* It is the field the UI reads to stop the player at three, and Bless
declares the same three with a different sign. Note this is exactly the cap-versus-headroom
question from the ref-ids slice: the number is 3 because Bane and Bless both say 3, not because
some future spell might want more.

**R5 — where is an illegal target list refused?**
*Recommend both places it is refused today, unchanged in kind.* Range is checked twice already:
once when the offer is compiled, so the client can grey out what is unreachable
(`session/offers.go:701` → `session/reach.go:28`, `Distance <= CellsFromFeet(rangeFeet)`), and
again at execution, which answers `ErrStaleDeclaration` when the board has moved. Both become
per-target. Duplicates and targets not in the interaction are refused in the cast machine's
preflight, which already runs before the door for exactly this reason
(`resolution/action.go:157-165`): after the door, the bard has paid a slot for a cast that
cannot run. Note there is no line-of-sight check in the cast path today and this slice adds
none; the candidate list is built from currently-sighted intel holdings
(`session/offers.go:711`), which is the nearest thing and is unchanged.

**R6 — all three save. Does the bard stay concentrating?**
RAW says yes: the spell is cast, the duration runs, and nobody is affected. *Recommend RAW,*
because the alternative silently returns a spell slot's worth of concentration on a die roll,
and the player can drop it themselves. Flagged rather than assumed — it is a concentration with
zero children, which is the first time that state means something.

**R7 — does Bane touch ability checks?**
No. RAW is attack rolls and saving throws only. `AbilityCheckChainEvent` exists and this slice
deliberately does not fold a die into it — the day a spell says "ability checks", it adds the
same `DieSources` field to that chain and nothing else.

---

## What this slice deliberately does not do

- **No area, no shape query, no spatial caller.** Bane names creatures. Thunderwave brings the
  area selector.
- **No half-damage-on-success.** The gate stays negate-only, refused where it is refused today.
- **No push.** Forced movement has no delivery anywhere and is its own conversation.
- **No upcast.** Bane at 2nd level targets more creatures; the spend profile cannot let a player
  choose a level yet, and that is a separate row on the shelf.
- **No Bless.** Named as the next customer and built by whoever brings the cleric online, on the
  mechanism this slice lands.
