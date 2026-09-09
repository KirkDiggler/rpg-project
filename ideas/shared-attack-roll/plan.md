# Shared Attack Roll Implementation Plan

> **For agentic workers:** Use `superpowers:executing-plans` for direct execution,
> task by task. Kirk selected parent-driven implementation, not implementation
> subagents. Independent read-only review remains required for substantive PRs.

**Goal:** Both players see the same authoritative d20 before the actor answers an
inspiration offer, without re-rolling or leaving a monster's reveal held.

**Architecture:** Toolkit captures a roll-stage value and persists AttackRolled
through the existing encounter/session spine. Protos/API carry it unchanged.
The web joins that authority to the existing visual throw stream by provider ID;
roll, private choice and final outcome are separate phases of one presentation.

**Tech stack:** Go multi-module toolkit, Buf Go/TypeScript protos, Go API,
React/TypeScript/Vitest, existing Rapier world-die renderer.

**Spec:** [design.md](design.md), approved by Kirk in conversation.

## Global constraints

- No client-computed game result, physics-derived face, new dice transport,
  animation manager, or timeout extension.
- Selected natural d20 only; no damage dice, two-face rendering, optimization
  pass, or persistent True Strike marker.
- One provider presentation ID survives every phase. Story sequences remain
  recipient-local and may differ across phases and clients.
- Early payload contains no AC, hit/miss, critical verdict, damage, offer,
  chosen option or inspiration die result.
- One writer in each isolated worktree. Preserve the main checkouts and :3006.
  Do not restart the running API without operator coordination.
- Every implementation issue goes on Project 19 under journey #289. All
  integration goes through PRs; no auto-merge or hand-created toolkit tags.
- Keep implementation checkpoints small. Each ends with commands/results,
  exact head, residual gaps, and a clean commit or explicitly saved partial diff.
- Build against local siblings only through uncommitted overrides. Committed
  consumers must build against actually published dependencies. Never fabricate
  version numbers or commit local `replace`/`go.work` files.

## Two discoveries to carry into execution

1. **Window privacy is currently UI-only.** Encounter `RecordRollWindow` and
   session's EventRollWindowOpened documentation explicitly broadcast to the
   full roster; the web filters the offer by its payload audience. The approved
   design's actor-private wire delivery is therefore a change, not an existing
   invariant. Implement it narrowly for roll offers through the central
   audience policy, without changing ordinary combat visibility or movement
   reaction windows. Update the old full-roster window test/documentation.
2. **Toolkit modules have independent release gates.** Existing concentration
   work landed encounter, resolution and session as separately tagged module
   changes. CI tests each module against its committed pins, not an automatic
   sibling workspace. Encounter/resolution providers can be developed without
   depending on each other's new APIs; Session cannot publish until their
   dependencies are real. Keep one development worktree/branch for the wave and
   stop at the provider publication checkpoint for owner coordination before
   dependent Session publication. Do not bypass the project branch policy,
   create side-branch tags, or pretend a local workspace is CI evidence.

## Checkpoint 1 — capture the original roll in Resolution

**Files (toolkit `rulebooks/dnd5e/resolution/`):**
- Create `attack_roll.go`, `attack_roll_test.go`.
- Modify `strike.go`, `resolve.go`; reuse `strike_pose_test.go` fixtures.

**Interface produced:**

```go
// AttackRoll is the selected natural d20 and its total before a post-roll offer.
// No final outcome or offer data belongs here.
type AttackRoll struct {
    AttackerID string
    TargetID   string
    Roll       int
    Total      int
}
// Add to Output; nil means this interaction did not make a new attack roll.
AttackRoll *AttackRoll
```

- [ ] Read module `doc.go`, `README.md`, `step.go`, current strike/pose/resolve
  code and the fixture helpers before editing. Record module/base/issue.
- [ ] Add a test using the real `heroSwings`, `actionHero`, `inspiredHero` and
  `actionRoller` fixtures. Normal 15+4 returns Roll=15/Total=19; posed 8+4
  returns Roll=8/Total=12 and Outcome=nil. Resume Spend with d6=4 yields final
  Total=16 but **AttackRoll=nil**. Resume Keep also reports no new roll.

```go
out, err := heroSwings(t, inspiredHero(t), &actionRoller{singles: []int{8}})
require.NoError(t, err)
require.Equal(t, &AttackRoll{
    AttackerID: heroID, TargetID: wolfID, Roll: 8, Total: 12,
}, out.AttackRoll)
require.Nil(t, out.Outcome)
```

- [ ] Run `go test ./... -run 'TestAttackRoll' -count=1`; observe failure.
- [ ] Capture the value exactly once in `strikeMachine.afterAttackChain` after
  successful d20 selection and bonus folding, before gathering offers. Store a
  private snapshot on the machine. An internal data-report interface lets
  `resolveOn` copy that snapshot onto Output after successful execution; do not
  add a new Step kind, bus event, exported callback or reverse calculation from
  final Total. The resumed path does not execute this capture point.

```go
type attackRollReporter interface { attackRoll() *AttackRoll }
// The helper returns a value copy, never the machine-owned mutable pointer.
func reportedAttackRoll(machine Machine) *AttackRoll
```

- [ ] Cover canceled/refused attacks (no invented roll), missing dice/error
  propagation, and the existing answers-are-data boundary suite. Non-strike
  machines continue to return nil. This checkpoint reports the top-level player
  weapon strike only; it does not introduce nested-roll collection.
- [ ] Run `go test ./...`, `go test -race ./...`, module `golangci-lint run ./...`,
  `gofmt`, and module-only `go mod tidy`; inspect resulting diffs. Commit this
  provider checkpoint without touching dependent modules' pins.

## Checkpoint 2 — record the public roll and correlate the private window

**Files (toolkit `rulebooks/dnd5e/encounter/`):**
- Create `attack_roll.go`, `attack_roll_test.go`.
- Modify `roll_window.go`, `roll_window_test.go`, `encounter.go` audience shelf.

**Interfaces produced:**

```go
const BeatAttackRolled = "attack_rolled"
type AttackRollInput struct {
    Attacker MemberID
    Target MemberID
    Attack AttackIdentity
    PresentationID string
    Roll int
    Total int
}
type AttackRollOutput struct { Seq uint64 }
func (e *Encounter) RecordAttackRoll(in *AttackRollInput) (*AttackRollOutput, error)
// Add PresentationID string to RollWindowInput and its persisted payload.
```

- [ ] Use the existing RollWindowTestSuite scene to write failing tests for
  one typed rolled payload, full permitted attack audience, no outcome fields,
  deterministic round-trip, and distinct roll/window sequences.
- [ ] Test nil input, closed encounter, missing/unknown participants, missing
  presentation/attack identity, and roll outside 1..20. Fail without appending.
- [ ] Implement a typed JSON payload and `appendBeat` call, using
  `audienceFor(subjectBeat, in.Attacker, in.Target)` and the existing clock.
  Do not route through `OutcomeStruck`/`OutcomeMissed` or mutate the clock.
- [ ] Add a narrowly named private-offer beat class to the existing audience
  helper and use it only for RollWindowOpened. Test that the actor reads the
  offer while another member reads AttackRolled but no offer payload. Movement
  WindowOpened and other audience classes remain unchanged.
- [ ] Preserve decoding of historical roll windows without a presentation ID;
  new Session callers always supply one. Update the old everybody-reads-the-
  offer test to test the intentional privacy boundary.
- [ ] Run module `go test ./...`, `go test -race ./...`, lint/format/tidy, then
  commit and independently review the provider-only changes. Publish through
  PR and wait for owner merge/CI-issued module tags before consumer pins.

## Checkpoint 3 — publish the Session transaction and typed projection

**Files (toolkit `rulebooks/dnd5e/session/`):**
- Modify `attack.go`, `types.go`, `events.go`, `post_roll_test.go`,
  `presentation_id_test.go`, `translate_internal_test.go`, `go.mod`, `go.sum`.
- Reuse persistence/failure tests in the module; add an `attack_roll_test.go`
  suite only if extending the existing suite would mix unrelated fixtures.

**Interface produced:**

```go
const EventAttackRolled EventKind = "attack_rolled"
type AttackRolledBody struct {
    PresentationID string `json:"presentation_id"`
    Attacker string `json:"attacker"`
    Target string `json:"target"`
    Attack AttackRef `json:"attack"`
    Roll int `json:"roll"`
    Total int `json:"total"`
}
func (AttackRolledBody) isEventBody() {}
// Add PresentationID string to RollWindowOpenedBody.
```

- [ ] Gate: provider APIs and their actual published versions must exist. Record
  the versions adopted, run module-only `go get`/tidy, and confirm no local
  replacement remains in the committed dependency graph.
- [ ] Extend PostRollWindowSuite: actor sees AttackRolled then private offer;
  peer sees the same ID/roll but no offer. Restart the manager, Keep or Spend,
  and assert a later outcome with that same ID and d20. Count exactly one
  AttackRolled across the pair; no final beat before React.
- [ ] Add normal-attack and persistence-before-publication checks. Recipient
  sequences are independent; do not assert equality across viewers or phases.
- [ ] Copy `out.AttackRoll` into `RecordAttackRoll` on the initial successful
  Attack path before choosing paused/completed recording. Attach the same
  minted presentation ID to the window. `answerPostRoll` records only the final
  outcome. Do not decode Frozen or rebuild a bonus in Session.
- [ ] Map the new beat to EventKind and typed body using existing payload
  validation/unknown-event behavior. Add historical window-ID absence tests.
- [ ] Run session tests/race/lint/tidy and the public-boundary tests. Commit,
  review and publish via the actual module-release gate; no invented tag.

## Checkpoint 4 — additive proto contract

**File:** `rpg-api-protos/dnd5e/api/session/v1alpha1/events.proto`.

At inspected main the free enum number is 30 and free Event body field is 36;
verify they remain free before editing, rather than renumber another change.

```proto
// EventKind addition:
EVENT_KIND_ATTACK_ROLLED = 30;
// Event.body addition:
AttackRolled attack_rolled = 36;

message AttackRolled {
  string presentation_id = 1;
  string attacker = 2;
  string target = 3;
  AttackRef attack = 4;
  int32 roll = 5;
  int32 total = 6;
}
// RollWindowOpened addition:
string presentation_id = 5;
```

- [ ] Add comments defining pre-choice total, immutable natural d20, missing
  legacy fields, and absence of outcome/offer data. Reuse AttackRef; no new
  DiceThrowDraft/Plan field or transport service.
- [ ] Run the repo's Buf format/lint/breaking checks and generation/compile
  gates. Do not add tests of protobuf's serialization mechanics or commit
  generated SDK output by hand.
- [ ] Open the owning PR. On owner merge, record CI's generated commit/version
  for the consumers; do not push `generated` yourself.

## Checkpoint 5 — API translation and wired acceptance

**Files:** `rpg-api/internal/handlers/dnd5e/session/v1alpha1/convert.go`,
`convert_test.go`, `internal/integration/session/bard_inspiration_acceptance_test.go`,
`go.mod`, `go.sum`.

- [ ] Adopt the real Session and generated-proto releases in the isolated API
  branch. Existing `Attack`, `React`, `GetStory` and stream handlers remain thin.
- [ ] Add failing conversion cases with literal ID/actor/target/roll/total and
  the new enum/oneof; private window presentation ID must survive conversion.
- [ ] Implement one EventKind mapping and one typed-body mapping. No lookup of
  attack outcomes in the visual-plan service, physics interpretation, or rule
  imports in handlers.
- [ ] Extend the real-handler Bard acceptance path to assert two-recipient
  early roll delivery, actor-only offer, later Spend/Keep outcome, and replay
  parity. Test no AC/outcome fields on the early payload.
- [ ] Run owning API unit/integration/lint gates, inspect no local overrides,
  and publish an independently reviewed PR. Do not restart the shared API.

## Checkpoint 6 — one web roll lifecycle, joined to the existing witness stream

**Files:** web `src/components/session/combat-experience/presentation.ts`,
`presentation.test.ts`, `presentation.test-fixtures.ts`,
`useSessionCombatExperience.ts`, `rollWindowFlow.test.tsx`, `story.ts`,
`story.test.ts`, `types.ts`, `src/components/session/SessionEncounterView.tsx`,
`SessionEncounterView.test.tsx`, and `downedReveal.test.ts`.

- [ ] Reuse #996's branch/worktree and preserve its playtest fixes. Adopt the
  actual generated contract in `package.json`/lockfile when published.
- [ ] Add roll-only fixture authority using the real generated AttackRolled
  schema. It has no final hit/miss/damage fields. A request and stream fact
  share provider ID while the final outcome deliberately has a later seq and,
  for Spend, a changed total.
- [ ] Red tests: a spectator gets an authoritative face/request on AttackRolled
  before any Struck/Missed; a plan matches and plays while the actor leaves
  inspiration unanswered beyond 1500ms. Test plan-before-roll and roll-before-
  plan network arrival within the existing transport buffer.
- [ ] Extend the existing reducer's typed roll authority and identity handling;
  keep Story keys separate. An actor response, early roll event and later
  outcome reconcile onto one physical presentation, not three queued rolls.
  Keep immutable ID/actor/target/natural-face conflicts fail-closed; an allowed
  later final total is not an immutable-roll conflict.
- [ ] Match new RollWindowOpened by its explicit presentation ID. Retain the
  documented legacy fallback only for old windows, not as a second current
  correlation system. Existing settled event gates the choice and Story.
- [ ] Feed the early request into the existing witness expectation/inbox and
  renderer. Preserve plan validation, collider fingerprint, role/ownership,
  and retry checks. Leave the 1500ms buffer constant unchanged.
- [ ] Test later outcome, duplicate/replay, next attack, downed reveal, and no
  re-arm in a real session-view integration test. Add actor response/event
  ordering, reconnect/catch-up, missing plan and explicit fallback tests.
- [ ] Keep old Struck/Missed-only history readable, without synthesizing an
  earlier roll. No replay of historical physical throws. Update scope/status
  docs only after behavior is tested.
- [ ] Run focused tests, full `npm run ci-check`, exact-head independent review,
  and publish the updated web PR and review verdict. Preserve draft status
  until integrated proof is accepted.

## Checkpoint 7 — two-player proof and closure

- [ ] Coordinate the compatible API selection with Kirk. Reuse a compatible
  already-running API if available; otherwise use an explicitly approved named
  test stack, never silently replace the shared one.
- [ ] On :3006, repeat Fighter-with-Inspiration attacks with two real clients.
  Both must see the same d20 before the choice. Wait several seconds before
  choosing; witnesses still saw the roll and have no offer controls.
- [ ] Verify Keep and Spend, changed final total only when provider says so,
  downed monster on both clients, no second d20, and the next attack roll.
- [ ] Reverse actor/witness and repeat without inspiration. Check reconnect
  during the offer, missing visual delivery, and explicit renderer fallback.
- [ ] Save screenshots/recorded observations with exact web/API/proto/toolkit
  revisions and commands. Do not call shell screenshots live-backend proof.
- [ ] Publish the current-head review/CI/live receipts on the implementation
  PRs and link them from #411/#410. Kirk merges; no automatic deployment or
  cleanup. Keep unresolved unrelated product gaps explicitly tracked.

## Spec coverage / self-review

Checkpoints 1–3 cover capture, persistence, audience and transaction order;
4–5 cover wire translation and real-handler proof; 6 covers one presentation,
legacy and replay; 7 proves the player promise. Missing-result inference and
buffer-extension approaches remain excluded. Module release and API switching
are explicit gates, not assumed successful commands.
