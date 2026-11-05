# How can the same behaviors serve authors and generators?

Written: **2026-11-05**.
Work: [machine foundation — #859](https://github.com/KirkDiggler/rpg-project/issues/859).
Previous entry: [What must a patrol remember?](2026-11-05-what-must-a-patrol-remember.md).
Decision proposal: [ADR-0049 at its first revision](https://github.com/KirkDiggler/rpg-toolkit/blob/a16c4573829718d2e119c832eb05359029e6cc36/docs/adr/0049-behavior-is-a-composer.md),
**Proposed, not accepted or implemented**, on
[toolkit PR #2271](https://github.com/KirkDiggler/rpg-toolkit/pull/2271).

## Question

The operator emphasized that the reusable behaviors are also for procedural
generation: what parameters do they take, and how can suitable values be chosen?
The studio should make the components, bindings and their reuse understandable.
The operator also identified making `mind/behavior` the composer as an
ADR-worthy thought, recorded with its limits in the
[direction/evidence comment](https://github.com/KirkDiggler/rpg-project/issues/859#issuecomment-6648962999).

What responsibility would that composer own, and what should stay with the
producers that choose its configuration? Reusability means more than making a
patrol's hard-coded numbers configurable; it also means generating coherent
combinations and preserving a meaningful authoring surface.

## Evidence

The existing code is not a blank slate. Read at toolkit `9f3e64d0`:

- [`keepgen.Options` and `Generate`](https://github.com/KirkDiggler/rpg-toolkit/blob/9f3e64d03700db8292116b3a537ac32887a896b6/rulebooks/dnd5e/encounter/dungeonspec/keepgen/keepgen.go)
  provide seeded generation, garrison/patrol knobs, and loader validation before
  generated content is returned.
- [`garrison.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/9f3e64d03700db8292116b3a537ac32887a896b6/rulebooks/dnd5e/encounter/dungeonspec/keepgen/garrison.go)
  chooses patrol routes that fit floor/placement constraints. It is not
  independent random coordinates for each waypoint.
- [`emit.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/9f3e64d03700db8292116b3a537ac32887a896b6/rulebooks/dnd5e/encounter/dungeonspec/keepgen/emit.go)
  writes ordinary table bindings and an embedded listening-post state recipe.
  Its benchmark artifact explicitly says to regenerate rather than edit it;
  that producer's measurement contract is not an existing editable behavior
  library or a proposed restriction on ordinary generated authoring content.

Fresh verification from `rulebooks/dnd5e/encounter`:

```sh
go test -count=1 -run '^(TestGarrison_EmitsBindings|TestGarrison_Refuses|TestGarrison_PostsListen)$' -v ./dungeonspec/keepgen
```

Pass, **1.362s**: three top-level tests including four refusal subcases. They
exercise generated bindings, declared count constraints and listening-post
recipes. They do not test a generic behavior composer, studio editing, retained
continuations, a generation-profile language, or the gameplay quality of sampled
parameter values. No new runtime behavior was implemented in this entry.

The ADR uses the repository's template and remains Proposed. Its alternatives
include keeping composition in the encounter, making `mind/behavior` the
composer, adding a separate composer module, and overloading the machine.
Earlier ADR-0016 is treated as a historical proposal, not as evidence that its
example interfaces exist. Existing driver and knowledge boundaries are preserved
by the proposal; no other ADR status is changed.

The number check found an existing 0048 collision between an equipment proposal
and an older encounter-backfill issue. It was
[reported to that existing issue](https://github.com/KirkDiggler/rpg-toolkit/issues/2142#issuecomment-6648840808);
this record uses 0049 without rewriting another session's work.

## Interpretation

The earlier entries followed a guard and then the missing continuation state.
The producer perspective adds a useful test: **can the same behavior definition
be configured by a designer and assembled by a generator without a second
runtime path?** A handwritten fixture alone cannot establish that contract.

The proposed cut has three distinct things: a definition describes supported
behavior and parameters; a binding supplies concrete values/references; a
runtime instance owns its private progress. Sharing a definition should not
share one guard's interrupted patrol with another guard.

There is another distinction between **valid** and **suitable** values. Types
and structural constraints can reject impossible configuration. Choosing values
that produce the desired experience needs content goals. For a desired patrol
revisit interval, route length, actor speed and waypoint waits need considering
together; independently valid long routes and long waits may fail the goal.
This is an illustrative profile question, not a new tuning rule or fixed number.

The proposal puts that contextual selection in authored generation profiles and
the generator, rather than inside the runtime machine or composer. Those profiles
may choose correlated combinations or derive values from earlier choices. Their
schema and algorithms are not selected. Existing seeded content generation also
does not imply changing gameplay rolls to a seeded simulation.

Making `mind/behavior` the composer is therefore a real boundary proposal, not a
package rename: it would expand a module currently centered on policy selection
into explicit behavior-instance composition. A separate composer remains a
credible alternative if that expansion cannot stay cohesive. The ADR, not this
journal, holds the proposed decision and consequences.

This authoring view may put a resume policy in a behavior definition rather than
make it universal. It does not silently answer the
[concrete S1 resume-target question](https://github.com/KirkDiggler/rpg-project/issues/866#issuecomment-6648376210)
or select a checkpoint API. The previous uncertainty remains visible.

## Next

Use S1's patrol/investigation as the first candidate for a definition, binding
and private-progress contract. Show how the studio would supply the route and
how a generator could supply an equivalent binding, then let the real consumer
test challenge that boundary with explicit inputs and rolls.

Keep the work bounded: the first retained-resumption slice need not implement a
complete generator or editor. But its inputs should not be designed as a
one-off Go fixture that cannot later be expressed by those producers. Selecting
quality profiles and proving editor round trips become explicit work where the
contracts require them, not assumed capabilities.

Review the proposed composer responsibility and settle the remaining concrete
behavior choices through [#859](https://github.com/KirkDiggler/rpg-project/issues/859)
and [S1](https://github.com/KirkDiggler/rpg-project/issues/866). No ADR acceptance,
shared-machine extraction or new implementation plan is inferred from this entry.
