# Character advancement — brainstorm

**Date:** 2026-09-15
**Status:** Brainstorm. One ruling taken, one ruling proposed, several shelves left empty.
**Umbrella:** `ideas/characters/`
**Initiative:** rpg-project#231 (four-player level-3 dungeon)
**Journeys:** rpg-project#242 (earn XP and level up between runs), #241 (class resources)

---

## What prompted this

Kirk, 2026-09-15: *"I would like to start checking what we need to level up our classes
to level 2."*

The honest answer turned out not to be a content list. Almost all of the level-2 content
for the classes the initiative names is **already written, already routed, and
unreachable**. What is missing is one primitive, and one decision about what a character
is.

Then, on being shown the fork: *"I would definitely want a record of each level and what
changed."* That settles the shape. This document records how we got there, what we
rejected, and what is deliberately left unbuilt.

## The facts this stands on

Measured 2026-09-15 against `rpg-toolkit` `main` @ `8c69a54f`, `rpg-api` `dev`,
`rpg-dnd5e-web` `dev`, `rpg-api-protos` `main`. Two read-only sweeps plus a probe
program that was actually run, not inferred.

### The engine is already level-parameterized almost everywhere

- `classes.Grant` carries a `Level` field, and `classes.GetGrantsForLevel(classID, level)`
  filters on it — `rulebooks/dnd5e/classes/grant.go`. The filter is **cumulative**
  (`grant.Level <= level`).
- `choices.GetClassRequirementsAtLevel(classID, level)` gates the subclass requirement on
  the character's level — `rulebooks/dnd5e/character/choices/requirements.go`.
- Resource maxima already scale from level: barbarian rage uses, monk Ki (already written
  as `if level >= 2`), and hit dice — `rulebooks/dnd5e/character/draft.go`,
  `initializeClassResources`.
- `GetExtraAttacksCount()` carries the whole Fighter 5/11/20 progression —
  `rulebooks/dnd5e/character/character.go`.
- Subclass domain spells are already tabled per level —
  `rulebooks/dnd5e/character/choices/subclass_modifications.go`.
- Level already projects through `resolution/projection.go` to the session view, and the
  web header renders it with rarity tiers up to level 15.

### Every one of those paths is pinned to 1 by hand

- `character/draft.go` sets `level: 1` and `proficiencyBonus: 2` as literals when it
  compiles a character.
- Every production call of `GetGrantsForLevel` passes the literal `1` — proficiencies,
  equipment, `compileFeatures`, `compileConditions`.
- Every production call of `GetClassRequirementsAtLevel` passes `1`.
- `maxHP = classData.HitDice + CON modifier` is the level-1 rule and nothing else; no
  code rolls or averages a later hit die.
- `GetRequirementsRequest` on the wire has **no level field**, so rpg-api's level
  parameter is always zero and always defaults to 1. The level-aware path is built and
  unreachable from any client.

### Every grant in the repository is `Level: 1`

Six of twelve classes have grants at all (fighter, barbarian, monk, rogue, bard, cleric);
the other six return nil. The entire granted vocabulary across all twelve classes is
**seven refs** — Second Wind, Rage, Unarmored Defense (twice), Martial Arts, Bardic
Inspiration, Sneak Attack. No class has a single grant at level 2 or above.

A probe run against the working tree:

```
fighter    grants@L1=1 grants@L2=1 (delta=0)
barbarian  grants@L1=1 grants@L2=1 (delta=0)
monk       grants@L1=1 grants@L2=1 (delta=0)
rogue      grants@L1=1 grants@L2=1 (delta=0)
BUILD OK   dnd5e:features:action_surge     -> *features.ActionSurge
BUILD OK   dnd5e:features:reckless_attack  -> *features.RecklessAttack
BUILD OK   dnd5e:features:flurry_of_blows  -> *features.FlurryOfBlows
BUILD OK   dnd5e:features:patient_defense  -> *features.PatientDefense
BUILD OK   dnd5e:features:step_of_the_wind -> *features.StepOfTheWind
```

Level 2 adds nothing for any of the four classes the initiative names, and every level-2
feature those classes need **already constructs successfully from its ref today**.

### The level-2 content that exists, and the two pieces that do not

| Class | Level 2 (2014 PHB) | State |
|---|---|---|
| Fighter | Action Surge | Built, routed, unreachable |
| Monk | Ki (2), Unarmored Movement; spent by Flurry / Patient Defense / Step of the Wind | Built, routed, unreachable; Ki already gated `level >= 2` |
| Barbarian | Reckless Attack, Danger Sense | Reckless Attack built and unreachable; **Danger Sense absent** |
| Rogue | Cunning Action | **Absent.** Everything it composes exists — Disengaging/Dodging/Hidden conditions and a bonus-action economy with a `GrantedActionKey` concept |

The other eight classes have nothing at level 2. That is not a backlog; it is a map.
Content arrives when a use case brings it.

### The one thing that is actually missing

**No path applies a `classes.Grant` to a character that already exists.** A Grant is
consumed only by `Draft` compile — `compileFeatures` and `compileConditions` in
`character/draft.go` are the only production callers of `features.CreateFromRef` and
`conditions.CreateFromRef`. `Character` exposes `AddResource` and `AddCombatAbility` but
has **no `AddFeature` and no `SetLevel`**; the only writer of `level` in the whole
package is a test helper.

Everything else on the list above is data, or a literal `1` waiting to be a parameter.

### Ownership is already where it belongs

rpg-api **never writes a level.** It reads the toolkit's value in three converters and
passes it through. The toolkit is the sole owner of what a character's level means. The
single `level = 1` in rpg-api is a *query* default on `GetRequirements`, not a character
default. Nothing needs to move.

### The contract already anticipated this

`Character.experience_points` exists on the wire, is never written by rpg-api, and so the
web sheet's Experience box permanently reads `0`. Alongside it sit
`CHOICE_SOURCE_LEVEL_UP`, `CHOICE_CATEGORY_FEATS`, `CHOICE_CATEGORY_ABILITY_SCORES`, and
`SpellSelectionInfo.requires_replace` ("whether spells can be replaced on level up") —
all declared, all unused.

There is **no LevelUp RPC on any of the fifteen services**. After `FinalizeDraft`, the
only character mutations on the wire are the four equipment/inventory calls.
`CharacterDraft` carries no level field, so the creation contract cannot say which level
it is building at.

## The fork, and the ruling

`character.Data` persists a **compiled sheet** — final ability scores, final
proficiencies, a stored `ProficiencyBonus`. The choices that produced it live only on the
`Draft` and are discarded at finalize. So a level-up is one of two things:

**Rejected — mutate the sheet.** Add an "exactly level N" accessor and a
`Character.ApplyGrant` that changes the live character in place. It is the smallest
possible cut and it is why it is tempting. What it forecloses: the sheet becomes the only
truth, so every level-up must be correct the first time forever. Nothing can be
recomputed when we fix a rule. "Where did this proficiency come from?" has no answer.
Multiclass and retroactive correction become surgery on live data.

**Taken — the character keeps a record of its levels.** Kirk, 2026-09-15: *"I would
definitely want a record of each level and what changed."* The record is append-only; the
sheet becomes a projection of it. Level-up is an append.

## The sharpening: a record of inputs, not of effects

Kirk's phrase was "what changed", and there are two records that both answer to that
description. They are not equivalent and the difference is load-bearing.

**A record of effects** — *"level 2 gave you Action Surge, +7 max HP, +1 Ki."* It reads
well and it is what a player wants to see. But it is frozen history. The day we correct a
bug in what level 2 grants, every existing character's record becomes a confident,
permanent account of something that should never have happened, and there is no way to
reconcile it with the character's actual sheet. The record and the truth drift, and a
drifted record is worse than no record because it lies with authority.

**A record of inputs** — *"level 2 taken in fighter; hit-die roll 7; no choices
required."* It replays. Recompile and the corrected grants come out. The stored roll
keeps the one genuinely irreproducible part stable, which is the same call already made
for authored wall runs: a random outcome is **stored and inert** because it cannot be
re-derived, while everything derivable is derived.

"What changed" is then rendered from inputs plus rules, and stays true as the rules move.
The reverse is not available: effects cannot be replayed into inputs.

**Proposed ruling:** store inputs, display effects. Flagged here for confirmation rather
than assumed silently.

## Why this is worth more than level 2

Applying a grant to an existing character is the same act as taking a subclass at level 3,
an ability score improvement at level 4, a feat, a boon, or anything that ever grants a
proficiency after creation. Today `classes.Grant` is a creation-time data format wearing
the name of a primitive. This is what makes it one.

Judged by the tool it adds rather than the feature it closes, that is the whole
justification for the slice. Level 2 is the cheapest thing that proves it.

## Shelves, left empty

Named attachment points with nothing on them. None of these is built until a use case
arrives.

- **Multiclassing.** A character has one `ClassID` and one `Level`, on the wire and in the
  toolkit; multiclass is not representable today. A levels-taken record makes it additive
  later — a level taken in a different class is a different entry — without building any
  of it now. Journey #242 lists it on its own possibility shelf.
- **XP.** `experience_points` exists on the wire and is never written. XP is a storage and
  economy question; grant application is the engine question. The engine capability should
  be provable with a directly granted level, so the two can land independently.
- **Ability score improvements and feats at level 4.** The choice vocabulary is already on
  the wire. The record gives them a home when they arrive.
- **Spell slot progression.** `classes.Data` pins `SpellSlots`, `CantripsKnown` and
  `SpellsKnown` as "at level 1" scalars; the wire's `SpellSlots` message is deprecated and
  its v1alpha2 replacement explicitly excludes slots. Caster advancement needs its own
  design and none of the four classes in the initiative is a caster.
- **Average versus rolled hit points.** Journey #242 lists both. The record stores
  whatever was decided; it does not care which.
- **Levelling down, or rebuilding a character.** An append-only record makes these
  discussable later. Nothing here builds them.

## Found in passing

- **Ranger's fighting style is offered at level 1.** It is a level-2 feature.
  `getRangerRequirements()` returns it from the base requirements, so it appears at any
  level — `character/choices/requirements.go`, known as `TODO(#306)`. A real rules bug
  sitting on exactly this seam.
- **rpg-toolkit#1760** — the class catalog offers subclass options a level-1 character
  cannot choose — is the same seam seen from the creation side.
- **`characterRepo.Update` exists in rpg-api with no orchestrator caller.** A
  whole-character persist door, built and unwired. Worth knowing it is there; not
  evidence that anyone intended this.
- The Dec-2025 toolkit `[L2]`/`[L3]` issues (#453, #454, #456–#460) predate this
  architecture. Good content lists, not plans.

— cross-team agent, on behalf of KirkDiggler
