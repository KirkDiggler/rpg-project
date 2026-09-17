# Placed footprint geometry — first provider

**Status:** proposed detailed contract, 2026-09-14. Implements the geometry
portion of the [approved architecture](design.md), not the whole editor.
**Owner:** `rpg-toolkit/tools/spatial`. One nearest-`go.mod` implementation PR.
**Plan:** [geometry-plan.md](geometry-plan.md).

## The paying case

A table's editable rectangular footprint can be moved by a fraction of a hex,
rotated freely, and offset relative to the visible assembly's origin. A narrow
barricade needs a segment/footprint query even when it occupies little of any
hex. Neither query should require a snapped authored anchor or load a mesh.

Deliver two geometric answers over the same placed box: cell-area coverage and
segment contact/interior traversal. Do not choose movement, sight, or cover
policy in this provider.

## Existing code to keep

`tools/spatial/coverage.go` already clips hex polygons against a rotated box;
`embedding.go` is the plane. Reuse its clipping and polygon-area code, rather
than introducing a library or a second rasterizer. `Coverage` remains the
cell-anchored compatibility entry point for spells. Keep its public input/output
and the existing valid-input tests.

The inspected spatial tree at toolkit `3968a04c` and current fetched main
`18d5b6e8` is identical: `450b89bd8af9f49055364c5ab38f820f35efef80`.
This observation is not a test result or a release pin.

## Value contract

```go
type FootprintPlacement struct {
    Footprint   Footprint
    Origin      Point
    Facing      float64
    LocalOffset Point
}

type PlacedCoverageInput struct {
    Embedding HexEmbedding
    Placement FootprintPlacement
    Cells     []Position
}

func PlacedCoverage(in PlacedCoverageInput) (CoverageOutput, error)

type FootprintTraceInput struct {
    Placement FootprintPlacement
    From      Point
    To        Point
}

type FootprintTraceOutput struct {
    Contact  bool
    Interior bool
    Enter    float64
    Leave    float64
}

func TraceFootprint(in FootprintTraceInput) (FootprintTraceOutput, error)
```

These are query values, not a durable entity model. `LocalOffset` comes from
the effective authored footprint; putting it in this query does not decide
where YAML stores definition defaults or instance overrides. No entity ID,
property flags, observer state, clock, bus, or persistence belongs here.

### Frame and shape

- All points and lengths use the embedding's caller-chosen unit. Spatial does
  not know feet. Cells are integral axial Q/R, as `HexEmbedding` already uses.
- `Origin` is a continuous point; there is no independent authored cell anchor.
- `Facing` follows the existing numeric embedding convention: local +X maps to
  `(cos(a), sin(a))`, local +Y to `(-sin(a), cos(a))`, `a = degrees * pi/180`.
  The plane's +X is east and +Y is south; positive 90 points south. Correct
  ambiguous clockwise prose without changing existing bearing outputs.
- The existing `Box.D` runs along local X; `Box.W` runs across local Y.
  `LocalOffset` shifts the box centre in those same local axes, before rotation.
- Zero origin, zero angle, and zero local offset are valid. Finite angles outside
  one revolution are accepted. There is no snap, scale, or vertical dimension.
- Only the existing box shape is supported. No polygon field or height simulator
  is added speculatively.

### Coverage

`Cells` is the explicit universe to inspect. It may include any finite integral
axial cells the caller wants; this query does not interpret floor, void, map
membership, or thresholds. Duplicates are idempotent; an empty universe returns
a non-nil empty map after validating the other input. It neither mutates nor
retains the input slice or footprint pointer.

For each supplied cell, use its embedded hex polygon, existing convex clipping,
and existing area epsilon. Return fractions in `(0,1]`, with misses absent.
No nearest-cell inversion or unbounded search is necessary. Work is linear in
the supplied cell count; callers own bounded authoring inputs and later indexes.

### Trace

Trace against the complete placed rectangle, independently of the coverage
universe. This keeps an obstacle outside painted cells available to a sightline
query. Inputs describe a closed segment parameterized as `From + t*(To-From)`.

- `Contact` means the segment meets the closed rectangle.
- `Enter` and `Leave` bound that contact interval in `[0,1]`.
- `Interior` means a positive-length portion of the segment lies strictly inside
  the rectangle. Edge overlap and a corner touch are contact, not interior.
- A miss returns the zero output, with no error.
- A zero-length segment inside or on the box has Contact true, Enter/Leave zero,
  and Interior false. A stationary point outside is a miss.
- Starting or ending inside clips the interval to the relevant endpoint.
- Reversing a nonstationary segment preserves classification and maps the
  interval to `[1-Leave,1-Enter]`, within numerical tolerance.

Compute in the rectangle's local frame using interval/slab clipping. Exposing
contact separately from interior avoids smuggling a tangent-is-blocked decision
into spatial. The encounter's next contract decides which facts its movement
and sight conventions use. This is not a replacement for the room's established
sight/lean-around rules and not a claim that one centre ray implements cover.

### Refusals

- No shape: existing `ErrNoFootprint`.
- Non-finite or nonpositive W/D: existing `ErrBadFootprint`.
- Non-finite origin, facing, local offset, or unrepresentable derived rectangle:
  new `ErrBadFootprintPlacement`.
- Nonpositive/non-finite embedding width: existing `ErrBadCellWidth`. Harden
  `HexEmbeddingConfig.Validate` for NaN and infinities as part of this use case.
- Non-finite or fractional coverage cell (Position has only X/Y): new
  `ErrBadCoverageCell`.
- Non-finite trace endpoints or non-finite intermediate arithmetic:
  new `ErrBadFootprintTrace`.

No partially filled coverage map accompanies an error. No invalid input silently
becomes a miss. This does not widen into unrelated Grid/Room validation changes.

## Legacy adapter

The old `Coverage` converts `At` through `CellCentre`. A centred box has zero
local offset; an edge-anchored box has local X offset `(cellWidth + D)/2`.
It obtains the same bounded grid candidate set as today, then delegates to
PlacedCoverage. Preserve the existing error classifications and shape tests;
new finite-input refusals strengthen invalid-input handling only. Do not change
spell thresholds or caller modules in this PR.

## Acceptance

1. An off-origin, locally offset, 90-degree box has four explicitly calculated
   corners; this catches sign/axis mistakes without a round-trip.
2. A fractional translation changes coverage without snapping. Full candidate
   sets conserve box area under both orientations, within clipping tolerance.
3. Empty/subset/duplicate candidate sets obey the declared universe.
4. A thin box returns a nonempty segment-interior interval even when each covered
   hex fraction is below one half.
5. Crossing, miss, tangent, corner-only, boundary-overlap, contained, endpoint-
   inside, reversed, and stationary segment cases have distinct expected values.
6. Non-finite and fractional inputs fail with the specified error, including
   invalid input with an empty candidate list.
7. The existing `CoverageSuite` and `EmbeddingSuite` pass unchanged except for
   additional tests. The full spatial module passes tests/race/lint and hooks.

This provider is done when its geometry works and is independently reviewed.
Multi-hex live occupancy, YAML, preview transport, save/play, and half-cover
rules are not delivered by that result. They remain required consumer work in
[the architecture's Proof A and Proof B](design.md#7-small-proofs-and-failure-behavior).
