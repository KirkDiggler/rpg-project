# Run — 2026-11-28 — Unreal client S4 Kingdom figures

Slice [rpg-project#1284](https://github.com/KirkDiggler/rpg-project/issues/1284) of [rpg-project#1237](https://github.com/KirkDiggler/rpg-project/issues/1237).

## Shipped

- Workspace ingest extracts native Kingdom and Modular Hero exports without conversion; merged to game-dev main — [game-dev#142](https://github.com/KirkDiggler/game-dev/pull/142).
- The store's `unreal/RpgGameAssets/` — synced content, the generated manifest (192 entries, 12 named missing of the web's 204), the promote, probe and lineup scripts, eight tests — merged to rpg-game-assets main `3c6a08c`, nothing licensed tracked, the repository private — [rpg-game-assets#353](https://github.com/KirkDiggler/rpg-game-assets/pull/353).
- The client mounts the pack under its own `Content/`, one symlink and one ignored path, no C++; merged to main `396b7ca` — [rpg-dnd5e-unreal#7](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/7).
- Two independent reviews on `glm-5.3:cloud`, both merge-ready at the first heads, five Minors accepted and fixed before merge — among them that a vanished default stand-in would have regenerated `players: {}` in silence; it now refuses — [B](https://github.com/KirkDiggler/rpg-game-assets/pull/353#pullrequestreview-6096304035), [C](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/7#pullrequestreview-6096304244).

## Decided

- The generated web catalog, not the store inventory, is the authority for the Kingdom NPC refs — [catalog ruling](https://github.com/KirkDiggler/rpg-project/issues/1284#issuecomment-7235008387).
- Project Content preserves native package paths; no plugin descriptor, redirects, conversion or package resave — [mount ruling](https://github.com/KirkDiggler/rpg-project/issues/1284#issuecomment-7235093519).
- Kirk picked the Rider as Alice’s temporary fighter look on 2026-11-28 — [fighter stand-in ruling](https://github.com/KirkDiggler/rpg-project/issues/1284#issuecomment-7235305503).

## Found

- The 204 published Kingdom keys partition into 192 entries and 12 missing Mage Cape variants; the pinned castle roster serves `class_ref=fighter` for Alice — [manifest and roster readback](https://github.com/KirkDiggler/rpg-game-assets/pull/353).
- Twenty-two native figures rendered at their logged positions; the web's merchant and soldier are the same presets under the web's lighting, and the Rider has no web counterpart — [comparison readback](https://github.com/KirkDiggler/rpg-project/issues/1284#issuecomment-7235345430).
- With content mounted, UE 5.9 Preview 2 built 0/0, client tests read 18/0 serverless and 25/0 on fresh pinned API, and castle passed the palette check — [client gate readback](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/7).

## Mistaken

- The issue named the store's `npcs/` directory as the list of refs to generate; a builder's preflight showed the directory and the web's published catalog disagree — a bartender the store holds and the web never published, and downed variants the glob would have turned into refs — and the authority became the file the web consumes, whose entries already carry the Synty `sourceName` — [the gap](https://github.com/KirkDiggler/rpg-project/issues/1284#issuecomment-7234990318), [the ruling](https://github.com/KirkDiggler/rpg-project/issues/1284#issuecomment-7235008387).
- The lineage of the lineage: the slice's "plugin" was the lead's word, not R6's — the design said *mount*, and the pack's own imports said *where* — [design](https://github.com/KirkDiggler/rpg-project/blob/main/ideas/unreal-client/design.md).
- The first design put the native assets under a plugin mount; the required classes failed to load their `/Game/` parents until the Content mount preserved the original package namespace — [load gap](https://github.com/KirkDiggler/rpg-project/issues/1284#issuecomment-7235074641).
- Castle play was run before the first server-backed suite on a shared mutable fixture; two tests failed, then both passed on a freshly started pinned fixture — [per-test gate readback](https://github.com/KirkDiggler/rpg-dnd5e-unreal/pull/7).
