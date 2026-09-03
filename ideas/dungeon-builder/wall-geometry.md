# Wall geometry — why square rooms fight a hex grid, and what to author instead

**Status:** design in progress. Kirk's ruling on the geometry is in; three
rulings remain open.
**Issues:** rpg-dnd5e-web#908 (walls as lines) · rpg-dnd5e-web#910 (trim) ·
rpg-toolkit#1443 (cliff edges) · supersedes rpg-dnd5e-web#904 (snapping)
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
than 80% of it remains**, and stops being standable below that. Exact, not
approximate, and it is the discriminator the derived-crossing rule needs.

### What this dissolves rather than fixes

- **Snapping (#904).** Snapping exists so a freehand drag *derives* into a good
  staircase. A recorded line does not need the drag to be precise, only
  captured. Close as superseded.
- **"Square over hex."** The square was never made of hexes; it is a line, and
  the hexes are what it cuts.
- **Corner spurs.** A line has corners by construction.

## The other half: let the walls decide the floor

Kirk: *"splay out a really large region, draw some walls and click trim edges
so remove the region outside the walls."* (rpg-dnd5e-web#910)

This is the same inversion from the other side. Every attempt so far tried to
make **hexes form a rectangle**. Trim lets the author **draw the walls and have
the floor conform** — they never fight the grid, because the grid is not what
they are drawing. It composes with the line model directly: draw lines, trim
floor to them.

## Open rulings

1. **What space is the line in?** Free world units make "cuts wherever it
   likes" literal. The hex corner lattice keeps the file grid-anchored and
   diffable. This decides whether the derived-crossing rule is exact or
   approximate.
2. **Do edges stay in the file?** Deriving them is the cleaner single source;
   keeping them is friendlier to hand-editing and to everything that reads
   `walls[i].edges` today.
3. **Is the 80% rule mechanics?** It reads as *"you can stand there"*, which
   makes it a floor rule and puts it in the toolkit rather than the client.
   This decides whether the work is a web change or a three-repo slice.

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
envelope being an absolute seal.

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
