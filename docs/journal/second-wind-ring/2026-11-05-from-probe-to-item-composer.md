# From a test probe to an item composer

Written: 2026-11-05. Work: [Keep equipment S1 — #862](https://github.com/KirkDiggler/rpg-project/issues/862).
Previous: [Composition layers and provisional names](2026-11-05-composition-layers-and-provisional-names.md).

## Question

What belongs to the item package rather than its test harness, and what should a
rulebook consumer receive from a granted ability? The operator authorized merging
the effects cleanup and continuing the item example to inform the ADR.

## Evidence

[Effects cleanup #2276](https://github.com/KirkDiggler/rpg-toolkit/pull/2276) merged
at `b5b2c101`. Its complete diff and current-head CI were checked; a head comment
review records the mechanical-change verdict and the operator's merge direction.
The item consumer now pins the canonical main pseudo-version
`v0.0.0-20261105185206-b5b2c1012341`. Examples do not receive release tags. A temporary
retained ref protected the previous provider pin until the re-pin was pushed.
The item PR and ADR were not merged.

[Item PR #2273](https://github.com/KirkDiggler/rpg-toolkit/pull/2273), at
[`00c13b72`](https://github.com/KirkDiggler/rpg-toolkit/commit/00c13b7247be425760b09c60ab523dd4438fa7d3),
now has an exported **experimental** package:

- `New(Config)` owns an instance, validated authored grants and named pools.
- `Attach` / `Detach` use real effects outputs on a caller-owned bus.
- `Offers` returns copied declarations, source identity and item-budget affordability.
- `Use[Result]` owns the selected grant's item charge and returns a consumer's typed
  result; only this operation is generic, not Item or Config.
- `Data` saves identity, definition ref and remaining pool counts; no bus, closure,
  offer or mutable effect state is persisted.

Second Wind parameter validation and real `healing.Resolve` stay in the consumer
tests, outside the generic package. The prototype's grant ref is
`example:abilities:second_wind`; it does not pretend the production class-feature
loader accepts this new parameter shape. Shared healing arithmetic is exercised,
not full production feature reuse.

At this head, the full module race suite passes: fourteen suite methods, the
import-boundary check and the runnable example. Vet and lint pass; the CI-equivalent
coverage run reports 83.6% statement coverage. Changed-module CI and ci-status are
green. Three intentional mutations fail their specific assertions: removing the
recovery holder filter, charging zero instead of the authored price, and exposing
the internal parameter buffer. All are removed.

The initial CI failure was real but not a gameplay failure: CI combined our branch
with the newly enabled import-grouping settings on main, while local lint used the
older base. Merging main reproduced it locally. Separating the third-party test
import from local imports fixed the source; no check was weakened. The existing
raw link-check experiment also needed to ignore inline Go code to avoid confusing
a generic signature with a Markdown link.

The existing top-level `items/item.go` was also inspected: it declares property
and equipment interfaces, not this grant/pool composer. Its relationship to an
extracted tool is not decided by giving this example the same package name.

## Interpretation

The ownership split is now observable in package use rather than private test
fields. Definitions are authored data; Item owns runtime mutation; saved Data and
returned Offers are detached copies. Two grants can explicitly reference one pool,
while two same-definition items remain separate owners. That demonstrates a
mechanism, not a selected gameplay rule for every multi-ability item.

Presence and affordability differ: an attached ring can still expose its grant
when its budget is exhausted. The example does not pretend that its budget answer
also covers actor action economy or every rulebook eligibility condition.

The typed resolver is a candidate seam. It must validate/compute a value without
world delivery or reentrant item mutation; only then can a failed local reservation
be refunded safely. It does not establish arbitrary transaction rollback or an
interruption/reaction protocol. The consumer still owns delivering HP changes or
ongoing effects exactly once.

The generated effects recipe still has one state. No evidence here calls for a
new machine, shared-machine extraction, or arbitrary passive/stateful item recipes.

## Next

Inspect the candidate API and the consumer call before freezing anything. Use
these results to refine the ADR's grant/activation and state-reporting seams, then
exercise a contrasting consumer where needed. Production action definitions,
class-feature reuse, world delivery, passive lifetimes and real equipment/rest
eligibility remain unproved. This is not a production item release or permission
to extract the current API unchanged.
