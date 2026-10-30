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

Provider implementation: `scripts/npc_activity_profile.py` accepts a runtime
root, cooked library and recipe. The recipe owns profile id, reference body,
exact appearance family refs, default/walk refs, activity descriptors and pose
bridges. Output schema v1 contains those descriptors, library path/hash/clip
inventory and admitted appearance refs/standing hashes. Re-read actual body
bytes, clip inventories and named-joint parents/rest-world/inverse-bind matrices
before admission; a stale manifest hash or incompatible rig refuses. Output is
a new candidate directory, never an overwrite of a body or existing profile.
Tests cover the real complete style/skin family selection, tampered body hashes,
incompatible library binding and missing clips. Metadata is not a new activity
command protocol.

Consumer generation validates the profile and hash-bound library/body files and
emits metadata only. Sync carries licensed files into ignored runtime paths;
never stage them in the public web repository. The existing appearance catalog
and original model URLs remain authoritative. Unknown explicit activities are
named diagnostics, not silent fallback. Concrete generator and sync edits follow
inspection of their existing pinned-provider flow; no guessed publication.

## Unit G1 — provider-backed Studio selection

Web owns `src/npc-activities/activityProfile.ts`: exact appearance lookup in the
generated profiles, with no inference from rig count or appearance family names.
`StudioArrangeFields.tsx` offers nonmoving provider activities, an absent/default
choice and a named unavailable choice for imported unsupported values. Changing
appearance must not clear or substitute a default. Extend the existing
`StudioActorArrangeEdit` and `applyStudioActorArrange` to use the existing pure
setter; omitted stays untouched, null clears. The normal owner history supplies
undo/redo, not a second settings store.

Tests: roomDefaultActivity exercises provider choices, simultaneous location/
default edit and preservation of unknown defaults after a look change. The real
StudioArrangePanel owner test selects Thoughtful and clears it, verifies save
and undo/redo and unchanged appearance/rules/faction. No hard-coded option list.

## Unit G2 — production sequencing and movement readiness

`src/npc-activities/activitySequencer.ts` owns only presentation phases. Input:
provider profile, play callback and state callback. Reset starts the observed
default's sustain (no imagined prior pose); request retains only the latest
activity. Real mixer loop/finished events, fenced by serial, advance phases.
Stop invalidates callbacks; reappearance resets rather than replaying old work.
Walking readiness means sustain of the profile's explicit walking activity.
Tests pin completed Enter/Exit, sustain boundaries, latest-wins, exact readiness,
stop/reset and stale events. No timers substitute for clip completion.

The body adapter loads the separate library and combines named clips with the
unchanged body's clips. The existing movement hook remains the only position
writer; a scoped readiness callback gates its frame advancement without accruing
elapsed time while blocked. Route completion remains the existing cursor's
completion signal. Mixer and route share the same overall rate; Enter/Exit use
an additional 1.5 multiplier. Unsupported refs/load failures must report a named
diagnostic and settle/release presentation, never wedge the story cursor.
Actual mixer/movement and lifecycle integration tests precede the game walk.

## Remaining connected unit

Studio choices consume the provider profile and preserve draft/undo/save/load.
The body adapter consumes the observed default, library and the existing
completion-driven movement seam. It waits for exit before translation, returns
to the default after arrival, and applies shared presentation speed. Initial
preview, unseen/remembered/downed handling and failure release must be explicit.
The named walkthrough remains the Pacing Yard; no provider-only proof claims it
is already implemented.
