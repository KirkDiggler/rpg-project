# Thoughtful runtime provider — bounded implementation contract

Goal: reuse the agreed 50-bone donor on the existing human NPC appearances,
without replacing their bodies, style/skin palettes, idle/walk or downed assets.
Authority: design R1/R2/R3/R6/R8; this is the first Thoughtful slice, not table-row
activity commands or publication of the other archived stances.

## Measured seam

The archived Blender exports and current runtime bodies name the same 50 joints
and parents, but their joint coordinate bases differ. Raw clip-name binding is
not sufficient. The provider must bake into the existing runtime basis; the web
must not retarget bones or guess compatibility. Body bytes remain unchanged.

## Unit F1 — provider-side animation library cooker

Owner: private assets worktree npc-idle-lab. New scripts/npc_activity_library.py
and test_npc_activity_library.py. Read existing bounded GLB parser. Produce a
separate GLB with only named Thoughtful Enter/Sustain/Exit animations, original
runtime skeleton nodes/skin, and referenced animation/inverse-bind buffers. No
meshes, images, materials or unused original buffer bytes. Do not edit donors,
body GLBs or the open Blender scene.

For corresponding joints, calculate the constant basis conversion
`C_joint = inverse(sourceRestWorld) * targetRestWorld`. Bake each local sample
as `inverse(C_parent) * sourceAnimatedLocal * C_joint`. Reject different names,
parents or rest joint positions, nonfinite values, unsupported interpolation,
unequal channel timelines, shear and unknown channels. Preserve quaternion
continuity and exact sample times. This is offline coordinate conversion of the
approved motion, not a new runtime animation/rules system.

Tests: synthetic rotated-basis example preserves skin deformation rather than
raw quaternion values; incompatible topology/positions refuse; real archived
Thoughtful clips preserve sampled skin deformation after float32 encoding;
output has no geometry/textures and only required buffers. Real binding proof
covers the existing 50-bone bodies before publication. A browser comparison
remains required: numeric equality alone is not the visual acceptance gate.

## Unit F2 — explicit provider profile and consumer catalog

After F1 proof, publish one private animation library and a profile under
harness/models/synty/npcs/activities/. Profile names opaque activity references,
labels, Enter/Sustain/Exit clips, pose compatibility and sustain exit boundaries;
explicit relaxed default and walking activity. Relaxed/walk clips remain on the
existing body. Only Thoughtful is newly selectable in this release. Bind supported
appearances to exact unchanged standing hashes and verified rig facts, including
all existing style/skin variants of each admitted family. Do not infer support
from bone count alone.

Consumer generation validates the profile and hash-bound library/body files and
emits metadata only. Sync carries licensed files into ignored runtime paths;
never stage them in the public web repository. The existing appearance catalog
and original model URLs remain authoritative. Unknown explicit activities are
named diagnostics, not silent fallback. Concrete generator and sync edits follow
inspection of their existing pinned-provider flow; no guessed publication.

## Remaining connected unit

Studio choices consume the provider profile and preserve draft/undo/save/load.
The body adapter consumes the observed default, library and the existing
completion-driven movement seam. It waits for exit before translation, returns
to the default after arrival, and applies shared presentation speed. Initial
preview, unseen/remembered/downed handling and failure release must be explicit.
The named walkthrough remains the Pacing Yard; no provider-only proof claims it
is already implemented.
