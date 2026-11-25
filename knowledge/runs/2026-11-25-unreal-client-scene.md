# Run — 2026-11-25 — Unreal client: the scene from the atlas

Idea [rpg-project#1154](https://github.com/KirkDiggler/rpg-project/issues/1154). Design and recipe: [rpg-project#1155](https://github.com/KirkDiggler/rpg-project/pull/1155). Base: `rpg-project` `main` at [`8220d38`](https://github.com/KirkDiggler/rpg-project/commit/8220d38).

## Shipped

- The web's view of an atlas, built in Unreal from the atlas fetched over the wire: the recipe, its sources and its record — [rpg-project#1155](https://github.com/KirkDiggler/rpg-project/pull/1155).
- `rpgwire::Session`, the signed-in connection in the plain-C++ core, returning the atlas in plain values — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#leg-3-the-scene-from-the-atlas).
- The `rpg.scene` command: floor, walls, door, the member at the start cell, two cameras, two frames — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#what-was-built).
- A second rendering of the same atlas from its JSON dump, in Python, to check the first against — [reference_top.py](../../ideas/unreal-client/spike/unreal/reference_top.py).
- The engine-side test launches a shipped dungeon by key and can dump its atlas — [zz_ue_spike_test.go](../../ideas/unreal-client/spike/go-side/zz_ue_spike_test.go).

## Decided

- The scene is built from the atlas as the seated member knows it, fetched over the wire, because that is what the web draws for them and it needs no second source of truth — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#what-was-built).
- Authored walls and doors are placed as boxes from the atlas's own spans, a wall with openings by its spans and one without by itself, because the atlas already cuts the openings and the web's wall fitting is a later leg — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#the-frame-from-the-webs-code).
- The core reads the wire types into plain values and exports its session class, because a symbol a host calls across a module needs the module's export mark, learned three times in one day — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#four-things-it-taught).
- The scene is checked against a second rendering made by separate code, because a wrong axis conversion passes every round-trip test and is only caught by drawing the shape, as the web's own notes warn — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#what-the-frames-showed).

## Found

- The authored layout is in feet, a hex being five feet flat to flat, so a foot is `sqrt(3)/5` scene units; the castle's gatehouse cells land inside its walls under that conversion — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#the-frame-from-the-webs-code).
- A freshly seated member's atlas of the castle holds 128 cells in two regions, 16 doorways, 5 authored walls, 1 double door and 12 placed footprints — [evidence](../../ideas/unreal-client/spike/unreal/evidence-2026-11-25-scene.log).
- Two renderings of one atlas, from Python and from Unreal, agree in every outline — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#what-the-frames-showed).
- Unreal's field of view is horizontal: 42 degrees across a 16:9 frame is about 24 down it — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#four-things-it-taught).
- A camera looking straight down in Unreal shows east on the left when north is up; rolled 180 degrees it is the web's top view — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#four-things-it-taught).
- A door is placed under its bare id while its authored record carries the dungeon key as a prefix — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#four-things-it-taught).
- The castle's walls name Dark Fortress pieces by reference; only the Fantasy Kingdom pack is on the machine in Unreal format — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#what-is-crude-in-this-cut).

## Mistaken

- The module boundary was met a third time, by the core's own class this time: the scene module failed to load on an undefined symbol until `Session` carried the export mark — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#four-things-it-taught).
- The first scene frame showed half the atlas and an empty yard: a vertical field of view assumed for a horizontal one — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#four-things-it-taught).
- The door was skipped on the first build, its placed id looked up without the key its record carries — [spike/unreal, leg 3](../../ideas/unreal-client/spike/unreal/README.md#four-things-it-taught).
- Four compile rounds were spent on slips the day's own notes had already answered: a missing `UCLASS()`, an include path learned that morning, a rotation syntax carried from Python into C++ — [rpg-project#1155](https://github.com/KirkDiggler/rpg-project/pull/1155).
