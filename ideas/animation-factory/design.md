# Animation Factory

## Status: Approved conversational design, awaiting written-spec review (2026-07-23)

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
