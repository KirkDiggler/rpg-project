# Explicit Tabletop Death Saves Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver a selector-bearing Death Save verb that keeps Dying players in initiative, uses the shared physical d20, persists public progress, and continues the turn only after an in-bounds reveal.

**Architecture:** The root D&D rulebook derives life state and executes Death Save rules; encounter consumes a richer participation capability for clocks and defeat; resolution owns the transient rule interaction; session compiles and executes the explicit verb; protos mirror the SDK; rpg-api maps without rules; the web reuses Attack's shared-die choreography. Each inner module merges and mints a release before its consumer adopts it.

**Tech Stack:** Go 1.25, rpg-toolkit nested Go modules, protobuf/Buf, gRPC/Connect, Redis-backed rpg-api integration harness, React 19, TypeScript, Vitest, React Testing Library, Three.js/Rapier local-world dice, Playwright/browser verification.

**Spec:** `ideas/death-saves/design.md`

## Global Constraints

- Death Save is a dedicated `VerbDeathSave`, never an Activate ability or hidden TurnStart auto-roll.
- Life state is derived from HP, DeathSaveState and monster defeat; no separately persisted life-state enum.
- Dying and Stabilized retain their initiative slot; Stabilized auto-passes; Dead and defeated monsters leave initiative but remain on map/roster.
- V1 ends the dungeon run when no party member remains Conscious.
- Death Save is SlotNone, TargetNone, selector-bearing, and available only to the active eligible Dying character.
- Exactly one Death Save capacity is granted and consumed per eligible turn; Action, Bonus Action and Reaction are not spent.
- The host supplies every authoritative d20; no default randomness is permitted on the session path.
- Attack's shared local-world-die contract is reused: whole-party witnesses, same authoritative result across off-table retries, and no semantic reveal before an in-bounds settlement.
- Nonterminal/Stabilized results author `END_TURN`; natural 20 authors `KEEP_TURN`; already-applied terminal advancement authors `ALREADY_ADVANCED`.
- Death Save progress and rolls are public whole-party facts; thresholds and continuation are provider-authored, never client-derived.
- No Medicine action, magic, healing UI, in-dungeon resurrection, secret saves, capture/rescue simulation, or monster finishing-AI policy.
- No character-module extraction is assumed.
- Source/API census starts from current `origin/main`, including merged rpg-toolkit#1418 / `rulebooks/dnd5e/v0.127.1`; the intentionally dirty/behind primary toolkit checkout is never used to decide whether a symbol exists.
- V1 command ingress is serialized by the turn-based host contract: one authoritative mutating command at a time per encounter/character. This slice adds no SessionLocker/CAS subsystem; a future concurrent-writer model applies to every write verb under a separate design.
- rpg-api remains a translator/orchestrator and owns no threshold, life-state, eligibility or continuation rule.
- Integrate through PRs only. Kirk alone approves merges.
- Merge/publish inside-out. Every committed `go.mod` pins a released tag; no committed `replace`, `go.work`, `go.work.sum`, or local path remains.
- Use isolated worktrees and one writer per worktree.
- Every behavior change follows RED → observed expected failure → minimal GREEN → full affected-module verification → independent review.
- Before every PR: format, module tidy check, full module tests, lint, repository pre-commit, `git diff --check`, no staged/untracked override residue.

---

## Delivery map and merge gates

Before Task 1, create one `rpg-project` slice issue titled **“slice: explicit tabletop Death Saves on the session stack”** under Journey `rpg-project#253`, plus child delivery issues matching each PR below. Add every issue to Project 19 with Team Platform, Area The Dungeon, Kind Build, Initiative Four-player Level-3 Dungeon, and leave merge approval with Kirk. The issue bodies link `ideas/death-saves/design.md` and this plan; they do not restate or fork the design.

The work is one journey delivered through independently reviewable provider PRs:

1. **Root D&D provider PR:** two reviewed commits covering derived participation, then authoritative Death Save, damage/healing transitions, dirty persistence and once-per-turn capacity.
2. **Encounter PR:** richer participation capability, initiative behavior and pragmatic party-defeat policy.
3. **Resolution PR:** strict data-in/data-out Death Save entry.
4. **Session PR:** explicit declaration/verb, selector, Story/event and current-state projection.
5. **Proto PR:** exact SessionService transcription.
6. **API PR:** thin handler/projection and real provider acceptance.
7. **Web PR:** explicit action, progress, shared die, settlement continuation and narration.
8. **Live verification/closure:** released dependency adoption, local `dev` refresh and multiplayer journey.

Do not start a consumer PR against an unpublished inner API. Local workspaces may prove the next layer while an inner PR is under review, but the consumer commit is rewritten onto the minted release before publication.

---

### Task 1: Root D&D derived life state and participation policy

**Repository/module:** `rpg-toolkit/rulebooks/dnd5e`

**Files:**
- Create: `rulebooks/dnd5e/combat/life_state.go`
- Create: `rulebooks/dnd5e/combat/life_state_test.go`
- Modify: `rulebooks/dnd5e/combat/attack_target.go`
- Modify: `rulebooks/dnd5e/combat/attack_target_test.go`
- Modify: `docs/status.md` — replace binary-down completeness claims with the derived life-state contract

**Interfaces:**
- Consumes: `combat.CombatantKind`, `combat.IsDown`, existing `CanBeAttackTarget` compatibility surface.
- Produces:

```go
type LifeState string

const (
    LifeStateUnknown    LifeState = ""
    LifeStateConscious  LifeState = "conscious"
    LifeStateDying      LifeState = "dying"
    LifeStateStabilized LifeState = "stabilized"
    LifeStateDead       LifeState = "dead"
    LifeStateDefeated   LifeState = "defeated"
)

type LifeStateInput struct {
    Kind       CombatantKind
    Down       bool
    Stabilized bool
    Dead       bool
}

type Participation struct {
    State             LifeState
    Down              bool
    CanActNormally    bool
    NeedsDeathSave    bool
    RetainsInitiative bool
    AutoPassesTurn    bool
    AttackTarget      bool
    Conscious         bool
}

func ClassifyLifeState(input LifeStateInput) LifeState
func ParticipationFor(state LifeState) Participation
// Members contains player-character participation only. An empty party is not defeated.
type PartyState struct {
    Members []Participation
}

func PartyDefeated(party PartyState) bool
```

`ClassifyLifeState` consumes the canonical `Down` answer rather than comparing HP, preserving Undead Fortitude and later exceptions.

- [ ] **Step 1: Write the failing state-table tests**

Create a literal table covering Conscious/Dying/Stabilized/Dead characters, Conscious/Defeated monsters and unknown input. Assert the complete `Participation` literal for each state and assert `PartyDefeated` for: one conscious party member false; Dying+Stabilized only true; and an empty `PartyState` false so an empty dungeon does not become a defeat.

Expected key rows:

```go
{
    name:  "dying character keeps a player turn",
    input: LifeStateInput{Kind: CombatantKindCharacter, Down: true},
    want: Participation{
        State: LifeStateDying, Down: true, NeedsDeathSave: true,
        RetainsInitiative: true, AttackTarget: true,
    },
},
{
    name:  "stabilized character auto-passes in place",
    input: LifeStateInput{Kind: CombatantKindCharacter, Down: true, Stabilized: true},
    want: Participation{
        State: LifeStateStabilized, Down: true, RetainsInitiative: true,
        AutoPassesTurn: true, AttackTarget: true,
    },
},
{
    name:  "dead character is no longer a target or participant",
    input: LifeStateInput{Kind: CombatantKindCharacter, Down: true, Dead: true},
    want: Participation{State: LifeStateDead, Down: true},
},
```

- [ ] **Step 2: Run RED**

Run:

```bash
cd rulebooks/dnd5e
go test ./combat -run 'Test(ClassifyLifeState|ParticipationFor|PartyDefeated)' -count=1
```

Expected: compile failure because the new state/participation API does not exist.

- [ ] **Step 3: Implement the minimal rule**

Implement exact precedence:

```go
func ClassifyLifeState(in LifeStateInput) LifeState {
    switch in.Kind {
    case CombatantKindCharacter:
        switch {
        case in.Dead:
            return LifeStateDead
        case in.Down && in.Stabilized:
            return LifeStateStabilized
        case in.Down:
            return LifeStateDying
        default:
            return LifeStateConscious
        }
    case CombatantKindMonster:
        if in.Down {
            return LifeStateDefeated
        }
        return LifeStateConscious
    default:
        return LifeStateUnknown
    }
}
```

Return literal participation values from one switch. Unknown fails closed: no action, save, initiative, targetability or consciousness.

Make existing `CanBeAttackTarget(kind, down)` delegate through `ClassifyLifeState`/`ParticipationFor` for states it can express, preserving its public signature and #1418's truth table.

- [ ] **Step 4: Run GREEN and mutation checks**

Run focused tests, then deliberately verify these mutations fail before restoring them:

- Dying `RetainsInitiative=false`;
- Stabilized `AutoPassesTurn=false`;
- Dead `AttackTarget=true`;
- `PartyDefeated` treats one Conscious member as defeated.

Then run:

```bash
go test ./combat -count=1
golangci-lint run ./combat/...
```

Expected: PASS, 0 lint issues.

- [ ] **Step 5: Commit Task 1**

```bash
git add rulebooks/dnd5e/combat docs/status.md
git commit -m "feat(combat): derive dungeon life-state participation"
```

Do not push yet; Task 2 shares the root provider branch but remains a separately reviewable commit.

---

### Task 2: Authoritative Death Save and once-per-turn capacity

**Repository/module:** `rpg-toolkit/rulebooks/dnd5e`

**Files:**
- Create: `rulebooks/dnd5e/character/death_save.go`
- Create: `rulebooks/dnd5e/character/death_save_test.go`
- Modify: `rulebooks/dnd5e/character/character.go` — remove the superseded inline Death Save method bodies after moving them
- Modify: `rulebooks/dnd5e/character/character_test.go` — replace legacy no-economy/repeated-save expectations with the selector-era capacity contract
- Modify: `rulebooks/dnd5e/character/action_economy_types.go`
- Modify: `rulebooks/dnd5e/character/action_economy.go`
- Modify: `rulebooks/dnd5e/character/ledger.go`
- Modify: `rulebooks/dnd5e/character/ledger_test.go`
- Modify: `rulebooks/dnd5e/combat/capacity.go`
- Modify: `rulebooks/dnd5e/combat/action_economy.go`
- Modify: `rulebooks/dnd5e/combat/action_economy_test.go`
- Modify: `rulebooks/dnd5e/combat/action_economy_ledger.go`
- Modify: `rulebooks/dnd5e/combat/action_economy_ledger_test.go`
- Modify: `rulebooks/dnd5e/saves/death_saves.go`
- Modify: `rulebooks/dnd5e/saves/death_saves_test.go`
- Modify: `rulebooks/dnd5e/conditions/unconscious.go`
- Modify: `rulebooks/dnd5e/conditions/unconscious_test.go`
- Modify: `rulebooks/dnd5e/character/status_view.go`
- Modify: `rulebooks/dnd5e/character/status_view_test.go`

**Interfaces:**
- Consumes: Task 1 `ClassifyLifeState`, `ParticipationFor`; host `dice.Roller`; current character/action-economy persistence.
- Produces:

```go
const combat.CapacityDeathSave combat.CapacityType = "death_save"
const character.GrantedDeathSaves character.GrantedActionKey = "death_saves"

type DeathSaveOutcome string

const (
    DeathSaveOutcomeSuccess      DeathSaveOutcome = "success"
    DeathSaveOutcomeFailure      DeathSaveOutcome = "failure"
    DeathSaveOutcomeCriticalFail DeathSaveOutcome = "critical_failure"
    DeathSaveOutcomeStabilized   DeathSaveOutcome = "stabilized"
    DeathSaveOutcomeDead         DeathSaveOutcome = "dead"
    DeathSaveOutcomeRecovered    DeathSaveOutcome = "recovered"
)

type DeathSaveProgress struct {
    Successes           int
    Failures            int
    SuccessesNeeded     int
    FailuresRemaining   int
    Stabilized          bool
    Dead                bool
}

type DeathSaveContinuation string

const (
    DeathSaveContinuationEndTurn         DeathSaveContinuation = "end_turn"
    DeathSaveContinuationKeepTurn        DeathSaveContinuation = "keep_turn"
    DeathSaveContinuationAlreadyAdvanced DeathSaveContinuation = "already_advanced"
)

type MakeDeathSaveInput struct {
    Roller dice.Roller
}

type MakeDeathSaveOutput struct {
    Roll              int
    Outcome           DeathSaveOutcome
    SuccessesAdded    int
    FailuresAdded     int
    Progress          DeathSaveProgress
    HPRestored        int
    RegainedConscious bool
    Continuation      DeathSaveContinuation
}

func CanMakeDeathSave(c *Character) bool
func CostOfDeathSave(c *Character) (*combat.SpendProfile, error)
func (c *Character) MakeDeathSave(ctx context.Context, in *MakeDeathSaveInput) (*MakeDeathSaveOutput, error)

// StatusView gains explicit provider-owned current state.
type StatusView struct {
    // existing fields remain
    LifeState  combat.LifeState
    DeathSaves *DeathSaveProgress
}
```

Preserve any legacy public method compatibility only when Go callers require it; do not keep two independent implementations. The new output is the one source for session projection.

- [ ] **Step 1: Write RED for rule outcomes and persistence**

Add table tests using literal rollers for 1, 2, 9, 10, 19 and 20. Assert:

- added counts, exact typed outcome and exact typed continuation;
- remaining counts supplied by provider;
- third success Stabilized;
- third failure Dead;
- natural 20 sets HP to 1 and resets progress;
- accepted rolls mark `IsDirty()`;
- `ToData`/`Load` round-trip exact progress;
- `StatusView` reports explicit life state and optional copied progress without exposing mutable state;
- nil input, nil roller, Conscious, Stabilized, Dead and spent capacity roll zero dice and return named errors.

Add a counting roller and assert exactly one `Roll(ctx, 20)` on success. Replace the existing `CharacterDeathSaveTestSuite` cases in `character_test.go` that call `MakeDeathSave` without action economy or repeat it freely; preserve their roll-band/serialization coverage through the new Dying-turn capacity fixture rather than maintaining two contracts.

- [ ] **Step 2: Write RED for capacity round trips**

Extend the closed capacity vocabulary tests so adding `CapacityDeathSave` fails every incomplete ledger. Add a turn test:

```go
func TestDyingTurnBanksExactlyOneDeathSave(t *testing.T) {
    // HP 0, nonterminal state, stale prior economy.
    // RefreshForTurn seeds normal slots plus GrantedDeathSaves:1.
    // One accepted MakeDeathSave consumes it; a second is refused.
}
```

Assert natural 20 leaves Action/Bonus/Movement untouched for the still-active turn.

- [ ] **Step 3: Write RED for damage/healing transitions**

Drive the real `Character.ApplyDamage` entry against a target already at zero. Pin positive normal damage adding one failure, positive critical damage adding two, stabilization loss before failure, third-failure death, and dirty `ToData()` output. Pin zero applied damage—including an empty/finally-immune damage input—adding no failure and leaving stabilization unchanged. Pin Dead ignoring further damage-save failures. These tests must fail if `ApplyDamage` merely floors HP at zero or if they depend on publishing `DamageReceivedEvent`.

Pin ordinary healing refusal for Dead and healing a Dying/Stabilized character above zero clearing progress and deriving Conscious. Load a legacy persisted `UnconsciousCondition` blob beside authoritative `Data.DeathSaveState` and prove it cannot auto-roll, add another damage failure, or overwrite the authoritative progress.

- [ ] **Step 4: Run all RED cases**

```bash
cd rulebooks/dnd5e
go test ./character ./combat ./saves -run 'DeathSave|LifeState|Capacity' -count=1
```

Expected: failures for missing capacity, missing eligibility/dirty behavior, natural-20 HP still zero, and old condition auto-roll behavior.

- [ ] **Step 5: Implement the minimal capacity and operation**

Add the capacity to every closed vocabulary/ledger bridge. Seed one grant only when Task 1 says `NeedsDeathSave`. Price Death Save as capacity-only:

```go
return &combat.SpendProfile{
    Capacity: map[combat.CapacityType]int{combat.CapacityDeathSave: 1},
}, nil
```

Inside `MakeDeathSave`:

1. validate input, roller, Dying state and capacity;
2. roll once through `saves.MakeDeathSave`;
3. classify outcome from the returned rule result;
4. apply natural-20 HP and reset state;
5. author continuation from the ruled outcome (`END_TURN`, `KEEP_TURN`, or `ALREADY_ADVANCED` for Dead);
6. consume capacity;
7. set dirty;
8. return copied progress, remaining counts and typed continuation.

Implement damage-at-zero directly in the character keeper's `ApplyDamage` path: only when pre-hit HP is zero, authoritative life state is not Dead, and total **applied** damage is positive, apply the Death Save failure transition from `ApplyDamageInput.IsCritical`, mark dirty, and do not wait for `DamageReceivedEvent` (resolution intentionally does not publish that event after direct damage). Zero/immune damage performs no Death Save transition.

Do not publish a TurnStart Death Save. Make legacy `UnconsciousCondition` Death Save handlers inert so persisted old blobs cannot maintain a second progress ledger: no TurnStart roll, no damage failure, no healing-state write. Authoritative character life state/progress owns all three transitions. Update condition tests to prove the legacy blob cannot mutate them.

- [ ] **Step 6: Run GREEN and full root checks**

```bash
go test ./... -count=1
golangci-lint run ./...
go mod tidy
git diff --exit-code -- go.mod go.sum
git diff --check
```

Expected: full module PASS, 0 lint issues, tidy produces no uncommitted dependency drift.

- [ ] **Step 7: Commit Task 2 and review the root provider PR**

```bash
git add rulebooks/dnd5e
git commit -m "feat(character): make death saves authoritative per turn"
```

Request an independent review against the branch base. Fix every Critical/Important finding, rerun the full root gates, push, open one ready PR with both commits, and wait for Kirk's merge. Record the minted root D&D tag before Tasks 4 and 5 publish consumers.

---

### Task 3: Encounter participation capability and clock behavior

**Repository/module:** `rpg-toolkit/rulebooks/dnd5e/encounter`

**Files:**
- Create: `rulebooks/dnd5e/encounter/participation.go`
- Create: `rulebooks/dnd5e/encounter/participation_test.go`
- Modify: `rulebooks/dnd5e/encounter/standing.go` — migrate its call sites and retain only compatibility needed by stored-data constructors
- Modify: `rulebooks/dnd5e/encounter/encounter.go`
- Modify: `rulebooks/dnd5e/encounter/field.go`
- Modify: `rulebooks/dnd5e/encounter/data.go`
- Modify: `rulebooks/dnd5e/encounter/clocks.go`
- Modify: `rulebooks/dnd5e/encounter/trigger.go`
- Modify: `rulebooks/dnd5e/encounter/dissolve.go`
- Modify: `rulebooks/dnd5e/encounter/outcome.go`
- Modify: focused standing/defeat/killing-blow/clock tests and constructor fixtures
- Modify: `rulebooks/dnd5e/encounter/doc.go`

**Interfaces:**
- Consumes no D&D type; this module remains rulebook-neutral.
- Produces:

```go
type TurnParticipation string

const (
    TurnParticipationWait     TurnParticipation = "wait"
    TurnParticipationAutoPass TurnParticipation = "auto_pass"
    TurnParticipationRemove   TurnParticipation = "remove"
)

type MemberParticipation struct {
    Member        MemberID
    Down          bool
    Contact       bool
    Conscious     bool
    Turn          TurnParticipation
}

type ParticipationAssessment struct {
    Members       []MemberParticipation
    PartyDefeated bool
}

type Participation interface {
    Assess(members []MemberID) (*ParticipationAssessment, error)
}

// StandingWithParticipation is the migration bridge: existing constructor and
// resolution fields keep their Standing source shape while the concrete
// capability must also answer the richer assessment.
type StandingWithParticipation interface {
    Standing
    Participation
}

// NextStorySeq returns the global sequence the next successful append will use.
// It is a read and does not reserve, increment or append.
func (e *Encounter) NextStorySeq() (uint64, error)

const OutcomeDeathSave OutcomeKind = "death_save"

type DeathSaveDetail struct {
    Roll              int
    Outcome           string
    SuccessesAdded    int
    FailuresAdded     int
    Successes         int
    Failures          int
    SuccessesNeeded   int
    FailuresRemaining int
    Stabilized        bool
    Dead              bool
    Recovered         bool
    HPRestored        int
    Continuation      string
    PresentationID    string
}
```

Add `RecordInput.DeathSave *DeathSaveDetail`, valid only for `OutcomeDeathSave`; the composition validates shape/presence and preserves these primitive facts without interpreting thresholds. This is the same closed-story precedent as `AttackIdentity`.

Keep `SetupInput.Standing`, `LoadEncounterInput.Standing`, and resolution's existing `Input.Standing` source-compatible. Encounter constructors require that the supplied concrete value also implements `Participation`; absence returns a named `ErrNoParticipation`, never legacy binary behavior. Existing adapters/fakes gain `Assess`, while production session's standing seam implements both interfaces from one rulebook answer. This carries the richer API through resolution without renaming every `Standing:` field or silently defaulting.

`Wait` retains the slot and waits for a player/driver; Conscious characters and Dying characters both use it for different rulebook reasons encounter does not know. `AutoPass` retains then advances without client input. `Remove` transfers out of the bubble. `Contact` decides contact sides. `PartyDefeated` is a supplied policy answer, not a composition threshold.

- [ ] **Step 1: Write RED capability contract tests**

Pin required constructor/load capability, `ErrNoParticipation` when a Standing-only value is supplied, unknown/duplicate/foreign member refusals, stable question order, and one assessment per pass. No nil-means-everyone-active and no binary-Standing fallback.

- [ ] **Step 2: Write RED initiative scenarios**

Use a scripted fake Participation capability and real clock:

1. Dying member (`Down:true`, `Turn:Wait`) remains at the exact index and can become active.
2. Stabilized member (`Down:true`, `Turn:AutoPass`) keeps the order entry but is passed automatically when reached.
3. Returning that member to ordinary `Wait` makes the same slot player-controlled without reinsertion.
4. Dead/defeated (`Turn:Remove`) leaves order but remains map/roster.
5. A current active member becoming Remove advances exactly once.
6. `PartyDefeated:true` closes the run with stable outcome key `party_defeated` after causal down/turn beats.
7. `PartyDefeated:false` with a Dying ally and Conscious ally leaves the fight active.
8. `OutcomeDeathSave` round-trips the authoritative `Roll` and every other `DeathSaveDetail` primitive, rejects absent/mismatched detail, and never accepts caller prose.
9. `NextStorySeq` equals the next `RecordOutput.Seq`, does not mutate the log, and advances only after a successful append.

- [ ] **Step 3: Run RED**

```bash
cd rulebooks/dnd5e/encounter
go test ./... -run 'Participation|Dying|Stabilized|PartyDefeated' -count=1
```

Expected: current `noticeDown` transfers Dying/Stabilized members and existing tests demonstrate the wrong behavior.

- [ ] **Step 4: Replace binary consequences with assessment behavior**

At both constructors, retain the supplied value as `Standing` for source compatibility and extract its `Participation` half once through a checked type assertion. Store both interfaces over the same concrete capability; `ErrNoParticipation` fails construction before play.

Expose `NextStorySeq` by delegating to the record log's existing `NextSeq()` read. It does not reserve or lock. Under the ruled v1 single-command-per-encounter ingress, no second append may interleave between this read and the immediately following Record. Test no-mutation and exact next-Record equality; do not claim safety for a future concurrent-writer host.

Keep the story ledger for first Down narration, but rename/refactor `noticeDown` so consequences come from `MemberParticipation.Turn`, not `Down` alone. Existing binary `standingNow` delegates to the same assessment's `Down` fields so no second capability read or rule can disagree.

Apply in order:

1. append all new Down beats;
2. apply supplied party defeat after cause beats;
3. remove `TurnParticipationRemove` members;
4. retain Wait/AutoPass members;
5. drive auto-pass members through ordinary boundary announcement and clock advancement;
6. apply contact using `Contact` rather than `!Down`;
7. record the authoritative d20 and result through the closed `OutcomeDeathSave`/`DeathSaveDetail` shape;
8. close a supplied party defeat with stable outcome key `party_defeated`.

Add a reentrancy guard/iterative loop so consecutive stabilized participants advance without recursive stack growth, and stop at the first Wait member.

- [ ] **Step 5: Run GREEN and mutate each branch**

Verify tests fail when Dying is removed, Stabilized does not auto-pass, Dead remains, or party defeat is ignored. Then run:

```bash
go test ./... -count=1
golangci-lint run ./...
go mod tidy
git diff --exit-code -- go.mod go.sum
git diff --check
```

- [ ] **Step 6: Commit, review, publish encounter**

```bash
git add rulebooks/dnd5e/encounter
git commit -m "feat(encounter): schedule members by participation"
```

Open a ready encounter PR. Human merge mints the encounter tag. Record it for session adoption.

---

### Task 4: Resolution Death Save entry

**Repository/module:** `rpg-toolkit/rulebooks/dnd5e/resolution`

**Files:**
- Create: `rulebooks/dnd5e/resolution/death_save.go`
- Create: `rulebooks/dnd5e/resolution/death_save_test.go`
- Create: `rulebooks/dnd5e/resolution/participation.go`
- Create: `rulebooks/dnd5e/resolution/participation_test.go`
- Modify: `rulebooks/dnd5e/resolution/standing.go` — delegate its binary compatibility read to the new participation entry
- Modify: `rulebooks/dnd5e/resolution/resolve.go` — document/enforce that `Input.Standing` carries the dual `encounter.StandingWithParticipation` concrete capability
- Modify: `rulebooks/dnd5e/resolution/testrollers_test.go` — make the shared `everyoneStanding` fake answer `Assess`
- Modify: `rulebooks/dnd5e/resolution/resolve_test.go` and `standing_test.go` — migrate custom Standing test doubles to the dual capability
- Modify: `rulebooks/dnd5e/resolution/strike_test.go` — prove real normal/critical strikes against Dying/Stabilized characters update authoritative Death Save progress
- Modify: `rulebooks/dnd5e/resolution/errors.go`
- Modify: `rulebooks/dnd5e/resolution/long_rest_clone_test.go` — extend the reflection clone guard for every new mutable field
- Modify: `rulebooks/dnd5e/resolution/doc.go`
- Modify: `rulebooks/dnd5e/resolution/go.mod`/`go.sum` to the minted root and encounter tags

**Interfaces:**
- Consumes Task 2 `character.MakeDeathSaveInput/Output`, root release, and Task 3's encounter participation release.
- Produces:

```go
type ParticipantParticipation struct {
    Member        string
    Participation combat.Participation
    DeathSaves    *character.DeathSaveProgress
}

type ParticipationInput struct {
    Participants []Participant
}

type ParticipationOutput struct {
    Members []ParticipantParticipation
}

func Participation(ctx context.Context, in *ParticipationInput) (*ParticipationOutput, error)

type DeathSaveInput struct {
    Character *character.Data
    Roller    dice.Roller
}

type DeathSaveOutput struct {
    Character *character.Data
    Result    character.MakeDeathSaveOutput
}

func DeathSave(ctx context.Context, in *DeathSaveInput) (*DeathSaveOutput, error)
```

- [ ] **Step 1: Write RED lifecycle and migration tests**

Copy the proven current-`origin/main` shape of `LongRest`/`MakeCheck`: nil input, nil roller, bad participant, strict unreadable condition, one transient bus, exact teardown on every failure, input record not aliased, output record independently owned.

Assert one Dying record returns dirty persisted progress and one Conscious record refuses without rolling. For `Participation`, assert character/monster records return exact root participation/progress in input order and `Standing` returns the Down subset from that same result.

Through the real strike machine, hit a character already at zero and assert positive normal damage adds one failure, positive critical damage adds two, a Stabilized target becomes Dying before the failure, third failure becomes Dead, and the dirty character record returns on `resolution.Output`. Strike that already-Dead character again with positive damage and assert the complete Death Save state is unchanged. Add zero-damage and full-immunity regressions proving no failure/stabilization change. The tests must not install or observe a `DamageReceivedEvent` subscriber.

Pin the migration bridge: `Resolve.Input.Standing` remains source-shaped as `encounter.Standing`, but Resolve refuses a concrete value that does not also implement `encounter.Participation`; the shared and custom test fakes implement both.

- [ ] **Step 2: Run RED**

```bash
cd rulebooks/dnd5e/resolution
go test ./... -run 'DeathSave' -count=1
```

Expected: compile failure because `resolution.DeathSave` is absent.

- [ ] **Step 3: Implement the data boundary**

Use `cloneCharacterData`, `Participant.validate`, `attachAll`, `installTruth`, strict effects, required input roller, `character.MakeDeathSave`, snapshot before teardown, and `errors.Join` for operation+teardown failure.

No encounter world is installed; Death Save needs the character and authoritative d20 only. If a future effect needs the world, that effect earns a world input rather than receiving an invented empty room.

- [ ] **Step 4: Run GREEN/full module**

```bash
go test ./... -count=1
golangci-lint run ./...
go mod tidy
git diff --check
git status --short
```

Expected status contains only Death Save/participation implementation, dual-capability fixture migration, and intentional released root/encounter pins.

- [ ] **Step 5: Commit, review, publish resolution**

```bash
git add rulebooks/dnd5e/resolution
git commit -m "feat(resolution): resolve an authoritative death save"
```

Open a ready PR only after the root tag is pinned. Wait for merge/tag before publishing session.

---

### Task 5: Session participation seam, explicit declaration and verb

**Repository/module:** `rpg-toolkit/rulebooks/dnd5e/session`

**Files:**
- Create: `rulebooks/dnd5e/session/participation.go`
- Create: `rulebooks/dnd5e/session/participation_test.go`
- Create: `rulebooks/dnd5e/session/death_save.go`
- Create: `rulebooks/dnd5e/session/death_save_test.go`
- Create: `rulebooks/dnd5e/session/presentation_id.go`
- Create: `rulebooks/dnd5e/session/presentation_id_test.go`
- Create: `rulebooks/dnd5e/session/presentation_id_internal_test.go` — deterministic generator for internal-package fixtures
- Modify: `rulebooks/dnd5e/session/session.go` and `manager_test.go` — require and pin the host-supplied opaque presentation ID generator
- Modify: `rulebooks/dnd5e/session/afford.go`
- Modify: `rulebooks/dnd5e/session/offers.go`
- Modify: `rulebooks/dnd5e/session/declaration_id.go`
- Modify: `rulebooks/dnd5e/session/declaration_id_test.go`
- Modify: `rulebooks/dnd5e/session/types.go`
- Modify: `rulebooks/dnd5e/session/events.go`
- Modify: `rulebooks/dnd5e/session/events_internal_test.go`
- Modify: `rulebooks/dnd5e/session/turn.go`
- Modify: `rulebooks/dnd5e/session/standing.go` — make the existing concrete seam implement both encounter Standing and Participation from one resolution answer
- Modify: `rulebooks/dnd5e/session/rosterlookup.go` — derive binary down and rich participation lookups from the same facts
- Modify: `rulebooks/dnd5e/session/read.go`, `start.go`, and `write.go` — preserve existing Standing-shaped fields while carrying the dual concrete capability through every encounter load/setup/write scope; add the character-record save/report helper used by Death Save's ordered write
- Modify: `rulebooks/dnd5e/session/testroller_test.go` — add shared deterministic `testPresentationIDs` and make shared Standing fakes implement Assess
- Modify: `rulebooks/dnd5e/session/cmd/session-workbench/main.go` — supply a concrete presentation ID generator and rich construction capability
- Modify: every compile-reported successful `session.NewManager(&session.Config{...})` test construction to supply `PresentationIDs:testPresentationIDs`; negative constructor cases supply it except the dedicated missing-generator case
- Modify: compile-reported session test files containing custom Standing fakes so each explicitly supplies an assessment; do not add a silent adapter/default
- Modify: `rulebooks/dnd5e/session/announcer.go` docs/tests to remove automatic condition-roll claims
- Replace current death tests that require every downed player to be spliced
- Modify: `rulebooks/dnd5e/session/doc.go`, `docs/status.md`
- Modify: `rulebooks/dnd5e/session/go.mod`/`go.sum` to minted root, encounter and resolution tags

**Interfaces:**
- Consumes root `Participation`, encounter `ParticipationAssessment`, resolution `DeathSave`.
- Produces:

```go
const VerbDeathSave Verb = "death_save"

type PresentationIDGenerator interface {
    Generate() string
}

type LifeState string

const (
    LifeStateUnknown    LifeState = ""
    LifeStateConscious  LifeState = "conscious"
    LifeStateDying      LifeState = "dying"
    LifeStateStabilized LifeState = "stabilized"
    LifeStateDead       LifeState = "dead"
    LifeStateDefeated   LifeState = "defeated"
)

type DeathSaveOutcome string

const (
    DeathSaveOutcomeSuccess      DeathSaveOutcome = "success"
    DeathSaveOutcomeFailure      DeathSaveOutcome = "failure"
    DeathSaveOutcomeCriticalFail DeathSaveOutcome = "critical_failure"
    DeathSaveOutcomeStabilized   DeathSaveOutcome = "stabilized"
    DeathSaveOutcomeDead         DeathSaveOutcome = "dead"
    DeathSaveOutcomeRecovered    DeathSaveOutcome = "recovered"
)

type DeathSaveProgress struct {
    Successes         int `json:"successes"`
    Failures          int `json:"failures"`
    SuccessesNeeded   int `json:"successes_needed"`
    FailuresRemaining int `json:"failures_remaining"`
    Stabilized        bool `json:"stabilized"`
    Dead              bool `json:"dead"`
}

type DeathSaveContinuation string

const (
    DeathSaveContinuationEndTurn         DeathSaveContinuation = "end_turn"
    DeathSaveContinuationKeepTurn        DeathSaveContinuation = "keep_turn"
    DeathSaveContinuationAlreadyAdvanced DeathSaveContinuation = "already_advanced"
)

type DeathSaveRef struct {
    Name string `json:"name"`
}

// Participant retains its existing Member, Name, Kind, Standing and Active
// fields and gains the provider-owned state below.
type Participant struct {
    Member     string
    Name       string
    Kind       MemberKind
    Standing   Standing
    Active     bool
    LifeState  LifeState
    DeathSaves *DeathSaveProgress
}

// Config gains required PresentationIDs PresentationIDGenerator.

type DeathSaveInput struct {
    Session       string
    Member        string
    DeclarationID string
}

type DeathSaveOutput struct {
    Roll              int
    Outcome           DeathSaveOutcome
    SuccessesAdded    int
    FailuresAdded     int
    Successes         int
    Failures          int
    SuccessesNeeded   int
    FailuresRemaining int
    Stabilized        bool
    Dead              bool
    Recovered         bool
    HPRestored        int
    Continuation      DeathSaveContinuation
    PresentationID    string
    Seq               uint64
    Saved             SaveReport
    Delivery          DeliveryReport
}
```

Add `Declaration.DeathSave *DeathSaveRef`, present only on compiled Death Save offers. Do not overload AbilityRef.

- [ ] **Step 1: Write RED for the participation seam**

Build characters/monsters from real records and assert exact mapping:

- Dying → Down, Contact false, Conscious false, Wait;
- Stabilized → Down, false, false, AutoPass;
- Dead → Down, false, false, Remove;
- defeated monster → Down, false, false, Remove;
- Conscious → Contact/Conscious true, Wait;
- no Conscious player → `PartyDefeated:true`.

Assert repository failures remain session vocabulary and no HP threshold exists in this file. Project root life state into the session-owned `LifeState` enum and attach `DeathSaveProgress` to every Dying/Stabilized participant; assert progress absence never makes the consumer infer life state.

- [ ] **Step 2: Write RED for Afford**

In a real fight with one Conscious ally, make the active character Dying and assert exact declarations:

- available Death Save: SlotNone, TargetNone, non-empty ID, DeathSave name;
- blocked Attack, Move and Activate;
- independently available End Turn;
- no Death Save for Conscious, Stabilized, Dead, non-active or world-clock characters;
- after one accepted save, no second Death Save on the same turn;
- Dying and Stabilized visible targets remain Attack candidates;
- Dead characters and defeated monsters are omitted from Attack candidates even though they remain on the map/roster;
- execution against an omitted target is stale and mutates nothing.

- [ ] **Step 3: Write RED for execution**

Drive literal rollers across ordinary success/failure, third success, third failure and natural 20. Assert:

- `NewManager` refuses a missing PresentationIDs capability, and every unrelated successful/negative fixture supplies the shared deterministic generator so its original assertion remains the test;
- no opaque token generation or roll before selector selection;
- exactly one non-empty, bounded, wire-safe opaque token is generated per accepted command;
- exactly one roll;
- exact progress, typed outcome and typed continuation;
- character+capacity persist;
- Story typed body and response reuse one projected result value, including the authoritative roll;
- `PresentationID` is one generated opaque token shared by response/Story and all witnesses; pending global Story sequence is used only to assert `RecordOutput.Seq`, and actor/witness tokens remain equal when recipient-local cursors differ;
- whole-party delivery;
- natural 20 keeps current active member and exposes normal offers;
- third failure removes from order or reports already-advanced;
- ordinary/Stabilized reports EndTurn;
- stale/replayed selector mutates nothing;
- partial save report prevents retry.

- [ ] **Step 4: Run RED**

```bash
cd rulebooks/dnd5e/session
go test ./... -run 'DeathSave|Participation|DownedMember' -count=1
```

Expected: current binary standing splices Dying, Afford has no verb, selector rejects it, and no executor exists.

- [ ] **Step 5: Implement participation lookup and declaration**

Fetch each requested record through the existing store split by calling `resolution.Participation`, map its root facts field-for-field to encounter values and the session-owned `LifeState`, project provider-owned progress, and compute `PartyDefeated` from the requested player set only.

The existing `standingSeam` concrete implements both methods. Its `Standing` method delegates to `Assess` and selects `Down`; every `Standing:` assignment continues carrying that same dynamic concrete value through resolution and encounter. Constructor tests prove a Standing-only fake is refused, so compatibility cannot degrade into legacy behavior.

Add deterministic test generators for both test packages: `testroller_test.go` (`package session_test`) and `presentation_id_internal_test.go` (`package session`). Supply the package-appropriate helper to every successful manager construction found by `rg -n 'NewManager\(&session.Config|NewManager\(&Config' rulebooks/dnd5e/session`. In the constructor matrix, every case except “presentation IDs absent” includes it; this prevents the new required dependency from masking older missing-field assertions.

Apply root `Participation.AttackTarget` to Attack's current candidate preflight after sight/reach collection. Keep Dying/Stabilized candidates, omit Dead/Defeated candidates, and let execution enforce the same regenerated offer rather than adding a second target rule.

Compile Death Save before the generic downed early-return in `compileOffersFor`, because Dying is the one state where down blocks normal verbs and enables this one. Keep EndTurn clock-only.

Extend selector validation/canonical variant with a sealed `death_save` variant and bump selector version as the design requires.

- [ ] **Step 6: Implement the verb/save order**

Follow Attack's write scope:

1. membership/kind/clock/active gate;
2. one actor record load;
3. regenerate/select Death Save;
4. generate and validate one opaque `PresentationID` through `Config.PresentationIDs`;
5. call resolution with manager dice;
6. map the root typed outcome/continuation once into a session-owned typed result carrying that token;
7. save returned character first;
8. read `pendingGlobalSeq := scope.enc.NextStorySeq()` inside the one active v1 write command for an internal append assertion only;
9. place the opaque token plus Roll in `DeathSaveDetail`;
10. record that same result and require `RecordOutput.Seq == pendingGlobalSeq` before continuing;
11. let participation consequences update clock/outcome;
12. assert `ALREADY_ADVANCED` actually advanced/closed and the other continuations retained the expected active slot;
13. commit and return the same typed result, projecting response `Seq` separately through the existing recipient-local sequence mapper.

The session mapping preserves enum meaning field-for-field; it does not recompute continuation from the d20. `PresentationID` is an opaque generated token and contains no sequence. `pendingGlobalSeq` remains inside the toolkit as a Record-order assertion. Response/event `Seq` remains recipient-local and is passed separately to the presentation service's numeric authority field; actor and witnesses correlate game/dice facts only by the shared opaque token.

- [ ] **Step 7: Run GREEN/full session gates**

```bash
go test ./... -count=1
golangci-lint run ./...
go mod tidy
(cd ../../.. && ./scripts/verify.sh rulebooks/dnd5e/session)
git diff --check
git status --short
```

Expected status contains only the session implementation and the intentional released root/encounter/resolution pins.

- [ ] **Step 8: Commit, review, publish session**

```bash
git add rulebooks/dnd5e/session
git commit -m "feat(session): offer the dying player a death save"
```

Independent review must explicitly compare the design's state table, one-roll trust boundary, save ordering and continuation outcomes. Merge/tag before API adoption.

---

### Task 6: Session proto contract

**Repository:** `rpg-api-protos`

**Files:**
- Modify: `dnd5e/api/session/v1alpha1/types.proto`
- Modify: `dnd5e/api/session/v1alpha1/service.proto`
- Modify: `dnd5e/api/session/v1alpha1/events.proto`
- Modify: `dnd5e/api/v1alpha2/encounter/types.proto` — owner CharacterData Death Save progress
- Modify: `docs/architecture/components/session-service.md`
- Modify: `docs/architecture/components/character-service.md`
- Modify: `docs/status.md`
- Generated files are regenerated by `buf generate` and follow repository policy; do not hand-edit them.

**Interfaces:**
- Mirrors the released session SDK exactly.
- Adds `VERB_DEATH_SAVE`, session-owned `LifeState`, `DeathSaveRef`, `DeathSaveProgress`, `DeathSaveOutcome`, `DeathSaveContinuation`, request/response RPC, `EVENT_KIND_DEATH_SAVE_ROLLED`, and typed event body.

- [ ] **Step 1: Write the proto changes from the released SDK**

Use additive tags. Reserve nothing unless an old field is actually removed. Add comments stating:

- whole-party visibility;
- provider-derived remaining counts;
- same-result presentation retries;
- declaration ID requirement;
- no client threshold derivation;
- continuation semantics;
- `presentation_id` is an opaque generated shared token and contains no Story sequence;
- the presentation service's numeric `authority_seq` is supplied separately from the actor's recipient-local response sequence and is never parsed from `presentation_id`.

`DeathSaveProgress` contains successes, failures, successes_needed, failures_remaining, stabilized and dead. `Participant.life_state` is always explicit and never inferred from progress presence; `Participant.death_saves` is present for Dying/Stabilized progress and absent when not applicable. Owner CharacterData carries the same explicit life state and optional progress. The typed event and response both carry the authoritative roll, typed outcome/continuation, and identical opaque presentation token.

- [ ] **Step 2: Run RED consumer compile before generation**

Update or add repository contract tests/fixtures that reference the new generated symbols, then run the repository test/generation check expected to fail because generated code is stale.

- [ ] **Step 3: Generate and validate**

```bash
buf format -w
buf lint
buf format --diff --exit-code
buf breaking --against 'https://github.com/KirkDiggler/rpg-api-protos.git#branch=main'
buf generate
git diff --check
```

Expected: lint/format/generate PASS. Breaking check has no unapproved finding; enum/message/RPC additions are additive.

- [ ] **Step 4: Inspect generated contract**

Verify generated Go/TS expose exact enum values, optional progress presence, oneof event body and service method. Do not add bespoke tests that merely test protobuf generation.

- [ ] **Step 5: Commit/review/publish protos**

```bash
git add dnd5e docs gen
git commit -m "feat(session): add explicit death save contract"
```

Open ready PR, wait for Kirk merge and record the minted proto version/commit.

---

### Task 7: Thin API handler and released-provider acceptance

**Repository:** `rpg-api`

**Files:**
- Create: `internal/handlers/dnd5e/session/v1alpha1/death_save.go`
- Create: `internal/handlers/dnd5e/session/v1alpha1/death_save_test.go`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/handler.go`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/convert.go`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/convert_test.go`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/errors.go`/tests only for new SDK sentinels
- Regenerate: manager mock after SDK interface grows
- Create: `internal/integration/session/death_save_acceptance_test.go`
- Modify: `internal/handlers/dnd5e/v2/character/character_data.go` and `handler_test.go` for owner progress
- Modify: `internal/orchestrators/session/orchestrator.go` and tests — supply `idgen.NewUUID("presentation-")` in production and deterministic generators in tests
- Modify: `internal/orchestrators/lobby/start_encounter_session_stack_test.go` — supply a deterministic generator to direct SDK manager construction
- Modify: `internal/sessionworld/sessionworld.go`
- Modify: `internal/sessionworld/sessionworld_test.go`
- Modify: `internal/integration/session/acceptance_test.go`
- Modify: `go.mod`, `go.sum` to released root/encounter/session/proto versions
- Modify: `docs/status.md`

**Interfaces:**
- Consumes released toolkit Session Manager `DeathSave` and released protos.
- Produces a gRPC handler that binds authenticated member, maps fields and statuses, and never decides rules.

- [ ] **Step 1: Write RED handler tests**

Pin:

- unauthenticated request rejected before manager call;
- authenticated member replaces/validates client member according to existing ownership pattern;
- exact request mapping including declaration ID;
- field-for-field response mapping, including authoritative roll and typed outcome/continuation;
- Turn Participant and owner CharacterData map explicit life state plus optional progress without HP/progress inference;
- stale/not-turn/not-Dying/spent errors map to FailedPrecondition;
- missing request fields map consistently;
- no API code compares roll, HP, success/failure counts or threshold 3;
- toolkit manager construction receives an explicit opaque ID generator; production uses `idgen.NewUUID("presentation-")`, tests use deterministic `idgen.NewSequential("presentation")`;
- generated tokens remain separate from recipient-local numeric authority sequence and contain no Story sequence;
- production `sessionworld.nobodyDown` and integration `allStanding` implement both `Standing` and `Assess`, projecting every supplied construction-time member as Conscious/Contact/Wait with `PartyDefeated:false`;
- world compilation succeeds after the encounter pin, and a deliberately Standing-only adapter fails with `ErrNoParticipation`.

- [ ] **Step 2: Write RED real-provider acceptance**

Use the integration harness and released session module:

1. create at least two player characters and a damaging monster/fixture;
2. reduce one player to zero while another remains Conscious;
3. assert the Dying player remains in order with explicit Dying life state and exact public progress;
4. Afford returns exact Death Save declaration;
5. execute with scripted ordinary success;
6. assert response, persisted CharacterData, spent capacity, Story event and whole-party delivery;
7. assert a replay is stale;
8. drive natural 20, third success and third failure subtests from seeded progress;
9. execute positive normal and critical strikes against Dying/Stabilized targets and assert persisted failures/life-state transitions through the session Attack path; execute zero/immune damage and attempt a subsequent positive Attack against an already-Dead target; assert the latter is stale before resolution and neither path moves progress/life state;
10. assert Dying/Stabilized remain Attack targets while Dead/Defeated do not;
11. assert no-conscious-party defeat.

The test must fail on the old API pins before bumping them.

- [ ] **Step 3: Run RED against old pins**

```bash
go test ./internal/integration/session -run TestAcceptance_DeathSave -count=1
```

Expected: compile/provider failure because old pins have no verb.

- [ ] **Step 4: Adopt releases and implement thin mapping**

Run `go get` for exact minted root/encounter/session/proto versions. Regenerate mocks through the repository generator. Implement the handler as one manager call plus conversion. Extend character projection from toolkit status/progress; do not read raw DeathSaveState in the handler when a provider view exists.

Wire the SDK's required `PresentationIDs` capability in the session orchestrator: production constructs `idgen.NewUUID("presentation-")`; tests and direct SDK constructions use deterministic sequential generators. The API never derives the token from Story sequence and never accepts one from the client.

Before running any dungeon/session acceptance, migrate the two direct encounter consumers: add `Assess` to production `internal/sessionworld.nobodyDown` and integration `allStanding`, returning one Conscious/Contact/Wait row per requested member and `PartyDefeated:false`. Pin those exact adapters in their unit/integration tests. This migration lands in the same API PR as the encounter/session bump so no deployed build can construct a world with a Standing-only capability.

- [ ] **Step 5: Run focused GREEN**

```bash
go test ./internal/sessionworld ./internal/handlers/dnd5e/session/v1alpha1 ./internal/handlers/dnd5e/v2/character -count=1
go test ./internal/integration/session -run 'TestAcceptance_DeathSave|TestServer' -count=1
```

- [ ] **Step 6: Run full API gates**

```bash
go test ./... -count=1
golangci-lint run ./...
./scripts/verify-release-pin.sh
make pre-commit
git diff --check
```

- [ ] **Step 7: Commit/review/publish API**

```bash
git add .
git commit -m "feat(session): expose the death save verb"
```

Open a ready PR to `dev`; Kirk alone merges. CI must be green before web adopts the deployed API.

---

### Task 8: Web declaration, progress and command path

**Repository:** `rpg-dnd5e-web`

**Files:**
- Create: `src/api/useSessionDeathSave.ts`
- Create: `src/api/useSessionDeathSave.test.ts`
- Modify: `src/components/session/combat-experience/ActionDock.tsx`
- Modify: `src/components/session/combat-experience/actionTooltip.ts`
- Modify: `src/components/session/combat-experience/useSessionCombatExperience.ts`
- Modify: `src/components/session/combat-experience/selection.ts` — recognize the selector-bearing no-target Death Save declaration
- Create/modify focused ActionDock/combat-experience tests
- Modify: `src/components/session/combat-experience/CombatExperience.tsx`
- Modify: `src/components/session/combat-experience/CombatExperience.test.tsx`
- Modify: `package.json` lockfile only for the released proto package version; add no new UI dependency

**Interfaces:**
- Consumes `VERB_DEATH_SAVE`, declaration DeathSaveRef, RPC response and public progress.
- Produces an explicit no-target button and authority-safe RPC flow.

- [ ] **Step 1: Write RED for declaration rendering**

Render a Dying active participant with exact declarations. Assert:

- Death Save button label/icon and SlotNone badge;
- no HP/progress-based local synthesis when declaration absent;
- unavailable/stale authority disables dispatch;
- no target mode is armed;
- whole-party progress/pips use provider counts and remaining values.

- [ ] **Step 2: Write RED for command/recovery**

Assert one click sends exact session/member/declaration ID, fences duplicate in-flight clicks, invalidates authority after response, never retries ambiguous mutation, and stores the provider continuation for release handling.

- [ ] **Step 3: Run RED**

```bash
npm run test:run -- \
  src/api/useSessionDeathSave.test.ts \
  src/components/session/combat-experience/ActionDock*.test.tsx \
  src/components/session/SessionEncounterView.test.tsx
```

Expected: generated enum/hook/dispatch cases missing.

- [ ] **Step 4: Implement minimal command UI**

Add explicit verb branches to label/icon/executable allowlist and `useSessionCombatExperience`. Do not treat unknown verbs as Move. Keep one generic button component; add no DeathSave-only parallel dock.

The hook calls the dedicated RPC and returns exact response. No comparison to 1, 10, 20 or 3 is allowed in command/UI selection code.

- [ ] **Step 5: Run GREEN**

Run the focused command/UI tests and typecheck. Commit separately from dice presentation so review can reject command semantics without rejecting physics reuse.

```bash
git add src/api src/components/session package.json package-lock.json
git commit -m "feat(session): render and invoke death saves"
```

---

### Task 9: Shared physical die, settlement reveal and automatic continuation

**Repository:** `rpg-dnd5e-web`

**Files:**
- Modify: `src/components/session/combat-experience/presentation.ts`
- Modify: `src/components/session/combat-experience/presentation.test.ts`
- Modify: `src/components/session/combat-experience/useCombatPresentation.ts`
- Modify: `src/components/session/combat-experience/useCombatPresentation.test.tsx`
- Modify: `src/components/session/SessionEncounterView.tsx`
- Modify: `src/components/session/SessionEncounterView.test.tsx`
- Modify: `src/components/session/local-world-die/localWorldDieAuthority.ts`
- Modify: `src/components/session/local-world-die/localWorldDieWitnessPlan.ts`
- Modify: `src/components/session/local-world-die/localWorldDieWitnessPlan.test.ts`
- Modify: `src/components/session/combat-experience/story.ts`
- Modify: `src/components/session/combat-experience/story.test.ts`
- Reuse unchanged: `LocalWorldDieLayer`, Rapier pre-simulation, PublishDiceThrow and StreamDiceThrows

**Interfaces:**
- Consumes Death Save response/event presentation ID, authoritative roll, outcome/progress and continuation.
- Produces one generalized d20 presentation state machine supporting Attack and Death Save.

- [ ] **Step 1: Write RED reducer tests**

Add Death Save facts parallel to Attack facts. Assert:

- response/event truth ingested immediately but semantic Story hidden;
- response and every recipient event carry the identical opaque presentation token even when their local `seq` values differ;
- the token contains no numeric Story sequence and no helper parses authority sequence from it;
- roller role belongs to the Dying member;
- witnesses receive shared plan;
- off-table terminal remains armed/retryable and reveals nothing;
- retry attempt retains presentation ID/result and issues no Death Save RPC;
- in-bounds settlement reveals once;
- conflicting/duplicate release is ignored/diagnosed exactly as Attack;
- event-before-release and release-before-event converge.

- [ ] **Step 2: Write RED continuation tests**

For settled responses:

- `END_TURN` calls EndTurn exactly once after settlement, never before;
- `KEEP_TURN` calls no EndTurn and refreshes CharacterData/Turn/Afford;
- `ALREADY_ADVANCED` calls no mutation and refreshes authority;
- failed automatic EndTurn preserves error, reconciles, and cannot reroll;
- remount after accepted save shows spent state/EndTurn, not another die.

- [ ] **Step 3: Generalize presentation authority**

Replace Attack-only authority fields with a typed interaction authority:

```ts
type DiceInteractionKind = 'attack' | 'death-save';
interface DiceAuthority {
  kind: DiceInteractionKind;
  roller: string;
  presentationId: string;
  authoritySeq: bigint;
  target?: string;
}
```

Keep Attack adapter compatibility and add `deathSaveResponseFact`. For Death Save, `presentationId` comes verbatim from the provider token and `authoritySeq` comes separately from the actor's recipient-local response sequence. Witness matching uses the token and never compares its own local event sequence to the roller's authority sequence. Do not parse the token and do not fork a second reducer.

- [ ] **Step 4: Reuse exact local-world die mechanics**

Feed Death Save's authoritative d20 into the same request, pre-simulation, authoritative face, plan publication, witness inbox and terminal handling. Send the opaque token as `presentation_id` and the actor-local response sequence as the separate numeric `authority_seq`. Rename only helpers whose Attack-specific name blocks reuse; keep physics schema and validation unchanged.

- [ ] **Step 5: Add table-facing narration**

Choose copy from provider `outcome` and provider remaining counts. Example assertions:

```text
Death save! 2 successes — 1 to stabilize.
Failure. 2 down — one more means death.
Natural 20! Back on your feet with 1 HP.
Three successes — stabilized.
Three failures — dead.
```

No web arithmetic computes thresholds or continuation.

- [ ] **Step 6: Run focused and full web gates**

```bash
npm run test:run -- src/components/session src/api/useSessionDeathSave.test.ts
npm run typecheck
npm run lint
npm run test:run
npm run build
git diff --check
```

- [ ] **Step 7: Commit/review/publish web**

```bash
git add src package.json package-lock.json
git commit -m "feat(session): roll death saves at the shared table"
```

Request independent logic review and visual/browser review. Open a ready PR to `dev`; Kirk alone merges.

---

### Task 10: Cross-stack acceptance, live proof and issue closure

**Repositories:** `rpg-toolkit`, `rpg-api-protos`, `rpg-api`, `rpg-dnd5e-web`, `rpg-deployment`, `rpg-project`

**Files:**
- Modify acceptance/docs only when the live run reveals a contract fact not already recorded
- Update `ideas/death-saves/implementation.md` after all merges with actual tags, commits, deviations and observed results
- Update `ideas/death-saves/CLAUDE.md` status from approved design to implemented only after proof
- Keep `ideas/death-saves/design.md` unchanged unless a human approves a design revision

**Interfaces:**
- Consumes merged releases and deployed `dev` images.
- Produces final evidence and Project 19 completion.

- [ ] **Step 1: Verify exact releases and deployments**

Record root/encounter/resolution/session tags, proto version, API merge/image revision and web merge revision. Pull/recreate only required local services. Verify healthy container labels and exact web `origin/dev` head.

- [ ] **Step 2: Run real multiplayer journey**

Use at least two real player identities in the same session:

1. one remains Conscious;
2. the other reaches zero;
3. Turn still lists the Dying character at the original slot;
4. Afford offers Death Save and blocks ordinary verbs;
5. pick up and throw the shared d20;
6. deliberately throw off-table and verify no reveal/progress/continuation;
7. retry and settle in bounds;
8. verify both clients see the same die/result/progress;
9. verify nonterminal settlement advances once;
10. seed/repeat controlled variants for natural 20, third success and third failure;
11. verify Stabilized auto-pass, Dead removal/map retention, and no-conscious-party defeat.

- [ ] **Step 3: Inspect persisted truth**

Verify character DeathSaveState/capacity, encounter clock/order, Story typed event sequence, whole-party delivery, and absence of duplicate mutations across off-table retry/reconnect.

- [ ] **Step 4: Run repository final gates**

Run each owning repository's full tests/lint/pre-commit/release-pin commands at the exact merged revision. Capture command outputs and residual risks.

- [ ] **Step 5: Write implementation record**

Create `ideas/death-saves/implementation.md` with:

- released tags/commits;
- PR/issue links;
- RED/GREEN evidence;
- final state/verb/wire shapes;
- live screenshots/transcript references;
- any approved deviation from design;
- deferred scope unchanged;
- cleanup performed.

- [ ] **Step 6: Close Project work and clean owned workspaces**

Post signed completion evidence, close only the slice issues actually completed, mark their Project 19 items Done, preserve unrelated active continuity, remove clean merged worktrees/local branches/temp overrides, and leave runtime services only when another active task owns them.

---

## Plan self-review checklist

Before execution begins, the coordinator verifies:

- [x] Every durable design decision 1–14 maps to at least one task and test.
- [x] No task assumes character-module extraction.
- [x] The old automatic TurnStart roll is explicitly retired and tested absent.
- [x] Dying retention, Stabilized auto-pass and Dead removal are distinct tests.
- [x] Party defeat is provider-authored and replaceable, not hardcoded from HP in encounter/session/API/web.
- [x] One-roll capacity and ambiguous-response recovery are covered at root, session, API and web.
- [x] Off-table retry is presentation-only and retains the authoritative result.
- [x] Continuation occurs after settlement and is provider-authored.
- [x] Whole-party progress has both durable Story and current-state reconnect surfaces.
- [x] Damage-at-zero and ordinary healing transitions are included without adding Medicine/magic UI.
- [x] Every nested module consumer waits for a minted provider tag.
- [x] Every PR has a fresh independent review and human merge gate.
- [x] The authoritative d20 is persisted in `DeathSaveDetail`, response and whole-party event.
- [x] Typed outcome/continuation originate in the rulebook and are threaded through resolution/session/proto/API without d20 inference.
- [x] Session/proto participants project explicit life state independently from optional progress.
- [x] Attack compilation/execution consume provider-owned targetability for Dying/Stabilized versus Dead/Defeated.
- [x] Both map-backed and fielded action-economy ledgers are included for `CapacityDeathSave`.
- [x] Encounter's richer assessment migrates through a required dual Standing+Participation concrete capability; resolution/session fields remain source-compatible and reject Standing-only values.
- [x] Task file/API baselines were verified against current `origin/main` and merged rpg-toolkit#1418, not the dirty/behind primary checkout.
- [x] The plan makes no nonexistent-lock claim: global-sequence precomputation relies on the explicit v1 single-command ingress ruling, while lock/CAS is deferred to a general concurrency design.
- [x] Direct rpg-api encounter constructors (`sessionworld.nobodyDown`, integration `allStanding`) migrate to the dual capability in the same PR as provider adoption.
- [x] Presentation identity is a host-generated opaque token persisted before Record; pending global Story sequence is used only for an internal `RecordOutput.Seq` assertion and never crosses the session boundary.
- [x] Dice presentation receives opaque `presentation_id` and recipient-local numeric `authority_seq` as separate values; no parser extracts one from the other.
- [x] Positive damage-at-zero is applied directly by `Character.ApplyDamage` and proven through resolution Strike plus session Attack; zero/immune damage and Dead are no-ops, and no path depends on the deliberately absent `DamageReceivedEvent` publication.
- [x] Legacy `UnconsciousCondition` Death Save handlers are inert and cannot create a second progress ledger.
- [x] Already-Dead positive damage is a no-op proven through the real resolution Strike path; session rejects the Dead target before resolution.
- [x] Legacy `character_test.go` Death Save cases migrate from unrestricted/no-economy calls to the one-capacity-per-Dying-turn contract.
- [x] Both external and internal session test packages provide deterministic presentation-ID generators to every successful manager construction.
