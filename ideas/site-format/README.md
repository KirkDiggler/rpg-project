# Site format — walkthrough

Story: rpg-project#725. Law: [design.md](design.md). This explains how the
pieces meet; the design doc says what holds.

## What this is

A site's floor becomes the union of its authored regions, and a region's cells
are written as row runs instead of one mapping per cell. It reuses the
encounter's existing region input, which already wants a name, an archetype
and a lighting level per region, and the existing wall partitioning. It
introduces **no second floor authority** (the walkable list goes away), **no
shape-derived floor** (a run is a listing, not a fill), and **no mechanical
meaning for an authored boundary** (R4).

## Component shape

```mermaid
flowchart TB
    subgraph web [rpg-dnd5e-web]
        Paint[paint tools] -->|cells into the active region| Draft[RoomDraft.regions]
        Draft --> Encode[encodeSingleRoomDungeon]
        Encode --> Yaml[site YAML]
        Grid[grid overlay] -.derives from content bbox.-> Draft
    end
    Yaml -->|AuthoringService.PutDungeon| api[rpg-api dungeons registry / world store]
    api --> Decode[dungeonspec.DecodeSingleRoom]
    subgraph toolkit [rpg-toolkit encounter]
        Decode -->|[]RegionInput| CompileRoom[dungeonspec.CompileSingleRoom]
        CompileRoom --> Field[encounter.compileField]
        Field --> Owner[owner: cell → region id]
        Field --> Part[partitions: cell → partition id]
        Owner --> AtlasFor[encounter.AtlasFor]
        Part -.-> Hearing[noise.go links / hopsFrom / hearersOf]
        Part -.-> Seats[dungeonspec seat derivation, reserve.go arrivalCell, flip.go sweepPresence]
    end
    AtlasFor -->|GetAtlasResponse regions + cells| Scene[atlasToScene3D]
    Scene --> Floor[floor per region, lit per region]
```

## Ownership and contracts

| Component | Owns | Input → output |
|---|---|---|
| Studio paint tools (`rpg-dnd5e-web`) | which region a brushed cell joins | pointer gesture + active region → `RoomDraft.regions[i].cells` |
| `RoomDraft` (`concepts/world-building/roomDraft.ts`) | the authored document in memory: regions, walls, props, bindings | edits → a draft the encoder accepts; no walkable list, no workspace dimensions |
| `encodeSingleRoomDungeon` / `parseRoomDocumentJson` | the YAML dialect on the web side | draft ↔ YAML with `regions[].cells` as row runs and optional `construction` |
| Grid overlay (`LayoutViewport`, `WorkspaceCellOverlay`) | what grid the author sees | content bounding box + margin → drawn cells; never stored |
| `AuthoringService.PutDungeon` (`rpg-api`) | storing the text verbatim, compiling for the preview | `{key, yaml, validate_only}` → `{errors, atlas}` |
| `dungeonspec.DecodeSingleRoom` (`rpg-toolkit`) | the strict read of the dialect | YAML → `SingleRoomSpec` with `Regions []RoomRegion`, runs expanded to `[]RoomCell` |
| `dungeonspec.CompileSingleRoom` | turning authored regions into compiler input | `SingleRoomSpec` → `[]encounter.RegionInput`, one per region, implicit region included |
| `encounter.compileField` | the floor, cell ownership, footprint and footing subtraction | `[]RegionInput` + walls + props → field with `owner`, `regionCells`; partitions derived from it at construction and at load, never stored |
| `encounter.AtlasFor` | what a seat may know | field + knowledge → `GetAtlasResponse` with `regions[].cells`, each region carrying its lighting |
| `atlasToScene3D` (`rpg-dnd5e-web`) | geometry from the atlas | atlas → floor tiles grouped by region, ambient per region |

Refusals, one per boundary:

- The studio does not compute the floor from walls or from a shape; it stores
  the cells the author painted.
- The encoder does not write workspace dimensions, a walkable list or a label
  list; the file carries regions and the frame.
- The decoder does not accept a cell in two regions and does not infer a
  region for an unpainted cell.
- `CompileSingleRoom` does not read a region's construction; it reads the runs.
- `compileField` does not let an authored edge block, cost or count as a hop;
  partitions come from walls only.
- `AtlasFor` does not send partitions; the atlas carries authored regions only,
  and the field answers `PartitionAt` for the readers that decide outcomes.
- The renderer does not derive gameplay from region membership; it lights and
  groups by it.

## Walk one thing through

The gatehouse. The author selects the region named Gatehouse, sets its lighting
to dim, and brushes the 54 cells between the two gate doors.

1. **Studio.** Each brushed cell moves from whichever region held it into
   `regions[gatehouse].cells`. The cell set is a sorted array keyed `q,r`; the
   label on the map is the region's name.
2. **Encoder.** The region becomes
   `{id: region-gatehouse, name: Gatehouse, lighting: {intensity: 0.3},
   cells: [{r: -6, q: [[-2, 2]]}, {r: -5, q: [[-3, 3]]}, ...]}`. Six rows, a
   handful of spans. The identity that survives is the region id and the cell
   coordinates; the span boundaries are derived from adjacency and carry no
   meaning of their own.
3. **PutDungeon.** The text is stored as sent. The preview compiles it.
4. **Decoder.** The runs expand to 54 `RoomCell{Q, R}`. A cell appearing in a
   second region is refused with the path of both regions.
5. **CompileSingleRoom.** One `RegionInput{ID: region-gatehouse, Name,
   Cells: 54 offset positions, Archetype, Lighting: &dim}` beside the other
   regions and the implicit one.
6. **compileField.** The 54 cells enter `owner` under the gatehouse id. The
   wall footprints of the two gate walls are subtracted. Partitioning by walls
   then finds the gatehouse and the courtyard share one partition, because no
   wall separates them. The cell now has region = gatehouse, partition = the
   yard's.
7. **AtlasFor.** The seat that has seen those cells receives
   `regions[gatehouse]{cells, lighting: dim}`; the hearing hop count between
   the gatehouse and the courtyard is zero.
8. **Scene.** The 54 tiles draw in the gatehouse group with dim ambient. A
   clang in the courtyard is heard in the gatehouse with no door DC added.

## Separations that look like one thing

| Query | Source of truth |
|---|---|
| Is this cell floor? | membership in any region's cells, minus footprints, in the compiled field |
| Which place is this cell, for light and sound? | the cell's authored region id |
| Can sound or an alarm cross here, at what cost, and which cells may seat the party? | the cell's wall partition and the doors between partitions |
| What does the author see as the grid? | the content bounding box plus margin, computed in the studio |
| Where did the author draw a rectangle? | the region's construction, read by the studio only |

An implementation that reads lighting from the partition lights an open hall
uniformly even when the author painted it as two places. One that reads hearing
from the authored region adds a door hop where no door exists. One that stores
the grid in the file makes the working canvas the shipped canvas.

## Trade-offs

| Decision | Benefit | Cost / boundary | Not taken |
|---|---|---|---|
| Floor is the union of regions (law) | one list instead of two; every cell has a place to take lighting from | the author must accept that unpainted cells are not floor; the implicit region exists so a new site needs no region step | keep `walkableHexes` beside regions |
| Row runs canonical (R2) | lossless, order-free, forty-odd runs for the castle; same cost whichever side is sparse | the decoder expands runs; the author's rectangle is not recoverable from runs alone | rectangles plus exception cells as the canonical grammar; bitmap; outline with fill |
| Construction optional alongside (R2) | a hall can be resized later | the studio pays to keep construction and runs consistent and to report a split | runs only |
| Unbounded canvas (R3) | no empty cells stored, no trim step, no "which side is sparse" | the studio derives its grid and keeps the view locally | workspace dimensions in the file |
| Regions and partitions independent (R4) | an author can divide an open space for ambience without changing sound or sight | the field derives partitions on every construction and load; the compiler reads lighting through the region id | partitions nested inside authored regions; partitions persisted in the snapshot |
| Brush assigns regions (R5) | one gesture for floor and place | no outline tool at first | an outline tool over painted floor |
| Convert once (R6) | no dual dialect to maintain | one script run, three pins regenerated | accept both dialects for a while |

## Edges

- A region's cells are a set in one frame. A site spanning two frames or two
  levels is not modelled (O3 for placed rooms).
- Sound is a named slot, not a field, until its use case (O1).
- The construction's schema is undefined until the studio slice emits one
  (O2).
- The wire is per cell. Nothing here changes `GetAtlasResponse` (O5).
- The runtime still floods the whole field per step. The region graph as a
  pathfinding level is a sibling story (R7).

## Where a change lands

- **Sound on a region.** One field in `RoomRegion`, one in `RegionInput`, one
  on `AtlasRegion`, and the renderer plays it. Four owners, each a field, no
  seam moves.
- **An author wants to label part of a hall without changing its light.**
  Paint a second region with the same lighting. Nothing changes; R4 holds.
- **A placed, reusable room.** Lands in the studio and the encoder as a new
  document kind that expands to regions before `CompileSingleRoom` sees it.
  The compiler and the atlas do not change. That is O3's story.
- **A bigger castle.** Nothing changes; the file grows by runs, not by cells.

## Source map

| Concern | Path |
|---|---|
| Authored dialect, decode and compile | `rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec` — `SingleRoomSpec`, `RoomRegion`, `DecodeSingleRoom`, `CompileSingleRoom` |
| Compiler input and field | `rpg-toolkit/rulebooks/dnd5e/encounter` — `RegionInput`, `compileField`, `PartitionRegion` |
| Atlas | `rpg-toolkit/rulebooks/dnd5e/encounter` — `AtlasFor`, `AtlasRegion` |
| Content goldens | `rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec/testdata`, `contentgolden_test.go` |
| Shipped content and pins | `rpg-api/content`, `rpg-api/internal/dungeons` — `castle_copies_test.go`, `atlas_fixture_test.go` |
| Authoring RPC | `rpg-api-protos/dnd5e/api/authoring/v1alpha1/service.proto` — `PutDungeon`, `GetDungeon` |
| Studio draft, encoder, decoder | `rpg-dnd5e-web/src/concepts/world-building` — `roomDraft.ts`, `singleRoomDungeon.ts`, `serialization.ts` |
| Studio grid and paint | `rpg-dnd5e-web/src/concepts/encounter-studio` — `LayoutViewport.tsx`, `overviewFrame.ts`; `src/concepts/world-building` — `WorkspaceCellOverlay.tsx`, `workspaceFloorGeometry.ts` |
| Play scene from the atlas | `rpg-dnd5e-web/src/components/session` — `atlasToScene3D.ts` |
