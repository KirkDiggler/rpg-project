# 2026-11-07 — Which saved data does the next Rage implementation need?

Work: [story #703](https://github.com/KirkDiggler/rpg-project/issues/703), [rulebook design #2295](https://github.com/KirkDiggler/rpg-toolkit/pull/2295).

## Question

The [released generic runtime](2026-11-07-from-pushed-code-to-released-adoption.md)
is available. The next cut brings the actual Barbarian feature and item consumer
to one Raging creator. Does the old saved shape constrain that design?

## Evidence

KirkDiggler selected the Raging-only vertical cut and clarified that existing
characters can be recreated. The saved shape need not remain the same, and no
historical Rage reader, writer or normalization pass is required. [R1 and R2](https://github.com/KirkDiggler/rpg-project/issues/703#issuecomment-6700593288)
record those decisions; no character data was reset by the investigation.

The existing native feature constructs the handwritten condition directly. The
example instead owns its recipe and captures item provenance. The shared creator
must replace both paths, not merely acquire another name. Current baseline
conditions/features tests pass. The draft design identifies complete-output
validation and nonterminal state-change notification as concrete provider gaps;
the first Rage turn must reach the host's dirty-state mechanism.

A separate [web reader finding](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1396)
records the stale requirement for saved damage_bonus. That is source/guard-probe
evidence, not an observed browser result or a reason to restore cached class data.

## Interpretation

My initial recommendation to read legacy blobs and normalize on save carried a
compatibility obligation the operator did not need. Removing it lets the proposed
record distinguish immutable origin/bindings from mutable lifecycle state, while
retaining one authoritative instance and the same gameplay.

That concrete representation and its query/validation interfaces are presented
together in the design proposal, not attributed as separately approved operator
choices. No product code or completed migration is claimed.

## Next

Use the complete [walkthrough and scoped design](https://github.com/KirkDiggler/rpg-toolkit/pull/2295)
as the agreement surface, then derive the implementation plan and its real consumer
checks without another planning-approval gate. Only Raging is being migrated;
other conditions, studio authoring and native item inventory remain outside this cut.

— cross-team agent, on behalf of KirkDiggler
