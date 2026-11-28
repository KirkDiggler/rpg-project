# Run — 2026-11-28 — Unreal client S5 resolve by reference

Slice [rpg-project#1287](https://github.com/KirkDiggler/rpg-project/issues/1287) of [rpg-project#1237](https://github.com/KirkDiggler/rpg-project/issues/1237).

## Shipped

- Six S4 evidence frames and their logs were archived in the private store with their provenance; PR A merged to main `053ee88` — [rpg-game-assets#354](https://github.com/KirkDiggler/rpg-game-assets/pull/354).
- PR B carries the exact-reference rendering and gates; verified merged to main `f504524` — [rpg-dnd5e-unreal#8](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/8).
- `ReadMember` carries the wire's `appearance_ref` alongside faction, without rewriting it — [field commit](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/16063d4).
- `FRpgManifest` resolves a player by `class_ref` and a monster by `appearance_ref`; world members, missing files and unknown keys resolve nothing — [manifest commit](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/50f5b14).
- The play module loads the project-config manifest once and hands it to the board through `SetManifest`; the board does not read a path — [hook commit](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/7bacbf4).
- `PlaceFigure` puts native feet on the floor using the spawned capsule's half-height (96 cm measured for the Rider and Merchant); `kKingdomCameraFacingYaw` is −90° to face the board camera. Missing appearances keep their S3 shape — [placement and readback](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/8).
- A downed native figure swaps to the tipped S3 shape and restores the native when standing; the sighting supplies the standing state — [downed commit](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/a8fded8).
- The active ring stays under either representation; native materials are not tinted, and side and faction logging is unchanged — [board change](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/8).
- `mount-content.sh` links the manifest under `Config/` alongside the existing content mount; the project still builds without the private store — [mount commit](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/dd18712).
- The palette gate checks native presence at its projected footprint rather than a prescribed colour and still checks stand-in colours — [gate commit](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/5266f5f).
- Mounted and unmounted builds read 0 errors and 0 warnings; suites read 25/0 serverless and 32/0 with the pinned server both ways. All four scenarios passed the palette gate both ways; every unmounted figure was a shape — [gate readbacks](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/8).
- Kirk walked the castle as Alice on 2026-11-28: the Rider, Merchant, captain, `gate-1` and `gate-2` all rendered natively, and the first-guard frame passed the palette gate. Kirk: “yeah the lack of camera controls makes it tough to see but they all look like synty models now.” The licensed frame stays local — [operator play](https://github.com/KirkDiggler/rpg-project/issues/1287#issuecomment-7236117778).

## Decided

- Use `ReadSighting` for standing state as S3 does; add the appearance reference to `Member` without moving standing into the roster — [standing ruling](https://github.com/KirkDiggler/rpg-project/issues/1287#issuecomment-7235551882).
- Dead-character props are not downed poses for exact refs; with `downed: null` in the manifest, use the shape instead of substituting a prop — [downed ruling](https://github.com/KirkDiggler/rpg-project/issues/1287#issuecomment-7235551882).
- Keep all S3 registrations and count the seven new cases in both mount states: 25/0 serverless and 32/0 with server — [totals ruling](https://github.com/KirkDiggler/rpg-project/issues/1287#issuecomment-7235571506).
- Construct the generated class name with `FPaths::GetBaseFilename` on UE 5.9 — [basename ruling](https://github.com/KirkDiggler/rpg-project/issues/1287#issuecomment-7235590339).
- Gate the passive castle on its two visible figures and gate guards at the first guard admission in the operator's interactive run, without scripting a route or demanding later joins in that frame — [castle ruling](https://github.com/KirkDiggler/rpg-project/issues/1287#issuecomment-7235590339).

## Found

- The spawned Rider and Merchant capsules have a scaled half-height of 96 cm, not the engine Character default of 88 cm — [placement readback](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/8).
- The seven legacy input warnings come from the S4 pack's parent Blueprint; they are separate from compiler diagnostics and missing-object checks — [runtime readback](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/8).
- The camera frames the floor bounds, so true-scale figures read small; the lack of zoom, pan and rotation makes them hard to see. Camera controls belong to the interaction slice — [story findings](https://github.com/KirkDiggler/rpg-project/issues/1237).

## Mistaken

- The lead's F2 put `standing` on `Member`; preflight found it on `Sighting` and the issue was corrected before building — [standing gap](https://github.com/KirkDiggler/rpg-project/issues/1287#issuecomment-7235540985).
- The lead's F8 said the pack had only dead trees; preflight found dead-character static props, which do not supply exact downed looks — [props gap](https://github.com/KirkDiggler/rpg-project/issues/1287#issuecomment-7235541125).
- The lead carried 18/0 and 25/0 forward from S3 despite adding seven serverless tests; the gate totals were corrected without dropping any tests — [totals gap](https://github.com/KirkDiggler/rpg-project/issues/1287#issuecomment-7235567707).
- The lead named an `FName` method that does not exist in 5.9; checking the engine headers found the helper on `FPaths` — [basename gap](https://github.com/KirkDiggler/rpg-project/issues/1287#issuecomment-7235584794).
- The lead's passive gate assumed the guards were in sight; the pinned opening roster serves only Alice and the pedlar — [castle gap](https://github.com/KirkDiggler/rpg-project/issues/1287#issuecomment-7235584942).
- `RpgBoard.cpp` borrowed the definition of `UPackage` from a neighbouring file; the amalgamated build forgave it, but the per-file gate caught it. The direct include restored standalone compilation — [review fix](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/8#discussion_r4726094645).
- The `--play` pass lines print to the terminal, not the UE log; copy the gate's output with the log so the frame's verdict is preserved — [gate script](https://github.com/KirkDiggler/rpg-dnd5e-unreal/blob/af9cf6319f94e2c09a4cd98b7d5b15710d8baf31/scripts/play.sh#L126-L135).
