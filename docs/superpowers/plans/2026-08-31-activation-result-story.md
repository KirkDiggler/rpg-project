# Activation and Result Story Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Record every successful activation and each actual result as ordered, durable, typed session events visible in both Story and Debug.

**Architecture:** The root D&D event bus publishes post-mutation facts, resolution captures interaction-scoped effects, encounter records one activation transaction, and session projects typed bodies for live and catch-up parity. Protos and rpg-api mirror those types; web resolves names and formats facts without feature switches.

**Tech Stack:** Go 1.24 multi-module toolkit, typed event bus, protobuf/buf, gRPC/miniredis acceptance, React/TypeScript/Vitest.

**Spec:** `docs/superpowers/specs/2026-08-31-run-readiness-and-activation-story-design.md`

## Global Constraints

- Keep SessionService Activate and Afford; add no RPC and no optimistic client log.
- Record only after successful resolution and dirty-sheet persistence.
- Refused/stale activations record nothing.
- Activation appears before all result events; result order matches synchronous effect order.
- Healing reports actual HP applied after clamping, with before/after and roll inputs.
- Live and GetStory catch-up use the same session projection.
- rpg-api has no feature/condition switch; web uses typed fields only.

---

### Task 1: Publish actual post-clamp healing facts from the sheet keeper

**Files:**
- Modify: `rpg-toolkit/rulebooks/dnd5e/events/events.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/character.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/character/sheet_keeper_test.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/features/second_wind.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/features/second_wind_test.go`

**Interfaces:**
- Produces:

```go
type HealingAppliedEvent struct {
    TargetID string
    Requested int
    Applied int
    HPBefore int
    HPAfter int
    Roll int
    Modifier int
    SourceRef *core.Ref
}

var HealingAppliedTopic = events.DefineTypedTopic[HealingAppliedEvent](
    "dnd5e.combat.healing.applied")
```

- [ ] **Step 1: Add failing post-clamp tests**

Subscribe to HealingAppliedTopic, publish HealingReceivedEvent to a character at 8/10 HP with amount 7, and assert:

```go
require.Equal(t, 7, got.Requested)
require.Equal(t, 2, got.Applied)
require.Equal(t, 8, got.HPBefore)
require.Equal(t, 10, got.HPAfter)
require.Equal(t, 6, got.Roll)
require.Equal(t, 1, got.Modifier)
require.True(t, got.SourceRef.Equals(refs.Features.SecondWind()))
```

Negative control: a HealingReceivedEvent for another member publishes no applied fact.

- [ ] **Step 2: Run and verify RED**

```bash
cd rpg-toolkit/rulebooks/dnd5e
go test ./character ./features -run 'Test.*Healing.*Applied|Test.*SecondWind' -count=1
```

Expected: HealingAppliedTopic/type is absent.

- [ ] **Step 3: Add SourceRef to requested healing and publish the applied fact**

Extend HealingReceivedEvent with optional `SourceRef *core.Ref` while retaining existing `Source string` during migration. Second Wind supplies `refs.Features.SecondWind()`.

Change `onHealingReceived` to retain the context, calculate before/after/applied, mutate HP, mark dirty, then synchronously publish HealingAppliedEvent. Clone SourceRef before publishing so callers cannot mutate the fact.

- [ ] **Step 4: Verify root packages**

```bash
go test ./character ./features ./conditions ./combat -count=1
```

Expected: PASS, including hit-die and unconscious healing publishers with nil SourceRef where no canonical source is available.

- [ ] **Step 5: Commit and publish the root event provider**

```bash
git add rulebooks/dnd5e/events/events.go rulebooks/dnd5e/character \
  rulebooks/dnd5e/features/second_wind.go rulebooks/dnd5e/features/second_wind_test.go
git commit -m "feat: publish actual healing applied facts"
```

Run root module tests/vet/lint, open the PR, wait for human merge, and record the exact `rulebooks/dnd5e` tag for Task 3.

### Task 2: Record an activation transaction in encounter

**Files:**
- Create: `rpg-toolkit/rulebooks/dnd5e/encounter/activation.go`
- Create: `rpg-toolkit/rulebooks/dnd5e/encounter/activation_test.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/encounter/outcome.go` only if shared primitive validation is extracted.

**Interfaces:**
- Produces:

```go
type ActivationIdentity struct { Ref, Name string }

type ActivationResultKind string
const (
    ResultHealingApplied ActivationResultKind = "healing-applied"
    ResultConditionApplied ActivationResultKind = "condition-applied"
    ResultConditionRemoved ActivationResultKind = "condition-removed"
    ResultCapacityGranted ActivationResultKind = "capacity-granted"
)

type ActivationResult struct {
    Kind ActivationResultKind
    Target MemberID
    Ref string
    Name string
    Amount, Roll, Modifier, Before, After int
    Description string
    Reason string
}

type RecordActivationInput struct {
    Actor MemberID
    Target MemberID
    Ability ActivationIdentity
    Results []ActivationResult
}

type RecordActivationOutput struct {
    Seqs []uint64
    IntelDeltas map[MemberID]*IntelDelta
}
```

- [ ] **Step 1: Write failing transaction tests**

Test one Second Wind input records exactly:

```json
{"beat":"activated","actor":"fighter","ability":{"ref":"dnd5e:features:second_wind","name":"Second Wind"}}
{"beat":"activation-result","actor":"fighter","result":{"kind":"healing-applied","target":"fighter","amount":2,"roll":6,"modifier":1,"before":8,"after":10,"ref":"dnd5e:features:second_wind","name":"Second Wind"}}
```

Assert activation seq precedes result seq, both share the current audience, and invalid actor/target/kind/ref/name is rejected before any append. Add a multi-result order test and a notice-down call-count test proving consequences are consulted once after the whole transaction.

- [ ] **Step 2: Run and verify RED**

```bash
cd rpg-toolkit/rulebooks/dnd5e/encounter
go test ./... -run 'TestRecordActivation' -count=1
```

Expected: RecordActivation types/method are absent.

- [ ] **Step 3: Implement closed validation and ordered append**

Validate the complete input first. Append one `activated` payload, then one `activation-result` payload per result. Use `audienceFor(subjectBeat, actor, target/result targets...)`, tag every beat `outcome`, and call `noticeDown` once after all appends.

Do not route activation through repeated `Encounter.Record` calls; that would run consequence processing between the cause and its results.

- [ ] **Step 4: Verify encounter**

```bash
gofmt -w activation.go activation_test.go
go test ./... -count=1
go vet ./...
```

Expected: PASS.

- [ ] **Step 5: Commit, merge, and record encounter tag**

```bash
git add activation.go activation_test.go outcome.go
git commit -m "feat: record activation transactions"
```

Open the encounter-module PR, wait for human merge, and record the exact `rulebooks/dnd5e/encounter` tag.

### Task 3: Capture activation effects in resolution

**Files:**
- Modify: `rpg-toolkit/rulebooks/dnd5e/resolution/go.mod`
- Modify: `rpg-toolkit/rulebooks/dnd5e/resolution/go.sum`
- Modify: `rpg-toolkit/rulebooks/dnd5e/resolution/activation.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/resolution/activation_test.go`

**Interfaces:**
- Consumes: root `HealingAppliedTopic`, ConditionAppliedTopic, ConditionRemovedTopic, `conditions.DisplayFor`.
- Produces:

```go
type ActivationEffectKind string
const (
    EffectHealingApplied ActivationEffectKind = "healing-applied"
    EffectConditionApplied ActivationEffectKind = "condition-applied"
    EffectConditionRemoved ActivationEffectKind = "condition-removed"
    EffectCapacityGranted ActivationEffectKind = "capacity-granted"
)

type ActivationEffect struct {
    Kind ActivationEffectKind
    TargetID string
    Ref string
    Name string
    Amount, Roll, Modifier, Before, After int
    Description string
    Reason string
}

type ActivationOutcome struct {
    Ability string
    GrantedCapacity string
    Effects []ActivationEffect
}
```

- [ ] **Step 1: Pin the published root tag**

Update resolution's D&D root requirement to Task 1's release. No local replace remains.

- [ ] **Step 2: Add failing effect-capture tests**

Extend the existing Rage and Second Wind activation tests:

```go
require.Equal(t, []ActivationEffect{{
    Kind: EffectHealingApplied,
    TargetID: fighterID,
    Ref: refs.Features.SecondWind().String(),
    Name: "Second Wind",
    Amount: 2, Roll: 6, Modifier: 1, Before: 8, After: 10,
}}, outcome.Effects)
```

Rage/Dodge capture a condition-applied effect with catalog display name. Dash captures capacity-granted. A refused activation returns no outcome/effects.

- [ ] **Step 3: Run and verify RED**

```bash
cd rpg-toolkit/rulebooks/dnd5e/resolution
go test ./... -run 'Test.*Activation' -count=1
```

Expected: effects are absent.

- [ ] **Step 4: Subscribe interaction-scoped collectors**

In `activationMachine.step`, use the provided interaction bus instead of discarding it. Subscribe before `actor.ActivateAbility`, append typed effects synchronously, and unsubscribe all collectors before returning.

For condition events, read `Condition.Ref()` and `conditions.DisplayFor`; an unknown display ref is an activation error rather than an unnamed result. For HealingApplied, report `Applied` as Amount. Clone slices/refs. Append capacity-granted after bus effects when `out.GrantedCapacity != ""`.

- [ ] **Step 5: Verify and publish resolution**

```bash
gofmt -w activation.go activation_test.go
go test ./... -count=1
go vet ./...
```

Commit:

```bash
git add go.mod go.sum activation.go activation_test.go
git commit -m "feat: capture typed activation effects"
```

Open PR, wait for human merge, and record the exact resolution tag.

### Task 4: Persist and project activation events from session

**Files:**
- Modify: `rpg-toolkit/rulebooks/dnd5e/session/go.mod`
- Modify: `rpg-toolkit/rulebooks/dnd5e/session/go.sum`
- Modify: `rpg-toolkit/rulebooks/dnd5e/session/activate.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/session/activate_test.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/session/types.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/session/events.go`
- Modify: `rpg-toolkit/rulebooks/dnd5e/session/events_internal_test.go`
- Create: `rpg-toolkit/rulebooks/dnd5e/session/activation_events_test.go`

**Interfaces:**
- Consumes: published root, encounter, and resolution tags.
- Produces:

```go
const (
    EventActivated EventKind = "activated"
    EventActivationResult EventKind = "activation_result"
)

type ActivatedBody struct {
    Actor string
    Ability AbilityRef
    Target string
}

type ActivationResultBody struct {
    Actor string
    HealingApplied *HealingAppliedBody
    ConditionApplied *ConditionAppliedBody
    ConditionRemoved *ConditionRemovedBody
    CapacityGranted *CapacityGrantedBody
}
```

Exactly one `ActivationResultBody` result pointer is non-nil. The result structs carry the same fields defined by `encounter.ActivationResult`; these types are the source mapped field-for-field into Task 5's proto messages.

- [ ] **Step 1: Pin provider tags**

Pin exact root/encounter/resolution releases and run `go mod tidy`. No replace remains.

- [ ] **Step 2: Add failing session story tests**

Use a real session Manager and Story/Event sink. Activate Second Wind and assert two events in order with the same facts as resolution. Activate Rage and Dash for condition/capacity variants. Assert stale selector leaves story length unchanged.

Also assert `Saved.Written` is populated before RecordActivation by injecting a record failure and checking the established unrecorded report path.

- [ ] **Step 3: Run and verify RED**

```bash
cd rpg-toolkit/rulebooks/dnd5e/session
go test ./... -run 'Test.*Activation.*Event|Test.*SecondWind.*Story' -count=1
```

Expected: Activate delivers no story event.

- [ ] **Step 4: Record after saveDirty**

Map resolution effects to encounter ActivationResult and call:

```go
recorded, err := scope.enc.RecordActivation(&encounter.RecordActivationInput{
    Actor: encounter.MemberID(in.Member),
    Target: encounter.MemberID(in.Target),
    Ability: encounter.ActivationIdentity{
        Ref: selected.declaration.Ability.Ref,
        Name: selected.declaration.Ability.Name,
    },
    Results: activationResults(activated.Effects),
})
```

Place this after `adopt` and `saveDirty`, before `commit`, matching Attack. On failure use `reportUnrecorded` and drop the unsaved encounter scope.

- [ ] **Step 5: Add typed event decoding**

Add session event kinds `activated` and `activation_result`. Define body structs with exactly one result variant populated. Extend `kindFor` and `bodyFor` to decode the new payloads; malformed known payload keeps its known kind and nil body.

- [ ] **Step 6: Verify live/catch-up parity and publish session**

```bash
gofmt -w activate.go activate_test.go types.go events.go \
  events_internal_test.go activation_events_test.go
go test ./... -count=1
go vet ./...
```

Commit, open PR, wait for human merge, and record the exact session tag.

### Task 5: Add typed activation events to protos

**Files:**
- Modify: `rpg-api-protos/dnd5e/api/session/v1alpha1/events.proto`
- Generated: `rpg-api-protos/gen/go/**`
- Generated: `rpg-api-protos/gen/ts/**`

**Interfaces:**
- Produces: EventKind values 15/16 and body tags 21/22.

- [ ] **Step 1: Add enum and body arms**

```protobuf
enum EventKind {
  // existing 0-14 and UNKNOWN=100
  EVENT_KIND_ACTIVATED = 15;
  EVENT_KIND_ACTIVATION_RESULT = 16;
}

oneof body {
  // existing tags 10-20
  Activated activated = 21;
  ActivationResult activation_result = 22;
}
```

Add:

```protobuf
message Activated {
  string actor = 1;
  AbilityRef ability = 2;
  string target = 3;
}

message ActivationResult {
  string actor = 1;
  oneof result {
    HealingApplied healing_applied = 2;
    ConditionApplied condition_applied = 3;
    ConditionRemoved condition_removed = 4;
    CapacityGranted capacity_granted = 5;
  }
}

message HealingApplied {
  string target = 1;
  int32 amount = 2;
  int32 roll = 3;
  int32 modifier = 4;
  string source_ref = 5;
  string source_name = 6;
  int32 hp_before = 7;
  int32 hp_after = 8;
}
message ConditionApplied { string target = 1; string ref = 2; string name = 3; }
message ConditionRemoved { string target = 1; string ref = 2; string name = 3; string reason = 4; }
message CapacityGranted { string member = 1; string description = 2; }
```

These message names are currently unused in `dnd5e.api.session.v1alpha1`; use them exactly so every downstream type name is fixed by this plan.

- [ ] **Step 2: Generate and verify compatibility**

```bash
cd rpg-api-protos
make generate
make test
buf lint
buf breaking --against '.git#branch=main'
```

Expected: PASS; all changes are additive.

- [ ] **Step 3: Commit, merge, and record proto tag**

```bash
git add dnd5e/api/session/v1alpha1/events.proto gen
git commit -m "feat: add activation result events"
```

After human merge, record the concrete published value as `PROTO_TAG`; Task 7 consumes that exact tag.

### Task 6: Pin and map activation events through rpg-api

**Files:**
- Modify: `rpg-api/go.mod`
- Modify: `rpg-api/go.sum`
- Modify: `rpg-api/internal/handlers/dnd5e/session/v1alpha1/convert.go`
- Modify: `rpg-api/internal/handlers/dnd5e/session/v1alpha1/convert_test.go`
- Modify: `rpg-api/internal/integration/session/activation_acceptance_test.go`
- Modify: `rpg-api/internal/integration/session/story_events_test.go`
- Modify if invalidated: `rpg-api/docs/status.md`

**Interfaces:**
- Consumes: published session tag and proto tag.
- Produces: field-for-field GetStory/StreamEvents activation bodies.

- [ ] **Step 1: Pin exact published versions**

Pin root/resolution/session and generated proto modules. Run `go mod tidy` and verify no replace.

- [ ] **Step 2: Add failing converter and acceptance tests**

Construct each SDK body variant and assert exact proto arm/fields. Extend Second Wind acceptance to subscribe before Activate and also query GetStory; both reads must contain:

```go
require.Equal(t, sessionpb.EventKind_EVENT_KIND_ACTIVATED, events[0].GetKind())
require.Equal(t, "Second Wind", events[0].GetActivated().GetAbility().GetName())
require.Equal(t, sessionpb.EventKind_EVENT_KIND_ACTIVATION_RESULT, events[1].GetKind())
require.Equal(t, int32(2), events[1].GetActivationResult().GetHealingApplied().GetAmount())
require.Equal(t, int32(8), events[1].GetActivationResult().GetHealingApplied().GetHpBefore())
require.Equal(t, int32(10), events[1].GetActivationResult().GetHealingApplied().GetHpAfter())
```

- [ ] **Step 3: Run and verify RED**

```bash
cd rpg-api
go test ./internal/handlers/dnd5e/session/v1alpha1 \
  ./internal/integration/session -run 'Test.*Activation|Test.*Story' -count=1
```

Expected: converter maps new kinds to UNKNOWN and leaves body nil.

- [ ] **Step 4: Add direct conversion arms**

Extend `eventKindToProto` and `setEventBody`. Use one helper per result variant; no switch on ability/condition refs and no prose composition.

- [ ] **Step 5: Verify API**

```bash
go test ./internal/handlers/dnd5e/session/v1alpha1 ./internal/integration/session -count=1
go test -short ./... -count=1
./scripts/verify-release-pin.sh
make pre-commit
```

Expected: PASS.

- [ ] **Step 6: Commit and open API PR**

```bash
git add go.mod go.sum internal/handlers/dnd5e/session/v1alpha1 \
  internal/integration/session docs/status.md
git commit -m "feat: pass through activation story events"
```

### Task 7: Render activation and result in web Story and Debug

**Files:**
- Modify: `rpg-dnd5e-web/package.json`
- Modify: `rpg-dnd5e-web/package-lock.json`
- Modify: `rpg-dnd5e-web/src/components/session/combat-experience/story.ts`
- Modify: `rpg-dnd5e-web/src/components/session/combat-experience/story.test.ts`
- Modify: `rpg-dnd5e-web/src/components/session/combat-experience/presentation.ts`
- Modify: `rpg-dnd5e-web/src/components/session/debugLogLine.ts`
- Modify: `rpg-dnd5e-web/src/components/session/debugLogLine.test.ts`
- Modify: `rpg-dnd5e-web/src/components/session/SessionEncounterView.tsx`
- Modify: `rpg-dnd5e-web/src/components/session/combat-experience/useCombatPresentation.test.tsx`
- Modify if invalidated: `rpg-dnd5e-web/docs/status.md`

**Interfaces:**
- Consumes: generated Activated/ActivationResult bodies.
- Produces: one Story entry per event and one raw Debug line per event.

- [ ] **Step 1: Pin the exact proto tag**

Use `npm i --save github:KirkDiggler/rpg-api-protos#$PROTO_TAG` where `PROTO_TAG` is the exact merged Task 5 release recorded by the coordinator. Verify package-lock resolves that commit.

- [ ] **Step 2: Add failing Story tests**

Build typed events and assert ordered entries:

```ts
expect(story.map((entry) => entry.headline)).toEqual([
  'Aldric uses Second Wind',
  'Aldric recovers 2 HP',
]);
expect(story[1]?.detail).toContain('8 → 10 HP');
```

Add condition and capacity variants. Story must use body names, not parse refs.

- [ ] **Step 3: Add failing Debug tests**

Assert complete lines include actor, ability ref/name, result kind, target, amount, roll, modifier, before/after, source ref/name, condition reason, or capacity description as applicable.

- [ ] **Step 4: Run tests and verify RED**

```bash
npm test -- --run src/components/session/combat-experience/story.test.ts \
  src/components/session/debugLogLine.test.ts
```

Expected: activation body cases are unhandled.

- [ ] **Step 5: Implement generic Story/Debug formatting**

Add `activated` and `activationResult` branches. Resolve member IDs through the existing name map. Branch only on the result oneof case; do not switch on feature or condition refs.

Update presentation's body-case/EventKind consistency map so valid activation bodies are accepted and conflicting duplicates remain rejected.

- [ ] **Step 6: Refresh owner truth on result events**

In `refreshKeysForEvent`, return `['characterData', 'afford', 'view']` for `activationResult`; `activated` needs no extra snapshot beyond the funnel's unconditional turn/Afford refresh. This keeps HP, conditions, and charges synchronized from server reads.

- [ ] **Step 7: Verify web**

```bash
npm test -- --run src/components/session/combat-experience/story.test.ts \
  src/components/session/debugLogLine.test.ts \
  src/components/session/combat-experience/useCombatPresentation.test.tsx \
  src/components/session/SessionEncounterView.test.tsx
npm run ci-check
```

Expected: PASS.

- [ ] **Step 8: Commit and open web PR**

```bash
git add package.json package-lock.json src/components/session docs/status.md
git commit -m "feat: render activation and result story events"
```

### Task 8: Live activation acceptance

**Files:**
- No production files unless evidence exposes a defect.

- [ ] **Step 1: Run exact branch heads in isolated lab1**

Build branch API and unchanged branch web through Envoy; preserve the shared primary environment.

- [ ] **Step 2: Activate Second Wind in the Reference Tomb**

Take damage first. Confirm Story shows “uses Second Wind” followed by the actual recovered HP, not the rolled amount when healing is clamped.

- [ ] **Step 3: Inspect Debug and catch-up**

Confirm Debug contains every typed field. Refresh/reconnect and confirm GetStory reproduces the same ordered events without duplicates.

- [ ] **Step 4: Verify another effect family**

Activate Rage or Dodge and confirm activation plus condition-applied result. Activate Dash and confirm activation plus capacity-granted result.

- [ ] **Step 5: Record evidence and stop lab**

Post exact tags/commits, CI, stream/GetStory transcript, screenshots, and human verdict. Stop only isolated services.
