# Authored normal/emissive controls — implementation plan

> Execute directly with the executing-plans skill, task by task. Delegation is
> not implied. Project design/planning and build-slice contracts apply.

**Goal:** Adjust existing verified normal/emissive maps through Appearance and
preserve the exact intent through native save, captured Play and reload.

**Architecture:** Assets declares exact capabilities. A shared typed adjustment
value and arithmetic kernel serve existing owner/renderer adapters; Toolkit
captures that value under existing static and observed presentation ownership.
No replacement textures, new scene lights or generalized appearance store.

**Tech stack:** Python/GLB provider tooling; TypeScript/React/Three/Vitest;
Go encounter/session modules; Buf/protobuf and existing API converters.

**Spec and authority:** [design](design.md), [walkthrough](README.md),
[operator agreement](https://github.com/KirkDiggler/rpg-project/pull/1189#issuecomment-7185560714).
R1–R10 are settled. The plan is executable without another approval ceremony.

**Tracking:** [Courtyard S7#1192](https://github.com/KirkDiggler/rpg-project/issues/1192),
[playtest#1193](https://github.com/KirkDiggler/rpg-project/issues/1193),
[Web1402](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1402), design PR1189.

## Global constraints and finish line

- Default is absence; Off is explicit; Scale is finite and positive. Explicit1
  remains explicit. Off cannot carry a multiplier; empty target rows refuse.
- Source-zero emission stays fixed. No guessed enabled tint or missing map.
- Adjustment ranges are provider-declared, positive, finite and contain1.
- Only verified targets/representations are enrolled. Initial proof candidates:
  Castle Wall01 masonry and `goblin-wizard-01` standing/downed. Candidate identity
  is not acceptance; check actual mapped contribution before publishing.
- Preserve geometry, normal-basis signs, source tint, cached resources, original
  behavior of unadjusted documents, and unrelated fields/native metadata.
- Existing wall-body projection covers only declared compatible masonry sections
  and fixed frames. No new side/section/leaf editing mode or door palette scope.
- No source-document fetch in Play; no hidden-parent lookup; memories retain the
  observed values. Unsupported explicit intent remains present and diagnostic.
- Licensed files/evidence remain private. Normal provider acceptance/custody,
  module gates, one authorized independent review round and released pins apply.
- Done: named targets work visibly in Studio and native/captured Play, including
  reload/cold host and recipient boundaries; actual checks recorded, PRs prepared
  for operator-owned merges. Publication/merge/deployment are separate authorities.

## Inspected baseline and sequence

| Owner | Baseline | Planning evidence |
| --- | --- | --- |
| Web | ea623f57 | `worldMaterialTypes.ts`, `materialOverrides.ts`, shared Appearance files; `ClassCharacterModel.materialOwnership` |
| Assets | 7f6d3ce | schema3 material catalog; `build_world_material_catalog_v2.py`; source material JSON inspection |
| Encounter/session | a193b5fb | `material_override.go`, `prop_presentation.go`, `door_frame_presentation.go`, `testimony.go`, `data.go`; session `read.go`, `launch.go`, `types.go` |
| Protos | 6cc9e94 | snapshot `types.proto`, live `act_geometry.proto`, existing session/presentation namespace |
| API | 6f13cf71 | session snapshot/live material, prop, structural and frame converters |
| Project | 4f7bf1b design draft | approved in conversation and recorded on PR1189 |

All code trees use `.worktrees/material-channels`, branch `feat/material-channels`;
Project retains `design/material-channels`. Session has its own Toolkit tree
`.worktrees/material-channels-session`, branch `feat/material-channels-session`.
No shared checkout edits. One Toolkit module per PR.

Development: T1 consumer value → T2 capabilities and T3 wire contract → T4
canonical capture → T5 host/API → T6 rendering → T7 editor/joined delivery → T8
proof/release. T3 can proceed after T1's value contract independently of provider
publication. No concurrent writers are required. T2/T6/T7 share Web files and
therefore execute serially. Toolkit consumer pins use real pushed provider
commits; generated proto SDKs require the operator's proto merge and CI output.

Merge: agreed proto contract/generated SDK; approved Assets publication/merge;
Encounter merge/tag → Session real tag pin/merge/tag → API real pins/merge → Web
merged provider pins/merge. This sequence does not grant merge authority.

## Current dependency finding

Protos456 is operator-merged at8bc21d0; publication CI46061574223 succeeded.
SDKv0.1.293 peels to generated37f124a and includes Go/TS channel bindings. Web
normally merged current dev96f317a5 and adopted that SDK at0b4a939e; typing and84
focused/regression checks passed. The base merge retained upstream's npcDescriptor
rename rather than recreating it here. No full Web gate or visible delivery claim.

T2/T8's real emissive proof target is unresolved: the one world emissive-map
candidate has zero source factors, and all72 published NPC looks with emissive
body maps have zero nonblack texels in a conservative base-level mapped footprint.
Private evidence is Assetsdf47957. [Assets351](https://github.com/KirkDiggler/rpg-game-assets/issues/351)
tracks one genuine emissive proof asset. The operator approved bounded additional
candidate preparation; publication remains separate. Original Castle Wall01
has black unlinked emission. A source-bound Dark Fantasy Lantern01 candidate
retains authored emission strength1 and has a visible authored/off difference
(4362 pixels in the staged Blender comparison). Its old source image names are
bound through an explicit three-resource candidate alias table, not renderer
inference. Runtime promotion/capture acceptance is still required.

## Shared task interfaces

**Authored/captured value:** `MaterialChannelAdjustment {targetId, normal?,
emissive?}`, with `ChannelAdjustment = {mode:'off'} | {mode:'scale',multiplier}`.
Owner fields are named `materialChannels` in native/TS and `material_channels`
in persisted/wire forms. The existing owner supplies asset identity; rows contain
neither texture refs nor parent identities. Absence/empty owner list is default.
Native codecs retain unknown source metadata; captured copies keep known facts only.

**Capability leaf:** `ChannelCapability = {kind:'absent'} | {kind:'fixed'} |
{kind:'adjustable',minMultiplier,maxMultiplier}`. This is provider output, not a
runtime guess. Effective-look resolution may refuse/restrict a declared capability
when the selected look lacks a compatible nonzero contribution; it never grants
one from image presence.

**Provider binding:** schema4 adds `channelBindings` to the existing shared
material catalog; schema3 remains readable unchanged. A binding names `assetRef`,
`representation` (`model`, `standing`, `downed`), `glbSha256`, and `targets`.
Each target has `id`, `label`, exact `selectors` (existing node/material selector
vocabulary), and `normal`/`emissive` capabilities. Source-verification receipts
bind selected channel image hashes, sampling/UV/basis, and nonzero default facts
for every selector. Existing world-surface/atlas and NPC manifest identities
remain their respective source authorities; no second resource catalog.

**Wire:** new `dnd5e/api/session/presentation/material/v1alpha1/types.proto`
in a neutral material subpackage beneath presentation defines `MaterialChannelAdjustment`
(`target_id`, optional message `normal`, optional message `emissive`) and
`ChannelAdjustment` with a oneof of `google.protobuf.Empty off` or
`double multiplier`. Missing channel message means default; a present message
with no oneof is invalid. Both snapshot and live geometry import this common
leaf; neither imports the other's version package.

**Runtime:** `resolveMaterialChannelValues` takes an immutable effective material
snapshot, the target's capabilities and its optional adjustment, and returns the
normal-scale vector/emissive intensity to apply. Default preserves original
values; normal Off yields `[0,0]`; emissive Off yields intensity0; Scale multiplies
only the corresponding authored values. Map pointers and emissive tint remain
unchanged. It rejects zero/fixed/absent and out-of-range explicit requests and
nonfinite results before a material writer mutates anything.

## Review focus

1. Source-zero emission with a nonblack texture must not become white/on (T2/T6).
2. A standing/downed or remembered transition must not lose settings or use live
   source values; a new model variant cannot borrow another GLB's selectors (T2/T4/T7).
3. Explicit1, Off and absent/default must survive every codec without aliasing or
   default collapse, including source metadata in native documents (T1/T3–T5/T7).
4. Mixed wall/section/frame materials must not enroll trim, leaves or unsupported
   inserts; a visible door cannot disclose its hidden parent (T4/T6–T8).
5. Invalid stale look/capability edits and material-treatment failures must not
   partially change history, shared caches or another instance (T2/T6/T7).

## Tasks

### Task 1: Shared adjustment value and native retention (T1)

**Delivers:** R4/R6/R7/R9's wire-independent value and refusal semantics.
**Owner:** Web, common rendering-value helpers and native codec adapter.
**Prerequisites:** approved contract; normal worktree `npm ci`.
**Files:** create `src/rendering/materialChannelTypes.ts`,
`materialChannelAdjustments.ts`, `materialChannelAdjustments.test.ts`;
create `src/concepts/world-building/materialChannels.ts` and
`materialChannels.test.ts`. Existing `materialOverrides.ts` and
`strictShape.ts` establish the captured-copy/native-metadata split.
**Interfaces:** `copyMaterialChannelAdjustments(value: unknown, path?: string):
MaterialChannelAdjustment[]`; native `validateMaterialChannels(value: unknown,
path: string): MaterialChannelAdjustment[]`. Capability leaf type is declared
beside adjustment types; no resource or UI dependency.
**Behavior:** accept lists only; unique nonblank IDs; at least one channel per
row; validate owned mode/multiplier presence. Preserve IDs verbatim, exact positive
values and explicit1. Reject null/undefined present channels, unknown modes,
missing multipliers, Off-with-multiplier, zero/-0/negative/NaN/infinite values,
empty/duplicate targets. Captured copies strip unrelated metadata; native wrapper
retains unknown row/channel metadata and detaches it through `carryUnknownKeys`.
**Tests:** `copies mixed modes without normalizing identity or explicit one`;
`refuses invalid adjustment shapes`; `detaches nested channel values`;
`retains future native row and channel metadata without aliasing`.
- [x] Write the named tests and observe the missing-module RED.
- [x] Implement the typed copier and native wrapper only.
- [x] Run focused GREEN plus existing Appearance/material regressions.
- [x] Commit the independently testable value contract: Web83020f7e;70 tests, lint/types passed. No UI or full Web gate claim.
**Verification:** Web `npm run test:run -- src/rendering/materialChannelAdjustments.test.ts src/concepts/world-building/materialChannels.test.ts`; then existing
`worldSurfaceMaterials.test.ts` and `appearanceSheet.test.ts`. Run lint/type checks
on the checkpoint; full `npm run ci-check` at the Web PR boundary, not each edit.
**Evidence:** exact RED/GREEN logs, assertions and commit; no visible feature claim.

### Task 2: Verified provider capabilities and versioned readers (T2)

**Delivers:** R3/R9/R10. **Owner:** Assets and Web catalog boundary.
**Prerequisites:** T1 types; source recipes/GLBs available; no publication permission implied.
**Files:** create Assets `scripts/material_channel_capabilities.py` and
`test_material_channel_capabilities.py`; modify `build_world_material_catalog_v2.py`,
`world_material_catalog.py`, `world_material_atlas_candidate.py`,
`publish_world_materials.py`, their relevant tests,
`docs/pipelines/world-materials.md` and selected source recipe. Web modifies
`scripts/world-material-catalog.mjs`, `src/rendering/worldMaterialTypes.ts`,
`worldMaterialCatalog.ts`, `worldAtlasCatalogSchema.mjs`/`.d.mts`, corresponding
catalog/generator tests. Actual consumer paths also include
`scripts/generate-world-material-catalog.mjs`, `stage-world-material-preview.mjs`,
`src/rendering/useWorldMaterialCatalog.ts`, the existing hook test and new
`src/rendering/materialChannelCatalogSchema.test.ts`. These preserve the new fact
through generation/staging/runtime projection; they are not another registry.
Generated catalogs change only through normal generators.
**Interfaces:** `verify_channel_bindings(catalog: dict, provider_root: Path,
*, world_provider_root: Path | None = None) -> list[dict]`
validates the shared schema4 binding shape above; versioned readers produce typed
bindings on `WorldMaterialCatalog`. Source recipe version3 adds `channelTargets`
to existing `wallMaterials`/`atlas`; legacy version2 recipes still reproduce schema3.
**Behavior:** verify actual used selectors/materials/maps, UV/sampling and basis,
nonzero default contribution, exact hashes and all consumed representations.
No image-only enrollment, blanket animated-mesh relaxation, hidden alias catalog,
or automatic installation. Candidate ranges include0.5/1/2 only if verification
and the later visual proof support them; synthetic test ranges are not publication.
**Tests:** `test_nonzero_verified_channels_enroll`; `test_zero_emission_stays_fixed`;
`test_missing_map_or_uv_or_stale_glb_refuses`; `test_representation_selector_mismatch_refuses`;
`test_range_must_be_finite_positive_and_contain_one`; Web schema3 regression and
schema4 round-trip/malformed binding tests.
- [x] Add refusing/passing provider and reader fixtures; run RED.
- [x] Implement source checks and schema4 generation without installation.
- [ ] Finish canonical lantern registration and combined proof-target staging; independent wall/lantern source facts and wall-only v4 candidate are retained privately.
- [x] Run focused provider/reader GREEN; update contract docs and custody checks.
**Verification:** Assets `python3 -m unittest discover -s scripts -p 'test_material_channel_capabilities.py'`; Web `npm run test:run -- scripts/worldMaterialCatalog.test.ts scripts/worldAtlasCatalog.test.ts src/rendering/worldMaterialCatalog.test.ts src/rendering/worldAtlasCatalog.test.ts` plus changed generator tests.
**Evidence:** exact staged catalog/receipt identities and verified target list;
publication remains T8.

Contract checkpoint: Assets945904a ([draft352](https://github.com/KirkDiggler/rpg-game-assets/pull/352))
passes90 focused provider/schema/publication tests. Webbbdad020
([draft1503](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1503)) passes182 focused
checks and the complete two-CPU gate (11834passed/31skipped; no test limits changed).
The earlier full run's4 timing/readiness failures remain recorded; all4 also
passed unchanged in isolation. Real CastleWall01 normal and staged-lantern emission
bindings verify. A real source-built **wall-only** schema4 candidate has digest
`037467dfd81b77c2e74dcb0dbe07e787805253c3237d40d9fb69a67596b33a35`, unchanged existing
material/atlas records, and stages through the Web preview reader. This does not
complete the lantern's canonical world registration or combined real enrollment;
those and publication/captured-play evidence remain open. The first verifier
profile is opaque source materials; nonopaque sources need opacity-aware proof. No broad Assets suite or art reapproval campaign.

### Task 3: Shared wire leaf and captured-owner fields (T3)

**Delivers:** R7/R9. **Owner:** Protos. **Prerequisites:** T1 value contract.
**Files:** create `dnd5e/api/session/presentation/material/v1alpha1/types.proto`;
modify `dnd5e/api/session/v1alpha1/types.proto` and
`dnd5e/api/session/v1alpha2/act_geometry.proto`; update the existing session
component contract and add a run record.
**Interfaces:** shared messages above; additive repeated `material_channels` on
PublicMemberInfo (snapshot field11, beside observed appearance_ref),
AtlasStructuralWall (field13), PropPresentation
(field11), DoorFramePresentation (field3); mirror applicable geometry fields in
v1alpha2. Verify field availability again before editing; no field renumbering.
**Behavior:** no new RPC or auth classification; normal/emissive message presence
and the oneof preserve all three intentions. Static, frame and observed member
owners remain separate. Unknown/never-seen actors are not added to any roster.
**Tests:** contract format/lint/breaking and CI-generated SDK validation only;
no hand-written tests of protobuf mechanics.
- [x] Add documented shared messages and additive owner fields.
- [x] Run `make format` and `make test` against current main623df7e.
- [x] Operator merged [Protos456](https://github.com/KirkDiggler/rpg-api-protos/pull/456) at8bc21d0; publication CI46061574223 passed.
- [x] Verified genuine SDKv0.1.293→generated37f124a, merge ancestry and both language bindings; Web adopted it through normal npm tooling.
**Verification:** Protos `make format && make test`; configured GitHub checks.
**Evidence:** PR/head/checks and genuine generated SDK identity; no manual generated code.

### Task 4: Canonical capture, persistence and observed custody (T4)

**Delivers:** R4/R5/R7/R9. **Owner:** Toolkit encounter module only.
**Prerequisites:** T1 semantics; no protobuf dependency.
**Files:** create `material_channels.go`, `material_channels_internal_test.go`,
`dungeonspec/single_room_material_channels.go` and its tests. Modify inspected
`field.go`, `data.go`, `testimony.go`, `encounter.go`, `projection.go`,
`prop_presentation.go`, `prop_observation.go`, `atlas.go`, `structural_walls.go`,
`door_frame_presentation.go`, `structural_reveal.go`, `dungeonspec/compile.go`,
`single_room_compile.go`, `single_room.go`, `single_room_decode.go`,
`single_room_presentation.go`, `single_room_material_overrides.go`,
`single_room_walls.go`, `single_room_lowering.go`, `single_room_door_frame.go`;
relevant comparison/hash/observation tests, native named fixture
`dungeonspec/testdata/material-channels/site.yaml`, readback's explicit NotYet
classification test, and `doc.go`.
**Interfaces:** canonical `MaterialChannelAdjustment` and `ChannelAdjustment`
with explicit mode and multiplier; persisted counterparts preserve presence.
`validateMaterialChannels`, deep-copy/data conversion helpers. Add typed fields
to existing wall/prop/frame, member/reserve/input and sight representations.
**Behavior:** structural validation only; no asset catalogs or unit conversion of
multipliers. Native capture/ToData/Load preserve exact values and empty legacy
behavior. Snapshot mutable appearance at sight; unknown has no facts; remembered
values are not backfilled. Deep copies include nested adjustment values. Opening-only
updates preserve wall adjustments. Frame payloads contain no parent ID.
**Tests:** `TestMaterialChannelsValidateAndDetach`; native mixed Off/Scale1/Scale0.5
compile and reload; `TestMaterialChannelsRememberedAppearanceDoesNotRefresh`;
`TestMaterialChannelsUnknownTestimonyRefusesFacts`; wall-hidden/door-permitted and
opening-only reveal assertions; empty legacy SiteHash unchanged. Recompute content
hash/goldens only from the measured combined encoding, never choose a stale pin.
- [x] Add named discriminating tests and see missing/omitted-field RED.
- [x] Implement canonical leaf, native capture and every existing custody route.
- [x] Run focused tests, then full module/race/lint at its PR boundary.
- [x] Push single-module [Encounter2618](https://github.com/KirkDiggler/rpg-toolkit/pull/2618), head4b9aee80, draft.
**Verification:** in encounter module `GOWORK=off go test ./...`,
`GOWORK=off go test -race ./...`, `GOWORK=off golangci-lint run`; targeted
`go test ./... -run MaterialChannels` during development.
**Evidence:** immutable/remembered/recipient assertions plus module checks and real pin.

T4 complete at4b9aee80 on base8517aac9. Full module and race checks pass; lint0.
Two observer/source boundaries and independent frames are tested. Removing only49
new nil channel fields from the castle encoding reproduces the base hash exactly;
new combined ContentHash4bcaf68f… is deliberate. Six goldens differ only by added
null fields; the persisted legacy SiteHash7c7fa22b… stays unchanged. Natural-language
read-back remains explicitly NotYet, not silently claimed. Adjacent unscoped Atlas
MaterialOverrides omission is tracked separately in Toolkit2617.

### Task 5: Session projection and API boundary (T5)

**Delivers:** R7/R9. **Owner:** separate session module PR, then API PR.
**Prerequisites:** T4 pushed commit; T3 generated SDK for API.
**Files:** session `types.go`, `read.go`, `launch.go`, `structural.go`,
`convert.go`, `prop_presentation.go`, `door_frame_presentation.go`, new
`material_channels.go`, `material_channels_internal_test.go`, `material_channels_test.go`,
existing roster/atlas/shape conversion tests and `doc.go`. API snapshot/live `convert_material.go`,
`convert_structural.go`/`convert_geometry.go`, `convert_prop_presentation.go`,
`convert_door_frame.go`, member conversion in `convert.go`; new scoped
`material_channels_test.go` and `internal/integration/session/material_channels_acceptance_test.go`
with `testdata/material-channels.yaml`.
**Interfaces:** session-owned detached types, never public aliases of encounter
internals; API produces T3 shared messages from canonical session values. Use
normal `go get @<pushed-sha>`/tidy and published SDK tooling; no local replacements.
**Behavior:** no dropped fields in snapshot or live reveal, no Off→default collapse,
no aliasing, no source-file enrichment. Keep authorization/visibility owners unchanged.
**Tests:** session source/input/output mutation isolation, remembered roster/reveal;
API native launch→read→save/reload→cold-host and two recipients; explicit1 and Off
have exact wire presence, hidden parent absent while frame values remain available.
- [x] Pin T4 and write the session boundary RED tests; implement and verify.
  [Session2623](https://github.com/KirkDiggler/rpg-toolkit/pull/2623), headab0d12c9,
  normally caught up to main and retaining Encounter4b9aee80. Full tests/race/lint
  pass. Correction: package race elapsed is not timeout margin; TestMain prepares
  fixtures before m.Run. No timeout change. Current-head remote checks remain unclaimed.
- [x] Adopt real SDK/T5 provider commit; write API conversion/integration RED tests.
  Retained dev's generated SDK294; normal Go pins consume Sessionab0d12c9.
- [x] Implement converters only; run normal gates and push separate PRs.
  [API1299](https://github.com/KirkDiggler/rpg-api/pull/1299), headf8369513, draft;
  complete local ci-check passes, including race/coverage tests. Native/two-recipient/
  cold-host/frame-with-hidden-wall evidence is in its tests and run record. Neither
  draft is merge-ready; release pins and whole-wave acceptance remain T8.
**Verification:** session module full/race/lint commands as T4; API
`GOWORK=off make ci-check`. Generated mechanics are not retested in Protos.
**Evidence:** provider and API claims separated; cold-host/recipient integration output.

### Task 6: Shared arithmetic with existing material owners (T6)

**Delivers:** R3/R4/R8/R9. **Owner:** Web rendering. **Prerequisites:** T1 for the
pure arithmetic kernel; T2 for real binding resolution and material-owner wiring.
The kernel can be implemented/tested before provider publication; doing so does
not complete T6's binding, ownership or visual verification.
**Files:** create `src/rendering/materialChannelValues.ts`,
`materialChannelMaterials.ts` and their tests; modify `src/components/hex-grid/useRememberedModelTint.ts`,
`ClassCharacterModel.tsx`, `WorldAssetModel.tsx`,
verify `src/npc-activities/ActivityCharacterModel.tsx` forwarding, and add ownership/lifecycle tests;
existing surface/atlas treatment paths only where needed for effective-look input.
**Interfaces:** pure `resolveMaterialChannelValues(input: {source, capabilities,
adjustment?}): {normalScale, emissiveIntensity}` with immutable tuple snapshots.
Binding resolution uses T2's pristine verified selectors and the existing fitted
instance target/provenance mechanism; no material-name guesses. World adapters
own clones; NPC `materialOwnership` keeps its stable owned clones and restores
base snapshots before applying adjustments/overlays. Extend its snapshots for
normal scale; do not replace its existing material lifecycle with a second writer.
**Behavior:** preserve normal signs/ratio and emissive tint/maps; Off only zeroes
selected contribution. Validate all targets and finite results before mutation.
Dispose/reapply/StrictMode never accumulates multipliers or touches shared cache.
**Tests:** source normal[-1,0.6] ×0.5→[-0.5,0.3]; emissive intensity3 ×2→6 with RGB
unchanged; default exact restore; Off independently zeroes chosen channel; zero
source and nonfinite/out-of-range refuse; two instances isolated; repeated setup/
cleanup not cumulative; fitted repeat/cap targets all receive the same logical
adjustment; unsupported trim/leaves unchanged; standing/downed selectors differ safely.
- [x] Write arithmetic/binding/lifecycle RED tests.
- [x] Implement kernel and adapt the existing world/NPC material writers.
- [x] Run focused GREEN and established surface/atlas/height/activity regressions.
**Verification:** Web `npm run test:run -- src/rendering/materialChannelValues.test.ts src/rendering/worldSurfaceMaterials.test.ts src/rendering/worldAtlasMaterials.test.ts src/components/hex-grid/WorldAnchoredHeight.test.tsx src/components/hex-grid/ClassCharacterModel.animation.test.tsx` plus new lifecycle tests.
**Evidence:** numerical, ownership and integration assertions, not just mocked calls.

Kernel checkpoint: Webba4cb643 passes104 focused/regression tests plus eslint and
typing. Default/Off/scaling, signed normals, source-zero refusal, interval checks,
detachment/nonaccumulation and shader-float overflow are covered. Writer checkpoint
Web535f7593 ([draft1503](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1503))
adds shared pristine binding/write planning, world clone treatment and stable NPC
snapshot treatment. Existing activity adapters already forward the typed property.
177 focused/regression tests across19files and complete local ci-check pass
(11855passed/31skipped). Real fitted repeats/cap, primitive slots, stable ownership,
StrictMode/reset/overlays and representation changes are tested. This completes
T6's code contract, not T7 controls or T8 real-asset visual acceptance.

### Task 7: Shared controls, native adapters and captured rendering (T7)

**Delivers:** R1/R5/R6/R7/R9/R10. **Owner:** Web. **Prerequisites:** T1/T2/T3/T5/T6.
**Files:** `appearance/{appearanceTypes,appearanceSheet,appearanceEdits,StudioAppearancePanel}`
and tests; `StudioArrangeFields.tsx`, `StudioArrangePanel.tsx`; world-building
`types.ts`, `structuralWalls.ts`, `roomDraft.ts`, `studioArrange.ts`, `serialization.ts`,
`wallFinishProjection.ts`, `doorFrameCapture.ts`, `RoomActorMarkers.tsx`;
`components/hex-grid/HexEntity.tsx`; session `structuralLayout.ts`, `applyReveal.ts`,
`SessionCanvas.tsx`, native integration/captured tests; manifests only via npm SDK update.
**Interfaces:** optional `materialChannels` on the three native owners; typed
channel `AppearanceEdit` names target/channel/mode/value. Default edits remove
only that channel and then empty rows. Existing complete-look adapters validate
retained settings against the new effective capabilities before Apply. Generated
wire values map to the T1 leaf and T6 inputs without querying source YAML.
**Behavior:** controls only on eligible targets; fixed/absent/read-only states
truthful. Staged atomic Apply/Cancel, exact Undo/Redo, duplicate isolation,
unknown native metadata survives. Existing per-side finish choices remain but
no new side-channel mode. Frame capture resolves permitted fixed scopes only;
actor and mutable prop rendering consume observed values, never live enrichment.
**Tests:** wall/prop/NPC controls; unavailable reset; stale selection/invalid range
creates no history; unrelated Apply retains channel values; switching to an
incompatible look refuses until explicit reset; default/off/1 native round-trip;
current served castle round-trip; captured wall/frame/member/mutable-prop reload;
unknown/hidden recipients receive no added facts; source refs and mechanical door
state remain unchanged.
- [ ] Add shared-panel/owner/native and wire-adapter RED tests.
- [ ] Implement adapters and staged controls, then connect permitted captured values.
- [ ] Run focused GREEN; full `npm run ci-check` before Web PR opening/update.
**Evidence:** real owner transitions and generated-wire integration, not a second store.

### Task 8: Named enrollment, early view, walk, review and release pins (T8)

**Delivers:** integrated R1–R10 finish line. **Owner:** lead across existing owners.
**Prerequisites:** T1–T7, genuine dependency pins, isolated environment.
**Files:** private Assets `evidence/material-channels/` and selected recipe/catalog
changes via normal custody; one untracked API `content/material-channels-walk.yaml`;
Project plan/checklist/run record; normal dependency files only for final repins.
**Interfaces:** approved provider candidate/receipt; served API/Web heads and exact
pins; saved native document and captured values; reproducible review/walk evidence.
**Behavior:** named wall normal plus genuinely mapped nonzero-emission asset;
Original/default exact restore; Off/0.5/1/2 only within verified ranges; two
placements differ independently. Source-zero case stays fixed. No broad enrollment,
operator deployment, parked-gateway work or substitute door-palette work.
- [ ] Boot own isolated ports and offer the early view before walk/review.
- [ ] Verify actual material effects, height/normal basis, standing/downed and
  supported frame behavior through ordinary editing; preserve private evidence.
- [ ] Obtain any required exact provider publication approval; install atomically
  with genuine receipt and normal merged-source consumption policy.
- [ ] Run named native/Play/reload/cold-host and two-recipient checks plus applicable
  standing castle steps; measure costs before deferring them.
- [ ] Conduct the required bounded independent feature review after the operator's
  early look; fix/disposition findings and run affected gates.
- [ ] Hand operator the inside-out merge order; adopt real tags/pins and recheck
  current heads. Preserve tests/evidence at their actual tested revisions.
**Verification:** normal provider build/check/sync tools; API `make ci-check`; Web
`npm run ci-check`; configured CI and actual current-head review verdicts; named
operator checklist. Read walk/pr-review/merge-out skills at the relevant boundary.
**Evidence:** private screenshots/measurements, saved/read-back values, exact heads,
review dispositions, release table. Cleanup/deployment need their own authority.

## Requirement coverage

| Requirement | Tasks | Concrete proof |
| --- | --- | --- |
| Authored maps, no texture swapping/new lights | T2/T6/T7/T8 | capability-only controls; unchanged maps/tint; no new light objects |
| Exact binding, representation, ranges | T2/T6 | stale hash/UV/variant/range refusal; standing/downed selectors |
| Default/Off/explicit1 and source-zero | T1/T3–T7 | copy/native/wire/reload cases; zero emission fixed; reset source exact |
| Normal basis and finite arithmetic | T6/T8 | signed[-1,0.6] example; fitted wall/cap/door visual proof |
| Atomic history, stale edit and native metadata | T1/T7 | deep copy and unknown metadata tests; refused edit no history |
| Wall/section/frame scope and disclosure | T4/T5/T7/T8 | unsupported material unchanged; visible independent frame/hidden wall |
| Member/mutable-prop memory and cold reload | T4/T5/T7/T8 | source changed after sight does not repaint memory; cold-host integration |
| Owned resources and repeat lifecycle | T6/T7 | two instances, StrictMode/cleanup/reapply, no accumulation |
| Unsupported intent and old documents | T1/T2/T4/T7 | preserved unavailable values, no substitute; legacy SiteHash/native fixture |
| Usable verified delivery | T8 | named real targets, operator early view, full native/Play evidence and gates |

## Provider / consumer seam check

| Provider | Consumer | Produced vs consumed | Availability/dependency | Proof |
| --- | --- | --- | --- | --- |
| T1 value | T3/T4/T6/T7 | targetId + optional Off/positive Scale; explicit1 preserved | T1 first | same boundary fixtures |
| T2 catalog | T6/T7 | schema4 channelBindings, representation/hash/selectors/capabilities | staged candidate before publication | parser and stale-binding tests; exact published sync |
| T3 Protos | T5/T7 | shared presence-aware leaf on existing owners | genuine generated SDK after operator merge | API and Web use generated types |
| T4 encounter | T5 session | detached canonical static/observed rows | pushed real Go version, later actual tag | source-mutation and persistence tests |
| T5 session | API | session-owned types, no inner aliases | separate module PR and real pin | session boundary guard plus API integration |
| API snapshot/live | T7 Web | same static/member/frame values and absence | SDK + API provider pins | native→capture→wire→renderer/cold resume |
| T6 kernel | T7 owner renderers | immutable baseline → resolved values; ownership remains caller's | source snapshots and caps | world + NPC lifecycle assertions |

## Plan check dispositions and explicit exclusions

- Actor rendering already owns stable clones in `ClassCharacterModel`; therefore
  T6 shares arithmetic while preserving that writer instead of adding a competing
  clone/replacement hook. Source normal scale must join its existing snapshots.
- Source image presence is not emission eligibility. Zero-source and mapped
  contribution checks remain explicit; no fabricated visible proof fixture.
- Snapshot/live packages must not import each other. T3 uses a neutral material
  subpackage beneath presentation for the new leaf, rather than copying oneof logic.
  The first same-package attempt failed Buf's option-uniformity checks because
  legacy presentation files have explicit Go/Java options and new files must not.
  A dedicated material package preserves both policies and existing generated
  identities without weakening lint or putting material types in the dice file.
- T3 pre-edit inspection corrected the plan's shorthand `Member` to
  `PublicMemberInfo`: only the latter carries observed appearance_ref. The
  position-only Member message stays untouched; this preserves the approved
  observed-appearance owner rather than adding a new delivery route.
- Case-file publication/receipt changes are a separate deliverable from enabling
  schema4 parsing. Read-only candidates do not authorize installed data changes.
- New RPC/auth policies, gameplay calculations, source asset ingestion, texture
  swapping, new side editors, player equipment/customization and performance
  rewrites are not needed for this cut.
- Full provider/consumer interface check above is a direct plan check, not an
  independent code-review claim. No new architectural decision is pending;
  verification can still expose a concrete gap, which returns under design's gap rule.
