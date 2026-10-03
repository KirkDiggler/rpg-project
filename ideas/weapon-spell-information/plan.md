# Weapon and spell information — working plan

Status: design investigation, not an implementation-ready handoff. R1, R2 and
R6–R15 in [design.md](design.md) are settled. Catalogue and current-action work
share the designed connection but remain separate delivery increments. The current
focus is the contribution contract and coverage inventory after the bounded
creation-description checkpoint. Resolution-specific R3–R5 contracts still need
agreement and checked handoffs; this document is not an implementation-ready plan.

Current contract: [read-contract.md](read-contract.md).
Concrete task handoffs: [implementation-plan.md](implementation-plan.md) — C1 is
implemented in draft toolkit#1932 with owning-module checks green; the target-aware
assessment/read wave is not yet handoff-ready. The source/projection gaps are explicit there. This working
exploration remains background, not the executable task list.

Tracking: [rpg-project#520](https://github.com/KirkDiggler/rpg-project/issues/520).
Related earlier weapon-damage gap: [#307](https://github.com/KirkDiggler/rpg-project/issues/307).

## Current design artifact

[Toolkit contract proposal](toolkit-contract.md) is the current concrete draft:
package dependencies, shared values, operation-specific assessments, composition,
worked cases, migration owners and measured baseline tests. R10 settles initial
information access independent of command eligibility and selection. Section 11
separates catalogue alternatives from actual character actions and proposes the
remaining bounded-context and freshness rules. It is not implemented output,
an approved wire schema or an implementation-ready handoff.

[Contribution contract sketch](contribution-sketch.md) is the introductory worked
example. R8 settles the Inspiration opportunity/offer behavior and frozen
continuation; R9 settles explicit action-fact normalization. The remaining
contracts in R3–R5 remain open.

## Contributions continuation

The operator clarified the player-facing requirement: show which effects can apply
with tooltips describing them. An active Sneak Attack indication with a useful
tooltip is the proof, not a results-preview panel. The shared mechanical contract
serves that presentation and execution; it does not authorize a dashboard redesign.
The [consumer trace](effect-info-delivery.md) pins the existing source paths and
acceptance scenarios. R15 settles the measured UI gap: hover/focus inspects effects,
touch gets an explicit read-only inspection affordance, and target clicks retain
immediate execution. No new confirmation step or information-fetch command gate.

The character-creation description checkpoint is implemented separately from the
broader catalogue proposal: toolkit#1927 merged; web#1218 has a scoped passing
review. That does not establish full catalogue browsing, API release adoption or
current-action contributions. The operator's current focus is the contribution
contract and coverage inventory, not finishing the catalogue browser first.

- [Assessment contract](assessment-contract.md): concrete proposed operation
  inputs/outputs, rule binding, per-facet coverage, normalization, calculation
  availability and execution custody. R3/R4 and remaining R5 interfaces are still
  proposals; no gameplay implementation is claimed.
- [Coverage inventory](contribution-coverage.md): source-checked census of 40
  condition-loader keys, 12 feature-loader arms, four monster-trait arms and the
  inherent rule/assembly producers. It includes lifecycle boundaries, required
  migration assertions and freshly executed baseline checks.
- R12 settles unsupported-assessment behavior: retain description, make the
  affected calculation unavailable, leave independent legality unchanged.
- R13 settles permitted character knowledge as the information boundary, with no
  hidden-state inference and unknown facts left unresolved. Execution still uses
  authoritative state; the exact projected fields and observation sources remain
  R4 work.
- R14 settles the initial scope as effects/contributions, not outcome prediction:
  attack roll, normal/critical damage before defenses, spell DC/potential healing,
  conditional effects and opportunities. Permitted selected-target facts refine
  applicability; no simulated rolls, hit probabilities or predicted final HP loss.
- Sneak Attack's Dexterity-as-weapon proxy is toolkit#1929, deliberately separate.
  Both consumers use its owning predicate; information contains no compensating
  rule. The inventory identifies broader extraction needs without authorizing
  unrelated gameplay corrections.

Next contract work: close the member-visible context sources in
[read-contract.md](read-contract.md) and complete the shared assessment interfaces
for R15's settled inspection interaction,
and derive acceptance within R14's settled scope. Existing position/standing
observations are mapped in the consumer trace; observer-known relationships
between two other participants and target-effect knowledge need provider work.
Then derive checked implementation handoffs rather than treating these source
inventories as an executable task plan.

## Broader catalogue follow-on (separate from current focus)

[Catalogue implementation plan](catalogue-implementation-plan.md) records the
earlier provider checkpoint, red/green evidence and remaining source/transport/UI
handoffs. The bounded creation work is distinguished above; the broader catalogue
read/browser is not complete and is not a prerequisite for this continuation.

The following remains the catalogue follow-on outline across root dnd5e content,
protos, API and web. It does not require the contribution packages or resolution
read to ship.

1. Inventory the canonical weapon/spell information already available; identify
   missing descriptions and structured base facts for the permitted choices.
   Keep authored content and executable mechanics connected rather than create
   a second table of numbers for display.
2. Define the catalogue read contract by canonical ref, including the exact
   choices/grants and later inspection of unchosen alternatives. Do not rely on
   the UI's hardcoded level-0/1 fetch or require a current executable offer.
3. Name the provider functions, proto fields/endpoints, API mappings and existing
   UI consumers. Use the existing enriched equipment path where it fits.
4. Cover honest base information and implementation status without introducing
   new spells, imaginary character modifiers or a dummy resolution interaction.
   Current-action values and live effects are explicitly not catalogue claims.
5. Prove normal creation, saved selection/reload, and reading alternatives without
   changing choices or affordability. Add provider coverage, API mapping and
   browser checks with exact handoff/release dependencies.
6. Keep the UI work to exposing and inspecting the information, not a general
   UX redesign. The catalogue increment is independently useful but does not
   complete the initiative's current-action/Rage/Bless requirements.

This is the plan to derive next, not a substitute for the concrete per-repo
implementation handoffs it calls for.

## Resolution follow-on — not catalogue prerequisites

The first concrete toolkit proposal is written and checked against current source
and focused baseline tests. Before starting its gameplay implementation, complete
the following contract checks:

1. **Drafted:** shared data vocabulary below an action-specific assessment
   package, with proposed rule interfaces, the resolution read entry and the
   detached result projected by session. The production import graph was checked
   to avoid an actions → saves → contributions → actions cycle.
2. **Normalization direction settled, R9:** complete payload, missing-context and
   stacking contracts around explicit normalization before dependent contributions.
   Keep actual face operations such as GWF rerolls at their post-roll boundary,
   with the existing sourced trace preserved.
3. **Worked on paper, not implemented:** Bless + Rage, an ability/die replacement,
   target-dependent eligibility, and Inspiration's advance read → frozen offer →
   spend/keep continuation; also save/healing spells and authored non-numeric
   content and GWF's pre-roll policy/post-roll trace. Paired-rule ordering and
   visibility require discriminating tests, not only these examples. The existing
   GWF suite also passes; this does not test the proposed assessment component.
4. **Inventoried; implementation handoffs pending:** chain-only handlers, assembly
   provenance, inherent visibility/healing producers, feature/trait paths and
   distinct lifecycle boundaries. No tooltip-only predicates, generic consumption
   callback or quiet omissions from an answer claimed to be complete. See
   [contribution-coverage.md](contribution-coverage.md).
5. **Independent inspection settled, R10:** current `Afford` omits specific action
   identities off-turn and while frozen. Information remains accessible without
   an executable offer, including unchosen catalogue alternatives after selection.
   Existing affordability messages and legality stay unchanged. Define the exact
   public read/choice-detail delivery shape and independent freshness; selector
   IDs are not information revisions. Current Afford/selector baseline tests pass.
6. Resolve only architectural decisions uncovered by those examples. Then derive
   the concrete module/API/web handoffs and verification commands in the checked
   implementation plan, preserving provider-first release adoption.

## Objective

A player can understand a weapon or spell while choosing a character and while
using that character's current actions. Active effects such as Rage and Bless
are required information, not a deferred enhancement. The layout redesign is
separate from delivering truthful information.

## Measured starting point

Snapshots: project `c0010a0`, toolkit `b784cf79`, API `fa1a0779`, protos
`729defe`, web `9cfd4ca6`. All inspection paths are under each repository's
`.worktrees/choice-info-scout`. Findings are source inspection, not browser or
runtime acceptance.

- Creation weapon choices already carry enriched equipment information.
- Creation spells use ref choices plus `ListSpellsByLevel`. `SpellInfo` already
  carries `description` (`rpg-api-protos/dnd5e/api/v1alpha1/character.proto:1014`).
  Web retains the catalog objects but supplies only the name to the picker
  (`rpg-dnd5e-web/src/components/ChoiceRenderer.tsx:212`). The catalog hook only
  requests levels 0 and 1 (`src/api/useSpellCatalog.ts:14`).
- In-play identity messages are deliberately thin: AttackRef is ref/name/damage
  type, SpellRef is ref/name (`rpg-api-protos/dnd5e/api/session/v1alpha1/types.proto:1411,1458`).
  The action tooltip explicitly records missing weapon arithmetic
  (`rpg-dnd5e-web/src/components/session/combat-experience/actionTooltip.ts:10`).
  Existing issue: https://github.com/KirkDiggler/rpg-project/issues/307 (open,
  previously deferred; narrower than this effort).
- The API scout verified the relevant fields against its pinned generated Go
  module and toolkit versions: dnd5e v0.197.0 and session v0.112.0. Toolkit HEAD
  and an API pin are distinct evidence; do not assume changing the former changes
  the latter.
- Spell content holds execution profiles and a separate incomplete prose catalog
  (`rpg-toolkit/rulebooks/dnd5e/spells/cast.go`, `spells/data.go`). Do not turn a
  catalog completeness fix into implementing every catalogued spell.
- `character.AssembleAttack` explicitly compiles static evidence only; situational
  effects contribute during resolution (`character/attack_definition.go:24`).
  The shared assembler makes the same boundary explicit
  (`combat/weaponattack/weaponattack.go:21`). Therefore an action definition
  alone does not establish a complete current-action explanation.
- Bless/Bane already implement a pure unresolved-contribution capability:
  `events/roll_trace.go:47`, `conditions/blessed.go:151`,
  `conditions/baned.go:152`. The recipient selects one provider per declared
  stacking group before asking it to describe (`conditions/baned.go:190`).
  Strike consumes those descriptions before rolling (`resolution/strike.go:477`).
- Rage's damage predicate and contribution live inside the damage chain
  (`conditions/raging.go:413`). It checks the effective ability after earlier
  modifiers, not just the initial action's ability. The contribution extraction
  must preserve that ordering.
- Damage dice are rolled before the damage chain (`resolution/strike.go:651`).
  Running a live chain as a tooltip dry run is not established as safe.

## Approaches to resolve for R3

1. **Independent informational rules.** Add tooltip-only calculations or tables.
   This duplicates applicability, stacking and arithmetic. It conflicts with the
   workspace's source-of-truth rule and is not recommended.
2. **Execute on copied state and discard the result.** Reuses more existing code,
   but current paths roll, consume effects, and invoke execution behavior. A clone
   alone does not establish read purity or correctly explain unknown context.
   Not recommended as the information primitive.
3. **Shared, unresolved rule contributions.** Extend the existing describe-before-
   roll direction: a rule decides its contribution once; explanation reads that
   decision, while execution rolls/applies it and performs consumption at the
   correct lifecycle point. Recommended direction, not yet a settled interface.
   Preserve staged transformations, known/unknown context, and sourced modifiers.
   Do not build a general expression language merely to describe this content.

## Candidate information requirements (R5 proposal)

- Shared content: name, concise effect, damage/healing, range/reach, targeting,
  costs, saving throw semantics, concentration/duration and relevant restrictions.
- Current action: resolved weapon/grip/ability, sourced attack and damage/healing
  contributions, current costs and availability, applicable active effects.
- Explain the contribution, not just an effect badge: Bless adds a die to an
  attack roll, not weapon damage; Rage changes eligible melee Strength damage,
  not every action of a raging character.
- Retain conditional alternatives where a target or outcome is not known. Do not
  promise one exact final damage number before a hit, save or resistance resolves.
- Read responses must preserve absence versus zero and must not grant authority
  to execute. Existing action selectors/availability remain the execution contract.

## Proposed proof obligations

These are requirements to check, not claims of passing tests.

1. Normally create and finalize a martial and caster; inspect their offered
   weapon/spell choices and current actions through API and browser.
2. Compare descriptions with execution's sourced contributions at the same state.
   Include unbuffed, Rage-eligible/ineligible and Bless/Bane cases, not just names.
3. Prove information reads do not roll, spend, refresh duration, consume a one-use
   effect, sustain Rage, or mutate any participant; repeat the read and compare.
4. Show current information updating when effects begin/end, including a buff
   caused by another player, concentration ending, and state reloading.
5. Cover grip and off-hand rules, attack-roll versus save spells, healing, and a
   non-damaging effect. Preserve damage types and source identity.
6. Check stacking and advantage/disadvantage cancellation using the same rules as
   execution. Do not silently drop a modifier unsupported by the explanation path.
7. Resolve R4 before adding selected-target proof. If implemented, prove the read
   cannot reveal hidden target state or missing participants through its output.
8. Verify provider, API mapping and browser separately at exact revisions; a green
   provider test is not end-to-end proof.

## Ownership and sequence to refine

- **Root dnd5e module:** canonical content, contribution vocabulary and rule-owned
  selectors; migrate touched execution consumers to the same decisions.
- **Resolution module:** interaction interpretation and any read entry requiring
  cross-participant rule evaluation; no new rules in the host seam.
- **Session module:** load/coordinate/project current information as detached data.
- **Protos:** the agreed public information contract, without exporting execution
  internals or condition JSON to the client.
- **API:** mapping and access control, preserving presence and source identity.
- **Web:** creation details and action information, rendering rather than deriving.

Develop the consumer requirement first. Protos can publish the agreed contract;
Go providers publish before consumer adoption. Toolkit changes follow nearest-
`go.mod` PR boundaries. Exact changed interfaces, module handoffs, commands,
branch/release sequence and acceptance cases must be filled in after R3–R5; this
outline is not a substitute for a concrete checked implementation plan.

## Follow-up findings and R4 proposal

The background modifier scout completed; parent inspection confirms the essential
split between descriptive dice providers and execution handlers. Source-level
checks, not the scout's rule summaries, govern exact mechanics.

- Bless/Bane describe unresolved dice. They are a real shared-read precedent, not
  proof that every modifier is already queryable.
- `conditions/helped.go:157` consumes the one-use benefit in its attack-chain
  handler. A read cannot safely reuse that handler unchanged.
- `conditions/sneak_attack.go:200` checks turn state and context, rolls dice, and
  then marks the use spent (`:279`). Extracting only a formula is not enough;
  eligibility and consumption must have distinct homes in the shared design.
- `conditions/divine_favor.go:184` contributes a sourced radiant dice pool during
  the damage fold. It rolls one d4, or two for a critical, not a per-level pool.
  The scout's per-level shorthand is incorrect and is not a design requirement.
- Source correction: Sneak Attack currently uses DEX as a proxy for weapon
  eligibility (`sneak_attack.go:215`), explicitly marked TODO. This is not proof
  of full finesse/ranged eligibility. Do not silently bundle a rules correction
  into information work or author text promising a rule execution does not use.
- Guiding Bolt's consumption is a post-roll handler (`guiding_bolt.go:267`), not
  the same lifecycle point as Helped. One generic 'consume on inspection/attack'
  callback would erase an important distinction.

The information must distinguish contributions that apply, requirements not yet
settled, and optional future choices. An active effect is not necessarily an
applicable modifier to every action. Inspiration is available to choose after a
roll, not already added to it; target-dependent effects are not unconditional
actor bonuses.

**Visibility boundary settled, R13:** information uses permitted character
knowledge, not hidden authoritative facts. The concrete R4 projection proposal
shows current actor contributions and conditional requirements before selection,
then refines them with permitted selected-target context.
Do not turn that refinement into a query for hidden defenses, unseen creatures,
or another player's future reaction choice. A non-applicable or unknown result
must not be mistaken for a missing effect.

R13 settles visibility; selected-target refinement still needs the concrete
projection fields, observation sources and API shape. Its architectural fit must
be designed now; shipping a selected-target UI is a separate delivery decision,
not the next prerequisite to understanding the component.

## Architecture-first direction

Operator clarification: prioritize the component, its composition, and how future
uses fit rather than choosing UI slices. R7 settles the layer ownership: session
carries inputs and answers; resolution assembles the explanation using supplied
context and rule-owned contributions. Shared rule-owned evaluation producing
unresolved contributions, with explanation and execution as distinct consumers,
is the mechanism to refine inside that boundary. Its interfaces are not settled.

Test that boundary against these questions before enumerating endpoint fields:

- Can catalog information exist without inventing a character or a zero modifier?
- Can actor and action context establish contributions without selecting a target?
- Can missing target context remain explicitly unresolved rather than false?
- Can additional permitted context refine the same explanation without a second
  rules implementation or leaking private truth?
- Can a rule replace a die or ability before another rule tests applicability,
  preserving staged ordering rather than adding every modifier in a flat list?
- Can optional spending and outcome-dependent effects remain alternatives rather
  than being included in a guaranteed total?
- Can execution consume the same rule decisions while keeping rolls, spending,
  effect consumption and state changes out of the explanatory read?
- Can another effect compose through an existing contribution shape without
  changing session, API or client logic by effect identity?

The result need not be one universal object containing all rules. Keep authored
content, evaluation context, unresolved mechanical contributions, presentation,
visibility, and execution lifecycle as distinct responsibilities. Use current
rules to justify the smallest typed vocabulary; do not invent a general-purpose
predicate language to represent hypothetical content.
