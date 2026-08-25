# Strike Breakdown in the Debug Feed Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Carry the existing resolution damage breakdown and advantage/disadvantage attribution through replayable SessionService `Struck` events and print it in the existing one-line debug feed.

**Architecture:** Resolution remains the only rules owner. Session projects a small primitive carrier into encounter's persisted story; the same story projection supplies live events and `GetStory`. Protos transcribe that shape, rpg-api copies it, and web prints it without calculations.

**Tech Stack:** Go 1.24/1.25, testify, Protocol Buffers/Buf, Connect/gRPC, TypeScript, React, Vitest.

**Spec:** `ideas/debug-feed/strike-breakdown/design.md`

## Global Constraints

- Scope is `Struck` only; `Missed` stays unchanged.
- Damage components carry only source, optional canonical source ref, dice notation, final rolls, flat bonus, damage type, and optional multiplier.
- Modifier attribution carries only optional canonical source ref and source entity ID. `AttackModifierSource.Reason` remains internal; no replacement reason vocabulary is added.
- Preserve component and modifier order. Do not sort, group, total, or infer effective advantage.
- Top-level `Damage` and `Critical` remain authoritative. API and web perform no rules calculations.
- Old story payloads decode to empty detail collections and render the existing aggregate line unchanged.
- Clients consume typed protobuf bodies and never decode `Event.payload`.
- No generic metadata bag, new event kind, story migration, ADR, or generated-code edit.
- One issue and one PR per repository/module: protos #246; encounter #1238; session #1239; API #836; web #805.
- Branch from `origin/main` for protos/toolkit and `origin/dev` for API/web, after `git fetch origin`.
- Deliver sequentially: proto release; encounter release; session release pinned to encounter; API pinned to all providers; web pinned to proto. Never commit `replace`, `go.work`, or generated proto output.
- Use normal GitHub CI/Copilot review once per PR; do not create internal review loops for already-written code.

---

### Task 1: Publish the additive SessionService contract — rpg-api-protos#246

**Files:**
- Modify: `dnd5e/api/session/v1alpha1/events.proto:242-258`

**Interfaces:**
- Consumes: the exact field contract in `ideas/debug-feed/strike-breakdown/design.md`.
- Produces: `DamageComponent`, `AttackModifierSource`, and `Struck.damage_components/advantage_sources/disadvantage_sources` in generated Go and TypeScript SDKs.

- [ ] **Step 1: Create an isolated branch from the current proto base and establish the baseline**

```bash
cd /home/kirk/game-dev/rpg-api-protos
git fetch origin
git worktree add /home/kirk/game-dev/.pi-worktrees/246-struck-detail \
  -b feat/246-struck-detail origin/main
cd /home/kirk/game-dev/.pi-worktrees/246-struck-detail
buf lint
buf format --diff --exit-code
buf breaking --against 'https://github.com/KirkDiggler/rpg-api-protos.git#branch=main'
```

Expected: all three commands exit 0. Do not proceed past a failing baseline.

- [ ] **Step 2: Add the minimal messages and fields**

Place these messages immediately before `Struck`, then add fields 9–11 to `Struck`:

```proto
// DamageComponent is one ordered source-attributed input to resolved damage.
message DamageComponent {
  string source = 1;
  string source_ref = 2;
  string dice = 3;
  repeated int32 final_rolls = 4;
  int32 flat_bonus = 5;
  DamageType damage_type = 6;
  // Presence distinguishes an additive component from immunity's real zero.
  optional double multiplier = 7;
}

// AttackModifierSource identifies one source of advantage or disadvantage.
message AttackModifierSource {
  string source_ref = 1;
  string source_id = 2;
}
```

```proto
  repeated DamageComponent damage_components = 9;
  repeated AttackModifierSource advantage_sources = 10;
  repeated AttackModifierSource disadvantage_sources = 11;
```

Do not add a `reason`, total, critical, property, reroll, or metadata field.

- [ ] **Step 3: Format and run the repository's mechanical contract gates**

```bash
buf format -w
buf lint
buf format --diff --exit-code
buf breaking --against 'https://github.com/KirkDiggler/rpg-api-protos.git#branch=main'
buf generate
make compile-go
make compile-ts
git status --short
```

Expected: all checks pass; generated output remains ignored and only `events.proto` is staged later. Per repository policy, do not add bespoke tests for generated protobuf mechanics.

- [ ] **Step 4: Commit, push, and open the provider PR**

```bash
git add dnd5e/api/session/v1alpha1/events.proto
git diff --cached --check
git commit -m 'feat(session): add typed strike breakdown (#246)'
git push -u origin feat/246-struck-detail
gh pr create --repo KirkDiggler/rpg-api-protos --base main \
  --head feat/246-struck-detail \
  --title 'feat(session): add typed strike breakdown (#246)' \
  --body 'Adds the approved typed Struck component and modifier-source fields. No raw payload contract and no generated files. Closes #246.'
```

- [ ] **Step 5: Merge and record the generated release**

After CI/Copilot approval, squash-merge. Wait for the generated branch and release workflow, then confirm the newest release contains the new generated Go and TypeScript fields:

```bash
OLD_PROTO_TAG=$(gh release list -R KirkDiggler/rpg-api-protos --limit 1 --json tagName --jq '.[0].tagName')
# Perform the approved squash merge, then wait for publish-packages.
PROTO_TAG=$(gh release list -R KirkDiggler/rpg-api-protos --limit 1 --json tagName --jq '.[0].tagName')
test -n "$PROTO_TAG"
test "$PROTO_TAG" != "$OLD_PROTO_TAG"
git fetch origin generated --tags
git show "origin/generated:gen/ts/dnd5e/api/session/v1alpha1/events_pb.ts" | \
  rg 'damageComponents|finalRolls|advantageSources|disadvantageSources|multiplier'
```

Record `PROTO_TAG` on #246 and parent #265. Consumers must use that exact published tag, not `generated` or an assumed future version.

---

### Task 2: Persist the closed encounter carrier — rpg-toolkit#1238

**Files:**
- Modify: `rulebooks/dnd5e/encounter/outcome.go:78-151,240-330`
- Modify: `rulebooks/dnd5e/encounter/outcome_test.go:44-205`

**Interfaces:**
- Consumes: primitive ordered component and modifier values from the session caller.
- Produces: `encounter.DamageComponent`, `encounter.AttackModifierSource`, and three optional `RecordInput` slices serialized under `damage_components`, `advantage_sources`, and `disadvantage_sources` for struck beats only.

- [ ] **Step 1: Create the encounter-only worktree and run its baseline**

```bash
cd /home/kirk/game-dev/rpg-toolkit
git fetch origin
git worktree add /home/kirk/game-dev/.pi-worktrees/1238-encounter-strike-detail \
  -b feat/1238-encounter-strike-detail origin/main
cd /home/kirk/game-dev/.pi-worktrees/1238-encounter-strike-detail/rulebooks/dnd5e/encounter
go test -race ./...
golangci-lint run ./...
```

Expected: both commands pass. Do not run tidy or edit any sibling module.

- [ ] **Step 2: Write the failing carrier and no-prose tests**

Extend `TestAnOutcomeCarriesNoProse` to expect the three new `RecordInput` fields, and add a test that records this exact shape twice in fresh scenes:

```go
immunity := 0.0
in := &encounter.RecordInput{
    Kind: encounter.OutcomeStruck, Actor: alice,
    Targets: []encounter.MemberID{goblin},
    DamageComponents: []encounter.DamageComponent{
        {
            Source: "weapon", SourceRef: "dnd5e:weapons:longsword",
            Dice: "1d8", FinalRolls: []int{4}, FlatBonus: 0, DamageType: "slashing",
        },
        {
            Source: "monster_trait", SourceRef: "dnd5e:monster-traits:immunity",
            DamageType: "slashing", Multiplier: &immunity,
        },
    },
    AdvantageSources: []encounter.AttackModifierSource{
        {SourceRef: "dnd5e:conditions:hidden", SourceID: "alice"},
    },
    DisadvantageSources: []encounter.AttackModifierSource{
        {SourceRef: "dnd5e:conditions:dodging", SourceID: "goblin"},
    },
}
```

Decode the recorded payload into a typed test struct, assert order and `Multiplier != nil && *Multiplier == 0`, and assert the two payload byte slices are equal. Assert no `reason` key appears. Update the structural expectation to:

```go
[]string{
    "Kind", "Actor", "Targets", "Values", "Critical", "Attack",
    "DamageComponents", "AdvantageSources", "DisadvantageSources",
}
```

- [ ] **Step 3: Run the focused test and confirm RED**

```bash
go test ./... -run 'TestOutcomeSuite/Test(ARecordedStrikeCarriesOrderedDetail|AnOutcomeCarriesNoProse)' -count=1
```

Expected: compile failure because `encounter.DamageComponent`, `encounter.AttackModifierSource`, and the new `RecordInput` fields do not exist.

- [ ] **Step 4: Add the minimal primitive carrier and struck-only payload fields**

Add documented public types beside `AttackIdentity`:

```go
// DamageComponent carries one ordered, rulebook-neutral damage contribution.
type DamageComponent struct {
    Source     string   `json:"source"`
    SourceRef  string   `json:"source_ref,omitempty"`
    Dice       string   `json:"dice,omitempty"`
    FinalRolls []int    `json:"final_rolls,omitempty"`
    FlatBonus  int      `json:"flat_bonus"`
    DamageType string   `json:"damage_type"`
    Multiplier *float64 `json:"multiplier,omitempty"`
}

// AttackModifierSource identifies an entity/content source without carrying prose.
type AttackModifierSource struct {
    SourceRef string `json:"source_ref,omitempty"`
    SourceID  string `json:"source_id,omitempty"`
}
```

Add the three slices to `RecordInput`. Update `TestAnOutcomeCarriesNoProse`'s explanation to state that the nested carriers contain only identifiers, notation, and numbers and deliberately omit `Reason`. In `Record`, only inside the existing `OutcomeStruck` payload block, add each JSON key when its slice is non-empty. Keep slice order, do not sort, and add no semantic validation of source/dice/type/ref strings. Do not add `Reason` to either type.

- [ ] **Step 5: Run focused and full encounter verification**

```bash
go test ./... -run 'TestOutcomeSuite/Test(ARecordedStrikeCarriesOrderedDetail|AnOutcomeCarriesNoProse)' -count=1
go test -race ./...
golangci-lint run ./...
git diff --check
! rg '^replace ' go.mod
```

Expected: all commands pass; only encounter-module files changed.

- [ ] **Step 6: Commit, push, and open the encounter PR**

```bash
cd /home/kirk/game-dev/.pi-worktrees/1238-encounter-strike-detail
git add rulebooks/dnd5e/encounter/outcome.go rulebooks/dnd5e/encounter/outcome_test.go
git commit -m 'feat(encounter): carry replayable strike detail (#1238)'
git push -u origin feat/1238-encounter-strike-detail
gh pr create --repo KirkDiggler/rpg-toolkit --base main \
  --head feat/1238-encounter-strike-detail \
  --title 'feat(encounter): carry replayable strike detail (#1238)' \
  --body 'Adds the approved primitive, ordered struck-detail carrier without rule imports or prose. Closes #1238.'
```

- [ ] **Step 7: Merge, verify the stable encounter tag, and record it**

After CI/Copilot approval, squash-merge, fetch the merge commit and tags, and identify the tag pointing at that commit:

```bash
ENCOUNTER_PR=$(gh pr list --repo KirkDiggler/rpg-toolkit --state merged \
  --search '#1238' --json number --jq '.[0].number')
ENCOUNTER_MERGE=$(gh pr view "$ENCOUNTER_PR" --repo KirkDiggler/rpg-toolkit \
  --json mergeCommit --jq '.mergeCommit.oid')
git -C /home/kirk/game-dev/rpg-toolkit fetch origin --tags
ENCOUNTER_TAG=$(git -C /home/kirk/game-dev/rpg-toolkit tag --points-at "$ENCOUNTER_MERGE" | \
  rg '^rulebooks/dnd5e/encounter/v' | sort -V | tail -1)
test -n "$ENCOUNTER_TAG"
```

Record `ENCOUNTER_TAG` on #1238 and #265 before starting session work.

---

### Task 3: Project resolution facts into replayable session bodies — rpg-toolkit#1239

**Files:**
- Modify: `rulebooks/dnd5e/session/go.mod`
- Modify: `rulebooks/dnd5e/session/go.sum`
- Modify: `rulebooks/dnd5e/session/types.go:650-700`
- Modify: `rulebooks/dnd5e/session/attack.go:432-475`
- Modify: `rulebooks/dnd5e/session/events.go:330-400`
- Modify: `rulebooks/dnd5e/session/attack_internal_test.go:60-135`
- Modify: `rulebooks/dnd5e/session/events_internal_test.go:145-175`
- Modify: `rulebooks/dnd5e/session/monster_turn_test.go:490-540`

**Interfaces:**
- Consumes: the stable `ENCOUNTER_TAG`; existing `resolution.StrikeOutcome.DamageComponents` and `Folded.AdvantageSources/DisadvantageSources`.
- Produces: session-owned `DamageComponent`, `AttackModifierSource`, and enriched `StruckBody` values for both live and story projections.

- [ ] **Step 1: Create the session-only worktree and pin the released encounter provider**

```bash
cd /home/kirk/game-dev/rpg-toolkit
git fetch origin
git worktree add /home/kirk/game-dev/.pi-worktrees/1239-session-strike-detail \
  -b feat/1239-session-strike-detail origin/main
cd /home/kirk/game-dev/.pi-worktrees/1239-session-strike-detail/rulebooks/dnd5e/session
GOPROXY=direct go get "github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter@${ENCOUNTER_TAG}"
go mod tidy
go test -race ./...
golangci-lint run ./...
```

Expected: the stable encounter tag appears in `go.mod`; baseline checks pass.

- [ ] **Step 2: Write failing projection and replay tests**

Rewrite `TestRecordUsesAggregateFromTypedStrikeOutcome` as `TestRecordProjectsSelectedStrikeDetail`. Construct a `resolution.StrikeOutcome` with:

```go
immunity := 0.0
struck := resolution.StrikeOutcome{
    Roll: 15, Total: 20, TargetAC: 12, Hit: true, Damage: 9,
    DamageComponents: []dnd5eEvents.DamageComponent{
        {
            Source: dnd5eEvents.DamageSourceWeapon,
            SourceRef: refs.Weapons.Longsword(), Dice: "1d8",
            OriginalDiceRolls: []int{2}, FinalDiceRolls: []int{4},
            Rerolls: []dnd5eEvents.RerollEvent{{DieIndex: 0, Before: 2, After: 4, Reason: "ignored"}},
            FlatBonus: 0, DamageType: damage.Slashing,
            Properties: []damage.Property{damage.AddsAttackAbilityModifier}, IsCritical: true,
        },
        {
            Source: dnd5eEvents.DamageSourceMonsterTrait,
            DamageType: damage.Slashing, Multiplier: &immunity,
        },
    },
    Folded: dnd5eEvents.AttackChainEvent{
        AdvantageSources: []dnd5eEvents.AttackModifierSource{
            {SourceRef: refs.Conditions.Hidden(), SourceID: "alice", Reason: "Hidden"},
        },
        DisadvantageSources: []dnd5eEvents.AttackModifierSource{
            {SourceRef: refs.Conditions.Dodging(), SourceID: "bob", Reason: "Dodging"},
        },
    },
}
```

Assert persisted JSON contains only the approved fields, canonical refs, ordered arrays, and a present zero multiplier. Assert it contains none of `original_dice_rolls`, `rerolls`, `properties`, `is_critical`, or `reason`.

Add a `decodeBeat` test for the same rich payload and assert exact `StruckBody` values. Keep the existing complete old payload test and assert its three new slices are empty.

Extend `TestLiveDeliveryAndStoryCatchUpAreByteEqual` to find its `StruckBody` and require non-empty `DamageComponents`; retain the existing whole-event equality assertion.

- [ ] **Step 3: Run focused tests and confirm RED**

```bash
go test ./... -run 'Test(RecordProjectsSelectedStrikeDetail|StruckBodyDecodesReplayDetail|LiveDeliveryAndStoryCatchUpAreByteEqual)' -count=1
```

Expected: compile failures for missing session/encounter detail types and `StruckBody` fields.

- [ ] **Step 4: Add session-owned types and the one projection**

Add documented session types mirroring the primitive contract:

```go
// DamageComponent is the replayable subset of one resolved damage component.
type DamageComponent struct {
    Source     string     `json:"source"`
    SourceRef  string     `json:"source_ref,omitempty"`
    Dice       string     `json:"dice,omitempty"`
    FinalRolls []int      `json:"final_rolls,omitempty"`
    FlatBonus  int        `json:"flat_bonus"`
    DamageType DamageType `json:"damage_type"`
    Multiplier *float64   `json:"multiplier,omitempty"`
}

// AttackModifierSource identifies one replayable advantage/disadvantage source.
type AttackModifierSource struct {
    SourceRef string `json:"source_ref,omitempty"`
    SourceID  string `json:"source_id,omitempty"`
}
```

Add `DamageComponents`, `AdvantageSources`, and `DisadvantageSources` to `StruckBody` with snake-case JSON tags.

In `attack.go`, add focused helpers:

```go
func recordDamageComponents(in []dnd5eEvents.DamageComponent) []encounter.DamageComponent
func recordAttackModifierSources(in []dnd5eEvents.AttackModifierSource) []encounter.AttackModifierSource
```

Each helper preserves order, copies `FinalDiceRolls`, deep-copies `Multiplier`, converts non-nil refs with `String()`, and ignores every excluded field including `Reason`. In `recordFor`, populate the three encounter slices only when `struck.Hit` is true.

In `structBody`, decode the three JSON arrays directly into session-owned fields and copy them into `StruckBody`. Old absent keys naturally decode as empty slices. Do not decode a second representation or inspect raw payload anywhere else.

- [ ] **Step 5: Run focused and full session verification**

```bash
go test ./... -run 'Test(RecordProjectsSelectedStrikeDetail|StruckBodyDecodesReplayDetail|LiveDeliveryAndStoryCatchUpAreByteEqual)' -count=1
go test -race ./...
golangci-lint run ./...
git diff --check
! rg '^replace ' go.mod
```

Expected: all commands pass; session's architecture boundary test continues to accept only session-owned exported types.

- [ ] **Step 6: Commit, push, and open the session PR**

```bash
cd /home/kirk/game-dev/.pi-worktrees/1239-session-strike-detail
git add rulebooks/dnd5e/session
git commit -m 'feat(session): expose replayable strike detail (#1239)'
git push -u origin feat/1239-session-strike-detail
gh pr create --repo KirkDiggler/rpg-toolkit --base main \
  --head feat/1239-session-strike-detail \
  --title 'feat(session): expose replayable strike detail (#1239)' \
  --body 'Projects the approved subset of existing resolution facts through the released encounter carrier. Live and Story use one typed body. Closes #1239.'
```

- [ ] **Step 7: Merge, verify the stable session tag, and record it**

After CI/Copilot approval, squash-merge, fetch the merge commit and tags, and identify the session tag pointing at that commit:

```bash
SESSION_PR=$(gh pr list --repo KirkDiggler/rpg-toolkit --state merged \
  --search '#1239' --json number --jq '.[0].number')
SESSION_MERGE=$(gh pr view "$SESSION_PR" --repo KirkDiggler/rpg-toolkit \
  --json mergeCommit --jq '.mergeCommit.oid')
git -C /home/kirk/game-dev/rpg-toolkit fetch origin --tags
SESSION_TAG=$(git -C /home/kirk/game-dev/rpg-toolkit tag --points-at "$SESSION_MERGE" | \
  rg '^rulebooks/dnd5e/session/v' | sort -V | tail -1)
test -n "$SESSION_TAG"
```

Record `SESSION_TAG` on #1239 and #265. Confirm the merged `session/go.mod` contains `ENCOUNTER_TAG` and no pseudo-version or local override.

---

### Task 4: Map the released session body onto SessionService — rpg-api#836

**Files:**
- Modify: `go.mod`
- Modify: `go.sum`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/convert.go:430-455`
- Modify: `internal/handlers/dnd5e/session/v1alpha1/convert_test.go:332-360,618-665`

**Interfaces:**
- Consumes: published `PROTO_TAG`, `ENCOUNTER_TAG`, and `SESSION_TAG`.
- Produces: direct typed protobuf mapping used by both StreamEvents and GetStory.

- [ ] **Step 1: Create the API worktree from `origin/dev` and pin all released providers**

```bash
cd /home/kirk/game-dev/rpg-api
git fetch origin
git worktree add /home/kirk/game-dev/.pi-worktrees/836-api-strike-detail \
  -b feat/836-api-strike-detail origin/dev
cd /home/kirk/game-dev/.pi-worktrees/836-api-strike-detail
GOPROXY=direct go get "github.com/KirkDiggler/rpg-api-protos/gen/go@${PROTO_TAG}"
GOPROXY=direct go get \
  "github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter@${ENCOUNTER_TAG}" \
  "github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/session@${SESSION_TAG}"
go mod tidy
go test ./internal/handlers/dnd5e/session/v1alpha1/...
```

Expected: the generated types and session detail types compile before converter changes; `go.mod` names real published versions.

- [ ] **Step 2: Write failing converter tests, including zero presence and shared conversion**

Extend the `Struck` arm in `TestEventToProto_TypedBodies` with ordered components and sources. Use `immunity := 0.0`, assert:

```go
require.Len(t, s.GetDamageComponents(), 2)
require.Equal(t, []int32{4}, s.GetDamageComponents()[0].GetFinalRolls())
require.Nil(t, s.GetDamageComponents()[0].Multiplier)
require.NotNil(t, s.GetDamageComponents()[1].Multiplier)
require.Zero(t, s.GetDamageComponents()[1].GetMultiplier())
require.Equal(t, "dnd5e:conditions:hidden", s.GetAdvantageSources()[0].GetSourceRef())
require.Equal(t, "char-1", s.GetAdvantageSources()[0].GetSourceId())
```

Add `TestEventsToProto_RichStruckMatchesDirectConversion`: create one rich `sdk.Event`, call both `eventToProto(in)` and `eventsToProto([]sdk.Event{in})[0]`, and assert `proto.Equal`. This pins GetStory's slice path to the same converter StreamEvents calls.

- [ ] **Step 3: Run focused tests and confirm RED**

```bash
go test ./internal/handlers/dnd5e/session/v1alpha1/... \
  -run 'Test(EventToProto_TypedBodies|EventsToProto_RichStruckMatchesDirectConversion)' -count=1
```

Expected: assertions fail because `setEventBody` leaves all new repeated fields empty.

- [ ] **Step 4: Add direct mapping helpers and wire the Struck arm**

Add:

```go
func damageComponentsToProto(in []sdk.DamageComponent) []*sessionpb.DamageComponent
func attackModifierSourcesToProto(in []sdk.AttackModifierSource) []*sessionpb.AttackModifierSource
```

Map fields one-for-one in input order. Convert `[]int` to `[]int32`, map session's closed damage type through the existing `damageTypeToProto`, and clone a non-nil multiplier pointer so zero remains present. Do not calculate totals, map `Reason`, sort, infer, or decode payload. Set the three fields in the existing `sdk.StruckBody` switch arm.

- [ ] **Step 5: Run focused and repository verification**

```bash
go test ./internal/handlers/dnd5e/session/v1alpha1/... \
  -run 'Test(EventToProto_TypedBodies|EventsToProto_RichStruckMatchesDirectConversion)' -count=1
make pre-commit
make ci-check
git diff --check
! rg '^replace ' go.mod
```

Expected: all checks pass.

- [ ] **Step 6: Commit, push, review, and squash-merge into `dev`**

```bash
git add go.mod go.sum internal/handlers/dnd5e/session/v1alpha1/convert.go \
  internal/handlers/dnd5e/session/v1alpha1/convert_test.go
git commit -m 'feat(session): map replayable strike detail (#836)'
git push -u origin feat/836-api-strike-detail
gh pr create --repo KirkDiggler/rpg-api --base dev --head feat/836-api-strike-detail \
  --title 'feat(session): map replayable strike detail (#836)' \
  --body 'Pins released providers and maps the approved Struck fields through the existing shared event converter. Closes #836.'
```

After CI/Copilot approval, squash-merge into `dev`. Record the merge commit on #265 before the live web gate.

---

### Task 5: Print the typed detail in the existing debug line — rpg-dnd5e-web#805

**Files:**
- Modify: `package.json`
- Modify: `package-lock.json`
- Modify: `src/components/session/debugLogLine.ts:55-140`
- Modify: `src/components/session/debugLogLine.test.ts:1-105`

**Interfaces:**
- Consumes: generated TypeScript SDK at `PROTO_TAG`; typed `Struck.damageComponents`, `advantageSources`, and `disadvantageSources`.
- Produces: one unchanged aggregate line for old events and one deterministic appended suffix for rich events.

- [ ] **Step 1: Create the web worktree from `origin/dev`, install the released proto, and establish the baseline**

```bash
cd /home/kirk/game-dev/rpg-dnd5e-web
git fetch origin
git worktree add /home/kirk/game-dev/.pi-worktrees/805-web-strike-detail \
  -b feat/805-web-strike-detail origin/dev
cd /home/kirk/game-dev/.pi-worktrees/805-web-strike-detail
npm i --save "github:KirkDiggler/rpg-api-protos#${PROTO_TAG}"
npm test -- --run src/components/session/debugLogLine.test.ts
```

Expected: the existing formatter tests pass and the lock file resolves the exact release commit associated with `PROTO_TAG`.

- [ ] **Step 2: Add the failing rich-line test while preserving the old-line test**

Keep the existing struck test unchanged as the old-event fallback. Add a second struck fixture with three damage components, a zero multiplier, one advantage source, and an empty disadvantage list. Extend `names` with `['helper-1', 'Helper']` and assert:

```text
seq=7 clock=42 struck attacker=Toolkit Sandbox Fighter target=Skeleton roll=17 total=20 against=13 damage=6 crit=false attack.ref=dnd5e:weapon:longsword attack.name="Longsword" type=SLASHING components=[{source=weapon ref=dnd5e:weapons:longsword dice=1d8 final_rolls=[4] flat=0 type=SLASHING}, {source=ability ref=dnd5e:abilities:strength final_rolls=[] flat=3 type=SLASHING}, {source=monster_trait ref=dnd5e:monster-traits:immunity final_rolls=[] flat=0 type=SLASHING multiplier=0}] advantage=[{ref=dnd5e:conditions:hidden source=Helper}] disadvantage=[]
```

Assert `line.ids` contains attacker, target, and `helper-1`. Do not put a reason in the fixture.

- [ ] **Step 3: Run the formatter test and confirm RED**

```bash
npm test -- --run src/components/session/debugLogLine.test.ts
```

Expected: the rich-line assertion fails because the formatter still ends after attack identity.

- [ ] **Step 4: Add small pure formatting helpers and append detail only when present**

Import generated `DamageComponent` and `AttackModifierSource` types. Add pure helpers that:

- emit component fields in the design's fixed order;
- omit empty `ref` and `dice` strings;
- always print `final_rolls`, `flat`, and the existing `DamageType` enum name;
- print `multiplier` when it is not `undefined`, including zero;
- resolve non-empty modifier `sourceId` through the existing name map;
- return an empty suffix when all three collections are missing or empty;
- otherwise print `components=[...] advantage=[...] disadvantage=[...]` in that order.

Use `b.damageComponents ?? []`, `b.advantageSources ?? []`, and `b.disadvantageSources ?? []` because hand-built/old fixtures may omit additive fields. Append non-empty modifier source IDs to the existing hover `ids`. Do not sum dice, apply multipliers, infer advantage, or touch `combatBeat.ts`/Story mode.

- [ ] **Step 5: Run targeted and full web verification**

```bash
npm test -- --run src/components/session/debugLogLine.test.ts
npm run ci-check
git diff --check
```

Expected: the rich line passes, the old line remains byte-for-byte unchanged, and all repository checks pass.

- [ ] **Step 6: Commit, push, and open the web PR against `dev`**

```bash
git add package.json package-lock.json \
  src/components/session/debugLogLine.ts \
  src/components/session/debugLogLine.test.ts
git commit -m 'feat(debug-feed): render strike breakdown (#805)'
git push -u origin feat/805-web-strike-detail
gh pr create --repo KirkDiggler/rpg-dnd5e-web --base dev \
  --head feat/805-web-strike-detail \
  --title 'feat(debug-feed): render strike breakdown (#805)' \
  --body 'Renders the approved typed Struck detail in the existing one-line debug feed with no client calculations. Closes #805.'
```

Do not merge until Task 6's branch walk passes.

---

### Task 6: Walk live and catch-up delivery, then close the slice

**Files:**
- Modify after evidence exists: `ideas/debug-feed/strike-breakdown/plan.md`

**Interfaces:**
- Consumes: merged proto/toolkit/API providers and the open web PR branch.
- Produces: observed end-to-end evidence, final merge records, and closed Project 19 slices.

- [ ] **Step 1: Run the local API from merged `dev` and web from the reviewed branch**

```bash
cd /home/kirk/game-dev/rpg-api
git fetch origin dev
git worktree add --detach /home/kirk/game-dev/.pi-worktrees/265-api-acceptance origin/dev
docker build -t rpg-api:local /home/kirk/game-dev/.pi-worktrees/265-api-acceptance
cd /home/kirk/game-dev
docker compose -f rpg-deployment/docker-compose.local-dev.yml \
  -f rpg-deployment/docker-compose.local-api-src.yml up -d rpg-api
# In a second terminal:
cd /home/kirk/game-dev/.pi-worktrees/805-web-strike-detail
npm run dev -- --host 0.0.0.0 --port 3003
```

Expected: API health passes and the branch game is available at `http://localhost:3003/?playerId=toolkit-sandbox-fighter`.

- [ ] **Step 2: Observe one live enriched strike**

Open the game, enter the reference tomb, equip/swing, and inspect Debug mode. Confirm one `struck` line contains ordered `components=[...]`; aggregate `damage=` still equals the authoritative event value; no reason field appears; no client calculation or payload decoding is visible in browser logs.

Capture evidence:

```bash
cd /home/kirk/game-dev
node tools/browser/screenshot.mjs \
  'http://localhost:3003/?playerId=toolkit-sandbox-fighter' \
  /tmp/265-strike-breakdown-live.png
```

Attach `/tmp/265-strike-breakdown-live.png` to web #805; do not dirty another checkout with evidence.

- [ ] **Step 3: Prove catch-up uses the same typed line**

Join `toolkit-sandbox-fighter` (Browser A) and `toolkit-sandbox-barbarian` (Browser B) to one session. After Browser A has a last sequence, put A's DevTools Network panel offline, trigger a strike from B or the driven monster, then restore A online. Confirm rule-6 catch-up supplies the missing `struck` sequence and its rendered text matches Browser B's live line exactly. Record the sequence and both lines on #805/#265. Do not build a new reconnect harness for this slice.

- [ ] **Step 4: Merge web and record implementation evidence**

After the branch walk and CI/Copilot approval, squash-merge web into `dev`. Append a short `## Landed` section to this plan containing the actual proto tag, encounter tag, session tag, API PR/merge commit, web PR/merge commit, and the observed live/catch-up sequence.

Then verify and push the rpg-project tracking PR:

```bash
cd /home/kirk/game-dev/.pi-worktrees/rpg-project-265-strike-debug
./scripts/verify-team-workflow.sh
./scripts/verify-opencode.sh
git diff --check
git add ideas/debug-feed/strike-breakdown/plan.md
git commit -m 'docs(debug-feed): record strike breakdown delivery (#265)'
git push
```

- [ ] **Step 5: Close the coordinated slice without closing the Debug Feed journey**

Merge rpg-project PR #266 after its implementation record is present. Verify #246, #1238, #1239, #836, and #805 are closed/Done; set #265 to Done and close it. Leave journey #235 open because this slice proves struck detail only, not the whole debug-feed journey.
