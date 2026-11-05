# What must a patrol remember to resume?

Written: **2026-11-05**.
Work: [NPC behavior S1 — #866](https://github.com/KirkDiggler/rpg-project/issues/866),
under [the machine foundation story](https://github.com/KirkDiggler/rpg-project/issues/859).
Previous entry: [Can the existing machine express the guard trace?](2026-11-04-can-the-existing-machine-express-the-guard-trace.md).

## Question

The operator agreed to the next capability slice: patrol progresses, a clang
activates investigation, investigation temporarily owns movement, and patrol
resumes from retained progress after investigation ends. What does the existing
patrol retain, and what should the first resumed movement mean?

This is a more demanding question than the earlier mode-switching probe.
Remembering that the guard is patrolling is not necessarily remembering the
work its patrol had reached.

## Evidence

Inspected and exercised toolkit
[`9f3e64d0`](https://github.com/KirkDiggler/rpg-toolkit/commit/9f3e64d03700db8292116b3a537ac32887a896b6).
The relevant patrol, dwell, table-driver and picker source files also match
fetched main `3182088c`. The
[slice's evidence and pending decision](https://github.com/KirkDiggler/rpg-project/issues/866#issuecomment-6648376210)
record the source links and commands.

- [`patrolStep`](https://github.com/KirkDiggler/rpg-toolkit/blob/9f3e64d03700db8292116b3a537ac32887a896b6/rulebooks/dnd5e/encounter/patrol.go)
  derives the next leg from the creature's position. Off-route, it chooses the
  nearest waypoint, with ties in authored order. There is no remembered cursor.
- [`spendDwell`](https://github.com/KirkDiggler/rpg-toolkit/blob/9f3e64d03700db8292116b3a537ac32887a896b6/rulebooks/dnd5e/encounter/dwell.go)
  clears the waypoint wait when the creature leaves. Its remaining rounds are
  not a suspended continuation.
- [`TableDriver.Act`](https://github.com/KirkDiggler/rpg-toolkit/blob/9f3e64d03700db8292116b3a537ac32887a896b6/rulebooks/dnd5e/encounter/tabledriver.go)
  selects the machine-named table and uses the real weighted picker.
  [`behavior.Pick` / `land`](https://github.com/KirkDiggler/rpg-toolkit/blob/9f3e64d03700db8292116b3a537ac32887a896b6/mind/behavior/table.go)
  treat row order as a mapping of die faces, not role priority.

A comment in the driver's patrol branch still says rows above it “fire first.”
That conflicts with the picker contract; the clarification is filed separately
as [toolkit#2274](https://github.com/KirkDiggler/rpg-toolkit/issues/2274), without
changing runtime behavior or making that cleanup a prerequisite.

Two existing focused tests were run from `rulebooks/dnd5e/encounter`:

```sh
go test -count=1 -run '^TestTableSuite$/TestALeftAndARightRowShareTheRoll$' -v .
go test -count=1 -run '^TestARowAboveThePatrolInterruptsTheDwellAndLeavingEndsIt$' -v .
```

Both pass (0.005s and 0.019s). The first uses faces 1 and 101 on a d200 to
select different equally weighted search directions. The second deliberately
expects the remaining dwell to be discarded after a disturbance and a fresh
full wait to begin on return.

These are observations of **existing behavior**, not a new retained-resumption
test made green. No new production code, continuation API, checkpoint schema,
full-module gate or live-game walk is claimed by this entry.

## Interpretation

The earlier probe establishes that a machine can remember its current mode.
This investigation exposes a different kind of state: **where an interrupted
behavior's work should continue**. Current position cannot always reconstruct
that history after another behavior moves the actor elsewhere.

The missing capability therefore is not just another state or conditional.
It needs an explicit continuation meaning and an owner. A patrol-specific
field added wherever convenient could hide that question; a general coordinator
built before defining the continuation could invent an answer just as easily.
Neither implementation is selected here.

Priority and weighted choice also are separate. Choosing investigation as the
movement-owning role should not be implemented by pretending the first row of
an existing weighted table always wins. Once a role is selected, scripted dice
can test its real action choices without changing the table's probability
contract.

There is a concrete ambiguity to settle before writing the expected resumed
movement. If interruption happens partway from A to B and the investigation
ends near C, retaining progress could mean continuing toward the unfinished
waypoint B, or returning to the exact interruption cell before finishing the
leg. Both differ from the current nearest-waypoint policy. The first is the
recommendation for the initial scenario; neither is accepted yet.

This may provide evidence for an architectural decision about behavior-owned
continuations. It does not yet justify choosing a shared-machine package,
a generic scheduler, or a transactional runtime.

## Next

Resolve the [resume-target choice on the slice](https://github.com/KirkDiggler/rpg-project/issues/866#issuecomment-6648376210),
then write the checked implementation plan around that behavior: the state
owner, produced/consumed continuation contract, real controlled-roll scenario,
and applicable reload proof. The approved slice outcome is not being reopened;
this is the specific remaining choice that changes its assertions.

Keep the initial question and current-behavior receipts intact. A later entry
should show the new test's actual pre-change failure and the implemented
boundary, or explain what further evidence changes the proposed approach.
