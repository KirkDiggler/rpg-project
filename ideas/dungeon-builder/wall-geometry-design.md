# Wall geometry — design

**Status:** PROPOSED 2026-09-03, third revision the same day, for Kirk's one
ruling. Record and the why: `wall-geometry.md`. Plan follows ruling.
**Scope:** one model, two slices. Slice 1 = scenery floor. Slice 2 = walls as
lines. Both ride journey rpg-project#169.

Nothing is left open for Kirk in this revision. What he ruled in
conversation and where it landed:

| ruling | where |
|---|---|
| wall ends come from a small named set of positions, picked, never typed freehand | §2.6, §3.3 |
| the file keeps numbers; the designer hides them | §2, §3.2 |
| standability is a number, tuned later | §4.3 |
| a door belongs to a hex with exactly one wall through it | §2.8, §3.4, §4.4 |
| legacy dungeons are deleted and recreated; the defaults are rewritten | §4.5, §7 |
| room templates come after this is clean | §8 |

## 0. Vocabulary

| thing | word | in the file |
|---|---|---|
| one straight wall with a start and an end | **wall** | a `walls[]` entry |
| a named point on a hex's boundary | **position** | `{cell, offset}` |
| the hexes a wall passes through | **cells**, its footprint | never written; derived |
| the hex-to-hex step a wall blocks | **crossing** | never written; derived |
| where two walls meet | **corner** | two ends with the same position |
| the hex a door sits in | **door hex** | `doors[].cell` |
| floor nobody stands on | **scenery** | `scenery` |

## 1. The model

1. A cell carries two facts: an **owner** (a region, or none) and whether it
   is **standable**. Floor is any cell with an owner or a scenery mark. Void
   is neither.
2. **Owner decides visibility and meaning** — concealment, lighting,
   archetype. A cell with no owner is visible to everyone.
3. **Standable decides feet.** Members and the start stand only on standable
   cells. Props may stand on any floor.
4. **Scenery** is floor that is not standable. Three producers, one state:
   the author's brush (owner none), the cells a wall cuts (owner as painted),
   and the footing of a presented wall (projection only, §4.6).
5. **A wall is a straight line between two positions, and the file holds
   nothing else.** The crossings it blocks and the cells it cuts are derived.
6. **A position is one of twelve points on a hex's boundary** — its six
   corners and six edge midpoints — written as `{cell, offset}` in
   bounding-box fractions. The set is closed: an offset outside it is refused
   by name. Kirk: *"there are prob less than a dozen total offset
   combinations we would want."*
7. **The wall invariant.** A wall looks the same from the visible side
   whatever is beyond it: floor, void, scenery, or hidden space.
8. **Scenery is transparent and impassable. It seals nothing.** Sight and the
   concealment walk pass through it; walls are the only seal.
9. **One offset unit.** Bounding-box fractions, x east and y south, for wall
   positions and for prop offsets alike. Props move from circumradius units
   to this in the same wave; the content that would have needed converting is
   being recreated.
10. **A door is a hex.** Exactly one wall passes through it; the door opens
    every crossing of that hex the wall blocks.

## 2. The panel — what a streamer sees and does

### Slice 1

- **2.1 Scenery brush.** A brush beside the room brush that paints floor
  belonging to no room. On the 2D board scenery is floor with a distinct
  hatch; in the 3D preview and in the game it is plain floor.
- **2.2 One state per cell.** Painting scenery over a room cell moves it out
  of the room; painting a room over scenery moves it in. Erase makes void and
  cascades as today (walls, doors, placements on the cell go with it).
- **2.3 Walls and doors** may stand on any floor, room or scenery.
- **2.4 Placement.** A prop drops on scenery. A monster or the start does
  not: the drop is refused in place with the reason ("nobody can stand
  here").
- **2.5 Errors point at the thing.** A compiler refusal that names a cell
  highlights that cell.

### Slice 2

- **2.6 A wall is two picked positions.** Click the hex the wall starts in
  and pick one of its twelve positions; click the hex it ends in and pick
  again. No freehand, no angle snap. Kirk: *"pick the cell the wall ends in
  and choose the offset from the dropdown."* The cells the wall makes
  unstandable hatch as soon as the second position is picked.
- **2.7 Corners.** Picking a position another wall already ends at joins
  them: the same `{cell, offset}` is written to both, and moving one end
  moves the other. That is the whole of snapping.
- **2.8 Doors.** Click a hex the wall passes through. If the wall cuts it
  too deep to stand in, the click is refused with that reason; otherwise the
  door is there, and every crossing of that hex across the wall opens with
  it.
- **2.9 The trap the picker cannot prevent.** Two corners on the same column
  edge draw a wall through the centre of every staggered cell between them,
  and those cells go scenery. The hatch shows it at once. **Wall this
  boundary** — the gesture for the common case — picks the quarter-line
  positions (slanted-edge midpoints) for a vertical boundary and the vertex
  line (corners) for a horizontal one, so the common case never trips it.

## 3. The file — dungeonspec version 2

### 3.1 Slice 1: `scenery`

| field | type | rule |
|---|---|---|
| `scenery` | rows of `[col,row]`, same encoding as `regions[].cells` | optional; omitted = none. Emitted after `regions`, before `start`. |

- **F1.** A cell MUST NOT appear in both `scenery` and any region's `cells`,
  nor twice in `scenery`. Refused naming the cell and the region.
- **F2.** `place[]` entries of prop refs MAY sit on scenery. Monster refs and
  `start` MUST NOT. Refused naming the placement and the cell.

### 3.2 Slice 2: a wall

Kirk: *"edges just need the starting and ending hex coordinate… not only be
easy to read it would be easy to edit."*

```yaml
walls:
  - start: { cell: [1, 2],  offset: [-0.25, -0.375] }
    end:   { cell: [1, 10], offset: [-0.25,  0.375] }
    height: 1
  - start: { cell: [1, 10], offset: [-0.25, 0.375] }
    end:   { cell: [9, 10], offset: [ 0.5,  0.25] }
```

| field | type | rule |
|---|---|---|
| `start`, `end` | position | required |
| `cell` | offset `[col,row]` | the hex the point is named from |
| `offset` | `[x,y]`, bounding-box fractions | MUST be one of the twelve positions (§3.3) |
| `height` | as today | |
| `name` | as today | |

- **F3.** Nothing derived is written. Footprint and crossings are the
  compiler's (§4.2).
- **F4.** The pair form (`edges: [[a,b], …]`) is **deleted**. Files that use
  it are refused at the header with a message naming the form. The defaults
  are rewritten by hand in this form (§7).
- **F5.** A corner is two walls carrying the same position at an end. Kirk:
  *"so long as the cells that overlap have the same offsets it is a
  corner."* The designer writes a join by copying the position; the compiler
  has no corner concept and needs none.
- **F6.** A wall's `offset` is mechanical (§4). A prop's `offset` stays
  visual only. Same shape, same unit, different law.
- **F7.** The list stays `walls`. `edges` is retired with the pair form.

### 3.3 The twelve positions

Bounding-box fractions: x in widths, east positive; y in heights, south
positive (the axis the prop offset's y already maps to in world z). Every
value is a dyadic rational, so the set compares exactly as floats, and a
60° rotation maps it onto itself.

Pointy-top:

| kind | positions |
|---|---|
| corners | `[0,-0.5]` `[0.5,-0.25]` `[0.5,0.25]` `[0,0.5]` `[-0.5,0.25]` `[-0.5,-0.25]` |
| edge midpoints | `[0.5,0]` `[-0.5,0]` `[0.25,-0.375]` `[-0.25,-0.375]` `[0.25,0.375]` `[-0.25,0.375]` |

Flat-top: the same set with x and y exchanged.

- **F8.** An `offset` not in the set for the file's orientation is refused
  naming the wall and the value. `[0,0]` is not in the set: a wall does not
  end in the middle of a hex.
- **F9.** Nothing stops the set growing; nothing in this design needs it to.

### 3.4 Slice 2: a door

```yaml
doors:
  - id: crypt-door
    cell: [1, 6]
    closed: true
```

| field | type | rule |
|---|---|---|
| `cell` | offset `[col,row]` | the door hex; replaces `edges` |
| `id`, `closed`, `locked`, `concealed` | as today | |

- **F10.** Exactly one wall MUST pass through the door hex. None, or two,
  is refused naming the hex.
- **F11.** The door hex MUST be standable after the wall's cut (§4.3):
  nobody can stand in a door that halves its hex. Refused naming the hex and
  the wall.
- **F12.** `doors[].edges` is retired with the pair form.

## 4. The compiler — `rulebooks/dnd5e/encounter/dungeonspec`

### 4.1 Floor (slice 1)

- **C1.** `owner` is unchanged. A `scenery` set is added. The flat cell list
  is `owner ∪ scenery`.
- **C2.** A wall MUST pass through at least one floor cell (owned or
  scenery); void cells on its path are not part of its footprint — nothing to
  cut, and the crossings into them are impassable already. This is how a wall
  stands against void (§1.7).
- **C3.** Props MUST be on floor; monsters and the start MUST be on standable
  cells.
- **C4. The concealment walk crosses scenery.** A *way* between regions A and
  B is a wall-free path from a cell of A to a cell of B whose interior cells
  are all scenery; it is a concealed way iff its first crossing is a concealed
  door. The frontier rule is otherwise as written: a way between visible and
  hidden space that is not a concealed door is refused, naming the scenery
  cell the path enters and the room. Visible reach from the start extends
  through scenery.
- **C5.** `hiddenFrom` is unchanged: hidden space is region cell sets. Scenery
  with no owner is in every member's atlas.
- **C6. The masquerade never stands a wall on scenery's far side.** Scenery is
  not visible *space*; a bare crossing from scenery into hidden space gets no
  synthesized wall. (C4 guarantees no such crossing is reachable from visible
  space without a wall.)

### 4.2 What a wall derives (slice 2)

- **C7. Crossings.** A crossing between adjacent cells P and Q is blocked iff
  the closed segment from the centre of P to the centre of Q intersects the
  closed wall segment.
- **C8. Footprint.** The wall's cells are every floor cell whose hex the
  closed segment intersects, in order along the wall.
- **C9. Embedding.** The compiler embeds cells in the plane for this alone:
  centre, corners, and bounding box of axial (q, r) under the file's
  orientation, circumradius 1. This is the toolkit's first world-unit
  geometry; `tools/spatial/room.go` names the corner-rule sight test as its
  second customer.

### 4.3 Standability (slice 2)

- **C10.** A footprint cell keeps its owner and is standable iff the area of
  its hex clipped by the near half-plane of every wall through it is at
  least `F` of the whole. `F` is one constant in the compiler, first value
  **0.75**, tuned in the game.

  Why 0.75 is safe with the picked set: the two straight walls the picker
  produces cut predictably. A vertical wall on the quarter line costs every
  adjacent cell 5/24 (20.8 %); a horizontal wall on the vertex line cuts the
  inside row nothing and the outside row 1/6. An inside corner formed by the
  two keeps 79 %. The one picked wall that cuts deeper — corners on the
  same column edge — halves the staggered cells, which is the trap of §2.9,
  shown by the hatch, never refused.
- **C11.** The designer displays the compiler's answer. A local mirror is
  deferred until the picker feels dead without one; if built, it is pinned to
  the compiler by a golden fixture.
- **C12.** A monster or the start on a cut cell is refused naming the wall
  that cuts it. A prop on a cut cell is allowed.
- **C13.** "Walkable anyway" on a cut cell is a named empty shelf. The
  scenery brush covers the direction the author needs today.

### 4.4 Doors (slice 2)

- **C14.** The door's crossings are every crossing of the door hex that its
  wall blocks (C7) — one to three. They open and close as one state, exactly
  as a multi-edge door does today, and reach the wire as one doorway per
  crossing under one door id.
- **C15.** The door's presented gap is the wall's chord through the door hex.
- **C16.** Lock and concealment semantics are unchanged: the door is the unit,
  its crossings are its shape.

### 4.5 Legacy — deleted

- **C17.** The pair form and its fitter are removed, not deprecated. Kirk:
  *"legacy dungeons will be deleted and started over again."* The client's
  chain-fitting (`CHAIN_TOLERANCE`, `boardWallScene`) retires in the same
  wave; a wall renders as the segment it is.

### 4.6 Projection (slice 2)

- **C18. Footing.** In `AtlasFor`, every footprint cell of a *presented* wall
  is in the recipient's atlas as scenery with no owner, even when its owner
  is hidden. Only presented walls foot: the footing of a withheld wall would
  trace the secret. This is rpg-dnd5e-web#898, built.
- **C19. Masquerade on a wall.** A concealed door in a wall is presented to a
  non-knower as the wall's whole segment without the chord gap. `maskHeight`
  reads the wall's height; it no longer reconstructs a chain.

## 5. The wire — `dnd5e.api.session.v1alpha1.GetAtlasResponse`

### 5.1 Slice 1: no change

`cells` is already the flat floor list and `regions[].cells` the membership.
A cell in `cells` and in no region is scenery. Movement is refused by the
engine, not the client.

### 5.2 Slice 2: additive

```proto
message AtlasSegment {
  AxialPoint from = 1;   // fractional axial: a point in the atlas's own frame
  AxialPoint to = 2;
  double height = 3;     // the wall's height multiplier, as boundaries carry it
}
message AxialPoint { double q = 1; double r = 2; }
repeated AtlasSegment segments = 10;   // on GetAtlasResponse
```

- The client's axial-to-world formula already accepts fractions; no unit and
  no second basis cross the wire.
- `boundaries` and `doorways` are unchanged and remain the mechanical truth.
  `segments` is presentation: what the client draws instead of fitting. A
  door's gap is derived client-side from its doorways and the segment.
- Prop offsets on the wire change unit (§1.9): the proto comment, not the
  field.
- rpg-api translates; the runtime never reads segments.

## 6. The runtime — what it answers today and what it needs

| question | today | needed |
|---|---|---|
| can a member step onto an ownerless cell | refused, "is not floor" | unchanged |
| can a wall stand on an ownerless cell | refused, "a crossing nobody can make" | accept scenery (C2) |
| does sight pass over an ownerless cell | treated as void, so per `void` setting | scenery is transparent regardless (§1.8) |
| can a prop sit on an ownerless cell | refused | accept scenery (C3) |
| which crossings does a wall block | the pairs as written | derived (C7), still delivered as pairs |
| which crossings does a door open | its listed pairs | its hex's blocked crossings (C14), still delivered as doorways |
| who sees a cut cell | n/a | footing (C18) |

Mechanics on the wire do not move. rpg-api is a pin in slice 1 and a pin plus
a translation in slice 2.

## 7. Acceptance

Slice 1:

- **A1.** Every current content file compiles to a byte-identical atlas.
  (Slice 1 touches no wall.)
- **A2. Yardstick.** Kirk's concealed room re-authored with a scenery strip
  behind the visible room's wall: a non-knower's `AtlasFor` is byte-identical
  to the twin dungeon with the secret room deleted and the strip kept.
- **A3.** The forgotten wall: visible room, scenery, hidden room, no wall.
  Refused, naming the scenery cell and the room.
- **A4.** A monster on scenery is refused; a prop on scenery compiles and
  renders.
- **A5.** In play, a step onto scenery is refused and two members in visible
  rooms separated only by scenery see each other.

Slice 2:

- **A6. The tomb, rewritten.** `reference-tomb.yaml` re-authored in the wall
  form compiles to the same cells and the same blocked crossings as its
  pair-form original, and every door opens the same crossings. This is the
  forcing case and the regression net that replaces byte-identity. Kirk:
  *"we built the thing, verify it works, then rewrite that and commit it."*
- **A7.** The record's 6×6 room: four walls emit four segments, the board
  draws four lines, and the blocked crossings are the same 46 the room tool
  authored.
- **A8. Calibration.** A quarter-line vertical wall leaves every adjacent
  cell standable at `F = 0.75`; a column-edge vertical wall makes every
  staggered cell between its ends scenery.
- **A9.** An offset outside the twelve is refused naming the wall and the
  value; a pair-form file is refused at the header naming the form.
- **A10.** A concealed door in a wall: the non-knower's atlas carries the
  full segment and no doorway; the knower's carries the chord gap. Yardstick
  twin identical.
- **A11.** The hugging layout with a quarter-line wall: the non-knower's atlas
  shows floor on both sides of the wall (C18) and equals the twin authored
  with scenery where the footprint falls.
- **A12.** Two walls carrying the same position at an end close a corner on
  the board; a door hex with two walls through it is refused; a door hex the
  wall halves is refused.
- **A13.** Every prop in the rewritten defaults renders where it did, after
  the unit change, by screenshot.

## 8. Shelves — named, empty

- **Room templates.** Kirk: *"template shapes that are a region built out…
  map parts that we assembled"* (Gloomhaven). A template is a region, its
  walls, and its doors in a local frame; placing one translates the cells and
  rotates by a multiple of 60°, under which the twelve positions map onto
  themselves. After this design is clean.
- "Walkable anyway" on a cut cell (C13).
- Region-scoped authored scenery (`regions[].scenery`): not needed; cuts
  produce it derived, with the region's visibility.
- More positions in the set (F9).
- More than one prop per hex: its own thing (record, ruling 8).
- Cliff edges, the third thing a floor edge can be: rpg-toolkit#1443.
- Sight across transparent void between two regions with no wall between
  them: a pre-existing gap of the same shape as C4, to be filed on the
  toolkit, not folded in here.
- A designer-local mirror of the standability rule (C11).

— cross-team agent, on behalf of KirkDiggler
