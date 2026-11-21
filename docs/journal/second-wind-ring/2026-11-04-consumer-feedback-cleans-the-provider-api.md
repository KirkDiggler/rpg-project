# The consumer makes the construction API hard to read

Written: 2026-11-04. Work: [Keep equipment S1 — #862](https://github.com/KirkDiggler/rpg-project/issues/862).
Previous: [The first consumer exposes the grant seam](2026-11-04-consumer-probe-exposes-the-seams.md).
Prerequisite cleanup: [toolkit#2275](https://github.com/KirkDiggler/rpg-toolkit/issues/2275).

## Question

What does another composer need from the effects API before we expand this
consumer? The operator liked the composition but found its positional Compose
call hard to read and requested an input struct and provider cleanup first.

## Evidence

The item probe called Compose with seven positional arguments, including an empty
vocabulary, nil parameters and an empty state. The adjacent effects/machine.New
constructor had five positional arguments. The remaining smaller accessors and
callback contracts did not have this same construction problem.

[Provider PR #2276](https://github.com/KirkDiggler/rpg-toolkit/pull/2276), at
[b3a8be6c](https://github.com/KirkDiggler/rpg-toolkit/commit/b3a8be6c6759), introduces
value `ComposeInput` and `machine.NewInput` structs. Return types, defaults,
validation timing, bag ownership and lifecycle behavior remain unchanged.

The pre-change provider race suite passed. Migrating its callers first failed
compilation on the absent types/signatures. After implementation, both package
race suites, the candle example, vet, lint and benchmark smoke pass. This is an
API migration with existing behavior checks, not a newly discovered gameplay bug.

Removing State from the input forwarded to machine.New caused the existing
byte-equal load test to fail: restored Rage became fresh instead of raging.
Restoring that field returned the suite to green.

The separate [item consumer PR #2273](https://github.com/KirkDiggler/rpg-toolkit/pull/2273)
adopts the actual pushed provider pseudo-version
`v0.0.0-20261105035942-b3a8be6c6759`. Its old call then fails compilation, proving
it is consuming the new API rather than an incidental sibling checkout. The
named call passes its full race suite, executable example, vet and lint.

## Interpretation

The example exposed an ergonomic problem that provider-internal tests did not
make obvious. Named fields distinguish the recipe, machine vocabulary, output
vocabulary, holder, source, parameters and restored state at the use site.
They do not select a new item schema or move any effect responsibility.

This is a bounded prerequisite, not an expansion of the equipment story. Two
constructor inputs are cleaned up; no generic accessor/callback rewrite or
public output wrapper is added. Separate module PRs and a real provider pin make
the consumer evidence reproducible without a local source override.

## Next

Inspect the cleaned provider boundary before growing the consumer further. The
grant/activation declaration and item-state reporting remain the architectural
questions; this cleanup does not settle either or approve a production item API.
