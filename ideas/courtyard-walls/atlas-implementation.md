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
