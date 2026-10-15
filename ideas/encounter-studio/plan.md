---
name: Encounter studio S0 — implementation plan
status: PLAN for slices S0, S1, S5, S2, S3 and S4, all SHIPPED to web dev 2026-10-14 (head e623c2b3), derived 2026-10-14 from design.md and the S0 issue; checked against the inspected baseline below; open items resolved by the team lead 2026-10-14; S1 planned 2026-10-14 against the S0 branch; S5 planned 2026-10-14 against the S1 branch, ahead of S2; S2 planned 2026-10-14 against the S5 branch; S3 planned 2026-10-14 against the S2 branch; S4 planned 2026-10-14 against the S2 branch (S3 not yet pushed)
authority: design.md (rulings R1, R9, R11, R13 and the document's refusals); README.md (the inspector forms); rpg-project#574 (S0 acceptance); epic rpg-project#545
planning guide: ../../.agents/skills/design/planning.md
---

# Encounter studio S0 — the selection inspector

**S0 SHIPPED 2026-10-14:** web dev 543396d5 (rpg-dnd5e-web#1270, with #1272 and #1273; design #581 on main); follow-ups web#1274, rpg-project#582, stale strings → #579.

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

**S1 SHIPPED 2026-10-14:** web dev 5c43f571 (rpg-dnd5e-web#1275); follow-up web#1276; the walk's density feedback → S5 (#579).

Planned 2026-10-14 against the S0 branch, which S1 stacks on. Authority:
design.md (R3, R4, R11; R14 and R14b deferred for S1), rpg-project#575, and
the seven cast decisions the operator accepted on 2026-10-14, recorded on
#575 (https://github.com/KirkDiggler/rpg-project/issues/575#issuecomment-6161987171).

## S1 goal and scope

The Encounter home's Creatures and Stat blocks sections become one **Cast**.
A cast member is a stat block — a `templates.<id>` entry, or a bare rulebook
monster — plus the studio's defaults for new placements. The palette lists
the cast. Placing stamps the default look and faction onto the placement;
the stat block is followed by id. A look is per placement, with a shuffle.
A cast member selected on the home lights up its placements on the map.
A prop's variant pick is not in S1.

**Repositories.** Web only. S0 has shipped, so S1 branches from rpg-dnd5e-web `dev` (543396d5 or later).
rpg-toolkit, rpg-api and rpg-api-protos are not touched: every edit writes
keys the dialect already declares (`templates`, `monsterDeclarations[].ref`
and `.appearanceRef`, `monsterBindings.<id>.faction`) or the studio's own
`room.scene`.

**Non-goals:**

- Per-prop surface material, provider look choices **and the per-instance prop variant pick** (R14, R14b; rpg-game-assets#226, rpg-dnd5e-web#1268). No family exists to pick within, so the variant pick waits with the look catalog. No material field, no variant field.
- A prop shuffle (R14).
- Read-back prose (S2, #576), and any change to Tables, Factions, Dispositions or Intel.
- Multi-select as an editing mode (R9). Selecting a cast member is one thing, not a set of placements.

## S1 requirement labels

| label | requirement | source |
|---|---|---|
| C1 | The cast is derived, never a second list: one member per monster ref used by a template, a studio-defaults entry or a placement. | R3 |
| C2 | A template-backed member's stat block lives only in `templates.<id>`. Editing it reaches every placement by id. | R3 |
| C3 | A bare rulebook monster is a cast member without a template. A legacy placement with no template and no defaults entry appears as its rulebook kind. | #575 decisions 2 and 3 |
| C4 | The Stat blocks free-text form becomes the cast member's override fields: base, abilities, hit dice, armor, proficiency, skills, weapons, experience. Editing an override on a bare member first gives it a template. | lead brief |
| C5 | Renaming a cast member's id rewrites every reference: the template key, every placement's ref, and its defaults entry. | #575 decision 4 (its table half is outside S1) |
| C6 | Deleting a cast member that has placements is refused with the count, naming them. Deleting one with none removes its template and defaults. | #575 decision 5; absorbs rpg-dnd5e-web#1262's removal item |
| C7 | The palette lists the cast. A drop writes a ref to the stat block, never its fields. | R3, README refusal |
| C8 | Placing stamps the default look onto `appearanceRef` and the default faction onto `monsterBindings.<id>.faction`. A later default change reaches no placed creature. The inspector labels both as stamped. | R3, R4 |
| C9 | A placement's look may be overridden in its inspector. A shuffle gives one cast member's placements different looks from its default family. | R4 |
| C10 | A cast member selected on the home lights up its placements on the map, and the inspector shows the cast member. A selected placement presses its cast member's row. | R11 |
| C12 | A placement whose ref names no template and no listed rulebook monster is warned in its inspector, which offers the cast. Local save does not refuse; the decoder's refusal surfaces at Save & Play. | README edge, as resolved |
| C13 | A template id that names a rulebook monster is warned, never refused. | #575 decision 6 |
| C14 | A cast member's defaults (faction, look) live in `room.scene` beside the palette entries, never on the template. | #575 decision 1 |

## S1 constraints

- **No new document key outside the studio's presentation.** The cast reads
  `templates` and `scene.creaturePalette`. The one added field is an optional
  `faction` on a `scene.creaturePalette` entry (#575 decision 1). It is in
  because the toolkit carries `room.scene` through unread. **Evidence:** a
  throwaway test at rpg-toolkit `origin/main` (2f570aa9), in a scratch
  worktree since removed, injected
  `creaturePalette: [{id, name, ref, modelRef, faction: garrison}]` into
  `room.scene` of `testdata/world-builder-v4-castle-kitchen.yaml`.
  `DecodeSingleRoom` and `CompileSingleRoom` both returned no error. A
  control in the same test put `bogusKey` under `room.room` and was refused
  ("`bogusKey` is not a key this build reads"), so the test detects
  refusals. Output: `--- PASS: TestZZSceneCreaturePaletteFactionOpaque`.
- **The owner stays the owner.** Every cast edit is one `commitRoomDocument` transaction that may touch `scope.templates`, `draft.scene.creaturePalette`, `draft.scene.actorNames` and `draft.room.monsterDeclarations` together, so undo reverses a rename or a placement in one step.
- **The web does not resolve the rulebook.** Whether a ref names a real rulebook monster, and whether a template id shadows one, are the rulebook's questions (`dungeonspec/single_room.go`, `spec.go`). The studio never refuses on them.
- **Refusal at the boundary.** `validateMonsterTemplates` and `validateCreaturePalette` keep their gates; the cast form shows their sentences at the field.
- S0's constraints carry over unchanged.

## S1 inspected baseline (2026-10-14)

| repo | revision |
|---|---|
| rpg-dnd5e-web `feat/studio-s0-inspector` | f0e0ef8e (inspected; S0 merged to `dev` as 543396d5) |
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
```

One web branch off `dev`, one PR to `dev`. It catches up by merging `dev` in, never by a rebase or force-push.

## S1 open items

| id | item | state | blocks |
|---|---|---|---|
| S1-Q1 | **Where the default faction lives.** | **resolved:** an optional `faction` on `scene.creaturePalette` entries; the toolkit decodes and compiles it (see Constraints for the evidence) | nothing |
| S1-Q2 | **The prop variant pick.** No family exists to pick within, and the look work is being redone (rpg-game-assets#226, rpg-dnd5e-web#1268). | **resolved:** dropped from S1 and deferred with R14b | nothing; no S1 task |
| S1-Q3 | **Rulebook catalog over the wire for pick-lists.** Armor, weapon and base pick-lists need a provider listing the rulebook's refs. S1 keeps typed refs with the grammar check. | open, routed to S5 (rpg-project#579) | nothing in S1 |
| S1-Q4 | **Two recipes on one ref.** | **resolved:** the first entry is the defaults; later ones stay in the file and are listed on the member as unused looks, which the author may remove | nothing |
| S1-Q5 | **A stat block that resolves to nothing.** | **resolved:** the inspector warns; local save does not refuse; the decoder's refusal surfaces at Save & Play | nothing |
| S1-Q6 | **What the shuffle varies.** | **resolved:** looks within the cast member's appearance family only (#575 decision 7) | nothing |
| S1-Q7 | **A template id that names a rulebook monster.** | **resolved:** warned, never refused (#575 decision 6) | nothing |
| S1-Q8 | **The cast decisions' record.** | **resolved:** the #575 comment cited in the authority line | nothing |
---

## Task S1-T1: The cast model

**Delivers:** C1, C3, C5, C6 and the S1-Q4 assumption, as pure functions.
**Owner:** rpg-dnd5e-web, new `src/concepts/world-building/cast.ts`.
**Prerequisites:** none.
**Files:** new `world-building/cast.ts` and `world-building/cast.test.ts`; `world-building/creaturePalette.ts`: `CreaturePaletteEntry` gains `faction?: string`, and `validateCreaturePalette` accepts the key (nonblank text up to 120 characters).
**Interfaces:**

```ts
export interface CastMember {
  readonly ref: string;                    // dnd5e:monsters:<id>; the identity
  readonly id: string;                     // the ref's id after the type
  readonly source: 'template' | 'rulebook';// template when templates[id] exists
  readonly template?: Readonly<MonsterTemplate>;
  readonly defaults?: Readonly<CreaturePaletteEntry>; // first entry with this ref
  readonly unusedDefaults: readonly CreaturePaletteEntry[]; // later entries with this ref, shown as unused looks
  readonly shadowsRulebook: boolean;       // template id equals a PALETTE_MONSTERS id; warned only
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
- `saveCastMember` refuses a default faction that is not a `scope.factions` id, naming it.
- A template id that equals a rulebook monster id saves; `shadowsRulebook` is true and the form warns (C13).
- Each function returns a document that `validateRoomDocument` accepts, or throws.

**Tests** (`cast.test.ts`):
- `a template, a recipe on it and two placements are one member`.
- `a placement of a rulebook monster with no recipe is a rulebook member with no defaults`.
- `two recipes on one ref: the first is the defaults and the second is an unused look` (S1-Q4).
- `a recipe's faction round-trips through validateCreaturePalette`; `a default faction naming no faction is refused`.
- `a template id equal to a rulebook monster id saves and is marked as shadowing`.
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
| Armor, weapons | `.armor`, `.actions` | typed refs checked by `validateMonsterTemplates`; weapons one per row, in order. A pick-list waits for a rulebook catalog over the wire (S1-Q3, S5) |
| Skills | `.skills` | pick-from-list of the 18 skill ids |
| Default look | the defaults `modelRef` | pick-from-list of `NPC_APPEARANCE_FAMILIES` with their thumbnails |
| Default faction | the defaults `faction` (`scene.creaturePalette[].faction`) | pick-from-list of `scope.factions` |
| Placements | read-only | list; clicking one selects it on the map (R11) |
| Unused looks | read-only, with Remove | list (S1-Q4) |
| Delete | `remove` | button; shows `CastMemberInUse`'s sentence in an alert when refused |

**Behavior:**
- An override field edited on a rulebook member opens the Id field and saves through `adopt` first, then `save`, in one transaction.
- Every field change is staged; Apply is one transaction; Cancel discards; a refusal shows at the form and keeps the staged values.
- The member list names each member, its source ("stat block" or "rulebook"), and its placement count.
- A member whose id shadows a rulebook monster shows "This id also names a rulebook monster; the rulebook decides which is used". Saving is not refused (C13).

**Tests:**
- `StudioCast.test.tsx` › `the home lists Cast, Tables, Factions, Dispositions and Intel, and no Creatures or Stat blocks`; › `a shadowing id warns and still saves`; › `editing armor on a template member is one transaction that reaches both placements`; › `an override on a rulebook member asks for an id and adopts it`; › `renaming the id rewrites the placements`; › `delete with placements shows the refusal naming them and changes nothing`; › `delete without placements removes the member`; › `skills are picked from the eighteen skill ids`.
- `EncounterStudioIntegration.test.tsx` › `undo after a cast rename restores the old id on every placement`.

- [ ] Write the tests; confirm they fail.
- [ ] Build `StudioCast`, wire the session, retire the two sections and their files.
- [ ] Grep `StudioStatBlocks`, `savePaletteEntry` and `Create creature recipe` under `src/`; each prints nothing.

**Verification:** `npx vitest run src/concepts/encounter-studio` passes.
**Completion evidence:** named tests; the grep output.

## Task S1-T3: The palette lists the cast; placing stamps

**Delivers:** C7, C8.
**Owner:** rpg-dnd5e-web; `StudioCreaturePalette.tsx`, `WorldBuildingConcept.tsx` `placeMonsterAt`.
**Prerequisites:** S1-T2.
**Files:** `encounter-studio/StudioCreaturePalette.tsx`; `world-building/WorldBuildingConcept.tsx` (`armPlacement`, `placeMonsterAt`, the `armedCreature` state); `encounter-studio/studioSession.ts` (`creatures.armPlacement(ref, appearanceRef)`, keyed by the member's ref instead of the recipe id); tests in `EncounterStudioIntegration.test.tsx` and `WorldBuildingConcept.test.tsx`.
**Interfaces:** `armPlacement(ref: string, appearanceRef: string): boolean` refuses a ref that is not a cast member, and an `appearanceRef` outside the member's default family. A member without a default look is listed but cannot be armed: "Choose a default look in the cast to place it."
**Behavior:**
- One transaction writes the declaration (`ref`, `appearanceRef`, cell), the map name (the member's name), and, when the defaults carry a faction, `monsterBindings.<id>.faction` through `setMonsterFaction` and `withBinding`.
- The declaration never copies template fields (C7).
- A default faction that no longer names a faction refuses the placement: "Guard's default faction kitchen no longer exists. Choose another in the cast."
**Tests:**
- › `the palette lists every cast member, including a rulebook member`.
- › `placing writes the ref, the chosen look and the name in one history entry`.
- › `placing stamps the default faction onto the binding`; › `a stale default faction refuses the placement without a history entry`.
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
- A ref that names no template and no listed rulebook monster shows the warning "This placement's stat block, dnd5e:monsters:x, is not in the cast", and a cast pick-list that writes through `creatures.setDefinition`. Local save does not refuse; if the decoder refuses the file, the refusal surfaces at Save & Play (S1-Q5).
**Tests:** › `the creature view names its cast member and links to it`; › `a look equal to the default reads as the default, a changed one as stamped`; › `changing one placement's look leaves its sibling unchanged`; › `a placement whose stat block is missing warns, offers the cast and reassigns through one intent`; › `a placement with a missing stat block still saves locally`.

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

## Task S1-T8: Integration and the walk

**Delivers:** proof of C1–C12 on the running studio.
**Prerequisites:** S1-T1–T6.
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
11. Save the local draft and reload: everything above is still there.

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
| C8 stamped look and faction | S1-T3, S1-T4 | `placing stamps the default faction onto the binding`; `changing the default faction afterwards …`; walk 3, 5 |
| C9 override and shuffle | S1-T4, S1-T6 | `changing one placement's look leaves its sibling unchanged`; shuffle tests; walk 6 |
| C10 cast selection | S1-T5 | `selecting a cast member highlights exactly its placements`; walk 7 |
| C12 missing stat block | S1-T4 | `a placement whose stat block is missing warns …`; `… still saves locally` |
| C13 shadowing warned | S1-T1, S1-T2 | `a template id equal to a rulebook monster id saves …`; `a shadowing id warns and still saves` |
| C14 defaults in the scene | S1-T1 | `a recipe's faction round-trips …`; the toolkit decode evidence under Constraints |

## S1 provider/consumer seams

| provider | consumer | contract | availability | proof |
|---|---|---|---|---|
| S1-T1 `castOf` and the edit functions | S1-T2, T3, T4, T5, T6 | `CastMember`; each edit returns a valid document or throws, `CastMemberInUse` naming placements | new | `cast.test.ts` |
| S1-T2 `session.cast` | `StudioCast`, `CastMemberView` | boolean intents, one transaction each | new | `StudioCast.test.tsx` |
| existing `validateMonsterTemplates`, `validateCreaturePalette` | S1-T1 | refusal sentences shown at fields | baseline | the rename-refusal test |
| existing `setMonsterFaction`, `withBinding` | S1-T3 | pure binding edit | baseline | `placing stamps the default faction …` |
| existing `npcAppearanceOptions` | S1-T3, T4, T6 | family and looks of a ref | baseline | shuffle tests |
| S1-T5 `{kind:'cast'}` target and `castMember` selection | S0 `StudioInspector`, `RoomActorMarkers` | one selection; `highlightedActorIds` | new, extending S0's T1 contract | `selecting a cast member highlights …` |
| `templates`, `monsterDeclarations[].ref`, `monsterBindings.<id>.faction` | rpg-toolkit decoder | keys already declared; refs checked for grammar only | baseline toolkit | `ci-check` codec tests; no toolkit change |
| `scene.creaturePalette[].faction` | rpg-toolkit decoder | carried through `room.scene` unread | baseline toolkit, verified | the throwaway decode and compile under Constraints |

**S1 deferrals:** the per-instance prop variant pick and surface material (R14, R14b); armor, weapon and base pick-lists from a rulebook catalog (S1-Q3, S5); renaming a table (#575 decision 4's table half).

**Shared files.** S1 edits S0's `StudioInspector.tsx`, `studioSession.ts` and `WorldBuildingConcept.tsx`, now on `dev`. S1 branches from `dev` after 543396d5.

---

# Encounter studio S5 — density: words, wrapping, panel width

**S5 SHIPPED 2026-10-14:** web dev e9a822ec (rpg-dnd5e-web#1277); follow-ups web#1278, web#1280.

Planned 2026-10-14, pulled ahead of S2. S2 adds read-back prose to the top
of the same inspector, so the inspector's type and grouping settle first.
Authority: design.md (the inspector forms in README.md; R10's on-screen word
stays open); rpg-project#579 and its two comments: the operator's S1-walk
feedback on the creature inspector, and the S0 review's stale strings.

## S5 goal and scope

The inspector and the Encounter home read as a document: headings at a
scale, labels distinct from values, one break between what a placement is
and how it behaves, words instead of ids, help off the main flow, rows that
summarise until opened, and no horizontal scroll at 1280 pixels.

**Repositories.** Web only, rpg-dnd5e-web. S5 builds after S1
(`feat/studio-s1-cast`, rpg-dnd5e-web#1275) merges to `dev`. S1 has merged, so S5 branches from `dev` at 5c43f571 or later. No
toolkit, API or proto change; no document change.

**Non-goals:**

- Read-back prose (S2, #576), the Layout overview (S3, #577), the hidden nouns (S4, #578).
- **Rulebook catalog over the wire for pick-lists** (S1-Q3). It stays open. The web has no armor, weapon or base list beyond `PALETTE_MONSTERS`, so S5 changes no picker source.
- The on-screen word for the document (R10, design Open).
- The old World Builder's look. Every style change is scoped to `.encounter-studio`.
- Behaviour: no field moves between owners, and no intent changes shape.

## S5 requirement labels

| label | requirement | source |
|---|---|---|
| E1 | One studio type scale: panel title, section heading, sub-heading, field label, value, help. Labels and values differ in weight; every section begins with a heading. | #579 operator feedback |
| E2 | One visual break between the placement form (what it is, where) and the configuration (how it behaves). Inherits and Overrides are proper sub-sections with headings, not small caps among body text. | #579 |
| E3 | No horizontal scroll in the inspector or any home form at a 1280-pixel window. | #579 issue scenario 3 |
| E4 | The retired word Arrange leaves every button, status line and message. Buttons read Apply and Cancel. | #579, S0 review comment |
| E5 | A raw id or ref is never the primary line. A placement shows its map name or kind name; its id sits behind a copy affordance. A rules ref shows as the kind's display name, with the ref on hover. | #579 |
| E6 | Help sentences leave the main flow: one help affordance per field, and one "About this panel" disclosure for panel-level text. Status and refusal lines stay inline. | #579 |
| E7 | Option rows wrap by word: the line field, the check rows and the summary. | #579 issue scenario 2 |
| E8 | A table entry is one summary line until opened. A table's collapsed summary shows its entry count and who uses it. | lead brief, measured at about 300 pixels per entry |
| E9 | The studio section of `world-building/CONTRACT.md` and the wall-corner message describe the inspector, not Arrange. | S0 review comment on #579 |
| E10 | Faction mind suggestions show authored map names, with placement ids kept as values. | absorbs rpg-dnd5e-web#1262's mind-suggestion item |

## S5 constraints

- **No behaviour change.** Every edit still crosses the intents S0 and S1 defined. The one-candidate-per-form contract (`StudioArrangeIntent`) stays.
- **Scoped styles.** `worldBuilding.css` (`wb-*`) is shared with the old World Builder. S5 overrides `wb-*` rules only under `.encounter-studio`, and adds `es-*` tokens.
- **Ids stay addressable.** A hidden id stays in the copy affordance, in `title`, and in `aria-label`s that tests and the walk rely on.
- **Words come from the document or the catalog.** Map names from `scene.actorNames`, kind names from `paletteNameForRef` or the cast member's name. The studio invents no name.

## S5 inspected baseline (2026-10-14)

| repo | revision |
|---|---|
| rpg-dnd5e-web `feat/studio-s1-cast` | bfdb0283 (inspected; S1 merged to `dev` as 5c43f571) |
| rpg-project `origin/main` | as S1 |

- **Type.** No type tokens exist. `encounterStudio.css` has 15 `font-size` declarations and `worldBuilding.css` 47, from 0.62rem to 0.78rem. Policy blocks use `h4`, `h5` and `h6` with no shared scale.
- **The creature inspector** (`StudioInspector.tsx` `CreatureView`, over `StudioArrangePanel.tsx` `SelectedArrange` and `session.socialControls.selection`):
  - `StudioArrangePanel.tsx` prints "Rules: {ref}. Map name is an authoring label, not a stat-block change." and has buttons "Apply Arrange" and "Cancel Arrange".
  - `SitePolicies.tsx` `CreatureOrders` prints "Selected creature" then "{kind name} · {placement id}", which is the raw UUID line. It also has `h5` Inherits and Overrides inside `wb-policy-block`.
  - "Configuration" is an `h3` directly under the form, with no break.
- **Other Arrange strings.** `StudioWallFinishes.tsx` "Wall finish staged · Apply Arrange to save."; `wallCornerFits.ts` "…Enter the same value on both walls in Arrange."; `CONTRACT.md` names Arrange in the studio section, including "options contain Arrange, Walls, Doors and Notes"; `encounterStudio.css` has a comment; `studioTestNavigation.ts` has an `'Arrange'` alias.
- **Tables.** `TablesPanel.tsx` renders each table as `<details>` whose summary is "{id} · N triggers". Every entry is an `AnswerEntryRow` (`SitePolicies.tsx`), fully expanded with its condition, word, selector, weight and line controls. A one-line sentence renderer already exists and is tested in `AnswerEntrySentence.test.tsx`.
- **Options.** `OptionsEditor` (`SitePolicies.tsx`) uses a single-line `input` for the line; its summary is `"{id}: “{say}” · {check}{gate}"`.
- **Mind and member pickers.** `SitePolicies.tsx` lists placements as "{id} · {kind}" or "{kind} · {id}" in three selects.
- **Browser checks.** The web has no browser test runner. Walks use game-dev `tools/browser/` drivers.

## S5 sequence

```
S5-T1 type scale and grouping ──▶ S5-T2 overflow ──▶ S5-T6 integration
S5-T3 words not ids ──────────────────────────────┤
S5-T4 Apply, Cancel and help ─────────────────────┤
S5-T5 table and option rows ──────────────────────┘
```

One branch off `dev`, one PR to `dev`. T1 goes first because T2–T5 use its tokens and section components. After T1, T3, T4 and T5 touch different files except `SitePolicies.tsx`, which T3 (labels), T4 (help) and T5 (rows) each edit. They run in order T3, T4, T5 by one builder.

## S5 open items

| id | item | state | blocks |
|---|---|---|---|
| S5-Q1 | **Apply-on-change.** | **lead-resolved 2026-10-14:** self-contained pickers (look, facing, faction) apply on change, each its own undo step; text and number fields stay staged behind Apply and Cancel | nothing |
| S5-Q2 | **Help placement.** | **lead-resolved:** a field's help sits behind that field's "?"; panel-level help sits in one "About this panel" | nothing |
| S5-Q3 | **Two placements with one map name.** | **lead-resolved:** "Guard", "Guard (2)" in document order; the id stays the value and the `title` | nothing |
| S5-Q4 | **Inspector width.** | **lead-resolved:** the width stays `min(24rem, 100%)`; the overflow causes are fixed | nothing |
| S5-Q6 | **Seeding the castle fixture as a local draft.** Static check at bfdb0283: `createCastleWorkspaceDocument()` (`encounter-studio/fixtures/castleWorkspace.ts`) returns a validated `RoomDraftDocument`, and the room owner persists drafts with `stringifyRoomDraft` under `rpg.concepts.world-building.room-draft.v3` (`ROOM_DRAFT_STORAGE_KEY`, `roomDraft.ts`). **Path:** a throwaway vitest writes `stringifyRoomDraft` of the fixture to JSON in the job tmp directory, and the driver sets that key in `localStorage` before opening the studio. The driver records whether the studio opened the castle; if it did not, it builds the room by hand and records that path instead. Not yet run. | **lead-resolved:** path recorded; the builder confirms at run time | nothing |
| S5-Q5 | **Rulebook catalog over the wire for pick-lists.** | open; not in S5 (S1-Q3) | nothing in S5 |

---

## Task S5-T1: Type scale and grouping

**Delivers:** E1, E2.
**Owner:** rpg-dnd5e-web; `encounter-studio/encounterStudio.css`, new `encounter-studio/StudioSection.tsx`, `StudioInspector.tsx`, `StudioArrangePanel.tsx`, `SitePolicies.tsx` `CreatureOrders`.
**Prerequisites:** S1 merged.
**Files:** `encounterStudio.css` (tokens and scoped `wb-*` overrides); new `StudioSection.tsx`; `StudioInspector.tsx` (each view composes sections); `StudioArrangePanel.tsx` (the "Configuration" heading becomes a section); `SitePolicies.tsx` (`CreatureOrders` Inherits and Overrides become sections); tests in `StudioInspector.test.tsx` and a new `StudioSection.test.tsx`.
**Interfaces:**

```ts
// encounterStudio.css, on .encounter-studio
--es-text-title; --es-text-section; --es-text-subsection;
--es-text-label; --es-text-value; --es-text-help;
--es-weight-label; --es-weight-value; --es-space-section;

export function StudioSection(props: {
  title: string;
  level: 'section' | 'subsection';   // h3 or h4 inside the inspector's h2
  children: ReactNode;
  about?: ReactNode;                 // S5-T4's panel help, optional
}): React.JSX.Element;               // <section aria-labelledby> with a heading
```

**Behavior:**
- The inspector `h2` uses `--es-text-title`. A placed creature has two sections, "Placement" (cast member, map name, look, cell, facing, Move) and "Behaviour" (faction, table, orders, says, holds, dispositions), separated by `--es-space-section` and a rule.
- Inherits and Overrides are "Inherits" and "Overrides" sub-sections in title case.
- A prop splits into "Placement" and "Behaviour" the same way. A wall, door, hex, label and party start have one section.
- Field labels use `--es-weight-label` and values `--es-weight-value`. Every `dt`/`dd` pair and every form row follows that rule.

**Tests:** `StudioSection.test.tsx` › `renders a labelled section with an h3 or h4`. `StudioInspector.test.tsx` › `a creature view has a Placement and a Behaviour section in that order`; › `Inherits and Overrides are subsection headings`; › `a prop view has Placement and Behaviour`; › `a wall view has one section`.

- [ ] Tests first; tokens; `StudioSection`; compose the views.

**Verification:** `npx vitest run src/concepts/encounter-studio` passes.
**Completion evidence:** named tests.

## Task S5-T2: No horizontal scroll

**Delivers:** E3, S5-Q4.
**Owner:** rpg-dnd5e-web; `encounterStudio.css`.
**Prerequisites:** S5-T1.
**Files:** `encounterStudio.css` only.
**Behavior:** inside `#studio-sidebar` and `.es-encounter-home`:
- values and inline code take `overflow-wrap: anywhere`;
- every `input`, `select` and `textarea` takes `max-width: 100%` and `min-width: 0`;
- flex and grid children take `min-width: 0`;
- no rule sets `overflow-x: hidden` to mask an overflow.

**Tests:** jsdom cannot measure layout, so the proof is the T6 driver's measurement. A CSS assertion test is not written.

- [ ] Change the rules; run the T6 driver's measurement.

**Verification:** the T6 driver reports `scrollWidth <= clientWidth` for `#studio-sidebar` and each home form at 1280×800, for the creature, prop, wall and hex views and the Cast, Tables, Factions, Dispositions and Intel forms.
**Completion evidence:** the driver's output table in the PR.

## Task S5-T3: Words, not ids

**Delivers:** E5, E10, S5-Q3.
**Owner:** rpg-dnd5e-web; new `encounter-studio/CopyId.tsx`, `StudioArrangePanel.tsx`, `SitePolicies.tsx`, `StudioInspector.tsx`.
**Prerequisites:** S5-T1.
**Files:** new `CopyId.tsx`; new `world-building/placementNames.ts`; `StudioArrangePanel.tsx` (the Rules line); `SitePolicies.tsx` (`CreatureOrders`' "Selected creature" line and the three placement selects); tests.
**Interfaces:**

```ts
export function CopyId(props: { id: string; label: string }): React.JSX.Element;
// a button "Copy id" with title=id and aria-label="Copy <label> id"; writes id to the clipboard; shows "Copied"

export function placementDisplayNames(input: {
  declarations: readonly RoomMonsterPlacement[];
  actorNames?: Readonly<Record<string, string>>;
  castName(ref: string): string;          // cast member name, else paletteNameForRef
}): ReadonlyMap<string, string>;          // id -> "Guard", "Guard (2)", ...
```

**Behavior:**
- The creature view's primary line is the map name, else the kind name, followed by `CopyId`. The id is never printed as text.
- "Rules: dnd5e:monsters:x" becomes "Kind: Prisoner", with the ref in `title`.
- The three placement selects in `SitePolicies.tsx` show `placementDisplayNames`, with the id as the option `value` and `title`.
- No change to which id is stored.

**Tests:** `placementNames.test.ts` › `a map name wins over the kind name`; › `two equal names get (2) in document order`; › `an unnamed placement shows its kind name`. `StudioInspector.test.tsx` › `the creature view never prints the placement id as text`; › `Copy id copies the placement id`; › `the kind shows its name with the ref on hover`. `SitePolicies.test.tsx` › `the mind picker shows map names and keeps ids as values`.

- [ ] Tests first; implement.

**Verification:** `npx vitest run src/concepts/encounter-studio src/concepts/world-building/SitePolicies.test.tsx src/concepts/world-building/placementNames.test.ts` passes.
**Completion evidence:** named tests.

## Task S5-T4: Apply, Cancel and help off the flow

**Delivers:** E4, E6, E9, S5-Q1, S5-Q2.
**Owner:** rpg-dnd5e-web; `StudioArrangePanel.tsx`, `StudioArrangeFields.tsx`, `StudioWallFinishes.tsx`, `wallCornerFits.ts`, `CONTRACT.md`, new `encounter-studio/FieldHelp.tsx`, `SitePolicies.tsx`, `StudioCast.tsx`.
**Prerequisites:** S5-T1.
**Files:** as Owner; plus `encounterStudio.css` (the comment), `studioTestNavigation.ts` (drop the `'Arrange'` alias, and update its callers), and their tests.
**Interfaces:**

```ts
export function FieldHelp(props: { field: string; children: ReactNode }): React.JSX.Element;
// a "?" button, aria-label="About <field>", aria-expanded; reveals the text below the field
```

`StudioArrangeField` gains an optional `help?: string`, rendered through `FieldHelp`. `ArrangeField.applyOnChange?: true` marks a self-contained picker (S5-Q1).

**Behavior:**
- Buttons read "Apply" and "Cancel". Status lines read "… staged · Apply to save".
- The wall-corner message ends "Enter the same value on both walls." `CONTRACT.md`'s studio section describes the inspector, its nine views and the pick strip, and keeps no sentence saying the sidebar holds Arrange, Walls, Doors and Notes.
- Field-specific sentences in the inspector, the cast form and `CreatureOrders` move into `FieldHelp`. For example, "Map name is an authoring label, not a stat-block change" moves to Map name. Panel-level sentences move into the section's `about`. `role="status"` and `role="alert"` lines stay inline.
- A picker marked `applyOnChange` (look family, style, skin palette, facing, faction) commits on change through its existing intent, and Cancel no longer covers it. Text and number fields stay staged.

**Tests:** › `no button, status or message in the studio contains Arrange`: a render sweep over each view's text, plus a source grep in T6. › `Map name has a help affordance and no inline sentence`; › `the About this panel disclosure is collapsed by default`; › `changing facing commits one intent without Apply`; › `typing a map name commits only on Apply`; new `world-building/wallCornerFits.test.ts` › `the appearance-mismatch message does not say Arrange`.

- [ ] Tests first; implement; update `CONTRACT.md`. Test › `each apply-on-change picker edit is its own history entry` (S5-Q1).

**Verification:** `npx vitest run src/concepts` passes; `git grep -nE "Apply Arrange|Cancel Arrange|in Arrange|Arrange to save" src/` prints nothing.
**Completion evidence:** named tests; the grep.

## Task S5-T5: Table and option rows

**Delivers:** E7, E8.
**Owner:** rpg-dnd5e-web; `TablesPanel.tsx`, `SitePolicies.tsx` (`AnswerEntryRow`, `OptionsEditor`).
**Prerequisites:** S5-T1.
**Files:** `TablesPanel.tsx`; `SitePolicies.tsx`; `encounterStudio.css`; tests in `TablesPanel.test.tsx`, `SitePolicies.test.tsx`.
**Interfaces:**
- `AnswerEntryRow` gains `collapsed` (default true in the studio). Collapsed, it renders the existing entry sentence on one line with "Edit" and "Remove". Open, it renders today's controls. Opening one row closes none of the others.
- `tableUsers(scope, room, id): { factions: string[]; placements: string[] }`, where factions have `table === id` and placements have `monsterBindings.<p>.table === id`.
- The table summary becomes "{id} · N entries · used by garrison, Guard, Guard (2)" (placement names from S5-T3), or "· not used".
- `OptionsEditor`'s line field becomes a `textarea` that grows with its text and wraps by word. The option summary, check rows and fact ids wrap.

**Behavior:** no entry, table or option changes shape, and the save order is unchanged.

**Tests:** › `a collapsed entry shows one sentence and opens on Edit`; › `editing an opened entry commits exactly as before`; › `the table summary counts entries, not triggers`; › `the summary names the factions and placements that use the table`; › `an unused table says not used`; › `the option line is a textarea and keeps its text on commit`.

- [ ] Tests first; implement.

**Verification:** `npx vitest run src/concepts/world-building/TablesPanel.test.tsx src/concepts/world-building/SitePolicies.test.tsx` passes.
**Completion evidence:** named tests.

## Task S5-T6: Integration, measurement and before/after

**Delivers:** proof of E1–E10.
**Prerequisites:** S5-T1–T5.
- [ ] `npm ci`; `npm run ci-check` once; grep the log.
- [ ] Write `tools/browser/_job_s5_density.mjs` in game-dev (local by name). It seeds the studio's local draft from the castle fixture by the S5-Q6 path, and records in its output whether the castle loaded or the room was built by hand. Then, at 1280×800, it opens each view and form listed in S5-T2, and records `scrollWidth` and `clientWidth`. It also screenshots the creature inspector in 3D, the prop inspector, one table with one entry open, and one option row.
- [ ] Run it on `dev` before the branch (the before set) and on the branch (the after set).
- [ ] Post the checklist below on #579 as `- [ ]` boxes, with the before/after pairs, before the PR leaves draft.

**Walk and #579 checklist:**

1. Before/after: the creature inspector in 3D. The after has a Placement section and a Behaviour section separated by a break. Its first line is the map name with Copy id. Kind shows a name, not a ref. No inline help sentence; Map name has a help button.
2. Before/after: the prop inspector, with Placement and Behaviour sections.
3. Before/after: a table with one entry. Collapsed, the entry is one line; the summary shows the entry count and its users.
4. Before/after: an option row whose 200-character line wraps by word.
5. The buttons read Apply and Cancel. Change facing: it applies at once. Type a map name: it applies on Apply.
6. The driver's table shows no horizontal overflow in any listed view or form at 1280.
7. The faction mind picker shows map names, and two Guards read Guard and Guard (2).
8. Undo after each edit above restores it: behaviour is unchanged.
9. Save the local draft and reload; nothing changed shape.

## S5 requirement coverage

| requirement | task(s) | proof |
|---|---|---|
| E1 type scale | S5-T1 | `StudioSection` tests; walk 1–2 |
| E2 the break; Inherits and Overrides | S5-T1 | `a creature view has a Placement and a Behaviour section …`; `Inherits and Overrides are subsection headings` |
| E3 no horizontal scroll | S5-T2, T6 | the driver's measurement table; walk 6 |
| E4 Arrange retired | S5-T4 | the render sweep; the source grep |
| E5 words not ids | S5-T3 | `the creature view never prints the placement id as text`; `Copy id …`; walk 1 |
| E6 help off the flow | S5-T4 | `Map name has a help affordance …`; `About this panel … collapsed` |
| E7 option rows wrap | S5-T5 | `the option line is a textarea …`; walk 4 |
| E8 table rows summarise | S5-T5 | `a collapsed entry shows one sentence …`; `the table summary counts entries …`; walk 3 |
| E9 stale strings | S5-T4 | the new `wallCornerFits.test.ts`; the `CONTRACT.md` diff; the grep |
| E10 mind picker names | S5-T3 | `the mind picker shows map names …`; walk 7 |

## S5 provider/consumer seams

| provider | consumer | contract | availability | proof |
|---|---|---|---|---|
| S5-T1 tokens and `StudioSection` | S5-T2–T5, and S2's read-back later | CSS custom properties on `.encounter-studio`; a labelled section | new | `StudioSection.test.tsx` |
| S5-T3 `placementDisplayNames` | `CreatureOrders`, the mind and member selects, S5-T5's table users | id to display name; ids unchanged | new | `placementNames.test.ts` |
| S5-T3 `CopyId` | creature view; later S2 | button with the id in `title` and the clipboard | new | `Copy id copies the placement id` |
| S5-T4 `FieldHelp` and `ArrangeField.help`, `.applyOnChange` | inspector, cast form, `CreatureOrders` | per-field help; picker commit on change through existing intents | new | the T4 tests |
| S5-T5 `tableUsers` | `TablesPanel` summary | factions and placements naming a table | new, over `scope` and `room` | `the summary names the factions and placements …` |
| existing intents (`commitArrange`, binding, cast) | all views | unchanged | baseline | walk 8 |

**Shared files.** `SitePolicies.tsx` is edited by S5-T3, T4 and T5, in that order on one branch. `encounterStudio.css` is edited by T1, T2 and T5; T1 lands first.

---

# Encounter studio S2 — read it back

**S2 SHIPPED 2026-10-14:** web dev 59653543 (rpg-dnd5e-web#1279); follow-up: engine law text over the wire (design.md Open).

Planned 2026-10-14 against the S5 branch; S5 merges first and S2 branches
from `dev` after it. Authority: design.md R5 and R12; README.md's read-back
prose for gate-1 and the cook, its fact-index rows, and its "Separations that
look like one thing" hazard; rpg-project#576; the config scout's four items
that are not in the document (the kind's default table, engine laws, a fact
cross-reference, patrol cells).

## S2 goal and scope

A placed creature's or prop's inspector opens with prose rendered from the
draft: who it is and whose side it is on, what it does on its time, what can
be said to it, and what it knows. The Encounter home gains a derived fact
index. Every weighted row list displays and saves heaviest first. The faction
readout shows the faction's named table, and a disposition's `until` reads
as a sentence.

**Repositories.** Web only, rpg-dnd5e-web, branched from `dev` after S5
(`feat/studio-s5-density`) merges. No toolkit, API or proto change; no new
document key.

**Non-goals:** the Layout overview (S3), the hidden nouns' editors (S4), the
contents of the kind's default table (not in the document or on the wire),
simulating a roll, and patrol cells (`path` is refused in this dialect).

## S2 requirement labels

| label | requirement | source |
|---|---|---|
| B1 | Prose heads the creature and prop views, rendered from the draft only. | R5 |
| B2 | Orders read in the compiler's layers, per trigger, nearest wins wholesale: the creature's own rows, its named table, its faction's own rows, the faction's named table, then the kind's default. Each surviving row is one sentence naming its source and its other users. | R5; README walk step 4; toolkit `inheritedOrders`, `ordersOf`, single-room binding layering |
| B3 | A trigger no layer names is stated as "the kind's default". Its rows are never listed. | R5; scout item 1 |
| B4 | Engine laws are stated as laws, each with a toolkit source: what `until` turns a stance to, a fact `until` judged on the faction's mind, provocation by a hostile act, temper dealt at spawn from a mix, hearing by passive Perception against a DC that rises per region hop, and holding when no row holds. No law without a source. | R5; scout item 2 |
| B5 | Prose uses the row editor's vocabulary, from one shared module. | lead brief |
| B6 | Every row in prose carries its weight as the number, with the weight law stated once per table (S2 weight decision below). | lead brief |
| B7 | Options read with their line, each check and DC, the gate (fact or held prop), and both outcomes. | R5; README cook prose |
| B8 | "It knows" lists the intel it holds and what each reveals, then the facts its rows and options write and read. | lead brief |
| B9 | The fact index lists every fact id the draft mentions, with its writers and readers named, and marks a fact written and never read, or read and never written. It stores nothing. Clicking a writer or reader selects that thing. | R5; scout item 3 |
| B10 | Every weighted row list displays and saves heaviest first, ties in authored order. | R12 |
| B11 | The faction readout reads the faction's named table under its own rows. | #576; README hazard |
| B12 | A disposition's `until`, a binding's `arrives` and an ending's `when` render as sentences, never JSON or "fact x". | lead brief |

**S2 weight decision: the number, not an adjective.** The engine picks among
the rows whose conditions hold at that moment, so a row's real share depends
on which others are eligible. "Most often" would claim a share the read-back
cannot know without simulating, and the read-back never simulates (README
refusal). The prose therefore says "(weight 3)" on each row. Each table
states once: "Among the rows that hold, one is picked in proportion to its
weight." When every row has the same weight it says "every row weighted
alike", as the walkthrough does.

## S2 constraints

- **The draft is the only input.** No API call, no rulebook lookup beyond the names the studio already shows (`paletteNameForRef`, the cast name).
- **One vocabulary.** Prose and the row editor read one module. A phrase changes in one place.
- **Laws carry sources.** `engineLaws.ts` holds every law sentence with the toolkit file and symbol it states. Changing the engine's law means changing that entry.
- **Layering mirrors the compiler.** The orders function cites `inheritedOrders`, `ordersOf` and the single-room binding layering in rpg-toolkit `dungeonspec`, and its tests mirror those functions' cases.
- **Refusals unchanged.** S2 adds no gate. The R12 sort happens inside the existing validators.

## S2 inspected baseline (2026-10-14)

| repo | revision |
|---|---|
| rpg-dnd5e-web `feat/studio-s5-density` | 3416490b |
| rpg-toolkit `origin/main` | 2f570aa9 |
| rpg-project `origin/main` | as S1 |

- **Vocabulary.** The condition and word labels live inline in `SitePolicies.tsx` `AnswerEntryRow`: "no enemy is seen or remembered", "an enemy is in reach", "I was attacked", "an ally was attacked", "I heard a sound", "I know a fact", and the word pickers. `entryText` and `predicateText` render plain strings such as "when …", "weight 3" and "fact x" for `AnswerTableReadout` and the `until` and `arrives` readouts.
- **The faction bug.** `CreatureOrders`' Inherits section and `FactionTableEditor` read only `faction.on`. A faction with `table:` and no `on:` reads "No shared table is authored."
- **JSON.** `WorldBuildingConcept.tsx`'s authored-dispositions list renders `<code>{JSON.stringify(entry.until)}</code>`.
- **Toolkit layering.** `inheritedOrders` (`compile.go`) sets a faction's base as `layerSpecs(rootTableOn(tables, fa.Table), fa.On)`. The single-room binding is `layerSpecs(rootTableOn(spec.Tables, b.Table), b.On)` (`single_room_compile.go`). `ordersOf` lays the creature's layer over its faction's, key by key, the nearer winning wholesale. `turnsTo` (`factions.go`) turns hostile and neutral into each other; an allied pair cannot carry `until`.
- **Row order.** `validateAnswerTable` (`answerTableShape.ts`) is the one gate for `tables.<id>`, `factions[].on` and `monsterBindings.<id>.on` (`siteScope.ts`, `roomDraft.ts`). Option outcomes are validated in the options validator.
- **Scope keys.** `templates`, `tables`, `factions`, `dispositions`, `intel`, `exits`, `endings`, `scenarios`, `concealments` (`siteScope.ts` `SCOPE_KEYS`).
- **The castle.** `fixtures/castleWorkspace.ts` is structural: walls, floor and two labels, with no gate guard, cook or tables. No repository holds the walkthrough's castle as a single-room document. rpg-api `content/reference-hearing.yaml` carries `gate-patrol` in the v2 dialect, and the mad-king plan's wave D reference content is not started.
- **Home sections.** `ENCOUNTER_SECTIONS` is Cast, Tables, Factions, Dispositions, Intel.

## S2 sequence

```
S2-T1 vocabulary ──▶ S2-T4 predicate sentences ──▶ S2-T6 read-back ──▶ S2-T8 integration
S2-T2 layers and R12 ──▶ S2-T3 faction readout ─────┤
S2-T5 engine laws ──────────────────────────────────┤
S2-T7 fact index (needs T4 for reader names) ────────┘
```

One branch off `dev`, one PR to `dev`, one builder in this order: T1, T2, T3, T4, T5, T6, T7, T8. T1, T3 and T4 all edit `SitePolicies.tsx`.

## S2 open items

| id | item | state | blocks |
|---|---|---|---|
| S2-Q1 | **Person.** | **lead-resolved 2026-10-14:** one phrase per term with a subject slot; the editor renders "I", the prose "it" | nothing |
| S2-Q2 | **Law numbers.** | **lead-resolved:** the hearing law is stated in words only; sound DCs are content and not on the wire | nothing |
| S2-Q3 | **"When no row holds, it holds its post."** | **lead-resolved:** kept as the law `holds`, sourced to rpg-toolkit `mind/behavior/table.go` `holdPick` (a `time` table with nothing eligible means the creature stands there, and the beat records it; confirmed at `origin/main`) | nothing |
| S2-Q4 | **R12's reach.** | **lead-resolved:** every weighted row list, including option `success` and `failure` rows | nothing |
| S2-Q5 | **Sorting rewrites saved files.** | **lead-resolved:** the next save of an unsorted draft rewrites its row order; weights are unchanged and order carries no meaning. Web goldens are updated as needed. The studio never touches rpg-api content files. | nothing |
| S2-Q6 | **Castle content for the walk.** | **lead-resolved:** S2-T8 authors `fixtures/castleReadBack.ts` from the walkthrough. It is the seed for the mad-king plan's wave D reference content (rpg-project#569, PR #570), and that plan's task D1 owns promoting it to rpg-api content. | nothing |
| S2-Q7 | **Engine law text over the wire.** The S2 contract is client-side text with a toolkit source per law. Receiving law text from the server is a later question. | open, not S2; see design.md Open | nothing in S2 |
| S2-Q8 | **Rulebook catalog over the wire for pick-lists** (S1-Q3). | open; not in S2 | nothing |
---

## Task S2-T1: One vocabulary

**Delivers:** B5, S2-Q1.
**Owner:** rpg-dnd5e-web; new `world-building/answerVocabulary.ts`, `SitePolicies.tsx`.
**Files:** new `answerVocabulary.ts` and its test; `SitePolicies.tsx` (`AnswerEntryRow`'s option labels and `entryText` read the module).
**Interfaces:**

```ts
export type Subject = 'I' | 'it';
export function conditionPhrase(c: AnswerWhenCondition, subject: Subject): string;
// "no enemy is seen or remembered" · "an enemy is in reach" · "{subject} was attacked within 3 rounds"
// · "{subject} heard a clang within 2 rounds" · "{subject} knows alarm-raised" · "not …"
export function wordPhrase(entry: AnswerEntryShape, subject: Subject): string | undefined;
// "attacks the attacker" · "walks toward the sound" · "holds" · "flees" · "raises alarm-raised"
export function triggerPhrase(trigger: string): string;   // "On its time"
```

**Behavior:** the editor's visible text is unchanged; existing tests pin it.
**Tests:** `answerVocabulary.test.ts` gives one case per condition kind and per word, in both subjects. The existing `AnswerEntrySentence.test.tsx` and `SitePolicies.test.tsx` stay green unchanged.
**Verification:** `npx vitest run src/concepts/world-building` passes.

## Task S2-T2: The layers, and heaviest first

**Delivers:** B2 (data), B10, S2-Q4, S2-Q5.
**Owner:** rpg-dnd5e-web; new `world-building/layeredOrders.ts`, `answerTableShape.ts`, the options validator.
**Interfaces:**

```ts
export type OrdersSource =
  | { kind: 'own' } | { kind: 'table'; id: string }
  | { kind: 'faction'; id: string } | { kind: 'faction-table'; faction: string; id: string }
  | { kind: 'kind-default' };
export function layeredOrders(document: Readonly<RoomDraftDocument>, placementId: string):
  Record<string, { source: OrdersSource; rows: readonly AnswerEntryShape[]; sharedWith: readonly string[] }>;
// per trigger key in ANSWER_TRIGGERS; sharedWith = other placements (display names, S5) whose same trigger resolves to the same source
```

- `validateAnswerTable` returns each trigger's rows stable-sorted by `weight ?? 1`, descending. The options validator does the same to `success` and `failure` (S2-Q4).
- Web goldens whose rows move are regenerated. No rpg-api content file is touched (S2-Q5).

**Tests:** `layeredOrders.test.ts`: › `own rows beat the named table key by key`; › `the binding's named table beats the faction's rows`; › `the faction's own rows beat the faction's named table`; › `a trigger no layer names is the kind default`; › `sharedWith lists gate-2 for gate-patrol`. `answerTableShape.test.ts`: › `rows sort heaviest first with ties in authored order`; › `an absent weight counts as 1`. `singleRoomDungeon.test.ts`: › `an unsorted table encodes heaviest first`. › `option success and failure rows sort heaviest first`. Regenerate any web golden whose rows move, and read the diff.
**Verification:** `npx vitest run src/concepts/world-building` passes.

## Task S2-T3: The faction readout reads its named table

**Delivers:** B11.
**Owner:** rpg-dnd5e-web; `SitePolicies.tsx` (`CreatureOrders` Inherits; `FactionTableEditor` and the Factions form readout).
**Behavior:** a faction with `table:` shows "Inherits gate-patrol" and that table's readout under its own rows, with each trigger labelled by its layer, as `layeredOrders` resolves it. "No shared table is authored" appears only when the faction names no table and has no rows.
**Tests:** › `a faction naming a table and no rows shows that table, not "No shared table"`; › `a faction's own time rows are labelled as overriding its table's time rows`; › `a faction naming an undeclared table says so by name`.
**Verification:** `npx vitest run src/concepts/world-building/SitePolicies.test.tsx` passes.

## Task S2-T4: Predicates as sentences

**Delivers:** B12.
**Owner:** rpg-dnd5e-web; new `world-building/predicateSentence.ts`; `SitePolicies.tsx` (`predicateText` callers); `WorldBuildingConcept.tsx` (the JSON line).
**Interfaces:** `predicateSentence(p: PredicateDoc, ctx: { mindName?: string; names: ReadonlyMap<string, string> }): string`.
- fact: "the captain, the garrison's mind, knows *party-recognized*", or "*x* is known" with no mind;
- round: "round 12 begins";
- down: "gate-1 is down";
- stance: "the garrison and the party are hostile".

**Behavior:** a disposition reads "Neutral to the party until …; then hostile", with the turn taken from `engineLaws` (S2-T5). An allied pair never shows `until`, because the decoder refuses it. No `JSON.stringify` of a predicate remains in the studio.
**Tests:** one case per form; › `the authored dispositions list renders no JSON`; `git grep -n "JSON.stringify(entry.until" src/` prints nothing.
**Verification:** `npx vitest run src/concepts/world-building` passes.

## Task S2-T5: Engine laws, with sources

**Delivers:** B4, S2-Q2, S2-Q3.
**Owner:** rpg-dnd5e-web; new `world-building/engineLaws.ts`.
**Interfaces:**

```ts
export interface EngineLaw { id: string; sentence: string; source: string } // "rpg-toolkit rulebooks/dnd5e/encounter/dungeonspec/factions.go turnsTo"
export const ENGINE_LAWS: Readonly<Record<'untilTurns' | 'untilOnMind' | 'provocation' | 'temperAtSpawn' | 'hearing' | 'weights' | 'holds', EngineLaw>>;
```

Sentences, in words, with no rulebook numbers:
- `untilTurns`: when it holds, hostile becomes neutral and neutral becomes hostile.
- `untilOnMind`: a fact `until` is judged on the faction's mind.
- `provocation`: a hostile act against a creature turns its side hostile to the actor, whatever the file says.
- `temperAtSpawn`: a mix is dealt once at spawn.
- `hearing`: a sound is heard when the listener's passive Perception meets its DC, which rises per region hop and per closed door.
- `weights`: among the rows that hold, one is picked in proportion to its weight.
- `holds`: when no `time` row holds, it stands where it is. Source: rpg-toolkit `mind/behavior/table.go` `holdPick`.

**Behavior:** the builder confirms each source exists at rpg-toolkit `origin/main` and records the file and symbol. A law whose source cannot be found is left out.
**Tests:** › `every law has a nonblank source naming an rpg-toolkit path`; › `no law sentence contains a digit`.
**Verification:** `npx vitest run src/concepts/world-building/engineLaws.test.ts` passes, and the PR lists each law's source as confirmed.

## Task S2-T6: The read-back in the inspector

**Delivers:** B1, B2 (prose), B3, B6, B7, B8.
**Owner:** rpg-dnd5e-web; new `world-building/readBack.ts`, new `encounter-studio/ReadBack.tsx`, `StudioInspector.tsx` (`CreatureView`, `PropView`).
**Interfaces:**

```ts
export interface ReadBackParagraph { heading: 'who' | 'on-its-time' | 'spoken-to' | 'knows'; sentences: readonly ReadBackSentence[] }
export type ReadBackSentence = { text: string; refs: readonly ReadBackRef[] }; // refs select things (table, faction, intel, fact, placement)
export function readBackOf(document: Readonly<RoomDraftDocument>,
  selection: { kind: 'creature' | 'prop'; id: string }): readonly ReadBackParagraph[];
```

**Behavior** (creature; a prop has no "who" stance or orders paragraph):
- **Who:** "{name}, a {cast member} of the {faction}", plus "and the {faction}'s mind" when it is. Each disposition naming its faction becomes one sentence, through `predicateSentence`. Then the provocation law, and the temper: its own word, or "dealt at spawn from the {faction}'s mix: soldier 2, coward 1".
- **On its time:** "it follows *gate-patrol*, shared with gate-2", then the weight law or "every row weighted alike". Each row is `conditionPhrase('it')`, then `wordPhrase('it')`, then the `say` line in quotes, then "(weight n)". A trigger the table names says "the kind's default {trigger} rows do not apply". A trigger no layer names says "it answers with the kind's default". A `time` layer ends with the `holds` law: "When no row holds, it stands where it is."
- **Spoken to:** "Nothing can be said to it. It has no options", or one bullet per option: the line in quotes; each check as "{Skill} DC n"; the gate ("Offered to a speaker who knows *x*", or "… carrying the {prop name}"); "Success: …" and "Failure: …" each as their lines and facts.
- **Knows:** "It knows nothing at spawn; it carries no intel", or "It carries {record}, which reveals *x*". Then "It writes *a*, *b*; it reads *c*", from its rows, options and `arrives`.
- Arrives: "It is held in reserve until …" through `predicateSentence`.
- The prose sits at the top of the view, above S5's Placement section, inside a `StudioSection` titled "Read back". A ref in a sentence is a link that selects or opens its target.

**Tests:** `readBack.test.ts` over the S2-T8 fixture: › `gate-1 follows gate-patrol shared with gate-2`; › `gate-1's rows read in the editor's vocabulary with their weights`; › `the cook answers with the kind's default`; › `gate-1's time paragraph ends with the holds law`; › `the cook's three options read with check, gate and both outcomes`; › `the garrison's disposition reads until the captain knows party-recognized, then hostile`; › `a creature with no options says nothing can be said to it`; › `a prop that holds intel says what it reveals`. `StudioInspector.test.tsx`: › `the creature view opens with the read-back above Placement`.
**Verification:** `npx vitest run src/concepts` passes.

## Task S2-T7: The fact index

**Delivers:** B9.
**Owner:** rpg-dnd5e-web; new `world-building/factIndex.ts`, new `encounter-studio/StudioFacts.tsx`, `EncounterStudioWorkspace.tsx`.
**Interfaces:**

```ts
export interface FactUse { kind: 'option' | 'table-row' | 'intel' | 'disposition' | 'arrives' | 'ending' | 'requires';
  label: string; target: StudioSelection | { home: 'tables' | 'factions' | 'dispositions' | 'intel'; id: string } }
export function factIndexOf(document: Readonly<RoomDraftDocument>):
  readonly { fact: string; writers: readonly FactUse[]; readers: readonly FactUse[] }[]; // sorted by fact id
```

- Writers: option `success` and `failure` rows with `fact` (creature and prop), table and `on` rows with `fact`, intel `reveals` with a fact.
- Readers: option `requires`, table and `on` `when` with `fact`, disposition `until`, binding `arrives`, ending `when`, at any nesting.
- `ENCOUNTER_SECTIONS` gains `facts: 'Facts'`, after Intel.

**Behavior:** each row reads "fact · written by … · read by …". A row with no readers is marked "written, never read"; one with no writers, "read, never written". Clicking a use selects its placement, or opens its home section on that id. Nothing is stored.
**Tests:** › `feast-tonight lists the cook's gossip and bribe as writers and ashford and the captain's troupe as readers` (fixture); › `king-is-cultist is marked written, never read`; › `a fact read only by an until is marked read, never written`; › `nested not and list conditions are found`; › `clicking a writer selects the cook`.
**Verification:** `npx vitest run src/concepts` passes.

## Task S2-T8: The castle fixture, integration and the walk

**Delivers:** proof of B1–B12; S2-Q6.
- [ ] Author `encounter-studio/fixtures/castleReadBack.ts` from README.md. It seeds the mad-king plan's wave D reference content, and that plan's task D1 promotes it to rpg-api content (S2-Q6). It holds gate-1 and gate-2 on `gate-patrol`; the garrison with its mind the captain, temper mix soldier 2 and coward 1, and the two dispositions; the cook with its three options; the intel `cooks-ledger`; and the altar's examine option writing `king-is-cultist`.
- [ ] Throwaway toolkit decode of the fixture's encoded file in a scratch worktree, as S1 did; record the output; commit nothing to the toolkit.
- [ ] `npm ci`; `npm run ci-check` once; grep the log.
- [ ] Walk with the fixture as the local draft, seeded by S5-Q6's path. Screenshot each step.
- [ ] Post the checklist below on #576 as `- [ ]` boxes before the PR leaves draft.

**Walk and #576 checklist:**

1. Select gate-1: the read-back sits above Placement. Compare it with the walkthrough's gate-1 prose sentence by sentence, and list every wording difference in the PR. Weights read as numbers, under one weight-law sentence.
2. Select the cook: it answers with the kind's default, and its three options read with checks, gates and outcomes. Compare with the walkthrough's cook prose the same way.
3. Open Facts: feast-tonight, king-burned-village, alarm-raised and king-is-cultist match the walkthrough's rows, and king-is-cultist is marked. Click a writer: that thing is selected.
4. In Tables, give a row weight 5: it moves to the top, in the form and in gate-1's read-back. Save and reload: it is first in the encoded file.
5. Factions: the garrison shows gate-patrol inherited, not "No shared table".
6. Dispositions and the creature view: every `until` reads as a sentence; no JSON anywhere.
7. Undo after the weight change restores the order; the read-back follows.

## S2 requirement coverage

| requirement | task(s) | proof |
|---|---|---|
| B1 prose heads the view | S2-T6 | `the creature view opens with the read-back above Placement`; walk 1 |
| B2 layers | S2-T2, T6 | the `layeredOrders` tests; `gate-1 follows gate-patrol shared with gate-2` |
| B3 kind's default stated | S2-T2, T6 | `the cook answers with the kind's default` |
| B4 laws with sources | S2-T5, T6 | `every law has a … source`; the PR's source list |
| B5 one vocabulary | S2-T1 | `answerVocabulary.test.ts`; editor tests unchanged |
| B6 weights | S2-T6 | `gate-1's rows read … with their weights`; walk 1 |
| B7 options | S2-T6 | `the cook's three options read …`; walk 2 |
| B8 knows | S2-T6 | `a prop that holds intel says what it reveals`; the knows sentences |
| B9 fact index | S2-T7 | the fact-index tests; walk 3 |
| B10 heaviest first | S2-T2 | `rows sort heaviest first …`; `an unsorted table encodes heaviest first`; walk 4 |
| B11 faction named table | S2-T3 | `a faction naming a table and no rows shows that table …`; walk 5 |
| B12 sentences, not JSON | S2-T4 | the predicate tests; the grep; walk 6 |

## S2 provider/consumer seams

| provider | consumer | contract | availability | proof |
|---|---|---|---|---|
| S2-T1 `answerVocabulary` | `AnswerEntryRow`, `entryText`, S2-T6 | one phrase, subject slot | new | its tests; editor tests unchanged |
| S2-T2 `layeredOrders` | S2-T3, T6, T7 | per-trigger source and rows; mirrors toolkit layering | new; toolkit at 2f570aa9 | the layering tests |
| S2-T2 sorted `validateAnswerTable` | every editor, encoder and readout | rows heaviest first, stable | changes an existing gate | the sort and codec tests |
| S2-T4 `predicateSentence` | dispositions, `arrives`, endings, S2-T6, T7 | a sentence per predicate form | new | the predicate tests |
| S2-T5 `ENGINE_LAWS` | S2-T4, T6 | law sentences with sources | new | the laws tests |
| S2-T6 `readBackOf` | `ReadBack` in the creature and prop views | paragraphs with selectable refs | new | `readBack.test.ts` |
| S2-T7 `factIndexOf` | `StudioFacts` | derived rows with uses | new | the fact-index tests |
| S5 `StudioSection`, `placementDisplayNames` | S2-T6, T7 | section layout; display names | S5 (lands first) | S5's tests |
| encoded file | rpg-toolkit decoder | unchanged keys, rows reordered | baseline | the T8 throwaway decode |

**Shared files.** `SitePolicies.tsx` is edited by T1, T3 and T4, in that order.

---

# Encounter studio S3 — the overview on Layout

**S3 SHIPPED 2026-10-14:** web dev c573e6cc (rpg-dnd5e-web#1281); follow-up: a prop footprint can cover a label.

Planned 2026-10-14 against the S2 branch. S3 builds after S5 and S2 merge,
and branches from `dev` then. Authority: design.md R7; README.md's Layout
overview row and the overview refusal ("a region fill is an annotation");
rpg-project#577 and its comment that S3 retires the Regions home; S0's Q5
resolution. The operator's reference picture of the keep is a visual target
only, recorded with the keep scout's comparison. It is not a specification.

## S3 goal and scope

Layout shows the whole encounter. It draws one colour per region with its
label, door markers, and every placed noun (creatures in rings coloured by
cast member, with their names; props with their footprints; the party
start). A room list beside the map frames a region when selected and gives
its size. A footer counts areas, creatures, hexes and doors, and "Show
entire" frames the whole keep. Clicking a drawn thing selects it, as in 3D.
The Regions home retires, and its forms move into the inspector and the room
list.

**Repositories.** Web only, rpg-dnd5e-web, from `dev` after S5 and S2. No
toolkit, API or proto change; no document key. Colours are derived, never
stored.

**Non-goals:** multi-room sites and shared-wall doors (rpg-project#528);
deriving membership from a fill; editing in the overview beyond today's
gestures. 3D region labels are an optional last task, because they are cheap
(S3-T7).

## S3 requirement labels

| label | requirement | source |
|---|---|---|
| V1 | One colour per region, derived and stable, with the region's label drawn in it. | R7 |
| V2 | Placed nouns drawn in 2D: creatures as rings in their cast member's colour with their display name, props as their footprint (declared) or a marker (undeclared) with their label, the party start, and doors as today. | R7 |
| V3 | A room list of the regions. Selecting one selects its label in the inspector and frames it; each row shows its size in hexes. | R7 |
| V4 | "Show entire" frames the whole workspace's content. | lead brief |
| V5 | A footer: areas · placed creatures · walkable hexes · doors. | lead brief |
| V6 | Clicking a drawn creature, prop or party start in Layout selects it through the S0 contract. Shift on a prop extends the pick, as in 3D. | S0, R9 |
| V7 | The Regions home retires. Region list and New region go to the room list. Boundary repair, explicit-area painting, background light, room wall finish and Remove region go to the inspector's label view for a region label. The hex view's Edit region selects that label instead of switching home. | #577 comment; S0-Q5 |
| V8 | Pan and zoom on the 73 × 48 castle fixture (3,504 hexes) stay usable, measured before and after. | lead brief |
| V9 | A fill never means membership. No rule, read-back or count calls a creature "in" a region because it sits inside a colour. | README refusal |

## S3 constraints

- **No document key.** `AuthoringRegion` has no colour field (`authoringRegions.ts`), and no other presentation colour exists for regions or cast members. Colours are derived from ids (S3-Q1).
- **The S0 selection contract.** Layout clicks call the existing intents: `creatures.select(id)`, `viewportProps.onSelect([id])` for props, the party-start target, `doorEditing.select`, and `mapLabelSelection.select`.
- **One render per layer.** New layers are memoised on their inputs, as `LayoutGrid` is, so pan and zoom do not re-render them.
- **Region size comes from the resolution, never from geometry the studio guesses.** A polygon region counts the workspace cells whose centre is inside its ring, the rule `regionsAtCell` already uses. A hex-union counts its cells.

## S3 inspected baseline (2026-10-14)

| repo | revision |
|---|---|
| rpg-dnd5e-web `feat/studio-s2-readback` | 6abde8ac |

- **Layout** (`LayoutViewport.tsx`) is one SVG. `LayoutGrid` draws one memoised `polygon` per workspace cell. `RegionBoundaryOverlay` draws each resolved region's ring or cells as a dashed outline with a faint fill, highlighting the selected one. `LayoutWallOverlay` draws walls and doors. `MapLabelOverlay` draws labels. No creature, prop or party start is drawn in Layout.
- **The Regions home** (`EncounterStudioWorkspace.tsx`, `home === 'regions'`) holds the following:
  - `StudioRegions` (the region list, "finish unapplied" status, New region);
  - the region label's `StudioArrangePanel`, with its boundary-repair and background-light fields from `StudioArrangeFields`, `onDefineRegion`, and `StudioRegionFinishes`;
  - the room-label `labels.controls` (`placementKind: 'room'`);
  - the explicit-area tool group (Paint, Erase, Rectangle region; Clear explicit area; Done).
  - The inspector's `HexView` sends Edit region to `changeHome('regions')`.
- **Colours.** `AuthoringRegion` is `{ id, labelId, lighting?, wallFinish?, boundary }`, with no colour. The only colour field in the scene is `WorldPointLight.color`.
- **3D labels.** `RoomActorMarkers.tsx` already draws actor chips with drei `Html`. 3D draws no region label.
- **Footprints.** `RoomPropDeclaration.footprint` (`roomDraft.ts`) is the authored footprint of a declared prop. Undeclared props have only a transform.
- **Framing.** `StudioDimensions` frames the committed workspace through `workspaceBounds`.

## S3 sequence

```
S3-T1 sizes, colours and region fills ──▶ S3-T2 placed nouns ──▶ S3-T3 room list, Show entire, footer ──▶ S3-T4 Regions home retires ──▶ S3-T6 integration
S3-T5 frame-time driver (before set, on dev, first) ──────────────────────────────────────────────────────────────────────────────┘
S3-T7 3D region labels (optional) ──▶ S3-T6
```

One branch off `dev`, one PR to `dev`. T5's before set is taken on `dev` before T1 starts. T1–T4 edit `LayoutViewport.tsx` and `EncounterStudioWorkspace.tsx` in order, by one builder.

## S3 open items

| id | item | state | blocks |
|---|---|---|---|
| S3-Q1 | **How colours are derived.** | **lead-resolved 2026-10-14:** each region's slot starts at a stable hash of its id in a 12-colour palette. Then the regions are walked in authored order, and a region whose slot equals an already-assigned neighbour's steps to the next free slot. Neighbours are regions whose counted cells touch or overlap. Cast members use a second palette, hashed by cast ref, with no adjacency step. A delete leaves other colours unchanged unless it removes a collision another region had stepped around. | nothing |
| S3-Q2 | **Undeclared props.** | **lead-resolved:** a marker with the label; declared props draw their footprint | nothing |
| S3-Q3 | **"Usable" frame time.** | **lead-resolved:** the after median stays within 16.7 ms on the walk machine; the after 95th percentile is no worse than 1.25 times the before. The before numbers go in the PR body. | nothing |
| S3-Q4 | **Region size.** | **lead-resolved:** both, as "27 hexes · 9 × 4": the hex count, then the bounding width × height in hexes (odd-r columns × rows of the counted cells); built in S3-T1 | nothing |
| S3-Q5 | **Overlapping regions.** | **lead-resolved:** both translucent, in authored order | nothing |
---

## Task S3-T1: Region colours and fills

**Delivers:** V1, V9, S3-Q1, S3-Q5.
**Owner:** rpg-dnd5e-web; new `world-building/regionSize.ts`; new `encounter-studio/overviewColours.ts`; `RegionBoundaryOverlay.tsx`; `MapLabelOverlay.tsx`.
**Interfaces:**

```ts
export function regionColours(input: {
  regions: readonly AuthoringRegion[];                 // authored order
  cellsOf(regionId: string): readonly RoomHexCell[];   // the S3-T3 counted cells
}): ReadonlyMap<string, string>;  // hash slot, then step past an already-assigned neighbour's slot
export function castColour(ref: string): string;       // stable hash, own palette
export function regionSize(draft, resolution):
  { cells: readonly RoomHexCell[]; hexes: number; width: number; height: number } | null;
// null when unresolved; width and height are the odd-r column and row spans of the counted cells
```
**Behavior:** each resolved region fills in its colour at a fixed opacity, with its outline in the same hue. The selected region's outline thickens. A region label is drawn in its region's colour. Unresolved regions are drawn as today, with no fill.
**Tests:** › `a hex-union region's size is its cell count`; › `a polygon region's size counts cells whose centre is inside`; › `a region's width and height are its column and row spans`; › `the same regions always get the same colours`; › `two adjacent regions never share a slot on the castle fixture`; › `a collision steps to the next free slot in authored order`; › `deleting a region with no collisions leaves the others' colours unchanged`; › `a resolved region renders a fill in its colour`; › `an unresolved region renders no fill`.
**Verification:** `npx vitest run src/concepts/encounter-studio` passes.

## Task S3-T2: Placed nouns in 2D

**Delivers:** V2, V6, S3-Q2.
**Owner:** rpg-dnd5e-web; new `encounter-studio/LayoutPlacedOverlay.tsx`; `LayoutViewport.tsx`; `studioSession.ts` (`LayoutViewportProps` gains `placedEditing`).
**Interfaces:**

```ts
interface LayoutPlacedEditing {
  selected: StudioSelection;
  selectCreature(id: string): boolean;       // session.creatures.select
  selectProp(id: string, extend: boolean): void; // viewportProps.onSelect([id]) or extend the pick
  selectPartyStart(): boolean;               // the existing start target
}
```

**Behavior:**
- Creatures are a ring at their starting cell in `castColour`, with the display name from S5's `placementDisplayNames` below. A ring is highlighted when selected, or when its cast member is selected (S1).
- Declared props draw their footprint, rotated by their transform; undeclared props draw a marker. Each shows its label at a zoom where it fits.
- The party start draws a distinct marker.
- In the select tool, a click on a glyph selects it, and Shift on a prop extends the pick. Glyphs ignore clicks under any other tool, so painting is unchanged.
- The layer is memoised on the draft's declarations, scene items and selection.

**Tests:** `LayoutPlacedOverlay.test.tsx` › `a creature ring sits at its cell in its cast colour`; › `clicking a ring selects that creature`; › `clicking a prop footprint selects the prop and shift extends the pick`; › `the party start marker selects the party start`; › `the paint tool ignores glyph clicks`.
**Verification:** `npx vitest run src/concepts/encounter-studio` passes.

## Task S3-T3: Room list, Show entire and the footer

**Delivers:** V3, V4, V5, S3-Q4.
**Owner:** rpg-dnd5e-web; new `encounter-studio/LayoutRoomList.tsx`; `EncounterStudioWorkspace.tsx` (Layout gains a left `aside`).
**Interfaces:** consumes S3-T1's `regionSize`. `frameOf(bounds, viewport): LayoutFrame`, reusing the framing `StudioDimensions` uses.
**Behavior:**
- The room list shows each region's label, colour swatch and size as "27 hexes · 9 × 4", or "unresolved". Clicking a row selects its label through `mapLabelSelection.select`, opening the label view (S3-T4), and frames the region's bounds.
- New region arms the room-label tool exactly as the Regions home does today.
- "Show entire" frames the bounds of the walkable floor, walls and placed nouns, or the workspace when empty.
- The footer reads "{areas} areas · {creatures} creatures · {hexes} hexes · {doors} doors". Areas counts regions, hexes counts walkable cells, and doors counts wall openings with a door.

**Tests:** › `selecting a room selects its label and frames it`; › `Show entire frames the content bounds`; › `the footer counts areas, creatures, hexes and doors`.
**Verification:** `npx vitest run src/concepts` passes.

## Task S3-T4: The Regions home retires

**Delivers:** V7.
**Owner:** rpg-dnd5e-web; `EncounterStudioWorkspace.tsx`, `StudioInspector.tsx` (`LabelView`, `HexView`), `studioSession.ts` (`StudioHome`), `StudioRegions.tsx`, `StudioArrangePanel.tsx`.
**Behavior:**
- `StudioHome` becomes `'build' | 'encounter'`, and the home nav shows Build and Encounter.
- The label view of a region label gains a "Region" section, holding exactly today's region-label fields:
  - boundary mode, with Use enclosing walls;
  - Define explicit area, which arms the region tool in Layout, with the Paint, Erase and Rectangle region group shown as an armed tool (S0 A2);
  - Clear explicit area;
  - background light (%);
  - the room wall finish (`StudioRegionFinishes`);
  - Remove region.
- The hex view's Edit region selects the label and stays on Build.
- `StudioRegions.tsx` is removed once the room list replaces it. Every `home === 'regions'` branch is removed.

**Tests:** › `the home nav shows Build and Encounter only`; › `a region label's view edits background light through one intent`; › `Define explicit area arms the region tool in Layout`; › `the hex view's Edit region selects the label without changing home`; › `Remove region removes the region and its label`. Each existing Regions-home test is retargeted to the label view and keeps its document assertion.
**Verification:** `npx vitest run src/concepts` passes; `git grep -n "'regions'" src/concepts/encounter-studio` prints nothing.

## Task S3-T5: The frame-time driver

**Delivers:** V8, S3-Q3.
**Owner:** game-dev, `tools/browser/_job_s3_overview.mjs` (local by name).
**Behavior:** the driver seeds the castle fixture by the S5-Q6 path and opens Layout at 1280×800. It scripts a 3-second pan and a zoom in and out, and records frame intervals with `requestAnimationFrame`. It prints the median and 95th percentile. It runs on `dev` before T1 (the before set) and on the branch after T4 (the after set). It also takes the walk's screenshots.
**Verification:** the before numbers are in the PR body from the first push. The after numbers are added beside them and meet S3-Q3.

## Task S3-T6: Integration and the walk

**Prerequisites:** S3-T1–T5. S3-T7 is built last, and only if T1–T6 are green.
- [ ] `npm ci`; `npm run ci-check` once; grep the log.
- [ ] Run the S3-T5 driver for the after set.
- [ ] Post the checklist below on #577 as `- [ ]` boxes before the PR leaves draft.

**Walk and #577 checklist** (castle fixture as the local draft):

1. Layout, Show entire: the whole keep is framed. Each region is filled in its own colour with its label, and no two neighbouring regions share a colour.
2. Doors, creature rings with names, prop footprints and the party start are all drawn.
3. The footer reads areas · creatures · hexes · doors, and matches the fixture.
4. Select each region in the room list. Each one is framed, its size reads like "27 hexes · 9 × 4", and the inspector shows its label view with the Region section.
5. Click a creature ring in 2D. The inspector opens on it, the same view as clicking it in 3D.
6. Click a prop, then Shift-click a second: the pick strip. Click the party start: its view.
7. The home nav shows Build and Encounter, with no Regions. Change a region's background light from its label view, then undo.
8. The frame-time numbers before and after meet S3-Q3.
9. (S3-T7 only) In 3D, each region's label floats over it.

## Task S3-T7 (optional): Region labels in 3D

**Delivers:** an S3 extra, not a ruling.
**Owner:** rpg-dnd5e-web; `world-building/WorldBuildingViewport.tsx`.
**Behavior:** one drei `Html` label per resolved region, at its label's world point, in its `regionColour`. It is pointer-transparent, like the actor chips.
**Cost:** one `Html` per region, following the actor-chip pattern. It is cheap enough to include last.
**Tests:** › `each resolved region renders one 3D label`.

## S3 requirement coverage

| requirement | task(s) | proof |
|---|---|---|
| V1 region colours | S3-T1 | the colour tests; walk 1 |
| V2 placed nouns | S3-T2 | the overlay tests; walk 2 |
| V3 room list | S3-T3 | `selecting a room selects its label and frames it`; the size tests; walk 4 |
| V4 Show entire | S3-T3 | `Show entire frames the content bounds`; walk 1 |
| V5 footer | S3-T3 | `the footer counts …`; walk 3 |
| V6 select in 2D | S3-T2 | `clicking a ring selects that creature`; walk 5–6 |
| V7 Regions home retires | S3-T4 | the T4 tests; the grep; walk 7 |
| V8 frame time | S3-T5, T6 | the before and after numbers; walk 8 |
| V9 fill is not membership | S3-T1, T3 | no count uses fills: the footer counts regions, not occupants; code review |

## S3 provider/consumer seams

| provider | consumer | contract | availability | proof |
|---|---|---|---|---|
| S3-T1 `regionColours`, `castColour` | region fills, labels, rings, the room list, S3-T7 | stable colour per id | new | the colour tests |
| S3-T2 `LayoutPlacedEditing` | `LayoutPlacedOverlay` | existing selection intents | the intents exist from S0 and S1 | the click tests |
| S3-T1 `regionSize` | the room list; S3-T1's neighbour step | counted cells, hexes, width, height; the count rule `regionsAtCell` uses | new, over S0's `pointInRing` use | the size tests |
| S0 `StudioSelection` and the label view | S3-T4 Region section | label selection | S0 | the T4 tests |
| S5 `placementDisplayNames` | ring labels | display names | S5 | S5 tests |
| S1 cast selection | ring highlight | `castMember` selection | S1 | `… highlighted when its cast member is selected` |

**Shared files.** `LayoutViewport.tsx` (T1, T2) and `EncounterStudioWorkspace.tsx` (T3, T4) are edited in order by one builder.

---

# Encounter studio S4 — the hidden nouns, and the draft as a file

**S4 SHIPPED 2026-10-14:** web dev e623c2b3 (rpg-dnd5e-web#1282); follow-ups: a concealment as a selection kind; list every import refusal.

Planned 2026-10-14. The S3 branch (`feat/studio-s3-overview`) was not yet
pushed when this was planned, so the baseline is the S2 branch at 6abde8ac,
which S3 stacks on. S4 builds after S3 merges and branches from `dev` then.
The builder re-checks every path below against `dev` first. Authority:
design.md R8, with R1 and R2 for placement; README.md's door, floor-hex and
"Endings and concealments" forms; rpg-project#578 and the operator's scope
addition on it (export and import of the local draft as a file).

## S4 goal and scope

Every saved noun has a studio view. A door's inspector edits open, closed or
locked, its lock's checks, and the concealment that hides it. A floor hex's
inspector edits its exit. The Encounter home gains Endings and Concealments,
and shows Scenarios read-only. The author can export the draft as the file
the decoder accepts, and import one. Import refuses what the decoder
refuses, naming the field.

**Repositories.** Web only, rpg-dnd5e-web, from `dev` after S3. No toolkit,
API or proto change. No new document key: `doorBindings`, `exits`,
`endings`, `concealments` and `scenarios` already exist and survive save.

**Non-goals:** Save & Play (rpg-dnd5e-web#1236), opening a reference
encounter from the API, multi-room sites (rpg-project#528), and editing
scenario bindings.

## S4 requirement labels

| label | requirement | source |
|---|---|---|
| H1 | The door view edits the state: open, closed or locked. Locked carries one or more lock checks (ability, optional tool, DC); any one beats the lock. | R8; README door form |
| H2 | The door view's Hidden by picks the concealment whose `props` list the door. Hiding a door never hides its wall or floor. | R8; epic #545 boundary |
| H3 | The hex view edits the hex's exit: add one with an id, rename it, remove it. | R8; README floor-hex form |
| H4 | Endings on the Encounter home: an id and a `when` predicate, edited with the same predicate vocabulary as `until` and `arrives`. | R8, R2 |
| H5 | Concealments on the Encounter home: an id; notice and search checks; attempts (max, reset hexes); members (cells painted, props and doors clicked on the map); and a read-only "revealed by" list of intel. | R8, R2 |
| H6 | Scenarios on the Encounter home are read-only: each scenario id and its field → id bindings. | lead brief |
| H7 | Export writes the exact bytes `encodeSingleRoomDungeon` produces, as a `.yaml` file named by its dungeon key. | #578 scope addition |
| H8 | Import decodes through the existing decoder. A refusal names its field and changes nothing. A v2 file is refused with one plain sentence naming the dialect. An accepted import replaces the draft in one undoable step. | #578 scope addition |
| H9 | The fact index's ending readers link to the new Endings form. Concealments carry no fact, so their reveal links appear on the concealment form. | lead brief; S2-T7 |
| H10 | The studio never offers a field the toolkit refuses (S4-Q1). | design Law: the document's refusals |

## S4 constraints

- **No new key.** Every form writes a key the dialect declares, confirmed against rpg-toolkit `single_room.go`. That file has `RoomDoorBinding { closed, locked: CheckSpec }`, `RoomExit { id, cell }`, `EndingSpec { id, when }`, `ConcealmentSpec { attempts { max, reset_hexes }, notice, checks, cells, props }`, and `scenarios: map[string]map[string]string`.
- **One predicate grammar.** The predicate grammar has exactly one key per predicate: fact, round, down or stance (`predicate.go`). An ending's `when` refuses `holds`, as the decoder does. "Either" is two endings, because any ending ends the run.
- **One vocabulary.** Checks pick abilities from the same `APPROACH_ABILITIES` list the option checks use, and predicates read through S2's `predicateSentence`.
- **The decoder is the gate.** Import uses `readSingleRoomDungeon` and `decodeSingleRoomDungeon`, then the owner's `importCanonicalRoomYaml`. No second parser.
- **Reuse field lists, not UI.** `DoorStates.tsx`, `ConcealmentPanel.tsx` and `src/author/EndingsPanel.tsx` supply the fields and edit functions (`doorBindingEdits.ts`, `concealmentEdits.ts`), and the studio builds its own views.

## S4 inspected baseline (2026-10-14)

| repo | revision |
|---|---|
| rpg-dnd5e-web `feat/studio-s2-readback` | 6abde8ac (S3 not yet pushed) |
| rpg-toolkit `origin/main` | 2f570aa9 |
| rpg-api `origin/dev` | registry reads `content/*.yaml` (`internal/dungeons/registry.go`) |

- **Door state.** `DoorStates.tsx` renders open, closed or locked with lock rows over `doorBindingEdits.ts`: `setDoorBindingState`, `addDoorApproach`, `patchDoorApproach`, `removeDoorApproach` and `normalizeDoorBinding`. It is mounted only on the non-studio path.
- **Concealments.** `ConcealmentPanel.tsx`, over `concealmentEdits.ts`, adds members by clicking hexes, walls, doors and props while `activeConcealmentId` is armed in the owner. It is mounted only on the non-studio path.
- **Endings.** `src/author/EndingsPanel.tsx` `EndingsSection` edits an id and a predicate. The studio has no endings view. S2's `factIndex.ts` already lists ending `when` as a reader, and its comment notes that endings have no studio view yet.
- **Exits.** `SiteExit { id, cell }` in `siteScope.ts`; the studio has no exit view. The toolkit requires the cell to be standable (`RoomExit` doc).
- **Concealment members.** `ConcealmentSpec` (`single_room.go`) accepts walkable cells and, in `props`, declared prop ids, attached door ids and structural wall ids, each in at most one concealment.
- **Exits at compile.** An exit cell that is not standable decodes, and is refused at compile (S4-Q4).
- **The attempts mismatch.** The web's `SiteDiscoveryAttempts` has `max`, `reset_hexes` and `lifetime`. The toolkit's `DiscoveryAttemptsSpec` has `max` and `reset_hexes` only. A throwaway decode at rpg-toolkit `origin/main`, in a scratch worktree since removed, added `attempts: { max: 2, lifetime: run }` to `testdata/world-builder-v4-tomb-vault.yaml`. The result: `concealments.vault.attempts.lifetime: "lifetime" is not a key this build reads: they are max, reset_hexes`. The control `{ max: 2, reset_hexes: 3 }` decoded. So the web accepts a key the toolkit refuses (S4-Q1).
- **Export and import exist in the World Builder.** `RoomPublishingPanel.tsx` has a "Canonical YAML" disclosure that downloads the encoder's YAML as `single-room-dungeon.yaml`, and imports pasted YAML. Import goes through `useRoomPublishing` `importYaml`, then `decodeSingleRoomDungeon`, then the owner's `importCanonicalRoomYaml`, which pushes the old document onto history. The dungeon key lives in `useRoomPublishing`'s state, inside that panel. The studio mounts neither.
- **Dialects.** `readSingleRoomDungeon` returns `{ dialect: 'other', version }` below version 3. The decoder throws one sentence for the first refusal, naming its field (for example, "Unsupported single-room field: x").
- **Format.** rpg-api's registry stores and loads `<key>.yaml` (`internal/dungeons/registry.go`). Publishing sends the encoder's YAML. JSON is not a registry format.

**Export format: YAML.** It is the one format the API registry stores and
publishing sends, the encoder already produces it, and the decoder reads
it. A JSON export would be a second format with no reader on the server.

## S4 sequence

```
S4-T1 door view ──▶ S4-T4 concealments ──▶ S4-T7 integration
S4-T2 exits on the hex view ─────────────┤
S4-T3 endings ───────────────────────────┤
S4-T5 scenarios read-only ───────────────┤
S4-T6 export and import ─────────────────┘
```

One branch off `dev`, one PR to `dev`. T1 goes before T4, because Hidden by and concealment membership meet on `concealments.<id>.props`. `EncounterStudioWorkspace.tsx` is edited by T3, T4, T5 and T6 in that order.

## S4 open items

| id | item | state | blocks |
|---|---|---|---|
| S4-Q1 | **`attempts.lifetime`.** The toolkit refuses it (evidence above), and the document's refusals are the studio's. | **lead-resolved 2026-10-14:** the web validator refuses `lifetime` with the toolkit's sentence; no toolkit change. A stored draft that carries it opens with the key stripped. A one-line notice names the concealment: "lifetime is not read by the engine; removed from {id}, attempts keep max and reset_hexes". **Lead-accepted deviation:** the strip is a load migration, not an undoable edit. A document carrying `lifetime` cannot pass the gate every commit and save goes through, so an undo entry restoring it would hold a draft that can never be saved. | S4-T4 |
| S4-Q2 | **The dungeon key for export.** | **lead-resolved:** the last imported file's key for this draft, else a slug of the draft's name, under the encoder's key rule. It is held in studio state and is never a document field. | nothing |
| S4-Q3 | **"At the field" for an import refusal.** | **lead-resolved:** the first refusal with its YAML path is enough for S4. Listing every refusal is a follow-up noted on #578. | nothing |
| S4-Q4 | **Exits only on walkable hexes.** | **lead-resolved, verified:** a throwaway test at rpg-toolkit `origin/main`, in a scratch worktree since removed, moved `testdata/world-builder-v4-tomb-vault.yaml`'s exit to `{ q: 40, r: 40 }`. It decoded, and `CompileSingleRoom` refused it with `exits[0].cell: at author's axial q=40 r=40: is not standable`. The control on its own cell decoded and compiled. Add exit is offered on walkable hexes, a document fact the web already holds; see S4-Q6. | nothing |
| S4-Q6 | **Standability in the client.** The toolkit decides standability (`encounter/cellfacts.go` `ValidateStaticPlacements`). | **ruled by the team lead 2026-10-14:** no engine rule in the client. Add exit is offered on walkable hexes. An exit under a blocking prop is refused by the toolkit at compile and surfaces at Save & Play; the exit form's help says so in one line. | nothing |
| S4-Q5 | **Concealment member kinds.** | **lead-resolved, verified against `single_room.go` `ConcealmentSpec`:** `cells` must be `walkableHexes` cells, with no cell in two concealments. `props` takes a declared prop id (`propDeclarations`), an attached door id, or a structural wall id, with no id in two concealments. Walls stay. An undeclared scene prop is not offered. | S4-T4 |
---

## Task S4-T1: The door view

**Delivers:** H1, H2.
**Owner:** rpg-dnd5e-web; `StudioInspector.tsx` (`DoorView`), `studioSession.ts`, `WorldBuildingConcept.tsx`.
**Files:** as Owner; tests in `StudioInspector.test.tsx` and `WorldBuildingConcept.test.tsx`.
**Interfaces:** the session gains:
- `doors.setBinding(doorId: string, next: RoomDoorBinding | undefined): boolean`, one transaction through `normalizeDoorBinding` and `withDoorBinding`;
- `concealments.setDoorHiddenBy(doorId: string, concealmentId: string | null): boolean`, which removes the door id from every concealment's `props` and adds it to the chosen one, in one transaction.

**Behavior:**
- State is a pick-list: open, closed, locked. It applies on change, per S5-Q1.
- Locked shows lock rows: ability from `APPROACH_ABILITIES`, an optional tool ref, and a DC. Add and Remove call the existing edits. A locked door is shut by definition, as `DoorStates` says.
- Hidden by lists the concealments, plus "not hidden".
- The wall and the floor are never added to the concealment by this field.

**Tests:** › `setting locked writes a lock check and one history entry`; › `removing the last lock check returns the door to closed`; › `Hidden by moves the door between concealments in one step`; › `hiding a door leaves its wall out of the concealment`.
**Verification:** `npx vitest run src/concepts` passes.

## Task S4-T2: Exits on the hex view

**Delivers:** H3, S4-Q4.
**Owner:** rpg-dnd5e-web; `StudioInspector.tsx` (`HexView`); `studioSession.ts` (`exits.set(cell, id | null): boolean`).
**Behavior:** a walkable hex with no exit shows Add exit, which asks for an id. The exit form's help (S5 `FieldHelp`) reads: "An exit under a blocking prop is refused when the encounter is played." A hex with an exit shows its id, with rename and Remove. Ids are unique, and a duplicate is refused at the field, using the validator's sentence. An unwalkable hex shows no Add exit. A walkable hex under a blocking prop is refused by the toolkit at compile (`exits[i].cell: … is not standable`, S4-Q4), surfaced at Save & Play; the studio adds no check for it (S4-Q6). The Layout overview draws an exit marker at its cell, using the S3 placed-overlay layer.
**Tests:** › `adding an exit writes exits[] with its cell`; › `a duplicate exit id is refused at the field`; › `an unwalkable hex offers no exit`; › `the exit form's help names the blocking-prop refusal`; › `the exit marker draws at its cell`.
**Verification:** `npx vitest run src/concepts` passes.

## Task S4-T3: Endings on the Encounter home

**Delivers:** H4, H9.
**Owner:** rpg-dnd5e-web; new `encounter-studio/StudioEndings.tsx`; new `world-building/PredicateEditor.tsx`, extracted from `SitePolicies.tsx`'s `until` editor and `ArrivesEditor`; `EncounterStudioWorkspace.tsx` (`ENCOUNTER_SECTIONS` gains `endings`); `factIndex.ts`.
**Interfaces:** `PredicateEditor({ value, onChange, forms })`, where `forms` lists the allowed predicate forms. Endings pass fact, round, down and stance, never `holds`. Each value reads back through `predicateSentence`.
**Behavior:**
- Endings lists each ending as "{id}: when {sentence}", in authored order, with Add, rename, Edit when, Remove and reorder.
- An ending with no `when` cannot be applied, because the decoder requires one.
- The fact index's ending readers now open this form at that ending.
- The `until` and `arrives` editors render through the same `PredicateEditor`, with their current forms.

**Tests:** › `an ending reads as its sentence`; › `holds is not offered for an ending`; › `the fact index opens the ending it names`; › `the until and arrives editors behave as before` (existing tests green).
**Verification:** `npx vitest run src/concepts` passes.

## Task S4-T4: Concealments on the Encounter home

**Delivers:** H5, H10, S4-Q1 (form side), S4-Q5.
**Owner:** rpg-dnd5e-web; new `encounter-studio/StudioConcealments.tsx`; `studioSession.ts` (`concealments.update`, `concealments.armMembers(id) / disarm()`); `WorldBuildingConcept.tsx` (expose `activeConcealmentId` to the studio); `EncounterStudioWorkspace.tsx` (`ENCOUNTER_SECTIONS` gains `concealments`); `siteScope.ts` (`validateSiteConcealments` refuses `lifetime`); `roomDraft.ts` (the stored-draft read strips it, S4-Q1).
**Behavior:**
- Each concealment shows its id; Notice checks; Search checks (at least one, as the validator requires); attempts with max and reset hexes; and members (cells, props and doors). It also shows "Revealed by", the intel records whose `reveals` names it, as read-only links.
- "Add members on the map" switches to Build with the concealment armed. Clicking a walkable hex, a declared prop, a door or a structural wall toggles membership; an undeclared prop or an unwalkable hex is not pickable (S4-Q5). A member already in another concealment is refused at the click, naming that concealment. Done or Escape disarms.
- `lifetime` is never offered. `validateSiteConcealments` refuses it with "lifetime is not a key this build reads: they are max, reset_hexes". The stored-draft read strips it before validation and returns the ids it removed it from; the studio shows the S4-Q1 notice once per open.
- Member cells are drawn in Layout with a hatch, using the S3 overlay layer, only while that concealment is armed or selected.

**Tests:** › `a concealment needs one search check`; › `members picked on the map toggle in one step each`; › `revealed by lists the intel that names it`; › `lifetime is not offered`; › `validateSiteConcealments refuses lifetime with the toolkit's sentence`; › `a stored draft with lifetime opens stripped and shows the notice naming the concealment`; › `a wall can be a member and an undeclared prop cannot`; › `a member already in another concealment is refused`; › `removing a concealment clears its door's Hidden by`.
**Verification:** `npx vitest run src/concepts` passes.

## Task S4-T5: Scenarios, read-only

**Delivers:** H6.
**Owner:** rpg-dnd5e-web; new `encounter-studio/StudioScenarios.tsx`; `EncounterStudioWorkspace.tsx`.
**Behavior:** lists each scenario id and its bindings as "field → {display name of the thing named}", using S5's names for placements. "None" when absent. Nothing is editable.
**Tests:** › `a scenario binding reads with the bound placement's name`; › `scenarios render no inputs`.
**Verification:** `npx vitest run src/concepts` passes.

## Task S4-T6: Export and import the draft

**Delivers:** H7, H8, S4-Q2, S4-Q3.
**Owner:** rpg-dnd5e-web; new `encounter-studio/StudioDraftFile.tsx` (in the toolbar's Save group); `studioSession.ts` (`importDocument(draft, scope): boolean`, wrapping `importCanonicalRoomYaml`); `WorldBuildingConcept.tsx`.
**Interfaces:**

```ts
export function exportDraftYaml(document: Readonly<RoomDraftDocument>, key: string): string;
// encodeSingleRoomDungeon({ key, draft, ...scope }), the exact bytes; throws the encoder's sentence
export type ImportResult =
  | { ok: true; key: string }
  | { ok: false; refusal: string };   // one sentence naming the field, or the dialect sentence
export function importDraftYaml(text: string, session: Pick<EncounterStudioSession, 'importDocument'>): ImportResult;
```

**Behavior:**
- Export asks for the key (S4-Q2) and downloads `{key}.yaml` with type `text/yaml;charset=utf-8`. If the encoder refuses, the sentence shows and no file is written.
- Import reads a chosen `.yaml` file and calls `readSingleRoomDungeon`.
  - For the other dialect, it refuses with: "This file is a version {n} dungeon, the multi-room dialect. The studio opens single-room files, version 3 or 4."
  - Otherwise it decodes. A refusal shows "Could not import: {decoder sentence}" and changes nothing.
  - An accepted import asks to replace the current draft, then replaces it through `importDocument` in one undoable step, and remembers the file's key for the next export.
- The World Builder's Canonical YAML disclosure is untouched.

**Tests:**
- › `export writes exactly encodeSingleRoomDungeon's bytes`;
- › `an exported file imports back to an equal document` (round trip on the S2 castle fixture plus the S4 walk's additions);
- › `a v2 file is refused with the dialect sentence and changes nothing`;
- › `a file with an unknown root key is refused naming the key`;
- › `an accepted import is one history entry and undo restores the previous draft`;
- › `the next export defaults to the imported key`.

**Verification:** `npx vitest run src/concepts` passes.

## Task S4-T7: Integration and the walk

- [ ] `npm ci`; `npm run ci-check` once; grep the log.
- [ ] Walk on the castle fixture as the local draft (S5-Q6 path), with a screenshot per step.
- [ ] Post the checklist below on #578 as `- [ ]` boxes before the PR leaves draft.

**Walk and #578 checklist:**

1. Encounter home: Endings, Concealments and Scenarios sections are present. Scenarios is read-only.
2. Place a King cast member. Add two endings: `king-slain` when the King is down, and `king-deposed` when *king-deposed* is known. Each reads as a sentence. The fact index lists the second as a reader.
3. Add the concealment `stash`, with a Search check (Investigation DC 14) and a notice check (Perception DC 12). Add members on the map: two hexes and a chest. Layout hatches them while selected.
4. Select a door. Set it to locked with Dexterity DC 15 and Thieves' Tools. Set Hidden by to `stash`. The stash lists the door; its wall is not listed.
5. Select a walkable hex and add exit `postern`. Its help names the blocking-prop refusal at play. The overview draws its marker.
6. Export: `{key}.yaml` downloads.
7. Clear the studio's local storage and reload: a blank draft.
8. Import the file. Confirm, and everything from steps 2–5 is back. Undo returns the blank draft; redo returns the castle.
9. Import a v2 file such as rpg-api `content/reference-hearing.yaml`: the dialect sentence, and nothing changes. Import a file with a typo'd root key: the refusal names it.
10. Seed a stored draft whose concealment carries `lifetime`, and open it: the notice names the concealment, and the key is gone from the next export.

## S4 requirement coverage

| requirement | task(s) | proof |
|---|---|---|
| H1 door state and lock | S4-T1 | the lock tests; walk 4 |
| H2 hidden by | S4-T1, T4 | `Hidden by moves the door …`; `hiding a door leaves its wall out …`; walk 4 |
| H3 exits | S4-T2 | the exit tests; walk 5 |
| H4 endings | S4-T3 | the ending tests; walk 2 |
| H5 concealments | S4-T4 | the concealment tests; walk 3 |
| H6 scenarios read-only | S4-T5 | `scenarios render no inputs`; walk 1 |
| H7 export | S4-T6 | `export writes exactly encodeSingleRoomDungeon's bytes`; walk 6 |
| H8 import | S4-T6 | the round-trip, dialect, refusal and undo tests; walk 7–9 |
| H9 fact index | S4-T3 | `the fact index opens the ending it names`; walk 2 |
| H10 nothing the toolkit refuses | S4-T2, T4 | `validateSiteConcealments refuses lifetime …`; `a stored draft with lifetime opens stripped …`; `a wall can be a member …`; `an unwalkable hex offers no exit` |

## S4 provider/consumer seams

| provider | consumer | contract | availability | proof |
|---|---|---|---|---|
| existing `doorBindingEdits.ts` | `DoorView` through `doors.setBinding` | binding normalisation | baseline | the T1 tests |
| existing `concealmentEdits.ts` and `activeConcealmentId` | `StudioConcealments`, `setDoorHiddenBy` | membership toggles | baseline, owner-only until T4 exposes them | the T4 tests |
| S4-T3 `PredicateEditor` | endings, `until`, `arrives` | allowed forms; sentences from S2 | new, extracted | T3 tests; existing editor tests |
| S2 `predicateSentence`, `factIndexOf` | S4-T3 | sentences; ending readers | S2 | T3 tests |
| S3 placed overlay | S4-T2 exit marker, S4-T4 hatch | a Layout layer | S3 | T2 and T4 tests |
| existing `encodeSingleRoomDungeon`, `readSingleRoomDungeon`, `decodeSingleRoomDungeon`, `importCanonicalRoomYaml` | S4-T6 | exact bytes; dialect read; first refusal; undoable replace | baseline | the T6 tests |
| encoded file | rpg-api registry and the toolkit decoder | `<key>.yaml`, the keys the dialect declares | baseline | the round-trip test; Save & Play later (#1236) |

**Shared files.** `EncounterStudioWorkspace.tsx` (T3, T4, T5, T6) and `studioSession.ts` (T1, T2, T4, T6) are edited in task order by one builder.

## Later slices

- S1, the cast and per-placement looks: rpg-project#575, planned above. A cast member selected on the home highlights its placements on the map, and the inspector shows the cast member. It is not a multi-selection (Q8).
- S2, read it back: prose, fact index, tables heaviest first: rpg-project#576, planned above.
- S3, overview on Layout: rpg-project#577, planned above. It retires the Regions home: region and lighting move to the hex view, and the room list moves to the Layout overview (Q5).
- S4, the hidden nouns: exits, endings, concealments, door state, and the draft as a file: rpg-project#578, planned above.
- S5, density: words, wrapping, panel width: rpg-project#579, planned above, ahead of S2.

All six slices are on web `dev` (e623c2b3). S6 below is Save and Play (web#1236); after it: open a reference encounter, look picker (web#1268, waits on assets#226).

---

# Section S6: Save and Play

Tracking issue: rpg-dnd5e-web#1236 (its acceptance walk is the QA list).
Law: design.md R15–R19 (rpg-project#583, merged 4dfce75f).

## S6 goal and scope

The author presses Save and Play in the studio header and is standing in the
room with the character picked on Home. The file saved is the file the
draft-file export writes, under one key. A server refusal is a jump to the
thing that caused it. With authoring off on the server, or no character, the
control says why and does nothing.

**Repositories.** Web only, rpg-dnd5e-web, from `dev` (e623c2b3). No toolkit,
API or proto change: `PutDungeon{validate_only}`, `GetDungeon`, `CreateLobby`,
`StartEncounter{dungeon_key}` and the file registry exist and are what World
Builder plays through today.

**Non-goals:** wall and region finishes or region lighting in play (R19);
opening a saved encounter from the server; a character picker in the studio;
a server-side overwrite guard; any change to `useRoomPublishing`'s or
`usePlayAuthoredDungeon`'s transaction logic.

## S6 requirement labels

| label | requirement | source |
|---|---|---|
| P1 | One Save and Play control in the header's Save group, on both homes and both views. Pressing it saves then launches; the app opens the game view on the returned encounter id with the picked character. | R15 |
| P2 | The saved bytes are `encodeSingleRoomDungeon({key, draft, ...renderScope(scope)})`, the same call the draft-file export makes. | R15, R16 |
| P3 | One key per draft: the Draft file panel and the publish row read and write the same key; default is the slug of the room name (`defaultDraftKey`), editable, adopted from an import. The server's key refusal shows at the key field. | R16 |
| P4 | The first save to a key the server already holds asks before overwriting; later saves to that key in the same editor session do not ask; changing the key drops the memory. | R16 |
| P5 | A refusal whose path maps to a placed creature, prop, door, exit hex, cast member, table, faction, disposition, intel, ending or concealment is a jump that selects it or opens its home section. Any other path, or an empty path, is a sentence under the Save group. The live validate-only preview's refusals and the save's refusals both jump. | R17 |
| P6 | Authoring off on the server: the control is present, disabled, with "The server is not accepting authored encounters." Server unreachable: disabled with "The server could not be reached." and a retry. No character on Home: Play is disabled with "Pick a character on Home to play." Save to server stays available. | R18 |
| P7 | While a save or launch is in flight, edits, undo, redo, selection changes and the local-draft save are refused with the owner's existing notice; a response for a retired capture (key, draft, character or client changed) saves and launches nothing. | R15 |
| P8 | Save failure and launch failure each show one sentence naming the seam, the draft is untouched and recoverable, and no "Saved as" or launch is claimed early. | #1236 scope |
| P9 | The wall-finish and region-finish fields and the region lighting field carry one help line: "Play does not render this yet; it stays in the studio until the look catalog." | R19 |

## S6 constraints

- **The hooks are reused unchanged.** `useRoomPublishing` (`world-building/useRoomPublishing.ts`) and `usePlayAuthoredDungeon` (`author/usePlayAuthoredDungeon.ts`) keep their contracts: owner token, `isCurrent`, overwrite memory, launch sequence `createLobby → setReady → startEncounter → onPlay`. The studio hosts the hook and builds its own row and panel; `RoomPublishingPanel.tsx` is untouched.
- **The key is the hook's key.** `useRoomPublishing` returns `key`/`setKey`, and `setKey` already drops an armed overwrite and the saved memory. The studio seeds it (P3) and passes it down; `StudioDraftFile` loses its local key state.
- **The lock is the owner's.** `WorldBuildingConcept` already refuses every edit, undo, redo, selection and `saveNow` while `publishBusy` (`refuseWhilePublishing`, 56 sites) and forces the tool to select. S6 only lets the studio set it. Home, view and frame changes are workspace state, not document changes, and stay ungated.
- **The path grammar is the server's.** `FieldError{path, message}` (rpg-api-protos `authoring/v1alpha1`), path into the file as written: the draft under `room.room.*` (for example `room.room.monsterBindings.goblin-1.faction`, `room.room.doorBindings.cellar-door.locked`), root policies at the root (`exits[0].cell`, `tables.gate-patrol.…`, `factions.garrison.…`, `endings[0].when`, `concealments.vault.cells[0]`), `line N` for a parse failure, empty for a whole-file failure. The map is prefix-based and tolerant: an unknown tail still jumps to the noun.
- **No studio-side standability.** An exit under a blocking prop is refused by the toolkit at compile (`exits[i].cell: … is not standable`, S4-Q4) and reaches the studio as a jump to that hex (P5).
- **No new document key.** The key lives in studio state; `RoomDraft` has `id` and `name` only.

## S6 inspected baseline (2026-10-14)

| repo | revision |
|---|---|
| rpg-dnd5e-web `origin/dev` | e623c2b3 |
| rpg-api `origin/dev` | `internal/dungeons/registry.go`: `^[a-z0-9-]+$`, request key must equal the file's `key:`, writes refused unless authoring, `<RPG_CONTENT_DIR>/<key>.yaml` by temp file and rename, no overwrite guard; `AuthoringService` registered only with `RPG_AUTHORING_ENABLED=1` (`cmd/server/server.go`) |
| rpg-api-protos `origin/main` | `PutDungeonResponse{errors: FieldError[], atlas}`, `FieldError{path, message}` |
| rpg-deployment | `docker-compose.local-dev.yml` sets `RPG_AUTHORING_ENABLED=1` and `RPG_CONTENT_DIR=/content`, so `scripts/dev-env.sh` stacks accept saves |

- **Publishing hook.** `useRoomPublishing({draft, scope, capability: {characterId, onPlay}, client?, onImportDraft, onBusyChange?})` → `{key, setKey, yaml, encodeError, preview, busy, phase, error, overwritePending, saver, savedFor, launch, save, saveAndPlay, confirmOverwrite, cancelOverwrite, importYaml}`. `preview` is `usePutDungeonPreview`'s `{status: 'idle'|'validating'|'compiled'|'errors'|'unreachable', errors: FieldError[], atlas, message}`, debounced 400 ms on every edit. `saver` is `useSaveDungeon`'s `{status: 'idle'|'saving'|'saved'|'invalid'|'error', errors, message, savedKey}`. Key default today is `room-<id>`.
- **Launch hook.** `usePlayAuthoredDungeon` → `play(key)`, `cancel()`, `{launching, phase, error}`; lobby on `DEV_CAMPAIGN_ID`; retires on character change and unmount; its error names the failing seam.
- **Gate.** `useAuthoringGate()` probes `GetDungeon("")`: Unimplemented → `gate-off`; Unavailable/Unknown → `unreachable`; else `live`; module-level cache; `retry()`. Consumer today: `DungeonBuilderHomeButton.tsx`.
- **App.** `EncounterStudioWorkspace` gets `compositionSource` and `onBack` only. `WorldBuilderWorkspace` gets `characterId` (Home's `selectedId` when `selectedType === 'character'`) and `onPlay={handlePlayAuthored}`; `handlePlayAuthored(encounterId, characterId)` sets the lobby character and the resume id and switches to the game view.
- **Studio header.** `StudioSurface` (`EncounterStudioWorkspace.tsx`) renders `es-save` with the local-draft status, `session.saveLocalDraft`, and the Draft file toggle; `fileKey: {key, roomId} | null` state is set only by an import; `StudioDraftFile` keeps the typed key in its own state and never lifts it.
- **Selection.** `StudioSelection` kinds: nothing, wall, door (`StudioDoorTarget{wallId, openingId, doorId}`), creature (`monsterDeclarations[].id`, equal to the `monsterBindings` key), prop and group (scene ids, equal to the `propDeclarations`/`propBindings` keys), hex (cell), label, partyStart, castMember (ref). Setters: `selectHex`, `selectScene`, `creatures.select`, `cast.select`, `doorEditing.select`, `wallEditing.select`, `selectPartyStart`. Home sections: `openDefinitions(section)` with `ENCOUNTER_SECTIONS`; `focusEnding({id})` opens one ending's form; no focus for cast, tables, factions, concealments.
- **Lock.** `WorldBuildingConcept` props `roomPublishing?` and `onPublishBusyChange?`; `handlePublishBusy` is called only by `RoomPublishingPanel`, which the studio branch never renders; `EncounterStudioWorkspace.test.tsx` asserts the studio passes neither.
- **Tests.** `RoomPublishingPanel.test.tsx` has a file-local `fakeClient(): AuthoringClient` over deferreds plus `vi.mock`s of `useCreateLobby`, `useSetLobbyReady`, `useStartLobbyEncounter`. `EncounterStudioIntegration.test.tsx` mounts the real owner with `compositionSource, storage, idFactory` and asserts no Save & Play button (lines 1689–1693). `usePlayAuthoredDungeon.test.tsx` covers the launch sequence.

## S6 sequence

```
S6-T1 the lock and the capability ──▶ S6-T3 the row and panel ──▶ S6-T5 integration and the walk
S6-T2 one key ───────────────────────┤                            │
S6-T4 refusal jumps ─────────────────┘                            │
S6-T6 finishes say studio-only ───────────────────────────────────┘
```

One branch off `dev`, one PR to `dev`. T1 and T2 before T3; T4 is a pure module T3 consumes. `EncounterStudioWorkspace.tsx` is edited by T1, T2, T3 in that order by one builder.

## S6 open items

| id | item | state | blocks |
|---|---|---|---|
| S6-Q1 | **Where the hook lives.** `StudioSurface` is rendered inside the owner through `studioPresentation.render(session)`, and `session.document` is `{draft, scope}`. | **lead-resolved:** the hook is called in `StudioSurface`, where the key, the panel and the header are; busy reaches the owner through the session (S6-T1). | nothing |
| S6-Q2 | **Which key default.** The hook defaults to `room-<id>`; the studio's export defaults to the slug of the name (S4-Q2). | **R16:** slug of the name. The studio seeds the hook's key once per draft id with `defaultDraftKey(document)`, or the imported key when `fileKey.roomId` matches. | nothing |
| S6-Q3 | **The live preview against the API.** `usePutDungeonPreview` sends a validate-only put 400 ms after every edit. | **lead-resolved:** keep it, gated on the authoring gate being `live`, because it is what makes the exit-under-a-prop refusal appear before Save and because World Builder already runs it. `enabled: gate === 'live'`. | nothing |
| S6-Q4 | **Home sections without a focus.** Tables, factions, concealments and cast have no focus prop. | **lead-resolved:** a jump to one of those opens the section (`openDefinitions`); the row id is carried in the sentence. Per-row focus for them is a follow-up noted on #1236. Endings already focus by id; creatures, props, doors, hexes and cast members select. | nothing |
| S6-Q5 | **Two buttons or one.** | **lead-resolved:** the header has one button, Save and Play (R15). The panel it opens carries Save to server for an author who wants the file without playing, the key field, the overwrite ask, the refusal list and the status line. | nothing |
| S6-Q6 | **The character on the walk.** The scripted walk needs a character on Home. | **lead-resolved:** the driver creates one through the existing character flow as the wave B walk driver did (`tools/browser/_job_*.mjs` history), or reuses the one the operator's stack holds; it never seeds redis by hand. | S6-T5 |

## Task S6-T1: The lock and the capability

**Delivers:** P1 (the plumbing), P7.
**Owner:** rpg-dnd5e-web; `App.tsx`, `encounter-studio/EncounterStudioWorkspace.tsx`, `encounter-studio/studioSession.ts`, `world-building/WorldBuildingConcept.tsx`.
**Prerequisites:** none.
**Files:** modify `App.tsx` (pass `characterId` and `onPlay` to `EncounterStudioWorkspace`, the same values `WorldBuilderWorkspace` gets); modify `EncounterStudioWorkspace.tsx` (props `characterId: string | null`, `onPlay: (encounterId: string, characterId: string) => void`; pass `onPublishBusyChange` to the owner; pass both through `StudioSurfaceProps`); modify `studioSession.ts` and the owner's studio session construction; tests `EncounterStudioWorkspace.test.tsx`, `EncounterStudioIntegration.test.tsx`.
**Interfaces:**

```ts
// studioSession.ts — new member
publishing: {
  busy: boolean;                       // mirrors the owner's publishBusy
  setBusy(next: boolean): void;        // calls the owner's handlePublishBusy
};
// EncounterStudioWorkspace props
characterId: string | null;
onPlay: (encounterId: string, characterId: string) => void;
```

**Behavior:** `session.publishing.setBusy(true)` makes the owner refuse edits, undo, redo, selection and `saveLocalDraft` with its existing notice and forces the tool to select; `setBusy(false)` releases and clears the notice. Home, view and frame changes are not gated. `characterId` and `onPlay` reach `StudioSurface` unchanged.
**Tests:**
- › `publishing.setBusy(true) makes an edit a refusal and canUndo false; setBusy(false) releases` (integration test, real owner);
- › `the studio passes onPublishBusyChange to the owner and characterId/onPlay to the surface` (workspace test; replaces the assertion that they are absent).
**Verification:** `npx vitest run src/concepts/encounter-studio` passes.

## Task S6-T2: One key

**Delivers:** P3.
**Owner:** rpg-dnd5e-web; `encounter-studio/StudioDraftFile.tsx`, `encounter-studio/draftFile.ts`, `EncounterStudioWorkspace.tsx`.
**Prerequisites:** S6-T1 (the surface hosts the hook).
**Files:** modify `StudioDraftFile.tsx` (props `key: string`, `onKeyChange(next: string)`; remove the local key state; keep `onFileKey` on import); modify `EncounterStudioWorkspace.tsx` (`StudioSurface` calls `useRoomPublishing` and seeds its key).
**Interfaces:**

```ts
// StudioSurface
const publishing = useRoomPublishing({
  draft: session.document.draft, scope: session.document.scope,
  capability: { characterId, onPlay }, client: authoringClient,
  onImportDraft: session.importDocument, onBusyChange: session.publishing.setBusy,
});
// seed, once per draft id: fileKey?.roomId === draft.id ? fileKey.key : defaultDraftKey(session.document)
```

**Behavior:** the Draft file panel and the publish panel show the same key field value; typing in either updates the hook's key (which drops an armed overwrite and the saved memory); export names the file `{key}.yaml`; an import sets `fileKey` and the hook's key to the file's key; a new draft id reseeds from the name. The server's key refusal (`FieldError` with an empty path and the registry's invalid-key sentence, or the mismatch sentence) renders under the key field.
**Tests:**
- › `export and publish share one key: typing in Draft file changes the key the publish row shows`;
- › `the key seeds from the room name slug on a fresh draft`;
- › `an import adopts the file's key for both`;
- › `changing the key drops an armed overwrite` (hook behavior, asserted through the studio panel).
**Verification:** `npx vitest run src/concepts/encounter-studio` passes.

## Task S6-T3: The row and the panel

**Delivers:** P1, P4, P6, P8, S6-Q5.
**Owner:** rpg-dnd5e-web; new `encounter-studio/StudioPublish.tsx`; `EncounterStudioWorkspace.tsx` (`es-save` group); `author/useAuthoringGate.ts` unchanged.
**Prerequisites:** S6-T1, S6-T2, S6-T4.
**Files:** create `StudioPublish.tsx` and `StudioPublish.test.tsx`; modify `EncounterStudioWorkspace.tsx` (button in `es-save` after the Draft file toggle; panel below the header beside the Draft file panel, same `es-panel` styling from S5); extract `RoomPublishingPanel.test.tsx`'s `fakeClient` into `author/testing/fakeAuthoringClient.ts` (move, no behavior change; the World Builder test imports it).
**Interfaces:**

```ts
export interface StudioPublishProps {
  publishing: ReturnType<typeof useRoomPublishing>;
  gate: AuthoringGateState;            // from useAuthoringGate(), with retry
  characterId: string | null;
  jumps: RefusalJump[];                // from S6-T4, computed by the surface
  onJump(target: RefusalTarget): void; // the surface routes it (S6-T4)
}
```

**Behavior:**
- Header button "Save & Play": disabled with the P6 sentence as its help when `gate !== 'live'` or `characterId === null`; otherwise calls `publishing.saveAndPlay()`. Pressing it opens the panel. A status span beside it reads the phase: "Checking key…", "Saving…", "Starting…", "Saved as {key}", or nothing.
- Panel: the key field (T2); "Save to server" (enabled when `gate === 'live'`); the overwrite ask "{key} already exists on the server. Replace it?" with Replace and Keep both, focus moving to Replace; the refusal list (T4) for whichever of `preview.errors` or `saver.errors` is current; "Save failed: {message}. Your draft is unchanged." for `saver.status === 'error'`; the launch error sentence from `launch.error`; the gate sentence with Retry when `gate === 'unreachable'`.
- The preview runs with `enabled: gate === 'live'` (S6-Q3); the studio does not render its atlas.
- After `onPlay` the app leaves the studio; nothing in the studio clears the local draft.
**Tests (StudioPublish.test.tsx, fake client, launch hooks mocked as in the World Builder test):**
- › `gate-off disables Save & Play with the server sentence and leaves export usable`;
- › `no character disables Save & Play with the Home sentence and leaves Save to server enabled`;
- › `Save & Play on a new key: GetDungeon NotFound, PutDungeon with the exact export bytes, then createLobby, setReady, startEncounter with the key, then onPlay(encounterId, characterId)`;
- › `Save & Play on an existing key asks; Keep saves nothing; Replace saves and plays`;
- › `a second Save & Play to the same key does not ask`;
- › `PutDungeon invalid shows the refusals and launches nothing`;
- › `transport failure shows "Save failed" and the draft is unchanged`;
- › `startEncounter failure shows the seam sentence and no onPlay`;
- › `edits during a save are refused and the saved bytes are the captured bytes` (integration: change the name after pressing, resolve the put, assert the put's yaml equals the capture and the notice fired).
**Verification:** `npx vitest run src/concepts/encounter-studio src/concepts/world-building/RoomPublishingPanel.test.tsx` passes.

## Task S6-T4: Refusal jumps

**Delivers:** P5, S6-Q4.
**Owner:** rpg-dnd5e-web; new `encounter-studio/refusalJump.ts` and `refusalJump.test.ts`; the routing in `StudioSurface`.
**Prerequisites:** none for the module; S6-T1 for the routing.
**Interfaces:**

```ts
export type RefusalTarget =
  | { kind: 'creature'; id: string }
  | { kind: 'prop'; id: string }
  | { kind: 'door'; target: StudioDoorTarget }
  | { kind: 'hex'; cell: HexCoordinate }
  | { kind: 'castMember'; ref: string }
  | { kind: 'home'; section: 'tables'|'factions'|'dispositions'|'intel'|'concealments'; id?: string }
  | { kind: 'ending'; id: string };
export interface RefusalJump { path: string; message: string; target: RefusalTarget | null; }
export function refusalJumps(errors: FieldError[], document: Readonly<RoomDraftDocument>): RefusalJump[];
```

**Behavior (prefix map, longest match, trailing segments ignored):**
- `room.room.monsterBindings.<id>` or `room.room.monsterDeclarations[<i>]` → creature `<id>` or `monsterDeclarations[i].id`;
- `room.room.propBindings.<id>` or `room.room.propDeclarations.<id>` → prop `<id>` when it is a scene item;
- `room.room.doorBindings.<id>` → door target found by scanning `walls[].openings[].door.id`;
- `exits[<i>]` → hex at `scope.exits[i].cell`;
- `templates.<id>` → castMember `dnd5e:monsters:<id>`;
- `tables.<id>`, `factions.<id>`, `dispositions[<i>]`, `intel[<i>]`, `concealments.<id>` → home section with the id;
- `endings[<i>]` → ending `endings[i].id`;
- anything else, `line N`, or empty → `target: null`.
A target whose id no longer exists in the document is `null`. The surface routes: creature → `creatures.select`; prop → `selectScene([id])`; door → `doorEditing.select`; hex → `selectHex`; castMember → `cast.select`; home → `openDefinitions(section)`; ending → `focusEnding`. The row renders as a button "{message}" with the noun name when `target` is set, otherwise a sentence with the path in small type.
**Tests (`refusalJump.test.ts`, castle fixture plus a door and an exit):**
- one case per bullet above, plus › `an unknown id is a sentence`, › `an empty path is a sentence`, › `line 12 is a sentence`, › `a deeper tail still jumps` (`room.room.monsterBindings.goblin-1.on.time[0].toward.at` → creature goblin-1).
**Verification:** `npx vitest run src/concepts/encounter-studio/refusalJump.test.ts` passes.

## Task S6-T5: Integration and the walk

- [ ] `npm ci`; `npm run ci-check` once; grep the log.
- [ ] Stack `local/studio-s6`: web :3030, api :8115 (memory `studio-walk-port-is-fixed`), `RPG_API_REF=dev`, web from a detached `rpg-dnd5e-web/.worktrees/studio-s6-walk` at the PR head. Confirm the API log prints `AuthoringService registered`.
- [ ] Driver `tools/browser/_job_s6_play.mjs` in the job tmp dir, run from `tools/browser/`; screenshots per step; the character per S6-Q6.
- [ ] Post the checklist below on rpg-dnd5e-web#1236 as `- [ ]` boxes before the PR leaves draft.

**Walk and #1236 checklist:**
1. Open the studio from Home with a character picked; the header shows Save & Play enabled.
2. Load the castle fixture as the local draft; the key reads the slug of its name in both the Draft file panel and the publish panel.
3. Press Save & Play on a fresh key: the status runs Checking key, Saving, Starting; the game view opens on the room; the cast stand where placed in their looks; the props are there; the authored door opens through the existing interaction.
4. Back on Home, reopen the studio: the draft is the same; press Save & Play again: no overwrite ask.
5. Change the key to one that exists: the ask appears; Keep saves nothing; Replace saves and plays.
6. Put an exit under a blocking prop: within a second the refusal list shows the exit's refusal; clicking it selects the hex and opens its inspector; remove the exit and the list clears.
7. Clear the character on Home and reopen the studio: Save & Play is disabled with the Home sentence; Save to server still saves.
8. Bring the stack up with `RPG_AUTHORING_ENABLED` unset once: the control is disabled with the server sentence; the Draft file export still downloads.
9. Press Save & Play, then try an edit and undo before the put resolves: both are refused with the notice; the saved file's bytes equal the capture.
10. Stop the API mid-save: "Save failed … draft is unchanged"; bring it back; Save & Play succeeds.
11. Wall inspector: the finish field carries the studio-only help line (P9).

## Task S6-T6: Finishes say studio-only

**Delivers:** P9.
**Owner:** rpg-dnd5e-web; the wall view and the hex view's region fields in `encounter-studio/` (S0's `StudioArrangeFields.tsx` or its S5 successor, found by the `wallFinish` and lighting field labels).
**Behavior:** one `FieldHelp` line (S5) under the wall finish, region wall finish and region lighting fields: "Play does not render this yet; it stays in the studio until the look catalog."
**Tests:** › `the wall finish field carries the studio-only help line`.
**Verification:** `npx vitest run src/concepts/encounter-studio` passes.

## Task S6-T7: The saved key outlives the studio mount

Added from the walk at 2b72b705 (step 4 failed: after a play, Back and reopen, the second Save and Play asked to replace the draft's own file, because `savedRef` died with the mount). R16 as clarified (rpg-project#584): the editor session is the draft's life in this browser.

**Delivers:** P4 per draft.
**Owner:** rpg-dnd5e-web; new `encounter-studio/publicationMemory.ts` (localStorage record `rpg.concepts.encounter-studio.publication.v1` = `{roomId, key}`, reads and writes in try/catch, malformed ignored); `useRoomPublishing` gains one optional input `rememberedSavedKey?: string | null` that seeds the saved memory on mount, transactions unchanged; `StudioSurface` seeds the key and the memory from a record with the draft's id, writes the record after each successful save of the current key, and an import clears it.
**Tests:** reopen after a save does not ask; a different draft id asks; an import clears the memory; a malformed record is ignored.

## S6 record

Built on `feat/studio-s6-play`, PR rpg-dnd5e-web#1283; independent review 5525939635 MERGE-READY at 42b01832 (four Minor threads, dispositions in-thread; follow-ups on #1236), closure 5526243472 MERGE-READY at fa1e6dee. Walk at fa1e6dee: 11 of 11 (steps 8 and 10 with the API toggled). Operator QA checklist on #1236. Design corrections from the build: rpg-project#584.

## S6 requirement coverage

| requirement | task(s) | proof |
|---|---|---|
| P1 one control, both homes and views, saves then launches | T1, T3 | T3 `Save & Play on a new key …` and the workspace test; walk 1, 3 |
| P2 same bytes as the export | T2, T3 | T3 new-key test asserts the put's yaml equals `exportDraftYaml(document, key)` |
| P3 one key, slug default, import adopts | T2 | T2 tests; walk 2 |
| P4 overwrite asked once per key, per draft | T3, T7 | T3 existing-key and second-save tests; T7 reopen test; walk 4, 5 |
| P5 refusals jump | T4, T3 | T4 tests; T3 invalid test renders jumps; walk 6 |
| P6 gate off, unreachable, no character | T3 | T3 gate and character tests; walk 7, 8 |
| P7 lock and stale responses | T1, T3 | T1 lock test; T3 edits-during-save test; the hook's own tests for retired captures; walk 9 |
| P8 failures named, draft kept | T3 | T3 transport and launch-failure tests; walk 10 |
| P9 finishes say studio-only | T6 | T6 test; walk 11 |

## S6 provider/consumer seams

| provider | consumer | contract | availability | proof |
|---|---|---|---|---|
| existing `useRoomPublishing` | S6-T2, S6-T3 | the input and result shapes in the baseline; `setKey` drops overwrite memory; `saveAndPlay` = save then `play(key)` | baseline | `RoomPublishingPanel.test.tsx` (unchanged) and the T3 tests |
| existing `usePlayAuthoredDungeon` through the hook | S6-T3 | `createLobby → setReady → startEncounter{dungeonKey} → onPlay(encounterId, characterId)` | baseline | `usePlayAuthoredDungeon.test.tsx`; T3 new-key test |
| existing `useAuthoringGate` | S6-T3 | `'probing'|'live'|'gate-off'|'unreachable'`, `retry()` | baseline | T3 gate tests |
| existing `encodeSingleRoomDungeon` / S4 `exportDraftYaml` | S6-T2 | the same bytes for the same key, draft and scope | baseline | T3 asserts equality |
| S6-T1 `session.publishing.setBusy` | S6-T2's `onBusyChange` | boolean, calls the owner's `handlePublishBusy` | new | T1 lock test; T3 edits-during-save test |
| S6-T4 `refusalJumps` | S6-T3 panel, surface routing | `RefusalJump[]` with `target | null` | new | T4 tests; walk 6 |
| App `characterId`, `onPlay` | S6-T1 | the same values `WorldBuilderWorkspace` receives | baseline in App, new on the studio | workspace test; walk 1, 3 |
| rpg-api registry, lobby service | the hooks | `PutDungeon`, `GetDungeon`, `CreateLobby`, `StartEncounter{dungeon_key}`; authoring flag | baseline, `local-dev` compose sets the flag | the walk |

**Shared files.** `EncounterStudioWorkspace.tsx` (T1, T2, T3) and `StudioDraftFile.tsx` (T2) are edited in task order by one builder. `RoomPublishingPanel.test.tsx` changes only its import of the fake client.
