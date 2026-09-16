# Direct material editing and ingestion handoff

Status: approved by Kirk at amendment commit `9901ec0`; implementation authorized.
Execution: GLM Flash implementation without elevated thinking, followed by bounded
Luna xhigh review; parent retains decisions, acceptance and publication authority.
Supplements [design.md](design.md) and supersedes
its restricted prepared-option menu and proposed unresolved-only editing rule.

## Correction and goal

The shipped preview answered "which inferred option?", but Kirk needs "let me set
the material, see it, and try again." Directory/name matching became an unwanted
restriction. Successful conversion was also not an onward ingestion path.

Reference experiment: on `SM_Bld_Wall_L_Curve_01.fbx`, Kirk used
`SmoothStone_Dark_Texture_01.png` for base color while retaining a brick normal,
and prefers `Brick_Medium_Normals_01.png`. The vendor calls both slots
`Brick_Large_01`; that label must not dictate which images he can select. Its
128 source uses are a proposed grouping, not128 visually approved assets.

**Edit → immediate preview → save profile → generate/validate selected GLBs →
normal asset review → Ready-only release.** Difficult assets remain deferred.

## First editor scope

In the existing Lab, select an asset surface/material slot or a displayed group.
Edit these channels independently:

- Base-color image, or none (neutral solid base).
- Normal image, or none, plus normal strength.
- Emissive image, or none, plus emissive color and strength.
- Explicit repeat/clamp sampling, retaining the existing setting rather than
  changing it because a filename resembles another material.

Browse the registered pack's image inventory by its real folders, with search and
thumbnails. Do not force color/normal pairing, reject cross-family image choices,
or require a detector to decide that a surface needs editing. Suggestions may be
shown but are optional. Editing an already-working material is allowed too.

Changes update the preview without rerunning the batch command for each trial.
A wrong-looking artistic combination is allowed. Missing/unreadable files,
non-finite settings and invalid target identities are technical errors with clear
messages. Loading errors must not masquerade as white materials or successful
previews. Saving a draft does not imply approval.

This is a supported static PBR editor, not a Blender node-graph editor. Animated
FX, arbitrary shader nodes and automatic import of edited `.blend` files are not
included. Blender remains useful for learning and investigating exceptions.

## Slot identity and scope

Original imported FBX objects, slots, face assignments and UVs are authoritative
for targeting. Vendor declarations provide labels/group suggestions, not a veto
on a deliberate edit. Unmatched declarations cannot fabricate a source target.

Prepare slot-addressable preview geometry with a distinct, stable identity for
each editable material slot. Do not edit the old flattened neutral review GLBs.
Live edits target only the selected slots; unrelated surfaces keep their current
appearance. Show the exact affected objects/slots/assets before applying a group.
The supplied curved wall must be selectable as an example, not omitted because
a deterministic sampler picked other assets.

A reusable material definition owns channel images/settings. The profile records
human-selected scope assignments to those definitions; script-generated inventory
and resolved recipes own membership, source fingerprints and expanded bindings.
Users do not maintain per-object mapping tables or copy hashes. Explicit slot
choices override a group default; the UI must show that override and offer a
return-to-group action. Group overlap with conflicting defaults is an error, not
an order-dependent winner. Changed source membership/identity is disclosed before
applying an old decision to a new input snapshot.

The existing profile remains the one editable choice document. New channel/scope
choices require a versioned schema; legacy selections remain readable and are not
silently reinterpreted. Import/export preserves every channel, assignment and
explicit None. No `trusted` switch is added.

## Preview is not export proof

Web applies the supported recipe to slot-preserving preview geometry for fast
experimentation. Assets compiles the same channel/settings contract against the
original source for the final GLB. Correct color spaces, normal strength, emissive
parameters and sampling must agree between both implementations. Preserve source
image quality; preview derivatives must not silently become lower-quality exports.

The final generated GLB is loaded and reviewed again in the game renderer. A live
browser material edit or a successful Blender render does not prove exported
appearance, and neither can mark an asset Ready by itself.

## Complete the onward path in this delivery

After explicit approval of a selected recipe/scope, the script compiles the
mechanical material authority update and prepares the affected source outputs.
Use existing worktree/PR/human-merge and verified cache-transition mechanisms;
add the required recipe/source binding rather than spoofing old converter receipts
or asking Kirk to hand-edit expanded pack configuration.

Prepare a validated cache generation, offer explicit activation, and open normal
asset review for the usable selected subset. Reuse unchanged verified outputs;
preserve old cache generations, review decisions and published GLB bytes. If the
selected application would change a published appearance, identify that conflict
and require an explicit revision decision or a narrower application scope. Do not
silently replace it or pretend it received the new material.

Normal calibration, successful final-model loading, Keep/Ready decisions and
Ready-only provider export remain required. Unresolved assets outside the chosen
application scope must not block the usable subset. Existing eligible-cache review
can continue while this capability is implemented.

## Acceptance before calling it ready

- Kirk can choose unrelated base-color, normal and emissive images, see the result,
  change his mind and save/reload it without any inferred-option compatibility gate.
- An actual multi-slot source proves an edit leaves other slots/UVs/geometry alone;
  group previews show their complete scope and overrides explicitly.
- The curved-wall reference and additional group representatives exercise live
  preview AND final exported GLBs, including normal/emissive settings and None.
- A small approved subset travels through generated authority, verified cache
  preparation/activation and the normal review/provider-export boundary, with
  unresolved neighbours retained and published bytes unchanged.
- Keep this work pragmatic: one implementer at a time and bounded independent
  review, not an autonomous agent campaign or per-asset repair PRs. A working
  editor alone is not completion of this delivery.
