# Spells, level-up, and the places they go — brainstorm

**Date:** 2026-09-06
**Status:** Brainstorm. Territory mapped against the composable stack as it stands on
`rpg-toolkit` main (`3477eb44`). No slice cut yet; the candidate first slice is in §8.
**Umbrella:** levels 1–3 — get every class from level 1 to level 3 and make the level-3
choices real.
**Journey:** none adopted. rpg-project#243 (*Cast a Spell in Play*) is prior art from the
old stack and is not carried; see §10.

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

## 6. Shapes — what exists, and what spellcasting asks for

Kirk's ruling, and the frame for everything below: **plan on the new places only.** The
composable `play` and `world` packages are the system — resolution machines, the door, the
bus, the world graph. If a thing exists only as a legacy file — `conditions/helped.go`,
`conditions/shield_spell.go`, Second Wind's private healing path, the dead `effects`
package, the web class allowlist — then for this plan **it is missing**, and naming it as
prior art is the most it gets.

Second ruling: **a machine exists only when we need a new shape or different steps.** Never a
spell-named machine. There is no thunderwave machine. A spell is *shapes × effect kinds*, and
if a spell seems to need its own machine, the shapes are wrong.

### 6.1 Shapes today

Every sequence the composable stack actually runs. A row is here only if a non-test caller
drives it.

| Shape | Steps | What makes it special | Callers today |
|---|---|---|---|
| Strike | Start → attack roll → damage chain → riders → Done, every phase boundary a yielded step | the machine's fields are its whole state between phases, so a window can be inserted without rebuilding it (`strike.go:96-101`) | `resolution/action.go:27` (Attack arm), `resolution/movement.go:313` (opportunity attack) |
| Save | one Gather, then Done | the smallest machine that folds a chain, and it never touches the bus — the fold happens once inside `saves.MakeSavingThrow` (`save.go:66-88`) | `resolution/contest.go:241` — the only one |
| Contest | Request(save) → outcome policy → Gather(impose) → Done (`contest.go:144-213`) | the only machine that composes another and resumes with its outcome | `resolution/strike.go:909` |
| Activation | Start(find actor and target in the cast) → Gather(run, collect typed effects) → Done | it charges **nothing** at the door on purpose: the ability spends its own slot, so a `Cost` beside it would bill twice (`activation.go:357-363`, effects `activation.go:61-124`) | `session/activate.go:228` |
| Movement | Start → Gather(announce, fold the chain) → Request(strike) per trigger → Done | the only machine that produces reaction triggers and resolves them inline (`movement.go:158-180`, `:313`) | `session/mover.go:95` |
| Boundary | Start → one Gather per crossing, sealed kind→topic lookup → Done | publishes the clock, and refuses a kind this build does not know at the door rather than publishing nothing (`boundary.go:76-110`) | `session/announcer.go:60` |
| Dispatch | not a machine: `NewAction` reads the definition's profile arm and returns one | the one place content chooses a sequence; one arm (`action.go:19-34`) | `session/attack.go:254`, `session/striker.go:98` |
| The door | `payAtTheDoor` charges a `combat.SpendProfile` all-or-none, after pure preflight and before the first step | the only place a price moves; `Pools` and `Requires` are expressible and unexercised (`cost.go:127-173`, `combat/spend_profile.go:37-45,59-95`) | every priced verb through `Resolve` (`resolve.go:299-427`) |
| Afford | compiles `Declaration`s off the sheet: verb, slot, available, shortfall, candidates | the server authors the offer and the label; five verbs, none of them a cast (`session/afford.go:36-95`) | session read path, web action dock |
| Condition on the bus | attach → subscribe (clock topics, chains) → publish removal → detach | the only ongoing effect, persisted as opaque JSON (`conditions/raging.go:88,118,148`, `events/events.go:1009-1016`) | 6 of 14 core conditions have behavior |
| World trigger | a `TriggerFact` on a disposition's `until`, folded per faction mind into graph edges | nothing stores the stance: it is derived on every question from declaration plus facts, and six of the seven trigger forms are refused on an `until` (`encounter/disposition.go:125-149`, `encounter/world.go:395-425`) | `encounter.IsHostile`/`IsAllied` → `resolution/cast.go:115-120` |
| Placement | validate content → place → reveal | placement is shared with monsters and spawning is not: `PlaceNPC` takes already-built content and never forms a fight (`session/write.go:653`), `Spawn` does (`write.go:539`) | session write path |

**What is not in this table, and why.** The pause. `Step` is sealed to `Gather | Request |
Done`; `Pose` is named in ADR-0038 and not built (`step.go:26-33`), and `Request` says so in
its own doc — "No suspension yet. The requested machine runs to Done inside this one's step
loop" (`step.go:66-71`). The custody half *is* built — `play/interrupt` has a ledger with
Pose and Answer — and it has **zero non-test importers**. So the interrupt window is a design
that exists and a shape that does not.

### 6.2 Shapes spellcasting asks for

`state` is judged against the new stack only. "Partial" means some steps exist and the shape
does not.

| Shape | Steps | What makes it special | Nearest existing shape, and the delta | First customer | Also serves | State |
|---|---|---|---|---|---|---|
| **Grant** | pay at the door → deliver to a willing target → one beat | no roll at all, because the target does not resist | Activation, which charges nothing at the door and acts on self (`activation.go:357-363`) — the delta is a price and a target who is not the actor | Bardic Inspiration | Healing Word, Cure Wounds, Bless, Aid | **partial** — heal effects and a range check exist (`activation.go:61-124`, `delivery.go:12-39`) |
| **Save-then-effect** | set a DC → target rolls → read an outcome policy (full / half / nothing) → deliver → beat | the outcome policy; today success means the effect simply never happened | Contest — the steps are already Request(save) → policy → deliver → Done. The delta is two refusals: `OnSuccess` must be `Negated` (`contest.go:70`, and again at the data layer `combat/actions/attack.go:258-261`) and recurrence is refused outright (`contest.go:69-74`) | Vicious Mockery | Sacred Flame, the wolf's knockdown, ghoul paralysis, every save-or-suffer | **partial** |
| **Fan-out** | resolve one shape once per target in a set or an area → one beat per target | N outcomes from one declaration; every machine input holds one target and every outcome describes one | none. Movement's per-trigger `Request` loop (`movement.go:313`) is the only precedent for repeating a sub-machine | Thunderwave — and the first monster with multiattack gets there first | every area spell, breath weapons, Sweeping Attack, healing word on a group | **missing** — shape queries have no non-test caller (`tools/spatial/hex_grid.go:174,185`); `TargetKind` is NONE\|MEMBER\|PATH |
| **Post-roll window** | roll → **pause** → offer a window to someone who is not the actor → read the answer → read the outcome | it is the first step that leaves the process | none. `Step` is sealed and `Pose` is unbuilt (`step.go:26-33,66-71`); the custody ledger in `play/interrupt` has no importer | Cutting Words | Shield, Silvery Barbs, Bardic Inspiration's spend, every reaction | **missing**, custody partial |
| **Concentration — an owning condition** | own N effects → on damage taken, request a CON save as Save-then-effect → on failure drop everything owned → a second concentration cast drops the first | ownership and cascade. One owner, many owned, and ending one ends all of them | a condition on the bus. `ConditionApplication` carries `Ref`, `Parameters`, `Save` and **no source binding** (`combat/actions/attack.go:240-244`), so nothing records who owns what. The DC is already in the roster (`saves/gate.go:115`) and the trigger constant exists with no driver (`events/events.go:386-387`) | any concentration spell | Hunter's Mark, Hex, Bless ending when the caster drops it, Dispel Magic | **missing** — this is §3.2, and it is the largest single piece |
| **Ritual / out-of-bubble cast** | declare with no turn and no round → pay a price that is not a slot on a turn → resolve | the action economy is the whole gate today, and it exists only inside a fight | the standing verbs (Search, Loot, Interact, Trade) run outside a bubble but carry no action definition and no price. Rounds are explicit that they exist only inside one (`encounter/field.go:1106-1113`) | Charm Person on a guard in a corridor | rituals, Detect Magic, every social spell, Song of Rest | **missing** |
| **Upcast** | choose a level at the door → charge that level's pool → the effect reads what was paid | the price is an input rather than a constant, and the effect is a function of it | `SpendProfile.Pools` (`spend_profile.go:82-84`) — expressible, unexercised, and compiled before the declaration, so nothing lets a player choose | Healing Word at 2nd | Thunderwave, Magic Missile, every scaling spell | **missing**, the pool half partial |
| **Persistent area with recurrence** | place a shape on the map → it persists → re-apply at a clock boundary → end | an effect owned by a region rather than by a member, plus a recurrence step | `saves.SaveGate.Recurrence` is declarable and refused (`ErrRecurrenceUnsupported`, `contest.go:69-74`); `MembersIn(region)` exists with no combat caller (`encounter/region.go:108`) | Fog Cloud, Web | Spirit Guardians, Darkness, authored hazards | **missing** — §3.4 plus a region-owned effect |
| **Summon / placement** | author a participant → place it → it acts | nothing about the placement; the delta is who authored the content and whether it takes a turn | `PlaceNPC` places already-built content and forms no fight (`session/write.go:653`); `Spawn` forms one (`write.go:539`) | Find Familiar | Animate Dead, conjurations, summoned swarms | **partial** — placement exists, a caster-authored participant does not |
| **Reaction to a cast** | a declaration is made → **pause** → offer a window → the declaration may not happen | the producer is the door rather than a step or a roll | Post-roll window: same custody, different trigger point | Counterspell | any declaration-time interrupt | **missing**, and there is no cast verb to hang it on |
| **Teleport** | leave → arrive, with no path between | it must produce no movement chain, so no opportunity attack reads it | Movement, which refuses `From == To` and folds a chain the mover's reactions read (`movement.go:158-180`, `session/mover.go:330`) — the delta is skipping the fold | Misty Step | Dimension Door, Thunder Step, and a push's forced displacement | **missing** |
| **Detection / illusion — a per-player beat** | resolve once → describe the same board differently to different members | audience. The wire carries one description of a beat for everyone | the grain already exists on the world side: a fact is judged on the *truth* grain or a mind's *audience* grain (`encounter/field.go:1129-1135`), and `Sight` is supplied never defaulted (`resolve.go:198-216`) | Detect Magic, Silent Image | fog of war, hidden creatures, rpg-toolkit#940 | **missing** — the grain exists, the wire does not |
| **Duration boundary** | a clock boundary is crossed → subscribed effects end | nothing; it is the one shape on this list that is finished | Boundary, publishing turn topics that conditions subscribe to (`boundary.go:76-110`) | already served | every timed condition | **exists** — with one gap: "10 minutes" and "1 hour" are not turn boundaries, and outside a bubble nothing crosses one |

### 6.3 Which kind of change each one is

Sorting the rows above by what they actually add, because most of them are not machines:

- **A new machine — two.** Fan-out, which is a loop over a target source with a per-target
  sub-machine, and the out-of-bubble cast, which is a resolution with no economy to gate it.
- **A new step, once, for two rows.** Post-roll window and reaction-to-a-cast are the *same*
  new `Step` (Pose) plus the ledger that already exists in `play/interrupt`. Built once, they
  serve both, and Teleport is Movement with a step removed rather than a machine added.
- **A new arm on a machine we have — two.** **Save-then-effect is the contest with an
  outcome policy and a delivery, not a new machine.** Its steps are already Request(save) →
  policy → deliver → Done (`contest.go:144-213`); what is hardcoded is that the policy is
  `Negated` and the delivery is a condition. Grant is the activation with a target and a
  price. Both are edits inside a machine that runs today.
- **An effect kind, not a shape.** Damage, heal, condition, push, world fact. Damage, heal and
  condition all have a delivery and a beat; push and world fact have neither.
- **A condition on the bus.** The held die, Charmed, and concentration's ownership — the last
  needing the source binding `ConditionApplication` does not carry.
- **A fact in the world graph.** A social spell's result lands on the existing fact → mind →
  disposition fold (`encounter/world.go:395-425`). There is no runtime stance API and this
  does not ask for one.
- **A field on the spend profile.** Upcast.

So: two machines, one step, two arms, and the rest is content, conditions and facts. No spell
gets a machine. Thunderwave is Fan-out × Save-then-effect(half) × {damage, push}.

---

## 7. The bard as the use case

The bard replaces the ranger as the pilot. Not because the ranger is wrong, but because the
bard's 1–3 kit is a **tour of the missing places** and the ranger's is a tour of the ones we
have.

- Bardic Inspiration is a condition granted to another creature that the recipient later
  spends — §3.2's owning condition and the pause, in the first feature the class gets.
- Vicious Mockery is a save that is not a contest and not an attack — §3.3's save beat, made
  visible on the client, with no slot to hide behind.
- Thunderwave is one declaration and many targets — §3.1 and §3.4 together.
- Charm Person lands on the world graph — §3.5's territory, reached through the fact fold
  that already exists rather than through a new stance API.

The bard also carries more spell choices than the ranger — `CantripsKnown: 2`,
`SpellsKnown: 4`, two first-level slots at level 1 (`classes/data.go:251-254`), against the
ranger's nothing until level 2 — and §5.2's ruling makes spells known **content refs we
author onto the sheet**, so the choice count is ours to gate. More choices is the goal, not a
cost.

### Rungs

| # | Spell or feature | Shapes × effect kinds | Places | Done-when |
|---|---|---|---|---|
| 1 | Bardic Inspiration | Grant × condition(held die) | the door with an inspiration pool (`spend_profile.go:82-84`, `character/ledger.go:178-195`); a bus condition on another member | a level-1 bard pays one use at the door, an ally carries a visible die, and a `resolution` test scene shows the pool decrement and the condition applied. The die's *spend* wants rung 4; until then it is spent automatically on the recipient's next strike, and that default is what rung 4 deletes |
| 2 | Healing Word | Grant × heal | the Cast door with slots as pools (§5.1); spells known on the sheet (§5.2); the range check (`delivery.go:12-39`) and the healing beat that already exists | walked: a bard casts it at range on a downed ally, and the client shows the slot as the price and the heal with its roll |
| 3 | Vicious Mockery | Save-then-effect × {damage, condition} | the outcome policy on the contest (§3.3); one of the inert conditions gets behavior (§3.6) | walked: the client shows a WIS save with its roll and DC, and the target's next attack rolls at disadvantage. Adds a proto enum row — Vicious Mockery is not in the `Spell` enum |
| 4 | Cutting Words | Post-roll window | the pause (§3.7) — its second producer, and the first that is not a reaction to being attacked | walked: an enemy's attack roll is seen, a bard at range is offered a window, and the answer changes whether the hit lands. Needs the College choice at 3, so it lands after level-up |
| 5 | Charm Person / Calm Emotions | Save-then-effect × {condition Charmed, world fact} | the fact → mind → disposition fold (`encounter/disposition.go:125-149`, `world.go:395-425`); the out-of-bubble cast | walked: a charmed guard's faction reads neutral through `IsHostile` inside the same run, with **no new stance API**. Per-member directed disposition stays on §3.5's shelf; this rung deliberately lands on the fold that exists |
| 6 | Thunderwave | Fan-out × Save-then-effect(half) × {damage, push} | §3.1, §3.4, a `TargetKind` for a shape, and push — which has no delivery anywhere today | walked: one declaration produces one beat per target with differing outcomes, and the board shows them moved |

### The precursor, and two hazards

**Level-up still comes first**, and it is class-agnostic (§4). No `LevelUp`, `GainLevel` or
`AdvanceLevel` symbol exists; the class tables' spellcasting rows are level-1 only; and
`classes.GetGrants` is a switch over four classes that returns nil for the bard
(`classes/grant.go:77-91`), so a bard compiles today with no features at all. §4's table,
pool-max and choice-pipeline work is the gate on any pilot.

Two things that will bite the day a slot is charged:

- `resolution.LongRest` **clones** the spell-slot map rather than resetting `Used`
  (`long_rest.go:130`). Invisible while nothing spends a slot. §5.1's pools delete the map and
  the bug with it.
- **There is no CAST anywhere in the wire vocabulary.** The session `Verb` enum has six values
  and none is a cast; `CombatAbilityId`, `ActionId` and `ActionType` have none; `TargetKind`
  is NONE | MEMBER | PATH. Every rung from 2 on adds a proto row, and rung 6 adds a target
  kind.

### The ranger, for the record

Passed over, not refuted. Its first two rungs are Hunter's Mark — a strike rider, the shape
Sneak Attack already is — and Cure Wounds, a heal, which is Second Wind with a target. Both
re-walk places we have, and the ranger reaches a missing one only at Ensnaring Strike. It also
gets no spells at level 1 (`character/draft.go:1202`), which made §5.1 a gate rather than a
choice. It stays the natural second class through the same rungs once the shapes exist.

---

## 8. Candidate first slice (not cut)

**Done-when:** a bard created at level 1 reaches level 3 on the local stack, chooses cantrips
and spells at creation and again at 2 and 3, picks a college at 3, grants Bardic Inspiration
to an ally and casts Healing Word in a fight, and the client shows the pool and the slot as
the price, the die on the ally, and the heal with its roll. That is §7's rungs 1 and 2.

Modules, one PR each, bottom-up:

1. toolkit `classes` — tables per level 1–3 (features, spellcasting rows, subclass level).
2. toolkit `character` — slots as pools; spells-known field; choice pipeline lands on the
   sheet; level-up verb shaped like `LongRest`.
3. toolkit `resolution` — the Grant shape: a price at the door for another creature's
   benefit, a heal arm and a condition arm, and the source binding an owning condition needs.
4. toolkit `session` — `Cast` verb, `VerbCast` in Afford, cast beat reusing healing and
   condition bodies.
5. protos / rpg-api / web — the per-verb pattern; level-up RPC; cost badge already renders a
   pool.

Multi-target (§3.1), the save beat (§3.3), half/recurrence (§3.4) and directed disposition
(§3.5) are **explicitly not in it**, and each gets its own design section before its first
customer, not a band-aid inside one.

---

## 9. Shelf — nothing here is lost

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

## 10. Prior art, for the record

Read, not carried: rpg-project#243; toolkit ADR-0027 (reactions; Shield worked end to end on
the old stack), ADR-0039 (the save gate), ADR-0045 (actions are data; §92 says a fireball
"requires a different profile and machine"), ADR-0042 (Afford answers in declarations);
toolkit#300, #385, #431, #205, #799, #800, #505, #695; rpg-api#120, #121, #168;
web#95, #99. The survey reports behind this document were produced on 2026-09-06 and are
summarized in §2–§4; their file:line anchors are the evidence.
