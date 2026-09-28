# Crypt shell landing evidence

Public-safe evidence for [rpg-project#284](https://github.com/KirkDiggler/rpg-project/issues/284), under [journey #169](https://github.com/KirkDiggler/rpg-project/issues/169). The post-fix builder, playable-game, and close-door frames are the only new binary evidence copied here.

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

## Web follow-up landing

- Issue [rpg-dnd5e-web#828](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/828) / PR [#829](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/829).
- Merge commit: `c38ab663a9ced71bd494035854ec67c662205f0c`.
- Reviewed head: `9ca2bf4d86a8a164c2b1ebe6fd54180c0f924a61`; merge tree: `786bc4bbff12406f9721c918c950675d3f85691e`.
- Root defect/fix: geometry-derived scale plus child-local registration under the exact `gapStart` hinge. Standard and raised walls across east, north, west, and south facings measured left/right/top cover ≥ `0.020000901`, with floor contact `0`.
- Thinness is deliberately unchanged. Kirk’s verbatim verdict: **`door is pretty thin but no gaps`**.
- Provider/profile/frame/leaf hashes are unchanged: profile `d02e6398b06f8b347fbe2e68d91d83bfeccd389ea412be5774d34454c2d164a7`, door frame `bd4d0a9ca3da8fcee72f8cfaf72d51040f6754920649b9e30c8c8a2e44093cc0`, and closed leaf `c1445b4dae6a02127be15fcbd59e6f02f207de28a3461cf95a1ceba18f8d4c15`.
- Focused wall/Atlas/DungeonShell suites: **21 files / 372 tests passed**. Full `npm run test:run`: **231 files / 3,653 tests passed; one file and one test skipped**.
- `npm run ci-check` passed all seven gates; all four GitHub checks passed (Lint and Type Check, Deploy Preview, Test, and Security Audit). Copilot’s one SHA typo was fixed in `9ca2bf4` and answered in the review reply.

The real flow remained `crypt-prop-showcase`: `240` cells / `44` boundaries / `2` regions, Save, reopen, byte-identical YAML, and Save & Play into the `240`-cell / `3`-prop atlas. The post-fix screenshots are all `1600×900`:

| Frame | SHA-256 |
| --- | --- |
| `close-door.png` | `07b7eda475ed812bad3ae8801071a568dd1cec13579c2559931b0b0b27010236` |
| `final-builder.png` | `6365360061dd9087dca5ccc9b62de79bd3f4e123e296e7a8701eee95a710e561` |
| `final-game.png` | `683501ed8983e89acdc9d514ccef467557a17d8f0e4b99b01b26a5bace033943` |

The prior `final-builder.png` (`0854b0d0bc4dd56a62185ffcfb774230ad77583f90696068308f6200368f7a83`) and `final-game.png` (`b44ef4dd027eaefc02db77f35bb31bb4461b1948dc80974b1bbbcfbe74d9baaa`) are superseded pre-registration evidence, not current final evidence.

## Review and boundaries

PR #829’s one Copilot finding was a SHA typo; commit `9ca2bf4` corrected it and received a reply in the review thread. No provider source or generated runtime bytes are included here.

[rpg-dnd5e-web#828](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/828)
remains **OPEN** pending the post-publication manual close. The separate
[rpg-dnd5e-web#823](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/823)
remains open. Lighting remains active under
[rpg-project#169](https://github.com/KirkDiggler/rpg-project/issues/169); authored
lighting is not part of this shell landing. This evidence record does not mutate
external issues, comments, or board state.
