# Manifest-driven Studio Appearance Implementation Plan

> Execute directly, task by task, using `executing-plans` and the workspace `build-slice` contract. Delegation is not authorized by this plan. Requirement and seam checks below precede implementation; preserve this same brief through integration.

**Goal:** Common manifest-driven appearance controls for walls, NPCs and props, including existing normal-bearing wall-face materials in captured play.

**Architecture:** Assets supplies verified choices; typed Web adapters produce one editor sheet and translate edits into existing owner transactions. Existing native wall-face selections become typed immutable face choices beside the wall's current captured geometry. The renderer retains separate complete-look, authored-UV and wall-local masonry mechanisms.

**Tech stack:** Python/JSON Assets tooling, React/TypeScript/Three.js, native YAML, Go encounter/session, protobuf and Go API, Redis.

**Spec:** [design.md](design.md), [walkthrough](README.md), [brief](brief.md); [R9 approval](https://github.com/KirkDiggler/rpg-project/pull/915#issuecomment-6758627344). Delivery slice: [Courtyard S5 #919](https://github.com/KirkDiggler/rpg-project/issues/919); implementation detail: [Web #1402](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1402); story: Project #867.

## Goal, finish line and authority

The one user-visible deliverable is Courtyard S5#919, with Web#1402 retaining its implementation discussion; tasks below follow code/test/release-unit boundaries, not a new issue for every branch. Done requires shared controls over actual provider metadata; independent appearance choices; explicit missing-resource state; consistent appearance Apply/Cancel/Undo/Redo; native save/reopen; actual captured play/reload with color and normal maps; focused visual evidence; owning gates and independent review. PRs remain drafts while integration/generated or release pins are outstanding. Merge and operator-stack deployment require separate authority.

The scoped design is approved. Ordinary contract elaboration, implementation, tests, fixes, commits and PRs proceed without another blanket approval. No tree YAML prerequisite. No universal form engine, asset registry, new ingestion pipeline, automatic room inference, new gameplay lighting, generic independent channel editing or door-section implementation.

## Global constraints

- Valid choices derive from Assets metadata and exact target/model bindings; artistic recommendations do not define compatibility.
- Keep existing complete NPC `appearanceRef`, atlas `MaterialOverride`, and native `scene.wallFinishes` representations. Do not migrate documents to one arbitrary property bag.
- A face choice changes no geometry, region membership, blockers, openings, door state, NPC mechanics or recipient policy.
- Shared scene resources remain immutable; material/sampler changes are per-instance and applied before memory/lighting.
- Normal-bearing masonry packs retain both maps. A color-only atlas and a complete NPC look do not acquire invented channel overrides.
- Work in `.worktrees/studio-1402`; toolkit session has `.worktrees/studio-1402-session`. One nearest-`go.mod` per toolkit PR; no replacements, hand tags, generated SDK edits or forced history updates.
- No operator3030/8115 or Lab3180 changes. Any later isolated walk checks process/container ownership first.

## Inspected baselines and narrowed contract

| Repository | Baseline | Owning area                                     |
| ---------- | -------- | ----------------------------------------------- |
| Project    | b2a2dda  | design branch `design/1402-studio-appearance`   |
| Web        | 3be3c439 | editor/renderers and generated metadata         |
| Assets     | 6345db54 | current material/NPC authorities                |
| Toolkit    | ba6d40f2 | encounter and separate session module worktrees |
| API        | b4af0f1d | dev; currently pins encounter187/session174     |
| Protos     | 45a0fb36 | main; actual generated outputs are CI-owned     |

Planning observation: `projectRegionWallBoundaries` always returns `not-wall-bound` for current listed-cell regions. Working explicit wall finishes use the whole wall and a local chart: phase0, positive-Z direction+1, negative-Z direction−1, wall elevation, provider repeat scale. Capture **face choices**, not a speculative regional chart/spans language. Existing unapplied region metadata remains round-trippable/unapplied.

Upstream toolkit now shares sites by hash. Its `SiteRepository` is an existing published host capability, but the inspected API repository does not implement it. Adopting this toolkit baseline therefore includes the minimal host storage adapter and a genuinely cold site-cache test in Task6; do not let a global in-process cache masquerade as fresh-host persistence.

## Shared interfaces

### Provider metadata

Material catalog schema3 extends schema2's existing atlas surfaces with required `compatibleFamilies: string[]`: nonempty, unique technical family identities. Source recipe/binding verification emits the enrolled atlas family's identity; every permitted family must resolve to authored-UV material rows. Suggested collections stay separate. Masonry already declares `family` per exact surface and per material; NPCs already declare exact complete-look tuples. Reuse those facts, rather than duplicate them in a new registry.

The new consumer explicitly accepts schema3 for atlas metadata; schema1 masonry-only support remains where existing readers require it. Schema2 atlas input is refused by the new strict reader rather than silently manufacturing compatibility. Old consumers remain on their old immutable pins. New provider adoption and reader generation ship in the same Web PR. Images, GLBs, NPC look refs, existing material refs and default bindings do not change.

### Captured masonry

- Encounter `WallFaceSide` is a string type with constants `WallFacePositiveZ = "positive-z"`, `WallFaceNegativeZ = "negative-z"`.
- `WallFaceFinish { Side WallFaceSide; MaterialRef string }`; matching data DTO with `json:"side"` / `json:"material_ref"`.
- Optional `FaceFinishes []WallFaceFinish` on `StructuralWallInput`, private compiled wall, `AtlasStructuralWall`; persistence stores `face_finishes` in `StructuralWallData` within `SiteData.Field`. Empty/absent means no explicit per-face finish. No region-derived default is guessed.
- Maximum two entries; sides must be known/unique and material refs nonblank. Material refs remain opaque to toolkit. Preserve every copy boundary and include these fields in the existing site hash naturally through canonical FieldData.
- Native source remains `room.scene.wallFinishes[rawWallId][positive-z|negative-z]`. Decode known face keys, preserve unknown editor keys in the YAML node, refuse malformed known values and orphan wall IDs. Lower after validating the source; canonical wall IDs remain unchanged.
- Session owns the analogous DTO and validates captured `face_finishes` on full wall introductions. Opening-only replacements never modify face choices.
- Both session proto versions add `message WallFaceFinish { string side = 1; string material_ref = 2; }` and `repeated WallFaceFinish face_finishes = 11` to `AtlasStructuralWall`. Strings use the two exact side tokens; empty/unknown sides are malformed, not a default. No field on props or NPCs; no change to field10 atlas overrides.
- Web maps the two tokens into `WallFinishOverrides` beside the converted wall, then uses the shared wall-local projection. Existing permitted wall geometry supplies length, elevation and direction; no client region lookup or source fetch.

### Editor boundary

New types under `src/concepts/encounter-studio/appearance/`:

- `AppearanceSheet { selectionKey: string; fields: readonly AppearanceField[]; diagnostics: readonly string[] }` is derived, never serialized.
- `AppearanceField` is a discriminated union: `npc-family`, `npc-style`, `npc-skin`, `asset`, `atlas-pack` (surfaceId), `wall-finish` (side or both), and read-only `channel` (color/normal/emissive state). Choice fields carry `value`, `label`, `choices: readonly {value,label}[]`, optional unavailable reason. Channel state is `pack-map` with resource identity, `no-map`, `complete-look` (not independently declared/editable), or `unavailable` with reason.
- `AppearanceEdit` mirrors the selectable field kinds with typed identity/value fields. It does not carry arbitrary document paths or callbacks.
- `buildAppearanceSheet(input)` consumes the existing selection/draft, world asset/material catalogs and NPC family catalog. It derives valid choices and retained unavailable selections.
- `appearanceEditPatch(input: {selection: StudioArrangeSelection; draft: ArrangeDraft; sheet: AppearanceSheet; edit: AppearanceEdit}): ArrangeDraft` returns the appropriate patch or throws before mutation. The owner still commits through `arrangeIntent` / `session.commitArrange`.
- Appearance picks stage. NPC style/skin changes resolve against the **staged** `appearanceRef`, not the originally selected ref. Unrelated immediate pickers keep their policy. Selection/revision changes discard stale staged edits through the existing selected scope; validation failures retain draft values.

## Review focus

1. Existing imported unknown refs/scopes remain visible and round-trippable; Apply cannot silently erase them. T1/T7/T8 pin this.
2. Changing asset/family with an already staged pack must not apply the old target to the new model; T1/T7 require compatibility checks and retained diagnostics.
3. Opposite faces, rotated walls and openings must keep the same texture phase and normal basis; T8 pins both directions and clipped/repeated pieces.
4. Partial discovery, hidden walls and opening-only patches must not leak or erase face choices; T3/T4/T6/T8 cover the actual joins.
5. A fresh host with an empty site provider must load the captured material, not current edited source or a warm global cache; T3/T6 exercise explicit isolated providers and durable site storage.

## Sequence and release ordering

Develop outside-in: T1 consumer contract/tests → T2 provider metadata and T3/T5 capture contracts → T4/T6 adapters → T7/T8 real UI/render integration → T9 acceptance. Tasks sharing Web files execute serially. No child writers are assumed.

Proto generated SDK publication is the one merge prerequisite to compiling its consumers; request the required merge only when the contract has its gates/review. Independent provider, engine and editor work continues meanwhile. Other Go consumers use real pushed pseudo-versions before release. Final merges, when authorized: Assets and Protos; encounter → actual tag → session → actual tag → API; Web adopts actual merged asset SHA and generated TS release. Re-run owning gates at pin/update boundaries and verify the final joined path.

### Task 1: Typed shared appearance model and provider-reader contract

**Delivers:** R1–R6; domain-neutral field data with exact provider-backed choices.
**Owner:** Web. **Prerequisites:** inspected existing catalogs; fixture schema3 contracts above.
**Files:** create `src/concepts/encounter-studio/appearance/{appearanceTypes.ts,appearanceSheet.ts,appearanceSheet.test.ts}` and `appearanceEdits.ts`/`.test.ts`; modify `src/rendering/{worldMaterialTypes.ts,worldAtlasCatalogSchema.mjs,worldAtlasCatalogSchema.d.mts,worldAtlasCatalog.ts,worldAtlasMaterials.ts}` and `scripts/{world-material-catalog.mjs,generate-world-material-catalog.mjs,stage-world-material-preview.mjs}`; update `scripts/worldAtlasCatalog.test.ts`, `worldAtlasCatalog.testFixtures.ts`, existing atlas/runtime tests.
**Interfaces:** provider/schema and editor contracts above. Bind compatibility validation in the shared reader/runtime, not only the select component.
**Behavior:** filter atlas choices by declared technical families; masonry by exact surface family; NPC by actual complete-look tuples. Recommendations do not restrict valid choices. Report selected unknowns without fallback. Current packs expose their actual channel facts; NPC complete looks report bundled channels, not invented absence or editable maps.
**Tests:** `filters atlas materials by binding family, not recommendation`; `rejects missing empty duplicate or unknown compatibility families`; `refuses schema2 atlas metadata explicitly`; `includes un-recommended compatible material`; `NPC options exclude missing style/skin tuple`; `paired masonry normal is supplied, atlas normal/emissive are absent`; `unknown selection is retained and blocked`; `edit validates selected owner and staged target`.

- [ ] Add named fixtures/tests and observe pre-change failures.
- [ ] Implement strict readers plus pure sheet/edit projection; update existing reader fixtures explicitly.
- [ ] Run focused schema/model tests; document consumer/provider version semantics.
      **Verification:** from Web worktree `npm run test:run -- scripts/worldAtlasCatalog.test.ts src/rendering/worldAtlasCatalog.test.ts src/rendering/worldAtlasMaterials.test.ts src/concepts/encounter-studio/appearance/appearanceSheet.test.ts src/concepts/encounter-studio/appearance/appearanceEdits.test.ts`; `npm run typecheck` after prepared provider generation supplies schema3 bindings.
      **Evidence:** RED/GREEN reader/model receipts, exact fixture capabilities, no UI or runtime-play claim yet.

### Task 2: Publish explicit atlas compatibility through the existing Assets pipeline

**Delivers:** R1/R3/R4; valid lists originate in provider metadata.
**Owner:** private Assets. **Prerequisites:** T1 schema3 contract/tests; existing registered source root and reviewed materials.
**Files:** modify `scripts/{world_material_atlas_candidate.py,prepare_prop_atlas_candidate.py,build_world_material_catalog_v2.py,publish_world_materials.py}` and their existing `test_*.py` files; generated `harness/catalogs/synty-world-materials.json` and affected publication/inventory metadata; owning material publication guide. Source recipe format may remain version2; published catalog is schema3.
**Interfaces:** each atlas surface emits `compatibleFamilies` from verified recipe/target identity, independent of `suggestedCollections`. Existing masonry/NPC metadata remains its authority and is not copied into a second UI manifest.
**Behavior:** refuse unverifiable target/family declarations, wrong GLB/image identity and inconsistent recommendations. Preserve every licensed runtime image and GLB byte. No new palette images or re-ingestion. Use source-complete stage → validate → exact-seal apply → installed check; metadata-only changes still have custody checks.
**Tests:** `schema3 declares compatibility on every bound surface`; `unknown family fails`; `recommendation omission does not remove compatible rows`; `catalog bytes deterministic`; `all prior image/model digests unchanged`; preserve publication refusal and path-safety tests.

- [ ] Add scoped provider failures for schema/family declarations.
- [ ] Implement metadata emission/verification through existing builders and publisher.
- [ ] Run source/resource/binding checks; prepare normal candidate and consumer projection.
- [ ] Publish only after the existing acceptance/seal conditions hold; record metadata-only custody evidence.
      **Verification:** `python3 -m unittest discover -s scripts -p 'test_world_material_atlas_candidate.py'`; corresponding `test_prepare_prop_atlas_candidate.py`, `test_publish_world_materials.py` and `test_publish_world_materials_v2.py`; use existing publisher CLI `--help` for stage/validate/apply/check arguments; `python3 scripts/build_synty_complete_inventory.py --check`. Broad unrelated baseline failures remain separately reported, not relabeled green.
      **Evidence:** exact prepared provider commit/digest/seal, unchanged image/model hash comparison, normal Web sync/check with that provider. No fabricated artistic verdict.

### Task 3: Native wall-face capture, site persistence and permitted projection

**Delivers:** R7–R9, immutable face choices without resource/region interpretation.
**Owner:** toolkit encounter nearest module. **Prerequisites:** captured interface above; no generated SDK dependency.
**Files:** create `wall_face_finish.go`, `wall_face_finish_test.go`, `wall_face_finish_internal_test.go`; modify `structural_walls.go`, `atlas.go`, `projection.go`, `data.go`, `structural_reveal.go`, `doc.go`; create `dungeonspec/single_room_wall_finishes.go` and `wall_finishes_test.go`; modify `dungeonspec/single_room_lowering.go`, `single_room_compile.go`, `single_room_walls.go`. Follow actual backing-wall type/copy sites found by `rg MaterialOverrides`.
**Interfaces:** `WallFaceFinish`, validation/clone/data helpers; `roomRead.WallFinishes` keyed by raw native wall ID; `canonicalStructuralWalls` consumes that typed read. Empty stays absent. Data goes in SiteData.Field, not EncounterData's removed field.
**Behavior:** lower known native face keys; orphan wall, malformed known values, duplicate runtime side and unknown runtime side refuse. Unknown editor-only keys remain in YAML. No derived material or regional default. Detached input, site data, atlas and reveal rows. A face choice changes the site's content hash; changing later source changes neither the stored site nor the old run.
**Tests:** `TestNativeWallFinishesPreserveNonAppearanceFacts`; `TestMalformedNativeFaceValuesRefuseWithPath`; `TestWallFaceChoicesSurviveColdSiteReload`; `TestWallFaceChoicesAreDetachedAtEveryBoundary`; `TestInvalidFaceChoicesRefuseConstructionAndSiteLoad`; `TestHiddenWallCarriesNoFinishBeforeReveal`; `TestOpeningReplacementRetainsWallAppearance`; `TestRegionFinishDoesNotInventWallOwnership`. Use the small structural fixture, not another full-castle oracle.

- [ ] Add the named RED capture/validation tests.
- [ ] Implement typed lowering and every copy/data/reveal boundary.
- [ ] Run focused tests and validation/transcription mutation probes; update godoc.
      **Verification:** encounter cwd: `go test ./dungeonspec -run 'TestSingleRoomWallSuite/(TestNativeWallFinishes|TestMalformedNativeFace|TestRegionFinish)' -count=1`; `go test . -run 'TestStructuralWallSuite/(TestWallFace|TestInvalidFace|TestHiddenWall)' -count=1`; full owning `go test -race -covermode=atomic -coverprofile=coverage.out ./...` and `golangci-lint run` at PR boundary under upstream budgets/shards, no local timeout bypass.
      **Evidence:** named assertions, new site-hash/source immutability proof and gates. Push real provider commit before T4 pins it.

### Task 4: Session DTO and reveal transcription

**Delivers:** R8/R9, same face facts in snapshot and reveal.
**Owner:** toolkit session, separate worktree/PR. **Prerequisites:** T3 pushed commit and real Go pseudo-version.
**Files:** create `wall_face_finish.go` and `wall_face_finish_internal_test.go`; modify `types.go`, `structural.go`, `atlasmap_test.go`, `convert_internal_test.go`, `doc.go`, `go.mod`/`go.sum`.
**Interfaces:** session-owned `WallFaceFinish {Side string, MaterialRef string}`; `AtlasStructuralWall.FaceFinishes` with `face_finishes` JSON. Project from encounter, do not export encounter types or resolve resource refs.
**Behavior:** copy detached values field-for-field; reject an invalid side, blank material or duplicate side in a complete reveal atomically. Missing legacy collection remains absent. Existing opening-only replacement is unchanged.
**Tests:** `TestWallFaceFinishesMatchSnapshotAndReveal`; `TestWallFaceFinishesDoNotAlias`; `TestMalformedWallFaceFinishesRefuseWholeReveal`; `TestLegacyWallRevealHasNoInventedFinishes`; reflection transcription tests include the field.

- [ ] Pin T3 pushed commit using `go get` and add RED tests.
- [ ] Implement DTO/projection/decode and update exact-field probes.
- [ ] Run focused tests, full owning gate/lint and publish draft.
      **Verification:** session cwd `go test . -run 'TestPropPresentationDecodeSuite/Test.*WallFace' -count=1`; `go test -race -covermode=atomic -coverprofile=coverage.out ./...`; `golangci-lint run`.
      **Evidence:** true pushed dependency, snapshot/reveal equality and invalid-payload refusal. Replace pseudo-pin with actual encounter tag only at authorized merge-out.

### Task 5: Additive wire contract and CI-generated SDKs

**Delivers:** R9 wire carrier, no resource bytes or source-scene fields.
**Owner:** Protos. **Prerequisites:** exact face-choice contract in this plan; independent of completed Go implementation.
**Files:** `dnd5e/api/session/v1alpha1/types.proto`, `v1alpha2/act_geometry.proto`, `docs/architecture/components/session-service.md`, run record.
**Interfaces:** both `WallFaceFinish` messages and `AtlasStructuralWall.face_finishes = 11` as defined above. Existing v1alpha1/v1alpha2 footprint/opening field-number differences stay untouched. No new RPC.
**Behavior:** absent list means no selected face material; known side tokens only, unique side, nonblank opaque ref; inherits the wall's audience and capture; opening-only replacements leave it alone.

- [ ] Add contract comments and additive fields.
- [ ] `make format`, `make test`; publish the contract PR with generated CI checks.
- [ ] Obtain required review/merge authority, then observe real generated Go commit/TS tag. Do not hand-generate bindings or tests of protobuf mechanics.
      **Verification:** Protos cwd `make format && make test`; CI generation/SDK validation. T6 and wire-facing T8 wait for actual generated outputs, while other tasks continue.
      **Evidence:** contract gate and generated release identities, separately recorded from source merge SHA.

### Task 6: API mapping and genuinely cold persistence

**Delivers:** R8/R9 through the native API and reload, including upstream site-store adoption needed by this module pin.
**Owner:** API. **Prerequisites:** T3/T4 pushed dependencies and T5 published SDKs.
**Files:** handler `internal/handlers/dnd5e/session/v1alpha1/convert_structural.go`, `v1alpha2/convert_geometry.go`; create `wall_face_finish_test.go` in both packages; `internal/orchestrators/session/redis_repos.go` and new `redis_sites.go`/`redis_sites_test.go`; `internal/integration/session/wall_finish_acceptance_test.go` plus `testdata/wall-finishes.yaml`; existing launch/helper/orchestrator tests and dependency files as required by current upstream SiteData API.
**Interfaces:** copy session Side/MaterialRef to each generated message; implement existing SDK `SiteRepository.GetSite(*GetSiteInput)` / `PutSite(*PutSiteInput)` on the encounter repository. Immutable site data is stored by hash separately from checkpoints, persistent without an encounter TTL, idempotent on repeated Put. No new resource service.
**Behavior:** resolve a fresh process through stored SiteData, not edited source or warm `DefaultSites`. Preserve fencing/seat behavior. Unknown site fails closed; no source fallback. API maps fields and stores opaque data, never calculates charts or inspects material refs.
**Tests:** `TestWallFaceFinishConversionsAgree`; `TestSitePutIsImmutableAndIdempotent`; `TestMissingSiteReturnsNotFound`; `TestNativeWallFinishSurvivesColdHostAfterSourceEdit`; `TestWallFinishSnapshotAndRevealAgree`; malformed stored data fails without partial view. Cold-host test injects a fresh empty `encounter.NewMemorySites` provider.

- [ ] Adopt actual SDK generation and pushed toolkit pins.
- [ ] Add RED mapper and cold-host tests, including site-store capability.
- [ ] Implement mapping/storage adapter; adapt old direct Field fixtures to current SiteData without weakening assertions.
- [ ] Run focused integration, full tests and `make ci-check` before push.
      **Verification:** `go test ./internal/handlers/dnd5e/session/v1alpha1 ./internal/handlers/dnd5e/session/v1alpha2 ./internal/orchestrators/session ./internal/integration/session -count=1`; `go test ./...`; `make ci-check`.
      **Evidence:** real native create/save/play/cold-reload test under source mutation; exact dependency receipts. No running operator Redis changes.

### Task 7: Shared Appearance panel in the real Studio transaction

**Delivers:** R1–R7, recognizable controls with consistent staged appearance edits.
**Owner:** Web. **Prerequisites:** T1 pure contracts, T2 prepared normal provider projection. T8 supplies masonry play separately.
**Files:** create `appearance/StudioAppearancePanel.tsx` and `.test.tsx`, `appearance/AppearanceChoiceField.tsx`; modify `StudioArrangePanel.tsx`, `StudioArrangeFields.tsx`, their tests and `EncounterStudioIntegration.test.tsx`; retire `StudioTexturePackControls.tsx` and the standalone picker portion of `StudioWallFinishes.tsx` only after shared replacements preserve all behavior. Keep reusable wall-side diagnostics/guides. Update owning Studio docs.
**Interfaces:** render the T1 sheet and emit typed edits; patches stay in existing ArrangeDraft. The sheet reads staged asset/appearance values. Existing asset thumbnails/selection helpers remain reusable, not another browser/preview store.
**Behavior:** NPC family/style/skin, prop/wall atlas targets and declared wall sides appear through common fields. Current normal/emissive channel state is visible, not a fake independently editable list. Original/reset preserves owner semantics. Cancel stages no canonical mutation; Apply is atomic and one undo entry. Unrelated immediate facing/faction edits retain their policy. Missing/changed bindings keep explicit intent and block unsafe Apply.
**Tests:** `wall NPC and prop use the same field component`; `NPC style then skin resolves from staged look`; `Cancel preserves original document for every owner`; `Apply undo redo round-trip owner appearance only`; `changing model retains invalid old target visibly until explicit resolution`; `unavailable saved choice is not replaced`; `nonappearance pickers retain immediate behavior`; `normal-bearing masonry pack exposes its actual normal`.

- [ ] Add RED common component/transaction tests.
- [ ] Integrate shared panel, remove duplicated selection implementations and update callers/tests.
- [ ] Verify native save/reopen and immutable inputs; update UI documentation.
      **Verification:** `npm run test:run -- src/concepts/encounter-studio/appearance src/concepts/encounter-studio/StudioArrangePanel.test.tsx src/concepts/encounter-studio/EncounterStudioIntegration.test.tsx`; typecheck/lint; full `npm run ci-check` at PR update boundary.
      **Evidence:** actual shared component usages and DOM/history assertions, not three renamed copies.

### Task 8: Captured masonry renderer and normal resource delivery

**Delivers:** R4/R8/R9 in play without hidden geometry inference.
**Owner:** Web. **Prerequisites:** T5 actual generated TS SDK, T2 prepared provider, T6 joined API for final verification.
**Files:** create `src/rendering/wallFaceFinishes.ts`/`.test.ts`; create `src/components/session/CapturedWallSurface.tsx`/`.test.tsx`; modify `structuralLayout.ts`, `RoomSceneEnvironment.tsx`, `materialOverrides.test.ts`, and snapshot/reveal conversion adapters reached through current generated types; refactor the wall-local chart helper out of `wallFinishProjection.ts` for shared use; update `src/rendering/publishedWorldAtlas.test.ts` and normal packaging checks as needed; dependency lock and generated material projection.
**Interfaces:** `copyWallFaceFinishes(value, path): WallFinishOverrides` rejects malformed sides/duplicates/nonblank violations. `projectWallFaceFinishes({wall, finishes, catalog}): WallFinishSpan[]` uses the same local chart helper as Studio, exact provider binding and whole-wall interval. `CapturedWallSurface` loads the shared catalog only for selected finishes, renders named loading/refusal markers and passes valid spans to existing `StructuralWallSurfacePieces` before memory/lighting.
**Behavior:** +Z/-Z face choices are independent; phase0 and direction±1 agree with Studio across repeats, rotations and openings. Repeat meters and paired normal scale come from the material. No full source, inferred regional owner, regenerated floor or altered door clicks. Unsupported resource/binding shows an explicit failure, not silent Original. Opening replacements retain the list.
**Tests:** `captured Brick plus gray opposite face matches Studio local charts`; `rotation and opening cuts preserve phase`; `cold/reveal adapters retain both faces`; `opening-only patch retains finishes`; `missing material or mismatched GLB refuses visibly`; `unselected wall does not load material textures`; `normal map and scale applied before memory tint`; `two instances and Original reset leave source materials unchanged`.

- [ ] Add RED packet-to-render/chart equality tests.
- [ ] Implement shared local projection, packet mapping and rendering wrapper; adopt actual SDK and prepared provider.
- [ ] Verify selected color/normal resource hashes through normal runtime assembly; no candidate-only completion claim.
      **Verification:** focused `wallFaceFinishes`, `CapturedWallSurface`, `materialOverrides`, existing `worldSurfaceMaterials` and structural wall tests; `npm run world-materials:check`; pinned-runtime assembly; full `npm run ci-check` at PR boundary.
      **Evidence:** detached typed facts, equality with current Studio mapping and actual normal-map selection, not only a dropdown test.

### Task 9: Joined acceptance, review and release handoff

**Delivers:** complete user milestone and explicit release readiness.
**Owner:** Assets lead across the owning PRs. **Prerequisites:** T1–T8 green, actual generated bindings and pushed dependencies.
**Files/artifacts:** existing per-repo run records; one small native fixture containing Castle Wall Fit01, a House Wall05, a chair and an NPC; private screenshots/evidence under Assets; current issue/PR receipts. Do not rewrite canonical castle or invent the tree placement.

- [ ] Start an owned isolated stack after verifying port/process/container ownership; never operator3030/8115 or Lab3180.
- [ ] Use the common controls on wall/NPC/prop, Cancel then Apply, Undo/Redo, native save/reopen, Save&Play and reload. Verify paired color+normal on the masonry wall, independent opposite side, atlas on wall/prop and complete NPC look.
- [ ] Edit the source after launch; reload the captured run and prove it keeps the old appearances. Exercise ordinary partial discovery and opening update without duplicating general castle combat QA.
- [ ] Inspect hardware-rendered evidence and actual fetched normal/color hashes; zero candidate requests in normal pinned delivery. No new FPS/budget claims.
- [ ] Obtain the one bounded applicable independent review round at implemented heads; fix findings and close their exact dispositions. Do not repeatedly re-review mechanical pins.
- [ ] Report unmerged/pseudo pins honestly; after separately authorized merges, wait for real tags, re-pin consumers, run owning gates and verify final integration. Retain provider branches while referenced.
      **Verification:** owning full gates, independent reviews, native browser assertions and private inspected screenshot/resource receipt. The current tree-containing castle YAML and door work are not prerequisites.
      **Completion evidence:** all acceptance claims tied to exact revisions; no hidden remaining required lane. If a required publication, review, operator visual judgment or merge authorization is missing, state precisely what remains.

## Visible plan checks

### Requirement coverage

| Requirement                                            | Tasks       | Concrete proof                                                                                   |
| ------------------------------------------------------ | ----------- | ------------------------------------------------------------------------------------------------ |
| Provider declares valid entries; no parallel registry  | T1,T2,T7    | foreign-family rejection, valid non-recommended choice, generated metadata driving common fields |
| NPC/wall/prop shared controls retain domain meaning    | T1,T7       | same component usage, actual tuple resolution and appearance-only document diff                  |
| Existing paired normals plus honest channel states     | T1,T2,T8,T9 | material channel assertion, actual normalMap/scale/hash and viewed hardware output               |
| Original/default/off/selected are not conflated        | T1,T7       | fixed/unavailable states, owner reset, no unsupported Off command                                |
| Per-instance atomic history and native persistence     | T7,T9       | all-owner Cancel/Apply/Undo/Redo/native round-trip and source immutability                       |
| Masonry captured through reload and disclosure         | T3–T6,T8,T9 | cold SiteData reload, hidden-wall omission, snapshot/reveal equality, source-edited old run      |
| No mechanics/door/lighting regression or source fetch  | T3,T6,T8,T9 | nonappearance comparison, opening-only patch test, native/network evidence                       |
| Actual normal provider delivery and release discipline | T2,T5,T9    | source/seal checks, generated SDK receipts, pinned runtime hashes and released-pin gates         |

### Provider/consumer seams

| Provider                        | Consumer       | Produced vs consumed contract                                      | Availability/dependency                                | Proof                                                        |
| ------------------------------- | -------------- | ------------------------------------------------------------------ | ------------------------------------------------------ | ------------------------------------------------------------ |
| Existing NPC manifest/generator | T1/T7          | exact family/style/skin tuples → complete appearanceRef            | existing published capability                          | tuple-filter and staged-ref tests                            |
| T2 Assets                       | T1/T7/T8       | schema3 compatibleFamilies + unchanged resource identities         | prepare before normal adoption; exact pin at merge-out | publisher/reader agreement and foreign-family refusal        |
| T3 native compiler              | encounter/site | explicit raw wall face map → canonical FaceFinishes                | source geometry already present; no catalog lookup     | native/path validation and unchanged nonappearance facts     |
| T3 encounter                    | T4 SDK         | detached FaceFinishes on permitted walls/reveals                   | real pushed Go pin                                     | snapshot/reveal/aliasing and hidden-wall tests               |
| T4 SDK                          | T6 API         | same side/ref DTO + existing SiteRepository capability             | pseudo/released module pin and real proto SDK          | mapper plus cold-host Redis acceptance                       |
| T5 Protos                       | T6/T8          | WallFaceFinish(side1,material_ref2), wall field11 in both packages | CI-generated after authorized source merge             | exact generated adoption and application mapping tests       |
| T1/T7 editor                    | existing owner | typed edit → ArrangeDraft → one owner transaction                  | existing Arrange commit/native codecs                  | Cancel/Apply/Undo/Redo and unavailable-state tests           |
| T6 wire                         | T8 renderer    | face choices + existing permitted wall geometry                    | no region/source fetch                                 | chart equality, normal sampling and opening-patch tests      |
| All providers                   | T9             | normally delivered scene with captured looks                       | all owning prerequisites                               | actual native walk + fresh-host and pinned resource receipts |

### Check dispositions

- Path verification removed a stale `scripts/world-atlas-catalog.mjs` reference: the actual shared parser is `src/rendering/worldAtlasCatalogSchema.mjs`, imported by `scripts/world-material-catalog.mjs`. No task depends on the absent file.
- Initial full room-chart payload idea is unnecessary: inspected current region binding is always unresolved. Preserve working local mapping rather than implement retired inference. No new region-default feature is smuggled into the plan.
- NPC palette selection is complete-look identity, not arbitrary channel mutation. Its adapter stays distinct without duplicating UI.
- Upstream SiteData affects persistence and cold-host tests, so T6 owns the existing host capability explicitly instead of weakening the acceptance test or retaining an obsolete checkpoint shape.
- Generated SDK availability is a real dependency; no local generated substitute is planned. That wait blocks only the wire consumers, not T1–T4/T7 work.
- No implementation task consumes an unspecified capture/spans type. The wire carries exactly two strings per selected face; drawing derives only from delivered wall geometry and verified material metadata.
