# Wall geometry — design

**Status:** PROPOSED 2026-09-03 (revised same day to Kirk's file shape:
two points per wall, nothing derived written), for Kirk's one ruling. Record and the why: `wall-geometry.md`.
Plan follows ruling.
**Scope:** one model, two slices. Slice 1 = scenery floor. Slice 2 = walls as
lines. Both ride journey rpg-project#169.

## Decisions for Kirk

- **D2. Legacy walls keep the fitter until straightened.** A wall in the
  pair form is a legacy wall: it compiles as today and renders as today. The
  fitter's scope narrows to legacy walls instead of dying on day one.
  See §4.5 and §2.9.

Resolved in conversation, recorded here so they stay resolved:

- *Standability is a number, tuned later* (Kirk: *"it is a number though and
  I imagine it is below 98%"*). Area fraction, one constant in the compiler,
  first value 0.75 for the tooth reason in §4.3. The scenery brush is the
  author's control when the number is wrong.
- *A wall's ends are cell plus offset*, the same noun props use. No corner
  numbering. A corner is two ends on one point.

## 0. Vocabulary

| thing | word | in the file |
|---|---|---|
| one straight wall with a start and an end | **wall** | a `walls[]` entry |
| the hexes a wall passes through | **cells**, its footprint | never written; derived |
| the hex-to-hex step a wall blocks | **crossing** | never written in the line form; the pair form's `edges` are crossings |
| where two walls meet | **corner** | two ends whose points coincide |
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
5. **A wall is a straight line from its start to its end, and the file
   holds nothing else.** The crossings it blocks and the cells it cuts are
   derived, never written. A legacy wall (pair form) blocks exactly its
   listed crossings and cuts nothing.
6. **The wall invariant.** A wall looks the same from the visible side
   whatever is beyond it: floor, void, scenery, or hidden space.
7. **Scenery is transparent and impassable. It seals nothing.** Sight and the
   concealment walk pass through it; walls are the only seal.
8. **One position noun.** Cell plus offset positions a prop today and a
   wall's ends next. A prop's offset is visual only; a wall's offset is
   mechanical. What the offset is attached to decides.

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
- **2.5 Errors point at the thing.** A compiler refusal that names a scenery
  cell highlights that cell.

### Slice 2

- **2.6 A wall is drawn from a start to an end** in board space, each end a
  handle. Dragging a handle moves the line; the cells it makes unstandable
  hatch live. No angle snapping.
- **2.7 Corners.** A handle released within the magnet radius of another
  wall's end takes that end's exact point, and the two move together
  thereafter. This is the only snap.
- **2.8 Doors on a wall.** Click the wall; the door takes the crossing under
  the click. Deleting the door restores the wall.
- **2.9 Straighten.** A legacy wall offers **Straighten**, per wall and for
  the whole dungeon: the fitted start and end are written into the file and
  the wall becomes a line. Until then it renders as today.

## 3. The file — dungeonspec version 2, additive

### 3.1 Slice 1: `scenery`

| field | type | rule |
|---|---|---|
| `scenery` | rows of `[col,row]`, same encoding as `regions[].cells` | optional; omitted = none. Emitted after `regions`, before `start`. |

- **F1.** A cell MUST NOT appear in both `scenery` and any region's `cells`,
  nor twice in `scenery`. Refused naming the cell and the region.
- **F2.** `place[]` entries of prop refs MAY sit on scenery. Monster refs and
  `start` MUST NOT. Refused naming the placement and the cell.

### 3.2 Slice 2: the line form of a wall

Kirk: *"edges just need the starting and ending hex coordinate. current shape
takes all the cells but that can be derived from it… not only be easy to
read it would be easy to edit."*

```yaml
walls:
  - start: { cell: [1, 2],  offset: [0.5, 0.5] }
    end:   { cell: [1, 10], offset: [0.5, 0.5] }
    height: 1
  - start: { cell: [1, 10], offset: [0.5, 0.5] }
    end:   { cell: [9, 10], offset: [0.5, 0.5] }
```

| field | type | rule |
|---|---|---|
| `start`, `end` | `{cell: [col,row], offset: [x,y]}` | both present = line form |
| `cell` | offset cell | the hex the point is named from |
| `offset` | `[x,y]`, the unit and bound of `place[].offset` | omitted = the cell's centre |
| `height` | as today | |
| `name` | as today | |

- **F3.** Nothing derived is written. The footprint and the crossings are the
  compiler's (§4.2); a file never carries them in this form, so there is
  nothing to fall out of step and nothing to refuse for disagreeing.
- **F4.** A wall with `edges` and no `start`/`end` is a legacy wall: its
  listed crossings are blocked, it cuts nothing, it renders as today. Every
  dungeon on disk is such a file: **today's file is today's file.**
- **F5.** `doors[].edges` is unchanged. Every door edge MUST lie among the
  crossings some wall blocks, as today.
- **F6.** A wall's `offset` is mechanical (§4). A prop's `offset` stays
  visual only. Same shape, different law.
- **F7. A corner is a shared end.** Two walls meet when they carry the same
  `{cell, offset}` at an end — Kirk: *"so long as the cells that overlap have
  the same offsets it is a corner."* The builder writes a join by copying the
  tuple. The compiler has no corner concept: each wall casts its own shadow,
  and equal points make a closed corner without anyone declaring one.
- **F8.** The list stays `walls`, because `edges` already means crossings in
  the pair form and one word means one thing across both forms.

## 4. The compiler — `rulebooks/dnd5e/encounter/dungeonspec`

### 4.1 Floor (slice 1)

- **C1.** `owner` is unchanged. A `scenery` set is added. The flat cell list
  is `owner ∪ scenery`.
- **C2.** A wall or door MUST stand on floor (owned or scenery). Pair form:
  both cells of every crossing. Line form: the wall MUST pass through at
  least one floor cell; void cells on its path are not part of its footprint
  — nothing to cut, and the crossings into them are impassable already. This
  is how a wall stands against void (§1.6).
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

### 4.2 What a line derives (slice 2)

- **C7. Crossings.** A crossing between adjacent cells P and Q is blocked iff
  the closed segment from the centre of P to the centre of Q intersects the
  closed wall line.
- **C8. Footprint.** The line's cells are every floor cell whose hex the
  closed line intersects, in order along the line. Derived, never written;
  the builder shows them.
- **C9. Embedding.** The compiler embeds cells in the plane for this alone:
  centre and corners of axial (q, r) under the file's orientation,
  circumradius 1, and `offset` in the unit `place[].offset` already uses.
  This is the toolkit's first world-unit geometry; `tools/spatial/room.go`
  names the corner-rule sight test as its second customer.

### 4.3 Standability (slice 2)

- **C10.** A footprint cell keeps its owner and is standable iff the area of
  its hex clipped by the near half-plane of every wall through it is at
  least `F` of the whole. `F` is one constant in the compiler, first value
  **0.75**.

  Why 0.75, both orientations: a straightened legacy wall sits on the midline
  between two staggered columns, and **every cell touching it loses 5/24 of
  its area (20.8 %)**. An 80 % rule would make every other cell along every
  straight wall unwalkable; 0.75 leaves a 4-point margin against handle
  jitter. A line through a cell's centre halves it and it is unstandable.
  The number is tuned in the game, not argued here.
- **C11.** The builder displays the compiler's answer. A local mirror is
  deferred until the handle drag feels dead without one; if built, it is
  pinned to the compiler by a golden fixture.
- **C12.** A monster or the start on a cut cell is refused naming the wall
  that cuts it. A prop on a cut cell is allowed.
- **C13.** The other direction — "this cut cell is walkable anyway" — is a
  named empty shelf. The brush covers the direction the author needs today.

### 4.4 Doors on a wall (slice 2)

- **C14.** What is blocked is the wall's crossings minus its doors' edges, as
  today. A door's edges MUST be among the crossings of the wall it stands
  in.

### 4.5 Legacy walls — **D2**

- **C15.** A wall in the pair form compiles exactly as today. It emits no
  segment (§5.2), so the client renders it through the existing fitter. The
  fitter's scope is *legacy walls*; it retires when the last content file
  has none.

### 4.6 Projection (slice 2)

- **C16. Footing.** In `AtlasFor`, every footprint cell of a *presented* wall
  is in the recipient's atlas as scenery with no owner, even when its owner
  is hidden. Only presented walls foot: the footing of a withheld wall would
  trace the secret. This is rpg-dnd5e-web#898, built.
- **C17. Masquerade on a wall.** A concealed door standing in a line wall is
  presented to a non-knower as the wall's whole segment without the door
  gap. `maskHeight` reads the wall's height; it no longer reconstructs a
  chain.

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
- `boundaries` is unchanged and remains the mechanical truth. `segments` is
  presentation: it is what the client draws instead of fitting.
- rpg-api translates; the runtime never reads segments.

## 6. The runtime — what it answers today and what it needs

| question | today | needed |
|---|---|---|
| can a member step onto an ownerless cell | refused, "is not floor" | unchanged |
| can a wall stand on an ownerless cell | refused, "a crossing nobody can make" | accept scenery (C2) |
| does sight pass over an ownerless cell | treated as void, so per `void` setting | scenery is transparent regardless (§1.7) |
| can a prop sit on an ownerless cell | refused | accept scenery (C3) |
| which crossings does a wall block | the pairs as written | derived from the line (C7), still delivered as pairs |
| who sees a cut cell | n/a | footing (C16) |

Mechanics on the wire do not move. rpg-api is a pin in slice 1 and a pin plus
a translation in slice 2.

## 7. Acceptance

Slice 1:

- **A1.** Every file in `rpg-deployment/content` compiles to a byte-identical
  atlas. Nothing authored, nothing changed.
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

- **A6.** Every content file compiles to a byte-identical atlas and emits no
  segments (legacy walls).
- **A7.** The record's 6×6 room: four walls in the line form emit four
  segments, the board draws four lines, and the blocked crossings are the
  same 46 the room tool authored.
- **A8. Calibration.** A straightened vertical legacy wall leaves every
  adjacent cell standable at `F = 0.75`; a line through a cell's centre makes
  it unstandable.
- **A9.** A wall whose path touches no floor cell is refused naming the
  wall; a wall that grazes void cells on its way along a room's edge
  compiles, cuts only the floor cells, and renders.
- **A10.** A concealed door in a line wall: the non-knower's atlas carries the
  full segment and no doorway; the knower's carries the gap. Yardstick twin
  identical.
- **A11.** The hugging layout with a midline wall: the non-knower's atlas
  shows floor on both sides of the wall (C16) and equals the twin authored
  with scenery where the footprint falls.
- **A12.** Two walls carrying the same `{cell, offset}` at an end compile to
  one point and the board draws a closed corner; the same point named from
  the neighbouring hex compiles identically.

## 8. Shelves — named, empty

- "Walkable anyway" on a cut cell (C13).
- Region-scoped authored scenery (`regions[].scenery`): not needed; cuts
  produce it derived, with the region's visibility.
- More than one prop per hex: its own thing (record, ruling 8).
- Cliff edges, the third thing a floor edge can be: rpg-toolkit#1443.
- Sight across transparent void between two regions with no wall between
  them: a pre-existing gap of the same shape as C4, to be filed on the
  toolkit, not folded in here.
- A builder-local mirror of the standability rule (C11).

— cross-team agent, on behalf of KirkDiggler
