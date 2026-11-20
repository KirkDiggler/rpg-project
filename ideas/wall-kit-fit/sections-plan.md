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
- [ ] RED then GREEN core codec/edit/interval tests.
- [ ] Integrate owner validation and all wall transforms.
- [ ] Verify native round-trip without changing mechanical declarations.
**Verification:** `npx vitest run src/concepts/world-building/wallSections.test.ts src/concepts/world-building/structuralWalls.test.ts src/concepts/world-building/structuralWallEditing.test.ts src/concepts/world-building/wallHeightEdits.test.ts src/concepts/world-building/singleRoomDungeon.test.ts` in the Web worktree.

### S6.5b — render once and inherit wall charts

**Owner:** Web rendering. **Prerequisite:** S6.5a.
**Files:** `StructuralWallSurfaces.tsx`, new `StructuralWallSections.tsx` and tests; `WorldPropModel.tsx`, `WorldAssetModel.tsx`, new detached geometry clipping helper/tests if required by partial fog; existing `plainWallFit.ts`, `wallSurfaceCharts.ts` remain fit/chart owners.
**Interfaces:** same `StructuralWallSurface.appearance.sections`; section-local placement and full-span fitting; temporary visible interval clipping only at the render boundary.
**Tests:** one wall/one window yields exactly one feature and no base art over its span; target heights 3.75/6 preserve fixed geometry; frame/above/cap and neighbouring plain pieces share phase at a non-repeat-aligned position. Partial remembered/absent coverage clips only supplied stretches without stretching the feature, duplicate caps or mutating cached source. Door surround and section coexist without shared state.
- [ ] RED/GREEN actual synthetic mesh and phase tests.
- [ ] Shared rendered placement, finish inheritance, cleanup and fog clipping.
**Verification:** focused new renderer tests plus `StructuralWallFit.test.tsx`, `StructuralWallVisual.test.tsx`, `assetHeightInstance.test.ts`, `assetHeightCharts.test.ts`.

### S6.5c — ordinary Studio authoring and history

**Owner:** existing Web document owner and Studio ports. **Prerequisites:** S6.5a–b.
**Files:** `WorldBuildingConcept.tsx`, `studioSession.ts`, `StudioInspector.tsx`, new `StudioWallSections.tsx` and tests; `fixtures/fakeStudioSession.tsx`, `EncounterStudioIntegration.test.tsx`; extend the existing placement gesture path if needed rather than duplicating document ownership.
**Interfaces:** a wall-scoped section edit command through the existing document/history transaction. Choose a configured feature, place/adjust its centre along the owning wall, remove it. The wall remains the inspected/selected owner; sections are not `scene.items` or independent wall entries.
**Tests:** add/window/move/remove, wall height change, Undo/Redo, native save/reopen. Each command yields one history entry; failed overlap/minimum-height edit writes nothing. A wall remains one document row and its blocker/openings are byte-equivalent after appearance-only edits. Existing served-castle round-trip remains covered.
- [ ] RED/GREEN owner/port/UI tests.
- [ ] Replace the three-object demonstration with a separate isolated one-wall sample; preserve earlier previews.
**Verification:** focused panel and owner integration tests; early actual Studio view before final review/play walk.

### S6.5d — native and captured visual list

**Owner:** existing Protos, Toolkit Encounter/Session, API boundaries. **Prerequisite:** consumer contract exercised by S6.5a–c; worktrees and each owning module's instructions before edits.
**Files:** Protos `dnd5e/api/session/v1alpha1/types.proto` plus v1alpha2 act geometry where it owns the corresponding type. Encounter: new visual-section type/tests, `structural_walls.go`, `data.go`, `atlas.go`, `structural_reveal.go`, `dungeonspec/single_room.go`, `single_room_walls.go`, `single_room_decode.go` as required by the actual decoder. Session: `structural.go`, `convert.go` and existing atlas/event deep-copy/round-trip tests. API: both `internal/handlers/dnd5e/session/v1alpha{1,2}/convert_structural.go`/`convert_geometry.go` owners and joined native session fixture. Web runtime: `components/session/structuralLayout.ts`, `applyReveal.ts` and their tests with generated bindings.
**Interfaces:** captured section list in feet, exact opaque refs/IDs; all complete-wall introduction, snapshot, persistence and replay lanes carry independent copies. Opening-only replacement preserves it. No new RPC, permission, mechanics, independent child disclosure or source fetch.
**Tests:** native value → canonical feet → persistence reload → two-recipient snapshot/reveal → Web scene-unit conversion; legacy absent list; duplicate/invalid/overlapping sections refuse; unchanged movement/sight/door topology. Hidden wall carries no section facts. Opening replacement does not erase/recompute sections. Inputs/results are detached. Legacy no-section hash remains unchanged if optional data is absent.
- [ ] Inspect provider package instructions/current bases; create isolated worktrees.
- [ ] Additive schema/generation checks (no hand-written proto tests).
- [ ] RED/GREEN provider and seam tests; one toolkit module per PR.
- [ ] Join on actual generated bindings/pushed candidate versions; keep incomplete/pseudo-version consumers draft.
**Verification:** proto `make test`, `make generate`; named module-focused tests during development; owning full module/API gates at PR boundaries. No generated bindings hand edits or fake release tags.

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
