# What do existing bundles already bind?

Written: **2026-11-05**.
Work: [machine foundation — #859](https://github.com/KirkDiggler/rpg-project/issues/859).
Previous entry: [Where interrupted behavior keeps progress](2026-11-05-where-does-interrupted-behavior-keep-progress.md).
Contract sketch and first decision: [definition/binding discussion](https://github.com/KirkDiggler/rpg-project/issues/859#issuecomment-6668089982).

## Question

The operator agreed to shape the next contract through one reusable Patrol
definition, two guards with different routes, and independent progress. Before
adding another authoring system, what can the existing content compiler and
studio already express, and where does their notion of a reusable bundle stop?

## Evidence

Toolkit source inspected at
[`6d8a753a`](https://github.com/KirkDiggler/rpg-toolkit/commit/6d8a753a4e255e38040b1b63a2bfb731832095c2):

- [`BundleSpec` and `bundleResolver`](https://github.com/KirkDiggler/rpg-toolkit/blob/6d8a753a4e255e38040b1b63a2bfb731832095c2/rulebooks/dnd5e/encounter/dungeonspec/bundles.go)
  already provide site-local named row groups, `use`/`with`, declared defaults,
  required parameters and whole-node substitution. A plain `$path` can be
  replaced by a complete route, not a stringified approximation. Each use gets
  deep-copied nodes before typed row decoding and validation.
- [`bundles_test.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/6d8a753a4e255e38040b1b63a2bfb731832095c2/rulebooks/dnd5e/encounter/dungeonspec/bundles_test.go)
  covers structured route parameters, parameters passed through nested bundles,
  refusals and serialization that preserves the authored use rather than
  replacing it with the expanded runtime rows. The runtime `Params` strings
  are an explanation of a binding, not the authoritative parameter values.
- [`StatesSpec`](https://github.com/KirkDiggler/rpg-toolkit/blob/6d8a753a4e255e38040b1b63a2bfb731832095c2/rulebooks/dnd5e/encounter/dungeonspec/states.go)
  is a separate authoring surface that lowers one creature/faction recipe.
  It does not yet describe an ordered list of independent composer attachments.

Five existing focused compiler tests pass (**0.023s**), including thirteen
refusal subcases:

```sh
cd rulebooks/dnd5e/encounter
go test -count=1 -run '^(TestAUseRowCompilesToAGroup|TestARouteIsAParameter|TestAParameterCrossesIntoABundleUsedInside|TestAUseRowMarshalsBackAsWritten|TestTheFormRefusesWhatABundleCannotBe)$' -v ./dungeonspec
```

Web source inspected at
[`3d337992`](https://github.com/KirkDiggler/rpg-dnd5e-web/commit/3d33799299c7f132479ba29c6f58a81a5f151976):
[BundlesPanel](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/3d33799299c7f132479ba29c6f58a81a5f151976/src/concepts/world-building/BundlesPanel.tsx)
already edits bundle definitions, parameters, route defaults and rows, with a
text alternative. The separate
[creature state codec](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/3d33799299c7f132479ba29c6f58a81a5f151976/src/concepts/world-building/creatureStates.ts)
preserves supported state/transition content and newer carried fields. Its
parameterized-template behavior must not be inferred from the row-bundle codec.

This is web source inspection only: no browser walk or web test ran. The new
worktree has no installed Vitest; no dependencies were installed or reused to
manufacture a test claim. No toolkit/web runtime or authoring field changed.

## Interpretation

There is real producer-side machinery to reuse. The missing contract is not
simply a generic string-template engine. It is the ownership of a reusable
**stateful behavior** and its per-actor attachments.

A bundle remains one weighted row group. Treating a bundle's weight as behavior
priority would conflate the picker with the composer, and treating a route
parameter as a continuation would conflate configuration with progress. The
existing route row still uses the old position-derived patrol behavior.

The proposed separation is: definition owns parameter/lifecycle/selection
source; binding owns an attachment ID, definition reference and supplied values;
producer validates/materializes; composer owns private progress. The same local
attachment ID can appear on two different actors without sharing state.
Declared default resolution belongs at producer ingress, not as a hidden default
inside the runtime checker or a new random draw during reload.

There are two concrete gaps to keep visible. The new `ParameterSpec` handles
scalar kinds, while a route is structured domain data whose validation has an
owner. Also, a lifecycle whose state structure varies with route length needs
an explicit materialization rule; whole-node substitution of a row's path does
not by itself provide retained progress. Neither gap is solved by renaming a
bundle or by stringifying geometry.

The first producer choice is consequently meaningful: editable data definitions
that extend the existing authoring approach, or toolkit-provided builders with
editable parameters first. Data definitions preserve direct designer control;
registered builders can construct variable structures but require code for new
internal behavior logic. The recommendation is data authoring, with Go owning
primitives and explicit lowering—not an arbitrary scripting language. This
recommendation is not a selected schema or an implementation commitment.

## Next

Resolve the [definition authoring choice](https://github.com/KirkDiggler/rpg-project/issues/859#issuecomment-6668089982),
then use the two-guard example to name the smallest source/binding/materialized
contract. Preserve source uses for studio round-trip and keep started runtime
configuration separate from later source edits.

The eventual proof should bind the same definition twice, show the correct
independent resolved values and continuations, and exercise the new content
through the real compiler and studio round-trip. It must not stop at a passing
Go literal or claim the existing bundle tests prove the new composer bridge.

**Follow-up, 2026-11-05:** KirkDiggler selected authored definitions as data:
“I like authored definitions as data too.” The [R1 receipt](https://github.com/KirkDiggler/rpg-project/issues/859#issuecomment-6668590252)
answers the authoring choice above; [the design record](../../../ideas/behavior-bindings/design.md)
owns that ruling. The earlier recommendation and uncertainty remain as the
record of the investigation, not a still-pending question.

The [working example](../../../ideas/behavior-bindings/README.md) now proposes
two versus three waypoints to expose variable-route materialization rather than
prove only different coordinates in one fixed graph. That case is not yet an
executed test or supported syntax. Further inspection of the same `StatesSpec`
source also finds `blocks`/`holds` creature-body annotations: reuse of its editor
cannot silently make private patrol progress a new physical-permission owner.
The open work is the source/binding contract and that reusable lifecycle tool,
not another choice between data definitions and per-behavior Go builders.
