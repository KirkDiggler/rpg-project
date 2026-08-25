# Wall authoring gesture — design (web#804)

Addendum to the dungeon-builder v2 design (`design.md`), panel-back. Design-first
gate from rpg-dnd5e-web#804 (Kirk, walking PR #803, 2026-08-25): "tech the wall
is straight, it is just at an angle" — straightness LANDED with #800, but the
gesture didn't: "I also do not have a way to drag the wall across the hexes" and
"getting the corners to snap together and the whole feel of it is nuanced."
Today's gesture is click-one-edge-at-a-time; the straight run is derived AFTER
the fact from whatever chain resulted. The governing principle (recorded on
#800): the old builder felt right because the gesture itself was straight — the
author drew the line, the tool derived the cells.

Laws carried forward:

- **The YAML is the artifact; walls stay EDGES between adjacent floor cells**
  (design §2). This slice changes NO file field, NO wire field, NO toolkit or
  api code — it is entirely a web authoring-UX slice.
- **One geometry module is the only source of the straight picture**
  (`boundariesToWallRuns`, #800/#802). The gesture's live preview is computed by
  that same module on the candidate document — the preview IS the commit, never
  a second formula. (The door-frame lesson, verbatim class: two independent
  computations that only approximately agree are how "the walls do not touch"
  happens.)
- **Presentation never decides mechanics.** The committed truth is the edge
  chain; movement and LOS follow the edges; the straight run is how it reads.
  A free-angle run can visually clip cell interiors while mechanics follow the
  zigzag chain — already true of every landed run, stays true here.

## The gesture

Wall tool:

- **Click an edge** — today's single-edge toggle, unchanged (`nearestEdge`).
  The drag is additive, not a replacement.
- **Press–drag–release** — press anchors point A, drag shows the live result,
  release commits.
  - A and B snap to the **hex corner lattice**. This is the old builder's own
    corner-anchoring lesson carried forward (Kirk, live, pre-restart: "it
    always hangs over a little" — cell-center anchoring overshoots by up to
    half a hex by construction; corners are the finest honest lattice). The
    old `hexCorner.ts` solved corner-lattice addressing + dedup; resurrect it
    from rpg-dnd5e-web history (`git show 6503936^:src/author/creation/hexCorner.ts`) rather than re-derive.
  - While dragging, the tool derives the **candidate edge chain** (next
    section) and previews: the run(s) the shared module would render for
    current doc + candidate chain, plus a faint literal trace of the candidate
    edges. What the author sees mid-drag is float-identical to what release
    produces.
  - Release writes the candidate edges into `walls[]` through the existing doc
    mutators. Escape, or releasing with A = B, cancels.
- **Erase along a line** — shift-drag (or right-button drag) removes the
  `walls[]` edges along the same derived path, the grammar the region brush
  already uses for erase. Door edges are never touched by the wall eraser.

On a flat-top document the straightened picture is literal edges today (#763);
the gesture works identically there, previewing the literal candidate edges.

## Line → chain: the taut path on the corner lattice

The dragged segment A→B derives its chain as the **taut path** along the hex
edge lattice: walk corner-to-corner from A, at each corner stepping to the
incident lattice edge that advances toward B with least deviation from segment
AB. Each lattice edge walked separates exactly one adjacent cell pair — that
pair is one `walls[]` edge.

By construction:

- The chain is **connected** — no gaps to eyeball.
- A drag along one of the grid's own seam directions (pointy: the vertical
  column seam, the horizontal row seam, the two diagonal seam families) derives
  exactly the chain whose rendered run IS that line — the column/row cases
  axis-true by the authored-pair declaration (#802, 1e-6 pins), the diagonals
  by the fit's own symmetry over a periodic chain.
- Any other angle derives the zigzag chain whose fitted run best expresses AB —
  visible live during the drag, so an angled wall is chosen, seen, then
  committed: **angle as intent, never an emergent surprise**.

Derivation rules:

- A path segment whose cell pair is not two floor cells is **skipped** (the
  envelope is implied, never authored — design §2); the preview shows it
  absent.
- An edge already in `walls[]` is deduplicated silently — drawing over a wall is
  idempotent.
- An edge belonging to a door is **skipped and the chain breaks there** — runs
  already break at doorways; the preview shows the door sitting in its gap. An
  edge in both `walls` and a door is a validation failure the gesture simply
  never authors.

## Corners snap by construction

Two magnetisms on the drag endpoints, strongest first:

1. **Existing wall vertices** — the lattice vertices at the ends of existing
   authored chains. Landing there means the new chain shares that vertex, and
   because the whole picture is re-derived from ALL of `walls[]` by the one
   shared module, the two chains become one graph: the corner joint (or
   straight continuation) closes by construction.
2. **The corner lattice** itself.

The author never eyeballs whether two walls meet — ending near an existing wall
snaps to sharing its vertex, and a shared vertex IS a closed corner.

## Editing an existing wall

- **Select tool on a run** — clicking a rendered run selects the whole run: the
  set of doc edges behind it, resolved at click time from the derived scene.
  There is no wall id in the file and none is added; a wall selection is
  `{ kind: 'wall', edges: Edge[] }`. The inspector shows "Wall — N edges";
  Delete removes all N.
- **Endpoint grab (wall tool)** — hovering near a run's endpoint shows a
  handle; dragging it re-derives the chain from the OPPOSITE endpoint to the
  new position (same taut path) and replaces the run's edges on release.
  Extend and shrink are the same motion.
- **Move a whole run** — Not now; delete + redraw covers it until draw/extend
  has been walked.

## Doors compose

The door tool inherits the same drag derivation — design §1 already promised
"click an edge (or drag across several)"; today it is click-only. A door drag's
chain becomes ONE door's `edges[]`. Wall runs already break at door gaps;
nothing else to design.

## The feel (in scope)

- Live preview on every pointer move. The candidate derivation is O(chain) plus
  the shared module's O(walls) — the board already re-derives per doc change
  (#800 chose doc-derivation over the debounced server compile for exactly this
  latency).
- Pre-press hover keeps today's nearest-edge highlight (the click affordance);
  the drag affordance appears on press.
- Snap radii: corner magnetism ~0.4·size, run-endpoint magnetism ~0.6·size
  (stronger). Constants in ONE exported place, pinned by tests. The numbers are
  starting points — Kirk's walk is the calibration instrument, the same way
  facing yaw was measured, not inferred.
- Angle magnetism per ruling 1 below.

## Proposed rulings (Kirk rules once)

1. **Angle magnetism** — proposed: ON by default (drag direction within ~6° of
   a seam direction snaps the preview to it), hold Alt for free angle.
   Alternative: no snapping — the live preview alone carries the honesty.
2. **Endpoint-drag semantics** — proposed: re-derive the whole chain from the
   fixed far endpoint (a wall stays one straight intent). Alternative: append
   from the old endpoint (walls become polylines — but a polyline is two
   walls, and two walls are two drags).
3. **Erase gesture** — proposed: shift/right-drag along a line, same grammar as
   the region brush. Alternative: erase only via run selection + Delete.

## Coherence with #798 (designed together, built separately)

The shared direct-manipulation grammar, stated once so #798 inherits it:

- **Select the thing you see; edit the artifact it came from.** A run selects
  its edges; a 3D prop will select its `place[]` entry. The doc stays the
  artifact; every view (2D, YAML, 3D) is a projection of it.
- **Gestures preview through the same modules that render the commit** — never
  a second formula.
- **Magnetism over modes** — snapping does the precision work; modifier keys
  loosen it; nothing requires a dialog.

#798's specifics (raycast pick, rotate/nudge vocabulary under the orbit camera,
palette-to-floor placement) get their own addendum on this grammar when it
starts.

## Tests

- **Pixel-formula, no round-trips** (the symmetric-bug discipline): a table of
  named drags on a fixture floor — vertical-seam drag, horizontal-seam drag, a
  ~40° free drag, a drag across a door, a drag ending on an existing wall
  vertex — each asserting the EXACT derived edge list; for the seam drags, that
  the resulting run is axis-true to 1e-6 (the #802 pins guard the geometry;
  these pin the derivation).
- **Preview identity**: for each table row, the mid-gesture preview runs equal
  the post-commit runs float-exactly — the one-formula law as an assertion.
- **Erase inverse**: draw then erase along the same drag returns byte-identical
  YAML (the emitter is deterministic).
- Existing goldens unchanged: the gesture writes through existing mutators;
  reference-tomb fixtures never move.

## Not now

Move-whole-run; multi-select of runs; polyline walls in one gesture (a polyline
is N drags); touch/pen affordances; door-state editing inside the gesture (the
inspector's job); any file or wire change (there is none in this slice).
