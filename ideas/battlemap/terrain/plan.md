---
status: PLAN for rung 1 of design.md, written 2026-09-11
design: ./design.md §3.3, §3.4, §8 rung 1
issues: rpg-toolkit#614 (spatial PR) · rpg-toolkit#1652 (encounter PR) · journey rpg-project#428
---

# Rung 1 — the field and the fold, proven by the pillar

> **For agentic workers:** REQUIRED SUB-SKILL: use superpowers:subagent-driven-development or superpowers:executing-plans, task by task. Steps use checkbox syntax. Every builder reads `rpg-toolkit/CLAUDE.md` and the module's own `CLAUDE.md`/`AGENTS.md` first. One Go module per PR. Draft PR on the first working push.

**Goal:** a monster standing behind a pillar walks around it, because the route and the step read one answer about what a cell allows.

**Architecture:** `tools/spatial` gains a distance field over `Position` (Dijkstra, uniform cost today) as a package function over the `Grid` interface. `encounter` gains a derived cell-fact fold, `CellAt`, over the sources `Step` already consults (sealed cells, props, members by stance), and both `Step` and the monster route read it. Session is untouched: `validateWalk` checks adjacency and defers the rest to `Step` (rpg-toolkit#1059), which stays true.

**Tech stack:** Go 1.24, testify suites (`suite.Run` with a named suite; `go test -run` must name the suite or it silently matches nothing), golangci-lint at the repo's cyclomatic limit of 15.

**Facts every task relies on** (verified at `rpg-toolkit` `76c85a41`):

| fact | where |
|---|---|
| `Grid` interface, eight methods incl. `GetNeighbors(pos Position) []Position` | `tools/spatial/interfaces.go:21-45` |
| `Position{X, Y float64}` is comparable, already a map key | `tools/spatial/position.go:10-13` |
| `bfsShortestPath(from, goal func(Position) bool) ([]Position, bool)` | `encounter/clocks.go:1240`; callers `:1113` (remembered, goal `cell == pos`) and `:1170` (seen, goal `Distance(cell,pos) <= range`) |
| `Step` order: integral cell → `e.field.isStandable(to)` → `moveMember` → canvas `MoveEntity` → `canPlaceEntityUnsafe` (valid + no `BlocksMovement()` occupant); boundaries refused inside the same spatial call | `encounter/step.go:195-219`, `tools/spatial/room.go:430` |
| `isStandable` = `!sealedCells[cell] && owned`; sealed from walls only | `encounter/compilefield.go:741`, `:601` |
| prop entity: `propEntityOf` builds `*propEntity` with `BlocksMovement()`; `GetSize()` hardcoded 1 | `encounter/reserve.go:407`, `encounter.go:1968`, `:1956` |
| member entity: `memberEntity.blocksMovement` from `MemberInput.BlocksMovement`, default false; `GetSize()` hardcoded 1 | `encounter/encounter.go:2450`, `:2099`, `:2463` |
| stance: `e.opposed(a, b MemberID) bool`; exported `IsHostile(a, b) (hostile, known bool)` | `encounter/world.go:430`, `:465` |
| canvas read-only wrapper; `GetEntitiesAt`, `GetGrid` | `encounter/canvas.go:113-218` |
| wall-avoidance route test to mirror for the pillar | `encounter/monsterturn_test.go:1389` `TestSeenMemberPathWalksAroundAWall` |
| region/wall fixtures | `encounter/regionfixtures_test.go` (`rectRegion`, `cellAt`, `wall`, `pointyCanvas`, `openAir`) |
| `PropInput` with required pointer bools `BlocksMovement`, `BlocksLoS` | `encounter/field.go:358`, `:365`; a literal to copy is at `reserve.go:327` and in `holdout_test.go` |
| pins: encounter → `tools/spatial v0.11.0` (`encounter/go.mod:11`); session → `encounter v0.71.0` | latest tags `tools/spatial/v0.11.0`, `rulebooks/dnd5e/encounter/v0.71.0`, `rulebooks/dnd5e/session/v0.73.0` |

**Out of scope, on purpose:** deleting `SimplePathFinder` (its one caller is legacy `tools/environments`); creature size (no input exists; the two-sizes exception waits); the client's `atlasPath.ts`; any change to session.

---

## PR A — `tools/spatial`: the distance field (closes rpg-toolkit#614)

Branch `feat/614-distance-field` in a worktree of `rpg-toolkit`. Title prefix `feat(spatial):` so the squash mints a minor tag.

### Task A1: `Field` with one source on an open hex grid

The game is hex by choice, so the field's primary tests run on the
`AxialHexGrid` the encounter runs (`hex_grid.go:319`), and distances are
asserted through the grid's own `Distance`, never as literals a square
reader would recognise. One square test arrives in A2 as the evidence that
closes #614; it is not the model.

**Files:**
- Create: `tools/spatial/field.go`
- Create: `tools/spatial/field_test.go`

- [ ] **Step 1: write the failing test**

```go
package spatial

import (
	"testing"

	"github.com/stretchr/testify/suite"
)

type FieldTestSuite struct {
	suite.Suite
	grid Grid // the axial hex grid the encounter runs
}

func (s *FieldTestSuite) SetupTest() {
	s.grid = NewAxialHexGrid(AxialHexGridConfig{Width: 7, Height: 7})
}

func (s *FieldTestSuite) TestOpenHexDistancesMatchTheGrid() {
	origin := Position{X: 3, Y: 3}
	out, err := Field(s.grid, FieldInput{
		Sources:  []Position{origin},
		Passable: func(from, to Position) bool { return true },
	})
	s.Require().NoError(err)
	s.Equal(0, out.Dist[origin])
	for cell, d := range out.Dist {
		s.Equal(int(s.grid.Distance(origin, cell)), d, "field distance is the grid's distance on open ground: %v", cell)
	}
	s.Equal(6, countAt(out, 1), "a hex has six neighbours")
}

func countAt(out FieldOutput, d int) int {
	n := 0
	for _, v := range out.Dist {
		if v == d {
			n++
		}
	}
	return n
}

func TestFieldTestSuite(t *testing.T) { suite.Run(t, new(FieldTestSuite)) }
```

Check the real constructor name and config type at `tools/spatial/hex_grid.go:319-340` before running, and use those. Do not assert the total cell count: it pins a collection's length and tests nothing.

- [ ] **Step 2: run it, expect a compile failure naming `Field`**

```
cd /path/to/worktree/tools/spatial && go test -run TestFieldTestSuite ./...
```

- [ ] **Step 3: implement**

```go
package spatial

import (
	"container/heap"
	"errors"
	"sort"
)

// FieldInput asks for a distance field: every cell reachable from Sources
// under Passable, with its cost-weighted distance. Cost nil means one per
// step. Limit 0 means unbounded.
type FieldInput struct {
	Sources  []Position
	Passable func(from, to Position) bool
	Cost     func(from, to Position) int
	Limit    int
}

// FieldOutput is the field. Dist holds every reached cell; Prev holds the
// cell each reached cell was entered from, absent for sources.
type FieldOutput struct {
	Dist map[Position]int
	Prev map[Position]Position
}

// ErrNoSources is returned when a field is asked for with nothing to flood from.
var ErrNoSources = errors.New("spatial: field needs at least one source")

// ErrNoPassable is returned when Passable is nil; a field with no predicate
// would silently treat every cell as open, which is a zero value that lies.
var ErrNoPassable = errors.New("spatial: field needs a passable predicate")

// Field floods outward from Sources over g.GetNeighbors, Dijkstra by Cost.
// Neighbors are visited in X-then-Y order so equal-cost ties are stable.
func Field(g Grid, in FieldInput) (FieldOutput, error) {
	if len(in.Sources) == 0 {
		return FieldOutput{}, ErrNoSources
	}
	if in.Passable == nil {
		return FieldOutput{}, ErrNoPassable
	}
	cost := in.Cost
	if cost == nil {
		cost = func(Position, Position) int { return 1 }
	}
	out := FieldOutput{Dist: map[Position]int{}, Prev: map[Position]Position{}}
	pq := &fieldQueue{}
	for _, src := range in.Sources {
		out.Dist[src] = 0
		heap.Push(pq, fieldItem{pos: src, dist: 0})
	}
	for pq.Len() > 0 {
		cur := heap.Pop(pq).(fieldItem)
		if cur.dist > out.Dist[cur.pos] {
			continue // stale entry
		}
		ns := g.GetNeighbors(cur.pos)
		sort.Slice(ns, func(i, j int) bool {
			if ns[i].X != ns[j].X {
				return ns[i].X < ns[j].X
			}
			return ns[i].Y < ns[j].Y
		})
		for _, n := range ns {
			if !in.Passable(cur.pos, n) {
				continue
			}
			d := cur.dist + cost(cur.pos, n)
			if in.Limit > 0 && d > in.Limit {
				continue
			}
			if prev, seen := out.Dist[n]; seen && prev <= d {
				continue
			}
			out.Dist[n] = d
			out.Prev[n] = cur.pos
			heap.Push(pq, fieldItem{pos: n, dist: d})
		}
	}
	return out, nil
}

type fieldItem struct {
	pos  Position
	dist int
}

type fieldQueue []fieldItem

func (q fieldQueue) Len() int { return len(q) }
func (q fieldQueue) Less(i, j int) bool {
	if q[i].dist != q[j].dist {
		return q[i].dist < q[j].dist
	}
	if q[i].pos.X != q[j].pos.X {
		return q[i].pos.X < q[j].pos.X
	}
	return q[i].pos.Y < q[j].pos.Y
}
func (q fieldQueue) Swap(i, j int)       { q[i], q[j] = q[j], q[i] }
func (q *fieldQueue) Push(x any)         { *q = append(*q, x.(fieldItem)) }
func (q *fieldQueue) Pop() any           { old := *q; n := len(old); it := old[n-1]; *q = old[:n-1]; return it }
```

- [ ] **Step 4: run, expect PASS**
- [ ] **Step 5: commit** `feat(spatial): distance field over Position — Field floods from sources under a passable predicate`

### Task A2: `Passable` cuts the field, `Limit` bounds it, and a `PathTo` reads it back

**Files:** modify `tools/spatial/field.go`, `tools/spatial/field_test.go`

- [ ] **Step 1: write the failing tests.** The wall is a full column of blocked hexes with one gap at the bottom, so every route from west to east must go through the gap; assert the path through the grid's own facts, never through a literal count.

```go
func (s *FieldTestSuite) blockedColumn(x float64, gapY float64) map[Position]bool {
	blocked := map[Position]bool{}
	for y := 0.0; y < 7; y++ {
		if y != gapY {
			blocked[Position{X: x, Y: y}] = true
		}
	}
	return blocked
}

func (s *FieldTestSuite) TestABlockedColumnForcesTheRouteThroughTheGap() {
	blocked := s.blockedColumn(3, 6)
	from, goal := Position{X: 1, Y: 3}, Position{X: 5, Y: 3}
	out, err := Field(s.grid, FieldInput{
		Sources:  []Position{from},
		Passable: func(_, to Position) bool { return !blocked[to] },
	})
	s.Require().NoError(err)
	for cell := range blocked {
		_, reached := out.Dist[cell]
		s.False(reached, "a blocked cell is never entered: %v", cell)
	}
	s.Greater(out.Dist[goal], int(s.grid.Distance(from, goal)), "the detour is longer than the crow flies")
	path, ok := out.PathTo(goal)
	s.Require().True(ok)
	s.Contains(path, Position{X: 3, Y: 6}, "the only way east is the gap")
}

func (s *FieldTestSuite) TestLimitStopsTheFlood() {
	origin := Position{X: 3, Y: 3}
	out, err := Field(s.grid, FieldInput{
		Sources:  []Position{origin},
		Passable: func(Position, Position) bool { return true },
		Limit:    2,
	})
	s.Require().NoError(err)
	for cell, d := range out.Dist {
		s.LessOrEqual(d, 2, "nothing past the limit is in the field: %v", cell)
	}
	s.Equal(6, countAt(out, 1))
	s.Equal(12, countAt(out, 2), "the second hex ring")
}

func (s *FieldTestSuite) TestPathToWalksPrevBackToASource() {
	from, goal := Position{X: 1, Y: 3}, Position{X: 5, Y: 3}
	out, err := Field(s.grid, FieldInput{
		Sources:  []Position{from},
		Passable: func(Position, Position) bool { return true },
	})
	s.Require().NoError(err)
	path, ok := out.PathTo(goal)
	s.Require().True(ok)
	s.Equal(goal, path[len(path)-1], "path ends at the goal")
	s.NotContains(path, from, "path excludes the source, matching bfsShortestPath's contract")
	s.Len(path, out.Dist[goal], "one cell per unit of distance on open ground")
	for i := 1; i < len(path); i++ {
		s.True(s.grid.IsAdjacent(path[i-1], path[i]), "every step is a neighbour")
	}
	_, ok = out.PathTo(Position{X: -1, Y: -1})
	s.False(ok, "an unreached cell has no path")
}

func (s *FieldTestSuite) TestTheFieldIsGridGeneric() {
	// The evidence that closes #614: the same function runs on a square grid.
	// Not the model — the game is hex by choice.
	sq := NewSquareGrid(SquareGridConfig{Width: 5, Height: 5})
	out, err := Field(sq, FieldInput{
		Sources:  []Position{{X: 0, Y: 0}},
		Passable: func(Position, Position) bool { return true },
	})
	s.Require().NoError(err)
	s.Equal(int(sq.Distance(Position{X: 0, Y: 0}, Position{X: 4, Y: 4})), out.Dist[Position{X: 4, Y: 4}])
}

func (s *FieldTestSuite) TestNoSourcesAndNoPredicateFailClosed() {
	_, err := Field(s.grid, FieldInput{Passable: func(Position, Position) bool { return true }})
	s.ErrorIs(err, ErrNoSources)
	_, err = Field(s.grid, FieldInput{Sources: []Position{{}}})
	s.ErrorIs(err, ErrNoPassable)
}
```

- [ ] **Step 2: run, expect `PathTo` undefined**
- [ ] **Step 3: implement `PathTo`**

```go
// PathTo reads the field backwards from goal to the source that reached it.
// The returned path excludes the source and ends at goal, matching the
// contract encounter's route has always had. ok is false when goal was
// never reached.
func (f FieldOutput) PathTo(goal Position) ([]Position, bool) {
	if _, reached := f.Dist[goal]; !reached {
		return nil, false
	}
	var rev []Position
	cur := goal
	for {
		prev, hasPrev := f.Prev[cur]
		if !hasPrev {
			break // cur is a source
		}
		rev = append(rev, cur)
		cur = prev
	}
	for i, j := 0, len(rev)-1; i < j; i, j = i+1, j-1 {
		rev[i], rev[j] = rev[j], rev[i]
	}
	return rev, true
}
```

- [ ] **Step 4: run, expect PASS; run `golangci-lint run ./tools/spatial/...` from the repo root and keep `Field` under the cyclomatic limit (split the neighbor loop into a helper if the linter says so)**
- [ ] **Step 5: commit** `feat(spatial): Field honours Passable and Limit; PathTo reads a path back`

### Task A3: the charter says it

**Files:** modify `tools/spatial/doc.go` (the Non-Goals list), `tools/spatial/README.md` (the API section)

- [ ] **Step 1:** in `doc.go` replace the non-goal line `Pathfinding algorithms: AI navigation belongs in behavior package` with `Movement rules: what a cell means (blocked, costly, burning) is the game's; spatial answers where and how far, and Field is the search over that`. Keep "Movement costs" and "Movement rules: Speed, difficult terrain" as non-goals.
- [ ] **Step 2:** in `README.md` add a short `Field` entry beside the existing query list, three lines, with the `FieldInput` fields.
- [ ] **Step 3:** run the whole module's tests: `go test ./...` in `tools/spatial`. Expect PASS.
- [ ] **Step 4: commit** `docs(spatial): Field is the search; the meaning of a cell is not spatial's`
- [ ] **Step 5:** push, open the **draft** PR titled `feat(spatial): distance field over Position (Field, PathTo) #614`, body signed per the cross-team charter. When merged, note the minted tag (expected `tools/spatial/v0.12.0`; verify the tag's content, not its name).

---

## PR B — `encounter`: the fold, read by the step and the route (closes rpg-toolkit#1652)

Branch `fix/1652-cell-facts` in a worktree of `rpg-toolkit`. Depends on PR A: pin `tools/spatial` to A's pushed pseudo-version while A is a draft, and to the minted tag before B leaves draft. Title prefix `fix(encounter):`.

### Task B1: the pillar regression, red

**Files:** modify `rulebooks/dnd5e/encounter/monsterturn_test.go` (beside `TestSeenMemberPathWalksAroundAWall` at `:1389`)

- [ ] **Step 1: write the failing test by copying the wall test and replacing the wall with a prop.** Keep the goblin, the party member, and the geometry of the wall test; replace the `wall(...)` fixture with a `PropInput` at the cell between them. The literal must match `PropInput` at `field.go:350-370`, so copy the field names from the literal in `holdout_test.go`; the shape is:

```go
yes, no := true, false
props := []encounter.PropInput{{
	ID:             "pillar",
	Ref:            "dnd5e:props:pillar",
	At:             cellAt(2, 2),
	BlocksMovement: &yes,
	BlocksLoS:      &no,
}}
```

and the test name is `TestSeenMemberPathWalksAroundAPillar`. Assert exactly what the wall test asserts about `SeenMember.Path`, plus `s.NotContains(path, cellAt(2, 2))`.

- [ ] **Step 2: run it, expect FAIL with the path containing the pillar's cell**

```
cd <worktree>/rulebooks/dnd5e/encounter && go test -run 'TestMonsterTurnTestSuite/TestSeenMemberPathWalksAroundAPillar' ./...
```

(Confirm the suite name at the top of `monsterturn_test.go` and use it; a filter that matches nothing prints PASS.)

- [ ] **Step 3: do not commit yet.** Keep the red failure output; it goes in the PR body as the before. (An earlier version of this plan said "commit the red test alone". That contradicts the repo's pre-commit hook, which runs `go test -race` on changed packages, and led the builder to bypass the hook on three commits, disclosed. The plan was wrong; a red commit is never asked for again.)

### Task B2: `CellAt`, the fold

**Files:**
- Create: `rulebooks/dnd5e/encounter/cellfacts.go`
- Create: `rulebooks/dnd5e/encounter/cellfacts_test.go`

- [ ] **Step 1: write the failing tests** (in the encounter test package, using `regionfixtures_test.go` helpers and a `SetupInput` with one region, one prop, and two monsters of opposed factions — copy the faction setup from `holdout_test.go`)

```go
func (s *CellFactsTestSuite) TestAnOpenFloorCellIsStandable() {
	got := s.enc.CellAt(encounter.CellAtInput{Cell: cellAt(1, 1), Mover: s.goblin})
	s.Equal(encounter.PassageStandable, got.Passage)
	s.Empty(got.Contribs)
}

func (s *CellFactsTestSuite) TestAPillarBlocks() {
	got := s.enc.CellAt(encounter.CellAtInput{Cell: cellAt(2, 2), Mover: s.goblin})
	s.Equal(encounter.PassageBlocked, got.Passage)
	s.Contains(got.Contribs, encounter.ContribRef{Kind: encounter.ContribProp, ID: "pillar"})
}

func (s *CellFactsTestSuite) TestAHostileCreatureBlocksAndAnAllyIsPassedThrough() {
	hostileCell := s.cellOf(s.hero)
	s.Equal(encounter.PassageBlocked, s.enc.CellAt(encounter.CellAtInput{Cell: hostileCell, Mover: s.goblin}).Passage)
	allyCell := s.cellOf(s.secondGoblin)
	s.Equal(encounter.PassagePassThrough, s.enc.CellAt(encounter.CellAtInput{Cell: allyCell, Mover: s.goblin}).Passage)
}

func (s *CellFactsTestSuite) TestASealedCellIsBlockedByTheWall() {
	got := s.enc.CellAt(encounter.CellAtInput{Cell: cellAt(0, 0), Mover: s.goblin}) // outside every region
	s.Equal(encounter.PassageBlocked, got.Passage)
	s.Contains(got.Contribs, encounter.ContribRef{Kind: encounter.ContribField})
}
```

- [ ] **Step 2: run, expect compile failure on `CellAt`**
- [ ] **Step 3: implement**

```go
package encounter

import "github.com/KirkDiggler/rpg-toolkit/tools/spatial"

// Passage is what a cell allows a mover to do with it.
type Passage int

const (
	// PassageBlocked: the mover may not enter.
	PassageBlocked Passage = iota
	// PassagePassThrough: the mover may cross but not stop (a nonhostile creature's space).
	PassagePassThrough
	// PassageStandable: the mover may enter and stop.
	PassageStandable
)

// ContribKind names who contributed a fact about a cell.
type ContribKind string

const (
	ContribField  ContribKind = "field"  // sealed or unowned by the compiled field
	ContribProp   ContribKind = "prop"   // a placed prop with BlocksMovement
	ContribMember ContribKind = "member" // a creature, by stance to the mover
)

// ContribRef is one contributor to a cell fact; ID is the prop id or member id.
type ContribRef struct {
	Kind ContribKind
	ID   string
}

// CellAtInput asks what one cell allows one mover.
type CellAtInput struct {
	Cell  spatial.Position
	Mover MemberID
}

// CellFact is the fold: everything standing on a cell, combined by policy.
// It is derived on every call and never stored.
type CellFact struct {
	Passage  Passage
	Cost     int // cells; always 1 today
	Contribs []ContribRef
}

// CellAt folds the sources Step has always consulted — the compiled field's
// sealed cells, props, and members — into one answer, so a route and a step
// can never disagree. Blocked wins over PassThrough wins over Standable.
func (e *Encounter) CellAt(in CellAtInput) CellFact {
	fact := CellFact{Passage: PassageStandable, Cost: 1}
	if !e.field.isStandable(in.Cell) {
		fact.Passage = PassageBlocked
		fact.Contribs = append(fact.Contribs, ContribRef{Kind: ContribField})
	}
	for _, ent := range e.canvas.GetEntitiesAt(in.Cell) {
		switch v := ent.(type) {
		case *propEntity:
			if v.BlocksMovement() {
				fact.Passage = PassageBlocked
				fact.Contribs = append(fact.Contribs, ContribRef{Kind: ContribProp, ID: v.GetID()})
			}
		case *memberEntity:
			if v.id == in.Mover {
				continue // a mover may re-enter its own cell
			}
			switch {
			case v.BlocksMovement(), e.opposed(in.Mover, v.id):
				fact.Passage = PassageBlocked
			case fact.Passage == PassageStandable:
				fact.Passage = PassagePassThrough
			}
			fact.Contribs = append(fact.Contribs, ContribRef{Kind: ContribMember, ID: string(v.id)})
		}
	}
	return fact
}
```

Read `memberEntity` (`encounter.go` around `:2440-2470`) for the real field holding its `MemberID` and `propEntity` (`encounter.go:1940-1970`) for `GetID`, and use those names. `e.field` and `e.canvas` are the names used at `step.go:201` and `:208`.

- [ ] **Step 4: run the new suite, expect PASS**
- [ ] **Step 5: commit** `feat(encounter): CellAt — one fold over sealed cells, props, and members by stance`

### Task B3: the route reads the fold through the field

**Files:** modify `rulebooks/dnd5e/encounter/clocks.go:1233-1290` (`bfsShortestPath` and its doc), callers at `:1113` and `:1170`

- [ ] **Step 1: replace `bfsShortestPath` with `routeTo`**

```go
// routeTo finds the shortest route for mover from `from` to the nearest cell
// satisfying goal that the mover may stop on. It reads CellAt for every
// cell, so a route can never cross a cell a step would refuse (#1652).
// The path excludes `from` and ends at the chosen cell. Ties between
// equally near goal cells break by X then Y.
func (e *Encounter) routeTo(mover MemberID, from spatial.Position, goal func(spatial.Position) bool) ([]spatial.Position, bool) {
	grid := e.canvas.GetGrid()
	field, err := spatial.Field(grid, spatial.FieldInput{
		Sources: []spatial.Position{from},
		Passable: func(a, b spatial.Position) bool {
			if e.canvas.IsBoundaryMovementBlocked(a, b) {
				return false
			}
			return e.CellAt(CellAtInput{Cell: b, Mover: mover}).Passage != PassageBlocked
		},
	})
	if err != nil {
		return nil, false
	}
	var best spatial.Position
	bestDist, found := 0, false
	for cell, d := range field.Dist {
		if cell == from || !goal(cell) {
			continue
		}
		if e.CellAt(CellAtInput{Cell: cell, Mover: mover}).Passage != PassageStandable {
			continue // may cross an ally, may not stop on one
		}
		if !found || d < bestDist || (d == bestDist && (cell.X < best.X || (cell.X == best.X && cell.Y < best.Y))) {
			best, bestDist, found = cell, d, true
		}
	}
	if !found {
		return nil, false
	}
	return field.PathTo(best)
}
```

- [ ] **Step 2: update both callers** to `e.routeTo(<the monster's MemberID>, from, goal)`. At `:1113` and `:1170` the monster whose view is being built is the mover; find its `MemberID` in the enclosing `buildMonsterView` signature.
- [ ] **Step 3: run the whole encounter suite:** `go test ./...` in the module. Expect the pillar test from B1 to PASS, `TestSeenMemberPathWalksAroundAWall` to still PASS, and any test that pinned an exact path to be examined: a path that changed but is equally short is a tie-break difference and the assertion should say "as short as", never bump a pinned literal without saying why in the commit.
- [ ] **Step 4: `golangci-lint run ./rulebooks/dnd5e/encounter/...`**; split `routeTo`'s goal scan into a helper if cyclomatic complexity trips.
- [ ] **Step 5: commit** `fix(encounter): the route reads CellAt through spatial.Field, so a pillar is routed around (#1652)`

### Task B4: the step reads the fold too, and names what blocked it

**Files:** modify `rulebooks/dnd5e/encounter/step.go:195-219`; test in `rulebooks/dnd5e/encounter/step_test.go` (or the file that owns `Step`'s refusals; find it with `grep -l "notStandable" *_test.go`)

- [ ] **Step 1: write the failing test:** a member steps onto the pillar's cell; assert the refusal names the pillar (`s.ErrorContains(err, "pillar")`). Today the refusal comes from spatial's placement check and does not say which entity refused.
- [ ] **Step 2: implement:** before `e.field.isStandable(to)` at `step.go:201`, call `fact := e.CellAt(CellAtInput{Cell: to, Mover: member})`; if `fact.Passage == PassageBlocked`, return the existing `notStandable` refusal extended with the contributor (`"pillar blocks the way"` for a prop, the member id for a member, the existing phrase for the field). Leave `moveMember` and the canvas refusal in place beneath it as the last line of defence; they now agree by construction.
- [ ] **Step 3: run the step suite and the whole module; expect PASS.** Note the one behaviour change for the record: a player may no longer walk through a **hostile** creature's cell (the 2014 rule); allies stay passable. Existing tests that walked a player through an enemy will need their intent restated, not their assertion bumped.
- [ ] **Step 4: commit** `fix(encounter): Step refuses on CellAt and names the blocker`

### Task B5: charter and pin

**Files:** modify `rulebooks/dnd5e/encounter/AGENTS.md` ("Where does this go?"), `rulebooks/dnd5e/encounter/go.mod`

- [ ] **Step 1:** in AGENTS.md, under "Where does this go?", add: *a new meaning for a cell (difficult, burning, blocked by a new kind of thing) → `CellAt`; a new geometric question → `tools/spatial`. Nothing in this module searches the grid itself: `routeTo` reads `spatial.Field` through `CellAt`.*
- [ ] **Step 2:** pin `tools/spatial` to PR A's minted tag; `go mod tidy`; `go build ./... && go test ./...`.
- [ ] **Step 3: commit** `chore(encounter): pin tools/spatial to the tag that carries Field; AGENTS.md names the fold`
- [ ] **Step 4:** push; the draft PR is titled `fix(encounter): the route and the step read one fold — the skeleton walks around the pillar (#1652)`. Body: the before/after path from the pillar test, the behaviour change from B4 stated plainly, and the cross-team signature.

---

## The walk

After PR B merges and the local stack pins the encounter tag (`game-dev/scripts/bump-toolkit-pin.sh`, then rpg-api's normal pin PR): load the reference tomb, stand a player where the skeleton at `[13,5]` can see them across the pillar at `[12,5]`, end turn. The skeleton moves. Record the walk on rpg-toolkit#1652 and the journey #428; that record is what lets #429 advance to rung 2.

## What differed in execution (2026-09-11, left visible)

- PR A (#1653): `AxialHexGridConfig` uses `SpanWidth`/`SpanHeight` and is origin-centred; the blocked column runs across the real R range; `(-1,-1)` is a valid hex so the unreached probe is out of bounds; `heap.Pop`'s assertion lives in one typed helper for `errcheck`; `doc.go`'s Scope line was corrected too.
- PR B (#1655): suite is `TestMonsterTurnSuite`; `PropInput.BlocksLineOfSight`, `At` is an authored `[col,row]`; `propEntity.GetID()` is index-derived so `ContribRef` carries `Ref` (the content ref) and `Blocks` (an ally is a contributor, not a blocker); `memberEntity.id` is a string; `cellAt(0,0)` is owned, so the sealed test uses `FieldInput.Sealed`; a monster cannot `Step` off-turn, so step tests drive the player; `Step` asks the boundary before the fold so a refusal never names what stands behind a door (intel rule); `routeTo` keeps `(nil, true)` when the goal already holds at `from` (rpg-project#254).
- Open ruling put to Kirk on #1652: whether a downed hostile still blocks its cell. As built, it does.

## Self-review against the design

- §3.3 field: A1–A2 (`Field`, `PathTo`, `Limit`, `Passable`, uniform `Cost` default). Sources plural is honoured for flee later; only one source is exercised here.
- §3.4 fold: B2 (three-valued `Passage`, `Contribs`), `Cost` present and constant, `OnEnter` deliberately absent until burning ground pays for it (design §8 rung 4). `viewer` is absent: the design says the first implementation ignores it, and an unused parameter is a zero value that lies; it arrives with the first per-observer fact.
- §8 rung 1: two modules, session untouched, `SimplePathFinder` left, pillar regression, charter lines.
- Types used consistently: `CellAtInput`, `CellFact`, `Passage*`, `ContribRef`, `routeTo`, `spatial.Field`, `FieldInput`, `FieldOutput.PathTo`.

— cross-team agent, on behalf of KirkDiggler
