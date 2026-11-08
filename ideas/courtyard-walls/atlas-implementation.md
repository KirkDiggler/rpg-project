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

**Behavior:** support exact schema1 and exact schema2; a partial v2 envelope refuses. Validate strict fields and identities, duplicate refs/IDs/targets, collection and suggested-collection references, selected material mapping kind, model/default-image hashes, UV channels and glTF sampler enums. Unknown fields, mixed masonry/atlas roles, null/invalid required records and dangling refs refuse. Suggested collections remain recommendations; an empty recommendation list does not make a valid target unusable. Reuse existing PNG decode/hash/CRC/dimension and exact owned-file checks across ALL image roles/rows. Never use metadata assertions in place of actual image verification. Binding shape validation alone does not certify actual GLB content; renderer/generator joins own that separate proof. Until generation/preview consumers are upgraded, `generate-world-material-catalog.mjs` and `stage-world-material-preview.mjs` explicitly refuse schema2 projections rather than silently discarding atlas fields. Add executable caller refusal tests to the existing generator/preview suites.

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
