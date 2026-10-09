# Action information — provider slices

Authority: [provider law](provider-design.md) R10–R14, the folder's [design](design.md)
R1–R9, toolkit#1987 hand-off and rulings, protos#384 wire. This plan replaces the
provider half of [implementation-plan.md](implementation-plan.md) (T3–T7); T1–T2 stand.

Survey heads (every `file:line` below is at these heads; the symbol is the anchor
when lines drift):

| Repo / module | Head |
|---|---|
| rpg-toolkit `origin/main` | `6a61db15` — root v0.206.0, resolution v0.67.0, session v0.124.0 |
| rpg-toolkit draft #1985 `feat/action-information` | `f68a3389` (root module only; merges cleanly with main) |
| rpg-api `origin/dev` | `86c84138` |
| rpg-dnd5e-web `origin/dev` | `4c2ad79b` |
| rpg-api-protos generated Go carrying #384 | `53694d32` (tag v0.1.230) |

Order: root, then resolution, then session, then API, then web walk. One
nearest-go.mod module per PR. Each dependent pins the provider's
**gate-approved pushed head** by pseudo-version (`go get …@<sha>` then
`go mod tidy`); never a local replace or `go.work`. Release inside-out after the
walk: root tag, resolution, session, then API.

## Site counts

| Slice | Write sites | New files |
|---|---|---|
| S1 root | 39 plus the merge | 0 (two exist on #1985) |
| S2 resolution | 5 | 1 test file |
| S3 session | 31 (the renderer grows inside the new `information.go`) | 4 |
| S4 API | 3 + go.mod | 0 |
| S5 web | 0 expected | walk script only |

---

## S1 — Root: typed facts, authored prose (toolkit `rulebooks/dnd5e`)

**Model:** Sonnet. Mechanical edits plus prose authoring; the prose instruction
below bounds the judgment.
**Branch:** continue `feat/action-information` (draft #1985). First write is
`git merge origin/main` (a merge commit; never rebase, never force-push).

### What #1985 keeps / what changes

| #1985 piece | Disposition |
|---|---|
| `Definition.Description` (`combat/actions/definition.go`) | Keep. Rewrite its comment: not in the selector's allow-list, rather than hosts strip it. |
| `CastOption.Description` (`combat/actions/cast.go`) | Keep. |
| Spell catalogue reuse in `CastDefinition` (`spells/cast.go:992`) | Keep. |
| Generic melee/ranged prose in `weaponattack.Assemble` (`combat/weaponattack/weaponattack.go:169`) | Keep. |
| `damage.IncludesAbilityModifier` + `AbilityModifierInformationSuite` | Keep unchanged. |
| `actions.Describe` entry point, nil/invalid refusal | Keep the name and validation; change the output to typed facts. |
| `Information`, `InformationDetail` (root) | **Delete.** The string types move to session as `ActionInformation` (R10). |
| String formatting inside `Describe` (`fmt.Sprintf`, `strings.ToUpper`, `Display()`) | **Delete.** Session renders (R10). |
| `BasicActionKind`, `BasicInformation`, `BasicInformationInput` and their test | **Delete** (R12). |
| `InformationSuite` assertions on strings | Rewrite against typed facts; same cases. |

### Types (in `combat/actions/information.go`)

```go
type DescribeInput struct{ Definition Definition }

type DescribeOutput struct {
    Description string    // Definition.Description verbatim; may be empty
    Facts       BaseFacts
}

type BaseFacts struct {
    Damage []DamageFact     // attack pools, then Cast.Damage pools, declared order; nil when none
    Grip   Grip             // GripNone unless the attack carries weapon context
    Melee  *MeleeDelivery   // copy of the attack's melee delivery, nil when ranged
    Ranged *RangedDelivery  // copy of the attack's ranged delivery, nil when melee
    Cast   *CastFacts       // nil unless Definition.Cast is set (R14)
}

type CastFacts struct {
    RangeFeet       int                // CastProfile.RangeFeet        combat/actions/cast.go:113
    Targets         TargetsFact        // Target, MinTargets, MaxTargets cast.go:117, :121, :122
    Save            *SaveFact          // CastProfile.Save              cast.go:132
    DamageIfInjured []DamageFact       // CastProfile.DamageIfInjured   cast.go:147
    Effects         []EffectFact       // CastProfile.Effects           cast.go:150
    Healing         *HealingFact       // CastProfile.Healing           cast.go:102
    Area            *CastArea          // copy of CastProfile.Area      cast.go:158 (Footprint, Catches: area.go:140)
    Concentration   *CastConcentration // copy of CastProfile.Concentration cast.go:175 (TurnEnds: cast.go:226)
}

type TargetsFact struct {
    Rule CastTargetRule // cast.go:20
    Min  int
    Max  int
}

type SaveFact struct {
    Abilities  []abilities.Ability // SaveGate.Abilities   saves/gate.go:157
    DC         int                 // SaveGate.DC.DC(saves.DCInput{}) when Kind() == saves.DCKindStatic (gate.go:84, :97)
    DCKnown    bool                // false for any non-static DC source; DC is then 0
    OnSuccess  saves.SaveEffect    // SaveGate.OnSuccess: Negated or Half (gate.go:20, :24, :163)
    Recurrence saves.Recurrence    // SaveGate.Recurrence: none or end_of_turn (gate.go:33, :37)
}

type EffectFact struct {
    Ref          core.Ref      // CastEffect.Ref        cast.go:248
    Recipient    CastRecipient // CastEffect.Recipient  cast.go:245
    OnFailedSave bool          // Save != nil; a gated cast delivers only to its target
                               // (resolution/action.go:928 refuses anything else)
}

type HealingFact struct {
    Dice      string          // healing.Declaration.Dice       healing/healing.go:33
    Modifiers []ModifierFact  // healing.Declaration.Modifiers  healing/healing.go:34
}

type ModifierFact struct {
    Name   string // healing.Modifier.Source.Name, already content-authored (character/casting.go:26)
    Amount int
}

type DamageFact struct {
    Dice      string
    FlatBonus int
    Type      damage.Type
    Ability   *AbilityFact  // nil when the pool lacks damage.AddsAttackAbilityModifier
}

type AbilityFact struct {
    Ability      abilities.Ability
    Modifier     int   // a present zero is a real zero
    Participates bool  // damage.IncludesAbilityModifier's answer
}

type Grip string
const (
    GripNone      Grip = ""
    GripOneHanded Grip = "one-handed"
    GripTwoHanded Grip = "two-handed"
    GripOffHand   Grip = "off-hand"
)

func Describe(in *DescribeInput) (*DescribeOutput, error)
```

`Describe` reads `Definition.Attack`, else `Definition.Cast.Attack`, for attack
pools and delivery. When `Definition.Cast` is set it also fills `Cast`. It appends
`Cast.Damage` (cast.go:137) to `Damage`. It never resolves a non-static DC
against a guessed input. Grip is
`GripOffHand` when `IsOffHandAttack`, else `GripTwoHanded`/`GripOneHanded` from
`WeaponContext.TwoHanded`, else `GripNone`. It imports no `fmt` for output and
returns no string other than `Description`.

### Content and plumbing sites (writes, in order)

1. `git merge origin/main` on `feat/action-information`.
2. `combat/actions/information.go`: replace the body as above; delete the basic-action block.
3. `combat/actions/definition.go:19` `Definition`: comment only.
4. `features/loader.go:13` `Feature`: add `Description() string` beside `Name()`.
5. Implement `Description()` beside each `Name()`:
   - `features/rage.go:77`
   - `features/second_wind.go:71`
   - `features/action_surge.go:65`
   - `features/bardic_inspiration.go:66`
   - `features/reckless_attack.go:51`
   - `features/flurry_of_blows.go:47`
   - `features/patient_defense.go:47` returns `""` per Open O2 until toolkit#1986, with a comment naming #1986
   - `features/step_of_the_wind.go:48`
   - `features/deflect_missiles.go:68`
   - `features/warding_flare.go:41`
   - `features/wrath_of_the_storm.go:43`
   - `features/blessing_of_the_trickster.go:30`
6. Test fakes that implement `Feature` gain `Description()`:
   - `character/status_view_test.go:806`
   - `character/advance_test.go:581`
   - `character/economy_dirty_test.go:403`
7. `character/action_economy_types.go:81` `AvailableAbility`: add `Description string`.
   Copy it at `character/action_economy.go:329` (`ca.Description()`) and `:355` (`f.Description()`).
8. Cast option prose:
   - Thorn Whip, `spells/cast.go:324`: three options.
   - Command, `spells/cast.go:701`: Approach, Flee, Grovel.
   - Shillelagh, `spells/shillelagh.go:47`: per held weapon, naming that weapon's label.
9. Reaction offer prose. Add `Description string` with a JSON tag matching each type's convention:
   - `events/offer.go:22` `Offer`
   - `events/attack_roll_offer.go:11` `AttackRollOffer`
   - `events/post_hit.go:24` `PostHitOffer`
   - `events/post_hit.go:34` `PostHitOption`
10. Condition display detail (R14), the source for an applied condition's prose.
    `conditions.DisplayFor(ref)` (`conditions/display.go:29`) already returns
    `{Name, Detail}` per ref. These conditions are applied by a spell's `Effects`
    but have an empty `Detail`, so author it beside their names:
    - `conditions/display.go:87` Blade Ward
    - `conditions/display.go:91` Commanded
    - `conditions/display.go:112` Shield of Faith
    - `conditions/display.go:113` Guided
    - `conditions/display.go:114` Resistance
    - `conditions/display.go:116` Sanctuary Immune

    The catalogue's voice rule applies: a condition that bears on attacks against
    its holder is written in the third person. Filling `Detail` only removes
    existing refusal paths (`conditions/action_effects.go:67`,
    `conditions/target_held.go:243`). The holder's status view also shows the new
    text (`character/status_view.go:260`), which is the intended additive display.
11. Reaction producers fill it:
   - `conditions/inspired.go:239`
   - `conditions/guided.go:212`
   - `conditions/resistance.go:215`
   - `features/warding_flare.go:139` uses `w.Description()`
   - `features/wrath_of_the_storm.go:139` uses `w.Description()`, plus each option's description

**Prose rule for the builder.** Read the implementing code first, then write what
that code does. Do not write tabletop text the code does not implement. Each
description is one or two plain sentences in the second person. A condition's
offer prose is a named constant beside its existing `…Name` constant.

### Done when (tests, named)

- `InformationSuite` (`combat/actions/information_test.go`):
  - `TestWarhammerGripDice`: one-handed `1d8`, two-handed `1d10`, with grips.
  - `TestFinesseStatesDex`: Rapier states DEX +4.
  - `TestOffHandParticipation`: modifiers +3, 0 and −2 give `Participates` false, false and true. Grip is off-hand.
  - `TestZeroModifierIsPresent`: `Ability` is non-nil with modifier 0 and participating.
  - `TestPoolsStaySeparateWithFlatBonus`: two pools keep their order, types and bonus.
  - `TestOnHitOnlyStatesNoDamage`: `Facts.Damage` is empty.
  - `TestReachAndRange`
  - `TestDescribeDoesNotMutate`: JSON before equals JSON after.
  - `TestBaneReusesCatalogue`: Bane's description equals `spells.GetData(spells.Bane).Description`.
  - `TestBaneCastFacts`: built with `SpellSaveDC: 13`. Save is CHA (the code's Bane is a Charisma save), DC 13 with `DCKnown`, `OnSuccess` Negated. One effect, `dnd5e:conditions:baned`, recipient target, `OnFailedSave`. Targets one_creature 1 to 3, range 30, concentration present, no damage.
  - `TestThunderwaveCastFacts`: CON save, `OnSuccess` Half, damage `2d8` thunder with no ability fact. Area box 15 ft from the caster's edge, catches others.
  - `TestCureWoundsCastFacts`: healing `1d8` with the input's modifiers in order (spellcasting modifier, then Disciple of Life when supplied). No save, no effects, touch, 1 to 1.
  - `TestNonStaticDCIsUnknown`: a gate with `saves.DCFivePlusDamageTaken()` gives `DCKnown == false` and `DC == 0`.
  - `TestNilAndInvalidInputRefused`
- `AbilityModifierInformationSuite`: unchanged and green.
- `features`:
  - `TestEveryLoadableFeatureDescribesItself`: iterates the loader's refs; non-empty for all except Patient Defense.
  - `TestPatientDefenseDescriptionIsAbsent`: pins Open O2.
- `character`:
  - `TestAvailableAbilitiesCarryDescriptions`: Dodge equals `combatabilities` Dodge's description. A spent Dodge (`CanUse == false`) keeps its text.
- `conditions`:
  - `TestEveryCastAppliedConditionHasDetail`: every `Effects` ref in every spell's cast profile has a non-empty `DisplayFor` detail.
- `spells`:
  - `TestEveryDeclaredCastOptionIsDescribed`: every spell in `castContent` plus two-weapon Shillelagh; every option has a non-empty description.
- Reaction producers:
  - `TestWardingFlareOfferDescribed`
  - `TestWrathOfTheStormOptionsDescribed`
  - `TestInspiredGuidedResistanceOffersDescribed`
- Gates: from `rulebooks/dnd5e`, run `go test -race ./...` and repository hooks and lint per `docs/how-to/run-tests.md`.

**Deletion-mutants (each must turn a named test red):**
- Replace the `IncludesAbilityModifier` call in `Describe` with `true`. Expected red: `TestOffHandParticipation`.
- Drop the Grip assignment. Expected red: `TestWarhammerGripDice`.
- Return `""` from `Rage.Description`. Expected red: `TestEveryLoadableFeatureDescribesItself`.
- Remove one Command option description. Expected red: `TestEveryDeclaredCastOptionIsDescribed`.
- Drop the copy at `action_economy.go:355`. Expected red: `TestAvailableAbilitiesCarryDescriptions`.
- Hard-code `OnSuccess` to Negated in `Describe`. Expected red: `TestThunderwaveCastFacts`.
- Resolve every DC source as known. Expected red: `TestNonStaticDCIsUnknown`.
- Blank the Commanded detail. Expected red: `TestEveryCastAppliedConditionHasDetail`.

---

## S2 — Resolution: one rule, prose through the freeze (toolkit `rulebooks/dnd5e/resolution`)

**Model:** Sonnet.
**Prerequisite:** S1 head approved by its gate; `go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@<S1 sha>` and `go mod tidy`.

Writes, in order:
1. `go.mod`, `go.sum`: pin S1.
2. `strike.go:749` (`rollDamage`): replace `(!offHand || modifier < 0)` with
   `damage.IncludesAbilityModifier(damage.AbilityModifierInput{Modifier: modifier, OffHand: offHand})`.
   Facts still come from `baseDamageFacts` (`strike.go:807`), unchanged.
3. `step.go:160` `Choice`: add `Description string \`json:"description,omitempty"\``.
4. `strike_pose.go:405` (`posePostHit`): `Choice{ID, Label, Description: option.Description}`.
5. `strike_pose.go:413`: the posed `dnd5eEvents.Offer` copies `Description: offer.Description`.
6. `attack_roll_reaction.go:23` (`poseBeforeRoll`): the posed `Offer` copies `Description: offer.Description`.
   The single Use choice carries no description of its own. Its row's information explains it.

The post-roll poses at `check_pose.go:105`, `save_pose.go:147` and `strike_pose.go:303` pass the `Offer` whole, so they need no edit.

### Done when

- Existing `off_hand_attack_test.go` stays green. The covered cases are:
  - `TestOffHandAttackAbilityModifierDamage`
  - `TestTwoWeaponFightingStyleRestoresPositiveOffHandDamage`
  - `TestOrdinaryWeaponAttackStillAddsPositiveAbilityDamage`
- New `TestOffHandZeroModifierStaysOutOfBase` covers the zero case.
- New `information_preservation_test.go`:
  - `TestPostHitChoicesKeepDescriptionAcrossFreeze`: Wrath options' descriptions on the pose, then marshal the frozen payload, thaw it and resume. Mechanics resume unchanged.
  - `TestBeforeRollOfferKeepsDescription`: Warding Flare.
  - `TestPostRollOfferKeepsDescription`: Inspired.
- Gates: from the `resolution` directory, run `go test -race ./...` and hooks. The diff touches only this module.

**Deletion-mutants:**
- Restore `true` in place of the helper call. Expected red: `TestOffHandAttackAbilityModifierDamage`.
- Drop `Description` at `strike_pose.go:405`. Expected red: `TestPostHitChoicesKeepDescriptionAcrossFreeze`.

---

## S3 — Session: render, own verb prose, allow-list selector (toolkit `rulebooks/dnd5e/session`)

**Model:** Opus. The selector projection and its completeness guard decide every
action identity in the game. A wrong omission fails open on staleness, and that
is not a mechanical slice.
**Prerequisite:** S1 and S2 heads approved by their gates and pinned by pseudo-version.

### Types

```go
// afford.go — seam-owned, beside EffectRow
type ActionInformation struct {
    Description string                    `json:"description,omitempty"`
    Details     []ActionInformationDetail `json:"details,omitempty"`
}
type ActionInformationDetail struct {
    Label string `json:"label"`
    Value string `json:"value"`
}
// Declaration gains (after Effects, afford.go:426):
Information *ActionInformation `json:"information,omitempty"`
// CastOption (types.go:3505) gains:
Description string `json:"description,omitempty"`
```

### Rendering (R10, in new `information.go`)

`renderFacts(actions.BaseFacts) []ActionInformationDetail`, in this order:

| Fact | Label | Value |
|---|---|---|
| each `DamageFact` | `Base damage` | dice, then ` %+d` when `FlatBonus != 0`, then ` + STR modifier (%+d)` when `Ability != nil && Participates`, then ` · ` and `Type.Display()` |
| each `Cast.DamageIfInjured` | `Damage if injured` | same form as base damage |
| `Grip` when not none | `Grip` | `One-handed` / `Two-handed` / `Off-hand` |
| `Cast.Save` | `Save` | abilities joined by ` or `, then ` save`; then ` · DC %d` when `DCKnown`; then ` · success: negated` or ` · success: half damage`; then ` · repeats at end of turn` for end_of_turn. Example: `CHA save · DC 13 · success: negated` |
| each `Cast.Effects` | `On a failed save` when `OnFailedSave`, else `On the target` / `On you` by recipient | the condition's `DisplayFor` name, for example `Baned` |
| that effect's detail, when non-empty | the condition's name | `DisplayFor(ref).Detail` verbatim |
| `Cast.Healing` | `Healing` | dice, then ` + %d (%s)` per modifier in order, for example `1d8 + 3 (Wisdom)` |
| `Melee`, only when `Cast == nil` | `Reach` | `%d ft` |
| `Ranged`, only when `Cast == nil` | `Range` | `%d ft`, or `%d ft (long %d ft)` when long > 0 |
| `Cast.RangeFeet` | `Range` | `Self` for the self rule, `Touch` for touch, else `%d ft` |
| `Cast.Targets`, only for one_creature and known_creature | `Targets` | `%d creature` when min equals max (plural above one), else `%d to %d creatures` |
| `Cast.Area` | `Area` | `%d ft` plus the shape word (`radius`, `cube` for box, `cone` for triangle), then ` from you` / ` from your edge` / ` at a point` by origin, then ` · affects others` or ` · affects everyone` by catches |
| `Cast.Concentration` | `Concentration` | `Required` |

The ability abbreviation is the upper-cased ability key. The expected warhammer
string is `1d8 + STR modifier (+3) · Bludgeoning`, which matches the web fixture.
Weapon delivery rows are skipped for casts because the cast's own range is what
the player reads; the facts are still stated below.

**Condition prose source (R14).** Session looks up `conditions.DisplayFor(ref)`
(`conditions/display.go:29`); session already imports `conditions`. A ref with no
catalogue entry fails `Afford` closed, matching the catalogue's own contract. An
entry with an empty detail renders the name row alone. The six empty details on
spell-applied conditions are filled by S1.

### Prose ownership (R12, in `information.go`)

- `sessionVerbProse map[Verb]string` holds Move, End Turn, Death Save, Intimidate and Persuade, authored here. The texts start from the deleted root strings.
- `reactionDescription` sits beside `reactionName` (`mover.go:51`) and holds the opportunity attack, which session names.
- `sessionVerbInformation(verb) *ActionInformation` returns nil for every verb session does not own.

### The allow-list selector (R11, new `selector_projection.go`)

- `selectorDefinition` mirrors `Definition` with the same JSON tags: `Ref`, `Name`, `Cost`, `Attack`, `Cast *selectorCastProfile` and `Sequence`. It has no `Description`.
- `selectorCastProfile` mirrors all 18 `CastProfile` fields with identical tags. Its `Options []selectorCastOption{ID, Label}` has no description.
- Root types that carry no prose are reused as-is: `AttackProfile`, `SequenceProfile`, `SpendProfile`, `CastEffect`, `CastArea`, `CastMove` and the rest.
- `selectorDefinitionOf(*Definition) *selectorDefinition` builds it.
- In `declaration_id.go:308` `definitionVariant`, keep `Validate()` on the real definition first. Then marshal `selectorDefinitionOf(definition)` through a pointer, so `core.Ref` keeps its string form. Keep the `json.Valid` guard.
- Name and Label stay in the projection (Open O1), so every selector is byte-identical to today's.

### Writes, in order

1. `go.mod`/`go.sum`: pin S1 and S2.
2. `declaration_id_test.go`: add `TestCastDeclarationIDGolden` for a compiled Command cast. Capture its value **before** any other change, against unmodified session.
3. `selector_projection.go` and `selector_projection_test.go`. Switch `definitionVariant` over to the projection.
4. `afford.go:286` `Declaration` and `types.go:3505` `CastOption`: the new fields. Add the two information types.
5. `information.go`: `attachInformation(*attachInformationInput) error`, the renderer and the prose tables.
6. Call sites:
   - `afford.go:651`: call `attachInformation` directly after `attachEffects`, in the turn path only, over the same compiled offers. By verb:
     - Attack: `Describe(offer.attack)`.
     - Cast: `Describe(offer.spell)`.
     - Activate: the offer's ability description.
     - Session verbs: `sessionVerbInformation`.
   - `afford.go:675` `blockedDeclaration`: set `Information: sessionVerbInformation(verb)`. Blocked attack, activate and cast rows stay nil.
   - `afford.go:944` world-clock social rows: `sessionVerbInformation(spec.verb)`.
   - `activations.go:82`: `compiledOffer` (`offers.go:38`) gains `abilityDescription string`, copied from `ability.Description`.
   - `casts.go:387` `castOptions`: copy `option.Description`.
7. Reaction windows. Each window payload gains `OfferDescription string \`json:"offer_description,omitempty"\``. This goes on the payloads, not on `ReactionRef`, which also rides stream events (`types.go:2426`).
   - Payload types:
     - `react_pending_attack.go:22`
     - `react_post_hit.go:21`
     - `window.go:105` post-roll
     - `window.go:147` check offer
     - `window.go:207` cast offer
   - Pose sites copy `ask.Offer.Description` and each `ask.Choices[i].Description` into `CastOption`:
     - `react_pending_attack.go:75` and `:79`
     - `react_post_hit.go:54` and `:56`
     - `cast.go:640` and `:642`
     - `attack.go:519`
     - `doors.go:400`
     - `social.go:381`
   - React row builders set `Information{Description: payload.OfferDescription}` when it is non-empty:
     - `react_pending_attack.go:110`
     - `react_post_hit.go:84`
     - `react_post_roll.go:158`
     - `react_check_offer.go:134`
     - `react_cast_offer.go:148`
   - The opportunity-attack row at `afford.go:888` uses `reactionDescription`.
8. `doc.go`: one paragraph on information, its single attach point and the allow-list.

Offer builders with **no edit**, because information arrives by verb in `attachInformation`:
- `offers.go:582` Attack
- `offers.go:664` End Turn
- `offers.go:688` Death Save
- `offers.go:707` Move
- `offers.go:779` social
- `casts.go:191`, `:219`, `:236` and `:341` Cast

### Done when (named, real `Afford` where noted)

- `selector_projection_test.go`:
  - `TestSelectorProjectionClassifiesEveryDefinitionField` (R11 guard). It reflects over the `Definition` type tree: structs, pointers, slices and maps. It requires every `Type.Field` path to be in exactly one of two explicit lists in the test, mechanical or prose. The prose list today is `Definition.Description` and `CastOption.Description`. An unlisted field fails.
  - `TestSelectorProjectionEqualsDefinitionWithoutProse`: for warhammer, Command, Thorn Whip, two-weapon Shillelagh and a multiattack sequence, the canonical JSON of the projection equals the canonical JSON of the definition with its prose fields cleared.
- `declaration_id_test.go`:
  - `TestSelectorIgnoresProse`: change `Definition.Description` and an option's `Description`, or add text to them. The ID is unchanged for both attack and cast.
  - `TestSelectorTracksOptionID`: change one option ID and the ID changes.
  - `TestAttackDeclarationIDGolden` and the new `TestCastDeclarationIDGolden` are byte-identical.
  - All existing `TestDeclarationID…` tests stay green.
- `information_test.go`, through real `Afford` on a built world:
  - `TestAffordWarhammerFactsAboveUnchangedEffects`: the rendered rows, plus `EffectRowsSuite` rows equal to their pre-change values.
  - `TestAffordBaneDescribedWithZeroEffects`: also asserts the rows `Save: CHA save · DC 13 · success: negated`, `On a failed save: Baned`, the Baned detail row, `Range: 30 ft`, `Targets: 1 to 3 creatures` and `Concentration: Required`, in that order.
  - `TestAffordThunderwaveFacts`: `Base damage: 2d8 · Thunder`, `Save: CON save · DC 13 · success: half damage`, `Area: 15 ft cube from your edge · affects others`.
  - `TestAffordCureWoundsFacts`: `Healing: 1d8 + <mod> (<ability>)`, `Range: Touch`, and no save row.
  - `TestAffordSpentDodgeStillDescribed`: `Available == false`, description present.
  - `TestAffordCommandOptionsDescribed`
  - `TestAffordSessionVerbsDescribedIncludingBlocked`: off-turn rows for Move, End Turn and social carry prose; blocked Attack carries none.
  - `TestAffordAbsentProseStaysAbsent`: an empty-description definition gets no description and is never synthesised from its name.
  - `TestReactWindowKeepsOfferAndOptionProseAcrossReload`: Wrath of the Storm; reload the session, Afford again, same text, same ID.
  - `TestInformationDoesNotAliasRoot`: mutating the returned details leaves a second Afford unchanged.
  - `TestExecutionPathsNeverAttachInformation`: compiled offers from an execution caller carry nil information.
- Gates: from the `session` directory, run `go test -race ./...` and hooks. Existing boundary and no-rules tests stay green.

**Deletion-mutants:**
- Marshal the raw definition in `definitionVariant`. Expected red: `TestSelectorIgnoresProse`.
- Drop `Options` from `selectorCastProfile`. Expected red: `TestSelectorTracksOptionID` and `TestCastDeclarationIDGolden`.
- Drop `Move` from `selectorCastProfile`. Expected red: `TestSelectorProjectionEqualsDefinitionWithoutProse`.
- Remove the `attachInformation` call. Expected red: `TestAffordBaneDescribedWithZeroEffects`.
- Drop the copy in `castOptions`. Expected red: `TestAffordCommandOptionsDescribed`.
- Drop `OfferDescription` at `react_post_hit.go:56`. Expected red: `TestReactWindowKeepsOfferAndOptionProseAcrossReload`.
- Render the save outcome as negated unconditionally. Expected red: `TestAffordThunderwaveFacts`.
- Drop the detail row lookup. Expected red: `TestAffordBaneDescribedWithZeroEffects`.
- Skip the `Participates` check in the renderer. Expected red: an off-hand case in `TestAffordWarhammerFactsAboveUnchangedEffects`.

---

## S4 — API: field for field (rpg-api, rpg-api#1084)

**Model:** Sonnet.
**Prerequisite:** S1–S3 heads approved by their gates.

Writes, in order:
1. `go.mod`/`go.sum`:
   - Pin root, resolution and session to their approved pseudo-versions.
   - Bump `rpg-api-protos/gen/go` from `v0.0.0-20261008083712-ecc7fef391cd` to `v0.0.0-20261008085529-53694d324223`. The current pin predates #384 and has no `ActionInformation`.
   - Run `go mod tidy`.
2. `internal/handlers/dnd5e/session/v1alpha1/convert.go:2581` `castOptionsToProto`: add `Description: option.Description`.
3. `convert.go:2642` `declarationToProto`: add `Information: actionInformationToProto(d.Information)`. The new helper returns nil for nil and copies the details in order.

### Done when

- `convert_test.go`:
  - `TestDeclarationToProtoCopiesInformation`: available and unavailable offers, description, three ordered details including duplicate labels, and effect rows unchanged beside them.
  - `TestDeclarationToProtoNilInformationStaysNil`
  - `TestCastOptionsToProtoCopiesDescription`: includes an empty description staying empty.
- Gates: `go test ./internal/handlers/dnd5e/session/v1alpha1/...`, then `make ci-check`. Grep its log, because the exit code is not a reliable signal. Draft while on pseudo-versions.

**Deletion-mutant:** drop `Information` in `declarationToProto`. Expected red: `TestDeclarationToProtoCopiesInformation`.

---

## S5 — Web: join and walk (rpg-dnd5e-web)

**Model:** Sonnet for the walk; any finding returns to its owning slice.

At `4c2ad79b` the consumer already reads `declaration.information` in
`actionTooltip.ts` and `option.description` in `ActionDock.tsx` and
`DesktopActionSurface.tsx` (web#1238). It is pinned to `rpg-api-protos#v0.1.230`.
**No source change is expected.** A change appears only if the walk shows a gap
in the consumer.

Walk, on an isolated local stack built from the S1–S4 pushed heads, one click per seam:
1. **Weapon with effects.** A raging barbarian hovers a Warhammer. Base damage, grip and reach show above the Rage row, with no combined total.
2. **Bane, zero effects.** Bane's catalogue description shows with an empty effects section. Its facts read: CHA save with the caster's DC, success negates, Baned on a failed save with Baned's detail, range, up to three targets, and concentration.
3. **Thunderwave.** Inspect it. It shows 2d8 thunder, a CON save where success halves the damage, and the 15 ft cube from your edge that affects others.
4. **Dodge spent.** Use Dodge, then hover it. It is unavailable with its description intact.
5. **Command choice.** Approach, Flee and Grovel each show their description before confirmation. The network log shows only the original option ID sent.

Then reload once. The same text returns and the IDs do not change. Hovering
causes no gameplay RPC. Read each screenshot; do not only save it.

---

## Requirement coverage

| Requirement | Slices | Proof |
|---|---|---|
| R10 typed facts, session renders | S1, S3 | `InformationSuite`, `TestAffordWarhammerFactsAboveUnchangedEffects` |
| R10 one inclusion rule | S1, S2 | `AbilityModifierInformationSuite`, off-hand suites, mutants |
| R11 allow-list selector | S3 | classification guard, prose/option-ID tests, goldens |
| R14 cast facts and condition prose | S1, S3, S5 | `TestBaneCastFacts`, `TestThunderwaveCastFacts`, `TestCureWoundsCastFacts`, `TestEveryCastAppliedConditionHasDetail`, Afford Bane, Thunderwave and Cure Wounds rows, walk |
| R12 prose with its owner | S1, S3 | feature, option and reaction producer tests; session verb tests |
| R5 every choice, unavailable described | S1–S5 | spent Dodge, Command, Wrath, blocked rows, walk |
| R7 one offer, prose not selector material | S3, S5 | `TestSelectorIgnoresProse`, reload in walk |
| Wire unchanged | S4 | converter tests on the published bindings |
