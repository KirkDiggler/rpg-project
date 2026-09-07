# Composition API and persistence — delivered seam

This note replaces the pre-implementation API investigation with the contract shipped by [rpg-api#924](https://github.com/KirkDiggler/rpg-api/pull/924). The earlier inspection correctly identified the existing Redis repository/DI pattern, but its proposed guild indexes, definition revisions, mutable heads, atomic Lua publication, and receipt machinery were not adopted.

## Data and wire contract

Toolkit `world/composition.Data` (world `v0.4.1`) is the stored domain value:

```go
type Data struct {
    ID      string          `json:"id"`
    WorldID string          `json:"world_id"`
    JSON    json.RawMessage `json:"json"`
}
```

The API validates that the payload is JSON, then preserves it without interpreting a typed web schema. API protos expose:

- `CreateComposition(world_id, json) -> composition`;
- `GetComposition(world_id, id) -> composition`;
- `ListCompositions(world_id) -> compositions`;
- `DeleteComposition(world_id, id) -> {}`.

The wire `Composition` has only `id`, `world_id`, and JSON string fields. Create assigns a caller-independent opaque ID. Delete is permanent and succeeds when the target is already absent.

## Repository shape

A separate composition repository owns persistence. It did not expand the dungeon registry or introduce a reusable document framework.

```text
composition:<WorldID>  Redis hash
  <CompositionID> -> JSON-encoded toolkit composition.Data
```

The operations are direct:

| Repository verb | Redis operation | Semantics |
| --- | --- | --- |
| Create | `HSETNX` | atomically refuse an existing field |
| Get | `HGET` | return one exact world/ID record or NotFound |
| List | `HGETALL` | decode the world's records and sort by ID |
| Delete | `HDEL` | permanent, idempotent absence |

Stored envelopes are checked against the addressed world and ID when decoded. There is no TTL. There are also no revision counters, head keys, Lua scripts, transactions, receipts, archives, tombstones, or reference counts.

## Ownership and authorization boundary

`WorldID` is the domain partition. Service inputs carry both `PlayerID` and `WorldID`; repository inputs carry only the world/record coordinates required for storage.

The current local/dev handler:

1. requires an authenticated player from request context;
2. requires the request `world_id` to match the server's configured world;
3. gates Create/Delete on authoring being enabled; and
4. passes the configured `WorldID` into the service.

This is an alpha selector/check, not production guild authorization. A future trusted header/Discord guild mapping must resolve and verify `WorldID` at the handler/server edge before repository access. Discord identity does not belong in the repository, and a client-supplied guild or world identifier alone is not authorization.

## Why this reused the right amount

The original read found durable no-TTL JSON records, typed layer inputs/outputs, thin handler translation, injected Redis dependencies, and domain-owned repositories already in the API. The delivered composition package reused those conventions and the shared Redis client.

Character ownership sets, character update behavior, draft TTLs, and the dungeon YAML registry have different domain lifecycles and were not copied. Atomic create-if-absent was earned directly with `HSETNX`; more elaborate WATCH/Lua machinery was unnecessary for the operations that shipped.

Runtime readers and the authoring UI share the narrow composition service/repository data, while mutation remains handler-gated. No separate publish state exists: each successful Save creates another immutable record.

## Delivery pins

- Toolkit [#1543](https://github.com/KirkDiggler/rpg-toolkit/pull/1543): merge `9fbb407e96ed4f328b3fc71b8636ef2f15e89dce`; API consumes `github.com/KirkDiggler/rpg-toolkit/world v0.4.1`.
- Protos [#300](https://github.com/KirkDiggler/rpg-api-protos/pull/300): merge `a4bfa7762cb64cc1ff62ec85649a5f4eaa9e2ad8`.
- Protos [#304](https://github.com/KirkDiggler/rpg-api-protos/pull/304): merge `266e11ca436e6b02bf08128e3968073817c2ba21`.
- API [#924](https://github.com/KirkDiggler/rpg-api/pull/924): reviewed head `757e6b79d59fe654a3cb1c3ba855c34d1e6e4419`, squash merge `7732bfb4342b59e7bfdc76927d5ee475533779aa`.

Production/header-derived world authorization and Redis durability across a full local teardown remain open; neither is delegated to this repository layer.
