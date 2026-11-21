# What does a ring actually need to grant?

Written: 2026-11-04. Work: [Keep signature combat and loot S1 — #862](https://github.com/KirkDiggler/rpg-project/issues/862).

## Question

How can equipment grant Second Wind using reusable behavior, rather than giving
the ring its own implementation? The ring is a bounded example of the keep's
NPC powers becoming usable equipment. We are beginning the ADR/investigation,
not implementing or publishing a finished design.

The conversation began with possible shared-machine research. The operator has
now separated that downstream question from this consumer's need: mind/machine
must frame its own problem; this session investigates the ring's composition.
That is a summary of the preceding conversation, recorded today—not a journal
entry retroactively dated to the earlier discussion.

## Evidence

Inspected toolkit main at
[`3182088c`](https://github.com/KirkDiggler/rpg-toolkit/commit/3182088c4cf0898596aa0c0f4eafe50bc5cc6d21):

- [`examples/effects/doc.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/3182088c4cf0898596aa0c0f4eafe50bc5cc6d21/examples/effects/doc.go)
  and [`composed.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/3182088c4cf0898596aa0c0f4eafe50bc5cc6d21/examples/effects/composed.go)
  describe and implement a condition composer: machine transitions change the
  currently attached outputs. This is existing code, not merely the effects proposal.
- [`features/second_wind.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/3182088c4cf0898596aa0c0f4eafe50bc5cc6d21/rulebooks/dnd5e/features/second_wind.go)
  owns uses and rest recovery, requires positive fighter levels, and publishes
  immediate healing of 1d10 plus the owner's fighter level. Its tests explicitly
  refuse non-fighter owners without spending the use.
- [`healing/healing.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/3182088c4cf0898596aa0c0f4eafe50bc5cc6d21/rulebooks/dnd5e/healing/healing.go)
  already offers sourced healing arithmetic. It does not require a spell or bus;
  its caller owns payment/delivery and its recipient owns HP mutation.
- [`actions.Definition`](https://github.com/KirkDiggler/rpg-toolkit/blob/3182088c4cf0898596aa0c0f4eafe50bc5cc6d21/rulebooks/dnd5e/combat/actions/definition.go)
  has Attack, Cast and Sequence arms. Cast has immediate healing; a separate
  Healing arm illustrated as future work in ADR-0045 is not present.
- The [composable-conditions design at `aa41ff1e`](https://github.com/KirkDiggler/rpg-toolkit/blob/aa41ff1e65a6c3bd3f591849a33f892f18919449/docs/ideas/composable-conditions/design.md)
  explicitly separates Grant from Activation and Condition, leaves Grant outside
  its story, and keeps instantaneous healing out of stored conditions. This is
  the design's stated scope, not proof of a completed grant implementation.

Fresh baseline commands in the isolated toolkit worktree:

```text
examples/effects$ go test -count=1 ./...
PASS: composer and machine packages

rulebooks/dnd5e$ go test -count=1 ./features -run TestSecondWindTestSuite -v
PASS: all 14 suite tests, including fighter-level lookup, non-fighter refusal,
      use serialization, healing publication and rest recovery

rulebooks/dnd5e$ go test -count=1 ./healing ./combat/actions
PASS: both existing packages
```

These are existing-component checks. No ring consumer has been written or run;
item grants, class/item coexistence, transfer and ring recharge are unverified.
No production, multiplayer or full-repository test claim is made.

## Interpretation

The first useful question is not whether two machines can be extracted. It is
how an item grants an opportunity to act, how using it pays a cost, and which
existing behavior produces the result. Those responsibilities may compose
without becoming one package or one state machine.

Second Wind is a useful pressure case precisely because it is an immediate heal,
not Rage's ongoing modifier. Treating every effect as a long-lived condition
would hide that distinction. Equally, handing the ring today's feature object
would silently inherit its fighter-only assumption unless we choose that rule.

The proposed ADR compares direct feature reuse, explicit grant/activation/effect
composition, and item-specific behavior. Composition is the recommended direction
to investigate; no API or item rules have been accepted.

## Next

First settle what the ring should do for a wearer with no fighter levels. Today's
refusal must not decide that product requirement by accident. Then establish the
amount, use ownership/restoration and grant lifetime needed by the example, and
turn those decisions into named consumer tests before implementing it.

Later evidence can show which effects machinery the ring actually consumes. If
it does not exercise ongoing effects, record that limitation and propose a
contrasting consumer rather than claiming this example proves all conditions and
spells. The journal and ADR will preserve that result whichever direction it takes.
