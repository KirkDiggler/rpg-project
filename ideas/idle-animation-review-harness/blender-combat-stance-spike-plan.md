# Blender Combat Stance Spike Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development to execute this single local-artifact task. The user has already selected subagent-driven execution; preserve the local review worktree and do not ask again. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build and verify a local Blender 5.0.1 scene that lets Kirk judge native Polygon one-handed sword idle stances before any production retargeting.

**Architecture:** A verifier is written first and opens the saved `.blend` to assert the review contract. A separate builder imports the native character and sword, copies native actions from one-at-a-time FBX imports onto the native rig, removes only temporary imported objects, creates review presentation, then saves the licensed local artifact. No web, game, export, retarget, or production Fighter work belongs here.

**Tech Stack:** Blender 5.0.1 (`blender`), Blender Python API, bundled `io_scene_fbx.import_fbx.load`, local Synty licensed FBX source files.

## Global Constraints

- Canonical design and plan tracking are on `rpg-project#113`; this task creates no git branch,
  issue, or commit for its local artifact.
- Never commit `/home/kirk/game-dev/assets/synty/review/combat-stances/`, its `.blend`, its Python
  scripts, preview PNGs, Synty source FBXs, or converted licensed assets.
- Source root: `/home/kirk/game-dev/assets/synty/animation-sword-combat/SourceFiles`.
- Native visible character: `Models/PolygonSyntyCharacter.fbx`; use `Models/POLYGONRig_01.fbx`
  only when the character FBX cannot supply a compatible visible armature and mesh.
- Sword: `Models/SM_Wep_Sword_01.fbx` as a separate object attached to an inspected native
  prop/hand bone.
- Actions use non-root-motion Polygon files:
  - `Animations/Polygon/Idle/Base/A_Idle_Base_Sword.fbx`
  - `Animations/Polygon/Idle/Fidgets/A_Idle_EnergeticStance01_Sword.fbx`
  - `Animations/Polygon/Idle/Menacing01/A_Idle_Menacing01_Begin_Sword.fbx`
  - `Animations/Polygon/Idle/Menacing01/A_Idle_Menacing01_Sword.fbx`
  - `Animations/Polygon/Idle/Menacing01/A_Idle_Menacing01_End_Sword.fbx`
- Required local directory: `/home/kirk/game-dev/assets/synty/review/combat-stances/`.
- Do not retarget, edit character/rig/action transforms or F-curves, make a game GLB, infer
  two-handed/shield behavior, or add a production Fighter. The separate sword's local grip
  transform is the sole allowed transform adjustment. If imported action bone names differ from
  the native rig exactly, stop with `BLOCKED` and report the differing name sets; never silently
  remap.

---

### Task 1: Build And Verify The Local Native Combat-Stance Review Scene

**Files:**
- Create locally: `/home/kirk/game-dev/assets/synty/review/combat-stances/verify_review_scene.py`
- Create locally: `/home/kirk/game-dev/assets/synty/review/combat-stances/build_review_scene.py`
- Create locally: `/home/kirk/game-dev/assets/synty/review/combat-stances/combat-stance-review.blend`
- Create locally: `/home/kirk/game-dev/assets/synty/review/combat-stances/previews/FRONT-A_Idle_Base_Sword.png`
- Create locally: `/home/kirk/game-dev/assets/synty/review/combat-stances/previews/SIDE-A_Idle_EnergeticStance01_Sword.png`
- Create locally: `/home/kirk/game-dev/assets/synty/review/combat-stances/previews/THREE_QUARTER-A_Idle_Menacing01_Sword.png`

**Interfaces:**

```text
verify_review_scene.py <blend-path> <preview-dir>
  opens <blend-path>, prints REVIEW_SCENE_RED: <reason> or REVIEW_SCENE_GREEN,
  and exits 1 or 0 respectively.

build_review_scene.py <output-blend-path>
  imports only the declared native source FBXs, prints BUILD_BLOCKED: <reason> on a
  native-rig/action incompatibility, otherwise saves <output-blend-path> and exits 0.
```

- [ ] **Step 1: Confirm Blender and source inputs before writing local scripts**

Run:

```bash
ROOT="/home/kirk/game-dev/assets/synty/animation-sword-combat/SourceFiles"
OUT="/home/kirk/game-dev/assets/synty/review/combat-stances"
blender --version
blender --background --python-expr "import bpy; from io_scene_fbx import import_fbx; assert bpy.app.version == (5, 0, 1); bpy.ops.wm.read_factory_settings(use_empty=True); import_fbx.load(bpy.context, filepath='$ROOT/Models/PolygonSyntyCharacter.fbx', use_manual_orientation=False); assert any(o.type == 'ARMATURE' for o in bpy.context.scene.objects); assert any(o.name == 'SK_DUMMY_POLYGON_01' for o in bpy.context.scene.objects); print('DIRECT_FBX_LOAD_READY')"
test -f "$ROOT/Models/PolygonSyntyCharacter.fbx"
test -f "$ROOT/Models/SM_Wep_Sword_01.fbx"
test -f "$ROOT/Animations/Polygon/Idle/Base/A_Idle_Base_Sword.fbx"
test -f "$ROOT/Animations/Polygon/Idle/Fidgets/A_Idle_EnergeticStance01_Sword.fbx"
test -f "$ROOT/Animations/Polygon/Idle/Menacing01/A_Idle_Menacing01_Begin_Sword.fbx"
test -f "$ROOT/Animations/Polygon/Idle/Menacing01/A_Idle_Menacing01_Sword.fbx"
test -f "$ROOT/Animations/Polygon/Idle/Menacing01/A_Idle_Menacing01_End_Sword.fbx"
mkdir -p "$OUT/previews"
```

Expected: Blender reports `5.0.1` and `DIRECT_FBX_LOAD_READY`; all seven FBX paths exist. The
direct loader is required because the exposed `bpy.ops.import_scene.fbx`/`bpy.ops.wm.fbx_import`
wrappers are broken in this Blender 5.0.1 installation. The Menacing files above are the
discovered native Polygon Begin, Loop, and End inputs; do not choose the similarly named
root-motion files.

- [ ] **Step 2: Write the verifier first**

Create `verify_review_scene.py` with these exact checks and exit contract:

```python
import sys
from pathlib import Path
import bpy

BLEND = Path(sys.argv[-2])
PREVIEWS = Path(sys.argv[-1])
REQUIRED_ACTIONS = {
    "A_Idle_Base_Sword", "A_Idle_EnergeticStance01_Sword",
    "A_Idle_Menacing01_Begin_Sword", "A_Idle_Menacing01_Sword",
    "A_Idle_Menacing01_End_Sword",
}
REQUIRED_CAMERAS = {"FRONT", "SIDE", "THREE_QUARTER"}

def red(message):
    print(f"REVIEW_SCENE_RED: {message}")
    raise SystemExit(1)

if not BLEND.is_file():
    red(f"missing blend: {BLEND}")
bpy.ops.wm.open_mainfile(filepath=str(BLEND))
armatures = [obj for obj in bpy.data.objects if obj.type == "ARMATURE" and obj.name == "NATIVE_POLYGON_RIG"]
meshes = [obj for obj in bpy.data.objects if obj.type == "MESH" and obj.name.startswith("NATIVE_POLYGON_")]
swords = [obj for obj in bpy.data.objects if obj.type == "MESH" and obj.name == "NATIVE_SWORD"]
if len(armatures) != 1 or not meshes or len(swords) != 1:
    red("expected one native rig, native visible mesh, and separate native sword")
if swords[0].parent != armatures[0] or swords[0].parent_type != "BONE" or not swords[0].parent_bone:
    red("sword is not parented to a native rig bone")
if not REQUIRED_ACTIONS.issubset(bpy.data.actions.keys()):
    red(f"missing actions: {sorted(REQUIRED_ACTIONS - set(bpy.data.actions.keys()))}")
if REQUIRED_CAMERAS != {obj.name for obj in bpy.data.objects if obj.type == "CAMERA"}:
    red("review cameras must be exactly FRONT, SIDE, THREE_QUARTER")
if bpy.context.scene.render.fps != 30 or bpy.context.scene.frame_start != 1:
    red("expected 30 FPS and frame start 1")
if any("FIGHTER" in obj.name.upper() or "RETARGET" in obj.name.upper() for obj in bpy.data.objects):
    red("production Fighter or retarget content is forbidden")
print("REVIEW_SCENE_GREEN")
```

The finished verifier defines `action_fcurves(action)` as
`action.layers[0].strips[0].channelbag(action.slots[0]).fcurves`; Blender 5 actions are slotted
and layered, so it must not use legacy `action.fcurves`. For each required action it asserts one
slot/layer/strip/channelbag, exactly 520 fcurves, a positive frame range, and boolean custom
metadata `action['review_loop']`. It requires `False` for Begin/End and `True` for
Base/Energetic/Menacing Loop. It also asserts `NATIVE_POLYGON_RIG` has no NLA tracks, collections
are `NATIVE_REFERENCE`, `REVIEW_ENVIRONMENT`, and `REVIEW_CAMERAS`, and the ground mesh is named
`REVIEW_GROUND`.

The verifier requires `NATIVE_SWORD.parent_bone == 'Prop_R'`, all three custom properties
`grip_position`, `grip_rotation_euler`, and `grip_scale`, and a sword origin near the evaluated
world-space head of `NATIVE_POLYGON_RIG.pose.bones['Prop_R']`; it rejects a sword remaining at the
world origin or near the rig feet.

- [ ] **Step 3: Run the verifier RED**

Run:

```bash
OUT="/home/kirk/game-dev/assets/synty/review/combat-stances"
blender --background --python "$OUT/verify_review_scene.py" -- "$OUT/combat-stance-review.blend" "$OUT/previews"
```

Expected: exit 1 and `REVIEW_SCENE_RED: missing blend:`. Do not create an empty `.blend` to bypass
this failure.

- [ ] **Step 4: Write the builder with native-only import and compatibility gates**

Create `build_review_scene.py`. Define these functions in the file:

```python
def import_fbx(path: Path) -> list[bpy.types.Object]:
    """Call import_fbx.load(bpy.context, filepath=str(path), use_manual_orientation=False) and return only objects added by that call."""

def one_armature(objects: list[bpy.types.Object], source: Path) -> bpy.types.Object:
    """Return the one imported armature or raise RuntimeError naming source and candidates."""

def visible_meshes(objects: list[bpy.types.Object]) -> list[bpy.types.Object]:
    """Return imported mesh objects, excluding none; caller links them to NATIVE_REFERENCE."""

def copy_action(source_armature: bpy.types.Object, native_rig: bpy.types.Object, action_name: str) -> None:
    """Require exact 52-name pose-bone equality, copy the source slotted/layered action, rename it, fake-user it, set review_loop metadata, and assign it to native_rig."""

def attach_sword(sword: bpy.types.Object, native_rig: bpy.types.Object) -> None:
    """Require Prop_R, bone-parent sword there, start its local grip at identity, and store local grip values as custom properties."""

def remove_temporary(objects: list[bpy.types.Object]) -> None:
    """Unlink and remove only imported temporary objects after copied actions have fake users."""
```

Use this fixed configuration:

```python
ROOT = Path("/home/kirk/game-dev/assets/synty/animation-sword-combat/SourceFiles")
CHARACTER = ROOT / "Models/PolygonSyntyCharacter.fbx"
SWORD = ROOT / "Models/SM_Wep_Sword_01.fbx"
ACTION_FILES = [
    (ROOT / "Animations/Polygon/Idle/Base/A_Idle_Base_Sword.fbx", "A_Idle_Base_Sword", True),
    (ROOT / "Animations/Polygon/Idle/Fidgets/A_Idle_EnergeticStance01_Sword.fbx", "A_Idle_EnergeticStance01_Sword", True),
    (ROOT / "Animations/Polygon/Idle/Menacing01/A_Idle_Menacing01_Begin_Sword.fbx", "A_Idle_Menacing01_Begin_Sword", False),
    (ROOT / "Animations/Polygon/Idle/Menacing01/A_Idle_Menacing01_Sword.fbx", "A_Idle_Menacing01_Sword", True),
    (ROOT / "Animations/Polygon/Idle/Menacing01/A_Idle_Menacing01_End_Sword.fbx", "A_Idle_Menacing01_End_Sword", False),
]
```

Builder order:

1. Factory-reset Blender and create `NATIVE_REFERENCE`, `REVIEW_ENVIRONMENT`, and
   `REVIEW_CAMERAS` collections.
2. `import_fbx` calls `from io_scene_fbx import import_fbx` then
   `import_fbx.load(bpy.context, filepath=str(path), use_manual_orientation=False)`; do not call
   either broken Blender wrapper. Import `CHARACTER`; require one 52-bone armature and visible mesh
   `SK_DUMMY_POLYGON_01`. If either is absent, retry only with
   `ROOT / "Models/POLYGONRig_01.fbx"`; if that does not provide one visible mesh and compatible
   armature, print `BUILD_BLOCKED: no compatible visible native rig` and exit 1. Rename the
   retained armature `NATIVE_POLYGON_RIG` and retained meshes with `NATIVE_POLYGON_`.
3. Import `SWORD`, require one mesh, rename it `NATIVE_SWORD`, require authored bone `Prop_R`
   (child of `Hand_R`), and call `attach_sword`. It must bone-parent the sword to `Prop_R`, set
   local position `[0, 0, 0]`, rotation `[0, 0, 0]`, and scale `[1, 1, 1]` because this separate
   weapon is authored for the prop socket, and write those final values to `grip_position`,
   `grip_rotation_euler`, and `grip_scale`. If identity is visibly wrong, tune only those local
   sword values, rewrite the three properties, and render a preview to validate the grip. If
   `Prop_R` is absent, print `BUILD_BLOCKED: missing Prop_R; available=` followed by comma-joined
   sorted native bone names and exit 1.
4. For each `ACTION_FILES` entry, import only that FBX; identify its one temporary armature;
   compare its exact 52 `pose.bones` names to native rig names. Each verified source action has
   520 curves under `source_action.layers[0].strips[0].channelbag(source_action.slots[0]).fcurves`.
   On any difference print `BUILD_BLOCKED: action=` followed by the absolute FBX path,
   `missing_on_native=` followed by comma-joined names, and `extra_on_native=` followed by
   comma-joined names, then exit 1. Otherwise copy the namespaced imported action (for example,
   `Armature|A_Idle_Base_Sword|BaseLayer`), rename the copy exactly to the configured review name,
   set `use_fake_user = True`, set `action['review_loop']` from the configured boolean, and assign
   it to the native rig. Do not create NLA strips or tracks; Action Editor review needs one clean
   active action. Remove all temporary imported objects and armatures only after preserving the
   copied action.
5. Create `REVIEW_GROUND`, cameras named `FRONT`, `SIDE`, `THREE_QUARTER`, a neutral world, 30 FPS,
   frame start 1, sensible clip distances, and a review-resolution render setting. Do not alter
   native character/rig transforms or action keyframes; only the separate sword local grip may be
   tuned as described above.
6. Save exactly `combat-stance-review.blend` at the command-line output path.

- [ ] **Step 5: Build and run the verifier GREEN**

Run:

```bash
OUT="/home/kirk/game-dev/assets/synty/review/combat-stances"
blender --background --python "$OUT/build_review_scene.py" -- "$OUT/combat-stance-review.blend"
blender --background --python "$OUT/verify_review_scene.py" -- "$OUT/combat-stance-review.blend" "$OUT/previews"
```

Expected: builder exits 0 after saving the exact `.blend`; verifier exits 0 and prints
`REVIEW_SCENE_GREEN`. A `BUILD_BLOCKED:` result is a valid stop condition requiring Kirk's asset
decision, not a reason to remap bones or edit authored animation.

- [ ] **Step 6: Independently inspect saved scene and create local previews**

Run these Blender commands after GREEN. The inline script opens the saved scene, assigns the named
action to `NATIVE_POLYGON_RIG`, sets the useful middle frame
`round((frame_start + frame_end) / 2)`, selects each review camera, asserts character and sword
are visible, and writes the three exact local PNG paths:

```bash
OUT="/home/kirk/game-dev/assets/synty/review/combat-stances"
blender --background --python-expr "import bpy; bpy.ops.wm.open_mainfile(filepath='$OUT/combat-stance-review.blend'); print([(o.name, o.type) for o in bpy.context.scene.objects]); print(sorted(bpy.data.actions.keys()))"
blender --background "$OUT/combat-stance-review.blend" --python-expr "import bpy, os; out='$OUT/previews'; shots=[('FRONT','A_Idle_Base_Sword','FRONT-A_Idle_Base_Sword.png'),('SIDE','A_Idle_EnergeticStance01_Sword','SIDE-A_Idle_EnergeticStance01_Sword.png'),('THREE_QUARTER','A_Idle_Menacing01_Sword','THREE_QUARTER-A_Idle_Menacing01_Sword.png')]; rig=bpy.data.objects['NATIVE_POLYGON_RIG']; sword=bpy.data.objects['NATIVE_SWORD']; assert any(o.name.startswith('NATIVE_POLYGON_') and o.visible_render for o in bpy.context.scene.objects) and sword.visible_render; rig.animation_data_create(); [setattr(rig.animation_data, 'action', bpy.data.actions[action]) or setattr(bpy.context.scene, 'camera', bpy.data.objects[camera]) or setattr(bpy.context.scene, 'frame_current', round(sum(bpy.data.actions[action].frame_range) / 2)) or setattr(bpy.context.scene.render, 'filepath', os.path.join(out, filename)) or bpy.ops.render.render(write_still=True) for camera, action, filename in shots]"
```

Expected: console scene listing includes `NATIVE_POLYGON_RIG`, `NATIVE_SWORD`, `REVIEW_GROUND`,
`FRONT`, `SIDE`, and `THREE_QUARTER`; action listing includes all five exact action names; previews
exist at the three paths in this task's Files list.

- [ ] **Step 7: Hand off the local review to Kirk without git work**

Run:

```bash
blender /home/kirk/game-dev/assets/synty/review/combat-stances/combat-stance-review.blend
```

In Blender, select `NATIVE_POLYGON_RIG`, open Dope Sheet > Action Editor, choose any named action,
and press Space to play. Use `FRONT`, `SIDE`, and `THREE_QUARTER` cameras to make the native
one-handed stance decision. Do not stage, commit, push, or copy any file from the local output
directory; do not disturb the parked `rpg-dnd5e-web` worktree.

---

## Plan Self-Review

- Spec coverage: local output, licensed boundaries, native character/sword, exact five actions,
  camera/ground presentation, Action Editor use, one-handed-first scope, and Kirk's next decision
  are covered by Task 1.
- Paths and filenames: all source, output, preview, Blender, and Menacing Begin/Loop/End paths are
  explicit; Blender importer verification is the first task step.
- Compatibility: Blender 5.0.1 uses the direct bundled importer and slotted/layered action API;
  the broken FBX operator wrappers and legacy `action.fcurves` are excluded.
- Command validity: Steps 1, 3, and 5 invoke Blender with `--background --python` and explicit
  script plus argument paths; Step 6 uses Blender's documented `--python-expr`; the GUI command
  opens the exact local `.blend`.
- Scope: no licensed/local artifact is committed. This plan does not change the approved eventual
  Concepts Lab showcase behavior and does not implement web work.
