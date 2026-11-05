# Behavior bindings — implementation planning preflight

**Goal:** Turn the accepted [R1–R2 contract](design.md) and
[two-guard example](README.md) into bounded, executable tasks without hiding a
new lifecycle mechanism inside a test fixture or per-behavior builder.

**Authority:** [Proposal acceptance and R3 discussion](https://github.com/KirkDiggler/rpg-project/pull/868#issuecomment-6669172634),
on [story #859](https://github.com/KirkDiggler/rpg-project/issues/859).

**Readiness:** This is a preflight, **not a checked implementation plan**. R3
changes a public authoring/runtime seam. The affected task contracts cannot be
made executable by silently choosing its answer. There is no additional blanket
plan-approval gate once the remaining contract is settled.

## Agreed outcome and finish line

The definition is authored data; bindings provide different route values and
local attachment identity; each attachment retains private continuation. West
has two waypoints and court three. Actual addressed completion advances west
without advancing court. Reload preserves both configurations and continuations
without rerolling or resampling.

The eventual implementation evidence must join a real production producer to
`behavior.NewComposer`, not hand-build the result in a test helper. Changed
source fields need studio load/edit/save/compile proof. The finished plan must
name these consumer checks, applicable module/PR boundaries and exact tests.
Publication and review are separate from operator-authorized merge. No deployment,
physical walk, effects extraction or completed playable journey follows from
this authoring agreement.

## Inspected baseline

| Repository | Revision | Relevant surface |
|---|---|---|
| `rpg-toolkit` | `6d8a753a4e255e38040b1b63a2bfb731832095c2` | `mind/behavior`, `mind/machine`, `rulebooks/dnd5e/encounter/dungeonspec` |
| `rpg-dnd5e-web` | `3d33799299c7f132479ba29c6f58a81a5f151976` | Bundle/state codecs and authoring controls; source inspection only |
| `rpg-project` | `6b116504b586a633431baff80554f811574180d4` | Accepted proposal as reviewed, before recording R2 |

Fresh runtime baseline, from toolkit `mind/behavior`:

```sh
go test -race -count=1 ./...
```

Pass: root **1.131s**, sound **1.022s**, stage **1.019s**; deed has no tests.
This establishes unchanged composer behavior, not a definition binder or
variable-route operation.

## What the source actually provides

| Existing symbol | Capability | Missing capability it must not be credited with |
|---|---|---|
| `dungeonspec.bundleResolver`, `substitute` in `bundles.go` | Resolve names/defaults and substitute whole parameter nodes into row groups | Generate a stateful sequence over the elements of a supplied list |
| `dungeonspec.recipeOf` in `states.go` | Translate explicitly authored states/transitions into `machine.Recipe` | Instantiate a repeated lifecycle body for an arbitrary route length |
| `machine.Recipe`, `SetAct` in `recipe.go`; `Program.apply` in `step.go` | Explicit finite states, named picks, literal bag writes | Increment a list cursor or resolve a table/target by a cursor expression |
| `behavior.Selector` in `table.go` | Carry fixed `At` or `Path` data | Retain which path leg is unfinished; `Path` explicitly leaves progress to the position-derived caller |
| `behavior.NewComposer`, `Observe`, `Decide`, `LoadComposer` | Consume resolved configuration, own progress, select once, reload against original definition identity | Interpret source templates or execute movement |
| `behavior.CheckParameters` in `parameters.go` | Validate declared bool/int/string values without coercion/defaulting | Describe or validate a structured route |

## R3: the affected public contract

The new tool must accept a **data-authored repeated body**, not a special behavior
name whose Go handler supplies the real logic.

| Approach | Input → output | Runtime consequence | Cost |
|---|---|---|---|
| A — finite expansion during binding, recommended | Finite supplied list + authored body/next relationships → ordinary recipe/table data per binding | Existing machine/composer contract remains; generated states hold progress | Materialized graph/table size grows with the list; producer needs stable generated identity, source mapping and refusals |
| B — reusable runtime sequence/cursor | Authored body + bound list → runtime configuration with private cursor/current-item operations | New progression, lookup, composition and reload contracts | A new runtime component and its consumer protocol must be designed and tested |

The rule does not require geometric knowledge in either generic mechanism. The
value's owner validates geometry; the author decides which action data consumes
the item. Definition/source edits still do not replace a started run's resolved
configuration. No expansion or cursor choice grants movement permission.

The recommendation is A: the current producer boundary already supplies fully
resolved `InstanceSpec{ID, Recipe, Table}`. Reusing the old stateless path or
putting the missing loop in `patrolComposition` test content would fail R2 rather
than implement either approach.

## Requirement and seam preflight

These are the requirements to assign to concrete tasks after R3, not completed
plan coverage checks or executed new tests.

| Requirement | Existing support | Required joined proof |
|---|---|---|
| A1 — one editable definition | Bundle/state source precedents; R1 | Decode one source definition; bind it twice without private copies of its logic |
| A2 — two and three route items | Structured row parameter substitution | Real selected targets follow the respective finite sequences; the same authored body handles both lengths |
| A3 — independent addressed progress | `ObserveInput.Instance`, private blobs | Complete west's first leg; west next requests west-B while court still requests court-A; no-budget selection changes neither progress nor dice |
| A4 — immutable configuration and reload | `ComposerData`, `LoadComposerInput.Config` | JSON reload preserves both continuations; changed configuration is refused rather than silently rebound; load performs no selection |
| A5 — common producer contract | Existing source-preserving bundle codec | Studio load/edit/save/compile preserves definition references, supplied values and generated content as editable source |
| A6 — no hidden Patrol coordinator | Explicit materialized runtime input | Another authored repeated body uses the same mechanism; no branch on the definition's name and no test-only recipe expander |

| Provider → consumer | Established contract | Unsettled part / dependency |
|---|---|---|
| Definition/binding source → producer | Source remains separate from resolved configuration | New source schema, structured metadata and R3's authored operation |
| Producer → composer | Existing `InstanceSpec` and `ComposerConfig.Instances` | Concrete binder API, vocabulary validation and source-to-materialized diagnostics |
| Composer → caller → observation | `DecideOutput.Instance`, selected `Pick`, addressed `ObserveInput` | Completion vocabulary and per-item correlation for the concrete consumer |
| Composer/configuration → persistence owner | Original config + `ComposerData` | Exact consumer storage/adaptation; no replacement of identity checking |
| Source compiler ↔ studio | One source contract, client does not execute game rules | Codec/editor paths and cross-language round-trip fixture after the source schema is fixed |

## Resume point

Resolve [R3 on the design PR](https://github.com/KirkDiggler/rpg-project/pull/868#issuecomment-6669172634).
Then finish the concrete source/binding/completion contracts, derive the task and
release boundaries, create the slices named by that plan, and publish requirement
coverage plus provider/consumer seam checks before implementation. Do not present
this preflight as a plan a fresh builder can already execute.
