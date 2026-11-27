# Run — 2026-11-25 — Unreal client: the record in the editor, and the assets

Idea [rpg-project#1154](https://github.com/KirkDiggler/rpg-project/issues/1154). Design and recipe: [rpg-project#1155](https://github.com/KirkDiggler/rpg-project/pull/1155). Base: `rpg-project` `main` at [`8220d38`](https://github.com/KirkDiggler/rpg-project/commit/8220d38).

## Shipped

- The recipe and record of two Unreal legs, beside the headless one: the record client as an Unreal module, and the asset bridge — [rpg-project#1155](https://github.com/KirkDiggler/rpg-project/pull/1155).
- Ten source files: the module's build rules, the asset lab, the Blender bridge and the Unreal import scripts — [spike/unreal](../../ideas/unreal-client/spike/unreal/README.md).
- One measurement log, the row of characters with its clip-less controls — [evidence-2026-11-25-game-row.log](../../ideas/unreal-client/spike/unreal/evidence-2026-11-25-game-row.log).

## Decided

- The generated code and its caller live in one module, because a module boundary is a binary boundary in Unreal: only API-macro-marked symbols export, and the first call into the generated code from a second module failed at runtime — [spike/unreal, leg 1](../../ideas/unreal-client/spike/unreal/README.md#the-module).
- The generated `.cc` files are flattened to unique names for Unreal, because one-file-per-action builds name objects by basename and four file names in the contract are shared across packages, `service.pb.cc` by ten of them — [spike/unreal, leg 1](../../ideas/unreal-client/spike/unreal/README.md#the-module).
- The asset bridge is a second exit from the existing pipeline (GLB → FBX in Blender, then Unreal's headless import), not a second pipeline; the pipeline's Blender step and its review flow are untouched — [spike/unreal, leg 2](../../ideas/unreal-client/spike/unreal/README.md#leg-2-the-assets).
- Bone names are capitalized at export, because Unreal's importer capitalizes a mesh's bone names and does not a track's; with lower-case names the bones animate and the mesh stays in its bind pose — [spike/unreal, leg 2](../../ideas/unreal-client/spike/unreal/README.md#what-the-frames-and-the-numbers-showed).
- Evidence images stay out of the repository, because they show licensed models and `rpg-project` is public; the record carries numbers — [spike/unreal](../../ideas/unreal-client/spike/unreal/README.md).
- Animation is judged by a picture or by a pose against a clip-less control, not by `IsPlaying()` or bone motion, because the first import reported both while its mesh stood still — [evidence log](../../ideas/unreal-client/spike/unreal/evidence-2026-11-25-game-row.log).

## Found

- Unreal 5.9.0 Preview 2 runs on Ubuntu 26.04 under Vulkan on this machine, plugin loaded, editor startup 30 seconds; its toolchain is clang 20.1.8 — [spike/unreal, leg 1](../../ideas/unreal-client/spike/unreal/README.md#the-engine).
- Protobuf 4.23.4 and gRPC 1.57.0, built with clang 16 for 5.6, link and load inside a module 5.9 compiles with clang 20, C++20, -O3 — [spike/unreal, leg 1](../../ideas/unreal-client/spike/unreal/README.md#the-module).
- A clean compile of the 72 generated files plus the module takes 73 seconds on 16 physical cores — [spike/unreal, leg 1](../../ideas/unreal-client/spike/unreal/README.md#numbers).
- In the editor, 31 beats replay within one millisecond of game-thread time and a live round reaches the game thread 10 to 13 ms after the engine begins it — [spike/unreal, leg 1](../../ideas/unreal-client/spike/unreal/README.md#numbers).
- Unreal 5.9 compiles the module with `-fexceptions` although the module asks for none, 87 of 87 actions — [spike/unreal, leg 1](../../ideas/unreal-client/spike/unreal/README.md#numbers).
- Unreal 5.9 has no glTF importer; the web's GLBs cross through Blender as FBX with six export choices: no leaf bones, a take per clip, 30 fps, no display shape, textures as files, bone names capitalized — [spike/unreal, leg 2](../../ideas/unreal-client/spike/unreal/README.md#the-bridge-and-the-six-choices-that-mattered).
- The Unreal-format Fantasy Kingdom pack, saved by 5.3, loads unchanged in 5.9; it holds 70 rigged characters, 2,393 static meshes and no animation clips — [spike/unreal, leg 2](../../ideas/unreal-client/spike/unreal/README.md#sources-on-the-machine).
- The bridged fighter and Unreal's own copy of the same model are the same height; a raw barrel measures 72 × 72 × 91 cm, so the pipeline's metres survive — [spike/unreal, leg 2](../../ideas/unreal-client/spike/unreal/README.md#what-the-frames-and-the-numbers-showed).
- The model source packs are no longer on the machine; only three animation packs are — [spike/unreal, leg 2](../../ideas/unreal-client/spike/unreal/README.md#sources-on-the-machine).

## Mistaken

- The first cut of the module put the generated code in its own module; it loaded, and the first call crashed the editor. The merge into one module and the frame that followed are the fix — [rpg-project#1155](https://github.com/KirkDiggler/rpg-project/pull/1155).
- The first reading of the T-pose was "no track finds its bone". The measurements say the bones do animate and the mesh does not follow; the record states both, and the corrected export is the evidence — [spike/unreal, leg 2](../../ideas/unreal-client/spike/unreal/README.md#what-the-frames-and-the-numbers-showed).
- Three stills carried no evidence before one did: a camera aimed at the sky by a positional rotation, a level saved without its actors, and an editor that does not tick poses. The lab that ticks in a game world is what the record rests on — [spike/unreal, measurement lessons](../../ideas/unreal-client/spike/unreal/README.md#measurement-lessons-each-paid-for).
- A rebuild was written under a suffixed module name because an editor was still open, and the next run silently used the old module for one round — [spike/unreal, measurement lessons](../../ideas/unreal-client/spike/unreal/README.md#measurement-lessons-each-paid-for).
