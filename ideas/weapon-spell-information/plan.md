# Plan: effect information, first delivery (rpg-project#520)

## Header

**Goal and scope.** Each Attack declaration, and each Cast declaration that makes a spell attack, carries the effects bearing on it. Every effect shows as APPLIES, DOES_NOT_APPLY (with the rule's reason), DEPENDS, or UNAVAILABLE, and each row has its description. Answers that change with the target ride each `TargetCandidate`. The first delivery covers the four effects in the #520 walk:
- Sneak Attack (depends on the target)
- Rage
- Bless
- Bardic Inspiration (as a later choice)

Bane answers too, because it uses the same roll-contribution seam as Bless at no extra cost (flagged, see Open items). Every other condition loader is classified once in a census. Execution handlers for those effects ask the same rule against a frame built by resolution, and fail the action when the rule answers DEPENDS.

**Non-goals.**
- No totals, folding, prediction or inspection RPC.
- No rows off-turn or during a frozen window (R12).
- No resolution-assembled contributions (R14).
- Not fixing toolkit#1929 (Sneak Attack's `"dex"` test is kept as is) or toolkit#1934 (Rage on raw Strength checks).
- No catalogue descriptions (shipped separately in toolkit#1927).
- No rows on Activate, Move, EndTurn, DeathSave, Intimidate, Persuade, or non-attack Casts.
- No rows for effects carried by the target (a Guiding Bolt mark, Faerie Fire, a prone or dodging target). Rows come from the acting character's own loaded effects; what the character may know of a target's effects has no source yet.

**Authority.** `rpg-project` `origin/design/effect-information-rewrite:ideas/weapon-spell-information/design.md` @ `106794a` (law plus R1–R14, "Open: None"), and issue rpg-project#520. This plan lives beside the design as `ideas/weapon-spell-information/plan.md`, linked from #520.

**Constraints (only the binding ones).**
- Toolkit: one nearest-go.mod module per PR. Root `rulebooks/dnd5e`, `encounter`, `resolution` and `session` are separate PRs.
- Session S2: `TestNoInnerTypeCrossesTheBoundary` (`session/boundary_test.go`) means new session wire types hold only strings and string enums.
- Protos: authored contract only, `make test`, no new `.proto` file (avoids the new-file options policy). Protos merge first so CI publishes bindings.
- rpg-api: base `origin/dev`, `make ci-check`. Its `release-pin-check` keeps a pseudo-pinned PR in draft until real tags are adopted.
- Web: base `origin/dev`, `npm run ci-check` once before opening the PR, protos pinned by tag in `package.json` and `package-lock.json`.
- Develop against pushed pseudo-versions. Merge inside-out and adopt real tags before each consumer merges.

**Inspected baseline.**

| Repo / module | Ref | SHA |
|---|---|---|
| rpg-toolkit | `origin/main` | `82aeb15e` |
| rpg-toolkit #1932 | `origin/feat/effect-information-content` | `1f73c0c8` |
| rpg-api-protos | `origin/main` | `11372b02` |
| rpg-api-protos #374 | `origin/feat/effect-information` | `72d18c9f` |
| rpg-api | `origin/dev` | `858c3e5a` |
| rpg-dnd5e-web | `origin/dev` | `af4ee892` |

Toolkit tags at that point:
- root `v0.198.0`
- resolution `v0.59.0`; its go.mod pins encounter `v0.103.1`, which predates ObservedContext
- session `v0.114.1`; pins encounter `v0.112.0`
- encounter `v0.112.0`

rpg-api pins session `v0.113.0`. Web pins protos `v0.1.219`.

**Sequence.**
- Develop outside-in: protos contract (T1) → web concept against T1 bindings (T6, begins on a fixture) → API mapping (T5) → toolkit root (T2) → resolution (T3) → session (T4). Each consumer pins its provider's pushed commit with `go get <module>@<sha>`. T5 pins T4's commit (plus root and resolution indirectly), then the walk (T7).
- Merge inside-out: T1 merges first (protos exception) → T2 → tag → T3 repins root tag → tag → T4 repins root and resolution tags → tag → T5 repins session tag and `gen/go@generated` → T6 adopts the protos tag → re-walk on released pins (T7 final check).
- Task 2A rides the root PR after T2's core work. Task 2E is its own encounter PR; it merges and tags before T3 and T4 pin encounter.
- T2 and T3 must ship as a pair. T2's handlers refuse a zero frame, so session must never combine new root with old resolution. T4's go.mod pins both.

**Open items.**
- *Decided:* Afford fails closed if `ObservedContext` errors. It errors only on internal inconsistency, and `buildTargetPreflight` already fails the same read on a roster inconsistency. This is a broken read, not a row refusing an action.
- *Scope flags (decided, flag only):*
  - Bane answers, because it is the same provider seam as Bless.
  - Bless on spell attacks is included, because execution already routes them through `NewStrike` (`resolution/action.go` `newAttackCast`) and `describeRollContributions(...RollKindAttack)` (`strike.go` `afterAttackChain`).
- *Rename:* `contributions.NeedsContext` becomes `contributions.Depends`, one word matching the wire.
- *Explicit deferrals:* R12, R14, toolkit#1929, toolkit#1934.
- *Ruled by the operator:* Martial Arts' ability and die move to assembly in this wave (Task 2A) unless the move proves distracting; its stop condition is in the task. A member with no faction has no stance and the frame carries that as a known no side, never neutral (Task 2E, T2, T3).

---

### Task 1: Wire contract (rework rpg-api-protos#374)

**Delivers:** D1, D2, D5, D7. Rows ride `Declaration` and `TargetCandidate`. No inspection RPC.

**Owner:** rpg-api-protos, `dnd5e/api/session/v1alpha1/types.proto`.

**Prerequisites:** none.

**Files:**
- Modify `types.proto` (messages `Declaration`, `TargetCandidate`).
- On branch `feat/effect-information`, add new commits (do not rewrite history) that remove everything #374 added:
  - from `service.proto`: `InspectActionsRequest`, `InspectActionsResponse`, `rpc InspectActions`
  - from `types.proto`: `Declaration.information = 21`, `ActionInformationVariant`, `ActionInformationRef`
  - from `events.proto`: all 137 added lines (need/facet enums)

**Interfaces:**
- `enum EffectState { EFFECT_STATE_UNSPECIFIED = 0; EFFECT_STATE_APPLIES = 1; EFFECT_STATE_DOES_NOT_APPLY = 2; EFFECT_STATE_DEPENDS = 3; EFFECT_STATE_UNAVAILABLE = 4; }`. UNSPECIFIED means a producer defect.
- `enum EffectParticipation { EFFECT_PARTICIPATION_UNSPECIFIED = 0; EFFECT_PARTICIPATION_CONTRIBUTES_NOW = 1; EFFECT_PARTICIPATION_LATER_CHOICE = 2; }`
- `message EffectRow { string id = 1; string ref = 2; string name = 3; string description = 4; EffectState state = 5; string reason = 6; EffectParticipation participation = 7; string benefit = 8; }`. `id` is opaque and unique within its declaration. `benefit` is empty when there is no line.
- `message TargetEffect { string id = 1; EffectState state = 2; string reason = 3; string benefit = 4; }`. It replaces state, reason and benefit wholesale for the row with the same `id`. Participation and description are never repeated.
- `Declaration`: `repeated EffectRow effects = 21;` 21 is free on main; `reserved 4, 6` are the only reservations.
- `TargetCandidate`: `repeated TargetEffect effects = 4;` 4 is free; fields 1–3 are used.
- Plain `ref` and `name` strings, not `RollSource`. `events.proto` imports `types.proto`, so `types.proto` cannot import `events.proto` without a cycle.
- Doc comments carry the law: rows never grant or refuse; an empty list on a non-content declaration; the client overlays by `id`; the wire names no feature. Note "lands with rpg-project#520" per the file's header convention.

**Behavior:** contract only.

**Tests:** none bespoke (repo rule: do not re-test generated mechanics).
- [ ] Rework commits. [ ] `buf format -w`. [ ] `make test`.

**Verification:** `cd rpg-api-protos && git fetch origin main && make format && make test`. Expect lint, format-check, proto-options and breaking all to pass. Adding fields and enums is non-breaking, and the removed draft symbols never shipped on main. Run `git diff origin/main --stat` and expect only `types.proto` changed.

**Completion evidence:** PR #374 diff limited to `types.proto`. CI green. After merge: `generated` branch and the release tag (record which tag) consumed by T5 and T6.

---

### Task 2: Toolkit root — facts, frame, rule answers, census (trim rpg-toolkit#1932)

**Delivers:** O1–O4, O6, O7 (handler half), O9 (types), D6, P2, S2, plus Bless/Bane/BI/Rage/Sneak rules.

**Owner:** rpg-toolkit module `rulebooks/dnd5e`.

**Prerequisites:** none. Develop on the #1932 branch.

**Files.**

Keep (from #1932):
- `contributions/{fact.go, source.go, doc.go}`
- `events/roll_trace.go` aliases
- `conditions/display.go` details
- `character/status_view_test.go`

Modify:
- `contributions/decision.go`:
  - Delete `Facet`, `NeedKind`, `Need`, `Decision.Needs`.
  - Rename `NeedsContext` to `Depends`.
  - Add `Participation` and `EffectState`.
- `events/events.go`: `DamageChainEvent` and `DamageChainInput` gain `Frame contributions.Frame`; `NewDamageChainEvent` copies it.
- `events/offer.go`: `PostRollOfferEvent` gains `Frame contributions.Frame`.
- `conditions/raging.go` and `conditions/raging_assessment.go`: keep `ragingDamageRule`; delete `AssessmentSnapshot` and the binding.
- `conditions/sneak_attack.go`: pure rule; remove the `gamectx` import.
- `conditions/blessed.go`, `conditions/baned.go`: `AssessAction` delegates to the existing `RollContributionMetadata`.
- `conditions/baned.go` `DescribeSelectedRollContributions`: re-expressed over the shared per-provider walk.
- `conditions/inspired.go`: `onPostRollOffer` asks the rule with `event.Frame`.
- `conditions/display.go`: add `Detail` to every "answers" and "not yet answering" entry (Inspired currently has none).
- Every conditions test that builds a `DamageChainEvent` or `PostRollOfferEvent` for Rage, Sneak or Inspired must now set a valid `Frame`.

Delete:
- `combat/assessment/` (context, damage, doc and tests). Its frame cannot live there: `events` must reference the frame, `assessment` imported `events` (for `ConditionAddress`), and that is a cycle. `contributions` is already the leaf that `events` imports, and it imports only `core` and `abilities`. `abilities` imports only `rpgerr`.

Create (proposed):
- `contributions/frame.go`
- `contributions/effect.go`
- `conditions/action_effects.go` (the fan-out)
- `conditions/action_census.go`
- `conditions/action_census_test.go`
- `conditions/sneak_attack_rule_test.go`
- `conditions/action_effects_test.go`

Implementation note: rename the local variables named `contributions` in `character/character.go` and `character/death_save.go` if those files import the package.

**Interfaces (produced; T3 consumes).**

In `contributions`:
- `type Stance string`, with `StanceHostile = "hostile"`, `StanceNeutral = "neutral"`, `StanceAllied = "allied"`, `StanceNone = "none"` (no side: a member with no faction; known, and not hostile).
- `type ActionFacts struct { Roll Fact[RollKind]; Ability Fact[abilities.Ability]; Melee Fact[bool]; WeaponPool Fact[bool]; Advantage Fact[bool] }`. `Ability` Known("") means "the attack declares no governing ability", which is a stat block's honest answer.
- `type PairFacts struct { From, To string; DistanceCells Fact[float64]; Stance Fact[Stance] }`
- `type Frame struct { Actor string; Target Fact[string]; Action ActionFacts; Pairs []PairFacts; Complete bool }`
- Frame methods:
  - `func (Frame) Validate() error`: Actor non-empty; `Action.Roll` known; pairs distinct, non-empty, non-duplicate; known distances finite and ≥ 0.
  - `func (Frame) Pair(from, to string) PairFacts`: a missing pair returns all-unknown, never a negative.
  - `func (Frame) Clone() Frame`
- `MemberFacts` is removed: no first-delivery rule reads Down, and ReactionReady has no producer.
- `type Applicability string`: `Applies`, `DoesNotApply`, `Depends`. `type Decision struct { Applicability Applicability; Reason string }`, plus `Validate()`.
- `type Participation string`: `ContributesNow = "contributes_now"`, `LaterChoice = "later_choice"`.
- `type DamageChange struct { PoolID string; Source Source; Fixed *int; Dice string }`, moved from assessment. `const PrimaryWeaponPool = "weapon:primary"`.
- `type Answer struct { Decision Decision; Participation Participation; Benefit string; Damage []DamageChange; Roll []DiceContribution }`. `Benefit`, `Damage` and `Roll` are non-empty only when the answer is Applies.
- `type AssessActionInput struct { Frame Frame }`, `type AssessActionOutput struct { Answer Answer }`, `type ActionAssessor interface { AssessAction(*AssessActionInput) (*AssessActionOutput, error) }`.
- `type EffectState string`: `StateApplies`, `StateDoesNotApply`, `StateDepends`, `StateUnavailable`.
- `type Effect struct { ID string; Source Source; Description string; State EffectState; Reason string; Participation Participation; Benefit string }`
- `var ErrRuleCannotAnswer = errors.New(...)`, the sentinel execution wraps on a Depends answer.

In `conditions`:
- `type AssessActionEffectsInput struct { Conditions []dnd5eEvents.ConditionBehavior; Frame contributions.Frame }`
- `type AssessActionEffectsOutput struct { Effects []contributions.Effect }`
- `func AssessActionEffects(*AssessActionEffectsInput) (*AssessActionEffectsOutput, error)`. It:
  - validates the frame;
  - walks the conditions in persisted order;
  - classifies each by census (unknown ref = error);
  - "does not bear" → no row;
  - "not yet answering" → `StateUnavailable`, reason `"This effect cannot yet say whether it applies to this action"`, description `DisplayFor(ref).Detail`, participation from the census;
  - "answers" → `AssessAction`, mapped one-to-one;
  - for `RollContributionProvider`s, when an earlier provider in the same group already Applies, the later one becomes DoesNotApply with the reason `"Another <Name> already adds to this roll"`.
- Row `ID`: `ref.String()`, plus `"@"+SourceID` when `ConditionAddressProvider` gives a non-empty SourceID. A duplicate ID is an error.
- The census lives in an unexported map in `action_census.go`, keyed by loader ref string, holding a class (`answers` / `notBearing` / `notYetAnswering`) and a participation.

Census table (40 loaders, `conditions/loader.go` `conditionLoaders`):
- **answers (5):** Raging, SneakAttack (`refs.Features`), Blessed, Baned, Inspired. Inspired = LaterChoice; the rest ContributesNow.
- **notYetAnswering → UNAVAILABLE (16):** Shillelagh, BrutalCritical, FightingStyleArchery, FightingStyleDueling, FightingStyleGreatWeaponFighting, FightingStyleTwoWeaponFighting, ImprovedCritical, RecklessAttack, MartialArts, Prone, Hidden, Helped, TrueStrike, ViciousMockery, DivineFavor, Sanctuary, SanctuaryImmune, InFog.
- **notBearing (19):** UnarmoredDefense, FightingStyleDefense, FightingStyleProtection, UnarmoredMovement, Disengaging, Dodging, Unconscious, OpportunityAttack, BladeWard, Commanded, Concentrating, FaerieFire, ShieldOfFaith, Guided, Resistance, GuidingBolt, Shield (`refs.Spells`).
- The counts above are indicative; the lists are the classification. The implementer reconciles them to exactly the 40 loader keys, and the census test enforces it.
- The census covers the acting character's own loaded effects only.

**Rules (pure functions of the frame plus the condition's own persisted state; no `gamectx`, roll, spend or publish).**
- **Raging** (`ragingDamageRule{owner, bonus}`):
  - actor ≠ owner → DoesNotApply.
  - Known non-weapon pool → DoesNotApply "Rage requires a weapon damage pool".
  - Known non-melee → DoesNotApply "Rage's damage bonus requires a melee weapon attack".
  - Known ability ≠ STR → DoesNotApply "Rage's damage bonus requires Strength".
  - Any of those three unknown → Depends.
  - Otherwise Applies "The melee weapon attack uses Strength", benefit `"+<bonus> damage"`, Damage = Fixed bonus on `PrimaryWeaponPool`.
- **SneakAttack** (`sneakAttackRule{owner, usedThisTurn, dice}`), in order:
  1. actor ≠ owner → DoesNotApply.
  2. used → DoesNotApply "Already used this turn".
  3. Known ability ≠ `abilities.DEX` → DoesNotApply "Sneak Attack requires a Dexterity attack". This keeps the #1929 defect.
  4. Known WeaponPool false → DoesNotApply.
  5. Target unknown → Depends "Depends on the target".
  6. Advantage known true → Applies.
  7. Any pair From=target, To=X (X ∉ {actor, target}) with known distance ≤ `combat.AdjacentCells` and known `StanceHostile` → Applies "Another enemy of the target is within 5 feet". Benefit `"+<dice>d6 damage"`.
  8. Complete and Advantage known false and every target pair has known distance and stance → DoesNotApply "No advantage and no other enemy of the target within 5 feet".
  9. Otherwise Depends "Needs advantage or another enemy of the target within 5 feet".
- **Blessed / Baned:** Applies iff `RollContributionMetadata(&DescribeRollContributionsInput{Kind: roll})` is applicable. This is the same function execution calls. Benefits: `"+1d4 to the attack roll"` and `"−1d4 to the attack roll"`. `Roll` carries the existing `DescribeRollContributions` output.
- **Inspired:**
  - actor == holder and Roll == attack → Applies, LaterChoice, benefit `"May add <Die> after seeing the roll"`.
  - Another actor → DoesNotApply.
  - Roll unknown → Depends.

**Behavior (execution handlers).**
- Handlers keep their own lifecycle and contributions (R14 deferred). Each handler calls `Frame.Validate()` first. An invalid frame (including the zero frame) returns an error wrapping `ErrRuleCannotAnswer`.
- Rage `onDamageChain`, attacker side, asks `ragingDamageRule` with `e.Frame.Clone()`:
  - Depends → error.
  - Applies → append the component from `Answer.Damage`.
  - The defender-side resistance path is unchanged.
- Sneak Attack `onDamageChain`:
  - Depends → error.
  - Applies → roll, set `UsedThisTurn`, publish state changed, add the modifier (existing code).
  - DoesNotApply → no-op.
- Inspired `onPostRollOffer`: Applies → append the offer; Depends → error.

**Tests** (testify suites, `rulebooks/dnd5e`):
- `contributions`:
  - `TestFactKeepsKnownFalseDistinctFromUnknown` (exists)
  - `TestDecisionRequiresReasonAndKnownApplicability`: empty reason, or `"needs_context"`, gives an error.
  - `TestFrameValidateRejectsZeroFrameDuplicatePairsAndNaN`
  - `TestFramePairMissingIsUnknown`
- `TestRagingRuleAppliesToMeleeStrengthWeapon`: `{Actor:"barb", Roll attack, Ability STR, Melee true, WeaponPool true}`, bonus 2 → Applies, benefit `"+2 damage"`, one change Fixed 2 on `weapon:primary`.
- `TestRagingRuleDoesNotApplyToDexterity`: exact reason as above.
- `TestRagingRuleDependsWhenAbilityUnknown`
- `TestRagingHandlerReadsFrameNotEventFields`: `e.AbilityUsed=DEX`, `Frame.Ability=STR` → component appended.
- `TestRagingHandlerFailsWhenRuleDepends`: `errors.Is(err, ErrRuleCannotAnswer)`, and no component appended.
- `TestRagingHandlerRejectsZeroFrame`
- Sneak Attack rule:
  - `TestSneakAttackRuleAppliesWithKnownAdvantage`
  - `TestSneakAttackRuleAppliesWithEnemyOfTargetAdjacent`: pair target→ally, 1.0, hostile.
  - `TestSneakAttackRuleNeutralAdjacentDoesNotQualify`: Depends if incomplete, DoesNotApply if complete with known false advantage.
  - `TestSneakAttackRuleIgnoresActorAdjacency`
  - `TestSneakAttackRuleDependsWithoutProofWhenIncomplete`
  - `TestSneakAttackRuleDoesNotApplyWhenCompleteWithoutAdvantageOrEnemy`
  - `TestSneakAttackRuleUsedThisTurn`
  - `TestSneakAttackRuleKeepsDexOnlyDefect`: STR finesse → DoesNotApply.
  - `TestSneakAttackRuleDependsWithoutTarget`
- `TestSneakAttackHandlerNeedsNoGameContext`: bare `context.Background()` plus a frame with an adjacent enemy → dice component added, `UsedThisTurn` true. The old code returned false here.
- `TestSneakAttackSourceImportsNoGameContext`: parses `sneak_attack.go` imports.
- `TestSneakAttackHandlerFailsWhenRuleDepends`
- `TestSecondBlessDoesNotApplyWithReason`: Blessed from sources A then B → A Applies, B DoesNotApply `"Another Bless already adds to this roll"`.
- `TestSelectedRollContributionsMatchAssessedApplies`: same conditions; the dice from `DescribeSelectedRollContributions` equal the union of `Answer.Roll` over Applies rows.
- `TestInspiredAnswersLaterChoiceOnHoldersAttack`, `TestInspiredDoesNotApplyToAnothersAttack`, `TestInspiredOfferHandlerFailsOnZeroFrame`
- `TestAssessingSpendsNothing`: assess Sneak, Inspired and Raging → JSON is byte-identical before and after.
- Census:
  - `TestEveryConditionLoaderIsClassified`: census keys == `conditionLoaders` keys, both directions.
  - `TestAnsweringLoadersImplementActionAssessor`: a load fixture per answering ref, type-asserted.
  - `TestBearingLoadersHaveDescriptions`: answers and notYet have a non-empty `Detail`.
  - `TestNotYetAnsweringYieldsUnavailableRow`: Archery → 1 row, `StateUnavailable`, description equals the catalog.
  - `TestNotBearingYieldsNoRow`: UnarmoredDefense → 0 rows.
  - `TestEffectIDsUniqueAndDeterministic`
- `TestNewDamageChainEventCarriesFrame`

Steps:
- [ ] Write the tests and confirm the zero-frame and frame-reading tests fail on `1f73c0c8`.
- [ ] Trim and implement.
- [ ] Update `conditions` tests that build chain events.
- [ ] Update the `contributions/doc.go` package doc. Remove the "Facet" language.

**Verification:** `cd rpg-toolkit/rulebooks/dnd5e && go test -race ./... && golangci-lint run ./...`. Expect a pass, and `grep -rn "gamectx" conditions/sneak_attack.go` empty. Push and record the commit SHA for T3.

**Completion evidence:** #1932 diff no longer contains `combat/assessment` or `Facet|Coverage|Need`. Named tests pass. Pushed SHA recorded.

---

### Task 2A: Toolkit root — Martial Arts settles ability and die at assembly

**Delivers:** O8 (R10) for a monk's unarmed strike and monk weapons: the frame's ability and the damage die are the ones the swing uses.

**Owner:** rpg-toolkit module `rulebooks/dnd5e`, same PR as T2, as its own commit after T2's core work.

**Prerequisites:** T2's frame and `attackActionFacts` contract (the frame reads the assembled profile).

**Files:**
- Modify `conditions/martial_arts.go`: implement the existing override provider method `WeaponAttackOverride(slot, itemID string) *weaponattack.Override` (the one `conditions/shillelagh.go` implements and `character/attack_definition.go` `AssembleAttack` consults). Delete the ability swap and the die replacement from `onDamageChain` and `onAttackChain`; delete a handler entirely when nothing remains in it.
- Modify `character/martial_arts_attack.go` `AssembleMartialArtsBonusAttack` so the bonus unarmed strike is assembled with the same override.
- Update `conditions/martial_arts_test.go` and the character assembly tests.

**Interfaces:** no new exported type. `weaponattack.Override{Dice, Ability}` is consumed as it stands.

**Behavior:**
- Unarmed strike or monk weapon: ability is Dexterity when its modifier is higher than Strength's, otherwise unchanged; an unarmed strike's die is the Martial Arts die for the monk's level.
- The die is selected before it is rolled. No roll is made and then discarded.
- A non-monk weapon is untouched.

**Stop condition:** if the override provider cannot carry this without changing `weaponattack.Override`'s shape, changing the provider method's signature, or touching non-monk weapon assembly, stop, leave Martial Arts as it is on main, classify it not-yet-answering, and report. The gap is then recorded on #520 and not built here.

**Tests:**
- `TestMartialArtsOverridesUnarmedAbilityAndDieAtAssembly`: monk level 1, DEX 16, STR 10, unarmed → assembled profile ability DEX, die `1d4`.
- `TestMartialArtsKeepsStrengthWhenHigher`: STR 16, DEX 10 → ability STR.
- `TestMartialArtsLeavesNonMonkWeaponAlone`: greataxe → no override.
- `TestMartialArtsBonusAttackUsesSameOverride`.
- `TestMartialArtsRollsItsDieOnce`: a counting roller sees exactly one damage roll for an unarmed hit.
- `TestAttackActionFactsForMonkUnarmedIsDexterity` (asserted in T3 against this commit).

**Verification:** `cd rpg-toolkit/rulebooks/dnd5e && go test -race ./... && golangci-lint run ./...`.

**Completion evidence:** named tests pass; `grep -n "AbilityUsed = " conditions/martial_arts.go` is empty; or the stop-condition report.

---

### Task 2E: Toolkit encounter — one authoritative stance read

**Delivers:** O9 (R5): one authoritative read for a relationship, with no faction answered as no stance, matching the believed read.

**Owner:** rpg-toolkit module `rulebooks/dnd5e/encounter`, its own PR from `origin/main`.

**Prerequisites:** none.

**Files:**
- Modify `encounter/world.go`: add the public read below. `believedStanceBetween` and `BelievedStance` are unchanged.
- Create `encounter/stance_between_test.go`; one line in `encounter/doc.go`.

**Interfaces:**
- `func (e *Encounter) StanceBetween(a, b MemberID) (Stance, bool)`: the authoritative stance between two members. The bool is false when no stance exists: either is not a member, or either has no faction. Hostile, neutral and allied otherwise follow the existing fold.
- `IsHostile` and `IsAllied` keep their signatures and answers, including known false for a member with no faction.

**Behavior:** for every pair of members, `StanceBetween` hostile ⇔ `IsHostile` true and allied ⇔ `IsAllied` true. For sighted members and absent deception, `BelievedStance` and `StanceBetween` agree, including on no stance.

**Tests:**
- `TestStanceBetweenFactionlessHasNoStance`.
- `TestStanceBetweenNoStanceForNonMember`.
- `TestStanceBetweenAgreesWithIsHostileAndIsAllied`: every ordered pair of a party, three factions and a world NPC.
- `TestStanceBetweenAgreesWithBelievedStance`: same fixture.
- The existing tests pinning that a world NPC has no stance to believe stay untouched and passing.

**Verification:** `cd rpg-toolkit/rulebooks/dnd5e/encounter && go test -race ./... && golangci-lint run ./...`.

**Completion evidence:** named tests pass; PR open; no change to `BelievedStance`, `ObservedContext` or any sighting on the wire.

The frame's relationship fact carries four known values: hostile, neutral, allied and no side (`contributions.StanceNone`, T2). T3's execution frame maps two placed members with no stance to known no side; the information frame maps a nil observed stance to unknown. A rule reads no side as known and not hostile.

---

### Task 3: Toolkit resolution — build frames; information producer; execution wiring

**Delivers:** O5, O7 (execution), O8 (as far as reachable; see A1), O9, K1–K5, S3 (frame rebuilt from truth on resume, rows never consulted).

**Owner:** rpg-toolkit module `rulebooks/dnd5e/resolution`.

**Prerequisites:** T2 pushed SHA (`go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@<T2sha>`). Bump encounter to `v0.112.0` (needs ≥ v0.111.0 for ObservedContext).

**Files:**
- Create (proposed) `resolution/frame.go` (both builders plus shared action facts), `resolution/inform.go`, `resolution/frame_test.go`, `resolution/inform_test.go`.
- Modify `resolution/strike.go`:
  - `strikeMachine` gains a `frame *contributions.Frame` memo.
  - `afterAttackChain` sets `offerEvent.Frame`.
  - `rollDamage` sets `DamageChainInput.Frame` and computes advantage from the frame.
- Modify `resolution/go.mod` and `go.sum`.

**Interfaces:**
- `func attackActionFacts(p *combatActions.AttackProfile) contributions.ActionFacts`. Unexported, and the one derivation shared by both frames:
  - Roll = Known(attack)
  - Ability = Known(p.Ability.Ability), or Known("") if nil
  - Melee = Known(p.Delivery.IsMelee())
  - WeaponPool = Known(Category==weapon && exactly one pool has `damage.AddsAttackAbilityModifier`), matching `conditions.primaryWeaponComponent`
  - Advantage = Unknown
- Information frame, built from an `*encounter.ObservedContextOutput`:
  - Actor = Observer; Target = Known(id) or Unknown for the no-target ask.
  - Pairs map `DistanceCells` to Known.
  - `Stance` nil → Unknown, else Known(`contributions.Stance(*Stance)`).
  - Complete = false. Advantage stays Unknown, because advantage is a fold result, not a frame fact.
- Execution frame (`func (m *strikeMachine) executionFrame(ctx) (contributions.Frame, error)`, memoized per machine):
  - Actor, Target Known.
  - Advantage = Known(len(Folded.AdvantageSources) > 0 && len(Folded.DisadvantageSources) == 0), the existing `effectiveAdvantage`.
  - Pairs over every cast member that `gamectx.Room(ctx)` places, with distance `room.GetGrid().Distance`. This is the same grid primitive as `encounter.Distance`, and so the same metric as ObservedContext.
  - Stance from the cast's authoritative `StanceBetween` (Task 2E): its stance when one exists; known `StanceNone` when both are placed members and no stance exists. Never unknown for two placed members.
  - Complete = true. No room → error.
- `type InformAttackInput struct { Observed *encounter.ObservedContextOutput; Actor *character.Character; Attack *combatActions.AttackProfile; Targets []string }`
- `type InformAttackOutput struct { Effects []contributions.Effect; ByTarget map[string][]contributions.Effect }`. `ByTarget` lists hold the same IDs in the same order as `Effects`.
- `func InformAttack(in *InformAttackInput) (*InformAttackOutput, error)`: calls `conditions.AssessActionEffects` with `Actor.GetConditions()` once with no target and once per target. Nil input or nil Observed is an error. There is no field for target sheets or the cast, which enforces K4 by construction.

**Behavior:**
- The frame is built once per strike machine:
  - first use in `afterAttackChain`, after the fold and before `PostRollOfferEvent`;
  - reused in `rollDamage`;
  - a resumed machine (`strike_pose.go`, `NewStrikeResumed`) rebuilds it from current truth plus the frozen fold.
- A handler error wrapping `ErrRuleCannotAnswer` propagates out of `foldDamage` or `gatherPostRollOffers`, and `Resolve` returns it. The session verb then saves nothing (S4 load-act-save).
- Rows are never read by execution.

**Tests:**
- `TestAttackActionFactsWeaponFinesse`: shortsword DEX profile → {attack, DEX, melee true, pool true}.
- `TestAttackActionFactsSpellAttack`: WeaponPool false.
- `TestAttackActionFactsNoAbilityIsKnownNone`
- `TestInformationFrameMapsNilStanceToUnknown`: Complete false, Advantage unknown.
- `TestInformAttackSneakAttackPerTarget`: real encounter with rogue R (shortsword), goblin G1 with fighter F adjacent and sighted, lone goblin G2.
  - `Effects` Sneak = Depends "Depends on the target".
  - `ByTarget[G1]` = Applies.
  - `ByTarget[G2]` = Depends.
- `TestUnseenAllyLeavesRowsUnchanged` (K3/K5): F adjacent to G1 but not sighted (blocked) → `ByTarget[G1]` deep-equals the same scene with no F.
- `TestInformAttackInputCarriesNoTargetSheets`: reflection, field set exactly {Observed, Actor, Attack, Targets}.
- `TestStrikeBuildsOneExecutionFrameForOfferAndDamage`: a spy condition records `PostRollOfferEvent.Frame` and `DamageChainEvent.Frame` → equal, Complete true, Advantage known, pair G1→F distance 1, hostile.
- `TestStrikeSneakAttackFromExecutionFrame`: R hits G1 with F adjacent → damage components contain Sneak Attack dice.
- `TestStrikeFailsWhenARuleCannotAnswer`: spy damage-chain handler returns `ErrRuleCannotAnswer` → `errors.Is` on the `Resolve` error, target HP unchanged.
- `TestResumedStrikeRebuildsFrameFromTruth`: post-roll Inspired offer taken → damage frame Complete true.
- Existing strike, bless and resume suites pass.

**Verification:** `cd rpg-toolkit/rulebooks/dnd5e/resolution && go test -race ./... && golangci-lint run ./...`. Expect a pass. Push and record the SHA.

**Completion evidence:** named tests pass. go.mod shows root `<T2 pseudo>` and encounter `v0.112.0`.

---

### Task 4: Toolkit session — attach rows in Afford only

**Delivers:** D1–D4, D6, S2, R12 behaviour, spell-attack Bless, K1 (ObservedContext is the only source).

**Owner:** rpg-toolkit module `rulebooks/dnd5e/session`.

**Prerequisites:** T2 and T3 SHAs pinned.

**Files:**
- Modify `session/types.go` (`TargetCandidate`) and `session/afford.go`:
  - `Declaration` gains `Effects`.
  - `Afford` calls attach after `compileOffersFor`.
- Create (proposed) `session/effects.go` and `session/effects_test.go` (suite `EffectRowsSuite`).
- Modify `session/wirecontract_test.go` (JSON keys).
- Update `session/README.md` and `session/AGENTS.md` where `Declaration` fields are enumerated.

**Interfaces:**
- `type EffectState string`: `EffectApplies = "applies"`, `EffectDoesNotApply = "does_not_apply"`, `EffectDepends = "depends"`, `EffectUnavailable = "unavailable"`.
- `type EffectParticipation string`: `ContributesNow = "contributes_now"`, `LaterChoice = "later_choice"`.
- `type EffectRow struct { ID, Ref, Name, Description string; State EffectState; Reason string; Participation EffectParticipation; Benefit string }`, with snake_case json tags.
- `type TargetEffect struct { ID string; State EffectState; Reason, Benefit string }`
- `Declaration.Effects []EffectRow` with `json:"effects,omitempty"` (repeated, so there is no presence to preserve). `TargetCandidate.Effects []TargetEffect` with `json:"effects,omitempty"`.
- Unexported `func (m *Manager) attachEffects(enc *encounter.Encounter, member string, sheet *character.Character, offers []compiledOffer) error`:
  - one `enc.ObservedContext(&encounter.ViewInput{Member: ...})` per Afford;
  - for each offer with `attack != nil`, or `spell != nil && spell.Cast.Attack != nil`, call `resolution.InformAttack` with the candidate members in candidate order;
  - project declaration rows;
  - per candidate, emit a `TargetEffect` only where (State, Reason, Benefit) differs from the no-target row with the same ID.

**Behavior:**
- Attach runs only in `Afford`, after `compileOffersFor`. The execution callers of `compileOffersFor` (`attack.go`, `cast.go`, `activate.go`, `move.go`, `death_save.go`) never compute rows, so rows cannot refuse execution.
- Afford's early returns (`affordWhileFrozen`, not-your-turn, world clock) and blocked declarations get no rows.
- A compiled-but-unavailable Attack still gets rows.
- An `ObservedContext` error fails Afford with a wrapped error (A3).

**Tests** (fixtures from `afford_test.go` `aFight` / `armedFighter`, and the condition-seating pattern from `clockboundary_test.go`):
- `TestAttackDeclarationCarriesRageRow`: raging barbarian, greataxe → row {Name "Raging", State applies, Reason "The melee weapon attack uses Strength", Benefit "+2 damage", Participation contributes_now, Description equal to the catalog}. No candidate carries a Raging effect.
- `TestSneakAttackAnswersPerTarget`: declaration row depends. G1 candidate effects = [{sneak id, applies}]. G2 = [{depends, "Needs advantage or another enemy of the target within 5 feet"}].
- `TestBlessAndInspirationRows`: Bless applies "+1d4 to the attack roll"; Bardic Inspiration applies, later_choice.
- `TestUnavailableAttackStillCarriesRows`: capacity spent → `Available` false, rows present.
- `TestNoRowsOffTurnOrWhileFrozen`: every declaration's `Effects` is empty in both cases.
- `TestNonAttackDeclarationsCarryNoRows`: Move, EndTurn, Activate, and save-cast Sacred Flame.
- `TestSpellAttackCastCarriesBlessRow`: blessed caster's Guiding Bolt Cast declaration has a Bless applies row.
- `TestUnansweringEffectShownUnavailable`: Archery fighter.
- `TestReadingRowsChangesNothing`: Afford twice → characters repo save count unchanged and sheet JSON identical; then Attack → Sneak Attack dice in the outcome.
- `TestEffectsAreNotSelectorMaterial`: Attack declaration ID identical before and after Bless lands.
- `TestEffectRowWireKeys`: JSON contains `effects`, `state`, `reason`, `participation`.
- `TestNoInnerTypeCrossesTheBoundary` still passes.

**Verification:** `cd rpg-toolkit/rulebooks/dnd5e/session && go test -race ./... && golangci-lint run ./...`. Push and record the SHA.

**Completion evidence:** named tests pass. go.mod pins the T2 and T3 pseudo-versions.

---

### Task 5: rpg-api mapping

**Delivers:** O10 (transport only), D5 across the boundary.

**Owner:** rpg-api, `internal/handlers/dnd5e/session/v1alpha1/convert.go`.

**Prerequisites:**
- T1 merged and `generated` published: `GOPROXY=direct go get github.com/KirkDiggler/rpg-api-protos/gen/go@generated`.
- T4 SHA: `go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/session@<T4sha>`, which pulls root and resolution.

**Files:**
- Modify `convert.go`:
  - `declarationToProto` sets `Effects: effectRowsToProto(d.Effects)`.
  - `targetCandidateToProto` sets `Effects: targetEffectsToProto(c.Effects)`.
  - New: `effectRowsToProto`, `targetEffectsToProto`, `effectStateToProto`, `effectParticipationToProto`.
- Modify `convert_test.go` and `afford_test.go`, plus `go.mod` and `go.sum`.

**Interfaces:**
- `sdk.EffectApplies` → `sessionpb.EffectState_EFFECT_STATE_APPLIES`, and likewise for the other states. An unknown value maps to `_UNSPECIFIED`.
- Participation maps the same way.
- Use make-then-map so empty stays non-nil.

**Behavior:** a field-for-field copy. No filtering, ordering or deriving.

**Tests:**
- `TestDeclarationToProto_CarriesEffectRows`: every field asserted for a two-row input.
- `TestTargetCandidateToProto_CarriesTargetEffects`
- `TestEffectStateToProto_UnknownIsUnspecified`
- `TestEffectParticipationToProto_UnknownIsUnspecified`
- Extend `TestDeclarationToProto_FieldForField` and `TestAfford_HappyPath_ProjectsNestedDeclaration` with effects.

**Verification:** `cd rpg-api && make ci-check`. Expect tests to pass. `release-pin-check` fails while toolkit pins are pseudo-versions, so the PR stays draft until tags are adopted.

**Completion evidence:** tests pass. PR in draft with a pin-adoption checklist.

---

### Task 6: rpg-dnd5e-web rendering

**Delivers:** P1–P5, O10 (renders, names nothing), D1 (overlay by id).

**Owner:** rpg-dnd5e-web, `src/components/session/combat-experience/`.

**Prerequisites:** T1 tag (`npm i --save github:KirkDiggler/rpg-api-protos#<tag>`, then commit `package.json` and `package-lock.json`).

**Files.**

Modify:
- `actionTooltip.ts`:
  - `ActionTooltip` gains `effects: readonly ActionEffectLine[]`.
  - `buildActionTooltip` projects `declaration.effects` verbatim.
  - `actionTooltipText` includes the rows.
  - New exported `effectLinesFor(declaration, candidateMember?: string)` overlays `candidate.effects` by `id`.
  - New `effectStateWord(EffectState)`: "Applies", "Does not apply", "Depends", "Unavailable".
  - A later-choice row renders as "Available after the roll". Never as added.
- `OrganizedActionSurface.tsx` `Inspection`, the production surface (`liveActionPresentation.ts` mode `'organized-hud'`): render the effect list. It already has mouse/pen hover, keyboard focus, and the touch long-press via `offerPress.ts` `bindOfferPress`.
- `ActionDock.tsx` `ActionTooltipCard` (classic dock, hover/focus-within CSS): render the same list.
- `TargetSurface.tsx` target list:
  - hover/focus on a candidate button sets `inspectedCandidate`;
  - a separate read-only "Effects" toggle button per candidate (`aria-expanded`) serves touch;
  - a panel renders `effectLinesFor(declaration, member)`;
  - `onTargetClick` is unchanged;
  - new prop `hoveredTarget?: string | null`.
- `CombatExperience.tsx`: thread `hoveredTarget`.
- `SessionEncounterView.tsx`: pass `onHoverEntity` to `<SessionCanvas>` (unused there today; only `src/concepts/session-combat/SessionCombatMap.tsx` uses it), store it, and pass it down.

Create (proposed): `EffectRows.tsx`, a shared renderer for name, state word, reason, description, and benefit.

Optional outside-in step: add effect rows to the `src/concepts/organized-hud/OrganizedHudConcept.tsx` fixture.

**Interfaces:** consume `Declaration.effects`, `TargetCandidate.effects`, `EffectState` and `EffectParticipation` from `@kirkdiggler/rpg-api-protos/gen/ts/dnd5e/api/session/v1alpha1/types_pb`.

**Behavior:**
- Names, reasons, descriptions and benefits are rendered verbatim. No ref-to-name table. No totals.
- A DOES_NOT_APPLY row stays visible.
- An UNAVAILABLE row shows its description with "Unavailable".
- Clicking an offer or a target behaves exactly as before.

**Tests:**
- `actionTooltip.test.ts`:
  - `projects effect rows verbatim` (two rows, every field)
  - `overlays candidate answers by id` (base depends + candidate applies → applies; description retained)
  - `ignores candidate answers for unknown ids`
  - `later choice reads as available, not added`
- `OrganizedActionSurface.test.tsx`:
  - `hover shows effect rows`
  - `keyboard focus shows effect rows`
  - `touch long-press shows the same rows read-only`
  - `click still selects the declaration once`
- `TargetSurfaceEffects.test.tsx`:
  - `candidate hover shows overlaid rows`
  - `effects toggle is read-only and does not call onTargetClick`
  - `candidate click calls onTargetClick unchanged`
  - `canvas hoveredTarget shows that candidate's rows`
- `EffectRows.test.tsx`: `renders all four states with reasons`.

**Verification:** `cd rpg-dnd5e-web && npm run test:run -- src/components/session/combat-experience` while iterating, then `npm run ci-check` once before the PR (paste its output in the PR body).

**Completion evidence:** tests pass. ci-check output in the PR.

---

### Task 7: Local-stack walk (integration)

**Delivers:** W1 plus end-to-end proof of D1, P1–P5, K3, S2, R12.

**Owner:** rpg-project #520 (evidence), running T4 pins through T5's image and T6's dev server.

**Prerequisites:** T1–T6 pushed. Follow `rpg-project/docs/howto/run-the-game-locally.md`: build the `rpg-api:local` image from T5's committed pins, compose with `docker-compose.local-dev.yml` plus `docker-compose.local-api-src.yml`, and run `npm run dev` in the T6 worktree. Normally created, unseeded level-1 Rogue, Barbarian, Cleric and Bard, with four browser sessions.

**Scenario:**
1. Rogue's turn, Rogue and Barbarian adjacent to goblin A, goblin B alone. Hover the Attack offer: Sneak Attack "Depends on the target", with its description. Hover goblin A in the list or on the canvas: Applies "+1d6 damage". Goblin B: Depends with its reason. Attack A: the outcome shows Sneak Attack dice. Hover again: "Already used this turn".
2. Barbarian activates Rage; hover Attack: Raging applies "+2 damage".
3. Cleric casts Bless on Rogue and Barbarian. Their Attack rows show Bless applies. Cleric's Guiding Bolt Cast row shows Bless if the Cleric blessed themself.
4. Bard grants Bardic Inspiration to Barbarian. Row reads "Available after the roll". Attack: the post-roll offer appears, and the total excludes the die until it is taken.
5. Touch emulation (devtools): long-press an offer and use the target "Effects" toggle, and see the same rows. Tapping still attacks.
6. Any member off-turn, and while a reaction window is open: no rows anywhere.

**Verification:** capture the Afford responses (devtools network) showing `effects` and `candidates[].effects`. Capture screenshots per step. Record the commit SHAs of every repo walked.

**Completion evidence:** a #520 comment with the SHAs, screenshots, the Afford JSON excerpt, and pass/fail per step. Then the release check: re-run steps 1 and 4 after T2–T6 adopt released tags.

---

## Requirement coverage

| Requirement (design law / ruling) | Task(s) | Concrete proof |
|---|---|---|
| P1 three states, not-applying stays readable (R7) | T2, T4, T6 | `TestSneakAttackAnswersPerTarget`; `EffectRows` "renders all four states"; walk step 1 |
| P2 description separate from reason (R11) | T2, T4, T6 | `TestAttackDeclarationCarriesRageRow` asserts Description == catalog and Reason separately |
| P3 no prediction, totals or panel (R1, R3) | T2, T6 | `Answer` has no total field; web renders verbatim; walk step 4 total excludes BI |
| P4 later choice shown available, not added (R9) | T2, T6 | `TestInspiredAnswersLaterChoiceOnHoldersAttack`; `later choice reads as available`; walk step 4 |
| P5 hover/focus inspect, click unchanged, touch read-only (R8) | T6, T7 | OrganizedActionSurface and TargetSurfaceEffects tests; walk step 5 |
| O1 descriptions authored once beside the rule | T2 | `TestBearingLoadersHaveDescriptions`; session, API and web hold no table (T5/T6 verbatim tests) |
| O2 decision, reason, contribution as data (R2) | T2 | `Answer` type; `TestRagingRuleAppliesToMeleeStrengthWeapon` asserts the change |
| O3 rule reads only the frame | T2 | `TestSneakAttackHandlerNeedsNoGameContext`; `TestSneakAttackSourceImportsNoGameContext` |
| O4 one rule function for both paths (R2) | T2, T3 | `TestRagingHandlerReadsFrameNotEventFields`; `TestSelectedRollContributionsMatchAssessedApplies`; `TestStrikeSneakAttackFromExecutionFrame` |
| O5 resolution builds the frame once per action and target | T3 | `TestStrikeBuildsOneExecutionFrameForOfferAndDamage`; `InformAttack` per target |
| O6 unknown is never false; known false stays known | T2 | `TestFramePairMissingIsUnknown`; `TestSneakAttackRuleDependsWithoutProofWhenIncomplete` |
| O7 DEPENDS at execution fails loudly (R13) | T2, T3 | `TestRagingHandlerFailsWhenRuleDepends`; `TestRagingHandlerRejectsZeroFrame`; `TestStrikeFailsWhenARuleCannotAnswer` |
| O8 ability and dice settled first (R10) | T3 | Frame from the assembled profile via `attackActionFacts`; `TestAttackActionFactsWeaponFinesse`. Martial Arts gap A1 |
| O9 relationship is the stance, not a bool or cached (R5) | T2, T3 | `PairFacts.Stance Fact[Stance]`; built per ask; `TestSneakAttackRuleNeutralAdjacentDoesNotQualify` |
| O10 toolkit owns, API transports, web renders | T5, T6 | Converter field-for-field tests; web "verbatim" tests |
| K1 information frame from the actor's knowledge | T3, T4 | `TestInformationFrameMapsNilStanceToUnknown`; attach uses only ObservedContext |
| K2 execution uses truth; information never authorizes | T3, T4 | Execution frame from room and cast; `TestEffectsAreNotSelectorMaterial`; rows attached only in Afford |
| K3 hidden state cannot change answers | T3 | `TestUnseenAllyLeavesRowsUnchanged` |
| K4 never target sheets | T3 | `TestInformAttackInputCarriesNoTargetSheets` |
| K5 partial sightings answer DEPENDS | T2, T3 | `TestSneakAttackRuleDependsWithoutProofWhenIncomplete`; `TestInformAttackSneakAttackPerTarget` (G2) |
| D1 rows on declaration, target answers on candidate (R4) | T1, T4, T5, T6 | `TestSneakAttackAnswersPerTarget`; overlay test |
| D2 no separate read, identity or freshness (R4) | T1, T4 | #374 reverted; `TestEffectsAreNotSelectorMaterial` |
| D3 availability independent (R7) | T4 | `TestUnavailableAttackStillCarriesRows` |
| D4 no rows without action content, off-turn or frozen (R12) | T4, T7 | `TestNoRowsOffTurnOrWhileFrozen`; `TestNonAttackDeclarationsCarryNoRows`; walk step 6 |
| D5 row field set | T1, T4, T5 | Proto messages; `TestEffectRowWireKeys`; `TestDeclarationToProto_CarriesEffectRows` |
| D6 unanswerable shown UNAVAILABLE; census | T2, T4 | `TestEveryConditionLoaderIsClassified`; `TestNotYetAnsweringYieldsUnavailableRow`; `TestUnansweringEffectShownUnavailable` |
| D7 wire names no feature or variant | T1 | Proto review: generic `EffectRow` only |
| S1 information does not fold | T2 | `AssessActionEffects` returns a list; no aggregate type |
| S2 reading moves no lifecycle (R9) | T2, T4 | `TestAssessingSpendsNothing`; `TestReadingRowsChangesNothing` |
| S3 frozen calculation kept (R9) | T3, T4 | `TestResumedStrikeRebuildsFrameFromTruth`; frozen Afford has no rows |
| W1 #520 walk | T7 | Walk steps 1–4 with evidence |
| Deferred, not covered: R12 rows off-turn, R14, #1929, #1934, catalogue descriptions (toolkit#1927) | — | Explicit deferrals by design or brief |

## Provider/consumer seams

| Provider | Consumer | Produced vs consumed contract | Availability | Proof |
|---|---|---|---|---|
| T1 protos `types.proto` | T5 rpg-api | `EffectRow{1..8}`, `TargetEffect{1..4}`, `Declaration.effects=21`, `TargetCandidate.effects=4`, enum names | T1 merged, then `gen/go@generated` | `TestDeclarationToProto_CarriesEffectRows` compiles against the generated `sessionpb` |
| T1 protos | T6 web | TS `Declaration.effects`, `TargetCandidate.effects`, `EffectState`, `EffectParticipation` | T1 tag in `package.json` | Web tests type-check under `ci-check` |
| T2 `contributions.Frame` / `ActionFacts` / `PairFacts` / `Stance` | T3 builders | Field names and types as listed; unknown is the zero `Fact` | T2 SHA pinned in resolution | `TestStrikeBuildsOneExecutionFrameForOfferAndDamage` |
| T2 `events.DamageChainInput.Frame`, `PostRollOfferEvent.Frame` | T3 `strike.go` (sole constructor of both, verified by grep) | Must be a valid frame or handlers return `ErrRuleCannotAnswer` | Same pin | `TestStrikeSneakAttackFromExecutionFrame`; `TestStrikeFailsWhenARuleCannotAnswer` |
| T2 `conditions.AssessActionEffects` | T3 `InformAttack` | Input {Conditions, Frame}; output Effects in persisted order; IDs unique | Same pin | `TestInformAttackSneakAttackPerTarget` |
| Existing encounter `(*Encounter).ObservedContext` (v0.111.0+, `encounter/observed_context.go`) | T3 information frame, called by T4 | `Pairs{From, To, DistanceCells float64, Stance *Stance}`; nil Stance means unknown | Resolution bumps encounter to v0.112.0 | `TestInformationFrameMapsNilStanceToUnknown` |
| T3 `resolution.InformAttack` | T4 `attachEffects` | {Observed, Actor, Attack, Targets} → {Effects, ByTarget, same IDs and order} | T3 SHA pinned in session | `TestSneakAttackAnswersPerTarget` |
| T4 `session.EffectRow` / `TargetEffect` / string enums | T5 converter | `"applies"` → `EFFECT_STATE_APPLIES`, and so on; unknown → UNSPECIFIED | T4 SHA pinned in rpg-api | `TestEffectStateToProto_UnknownIsUnspecified`; afford happy path |
| T3 execution frame | T2 handlers (Rage, Sneak Attack, Inspired) | Complete true; Advantage, Ability, Melee, WeaponPool known; pairs over placed participants | Paired release of T2 and T3 | `TestStrikeSneakAttackFromExecutionFrame`; resolution suites |
| T2 `conditions/display.go` catalog | T2 `AssessActionEffects` description | `DisplayFor(ref).Detail` non-empty for bearing loaders | Same module | `TestBearingLoadersHaveDescriptions` |

Shared files: none across tasks. T2, T3 and T4 are disjoint modules. Within T6, `actionTooltip.ts` lands before the components that consume it.

**Check findings.**
- Every consumed capability is either existing (ObservedContext, `GetConditions`, `RollContributionMetadata`, `bindOfferPress`, `onHoverEntity`) or produced by a named task.
- There are no placeholders.
- T3's `PostRollOfferEvent.Frame` requires T2's offer.go change, which is listed.
- The T2/T3 pairing risk is recorded in Sequence.

---

## Evidence for the specific questions

1. **Where the execution frame is built and how it reaches handlers.**
   - `resolution/strike.go` is the only constructor of both `DamageChainEvent` (`rollDamage` → `dnd5eEvents.NewDamageChainEvent`) and `PostRollOfferEvent` (`afterAttackChain`). `foldDamage` is the only publisher.
   - The frame is built there and carried as a field on both events.
   - Import cycle: `combat/assessment` imports `events` (`RuleBinding.Address dndevents.ConditionAddress`), so putting `assessment` types on `events` would cycle. The frame therefore lives in `contributions`, which `events` already imports on #1932 (`roll_trace.go` aliases) and which imports only `core` and `abilities`. `assessment` is deleted.
2. **Loaded rules without a bus.**
   - Session's actor comes from `session/attack.go` `loadAttackSheet` → `character.Load`, a pure load with no Apply. `(*Character).GetConditions()` (`character/character.go`) returns the loaded behaviours. `AssessAction` is pure, so no bus is needed. `no_bus_lives_here_test.go` stays valid.
   - The fan-out and census live in root `conditions`, which owns `conditionLoaders` and `displayCatalog`.
   - Frame building lives in resolution. It is the only rules-side module that requires encounter (go.mod); root has no encounter dependency; the design places frame construction in resolution.
3. **Positions.**
   - Candidates use authoritative roster positions: `offers.go` `rosterPositions(enc.Members())` plus `inRange` → `enc.Distance`.
   - ObservedContext uses sight-testimony positions for others (`seen.Position`) and `placementOf` for the observer.
   - Both use the same grid metric (`Encounter.Distance` = `canvas.GetGrid().Distance`, which execution matches with `room.GetGrid().Distance`).
   - Mismatches that matter:
     - (a) `buildTargetPreflight` admits any holding with `CurrentOn(Sight)`, but ObservedContext also requires `holding.Channel == perception.Sight`. A currently-sighted subject whose latest testimony came from a Report is a candidate with no frame facts, so its target rows are DEPENDS. That is honest, but visible.
     - (b) World NPCs are excluded from candidates (`excludeWorldNPCs`) but present in ObservedContext, which is correct for adjacency.
     - (c) ObservedContext errors on invalid testimony, while candidate building does not parse payloads (see A3).
     - (d) Testimony positions equal the roster only as of the last sight pass. Information correctly uses testimony.
4. **Free proto fields.** `Declaration` uses 1–3, 5, 7–20 and reserves 4 and 6, so 21 is free (#374's use of it never merged). `TargetCandidate` uses 1–3, so 4 is free.
5. **Web.**
   - The production dock is `OrganizedActionSurface` (mode `'organized-hud'` from `liveActionPresentation.ts`). Its `Inspection` already serves mouse/pen hover, keyboard focus, and touch long-press (`offerPress.ts`). The classic `ActionDock` `ActionTooltipCard` uses `:hover` / `:focus-within`. Both render `buildActionTooltip` (`actionTooltip.ts`).
   - Candidates render in `TargetSurface.tsx`'s target list.
   - `SessionCanvas` exposes `onHoverEntity`, but `SessionEncounterView` does not wire it.
   - Smallest honest path: extend `buildActionTooltip` and its two renderers; add hover/focus plus a read-only toggle on target-list rows; wire `onHoverEntity` for canvas mouse hover.
