# Effect information — implementation handoffs

**Implementation in progress.** C1 is implemented in
[toolkit#1932](https://github.com/KirkDiggler/rpg-toolkit/pull/1932) at `f94ed5a7`;
its owning-module checks pass. C2
[toolkit#1933](https://github.com/KirkDiggler/rpg-toolkit/pull/1933) is merged as
`f06cacf2`; CI published `rulebooks/dnd5e/encounter/v0.111.0`, verified to point at
that merge. Its encounter-module race tests/lint and normal commit hook passed.
C3–C9 below name the remaining contracts, owners, dependency order and distinguishing
checks. Consumer adoption and joined acceptance remain pending; planned checks
are not reported as passes. These provider checkpoints do not complete #520.

## Goal, authority and constraints

Show provider-assessed effects for the current action/inspected target, with
canonical tooltips; preserve gray/ineligible, conditional and optional states.
Hover/focus/touch inspection is read-only, and clicking retains existing commands.
Authority: [design.md](design.md), R7–R15; [consumer acceptance](effect-info-delivery.md).
The [read contract](read-contract.md) and [assessment contract](assessment-contract.md)
record the design exploration; the concrete task contracts below resolve build
details under those settled semantics, without adding eligibility or disclosure rules.

Non-goals: outcome prediction, a calculation dashboard, a full catalogue browser,
new spells, a new command confirmation, or correcting Sneak Attack in information
code. The owning-rule defect remains toolkit#1929.

Owning instructions: `rpg-toolkit/CLAUDE.md`, `rulebooks/dnd5e/CLAUDE.md`, the
character package's `CLAUDE.md`, and the consumer repositories' own instructions.
One nearest-Go-module per toolkit PR; publish provider commits for development,
then adopt CI release tags before consumer merges. No automatic merge.

Inspected baselines: toolkit `3955eaf7`; C1 worktree starts at `a088baa9`
(`origin/main`, identical root-dnd5e source to the inspected baseline); web
`af4ee892`; API `858c3e5a`; protos `e80efe0`; project design `e0cf1b9` plus the
current read-contract draft. Exact
source inventory and earlier baseline commands are linked from the consumer trace
and [coverage inventory](contribution-coverage.md).

## Task C1: Canonical tooltip content for the three motivating effects

**Delivers:** R7/R14 and the canonical-content portion of the effect-tooltip
acceptance: Sneak Attack, Raging and Blessed have actual descriptions under their
existing content owner. No eligibility calculation, contribution engine, or UI
integration is delivered by this checkpoint.

**Owner:** `rpg-toolkit`, root `rulebooks/dnd5e` module; conditions owns content and
character owns its existing status projection.

**Prerequisites:** existing `conditions.DisplayFor`, `character.StatusView` and
valid loaded condition fixtures. No new proto, consumer release, context provider,
assessment API or running stack is needed to execute this task.

**Files:**

- Modify `rulebooks/dnd5e/conditions/display.go`: the existing entries for
  `refs.Features.SneakAttack()`, `refs.Conditions.Raging()` and
  `refs.Conditions.Blessed()` only.
- Modify `conditions/display_test.go`: update existing exact expected displays
  for Raging/Sneak Attack and add the descriptor assertions below.
- Modify `character/status_view_test.go`: extend
  `TestStatusViewRogueProjectsSneakAttackCondition` to prove canonical detail is
  copied; add a named case for the Raging/Blessed projection.
- No handler, predicate, loader dispatch, price, resource, dependency pin or
  selection-list changes.

**Interfaces:** preserve `DisplayFor(core.Ref) (Display, bool)`, `Display{Name,
Detail}` and `Character.StatusView(*StatusViewInput) (*StatusViewOutput, error)`.
`ConditionView.Detail` must equal the returned canonical descriptor's `Detail`;
`SourceMember` behavior and the condition ref remain unchanged. Do not add a
parallel `tooltipDescriptions` registry or a new transport field for data already
represented by `Detail`.

**Behavior:** add original, truthful descriptions of the implemented benefits.
Sneak Attack explains extra damage, once-per-turn use and the advantage/nearby-
enemy alternatives without claiming its known weapon-eligibility defect is fixed.
Raging explains the eligible melee-Strength damage benefit, advantage on
Strength-based skill checks and Strength saves, and physical resistance; do not bake its current instance's level-based
bonus into universal copy. Blessed explains its d4 on attacks/saves and nonstacking
of duplicate Bless instances, not a damage bonus. Instance-specific amounts and
contextual eligibility reasons remain the future shared assessor's responsibility.
Unknown refs still return false; Shield's existing status-catalog exclusion stays
unchanged. Reading content mutates no condition or character.

**Tests:**

- New `TestEffectTooltipContentSuite`: all three canonical refs are found, names
  are unchanged and details are nonempty. Exact expected copy is pinned after
  checking it against the owning implementations; no test introduces a new
  gameplay-eligibility assertion.
- Existing `TestDisplayForKnownConditions` updates the intentional new detail
  rather than deleting its equality assertions.
- Existing rogue status-view test plus new
  `TestStatusViewProjectsCanonicalEffectDetails`: detail matches `DisplayFor`,
  refs/source identities remain correct and serialized character data before and
  after the read is equal, normalizing only `ToData`'s generated `UpdatedAt` stamp.
  That field is serialization time, not mutable character state. Use existing
  fixture construction patterns.
- Existing unknown-ref and Shield exclusion regressions remain green.

- [x] Add nonempty-detail assertions; demonstrate they fail against name-only entries.
- [x] Author checked canonical copy in the existing three entries.
- [x] Update exact display expectations and status projection tests.
- [x] Run focused and owning-module gates; record actual outputs on the PR body.
- [x] Publish a root-module checkpoint PR linked to #520; do not claim live tooltips.

**Verification:** from the task worktree's `rulebooks/dnd5e` directory:

```sh
GOWORK=off go test -mod=readonly ./conditions ./character \
  -run 'TestEffectTooltipContentSuite|TestDisplayFor|TestDisplayCatalogExcludesShieldSpell|TestStatusViewRogueProjectsSneakAttackCondition|TestStatusViewProjectsCanonicalEffectDetails' \
  -count=1
GOWORK=off go test -mod=readonly -race ./...
golangci-lint run ./...
```

Before commit, run `git diff --check` and the repository's required hooks/gates
without bypassing them. Full module/lint checks apply at the provider readiness
boundary, not after each prose edit.

**Observed checkpoint evidence:** toolkit#1932 at `9b127988`; descriptor/status
regressions red then green; full root-dnd5e race suite passed; root lint reported
zero issues; normal commit hook passed with cache disabled and no bypass. Tooling:
Go 1.24.6 and repo-pinned golangci-lint v2.3.1. No dependency changes.

Repository-wide `make pre-commit` failed at the documented core coverage parsing
problem (#769): package output enters `bc`, and the target reports 78.4%. This is
not a successful repository-wide gate. Log: `/tmp/effect-info-content-repo-gate.log`;
root test/lint logs: `/tmp/effect-info-content-race.log` and
`/tmp/effect-info-content-lint.log`. The read-state assertion initially included
`ToData`'s generated timestamp; correcting that measurement left the intended
missing-description regressions red before the content fix.

Independent review at `9b127988` reported no Critical/Important findings and one
Minor: the raw-Strength-check path does not receive Rage advantage. A real-chain
probe confirmed one d20 for an unset skill versus two for Athletics. Copy and its
expected string were narrowed at `f94ed5a7`; the underlying governing-ability gap
is tracked separately in toolkit#1934. Focused descriptor/status/Rage race tests
and the normal uncached commit hook passed at that head. The full root gate was
not repeated for the two-line copy correction. The finding has an Addressed
thread disposition; reviewer closure at the new head remains pending.

C1 has green PR CI at `f94ed5a7`, but is held for #520's local-stack wave rather
than merged as a standalone checkpoint. Its outstanding integration acceptance,
not further copy polishing, is the draft reason. Current PR/merge status lives on
#1932 and #520's checklist; unmerged provider commits remain usable during
integration. This checkpoint provides no API or browser tooltip acceptance.

## Task C2: Detached observed context from encounter

**Delivers:** R7/R13's spatial/relationship input to shared assessments: the
observer's own position, current sighted member snapshots and pairwise distance/
relationship facts. It never decides whether a game effect applies.

**Owner:** `rpg-toolkit`, independent `rulebooks/dnd5e/encounter` module.
Worktree `.worktrees/effect-observed-context`, branch `feat/effect-observed-context`,
base `a088baa9` (same encounter source as inspected `3955eaf7`). No dependency on
C1's root-module changes; no root rulebook import or dependency bump.

**Prerequisites:** existing `Encounter.View`, `DecodeSightTestimony`, `placementOf`,
`Distance` and the current `BelievedStance` policy. The latter already specifies
that, absent deception, a creature shows the derived stance. C2 shares that owner
with an explicit observer for a pair of other sighted members; it does not infer
relationships from two ring colors or query a target's private viewpoint.

**Files:** create `encounter/observed_context.go` and
`encounter/observed_context_test.go`; modify `encounter/world.go` only to factor
`BelievedStance` through its shared observer-aware pair helper; update
`encounter/doc.go` with the read's bounded contract. All paths are inside the
encounter module. Existing `believedstance_test.go` remains regression coverage.

**Interfaces:**

```go
// Proposed here; implemented by C2 before a consumer may import it.
func (e *Encounter) ObservedContext(in *ViewInput) (*ObservedContextOutput, error)

type ObservedContextOutput struct {
    Observer MemberID
    Position spatial.Position // Observer's own canonical placement.
    Members []ObservedContextMember // Other members in CURRENT sight only.
    Pairs []ObservedContextPair // Ordered distinct pairs over observer + Members.
}
type ObservedContextMember struct {
    ID MemberID
    Position spatial.Position
    Down *bool // Nil = unobserved, not standing or incapacitation inferred.
    Equipment *HeldEquipment // Nil = unobserved, not empty hands.
}
type ObservedContextPair struct {
    From MemberID
    To MemberID
    DistanceCells float64
    Stance *Stance // Nil = no known relation; present neutral is a real fact.
}
```

**Behavior:**

- Validate through the existing member read: nil input is `ErrNilInput`; an absent
  or nonmember observer is `ErrNotMember`. Return no partial output on error.
- Own placement comes through `placementOf`. Other positions, standing and hands
  come only from that observer's current sight testimony. Exclude memories,
  unknown remembered locations and non-sight channels; never refill missing
  observation fields from the roster or capability providers. Invalid current sight testimony
  is an `ErrInvalidData` failure, not a guessed position.
- Sort member IDs; pairs are sorted by From then To, omit self pairs, and range
  only over the observer and those current sightings. Distances use `Distance`
  over the projected positions, not live target positions. Each ordered pair
  carries the relationship answered by encounter's shared believed-stance owner
  for the original observer; no faction means nil, not neutral.
- Preserve the existing stance policy and the old `BelievedStance(viewer,subject)`
  answers. Factor a private `believedStanceBetween(observer,from,to)` helper so
  future observer-specific stance belongs at one owner, not inside information
  projection. The known participant set is what authorizes pair enumeration.
- Return detached values. Read no live Sight/Equipment/Participation capability,
  publish nothing, resurvey nothing and change no persisted encounter data.
- The result enumerates **observations, not a complete world neighborhood**. It
  cannot prove absence of unseen participants, reveal target effects/senses, or
  turn observed standing into general reaction eligibility. Those fields remain
  unavailable to the later assessment unless another permitted provider supplies
  them. C2 is not a complete target-effect observation system.

**Tests:** new `TestObservedContextSuite` in the external encounter test package:

- `TestCurrentMembersAndPairs`: observer at `cellAt(0,0)`, ally at `cellAt(1,0)`,
  hostile target at `cellAt(2,0)`, world NPC at `cellAt(4,0)`. Assert sorted member
  IDs, exact pair count/order, ally-target distance 1, observer-target distance 2,
  known hostile target/ally relation and nil NPC relation (not neutral).
- `TestSnapshotFactsWinOverLiveState`: snapshot hands/standing, change their live
  providers to different answers or errors, read twice; output stays identical and
  provider call counts do not increase. A loaded known sight position differing
  from the true target cell supplies the projected position and pair distances.
- `TestMemoryAndOtherChannelsAreExcluded`: reload a holding with CurrentVia empty,
  an unknown remembered location, or another channel; it appears in neither
  Members nor Pairs. Hidden subject changes do not alter the observed output.
- `TestUnknownObservedFieldsStayUnknown`: a current known location with nil Down
  and Equipment remains nil in the result; no fallback consult occurs.
- `TestDetachedAndReadOnly`: mutate returned positions/pointers/rows and verify a
  fresh read is unchanged; compare full serialized encounter data before/after.
- `TestInputErrors`: nil, empty observer and stranger return the existing errors
  with nil output. Existing strict testimony-load tests still reject malformed
  current records; no reader-side repair is introduced.
- Existing `TestBelievedStanceSuite` stays green, pinning unchanged relation rules.

- [x] Add the named tests and demonstrate the missing API fails to compile.
- [x] Implement the bounded projection and shared stance helper.
- [x] Run focused and full module/race/lint gates; check no other module changes.
- [x] Publish a draft encounter-provider PR linked from the #520 body.

**Verification:** from the worktree's `rulebooks/dnd5e/encounter`:

```sh
GOWORK=off go test -mod=readonly . -run 'TestObservedContextSuite|TestBelievedStanceSuite|TestPassageSuite' -count=1
GOWORK=off go test -mod=readonly -race ./...
golangci-lint run ./...
```

Also run the normal commit hook without bypasses and report the repository-wide
gate separately. No API, browser or released-consumer adoption is proven by these
provider tests. Do not merge before the wave's applicable review/integration gates.

**Observed checkpoint evidence:** toolkit#1933 at `01e429e6`. The new API's absence
was demonstrated red; the observed-context, existing stance and passage tests
passed. Full encounter-module race tests passed, full lint reported zero issues,
and the normal commit hook passed with cache disabled. No dependency changes.
The suite also pins known empty hands, an empty observed universe and another
observer's conflicting testimony. Logs: `/tmp/effect-observed-context-focused.log`,
`/tmp/effect-observed-context-race.log`, `/tmp/effect-observed-context-lint.log`.

The repository-wide gate first hit a disk/cache failure. Available space was
rechecked; no cleanup was needed. Retrying with task-local caches reached the same
known core coverage/parser failure (#769), not a successful gate. Retry log:
`/tmp/effect-observed-context-repo-gate-retry.log`. Tooling remains Go 1.24.6 and
repo-pinned golangci-lint v2.3.1; no hook was bypassed.

The operator merged C2 on 2026-10-03 as `f06cacf2`. CI's auto-tag run
[37160884903](https://github.com/KirkDiggler/rpg-toolkit/actions/runs/37160884903)
succeeded; the annotated `rulebooks/dnd5e/encounter/v0.111.0` tag dereferences to
that merge commit. No independent review record is claimed here. Consumer pin
adoption, integration and browser evidence remain pending; tagging is not proof
of those boundaries. Later consumers must mirror these fields without an inner
encounter type crossing session's host surface.

## Remaining wave — connected implementation handoffs

**Build record, not verification evidence.** The contracts below translate R7–R15
into the remaining work. C1/C2 are implemented; these tasks are unchecked until
code and evidence exist. The running checklist and operator decisions live in
#520's body. Do not turn each task boundary into another approval ceremony.

### Contract decisions carried into this build

- Extend #1932 on `feat/effect-information-content` for the root-module work;
  do not split another root PR out of this same wave. Resolution and session
  each require their own module worktree/PR. All remain integration drafts.
- C2 is the factual projection for this wave. Its distance unit is **cells**, a
  `float64`, not feet. A rule compares against its existing owning constant.
  Missing reverse sight, target effects and third-party resource readiness are
  explicitly unknown. The read never enumerates hidden target providers or loads
  target sheets. A generic scope/need statement cannot vary with hidden state.
- This is not a deferral of target-aware inspection: known observed distances and
  pair stances refine the same rule assessment; unknown facts stay conditional.
  A new observation lifecycle is not required merely to manufacture certainty.
  An empty observed universe does not establish a negative existential predicate.
- Preserve current eligibility, including the separate defects #1929/#1934.
  Execution's existing handling of absent ambient facts remains at its adapter;
  a read does not inherit that fallback as knowledge. A known positive neighbor
  can establish Sneak Attack under its existing predicate without a complete
  neighborhood; lack of a neighbor cannot disprove it in an incomplete one.
- Shared source vocabulary moves below events by **type aliases**, not a new
  conversion pipeline. Resolved dice traces stay in events; unresolved rule
  values own no bus, RNG, sheet, repository or continuation.
- New assessment code cannot call Apply/Resolve to obtain an explanation.
  Execution adapters call the extracted decisions and retain their specific
  lifecycle points. An adapter may not keep a second eligibility predicate.
- Existing affordability and declaration IDs remain command authority. Information
  refs identify actual variants only; they neither price nor authorize actions.
- Develop against pushed provider SHAs and real pseudo-versions. Do not merge
  #1932 or a downstream Go module to unblock development. Protos are the existing
  generated-SDK exception, requiring the operator's merge before SDK adoption.

### Task C3: Shared rule values and detached operation contracts

**Delivers:** R7/R9/R12/R13; the concrete vocabulary consumed by C4/C5, without
changing eligibility or pretending the interpreter is already implemented.
**Owner:** root dnd5e; extend #1932. **Prerequisite:** existing action definitions
and C1 content. Root baseline `f94ed5a7`; the inventory's functional rule sources
remain the same as `3955eaf7` (C1 changed content/projection tests only).
**Files:** create `contributions/{doc.go,source.go,fact.go,decision.go}` and their
`*_test.go` files; modify `events/roll_trace.go`; create
`combat/assessment/{doc.go,context.go,damage.go,frame.go,binding.go,evaluate.go}`
and corresponding tests; context/damage contracts are the first implemented part.
All these paths are relative to `rulebooks/dnd5e/`.

**Shared vocabulary:** `contributions.Source` is the existing RollSource shape
(`Ref *core.Ref`, `Name`, `Label`, `SourceID string`); `CloneSource` owns ref copying.
`events.RollSource` becomes its alias and `CloneRollSource` delegates. Move the
unresolved `DiceContribution` and `RollKind` values below events the same way,
preserving existing event-facing names and callers. No persisted format changes.

`Fact[T]` carries a private known flag and private scalar value, with `Known(T)`,
`Unknown[T]()` and `Get() (T,bool)`. Zero is unknown; known false/zero/empty stays
known. Its scalar constraint permits bool, integer, float64 and string-backed
rule enums, not pointers/maps/slices. Facts do not smuggle live objects. Field
validators reject impossible values, including negative/nonfinite distances;
invalid is an error, not unknown.

`Facet` names AttackRoll, DamageNormal, DamageCritical, SpellDC, Healing.
`Applicability` names Applies, DoesNotApply, NeedsContext; its zero is invalid.
`Need` has `Kind`, `Subject`, `Other` (selection, distance, relationship,
directional sight, observed universe, target effects, reaction readiness,
normalized action facts).
`Decision` contains applicability, owner-authored reason and needs. Applies and
DoesNotApply reject needs; NeedsContext requires at least one valid need. Validation
is distinct from calculation availability and never converts an error to a row.

**Operation contracts in assessment:**

- `NormalizationFrame`: ActorID, cloned `actions.Definition`, scalar ability-score
  modifiers, explicit own equipment facts and ordered base/source evidence. No
  target result or rolled face. `AttackNormalizer.NormalizeAttack(*NormalizeAttackInput)
  (*NormalizeAttackOutput,error)` returns a decision and exact ability/pool
  replacements with displaced evidence; no generic callbacks.
- `AttackFrame`: actor ID, normalized action, `Target` selection/ID, immutable
  `Context` values. Context contains ordered participants and directed pairs:
  ID plus optional observed standing/equipment; pair From/To, distance-cells,
  hostility and directional sight facts; explicit universe completeness.
  `AttackRule.AssessAttack(*AssessAttackInput) (*AssessAttackOutput,error)` returns
  decision, sourced fixed/dice terms, keep sources, critical-threshold changes
  and typed later opportunities. No attack result/AC is supplied.
- `DamageFrame`: the AttackFrame, named normal/critical hypothetical or established
  branch, effective keep facts, exact normalized pool IDs/types/properties, and
  execution-established choices only. `DamageRule.AssessDamage(*AssessDamageInput)
  (*AssessDamageOutput,error)` returns decision plus sourced fixed/dice changes
  and per-pool face-reroll policy. It never rolls faces.
- `RuleBinding`: evaluation-local ID, cloned Source, optional condition address,
  deterministic order, per-operation/facet Coverage, and optional Normalizer,
  Attack and Damage capabilities. Capabilities contain detached scalar/slice
  snapshots, not live condition receivers. Binding output never crosses session.
  Existing save/check dice and healing declarations retain their narrow consumers;
  they use relocated common values, not artificial attack frames.
- `Evaluate(*EvaluateInput) (*EvaluateOutput,error)` receives an owned action,
  detached bindings, context and requested facets. It validates coverage, normalizes,
  assesses attack/keep, then damage branches. Outputs include ordered decisions,
  selected/suppressed evidence and calculations. Complete, PendingContext and
  Unavailable are distinct; only Complete has a settled expression. Unsupported
  coverage invalidates affected facets; unknown impact invalidates all requested
  facets. Actor-side pre-defense scope is explicit, not final target damage.

**Tests:** `TestSourceSuite` pins source/ref detachment and event alias identity;
`TestFactSuite` distinguishes unknown from known false/0/empty; `TestDecisionSuite`
rejects empty reasons, invalid states and contradictory needs.
`TestAssessmentSuite` uses fixture-owned capabilities: one changed return changes
both consumer paths; normalization precedes dependent eligibility; contradictory
coverage or nil-success output errors; unknown earlier stacking candidate leaves
selection pending; duplicate oldest-wins sources retain suppressed evidence;
unsupported damage removes only damage's expression; returned data cannot mutate
bindings/action. No concrete rule-ref switch belongs in this evaluator.

- [x] Leaf source/fact/decision contracts and bounded context/damage contracts,
  demonstrated red then green at `acf5fdda` / `1f73c0c8`.
- [ ] Complete the remaining operation/selection/fold regressions.
- [x] Move source/unresolved values with aliases; validate facts and decisions.
- [ ] Complete normalization/attack operation contracts and coverage validation.
- [ ] Implement ordered pure evaluation and facet availability.
- [ ] Run focused tests and normal commit hooks; publish on the existing draft.

**Verification:** root cwd; `go test -mod=readonly ./contributions ./events
./combat/assessment ./rolls ./saves ./healing`. Normal root-module gate applies at
its PR boundary. Completion requires those APIs compiling, not just this prose.

### Task C4: Rule-owned snapshots, execution adapters and action variants

**Delivers:** the shared answers, normalization and lifecycle custody behind
R2/R7–R14. **Owner:** root dnd5e, same #1932. **Prerequisite:** C3.
**Files:** the exact condition/feature/monstertrait files named in
[contribution-coverage.md](contribution-coverage.md), their existing tests, and
new `assessment.go`/`assessment_test.go` under those owning packages. Modify the
existing loader registrations (`conditions/loader.go`, `features/loader.go`,
`monstertraits/loader.go`) rather than adding a second ref registry. Modify
`combat/weaponattack/weaponattack.go` (including its existing Override type),
`combat/actions/attack.go`,
`character/{attack_definition.go,off_hand_attack.go,martial_arts_attack.go,casting.go}`;
create `character/{action_information.go,action_information_test.go}`. Content
changes stay in the existing display/content owners.

**Binding contract:** `AssessmentSnapshot(*assessment.BindInput)
(*assessment.BindOutput,error)` is a condition/feature capability. BindInput names
instance ID, owner ID and persisted order. The output deep-copies source, captured
usage and equipment/resource facts needed by the owning rule. Existing loader
entries declare explicit coverage; every loadable key must be classified Supported,
NotRelevant or Unsupported per affected operation/facet. A missing capability is
not NotRelevant. A census regression fails if a new loader is added unclassified.
Monster reads must not fabricate a roller to decode inert state.

**Migration sequence within this task:**

| Source-owned group | Extraction and paired proof |
|---|---|
| Bless/Bane | Reuse unresolved dice and declared stacking; two casters yield one selected contribution and one suppressed source. Saving-throw consumers still use that same decision. |
| Rage/Sneak Attack | Evaluate effective ability, type/usage and bounded facts in the owning rule. Known failures win before irrelevant unknowns. Positive target↔neighbor hostility within the existing adjacency threshold can establish Sneak Attack; incomplete absence cannot. Preserve #1929 and #1934, Rage sustain, and Sneak's current use transition. |
| Shillelagh/Martial Arts/TWF | One normalization path for attack and damage, preserving existing winner/eligibility rules. No double override or discarded base-damage roll. Negative off-hand ability remains base policy; TWF restores the positive term once. Freeze effective ability/pools for continuation. |
| Archery/Dueling/Improved Critical/Brutal Critical | Sourced terms/policies with owning predicates. Compare ranged/melee, one/two hands and normal/critical branches against existing execution tests. |
| GWF/Divine Favor | Exact weapon-pool reroll policy, preserving faces and one reroll per eligible face; Divine Favor's existing first-contributor selection and critical dice. Reads roll neither selected nor suppressed candidates. |
| Reckless/Prone/Hidden/Help/True Strike/Mockery | Typed keep decisions and pending needs. Preserve each owner's existing consumption/removal boundary; no generic consume callback. Own effects remain readable when ineligible. |
| Inspired | Advance after-roll opportunity, separate from automatic terms and executable offer IDs; accepted offer consumes/rolls only at the existing continuation boundary. |
| Protection/Dodging/Faerie Fire/Guiding Bolt/Pack Tactics and reactive features | Extract the same predicate for authoritative execution and any permitted supplied facts. Never bind a hidden target's provider just to learn whether information is pending. Protection stays automatic; actual offers stay optional; Guiding Bolt consumes after the real roll. |
| Other inventory entries | Explicit facet classification, retaining existing defensive, movement, lifetime and legality paths. NotRelevant is an owner statement about this pre-defense question, not a claim that the content has no rules. Unsupported remains visible/unavailable where it affects the requested question. |
| Base weapon/casting/healing | Emit ability/proficiency/base-pool/override evidence where derived. Reuse existing Disciple of Life and healing declarations. No reverse engineering aggregate modifiers and no Bless added to caster DC. |

**Action identity:** `character.ActionInformationRef` carries Kind (Weapon/Spell),
canonical Ref, Variant (MainHand/OffHand/MartialArts/Spell), Hand slot, ItemID and
TwoHanded. `(*Character).ActionInformation(*ActionInformationInput)
(*ActionInformationOutput,error)` returns ordered entries containing Ref, canonical
name/description, current definition (optional for known non-executable content),
canonical options and detached bindings. Input optionally selects an exact Ref and
spell Option. Enumerate existing actual equipment variants, preserving static
bonus-attack precedence, independent of current turn/budget. No hypothetical grips,
re-equipped inventory or assumed ownership. A stale/mismatched ref errors; known
spell content without a cast definition retains content with unavailable mechanics.
Shared assembly supplies the matching information ref for command declarations.

**Tests:** `TestConditionAssessmentSuite`, `TestAssessmentCoverageSuite`,
`TestActionInformationSuite`; extend existing rule suites rather than delete their
execution assertions. Pin once-used Sneak before target need; DEX-only baseline
versus #1929; critical doubling; Rage on STR melee versus DEX/ranged; Bless+Bane
operators/stacking; Inspiration not in committed terms; normalization paired cases;
read no RNG/events/state changes; condition removal cannot erase detached source;
all 56 inventory registrations and inherent producers classified. Normally finalize
rogue/barbarian/cleric/bard, serialize/load, then enumerate actions. Spent/off-turn/
paused reads still enumerate; same weapon ref in distinct slots does not collide.

- [x] First owner: Rage outgoing damage has a detached snapshot and the real
  damage chain uses the same decision/amount (`1f73c0c8`). Paired tests cover
  STR/DEX, melee/ranged, changed/zero bonuses and no sustain/consumption on reads.
  Existing Rage lifecycle/resistance tests and full root race suite pass;
  normal uncached commit hook passes with zero lint issues. No full-wave claim.
- [ ] Complete other detached snapshots and loader-wide coverage contracts/tests.
- [ ] Extract owner decisions group-by-group with paired execution/read tests.
- [ ] Emit shared assembly evidence and actual action identities.
- [ ] Add remaining canonical descriptions used by supported effect rows.
- [ ] Run focused owner suites and publish meaningful updates to #1932.

**Verification:** root cwd; targeted owner suites while editing, then owning
root-module tests/lint at the stated PR boundary. Completion evidence must enumerate
migrated versus explicitly unsupported facets; a green census alone is not migration.

### Task C5: Resolution information read and execution custody

**Delivers:** read assembly plus shared execution under R7–R9/R12–R14.
**Owner:** resolution module, new `.worktrees/effect-information-resolution` from
origin/main. **Prerequisite:** pushed C3/C4 root commit and encounter v0.111.0.
**Files:** create `information.go`, `information_test.go`; modify `visibility.go`,
`strike.go`, `strike_pose.go`, their tests and `doc.go`; update this module's
`go.mod`/`go.sum` only.

`InspectActions(*InspectActionsInput) (*InspectActionsOutput,error)` takes actor
`character.Data`, optional exact root ActionInformationRef/Option/Target, and C2's
`ObservedContextOutput`. It takes **no World, target sheets, repositories, bus or
roller**. It purely loads the actor, obtains C4 variants/bindings, maps permitted
pair facts, calls C3 and projects information values. Target must be self or a
current sighted member; unknown and unobserved IDs share one not-found result.
Self-selection uses own permitted facts, not an invented self pair. No target is
an actor-only query. Observation completeness is false; reverse sight and target
provider state remain unknown unless explicitly supplied by a permitted producer.

Output Actions carry Ref/Name/Description/Context/Options/Effects/AssessmentIssues.
Effect fields follow the read contract: local instance, source/ref, description,
availability, optional decision/needs, participation, selection and benefit detail.
Issues name affected facets and provider-authored scope/reason; unsupported is not
an action refusal. Canonical content is not replaced by contextual reason text.
For one source spanning phases, the generic row projection is conservative:
unsupported assessment precedes pending context; otherwise any participating phase
makes the row applicable, and all known negatives make it ineligible. Preserve
owner-authored phase reasons/known benefits and per-facet issues; a row summary
never erases a supported calculation or invents another eligibility predicate.

Extract visibility's pure two-direction rule under its owning rule layer and use
it from this read and execution. At execution boundaries, existing adapters call
C4 decisions with authoritative facts. Normalize before dependent chains and store
effective ability/pool/source evidence in the existing frozen strike data. Resume
uses it; no new independent continuation store. Existing paid/readied/roll/pause
boundaries remain. Do not invoke an entire live interaction from the new read.

**Tests:** `TestInspectActionsSuite`: identical allowed context + changed hidden
world/sheets cannot change results (the entry cannot even accept those sheets);
current sightings versus memories; distance/stance unknown versus known neutral;
unknown target error equality; repeated/detached reads; action stale identity;
actor-only/selected target refinements. Extend frozen Bane/Bless/Inspiration tests:
no rerolled d20 or prior bonus on resume, declined benefit unspent, accepted benefit
rolls exactly its die, tampered frozen input refused. Paired normalization retains
same attack/damage ability and pools across suspension.

- [ ] Pin pushed root and published encounter versions with `go get`.
- [ ] Add pure read regression fixtures and source noninterference assertions.
- [ ] Implement read and shared execution/normalization wiring.
- [ ] Extend frozen continuation tests and publish a draft.

**Verification:** module cwd; `go test -mod=readonly . -run
'Test(InspectActionsSuite|BaneCalculationIsFrozenAcrossInspirationAnswers|ATamperedFrozenBlobIsRefused|SightAttackModifiers)'`;
normal module hook. Root sibling edits on disk do not count as this module's pin.

### Task C6: Session host verb and public mirrors

**Delivers:** R7/R10/R13/R15; authorized IDs in, detached answers out.
**Owner:** session module, `.worktrees/effect-information-session`.
**Prerequisites:** pushed root/resolution commits plus encounter v0.111.0.
**Files:** create `inspect_actions.go`, `inspect_actions_test.go`; modify
`types.go`, `offers.go`, `casts.go`, `doc.go`, `boundary_test.go` only if its
explicit public method census requires it, and module pins.

`(*Manager).InspectActions(ctx,*InspectActionsInput) (*InspectActionsOutput,error)`
takes Session/Member strings, optional session-owned ActionInformationRef,
Target and Option. Validate their combinations; load membership, actor data and
world observation context, then call C5. No target character/monster repository
reads, compileResolutionCast, affordability test, writeScope, commit, record, RNG
or event publication. Project every result into session-owned mirror types; no
inner type crosses S2. Map malformed/stale refs and unpermitted targets to the
existing typed read error categories without exposing hidden membership.

Add optional `Declaration.Information` from the same C4 identity used when its
actual attack/cast definition is assembled. It is not part of the executable
selector hash or permission. Early blockers may omit it; the independent listing
still provides inspectable actions. No effect evaluation in offer compilation.

**Tests:** `TestInspectActionsSuite`: fake repositories count exactly actor reads
and zero saves/events/dice; actor auth/membership failures; off-turn/spent/frozen
states do not suppress the list; stale equipment identity; optional presence;
returned source/needs slices detach. Run existing no-bus/no-check/no-inner-type
architecture tests and offer-selector regressions. A real C5 projection fixture
pins active/gray/conditional/opportunity rows, not just a fake SDK result.

- [ ] Pin actual pushed providers; add host mirror/input-output types.
- [ ] Add read/no-write tests, implement orchestration and exact declaration join.
- [ ] Run focused and seam tests; publish a draft.

**Verification:** module cwd; `go test -mod=readonly . -run
'Test(InspectActionsSuite|NoBusLivesInThisModule|SessionConstructsNoCheck|NoInnerTypeCrossesTheBoundary)'`;
normal module hook.

### Task C7: Wire contract and thin API mapping

**Delivers:** agreed read/row semantics without a client-side evaluator.
**Owners:** protos from main; API from origin/dev, each own
`.worktrees/effect-information` worktree. **Prerequisites:** C3–C6 contract fields
above for authoring; published proto SDK + pushed session commit for API compilation.
**Proto files:** existing `dnd5e/api/session/v1alpha1/{types.proto,events.proto,service.proto}`.
Identity and Declaration tag 21 live in types.proto; sourced information output
lives beside RollSource in events.proto; RPC/messages live in service.proto.
The attempted new-file layout hit Buf package-option consistency against legacy
options. Existing homes avoid copied options, policy changes, duplicate source
vocabulary and import cycles. Proto baseline: `11372b0`.
**API files:** new `internal/handlers/dnd5e/session/v1alpha1/inspect_actions.go` and
`inspect_actions_test.go`; extend `convert.go`'s declaration mapper and the
`handler.go`'s Manager interface and regenerated `mock/mock_manager.go`; update Go pins.

Wire shape: `InspectActionsRequest{session=1,member=2,action=3,target=4,option=5}`;
`InspectActionsResponse{actions=1}`; RPC `SessionService.InspectActions`.
ActionInformationRef: `ref=1`, `variant=2` enum (main hand/off hand/martial arts/spell),
`item_id=3`, `two_handed=4`. Variant fixes the applicable hand; no redundant hand
field or opaque command selector. Empty item ID is allowed only for the provider's
unarmed/spell forms. Message presence distinguishes list from selected-action read;
nonempty target/option requires action, and option requires a spell variant.
ActionInformation: `action=1,name=2,description=3,target=4,option=5,effects=6,
assessment_issues=7,options=8` (existing CastOption shape).
EffectInformation: `instance=1,source=2,description=3,assessed=4,unavailable=5,
participation=6,selection=7,benefit_detail=8`. Source reuses existing RollSource
(and session.RollSource / rollSourceToProto), not another identity vocabulary.
The assessment oneof contains assessed applicability/reason/needs or unavailable
facets/reason. No raw blobs. Applicability UNSPECIFIED is invalid, not false.
Participation is an enum with UNSPECIFIED meaning absent. Optional selection is
`EffectSelection{state=1,reason=2}`; a present message requires selected/suppressed
state and its provider-authored reason. Absence means no selection decision.
No terms/expressions are required in the public wire for a tooltip. SDK retains
internal calculation evidence; UI receives provider-authored benefit detail.
Declaration tag 21 is `information`, optional and non-authorizing.

API calls `callerActingAs` exactly as Afford, converts input, calls the session
manager and uses sdkerr for failures. Map presence and enums explicitly; unknown
producer enum is an error, not a negative. No handler-owned rule names, target
sheet lookup or fallback description. Leave other RPCs unchanged.

**Tests:** proto make test/build/CI-generated SDK validation (no bespoke protoc
roundtrip tests). API `TestInspectActionsSuite`: auth failure never calls manager;
exact slot/item/grip identity roundtrip, all row states/source identities, empty
known content versus unavailable mechanics, absent selection, typed SDK errors.
Use a real session-output fixture for converter coverage in addition to the mock
manager; no new orchestration layer.

- [x] Author proto contract: protos#374 at `72d18c9`; `make format`/`make test`
  and PR lint/generated-SDK checks pass. No generated bindings were hand-committed.
- [x] Publish proto PR for operator inspection. Independent contract review at
  `72d18c9` found no findings ([record](https://github.com/KirkDiggler/rpg-api-protos/pull/374#pullrequestreview-5403966081));
  CI generated-SDK checks also pass. Ready for operator merge under the proto
  exception; actual CI publication is still required before API/web SDK adoption.
- [ ] Adopt actual generated SDK and pushed session provider in API.
- [ ] Implement mapping/tests; run `make ci-check` and publish API draft.

**Completion evidence:** published proto tag and exact API/session pins, mapper
assertions and API gate output. A proto PR does not prove an implemented RPC.

### Task C8: Fresh accessible effect inspection in web

**Delivers:** R10/R14/R15, effects before acting with no command-flow redesign.
**Owner:** web from origin/dev; `.worktrees/effect-information`.
**Files:** new `src/api/useSessionInspectActions.ts` and `.test.tsx`; new
`src/components/session/combat-experience/{EffectInformation.tsx,EffectInformation.test.tsx}`;
modify `SessionEncounterView.tsx`, `SessionCanvas.tsx` only if existing hover
forwarding needs adaptation, `combat-experience/{ActionDock.tsx,TargetSurface.tsx,
types.ts,CombatExperience.tsx,CombatExperience.module.css}` and their tests;
add focused `useSessionCombatExperience.test.tsx` assertions, not command rules.
Update package.json and package-lock.json to the actual proto SDK together.

The hook asks the list independently of Afford, then one action/target/option.
Key includes session/member/full information ref/target/option and a monotonic
invalidation generation. Late A response cannot replace B; an older generation of
B cannot replace a refreshed B. Delivered events invalidate existing answers;
retain tooltip content marked stale while refetching. No persistent cache or new
stream. Inspection failures leave command availability untouched.

Join declarations to list rows only by provider Information equality. Unaffordable
or off-turn actions still have a read-only information affordance from the list;
never mint a command selector. Reuse SessionCanvas.onHoverEntity, target-list focus
and a separate touch inspect button. Hover-out does not erase the latched target
while reading its tooltip; explicit dismiss/action/member change clears it.
Gray/ineligible effects are focusable/readable, not disabled controls. Display
canonical description and contextual reason separately; optional participation
has its own label, not an already-added bonus. No rule-name/weapon-property branch.
Frozen rollWindow continues to display frozen values, not this current-state read.

**Tests:** stale response A/B and same-key generations; off-turn list; absent
information join; same ref in two equipment slots; five row states; focusable gray
tooltip; inspection sends zero commands; normal attack/single-target cast click
still sends exactly one existing command immediately even while a read is pending.
Existing multi-target confirmation and refusal handling remain unchanged.

- [ ] Add the hook and provider-shaped fixtures/tests.
- [ ] Implement effect rows and pointer/focus/touch wiring.
- [ ] Run focused tests while editing, then one `npm run ci-check` at PR boundary.
- [ ] Publish web draft with unrun native acceptance clearly pending.

### Task C9: Local stack walkthrough, checklist review, then finalization

**Delivers:** the joined acceptance, not another component-only claim.
**Owner:** cross-team; #520's checklist is the receipt. **Prerequisites:** C3–C8
pushed, API on real pseudo-version pins, published proto SDK. Use the workspace
root's `scripts/dev-env.sh` and its existing local configuration contract; do not
invent a dependency manifest or replace/source-copy development loop.

Prepare isolated `local/effect-information`; record exact API/web commits and all
Go provider pins. Create rogue/barbarian/cleric/bard through normal draft/finalize
paths and verify private-sheet reads. Use their normal acquired actions to grant
Rage/Bless/Inspiration. Focused seeded provider fixtures supplement, never replace,
this acquisition/cast proof. Capture request/response and native rendered evidence.

Walk assertions: actor-only conditional Sneak Attack → permitted positive target
context when the current rule establishes it; used-this-turn gray with same tooltip;
Rage STR melee versus ineligible action; Bless active; Inspiration optional;
hover/focus/touch zero commands; click still immediate; effects expire/change and
refresh; target switch ignores old answer; reload has current source/usage. Identify
unknown facts as unknown, not a concealed refusal. Record unsupported facets rather
than rendering an apparently complete result. No hit probability/HP prediction panel.

- [ ] Bring up the isolated stack and complete the real-path first walkthrough.
- [ ] **Pause with the operator:** review the checklist, decisions and remaining gaps.
- [ ] Apply agreed polish and required independent review/finding closure.
- [ ] Operator merges root → resolution → session → API/web after actual tag adoption
  and affected checks; encounter v0.111.0 is already published. No automatic merge.
- [ ] Record integrated/released evidence and close #520 only when its scope is done.

**Verification:** from workspace root, `scripts/dev-env.sh up local/effect-information`
then `scripts/dev-env.sh status local/effect-information`; the existing
`envs/local/effect-information.env` schema selects RPG_API_PATH and RPG_DND5E_WEB_PATH
and distinct available host ports (check `ss -ltn` before allocation), with
RPG_REDIS_PERSIST=0 and no toolkit override. Capture native screenshots through
`node tools/browser/screenshot.mjs <reported-url> <local-output.png>`. Targeted
browser/real-API journeys run while building; final repository and release-pin
checks follow the walk/review point. Record allocated ports/fixture identities in
#520's body; they are runtime facts, not new manifest policy.

### Ownership/dependency audit

The following inventory retains the original workstream audit; its dependencies
are now named producing tasks above, not instructions to invent a contract later.

| Owner / source | Required deliverable | Readiness dependency |
|---|---|---|
| Root dnd5e: `events/roll_trace.go`, `conditions`, `features`, `monstertraits`, weapon/cast assembly | Shared values, detached assessments, per-facet coverage, normalization and lifecycle-preserving adapters for the inventory | C3/C4 produce these contracts; C1 supplies only initial descriptions |
| Encounter: C2 above; remaining observed projections | C2 supplies current observed positions/distances and existing public relationship policy; target-carried effects/senses stay separate | C2 merged/tagged via #1933 (encounter v0.111.0); additional target-effect/sense knowledge still needs its own permitted source, never a hidden-truth fallback |
| Resolution: `strike.go`, `strike_pose.go`, `visibility.go`, new information read | Assemble/fold rule-owned assessments and return effect information without rolling/mutating; migrate execution consumers to the same decisions | C5 consumes C3/C4 and published C2; preserve per-boundary frozen state |
| Session: `offers.go`, `casts.go`, `read.go`, `types.go`, new inspection verb | Enumerate actual action variants independent of affordability; carry permitted context; project detached information refs/effects | C6 consumes C4 variants and C5's read |
| Protos/API: session service/types and session handler/converters | Inspection request/response, informational action ref on declarations, authorization and one mapping point | C7 mirrors C6; operator-published SDK required before consumer compilation |
| Web: `SessionEncounterView`, `SessionCanvas`, action dock/target surface, new read hook | Separate inspection state and effect tooltips, newest-generation responses, explicit touch inspection, unchanged click commands | C8 consumes C7 SDK and RPC; R15 interaction is settled |
| Joined stack | Normal acquisition, effect changes, target switching, reload and no-side-effect/native-tooltip proof | C9 consumes the pushed C3–C8 wave and existing dev-env workflow |

No persistence/event expansion is assumed yet. If a newly required observation
must survive loss of sight/reload, that lifecycle belongs in the existing
observer testimony owner, with an explicit task and tests—not a hidden cache in
the information read. Existing multiplayer events can invalidate current reads;
new event types need an actual uncovered producer/consumer requirement.

## Visible checks

### Requirement coverage

| In-scope requirement / acceptance | Task or status | Concrete proof |
|---|---|---|
| Canonical descriptions for Sneak Attack/Raging/Blessed | C1 implemented in #1932; current status on the issue checklist | Nonempty descriptor regression, unchanged names/refs, status detail equals canonical content |
| Rule-owned applicability/reason and shared execution | C3/C4: contracts and paired owner migrations | Inventory's paired consumer/execution and no-duplicate-predicate assertions; not covered by C1 |
| Explicit normalization and frozen/consumption custody | C3/C4/C5: normalization, adapters and frozen continuation | Paired-rule and RNG-count/freeze tests in inventory; not covered by C1 |
| Permitted selected-target context | C2 + C5: observed pair facts; unsupported target-effect/reverse-sight facts explicitly Unknown | Snapshot, current-only, no-live-provider, unknown and pair-scope tests; not a claim of complete target knowledge |
| Independent read, freshness and command legality | C6/C7/C8: independent read and generation-safe client | Off-turn/spent/frozen reads, exact action identity and stale response cases |
| Active/gray/conditional/opportunity UI and accessible tooltips | C7/C8: typed effect rows and inspection controls | Pointer/keyboard/touch acceptance and zero command calls on inspection |
| End-to-end normal character acquisition/reload/effect changes | C9: first real-stack walk, then checklist/review phase | Real API/native browser proof, not seeded-provider tests alone |

### Provider/consumer seams

| Provider | Consumer | Produced vs consumed contract | Availability/dependency | Proof |
|---|---|---|---|---|
| C1 / conditions | Existing character status projection | `Display.Detail` → `ConditionView.Detail`, same canonical ref | Existing compiled interface; no new dependency | New status-detail equality and no-mutation test |
| Root variant/assessment work | Resolution and session | Typed action identity, frames, decisions and coverage | Produced by C3/C4; pending implementation | C3/C4 named contracts → C5; joined tests required before claiming the seam works |
| C2 / encounter | Resolution information read | `ObservedContextOutput` members/pairs; optional observation facts remain optional | Published as encounter v0.111.0 via #1933; consumer handoff/adoption still pending | Provider tests pass; C5 supplies joined information noninterference proof |
| Session → protos/API → web | Effect indicators and tooltips | `InspectActions` / `ActionInformation` / `EffectInformation` semantics | C6/C7/C8 define matching fields; pushed Go pins plus published proto SDK | C7 mapper assertions and C8/C9 interaction/native proof |

**Check findings:** the previous source inventory was not an implementation plan;
this document does not relabel it as one. C1 consumes only inspected existing APIs;
C2 produces the bounded observed-context API from inspected owners without changing
stance/visibility policy. C3–C9 now name the remaining producers and consumers. Unknown is a valid produced
fact under R13, not a promise to build a new observation system or a hidden-truth
fallback. Checked corrections: C2 distances are float64 cells; weapon Override
lives in weaponattack.go; wire identity stays in types while sourced output stays
beside RollSource, avoiding cycles and incompatible new-file options; real provider
pins—not sibling directories—drive
consumer builds. No user-visible scope was deferred to make this table complete.
Implementation and joined proof remain unchecked above.
