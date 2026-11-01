# Approved social activities — execution plan

Story: project#784. Provider: assets#334. Consumer/acceptance: web#1370.

## Goal and authority

The operator approved the three motions in the existing lab and the outcome:
choose Arms folded, Hands on hips or Standing prayer in either Studio activity
picker, save/reopen and see correct default/count-response playback in game.
The approved conversational brief is recorded on #784. This extends the existing
NPC activity design/profile, not a new gameplay capability or playback owner.
Routine integration fixes continue to the finish line without new approval gates.
New-wave merges still need explicit authorization. Predecessor #746 is merged to
dev (API7e3f267, web b5cc3417; behavior0.18.0/encounter0.164.0/session0.152.0).

## Constraints and inspected baseline

Assets main7d421f1, web dev b5cc3417. Dedicated `.worktrees/npc-social-activities`
in assets/web/project. Preserve operator3046/8136, castle3030/8115, lab3042,
older pacing3044. Isolated QA3047/8137 may be reused; never modify the lab donor.
No patrol/route triggers, gameplay delay, API/proto/toolkit extension, new rig
family or appearance replacement. Existing72 human looks, body/downed/atlas/
idle/walk bytes and Thoughtful samples remain unchanged. Licensed bytes stay
in private assets. Folded-thumb seam is inspected, not silently re-authored.

Existing owners inspected:
- assets `npc_activity_library.py`: CookInput(source,target,clips) -> geometry-
  free CookOutput; offline basis conversion, rejecting incompatible rigs.
- assets `npc_activity_profile.py`: ProfileInput(runtime_root,library,recipe)
  -> hash-bound schema1 profile; verifies declared clips and all body bindings.
- archive `library/animation-donors/npc-activities-v1/models/prisoner-02.glb`
  is the accepted50-joint14-clip donor; original55-joint fighter remains a lab
  reference, not an added runtime family.
- web `scripts/npc-activity-profile.mjs` and catalog generator consume the
  provider map, not a hard-coded stance enum. Existing mixers own sequencing;
  `SayActivityFields` and default activity controls use provider choices.

## Task1 — publish a private candidate (assets#334)

**Delivers:** three new named refs plus unchanged Thoughtful on the existing
supported bodies. No body re-export.
**Files:** new `scripts/configs/animation/npc-social-runtime-v1.json`,
`harness/models/synty/npcs/activities/warcamp-human-social-v1/{clips.glb,profile.json}`,
`scripts/test_npc_social_activities.py`, private
`evidence/npc-activities/social-activities-v1/`; regenerate complete inventory
and mesh stats through existing scripts. Existing cooker/profile stay generic;
change them only if a demonstrated contract-preserving defect requires it.
**Interface:** schema1 profile, same fields/absence semantics. New refs:
`dnd5e:activities:arms-folded`, `dnd5e:activities:hands-on-hips`,
`dnd5e:activities:standing-prayer`. Labels match the approved lab. All are
nonmoving, relaxed entry/exit poses, loop-boundary leave. Clip triples are
Idle_ArmsFolded_Casual{_Enter,,_Exit}, Idle_HandsOnHips{_Enter,,_Exit},
Idle_PrayerStanding{_Enter,,_Exit}. Include unchanged Thoughtful triple in one
12-clip geometry-free library. Keep the old Thoughtful profile/files immutable;
web selects the expanded profile, never two overlapping appearance owners.
**Tests:** `test_published_social_profile_covers_unchanged_supported_looks`
compares regenerated profile and all72 body hashes; fails before publication.
`test_cooked_social_clips_preserve_donor_motion_and_thoughtful` compares baked
sample timing and semantic Thoughtful channels against the old library, checks
all12 clips and geometry absence. Existing cooker negative/transform tests stay.
Inspect folded thumb endpoint rotations and rendered loop; preserve approved
motion unless an actual blocking defect requires correction and a new look.
**Verify:** from assets worktree, `python3 -m unittest discover -s scripts -p
 'test_npc*activit*.py'`, `python3 scripts/build_synty_complete_inventory.py --check`,
`python3 scripts/verify_web_asset_stage.py --verify-only`; record any unrelated
known broad-suite failure separately, never call it green. Compare `git diff`
for zero changes to existing body/palette/downed/old library files.
**Evidence:** reproducible cook command, source/output hashes, compatible looks,
clip timings/endpoints, native binding views and private scoped evidence.

## Task2 — adopt via the existing consumer (web#1370)

**Prerequisite:** pushed provider candidate from Task1, not a provider merge.
**Files:** `scripts/configs/npc-appearance-releases.json`, generated
`src/generated/npcAppearanceCatalog.ts`; new
`src/npc-activities/socialActivities.test.ts` and relevant existing picker tests
if they fail to cover the generated choices. No licensed bytes in git.
**Interface:** pin the actual pushed provider revision/profile SHA, replace the
single selected Thoughtful profile with expanded social profile. Preserve all
appearance entries and unrelated provider pins. Regenerate with the existing
`generate-npc-appearance-catalog.mjs` CLI and mirror pinned runtime assets.
**Tests:** `socialActivities.test.ts` verifies each new ref is offered for the
same complete supported look set, each Enter/Sustain/Exit resolves, moving
choices are not offered as stationary cues, and Thoughtful/default/walk remain.
Test through generated provider data and existing picker helpers, not copied
label lists as a substitute. Add targeted actual picker assertions for selection
and positive-count save/export if existing parameterized coverage is insufficient.
**Verify:** focused Vitest tests first red on old profile, then green; one
`npm run ci-check` before PR publication. Preserve the existing one-mixer/cursor
and native finite-loop model; do not implement per-animation dispatch branches.
**Evidence:** generated metadata diff, pinned sync, unchanged appearances,
complete local/hosted gates, draft integration PR while provider unpublished.

## Task3 — prove and hand over the integrated experience (web#1370)

**Prerequisites:** Tasks1–2 and released existing API. Boot isolated web preview
with exact candidate pins and the existing API on QA ports; accepted preview and
lab stay unchanged. Use the small open yard, walk within current Speak reach.
**Behavior/acceptance:** for each of the three, choose a default and a counted
response in native Studio, save/reopen, launch on custom03/B look. Observe real
Enter/Sustain/Exit and exactlyN sustain repetitions; return to configured default.
Both clients must receive selected-response playback. Interrupt a longer cue by
movement and prove exit-before-walk and permanent discard, not catch-up revival.
Include a different non-Relaxed default to exercise inter-stance return. Existing
absence/unsupported-body diagnostics, downed/failed-body retirement and scope
fencing regressions must remain green. No server waits for animation.
**Files:** local `_job_npc_social_*` drivers under the game-dev worktree; compact
private receipts/screenshots under Task1 evidence, public metadata-only summary
on issues/PRs. Temporary YAML stays untracked and out of shipped API shelf.
**Verify:** browser screenshots inspected, no page/binding errors, sampled real
mixer counts/movement, actual two-client delivery. One bounded independent review
of assets+web, focused failing-first corrections; no repeated broad campaign.
**Finish:** working preview/Try-it, evidence, applicable gates and ready-for-merge
verdicts with provider-publication hold explicit. Request new-wave merge approval;
then provider merge -> exact released hash -> web released pin/dev, no game release.
Clean disposable QA/review resources without deleting operator/lab custody.

## Visible checks

| Requirement | Task | Concrete proof |
|---|---|---|
| Approved three motions, same look |1| Donor binding/sample equality, loop inspection, private render |
| Default and counted pickers |2,3| Generated-data tests, actual select/save/reopen/launch |
| Correct phases/count/default return |3| Native action repetitions and phase trace per stance |
| Movement permanently discards |3| Long cue interruption, walking-only translation/no revival |
| Custom looks and multiplayer |1,3| All72 unchanged hashes; two clients on03/B |
| Usable preview, gates/review |2,3| Exact-head stack, inspected images, one review and dispositions |

| Provider | Consumer | Contract / availability | Proof |
|---|---|---|---|
| Archived50-joint donor |Existing cooker| Named baked clips; donor present and immutable | Source hash, independent transform tests |
| Cooked library/profile |Catalog generator| Schema1/hash-bound map and72 appearances; T1 pushed commit | Rebuild equality and parser |
| Generated catalog |Both Studio pickers/mixer| Same refs/triples/nonmoving flags; T2 generated data | Picker tests and native T3 |
| Existing selected-answer API |Existing cue cursor| Opaque ref+uint32 count already released in#746 | Both-client real playback, no new backend work |

Plan checked against inspected code: no new seam/schema needed; expanded profile
avoids overlapping appearance ownership. Actual archive approval is not silently
claimed as runtime readiness. UI/count/multiplayer proof remains required. No
architectural question is currently open. Provider publication and new-wave merge
permission are release holds, not reasons to stop development.
