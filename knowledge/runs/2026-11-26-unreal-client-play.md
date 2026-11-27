# Run — 2026-11-26 — Unreal client: a game started from Unreal, on the real stack

Idea [rpg-project#1154](https://github.com/KirkDiggler/rpg-project/issues/1154). Design and recipe: [rpg-project#1155](https://github.com/KirkDiggler/rpg-project/pull/1155). Base: `rpg-project` `main` at [`8220d38`](https://github.com/KirkDiggler/rpg-project/commit/8220d38).

## Shipped

- A game started by the Unreal client as a real player on the real stack: the recipe, sources and evidence — [rpg-project#1155](https://github.com/KirkDiggler/rpg-project/pull/1155).
- `Session::Characters`, `Session::StartGame` and `Session::Roster` in the plain-C++ core; `RecordStream` public — [RpgSession.h](https://github.com/KirkDiggler/rpg-project/blob/f24359d38617b4c9b30f833ddc621efcd94f6f08/ideas/unreal-client/spike/unreal/RpgSession.h).
- The `rpg.play` command and the figures it places from the record — [RpgPlayLabActor.cpp](https://github.com/KirkDiggler/rpg-project/blob/f24359d38617b4c9b30f833ddc621efcd94f6f08/ideas/unreal-client/spike/unreal/RpgAssetLab/RpgPlayLabActor.cpp).
- The log of one such game — [evidence-2026-11-25-play.log](../../ideas/unreal-client/spike/unreal/evidence-2026-11-25-play.log).

## Decided

- The slice is a functional game in C++ on the real stack, with Blueprint deferred, because the operator writes C++ and a scene connected to the backend is worth more to him than a graph; ruled by the operator in conversation 2026-11-25 — [spike/unreal, leg 4](../../ideas/unreal-client/spike/unreal/README.md#leg-4-a-game-started-from-unreal-on-the-real-stack).
- The client starts a game the way the web's lobby does and leaves an earlier run with the session's `Exit` verb when the stack refuses a second seat, because the refusal names the run and the lobby's abandon did not free the seat — [spike/unreal, leg 4](../../ideas/unreal-client/spike/unreal/README.md#what-the-real-stack-taught).
- Figures are placed from the record's `joined_at` beats, because the roster carries no positions — [spike/unreal, leg 4](../../ideas/unreal-client/spike/unreal/README.md#what-the-real-stack-taught).
- Both labs fetch the moment their command runs, because the editor stops ticking while its window is hidden and a timer waits for a tick — [spike/unreal, leg 4](../../ideas/unreal-client/spike/unreal/README.md#what-the-real-stack-taught).

## Found

- As `toolkit-sandbox-bard` on a local environment, the Unreal client starts the castle and the record places 18 figures, the bard and the castle's whole cast — [evidence](../../ideas/unreal-client/spike/unreal/evidence-2026-11-25-play.log).
- The roster of a freshly seated member of the castle names two members, the bard and the pedlar; the rest of the cast is known to the record in development mode — [evidence](../../ideas/unreal-client/spike/unreal/evidence-2026-11-25-play.log).
- A published API port on a local environment is the browser's; native gRPC is reached at the API container's own address — [spike/unreal, leg 4](../../ideas/unreal-client/spike/unreal/README.md#the-stack).

## Mistaken

- The first two editor attempts logged nothing after the command: a fetch on a timer in an editor whose window was not on screen — [spike/unreal, leg 4](../../ideas/unreal-client/spike/unreal/README.md#what-the-real-stack-taught).
- The first recovery from a taken seat abandoned the active lobby, which did not free it; the refusal's own named run and the `Exit` verb did — [spike/unreal, leg 4](../../ideas/unreal-client/spike/unreal/README.md#what-the-real-stack-taught).
- A `UCLASS()` marker was forgotten for the third time in one day, and a `size_t` went to a `%d` that the engine's format checker caught — [rpg-project#1155](https://github.com/KirkDiggler/rpg-project/pull/1155).
