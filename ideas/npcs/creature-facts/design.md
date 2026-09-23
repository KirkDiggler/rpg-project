# Facts, not conditions — what a creature can know, and why `when` is not the bottleneck

**Status:** RULED 2026-09-23 — IMPLEMENTED, in review. Written 2026-09-22/23
from a World-Builder walk (rpg-dnd5e-web#1192) and a set of runnable experiments
(six unit tests in `mind/behavior`, no board, no clock, no rulebook). Supersedes
the first draft of this idea, which framed the problem as "named parameterized
regimes" and was wrong about where the gap is. Builds on the shipped
creature-table design (`ideas/creature-table/design.md`, SHIPPED 2026-09-18).

**The ruling:** widen the fact projection — a deed against the creature's own
side, and its own actions as facts it holds — and let `when` gain forms that read
them. The implementation is issue rpg-toolkit#1883 (spec), provider
rpg-toolkit#1884, consumer rpg-toolkit#1890, and the stack that walked it
rpg-api#1043. **Walked 2026-09-23**: a goblin authored with `attack: attacker`
pursued the fighter that struck it and died to an opportunity attack on the way,
the `answered` beat naming `"selector": "attacker"`.

The ruling also confirmed what this doc argued: named reusable tables are the
**site's** business and are not in this slice (see the pointer at
`ideas/dungeon-authoring/world-builder/named-behavior-tables.md`).

## The one sentence

**The evaluator (`when`) is not the constraint; the facts it reads are.** Two of
the behaviours we want — a guard's pause and "a goblin who watched its friends
fall" — are inexpressible today not because the table lacks a word, but because
the creature is not *given* the fact. And the fact is usually already in the
room: the deed carries it, the witness holds it, and the projection discards it
on the way in.

## How the first draft was wrong (kept visible)

The first version of this idea proposed **named, parameterized behavior
regimes** at the site root — a "guard" or "patrol" block with `params`. Kirk's
correction, in the noodle-throwing that followed:

> "when is the evaluator. I like the new selector work… route would need to be in
> the table, it is another option like pause. Those parameters need to land
> somewhere."

And then the reframe that actually landed:

> "seeing the evaluator for what it is and seeing new facts could feed it. the
> that deeds are always target self is important. we have audience idea so others
> can observe things maybe we just need a little of that."

The draft was wrong in one specific way worth recording: it treated the gap as a
**document-shape** problem (a place to put a reusable named block) when the
evidence says it is a **fact-projection** problem.

Kirk's ruling on the deleted half, kept here because it is the reason the idea
does not linger: *"we will be building them in the site. I think we name them
there to start and when we want to carry over one dungeon's creature facts to
another, then that idea flows out of there."* Names are the **site's** business;
the engine never learns one. Building the named block first would have added
surface without adding capability — the same failure this doc exists to avoid.

## The evaluator, seen plainly

`behavior.When` is a condition over `behavior.Facts`:

```go
type Facts struct {
    EnemyInReach, EnemySeen, EnemyRemembered bool   // where THEY are
    CanAttack, CanMove                       bool   // what I can AFFORD
    Deeds []HeldDeed                                // what was done TO ME
    Now uint64
}
```

Four things, and the experiments confirm each behaves as designed:

- **`when` picks a row**; an unmet `when` is **absent from the candidates, not
  weighted zero** — the candidate list is the honest account of what the
  creature could have done. This works. (`TestWhenIsTheEvaluator`)
- **A place is already a selector** — `{ at: [col, row] }` exists, so "a where"
  is not new machinery. The real gap is narrower: a selector *word* names a
  **member** (`enemy`/`attacker`/`actor`) and never a **placed thing**, and an
  entry carries **exactly one** selector. So "guard the reliquary, within 2"
  needs either a placed-thing selector or an entry that carries two targets.
  (`TestPlaceIsAlreadyASelector`)

## The two behaviours we cannot express, and why they are ONE problem

### 1. The pause — "after I strike, stand still for 2 rounds"

Proven inexpressible by construction (`TestPauseIsNotExpressibleToday`): the row
is **dead weight**. `Within` ages a deed, and the only deeds a creature holds are
deeds **against itself**. Nothing records what the creature **did**, so a
condition about its own past action can never hold — and the failure is *silent*:
the attack simply stands and the pause never happens.

### 2. "A goblin who watched its friends fall"

Proven invisible (`TestDeedsAreOnlyAgainstSelf`): a deed aimed at somebody else
produces **byte-identical output** to no deed at all.

### They are one problem, and the information already exists

This is the finding that makes the doc worth writing:

- **Deeds land on every WITNESS, not just the target.** `landDeed` "puts one deed
  on every witness, at the clock's high-water mark" — so a creature that watched
  a friend get hit **already holds that deed**.
- **The deed payload already carries `Target`.** `deed.Deed` is
  `{Verb, Actor, Target, Where}` — encoded, decoded, and available.
- **The projection discards it.** `heldDeedsAgainst` filters `if d.Target != self
  { continue }` — deliberately, and the reason is written down ("reading it as
  one would have a goblin retaliate for a blow it merely witnessed"). The filter
  is *right*; what is missing is a **second reading** beside it.
- **`HeldDeed` has no `Target` field.** The evaluator is handed `{Kind, Actor,
  At}` — so a table cannot ask "was my ally hit" because the subject was dropped
  one layer down, even though the room knows.
  (`TestTheTargetIsPresentAndThenDiscarded`)

**So the fix is a READ change, not a new fact source and not a new wire field:**
widen the projection to produce a second reading (a deed against *my side*),
and/or project the creature's **own** actions as deeds it holds about itself.

## What this asks for

Two projections beside `Deeds`, in the same `Facts` literal where `Deeds` is
already set (`encounter/facts.go`) — which is the "next to / in line with the
deeds" placement Kirk asked for:

1. **A deed against my side.** Today `e.opposed(id, subject)` answers "is this
   an enemy"; the mirror — "is this one of mine" — is what a `my side` reading
   needs. The audience/witness machinery already files the deed; only the
   projection's target filter changes.
2. **My own actions as a fact.** A pause needs "what I just did". The same
   machinery files the actor on every deed; nothing reads it back.

Then `When` gains forms that read those, and **pause, morale-by-attrition and
intel-driven behaviour all fall out of machinery that already exists** (`Within`
spans for the pause; the existing deed verbs for the rest).

## Deferred deliberately

- **Temperament as a function of facts.** Kirk: *"we defer the temperament part.
  It will deserve its own thing."* The experiments show it is a **fourth axis** —
  `factor()` is a percent multiplier, so "more likely to flee" is a *load* change,
  not a condition (`TestTemperamentReloadsTheSameTable`: same table, same facts,
  a coward loads `Attack 50 / Away 300` where a soldier loads `100/100`). But it
  is dealt **once at the door** with a `tempered` beat, and making it dynamic
  raises its own questions (does it re-deal? does the beat fire again? is it a
  second computable layer?). **Its own doc.**
- **Storage repository, loader/embed.** Unchanged from the first draft: defer,
  lean into the encounter.

**NOT deferred — deleted.** Named reusable regimes do not belong in this idea at
all. Kirk: *"we will be building them in the site. I think we name them there to
start and when we want to carry over one dungeon's creature facts to another,
then that idea flows out of there."* A named block is therefore a **site-document**
idea, born when a site needs it, and the engine learns nothing about names. It
appears in this doc only as the first draft's mistake, never as a queued idea.

## Evidence

Six experiments, all passing, in `mind/behavior` — runnable claims with a die the
test controls, no board, no clock, no rulebook, exactly as that package's own
test file describes its method:

| Experiment | Claim |
|---|---|
| `TestWhenIsTheEvaluator` | an unmet `when` is absent, not zero-weighted |
| `TestPlaceIsAlreadyASelector` | `{at}` exists; the gap is a placed-thing selector |
| `TestTemperamentReloadsTheSameTable` | temperament is a load, not a condition (deferred) |
| `TestDeedsAreOnlyAgainstSelf` | an ally's deed is invisible to the table |
| `TestPauseIsNotExpressibleToday` | a pause row is dead weight, silently |
| `TestTheTargetIsPresentAndThenDiscarded` | the deed carries `Target`; the projection drops it |

## What is being ruled on

**Widen the fact projection — a deed against my side, and my own actions as facts
I hold — and let `when` gain forms that read them.**

There is no competing option to weigh. The document-shape answer (named reusable
regimes) was the first draft's mistake and is deleted: names belong to the site,
and the engine will learn them when a site needs to carry creature facts between
dungeons. What remains is one question with a small, testable answer.

If ruled yes, the first slice is small: one projection change in
`encounter/facts.go`, one or two `When` forms, and a guard's pause plus
attrition morale become expressible with machinery already built.

— ui/ux lane, on behalf of KirkDiggler