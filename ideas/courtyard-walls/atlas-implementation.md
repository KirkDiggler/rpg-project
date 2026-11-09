# Courtyard S3 — atlas-pack adoption

**Goal:** select an existing 01_A–04_C texture pack for a wall or prop, retain its authored geometry/UVs and independent instance state, and carry the choice through Save & Play before moving to mixed wall/door sections.

**Authority:** [#895](https://github.com/KirkDiggler/rpg-project/issues/895); operator clarifications recorded on comments 6726578497 and 6726761156. Reuse [atlas-provider law A1–A8](../assets/pack-material-profiles/atlas-provider-design.md), not the superseded proposal to split timber panels for masonry projection. Independent A/B choice across texture families remains required but is expressly deferred. Mixed feature/door wall runs are next, not part of this texture implementation.

**Architecture:** the existing schema2 material catalog supplies shared color-only authored-UV materials, collection recommendations and exact model/surface bindings. Web consumes that contract and applies a selected material to one placement without altering cached GLBs. Existing masonry material records, charting and A/B controls stay separate and unchanged. There is no image-to-UV or node-name guessing, new asset variant per pack, second material publication authority, or mechanical material meaning.

**Inspected baseline:** Web 7677fec1 (runtime e40819ca), Assets fe6b5ee, current live material schema1. Assets #320/#324 are merged; `evidence/322-house-kit/atlas-193-candidate.json` includes the actual House Wall05 hash and atlas target. The twelve source atlas samples used by that wall's beams are identical. Current native/play DTOs carry no material selections; model appearance parity is not texture parity.

**Finish line:** existing texture collection usable on wall/prop placements; Original reset, history/native persistence and captured recipient play parity verified; normal provider publication and exact consumer pin; applicable gates/review and explicit merge authorization. No completion claim from a strict reader or private preview alone. Existing floor/tree composition and all authoritative barriers/doors remain unchanged.

## Ordered work and boundaries

1. **Consumer resource/binding reader and renderer adapter** — executable contract below. It consumes the already-agreed schema2 without requiring live publication. Provides real consumer-readiness evidence, not a fabricated acceptance flag.
2. **Studio selection/persistence and preview** — use existing arrangement/history owners. Persist explicit `{surfaceId, materialRef}` selections at the owning prop/wall appearance; absence means Original. The first UI picks the existing `atlas` target for the wall as a whole, not independent A/B. Reject duplicate/empty targets and incompatible/stale bindings visibly; recommendations are not a hard allow-list. Check and publish the concrete document/wire task contracts before editing those seams.
3. **Captured play chain** — additive proto/encounter/SDK/API data carrying the same opaque selections, with copy-out, immutable encounter reload and existing recipient disclosure. No source-scene fetch in play, no generated bindings by hand and no local Go replacements. Protos need their normal authorized merge/generation before real consumers adopt them. Inspect/record exact affected fields/copy/filter paths and tests before implementation; this is required remaining work, not deferred acceptance.
4. **Provider activation and final joined proof** — existing publisher source rebuild → validate → exact-seal apply/check in private worktree after actual reader/adapter evidence, then normal sync. Assets publication may be prepared on its issue branch; actual merges remain operator-owned. Native save/start/reload and two independently textured placements complete the proof before door work.

Only work whose concrete contract is checked below starts now. Later contracts must be completed from inspected code rather than guessed during implementation. This phased planning does not narrow the end-to-end finish line.

## Task A1 — Strict shared schema2 consumer reader

**Owner/files:** Web `scripts/world-material-catalog.mjs`; new `scripts/world-atlas-catalog.mjs` for atlas/collection/binding schema checks; `scripts/worldMaterialCatalog.test.ts` and new `scripts/worldAtlasCatalog.test.ts`; `src/rendering/worldMaterialTypes.ts` adds the shared atlas types. Existing `runtime-image-resources.mjs` remains resource authority.

**Interface:** `readWorldMaterialCatalog({catalogPath, modelRoot})` retains v1 `{catalogSha256, materials}` exactly. For v2 it additionally returns `schemaVersion:2`, `atlasMaterials`, `collections`, `bindings`: typed projections from the SAME catalog, not separate catalogs. `materials` remains the existing masonry view so existing physical chart consumers cannot accidentally consume atlas data. Atlas materials retain `mapping: authored-uv`, `normal:null`, `emissive:null` and one verified baseColor image. Collections and bindings retain the agreed public fields; image paths project to versioned runtime URLs.

**Behavior:** support exact schema1 and exact schema2; a partial v2 envelope refuses. Validate strict fields and identities, duplicate refs/IDs/targets, collection and suggested-collection references, selected material mapping kind, model/default-image hashes, UV channels and glTF sampler enums. Unknown fields, mixed masonry/atlas roles, null/invalid required records and dangling refs refuse. Suggested collections remain recommendations, not exhaustive permissions. The agreed v2 wire requires a nonempty valid recommendation list; a valid atlas material outside those lists is still retained for consumer experimentation. Reuse existing PNG decode/hash/CRC/dimension and exact owned-file checks across ALL image roles/rows. Never use metadata assertions in place of actual image verification. Binding shape validation alone does not certify actual GLB content; renderer/generator joins own that separate proof. Until generation/preview consumers are upgraded, `generate-world-material-catalog.mjs` and `stage-world-material-preview.mjs` explicitly refuse schema2 projections rather than silently discarding atlas fields. Add executable caller refusal tests to the existing generator/preview suites.

- [ ] RED: named mixed-catalog case preserves old masonry while exposing color-only atlas materials and exact declared targets; current v1 reader refuses it.
- [ ] Add refusal cases for incomplete/version-spoofed envelopes, atlas role/mapping errors, duplicate material/collection/binding/surface ownership, stale-shaped hashes, bad channels/sampler enums and dangling references.
- [ ] Implement parser with shared image verification; v1 behavior and existing resource-owner boundaries remain unchanged.
- [ ] Run `npx vitest run scripts/worldMaterialCatalog.test.ts scripts/worldAtlasCatalog.test.ts scripts/generateWorldMaterialCatalog.test.ts scripts/syncWorldMaterials.test.ts` and types/lint for changed files; no whole Web gate until PR boundary.

**Completion evidence:** expected RED, focused GREEN including old v1 refusals; actual schema2 candidate read/resource validation when its source-built resources are available. No generated/live material pin changes in this task.

## Task A2 — Instance-owned authored-UV material adapter

**Owner/files:** Web `src/rendering/worldMaterialTypes.ts` (new atlas types, masonry type unchanged), new `worldAtlasMaterials.ts` and `.test.ts`. Reuse the existing exact node/material target resolver from `worldSurfaceMaterials.ts`, without its masonry chart/default-normal assumptions.

**Interface:** `applyWorldAtlasMaterials({assetRef, glbSha256, root, binding, selections})` consumes a detached hierarchy, exact verified asset binding, and `{surfaceId, material, texture}` values; returns owned materials/samplers with idempotent `dispose()`. No selection is inert; absence/reset restores original references. The adapter does not fetch resources or store authoring state. Runtime resource decoding/checks remain a caller obligation before supplying textures.

**Behavior:** refuse asset/hash mismatch, missing/ambiguous node/material scopes, duplicate/unknown selected targets, bad authored channel or sampler mismatch. Apply only the named scope. Keep all geometry/UV attributes, material roughness/metalness/opacity/side and non-selected texture properties intact. Clone role/channel sampler state over shared source pixels; preserve the original transform/wrap/filter/channel instead of installing masonry repeat UVs. Never mutate GLTF cache materials/textures or another instance; dispose only owned clones, atomically refuse before mutation.

- [ ] RED: two copies can select different atlas colors on a bound scope while geometry/UVs and an unbound companion remain exact; original textures/materials stay untouched.
- [ ] Cover color-only/no fake normal or emissive, Original/reset and double-dispose, stale hash/target/channel/sampler, duplicate selection and invalid second target causing no partial mutation.
- [ ] Implement the pure adapter and typed contract; run `npx vitest run src/rendering/worldAtlasMaterials.test.ts src/rendering/worldSurfaceMaterials.test.ts` and `npm run typecheck`.
- [ ] Exercise real House Wall05 model/binding/source resources in the existing preview path before marking consumer readiness; unit fakes alone do not satisfy release acceptance.

## Task A3 — Additive captured selection wire

**Authority:** [texture design](texture-design.md), R2–R4. This transcribes the planned encounter/SDK `MaterialOverride{SurfaceID, MaterialRef}` value, not a new resource service or game mechanic. Concrete native storage is `materialOverrides` at each prop/wall appearance owner; snapshot/act records use the same value list.

**Owner/files:** Protos `dnd5e/api/session/v1alpha1/types.proto`, `dnd5e/api/session/v1alpha2/act_geometry.proto`, owning `docs/architecture/components/session-service.md` and a scoped run record. Baseline origin/main `fac26ce`; isolated `.worktrees/courtyard-895`.

**Interface:** each package gains `MaterialOverride { string surface_id = 1; string material_ref = 2; }`. Add `repeated MaterialOverride material_overrides = 10` to `PropPresentation` and `AtlasStructuralWall` in both packages. Field10 is unused in all four inspected records. No structural-door field yet: that belongs to the later door task, not guessed enrollment.

**Behavior:** both strings nonempty and target IDs unique within an owner; empty list means Original for that complete render record. A supplied invalid/unavailable override refuses visibly, not silent default. Records remain captured, follow owner audience, and contain no paths, image bytes, editor lookup IDs or mechanics. No separate recolor event or new RPC.

- [ ] Add only the named fields/messages and document their absence/validation/capture semantics; unchanged existing numbering.
- [ ] Run `make format` and `make test` in the proto worktree; normal contract checks only, no handwritten serialization tests or local SDK generation.
- [ ] Open the additive PR against main with design/consumer handoff. CI validates generation. Request the operator's actual merge authorization before generated adoption; never merge or push generated automatically.

**Completion evidence:** additive diff, local Buf gates and CI generation; wire publication is not end-to-end feature completion. Consumers wait for actual CI-published revisions.

| Requirement | Wire proof |
|---|---|
| Native selection identity survives snapshot and reveal | same MaterialOverride fields on both owner messages in both packages |
| No inferred default or hidden-scene lookup | explicit empty-list/original and invalid-selection semantics |
| No gameplay/material interpretation | opaque IDs only, owner disclosure unchanged |
| Release safety | checks + CI-owned SDK publication + explicit merge authorization |

**Seam check:** renderer adapter already consumes `{surfaceId, material}` and strict catalog bindings; selection persistence resolves materialRef at the consumer, not in the engine. Encounter/SDK follow-up must add matching MaterialOverrides fields, deep copies and captured recipient projection tests before API use; no locally invented binding is accepted as generated code. This contract task is executable before those consumers under the outside-in development policy.

## Task A4 — Native appearance capture in encounter

**Owner/baseline:** toolkit encounter module only, `.worktrees/courtyard-895`, stacked on ground provider 241e4263. No rulebook/API/proto imports. Published wire is v0.1.262/generated c23b703; Web has adopted the real package and types pass.

**Files/interfaces:** new `material_override.go` defines runtime `MaterialOverride{SurfaceID,MaterialRef}`, separate snake-case `MaterialOverrideData`, validation and detached conversions. Extend `PropPresentation`/`PropPresentationData`, `StructuralWallInput`/private `structuralWall`, `AtlasStructuralWall`, and `StructuralWallData` in their existing files. Capture/clone in `prop_presentation.go`, `structural_walls.go`, `projection.go`, `data.go`; encode full introduced wall rows in `structural_reveal.go`. Existing mutable prop observation paths already route through prop presentation conversions and clone helpers; verify rather than add a parallel observation store.

Native `dungeonspec/single_room.go::RoomWallAppearance` gains `MaterialOverrides []RoomMaterialOverride` with camel-case YAML/JSON fields; new `single_room_material_overrides.go` owns shape/value reading and conversion. `single_room_walls.go` checks the optional list during shape validation and copies it in `canonicalStructuralWalls`; `single_room_presentation.go::readPropPresentation` reads prop-owned selections from the source node. Empty/absent means Original; present null, non-list, empty/blank IDs, duplicate surfaces and non-string fields refuse with source paths. Runtime validates nonblank opaque strings/unique IDs, never catalog membership.

- [ ] RED new `material_override_test.go` methods on existing StructuralWallSuite / PlacedPropsSuite: construction-input and AtlasFor/ToData mutations cannot reach stored choices; JSON host reload preserves wall/prop selections; malformed choices fail NewEncounter and load; concealed owners do not leak selections.
- [ ] RED `dungeonspec/material_overrides_test.go`: actual native castle source plus two selected scopes lowers correctly, retains choices through input/native compilation and field data, and leaves geometry/blocking/regions/ground unchanged. Table-test null/wrong-kind/blank/duplicate values at both owner paths.
- [ ] Implement exactly the listed capture/copy/projection seams. Add narrow mutable-sighting/reveal assertions to existing scenario helpers; no new audience calculation.
- [ ] `GOWORK=off go test ./...` and module lint, then same-turn commit/push/draft provider PR. No local replacements or fabricated module versions.

**Completion:** all module packages/lint pass; actual pushed commit is available for the separate SDK worktree. Provider runtime no longer loses material choices across its own capture/reload/projection, but SDK/API/Web adoption is still required.

**Coverage/seam check:** native camel-case pair → encounter runtime pair → snake-case persisted/reveal pair → detached Atlas/observation pair. All conversions use the same two strings. Wall-opening replacement events do not resend or alter immutable appearance. The new list is mutable Go data, so copying only parent structs is insufficient; tests deliberately mutate nested entries. Existing appearance fields provide an owner/audience, not permission to fetch source or infer material rules.

## Task A5 — SDK transcription and reveal integrity

**Owner:** separate session module worktree `.worktrees/courtyard-895-session`, based on cab0ad8f, consuming pushed encounter e3849c88 through actual pseudo-version `v0.179.1-0.20261108215032-e3849c889aa5`. No local replacement or proto import.

**Files:** new `material_override.go` / `material_override_internal_test.go`; existing `types.go::AtlasStructuralWall`, `structural.go::projectStructuralWall/validStructuralRows`, `prop_presentation.go::PropPresentation/projectPropPresentation/validPropPresentation`, module go.mod/go.sum and run record. Existing explicit projection-field inventory tests in `atlasmap_test.go` / `convert_internal_test.go` gain MaterialOverrides rather than being weakened.

**Contract:** SDK `MaterialOverride{SurfaceID,MaterialRef}` has snake-case JSON tags matching captured reveal data. Owner fields are `MaterialOverrides []MaterialOverride` with omitempty. Projection allocates detached slices. Shared validity requires nonblank strings and no repeated surface ID; invalid wall/prop lists refuse the whole reveal body through existing decoders. No catalog lookup, resource-derived rules, defaults or audience logic.

- [ ] RED methods on existing PropPresentationDecodeSuite: snapshots carry both owner lists without aliasing; valid event payload retains identical selections; malformed/null-item/blank/duplicate/wrong-kind selections refuse wall and prop payloads wholesale.
- [ ] Implement DTO/projection/decoder validation and update explicit field-inventory assertions for the added fact.
- [ ] Run `GOWORK=off go test ./...` and `GOWORK=off golangci-lint run ./...` in session; commit/push and open a draft against the ground SDK branch. Actual release tag replaces pseudo-version before merge.

**Seam check:** provider runtime pair → SDK-owned pair → existing raw JSON event decoder pair is field-for-field. Snapshot and event tests name the same material refs; this seam never looks up current definitions to enrich old observations. API can consume the resulting actual pushed SDK revision next.

## Task A6 — API transcription and real saved-host proof

**Owner/baseline:** API `.worktrees/courtyard-895`, stacked on 9a3896e. Actual SDK 579875f9 and nested generated module `rpg-api-protos/gen/go` at c23b703; no local replacements or generation.

**Files/interfaces:** snapshot `internal/handlers/dnd5e/session/v1alpha1/convert_structural.go` and `convert_prop_presentation.go`; act `v1alpha2/convert_geometry.go`. Add one package-local list converter per wire version, mapping SDK SurfaceID/MaterialRef to generated MaterialOverride. New focused conversion tests in each package cover both owners and detached output. New `internal/integration/session/testdata/material-overrides.yaml` and `material_override_acceptance_test.go` use existing Redis-backed acceptance harness/native compile/fresh host read helpers.

**Behavior/proof:** no new RPC, permission or business logic. Native fixture has one wall and one decorative prop with atlas_04_c; launch normally, read GetKnowledge, edit/recompile source to atlas_01_a, create a fresh API host over the saved repositories, and still read original selections. All floor, wall and prop identities/poses remain unchanged. Full owner records, not a client-side source fallback, supply the choices.

- [ ] RED snapshot/act conversion tests for both appearance owners with nonempty lists and no aliasing.
- [ ] RED Redis/native/fresh-host acceptance test; implement only field transcription and dependency pins.
- [ ] Run focused handler/integration checks, then `make test` (including cmd/server) and `make ci-check` at PR boundary. Commit/push/draft with generated version and actual toolkit pins.

**Seam check:** session owns validation and audience; API only transcribes the same pair. Protos field10 is present in both actual generated packages. Nested `gen/go` is the imported module, not the repository's root Go module. SDK/toolkit release tags and remaining Web/publisher adoption still gate readiness.

## Task A7 — Generated and private-preview atlas views

**Owner/files:** Web `scripts/generate-world-material-catalog.mjs`, `stage-world-material-preview.mjs`, `src/generated/worldMaterialCatalog.ts`, `src/rendering/worldMaterialCatalog.ts`, and associated reader/generator/preview tests. Reuse the pure atlas metadata validator for browser boundary checks; no new catalog authority.

**Contract:** generated module retains existing masonry exports and adds typed `GENERATED_WORLD_ATLAS_MATERIALS`, `GENERATED_WORLD_MATERIAL_COLLECTIONS`, `GENERATED_WORLD_ATLAS_BINDINGS` from the same provider/digest. V1 emits empty atlas views, never fake choices. Runtime catalog carries optional `atlas: {materials,collections,bindings}`; masonry callers continue using their existing material view. A2 receives selections resolved against these exact bindings. Private preview uses the existing dev-only opt-in and dedicated ignored resource root; v2 image URLs stay hash-bound and contained. Each atlas binding must match the selected generated world asset and the actual staged model bytes.

- [ ] Replace transitional refusal tests with executable generated-module assertions retaining all atlas rows/bindings; v1 tests remain green.
- [ ] Extend existing preview transaction to stage all owned shared images and atlas metadata after model identity checks. Test real temporary files, unowned-output refusal, mismatched model hash, and no partial overwrite on failure.
- [ ] Extend runtime pinned/preview catalog handling without confusing authored-UV atlas materials with masonry. Invalid/stale/unavailable bindings stay explicit errors.
- [ ] Regenerate the still-live v1 metadata through its normal generator, retaining the provider pin/bytes and empty atlas views; do not activate an unapproved v2 provider.
- [ ] Run focused generator/preview/catalog tests and type checking. Actual private source-built v2 candidate staging demonstrates the joined reader/metadata boundary; it is not publication acceptance or the missing UI/play wiring.

**Seam check:** producer schema2 → A1 strict typed views → generated/runtime optional atlas view → A2 verified material target adapter. Previous v1 consumer guards are removed only as each projection carries the full view, not before. An independently pinned world catalog may reject stale atlas bindings; the client never fixes their hashes itself.

## Task A8 — Native selection data and Studio history

**Owner/files:** Web rendering types gain `MaterialOverride{surfaceId,materialRef}`; new pure `concepts/world-building/materialOverrides.ts` validates/copies owner lists and changes one explicit target. Extend `WorldProp` in `types.ts`, wall appearance in `structuralWalls.ts`, `serialization.ts` and `structuralWallEditing.ts`. Use existing nested-owner codec evolution; no new root scene field/store/version is needed. Add tests `materialOverrides.test.ts` and native current-castle import/persist/export coverage.

**Behavior:** absent/empty retains Original; null/wrong-kind/blank strings/duplicate targets refuse. Preserve unknown authored fields by the existing carryUnknownKeys mechanism, and never alias nested selection arrays. Changing/resetting one scope preserves unrelated scopes; future A/B does not require a shared global override. Pure data validation does not invent catalog compatibility.

- [ ] RED invalid-owner-input tests at actual prop and wall codecs (unknown fields previously passed through without validating these values).
- [ ] Add native import/persist/reopen tests with wall and prop choices, Original reset and all other source facts preserved.
- [ ] Implement optional owner fields and detached pure helpers; preserve existing source round-trip and wall/ground tests.

**Studio wiring contract (next checked edit):** stage choice in existing ArrangeDraft/Apply/Cancel, dispatch through current scene-edit or wall-edit transaction and history. Select the provider-declared atlas scope; expose Original and named 01_A–04_C choices without pretending they are masonry faces. Selection resolution checks the binding/asset hash; incompatible explicit choices remain visible. No independent browser state or fake per-side UI.

**Verification:** focused materialOverrides/serialization/native wall tests, types and changed-file lint. UI/history and live renderer wiring remain required after these data tests; data retention alone is not a picker or Save & Play completion.

## Task A9 — Shared resource access and product render wiring

**Owner/files:** move the existing resource hook to `src/rendering/useWorldMaterialCatalog.ts` with a compatibility re-export from its old concept path. New `worldMaterialPreviewLoader.ts` owns one cached, validated request per explicit dev preview; tests prove shared success/failure without fallback. Normal pinned metadata is one immutable cached view. The existing MODE=development + explicit preview flag remains unchanged.

`WorldAssetModel` gains optional `materialOverrides` and a prepared atlas branch; resolve exact model/scope/material against the shared catalog, then call A2 before existing memory/lighting treatment. `WorldPropModel`, structural-wall surfaces and placed-prop render callers pass the owner values. Extend `useRememberedModelTint` ownership/cleanup to compose atlas treatment before overlays, restoring it afterward. No shared GLTF/image mutation or per-model catalog request.

Player `propPresentations.ts` and `structuralLayout.ts` validate/copy generated materialOverrides into the same existing rendered owner fields; both snapshot and reveal paths use those conversions. Never reconstruct choices from source or guess a material from the asset name.

**Checks:** shared-request tests, actual material target resolution/missing/refusal tests, existing renderer/tint/structural surface tests, live snapshot/reveal conversion tests, and real native Save & Play on the owned stack. Product UI is not complete until that joined path is shown. The private preview remains candidate-labeled; normal provider publication/custody and final pins still apply.

## Joined checkpoint — 2026-11-08

- A1/A2: Web reader and authored-UV adapter implemented; real House Wall05 01_A/04_C rendering and pixel-exact Original reset verified.
- A3: operator merged Protos #425; generated c23b703 / TS v0.1.262 adopted.
- A4: encounter e3849c88, draft #2330; all nine module packages and zero-issue lint pass.
- A5: session 579875f9, draft #2333; both packages/lint pass on the real encounter pseudo-version.
- A6: API 8059e8b, draft #1209; full tests including cmd/server and ci-check pass. Actual native/Redis/fresh-host source-immutability proof passes.
- A7–A9: Web 60f802e9, draft #1401. Full gate passes 10,086 tests, five skipped. Existing Stage preview transaction retains all twelve atlas choices/193 bindings and validates actual model bytes. Native owner fields, existing Arrange transactions, cached shared catalog access, memory/lighting composition and player snapshot/reveal adapters are wired.
- Actual Studio on :3183: selector exposes Original/01_A–04_C; Cancel/Apply/Undo/Redo and native Save a copy → Play pass. After reload and movement, recipient metadata and actual wall material both select atlas_04_c on authored UV0; inspected images and no page exceptions. [Exact heads and early-look handoff](https://github.com/KirkDiggler/rpg-project/issues/895#issuecomment-6729452773).
- Still open: operator texture-control look, one applicable independent review, normal material publication/custody, release pins/authorized merges and accepted composition fold. The private candidate is not a normal delivery or shipped claim. The existing brick-source rebuild differs at one red value in one pixel; unchanged versions, CPU-dispatch probes and Blender-hosted derivation reproduce the new result, so the historical-byte cause remains unresolved. Do not fabricate acceptance or source from installed output.

## Current coverage and seams

| Requirement | Task | Proof / remaining work |
|---|---|---|
| Reuse published schema2/01_A–04_C, not masonry surgery | A1 | mixed catalog and exact resource/binding checks |
| Preserve UVs, beams, defaults and independent instances | A2 | real material isolation/reset tests, then actual model preview |
| Select/save/undo/redo per wall/prop | 2 | required next checked authoring contract; not claimed by A1/A2 |
| Same saved choice in recipient play/reload | 3,4 | required captured protocol chain and joined walk |
| Normal publication, no fabricated Ready/readiness | 4 | source-built existing publisher transaction, exact provider pin |
| A/B later, mixed wall pieces afterward | explicit sequencing | neither lost nor implemented prematurely |

| Provider | Consumer | Contract / check |
|---|---|---|
| Existing Assets v2 candidate | A1 | agreed mixed materials/collections/bindings; resource checks reuse current verifier |
| A1 atlas projection | A2 + later loader | exact refs/hash/node/material/channel/sampler; no fabricated normal or UV projection |
| Existing GLTFLoader hierarchy | A2 | named node may be Group; actual material scopes validated; owned clones only |
| Existing v1 masonry | unchanged consumer | exact old typed view and rejection/resource tests remain green |
| Saved surface/material selection | later native/play owners | identity shape named here; field numbers/lifecycle tests must be checked before editing seams |

**Plan check:** schema2/collections/bindings were already approved and prepared. Do not reopen that wire or repeat ingestion. An atlas swap preserves UV topology; it does not justify arbitrary image pairing or an independent A/B UI without separately declared targets. The former per-face miter investigation is retained as case evidence, not this implementation. Actual resource/GLB checks and captured play parity are separate completion obligations.
