---
name: Encounter studio S0 — implementation plan
status: PLAN for slice S0 only, derived 2026-10-14 from design.md and the S0 issue; checked against the inspected baseline below; S1–S5 are stubs
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
workspace up and to the right without moving a cell.

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
- Multi-select and apply-to-all (R9).
- Any change to the Encounter home's forms or its section list.
- The Regions home. It keeps its own sidebar in S0 (open item Q5).

## Requirement labels

| label | requirement | source |
|---|---|---|
| I1 | The Build home has no Arrange, Walls, Doors or Notes tabs and no Size toolbar button. Every field they held is reached through the inspector or an armed tool. | R1 |
| I2 | The inspector is keyed by one selection: nothing, wall, door, creature, prop or hex. Each kind has one view. | R1, design Shape table |
| I3 | Clicking a thing selects it and opens the inspector even when it is hidden. N toggles the inspector. N keeps its current input-focus and modifier exclusions. | R1 |
| I4 | One selection at a time. A selection is never a set. | R9 |
| I5 | A selection made on the Encounter home is the selection on the map, and the reverse. Switching homes keeps it. | R11 (cast half in S1) |
| I6 | A resize adds columns on the right and rows on top by default. Every cell, wall, prop, label and creature keeps its coordinate. | R13 |
| I7 | Growing another side is the author's explicit choice, stated before the edit. | R13 (blocked, Q6) |
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
- **No room-wide default finish exists.** Finishes are per wall
  (`scene.wallFinishes`) and per region (`AuthoringRegion.wallFinish`). See Q4.
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
| Q1 | **Map labels and party start.** Both are selectable today (`label` and `start` targets) but are not among the design's six kinds. Do they stay selectable with their own views, or fold into the room and hex views? | open, for the operator | the final `StudioSelection` shape in T1. **Assumption until answered:** both stay as the provisional kinds `note` and `start`, with exactly their current Arrange fields inside the inspector, so nothing that works today stops working. |
| Q2 | **Reaching nothing.** If every floor click selects a hex, the room is reached only by Escape or a click off the floor. Is that the gesture? | open | T2 click mapping. **Assumption:** a floor-cell click selects the hex in both views; Escape and a click outside the workspace select nothing. |
| Q3 | **The play contract.** The design lists void, lighting and standing as pick-from-list in the room view, but the dialect accepts one value each. Editing them is a toolkit change. | open | the editable form. **S0 shows them read-only**, stated as the dialect's fixed values. |
| Q4 | **Default finishes.** The room view lists default finishes, but no room-wide finish key exists. Is it a new `room.scene` key, or the existing per-region finishes reached from the hex view? | open | the room view's finishes field. S0 ships the room view without it. |
| Q5 | **The Regions home.** The design has two surfaces; the code has three homes. Does Regions fold into the map inspector (a region selected from the hex view), or stay a home? | open | nothing in S0. The hex view links to the Regions home. |
| Q6 | **Growing another side.** With explicit bounds, a workspace grows on any side without moving a cell, so R13's warning that cells shift would never be true. Is the other-side choice still a shift, with the origin pinned to a corner, or is the warning dropped? | open | I7. **T6 ships the default up-and-right growth only.** |
| Q7 | **Grouping under R9.** Scene groups are made by multi-selecting props (Shift, Ctrl or Meta in the tree and Shift in 3D), and the epic keeps grouping. R9 forbids a multi-selection. Does grouping get its own gesture, or does the scene multi-select stay as a tool rather than a selection? | open | the R9 narrowing step in T2. **Until answered, T2 leaves scene multi-select as it is and the inspector shows the existing "N scenery roots" view for it.** |
| Q8 | **R11 and R9 together.** S1's "select a cast member, select its placements" selects several placements. Is a cast-member selection its own kind that highlights placements, rather than a multi-selection? | open | nothing in S0; S1's selection shape |
| A1 | Shrinking removes columns from the right and rows from the top, and is refused, as today, when content falls outside. | assumption | T6 |
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
- `encounter-studio/studioSession.ts`: exports `StudioSelection`, `studioSelectionOf`, and re-exports the new names.
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

// studioSession.ts — the inspector's key, derived, never stored
export type StudioSelection =
  | { readonly kind: 'nothing' }
  | { readonly kind: 'wall'; readonly id: string }
  | { readonly kind: 'door'; readonly target: StudioDoorTarget }
  | { readonly kind: 'creature'; readonly id: string }   // monsterDeclarations[].id
  | { readonly kind: 'prop'; readonly id: string }       // one scene root id: an item or a group
  | { readonly kind: 'hex'; readonly cell: RoomHexCell }
  // provisional until Q1 is answered:
  | { readonly kind: 'note'; readonly id: string }       // a map label, with or without a region
  | { readonly kind: 'start' }
  // provisional until Q7 is answered:
  | { readonly kind: 'props'; readonly ids: readonly string[] };

export function studioSelectionOf(
  arrange: StudioArrangeSelection | null
): StudioSelection;
```

**Behavior:**
- `projectStudioArrange` returns null for a hex target whose cell is outside the workspace (`isCellWithinWorkspace`), so the arbiter masks it, as it masks an absent noun.
- A hex projection with an empty `walkableHexes` membership still projects; `walkable: false`.
- `regionsAtCell` includes a region when its resolution is `resolved` and the hex centre is inside its polygon ring (`pointInRing`, `regionBoundaryPredicates.ts`), or the cell is in its `hex-union`. Unresolved regions are excluded. Overlaps return every match.
- `studioSelectionOf` maps null to `nothing`; `actor` to `creature`; `scene` with one root to `prop` and with more to `props`; `label` to `note`; `start`, `wall`, `door` and `hex` to themselves.

**Tests:**
- `studioArrange.test.ts` › `projects a hex inside the workspace with walkable and regions`: a 6×6 draft, cell `{q:1, r:1}` walkable and inside an explicit region `hall`; expect `kind 'hex'`, `walkable true`, `regions[0].id === 'hall'`.
- › `masks a hex outside the workspace`: cell `{q:40, r:0}`; expect `null`.
- › `projects a non-walkable hex`: expect `walkable false`.
- `regionAtCell.test.ts` › `polygon region contains its interior hex`, `hex-union region contains listed cells only`, `unresolved region is never reported`, `overlapping regions are both reported in authored order`.
- `studioSelection.test.ts` › one case per mapping above, including `scene with two roots maps to props`.

- [ ] Write the tests; run them and confirm they fail on the missing kinds.
- [ ] Implement the target, projection, key, helper and mapping.
- [ ] Run the focused tests and `npm run typecheck`.

**Verification:** from the web worktree, `npx vitest run src/concepts/world-building/studioArrange.test.ts src/concepts/world-building/regionAtCell.test.ts src/concepts/encounter-studio/studioSelection.test.ts` passes; `npm run typecheck` exits 0.
**Completion evidence:** the passing test names and the typecheck result in the commit message body.

## Task T2: Selection gestures

**Delivers:** I3 (the click half), I4, the hex selection gesture (Q2 assumption).
**Owner:** rpg-dnd5e-web; `LayoutViewport.tsx`, `WorldBuildingViewport.tsx`, `WorldBuildingConcept.tsx`.
**Prerequisites:** T1.
**Files:**
- `encounter-studio/studioSession.ts`: `EncounterStudioSession` gains `selectHex(cell: RoomHexCell | null): boolean` and `clearSelection(): boolean`. `LayoutViewportProps` gains `onSelectHex?(cell: RoomHexCell): void`.
- `encounter-studio/LayoutViewport.tsx` `pointerDown`: in `select`, a left click on a workspace cell that hits no wall, door, label or endpoint calls `onSelectHex(cell)` (cell from `workspaceCellAtPoint`) after clearing the others, as the bare-floor branch does today. A click outside the workspace clears every selection.
- `world-building/WorldBuildingViewport.tsx`: the ground `onPointerDown` in `select` with a room cell under the pointer calls a new `roomAuthoring.onSelectHex?(cell)` instead of `onSelect([])`. The Shift exception stays (Q7).
- `world-building/WorldBuildingConcept.tsx`: implements `selectHex` and `clearSelection` through `activateStudioTarget`, clearing `selectedIds`, `selectedWallId`, `selectedLabelId`, `selectedActorId` and the door preview as the existing select functions do. The `Escape` branch of `handleKey` calls `clearSelection` in studio mode. Wraps both with `guardSnapshotIntent`.
- Tests: `LayoutViewport.test.tsx`, `WorldBuildingViewport.test.tsx`, `WorldBuildingConcept.test.tsx`.

**Interfaces:** `selectHex(cell)` returns false and changes nothing when the cell is outside the workspace, while publishing is busy, or from a retired snapshot. `clearSelection()` returns true and sets the target to null.

**Behavior:**
- A hex selection and a wall, door, note, creature or prop selection replace each other; there is never more than one target.
- Selecting a hex never paints, erases or edits the floor. The floor tools are unchanged.
- Escape inside an input, a select or a dialog keeps its local meaning. The window handler already returns early for those targets, and that guard stays.
- Scene multi-select through Shift, Ctrl or Meta is unchanged until Q7 is answered.

**Tests:**
- `LayoutViewport.test.tsx` › `select click on bare floor selects that hex`: `onSelectHex` called once with the cell under the pointer; `wallEditing.select(null)` and `labelEditing.onSelect(null)` called.
- › `select click on a wall selects the wall, not a hex`: `onSelectHex` not called.
- › `click outside the workspace selects nothing`: `onSelectHex` not called; the three clears called.
- › `paint tool on bare floor never selects a hex`: `onCommit` path unchanged; `onSelectHex` not called.
- `WorldBuildingViewport.test.tsx` › `ground click in select mode selects the hex under the pointer`; › `shift ground click keeps the current scene selection`.
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
  selection: StudioSelection;           // studioSelectionOf(session.arrange)
  view: EncounterStudioView;
  layoutTool: LayoutTool;
  armedTool: ReactNode | null;          // StudioWallControls | StudioDoorControls | labels.controls, per A2
  onConfigure(section: StudioSocialSection): void;
  onMoveCreature(): void;
  onFrameChange(frame: LayoutFrame): void;
  onArmTool(tool: 'wall' | 'door' | 'label'): void;
}): React.JSX.Element;
```

The inspector's root is `<section aria-label="Inspector">` with an `h2` naming the selection: "Room" for nothing, otherwise the name `selectionName` gives today. T4 fills the per-kind body; T3 renders the existing `SelectedArrange` body for every kind except nothing and hex, so the slice stays working between commits.

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

**Delivers:** I2 (one view per kind), I8, K4; the room, wall, door, creature, prop and hex views.
**Owner:** rpg-dnd5e-web; `StudioInspector.tsx`, `StudioArrangePanel.tsx`, `StudioArrangeFields.tsx`, `WorldBuildingConcept.tsx`.
**Prerequisites:** T3. T6 for the room view's Size field, or T6 adds that field itself.
**Files:**
- `encounter-studio/StudioInspector.tsx`: one view component per kind: `RoomView`, `WallView`, `DoorView`, `CreatureView`, `PropView`, `HexView`, plus the provisional `NoteView`, `StartView` and `PropsView`.
- `encounter-studio/StudioArrangePanel.tsx`: `SelectedArrange` splits into the per-kind field forms the views reuse. The form keeps `aria-label="Arrange selected noun"` and the one-transaction submit. The empty-selection branch and the "Arrange" heading are removed.
- `encounter-studio/StudioDimensions.tsx`: rendered in `RoomView`; `onDismiss` becomes optional and the form stays open after Apply.
- `encounter-studio/studioSession.ts` and `WorldBuildingConcept.tsx`: the session gains `armPartyStart(): boolean`, which wraps the owner's `armStartPlacement` with `guardSnapshotIntent` and `refuseWhilePublishing`.
- Tests: `StudioInspector.test.tsx`, `StudioArrangePanel.test.tsx`, `EncounterStudioIntegration.test.tsx`.

**Interfaces:** each view takes `{ session, selection }` narrowed to its kind. Field edits use the existing intents only:

| view | fields, in order | intents |
|---|---|---|
| Room (nothing) | Size; Notes (the existing `useStudioLabels` controls: create, rename, move, delete); Party start (cell or "not placed", and Place party start); Play: void, lighting, standing, read-only (Q3) | `resizeWorkspace`, the label intents, `armPartyStart` |
| Wall | finish (`StudioWallFinishes`); appearance (`StudioWallAppearanceChoices`); start, end, length, fixed endpoint, height and thickness (`arrangeFields`); constraints (`StudioWallConstraints`); Place a door; Remove wall | `commitArrange` `wall-edit` and `wall-remove`; Place a door calls `doorEditing.setActive(true)` and arms the Door tool |
| Door | position, width, model; Remove door | `commitArrange` `door-edit` and `door-remove` |
| Creature | map name; look (family, style, skin palette); cell and facing; Move; then `socialControls.selection` (`CreatureOrders`: faction, table, own rows, temper, weapons, says, holds, arrives) and the authored-dispositions list; then links to the Encounter home definitions | `commitArrange` `actor-start`; the binding intents behind `CreatureOrders` |
| Prop | position, rotation, height; `propControls.arrangeExtras` (duplicate, delete, group, ungroup, light); the declaration editor and `PropOrders` from `socialControls.selection` | `commitArrange` `scene-edit`; the prop binding intents |
| Hex | cell coordinates; walkable or not; the drawn regions at the cell with their lighting, each with "Edit region", which opens the Regions home with that region's label selected | read-only in S0; `mapLabelSelection.select` |
| Note, Start, Props (provisional) | exactly their current Arrange fields | as today |

**Behavior:**
- A view shows only its own selection's fields. A faction's table, a table's rows and intel records are never edited from a creature or prop view; the links open the Encounter home (README refusal).
- A refused submit leaves the form's staged values and shows the refusal in `role="alert"` inside the view, as `#arrange-error` does today.
- A hex with no drawn region says "No region is drawn here". An unwalkable hex says so.
- The room view states play's values as the dialect's fixed contract: "Void: transparent · Lighting: bright · Standing: centre-covered".

**Tests:**
- `StudioInspector.test.tsx`, one per view: `room view shows size, notes, party start and the fixed play values`; `wall view shows finish, endpoints, height and Place a door, and Place a door arms the door tool`; `door view shows position and width and removes the door through one intent`; `creature view shows look, facing and the creature orders form`; `prop view shows position, the arrange extras and prop orders for a declared prop`; `hex view names the region and lighting and opens that region`; `hex view with no region says none is drawn`.
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

## Task T6: The workspace grows up and to the right

**Delivers:** I6, A1. I7 is blocked on Q6.
**Owner:** rpg-dnd5e-web; `world-building/workspaceGeometry.ts`, `roomDraft.ts`, the kind's consumers, `StudioDimensions.tsx`.
**Prerequisites:** none. It may run in parallel with T1–T5 on its own worktree.
**Files:**
- `world-building/workspaceGeometry.ts`: `RoomWorkspace` gains the kind `odd-r-rect`; new `rectangleOf`, `anchoredRoomWorkspace` and `growRoomWorkspace`; `validateWorkspace`, `containsWorkspaceCell`, `workspaceCells` and `workspaceBounds` accept it.
- Every consumer listed in the baseline replaces `kind === 'centered-odd-r'` with `rectangleOf(workspace) !== null` and reads `minCol`, `minRow`, `widthHexes` and `heightHexes` from the result.
- `world-building/roomDraft.ts`: `resizeRoomWorkspace` calls `growRoomWorkspace`; the version gate that requires scene version 2 for a centred workspace applies to `odd-r-rect` too.
- `encounter-studio/StudioDimensions.tsx`: reads the rectangle through `rectangleOf`; after Apply it frames the workspace centre (`workspaceBounds`), not the origin; the help line says "Adds columns on the right and rows on top".
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

export function growRoomWorkspace(
  current: RoomWorkspace, widthHexes: number, heightHexes: number
): RoomWorkspace;
// Keeps minCol. Sets minRow = old minRow - (heightHexes - old heightHexes).
// So the right edge and the top edge move, and the left and bottom edges stay.
// A legacy radius workspace is not a rectangle, so it has no edges to keep: it is
// replaced by centeredRoomWorkspace(widthHexes, heightHexes), exactly as resize does today.
```

**Behavior:**
- Growing keeps every cell's `{q, r}`. Walls, props, labels and regions hold world points or cells and are not rewritten.
- Shrinking moves the same two edges inward (A1). `validateWorkspaceContent` still refuses a shrink that leaves content outside, with today's message.
- A `centered-odd-r` document still decodes unchanged. Its first resize saves `odd-r-rect`; an unresized document saves byte-identical.
- `validateWorkspace` refuses an `odd-r-rect` whose `hexRadius` or `horizontalLimit` does not match its derivation, with the sentence the centred kind uses.
- The toolkit's floor check passes because `hexRadius` covers every cell by derivation.

**Tests:**
- `workspaceGeometry.test.ts` › `growing 20x20 to 24x22 keeps every old cell and adds 4 columns right and 2 rows on top`: every old cell is in the new workspace; the new cells are exactly those with `col >= oldMaxCol + 1` or `r < oldMinRow`.
- › `shrinking moves the right and top edges`; › `a centred workspace reads as its rectangle`; › `a mismatched hexRadius on odd-r-rect is refused`; › `hexRadius covers the farthest cell of an off-centre rectangle`.
- `roomDraft.test.ts` › `resize keeps a monster's starting cell, the party start and every walkable cell`; › `a shrink that cuts a wall is refused with no history entry`.
- `singleRoomDungeon.test.ts` › `an off-centre workspace encodes and round-trips through parse`; › `an unresized centred document encodes byte-identical`.
- `EncounterStudioWorkspace.test.tsx`: the staged-dimensions tests updated; › `apply frames the new workspace centre`.

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

1. Layout, nothing selected: no Arrange, Walls, Doors, Notes or Size buttons. The inspector reads Room, with size, notes, party start and the play values.
2. Arm Wall from the toolbar, choose an appearance in the inspector, and draw two walls. Press Escape.
3. Press N to hide the inspector. Click a wall: the inspector opens on it. Change its height and press Undo. The old height returns, and the wall stays selected.
4. Press Place a door, then click the wall. Click the new door: the door view shows position and width.
5. Click an empty floor hex: the hex view names the cell and says whether it is walkable. Press Escape: Room.
6. Switch to 3D. Drag a prop from the palette onto the floor: the prop view opens. Place a creature from the Monsters and NPCs palette, click it, and see its look, facing and orders.
7. Open the Encounter home, then Creatures. The creature is pressed in Placed instances. Click Show on map: Build, with the creature selected and the inspector open.
8. In the room view, set the size from 20 by 20 to 24 by 22. New columns appear on the right and new rows on top. The walls, prop, creature and floor are where they were.
9. Save the local draft and reload. Everything above is still there.

**Verification:** `ci-check` log clean; nine screenshots; the checklist on #574 ticked.
**Completion evidence:** the PR body carries the `ci-check` output, the screenshot list and the T6 toolkit decode output.

---

## Requirement coverage

| requirement / scenario | task(s) | proof |
|---|---|---|
| I1 tabs and Size retire | T3, T7 | `Build has no Arrange, Walls, Doors or Notes buttons and no Size button`; the StudioSidebar grep; walk 1 |
| I2 one view per kind | T1, T4 | `studioSelection.test.ts` mappings; one `StudioInspector.test.tsx` case per view |
| I3 click opens, N toggles | T2, T3 | `selecting a different thing reopens a hidden inspector`; the retargeted N test; walk 3 |
| I4 one selection | T1, T2 | `StudioSelection` has no set except the provisional `props` (Q7); `select click on a wall selects the wall, not a hex` |
| I5 one selection across homes | T5 | `a placed instance selected on the home is the map selection with the inspector open`; `a creature selected on the map is pressed in the home's placed list`; walk 7 |
| I6 grows up and right | T6 | `growing 20x20 to 24x22 keeps every old cell…`; `resize keeps a monster's starting cell…`; walk 8 |
| I7 another side, with warning | none | **blocked on Q6**; not a deferral |
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
| #574 scenario 6 | T6 | walk 8; the other-side half is blocked on Q6 |

**Explicit deferrals:** the editable play contract (Q3), the room-wide
default finish (Q4), and the creature view's read-back prose, which is S2.

## Provider/consumer seams

| provider | consumer | produced vs consumed contract | availability | proof |
|---|---|---|---|---|
| T1 `StudioArrangeTarget` `hex` and its projection | T2 owner intents, T4 `HexView` | `{kind:'hex', cell}` with `walkable` and `regions`; null when outside the workspace | new in T1 | `projects a hex inside the workspace…`; `masks a hex outside the workspace` |
| T1 `studioSelectionOf` | T3 `StudioInspector` key, T4 views | eight variants; `nothing` for null | new in T1 | `studioSelection.test.ts` |
| T1 `regionsAtCell` | T4 `HexView` | resolved regions at the cell, authored order; an annotation | new in T1, over the existing `pointInRing` | `regionAtCell.test.ts` |
| T2 `selectHex`, `clearSelection` on the session | T2 `LayoutViewport` and `WorldBuildingViewport` | boolean; false without a history entry on refusal | new in T2, through the existing `activateStudioTarget` | `selectHex refuses a cell outside the workspace…` |
| T4 `armPartyStart` | T4 `RoomView` | boolean; wraps the existing `armStartPlacement` | the owner function exists; the session member is new | `room view shows … party start` |
| existing `commitArrange` and `StudioArrangeIntent` | T4 views | unchanged; one transaction per form | exists at the baseline | existing `StudioArrangePanel.test.tsx` cases |
| existing `socialControls.selection` and `propControls.arrangeExtras` | T4 creature and prop views | unchanged ReactNodes from the owner | exist at the baseline | `creature view shows … creature orders form`; `prop view shows …` |
| existing `session.creatures.select` | T5 | unchanged | exists at the baseline | `a placed instance selected on the home…` |
| T6 `odd-r-rect`, `rectangleOf`, `growRoomWorkspace` | T4 `RoomView` through `resizeWorkspace`, and every listed consumer | rectangle fields; `hexRadius` derived | new in T6 | `growing 20x20…`; codec round-trip |
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
  rerender does not. T6's growth formula moves `minRow` down by the added
  height, which matches the growth test's new-cell set.
- **Unowned requirements.** I7 has no task and is blocked on Q6. That is a
  blocker, not a deferral.
- **Persistence.** T6 changes the saved workspace and is covered by the
  codec round-trip, the byte-identical test and the toolkit decode. T1–T5
  add no saved data; the selection is never persisted.
- **Multiplayer events, generated bindings, pins.** Not applicable. No wire
  is touched.
- **Removal of replaced paths.** T3 retires the sections; T7 greps for the
  leftovers.
- **Decisions left open.** Q1, Q2, Q7 and A2 carry assumptions that keep
  today's behaviour or the narrowest reading. Q3, Q4 and Q6 block exactly
  the fields named.

## Later slices

- S1, the cast and per-placement looks: rpg-project#575.
- S2, read it back: prose, fact index, tables heaviest first: rpg-project#576.
- S3, overview on Layout: rpg-project#577.
- S4, the hidden nouns: exits, endings, concealments, door state: rpg-project#578.
- S5, density: words, wrapping, panel width: rpg-project#579.
