# Authored wall height — design + plan (rpg-project#273)

Addendum to the dungeon-builder v2 design (`design.md`), panel-back. Builds on
the walls thread (#788/#794/#802 axis-true runs, `wall-authoring-gesture.md`'s
drag-to-author + selection grammar). Kirk's ruling (2026-08-25, after the #804
walk): "setting the wall height I see as the next priorities."

## The identity question (the design's center)

`walls:` is a flat list of adjacent-cell edge pairs; runs are DERIVED by the
one shared engine (`boundariesToWallRuns`). A "wall" the author sees is not a
thing the file stores — so who carries a height?

**Proposal: the edge carries it.** The dialect grows an optional object form
per entry; the bare pair stays valid and means default height:

```yaml
walls:
  - [[5,0],[6,0]]                          # bare pair — default height
  - { between: [[5,1],[6,1]], height: 2 }  # double-height wall
```

- `height` is a MULTIPLIER of the standard rendered wall height (1.0 =
  exactly today's walls, 2.0 = double), valid in [1, 3]. Raise-only, per
  Kirk's ruling (below): no value below standard, and no `0` — an absent
  wall is an absent entry, not a zero-height one.
- Why the edge and not some named-run object: runs are derived, order-free,
  and reshape under editing; giving them stored identity re-opens everything
  #804 just closed (order-invariance, the walkChain degree rule). An edge is
  the one stable unit of authored fact. The gesture already writes N edges
  per stroke; the height control writes the same N.

**Chain-breaking:** height joins the run-identity criteria inside
`boundariesToWallRuns` — a chain SPLITS where adjacent edges disagree on
height, exactly as doors split runs today. Uniform-height documents derive
exactly today's runs (golden: the tomb, all-default, byte-identical). The
canonical, order-invariant engine from #804 is the only place this logic
lives; 2D preview, 3D preview, and the game route all inherit it.

## The mechanics ruling (RULED — Kirk, 2026-08-25)

> "yeah visual only … and yeah a wall cannot be seen past. I am looking to
> raise the walls not lower them now."

Three facts, and the second makes the third cheap:

- **Visual only.** Height changes nothing in Sight or movement — a wall
  blocks exactly as today at any height. The law holds: presentation never
  decides mechanics.
- **Raise-only.** Heights below standard are refused (`[1, 3]`), so the
  waist-high parapet — the one case where "visual only" was genuinely
  contestable, because seeing over a low wall IS mechanics — cannot be
  authored at all. The contestable case is not shelved; it is unexpressible.
- **A wall cannot be seen past**, at any authored height. This is the
  existing Sight behavior restated as a design invariant of this slice, not
  a new rule. If low walls ever return, they must bring their own ruling and
  a Sight design (cover, elevation) with them.

## The wire (additive)

`AtlasBoundary` gains `float height = 5` — the
authored multiplier verbatim when one was authored, `0` = not authored.
Because the YAML bounds are `[1, 3]`, a literal `0` can never be an authored
value, so the wire is unambiguous — but spell the client contract out to
kill the multiply-by-zero trap: a reader maps `0` to the STANDARD height
(i.e. renders as multiplier `1.0`), and never multiplies by the raw field.
"Said nothing" and "said 1.0" render identically by design. No pixels on the wire; the client multiplies
into its own calibrated wall height. The web end lands on an existing knob:
the renderer already draws per-run height overrides (the cutaway machinery's
`WALL_HEIGHT` 0.8 vs `CUTAWAY_TALL_WALL_HEIGHT` 2.4) — authored height feeds
that same input, it does not invent a new render path.

## Authoring surface (rides #804's selection)

Select a wall — the selection is a run's edges already — and the Inspector
shows a height stepper alongside the door affordance. Writing it stamps the
value on EVERY edge of the selection (chain-level intent), writes through to
the YAML pane, and previews through the shared engine ("the preview IS the
commit"). Editing interplay, proposed:

- **Endpoint/corner drag** re-derives the chain's edges; the new chain
  inherits the dragged chain's height on every edge, including edges the
  reshape created. Chain-level intent survives reshapes; per-edge survival
  (keep old edges' heights, default the new) is the rejected alternative —
  it turns one drag into a mixed-height chain nobody asked for.
- **Erase** deletes entries regardless of form; **redraw** over an erased
  span writes the CURRENT stroke's height (default unless the author set
  the stepper before drawing).
- **Doors** punch through runs as today; a door in a low wall renders today's
  door in a shorter run. Door height itself is not authored here (shelf).

## The stack

| layer | change |
|---|---|
| **protos** | `AtlasBoundary.height = 5`, one PR, tags |
| **toolkit `dungeonspec`** | parse both wall-entry forms; bounds `[1, 3]` at `walls[i].height` (raise-only); compile through |
| **toolkit `encounter`** | `Boundary`/`BoundaryData` carry `Height float64` (`0` = default); construction + persistence round-trip |
| **toolkit `session`** | projection copies it |
| **rpg-api** | passthrough + pin bumps |
| **web** | engine: height in run identity (chain-break); YAML emit/parse of the object form; Inspector stepper; render multiplier into the existing per-run height input, builder preview AND game route |

## Tests

- dungeonspec: both entry forms parse; bounds refusal at `walls[i].height`
  (below 1 refused — `0.5` is the named refusal case — and above 3);
  bare-pair-only files byte-identical through compile.
- Engine: mixed-height chain splits at the boundary edge; uniform-height
  chain derives today's runs exactly; the #804 seeded-permutation
  order-invariance suite extended with heights (a permuted mixed-height
  document derives identical runs); the 3-way-junction fixture gets a
  differing-height arm.
- Golden: reference tomb byte-identical, plus an explicit all-heights-zero
  assertion (the #261 ledger lesson: goldens are blind to fields their
  comparison struct omits).
- Round-trip: object-form YAML → compile → atlas → wire with the multiplier
  intact at every layer; 2D, 3D, and game route agree on where a chain
  splits (the #804 divergence lesson, now with a height axis).

## Not now

Low walls entirely (raise-only ruling; a future below-standard height must
arrive together with its see/shoot-over Sight ruling); door height; per-face
wall textures; height in the 2D canvas beyond a simple tone/label on the run
(2D stays schematic).

## Plan — four PRs, bottom-up

1. rpg-api-protos: `AtlasBoundary.height`. Merges first, tags.
2. rpg-toolkit: dungeonspec + encounter + session, one chain, real tags
   before the upper pin.
3. rpg-api: pin bump + passthrough.
4. rpg-dnd5e-web: engine chain-break + YAML + stepper + render; Kirk walks
   it — draw a room, raise one wall to double height, see the tall wall in
   2D, 3D, and Save & Play.

## Adjustments (ledger — filled during implementation)

| date | where | designed | landed | why |
|---|---|---|---|---|
| 2026-08-25 | protos | `AtlasBoundary.height = 3` | `height = 5` (rpg-api-protos#254) | the doc said "today bare `{from, to}`" but the walls slice had already landed `blocks_movement = 3` / `blocks_line_of_sight = 4` on the message; the doc was written from a stale shape |
