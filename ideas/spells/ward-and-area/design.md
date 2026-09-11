# Blade Ward and Thunderclap — the shared grace, and the area source

**Date:** 2026-09-09
**Status:** Proposed. No implementation has started; no production test is claimed to pass.
**Umbrella:** `ideas/spells/` — levels 1–3, and the choices at 3 made real.
**Journey:** [rpg-project#243](https://github.com/KirkDiggler/rpg-project/issues/243) — *Cast a Spell in Play*.
**Siblings:** [Bane #409](https://github.com/KirkDiggler/rpg-project/pull/409) (approved, unimplemented) ·
[Cure Wounds + Dissonant Whispers #414](https://github.com/KirkDiggler/rpg-project/pull/414) (held until Bane lands).
**Layer overview:** [Bane's overview](../bane/overview.md). Not restated here.

**Evidence boundary:** every `file:line` is against `rpg-toolkit` `origin/main` at `0aaa1807`, read
directly in a detached worktree. Bane citations are `rpg-project` `pr409`. **EXISTS** means a
current, non-test-driven responsibility. **PARTIAL** means some of it is real. **NEW** describes
this proposal, not shipped code.

**Method note, because it changed three conclusions.** Six independent surveys were run, and five
load-bearing claims were then handed to three adversarial skeptics each with instructions to
refute. **All five were refuted in part.** Three of this document's positions are the skeptics'
corrections, not the surveys' first answers, and each is marked where it matters. The most
important: a non-concentration turn-end clock *does* already exist twice, which the first survey
pass missed.

---

## Why these two spells

These are the first two spells that need **nothing from Bane**. Cure Wounds and Dissonant Whispers
are both paid for with Bane's slot and wait on it (#414). Blade Ward is a cantrip and Thunderclap
is a cantrip, so neither needs a slot at all — they can be planned and built alongside Bane rather
than behind it.

| Spell | The place it proves | Waits on |
|---|---|---|
| **Blade Ward** | a **shared turn-end grace**, plus the first self-targeted cast content | nothing |
| **Thunderclap** | the **area target source** — the engine derives the target set | the area source itself |

They are deliberately unequal. Blade Ward gates nothing and is blocked on nothing. Thunderclap is
the *minimum* proof of an area, chosen because it isolates that one variable and nothing else.

### And Blade Ward makes the bard's cantrip pick a real choice

`spells.Castable` gates the bard's cantrip options to the cantrips this build can cast, and its own
comment names the cost: *"The cost is that 'choose 2 of 2' is not a choice, which is honest about
where the build is and **disappears the moment a third cantrip gets a profile**"*
(`spells/cast.go:49-54`).

Blade Ward is that third cantrip. Adding its profile flips the bard's creation pick from 2-of-2 to
2-of-3 — the code already anticipates it, gated through `spells.Castable`
(`character/choices/requirements.go:429-444`). Thunderclap would make it 2-of-4.

**Walk note:** because the pick changes, verifying this needs a *new* character, not a reloaded one.

---

## 1. Blade Ward — a shared grace, and the first self-cast

### The player promise

A level-1 Bard spends one action, targets nobody, and until the end of its next turn takes **half**
damage from bludgeoning, piercing and slashing dealt by weapon attacks. The client shows the ward on
the bard and the halving in the damage trace when a goblin swings.

### Kirk's read, and where it needs one correction

> *"I think bladeward can go in at anytime."*

**Directionally right.** It needs no new machine, no new layer, and nothing from Bane or the area
work. It is not, however, content-only: the cast profile is one table row, and the *condition* is Go
in six places. That is ordinary condition work, not architecture — `true_strike.go` and
`vicious_mockery.go` are each hand-written the same way.

### (a) The resistance seat — EXISTS, and it reaches an incoming attack

Resistance is not a flag, a set, or a sheet field. It is a `DamageComponent` carrying
`Multiplier: Multiply(0.5)` and a matching damage type, appended at `combat.StageFinal` by a
subscriber to the chained `dnd5eEvents.DamageChain` topic, and folded into a number by
`combat.FinalDamage`.

- `conditions/raging.go:107-115` subscribes with `SubscribeWithChain`; `:464-492` is the
  defender-side branch (`if event.TargetID == r.CharacterID`) appending `Multiply(0.5)` per
  physical type at `StageFinal` under the key `"rage_resistance"`.
- `combat/final_damage.go:139-173` `resolveMultipliers` is the **single** stacking implementation —
  immunity 0.0 wins, resistance and vulnerability cancel, resistance 0.5.
- `resolution/strike.go:932-942` `foldDamage` is the **only** publisher of `DamageChain` in the
  repository, and it is the weapon-attack path.
- Proven end to end on the real driver:
  `resolution/damage_custody_test.go:831-878` `TestARagingTargetsResistanceReadsTheEventsDamageType`
  — a wolf's 9 piercing lands as 4 on a raging hero through `Resolve`.
- No sheet field exists anywhere. `monster/data.go:16-52` has no defensive field, and a monster's
  immunity/vulnerability is itself a condition (`monstertraits/immunity.go:164-179`). Neither
  `character.ApplyDamage` (`character/character.go:624-630`) nor `monster.ApplyDamage`
  (`monster/monster.go:155-158`) applies a multiplier.

**So Blade Ward's resistance is a second tenant in a seat that already works.** **EXISTS.**

**Scope, and one honest limit.** RAW resists B/P/S *dealt by weapon attacks*; Rage resists all B/P/S
from any source (`raging.go:470` predicates on `DamageType.IsPhysical()` alone). `DamageChainEvent`
carries no weapon-attack fact — no `AttackCategory`, no `IsAttack`, and `WeaponRef` is always
populated on a strike because `strike.go:199-207` seeds it from the *action* ref. The discriminator
that does exist is **per-component**: `DamageComponent.Source` is stamped `DamageSourceWeapon` vs
`DamageSourceSpell` from `m.attack.Category` (`strike.go:605-608`). So predicate on `Source`.

Copying Rage's broader predicate would be correct *today only by accident* — cast damage explicitly
skips the fold (`resolution/contest.go:300-310`), so a strike is currently the only thing that could
over-resist. Accidental correctness is not the kind we keep, so the narrower predicate is the
declared behaviour. **Flagged as untested** (see §6).

### (b) The duration — one shared grace, per Kirk's ruling

**This is where the first survey pass was wrong and the skeptics corrected it.** The surveys
concluded that no counted or skip-first non-concentration clock exists. All three skeptics found
one, in the same package Blade Ward's condition would live in:

- `RagingCondition.SawTurnEnd` — a **persisted skip-first grace** (`conditions/raging.go:31`, `:45`,
  `:290-296`), surviving the blob round trip.
- `RagingCondition.RoundActivated` vs `rageDurationRounds = 10` — a clock-anchored duration
  (`raging.go:252`, `:330`).
- `ConcentratingCondition.TurnEndsLeft` — a persisted counter (`conditions/concentrating.go:145`,
  counted at `:393-406`), reachable only through `CastConcentration`.

So the pattern exists **twice**. What does not exist is a **shared primitive**
(`SkipFirstTurnEnd`/`SkipNextTurnEnd`: zero hits repo-wide) or any way for *content* to declare a
non-concentration duration.

**Why the off-by-one is real.** `spells/cast.go:36-43` records that a cantrip is always cast during
the caster's own turn, so the next `TurnEndEvent` carrying that subject is the **casting turn's
own** — which is why `TrueStrikeTurnEnds = 2`. Vicious Mockery's one-shot expiry
(`conditions/vicious_mockery.go:212-217`) is correct *only* because it sits on the **target**
(`:50-51`), whose next turn end genuinely is its own next turn. Blade Ward sits on the caster.

**Correction to a strong claim:** a first-turn-end Blade Ward would not protect against *nothing* —
an opportunity attack provoked by the caster's own post-cast movement targets the mover
(`resolution/movement.go:297-319`) and would be resisted. It is RAW-wrong and near-worthless, not
literally inert.

**Kirk's ruling, 2026-09-09: extract one shared grace helper.** Recorded on
[#409](https://github.com/KirkDiggler/rpg-project/pull/409#issuecomment-5597499946), because Bane is
where `SkipFirstTurnEnd` lands first and that is the cheap moment. This **reverses** Bane's
`design.md:239-241`, which currently declines to generalize; that line updates with the design
rather than being left to contradict the implementation.

Scope discipline, so it stays a helper and not a duration framework:

- One embeddable struct, or two fields plus a guard method — persisted, round-tripping the way
  `SawTurnEnd` and `TurnEndsLeft` already do.
- Concentration keeps `TurnEndsLeft` and its own counting. **Only the grace is shared.**
- Rage's migration onto it must be behaviour-preserving and provable by Rage's existing tests. If
  that migration starts wanting to change rage's round/activity semantics, **leave Rage alone and
  take two customers instead of three.** Rage is the optional one.
- **Not in scope:** a content-level duration declaration. A cast still cannot declare a
  non-concentration duration. Blade Ward's clock lives in its condition and arrives through
  `CastEffect.Parameters` — opaque JSON (`combat/actions/cast.go:118`) →
  `prepareCondition` → `conditions.CreateFromRef(Config: …)` (`resolution/contest.go:204-209`), a
  channel that already exists.

**Concentration is not an option, and not merely an inelegant one.** Declaring
`Concentration: &CastConcentration{TurnEnds: 2}` would (i) drop whatever the bard was holding, via
`ConcentrationEndedRecast`, and (ii) make the first weapon hit the ward exists to soften provoke a
CON check at `max(10, damage/2)` that can strip it (`conditions/concentrating.go:347-388`) — the
spell cancelled by the damage it resists.

### (c) Self-targeted cast — EXISTS in resolution, MISSING in session, MISSING in content

**The second place the skeptics corrected the survey.** A self-targeted cast is not unproven:

- `resolution/cast_action_test.go:394-413` `TestASelfTargetedCastNamesNoCreature` drives a
  self-target, gateless, damage-free cast with `TargetID == ""` through the real `Resolve` driver
  into `newGatelessCast` → `NewActivation`, asserting an empty `outcome.TargetID` and one condition
  on the caster. **It passes.**
- The empty id is structurally inert on that arm: `bindCounterpart` returns early when
  `CounterpartKey == ""` (`resolution/action.go:539-541`) — the only shape a self cast may declare
  (`combat/actions/cast.go:230-236`) — and `startCast` (`resolution/activation.go:648-660`) reads
  only `delivery.recipientID`, never `m.targetID`. `action.go:132-134` calls empty "the one
  spelling".

**What is untested is the session layer**, and that is Blade Ward's real first-user risk:
`session/casts.go:194-202` (the `TargetKind: TargetNone`, zero-candidate offer) and
`session/cast.go:395-401` (`castTarget`'s self arm) report **zero executions** at 85.1% session
coverage, and no session test names `CastTargetSelf`. That is a benefit as much as a risk — Blade
Ward is what exercises two dead branches.

**No content declares it:** all three `castContent` entries are `CastTargetOneCreature`
(`spells/cast.go:85`, `:101`, `:123`). Note `CastProfile.Validate` refuses `RangeFeet <= 0` even for
a self cast (`cast.go:141`), so a Range: Self spell must still declare a positive number.

### (d) The bill, precisely

One line is content. The condition is Go in six places, **four of which fail closed**:

| # | Site | Why it is load-bearing |
|---|---|---|
| 1 | `refs/conditions.go` | a `dnd5e:conditions` ref + accessor + `ByID` entry. **MISSING** — the only Blade Ward ref today is the *spell* ref (`refs/spells.go:43`) |
| 2 | `conditions/blade_ward.go` | the behavior type (Apply/Remove/Ref/ToJSON/loadJSON) with its clock |
| 3 | `conditions/factory.go:115-117` | a `CreateFromRef` arm. **Fails closed** — the default refuses `"unknown condition: %s"`, reached at *preflight* via `contest.go:204` |
| 4 | `conditions/loader.go:216-219` | a `conditionLoaders` route. **Fails closed** — without it the ward never rehydrates, fatal for a condition whose whole job is to be live in the interaction where the goblin swings |
| 5 | `conditions/display.go` | a `displayCatalog` row. **Fails closed** — the gateless cast's effect collector hard-errors `"condition ref %s has no display catalog entry"` (`resolution/activation.go:311-315`) |
| 6 | `conditions/ref_contract_test.go:24-51` | the routable-condition table |

**Proof sites 3–5 are not paranoia:** `ShieldSpellCondition` is a live, tested condition carrying a
*spells* ref with no factory arm and no catalog row — and it is therefore **undeliverable by any
cast profile** (`conditions/shield_spell.go:79`; `combat/actions/cast.go:241-243` requires a
`dnd5e:conditions` ref). It is the failure this table prevents, already in the tree.

### Content declaration

```go
// NEW, in spells/cast.go
BladeWard: {
    name: "Blade Ward",
    build: func(_ castBuildInput) actions.CastProfile {
        return actions.CastProfile{
            RangeFeet: BladeWardRangeFeet, // 5 — Range: Self, but Validate requires positive
            Target:    actions.CastTargetSelf,
            // NO GATE, and NOT concentration. See §1(b).
            Effects: []actions.CastEffect{{
                Recipient: actions.CastRecipientCaster,
                Ref:       *refs.Conditions.BladeWard(),
                // No CounterpartKey — a self cast has no counterpart to bind.
                Parameters: bladeWardClock, // the grace + one turn end
            }},
        }
    },
},
```

---

## 2. Thunderclap — the minimum proof of an area

### Disambiguation first, because the two readings differ sharply

Kirk wrote *"thunder clap"* once and *"thunderway"* once. Both readings were surveyed.

**Thunderclap** — XGE cantrip. 5-foot radius centred on the caster; every creature other than you
within 5 feet makes a Constitution save or takes 1d6 thunder. No slot, no push, no half.

**Thunderwave** — 2014 PHB level-1. 15-foot cube originating from you; CON save, 3d8 thunder and
pushed 10 feet on a failure, **half** damage and no push on a success.

| | Thunderclap | Thunderwave |
|---|---|---|
| In the tree | **MISSING entirely** — zero hits, every spelling, binary-inclusive | ref (`refs/spells.go:55`, `:241`, `:423`), id (`spells/types.go:73`), `Level: 1` row (`spells/data.go:161-166`) |
| Bard access | needs a `BardCantrips` ruling | **MISSING** — wizard/sorcerer/Tempest only (`requirements.go:331`, `:631`, `subclass_modifications.go:211`) |
| Slot | **none needed** | needs Bane's `SpellSlotLevel1` |
| Save policy | negated-on-success — the only shape the door accepts | **half** — refused by name in *three* validators |
| Rider | none | 10-foot push — **no forced-movement primitive anywhere** |
| Geometry | 5-ft radius — circle delegates to `GetPositionsInRange` | 15-ft **cube on hex** — its own ruling |
| New primitives needed | **1** (the area source) | **4** |

### Verdict: Thunderclap, decisively

It **isolates the one variable.** Every clause of it other than the area is already shipped
machinery — its gate shape is Sacred Flame verbatim with DEX→`abilities.CON` and
Radiant→`damage.Thunder` (`spells/cast.go:86-92`; `damage.Thunder` exists at `damage/damage.go:105`;
CON is supported at `resolution/contest.go:189-196`; `SpendProfile.Slots` already prices an action
at `combat/spend_profile.go:60-65`).

Thunderwave bundles the area with a slot, half-on-success, forced movement, a new bard spell-list
door **and** a cube-on-hex question. A failure there would tell you nothing about the area design,
which is the whole reason to build it.

**Kirk's read — *"thunderclap will need the new aoe pattern that will be a new thing to do"* — is
correct, and the survey strengthens it: it will need *only* that.**

**One real content bug found in passing.** Thunderwave's description strings say **2d8** where 2014
PHB says **3d8** — `spells/types.go:370` and `spells/data.go:165`, both verified. Descriptions only;
no dice are declared anywhere yet, so nothing computes a wrong number today. Worth a one-line fix on
whichever branch next touches that file rather than a PR of its own. (`Shatter` next to it is
correctly 3d8, which is how the typo reads as plausible.)

### What Thunderclap costs beyond the area source

Six mechanical sites plus one ruling:

- `Spell` const (`spells/types.go:57-65`).
- **Three** edits in `refs/spells.go` — var (~`:43`), accessor (~`:229`), and the `byID` map
  (~`:413`). **Omitting the map entry makes `CastDefinition` return nil** (`spells/cast.go:167-173`)
  — a silent no-row, not an error.
- An optional `spells/data.go` row (eight existing XGE cantrip ids ship with no `SpellData` row, so
  this is precedented either way).
- A `castContent` row.
- **The ruling, not code:** `BardCantrips` is documented as *"the 2014 PHB bard cantrip list… ALL
  ELEVEN"* (`spells/cast.go:49-54`) and pinned by `spells/cast_test.go:151`
  `s.Len(spells.BardCantrips, 11)`. Thunderclap is XGE. Either the list's contract widens beyond
  2014 PHB with its comment and test updated, or Thunderclap is not a bard cantrip in this build.
  **This is a content-policy call, and it is Kirk's** — [[raw-is-not-the-authority]] says present
  the letter then the divergence as one named line, which is what this is.

**Deferred, with precedent:** cantrip damage scaling at levels 5/11/17. Dice are fixed string
consts and nothing in the cast path reads a level; `spells/cast.go:28-30` already declares scaling
out of slice, and Sacred Flame shipped unscaled. Thunderclap ships unscaled the same way.

### Content declaration

```go
// NEW, in spells/cast.go — everything but Area is Sacred Flame's shape
Thunderclap: {
    name: "Thunderclap",
    build: func(in castBuildInput) actions.CastProfile {
        return actions.CastProfile{
            RangeFeet: ThunderclapRadiusFeet, // 5
            Target:    actions.CastTargetSelf,
            Area: &actions.CastArea{           // NEW — see §3
                Shape:    actions.AreaRadius,
                SizeFeet: ThunderclapRadiusFeet,
                Origin:   actions.AreaOriginCaster,
                Excludes: actions.AreaExcludeCaster, // "every creature other than you"
            },
            Save: &saves.SaveGate{
                Abilities:  []abilities.Ability{abilities.CON},
                DC:         saves.DCStatic(in.SpellSaveDC),
                OnSuccess:  saves.Negated,
                Recurrence: saves.RecurrenceNone,
            },
            Damage: []damage.Damage{{Dice: ThunderclapDamage, Type: damage.Thunder}},
        }
    },
},
```

---

## 3. The area target source — three layers, in order

### The geometry exists and is unreachable

This is the sharpest finding of the survey, and it inverts the usual reading.

`tools/spatial` ships `GetPositionsInCircle` (4 implementations), `GetPositionsInCone` (4, with
**zero callers even in tests**), `GetPositionsInLine`, `GetHexRing`, `GetHexSpiral`,
`GetPositionsInRectangle`, `GetPositionsInArc`. **Every one has zero non-test callers**, and —
the part that matters — **none is on the `Grid` or `Room` interface** (`tools/spatial/interfaces.go:20-89`).

So `encounter`'s read-only canvas **physically cannot reach them**. A cone requires a concrete
`*HexGrid` type assertion that nothing in the tree performs, and nothing outside `tools/spatial`
ever constructs a `spatial.Circle`. "The geometry exists" is true and, today, useless.

**Reachable today:** `readOnlyRoom.GetEntitiesInRange(center, radius)` (`encounter/canvas.go:197` →
`tools/spatial/room.go:403`, radius in **cells**), plus `GetPositionsInRange`, `GetLineOfSight`,
`IsLineOfSightBlocked`. Live callers are adjacency checks only
(`monstertraits/pack_tactics.go:183`, `conditions/sneak_attack.go:332`) and spawn constraints.
Feet→cells conversion exists at `session/reach.go:28-30`.

**Regions are the wrong tool.** They are authored cell sets, and the module says so: *"a region
lists its cells… not a coordinate space"* (`encounter/region.go:16-21`). `Encounter.Region` and
`MembersIn` have **zero** production callers.

### The three layers, and the order

**(i) Expose one derived query the canvas can actually reach.** Either put the shape queries on
`spatial.Grid`/`Room`, or — smaller and sufficient for a radius — add "members within N feet of a
cell, excluding X" to the encounter canvas. A radius is the only shape either thunder spell's first
cut needs; cones and lines can wait for their own customer.

**Critically, this query is not the existing candidate query.** `session/offers.go:701-746`
`buildTargetPreflight` → `inRange` is a radius-around-the-actor query, but it is **sight-gated and
actor-excluded** (`:713` skips `len(h.CurrentVia) == 0`). That is correct for "creatures you can
see" and **wrong for an area**, which catches unseen creatures and allies. Reusing it would make a
thunderclap silently spare anything the bard could not see.

**(ii) Add the profile's target-source arm.** An `Area *CastArea{Shape, SizeFeet, Origin, Excludes}`
pointer beside `Save` and `Concentration` reads more like this codebase than a third
`CastTargetRule` value — a new enum value needs arms in three closed switches
(`combat/actions/cast.go:144-148`, `resolution/action.go:414-432`, and `Validate`), whereas a nil-able
pointer is the shape `Concentration` already established: *"A POINTER RATHER THAN A BOOL, because a
bool beside a duration that means nothing when the bool is false is a zero value that lies."*

**(iii) Add the session `TargetKind` *with* its executor.** `session.TargetKind` is closed at
`{none, member, path}` (`session/types.go:2395-2411`) and carries its own policy: *"A new kind
arrives only with a proven executor for it, never in advance."* Honour that — the kind lands in the
same wave as the thing that resolves it. Note `character.TargetKindArea` already exists as a label
that `targetKindOfAbility` collapses to `TargetNone` (`session/activations.go:246-256`), which is a
name to reuse rather than a mechanism to inherit.

**A self-origin area needs no new wire kind for its first cut.** Thunderclap is centred on the
caster and prompts for nobody, so its declaration is `TargetKind: TargetNone` with zero candidates —
the same shape Blade Ward uses. A *point*-origin area (Fireball) is what forces a new kind. So
layer (iii) can be deferred behind (i) and (ii) for Thunderclap specifically, and that is worth
knowing because it makes the first area cheaper than it looks.

### One semantic trap, and it is in Bane's loop

Damage is rolled **inside each contest** (`resolution/contest.go:296-311`), so a naive per-target
loop rolls the pool **once per creature** where the PHB rolls area damage **once for all**. This is
recorded here and raised on
[#409](https://github.com/KirkDiggler/rpg-project/pull/409#issuecomment-5597126062) as change (d),
because it is Bane's loop that would bake it in, Bane declares no damage so fixing it costs Bane
nothing behaviorally, and it is trivial now and invasive after Bane ships.

---

## 4. The Bane coupling

Bane's loop is **shape-agnostic and free at any cardinality**: whole-list pure preflight → one
atomic `combat.Pay` → ordered per-target contest/delivery → ordered `[]CastTargetOutcome` → one
owner (`design.md:204-215`, plus `plan.md:536` "Session must not loop targets"). Cardinality is data,
not structure (`plan.md:117-126`). An N-target *chosen* spell like Bless reuses all of it on content
alone.

**What an area cannot reuse is the entry**, and the risk is not low reuse. The list arrives as
`CastInput{… Targets []string}`, client-clicked in click order (`plan.md:168-172`, `:721`), and every
preflight bullet validates a caller's list as a **subset** of a candidate universe
(`design.md:180-181`). An area needs a **derived, complete** set.

**As written, Bane installs the client as the authority on the target set, with no seam for "the
engine derived this list."** Ride that list for an area and a client can *spare* creatures inside the
blast, and the sight-gated candidate query silently exempts unseen ones. That is a rules calculation
above the toolkit — the thing the boundary rule exists to stop.

Six changes, all cheap while Bane is unimplemented, are on
[#409](https://github.com/KirkDiggler/rpg-project/pull/409#issuecomment-5597126062) and not restated
here. Areas are an explicit non-goal of Bane's slice (`design.md:61`) and neither thunder spell is
named anywhere in its docs (zero hits) — so §4 is inference from Bane's specified contracts, not a
claim about its intent.

---

## 5. Acceptance contract

Through production entrypoints and persistence boundaries.

**Blade Ward**

1. **The halving.** A goblin's weapon hit for 9 slashing against a warded bard applies 4, through
   the real `Resolve`, and the recorded damage trace shows the resistance component with Blade
   Ward's ref — not a bare halved total.
2. **Scope.** The ward halves B/P/S from a weapon attack and **does not** halve the same damage type
   from a non-weapon source, predicated on `DamageComponent.Source`.
3. **Duration.** Cast on the bard's turn, the ward survives that turn's end, is live for an enemy
   attack on the following round, and ends at the end of the bard's next turn. Reload preserves both
   the remaining clock and whether the grace was consumed.
4. **The grace is shared.** Concentration's turn-end behaviour is unchanged by the extraction, and
   Rage's existing tests pass unmodified. If Rage was migrated, its round/activity semantics are
   byte-identical before and after.
5. **Not concentration.** Casting Blade Ward while concentrating on True Strike leaves True Strike
   held, and a weapon hit on the warded bard provokes **no** concentration check on the ward itself.
6. **Self-cast through the session door.** A real `Cast` for Blade Ward with no target reaches
   resolution, exercising `session/casts.go:194` and `session/cast.go:395` — both currently at zero
   executions — and the client renders a targetless cast row.
7. **The six sites fail closed, deliberately.** Omitting the factory arm, the loader route, or the
   display-catalog row each produces a *refusal*, not a silent no-op. Assert each error rather than
   trusting the wiring.
8. **Creation.** A fresh level-1 bard is offered 2-of-3 cantrips including Blade Ward; finalize and
   reload preserve the picks.

**Thunderclap** (after the area source)

9. **Derivation, not selection.** One cast catches **every** creature within 5 feet — allies
   included, and creatures the caster cannot see included — and excludes the caster. Specifically:
   a creature in range that the bard has no current sight line to **is** caught, which is the
   regression that proves the candidate query was not reused.
10. **One roll for all.** The 1d6 is rolled **once** and the same total is applied to every failed
    save; the record shows one damage roll, not N.
11. **Per-target saves.** Each caught creature rolls its own CON save; failures take the shared
    total, successes take nothing, and the beat carries one cast plus per-target outcomes in a
    stable order.
12. **Zero targets is not an error.** A thunderclap with nobody in range pays the action, delivers
    nothing, and records honestly.
13. **Regression.** Sacred Flame, Vicious Mockery and True Strike resolve unchanged; the bard's
    cantrip pick becomes 2-of-4.

---

## 6. Rejected alternatives

- **Concentration for Blade Ward.** Actively wrong, not inelegant: it drops the held spell and lets
  the first hit strip the ward. §1(b).
- **A third private turn-end grace.** Kirk's ruling took the shared helper. The alternative was
  three copies of one documented off-by-one.
- **A duration framework, or content-declared non-concentration durations.** Out of scope. Only the
  grace is shared; the clock stays in the condition and rides `CastEffect.Parameters`.
- **Copying Rage's all-B/P/S predicate for Blade Ward.** Correct today only because cast damage
  skips the fold. Accidental correctness is not the kind we keep.
- **A new `CastTargetRule` enum value for areas.** Needs arms in three closed switches. A nil-able
  `Area *CastArea` pointer is the shape `Concentration` already established.
- **Reusing the existing candidate query for area membership.** Sight-gated and actor-excluded —
  correct for "creatures you can see", wrong for a blast. Reusing it is the bug, not the shortcut.
- **Computing area membership above the toolkit.** "Who is inside 5 feet" is a rule. §4.
- **Thunderwave as the first area spell.** Four new primitives instead of one; a failure there tells
  you nothing about the area design.
- **Cones and lines in the first cut.** A radius is all either thunder spell needs. `GetPositionsInCone`
  has zero callers even in tests; it gets exposed when it has a customer.
- **Cantrip damage scaling.** Deferred with precedent — Sacred Flame shipped unscaled and the cast
  path reads no level.

---

## 7. Delivery and sequence

1. **Blade Ward — ships first, gates nothing, blocked on nothing.** Its only dependency was the
   duration ruling, which is made. It pays for itself three ways: first `CastTargetSelf` content,
   first shared-grace customer, second tenant in the resistance seat.
2. **Bane's six shape changes — the only item with a hard deadline.** Before Bane's implementation
   starts. Every one is a rewrite afterwards. On #409.
3. **Bane, as planned plus those changes** — delivers the slot, the paid multi-target loop, repeated
   wire targets, and the bard level-1 spell door.
4. **The area target source, proven by Thunderclap** — layers (i) and (ii) of §3; (iii) deferrable
   for a self-origin area.
5. **Half-on-success and forced movement** — independently useful, both prerequisites for
   Thunderwave. Half is #414's business; forced movement has no owner by design.
6. **Thunderwave last** — needs 3, 4 and 5, plus a cube-on-hex ruling.

The shared grace (1) and Bane (3) touch the same field, so **1 and 2 should be sequenced against
each other deliberately**: either the helper lands on Bane's branch and Blade Ward consumes it, or
Blade Ward lands first with the helper and Bane adopts it. Both are fine; doing them blind in
parallel is not.

Nothing here authorizes a merge, a force-push, a shared-stack deployment, or an environment wipe.

---

## 8. Where the evidence is thin

Stated rather than smoothed over.

- **The `Source == DamageSourceWeapon` predicate is an untested hypothesis.** The field exists and
  is stamped (`strike.go:605-608`); nobody tested a resistance predicated on it across a crit,
  off-hand, or two-weapon chain. Verify before relying on it for §5 scene 2.
- **rpg-api and rpg-dnd5e-web were not surveyed at all.** Every "no work above the toolkit"
  statement here is unverified for the wire and the client — specifically a `TargetNone` cast row, a
  new condition label, and an area footprint preview. Treat all three as **unknown, not zero**.
- **Cube-on-hex is unaddressed.** No survey checked whether a 15-foot cube is expressible on the
  `AxialHexGrid` the encounter runs. Circle is (it delegates to `GetPositionsInRange`);
  `GetPositionsInRectangle` exists on hex (`hex_grid.go:418`) with zero callers and unverified
  semantics. Thunderwave's blocker, not Thunderclap's.
- **Half-on-success cost is unestimated.** Established: three refusals by name
  (`combat/actions/cast.go:151`, `resolution/contest.go:179`, `combat/actions/attack.go:259`), zero
  non-test uses, and a success branch that returns `Done` before any delivery step exists
  (`contest.go:619-632`). Not established: how much building it costs.
- **Whether a first-turn-end Blade Ward is literally inert.** The survey said zero enemy turns; the
  skeptics said an own-turn opportunity attack would still be resisted. Both true; the practical
  answer is RAW-wrong and near-worthless.
- **Bane docs were read from `pr409`.** If that PR moves, §4's citations move with it.
- **One skeptic partly disagreed with the other two** on how much of Blade Ward is content. All
  three refuted "content only"; only two enumerated the loader and display failure modes. §1(d)'s
  table is the union. The display catalog (`activation.go:311-315`) and the loader
  (`loader.go:216-219`) are the two that fail non-obviously and are worth verifying first when work
  starts.

— cross-team agent, on behalf of KirkDiggler
