# Courtyard S2 implementation plan

**Goal:** region base + explicit hex ground paint, from Studio through native save, immutable encounter snapshots, recipient-specific discovery and play.
**Spec:** [design.md](design.md), [walkthrough](README.md), [#874](https://github.com/KirkDiggler/rpg-project/issues/874).
**Authority:** operator approved S2 execution; discussion agreements are on #867. Direct execution, no implementer delegation. Merges, deployment to the operator's castle, and final visual acceptance remain operator actions.
**Finish line:** checked PRs, isolated early view, operator look, standing QA walk, independent review, released-pin integration and authorized merges. A working editor alone does not close S2.
**Stack:** TypeScript/React/Three.js, Python/Pillow asset tooling, Go encounter/session, protobuf/Buf, Go API.

## Constraints and baseline

One worktree per repository/line; one toolkit module per PR. Never commit licensed images to public repositories. No source GLB mutation, terrain movement, client-side visibility inference, local Go replacements or manually generated proto bindings. Keep the existing Lab on :3180 and castle on :3030 unchanged.

Inspected baselines: Web `2150256f`, Assets `fe6b5ee`, toolkit `9922e218`, API `590a201`, protos `0f27df9`. Project branch `design/867-courtyard-ground`. Implementation worktrees are `.worktrees/courtyard-874`; session gets its own toolkit worktree/branch when adopting the encounter provider. Existing Web region/draft/native-document baseline: 104 tests pass.

The actual Kingdom Grass 01 and Mud 01 color images already contain faceted variation. Use them as reviewed candidate surface images with world-space UVs, not inferred FBX material bindings or flattened volumetric props. The first in-game view is still required; no final visual approval of new resource bytes is implied.

## Shared contracts

### Authoring

New `RegionGround` in Web `regionGround.ts`:

```ts
interface GroundPaint { ref: string; cells: RoomCellRow[] }
interface RegionGround { baseRef?: string; paint?: GroundPaint[] }
```

`RoomRegion.ground?: RegionGround`. Missing base means the existing stone default; no synthesized base written into old documents. Empty/whitespace/null refs are refused. Paint cells are canonical row runs, belong to the containing region, and have exactly one explicit ref. Group output sorts refs lexically and uses existing `toRows`; do not remove paint equal to base. No-op edits retain draft identity. Empty paint is omitted by edits; parsing may preserve a structurally valid empty list. Unknown future source fields retain the existing editor's carry-through convention.

Pure edits: `setRegionGroundBase(draft, regionId, ref: string | null): RoomDraft`; `paintRegionGround(draft, regionId, cells: readonly RoomHexCell[], ref: string | null): RoomDraft`. Null resets base/erases paint; it is not serialized as a null reference. Resolve with `groundRefAt(region, cell): string | undefined`; undefined is existing renderer default. Off-region edits refuse atomically. UI brush clips to existing cells of the active region before invoking the edit. Floor removal/reassignment prunes departing-region paint through `withOwners`, never creates destination paint implicitly.

### Encounter / wire

`RegionGround { BaseRef string; Paint []GroundPaint }`, `GroundPaint { Ref string; Cells []spatial.Position }`; optional `*RegionGround` on `RegionInput` and `AtlasRegion`. Empty BaseRef is absent, not a newly invented asset. Construction uses authored offset cells, converts once to absolute, validates ownership/duplicates and deep-copies. `RegionGroundData`/`GroundPaintData` mirror this using `PositionData`. Old absent fields remain absent.

Dungeonspec `RoomRegion.Ground *RoomGround`, `RoomGround{BaseRef string, Paint []RoomGroundPaint}`, `RoomGroundPaint{Ref string, Cells []RoomCellRow}`; YAML/JSON spellings `ground`, `baseRef`, `paint`, `ref`, `cells`. It validates shape/nulls/refs/canonical rows/ownership, then lowers through the same coordinates as the region. Wall footing removed from the compiled region also removes its appearance assignments, without changing authored data.

Both proto packages add `GroundPaint { string ref=1; repeated Position cells=2; }`, `RegionGround { string base_ref=1; repeated GroundPaint paint=2; }`, and `AtlasRegion.ground = 6`. Values are immutable appearance, not navigation. Every projection trims paint to that projection's delivered region cells. Incremental patches union paint by cell while retaining previous permitted paint; no full-region paint map accompanies a partial region.

### Assets and rendering

New isolated `ground/manifest.json`: schemaVersion 1 and styles, sorted by ref. A style has `ref`, `displayName`, `diffuse` (contained `ground/images/<sha256>.png`), `sha256`, `width`, `height`, and positive finite `worldUnitsPerRepeat`. Candidate refs: `dnd5e:ground:dungeon:stone_01`, `dnd5e:ground:fantasy-kingdom:grass_01`, `dnd5e:ground:fantasy-kingdom:dirt_01`. Source-image content is private; public code carries metadata only. The provider verifies decoded PNG, hashes, dimensions, duplicate refs, source/output separation and exact owned output set. No prop schema relaxation.

Stone names the current crypt floor image and repeat 6.0. Initial grass/dirt candidate repeat is also 6.0; final visual scale is checked in the early view. Use original source resolution. First ground styles are color-only; source image facets provide the approved visual direction without depending on raised geometry or adding normal/PBR lighting semantics. Existing region lighting, fog, watch-zone and memory treatment remain authoritative.

`GroundCatalog` and loading hook resolve explicit refs; malformed/missing catalog or unknown explicit style produces a visible ground diagnostic, never fallback stone. Missing authored base preserves existing rendering. The shared renderer consumes resolved per-cell refs and uses absolute world XZ for UV phase, a single ground plane and non-overlapping cell coverage. Batch repeated style geometry/materials; do not add a scene object per new texture layer. Exclude the old underlay wherever replacement ground renders.

## Sequence

Develop outside-in: T1 fixes consumer intent and tests; T2 supplies resources; T3 publishes wire contract for generated bindings; T4/T5 build toolkit providers; T6 maps the host; T7 finishes shared Web rendering/controls; T8 integrates and walks. T3 may be authored while T2 is verified. No consumer commits hand-generated SDKs to bypass the proto release checkpoint.

Merge after proof: Assets provider, protos publication, encounter tag, session tag, API pins, Web exact catalog/provider pin. Operator authorization is required for each merge. Asset sync uses existing exact-inventory tooling and never overwrites the running review queue.

## Review focus

1. Explicit paint equal to the old base must survive a base change (T1).
2. A discovery patch must neither disclose future painted cells nor discard previously known paint (T4–T7).
3. Save/reload must use captured ground, not edited source files (T4/T6).
4. Removing/reassigning floor must not leave orphan paint or move paint into another region (T1/T4).
5. Unavailable explicit styles must not silently show stone, and overlays must stay legible (T2/T7/T8).

## T1 — Authoring state, persistence and edit invariants

**Owner:** Web. **Prerequisites:** existing region/draft/native-site helpers.
**Files:** new `src/concepts/world-building/regionGround.ts`, `regionGround.test.ts`; modify `roomDraft.ts`, extend `roomDraft.test.ts`, `singleRoomDungeon.test.ts`; use real document history in `src/compositions/roomDocument.test.ts` where appropriate.
**Interfaces:** authoring contract above; only type imports from roomDraft in the new helper to avoid a runtime cycle.

- [ ] Write `regionGround.test.ts` assertions before implementation: base affects only target region; paint/erase touch only declared cells; same-as-base paint survives; base reset does not clear paint; invalid/off-region edits leave source unchanged; identical edits return original reference.
- [ ] Add real native-site JSON/YAML round-trip assertions, including source unknown-field carry, old absence and malformed/duplicate/off-region paint refusal. Add floor erase/reassignment cases with literal expected rows.
- [ ] Run focused tests and observe the missing-contract failure.
- [ ] Implement helpers, typed validation and `withOwners` pruning; no UI stubs or provider defaults.
- [ ] Run `npm run test:run -- src/concepts/world-building/regionGround.test.ts src/concepts/world-building/roomDraft.test.ts src/concepts/world-building/singleRoomDungeon.test.ts src/concepts/world-building/regionEdits.test.ts src/compositions/roomDocument.test.ts` and `npm run typecheck`.
- [ ] Commit the tested checkpoint; this is not the complete slice or a merge-ready Web PR.

## T2 — Ground appearance provider and strict client reader

**Owner:** Assets + Web catalog reader. **Prerequisites:** reviewed source pack path; ordinary provider inventories and sync are present.
**Files:** new Assets `scripts/build_ground_catalog.py`, `scripts/test_build_ground_catalog.py`, `scripts/configs/ground/courtyard-v1.json`; generated private `harness/models/synty/ground/`; update existing complete inventory. New Web `src/rendering/groundCatalog.ts`, `groundCatalog.test.ts`, `src/components/session/useGroundCatalog.ts` and tests.
**Interfaces:** catalog contract above. Generator stages into a fresh private output root; check rebuild compares exact generated bytes without altering installed assets. Recipe pins source hashes, including current stone image; publication remains existing provider workflow, not an auto-merge command.

- [ ] Test deterministic generation/resource verification on small real PNG fixtures, source hash drift, invalid/transparent images, malformed dimensions/scale, duplicate refs, escaping/symlink destinations and unexpected output files; tests run the real builder.
- [ ] Observe RED, implement builder/validator, run `python3 -m unittest discover -s scripts -p 'test_build_ground_catalog.py'`.
- [ ] Prepare the three candidate images at native resolution and verify the staged resources. Preserve originals and capture exact recipe/catalog hashes.
- [ ] Test client reader accepts the exact staged schema and refuses invalid refs/paths/hashes/scale/duplicates; hook tests HTTP and missing-catalog failure without a silent default for explicit style use.
- [ ] Run Web ground-catalog tests and provider inventory checks. Do not publish a source or image into Web git.
- [ ] Record first-candidate appearance and decoded-memory facts for T8; final acceptance belongs to the early view.

## T3 — Typed region-ground transport

**Owner:** protos only. **Files:** session v1alpha1 `types.proto`; session v1alpha2 `act_geometry.proto`; owning schema documentation and run record.
**Interfaces:** exact three additions above; existing field numbers are unchanged.

- [ ] Add documented messages and `AtlasRegion.ground = 6` to both packages. State partial-cell paint semantics, absent-base behavior and appearance-only ownership.
- [ ] Run `make format` and `make test` (Buf lint/breaking/new-file options). No serialization tests of generated protobuf mechanics.
- [ ] Commit/push a PR linked to #874. Generated bindings remain a merge/publication dependency, not hand-authored local replacements.

## T4 — Encounter source, saved data and disclosure

**Owner:** toolkit encounter module only. **Prerequisites:** T1 shape (no dependency on Web code), existing region/atlas projection.
**Files:** new `region_ground.go`, `region_ground_test.go`, `dungeonspec/single_room_ground.go`, `dungeonspec/single_room_ground_test.go`; modify `field.go`, `compilefield.go`, `data.go`, `atlas.go`, `region.go`, `projection.go`, region discovery producers, `dungeonspec/single_room.go`, `single_room_decode.go`, `single_room_regions.go`; extend existing atlas/discovery/reload tests.
**Interfaces:** encounter contract above; helper `groundForCells` deep-copies/filter paint against the exact supplied cell set and is used by every partial-region producer.

- [ ] Add native source fixtures with grass base, explicit same-as-base patch and two dirt cells in separate disclosure areas. Reject null/empty refs, duplicate/conflicting paints, malformed runs and paint outside owner.
- [ ] Add construction/reload tests: caller mutation cannot change captured appearance; `ToData` → `LoadFromData` preserves it; old saves remain absent; invalid saved ownership is refused.
- [ ] Add tests distinguishing full Atlas, AtlasFor, ordinary CellsLearned and concealment reveal; an unrevealed dirt cell must not be in any delivered paint group. Test wall-footing removal and no gameplay changes using the same route/cell facts before/after appearance.
- [ ] Observe RED, implement shape/lowering/deep-copy/filter/save/load. Never look up a style resource in the engine.
- [ ] From the encounter module run `go test ./dungeonspec ./...` with the owning module's bounded gate; run nearest module lint as required. Add one small source fixture and a castle route assertion without changing existing gameplay.
- [ ] Commit/push and open the module draft; publish a resolvable pseudo-version for T5. Do not merge to unblock development.

## T5 — Session SDK carries ground on snapshots and acts

**Owner:** toolkit session module, separate worktree/PR. **Prerequisite:** T4 pushed provider commit, pinned by `go get`.
**Files:** `types.go`, `convert.go`, `act_family.go`, their conversion/fold tests; module pins and run record.
**Interfaces:** session `RegionGround`/`GroundPaint` match encounter semantics; JSON nested names align with generated protobuf JSON (`baseRef`, `paint`, `ref`, `cells`).

- [ ] Add assertions on public Atlas and partial discovery act bodies that retain explicit paint and cannot add cells omitted by encounter.
- [ ] Observe RED, add typed projection and act-body conversion. No independent visibility filtering.
- [ ] Run focused tests plus module gate; commit/push draft on real encounter pseudo-version. Tag pin required before final readiness.

## T6 — API snapshot/act mapping and real save/reload proof

**Owner:** API. **Prerequisites:** generated T3 bindings; T5 pushed SDK commit.
**Files:** `internal/handlers/dnd5e/session/v1alpha1/convert.go`, `convert_test.go`; v1alpha2 `convert_geometry.go`, `convert_act_test.go`; existing authoring/session integration tests; module pins; private untracked `content/courtyard-ground-walk.yaml`.
**Interfaces:** field-for-field mappings in both wire versions. No RPC/permission additions.

- [x] Add distinguishing conversion assertions for nonempty base, explicit same-as-base paint and partial delivered paint; null ground remains absent.
- [x] Add a real native-document create/start/reload integration path; changing stored source after starting must not repaint the encounter snapshot.
- [x] Observe RED, implement pure mapping and real dependency pins.
- [x] Run focused handler/integration checks and `make ci-check` plus `make test` before PR readiness. No local Go replacement or source-copy build.

## T7 — Studio controls, shared ground drawing and discovery merge

**Owner:** Web. **Prerequisites:** T1/T2; T3 bindings for live-play types, T6 stack for joined checks.
**Files:** new `StudioGroundControls.tsx`/tests in `src/concepts/encounter-studio`; extend `studioSession.ts`, `EncounterStudioWorkspace.tsx`, `WorldBuildingConcept.tsx`, `studioArrange.ts`, region inspector controls and integration tests. Add shared ground rendering helpers/component under `src/components/hex-grid/`; extend `WorkspaceFloorUnderlay.tsx`, `SyntyHexFloor.tsx`, `atlasToScene3D.ts`, `applyReveal.ts`, `actGeometry.ts` and their tests.
**Interfaces:** commands call T1 edits through the current document/selection generation checks and one history transaction per committed stroke. The base picker is region-owned; paint/erase-ground tools are distinct from floor-membership paint/erase. Shared resolution returns the override or base/absence contract; only permitted floor cells enter play drawing.

- [ ] UI tests: region selection/base picker, active-region-limited brush, erase-to-base, unavailable style notice, stale session refusals, one history record per stroke, undo/redo and native save/reopen. Round-trip current served castle source preserving unrelated fields.
- [ ] Renderer tests: matching world XZ coordinates produce identical UV phase across adjacent hexes; all styles share ground Y; default output unchanged when no ground authored; no extra underlay under replacement; fog/watch-zone/memory overlays remain represented; repeated styles reuse material resources and owned resources dispose safely.
- [ ] Discovery tests: two disjoint partial patches retain both cells' paint; repeats are idempotent; base metadata is carried; no undisclosed source data is requested. Extend act adapter field-copy tests for nested ground.
- [ ] Observe RED, implement controls and shared rendering; no source-only Studio feature claimed as playable.
- [ ] Run named focused files and typecheck; run `npm run ci-check` once at PR boundary, not after every edit.

## T8 — Integrated early view, walk, measurement and delivery

**Owner:** cross-repo integration under Assets Team. **Prerequisites:** working provider and consumer branches; no local replacement graph.
**Files/artifacts:** untracked walk site cut from castle courtyard; private screenshots and fixed camera/arrival driver; API castle content changes only after verified fold-back; issue/PR evidence and run records.

- [ ] Boot builder-owned stack using root `scripts/dev-env.sh`; verify exact served API/Web/content/catalog hashes and normal asset sync. Do not alter :3030 or the Lab.
- [x] Publish early-view URL and head on #874. Obtain the operator's look before the formal walk/review round.
- [ ] Walk every #874 checklist item, including player and NPC motion, base-equal paint, remove/reassign floor behavior, unavailable styles and fog/reload. Verify ordinary gate/route behavior after fold-back.
- [ ] Measure same scene/camera/device/build configuration before and after: load-to-first-frame, frame-time distribution, draw calls, resource/memory observations. Keep asset loading distinct from engine latency and do not certify hardware from software-rendered captures.
- [ ] Independent review per repository policy, then fix findings with tests. Feature remains draft while integration or released pins are pending.
- [ ] After authorized merges, adopt actual published tags/catalog pin, repeat joined verification, obtain final castle walk and close #874 only when all finish-line evidence exists.

## Current execution checkpoint

| Task | Evidence / state |
|---|---|
| T1 | Implemented in Web `ffd31384`; edit, ownership, source/composition/local round trips and current full Web gate pass. |
| T2 | Assets `cb2bbb5`, draft assets#342: identical approved-color resources installed in the private provider candidate; 9 ground checks, source rebuild and complete 4501-file inventory/stage closure pass. Independent custody finding fixed and closure clear. Full discovery: 2177 tests, unchanged 21 failures/15 errors/119 skips from assets#333; no waiver. Publication/exact consumer pin remain pending. |
| T3 | Protos#421 is operator-merged and generated: TS v0.1.258 and Go v0.0.0-20261106231558-dea142c77291 are adopted. |
| T4 | Encounter `45055ff6`, draft toolkit#2289: all module packages and lint pass locally and CI passes. Includes native fixture, save/reload/copy, source validation, footing and partial/concealment disclosure tests. |
| T5 | Session `74bc4ea8`, draft toolkit#2290: full module/lint/CI pass; real encounter pseudo-version remains until release. |
| T6 | API `2355058`, draft api#1190: both wire mappings and native create/start/fresh-host captured reload pass; full local gates pass. Runs only in the isolated :8124 environment. |
| T7 | Web `78b4901d`, draft web#1395: Studio and live rendering/discovery union implemented; full gate passes 9972 tests with five skips. Layout UVs match 3D; unused source GPU uploads are removed. |
| T8 | Early Studio appearance approved. Joined native save/play, two-client gate discovery, actor/NPC motion, reload and source immutability pass. Three-pair 431-cell RTX 4090 benchmark records 369→71 calls, unchanged 16.8-ms p95 and worse startup p99 (40.7→65.1 ms). Independent code reviews are clear at all listed heads, including focused Assets custody closure. Full castle/large-keep acceptance, the unwaived Assets baseline gate, publication, final pins and authorized merges remain open. |

The current paper trail and preview instructions are on #874. The preview is
http://127.0.0.1:3181/; the operator castle (:3030) and review Lab (:3180) are
unchanged. Only the operator-merged proto contract has merged. The playable link,
measurements and remaining gates are on [the joined checkpoint](https://github.com/KirkDiggler/rpg-project/issues/874#issuecomment-6701542844).
[Review closure and the current provider gate](https://github.com/KirkDiggler/rpg-project/issues/874#issuecomment-6712221345)
name the remaining stop. Discussion remains in chat before recording decisions.

Plan refinements supported by inspection: the new `scripts/groundPreviewPlugin.ts`
serves only hash-verified private staging under an explicit environment opt-in,
marks the UI as candidate, and serves development or explicit `bench` preview
only; ordinary production preview refuses private staging. Existing authoring fill annotations
are omitted on textured ground while outlines/grid remain; native screenshots
showed that otherwise those annotations masked the intended surface colors.
Neither refinement changes mechanics or source ownership. Cross-repository
progress is recorded here and in the linked issue/PRs rather than assigning
all code commits to the documentation worktree's git history.

## Requirement coverage

| Requirement | Tasks | Concrete proof |
|---|---|---|
| Region base, explicit paint, erase and preserved equality | T1/T7 | Literal edit tests + real Studio history/native round trip |
| Existing default; missing explicit style errors | T1/T2/T7 | Old absence fixture + resource failure/unknown ref UI |
| Level faceted grass/dirt with continuous scale | T2/T7/T8 | Real source image candidates + world-UV/Y tests + operator view |
| Snapshot/reload and partial disclosure | T3–T7 | Real provider round trip, partial projection, both wire mappings, union reducer |
| Gameplay/region membership unchanged | T1/T4/T8 | Owner-map assertions + before/after route facts + castle walk |
| Measured performance and deployment evidence | T8 | Fixed route table and exact build/content/catalog hashes |

## Provider/consumer seam check

| Provider | Consumer | Contract and availability | Joined proof |
|---|---|---|---|
| T1 native region ground | T4 dungeonspec | Same baseRef/paint/ref/canonical rows; new typed source field required | Shared literal YAML fixture + real parse/compile |
| T2 ground catalog | T7 renderer | Hash-bound color images and worldUnitsPerRepeat; no GLB volume requirement | Exact staged catalog browser load + unknown-ref refusal |
| T4 RegionGround | T5 SDK | Opaque refs, converted absolute cells, partial filtering; pushed commit pin | SDK snapshot/discovery tests against real provider |
| T5 SDK + T3 protos | T6 API | Both AtlasRegion versions carry optional ground at field 6 | Handler assertions + persisted integration |
| T6 snapshot/acts | T7 play | Same partial-region paint semantics; generated SDK required | Two-discovery/reload browser acceptance |

## Check findings and dispositions

- Generic prop Ready rejects zero-height surfaces. Do not relax it: T2 supplies color surface resources to the existing flat floor model.
- Kingdom ground color images already carry facets. Direct source-color use avoids inventing FBX shader equivalence; candidate visual acceptance remains T8.
- Partial `unionRegions` currently spreads the newest region object. T7 must merge ground paint, not overwrite previous discovered cells.
- Region owner edits currently preserve extra fields unchanged. T1 must explicitly prune departing paint rather than leave invalid assignments.
- Session snapshot and act geometry use different proto packages. T3/T6 must carry both; updating only GetAtlas would regress later discoveries.
- Full-source scene presentation is retired for play. All new ground must follow typed region projection.
- Proto publication is an explicit external gate. It does not block T1/T2/T4 provider work, but it blocks claiming joined T6/T7 completion.

The plan is checked against the named seams and acceptance scope. Exact art repeat scale is a visible candidate choice, not an approved numeric promise. No required acceptance scenario is deferred; operator visual approval, review, release pins and merges remain pending until their evidence exists.
