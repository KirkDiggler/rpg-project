# Rage, retrieved loot, and an items-first extraction path

Written: 2026-11-05. Work: [Keep equipment S2 — #873](https://github.com/KirkDiggler/rpg-project/issues/873), following [S1 #862](https://github.com/KirkDiggler/rpg-project/issues/862).
Previous: [Accepting the ownership decision](2026-11-05-accepting-the-ownership-decision.md).

## Question

Does the item composition hold for an ongoing effect, and can a built item exist
outside an encounter before being retrieved and given as loot?

## Evidence

The operator requested the Rage-granting example, proportionate example review,
and a changed promotion order: items drive extraction of effects into its own
package/module; existing condition migration follows later. Items must also be
usable outside encounters. [Story #703](https://github.com/KirkDiggler/rpg-project/issues/703)
records that direction; [#705](https://github.com/KirkDiggler/rpg-project/issues/705)
marks its older combined condition-first brief as superseded pending re-planning.
The old condition parity requirements remain recorded for later adoption.

[Toolkit PR #2287](https://github.com/KirkDiggler/rpg-toolkit/pull/2287), initially at
[`6aeafe68`](https://github.com/KirkDiggler/rpg-toolkit/commit/6aeafe68bd1f7edc798c6fa8023c37341e58de60),
is stacked on #2273 and changes only the example module's consumers/tests/docs.
Generic item/effects runtime code is unchanged.

The Rage consumer prepares an unattached `effects.Composed`, then its caller
applies it. A recipe over the real machine drives grace, sustain and endings.
Actual D&D damage/incoming/save/check chains demonstrate the configured Strength
melee weapon bonus, physical resistance, and Strength save/check advantages with
item-instance provenance. Fourteen Rage suite methods, three additional host
checks, the new runnable example and the original module tests pass with race
detection. Vet/lint pass. Expiry-threshold and ignored-saved-state mutants fail
their intended checks; both are removed.

The example host stores Definition and instance Data in separate temporary JSON
files before creating a bus or holder association. Retrieval honors stored
configuration, including a non-default +3 test variation, and preserves the ID
and remaining charges. Missing/null instance data refuses instead of becoming
fresh loot. A spent item remains spent when retrieved and handed to another holder.

The verified runnable trace is:

```text
retrieved outside encounter: loot-ring-17; charges 1
prepared Rage: applied false; charges 0
raging Strength melee damage: 12
source detached; active effect remains: true
combat ended; effect active: false; charges 0
retrieved after encounter: loot-ring-17; charges 0
```

## Interpretation

The item/grant association, item budget and applied effect really are separate
lifetimes in this composition. Source detachment alone does not move/remove the
result; effect end does not restore a charge; recovery does not reactivate an
ended result. These are mechanism observations under explicit caller control,
not final equipment/attunement rules.

The preparation/delivery seam is now visible rather than hypothetical. A bus can
refuse application after Use spent the working copy's charge. The example host
therefore withholds that failed copy from persistence and reloads the original.
Successful delivery precedes saving the spent Data. This is a bounded
load–act–commit-on-success example, not magic library rollback or atomic storage
across arbitrary world objects.

No generic library needs an encounter or repository dependency to support the
artifact flow. A fresh item from a definition and retrieval of an existing item
are distinct paths; choosing the former on a missing load would create free uses.

The vocabulary uses prototype refs and real event contracts, not production
condition registration or a complete class feature. Casting/concentration/armor
eligibility, stacking enforcement, actor action economy, native loot APIs and
production persistence remain outside this example's evidence.

## Next

Use the demonstrated item consumers to re-cut #705's extraction plan, including
where the consumer vocabulary belongs and what the host commits after delivery.
Do not restore condition migration as a prerequisite or infer a shared-machine
extraction. Choose the module home and production adoption contracts from this
evidence; the example does not make those decisions automatically.
