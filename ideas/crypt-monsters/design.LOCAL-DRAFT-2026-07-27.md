# Crypt Monsters

**Date:** 2026-07-25
**Status:** Draft / verbally approved in session pending written review
**Coordination:** rpg-project#110; rpg-dnd5e-web#559 and #577
**Open implementation reviews:** rpg-game-assets PR #28, branch `asset/559-undead-roster-promotion`; rpg-dnd5e-web PR #594

## Goal

Make the crypt's first monster presentation believable without changing a
monster's rules identity to fit a visual. Reuse and harden the open asset and
web PRs rather than rebuilding them. Neither PR merges until its applicable
gates pass.

The first specimen is `Skeleton_Soldier_01`. It must establish an intentional
standing idle and its downed sibling on the Dungeon target rig before any
roster-wide work. Walk clips are explicitly out of scope.

## Rules And Visual Identity

Rules identity is authoritative; a visual may not masquerade as another
creature's rules identity.

| Rules identity | Visual | Availability |
| --- | --- | --- |
| `dnd5e:monsters:skeleton` | `Skeleton_Soldier_01`, `Skeleton_Soldier_02`; `Slave_01` is an optional candidate | Runtime after the standing/downed asset gate |
| `dnd5e:monsters:skeleton-captain` | `Skeleton_Knight` | Runtime after the standing/downed asset gate |
| Future Specter reference | `Tormented_Soul` | Private technical asset only until its real reference and material gate pass |
| Future Ghost reference | `Ghost_01`, `Ghost_02` | Private technical asset only until its real reference and material gate pass |

The crypt boss is exactly `dnd5e:monsters:skeleton-captain`, implemented by
rpg-toolkit PR #820, with `Skeleton_Knight` as its visual. This supersedes the
earlier Wight mapping: there is no Wight identity or Life Drain claim.

Private technical asset promotion and runtime rules promotion are separate
events. Ghost and Tormented assets may appear in the private NPC manifest with
`rulesRef: null`, but the resolver must not select them until a real rules
reference exists and the material gate passes. Skeleton and Skeleton_Knight may
be exposed at runtime after their applicable gates pass.

## Resolver Contract

For this initiative, accept PR #594's single centralized
`MONSTER_REF_MODELS` table as the bounded current implementation. Runtime
manifest loading is explicitly out of scope. No scattered resolver conditionals
may be added outside `src/components/hex-grid/monsterModels.ts`.

The private technical source of truth is
`harness/models/synty/npcs/manifest.json` on rpg-game-assets PR #28. The
runtime candidates in PR #594 are exact and must be covered by resolver tests:

| Rules reference | Standing candidate(s) | Downed candidate(s) |
| --- | --- | --- |
| `dnd5e:monsters:skeleton` | `/models/synty/npcs/skeleton-soldier-01.glb`, `/models/synty/npcs/skeleton-soldier-02.glb` | `/models/synty/npcs/skeleton-soldier-01-downed.glb`, `/models/synty/npcs/skeleton-soldier-02-downed.glb` |
| `dnd5e:monsters:skeleton-captain` | `/models/synty/npcs/skeleton-knight.glb` | `/models/synty/npcs/skeleton-knight-downed.glb` |

Tests must assert the exact first-resolved skeleton paths, exact captain paths,
the downed sibling paths, and that Ghost/Specter remain unresolved. A future
manifest-driven resolver is a separate initiative; until then, changes update
the one table and its exact-path tests together.

## Animation Contract

The 55-joint, animated, Dungeon-target invariant applies only to **standing
GLBs**. Every standing in-scope model must have the 55-joint Dungeon target rig,
the approved deliberate armed idle, preserved facing/forward axis, and its
verified loop seam.

Downed siblings are static, one-mesh, no-animation, Root-wrapped outputs. They
may have no rig. Their gate is Root wrapper, one mesh/material profile, scale,
and bounding box, not the standing 55-joint/animation invariant.

`Skeleton_Soldier_01` has exactly these specimen candidates:

1. **Candidate A, baseline:** PR #28's staged/canonical
    `harness/models/synty/npcs/skeleton-soldier-01.glb`, using `Idle_Base`.
    Its provenance is PR #28's same source `A_POLY_IDL_Base_Masc.fbx`; it is
    the baseline only, and its T-pose silhouette does not pass.
2. **Candidate B, hypothesis:** party-derived ARP `bones_map_v2` with
    `Hips -> Pelvis` and the preserved-rest workflow, adapted to the Dungeon
    target. It uses the same `A_POLY_IDL_Base_Masc.fbx` source as Candidate A,
    but differs through the ARP preserved-rest experiment. It is not assumed
    valid: compare rest matrices, constraints, pose, axis, and loop against the
    Dungeon target before approval.
3. **Candidate C, fallback if B fails:** a directly authored armed standing
   stance on the Dungeon target rig with an exact verified loop seam. Do not
   replace B with another generic retargeting assumption.

Existing embedded monster weapons are preserved. No batch work begins until an
independent asset-gate reviewer and Kirk both approve one specimen candidate.

## Reproducible Specimen Gate

Use the persistent asset worktree
`/home/kirk/recovery-worktrees/crypt-monsters-assets` for rpg-game-assets PR
#28 branch `asset/559-undead-roster-promotion`. Do not use `/tmp` for the sole
worktree, checkpoint, stage, or evidence.

The private durable roots are:

```text
/home/kirk/game-dev/assets/synty/crypt-monsters/review
/home/kirk/game-dev/assets/synty/crypt-monsters/approved
/home/kirk/game-dev/assets/synty/crypt-monsters/stage
```

`review` and `approved` are ignored local `.blend` conveniences, not durable
backup. The durable state is the versioned script, configuration, and Dungeon
profile; the approved exported GLB committed and pushed promptly to the private
`rpg-game-assets` branch; and the permitted screenshot evidence committed to
the public evidence branch. Licensed source FBX remains local and can be
re-downloaded. A crash before the private push is recovered by reproducing from
the versioned script/configuration/profile and licensed source; never claim an
ignored `.blend` is recoverable.

For an approved candidate, perform this checkpoint procedure before proceeding:

1. Save or copy the candidate `.blend` to its local `approved` path.
2. Record that `.blend`'s SHA-256 in the versioned private recipe or manifest.
3. Immediately export the approved GLB, then commit and push the GLB plus its
   recipe, generating script, pose configuration, and profile to the private
   branch.
4. Commit only permitted screenshots to public branch
    `evidence/asset-pipeline-wave1` under
    `playtest-evidence/asset-pipeline/issue-559-crypt-monsters/<run-or-candidate>/`.
    Do not include licensed assets, recordings, or reports.

Use one run identifier such as `RUN=2026-07-25-skeleton-soldier-01`. The
candidate producer must complete before its validator or renderer consumes the
GLB. Candidate A has no producer: its input is the PR #28 canonical file.

**Candidate B implementation deliverable, not an existing command:** implement
`scripts/retarget_crypt_monster_idle_arp.py`. Its source idle FBX is
`/home/kirk/game-dev/assets/synty/animation-idles/SourceFiles/Animations/Polygon/Masculine/Base/Stances/A_POLY_IDL_Base_Masc.fbx`
(SHA-256 `f495cdc98e2ebe9fab22945494eb5990d393c4b5fc888701481b2958bd16246a`).
Its bone map is `scripts/configs/animation/polygon_to_rivals_bones.json`
(SHA-256 `5415490a76475e6892b85f063afa141fc307eade62c70743d9094f7f6ed004b0`),
resolved from the rpg-game-assets worktree, with `Hips -> Pelvis`. The script
must consume that exact source idle FBX, the target Soldier01 GLB or FBX, that
bone map, and the exact versioned Dungeon profile; it must export the candidate
only to this persistent stage path:

```bash
RUN=2026-07-25-skeleton-soldier-01
ASSET_WORKTREE=/home/kirk/recovery-worktrees/crypt-monsters-assets
SOURCE_IDLE_FBX=/home/kirk/game-dev/assets/synty/animation-idles/SourceFiles/Animations/Polygon/Masculine/Base/Stances/A_POLY_IDL_Base_Masc.fbx
BONE_MAP="$ASSET_WORKTREE/scripts/configs/animation/polygon_to_rivals_bones.json"
blender -b -P scripts/retarget_crypt_monster_idle_arp.py -- --source-idle "$SOURCE_IDLE_FBX" --target harness/models/synty/npcs/skeleton-soldier-01.glb --bone-map "$BONE_MAP" --profile scripts/configs/crypt-monsters/dungeon-soldier01-retarget-v1.json --out /home/kirk/game-dev/assets/synty/crypt-monsters/stage/$RUN/candidate-b/skeleton-soldier-01.glb
```

**Candidate C implementation deliverable, not an existing command:** implement
`scripts/author_crypt_monster_stance.py` and versioned pose configuration
`scripts/configs/crypt-monsters/skeleton-soldier-01-armed-stance-v1.json`. It
must author directly on the Dungeon target and export only to its persistent
candidate path:

```bash
RUN=2026-07-25-skeleton-soldier-01
blender -b -P scripts/author_crypt_monster_stance.py -- --target harness/models/synty/npcs/skeleton-soldier-01.glb --profile scripts/configs/crypt-monsters/dungeon-soldier01-retarget-v1.json --pose-config scripts/configs/crypt-monsters/skeleton-soldier-01-armed-stance-v1.json --out /home/kirk/game-dev/assets/synty/crypt-monsters/stage/$RUN/candidate-c/skeleton-soldier-01.glb
```

Render the full loop, including the seam, only after selecting the appropriate
candidate input. These are the actual `animation_qa_render.py` flags:

```bash
RUN=2026-07-25-skeleton-soldier-01
# Candidate A: PR #28 staged/canonical baseline.
CANDIDATE=candidate-a
CANDIDATE_GLB=harness/models/synty/npcs/skeleton-soldier-01.glb

# Candidate B: generated by retarget_crypt_monster_idle_arp.py above.
CANDIDATE=candidate-b
CANDIDATE_GLB=/home/kirk/game-dev/assets/synty/crypt-monsters/stage/$RUN/candidate-b/skeleton-soldier-01.glb

# Candidate C: generated by author_crypt_monster_stance.py above.
CANDIDATE=candidate-c
CANDIDATE_GLB=/home/kirk/game-dev/assets/synty/crypt-monsters/stage/$RUN/candidate-c/skeleton-soldier-01.glb
```

Run one of those three assignments, not all three, then use its
`$CANDIDATE_GLB` for every render:

```bash
blender -b -P scripts/animation_qa_render.py -- --character "$CANDIDATE_GLB" --clip Idle_Base --out-dir /home/kirk/game-dev/assets/synty/crypt-monsters/stage/$RUN/$CANDIDATE/front --frames 28 --wrap-frames 2 --angle 0
blender -b -P scripts/animation_qa_render.py -- --character "$CANDIDATE_GLB" --clip Idle_Base --out-dir /home/kirk/game-dev/assets/synty/crypt-monsters/stage/$RUN/$CANDIDATE/side --frames 28 --wrap-frames 2 --angle 90
blender -b -P scripts/animation_qa_render.py -- --character "$CANDIDATE_GLB" --clip Idle_Base --out-dir /home/kirk/game-dev/assets/synty/crypt-monsters/stage/$RUN/$CANDIDATE/three-quarter --frames 28 --wrap-frames 2 --angle 45
```

`animation_qa_render.py` currently has no elevation or top-down flag: `--angle`
is a horizontal orbit only. Therefore there is no truthful top-view command
template using its current flags. The top-view requirement blocks this gate
until that renderer has an implemented, documented top-view option and the gate
record includes its exact command; do not mislabel another `--angle` value as a
top view.

Copy only resulting screenshots to branch `evidence/asset-pipeline-wave1` at
`playtest-evidence/asset-pipeline/issue-559-crypt-monsters/$RUN-$CANDIDATE/`.
Licensed source files, converted binaries, `.blend` checkpoints, recordings,
and reports remain private. The asset gate passes only with the front, side,
three-quarter, and implemented top view; structural validation; exact loop
seam; and approval from an independent asset-gate reviewer plus Kirk visual
acceptance.

### Structural Promotion Gate

**Implementation deliverable, not an existing command:** implement
`scripts/validate_npc_promotion.py`. It must write the specified JSON report
and fail its process when a required assertion fails. Its Soldier01 profile
must validate the exact Root transform; expected body name; one standing
mesh/material; 55 joints; unchanged geometry and materials from the PR #28
baseline; scale and bounds within `1e-6`; and the exact `Idle_Base` clip and
loop-seam contract. It must
also validate the downed sibling as Root-wrapped, one static mesh/material,
with no animation and the approved scale and bounding box. The Soldier01
baseline values are exactly 3258 triangles, one primitive/material,
one 1024x1024 texture, and 55 joints.

Run it after the candidate producer and before rendering or promotion:

```bash
python3 scripts/validate_npc_promotion.py --standing "$CANDIDATE_GLB" --standing-baseline harness/models/synty/npcs/skeleton-soldier-01.glb --downed harness/models/synty/npcs/skeleton-soldier-01-downed.glb --profile scripts/configs/crypt-monsters/skeleton-soldier-01-promotion-v1.json --report /home/kirk/game-dev/assets/synty/crypt-monsters/stage/$RUN/$CANDIDATE/validation.json
```

The report's committed private recipe or summary must name its candidate input,
baseline input, downed input, profile, pass/fail result, and the measured
values. Use the existing supporting gates with their current exact interfaces:

```bash
python3 scripts/build_mesh_stats.py
python3 scripts/animation_qa.py --report harness/animation-qa/report.json
```

Record the relevant Soldier01 entries from `harness/models/synty/mesh-stats.json`
and `harness/animation-qa/report.json` in the same committed private recipe or
summary. These existing scripts measure the harness-wide canonical files; the
proposed validator is the candidate-specific structural gate.

## Ghost And Tormented Material Gate

Evaluate `Ghost_01`, `Ghost_02`, and `Tormented_Soul` in a real crypt route or
composition lab under ordinary crypt lighting, then in a doorway/backlight
setup and against floor and wall overlap. Capture gameplay-camera evidence plus
close, front, and three-quarter views as screenshots only.

The material gate passes only when gameplay zoom has a readable silhouette and
floor boundary; transparent layers sort stably with no order inversion or
popping; no floor/wall z-fighting or vanishing occurs; and the creature remains
visually distinguishable from out-of-line-of-sight ghosting. An independent
reviewer and Kirk must approve the captured evidence. Failure keeps these assets
private technical entries with `rulesRef: null` and blocks runtime selection.

## Gates And Evidence

The following are separate gates, each with an explicit pass or fail record:

1. **Standing asset gate:** the specimen evidence proves a deliberate armed
   stance rather than a T-pose; the exact loop seam, facing/forward axis, Root
   wrapper, one mesh/material profile, 55 joints, scale, and mesh stats pass.
   Clip existence alone never passes.
2. **Downed asset gate:** the static Root-wrapped one-mesh/no-animation sibling
   has the approved scale and bounding box.
3. **Client resolver gate:** PR #594's centralized table resolves only the
   exact approved NPC paths and candidates above, including the explicit downed
   sibling; Ghost/Specter paths remain unresolved.
4. **Rules promotion gate:** every runtime mapping has its real rules reference.
   Skeleton and Skeleton_Knight may pass this gate; Ghost/Tormented remain
   private until their real reference and material gate pass.
5. **Spirit material gate:** the crypt-route evidence and approvals above pass.

The web PR #594 cannot merge while the applicable asset or resolver gate fails,
even if its code is otherwise ready. A gate failure leaves canonical assets and
runtime mappings unchanged, records the failing evidence and checkpoint hash,
and returns work to the failed gate.

## Release Workflow

Reuse the established private stage -> public screenshots -> independent review
-> validate -> apply/index -> one release commit -> sync/CI/client gate
workflow. Persistent worktrees and the review, approved, and stage roots above
are required. Licensed source and converted binaries remain private and never
appear in public evidence, commits, issues, or PRs. Public evidence is
screenshots only, committed to `evidence/asset-pipeline-wave1` under
`playtest-evidence/asset-pipeline/issue-559-crypt-monsters/<run-or-candidate>/`.

## Rollback

For an asset regression, open a new private asset PR that reverts the single
private asset release commit. Do not partially copy old GLBs, manifests, or
reports. After the revert, sync and rebuild, then rerun the structural,
standing-anatomy, downed, and spirit gates for every affected model.

For a resolver regression, open a new web PR that reverts the resolver commit.
Do not partially copy old resolver entries or tests. Run CI, then rerun the
real crypt-route identity and death-swap gate to confirm the reverted resolver
selects the correct visual and downed sibling. Every rollback uses a commit
revert, not a file-level reconstruction.

## Alternatives Rejected

### Clean Rebuild

Rejected as duplicative of the existing open asset and web work. The direction
is to harden rpg-game-assets#28 and rpg-dnd5e-web#594 with the gates above.

### Merge Then Repair

Rejected because it knowingly ships the T-pose silhouette and makes visual
acceptance a post-release repair task. No implementation merges around the
specimen pose gate.

## Risks And Handling

- Candidate B's party-derived ARP preserved-rest workflow may be wrong for the
  Dungeon target. Treat it as a hypothesis and use Candidate C if it fails.
- A valid clip name can conceal an invalid silhouette, axis, loop, or downed
  wrapper. Require structural validation and motion evidence together.
- The current QA renderer cannot produce top-view evidence. Do not waive or
  fake that view; add a real renderer option before the specimen gate passes.
- Transparent spirits can disappear into the floor or sort incorrectly. Keep
  them private until the material gate proves readability and stability.
- Resolver shortcuts can leak unavailable Ghost/Tormented visuals into
  gameplay. Keep all mapping in the single resolver table and require a real
  rules reference plus the material gate before adding either.
- Private licensed asset handling can be broken by evidence collection. Keep
  checkpoints and binaries private; publish only permitted evidence.

## Success Criteria

This initiative succeeds only when `Skeleton_Soldier_01` has independently
reviewed and Kirk-approved, evidence-backed standing and downed outputs; batch
work starts only after that approval; standing outputs meet the 55-joint and
animation invariant while downed outputs meet their static contract; the client
uses the bounded centralized resolver with exact-path tests; and every runtime
visual has an honest rules identity. Ghost and Tormented assets remain private
technical entries unless their rules and material gates pass.

## Fresh Session Start

1. Read `memory/unarmed-character-animation-workflow.md`.
2. Fetch rpg-game-assets PR #28 branch `asset/559-undead-roster-promotion` into
   `/home/kirk/recovery-worktrees/crypt-monsters-assets`; do not create the
   working state in `/tmp`.
3. Inspect rpg-dnd5e-web PR #594 and its centralized
   `src/components/hex-grid/monsterModels.ts` table and exact-path tests.
4. Use the private roots `/home/kirk/game-dev/assets/synty/crypt-monsters/` with
   `review`, `approved`, and `stage` subdirectories. Treat the ignored `.blend`
   files as local convenience only; record an approved checkpoint SHA-256 in a
   versioned private recipe, export its GLB, and commit/push the GLB, recipe,
   script, configuration, and profile before proceeding.
5. For Candidate A, set `CANDIDATE=candidate-a` and
   `CANDIDATE_GLB=harness/models/synty/npcs/skeleton-soldier-01.glb`; for B or
   C, first run its producer command and use its distinct staged GLB path.
   Run the candidate-specific structural validator, then the front, side, and
   three-quarter render commands in **Reproducible Specimen Gate**. Do not
   claim top evidence from `--angle`; implement and document a real top-view
   renderer option first.
6. Copy only permitted screenshots to branch `evidence/asset-pipeline-wave1`
    under
    `playtest-evidence/asset-pipeline/issue-559-crypt-monsters/$RUN-$CANDIDATE/`.
    Do not include licensed assets, recordings, or reports.
7. Do not batch until an independent asset-gate reviewer and Kirk both approve
   the specimen.
