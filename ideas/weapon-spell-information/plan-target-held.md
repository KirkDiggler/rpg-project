# Plan: effect information, target-held effects (rpg-project#520)

## Decisions needed

Two decisions block parts of this plan. Every other task can start without
them. Each one says which tasks it blocks.

### D1 — What refreshes a sighting when only a condition changes (blocks Task 5's freshness step and the walk)

**Evidence.**
- R16 makes conditions part of the sight snapshot. A snapshot is written only
  when sight refreshes. In `encounter`, that happens in
  `(*Encounter).refreshSightDeclaring`, which is called from movement
  (`step.go`, `clocks.go` `settleWalk`), join/exit (`encounter.go`), doors,
  hold, reserve, world time, and `Recheck` (`sightedbeat.go`).
- `RecordCast` (`cast.go`) and `RecordActivation` (`activation.go`) record
  condition-applied and condition-removed results, but they do not refresh
  sight. Ending a turn does not refresh sight either. Turn-boundary removals
  (Dodging at its holder's turn start, Guiding Bolt at its caster's turn end)
  go through the same commit path without a re-look.
- So without a trigger: the cleric casts Faerie Fire on a goblin, the turn
  passes to the rogue, and the rogue's Attack candidate shows no Faerie Fire
  row until somebody moves. A Guiding Bolt row would also stay after the bolt
  is spent.
- The workspace already has a pattern for "an observable fact changed
  without movement": `encounter.(*Encounter).Recheck` and
  `session.(*Manager).Recheck`. The writer says *who* changed, never *what*,
  and every watcher re-reads their own testimony. Equipment uses it today.
- The session commit path (`session/write.go` `(*Manager).commit`) is the one
  place every verb passes through before numbering. It already holds
  commit-time settlements: `reconcileFogMembership`,
  `exitDissolvedCombatants`, `settleExperience`.

**Consequence.** Any trigger that goes through `Recheck` also emits a
sighting "changed" beat to each watcher of the declared member. That is a new
user-visible story event on every condition change a watcher can see. With no
trigger, the rows go stale and the Task 8 walk fails.

**Recommendation.** At commit, before numbering, the session declares every
member whose set of condition addresses differs from what it was when the
verb opened. It does this through `encounter.Recheck`, and skips the call on
a closed encounter. This is the equipment precedent applied to conditions:
one writer, the composition is told who changed, and testimony stays per
observer. Task 5 builds this.

### D2 — In Fog does not answer by gaining a "sees" fact (it changes the brief's expectation; blocks nothing in this plan)

**Evidence.**
- `conditions/in_fog.go` `InFogCondition` subscribes to no attack event. Its
  godoc says "this condition never adds Blinded or an attack-roll modifier".
- What fog does to an attack comes from the generic sight rule in
  `resolution/visibility.go` `addSightAttackModifiers`:
  - an attacker that cannot see its target has disadvantage;
  - a target that cannot see its attacker gives the attacker advantage.
  That rule is called from `strike.go` (in `effectiveACStep`), reads
  `gamectx.Visibility`, and is keyed to no condition.
- If an In Fog row answered "disadvantage, you cannot see the target", that
  would be a second predicate for a modifier owned by another function. The
  law forbids that ("a rule keeps no second predicate").
- Hidden needs no sight fact. `conditions/hidden.go` `onAttackChain` applies:
  - advantage to the holder's attacks (an answering rule on the batch1
    branch);
  - disadvantage to attacks against the holder, unconditionally.
  So the target-held Hidden rule answers unconditionally, matching execution.
  Gating it on sight would be a gameplay change no ruling asks for.

**Consequence.**
- In Fog stays not-yet-answering in both censuses.
- Only Faerie Fire reads the new sight fact.
- The unseen-combatant rule stays invisible to information. That is a gap in
  what the tooltip covers, not a disagreement with the swing, because no row
  claims otherwise.

**Recommendation.** Defer. Showing the generic sight rule as a row needs its
own ruling: it has no condition source and no census entry, so the row would
have no owner. The `Sees` pair fact this plan adds reads in both directions,
which is exactly what that rule needs. So this wave does not rule that out.

---

## Header

**Goal and scope.** When a player inspects a weapon Attack against a
candidate, they see rows for the conditions *that candidate* holds which bear
on the attack. The answering set:

- **Faerie Fire.** Advantage if the attacker can see the target.
- **Guiding Bolt.** Advantage on the next attack against the target.
- **Dodging.** Disadvantage.
- **Prone when attacked.** Advantage within 5 feet, disadvantage beyond.
- **Sanctuary.** The attacker must save, or the attack is lost.
- **Hidden.** Disadvantage on attacks against the hidden target.

Every other condition a target can hold is classified once, in a second
census:
- "bears but cannot yet answer" gives an unavailable row;
- "does not bear" gives no row.

The plan delivers all four parts:
- R16: sightings carry conditions.
- R17: by-reference rules read the frame's held list.
- R18: a full row on the candidate, through a new proto field.
- A `Sees` pair fact for Faerie Fire.

The execution handlers for the answering set ask the same rule against the
execution frame, and the frame now carries held conditions and sight.

**Non-goals.**
- Effects held by a third party (Protection fighting style, for example).
- Rows for save spells or checks.
- Off-turn rows (R12).
- A perceivability filter (R16).
- The generic unseen-combatant rule as a row (D2).
- Reckless Attack's "attacks against you" half. It is classified as
  not-yet-answering, with a flag to follow up.
- Projecting seen conditions onto the session's sighting wire. Testimony
  gains them for rules; the client's sighting payload is unchanged.
- Fixing the Sanctuary cast-path predicate (see Open items).

**Authority.** `ideas/weapon-spell-information/design.md` on branch
`docs/effect-frame-moments` (rpg-project PR #533): law plus R1–R18,
"Open: None". Issue rpg-project#520. This plan sits beside the design and the
first-delivery `plan.md`.

**Constraints (only the binding ones).**
- Toolkit: one nearest-go.mod module per PR. Root `rulebooks/dnd5e`,
  `encounter`, `resolution` and `session` are separate PRs (`rpg-toolkit/CLAUDE.md`).
- Encounter cannot import the rulebook (law C1, `encounter/go.mod` has no root
  dependency). It learns conditions only through an injected capability, the
  same way it learns `Equipment`.
- Session S2 (`session/boundary_test.go` `TestNoInnerTypeCrossesTheBoundary`):
  new session wire fields hold only strings and string enums.
- Protos: authored contract only, `make test`, additive field, no new file.
  Protos merge first so CI publishes bindings.
- rpg-api: base `origin/dev`, `make ci-check`. A PR pinned to pseudo-versions
  stays draft until the real tags are adopted.
- Web: base `origin/dev`, `npm run ci-check` once before opening the PR, and
  the protos tag pinned in both `package.json` and `package-lock.json`.
- Toolkit CI cannot resolve a sibling module's branch pseudo-version. So the
  resolution and session PRs build and walk locally on pseudo pins, and they
  go green only after repinning to tags.

**Inspected baseline.**

| Repo / module | Ref | Revision |
|---|---|---|
| rpg-toolkit | `origin/main` | `d1ff7e03` (root `v0.200.2`, encounter `v0.113.0`, resolution `v0.60.0`, session `v0.115.0`) |
| rpg-toolkit #1948 (in flight, root) | `origin/feat/effect-answers-batch1` | `e47348dc` (draft) |
| rpg-toolkit resolution R15 (in flight) | `feat/attack-roll-frame-resolution` | not yet pushed; assumptions A1–A3 |
| rpg-api-protos | `origin/main` | `f1a9756` (latest tag `v0.1.222`) |
| rpg-api | `origin/dev` | `10a52a3f` (pins session `v0.115.0`) |
| rpg-dnd5e-web | `origin/dev` | `2691f6d9` (pins protos `v0.1.221`) |

**Sequence.**
- *Development dependencies* (outside-in, each consumer pins its provider's
  pushed commit with `go get <module>@<sha>`):
  1. T1 protos.
  2. T7 web, against the T1 bindings; can start on a fixture.
  3. T6 api.
  4. T2 root, stacked on #1948.
  5. T3 encounter, from `origin/main`. Independent of T2.
  6. T4 resolution, on top of R15 resolution plus T2 and T3.
  7. T5 session, on T2, T3 and T4.
  8. T8 walk, on all of them.
- *Merge and release order* (inside-out):
  1. T1 merges.
  2. #1948 merges and tags root.
  3. T2 rebases onto main, merges, tags root.
  4. T3 merges and tags encounter. It may merge any time before T4.
  5. R15 resolution merges.
  6. T4 repins root and encounter tags, merges, tags resolution.
  7. T5 repins root, encounter and resolution tags, merges, tags session.
  8. T6 repins the session tag and `gen/go@generated`, then leaves draft.
  9. T7 adopts the protos tag.
  10. T8 re-walks on released pins.
- *Pairing.*
  - T2's handlers read `Frame.Held` and `PairFacts.Sees`, which only T4's
    builders fill. A Faerie Fire holder struck under new root with old
    resolution would answer Depends and fail the action (R13). So T2 and T4
    ship as a pair, and T5's go.mod pins both.
  - T3 and T5 also pair: an encounter that requires the conditions
    capability refuses a session that does not provide it.

**Open items.**
- *Resolved assumption: an unknown held list gives no rows.* A candidate whose
  sighting carries no conditions (legacy testimony from before T3, or a
  candidate that is current only through a non-sight channel) gets no held
  rows. No effect is known to exist there, so nothing is dropped. The wire
  cannot tell "unknown" from "holds nothing that bears". This happens only
  until the next sight refresh.
- *Resolved assumption: where the capability is injected.* The conditions
  capability is asserted on the `Equipment` value, the way `Participation` is
  asserted on `Standing` (`encounter.go` `NewEncounter`, `ErrNoParticipation`).
  It is not a new required field.
  - Its semantics are the same either way.
  - The cost differs a lot. A new field means editing every
    `NewEncounterInput`/`LoadInput` and `resolution.Input` literal: about 670
    test sites across the encounter, resolution and session modules. The
    assertion touches the 13 `Equipment` implementations and leaves
    `resolution.Input` alone.
  - The two remain separate interfaces.
- *Scope flag (decided, flag only): spell-attack Casts.* Held rows also ride
  spell-attack Cast candidates, because `session/effects.go`
  `informedAttackOf` serves both through one `InformAttack`. Execution runs
  both through the same strike (`resolution/action.go`: "The strike owns both
  Sanctuary checks for an attack cast"). Leaving Casts out would need a
  special case.
- *Flag (not a ruling): Dodging and sight.* The 5e text requires the dodger to
  see the attacker. `dodging.go` `onAttackChain` applies unconditionally. The
  rule mirrors the shipped behaviour. Adding sight is a gameplay change for
  the operator to rule on.
- *Flag: Sanctuary has two predicates.*
  - The strike path asks the new held rule, which applies to any attacker
    other than the holder.
  - The non-attack cast path (`action.go` `sanctuaryGate`) keeps
    `pendingSanctuaryWards`, gated on hostility.
  These are two predicates for one condition, answering different questions
  (an attack versus a harmful spell). The spell wave reconciles them. This
  wave does not touch the cast path.
- *Follow-up candidate.* Reckless Attack's target half is the same shape as
  Dodging (the target is the holder, so attackers get advantage). It is
  classified not-yet-answering here.
- *Explicit deferrals.* R12, R14, D2, third-party effects, catalogue
  descriptions.

---

### Task 1: Wire contract — `TargetCandidate.held_effects`

**Delivers:** H-WIRE (R18). A held effect is a full `EffectRow` on its
candidate.

**Owner:** rpg-api-protos, `dnd5e/api/session/v1alpha1/types.proto`.

**Prerequisites:** none.

**Files:** modify `types.proto` message `TargetCandidate`.

**Interfaces:**
- `TargetCandidate`: `repeated EffectRow held_effects = 5;`. Field 5 is free:
  fields 1–4 are `member`, `available`, `why`, `effects`.
- Doc comment carries the law:
  - These are effects *this target* holds that bear on the enclosing
    declaration's action, as full rows.
  - They are never overlaid onto, or joined with, the declaration's rows.
  - Each `id` is unique within this candidate and never equals a declaration
    `EffectRow.id`.
  - They never grant or refuse targeting.
  - The list is empty when the target holds nothing that bears, or when what
    it holds is not known.
  - Note "lands with rpg-project#520".
- No other change. `EffectRow` is reused as it stands (fields 1–8).

**Behavior:** contract only.

**Tests:** none bespoke. The repo rule is not to re-test generated mechanics.

- [ ] Add the field and its doc.
- [ ] `buf format -w`.
- [ ] `make test`.

**Verification:**
```
cd rpg-api-protos && git fetch origin main && make format && make test
```
- Lint, format-check, options and breaking all pass. An added field is not
  breaking.
- `git diff origin/main --stat` shows only `types.proto`.

**Completion evidence:** PR diff limited to `types.proto`, CI green. After
merge: `generated` updated and a tag cut (record it for T6 and T7).

---

### Task 2: Toolkit root — held facts, sight, by-reference rules, second census

**Delivers:** H-FRAME, H-SEES, H-RULE (R17), H-CENSUS, H-ROW (the ID rule),
H-EXEC (handler half), H-DEPENDS (R13 for target-held), H-DESC.

**Owner:** rpg-toolkit module `rulebooks/dnd5e`.

**Prerequisites:**
- #1948 head (`e47348dc` or later). Branch `feat/target-held-effects` from it,
  and rebase onto `origin/main` once #1948 merges.
- Assumptions A4 and A5.

**Files.**

Modify:
- `contributions/frame.go`. Add the held types, `Frame.Held`, `Frame.HeldBy`,
  `PairFacts.Sees`, validation, and a deep `Clone`.
- `contributions/effect.go`. Add `AttackMode` and `Answer.AttackMode`.
- `conditions/action_effects.go`. `validateAnswer` refuses an `AttackMode`
  on an answer that does not apply.
- `conditions/action_census.go`. Add the second classification and
  `TargetBearingRefs()`.
- `conditions/faerie_fire.go`. Rewrite `onAttackChain` to ask the held rule.
  Remove the `gamectx` and `math` imports.
- `conditions/guiding_bolt.go`. Rewrite `onAttackChain` to ask the held rule.
  `onRolled` (the consumption) is unchanged.
- `conditions/dodging.go`. Rewrite `onAttackChain`.
- `conditions/prone.go`.
  - `attackedWhileProne` asks the held rule, and `Answer.AttackMode` picks
    the list.
  - Delete `attackerIsWithinReach` and the `gamectx` import.
  - Rewrite the godoc that justified rolling straight on unknown placement.
- `conditions/hidden.go`. Rewrite the target branch of `onAttackChain`.
- `conditions/display.go`. Add `Detail` text (see Behavior).

Create (proposed):
- `conditions/target_held.go`: the rules, the registry, the fan-out and the
  exported execute.
- `conditions/target_held_test.go`.
- `conditions/target_census_test.go`.
- `contributions/held_test.go`.

**Interfaces (produced; T4 consumes).**

In `contributions`:
```go
// HeldCondition is one condition a member holds: its canonical condition ref
// and its source qualifier ("" when it has none).
type HeldCondition struct {
	Ref      string
	SourceID string
}

// MemberHeld lists what one member holds, in the order the producer reported.
// A member present with an empty list is KNOWN to hold nothing; a member
// absent from Frame.Held is unknown.
type MemberHeld struct {
	Member     string
	Conditions []HeldCondition
}

type Frame struct { /* existing fields */ ; Held []MemberHeld }

// HeldBy returns what member holds and whether that is known.
func (f Frame) HeldBy(member string) ([]HeldCondition, bool)

type PairFacts struct { /* existing fields */ ; Sees Fact[bool] } // From sees To

type AttackMode string
const (
	AttackAdvantage    AttackMode = "advantage"
	AttackDisadvantage AttackMode = "disadvantage"
)
type Answer struct { /* existing fields */ ; AttackMode AttackMode } // "" = none
```

Rules for these types:
- `Validate` adds:
  - every `MemberHeld.Member` non-empty and listed once;
  - every `Ref` parses with `core.ParseString`;
  - no repeated (`Ref`, `SourceID`) within one member.
- `Clone` copies `Held` and each inner `Conditions` slice.
- `AttackMode` is non-empty only on an answer that Applies.

In `conditions`:
```go
// AssessTargetHeldEffectsInput carries the frame; its Target must be known.
type AssessTargetHeldEffectsInput struct{ Frame contributions.Frame }
type AssessTargetHeldEffectsOutput struct{ Effects []contributions.Effect }
func AssessTargetHeldEffects(*AssessTargetHeldEffectsInput) (*AssessTargetHeldEffectsOutput, error)

// ExecuteHeldEffectInput names one held condition and the execution frame.
type ExecuteHeldEffectInput struct {
	Holder string
	Held   contributions.HeldCondition
	Frame  contributions.Frame
}
type ExecuteHeldEffectOutput struct{ Answer contributions.Answer }
// ExecuteHeldEffect asks the by-reference rule with execution semantics:
// invalid frame or Depends → error wrapping ErrRuleCannotAnswer; a ref with no
// held rule → error.
func ExecuteHeldEffect(*ExecuteHeldEffectInput) (*ExecuteHeldEffectOutput, error)

func TargetBearingRefs() []string // answering or not-yet-answering when held by the target
```

Registry and census:
- Unexported registry:
  `var targetHeldRules map[string]func(holder string, held contributions.HeldCondition) contributions.ActionAssessor`,
  keyed by condition ref string.
- Each handler builds its rule through the same constructor the registry
  points at. This is R17's "the loaded condition asks the same function".
- Second classification:
  `var targetCensus = map[string]actionCensusEntry{...}`, in
  `action_census.go`, using the existing `actionCensusEntry` type and classes.
  All its participations are `ContributesNow`.

**Behavior.**

*Shared gate.* Every held rule runs these checks in order, through one
unexported helper:
1. `frameOf` (an invalid frame is an error).
2. `HeldBy(holder)` unknown → Depends "Depends on what the target holds".
3. Not listing (`Ref`, `SourceID`) → DoesNotApply "The target does not hold
   this effect".
4. Roll is not attack → DoesNotApply "<Name> affects only attack rolls".
5. Target unknown → Depends "Depends on the target".
6. Target ≠ holder → DoesNotApply "<Name> affects only attacks against its
   holder".

*Rule predicates, after the gate.* Every Applies answer has participation
`ContributesNow`.

| Rule | Predicate | Answer |
|---|---|---|
| **Faerie Fire** | `Pair(Actor, holder).Sees` unknown | Depends "Depends on whether you can see the target" |
| | `Sees` known false | DoesNotApply "You cannot see the target" |
| | `Sees` known true | Applies "You can see the outlined target", benefit "Advantage on the attack roll", mode advantage |
| **Guiding Bolt** | — | Applies "The target is lit by Guiding Bolt", benefit "Advantage on the attack roll", mode advantage |
| **Dodging** | — | Applies "The target is dodging", benefit "Disadvantage on the attack roll", mode disadvantage |
| **Hidden** | — | Applies "The target is hidden", benefit "Disadvantage on the attack roll", mode disadvantage |
| **Prone** | `Pair(Actor, holder).DistanceCells` unknown | Depends "Depends on how far you are from the target" |
| | ≤ `combat.AdjacentCells` | Applies "The prone target is within 5 feet", benefit "Advantage on the attack roll", mode advantage |
| | otherwise | Applies "The prone target is beyond 5 feet", benefit "Disadvantage on the attack roll", mode disadvantage |
| **Sanctuary** | `Actor == holder` | DoesNotApply "Your own ward does not stop your attack" |
| | otherwise | Applies "The target is warded by Sanctuary", benefit "Wisdom saving throw first; on a failure the attack is lost", no mode |

*Target census.* The lists are the classification. The test enforces them
against `conditionLoaders`.
- **Answers (6).** FaerieFire, GuidingBolt, Dodging, Prone, Sanctuary, Hidden.
- **Not yet answering (9).**
  - RecklessAttack (target half).
  - Raging (resistance as defender).
  - BladeWard (`DamageChain`).
  - ShieldOfFaith, UnarmoredDefense, FightingStyleDefense (`combat.ACChain`).
  - `refs.Spells.Shield` (`PostAttackRollChain`).
  - InFog (D2).
- **Not bearing.** Every other loader key. That includes Unconscious: no
  handler in `conditions/unconscious.go` acts on an attack against its holder,
  so a row would claim what execution does not do.

*Fan-out: `AssessTargetHeldEffects`.*
1. Validate the frame. Error if `Target` is not known.
2. `HeldBy(target)` unknown → return zero rows with no error (the resolved
   assumption).
3. Otherwise walk the held list in frame order. Look each `Ref` up in
   `targetCensus`; an unclassified ref is an error.
   - Not bearing → skip.
   - Not yet answering → `StateUnavailable`, with the existing
     `unavailableReason` and description `DisplayFor(ref).Detail`.
   - Answers → `targetHeldRules[ref](target, held).AssessAction`, validated
     with `validateAnswer` and mapped with `effectState`.
4. A bearing ref with an empty `Detail` is an error.
5. There is no roll-group stacking: none of these is a
   `RollContributionProvider`.

*Row ID rule.*
- `ID = "target:" + Ref`, plus `"@" + SourceID` when `SourceID` is non-empty.
- `Source = {Ref: parsed ref, Name: DisplayFor(ref).Name, SourceID}`.
- A duplicate ID is an error.
- The `target:` prefix means a held ID can never equal a declaration row ID,
  which is `Ref` or `Ref@SourceID` of the actor's own condition. That holds
  even when actor and target both hold Prone.

*Execution handlers.*
- Each handler routes on `event.TargetID == holder`. That is routing to the
  target branch, as `prone.go` already switches, not a second predicate.
- Then it calls the unexported `executeRule` with its held rule and
  `event.Frame`:
  - Depends or invalid frame → return the error.
  - Applies → append to `AdvantageSources` or `DisadvantageSources` according
    to `Answer.AttackMode`, with the existing `SourceRef`, `SourceID` and
    `Reason` values.
  - DoesNotApply → no-op.
- The held condition each handler passes is its own address: FF and GB give
  `{Ref, SourceID}`, Dodging, Prone and Hidden give `{Ref, ""}`.
- Behaviour change, by ruling (R13): a prone target with no measurable pair,
  or a Faerie Fire holder whose sight is unknown, fails the strike. Today
  those cases roll straight or skip silently.

*Descriptions.* Add `Detail` to `display.go`:
- FaerieFire: "Attack rolls against you have advantage if the attacker can see you."
- GuidingBolt: "The next attack roll against you has advantage."
- Dodging: "Attack rolls against you have disadvantage, and you have advantage on Dexterity saving throws."
- ShieldOfFaith, BladeWard, UnarmoredDefense, FightingStyleDefense and
  `Spells.Shield`: authored from each handler's behaviour.
- Raging, Prone, Hidden, Sanctuary, RecklessAttack and InFog already have one.

**Tests** (testify suites):
- `contributions`:
  - `TestHeldByAbsentMemberIsUnknown`: `HeldBy("x")` gives `(nil, false)`;
    `{Member:"x", Conditions: []}` gives `([], true)`.
  - `TestFrameValidateRejectsMalformedHeld`: table of empty member, repeated
    member, empty ref, `"not a ref"`, repeated (ref, source). Each is an error.
  - `TestFrameCloneDetachesHeld`: mutate the clone's
    `Held[0].Conditions[0].SourceID`; the original is unchanged.
  - `TestPairSeesKnownFalseIsNotUnknown`.
- `conditions` census:
  - `TestEveryConditionLoaderIsClassifiedForTargetHeld`: `targetCensus` keys
    equal `conditionLoaders` keys, in both directions.
  - `TestTargetAnsweringRefsHaveHeldRules`: the answers-class keys of
    `targetCensus` equal the `targetHeldRules` keys.
  - `TestTargetBearingLoadersHaveDescriptions`: every `TargetBearingRefs()`
    entry has a non-empty `Detail`.
- Rules. The base frame `F` is `{Actor:"rogue", Target:Known("gob"),
  Roll attack, Held:[{gob, [FF@cleric]}], Pair rogue→gob {Distance 1,
  Sees Known(true)}}`.
  - `TestFaerieFireHeldRuleAppliesWhenActorSeesTarget`: Applies, mode
    advantage, benefit "Advantage on the attack roll".
  - `TestFaerieFireHeldRuleDependsWhenSightUnknown`.
  - `TestFaerieFireHeldRuleDoesNotApplyWhenActorCannotSee`.
  - `TestHeldRuleDoesNotApplyToAnotherTarget`: Target "orc".
  - `TestHeldRuleDependsWithoutTarget`.
  - `TestHeldRuleDependsWhenHoldingsUnknown`: `Held` is empty.
  - `TestHeldRuleDoesNotApplyWhenFrameDoesNotListIt`: gob holds `FF@other`.
  - `TestGuidingBoltDodgingHiddenHeldRules`: table of exact reason, benefit
    and mode.
  - `TestProneHeldRuleSplitsAtFiveFeet`: distance 1 gives advantage, 2 gives
    disadvantage, unknown gives Depends.
  - `TestSanctuaryHeldRuleAppliesToAnotherAttacker` and
    `TestSanctuaryHeldRuleDoesNotStopHoldersOwnAttack`.
  - `TestValidateAnswerRefusesModeWhenNotApplying`.
- Fan-out:
  - `TestAssessTargetHeldEffectsListsBearingHeldEffects`: gob holds
    `[FF@c1, Concentrating, BladeWard, Prone]`. Expect three rows in order:
    1. ID `"target:dnd5e:conditions:faerie_fire@c1"`, Applies.
    2. BladeWard, Unavailable with its description.
    3. Prone, Applies.
  - `TestAssessTargetHeldEffectsUnknownHoldingsYieldNoRows`.
  - `TestAssessTargetHeldEffectsRequiresKnownTarget`.
  - `TestAssessTargetHeldEffectsRefusesUnclassifiedRef`.
  - `TestTargetHeldIDsNeverEqualActorRowIDs`: actor and target both prone.
    The `AssessActionEffects` ID and the held ID differ.
  - `TestAssessingHeldEffectsSpendsNothing`: a Guiding Bolt condition's JSON
    is byte-identical before and after assessing.
- Handlers:
  - `TestFaerieFireHandlerReadsFrameSight`: bare `context.Background()`, no
    `gamectx`. `Sees` true adds an FF advantage source; `Sees` false adds
    none.
  - `TestFaerieFireHandlerFailsWhenSightUnknown`: `errors.Is(err,
    ErrRuleCannotAnswer)`.
  - `TestProneTargetHandlerReadsFrameDistance`: no room installed. Pair 1.0
    gives "Prone target within 5 feet" advantage; 3.0 gives disadvantage.
  - `TestProneTargetHandlerFailsWithoutDistance`.
  - `TestHeldHandlersRejectZeroFrame`: table over FF, GB, Dodging,
    Prone-target and Hidden-target.
  - `TestHeldHandlerSourcesImportNoGameContext`: parses `faerie_fire.go` and
    `prone.go` imports.
  - `TestHandlerAndRegistryAskTheSameHeldRule`: for each answering ref, the
    handler's rule constructor and `targetHeldRules[ref]` give deep-equal
    answers over the frames above.
  - `TestExecuteHeldEffectWrapsDepends` and
    `TestExecuteHeldEffectRefusesRefWithoutRule`.
- The existing FF, GB, Dodging, Prone, Hidden and Sanctuary suites are
  updated so attack-chain events carry a valid frame with `Held` and `Sees`.
  They keep their behavioural assertions.

Steps:
- [ ] Write the tests. Confirm the frame-reading and zero-frame handler tests
  fail on the #1948 head.
- [ ] Implement.
- [ ] Update existing suites.
- [ ] Update the `contributions/doc.go` package doc for held facts and sight.

**Verification:**
```
cd rpg-toolkit/rulebooks/dnd5e && go test -race ./... && golangci-lint run ./...
```
- Expect a pass.
- `grep -n "gamectx" conditions/faerie_fire.go conditions/prone.go` is empty.
- Push and record the SHA for T4 and T5.

**Completion evidence:** the named tests pass; the PR is stacked on #1948 (or
rebased after it); the pushed SHA is recorded.

---

### Task 3: Toolkit encounter — sightings carry conditions (R16)

**Delivers:** H-SIGHT (R16). Every sighting snapshots every condition on the
sighted member as testimony. No filter.

**Owner:** rpg-toolkit module `rulebooks/dnd5e/encounter`.

**Prerequisites:** none. Branch from `origin/main`.

**Files:**
- Create (proposed) `encounter/conditions.go`: the capability, `ConditionSet`,
  `SeenCondition`, `conditionsNow`, `ErrNoConditions`.
- Create `encounter/conditions_test.go`.
- Modify:
  - `encounter/testimony.go`: `SightTestimony.Conditions`, wire, encode,
    decode, `sightPayloadFields`.
  - `encounter/encounter.go`: `NewEncounter` assertion; `rebuildPercepts`
    pulls and encodes.
  - `encounter/data.go`: the Load path's equipment check gains the same
    assertion.
  - `encounter/observed_context.go`: `ObservedContextMember.Conditions`;
    rewrite the godoc line "It contains no target sheet, effect state…".
  - `encounter/doc.go`: correct the "expose target effects" sentence.
  - `encounter/errors.go`.
  - The eight `Equipment` implementations in this module, including
    `cmd/freeroam-workbench/main.go` `noHandsAreObserved`, each gain
    `Conditions`.

**Interfaces:**
```go
// SeenCondition is one condition a member was seen holding: canonical ref and
// source qualifier ("" when none). Bare strings; this module learns no ref.
type SeenCondition struct{ Ref, SourceID string }

// ConditionSet is what a member was seen holding. A nil *ConditionSet means
// there was nothing to observe; a non-nil empty set is "seen holding none".
type ConditionSet struct{ Conditions []SeenCondition }

// Conditions reports what each given member holds, as truth. Every member
// asked must appear and nobody else may; a nil value says nothing to observe.
type Conditions interface {
	Conditions(members []MemberID) (map[MemberID]*ConditionSet, error)
}

// EquipmentWithConditions is the Equipment capability that also answers
// Conditions — the shape Standing/Participation already uses.
type EquipmentWithConditions interface { Equipment; Conditions }

var ErrNoConditions = errors.New(...)

// SightTestimony gains:
Conditions *ConditionSet
// ObservedContextMember gains:
Conditions *ConditionSet // detached copy; nil = not observed
```
- Wire key: `"conditions"`, an array of `{"ref","source_id"}` encoded as
  `*[]conditionWire` with `omitempty`. A nil pointer omits the key; a non-nil
  empty pointer encodes `[]`.

**Behavior:**
- `NewEncounter` and Load refuse an `Equipment` that does not implement
  `Conditions`, with `ErrNoConditions`. The refusal is at the door, never
  defaulted, following the equipment godoc's reasoning.
- `conditionsNow`, like `equipmentNow`:
  - asks the sorted roster once per refresh, beside sight, equipment and
    standing;
  - refuses a stranger (`ErrNotMember`) or a skipped member
    (`ErrNoConditions`);
  - refuses an entry with an empty `Ref`, or a repeated (`Ref`, `SourceID`)
    within one member, with `ErrInvalidData`;
  - keeps each member's list in the order reported. It does not sort, because
    sheet order is persisted and deterministic.
- `rebuildPercepts` writes `Conditions: sets[subjectID]` into each placed
  member's one encoded payload. It stays one encode per member.
- Encode:
  - Known testimony carries `Conditions` as given.
  - Unknown testimony carrying conditions is refused ("unknown location
    cannot carry conditions").
- Decode:
  - Legacy untagged testimony carrying `conditions` is refused.
  - Tagged testimony without the key decodes to nil (not observed).
  - Entries are validated the same way as on encode.
- `ObservedContext` copies `seen.Conditions` into each member, detached. It
  still never consults a live capability.

**Tests:**
- `TestSightTestimonyConditionsRoundTrip`: nil, empty and two-entry sets.
  - nil omits the key;
  - empty encodes `[]`;
  - order is preserved.
- `TestSightTestimonyRefusesConditionsOnUnknownAndLegacy`.
- `TestDecodeRefusesMalformedConditions`: empty ref, repeated entry.
- `TestNewEncounterRefusesEquipmentWithoutConditions` and the same for Load.
- `TestConditionsNowRefusesStrangerAndSkippedMember`.
- `TestSightingSnapshotsConditions`. A capability reports goblin
  `[{faerie_fire, cleric}]`. After a refresh, the fighter's
  `ObservedContext` member goblin has exactly that set.
- `TestSightingIsTestimonyNotALiveRead`. Change the capability's answer
  without a refresh: `ObservedContext` still shows the old set. After
  `Recheck{goblin}` it shows the new one.
- `TestConditionsAskedOncePerRefresh`: a counting capability is called once
  per refresh, whatever the observer count.
- The existing `encodeSighting` count test stays at N per pass.

**Verification:**
```
cd rpg-toolkit/rulebooks/dnd5e/encounter && go test -race ./... && golangci-lint run ./...
```
Push and record the SHA.

**Completion evidence:** the named tests pass; the PR is open; the
observed-context godoc no longer says it carries no effect state.

---

### Task 4: Toolkit resolution — frames carry held conditions and sight; per-target held rows; Sanctuary asks the rule

**Delivers:** H-FRAME (both builders), H-SEES (both builders), H-INFORM,
H-EXEC (Sanctuary and frame), H-K3 (testimony, not a live read).

**Owner:** rpg-toolkit module `rulebooks/dnd5e/resolution`.

**Prerequisites:**
- R15 resolution branch `feat/attack-roll-frame-resolution` (assumptions
  A1–A3).
- T2 SHA: `go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@<T2sha>`.
- T3 SHA: `go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter@<T3sha>`.

**Files:**
- Modify:
  - `resolution/frame.go`: both builders.
  - `resolution/inform.go`.
  - `resolution/strike.go`: `sanctuaryStep`.
  - `resolution/sanctuary.go`.
  - The one `Equipment` test fake in this module, which gains `Conditions`.
  - `go.mod` and `go.sum`.
- Create (proposed) `resolution/held_frame_test.go`.

**Interfaces:**
- Information frame (`informationFrame`):
  - `informationFrameInput` gains `ActorHeld []contributions.HeldCondition`.
  - `Frame.Held` gets one entry for the actor (`ActorHeld`, always known)
    plus one per `Observed.Members[i]` whose `Conditions != nil`. Each entry
    maps `SeenCondition{Ref, SourceID}` to `HeldCondition` in order.
  - A member with nil `Conditions` is absent from `Held`, which means
    unknown.
  - `PairFacts.Sees` is `Known(true)` for every pair with `From == Observer`
    and `To` a member of `Observed.Members`, because each one is a current
    sight sighting by that type's contract. Every other pair has `Sees`
    unknown.
- Execution frame (the R15 base-frame builder, memoized per strike machine):
  - `Frame.Held` gets one entry per `m.cast` member with known identity, from
    `heldConditions(m.cast, id)` mapped through `conditions.ConditionAddressOf`
    (`ConditionRef`, `SourceID`), in persisted order.
  - For each placed ordered pair, `PairFacts.Sees` comes from
    `gamectx.Visibility(ctx).SeesWithin(from, to, math.MaxInt)`:
    - known and visible → `Known(true)`;
    - known and not visible → `Known(false)`;
    - not known → unknown.
  - Visibility is always installed by `truth.go` `installTruth`, so no
    provider is an `ErrBadWorld` error.
  - `Held` and `Sees` live in the one base derivation, so the attack-roll and
    damage frames carry identical held and sight facts (R15).
- `InformAttackInput` is unchanged.
- `InformAttackOutput` gains
  `HeldByTarget map[string][]contributions.Effect`. It is non-nil with one
  entry per target, and each entry is the `conditions.AssessTargetHeldEffects`
  output over that target's information frame (the same frame `ByTarget`
  used).
- `pendingSanctuaryWards` stays for the cast path.
- New `strikeWards(frame contributions.Frame, cast *Participants, attackerID,
  targetID string) ([]*conditions.SanctuaryCondition, error)`. It returns the
  target's wards whose `conditions.ExecuteHeldEffect` answer Applies, and
  propagates any error.

**Behavior:**
- `sanctuaryStep` builds the base frame on first use, after
  `endSanctuaryIfHeld` and before ward selection. Then it calls
  `strikeWards`. An `ErrRuleCannotAnswer` fails the strike.
- The attack chain and the damage fold reuse the same memoized frame (A1).
  A resumed strike rebuilds it from current truth (S3, unchanged).
- `InformAttack`:
  - passes `ActorHeld` from `in.Actor.GetConditions()` via
    `ConditionAddressOf`;
  - reads no target sheet: `Held` for others comes only from `Observed`.
  - K4's field-set test stays unchanged.

**Tests:**
- `TestInformationFrameHeldFromSightingsOnly`:
  - observed member A has a set, so it is listed;
  - member B has nil, so it is absent;
  - the actor is listed from its own sheet.
- `TestInformationFrameSeesOnlyObserverToSighted`:
  - observer→A is `Known(true)`;
  - A→observer and A→B are unknown.
- `TestInformAttackHeldRowsPerTarget`. Real encounter: rogue R; goblin G1
  holding FF@cleric (through the conditions capability); goblin G2 holding
  nothing; both sighted.
  - `HeldByTarget[G1]` is one row, ID
    `"target:dnd5e:conditions:faerie_fire@cleric"`, Applies, benefit
    "Advantage on the attack roll".
  - `HeldByTarget[G2]` is empty.
  - `Effects` contains no FF row.
- `TestHeldRowsAreTestimony` (K3, R16). After the sight pass, add Dodging to
  G1's truth without a refresh: `HeldByTarget[G1]` is unchanged.
- `TestStrikeFrameCarriesHeldAndSight`. A spy attack-chain subscriber
  records `event.Frame`:
  - `HeldBy(G1)` lists FF@cleric;
  - `Pair(R, G1).Sees` is `Known(true)`;
  - the damage frame deep-equals it except `Advantage`.
- `TestStrikeFaerieFireAdvantageFromFrame`: the folded `AdvantageSources`
  contains the FF source.
- `TestStrikeProneTargetBeyondFiveFeetHasDisadvantage`: a ranged attack from
  3 cells.
- `TestStrikeSanctuaryWardsComeFromTheHeldRule`: a warded target gives a ward
  save request (existing shape); with the attacker's own ward, no request.
- `TestStrikeFailsWhenAHeldRuleCannotAnswer`. A target holds FF and
  `SeesWithin` is unknown (a run-less visibility fake): `errors.Is(err,
  ErrRuleCannotAnswer)`, and target HP is unchanged.
- The existing strike, sanctuary, inform and resume suites pass.

**Verification:**
```
cd rpg-toolkit/rulebooks/dnd5e/resolution && go test -race ./... && golangci-lint run ./...
```
- Expect a pass.
- go.mod shows the T2 and T3 pseudo-versions on top of the R15 branch.
- Push and record the SHA.

**Completion evidence:** the named tests pass; the SHA is recorded.

---

### Task 5: Toolkit session — candidates carry held rows; the seam answers conditions; condition changes re-look (D1)

**Delivers:** H-ATTACH (R18), H-SEAM (R16's source), H-FRESH (D1), H-WIRE
(session half).

**Owner:** rpg-toolkit module `rulebooks/dnd5e/session`.

**Prerequisites:**
- T2, T3 and T4 SHAs pinned.
- D1 ruled, for the freshness step only.

**Files:**
- Modify:
  - `session/types.go` (`TargetCandidate`).
  - `session/effects.go` (`attachEffects`).
  - `session/equipment.go`: `equipmentSeam` gains `Conditions`; a doc line.
  - `session/write.go`: `writeScope` captures conditions at open; `commit`
    rechecks.
  - `session/cmd/session-workbench/main.go` `encNoHandsObserved`.
  - The four `Equipment` implementations in this module.
  - `session/wirecontract_test.go`.
  - `session/README.md` and `session/AGENTS.md`, where they enumerate
    `TargetCandidate`.
- Create (proposed) `session/conditions_seen.go` and
  `session/conditions_seen_test.go`.
- Extend `session/effects_test.go` (`EffectRowsSuite`).

**Interfaces:**
- `TargetCandidate.HeldEffects []EffectRow` with
  `json:"held_effects,omitempty"`.
- `func (s equipmentSeam) Conditions(members []encounter.MemberID)
  (map[encounter.MemberID]*encounter.ConditionSet, error)`, satisfying
  `encounter.EquipmentWithConditions` (compile-time assertion):
  - `KindWorld` → nil.
  - `KindPlayer` with a sheet → each `data.Conditions` raw entry goes through
    `conditions.LoadJSON` then `conditions.ConditionAddressOf(name, c)`,
    giving `{ConditionRef, SourceID}` in persisted order. An empty sheet gives
    a non-nil empty set.
  - Player not found → nil.
  - `KindMonster` with `npcSheet` → the same, over `monster.Data.Conditions`.
  - No sheet → nil.
  - A load error fails the verb (R5).
- `writeScope` gains `conditionsAtOpen map[string]string`: member → a
  canonical key of its sorted addresses. It is filled in `openForChange` from
  the same seam.

**Behavior:**
- `attachEffects` sets
  `candidates[j].HeldEffects = effectRowOf(...)` for each entry of
  `informed.HeldByTarget[member]`.
  - It is set only on the candidate.
  - Declaration rows stay the actor's own.
  - An effect no candidate holds appears nowhere.
- Afford's early returns (off-turn, frozen, world clock) and blockers get no
  held rows, because attach does not run for them.
- **Freshness (D1).** In `commit`, after `settleExperience` and before
  `buildStreamNumbers`:
  1. Ask the seam again.
  2. Collect the members whose key differs from `conditionsAtOpen`, in
     sorted order.
  3. If any, and the encounter is not closed, call
     `scope.enc.Recheck(&encounter.RecheckInput{Members: changed})`.
  The resulting beats are numbered and delivered with this act.

**Tests** (`EffectRowsSuite` fixtures from `afford_test.go` `aFight`, plus
the condition-seating pattern in `clockboundary_test.go`):
- `TestCandidateCarriesFaerieFireHeldRow`:
  - goblin G1 seated with FF from the cleric;
  - the rogue's Attack candidate G1 has `HeldEffects == [{ID
    "target:dnd5e:conditions:faerie_fire@cleric", Name "Faerie Fire", State
    applies, Benefit "Advantage on the attack roll", Description == catalog}]`;
  - G2's `HeldEffects` is empty;
  - the declaration's `Effects` has no Faerie Fire row.
- `TestHeldRowIDsDistinctFromDeclarationRows`: a prone rogue attacks a prone
  goblin.
- `TestNoHeldRowsOffTurnOrWhileFrozen`.
- `TestConditionSeamReportsSheets`: a player set in order; a monster set; a
  world member nil; a missing player nil; a stranger in the question is
  answered only for those asked.
- `TestConditionChangeRechecksWatchers` (D1):
  1. The cleric casts Faerie Fire on G1, through the verb.
  2. With no movement, the rogue's next Afford candidate G1 carries the FF
     held row.
  3. The fighter's stream has a sighting "changed" beat naming G1.
- `TestGuidingBoltRowLeavesWhenSpent`: the fighter attacks G1 under Guiding
  Bolt. The rogue's next Afford G1 has no Guiding Bolt row.
- `TestUnchangedConditionsDoNotRecheck`: a Move verb with no condition change
  adds no "changed" beat.
- `TestEffectRowWireKeys` is extended to `held_effects`.
- `TestNoInnerTypeCrossesTheBoundary` still passes.

**Verification:**
```
cd rpg-toolkit/rulebooks/dnd5e/session && go test -race ./... && golangci-lint run ./...
```
Push and record the SHA.

**Completion evidence:** the named tests pass; go.mod pins the T2, T3 and T4
pseudo-versions.

---

### Task 6: rpg-api mapping

**Delivers:** H-WIRE across the boundary (transport only).

**Owner:** rpg-api, `internal/handlers/dnd5e/session/v1alpha1/convert.go`.

**Prerequisites:**
- T1 merged:
  `GOPROXY=direct go get github.com/KirkDiggler/rpg-api-protos/gen/go@generated`.
- T5 SHA pinned (`go get .../session@<T5sha>`).

**Files:** modify `convert.go` `targetCandidateToProto`, `convert_test.go`,
`go.mod` and `go.sum`.

**Interfaces:** `HeldEffects: effectRowsToProto(c.HeldEffects)`, reusing the
existing converter.

**Behavior:** a field-for-field copy. No join, filter or reorder.

**Tests:**
- `TestTargetCandidateToProto_CarriesHeldEffects`: every field of two rows.
- Extend `TestTargetCandidateToProto_FieldForField`.

**Verification:** `cd rpg-api && make ci-check`. The PR stays draft until the
toolkit pins are tags.

**Completion evidence:** tests pass; the draft PR has a pin-adoption
checklist.

---

### Task 7: rpg-dnd5e-web rendering

**Delivers:** H-RENDER. The candidate inspection shows that target's held
rows, verbatim.

**Owner:** rpg-dnd5e-web, `src/components/session/combat-experience/`.

**Prerequisites:** the T1 tag (`npm i --save
github:KirkDiggler/rpg-api-protos#<tag>`, then commit `package.json` and
`package-lock.json`).

**Files:**
- Modify `actionTooltip.ts` `effectLinesFor`. With a candidate, append that
  candidate's `heldEffects` after the overlaid declaration rows, each mapped
  through the same tone and state-word path.
- `TargetSurface.tsx` already renders
  `effectLinesFor(declaration, inspectedMember)`, so it needs no change.
- Tests in `actionTooltip.test.ts` and `TargetSurfaceEffects.test.tsx`.

**Interfaces:** consume `TargetCandidate.heldEffects` (`EffectRow[]`) from
`types_pb`.

**Behavior:**
- With no candidate, there are no held rows.
- With a candidate, the declaration rows (overlaid) come first, then held
  rows, with names, reasons, descriptions and benefits verbatim.
- Clicking a target is unchanged.

**Tests:**
- `actionTooltip.test.ts`:
  - `appends the candidate's held rows after the declaration rows`;
  - `held rows are absent without a candidate`;
  - `held rows keep their own ids`.
- `TargetSurfaceEffects.test.tsx`: `candidate hover shows that target's held
  rows`.

**Verification:**
- While iterating:
  `npm run test:run -- src/components/session/combat-experience`.
- Once before the PR: `npm run ci-check`, with its output in the PR body.

**Completion evidence:** tests pass; ci-check output is in the PR.

---

### Task 8: Local-stack walk (integration)

**Delivers:** H-WALK. One click per seam: testimony, held rows, the sight
fact, freshness, and the execution agreeing.

**Owner:** rpg-project#520 holds the evidence. The walk runs T5's pins
through T6's image and T7's dev server.

**Prerequisites:** T1–T7 pushed, and D1 ruled. Follow
`rpg-project/docs/howto/run-the-game-locally.md`. Use a level-1+ Cleric with
Faerie Fire and Guiding Bolt prepared, plus a Rogue and a Fighter, against two
goblins.

**Scenario:**
1. On the Cleric's turn, cast Faerie Fire on goblin A, then end the turn.
2. On the Rogue's turn, with nobody moving:
   - hover the Attack offer: no Faerie Fire row;
   - hover goblin A: "Faerie Fire — Applies — Advantage on the attack roll";
   - goblin B has none;
   - attack A: the roll shows advantage, sourced to Faerie Fire.
3. The Cleric casts Guiding Bolt on goblin B. It hits.
4. On the Fighter's turn, hover goblin B: Guiding Bolt applies. Attack B. Then
   hover B again: the Guiding Bolt row is gone.
5. Off-turn, or with a reaction window open: no held rows anywhere.

**Verification:**
- Afford JSON excerpts showing `candidates[].held_effects`.
- Screenshots per step.
- The SHAs of every repo walked.

**Completion evidence:**
- A #520 comment with the SHAs, screenshots, JSON and pass/fail per step.
- Then the release check: re-run steps 2 and 4 after T2–T7 adopt released
  tags.

---

## Assumptions about the in-flight frame work

Each one is a seam check. Re-verify it against the pushed branch before the
dependent task starts.

| # | Assumption | Inspected evidence | Dependent task | Re-verify by |
|---|---|---|---|---|
| A1 | Resolution builds one base frame per strike machine (Advantage unknown), hands it to `AttackChainEvent.Frame` before the attack-chain fold, and derives the damage and offer frame as that frame with `Advantage` known. | `events.go` `AttackChainEvent.Frame` godoc on #1948 `e47348dc` says exactly this; shipped `resolution/frame.go` `executionFrame` memoizes `m.frame` per machine | T4 | Read `feat/attack-roll-frame-resolution` `strike.go` and `frame.go`: one builder, one memo |
| A2 | Every non-advantage fact comes from that one derivation, so adding `Held` and `Sees` there puts them identically in both moments. | Law: "every other fact is the same in both, because both come from one derivation" | T4 | `TestStrikeFrameCarriesHeldAndSight` deep-equal |
| A3 | The base frame needs only the preflighted attack profile, the installed room, cast and visibility, and the cast participants. So it can be built at `sanctuaryStep`, before `effectiveACStep`. | Shipped `executionFrame` reads `m.attack`, `gamectx.RequireRoom`, `gamectx.CastOf`; `m.cast` is set in `Start` | T4 | If the R15 builder reads the effective AC or the folded event, split the builder so the fold-independent part is built first, and record the change |
| A4 | T2's `contributions/frame.go` edits (`Held`, `Sees`, `Validate` additions) meet #1948's edits (weapon facts in `ActionFacts`, the weapon ref check in `Validate`) only in `Validate`. | #1948 diff of `frame.go` | T2 | Rebase onto #1948's final head; conflicts limited to `Validate` |
| A5 | #1948 keeps `executeRule`, `frameOf`, `attackRollRule`/`assessed`, and its holder-side census answers for Prone and Hidden. T2 changes only the target branches of `prone.go` and `hidden.go`. | `e47348dc` `attack_roll_rule.go`, `prone.go` `attackingWhileProne`, `hidden.go` attacker branch | T2 | Re-read those files at #1948's merge |

## Requirement coverage

| In-scope requirement (law line / ruling) | Task(s) | Concrete proof |
|---|---|---|
| H-SIGHT: a sighting carries every condition on the sighted member, as snapshot testimony, with no filter (R16) | T3, T5 | `TestSightingSnapshotsConditions`; `TestSightingIsTestimonyNotALiveRead`; `TestConditionSeamReportsSheets` |
| H-K3: hidden state cannot change an information answer; never a live read | T3, T4 | `TestHeldRowsAreTestimony`; `TestSightingIsTestimonyNotALiveRead` |
| K4: information never reads target sheets | T4 | `InformAttackInput` unchanged; the existing field-set test; `TestInformationFrameHeldFromSightingsOnly` |
| H-FRAME: the frame carries each member's held conditions (ref + source), known or unknown | T2, T4 | `TestHeldByAbsentMemberIsUnknown`; `TestFrameValidateRejectsMalformedHeld`; `TestInformationFrameHeldFromSightingsOnly`; `TestStrikeFrameCarriesHeldAndSight` |
| Typed facts: unknown never false; known false stays known | T2, T4 | `TestPairSeesKnownFalseIsNotUnknown`; `TestFaerieFireHeldRuleDependsWhenSightUnknown` vs `…DoesNotApplyWhenActorCannotSee` |
| H-SEES: a "sees" fact on the frame pair, for Faerie Fire | T2, T4 | `TestInformationFrameSeesOnlyObserverToSighted`; `TestFaerieFireHandlerReadsFrameSight`; `TestStrikeFaerieFireAdvantageFromFrame` |
| H-RULE: a target-held effect answers through a rule keyed by reference, reading only the frame; the loaded condition asks the same function (R17) | T2 | `TestHandlerAndRegistryAskTheSameHeldRule`; `TestHeldHandlerSourcesImportNoGameContext`; `TestProneTargetHandlerReadsFrameDistance` |
| A rule reads nothing but the frame; resolution builds every frame | T2, T4 | FF and Prone lose `gamectx`; execution `Held`/`Sees` built in resolution (`TestStrikeFrameCarriesHeldAndSight`) |
| Execution and information call the same rule; no second predicate | T2, T4 | `TestHandlerAndRegistryAskTheSameHeldRule`; `TestStrikeSanctuaryWardsComeFromTheHeldRule` (strike path; cast path flagged) |
| H-DEPENDS: at execution, Depends fails the action (R13) | T2, T4 | `TestFaerieFireHandlerFailsWhenSightUnknown`; `TestProneTargetHandlerFailsWithoutDistance`; `TestStrikeFailsWhenAHeldRuleCannotAnswer`; `TestHeldHandlersRejectZeroFrame` |
| H-CENSUS: every loader classified for target-held (answers / unavailable / no row) | T2 | `TestEveryConditionLoaderIsClassifiedForTargetHeld`; `TestTargetAnsweringRefsHaveHeldRules`; `TestAssessTargetHeldEffectsListsBearingHeldEffects` |
| Three states; an unanswerable rule is unavailable, never dropped (R7) | T2, T7 | BladeWard unavailable row in the fan-out test; web renders all states (existing `EffectRows` test) |
| H-DESC: descriptions are content beside the rule (R11) | T2 | `TestTargetBearingLoadersHaveDescriptions`; session asserts Description == catalog |
| H-ROW: a held row is a full row on its candidate; declaration rows stay the actor's own; an effect no candidate holds appears nowhere (R18) | T1, T4, T5 | `TestCandidateCarriesFaerieFireHeldRow` (G2 empty, no FF on the declaration); `TestInformAttackHeldRowsPerTarget` |
| Row ID uniqueness across declaration and candidate | T2, T5 | `TestTargetHeldIDsNeverEqualActorRowIDs`; `TestHeldRowIDsDistinctFromDeclarationRows` |
| Rows not delivered off-turn or frozen (R12 behaviour kept) | T5, T8 | `TestNoHeldRowsOffTurnOrWhileFrozen`; walk step 5 |
| Reading changes nothing; lifecycle untouched (R9) | T2 | `TestAssessingHeldEffectsSpendsNothing`; Guiding Bolt's `onRolled` unchanged |
| No prediction, totals or folding (R1, R3) | T2, T7 | `Answer` holds no total; `AttackMode` is a per-rule contribution, not a fold; web renders verbatim |
| Toolkit owns, API transports, web renders and names no effect | T6, T7 | `TestTargetCandidateToProto_CarriesHeldEffects`; web held-row tests render verbatim |
| The wire names no feature or variant | T1 | Reuses the generic `EffectRow` |
| H-FRESH: testimony refreshes when a condition changes (D1) | T5, T8 | `TestConditionChangeRechecksWatchers`; `TestGuidingBoltRowLeavesWhenSpent`; walk steps 2 and 4 |
| H-WALK | T8 | Walk evidence on #520 |
| Deferred, not covered: third-party effects, save/check rows, R12, R14, the unseen-combatant row (D2), Reckless target half, Dodging sight, Sanctuary cast-path predicate | — | Brief non-goals and the flags above |

## Provider/consumer seams

| Provider | Consumer | Produced vs consumed contract | Availability | Proof |
|---|---|---|---|---|
| T1 `TargetCandidate.held_effects = 5` (`EffectRow`) | T6 converter | `HeldEffects []*sessionpb.EffectRow` | T1 merged → `gen/go@generated` | `TestTargetCandidateToProto_CarriesHeldEffects` compiles against generated code |
| T1 | T7 web | TS `TargetCandidate.heldEffects: EffectRow[]` | T1 tag in `package.json` | Web tests type-check under `ci-check` |
| T3 `encounter.Conditions` / `EquipmentWithConditions` | T5 `equipmentSeam.Conditions` | Members in; `map[MemberID]*ConditionSet`; nil = nothing to observe; every asked member present | T3 SHA pinned in session (and resolution, which builds encounters through `resolution.Input.Equipment`, unchanged) | Compile-time assertion in `session/equipment.go`; `TestNewEncounterRefusesEquipmentWithoutConditions` |
| T3 `ObservedContextMember.Conditions *ConditionSet` | T4 `informationFrame` | nil → absent from `Held` (unknown); non-nil → listed in order; `SeenCondition{Ref, SourceID}` → `HeldCondition{Ref, SourceID}` | T3 SHA pinned in resolution | `TestInformationFrameHeldFromSightingsOnly` |
| Existing `ObservedContext` contract: Members are current sight sightings (`observed_context.go`) | T4 `Sees` derivation | observer→member is `Known(true)`; everything else unknown | encounter ≥ T3 | `TestInformationFrameSeesOnlyObserverToSighted` |
| T2 `contributions.Frame.Held`, `HeldBy`, `PairFacts.Sees`, `HeldCondition` | T4 builders | Absent member = unknown; empty list = known none; `Sees` is From-sees-To | T2 SHA pinned | `TestStrikeFrameCarriesHeldAndSight` |
| Existing `gamectx.Visibility` (`gamectx/room.go`, installed by `resolution/truth.go` `installTruth`) | T4 execution `Sees` | `SeesWithin(from, to, MaxInt) (visible, known)`; not known → unknown | Shipped | `TestStrikeFailsWhenAHeldRuleCannotAnswer` (unknown sight) |
| Existing `conditions.ConditionAddressOf` (`baned.go`) | T4 execution `Held`; T5 seam | `{ConditionRef, SourceID}`; SourceID "" for unaddressed conditions | Shipped | `TestStrikeFrameCarriesHeldAndSight`; `TestConditionSeamReportsSheets` |
| T2 `conditions.AssessTargetHeldEffects` | T4 `InformAttack` | Input frame with a known Target; output rows in held order; IDs `target:` prefixed; unknown holdings give zero rows | T2 SHA | `TestInformAttackHeldRowsPerTarget` |
| T2 `conditions.ExecuteHeldEffect` | T4 `strikeWards` | Execution semantics: Depends or invalid → `ErrRuleCannotAnswer`; Applies selects the ward | T2 SHA | `TestStrikeSanctuaryWardsComeFromTheHeldRule` |
| T4 execution frame (`Held`, `Sees` filled) | T2 handlers (FF, GB, Dodging, Prone-target, Hidden-target) | A valid frame listing the holder's own address; `Sees` known when visibility answers | T2 and T4 paired release | `TestStrikeFaerieFireAdvantageFromFrame`; `TestStrikeProneTargetBeyondFiveFeetHasDisadvantage` |
| R15 in-flight base frame (A1–A3) | T4 | One memoized derivation, before the attack chain; damage = same + Advantage | R15 branch | A1–A3 re-verify column |
| T4 `InformAttackOutput.HeldByTarget` | T5 `attachEffects` | One entry per target, non-nil; rows map through the existing `effectRowOf` | T4 SHA | `TestCandidateCarriesFaerieFireHeldRow` |
| T5 `TargetCandidate.HeldEffects` (`held_effects`) | T6 | Session `EffectRow` → proto `EffectRow` field for field | T5 SHA | `TestTargetCandidateToProto_CarriesHeldEffects` |
| Existing `encounter.Recheck` (`sightedbeat.go`) | T5 commit freshness | Names who; refuses a closed encounter or a stranger; whole-roster re-look; "changed" beats per watcher | Shipped | `TestConditionChangeRechecksWatchers` |

**Shared files and ordering.**
- T2 and #1948 share `contributions/frame.go`, `prone.go` and `hidden.go`.
  T2 stacks on #1948 and rebases after it merges (A4, A5).
- T3 and T4 do not share files.
- T4 and the R15 branch share `resolution/frame.go` and `strike.go`. T4
  stacks on R15.
- Within T5, the `write.go` freshness step is the only D1-dependent change.
  It is a separate commit so the rest can land if D1 changes.

**Check findings.**
- Every consumed capability is either an inspected existing symbol
  (`ObservedContext`, `Recheck`, `Visibility`, `ConditionAddressOf`,
  `LoadJSON`, `heldConditions`, `effectRowOf`, `effectRowsToProto`,
  `effectLinesFor`) or produced by a named task.
- `AttackMode` is required by Prone's two-way answer (R2: the contribution
  as data). Without it the handler would need its own distance predicate.
- The T2 and T4 pairing risk is recorded in Sequence.
- Each handler's prescribed behavior agrees with its test: the zero frame,
  unknown sight and missing distance all error.
- The plan has no placeholders. D1 blocks only T5's freshness commit and T8.

## Current code that contradicts a ruling

These are evidence for the case file. Each one is fixed by the task named.

1. `conditions/faerie_fire.go` `onAttackChain` reads `gamectx.Visibility`
   itself, and silently grants nothing when no provider is installed.
   - Contradicts "a rule reads nothing but the frame", "resolution builds
     every frame" and R13.
   - Fixed by T2 and T4.
2. `conditions/prone.go` `attackedWhileProne` and `attackerIsWithinReach` read
   `gamectx.Room`, and roll straight when placement is unknown.
   - Contradicts the same law lines and R13.
   - Fixed by T2 and T4.
3. Guiding Bolt, Dodging, and the target branches of Hidden and Reckless
   Attack keep their predicate only inside the handler. Information has no
   rule to ask, which contradicts R17.
   - Fixed by T2 for all but Reckless, which is flagged.
4. `encounter/observed_context.go` (`ObservedContextOutput` godoc: "contains
   no target sheet, effect state…") and the sight testimony carry no
   conditions. This contradicts R16.
   - Fixed by T3.
5. Sanctuary has two predicates: the strike path in `strike.go` uses
   `pendingSanctuaryWards`, and the cast path in `action.go` `sanctuaryGate`
   is hostility-gated. T4 moves the strike path onto the held rule. The cast
   path is flagged for the spell wave.
