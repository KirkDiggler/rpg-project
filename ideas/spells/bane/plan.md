# Bane Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Ship the approved Bane slice through creation, one paid three-target cast, automatic sourced d4 subtraction, persistence, API, and minimal web acquisition/cast presentation.

**Architecture:** The D&D rulebook owns Bane content, applicability, resources, and described contributions; resolution owns preflight, payment sequencing, dice, and freeze/resume; encounter/session carry neutral records and host seams. Protos define one clean alpha contract, API only projects it, and the web only submits provider-authorized choices/targets and renders provider-authored facts.

**Tech Stack:** Go 1.24.1+, rpg-toolkit multi-module Go packages, Protocol Buffers/Buf, ConnectRPC, Redis-backed rpg-api integration tests, React/TypeScript/Vitest.

**Spec:** `ideas/spells/bane/design.md`

## Global constraints

- Implement exactly the 16 acceptance scenes in the design; every test named below is future acceptance and is not claimed to pass today.
- Bane is the one supported level-1 Bard choice through existing `BardSpells1`, `SpellbookRequirement`, and `CHOICE_CATEGORY_SPELLS`; keep the factual four-known-spells class table unchanged.
- Recoverable resources are the sole mutable slot authority. Delete speculative `SpellSlots`; add no migration, legacy reader, dual writer, converter, or data wipe.
- One cast is one ordered target list, one pure whole-list preflight, one atomic price, and one concentration owner. The casting turn does not tick; ten subsequent caster turn ends do.
- The recipient condition owner chooses the oldest active equal-potency Bane from persisted order. Conditions describe, resolution rolls, and session does neither.
- Keep strict `RollCalculation` source-ref validation. Every base d20, fixed modifier, reroll, Inspiration, and Bane component has a valid canonical rule/content ref; Bane additionally has its selected caster entity ID.
- Keep contributor entity identity in exactly one calculation location: `RollSource.SourceID`. `DiceContribution` embeds that source; projection copies it without a sibling contributor-ID field.
- Use source-qualified `{MemberID, ConditionRef, SourceID}` addresses. Do not add application IDs or a global registry.
- Preserve True Strike timing/effect consumption and Rage sustain. Death saves remain a separate evaluation path with their existing natural-d20 policy.
- UI dice-set lookup, customization, animation, presets/assets, and a new Story/dice subsystem are outside scope. The web only renders the existing generic calculation text and roster-resolved source name.
- One branch per repository for the wave. Protos open a real PR early so CI produces generated clients. Toolkit provider commits are pushed and consumed by actual SHA-derived pseudo-versions for local proof; eventual consumers adopt only real CI tags after providers merge.
- Before any repository command, the parent creates and verifies issue worktrees from fetched canonical refs, then binds absolute `TOOLKIT_WORKTREE`, `API_WORKTREE`, `PROTOS_WORKTREE`, and `WEB_WORKTREE` values. Commands fail when a binding is unset and never default to `/home/kirk/game-dev/<repo>` primary checkouts. `GAME_DEV_WORKSPACE` is bound separately and is used only for the named local runtime.
- Preserve primary checkouts, other worktrees, and other runtime lanes. No committed `replace`, `go.work`, generated-source hand edit, CI redesign, force-push, production/shared-stack deployment, environment reset, or destructive cleanup. Only the unclaimed named local proof stack in Task 12 may be started.

---

## File-responsibility map

Paths below are relative to their named repository.

| Repository / files | Responsibility |
|---|---|
| `rpg-toolkit/rulebooks/dnd5e/{README.md,character/CLAUDE.md}`, `docs/architecture/components/rulebook-dnd5e.md`, `docs/how-to/add-a-mechanic.md` | Narrowly refresh stale ownership/API guidance before implementation; do not rewrite ADR history. |
| `rpg-toolkit/rulebooks/dnd5e/resources/keys.go`, `character/{data.go,character.go,draft.go,load.go,ledger_test.go,known_spells_test.go,long_rest_test.go,character_test.go}`, `character/choices/{requirements.go,requirements_detail_test.go,class_comprehensive_test.go}` | Canonical level-1 recoverable resource, removal of duplicate `SpellSlots`, and Bane-only Bard acquisition. Existing `classes/data.go` and `choice_ids.go` are read-only inputs. |
| `rpg-toolkit/rulebooks/dnd5e/events/{events.go,damage_taken.go,roll_trace.go}`, `conditions/{baned.go,loader.go,concentrating.go,display.go}`, `refs/conditions.go`, `combat/actions/cast.go`, `character/{sheet_keeper.go,character.go}`, `monster/{monster.go,load.go}` | Task 4 vocabulary and behavior: canonical source metadata and contribution description, `refs.Conditions.Baned`, loader plus `conditions.DISPLAY` registration, qualified matching, recipient-owned oldest selection, and per-profile owner timing. |
| `rpg-toolkit/rulebooks/dnd5e/events/roll_trace.go`, `saves/{saves.go,death_saves.go}`, `refs/actions.go`, `spells/{cast.go,cast_test.go}`, `combat/actions/{cast.go,cast_test.go}` | Task 5 calculation/evaluation plus the complete Bane cast definition, action+pool cost, condition effect, and generic min/max target profile. Existing `spells/data.go` and `refs/spells.go` remain the factual Bane identity/catalog. |
| `rpg-toolkit/rulebooks/dnd5e/saves/{saves.go,death_saves.go}`, `refs/actions.go` | Generic ordinary-save calculation, a canonical Death Save action ref, and the separate death-save contribution/calculation path. |
| `rpg-toolkit/rulebooks/dnd5e/encounter/{cast.go,outcome.go,roll_trace.go,activation.go}` | Bus-free neutral ordered cast/result and calculation records; no Bane selection or arithmetic. |
| `rpg-toolkit/rulebooks/dnd5e/resolution/{action.go,cast.go,contest.go,save.go,death_save.go,strike.go,concentration.go}` | Whole-list preflight/fan-out, one payment, condition lookup, roll evaluation, natural-d20 settlement, and frozen calculation custody. |
| `rpg-toolkit/rulebooks/dnd5e/session/{casts.go,cast.go,castoutcome.go,death_save.go,afford.go,offers.go,attack.go,react_post_roll.go,types.go,events.go}` | Offer/candidate/cost compilation, action/resource recheck, participant repository access, and opaque result projection. |
| `rpg-api-protos/dnd5e/api/v1alpha1/{character.proto,choices.proto}`, `dnd5e/api/session/v1alpha1/{service.proto,types.proto,events.proto}` | Clean alpha target-list, cost/cap, resource retirement, qualified condition, and generic calculation wire. Generated artifacts remain CI-owned. |
| `rpg-api/internal/handlers/dnd5e/v1alpha1/character/{handler.go,converters.go,spell_refs.go}` | Existing spell choice/ref projection only. |
| `rpg-api/internal/handlers/dnd5e/session/v1alpha1/{cast.go,convert.go}` | Field-for-field target/cost/calculation/source projection only. |
| `rpg-api/internal/integration/{character/creation_test.go,session/cast_acceptance_test.go,session/bard_inspiration_acceptance_test.go}` | Real Character/Session handler + Redis acceptance. |
| `rpg-dnd5e-web/src/api/useSessionCast.ts`, `src/{types/choices.ts,utils/choiceConverter.ts,components/ChoiceRenderer.tsx}`, `src/character/creation/{ClassSelectionModal.tsx,InteractiveCharacterSheet.tsx}`, and `src/components/session/combat-experience/{selection.ts,TargetSurface.tsx,types.ts,useSessionCombatExperience.ts,ActionDock.tsx,story.ts,rollTrace.ts}` | Minimal spell creation, provider-capped multi-select, one Cast request, and generic text presentation. |
| `rpg-dnd5e-web/src/{components/ChoiceRenderer.test.tsx,character/creation/cantripChoice.test.tsx,character/creation/ClassSelectionModal.test.tsx,components/session/combat-experience/castFlow.test.tsx,components/session/combat-experience/castStory.test.ts}` | Focused consumer contracts; not substitutes for real-path acceptance. |

## Cross-task contracts

These are concrete implementation proposals under the approved behavior. They are the single spelling later tasks consume; a task may adjust private helpers but must preserve these public facts.

```go
// rulebooks/dnd5e/events
type RollSource struct {
    Ref      *core.Ref // required and valid for every component
    Name     string
    Label    string
    SourceID string    // optional entity provenance; required for Bane
}

type DiceContribution struct {
    Source   RollSource // the sole home of contributor SourceID
    Dice     string     // unsigned homogeneous notation, e.g. "1d4"
    Subtract bool
}

type RollComponent struct {
    Source       RollSource
    Dice         *DiceTrace
    Modifier     *int
    SubtractDice bool // applies only to Dice.Subtotal
}

type RollKind string
const (
    RollKindAttack RollKind = "attack"
    RollKindSavingThrow RollKind = "saving_throw"
)

type DescribeRollContributionsInput struct { Kind RollKind }
type DescribeRollContributionsOutput struct { Contributions []DiceContribution }
type RollConditionOwner interface {
    DescribeRollContributions(*DescribeRollContributionsInput) (*DescribeRollContributionsOutput, error)
}
```

`RollSource.Ref` remains mandatory under existing `ValidateRollCalculation`. Callers pass the canonical ref that owns each fact: the compiled attack ref for its d20/attack modifier; the validated `SaveCause.EffectRef` for an ordinary/concentration save d20 and the ability ref for its fixed modifier; new `refs.Actions.DeathSave()` for the separate death-save d20; the Inspiration condition/feature ref for Inspiration; and the Bane spell ref for Bane. `SavingThrowInput` therefore gains required `D20Source` and `ModifierSource` values, while `DeathSaveInput` gains required `D20Source`; neither evaluator manufactures anonymous sources. Validation is not weakened and synthetic “uncatalogued” refs are not invented.

```go
// generic condition addressing
type ConditionAddress struct {
    MemberID     string `json:"member_id"`
    ConditionRef string `json:"condition_ref"`
    SourceID     string `json:"source_id"`
}

// generic cast result
type CastProfile struct {
    RangeFeet    int
    Target       CastTargetRule // self or creature; no cardinality in the enum
    MinTargets   int
    MaxTargets   int
    Save         *saves.SaveGate
    Damage       []damage.Damage
    Effects      []CastEffect
    Concentration *CastConcentration
}

type CastTargetOutcome struct {
    TargetID string
    Save     *ContestOutcome
    Applied  []ImposedEffect
}
type CastOutcome struct {
    Spell     core.Ref
    CasterID  string
    Targets   []CastTargetOutcome
    FollowUps []FollowUpOutcome
}
```

`ConditionAddress` replaces unqualified child/removal tuples through rulebook, encounter, and session records. Empty `SourceID` is exact legacy-unqualified identity, never a wildcard; new Bane owners/children reject it. `ImposedEffect` carries this address plus `SourceRef` provenance.

```go
// session-neutral offer surface
type CostComponent struct {
    Currency Currency
    Needed   int
    Label    string // provider-authored; pool keys stay private
}
type Declaration struct {
    Verb        Verb
    Slot        Slot
    Available   bool
    Remaining   *int
    Why         *Shortfall
    ID          string
    Attack      *AttackRef
    Ability     *AbilityRef
    DeathSave   *DeathSaveRef
    Reaction    *ReactionRef
    Spell       *SpellRef
    TargetKind  TargetKind
    Candidates  []TargetCandidate
    MinTargets  int
    MaxTargets  int
    Cost        []CostComponent
}
type CastInput struct {
    Session, Member, DeclarationID string
    Targets []string
}
```

`Cost` is projected from the same `SpendProfile` the door charges: one action component and, for Bane, one generic charges component labelled “Level 1 spell slot.” It is display data, not authorization. Execution always regenerates and revalidates the declaration.

Proto source uses corresponding `min_targets`, `max_targets`, repeated generic cost components, and repeated `targets`. Implementers must inspect the current source branch and allocate/reserve tags there; this plan deliberately does not invent field numbers. Retired singular target and `spell_slots` names/numbers are reserved, not read or written.

## Reusable role prompts

At the start of each repository task, read that repository's `AGENTS.md`/`CLAUDE.md` and use the matching prompt from this project worktree:

- Toolkit tasks: `docs/teams/roles/rpg-toolkit-member/prompt.md`
- Proto task: `docs/teams/roles/rpg-api-protos-member/prompt.md`
- API task: `docs/teams/roles/rpg-api-member/prompt.md`
- Web task: `docs/teams/roles/rpg-dnd5e-web-member/prompt.md`
- Cross-repository proof/review: `docs/teams/roles/cross-team/prompt.md`

Keep one role-task prompt per task; do not turn these references into a broad policy rewrite.

---

### Task 1: Align the four stale toolkit guides

**Files:**
- Modify: `rpg-toolkit/rulebooks/dnd5e/README.md`
- Modify: `rpg-toolkit/docs/architecture/components/rulebook-dnd5e.md`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/CLAUDE.md`
- Modify: `rpg-toolkit/docs/how-to/add-a-mechanic.md`

**Interfaces:** Documents current composable `rulebooks/dnd5e/encounter` + `resolution` + `session`, `play/clock`, `core.Ref.ID`, `ConditionBehavior.Ref`, `Apply(ctx,bus)`, resolution-owned bus, and recoverable-resource spell slots. ADR-0038 stays authoritative; ADR-0045 is not edited. Existing resolution-local `gamectx` cast/room views are legitimate and are not prohibited.

- [ ] **Step 1: Confirm only the mapped obsolete examples are present**

```bash
: "${TOOLKIT_WORKTREE:?parent must bind the verified toolkit issue worktree}"
cd "$TOOLKIT_WORKTREE"
grep -nE 'combat\.ResolveAttack|spell slots are not implemented' \
  rulebooks/dnd5e/README.md \
  docs/architecture/components/rulebook-dnd5e.md \
  rulebooks/dnd5e/character/CLAUDE.md
```

Expected: the mapped obsolete symbols/claims are present. Review the changed examples against the current source-backed interfaces; do not treat every `gamectx` mention as stale. This is the RED proof for the documentation correction; no permanent grep-only test is added.

- [ ] **Step 2: Make only the mapped pointer/example corrections**

Replace old composition and signature examples; do not rewrite historical rationale, valid resolution-local `gamectx` usage, or publication policy.

- [ ] **Step 3: Verify GREEN and commit**

Rerun the scoped obsolete-symbol grep with `!`; expected: no stale matches. Review the edited examples against their mapped source interfaces, run `git diff --check`, then commit only these four files with `docs: align dnd5e mechanic guidance`.

---

### Task 2: Open the clean proto contract PR early

**Files:**
- Modify: `rpg-api-protos/dnd5e/api/v1alpha1/character.proto`
- Modify: `rpg-api-protos/dnd5e/api/session/v1alpha1/service.proto`
- Modify: `rpg-api-protos/dnd5e/api/session/v1alpha1/types.proto`
- Modify: `rpg-api-protos/dnd5e/api/session/v1alpha1/events.proto`

**Interfaces:** Produces the exact field names in Cross-task contracts: repeated cast targets; declaration min/max and generic cost; `RollSource.source_id`; `RollComponent.subtract_dice`; calculation on attack response, Struck, Missed, Saved, death-save response/event, and RollWindowOpened; qualified condition source ID. Retires and reserves singular targets and character `spell_slots`. No dual fields/readers. Kirk approved this deliberate alpha breaking change.

- [ ] **Step 1: Verify the new contract is absent**

```bash
: "${PROTOS_WORKTREE:?parent must bind the verified protos issue worktree}"
cd "$PROTOS_WORKTREE"
! sed -n '/message CastRequest/,/^}/p' dnd5e/api/session/v1alpha1/service.proto | \
  grep -q 'repeated string targets'
! sed -n '/message RollSource/,/^}/p' dnd5e/api/session/v1alpha1/events.proto | \
  grep -q 'source_id'
```

Expected: both commands pass, proving the current singular/source-less contract is the RED baseline. Do not add descriptor tests that merely retest generated protobuf mechanics.

- [ ] **Step 2: Edit proto source only**

Use the next verified free tags in the checked-out feature branch; reserve retired names/numbers. Define generic cost as currency/needed/label, not a spell-slot enum or resource-key registry. Do not edit generated Go or TypeScript and do not invent fields beyond the approved contract.

- [ ] **Step 3: Format/generate, then observe the intentional breaking report**

```bash
: "${PROTOS_WORKTREE:?parent must bind the verified protos issue worktree}"
cd "$PROTOS_WORKTREE"
buf format -w
make test
if make breaking; then
  echo 'expected the approved alpha retirements to be reported' >&2
  exit 1
fi
```

Expected: formatting, lint, and generation checks succeed. `make breaking` is intentionally **not green**: inspect its report and confirm it contains only the approved singular-target and `spell_slots` retirements. Any unrelated breaking finding blocks the task. Rerun both `sed` pipelines without leading `!` to prove the new fields separately; do not misreport the breaking check as passing.

- [ ] **Step 4: Commit, push, open the proto PR, and apply the approval label**

Commit the four proto files and push the wave branch. Open the real PR, apply the repository's explicit `breaking-change-approved` label, and verify it before relying on the intentional-breaking CI override:

```bash
: "${PROTOS_WORKTREE:?parent must bind the verified protos issue worktree}"
cd "$PROTOS_WORKTREE"
PROTO_PR=$(gh pr view --json number --jq .number)
gh pr edit "$PROTO_PR" --add-label breaking-change-approved
gh pr view "$PROTO_PR" --json labels --jq '.labels[].name' | grep -Fx breaking-change-approved
```

Wait for CI to publish the generated Go/TypeScript package versions; record the actual versions from CI output. Do not guess them and do not start API/web compilation against handwritten generated files. The label does not replace formatting/lint/generation checks; it only approves the reviewed intentional break.

---

### Task 3: Add Bane acquisition and one resource authority

**Files:**
- Modify: `rpg-toolkit/rulebooks/dnd5e/resources/keys.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/{data.go,character.go,draft.go,load.go}`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/{ledger_test.go,known_spells_test.go,long_rest_test.go,character_test.go}`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/choices/{requirements.go,requirements_detail_test.go,class_comprehensive_test.go}`

**Interfaces:** Produces only `SpellSlotLevel1`, the existing `BardSpells1` requirement with count 1/options Bane, finalized `KnownSpells`, and the level-1 recoverable resource. Consumes existing `RecoverableResource`, `KnownSpells`, and `SpellbookRequirement`. It does not call or assert cast profiles, target bounds, condition behavior, or contribution types that arrive in Tasks 4–5.

- [ ] **Step 1: Write failing acquisition/resource tests**

Representative assertions:

```go
req := choices.GetClassRequirements(classes.Bard).Spellbook
require.Equal(t, choices.BardSpells1, req.ID)
require.Equal(t, 1, req.Count)
require.Equal(t, 1, req.SpellLevel)
require.Equal(t, []spells.Spell{spells.Bane}, req.Options)
require.Equal(t, 2, bard.GetResource(resources.SpellSlotLevel1).Maximum())
```

Also reject zero/two spell selections and any non-Bane ref; prove finalization/reload retains Bane in `KnownSpells`, `KnownCantrips` is unchanged, and the four-spell class progression fact is unchanged.

- [ ] **Step 2: Run RED**

```bash
: "${TOOLKIT_WORKTREE:?parent must bind the verified toolkit issue worktree}"
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e"
go test -race ./character/... ./character/choices/... -run 'Bane|BardSpells1|SpellSlot|KnownSpell'
```

Expected: FAIL because the supported requirement/resource does not exist and duplicate slot state still exists. No cast-profile symbol is referenced.

- [ ] **Step 3: Implement only acquisition and canonical resource state**

Seed two recoverable uses from Bard level-1 progression during finalization. Delete duplicate persisted/runtime slot state and its proto-facing toolkit projection; do not seed higher levels/classes and do not enable Bane casting yet.

- [ ] **Step 4: Prove persistence and rest**

Add tests that a generic resource spend survives `ToData`/load, `LongRest` restores two, and serialized character data contains no `SpellSlots` field. Bane's concrete action+pool payment is deferred until its complete definition exists in Task 5.

- [ ] **Step 5: Run GREEN and commit**

Run the focused command, then `go test -race ./character/...`; commit only this root-module acquisition/resource unit.

---

### Task 4: Add contribution/address vocabulary, real Bane ownership, and the owner clock

**Files:**
- Modify: `rpg-toolkit/rulebooks/dnd5e/events/{events.go,damage_taken.go,roll_trace.go}`
- Create: `rpg-toolkit/rulebooks/dnd5e/conditions/{baned.go,baned_test.go}`
- Modify: `rpg-toolkit/rulebooks/dnd5e/conditions/{loader.go,concentrating.go,concentrating_test.go,display.go}`
- Modify: `rpg-toolkit/rulebooks/dnd5e/refs/conditions.go` and `refs/refs_test.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/combat/actions/{cast.go,cast_test.go}`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/{sheet_keeper.go,character.go,concentration_test.go}`
- Modify: `rpg-toolkit/rulebooks/dnd5e/monster/{monster.go,load.go,condition_removed_test.go}`

**Interfaces:** First declares `RollSource.SourceID`, `DiceContribution`, `RollKind`, `RollConditionOwner`, `ConditionAddress`, and the per-profile/per-owner skip-first fields. It then produces `refs.Conditions.Baned`, a minimal persisted `BanedCondition`, loader and `conditions.DISPLAY` registration in `conditions/display.go`, source-qualified imposed/removal/child facts, exact qualified matching, recipient-owned oldest selection, and the shared owner clock. It consumes the already-existing factual Bane spell ref; no complete cast profile is required.

- [ ] **Step 1: Write failing vocabulary and registration tests**

Pin the sole identity home at `RollSource.SourceID`, unsigned described `DiceContribution`, exact three-field `ConditionAddress`, and the per-profile `SkipFirstTurnEnd`/persisted `SkipNextTurnEnd` lifecycle. Assert `refs.Conditions.Baned` is canonical and the real loader plus `conditions.DISPLAY` in `conditions/display.go` resolve it after data round-trip.

- [ ] **Step 2: Write failing two-owner keeper tests**

Use two actual concentrating conditions, bard A and bard B, with same recipient/ref and qualified children. Ending A must remove only A; a mismatched source must not detach or dirty either character or monster. Empty source matches only empty source and is never a wildcard; all new Bane owners/children reject it.

- [ ] **Step 3: Write failing order/reload/clock tests**

Construct `[Bane(A), Bane(B)]`, round-trip through condition JSON, and assert one A description. Remove A and assert B without resetting either owner's remaining turns. For duration assert cast-turn end consumes grace at 10, other member turns leave 10, and exactly ten later caster ends expire all children after reload midway.

- [ ] **Step 4: Run RED**

```bash
: "${TOOLKIT_WORKTREE:?parent must bind the verified toolkit issue worktree}"
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e"
go test -race ./events/... ./conditions/... ./refs/... ./combat/actions/... ./character/... ./monster/... \
  -run 'Bane|Display|Qualified|Concentrat|Oldest|TurnEnd|RollSource|Contribution'
```

Expected: FAIL on absent vocabulary/ref/loader/display registration, bare-ref removal, absent Bane selector, and first-turn decrement.

- [ ] **Step 5: Declare contracts before their consumers, then implement behavior**

Add the event/address/contribution and per-profile timing fields first so the package compiles before condition/keeper code calls them. `BanedCondition` stores recipient, strict Bane source ref, caster source ID, and no face; it describes one subtractive `1d4` only for attack/save kinds. Character/monster scan a copied keeper slice in persisted order and choose the first active equal-potency Bane. Register the condition in both the loader and `conditions.DISPLAY`. Matching compares all three address fields.

- [ ] **Step 6: Preserve unrelated timing behavior and run GREEN**

Run the focused suite plus existing True Strike and Rage tests explicitly; change neither implementation to gain Bane behavior. Commit this vocabulary/condition/ownership/timing unit without enabling the Bane cast definition.

---

### Task 5: Complete strict calculations and enable the full Bane profile

**Files:**
- Modify: `rpg-toolkit/rulebooks/dnd5e/events/{roll_trace.go,roll_trace_test.go}`
- Modify: `rpg-toolkit/rulebooks/dnd5e/saves/{saves.go,saves_test.go,death_saves.go,death_saves_test.go}`
- Modify: `rpg-toolkit/rulebooks/dnd5e/refs/actions.go` and `refs/refs_test.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/spells/{cast.go,cast_test.go}`
- Modify: `rpg-toolkit/rulebooks/dnd5e/combat/actions/{cast.go,cast_test.go}`

**Interfaces:** Extends Task 4's already-compiled `RollSource`/`DiceContribution` vocabulary into strict `RollComponent` clone/validation/evaluation and `SavingThrowResult.Calculation`, then enables Bane's complete `CastDefinition`: action+level-1 pool price, range/save, 1..3 bounds, Task 4's Baned effect, and Task 4's per-profile grace. Generic evaluation consumes already-selected contributions and never sees condition lists or Bane stacking.

- [ ] **Step 1: Write strict trace/evaluator tests**

```go
calc := &events.RollCalculation{Components: []events.RollComponent{
    {Source: attackSource, Dice: d20Trace(14)},
    {Source: attackSource, Modifier: ptr(4)},
    {Source: baneSource("bard-a"), Dice: d4Trace(3), SubtractDice: true},
}, Total: 15}
require.NoError(t, events.ValidateRollCalculation(calc))
```

Reject missing/invalid refs on every component, missing Bane `SourceID`, signed/composite notation, negative/zero faces, subtract-without-dice, and wrong totals. Assert clone preserves exactly one `RollSource.SourceID` and the operator.

- [ ] **Step 2: Run calculation RED**

```bash
: "${TOOLKIT_WORKTREE:?parent must bind the verified toolkit issue worktree}"
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e"
go test -race ./events/... ./saves/... -run 'RollCalculation|Contribution|Bane|DeathSave'
```

Expected: FAIL because operator/calculation/evaluator paths are absent; Task 4's vocabulary itself already compiles.

- [ ] **Step 3: Implement generic and save evaluation without weakening validation**

Subtract only positive `Dice.Subtotal`; continue adding signed fixed modifiers. Require valid canonical source refs. Add explicit source-ref input where base d20/fixed call sites currently lack one rather than making refs optional. Ordinary saves classify success from checked total with no natural override. Death save face 1/20 keeps current special classification; faces 2..19 use the Bane-adjusted total against DC 10. Both return full calculations.

- [ ] **Step 4: Write the complete Bane definition RED test**

```go
definition := spells.CastDefinition(spells.Bane, 13)
require.NotNil(t, definition)
require.NotNil(t, definition.Cost)
require.NotNil(t, definition.Cast)
require.Equal(t, 1, definition.Cost.Slots[coreCombat.ActionStandard])
require.Equal(t, 1, definition.Cost.Pools[resources.SpellSlotLevel1])
require.Equal(t, 1, definition.Cast.MinTargets)
require.Equal(t, 3, definition.Cast.MaxTargets)
require.Len(t, definition.Cast.Effects, 1)
require.Equal(t, *refs.Conditions.Baned(), definition.Cast.Effects[0].Ref)
require.NotNil(t, definition.Cast.Concentration)
require.True(t, definition.Cast.Concentration.SkipFirstTurnEnd)
```

Use `coreCombat` for `github.com/KirkDiggler/rpg-toolkit/core/combat`; the existing action-economy constant is `ActionStandard`, and the existing `CastEffect` identity field is `Ref`. Also assert 30-foot reach, Charisma save, ten owner turn ends, and that unsupported catalog spells still return nil. Observe this focused test failing before adding the definition.

- [ ] **Step 5: Enable Bane using only now-existing contracts**

Compile the complete Bane definition and concrete `SpendProfile` using Task 3's resource and Task 4's condition/ref/timing types. Keep cantrip definitions action-only. Add atomic `CanPay`/`Pay` tests proving either action or pool shortage preserves both balances; do not introduce any new type consumed by an earlier task.

- [ ] **Step 6: Run GREEN, commit, and push the coherent root provider**

Run all `./events/... ./saves/... ./spells/... ./combat/actions/...` tests and the full root-module tests/lint. Commit the calculation/profile unit, then push the toolkit wave branch and verify the remote SHA:

```bash
: "${TOOLKIT_WORKTREE:?parent must bind the verified toolkit issue worktree}"
cd "$TOOLKIT_WORKTREE"
git push -u origin HEAD
ROOT_SHA=$(git rev-parse HEAD)
git ls-remote origin "$(git branch --show-current)" | grep "$ROOT_SHA"
```

Expected: the remote branch names the exact root-provider commit containing Tasks 3–5. This push enables the first real pseudo-version; it does not create a tag.

---

### Task 6: Carry neutral ordered casts and calculations in encounter

**Files:**
- Modify: `rpg-toolkit/rulebooks/dnd5e/encounter/{cast.go,cast_test.go,outcome.go,outcome_test.go,roll_trace.go,roll_trace_test.go,activation.go,activation_test.go,concentration_test.go}`

**Interfaces:** Mirrors target lists, `ConditionAddress`, `RollSource.SourceID`, `SubtractDice`, and calculations without importing Bane or interpreting arithmetic. Produces one Cast beat plus ordered per-target Saved/result beats. It does not edit the resolution consumer's module pins.

- [ ] **Step 1: Write failing neutral-record tests**

Record three targets with save outcomes fail/pass/fail; assert one ordered Cast beat, then three Saved beats and only two applied-condition beats in target order. Round-trip attack, ordinary save, concentration save, cast save, and death save calculations; malformed source/operator/total must fail generic validation.

- [ ] **Step 2: Run RED**

```bash
: "${TOOLKIT_WORKTREE:?parent must bind the verified toolkit issue worktree}"
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e/encounter"
go test -race ./... -run 'Cast|Outcome|Roll|Condition|DeathSave'
```

Expected: FAIL on singular target and scalar-only save/attack records.

- [ ] **Step 3: Implement field-for-field neutral carriers**

Keep one generic calculation converter/validator. Never select a condition, roll, recompute total, or import D&D Bane content.

- [ ] **Step 4: Run GREEN, commit, and push encounter**

Run `go test -race ./...`, `golangci-lint run ./...`, commit, and push the toolkit wave branch. Resolution remains on its prior pins until Task 7 can adopt these provider commits together with adapting code.

---

### Task 7: Resolve one paid fan-out and every Bane-affected roll

**Files:**
- Modify: `rpg-toolkit/rulebooks/dnd5e/resolution/{go.mod,go.sum}` together with adapting code
- Modify: `rpg-toolkit/rulebooks/dnd5e/resolution/{action.go,cast.go,contest.go,save.go,death_save.go,strike.go,concentration.go}`
- Modify: `rpg-toolkit/rulebooks/dnd5e/resolution/{cast_action_test.go,cost_test.go,concentration_test.go,death_save_test.go,long_rest_test.go,strike_pose_test.go,strike_test.go}`

**Interfaces:** At task start consumes the pushed Task 6 root/encounter pseudo-versions, then consumes generic target bounds, participant `RollConditionOwner`, `DiceContribution`, strict calculations, qualified imposed effects, and encounter records. Produces ordered `CastOutcome`, calculation-bearing strike/save/follow-ups, and frozen strike version 2. Pin changes are committed only with the adapting resolution code after GREEN.

- [ ] **Step 1: Adopt the pushed root/encounter providers before writing RED tests**

```bash
: "${TOOLKIT_WORKTREE:?parent must bind the verified toolkit issue worktree}"
PROVIDER_SHA=$(git -C "$TOOLKIT_WORKTREE" rev-parse HEAD)
git -C "$TOOLKIT_WORKTREE" ls-remote origin "$(git -C "$TOOLKIT_WORKTREE" branch --show-current)" | grep "$PROVIDER_SHA"
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e/resolution"
GOPROXY=direct go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@"$PROVIDER_SHA"
GOPROXY=direct go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter@"$PROVIDER_SHA"
go mod tidy
go list -m github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e \
  github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter
```

Expected: `go list -m` reports Go-generated pseudo-versions derived from the verified pushed SHA. Keep `go.mod`/`go.sum` uncommitted while adapting resolution; never make a pin-only broken commit and never add `replace` or `go.work`.

- [ ] **Step 2: Write whole-list preflight/payment tests**

For empty, duplicate, fourth, absent, stale/ineligible, and out-of-range IDs assert: error, zero roller calls, unchanged action/pool, and unchanged old concentration. For 1/2/3 valid targets assert one payment and caller order.

For the all-save recast, start with a real existing concentration owner and its old qualified children, plus an unrelated caster's overlapping owner/child records. Force every new target save to succeed and pin the observed order as payment → removal of the casting owner's old owner/children → ordered saves → installation of exactly one **new** zero-child owner. Assert one payment, no old child survives, and the unrelated caster's owner, children, and clock are unchanged.

- [ ] **Step 3: Run cast tests for RED**

```bash
: "${TOOLKIT_WORKTREE:?parent must bind the verified toolkit issue worktree}"
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e/resolution"
go test -race ./... -run 'Bane|Cast|Cost|Concentration|AllSave'
```

Expected: FAIL because action/cast outcomes and contests are singular and the recast lifecycle is not implemented.

- [ ] **Step 4: Implement preflight, door, drop, fan-out, and new owner**

The sequence is pure whole-list construction, one `combat.Pay`, synchronous removal of only the caster's qualified prior owner/children, ordered contest/delivery, then one new owner holding every qualified delivered child (including zero). Session must not loop targets.

- [ ] **Step 5: Write attack/save/concentration/death-save tests**

Assert exactly one selected d4 with two active Banes, Bane + advantage rolls two d20 faces but one d4, ordinary/concentration/death saves use the correct policy, and ability checks remain untouched.

- [ ] **Step 6: Write frozen Inspiration tests using the public answers**

Pause after a Bane-adjusted attack, serialize/reload, then answer the actual public `OfferKeep` and `OfferSpend` choices. Assert the exact Bane face/source remains, no extra Bane roller call occurs, and `OfferSpend` appends only the Inspiration component once. Version-1 frozen input must fail closed.

- [ ] **Step 7: Implement roll custody, run GREEN, then commit pins with code**

Resolution asks the target owner for descriptions before RNG, evaluates generically, and stores the calculation before the post-roll pose. Run the focused suite, then `go test -race ./...` and `golangci-lint run ./...`. Commit `resolution/go.mod`, `resolution/go.sum`, and the adapting resolution code/tests as one green consumer unit; then push the toolkit wave branch. Do not commit an isolated pseudo-pin.

---

### Task 8: Compile session offers and project results without rules

**Files:**
- Modify: `rpg-toolkit/rulebooks/dnd5e/session/{go.mod,go.sum}` together with adapting code
- Modify: `rpg-toolkit/rulebooks/dnd5e/session/{casts.go,cast.go,castoutcome.go,death_save.go,afford.go,offers.go,attack.go,react_post_roll.go,types.go,events.go}`
- Modify: `rpg-toolkit/rulebooks/dnd5e/session/{cast_test.go,concentration_break_test.go,death_save_test.go,events_internal_test.go,post_roll_test.go,turn_concentration_test.go}`

**Interfaces:** At task start consumes the pushed root/encounter/resolution pseudo-versions, then consumes complete rulebook definitions and resolution/encounter outputs. Produces one Bane declaration per authorized known spell, ordered candidates, min/max, generic price, `CastInput.Targets`, and exact calculation/address projection. Pin changes are committed only with adapting session code after GREEN.

- [ ] **Step 1: Adopt the pushed toolkit providers before writing RED tests**

```bash
: "${TOOLKIT_WORKTREE:?parent must bind the verified toolkit issue worktree}"
PROVIDER_SHA=$(git -C "$TOOLKIT_WORKTREE" rev-parse HEAD)
git -C "$TOOLKIT_WORKTREE" ls-remote origin "$(git -C "$TOOLKIT_WORKTREE" branch --show-current)" | grep "$PROVIDER_SHA"
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e/session"
GOPROXY=direct go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@"$PROVIDER_SHA"
GOPROXY=direct go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter@"$PROVIDER_SHA"
GOPROXY=direct go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/resolution@"$PROVIDER_SHA"
go mod tidy
go list -m github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e \
  github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter \
  github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/resolution
```

Expected: all three are Go-generated pseudo-versions from the verified pushed commit. Keep pin edits uncommitted while adapting session; no `replace`, `go.work`, or pin-only commit.

- [ ] **Step 2: Write offer and cast-door tests**

Known Bane yields one row with 1..3 target bounds and action + labelled charge cost; unsupported known content yields no row. A three-target request reaches resolution once. After success, refreshed Afford is unavailable for action, and a same-turn second Cast is refused with no pool spend or RNG despite one slot use remaining.

- [ ] **Step 3: Write projection tests**

Assert `SourceID`, positive d4 face, subtraction, and full calculations survive hit, miss, ordinary/cast/concentration/death save and frozen-window records. Strict decode rejects absent source refs and mismatched totals; session never repairs them.

- [ ] **Step 4: Run RED**

```bash
: "${TOOLKIT_WORKTREE:?parent must bind the verified toolkit issue worktree}"
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e/session"
go test -race ./... -run 'Bane|Cast|Afford|Saved|Struck|DeathSave|Inspiration|Roll'
```

Expected: FAIL on cantrip-only offers, singular targets, and missing calculation fields.

- [ ] **Step 5: Implement opaque compilation/projection**

Use `Definition.Cost`, `KnownSpells`, and current candidate/reach seams. Regenerate the selected offer before execution. Copy calculations and addresses field for field; add no Bane ref comparisons.

- [ ] **Step 6: Run GREEN, commit pins with code, and push**

Run focused and full session tests plus lint. Commit `session/go.mod`, `session/go.sum`, and adapting session code/tests together only after GREEN; push the toolkit wave branch and verify `git ls-remote` reports the exact tested SHA.

---

### Task 9: Verify and capture the pushed toolkit wave

**Files:**
- No source or dependency-manifest changes

**Interfaces:** Read-only publication preparation. Captures the exact pushed provider SHA, real pseudo-version graph already committed inside toolkit consumers, and completed toolkit gates. API adoption is intentionally deferred to Task 10 so no consumer is pinned without its adapting code.

- [ ] **Step 1: Verify the final toolkit provider head is pushed**

```bash
: "${TOOLKIT_WORKTREE:?parent must bind the verified toolkit issue worktree}"
cd "$TOOLKIT_WORKTREE"
TOOLKIT_SHA=$(git rev-parse HEAD)
git ls-remote origin "$(git branch --show-current)" | grep "$TOOLKIT_SHA"
```

Expected: the remote branch resolves to the exact full-toolkit commit; no tag is created by hand.

- [ ] **Step 2: Run and record the completed toolkit gates**

From the bound toolkit worktree, run `go test -race ./...` and `golangci-lint run ./...` in root, encounter, resolution, and session modules, followed by toolkit `make test-all`, `make lint-all`, and `git diff --check`. Record the exact SHA, commands, and results. Inspect resolution/session `go list -m` output to capture their real Go-generated pseudo-versions and verify there is no `replace` or `go.work`.

- [ ] **Step 3: Leave consumers untouched**

Do not enter or edit the API worktree, run an API gate, or create a commit in this task. Task 10 adopts this captured SHA together with generated protos and handler adaptation.

---

### Task 10: Adopt providers with API adaptation and real Redis acceptance

**Files:**
- Modify: `rpg-api/go.mod`, `go.sum` together with adapting code
- Modify: `rpg-api/internal/handlers/dnd5e/v1alpha1/character/{handler.go,converters.go,converters_test.go,handler_test.go}`
- Modify: `rpg-api/internal/handlers/dnd5e/session/v1alpha1/{cast.go,convert.go,convert_test.go}`
- Modify: `rpg-api/internal/integration/character/creation_test.go`
- Modify: `rpg-api/internal/integration/session/{cast_acceptance_test.go,bard_inspiration_acceptance_test.go}`

**Interfaces:** At task start consumes the actual Task 9 toolkit SHA as Go-generated pseudo-versions and the actual CI-generated proto version. Produces thin field-for-field Character/Session handlers. No rulebook imports in handler mapping. All dependency pins are committed only with adapted API code after the full gate.

- [ ] **Step 1: Adopt actual toolkit and generated-proto providers before RED tests**

```bash
: "${TOOLKIT_WORKTREE:?parent must bind the verified toolkit issue worktree}"
: "${API_WORKTREE:?parent must bind the verified API issue worktree}"
TOOLKIT_SHA=$(git -C "$TOOLKIT_WORKTREE" rev-parse HEAD)
git -C "$TOOLKIT_WORKTREE" ls-remote origin "$(git -C "$TOOLKIT_WORKTREE" branch --show-current)" | grep "$TOOLKIT_SHA"
cd "$API_WORKTREE"
GOPROXY=direct go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@"$TOOLKIT_SHA"
GOPROXY=direct go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter@"$TOOLKIT_SHA"
GOPROXY=direct go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/resolution@"$TOOLKIT_SHA"
GOPROXY=direct go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/session@"$TOOLKIT_SHA"
```

Use `go get` for the existing proto dependency module path with the exact CI-published Go version recorded in Task 2, then run `go mod tidy` and `go list -m` for every changed toolkit/proto module. Do not invent a module/version or edit generated clients. Keep all `go.mod`/`go.sum` changes uncommitted while adapting handlers; the current singular `sdk.CastInput{Target: ...}` consumer is expected to require adaptation before compilation can be green.

- [ ] **Step 2: Write failing converter tests**

Test one `SpellbookRequirement` projection via existing `SpellOptions`, repeated target request mapping, declaration bounds/cost, and every generic calculation/address field. Assert absent stays absent and malformed toolkit output is refused rather than repaired.

- [ ] **Step 3: Run RED**

```bash
: "${API_WORKTREE:?parent must bind the verified API issue worktree}"
cd "$API_WORKTREE"
go test ./internal/handlers/dnd5e/session/v1alpha1 ./internal/handlers/dnd5e/v1alpha1/character \
  -run 'Bane|Cast|Declaration|Roll|Choice'
```

Expected: FAIL until the new generated target-list and trace fields are mapped.

- [ ] **Step 4: Implement thin mappings**

Map only fields; do not inspect Bane refs, choose sources, calculate totals, or infer spell prices.

- [ ] **Step 5: Add real acceptance, including the paid all-save replacement**

Extend creation acceptance for Bane-only selection/finalize/reload and session acceptance for payment refusal, invalid third target, mixed saves, same-turn second cast, qualified two-owner removal, duration, all calculation paths, and Inspiration freeze. The all-save case starts with the casting character's real old concentration owner and qualified children plus an unrelated caster's overlapping owner/children; it proves one payment, old-owner/child removal after payment, all ordered saves, exactly the new zero-child owner, and preservation of the unrelated owner/children/clock. Use the public `OfferKeep` and `OfferSpend` Inspiration answers. Use real handlers/session manager/miniredis; no hand-seeded known-spell shortcut for the creation scene.

- [ ] **Step 6: Run the full API gate, verify pseudo-pinned release hygiene, and commit once**

```bash
: "${API_WORKTREE:?parent must bind the verified API issue worktree}"
cd "$API_WORKTREE"
go test ./...
go test -v -race ./internal/integration/session \
  -run 'TestAcceptance_(TheCastDoor|TrueStrike|ViciousMockery|BardicInspiration|Bane)'
make ci-check
scripts/verify-release-pin.sh
git diff --check
```

Expected: PASS. `verify-release-pin.sh` permits real Go pseudo-versions; it verifies the absence of `go.work`, local-toolkit linkage, and `replace`, not tag-only dependencies. Run only one Docker-backed integration runner at a time per the runbook. Commit API `go.mod`/`go.sum`, handlers, converters, and acceptance tests together only after this gate; then push and capture the actual API SHA for Task 12.

---

### Task 11: Add minimal web creation, cast selection, and text trace

**Files:**
- Modify: `rpg-dnd5e-web/package.json`, lockfile only via package manager when adopting generated protos
- Modify: `rpg-dnd5e-web/src/{types/choices.ts,utils/choiceConverter.ts,components/ChoiceRenderer.tsx}`
- Modify: `rpg-dnd5e-web/src/character/creation/{ClassSelectionModal.tsx,InteractiveCharacterSheet.tsx}`
- Modify: `rpg-dnd5e-web/src/api/useSessionCast.ts`
- Modify: `rpg-dnd5e-web/src/components/session/combat-experience/{selection.ts,TargetSurface.tsx,types.ts,useSessionCombatExperience.ts,ActionDock.tsx,story.ts,rollTrace.ts}`
- Modify: `rpg-dnd5e-web/src/{components/ChoiceRenderer.test.tsx,character/creation/cantripChoice.test.tsx,character/creation/ClassSelectionModal.test.tsx}`
- Modify: `rpg-dnd5e-web/src/components/session/combat-experience/{castFlow.test.tsx,castStory.test.ts}`

**Interfaces:** Consumes generated `CHOICE_CATEGORY_SPELLS`, SpellOptions, target bounds/candidates/cost, repeated targets, and provider calculation. Produces no D&D calculation or dice-style lookup.

- [ ] **Step 1: Adopt the actual CI-generated TypeScript package**

Use the repository's package-manager command with the exact version from Task 2; commit the real lockfile result, never a guessed version.

- [ ] **Step 2: Write failing creation and cast tests**

Creation renders one Bane option, requires exactly one selection, submits `spellRefs`, and rehydrates known spells. Cast multi-select preserves click order, rejects duplicate/unavailable/fourth targets, submits one request, and displays provider cap/cost.

- [ ] **Step 3: Write failing Story tests**

Feed actual generated event shapes with d20/fixed/Bane components and assert positive face plus subtraction and roster-resolved contributor name. Assert displayed total is provider-authored; remove `saved.total - saved.roll` derivation.

- [ ] **Step 4: Run RED**

```bash
: "${WEB_WORKTREE:?parent must bind the verified web issue worktree}"
cd "$WEB_WORKTREE"
npm test -- src/components/session/combat-experience/castFlow.test.tsx \
  src/components/session/combat-experience/castStory.test.ts
```

Expected: FAIL on one-target selection and scalar save formatting.

- [ ] **Step 5: Implement the minimal normal flow**

Generalize the existing cantrip choice renderer for SPELLS and the member selector for provider-capped ordered lists. Send one Cast call. Reuse generic text trace; do not touch dice assets, preset selection, animation, or player dice settings.

- [ ] **Step 6: Run GREEN, commit, and push the tested web ref**

Run focused tests, all related creation tests, then `npm run ci-check`; commit and push the wave branch. Verify `git ls-remote` reports the exact tested web SHA and capture it for Task 12.

---

### Task 12: Isolated named-stack proof, independent review, then eventual tag adoption

**Files:**
- No source changes for proof/review
- Local uncommitted runtime inputs/state: `envs/local/bane.env` and `.runtime/local/bane/state.env` under the bound game-dev workspace
- Later pin-only commits: intended toolkit consumer `go.mod`/`go.sum`, rpg-api `go.mod`/`go.sum`, and web package lockfile

**Interfaces:** Separates exact pushed API/web refs plus the API's committed toolkit pseudo-version graph from post-merge real tags. Uses the existing `scripts/dev-env.sh` named-stack contract, not a shared primary overlay or the one-module toolkit override.

- [ ] **Step 1: Prove the `local/bane` namespace is unclaimed before creating anything**

```bash
set -euo pipefail
: "${GAME_DEV_WORKSPACE:?parent must bind the game-dev workspace for named runtime only}"
: "${API_WORKTREE:?parent must bind the verified API issue worktree}"
: "${WEB_WORKTREE:?parent must bind the verified web issue worktree}"
cd "$GAME_DEV_WORKSPACE"
test ! -e envs/local/bane.env
test ! -e .runtime/local/bane/state.env
API_SHA=$(git -C "$API_WORKTREE" rev-parse HEAD)
WEB_SHA=$(git -C "$WEB_WORKTREE" rev-parse HEAD)
export API_SHA WEB_SHA
git -C "$API_WORKTREE" ls-remote origin "$(git -C "$API_WORKTREE" branch --show-current)" | grep "$API_SHA"
git -C "$WEB_WORKTREE" ls-remote origin "$(git -C "$WEB_WORKTREE" branch --show-current)" | grep "$WEB_SHA"
```

Both absence checks must pass. If either path exists, stop and obtain the recorded owner's explicit authorization; do not overwrite it or run `up`, because `up local/bane` first tears down that same namespace. This task never stops/restarts a primary, lab, or other existing lane and never wipes data.

- [ ] **Step 2: Supply and verify two actually free host ports**

The operator supplies distinct `BANE_API_HOST_PORT` and `BANE_WEB_HOST_PORT` values; there are no sample ports assumed free. Validate them immediately before manifest creation:

```bash
set -euo pipefail
: "${BANE_API_HOST_PORT:?operator must supply a verified-free API port}"
: "${BANE_WEB_HOST_PORT:?operator must supply a verified-free web port}"
python3 - "$BANE_API_HOST_PORT" "$BANE_WEB_HOST_PORT" <<'PY'
import socket, sys
ports = [int(value) for value in sys.argv[1:]]
assert all(1 <= port <= 65535 for port in ports), "ports must be in 1..65535"
assert len(set(ports)) == len(ports), "ports must be distinct"
for port in ports:
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", port))
print("verified free:", *ports)
PY
```

Any bind failure blocks proof; do not substitute an unverified port or disturb another listener.

- [ ] **Step 3: Create the real named manifest and start only its isolated stack**

```bash
set -euo pipefail
: "${GAME_DEV_WORKSPACE:?parent must bind the game-dev workspace for named runtime only}"
: "${API_SHA:?Step 1 must capture the pushed API SHA}"
: "${WEB_SHA:?Step 1 must capture the pushed web SHA}"
cd "$GAME_DEV_WORKSPACE"
test ! -e envs/local/bane.env
test ! -e .runtime/local/bane/state.env
mkdir -p envs/local
cat > envs/local/bane.env <<EOF
RPG_API_REF=$API_SHA
RPG_DND5E_WEB_REF=$WEB_SHA
RPG_API_HOST_PORT=$BANE_API_HOST_PORT
RPG_WEB_HOST_PORT=$BANE_WEB_HOST_PORT
EOF
! grep -qE '^RPG_TOOLKIT_(REF|PATH)=' envs/local/bane.env
scripts/dev-env.sh up local/bane
scripts/dev-env.sh status local/bane
```

Expected: the script loads `envs/local/bane.env`, creates `.runtime/local/bane/state.env`, uses Compose project `rpg-local--bane` and image `rpg-api:local--bane`, and starts fresh namespace-owned Redis/sandbox services. `RPG_TOOLKIT_REF`/`RPG_TOOLKIT_PATH` stay omitted: they select a different one-module override, while this API image must build from the pushed API SHA whose `go.mod` contains the real toolkit pseudo-versions.

- [ ] **Step 4: Capture runtime and dependency receipts**

Capture `RPG_API_REF` and `RPG_DND5E_WEB_REF` from `envs/local/bane.env`. Capture the non-secret namespace/image/port/resolved-source-path receipts from `.runtime/local/bane/state.env` and `scripts/dev-env.sh status local/bane`. `state.env` does not store ref fields. Compare resolved checkout HEADs with the manifest's pushed SHAs:

```bash
(
  set -euo pipefail
  : "${GAME_DEV_WORKSPACE:?parent must bind the game-dev workspace}"
  cd "$GAME_DEV_WORKSPACE"
  source envs/local/bane.env
  source .runtime/local/bane/state.env
  test "$(git -C "$API_SOURCE" rev-parse HEAD)" = "$RPG_API_REF"
  test "$(git -C "$WEB_SOURCE" rev-parse HEAD)" = "$RPG_DND5E_WEB_REF"
  printf '%s\n' "API_REF=$RPG_API_REF" "WEB_REF=$RPG_DND5E_WEB_REF" \
    "COMPOSE_PROJECT=$COMPOSE_PROJECT" "API_IMAGE=$API_IMAGE" \
    "API_PORT=$API_PORT" "WEB_PORT=$WEB_PORT" \
    "API_SOURCE=$API_SOURCE" "WEB_SOURCE=$WEB_SOURCE"
)
```

Also record the CI-generated proto versions and committed toolkit pseudo-versions from the pushed API `go.mod`/`go.sum`. Verify the running Character, Afford, Cast, React, record, and stream paths resolve to those receipts. Do not claim a local-file-linked or shared-primary graph.

- [ ] **Step 5: Walk the 16-scene matrix below**

Capture commands, request/event observations, and Story screenshots/logs. Generic textual d4 proof is sufficient; do not claim source-styled 3-D dice.

- [ ] **Step 6: Independent review of exact tested heads**

Use a fresh read-only engine/contract reviewer. Review boundaries, source identity, RNG custody, freeze/reload, removal qualification, proto cleanliness, API thinness, web non-arithmetic, and named-stack receipts. Fix findings on the same wave branches and rerun affected gates/proof.

- [ ] **Step 7: Merge providers inside-out only after approval**

Merge toolkit providers in dependency order. Wait for CI to publish real module tags. Replace pseudo-version pins in toolkit consumers/API with those observed tags and rerun module/API gates. Merge API to `dev`, then web to `dev`. Protos retain their already-generated CI release. Do not invent or predeclare tags.

- [ ] **Step 8: Publish the final review record**

The PR comment names exact reviewed heads, verdict, Critical/Important/Minor counts, commands/evidence, and disposition. No automatic merge/release/deploy follows.

---

## Acceptance coverage matrix

| Design scene | Primary future automated proof | Real-path proof |
|---|---|---|
| 1 Payment refusal | Tasks 5 and 7 cost/door tests | API Bane acceptance: action-only and pool-only shortage, zero RNG/mutation |
| 2 Same-turn refresh | Task 8 session Cast/Afford test | one cast leaves one pool point; second RPC refused for action |
| 3 Invalid third target | Tasks 7–8 whole-list tests | empty/duplicate/absent/ineligible/range/fourth request shows no payment/drop/RNG |
| 4 One supported acquisition | Tasks 3, 10, 11 | real Character Service + web choice finalization/reload |
| 5 Resource lifecycle | Task 3 persistence/rest tests | API reload and LongRest shows 1 then 2; no SpellSlots projection |
| 6 Mixed saves | Tasks 6–8 ordered result tests | one three-target Cast RPC and ordered stream beats |
| 7 All save | Tasks 7–8 ordered recast tests; Task 10 acceptance | old owner+qualified children → payment → drop → all saves → new zero-child owner; unrelated owner/children persist |
| 8 Two real owners | Task 4 character+monster keeper tests | overlap from bard A/B; ending A preserves B owner/child/clock |
| 9 Oldest and reload | Task 4 selector JSON tests; Task 7 roll test | reload, A contributes once; end A; B contributes with no duration reset |
| 10 Duration | Task 4 owner turn tests | cast turn skipped, other turns ignored, ten subsequent caster ends expire |
| 11 Attack calculation | Tasks 5–8 trace/strike/wire tests | response/record/Story show positive face, subtraction, ref, caster ID, total |
| 12 Ordinary save | Tasks 5–8 save trace tests | non-concentration Saved event/Story carries calculation |
| 13 Concentration save | Tasks 4, 5, 7–8 follow-up tests | held/failed checks carry calculation; failed removals qualified |
| 14 Death save | Tasks 5–8 separate-path tests | faces 1/20 preserve policy; 2..19 use Bane-adjusted DC-10 total |
| 15 Inspiration | Tasks 7–8 frozen reload tests; Task 10 acceptance | `OfferKeep`/`OfferSpend` after reload reuse Bane face/source; no reroll/action spend |
| 16 Regression | Tasks 3–8 existing suites | current cantrips list-of-one/action-only; True Strike and Rage unchanged |

## Final verification checklist

- [ ] Every new test was first observed failing for the intended missing behavior, then passing.
- [ ] Run `git diff --check` in every changed repository.
- [ ] Run `go test -race ./...` and `golangci-lint run ./...` in all four changed toolkit modules; run toolkit `make test-all` and `make lint-all`.
- [ ] Run proto `buf format -w` and `make test` green; observe `make breaking` report the approved retirements, verify no unrelated break, and verify the PR has `breaking-change-approved`.
- [ ] Run API `go test ./...`, named real integration acceptance, `make ci-check`, and `scripts/verify-release-pin.sh`.
- [ ] Run focused web tests and `npm run ci-check`.
- [ ] Search for forbidden residue: committed `replace`, `go.work`, dual target fields/readers, `SpellSlots`, missing source refs, sibling contributor-ID fields, Bane ref comparisons outside rulebook/conditions, session dice/bus code, UI dice-set/preset additions, invented tags/versions, primary-root `cd`, shared-primary overlays, or local-file links.
- [ ] Diff scope contains only planned files; no dependency changed except generated proto/toolkit pins and no environment state is committed.
- [ ] Independent review is bound to exact tested heads; local pseudo-version proof and eventual real-tag adoption are reported as separate stages.
