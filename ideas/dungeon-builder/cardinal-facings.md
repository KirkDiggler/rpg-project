# Cardinal facings and raised offsets — design + plan (rpg-project#272)

Addendum to the dungeon-builder v2 design (`design.md`), panel-back. Amends
`prop-facing-offset.md` (#261): the six-name, orientation-scoped facing set is
superseded. Kirk's ruling (2026-08-25, after the #804 walk):

> "positioning directions not following the hex. I think each position should
> be all 8 positions n, ne, e, se, s ... and should add the height to the
> offset"

Two changes, one dialect:

- **Facing is compass, not hex.** ONE eight-name set — `n ne e se s sw w nw` —
  valid in BOTH orientations. The reason is the walls thread: since #802 walls
  render axis-true, so a statue must be able to stand squarely against a
  horizontal wall in a pointy-top dungeon — and pointy-top's hex edges have no
  `n`. The prop vocabulary sided with the hex shape in #261; Kirk has now
  sided with the walls.
- **Offset gains height.** `offset` grows an optional third component: raise a
  prop off the floor (a torch onto a sconce line, a skull onto an altar).
  Still visual only.

> **Interpretation flag (for Kirk):** "add the height to the offset" is read
> here as the PROP `offset` gaining a vertical component. If you meant
> something else — say, wall height expressed through some offset — reply on
> this PR and this half reshapes; the facings half stands either way.

Law carried forward: **presentation never decides mechanics.** Facing and all
three offset components are render facts; `blocks_*` stays cell-scoped and
nothing here reaches Sight, Standing, or the turn loop.

## The file (dungeonspec v2)

```yaml
place:
  - ref: "dnd5e:props:statue"
    at: [3, 2]
    facing: n                # any of n|ne|e|se|s|sw|w|nw — the SAME eight
                             # names under BOTH orientations. Omitted = the
                             # asset's own default, exactly today.
    offset: [0.2, -0.1, 0.6] # x, y as today (fractions of cell size, each in
                             # [-0.5, 0.5]); OPTIONAL third component = height
                             # above the floor in the same cell-size unit, in
                             # [0, 3]. The two-component form stays valid and
                             # means height 0.
```

Validation (fail-loud, path-addressed, same collecting pass as today):

- `facing` not in the ONE eight-name set → `place[i].facing`. The
  orientation-scoped refusal ("pointy-top has no facing 'n'") is DELETED —
  those exact tests flip from refusal to acceptance.
- `offset` third component outside `[0, 3]`, or a list that is not 2 or 3
  numbers → `place[i].offset`.
- Monster entries still refuse both fields (unchanged; spawn facing remains
  the Monster AI journey's call).

Old files: every previously valid file stays parseable — the old six names
are a subset of the eight and two-component offsets remain legal. Rendered
yaw CHANGES for names whose hex-edge direction differed from true compass
(pointy-top `ne` pointed at the 30° edge; compass `ne` is 45°). Pre-v1, no
backcompat baggage: goldens that pinned the old yaws flip deliberately and
say so.

## Yaw is one table, orientation-independent, and measured

Compass directions live in world space, so the facing→yaw map no longer
depends on hex orientation at all — a simplification: ONE exported
eight-entry table serves both orientations and both the builder preview and
the game route (symmetric-bug rule: the basis lives in one exported place).
Entries are 45° apart *after* the empirical Synty forward-axis offset is
applied — that offset is measured against rendered models exactly as #261's
calibration was (screenshot a visibly asymmetric prop at all eight names,
eyeball that it faces the named compass direction, link the evidence in the
PR). The character-facing lesson stands: measured, not inferred.

## The wire (additive)

| field | change |
|---|---|
| `AtlasProp.facing = 5` | field shape unchanged (authored NAME verbatim); the comment's six-name orientation-scoped contract rewrites to the one eight-name set |
| `AtlasProp.offset_z = 8` | NEW float; height above the floor as a fraction of cell size. Zero = on the floor = not authored — same "said nothing and said zero render identically" contract as offset_x/y |

No angles and no pixels on the wire, as before: the client derives yaw and
world-Y from the authored words/values (ADR-0040 spirit).

## The stack (proto first, then parallel)

| layer | change |
|---|---|
| **protos** | `offset_z = 8` + the facing comment rewrite, one PR, tags |
| **toolkit `dungeonspec`** | validation set swap (8 names, both orientations); 3-component offset parse + bounds |
| **toolkit `encounter`** | `PropInput.Offset`/`Atlas.Props[].Offset`/`PropData.Offset` widen `[2]float64` → `[3]float64` (breaking is free pre-adoption, per no-backcompat) |
| **toolkit `session`** | projection copies the third component |
| **rpg-api** | passthrough + pin bumps |
| **web** | panel: eight fixed compass buttons (the rose does NOT rotate with orientation) + none; ONE exported yaw table; `offset[2] * HEX_SIZE` applied as world-Y in the shared scene path (`atlasToScene3D` → `SceneProp3D`) |

## Tests

- dungeonspec: the orientation-refusal rows flip to acceptance (all 8 names ×
  both orientations); third-component bounds; 2-component form still parses;
  monster refusal unchanged.
- Golden: the reference tomb (no facing/offset anywhere) compiles
  byte-identical, with the explicit zero-value assertion #261's ledger added.
- Web: pixel-formula test on the ONE table (8 names → 8 exact yaws, same both
  orientations); world position test (`SceneProp3D` y == floor +
  offset_z·HEX_SIZE, exact); calibration screenshots at all eight names.
- Session: projection copy test extended to the third component.

## Not now

Free-angle facing (eight is the ruling); monster spawn facing (#201);
wall height (that is #273, `wall-height.md`, its own addendum); props
snapping their height to wall tops or furniture (raised offset is a number,
not an attachment system).

## Plan — four PRs, bottom-up (the #261 shape)

1. rpg-api-protos: `offset_z` + facing comment rewrite. Merges first, tags.
2. rpg-toolkit: dungeonspec + encounter + session, one module chain, real
   tags before the upper pin (the #1228 lesson).
3. rpg-api: pin bump + passthrough.
4. rpg-dnd5e-web: panel, yaw table, world-Y; Kirk walks it — face a statue
   all eight ways against an axis-true wall in a pointy dungeon, raise a
   skull onto an altar, Save & Play, see the same scene in the game.

## Adjustments (ledger — filled during implementation)

| date | where | designed | landed | why |
|---|---|---|---|---|
