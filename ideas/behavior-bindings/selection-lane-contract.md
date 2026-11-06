# Independent machines and the first selection lane

This specifies the next boundary for [consumer plan #877](https://github.com/KirkDiggler/rpg-project/pull/877),
following the accepted [source → instance contract](source-instance-contract.md)
and [R1–R7](consumer-design.md). It carries forward the shipped composer's
selection semantics rather than inventing a resource scheduler. The encounter
adapter, beat mapping and migration are not implemented by this document.

## What a lane means here

The first consumer has **one action-selection lane per actor per offered
opportunity**. It produces at most one selected answer in each `Decide` call.
An existing turn loop may offer another opportunity using the remaining real
budget; that is not a second roll for the same selected answer and not a replay
of the occurrence that triggered the first opportunity.

A lane is not a machine, a bag, a definition or a universal actor state. An actor
may have independent `duty`, `investigate`, `needs` and body-state instances.
Their states, anchors, wakes and bags remain private. A data-only needs update
neither requests nor wins the movement/action lane. If a concern explicitly
requests a physical action through Pick, that request participates in the offered
lane and its actual budget checks.

No new source `lane` field, lane registry, concurrent action queue or simultaneous
resource-allocation policy is introduced in this first consumer. A future case
requiring independently selected concurrent resources must name those resources
and their owner; it is not inferred from adding another machine instance.

## Delivery and opportunity are different

| Caller input class | Component operation | Authority / result |
|---|---|---|
| Lawful data-only occurrence | `Observe` | Update listening machines without selection; no die or action permission |
| Clock/round notification | `Observe` | Supply lifecycle information; notification alone grants no action |
| Paid world opportunity (`time`) | Driver/compulsion gate, then `Decide` if delegated | Offer ordinary world action selection using the real remaining budget |
| Fight-turn opportunity (`turn`) | Same gate, then `Decide` if delegated | Distinct opportunity; a world-only Patrol does not subscribe accidentally |
| Existing rule-authorized reaction opportunity | Explicit caller authorization, then appropriate selection path | Preserve its actual permission/pricing/timing; do not obtain permission from a failed Observe |
| Actual work completion | Addressed `Observe` | Advance only the selected attachment's authored completion transition; no new opportunity |

`clock:fight` is the existing faction-addressed round notification, not an
individual actor's fight-turn opportunity. The exact source mapping for existing
reaction-capable recipes belongs to migration; this table does not silently
reclassify all heard/barred/fact inputs as either free actions or notifications.

**Do not catch `ErrSelectionRequired` and automatically retry with Decide.** A
Pick requested during data-only Observe is an input/definition-contract mismatch;
the observation commits none of its previewed changes. A caller may select only
when it independently has the appropriate opportunity. Source validation should
refuse an action request on a notification-only contract before normal execution.

The driver/compulsion wrapper stays ahead of ordinary choice. A concrete imposed
or custom intent bypasses composed selection; `Delegated{}` explicitly requests
it. Compulsion must not consume the ordinary lane's die or advance its pending
work merely because a composed policy exists.

## Selection order and commit behavior

The shipped `Composer.Decide` contract supplies the following sequence:

1. Preview the delivered beat against each instance's detached progress.
2. Select the first instance in authored order that requests a Pick.
3. Run only that instance's real weighted table with lawful facts and the die.
4. Commit applicable data-only transitions alongside an eligible winner.
5. Retain the full original progress of losing action candidates.

Source list order is **role precedence**, not weighted-table row priority.
Weights choose within the selected role. A large weight on a lower-priority
role does not make it win the lane.

There are three different no-action results:

| Result | Rolls / progress |
|---|---|
| No instance requests a Pick | No selection roll; data-only transitions may still commit |
| Selected role has no eligible row | Explained no-candidate hold, no roll; that requesting role keeps progress; **no fallthrough** to another role |
| Eligible explicit Hold row is selected | A real selection; its authored transition can commit |

A no-candidate hold does not cancel independent successful observations already
delivered, nor suppress a data-only transition in another instance in the same
successful Decide. Conversely, an error refuses the whole composer operation;
there is no partial data-only commit hidden behind a later observation/selection
failure. External reader/die effects are not a transaction the composer rolls back.

## Movement plus needs: the acceptance trace

Use three independent instances in authored order: `investigate`, `duty`, `needs`.
`duty` holds a pending waypoint. `investigate` is initially idle. `needs` has an
illustrative data-only `settled → waiting → due` lifecycle driven by clock
notifications; this is a contract fixture, not a new eating or hunger mechanic.

| Delivered input / offered opportunity | Required observation |
|---|---|
| `clock:world` notification | `needs` may advance its own state/anchor; duty's pending waypoint is unchanged; no selection roll |
| World opportunity, no active investigation | Duty selects its movement row once; needs retains its independently updated state |
| Qualifying heard occurrence, handled as data activation | Investigation becomes active without choosing movement; duty and needs do not reset |
| Next authorized world opportunity | Investigation wins its authored priority; duty's requesting candidate is deferred with its full progress intact |
| Notification makes needs reach `due` while investigation owns action | Needs advances independently; no synthetic `investigating-and-needs-due` combined state is created |
| Movement unavailable on the next world opportunity | The selected movement role holds without a roll or spent step; already committed needs progress remains |
| Investigation completes and a later world opportunity is offered | Duty resumes its pending waypoint; its progress is not inferred from current position |
| Fight opportunity | World-only duty/investigation receive no matching selection transition; an explicitly fight-capable policy may answer |
| Completion for duty's actual selected leg | Address only duty; neither needs nor another actor's same local ID advances |

For the same-input variant, give `needs` a data-only transition on the offered
beat while duty requests a Pick. Both changes can commit when the selection is
eligible: needs' data update and duty's selected transition. If duty has no
eligible row, needs' data-only update can still commit while duty stays put.
This is distinct from an errored operation, which commits neither.

## Completion does not own the lane

After selection, the caller executes the already-selected answer once. The
returned instance ID and table key identify the completion mapping; a paused
execution carries its work claim rather than reselecting on resume. Selecting a
movement target, exhausting movement, hitting an obstruction and ending a turn
are not proof that the target was reached.

Actual completion updates progress through Observe. It does not immediately ask
the next role to act, reset another role, or mint a free world/fight opportunity.
The existing clock/reaction owner decides when another selection call is legal.
This preserves separation between observation, progress, choice, permission and
physical execution.

## Required consumer assertions

These are future consumer tests, not a claim the adapter exists:

- `TestNeedsProgressDoesNotCompeteForMovement`: delivered data-only needs updates
  commit while duty/investigation selection and pending work remain independent.
- `TestDataOnlyProgressSurvivesSelectedRoleHold`: no-candidate hold retains the
  selected request, rolls nothing and permits other data-only updates in that
  successful Decide; no lower-role fallback roll occurs.
- `TestNotificationCannotAuthorizeSelection`: malformed Pick-on-notification
  refuses without partial commit or automatic retry as a decision.
- `TestWorldDutyIsNotOfferedAFightTurn`: `time`/`turn` opportunities and
  `clock:fight` notification retain different meanings; a deliberately dual-beat
  body can handle both opportunities.
- `TestCompulsionPrecedesTheOrdinaryLane`: imposed intent performs no composed
  selection or ordinary-lane roll; no duplicate action follows delegation.
- `TestCompletionDoesNotGrantAnotherOpportunity`: addressed actual completion
  changes the intended continuation but does not invoke the picker again.

## Remaining boundary: migration semantics

This contract describes the target lane, not an assumption that current encounter
execution already obeys it. Current `afterStep` commits a creature transition
before running its picks; current composer observation rejects Picks and a
no-candidate decision retains its requesting transition. Existing multi-pick,
terminal-world-state, wake and combat-table cases therefore require an explicit
normalization/parity contract. The canonical-runtime ruling does not justify
silently dropping a request or changing physical-state timing. That migration
section follows separately; runtime implementation remains outside this pass.
