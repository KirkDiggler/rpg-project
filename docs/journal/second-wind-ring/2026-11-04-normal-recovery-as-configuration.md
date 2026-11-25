# Normal recovery is configuration, not a different effect

Written: 2026-11-04. Work: [Keep signature combat and loot S1 — #862](https://github.com/KirkDiggler/rpg-project/issues/862).
Previous: [The item composes usage, its pool and reusable effects](2026-11-04-item-composes-usage-and-effects.md).

## Question

What restores the ring's budget, and should that policy belong to the shared
ability or to the item composition?

## Evidence

The initial rest suggestion was tentative. Checking the current implementation
showed Second Wind recovers on a short or long rest, while the Rage class pool
uses long-rest recovery. The earlier [baseline entry](2026-11-04-what-the-ring-grants.md)
records passing Second Wind short-rest and long-rest tests; those are existing
feature tests, not new ring tests.

After that clarification, the operator chose normal recovery for the Second Wind
ring and stated that studio configurability is the ultimate goal. This is
[R4 in ADR-0048](https://github.com/KirkDiggler/rpg-toolkit/blob/docs/second-wind-ring/docs/adr/0048-equipment-granted-abilities.md#settled-use-case-parameter).
No ring code or studio functionality has been implemented.

## Interpretation

The first ring's own pool restores on a short or long rest. The item composition
supplies that recovery policy separately from the shared healing behavior, just
as it supplies the ability level. A different authored recharge policy should
not require a different Second Wind implementation.

Studio configurability is a direction for the authored data, not permission to
build the editor now or invent a universal schema before the example consumes
it. The example can use explicit configuration and test the actual connections.

The clarification does not set the illustrative Rage item's recovery rule.
Nor does “normal recovery” select the ring's capacity or decide whether it must
be worn, carried or otherwise associated with the resting character.

## Next

Select the first ring's capacity, then the equipment/association rules that make
its grant and recovery eligible. The planned tests should exercise short and long
rest separately and refuse unrelated rest events. These remain intended checks,
not observed ring behavior.
