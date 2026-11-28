# Run — 2026-11-28 — Unreal client S4 Kingdom figures

Slice [rpg-project#1284](https://github.com/KirkDiggler/rpg-project/issues/1284) of [rpg-project#1237](https://github.com/KirkDiggler/rpg-project/issues/1237).

## Shipped

- Workspace ingest extracts native Kingdom and Modular Hero exports without conversion; merged to game-dev main — [game-dev#142](https://github.com/KirkDiggler/game-dev/pull/142).
- Private store promotion, generated manifest and native lineup tooling are complete; merge pending — [rpg-game-assets#353](https://github.com/KirkDiggler/rpg-game-assets/pull/353).
- Client symlink mounts original `/Game/` content without C++ changes; merge pending — [rpg-dnd5e-unreal#7](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/7).

## Decided

- The generated web catalog, not the store inventory, is the authority for the Kingdom NPC refs — [catalog ruling](https://github.com/KirkDiggler/rpg-project/issues/1284#issuecomment-7235008387).
- Project Content preserves native package paths; no plugin descriptor, redirects, conversion or package resave — [mount ruling](https://github.com/KirkDiggler/rpg-project/issues/1284#issuecomment-7235093519).
- Kirk picked the Rider as Alice’s temporary fighter look on 2026-11-28 — [fighter stand-in ruling](https://github.com/KirkDiggler/rpg-project/issues/1284#issuecomment-7235305503).

## Found

- The 204 published Kingdom keys partition into 192 entries and 12 missing Mage Cape variants; the pinned castle roster serves `class_ref=fighter` for Alice — [manifest and roster readback](https://github.com/KirkDiggler/rpg-game-assets/pull/353).
- Twenty-two native figures rendered at their logged positions; the merchant and soldier web checkpoint images differ from the native art, and the Rider has no web counterpart — [comparison readback](https://github.com/KirkDiggler/rpg-project/issues/1284#issuecomment-7235345430).
- With content mounted, UE 5.9 Preview 2 built 0/0, client tests read 18/0 serverless and 25/0 on fresh pinned API, and castle passed the palette check — [client gate readback](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/7).

## Mistaken

- The first design put the native assets under a plugin mount; the required classes failed to load their `/Game/` parents until the Content mount preserved the original package namespace — [load gap](https://github.com/KirkDiggler/rpg-project/issues/1284#issuecomment-7235074641).
- Castle play was run before the first server-backed suite on a shared mutable fixture; two tests failed, then both passed on a freshly started pinned fixture — [per-test gate readback](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/7).
