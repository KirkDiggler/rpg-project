# Cure Wounds and Dissonant Whispers — the heal arm, and half on a save

**Date:** 2026-09-09
**Status:** Proposed. No implementation has started; no production test is claimed to pass.
**Umbrella:** `ideas/spells/` — levels 1–3, and the choices at 3 made real.
**Journey:** [rpg-project#243](https://github.com/KirkDiggler/rpg-project/issues/243) — *Cast a Spell in Play*.
**Sibling slice:** [Bane, PR #409](https://github.com/KirkDiggler/rpg-project/pull/409) — approved, not yet implemented.
**Layer overview:** [Bane's overview](../bane/overview.md) describes the composable layer all
three spells join. This document does not restate it.

**Evidence boundary:** every `file:line` below is against `rpg-toolkit` `origin/main` at
`0aaa1807`, read directly. **EXISTS** means a current, non-test-driven responsibility.
**NEW** describes this proposal, not shipped code.

---

## Why these two spells

The bard can cast two cantrips. Bane is in flight as the first *levelled* spell, and it is
the expensive one: it brings the slot, an ordered target list, source-qualified condition
addresses, and an automatic contributed die. These two are the cheap ones, and each proves
exactly one new place:

| Spell | The place it proves | Size |
|---|---|---|
| **Cure Wounds** | a cast can deliver **healing**, not only damage and conditions | one consequence kind |
| **Dissonant Whispers** | a save can buy **half**, not only nothing | one outcome policy |

Neither needs multi-target, neither needs a contributed die, and neither needs concentration.
That is the point of picking them: after Bane, the next two spells should cost content and one
arm each, not another architecture.

### Three castable spells is what makes creation a choice again

The bard's creation is honest but thin, and the code says so out loud. `spells.Castable` gates
the cantrip option list to the cantrips this build can actually cast, and its own comment names
the cost: *"The cost is that 'choose 2 of 2' is not a choice, which is honest about where the
build is and disappears the moment a third cantrip gets a profile."*

Bane's acquisition has the same shape one level up — `BardSpells1`, `Count: 1`, `Options:
[spells.Bane]`. **Correct, and still not a choice.** A player picks the one supported spell.

These two spells are what turn it into one: `Count: 1` over `[Bane, CureWounds,
DissonantWhispers]` is a real decision with three genuinely different answers — debuff the
room, keep the party standing, or burst one target. That is the point of shipping them
together rather than one per wave, and it is the reason to prefer three cheap spells over one
more expensive one.

It also spreads the bard across the three things a bard does, which is what makes the class
worth testing at all: **Bane** is control, **Cure Wounds** is support, **Dissonant Whispers**
is damage. Two of the three cost one arm each.

### What they need from Bane, and it is one thing

**The leveled spell slot at the door.** Nothing else. Not the ordered target list, not the
source-qualified address, not `RollConditionOwner`, not the skip-first-turn-end grace.

So these two are gated on one Bane task rather than on all of Bane, and they can be planned
now and built the day that task lands:

- the `SpellSlotLevel1` resource key and the Bard's two level-1 uses seeded from the class
  table — `rulebooks/dnd5e/resources/keys.go` today holds six keys and no slot;
- `combat.SpendProfile.Pools` charged at the door for the first time;
- the orphaned `Data.SpellSlots map[int]SpellSlotData` (`character/data.go:80`) deleted, and
  with it the `resolution.LongRest` bug that clones the map instead of resetting it.

### What they do NOT need, because it already landed

Two things Bane's design describes are already live, and reading them as pending would
overstate this wave:

- **`KnownSpells` is on the sheet.** `Character.KnownSpells()` (`character/character.go:1659`),
  `Data.KnownSpells` (`character.go:1034`), and `Draft.compileKnownSpells(shared.ChoiceSpells, …)`
  (`draft.go:655`) all run. The choice pipeline lands leveled spell refs today. **EXISTS.**
- **A concentration condition already owns other members' conditions.**
  `ConcentratingCondition` carries `Children []dnd5eEvents.ChildRef` and `TurnEndsLeft`, ends
  them when it ends, and *describes* the Constitution check rather than rolling it
  (`conditions/concentrating.go:96-148`). One-to-many ownership is not this wave's problem, and
  it is not Bane's either — Bane's genuinely new part there is the `SourceID` on the address.
  **EXISTS.**

Neither Cure Wounds nor Dissonant Whispers concentrates, so neither touches that.

---

## 1. Cure Wounds — a cast delivers healing

### The player promise

A level-1 Bard who chose Cure Wounds spends one action and one level-1 slot, touches one
creature within 5 feet, and that creature regains `1d8 +` the bard's spellcasting modifier hit
points. A creature at 0 hit points is picked up: the heal clears its death-save progress and it
stands in the fight again. The client shows the slot as the price and the heal with its roll.

### Why this is the cheapest possible next spell

A gateless cast is **already an activation**, and an activation **already heals**.

`newGatelessCast` does not build a machine of its own — it returns
`NewActivation(&ActivationInput{…, cast: &preparedCast{…}})` (`resolution/action.go:495-527`).
And the activation's effect collector already subscribes to `HealingAppliedTopic` and emits
`EffectHealingApplied` carrying a complete sourced roll calculation
(`resolution/activation.go:101,188,215`).

The whole heal lane runs end to end today, for Second Wind:

```text
HealingReceivedEvent on the interaction bus        (features/second_wind.go:172)
  -> the sheet adds HP, clamps at max, CLEARS death-save progress, marks dirty
                                                    (character/character.go:1360-1375)
  -> HealingAppliedEvent {Requested, Applied, HPBefore, HPAfter, Calculation}
                                                    (character.go:1394, events/events.go:655)
  -> activation effect collector -> EffectHealingApplied
                                                    (resolution/activation.go:101,215)
  -> encounter ResultHealingApplied                 (encounter/activation.go:27)
  -> session HealingAppliedBody                     (session/types.go:1429)
```

**EXISTS**, all of it. Cure Wounds does not build a heal; it points the cast door at one.

### What is NEW, and where

Four touch points. Three of them are places that already name this gap in their own comments.

**1.1 — `CastProfile` gains a healing arm.** `combat/actions/cast.go` declares `Damage []damage.Damage`
and `Effects []CastEffect` and nothing else a cast can do:

```go
// NEW, in combat/actions/cast.go
// Healing is what the cast restores when it lands, or nil for a cast that
// restores nothing.
Healing *CastHealing `json:"healing,omitempty"`

// CastHealing is one pool of restored hit points.
type CastHealing struct {
    // Dice is the unsigned homogeneous pool, "1d8".
    Dice string `json:"dice"`

    // AddsSpellcastingModifier adds the caster's spellcasting ability
    // modifier to the roll. Declared rather than assumed: Healing Word and
    // Cure Wounds add it, a potion does not.
    AddsSpellcastingModifier bool `json:"adds_spellcasting_modifier,omitempty"`
}
```

`CastProfile.Validate`'s "must declare damage or a delivered condition" becomes "damage,
healing, or a delivered condition". A profile declaring both damage and healing is refused —
no supported 5e content does both, and permitting it would invent a rule.

**1.2 — the gateless delivery publishes the heal.** `preparedCast` today carries
`conditions []preparedDelivery`; it gains a healing delivery beside them. The Gather publishes
`HealingReceivedEvent` with its `Calculation` on the interaction bus, which is the *existing*
publisher shape — Second Wind's — and is what the collector is subscribed to.

> **Named asymmetry, deliberately not resolved here.** Damage is applied bus-free by a direct
> `target.ApplyDamage` call (`resolution/contest.go:355`); healing goes *through* the bus
> because the sheet subscribes. This wave uses the bus path because the collector depends on
> it. Whether resolution should eventually gain a symmetric `ApplyHealing` on the combatant
> and publish the applied fact itself is a real question and a separate one. Naming it beats
> quietly building a second healing path.

**1.3 — `deliveredConditions` gains a heal arm.** It refuses any effect kind but a condition,
and says so plainly: *"a gateless cast delivers conditions"* (`resolution/action.go:387-408`).
It becomes `deliveredEffects`, mapping `EffectHealingApplied` to a new
`ImposedHealing ImposedEffectKind`.

`ImposedEffect` needs **no new fields**. Its `Amount`, `Requested`, `Before` and `After` already
mean exactly what a heal means, and its own godoc says so: *"It is Amount's sibling for the same
reason healing has one: the two separate the moment somebody's hit points stop moving"*
(`contest.go:111-119`).

**1.4 — the spellcasting modifier reaches the door.** `CastDefinition(id Spell, spellSaveDC int)`
(`spells/cast.go`) takes a DC and nothing else. Bane's design replaces the positional call with
`CastDefinitionInput{Spell, SpellSaveDC}` and says explicitly that the spellcasting modifier is a
different fact to be added "only when a supported shape consumes them."

**Cure Wounds is the shape that consumes it.** So this wave adds one field to the named input
Bane introduces:

```go
spells.CastDefinition(spells.CastDefinitionInput{
    Spell:                spells.CureWounds,
    SpellSaveDC:          13,   // unused by a gateless cast; still the caster's fact
    SpellcastingModifier: 3,
})
```

The character already computes it — `SpellSaveDC()` is `8 + proficiency + spellcasting modifier`
— so this exposes an existing derivation rather than adding one.

### The content declaration

```go
// NEW, in spells/cast.go
CureWounds: {
    name: "Cure Wounds",
    build: func(in castBuildInput) actions.CastProfile {
        return actions.CastProfile{
            RangeFeet: CureWoundsRangeFeet, // 5 — touch
            Target:    actions.CastTargetOneCreature,
            // NO GATE. Nobody resists being healed.
            Healing: &actions.CastHealing{
                Dice:                     CureWoundsHealing, // "1d8"
                AddsSpellcastingModifier: true,
            },
        }
    },
},
```

### Targeting, and the smallest honest cut

`CastTargetOneCreature` is "one other creature within range", and the cast candidate universe
is **not** hostility-filtered. The only hostility read anywhere in the `session` package is
opportunity-attack reactor selection (`session/mover.go:520`); no declaration compiler asks the
question. So a bard can already point a cast at an ally, and Cure Wounds needs no new target
rule.

Two refinements are **deliberately out of scope**, each recorded rather than smuggled in:

- **A willing-creature restriction.** Nothing stops a bard healing a goblin. That is a
  candidate-filter nicety, not a rules hole, and `castView.IsAllied` (`resolution/cast.go`) is
  where it would read from when somebody wants it.
- **Self-targeting.** RAW Cure Wounds may target the caster; `CastTargetOneCreature` refuses an
  empty target and `CastTargetSelf` refuses a named one, so "self or another" is a third target
  rule. Not needed to prove the heal arm.

---

## 2. Dissonant Whispers — a save buys half

### The player promise

A level-1 Bard who chose Dissonant Whispers spends one action and one level-1 slot and
whispers at one creature within 60 feet. The creature makes a Wisdom saving throw. On a
failure it takes `3d6` psychic damage; on a success it takes **half**. The client shows the
save with its roll and DC, and the damage with a trace that explains the halving.

### What already runs, and it is almost all of it

`newGatedCast` builds `NewContest(&ContestInput{Gate, SaverID, Application, Damage, SourceName,
Cause, Roller})`, and a profile with damage and **no** conditions is already supported — the
switch on `len(profile.Effects)` has a bare `case 0:` (`resolution/action.go:452-470`).

So **Dissonant Whispers with `OnSuccess: Negated` would work today as pure content**: a WIS gate,
`3d6` psychic, no rider. Sacred Flame is that shape exactly (`spells/cast.go`). The single new
thing is `Half`.

And `Half` is not an invention. It is already in the vocabulary, documented for precisely this:

```go
// saves/gate.go:22-24, EXISTS
// Half means a successful save halves the damage. Only meaningful for a gate
// on a damage pool.
Half SaveEffect = "half"
```

`CastProfile.Save`'s own godoc names the spell that would use it: *"Negated-on-success only: a
successful save against a cantrip negates every consequence, and **half-on-success arrives with
a spell that has one**."* This is that spell.

### What is NEW, and the part that is not a division

Three touch points, and the third is the real work.

**2.1 — two refusals lifted.**

- `combat/actions/cast.go` `Validate`: `"cast save must negate the cast on success"` becomes
  a narrower rule — `Half` is permitted **only** when the profile declares damage and no
  delivered condition. A halved condition is meaningless, and permitting one would be an
  affordance with nothing behind it.
- `resolution/contest.go:167` `validateConditionGate`: same widening, same restriction. (Its
  name stops being accurate once a damage-only gate reaches it; rename to `validateGate`.)

**2.2 — the success branch delivers.** Today it returns `Done` before any delivery exists, which
is what makes `Negated` true by construction (`contest.go`, and the #1575 commit says so
outright: *"THE SUCCESS BRANCH IS UNTOUCHED"*). It gains one branch: on success with
`OnSuccess == Half`, run the damage delivery at half.

**2.3 — the halving must be visible in the trace, and this is the actual design problem.**

`applyPreparedDamage` carries a hard guard (`contest.go:332-342`):

```go
final, total := combat.FinalDamage(components)
if total != calculation.Total {
    return nil, fmt.Errorf(
        "%w: %s dealt %d, and its roll trace explains %d",
        ErrBadAction, describeDamage(pools), total, calculation.Total)
}
```

So you cannot quietly divide by two — the trace would stop explaining the number and the
machine would refuse its own damage. And `RollCalculation` has nowhere to say it:

```go
// events/roll_trace.go:52-55, EXISTS
type RollCalculation struct {
    Components []RollComponent
    Total      int
}
```

`RollComponent` is `{Source, Dice, Modifier}` — no operator, no sign, no multiplier.

**This is a total-level operation, and Bane's is a component-level one.** Bane adds
`RollComponent.SubtractDice` — one component contributes negatively. Half is not that: it
applies to the settled total after every component. Two different extensions to one type.

**Recommendation: sequence Dissonant Whispers after Bane's calculation work, and let it add the
total-level operator itself**, citing Bane's component-level signs as the precedent. Designing
one shared "operator vocabulary" up front for two customers we have barely met is the
speculative-expression-engine Bane's design already rejected. Two named, minimal extensions —
`SubtractDice` on a component, and a halving on the total — are honest; a grammar is not.

The minimal shape, for review rather than as settled API:

```go
// NEW, in events/roll_trace.go
type RollCalculation struct {
    Components []RollComponent
    Total      int

    // Halved reports that Total is the components' sum halved and rounded
    // down — a successful save against a spell that deals half. A BOOL rather
    // than a factor, because "half, rounded down" is the only rule 5e has here
    // and a general multiplier would invite arithmetic the rules never ask for.
    Halved bool
}
```

`ValidateRollCalculation` then checks `Total == sum/2` when `Halved`, and `Total == sum`
otherwise — so the guard above keeps its meaning instead of being weakened.

### The content declaration

```go
// NEW, in spells/cast.go
DissonantWhispers: {
    name: "Dissonant Whispers",
    build: func(in castBuildInput) actions.CastProfile {
        return actions.CastProfile{
            RangeFeet: DissonantWhispersRangeFeet, // 60
            Target:    actions.CastTargetOneCreature,
            Save: &saves.SaveGate{
                Abilities:  []abilities.Ability{abilities.WIS},
                DC:         saves.DCStatic(in.SpellSaveDC),
                OnSuccess:  saves.Half,
                Recurrence: saves.RecurrenceNone,
            },
            Damage: []damage.Damage{{
                Dice: DissonantWhispersDamage, // "3d6"
                Type: damage.Psychic,
            }},
        }
    },
},
```

### The forced move is deliberately deferred

RAW 2014: a creature that fails *"must immediately use its reaction, if available, to move as
far as its speed allows away from you."*

**Kirk's call, 2026-09-09: this wave ships the damage and the half, and the movement is recorded
as a gap rather than built.** That is the smallest cut that keeps the slice true, and it keeps a
one-arm slice from becoming a movement-ownership slice.

Recorded honestly, because the cut has a cost: **Dissonant Whispers without the flee is Sacred
Flame with better dice and a save that buys something.** The spell's identity as a *tool* — pull
a creature off the squishy, out of a doorway, into the open — is the half we are not shipping.
The gap issue should say that in those words, so nobody later reads the shipped spell as the
finished one.

What would fill the place, when it is cut:

- **A new consequence kind: an effect moves a creature.** `ImposedEffectKind` is
  `condition | damage | condition-removed` and no delivery anywhere pushes, pulls or displaces.
- **The path is a rules computation, not a brain decision.** "As far as its speed allows away
  from you" is the furthest reachable hex maximising distance from the caster — a `tools/spatial`
  query, deterministic, owned by resolution. It does **not** need monster AI, which is the
  finding that makes this tractable and keeps it out of Billy's lane.
- **The reaction is spent by somebody else's effect.** Opportunity attack established a reaction
  economy; a cast forcing a target to spend its reaction is a new direction through it.
- **The move is ordinary movement and provokes.** The Movement machine already produces reaction
  triggers and resolves them inline (`resolution/movement.go:313`), so "flee and get shot at"
  needs no new custody.

First customers when it lands: Dissonant Whispers' flee, Thunderwave's push, Thorn Whip's pull,
Command's approach. It is a primitive with four customers, which is why it deserves its own
slice rather than a corner of this one.

---

## 3. One small correction this wave should carry

True Strike declares a duration that is off by one, and apologises for it in a comment
(`spells/cast.go`):

```go
// TrueStrikeTurnEnds is how many of the caster's turn ends the concentration
// survives: the end of the turn it was cast on, and the end of the next one.
const TrueStrikeTurnEnds = 2
```

True Strike lasts **1 round**. It says `2` because the first turn end reached is the casting
turn's own, and ending there would mean the advantage never existed.

Bane's design introduces `SkipFirstTurnEnd` on the concentration profile for exactly this
problem, one spell later. **That flag is the honest version of True Strike's fudge**, and the
number should stop lying about the duration:

```go
Concentration: &actions.CastConcentration{TurnEnds: 1, SkipFirstTurnEnd: true},
```

Same observable behaviour, one fewer comment explaining why a constant is not what it says. This
belongs in whichever of the two waves lands `SkipFirstTurnEnd` first; it is named here so it is
not lost.

---

## 4. Acceptance contract

Through production entrypoints and persistence boundaries, not research models.

**Cure Wounds**

1. **Payment.** Missing action or missing level-1 pool refuses the cast with no HP change, no
   pool debit and no RNG consumed.
2. **The heal.** One paid cast on a wounded ally raises its HP by `1d8 + modifier`, clamped at
   max, and the record carries `Requested`, `Applied`, `HPBefore`, `HPAfter` and the full
   sourced calculation.
3. **The clamp is visible.** A heal larger than the ally's missing HP records `Requested >
   Applied` rather than reporting the clamped number as the roll.
4. **Picking somebody up.** Cure Wounds on an ally at 0 HP raises it above 0 **and** clears its
   death-save progress, through `Character`'s existing transition and not a second write.
5. **Modifier provenance.** Two bards with different Charisma heal different amounts from the
   same declaration, and the modifier arrives from `CastDefinitionInput` rather than being
   re-derived in resolution.
6. **Refusals.** A profile declaring both damage and healing is refused at content validation. A
   gateless cast that delivered a kind other than a condition or a heal is refused, not dropped
   from the record.

**Dissonant Whispers**

7. **Failed save.** Full `3d6` psychic applied through the sheet's own `ApplyDamage`, so a
   target dropped to 0 flows through the death-save transition that call already owns.
8. **Successful save.** Half the settled total, rounded down, applied — **not** zero, and not a
   negated cast.
9. **The trace explains the number.** On a successful save the recorded calculation's components
   sum to the pre-halving total, `Halved` is true, and `Total` is the halved value.
   `ValidateRollCalculation` refuses a calculation whose total matches neither rule.
10. **The save is visible.** The client receives the WIS save's roll, DC and outcome, and the
    damage beat, for both branches.
11. **Regression.** Sacred Flame and Vicious Mockery still negate on success and record
    identically. A `Half` gate on a profile with a delivered condition is refused.

**Both**

12. **Acquisition.** Real creation offers the supported level-1 spells, finalization stores the
    chosen refs in `KnownSpells`, and reload preserves them.
13. **Slot lifecycle.** Each cast spends one level-1 use, persistence retains the spend, and
    LongRest restores two. No `Data.SpellSlots` state or proto projection remains.
14. **Afford honesty.** After one cast, a refreshed Afford reports the spell unavailable for the
    spent action, and a second same-turn cast is refused before mutation or RNG.

---

## 5. Rejected alternatives

- **A heal machine.** A machine exists only when a shape needs different steps. A gateless cast
  is already an activation and an activation already heals; a Cure Wounds machine would be the
  spell-named machine the brainstorm's second ruling forbids.
- **A second healing path in resolution.** Publishing `HealingReceivedEvent` reuses the publisher
  the collector is subscribed to. A direct `ApplyHealing` write that then published its own
  applied fact would be a second way to heal, and the collector would have to trust it.
- **Reusing `ImposedDamage` with a negative amount for healing.** Falsifies the fact. Healing has
  its own clamp, its own death-save consequence and its own beat, all of which already exist.
- **A general damage multiplier on the calculation.** "Half, rounded down" is the only rule 5e
  has here. A factor invites arithmetic no rule asks for and a UI that has to render it.
- **One shared operator vocabulary designed now for Bane's signs and this wave's halving.** Two
  customers, barely met. Bane's design already rejected a speculative expression engine; this
  would be the same mistake from the other end.
- **Half applied per damage component.** Rounding each pool separately gives a different answer
  from halving the total, and 5e halves the total.
- **Building the forced move inside this slice.** It is a primitive with four customers and an
  ownership question of its own. Kirk's call is to defer it and record the cost.
- **Adding a willing-creature target rule to prove the heal.** The candidate universe is not
  hostility-filtered today, so the heal arm can be proven without it. Recorded as a nicety.

---

## 6. Delivery

Develop outside-in, merge inside-out; one branch per repo per wave.

1. **Gated on one Bane task** — the `SpellSlotLevel1` pool charged at the door. Until it lands,
   neither spell can be paid for honestly.
2. **Dissonant Whispers is additionally gated on Bane's `RollCalculation` work**, so the trace
   grows one operator vocabulary in two named steps rather than two in parallel.
3. **Cure Wounds is gated on nothing else** and is the smaller of the two. It can land first.
4. Toolkit work stays on one wave branch; provider commits are pushed and resolved to actual
   pseudo-versions for a named local stack. No pseudo-version is written by hand.
5. Protos are the early real-PR exception because CI mints the generated clients.
6. One independent review round examines the exact tested heads, with the verdict published on
   the PR.

Nothing here authorizes a merge, a force-push, a shared-stack deployment, or an environment wipe.

— cross-team agent, on behalf of KirkDiggler
