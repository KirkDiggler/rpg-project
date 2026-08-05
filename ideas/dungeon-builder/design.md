# Dungeon Builder: the in-game authoring loop

**Status:** approved target; delivery tracked by rpg-project#175 and PR #181

The canonical target is the current versioned spec under `ideas/dungeon-builder/spec/` — not a fixed version, so a future spec bump needs no edit here — superseding v0.1 and the retired [Specimen Pack v0.2 comment](https://github.com/KirkDiggler/rpg-project/issues/175#issuecomment-5185751479) as grammar authority. v0.1 remains historical delivery evidence only. The dungeon document remains `version: 1`.

## Authority and model

The editor writes canonical YAML; the toolkit validates, compiles, and owns rules; the API projects and orchestrates; the web renders authorized server truth. Current prototype behavior is not a compatibility contract.

Physical geometry is independent of semantic regions. The prerequisite canvas is `canvas: {width, height}`, origin `[0,0]`, columns `[0,width)`, rows `[0,height)`, with positive dimensions. Source YAML supplies dimensions; the toolkit derives every in-bounds odd-q cell as structural floor. Authoring projection exposes those explicit canonical cells and canonical physical edges. There are no holes, masks, levels, rooms, connectors, or procedural-compatibility requirements in this target. Runtime is fog-authorized: it must not leak hidden extents.

Regions are a flat authored list of complete absolute extents. The parent is the smallest strict superset; the innermost region is the smallest containing region. Disjoint regions are siblings; equal or partial overlap is invalid; no parent is authored. The unpainted root area remains valid content space. `archetype` is optional and inherits when omitted; explicit empty or unknown values reject; vocabulary is `entrance | chamber | corridor | boss`. A root may resolve without an archetype absent a consumer default. Source extents are authoritative; persistence includes derived parent/index, and Load recomputes and rejects missing or disagreeing derivations.

Painting, repainting, or deleting a region cannot alter structural floor, canonical edges, endpoint-derived door IDs, content, or content validity; removed semantics fall back to the enclosing scope/root. Existing room-based semantic lookups are not renamed: after an existing path has an absolute location, it deliberately resolves scope from that location.

Physical door identity is normalized from its endpoints and is region-independent. A physical lock, when authored, uses `lock.options` with one or more alternatives, each with a valid ability and positive DC; alternatives are ORed. Omitted lock means initially unlocked. Empty options and locks on non-door edges reject. Authoring projects every option when it exposes lock configuration; runtime gameplay may remain interaction-opaque. Lock behavior is a separate #175 follow-up, not Wave 0 or #180 acceptance.

## Delivery waves

**Wave 0 — roomless canvas vertical (prerequisite).** Deliver canvas dimensions, toolkit-derived floor cells, and edges in authoring proto, toolkit validation/compile/persistence, API projection, and fog-safe runtime geometry. Use unlocked doors in executable fixtures. This wave has no semantic-region scope.

**Wave 1 — semantic scopes (#180).** Deliver region extents, derived containment and index, innermost/scope-path resolution, archetype validation, persistence checks, authoring projection, `Zone.parent_id`, and fog-safe runtime projection. Do not add future properties, hidden extents, or web implementation scope.

For each wave, proto contract merges/releases before API consumer development/evidence; API states the provider need; toolkit releases; API pins released providers and merges last. One issue/branch/PR per repo per wave. None are created by this dispatch. PR #181 is the approved exception: it may merge before implementation so canonical docs and the living handoff reach `main`; Kirk alone decides and merges.

## Non-goals and validation

No room-first compilation authority, room-derived placement/spawn/door identity, room chains, connectedness requirement, rectangle compatibility, procedural compatibility, lighting/audio/trigger capabilities, new boss/spawn rules, geometry/traversal changes, holes, levels, lock behavior, or web implementation in #180. Preserve only a generic future extension seam and current location-to-scope lookup. #176–#179 are delivered/open history, not reopened. Validation covers positive dimensions, complete floor membership, odd-q adjacency for edge endpoints, normalized duplicate edges, lock option rules, region containment/overlap rules, archetype vocabulary, and derived-state Load agreement.

## Evidence and pointers

Delivered backend history: [rpg-api-protos#206](https://github.com/KirkDiggler/rpg-api-protos/pull/206), [rpg-toolkit#881](https://github.com/KirkDiggler/rpg-toolkit/pull/881) with tags `tools/spatial/v0.6.0`, `rulebooks/dnd5e/v0.71.0`, and `encounter/v0.48.0`, and [rpg-api#769](https://github.com/KirkDiggler/rpg-api/pull/769) are delivery evidence (comment [5183123668](https://github.com/KirkDiggler/rpg-project/issues/179#issuecomment-5183123668)). #176–#179 remain OPEN on GitHub but are delivered history; web is independently owned. Acceptance is schema/toolkit/API wire and persistence evidence for the applicable wave; no current implementation or test evidence is claimed here. See #175, #180, and PR #181 for the live gate.

Use the active role signature for future edits; do not hard-code an asset-pipeline signature.
