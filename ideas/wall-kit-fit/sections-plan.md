# One wall with individually placed sections — continuation

**Authority:** Courtyard S6 [#1022](https://github.com/KirkDiggler/rpg-project/issues/1022), under #867. The operator accepted the aligned textures, identified the three-object sample as unfinished, and asked to continue with one owning wall. This extends the existing [plan](plan.md); it does not replace its remaining inventory/native/play obligations.

**Goal:** one selectable wall owns its line, height, thickness, elevation and face finishes. A window/decorative section occurs once at its authored position and replaces only that visual span. It is not a separate wall/prop or a gameplay opening. Door/opening authority stays unchanged.

**Inspected baseline:** Web `df2949e0`, Assets data `05cfd6a`; existing `StructuralWall`, `WallGeometry`, `StructuralWallSurfacePieces`, `projectCapturedWallFinishes`, native room document writer; Toolkit `StructuralWallInput`/`RoomWallAppearance`; proto `AtlasStructuralWall`. Exact provider module bases are recorded when their isolated worktrees are opened.

**Seam finding / ruling:** existing `openings` are gameplay gaps; reusing them would change blockers. The height-only plan needed no wire field, but a once-placed section needs a captured visual list. Add that list to the existing wall owner rather than another registry, hidden prop attachment or fake opening. This follows the approved ownership and appearance-only behavior; no new gameplay authority or independent disclosure entity is introduced.

## Contract

- Authored `wall.appearance.sections?: WallSection[]` with `{id:string, assetRef:string, position:number, width:number}`. Position is the centre distance from the wall start; width is its visual replacement span, both scene units. IDs are nonempty and unique within their wall. Omitted means none; explicit empty remains empty. The owner codec retains unknown nested fields under the existing forward-preservation contract.
- Captured `AtlasStructuralWall.sections` carries the same facts in canonical feet (`ref` rather than authoring `assetRef`), through Encounter → Session → API → proto. No catalog lookup occurs in those layers. Legacy absence means no sections. This immutable component follows the wall's existing disclosure and survives opening-only replacement unchanged.
- Creation uses explicit provider `wall-section` plus `wall-body-fit` capability and measured native width, not asset names. Captured/imported references remain opaque; new edits validate capability without silently replacing imported intent.
- Sections fit entirely inside the line and neither overlap each other nor actual opening/surround spans. Touching endpoints are valid. Invalid dimensions, duplicate IDs and overlap refuse before owner commit. A failed transaction leaves document/history unchanged.
- Height/thickness/elevation and compatible face finishes are inherited. Provider-specific overrides for the plain wall are not blindly applied to another asset's slots. Source fixed details/caps remain protected by the common height assembler.
- Move/rotate the wall: every section follows it. Resize a wall endpoint: preserve section world positions along that axis and clamp at protected section edges, as for doors; never stretch, discard or silently reposition the feature. Delete the wall: its owned sections disappear with it. Remove a section: restore plain art, leaving the wall/blocker/openings unchanged.
- Rendering subtracts section intervals only from the plain visual repeats, not from `wallSolidIntervals` used by mechanical authoring/lowering. One feature instance receives its full span fit and wall-relative texture phase. Fog/finish boundaries clip the configured geometry, never squeeze or multiply its fixed feature. Absent stretches draw nothing.
- Native files retain this field through existing source-aware writers; no alternative persistence service. Final play uses captured data, never the source scene.

## Tasks

### S6.5a — owned section document and editing geometry

**Owner:** Web world-building. **Prerequisites:** current provider metadata and common height/material assembly.
**Files:** new `wallSections.ts`, `wallSections.test.ts`; `structuralWalls.ts`, `structuralWallEditing.ts`, `wallHeightEdits.ts`; existing corresponding tests and `singleRoomDungeon.test.ts`.
**Interfaces:** `WallSection`; detached parser; pure insert/update/remove helpers; visual exclusion geometry; endpoint-resize protection using shared interval arithmetic without changing mechanical openings.
**Tests:** a 12-unit wall with one 3.75-unit section centred at 6 remains one wall, has unchanged blocker and zero gameplay openings, and has plain visual spans [0,4.125] / [7.875,12]. Duplicate IDs, nonfinite/zero width, beyond-end, section overlap and door-surround overlap refuse. Two touching sections pass. Translate/rotate/resize preserve owned records and input isolation. Resize from Start shifts local centres to preserve world pose; shrink clamps. Removing restores the uninterrupted visual span. JSON/native round-trip retains fields and unknown siblings.
- [x] RED then GREEN core codec/edit/interval tests.
- [x] Integrate owner validation and wall transforms; endpoint resize protects section edges and shifts Start-relative centres.
- [x] Verify editor-native round-trip without changing mechanical declarations. This is not engine compilation (5d remains open).
**Verification:** `npx vitest run src/concepts/world-building/wallSections.test.ts src/concepts/world-building/structuralWalls.test.ts src/concepts/world-building/structuralWallEditing.test.ts src/concepts/world-building/wallHeightEdits.test.ts src/concepts/world-building/singleRoomDungeon.test.ts` in the Web worktree.

### S6.5b — render once and inherit wall charts

**Owner:** Web rendering. **Prerequisite:** S6.5a.
**Files:** `StructuralWallSurfaces.tsx`, new `StructuralWallSections.tsx` and tests; `WorldPropModel.tsx`, `WorldAssetModel.tsx`, new detached geometry clipping helper/tests if required by partial fog; existing `plainWallFit.ts`, `wallSurfaceCharts.ts` remain fit/chart owners.
**Interfaces:** same `StructuralWallSurface.appearance.sections`; section-local placement and full-span fitting; temporary visible interval clipping only at the render boundary.
**Tests:** one wall/one window yields exactly one feature and no base art over its span; target heights 3.75/6 preserve fixed geometry; frame/above/cap and neighbouring plain pieces share phase at a non-repeat-aligned position. Partial remembered/absent coverage clips only supplied stretches without stretching the feature, duplicate caps or mutating cached source. Door surround and section coexist without shared state.
- [x] RED/GREEN synthetic render/clip tests; actual one-wall source meshes measured at an off-repeat-aligned window position.
- [x] Shared rendered placement, finish inheritance, cleanup and fog clipping. Loaded fitter omission mutant fails without clipping.
**Verification:** focused new renderer tests plus `StructuralWallFit.test.tsx`, `StructuralWallVisual.test.tsx`, `assetHeightInstance.test.ts`, `assetHeightCharts.test.ts`.

### S6.5c — ordinary Studio authoring and history

**Owner:** existing Web document owner and Studio ports. **Prerequisites:** S6.5a–b.
**Files:** `WorldBuildingConcept.tsx`, `studioSession.ts`, `StudioInspector.tsx`, new `StudioWallSections.tsx` and tests; `fixtures/fakeStudioSession.tsx`, `EncounterStudioIntegration.test.tsx`; extend the existing placement gesture path if needed rather than duplicating document ownership.
**Interfaces:** a wall-scoped section edit command through the existing document/history transaction. Choose a configured feature, place/adjust its centre along the owning wall, remove it. The wall remains the inspected/selected owner; sections are not `scene.items` or independent wall entries.
**Tests:** add/window/move/remove, wall height change, Undo/Redo, native save/reopen. Each command yields one history entry; failed overlap/minimum-height edit writes nothing. A wall remains one document row and its blocker/openings are byte-equivalent after appearance-only edits. Existing served-castle round-trip remains covered.
- [x] RED/GREEN owner/port/UI tests through existing `wallEditing.edit`; no new document/history owner.
- [x] Add a separate isolated one-wall sample at :3192; earlier preview drafts remain intact.
**Verification:** focused panel and owner integration tests; early actual Studio view before final review/play walk.

### S6.5d — native and captured visual list

**Owner:** existing Protos, Toolkit Encounter/Session, API boundaries. **Prerequisite:** consumer contract exercised by S6.5a–c; worktrees and each owning module's instructions before edits.
**Files:** Protos `dnd5e/api/session/v1alpha1/types.proto` plus v1alpha2 act geometry where it owns the corresponding type. Encounter: new visual-section type/tests, `structural_walls.go`, `data.go`, `atlas.go`, `structural_reveal.go`, `dungeonspec/single_room.go`, `single_room_walls.go`, `single_room_decode.go` as required by the actual decoder. Session: `structural.go`, `convert.go` and existing atlas/event deep-copy/round-trip tests. API: both `internal/handlers/dnd5e/session/v1alpha{1,2}/convert_structural.go`/`convert_geometry.go` owners and joined native session fixture. Web runtime: `components/session/structuralLayout.ts`, `applyReveal.ts` and their tests with generated bindings.
**Interfaces:** captured section list in feet, exact opaque refs/IDs; all complete-wall introduction, snapshot, persistence and replay lanes carry independent copies. Opening-only replacement preserves it. No new RPC, permission, mechanics, independent child disclosure or source fetch.
**Tests:** native value → canonical feet → persistence reload → two-recipient snapshot/reveal → Web scene-unit conversion; legacy absent list; duplicate/invalid/overlapping sections refuse; unchanged movement/sight/door topology. Hidden wall carries no section facts. Opening replacement does not erase/recompute sections. Inputs/results are detached. Legacy no-section persisted SiteHash remains unchanged if optional data is absent. The separate compiler ContentHash includes every compiled struct field and is engine-version-local.
- [x] Inspect provider package instructions/current bases; create isolated worktrees. Protos base ba987c8, Encounter base 771116fb, API base 2cddb1a3. Session reconciles main after #2502 landed.
- [x] Additive schema checks: Protos #449 at b17e26b, local `make format`/`make test` and remote generation/SDK checks green; publication still requires authorized merge.
- [x] RED/GREEN Encounter/native and Session projection/decode tests; full module race tests and lint pass. Separate module PRs #2503 / #2505. API/generated-wire integration remains below.
- [ ] Join on actual generated bindings/pushed candidate versions; keep incomplete/pseudo-version consumers draft.
**Verification:** proto `make format`, `make test`, and CI-owned generation/SDK checks (owning repository explicitly does not require local generation for contract authoring); named module-focused tests during development; owning full module/API gates at PR boundaries. No generated bindings hand edits or fake release tags.

### S6.5e — joined delivery evidence

**Owner:** Assets team across existing line of work. **Prerequisites:** S6.5a–d.
- [ ] One actual wall with a door and a once-placed window/decorative section, wall height/finish edits, Undo/Redo, native save/reopen and Play, and independent captured rendering.
- [ ] One ordinary Web gate at the completed PR update; focused Assets regressions, not a broad verification campaign.
- [ ] One bounded independent review of the completed branches. Required fixes use focused RED/GREEN tests.
- [ ] Update #1022/parent story and exact pins. No merge/operator deployment without authorization; actual releases before merge-out.

## Coverage and seam checks

| Requirement | Owner/task | Concrete proof |
|---|---|---|
| One logical wall, one feature | 5a–c | One wall row, no new items/openings, one configured mesh |
| Preserve aligned textures and fixed geometry | 5b | Non-grid-aligned phase continuity; measured 3.75/6 heights |
| Wall owns lifecycle and history | 5a, 5c | Move/rotate/resize, refusal, removal, Undo/native reload |
| Appearance never changes blockers/doors | 5a, 5d | Mechanical declarations/topology unchanged with sections |
| Captured play / independent clients | 5d–e | Native compiler → persistence/event → both clients; hidden-wall absence |
| Current wider scope stays visible | parent plan | Inventory/adapters/Original charts remain tracked, not silently waived |

| Producer | Consumer | Contract / availability | Joined proof |
|---|---|---|---|
| Assets candidate 05cfd6a | 5a–b | Existing body/height/surface facts; nine explicit `wall-section` tags | Exact refs and loaded geometry |
| 5a owner document | 5b renderer, 5c Studio | Wall-local visual sections, scene units, no mechanical cuts | One-wall visual/history tests |
| Native room appearance / Encounter 5d | Session/API/proto 5d | Opaque captured list in canonical feet, optional absence | Round-trip/deep-copy/disclosure tests |
| Generated schema + runtime adapter 5d | Shared 5b renderer | Feet converted once; section facts preserved through opening patches | Synthetic plus actual Play view |

No implementation reuses gameplay openings to store visual sections. Native additions are required work, not an optional promise to defer after a Studio-only result.

## Shared visuals versus ownership — continuation ruling

The future direction is one consistent wall-insert authoring experience and shared visual placement/fitting/material primitives, not one universal gameplay or disclosure record. Doors are wall inserts visually, but their passage, mutable state and independent disclosure remain in the existing opening/door owners. Static window/decorative sections remain immutable visual facts on their owning wall. Both can feed shared visual tooling without duplicating a door's pose or requiring a hidden parent to draw an independently permitted door.

The current section record is therefore a durable representation of static wall-owned appearance, not a temporary door substitute. The implementation uses the existing wall edit/history/native paths and shared `WorldPropModel`/height/material fitting; it adds no parallel asset registry, fake gameplay opening or replacement persistence service. The local `_job_1022_*` launchers are untracked preview aids, not shipped public application code. Common insertion controls and further placement-helper consolidation can grow over these owners without discarding the captured section data.

The session's earlier statement that the separation itself was necessarily excessive conflated visual reuse with canonical ownership. The useful finding is that the current static section chooser does not accept doors; the incorrect inference was that their distinct captured records must therefore be temporary. The operator asked to record the reasoning and continue provided no throwaway architecture is added. Proceed on the durable distinction above; do not force door state or independently disclosed door facts into the wall section array.

The operator authorized recording door adoption separately where appropriate. [Web #1478](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1478) tracks shared door/static-section visual authoring and any justified migration, coordinated with existing #1234/#1402. It does not absorb this slice's unfinished native/capture/Play work or prescribe collapsing distinct owners into one record.

This resolves the architectural publication hold, not merge/deployment authorization. The existing proto-only SDK-publication decision remains operator-owned.

## Provider checkpoint and publication dependency

- Protos #449: b17e26b, additive wall `sections = 12` in both session versions; contract and remote generation gates pass. No SDK publication until merge.
- Encounter #2503: 9c166b2d. Native decoding/lowering, immutable capture, detached persistence/read/reveal paths, real finder-vs-peer disclosure tests; full `go test -race ./...` and lint pass.
- Session #2505: 43538a14. Real candidate pin `v0.237.1-0.20261120222209-9c166b2d562e`, detached seam DTOs, required numeric presence and malformed-whole-reveal refusal; full race suite and lint pass. The already-merged Encounter initiative change initially broke the old Session pin, so the branch consumed existing #2502 and then merged main after it landed. No duplicate initiative implementation or rebasing.
- The first Encounter full gate exposed six JSON atlas characterizations adding only `Sections:null` and the engine-version-local ContentHash change. Programmatic comparison proved no other golden delta. Updated the content hash pin without modifying its generic encoder. The persisted SiteHash/empty-list checks remain green.
- API's isolated worktree is clean and unchanged. API mapping and Web runtime adoption are blocked on actual generated bindings. No generated-branch push, manual SDK copy or local replace is used to bypass the publication seam. Only a specific operator authorization to merge Protos #449 is being requested; all other merge/deployment holds remain.

## Studio checkpoint (not Play completion)

Web `53bcb915` supplies the editor continuation. `http://127.0.0.1:3192/_job_1022_sections.html` has two comparison samples, each exactly one wall containing one real door and one window section, with zero extra scene items. Both actual surfaces reach heights 3.75/6; editing the high wall to 5.25 moves all three visual components together while the window's fixed lower/cap geometry stays unchanged. Moving the window and editor-native encode/decode preserve the owning wall. Chart errors stay below 2e-7.

The section inspector edits apply immediately outside the wall's staged placement form. Sections tagged by the provider are no longer offered as new repeating walls, while old imported repeat intent remains renderable. Full Web gate: 11326 passed, 30 skipped, 880 files passed/1 skipped; format/lint/types/build/production guards passed.

The preview's **two engine refusals are expected and unresolved**: the currently connected backend does not read `appearance.sections`. They are not suppressed. Task 5d must add native compilation, immutable capture and recipient delivery before Play is complete. This is the early real editor view required before the play walk/final review, not a narrowed finish line.
