# Dungeon Builder on the Session Stack — design

*Journey: rpg-project#169. Rulings 2026-08-23 (Kirk): fresh start from the new
`rulebooks/dnd5e/encounter/dungeonspec`, the way the composable encounter got
one; spec v0.4 / PR #203 is history. Hex only, orientation authored. Regions
replace rooms — there are no rooms in the atlas projection. A region carries
lighting as a dimmable level now, with a ref for the asset kind (flame glow,
etc.) later, and an audio profile later. Version 1 is deleted; every combat
fixture moves to version 2. Survey that led here:
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
  `buildScene3D` (extended to take `layout` and `props`, which it does not
  today — plan W). If the preview and the game ever differ, one of them is
  lying and the test says which.
- **A region is the carrier of per-area world facts.** Its cells, its
  lighting today, its audio profile later. Regions go on the atlas because
  lighting is a fact the player's sight needs, not a render hint.

## 1. The panel — what the builder shows and does

One route, `/author`, the existing `src/author/` shell kept. Three columns.

**Left — palette.** Tools, top to bottom:

| Tool | Does | Writes |
|---|---|---|
| **Region brush** | paint/erase cells into the selected region | `regions[].cells` |
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
  as a dimmer slider (`level` 0–1). Later rows: lighting `ref` (what kind of
  light the assets should show — flame glow, etc.), `audio`.
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
name (`errors` carry a path) and the Save verb stays disabled until the
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

# The floor is the union of the regions' cells. Everything else is void.
# Cells are absolute [col,row]; the builder writes them sorted, one row per
# line, so a repaint diffs as a line change (emitter convention, not grammar).
regions:
  - id: entrance
    name: Entrance
    lighting: { level: 0.6 }
    cells:
      - [[0,0],[1,0],[2,0],[3,0],[4,0],[5,0]]
      - [[0,1],[1,1],[2,1],[3,1],[4,1],[5,1]]
      # ... rows 2–7
  - id: hall
    name: Hall
    lighting: { level: 0.4 }
    cells:
      - [[6,0],[7,0],[8,0],[9,0],[10,0],[11,0],[12,0],[13,0],[14,0],[15,0]]
      # ...
  - id: tomb
    name: Tomb
    lighting: { level: 0.15 }
    cells:
      - [[16,0],[17,0],[18,0],[19,0],[20,0],[21,0],[22,0],[23,0],[24,0],[25,0],[26,0],[27,0]]
      # ...

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
key fails); pointers where omission must be *detectable* so it can be
refused — `start`, `blocks_*`, `lighting.level` are all REQUIRED, the pointer
is how "said nothing" stays distinct from "said zero"; `blocks_*` REQUIRED on
props and REFUSED elsewhere, refs never
resolved, `targeting` opaque, at most one `boss` per region, `key` must match
the requested key on `PutDungeon`.

New rules:

- **The regions are the floor.** `cells` is a list of rows, each a list of
  `[col,row]` — the nesting is for diff-readability only; the compiler flattens
  it. A region with no cells fails; a cell in two regions fails; there is no
  other floor. Flat — no nesting (Not now). The canvas is the only picture of
  the floor: the file says nothing visual (a hex grid's stagger cannot be drawn
  in text honestly, which is why there is no glyph map — ruled 2026-08-23).
- **The envelope is implied, never written.** A crossing from floor into void
  is a crossing nobody can make; `void` already says whether sight crosses it.
  That is the runtime's rule today (`seamWall`'s comment) — the file just stops
  pretending otherwise.
- **`walls` and `doors` are edges between adjacent floor cells.** Adjacency is
  checked by spatial under the declared orientation. An edge listed twice, or
  in both `walls` and a door, fails. A door edge need not sit on a region seam
  — a door inside a room is legal. A door's state (open / closed / locked) is
  encounter state, not atlas geometry: it compiles to `DoorInput.State`, lives
  in the started snapshot, and reaches a player through the existing door
  verbs. The atlas carries the doorway (two cells); the builder's inspector
  shows the lock from the YAML.
- **`lighting` is a block with one field today: `level`**, a dimmable switch
  in `[0,1]` (0 = no light, 1 = full). A block rather than a bare number so the
  next field — `ref`, naming the kind of light for the assets (flame glow,
  moonlight, …) — and later `audio` land beside it without reshaping the file.
  Carried through the composition unread like `targeting`; how the rulebook
  turns a level into obscurement is a rule and lives there. REQUIRED per
  region (no default, per #1033).
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
message FieldError { string path = 1; string message = 2; }   // "walls[3]", "place[7].blocks_los", "regions[1].cells[0][3]"
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
  Lighting lighting = 4;
}
message Lighting { double level = 1; }   // 0..1; `ref` joins here when the assets need a kind
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
| **toolkit `encounter`** | `FieldInput{Canvas, Rooms[rect+Origin], Connections, Doors}`; a room is a region; `RegionAt` is the mask | **Ruled:** `RegionInput{ID, Name, Cells, Lighting}` replaces `RoomInput`+`Origin`+`Connections` (a connection was a chain artefact; a doorway is already two cells); authored walls move up to `FieldInput.Walls`. Hex only; `Orientation` stays authored. `CanvasInput` keeps `Void`; lighting lands on the region, not the canvas (closes toolkit#1113 by relocation). `Sight` receives the region's lighting level. `EncounterData` carries regions + lighting |
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

**Combat testing keeps working (ruled):** version 1 is deleted in the same PR
that lands version 2, and every fixture that feeds combat tests moves with it
— `dungeonspec/tomb_test.go`, `rpg-api/internal/sessionworld` (the embed
becomes `content/reference-tomb.yaml`), the slice-2 tomb variant with the
second skeleton behind a wall (rpg-project#254), and the web's session
fixtures. The golden atlas is what proves nothing moved under them.

**Round trip:** builder `New` → paint three regions → walls → locked door →
props → `Save & Play` → the 3D game route opens on it → walk entrance to
hall → `Open` reloads the same YAML byte-for-byte. Byte identity holds
because the server stores the submitted bytes verbatim (`GetDungeon` returns
the file, not a re-marshal) and the builder's emitter is deterministic; the
builder is the writer, so comment preservation is not a requirement.

**Discriminators** (the symmetric-bug lesson): a pixel-formula test that draws
an L-shaped region under both orientations and checks one named cell's world
position in the builder and in `buildScene3D` — not a round-trip.

**Error paths:** a cell in two regions → `regions[1].cells[0][3]`; a wall between
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

Board: this is slice #256 under journey #169. rpg-api#806 is its sub-issue
(body to be rewritten to this design). On landing: toolkit#1139 closes (the
seam it fixes no longer exists), rpg-project#131 closes (picker delivered by
`ListDungeons`), toolkit#1113 closes (lighting landed on regions). Until then
they are open dependencies. PR #203 and the v0.4 Wave A/B issues are already
closed as history.

## 7. Rulings

1. **Floor format** — RULED 2026-08-23: `regions[].cells` lists, not a glyph
   map. A character grid lies about hex adjacency (rows/columns stagger), and
   the builder is the only hand that touches the floor — the YAML is the
   machine artifact, the canvas is the view. Emitter writes cells sorted, one
   row per line, for diff sanity.
2. **Lighting** — RULED 2026-08-23: a dimmable switch (`level` 0–1) per
   region now; a `ref` for the asset kind (flame glow, etc.) later.
3. **`RegionInput` replaces `RoomInput`** — RULED 2026-08-23: regions are the
   way; no rooms in the atlas projection. Hex only, orientation authored.
4. **Version 1 deleted** — RULED 2026-08-23, with the condition that the
   fixture combat testing runs on stays usable (§5).

## 7b. Open threads (Kirk, 2026-08-23 — deliberately unresolved)

This design stays open through implementation; adjustments made while
building are logged in `implementation.md`, and these threads get ruled when
they are ready, not before.

- **Staggered glyph map.** `E-E-E-T-T-T` over `-E-E-E-T-T-T` is the honest
  hex idiom: every character touches two above, two below, two beside, which
  is pointy-top adjacency. The open question is flat-top, whose stagger is
  per *column* by half a line — text cannot draw it. Shapes on the table:
  glyphs for pointy-top only (cell lists for flat — two ways to say one
  thing), glyphs everywhere (the lie returns for flat), or cell lists for
  both (ruling 1 today). Reversible: either form compiles to the same
  `FieldInput`; the choice lives in the decoder and the emitter.
- **Region archetype as a presentation ref.** v1's `archetype` was deleted
  because it silently decided a world fact (`entrance` chose where the party
  stands — the #1033 trap). An archetype that carries *presentation* is a
  different thing: `archetype: crypt` on a region, a ref the assets resolve
  into lighting kind + audio profile, with `lighting.level` as the dimmer on
  top — the `ref` slot generalised. It keeps the law the same way `targeting`
  and prop refs do: the composition carries the string unread. The rule to
  write down before it lands: **an archetype may never decide mechanics
  (start, blocking, sight, lighting level); only what the assets show and
  play.**

## 8. Not now

Facing / height / offsets / mounts (need their own wire fields — parked under
#131 already); region nesting; audio (the slot is designed, the field is not
cut); square grids (the composition accepts them; the dialect would need
`grid:`); `DeleteDungeon`; prop catalog authoring; free-roam walkthrough; DM
mode; hosted deployment.
