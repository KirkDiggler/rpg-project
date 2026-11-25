# The Unreal legs

Two legs ran after the headless one, on the same Linux machine, with no
Windows reboot. Both are recorded here; the code that ran is beside this file.
Evidence images are not in git: they show licensed models, and this
repository is public. The numbers below are the evidence.

## Leg 1: the record client as an Unreal module

Question: does the spike's client become a module of a real editor, and does
the record reach the game thread?

### The engine

- Unreal **5.9.0 Preview 2**, Linux installed build, 41.2 GB zipped, 81.1 GB
  and 331,716 files unzipped (exact, by the zip's own directory).
- Machine: Ubuntu 26.04.1, a Wayland session, RTX 4090 on driver 595. Epic
  asks for driver 570 or later for Vulkan and tests on Ubuntu 22.04. Both
  differences were untested territory; neither caused a problem.
- The editor starts under Vulkan with the plugin loaded, zero errors, in
  30 seconds.
- The build toolchain is **clang 20.1.8** (the documentation says 18.1.0),
  with a bundled .NET 10 SDK for the build tool. Nothing had to be installed.

### The module

A blank C++ project with one plugin, `RpgWire`: the generated protobuf and
gRPC code under `Private/pb/`, the stream class from the headless spike in
plain C++, a console command `rpg.record` that logs each beat from the game
thread. Three things the build taught, each of which changes the recipe:

1. **One module.** The first cut put the generated code in its own module.
   Both modules loaded and the first call died on `undefined symbol`:
   Unreal exports only what a module's API macro marks, generated protobuf
   classes carry no such mark. The seam between the plain-C++ core and the
   Unreal face is kept in directories, as Tumult's was.
2. **Flatten the `.cc` names.** With one file per compile action, Unreal
   names object files by source basename, and four names are shared across
   packages: `service.pb.cc` by ten, `types.pb.cc` by five, `events.pb.cc`
   and `common.pb.cc` by three each. The headers keep their package paths.
3. **The engine's build settings, unmodified.** A project on an installed
   engine shares the engine's build environment; the targets use
   `BuildSettingsVersion.V8`. The build tool also wants an absolute project
   path and the platform as a positional argument.

### Numbers

| What | Measured |
|---|---|
| Clean compile, 72 generated files plus the module | 73 s on 16 physical cores (63 s on a second run) |
| Intermediates | about 300 MB (446 MB with two modules) |
| Beat replay in the editor, headless | 31 beats logged between +510 ms and +511 ms |
| Live rounds | +19.1 s, +39.1 s, +59.1 s: the engine-side test's own 20 s cadence |
| Engine act begins to beat on the game thread | 10 to 13 ms, median 11, over 13 beats (localhost) |
| First beat's wait | about 500 ms: the editor's very first frame, which the command ran in |

The 510 ms is not the wire: the headless client on the same wire logs its
first beat at +8 ms. The editor log's frame counter shows the subscription in
frame 0 and the beats in frame 1.

One flag to understand rather than accept: Unreal 5.9 compiled 87 of 87
actions with `-fexceptions` although the module sets `bEnableExceptions =
false`, so the module's code has exceptions on against libraries built
without them. It built, linked and loaded; the rule that adds the flag was
not found.

### Still open from this leg

- Frame stability during a burst in the rendering editor.
- gRPC's threads beside the game thread: it ran for minutes without a
  problem; nothing was measured.

### Recipe

```
~/UnrealEngine/UE_5.9/Engine/Build/BatchFiles/Linux/Build.sh RpgUeProofEditor \
  ~/UnrealEngine/Projects/RpgUeProof/RpgUeProof.uproject Development Linux
```

With the engine side up (the headless spike's recipe), in the editor's
console: `rpg.record 127.0.0.1:50151 <session> <player> [member|-]`.
Headless, with the command at startup: see the project's README.
`RpgWire.Build.cs` in this folder is the module's build rules.

## Leg 2: the assets

Question: do the web's models work in Unreal, or does it need another asset
pipeline?

Answer: **the GLB files do not go in directly, and no second pipeline is
needed.** Unreal 5.9 imports FBX natively and ships only a glTF *exporter*.
The bridge is Blender, already the pipeline's workhorse: GLB in, FBX out,
then Unreal's headless Python import. The pipeline's review-and-promote flow
is untouched; this is a second exit from it.

### What the web's view is made of

- Floors and walls are **built, not imported**: hex tiles as instanced
  meshes, one world unit per hex, five feet per cell; walls generated from
  the atlas. That is code to port, not assets to convert.
- Props, environment pieces and characters are GLBs from the reviewed
  runtime tree, `rpg-game-assets/harness/models/synty/`: 858 MB, about
  1,700 files (597 characters, 599 NPCs, 314 world assets, 111 props).
- The characters are the pipeline's own, with clips the game plays by name
  (`Idle_Relaxed`, `Walk_Forward`, `Attack`).
- The camera is a perspective camera with a field-of-view dial and zoom bands.

### Sources on the machine

- The model source packs (Fantasy Kingdom, Dungeon, Dark Fortress, Goblin War
  Camp) are not on this disk; only three animation packs are. So the bridge
  starts from the finished GLBs.
- The Unreal-format pack, `POLYGON_Fantasy_Kingdom_Unreal_5_3_v1_12_0.zip`:
  0.5 GB, 2,718 `.uasset` files (70 rigged characters, 2,393 static meshes),
  no FBX, no animation clips, saved by 5.3. **It loads unchanged in 5.9** and
  served as the control.

### The bridge, and the six choices that mattered

`glb_to_fbx.py` runs in headless Blender 5.0. In this Blender build the
export operator registers only a path and two axes, so the regular
`bpy.ops.export_scene.fbx` fails on its own sixth option; the script calls
the exporter's `save()` directly with every option spelled out.

1. No leaf bones.
2. One FBX take per clip, no key simplification.
3. **30 fps, the clips' authored rate.** At Blender's default 24 fps a
   53-frame clip lands on a fractional frame and Unreal refuses it: "not
   compatible with import frame-rate".
4. Drop the display shape the glTF importer adds for bones (it exported as a
   second skinned mesh, `Icosphere`).
5. **Textures written as files beside the FBX.** Unreal's importer cannot
   read Blender's embedded ones: "Invalid translator couldn't retrieve a
   payload".
6. **Bone names capitalized.** Unreal's importer capitalizes the first letter
   of a skeletal mesh's bone names (`Pelvis`) and keeps an animation track's
   name as written (`pelvis`). See below for what that does.

### Import

`asset_import.py` (and `asset_import_matched_names.py`) build
`AssetImportTask`s with `FbxImportUI` and run under `-ExecutePythonScript`.
`census.py` lists what an import produced. Each character arrives as a
skeletal mesh, a skeleton, two animation sequences (53 and 31 frames, 1.767 s
and 1.033 s), a `Texture2D` (a 2048² atlas for the fighter; 1024² for the
soldier's baked head) and a `MaterialInstanceConstant` whose parent is
`FBXLegacyPhongSurfaceMaterial`. Clips are named `<mesh>Root_<take>`. The
importer warns on both characters: "Imported skeleton has some invalid bind
poses. Skeletal mesh skinning has been rebind using the time zero pose."

### What the frames and the numbers showed

The lab (`RpgAssetLab/`, `lab_shot.py`) builds a row in a game world, waits,
measures and shoots. Left to right: the fighter as first imported; the same
fighter with bone names matched; the soldier with names matched and no clip;
the soldier with its idle; the raw pack character.

- **Textures crossed.** Every bridged asset rendered with its atlas; the
  soldier with its baked head texture.
- **Scale is preserved.** The bridged fighter and Unreal's own copy of the
  same model are the same height. A raw barrel is 72 × 72 × 91 cm; the
  bridged rug 273 × 586 × 20 cm. The pipeline's units are metres, which
  Unreal converts to centimetres.
- **The bone-name defect, measured.** Over half a clip, the left hand turned
  3.5 degrees for every character with a clip and **0.0 degrees for both
  clip-less controls**. The corrected fighter stands in its idle where the
  first import stands in a T-pose. The corrected soldier's hand points
  somewhere its clip-less twin's does not.
- **What the first import got wrong, precisely.** Its bones animate: its
  hand rotation is identical to the corrected import's at the same moment.
  Its mesh does not follow them. So the tracks did reach the bones, and the
  skin did not; most likely the importer compares names by case when it
  binds the skin and not when it looks up a track. Capitalizing the names at
  export makes both agree.

### Measurement lessons, each paid for

- `IsPlaying()` and bone motion do not prove a mesh deforms. A picture does;
  so does a pose compared against a clip-less control.
- An editor run that only draws on demand never ticks a pose into existence;
  a still from it cannot show or disprove animation. A game run ticks.
- Two screenshot requests in a row replace each other; only the second is
  written.
- `unreal.Rotator`'s positional order is (roll, pitch, yaw). A positional
  camera rotation pointed at the sky; keywords do what they say.
- An editor left open on the project makes later builds land under suffixed
  module names that a new process does not load. Close it before rebuilding.
- The default map's player pawn draws a sphere where it spawns. It is not
  part of the evidence.

### Still open from this leg

- The bind-pose warning on every character: what pose the skin is bound to,
  and whether it costs anything visible in motion.
- One skeleton for the rig family: both characters import their own; the web
  pipeline built them on one.
- The animation packs on this machine are FBX: a native route for raw Synty
  characters, untested.
- How a beat becomes motion, sound and animation: the game's own design.

### Not kept here

The proof project, `~/UnrealEngine/Projects/RpgUeProof` (local, no
repository): the plugin with its compiled contract, the imported assets, the
lab, and the evidence frames under `AssetLab/evidence/`. The engine at
`~/UnrealEngine/UE_5.9`. The Unreal-format pack on the Windows data volume.
