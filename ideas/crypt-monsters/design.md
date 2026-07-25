# Crypt Monsters

**Date:** 2026-07-25
**Status:** Durable Phase 1 animation design
**Tracker:** [rpg-project#128](https://github.com/KirkDiggler/rpg-project/issues/128)
**Design review:** [rpg-project#129](https://github.com/KirkDiggler/rpg-project/pull/129)

## Goal

Create a repeatable private asset-pipeline workflow for the standing crypt
monster roster. Phase 1 gives every in-scope standing model exactly two clips:
`Idle_Relaxed` and an in-place `Walk_Forward`. It preserves each model's native
geometry and weapons unchanged.

Existing static downed siblings remain available as authored. They receive no
new animation work in Phase 1.

## Phase Boundaries

### Phase 1: Locomotion

- Retarget `Idle_Relaxed` and in-place `Walk_Forward` for every in-scope
  standing crypt model.
- Preserve native model geometry, materials, embedded or separate weapons, and
  their authored placement.
- Obtain visual approval for each source result before baking or batch work.
- Keep all source and converted licensed assets private.

### Later Phases

- Phase 2 is weapon placement and socket work. It may change how weapons are
  attached only after Phase 1 is complete.
- One attack animation is a possible later phase after weapon placement. It is
  explicitly not current scope.
- Downed animation, runtime resolver or web wiring, renderer work, and release
  rollout are not part of this design.

## Roster And Runtime Contracts

The in-scope standing roster is `Skeleton_Soldier_01`,
`Skeleton_Soldier_02`, `Skeleton_Slave`, `Skeleton_Knight`, `Ghost_01`,
`Ghost_02`, and `Tormented_Soul`. Their existing static downed siblings stay
in the roster without new clips.

This design changes asset preparation only. It does not change the runtime
resolver, web wiring, renderer behavior, model identifiers, or consumer
contracts. [rpg-game-assets#28](https://github.com/KirkDiggler/rpg-game-assets/pull/28)
and [rpg-dnd5e-web#594](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/594)
remain paused for later reuse and must not be rebuilt, merged around, or
advanced by this work.

Ghost material behavior remains subject to the explicit material review in the
batch proof order below; this animation design does not substitute materials or
alter their authored treatment. Approved outputs remain private in the normal
asset pipeline. No public source or converted Synty asset is committed, and no
runtime or release promotion occurs without its separately approved release
work.

## Verified Soldier01 Retarget Discovery

The Soldier01 experiment produced a clean true T-pose source retargeted to a
live relaxed result without manually posing the target rig. The approved idle
source is:

`/home/kirk/game-dev/assets/synty/animation-base-locomotion/SourceFiles/Animations/Polygon/Masculine/Idle/A_Idle_Standing_Masc.fbx`

Its SHA-256 is
`76dc31c92fd3ca69f28e92f1f530eac6f60ee71ea0f25d490a633f49d2088dd0`.
The experiment blend is retained locally with SHA-256
`9c1dab858124407968e71664cff80f60fa832ceda2ba708718b745a6d3541f7b`.

The retarget uses the exact 46 v2 mappings, with `Hips` mapped to the `Pelvis`
root, seven idle corrections, `Clavicle_R` excluded, and 47 target constraints.
Those mappings and corrections are evidence for the profile, not reusable
Soldier matrices.

Dungeon-authored arm tails are approximately 100 times too long and 78-102
degrees away from the actual child-joint directions. Built-in ARP copy-rest
therefore failed. The successful temporary direction-proxy method:

1. Sets each corrected target bone proxy tail to the mapped child
   `head_local` direction.
2. Computes the per-target offsets from that proxy.
3. Binds the real target using those offsets.

The proxy never alters the real rig or mesh. Offsets and matrices must be
computed per target: Soldier01, Soldier02, Skeleton_Slave, and the ghosts share
a rest hash; Skeleton_Knight and Tormented_Soul differ.

## Walk Retargeting

Phase 1 extends the dynamic child-direction profile to the mapped leg chains
for walking. The source must be the in-place masculine forward walk
`A_Walk_F_Masc`. A visual review must approve the target walk before it is baked
or admitted to any batch work.

## Batch Proof Order

Proceed only after Soldier01 has visually approved `Idle_Relaxed` and
`Walk_Forward`, in this order:

1. `Skeleton_Soldier_01`: complete both clips.
2. `Skeleton_Soldier_02` and `Skeleton_Slave`.
3. `Skeleton_Knight`: separate right-arm review.
4. `Ghost_01` and `Ghost_02`: material review.
5. `Tormented_Soul`: separate full-rig review.

Each result must preserve scale, floor contact, facing, root/export shape, and
native geometry and weapon state. Record profile differences and their evidence
rather than silently applying Soldier01 matrices to a different rest hash.

## Validation And Release

Blender viewport review is the approval gate for each model and clip. Verify
idle relaxation, in-place forward travel for walking, floor contact, facing,
and the unchanged native weapon/geometry presentation before baking. Record
the exact source, profile, rest-hash relationship, and any model-specific
correction with each approved output.

Exports stay private and follow the existing game-asset contract only after
their separate integration and release approvals. Licensed Synty source and
converted GLBs are never committed to a public repository. Public evidence is
limited to permitted viewport screenshots.

## Decision Log

### 2026-07-25

- Replaced the idle/downed-only discovery slice with durable Phase 1 coverage:
  two clips for every in-scope standing model.
- Kept downed siblings static and deferred weapon placement/socket work to
  Phase 2.
- Recorded the Soldier01 child-joint direction-proxy result and its root cause:
  invalid Dungeon-authored arm tails make ARP copy-rest unsuitable.
- Required dynamic, per-target offsets and rest-hash-aware profiles rather than
  reusing Soldier matrices.
- Added in-place `A_Walk_F_Masc` as the only Phase 1 walk source, with visual
  approval before baking or batching.
