# Placed Footprint Geometry Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Query cell coverage and segment contact for an explicitly positioned,
rotated rectangular footprint without snapping its origin to a hex.

**Architecture:** Add a common placed-box representation beneath two pure spatial
queries. Reuse existing hex embedding/clipping, and retain cell-anchored Coverage
as the spell-facing adapter. No game rules or entity occupancy are added here.

**Tech Stack:** Go, existing `tools/spatial` module, standard math/errors,
Testify suites. No new dependency.

**Spec:** [geometry.md](geometry.md), implementing the first provider for
[World Builder dungeon authoring](design.md).

## Global Constraints

- One implementation PR in `rpg-toolkit/tools/spatial`, its nearest Go module.
- "Spatial does not know feet." No movement/sight/cover property flags or D&D
  modifiers in these query inputs/outputs.
- "Cells is the explicit universe to inspect." No unbounded scan or silently
  inferred playable-floor universe.
- "A miss returns the zero output, with no error." Invalid inputs return errors,
  not misses; coverage errors return no partial map.
- "No entity ID, property flags, observer state, clock, bus, or persistence belongs here."
- "No nearest-cell inversion or unbounded search is necessary."
- "Positive 90 points south" in the existing embedding's X-east/Y-south frame.
  Preserve numeric bearing behavior; do not copy ambiguous clockwise prose.
- "The existing Box.D runs along local X; Box.W runs across local Y."
- "The current editor still has no scale or tilt tool." Do not add either here.
- Preserve existing valid `Coverage` behavior and all current call signatures.
- Existing root/scoped AGENTS/CLAUDE instructions win on commands and release.
  Read `tools/spatial/{CLAUDE.md,doc.go}` and the README geometry section first.
- Parent creates/assigns the toolkit issue and Project 19 entry before an
  implementation branch. Use an isolated `.worktrees/` tree, not scout snapshots.
- TDD red results stay uncommitted. Green focused checkpoints use normal hooks;
  never bypass hooks or commit dependency overrides.
- Follow the current operator's serial release rule: review this provider, obtain
  merge authorization, verify its CI-issued tag, then prepare its consumer.
  No hand tags, speculative version number, parallel dependent PR stack, or
  changes to existing playtest stacks.

## Scope and evidence baseline

Architecture is approved; this detailed API/plan is the next review surface.
This is not the implementation plan for the entire editor or for half cover.
No test in the snippets below has been run as part of writing this plan.

Spatial subtree `450b89bd8af9f49055364c5ab38f820f35efef80` was equal at toolkit
`3968a04c` and fetched main `18d5b6e8`. At implementation preflight, compare the
current subtree and re-read any changed files before applying the plan.

The run root for every command below is the implementation worktree's
`tools/spatial` directory unless a command explicitly begins at repository root.

## File map

| File | Responsibility |
|---|---|
| Create `tools/spatial/placed_footprint.go` | Query placement value; validated common box frame/polygon |
| Create `tools/spatial/placed_footprint_test.go` | Independent position/angle/offset/invalid-input proofs |
| Create `tools/spatial/placed_coverage.go` | Explicit-cell-universe coverage over the shared polygon |
| Create `tools/spatial/placed_coverage_test.go` | Fractions, conservation, universe, finite-input tests |
| Create `tools/spatial/footprint_trace.go` | Closed-segment contact interval and interior distinction |
| Create `tools/spatial/footprint_trace_test.go` | Cross/miss/tangent/contained/reverse/stationary proofs |
| Create `tools/spatial/placed_footprint_example_test.go` | Compilable external-package API example |
| Modify `tools/spatial/coverage.go` | Existing Coverage adapts its anchor into the common placement |
| Modify `tools/spatial/coverage_test.go` | Adapter equivalence and existing spell-shape regressions |
| Modify `tools/spatial/embedding.go` | Reject non-finite width; clarify numeric bearing documentation |
| Modify `tools/spatial/embedding_test.go` | NaN/infinity width refusal and positive-90 bearing proof |
| Modify `tools/spatial/{doc.go,README.md}` | Public query contracts, examples, explicit non-goals |

Do not change `Box`, `Footprint`, or their JSON layout merely to add the query
placement. `FootprintPlacement.LocalOffset` supplies the effective definition's
local offset without deciding durable authoring representation.

---

## Task 1: One validated placed rectangle

**Files:** `placed_footprint.go`, `placed_footprint_test.go`.

**Consumes:** existing `Footprint`, `Box`, and `Point` in this module.
**Produces:** `FootprintPlacement`; internal
`footprintBox(in FootprintPlacement) (placedBox, error)` and copied frame values
used by Tasks 2–3. `placedBox` owns no pointers/slices from its caller.

- [ ] **Step 1: Add the geometric discriminator and refusal tests.**

```go
package spatial

import (
    "math"
    "testing"

    "github.com/stretchr/testify/suite"
)

type PlacedFootprintSuite struct { suite.Suite }

func TestPlacedFootprintSuite(t *testing.T) {
    suite.Run(t, new(PlacedFootprintSuite))
}

func (s *PlacedFootprintSuite) TestOffsetRotatesBeforeTranslation() {
    frame, err := footprintBox(FootprintPlacement{
        Footprint: Footprint{Box: &Box{W: 2, D: 4}},
        Origin: Point{X: 3.25, Y: -1.75}, Facing: 90,
        LocalOffset: Point{X: 1, Y: 0.5},
    })
    s.Require().NoError(err)
    want := [4]Point{{X: 3.75, Y: -2.75}, {X: 3.75, Y: 1.25},
        {X: 1.75, Y: 1.25}, {X: 1.75, Y: -2.75}}
    for i, p := range want {
        s.InDelta(p.X, frame.corners[i].X, 1e-12)
        s.InDelta(p.Y, frame.corners[i].Y, 1e-12)
    }
}

func (s *PlacedFootprintSuite) TestInvalidPlacements() {
    cases := []struct {
        name string
        in FootprintPlacement
        want error
    }{
        {"no shape", FootprintPlacement{}, ErrNoFootprint},
        {"zero width", FootprintPlacement{Footprint: Footprint{Box: &Box{W: 0, D: 2}}}, ErrBadFootprint},
        {"negative depth", FootprintPlacement{Footprint: Footprint{Box: &Box{W: 1, D: -2}}}, ErrBadFootprint},
        {"NaN width", FootprintPlacement{Footprint: Footprint{Box: &Box{W: math.NaN(), D: 2}}}, ErrBadFootprint},
        {"infinite depth", FootprintPlacement{Footprint: Footprint{Box: &Box{W: 1, D: math.Inf(1)}}}, ErrBadFootprint},
        {"NaN origin", FootprintPlacement{Footprint: Footprint{Box: &Box{W: 1, D: 2}}, Origin: Point{X: math.NaN()}}, ErrBadFootprintPlacement},
        {"infinite angle", FootprintPlacement{Footprint: Footprint{Box: &Box{W: 1, D: 2}}, Facing: math.Inf(-1)}, ErrBadFootprintPlacement},
        {"infinite offset", FootprintPlacement{Footprint: Footprint{Box: &Box{W: 1, D: 2}}, LocalOffset: Point{Y: math.Inf(1)}}, ErrBadFootprintPlacement},
    }
    for _, tc := range cases {
        s.Run(tc.name, func() {
            _, err := footprintBox(tc.in)
            s.ErrorIs(err, tc.want)
        })
    }
}
```

- [ ] **Step 2: Run the red tests.**

```bash
go test ./... -run '^TestPlacedFootprintSuite$' -count=1
```

Expected: undefined `FootprintPlacement` / `footprintBox`; no commit yet.

- [ ] **Step 3: Implement the common frame in `placed_footprint.go`.**

```go
// FootprintPlacement positions a footprint in the caller's continuous plane.
// LocalOffset is in the footprint's own along/across axes, before Facing.
type FootprintPlacement struct {
    Footprint Footprint
    Origin Point
    Facing float64
    LocalOffset Point
}

// ErrBadFootprintPlacement reports non-finite or unrepresentable placed geometry.
var ErrBadFootprintPlacement = errors.New("spatial: invalid footprint placement")

type placedBox struct {
    centre Point
    along Point
    across Point
    halfDepth float64
    halfWidth float64
    corners [4]Point
}

func footprintBox(in FootprintPlacement) (placedBox, error) {
    if in.Footprint.Box == nil {
        return placedBox{}, ErrNoFootprint
    }
    b := *in.Footprint.Box
    for _, v := range []float64{b.W, b.D} {
        if v <= 0 || math.IsNaN(v) || math.IsInf(v, 0) {
            return placedBox{}, ErrBadFootprint
        }
    }
    for _, v := range []float64{in.Origin.X, in.Origin.Y, in.Facing,
        in.LocalOffset.X, in.LocalOffset.Y} {
        if math.IsNaN(v) || math.IsInf(v, 0) {
            return placedBox{}, ErrBadFootprintPlacement
        }
    }
    a := math.Mod(in.Facing, 360) * math.Pi / 180
    f := placedBox{
        along: Point{X: math.Cos(a), Y: math.Sin(a)},
        across: Point{X: -math.Sin(a), Y: math.Cos(a)},
        halfDepth: b.D / 2, halfWidth: b.W / 2,
    }
    f.centre = Point{
        X: in.Origin.X + f.along.X*in.LocalOffset.X + f.across.X*in.LocalOffset.Y,
        Y: in.Origin.Y + f.along.Y*in.LocalOffset.X + f.across.Y*in.LocalOffset.Y,
    }
    local := [4]Point{
        {X: -f.halfDepth, Y: -f.halfWidth},
        {X: f.halfDepth, Y: -f.halfWidth},
        {X: f.halfDepth, Y: f.halfWidth},
        {X: -f.halfDepth, Y: f.halfWidth},
    }
    for i, p := range local {
        f.corners[i] = Point{
            X: f.centre.X + f.along.X*p.X + f.across.X*p.Y,
            Y: f.centre.Y + f.along.Y*p.X + f.across.Y*p.Y,
        }
        for _, v := range []float64{f.corners[i].X, f.corners[i].Y} {
            if math.IsNaN(v) || math.IsInf(v, 0) {
                return placedBox{}, ErrBadFootprintPlacement
            }
        }
    }
    area := polygonArea(f.corners[:])
    if area <= 0 || math.IsNaN(area) || math.IsInf(area, 0) {
        return placedBox{}, ErrBadFootprintPlacement
    }
    return f, nil
}
```

Import standard `errors` and `math`. The area check refuses arithmetic collapse;
it is not a promise of arbitrary-magnitude precision. Keep existing
`polygonArea` as the shared helper.

- [ ] **Step 4: Add angle-wrap and alias discriminators, then run green.**

```go
func (s *PlacedFootprintSuite) TestEquivalentAnglesAndCopiedDimensions() {
    box := &Box{W: 2, D: 4}
    in := FootprintPlacement{Footprint: Footprint{Box: box}, Facing: -270}
    before, err := footprintBox(in)
    s.Require().NoError(err)
    in.Facing = 450
    after, err := footprintBox(in)
    s.Require().NoError(err)
    for i := range before.corners {
        s.InDelta(before.corners[i].X, after.corners[i].X, 1e-12)
        s.InDelta(before.corners[i].Y, after.corners[i].Y, 1e-12)
    }
    box.W = 100
    s.Equal(1.0, before.halfWidth)
}
```

```bash
go test ./... -run '^TestPlacedFootprintSuite$' -count=1
gofmt -w placed_footprint.go placed_footprint_test.go
git add placed_footprint.go placed_footprint_test.go
git commit -m 'feat(spatial): represent freely placed rectangular footprints'
```

Normal hooks run the module's required checks. Fix failures before continuing.

## Task 2: Coverage over an explicit cell universe

**Files:** `placed_coverage.go`, `placed_coverage_test.go`,
`embedding.go`, `embedding_test.go`.

**Consumes:** Task 1's `FootprintPlacement` / `footprintBox`; existing
`HexEmbedding`, `CoverageOutput`, `clipConvex`, `polygonArea`, `coverageEpsilon`.
**Produces:** `PlacedCoverage(PlacedCoverageInput) (CoverageOutput, error)`.

- [ ] **Step 1: Add focused coverage and width-refusal tests.**

Create the suite below. `sumFractions` and `keys` already exist in the unchanged
`coverage_test.go`; they are test helpers, not new production APIs.

```go
package spatial

import (
    "math"
    "testing"

    "github.com/stretchr/testify/suite"
)

type PlacedCoverageSuite struct { suite.Suite }

func TestPlacedCoverageSuite(t *testing.T) {
    suite.Run(t, new(PlacedCoverageSuite))
}

func (s *PlacedCoverageSuite) TestFractionalPlacementAndArea() {
    for _, orientation := range []HexOrientation{HexOrientationPointyTop, HexOrientationFlatTop} {
        emb := NewHexEmbedding(HexEmbeddingConfig{Orientation: orientation, CellWidth: 5})
        g := NewAxialHexGrid(AxialHexGridConfig{SpanWidth: 15, SpanHeight: 15})
        in := PlacedCoverageInput{
            Embedding: emb,
            Placement: FootprintPlacement{Footprint: Footprint{Box: &Box{W: 2, D: 4}},
                Origin: Point{X: 3.25, Y: -1.75}, Facing: 37,
                LocalOffset: Point{X: 1, Y: 0.5}},
            Cells: g.GetPositionsInRange(Position{}, 7),
        }
        first, err := PlacedCoverage(in)
        s.Require().NoError(err)
        s.InDelta(8, sumFractions(first.Cells)*25*math.Sqrt(3)/2, 1e-8)
        in.Placement.Origin.X += 0.35
        second, err := PlacedCoverage(in)
        s.Require().NoError(err)
        s.NotEqual(first.Cells, second.Cells, "fractional movement must not snap")
        s.InDelta(8, sumFractions(second.Cells)*25*math.Sqrt(3)/2, 1e-8)
    }
}

func (s *PlacedCoverageSuite) TestUniverseAndInvalidEmptyInput() {
    emb := NewHexEmbedding(HexEmbeddingConfig{CellWidth: 5})
    in := PlacedCoverageInput{Embedding: emb,
        Placement: FootprintPlacement{Footprint: Footprint{Box: &Box{W: 1, D: 1}}}}
    out, err := PlacedCoverage(in)
    s.Require().NoError(err)
    s.NotNil(out.Cells)
    s.Empty(out.Cells)
    in.Cells = []Position{{}, {}}
    out, err = PlacedCoverage(in)
    s.Require().NoError(err)
    s.Len(out.Cells, 1)
    s.InDelta(1, out.Cells[Position{}]*25*math.Sqrt(3)/2, 1e-9)
    in.Cells = []Position{{X: 0.5}}
    out, err = PlacedCoverage(in)
    s.ErrorIs(err, ErrBadCoverageCell)
    s.Nil(out.Cells)
    in.Cells = nil
    in.Placement.Facing = math.NaN()
    out, err = PlacedCoverage(in)
    s.ErrorIs(err, ErrBadFootprintPlacement)
    s.Nil(out.Cells)
}
```

In `EmbeddingTestSuite`, add:

```go
func (s *EmbeddingTestSuite) TestNonFiniteWidthsAndNumericBearing() {
    for _, width := range []float64{math.NaN(), math.Inf(1), math.Inf(-1)} {
        s.ErrorIs(HexEmbeddingConfig{CellWidth: width}.Validate(), ErrBadCellWidth)
        emb := NewHexEmbedding(HexEmbeddingConfig{CellWidth: width})
        s.Equal(Point{}, emb.CellCentre(Position{X: 1}))
    }
    emb := NewHexEmbedding(HexEmbeddingConfig{Orientation: HexOrientationFlatTop, CellWidth: 5})
    angle, ok := emb.Bearing(Position{}, Position{Y: 1})
    s.True(ok)
    s.InDelta(90, angle, 1e-12)
}
```

- [ ] **Step 2: Observe the failures.**

```bash
go test ./... -run '^(TestPlacedCoverageSuite|TestEmbeddingSuite)$' -count=1
```

Expected: the new query is undefined and current width validation accepts
non-finite values. Separate those causes; do not mislabel setup failures.

- [ ] **Step 3: Add the explicit-universe rasterizer.**

```go
// PlacedCoverageInput declares the exact axial cells to inspect in an embedding.
type PlacedCoverageInput struct {
    Embedding HexEmbedding
    Placement FootprintPlacement
    Cells []Position
}

// ErrBadCoverageCell reports a non-finite, fractional, or unrepresentable cell.
var ErrBadCoverageCell = errors.New("spatial: invalid coverage cell")

// PlacedCoverage returns area fractions for a freely placed footprint over Cells.
// Misses are absent. Inputs are not retained; errors return no partial map.
func PlacedCoverage(in PlacedCoverageInput) (CoverageOutput, error) {
    f, err := footprintBox(in.Placement)
    if err != nil {
        return CoverageOutput{}, err
    }
    if err := (HexEmbeddingConfig{CellWidth: in.Embedding.cellWidth}).Validate(); err != nil {
        return CoverageOutput{}, err
    }
    out := CoverageOutput{Cells: make(map[Position]float64)}
    for _, cell := range in.Cells {
        for _, v := range []float64{cell.X, cell.Y} {
            if math.IsNaN(v) || math.IsInf(v, 0) || math.Trunc(v) != v {
                return CoverageOutput{}, ErrBadCoverageCell
            }
        }
        hex := in.Embedding.CellCorners(cell)
        for _, p := range hex {
            if math.IsNaN(p.X) || math.IsNaN(p.Y) || math.IsInf(p.X, 0) || math.IsInf(p.Y, 0) {
                return CoverageOutput{}, ErrBadCoverageCell
            }
        }
        whole := polygonArea(hex[:])
        if whole <= 0 || math.IsNaN(whole) || math.IsInf(whole, 0) {
            return CoverageOutput{}, ErrBadCoverageCell
        }
        clipped := clipConvex(hex[:], f.corners[:])
        area := polygonArea(clipped)
        fraction := area / whole
        if math.IsNaN(fraction) || math.IsInf(fraction, 0) {
            return CoverageOutput{}, ErrBadFootprintPlacement
        }
        if fraction > coverageEpsilon {
            out.Cells[cell] = math.Min(fraction, 1)
        }
    }
    return out, nil
}
```

Import `errors` and `math`. Extend width validation's existing condition to
`c.CellWidth <= 0 || math.IsNaN(c.CellWidth) || math.IsInf(c.CellWidth, 0)`.
Document that inert embeddings follow invalid widths. Clarify Bearing's
positive-angle direction without changing the numeric calculation.

- [ ] **Step 4: Prove subset/NaN/overflow refusal and input preservation.**

```go
func (s *PlacedCoverageSuite) TestSubsetAndNoPartialResult() {
    emb := NewHexEmbedding(HexEmbeddingConfig{CellWidth: 5})
    cells := []Position{{}, {X: 1}, {X: math.NaN()}}
    in := PlacedCoverageInput{Embedding: emb,
        Placement: FootprintPlacement{Footprint: Footprint{Box: &Box{W: 12, D: 12}}},
        Cells: cells[:2]}
    out, err := PlacedCoverage(in)
    s.Require().NoError(err)
    s.Len(out.Cells, 2)
    s.Equal(Position{X: 1}, cells[1])
    in.Cells = cells
    out, err = PlacedCoverage(in)
    s.ErrorIs(err, ErrBadCoverageCell)
    s.Nil(out.Cells)
    in.Cells = []Position{{X: math.MaxFloat64}}
    out, err = PlacedCoverage(in)
    s.ErrorIs(err, ErrBadCoverageCell)
    s.Nil(out.Cells)
}
```

- [ ] **Step 5: Run green, format, and commit.**

```bash
go test ./... -run '^(TestPlacedCoverageSuite|TestEmbeddingSuite)$' -count=1
gofmt -w placed_coverage.go placed_coverage_test.go embedding.go embedding_test.go
git add placed_coverage.go placed_coverage_test.go embedding.go embedding_test.go
git commit -m 'feat(spatial): rasterize free footprints over declared cells'
```

## Task 3: Thin footprints answer segment queries

**Files:** `footprint_trace.go`, `footprint_trace_test.go`.
**Consumes:** Task 1's frame; no Grid/Room/coverage threshold.
**Produces:** `TraceFootprint(FootprintTraceInput) (FootprintTraceOutput, error)`.

- [ ] **Step 1: Add a trace suite and the full interval truth table.**

```go
package spatial

import (
    "math"
    "testing"

    "github.com/stretchr/testify/suite"
)

type FootprintTraceSuite struct { suite.Suite }

func TestFootprintTraceSuite(t *testing.T) {
    suite.Run(t, new(FootprintTraceSuite))
}

func (s *FootprintTraceSuite) TestContactIntervals() {
    cases := []struct {
        name string
        from, to Point
        contact, interior bool
        enter, leave float64
    }{
        {"cross", Point{X: -4}, Point{X: 4}, true, true, .25, .75},
        {"miss", Point{X: -4, Y: 2}, Point{X: 4, Y: 2}, false, false, 0, 0},
        {"edge overlap", Point{X: -4, Y: 1}, Point{X: 4, Y: 1}, true, false, .25, .75},
        {"corner", Point{X: -4, Y: -1}, Point{Y: 3}, true, false, .5, .5},
        {"start inside", Point{}, Point{X: 4}, true, true, 0, .5},
        {"contained", Point{X: -1}, Point{X: 1}, true, true, 0, 1},
        {"stationary inside", Point{}, Point{}, true, false, 0, 0},
        {"stationary edge", Point{X: 2}, Point{X: 2}, true, false, 0, 0},
        {"stationary outside", Point{X: 3}, Point{X: 3}, false, false, 0, 0},
    }
    for _, tc := range cases {
        s.Run(tc.name, func() {
            in := FootprintTraceInput{Placement: FootprintPlacement{
                Footprint: Footprint{Box: &Box{W: 2, D: 4}}}, From: tc.from, To: tc.to}
            out, err := TraceFootprint(in)
            s.Require().NoError(err)
            s.Equal(tc.contact, out.Contact)
            s.Equal(tc.interior, out.Interior)
            s.InDelta(tc.enter, out.Enter, 1e-12)
            s.InDelta(tc.leave, out.Leave, 1e-12)
            in.From, in.To = in.To, in.From
            reverse, err := TraceFootprint(in)
            s.Require().NoError(err)
            s.Equal(out.Contact, reverse.Contact)
            s.Equal(out.Interior, reverse.Interior)
            if out.Contact && tc.from != tc.to {
                s.InDelta(1-out.Leave, reverse.Enter, 1e-12)
                s.InDelta(1-out.Enter, reverse.Leave, 1e-12)
            }
        })
    }
}
```

- [ ] **Step 2: Run red.**

```bash
go test ./... -run '^TestFootprintTraceSuite$' -count=1
```

Expected: undefined trace query/types.

- [ ] **Step 3: Implement interval clipping in the shared frame.**

```go
// FootprintTraceInput describes a closed planar segment and one placed footprint.
type FootprintTraceInput struct {
    Placement FootprintPlacement
    From Point
    To Point
}

// FootprintTraceOutput distinguishes closed contact from positive-length interior.
// Enter and Leave parameterize the contact interval; a miss is the zero value.
type FootprintTraceOutput struct {
    Contact bool
    Interior bool
    Enter float64
    Leave float64
}

// ErrBadFootprintTrace reports non-finite endpoints or unrepresentable arithmetic.
var ErrBadFootprintTrace = errors.New("spatial: invalid footprint trace")

// TraceFootprint reports contact with the full rectangle, independent of any grid.
// It does not decide whether contact blocks movement, sight, or an attack.
func TraceFootprint(in FootprintTraceInput) (FootprintTraceOutput, error) {
    f, err := footprintBox(in.Placement)
    if err != nil {
        return FootprintTraceOutput{}, err
    }
    for _, v := range []float64{in.From.X, in.From.Y, in.To.X, in.To.Y} {
        if math.IsNaN(v) || math.IsInf(v, 0) {
            return FootprintTraceOutput{}, ErrBadFootprintTrace
        }
    }
    var ends [2]Point
    for i, p := range [2]Point{in.From, in.To} {
        dx, dy := p.X-f.centre.X, p.Y-f.centre.Y
        ends[i] = Point{X: dx*f.along.X + dy*f.along.Y,
            Y: dx*f.across.X + dy*f.across.Y}
    }
    delta := Point{X: ends[1].X-ends[0].X, Y: ends[1].Y-ends[0].Y}
    for _, v := range []float64{ends[0].X, ends[0].Y, ends[1].X, ends[1].Y, delta.X, delta.Y} {
        if math.IsNaN(v) || math.IsInf(v, 0) {
            return FootprintTraceOutput{}, ErrBadFootprintTrace
        }
    }
    if in.From == in.To {
        contact := math.Abs(ends[0].X) <= f.halfDepth && math.Abs(ends[0].Y) <= f.halfWidth
        return FootprintTraceOutput{Contact: contact}, nil
    }
    enter, leave := 0.0, 1.0
    axes := [2][3]float64{
        {ends[0].X, delta.X, f.halfDepth},
        {ends[0].Y, delta.Y, f.halfWidth},
    }
    for _, axis := range axes {
        start, change, half := axis[0], axis[1], axis[2]
        if change == 0 {
            if start < -half || start > half {
                return FootprintTraceOutput{}, nil
            }
            continue
        }
        a, b := (-half-start)/change, (half-start)/change
        if math.IsNaN(a) || math.IsNaN(b) || math.IsInf(a, 0) || math.IsInf(b, 0) {
            return FootprintTraceOutput{}, ErrBadFootprintTrace
        }
        if a > b { a, b = b, a }
        enter, leave = math.Max(enter, a), math.Min(leave, b)
        if enter > leave {
            return FootprintTraceOutput{}, nil
        }
    }
    mid := (enter+leave)/2
    x, y := ends[0].X+mid*delta.X, ends[0].Y+mid*delta.Y
    interior := enter < leave && math.Abs(x) < f.halfDepth && math.Abs(y) < f.halfWidth
    return FootprintTraceOutput{Contact: true, Interior: interior, Enter: enter, Leave: leave}, nil
}
```

Import `errors` and `math`. Exact tangent classifications above use ordinary
representable cases. Do not add a game-size epsilon or convert touches into
interior traversal. If a numerical improvement is needed, preserve this
contract and add its geometric discriminator instead of changing policy.

- [ ] **Step 4: Prove rotation, thinness, and endpoint refusals.**

```go
func (s *FootprintTraceSuite) TestRotatedOffsetUsesSameFrame() {
    out, err := TraceFootprint(FootprintTraceInput{
        Placement: FootprintPlacement{Footprint: Footprint{Box: &Box{W: 2, D: 4}},
            Origin: Point{X: 3.25, Y: -1.75}, Facing: 90,
            LocalOffset: Point{X: 1, Y: .5}},
        From: Point{X: 2.75, Y: -4.75}, To: Point{X: 2.75, Y: 3.25},
    })
    s.Require().NoError(err)
    s.True(out.Interior)
    s.InDelta(.25, out.Enter, 1e-12)
    s.InDelta(.75, out.Leave, 1e-12)
}

func (s *FootprintTraceSuite) TestThinBoxAndInvalidEndpoints() {
    placement := FootprintPlacement{Footprint: Footprint{Box: &Box{W: 4, D: .2}}}
    emb := NewHexEmbedding(HexEmbeddingConfig{CellWidth: 5})
    g := NewAxialHexGrid(AxialHexGridConfig{SpanWidth: 9, SpanHeight: 9})
    covered, err := PlacedCoverage(PlacedCoverageInput{Embedding: emb,
        Placement: placement, Cells: g.GetPositionsInRange(Position{}, 4)})
    s.Require().NoError(err)
    s.NotEmpty(covered.Cells)
    for _, fraction := range covered.Cells { s.Less(fraction, .5) }
    in := FootprintTraceInput{Placement: placement, From: Point{X: -2}, To: Point{X: 2}}
    trace, err := TraceFootprint(in)
    s.Require().NoError(err)
    s.True(trace.Interior)
    s.InDelta(.475, trace.Enter, 1e-12)
    s.InDelta(.525, trace.Leave, 1e-12)
    for _, bad := range []float64{math.NaN(), math.Inf(1), math.Inf(-1)} {
        in.To.Y = bad
        _, err = TraceFootprint(in)
        s.ErrorIs(err, ErrBadFootprintTrace)
    }
}
```

- [ ] **Step 5: Run green and commit.**

```bash
go test ./... -run '^TestFootprintTraceSuite$' -count=1
gofmt -w footprint_trace.go footprint_trace_test.go
git add footprint_trace.go footprint_trace_test.go
git commit -m 'feat(spatial): trace segments through placed footprints'
```

## Task 4: Reuse the provider from existing Coverage and deliver it

**Files:** `coverage.go`, `coverage_test.go`, `doc.go`, `README.md`.
**Consumes:** Tasks 1–3. **Produces:** unchanged public
`Coverage(HexEmbedding, Grid, CoverageInput) (CoverageOutput, error)` backed by
the new provider; documented public query contract.

- [ ] **Step 1: Add the adapter-equivalence proof before replacing its body.**

```go
func (s *CoverageTestSuite) TestCellAnchorAdapterMatchesContinuousQuery() {
    for _, anchor := range []AnchorRule{AnchorAtCentre, AnchorAtEdge} {
        for _, angle := range []float64{0, 15, 60, 90, 137} {
            in := CoverageInput{Footprint: Footprint{Box: &Box{W: 15, D: 15}},
                At: Position{X: 1, Y: -1}, Facing: angle, Anchor: anchor}
            old, err := Coverage(s.emb, s.grid, in)
            s.Require().NoError(err)
            placement := FootprintPlacement{Footprint: in.Footprint,
                Origin: s.emb.CellCentre(in.At), Facing: in.Facing}
            if anchor == AnchorAtEdge { placement.LocalOffset.X = 10 }
            direct, err := PlacedCoverage(PlacedCoverageInput{Embedding: s.emb,
                Placement: placement, Cells: s.grid.GetPositionsInRange(in.At, 6)})
            s.Require().NoError(err)
            s.Equal(keys(old.Cells), keys(direct.Cells))
            for cell, fraction := range old.Cells {
                s.InDelta(fraction, direct.Cells[cell], 1e-9)
            }
        }
    }
}
```

This test is expected to pass against the old implementation: it is a
refactoring discriminator, not a fabricated red test. Tasks 1–3 earned the new
behavior with real red/green cycles. Record its pre-refactor result.

- [ ] **Step 2: Delegate existing anchored coverage to PlacedCoverage.**

Replace only the existing function body, retaining its public signature and
nil-shape / bad-dimensions / bad-width error order:

```go
func Coverage(emb HexEmbedding, g Grid, in CoverageInput) (CoverageOutput, error) {
    if in.Footprint.Box == nil { return CoverageOutput{}, ErrNoFootprint }
    b := *in.Footprint.Box
    for _, v := range []float64{b.W, b.D} {
        if v <= 0 || math.IsNaN(v) || math.IsInf(v, 0) {
            return CoverageOutput{}, ErrBadFootprint
        }
    }
    if err := (HexEmbeddingConfig{CellWidth: emb.cellWidth}).Validate(); err != nil {
        return CoverageOutput{}, err
    }
    placement := FootprintPlacement{
        Footprint: in.Footprint, Origin: emb.CellCentre(in.At), Facing: in.Facing,
    }
    if in.Anchor == AnchorAtEdge {
        placement.LocalOffset.X = (emb.cellWidth + b.D) / 2
    }
    if _, err := footprintBox(placement); err != nil {
        return CoverageOutput{}, err
    }
    radius := math.Ceil((b.D+b.W/2)/emb.cellWidth) + 1
    if math.IsNaN(radius) || math.IsInf(radius, 0) {
        return CoverageOutput{}, ErrBadFootprintPlacement
    }
    return PlacedCoverage(PlacedCoverageInput{
        Embedding: emb, Placement: placement,
        Cells: g.GetPositionsInRange(in.At, radius),
    })
}
```

Delete the now-unused `boxPolygon`; keep the existing clipping/area helpers as
the single implementation. Do not edit spell consumers.

- [ ] **Step 3: Update documentation with a complete example and limitations.**

Use this complete example in `placed_footprint_example_test.go` and the README.
Go test compiles an example without an Output marker; the suites above execute
the behavioral assertions, rather than asserting nondeterministic map printing.

```go
package spatial_test

import (
    "fmt"

    "github.com/KirkDiggler/rpg-toolkit/tools/spatial"
)

func ExamplePlacedCoverage() {
    emb := spatial.NewHexEmbedding(spatial.HexEmbeddingConfig{CellWidth: 5})
    placement := spatial.FootprintPlacement{
        Footprint: spatial.Footprint{Box: &spatial.Box{W: 4, D: 10}},
        Origin: spatial.Point{X: 3.25, Y: -1.75}, Facing: 37,
    }
    coverage, err := spatial.PlacedCoverage(spatial.PlacedCoverageInput{
        Embedding: emb, Placement: placement,
        Cells: []spatial.Position{{}, {X: 1}, {Y: -1}},
    })
    if err != nil { fmt.Println(err); return }
    trace, err := spatial.TraceFootprint(spatial.FootprintTraceInput{
        Placement: placement,
        From: spatial.Point{X: -5}, To: spatial.Point{X: 8},
    })
    if err != nil { fmt.Println(err); return }
    fmt.Println(coverage.Cells, trace.Contact, trace.Interior)
}
```

Explain candidate universes, units, origin/local offset,
positive angle convention, interval classification, finite refusal, and why
this does not make props occupy cells in BasicRoom yet. `doc.go` keeps game
rules and 3D out of scope.

- [ ] **Step 4: Verify the whole provider and normal tooling.**

```bash
gofmt -w placed_footprint.go placed_footprint_test.go placed_coverage.go placed_coverage_test.go footprint_trace.go footprint_trace_test.go placed_footprint_example_test.go coverage.go coverage_test.go embedding.go embedding_test.go doc.go
go test ./... -run '^(TestPlacedFootprintSuite|TestPlacedCoverageSuite|TestFootprintTraceSuite|TestCoverageSuite|TestEmbeddingSuite)$' -count=1
go test -race -count=1 ./...
golangci-lint run ./...
go mod tidy
git diff --check
git status --short
```

No dependency addition is expected. Investigate any tidy diff; do not refresh
other modules. Missing tools/setup problems are blockers, not authorization to
change global tooling or bypass checks.

- [ ] **Step 5: Commit and request independent review through normal project flow.**

```bash
git add coverage.go coverage_test.go embedding.go embedding_test.go doc.go README.md placed_footprint.go placed_footprint_test.go placed_coverage.go placed_coverage_test.go footprint_trace.go footprint_trace_test.go placed_footprint_example_test.go
git commit -m 'refactor(spatial): share footprint geometry with anchored coverage'
```

Parent owns issue/Project 19 linkage, draft-first publication, independent
review, and user-facing handoff. Review should try to falsify axis/offset order,
finite-input refusal, thin-contact classification, and unchanged legacy spell
coverage. Before merge, record exact-head checks and the public review verdict.
No merge or cleanup is automatic.

## Coverage matrix and next boundary

| Contract claim | Task |
|---|---|
| continuous origin, free yaw, local rectangle offset | 1 |
| existing box shape/JSON unchanged, no retained aliases | 1 |
| explicit cell universe, no snapping, area fractions | 2 |
| both orientations, finite width/cells, no partial errors | 2 |
| thin obstacle independently of area threshold | 3 |
| contact/interior, tangency, reverse, stationary | 3 |
| old Coverage API and shape behavior remain | 4 |
| one math implementation, documented limitations, module checks | 4 |

**After this provider:** the next plan is the authoring/encounter consumer,
including document-local definitions, typed properties/overrides, canonical
transforms, occupancy/sight integration, wire projection, and the World Builder
floor/footprint interface. Establish that contract against the reviewed provider,
not by filling it with guessed APIs now. Half-cover qualification and rule
application remain the separately named Proof B; neither is claimed by this PR.
