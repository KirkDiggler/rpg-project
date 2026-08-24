# Prop facing and offset — design + plan (rpg-project#261)

Addendum to the dungeon-builder v2 design (`design.md`), panel-back. Supersedes
the dialect of pre-restart #178. Both rulings are in (issue #261, 2026-08-24):

- **Facing (Kirk):** "straight hex facing works with flat edges but we will need
  adjustments if it is pointy" → six hex facings, snapped to the hex's edge
  directions, **orientation-aware**. The vocabulary and the rendered yaw both
  derive from the dungeon's authored `orientation`.
- **Offset (Kirk):** "offset is visual only, agreed" → a within-cell visual
  nudge; the prop still occupies its whole cell for movement and LOS.

Law carried forward: **presentation never decides mechanics.** `blocks_*` stays
cell-scoped; nothing here reaches Sight, Standing, or the turn loop.

## The panel

Selecting a placed prop in the builder shows, alongside today's `blocks_*`:

- **Facing** — six direction buttons, drawn rotated to match the dungeon's own
  orientation (flat-top shows N/S/NE/NW/SE/SW; pointy-top shows
  E/W/NE/NW/SE/SW). One active at a time; a "none" state means the asset's
  own authored default, which is exactly today's rendering.
- **Offset** — a small 2D nudge pad (or X/Y steppers) bounded to keep the model
  visually inside its cell. Zero by default.

Both write through to the YAML pane immediately; both render live in the 2D
canvas (facing as a small direction tick on the placement marker; offset moves
the marker within the hex) and in the 3D preview through the game's own
renderer.

## The file (dungeonspec v2, additive)

```yaml
place:
  - ref: "dnd5e:props:brazier"
    at: [1, 1]
    blocks_movement: true
    blocks_los: false
    facing: ne          # optional; one of the SIX names valid under this
                        # dungeon's orientation — flat: n|s|ne|nw|se|sw,
                        # pointy: e|w|ne|nw|se|sw. Omitted = asset default.
    offset: [0.2, -0.1] # optional; fractions of the cell size, each in
                        # [-0.5, 0.5]. Omitted = centered. Visual only.
```

Validation (fail-loud, path-addressed, in `Validate`'s one collecting pass):

- `facing` not in the orientation's own six-name set → `place[i].facing`
  ("pointy-top has no facing 'n'"). The wrong-orientation name is an ERROR,
  never a silent snap to the nearest valid one.
- `offset` component outside `[-0.5, 0.5]`, or not a two-number list →
  `place[i].offset`.
- Both REFUSED on monster entries for now (same shape as `blocks_*` being
  refused off props): monsters face dynamically in play; authored spawn facing
  is a shelf item for the Monster AI journey, not smuggled in here.

Old files stay valid: both fields optional, absent means today's behavior.

## The stack (proto first, then parallel)

| layer | today | change |
|---|---|---|
| **protos** | `AtlasProp{ref, at, blocks_movement, blocks_line_of_sight}` | additive: `string facing = 5` (empty = none), `float offset_x = 6`, `float offset_y = 7`. Wire carries the AUTHORED words/values verbatim — no angles on the wire; angle math is a render concern (ADR-0040 spirit: the wire names facts, the client derives pixels) |
| **toolkit `dungeonspec`** | `place[]` prop entries → `PropInput` | parse + validate the two fields per the rules above; compile through |
| **toolkit `encounter`** | `PropInput{Ref, At, BlocksMovement, BlocksLOS}`; `Atlas.Props` | `Facing string` (authored name, `""` = none) and `Offset [2]float64` (`{0,0}` = none) on both; no pointer ceremony — these are optional presentational facts, "said nothing" and "said zero/center" render identically by design |
| **toolkit `session`** | projection copies props | copies the two new fields; `EncounterData` persists them |
| **rpg-api** | registry compiles + projects | passthrough only; pin bumps |
| **web** | Inspector, YAML round-trip, `PropModel` render | the panel above; YAML emit/parse; ONE exported facing→yaw map per orientation used by BOTH the builder preview and the game route (symmetric-bug rule: the basis lives in one exported place); `offset * HEX_SIZE` applied in the shared scene path (`atlasToScene3D` → `SceneProp3D`) |

## Angle math is measured, not inferred (the character-facing lesson)

The facing→yaw map is calibrated against rendered models, not derived on paper:
render a visually asymmetric prop (statue) at each of the six facings in both
orientations, screenshot, and eyeball that the model faces the named neighbor
edge. The map is a constant table in one exported place with the calibration
evidence linked in the PR. Which local axis is "forward" for Synty props is an
empirical fact of the assets, not an assumption.

## Tests

- dungeonspec: table-driven validation — every wrong-orientation name refused
  with `place[i].facing`; offset bounds; monster-entry refusal; old files
  (no fields) unchanged.
- Golden: the reference tomb (no facing/offset anywhere) compiles to a
  byte-identical atlas — the additive fields change nothing absent.
- New fixture: a dungeon with faced+offset props round-trips YAML → compile →
  atlas → (api) → wire with the authored words/values intact at every layer.
- Web: pixel-formula test on the facing→yaw table (six names → six yaws per
  orientation, exact values); offset applied in world space
  (`SceneProp3D` position == cell center + offset·HEX_SIZE, exact).
- Session: projection copy test extended to the two fields.

## Not now

Monster spawn facing (Monster AI journey #201's call); mount points; prop
catalog validation (#185); free-angle facing (six is the ruling; revisit only
with a new ruling).

## Adjustments (ledger — filled during implementation)

| date | where | designed | landed | why |
|---|---|---|---|---|
| 2026-08-24 | toolkit#1227 | stack table named `PropInput`/`Atlas.Props` only | `PropData` (encounter's own construction-time persistence, `FieldData` mirror) also carries Facing/Offset, wired through `ToData`/`LoadFromData` | without it an authored facing/offset silently vanishes on save/reload of a running encounter; `data.go`'s own contract says FieldData mirrors FieldInput exactly |
| 2026-08-24 | toolkit#1227 | golden = tomb byte-identical | plus an explicit assertion that every tomb prop's Facing/Offset is the zero value | the golden comparison struct doesn't include the new fields — without the added assertion it would be silently blind to them |

## Sequencing

1. rpg-api-protos PR (additive fields) — merges first, tags.
2. toolkit PR (dungeonspec + encounter + session, one module chain).
3. rpg-api pin bump + passthrough PR.
4. web PR (panel, YAML, calibrated render) — Kirk walks the builder once:
   place a statue, face it six ways, nudge it, Save & Play, see the same
   statue the same way in the game.
5. Bottom-up merges with real tags, per how-we-build.
