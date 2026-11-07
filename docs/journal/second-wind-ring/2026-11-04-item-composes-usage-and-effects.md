# The item composes usage, its pool and reusable effects

Written: 2026-11-04. Work: [Keep signature combat and loot S1 — #862](https://github.com/KirkDiggler/rpg-project/issues/862).
Previous: [The ring supplies the Second Wind level](2026-11-04-ring-supplies-the-level.md).

## Question

Whose budget funds an item-granted ability, and what responsibility does that give
the item composition?

## Evidence

In the conversation following the previous entry, the operator confirmed separate
pools, using a Raging ring and a Reckless Attack hotdog as further examples. The
operator then confirmed that the charge is on the ring: a spent ring stays spent
when passed to another character.

The operator described the item as a composer like a condition, using the same
underlying effect while orchestrating usage and resource pools. Those decisions
are now R2 and R3 in [ADR-0048](https://github.com/KirkDiggler/rpg-toolkit/blob/docs/second-wind-ring/docs/adr/0048-equipment-granted-abilities.md#settled-use-case-parameter).
No item implementation or consumer test has run. This entry records requirements,
not evidence that the proposed composition already works.

## Interpretation

The ring has its own durable budget, distinct from the character's class uses
and another ring's budget. Its remaining uses follow the item through transfer;
changing wearers is not a recharge. The shared ability ref therefore cannot by
itself identify the mutable item-instance budget.

The item is more than a pointer to a feature: it composes the granted behavior,
parameters such as level, and the usage/resource relationship. A condition is
another composer with a different job, not necessarily the same runtime type.
The shared part is underlying behavior, not a requirement to merge all composers
or move item orchestration into the generic effects tool.

The implementation still has to respect existing action-data and payment
boundaries. An item composition cannot become a second path that spends the
same cost or applies the same healing twice. That is an architectural constraint
to exercise in the example, not an API selected by this conversation.

## Next

Settle what restores a charge on the first Second Wind ring, then its initial
capacity and the availability/removal rules required for the example. Pool
sharing between multiple abilities on a single item remains open; separate
item-owned pools do not settle that question.

The example should eventually spend a ring use, verify class/other-item budgets
are unchanged, transfer and re-equip the ring, and round-trip its state without
refilling it. These are intended checks, not passing tests. The new item examples
motivate the design but do not add implementation deliverables to this slice.
