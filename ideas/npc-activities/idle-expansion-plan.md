# Six approved idles — implementation plan

Story project#797; provider assets#337; consumer/acceptance web#1373. Selection
accepted on assets#336. Kirk approved all six3048 auditions and authorized
continuing through the existing Studio default/count-response path.

## Goal, finish and exclusions

Shy, Slumped, Dramatic, Drunken sway, Grumpy arms folded and Grumpy hands on hips
work as defaults and counted replies on the existing72 supported human NPC looks.
Finish includes actual Studio save/reopen/launch, custom appearance, native phases/
count/default return, two-client cancellation/no revival, a working preview,
applicable gates and one bounded independent review. Explicit new-wave merge
permission remains required. Routine integration details/fixes are not new gates.

No combat/class behavior, conditions, new movement, props/furniture or new rig
family. The monk fighting-stance suggestion is recorded as a future idea only.
Keep3048 audition,3042 lab,3046/3047 previews, castle3030 and pacing3044 unchanged.
Use new isolated3049/8139 for this work. Licensed material stays private.

## Inspected baseline and shape

Assets main f7b12f8; web dev12bc58d3; dedicated `.worktrees/npc-idle-expansion`
for both and project. Existing cooker/profile/catalog/pickers/mixer retain their
owners and schema. The accepted55-joint checkpoint lives in the preserved
idle-discovery worktree;18 new baked clips. The canonical50-joint donor,
48-row55→50 mapping and constant0.00085065932944417m correction are already
archived in `library/animation-donors/npc-activities-v1/` (recipes.json).

Develop provider candidate -> pinned consumer -> native joined proof. Release
only after approval: private provider merge -> actual SHA re-pin/gates -> web dev.
No proto/toolkit/API changes expected or authorized merely for these opaque refs.

## T1 — preserve and convert the approved motions (assets#337)

**Owner/output:** private assets; immutable new donor archive and one50-joint
conversion, not per-look retargeting. Preserve original source/checkpoint bytes.
**Files:** new `library/animation-donors/npc-idle-expansion-v1/` (approved55 blend/
GLB, expanded50 blend/GLB, portable provenance and recipe); new
`scripts/extend_npc_idle_donor.py`; existing `retarget_lab.py`, established
legacy55-to-family50 mapping and old donor read-only.
**Interface:** background-only CLI accepts exact source/target/mapping, fresh
stage output and installed ARP module; validates hashes, no source overwrite,
no live constraints, same target rest and identical old Action digests.18 new
clip names: Idle_Shy, Idle_Slumped, Idle_Dramatic, Idle_DrunkSway,
Idle_FoldedGrumpy, Idle_HipsGrumpy, each Enter/Sustain/Exit. Original14 clips
stay untouched. New phases use one established constant grounding correction;
no frame-by-frame grounding or invented movement semantics.
**Checks:** real Blender conversion asserts old Action/rest invariants and
complete32-clip export. Compare exported old clip semantics to the archived
source before using it to cook a combined library. If export round-tripping
would alter old motion, preserve old published tracks verbatim during packing;
this is a private artifact operation, not permission to change the runtime seam.
**Proof:** hashes, portable provenance, per-clip mapping/bake records, native
source/runtime visual comparisons. No claim that55→50 retargets are identical
joint matrices across different skeletons; exact comparison applies to the
subsequent50-joint basis cook and existing published motion.

## T2 — expanded geometry-free runtime profile (assets#337)

**Files:** new `scripts/configs/animation/npc-idles-runtime-v1.json`,
`harness/models/synty/npcs/activities/warcamp-human-idles-v1/{clips.glb,profile.json}`,
`scripts/test_npc_idle_expansion.py`, private
`evidence/npc-activities/idle-expansion-v1/`, required root knowledge/runs record;
regenerate inventory/mesh stats and designated live pointers via existing tools.
**Contract:** same schema1, one30-clip library, one owner per supported look.
Existing12 published tracks, all old activity refs/labels and every body/downed/
atlas/idle/walk byte are unchanged. New refs under `dnd5e:activities:`:
shy, slumped, dramatic, drunken-sway, arms-folded-grumpy, hands-on-hips-grumpy.
Labels match the approved audition. All nonmoving, relaxed entry/exit,
loop-boundary leave. Existing profiles remain immutable.
**Tests first:** missing publication/refs fail; `test_expanded_profile_preserves_existing_looks_and_activities`
checks exact72 body hashes and prior activities; `test_existing_published_tracks_remain_exact`
compares all12 semantic tracks/timing; `test_new_clips_rebuild_and_preserve_skin_deformation`
rebuilds the18 new tracks and checks every baked50-joint sample against its donor
with existing independent skin-world tolerance1e-4. Missing clip/hash/rig cases
remain covered by existing cooker/profile negatives.
**Commands:** background Blender with --python-exit-code1; scoped unittest
`test_npc_activity*.py`, `test_npc_social_activities.py`, `test_npc_idle_expansion.py`;
inventory --check, stage --verify-only. Measure library bytes/decoded channels;
record costs rather than claiming free expansion. Unrelated broad-suite caveats
are not represented as green. Native proof is required in T4.

## T3 — adopt in both existing pickers (web#1373)

**Files:** `scripts/configs/npc-appearance-releases.json`, generated
`src/generated/npcAppearanceCatalog.ts`, `scripts/npcAppearancePublication.test.ts`,
`src/npc-activities/idleExpansion.test.ts`, existing
`src/concepts/encounter-studio/StudioArrangePanel.test.tsx`,
`src/concepts/world-building/{SayActivityFields.test.tsx,roomDefaultActivity.test.ts}`,
root knowledge/runs record. No production renderer/controller change expected.
**Contract:** actual pushed candidate pin and exact profile hash; replace the
single selected social profile, not overlapping profile ownership. Keep283
appearance entries and unrelated provider pins. No schema/SDK change.
**Tests first:** every new ref offered in both actual editor suites; default
undo/redo and unsupported-look preservation; positive counted response edits
preserve line/weight. Generated-data tests cover full72-look support, phase
names, moving exclusion and old activities. Existing failed-body fixture already
consumes the whole declared library; no assertion weakening.
**Commands:** npm ci; focused Vitest red -> generator with clean private provider
candidate -> pinned runtime sync -> focused green -> one full npm run ci-check
at PR boundary. Draft while provider unpublished; publish first working push.

## T4 — actual joined acceptance (web#1373 + provider evidence)

**Setup:** independent3049/8139 with normal released API pins, untracked open
walk fixture, supported03/B NPC and reach-respecting actions. No source-copy
Go overrides. Keep previous operator services untouched.
**Cases:** six default/reply permutations covering every new choice in both roles;
actual native Studio select/save/reopen/Play, complete Enter/Sustain/Exit and
count2 then return to a DIFFERENT authored default. Host and second client see
same look/count. Each long count20 is interrupted by NPC movement: finish exit,
translate only while walking, return to configured default, never revive old
cue. Reuse unchanged lifecycle unit regressions; no new timer/queue/mixer.
**Artifacts:** local `_job_npc_idle_expansion_*` drivers under a game-dev worktree,
private compact traces/screenshots/verifier and public metadata-only QA summary.
Inspect source/runtime poses, especially Shy hands-behind-back and grumpy hands;
raise only genuine art decisions, not fixture setup questions. Preserve source
motion unless the operator agrees to a visible correction.
**Finish:** exact-head local/hosted evidence, applicable independent review once,
focused finding dispositions, useful preview/Try-it. New-wave merge authorization
is a final hold, not permission to stop at a successful provider test.

## Checked coverage

| Requirement | Tasks | Proof |
|---|---|---|
| All six approved motions |T1,T2,T4| Hash-bound archive, sample cook checks, inspected native poses |
| Existing art/motion preserved |T1,T2,T3| Old Action/rest digests,12 exact tracks,72 body hashes/283 catalog rows |
| Both pickers and persistence |T3,T4| Real picker tests plus native server save/reopen/launch |
| Counts/default/interruptions |T4| Six native permutations, count2/count20 on both clients |
| No combat/locomotion scope |T1–T4| Metadata-only activities; no gameplay/SDK/controller change |
| Working preview and readiness |T4| New ports, evidence, one review, new merge hold |

| Provider | Consumer | Produced/consumed contract | Proof |
|---|---|---|---|
| Approved55 checkpoint |Established ARP mapping |18 named actions + explicit source/target identities | Original hashes,48 rows, zero constraints, native comparison |
|50 donor |Existing offline cooker |Named baked tracks in compatible bind basis | Independent skin-world/timing check |
|30-clip profile |Catalog generator |Schema1/hash/72 exact body bindings | Rebuild and parser gates |
|Generated metadata |Both pickers + existing mixer |Opaque refs, phases, moving=false | Real editor tests + T4 |
|Existing released answer stream |Existing cursor |Captured ref+count; no new contract | Both-client native playback |

No architectural question is currently open. Conversion/export preservation is
measured before publication; fixture/metadata details follow existing mechanisms.
No fresh broad review or repeated approval ceremony is part of this plan.
