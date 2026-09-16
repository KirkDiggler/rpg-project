# The level-up system — design

**Date:** 2026-09-16
**Status:** Design, **BUILDING** — Kirk 2026-09-16: *"let's get started on the proper
foundation for leveling up."* Amended the same day after fact-finding against the live
code; every amendment is logged in §11 with the fact that forced it.
**Foundation:** `ideas/characters/advancement/design.md` (rung 1, built as rpg-toolkit#1766,
shipped as dnd5e v0.173.0)
**Initiative:** rpg-project#231 · **Journeys:** rpg-project#242, #241
**Scope:** the system. Class content is configuration and is named, not designed, here.

Facts below are cited against rpg-toolkit `c59189f2`, rpg-api `dev` at `f5ecaad6`,
rpg-api-protos `a490b6a`, rpg-dnd5e-web `dev` at `3da02a55`; toolkit paths are relative
to `rulebooks/dnd5e/`.

---

## 1. What this is, and what it is not

> *"technically I want the level up system built and what is filled in is available …
> we can configure it however we want so we don't need to focus on that. we are building
> a game here not showing things off."*
> — Kirk, 2026-09-16

The deliverable is **a system that levels a character up**: hold experience, cross a
threshold, be offered the choices that level actually requires, make them, and come out
the other side with the character the rules say you should have.

Which features a class gets, and at which level, is **configuration**. This document
names the configuration we start with and designs the machinery that reads it. A class
table is not a feature of this design; it is input to it.

**The test of this design is not that a bard levels.** It is that a class nobody has
written yet levels correctly the day someone fills its table in, without touching the
screen, the API, or the engine.

> *"I restarted the session with you because the last one was in the trying to make it
> work path and every time we do that these holes show up. we are not here to make
> things work."* — Kirk, 2026-09-16

That is the standard this design is held to. Where a shortcut would have made the walk
work, the shortcut is named in §8 as the thing we did **not** build.

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

Two surfaces must answer *"what does level N bring?"* Today only one can — and a third
surface must *apply* the answer, and cannot.

**Grants are data indexed by level.** `classes.Grant` carries a `Level`, and rung 1 added
`GetGrantsGainedAtLevel(class, level)` returning exactly that level's grants. Fighter's
level-2 Action Surge is one table row (`classes/grant.go:136`). It is the **only** grant
row above level 1 in the repository.

**Requirements are code, and level-blind.** `getBaseClassRequirements(class)`
(`character/choices/requirements.go:209`) takes no level. `GetClassRequirementsAtLevel`
(`:192`) is that plus a subclass requirement gated on `SubclassLevel`. **The only field
of `Requirements` that varies by level is `Subclass`.**

The consequence, verified by running `GetClassChoiceIDsGainedAtLevel` for all twelve
classes at levels 1–5: **above level 1 the only thing any class ever gains is its subclass
choice, at exactly one level.** Bard at 2: nothing. Monk, paladin, ranger, sorcerer and
warlock gain nothing at any level above 1, ever (their `SubclassLevel` is 0).

A level-up screen driven by that surface would correctly render an empty form forever.

### 3.1 The third hole: progression is a level-1 scalar

`classes.Data` carries `CantripsKnown`, `SpellsKnown` and `SpellSlots`, each commented
*"At level 1"* (`classes/data.go:27-29`). There is no per-level table. Spell slots are one
resource key, `spell_slot_level_1` (`resources/keys.go:54`, the only slot key that
exists), sized from the constant `classData.SpellSlots[0]` with no level input
(`character/draft.go:2157`), and only bard and cleric ever receive a pool
(`buildClassResources`, `draft.go:2076`). A level-2 bard's `resizeClassResources` computes
a gain of `2 − 2 = 0` and leaves the pool alone. Wizard, druid, sorcerer and warlock have
slot data and never get a pool at all.

On the wire it is worse: `Character.spell_slots` is `[deprecated = true]`
(`character.proto:225`), nothing reads it, and `ResourceView` explicitly excludes spell
slots (`v1alpha2/encounter/types.proto:441`).

### 3.2 The fourth hole: a level-up choice is recorded and never applied

`Advance` validates a choice is *present* and writes it into `LevelEntry.Choices`.
`buildGranted` (`advance.go:320`) reads only grants; `commitLevel` (`:423`) appends the
entry and nothing else. **A spell chosen at level-up would reach the record and nowhere
else** — not `knownSpells`, not the sheet, not the cast menu. Creation compiles its choices
into the character by category (`compileKnownSpells` at `draft.go:1233`, `compileSkills`
at `:1041`, the fighting-style condition at `:1466`). Advance runs none of them, and
nothing anywhere reads `LevelEntry.Choices` back.

### 3.3 The fifth hole: the level-up check is presence-only

`checkLevelChoices` (`advance.go:281`) builds a set of supplied choice IDs and checks each
required ID is in it. It does not check the count, the options, or the category. Three
off-list spells under the right ID would pass. Creation validates through
`draft.ValidateChoices()`; advancement has no equivalent.

## 4. The shape

### 4.1 Requirements become level-indexed data

- **R4.1** A class's requirements MUST be authored **per level, as the level's gained
  requirements** — the same shape grants already are: a level-tagged row that says what
  this level *adds*. Not a cumulative total per level, and not a function. Level 1's row is
  today's `get<Class>Requirements()` body, moved.
- **R4.2** *(amended, see §11)* `choices.GetClassRequirementsGainedAtLevel(class, N)` is
  the primitive: it returns row N, with any derived requirement (R4.6) folded in.
  `GetClassRequirements(class)` — what creation and the API's `ClassInfo` builder call —
  keeps its meaning and is row 1 plus the level-1 subclass. `GetClassChoiceIDsGainedAtLevel`
  becomes row N's `ChoiceIDs()`. **The cumulative-above-level-1 function is removed**: its
  only caller was the delta, nothing asks for a level-N total, and a fold would have to put
  rogue's level-1 and level-6 expertise into one singular field, which cannot be done
  without clobbering one of them. A function that cannot represent its answer should not
  exist rather than return a wrong one.
- **R4.3** The subclass requirement stays derived from `SubclassLevel` rather than being
  restated per class in the new table. One fact, one home.
- **R4.4** Every existing level-1 requirement MUST come through the new table unchanged,
  **choice IDs byte-identical**. Creation is the regression test: if a level-1 character
  can still be built choice for choice, the table is faithful.
- **R4.4a** A requirement's identity includes the class level it is gained at:
  `<class>-<kind>-<classLevel>`. This convention already exists — `rogue-expertise-6`,
  `bard-expertise-3`, `bard-expertise-10` (`choice_ids.go:37-39`, unreferenced) — and the
  existing `bard-spells-1` / `bard-cantrips-1` constants are hereby read as "gained at class
  level 1", which leaves every level-1 ID unchanged. The record keys a level's choices by
  this ID; two levels of one class MUST NOT share one (a table test asserts it).
- **R4.4b** `Advance` MUST validate a level's choices the way creation validates a draft —
  count, options, category — through the same validator, against row N. Presence is not
  validation.
- **R4.4c** `Advance` MUST **apply** a level's choices through the same compilers creation
  uses, by category: a spell reaches `knownSpells`, a cantrip `knownCantrips`. A category
  advancement cannot yet apply is **refused before any mutation**, the precedent being
  `checkGrantsApplicable` (`advance.go:241`): failing closed and loudly beats a sheet that
  is quietly missing half a level. This wave applies spells and cantrips; skills,
  expertise, fighting style and the rest refuse until a level that asks for them arrives
  with its use case.

### 4.2 Progression becomes a per-level table

- **R4.5** *(amended)* `CantripsKnown`, `SpellsKnown` and `SpellSlots` are **replaced** by
  a per-level progression table, one row per class level. The level-1 scalars do not stay
  alongside it — a narrow copy of a wide fact is a second source of truth. Level 1 reads
  row 1 and reads the same values it reads today; the API's `SpellcastingInfo` reads row 1.
- **R4.6** *(amended)* A spell or cantrip requirement is **derived from the progression**,
  never authored as a separate row: at class level N where `known(N) > known(N−1)` the
  engine emits one requirement with `Count = known(N) − known(N−1)`, `SpellLevel` the
  highest slot level with a non-zero slot at N, `Options` the class list filtered to what
  this build can cast (the `spells.Castable` filter creation already applies), and the ID
  from R4.4a. One fact, one home: the table says four then five, and "choose one" follows.
  A slot increase is not a question and is applied without asking.
- **R4.6a** Slot pools are sized **from the table by class level** for every class whose
  table has slots; the per-class switch case in `buildClassResources` goes. Visible
  consequence, stated so it is a decision and not a surprise: wizard, druid and sorcerer
  gain the slot pools they never had (they have slot data and no pool, so casting for
  them was silently impossible). Warlock's table stays empty this wave — Pact Magic resets
  on a short rest and is a different rule, named in §8. Slot keys exist for every spell
  level the authored tables reach.
- **R4.7** *(amended)* The engine reports what a level changed in every pool:
  `GainedAtLevel` gains a resources delta (key, from, to), so the level-up response can
  say "1st-level spell slots 2 → 3" and Kirk can see it on the confirmation. The sheet's
  *persistent* slot display is not built here: `ResourceView` excludes slots by design and
  a spellcasting view is its own slice (§8).

### 4.3 Experience

> *"api should never be able to grant xp, that is internal toolkit choices. it should be
> read only over the wire."* — Kirk, 2026-09-16

- **R4.8** Experience is **cumulative and never debited.** The total only grows. There is
  nothing in the toolkit today that awards it (a repo-wide grep finds no field, table or
  function), and this design adds no mutator: the first in-toolkit source of experience —
  an encounter's end, a milestone — brings `AddExperience` with it and with this rule.
- **R4.9** Level **entitlement** is derived from the total by a threshold table the
  toolkit owns (2014 PHB p.15: 0, 300, 900, 2 700, 6 500, … 355 000). Entitlement is not
  level: it is what the character *may* take.
- **R4.10** The character's actual level is `len(Levels)` — the record from rung 1.
  **The gap between entitlement and the record is the "level up available" signal.**
  No flag, no stored state, nothing to keep in sync; and journey #242's "an eligible
  character may keep playing without levelling" falls out for free.
- **R4.11** `Advance` MUST refuse a level the character is not entitled to, naming the
  total and the threshold. That is a game rule and the toolkit owns rules; the same check
  in the orchestrator would be logic in the API.
- **R4.12** *(amended)* There is **no bypass and no RPC**. Experience is **read-only over
  the wire**: it reaches the client as the total plus the derived entitled level, and no
  service call writes it. Seeding a character for a walk means writing experience on the
  persisted sheet through the fixture tool, the way every fixture is written — not
  through the served API, which has no code path that writes it.
- **R4.12a** Load does not re-validate entitlement. `Advance` enforces the rule at the
  moment a level is taken; a later change to the threshold table must not refuse every
  stored sheet that was legal when it was written.

### 4.4 The screen

- **R4.13** The level-up screen MUST render whatever requirements the toolkit returns for
  that level, and MUST contain no class-specific branch. It is the creation screen's
  choice renderer (`ChoiceRenderer` over the v1alpha1 `Choice` proto) pointed at a delta
  instead of a total.
- **R4.14** *(amended)* A level that requires nothing MUST still be a correct screen — a
  confirmation, not an empty form. A confirmation has content: what the level brings
  (features gained, hit points by the chosen method, pools that grew), taken from the
  same response, so a fighter reads "Level 2: Action Surge" and not a bare button.
- **R4.15** The client sends the choices it was asked for and nothing else. Validation is
  the engine's (R4.4b).
- **R4.16** The screen is its own view, reached from the sheet when entitlement exceeds
  level. It MUST NOT depend on the creation draft context: there is no draft — the level
  is one atomic call (rung 1, R4.2) and the screen holds its own state until then.

## 5. The configuration we start with

2014, per R2.1. **Named, not designed** — each is a table row once §4 lands, and what is
already built is noted so the cost is visible.

| class | level 2 | choices it asks | built? |
|---|---|---|---|
| Fighter | Action Surge | — | **shipped** (#1766) |
| Barbarian | Reckless Attack, Danger Sense | — | Reckless Attack built; Danger Sense absent |
| Monk | Ki, Unarmored Movement (+ Flurry / Patient Defense / Step of the Wind) | — | all built |
| Rogue | Cunning Action | — | absent |
| Bard | Jack of All Trades, Song of Rest, spells known 4→5, slots 2→3 | **1 spell** (derived, R4.6) | both features absent |

Bard is the only one of the five that asks a question, which is why it is the proof case:
the four martial classes prove R4.14 (ask nothing, correctly) and bard proves R4.13 and
R4.4c (ask, apply).

Bard's **Expertise is level 3 in 2014** and stays there (R2.2), arriving with the Bard
College — which is **rpg-toolkit#1767**, the subclass that cannot be expressed as a
choice. Level 3 is therefore the next wave, not this one, and it is gated on #1767 for
every class, not just bard.

**Content is not a gate on this design.** Per Kirk: *"what is filled in is available."* A
class whose level-2 features are absent still levels correctly — it gains its hit points,
its record entry, and whatever its table does carry.

## 6. Ownership

- **Toolkit (`rulebooks/dnd5e`, one module, one PR)** owns the thresholds, the per-level
  tables, what a level requires, what it grants, what a chosen option does to the sheet,
  and whether an `Advance` is legal. `cloneCharacterData` in resolution starts from a
  struct copy (`resolution/long_rest.go:107`), so a new scalar rides through; no
  resolution or session change is needed. The field-by-field surfaces are `ToData`
  (`character/character.go:1094`) and `loadSheet` (`load.go:268`), both in this module.
- **rpg-api** stores and projects. Persistence is the whole `character.Data` as a JSON
  blob (`internal/entities/character.go:9`), so the new field round-trips with no
  repository change. It projects experience and entitlement read-only on the v1alpha1
  `Character` (the sheet's message, `converters.go:1182`, which today never writes
  `experience_points`). Its level-up write follows the `EquipItem` shape
  (`orchestrators/character/orchestrator.go:888`: load, toolkit verb, project, save) with
  the repository's `Update`, not the narrow equipment patch. The character orchestrator
  has no toolkit `dice.Roller` today; it gets one in its `Config`, supplied explicitly the
  way the session orchestrator's is (`orchestrators/session/orchestrator.go:207`), never
  defaulted. The served API has **no** write path for experience.
- **Web** renders what it is handed and sends back the choices it was asked for. The
  renderer exists and is generic (`src/components/ChoiceRenderer.tsx`,
  `components/choices/EnumChoice.tsx`, `EquipmentBundleChoice.tsx`); the packing to
  `ChoiceData` exists (`src/utils/choiceConverter.ts`). The sheet header already renders
  `experiencePoints` (`character/sheet/components/CharacterHeader.tsx:137`) and has shown
  0 since it was written.
- **Protos** carry the contract, §10, and merge first. `CHOICE_SOURCE_LEVEL_UP`
  (`choices.proto:34`) has zero readers and zero writers in either consumer; it is a
  leftover, marked deprecated and not built on. `GetRequirements` / `SubmitChoices`
  (`character.proto:60,63`) are likewise dead — no handler, no caller — and are not the
  level-up path.
- **The walk fixture** is `cmd/sandboxseed`, which `dev-env.sh` builds from the selected
  API source and runs on every `up` of a disposable environment (`dev-env.sh:263-303`).
  It creates through the production creation RPCs (so feature blobs are real, which a
  hand-authored sheet cannot be — `load.go` reconstitutes from blobs, never from grants),
  and then writes experience on the persisted sheet through the repository it already
  holds a Redis address for. That is fixture persistence, not the served API (R4.12).

## 7. Done when

1. A class's per-level requirements and progression are **data**. Moving a feature between
   levels, or between editions, is a table edit with no code change (R2.3).
2. Creation is unchanged — every level-1 character builds choice for choice as before,
   with byte-identical choice IDs (R4.4).
3. A character holds experience; entitlement is derived; the gap between entitlement and
   the record is what the client is told, read-only (R4.10, R4.12).
4. `Advance` refuses a level the character has not earned, with no bypass and no RPC that
   could grant one (R4.11, R4.12).
5. A **bard** at 300 XP is offered its spell choice on a screen with no bard in it,
   chooses, and comes out at level 2 with the spell **on its known list**, three first-level
   slots reported in the response, and a two-entry record (R4.4c, R4.6, R4.7).
6. A **fighter** at 300 XP is offered a confirmation that names Action Surge, takes it, and
   comes out with Action Surge (R4.14).
7. A freshly created character shows 0 of 300 and no prompt — the true state of a game
   that awards no experience yet.
8. Seeding a levelled character means seeding experience on the persisted sheet, not
   writing a level and not calling anything (R4.12).
9. A class whose table is empty at level 2 still levels correctly.
10. Every derivation is mutation-tested: swapping class level for character level, or
    known(N) for known(N−1), breaks a test.

## 8. Out of scope, seams named

Level 3 and subclass-as-a-choice (**#1767**, gates every class) · multiclassing (R2.4 of
the rung-1 design names the seam) · ability score improvements and feats at level 4 (the
record and the row table both have room; the web has no generic ASI picker,
`AbilityScoresSection.tsx` is hand-rolled) · **what awards experience and how a party
splits it** (#242's shelf — this design consumes a total and adds no way to produce one;
that is where `AddExperience` will arrive) · the 2014 bard's optional known-spell swap on
level-up (a different shape: an optional choice with a minimum of zero) · a spellcasting
view that shows slots on the sheet persistently (`ResourceView` excludes them by design) ·
warlock Pact Magic's slot table and short-rest reset · prepared-versus-known casting (#445)
· levelling down or respec.

**Not built, deliberately:** an `AwardExperience` RPC and a dev button to grant XP so a
character created on the walk could level. Both would have made the walk work; both let
the API decide a game quantity (Kirk's ruling, §4.3). The walk levels seeded fixtures.

## 9. Known blockers

- **rpg-toolkit#1767** — a subclass is the one requirement the engine can ask for and
  cannot receive. Gates level 3 for every class.
- **rpg-toolkit#1769** — Action Surge can never be activated; no caller supplies an
  `ActionEconomy`. Does not block levelling, but a fighter who levels cannot use what he
  gained.

## 10. The wire shape (rpg-api-protos, merges first)

All on `dnd5e.api.v1alpha1` `CharacterService`, because that is where the choice
vocabulary (`Choice`, `ChoiceData`) and the sheet's `Character` live, and the screen is
built from components that speak it. Additive only; nothing renamed or retyped.

- `Character` gains `entitled_level` and `next_level_threshold`, both derived by the API
  from the toolkit, read-only. `experience_points` (field 4, exists) is finally written.
  Zero values are truthful: a loaded character is always entitled to at least 1, and a
  next threshold of 0 means there is no next level.
- A **read** RPC for the character's next level: given a character id, returns the level
  it would take, the `Choice`s that level requires (the same message creation renders),
  the features it grants, and the hit die — everything the screen needs to be either a
  form or a confirmation, with no class in it.
- A **write** RPC, the level-up itself: character id, a hit-point method (`ROLLED` or
  `AVERAGE`; the level-1-only `MAX` is not on the wire), and the `ChoiceData` it was asked
  for. Returns the character and what was gained (level, hit points, features, pools that
  changed). One call, atomic, no draft.
- `CHOICE_SOURCE_LEVEL_UP` gets `[deprecated = true]`. Level-up choices carry
  `CHOICE_SOURCE_CLASS`, because the record entry already says which level they belong to
  (§7.2 of the rung-1 design).
- No RPC writes experience.

## 11. Amendments after fact-finding (2026-09-16)

Each entry names the fact that forced it, so the change is a consequence and not a mood.

- **R4.2 rewritten; cumulative-above-1 removed.** Fact: the only caller of
  `GetClassRequirementsAtLevel` above level 1 was `GetClassChoiceIDsGainedAtLevel`; the API
  calls `GetClassRequirements` (level 1) and `GetClassRequirementsWithSubclass` at the
  subclass level, which the #990 guard limits to cleric at 1. A fold would clobber a
  recurring singular kind (rogue expertise at 1 and 6).
- **R4.4a added.** Fact: no choice ID anywhere is composed from a level; all are string
  literals. But the orphaned `rogue-expertise-6` / `bard-expertise-3` / `bard-expertise-10`
  already carry the class-level suffix, so the rule was latent in the code and level-1 IDs
  survive it unchanged.
- **R4.4b, R4.4c and §3.2, §3.3 added.** Fact: `checkLevelChoices` is presence-only and
  `commitLevel` only appends the entry; nothing applies a level-up choice. Without these
  two rules the bard proof case would pass every toolkit test and put no spell on the sheet
  — the same shape as Action Surge shipping dead (#1769).
- **R4.5 changed from "become" to "replaced".** Fact: `Data.CantripsKnown` etc. are
  commented "At level 1" and read in two places (`addStartingSpellSlots`, the API's
  `SpellcastingInfo`). Keeping them beside a table is the narrow-field hazard.
- **R4.6 made derived, not authored.** Fact: authoring "choose 1 spell" at level 2 next to
  a table that says 4 → 5 is two homes for one fact that can disagree.
- **R4.6a added.** Fact: `buildClassResources` sizes slots from `SpellSlots[0]` for bard and
  cleric only, and a level-2 bard's pool grows by zero.
- **R4.7 made concrete.** Fact: no wire carries slots and the dock's `ResourceView` excludes
  them by design; the response reporting the delta is the minimum true thing.
- **R4.8 / R4.12 / R4.12a and §8's "not built" — Kirk's ruling.** The plan carried an
  `AwardExperience` RPC and a dev XP button for the walk. Fact: nothing in the toolkit
  awards experience, so the only consumer of a mutator would have been the API.
- **§6 corrected.** The summary carried into this session said a seed generator and a
  level-2 Arthur were committed. Fact: rpg-api contains no call to `Advance`; #993 is
  fixtures only; Arthur was produced by a throwaway program in the job's tmp directory and
  `local--advancement` is the environment slug, not a seeded player. The fixture path is
  `sandboxseed`, which the walk env rebuilds and reruns on every `up`.
- **R4.16 added.** Fact: the web has no router; views are an `AppView` ternary chain in
  `App.tsx:481-566`, and creation is a single-page sheet bound to `CharacterDraftContext`.
  A level-up screen inside that context would need a draft that does not exist.
- **#1770 removed from §9.** Fixed by rpg-toolkit#1773 and the dnd5e v0.173.0 line.
- **Ranger's known-spells column is the real 2014 one, not a placeholder** (built as
  rpg-toolkit#1781, ruled on the builder's question). Ranger is a known caster, not a
  prepared one, so the placeholder rule for cleric, druid and paladin does not fit it.
  Consequence, accepted: a ranger taking level 2 is asked for two spells, this build can
  cast none from its list, and `Advance` refuses with *"requires choosing from the spells
  ranger knows, and this build has none"* — the same refusal a wizard meets at level 3
  for second-level spells. That is R4.4c's fail-closed shape, not a bug: a ranger who
  levelled and learned nothing would be a sheet quietly missing half a level. The cure is
  castable ranger spells, a content table, not a code change. Neither class is a proof
  case of this wave.
- **Every class's spell list above spell level 1 is empty in this build** (#1781, the
  builder's note). So every caster reaches an unanswerable level the moment its table
  asks for a spell at a level it has no content for: bard, wizard and sorcerer at 3,
  ranger at 2. The refusal names the class and the spell level; the cure is the class
  spell table (`choices.classSpellOptions`), a content edit. And the subclass wall
  (#1767) fires *before* the spell question for wizard at 2 and cleric at 1, so a wizard
  cannot reach its spellbook question until a subclass can be expressed as a choice.
  Neither changes this wave's proof cases; both are why the proof cases are what they are.
- **Walk finding 1 (2026-09-16, headless render check before Kirk's walk): a level-up
  offered, and `Advance` accepted, a spell the character already knew.** The bard fixture
  knowing bane / thunderwave / dissonant whispers / command was offered all four again;
  taking level 2 with bane returned no error and left the known list holding bane twice,
  with the choice in the append-only record. Cause: `GetClassRequirementsGainedAtLevel`
  is class-and-level only, so it cannot subtract what a character knows, and nothing in
  `Advance` checked. Every toolkit test passed. Rulings, fixed on #1781:
  - **R4.4d** `Advance` MUST refuse a spell or cantrip the character already knows,
    before any mutation, naming the spell.
  - **R4.4e** The requirements a character is *offered* for its next level are a
    character-aware view — row N with already-known spells and cantrips removed from the
    options — exposed on `Character`, and that is what rpg-api's `GetNextLevel` projects.
    The class-and-level function stays for creation, where nothing is known yet.
  - **R2.8 (load)** A stored sheet whose known-spell or known-cantrip list holds a
    duplicate is refused at load, not repaired — the discipline `parseSpellRefs` already
    applies to a spell this build cannot read. Repair would hide a level taken wrongly,
    and the record cannot be corrected anyway.
  - **R4.4f** The character-aware view removes *options*, never the *count*. How many
    spells a level teaches is the class's rule, not a property of who is taking it; a
    level whose remaining options cannot satisfy its count is refused as unanswerable
    rather than quietly teaching fewer than the table says (the builder's call on #1781,
    accepted). Not reachable today — no class both derives above level 1 and hands its
    whole list over — but that is the behaviour if it ever is.
  Built as `(*Character).NextLevelRequirements()`; every refusal a level can make now
  happens before the hit die is rolled, not merely before the sheet changes.
- **Walk finding 2 (2026-09-16, from the per-class fixture set on rpg-api#995): a ranger
  cannot be created at all**, by any client, with any choices. `character/draft.go`
  `getClassSubmissions` builds the fighting-style submission with the fighter's choice id
  for every class (its own comment: "Would need mapping for other classes"); a ranger's
  requirement is `ranger-fighting-style`, the validator never sees it answered, and
  finalize refuses forever. Ranger is the only class hit because it is the only
  non-fighter with a level-1 fighting style. Nobody had created a ranger before a fixture
  tried every class. Fixed on #1781: the submission carries the requirement's own id, no
  constant, no per-class map.
- **Walk finding 3 (same source), logged, not fixed this wave: expertise options are
  invented in the API.** `ExpertiseRequirement` names no options, so rpg-api fills the
  wire with all eighteen skills and the engine refuses the illegal ones at finalize
  ("expertise skill animal-handling must be from a proficient skill"). A client can only
  compute the legal set by intersecting with the skills it just chose, which is a rule in
  the wrong place. The seam is the character-aware view (R4.4e): when a level asks for
  expertise (bard 3, rogue 6), its options are the character's proficient skills. Not
  reachable this wave.
- **The per-class shape at level 2, read from the real service** (rpg-api#995's
  fixtures): bard, sorcerer and wizard ask one spell; fighter gains Action Surge; the
  other seven are confirmations with nothing but a hit die. Warlock asks nothing because
  its known-spells column is deferred with Pact Magic (2014 says 2 → 3); wizard and druid
  are then refused at confirm by the subclass wall (#1767). R4.14 carries most of the
  roster, which is why the confirmation is a screen and not a button.
- **Warlock keeps its level-1 slot row, with a `SlotReset` on the progression** (#1781).
  Fact: the derivation reads the slot row to know it is asking for a first-level spell,
  so the row cannot be empty; pool sizing builds only long-rest pools, so warlock's
  behaviour is unchanged at every level and Pact Magic stays the named seam.

— cross-team agent, on behalf of KirkDiggler
