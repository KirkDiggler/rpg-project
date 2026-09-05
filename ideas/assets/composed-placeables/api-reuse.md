# Existing API seams for composition definitions

Focused read-only follow-up, 2026-09-05. API source is pinned to `d4cbcab0d36fed09ae3f65347de2d78ab643b639`; web authoring was inspected at `859c2be8506ea8b23b7673503c563efcfd86a02d`. The API uses proto `1bb4fe24891a380d86319a10c81ef3b1800ba355`. The parent directly checked the character Redis implementation, Redis client interface, repository DI and authoring orchestrator after the scouts returned.

## Redis is already the API's entity-store convention

`internal/repositories/character/redis.go:77-150` marshals entity records to JSON and stores them with ordinary `SET`, with expiry zero. A Redis set indexes records by player. `internal/repositories/character/repository.go` defines typed Create/Get/Update/Delete/List input/output contracts. The package owns its Redis implementation and generated mock.

`cmd/server/server.go:154-180` creates one Redis client and injects it into the character, draft and dice repositories. `internal/redis/interface.go:14-16` embeds `go-redis.UniversalClient`; transactional pipelines, WATCH and scripts are already available. Equipment patching already uses a digest plus WATCH/TxPipelined; session presentation already uses an atomic Lua idempotence/conflict operation.

Characters are the closest durable JSON precedent. Character drafts have a different lifecycle: one replaceable draft per player and a 24-hour record expiry. Lobby/session/presentation TTLs are likewise domain-specific, not an API-wide policy for every Redis value.

No new Redis module or generic document-store framework is required to persist a composition definition. Follow the existing domain-repository/DI convention. The current runServer entity-repository wiring is Redis-based; other database names in repository guidance are examples, not a reason to introduce one here. This inspection did not establish production Redis persistence/topology.

Do not copy character Create's implementation blindly for immutable publication: its EXISTS check precedes its transaction. Primary/index mutations are transactional, but create-if-absent is not atomic as a whole. If immutable creation or CAS is part of the chosen definition contract, implement that operation using the already-exposed Redis primitives and test the actual race semantics. This is a definition-specific requirement, not a request to refactor unrelated repositories.

## The builder's existing path

```text
DungeonBuilder -> authoringRpc -> AuthoringService handler
  -> authoring.Orchestrator -> dungeons.Registry.Put
  -> sessionworld.Compile -> Manager.AtlasOf
  -> FieldErrors or compiled Atlas
```

Save uses the same path without validate-only. `FileRegistry.Put` persists the original YAML bytes only, via a temporary-file rename, and replaces its keyed in-memory compiled entry. Open uses lobby ListDungeons plus authoring GetDungeon. Save & Play saves first, then starts a lobby encounter by dungeon key. Lobby and authoring share the same dungeon registry instance.

Sources: web `src/author/{authoringRpc.ts,AuthorView.tsx,DungeonBuilder.tsx}`; API `internal/handlers/dnd5e/authoring/v1alpha1/{handler,put_dungeon,get_dungeon}.go`, `internal/orchestrators/authoring/orchestrator.go:21-107`, `internal/dungeons/registry.go:90-160,282-391`, `cmd/server/server.go:272-368`.

The reusable patterns are thin handler translation, explicit layer Input/Output types, injected domain dependencies, domain-error/status mapping, and path-addressed validation errors when useful to authors. `PutDungeon` intentionally distinguishes transport/target errors from compiler defects returned as a FieldError batch.

## Keep the dungeon registry specific

`dungeons.Registry.Entry` contains exact YAML, a compiled Dungeon and its Atlas. The registry also owns default-tomb, key-as-filename, startup compilation and playable-list invariants. Its `List` is a playable dungeon catalog, not a generic list of drafts or definitions.

A composition definition belongs behind a separate domain repository/catalog interface, not new unrelated methods on `dungeons.Registry`. The existing Redis client and server DI can back that interface. Reusing repository conventions is appropriate; turning the dungeon registry into a generic content framework is not.

Extending the AuthoringService RPC surface can be coherent for builder editing operations. That does not mean all definition reads should share its deployment gate: runtime consumers must be able to resolve usable content when authoring is disabled, just as ListDungeons already lives on the runtime lobby path. Service placement and repository ownership are separate decisions.

## Concrete next contract decisions

The storage mechanism fits existing code. The remaining decisions concern meaning:

- Is a composition copied into the dungeon on placement/save, or referenced by an immutable definition identity/revision? Both must preserve the already-approved independent-copy behavior.
- Where does expansion/resolution occur: authoring compilation, encounter start, or visual definition retrieval? Give that consumer a narrow read-only resolver, not write/lifecycle ownership.
- Does Save make a valid composition available, or is a separate Publish operation actually required? Do not add draft/publication workflow solely because the earlier proposal used those words.
- What owns the library: author, Discord-server scope, or an explicitly global catalog? Existing per-player indexes are a storage precedent, not a product decision.
- What do rename/delete affect, and which snapshots/references must remain valid afterward?

These decisions determine the new repository and RPC contracts. They do not require reopening the choice of database technology or explaining ordinary API/Redis plumbing.
