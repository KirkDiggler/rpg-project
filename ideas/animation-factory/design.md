# Animation Factory

## Status: Approved conversational design including the toy diagnostic and Step 0, awaiting amended written-spec review (2026-07-23)

Tracking: `rpg-project#114`; related investigation: `rpg-dnd5e-web#548`.

## Objective and scope

Build an agent-operated, private animation factory: Kirk selects an owned Synty
source, describes a requested clip change, inspects immutable evidence, and
explicitly approves promotion. The first proof is one all-or-nothing replacement
batch for `Walk_Forward` on exactly these canonical class GLBs:

| Family | Targets |
| --- | --- |
| standard | monk, rogue |
| Big-Rig | fighter, barbarian |

The factory interfaces are reusable for later clips. This proof does not add a
rig family, support every animation category, permit per-character correction,
or change runtime movement to root motion. A new family must complete its own
proof; there is no generic fallback. If either family profile needs a
per-character exception, this direction fails and the team reconsiders
native target-family clips or a canonical rig instead of weakening that rule.

Private owned FBX inputs and canonical outputs remain within the licensed asset
pipeline. Canonical private GLBs live under
`rpg-game-assets/harness/models/synty/characters/`; the web receives them only
through its ignored public asset tree. Evidence may identify and hash licensed
inputs but must not publish raw Synty files.

## Proven facts and baseline

- `scripts/retarget_locomotion_common.py` is the current shared custom
  retarget core. It computes corrections from live source/target rests and
  presently uses direct multiplication for its arm chain and conjugation for
  other mapped bones. That mixed implementation is the failing walk baseline,
  not accepted production quality.
- `scripts/configs/animation/polygon_to_rivals_bones.json` contains the exact
  46-bone Polygon-to-Rivals map. The current mapping folds source `Hips` onto
  target `Pelvis`; the source `Root` is not a driven target bone.
- `bigrig_family_rest.json` is the existing Big-Rig guard. Its controlled
  measurements establish one 55-bone rest table for the tested Big-Rig family;
  the current runner checks it with a 1.0-degree tolerance.
- The non-root-motion `A_Walk_F_Masc.fbx` baseline has 31 keys from frames
  2--33, is about 1.03 seconds at 30 fps, has zero top-level `Root`
  translation, and contains natural Hips bounce. The root-motion sibling is
  forbidden because the web owns board translation.
- The existing temporal harness gates loop seam and root drift. Its
  lowest-foot-Y instantaneous-speed proxy is informational: its catalog
  distribution cannot distinguish valid stance travel from unintended skate.
- Disposable `pose_qa.py` evidence found the shipped rogue and barbarian walks
  failing all four approved anatomy/ground gates. The factory must prove the
  replacement rather than characterize the current output as passing.
- The web's class model resolves a walk-named clip, clones each skeleton with
  `SkeletonUtils.clone()`, and binds it with `useAnimations`. It currently
  free-runs the selected action. `useHexMovePath.ts` interpolates an adjacent
  `HEX_SIZE=1` hex step of `sqrt(3)` world units in 0.45 seconds, approximately
  3.85 world units/second.

## Decision

Use a controlled custom retargeter with two versioned family profiles:
`standard` for monk/rogue and `bigrig` for fighter/barbarian. A profile contains
only reusable calibration, including axis/roll conversion and the physical
family-neutral pose correction. It never names an individual character or
contains an exception offset.

The output action remains named `Walk_Forward`. Root/world translation remains
asset-free and web-owned. The sole added cross-repo seam is measured gait-speed
metadata and a defined web action playback rate.

### Rejected alternatives

- The tested generic-target Auto-Rig Pro FK retarget reduced torso direction
  below one degree but left upper-arm deltas around 72--76 degrees and failed
  ground gates. Verdict: `ARP_GENERIC_TARGET_REJECTED` only for the tested
  source, map, configuration, and rogue/barbarian generic targets. It does not
  reject canonical ARP rigs, Smart rigging, or future ARP configurations.
- The exact eight-bone preserved virtual-rest experiment aligned selected
  directions to zero before bake but worsened output upper arms to about
  92--94 degrees and still failed ground gates.
- Task 4 independently rated the specification `PASS WITH MINOR DEVIATION` and
  experiment quality `PASS WITH IMPORTANT LIMITATION`. The minor deviation was
  research pseudocode's identity-transform precondition; actual source and
  targets have matching +90-degree-X, 0.01-scale world transforms and ARP uses
  world-space math. The important limitation was thin integration-regression
  coverage in the disposable proof.
- Canonical ARP re-rigging is unselected: no canonical rig or migration tooling
  exists, and it risks skins, weapons, `Root`, actions, and runtime scale
  contracts.
- Native target-family clips are geometrically attractive, but the owned
  inventory proves native Sidekick binding, not native Fantasy Rivals locomotion
  for these four classes.

## Pre-Step 0: toy armature/weapon diagnostic

Before the canonical fighter proof, run one deterministic, fresh, headless
Blender diagnostic in the disposable
`/tmp/opencode/toy-weapon-proof/` workspace. It uses a toy armature at uniform
`0.01` scale with `Root -> Hand_R`, plus a simple colored weapon mesh with one
Armature modifier and 100% `Hand_R` weights. It starts with `Idle_Toy` and adds
only `Pose_ToyWeapon`, with constant full-pose keys at frames 1 and 2. The
candidate pose must create obvious weapon world motion of at least `0.25`
meters from `Idle_Toy` at deterministic frame `1.0`.

Export a matched no-op control and the candidate, then reset Blender and
fresh-reimport both. The candidate must equal the control for hierarchy, rest
matrices, object transforms and parenting, mesh/material contract, and the
evaluated pre-existing `Idle_Toy` action. `Pose_ToyWeapon` is the sole allowed
addition; any other deviation fails. Compare transforms with matrices and
quaternions rather than raw Euler values.

At baseline, pose, pre-export, and fresh-reimport samples, derive the scale-free
hand frame from the decomposed world hand matrix. The weapon's hand-frame
coordinates must remain rigid within `<= 0.001` meters both in-scene and across
the round trip. The unchanged in-scene topology may compare vertices by index.
For pre-export/fresh-reimport comparison, use a deterministic,
topology-independent complete evaluated triangle-surface representation that
preserves material and triangle identity, because glTF may split or reorder
vertices.

The diagnostic emits a fail-closed report, before/after render, candidate GLB,
and clean inspection `.blend`. Before saving the inspection file, remove any
importer-only helpers and clear every `PoseBone.custom_shape`. This is a bounded
proof of basic Blender/glTF armature, skinning, action, and export mechanics;
it does not establish fighter correctness, canonical-asset behavior, or
retargeting.

## Step 0: disposable weapon-pose proof

After the toy diagnostic passes, before production factory code or walk-retarget
work, run a disposable proof that an agent can author and round-trip a
weapon-bearing pose on canonical `fighter.glb`. The canonical target is
read-only. The output is a constant held-pose action named `Pose_WeaponProof`,
not a static baked mesh or a Blender-session-only render.

The pose is a one-handed ready guard: the weapon arm and torso make an obvious
guard, while the off-hand remains intentionally free. It is not a two-hand
grip or IK problem. The proof uses deterministic local-bone pose data; BlenderMCP
may help visual iteration, but the result must be reproducible without session
state. Scripts, staged GLB, reports, and renders are disposable under
`/tmp/opencode/weapon-pose-proof/`. Step 0 creates no repository implementation,
does not change a canonical GLB, and cannot promote an asset.

The existing fighter weapon is already a baked, full-weight `Hand_R` skin and
prior idle renders retained its grip. This proof does not re-prove attachment in
all contexts. It validates one controlled unit: an agent-authored pose/action,
the baked weapon following it, and a factory-style export/fresh-reimport round
trip. Static mesh baking cannot prove that action contract; BlenderMCP-only state
cannot reproduce it; and Auto-Rig Pro or IK would add unneeded retargeting or
two-hand variables.

### Step 0 data flow

1. Start a fresh Blender process and import the canonical fighter.
2. Snapshot skeleton/rest data, canonical `Root`, object hierarchy, meshes,
   materials/images, existing action semantics, and the unambiguous existing
   `Hand_R`-weighted baked weapon mesh.
3. Evaluate a deterministic `Idle_Relaxed` frame as the natural standing
   baseline.
4. Layer explicit local offsets on the torso and right clavicle, upper-arm,
   forearm, and hand chain. Do not edit armature rest data or the weapon socket;
   keep the off-hand free.
5. Key the resulting full pose identically at frames 1 and 2 into
   `Pose_WeaponProof`. This one-frame span gives glTF a nonzero constant clip.
6. Export only to staging, reset Blender, fresh-reimport the candidate, and
   compare it with a matched same-exporter no-op control.
7. Produce synchronized before/after front, side, back, and three-quarter
   full-body renders plus a hand/grip close-up.
8. Kirk's visual approval is the final gate. Nothing is promoted by this proof.

### Step 0 gates and scope

The proof fails unless `Root`, skeleton hierarchy/rest matrices, object contract,
meshes, materials, and images match the no-op control; all pre-existing clips
have equivalent sampled transforms; and only `Pose_WeaponProof` is new. The
weapon must be identified unambiguously as the existing `Hand_R`-weighted baked
weapon, move at least `0.25` meters in world space from baseline to proof pose,
and pass both rigid-follow checks below. Both feet must retain baseline ground
height within `0.01` meters, every required render must complete, and Kirk must
visually accept the one-handed ready guard and hand close-up.

For every baseline, proof-pose, pre-export, and fresh-reimport weapon sample,
compute `hand_world = armature.matrix_world @ hand_pose_bone.matrix`, decompose
it as `location, rotation, _scale = hand_world.decompose()`, then build
`hand_frame_m = Matrix.Translation(location) @ rotation.to_matrix().to_4x4()`.
Express evaluated world-space weapon coordinates through
`hand_frame_m.inverted()`. This scale-free rigid frame preserves hand
position/orientation in world meters; the armature `Root` scale is an encoding
conversion and must not redefine the metric unit.

The original imported scene proves rigid following: compare every evaluated
weapon vertex at the deterministic `Idle_Relaxed` baseline and at
`Pose_WeaponProof` frame 1. At each frame, use the defined scale-free
`hand_frame_m.inverted()` coordinates and match vertices by index because
topology is unchanged in-scene. The maximum Euclidean displacement must be
`<= 0.001` meters.

The export round trip proves preservation separately: compare the complete
evaluated weapon loop-triangle surface at `Pose_WeaponProof` frame 1 immediately
before export and after fresh reimport, each through its defined scale-free
`hand_frame_m.inverted()` coordinates. Use a deterministic sorted multiset that
preserves triangle and material identity, not raw vertex indices, because glTF
may split or reorder vertices. The maximum matched coordinate delta must be
`<= 0.001` meters.

Any missing baseline clip, ambiguous weapon mesh, rest mutation, existing-action
change, grip drift, structural/export difference, ground-height failure, or
render failure stops the proof. A retry may adjust pose-bone offsets only; it
may not hand-tune the existing weapon socket or introduce per-character
retarget corrections.

Passing Step 0 proves agent-authored pose control, baked-weapon following, action
addition, export/fresh-reimport preservation, staging, and visual evidence. It
does not prove locomotion retargeting, family profiles, foot contact,
gait/cadence, Auto-Rig Pro, or production-factory readiness. The four-target
walk proof remains the production-factory proof.

## Factory contract

### Declarative batch job

A batch job declares the licensed private FBX identity, `Walk_Forward` output
name, source frame range, exact map version, profile version, and all four
targets with their approved family assignments. It also records source and
target content hashes and the requested tool version. The job has no target
specific correction field.

### Headless runner

The Blender runner launches a fresh process and factory-empty scene for every
target. It imports one canonical target, preserves the canonical `Root`, meshes,
materials, images, baked weapons, object parenting, evaluated rest surface, and
all existing idle actions, then bakes only `Walk_Forward`. It writes candidates
only to staging; it never writes a canonical asset.

Preflight rejects the whole batch unless the private source exists within the
license boundary, the 46-bone map is exact, the target matches its declared
family, input hashes match the job, channels are supported, and the source has
no root motion.

### Cadence contract

Each affected class entry in
`harness/models/synty/characters/manifest.json` gains this canonical asset
metadata after its output passes QA:

```json
"locomotion": {
  "clip": "Walk_Forward",
  "gaitSpeedMps": <positive number>,
  "measurement": "median-stance-foot-speed-v1"
}
```

`gaitSpeedMps` is the median horizontal root-space foot speed, in authored
meters per second, across accepted left and right stance intervals. It is
measured independently from each output character and recorded in that output's
evidence metadata. It is a measurement, not a retarget correction: it cannot
change a family profile, calibration, pose, floor normalization, or any other
per-character processing parameter.

The paired web change copies the approved manifest value into that class's
existing manifest-derived `ClassCharacterModelEntry`; the web does not fetch the
asset manifest at runtime. Evidence must verify exact asset/web parity for the
clip, `gaitSpeedMps`, and measurement identifier.

For a moving class model, the web calculates:

```
rawRate = runtimeWorldSpeed / (gaitSpeedMps * SYNTY_SCALE)
effectiveRate = clamp(rawRate, 0.5, 2.0)
```

The bounds are inclusive. Runtime defensively uses `1.0` and emits a diagnostic
when gait metadata is missing, non-finite, or non-positive; this preserves
legacy/existing assets. Those conditions cannot pass this batch's promotion
gate. Factory acceptance also requires the *unclamped* `rawRate` to be within
`[0.5, 2.0]`; otherwise the batch is rejected and movement pacing or the source
animation must change rather than hiding the mismatch with a clamp.

### Retarget pipeline

1. Calibration explicitly separates coordinate axis/roll conversion from the
   physical neutral-pose correction. The implementation decodes source motion
   as rest-relative local deltas, converts it into the target joint basis, then
   composes it on the family-neutral target pose. The exact quaternion equation
   is an implementation hypothesis to prove with this walk batch; the current
   mixed conjugation/direct-multiply formula is not assumed correct.
2. Deterministic floor/contact normalization is derived from source contact
   phases and target skeleton geometry. Hand-positioned per-character floor or
   contact values are forbidden.
3. The candidate is fresh-reimported and compared with a matched no-op control.
   The comparison covers skeleton, `Root`, object parenting, evaluated rest
   surface, mesh/material/image/weapon contracts, and byte- or semantic-action
   preservation for every pre-existing action. Only the additional/replaced
   `Walk_Forward` action may differ.
4. QA runs anatomy, ground, temporal, and contact diagnostics. A missing or
   skipped metric is a failure. Contact diagnostics compare expected source
   stance motion, retargeted stance motion, and runtime travel speed; the old
   lowest-foot-Y speed proxy remains non-gating.
5. QA derives the defined gait metadata from accepted stance phases, verifies
   manifest/web parity, and evaluates the unclamped cadence rate against the
   required inclusive bounds.
6. QA renders synchronized front, side, back, and three-quarter loops for every
   target.

### Evidence and approval

The QA orchestrator emits an immutable, content-addressed evidence bundle. It
contains all four staged candidates, no-op and structural diffs, motion and
contact metrics, gait metadata, synchronized renders, logs, and hashes for the
job, source, targets, tools, profiles, and outputs. Any changed input produces
a different evidence hash and invalidates prior approval.

Kirk records explicit visual approval against the evidence hash. Automation can
only reject or present evidence; it cannot promote. Promotion is a separate
operation that revalidates the approved hash and temporary outputs, then swaps
all four canonical outputs as one logical operation before asset sync. A failed
validation or swap leaves canonical files unchanged; the prior four canonical
files remain the rollback set.

## Acceptance and rejection

Every candidate in the four-character batch must satisfy all of the following:

- maximum torso direction delta `<= 5.0` degrees;
- maximum upper-arm direction delta `<= 25.0` degrees;
- ground-height span `<= 0.06` meters;
- minimum ground height `>= -0.03` meters;
- existing loop-seam and root-drift gates;
- exact preservation of the structural, rest, asset, weapon, and existing-action
  contracts;
- valid positive finite gait metadata, exact asset/web parity, and an unclamped
  cadence rate within `[0.5, 2.0]`; and
- Kirk's explicit visual approval of the evidence bundle.

Any missing mapping, profile mismatch, unsupported channel, source root motion,
structural/action mutation, skipped QA, failed threshold, undefined gait speed,
invalid gait metadata, out-of-range unclamped cadence rate, parity mismatch, or
render failure rejects the entire batch. There is no partial promotion.

## Ownership and validation

`rpg-game-assets` owns the batch schema, profiles, private-source preflight,
Blender runner, staging, structural comparison, QA, evidence bundle, approval
record, and atomic promotion. `rpg-dnd5e-web` owns consumption of gait metadata,
actual path-speed comparison, the `[0.5, 2.0]` effective action time-scale clamp,
and graceful legacy degradation. Neither repo changes the other's authority:
assets certify authored gait; the web owns world displacement and playback
application.

Tests are required at three levels:

- Pure tests: basis/transform conversion, neutral-pose composition, profile
  schema, contact-phase detection, gait derivation, and the specified cadence
  formula and inclusive bounds.
- Blender integration: each real target, wrong-family rejection, and injected
  structural/rest mutation negatives.
- Web tests: manifest-derived parity, playback-rate calculation and clamping,
  idle/walk transitions, and missing, non-finite, non-positive metadata or clip
  degradation.

Visual review is a human acceptance gate, not a claim that CI understands
anatomy.

## Provenance

- `rpg-game-assets/scripts/retarget_locomotion_common.py`
- `rpg-game-assets/scripts/retarget_walk_multi.py`
- `rpg-game-assets/scripts/configs/animation/polygon_to_rivals_bones.json`
- `rpg-game-assets/scripts/configs/animation/bigrig_family_rest.json`
- `rpg-game-assets/scripts/animation_qa.py`
- `rpg-dnd5e-web/src/components/hex-grid/{ClassCharacterModel.tsx,classCharacterModels.ts,useHexMovePath.ts,hexMath.ts}`
