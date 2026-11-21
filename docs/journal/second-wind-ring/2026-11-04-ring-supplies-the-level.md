# The ring supplies the Second Wind level

Written: 2026-11-04. Work: [Keep signature combat and loot S1 — #862](https://github.com/KirkDiggler/rpg-project/issues/862).
Previous: [What does a ring actually need to grant?](2026-11-04-what-the-ring-grants.md).

## Question

What supplies Second Wind's level when the wearer has no fighter levels?

## Evidence

The operator answered today that the ring sets the level, giving level-1 and
level-3 Second Wind rings as examples. Rarity possibly changing that level was
suggested, not selected as a rule. The requirement is recorded as
[R1 in ADR-0048](https://github.com/KirkDiggler/rpg-toolkit/blob/docs/second-wind-ring/docs/adr/0048-equipment-granted-abilities.md#settled-use-case-parameter).

The earlier entry's inspected implementation uses the owner's Fighter class
level. No code has changed or ring example run since that inspection. The
operator's requirement is evidence of intended behavior, not implementation proof.

## Interpretation

The ring's grant supplies an ability-level parameter: a level-1 use heals
`1d10 + 1`; a level-3 use heals `1d10 + 3`. This makes a non-fighter use case
concrete without pretending the wearer has gained Fighter class levels.

The session interprets “sets the level for the wearer” as scoped to the
ring-granted ability, not mutation of the character's class progression or
replacement of their class-granted Second Wind. No class-level change was requested.

The original direct-feature-reuse alternative remains useful history, but the
unmodified feature cannot honor this parameter source. The potential reusable
boundary is behavior whose parameters can come from a class or an item, with
provenance retained. Its API and budget ownership are still undecided.

Rarity could become an authoring policy that chooses the level. We do not yet
need a rarity table, and the two example levels do not establish a valid range.

## Next

Settle whether the ring has its own uses or shares the wearer's class-granted
pool, then select restoration and grant lifetime. Keep those questions separate
from the settled level source.

The future example can use a controlled d10 result of 6 to expect calculations
of 7 and 9 for level-1 and level-3 rings on a non-fighter, while verifying class
levels are unchanged. These are proposed assertions, not test results. The HP
recipient still owns how much of that healing is actually applied.
