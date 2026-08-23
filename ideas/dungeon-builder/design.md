# Dungeon Builder on the Session Stack — design

*Journey: rpg-project#169. Ruling 2026-08-23 (Kirk): fresh start from the new
`rulebooks/dnd5e/encounter/dungeonspec`, the way the composable encounter got
one. Spec v0.4 / PR #203 is history. Regions are authored and carry lighting;
an audio profile attaches to regions later. Survey that led here:
`restart-survey-2026-08-23.md`.*

**One sentence:** build a dungeon in the builder, then play through it — and
the thing you drew is, cell for cell and edge for edge, the thing the session
serves.

## 0. The shape in three lines

- **The author paints regions on one canvas, draws edges, places things.** No
  rooms, no chain, no room-local frame. Every coordinate in the file is one
  absolute `[col,row]` under one declared orientation.
- **The compiler makes the world; the atlas is the proof.** `PutDungeon`
  compiles and answers with the same `GetAtlas` shape the game plays from. The
  builder's preview renders that atlas through the game's own
  `buildScene3D`. If the preview and the game ever differ, one of them is
  lying and the test says which.
- **A region is the carrier of per-area world facts.** Its cells, its
  lighting today, its audio profile later. Regions go on the atlas because
  lighting is a fact the player's sight needs, not a render hint.

## 1. The panel — what the builder shows and does

One route, `/author`, the existing `src/author/` shell kept. Three columns.

**Left — palette.** Tools, top to bottom:

| Tool | Does | Writes |
|---|---|---|
| **Region brush** | paint/erase cells into the selected region; new region = new glyph | `map:` + `regions[]` |
| **Wall** | click an edge between two adjacent floor cells to toggle a wall | `walls[]` |
| **Door** | click an edge (or drag across several) to make a doorway; inspector sets open / closed / locked `{dc, ability}` | `doors[]` |
| **Start** | one cell; the party's entry | `start` |
| **Props** | catalog from `rpg-game-assets` manifest (existing thumbnails); drop on a floor cell; inspector shows `blocks_movement` / `blocks_los` prefilled from the catalog, **always written explicitly** | `place[]` |
| **Monsters** | catalog; drop; inspector: `targeting`, `boss` | `place[]` |

**Center — the canvas.** The existing 2D hex board (`CreationBoard.tsx`), one
orientation per dungeon (`pointy` / `flat`, set at New). Void is everything
unpainted. Floor/void boundary draws as the envelope automatically — the
author never draws an outer wall.

**Right — inspector + YAML.**
- **Region inspector** (when a region is selected): `id`, `name`, `lighting`
  (`bright` / `dim` / `dark`). Later rows: `audio`.
- **Dungeon inspector**: `key`, `name`, `orientation`, `void` (`opaque` /
  `transparent`).
- **YAML pane**: the file, read-only mirror of the canvas, with Download /
  Load. The YAML is the artifact; the canvas is a view of it.
- **3D preview** tab: the atlas `PutDungeon(validate_only)` returned, drawn by
  the game's renderer. Orbit + the game's tactical camera.

**Top bar — verbs.** `New` · `Open` (picker over `ListDungeons`, loads via
`GetDungeon`) · `Save` (`PutDungeon`) · **`Save & Play`** (`PutDungeon`, then
the lobby's `StartEncounter{dungeon_key}` — the real game route opens on the
authored dungeon). Problems from the compiler appear on the cell/edge they
name (`field_errors` carry a path) and the Save verb stays disabled until the
file compiles; `validate_only` never refuses a half-drawn map.

**What the panel does not have (Not now):** facing, height, intra-cell
offsets, mount points, prop catalog editing, region nesting, free-roam
walkthrough, DM mode, a second dungeon open at once.

## 2. The YAML — `dungeonspec` version 2, field by field

Version 2 because the topology changed (canvas + regions replaces the room
chain) — the one kind of change the old versioning rule reserved a bump for.
Version 1 is deleted, not supported; the reference tomb is re-authored in
version 2 and compiles to the identical atlas (§5). No `spec:` marker.

```yaml
version: 2
key: reference-tomb            # [a-z0-9-]; the file's identity
name: The Reference Tomb       # display only
orientation: pointy            # pointy | flat — REQUIRED; every [col,row] below depends on it
void: opaque                   # opaque | transparent — REQUIRED

# The floor. One glyph per cell, one line per row, row 0 at the top.
# '.' is void. Any other printable glyph names a region via `regions`.
map: |
  EEEEEEHHHHHHHHHHTTTTTTTTTTTT
  EEEEEEHHHHHHHHHHTTTTTTTTTTTT
  EEEEEEHHHHHHHHHHTTTTTTTTTTTT
  EEEEEEHHHHHHHHHHTTTTTTTTTTTT
  EEEEEEHHHHHHHHHHTTTTTTTTTTTT
  EEEEEEHHHHHHHHHHTTTTTTTTTTTT
  EEEEEEHHHHHHHHHHTTTTTTTTTTTT
  EEEEEEHHHHHHHHHHTTTTTTTTTTTT

regions:
  - { glyph: E, id: entrance, name: Entrance, lighting: dim }
  - { glyph: H, id: hall,     name: Hall,     lighting: dim }
  - { glyph: T, id: tomb,     name: Tomb,     lighting: dark }

start: [1, 3]                  # absolute; must be floor

# Edges. Each is two ADJACENT floor cells. Walls block both movement and sight.
walls:
  - [[5,0],[6,0]]
  - [[5,1],[6,1]]
  # ... the hall/tomb seam etc. The builder writes these; a human can too.

# A door is one state over one or more edges (toolkit#1123).
doors:
  - id: hall-tomb
    edges: [[[15,3],[16,3]]]
    locked: { dc: 12, ability: dex }   # omit for an open doorway; `closed: true` for shut-not-locked

place:
  - { ref: "dnd5e:props:brazier",  at: [1,1],  blocks_movement: true,  blocks_los: false }
  - { ref: "dnd5e:props:pillar",   at: [8,2],  blocks_movement: true,  blocks_los: true }
  - { ref: "dnd5e:monsters:skeleton", at: [11,3], targeting: lowest-health }
  - { ref: "dnd5e:monsters:skeleton-captain", at: [23,5], targeting: closest, boss: true }
```

Field rules, carried over from version 1 unchanged: strict decode (unknown
key fails), pointers for "the author may leave this out" (`start`,
`blocks_*`), `blocks_*` REQUIRED on props and REFUSED elsewhere, refs never
resolved, `targeting` opaque, at most one `boss` per region, `key` must match
the requested key on `PutDungeon`.

New rules:

- **`map` is the floor.** Width = longest line; short lines are padded with
  void. A glyph with no `regions` entry fails; a `regions` entry with no cells
  fails. Every floor cell belongs to exactly one region (the glyph says which).
  Flat — no nesting (Not now).
- **The envelope is implied, never written.** A crossing from floor into void
  is a crossing nobody can make; `void` already says whether sight crosses it.
  That is the runtime's rule today (`seamWall`'s comment) — the file just stops
  pretending otherwise.
- **`walls` and `doors` are edges between adjacent floor cells.** Adjacency is
  checked by spatial under the declared orientation. An edge listed twice, or
  in both `walls` and a door, fails. A door edge need not sit on a region seam
  — a door inside a room is legal.
- **`lighting` is one of `bright` / `dim` / `dark`** — 5e's three words,
  because they are the fact a player's perception needs (dim = lightly
  obscured, dark = heavily obscured). Carried opaquely by the composition like
  `targeting`; the rulebook interprets, the client maps to render intensity.
  REQUIRED per region (no default, per #1033).
- **Coordinates are absolute offset `[col,row]`** under `orientation`. The
  compiler converts once (`HexCellAt`); no caller ever adds an origin. The
  room-local → absolute seam (toolkit#1139) **ceases to exist** rather than
  getting fixed.

## 3. The wire — field by field

### 3a. `authoring.v1alpha1` — replaced, not grown

The existing `AuthoringService` describes the deleted dialect (`FloorPlan`,
`door_row`, `archetype`). Under no-backcompat it is deleted and redefined:

```proto
service AuthoringService {
  rpc PutDungeon(PutDungeonRequest)  returns (PutDungeonResponse);   // validate | save
  rpc GetDungeon(GetDungeonRequest)  returns (GetDungeonResponse);   // reopen
}
message PutDungeonRequest  { string key = 1; string yaml = 2; bool validate_only = 3; }
message PutDungeonResponse {
  repeated FieldError errors = 1;              // path + message; empty = compiled
  dnd5e.api.session.v1alpha1.GetAtlasResponse atlas = 2;  // THE SAME MESSAGE THE GAME PLAYS FROM
}
message FieldError { string path = 1; string message = 2; }   // "walls[3]", "place[7].blocks_los", "map:4:17"
message GetDungeonRequest  { string key = 1; }
message GetDungeonResponse { string yaml = 1; }
```

`PutDungeon` returns the atlas — not a builder-shaped `FloorPlan` — so the
builder has no second geometry to keep in step with the game. `DeleteDungeon`
is Not now.

### 3b. `session.v1alpha1.GetAtlasResponse` — grows regions

```proto
message AtlasRegion {
  string id = 1;
  string name = 2;
  repeated Position cells = 3;    // absolute axial, sorted — same frame as `cells`
  Lighting lighting = 4;          // BRIGHT | DIM | DARK
}
message GetAtlasResponse {
  // ... grid, layout, cells, props, boundaries, doorways unchanged
  repeated AtlasRegion regions = 9;
}
```

Additive. Regions join the atlas because lighting is a world fact — a client
that cannot read "this hall is dark" off the wire re-derives it by experiment
(the ADR-0040/0041 argument). Audio will be a second field on `AtlasRegion`.
Every floor cell appears in exactly one region's `cells`.

### 3c. `lobby.v1alpha1` — shapes stay, behaviour starts

`StartEncounterRequest.dungeon_key` is honoured: unknown key →
`NotFound`, never the tomb. `ListDungeons` answers from the registry —
ungated, because a picker needs it with authoring off.

## 4. What the stack answers today vs. what it must grow

| Layer | Today | Grows |
|---|---|---|
| **toolkit `dungeonspec`** | v1 room chain, hex only, seam walls generated | **v2**: `map`/`regions`/`walls`/`doors`/absolute `place`; `Validate` reports path-addressed errors; `Compile` → `FieldInput` |
| **toolkit `encounter`** | `FieldInput{Canvas, Rooms[rect+Origin], Connections, Doors}`; a room is a region; `RegionAt` is the mask | `RegionInput{ID, Cells, Lighting}` replaces `RoomInput`+`Origin`+`Connections` (a connection was a chain artefact; a doorway is already two cells). `CanvasInput` keeps `Void`; lighting lands on the region, not the canvas (closes toolkit#1113 by relocation). `Sight` capability receives the region's lighting word. `EncounterData` carries regions + lighting |
| **toolkit `session`** | `Atlas{cells, props, boundaries, doorways, layout}` | `Atlas.Regions` |
| **protos** | authoring = dead dialect; atlas has no regions | §3 |
| **rpg-api** | one `go:embed` tomb; `dungeon_key` dropped; `ListDungeons` unimplemented; projection borrowed via throwaway encounter | `internal/dungeons` registry: loads every YAML under `RPG_CONTENT_DIR` at boot, compiles each once, **refuses to boot on a file that does not compile**; `PutDungeon` gated by `RPG_AUTHORING_ENABLED` — validate → compile → write-through → atomic swap, puts serialised; `GetDungeon` reads the file; `ListDungeons` from the registry; `StartEncounter` looks the key up. The throwaway-encounter projection is deleted (no room-local frame remains). The tomb ships as `content/reference-tomb.yaml`, not an embed |
| **web** | `/author` speaks the dead proto; preview has its own wall renderer; builder is odd-q, wall runs odd-r | `/author` emits v2; preview = `buildScene3D(atlas)` + the session leaf renderers; one coordinate bridge (`positionBridge.ts`), the builder's odd-q layout deleted; `DungeonPicker` live; `Save & Play` calls `StartEncounter{dungeon_key}` |

Deleted outright: old `rpg-toolkit/encounter/dungeonspec` (zero consumers),
`src/author/{straightWallGeometry,hexLayout,specCompat,capabilityProbe,
specimens}`, `CONTRACT.md`, `TARGET-YAML.md`.

## 5. The forcing case and the tests that guard it

**Forcing case:** the reference tomb, re-authored in version 2 by hand, compiles
to the **same atlas** the v1 embed produced — same 224 cells, same boundaries,
same doorway, same props — with `regions` added. Pinned as a golden test in
`dungeonspec` before v1 is deleted.

**Round trip:** builder `New` → paint three regions → walls → locked door →
props → `Save & Play` → the 3D game route opens on it → walk entrance to
hall → `Open` reloads the same YAML byte-for-byte.

**Discriminators** (the symmetric-bug lesson): a pixel-formula test that draws
an L-shaped region under both orientations and checks one named cell's world
position in the builder and in `buildScene3D` — not a round-trip.

**Error paths:** a glyph with no region → `map:2:7`; a wall between
non-adjacent cells → `walls[3]`; a prop with no `blocks_los` →
`place[4].blocks_los`; `start` on void → `start`. Each appears on the canvas at
the thing it names.

## 6. Sequence

1. **This doc** — Kirk rules once (the rulings in §7).
2. **Protos first** (one PR, additive to session, replace authoring): §3.
3. **In parallel against the proto:** toolkit `dungeonspec` v2 +
   `RegionInput` + `Atlas.Regions` · rpg-api registry + RPCs · web `/author`
   retarget + preview on `buildScene3D`.
4. **Kirk walks the branch once:** branch api on :50051, branch web on :3003 —
   New → paint → Save & Play → play.
5. **Merge bottom-up:** toolkit → protos already in → rpg-api → web, re-pinning
   to real tags.

Board: this is a slice under journey #169 with rpg-api#806 (rewritten to this
design), toolkit#1139 (closed: seam removed), rpg-project#131 (picker,
delivered by `ListDungeons`), and the new repo issues as sub-issues. PR #203
and the v0.4 Wave A/B issues close as history.

## 7. Rulings needed

1. **`map:` glyph block as the floor format** (recommended — readable, diffable,
   a human can author the tomb in eight lines) vs. `regions[].cells` lists.
2. **Lighting vocabulary `bright | dim | dark`** (recommended — the fact
   perception needs) vs. a 0–1 intensity.
3. **`RegionInput` replaces `RoomInput`** in the composition (recommended — a
   region is already what survives the compile; rectangles were the chain's
   constraint) vs. keeping rectangles and compiling a painted region into
   several.
4. **Version 1 deleted with the tomb re-authored** (recommended, per
   no-backcompat) vs. keeping v1 readable.

## 8. Not now

Facing / height / offsets / mounts (need their own wire fields — parked under
#131 already); region nesting; audio (the slot is designed, the field is not
cut); square grids (the composition accepts them; the dialect would need
`grid:`); `DeleteDungeon`; prop catalog authoring; free-roam walkthrough; DM
mode; hosted deployment.
