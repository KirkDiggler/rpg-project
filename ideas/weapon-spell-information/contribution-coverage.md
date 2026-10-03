# Contribution coverage — source inventory and migration checks

Investigation record for [#521](https://github.com/KirkDiggler/rpg-project/pull/521),
not a new registry or an implementation plan. Inspected toolkit revision:
[`3955eaf7094c56a713ae055f44600c125081c4ce`](https://github.com/KirkDiggler/rpg-toolkit/tree/3955eaf7094c56a713ae055f44600c125081c4ce/rulebooks/dnd5e).
All source paths below are relative to `rulebooks/dnd5e/` at that revision.

The census covers all **40 condition-loader keys, 12 feature-loader arms and
4 monster-trait-loader arms**, plus the inherent producers listed separately.
It inventories loadable content, a conservative superset of currently offered
content. It does not claim every listed rule is normally obtainable, every legacy
handler is exercised by current session play, or any new assessment is implemented.

The [assessment contract](assessment-contract.md) proposes the extraction shape.
Rows describe measured code and the corresponding migration requirement, not a
fresh statement of tabletop rules. The owning implementation is authoritative;
known defects remain separate issues rather than fixes in the explanation layer.

## Reading the inventory

- **Pure foothold:** current rule already returns unresolved data; adapt/reuse it.
- **Extract:** isolate a pure rule-owned decision from a live handler. Keep its
  execution adapter and lifecycle explicit; remove duplicate predicate logic.
- **Adjacent:** not a term in a scoped actor-side attack/damage/healing calculation,
  but must be explicitly classified. This is not a claim it never affects play.
- **Context:** supplied typed facts are required; absent/unknown cannot be mapped
  to a negative by the information adapter.

“Attack” here means the roll, keep policy and critical threshold. “Damage” means
normal/critical hit branches before target defenses. Save, defense, movement and
post-hit consequences are named separately so a bounded answer cannot masquerade
as a complete interaction forecast. The delivery scope still needs a ruling.

## Condition loader: all 40 entries

Keys use the exact `refs.<namespace>.<method>` symbols in `conditions/loader.go`.
Source paths name the existing evaluator/handler, not proposed file names.

| Loader key | Source / present boundary | Read and migration requirement; lifecycle owner |
|---|---|---|
| `Conditions.Shillelagh` | `conditions/shillelagh.go:WeaponAttackOverride`; `combat/weaponattack/weaponattack.go:Assemble` | Pure foothold: slot/item-bound override. Retain dice/ability/magical provenance; normalize once. Condition keeps expiry/unequip lifecycle. |
| `Conditions.Raging` | `conditions/raging.go:onDamageChain`, save/check chains, `onPostAttackRoll` | Extract outgoing flat damage after effective ability is settled. Separate defensive resistance and STR save/check keep policy. Post-attack sustain and duration/removal remain execution-only. |
| `Conditions.BrutalCritical` | `conditions/brutal_critical.go:onDamageChain` | Extract critical-branch extra dice from effective primary weapon die size; currently rolls inside the fold. No normal-hit extra dice; execution rolls. |
| `Conditions.UnarmoredDefense` | `conditions/unarmored_defense.go:onACChain` | Adjacent AC derivation; needs own sheet/equipment facts. Not an outgoing attack term or permission to publish target AC. |
| `Conditions.FightingStyleArchery` | `conditions/fighting_style_archery.go:onAttackChain` | Extract ranged attack fixed bonus; current handler mutates aggregate bonus, so preserve source at extraction. |
| `Conditions.FightingStyleDefense` | `conditions/fighting_style_defense.go:onACChain` | Adjacent equipped-armor AC bonus. Classify under defense, not outgoing roll arithmetic. |
| `Conditions.FightingStyleDueling` | `conditions/fighting_style_dueling.go:onDamageChain` | Extract sourced flat damage from supplied weapon/grip/other-hand facts. No new equipment lookup in the renderer. |
| `Conditions.FightingStyleGreatWeaponFighting` | `conditions/fighting_style_great_weapon_fighting.go:onDamageChain` | Extract pre-roll policy on the exact primary pool; execution rerolls current faces once, preserves original faces and sourced reroll history. Existing eligibility is not broadened by prose. |
| `Conditions.FightingStyleProtection` | `conditions/fighting_style_protection.go:onAttackChain` | Extract third-party/context predicate and imposed keep source. Current execution automatically spends a reaction inside the modifier; read must not spend or silently turn this into a new optional offer. |
| `Conditions.FightingStyleTwoWeaponFighting` | `conditions/fighting_style_two_weapon_fighting.go:onDamageChain` | Extract positive off-hand ability restoration. Keep base negative-modifier rule separate and prevent double addition; use effective normalized ability. |
| `Conditions.ImprovedCritical` | `conditions/improved_critical.go:onAttackChain` | Extract threshold policy. It changes a roll policy, not attack bonus or guaranteed damage. |
| `Conditions.RecklessAttack` | `conditions/reckless_attack.go:onAttackChain` | Extract separate owner/target keep-source decisions. Current predicates remain with the rule; owner-turn expiration remains lifecycle. |
| `Conditions.MartialArts` | `conditions/martial_arts.go:onAttackChain`, `onDamageChain` | Normalize ability/die once for both attack and damage. Current damage handler rolls replacements after base damage. Paired-rule and RNG-count proofs required; do not reuse live handlers for reads. |
| `Conditions.UnarmoredMovement` | `conditions/unarmored_movement.go:SpeedBonus` | Adjacent movement projection with known/unknown equipment context; not an outgoing damage modifier. |
| `Features.SneakAttack` | `conditions/sneak_attack.go:onDamageChain`, `sneakAttackApplies` | Extract eligibility, unresolved dice and reason from usage + normalized action + context. Current handler rolls and marks used while collecting the chain. Keep owner lifecycle, and keep #1929 separate from information logic. |
| `Conditions.Disengaging` | `conditions/disengaging.go:onMovementChain` | Adjacent movement/reaction suppression; duration remains with condition. No new outgoing formula term. |
| `Conditions.Dodging` | `conditions/dodging.go:onAttackChain`, `onSavingThrowChain` | Extract defender-imposed attack keep source and own DEX-save policy separately. Target disclosure is R4; turn-start expiry is execution-only. |
| `Conditions.Prone` | `conditions/prone.go:attackingWhileProne`, `attackedWhileProne` | Extract own disadvantage and target distance-dependent keep policy. Existing `(within, known)` helper demonstrates why absent distance is not “far away.” |
| `Conditions.Hidden` | `conditions/hidden.go:onAttackChain` | Extract attacker/target keep decisions. Owner attack removes Hidden during collection; selected source must survive removal. Hidden target data is not automatically public. |
| `Conditions.Inspired` | `conditions/inspired.go:onPostRollOffer`, `onOfferTaken` | Pure offer foothold, but new advance opportunity needed. Not an automatic die. Execution poses after roll and consumes only on accepted offer; rest/combat expiry stays with owner. |
| `Conditions.Helped` | `conditions/helped.go:onAttackChain` | Extract advantage before consumption. Current collection removes the benefit; information must not publish removal or unsubscribe. Preserve helper-turn expiry. |
| `Conditions.Unconscious` | `conditions/unconscious.go`; root life-state transitions | Adjacent participation/life state. Existing condition handlers are inert compatibility hooks; do not invent a contribution from the condition name. Audit actual life-state consumers, not just subscriptions. |
| `Conditions.OpportunityAttack` | `conditions/opportunity_attack.go:onMovementChain`, `onReactionTaken` | Adjacent movement-triggered reaction. Its eventual strike must use shared assessments; reading weapon information never triggers movement or spends a reaction. |
| `Conditions.TrueStrike` | `conditions/true_strike.go:onAttackChain` | Extract target-bound advantage and pending target need. Current matching attack consumes during collection; do not add a new next-turn restriction in information. |
| `Conditions.BladeWard` | `conditions/blade_ward.go:onDamageChain` | Adjacent defensive multiplier for qualifying weapon components. Do not apply it to actor-side pre-defense damage; duration/removal stays with owner. |
| `Conditions.ViciousMockery` | `conditions/vicious_mockery.go:onAttackChain` | Extract imposed disadvantage. Current attack collection consumes the condition; turn/combat expiry remains distinct. |
| `Conditions.Commanded` | `conditions/commanded.go`; `resolution/obey.go` | Adjacent compelled behavior and expiry, not a numeric contribution. Existing legality/turn-driving surfaces remain authoritative. |
| `Conditions.Concentrating` | `conditions/concentrating.go:onDamageTaken`; `resolution/damagetaken.go` | Adjacent hold/child lifecycle and post-damage save. Describing a new cast must not end concentration or preselect stale contributions across its execution-time removal. |
| `Conditions.Blessed` | `conditions/blessed.go:RollContributionMetadata`, `DescribeRollContributions` | Pure foothold: attack/save dice and stacking metadata. Retain suppressed evidence; reads roll no d4 and never attach a bus. |
| `Conditions.Baned` | `conditions/baned.go:RollContributionMetadata`, `DescribeRollContributions` | Pure foothold: subtractive attack/save die. Reuse recipient selection; do not negate notation or create a second source identity. |
| `Conditions.DivineFavor` | `conditions/divine_favor.go:onDamageChain` | Extract weapon rider dice and duplicate suppression; current fold scans prior contributions, then rolls. Preserve first contributing instance and critical treatment without rolling discarded candidates. |
| `Conditions.FaerieFire` | `conditions/faerie_fire.go:onAttackChain` | Extract target-carried keep source. Use only permitted target facts for information; lifecycle remains with its concentration/removal owner. |
| `Conditions.ShieldOfFaith` | `conditions/shield_of_faith.go:onACChain` | Adjacent sourced AC contribution, not actor attack arithmetic or publicly known target defense. |
| `Conditions.Guided` | `conditions/guided.go:onPostCheckRollOffer`, `onOfferTaken` | Check-specific opportunity, not an attack or save bonus. Consumption only on accepted check offer; concentration/rest lifecycle stays separate. |
| `Conditions.Resistance` | `conditions/resistance.go:onPostSaveRollOffer`, `onOfferTaken` | Save-specific opportunity, not caster DC or automatic save dice. Existing offer-taking boundary owns consumption. |
| `Conditions.Sanctuary` | `conditions/sanctuary.go`; `resolution/sanctuary.go` | Not a chain arithmetic provider: resolution owns ward saves/self-break. Keep nonnumeric consequences distinct from pre-defense formulas; reading never triggers a ward save or self-break. |
| `Conditions.SanctuaryImmune` | `conditions/sanctuary_immune.go`; cast eligibility | Adjacent recipient reapplication restriction, not an aggressor damage bonus or ward-save bypass. Preserve existing cast-policy ownership. |
| `Conditions.InFog` | `conditions/in_fog.go`; `resolution/visibility.go` | Marker/lifecycle alone is not the contribution. Sight's inherent rule produces keep sources from encounter facts; avoid counting both a “fog modifier” and the sight modifier. |
| `Conditions.GuidingBolt` | `conditions/guiding_bolt.go:onAttackChain`, `onRolled` | Extract target-carried advantage. Consumption occurs after actual roll, before pause, not during information or attack collection. |
| `Spells.Shield` | `conditions/shield_spell.go:onPostAttackRoll` | Outcome/readiness-dependent reaction trigger; not a guaranteed pre-roll AC addition. Do not run the trigger or expose unpublished hit/AC information in an advance read. |

The condition registry includes feature- and spell-namespace refs. A census based
only on `refs.TypeConditions` or `conditions.DISPLAY` misses real providers.

## Feature loader: all 12 arms

Feature activation and an already-held condition are different producers. A
feature button is not evidence its benefit is already affecting an attack.
`character/sheet_keeper.go:applyFeatures` is the separate attach path for
features implementing lifecycle behavior.

| Loader key | Source | Classification / migration requirement |
|---|---|---|
| `Features.BlessingOfTheTrickster` | `features/blessing_of_the_trickster.go` | Adjacent activation/status implementation; no attached attack/damage-chain producer found. Do not invent passive terms. |
| `Features.BardicInspiration` | `features/bardic_inspiration.go` | Activation grants the Inspired condition. The recipient's held effect, not the bard's feature ownership, supplies the advance opportunity. |
| `Features.Rage` | `features/rage.go` | Activation grants Raging; keep cost/grant lifecycle out of assessment and avoid counting the feature plus condition twice. |
| `Features.SecondWind` | `features/second_wind.go` | Separate active healing action, not a weapon/spell passive modifier. Wider action-information support would need its own declaration, not a guessed healing term. |
| `Features.WardingFlare` | `features/warding_flare.go:onAttack` | Extract before-roll opportunity predicate from target state/resources/sight. Execution creates concrete offer and owns resource/reaction spending; do not impose automatic disadvantage in an advance read. |
| `Features.WrathOfTheStorm` | `features/wrath_of_the_storm.go:onHit` | Extract post-hit opportunity and typed alternatives. Not outgoing attack damage; a hit and answer are not established in an advance explanation. |
| `Features.ActionSurge` | `features/action_surge.go` | Adjacent activation/action economy; existing permission/status surfaces own it, not attack arithmetic. |
| `Features.FlurryOfBlows` | `features/flurry_of_blows.go` | Adjacent activation/capacity grant; resulting strikes use the shared evaluator. Do not treat capacity as damage. |
| `Features.PatientDefense` | `features/patient_defense.go` | Activation grants Dodging; only the held condition supplies current policy. |
| `Features.StepOfTheWind` | `features/step_of_the_wind.go` | Adjacent movement/economy activation, outside scoped attack/damage/healing terms. |
| `Features.RecklessAttack` | `features/reckless_attack.go` | Activation grants the separately inventoried condition; no duplicate feature term. |
| `Features.DeflectMissiles` | `features/deflect_missiles.go:onDamageReceived`, `calculateReduction` | Legacy reactive handler with RNG and state. Adjacent defender outcome, not a read-safe evaluator; any later support needs an explicit extraction, not direct invocation. |

`features.DiscipleOfLife` is **not** one of these loader arms. It is a pure
healing contributor invoked from `character/casting.go:CastDefinition`, and must
be covered at that assembly site. Loader completeness alone is insufficient.

## Monster trait loader: all four arms

| Loader key | Source | Classification / migration requirement |
|---|---|---|
| `MonsterTraits.Immunity` | `monstertraits/immunity.go:onDamageChain` | Adjacent target defense multiplier; only belongs in an explicitly permitted defense/outcome scope. |
| `MonsterTraits.Vulnerability` | `monstertraits/vulnerability.go:onDamageChain` | Adjacent target defense multiplier; preserve component/type scope, not an actor-side modifier. |
| `MonsterTraits.PackTactics` | `monstertraits/pack_tactics.go:onAttackChain`, `allyAdjacentToTarget` | Extract attack keep predicate from bounded participant/position/relationship facts. No assumption that an absent neighborhood means no ally. |
| `MonsterTraits.UndeadFortitude` | `monstertraits/undead_fortitude.go:onDamageReceived` | Adjacent post-damage survival behavior with saving-throw RNG. Never run while inspecting an attack; outcome not promised by damage-before-defenses. |

`monstertraits.LoadJSON` currently requires a roller when loading Undead Fortitude;
`LoadMonster` alone leaves trait blobs unassessed. A bus-free read therefore needs
loader-owned pure snapshot construction, not attachment with a dummy roller or a
second decoder in resolution. Keep execution's roller requirement at its runtime
boundary. This is a measured migration seam, not a claim that all current loaders
already satisfy the proposed read contract.

## Inherent producers and non-condition seams

| Source | Existing evidence / required extraction |
|---|---|
| `combat/weaponattack/weaponattack.go:Assemble`; `character/attack_definition.go` | Weapon/grip/ability/proficiency/pools. Aggregate `AttackBonus` loses proficiency provenance; emit it at derivation. Preserve selected equipment instance and override evidence. |
| `character/casting.go:CastDefinition`; `spells/cast.go` | Spell attack bonus and save DC assembly; source evidence must be retained at construction, not inferred from the totals. Content remains canonical and actor-free catalogue information remains independent. |
| `features/disciple_of_life.go:DiscipleOfLife`; `healing/healing.go:Declaration` | Existing pure sourced healing terms and separate rolling. Reuse the declaration; do not call `healing.Resolve` from a read. Potential healing is not guaranteed HP restored. |
| `resolution/strike.go:rollDamage` | Base pool rolling, critical treatment and off-hand positive/negative ability policy. Extract unresolved base terms before rolling; keep source presence for a real zero modifier. |
| `resolution/visibility.go:addSightAttackModifiers` | Two directional sight facts → granted/imposed keep sources. Current unknown visibility contributes nothing; information must report its scope/pending facts, not claim a complete target-aware roll. Use the same pure rule function from execution and explanation. |
| `rolls/resolve.go`; `events/roll_trace.go`; `saves/saves.go` | Selected unresolved dice, arithmetic and sourced resolved traces. Save/check ownership must not acquire an actions import cycle when common values move below events. |
| `resolution/strike_pose.go` and other offer continuations | Frozen calculation/offer authority. Read is detached projection; accepting rolls only the offered benefit. Live effect removal cannot erase frozen source evidence. |
| `resolution/sanctuary.go`, `resolution/damagetaken.go`, `combat/life_state.go` | Gates, later consequences and life state are not ordinary additive modifiers. Classify the scope honestly; do not rewrite legality, survival or concentration in an information fold. |

## Migration proof matrix

These are proposed acceptance checks, **not new passing tests**. A mapped producer
is not a migrated producer. Each extraction needs paired explanation/execution
coverage and its legacy callers checked or retired.

| Contract claim | Discriminating assertion |
|---|---|
| Rule ownership | Change a test rule's returned decision once: both explanation and execution change, with no rule-ref branch above its owner. |
| Coverage | Every enumerated loader key and every inherent producer has an explicit operation/facet classification. An added provider without coverage fails the census. |
| Purity | Repeat reads and compare serialized input/output state; no roller calls, events, spending, removals, clock ticks or duration changes. Mutating returned slices/refs cannot alter the next read or live state. |
| Assembly provenance | STR, proficiency, weapon, overrides and off-hand omissions are evidenced where derived. No subtraction from aggregate attack bonus to infer sources. |
| Normalization | Martial Arts + Rage, Shillelagh + dependent ability rules, and off-hand ability restoration agree on the effective ability. No discarded base-damage roll; GWF still executes only after real faces exist. |
| Selection | Duplicate Bless/Bane and Divine Favor produce the intended one contribution, retaining suppressed source evidence. Unknown earlier selection cannot silently produce a certain later winner. |
| Unknown versus negative | Missing target/position/universe stays pending; known failed eligibility returns a reason. Already-spent usage can decide without target facts. |
| Sneak Attack boundary | Both consumers use the same owning predicate; no display-only repair for toolkit#1929. A later predicate correction requires no effect-specific UI change. |
| One-use lifecycle | Help/Hidden/True Strike/Mockery keep their collection-time transition; Guiding Bolt waits for roll; Inspiration waits for accepted answer. Reads perform none of those transitions. |
| Automatic versus optional | Protection retains its existing automatic execution behavior; Warding Flare/Inspiration remain opportunities until their real choice boundary. No blanket conversion between them. |
| Frozen continuation | Bane/Bless and d20 are unchanged on resume; spend rolls only offered die, keep spends nothing. Tampered data still fails. |
| Hidden-state noninterference | Identical permitted inputs yield identical expressions, source rows, pending rows and unavailable/error shape even when hidden opponent state changes. |
| Unsupported facet | Unsupported damage assessment preserves description and a supported attack calculation, removes settled damage expression, and does not change action legality. Unknown impact suppresses all potentially affected calculations. |
| Scope honesty | Actor-side pre-defense damage is never labeled final HP loss; caster Bless does not become spell DC; defensive traits and third-party opportunities do not leak through partial formulas. |

## Baseline verification, 2026-10-03

Executed from the isolated `rpg-toolkit/.worktrees/contributions-review` checkout
at the pinned revision above, with no source or dependency changes:

```sh
# cwd: rulebooks/dnd5e
GOWORK=off GOPROXY=off go test -mod=readonly \
  ./conditions ./rolls ./healing ./features ./monstertraits -count=1

# cwd: rulebooks/dnd5e/resolution
GOWORK=off GOPROXY=off go test -mod=readonly . \
  -run 'Test(BaneCalculationIsFrozenAcrossInspirationAnswers|BaneAttackUsesOneSelectedContributionAndRecordsCalculation|ATamperedFrozenBlobIsRefused|SightAttackModifiers|SanctuaryBlocksAnAttackOnAFailedWardSave|AttackingEndsTheAttackersOwnSanctuary)$' \
  -count=1
```

Both commands passed. Root tests use core v0.11.0/events v0.6.2. Resolution tests
use their own pinned dnd5e v0.196.0, encounter v0.103.1, core v0.12.0/events v0.6.3;
they do **not** compile against the sibling root module merely because it is in
the same worktree. These tests establish current behavior only, not the proposed
assessment interfaces, cross-module integration, API mapping, browser acceptance,
or an independent review.
