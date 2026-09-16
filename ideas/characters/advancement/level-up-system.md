# The level-up system — design

**Date:** 2026-09-16
**Status:** Design, PROPOSED.
**Foundation:** `ideas/characters/advancement/design.md` (rung 1, built as rpg-toolkit#1766)
**Initiative:** rpg-project#231 · **Journeys:** rpg-project#242, #241
**Scope:** the system. Class content is configuration and is named, not designed, here.

---

## 1. What this is, and what it is not

> *"technically I want the level up system built and what is filled in is available …
> we can configure it however we want so we don't need to focus on that. we are building
> a game here not showing things off."*
> — Kirk, 2026-09-16

The deliverable is **a system that levels a character up**: earn experience, cross a
threshold, be offered the choices that level actually requires, make them, and come out
the other side with the character the rules say you should have.

Which features a class gets, and at which level, is **configuration**. This document
names the configuration we start with and designs the machinery that reads it. A class
table is not a feature of this design; it is input to it.

**The test of this design is not that a bard levels.** It is that a class nobody has
written yet levels correctly the day someone fills its table in, without touching the
screen, the API, or the engine.

## 2. The ruling that shapes everything below

> *"we stick to 2014 as much as we can until it presents a problem. choosing what from
> where should be deliberate and not off the cuff … we start setting up 2014 but if
> configured differently we can do anything we want."*

- **R2.1** The starting configuration is **2014**. Every per-level fact we author comes
  from the 2014 tables unless a divergence is recorded as a ruling with its reason.
- **R2.2** An edition divergence is a **deliberate, recorded decision**, never a
  convenience taken mid-implementation. (Working example: Expertise is bard level 3 in
  2014 and level 2 in 2024. We take **2014** — level 3 — and if that ever moves, it moves
  as a ruling and as a one-line table change, not a code change.)
- **R2.3** **No per-level fact may be expressible only in code.** If changing when a class
  gets something requires editing a function, the system has failed its purpose. This is
  the rule the rest of the design serves.

`classes.Data.SubclassLevel` is the existing proof that R2.3 is achievable: cleric takes
its Divine Domain at 1, fighter its Archetype at 3, wizard its Tradition at 2 — three
different answers, one field, no branching. Everything below aims to make the rest of the
per-level facts behave the way that one already does.

## 3. The structural problem

Two surfaces must answer *"what does level N bring?"* Today only one can.

**Grants are data indexed by level.** `classes.Grant` carries a `Level`, and rung 1 added
`GetGrantsGainedAtLevel(class, level)` returning exactly that level's grants. Fighter's
level-2 Action Surge is one table row.

**Requirements are code, and level-blind.** `GetClassRequirementsAtLevel(class, level)`
returns `getBaseClassRequirements(class)` — a per-class Go function — plus a subclass
requirement gated on `SubclassLevel`. The base half does not vary by level at all.

The consequence is exact and it is the reason this design exists: rung 1's
`GetClassChoiceIDsGainedAtLevel` computes requirements(N) minus requirements(N−1), so
**the only thing that can ever appear in a level-up delta today is a subclass.** Bard at
level 2 returns nothing. Expertise at bard level 2 or 3 is not expressible at any level,
in any edition, because there is nowhere to put it.

A level-up screen driven by that surface would correctly render an empty form forever.

### 3.1 The third hole: progression is a level-1 scalar

`classes.Data` carries `CantripsKnown`, `SpellsKnown` and `SpellSlots` each commented
*"at level 1"*. There is no per-level table. A caster's progression is not merely
unconfigured — it is **inexpressible**. On the wire it is worse: `SpellSlots` is already
`[deprecated = true]` and its v1alpha2 replacement `ResourceView` explicitly excludes
spell slots.

## 4. The shape

### 4.1 Requirements become level-indexed data

- **R4.1** A class's requirements MUST be authored per level, in the same shape grants
  already are — a level-tagged table, not a function. `GetClassRequirementsAtLevel`
  becomes a fold over that table rather than a switch over classes.
- **R4.2** `GetClassRequirementsAtLevel(class, N)` keeps its current meaning — the
  **cumulative** requirements of a level-N character, which is what creation asks.
  `GetClassChoiceIDsGainedAtLevel` keeps returning the delta, which is what level-up asks.
  Both then work for every level rather than only for subclasses.
- **R4.3** The subclass requirement stays derived from `SubclassLevel` rather than being
  restated per class in the new table. One fact, one home.
- **R4.4** Every existing level-1 requirement MUST come through the new table unchanged.
  Creation is the regression test: if a level-1 character can still be built choice for
  choice, the table is faithful.

### 4.2 Progression becomes a per-level table

- **R4.5** `CantripsKnown`, `SpellsKnown` and `SpellSlots` MUST become per-level
  progressions. Level 1 reads the same values it reads today.
- **R4.6** A progression that *increases* at level N MUST surface as a **requirement** at
  level N, not as a silent grant — gaining a spell is a question, and the screen exists to
  ask it. A slot increase is not a question and is applied without asking.
- **R4.7** Spell slots on the wire are unresolved (`SpellSlots` deprecated, `ResourceView`
  excludes them). The engine owns the progression regardless; how slots reach the client
  is settled when a caster levels, not before.

### 4.3 Experience

- **R4.8** Experience is **cumulative and never debited.** The total only grows.
- **R4.9** Level **entitlement** is derived from the total by a threshold table the
  toolkit owns. Entitlement is not level: it is what the character *may* take.
- **R4.10** The character's actual level is `len(Levels)` — the record from rung 1.
  **The gap between entitlement and the record is the "level up available" signal.**
  No flag, no stored state, nothing to keep in sync; and journey #242's "an eligible
  character may keep playing without levelling" falls out for free.
- **R4.11** `Advance` MUST refuse a level the character is not entitled to. That is a game
  rule and the toolkit owns rules; the same check in the orchestrator would be logic in
  the API.
- **R4.12** There is **no debug bypass.** To seed a level-2 character you seed the
  experience that entitles it and call `Advance`. A bypass flag is a second way to level,
  and a second way to level is a second thing to be wrong.

### 4.4 The screen

- **R4.13** The level-up screen MUST render whatever requirements the toolkit returns for
  that level, and MUST contain no class-specific branch. It is the creation screen's
  choice components pointed at a delta instead of a total.
- **R4.14** A level that requires nothing MUST still be a correct screen — a confirmation,
  not an empty form. This is the common case: every martial class at level 2 asks nothing.
- **R4.15** The client sends the choices it was asked for and nothing else. Validation is
  the engine's (`Advance` already refuses choices nobody asked for).

## 5. The configuration we start with

2014, per R2.1. **Named, not designed** — each is a table row once §4 lands, and what is
already built is noted so the cost is visible.

| class | level 2 | choices it asks | built? |
|---|---|---|---|
| Fighter | Action Surge | — | **shipped** (#1766) |
| Barbarian | Reckless Attack, Danger Sense | — | Reckless Attack built; Danger Sense absent |
| Monk | Ki, Unarmored Movement (+ Flurry / Patient Defense / Step of the Wind) | — | all built |
| Rogue | Cunning Action | — | absent |
| Bard | Jack of All Trades, Song of Rest, spells known 4→5, slots 2→3 | **1 spell** | both features absent |

Bard is the only one of the five that asks a question, which is why it is the proof case:
the four martial classes prove R4.14 (ask nothing, correctly) and bard proves R4.13.

Bard's **Expertise is level 3 in 2014** and stays there (R2.2), arriving with the Bard
College — which is **rpg-toolkit#1767**, the subclass that cannot be expressed as a
choice. Level 3 is therefore the next wave, not this one, and it is gated on #1767 for
every class, not just bard.

**Content is not a gate on this design.** Per Kirk: *"what is filled in is available."* A
class whose level-2 features are absent still levels correctly — it gains its hit points,
its record entry, and whatever its table does carry.

## 6. Ownership

Unchanged from the existing seams, and worth stating because level-up spans all of them:

- **Toolkit** owns the thresholds, the per-level tables, what a level requires, what it
  grants, and whether an `Advance` is legal.
- **rpg-api** stores and projects. It already writes no level (three read-only
  converters); it must not start deciding one.
- **Web** renders what it is handed and sends back the choices it was asked for.
- **Protos** carry experience (the field exists and has never been written), the
  requirements for a level, and the level-up call itself. There is **no level-up RPC on
  any of the fifteen services** today, and `CharacterDraft` has no level field.

## 7. Done when

1. A class's per-level requirements and progression are **data**. Moving a feature between
   levels, or between editions, is a table edit with no code change (R2.3).
2. Creation is unchanged — every level-1 character builds choice for choice as before
   (R4.4).
3. A character accumulates experience; entitlement is derived; the gap between entitlement
   and the record is what the client is told (R4.10).
4. `Advance` refuses a level the character has not earned, with no bypass (R4.11, R4.12).
5. A **bard** crossing the threshold is offered its spell choice on a screen with no bard
   in it, chooses, and comes out at level 2 with the spell, the slot, and a two-entry
   record.
6. A **fighter** crossing the threshold is offered a confirmation, takes it, and comes out
   with Action Surge.
7. Seeding a levelled character means seeding experience, not writing a level.
8. A class whose table is empty at level 2 still levels correctly.

## 8. Out of scope, seams named

Level 3 and subclass-as-a-choice (**#1767**, gates every class) · multiclassing (R2.4 of
the rung-1 design names the seam) · ability score improvements and feats at level 4 (the
record and the requirement table both have room) · what awards experience and how a party
splits it (#242's shelf — this design consumes a total, it does not produce one) ·
prepared-versus-known casting (#445 is already in it) · levelling down or respec.

## 9. Known blockers

- **rpg-toolkit#1767** — a subclass is the one requirement the engine can ask for and
  cannot receive. Gates level 3 for every class.
- **rpg-toolkit#1769** — Action Surge can never be activated; no caller supplies an
  `ActionEconomy`. Does not block levelling, but a fighter who levels cannot use what he
  gained.
- **rpg-toolkit#1770** — `dnd5e` pins a `tools/spatial` revision that no longer exists,
  blocking any gorelease whose baseline pulls `dnd5e v0.172.0`.

— cross-team agent, on behalf of KirkDiggler
