# Dungeon Walls: straight modular walls at the room envelope

**Status:** design for review
**Parent:** rpg-project#132 (Dungeon visual fidelity umbrella) · relates to `ideas/dungeon-authoring/` (PR #121) · supersedes the render-layer half of rpg-toolkit#848

## Problem

Walls are rendered per hex cell today: the toolkit emits perimeter wall entities
cell by cell and the client draws each one. On a hex grid that produces zig-zag
"teeth" along every room edge, plus the degenerate rubble blocks flanking doors
diagnosed in toolkit#848. It also fights the art direction: the Synty POLYGON
Dungeon pack — and the promo shot that is this initiative's quality bar — is
built from straight modular wall segments, corner pieces, and doorway frames.
As long as walls follow hex boundaries, those assets cannot be used as designed.

## Principle

> toolkit is the rules, we are the presentation — Kirk, 2026-07-25

The hex grid is the *rules* model: walkability, movement, LOS, blocking —
server-authoritative (the server already rejects invalid movement; client
clickability follows the same data). The wall *mesh* is presentation, and
nothing requires presentation to be hex-shaped. This design splits the layers:

- **Toolkit (rules): unchanged in v1.** Cells, doors, blocking, LOS stay
  exactly as emitted today.
- **Client (presentation):** stops rendering the per-cell room-boundary wall
  entities. Instead it derives each room's **wall runs** — straight segments
  along the room's outer envelope, with gaps at doors — and lays Synty modular
  pieces (segments, corners, door frames) along them.

## Geometry

- The wall line sits at the **outer envelope** of the room's playable hex
  region: the straight bounding line at the outermost cell extent on each side.
- The one hard constraint (Kirk): **character models standing in any playable
  cell must not clip through the wall.** Boundary-cell centers sit at least
  half a hex from the envelope, so an envelope-line wall with standard Synty
  thickness clears standard character radius — verify with the largest
  character/monster mesh before locking the offset. If clipping appears, push
  the wall outward; never mask cells to solve clipping.
- **No cells are masked.** With the wall at the outer envelope there are no
  exposed hex teeth — every cell playable today stays playable and clickable.
  (The earlier "unclickable exposed hexes" idea dissolves: nothing is exposed.)
- The half-hex scallop strip between the offset columns' floor tiles and the
  wall base is covered by presentation: extend the floor slab / skirting to the
  wall line. (Implementation detail for the web work — floor is presentation
  too.)

## Doors

Wall runs carry gaps at connector columns. Door position and locked state are
already in the client's data (door wall kinds incl. locked); the gap gets a
Synty door-frame piece and locked state renders as it does today. Door ids and
flank data are being confirmed by the toolkit#848 investigation (see Inputs).

## What happens to the per-cell wall entities

They keep existing in the data — rules (blocking/LOS) are untouched — the
client just stops *rendering* the room-boundary ones. Interior obstacles
(pillars, coffins, placed props) obviously still render; the client needs a
reliable way to distinguish boundary-wall entities from interior obstacles.

The toolkit#848 investigation (in flight) answers:
1. which entity types/labels identify room-boundary walls in the client's data,
2. whether the degenerate connector-column rubble blocks carry any rules
   semantics or are render-inert noise,
3. what data keys door-frame placement,
4. coordinate quirks a web implementer computing envelopes should know
   (doorRow reservation, per-room offsets).

Depending on (2), toolkit#848 becomes a small data-hygiene cleanup or closes as
superseded.

## Deferred (explicitly out of v1)

- **Breakable wall sections.** Awkward on a continuous run; if wanted later it
  becomes an authored, `place:`-able breakable-wall prop with its own mesh —
  not a property of every wall cell.
- **Toolkit-emitted wall runs.** If the server ever needs to know about runs
  (destructible runs, procedural wall variation), promote run computation into
  the toolkit and project it. v1 keeps it client-derived because it is pure
  presentation policy and needs zero proto/api/toolkit churn.

## Verification

Screenshot-judged (per the team's rigor calibration: visual work is judged by
the harness + Kirk's eyes against the Synty bar, not review machinery):

- reference-tomb before/after at matching camera angles;
- clip-check with the largest model standing in boundary cells on both the
  flush and scalloped columns;
- Kirk walks it.

## Rollout

1. toolkit-w's findings land on toolkit#848 (entity taxonomy + rules-semantics
   answer) — input to the web work, no toolkit code change expected.
2. Web implementation issue in rpg-dnd5e-web (asset-pipeline team, client
   seams): envelope computation + modular wall placement + floor skirt +
   boundary-entity render filter. Replaces the old wall rendering outright.
3. Kirk walk + screenshot set; asset gaps found along the way feed the M3
   backlog.

— asset-pipeline agent, on behalf of KirkDiggler
