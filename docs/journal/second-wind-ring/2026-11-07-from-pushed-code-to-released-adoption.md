# 2026-11-07 — When is the extraction actually adopted?

Work: [first-cut extraction #705](https://github.com/KirkDiggler/rpg-project/issues/705).

## Question

A consumer working against a pushed commit proves integration, but not adoption
of a published module. What remained after the [source extraction and naming decision](2026-11-06-what-compose-names.md),
and how should the reused module path describe its retired API?

## Evidence

KirkDiggler approved `retract [v0.1.0, v0.2.1]` with a rationale identifying the
retired, non-drop-in-compatible API. After independent review, [provider #2291](https://github.com/KirkDiggler/rpg-toolkit/pull/2291)
merged as `c7cf2461`; CI published `mechanics/effects/v0.3.0` at that commit.
Normal Go version listing now returns only v0.3.0. Explicit queries still resolve
the three legacy tags and report their retraction rationale. The first probe
immediately after tagging saw stale proxy metadata; direct and subsequent normal
queries verified propagation rather than assuming it.

[Second Wind S1 #2273](https://github.com/KirkDiggler/rpg-toolkit/pull/2273) merged
as `b76a1f6d`, and [Rage/loot S2 #2287](https://github.com/KirkDiggler/rpg-toolkit/pull/2287)
as `3b82d2e4`. Both consume v0.3.0. Full race tests, examples, vet and lint passed
again on released pins, and both final-head CI runs passed. The final pin audit
also found two inherited main-commit pseudo-versions: resources and D&D could use
existing tags with identical code, so those were adopted without another release.

One proportionate independent consumer pass found three Minor documentation
issues: incomplete dependency arrows and stale provider/planning/CI descriptions.
All were corrected and their threads resolved. No runtime blockers were found.
The [implementation record](https://github.com/KirkDiggler/rpg-toolkit/blob/main/docs/ideas/item-effect-composition/implementation.md)
links the review, CI and merge receipts. No game-server or browser integration
was exercised by this cut.

## Interpretation

Retraction communicates withdrawal from new automatic selection; it does not
delete history or force existing consumers to migrate. That matches our module
model: changing a dependency pin is the deliberate adoption step.

The earlier source tests were useful, but the released-tag checks answer a
different question. Likewise, an empty CI list on a stacked PR was not success:
S2 needed a main target and its own actual head result. Documentation needed the
same care—its old future-tense language survived after the code had already moved.

Historical links must survive branch cleanup. The two preceding journal entries
now point to their original plan/implementation commits; their account of what
was known at the time is unchanged. Current guidance points to main.

## Next

The bounded extraction is delivered. The remaining architectural question is the
shared rulebook vocabulary and condition-creation boundary, followed by deliberate
production adoption—not a requirement to migrate every condition before making
these reusable tools available. The broader [effects story #703](https://github.com/KirkDiggler/rpg-project/issues/703)
and [item story #850](https://github.com/KirkDiggler/rpg-project/issues/850) stay open.

— cross-team agent, on behalf of KirkDiggler
