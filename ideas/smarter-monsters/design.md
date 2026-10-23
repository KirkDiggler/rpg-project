# Smarter monsters: pursuit

## Shape

```mermaid
stateDiagram-v2
    [*] --> none: never saw an enemy
    none --> seen: sight lands
    seen --> remembered: sight lost (testimony kept, not current)
    remembered --> seen: sight lands again
    remembered --> lost: decider stands on the remembered cell, or no route reaches it
    lost --> seen: sight lands again
    lost --> none: search leg ends (testimony becomes unknown)
```

Each state is a band of `when: { enemy: … }`, read from the decider's own
sight testimony of the subject. The rows an author writes per band carry the
behaviour:

| band | row in the castle | what the guard does |
|------|-------------------|---------------------|
| `seen` | `toward: enemy` | closes on where it sees the hero |
| `remembered` | `toward: enemy` | walks to the cell it last saw the hero on |
| `lost` | `search: { distance: 6 }` | walks on from there, along the heading it holds |
| `none` | `toward: { path }` or `toward: { at }` | resumes its loop or post |

## Law

- A pursuit reads only the decider's own testimony. The remembered cell and
  heading are what this observer last saw, stale or wrong, never the subject's
  live position.
- `enemy: remembered` holds when the decider has a placed sight testimony of an
  opposed member that is not current and not lost.
- `toward: enemy` with no opposed member in sight walks the route to the
  remembered cell. No second selector word names the same target.
- A remembered testimony becomes lost when the decider stands on its cell, or
  when no route reaches that cell. Lost keeps the cell and the heading.
- `enemy: lost` holds when the decider's nearest opposed testimony is lost and
  none is seen, in reach or remembered. The bands stay exclusive.
- `search: { distance: N }` walks toward the cell N cells from the lost cell
  along the held heading. N defaults to 6 and is at least 1.
- The held heading is the subject's facing in the testimony. With none held,
  it is the decider's own facing on arriving, which is its direction of travel.
- A search ends when the decider stands on the search cell or no route brings
  it closer. Its testimony of that subject then becomes unknown: known to
  exist, not placed.
- Return is the `none` band. Unknown testimony places nobody, so the rows the
  author wrote under `enemy: none` fire, and a patrol resumes from the nearest
  waypoint. A decider with no such row stands where the search ended.
- Fresh sight replaces a lost or unknown testimony, so a glimpse at any point
  restarts the chase.
- A decider that never saw the subject holds no testimony of it and never
  pursues.
- The search holds no state of its own. The search cell is derived each turn
  from the lost testimony and the row's distance, as a patrol's next leg is
  derived from where the walker stands.

## Rulings

| ID | ruling | status | scope | ruled by | date |
|----|--------|--------|-------|----------|------|
| R1 | `enemy: remembered` reads the decider's own non-current, placed, not-lost sight testimony of an opposed member | open | encounter bands | proposed | 2026-10-22 |
| R2 | pursuit walks with the existing `toward: enemy`; no `toward: remembered` word | open | table vocabulary | proposed | 2026-10-22 |
| R3 | arriving on the remembered cell, or finding no route to it, turns the testimony lost; `enemy: lost` is a fifth band | open | encounter bands, testimony | proposed | 2026-10-22 |
| R4 | `search: { distance: N }`, default 6, walks to the cell N along the held heading (subject's seen facing, else the decider's own facing) | open | table vocabulary | proposed | 2026-10-22 |
| R5 | a search's end turns the testimony unknown; return is the author's own `enemy: none` rows, not a word | open | table vocabulary, testimony | proposed | 2026-10-22 |
| R6 | a decider that never saw the subject holds no testimony and never pursues | open | encounter bands | proposed | 2026-10-22 |
| R7 | the words `lost` and `search` are sealed in mind/behavior, so the slice is two module PRs: mind/behavior first, then rulebooks/dnd5e/encounter on its pseudo-version | open | delivery | proposed | 2026-10-22 |

## Open

- Dash while out of reach (rpg-project#666, shared with #661) is not in this
  slice. Pursuit walks at the decider's movement.
- Whether a search leg should sweep, turning to face as it walks, or walk a
  straight line. This design walks the router's route to the search cell.
- Whether allies share a lost or unknown testimony by report. This design
  keeps each guard's search its own.
