# One shared d20, a later choice, then an outcome

Status: design approved by Kirk in conversation. Implementation checkpoints are
in [plan.md](plan.md); runtime delivery is not yet complete.

Issue: #410. Parent: #289 (Roll Dice Together).
Related: rpg-dnd5e-web#964, #996 / PR #1003; #408 (both d20 faces).

## Player promise

The roll is the roll whether inspiration is spent or kept. Both players see
that same d20. After it settles, only the actor receives the actionable
inspiration offer. Until the actor answers, neither the answer, an inspiration
bonus, nor hit/miss is known. The eventual outcome belongs to the original roll;
it does not request another d20 or keep its target artificially standing.

## Verified current code

- Toolkit `session/attack.go:poseAttackWindow` persists RollWindowOpened and
  returns roll/total plus a provider presentation ID. Resolution is paused
  before its final outcome. `answerPostRoll` records Struck/Missed later, reusing
  the presentation ID at a different Story sequence. Planning verified that the
  current window payload is broadcast to the full roster and filtered in the
  web; the actor-private wire delivery specified below is a change, not an
  existing invariant.
- Protos `session/presentation/v1alpha1/service.proto:DiceThrowPlan` carries
  session, bound roller, presentation ID, attempt, collider fingerprint and
  physical states. **It does not carry the authoritative d20 result.**
- API's session-presentation path binds identity and validates a visual plan;
  it does not resolve combat. That boundary should remain intact.
- Web `SessionEncounterView` builds witness expectations and the rendered face
  from its current authoritative dice request, presently supplied by the final
  Struck/Missed for a peer. `LocalWorldDieWitnessInbox` expires an unmatched plan
  after 1500ms of receipt-relative time. An inspiration decision can outlast it.
- Local playtesting isolated this failure to the fighter holding inspiration.
  Web#996 separately fixed a second physical roll being armed at the later
  outcome, and the old response's unresolved downed-reveal hold. Those
  regressions remain load-bearing during this correction.

Inspected refs: web `93636165`, API cached `origin/dev` `736084b`, toolkit cached
`origin/main` `b3f899c5`, protos cached `origin/main` `5d9ef1a`. Refresh and reconcile
before implementation; these observations are not a claim about all running
container revisions.

## Approaches considered

1. **Extend the witness buffer timer. Rejected.** It substitutes an estimate for
   a human decision and still cannot show the d20 before the decision.
2. **Let the visual plan supply a result. Rejected for this slice.** Its sender
   supplies physics, not game truth. Inferring a die result from its rotation or
   accepting a new sender-authored face would violate the existing boundary.
   Looking up authoritative faces in the presentation API would instead couple
   that service to combat and create a second result-delivery mechanism.
3. **Publish the authoritative roll through the existing session event spine.
   Recommended.** Keep shared motion on its current stream. Join those existing
   paths by the provider's opaque presentation ID before the outcome exists.

## Minimal authoritative contract

Add one typed session event, **AttackRolled**, for a supported player weapon
attack's selected natural d20. It is a roll-stage fact, not an outcome.

Payload:

- `presentation_id`: the same provider-minted ID used by AttackResponse, the
  shared throw, and the final Struck/Missed;
- `attacker`, `target`: provider member identities;
- `attack`: the existing AttackRef;
- `roll`: the selected natural d20, before an optional post-roll die;
- `total`: the rolled total before that post-roll choice, explicitly not a final
  total. Consumers must not add later bonuses to this themselves.

The enclosing Event supplies session and recipient-local sequence as usual.
There is no AC, hit/miss, critical verdict, damage, offer, chosen option, or
inspiration result on AttackRolled. It is not a sentinel-filled Struck/Missed.

Add `presentation_id` to the existing private **RollWindowOpened** payload so
it names its roll directly. Do not reuse equal sequence numbers or coincidentally
matching roll/total as the new contract's correlation rule.

Keep existing AttackResponse and Struck/Missed fields compatible. Their sequence
numbers remain their own receipt/Story coordinates; different phases of one
attack need not share a sequence. The d20 value and presentation ID cannot change
when inspiration changes the final total.

### Delivery and ownership

- **Toolkit** owns the immutable rolled fact and its transaction ordering. Expose
  the same rolled-stage data on both paused and completed player-attack paths;
  session must not reconstruct it by reversing final modifiers. Capture it at
  the roll boundary and record it before the private window or final outcome.
- Persist before broadcast through the existing session/encounter machinery.
  Resume records the outcome, not a second AttackRolled. Failure/retry and
  duplicate delivery must not create a new roll identity.
- Send AttackRolled to the witnesses allowed by the encounter's existing attack
  audience policy, including the actor. Do not widen unrelated perception rules.
  The two-player inspiration case must supply both clients with the same roll
  payload even though their Event.seq values differ.
- RollWindowOpened becomes actor-private on the wire, matching its existing
  actor-only controls. AttackRolled must not expose whether
  that actor possesses a spendable offer. The private offer cannot be actionable
  on a witness client.
- **Protos** transcribe these typed facts. **API** translates them without adding
  rules, reconstructing bonuses, or reading physics. No change is required to
  the authority-free DiceThrowDraft/Plan format for this correction.

## Web presentation: one roll lifecycle, separate Story beats

Extend the existing combat-presentation reducer to accept the roll-stage fact.
Use the provider presentation ID as the physical roll identity, with session
and typed identity validation. Keep recipient-local Story identities/order for
roll, private window, and outcome separately.

- An actor response and AttackRolled are two observations of one roll. Either
  may arrive first; they arm one existing die control, never two.
- A witness's AttackRolled supplies the authoritative face and expected ID for
  the existing witness inbox, collider validation and LocalWorldDieLayer. The
  published plan can now be matched without waiting for Struck/Missed.
- Keep the existing short inbox as a buffer for ordinary network ordering, not
  for a human decision. Do not lengthen its timer or bypass fingerprint checks.
  Once a live plan is matched, play it independently of any pending choice.
- Existing physical settled events release the actor's private choice and its
  concealed Story suffix. Do not add a timeout-based decision reveal.
- A later Struck/Missed completes that same presentation. It may change the
  total but not the natural d20. Preserve settlement, retire provisional holds,
  reveal authoritative downed state appropriately, and never re-arm the die.
- No future choice, offered-die result, or hit/miss is invented while waiting.
  After the actual answer, render the available provider-authored final facts.
  This slice does not invent an inspiration-spent announcement if the current
  provider outcome does not carry one.
- Keep the existing shared visual renderer, throw stream, retry handling, and
  explicitly nonphysical fallback paths. Do not add a second animation manager
  or result store. History is not a request to physically replay old throws.

This does not redesign general witness suspense for every ordinary attack
(web#865). Missing visual delivery remains a presentation limitation, never an
instruction to mutate or stall the backend. A renderer failure uses the existing
explicit fallback; a successful live witnessed throw still uses its real terminal.

## Legacy and reconnect behavior

This is additive under the workspace's current few-consumer policy.

- Old records lack AttackRolled and/or the private window's presentation ID.
  Preserve their existing outcome-based rendering and documented nonphysical
  catch-up behavior; never fabricate a past roll-stage event.
- New clients with an old provider cannot guarantee pre-choice witness dice.
  Surface that integration limitation rather than silently treating a visual
  plan as authority. A coordinated provider/client deployment is required for
  the new behavior to be complete.
- Catch-up projects the persisted rolled and final facts without re-rolling or
  replaying old physical plans. A reconnected actor can answer an already-open
  window without an animation that happened on another client.
- Replayed/duplicate roll events, window events and outcomes are idempotent.
  Conflicting immutable roll identities fail closed and are diagnosable.

## Verification / done when

### Toolkit and wire

- Spend and Keep both produce exactly one AttackRolled before the private
  window, then a later outcome with the same presentation ID and natural d20.
- A normal supported player attack also produces one rolled stage and one
  outcome, in order, without requiring a pending inspiration offer.
- Two recipients get the same shared roll ID/face and different permissible
  recipient-local sequences; only the actor receives the private window.
- No final-outcome field leaks through the early payload. Persistence failure
  does not publish an unrecorded fact. Live/catch-up conversion preserves it.

### Web

- Exercise roll-event/response and roll-event/visual-plan arrival orders.
- Show the witness die before the actor selects Spend/Keep, even when the actor
  leaves that choice unanswered far longer than 1500ms.
- Keep the choice hidden until the actor's matching terminal; an unrelated
  settlement cannot release it. Keep offer controls absent from witnesses.
- Feed the later outcome at a different seq and a changed inspiration total;
  no second d20, no stale target hold, and the next attack can be rolled.
- Check missing plan, retry/off-table, explicit renderer fallback, catch-up,
  reconnect, duplicate events, and unsupported legacy records without guessing.
- Preserve the existing concentration and run-ended presentation tests.

### Two-player local proof

Use the named web#996 preview path (currently :3006) with the compatible provider
selected explicitly for the test. Do not restart or replace the shared running
API without operator coordination.

1. Give the fighter inspiration through normal gameplay.
2. Fighter attacks; both clients see the same selected d20.
3. Leave the private offer open for several seconds; the witness has already
   seen the roll and has no Spend/Keep controls or final outcome.
4. Keep, then repeat with Spend: the final authoritative outcome follows the
   choice, a downed monster is downed for both, and neither client sees a second
   attack d20.
5. Attack without inspiration and reverse the actor/witness roles.

## Boundaries and delivery

No damage dice, both-d20 rendering (#408), optimization pass, persistent True
Strike target marker, perception redesign, new dice renderer, or change to the
rules engine's Spend/Keep behavior.

After design approval, add the implementation plan to this same design PR and
create owning-repository slices on Project 19. Develop from the web contract
inward; land tagged toolkit support, generated protos/API pins, then web. Keep
one branch per repository for the wave. Keep this design PR open through the
end-to-end proof. All integration goes through PRs.
