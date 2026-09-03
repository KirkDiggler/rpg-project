# Wall geometry — design

**Status:** PROPOSED 2026-09-03, for Kirk's one ruling. Record and the why:
`wall-geometry.md`. Plan follows ruling.
**Scope:** one model, two slices. Slice 1 = scenery floor. Slice 2 = walls as
lines (offsets on runs). Both ride journey rpg-project#169.

## Decisions for Kirk

- **D1. The standability discriminator.** Two candidates, same shadow rule,
  same file, different compiler constant. See §4.3. Recommendation: the
  mini-distance rule.
- **D2. Legacy runs keep the fitter until straightened.** A run with no
  endpoints is an edge run and renders as today; the tolerance's scope narrows
  to legacy runs instead of dying on day one. See §4.5 and §2.9.
- **D3. Endpoint encoding.** `{at, corner, offset}`, offset bounded to ±1 cell
  unit (props keep ±0.5); the wire carries fractional axial points. See §3.2
  and §5.2.

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
   and the footing of a presented boundary (projection only, §4.6).
5. **A wall is a line.** The crossings it blocks are its shadow; the cells it
   cuts are scenery. A run without endpoints is an edge run: its edges are
   its shadow, it cuts nothing.
6. **The wall invariant.** A wall looks the same from the visible side
   whatever is beyond it: floor, void, scenery, or hidden space.
7. **Scenery is transparent and impassable. It seals nothing.** Sight and the
   concealment walk pass through it; walls are the only seal.

## 2. The panel — what a streamer sees and does

### Slice 1

- **2.1 Scenery brush.** A brush beside the room brush that paints floor
  belonging to no room. On the 2D board scenery is floor with a distinct
  hatch; in the 3D preview and in the game it is plain floor.
- **2.2 One state per cell.** Painting scenery over a room cell moves it out
  of the room; painting a room over scenery moves it in. Erase makes void and
  cascades as today (walls, doors, placements on the cell go with it).
- **2.3 Walls and doors** may be drawn on any edge whose two cells are both
  floor, room or scenery.
- **2.4 Placement.** A prop drops on scenery. A monster or the start does
  not: the drop is refused in place with the reason ("nobody can stand
  here").
- **2.5 Errors point at the thing.** A compiler refusal that names a scenery
  cell highlights that cell.

### Slice 2

- **2.6 Every new wall stroke has two endpoints**, dragged in board space and
  drawn as handles. Dragging a handle moves the line; the cells it makes
  unstandable hatch live. No angle snapping.
- **2.7 Joins.** A handle released within the magnet radius of another run's
  endpoint takes that endpoint's exact position, and the two move together
  thereafter. This is the only snap.
- **2.8 Doors on a line.** Click the line; the door takes the crossing under
  the click. Deleting the door restores the wall.
- **2.9 Straighten.** A legacy run (no endpoints) offers **Straighten**, per
  run and for the whole dungeon: the fitted endpoints are written into the
  file and the run becomes a line. Until then it renders as today.

## 3. The file — dungeonspec version 2, additive

### 3.1 Slice 1: `scenery`

| field | type | rule |
|---|---|---|
| `scenery` | rows of `[col,row]`, same encoding as `regions[].cells` | optional; omitted = none. Emitted after `regions`, before `start`. |

- **F1.** A cell MUST NOT appear in both `scenery` and any region's `cells`,
  nor twice in `scenery`. Refused naming the cell and the region.
- **F2.** `place[]` entries of prop refs MAY sit on scenery. Monster refs and
  `start` MUST NOT. Refused naming the placement and the cell.

### 3.2 Slice 2: endpoints on a run

The run form of `walls[]` (`{edges, height, name}`) gains `from` and `to`.

| field | type | rule |
|---|---|---|
| `from`, `to` | `{at: [col,row], corner: 0–5, offset: [x,y]}` | both present or both absent |
| `at` | offset cell | MUST be floor |
| `corner` | integer 0–5 | clockwise from 12 o'clock (pointy) or 3 o'clock (flat) |
| `offset` | `[x,y]`, cell units (circumradius = 1) | each in [-1, 1]; `[0,0]` and omitted mean the corner itself |

- **F3.** With endpoints, `edges` MUST equal the line's shadow (§4.2). A file
  whose edges disagree is refused naming the first differing edge in the
  author's coordinates. The builder always writes the shadow; a hand edit
  that breaks it is caught, never silently repaired.
- **F4.** Without endpoints, the run is an edge run. Its edges are its
  shadow, it cuts nothing, and it renders as today. Every dungeon on disk is
  such a file: **zero offset is today's file.**
- **F5.** `doors[].edges` is unchanged. Every door edge MUST lie in the
  shadow of some wall, as today.
- **F6.** `offset` on a wall endpoint is mechanical (§4). `offset` on a prop
  stays visual only, bounded to ±0.5. Same shape, different law: what the
  offset is attached to decides.

## 4. The compiler — `rulebooks/dnd5e/encounter/dungeonspec`

### 4.1 Floor (slice 1)

- **C1.** `owner` is unchanged. A `scenery` set is added. The flat cell list
  is `owner ∪ scenery`.
- **C2.** A wall or door edge MUST join two floor cells (owned or scenery).
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

### 4.2 The shadow (slice 2)

- **C7.** A crossing between adjacent cells P and Q is blocked by a line iff
  the closed segment from the centre of P to the centre of Q intersects the
  closed wall segment.
- **C8.** The compiler embeds cells in the plane for this alone: centre of
  axial (q, r) under the file's orientation, circumradius 1. This is the
  toolkit's first world-unit geometry; `tools/spatial/room.go` names the
  corner-rule sight test as its second customer.

### 4.3 Standability (slice 2) — **D1**

A cell touched by a line keeps its owner and may lose standability.

- **(a) Area rule** — standable iff the area of the hex clipped by the near
  half-plane of every wall is at least `F` of the whole. Needs polygon
  clipping (the web had it and deleted it; recoverable).
- **(b) Mini-distance rule** — standable iff the distance from the cell's
  centre to every wall segment is at least `R`. A point-to-segment distance
  and nothing else. Reads as *"a wall makes a hex unwalkable when it passes
  closer than a mini's radius to the hex's centre."*

Calibration, same for both orientations: a straightened legacy run sits on
the midline between two staggered columns, and **every cell touching it loses
5/24 of its area (20.8 %) and has its centre √3/4 = 0.433 from the line.**
So under (a) the 80 % Kirk intuited is 1 point above the natural tooth and
would make every other cell along every straight wall unwalkable; `F = 0.75`
leaves a 4-point margin against handle jitter. Under (b) `R = 0.30` leaves
0.13 of margin, and the number is a thing a streamer can see on the table.
A line through a cell's centre is unstandable under both.

- **C9.** The chosen constant lives once, in the compiler, and the builder
  displays the compiler's answer. A local mirror in the builder is deferred
  until the handle drag feels dead without one; if built, it is pinned to
  the compiler by a golden fixture.
- **C10.** A monster or the start on a cut cell is refused naming the wall
  that cuts it. A prop on a cut cell is allowed.
- **C11.** The per-cell author override ("this cell is walkable anyway") is
  a named empty shelf: no field until a streamer needs it.

### 4.4 Doors on a line (slice 2)

- **C12.** The wall's shadow minus its doors' edges is what is blocked, as
  today. A door's edges MUST lie in the shadow of the line it stands in.

### 4.5 Legacy runs — **D2**

- **C13.** A run without endpoints compiles exactly as today. It emits no
  segment (§5.2), so the client renders it through the existing fitter. The
  fitter's scope is *legacy runs*; it retires when the last content file has
  none.

### 4.6 Projection (slice 2)

- **C14. Footing.** In `AtlasFor`, every cell cut by a *presented* segment is
  in the recipient's atlas as scenery with no owner, even when its owner is
  hidden. Only presented segments foot: the footing of a withheld boundary
  would trace the secret. This is rpg-dnd5e-web#898, built.
- **C15. Masquerade on a line.** A concealed door standing in a line wall is
  presented to a non-knower as the wall's whole segment without the door gap.
  `maskHeight` reads the segment's height; it no longer reconstructs a run.

## 5. The wire — `dnd5e.api.session.v1alpha1.GetAtlasResponse`

### 5.1 Slice 1: no change

`cells` is already the flat floor list and `regions[].cells` the membership.
A cell in `cells` and in no region is scenery. Movement is refused by the
engine, not the client.

### 5.2 Slice 2: additive — **D3**

```proto
message AtlasSegment {
  AxialPoint from = 1;   // fractional axial: a point in the atlas's own frame
  AxialPoint to = 2;
  double height = 3;     // the run's height multiplier, as boundaries carry it
}
message AxialPoint { double q = 1; double r = 2; }
repeated AtlasSegment segments = 10;   // on GetAtlasResponse
```

- The client's axial-to-world formula already accepts fractions; no corner
  enum, no unit, and no second basis cross the wire.
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
| which crossings does a wall block | the edges as written | the shadow (C7), still delivered as edges |
| who sees a cut cell | n/a | footing (C14) |

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
  segments (legacy runs).
- **A7.** The record's 6×6 room: four strokes with endpoints emit four
  segments, the board draws four lines, and the shadow is the same 46 edges
  the room tool authored.
- **A8. Calibration.** A straightened vertical legacy run leaves every
  adjacent cell standable at the default constant; a line through a cell's
  centre makes it unstandable.
- **A9.** Hand-edited edges that disagree with the line are refused naming
  the first differing edge.
- **A10.** A concealed door in a line wall: the non-knower's atlas carries the
  full segment and no doorway; the knower's carries the gap. Yardstick twin
  identical.
- **A11.** The hugging layout with a midline wall: the non-knower's atlas
  shows floor slivers on both sides of the wall (C14) and equals the twin
  authored with scenery where the cuts fall.

## 8. Shelves — named, empty

- Per-cell standability override (C11).
- Region-scoped authored scenery (`regions[].scenery`): not needed; cuts
  produce it derived, with the region's visibility.
- More than one prop per hex: its own thing (rpg-project#360 record, ruling
  8).
- Cliff edges, the third thing a floor edge can be: rpg-toolkit#1443.
- Sight across transparent void between two regions with no wall between
  them: a pre-existing gap of the same shape as C4, to be filed on the
  toolkit, not folded in here.
- A builder-local mirror of the standability rule (C9).

— cross-team agent, on behalf of KirkDiggler
