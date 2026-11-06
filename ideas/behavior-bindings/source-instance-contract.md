# Source → instance boundary

This is the source specification section for [consumer plan #877](https://github.com/KirkDiggler/rpg-project/pull/877),
accepted as [R8](consumer-design.md#rulings). It defines identity, value ownership
and the producer handoff. It does **not** settle selection-lane policy, legacy execution
parity or a general item-schema language, and adds no runtime implementation.

## Four different identities

| Identity | Scope / owner | Example | Meaning |
|---|---|---|---|
| Source definition key | One authored dungeon document | `patrol` | The name a source binding resolves locally |
| Definition ref | The reusable definition | `example:behaviors:patrol` | Identity carried by the resulting recipe; not a particular guard |
| Actor/member identity | Encounter placement | West or Court's compiled member ID | The owner of the live composition and its saved data |
| Attachment ID | One actor's composition | `duty` | Which independently progressing instance receives a result/completion |

A local key is not automatically the definition ref. Two actors may both bind
`patrol` as attachment `duty`; they still own different configuration values and
continuations. Within one actor, attachment IDs must be nonempty and unique,
even when two bindings use different definitions.

Definition identity alone is insufficient for reload. The existing composer
also checks the deterministic identity of the **materialized InstanceSpec**;
changing a route or supplied parameter can make that instance incompatible even
when its reusable definition ref is unchanged.

## Source forms and binding values

The candidate field names remain those in the consumer plan:

- `behaviorDefinitions` holds site-local reusable definition data.
- A creature/faction's ordered `behaviors` list holds bindings, not expanded
  state/table copies. A binding's `id` names the attachment and `use` resolves a
  definition key in that document.
- A sequence binding supplies an explicit finite `items` collection and named
  scalar `with` values. These map to the existing `ExpandSequenceInput.Items`
  and `.Parameters`; they are not mutable runtime bags.
- A concrete non-sequence definition supplies reusable recipe/table data. Its
  attachment ID comes from the binding, not from a shared instance ID inside the
  definition. Unsupported `items`/`with` on this form are refused, not ignored.
- `creature: true` is the explicit shorthand-import arm already described in the
  plan. It is not a third behavior factory selected by an NPC name. Its legacy
  execution normalization is a separate contract, not assumed resolved here.

Source references and author-supplied values remain source. Saving/exporting an
edited dungeon does not replace a `use` with generated states, serialize private
progress into it or spread one guard's route into the shared definition.

An absent binding list, an explicitly empty list and a nonempty list remain
representably distinct. This boundary preserves that distinction; the separate
normalization contract determines the existing simple-source case. It does not
silently erase an explicit empty list while encoding.

## The producer handoff

For a sequence binding, the existing production API is the handoff target:

```text
source lookup:
  behaviorDefinitions[use] → one SequenceDefinition

binding/domain adapter:
  id                     → ExpandSequenceInput.ID
  items                  → chosen, domain-validated canonical JSON items
  with                   → typed named scalar values
  owning input vocabulary → Vocabulary

ExpandSequence:
  → InstanceSpec
  → completion mapping
  → state/table source mappings
  → vocabulary with generated completion beats
```

`ExpandSequence` does not resolve `use`, choose a default, discover a route,
convert coordinates, inspect the world or instantiate live progress. Its inputs
are already chosen. Named scalar values are checked through the shipped
`ParameterSpec`/`CheckParameters` rules; false, zero and empty string remain
supplied values rather than absence.

For `cells` items, the dungeon adapter uses the existing `RoomCell{Q,R}` and
`roomCells.cellAt`/`hexCell` owner. Validate the authored cell and lower it once
to canonical component data. Do not infer geometry from a stringified parameter
or treat the component's opaque JSON item as proof that the cell is valid in the
site. Literal emitted spatial selectors also need their owner's validation.

Existing row-bundle `params`/`use`/`with` semantics remain unchanged. Reuse their
source-preservation and value-resolution mechanisms where the contract matches;
do not reinterpret a weighted bundle group as an independently progressing role.
No new implicit defaults are introduced by this handoff. Any explicitly supported
source defaults must be resolved at their existing producer boundary before
calling the component checker/expander.

## Concrete two-binding trace

This is a binding fragment, not a complete launchable dungeon or a newly
implemented parser:

```yaml
west:
  behaviors:
    - id: duty
      use: patrol
      items: [{q: 0, r: 0}, {q: 12, r: 0}]
      with: {weight: 1}
court:
  behaviors:
    - id: duty
      use: patrol
      items: [{q: 20, r: 0}, {q: 23, r: 0}, {q: 26, r: 0}]
      with: {weight: 1}
```

Both lookups resolve the same source definition and ref. The site adapter gives
West two canonical items and Court three. Each producer result has local ID
`duty`, but different materialized data and independent runtime ownership.
Generated state/table names are private to each resulting instance. The caller
uses the returned source maps for diagnostics rather than parsing generated
strings to recover author intent.

The binding values do not say which leg is pending. That becomes private
`ComposerData` only after the owning runtime constructs and advances an instance.
A saved run retains its original materialized configuration; editing the source
fragment later affects subsequent binding, not that saved run silently.

## Independent concerns stay independent

The same actor may also attach a `needs` definition. `duty` and `needs` have
separate machine state, bags, wake/anchor data and definition identity. A source
compiler must not generate an actor-wide cross-product such as
`walking-and-hungry-and-alert` merely to attach another concern.

This boundary carries the instances and their authored order. It does not decide
whether a need is a data-only update, requests an action opportunity, or belongs
to a different resource/selection lane. The [lane contract](selection-lane-contract.md)
addresses that separate boundary. In particular,
source list order must not be treated as permission to stop unrelated data-only
progress, nor as a claim that all concerns compete for movement.

## Refusals at this boundary

| Input | Required result |
|---|---|
| `use` names no definition in the document | Error at that binding's `use`; no guessed/global lookup |
| Empty or duplicate attachment ID within an actor | Error at the binding ID; no rename/deduplication |
| Same ID on different actors | Valid; distinct instance ownership |
| Mixed binding arms (`use` and `creature`) | Named structural refusal; no precedence by field order |
| Required scalar absent or wrong-kind/out-of-bounds | Existing parameter refusal attached to the source path |
| Empty/non-finite/malformed sequence input | Refusal before live construction; no fabricated first state |
| Cell not valid for the site/item domain | Owner's source-coordinate refusal before expansion is accepted |
| Concrete definition contains a shared attachment ID | Refuse ambiguity rather than overwrite an author-owned identity silently |
| Malformed reference, recipe or row after binding | Component refusal mapped to source binding/item/local name |
| Definition/binding contains instance `progress` or `blob` overrides | Refuse; this is distinct from the recipe's legitimate declared initial state/bag |

A failure returns no accepted partial binding set and does not rewrite the input
document. A successfully materialized result owns detached values; changing one
binding or result must not mutate the shared definition or another actor.

## Named acceptance assertions for the implementation plan

These are required future tests, **not tests executed by this documentation**:

- `TestBindingsPreserveDefinitionAndAttachmentIdentity`: one definition, two
  actors both using `duty`, different item counts; same reusable ref, distinct
  materialized configuration, no shared progress.
- `TestDuplicateAttachmentIsLocalToActor`: reject duplicates within West while
  accepting the same local name on Court.
- `TestSourceRoundTripKeepsUsesAndValues`: decode/encode preserves `use`, order,
  items/with and explicit absence; no generated recipe/blob appears in source.
- `TestCellItemsAreLoweredOnce`: source `{q:12,r:0}` becomes canonical
  `{"x":12,"y":0}` through the existing owner; invalid source cells are refused.
- `TestBindingsCannotAliasDefinitionOrEachOther`: mutate one detached output;
  source and the other binding remain unchanged.
- `TestBindingRefusalsNameTheOriginalSource`: malformed definition/parameter/item
  errors identify actor, binding, parameter/item and local source field.

The [selection-lane contract](selection-lane-contract.md) and exact legacy
normalization/parity cases remain separate. This section makes the producer
boundary concrete without claiming the whole consumer plan is already executable.
