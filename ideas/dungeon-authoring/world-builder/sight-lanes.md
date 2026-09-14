# Shared sight lanes — the encounter integration prerequisite

**Status:** implementation contract, 2026-09-14. Scope: reuse the existing spatial
sight algorithm with caller-supplied obstruction facts. No change to valid
BasicRoom sight results. Parent: [World Builder dungeon authoring](design.md).

## Why this small provider precedes the encounter change

The consumer seam check found `BasicRoom.IsLineOfSightBlocked` owns the direct /
progress-making-neighbour lane evaluation, but its obstruction reads are private
single-cell occupancy methods (`tools/spatial/room.go:500-668`). The encounter's
canvas delegates to it after the existing void check. Merely adding footprint
math to CellAt cannot make that sight path understand a freely placed shape.

Extract the existing lane evaluation once into spatial, then have BasicRoom use
it. The encounter can supply legacy obstacles plus placed-footprint trace facts
without copying the neighbour-lane algorithm or treating a thin prop as a wall
that nobody can lean around. This is a provider seam paid for by the consumer,
not a new RPC or a redesign of sight rules.

Spatial v0.14.0 is released. Its TraceFootprint is the forcing consumer in this
provider's test. Root source was inspected at toolkit main `05ef17487ceb69778ecb11312bf23214100a2c13`.

## Public contract

```go
type SightLaneInput struct {
    From, To Position
    Ray []Position
}
type SightLaneOutput struct {
    HardBlocked bool
    SoftBlocked bool
}
type SightCellInput struct { At Position }
type SightCellOutput struct { Blocked bool }
type SightObstructions interface {
    Along(SightLaneInput) (SightLaneOutput, error)
    At(SightCellInput) (SightCellOutput, error)
}
type SightLanesInput struct {
    Grid Grid
    From, To Position
    Obstructions SightObstructions
}
type SightLanesOutput struct { Blocked bool }
func SightLanes(in SightLanesInput) (SightLanesOutput, error)
```

- `Ray` is the existing canonical grid ray, oriented From toward To. It is
  provided once per evaluated lane; callbacks treat it as read-only and retain
  no reference. Continuous shapes may instead use the supplied endpoints with
  the embedding and TraceFootprint.
- HardBlocked is a direct-lane obstruction that cannot be bypassed by neighbour
  lanes (existing boundaries). SoftBlocked is one that can (existing entities,
  later footprint props). Both kinds block an alternative lane.
- At identifies an opaque alternate origin, matching the existing exclusion of
  neighbours occupied by sight blockers. It is not a movement/standing rule.
- Preserve direct-lane early return, hard-boundary precedence, gridless's one
  lane, strictly-closer neighbours, and alternatives from both endpoints.
- Callbacks supply facts, not an alternative search. For symmetric obstruction
  facts the existing direction symmetry remains; the query does not promise to
  turn an intentionally asymmetric callback into an undirected obstacle.
- Any callback error returns the zero output plus that error, not an invented
  clear or blocked answer. Missing Grid/Obstructions return
  ErrNoSightGrid/ErrNoSightObstructions. Non-finite endpoints return
  ErrBadSightPosition; do not impose integer coordinates on gridless inputs or
  add new membership restrictions to the existing discrete ray contract.
- No locking, callbacks retained for later, global registration, storage, clocks,
  rules bonuses, or new dependencies in SightLanes. The caller supplies a stable
  view for the duration of the query.

## BasicRoom integration

BasicRoom keeps its existing read lock around the complete query. An unexported
value adapter reads the existing unsafe boundary/entity helpers while that lock
is held; it never calls a locking public Room method recursively. Boundary reads
short-circuit before the entity scan, just as today.

`IsLineOfSightBlocked` retains its signature and delegates to SightLanes. Its
owned adapter cannot return an error; a rejected invalid query fails closed at
this boolean-only boundary. Valid queries retain their prior behavior. Do not
change movement, CanPlaceEntity, entity placement, boundaries, or the encounter
module in this PR. Remove the duplicate lane walk from BasicRoom; keep the low-
level occupancy/boundary readers used by its adapter and other methods.

## Proof

1. A supplied thin rectangle can obstruct every lane without any Room entity
   occupying its anchor. Moving that rectangle out of the way clears sight.
2. Direct hard blocking remains absolute; a soft obstruction can be leaned
   around. Gridless uses the direct lane only.
3. Callback errors, missing collaborators, and non-finite endpoints are errors.
4. Existing room, boundary, and sight-law suites retain their outcomes, including
   symmetry and no movement dependency on sight. No new fuzz campaign is needed.
5. Run focused tests during work, one final spatial module race/lint gate, and
   one GLM 5.3 Flash review using rpg-project's pr-review skill. Bring Kirk in
   before a fix/re-review loop, not after repeated speculative passes.

## Next consumer and the standing policy

Kirk agreed that prop standing rules must be adjustable and suggested the hex
centre as the first useful model. Carry that into the encounter contract:
centre-covered initially, with a percentage-coverage alternative and an explicit
threshold, separate from prop definitions and from spatial geometry. No fixed
50% rule is implied. `TraceFootprint`'s stationary Contact query can test whether
the centre is covered; this includes the rectangle boundary. Segment-interior
crossing remains a distinct movement check, and neither fact is D&D half cover.

That consumer still owns live prop data, placement lifetime, standing and crossing
policy, YAML/atlas representation, and integration into the existing authoring
RPCs. This extraction does not claim any of those delivered.
