# Existing Roll-Window Correlation Implementation Plan

**Execution status: delivered.** Retained below as the implementation plan, not
a request to repeat the work. See [implementation.md](implementation.md) for
actual merged revisions, verified evidence and explicitly unverified limits.

> **For agentic workers:** Use `superpowers:executing-plans` for direct execution,
> one checkpoint at a time. Kirk selected parent-driven implementation.
> Independent read-only review remains required for substantive PRs.

**Goal:** Let every permitted recipient join the existing early roll-window
facts to the shared d20, while only its owner receives actionable choices.

**Architecture:** Pass the existing presentation ID through the already-recorded
RollWindowOpened. Reuse the current session event spine, dice transport,
presentation reducer and physical renderer. Resolution and its roll data stay
unchanged.

**Tech stack:** Go toolkit modules/API, Buf Go/TypeScript contract, React/Vitest.
**Spec:** [design.md](design.md), revised and approved after the data trace.

## Constraints

- No new roll snapshot/reporter, AttackRolled event, result store or transport.
- Preserve current event audience policy and backend Afford/React authorization.
- No timer extension, physics-derived face, inferred identity, or invented
  target/attack/final-outcome facts on the rolled stage.
- Provider presentation ID is shared; Story sequences are recipient-local and
  phase-specific. Reuse the existing physical settled signal.
- Keep ordinary attack, historical/catch-up, explicit fallback, next-attack,
  downed, concentration and run-ended behavior covered.
- Retire the unmerged duplicate proposal #1597; preserve its branch/worktree as
  evidence rather than folding that code into the correction.
- One writer per isolated worktree, every issue on Project 19, all delivery by
  PR. Do not reset unrelated checkouts or replace the running API.
- Module publication is sequential: provider PR -> owner merge -> actual CI tag
  -> consumer pin. No side-branch tags or committed local overrides.

## Checkpoint 0 — retire the superseded proposal

- [ ] Close toolkit PR #1597 and issue #1596 as superseded by this revised
  contract, linking #411 and the trace. Do not delete their branch/worktree.
- [ ] Publish this revised plan/design on #411 and update #410's summary so
  neither the old snapshot proposal nor a privacy change looks current.

## Checkpoint 1 — Encounter preserves the ID it is handed

**Files:** toolkit `rulebooks/dnd5e/encounter/roll_window.go` and
`roll_window_test.go`. New owning issue/branch from fresh `origin/main`; do not
carry the abandoned Resolution changes.

**Existing types extended, no new representation:**

```go
// RollWindowInput addition:
PresentationID string
// rollWindowPayload addition:
PresentationID string `json:"presentation_id,omitempty"`
```

- [ ] Add a failing RollWindowTestSuite case: pass an opaque ID to
  RecordRollWindow and read both Alice's and the goblin's persisted Story.
  Both window payloads must preserve the identical ID, roll and total.
- [ ] Keep a legacy case without the ID; it must remain valid and readable.
- [ ] Implement only field pass-through into the existing payload. Leave
  audienceFor, validation of existing fields, and pause semantics unchanged.
- [ ] Run module tests/race/lint/format/tidy, commit, independently review and
  publish the PR. Wait for owner merge and verify the actual module tag before
  making Session depend on the new input field.

## Checkpoint 2 — additive proto field

**File:** `rpg-api-protos/dnd5e/api/session/v1alpha1/events.proto`.

```proto
// Add to the EXISTING RollWindowOpened; tag 5 was taken by proto #318:
// Same provider token used by AttackResponse, the throw plan and final outcome.
// Absent on legacy recorded windows; never synthesize it from Event.seq.
string presentation_id = 6;
```

- [ ] Create/adopt the owning issue on Project 19 and isolate from fresh main.
- [ ] Add the field/comment only; no event enum, new message or DiceThrowPlan
  field. Preserve existing field numbers.
- [ ] Run repo Buf format/lint/breaking and generation/compile gates. Do not
  hand-edit generated SDKs or add tests of protobuf mechanics.
- [ ] Publish through PR; record CI's generated version after owner merge.

## Checkpoint 3 — Session forwards its already-minted token

**Files:** toolkit `rulebooks/dnd5e/session/attack.go`, `events.go`, `types.go`,
`post_roll_test.go`, `translate_internal_test.go`, `go.mod`, `go.sum`.

```go
// Existing RollWindowOpenedBody addition:
PresentationID string `json:"presentation_id,omitempty"`
// Existing RecordRollWindow call in poseAttackWindow:
PresentationID: presentationID,
```

- [ ] Adopt the verified Encounter release, not a guessed version.
- [ ] Extend existing PostRollWindowSuite: actor and peer receive the same
  window ID; it equals AttackResponse.PresentationID. Restart before answering,
  then Keep/Spend yields a later outcome with that same ID and natural d20.
  No new roll event or outcome before React; peer Afford still cannot answer.
- [ ] Test legacy decoding without the field and unchanged delivery audiences.
- [ ] Copy the ID from poseAttackWindow into its existing recorded input,
  decode it in rollWindowOpenedBody, and expose it in the existing body type.
  Do not read Frozen, add Resolution.Output fields, or reconstruct roll data.
- [ ] Run module tests/race/lint/tidy and boundary checks. Review and publish
  through PR; dependent API waits for the actual Session version.

## Checkpoint 4 — API conversion

**Files:** `rpg-api/internal/handlers/dnd5e/session/v1alpha1/convert.go`,
`convert_test.go`, `internal/integration/session/bard_inspiration_acceptance_test.go`,
`go.mod`, `go.sum`.

```go
// Existing RollWindowOpened conversion gains exactly this pass-through:
PresentationId: b.PresentationID,
```

- [ ] Adopt real Session/proto versions in the isolated API issue branch.
- [ ] Red conversion assertion: a nonempty opaque ID survives the existing
  body mapping; an absent legacy ID stays absent, not inferred from seq.
- [ ] Extend wired Bard acceptance to prove matching early roll ID/face for
  both recipients, actor-only executable offer, and same-ID later outcome.
- [ ] Implement field conversion only. No rules or visual-service changes.
- [ ] Run API gates, review, publish PR. Do not restart shared services.

## Checkpoint 5 — reuse the rolled facts in the web dice path

**Files:** web `src/components/session/combat-experience/presentation.ts`,
`presentation.test.ts`, `presentation.test-fixtures.ts`,
`useSessionCombatExperience.ts`, `rollWindowFlow.test.tsx`, `types.ts`,
`src/components/session/SessionEncounterView.tsx`, `SessionEncounterView.test.tsx`,
`downedReveal.test.ts`; exact dependency pin/lockfile and truthful status docs.

- [ ] Reuse #996's branch and its bugfixes; adopt generated proto version.
- [ ] Add tests with real RollWindowOpened schema carrying presentationId.
  Before Struck/Missed, a spectator must get a request using window.audience
  as roller and window.roll as natural d20. No made-up target or verdict.
- [ ] Route its rolled facts through the existing presentation reducer and
  DicePresentationRequestedEvent. Keep Story identity separate from physical
  identity. Refine existing phase handling rather than add a parallel store.
- [ ] Match current owner window to the provider token directly. Keep legacy
  fallback only for ID-less old records. Actor controls still require its
  backend-authored reaction declaration; peers never receive executable UI.
- [ ] Exercise actor response/window arrival orders, plan/window arrival
  orders and a decision left open longer than 1500ms: the witness has already
  matched and played the roll. Keep existing plan/fingerprint/role checks and
  the inbox's network-ordering buffer unchanged.
- [ ] Verify later outcome at a different seq and changed final total does not
  re-arm, replay, or retain a target hold. Keep next-attack/downed regressions.
- [ ] Test duplicates, stale settlement, missing visual plan, off-table retry,
  explicit nonphysical fallback, catch-up and reconnect. Missing legacy ID
  cannot strand an answerable actor window or fabricate a past throw.
- [ ] Run focused/full web CI and independent review; publish current-head
  evidence to the existing draft PR. No ready claim before live proof.

## Checkpoint 6 — two-player proof

- [ ] Coordinate a compatible API with Kirk before any environment switch.
- [ ] On :3006 with fighter and peer, grant inspiration through normal play.
  Both see the fighter's same d20 before Spend/Keep is chosen. Leave the offer
  open several seconds; only the owner sees actionable options.
- [ ] Keep, then repeat with Spend. Final outcome follows the real choice;
  both see a downed monster, no second attack d20, and the next roll works.
- [ ] Reverse roles and repeat without inspiration; check reconnect/fallback.
- [ ] Record exact revisions and live evidence on PRs/#411. Owner merges via
  PR; CI tags providers. No automatic deployment or destructive cleanup.

## Self-review

The changed data is one existing identifier across existing boundaries. The
roll/offer source, audience policy, visual transport and rules are unchanged.
No new independent roll model or general dice migration is hidden in the tasks.
Provider publication and live API selection are explicit gates.
