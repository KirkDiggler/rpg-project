# The Unreal legs

> The Unreal side's files were removed from this folder; [where each went](../README.md#where-the-code-went). They read as written at [the commit that held them](https://github.com/KirkDiggler/rpg-project/tree/f24359d38617b4c9b30f833ddc621efcd94f6f08/ideas/unreal-client/spike/unreal).

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

## Leg 3: the scene from the atlas

Question: can Unreal show what the web shows for the same atlas?

### What was built

- `RpgSession.h`, `RpgSession.cpp` (in `RpgWire`, plain C++): a signed-in
  connection. `Session::Atlas` fetches the atlas as a seated member and
  returns it in plain values; the wire types stay inside that module.
- `RpgAssetLab/RpgSceneLabActor.*`: the `rpg.scene` console command. It
  fetches the atlas on a worker thread, then on the game thread builds one
  hex per revealed cell tinted by region, the authored walls and doors as
  boxes, the seated member at the start cell, and two cameras; it shoots
  one frame straight down, one at an angle, and quits.
- `reference_top.py`: draws the same atlas from its JSON dump with the web's
  formulas, in Python, so the Unreal scene can be checked against a second
  code path.
- The engine-side test now also launches a shipped dungeon by key (the
  castle), the way the lobby does, logs the atlas's counts and can dump it
  (`UE_SPIKE_ATLAS`).

### The frame, from the web's code

- Pointy-top hexes of circumradius one scene unit. A cell `(q, r)` sits at
  `x = sqrt(3) * (q + r/2)`, `y = 1.5 * r`. Here one scene unit is one metre.
- Authored walls and doors are in **feet**; a hex is five feet flat to flat,
  so a foot is `sqrt(3)/5` scene units (`structuralLayout.ts` says so in as
  many words). A placement's `depth` runs along its `facing`, its `width`
  across it.
- A wall with openings is drawn by its spans, which the atlas already
  places; one without, by itself.

### The atlas at the start of the castle

For a freshly seated member: 128 cells (q from -17 to 5, r from 7 to 17) in
two regions, Gatehouse with 8 cells and the castle's yard with 115; 16
doorways; 5 authored walls, one with an opening; 1 double door; 12 placed
footprints (the walls, their spans, the door); no props revealed yet. The
rest of the castle is beyond what this member knows, as it is on the web.

### What the frames showed

Drawn from two independent code paths, the atlas has the same outlines: the
band of revealed yard, the gatehouse rectangle with its unknown interior
showing through, the notch of hexes at its upper left, the column at the far
right, the gap in the south wall where the door sits, and the start cell in
the same place. Unreal built 128 floor cells and 7 wall and door boxes, and
skipped nothing. The angled frame reads as the place: a walled yard, the
gatehouse, its door in the far wall, the member tiny on the far side.

### Four things it taught

1. **The module boundary bit a third time.** First the generated symbols,
   then my own plain-C++ class: anything a host calls across a module needs
   the module's export mark. `Session` now carries `RPGWIRE_API`, the one
   Unreal-ism in the core, and reads the wire types into plain values so no
   host ever holds one.
2. **Unreal's field of view is horizontal.** A 42-degree lens sees 42
   degrees across a 16:9 frame and about 24 down it. The first top-down
   frame showed half the scene; the distance now comes from both half-angles.
3. **Unreal is left-handed.** A camera looking straight down with north up
   shows east on the left. Rolled 180 degrees it is the web's top view: x
   right, the web's z down the screen. The web's default camera azimuth is
   not ported; the angled view looks from the south.
4. **A door is placed under its bare id** while its authored record carries
   the dungeon key as a prefix.

### What is crude in this cut

- Walls are plain boxes. The web fits wall-kit pieces by reference (this
  castle names `dnd5e:env:dark-fortress:45_wall_01` and a double door).
- Hexes are flat and tinted. The web lays textured tiles with each region's
  ground and lighting.
- No props: none are revealed at the start, and the manifest that maps a
  reference to a model is not ported.
- Only the seated member is placed; the roster's other creatures are not.
- Sky where the web shows darkness, and a camera angle chosen by eye.

### Recipe

With the engine side up (the throwaway test, which now serves the castle as
`castle_1` for member `alice`, player `player-alice`):

```
UnrealEditor-Cmd <project> -game -RenderOffscreen -unattended -NoSound \
  -windowed -ResX=1920 -ResY=1080 -ExecCmds="rpg.scene <addr> <session> <player> <member>"
python3 reference_top.py castle-atlas.json reference-top.png
```

The frames are under the project's `AssetLab/evidence/`, and stay out of git.

In the **editor** (no `-game`), or with a fifth argument `stay`, the scene is
built into the editor's own world and nothing is shot or closed: the viewport
is placed where the angled camera is, and the right mouse button with W A S D
flies. An actor spawned into the editor's world never begins play, so the
fetch starts when the command sets its target, not from BeginPlay.

## Leg 4: a game started from Unreal, on the real stack

Question: does a game started by the Unreal client, as a real player on the
real stack, play? This is the slice Kirk ruled for: a functional game in
whatever slice we can manage, in C++; Blueprint is a nice-to-have.

### The stack

The real one, brought up the sanctioned way: `scripts/dev-env.sh up
local/ue-play` with a five-line `envs/local/ue-play.env` (the `dev` refs for
the API and the web, host ports 8143 and 3143, no Redis persistence). Local
environments seed the sandbox identities on every start
(`docs/dev-environments.md`): `toolkit-sandbox-bard` among them, with one
character. The published API port is the browser's (gRPC-Web through Envoy);
the native gRPC port is reached at the API container's own address, which the
Unreal client is given on its command line.

### What was built

- The core's `Session` gains three calls: `Characters` (the signed-in
  player's), `StartGame` (create a lobby with the character, ready, start
  the default dungeon, as the web's `LobbyFlow` does), and `Roster`.
  `RecordStream` became public, with the module's export mark.
- `RpgPlayLabActor.*`: `rpg.play <api-host:port> <player>`. It signs in,
  starts a game, reads the roster, has the scene lab build the atlas scene,
  follows the record as the seated character, and places each figure where
  its `joined_at` beat says it stood. The player's figure is the bridged
  fighter; every other is, for now, the bridged skeleton soldier, whatever
  its reference (the reference is logged).

### What happened

Signed in as the bard; a lobby created, readied, the castle launched
(session ids differ per run); the roster read: the bard and the pedlar. The
record then placed **18 figures**: the bard at the start cell, the two gate
guards, the captain, the cook, the king, the sergeant, the turnkey, four
prisoners, the prince, the pedlar and the rest of the castle's cast, each at
its cell. The scene: 128 floor cells, 7 wall and door boxes. In the editor
it stays open to walk around in.

### What the real stack taught

- **A character sits in one run at a time.** A second start is refused, and
  the refusal names the earlier run. The client now leaves it with the
  session's `Exit` verb and starts again. (Abandoning the player's active
  lobby, the web's route, did not free the seat.)
- **The roster carries no positions.** Where each member stood is in the
  record's `joined_at` beats; the roster only says who and of what kind. In
  development mode the record places the whole cast; a seated member's
  record is narrower, as on the web.
- **The editor stops ticking when its window is not on screen.** A fetch on a
  timer never ran until the window was shown; both labs now fetch the moment
  the command runs, and what needs the game thread completes when the window
  appears.
- Figures outside the roster read as "unspecified"; a client maps a figure
  to a model by reference, which is the next leg's work.

### Still open from this leg

- Figures do not move yet: walk, attack and the other acts arrive and are
  logged, not yet shown. No input: nothing sends a verb.
- Models by reference; the castle's guards and its people are stock Synty
  characters that the Unreal-format packs hold natively.
- A figure placed where the record says is not yet a figure that faces,
  animates, or is hidden until seen.

## Leg 5: a stock character animated natively, and the rig gap

Question: do the Unreal-format packs animate natively, so the Blender bridge
stays only for the characters the pipeline assembles itself?

### What ran

- Three clips from the animation packs on this machine (locomotion v3, idles
  v3, sword combat v5: 721, 670 and 243 FBX files) imported with Unreal's own
  importer onto the Fantasy Kingdom pack's stock soldier skeleton, clips only.
- `rpg.lab native`: that soldier with the native idle, the same soldier with
  no clip (the control), and the soldier walking; the hand pose and motion
  measured as in leg 2 (`rename_pack_skeleton.py`, `native_anim_import.py`,
  `native_anim_census.py`).

### What it showed

- **The importer works and the clips bind** to the pack's own skeleton:
  idle 53 frames, walk 31, attack 36 to 41 by clip; the hand moves where the
  clip plays and 0.0 degrees on the control.
- **And the soldier is crumpled**: legs right, torso and head folded. The
  importer says why in one line: `Unable to retrieve bone index for track:
  Hips`.
- **The two sides do not share a rig.** The Fantasy Kingdom pack (saved for
  5.3) is on Synty's older rig: `Pelvis, spine_01, UpperArm_L, lowerarm_l,
  Thigh_L, calf_l, Foot_L`. The animation packs are on the current one:
  `Hips, Spine_01, Shoulder_L, Elbow_L, UpperLeg_L, LowerLeg_L, Ankle_L`. Same
  skeleton, a different vocabulary. Only the tracks whose names happen to
  match bind, and a body animated in part folds.
- The clips are also sparse by design: 15 tracks each, the bones a clip moves.
- Two prefixes in the packs, `A_POLY_` and `A_MOD_`, are two *newer* rigs;
  neither is the pack's old one.

### The route, and where it stopped

The mapping is a rename, which is what the web pipeline does in Blender.
Unreal's `SkeletonModifier` performs it: `rename_bones` took the whole table
(56 bones) in memory. Its `commit_skeleton_to_skeletal_mesh` returned false
and the asset on disk kept the old names, so the end-to-end proof is not
done. The commit is the one open step; the alternative is the bridge, which
already carries this retarget for the web's characters.

### What this decides

- **Native works where a character pack and the animation packs share a
  rig.** A newer pack on the current rig needs nothing but the importer.
- **The Fantasy Kingdom pack needs the mapping once per skeleton.** The table
  is in `rename_pack_skeleton.py`; which side of the fingers takes the `_1`
  suffix is still a guess to confirm by eye.
- The bridge is not made redundant by the Unreal-format packs; it is the
  other route, and the only one for the pipeline's own characters.

## Editor

VS Code or Windsurf, from `RpgUeProof.code-workspace` (a copy is in
`editor/`). C++ IntelliSense reads a compile database the build tool
writes (`-Mode=GenerateClangDatabase`, hand-written files only; exclusions
are path prefixes, absolute ones work); Python resolves `unreal` from the
stubs the editor writes in developer mode.

The database found that three lab files built only because Unreal compiles
a module amalgamated: alone, each borrowed `#include`s from its neighbour.
`perfile_check.py` compiles every hand-written file on its own under the
database's flags; all eight are clean, and the real build still passes.

## Leg 6: intent in, record out

The plan and its checks are in [../round-verbs.md](../round-verbs.md).

### What ran

- The core's verbs, `Afford`, `Move`, `Attack`, `EndTurn`, `Exit` and a
  record read-back, each proven by an automation test against the server the
  engine-side test serves (`RpgWire.Session.*`, six tests, stable to repeat).
- What a beat means, as two functions with no world in them (`PlanMotion`,
  `HexLine`; `RpgAssetLab.Motion.*`, seven tests on the beats the engine
  wrote).
- The play lab in a standalone game on the real stack, as the sandbox bard:
  `rpg.play`, then `rpg.verb move -1 15`, `rpg.verb attack pedlar`,
  `rpg.verb end`.

### What it showed

- **A verb in is the record out.** A step comes back as a `moved` beat and
  the figure walks it, 0.40 s a cell, 25 walks and 25 arrivals in the first
  full run. A swing comes back as a `struck` or `missed` beat (the engine's
  dice: 14 to hit, 6 and 7 to miss, in three runs) and the figure turns to
  its target and lunges. A `turn-ended` beat moves a marker over whoever
  acts next.
- **The world answers.** The bard's swing turned the pedlar `hostile`,
  formed a fight, and the garrison heard it: gate-1 walked seven cells
  toward the bard in the world tick that followed, gate-2 closed too.
- **The frame cost, measured.** 75 beats in the burst after a swing: a
  maximum frame of 10.3 ms and a p99 of 9.0 ms, uncapped and offscreen,
  against 11.3 ms p99 idle. A frame that took a screenshot alongside 57 of
  those beats ran 126 ms: the screenshot, not the beats. One hitch of about
  350 ms sits half a second after the scene builds, on first-time loads.

### What the game world showed that the editor had hidden

- The scene lab began play as it was spawned, before it had a target, and
  so had never built the scene in a game; the figures had been standing on
  the engine template's floor.
- A `Move` path names the cells to step onto: the engine refuses one that
  begins where the member stands.
- A marker spawned in the frame of a screenshot is a package still loading.

### Input

A click on the floor names the cell it landed on (the ray under the
cursor met with the floor's plane, rounded to a hex), and sends the
straight hex line to it: the engine walks it cell by cell and stops where
it says. A click on a figure swings at it. `E` ends the turn. Each result
and each refusal is written on the screen, in the engine's words. Proven
through the game's own input, a real mouse event at a cell's screen
position and a real key event, not by calling the handlers.

Not in this leg: a route around walls (the client names the straight
line; the engine stops it), models by reference, and the interaction
model behind any of it, which is O1.

## Not kept here

The proof project, `~/UnrealEngine/Projects/RpgUeProof` (local, no
repository): the plugin with its compiled contract, the imported assets, the
lab, and the evidence frames under `AssetLab/evidence/`. The engine at
`~/UnrealEngine/UE_5.9`. The Unreal-format pack on the Windows data volume.
