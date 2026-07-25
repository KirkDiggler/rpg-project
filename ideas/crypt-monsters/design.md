# Crypt Monsters

**Date:** 2026-07-25
**Status:** Revised discovery design pending written review
**Tracker:** [rpg-project#128](https://github.com/KirkDiggler/rpg-project/issues/128)
**Design review:** [rpg-project#129](https://github.com/KirkDiggler/rpg-project/pull/129)

## Goal

Discover a repeatable workflow for processing the monster assets already
available to the project. This is a two-model experiment, not a production
rollout design.

The discovery begins with `Skeleton_Soldier_01`. Its result is useful only if
the recorded process can then be applied to `Skeleton_Soldier_02`, or if the
second model clearly documents why it cannot.

## Starting Point

The first loop uses Blender interactively. Kirk can rapidly judge viewport
screenshots, so visual judgment is the primary feedback signal rather than a
proxy gate invented in advance.

Inspect the source asset and the current candidate together. Compare only the
character conventions already known to matter:

- Scale relative to the gameplay floor.
- Feet and body contact with the floor.
- Facing and forward direction.
- Root and export shape.

Do not assume the monster has character-rig or animation parity. Rig
compatibility, animation suitability, and export behavior are questions to
learn from the specimen.

## Soldier01 Discovery Loop

1. Open the source/current candidate in Blender and inspect it against the
   known conventions.
2. Make one bounded change that addresses the most visible problem.
3. Capture gameplay-relevant viewport screenshots and show them to Kirk.
4. Use Kirk's visual judgment to keep, adjust, or discard that change.
5. Repeat only while the next bounded change is justified by the inspection or
   feedback.

The loop should produce an intentional-looking model, not merely a file that
exports or a clip that exists. It must not turn into unreviewed production
work, batch processing, or an attempt to solve every monster category.

## Unknowns To Resolve

The first loop deliberately leaves these open until Blender evidence answers
them:

- Whether the rig is compatible with existing character conventions.
- Whether an existing animation is usable.
- Whether retargeting, a direct pose, or neither is appropriate.
- How embedded or separate weapons should be handled.
- What Root and export behavior the output needs.
- Whether the Soldier01 result generalizes to Soldier02.

No presumed answer becomes a requirement before it is observed and accepted.

## Record Only What Works

The Soldier01 producer records the exact successful inputs and operations at
the level warranted by the result. This may be a concise recipe,
configuration, or script; it is not a commitment to create automation before
repetition proves it valuable.

Export the approved output privately. Licensed source assets and converted
binaries remain private. Viewport screenshots may be shared publicly when they
contain only permitted visual evidence.

A fresh independent agent applies those recorded steps to
`Skeleton_Soldier_02` and reports where they generalize or fail. Do not silently
tune the second model into a separate undocumented workflow. If it needs a
different step, record the difference and its reason. This is a learning test,
not another elaborate approval gate.

## Two-Model Retro

After the Soldier02 attempt, hold a short retro that answers:

- Which Soldier01 steps reproduced unchanged?
- Which steps were model-specific, and why?
- Is a shared recipe sufficient, or is targeted automation justified?
- Is any validation justified by a real failure or repeated check?
- Does the result support another specimen, or does it expose a boundary that
  needs a separate design?

This is the decision point for future work. There is no precommitted batch or
production plan before the two-model result is reviewed.

## First-Slice Outputs

- One visually approved private `Skeleton_Soldier_01` output.
- A concise processing record of the successful Soldier01 steps.
- A `Skeleton_Soldier_02` repeatability result, including exact reasons if it
  does not reproduce.

## First-Slice Acceptance

The discovery slice is accepted when:

- Kirk says the Soldier01 model looks intentional from gameplay-relevant
  views.
- Basic scale, facing, floor, and Root/export sanity checks pass.
- A fresh independent agent applies the recorded process to Soldier02 and
  reports exactly what reproduced or why it did not.

## Out Of Scope Until Learned

The following are explicitly deferred: roster batch processing, runtime
resolver or web wiring, downed models, ghost materials, walk clips, elaborate
validators, renderer features, and release choreography.

## Existing Work

[rpg-game-assets#28](https://github.com/KirkDiggler/rpg-game-assets/pull/28)
and [rpg-dnd5e-web#594](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/594)
remain paused. Preserve them for later reuse; do not rebuild, merge around, or
otherwise advance them based on this discovery result.

## Next Decision

After written review, begin the small Blender discovery loop directly; no
`plan.md` is a prerequisite. Create a later plan only if the two-model retro
establishes a justified production workflow and scope.
