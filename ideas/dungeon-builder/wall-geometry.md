# Wall geometry — why square rooms fight a hex grid, and what to author instead

**Status:** rulings in (2026-09-03, five rounds). Design PROPOSED in
`wall-geometry-design.md`, sixth revision; Kirk rules once; then plan and build.
**Issues:** rpg-dnd5e-web#908 (walls as lines) · rpg-dnd5e-web#910 (trim) ·
rpg-dnd5e-web#898 (a wall must stand on floor) · rpg-toolkit#1443 (cliff
edges) · supersedes rpg-dnd5e-web#904 (snapping)
**Shipped alongside:** rpg-dnd5e-web#903 (region rect, editable YAML, the rail)

This record exists because a day of work on the dungeon builder converged on a
problem none of the individual issues names, and the thinking was scattered
across three issue bodies. Kirk: *"we are chasing the wrong problem."*

## The problem

An author wants a square room. The floor is hexes. **A rectangle of hex cells
does not have a straight boundary** — its sides are staircases, because
alternate rows stagger by half a cell.

Everything built on top of that inherits the staircase:

- The wall gesture snaps a drag onto a seam family within **6°** and otherwise
  does not straighten at all — an all-or-nothing cliff that reads as jank
  (rpg-dnd5e-web#904).
- The renderer fits straight segments back out of the staircase with a
  Douglas-Peucker tolerance, `CHAIN_TOLERANCE = HEX_SIZE * 1.5`.
- Kirk's dungeons carry deliberate corner spurs that exist *only* to pull
  corners square (rpg-project#355) — a workaround for the same thing.

## The measurement that settled it

A room tool was built that authors a rectangle's perimeter as wall runs. For a
6×6 room it authored **exactly right**:

```
room 1 north=12   room 1 east=11   room 1 south=12   room 1 west=11    — 46 edges
```

and the board **rendered it as 8 segments** at `99°, 99°, 178°, 90°, 30°, 2°,
94°, 79°`.

Two conclusions, both load-bearing:

1. **The authoring was never the problem.** Four clean sides went in.
2. **`boardWallScene` flattens the authored runs back to loose edges and
   re-derives its own chains.** The grouping shipped in rpg-project#355 is
   discarded exactly where it would have helped.

And a third that killed the obvious repair: *rendering one polyline per
authored run does not fix it either*, because a polyline through the midpoints
of one-hex slices **is a staircase**. Drawing it square still needs a fitter.

Kirk, on the primitive: *"we are not using the wall groupings, every edge has a
slice of 1 in it."* The group records **which slices belong together** and never
records **what shape they are**.

## The model

**A wall is a LINE. The hex edges it blocks are its shadow.**

- The author draws a line; the file records the line.
- The compiler derives which crossings it blocks.
- The renderer draws the line that was drawn — no tolerance, no chaining, no
  fitting.

### Ruled (Kirk)

> *"i think the walls can cut through a hex wherever it likes if we have say
> >80% hex visible then we can stand on it"*

A wall cuts hexes freely. A hex sliced by a wall stays **standable while more
than a threshold of it remains**, and stops being standable below that. The
threshold is the discriminator the derived-crossing rule needs. *Amended
2026-09-03 (ruling 3 below): the number is a tunable, not a rule.*

### What this dissolves rather than fixes

- **Snapping (#904).** Snapping exists so a freehand drag *derives* into a good
  staircase. A recorded line does not need the drag to be precise, only
  captured. Close as superseded.
- **"Square over hex."** The square was never made of hexes; it is a line, and
  the hexes are what it cuts.
- **Corner spurs.** A line has corners by construction.

## The stepback (2026-09-03)

Kirk: *"the shape of the dungeon is #1 and laying it on the hex grid is
secondary… this is a stepback moment. I think we went as far as we can with
the current builder."* The builder's users are streamers setting up dungeons
for their subscribers, and the builder is also where a living-world scenario's
config form gets filled in. The conversation ran as a back-and-forth by his
ruling — *"multi choice questions… narrow possibilities when they come too
early."*

The stepback narrowed itself in one round. Regions stay painted cells.
**The walls are the shape.** A wall run keeps its edges and gains an offset at
each end. Kirk: *"the offset idea and using edges for their strength would be
the biggest impact with the lowest hanging fruit."*

### Rulings

1. **Space of the line (closes open ruling 1): the corner lattice plus an
   offset.** Each end of a run is anchored to a hex corner and carries an
   `offset` in the shape props use today, `[x, y]` as fractions of a cell.
   **Zero offset is today's file exactly.** Every dungeon on disk is already
   valid under this model, and no migration exists.
2. **Nothing derived is written (closes open ruling 2).** *Amended later the
   same day:* the first form of this ruling kept the edges in the file as a
   checked shadow. Kirk's shape needs only the two ends — *"edges just need
   the starting and ending hex coordinate… that can be derived… easy to read
   [and] easy to edit"* — so the line form carries `start` and `end` and
   nothing else; the footprint and the crossings are the compiler's. The
   legacy pair form is untouched, so everything that reads `walls[i].edges`
   today keeps working on today's files.
3. **The threshold is a tunable, not a rule (closes open ruling 3), and it
   lives in the compiler.** Kirk: *"the 80% rule is probably not a real rule.
   the real percentage will likely be tuned."* The runtime never learns the
   number; it reads cells and crossings. So this is a projection rule in
   `dungeonspec`, stated once. The builder shows the compiler's answer — the
   3D preview already round-trips `PutDungeon validate_only` — and a local
   mirror waits until the gesture feels dead without one. A per-cell author
   override (*"control over whether a space can be traveled on"*) is a named
   empty shelf. Noodle left on that shelf: the tuned thing may turn out to be
   *"a mini fits here"* rather than a percentage, which a streamer can see.
4. **Joins are a gesture, not a file fact.** Two runs meeting at a corner
   share an anchor and matching offsets; the drag keeps them matched. The
   compiler does not care whether lines touch — each casts its own shadow and
   crossings are discrete, so a hairline gap blocks the same as a join.
   Snapping survives only here.
5. **One position noun.** Anchor plus offset is the position type props use
   today and wall endpoints use next. Same shape, opposite law: a prop's offset
   is visual only (Kirk's earlier ruling); a wall's offset casts a shadow. What
   the offset is attached to decides whether it is mechanical.
6. **Scenery floor — the third floor state.** Kirk: *"a no region option that
   looks like the floor in the game but cannot be traveled on."* The floor has
   two states today, void and owned by a region. Scenery is rendered as floor,
   belongs to no region, nobody stands on it, it is never a way in, and a wall
   may stand on it. Three producers, one noun, and the compiler owns the state:
   the author's brush; wall footing (rpg-dnd5e-web#898, derived as the cells
   under a presented boundary); and the cells an offset wall cuts below the
   threshold. The wire already expresses it — the atlas's flat `cells` and
   each region's own `cells` are equal by construction today, and a cell in
   the flat list and in no region is scenery.
7. **The wall invariant: a wall looks the same from the visible side whatever
   is beyond it.** Floor, void, scenery, or hidden space must be
   indistinguishable through it — the never-authored yardstick applied to the
   boundary rather than the space. Two moves make it true: **walls may stand
   against void** (the outer wall of a room becomes the normal case, so black
   beyond a wall means nothing), and **the cells a wall cuts are scenery for
   everyone**, so the slivers always show floor.
8. **Slice order.** Scenery floor first (smallest; unblocks the dungeon Kirk
   has now), then offsets on runs, then the scenario config builder.
   **Parked as its own thing:** more than one prop per hex — the compiler
   refuses two placements on a cell today, and the rule that replaces it is
   "a cell blocks if any prop on it blocks"; it rides the same anchor-plus-
   offset foundation and is not needed for a while. Cliff edges stay adjacent
   (rpg-toolkit#1443).

### Second round, same day — the picker

Kirk asked to be picked apart rather than agreed with (*"I am wrong probably
about 50% of the time. that number drops prob to about 20% when we pick apart
the idea"*), and the pick-apart of the two-point wall found four things,
each answered by a ruling:

- The prop offset is rendered in circumradius units and bounded to ±0.5, so
  it cannot name a hex corner (they sit at (0, ±1) and (±0.866, ±0.5)).
  **Ruled: one unit, bounding-box fractions, for walls and props alike**;
  the content that would have needed converting is being recreated.
- A vertical wall from corner to corner on the same column edge runs through
  the centre of every staggered cell between them and halves each. **Ruled:
  ends are picked from a small named set of positions, never typed freehand
  — "there are prob less than a dozen total offset combinations we would
  want."** Twelve: six corners, six edge midpoints. The trap survives the
  picker (it is the line, not the ends) and is shown by the hatch; the
  common-case gesture picks the quarter line.
- A corner needs one snap, the end magnet. **Ruled: yes, that is the
  snapping.**
- Doors spoke crossings while walls spoke points. **Ruled: a door is a hex
  with exactly one wall through it**; it opens every crossing of that hex the
  wall blocks (one to three), as one state, on today's wire.

And **legacy dungeons are deleted and recreated** — the pair form, the
fitter, and `CHAIN_TOLERANCE` go, and the tomb rewritten by hand becomes the
regression net (*"we built the thing, verify it works, then rewrite that and
commit it"*). **Room templates** (Gloomhaven map parts) are the shelf after
this is clean; the closed position set rotates onto itself under 60°, which
is what makes them cheap.

### Third round, same day — the axis

Kirk, on the three-crossing door hex: *"we do not need to go too wild to
start. I think walls need to fit on one of the hex axes if you wanna place a
door on it… a hex has 60 deg around and that is more than enough."* Worked
through, the constraint is a larger win than the door fix. With ends on
named points and directions constrained, only five wall lines exist per
family, and three of them are the two good ones and the trap:

| wall | through | cuts per adjacent cell |
|---|---|---|
| along a row, midpoint line | side midpoints | 1/24 both sides |
| along a row, vertex line | corners only | 0 inside, 1/6 outside; closes no corner |
| along a row, centre line | centres | 1/2 — the trap |
| across rows, quarter line | side midpoints | 5/24, alternating |
| across rows, column edge | corners and centres | 1/2 — the trap |

**Ruled from that:** the positions are the **six side midpoints** (corners
drop; Kirk's *"could prob get away with 5"*); a wall may pass through a
cell's centre and seals what it halves (refused in this round, reinstated
in the next as the thick wall); **a door is a midpoint on its wall and opens one crossing**; and
**30° steps, not 60°** — Kirk: *"i guess it would be 30 deg if we get the six
sides in the middle"* — because a rectangle needs one direction from each
family. Every legal wall is standable-safe on its own; only corners are
decided by the number, which starts at **0.7** because a square room's inside
corner keeps exactly 3/4. The centre as a seventh point is the first shelf
for the set: it buys a wall stub at the cost of its own cell.

Kirk, reading "no centre": *"you mean we cannot go down the center? no
right? the flat side would be edge, centerline. maybe in the design we can
visualize where we can go."* Right — and the wall along a column's flat sides
IS the centreline of the neighbouring rows. Then: *"ideally one of the lines
would be on the edge of the flat side not cutting off the hex at all."* That
line exists, and the third-round refusal of it was wrong: on the flat sides
the even-row hexes are whole, and the price is the odd-row cell the line
centres, sealed, one per two rows. So each family has a **thin** wall (shaves
neighbours, kills nothing) and a **thick** wall (neighbours whole, seals
what it centres), both legal, the cost shown at pick time. The picture is
`wall-geometry-lines.svg`, generated from the geometry, in the design at
§4.3; the designer draws the same lines from a picked start (§2.6).

Then, reading the picture: *"to change angles the wall would have to end
dead center right? then it would go out a clean axis on one of the 30 deg
angles… if you want to change angles I think it has to end in the center…
what is the resistance to the center. I do not understand that."* The
resistance was a mispricing: I had costed the centre as "a thin wall ending
here spends the cell" and argued it out on that. Checked, every line through
a centre is a thick line, so a thin wall cannot reach a centre at all and
the price never arises. **The centre is the seventh position** — Kirk's
*"1 for center"* from the second round — and the corner rule is one sentence
per kind: thick walls turn at centres (all twelve rays from a centre are
thick, and a thick wall seals it anyway); thin walls turn at midpoints
(three thin lines through every slanted midpoint, four through every
flat-side one), keeping the corner cell.

### The tell, and what it taught

Two maps of the same concealed room, both leaked. In one the secret sat inside
visible floor behind the wall: every bare crossing around it was a
visible-to-hidden adjacency, the masquerade stood a wall on each, and the
outline drew itself. In the other the secret hugged the wall, and the tell
moved to the slivers — black where floor should show, because a straight wall
over a staircase boundary leaves every other far-side cell poking through.

One failure. A wall may only stand between two floor cells today, so a wall
with black beyond it can mean only one thing: there is floor back there you
may not see. An honest twin dungeon cannot even author that wall. That same
rule is what forced the secret to hug the wall in the first place — the room's
shape was dictated by the wall's edge set, geometry the author never thinks
about. Ruling 7 is the invariant; ruling 6 and the offset model are what make
it true. rpg-dnd5e-web#898 is the same failure one layer down and dissolves
into ruling 6.

## Ground truth (2026-09-03; web `dev`, toolkit `origin/main` bf234c7)

- Compile output is `encounter.FieldInput`: cells, blocked crossings, doorway
  pairs, region membership, placements. Everything else is carried payload.
  Runs die in `wallsOf`; no runtime code reads YAML or runs. **Mechanics on
  the wire do not move**; rpg-api is a pin.
- **Presentation on the wire moves, additively:** the atlas gains the drawn
  segment (anchor, offset, height) so the client draws what was drawn and the
  masquerade draws a straight segment too. This is what lets
  `boardWallScene` and `CHAIN_TOLERANCE` retire.
- Concealment is indifferent to where an edge came from: `hiddenFrom`,
  `Search`, and the frontier walk work on cell sets and the compiled crossing
  map. `maskHeight` reverse-engineers a run today and gets simpler with a
  stored line. The masquerade must never stand a wall on scenery's far side —
  scenery is not visible space.
- The frontier walk already skips ownerless neighbours as void, and
  `stepMember` already refuses an ownerless cell as "not floor". Scenery floor
  is close to free on the runtime; the wall-on-floor rule must accept it.
- The toolkit has **zero world-unit geometry** — no hex size, corners,
  polygons, or area. `tools/spatial/room.go` says twice that the corner-rule
  sight test needs cell-polygon geometry it does not have. The threshold is
  the toolkit's first geometry and already has a second customer waiting.
- The web **had** that geometry and deleted it in rpg-dnd5e-web commit
  `6503936` (`straightWallGeometry.ts`: Cyrus-Beck clip plus Sutherland-
  Hodgman coverage; `canvasFloor.ts`: the old threshold, inverse framing).
  Recoverable. The corner lattice and its point-to-cell inverse survive in
  `src/author/creation/hexCorner.ts`.
- No grid origin exists on either side, and under ruling 1 none is needed:
  "slide the grid" is a nudge to every wall's offsets.
- Placements today are constrained only by region membership. A compiler
  refusal for a placement left on a sliver, pointing at the thing, is an
  addition, not a rewrite.
- On the web this is a rewrite of the document layer (`RegionDoc.cells`, the
  `Edge` type, every mutator, `tautPath`), not a refactor. The rendering layer
  stays because it reads the atlas.

## The other half: let the walls decide the floor

Kirk: *"splay out a really large region, draw some walls and click trim edges
so remove the region outside the walls."* (rpg-dnd5e-web#910)

This is the same inversion from the other side. Every attempt so far tried to
make **hexes form a rectangle**. Trim lets the author **draw the walls and have
the floor conform** — they never fight the grid, because the grid is not what
they are drawing. Under ruling 6 the cells a wall cuts conform on their own;
trim remains the author's tool for painted cells wholly outside the walls.

## Adjacent, and not the same thing

**A region may end in a cliff** (rpg-toolkit#1443). Kirk: *"regions should be
allowed to have cliff edges. we roll dice off the table because of that."*
Today the toolkit rules that a floor-into-void crossing is *"a crossing nobody
can make"*, so every boundary is the same absolute barrier and a cliff cannot
be authored at all. Three edge behaviours are wanted; two exist:

| | today |
|---|---|
| sealed (implied envelope) | yes |
| wall (authored) | yes |
| **cliff (crossable, with a consequence)** | **no** |

It bears on this design because the concealment frontier check leans on the
envelope being an absolute seal, and because ruling 7 makes a wall against void
legal — the cliff is the third thing a floor edge can be, and it stays its own
issue.

Until then the builder draws that boundary **dashed and dim** — Kirk: *"walls
are intentional"* — so it says only "the floor stops here" and cannot be
mistaken for a wall nobody authored.

## What was thrown away, deliberately

A room tool that drew a rectangle of walls. It worked, and it was built on the
edge-slice primitive, so it would have been replaced by this design. Kirk:
*"we can throw it away now or we can throw it away later."* Cut before merge —
the reason is recorded in its removal commit so the measurement is not lost.

What survived from that slice is independent of this design and shipped in
rpg-dnd5e-web#903: the region rect, the editable YAML pane, the resizable rail,
and the dashed floor edge.

— cross-team agent, on behalf of KirkDiggler
