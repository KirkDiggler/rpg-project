# Crypt shell landing evidence

Public-safe evidence for [rpg-project#284](https://github.com/KirkDiggler/rpg-project/issues/284), under [journey #169](https://github.com/KirkDiggler/rpg-project/issues/169). The accepted final builder and playable-game frames are the only new binary evidence copied here.

## Provider landing

- Issue [rpg-game-assets#65](https://github.com/KirkDiggler/rpg-game-assets/issues/65) / PR [#68](https://github.com/KirkDiggler/rpg-game-assets/pull/68).
- Merge commit: `f183c96d6d89ecdaf9a2f5dd2c452de485882ed3`.
- Reviewed head: `2facea936b47dd0a5750668be6bfa9a664bcc71d`; reviewed and merge tree: `46e41c26e39f0b1434e1282379bd2cad06f7fd7f`.
- Provider evidence commit: `b9eaec454bc43bbfdb284fa647ffbdef60ff8a3a`.
- Selected candidates: floor `floor-09-01-u6` (repeat `6u`), wall `wall-double-01-worked`; axes `+X` span, `+Y` up, `+Z`/`-Z` faces.
- Runtime profile `harness/models/synty/env/shell-profiles.json`: `d02e6398b06f8b347fbe2e68d91d83bfeccd389ea412be5774d34454c2d164a7`.
- Complete inventory: 2,141 files; payload `fc815f39b4056b0cbbb4edf8d76b552a78c26a80a66da92e77a55d4c22c08303`; tree `f2935197b3c280131afc2da5ac732c0f1e82e1c7b623f2259e9a4ba148f8d57d`.
- Floor `harness/models/synty/textures/Dungeons_Texture_FloorTile_09_01.png`: `ec84f155a32297c64e86b8c678955e25d8f8180023327e42c840dd086916b841`.

| role | provider path | SHA-256 |
| --- | --- | --- |
| body | `harness/models/synty/env/Crypt_Wall_Body_01.glb` | `2216b24e5ea943841682a95c5f4a7692525be42f1cb295bf6d69df33a2e142fc` |
| base | `harness/models/synty/env/Crypt_Wall_Base_01.glb` | `6933008930a251aec0f27ac757611097faa15f38db06cb542e91128fa60c4f6f` |
| cap | `harness/models/synty/env/Crypt_Wall_Cap_01.glb` | `f56b63ded7b8f8f5ca02a8824df9b9f2a2ca4052d1bf7939281b06c68c059a67` |
| door surround | `harness/models/synty/env/Crypt_Wall_Door_Surround_01.glb` | `bd4d0a9ca3da8fcee72f8cfaf72d51040f6754920649b9e30c8c8a2e44093cc0` |
| closed leaf | `harness/models/synty/env/SM_Env_Door_01.glb` | `c1445b4dae6a02127be15fcbd59e6f02f207de28a3461cf95a1ceba18f8d4c15` |

Provider gates: full discovery `337` passed / `20` skipped; Blender export `12/12`; renderer `19/19`; staging `7/7`; combined renderer/staging `26/26`; inventory, profile byte-identity, and stage `--verify-only` checks passed. The recorded provider commands were the full `python3 -m unittest discover -s scripts -p 'test_*.py'` gate, the Blender export/reimport gate, `python3 scripts/build_synty_complete_inventory.py --check`, and `python3 scripts/verify_web_asset_stage.py --verify-only`, followed by the six-view Blender evidence render. Kirk’s verbatim provider verdict: **`looks great`**. The expected optional Draco and Blender `use_nodes` warnings were non-fatal.

## Web landing

- Issue [rpg-dnd5e-web#825](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/825) / PR [#827](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/827).
- Merge commit: `548f561bf8ddab41da53a174e5b69a08358b11e1`.
- Reviewed head: `a770746d73c6bfe35cc743383005e7f796ec672e`; merge tree: `3094d2f0c7b53ea8679229123e5773d59c6f9255`.
- Public evidence head: `404738a9b70ad3a1034252b4c8959cb8012eb0e1`.
- Fresh reviewed-head `npm run test:run`: 231 files passed, 1 skipped; 3,648 tests passed, 1 skipped. The existing jsdom `Window's scrollTo()` notice was the only notice.
- The four GitHub checks were green: Lint and Type Check, Deploy Preview, Test, and Security Audit.
- Web CI gates were green: format, ESLint, TypeScript, production build, combat-HUD CSS guard, Toolkit Contributor Sandbox exclusion, and test gate.

The real flow loaded `crypt-prop-showcase`, compiled `240` cells / `44` boundaries / `2` regions, performed Save, reopened it, and retained byte-identical YAML: `94` lines, `3,593` bytes, SHA-256 `1b5effb21b3ccc5c26153714cff62d7a08041808a1782cd79d9999b3755fca25`. Real Save & Play completed through encounter creation and loaded the `240`-cell, `3`-prop atlas.

Observed provider/runtime requests returned HTTP 200:

- `/models/synty/env/shell-profiles.json` — `d02e6398b06f8b347fbe2e68d91d83bfeccd389ea412be5774d34454c2d164a7`
- `/models/synty/textures/Dungeons_Texture_FloorTile_09_01.png` — `ec84f155a32297c64e86b8c678955e25d8f8180023327e42c840dd086916b841`
- `/models/synty/env/Crypt_Wall_Body_01.glb` — `2216b24e5ea943841682a95c5f4a7692525be42f1cb295bf6d69df33a2e142fc`
- `/models/synty/env/Crypt_Wall_Base_01.glb` — `6933008930a251aec0f27ac757611097faa15f38db06cb542e91128fa60c4f6f`
- `/models/synty/env/Crypt_Wall_Cap_01.glb` — `f56b63ded7b8f8f5ca02a8824df9b9f2a2ca4052d1bf7939281b06c68c059a67`
- `/models/synty/env/Crypt_Wall_Door_Surround_01.glb` — `bd4d0a9ca3da8fcee72f8cfaf72d51040f6754920649b9e30c8c8a2e44093cc0`
- `/models/synty/env/SM_Env_Door_01.glb` — `c1445b4dae6a02127be15fcbd59e6f02f207de28a3461cf95a1ceba18f8d4c15`
- `/models/synty/props/Crypt_Skeleton_Cage_01.glb`
- `/models/synty/props/Crypt_Skeleton_Table_01.glb`
- `/models/synty/props/Crypt_Rug_01.glb`

The approved final builder frame is `final-builder.png`, `1600×900`, SHA-256 `0854b0d0bc4dd56a62185ffcfb774230ad77583f90696068308f6200368f7a83`. The approved final game frame is `final-game.png`, `1600×900`, SHA-256 `b44ef4dd027eaefc02db77f35bb31bb4461b1948dc80974b1bbbcfbe74d9baaa`. Kirk’s verbatim integrated verdict: **`looks really good`**.

## Review and boundaries

Because web PR #827 merged into the non-default `dev` branch, rpg-dnd5e-web
[issues #825](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/825) and
[#791](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/791) were still
**OPEN** at the Task 12 documentation commit. After publication, the reviewed
Task 12 external reconciliation will manually close/mark Done #825 and #791.
This evidence record does not mutate that external state.

The final web range includes the fallback fixes `aefde18` (preserve closed doors) and `a770746` (unmount rejected open-door leaves), followed by a broad review of the provider, web, and project ranges. For the final web fallback range, Copilot was unavailable for the supported attempts and produced no review event; no absent review was treated as approval. The visible `The Reference Tomb` banner is pre-existing static `SessionEncounterView` copy, not atlas identity, and was left unchanged.

[rpg-dnd5e-web#823](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/823) remains a separate open overlay-depth issue. Lighting remains active under [rpg-project#169](https://github.com/KirkDiggler/rpg-project/issues/169); authored lighting is not part of this shell landing. No provider source or generated runtime bytes are included here.
