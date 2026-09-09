# Correlate the existing roll window with the shared d20

Status: revised design approved by Kirk in conversation after a read-only data
trace. Supersedes this PR's earlier AttackRolled/snapshot proposal.

Issue: #410. Parent: #289 (Roll Dice Together).
Related: rpg-dnd5e-web#964, #996 / PR #1003; implementation [plan](plan.md).

## Player promise

The roll is the roll whether inspiration is spent or kept. Both players see the
same d20. After it settles, the owning player can answer the backend's Spend/Keep
offer. The unmade choice, added inspiration result and final hit/miss are not
invented ahead of that answer. A later outcome does not request another d20 or
keep the monster's downed reveal held.

## What the trace established

Existing data already flows all the way out of the toolkit:

```text
Strike's original Roll / Total
  -> Pose.Ask (and Frozen for resumption)
  -> Session.poseAttackWindow
  -> Encounter.RecordRollWindow
  -> persisted RollWindowOpened
  -> typed Session body -> API -> all existing permitted recipients
```

For attacks without a post-roll offer, the existing Struck/Missed path supplies
the roll. For a paused attack, **RollWindowOpened already supplies the d20 and
pre-choice total before the answer**. Its payload `audience` names the player
who answers; it does not limit the event envelope to that player. The current
encounter audience policy broadcasts the window to the roster. Preserve that
policy; the UI already makes controls actor-only, backed by Afford/React.

The presentation ID also already exists at `poseAttackWindow`. It is stored in
the pending ledger payload, returned in AttackResponse, used by DiceThrowPlan,
and reused by the later Struck/Missed. **It is omitted only from the recorded
RollWindowOpened and its downstream projections.**

Web currently puts RollWindowOpened in Story, not the dice-authority path. Thus
the witness has the early number and the independently delivered physical plan,
but cannot join them reliably. It waits for Struck/Missed; an inspiration decision
can exceed the visual inbox's 1500ms receipt window.

Inspected baselines: toolkit `b3f899c5`, API `736084b8`, protos `5d9ef1a`, web
playtest branch `93636165`. Refresh before execution and trust current code.

## Smallest correction

### Toolkit and wire: pass through one existing identity

Add `PresentationID string` to the existing encounter RollWindowInput and its
recorded payload. Session passes the already-minted `presentationID` into it.
Preserve it in the existing Session RollWindowOpenedBody, then add
`string presentation_id = 5` to the existing proto RollWindowOpened and map it
unchanged in API conversion. Verify field 5 is still free before editing.

No new roll source, machine snapshot, optional reporter, outcome type, event
kind or stream. No dice calculation, timer extension or change to resolution.
Existing windows without an ID remain readable for backward compatibility.

### Web: one existing die presentation, owner-only controls

- Use RollWindowOpened's authoritative `roll`, owning member (`audience`) and
  `presentation_id` to feed the existing dice presentation and witness inbox.
  Its `total` remains pre-choice; it is not a hit/miss result.
- Match actor AttackResponse, roll window, shared throw and later outcome by
  the provider ID. Do not compare recipient-local sequences across players or
  across phases, or match coincidentally equal roll numbers.
- Reuse the existing DicePresentationRequestedEvent and world-die renderer.
  A window lacks target/attack/final-outcome fields: do not fabricate them or
  put a sentinel-filled result into Story. Reconcile phases in the existing
  presentation reducer rather than introducing a second result store.
- Every permitted recipient may witness the die. Only the owning player's
  backend-authored Afford row can render actionable Spend/Keep controls. UI
  visibility is not the authorization boundary; backend React remains so.
- Use the matching existing settled signal to reveal the actor's choice and
  pending-roll Story. Keep the final outcome separate until the server sends it.
- Attach that outcome to the already-presented d20, retain settlement and retire
  provisional reveal holds. Preserve the #996 next-attack/downed regressions.
- Keep existing role, plan validation, collider fingerprint and retry checks.
  The visual stream still carries motion, not a new client-authored result.

## What is explicitly removed from the earlier proposal

Do not merge toolkit PR #1597's `m.rolled`, `attackRollReporter` or
`Output.AttackRoll`. Those duplicate data already leaving Resolution through
Pose.Ask. No new AttackRolled event or actor-private wire policy is needed for
this bug. Tests proving an extra copy accurate did not justify its architecture.

Existing RollCalculation/DiceTrace types serve richer damage/healing traces;
attack d20s do not currently populate them. Do not fabricate a trace or widen
this correction into that separate migration.

## Compatibility and proof

- Preserve old windows without an ID and Struck/Missed-only history. Missing
  metadata must not cause inferred identity, a fabricated past roll, or a stuck
  answerable window. The new pre-choice witness behavior requires updated
  provider and client; legacy rendering remains the existing fallback.
- Catch-up/reconnect reads persisted facts but does not replay old physical
  throws. A reconnecting actor can answer an already-open window without an
  animation from another client.
- Test actor response/window arrival orders, window/plan arrival orders within
  the existing network buffer, duplicate/replayed facts and stale settlement.
- In two clients, hold the fighter's inspiration choice open for several seconds:
  both must have already seen the same d20; only the fighter has offer controls.
  Repeat Keep and Spend, then verify a later changed total if applicable, a
  downed monster on both clients, no second d20, and the next attack.
- Reverse actor/witness and test without inspiration. Preserve missing-plan,
  off-table retry, explicit nonphysical fallback, concentration and run-ended
  behavior. Do not change the live API without operator coordination.

## Boundaries and delivery

No damage dice, both-d20 rendering (#408), optimization pass, persistent True
Strike marker, perception/privacy redesign, new transport, or new roll model.
Use the existing :3006 web worktree for final proof with an explicitly selected
compatible API. All integration is through PRs. Toolkit consumers use actual
CI-issued module versions, never invented tags or committed local overrides.
Keep this design PR open until the scoped two-player proof is accepted.
