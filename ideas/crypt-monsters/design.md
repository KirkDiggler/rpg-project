# Crypt Monsters

**Date:** 2026-07-25
**Status:** Approved Phase 1 animation, runtime, and release design
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

### Phase 1: Standing Contract And Tomb Runtime Slice

- Every in-scope standing model exports exactly the ordered two-clip contract:
  `Idle_Relaxed`, then in-place `Walk_Forward`.
- Preserve native model geometry, materials, embedded or separate weapons, and
  their authored placement.
- Keep static downed siblings unchanged and byte-identical.
- Ship the tomb runtime slice only for the two references used by
  `/home/kirk/game-dev/dungeon-content/reference-tomb.yaml`:
  `dnd5e:monsters:skeleton` and `dnd5e:monsters:skeleton-captain`.
- Keep all source and converted licensed assets private.

### Later Phases

- Weapon placement and socket work remain deferred. They may change how weapons
  are attached only after Phase 1.
- Attack clips remain deferred until after weapon placement.
- Phase 2 is a separate coverage initiative: inventory all 15 current toolkit
  monster references, deliberately map each reference or record its asset gap,
  and add a coverage regression test. The other 13 references do not block the
  tomb delivery.

## Roster And Runtime Contracts

The seven approved standing models are `Skeleton_Soldier_01`,
`Skeleton_Soldier_02`, `Skeleton_Slave`, `Skeleton_Knight`, `Ghost_01`,
`Ghost_02`, and `Tormented_Soul`. All seven checkpoints have structural and
visual approval; correct runtime GLBs still need export. Their existing static
downed siblings stay in the roster without new clips.

The asset roster is established by
[rpg-game-assets#28](https://github.com/KirkDiggler/rpg-game-assets/pull/28),
merged 2026-07-25. The pending private asset PR exports all seven approved
standing models through the existing exporter, updates the NPC manifest and mesh
stats, and leaves each downed file byte-identical. Re-import validation must
confirm the `Root` wrapper, one source mesh, `SyntyAtlas`, geometry and weights,
the two ordered clips and their ranges, and finite bounds.

[rpg-dnd5e-web#594](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/594) remains open and proceeds only after that private asset PR merges. Its Phase 1 runtime scope is deliberately narrow:

- `dnd5e:monsters:skeleton` uses the Soldier candidate table, deterministically
  selecting `Skeleton_Soldier_01` for now.
- `dnd5e:monsters:skeleton-captain` uses `Skeleton_Knight`.
- `Skeleton_Slave`, `Ghost_01`, `Ghost_02`, and `Tormented_Soul` are
  asset-ready but not client-selectable. Stable variant selection is not part
  of this phase.

The verified proof order below includes the ghost material review; this design
does not substitute materials or alter their authored treatment. Approved
outputs remain private in the normal asset pipeline. No public source or
converted Synty asset is committed, and runtime or release promotion requires
all applicable gates in this design.

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
for walking. The source is the in-place masculine forward walk `A_Walk_F_Masc`.
The target walk received visual approval before baking and batch work.

## Verified Retarget Proof Order

The following checkpoint order produced the approved structural and visual
results. It remains the evidence history for the seven-model contract:

1. `Skeleton_Soldier_01`: complete both clips.
2. `Skeleton_Soldier_02` and `Skeleton_Slave`.
3. `Skeleton_Knight`: separate right-arm review.
4. `Ghost_01` and `Ghost_02`: material review.
5. `Tormented_Soul`: separate full-rig review.

Each result preserves scale, floor contact, facing, root/export shape, and
native geometry and weapon state. The profile differences and their evidence
must remain recorded rather than silently reusing Soldier01 matrices for a
different rest hash.

## Cross-Repo Sequence

1. Create and merge the private asset PR after exporting and validating all
   seven standing runtime GLBs, updating the NPC manifest and mesh stats, and
   proving unchanged downed-file hashes.
2. Advance web PR #594 only from that merged asset main. Run local asset sync,
   unit tests, build, typecheck, and browser checks for the reference tomb's
   standing, moving, and dead behavior.
3. Merge #594 to web `main`. Its normal merge automation builds the Docker
   image, clones current private asset `main`, pushes the image, and dispatches
   production. The asset merge alone does not deploy.

Do not use an empty commit or manual dispatch. Use them only if the normal web
merge run fails and the failure has been diagnosed.

## Validation And Release Gates

Blender viewport review is the approval gate for each model and clip. Verify
idle relaxation, in-place forward travel for walking, floor contact, facing,
and the unchanged native weapon/geometry presentation before baking. Record
the exact source, profile, rest-hash relationship, and any model-specific
correction with each approved output.

The asset PR cannot merge if a stale hash, missing path, missing or malformed
clip, T-pose, deformation failure, invalid export structure, or workflow
failure is present. The web PR cannot proceed until the asset PR is merged, and
the release cannot proceed if asset sync, unit tests, build, typecheck, or the
reference-tomb browser checks fail.

Licensed Synty source and converted GLBs are never committed to a public
repository. Public evidence is limited to permitted viewport screenshots.

## Decision Log

### 2026-07-25

- Replaced the idle/downed-only discovery slice with durable Phase 1 coverage:
  two clips for every in-scope standing model.
- Kept downed siblings static and deferred weapon placement/socket work beyond
  the Phase 2 coverage initiative.
- Recorded the Soldier01 child-joint direction-proxy result and its root cause:
  invalid Dungeon-authored arm tails make ARP copy-rest unsuitable.
- Required dynamic, per-target offsets and rest-hash-aware profiles rather than
  reusing Soldier matrices.
- Added in-place `A_Walk_F_Masc` as the only Phase 1 walk source, with visual
  approval before baking or batching.
- Approved the exact ordered two-clip standing contract for all seven standing
  models and recorded that all seven visual and structural checkpoints passed;
  runtime exports remain the next asset gate.
- Established the private asset PR as the prerequisite for web PR #594, which
  maps only skeleton and skeleton-captain for the reference tomb in Phase 1.
- Defined the normal release path from merging #594 to web `main`; an asset
  merge alone does not deploy, and manual dispatch or empty commits are only a
  diagnosed normal-run recovery.
- Scoped Phase 2 as deliberate coverage for all 15 toolkit monster references
  without blocking the other 13 on the tomb delivery; weapon placement and
  attacks remain deferred.
