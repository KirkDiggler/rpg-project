# Server-Authored Roll Traces Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Preserve reusable provider-authored dice, reroll, modifier, and total facts through domain-specific damage and healing results, proving the contract end to end with Great Weapon Fighting and Second Wind.

**Architecture:** D&D 5e keeps separate domain chains and durable result bodies. Root rulebook events introduce shared roll primitives; domain providers populate them; encounter and Session persist neutral mirrors; proto/API transport them; web uses one shared formatter inside domain-specific Story and Debug presentation.

**Tech Stack:** Go 1.24.1, rpg-toolkit multi-module Go workspace, protobuf/buf, Connect RPC, TypeScript 5.8, React 19, Vitest 4, Docker Compose local lab.

**Spec:** `docs/superpowers/specs/2026-09-03-server-authored-roll-traces-design.md`

## Global Constraints

- Toolkit owns all dice, reroll, modifier, source, total, clamp, and D&D rule facts.
- Keep separate attack, damage, healing, save, check, initiative, and Hit Dice domain results; do not add a universal result event or arithmetic AST.
- The reusable root contract is `RollSource`, `DiceReroll`, `DiceTrace`, `RollComponent`, and `RollCalculation` as defined in Task 1.
- A zero modifier is present when `RollComponent.Modifier != nil && *Modifier == 0`; absence means it did not participate.
- Every slice and map crossing an event/persistence boundary is deep-cloned.
- Session and rpg-api copy fields only. They never parse provider refs, calculate subtotals/totals, or reconstruct missing traces.
- New persisted events use the trace representation only. Legacy scalar fields may remain solely for decoding pre-change records and must be rejected when mixed with a new trace.
- Existing aggregate-only persisted events remain readable with their existing fallback presentation.
- New malformed traces fail before durable append; no partial activation transaction may be written.
- Current full-roster audience behavior is unchanged; rpg-toolkit#940 remains separate.
- 3D damage/healing dice and catch-up animation are out of scope.
- Second Wind spends its own use, never Hit Dice.
- Fold rpg-toolkit#1427 into the root/resolution delivery: Second Wind consumes the scoped interaction roller, and tests stop replacing `crypto/rand.Reader`.
- Toolkit branches target `main`; API and web feature branches target `dev`; proto source targets `main` and consumers pin the immutable generated-branch commit.
- Develop toolkit modules outside-in with local overrides if needed, publish inside-out, and never commit `replace` directives or `go.work` files.
- Each behavioral PR receives fresh implementation, Terra review, public GLM review, at most one bounded final fix wave, scoped re-review, green CI, and resolved threads.
- Before implementation, create one linked behavioral issue per task in the owning repository under rpg-project#361. Do not use task prose as a substitute for issue state.

## Contract fixed by this plan

Root D&D event types use `*core.Ref`; encounter/Session mirrors and proto use canonical ref strings:

```go
type RollSource struct {
    Ref   *core.Ref
    Name  string
    Label string
}

type DiceReroll struct {
    DieIndex int
    Before   int
    After    int
    Source   RollSource
}

type DiceTrace struct {
    Notation      string
    DieSize       int
    OriginalRolls []int
    Rerolls       []DiceReroll
    FinalRolls    []int
    KeptIndices   []int // empty means all final rolls
    Subtotal      int
}

type RollComponent struct {
    Source   RollSource
    Dice     *DiceTrace
    Modifier *int // pointer preserves a participating zero
}

type RollCalculation struct {
    Components []RollComponent
    Total      int
}
```

`RollComponent` may carry dice, a modifier, or both. A calculation component must carry at least one. Domain-only transformations remain outside it: `DamageComponent` retains damage type, critical metadata/properties, and multiplier; `HealingApplied` retains requested/applied/HP facts.

---

### Task 1: Root D&D reusable roll-trace primitives

**Repository:** `rpg-toolkit` root D&D module<br>
**Files:**
- Create: `rulebooks/dnd5e/events/roll_trace.go`
- Create: `rulebooks/dnd5e/events/roll_trace_test.go`
- Modify: `rulebooks/dnd5e/events/doc.go` if its public surface list requires the new contract

**Interfaces:**
- Consumes: `core.Ref`, canonical dice notation accepted by `dice.ParseNotation`.
- Produces: the five public types in “Contract fixed by this plan”; `CloneRollCalculation(*RollCalculation) *RollCalculation`; `ValidateRollCalculation(*RollCalculation) error`.

- [ ] **Step 1: Write failing contract and validation tests**

Cover exact deep equality for a valid calculation containing `2d6`, original `[1,5]`, a sourced die-0 `1→4` reroll, final `[4,5]`, subtotal `9`, and `+3 Strength`, total `12`. Mutate the input after cloning and prove no ref/slice aliases remain. Add table cases for invalid notation, die size, face range, cardinality, reroll index, reroll `before`, reroll `after` propagation, duplicate/out-of-range kept index, wrong subtotal, missing source ref/name, empty component, and wrong total. Include a valid participating zero modifier.

```go
func intPtr(v int) *int { return &v }

zero := 0
calc := &RollCalculation{Components: []RollComponent{
    {
        Source: RollSource{Ref: refs.Weapons.Greatsword(), Name: "Greatsword"},
        Dice: &DiceTrace{
            Notation: "2d6", DieSize: 6,
            OriginalRolls: []int{1, 5},
            Rerolls: []DiceReroll{{
                DieIndex: 0, Before: 1, After: 4,
                Source: RollSource{
                    Ref: refs.Conditions.FightingStyleGreatWeaponFighting(),
                    Name: "Great Weapon Fighting",
                },
            }},
            FinalRolls: []int{4, 5}, Subtotal: 9,
        },
    },
    {
        Source: RollSource{Ref: refs.Abilities.Strength(), Name: "Strength"},
        Modifier: intPtr(3),
    },
    {
        Source: RollSource{Ref: refs.Abilities.Dexterity(), Name: "Dexterity"},
        Modifier: &zero,
    },
}, Total: 12}
```

- [ ] **Step 2: Run the focused test and confirm RED**

Run from `rulebooks/dnd5e`:

```bash
go test ./events -run 'Test(RollCalculation|CloneRollCalculation)' -count=1
```

Expected: compile failure because the types/functions do not exist.

- [ ] **Step 3: Implement the primitives, clone, and validator**

`ValidateRollCalculation` must replay only structural trace transitions: parse notation, check every face against `DieSize`, apply ordered rerolls against a scratch copy, compare final rolls, select all final indices when `KeptIndices` is empty, sum the kept faces into `Subtotal`, then add every present modifier into `Total`. It does not decide whether a D&D rule was eligible to reroll.

`CloneRollCalculation` returns nil for nil and clones refs, component slices, face/index slices, and reroll slices/sources.

- [ ] **Step 4: Run focused and root-module verification**

```bash
go test ./events -count=1
go test ./... -count=1
golangci-lint run ./...
git diff --check
```

Expected: all pass from `rulebooks/dnd5e`; no other module is tidied or edited.

- [ ] **Step 5: Commit, review, merge, and record the CI-minted root tag**

```bash
git add rulebooks/dnd5e/events/roll_trace.go \
        rulebooks/dnd5e/events/roll_trace_test.go \
        rulebooks/dnd5e/events/doc.go
git commit -m "feat(dnd5e): define reusable roll traces"
```

Open the linked root-module PR, complete the required review cycle, merge, and record the exact `rulebooks/dnd5e/v…` tag emitted by CI for Task 2.

---

### Task 2: Root damage chains retain generic dice and sourced GWF rerolls

**Repository:** `rpg-toolkit` root D&D module<br>
**Files:**
- Modify: `rulebooks/dnd5e/events/events.go`
- Modify: `rulebooks/dnd5e/combat/final_damage.go`
- Modify: `rulebooks/dnd5e/combat/composable_damage_test.go`
- Modify: `rulebooks/dnd5e/combat/final_damage_test.go`
- Modify: `rulebooks/dnd5e/conditions/primary_damage.go`
- Modify: `rulebooks/dnd5e/conditions/fighting_style_great_weapon_fighting.go`
- Modify: `rulebooks/dnd5e/conditions/fighting_style_great_weapon_fighting_test.go`
- Modify: root-module condition producers/tests that construct `DamageComponent`: `brutal_critical*`, `fighting_style_dueling*`, `fighting_style_two_weapon_fighting*`, `martial_arts*`, `raging*`, `sneak_attack*`
- Modify: root-module monster-trait consumers/tests that construct/read `DamageComponent`

**Interfaces:**
- Consumes: Task 1 roll types and validation.
- Produces: `events.DamageComponent{Source DamageSourceType, Roll RollComponent, DamageType damage.Type, Properties []damage.Property, IsCritical bool, Multiplier *float64}`; canonical GWF reroll source.

- [ ] **Step 1: Make GWF and final-damage tests demand the new shape**

Pin a weapon component whose `Roll.Dice` starts `[1,2,6]`, GWF rerolls to `[5,4,6]`, and each reroll carries ref `dnd5e:conditions:fighting_style_great_weapon_fighting` and name `Great Weapon Fighting`. Assert subtotal changes from `9` to `15`. Pin a modifier-only component with `Modifier=&3` and prove final damage is `18` without consumers summing face arrays.

- [ ] **Step 2: Run focused tests and confirm RED**

```bash
go test ./conditions ./combat -run 'Test.*(GreatWeapon|FinalDamage|ComposableDamage)' -count=1
```

Expected: compile failures against the old flat `DamageComponent` fields.

- [ ] **Step 3: Migrate root damage components**

Replace the roll-only flat fields (`SourceRef`, `Dice`, `OriginalDiceRolls`, `FinalDiceRolls`, `Rerolls`, `FlatBonus`) with `Roll RollComponent`. Every producer supplies a provider-owned source ref/name. A modifier-only component uses `Roll.Modifier`; dice components use `Roll.Dice`; multipliers retain their domain field and source identity.

GWF mutates only `component.Roll.Dice`: clone final faces, append ordered `DiceReroll` with the canonical condition source, update the indexed final face, and update the authoritative subtotal. It must never rewrite original faces.

- [ ] **Step 4: Run root module verification and source scans**

```bash
go test ./... -count=1
golangci-lint run ./...
! rg 'Reason:\s*"great_weapon_fighting"|OriginalDiceRolls|FinalDiceRolls' \
  events combat conditions monstertraits --glob '*.go'
git diff --check
```

Expected: all root tests pass; the removed representation is absent from root-module production code.

- [ ] **Step 5: Commit, review, merge, and record the new root tag**

```bash
git add rulebooks/dnd5e/events rulebooks/dnd5e/combat \
        rulebooks/dnd5e/conditions rulebooks/dnd5e/monstertraits
git commit -m "feat(dnd5e): preserve sourced damage roll traces"
```

Complete review/merge and record the CI-minted root tag.

---

### Task 3: Root healing chain carries Second Wind calculation and scoped dice

**Repository:** `rpg-toolkit` root D&D module<br>
**Files:**
- Modify: `rulebooks/dnd5e/features/types.go`
- Modify: `rulebooks/dnd5e/features/second_wind.go`
- Modify: `rulebooks/dnd5e/features/second_wind_test.go`
- Modify: `rulebooks/dnd5e/character/action_economy_types.go`
- Modify: `rulebooks/dnd5e/character/action_economy.go`
- Modify: `rulebooks/dnd5e/character/character.go`
- Modify: `rulebooks/dnd5e/character/sheet_keeper_test.go`
- Modify: `rulebooks/dnd5e/monster/monster.go`
- Modify: `rulebooks/dnd5e/monster/load_test.go`
- Modify: `rulebooks/dnd5e/events/events.go`
- Modify: healing publisher tests that initialize `HealingReceivedEvent`

**Interfaces:**
- Consumes: Task 1 `RollCalculation`; `dice.Roller`.
- Produces: `features.FeatureInput.Roller`, `character.ActivateAbilityInput.Roller`, and `HealingReceivedEvent.Calculation` / `HealingAppliedEvent.Calculation` as deep-cloned `*RollCalculation` values. Removes the roll-bearing use of scalar `Roll` and `Modifier`.

- [ ] **Step 1: Write deterministic Second Wind and HP-owner tests**

Use a scoped roller returning 6. Assert the published calculation has a `1d10`/d10 trace with original/final `[6]`, subtotal 6, plus a modifier component sourced to `dnd5e:classes:fighter`, name `Fighter`, label `Fighter level`, amount 1, total 7. At HP 8/10 assert HealingApplied copies the calculation and reports requested 7, applied 2, HP 8→10. Mutate the received event after publication and prove the applied event does not alias it. Add character and monster rejection tests for invalid/mismatched calculations before HP mutation.

- [ ] **Step 2: Run focused tests and confirm RED**

```bash
go test ./features ./character ./monster -run 'Test.*(SecondWind|Healing.*Roll|HealingReceived)' -count=1
```

Expected: missing Roller/Calculation fields.

- [ ] **Step 3: Pass the scoped roller and publish one calculation**

Add `Roller dice.Roller` to both activation input types. `activateFeature` copies it into `features.FeatureInput`. Second Wind uses:

```go
result := pool.RollContext(ctx, input.Roller)
faces := append([]int(nil), result.Rolls()[0]...)
modifier := s.level
calculation := &dnd5eEvents.RollCalculation{
    Components: []dnd5eEvents.RollComponent{
        {
            Source: dnd5eEvents.RollSource{Ref: refs.Features.SecondWind(), Name: "Second Wind"},
            Dice: &dnd5eEvents.DiceTrace{
                Notation: "1d10", DieSize: 10,
                OriginalRolls: faces, FinalRolls: append([]int(nil), faces...),
                Subtotal: result.Total(),
            },
        },
        {
            Source: dnd5eEvents.RollSource{
                Ref: refs.Classes.Fighter(), Name: "Fighter", Label: "Fighter level",
            },
            Modifier: &modifier,
        },
    },
    Total: result.Total() + modifier,
}
```

Publish `Amount: calculation.Total` with the calculation. Nil `input.Roller` retains production default behavior for direct non-resolution callers; resolution supplies its required roller in Task 5.

- [ ] **Step 4: Validate and clone at HP ownership**

Character and monster handlers validate any non-nil calculation and require `event.Amount == calculation.Total` before changing HP. After applying the clamp and marking dirty, HealingApplied receives `CloneRollCalculation(event.Calculation)`. Non-roll healing may carry nil calculation and remains legal.

- [ ] **Step 5: Run root verification and eliminate global-random test mutation**

```bash
go test ./... -count=1
golangci-lint run ./...
! rg 'crypto/rand|cryptorand' features character monster --glob '*_test.go'
git diff --check
```

- [ ] **Step 6: Commit, review, merge, and record the final root tag**

```bash
git add rulebooks/dnd5e/features rulebooks/dnd5e/character \
        rulebooks/dnd5e/monster rulebooks/dnd5e/events
git commit -m "feat(dnd5e): carry Second Wind healing calculations"
```

Complete review/merge. Record the CI-minted root tag; this is the root version consumed by Tasks 5–6.

---

### Task 4: Encounter validates and persists neutral roll traces atomically

**Repository:** `rpg-toolkit` encounter module<br>
**Files:**
- Create: `rulebooks/dnd5e/encounter/roll_trace.go`
- Create: `rulebooks/dnd5e/encounter/roll_trace_test.go`
- Modify: `rulebooks/dnd5e/encounter/outcome.go`
- Modify: `rulebooks/dnd5e/encounter/outcome_test.go`
- Modify: `rulebooks/dnd5e/encounter/activation.go`
- Modify: `rulebooks/dnd5e/encounter/activation_test.go`

**Interfaces:**
- Consumes: primitive strings/ints only; no root D&D import.
- Produces: neutral mirrors of Task 1 with `RollSource.Ref string`; `ValidateRollCalculation(*RollCalculation) error`; `DamageComponent.Roll`; `ActivationResult.Calculation`.

- [ ] **Step 1: Write validation and no-partial-write tests**

Mirror every Task 1 malformed table case with canonical ref strings. For `Record`, demand a GWF damage payload containing original/reroll/final facts. For `RecordActivation`, demand a valid healing calculation and then mutate every nested field to invalid values. Assert the encounter's beat count remains unchanged whenever any result is invalid, including when the invalid result follows a valid result.

- [ ] **Step 2: Run focused tests and confirm RED**

```bash
go test ./... -run 'Test.*(RollCalculation|DamageComponent|Activation.*Roll)' -count=1
```

- [ ] **Step 3: Implement neutral types and domain validation**

New JSON writes use `roll` inside damage components and `calculation` inside healing results. `ActivationResult` drops its new-write `Roll`/`Modifier` scalar path. Healing requires a non-nil valid calculation with `calculation.Total == Requested`; condition/capacity variants forbid calculations.

`Record` validates damage dice traces and sourced modifiers before append while leaving damage multiplier rules uninterpreted. `RecordActivation` continues preparing the complete activation plus all results before its first append.

- [ ] **Step 4: Run encounter verification**

```bash
go test ./... -count=1
golangci-lint run ./...
git diff --check
```

- [ ] **Step 5: Commit, review, merge, and record the encounter tag**

```bash
git add rulebooks/dnd5e/encounter
git commit -m "feat(encounter): persist validated roll traces"
```

Complete review/merge and record the CI-minted `rulebooks/dnd5e/encounter/v…` tag.

---

### Task 5: Resolution preserves damage/healing traces and supplies activation dice

**Repository:** `rpg-toolkit` resolution module<br>
**Files:**
- Modify: `rulebooks/dnd5e/resolution/go.mod`
- Modify: `rulebooks/dnd5e/resolution/go.sum`
- Modify: `rulebooks/dnd5e/resolution/strike.go`
- Modify: `rulebooks/dnd5e/resolution/damage_custody_test.go`
- Modify: `rulebooks/dnd5e/resolution/activation.go`
- Modify: `rulebooks/dnd5e/resolution/activation_test.go`

**Interfaces:**
- Consumes: Task 3 root tag and Task 4 encounter tag.
- Produces: `ActivationInput.Roller dice.Roller`; `ActivationEffect.Calculation *events.RollCalculation`; root-shaped `StrikeOutcome.DamageComponents` with provider source names and untouched GWF trace history.

- [ ] **Step 1: Pin published dependencies and write failing tests**

Use `go get` with the exact CI-minted tags, then remove any local override before commit. Add an activation test whose injected roller returns 6 and assert collector output owns the complete Second Wind calculation. Add a GWF strike test with original `[1,5]`, reroll `1→4`, final `[4,5]`, and the Strength modifier component. Mutate publisher-owned traces after capture and prove no aliasing.

- [ ] **Step 2: Run focused tests and confirm RED**

```bash
go test ./... -run 'Test.*(Activation.*Calculation|GreatWeapon.*Trace|DamageCustody)' -count=1
```

- [ ] **Step 3: Carry the roller through the activation machine**

Add `Roller dice.Roller` to `ActivationInput`, reject nil at `NewActivation`, clone/store the interface on `activationMachine`, and pass it into `character.ActivateAbilityInput`. This follows `resolve.Input`'s existing rule that a machine which rolls carries its own roller.

- [ ] **Step 4: Preserve calculations in collectors and strike output**

`captureHealingApplied` validates and deep-clones `event.Calculation` into `ActivationEffect`. Strike construction populates the new root damage shape: weapon identity/name comes from the compiled action definition, ability identity/name from canonical refs, and condition/feature producers retain their own names. Do not sum or reconstruct GWF history in resolution.

- [ ] **Step 5: Run resolution verification and release-pin checks**

```bash
go test ./... -count=1
golangci-lint run ./...
! grep -n '^replace ' go.mod
git diff --check
```

- [ ] **Step 6: Commit, review, merge, and record the resolution tag**

```bash
git add rulebooks/dnd5e/resolution
git commit -m "feat(resolution): preserve roll traces across interactions"
```

Complete review/merge and record the CI-minted resolution tag.

---

### Task 6: Session records and projects new traces with legacy read compatibility

**Repository:** `rpg-toolkit` Session module<br>
**Files:**
- Modify: `rulebooks/dnd5e/session/go.mod`
- Modify: `rulebooks/dnd5e/session/go.sum`
- Modify: `rulebooks/dnd5e/session/attack.go`
- Modify: `rulebooks/dnd5e/session/attack_internal_test.go`
- Modify: `rulebooks/dnd5e/session/activate.go`
- Modify: `rulebooks/dnd5e/session/activation_events_test.go`
- Modify: `rulebooks/dnd5e/session/events.go`
- Modify: `rulebooks/dnd5e/session/events_internal_test.go`
- Modify: `rulebooks/dnd5e/session/types.go`

**Interfaces:**
- Consumes: final root, encounter, and resolution tags.
- Produces: exported string-ref mirrors `RollSource`, `DiceReroll`, `DiceTrace`, `RollComponent`, `RollCalculation`; new `DamageComponent.Roll`; new `HealingAppliedBody.Calculation`; legacy-only scalar read fallback.

- [ ] **Step 1: Pin exact tags and write failing mapper/projection tests**

Assert `recordDamageComponents` copies all GWF arrays, reroll source, labels, zero-modifier presence, subtotal, and total without aliasing. Assert `activationResults` copies the healing calculation. Assert live and `GetStory` return equal typed bodies. Add decoder fixtures for old damage/healing JSON and new JSON; reject a payload carrying both legacy scalars and a new trace, forbidden nulls, duplicate nested keys, bad arithmetic, and unknown fields under a known trace.

- [ ] **Step 2: Run focused tests and confirm RED**

```bash
go test ./... -run 'Test.*(RollTrace|DamageComponents|Activation.*Calculation|Legacy.*Roll)' -count=1
```

- [ ] **Step 3: Implement pure root→encounter adapters and supply the roller**

Add clone-only helpers for source, reroll, dice, component, and calculation. `recordDamageComponents` maps the root chain's existing facts one-for-one. `activationResults` maps `ActivationEffect.Calculation`. The Activate path passes `m.dice` to `resolution.NewActivation`.

- [ ] **Step 4: Project typed SDK bodies and legacy records**

New persisted payloads contain only `roll`/`calculation`. Session SDK bodies expose the generic nested types. Retain deprecated `Roll`/`Modifier` and old damage scalar fields only as a read representation for old persisted payloads. The strict decoder accepts exactly one representation and maps old records to their legacy fallback fields; it never fabricates a trace.

Update known-body duplicate/null/unknown-key validation recursively. Preserve known event kind with nil body for a malformed known payload, matching the existing activation decoder contract.

- [ ] **Step 5: Run Session verification**

```bash
go test ./... -count=1
golangci-lint run ./...
! grep -n '^replace ' go.mod
git diff --check
```

- [ ] **Step 6: Commit, review, merge, and record the Session tag**

```bash
git add rulebooks/dnd5e/session
git commit -m "feat(session): project durable roll traces"
```

Complete review/merge and record the CI-minted Session tag.

---

### Task 7: Add reusable roll trace messages to the canonical Session proto

**Repository:** `rpg-api-protos` source branch<br>
**Files:**
- Modify: `dnd5e/api/session/v1alpha1/events.proto`

**Interfaces:**
- Consumes: Task 6 exported SDK shape.
- Produces: generated Go/TypeScript messages `RollSource`, `DiceReroll`, `DiceTrace`, `RollComponent`, `RollCalculation`; `DamageComponent.roll = 8`; `HealingApplied.calculation = 10`.

- [ ] **Step 1: Add the proto contract with fixed tags**

```proto
message RollSource {
  string ref = 1;
  string name = 2;
  string label = 3;
}

message DiceReroll {
  int32 die_index = 1;
  int32 before = 2;
  int32 after = 3;
  RollSource source = 4;
}

message DiceTrace {
  string notation = 1;
  int32 die_size = 2;
  repeated int32 original_rolls = 3;
  repeated DiceReroll rerolls = 4;
  repeated int32 final_rolls = 5;
  repeated int32 kept_indices = 6;
  int32 subtotal = 7;
}

message RollComponent {
  RollSource source = 1;
  DiceTrace dice = 2;
  optional int32 modifier = 3;
}

message RollCalculation {
  repeated RollComponent components = 1;
  int32 total = 2;
}
```

Retain current damage tags 1–7 and healing tags 1–9. Mark old roll-bearing damage fields 2–5 and healing fields 3–4 deprecated but do not reuse/remove them. Old-record conversion may populate them; new events populate only the new message fields.

- [ ] **Step 2: Format and run canonical proto gates**

```bash
make format
make lint
make breaking
make test
git diff --check
```

Expected: no breaking changes against main; generated outputs are not committed to the source branch.

- [ ] **Step 3: Commit, review, and merge source**

```bash
git add dnd5e/api/session/v1alpha1/events.proto
git commit -m "feat(session): add reusable roll trace messages"
```

Complete the required reviews and merge to `main`.

- [ ] **Step 4: Wait for canonical generation and record immutable coordinates**

Verify CI updates `generated`. Record the exact generated commit, generated Go pseudo-version, and source release tag. Confirm generated Go and TypeScript expose the fields/tags above. Do not create a manual generated release branch or tag.

---

### Task 8: API maps traces field-for-field and proves real handler delivery

**Repository:** `rpg-api`<br>
**Files:**
- Modify: `go.mod`
- Modify: `go.sum`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/convert.go`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/convert_test.go`
- Modify: `internal/integration/session/acceptance_test.go`
- Modify: `internal/integration/session/activation_acceptance_test.go`
- Create: `internal/integration/session/roll_trace_acceptance_test.go`

**Interfaces:**
- Consumes: exact final toolkit tags and Task 7 generated proto pseudo-version.
- Produces: thin `rollSourceToProto`, `diceRerollToProto`, `diceTraceToProto`, `rollComponentToProto`, and `rollCalculationToProto` converters; handler/broker acceptance for GWF and Second Wind.

- [ ] **Step 1: Pin exact releases and write failing converter tests**

Update only required toolkit modules and the generated proto. Preserve any independently newer module pins. Converter tests assert all nested fields, nil safety, empty-but-present zero modifier, order, and no alias assumptions. New events populate `DamageComponent.roll` / `HealingApplied.calculation`; legacy SDK bodies populate only deprecated scalar proto fields.

- [ ] **Step 2: Run converter tests and confirm RED**

```bash
go test ./internal/handlers/dnd5e/session/v1alpha1 -run 'Test.*Roll' -count=1
```

- [ ] **Step 3: Implement field-for-field converters**

No converter sums arrays, validates rules, maps refs to labels, or selects fallbacks. Preserve repeated-field order exactly.

- [ ] **Step 4: Replace the crypto reader workaround with the scoped Session roller**

Delete `activationCryptoReaderMu`, `activateWithCryptoByte`, and `crypto/rand` imports. Add a local sequence roller and an `inAFightWithDice` helper that calls `newAcceptanceHarnessWithDice`. A Second Wind sequence producing face 6 must prove, through the real handler and broker, `1d10 [6]`, `+1 Fighter level`, total/requested 7, applied 2, and HP 8→10.

- [ ] **Step 5: Add real GWF handler/broker acceptance**

Seed a level-1 Fighter with a greatsword and the canonical Great Weapon Fighting condition. Supply sequence rolls for formation, attack, original damage `[1,5]`, and reroll `4`. Assert the Struck proto carries original `[1,5]`, sourced `1→4`, final `[4,5]`, Strength modifier, and authoritative damage total. Compare live and GetStory with `proto.Equal` and assert no duplicate sequence.

- [ ] **Step 6: Run focused and full API gates**

```bash
go test ./internal/handlers/dnd5e/session/v1alpha1 ./internal/integration/session -count=1
make release-pin-check
make ci-check
docker build -t rpg-api:roll-trace-check .
! grep -n '^replace ' go.mod
git diff --check
```

- [ ] **Step 7: Commit, review, merge to dev**

```bash
git add go.mod go.sum internal/handlers/dnd5e/session/v1alpha1 \
        internal/integration/session
git commit -m "feat(session): pass through server roll traces"
```

Complete review/merge and record the API merge commit and successful dev image workflow.

---

### Task 9: Web renders one shared trace format in damage and healing Story/Debug

**Repository:** `rpg-dnd5e-web`<br>
**Files:**
- Modify: `package.json`
- Modify: `package-lock.json`
- Create: `src/components/session/combat-experience/rollTrace.ts`
- Create: `src/components/session/combat-experience/rollTrace.test.ts`
- Modify: `src/components/session/combat-experience/story.ts`
- Modify: `src/components/session/combat-experience/story.test.ts`
- Modify: `src/components/session/debugLogLine.ts`
- Modify: `src/components/session/debugLogLine.test.ts`
- Modify: `src/components/session/combat-experience/presentation.ts`
- Modify: `src/components/session/combat-experience/useCombatPresentation.test.tsx`

**Interfaces:**
- Consumes: Task 7 exact generated commit.
- Produces: `formatRollCalculation(calculation: RollCalculation): string | undefined`; `formatDamageRolls(components: readonly DamageComponent[]): string | undefined`; lossless Debug formatting.

- [ ] **Step 1: Pin the exact generated commit and write formatter tests**

Pin both package files to the immutable Task 7 generated commit and reinstall. Tests require exact output:

```text
2d6 [1 → 4, 5] + 3 Strength = 12
1d10 [6] + 1 Fighter level = 7
```

Cover multiple ordered rerolls on one index, multi-die groups, negative/zero modifiers, labels, kept indices, malformed/absent calculations, and JSON-string escaping for provider names/labels. Formatter logic may arrange punctuation but must not recognize any ref or recalculate totals.

- [ ] **Step 2: Run formatter tests and confirm RED**

```bash
npm test -- --run src/components/session/combat-experience/rollTrace.test.ts
```

- [ ] **Step 3: Implement the shared formatter and domain Story composition**

Damage Story appends provider trace detail to the existing hit line. Healing Story replaces scalar arithmetic with `formatRollCalculation`, then retains applied/requested clamp wording. For legacy events lacking calculations, preserve the existing aggregate/scalar fallback.

Do not import 3D dice modules or emit dice-presentation events.

- [ ] **Step 4: Expand lossless Debug and conflict identity**

Debug includes notation, die size, original/final/kept arrays, every reroll index/before/after/source, component source, modifier presence/value, subtotal, calculation total, and domain totals. Extend the existing duplicate/conflict mutation table with every new mutable nested field; each mutation must diagnose conflict rather than deduplicate.

- [ ] **Step 5: Run focused and full web gates**

```bash
npm test -- --run \
  src/components/session/combat-experience/rollTrace.test.ts \
  src/components/session/combat-experience/story.test.ts \
  src/components/session/debugLogLine.test.ts \
  src/components/session/combat-experience/useCombatPresentation.test.tsx
npm test -- --run
npm run ci-check
git diff --check
```

- [ ] **Step 6: Commit, review, merge to dev**

```bash
git add package.json package-lock.json src/components/session
git commit -m "feat(session): render server-authored roll traces"
```

Complete Terra/public-GLM review, one bounded fix wave if required, exact-head CI, resolved threads, and human merge. Record the merge commit.

---

### Task 10: Isolated toolkit→API→web live acceptance and cleanup

**Repositories:** published toolkit stack, merged API `dev`, merged web `dev`, `rpg-deployment` lab<br>
**Files:**
- Create evidence under the owning implementation record selected for #361; do not commit licensed GLBs or temporary lab files.

**Interfaces:**
- Consumes: all merged/published outputs from Tasks 1–9.
- Produces: human-visible GWF and Second Wind Story/Debug/reconnect receipt; clean lab shutdown.

- [ ] **Step 1: Record the shared primary baseline and prepare private assets**

Capture current primary container IDs/start times and listeners. Run the documented `npm run assets:sync` from the merged web checkout before visual verification. Do not restart or repoint the shared primary while preparing the isolated lab.

- [ ] **Step 2: Start an isolated lab from merged API source**

Record the current image ID behind the lab overlay's required `rpg-api:local` tag, then build that tag from the merged API checkout with published toolkit pins and no local overrides. Start only `rpg-api-lab`/`envoy-lab` using `docker-compose.local-dev.yml` plus `docker-compose.local-lab.yml`. Start merged web on a free strict port with `VITE_API_HOST=http://localhost:8081`. The primary API remains on `ghcr.io/kirkdiggler/rpg-api:dev`; restore or remove the prior `rpg-api:local` tag during cleanup.

- [ ] **Step 3: Prove Second Wind**

Injure the Fighter, activate Second Wind, and capture both Story and Debug showing the actual live `1d10` face, `Fighter level` modifier, requested/applied values, and HP before/after. Verify the Hit Dice resource did not change.

- [ ] **Step 4: Prove Great Weapon Fighting**

Attack with a greatsword until at least one damage die naturally lands on 1 or 2. Capture Story and Debug showing original faces, ordered GWF reroll(s), final faces, Strength modifier, damage type, and total. The visual 3D layer must remain attack-d20-only; no damage die animation is expected.

- [ ] **Step 5: Prove reconnect parity**

Refresh/reconnect and compare the restored Story/Debug entries with the live entries. Assert the same sequence IDs, same nested roll facts, and no duplicates.

- [ ] **Step 6: Obtain human verdict and clean up**

Record the human verdict on rpg-project#361. Stop/remove only the lab API/envoy and temporary web process, remove temporary evidence/assets residue, and verify the shared primary container IDs/start times and listeners match the baseline.

- [ ] **Step 7: Close the implementation record**

Update the durable implementation record with exact toolkit tags, generated proto commit/pseudo-version, API/web merge commits, commands, live receipt, cleanup evidence, and residual risks. Close linked child issues and #361 only after all evidence is present.
