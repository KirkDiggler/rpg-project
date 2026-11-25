# Accepting the ownership decision, not freezing the example

Written: 2026-11-05. Work: [Keep equipment S1 — #862](https://github.com/KirkDiggler/rpg-project/issues/862).
Previous: [From a test probe to an item composer](2026-11-05-from-probe-to-item-composer.md).

## Question

Is there enough evidence to accept ADR-0048, and what would acceptance actually bind?

## Evidence

After inspecting the package, the operator asked for a judgment on ADR status.
The recommendation was to accept the bounded composition/ownership decision while
leaving the exact API, ongoing-effect workflows and extraction/integration choices
open. The operator agreed on 2026-11-05.

[ADR-0048's Decision](https://github.com/KirkDiggler/rpg-toolkit/blob/docs/second-wind-ring/docs/adr/0048-equipment-granted-abilities.md#decision)
now records four accepted boundaries: the item is a composer; item-specific pools
belong to its instance; grants are inspectable data with rulebook-owned meaning;
configuration, live state, saved state and views have distinct responsibilities.
The existing example/test evidence is linked, not expanded into an unrun claim.

## Interpretation

Accepted means implementations follow those responsibilities. It does not mean
the code is extracted, every implementation task is complete, or the example's
names and generic resolver protocol are permanent.

The earlier ADR mixed architectural acceptance with checks for production
adoption. Those broader checks remain explicitly required for the work they
cover; they are not marked passed or waived. The prototype surface is now under
an explicitly non-normative evidence section. Existing journal entries retain
their earlier Proposed status as history.

No runtime code, item rules or PR merges are changed by this acceptance record.

## Next

Use the accepted boundaries to guide further examples. A contrasting ongoing
effect, such as a Rage-granting item, is the recommended next experiment before
extraction, to test grant, charge and resulting-effect lifetimes separately.
Its detailed gameplay and implementation are not selected by this status change.
