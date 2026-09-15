# Character advancement — design

**Date:** 2026-09-15
**Status:** Design, RATIFIED. Both rulings taken (§7).
**Brainstorm:** `ideas/characters/advancement/brainstorm.md`
**Initiative:** rpg-project#231 · **Journeys:** rpg-project#242, #241
**Scope:** `rpg-toolkit` only. No proto, rpg-api or web change in rung 1.

---

## 1. What this adds

A character keeps an append-only **record of the levels it has taken**, and the toolkit
gains **one primitive that applies a class grant to a character that already exists**.

Level 2 is the proof case, not the goal. The same primitive is what a subclass at 3, an
ability score improvement at 4, a feat, or a boon will each use.

## 2. The record

### 2.1 Shape

```go
// character/data.go

// LevelEntry records one level this character has taken. Append-only.
// It holds the INPUTS to that level, never the effects: what a level granted
// is derived from the entry plus the current rules, so a corrected rule
// corrects every character. See design §7.1.
type LevelEntry struct {
    // Level is the character level this entry produced. Entry n has Level n+1.
    Level int `json:"level"`

    // ClassID is the class the level was taken in. Always equal to the
    // character's class until multiclassing exists (R2.4).
    ClassID classes.Class `json:"class_id"`

    // HitPointGain is the hit points this level added, and the method that
    // produced it. Stored because a roll cannot be re-derived.
    HitPointGain int                `json:"hit_point_gain"`
    HitPointMethod HitPointMethod   `json:"hit_point_method"`

    // Choices are the choices this level required, if any. Empty for a level
    // that requires none — which is every level 2 of the four SRD classes.
    Choices []choices.ChoiceData `json:"choices,omitempty"`
}

type HitPointMethod string

const (
    HitPointMethodMax     HitPointMethod = "max"     // level 1 only: full hit die
    HitPointMethodRolled  HitPointMethod = "rolled"
    HitPointMethodAverage HitPointMethod = "average"
)
```

added to `character.Data`:

```go
// Levels is the record of how this character reached its current level, in
// order. Levels[0] is level 1. This is the source of truth for the
// character's level; Data.Level is a projection of it (R2.2).
Levels []LevelEntry `json:"levels"`
```

### 2.2 Rules

- **R2.1** `Levels` MUST be append-only. No operation in this design removes or edits an
  entry.
- **R2.2** `Data.Level` and `Data.ProficiencyBonus` are **projections**. `ToData` MUST
  write them from the record. No toolkit code may read them as truth; the runtime derives
  level from the record.
- **R2.3** `LoadCharacter` MUST fail, loudly, when `len(Levels) != Level`. Two
  representations that can disagree must not disagree silently.
- **R2.4** Every entry's `ClassID` MUST equal the character's `ClassID`. A differing value
  MUST be rejected. This is the seam multiclassing will open; it is not opened here.
- **R2.5** `Levels[i].Level` MUST equal `i+1`.

### 2.3 Characters that predate the record

- **R2.6** A character loaded with `Levels == nil` and `Level == 1` MUST be given a
  synthesized level-1 entry (`ClassID` from the sheet, `HitPointMethod: max`,
  `HitPointGain` = hit die + CON modifier, `Choices` empty). It loads normally.
- **R2.7** A character loaded with `Levels == nil` and `Level != 1` MUST fail. No such
  character can exist today, and guessing at one would invent history.

### 2.4 Level 1 is a level

`Draft.ToCharacter` MUST write `Levels[0]` at creation, carrying the draft's own
`Choices` verbatim. The record is complete from the first level, not backfilled from
level 2 onward.

## 3. Grant accessors

`classes.GetGrantsForLevel(class, level)` is **cumulative** (`grant.Level <= level`) and
answers *what a level-N character has*. It is correct for creation and MUST NOT change.

- **R3.1** Add `classes.GetGrantsGainedAtLevel(classID, level)` returning only grants
  where `grant.Level == level` — *what level N adds*. The name states which question it
  answers; `ForLevel`/`AtLevel` would not.
- **R3.2** Advancement MUST use `GetGrantsGainedAtLevel`. Applying the cumulative set to
  an existing character double-grants every proficiency it already has.

## 4. The primitive

```go
// character/advance.go

type AdvanceInput struct {
    // ClassID is the class the new level is taken in.
    ClassID classes.Class

    // HitPointMethod selects how the hit point gain is produced. The toolkit
    // computes the value; the caller never supplies it (R4.3).
    HitPointMethod HitPointMethod

    // Choices are the selections this level requires. May be empty.
    Choices []choices.ChoiceData
}

type AdvanceOutput struct {
    // Entry is the record entry that was appended.
    Entry LevelEntry

    // Gained describes what this level added, for display. Derived, never stored.
    Gained GainedAtLevel
}

func (c *Character) Advance(input *AdvanceInput) (*AdvanceOutput, error)
```

### 4.1 What it does, in order

1. Validate the input (§4.2). Reject before mutating anything.
2. `characterLevel = len(c.levels) + 1` — the total, and what §5 derives from.
3. `classLevel` = the number of existing entries whose `ClassID` equals
   `input.ClassID`, plus one. **Grants are indexed by class level, never by character
   level** (§4.4).
4. `grants := classes.GetGrantsGainedAtLevel(input.ClassID, classLevel)`.
5. Validate that `choices.GetClassRequirementsAtLevel` for `classLevel` is satisfied by
   `input.Choices`, ignoring requirements already satisfied at a previous level.
6. Build features and conditions from the grants' refs via the existing
   `features.CreateFromRef` / `conditions.CreateFromRef` factories, and attach them.
7. Recompute every level-derived number: proficiency bonus from `characterLevel` (§5),
   resource maxima from `classLevel` via the existing `initializeClassResources` math,
   max hit points.
8. Append the entry, mark the sheet dirty.
9. Return what was gained.

### 4.2 Rules

- **R4.1** `Advance` MUST refuse when the character is seated in an encounter. Advancement
  is a between-run act (journey #242), and attaching a condition to a character mid-turn
  is a different problem this design does not solve.
- **R4.2** `Advance` MUST be atomic: a failure at any step leaves the character exactly as
  it was, record included.
- **R4.3** The hit point gain MUST be computed inside the toolkit, from the class hit die
  and the CON modifier. A caller-supplied number would put a game rule in the orchestrator.
- **R4.4** `Advance` MUST return `(nil, error)` or a populated output. Never `(nil, nil)`.
- **R4.5** `Advance` MUST reject a level for which the class has no grant entry **only if**
  that level also requires choices it did not receive. A level that legitimately grants
  nothing is a valid level, not an error — several classes have them.

### 4.3 What is NOT added

- No `SetLevel`. Level moves only by appending a record entry.
- No `AddFeature` as public surface. Features arrive through a grant, never loose.
- No un-advance, no respec, no level-down.

### 4.4 Class level and character level are different numbers

Kirk, 2026-09-15: *"I think it will even work with multiclassing when we get there."* It
does, and this rule is what makes that true rather than accidentally true.

Today R2.4 forces every entry to the same class, so the count of entries in that class
and the total count are the same number, and either would compute correctly. Writing the
distinction down now costs one variable; discovering it later means auditing every
level-derived number in the engine.

- **R4.6** Grants and class resources MUST be indexed by **class level** — the number of
  entries in that class. A fighter's Action Surge arrives at his second *fighter* level,
  not his second level.
- **R4.7** Proficiency bonus MUST be derived from **character level** — the total number
  of entries, regardless of class. This is the 5e rule and the reason the two numbers
  cannot be collapsed into one.

Nothing here builds multiclassing. It only refuses to write down a falsehood that
multiclassing would later have to unpick.

## 5. Proficiency bonus

- **R5.1** The proficiency bonus MUST be derived as `2 + (level-1)/4`, not stored as
  truth. Today it is the literal `2` written at creation and never recomputed — invisible
  while every character is level 1, wrong the moment one reaches 5.
- **R5.2** `Data.ProficiencyBonus` remains on the struct as a projection (R2.2) so the
  wire and existing readers are unaffected.

## 6. Content this slice lands

Rung 1 proves the primitive with the smallest true content:

- **R6.1** Fighter gains `Grant{Level: 2, Features: [dnd5e:features:action_surge]}`.
  Action Surge is already implemented, routed in the factory and loader, and has a
  resource key and status-catalog entry. Nothing new is written for it.

Deliberately **not** in rung 1, though all are one data row each once the primitive
exists: Barbarian Reckless Attack, the Monk Ki trio and Unarmored Movement. They are
listed so it is clear they are cheap, not so they are done at once.

Not in this design at all, because they are absent code rather than absent data: Rogue's
Cunning Action and Barbarian's Danger Sense. Each is an ordinary feature build, and
Cunning Action composes only things that already exist.

## 7. Rulings

### 7.1 Inputs, not effects — RULED 2026-09-15

Kirk, 2026-09-15: *"I would definitely want a record of each level and what changed."*

This design stores **inputs** (level taken in class C, hit-point method and result,
choices made) and **derives** "what changed" from them plus current rules.

The alternative — storing the effects directly — reads better and rots: correcting a rule
leaves every existing record a permanent account of something that should not have
happened, with no way to reconcile it against the sheet. Inputs replay; effects cannot be
replayed backward into inputs. The stored hit-point result is the one exception, stored
because a roll cannot be re-derived — the same call already made for authored wall runs.

Kirk, 2026-09-15, confirming: *"Def inputs."* **Ruled. This is the shape.**

### 7.2 Not adopted: `CHOICE_SOURCE_LEVEL_UP` — RULED 2026-09-15

The wire already declares `CHOICE_SOURCE_LEVEL_UP = 6` beside `RACE`, `CLASS`,
`BACKGROUND` and `PLAYER`. Those are **provenance**; a level is **when**. A choice made at
level 2 that comes from your class can only be tagged one of the two, and either tag
loses the other fact. The record entry carries the when and `ChoiceSource` keeps carrying
the where-from. No toolkit `SourceLevelUp` is added.

Kirk, 2026-09-15: *"there is probably a lot of older style in here — that level up proto
seems like it was left over from the data driven choices stage."* The history agrees. The
value entered in `map to the toolkit's new structure` (rpg-api-protos#74, 2025-09-13), the
same commit that created `choices.proto`, and in the year since **no code in rpg-api or
rpg-dnd5e-web has ever referenced it**. It is vocabulary from a superseded era, not a
decision this design is overturning.

**R7.1** `CHOICE_SOURCE_LEVEL_UP` SHOULD be marked `[deprecated = true]` so the next
consumer does not adopt it — additive, never an in-place change. That is a protos change
with its own PR and is **not** part of rung 1; it is recorded here so the reason survives.

## 8. Rungs

**Rung 1 — this design.** Toolkit only: the record, `GetGrantsGainedAtLevel`, `Advance`,
derived proficiency bonus, Fighter level 2. Proof is a toolkit integration test: a level-1
fighter advances, gains Action Surge, spends it, persists, reloads with the feature and
the record intact.

**Rung 2 — the walk.** A level on the wire and a way to trigger it, so a fighter can be
levelled and played in the local stack. Needs: a level field on the creation/requirements
contract, and a level-up door on `CharacterService` (there is none — after `FinalizeDraft`
the only character mutations on the wire are the four equipment RPCs). This is where
journey #242's between-run flow lands. **Rung 1 cannot be walked; its evidence is a test.**

**Rung 3 — the rest of level 2.** Barbarian, Monk data rows; Cunning Action and Danger
Sense as feature builds.

## 9. Done when

1. `character.Data.Levels` persists, round-trips, and a character reloaded from storage
   reports the same level, features, resources and record it had before.
2. A pre-record character loads and gains a synthesized level-1 entry (R2.6); a
   `Levels == nil, Level != 1` character fails to load (R2.7).
3. `GetGrantsGainedAtLevel(Fighter, 2)` returns exactly the Action Surge grant, and
   `GetGrantsForLevel(Fighter, 2)` still returns the cumulative set.
3a. Grants and resources are computed from class level and the proficiency bonus from
   character level (R4.6, R4.7), with a test that would fail if the two were swapped —
   they are equal today, so the test must assert the derivation, not the value.
4. A level-1 fighter that calls `Advance` reaches level 2, holds Action Surge as a usable
   feature with its resource, and has a two-entry record.
5. `Advance` refuses inside an encounter (R4.1) and leaves the character untouched on any
   failure (R4.2).
6. Proficiency bonus is derived; a synthetic level-5 character reports +3 rather than +2.
7. No behaviour change for any existing level-1 character.

## 10. Out of scope, with the seam named

Multiclassing (R2.4 and R4.6/R4.7 name the seam; a level taken in another class is another entry) ·
XP and its persistence (journey #242; the field exists on the wire and is never written) ·
ability score improvements and feats at level 4 (the record gives them a home) · spell
slot progression for casters (`classes.Data` pins slots, cantrips and spells known as
level-1 scalars; the wire's `SpellSlots` is deprecated and its replacement excludes slots)
· levelling down or respec · average-versus-rolled as a product default (both methods
exist; which one the game uses is not decided here).

## 11. Bugs on this seam, not fixed here

- **Ranger's fighting style is offered at level 1.** It is a level-2 feature;
  `getRangerRequirements()` returns it from the base requirements so it appears at every
  level (`character/choices/requirements.go`, `TODO(#306)`).
- **rpg-toolkit#1760** — the class catalog offers subclass options a level-1 character
  cannot choose. The same seam from the creation side.

— cross-team agent, on behalf of KirkDiggler
