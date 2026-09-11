---
status: DESIGN, proposed 2026-09-11
journey: rpg-project#428 · first slice: rpg-toolkit#1652 · builds on: ../../spells/ward-and-area/design.md §3 (the area source) and answers its §8 cube-on-hex item
law: spatial owns geometry and search over geometry, and owns no rule about what a covered cell means; encounter owns the meaning and holds no geometry of its own
---

# The spatial foundation — design

## 0. Purpose

Four tools, each judged by what it makes possible next, not by the feature it
closes:

1. **A footprint on the definition.** A prop, a creature, or an area effect
   declares a shape in world space. Placement adds position and rotation.
2. **Coverage.** One geometric question, asked of every shape in the game: how
   much of this cell does that footprint cover, and which edges does it cross.
3. **A distance field.** Flood from sources under a passability predicate,
   keeping distance and predecessor per cell. Reach, a path, a spreading
   blast, and a flight are all reads of one field.
4. **Cell facts, derived.** Encounter folds the properties of everything
   covering a cell into blocked / pass-through / standable, a cost, and what an
   entrant trips. Every reader of "can I step here" reads that one fold.

The use cases that pay for them are all in the tree or on the board:

| use case | pays for |
|---|---|
| the skeleton behind the pillar — rpg-toolkit#1652 | cell facts fold + distance field |
| Synty pieces that do not fit in a hex — the world builder's queue | footprint on the definition + coverage |
| Thunderwave at half coverage — ward-and-area §8, "cube-on-hex" | a spell shape through the same coverage call |
| Fireball burning the ground | an area effect as a placed entity with a clock-owned lifetime |
| flee | the distance field from threats, read against reach |

## 1. What is true today

Verified against `rpg-toolkit` `origin/main` on 2026-09-11 (`tools/spatial/v0.11.0`).

**Encounter is built on spatial, not beside it.** Spatial's `Room` is the only
position store (`encounter/canvas.go:183-218` embeds it and forwards `GetGrid`,
`CanPlaceEntity`, `GetLineOfSight`, `IsLineOfSightBlocked`; `cellOf` at
`encounter.go:1059` reads `GetEntityPosition`; `memberRecord` carries no cell).
Distance is `canvas.GetGrid().Distance` (`encounter.go:1054`); sight, boundary
rays, doors on a step, and step execution all delegate.

**One search is hand-rolled, and it is missing one input.**
`(*Encounter).bfsShortestPath` (`encounter/clocks.go:1240`) is a breadth-first
search over `grid.GetNeighbors`, gated by `RegionAt` (is it floor) and
`canvas.IsBoundaryMovementBlocked` (walls and doors). It has no notion of a
blocked *cell*. `field.isStandable` (`compilefield.go:741`) is walls-only too
(`sealedCells`, `:601`). Props carry `BlocksMovement` (`field.go:358-361`,
`atlas.go:206`) and nothing that searches reads it. Placement does
(`BasicRoom.CanPlaceEntity`, noted at `canvas.go:135`), so the monster gets a
route it cannot walk and stands still. That is #1652. The search was introduced
on 2026-08-23 (PRs #1189, #1191) with the argument that uniform edge cost makes
BFS exact; the argument is right and is not the defect.

**Spatial's pathfinder serves nobody.** `SimplePathFinder.FindPath(start, goal
CubeCoordinate, blocked map[CubeCoordinate]bool)` (`tools/spatial/pathfinder.go:61`)
is hex-cube A\* typed on cube coordinates, not `Position`, so it fits none of
the three grid families the encounter runs. Its one caller is
`tools/environments.BasicEnvironment.FindPathCube` (`environment.go:245`), and
`tools/environments` is imported only by `tools/spawn` and the legacy
`rulebooks/dnd5e/dungeon` package; nothing live, and not rpg-api, reaches it.
(An earlier draft of this document said "zero callers". That was wrong, and
the correction is left visible.) Square and gridless have no pathfinder
(#614, open since 2026-05). Its own `doc.go`
lists "pathfinding algorithms" under non-goals. ADR-0008 anticipated a
`tools/movement/` that never came.

**Passability has three authors, and they disagree.**

| reader | where | walls | props | creatures |
|---|---|---|---|---|
| player step | encounter `Step` (`step.go:201` `isStandable`, then the canvas's `CanPlaceEntity`) | yes | yes | only if the member is flagged `BlocksMovement`, default false |
| monster route | encounter `bfsShortestPath` | yes | no | no |
| client preview | web `components/session/atlasPath.ts:169` `edgePassable` | yes | yes | yes |

Session is not an author. `validateWalk` (`session/move.go:611`) checks
adjacency and nothing else, and its comment says the rest was deliberately
moved into `Step` (rpg-toolkit#1059). That is right and is kept. `rpg-api`
holds no path logic either: the move handler forwards the client's whole path
to the session SDK (`handlers/dnd5e/session/v1alpha1/move.go:18-21`). So the
duplication is inside encounter (a step and a route disagree) plus the
client's preview. An earlier draft named session as an author; it was wrong.

**Everything occupies exactly one cell.** `Placeable.GetSize() int`
(`tools/spatial/interfaces.go:123`) is serialised (`data.go:188`) and never
read by `room.go`. A prop is authored as a cell plus two booleans:

```yaml
- { ref: "dnd5e:props:pillar", at: [8,2], blocks_movement: true, blocks_los: true }
```

(`rpg-api/content/reference-tomb.yaml:99`). The cell *is* the object.

**Shapes exist and are unreachable.** ward-and-area §3 established that every
`GetPositionsIn*` query has zero callers and none is on the `Grid`/`Room`
interface. The square cone is marked "simplified" (`square_grid.go:184`); the
hex cone measures its angle in a skewed basis (#1626).

**The plane embedding already exists, waiting for its second customer.**
`encounter/dungeonspec/geometry.go:25-34` embeds hexes in the plane for wall
derivation and says, in its header, that it moves to `tools/spatial` the day
a second caller needs where a corner is. Coverage is that caller.

## 2. Rulings (Kirk, 2026-09-11)

- **The footprint is authored on the prop definition**, as a rules shape. The
  placement line gives position and rotation only. The mesh may overhang the
  footprint; the footprint is the designer's tool, not the mesh's bounds.
- **Ground effects are placed entities** in the room, like props. Spatial holds
  no clock. The effect that placed the fire owns its duration on the existing
  clock and removes the entity when the boundary fires.
- **Half coverage** is the rule for a shape affecting a cell (the 2014 Dungeon
  Master's Guide template rule). It is an input encounter passes in. Spatial
  has no opinion about thresholds.

## 3. The four pieces

### 3.1 Footprint on the definition — content owns it

A footprint is a shape in world space, in feet, with the definition's origin
at its anchor: a box (`w × d`, optional offset) or a convex polygon. A
placement is a definition plus a position and a rotation. Rotation is free
(degrees), because coverage is computed in the plane and no rule cares
whether a sarcophagus sits at 37°.

```yaml
# on the definition (content), once
dnd5e:props:sarcophagus:
  footprint: { box: { w: 5, d: 10 } }
  blocks_movement: true
  blocks_los: false

# on the placement, per instance
- { ref: "dnd5e:props:sarcophagus", at: [22,3], facing: 90 }
```

A creature's footprint comes from its size (Medium = 5 ft square, Large = 10 ft
square, and so on). An area effect's footprint is its `CastArea` shape at the
cast's origin. Nothing has a footprint of "one cell"; a one-cell thing is a
5-foot footprint that happens to cover one cell past threshold.

The two booleans stay on the definition and move off the placement line.
The reference tomb's per-placement flags become a migration, not a contract.

### 3.2 Coverage — spatial owns it

```go
// Coverage rasterises a footprint at a transform onto a grid.
type CoverageInput struct {
    Footprint Footprint
    At        Position
    Facing    float64 // degrees
}
type CoverageOutput struct {
    Cells map[Position]float64 // fraction of the cell's area covered, (0,1]
    Edges []Edge               // cell boundaries the footprint's outline crosses
}
func (g Grid) Coverage(in CoverageInput) (CoverageOutput, error)
```

Two outputs, not one, and this is load-bearing. A thick thing (pillar, ogre,
blast) is a set of cells past a threshold. A **thin** thing (a fallen beam, a
half-height wall, a curtain) covers under half of every cell it lies across
and would block nothing under a cell rule. Walls already solve this by being
boundaries *between* cells (`IsBoundaryMovementBlocked`), so the rasteriser
reports the edges an outline crosses and a thin prop contributes edges.
Without the second output the beam is foreclosed and someone hand-authors its
edges.

Fractions come out; thresholds go in above. Encounter applies "half" for a
blast and for a prop, and could apply a different number for a different rule
without spatial changing.

Coverage needs the plane embedding of a cell (its polygon in feet), which is
why `dungeonspec/geometry.go` moves to `tools/spatial` on this slice: the
header names exactly this condition.

### 3.3 Distance field — spatial owns it

```go
type FieldInput struct {
    Sources  []Position
    Passable func(from, to Position) bool // encounter's fold, see 3.4
    Cost     func(from, to Position) int  // 1 today; difficult terrain later
    Limit    int                          // stop past this distance; 0 = unbounded
}
type FieldOutput struct {
    Dist map[Position]int
    Prev map[Position]Position
}
func (g Grid) Field(in FieldInput) (FieldOutput, error)
func (f FieldOutput) PathTo(goal Position) ([]Position, bool)
```

Dijkstra over `Grid.GetNeighbors`, which degenerates to the existing BFS while
every cost is 1. Typed on `Position`, so it serves square, hex, and gridless
alike and closes #614 as a side effect. It replaces `bfsShortestPath`.
`SimplePathFinder` stays where it is for the legacy `tools/environments` tree
and is deleted when that tree is retired; reaching into environments to
remove it would widen this slice for no live customer.

What one field serves:

- **reach**: `Field{Sources: [me], Limit: budget}` → every cell I can afford.
- **a path**: the same field, `PathTo(target)`.
- **a spreading blast**: `Field{Sources: [origin], Limit: radius}` under a
  walls-only predicate — Fireball "spreads around corners" is exactly a
  walking field, not a circle.
- **flee**: `Field{Sources: threats}` read against my reach field; the
  behavior picks the reachable cell with the greatest threat distance. No
  point-to-point search can answer this.

A\* is an optimisation the field can grow when a use case pays for it. It is
not the primitive.

### 3.4 Cell facts — encounter owns them, derived never stored

```go
type CellFact struct {
    Passage  Passage        // Blocked | PassThrough | Standable
    Cost     int            // movement cost to enter, in cells
    OnEnter  []core.Ref     // facts an entrant trips (burning ground, a rune)
    Contribs []ContribRef   // who said so — for the intel record and the debug feed
}
func (e *Encounter) CellAt(p Position, viewer MemberID) CellFact
```

The fold: collect every placed entity whose coverage of `p` passes the
threshold, and every boundary on the way in, and combine by policy:

| contributor | Passage | how it got there |
|---|---|---|
| wall / shut door | Blocked (edge) | spatial boundary, unchanged |
| prop, `blocks_movement` | Blocked | coverage past half |
| thin prop | Blocked (edge) | coverage's `Edges` |
| hostile creature | Blocked (the two-sizes exception waits for size to exist on a member; today `GetSize` is a hardcoded 1 at `encounter.go:2463`) | coverage past half |
| nonhostile creature | PassThrough | coverage past half |
| burning ground | Standable, `OnEnter` carries the effect | coverage past half |
| difficult terrain (later) | Standable, `Cost: 2` | coverage past half |

The creature rows are the 2014 rules as written (you may move through a
nonhostile creature's space but not stop there; a hostile creature's space is
blocked unless the sizes differ by two). That is why the monster route
ignoring occupancy was half right: allies *are* passable. No divergence from
the letter is taken yet.

The three-valued `Passage` is what the field's `Passable` reads for routing,
and what `Step` reads for the player. One fold, four readers: the step, the
route, flee, and the atlas the web previews from. The web's `edgePassable`
becomes a rendering of the fold, not a fourth author. Session keeps checking
adjacency only.

`OnEnter` is a fact on the log, not a resolution. Stepping onto burning ground
publishes "member entered cell carrying `dnd5e:effects:burning`"; the spell's
effect subscribes and rolls the damage nested, the way rules stay in
resolution today. Encounter never knows what burning does.

`viewer` is on the signature from birth because a wall that is not there for
one observer (an illusion) is the fact this project exists to keep possible.
The first implementation ignores it.

## 4. Shapes, and the one place a rule touches geometry

A spell's `CastArea` (`combat/actions/area.go`) names a shape and an
**occlusion mode**. The shape becomes a footprint at the origin; coverage
gives the candidate cells; the mode cuts them:

| mode | cut by | examples |
|---|---|---|
| `sight` | line of sight from the origin (spatial, exists) | cone, line, Thunderclap's radius |
| `spread` | the walking field from the origin under a walls-only predicate | Fireball |

Both are spatial calls made by encounter. The mode is the spell's to declare;
the cut is geometry.

This is ward-and-area §3 layer (i) grown to its second customer. That design
exposed a radius and deferred cones, lines, and the cube "until they have a
customer". Coverage is the exposure: a cube is a box footprint; a cone is a
polygon; #1626's skewed hex cone goes away because a cone is no longer
computed in the hex basis at all. Its §8 "cube-on-hex is unaddressed" is
answered by the half rule in §2.

## 5. What this does not foreclose

Read against full D&D, which is the test:

- **Large and larger creatures** are footprints. `GetSize` finally has a reader.
- **Difficult terrain** is a `Cost` on a cell fact. The field is already Dijkstra.
- **Cover** is a ray against footprints in the same plane embedding coverage
  uses. Not built here; the embedding is chosen so it can be.
- **Illusory walls** are entities whose `blocks_los` is per observer, which is
  the intel rule (`viewer` on `CellAt`).
- **Forced movement** (Thunderwave's push) reads the same cell facts to find
  where a shove stops. No owner yet, by design; the fold is where it will look.
- **Squares.** Nothing in §3 is hex-specific. The grid answers neighbours,
  distance, and cell polygons; everything above it is the same code.

## 6. Rejected alternatives

- **Hand the BFS a blocked set** (#1652's option 1). Closes the pillar bug and
  leaves monster routing and walk validation as two authors of passability,
  which is the class of bug that just bit. A band-aid.
- **Ground state as a separate layer beside the room.** Reimplements placement
  and coverage for one contributor. Three versions of one thing.
- **Footprint derived from mesh bounds.** Ships a bounding box that is wrong
  for anything with a base narrower than its top, and takes the tool out of
  the designer's hands.
- **Per-cell authored blocking flags** (the status quo, extended). Forecloses
  rotation, moving props, and grid changes; every multi-cell object becomes
  hand-listed cells.
- **A\* as the primitive.** Cannot answer reach or flee; the field can answer
  a path.
- **Baking half into spatial.** A rule in the geometry package. The next rule
  with a different threshold would fork the call.
- **Cone and line in the hex basis, fixed in place.** Fixes #1626 once for
  hex; coverage fixes it for every grid and never computes a cone twice.

## 7. Charter changes

- `tools/spatial/doc.go`: drop "pathfinding algorithms" from non-goals.
  Spatial owns geometry and search over geometry, and owns no rule about what
  a covered cell means. Movement costs and thresholds remain non-goals.
- `rulebooks/dnd5e/encounter/AGENTS.md`: "what a path crosses" stays
  encounter's question; the answer is one fold (`CellAt`) and no search of its
  own. Add the ownership test: a new geometric shape → spatial; a new meaning
  for a covered cell → the fold.
- `dungeonspec/geometry.go` header: its stated condition has arrived; it moves.

## 8. Delivery and sequence

Each rung leaves the game walkable and adds one tool.

1. **Field + fold, proven by #1652.** Two modules, merged bottom-up:
   `tools/spatial` gains `Grid.Field` on `Position` (and `doc.go` says so;
   `SimplePathFinder` is left for environments); `encounter` gains `CellAt`
   folding walls, props, and creatures by stance (one cell each, as today),
   and both `Step` and the route read it, so a step and a route can no longer
   disagree. Session is untouched. Regression test: a monster behind a pillar
   routes around it. The web preview is unchanged and now merely agrees.
2. **Footprint + coverage, proven by one prop that does not fit.** The
   embedding moves to spatial; `Coverage` with cells and edges; the two
   booleans move to the definition; the tomb's placements migrate; the world
   builder authors a footprint and a facing. Kirk walks a sarcophagus.
3. **Shapes through coverage, proven by Thunderwave's cube.** `CastArea`
   gains its occlusion mode; cone and line ride the same call; #1626 closes.
   Sits behind ward-and-area's sequence (Thunderclap first, on a radius).
4. **Placed area effects, proven by burning ground.** An effect places an
   entity with a footprint and an `OnEnter` ref; the clock removes it; the
   fold reports it; resolution subscribes.
5. **Flee, proven by a monster that runs.** Threat field against reach field
   in the behavior package, which computes no geometry of its own.

Rung 1 is two toolkit modules. Rung 2 touches content and the world builder.
Rung 3 touches protos only if an area preview needs a new shape on the wire
(ward-and-area §8 flagged this as unknown, not zero). Nothing here authorises
a merge, a force-push, or a shared-stack change.

## 9. Where the evidence is thin

- **Not run.** No production code, no test claimed passing. Every citation
  was read at `origin/main`; the causal chain in #1652 is inferred from
  `stepMember` refusing and `walkCells` breaking, not observed under debug.
- **The world builder was not surveyed.** How it places a prop today, and
  whether it can author a facing, is unknown. Rung 2 starts by reading it.
- **Coverage cost.** Fractions of a cell under a rotated polygon are a
  clipping problem. Fine at dungeon scale; not measured.
- **Placement's contract changes.** `CanPlaceEntity` goes from "is anything at
  this cell" to "does anything cover this cell past threshold". Every caller
  inherits that; the list of callers was not enumerated.
- **The client's edge rule** (`atlasPath.ts`) reconstructs passability from
  the atlas. Whether the atlas carries enough for it to render the fold, or
  needs the fold on the wire, is a rung-1 finding.

— cross-team agent, on behalf of KirkDiggler
