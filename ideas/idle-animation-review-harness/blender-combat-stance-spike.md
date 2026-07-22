# Blender Combat Stance Spike

## Purpose

Establish Synty's authored one-handed combat-stance truth in Blender before any retargeting,
repair, export, or Concepts Lab showcase work. This changes execution order only: the approved
eventual showcase remains the final staging gallery for every candidate and shipped animation,
including clips not wired into production.

## Immediate Deliverable And Licensing

The immediate deliverable is the local licensed Blender review file:

`/home/kirk/game-dev/assets/synty/review/combat-stances/combat-stance-review.blend`

Do not commit the `.blend`, Synty source files, or converted licensed assets. The file remains a
local review artifact under the licensed asset workspace.

## First Scene

Create a clearly named native-reference scene and collections that preserve authored truth before
retargeting:

- Native Polygon reference character on its authored rig.
- Separate `SM_Wep_Sword_01.fbx`, attached correctly to the native character.
- Ground and front, side, and three-quarter review cameras.
- Clearly named actions: `A_Idle_Base_Sword`, `A_Idle_EnergeticStance01_Sword`, and Menacing
  Begin, Loop, and End actions.

Kirk can switch and play these actions in Blender's Action Editor. The scene collections and
actions use descriptive names so the authored character, sword, cameras, and each action are
unambiguous.

## Review Order

The first judgment is which native one-handed stance reads correctly. Only after that visual call,
add the production Fighter beside the native reference and retarget or repair the selected stance:
grip, wrist, elbows, feet, weapon alignment, and loop are reviewed together.

## Visual Decision: 2026-07-22

Selected native action: `A_Idle_Menacing01_Sword`, using its loop frames 42-92. Its body and sword
read as the accepted one-handed carry. `A_Idle_Base_Sword` is rejected as T-pose-like.
`A_Idle_EnergeticStance01_Sword` is rejected as a disco-like fidget that returns to Base. The
required repair for the selected Menacing loop is a closed right weapon hand.

The next deliverable preserves the native reference and adds the production Fighter beside it for
side-by-side Menacing-loop retargeting, grip repair, and visual judgment.

The initial stance family is one-handed melee. Large two-handed weapons may rest in one hand
during idle, so a separate two-handed idle is not required initially. True two-handed handling
remains required for attacks, blocks, and tense guards. Shield is a later left-hand or forearm
overlay variant because this pack has no shield-specific authored stance. Unarmed/staff and ranged
are later families.

Drinking remains a future signature premium idle and includes its separate cup prop. Nose-pick and
the fourth idle are later review work.

## Success Criteria

- The local review file opens in the current Blender installation.
- Native rig, model, and sword are visible at a sensible scale.
- Every listed action is selectable, plays, and loops as authored where applicable.
- Front, side, and three-quarter cameras work for visual comparison.
- No retarget, export, game, manifest, or Concepts Lab implementation change is made by this
  spike.
- Kirk can make a visual call on the native one-handed stance.

## Non-Goals

- No game-ready GLB, asset sync, production resolver wiring, or Concepts Lab implementation.
- No production game GLB or manifest change from the local Fighter retarget review.
- No separate initial two-handed idle, shield-specific stance, unarmed/staff stance, ranged
  stance, drinking animation, nose-pick animation, or fourth idle.

## Next Decision

After Kirk views the native reference scene, choose the one-handed stance to carry forward. That
decision determines whether to add the production Fighter for retargeting and repair, while the
eventual Concepts Lab showcase remains parked until current animations are repaired.
