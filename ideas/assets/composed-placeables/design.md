# World composition library — accepted design

Design slice: [rpg-project#378](https://github.com/KirkDiggler/rpg-project/issues/378), under the still-open [Composable Dungeon Builder journey #169](https://github.com/KirkDiggler/rpg-project/issues/169).

**Status (2026-09-07): shipped and accepted for the bounded local/development slice.** API [#924](https://github.com/KirkDiggler/rpg-api/pull/924) and web [#954](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/954) are merged. This document records what was actually delivered, not the earlier proposed publication/revision framework.

## Accepted design

### A composition is simple world-owned data

The domain owner is `WorldID`. A saved composition is the toolkit's deliberately small `world/composition.Data`:

```go
type Data struct {
    ID      string
    WorldID string
    JSON    json.RawMessage
}
```

The API treats `JSON` as valid but otherwise opaque authored source. It does not define a typed composition schema, interpret asset semantics, or turn this slice into a general content framework. The web owns the current scene envelope and validation.

The local API receives both authenticated caller identity and `world_id`. The current alpha handler accepts only its configured world. `WorldID` remains the ownership boundary; an eventual verified Discord guild/header-to-world mapping belongs at the trusted handler/server edge, before repository access. The repository must not authenticate callers or derive worlds from Discord data.

### The library is immutable-create, read, list, and permanent delete

The protobuf service has `CreateComposition`, `GetComposition`, `ListCompositions`, and `DeleteComposition`. Records contain `id`, `world_id`, and a JSON string.

- Create assigns a fresh opaque ID and stores one immutable snapshot. Saving an edit creates another ID; there is no update or mutable head.
- Get and List are scoped by `WorldID`.
- Delete is permanent and idempotent when the record is already absent.
- There are no revisions, head pointers, receipts, TTLs, Lua operations, archives, tombstones, or reference counts.

Redis stores one hash per world at `composition:<WorldID>`. The composition ID is the hash field. Create uses `HSETNX`; Get/List/Delete use `HGET`, `HGETALL`, and `HDEL`. This follows the existing repository/orchestrator/handler boundaries without introducing a generic document store or adding composition behavior to the dungeon file registry. See [API reuse](api-reuse.md).

### One world prop can have a multipart appearance

A composition source keeps authored part IDs, asset refs, labels, transforms, optional group/support relationships, and optional visual point-light declarations. Saving freezes those ref strings and authoring facts in the JSON snapshot; it does **not** copy or bind the referenced GLB bytes.

The dungeon builder places a saved composition through the existing opaque prop ref as `composition:props:<composition-id>`. Each placement has its own required placement ID, cell position, facing, and YAML/Atlas lifecycle. Resolving that ref renders every authored part beneath one placement root. The part IDs remain distinct for rendering and authoring, but they do not become independent gameplay entities.

Deleting a library record never rewrites a dungeon. Existing references remain visible, selectable, and explicitly marked deleted/missing until the author removes them. This preserves authored dungeon intent without inventing archival or referential-integrity machinery.

### One editor, with distinct local and world data

The existing World Building editor is reused from the development main menu as **World Builder**; no second editor or scene dialect was created. Its local scene draft, local arrangement library, import/export, continuous placement, grouping/supports, and transform tools remain available.

The world library is separate. Its real RPC-backed controls save, list, open, and permanently delete immutable composition snapshots. Opening a world snapshot preserves the latest local draft and marks the open workspace as world-origin; subsequent edits do not overwrite that local draft unless the author explicitly chooses **Save local draft**. This fixes the earlier draft-origin overwrite failure.

World-library rows and dungeon palette entries use authored names and tooltips. Client thumbnail generation is serialized through one R3F renderer and cached by world plus complete immutable snapshot; the implementation handles the renderer's observed asynchronous setup rather than claiming synchronous generation or a server thumbnail service.

The dungeon builder retains its existing real Save and Save & Play path. Composition placement survives YAML save/reopen and Atlas/play resolution with its own placement identity, ref, facing, and offset.

### Visual point lights are optional presentation data

Each authored part may carry an explicit point light with:

- `enabled`;
- a part-local offset;
- `#RRGGBB` color;
- render intensity in the supported `0..20` range; and
- range in scene units in the supported `0.01..24` range.

Offsets rotate with the part, then the full composition placement is applied once, including the same dungeon-surface alignment as the rendered multipart prop. A shared collector chooses at most 12 lights for the scene using the existing nearest-to-view policy. These lights illuminate rendered meshes only: they do not create gameplay illumination, visibility facts, shadows, flicker, or crypt floor-light pools.

## Why this boundary

The shipped shape is intentionally functional and small. It earns reuse at explicit seams:

- toolkit owns the transport-neutral data envelope;
- API owns world-scoped persistence and caller/world checks;
- web owns the current authored JSON and rendering behavior;
- dungeon YAML keeps gameplay placement identity separate from visual-part identity.

JSON remains editable source. A GLB is neither required nor treated as a semantic authority, and a single GLB would not automatically reduce draws, materials, or textures. The slice also does not claim that logical asset refs are permanent byte bindings. Those decisions can be made from measured needs later without pretending a hypothetical final-version framework is already approved.

## Explicitly open

Do not treat these as implemented:

1. production/header-derived `WorldID` authorization, including verified Discord guild mapping, before promotion to `main`;
2. durable Redis composition data across a full local-stack teardown;
3. local-template/arrangement versus world-library UX, including missing local arrangement deletion;
4. pack grouping and any custom asset drawer;
5. absolute asset-version/byte binding and retention for saved source refs.

The destructive API CI-check problem already has issues [rpg-api#906](https://github.com/KirkDiggler/rpg-api/issues/906), [#901](https://github.com/KirkDiggler/rpg-api/issues/901), [#883](https://github.com/KirkDiggler/rpg-api/issues/883), and related records; this design does not create another duplicate.

## Delivery

- Toolkit `world/composition.Data`: [rpg-toolkit#1543](https://github.com/KirkDiggler/rpg-toolkit/pull/1543), consumed as `world/v0.4.1`.
- Proto create/get/list contract: [rpg-api-protos#300](https://github.com/KirkDiggler/rpg-api-protos/pull/300); permanent delete: [#304](https://github.com/KirkDiggler/rpg-api-protos/pull/304).
- API world library: [rpg-api#924](https://github.com/KirkDiggler/rpg-api/pull/924).
- Web editor, world library, placement, rendering, lights, and delete UX: [rpg-dnd5e-web#954](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/954).
- Earlier editor concept: [rpg-dnd5e-web#938](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/938).

Exact revisions and verification are recorded in [evidence.md](evidence.md).
