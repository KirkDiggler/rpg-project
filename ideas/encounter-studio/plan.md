---
name: Encounter studio S0 — implementation plan
status: PLAN for slices S0 and S1, derived 2026-10-14 from design.md and the S0 issue; checked against the inspected baseline below; open items resolved by the team lead 2026-10-14; S1 planned 2026-10-14 against the S0 branch; S2–S5 are stubs
authority: design.md (rulings R1, R9, R11, R13 and the document's refusals); README.md (the inspector forms); rpg-project#574 (S0 acceptance); epic rpg-project#545
planning guide: ../../.agents/skills/design/planning.md
---

# Encounter studio S0 — the selection inspector

## Goal and scope

One inspector, keyed by the single current selection, replaces the Build
home's Arrange, Walls, Doors and Notes sidebar tabs and the toolbar's Size
button. Clicking a thing in Layout or 3D opens the inspector on it; with
nothing selected the inspector answers for the room. The selection is one
owner shared by the map and the Encounter home, and a resize grows the
workspace up and to the right by default, or on any side the author
chooses, without moving a cell.

**Repositories.** S0 is web-only: rpg-dnd5e-web, branch off `dev`.

| repo | touched |
|---|---|
| rpg-dnd5e-web | yes, every task |
| rpg-toolkit | no. `room.workspace` is opaque presentation to the dialect except `hexRadius`, which T6 keeps derived (see baseline) |
| rpg-api | no. It never reads the workspace or the studio's scene |
| rpg-api-protos | no |

**Non-goals** (each is a later slice or a design deferral):

- The cast, per-placement looks, the shuffle and prop variants (S1, #575). Per-prop surface material waits for the look catalog contract (rpg-game-assets#226, rpg-dnd5e-web#1268).
- Read-back prose, the fact index, rows heaviest first (S2, #576).
- Layout glyphs for placed nouns, region colours, the room list (S3, #577).
- Door state, concealments, exits, endings (S4, #578). The door and hex views in S0 leave those fields out.
- Words, wrapping, panel width (S5, #579).
- Multi-select and apply-to-all (R9). The existing scene multi-pick that feeds Group selection stays exactly as it is (Q7).
- Any change to the Encounter home's forms or its section list.
- The Regions home. It stays untouched in S0, and S3 retires it (Q5).

## Requirement labels

| label | requirement | source |
|---|---|---|
| I1 | The Build home has no Arrange, Walls, Doors or Notes tabs and no Size toolbar button. Every field they held is reached through the inspector or an armed tool. | R1 |
| I2 | The inspector is keyed by one selection, one of nine kinds: nothing, wall, door, creature, prop, group, hex, label or party start. Each kind has one view. Labels and party start are placed things, so they get views. | R1, design Shape table, resolutions Q1 and Q7 |
| I3 | Clicking a thing selects it and opens the inspector even when it is hidden. N toggles the inspector. N keeps its current input-focus and modifier exclusions. | R1 |
| I4 | The inspector always shows one thing. The scene multi-pick that feeds Group selection is a pick, not a selection, and works exactly as today. | R9 as read in Q7 |
| I5 | A selection made on the Encounter home is the selection on the map, and the reverse. Switching homes keeps it. | R11 (cast half in S1) |
| I6 | A resize adds columns on the right and rows on top by default. Every cell, wall, prop, label and creature keeps its coordinate. | R13 |
| I7 | The author may choose the side a resize grows or shrinks: left or right, and top or bottom. No side moves a cell, so there is no shift warning. | R13 as read in Q6 |
| I8 | An inspector edit that the owner refuses shows the refusal at the form and changes nothing. No save writes a file the strict decoder refuses. | design Law, the document's refusals |
| K1 | Undo and redo, from the toolbar and Ctrl+Z, Ctrl+Shift+Z and Ctrl+Y, keep working. Undo keeps the selection while its noun exists, and falls back to nothing when it does not. | must keep working |
| K2 | 3D drag placement of props from the palette, move, rotate, repeat, stacking and creature placement keep working. A dropped prop becomes the selection. | must keep working, epic #545 |
| K3 | The Encounter home keeps its sections: Creatures, Stat blocks, Tables, Factions, Dispositions, Intel. Their forms are unchanged. | must keep working |
| K4 | Wall drawing, door placement, note creation, and label rename, move and delete keep working with the same refusals. | must keep working |

## Constraints

- **The owner stays the owner.** `WorldBuildingConcept` holds the document,
  history and the selection arbiter `activeStudioTargetRef`. The inspector
  is presentation over `EncounterStudioSession`. It holds no document copy
  and no selection store (`studioSession.ts` header comments).
- **One transaction per form.** Inspector edits cross `commitArrange` and
  the existing binding intents unchanged (`StudioArrangeIntent` contract in
  `studioArrange.ts`).
- **Refusals stay at the boundary.** `validateRoomDocument` and
  `encodeSingleRoomDungeon` keep their gates. The inspector shows a refusal
  at the form (`#arrange-error` today) and never writes around it.
- **The overview refusal applies to the hex view.** The region a hex view
  names is the drawn annotation at that cell. It is never membership for a
  rule (README refusals).
- **The screen does not say single room (R10).** No new copy uses the phrase.
- Web conventions: `rpg-dnd5e-web/CLAUDE.md`. `npm run ci-check` once before
  the PR; never `--no-verify`; PR to `dev`.
- Attribution: the builder's loaded charter signature only.

## Inspected baseline (2026-10-14)

| repo / module | revision |
|---|---|
| rpg-dnd5e-web `origin/dev` | 1ec90316 |
| rpg-toolkit `origin/main`, `rulebooks/dnd5e/encounter` | v0.129.0 plus one session commit (2f570aa9) |
| rpg-project `origin/main` | 2bf183d (design merged) |

Findings that shape the tasks, with paths at the web revision:

- **The studio files** live in `src/concepts/encounter-studio/`, and the
  owner in `src/concepts/world-building/WorldBuildingConcept.tsx`. Its studio
  early return is `if (roomMode && studioPresentation)`, which builds the
  whole `EncounterStudioSession`.
- **A selection arbiter already exists.** `activeStudioTargetRef` holds one
  `StudioArrangeTarget` (`studioArrange.ts`): `scene` (a list of ids),
  `wall`, `label`, `actor`, `start` or `door`. `activateStudioTarget` bumps
  `selectionRevision`. `projectStudioArrange` derives `session.arrange` and
  masks a target whose noun is gone. A null target is the room.
- **The inspector's seed is `SelectedArrange`** in `StudioArrangePanel.tsx`.
  It renders `arrangeFields` from `StudioArrangeFields.tsx`, wall finishes
  and constraints, `session.propControls.arrangeExtras`, and
  `session.socialControls.selection`. For an actor that last one is
  `CreatureOrders` plus the authored-dispositions list. For a single declared
  prop it is the declaration editor plus `PropOrders`.
- **The tabs.** `StudioSidebar.tsx` defaults its sections to Arrange, Walls,
  Doors and Notes. `EncounterStudioWorkspace.tsx` `StudioSurface` holds
  per-home sections, a `visited` set, `sidebarOpen`, and the Size toolbar
  button that shows `StudioDimensions`. It hides the Arrange panel unless
  `section === 'arrange'`, so selecting a thing does not open it. The test
  "collapse and preview/thumbnail rerenders preserve canvas, tool and
  selection; selection does not reopen options" in
  `EncounterStudioWorkspace.test.tsx` pins that behaviour. R1 reverses it.
- **Clicks today.** Layout `select` on bare floor clears wall, label and door
  (`LayoutViewport.tsx` `pointerDown`). 3D bare ground calls `onSelect([])`
  unless Shift is held (`WorldBuildingViewport.tsx`). 3D prop click with
  Shift appends to the selection, and the scene tree also multi-selects with
  Shift, Ctrl or Meta. There is no hex selection.
- **Door state and concealments** (`DoorStates`, `ConcealmentPanel`) mount
  only on the non-studio path, which is S4's work.
- **Party start** has no studio control. `armStartPlacement` exists in the
  owner and is not exposed on the session.
- **Workspace.** `centered-odd-r` in `workspaceGeometry.ts` is centred on
  the origin. `resizeRoomWorkspace` in `roomDraft.ts` replaces it with
  `centeredRoomWorkspace(w, h)`, so a resize grows or shrinks every side
  evenly. The kind is checked by name in `LayoutViewport.tsx`,
  `StudioDimensions.tsx`, `WorldBuildingInteraction.tsx`,
  `WorldBuildingViewport.tsx` (three sites), `groundBoundaryGeometry.ts`,
  `roomDraft.ts` (two sites), `structuralWalls.ts`,
  `usePresentationWorkspace.ts`, `workspaceContentBounds.ts`,
  `workspaceFloorGeometry.ts`, `workspaceGeometry.ts` and the saved-shape
  section of `world-building/CONTRACT.md`.
- **The toolkit reads only `room.workspace.hexRadius`.** The dialect decodes
  `room.workspace` as an opaque node (`single_room_decode.go`) and checks
  each walkable cell's cube distance from the origin against `hexRadius`
  (`walkableValues`). It has no upper bound. An off-centre rectangle stays
  legal when `hexRadius` is derived from its cells, as `centeredRoomWorkspace`
  already does.
- **`play` is fixed.** The encoder writes `PLAY_CONTRACT` (`void:
  transparent`, `lighting: bright`, `standing: centre-covered`), and the
  toolkit refuses any other value (`playValues`). See Q3.
- **The room-default wall finish is per region.** rpg-dnd5e-web#1265 (merged
  to `dev`, in the baseline) stores it as `scene.authoringRegions[].wallFinish`,
  shaped `{ materialRef, patternStart }` (`RoomWallFinish` in
  `regionFinishTypes.ts`). Per-wall overrides are `scene.wallFinishes`. The
  draft has no single document-wide wall default and no floor default
  (`types.ts` `WorldScene`). See Q4.
- **Grouping needs a multi-pick.** Group selection (`groupUngroupActions` in
  the owner, `groupSelection` in `sceneState.ts`) groups the current
  `selectedIds` and refuses fewer than two. The author picks them with Shift
  in 3D or Shift, Ctrl or Meta in the scene tree (`selectTreeRow`). A group
  is one scene root; Ungroup needs exactly one selected group. See Q7.
- **Layout's screen-up is decreasing `r`.** `layoutGeometry.ts` maps world
  `z` to screen `y` with the same sign, and `z` grows with `r`.

## Sequence

```
T1 selection contract ──▶ T2 selection gestures ──▶ T3 inspector shell ──▶ T4 inspector views ──▶ T5 shared selection ──▶ T7 integration
T6 workspace growth (independent; its form lands in T4's room view, so T4 consumes T6 if T6 is first, else T6 adds the field) ──┘
```

One web branch and one PR to `dev`. Tasks are commits on that branch, in
order. T1–T5 share `WorldBuildingConcept.tsx` and
`EncounterStudioWorkspace.tsx`, so they run in sequence by one builder and
never in parallel. T6 touches the workspace files and `StudioDimensions.tsx`
only, and may be built in parallel on its own worktree and merged into the
branch before T4's room view. No provider release, pin or proto is involved,
so there is no merge order beyond the one PR.

## Open items

| id | item | state | blocks |
|---|---|---|---|
| Q1 | **Map labels and party start** are placed things, so they get inspector views. | **resolved by the team lead:** kinds `label` (text and cell) and `partyStart` (cell and the move gesture) | nothing |
| Q2 | **Reaching nothing.** | **resolved:** Escape and a click off the floor both select nothing. A floor-cell click selects the hex in Layout and 3D. | nothing |
| Q3 | **The play contract.** Void, lighting and standing accept one value each in the dialect. **Editing them is a toolkit change** and is not in S0. | **resolved for S0:** the hex view shows each as a read-only value, the one the dialect accepts | nothing in S0 |
| Q4 | **Default finishes.** S0 invents no document key. | **resolved:** the room view lists each region's room-default wall finish from `scene.authoringRegions[].wallFinish`, read-only with a link to that region. It shows nothing for a floor default, because none exists. | nothing |
| Q5 | **The Regions home.** | **resolved:** untouched in S0. S3 (#577) retires it: region and lighting move to the hex view, and the room list moves to the Layout overview. | nothing in S0 |
| Q6 | **Growing another side.** With explicit bounds no cell moves on any side, which serves R13's reason better than the ruled warning. | **R13's warning is dropped; the design row is amended (#581).** T6 plans default growth up and right, any side choosable, and no warning. | nothing |
| Q7 | **R9 and grouping.** Grouping needs a multi-pick (baseline). | **R9 is read as a single inspected thing; the design row is amended (#581).** The multi-pick stays exactly as today and shows as a pick strip; a selected group is one thing with a `group` view. | nothing |
| Q8 | **A cast member selected on the home.** | **resolved, for S1:** it highlights its placements, and the inspector shows the cast member. It is not a multi-selection. See Later slices. | nothing in S0 |
| A1 | Shrinking moves the chosen edges inward, and is refused, as today, when content falls outside. | assumption | T6 |
| A2 | Wall, door and note tools are toolbar tools in Layout. While one is armed, its options (wall appearance and snaps, door assembly, note text) show at the top of the inspector, and disarming removes them. Tool options are transient state, not document configuration. | assumption | T3 |
---

## Task T1: The selection contract

**Delivers:** I2, I4 (type level), the hex target the later tasks consume.
**Owner:** rpg-dnd5e-web, `src/concepts/world-building/studioArrange.ts` and `src/concepts/encounter-studio/studioSession.ts`.
**Prerequisites:** none.
**Files:**
- `world-building/studioArrange.ts`: `StudioArrangeTarget` gains `{ kind: 'hex'; cell: RoomHexCell }`; `StudioArrangeSelection` gains the hex projection; `projectStudioArrange` projects it.
- `world-building/WorldBuildingConcept.tsx`: `studioTargetKey` keys `hex` by `{q, r}`.
- New `world-building/regionAtCell.ts`: `regionsAtCell`.
- `encounter-studio/studioSession.ts`: exports `StudioSelection`, `StudioInspection`, `studioInspectionOf`, and re-exports the new names.
- Tests: `world-building/studioArrange.test.ts`, new `world-building/regionAtCell.test.ts`, new `encounter-studio/studioSelection.test.ts`.

**Interfaces:**

```ts
// studioArrange.ts — the projection of a hex target
| {
    readonly kind: 'hex';
    readonly target: { readonly kind: 'hex'; readonly cell: RoomHexCell };
    readonly cell: Readonly<RoomHexCell>;
    readonly walkable: boolean;            // cell is in room.walkableHexes
    readonly regions: readonly Readonly<AuthoringRegion>[]; // drawn at this cell, authored order
  }

// regionAtCell.ts — presentation only, never membership for a rule
export function regionsAtCell(input: {
  draft: Readonly<RoomDraft>;
  resolutions: readonly RegionResolution[];
  cell: RoomHexCell;
}): AuthoringRegion[];

// studioSession.ts — the inspector's key, derived, never stored. Nine kinds.
export type StudioSelection =
  | { readonly kind: 'nothing' }
  | { readonly kind: 'wall'; readonly id: string }
  | { readonly kind: 'door'; readonly target: StudioDoorTarget }
  | { readonly kind: 'creature'; readonly id: string }   // monsterDeclarations[].id
  | { readonly kind: 'prop'; readonly id: string }       // one scene root that is an item
  | { readonly kind: 'group'; readonly id: string }      // one scene root that is a group
  | { readonly kind: 'hex'; readonly cell: RoomHexCell }
  | { readonly kind: 'label'; readonly id: string }      // a map label, with or without a region
  | { readonly kind: 'partyStart' };

// The multi-pick that feeds Group selection is a pick, not a selection (Q7).
export interface StudioInspection {
  readonly selection: StudioSelection;          // 'nothing' while a pick is active
  readonly pick: readonly string[] | null;      // scene root ids when two or more are picked
}

export function studioInspectionOf(
  arrange: StudioArrangeSelection | null,
  scene: Readonly<WorldScene>
): StudioInspection;
```

**Behavior:**
- `projectStudioArrange` returns null for a hex target whose cell is outside the workspace (`isCellWithinWorkspace`), so the arbiter masks it, as it masks an absent noun.
- A hex projection with an empty `walkableHexes` membership still projects; `walkable: false`.
- `regionsAtCell` includes a region when its resolution is `resolved` and the hex centre is inside its polygon ring (`pointInRing`, `regionBoundaryPredicates.ts`), or the cell is in its `hex-union`. Unresolved regions are excluded. Overlaps return every match.
- `studioInspectionOf` maps null to `nothing`; `actor` to `creature`; `start` to `partyStart`; `scene` with one root to `prop` when the root is in `scene.items` and to `group` when it is in `scene.groups`; `label`, `wall`, `door` and `hex` to themselves. Every mapping has `pick: null`.
- A `scene` target with two or more roots maps to `{ selection: nothing, pick: rootIds }`. The inspector then shows only the pick strip (T4), never the room view under it.

**Tests:**
- `studioArrange.test.ts` › `projects a hex inside the workspace with walkable and regions`: a 6×6 draft, cell `{q:1, r:1}` walkable and inside an explicit region `hall`; expect `kind 'hex'`, `walkable true`, `regions[0].id === 'hall'`.
- › `masks a hex outside the workspace`: cell `{q:40, r:0}`; expect `null`.
- › `projects a non-walkable hex`: expect `walkable false`.
- `regionAtCell.test.ts` › `polygon region contains its interior hex`, `hex-union region contains listed cells only`, `unresolved region is never reported`, `overlapping regions are both reported in authored order`.
- `studioSelection.test.ts` › one case per mapping above, including `a group root maps to group`, `an item root maps to prop`, `start maps to partyStart`, and `two scene roots map to a pick with nothing selected`.

- [ ] Write the tests; run them and confirm they fail on the missing kinds.
- [ ] Implement the target, projection, key, helper and mapping.
- [ ] Run the focused tests and `npm run typecheck`.

**Verification:** from the web worktree, `npx vitest run src/concepts/world-building/studioArrange.test.ts src/concepts/world-building/regionAtCell.test.ts src/concepts/encounter-studio/studioSelection.test.ts` passes; `npm run typecheck` exits 0.
**Completion evidence:** the passing test names and the typecheck result in the commit message body.

## Task T2: Selection gestures

**Delivers:** I3 (the click half), I4, the hex selection gesture, and the two ways to select nothing (Q2).
**Owner:** rpg-dnd5e-web; `LayoutViewport.tsx`, `WorldBuildingViewport.tsx`, `WorldBuildingConcept.tsx`.
**Prerequisites:** T1.
**Files:**
- `encounter-studio/studioSession.ts`: `EncounterStudioSession` gains `selectHex(cell: RoomHexCell | null): boolean` and `clearSelection(): boolean`. `LayoutViewportProps` gains `onSelectHex?(cell: RoomHexCell): void`.
- `encounter-studio/LayoutViewport.tsx` `pointerDown`: in `select`, a left click on a workspace cell that hits no wall, door, label or endpoint calls `onSelectHex(cell)` (cell from `workspaceCellAtPoint`) after clearing the others, as the bare-floor branch does today. A click outside the workspace clears every selection.
- `world-building/WorldBuildingViewport.tsx`: the ground `onPointerDown` in `select` with a workspace cell under the pointer calls a new `roomAuthoring.onSelectHex?(cell)` instead of `onSelect([])`. With no workspace cell under the pointer it calls `onSelect([])`, which selects nothing. The Shift exception stays, because Shift extends the pick (Q7).
- `world-building/WorldBuildingConcept.tsx`: implements `selectHex` and `clearSelection` through `activateStudioTarget`, clearing `selectedIds`, `selectedWallId`, `selectedLabelId`, `selectedActorId` and the door preview as the existing select functions do. The `Escape` branch of `handleKey` calls `clearSelection` in studio mode. Wraps both with `guardSnapshotIntent`.
- Tests: `LayoutViewport.test.tsx`, `WorldBuildingViewport.test.tsx`, `WorldBuildingConcept.test.tsx`.

**Interfaces:** `selectHex(cell)` returns false and changes nothing when the cell is outside the workspace, while publishing is busy, or from a retired snapshot. `clearSelection()` returns true and sets the target to null.

**Behavior:**
- A hex selection and any other selection replace each other; there is never more than one target.
- Escape and a click off the floor both select nothing, in Layout and in 3D.
- Selecting a hex never paints, erases or edits the floor. The floor tools are unchanged.
- Escape inside an input, a select or a dialog keeps its local meaning. The window handler already returns early for those targets, and that guard stays.
- The scene multi-pick through Shift in 3D and Shift, Ctrl or Meta in the scene tree is unchanged. It feeds Group selection exactly as today (Q7).

**Tests:**
- `LayoutViewport.test.tsx` › `select click on bare floor selects that hex`: `onSelectHex` called once with the cell under the pointer; `wallEditing.select(null)` and `labelEditing.onSelect(null)` called.
- › `select click on a wall selects the wall, not a hex`: `onSelectHex` not called.
- › `click outside the workspace selects nothing`: `onSelectHex` not called; the three clears called.
- › `paint tool on bare floor never selects a hex`: `onCommit` path unchanged; `onSelectHex` not called.
- `WorldBuildingViewport.test.tsx` › `ground click in select mode selects the hex under the pointer`; › `click off the workspace floor selects nothing`; › `shift ground click keeps the current pick`.
- `WorldBuildingConcept.test.tsx` › `Escape clears a studio wall selection to nothing`; › `selectHex refuses a cell outside the workspace without a history entry`.

- [ ] Write the tests and confirm they fail.
- [ ] Implement the props, intents and handlers.
- [ ] Run the three focused files.

**Verification:** `npx vitest run src/concepts/encounter-studio/LayoutViewport.test.tsx src/concepts/world-building/WorldBuildingViewport.test.tsx src/concepts/world-building/WorldBuildingConcept.test.tsx` passes.
**Completion evidence:** named tests passing.

## Task T3: The inspector shell

**Delivers:** I1 (the tabs retire), I3 (opens on select, N toggles), A2.
**Owner:** rpg-dnd5e-web; `encounter-studio/EncounterStudioWorkspace.tsx`, `StudioSidebar.tsx`, new `StudioInspector.tsx`.
**Prerequisites:** T1, T2.
**Files:**
- New `encounter-studio/StudioInspector.tsx`: `StudioInspector`, the Build home's only sidebar content.
- `encounter-studio/StudioSidebar.tsx`: `StudioSidebarSection` loses `arrange`, `walls`, `doors`, `labels` and `size`. The default `sections` list is removed. A sidebar with one section renders no section nav. The `N` handler is unchanged.
- `encounter-studio/EncounterStudioWorkspace.tsx` `StudioSurface`: on Build, the sidebar renders `StudioInspector`. The Size toolbar button is removed. The Layout tool group becomes Select, Paint, Erase, Rectangle, Wall, Door and Note. `sections`, `visited`, `showSection('arrange' | 'walls' | 'doors' | 'labels' | 'size')`, `defaultSection` for Build and `arrangeVisible` are removed. Regions keeps its `regions` and `region` sections (Q5).
- `encounter-studio/studioTestNavigation.ts` `studioButton`: Wall, Door and Label return the toolbar tool buttons; Arrange opens the inspector.
- Tests: new `encounter-studio/StudioInspector.test.tsx`; `EncounterStudioWorkspace.test.tsx`.

**Interfaces:**

```ts
export function StudioInspector(props: {
  session: EncounterStudioSession;
  inspection: StudioInspection;         // studioInspectionOf(session.arrange, scene)
  view: EncounterStudioView;
  layoutTool: LayoutTool;
  armedTool: ReactNode | null;          // StudioWallControls | StudioDoorControls | labels.controls, per A2
  onConfigure(section: StudioSocialSection): void;
  onMoveCreature(): void;
  onFrameChange(frame: LayoutFrame): void;
  onArmTool(tool: 'wall' | 'door' | 'label'): void;
}): React.JSX.Element;
```

The inspector's root is `<section aria-label="Inspector">` with an `h2` naming the selection: "Room" for nothing, "N props picked" for a pick, otherwise the name `selectionName` gives today. T4 fills the per-kind body; T3 renders the existing `SelectedArrange` body for every kind except nothing and hex, so the slice stays working between commits.

**Behavior:**
- `StudioSurface` opens the sidebar whenever `session.arrange?.selectionKey` changes to a different non-null key. A key that stays the same, such as a preview rerender, never reopens a sidebar the author hid.
- N and the "Options (N)" toolbar button toggle the inspector. The button's label becomes "Inspector (N)", with `aria-controls="studio-sidebar"` kept.
- Arming Wall, Door or Note shows that tool's options at the top of the inspector, above the selection body. Exiting the tool by Escape, its exit button or another tool removes them. The armed wall appearance and snap settings persist across arming, as they do today.
- Switching to 3D disarms Door and Note as `switchView` does today.

**Tests:**
- `EncounterStudioWorkspace.test.tsx`: replace "selection does not reopen options" with `selecting a different thing reopens a hidden inspector`. Hide with N, set `observed.session.arrange` to a wall projection, rerender, and expect `aria-expanded="true"` and the wall's heading.
- › `a preview rerender of the same selection does not reopen a hidden inspector`.
- › `Build has no Arrange, Walls, Doors or Notes buttons and no Size button`: `queryByRole('button', { name })` is null for each.
- › `Wall, Door and Note are Layout tools whose options show in the inspector while armed`.
- › `N toggles the inspector and ignores text fields, selects, contenteditable, modifiers, repeats and composition`: the existing N test, retargeted.
- `StudioInspector.test.tsx` › `nothing selected renders the Room heading`; › `a wall selection renders the wall name heading`.
- Update every test that clicks Arrange, Walls, Doors, Notes or Size (grep the names in `EncounterStudioWorkspace.test.tsx`, `StudioControls.test.tsx`, `StudioArrangePanel.test.tsx`, `EncounterStudioIntegration.test.tsx` and `WorldBuildingConcept.test.tsx`) to go through `studioButton` or the inspector. Each keeps the assertion it makes about the document.

- [ ] Write the new tests and the inverted one; confirm they fail.
- [ ] Implement the shell and retire the sections.
- [ ] Run the encounter-studio directory's tests.

**Verification:** `npx vitest run src/concepts/encounter-studio` passes; `git grep -nE "'(arrange|walls|doors|labels|size)'" src/concepts/encounter-studio/StudioSidebar.tsx` prints nothing.
**Completion evidence:** test run output; the grep result.

## Task T4: The inspector views

**Delivers:** I2 (one view per kind), I4 (the pick strip), I8, K4; the nine views and the pick strip.
**Owner:** rpg-dnd5e-web; `StudioInspector.tsx`, `StudioArrangePanel.tsx`, `StudioArrangeFields.tsx`, `WorldBuildingConcept.tsx`.
**Prerequisites:** T3. T6 for the room view's Size field, or T6 adds that field itself.
**Files:**
- `encounter-studio/StudioInspector.tsx`: one view component per kind: `RoomView`, `WallView`, `DoorView`, `CreatureView`, `PropView`, `GroupView`, `HexView`, `LabelView` and `PartyStartView`, plus `PickStrip` for a multi-pick.
- `encounter-studio/StudioArrangePanel.tsx`: `SelectedArrange` splits into the per-kind field forms the views reuse. The form keeps `aria-label="Arrange selected noun"` and the one-transaction submit. The empty-selection branch and the "Arrange" heading are removed.
- `encounter-studio/StudioDimensions.tsx`: rendered in `RoomView`; `onDismiss` becomes optional and the form stays open after Apply.
- `encounter-studio/studioSession.ts` and `WorldBuildingConcept.tsx`: the session gains `armPartyStart(): boolean`, which wraps the owner's `armStartPlacement` with `guardSnapshotIntent` and `refuseWhilePublishing`. Place party start in the room view and Move in the party-start view both call it. The session also exposes `propControls.groupActions` (today's `groupUngroupActions`) separately from `arrangeExtras`, so the group view and the pick strip can each mount the half they need.
- Tests: `StudioInspector.test.tsx`, `StudioArrangePanel.test.tsx`, `EncounterStudioIntegration.test.tsx`.

**Interfaces:** each view takes `{ session, selection }` narrowed to its kind. Field edits use the existing intents only:

| view | fields, in order | intents |
|---|---|---|
| Room (nothing) | Size, with the side to grow or shrink (T6); Notes (the existing `useStudioLabels` controls: create a note, and the list of notes, each of which selects its label); Party start (cell or "not placed", and Place party start); Room-default wall finishes: one read-only row per region that has `scene.authoringRegions[].wallFinish`, naming the region and its material, with "Edit region", which opens the Regions home on that region's label. There is **no floor-default field**, because the document has no floor default, and S0 adds no key (Q4). | `resizeWorkspace`, the label create intent, `armPartyStart`, `mapLabelSelection.select` |
| Wall | finish (`StudioWallFinishes`); appearance (`StudioWallAppearanceChoices`); start, end, length, fixed endpoint, height and thickness (`arrangeFields`); constraints (`StudioWallConstraints`); Place a door; Remove wall | `commitArrange` `wall-edit` and `wall-remove`; Place a door calls `doorEditing.setActive(true)` and arms the Door tool |
| Door | position, width, model; Remove door | `commitArrange` `door-edit` and `door-remove` |
| Creature | map name; look (family, style, skin palette); cell and facing; Move; then `socialControls.selection` (`CreatureOrders`: faction, table, own rows, temper, weapons, says, holds, arrives) and the authored-dispositions list; then links to the Encounter home definitions | `commitArrange` `actor-start`; the binding intents behind `CreatureOrders` |
| Prop | position, rotation, height; `propControls.arrangeExtras` (duplicate, delete, light); the declaration editor and `PropOrders` from `socialControls.selection` | `commitArrange` `scene-edit`; the prop binding intents |
| Group | position, rotation, height (today's fields for one scene root); its members, listed by label, each of which selects that member; Ungroup; duplicate and delete | `commitArrange` `scene-edit`; `propControls.groupActions` |
| Hex | cell coordinates; walkable or not; the drawn regions at the cell with their lighting, each with "Edit region", which opens the Regions home with that region's label selected; Play: void, lighting and standing, read-only, each the one value the dialect accepts (Q3) | read-only in S0; `mapLabelSelection.select` |
| Label | text; cell (world X and Z); Delete; for a region's label, "Edit region", which opens the Regions home | `commitArrange` `label-edit` and `label-remove`, as today |
| Party start | its cell; Move, which arms the party-start gesture, and the next floor click places it | `armPartyStart`; `commitArrange` `start-position`, as today |
| Pick strip (two or more scene roots picked) | "N props picked"; today's multi-root position, relative rotation and height fields; Group selection; duplicate and delete. No room view shows under it. | `commitArrange` `scene-edit`; `propControls.groupActions` |

**Behavior:**
- A view shows only its own selection's fields. A faction's table, a table's rows and intel records are never edited from a creature or prop view; the links open the Encounter home (README refusal).
- A refused submit leaves the form's staged values and shows the refusal in `role="alert"` inside the view, as `#arrange-error` does today.
- A hex with no drawn region says "No region is drawn here". An unwalkable hex says so.
- The hex view states play's values as the dialect's fixed contract: "Void: transparent · Lighting: bright · Standing: centre-covered". Editing them is a toolkit change (Q3).
- A selected group is one thing. Picking a second root with Shift turns the inspector into the pick strip; Group selection then makes one group and selects it.

**Tests:**
- `StudioInspector.test.tsx`, one per view: `room view shows size, notes, party start and each region's room-default wall finish, and no floor default`; `group view lists its members and ungroups through one intent`; `label view shows text and cell and renames through one intent`; `party start view shows its cell and Move arms the gesture`; `two picked roots show only the pick strip, and Group selection selects the new group`; `hex view shows the fixed play values read-only`; `wall view shows finish, endpoints, height and Place a door, and Place a door arms the door tool`; `door view shows position and width and removes the door through one intent`; `creature view shows look, facing and the creature orders form`; `prop view shows position, the arrange extras and prop orders for a declared prop`; `hex view names the region and lighting and opens that region`; `hex view with no region says none is drawn`.
- › `a refused wall edit keeps the staged height and shows the refusal`: a `commitArrange` stub returns false; expect the alert and the input value kept.
- › `a creature view never renders the Factions form`: query the factions form by name; expect null.
- `StudioArrangePanel.test.tsx`: existing field tests stay green, reached through the views.
- `EncounterStudioIntegration.test.tsx` › `wall height edited in the inspector is one history entry and undo restores it with the wall still selected` (K1); › `undo of a creature placement falls back to the room view` (K1); › `a prop dropped in 3D becomes the selection and opens the prop view` (K2).

- [ ] Write the view tests; confirm they fail.
- [ ] Split `SelectedArrange` into the per-kind forms; build each view.
- [ ] Add `armPartyStart`.
- [ ] Run `npx vitest run src/concepts/encounter-studio src/concepts/world-building/WorldBuildingConcept.test.tsx`.

**Verification:** the run passes; `npm run typecheck` exits 0.
**Completion evidence:** named tests passing.

## Task T5: One selection across the homes

**Delivers:** I5 (R11 in S0), K3.
**Owner:** rpg-dnd5e-web; `EncounterStudioWorkspace.tsx`, `StudioCreatures.tsx`.
**Prerequisites:** T4.
**Files:** `encounter-studio/EncounterStudioWorkspace.tsx` (`changeHome`, `onShowMap`); `encounter-studio/StudioCreatures.tsx` (the Placed instances list); tests in `EncounterStudioIntegration.test.tsx`.
**Interfaces:** consumes `session.creatures.select(id)` (exists) and `session.arrange` (exists). Produces no new session member.
**Behavior:**
- Selecting a placed instance on the home selects it through `session.creatures.select`, the one arbiter. Show on map switches to Build with the inspector open on that creature, in the view the author last used.
- A creature selected on the map shows as `aria-pressed="true"` in the home's Placed instances list.
- `changeHome` keeps the selection, as its comment states today. Only gestures retire.
- The home's section list and forms are unchanged.

**Tests:**
- `EncounterStudioIntegration.test.tsx` › `a placed instance selected on the home is the map selection with the inspector open`.
- › `a creature selected on the map is pressed in the home's placed list`.
- › `switching homes keeps a wall selection`.
- › `the Encounter home lists Creatures, Stat blocks, Tables, Factions, Dispositions and Intel`.

- [ ] Write the tests; confirm the first two fail.
- [ ] Wire the pressed state and the open-on-show behaviour.
- [ ] Run the integration file.

**Verification:** `npx vitest run src/concepts/encounter-studio/EncounterStudioIntegration.test.tsx` passes.
**Completion evidence:** named tests passing.

## Task T6: The workspace grows up and to the right, or on a chosen side

**Delivers:** I6, I7, A1.
**Owner:** rpg-dnd5e-web; `world-building/workspaceGeometry.ts`, `roomDraft.ts`, the kind's consumers, `StudioDimensions.tsx`.
**Prerequisites:** none. It may run in parallel with T1–T5 on its own worktree.
**Files:**
- `world-building/workspaceGeometry.ts`: `RoomWorkspace` gains the kind `odd-r-rect`; new `rectangleOf`, `anchoredRoomWorkspace` and `growRoomWorkspace`; `validateWorkspace`, `containsWorkspaceCell`, `workspaceCells` and `workspaceBounds` accept it.
- Every consumer listed in the baseline replaces `kind === 'centered-odd-r'` with `rectangleOf(workspace) !== null` and reads `minCol`, `minRow`, `widthHexes` and `heightHexes` from the result.
- `world-building/roomDraft.ts`: `resizeRoomWorkspace` calls `growRoomWorkspace`; the version gate that requires scene version 2 for a centred workspace applies to `odd-r-rect` too.
- `encounter-studio/StudioDimensions.tsx`: reads the rectangle through `rectangleOf`; gains two selects, "Columns change on" (Right, default; Left) and "Rows change on" (Top, default; Bottom); after Apply it frames the workspace centre (`workspaceBounds`), not the origin; the help line says "No cell moves, whichever side changes". There is no shift warning (Q6).
- `encounter-studio/studioSession.ts`: `resizeWorkspace(width, height, edges?: ResizeEdges)`, with `edges` absent meaning right and top.
- `world-building/CONTRACT.md`: the saved-rectangle section documents `odd-r-rect`.
- Tests: `workspaceGeometry.test.ts`, `roomDraft.test.ts`, `singleRoomDungeon.test.ts`, `EncounterStudioWorkspace.test.tsx`.

**Interfaces:**

```ts
| {
    kind: 'odd-r-rect';
    minCol: number;      // odd-r column, col = q + floor(r / 2)
    minRow: number;      // r; screen-up in Layout is decreasing r
    widthHexes: number;  // 1..128, width*height <= 16384, as today
    heightHexes: number;
    hexRadius: number;        // max cube distance from the origin over its cells, as today
    horizontalLimit: number;  // derived, as today
  }

export function rectangleOf(w: RoomWorkspace):
  { minCol: number; minRow: number; widthHexes: number; heightHexes: number } | null;
// centered-odd-r: minCol = -floor(w/2), minRow = -floor(h/2). Legacy radius: null.

export function anchoredRoomWorkspace(rect: {
  minCol: number; minRow: number; widthHexes: number; heightHexes: number;
}): RoomWorkspace; // derives hexRadius and horizontalLimit exactly as centeredRoomWorkspace does

export interface ResizeEdges {
  columns: 'right' | 'left';   // default 'right'
  rows: 'top' | 'bottom';      // default 'top'; top is the smallest r
}
export function growRoomWorkspace(
  current: RoomWorkspace, widthHexes: number, heightHexes: number,
  edges: ResizeEdges = { columns: 'right', rows: 'top' }
): RoomWorkspace;
// dw = widthHexes - old widthHexes; dh = heightHexes - old heightHexes (either may be negative).
// columns 'right': minCol kept.            columns 'left': minCol = old minCol - dw.
// rows 'top':      minRow = old minRow - dh. rows 'bottom': minRow kept.
// Only the chosen edges move; no cell coordinate changes on any side.
// A legacy radius workspace is not a rectangle, so it has no edges to keep: it is
// replaced by centeredRoomWorkspace(widthHexes, heightHexes), exactly as resize does today.
```

**Behavior:**
- Growing keeps every cell's `{q, r}`. Walls, props, labels and regions hold world points or cells and are not rewritten.
- Growing on the left or bottom also keeps every cell's `{q, r}`; there is no warning, because nothing shifts (Q6).
- Shrinking moves the chosen edges inward (A1). `validateWorkspaceContent` still refuses a shrink that leaves content outside, with today's message.
- A `centered-odd-r` document still decodes unchanged. Its first resize saves `odd-r-rect`; an unresized document saves byte-identical.
- `validateWorkspace` refuses an `odd-r-rect` whose `hexRadius` or `horizontalLimit` does not match its derivation, with the sentence the centred kind uses.
- The toolkit's floor check passes because `hexRadius` covers every cell by derivation.

**Tests:**
- `workspaceGeometry.test.ts` › `growing 20x20 to 24x22 keeps every old cell and adds 4 columns right and 2 rows on top`: every old cell is in the new workspace; the new cells are exactly those with `col >= oldMaxCol + 1` or `r < oldMinRow`.
- › `growing on the left and bottom keeps every old cell and moves only those edges`; › `shrinking moves the chosen edges`; › `a centred workspace reads as its rectangle`; › `a mismatched hexRadius on odd-r-rect is refused`; › `hexRadius covers the farthest cell of an off-centre rectangle`.
- `roomDraft.test.ts` › `resize keeps a monster's starting cell, the party start and every walkable cell`; › `a shrink that cuts a wall is refused with no history entry`.
- `singleRoomDungeon.test.ts` › `an off-centre workspace encodes and round-trips through parse`; › `an unresized centred document encodes byte-identical`.
- `EncounterStudioWorkspace.test.tsx`: the staged-dimensions tests updated; › `apply frames the new workspace centre`; › `choosing Left passes columns left to resizeWorkspace and shows no warning`.

- [ ] Write the tests; confirm the growth test fails against today's even resize.
- [ ] Add the kind, helpers and consumers; update `CONTRACT.md`.
- [ ] Run the workspace, room-draft and codec tests.
- [ ] Toolkit cross-check. Write the encoded file of a room grown off-centre to its full 128 by 128, with walkable cells in the far top-right corner, to the job tmp directory. In a scratch, detached toolkit worktree at `origin/main`, add a throwaway `_test.go` in `rulebooks/dnd5e/encounter/dungeonspec` that reads that file and calls `DecodeSingleRoom(SingleRoomDecodeInput{Source: bytes})`. Run it with `go test -run <its name> .` and expect a nil error, with no `outside the workspace floor` refusal. Record the command and output in the PR, then delete the scratch worktree. Nothing is committed to the toolkit. No CLI decoder exists in the toolkit at the baseline.

**Verification:** `npx vitest run src/concepts/world-building/workspaceGeometry.test.ts src/concepts/world-building/roomDraft.test.ts src/concepts/world-building/singleRoomDungeon.test.ts` passes; the toolkit decode reports no refusal.
**Completion evidence:** test output; the toolkit decode command and output.

## Task T7: Integration, retirement check and the walk

**Delivers:** proof of I1–I6, I8 and K1–K4 on the running studio.
**Owner:** rpg-dnd5e-web.
**Prerequisites:** T1–T6 on one branch.
**Files:** none new. Remove anything T3 and T4 left unreferenced: the Build sidebar sections in `StudioSidebar.tsx`, the `visited` set, and the empty-selection branch of `StudioArrangePanel`. Grep each removed export and section name under `src/`; each grep prints nothing.
**Interfaces:** not applicable; integration only.
**Behavior:** not applicable; integration only.
**Tests:** the full suite through `npm run ci-check`.

- [ ] `npm ci` in the fresh worktree, then `npm run ci-check` once; grep its log for `failed` and `error`, never trust the exit code alone.
- [ ] Walk the studio on the local dev server (`npm run dev`, or `scripts/dev-env.sh` from the game-dev root for the full stack). Take one screenshot per walk step with `node tools/browser/screenshot.mjs <url> <out.png>` from the game-dev root, and confirm each one renders.
- [ ] Post the walk checklist as `- [ ]` boxes on #574 before the PR leaves draft.

**Walk** (fresh local draft, Build home):

1. Layout, nothing selected: no Arrange, Walls, Doors, Notes or Size buttons. The inspector reads Room, with size, notes, party start and the room-default wall finishes. It has no floor-default field.
2. Arm Wall from the toolbar, choose an appearance in the inspector, and draw two walls. Press Escape.
3. Press N to hide the inspector. Click a wall: the inspector opens on it. Change its height and press Undo. The old height returns, and the wall stays selected.
4. Press Place a door, then click the wall. Click the new door: the door view shows position and width.
5. Click an empty floor hex: the hex view names the cell, says whether it is walkable, and shows void, lighting and standing read-only. Press Escape: Room. Click a hex, then click outside the floor: Room.
5a. Add a note, then click its label: the label view shows its text and cell. Click the party start marker in 3D: its view shows the cell and Move.
6. Switch to 3D. Drag a prop from the palette onto the floor: the prop view opens. Drag a second prop, Shift-click the first: the pick strip reads 2 props picked. Press Group selection: the group view lists both members. Place a creature from the Monsters and NPCs palette, click it, and see its look, facing and orders.
7. Open the Encounter home, then Creatures. The creature is pressed in Placed instances. Click Show on map: Build, with the creature selected and the inspector open.
8. In the room view, set the size from 20 by 20 to 24 by 22. New columns appear on the right and new rows on top. The walls, props, creature and floor are where they were. Set Columns change on to Left and the width to 26: two columns appear on the left, no warning shows, and nothing moves.
9. Save the local draft and reload. Everything above is still there.

**Verification:** `ci-check` log clean; one screenshot per walk step; the checklist on #574 ticked.
**Completion evidence:** the PR body carries the `ci-check` output, the screenshot list and the T6 toolkit decode output.

---

## Requirement coverage

| requirement / scenario | task(s) | proof |
|---|---|---|
| I1 tabs and Size retire | T3, T7 | `Build has no Arrange, Walls, Doors or Notes buttons and no Size button`; the StudioSidebar grep; walk 1 |
| I2 one view per kind, nine kinds | T1, T4 | `studioSelection.test.ts` mappings; one `StudioInspector.test.tsx` case per view; walk 5a |
| I3 click opens, N toggles | T2, T3 | `selecting a different thing reopens a hidden inspector`; the retargeted N test; walk 3 |
| I4 one inspected thing; the pick stays | T1, T2, T4 | `two scene roots map to a pick with nothing selected`; `two picked roots show only the pick strip…`; `select click on a wall selects the wall, not a hex`; walk 6 |
| I5 one selection across homes | T5 | `a placed instance selected on the home is the map selection with the inspector open`; `a creature selected on the map is pressed in the home's placed list`; walk 7 |
| I6 grows up and right | T6 | `growing 20x20 to 24x22 keeps every old cell…`; `resize keeps a monster's starting cell…`; walk 8 |
| I7 any side, no warning | T6 | `growing on the left and bottom keeps every old cell…`; `choosing Left passes columns left…`; walk 8 |
| I8 refusals at the form | T4, T6 | `a refused wall edit keeps the staged height and shows the refusal`; `a shrink that cuts a wall is refused with no history entry`; T6 toolkit decode |
| K1 undo and redo | T4 | `wall height edited in the inspector is one history entry…`; `undo of a creature placement falls back to the room view`; walk 3 |
| K2 3D drag placement | T4, T7 | `a prop dropped in 3D becomes the selection and opens the prop view`; walk 6 |
| K3 Encounter home untouched | T5 | `the Encounter home lists Creatures, Stat blocks, Tables, Factions, Dispositions and Intel` |
| K4 drawing, doors, notes | T3, T4 | `Wall, Door and Note are Layout tools…`; `wall view … Place a door arms the door tool`; walk 2 and 4 |
| #574 scenario 1 | T3, T4 | walk 1 |
| #574 scenario 2 | T3, T4 | walk 3 and 4 |
| #574 scenario 3 | T2, T4 | walk 5 and 6 |
| #574 scenario 4 | T1, T4 | `hex view names the region and lighting and opens that region`; walk 5 |
| #574 scenario 5 | T4 | walk 3 |
| #574 scenario 6 | T6 | walk 8 |

**Explicit deferrals:** editing the play contract, which is a toolkit
change (Q3); a floor-default finish, which has no document key (Q4); and
the creature view's read-back prose, which is S2.

## Provider/consumer seams

| provider | consumer | produced vs consumed contract | availability | proof |
|---|---|---|---|---|
| T1 `StudioArrangeTarget` `hex` and its projection | T2 owner intents, T4 `HexView` | `{kind:'hex', cell}` with `walkable` and `regions`; null when outside the workspace | new in T1 | `projects a hex inside the workspace…`; `masks a hex outside the workspace` |
| T1 `studioInspectionOf` | T3 `StudioInspector` key, T4 views and pick strip | nine selection kinds plus a pick of two or more roots; `nothing` for null | new in T1 | `studioSelection.test.ts` |
| T1 `regionsAtCell` | T4 `HexView` | resolved regions at the cell, authored order; an annotation | new in T1, over the existing `pointInRing` | `regionAtCell.test.ts` |
| T2 `selectHex`, `clearSelection` on the session | T2 `LayoutViewport` and `WorldBuildingViewport` | boolean; false without a history entry on refusal | new in T2, through the existing `activateStudioTarget` | `selectHex refuses a cell outside the workspace…` |
| T4 `armPartyStart` | T4 `RoomView` and `PartyStartView` | boolean; wraps the existing `armStartPlacement` | the owner function exists; the session member is new | `room view shows … party start`; `party start view shows its cell and Move arms the gesture` |
| T4 `propControls.groupActions` | T4 `GroupView` and `PickStrip` | today's Group selection and Ungroup buttons, split out of `arrangeExtras` | the owner node exists; the session member is new | `group view lists its members and ungroups…`; `two picked roots show only the pick strip…` |
| existing `scene.authoringRegions[].wallFinish` (rpg-dnd5e-web#1265) | T4 `RoomView` | `{ materialRef, patternStart }` per region, read-only | in the baseline | `room view shows … each region's room-default wall finish` |
| existing `commitArrange` and `StudioArrangeIntent` | T4 views | unchanged; one transaction per form | exists at the baseline | existing `StudioArrangePanel.test.tsx` cases |
| existing `socialControls.selection` and `propControls.arrangeExtras` | T4 creature and prop views | unchanged ReactNodes from the owner | exist at the baseline | `creature view shows … creature orders form`; `prop view shows …` |
| existing `session.creatures.select` | T5 | unchanged | exists at the baseline | `a placed instance selected on the home…` |
| T6 `odd-r-rect`, `rectangleOf`, `growRoomWorkspace`, `ResizeEdges` | T4 `RoomView` through `resizeWorkspace(width, height, edges?)`, and every listed consumer | rectangle fields; `hexRadius` derived; absent `edges` means right and top | new in T6 | `growing 20x20…`; `growing on the left and bottom…`; codec round-trip |
| T6 saved `room.workspace` | rpg-toolkit single-room decoder | opaque node; `hexRadius` bounds walkable cells by cube distance | toolkit at the baseline, unchanged | the T6 toolkit decode with no floor refusal |

**Shared files.** T1–T5 edit `WorldBuildingConcept.tsx` and
`EncounterStudioWorkspace.tsx` in sequence on one branch. T6 edits
`StudioDimensions.tsx` and the workspace files. Only `StudioDimensions.tsx`
is shared with T4, and T4 only mounts it. T6 merges into the branch before
T4 starts the room view, or T4 mounts the current component and T6 updates
it in place.

## Plan checks

- **Implementation against assertions.** T3's open-on-select rule keys on
  `selectionKey`, which matches both T3 tests: a new key reopens, a preview
  rerender does not. T6's growth formula moves only the chosen edges,
  which matches both growth tests' new-cell sets.
- **Unowned requirements.** None. Q6 and Q7 follow the R13 and R9 rows as
  amended in #581.
- **Persistence.** T6 changes the saved workspace and is covered by the
  codec round-trip, the byte-identical test and the toolkit decode. T1–T5
  add no saved data; the selection is never persisted.
- **Multiplayer events, generated bindings, pins.** Not applicable. No wire
  is touched.
- **Removal of replaced paths.** T3 retires the sections; T7 greps for the
  leftovers.
- **Decisions left open.** Q1–Q8 are resolved. A1 and A2 are planning
  assumptions.

---

# Encounter studio S1 — the cast and per-placement looks

Planned 2026-10-14 against the S0 branch, which S1 stacks on. Authority:
design.md (R3, R4, R11, R14; R14b deferred), rpg-project#575, and the
team lead's cast recommendations (listed under Constraints; they are not on
the PR #572 record, so S1-Q8 asks where they are recorded).

## S1 goal and scope

The Encounter home's Creatures and Stat blocks sections become one **Cast**.
A cast member is a stat block — a `templates.<id>` entry, or a bare rulebook
monster — plus the studio's defaults for new placements. The palette lists
the cast. Placing stamps the default look and faction onto the placement;
the stat block is followed by id. A look is per placement, with a shuffle.
A cast member selected on the home lights up its placements on the map.

**Repositories.** Web only, stacked on rpg-dnd5e-web `feat/studio-s0-inspector`.
rpg-toolkit, rpg-api and rpg-api-protos are not touched: every edit writes
keys the dialect already declares (`templates`, `monsterDeclarations[].ref`
and `.appearanceRef`, `monsterBindings.<id>.faction`) or the studio's own
`room.scene`.

**Non-goals:**

- Per-prop surface material and provider look choices (R14b; rpg-game-assets#226, rpg-dnd5e-web#1268). No material field.
- A prop shuffle (R14).
- Read-back prose (S2, #576), and any change to Tables, Factions, Dispositions or Intel.
- Multi-select as an editing mode (R9). Selecting a cast member is one thing, not a set of placements.

## S1 requirement labels

| label | requirement | source |
|---|---|---|
| C1 | The cast is derived, never a second list: one member per monster ref used by a template, a studio-defaults entry or a placement. | R3 |
| C2 | A template-backed member's stat block lives only in `templates.<id>`. Editing it reaches every placement by id. | R3 |
| C3 | A bare rulebook monster is a cast member without a template. A legacy placement with no template and no defaults entry appears as its rulebook kind. | lead recommendation |
| C4 | The Stat blocks free-text form becomes the cast member's override fields: base, abilities, hit dice, armor, proficiency, skills, weapons, experience. Editing an override on a bare member first gives it a template. | lead brief |
| C5 | Renaming a cast member's id rewrites every reference: the template key, every placement's ref, and its defaults entry. | lead recommendation |
| C6 | Deleting a cast member that has placements is refused, naming them. Deleting one with none removes its template and defaults. | lead recommendation; absorbs rpg-dnd5e-web#1262's removal item |
| C7 | The palette lists the cast. A drop writes a ref to the stat block, never its fields. | R3, README refusal |
| C8 | Placing stamps the default look onto `appearanceRef` and the default faction onto `monsterBindings.<id>.faction`. A later default change reaches no placed creature. The inspector labels both as stamped. | R3, R4 |
| C9 | A placement's look may be overridden in its inspector. A shuffle gives one cast member's placements different looks from its default family. | R4 |
| C10 | A cast member selected on the home lights up its placements on the map, and the inspector shows the cast member. A selected placement presses its cast member's row. | R11 |
| C11 | A prop's catalog variant is chosen per placement, by swapping its `assetRef` within its family. | R14, rpg-dnd5e-web#523 |
| C12 | A placement whose ref names no template is told so in its inspector, which offers the cast to choose from. | README edge |

## S1 constraints

- **No new document key.** The cast reads `templates` and `scene.creaturePalette`. One field is needed beyond that for the default faction, and S1-Q1 asks whether it may be added; only S1-T3's faction stamp waits on it.
- **The owner stays the owner.** Every cast edit is one `commitRoomDocument` transaction that may touch `scope.templates`, `draft.scene.creaturePalette`, `draft.scene.actorNames` and `draft.room.monsterDeclarations` together, so undo reverses a rename or a placement in one step.
- **The web does not resolve the rulebook.** Whether a ref names a real rulebook monster, and whether a template id shadows one, are the rulebook's questions (`dungeonspec/single_room.go`, `spec.go`). The studio never refuses on them.
- **Refusal at the boundary.** `validateMonsterTemplates` and `validateCreaturePalette` keep their gates; the cast form shows their sentences at the field.
- S0's constraints carry over unchanged.

## S1 inspected baseline (2026-10-14)

| repo | revision |
|---|---|
| rpg-dnd5e-web `feat/studio-s0-inspector` | f0e0ef8e |
| rpg-toolkit `origin/main` | as S0 |
| rpg-project `origin/main` | 1c89e08 (design amendments #573, #581) |

- **Recipes.** `CreaturePaletteEntry { id, name, ref, modelRef }` in `world-building/creaturePalette.ts`, stored at `scene.creaturePalette` (scene version 5). `ref` is any monster ref; `modelRef` must be an appearance family (`NPC_APPEARANCE_FAMILIES`). Nothing stops two entries sharing a `ref`.
- **Stat blocks.** `MonsterTemplate` and `validateMonsterTemplates` in `world-building/monsterTemplates.ts`; stored at `scope.templates`; written by `session.commitTemplates`. `StudioStatBlocks.tsx` is a free-text form: base, armor and weapons are typed refs, skills and weapons comma-separated.
- **Creatures home.** `StudioCreatures.tsx` holds the recipe form, the recipe list, a Placed instances list and a "Change creature definition" form (`session.creatures.setDefinition`, over `setRoomMonsterDefinition`).
- **Palette and placement.** `StudioCreaturePalette.tsx` lists recipes; `PlacementChoice` picks a style and skin palette within the recipe's family before `session.creatures.armPlacement(id, appearanceRef)`. The owner's `placeMonsterAt` writes the declaration (`ref`, `appearanceRef`, `startingCell`) through `placeRoomMonster` and the map name through `setActorName`, and **stamps no faction**.
- **Looks.** `npcAppearanceOptions(ref)` (`npc-appearances/npcAppearanceChoices.ts`) gives a ref's family and its looks; a family ref is itself a valid look (the family's default). The S0 creature view edits family, style and skin palette through `commitArrange` `actor-start` `appearanceRef`.
- **Factions.** `setMonsterFaction` and `withBinding` in `world-building/monsterOrderEdits.ts` are pure.
- **Rulebook lists in the web.** `PALETTE_MONSTERS` and `paletteNameForRef` (`author/paletteData.ts`) list rulebook monsters; skills are `APPROACH_ABILITIES` minus `ABILITIES` (`author/types.ts`). **No armor or weapon ref catalog exists in the web** (S1-Q3).
- **Prop variants.** `resolvePropVariant(key)` (`components/hex-grid/propManifest.ts`) returns the first of a legacy key's variants, or the default exact ref of a generated family (`GENERATED_PROP_FAMILY_DEFAULTS`, one family today). The generated world-asset props (`generated/worldAssetCatalog.ts`, such as `dnd5e:props:fantasy-kingdom:chair_fancy_02`) **declare no family** (S1-Q2).
- **Markers.** `RoomActorMarkers.tsx` takes `selectedActorId` and colours one marker; it has no highlight set.

## S1 sequence

```
S1-T1 cast model ──▶ S1-T2 cast on the home ──▶ S1-T3 palette and stamped placement ──▶ S1-T4 creature inspector ──▶ S1-T5 cast selection ──▶ S1-T8 integration
                                                          └──▶ S1-T6 shuffle ──────────────────────────────────┘
S1-T7 prop variant pick (blocked on S1-Q2) ───────────────────────────────────────────────────────────────────────┘
```

One web branch stacked on `feat/studio-s0-inspector`, one PR to `dev`, opened as a draft until S0 merges; it catches up by merging `dev` in, never by a rebase or force-push. Tasks are commits in order; one builder, no parallel writes.

## S1 open items

| id | item | state | blocks |
|---|---|---|---|
| S1-Q1 | **Where the default faction lives.** The only studio-owned home is a new optional `faction` on `scene.creaturePalette` entries. That is a new field inside the studio's scene, opaque to the toolkit. Is it allowed under "no new document key", or is there another home? | open | the faction stamp in S1-T3 and the default-faction field in S1-T2. The look stamp proceeds. |
| S1-Q2 | **Where a prop's family comes from.** Swapping `assetRef` within a family needs the family's members. The generated world-asset catalog publishes none, the generated prop catalog has one family default and no member list, and a legacy key's extra variants can only be picked with a new item field. Should the provider publish families (rpg-game-assets), or is splitting a ref's name allowed? | open | S1-T7 entirely |
| S1-Q3 | **Armor and weapon pick-lists.** The README asks for pick-lists, but the web has no armor or weapon catalog. S1 keeps typed refs with the grammar check. A list needs a provider: an API listing or a generated catalog. | open | nothing; S1-T2 ships typed refs |
| S1-Q4 | **Two recipes on one ref.** Existing drafts may hold two recipes with the same `ref` (Guard in red, Guard in blue). The cast has one member per ref. **Assumption:** the first entry is the member's defaults; later ones stay in the file, are listed on the member as "Unused defaults", and the author may remove them. Nothing is dropped silently. | assumption | S1-T1 |
| S1-Q5 | **A stat block that resolves to nothing.** The web cannot tell a rulebook monster it does not list from a deleted template. The README edge says Save refuses. **Assumption:** the inspector warns when a ref names no template and no listed rulebook monster, and local save does not refuse. Publish refusal stays with the toolkit. | assumption | S1-T4 |
| S1-Q6 | **What the shuffle varies** (design Open). **Assumption:** looks within the member's default family (style and skin palette), never another family. | assumption | S1-T6 |
| S1-Q7 | **A template id that names a rulebook monster.** Shadowing is the rulebook's question. The studio neither refuses nor warns. Should it warn? | open | nothing |
| S1-Q8 | **The cast recommendations' record.** The bare-member, legacy-kind, rename and delete rules come from the team lead's brief. PR #572 carries only the merge verdict. Where are they recorded? | open | nothing |

---

## Task S1-T1: The cast model

**Delivers:** C1, C3, C5, C6 and the S1-Q4 assumption, as pure functions.
**Owner:** rpg-dnd5e-web, new `src/concepts/world-building/cast.ts`.
**Prerequisites:** none.
**Files:** new `world-building/cast.ts` and `world-building/cast.test.ts`.
**Interfaces:**

```ts
export interface CastMember {
  readonly ref: string;                    // dnd5e:monsters:<id>; the identity
  readonly id: string;                     // the ref's id after the type
  readonly source: 'template' | 'rulebook';// template when templates[id] exists
  readonly template?: Readonly<MonsterTemplate>;
  readonly defaults?: Readonly<CreaturePaletteEntry>; // first entry with this ref
  readonly unusedDefaults: readonly CreaturePaletteEntry[]; // later entries with this ref
  readonly placements: readonly string[];  // monsterDeclarations[].id with this ref, authored order
  readonly name: string;                   // template.name, else defaults.name, else paletteNameForRef(ref)
}
export function castOf(document: Readonly<RoomDraftDocument>): CastMember[];
// Order: templates in key order, then rulebook refs in first-mention order.

export function saveCastMember(document, input: {
  ref: string;                             // existing member, or a new ref
  template?: MonsterTemplate;              // absent: leave the template as it is
  defaults?: Omit<CreaturePaletteEntry, 'id' | 'ref'>; // absent: leave defaults
}, idFactory: IdFactory): RoomDraftDocument;

export function renameCastMember(document, from: string, toId: string): RoomDraftDocument;
// template-backed only; rewrites templates key, every declaration ref, every palette entry ref.

export function adoptCastMember(document, ref: string, templateId: string): RoomDraftDocument;
// a rulebook member gains templates[templateId] = { base: ref }; its placements and defaults move to the new ref.

export function removeCastMember(document, ref: string): RoomDraftDocument;
// throws CastMemberInUse when placements.length > 0.
export class CastMemberInUse extends Error { readonly placements: readonly string[] }
```

**Behavior:**
- `castOf` never writes. A ref used only by placements yields a `rulebook` member with no defaults (C3).
- `renameCastMember` refuses an id that is already a template key or fails the template-id grammar, with `validateMonsterTemplates`'s sentence. It refuses a rulebook member: "A rulebook monster's id is its kind. Give it a stat block to rename it."
- `adoptCastMember` refuses an existing template id. Every placement of the rulebook ref moves to the new ref, so the kind and its placements stay one member.
- `removeCastMember` with placements throws `CastMemberInUse` with the message: "Guard has 2 placements: gate-1, gate-2. Remove them from the map or change their cast member first." With none, it deletes the template and every palette entry with that ref.
- Each function returns a document that `validateRoomDocument` accepts, or throws.

**Tests** (`cast.test.ts`):
- `a template, a recipe on it and two placements are one member`.
- `a placement of a rulebook monster with no recipe is a rulebook member with no defaults`.
- `two recipes on one ref: the first is the defaults and the second is unused` (S1-Q4).
- `rename rewrites the template key, both placements and the recipe`; `rename refuses an existing template id`; `rename refuses a rulebook member`.
- `adopt gives a rulebook member a template and moves its placements`.
- `remove with placements throws CastMemberInUse naming them`; `remove without placements deletes the template and its recipes`.
- `castOf keeps template key order, then rulebook refs by first mention`.

- [ ] Write the tests; confirm they fail.
- [ ] Implement `cast.ts`.
- [ ] Run the focused file and `npm run typecheck`.

**Verification:** `npx vitest run src/concepts/world-building/cast.test.ts` passes.
**Completion evidence:** named tests passing.

## Task S1-T2: The cast on the Encounter home

**Delivers:** C2, C4, C5, C6 (on screen), C1; retires Creatures and Stat blocks.
**Owner:** rpg-dnd5e-web; `encounter-studio/`.
**Prerequisites:** S1-T1.
**Files:**
- New `encounter-studio/StudioCast.tsx`: the cast list and `CastMemberForm`.
- `encounter-studio/EncounterStudioWorkspace.tsx`: `ENCOUNTER_SECTIONS` becomes `cast`, `tables`, `factions`, `dispositions`, `intel`; Cast is the default section; the palette's Configure opens Cast.
- `encounter-studio/studioSession.ts` and `world-building/WorldBuildingConcept.tsx`: the session gains `cast: { save, rename, adopt, remove }`, each a `guardSnapshotIntent` over one `commitRoomDocument`. `commitTemplates` and `creatures.savePaletteEntry` are removed once nothing calls them.
- Removed: `encounter-studio/StudioStatBlocks.tsx` and its test, and the recipe form, recipe list and definition form in `StudioCreatures.tsx`. The file goes once its Placed instances list moves into the cast member's placements.
- Tests: new `encounter-studio/StudioCast.test.tsx`; `EncounterStudioIntegration.test.tsx`.

**Interfaces:** `CastMemberForm({ session, member | null })`. Fields, in order:

| field | writes | widget |
|---|---|---|
| Id | the template key, through `rename` or `adopt` | text; read-only with "Give it a stat block" for a rulebook member |
| Name | `templates.<id>.name`, or the defaults `name` for a rulebook member | text |
| Base | `templates.<id>.base` | pick-from-list: `PALETTE_MONSTERS` plus Human, and the current value if it is unlisted |
| STR … CHA | `.abilities` | number per score; blank is the base's |
| Hit dice, proficiency, experience | `.hitDice`, `.proficiency`, `.experience` | text, number, number |
| Armor, weapons | `.armor`, `.actions` | typed refs checked by `validateMonsterTemplates` (S1-Q3); weapons one per row, in order |
| Skills | `.skills` | pick-from-list of the 18 skill ids |
| Default look | the defaults `modelRef` | pick-from-list of `NPC_APPEARANCE_FAMILIES` with their thumbnails |
| Default faction | the defaults `faction` | pick-from-list of `scope.factions` (**blocked on S1-Q1**) |
| Placements | read-only | list; clicking one selects it on the map (R11) |
| Unused defaults | read-only, with Remove | list (S1-Q4) |
| Delete | `remove` | button; shows `CastMemberInUse`'s sentence in an alert when refused |

**Behavior:**
- An override field edited on a rulebook member opens the Id field and saves through `adopt` first, then `save`, in one transaction.
- Every field change is staged; Apply is one transaction; Cancel discards; a refusal shows at the form and keeps the staged values.
- The member list names each member, its source ("stat block" or "rulebook"), and its placement count.

**Tests:**
- `StudioCast.test.tsx` › `the home lists Cast, Tables, Factions, Dispositions and Intel, and no Creatures or Stat blocks`; › `editing armor on a template member is one transaction that reaches both placements`; › `an override on a rulebook member asks for an id and adopts it`; › `renaming the id rewrites the placements`; › `delete with placements shows the refusal naming them and changes nothing`; › `delete without placements removes the member`; › `skills are picked from the eighteen skill ids`.
- `EncounterStudioIntegration.test.tsx` › `undo after a cast rename restores the old id on every placement`.

- [ ] Write the tests; confirm they fail.
- [ ] Build `StudioCast`, wire the session, retire the two sections and their files.
- [ ] Grep `StudioStatBlocks`, `savePaletteEntry` and `Create creature recipe` under `src/`; each prints nothing.

**Verification:** `npx vitest run src/concepts/encounter-studio` passes.
**Completion evidence:** named tests; the grep output.

## Task S1-T3: The palette lists the cast; placing stamps

**Delivers:** C7, C8.
**Owner:** rpg-dnd5e-web; `StudioCreaturePalette.tsx`, `WorldBuildingConcept.tsx` `placeMonsterAt`.
**Prerequisites:** S1-T2. The faction stamp also needs S1-Q1.
**Files:** `encounter-studio/StudioCreaturePalette.tsx`; `world-building/WorldBuildingConcept.tsx` (`armPlacement`, `placeMonsterAt`, the `armedCreature` state); `encounter-studio/studioSession.ts` (`creatures.armPlacement(ref, appearanceRef)`, keyed by the member's ref instead of the recipe id); tests in `EncounterStudioIntegration.test.tsx` and `WorldBuildingConcept.test.tsx`.
**Interfaces:** `armPlacement(ref: string, appearanceRef: string): boolean` refuses a ref that is not a cast member, and an `appearanceRef` outside the member's default family. A member without a default look is listed but cannot be armed: "Choose a default look in the cast to place it."
**Behavior:**
- One transaction writes the declaration (`ref`, `appearanceRef`, cell), the map name (the member's name), and, when the defaults carry a faction, `monsterBindings.<id>.faction` through `setMonsterFaction` and `withBinding`.
- The declaration never copies template fields (C7).
**Tests:**
- › `the palette lists every cast member, including a rulebook member`.
- › `placing writes the ref, the chosen look and the name in one history entry`.
- › `placing stamps the default faction onto the binding` (after S1-Q1).
- › `changing the default faction afterwards leaves the placed binding unchanged`.
- › `a member without a default look cannot be armed`.

- [ ] Tests first; implement; run the two files.

**Verification:** `npx vitest run src/concepts/encounter-studio/EncounterStudioIntegration.test.tsx src/concepts/world-building/WorldBuildingConcept.test.tsx` passes.
**Completion evidence:** named tests.

## Task S1-T4: The creature inspector names its cast member

**Delivers:** C8 (labels), C9 (override), C12.
**Owner:** rpg-dnd5e-web; `encounter-studio/StudioInspector.tsx` `CreatureView`, `StudioArrangeFields.tsx`.
**Prerequisites:** S1-T3.
**Files:** `StudioInspector.tsx`; `StudioArrangeFields.tsx` (the actor fields); tests in `StudioInspector.test.tsx`.
**Behavior:**
- The view opens with "Cast member: Guard · stat block guard", linked to the member (S1-T5 selects it), and "Changes to Guard reach every placement."
- Look shows "Stamped at placement" when the placement's `appearanceRef` differs from the member's default, and "Cast member's default" when equal. The look fields edit this placement only, through `actor-start` `appearanceRef`, as in S0.
- Faction shows "Stamped at placement. Changing Guard's default faction does not change this."
- A ref that names no template and no listed rulebook monster shows "This placement's stat block, dnd5e:monsters:x, is not in the cast", and a cast pick-list that writes through `creatures.setDefinition` (S1-Q5).
**Tests:** › `the creature view names its cast member and links to it`; › `a look equal to the default reads as the default, a changed one as stamped`; › `changing one placement's look leaves its sibling unchanged`; › `a placement whose stat block is missing offers the cast and reassigns through one intent`.

- [ ] Tests first; implement; run `StudioInspector.test.tsx`.

**Verification:** `npx vitest run src/concepts/encounter-studio/StudioInspector.test.tsx` passes.
**Completion evidence:** named tests.

## Task S1-T5: Selecting a cast member

**Delivers:** C10 (R11, the S1 half).
**Owner:** rpg-dnd5e-web; `studioArrange.ts`, `studioSession.ts`, `StudioInspector.tsx`, `RoomActorMarkers.tsx`, `StudioCast.tsx`.
**Prerequisites:** S1-T2, S1-T4.
**Files:** `world-building/studioArrange.ts` (target `{ kind: 'cast'; ref: string }` and its projection with `placements`); `world-building/WorldBuildingConcept.tsx` (`studioTargetKey`, a `selectCastMember(ref)` intent); `encounter-studio/studioSession.ts` (`StudioSelection` gains `{ kind: 'castMember'; ref }`, mapped in `studioInspectionOf`); `encounter-studio/StudioInspector.tsx` (`CastMemberView` renders `CastMemberForm`); `world-building/RoomActorMarkers.tsx` (a `highlightedActorIds` set drawn in a second colour); tests.
**Interfaces:** `selectCastMember(ref): boolean` refuses a ref that is not in `castOf`. The projection is null when the member is gone.
**Behavior:**
- Selecting a member on the home, or the member link in a creature view, selects the member. The inspector shows the member's form, and its placements are highlighted in 3D. It is one selection, not a set (R9).
- Selecting a placement presses its member's row on the home.
- Clicking a highlighted placement selects that placement.
**Tests:** › `selecting a cast member highlights exactly its placements`; › `the inspector shows the cast member form, not a creature view`; › `a selected placement presses its member's row`; › `deleting the selected member's template falls back to nothing`.

- [ ] Tests first; implement; run the inspector, selection and marker tests.

**Verification:** `npx vitest run src/concepts/encounter-studio src/concepts/world-building/RoomActorMarkers.test.tsx` passes.
**Completion evidence:** named tests.

## Task S1-T6: The shuffle

**Delivers:** C9 (shuffle), the S1-Q6 assumption.
**Owner:** rpg-dnd5e-web; `world-building/cast.ts`, `StudioCast.tsx`.
**Prerequisites:** S1-T2.
**Interfaces:** `shuffleCastLooks(document, ref, random: () => number): RoomDraftDocument`.
**Behavior:** each placement of the member gets a look from its default family's looks. Looks are dealt without repeats until the family runs out, in an order drawn from `random`. A member with one look, or no default look, refuses: "Guard's look family has one look." One transaction; undo restores every look.
**Tests:** › `three placements of a five-look family get three different looks` (seeded random); › `six placements of a five-look family repeat only after all five`; › `a one-look family refuses`; › `undo after a shuffle restores every look`.

- [ ] Tests first; implement; add Shuffle looks to the member form.

**Verification:** `npx vitest run src/concepts/world-building/cast.test.ts src/concepts/encounter-studio/StudioCast.test.tsx` passes.
**Completion evidence:** named tests.

## Task S1-T7: A prop's variant (blocked on S1-Q2)

**Delivers:** C11.
**Owner:** rpg-dnd5e-web; `world-building/catalog.ts`, `studioArrange.ts`, `StudioInspector.tsx` `PropView`.
**Prerequisites:** S1-Q2 answered with a family source.
**Interfaces:** `propVariantChoices(assetRef): readonly { ref; label; thumbnail? }[]`, reading only provider-declared families and empty for a prop without one. `StudioSceneArrangeEdit` gains `assetRef?: string`, an intent field, not a document key, accepted only when the target is one item and the ref is in `propVariantChoices` of its current ref.
**Behavior:** the prop view lists the variants with thumbnails; picking one swaps that item's `assetRef` in one transaction. Its transform, label, declaration and bindings are kept. No shuffle.
**Tests:** › `a prop with a declared family lists its variants`; › `picking a variant swaps only that item's assetRef`; › `a prop with no family shows no variant field`; › `an assetRef outside the family is refused`.

- [ ] Wait for S1-Q2; then tests first; implement.

**Verification:** `npx vitest run src/concepts/world-building/studioArrange.test.ts src/concepts/encounter-studio/StudioInspector.test.tsx` passes.
**Completion evidence:** named tests.

## Task S1-T8: Integration and the walk

**Delivers:** proof of C1–C12 on the running studio.
**Prerequisites:** S1-T1–T6, and S1-T7 if unblocked.
- [ ] `npm ci`; `npm run ci-check` once; grep the log.
- [ ] Walk on the local dev server, one screenshot per step.
- [ ] Post the checklist below on #575 as `- [ ]` boxes before the PR leaves draft.

**Walk and #575 checklist** (fresh draft):

1. Encounter home: Cast, Tables, Factions, Dispositions, Intel; no Creatures or Stat blocks.
2. Add Guard: base Guard, default look chosen, default faction garrison. It is listed as a stat block with no placements.
3. 3D palette lists Guard. Place it twice. Each placement's inspector names Guard, shows the default look and the faction garrison marked stamped.
4. Change Guard's armor on the home. Both placements' inspector summaries show it.
5. Change Guard's default faction to kitchen. Both placements still show garrison.
6. Shuffle looks on Guard. The two placements look different. Change one placement's look: the other is unchanged.
7. Select Guard on the home, then open Build: both placements are highlighted and the inspector shows Guard.
8. Rename Guard's id to gate-guard. Both placements follow. Undo: both return.
9. Delete Guard: refused, naming both placements. Remove one from the map, delete again: still refused, naming the other.
10. Load a draft with a bare goblin placement: Goblin is in the cast as a rulebook member. Give it 12 STR: it asks for an id, adopts, and the goblin follows.
11. (S1-T7 only) Select a barrel, pick another variant: only that barrel changes.
12. Save the local draft and reload: everything above is still there.

**Verification:** `ci-check` clean; screenshots; the checklist on #575 ticked.

## S1 requirement coverage

| requirement | task(s) | proof |
|---|---|---|
| C1 derived cast | S1-T1, S1-T2 | `a template, a recipe on it and two placements are one member`; walk 1–2 |
| C2 template followed by id | S1-T2 | `editing armor on a template member … reaches both placements`; walk 4 |
| C3 bare and legacy members | S1-T1 | `a placement of a rulebook monster with no recipe …`; walk 10 |
| C4 override fields | S1-T2 | `an override on a rulebook member asks for an id and adopts it`; walk 10 |
| C5 rename rewrites | S1-T1, S1-T2 | `rename rewrites …`; `undo after a cast rename …`; walk 8 |
| C6 delete refused with placements | S1-T1, S1-T2 | `remove with placements throws …`; `delete with placements shows the refusal …`; walk 9 |
| C7 palette lists the cast; ref only | S1-T3 | `the palette lists every cast member …`; `placing writes the ref …` |
| C8 stamped look and faction | S1-T3, S1-T4 | `placing stamps the default faction` (after S1-Q1); `changing the default faction afterwards …`; walk 3, 5 |
| C9 override and shuffle | S1-T4, S1-T6 | `changing one placement's look leaves its sibling unchanged`; shuffle tests; walk 6 |
| C10 cast selection | S1-T5 | `selecting a cast member highlights exactly its placements`; walk 7 |
| C11 prop variant | S1-T7 | **blocked on S1-Q2** |
| C12 missing stat block | S1-T4 | `a placement whose stat block is missing …` |

## S1 provider/consumer seams

| provider | consumer | contract | availability | proof |
|---|---|---|---|---|
| S1-T1 `castOf` and the edit functions | S1-T2, T3, T4, T5, T6 | `CastMember`; each edit returns a valid document or throws, `CastMemberInUse` naming placements | new | `cast.test.ts` |
| S1-T2 `session.cast` | `StudioCast`, `CastMemberView` | boolean intents, one transaction each | new | `StudioCast.test.tsx` |
| existing `validateMonsterTemplates`, `validateCreaturePalette` | S1-T1 | refusal sentences shown at fields | baseline | the rename-refusal test |
| existing `setMonsterFaction`, `withBinding` | S1-T3 | pure binding edit | baseline | `placing stamps the default faction …` |
| existing `npcAppearanceOptions` | S1-T3, T4, T6 | family and looks of a ref | baseline | shuffle tests |
| S1-T5 `{kind:'cast'}` target and `castMember` selection | S0 `StudioInspector`, `RoomActorMarkers` | one selection; `highlightedActorIds` | new, extending S0's T1 contract | `selecting a cast member highlights …` |
| S1-T7 `propVariantChoices` | `PropView` | provider-declared family members | **missing; S1-Q2** | — |
| `templates`, `monsterDeclarations[].ref`, `monsterBindings.<id>.faction` | rpg-toolkit decoder | keys already declared; refs checked for grammar only | baseline toolkit | `ci-check` codec tests; no toolkit change |

**Shared files.** S1 stacks on S0 and edits S0's `StudioInspector.tsx`, `studioSession.ts` and `WorldBuildingConcept.tsx`. S1 starts only from S0's final head, and takes later S0 fixes by merging the S0 branch in.

## Later slices

- S1, the cast and per-placement looks: rpg-project#575, planned above. A cast member selected on the home highlights its placements on the map, and the inspector shows the cast member. It is not a multi-selection (Q8).
- S2, read it back: prose, fact index, tables heaviest first: rpg-project#576.
- S3, overview on Layout: rpg-project#577. It retires the Regions home: region and lighting move to the hex view, and the room list moves to the Layout overview (Q5).
- S4, the hidden nouns: exits, endings, concealments, door state: rpg-project#578.
- S5, density: words, wrapping, panel width: rpg-project#579.
