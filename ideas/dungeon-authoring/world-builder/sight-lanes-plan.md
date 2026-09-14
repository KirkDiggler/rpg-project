# Shared Sight Lanes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let footprint consumers use the existing spatial sight-lane evaluator
without copying it or registering fake single-cell occupants.

**Architecture:** Extract the existing lane walk into a pure value query with
caller-supplied obstruction reads. BasicRoom delegates through an internal adapter
under its existing lock. Its valid-input sight behavior stays unchanged.

**Tech Stack:** Go, existing tools/spatial, Testify; no new dependencies.

**Spec:** [sight-lanes.md](sight-lanes.md), under [the architecture](design.md).

## Global Constraints

- One `tools/spatial` implementation PR; no encounter/YAML/proto/API/web edits.
- "No change to valid BasicRoom sight results."
- "No locking, callbacks retained for later, global registration, storage, clocks,
  rules bonuses, or new dependencies in SightLanes."
- "Any callback error returns the zero output plus that error."
- Keep direct hard precedence, soft leaning, gridless single-lane behavior,
  progress-making alternatives from both ends, and canonical rays.
- The existing read lock covers BasicRoom's whole query; its adapter must not
  call public Room methods that acquire another lock.
- Preserve the operator's serial release workflow: isolated issue worktree,
  normal hooks, first working push -> draft PR, human merge, then CI-issued tag.
- Kirk requests one builder, focused TDD, one final module gate, one GLM 5.3 Flash
  review using rpg-project/.agents/skills/pr-review/SKILL.md. No per-task reviews,
  duplicate unchanged gate runs, broad audits, or automatic fix/re-review loop.
- Proposed code is subordinate to the behavior contract. Report small justified
  corrections; ask Kirk via the supervisor before changing geometry/game policy.

## Files

- Create `tools/spatial/sight_lanes.go`: query, input/output contracts, errors.
- Create `tools/spatial/sight_lanes_test.go`: independent geometric consumer and
  hard/soft/error proofs.
- Modify `tools/spatial/room.go`: delegate using a private locked adapter;
  remove the old duplicate lane walk and its unused wrapper.
- Modify `tools/spatial/README.md` and `doc.go`: document ownership and use.
- Existing regression files stay load-bearing:
  `room_test.go`, `boundary_test.go`, `los_law_test.go`, `gridless_test.go`.

All commands below run in the implementation worktree's `tools/spatial`.
No commands in this plan were executed while drafting it.

## Task 1: Expose the lane evaluator and force the footprint use case

**Consumes:** existing `Grid`, `Position`, `CanonicalBoundaryRay`,
`HexEmbedding`, `FootprintPlacement`, and `TraceFootprint` (spatial v0.14.0).
**Produces:** `SightLanes(SightLanesInput) (SightLanesOutput, error)` and the
obstruction interface below. There is no registration or persistent object.

- [ ] **Step 1: Add the independent consumer test before implementation.**

```go
package spatial

import (
    "errors"
    "math"
    "testing"

    "github.com/stretchr/testify/suite"
)

type SightLanesSuite struct { suite.Suite }
func TestSightLanesSuite(t *testing.T) { suite.Run(t, new(SightLanesSuite)) }

type footprintSight struct {
    emb HexEmbedding
    placement FootprintPlacement
}
func (f footprintSight) Along(in SightLaneInput) (SightLaneOutput, error) {
    out, err := TraceFootprint(FootprintTraceInput{Placement: f.placement,
        From: f.emb.CellCentre(in.From), To: f.emb.CellCentre(in.To)})
    return SightLaneOutput{SoftBlocked: out.Interior}, err
}
func (f footprintSight) At(in SightCellInput) (SightCellOutput, error) {
    p := f.emb.CellCentre(in.At)
    out, err := TraceFootprint(FootprintTraceInput{Placement: f.placement, From: p, To: p})
    return SightCellOutput{Blocked: out.Contact}, err
}

func (s *SightLanesSuite) TestThinFootprintNeedsNoRoomOccupants() {
    g := NewAxialHexGrid(AxialHexGridConfig{SpanWidth: 9, SpanHeight: 9})
    obstruction := footprintSight{
        emb: NewHexEmbedding(HexEmbeddingConfig{CellWidth: 5}),
        placement: FootprintPlacement{Footprint: Footprint{Box: &Box{W: 30, D: .2}}},
    }
    in := SightLanesInput{Grid: g, From: Position{X: -2}, To: Position{X: 2},
        Obstructions: obstruction}
    out, err := SightLanes(in)
    s.Require().NoError(err)
    s.True(out.Blocked)
    in.From, in.To = in.To, in.From
    reverse, err := SightLanes(in)
    s.Require().NoError(err)
    s.Equal(out, reverse)
    obstruction.placement.Origin.X = 30
    in.Obstructions = obstruction
    out, err = SightLanes(in)
    s.Require().NoError(err)
    s.False(out.Blocked, "moving the rectangle away clears sight")
}

type directObstruction struct { hard bool; err error }
func (d directObstruction) Along(in SightLaneInput) (SightLaneOutput, error) {
    if d.err != nil { return SightLaneOutput{}, d.err }
    direct := (in.From == (Position{}) && in.To == (Position{X: 3})) ||
        (in.To == (Position{}) && in.From == (Position{X: 3}))
    return SightLaneOutput{HardBlocked: direct && d.hard,
        SoftBlocked: direct && !d.hard}, nil
}
func (d directObstruction) At(SightCellInput) (SightCellOutput, error) {
    return SightCellOutput{}, nil
}

func (s *SightLanesSuite) TestHardDirectLaneCannotBeLeanedAround() {
    in := SightLanesInput{
        Grid: NewAxialHexGrid(AxialHexGridConfig{SpanWidth: 11, SpanHeight: 11}),
        To: Position{X: 3}, Obstructions: directObstruction{hard: true},
    }
    out, err := SightLanes(in)
    s.Require().NoError(err)
    s.True(out.Blocked)
    in.Obstructions = directObstruction{}
    out, err = SightLanes(in)
    s.Require().NoError(err)
    s.False(out.Blocked, "a clear progress-making alternate bypasses soft obstruction")
}

func (s *SightLanesSuite) TestRefusalsAndCallbackError() {
    _, err := SightLanes(SightLanesInput{})
    s.ErrorIs(err, ErrNoSightGrid)
    in := SightLanesInput{Grid: NewAxialHexGrid(AxialHexGridConfig{SpanWidth: 9, SpanHeight: 9})}
    _, err = SightLanes(in)
    s.ErrorIs(err, ErrNoSightObstructions)
    failure := errors.New("obstruction unavailable")
    in.Obstructions = directObstruction{err: failure}
    out, err := SightLanes(in)
    s.ErrorIs(err, failure)
    s.Equal(SightLanesOutput{}, out)
    in.From.X = math.NaN()
    _, err = SightLanes(in)
    s.ErrorIs(err, ErrBadSightPosition)
}
```

The synthetic direct-obstruction case tests a geometric query contract by its
returned visibility, not mock call counts. The thin-footprint case uses real
geometry with no Room, proving why this public seam exists.

- [ ] **Step 2: Run red.**

```bash
go test ./... -run '^TestSightLanesSuite$' -count=1
```

Expected: undefined new query/types, not an unrelated setup failure.

- [ ] **Step 3: Implement the query in `sight_lanes.go`.**

Use package spatial and imports errors/math. Add public godoc for every type,
method contract, and error; preserve the distinctions explained in the spec.

```go
type SightLaneInput struct { From, To Position; Ray []Position }
type SightLaneOutput struct { HardBlocked, SoftBlocked bool }
type SightCellInput struct { At Position }
type SightCellOutput struct { Blocked bool }
type SightObstructions interface {
    Along(SightLaneInput) (SightLaneOutput, error)
    At(SightCellInput) (SightCellOutput, error)
}
type SightLanesInput struct { Grid Grid; From, To Position; Obstructions SightObstructions }
type SightLanesOutput struct { Blocked bool }

var ErrNoSightGrid = errors.New("spatial: sight lanes require a grid")
var ErrNoSightObstructions = errors.New("spatial: sight lanes require obstruction reads")
var ErrBadSightPosition = errors.New("spatial: sight endpoints must be finite")

func (in SightLanesInput) lane(pair SightLaneInput) (SightLaneOutput, error) {
    pair.Ray = CanonicalBoundaryRay(in.Grid, pair.From, pair.To)
    return in.Obstructions.Along(pair)
}

func SightLanes(in SightLanesInput) (SightLanesOutput, error) {
    if in.Grid == nil { return SightLanesOutput{}, ErrNoSightGrid }
    if in.Obstructions == nil { return SightLanesOutput{}, ErrNoSightObstructions }
    for _, v := range []float64{in.From.X, in.From.Y, in.To.X, in.To.Y} {
        if math.IsNaN(v) || math.IsInf(v, 0) {
            return SightLanesOutput{}, ErrBadSightPosition
        }
    }
    direct, err := in.lane(SightLaneInput{From: in.From, To: in.To})
    if err != nil { return SightLanesOutput{}, err }
    if direct.HardBlocked { return SightLanesOutput{Blocked: true}, nil }
    if !direct.SoftBlocked { return SightLanesOutput{}, nil }
    if in.Grid.GetShape() == GridShapeGridless {
        return SightLanesOutput{Blocked: true}, nil
    }
    distance := in.Grid.Distance(in.From, in.To)
    for _, alt := range in.Grid.GetNeighbors(in.From) {
        occupied, err := in.Obstructions.At(SightCellInput{At: alt})
        if err != nil { return SightLanesOutput{}, err }
        if occupied.Blocked || in.Grid.Distance(alt, in.To) >= distance { continue }
        lane, err := in.lane(SightLaneInput{From: alt, To: in.To})
        if err != nil { return SightLanesOutput{}, err }
        if !lane.HardBlocked && !lane.SoftBlocked { return SightLanesOutput{}, nil }
    }
    for _, alt := range in.Grid.GetNeighbors(in.To) {
        occupied, err := in.Obstructions.At(SightCellInput{At: alt})
        if err != nil { return SightLanesOutput{}, err }
        if occupied.Blocked || in.Grid.Distance(in.From, alt) >= distance { continue }
        lane, err := in.lane(SightLaneInput{From: in.From, To: alt})
        if err != nil { return SightLanesOutput{}, err }
        if !lane.HardBlocked && !lane.SoftBlocked { return SightLanesOutput{}, nil }
    }
    return SightLanesOutput{Blocked: true}, nil
}
```

This is the existing BasicRoom lane sequence, not a new corner/ray policy. No
special footprint interpretation appears in the evaluator; the test adapter
shows how a consumer supplies those facts.

- [ ] **Step 4: Run green; add the alternate-origin error discriminator.**

```go
type originErrorObstruction struct { failure error }
func (o originErrorObstruction) Along(SightLaneInput) (SightLaneOutput, error) {
    return SightLaneOutput{SoftBlocked: true}, nil
}
func (o originErrorObstruction) At(SightCellInput) (SightCellOutput, error) {
    return SightCellOutput{}, o.failure
}
func (s *SightLanesSuite) TestAlternateOriginFailureIsNotClearSight() {
    failure := errors.New("origin unavailable")
    out, err := SightLanes(SightLanesInput{
        Grid: NewAxialHexGrid(AxialHexGridConfig{SpanWidth: 9, SpanHeight: 9}),
        To: Position{X: 2}, Obstructions: originErrorObstruction{failure: failure},
    })
    s.ErrorIs(err, failure)
    s.Equal(SightLanesOutput{}, out)
}
```

```bash
go test ./... -run '^TestSightLanesSuite$' -count=1
gofmt -w sight_lanes.go sight_lanes_test.go
```

## Task 2: BasicRoom delegates and existing behavior stays put

**Consumes:** Task 1 query. **Produces:** unchanged
`(*BasicRoom).IsLineOfSightBlocked(from, to Position) bool` over the shared query.
**Files:** room.go, README.md, doc.go; existing regression suites unchanged.

- [ ] **Step 1: Record the focused pre-refactor regression result.**

```bash
go test ./... -run '^(TestRoomSuite|TestBoundarySuite|TestLineOfSightLawSuite|TestGridlessRoomSuite)$' -count=1
```

Expected green. This is a behavior-preserving refactor checkpoint, not fake red.
Do not begin new fuzz/mutation/benchmark campaigns.

- [ ] **Step 2: Replace the method body and add its private adapter.**

```go
func (r *BasicRoom) IsLineOfSightBlocked(from, to Position) bool {
    r.mutex.RLock()
    defer r.mutex.RUnlock()
    out, err := SightLanes(SightLanesInput{Grid: r.grid, From: from, To: to,
        Obstructions: roomSightObstructions{room: r}})
    return err != nil || out.Blocked
}

type roomSightObstructions struct { room *BasicRoom }
func (b roomSightObstructions) Along(in SightLaneInput) (SightLaneOutput, error) {
    if b.room.boundaryBlocksAlongUnsafe(in.Ray) {
        return SightLaneOutput{HardBlocked: true}, nil
    }
    return SightLaneOutput{SoftBlocked: b.room.entityBlocksAlongUnsafe(in.Ray)}, nil
}
func (b roomSightObstructions) At(in SightCellInput) (SightCellOutput, error) {
    return SightCellOutput{Blocked: b.room.blocksLineOfSightUnsafe(in.At)}, nil
}
```

Delete `lineOfSightLaneBlockedUnsafe` after confirming its old callers moved.
Keep `boundaryBlocksAlongUnsafe`, `entityBlocksAlongUnsafe`, and
`blocksLineOfSightUnsafe`. Keep the existing rationale about hard boundaries,
soft entities, both-end symmetry, and gridless behavior in the appropriate
public godoc. Replace obsolete links to the deleted private method and stale
claims that spatial has no plane embedding. Do not change valid behavior to
match new prose.

- [ ] **Step 3: Verify the consumer regression and update documentation.**

```bash
go test ./... -run '^(TestSightLanesSuite|TestRoomSuite|TestBoundarySuite|TestLineOfSightLawSuite|TestGridlessRoomSuite)$' -count=1
gofmt -w sight_lanes.go sight_lanes_test.go room.go doc.go
```

README must explain the two callback answers, read-only Ray, stable-view caller
responsibility, errors, and locked BasicRoom adapter. Use the complete
`footprintSight` test as the example of footprint composition; don't claim live
encounter props already use it. Preserve existing Grid/Room public signatures.

- [ ] **Step 4: One final module gate, normal commit, and draft PR.**

```bash
go test -race -count=1 ./...
golangci-lint run ./...
go mod tidy
git diff --check
git add sight_lanes.go sight_lanes_test.go room.go README.md doc.go
git commit -m 'feat(spatial): expose shared sight-lane evaluation'
```

No dependency diff is expected. Do not bypass hooks or repeat an identical
unchanged-head gate already proven by the normal hook. Preserve raw evidence
under the worktree's ignored SDD directory. Push only the assigned issue branch
and open a draft to main on the first meaningful working push. Independent
review is GLM 5.3 Flash with the project skill; findings come back to Kirk before
fix/re-review cycles. No automatic merge or consumer pin.

## Self-review / coverage map

| Requirement | Proof |
|---|---|
| Footprints without single-cell room occupancy | Task 1 real rectangle adapter |
| Hard direct obstruction vs soft alternatives | Task 1 hard/soft test |
| Errors do not become clear visibility | Task 1 collaborator/position/origin-error tests |
| Existing symmetry, gridless and wall behavior | Task 2 existing regression suites |
| One lane algorithm, lock not duplicated | Task 2 delegate and unsafe adapter |
| No game-policy or RPC addition | Scope/file gate and independent review |

After its released tag, the encounter consumer can combine legacy obstruction
facts and continuous footprint traces through this same evaluator. Standing
policy and movement crossings remain encounter's separate work, not settings
on this spatial query.
