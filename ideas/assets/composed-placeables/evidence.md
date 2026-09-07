# World composition evidence and handoff

This record separates delivered facts from useful pre-delivery investigation. The accepted design is [design.md](design.md).

## Verified delivery

GitHub reports the following PRs merged. Local Git object inspection confirmed each reviewed implementation head has the same tree as its squash-merge commit.

| Delivery | Reviewed head | Merge commit | Tree equality |
| --- | --- | --- | --- |
| API [#924](https://github.com/KirkDiggler/rpg-api/pull/924) | `757e6b79d59fe654a3cb1c3ba855c34d1e6e4419` | `7732bfb4342b59e7bfdc76927d5ee475533779aa` | `eb36e8704348e83c6d992337313bf173595da94c` |
| Web [#954](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/954) | `b02b4cd090a5ad09be94b777f6ee69fe18a4d3ba` | `6683f02a396e69d190a01e80c4390ed6cd1a85b1` | `f9c300a66ae4d7298d127c650919ded25353f833` |
| Protos [#300](https://github.com/KirkDiggler/rpg-api-protos/pull/300) | `e06d022af0b0006359d24af51da24245df886c21` | `a4bfa7762cb64cc1ff62ec85649a5f4eaa9e2ad8` | `4bd1413592cbfaa1068bd390c398bbcf0c532e69` |
| Protos [#304](https://github.com/KirkDiggler/rpg-api-protos/pull/304) | `7108d76382862a56f13408c4fd8ee82709dd3a18` | `266e11ca436e6b02bf08128e3968073817c2ba21` | `878b1630937ac4bd8d92dcecc41ae23958f5a0cd` |

Toolkit [#1543](https://github.com/KirkDiggler/rpg-toolkit/pull/1543) merged as `9fbb407e96ed4f328b3fc71b8636ef2f15e89dce` and supplies the API's pinned `world/v0.4.1` `composition.Data`. The earlier web concept [#938](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/938) merged as `6ff789e355c2c65dfe3d10aa788bd05b3c016c87`.

Hosted checks are green for the merged heads: API build/test; web lint/type check, security audit, tests, and deploy preview; proto generation/test and lint/format; toolkit changed-world tests. The parent also freshly ran full API and web CI locally, all green. Native final review records are public at:

- API permanent Delete: [comment 5564045202](https://github.com/KirkDiggler/rpg-api/pull/924#issuecomment-5564045202), accepted 0 Critical / 0 Important / 0 Minor.
- Web permanent Delete UI: [comment 5564229744](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/954#issuecomment-5564229744), accepted 0 Critical / 0 Important / 0 Minor.

The implementation issues [rpg-api#921](https://github.com/KirkDiggler/rpg-api/issues/921), [rpg-dnd5e-web#951](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/951), and [rpg-dnd5e-web#935](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/935) are closed as completed. Parent journey [rpg-project#169](https://github.com/KirkDiggler/rpg-project/issues/169) remains open.

## Delivered behavior evidenced by source and tests

### API

- Toolkit `composition.Data` is exactly `{ID, WorldID, JSON RawMessage}`.
- The protobuf surface is Create/Get/List/Delete with `id`, `world_id`, and JSON string; missing Delete is successful.
- Redis uses one `composition:<WorldID>` hash and `HSETNX`/`HGET`/`HGETALL`/`HDEL`, with no TTL.
- Handler inputs derive caller identity from auth context and compare the requested world to the configured alpha world before service access.
- Tests across repository, orchestrator, and handler layers cover round trips, no TTL, atomic create conflict, world isolation, malformed stored data, authorization/gating, and idempotent delete.

### Web

- The development main menu mounts the reused World Building editor as World Builder with the real RPC source by default; the fixed fixture requires the explicit development flag.
- World save/open/list/delete remains distinct from browser-local drafts and arrangements. Tests cover preservation of the local draft when opening or deleting a world record and explicit transfer back to local ownership.
- Authored scene names drive library labels, palette labels, and tooltips. Malformed records fall back to IDs and remain deletable.
- Thumbnail work is serialized through one R3F canvas and cached by world plus complete snapshot. Tests bind the behavior to the observed asynchronous renderer setup.
- A dungeon placement has an independent instance ID and `composition:props:<id>` ref. Facing, cell, offset, YAML round-trip, Atlas projection, multipart rendering, and Save & Play use the real builder/game paths.
- Confirmed permanent Delete refreshes the list and resolution caches but leaves dungeon references untouched. Missing compositions stay visible/selectable with an explicit missing marker until author removal.
- Explicit per-part point lights preserve enabled/offset/color/intensity/range through scene and arrangement operations. Projection matches the prop's surface alignment; one shared nearest-focus collector caps the dungeon scene at 12 and does not create gameplay illumination or crypt floor pools.

## Local runtime handoff (not a product guarantee)

At cleanup time the owned local path is API `8090` plus web `3031`, with the main-menu World Builder using real RPC against `test-world` and the fixed composition fixture off.

- Web `3031` serves the merged runtime checkout `/home/kirk/game-dev/.runtime/local/compositions/sources/rpg-dnd5e-web` at `6683f02a396e69d190a01e80c4390ed6cd1a85b1`.
- API source is `7732bfb4342b59e7bfdc76927d5ee475533779aa`; the running container was built from tree-equivalent reviewed head `757e6b79d59fe654a3cb1c3ba855c34d1e6e4419` and was not restarted for documentation cleanup.
- The browser origin remains `3031`; the old `3030` preview is retired. No full-stack reset was performed because Redis is ephemeral.
- Composition/content data was preserved byte-identical. The backup and cleanup manifest is `/home/kirk/game-dev/.runtime/local/compositions/cleanup-receipt.olI34G`.
- Primary user assets remain dirty and the pinned provider is still `/home/kirk/game-dev/.runtime/local/compositions/assets-0fc2ced`.

The former feature worktrees were removed after merge verification and backup; they are not active dependencies.

## Historical investigation retained as context

These findings shaped the boundary but are **not** claims that additional systems shipped.

### Forcing scene and rendering

Kirk's five-placement specimen contained one existing skeleton-table, two candles, and two book piles. The source JSON SHA-256 was `c6e50c800d869eecaac74dac6670a37a30ba5bbbade49ca6ccead5ecd1cdf2c8`. Static inspection found four unique cached GLB resources, 1,291,516 source bytes, eight model-mesh instances/candidates, and 7,984 instance-expanded triangles.

Those counts were not frame-time or draw-call measurements. Three's browser `GLTFExporter` could produce a GLB, but one GLB would not automatically merge primitives, materials, textures, or ordinary cloned meshes. This is why the delivered slice retained editable JSON and existing shared `PropModel` resources rather than requiring browser/server baking. Asset refs are frozen as strings, not absolute source bytes; binding and retention remain future work.

### Spatial identity

Toolkit spatial storage was already collection-valued (`map[Position][]string`), while authored dungeon validation and compilation rejected duplicate placement cells. A lower `BasicRoom.PlaceEntity` rule also rejected placement behind an existing movement blocker, so simply deleting duplicate-cell checks would have made construction order matter. Held/dropped projection could show same-cell prop locations without proving every physical-index path supported general co-location.

That investigation supports separating gameplay object identity from visual parts. The delivered composition is one independently identified dungeon prop with a multipart appearance; it did not change engine occupancy rules or turn each candle/book mesh into a gameplay entity.

### Asset and licensing boundary

The private provider remains the authority for licensed source assets and catalog promotion. No licensed GLB or screenshot source was added to this public design repository or to the web composition records. The composition snapshot stores catalog refs and transforms only. Provider pack grouping, custom asset browsing, and absolute asset-version binding were deliberately not absorbed into this slice.
