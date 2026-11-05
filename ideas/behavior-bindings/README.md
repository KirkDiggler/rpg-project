# One definition, two guards

Working example for [story #859](https://github.com/KirkDiggler/rpg-project/issues/859)
and [the authoring direction, R1](design.md#law). This is a contract sketch, not
supported YAML, a checked implementation plan or a claim of playable integration.
The source and binding type names remain part of [the open contract](design.md#open).

## What the author changes

There is one Patrol definition to edit. It declares a route parameter and
expresses its lifecycle and table composition as data. The author changes that
shared source when changing the behavior's logic, rather than editing a separate
Go constructor or copied state graph for each guard.

Each placed guard supplies its own route. **Configuration is not progress:** the
route belongs to the binding's configuration; the unfinished leg belongs to that
attachment's runtime continuation.

```text
Shared definition: Patrol
  Declares: route
  Describes: request the pending leg; advance on actual arrival; repeat

West guard
  Attachment: duty
  Definition: Patrol
  Supplied route: west-A → west-B → repeat

Court guard
  Attachment: duty
  Definition: Patrol
  Supplied route: court-A → court-B → court-C → repeat
```

The two-versus-three waypoint example is a proposed acceptance case: it prevents
a fixed two-state fixture from standing in for a reusable route contract. The
notation describes the desired content, not a newly invented `repeat` grammar.

## Where each value goes

| Boundary | Concrete value | What the boundary contributes |
|---|---|---|
| Definition → authoring tool | One editable lifecycle/table source and parameter declarations; source schema open | Studio can edit the definition; generation can consume the same contract |
| Placement → producer | Local attachment ID `duty`, definition reference and route values; binding schema open | Associates values with this guard without copying behavior logic |
| Producer → composer | Existing `behavior.InstanceSpec{ID, Recipe, Table}`, inside `ComposerConfig.Instances` | Resolved configuration; the producer handles source references and validation |
| Composer → caller | Existing `DecideOutput.Instance` and `Pick` | Identifies the selected attachment and already-selected answer, not a completed movement |
| Caller → composer | Existing addressed `ObserveInput`; completion vocabulary open | Delivers actual completion to the owning actor/attachment |
| Composer → persistence owner | Existing `ComposerData` plus retained original configuration | Keeps continuation separate from source editing and subsequent generation |

Both guards can name their attachment `duty`: that identity is local to each
composer, not a shared bag of patrol state. Their materialized definitions may
differ because their parameter values differ.

The proposed observable trace is: both begin on their first leg; west receives
its addressed arrival and advances to west-B; court remains on court-A. Saving
and reloading both preserves those different continuations without generating
new routes or selecting another answer during load. Changing Patrol's source is
not a request to replace either saved run's configuration silently.

## What we can reuse, and what still needs a tool

The existing `dungeonspec` bundle resolver already substitutes whole route
values and preserves authored `use`/`with` on serialization. The studio already
has bundle, route and state editing surfaces. These are reuse opportunities,
not proof that a stateful attachment format already exists.

**A route value does not explain how to remember its next leg.** The materializer
must connect a data-authored lifecycle to the supplied route without concealing
that behavior in a Patrol-specific Go factory. The source form for that connection
is the central open question, not a finished feature of the scalar checker.

Likewise, `StatesSpec` is not a ready-made generic behavior definition: its
`blocks` and `holds` fields carry creature-body/post bindings. Reusing state
editing must not make a private patrol state a second authority for physical
permission. The two-guard contract does not change those existing creature fields.

## Sources to use when completing the contract

| Concern | Source |
|---|---|
| Existing source binding and preservation | `rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec/bundles.go`, `bundles_test.go` |
| Creature states and physical binding annotations | `rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec/states.go` |
| Materialized attachment, selection and persistence APIs | `rpg-toolkit/mind/behavior/composer.go`, `composer_data.go` |
| Scalar metadata and validation | `rpg-toolkit/mind/behavior/parameters.go` |
| Explicit finite recipe data | `rpg-toolkit/mind/machine/recipe.go` |
| Current retained-leg component example | `rpg-toolkit/mind/behavior/patrol_composition_test.go` |
| Existing authoring surfaces | `rpg-dnd5e-web/src/concepts/world-building/BundlesPanel.tsx`, `StatesEditor.tsx`, `creatureStates.ts` |

Inspected revisions, test receipts, alternatives and the operator's choice live
in [the case file](https://github.com/KirkDiggler/rpg-project/issues/859#issuecomment-6668089982).
