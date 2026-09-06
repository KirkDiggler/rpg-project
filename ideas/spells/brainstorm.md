# Spells, level-up, and the places they go — brainstorm

**Date:** 2026-09-06
**Status:** Brainstorm. Territory mapped against the composable stack as it stands on
`rpg-toolkit` main (`3477eb44`). No slice cut yet; the candidate first slice is in §7.
**Umbrella:** levels 1–3 — get every class from level 1 to level 3 and make the level-3
choices real.
**Journey:** none adopted. rpg-project#243 (*Cast a Spell in Play*) is prior art from the
old stack and is not carried; see §9.

---

## What prompted this

The engine is proven: two encounter systems in, the composable one pushes verbs down to the
players through Afford, the resolution machines run every fight, and the hold-out slice
landed factions and dispositions into the run's world in one wave because there was already a
place for them. Kirk asked what it would take to bring spells in, and then reframed the
question twice:

> *"we took a swing at spells very early on. we don't need to carry anything that we have
> forward ... we're not looking for what we already have. we are investigating what we can do
> now with our new components."*

> *"it is not what these shapes are doing but the shapes themselves. disposition just came in
> because there was a place for it. what we need should have a place to put it now between
> the machines, the graph and the encounter."*

And the goal it serves:

> *"our main goal is to get level 1 to 3 and be able to choose the choices that happen at
> level 3 for our characters ... if we can get magic in, we'll have more choices at level 3
> for our classes."*

So this document is a map of **places**, not features. For each thing a spell — or a level
gain — needs, it says where that thing goes today, with evidence, or states plainly that no
place exists yet. The missing places are the design work. Everything else is content.

Four read-only surveys of the toolkit, protos, rpg-api, web and this repo produced the
evidence; every `file:line` below is against `rpg-toolkit` `origin/main` at `3477eb44`
unless another repo is named.

---

## 1. The premise, checked

Two claims were on the table at the start. One held, one did not.

**"rpg-api does nothing; it is a converter on the session SDK."** Holds.
`rpg-api/internal/handlers/dnd5e/session/v1alpha1/handler.go:57-60` says every method is
proto↔SDK translation and no rule lives there. The SDK decides persistence; rpg-api has no
save, dirty or persist call in the session handlers. A new verb is one handler file beside
`attack.go`, one interface entry, and arms in `eventKindToProto`, `setEventBody` and
`verbToProto` in `convert.go`. There are 26 copies of the pattern.

**"The resolution machines can pause and interrupt."** Does not hold yet — and that is
fine, because spells are what will make it true.
`resolution/step.go:66-71`: *"No suspension yet. The requested machine runs to Done inside
this one's step loop."* The session's suspension spine (Pending, Answer, the verb freeze) was
deleted because the walk stopped posing windows and a spine with no producer is unreachable;
`session/doc.go:186-216` names wave 5 — a reaction — as the first honest producer that
re-creates it, and says the custody design lives untouched in `play/interrupt`. Shield and
Counterspell therefore do not *ride* the pause machinery; they will be its producer. They are
wave two of spells, not a head start.

---

## 2. Places that exist

Each row is a thing a spell needs, the place it goes, and the evidence that the place is real.
"Empty" means the place exists and nothing fills it yet.

| Need | Place | Evidence | State |
|---|---|---|---|
| A resolution sequence | a machine in `resolution/` yielding `Gather \| Request \| Done` over the attached cast, paid at the door | `step.go:19-96`, `resolve.go:299-427` | 7 machines live |
| Content saying which sequence it wants | a profile arm on `actions.Definition`, dispatched by `NewAction` | `combat/actions/definition.go:14-19`, `resolution/action.go:26-34`; ADR-0045 line 117 names spell content as a producer | one arm (`Attack`) |
| A price | `combat.SpendProfile` — slots, capacity, grants, pools, requires — charged all-or-none by `payAtTheDoor` before the first step | `combat/spend_profile.go:59-95`, `resolution/cost.go:148-173` | `Pools` expressible, unexercised (`spend_profile.go:37-45`) |
| A spendable pool | `Data.Resources` keyed by `ResourceKey`; `PoolLeft`/`SpendPool` read it; rest verbs refill it | `character/ledger.go:178-195`, `character.go:411-478` | five keys (`resources/keys.go:18-44`) |
| "What can I do right now" | an Afford `Declaration`: verb, slot, available, shortfall, candidates, compiled off the sheet the way attacks are compiled off equipment | `session/afford.go:36-211`, `character/attack_definition.go` | five verbs |
| A one-target attack roll | `NewStrike`; `AttackCategorySpell` already routes damage provenance to `DamageSourceSpell` | `strike.go:102`, `strike.go:501-504`, `combat/actions/attack.go:26` | category has no caller |
| A saving throw | `NewSave` (rolls) and `NewContest` (rolls, imposes a condition on failure) | `save.go:80`, `contest.go:136-216` | one saver each |
| Contestability as data | `saves.SaveGate`: abilities, closed `DCSource` set, `OnSuccess`, `Recurrence`; concentration's DC is already in the roster | `saves/gate.go:15-60`, `gate.go:115` (`DCHalfDamageFloorTen`) | `Half` and `end_of_turn` declarable, refused at `contest.go:69-74` |
| Damage with types, resistances, crits | the damage chain and `combat.FinalDamage` | `strike.go:379-600`, `combat/final_damage.go:60` | shared with monsters |
| An ongoing effect | a condition on the bus with clock subscriptions (start/end of turn, combat end, rest) | `conditions/raging.go:88,118,148`, `events/events.go:1009-1016` | 6 of 14 core conditions have behavior |
| Who is friend or foe | the cast view over the run's faction fold: `IsHostile`, `IsAllied`, `Members` | `resolution/cast.go:75-130`, `encounter.IsHostile` | live since rpg-project#375 |
| A change to who fights whom | a fact the run learns, folded by every side reader | `encounter/disposition.go`, hold-out design §4 | hostile→neutral only (R2) |
| Range | `deliveryRangeState` against the room's grid | `resolution/delivery.go:12-39` | melee + ranged |
| Shapes on the map | `GetPositionsInCircle/Cone/Line`, `GetHexRing`, room `GetEntitiesInRange`, `MembersIn(region)` | `tools/spatial/hex_grid.go:174-233`, `encounter/canvas.go:197-209`, `encounter/region.go:108` | no non-test caller |
| Duration boundaries | `NewBoundary` publishing turn topics; rounds are coordinates | `boundary.go:76-104` | live |
| Typed per-target facts | the activation effect collector: healing applied, condition applied/removed, capacity granted | `activation.go:61-124` | live for rage/second wind |
| What the client sees | beat kinds in `encounter/outcome.go`, `EventBody` in `session/types.go`, proto bodies with full roll traces | `outcome.go:31-80`, `types.go:1168-1265`, protos `events.proto:567-683` | struck, missed, healed, condition on/off, death save |
| A monster that casts | `monster.Data.Actions []actions.Definition` — a spell goes where a bite goes | `monster/data.go:36`, `monster/monsters/wolf.go:38-51` | no caster authored |
| What a character carries | features on the sheet, hand-written factory + loader switch, `AvailableAbilities` → `ActivateAbility` | `features/factory.go:65-84`, `character/action_economy.go:180-238` | 8 features |
| Spell identity | `refs.Spells` (~180 refs under `TypeSpells`) | `refs/spells.go:9-190` | names only |
| Authoring | dungeonspec refs by type; scenarios declare the fields they need | `dungeonspec/validate.go`, `encounter/scenarios` | live |

The conclusion this table supports: **a spell that attacks one target, heals one ally, or
puts one condition on one creature has a place for every part of itself today.** What it
lacks is content and one verb.

---

## 3. Places that do not exist

These are the design work. Each is named once here, with what would fill it. Nothing in this
list is spell-specific; each one is a shape the stack is missing and spells are merely the
first customer.

### 3.1 One actor, many targets

Every machine input holds one target (`strike.go:30-35`, `save.go:23-45`,
`contest.go:21-29`, `activation.go:23-59`). Every outcome describes one target. The recorded
beat has a `Targets []MemberID` slice (`encounter/outcome.go:128`), but every producer writes
one element (`session/attack.go:448`, `trade.go:237,306`) and the decoder reads element zero
(`session/events.go:1241,1249`). `TargetKind` on the wire is `NONE | MEMBER | PATH`
(protos `types.proto:425-429`); the client arms only `MEMBER` and rings only candidates
(`web/src/components/session/combat-experience/TargetSurface.tsx:42-95`).

Customers: every area spell, multiattack, breath weapons, Sweeping Attack, Great Weapon
Master cleave, healing word on a group. Monster multiattack — none is authored yet — lands on the same
shape.

What fills it: a machine that loops a per-target sub-machine and returns per-target outcomes
under one actor; a beat whose targets slice is honestly plural; a `TargetKind` for a point
and a shape; a client surface that highlights a template and confirms. The loop is the
primitive; the rest is plumbing. **Design deliberately, once, for multiattack and area
together** — they are the same loop with different target sources.

### 3.2 A condition that owns other things

`ConditionApplication` names a ref and parameters and a gate, and nothing about *whose*
effect it is (`combat/actions/attack.go:240-264`). `ConditionRemovedEvent` carries member,
ref and reason only (`events/events.go:267-271`). There is no link from a condition to its
source, and no condition that holds a link to a target or to other conditions.

Customers: concentration (caster-side condition that, on a failed concentration save, ends
the spell's conditions on every target), Hunter's Mark (caster-side condition that names one
target and adds damage to strikes against it), Dispel Magic, Hex, Bless ending when the
cleric drops it, and every "until the caster does X".

What fills it: a source binding on applications, and a condition kind that carries member
links and removes what it owns when it ends. Concentration is then a condition whose damage
subscription requests a contest at `DCHalfDamageFloorTen` and tears down its links on
failure. The DC and the contest exist; the ownership does not.

### 3.3 A saving throw the client can see

The only save on the session wire is the death save (`types.go` `DeathSaveBody`; protos
`events.proto:368-397`). A contest's save inside a strike rider is computed and never
projected: `StrikeOutcome.Conditions` exists (`strike.go:81`) and the struck beat drops it
(`session/events.go:1240-1247`).

Customers: Sacred Flame, the wolf's knockdown, ghoul paralysis, every save-or-suffer.

What fills it: a save beat (or a save arm on a per-target result) carrying roll, total, DC,
ability, success, and the imposed effect — the death save body is the template.

### 3.4 Half on success, and repeat at end of turn

`saves.Half` and `RecurrenceEndOfTurn` are in the gate's vocabulary (`gate.go:20-37`) and
refused by the contest machine (`contest.go:69-74`, `ErrRecurrenceUnsupported`). The place is
half-built: the data shape exists, the executor does not.

Customers: fireball, Hail of Thorns, hold person, ghoul paralysis.

### 3.5 Directed, reversible disposition

Dispositions are unordered faction pairs; the only reachable change is hostile→neutral once
an until holds (`encounter/disposition.go:37-39`, `:219-227`). Directed dispositions are the
hold-out design's §11 shelf.

Customers: Charm Person's *ending* (friendly, then hostile again after an hour or when
harmed), Sanctuary, Command. Charm's *start* — one Wisdom save, one fact, the camp turns —
has a place today where the author declared the until.

### 3.6 Conditions that are only names

Blinded, charmed, frightened, grappled, incapacitated, invisible, paralyzed, petrified,
poisoned, restrained, stunned, deafened: `ConditionType` strings with no behavior and no
factory case (`events/events.go:23-95`, `conditions/factory.go:70-109`). Not a missing place
— the place is the condition factory — but a missing floor under half the spell list.
Ensnaring Strike (restrained), Hold Person (paralyzed), Sleep (unconscious exists) land here.

### 3.7 The suspension producer

§1. Wave 5. Reaction spells wait on it and give it its first honest producer. Not to be
designed here beyond the note that the cast machine's phase boundaries must be yielded steps
like strike's (`strike.go:96-101`), so windows can be added without rebuilding it.

---

## 4. Level-up: the precursor, and its places

There is no level-up in the toolkit: no `LevelUp`, `GainLevel` or `AdvanceLevel` symbol
anywhere. What exists is the material level-up would read and write:

| Need | Place | Evidence | State |
|---|---|---|---|
| A character's level | `Data.Level` | `character/data.go:34` | written by the draft only |
| Per-level grants | class tables (`classes.Data`) — features, subclass level, spellcasting rows | `classes/data.go:20-35`, `SubclassLevel` rows at `:69,105,145,191,224,255,305` | spellcasting rows are **level 1 only** (`CantripsKnown`, `SpellsKnown`, `SpellSlots []int` all "at level 1") |
| Pools with a level-driven max | `Data.Resources`; ki is already gated on `level >= 2` at finalize | `draft.go:1770-1792` | maxes computed at finalize, never re-derived |
| Choices with options and answers | the choice pipeline: requirements, choice ids, `ChoiceData` | `character/choices/requirements.go`, `choice_ids.go:173-182` | creation only; cantrip/spell answers are recorded and **dropped** at `ToCharacter` (`draft.go:564-713`) |
| A verb that writes a sheet outside combat | `resolution.LongRest` — load one character on a transient surface, act, snapshot | `resolution/long_rest.go:37-105` | the template for a record-only verb |

The dependency on spells is real and runs one way: **level-up needs the class tables to be
true at levels 1–3 and needs a choice pipeline that lands its answers on the sheet.** Spells
need exactly those two things too (spells known at 2, slots growing with level, subclass
spells at 3). So the shared work is:

1. Extend the class tables from "at level 1" to rows per level for 1–3: features, cantrips,
   spells known, slots, subclass level. One table, read by the draft, by level-up, and by the
   seed. Nobody derives.
2. Make every pool's max a table read, so level gain refreshes rage, ki, hit dice and slots
   the same way. Slots become pools (§5.1) as part of this.
3. Make the choice pipeline land on the sheet, and make it run at three moments: creation,
   level 2, level 3.

Level-up itself is then a verb shaped like `LongRest`: load the sheet, read the table at
`Level+1`, refresh pools, run the choices due at that level, write. If level-up is built
first, spells inherit tables and choices that already work. If spells are built first on a
seeded level-2 character, the guard is that the seed reads the same tables — otherwise two
derivations of "how many slots at 2" appear and drift.

**Recommendation: level-up's table and choice work first, as its own slice; the verb and the
first spell can then land in either order.**

---

## 5. Rulings this asks for

### 5.1 Slots as pools (recommend) vs a slot arm on the cost

`Data.SpellSlots map[int]SpellSlotData` is a second store beside `Data.Resources`, unreachable
from the gate; toolkit#799 records it as orphaned. Two ways to make a slot chargeable:

- **Pools.** One resource key per slot level (`spell_slot_1`, `spell_slot_2`, ...), long-rest
  refill, max from the class table. The existing gate charges it, Afford prices it, the
  status view lists it, level-up refreshes it — with no new code in any of them. The old map
  is deleted.
- **A slot arm on `Cost`.** Keeps the map, teaches the gate and Afford and status about a
  second currency. Every consumer grows a special case.

Pools is the primitive below; the arm is the special case above. Recommend pools. The one
thing pools do not express is Pact Magic's short-rest refill and slot-level-as-caster-level;
that is a warlock problem and a different reset type on the same key.

### 5.2 Spells known live on the sheet as content refs

A known/prepared list is a sheet field of `dnd5e:spells:<id>` refs, written by the choice
pipeline. The Afford compiler reads it to mint Cast declarations, the way it reads equipment
to mint Attack declarations. Nothing else reads it.

### 5.3 One Cast verb, one machine, several arms

`Cast` is one verb on the session, one declaration verb, one profile arm on the definition,
and one machine whose step sequence is chosen by the spell's arm: attack (delegates to
Strike with the spell category), save-or-suffer (per target: contest, then damage or
condition), heal or buff (activation-shaped, allies), fact (writes into the run, like
Interact). Spells are content declaring which arm; nothing dispatches on spell identity
(ADR-0045's rule, `resolution/ARCHITECTURE.md:89-97`).

### 5.4 What is not carried

Every earlier swing at spells is replaced, not extended, and deleted when its replacement
lands:

- `rulebooks/dnd5e/spells/` — ~180 ids with names and prose, no mechanics (`spells/data.go:12`
  says it mirrors the proto enum). Replaced by content that declares profiles.
- `core/spells/types.go` — school, components, range vocabulary; nothing imports it.
- `rulebooks/dnd5e/effects/types.go` — dead; `Effect{Concentration bool}` and Bless/Shield
  constructors with no importer.
- `conditions/shield_spell.go` — a pre-wave-5 reaction condition that explicitly does not
  check a slot. Shield returns as wave 5's producer on the cast machine.
- `Data.SpellSlots` and `Data.ClassResources` (toolkit#799, #800).
- Legacy character protos: `SpellSlots`, `SpellcastingInfo`, `SpellInfo`, `ListSpellsByLevel`,
  the `Spell` enum — never populated by rpg-api (`converters.go:1211` TODO, `:344-352`
  placeholder). Replaced by the typed-ref vocabulary's `spells.proto` when it comes.
- Web: `SpellSelectionModal.tsx` ends in a TODO; `useSetReactionReady.ts` and
  `useTakeAction.ts` are old-stack, dev-only.
- rpg-project#243 — reshaped by this doc; close or supersede when a journey is cut.

---

## 6. The ranger as the use case

Kirk's proposal: bring in a simple caster and prove the cast shape on a solid use case. The
ranger, checked against the goal:

- Rangers get spellcasting at level 2, an archetype at 3, and nothing usable at 1. One
  character walked 1→3 touches level-up twice, spell choice, slot growth and a subclass pick.
- No cantrips, so the first spell cannot dodge slots — which is why §5.1 comes first.
- Its first spells land on the missing places in a useful order, without touching the big
  one:
  - **Hunter's Mark** — bonus action, a slot, concentration, self. A caster-side condition
    that owns a link to one target and adds d6 to weapon strikes against it (a chain
    subscriber shaped like Sneak Attack). The cleanest test of §3.2, composing with the
    strike we have.
  - **Cure Wounds** — action, a slot, touch, heal an ally. The heal arm, the cast view's
    `IsAllied`, the healing beat that already exists.
  - **Ensnaring Strike** — adds a Strength save and *restrained* (§3.3, §3.6).
  - **Hail of Thorns** — adds a burst and half-on-save (§3.1, §3.4).
  - **Fog Cloud** — an area with no targets at all; heavily obscured is a sight problem.

Each rung reuses the previous one, and the class never leaves levels 1–3.

Alternative considered: a level-1 cleric (Sacred Flame, Cure Wounds, Bless) exercises more
arms at once but skips the level-2 moment entirely and needs the save beat in slice one.

---

## 7. Candidate first slice (not cut)

**Done-when:** a ranger created at level 1 reaches level 3 on the local stack, chooses two
spells at 2 and an archetype at 3, casts Hunter's Mark and Cure Wounds in a fight, and the
client shows the slot as the price, the mark on the target, and the heal with its roll.

Modules, one PR each, bottom-up:

1. toolkit `classes` — tables per level 1–3 (features, spellcasting rows, subclass level).
2. toolkit `character` — slots as pools; spells-known field; choice pipeline lands on the
   sheet; level-up verb shaped like `LongRest`.
3. toolkit `resolution` — cast machine with heal and condition arms; owning condition with
   a target link; concentration.
4. toolkit `session` — `Cast` verb, `VerbCast` in Afford, cast beat reusing healing and
   condition bodies.
5. protos / rpg-api / web — the per-verb pattern; level-up RPC; cost badge already renders a
   pool.

Multi-target (§3.1), the save beat (§3.3), half/recurrence (§3.4) and directed disposition
(§3.5) are **explicitly not in it**, and each gets its own design section before its first
customer, not a band-aid inside one.

---

## 8. Shelf — nothing here is lost

- Multiattack and area as one loop (§3.1); the first monster with two attacks is a customer before any spell is.
- Owning conditions and concentration (§3.2); Dispel Magic follows.
- The save beat (§3.3); the wolf's knockdown becomes visible on the client the day it lands.
- Half and recurrence (§3.4).
- Directed disposition (§3.5); Charm Person's ending.
- The twelve inert conditions (§3.6); order by first customer.
- Reaction spells as wave 5's producer (§3.7): Shield on the post-attack-roll boundary,
  Counterspell on the cast declaration boundary; costed reactions default-off
  (`truth.go:107-113`) and the opt-in is a player decision.
- Monster casters: a goblin shaman is a definition with a cast arm and slot pools on a
  monster sheet; monster AI (Billy's lane) picks it like any action.
- Spells that write facts: charm, calm emotions, suggestion. Same seam as Interact.
- Audience: the world kernel's witnessing says who saw an act. A fireball is seen by the
  room; a charm by nobody but the two. Spells inherit the law when beats become per-player
  (rpg-toolkit#940).
- Components, focus, ritual casting, upcasting, prepared vs known, spellbooks, Pact Magic.
- Sight-affecting areas (Fog Cloud, Darkness) — the `Sight` capability is supplied, never
  defaulted (`resolve.go:198-216`); an area that changes it is a new customer of that seam.

---

## 9. Prior art, for the record

Read, not carried: rpg-project#243; toolkit ADR-0027 (reactions; Shield worked end to end on
the old stack), ADR-0039 (the save gate), ADR-0045 (actions are data; §92 says a fireball
"requires a different profile and machine"), ADR-0042 (Afford answers in declarations);
toolkit#300, #385, #431, #205, #799, #800, #505, #695; rpg-api#120, #121, #168;
web#95, #99. The survey reports behind this document were produced on 2026-09-06 and are
summarized in §2–§4; their file:line anchors are the evidence.
