# Unarmed Character Animation Promotion

**Date:** 2026-07-23  
**Status:** Draft  
**Scope:** Replace the four canonical standing class GLBs with approved, intentionally
unarmed models and promote their approved idle/walk animation checkpoints.  
**Primary repository:** `rpg-game-assets`

## Goal

Promote the approved unarmed checkpoints into the canonical runtime models:

- `characters/fighter.glb`
- `characters/barbarian.glb`
- `characters/monk.glb`
- `characters/rogue.glb`

The approved sources are:

```
assets/synty/animation-retarget/approved/
  fighter-unarmed-idle-walk-approved.blend
  barbarian-unarmed-idle-walk-approved.blend
  monk-unarmed-idle-walk-approved.blend
  rogue-unarmed-idle-walk-approved.blend
```

This is a checkpoint-led, direct replacement. Canonical filenames and the web resolver
remain unchanged. The new standing models are intentionally unarmed; standalone weapon
GLBs remain available for a future equipment system.

## Governing Decision

**2026-07-23:** Kirk explicitly chose intentionally unarmed runtime models. For each class,
preserve the two existing non-relaxed idles: the class-distinctive idle and `Idle_Drinking`.
Replace the old `Idle_Relaxed` with the newly approved checkpoint version and ship the newly
approved `Walk_Forward`. This supersedes prior weapon-bearing or baked-weapon assumptions for
these four standing model outputs. Standalone weapon assets remain available; future equipment
swapping remains deferred. This decision governs any conflict in this promotion.

## Runtime Contract

Each promoted base GLB contains exactly four clips: its current three manifest idle
clips, plus `Walk_Forward`. The newly approved `Idle_Relaxed` replaces only the prior
`Idle_Relaxed`; the distinctive and drinking idles are preserved without retargeting.

| Class | Exact clips |
| --- | --- |
| Fighter | `Idle_Relaxed`, `Idle_Stretch`, `Idle_Drinking`, `Walk_Forward` |
| Barbarian | `Idle_Relaxed`, `Idle_ChinScratch`, `Idle_Drinking`, `Walk_Forward` |
| Monk | `Idle_Relaxed`, `Idle_Meditative`, `Idle_Drinking`, `Walk_Forward` |
| Rogue | `Idle_Relaxed`, `Idle_CheckWatch`, `Idle_Drinking`, `Walk_Forward` |

`idleClips` remains the ordered three-idle metadata array, with `Idle_Relaxed` first,
the class-distinctive idle second, and `Idle_Drinking` last. `Walk_Forward` is embedded
in the GLB but is not an idle metadata entry. The updated manifest must record the
unarmed standing-model truth: each class weapon's `bakedIntoModel` is `false`; the
standalone weapon file and metadata remain. Update accompanying manifest/asset docs to
remove claims that a runtime character model carries a baked weapon.

Every base and color variant preserves the established Root wrapper exactly:

- Node name: `Root`.
- Float32 scale: `[0.009999999776482582, 0.009999999776482582,
  0.009999999776482582]`.
- glTF `[x, y, z, w]` X-axis 90-degree rotation:
  `[0.70710688829422, 0, 0, 0.7071066498756409]`.
- `Root` has exactly one identity-local-transform child retaining the exact source-derived
  assembly name below; that child owns the armature/body hierarchy and contains no weapon mesh.

| Class | Exact Root child assembly name |
| --- | --- |
| Fighter | `SK_BR_Character_Slayer_01` |
| Barbarian | `SK_BR_Character_BarbarianGiant_01` |
| Monk | `SK_Character_Mystic_01` |
| Rogue | `SK_Character_DarkElf_01` |

## Promotion Flow

1. Export every approved `.blend` to staging, never directly over
   `harness/models/synty/characters/<class>.glb`, and generate its staged `B`/`C`/`D` color
   GLBs with `swap_atlas.py`. Do not hand-maintain variant geometry or animations.
2. Produce a four-class contact sheet of the current 512x512 portraits and obtain an
   independent review with a recorded viewed statement. It passes only when no weapon or
   weapon fragment is visible anywhere in any frame. If a portrait fails, regenerate every
   affected class's `A`/`B`/`C`/`D` portraits in staging and repeat this same contact-sheet
   gate for the regenerated portraits.
3. Stage the updated characters manifest, weapon metadata, README/relevant asset docs, mesh
   stats report, animation QA report, and all required evidence with the staged GLBs and any
   regenerated portraits.
4. Validate the complete staged set for all four classes and every affected artifact.
5. Promote the complete validated set as one unit: all four classes' `A`/`B`/`C`/`D` standing
   GLBs, any regenerated portraits, manifest and metadata, README/docs, mesh stats report,
   animation QA report, and evidence. No class or artifact may be promoted separately.
6. Sync assets and verify the consuming web client without changing its resolver.

No canonical overwrite is permitted before complete-set staging validation completes. A
validation failure leaves the complete canonical set untouched, preserves staging artifacts
for diagnosis, and returns to the relevant approved checkpoint/export step. If a
post-promotion regression is found, restore the complete previous set as one unit: every
class's `A`/`B`/`C`/`D` standing GLBs, portraits, manifest and metadata, README/docs, mesh
stats report, animation QA report, and evidence; then re-run asset sync and client
verification. No mixed old/new class set, portraits, generated reports, or metadata is
allowed.

## Validation And Evidence

For every staged base and every A/B/C/D runtime GLB, establish:

- Exactly one armature/body assembly and no weapon mesh, weapon attachment, or visible
  weapon geometry.
- The exact four-clip set above, no extra clips, and the preserved idle metadata order.
- `Idle_Relaxed` is the approved replacement; the two non-relaxed idle clips retain
  their existing names and keyframe data.
- Identical animation clip names and keyframe counts across A/B/C/D for the same class.
- The Root wrapper node, float32 scale, glTF quaternion, one-child identity structure,
  exact class assembly name, and source-derived child ownership match the runtime contract.
- Direct glTF JSON comparison against each current canonical class file permits only weapon-node
  removal and animation payload changes; all other differences fail validation.
- `scripts/build_mesh_stats.py` and its `PROPOSED_BUDGETS` are authoritative. It must exit 0
  and emit no new warning for any changed standing model compared with the pre-promotion mesh
  stats report.
- The hard animation gates from `scripts/animation_qa.py` are authoritative and must pass in
  the animation QA report. Foot-slide remains informational and is not a promotion gate.
- The portrait contact sheet, independent recorded viewed statement, and all non-licensed
  evidence live under the existing public playtest-evidence asset-pipeline convention; no
  licensed GLB is placed there.
- Multi-angle anatomy evidence exists for both `Idle_Relaxed` and `Walk_Forward`: front,
  side, three-quarter, and back views, sufficient to detect weapons, skinning, clipping,
  and pose defects.
- Asset sync succeeds; web tests and CI pass; and the actual client demonstrates an
  idle-to-walk-to-idle transition using the promoted canonical file paths.

## Repositories Affected

`rpg-game-assets` is the only expected tracked-code/content repository: canonical GLBs,
color variants, portraits only when necessary, manifest metadata, documentation, generated
mesh-stat and animation-QA reports, and evidence live there.

`rpg-dnd5e-web` participates in asset-sync and actual-client verification only. No web
resolver or tracked web-code change is expected. If verification finds a consumer defect,
record it as a separate issue rather than expanding this promotion scope.

## Non-Goals

- Runtime equipment swapping.
- Weapon grip, socket, or placement work.
- A new client animation loader or resolver change.
- Changes to any downed GLB.
- Retargeting already shipped distinctive idles.
- Portrait regeneration unless inspection shows a weapon.

## Alternatives Considered

### Animation sidecars

Shipping `Walk_Forward` or the replacement idle as sidecar animation files would require
new resolution/loading behavior and create synchronization risk between a body GLB and
its animations. It is rejected because the existing canonical model contract already
embeds animations and direct replacement preserves it.

### Retarget all idles

Retargeting every idle from the new checkpoints would increase animation and anatomy risk
without a product need. It is rejected because the existing class-specific distinctive
idles are shipped, approved identity; only `Idle_Relaxed` and `Walk_Forward` are in the
approved promotion boundary.

## Success Criteria

The work succeeds only when all four canonical standing classes and their A/B/C/D variants
are unarmed, meet the exact Root, assembly-name, glTF-comparison, and four-clip contracts,
preserve the manifest's idle metadata and class-specific idles, and have complete staged-set
validation evidence before one-unit canonical promotion. Weapon metadata accurately says
`bakedIntoModel: false`; every retained or regenerated portrait passes the independent
weapon-free contact-sheet gate; authoritative mesh-stat and animation-QA gates pass; and
asset sync, web tests/CI, and actual-client idle/walk transition verification all pass
without a web resolver change.
