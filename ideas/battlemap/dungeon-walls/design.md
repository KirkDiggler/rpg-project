# Dungeon Walls: straight modular walls at the room envelope

**Status:** design for review
**Parent:** rpg-project#132 (Dungeon visual fidelity umbrella) · relates to `ideas/battlemap/dungeon-authoring/` (PR #121) · supersedes the render layer of rpg-toolkit#848 (whose findings report is this design's data reference)

## Problem

Walls are rendered per hex cell today: the client draws each wall entry the
toolkit emits, cell by cell. On a hex grid that produces zig-zag "teeth" along
every room edge, plus the degenerate rubble blocks flanking doors diagnosed in
toolkit#848. It also fights the art direction: the Synty POLYGON Dungeon pack —
and the promo shot that is this initiative's quality bar — is built from
straight modular wall segments, corner pieces, and doorway frames. As long as
walls follow hex boundaries, those assets cannot be used as designed.

## Principle

> toolkit is the rules, we are the presentation — Kirk, 2026-07-25

The hex grid is the *rules* model: walkability, movement, LOS, blocking —
server-authoritative (the server already rejects invalid movement; client
clickability follows the same data). The wall *mesh* is presentation, and
nothing requires presentation to be hex-shaped. This design splits the layers:

- **Toolkit (rules): unchanged.** Cells, doors, blocking, LOS stay exactly as
  emitted today. Zero toolkit/proto/api changes in v1.
- **Client (presentation):** stops rendering room-boundary wall entries.
  Instead it derives **wall runs** — straight segments along each room's outer
  envelope *and along each connector column* — and lays Synty modular pieces
  (segments, corners, door frames) along them.

**Corollary (the invisible-wall rule): every server-side blocker must have a
visual representation.** Blocking is server-authoritative and unaffected by
what the client draws — but a blocked cell rendered as open floor is a worse
bug than ugly walls. The runs below cover every boundary and connector blocker.

## Geometry

- **Room envelope runs**: the wall line sits at the **outer envelope** of each
  room's playable hex region — the straight bounding line at the outermost
  cell extent per side.
- **Connector runs**: adjacent rooms are separated by a reserved one-column
  gap that belongs to no room (that gap *is* the connector — see
  Implementation notes). The gap's non-door cells are real server-side
  blockers, so the client draws **one straight wall along each connector
  column**, with the opening at the door cell. This replaces today's rubble
  blocks with the same visual language as the rest of the walls.
- The one hard constraint (Kirk): **character models standing in any playable
  cell must not clip through the wall.** Boundary-cell centers sit at least
  half a hex from the envelope, so an envelope-line wall with standard Synty
  thickness clears standard character radius — verify with the largest
  character/monster mesh before locking the offset. If clipping appears, push
  the wall outward; never mask cells to solve clipping.
- **No cells are masked.** Every cell playable today stays playable and
  clickable; there are no exposed hex teeth — the wall thickness swallows the
  scallops.
- The half-hex scallop strip between the offset columns' floor tiles and the
  wall base is covered by presentation: extend the floor slab / skirting to
  the wall line.

## What the client's data actually says (toolkit#848 findings)

The wire already separates everything this design needs (full detail in the
findings report on toolkit#848):

- **Props vs walls are different channels.** Obstacles (pillars, coffins,
  braziers, …) ride the *entities* list as Obstacle payloads with content refs
  and per-viewer reveal gating. Walls are a flat, unconditional per-room list.
  No ambiguity to resolve.
- **Within the walls list**: boundary-edge entries (`From != To`) are room
  boundaries — under this design the client stops rendering these and draws
  envelope/connector runs instead. Degenerate entries (`From == To`) are
  interior pattern walls — those **still render** (the crypt has them; the
  reference tomb doesn't). Door entries carry `Kind = DOOR_*` and the only
  non-nil `Id`s in the list — they key door-frame placement and locked-state
  rendering exactly as today (lock DC/ability stay server-side).
- Rules are untouched by any of this: movement/LOS blocking is rebuilt
  server-side from the data and never reads the client's mesh choices.
- **Region membership is per-viewer reveal-gated** (added post-review): the
  hex membership reaching a client is the REVEALED subset (sight-range gated),
  not full room membership — while the walls list is whole-room and
  unconditional. Any geometry derived from region hexes therefore tracks the
  player's explored frontier, not the room. Caught by gate review on the W1
  implementation; governed by the section below.

## Fog of war / partial reveal (v1 decision, 2026-07-25)

Because region membership is reveal-gated, runs derived from it follow the
explored frontier. v1 decisions:

- **Envelope runs trace the frontier.** The wall line sits at the edge of the
  explored area and steps outward as more of the room is revealed — accepted
  as the fog-of-war visual for v1; revisit after Kirk's walk if it reads
  badly. (Future alternative needing no backend change: derive full-room
  envelopes from the unconditional walls channel, which already carries
  whole-room boundary edges.)
- **Two-tier exception, now load-bearing (PR #626):** a region's row extent
  tracks its own frontier exactly as above UNLESS it participates in a
  validated connector, in which case it may snap early to the connector's
  proven extent via the shared-Height assumption (every region shares one
  `DungeonParams.Height`, so widening to a peer's proven extent is never
  wrong geometry — just early). Isolated regions, and regions whose peer
  hasn't revealed further either, still track the frontier. Column extent is
  never widened this way — room widths genuinely differ, unlike height.
- **Connector coverage must not depend on reveal state.** Doors pair to the
  nearest region on each side (not exact column adjacency), and — the
  structural guarantee — any connector-flanking wall entry not covered by an
  emitted run FALLS BACK to per-cell rendering. The invisible-wall rule is
  enforced by construction, not by hoping reveal state cooperates.
- The clip-clearance dial's default is ~√3 hex units, not 1.0 — the row
  staircase eats 0.69–0.87 of a nominal offset (measured in review); W4 tunes
  from there.
- Different-height adjacent rooms: connector runs take the union of the two
  row ranges — fine at uniform height (all current content), documented as a
  known v1 limit.
- **Load-bearing coupling (for any future server change):** the client
  derives true grid bounds from `Wall.from` min/max over the walls list,
  which works because walls are projected whole-room and never reveal-gated
  (the proto exposes no space dimensions). If per-viewer wall reveal is ever
  added server-side, that derivation silently narrows and the invisible-wall
  class returns — put this on that change's checklist.
- **Wall height and cutaway now exist** (PR #626): a `?wallHeight=` dial and
  a `?wallCutaway=1` prototype classifying each run/partition as
  camera-facing (stub) or away-facing (tall), including a peel-away rule for
  connector doors/walls that sit between the camera and the player's own
  current room — see that PR for the full geometry and Kirk's default-look
  decision.

## Implementation notes (feeds plan.md)

From the toolkit#848 findings — the traps a web implementer must respect:

- No per-room width/offset exists on the wire: derive each room's bounding
  rect as min/max over `RegionData.Hexes`. `Space.Width/Height` is the
  combined space, not the room.
- The reserved one-column gap between consecutive rooms belongs to no region's
  `Hexes` — that's the connector column, not a data error. A cell in bounds,
  in no region, and not a door position is a connector flanking cell.
- Every door in a dungeon sits on the same `doorRow = height/2`. Boss rooms
  have their entire doorRow deliberately open across their full width
  (toolkit#819 tactical invariant) — not a generation bug.
- Hex adjacency must use real cube/neighbor math (the six direction vectors /
  `HexNeighbors`), never same-row column-stepping: in the odd-q scheme,
  stepping to an adjacent column changes row depending on column parity.
  Same-column row steps are safe.

## Deferred (explicitly out of v1)

- **Breakable wall sections.** If wanted later, an authored `place:`-able
  breakable-wall prop with its own mesh — not a property of every wall cell.
  (Wave 1 persists no destructibility model; emitted walls are indestructible.)
- **Toolkit-emitted wall runs.** If the server ever needs runs (destructible
  runs, procedural wall variation), promote run computation into the toolkit
  and project it. v1 keeps it client-derived because it is pure presentation
  policy with zero proto/api/toolkit churn.
- **Interior pattern-wall restyle.** Crypt-style interior walls keep their
  current per-cell rendering in v1; restyling them to modular pieces can
  follow once the envelope/connector language is proven.

## Interim: toolkit PR #849

PR #849 (the original #848 fix, already implemented and live-verified) merges
independently of this design as the interim improvement: it converts the
connector rubble to uniform boundary-edge walls in the *current* renderer, so
doors look right now, and leaves the walls list uniformly shaped for this
design's filter. This design later supersedes how those entries are drawn —
not their existence or their blocking role.

## Verification

Screenshot-judged (per the team's rigor calibration: visual work is judged by
the harness + Kirk's eyes against the Synty bar, not review machinery):

- reference-tomb before/after at matching camera angles;
- clip-check with the largest model standing in boundary cells on both the
  flush and scalloped columns;
- door areas at both connectors — frame, opening, locked-state rendering;
- Kirk walks it.

## Rollout

1. toolkit#848 findings are on the issue (done — data reference for this doc).
2. toolkit PR #849 merges as the interim fix (independent track).
3. Web implementation issue in rpg-dnd5e-web (asset-pipeline team, client
   seams): envelope + connector run computation, modular wall placement, floor
   skirt, boundary-entry render filter. Replaces the old boundary-wall
   rendering outright.
4. Kirk walk + screenshot set; asset gaps found along the way feed the M3
   backlog.

— asset-pipeline agent, on behalf of KirkDiggler
