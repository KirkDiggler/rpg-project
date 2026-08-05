# Dungeon Builder: implementation plan

**Parent:** rpg-project#175 · **Tracking:** PR #181

## Current authority

The current versioned spec under `ideas/dungeon-builder/spec/` — not a fixed version, so a future spec bump needs no edit here — is the target grammar and acceptance authority, superseding the retired [Specimen Pack v0.2 comment](https://github.com/KirkDiggler/rpg-project/issues/175#issuecomment-5185751479). v0.1 is historical delivery evidence, not the current target. The target separates physical canvas from semantic regions and keeps the toolkit authoritative.

## Delivery order

### Wave 0 — roomless canvas vertical

1. `rpg-api-protos` defines authoring canvas dimensions, projected canonical structural-floor cells, and canonical physical edges; release the contract. Source YAML supplies dimensions; the toolkit derives every in-bounds structural cell.
2. `rpg-api` states the provider need and develops the consumer projection only after the proto release.
3. `rpg-toolkit` derives and validates complete `[0,width) × [0,height)` odd-q floor coverage, normalized edge endpoints, door identity, and unlocked executable fixtures; release the provider.
4. `rpg-api` pins released providers, records wire/persistence/fog authorization evidence, and merges last.

No rooms, connectors, holes, masks, levels, procedural compatibility, or lock behavior acceptance belongs here. Physical lock grammar may be projected, but gameplay behavior is a later #175 follow-up.

### Wave 1 — semantic scopes (#180)

1. `rpg-api-protos` defines authoring extents plus toolkit-derived parent ID and runtime `Zone.parent_id`; release before consumers.
2. `rpg-api` states the provider need and consumes the released contract without deriving geometry or hidden extents.
3. `rpg-toolkit` compiles flat absolute extents, derives parent/index (parent = smallest strict superset; innermost = smallest containing region), validates strict containment/overlap rules and archetypes, persists source plus derived state, and releases.
4. `rpg-api` pins released providers and merges last after authoring/runtime wire and Load evidence.

Regions may be disconnected. Unpainted root area is valid. Painting, repainting, or deleting a region cannot alter structural floor, canonical edges, endpoint-derived door IDs, content, or content validity; removed semantics fall back. Existing room-based semantic lookups resolve scope from their established absolute location and are not renamed. No lighting/audio/trigger capabilities, new boss/spawn rules, geometry/traversal, room/connector/procedural compatibility, holes/levels, lock behavior, or web implementation scope is included. Preserve only a generic future extension seam and location-to-scope lookup. #176–#179 remain delivered/open history and are not reopened.

## Cross-repo rules and gates

One issue, branch, and PR per repo per wave; none are created in this dispatch. Develop outside-in only to name a provider need, then merge inside-out as proto → toolkit → API pin/evidence. The API never performs rule calculations; runtime exposes only fog-authorized region information. PR #181 may merge before implementation under the approved exception so these canonical docs and the living handoff land on `main`; Kirk alone decides and merges.

Delivered backend history is recorded by [rpg-api-protos#206](https://github.com/KirkDiggler/rpg-api-protos/pull/206), [rpg-toolkit#881](https://github.com/KirkDiggler/rpg-toolkit/pull/881) and tags `tools/spatial/v0.6.0`, `rulebooks/dnd5e/v0.71.0`, `encounter/v0.48.0`, and [rpg-api#769](https://github.com/KirkDiggler/rpg-api/pull/769); see delivery comment [5183123668](https://github.com/KirkDiggler/rpg-project/issues/179#issuecomment-5183123668). #176–#179 remain OPEN on GitHub but are delivered history; web is independently owned. Validation/evidence must cover the relevant proto contract, toolkit validation and persistence, API projection, and real wire state. This dispatch claims no implementation or implementation tests.

Use the active role signature for future edits; do not hard-code an asset-pipeline signature.
