# Blender Fighter Menacing Review Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development to execute this one local-artifact task. The user selected subagent-driven execution; do not ask again. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Create and verify a local Blender comparison scene with the accepted native Menacing loop on the left and a Big-Rig production Fighter retarget on the right, including a repairable closed right weapon hand.

**Architecture:** The existing native `combat-stance-review.blend` remains read-only. A local retarget CLI produces one temporary Fighter GLB; a separate builder creates a new side-by-side review `.blend`; a verifier is written first and enforces the structural, action, placement, hand, weapon, and no-NLA contract. No result becomes a game asset until Kirk approves the visual repair.

**Tech Stack:** Blender 5.0.1 (`blender`), Blender Python API, bundled `io_scene_fbx.import_fbx.load`, `rpg-game-assets/scripts/retarget_idle_multi.py`, Blender MCP visual inspection.

## Global Constraints

- Canonical tracking remains `rpg-project#113`; local licensed artifacts create no git branch,
  issue, or commit. Do not disturb the parked web worktree.
- Never commit `/home/kirk/game-dev/assets/synty/review/combat-stances/`, local Python scripts,
  temporary GLBs, `.blend` files, preview PNGs, or licensed Synty source assets.
- Preserve `/home/kirk/game-dev/assets/synty/review/combat-stances/combat-stance-review.blend`
  unchanged. The new review scene is a separate file.
- Native source action:
  `/home/kirk/game-dev/assets/synty/animation-sword-combat/SourceFiles/Animations/Polygon/Idle/Menacing01/A_Idle_Menacing01_Sword.fbx`.
- Production source:
  `/home/kirk/game-dev/rpg-game-assets/harness/models/synty/characters/fighter.glb`.
- Retarget script:
  `/home/kirk/game-dev/rpg-game-assets/scripts/retarget_idle_multi.py`.
- Mapping:
  `/home/kirk/game-dev/rpg-game-assets/scripts/configs/animation/polygon_to_rivals_bones.json`.
- Big-Rig rest table:
  `/home/kirk/game-dev/rpg-game-assets/scripts/configs/animation/bigrig_family_rest.json`.
- The production skinned weapon is `SM_Wep_Slayer_01`, weighted to `Hand_R`; do not replace it.
- Do not change the native reference, source action, Fighter character transforms, source keyframes,
  game GLBs, manifests, or Concepts Lab. The only animation edit is the constant local quaternion
  closure delta on target right finger bones at every Menacing keyed frame.
- Stop with `BLOCKED` on Root convention, Big-Rig rest verification, retarget, imported-action,
  production weapon, or target-finger incompatibility. Do not silently substitute an asset, rig,
  mapping, root convention, or weapon.

---

### Task 1: Retarget, Repair, And Verify The Local Fighter Menacing Comparison

**Files:**
- Read only: `/home/kirk/game-dev/assets/synty/review/combat-stances/combat-stance-review.blend`
- Create locally: `/home/kirk/game-dev/assets/synty/review/combat-stances/tmp/fighter-menacing-retarget.glb`
- Create locally: `/home/kirk/game-dev/assets/synty/review/combat-stances/build_fighter_menacing_review.py`
- Create locally: `/home/kirk/game-dev/assets/synty/review/combat-stances/verify_fighter_menacing_review.py`
- Create locally: `/home/kirk/game-dev/assets/synty/review/combat-stances/fighter-menacing-review.blend`
- Create locally: `/home/kirk/game-dev/assets/synty/review/combat-stances/previews/fighter-menacing/`

**Interfaces:**

```text
verify_fighter_menacing_review.py receives the review blend path followed by the preview directory.
  It opens the review blend, prints FIGHTER_MENACING_RED followed by a concrete failure message or
  FIGHTER_MENACING_GREEN, and exits 1 or 0.

build_fighter_menacing_review.py receives the native review blend path, temporary Fighter retarget
  GLB path, and output review blend path. It reads the native blend without saving it, imports the
  temporary retarget GLB, prints FIGHTER_MENACING_BLOCKED followed by a concrete incompatibility
  message, otherwise saves the output review blend and exits 0.
```

- [ ] **Step 1: Confirm exact inputs and produce the local temporary retarget GLB**

Run:

```bash
ROOT="/home/kirk/game-dev/assets/synty/review/combat-stances"
ASSETS="/home/kirk/game-dev/rpg-game-assets"
SOURCE="/home/kirk/game-dev/assets/synty/animation-sword-combat/SourceFiles/Animations/Polygon/Idle/Menacing01/A_Idle_Menacing01_Sword.fbx"
CHARACTER="$ASSETS/harness/models/synty/characters/fighter.glb"
RETARGET="$ASSETS/scripts/retarget_idle_multi.py"
MAPPING="$ASSETS/scripts/configs/animation/polygon_to_rivals_bones.json"
BIGRIG_REST="$ASSETS/scripts/configs/animation/bigrig_family_rest.json"
OUT="$ROOT/tmp/fighter-menacing-retarget.glb"
test -f "$ROOT/combat-stance-review.blend"
test -f "$SOURCE"
test -f "$CHARACTER"
test -f "$RETARGET"
test -f "$MAPPING"
test -f "$BIGRIG_REST"
mkdir -p "$ROOT/tmp" "$ROOT/previews/fighter-menacing"
blender --background --python "$RETARGET" -- --character "$CHARACTER" --mapping "$MAPPING" --bigrig-rest-table "$BIGRIG_REST" --rig-family bigrig --out "$OUT" --clip "Idle_Menacing01_Sword=$SOURCE"
```

Expected: the CLI verifies the Fighter as Big-Rig, writes only
`tmp/fighter-menacing-retarget.glb`, and embeds `Idle_Menacing01_Sword`. Any CLI failure is
`FIGHTER_MENACING_BLOCKED: retarget CLI failed` with the command output retained locally; do not
attempt an alternative mapper or root workaround.

- [ ] **Step 2: Write the verifier first and run RED against the missing review blend**

Create `verify_fighter_menacing_review.py` with these names and assertions:

```python
import sys
from pathlib import Path
import bpy

BLEND = Path(sys.argv[-2])
PREVIEWS = Path(sys.argv[-1])
FINGER_CHAINS = {
    "right_thumb": ("thumb_01_r", "thumb_02_r", "thumb_03_r"),
    "right_index": ("indexFinger_01_r", "indexFinger_02_r", "indexFinger_03_r", "indexFinger_04_r"),
    "right_fingers": ("finger_01_r", "finger_02_r", "finger_03_r", "finger_04_r"),
}

def red(message):
    print(f"FIGHTER_MENACING_RED: {message}")
    raise SystemExit(1)

if not BLEND.is_file():
    red(f"missing blend: {BLEND}")
bpy.ops.wm.open_mainfile(filepath=str(BLEND))
native = bpy.data.objects.get("NATIVE_POLYGON_RIG")
fighter = bpy.data.objects.get("FIGHTER_BIGRIG")
weapon = bpy.data.objects.get("FIGHTER_SM_Wep_Slayer_01")
action = bpy.data.actions.get("Idle_Menacing01_Sword")
if not native or not fighter or not weapon or not action:
    red("missing native reference, Fighter Big-Rig, Slayer weapon, or Menacing action")
if "Root" not in fighter.pose.bones or "Pelvis" not in fighter.pose.bones:
    red("Fighter Root convention requires Root and Pelvis")
if weapon.parent != fighter or weapon.parent_bone != "Hand_R":
    red("SM_Wep_Slayer_01 is not skinned/parented to Hand_R")
if action.frame_range[0] > 42 or action.frame_range[1] < 92 or not bool(action.get("review_loop")):
    red("Menacing action does not cover frames 42-92 with review_loop metadata")
if fighter.animation_data and fighter.animation_data.nla_tracks:
    red("Fighter NLA contamination is forbidden")
if any(not fighter.pose.bones[name] for chain in FINGER_CHAINS.values() for name in chain):
    red("required target right finger bone is absent")
if not all(key in action for key in ("closure_right_thumb", "closure_right_index", "closure_right_fingers")):
    red("missing closure quaternion metadata")
if abs(native.location.x - fighter.location.x) < 2.5 or abs(native.location.x - fighter.location.x) > 3.0:
    red("native and Fighter must be 2.5-3.0m apart")
if {"FRONT", "SIDE", "THREE_QUARTER", "HAND_CLOSE_FRONT", "HAND_CLOSE_SIDE"} - set(bpy.data.objects.keys()):
    red("missing comparison cameras")
print("FIGHTER_MENACING_GREEN")
```

Extend the verifier to require the original native reference collection and its native Menacing
action unchanged by saved action name, frame range, and collection object names; the Fighter
collection `FIGHTER_REVIEW` must be separate. Require a positive action range, action keyframes at
42, 67, and 92 for every target closure bone, no weapon mesh replacement, and camera/ground
collections. The verifier checks that action custom properties contain the final per-chain
quaternion constants and that the closure delta is identical at every keyed frame, including first
and last, preserving the loop seam.

Run RED:

```bash
ROOT="/home/kirk/game-dev/assets/synty/review/combat-stances"
blender --background --python "$ROOT/verify_fighter_menacing_review.py" -- "$ROOT/fighter-menacing-review.blend" "$ROOT/previews/fighter-menacing"
```

Expected: exit 1 and `FIGHTER_MENACING_RED: missing blend:`.

- [ ] **Step 3: Build the separate side-by-side review scene and closed-grip action**

Create `build_fighter_menacing_review.py` with these exact responsibilities:

```python
RIGHT_THUMB_CLOSURE_DELTA = (0.980785, 0.0, 0.19509, 0.0)
RIGHT_INDEX_CLOSURE_DELTA = (0.965926, 0.0, 0.258819, 0.0)
RIGHT_FINGERS_CLOSURE_DELTA = (0.965926, 0.0, 0.258819, 0.0)
FINGER_CHAINS = {
    "right_thumb": ("thumb_01_r", "thumb_02_r", "thumb_03_r"),
    "right_index": ("indexFinger_01_r", "indexFinger_02_r", "indexFinger_03_r", "indexFinger_04_r"),
    "right_fingers": ("finger_01_r", "finger_02_r", "finger_03_r", "finger_04_r"),
}

def import_glb(path: Path) -> list[bpy.types.Object]:
    """Import path with bpy.ops.import_scene.gltf and return the newly added objects."""

def require_fighter_root(rig: bpy.types.Object) -> None:
    """Require Root, Pelvis, Hand_R, all FINGER_CHAINS bones, and Big-Rig collection membership; otherwise raise RuntimeError with FIGHTER_MENACING_BLOCKED text."""

def copy_native_reference(native_blend: Path) -> tuple[bpy.types.Object, bpy.types.Collection]:
    """Append the native reference collection from native_blend without saving or mutating native_blend."""

def apply_constant_closure(action: bpy.types.Action, rig: bpy.types.Object) -> None:
    """At every existing action keyed frame, including first and last, multiply each target right finger local rotation by its named chain quaternion and write constant closure metadata to action and rig."""

def verify_keyed_closure_seam(action: bpy.types.Action, rig: bpy.types.Object) -> None:
    """Require equal closure deltas at first and last action frames for every target closure bone."""
```

Builder order:

1. Start a new scene, append the native reference collection from the original local blend, and
   never save to or alter that original file. Put native reference at X=-1.4m and preserve its
   Menacing action and source collection contents.
2. Import `tmp/fighter-menacing-retarget.glb`; identify its sole character armature, rename it
   `FIGHTER_BIGRIG`, place it at X=+1.4m, and link it plus its meshes to `FIGHTER_REVIEW`. Require
   `Root`, `Pelvis`, `Hand_R`, all declared right thumb/index/general finger bones, and exact Big-Rig
   rest compatibility. On failure print `FIGHTER_MENACING_BLOCKED: target Root or Big-Rig structure
   incompatible` and exit 1.
3. Require the imported production skinned weapon mesh named `SM_Wep_Slayer_01`, rename only its
   object to `FIGHTER_SM_Wep_Slayer_01`, retain its existing `Hand_R` skinning/parent relationship,
   and do not import or substitute a sword.
4. Find the imported `Idle_Menacing01_Sword` action, set fake user and
   `action['review_loop'] = True`, clear any NLA tracks, and assign it as the sole active Fighter
   action. Source fingers are neutral/open: call `apply_constant_closure` on every action keyed
   frame, including frames 42, 67, and 92 plus first/last. Do not edit native action curves.
5. Store `closure_right_thumb`, `closure_right_index`, and `closure_right_fingers` as four-float
   custom properties on both the Fighter action and `FIGHTER_BIGRIG`. Call
   `verify_keyed_closure_seam`; a mismatch prints `FIGHTER_MENACING_BLOCKED: closure loop seam`
   and exits 1.
6. Create shared ground and `FRONT`, `SIDE`, `THREE_QUARTER`, `HAND_CLOSE_FRONT`, and
   `HAND_CLOSE_SIDE` cameras. Frame both characters for comparison; aim hand cameras at the Fighter
   right weapon hand. Set 30 FPS and save only
   `fighter-menacing-review.blend`.

Use Blender MCP viewport/render inspection on the native and target at frames 42, 67, and 92.
Tune only the three named closure quaternion constants if the right hand remains open, misses the
weapon, or penetrates it; rerun the builder after each constant change. Do not alter native
reference, Fighter transforms, retarget mapping, weapon, or game assets.

- [ ] **Step 4: Run GREEN, render QA evidence, and hand off visual decision**

Run:

```bash
ROOT="/home/kirk/game-dev/assets/synty/review/combat-stances"
blender --background --python "$ROOT/build_fighter_menacing_review.py" -- "$ROOT/combat-stance-review.blend" "$ROOT/tmp/fighter-menacing-retarget.glb" "$ROOT/fighter-menacing-review.blend"
blender --background --python "$ROOT/verify_fighter_menacing_review.py" -- "$ROOT/fighter-menacing-review.blend" "$ROOT/previews/fighter-menacing"
blender --background "$ROOT/fighter-menacing-review.blend" --python-expr "import bpy, os; out='$ROOT/previews/fighter-menacing'; rig=bpy.data.objects['FIGHTER_BIGRIG']; action=bpy.data.actions['Idle_Menacing01_Sword']; rig.animation_data_create(); rig.animation_data.action=action; shots=[(camera, frame, f'{camera}-frame-{frame}.png') for camera in ('FRONT','SIDE','THREE_QUARTER','HAND_CLOSE_FRONT','HAND_CLOSE_SIDE') for frame in (42,67,92)]; [setattr(bpy.context.scene,'camera',bpy.data.objects[camera]) or setattr(bpy.context.scene,'frame_current',frame) or setattr(bpy.context.scene.render,'filepath',os.path.join(out,filename)) or bpy.ops.render.render(write_still=True) for camera,frame,filename in shots]"
blender "$ROOT/fighter-menacing-review.blend"
```

Expected: builder and verifier exit 0; verifier prints `FIGHTER_MENACING_GREEN`; 15 local PNGs
exist under `previews/fighter-menacing/`; front, side, three-quarter, and both hand close-ups show
frames 42, 67, and 92 with a closed right hand and the existing Slayer weapon. Use Blender MCP to
inspect weapon intersection and hand closure in those images and the viewport. If
`retarget_idle_multi.py` exposes local seam/drift QA through its imported shared core, run that
local QA against `tmp/fighter-menacing-retarget.glb`; if it reports a failure, stop BLOCKED and
record its output locally.

For Kirk's GUI handoff, select `FIGHTER_BIGRIG`, open Dope Sheet > Action Editor, select
`Idle_Menacing01_Sword`, and play the loop. Keep the native reference visible at left and the
Fighter visible at right. Do not stage, commit, push, copy, or sync any local review artifact.

---

## Plan Self-Review

- Coverage: selected frames 42-92, native preservation, Big-Rig retarget CLI, Root convention,
  production Slayer weapon, action loop, closed right hand, side-by-side layout, cameras, renders,
  MCP inspection, RED/GREEN verification, and GUI handoff are covered by Task 1.
- Paths: all source, script, mapping, rest-table, temporary GLB, review blend, script, and preview
  paths are absolute and explicit.
- Scope: no game GLB, manifest, production asset, or web change is authorized; all outputs are
  licensed local artifacts.
- Compatibility: structural/root/retarget incompatibility is BLOCKED with no silent workaround;
  closure tuning affects only declared target finger local rotations and preserves first/last seam.
