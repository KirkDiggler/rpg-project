# Crypt prop specimens — landed public evidence

This folder preserves the accepted integrated builder/game evidence for
[rpg-project#275](https://github.com/KirkDiggler/rpg-project/issues/275)
without copying any private Blender source, raw licensed GLBs, provider
manifests, or neutral private renders. The two PNGs here are the final public
runtime/build captures accepted after the provider and web slices merged.

## Provenance and authority

- Provider slice: [rpg-game-assets#63](https://github.com/KirkDiggler/rpg-game-assets/issues/63)
  / [PR #64](https://github.com/KirkDiggler/rpg-game-assets/pull/64), merged to
  `main` as `6c24b19861df127faa69bd4d1ab6ec8fdfad537e` from reviewed feature head
  `951ac2a44dd40e0974e102434dbc7164665c571f`.
- Provider merge tree and reviewed feature tree are identical:
  `37b17b93e82cea57fc1fa5a6e2dc3a6ed6d95bdb`.
- Web slice: [rpg-dnd5e-web#814](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/814)
  / [PR #820](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/820), merged to
  `dev` as `12b9c8a69ae0335e076872f8b89cba3fe025f5aa` from reviewed head
  `3de2e1ad7b83eaee9c7a3ef10d1052c8175475fa`.
- Web merge tree and reviewed tree are identical:
  `4ff6ca291d8e9cc01f34dc5854d7aa26f4fc70ec`.
- These accepted PNGs were copied from the local integrated web worktree at
  `/home/kirk/.pi/worktrees/rpg-dnd5e-web/814-crypt-props/docs/evidence/275-crypt-prop-specimens/`
  because PR #820 merged before their evidence commit. The product tree merged to
  `dev` is identical to the reviewed head, so this project record is the durable
  public landing surface.

## Exact provider artifacts

- `Crypt_Skeleton_Cage_01.glb` —
  `fb16c3bed0fb284e37cfbe2914e7dfa2eeae54fb429796ab5ae1526f4126a4af`
- `Crypt_Skeleton_Table_01.glb` —
  `be957e0e59b4efff6cbbcafba0a473e4e20f9dbca0a7892691121b8e477a0ff6`
- `Crypt_Rug_01.glb` —
  `9f3861707a9b3416b89ddee307662fb7c3d106751f2a3f733ac902b7432184ed`
- Provider complete-inventory tree SHA-256 (`synty-complete-tree`, 2,136 files):
  `eaf6e1f2128c0dfe134486c2d1c22ea9c9c4535bcb1adc9306b7522343367191`
- Provider inventory document SHA-256:
  `24e629c850185c2859cd0efc6efdead3dbfb04a561b99e5755615c617d9768cb`

## Final public image hashes

- `final-builder.png` — PNG, 1600×900, 279473 bytes,
  `29f965fa4d38440a62d0250763d8e773aaa2f3278247fd04f7e39af1c59efb59`
- `final-game.png` — PNG, 1600×900, 728748 bytes,
  `e8d3625906992bcbc504f85196203efe6fd01c44de79bdc9120e396bd6ed7720`

## Shared render rule and routes

- Shared renderer contract: `DUNGEON_SURFACE_Y = 0.2` lifts the Synty floor and
  all props uniformly in both builder and playable game; no specimen-specific
  repair transform or rug `renderScale` remains.
- Builder review route:
  `http://127.0.0.1:3012/?concept=dungeon-builder&authorFixture=crypt-props`
- Live authoring route:
  `http://127.0.0.1:3012/?playerId=test-player`
- Save/reopen proof used the real path Home → Stanthony → Dungeon Builder →
  Load `/tmp/crypt-prop-showcase.yaml` → Save → Open `Crypt Prop Showcase
  (crypt-prop-showcase)` → Save & Play.

## Save/reopen identity proof

- Downloaded source YAML, loaded editor YAML, and reopened server YAML were
  byte-identical at SHA-256
  `a3927fdbf6b38fb886c55b72f58ad116dd8282917fb279a4f8c5f3c4a5e25542`
  (1,187 bytes).
- Preserved placements through reopen and play:
  - `dnd5e:props:skeleton-cage` at `[3,2]`, facing `se`, offset `[0,0]`
  - `dnd5e:props:skeleton-table` at `[6,4]`, facing `e`, offset `[0,0]`
  - `dnd5e:props:rug` at `[9,3]`, facing `e`, offset `[0,0]`

## Tests and checks

- PR #820 had four green GitHub checks before merge: **Lint and Type Check**,
  **Deploy Preview**, **Test**, and **Security Audit**.
- Current integrated head `3de2e1a` passed local `npm run ci-check` (format check, lint check, TypeScript type check, build, built theme CSS carries the combat-HUD block, production assets exclude Toolkit Contributor Sandbox code, tests).
- Current integrated head also passed the visible `npm run test:run`: `Test Files 210 passed | 1 skipped (211)` and `Tests 3443 passed | 1 skipped (3444)`.
- The accepted integrated evidence also sits on a previously verified full local
  suite run at the same head family: `210 passing files / 1 skipped file` and
  `3,443 passing tests / 1 skipped test`, with the one intentional skipped
  file/test pair unchanged.
- Builder capture logged zero browser errors. The playable route returned HTTP
  `200` for the three crypt GLBs and the completed authoring/lobby/session RPCs;
  the only console noise was the already-known React StrictMode cleanup abort on
  the first `GetStory` / `StreamEvents` mount.

## Kirk verdicts

- Provider exact-merge verdict: “yeah they look great.”
- Current integrated verdict: “it does.”

## Why the next lane stays open

Journey [rpg-project#169](https://github.com/KirkDiggler/rpg-project/issues/169)
remains open. This wave proved the specimen/promotion/evidence flow and landed
three builder-ready crypt props; the next visual-fidelity lane is authored
lighting, with [rpg-project#190](https://github.com/KirkDiggler/rpg-project/issues/190)
as historical input to reconcile against the current stack.
