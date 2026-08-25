# Session Combat Experience Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Promote the approved session-combat concept into the production session route with server-authored Attack/Move/End Turn declarations, owner-private character status, authoritative dice presentation, readable Story, exhaustive developer Debug, and no client-side rules.

**Architecture:** `SessionService.Afford` projects one compiled declaration per executable variant and the exact target preflight facts for that variant; mutating verbs echo a deterministic selector and regenerate the same compiled offer before execution. Owner-private character facts stay on the existing owner-gated `CharacterService.CharacterData`, while the web promotes the concept as a data-source swap over shared components. The event stream remains immediate/best-effort, with persisted `GetStory` polling closing terminal loss and the acting player's Story/result held behind explicit dice release.

**Tech Stack:** Protocol Buffers/Buf, Go 1.x modules (`rpg-toolkit`, `rpg-api`), Connect/gRPC, React 19, TypeScript 5.8, Vitest/Testing Library, Three.js/React Three Fiber, Playwright visual verification.

**Spec:** `ideas/session-combat/experience/design.md`

## Global Constraints

- There is no magic system in this wave. Do not add spells, spell slots, concentration, magical items/resources, magical target shapes, or a magic-oriented extension shelf.
- The only executable combat declarations are Attack, turn-clock Move, and End Turn. Dodge, Dash, Disengage, class-feature activation, and item activation remain non-executable information.
- The web renders provider facts and sends intent. It does not calculate legality, cost, reach, target eligibility, hit, damage, post-hit HP, conditions, features, or resource changes.
- `AttackRef` is the one Attack identity. Its `ref` is full `core.Ref.String()` everywhere in this wave.
- Public member identity stays on `SessionService.GetRoster`; exact self data stays on owner-gated `CharacterService.GetCharacterData` and is flattened directly into existing `CharacterData`.
- Never inspect feature/condition persistence JSON in rpg-api or the web.
- Every player-facing route continues to bind the character/member ID to the authenticated owner. Foreign and missing private characters remain indistinguishable `NOT_FOUND` responses.
- Develop from the consumer proof and approved protos; merge toolkit → rpg-api → web. Kirk alone merges.
- One issue, branch, worktree, and PR per repository for this wave. Do not split one repository's implementation into stacked PRs.
- Branch from fresh `origin/main` for protos/toolkit and fresh `origin/dev` for rpg-api/web. Never branch from a stale local branch.
- Protos merge before implementation branches pin generated output. Never edit the generated branch or generated SDK files by hand.
- Local `replace`/`go.work` overrides are permitted for iteration only. No `replace`, `go.work`, `go.work.sum`, or `local-toolkit/` residue may be committed.
- Keep the rpg-project design PR open through the integrated walk; add implementation amendments to the design before code relying on them merges.

## Wave File Map

### `rpg-api-protos`

- Modify `dnd5e/api/session/v1alpha1/types.proto` — declaration/candidate/target-kind shape, `VERB_END_TURN`, full-ref `AttackRef`, shortfalls.
- Modify `dnd5e/api/session/v1alpha1/service.proto` — selector echo on Attack/Move/EndTurn and updated Afford docs.
- Modify `dnd5e/api/v1alpha2/encounter/types.proto` — direct `CharacterData` status fields and feature/condition/resource views.
- Modify `docs/architecture/components/{session-service,character-service}.md` and `docs/status.md` — current contract and intentional break.

### `rpg-toolkit`

- Modify `rulebooks/dnd5e/events/events.go` — `ConditionBehavior.Ref()`.
- Modify condition/monster-trait implementations listed in Task 3 — canonical refs.
- Modify `rulebooks/dnd5e/features/loader.go` and feature implementations — non-mutating feature status/resource projection.
- Modify `rulebooks/dnd5e/resources/keys.go` — stable Second Wind and Action Surge keys.
- Create `rulebooks/dnd5e/character/status_view.go` and `status_view_test.go` — immutable owner-private status projection.
- Create `rulebooks/dnd5e/session/declaration_id.go` and `declaration_id_test.go` — RFC 8785/full-SHA-256 selectors.
- Create `rulebooks/dnd5e/session/offers.go` and `offers_test.go` — compiled offers, per-verb blockers, target preflight.
- Modify `rulebooks/dnd5e/session/afford.go`, `attack.go`, `move.go`, `turn.go`, `types.go`, `errors.go`, and their tests — projection and selector validation.
- Modify `rulebooks/dnd5e/session/go.mod` / `go.sum` — pinned RFC 8785 implementation.
- Modify `docs/architecture/components/{rulebook-dnd5e,rulebook-dnd5e-session}.md` and `docs/status.md` — status/offer boundaries and module health.

### `rpg-api`

- Modify `internal/orchestrators/character/service.go`, `orchestrator.go`, and `equip_item_test.go` — strict pre-write projection and post-state output.
- Create `internal/orchestrators/character/view.go` and `view_test.go` — one strict `EquipmentView` + `StatusView` application path.
- Modify `internal/handlers/dnd5e/v2/character/character_data.go`, `handler.go`, and `handler_test.go` — direct flattened `CharacterData` mapping.
- Modify `internal/handlers/dnd5e/session/v1alpha1/{afford,attack,move,end_turn,convert,errors}.go` and paired tests — field-for-field selector/declaration translation.
- Modify `go.mod` / `go.sum` — generated proto and final toolkit tags.
- Modify `docs/architecture/components/{character-handler,character-orchestrator,character-v2-handler}.md` and `docs/status.md` — strict view/application and session translation.

### `rpg-dnd5e-web`

- Modify `package.json` / `package-lock.json` — merged proto tag.
- Modify `src/api/useSession{Afford,Attack,EndTurn}.ts` and tests — nested declarations and selector echo.
- Modify `src/api/useGetCharacterData.ts`; create `src/api/useCharacterData.ts` and test — cached last-confirmed private data with refresh.
- Modify `src/components/session/useSessionEventStream.ts` and test — five-second/focus catch-up polling and delivery provenance.
- Create `src/components/session/combat-experience/` shared components, selectors, controller, presentation adapter, Story model, styles, and tests.
- Modify `src/components/session/{SessionEncounterView,useSessionWalk,SessionCanvas}.tsx`/`.ts` and tests — production integration and panel-first targeting.
- Modify `src/concepts/session-combat/` — use the shared production components, remove magic/future-action fixtures, retain contract/evidence controls only.
- Modify `docs/how-to/concepts-route.md`, `docs/architecture/components/combat-v2.md`, and `docs/status.md` — promoted ownership and interaction.

---

### Task 1: Create and board the four implementation slices

**Files:**
- No repository files; this task establishes durable GitHub/Project 19 work before code branches.

**Interfaces:**
- Consumes: approved design issue `rpg-project#270`, design PR `rpg-project#271`, parent journey `rpg-project#253`.
- Produces: four issue URLs and numeric issue IDs exported as `PROTO_ISSUE`, `TOOLKIT_ISSUE`, `API_ISSUE`, `WEB_ISSUE` for every later branch name.

- [ ] **Step 1: Create exact issue bodies with the approved boundaries**

Create four `/tmp/session-combat-*.md` bodies. Each body must reference `rpg-project#270`, parent journey `#253`, the owning repository scope below, the no-magic rule, and the Team signature derived from its board Team:

```text
protos: session Declaration/CharacterData contract only
  Team Platform · Area Game Screen · Kind Build

toolkit: compiled offers/selectors + character StatusView only
  Team Platform · Area The Dungeon · Kind Build

api: pure translation + strict character application only
  Team Platform · Area The Dungeon · Kind Build

web: production promotion + recovery/presentation only
  Team UI/UX · Area Game Screen · Kind Build
```

Every issue's Done-when links to the exact repository gate in Tasks 2–13 and states “Kirk alone merges.”

- [ ] **Step 2: Create issues and capture their numbers**

Run:

```bash
export PROTO_URL=$(gh issue create -R KirkDiggler/rpg-api-protos --title 'build: session combat experience contract' --body-file /tmp/session-combat-protos.md --assignee KirkDiggler)
export TOOLKIT_URL=$(gh issue create -R KirkDiggler/rpg-toolkit --title 'build: session combat offers and character status' --body-file /tmp/session-combat-toolkit.md --assignee KirkDiggler)
export API_URL=$(gh issue create -R KirkDiggler/rpg-api --title 'build: serve production session combat experience' --body-file /tmp/session-combat-api.md --assignee KirkDiggler)
export WEB_URL=$(gh issue create -R KirkDiggler/rpg-dnd5e-web --title 'build: promote session combat experience' --body-file /tmp/session-combat-web.md --assignee KirkDiggler)

export PROTO_ISSUE=${PROTO_URL##*/}
export TOOLKIT_ISSUE=${TOOLKIT_URL##*/}
export API_ISSUE=${API_URL##*/}
export WEB_ISSUE=${WEB_URL##*/}
printf '%s\n' "$PROTO_URL" "$TOOLKIT_URL" "$API_URL" "$WEB_URL"
```

Expected: four open issues assigned to `KirkDiggler`.

- [ ] **Step 3: Parent and board every slice**

Use the current Project 19 IDs already verified by design #270:

```bash
PROJECT_ID=PVT_kwHOAASbwc4Bcj4v
STATUS_FIELD=PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM
AREA_FIELD=PVTSSF_lAHOAASbwc4Bcj4vzhXLt3s
KIND_FIELD=PVTSSF_lAHOAASbwc4Bcj4vzhXLt3w
TEAM_FIELD=PVTSSF_lAHOAASbwc4Bcj4vzhYbEWs
INITIATIVE_FIELD=PVTSSF_lAHOAASbwc4Bcj4vzhf2z3s
READINESS_FIELD=PVTSSF_lAHOAASbwc4Bcj4vzhf2z6Y

STATUS_IN_PROGRESS=a434eab1
AREA_GAME_SCREEN=99f1a1b1
AREA_DUNGEON=0e68c572
KIND_BUILD=ea162471
TEAM_PLATFORM=f9c87bc7
TEAM_UIUX=de75cd8d
INITIATIVE_FOUR_PLAYER=20c80cbf
READINESS_READY=51997600

board_slice() {
  url=$1 team=$2 area=$3
  item=$(gh project item-add 19 --owner KirkDiggler --url "$url" --format json --jq .id)
  gh project item-edit --id "$item" --project-id "$PROJECT_ID" --field-id "$STATUS_FIELD" --single-select-option-id "$STATUS_IN_PROGRESS" >/dev/null
  gh project item-edit --id "$item" --project-id "$PROJECT_ID" --field-id "$AREA_FIELD" --single-select-option-id "$area" >/dev/null
  gh project item-edit --id "$item" --project-id "$PROJECT_ID" --field-id "$KIND_FIELD" --single-select-option-id "$KIND_BUILD" >/dev/null
  gh project item-edit --id "$item" --project-id "$PROJECT_ID" --field-id "$TEAM_FIELD" --single-select-option-id "$team" >/dev/null
  gh project item-edit --id "$item" --project-id "$PROJECT_ID" --field-id "$INITIATIVE_FIELD" --single-select-option-id "$INITIATIVE_FOUR_PLAYER" >/dev/null
  gh project item-edit --id "$item" --project-id "$PROJECT_ID" --field-id "$READINESS_FIELD" --single-select-option-id "$READINESS_READY" >/dev/null
  repo_and_number=${url#https://github.com/}
  repo=${repo_and_number%/issues/*}
  number=${url##*/}
  database_id=$(gh api "repos/$repo/issues/$number" --jq .id)
  gh api --method POST repos/KirkDiggler/rpg-project/issues/253/sub_issues -F sub_issue_id="$database_id" >/dev/null
}

board_slice "$PROTO_URL" "$TEAM_PLATFORM" "$AREA_GAME_SCREEN"
board_slice "$TOOLKIT_URL" "$TEAM_PLATFORM" "$AREA_DUNGEON"
board_slice "$API_URL" "$TEAM_PLATFORM" "$AREA_DUNGEON"
board_slice "$WEB_URL" "$TEAM_UIUX" "$AREA_GAME_SCREEN"

gh project item-list 19 --owner KirkDiggler --limit 1000 --format json \
  --jq '.items[] | select(.content.url == env.PROTO_URL or .content.url == env.TOOLKIT_URL or .content.url == env.API_URL or .content.url == env.WEB_URL) | {url:.content.url,status,team,area,kind,initiative,readiness}'
```

Expected: exactly four complete items; no blank Team/Area/Kind/Initiative/Readiness.

- [ ] **Step 4: Create isolated worktrees from correct remote bases**

Use `superpowers:using-git-worktrees`. Fetch first. Use these external paths so no unignored in-repository worktree directory can enter a diff:

```bash
export PROTO_WORKTREE=/home/kirk/.pi/worktrees/rpg-api-protos/${PROTO_ISSUE}-session-combat-experience
export TOOLKIT_WORKTREE=/home/kirk/.pi/worktrees/rpg-toolkit/${TOOLKIT_ISSUE}-session-combat-experience
export API_WORKTREE=/home/kirk/.pi/worktrees/rpg-api/${API_ISSUE}-session-combat-experience
export WEB_WORKTREE=/home/kirk/.pi/worktrees/rpg-dnd5e-web/${WEB_ISSUE}-session-combat-experience

mkdir -p "$(dirname "$PROTO_WORKTREE")" "$(dirname "$TOOLKIT_WORKTREE")" \
         "$(dirname "$API_WORKTREE")" "$(dirname "$WEB_WORKTREE")"

git -C /home/kirk/game-dev/rpg-api-protos fetch origin
git -C /home/kirk/game-dev/rpg-api-protos worktree add "$PROTO_WORKTREE" -b "feat/${PROTO_ISSUE}-session-combat-experience" origin/main

git -C /home/kirk/game-dev/rpg-toolkit fetch origin
git -C /home/kirk/game-dev/rpg-toolkit worktree add "$TOOLKIT_WORKTREE" -b "feat/${TOOLKIT_ISSUE}-session-combat-experience" origin/main

git -C /home/kirk/game-dev/rpg-api fetch origin
git -C /home/kirk/game-dev/rpg-api worktree add "$API_WORKTREE" -b "feat/${API_ISSUE}-session-combat-experience" origin/dev

git -C /home/kirk/game-dev/rpg-dnd5e-web fetch origin
git -C /home/kirk/game-dev/rpg-dnd5e-web worktree add "$WEB_WORKTREE" -b "feat/${WEB_ISSUE}-session-combat-experience" origin/dev
```

Run repository baselines before edits:

```bash
(cd "$PROTO_WORKTREE" && buf lint && buf format --diff --exit-code)
(cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e" && go test ./...)
(cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e/session" && go test ./...)
(cd "$API_WORKTREE" && go test ./internal/handlers/dnd5e/session/v1alpha1 ./internal/handlers/dnd5e/v2/character ./internal/orchestrators/character)
(cd "$WEB_WORKTREE" && npm install && npm run test:run -- src/components/session src/concepts/session-combat src/api/useSessionAfford.test.ts)
```

Expected: all baseline commands exit 0. Stop and report any pre-existing failure before implementation.

---

### Task 2: Land the proto contract first

**Files:**
- Modify: `dnd5e/api/session/v1alpha1/types.proto`
- Modify: `dnd5e/api/session/v1alpha1/service.proto`
- Modify: `dnd5e/api/v1alpha2/encounter/types.proto`
- Modify: `docs/architecture/components/session-service.md`
- Modify: `docs/architecture/components/character-service.md`
- Modify: `docs/status.md`

**Interfaces:**
- Consumes: exact wire sketch and literal values from `ideas/session-combat/experience/design.md`.
- Produces: `Declaration.attack: AttackRef`, nested `TargetCandidate`, `TargetKind`, selector-bearing requests, full-ref `AttackRef`, and flattened `CharacterData` fields consumed by Tasks 4–13.

- [ ] **Step 1: Reshape the session declaration vocabulary**

In `types.proto`, add exactly:

```proto
enum TargetKind {
  TARGET_KIND_UNSPECIFIED = 0;
  TARGET_KIND_NONE = 1;
  TARGET_KIND_MEMBER = 2;
  TARGET_KIND_PATH = 3;
}

message TargetCandidate {
  string member = 1;
  bool available = 2;
  Shortfall why = 3;
}
```

Add `VERB_END_TURN = 3` and `SHORTFALL_REASON_TARGET_OUT_OF_REACH = 6`. Reshape `Declaration`:

```proto
message Declaration {
  Verb verb = 1;
  Slot slot = 2;
  bool available = 3;
  reserved 4, 6;
  reserved "shortfall", "target";
  optional int32 remaining = 5;
  Shortfall why = 7;
  string id = 8;
  AttackRef attack = 9;
  TargetKind target_kind = 10;
  repeated TargetCandidate candidates = 11;
}
```

Update `AttackRef.ref` docs to require full `core.Ref.String()` and update `UNREADABLE` docs to cover the per-verb character/action dependency matrix. Do not add magic values or future target kinds.

- [ ] **Step 2: Add selector echo to current verbs**

In `service.proto`, add:

```proto
message AttackRequest {
  string session = 1;
  string attacker = 2;
  string target = 3;
  string declaration_id = 4;
}

message MoveRequest {
  string session = 1;
  string member = 2;
  repeated Position path = 3;
  string declaration_id = 4;
}

message EndTurnRequest {
  string session = 1;
  string member = 2;
  string declaration_id = 3;
}
```

Document: Attack/EndTurn require non-empty selectors; turn-clock Move requires one; world-clock Move requires empty; a non-empty selector arriving after transition to world clock is stale and must not become a free move.

- [ ] **Step 3: Flatten owner-private status directly into CharacterData**

In `dnd5e/api/v1alpha2/encounter/types.proto`, add direct fields 9–14 and the three display messages:

```proto
message CharacterData {
  // existing 1-8 unchanged
  int32 level = 9;
  HitPoints hit_points = 10;
  int32 base_speed_feet = 11;
  repeated FeatureView features = 12;
  repeated ConditionView conditions = 13;
  repeated ResourceView resources = 14;
}

message FeatureView {
  Ref ref = 1;
  string name = 2;
  string detail = 3;
  optional string resource_key = 4;
}

message ConditionView {
  Ref ref = 1;
  string name = 2;
  string detail = 3;
  optional string source_member = 4;
}

message ResourceView {
  string key = 1;
  string name = 2;
  int32 current = 3;
  int32 maximum = 4;
}
```

Document that `HitPoints.temp` is zero until toolkit character state owns temporary HP; `resources` excludes `SpellSlots` and legacy `ClassResources`.

- [ ] **Step 4: Update contract/status documentation**

Record the nested declaration semantics, full-ref AttackRef migration, direct CharacterData fields, no-magic boundary, and in-place breaking label in the two component docs and `docs/status.md`. Remove old claims that Afford is flat or CharacterData is equipment-only.

- [ ] **Step 5: Run contract gates**

Run:

```bash
cd "$PROTO_WORKTREE"
buf format -w
buf lint
buf format --diff --exit-code
buf breaking --against 'https://github.com/KirkDiggler/rpg-api-protos.git#branch=main'
buf generate
```

Expected: format/lint/generate PASS. `buf breaking` reports the intentional `Declaration` source break; no unrelated break is accepted.

- [ ] **Step 6: Commit, open the PR, and mark the intentional break**

```bash
cd "$PROTO_WORKTREE"
git add dnd5e/api/session/v1alpha1/types.proto \
        dnd5e/api/session/v1alpha1/service.proto \
        dnd5e/api/v1alpha2/encounter/types.proto \
        docs/architecture/components/session-service.md \
        docs/architecture/components/character-service.md docs/status.md
git commit -m "feat(session): add production combat experience contract (#${PROTO_ISSUE})"
git push -u origin "feat/${PROTO_ISSUE}-session-combat-experience"
export PROTO_PR_URL=$(gh pr create --base main --title 'feat(session): production combat experience contract' --body-file /tmp/session-combat-protos-pr.md)
gh pr edit "$PROTO_PR_URL" --add-label breaking-change-approved
```

Expected: PR targets `main`, references the owning proto issue and design #270, and contains no generated files.

- [ ] **Step 7: Human merge checkpoint and generated release verification**

Kirk reviews and merges the proto PR. After CI publishes generated output:

```bash
export PROTO_MERGE_SHA=$(gh pr view "$PROTO_PR_URL" --json mergeCommit --jq .mergeCommit.oid)
git -C "$PROTO_WORKTREE" fetch origin --tags
export PROTO_TAG=$(git -C "$PROTO_WORKTREE" tag --points-at "$PROTO_MERGE_SHA" | grep '^v[0-9]' | sort -V | tail -1)
export PROTO_GENERATED_SHA=$(gh api repos/KirkDiggler/rpg-api-protos/branches/generated --jq .commit.sha)
test -n "$PROTO_TAG" && test -n "$PROTO_GENERATED_SHA"
printf 'PROTO_TAG=%s PROTO_GENERATED_SHA=%s\n' "$PROTO_TAG" "$PROTO_GENERATED_SHA"
```

Record both values in the owning issue. Do not begin committed consumer pins until the generated artifact exists.

---

### Task 3: Give active conditions and class features a non-JSON status surface

**Files:**
- Modify: `rulebooks/dnd5e/events/events.go`
- Modify: `rulebooks/dnd5e/conditions/{brutal_critical,disengaging,dodging,fighting_style_archery,fighting_style_defense,fighting_style_dueling,fighting_style_great_weapon_fighting,fighting_style_protection,fighting_style_two_weapon_fighting,helped,hidden,improved_critical,martial_arts,opportunity_attack,prone,raging,reckless_attack,shield_spell,sneak_attack,unarmored_defense,unarmored_movement,unconscious}.go`
- Modify: `rulebooks/dnd5e/monstertraits/{immunity,pack_tactics,undead_fortitude,vulnerability}.go`
- Modify: test stubs implementing `ConditionBehavior` under `character/`, `monster/`, and `resolution/`
- Modify: `rulebooks/dnd5e/features/loader.go`
- Modify: `rulebooks/dnd5e/features/{action_surge,deflect_missiles,flurry_of_blows,patient_defense,rage,reckless_attack,second_wind,step_of_the_wind}.go`
- Modify: `rulebooks/dnd5e/resources/keys.go`
- Create: `rulebooks/dnd5e/features/status.go`
- Test: `rulebooks/dnd5e/features/status_test.go`
- Test: `rulebooks/dnd5e/conditions/loader_test.go`

**Interfaces:**
- Consumes: existing `core.Ref`, `core/resources.ResourceKey`, feature-private `RecoverableResource`, character-owned Rage/Ki pools.
- Produces: `ConditionBehavior.Ref() *core.Ref`, `features.StatusProvider`, `features.StatusInput`, `features.StatusOutput`, `features.Status`, `features.ResourceStatus`, and stable `resources.SecondWind` / `resources.ActionSurge` keys for Task 4.

- [ ] **Step 1: Write failing identity/status tests**

Add tests proving:

```go
func TestEveryLoadedConditionNamesItsCanonicalRef(t *testing.T) {
    // Iterate loader fixtures/registry; require condition.Ref().String()
    // equals the ref used to load it.
}

func TestSecondWindStatusReportsPrivateResourceWithoutJSON(t *testing.T) {
    out, err := secondWind.Status(&features.StatusInput{})
    require.NoError(t, err)
    require.Equal(t, resources.SecondWind, out.Status.Resource.Key)
    require.Equal(t, 1, out.Status.Resource.Current)
    require.Equal(t, 1, out.Status.Resource.Maximum)
}

func TestKiFeaturesReportOneSharedResourceKey(t *testing.T) {
    // Flurry/Patient Defense/Step of the Wind all report resources.Ki.
}
```

Run:

```bash
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e"
go test ./conditions ./features
```

Expected: FAIL because `Ref`, status types, and keys do not exist.

- [ ] **Step 2: Add ConditionBehavior.Ref and canonical implementations**

Change the interface:

```go
type ConditionBehavior interface {
    Ref() *core.Ref
    IsApplied() bool
    Apply(context.Context, events.EventBus) error
    Remove(context.Context, events.EventBus) error
    ToJSON() (json.RawMessage, error)
}
```

Each production condition/trait returns its existing canonical `refs.Conditions.*()` or `refs.MonsterTraits.*()` value. Update compile-only test stubs with explicit fixture refs; do not return nil just to satisfy compilation.

Run:

```bash
go test ./conditions ./monstertraits ./character ./monster ./resolution
```

Expected: PASS; no implementation remains unidentified.

- [ ] **Step 3: Add stable non-magical feature resource status**

In `resources/keys.go` add:

```go
SecondWind coreResources.ResourceKey = "second_wind"
ActionSurge coreResources.ResourceKey = "action_surge"
```

In `features/status.go`, define immutable value types:

```go
type ResourceReader interface {
    ResourceStatus(coreResources.ResourceKey) (current, maximum int, ok bool)
}

type StatusInput struct { Owner ResourceReader }
type ResourceStatus struct {
    Key coreResources.ResourceKey
    Name string
    Current int
    Maximum int
}
type Status struct {
    Ref core.Ref
    Name string
    Detail string
    Resource *ResourceStatus
}
type StatusOutput struct { Status *Status }
type StatusProvider interface { Status(*StatusInput) (*StatusOutput, error) }
```

Make `Feature` embed `StatusProvider`. Private-resource features read their own resource; Rage/Ki features ask `Owner`; no implementation serializes `ToJSON`. Detail strings are server-authored but may be empty; names never are.

- [ ] **Step 4: Run focused tests and commit**

```bash
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e"
go test ./conditions ./monstertraits ./features ./character ./monster ./resolution
golangci-lint run ./conditions/... ./monstertraits/... ./features/... ./character/... ./monster/... ./resolution/...
cd "$TOOLKIT_WORKTREE"
git add rulebooks/dnd5e/events rulebooks/dnd5e/conditions \
        rulebooks/dnd5e/monstertraits rulebooks/dnd5e/features \
        rulebooks/dnd5e/resources rulebooks/dnd5e/character \
        rulebooks/dnd5e/monster rulebooks/dnd5e/resolution
git commit -m "feat(dnd5e): expose character effect status (#${TOOLKIT_ISSUE})"
```

Expected: tests/lint PASS and the commit touches no session module yet.

---

### Task 4: Build immutable Character StatusView

**Files:**
- Create: `rulebooks/dnd5e/character/status_view.go`
- Create: `rulebooks/dnd5e/character/status_view_test.go`
- Create or modify: `rulebooks/dnd5e/conditions/display.go` and `display_test.go`
- Modify: `rulebooks/dnd5e/character/character.go` — `ResourceStatus` reader implementation only

**Interfaces:**
- Consumes: Task 3 condition refs, feature status providers, resource keys.
- Produces: `character.StatusView`, `FeatureView`, `ConditionView`, `ResourceView`, and `(*Character).StatusView(*StatusViewInput) (*StatusViewOutput, error)` consumed by rpg-api Task 8.

- [ ] **Step 1: Write failing status-view tests for the four builds**

Cover Fighter, Barbarian, Monk, and Rogue fixtures:

```go
func TestStatusViewProjectsFighterWithoutPersistenceJSON(t *testing.T) {
    fighter := newLevel3Fighter(t) // test helper finalizes the existing Fighter draft fixture
    out, err := fighter.StatusView(&StatusViewInput{})
    require.NoError(t, err)
    view := out.View
    require.Equal(t, fighter.GetLevel(), view.Level)
    require.Equal(t, fighter.GetHitPoints(), view.HitPoints.Current)
    require.Contains(t, resourceKeys(view.Resources), resources.SecondWind)
    require.Contains(t, resourceKeys(view.Resources), resources.ActionSurge)
}
```

Add separate tests with exact assertions: the level-3 Monk returns exactly one `resources.Ki` row despite three consuming features; injected duplicate key reports an error when counts differ; an unknown condition ref returns an error and no partial view; non-empty `Data.SpellSlots`/`Data.ClassResources` never produces a status resource. Define `newLevel3Fighter`, `newLevel3Barbarian`, `newLevel3Monk`, and `newLevel3Rogue` in `status_view_test.go` by reusing the existing draft-finalization fixtures, not hand-authoring runtime feature objects.

Run `cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e" && go test ./character -run StatusView -count=1`.

Expected: FAIL because `StatusView` does not exist.

- [ ] **Step 2: Implement rulebook-owned condition display descriptors**

`conditions/display.go` maps canonical condition refs reachable by the four builds to display-ready names and optional details. It must include the current fighting styles, Rage/Raging, Martial Arts, Unarmored Defense/Movement, Sneak Attack, Brutal/Improved Critical, Reckless Attack, Dodging, Disengaging, Hidden, Helped, Prone, Opportunity Attack, and Unconscious. Unknown refs return `(Display{}, false)`; the character projection turns that into an error.

Do not add spell-slot or magic-oriented status fields. Existing condition code outside the selected builds is only given `Ref()` for interface completeness.

- [ ] **Step 3: Implement StatusView with deterministic ordering**

Define:

```go
type HitPointView struct { Current, Maximum int }
type FeatureView struct { Ref core.Ref; Name, Detail string; ResourceKey *coreResources.ResourceKey }
type ConditionView struct { Ref core.Ref; Name, Detail string; SourceMember *string }
type ResourceView struct { Key coreResources.ResourceKey; Name string; Current, Maximum int }
type StatusView struct {
    Level int
    HitPoints HitPointView
    BaseSpeedFeet int
    Features []FeatureView
    Conditions []ConditionView
    Resources []ResourceView
}
type StatusViewInput struct{}
type StatusViewOutput struct { View *StatusView }
```

Sort features/conditions by `Ref.String()` and resources by key. Validate non-empty refs/names, non-negative resource counts, `current <= maximum`, and conflicting duplicate keys. Return detached values; never expose live feature/resource pointers.

- [ ] **Step 4: Run focused and module gates; commit**

```bash
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e"
go test ./character ./conditions ./features -count=1
golangci-lint run ./character/... ./conditions/... ./features/...
cd "$TOOLKIT_WORKTREE"
git add rulebooks/dnd5e/character rulebooks/dnd5e/conditions
git commit -m "feat(character): project owner-private status (#${TOOLKIT_ISSUE})"
```

Expected: PASS. `git diff --check` reports no whitespace errors.

---

### Task 5: Implement canonical declaration selectors

**Files:**
- Create: `rulebooks/dnd5e/session/declaration_id.go`
- Create: `rulebooks/dnd5e/session/declaration_id_test.go`
- Modify: `rulebooks/dnd5e/session/go.mod`
- Modify: `rulebooks/dnd5e/session/go.sum`

**Interfaces:**
- Consumes: validated `actions.Definition`, exact `session.Verb`/`Slot` strings.
- Produces: `declarationID(input declarationIDInput) (string, error)` and `indexCompiledOffers` collision guard consumed by Task 6.

- [ ] **Step 1: Add failing RFC 8785 golden tests**

Use exact golden selectors:

```go
func TestMoveDeclarationIDGolden(t *testing.T) {
    got, err := declarationID(declarationIDInput{
        Session: "session-1", Member: "fighter-1",
        Verb: VerbMove, Slot: SlotNone,
    })
    require.NoError(t, err)
    require.Equal(t, "v1.Mhnl9aRJjeAvMxtlbRFFHVH-XoMkgR4l1pasCSyzrjc", got)
}

func TestEndTurnDeclarationIDGolden(t *testing.T) {
    // Expected: v1.yZ1FHWnV7SnulpNVV4kz1BSkslDxfXr-lEINSahGUDo
}
```

Also test map insertion order, nil-vs-empty `omitempty` normalization, embedded raw JSON canonicalization, changed profile → changed ID, same state recurrence, full 43-character digest payload, and duplicate-ID fail-closed through an injected test ID function.

Run `go test ./... -run DeclarationID -count=1` from `rulebooks/dnd5e/session`.

Expected: FAIL because selector code is absent.

- [ ] **Step 2: Add the vetted RFC 8785 dependency**

```bash
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e/session"
go get github.com/cyberphone/json-canonicalization@v0.0.0-20241213102144-19d51d7fe467
go mod tidy
```

Import `github.com/cyberphone/json-canonicalization/go/src/webpki.org/jsoncanonicalizer` and use `jsoncanonicalizer.Transform`; do not implement a partial canonicalizer.

- [ ] **Step 3: Implement the exact selector document**

Use a struct with JSON keys `domain`, `session`, `member`, `verb`, `slot`, `variant`; serialize the complete validated Attack definition into `json.RawMessage`, use sealed strings for Move/EndTurn, canonicalize, SHA-256 hash, and encode `v1.` + unpadded base64url. Reject unsupported verb/slot strings and malformed raw JSON.

- [ ] **Step 4: Run tests and commit**

```bash
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e/session"
go test ./... -run 'DeclarationID|CompiledOfferCollision' -count=1
golangci-lint run ./...
cd "$TOOLKIT_WORKTREE"
git add rulebooks/dnd5e/session/declaration_id.go \
        rulebooks/dnd5e/session/declaration_id_test.go \
        rulebooks/dnd5e/session/go.mod rulebooks/dnd5e/session/go.sum
git commit -m "feat(session): add canonical declaration selectors (#${TOOLKIT_ISSUE})"
```

Expected: exact golden tests PASS; no local replace exists.

---

### Task 6: Compile and project current offers

**Files:**
- Create: `rulebooks/dnd5e/session/offers.go`
- Create: `rulebooks/dnd5e/session/offers_test.go`
- Modify: `rulebooks/dnd5e/session/afford.go`
- Modify: `rulebooks/dnd5e/session/types.go`
- Modify: `rulebooks/dnd5e/session/afford_test.go`
- Modify: `rulebooks/dnd5e/session/attack.go` / `attack_test.go` — full-ref `AttackRef` projection

**Interfaces:**
- Consumes: Task 5 selectors, current clock/standing/holdings/positions, `character.AssembleAttack`.
- Produces: internal `compiledOffer`, public nested `Declaration`, one shared target preflight, and `compileOffers` consumed by Task 7.

- [ ] **Step 1: Replace flat-declaration tests with failing nested-offer tests**

Add tests for:

```go
func TestAffordReturnsOneAttackOfferWithEveryLiveCandidate(t *testing.T) {
    out := runAffordCandidateFixture(t) // helper creates in-range, out-of-range, stale, and self holdings
    attack := requireSingleAttackDeclaration(t, out.Declarations)
    require.Equal(t, "dnd5e:weapons:longsword", attack.Attack.Ref)
    require.Equal(t, TargetMember, attack.TargetKind)
    require.Equal(t, []string{"skeleton-near", "skeleton-far"}, candidateIDs(attack.Candidates))
    require.True(t, attack.Candidates[0].Available)
    require.Equal(t, ShortfallTargetOutOfReach, attack.Candidates[1].Why.Reason)
}
```

Add table tests that assert: unreadable Attack blocks only Attack; unreadable character blocks Attack/Move but not clock-valid End Turn; every blocked declaration has empty ID and fixed target kind; compiled Move/EndTurn have non-empty selectors; a live candidate missing position makes Afford return an error. Define the four named test helpers in `offers_test.go` so assertions do not depend on declaration ordering beyond the documented deterministic sort.

Run `cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e/session" && go test ./... -run 'Afford|Offer|Blocker' -count=1`.

Expected: compile/test failures against the old flat shape.

- [ ] **Step 2: Define the exported session contract types**

Add string enums with exact literals:

```go
const (
    VerbAttack Verb = "attack"
    VerbMove Verb = "move"
    VerbEndTurn Verb = "end_turn"
    TargetNone TargetKind = "none"
    TargetMember TargetKind = "member"
    TargetPath TargetKind = "path"
)
```

`Declaration` carries `Available`, `Why`, `Remaining`, `ID`, `Attack *AttackRef`, `TargetKind`, and `Candidates []TargetCandidate`. Preserve non-nil empty candidate slices on Move/EndTurn/world responses.

- [ ] **Step 3: Implement one compiled-offer builder and target preflight**

In `offers.go`, keep inner toolkit types private:

```go
type compiledOffer struct {
    declaration Declaration
    attack *combatActions.Definition
    targets map[string]targetPreflight
}
```

Build EndTurn from clock first, then independently build Move and Attack. Enumerate every holding with `CurrentVia` non-empty except self; missing live position is an error. Sort candidates by member ID before projection. Shared target preflight emits `TARGET_OUT_OF_REACH`; top-level Attack is available only when global gates pass and at least one candidate is available.

Return per-verb `UNREADABLE` blockers rather than an incomplete declaration list. Keep session/world load failures as hard errors.

- [ ] **Step 4: Project full AttackRef once**

Change `attackRefFor` to:

```go
ref := AttackRef{Ref: definition.Ref.String(), Name: definition.Name}
```

Use the same value in Declaration, AttackOutput, and encounter beat identity. Update all fixtures expecting bare `longsword` to full `dnd5e:weapons:longsword`.

- [ ] **Step 5: Run focused tests and commit**

```bash
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e/session"
go test ./... -run 'Afford|Offer|Blocker|AttackRef' -count=1
golangci-lint run ./...
cd "$TOOLKIT_WORKTREE"
git add rulebooks/dnd5e/session
git commit -m "feat(session): project compiled combat offers (#${TOOLKIT_ISSUE})"
```

Expected: nested offer tests PASS; old flat-target assumptions are removed rather than adapted in parallel.

---

### Task 7: Revalidate selectors at Attack, Move, and EndTurn

**Files:**
- Modify: `rulebooks/dnd5e/session/{attack,move,turn,errors}.go`
- Modify: paired tests plus `boundary_test.go` and sentinel-count tests
- Modify: `rulebooks/dnd5e/session/doc.go` if exported error/input contract counts are documented
- Modify: `docs/architecture/components/rulebook-dnd5e.md`
- Modify: `docs/architecture/components/rulebook-dnd5e-session.md`
- Modify: `docs/status.md`

**Interfaces:**
- Consumes: Task 6 `compileOffers` and selector-index collision guard.
- Produces: `AttackInput.DeclarationID`, `MoveInput.DeclarationID`, `EndTurnInput.DeclarationID`, `ErrNoDeclarationID`, `ErrStaleDeclaration` consumed by rpg-api Task 9.

- [ ] **Step 1: Write failing execution-gate tests**

Start with the no-mutation stale-selector gate:

```go
func TestStaleSelectorRollsNothingWritesNothingAndRecordsNothing(t *testing.T) {
    fixture := newAttackFixture(t)
    out, err := fixture.manager.Attack(fixture.ctx, &AttackInput{
        Session: fixture.sessionID, Attacker: fixture.fighterID,
        Target: fixture.skeletonID, DeclarationID: "v1.stale",
    })
    require.ErrorIs(t, err, ErrStaleDeclaration)
    require.Nil(t, out)
    require.Equal(t, 0, fixture.dice.RollCount())
    require.Empty(t, fixture.characterRepo.Writes())
    require.Empty(t, fixture.storyRepo.Entries())
}
```

Add focused tests for missing Attack ID, candidate from another offer, turn-clock Move ID requirement, empty-ID world Move, stale turn selector after transition to world (no movement), current EndTurn selector, and a shared target-preflight seam whose injected refusal changes both Afford and Attack. Reuse existing session fixture repositories/dice seams; add counters only to those fakes rather than creating mock-only production interfaces.

Run `cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e/session" && go test ./... -run 'Declaration|Selector|Stale|Preflight' -count=1`.

Expected: FAIL because inputs and sentinels are absent.

- [ ] **Step 2: Add selector inputs and sentinels**

Add `DeclarationID string` to all three inputs. `ErrNoDeclarationID` is invalid input; `ErrStaleDeclaration` is current-world refusal. Update exported sentinel/boundary allow-list tests rather than weakening them.

- [ ] **Step 3: Recompile under the existing verb load/lock**

For each turn-clock verb, regenerate offers from the already-loaded state, find exactly one matching ID, recheck target/path against it, then proceed. Resolution retains final defensive validation. World Move accepts empty ID only; a non-empty stale turn ID returns `ErrStaleDeclaration` before walking any cell.

- [ ] **Step 4: Update toolkit architecture/status docs**

Document `ConditionBehavior.Ref`, feature/resource StatusView, compiled offers/selectors, per-verb blocker matrix, full-ref AttackRef, and the two changed module boundaries. Update `docs/status.md` without predicting release versions.

- [ ] **Step 5: Run full affected module gates**

```bash
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e/session"
go test ./... -count=1
golangci-lint run ./...
"$TOOLKIT_WORKTREE/scripts/verify.sh" rulebooks/dnd5e/session
cd "$TOOLKIT_WORKTREE/rulebooks/dnd5e"
go test ./... -count=1
golangci-lint run ./...
cd "$TOOLKIT_WORKTREE"
git diff --check
```

Expected: all PASS; `git grep 'replace ' -- '*/go.mod'` shows no new local replacement.

- [ ] **Step 6: Commit and open one toolkit PR**

```bash
cd "$TOOLKIT_WORKTREE"
git add rulebooks/dnd5e docs/architecture/components/rulebook-dnd5e.md \
        docs/architecture/components/rulebook-dnd5e-session.md docs/status.md
git commit -m "feat(session): execute echoed combat declarations (#${TOOLKIT_ISSUE})"
git push -u origin "feat/${TOOLKIT_ISSUE}-session-combat-experience"
export TOOLKIT_PR_URL=$(gh pr create --base main --title 'feat(session): production combat offers and character status' --body-file /tmp/session-combat-toolkit-pr.md)
```

PR body lists both affected modules, exact verification commands, proto version, no-magic boundary, and design #270. Do not split root/status and session/offers into separate PRs.

- [ ] **Step 7: Publish RCs for API integration**

Publish pre-release tags from the reviewed toolkit PR head so one API branch can consume both changed Go modules without committing local path overrides:

```bash
cd "$TOOLKIT_WORKTREE"
export TOOLKIT_HEAD=$(git rev-parse HEAD)
next_minor_rc() {
  python3 - "$1" <<'PY'
import sys
major, minor, _patch = map(int, sys.argv[1].removeprefix('v').split('.'))
print(f"v{major}.{minor + 1}.0-rc1")
PY
}
LATEST_DND5E=$(git tag -l 'rulebooks/dnd5e/v*' --sort=-v:refname | grep -v -- '-rc' | head -1 | sed 's#rulebooks/dnd5e/##')
LATEST_SESSION=$(git tag -l 'rulebooks/dnd5e/session/v*' --sort=-v:refname | grep -v -- '-rc' | head -1 | sed 's#rulebooks/dnd5e/session/##')
export DND5E_VERSION=$(next_minor_rc "$LATEST_DND5E")
export SESSION_VERSION=$(next_minor_rc "$LATEST_SESSION")
git tag "rulebooks/dnd5e/$DND5E_VERSION" "$TOOLKIT_HEAD"
git tag "rulebooks/dnd5e/session/$SESSION_VERSION" "$TOOLKIT_HEAD"
git push origin "rulebooks/dnd5e/$DND5E_VERSION" "rulebooks/dnd5e/session/$SESSION_VERSION"
```

Verify both tags point at `$TOOLKIT_HEAD`. API Task 8 pins these RCs; Task 9 replaces them with merge-generated final tags before the API PR is review-ready.

---

### Task 8: Make character reads and equipment writes strict before persistence

**Files:**
- Create: `internal/orchestrators/character/view.go`
- Create: `internal/orchestrators/character/view_test.go`
- Modify: `internal/orchestrators/character/service.go`
- Modify: `internal/orchestrators/character/orchestrator.go`
- Modify: `internal/orchestrators/character/equip_item_test.go`
- Modify: `internal/handlers/dnd5e/v2/character/character_data.go`
- Modify: `internal/handlers/dnd5e/v2/character/handler.go`
- Modify: `internal/handlers/dnd5e/v2/character/handler_test.go`

**Interfaces:**
- Consumes: toolkit `character.StatusView`/`EquipmentView`, merged proto `CharacterData`.
- Produces: internal `character.View`, strict project-before-write application path, full proto mapping consumed by web Task 10.

- [ ] **Step 1: Pin merged proto and toolkit artifacts**

After proto generation and toolkit RC/final tags exist:

```bash
cd "$API_WORKTREE"
GOPROXY=direct go get github.com/KirkDiggler/rpg-api-protos/gen/go@generated
GOPROXY=direct go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@${DND5E_VERSION}
GOPROXY=direct go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/session@${SESSION_VERSION}
go mod tidy
```

Verify `go.mod` contains published versions and no `replace`.

- [ ] **Step 2: Write failing no-write and projection tests**

Add the no-write regression first:

```go
func (s *EquipItemTestSuite) TestMalformedConditionWritesNothing() {
    entity := s.fighterWithLongswordAndShield()
    entity.Data.Conditions = []json.RawMessage{json.RawMessage(`{"ref":{"module":"dnd5e","type":"conditions","id":"unknown"}}`)}
    s.mockCharacterRepo.EXPECT().Get(s.ctx, characterrepo.GetInput{ID: s.testCharacterID}).
        Return(&characterrepo.GetOutput{Character: entity}, nil)
    // No Update expectation: gomock fails if a write occurs.
    out, err := s.orchestrator.EquipItem(s.ctx, &EquipItemInput{
        CharacterID: s.testCharacterID, ItemID: "longsword", Slot: character.SlotMainHand,
    })
    s.Require().Error(err)
    s.Nil(out)
}
```

Add separate tests that inject a failing post-state descriptor and prove no Update; compare successful `EquipItemOutput.View` against the data captured by Update; and project the level-3 Fighter into equipment plus level/HP/speed/features/conditions/resources. Handler tests assert level, HP, base speed, full refs, conditions, feature-resource relation, and no spell/legacy resource rows.

Run:

```bash
cd "$API_WORKTREE"
go test ./internal/orchestrators/character ./internal/handlers/dnd5e/v2/character -count=1
```

Expected: FAIL against forgiving `LoadFromData` and equipment-only CharacterData.

- [ ] **Step 3: Implement one strict internal View**

In `view.go`:

```go
type View struct {
    Equipment *tkcharacter.EquipmentView
    Status *tkcharacter.StatusView
}
type ProjectViewInput struct { Data *tkcharacter.Data }
type ProjectViewOutput struct { View *View }
type ProjectLoadedCharacterInput struct { Character *tkcharacter.Character }
type ProjectLoadedCharacterOutput struct { View *View }

func ProjectView(ctx context.Context, input *ProjectViewInput) (*ProjectViewOutput, error)
func projectLoadedCharacter(ctx context.Context, input *ProjectLoadedCharacterInput) (*ProjectLoadedCharacterOutput, error)
```

`ProjectView` uses strict `tkcharacter.Load` followed by `tkcharacter.Attach`—never forgiving `LoadFromData`—then delegates to `projectLoadedCharacter`; Equip/Unequip call the same internal projector on their already-loaded/mutated sheet. The loaded helper's Input carries the character and its Output carries `View`. Keep Input/Output structs. Return detached projections; do not expose a live `*Character` outside the orchestrator operation.

- [ ] **Step 4: Compose post-state before repository Update**

Equip/Unequip flow:

```text
repository Get → strict load/attach → validate pre-view → mutate in memory
→ build merged persisted data + complete post-view → repository Update
→ return already-composed post-view
```

No response path performs a repository reload after Update. Update outputs include the post-view so handlers do not call `recomputedCharacterData`.

- [ ] **Step 5: Map flattened CharacterData and preserve ownership**

`BuildCharacterData(view)` maps equipment/status field-for-field. `verifyCallerOwnsCharacter` still checks authenticated `PlayerID` on repository data before calling `ProjectView`; missing/foreign responses remain byte-identical `NOT_FOUND`. Equip/Unequip already passed that same handler gate before their orchestrator calls.

- [ ] **Step 6: Run focused gates and commit**

```bash
cd "$API_WORKTREE"
go test ./internal/orchestrators/character ./internal/handlers/dnd5e/v2/character -count=1
golangci-lint run ./internal/orchestrators/character/... ./internal/handlers/dnd5e/v2/character/...
git add internal/orchestrators/character internal/handlers/dnd5e/v2/character go.mod go.sum
git commit -m "feat(character): serve strict owner-private status (#${API_ISSUE})"
```

Expected: malformed fixtures prove zero Update calls; all ownership tests remain PASS.

---

### Task 9: Translate nested session declarations and selectors in rpg-api

**Files:**
- Modify: `internal/handlers/dnd5e/session/v1alpha1/{afford,attack,move,end_turn,convert,errors}.go`
- Modify: paired tests, `ownership_test.go`, `errors_test.go`
- Modify: `go.mod`, `go.sum` final toolkit pins before PR review
- Modify: `docs/architecture/components/character-handler.md`
- Modify: `docs/architecture/components/character-orchestrator.md`
- Modify: `docs/architecture/components/character-v2-handler.md`
- Modify: `docs/status.md`

**Interfaces:**
- Consumes: merged proto Task 2 and toolkit Tasks 5–7.
- Produces: pure wire/SDK translation with unchanged authorization.

- [ ] **Step 1: Write failing handler/converter tests**

Test exact mapping of:

```text
Declaration.id/available/attack/target_kind/candidates/why
AttackRequest.declaration_id
MoveRequest.declaration_id
EndTurnRequest.declaration_id
ErrNoDeclarationID → INVALID_ARGUMENT
ErrStaleDeclaration → FAILED_PRECONDITION
full-ref AttackRef in response and typed event conversion
```

Run `cd "$API_WORKTREE" && go test ./internal/handlers/dnd5e/session/v1alpha1 -count=1`.

Expected: FAIL against old generated/SDK fields.

- [ ] **Step 2: Implement field-for-field converters**

Add total converters:

```go
func targetKindToProto(sdk.TargetKind) sessionpb.TargetKind
func targetCandidateToProto(sdk.TargetCandidate) *sessionpb.TargetCandidate
func declarationToProto(sdk.Declaration) *sessionpb.Declaration
```

Unknown SDK enum values map to proto `UNSPECIFIED`; tests identify that as producer defect. Do not derive target availability or copy target rows into prose.

- [ ] **Step 3: Echo selectors through handlers**

Pass `req.GetDeclarationId()` into the three SDK inputs. Preserve `callerActingAs` before every manager call. Add new sentinel rows to the single error translation table and update its count pin.

- [ ] **Step 4: Update API architecture/status docs**

Document strict pre-write character application, flattened CharacterData translation, nested session declaration mapping, selector errors, and final provider pins in the three component docs and `docs/status.md`.

- [ ] **Step 5: Run API gates and replace RCs with final tags**

After API tests pass against the RCs, Kirk merges `$TOOLKIT_PR_URL`. Derive and pin the merge-generated final tags:

```bash
export TOOLKIT_MERGE_SHA=$(gh pr view "$TOOLKIT_PR_URL" --json mergeCommit --jq .mergeCommit.oid)
git -C "$TOOLKIT_WORKTREE" fetch origin --tags
export FINAL_DND5E_VERSION=$(git -C "$TOOLKIT_WORKTREE" tag --points-at "$TOOLKIT_MERGE_SHA" | grep '^rulebooks/dnd5e/v' | grep -v '/session/' | sed 's#rulebooks/dnd5e/##' | sort -V | tail -1)
export FINAL_SESSION_VERSION=$(git -C "$TOOLKIT_WORKTREE" tag --points-at "$TOOLKIT_MERGE_SHA" | grep '^rulebooks/dnd5e/session/v' | sed 's#rulebooks/dnd5e/session/##' | sort -V | tail -1)
test -n "$FINAL_DND5E_VERSION" && test -n "$FINAL_SESSION_VERSION"
cd "$API_WORKTREE"
GOPROXY=direct go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@${FINAL_DND5E_VERSION}
GOPROXY=direct go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/session@${FINAL_SESSION_VERSION}
go mod tidy
make pre-commit
make ci-check
git diff --check
scripts/verify-release-pin.sh
if rg -n '^replace ' --glob go.mod .; then exit 1; fi
```

Expected: all gates PASS and no override residue.

- [ ] **Step 6: Commit and open one API PR**

```bash
cd "$API_WORKTREE"
git add internal/handlers/dnd5e/session/v1alpha1 internal/handlers/dnd5e/v2/character \
        internal/orchestrators/character docs/architecture/components/character-handler.md \
        docs/architecture/components/character-orchestrator.md \
        docs/architecture/components/character-v2-handler.md docs/status.md go.mod go.sum
git commit -m "feat(session): serve production combat experience (#${API_ISSUE})"
git push -u origin "feat/${API_ISSUE}-session-combat-experience"
export API_PR_URL=$(gh pr create --base dev --title 'feat(session): serve production combat experience' --body-file /tmp/session-combat-api-pr.md)
```

PR body names final toolkit/proto artifacts and includes focused/full gate evidence.

---

### Task 10: Update web hooks and pure server-offer selection

**Files:**
- Modify: `package.json`, `package-lock.json`
- Modify: `src/api/useSessionAfford.ts` / test
- Modify: `src/api/useSessionAttack.ts` / test
- Modify: `src/api/useSessionEndTurn.ts` / test
- Modify: `src/components/session/useSessionWalk.ts` / test
- Create: `src/components/session/combat-experience/selection.ts`
- Create: `src/components/session/combat-experience/selection.test.ts`
- Create: `src/components/session/combat-experience/types.ts`

**Interfaces:**
- Consumes: generated declaration/CharacterData types.
- Produces: thin selector-bearing RPC hooks and pure `selectCombatExperience`/`selectDirectMapAttack` used by Task 11.

- [ ] **Step 1: Pin the merged proto tag and regenerate lock state**

```bash
cd "$WEB_WORKTREE"
npm i --save github:KirkDiggler/rpg-api-protos#${PROTO_TAG}
grep -n 'rpg-api-protos' package.json package-lock.json
```

Expected: package and lock resolve the same merged tag/commit.

- [ ] **Step 2: Write failing hook/selection tests**

Tests prove:

```ts
expect(attackRequest).toMatchObject({ declarationId: 'v1.selector' });
expect(endTurnRequest).toMatchObject({ declarationId: 'v1.end' });
expect(turnMoveRequest).toMatchObject({ declarationId: 'v1.move' });
expect(worldMoveRequest.declarationId).toBe('');
```

Pure selection tests cover unavailable declaration vs unavailable candidate, exact `why.text`, fixed target kinds, stale generic copy, and direct-map matching: zero → no dispatch; one → exact declaration ID; two → require panel choice.

Run:

```bash
cd "$WEB_WORKTREE"
npm run test:run -- src/api/useSessionAfford.test.ts src/api/useSessionAttack.test.ts \
  src/api/useSessionEndTurn.test.ts src/components/session/useSessionWalk.test.ts \
  src/components/session/combat-experience/selection.test.ts
```

Expected: FAIL on missing fields/modules.

- [ ] **Step 3: Implement thin hooks and pure selection**

Do not normalize declarations into rule-bearing booleans. `types.ts` contains presentation state only; generated `Declaration`, `Participant`, and `CharacterData` remain the data contract.

`selectDirectMapAttack(declarations, subject)` returns a declaration only when exactly one available Attack declaration contains an available candidate with that member.

- [ ] **Step 4: Run focused tests and commit**

```bash
cd "$WEB_WORKTREE"
npm run test:run -- src/api src/components/session/combat-experience/selection.test.ts
npm run typecheck
git add package.json package-lock.json src/api src/components/session/useSessionWalk* \
        src/components/session/combat-experience/selection* \
        src/components/session/combat-experience/types.ts
git commit -m "feat(session): consume server combat declarations (#${WEB_ISSUE})"
```

Expected: focused tests/typecheck PASS.

---

### Task 11: Promote the concept components into a shared production shell

**Files:**
- Create/move under `src/components/session/combat-experience/`: `CombatExperience.tsx`, `ActionDock.tsx`, `DiceDrawer.tsx`, `StoryLog.tsx`, `TargetSurface.tsx`, `CombatExperience.module.css`, component tests
- Modify: `src/concepts/session-combat/{SessionCombatConcept,fixtures,sessionCombatTypes,sessionCombatSelection,ActionDock,DiceDrawer,StoryLog,TargetSurface,SessionCombatConcept.module.css}*`
- Keep: `ContractInspector.tsx`, concept fixture/evidence controls
- Modify: `docs/how-to/concepts-route.md`

**Interfaces:**
- Consumes: Task 10 generated declarations/pure selection plus existing `SessionCanvas` and `DiceTrayPresentation`.
- Produces: one shared `CombatExperience` render component instantiated by concept fixtures and production route.

- [ ] **Step 1: Write failing shared-component tests**

Move/extend current concept tests to assert:

```text
fresh/spent/spectating/world/reconnect states render from the shared component
only Attack/Move/End Turn are executable
no Spells group or Healing Potion fixture exists
features/conditions/resources are informational
panel-first selection exposes only provider candidates
1024px structural regions remain present
```

Run `cd "$WEB_WORKTREE" && npm run test:run -- src/components/session/combat-experience src/concepts/session-combat`.

Expected: FAIL until shared components exist.

- [ ] **Step 2: Move production-intent rendering without duplicating it**

`CombatExperience` receives generated provider values and callbacks. The concept builds valid generated-shape fixtures with `create()` and passes them to the same component. Delete concept copies after imports switch; keep contract inspector and state controls concept-only.

Remove `Core/Features/Spells/Items` fixture assumptions from the first-wave action dock. Render authored Attack name/ref, Move allowance, informational feature/condition badges, and the separate server-declared End Turn.

- [ ] **Step 3: Preserve accessibility and responsive behavior**

Retain native buttons, focus states, semantic unavailable text, reduced motion, horizontal action overflow, and 1024×768 floor. End Turn uses the server declaration; no active-ID comparison enables it independently.

- [ ] **Step 4: Run focused tests and commit**

```bash
cd "$WEB_WORKTREE"
npm run test:run -- src/components/session/combat-experience src/concepts/session-combat
npm run typecheck
npm run format:check
git add src/components/session/combat-experience src/concepts/session-combat docs/how-to/concepts-route.md
git commit -m "refactor(session): share the approved combat experience (#${WEB_ISSUE})"
```

Expected: concept deep link still renders all review states through shared components.

---

### Task 12: Add private CharacterData caching and terminal stream recovery

**Files:**
- Create: `src/api/useCharacterData.ts`
- Create: `src/api/useCharacterData.test.ts`
- Modify: `src/api/useGetCharacterData.ts`
- Modify: `src/components/session/useSessionEventStream.ts`
- Modify: `src/components/session/useSessionEventStream.test.ts`
- Create: `src/components/session/combat-experience/characterPresentation.ts`
- Create: `src/components/session/combat-experience/characterPresentation.test.ts`

**Interfaces:**
- Consumes: flattened owner-gated CharacterData and GetStory.
- Produces: last-confirmed private data, coalesced `refetch`, and event delivery metadata `{source: 'live' | 'catchup'}` used by Tasks 13–14.

- [ ] **Step 1: Write failing private-data tests**

Prove initial fetch, Equip/Unequip replacement, coalesced event bursts, last-good retention on error, owner `NOT_FOUND`, and generic ref-based icon fallback. Verify no test subtracts damage or decrements resources.

- [ ] **Step 2: Write failing terminal-loss stream tests with fake timers**

Add:

```ts
it('recovers the last dropped event without a later stream event', async () => {
  const harness = createSessionStreamHarness({ live: [event(7n)], story: [event(8n)] });
  const delivered = vi.fn();
  renderHook(() => useSessionEventStream('session-1', 'fighter-1', delivered));
  await harness.openAndFlush();
  await vi.advanceTimersByTimeAsync(STORY_RECOVERY_INTERVAL_MS);
  expect(delivered).toHaveBeenCalledWith(event(8n), { source: 'catchup' });
  expect(delivered.mock.calls.filter(([value]) => value.seq === 8n)).toHaveLength(1);
});
```

Add tests that dispatch `visibilitychange` after changing the harness to visible and require an immediate catch-up; and force `ErrStoryTrimmed`, require a from-zero retry, and require every recovered entry to carry `{source:'catchup'}`. The harness owns fake timers and restores document property descriptors in cleanup.

Run focused tests; expect FAIL because polling/provenance is absent.

- [ ] **Step 3: Implement bounded polling inside the existing stream sequencer**

Export `STORY_RECOVERY_INTERVAL_MS = 5000`. Serialize catch-up calls so interval/focus/gap/reconnect cannot overlap. Feed recovered/live events through the same sequence deduper, now passing source metadata. Add/remove `visibilitychange` and `focus` listeners with effect cleanup; clear interval on session/member change.

- [ ] **Step 4: Implement cached owner-private data**

`useCharacterData(characterId)` fetches once, keeps last confirmed state on error, exposes `replace(CharacterData)` for equip responses, and exposes a coalesced `refetch`. Character presentation maps refs to icons/tone only; it renders server names/details/counts unchanged.

- [ ] **Step 5: Run focused tests and commit**

```bash
cd "$WEB_WORKTREE"
npm run test:run -- src/api/useCharacterData.test.ts \
  src/components/session/useSessionEventStream.test.ts \
  src/components/session/combat-experience/characterPresentation.test.ts
npm run typecheck
git add src/api/useCharacterData* src/api/useGetCharacterData.ts \
        src/components/session/useSessionEventStream* \
        src/components/session/combat-experience/characterPresentation*
git commit -m "feat(session): recover private combat state (#${WEB_ISSUE})"
```

Expected: terminal event recovery occurs within five seconds without a subsequent stream event.

---

### Task 13: Gate actor Story and dice presentation without delaying game truth

**Files:**
- Create: `src/components/session/combat-experience/presentation.ts`
- Create: `src/components/session/combat-experience/presentation.test.ts`
- Create: `src/components/session/combat-experience/useCombatPresentation.ts`
- Create: `src/components/session/combat-experience/useCombatPresentation.test.tsx`
- Create: `src/components/session/combat-experience/story.ts`
- Create: `src/components/session/combat-experience/story.test.ts`
- Modify shared `DiceDrawer.tsx`, `StoryLog.tsx`

**Interfaces:**
- Consumes: AttackResponse, typed Struck/Missed events with source metadata, existing dice presentation events.
- Produces: one `(session, seq)` presentation, release-gated actor Story/verdict/live announcement, auto-settled witnesses/history, developer-only raw Debug exception.

- [ ] **Step 1: Write failing response/event ordering tests**

Cover both orders:

```ts
it('response first arms once and hides Story until release', () => {
  const armed = reduceCombatPresentation(emptyPresentation(), attackResponseFact());
  const reconciled = reduceCombatPresentation(armed, matchingStreamFact('live'));
  expect(reconciled.diceEvents.filter((event) => event.type === 'dice-presentation-requested')).toHaveLength(1);
  expect(selectVisibleStory(reconciled)).toEqual([]);
  const released = reduceCombatPresentation(reconciled, localReleaseFact());
  expect(selectVisibleStory(released)).toHaveLength(1);
});
```

Add mirrored event-first coverage, duplicate response/event delivery, catch-up immediate settlement, current locally armed response surviving a later catch-up copy, witness neutral auto-release, and a rendered closed Debug control with no raw-event live-region text. Use one shared fixture builder for response and stream facts so mismatched authority is a deliberate test case, not fixture drift.

- [ ] **Step 2: Implement one presentation identity and reducer**

Use validated `presentationId = session:<sessionId>:<seq>` when it satisfies the existing identifier contract. Refuse/fallback semantically if an unexpected session ID would exceed that boundary; never use arbitrary asset URLs. Request uses Original carved d20 safe default and authoritative result; actor release appends only presentation data.

- [ ] **Step 3: Build Story from typed facts**

`story.ts` groups one first-wave Struck/Missed per `seq`, uses `AttackRef` full ref/name/damage type, and never uses authored fixture prose or client HP subtraction. For the acting player, omit/buffer the outcome until release; spectators and recovered history render settled immediately.

- [ ] **Step 4: Gate Debug to development diagnostics**

Shared Story always exists. The raw Debug tab/control renders only under `import.meta.env.DEV` or the existing explicit diagnostic concept surface. Raw events ingest immediately; closed Debug has no active live region.

- [ ] **Step 5: Run focused tests and commit**

```bash
cd "$WEB_WORKTREE"
npm run test:run -- src/components/session/combat-experience/presentation.test.ts \
  src/components/session/combat-experience/useCombatPresentation.test.tsx \
  src/components/session/combat-experience/story.test.ts
npm run typecheck
git add src/components/session/combat-experience
git commit -m "feat(session): present authoritative combat outcomes (#${WEB_ISSUE})"
```

Expected: no actor-facing result before explicit release; game events remain ingested immediately.

---

### Task 14: Replace the old production CombatPanel and verify the journey

**Files:**
- Modify: `src/components/session/SessionEncounterView.tsx` / test
- Modify: `src/components/session/SessionCanvas.tsx` / test
- Modify: `src/components/session/useSessionWalk.ts` / test
- Delete after migration: `src/components/session/{CombatPanel,useCombatPanel,combatPanel}.*` when no remaining import exists
- Preserve/refactor pacing helpers only if shared Story still consumes them
- Modify: `docs/architecture/components/combat-v2.md`
- Modify: `docs/status.md`
- Add visual evidence under the owning web issue's established `docs/evidence/` convention

**Interfaces:**
- Consumes: Tasks 10–13 shared experience/controller/recovery.
- Produces: production session route with panel-first Attack, exact selector dispatch, owner-private dock, Story/dice gating, and world-clock fallback.

- [ ] **Step 1: Write failing production integration tests**

`SessionEncounterView.test.tsx` must prove:

```text
old direct-floor Attack does not dispatch when no action is armed
selecting Attack then an available candidate sends exact declaration_id + target
unavailable candidate renders why.text and never dispatches
two matching offers make direct map shortcut refuse ambiguity
Move sends selector only on turn clock
End Turn sends its own server selector
CharacterData replaces old v1 Character HP/level source
stream/catchup batches coalesce CharacterData/Turn/Afford refresh
world clock has no combat declaration row
```

Run `cd "$WEB_WORKTREE" && npm run test:run -- src/components/session/SessionEncounterView.test.tsx`.

Expected: FAIL against old `CombatPanel`/direct-floor behavior.

- [ ] **Step 2: Integrate shared CombatExperience**

Replace old `useCombatPanel` composition with the shared controller. Keep SessionCanvas map/door/movement behavior unchanged except target rings/clicks now come from the armed server declaration. Remove the old v1 `GetCharacter` fetch once roster + CharacterData supply every used fact.

Update `useSessionWalk` to receive the current Move declaration ID; a state transition to world clock clears it before a free-roam request.

- [ ] **Step 3: Preserve other-member pacing and run-ending behavior**

Route typed events through one funnel: sequence delivery → private/query invalidation → presentation/story pacing → debug buffer → door/run-ending handlers. Do not regress monster beat pacing, door state refresh, run-ended overlay, roster pull-on-join, or self-MOVED GetWhere behavior.

- [ ] **Step 4: Delete superseded panel code and update architecture docs**

Run:

```bash
cd "$WEB_WORKTREE"
rg -n 'CombatPanel|useCombatPanel|combatPanel' src
```

Delete only files with no remaining production/concept import. Update `combat-v2.md` and `docs/status.md` to name nested declarations, panel-first targeting, shared concept/production components, private CharacterData, recovery poll, developer Debug, and the retired old panel/direct-floor flow.

- [ ] **Step 5: Run focused and full web gates**

```bash
cd "$WEB_WORKTREE"
npm run test:run -- src/api src/components/session src/components/session/combat-experience src/concepts/session-combat
npm run ci-check
git diff --check
```

Expected: all tests, format, lint, typecheck, build, and full suite PASS.

- [ ] **Step 6: Commit and open the web PR**

```bash
cd "$WEB_WORKTREE"
git add src docs package.json package-lock.json
git commit -m "feat(session): promote production combat experience (#${WEB_ISSUE})"
git push -u origin "feat/${WEB_ISSUE}-session-combat-experience"
export WEB_PR_URL=$(gh pr create --base dev --title 'feat(session): promote production combat experience' --body-file /tmp/session-combat-web-pr.md)
```

PR body links design #270, concept #809/#810, provider PRs/tags, and exact local gate output.

- [ ] **Step 7: Run the two-browser live journey gate before merge**

Build the API from its feature branch and serve the web feature branch. With two authenticated characters in the reference tomb, record:

```text
1. both see real roster/map/initiative and only their own exact CharacterData;
2. active player selects full-ref Longsword Attack;
3. out-of-range live target is visible but unavailable with provider text;
4. available target dispatches exact selector and target;
5. actor explicitly releases authoritative d20; witness auto-settles;
6. Story stays concealed for actor until release; developer Debug is immediate;
7. target HP changes only after owner-private refetch;
8. spent offers/shapes update; selector-bearing Move/EndTurn work;
9. dropping the terminal stream event recovers via GetStory within five seconds;
10. reconnect settles history, and world clock restores translucent exploration UI.
```

Capture 1280×800 and 1024×768 screenshots with `node tools/browser/screenshot.mjs`; attach evidence to the web issue without committing licensed GLBs/private evidence.

---

### Task 15: Merge inside-out and close the tracking surfaces

**Files:**
- Modify: `ideas/session-combat/experience/design.md` only if integration forced a contract amendment.
- Modify: `ideas/session-combat/experience/plan.md` checkboxes/status as execution evidence.
- Update ignored local `rpg-project/active.md` for operator continuity.

**Interfaces:**
- Consumes: green toolkit/API/web PRs and Kirk's exact live walk.
- Produces: merged production wave, closed implementation slices, Done board items, then merged design PR #271.

- [ ] **Step 1: Confirm provider/consumer PR heads and review findings**

For each PR, record head SHA, checks, inline review comments, and unresolved conversations. Fix every applicable Critical/Important finding on the same wave branch and rerun its full gate.

- [ ] **Step 2: Reconfirm the already-merged toolkit provider**

Task 9 merged toolkit to replace RCs before the API PR became review-ready. Reconfirm both final module tags still point at `$TOOLKIT_MERGE_SHA` and that the API PR pins those exact versions; no RC remains in `go.mod`.

- [ ] **Step 3: Kirk merges rpg-api**

Verify the API PR targets `dev`, contains final proto/toolkit pins, and local Docker stack is rebuilt from merged `dev`. Re-run the web against merged API.

- [ ] **Step 4: Kirk walks and merges web**

Kirk repeats the named two-browser path on the exact web PR head, then alone merges to `dev`.

- [ ] **Step 5: Close issues and Project 19 items**

Close the four implementation issues as completed, set their Project 19 status to Done, and record final PR/tag/commit evidence on design issue #270.

- [ ] **Step 6: Commit final plan evidence and merge design PR last**

```bash
git add ideas/session-combat/experience/design.md ideas/session-combat/experience/plan.md
git commit -m "docs(session-combat): record production experience delivery (#270)"
git push
```

Kirk merges rpg-project PR #271 only after the implementation is complete. Then set #270 Done and remove worktrees/branches only when directed.
