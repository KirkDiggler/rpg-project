---
status: PLAN for ./design.md, written 2026-09-11
design: ./design.md · directive: ../../battlemap/directed-movement/design.md §3–4 · coverage: ../../battlemap/terrain/design.md §3.2
journeys: rpg-project#430 · rpg-project#243
---

# Thunderwave — the plan, one module per PR, merged bottom-up

> **For agentic workers:** REQUIRED SUB-SKILL: superpowers:subagent-driven-development or superpowers:executing-plans, task by task. Every builder reads `rpg-toolkit/CLAUDE.md` and the module's own `CLAUDE.md`/`AGENTS.md` first. Draft PR on the first working push. Test first; run the test and SEE it fail before implementing; **never commit red** (the pre-commit hook runs the module's tests; the red output goes in the PR body). Name the suite in `-run` and grep `=== RUN`. Never pin a collection's length. No session URLs anywhere.

**Goal:** the bard casts Thunderwave toward a chosen cell; every creature under the cube saves; failures take one shared 2d8 and slide two cells straight away, stopping in front of whatever the fold refuses; the story names the pillar and nobody's reaction fires.

**Architecture:** coverage in `tools/spatial` over a hex embedding; `MembersCovered` and the `Line` directive in `encounter`; the shape, origin, and `CastMove` declarations plus the profile in the `rulebooks/dnd5e` root module; `ImposedMove` in `resolution`; `TargetCell` with its executor and the shape switch routed down in `session`; one additive proto field; rpg-api copies it; the web routes the ground click to the armed cast.

**Modules and order.** `tools/spatial` → `rulebooks/dnd5e/encounter` → `rulebooks/dnd5e` (root: `combat/actions`, `spells`, `character/choices`) → `rulebooks/dnd5e/resolution` → `rulebooks/dnd5e/session` → `rpg-api-protos` → `rpg-api` → `rpg-dnd5e-web`. Each toolkit PR pins the one below it at a pushed pseudo-version while drafting and at the minted tag before leaving draft (`game-dev/scripts/bump-toolkit-pin.sh`). The walk happens from pseudo-versions on the local stack before anything leaves draft.

**Facts every task relies on** (verified 2026-09-11 at rpg-toolkit `dd8620ec`, protos `5768b06`, rpg-api/web `dev`):

| fact | where |
|---|---|
| `hexGeom{kind, qx,qy,rx,ry, width,height, corner [6]worldPoint, sides}`; `world`, `centreOf`, `hexOf`, `directionOf` (bearing in degrees, ok=false when from==to); `clipHalfPlane`, `polygonArea`, `nearHalfPlane` | `encounter/dungeonspec/geometry.go:148-171, 247-300, 431-490` |
| `AxialHexGrid{dimensions}`, `NewAxialHexGrid(AxialHexGridConfig{SpanWidth, SpanHeight})`, origin-centred, no orientation | `tools/spatial/hex_grid.go:293-330` |
| encounter orientation `OrientationPointyTop / FlatTop` | `encounter/orientation.go:45-49`; grid built at `compilefield.go:691` |
| `MembersWithin` (linear scan over `e.Members()` by `e.Distance`) | `encounter/shape.go:60-95` |
| `Footprint{Shape, SizeFeet, Origin}`, `CastArea{Footprint, Catches}`, closed `Validate` switches | `combat/actions/area.go` |
| `CastProfile{RangeFeet, Target, MinTargets, MaxTargets, Save, Damage, Effects, Area, Concentration}`; area/target binding in `Validate` | `combat/actions/cast.go:78-121, 202-211` |
| Thunderclap profile (the template) | `spells/cast.go:181-212`; `baneCost()` `:144-149`; the map `castContent` `:55` |
| bard levelled pick `Options: {Bane}`, `Count: 1` | `character/choices/requirements.go:430-437` |
| `ImposedEffectKind` consts, `ImposedEffect` struct; damage producer `applyPreparedDamage` appends `Kind: ImposedDamage` at `:383` | `resolution/contest.go:73-115, 336-395` |
| `ActionInput.AreaMembers` replaces the caller's targets when `Target == CastTargetArea` | `resolution/action.go:37-56, 160-170` |
| `deriveAreaMembers` (switch on origin, then shape → `MembersWithin`; caster dropped for `Others`; world members `UnresolvedNoSheet`) | `session/area.go:69-147` |
| area offer: `TargetKind: TargetArea, Candidates: []` | `session/casts.go:207-217` |
| `CastInput{Session, Member, DeclarationID, Target (deprecated), Targets}` | `session/cast.go:24` |
| `castTargets` refuses any target for Self/Area | `session/cast.go:437` |
| `TargetKind` consts `none/member/area/path` | `session/types.go:2465-2491` |
| `imposedResult` switch → `ResultConditionApplied` / `ResultDamageApplied` | `session/castoutcome.go:120-160` |
| `MoveInput.Path` rides the verb; `selectCompiledOffer(offers, VerbMove, id)` | `session/move.go:21-59, 221-230` |
| proto `CastRequest{session=1, member=2, declaration_id=3, target=4 deprecated, targets=5}`; `enum TargetKind {UNSPECIFIED=0, NONE=1, MEMBER=2, PATH=3, AREA=4}`; `Position` message | `rpg-api-protos dnd5e/api/session/v1alpha1/service.proto:579-592`, `types.proto:474-484` |
| rpg-api cast handler copies `Targets` verbatim | `internal/handlers/dnd5e/session/v1alpha1/cast.go:40-71` (dev) |
| sandbox bard `SpellRefs: []string{baneRef}` | `rpg-api internal/sandboxseed/sandboxseed.go:188` (dev) |
| web: `onHexClick={runEnded === null ? walkTo : undefined}`; MEMBER arm at `:657-673`; AREA fires on arm `:1083-1093` | `SessionEncounterView.tsx:1699`; `combat-experience/useSessionCombatExperience.ts` (dev) |
| directive types (`Route`, `Direct`, `MoveDirective`) | directed-movement design §3–4; not yet built |

**Unit ruling for the plane.** A cell is 5 feet **across the flats** (`FeetPerCell`). The embedding takes `CellWidth` in the caller's unit so spatial stays unit-agnostic; encounter passes 5. A 15-foot box at an axis bearing is then exactly three cells deep.

**Out of scope, on purpose:** `Edges` on coverage (arrives with the first thin prop); half on a save (#414); a client preview of the cube; Fireball's origin at range; cones.

---

## PR 1 — `tools/spatial`: the embedding and coverage (closes rpg-toolkit#1626 by construction)

Branch `feat/coverage`; title `feat(spatial): hex embedding and Coverage — a footprint at a transform becomes cells with fractions`.

### Task 1.1: the embedding, moved

**Files:** create `tools/spatial/embedding.go`, `tools/spatial/embedding_test.go`

- [ ] **Step 1: failing test** (suite `EmbeddingTestSuite`)

```go
func (s *EmbeddingTestSuite) TestPointyCentreAndCornersAtOrigin() {
	emb := NewHexEmbedding(HexEmbeddingConfig{Orientation: HexPointyTop, CellWidth: 5})
	c := emb.CellCentre(Position{X: 0, Y: 0})
	s.InDelta(0, c.X, 1e-9); s.InDelta(0, c.Y, 1e-9)
	corners := emb.CellCorners(Position{X: 0, Y: 0})
	s.Len(corners, 6)
	for _, k := range corners {
		s.InDelta(5/math.Sqrt(3), math.Hypot(k.X, k.Y), 1e-9, "circumradius for 5 across the flats")
	}
}

func (s *EmbeddingTestSuite) TestBearingBetweenNeighboursIsAMultipleOfSixty() {
	emb := NewHexEmbedding(HexEmbeddingConfig{Orientation: HexPointyTop, CellWidth: 5})
	g := NewAxialHexGrid(AxialHexGridConfig{SpanWidth: 9, SpanHeight: 9})
	for _, n := range g.GetNeighbors(Position{}) {
		deg, ok := emb.Bearing(Position{}, n)
		s.True(ok)
		s.InDelta(0, math.Mod(deg+30, 60), 1e-9, "pointy-top neighbours sit at 30°+k·60°")
	}
	_, ok := emb.Bearing(Position{}, Position{})
	s.False(ok, "no bearing to yourself")
}
```

- [ ] **Step 2: run, see `NewHexEmbedding` undefined.**
- [ ] **Step 3: implement** by moving `hexGeom`'s frame from `encounter/dungeonspec/geometry.go:148-300` into spatial, exported, scaled by `CellWidth` instead of circumradius 1:

```go
type HexOrientation int
const (
	HexPointyTop HexOrientation = iota
	HexFlatTop
)
type Point struct{ X, Y float64 }
type HexEmbeddingConfig struct {
	Orientation HexOrientation
	CellWidth   float64 // across the flats, in the caller's unit; must be > 0
}
type HexEmbedding struct { /* qx, qy, rx, ry, corner [6]Point — copied from hexGeom, multiplied by circumradius = CellWidth/√3 */ }
func NewHexEmbedding(c HexEmbeddingConfig) HexEmbedding
func (e HexEmbedding) CellCentre(cell Position) Point
func (e HexEmbedding) CellCorners(cell Position) [6]Point
func (e HexEmbedding) Bearing(from, to Position) (degrees float64, ok bool) // hexGeom.directionOf, without the on-axis check
```

Read `geometryOf` (`geometry.go:172-207`) for the exact basis vectors per orientation and copy them. Panic-free: a `CellWidth <= 0` returns an embedding whose methods return zero points; `Validate() error` on the config says so, and `Coverage` calls it.

- [ ] **Step 4: run, PASS; commit** `feat(spatial): HexEmbedding — cell centre, corners, bearing, moved from encounter's dungeonspec`

### Task 1.2: `Coverage`

**Files:** create `tools/spatial/coverage.go`, `tools/spatial/coverage_test.go`

- [ ] **Step 1: failing tests**

```go
func (s *CoverageTestSuite) SetupTest() {
	s.emb = NewHexEmbedding(HexEmbeddingConfig{Orientation: HexPointyTop, CellWidth: 5})
	s.grid = NewAxialHexGrid(AxialHexGridConfig{SpanWidth: 15, SpanHeight: 15})
}

func (s *CoverageTestSuite) TestABoxOnTheEdgeAlongAnAxisCoversThreeDeep() {
	origin := Position{}
	toward := s.grid.GetNeighbors(origin)[0]
	deg, _ := s.emb.Bearing(origin, toward)
	out, err := Coverage(s.emb, s.grid, CoverageInput{
		Footprint: Footprint{Box: &Box{W: 15, D: 15}},
		At: origin, Facing: deg, Anchor: AnchorAtEdge,
	})
	s.Require().NoError(err)
	_, casterCovered := out.Cells[origin]
	s.False(casterCovered, "the caster's own cell is never under an edge-anchored box")
	s.InDelta(1.0, out.Cells[toward], 1e-9, "the cell straight ahead is fully under the box")
	// three deep: the cell three steps along the bearing is at least half covered, the fourth is not
	third := stepAlong(s.grid, origin, toward, 3)
	fourth := stepAlong(s.grid, origin, toward, 4)
	s.GreaterOrEqual(out.Cells[third], 0.5)
	s.Less(out.Cells[fourth], 0.5)
	for cell, f := range out.Cells {
		s.True(f > 0 && f <= 1, "fraction in (0,1]: %v=%v", cell, f)
	}
}

func (s *CoverageTestSuite) TestABoxAtAnOffAxisBearingIsAsymmetricButHonest() {
	// 15° off the first neighbour's bearing: the covered set must be the drawn box, not a snapped one.
	origin := Position{}
	toward := s.grid.GetNeighbors(origin)[0]
	deg, _ := s.emb.Bearing(origin, toward)
	on, _ := Coverage(s.emb, s.grid, CoverageInput{Footprint: Footprint{Box: &Box{W: 15, D: 15}}, At: origin, Facing: deg, Anchor: AnchorAtEdge})
	off, err := Coverage(s.emb, s.grid, CoverageInput{Footprint: Footprint{Box: &Box{W: 15, D: 15}}, At: origin, Facing: deg + 15, Anchor: AnchorAtEdge})
	s.Require().NoError(err)
	s.NotEqual(keys(on.Cells), keys(off.Cells), "a rotated box covers a different set; nothing snaps to an axis")
	s.InDelta(sumFractions(on.Cells), sumFractions(off.Cells), 0.5, "total covered area is the box's area either way, ±half a cell of edge effects")
}

func (s *CoverageTestSuite) TestRefusals() {
	_, err := Coverage(s.emb, s.grid, CoverageInput{At: Position{}})
	s.ErrorIs(err, ErrNoFootprint)
	_, err = Coverage(s.emb, s.grid, CoverageInput{Footprint: Footprint{Box: &Box{W: 0, D: 15}}, At: Position{}})
	s.ErrorIs(err, ErrBadFootprint)
}
```

(`stepAlong`, `keys`, `sumFractions` are three-line test helpers in the test file.)

- [ ] **Step 2: run, see `Coverage` undefined.**
- [ ] **Step 3: implement**

```go
type Box struct{ W, D float64 } // width across the bearing, depth along it; caller's unit
type Footprint struct{ Box *Box } // Polygon later, when a customer brings one
type AnchorRule int
const (
	AnchorAtCentre AnchorRule = iota // the footprint's centre sits on At's centre
	AnchorAtEdge                     // the footprint's near edge sits on At's boundary along Facing
)
type CoverageInput struct {
	Footprint Footprint
	At        Position
	Facing    float64 // degrees
	Anchor    AnchorRule
}
type CoverageOutput struct{ Cells map[Position]float64 }
var ErrNoFootprint = errors.New("spatial: coverage needs a footprint")
var ErrBadFootprint = errors.New("spatial: footprint sides must be positive")

// Coverage rasterises the footprint at the transform onto g, returning the
// fraction of each cell's area under it. Fractions out; thresholds are the
// caller's. Only cells the grid considers valid are reported.
func Coverage(emb HexEmbedding, g Grid, in CoverageInput) (CoverageOutput, error) {
	if in.Footprint.Box == nil { return CoverageOutput{}, ErrNoFootprint }
	b := *in.Footprint.Box
	if b.W <= 0 || b.D <= 0 { return CoverageOutput{}, ErrBadFootprint }
	rect := boxPolygon(emb, in, b) // four corners in the plane
	radius := math.Ceil((b.D+b.W/2)/emb.cellWidth) + 1
	out := CoverageOutput{Cells: map[Position]float64{}}
	for _, cell := range g.GetPositionsInRange(in.At, radius) {
		hex := emb.CellCorners(cell)
		clipped := clipConvex(hex[:], rect)
		if len(clipped) < 3 { continue }
		f := polygonArea(clipped) / polygonArea(hex[:])
		if f > 0 { out.Cells[cell] = math.Min(f, 1) }
	}
	return out, nil
}
```

`boxPolygon`: unit vector `u` at `Facing`, normal `n`; centre `c = CellCentre(At)`, plus `u·(cellWidth/2)` when `AnchorAtEdge` (the inradius, so the near edge sits on the boundary); the rectangle spans `c + u·[0,D]` (edge) or `c + u·[-D/2, D/2]` (centre) and `n·[-W/2, W/2]`. `clipConvex` is `geometry.go`'s `clipHalfPlane` applied to the rectangle's four edges (`nearHalfPlane`); `polygonArea` moves as is. Keep `Coverage` under the cyclomatic limit by leaving the clipping in helpers.

- [ ] **Step 4: run, PASS; lint; commit** `feat(spatial): Coverage — a box at a transform becomes cells with fractions`
- [ ] **Step 5: `doc.go`** scope gains "Coverage: which cells a footprint lies on, as fractions" and the non-goals keep "thresholds are the game's". `README.md` gains a `Coverage` entry. Commit `docs(spatial): coverage and the embedding`.
- [ ] **Step 6: push; draft PR; body states the unit ruling (cell width across the flats) and that `Edges` waits for the first thin prop; signature `— cross-team agent, on behalf of KirkDiggler`.** Expected tag `tools/spatial/v0.13.0`.

---

## PR 2 — `encounter`: the embedding consumed, `MembersCovered`, and the `Line` directive

Branch `feat/members-covered`; title `feat(encounter): MembersCovered through spatial coverage; Route and Direct with the Line policy`. Pin `tools/spatial` at PR 1's pushed pseudo-version, then its tag.

### Task 2.1: `dungeonspec/geometry.go` calls spatial

**Files:** modify `rulebooks/dnd5e/encounter/dungeonspec/geometry.go`

- [ ] **Step 1:** existing wall-derivation tests are the safety net; run `go test ./dungeonspec/...` green before touching anything and record the count of `=== RUN` lines.
- [ ] **Step 2:** replace `hexGeom`'s frame (`qx..ry`, `corner`, `world`, `centreOf`, `hexOf`, `directionOf`) with a `spatial.HexEmbedding` field built by `geometryOf` via `spatial.NewHexEmbedding(HexEmbeddingConfig{Orientation: toSpatial(o), CellWidth: 1})` — **CellWidth 1 keeps the file's circumradius-1 frame? No:** the file's frame is circumradius 1, which is width `√3`; pass `CellWidth: math.Sqrt(3)` so every existing number is unchanged. Keep `axialAt`, `stepAt`, `standingFraction`, `blocks`, `meets`, `alongWall` here (walls are this file's business); delete the moved helpers. Update the file header: the second customer arrived (coverage), so the frame moved, and this file keeps only wall derivation.
- [ ] **Step 3:** run the same tests; identical `=== RUN` set, all PASS. Commit `refactor(encounter): dungeonspec reads the hex frame from spatial's embedding; wall derivation stays`.

### Task 2.2: `MembersCovered`

**Files:** modify `rulebooks/dnd5e/encounter/shape.go`; test in `shape_test.go`

- [ ] **Step 1: failing test** (build an encounter with `pointyCanvas()`, a region, a caster at `[10,6]`, one member two cells ahead along the first neighbour's bearing, one member behind the caster, one four cells ahead)

```go
func (s *ShapeTestSuite) TestMembersCoveredByAnEdgeAnchoredBox() {
	out, err := s.enc.MembersCovered(&encounter.MembersCoveredInput{
		Footprint: spatial.Footprint{Box: &spatial.Box{W: 15, D: 15}},
		Anchor:    s.casterCell,
		Toward:    s.aheadCell,
		AtEdge:    true,
	})
	s.Require().NoError(err)
	ids := memberIDs(out.Members)
	s.Contains(ids, s.ahead)
	s.NotContains(ids, s.caster, "the caster is never under an edge-anchored box")
	s.NotContains(ids, s.behind)
	s.NotContains(ids, s.farAhead, "four cells out is past a 15-foot box")
}

func (s *ShapeTestSuite) TestTowardEqualToAnchorIsRefused() {
	_, err := s.enc.MembersCovered(&encounter.MembersCoveredInput{
		Footprint: spatial.Footprint{Box: &spatial.Box{W: 15, D: 15}},
		Anchor: s.casterCell, Toward: s.casterCell, AtEdge: true,
	})
	s.ErrorIs(err, encounter.ErrBadReach)
}
```

- [ ] **Step 2: run, see `MembersCovered` undefined.**
- [ ] **Step 3: implement**

```go
// CoverageThreshold is the fraction of a cell a footprint must cover for the
// cell to count: half, the tabletop's template rule and ours.
const CoverageThreshold = 0.5

type MembersCoveredInput struct {
	Footprint spatial.Footprint
	Anchor    spatial.Position
	Toward    spatial.Position
	AtEdge    bool
}
type MembersCoveredOutput struct {
	Members []Member            // stable order: by the roster's order
	Cells   map[spatial.Position]float64 // what was covered, for the beat and the atlas
}

func (e *Encounter) MembersCovered(in *MembersCoveredInput) (MembersCoveredOutput, error) {
	if in == nil { return MembersCoveredOutput{}, fmt.Errorf("members covered: %w", ErrNilInput) }
	emb := e.embedding() // NewHexEmbedding from e.orientation, CellWidth: FeetPerCell
	deg, ok := emb.Bearing(in.Anchor, in.Toward)
	if !ok { return MembersCoveredOutput{}, fmt.Errorf("members covered: toward is the anchor: %w", ErrBadReach) }
	anchor := spatial.AnchorAtCentre
	if in.AtEdge { anchor = spatial.AnchorAtEdge }
	cov, err := spatial.Coverage(emb, e.canvas.GetGrid(), spatial.CoverageInput{
		Footprint: in.Footprint, At: in.Anchor, Facing: deg, Anchor: anchor,
	})
	if err != nil { return MembersCoveredOutput{}, fmt.Errorf("members covered: %w", err) }
	covered := map[spatial.Position]float64{}
	for cell, f := range cov.Cells {
		if f >= CoverageThreshold { covered[cell] = f }
	}
	all, err := e.Members()
	if err != nil { return MembersCoveredOutput{}, fmt.Errorf("members covered: %w", err) }
	out := MembersCoveredOutput{Cells: covered}
	for _, m := range all {
		if _, hit := covered[m.Position]; hit { out.Members = append(out.Members, m) }
	}
	return out, nil
}
```

`e.embedding()` is a one-line helper reading the orientation the field already holds (`orientation.go`); find where `compilefield.go:691` chooses it and read the same value.

- [ ] **Step 4: run, PASS; commit** `feat(encounter): MembersCovered — a footprint toward a cell, at half coverage`

### Task 2.3: `Route` with `Line`, and `Direct`

**Files:** create `rulebooks/dnd5e/encounter/directive.go`, `directive_test.go`; modify `clocks.go` (share `floodFrom`/`nearestStop`), `encounter.go:1291` (`appendMovementBeat` gains a cause)

Build exactly the directed-movement design §3 (`RouteInput{Mover, Policy, Anchor, Budget}`, `RouteOutput{Path}`) and §4 (`DirectInput{Mover, Cause, Route, Provokes}`, `DirectOutput{Moved, StoppedBy}`). This PR implements **`MoveLine` only**; `MoveAway`/`MoveToward` are declared as constants and `Route` refuses them with `ErrUnsupportedPolicy` until their customer (Dissonant Whispers) lands. That is a closed switch failing closed, not a stub.

- [ ] **Step 1: failing tests**

```go
func (s *DirectiveTestSuite) TestLineContinuesPastTheMoverAndStopsBeforeAPillar() {
	// caster at C, mover one cell ahead at M, pillar two cells past M
	out, err := s.enc.Route(encounter.RouteInput{Mover: s.mover, Policy: encounter.MoveLine, Anchor: s.casterCell, Budget: 2})
	s.Require().NoError(err)
	s.Len(out.Path, 1, "one open cell, then the pillar")
	s.True(s.grid.IsAdjacent(s.moverCell, out.Path[0]))
	s.NotEqual(s.pillarCell, out.Path[0])
}

func (s *DirectiveTestSuite) TestLineWithOpenFloorGoesTheWholeBudget() {
	out, err := s.enc.Route(encounter.RouteInput{Mover: s.mover, Policy: encounter.MoveLine, Anchor: s.casterCell, Budget: 2})
	s.Require().NoError(err)
	s.Len(out.Path, 2)
	for i := 1; i < len(out.Path); i++ { s.True(s.grid.IsAdjacent(out.Path[i-1], out.Path[i])) }
	d0 := s.enc.Distance(s.casterCell, s.moverCell)
	for _, p := range out.Path { s.Greater(s.enc.Distance(s.casterCell, p), d0, "every pushed cell is farther from the anchor") }
}

func (s *DirectiveTestSuite) TestDirectMovesOffTurnAndNamesTheBlocker() {
	// the mover is NOT the active member
	route, _ := s.enc.Route(encounter.RouteInput{Mover: s.mover, Policy: encounter.MoveLine, Anchor: s.casterCell, Budget: 2})
	out, err := s.enc.Direct(context.Background(), encounter.DirectInput{Mover: s.mover, Cause: thunderwaveRef, Route: route.Path, Provokes: false})
	s.Require().NoError(err)
	s.Equal(1, out.Moved)
	s.Contains(out.StoppedBy, "dnd5e:props:pillar")
	pos, _ := s.enc.Canvas() // read the mover's cell: it is route.Path[0]
	// assert the last story beat's payload carries "cause": thunderwaveRef
}

func (s *DirectiveTestSuite) TestAwayIsRefusedUntilItsCustomerArrives() {
	_, err := s.enc.Route(encounter.RouteInput{Mover: s.mover, Policy: encounter.MoveAway, Anchor: s.casterCell, Budget: 2})
	s.ErrorIs(err, encounter.ErrUnsupportedPolicy)
}
```

- [ ] **Step 2: run, see undefined.**
- [ ] **Step 3: implement `Route{Line}`:** the continuation of the line from anchor through mover is `far := mover + (mover − anchor)·budget` in axial coordinates (`Position` arithmetic); `cells := grid.GetLineOfSight(mover, far)` excluding `mover`; take up to `Budget` cells in order, stopping before the first whose `CellAt(cell, mover).Passage != PassageStandable` (a creature in the way stops a push; nothing passes through anyone) or whose crossing `IsBoundaryMovementBlocked(prev, cell)`. Record why it stopped for `StoppedBy`.
- [ ] **Step 4: implement `Direct`:** copy `walkCells` (`clocks.go:908`) into `directive.go` as `directCells`, minus the active-turn assumption (call `e.mover.Move` and `e.stepTo` with the directed member, not `activeID`), plus `Cause` on each `appendMovementBeat`. Refactor `walkCells` to call the same helper with an empty cause so there is one walker. When `Provokes` is false, pass an OA-prevention source naming `Cause` into the movement announcement (see `resolution/movement.go:198-215` for the field the fold reads). Charge no turn budget.
- [ ] **Step 5: run the whole module (`go test -race ./...`); lint; commit** `feat(encounter): Route with the Line policy and Direct — an effect moves a creature, the beat says why`
- [ ] **Step 6: AGENTS.md** gains under "Where does this go?": *an effect that moves a creature → a directive: resolution describes it, `Route` finds the cells, `Direct` walks them; the beat carries the cause.* Pin spatial to the minted tag; push; draft PR. Expected tag `rulebooks/dnd5e/encounter/v0.72.0`.

---

## PR 3 — `rulebooks/dnd5e` root: the declarations and the profile

Branch `feat/thunderwave-profile`; title `feat(dnd5e): AreaBox, AreaOriginCasterEdge, CastMove, and the Thunderwave profile`. Pin `encounter` at PR 2's pseudo-version (the root module imports encounter for `CellsFromFeet`? check `go.mod`; if not, no pin needed here).

### Task 3.1: the shape and origin

**Files:** modify `combat/actions/area.go`, `area_test.go`

- [ ] **Step 1: failing tests:** `Footprint{Shape: AreaBox, SizeFeet: 15, Origin: AreaOriginCasterEdge}.Validate()` is nil; `Footprint{Shape: AreaBox, Origin: AreaOriginCaster}` is refused ("a box must be anchored on the caster's edge" — a centred box on the caster would cover the caster, which no spell says); `AreaRadius` with `AreaOriginCasterEdge` is refused.
- [ ] **Step 2: implement:** `const AreaBox AreaShape = "box"`; `const AreaOriginCasterEdge AreaOrigin = "caster-edge"`; extend both switches; add the shape/origin pairing rule to `Footprint.Validate`. Update the doc comments at `:19-25` (the chosen-point origin is still Fireball's; this one keeps the range check caster-centred).
- [ ] **Step 3: run, PASS; commit** `feat(actions): AreaBox and AreaOriginCasterEdge`

### Task 3.2: `CastMove`

**Files:** modify `combat/actions/cast.go`; create `combat/actions/move.go`, `move_test.go`

- [ ] **Step 1: failing tests:** `CastMove{Policy: MoveLine, Cells: 2}.Validate()` nil; `Cells: 0` refused; unknown policy refused; `CastProfile{...Move: &CastMove{...}}` without a `Save` is refused ("a move imposed on a failed save needs a save").
- [ ] **Step 2: implement**

```go
type MovePolicy string
const (
	MoveLine   MovePolicy = "line"
	MoveAway   MovePolicy = "away"
	MoveToward MovePolicy = "toward"
)
type MovePays string
const (
	PaysNothing  MovePays = ""          // zero value: the push
	PaysReaction MovePays = "reaction"
)
type CastMove struct {
	Policy   MovePolicy `json:"policy"`
	Cells    int        `json:"cells,omitempty"`  // fixed budget; 0 with Speed=true means the mover's speed
	Speed    bool       `json:"speed,omitempty"`
	Pays     MovePays   `json:"pays,omitempty"`
	Provokes bool       `json:"provokes,omitempty"`
}
func (m CastMove) Validate() error
```

and `CastProfile.Move *CastMove \`json:"move,omitempty"\`` with its `Validate` arm.

- [ ] **Step 3: run, PASS; commit** `feat(actions): CastMove — a cast may impose a move on a failed save`

### Task 3.3: the profile, the slot, the pick

**Files:** modify `spells/cast.go`, `spells/cast_test.go`, `character/choices/requirements.go`, its test

- [ ] **Step 1: failing tests:** `castContent[Thunderwave]` exists and its profile validates; `Options` of the bard's levelled pick contains `Thunderwave` and `Bane` (assert membership, not length); a level-1 bard with no slot is not offered it (mirror Bane's test).
- [ ] **Step 2: implement:** generalise `baneCost()` to `slotCost(level resources.ResourceKey) *combat.SpendProfile` and have Bane call it; add the profile from design §5 verbatim (`Cost: slotCost(resources.SpellSlotLevel1)`, `RangeFeet: 0`, `CastTargetArea`, `Area{Footprint{AreaBox, 15, AreaOriginCasterEdge}, AreaCatchesOthers}`, CON gate `Negated` with a comment naming #414, `Damage 2d8 Thunder`, `Move{MoveLine, Cells: 2}`); `Options: []spells.Spell{spells.Bane, spells.Thunderwave}`. Delete the leftover Thunderwave row in `spells/data.go:161` if the profile now carries name and description; if `data.go` is load-bearing for the catalogue, update it instead and say which in the commit.
- [ ] **Step 3: run the root module's tests; commit** `feat(spells): Thunderwave — the profile, a level-1 slot, and the bard's 1-of-2`
- [ ] **Step 4: push; draft PR.** Expected tag `rulebooks/dnd5e/v0.X` per the module's bump.

---

## PR 4 — `resolution`: `ImposedMove`

Branch `feat/imposed-move`; title `feat(resolution): ImposedMove — a failed save imposes the cast's move after its damage`. Pin the root module at PR 3's pseudo-version.

### Task 4.1

**Files:** modify `resolution/contest.go` (consts, struct, a producer beside `applyPreparedDamage`), `resolution/action.go` (thread `profile.Move` into the contest input), tests beside them

- [ ] **Step 1: failing test:** a gated cast whose profile declares `Move{MoveLine, Cells: 2}`; the saver fails; the imposed list contains, in order, `ImposedDamage` then `ImposedMove{Move: {Policy: line, Cells: 2, AnchorID: caster}}` with `RecipientID` the saver. A second test: the saver succeeds; no `ImposedMove`. A third: the saver is dropped by the damage; no `ImposedMove` ("the dropped are not pushed").
- [ ] **Step 2: implement:** `const ImposedMove ImposedEffectKind = "move"`; `ImposedEffect.Move *MoveDirective` where `MoveDirective{Policy, AnchorID, Cells, Speed, Pays, Provokes}` (mirrors `CastMove` plus the anchor); a `Gather` step `imposeMove` after the damage step on the failure branch (find where `applyPreparedDamage` is chained on failure — `contest.go` around `:655` — and chain after it), which checks the target still stands (the combatant's HP > 0 via the same accessor the damage step used) before appending.
- [ ] **Step 3: run, PASS; commit** `feat(resolution): ImposedMove after ImposedDamage on a failed save; the dropped are not pushed`
- [ ] **Step 4: push; draft PR.**

---

## PR 5 — `session`: `TargetCell`, the cell on the verb, the switch routed down, the result arm

Branch `feat/target-cell`; title `feat(session): TargetCell with its executor — the cell rides the cast, encounter covers, ImposedMove is walked`. Pin resolution and encounter at their pseudo-versions.

### Task 5.1: the kind and the offer

- [ ] **Step 1: failing test:** compiling offers for a bard whose profile has `AreaOriginCasterEdge` yields a declaration with `TargetKind: TargetCell` and empty candidates; Thunderclap's still says `TargetArea`.
- [ ] **Step 2: implement:** `TargetCell TargetKind = "cell"` in `types.go:2465-2491` with a comment in the same voice: *the caster names a cell the shape is aimed toward; it is a reference, not a calculation, and it rides the verb like a path does.* In `casts.go:207-217`, choose `TargetCell` when `profile.Area.Footprint.Origin == AreaOriginCasterEdge`.
- [ ] **Step 3: commit** `feat(session): TargetCell — the offer announces a cell is needed`

### Task 5.2: the cell on the verb, refused when wrong

- [ ] **Step 1: failing tests** (in `cast_test.go`, driving `Manager.Cast`): a `TargetCell` cast with `Cell == nil` is refused with `ErrBadCast` naming the missing cell; with the caster's own cell is refused; with any `Targets` is refused (as Area is today); a `TargetArea` cast with a `Cell` set is refused ("names no cell").
- [ ] **Step 2: implement:** `CastInput.Cell *spatial.Position` (`cast.go:24`); extend `castTargets` (`cast.go:437`) with the `TargetCell` arm; carry the cell to `deriveAreaMembers`.
- [ ] **Step 3: commit** `feat(session): the cell rides CastInput and is validated against the offer`

### Task 5.3: the shape switch leaves session

- [ ] **Step 1: failing test:** `deriveAreaMembers` for a `CasterEdge` box catches the member two cells ahead and not the one behind (build on the encounter fixture from PR 2, through the session's real `Cast` if the test harness allows; otherwise the derive function directly).
- [ ] **Step 2: implement:** in `area.go:104-120`, `case AreaOriginCasterEdge:` calls `enc.MembersCovered(&encounter.MembersCoveredInput{Footprint: spatial.Footprint{Box: &spatial.Box{W: feet, D: feet}}, Anchor: origin.Position, Toward: *cell, AtEdge: true})`; the `AreaRadius` arm stays for now (moving it to encounter is a one-line follow-up named in the PR body, not done here). The `Catches` filter and the `UnresolvedNoSheet` mapping stay in session: that is the sheet's business.
- [ ] **Step 3: commit** `feat(session): a caster-edge box is covered by encounter, not measured here`

### Task 5.4: the result arm and the walk

- [ ] **Step 1: failing test:** an `ImposedMove` maps to `encounter.ActivationResult{Kind: ResultMoved, Target, Move}`, and applying the cast's results moves the target (assert its cell changed and the story beat carries the spell ref as cause).
- [ ] **Step 2: implement:** `castoutcome.go:131` gains `case resolution.ImposedMove:` → `ResultMoved`. Where session applies `ResultDamageApplied` to the encounter (grep `ResultDamageApplied` in `session/*.go` non-test for the applier), add the `ResultMoved` arm: `route := enc.Route(...)` from the directive with `Anchor` = the caster's current cell, then `enc.Direct(ctx, DirectInput{Mover, Cause: spell.Ref, Route: route.Path, Provokes: move.Provokes})`, and put `Moved`/`StoppedBy` on the outcome for the wire.
- [ ] **Step 3: run the session module; commit** `feat(session): ImposedMove is routed and walked by encounter; the outcome says how far and what stopped it`
- [ ] **Step 4: push; draft PR;** pins to tags before ready.

---

## PR 6 — `rpg-api-protos`: the cell on the wire

Branch `feat/cast-cell`; title `feat(session): CastRequest.cell and TargetKind CELL`.

- [ ] `service.proto:579-592`: add `// The cell a caster-edge shape is aimed toward. Set only when the selected declaration's target_kind is CELL; a reference the engine reads, never a computed shape.` `Position cell = 6;`
- [ ] `types.proto:474-484`: add `TARGET_KIND_CELL = 5;` with a comment in the file's voice.
- [ ] Add `moved_cells` and `stopped_by` to the per-target cast outcome message if one exists (find where `Caught` is reported: `CastResponse` at `:594`); additive.
- [ ] `buf lint && buf format -d && buf breaking --against '.git#branch=main'` clean; generate; the generated Go compiles. No hand-written tests. Draft PR; tag on merge (`v0.1.186` or next).

## PR 7 — `rpg-api`: copy it through

Branch `feat/cast-cell` off `dev`. Pin protos at the minted tag and session at its tag (or pseudo-versions while drafting).

- [ ] `cast.go:40-71`: `Cell: positionFromProto(req.GetCell())` (the same converter `move.go:18-21` uses, returning `nil` for an unset message); `CastResponse` gains the moved fields if PR 6 added them.
- [ ] `internal/sandboxseed/sandboxseed.go:188`: the bard's `SpellRefs` becomes `[]string{baneRef, thunderwaveRef}` — **check the choice's `Count: 1`** first: if the seed picks one known spell per the pick, seed Thunderwave for the walk and say so in the manifest.
- [ ] Handler tests mirror the existing cast handler test with a cell. Draft PR to `dev`.

## PR 8 — `rpg-dnd5e-web`: the armed cast takes the ground click

Branch `feat/cast-cell` off `dev`. Regenerate the proto client.

- [ ] `useSessionCombatExperience.ts:657-673`: arming a `CAST` whose `targetKind === TargetKind.CELL` sets `armedDeclarationId` and does **not** fire (unlike AREA at `:1083-1093`).
- [ ] `SessionEncounterView.tsx:1699`: `onHexClick` becomes `handleGroundClick`: if a CELL cast is armed, send `Cast{declarationId, cell}` and disarm; otherwise `walkTo`. Entity clicks still take priority in `SessionCanvas`, so a click on a creature while armed is a no-op with the existing "pick a cell" hint text, not a misread.
- [ ] Render `moved_cells`/`stopped_by` in the cast's combat-log row if PR 6 carried them; otherwise the peer steps already animate.
- [ ] Focused tests for the routing; `npm run ci-check` once at PR time (grep its log; its exit code lies). Draft PR to `dev`.

---

## The walk

`game-dev/envs/local/thunderwave.env` (ports unclaimed by any other manifest: check `envs/local/*.env` and `ss -ltn` first), `RPG_API_PATH` at an rpg-api worktree pinned to the toolkit pseudo-versions (session, resolution, encounter, root, spatial) plus PR 7's handler, `RPG_DND5E_WEB_PATH` at PR 8's worktree. Seed the bard with Thunderwave.

In the reference tomb: stand with two skeletons ahead, the pillar behind one. Arm Thunderwave, click a cell past the far skeleton. Check, in order: both skeletons were caught and the near ally was not; both saves rolled; one 2d8 shown once; the open skeleton slid two cells; the pillar skeleton slid one and the story names the pillar; no opportunity attack; the beat's cause is Thunderwave. Record the walk on #430 and #432.

## Self-review against the design

- §3 coverage: PR 1 (embedding, box, edge anchor, fractions), PR 2 (`MembersCovered`, threshold). Unit ruling stated once above.
- §4 target: PR 5.1–5.3, PR 6, PR 7, PR 8. Range check untouched. No preview.
- §5 cast: PR 3 (profile, slot, pick, `CastMove`), PR 4 (imposed after damage; dropped not pushed), PR 5.4 (walked by encounter through `Direct`).
- §6 acceptance: 1 → PR 1/2 tests; 2 → PR 5.3; 3 → PR 5.2; 4 → existing Thunderclap tests plus PR 4; 5–7 → PR 2.3 and PR 5.4; 8–10 → PR 3 and the regression runs in every module.
- Types consistent across PRs: `spatial.Footprint/Box/Coverage/HexEmbedding`, `encounter.MembersCovered/CoverageThreshold/Route/Direct/MoveLine`, `actions.AreaBox/AreaOriginCasterEdge/CastMove/MovePolicy`, `resolution.ImposedMove/MoveDirective`, `session.TargetCell/CastInput.Cell/ResultMoved`.
- Known thin spots carried from design §10: the hexagon-under-rectangle clip reuses `clipHalfPlane`; the `Line` continuation via `GetLineOfSight` at off-axis bearings is asserted by the PR 2 tests at one bearing only, and the walk's skeleton is the second.

— cross-team agent, on behalf of KirkDiggler
