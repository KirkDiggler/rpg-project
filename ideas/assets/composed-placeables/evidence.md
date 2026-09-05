# World-building architecture evidence

Read-only investigation, 2026-09-05. This records current facts and unresolved measurements, not an implementation verdict. See [the decision frame](design.md).

## Source pins

| Seam | Inspected revision |
| --- | --- |
| Toolkit | `23eda87beb1709f194bc8ee0827173c7543e6e27` |
| API | `d4cbcab0d36fed09ae3f65347de2d78ab643b639` |
| API-pinned protos | `1bb4fe24891a380d86319a10c81ef3b1800ba355` |
| Latest fetched protos, distinguished from API pin | `365d579b7566b5074206955806e6a9779e05c755` |
| World Building/shared renderer | `6ecbf1613235b571b7967de39b437f294bacb66d` (concept subsequently squash-merged in web#938) |
| Private asset-provider baseline | `45fc7798c8037029e6d0260b2d300e09700ae136` |

The API pins encounter/session v0.59.0. The toolkit root snapshot and the API's independently tagged modules are identified separately; latest proto fields are not presumed adopted by that API.

The first engine scout exceeded its 30-minute limit without a report and made no tracked/index/untracked changes. A bounded native-protocol retry completed the missing evidence. The parent then directly checked the admission-policy and canvas-construction code to resolve an important uncertainty in that report. All source investigation was read-only.

## Engine: plurality is supported by storage, restricted by admission

`tools/spatial/room.go:26-28` stores occupancy as `map[Position][]string`, with entities and positions keyed separately by entity ID. `GetEntitiesAt` returns all occupants. There is no fundamental one-entity slot in this index.

There are nevertheless multiple active constraints:

- `rulebooks/dnd5e/encounter/dungeonspec/validate.go` rejects duplicate authored placement cells and conflicting start/placement declarations.
- `encounter/compilefield.go:368-407` independently rejects two authored props at one cell.
- `encounter/field.go:253-296` declares an integral floor-cell location, an eight-compass presentation facing, and visual-only offsets. `dungeonspec/validate.go:693-704` limits the two planar offset components to `[-0.5, 0.5]` and the height component to `[0, 3]`.
- `encounter/compilefield.go:740-768` constructs the canvas by calling `BasicRoom.PlaceEntity` for each prop in declaration order. Physical-index IDs are `prop-<index>`, distinct from authored placement IDs.
- `tools/spatial/room.go:438-460` rejects placement if any existing occupant reports `BlocksMovement()`. It does not distinguish static scene registration from creature movement admission.

**Consequence:** removing the two explicit duplicate-cell guards would not create a sound multi-prop contract. With those guards removed, adding nonblocking candles before a blocking table can be admitted, while adding them after that table is refused. This order-dependence is source-derived, not a production failure claim: today's earlier guards keep that construction input unreachable.

A proper change must decide what world construction/registration validates, what spatial membership stores, and what movement admission asks. It must not globally disable movement blocking to make authoring overlap succeed.

## Engine: behavior, visual refs, and instance identities already differ

`PropInput`, `PropData`, and `AtlasProp` separate an authored placement ID from the opaque content/visual ref. Movement and LOS flags are explicit required booleans; behavior is not derived from the ref (`encounter/field.go:145-296`, `data.go:258-296`, `atlas.go:185-219`). A holdable prop requires an ID; an unaddressed scenery prop may have none. That existing separation should be preserved rather than replacing every mesh with a new rules entity.

The interfaces named `Standing` and `Sight` do not own prop geometry: the inspected `Standing` supplies downed member state, and `Sight` supplies member sight distances. Actual spatial checks consult geometry and blocking capabilities. `BasicRoom.blocksLineOfSightUnsafe` returns true if any indexed occupant blocks LOS (`tools/spatial/room.go:653-669`), subject to the higher-level lane/edge LOS rules. Multiple opaque objects in one cell are not automatically a wider wall.

## Runtime projection already permits same-cell dropped props

`encounter/holdings.go:435-454` drops every carried PropID at the departing member's cell. `propPlacements` folds state into a map keyed by PropID, so different props may have equal positions (`:315-339`). Atlas applies those locations independently (`atlas.go:343-360`).

This proves that projected runtime prop locations can overlap even though authored construction refuses the same layout. It does **not** prove that every physical-index/mutable-blocker path already implements general multi-occupancy: the observed holdings fold does not itself update the BasicRoom occupancy index. Authoring, runtime placement projection, and physical indexing must be tested together instead of assuming they share a single admission path.

## Deployed lifecycle: current authoring is not a publishing system

At the inspected API, `PutDungeon` takes a key, YAML, and validate-only flag. `internal/dungeons/registry.go:90-192,282-392` compiles source, writes it through a temporary-file rename, and replaces a keyed in-memory entry. `GetDungeon` returns source bytes. The same key can be overwritten.

This seam has no authored-content owner/scope, immutable published revision, publish state, or asset-definition/artifact store. Authoring is feature-gated and authenticates the caller, but caller identity is not passed into the registry as an ownership decision (`internal/handlers/dnd5e/authoring/v1alpha1/handler.go:51-60`, `internal/orchestrators/authoring/orchestrator.go:21-107`). This is a finding about the authoring seam, not a claim that all authenticated gameplay/session operations lack access checks.

Starting a run copies the compiled world into persisted encounter state (`internal/orchestrators/lobby/start_encounter_session_stack.go`, `internal/sessionworld/sessionworld.go`). Existing runs retain copied game facts, but the source key/revision is not retained as an immutable content provenance pointer. A mutable visual catalog behind the same opaque ref could still change the appearance of an old run.

The pinned proto already has separate `AtlasProp.id` and `AtlasProp.ref`, and repeated AtlasProp records. It has no published assembly, GLB upload/artifact, or asset-revision retrieval contract. The deployed content lifecycle is work required by every proposed publishing representation; neither JSON nor GLB removes it.

## Renderer: what the actual table costs before any bake

The supplied five-placement scene resolves to seven GLB occurrences but four unique URL/cache resources:

| Resource | File bytes | Primitives | Materials | Triangles |
| --- | ---: | ---: | ---: | ---: |
| Existing skeleton-table | 702,228 | 2 | 2 | 7,028 |
| Candles primary | 114,616 | 1 | 1 | 133 |
| Candles companion | 101,400 | 1 | 1 | 1 |
| Book pile | 373,272 | 1 | 1 | 344 |

Static inspection totals:

- 1,291,516 bytes across the four unique source GLBs;
- eight model mesh instances / draw-call candidates for this composition;
- 7,984 instance-expanded triangles;
- repeated candles/books already reuse URL-keyed loader resources;
- identical atlas image bytes appear in three separate GLBs, while the current code does not deduplicate textures across those URL-local resources.

`PropModel` uses `useGLTF` plus `scene.clone(true)`, which shares geometry/material resources for clones but does not merge meshes or create InstancedMesh batches. Companions are additional loaded siblings. See web `PropModel.tsx:140-224` and `propManifest.ts:248-325`.

These are resource/file/graph measurements, **not measured frame times or a claim of exactly eight draw calls per complete frame**. Transparency, passes, visibility, lighting, and renderer behavior affect actual work. A static texture-size estimate suggested four 1024-square decoded texture resources, but actual GPU allocation is unmeasured. `renderer.info.memory` exposes geometry/texture counts, not a material-memory counter; material instances need separate traversal and memory needs an explicitly labeled estimate or profiling method.

The older two-entry `synty-web-assets.json` visual-anchor catalog is not the whole prop catalog. Do not equate its enrollment count with all supported assets. The newer compact prop-catalog path belongs to the separately evolving #365/#936 workflow; the inspected concept still uses the legacy mirrored prop resolver.

## GLB generation is feasible, not automatically an optimization

The installed Three r181 addon `examples/jsm/exporters/GLTFExporter.js` supports `parseAsync(scene, { binary: true })`. A deployed browser can generate a static GLB and upload it at publication time. The implementation uses browser canvas/Blob/FileReader facilities; main-thread/export memory limits still need measurement.

The exporter caches by object identity and does not automatically content-deduplicate separate same-byte textures or merge ordinary cloned meshes. InstancedMesh can use its supported instancing extension, but current PropModel clones are not InstancedMesh. ShaderMaterial/custom rendering behavior is not automatically portable into glTF, and semantic groups/support/game-object identities do not become a durable gameplay contract just because nodes are exported.

One GLB may reduce separate network resources. It does not by itself reduce primitive draws, material changes, GPU textures, or preserve all future rendering features. Geometry/material consolidation and instancing are separate render choices. Server generation offers more controlled processing but adds a deployed build/status/resource-management responsibility; a private workstation pipeline is not a deployed publisher.

## Discriminating next proofs

Before committing implementation scope, use small tests/measurements that distinguish designs:

1. Register two named props at the same cell, in both declaration orders and with every blocking combination. Assert construction, membership, movement admission, and LOS separately. Include a blocking table with nonblocking decoration.
2. Exercise authored props and held/dropped props through save/load, Atlas, replay and stable targeting IDs. Check physical index behavior rather than only rendered positions.
3. Separate one gameplay object with a multipart appearance from two independent world objects at the same cell. Both should be expressible without file packaging deciding object identity.
4. Publish an immutable definition, start a run, publish a changed revision, then load both old and new runs/clients. Check ownership and which exact content each resolves. Current key-overwrite authoring is not this contract.
5. Compare the same visual source as shared model instances, a plain exported GLB, and an actually optimized/instanced representation under the same camera. Measure cold/warm requests, renderer calls/triangles, resource counts, export/load time and long tasks. Sweep repeated arrangements and diverse asset families on representative devices before choosing budgets or mandatory baking.

No source implementation, publish operation, engine policy change, or hardware-performance benchmark was performed by this investigation.
