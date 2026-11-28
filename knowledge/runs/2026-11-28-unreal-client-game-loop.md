# Run — 2026-11-28 — Unreal client S2, the game loop as a game mode

The second slice of the Unreal client, [rpg-project#1255](https://github.com/KirkDiggler/rpg-project/issues/1255), built by one builder from its issue alone through seven revivals; every gate green at `936fd12` in [rpg-dnd5e-unreal#5](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/5).

## Shipped

- The project launches, for the first time: `RpgDnd5eUnreal` listed in the `.uproject`, `GlobalDefaultGameMode` in `DefaultEngine.ini`, and a first launch whose log says *module loaded* and *Game class is 'RpgGameMode'* — [rpg-project#1255, F9 and the receipt](https://github.com/KirkDiggler/rpg-project/issues/1255).
- `RpgBoard`, the module table's empty row: the board — a known-cell floor as one procedural mesh, figures as `Cylinder`, `Sphere` and `Cube` from the engine's own basic shapes, a `Cone` for the turn marker — plus `ARpgHud` with one status line, `ARpgPlayerController` with three binds, and `URpgPlay`, the loop — [rpg-dnd5e-unreal#5](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/5).
- The rules of the loop: every click is sent with whatever declaration `Afford` last offered; a refusal's words go to the status line and nothing is planned; figures move only on the record's beats; one act in flight; the clock read from `Afforded.clock`, never chosen — [rpg-project#1255, step 7](https://github.com/KirkDiggler/rpg-project/issues/1255).
- A figure enters the board on the engine's word about sight — a `sighted` beat's `gained` with the player as observer — and leaves on its `lost`; the roster settles what it is by its kind; `joined_at` and `moved` for members not in sight move map entries and nothing else — [the disposition](https://github.com/KirkDiggler/rpg-project/issues/1255#issuecomment-7231073554).
- `scripts/play.sh`: four log-driven scenarios over the engine's test server — a fight from the start (swing, turn ended), a refusal with the engine's words, a free walk of two moved beats, the castle's opening — each leaving a frame, and `--play` for the operator's own windowed turn — [rpg-dnd5e-unreal#5](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/5).
- Step 1b, a find against S1's recipe carried here: the libraries' OpenSSL version script, `UE_OpenSSL.ver`, written by the recipe from the archives' own symbols, a fourth `test-libs` check, and `readelf -V` showing the version node where the symbols were unversioned — [06de7c8](https://github.com/KirkDiggler/rpg-dnd5e-unreal/commit/06de7c82408b3b8861e1a679e166a4c0445ee40e).
- The gates as printed: build `Succeeded` with no compiler warning and no UBT warning, by a count that reads what a warning says; `test-libs` four checks; per-file clean; thirteen automation tests without the server and nineteen with it; four scenarios all `PASS` — [rpg-dnd5e-unreal#5](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/5).

## Decided

- Seated by four command-line switches, no lobby: creating or joining a session and choosing a character are the control panel's, S4 — [rpg-project#1255, section 1](https://github.com/KirkDiggler/rpg-project/issues/1255).
- The client plays whichever clock the engine is on — the operator's correction, from memory of playing the spike, of the lead's *turn clock only* — [rpg-project#1237](https://github.com/KirkDiggler/rpg-project/issues/1237).
- The test server gates the slice and the real stack's two-clients-at-one-door is a find to attempt; the operator's own turn in the castle is part of done — [rpg-project#1255, section 1](https://github.com/KirkDiggler/rpg-project/issues/1255).
- One new module, not two: a module boundary is a binary boundary here, and no core type crosses into `RpgBoard/Public/`, which keeps the `verify` guard in one module — [rpg-project#1255, F7, F8 and the Path](https://github.com/KirkDiggler/rpg-project/issues/1255).
- The loop owns the session and the game module owns the process: `URpgPlay` reports `OnLeft` and the game mode ends the play session or exits — [the disposition](https://github.com/KirkDiggler/rpg-project/issues/1255#issuecomment-7230707041).
- A figure's side is the engine's kind first and the references only when the kind is unspecified; the pedlar is a monster because the engine says so, and a friendly commoner's shape is S3's question — [the disposition](https://github.com/KirkDiggler/rpg-project/issues/1255#issuecomment-7230960384).
- A larger builder, with its reason on the issue: the repository's first module with a world in it — [rpg-project#1255, section 8](https://github.com/KirkDiggler/rpg-project/issues/1255).

## Found

- The project had never been launched: no `Modules` in the `.uproject`, the game module built through `ExtraModuleNames` alone, which the runtime does not load — `ProjectManager.cpp:51-60` — [the disposition](https://github.com/KirkDiggler/rpg-project/issues/1255#issuecomment-7230476654).
- No module outside the core had included the core's headers, and UBT carries no force-include across a module line — `ModuleRules.cs:812` is the only such property — [rpg-project#1255, F7](https://github.com/KirkDiggler/rpg-project/issues/1255).
- S1's recipe left the bundled OpenSSL unversioned, its symbols exposed to interposition, and its gate said zero warnings because UBT prints these with no prefix and the gate counted prefixes — [the gap](https://github.com/KirkDiggler/rpg-project/issues/1255#issuecomment-7230595264).
- `/Engine/BasicShapes/` has no `Capsule`; `ProceduralMeshComponent` is another plugin's module and needs a plugin dependency; 100 cm is a hex's circumradius, not the distance between its cells — [the disposition](https://github.com/KirkDiggler/rpg-project/issues/1255#issuecomment-7230527083).
- `Move`'s path must not carry the cell the member stands on; the serverless `test.sh` filter was the literal `RpgClient.` and would never have selected the new module's tests — [the disposition](https://github.com/KirkDiggler/rpg-project/issues/1255#issuecomment-7230445145).
- The record replays every member's `joined_at` to every reader; the roster is sight-filtered, skips world members, and calls a commoner a monster; the guards of the-mad-king are declared monsters, absent from the roster only until sighted; the pinned `sighted` payload says `gained`, `changed` and `lost` — [the dispositions](https://github.com/KirkDiggler/rpg-project/issues/1255#issuecomment-7231073554).
- The engine's test server stands up five sessions, and one is the castle with its guards, so the operator's walk needs no real stack — [rpg-project#1255, F10](https://github.com/KirkDiggler/rpg-project/issues/1255).

## Mistaken

- An issue with nine tests where eight exist, two line counts off, and a camera roll read from the line above it in an output cut short at the width — [the disposition](https://github.com/KirkDiggler/rpg-project/issues/1255#issuecomment-7230445145).
- A probe of an unbuilt worktree, named by a relative project path the engine resolves against its own directory; the question it asked was answerable from the engine's source in a line — [the disposition](https://github.com/KirkDiggler/rpg-project/issues/1255#issuecomment-7230476654).
- A `StartupModule` hook required with no file named to hold it — [the gap](https://github.com/KirkDiggler/rpg-project/issues/1255#issuecomment-7230598486).
- A figure's side read from references, because a scout quoted a struct through a filter that dropped the kind's two lines without saying so — [the disposition](https://github.com/KirkDiggler/rpg-project/issues/1255#issuecomment-7230960384).
- *No beat exists for losing sight*, and admission on `changed`, both ruled from a spike's log that an older writer had produced, where the pinned engine says `gained` and `lost` — [the disposition](https://github.com/KirkDiggler/rpg-project/issues/1255#issuecomment-7231073554).
- An unquoted shell heredoc ran an amendment's backticks as commands; nothing was written, and the amendment was redone from a file — [the disposition](https://github.com/KirkDiggler/rpg-project/issues/1255#issuecomment-7231025013).
- A merge whose `case` tested `true` where the tool says `MERGEABLE`, then a slice closed as if it had merged; reopened and merged in order — [rpg-project#1249](https://github.com/KirkDiggler/rpg-project/issues/1249).
- Nineteen gaps in one lead's brief, every one caught by one builder's preflight before a line of code: the preflight is the cheapest session there is, and a brief is not clean because its author believes it — [rpg-project#1255](https://github.com/KirkDiggler/rpg-project/issues/1255).
