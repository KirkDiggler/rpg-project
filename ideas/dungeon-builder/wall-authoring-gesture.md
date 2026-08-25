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
- **Endpoint grab** (ruling 5: on the SELECTED wall, any tool that can
  select) — hovering near a run's endpoint shows a
  handle; dragging it re-derives the chain from the OPPOSITE endpoint to the
  new position (same taut path) and replaces the run's edges on release.
  Extend and shrink are the same motion.
- **Shared-corner grab (wall tool)** — a vertex where two (or more) chains
  meet is itself a handle (ruling 4, Kirk's own addition). Dragging it moves
  the vertex on the corner lattice and re-derives EVERY incident chain from
  its own far endpoint to the new vertex — both walls re-angle in one motion,
  the corner stays closed by construction at every pointer position, and the
  live preview shows all affected runs through the shared module. The
  endpoint grab above is the one-incident-chain case of this same operation.
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
- Angle magnetism per ruling 1.

## Rulings (Kirk, 2026-08-25, on PR #267)

1. **Angle magnetism — RULED: ON by default.** Drag direction within ~6° of a
   seam direction snaps the preview to it; hold Alt for free angle.
2. **Endpoint-drag semantics — RULED: re-derive the whole chain** from the
   fixed far endpoint. A wall stays one straight intent; a polyline is two
   walls, and two walls are two drags.
3. **Erase gesture — RULED: shift/right-drag along a line**, the same grammar
   the region brush already uses.
4. **Shared-corner relocation — RULED IN** (Kirk, same message): "once snapped
   we should be able to relocate the corner that would drag both wall lines to
   reangle them." Designed in under §Editing: dragging a shared vertex
   re-derives every incident chain to the new position — both walls re-angle
   in one motion and the corner never opens.
5. **Manipulation rides selection — RULED** (Kirk, walking PR #808,
   2026-08-25): "walls look and can be drawn easily but continuing them and
   getting them lined up where I want is pretty tricky. i think if I select an
   existing wall, I should be able to manipulate it. I think the strava route
   builder does this pretty well." Selecting a wall shows its handles
   (rendered endpoints + shared corners) immediately, draggable right there —
   no tool switch, not hover-only. The wall tool keeps draw/erase. Same walk
   surfaced the snap failure ("I cannot get that upper right corner to snap
   in") — diagnosis + mechanism in the ledger below.

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
  vertex, a shared-corner drag on two snapped walls (both chains' re-derived
  edge lists exact, the new vertex still shared) — each asserting the EXACT
  derived edge list; for the seam drags, that
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

## Plan — one web PR

**Branch:** `feat/804-wall-gesture` · **Base:** `origin/dev` · **Repo:**
rpg-dnd5e-web · **Evidence:** `npm run ci-check`; the drag-table +
preview-identity tests; screenshots from the :3001 walk. One Copilot review on
open.

**Files**

- Resurrect `src/author/creation/hexCorner.ts` (+test) from history
  (`git show 6503936^:src/author/creation/hexCorner.ts`), adapted to
  `canvasGeometry.ts`'s orientation-aware SVG space (the old copy was
  pointy-only `hexLayout`).
- New `src/author/creation/wallGesture.ts` (+test) — the PURE derivation
  module, zero React: `tautPath(A, B, o)` → lattice edge path → `walls[]`
  edges (floor filter, door break, dedup); `snapGesturePoint` (wall-vertex
  magnetism, then corner magnetism; angle magnetism with Alt bypass);
  `wallVertices(doc)`; the erase path; ONE exported `GESTURE_TUNING`
  constants object (snap radii, angle tolerance) pinned by tests.
- `src/author/creation/boardWallRuns.ts` — thread each run's source edge list
  through additively (presentation metadata; the shared 3D module's geometry
  is untouched) so a rendered run can answer "which doc edges am I" for
  hit-testing and selection.
- `src/author/types.ts` — `Selection` gains `{ kind: 'wall'; edges: Edge[] }`.
- `CreationBoard.tsx` — press/drag/release for wall + door tools; the preview
  layer (candidate runs via `boardWallScene` on the candidate doc + faint
  literal trace); erase modifier; run hit-testing for Select; vertex/endpoint
  handles with the re-derive drags (rulings 2 and 4).
- Inspector — "Wall — N edges" panel with Delete.

**Order within the PR** (each unit's tests green before the next):

1. hexCorner resurrection + adaptation.
2. wallGesture derivation + the drag table (TDD — §Tests' rows written first).
3. Board wiring: draw + erase gestures with live preview; preview-identity
   test.
4. Snapping + `GESTURE_TUNING`.
5. Selection + handles: run select, Delete, endpoint drag, shared-corner drag.
6. Door-tool drag.

**Done when:** ci green; the drag table and preview-identity tests pin the
derivation; Kirk walks :3001 parked at the PR head — draw a wall in one drag,
snap a second to its corner, grab the shared corner and re-angle both, erase
one with shift-drag — and the feel verdict is his. `GESTURE_TUNING`
recalibrates from the walk before merge, not after.

## Adjustments (ledger — filled during implementation)

| date | where | designed | landed | why |
|---|---|---|---|---|
| 2026-08-25 | web PR #808 | taut path "least deviation" left ties unspecified | ties break toward greater x, then greater y — geometric, direction-independent, pinned by the drag table | a drag exactly along a corner column reaches corners where both zigzag sides advance with identical deviation; the walk needs a deterministic answer |
| 2026-08-25 | web PR #808 | corner magnetism ~0.4·size read as an endpoint snap radius | endpoints ALWAYS land on the corner lattice; 0.4 became the handle-pickup radius, 0.6 the wall-vertex magnetism gate, plus runHitRadius 0.25 for select-tool hit-testing — all in `GESTURE_TUNING`, all pinned | "A and B snap to the hex corner lattice" is unconditional in §The gesture; a radius beyond which the endpoint floats free would contradict it |
| 2026-08-25 | web PR #808 | angle magnetism stated for the draw drag; reshape drags unaddressed | endpoint/shared-corner reshape drags skip angle magnetism (wall-vertex + lattice magnetism still apply; the grabbed runs' own vertices excluded from magnetism) | a reshape has multiple chains and no single drag origin to measure an angle from |
| 2026-08-25 | web PR #808 | door tool "inherits the same drag" | door drag = draw only; no door erase gesture (door shift/right keeps the immediate toggle) | ruling 3 makes the wall eraser door-blind, so a door erase drag has nothing to mean |
| 2026-08-25 | web PR #808 (walk round, 5bfc1da) | wall-vertex magnetism targeted chain-end LATTICE vertices | magnetism, handle display, and pickup all target the RENDERED endpoint (mean of incident runs' drawn ends); the lattice ref stays the derivation anchor | Kirk's walk: "cannot get that upper right corner to snap in." Measured: the fit/closure moves a drawn end 0.15–0.52·size off its lattice vertex, so aiming at the visible corner missed the 0.6 gate — the see-vs-snap mismatch. "Select the thing you see" applies to magnetism too |
| 2026-08-25 | web PR #808 (walk round, 5bfc1da) | endpoint grab = wall-tool hover | ruling 5: handles ride selection, draggable with Select; wall tool is pure draw/erase | pressing within pickup range of a chain end — the exact place you press to CONTINUE a wall — started a reshape instead of a draw; continuation was structurally impossible near endpoints |
| 2026-08-25 | web PR #808 | shared-corner handle position | lives at the scene's own run-break vertex (the chaining engine's straightness judgment), which can sit an edge past the drag boundary | that IS the corner the author sees and grabs; pinned by test |
| 2026-08-25 | web PR #808 (walk round 3, 3a98303) | one shared geometry module was assumed to guarantee 2D === 3D | the module was input-ORDER-sensitive at branch vertices: 2D fed stroke order, 3D fed the server's compile order, and Kirk's 3-way seam corner rendered differently in each (fitted lines half a hex apart; 20/20 permutations differed). Fix: `boundariesToWallRuns` canonicalizes its edge list before chaining; pinned by seeded-permutation float-exact invariance | the one-formula law needs order-invariance to actually hold — a shared formula fed different orders is two formulas. The reference-tomb golden never caught it (no 3-way junction, orders happened to agree) |
| 2026-08-25 | web PR #808 (walk round 3, 3a98303) | `walkChain` breaks at any branch vertex per its own header contract | it judged "branch" by UNUSED incident edges, so once an earlier chain consumed one arm, a later walk sailed THROUGH the junction and absorbed the next chain's edges — the 3D overshoot Kirk photographed. Fix: break on TRUE vertex degree; the #793/#794 three-member corner closure (built for exactly this, barely reachable before) now closes the junction; pinned by a 3-way-corner fixture | found because the walk's screenshot showed the east wall's top missing a corner the 2D closed; zero existing goldens moved |
| 2026-08-25 | web PR #808 (walk round 2, cc397a4) | 3D preview silently kept the last-good atlas during validating/errors/unreachable | `staleAtlasNotice` banners the 3D view whenever the atlas on screen is not the current document's own compile | lag read as broken geometry during the walk; a named stale state is diagnosable, a silent one is a wild goose chase |
| 2026-08-25 | web PR #808 (98d49cd) | "order" was the named divergence axis | the two callers differ on MORE: pair direction (web normalizes (r,q), server (q,r) — door.go:306) and doorway ordering; a door-on-the-arm 3-way fixture fed both styles now pins convergence to trig noise (1e-9 tolerance, explicit headroom) | the one-formula law holds only if EVERY representational difference between callers is canonicalized or proven self-cancelling — direction and doorway order were latent variants of the same class |
