# Fog of war — draw what this seat knows

Tracking: [rpg-project#586](https://github.com/KirkDiggler/rpg-project/issues/586).
Walkthrough: [README.md](README.md). Plan: [plan.md](plan.md).

## Shape

```mermaid
flowchart LR
    subgraph enc[toolkit encounter]
        Pass["sight pass<br/>sightReach.reachesCell"]
        Learn["learnCells<br/>fact known:cells (per member)"]
        Atlas["AtlasFor(member)<br/>withholds unknown cells"]
        Seen["VisibleCellsFor(member)<br/>seen now ∩ known"]
        Obs["PropSightings / DoorSightings /<br/>perception holdings"]
        Pass --> Learn --> Atlas
        Pass --> Seen
        Pass --> Obs
    end
    subgraph ses[toolkit session]
        Know["Manager.Knowledge / View<br/>ViewOutput.VisibleCells"]
        Beat["EventCellsRevealed<br/>(per recipient)"]
    end
    subgraph wire[protos + rpg-api]
        View["GetViewResponse.visible_cells"]
        AtlasW["GetAtlasResponse (per-cell)"]
        Ev["CELLS_REVEALED"]
    end
    subgraph web
        State["per-cell state:<br/>seen / remembered / absent"]
        Draw["lit · grey · nothing<br/>soft edge"]
    end
    Atlas --> Know
    Seen --> Know
    Obs --> Know
    Learn --> Beat
    Know --> View & AtlasW
    Beat --> Ev
    View & AtlasW & Ev --> State --> Draw
```

Every seat holds three grains of the map. **Seen now** is the cells its own
character's sight reaches this moment. **Remembered** is every cell that
character has ever learned and does not see now. **Unseen** is everything else,
and it is absent from what the seat receives. The engine decides all three; the
session delivers them; the client draws the state it is sent.

## Law

- **The cell is the unit of knowledge.** A member learns a cell when the sight
  pass's `reachesCell` answers true for it, when it opens a door on that cell's
  edge, or when an explicit concealment reveal teaches it. Nothing teaches a
  whole region at once. (R1)
- **One sight test.** Seen-now and learning both ask `sightReach.reachesCell`
  in the same pass's geometry; no second visibility algorithm exists anywhere,
  client included. (R5)
- **Seen now is per seat and never stored.** `VisibleCellsFor(member)` is the
  known cells the member's sight reaches at the moment of the read. It is never
  a union across the party and never persisted; it is derived from current
  positions. (R1)
- **Remembered is per seat.** A learned cell is a per-member journal fact,
  audience that member alone. Memory is never forgotten within the run. (R4)
- **Unseen is absent.** `AtlasFor(member)` withholds every unknown cell, and
  every wall, door, doorway, fixed prop and placement that touches only unknown
  cells. A presented wall adds no cell of its own to the atlas. (R2, R9) A door on the edge of a known cell is presented; a door with no known
  cell on any edge is absent, exactly like the wall it stands in. (R2)
- **Unknown floor is not a wall.** Withholding unexplored space adds no
  synthesized boundary; only concealment masquerades as wall. The two masks stay
  separate.
- **Mutable things keep their observation lifecycle.** Holdable props and door
  state are `PropSightings` and `DoorSightings`; creatures are perception
  holdings. Current means `CurrentVia` non-empty; remembered means empty. The
  atlas never carries them. (R3)
- **The session projects, never computes.** `ViewOutput.VisibleCells` and
  `KnowledgeOutput` pass the encounter's answers through unchanged; the
  per-recipient stream delivers newly learned cells as one cells-revealed beat
  per member per pass. (R5)
- **The wire is additive.** `GetViewResponse.visible_cells` and the
  `CELLS_REVEALED` kind are new. `GetAtlasResponse`'s schema is unchanged and
  its contract becomes per-cell. `ROOM_REVEALED` is deprecated and no longer
  produced.
- **The client draws three states and nothing else.** A seen-now cell draws lit.
  A remembered cell draws in the remembered treatment: a lighter grey shadow
  with its shape readable, its fixed layout, and its props and creatures as
  last seen. An absent cell draws nothing. Walls and doors take the brightest
  state of the cells they border. (R3)
- **A wall is known where sight reached its footprint (R9).** A presented
  wall carries the hexes its blocker footprint overlaps; a stretch of it is
  known only where one of those hexes is known, and presenting a wall teaches
  no footing hex. A footprint hex is learned by the cover rule, from the wall
  face sight reaches. Why: footing that arrives with the wall is evidence of
  the wall, not of sight, and grading by it drew the wall where nobody had
  looked.
- **The masquerade is the wall (R9).** For a member who has not found a bound
  door, the wall it masquerades as covers and teaches: the door's threshold hex
  is learned from the wall face and presented as sealed footing in its region,
  so the member's atlas equals the solid-wall twin's. Concealed cells under a wall are
  withheld as they always were, a threshold that is itself concealed included. Why: a gap where the secret door stands is the
  tell concealment exists to remove.
- **Sharing is a choice (R8).** Discovery sharing defaults off, so each
  player's map is what that player has seen until the party turns sharing on.
  Why: a default of off shows more in playtests than a default of on.
- **Soft edges, no wedges.** The boundary between seen-now and remembered is a
  short gradient over at most two cells, computed from the per-cell states the
  client was sent. It never extends into an absent cell and never draws a cone,
  ray or polygon. (R5)

## Rulings

| ID | status | scope | ruled by | date |
|---|---|---|---|---|
| R1 | settled | Live sight is per seat and per hex: a client draws the hexes its own character sees now, never a party union; carries the earlier per-viewer visibility ruling of rpg-project#130 | KirkDiggler | 2026-10-15 |
| R2 | settled | A never-seen cell draws nothing; an unobserved door is an unobserved wall; first sight reveals wall, door or floor as they are | KirkDiggler | 2026-10-15 |
| R3 | settled | Remembered draws as a lighter grey shadow with land shape readable, props and creatures as last seen; tint (grey or sepia) is a presentation choice still open | KirkDiggler | 2026-10-15 |
| R4 | settled | Memory is per seat; the existing discovery-sharing setting is the one switch that pools remembered across the party; live sight stays per seat regardless | KirkDiggler | 2026-10-15 |
| R5 | settled | No cone wedges: hex-based reveal with soft edges; the client computes no visibility, the session projection owns delivery (rpg-project#541) | KirkDiggler | 2026-10-15 |
| R6 | superseded by R1 | The region partition as the unit of reveal, including the engine's whole-room discovery | KirkDiggler | 2026-10-15 |
| R7 | superseded by R4 | Remembered map shared by the whole party by default | KirkDiggler | 2026-10-15 |
| R8 | settled | discovery sharing defaults off; a party pools memory by choosing to; playtests may move the default | KirkDiggler | 2026-10-15 |
| R9 | settled | the blocker footprint that stops sight is what reveals a wall: a wall is known where sight reached a hex its footprint overlaps; presentation footing (wall-geometry C18) is superseded under fog; for a non-knower an unfound bound door masquerades as the wall it stands in (C19 extended into sight); concealed footing stays withheld (O4) | KirkDiggler | 2026-10-16 |

## Open

- **O1 — the pooling fold under discovery sharing.** R4 names the switch and
  R8 sets its default; the fold itself is not built. When it is, a sharing
  member's learned cells join every placed player's remembered set, as
  `discoveryAudience` already does for concealment results.
- **O2 — remembered tint.** Grey or sepia (R3). The client's remembered
  material is one value; the choice does not touch the wire.
- **O3 — older working documents.** `production-design.md`, `handoff.md` and
  `progress.json` predate this design and describe the web concept route. They
  are not law; where they disagree, this document wins.
- **O4 — walls of a secret room.** A structural wall whose footprint hexes are
  a concealed region's cells withholds those hexes for a non-knower, so under
  R9 such a wall grades by its corridor-side hexes only; a centre-line wall
  through concealed cells would show stubs. No authored secret room exists yet
  to say what the non-knower should see; the twin of a concealment that is the
  footing hex itself is not obvious. Settle with the first such room.
- **O5 — a bound door in a hidden wall.** When the parent wall is itself
  withheld from a member, the unfound leaf blocks sight into its threshold and
  teaches nothing, while the twin shows that hex as the wall's body would:
  floor if the body does not block, nothing if it does. The engine cannot
  answer because a structural wall declares no body blocker of its own; its
  blocking spans are separate placements linked only by name. This predates
  R9. The primitive is a wall that declares whether its body blocks movement
  and sight; the masquerade then answers exactly as the twin's solid wall
  would. Settle when a hidden wall with a secret door is authored.
