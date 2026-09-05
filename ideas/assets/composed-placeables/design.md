# Deployed world-building composition — architectural decision

Design slice: [rpg-project#378](https://github.com/KirkDiggler/rpg-project/issues/378), under [Dungeon Builder journey #169](https://github.com/KirkDiggler/rpg-project/issues/169).

**Status: reopened for architectural comparison. No implementation choice is approved.** The previous private-pipeline publisher proposal at `f8d100d` is withdrawn as a recommendation. It assumed a developer-assisted workflow where the user needs a deployed authoring product, and treated one-prop-per-hex as a fixed requirement instead of examining it.

## Governing principle

Kirk, 2026-09-05:

> We are not here to make things work. Full stop. We are here to make composable, extensible components that can build a solid foundation that will allow our game to evolve on.

The future feature path is unknown. A workaround that satisfies the current table example is not sufficient justification for the architecture. Neither is a speculative universal framework. The design must identify focused parts, explicit ownership, and contracts that let one part change without forcing unrelated parts to change. This standing principle is recorded in [the shared startup context](../../../CLAUDE.md#design-principle--composable-foundations).

## What the product must support

- DMs/streamers author through the deployed web application, initially on Kirk's Discord server.
- An author composes existing assets, saves the editable source, and makes reusable content available to the dungeon builder and players.
- Publishing cannot depend on a developer committing a recipe to a private repository or running Blender on their workstation.
- The current concept's scene/library files prove editing and reuse, not deployed persistence, authorization, publication, or delivery.
- The first composition may behave as one gameplay prop. Individual pickups/interactions are not required now, but file packaging must not silently decide the domain model.
- Changing engine placement rules is allowed if it is the better foundation. Avoiding such changes is not an acceptance criterion.
- The private base-asset ingestion pipeline remains a separate responsibility. This decision must not commandeer its active work or violate its execution guardrails.

## Concrete forcing case

Kirk supplied a five-placement scene: one existing skeleton-table (skeleton and table are already one asset), two candles, and two book piles. Its SHA-256 is `c6e50c800d869eecaac74dac6670a37a30ba5bbbade49ca6ccead5ecd1cdf2c8`. It contains explicit transforms and no formal group/support links. The source asset set includes candle companion geometry.

The scene and its screenshot establish the intended appearance. They do not require a baked GLB, one runtime entity per mesh, or preservation of today's cell-exclusivity rule.

## Questions that must remain separate

1. **Authoring composition:** what is saved, how components are positioned/grouped, how saved content is copied, and how the source remains editable.
2. **Gameplay representation:** which things have identity/behavior, what an instance is, how occupancy and sight/movement contributions work, and which facts the engine owns.
3. **Visual representation:** how a published appearance references reusable base assets, how transforms/materials are represented, and whether render artifacts are derived from that source.
4. **Deployed content lifecycle:** who can save/publish/use content, where source and published revisions live, how dungeon/run snapshots resolve them, and how clients receive/cache them.
5. **Render execution:** scene nodes, primitives, materials, geometry sharing, instancing/batching, texture memory, loading, and frame cost.

JSON and GLB are representations. Choosing either does not answer the identity, ownership, or lifecycle questions. One GLB is not necessarily one mesh, one material, one draw call, or less GPU work.

## Options to compare

These are candidates, not preselected decisions. Some can be combined cleanly.

### A. Published JSON visual assembly, one gameplay prop

The deployed service stores an immutable assembly definition referencing bound asset versions and transforms. The renderer resolves it into a visual hierarchy; the engine sees an explicit gameplay prop/appearance reference.

Potential strengths: small publish payloads, retained structure, reusable source geometry/materials, no mandatory bake job. Questions: assembly resolver ownership, source/version binding, aggregate bounds versus gameplay footprint, caching, nesting policy, and whether this preserves a sound distinction between presentation components and game objects.

### B. Browser-generated GLB at publish time

The deployed editor exports a selected visual composition and uploads/registers the artifact. Editable source is retained separately; the runtime can consume a model file.

Potential strengths: reuse of a single-model delivery path, no mandatory server graphics toolchain. Questions: supported materials/effects, export memory and main-thread work, data duplication, untrusted binary validation, upload/storage quotas, reproducibility, and whether the generated file should be an authority or only a derived artifact.

### C. Server-generated GLB or other optimized artifact

The deployed editor publishes source data. A server-side build step derives the render artifact from trusted asset inputs, with explicit failure/status handling.

Potential strengths: controlled toolchain and resource validation, possible optimization, consistent generation. Questions: latency, resource isolation, retries/idempotence, storage and invalidation, asset access, and whether a bake is necessary for the first supported workload. A bounded build step does not imply a new generalized job framework or a Blender-specific domain model.

### D. Multiple props per hex with authoring groups

The engine represents multiple placements in a cell; authoring groups can be instantiated as multiple objects rather than one compound appearance.

Potential strengths: removal of an artificial exclusivity rule, independently addressable world objects where needed. Questions: actual core/index support, movement/LOS aggregation, placement offsets/heights, instance identity, targeting, snapshot/projection behavior, and whether visual-only decorations would acquire unnecessary gameplay identity. Removing one validation check is not proof this option is complete.

### E. Separate semantic source from derived representations

An authoring composition can produce gameplay declarations and visual representations through explicit independent contracts. A renderer may use source instances, a compiled GLB, batching, or another supported artifact without redefining the saved source.

This is a combination to evaluate, not a mandate to build every option or an abstract scene engine. Its value must be demonstrated by real current consumers and clear boundaries, not hypothetical future features.

## Decision criteria

Evaluate each candidate against:

- deployed author usability and complete save/publish/use lifecycle;
- coherent domain ownership and authoritative engine behavior;
- preservation of source editing, independent copies, and existing dungeon/run meaning;
- source and published identity/version binding without accidental mutation;
- ability to change rendering/packaging without changing gameplay semantics;
- concrete implementation and migration cost, including coupled invariants;
- observable loading/frame/memory cost, not file-count assumptions;
- clear failure, authorization, validation, and resource-limit behavior;
- testability and a bounded first implementation that earns its extensibility.

Current editor safety caps are not hardware or engine performance limits. Measurements must state scene contents, resource sharing, device/browser, camera/visibility, cold versus warm cache, and what was actually measured. Counts of meshes/primitives/textures are not measured FPS.

## Evidence gathering and next decision gate

The read-only investigation covers three independent evidence seams:

1. toolkit placement validation, core spatial storage, blocking/sight, identity and projection;
2. API/proto deployed content storage, authorization, reference and snapshot lifecycles;
3. web/shared-renderer and asset structure, GLB/export support, resource counts and meaningful benchmark candidates.

Reports must bind findings to source revisions, separate current behavior from unknowns, and identify discriminating tests. The comparison will be updated from those findings before recommending an implementation. Kirk reviews that recommendation; only then is an implementation plan added.

No publisher, runtime assembly resolver, uploaded-asset service, or engine occupancy change has been implemented. The merged World Building concept remains the practical authoring test surface.
