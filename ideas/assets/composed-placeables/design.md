# Authored arrangements as single placeable props

Design slice: [rpg-project#378](https://github.com/KirkDiggler/rpg-project/issues/378), under [Composable Dungeon Builder #169](https://github.com/KirkDiggler/rpg-project/issues/169).

Status: the outcome and first specimen are approved in conversation; this written publishing contract awaits Kirk's review. Implementation planning follows that review.

## Outcome

An author builds an arrangement visually, retains its editable JSON source, and publishes a fixed snapshot as one ordinary prop. Dungeon Builder and the playable game render that same artifact through their existing prop path. The game does not gain multiple occupants per hex, component-level targeting, pickup actions, or inferred collision rules.

The first specimen is Kirk's actual exported scene, not a reconstruction of its screenshot:

- one existing `dnd5e:props:skeleton-table` asset, already containing the skeleton and table;
- two `dnd5e:props:candles` placements;
- two `dnd5e:props:books` placements;
- five authored transforms, no group or support links;
- source scene SHA-256 `c6e50c800d869eecaac74dac6670a37a30ba5bbbade49ca6ccead5ecd1cdf2c8`;
- anchor item `17c67f39-6b5d-49ad-a9be-e63722824dc1`, the skeleton-table, at authored Y zero.

All five placements become the composition. The existing skeleton-table is neither decomposed nor overwritten. Books and candles remain scenery. The new family is `dnd5e:props:decorated-skeleton-table`, with first exact snapshot `dnd5e:props:decorated-skeleton-table:01` as its default. Publishing a changed composition creates a new exact snapshot rather than mutating `:01`.

## Relationship to work already in flight

The [finished crypt specimens](../../dungeon-builder/crypt-prop-specimens.md) established one GLB, one placement anchor, and one gameplay reference for multi-mesh props. This wave changes how their editable source is authored, not that runtime contract.

The newer [asset ingestion and promotion design](../asset-library-ingestion/design.md) and [plan](../asset-library-ingestion/plan.md) own the shared promotion/catalog infrastructure:

- private provider tooling landed in assets#142, while assets#141 still owns first-batch acceptance;
- the local calibration lab landed in web#937;
- web#936 still owns generated exact-ref catalog consumption and the real Builder/game round trip;
- its first source recipe deliberately excludes assemblies and companions.

This is the separately earned composed-source extension. It must not disguise a derived assembly as a raw FBX/pack-cache source, bypass existing validation, or maintain a second production prop catalog. No implementation takes over the active #141/#936 branches. Their unchanged v1 source recipes and checks must continue to work.

## Authoring and publishing are distinct actions

The current World Building concept remains the editor. A scene or saved arrangement is editable authoring data; it is not a playable dungeon payload.

For the first publication, a provider command consumes the exported scene plus explicit, reviewed source bindings and an anchor choice. The private provider retains those inputs. After the real export/promotion/consumer path is proved, a browser-facing **Make placeable prop** action can prepare this recipe. This slice introduces no filesystem-writing browser service, asset-upload API, daemon, or hosted publication service.

The pipeline performs these stages:

```text
scene JSON + bound source assets + chosen placement anchor
  -> reproducible composed GLB in a disposable stage
  -> geometry/material/anchor and visual verification
  -> existing provider stage/validate/apply/check lifecycle
  -> existing generated exact-ref web catalog
  -> Dungeon Builder placement
  -> Save, reopen, Save & Play, same prop in the game
```

A completed export is a candidate, not a published or visually approved asset.

## Private source recipe

The provider stores the original scene bytes, a binding document, and a small assembly recipe under `library/assemblies/<assembly-id>/<version>/`. No user Downloads path enters a tracked recipe. Raw source GLBs and existing promoted components remain unchanged.

The assembly recipe names:

- its schema version and stable assembly ID;
- source-scene relative path and SHA-256;
- source-binding document relative path and SHA-256;
- explicit anchor item ID;
- the versioned authoring-render contract used to interpret transforms.

The binding document maps each referenced semantic key to the exact primary GLB path/hash and ordered companion paths/hashes used by the authoring view. It also records the provider revision and consumer/render-contract identity used to establish those bindings.

This matters because the current scene export contains semantic refs, not exact variant IDs or content hashes. The importer must not silently resolve today's mutable family default and assume it was what the author saw. The first scene receives an explicit binding receipt checked against the actual World Building source/served assets. Future richer exports may supply that receipt themselves; the existing scene envelope is not silently redefined.

Paths must be portable and confined to reviewed provider inputs. Hash drift, missing components, unknown refs, invalid identities/transforms, cycles, incompatible render-contract versions, or unsupported animated/skinned sources fail before canonical mutation. A role, source, or scale is never guessed from a display name.

## Coordinate and anchor contract

World Building currently renders each primary variant with:

1. its loaded GLB node transforms;
2. a primary-mesh bounds-based floor/center correction;
3. the shared `SYNTY_SCALE = 0.75`;
4. its authored positive-Y yaw and continuous scene position;
5. the shared dungeon surface lift, which is not stored in scene Y.

Companions receive the primary's same correction, scale, position, and yaw. They do not calculate a separate centering correction. Both candle placements therefore need the same complete bound candle visual, including its companion GLB.

For this first floor-standing source class, the selected anchor must be a grounded prop at authored Y zero. The assembly root uses that item's authored position. Every child's authored yaw remains baked into the composition; the exporter does not guess a new forward direction or recenter the complete assembly around its combined bounds.

Let `q` be a source GLB point after that GLB's own node transforms, `a_i` the primary's raw-unit bounds correction, `p_i` the authored item position, `p_a` the chosen anchor position, `R_i` its Three.js-positive-Y authored rotation, and `S = 0.75`. The exported point, before the game's shared scale, is:

```text
q_out = (p_i - p_a) / S + R_i * (q + a_i)
```

The same formula applies to a companion using its primary's `a_i`. At a runtime placement origin, shared `PropModel` applies `S` and the dungeon surface lift once. Thus the assembled runtime geometry equals the source arrangement translated to the selected anchor, with no specimen-specific consumer repair.

The exported file has identity runtime node transforms where required by the provider's static-output convention and preserves named component provenance where practical. Blender import/export axis conversion is an implementation detail verified against independently decoded glTF vertices and Three.js expectations, not a second authoring coordinate system.

The full-assembly AABB center need not equal the placement anchor. Assembly validation must prove the chosen anchor and expected bounds rather than forcing the existing single-source centering rule onto the finished composition. The existing v1 normalizer's centering/grounding gates remain unchanged.

## Extension to the existing promotion lifecycle

Introduce an explicit composed-source branch in a version-2 provider batch recipe. Existing schema-v1 raw-pack recipes remain accepted and byte-stable; they are not rewritten.

A version-2 composed entry uses the same exact/family ref, display, role, themes, and explicit movement/LoS fields as the existing catalog path. Its source identifies `kind: composed-scene` and the private assembly recipe path/hash instead of claiming `packSlug`/FBX provenance. It has no freely editable second calibration transform: the bound composition and anchor are the visual authority.

`stage` resolves and verifies the complete recipe dependency closure, builds the composed GLB through headless Blender, validates the static output against the assembly coordinate contract, and generates the existing combined prop manifest, compact `synty-props-web.json`, mesh statistics, and complete inventory. It mutates no canonical runtime files.

Reuse generic finite-geometry, material/texture, path/hash, output-budget reporting, and static-output validation from the existing provider tooling. Keep the source-specific checks separate: a raw-pack source is normalized by its calibration recipe; a composed source is baked by the equation above. Do not introduce a plugin registry or generalized processing framework for two cases.

`validate` repeats the exact source/output binding and metadata checks. `apply` remains an explicitly authorized, whole-batch atomic operation using the existing provider mechanism. `check` derives expected output metadata from both supported recipe versions and detects drift. Source or output changes after review invalidate that review; no stale stage can be applied.

The new source kind is limited to the proven static, grounded visual-assembly case. It is not permission to accept animation, physics, mounts, arbitrary runtime companion behavior, or unknown glTF extensions. Unsupported sources fail explicitly.

## Gameplay and consumer contract

The composed prop keeps the existing skeleton-table's explicit movement/LoS behavior for the first specimen: movement blocked, sight not blocked. Its existing role/footprint metadata is recorded explicitly and checked against the current provider, never inferred from new decorative bounds. One gameplay placeable does not make every visual vertex a separate occupied cell.

The output's exact ref maps to one published GLB through the existing generated catalog path owned by web#936. Family aliases resolve only their reviewed default and do not create duplicate palette tiles. The new exact ref is what new Builder placements serialize. Unsupported exact refs do not substitute another visual.

Consumer integration waits for that shared generated-catalog seam instead of temporarily adding a second hand-maintained resolver. No toolkit/API/proto change is expected for the opaque prop ref, but the real round trip must prove this; discovering a parser/contract limit is a stop-and-escalate condition, not permission for a client workaround.

The source recipe stays editable in the asset builder; a published runtime prop is one object in the dungeon builder. Editing the source, renaming a local arrangement, or deleting a library entry cannot mutate published snapshots or already-stamped copies.

## Verification and delivery gates

1. Preserve and hash the exact user scene. Bind the skeleton-table, books, candles, and candle companions to actual source bytes.
2. Test malformed/unsupported input, traversal/hash drift, repeated component instances, companions, nonzero yaw, non-origin anchors, and failure before canonical mutation. Use real source-independent synthetic glTF/Blender cases, not only arithmetic round trips.
3. Compare decoded exported geometry with the source-render equation, including the shared scale and anchor. Verify finite geometry, expected static transforms, materials/textures, bounds, and the original assets remaining byte-identical.
4. Render the source arrangement and single-GLB candidate through the shared web prop rendering path. Kirk compares the candidate with his composition before canonical provider apply/promotion.
5. Exercise stage purity, v1 compatibility, v2 composition, metadata generation, exact-ref identity, atomic apply/rollback, and `check` drift detection. Regenerate the existing reports; do not hand-edit generated metadata.
6. After approved provider publication, consume the exact merged provider revision through the existing web synchronization/generation path. No licensed GLB or screenshot source enters the public web repository.
7. In the actual Dungeon Builder, select the new prop, place/facing/offset it as one object, save/reopen, Save & Play, and verify the exact ref/artifact and composition in the game. This is the closing proof, not the earlier concept screenshot or a successful export command.

The canonical design PR stays open through implementation. After this design is approved, add `plan.md` to the same PR and file only the earned provider/source-extension and consumer-integration slices. Provider publishes before the consumer pins its merged revision. Follow the adjacent provider workflow's inline/bounded execution guardrail when changing its shared promotion/consumer seam; do not delegate into or commandeer its active work.

## Separate small UI follow-up

World Building library **Rename** and **Delete from library** belong to web#935, not this provider contract. Delete requires an explicit confirmation naming the saved arrangement and must preserve stamped copies and published assets. The user's existing local scene/library must not be cleared as part of testing or migration.

## Not in this slice

- a full environment/terrain/wall editor or waiting for additional source packs;
- relaxing one-prop-per-cell gameplay occupancy;
- individually targetable, lootable, destructible, or simulated component items;
- linked-template propagation or mutable published snapshot identities;
- arbitrary animation, skinning, mounting, runtime physics, or full XYZ tilt authoring;
- a hosted asset service or automatic publication from browser local storage;
- a second asset catalog or consumer-specific scale/pivot repairs.
