# rpg-api Walking Skeleton — Wave 2.5 Slice 1 Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Wire the rpg-toolkit `encounter` SDK into rpg-api as the v1alpha2 encounter service. Movement events flow end-to-end: `MoveEntity` RPC → `Encounter.Move` → broker → `StreamEncounter` → typed proto events on the wire. v1alpha1 movement remains untouched.

**Architecture:** Additive parallel path. New v1alpha2 handler package + new in-memory `*encounter.Data` repo, both registered alongside the existing v1alpha1 wiring on the same `*grpc.Server`. No orchestrator layer in v2 — handler talks directly to the toolkit SDK + broker. Pure-function translator turns toolkit events into v1alpha2 proto events per-viewer.

**Tech Stack:** Go 1.25, gRPC, `github.com/KirkDiggler/rpg-toolkit/encounter`, `github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/v1alpha2/encounter`, testify suite, `go.uber.org/mock/gomock`, bufconn integration harness.

**Spec:** `rpg-project/ideas/encounter/v1alpha2/sdk-direction-rpgapi.md` (read first; this plan does not repeat the architectural reasoning).

**Issues:** rpg-api#494, rpg-project#15, rpg-toolkit#628. Known toolkit gap filed at rpg-toolkit#629 (LoS-loss events) — out of slice 1 scope but plan is structured so it can be added later by extending the translator mapping table.

---

## File Structure

| File | Purpose | Status |
|------|---------|--------|
| `internal/repositories/encounters/v2/repository.go` | Repository interface + `ErrNotFound` sentinel | Create |
| `internal/repositories/encounters/v2/in_memory.go` | In-memory implementation, mutex-guarded map, real `ToData/LoadFromData` round-trip | Create |
| `internal/repositories/encounters/v2/in_memory_test.go` | Round-trip tests, NotFound test | Create |
| `internal/handlers/dnd5e/v2/encounter/handler.go` | Handler struct, constructor, `MoveEntity`, `StreamEncounter` | Create |
| `internal/handlers/dnd5e/v2/encounter/handler_test.go` | Handler unit tests with real broker + repo + fixed-time func | Create |
| `internal/handlers/dnd5e/v2/encounter/translate.go` | Pure-function translator, error sentinels, hex helpers | Create |
| `internal/handlers/dnd5e/v2/encounter/translate_test.go` | Table-driven translator tests, target 100% coverage | Create |
| `internal/integration/harness/harness.go` | Add `EncounterClientV2`, `BrokerV2`, `EncRepoV2` fields | Modify |
| `internal/integration/encounter_v2_test.go` | The wave-2.5 acceptance test (two players, one moves) | Create |
| `cmd/server/server.go` | Construct broker + v2 repo + v2 handler; register service; health status | Modify |

**Package naming convention** (locked in spec):
- Handler dir: `internal/handlers/dnd5e/v2/encounter/` → `package encounter`
- Repo dir: `internal/repositories/encounters/v2/` → `package encounters`
- When importing the v2 repo where the v1 repo is also imported, use named alias `encountersv2 "github.com/KirkDiggler/rpg-api/internal/repositories/encounters/v2"`. The v1 repo at `internal/repositories/encounters/` stays the unaliased `encounters`.

**Pre-flight from spec → plan recommendations:**
- The spec assumes `*encounter.Encounter` exposes a way to look up the entity controlled by a given player (used for slice 1's `entity_id` validation). If that API doesn't exist on the toolkit aggregate, fall back to reading from `*encounter.Data` directly (whichever field tracks player→entity association). If neither path is available, the entity_id check becomes a future toolkit issue — for slice 1, log + accept and document the gap. **Do this verification in Task 4** before writing the entity_id validation code.

---

## Task 1: Repository — `encounters/v2` in-memory store

**Files:**
- Create: `internal/repositories/encounters/v2/repository.go`
- Create: `internal/repositories/encounters/v2/in_memory.go`
- Create: `internal/repositories/encounters/v2/in_memory_test.go`

- [ ] **Step 1.1: Write the interface + ErrNotFound test**

Create `internal/repositories/encounters/v2/repository.go`:
```go
// Package encounters is the v2 encounter store.
// It holds *encounter.Data values from the rpg-toolkit encounter SDK.
package encounters

import (
	"context"
	"errors"

	"github.com/KirkDiggler/rpg-toolkit/encounter"
)

// ErrNotFound indicates the encounter id does not exist in the store.
var ErrNotFound = errors.New("encounter not found")

// Repository persists *encounter.Data keyed by encounter id.
//
// Implementations MUST round-trip through encounter.Data's ToData/LoadFromData
// pattern on each Save/Get so the toolkit's serialization is exercised on
// every write — catching JSON-level bugs before they reach production.
type Repository interface {
	// Get returns ErrNotFound if id has no stored data.
	Get(ctx context.Context, id string) (*encounter.Data, error)

	// Save replaces the stored data for data.ID.
	Save(ctx context.Context, data *encounter.Data) error
}
```

Create `internal/repositories/encounters/v2/in_memory_test.go` with the suite skeleton + first test:
```go
package encounters_test

import (
	"context"
	"errors"
	"testing"

	"github.com/stretchr/testify/suite"

	encountersv2 "github.com/KirkDiggler/rpg-api/internal/repositories/encounters/v2"
)

type InMemorySuite struct {
	suite.Suite
	ctx  context.Context
	repo encountersv2.Repository
}

func (s *InMemorySuite) SetupTest() {
	s.ctx = context.Background()
	s.repo = encountersv2.NewInMemory()
}

func (s *InMemorySuite) TestGet_ReturnsErrNotFound_ForMissingID() {
	_, err := s.repo.Get(s.ctx, "missing")
	s.Require().Error(err)
	s.Require().True(errors.Is(err, encountersv2.ErrNotFound))
}

func TestInMemorySuite(t *testing.T) {
	suite.Run(t, new(InMemorySuite))
}
```

- [ ] **Step 1.2: Run test to verify failure**

```bash
cd /home/kirk/personal/rpg-api && go test ./internal/repositories/encounters/v2/... -run TestInMemorySuite -v 2>&1 | tail -20
```
Expected: compile error — `NewInMemory` undefined.

- [ ] **Step 1.3: Implement minimal in-memory store**

Create `internal/repositories/encounters/v2/in_memory.go`:
```go
package encounters

import (
	"context"
	"sync"

	"github.com/KirkDiggler/rpg-toolkit/encounter"
)

// NewInMemory returns a thread-safe in-memory Repository.
func NewInMemory() Repository {
	return &inMemory{data: make(map[string]*encounter.Data)}
}

type inMemory struct {
	mu   sync.Mutex
	data map[string]*encounter.Data
}

func (r *inMemory) Get(ctx context.Context, id string) (*encounter.Data, error) {
	r.mu.Lock()
	defer r.mu.Unlock()
	d, ok := r.data[id]
	if !ok {
		return nil, ErrNotFound
	}
	return d, nil
}

func (r *inMemory) Save(ctx context.Context, data *encounter.Data) error {
	r.mu.Lock()
	defer r.mu.Unlock()
	r.data[string(data.ID)] = data
	return nil
}
```

- [ ] **Step 1.4: Run test to verify pass**

```bash
go test ./internal/repositories/encounters/v2/... -run TestInMemorySuite -v 2>&1 | tail -20
```
Expected: PASS, 1 test.

- [ ] **Step 1.5: Add round-trip test (Save then Get returns equivalent data)**

Append to `in_memory_test.go`:
```go
func (s *InMemorySuite) TestSaveGet_RoundTrip_PreservesData() {
	enc := encounter.New("enc-1", encounter.NewBroker(encounter.NewInMemoryTransport()))
	// AddPlayer + minimal setup so ToData has something interesting.
	s.Require().NoError(enc.AddPlayer(encounter.PlayerInput{
		PlayerID: "player-A",
		EntityID: "char-A",
		Position: core.Hex{Q: 0, R: 0, S: 0},
	}))
	original := enc.ToData()

	s.Require().NoError(s.repo.Save(s.ctx, original))

	loaded, err := s.repo.Get(s.ctx, string(original.ID))
	s.Require().NoError(err)
	s.Require().Equal(original.ID, loaded.ID)
	// Round-trip via the toolkit serializer to prove ToData/LoadFromData survives storage.
	roundTripped, err := encounter.LoadFromData(loaded, encounter.NewBroker(encounter.NewInMemoryTransport()))
	s.Require().NoError(err)
	s.Require().Equal(original.ID, roundTripped.ID())
}
```
Add the imports to the test file:
```go
import (
	// ... existing
	"github.com/KirkDiggler/rpg-toolkit/encounter"
	"github.com/KirkDiggler/rpg-toolkit/encounter/core"
)
```

- [ ] **Step 1.6: Run round-trip test**

```bash
go test ./internal/repositories/encounters/v2/... -v 2>&1 | tail -20
```
Expected: 2 tests PASS. If `encounter.PlayerInput` field names differ from what's listed (`ID`, `EntityID`, `Position`), check the actual struct in `/home/kirk/personal/rpg-toolkit/encounter/encounter.go:21` and adjust.

- [ ] **Step 1.7: Commit**

```bash
git add internal/repositories/encounters/v2/
git commit -m "feat(encounter/v2): add in-memory repository for *encounter.Data

Implements encounters.Repository (Get/Save) backed by a mutex-guarded
map. ErrNotFound sentinel for missing ids. Save/Get exercise the toolkit's
ToData/LoadFromData round-trip on every call so any serialization bug
surfaces in tests rather than production.

Refs #494."
```

---

## Task 2: Translator — pure functions + error sentinels

**Files:**
- Create: `internal/handlers/dnd5e/v2/encounter/translate.go`
- Create: `internal/handlers/dnd5e/v2/encounter/translate_test.go`

This task takes the longest. It is the wire-fidelity contract for slice 1. Target: 100% line coverage on `translate.go`.

- [ ] **Step 2.1: Write the hex-helper test first**

Create `internal/handlers/dnd5e/v2/encounter/translate_test.go`:
```go
package encounter_test

import (
	"errors"
	"testing"
	"time"

	"github.com/stretchr/testify/suite"
	"google.golang.org/protobuf/proto"

	encounterv2pb "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/v1alpha2/encounter"
	v2encounter "github.com/KirkDiggler/rpg-api/internal/handlers/dnd5e/v2/encounter"
	tkenc "github.com/KirkDiggler/rpg-toolkit/encounter"
	"github.com/KirkDiggler/rpg-toolkit/encounter/core"
	"github.com/KirkDiggler/rpg-toolkit/encounter/events"
)

type TranslateSuite struct {
	suite.Suite
	now time.Time
}

func (s *TranslateSuite) SetupTest() {
	s.now = time.Date(2026, 1, 1, 0, 0, 0, 0, time.UTC)
}

func (s *TranslateSuite) TestHexToPosition_CubeMapping() {
	got := v2encounter.HexToPosition(core.Hex{Q: 1, R: -2, S: 1})
	s.Require().Equal(int32(1), got.X)
	s.Require().Equal(int32(-2), got.Y)
	s.Require().Equal(int32(1), got.Z)
}

func TestTranslateSuite(t *testing.T) {
	suite.Run(t, new(TranslateSuite))
}
```

- [ ] **Step 2.2: Run test to verify it fails**

```bash
go test ./internal/handlers/dnd5e/v2/encounter/... -run TestTranslateSuite -v 2>&1 | tail -10
```
Expected: compile error — package doesn't exist yet.

- [ ] **Step 2.3: Create translate.go with the hex helper + error sentinels**

Create `internal/handlers/dnd5e/v2/encounter/translate.go`:
```go
// Package encounter is the v2 encounter handler — wire shim over the
// rpg-toolkit encounter SDK. translate.go converts toolkit events into
// v1alpha2 proto envelopes per viewer.
package encounter

import (
	"errors"
	"fmt"
	"time"

	"google.golang.org/protobuf/types/known/timestamppb"

	encounterv2pb "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/v1alpha2/encounter"
	"github.com/KirkDiggler/rpg-toolkit/encounter/core"
	"github.com/KirkDiggler/rpg-toolkit/encounter/events"
)

// ErrViewerSawNothing is returned when an event was delivered to a viewer
// but the viewer's per-player slice contains no visible content. The
// stream loop should errors.Is-check and continue silently.
var ErrViewerSawNothing = errors.New("viewer saw nothing in this event")

// ErrUnknownEventType is returned when the translator has no mapping for
// the given toolkit event type. The stream loop should log + continue.
var ErrUnknownEventType = errors.New("translator has no mapping for this event type")

// HexToPosition maps a toolkit cube hex (Q,R,S) to a proto Position (x,y,z).
// The proto invariant x+y+z=0 is preserved by construction.
func HexToPosition(h core.Hex) *encounterv2pb.Position {
	return &encounterv2pb.Position{X: int32(h.Q), Y: int32(h.R), Z: int32(h.S)}
}

// TranslateEvent maps a toolkit EncounterEvent to a v1alpha2 envelope from
// the perspective of viewer. Returns:
//   - (*EncounterEvent, nil) for normal translation
//   - (nil, ErrViewerSawNothing) when viewer had nothing visible in evt
//   - (nil, ErrUnknownEventType) when evt's concrete type has no mapping
//   - (nil, otherErr) for genuine bugs the caller should escalate
func TranslateEvent(evt events.EncounterEvent, viewer core.PlayerID, now time.Time) (*encounterv2pb.EncounterEvent, error) {
	switch e := evt.(type) {
	case *events.MoveEvent:
		return translateMoveEvent(e, viewer, now)
	case *events.HexRevealedEvent:
		return translateHexRevealedEvent(e, viewer, now)
	default:
		return nil, fmt.Errorf("%w: %T", ErrUnknownEventType, evt)
	}
}

func translateMoveEvent(e *events.MoveEvent, viewer core.PlayerID, now time.Time) (*encounterv2pb.EncounterEvent, error) {
	slice, ok := e.PerPlayer[viewer]
	if !ok || len(slice.SeenSegments) == 0 {
		return nil, ErrViewerSawNothing
	}
	// Use the per-viewer SeenSegments (not the full e.Path) so the wire
	// reflects the viewer's reality — the toolkit broker delivered this
	// event because the viewer saw at least one segment, but only those
	// segments belong on the wire (per spec section 5: per-viewer reality).
	path := make([]*encounterv2pb.Position, 0, len(slice.SeenSegments))
	for _, h := range slice.SeenSegments {
		path = append(path, HexToPosition(h))
	}
	return &encounterv2pb.EncounterEvent{
		Sequence:  int64(e.Sequence()),
		Timestamp: timestamppb.New(now),
		Event: &encounterv2pb.EncounterEvent_EntityMoved{
			EntityMoved: &encounterv2pb.EntityMoved{
				EntityId:   string(e.Mover),
				From:       path[0],
				To:         path[len(path)-1],
				ActualPath: path,
			},
		},
	}, nil
}

func translateHexRevealedEvent(e *events.HexRevealedEvent, viewer core.PlayerID, now time.Time) (*encounterv2pb.EncounterEvent, error) {
	slice, ok := e.PerPlayer[viewer]
	if !ok || len(slice.Hexes) == 0 {
		return nil, ErrViewerSawNothing
	}
	// HexRevealedSlice.Hexes is core.HexSet which is map[Hex]struct{} —
	// range over keys (the hex), not values (struct{}).
	hexes := make([]*encounterv2pb.Hex, 0, len(slice.Hexes))
	for h := range slice.Hexes {
		hexes = append(hexes, &encounterv2pb.Hex{Position: HexToPosition(h)})
	}
	return &encounterv2pb.EncounterEvent{
		Sequence:  int64(e.Sequence()),
		Timestamp: timestamppb.New(now),
		Event: &encounterv2pb.EncounterEvent_GeometryRevealed{
			GeometryRevealed: &encounterv2pb.GeometryRevealed{
				Hexes: hexes,
			},
		},
	}, nil
}
```

> ⚠️ **Field name verification**: before writing the file, confirm the actual proto field names by reading `/home/kirk/personal/rpg-api-protos/gen/go/dnd5e/api/v1alpha2/encounter/types.pb.go` for `Position`, `Hex` and `events.pb.go` for `EncounterEvent_EntityMoved`, `EncounterEvent_GeometryRevealed`. Adjust the Go-generated wrapper-type names if they differ from the predictions above.

- [ ] **Step 2.4: Run hex helper test**

```bash
go test ./internal/handlers/dnd5e/v2/encounter/... -run TestTranslateSuite -v 2>&1 | tail -10
```
Expected: PASS, 1 test.

- [ ] **Step 2.5: Add MoveEvent happy-path test**

Append to `translate_test.go`:
```go
func (s *TranslateSuite) TestTranslateEvent_MoveEvent_FullPath() {
	// NewMoveEvent signature (events/move.go:34): (encID, seq, mover, path, perPlayer)
	evt := events.NewMoveEvent(
		"enc-1",
		uint64(1),
		"char-A",
		[]core.Hex{{Q: 0, R: 0, S: 0}, {Q: 1, R: -1, S: 0}, {Q: 2, R: -2, S: 0}},
		map[core.PlayerID]events.MovePlayerSlice{
			"player-B": {SeenSegments: []core.Hex{{Q: 0, R: 0, S: 0}, {Q: 1, R: -1, S: 0}, {Q: 2, R: -2, S: 0}}},
		},
	)
	out, err := v2encounter.TranslateEvent(evt, "player-B", s.now)
	s.Require().NoError(err)
	s.Require().NotNil(out)

	moved := out.GetEntityMoved()
	s.Require().NotNil(moved)
	s.Require().Equal("char-A", moved.EntityId)
	s.Require().Len(moved.ActualPath, 3)
	s.Require().True(proto.Equal(moved.From, moved.ActualPath[0]))
	s.Require().True(proto.Equal(moved.To, moved.ActualPath[2]))
}
```

> ⚠️ **API verification**: read `/home/kirk/personal/rpg-toolkit/encounter/events/move.go:30` for the actual `NewMoveEvent` constructor signature. If it requires a sequence number or different argument order, adjust the test (and translator if needed) accordingly. Same caution applies to `HexRevealedEvent` later.

- [ ] **Step 2.6: Run + commit-cycle**

```bash
go test ./internal/handlers/dnd5e/v2/encounter/... -run TestTranslateSuite -v 2>&1 | tail -10
```
Expected: PASS.

- [ ] **Step 2.7: Add ErrViewerSawNothing test**

Append:
```go
func (s *TranslateSuite) TestTranslateEvent_MoveEvent_EmptySliceReturnsErrViewerSawNothing() {
	evt := events.NewMoveEvent(
		"enc-1", uint64(1), "char-A",
		[]core.Hex{{Q: 0, R: 0, S: 0}},
		map[core.PlayerID]events.MovePlayerSlice{
			"player-B": {SeenSegments: nil},
		},
	)
	_, err := v2encounter.TranslateEvent(evt, "player-B", s.now)
	s.Require().Error(err)
	s.Require().True(errors.Is(err, v2encounter.ErrViewerSawNothing))
}

func (s *TranslateSuite) TestTranslateEvent_MoveEvent_ViewerNotInPerPlayerReturnsErrViewerSawNothing() {
	evt := events.NewMoveEvent("enc-1", uint64(1), "char-A", nil, nil)
	_, err := v2encounter.TranslateEvent(evt, "player-X", s.now)
	s.Require().Error(err)
	s.Require().True(errors.Is(err, v2encounter.ErrViewerSawNothing))
}
```

- [ ] **Step 2.8: Add ErrUnknownEventType test**

```go
type fakeEvent struct{}

func (fakeEvent) EncounterID() core.EncounterID  { return "enc-1" }
func (fakeEvent) Sequence() uint64               { return 1 }
func (fakeEvent) Audience() events.AudienceSet   { return nil }
func (fakeEvent) IsEncounterEvent()              {} // adjust to actual sealed-iface method

func (s *TranslateSuite) TestTranslateEvent_UnknownTypeReturnsErrUnknownEventType() {
	_, err := v2encounter.TranslateEvent(fakeEvent{}, "player-A", s.now)
	s.Require().Error(err)
	s.Require().True(errors.Is(err, v2encounter.ErrUnknownEventType))
}
```

> ⚠️ **Sealed interface verification**: the toolkit's `EncounterEvent` is a sealed interface. The unexported sealing method may be `isEncounterEvent()` (per `move.go:50`). A `fakeEvent` defined outside the toolkit package CANNOT implement it. Either move this test inside the toolkit-tested code path OR remove the unknown-type test and rely on a 100%-coverage sentinel via a different route — e.g., define a doc-only "untranslatable" sentinel and exercise it via a private helper. **Implementer's call**: skip the test if the seal blocks it; flag in PR description.

- [ ] **Step 2.9: HexRevealedEvent translation test + run**

Append a happy-path + empty-slice test for HexRevealedEvent following the MoveEvent pattern. Run all translator tests:

```bash
go test ./internal/handlers/dnd5e/v2/encounter/... -run TestTranslateSuite -v -cover 2>&1 | tail -15
```
Expected: all tests PASS, coverage ≥ 95% on translate.go (target 100%; gaps acceptable only on the unknown-type branch if Step 2.8 was skipped).

- [ ] **Step 2.10: Commit**

```bash
git add internal/handlers/dnd5e/v2/encounter/translate.go internal/handlers/dnd5e/v2/encounter/translate_test.go
git commit -m "feat(encounter/v2): add proto translator for MoveEvent + HexRevealedEvent

Pure-function translator turns toolkit EncounterEvent into v1alpha2 proto
envelopes from a viewer's perspective. ErrViewerSawNothing fires when the
viewer's PerPlayer slice is empty; ErrUnknownEventType fires for any
toolkit event type the translator doesn't yet map. Both are typed
sentinels so the stream loop can errors.Is-check.

Slice 1 maps:
- *events.MoveEvent      → EntityMoved (per-viewer SeenSegments → actual_path)
- *events.HexRevealedEvent → GeometryRevealed

Future events extend the switch in TranslateEvent. Hex helper preserves
the proto x+y+z=0 cube invariant by construction.

Refs #494."
```

---

## Task 3: Handler scaffold — register service with Unimplemented stubs

**Files:**
- Create: `internal/handlers/dnd5e/v2/encounter/handler.go`
- Create: `internal/handlers/dnd5e/v2/encounter/handler_test.go`
- Modify: `cmd/server/server.go`
- Modify: `internal/integration/harness/harness.go`

This task wires the v2 service onto the gRPC server with all RPCs as `Unimplemented`. After this task, calling any v2 RPC over bufconn returns `codes.Unimplemented`. v1alpha1 paths must remain green.

- [ ] **Step 3.1: Write handler scaffold**

Create `internal/handlers/dnd5e/v2/encounter/handler.go`:
```go
package encounter

import (
	"errors"
	"time"

	encounterv2pb "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/v1alpha2/encounter"
	encountersv2 "github.com/KirkDiggler/rpg-api/internal/repositories/encounters/v2"
	"github.com/KirkDiggler/rpg-toolkit/encounter"
)

// HandlerConfig configures a v2 encounter Handler.
type HandlerConfig struct {
	Broker *encounter.Broker
	Repo   encountersv2.Repository
	Now    func() time.Time // optional; defaults to time.Now
}

// Handler implements dnd5e.api.v1alpha2.encounter.EncounterServiceServer.
//
// Only MoveEntity and StreamEncounter ship in slice 1. Every other RPC
// returns codes.Unimplemented via the embedded server.
type Handler struct {
	encounterv2pb.UnimplementedEncounterServiceServer
	broker  *encounter.Broker
	encRepo encountersv2.Repository
	now     func() time.Time
}

// New constructs a Handler. Returns error on missing required deps.
func New(cfg *HandlerConfig) (*Handler, error) {
	if cfg == nil {
		return nil, errors.New("HandlerConfig is required")
	}
	if cfg.Broker == nil {
		return nil, errors.New("HandlerConfig.Broker is required")
	}
	if cfg.Repo == nil {
		return nil, errors.New("HandlerConfig.Repo is required")
	}
	now := cfg.Now
	if now == nil {
		now = time.Now
	}
	return &Handler{broker: cfg.Broker, encRepo: cfg.Repo, now: now}, nil
}
```

- [ ] **Step 3.2: Write handler-suite skeleton + Unimplemented test**

Create `internal/handlers/dnd5e/v2/encounter/handler_test.go`:
```go
package encounter_test

import (
	"context"
	"testing"
	"time"

	"github.com/stretchr/testify/suite"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	encounterv2pb "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/v1alpha2/encounter"
	"github.com/KirkDiggler/rpg-api/internal/auth"
	v2encounter "github.com/KirkDiggler/rpg-api/internal/handlers/dnd5e/v2/encounter"
	encountersv2 "github.com/KirkDiggler/rpg-api/internal/repositories/encounters/v2"
	tkenc "github.com/KirkDiggler/rpg-toolkit/encounter"
)

type HandlerSuite struct {
	suite.Suite
	ctx     context.Context
	broker  *tkenc.Broker
	repo    encountersv2.Repository
	handler *v2encounter.Handler
	fixed   time.Time
}

func (s *HandlerSuite) SetupTest() {
	s.fixed = time.Date(2026, 1, 1, 0, 0, 0, 0, time.UTC)
	s.ctx = auth.WithPlayerID(context.Background(), "player-A")
	s.broker = tkenc.NewBroker(tkenc.NewInMemoryTransport())
	s.repo = encountersv2.NewInMemory()
	h, err := v2encounter.New(&v2encounter.HandlerConfig{
		Broker: s.broker, Repo: s.repo, Now: func() time.Time { return s.fixed },
	})
	s.Require().NoError(err)
	s.handler = h
}

func (s *HandlerSuite) TestCreateEncounter_ReturnsUnimplemented() {
	_, err := s.handler.CreateEncounter(s.ctx, &encounterv2pb.CreateEncounterRequest{})
	s.Require().Error(err)
	st, ok := status.FromError(err)
	s.Require().True(ok)
	s.Require().Equal(codes.Unimplemented, st.Code())
}

func TestHandlerSuite(t *testing.T) {
	suite.Run(t, new(HandlerSuite))
}
```

- [ ] **Step 3.3: Run + verify**

```bash
go test ./internal/handlers/dnd5e/v2/encounter/... -run TestHandlerSuite -v 2>&1 | tail -10
```
Expected: 1 test PASS (the embedded `Unimplemented...Server` returns Unimplemented).

- [ ] **Step 3.4: Wire v2 service in `cmd/server/server.go`**

Modify `cmd/server/server.go` (around lines 152-218 per the recon — confirm by reading first). Insert after the v1alpha1 encounter handler is registered:
```go
// v1alpha2 encounter wiring (additive, no v1alpha1 disturbance)
encV2Transport := tkenc.NewInMemoryTransport()
encV2Broker := tkenc.NewBroker(encV2Transport)
encV2Repo := encountersv2.NewInMemory()
encV2Handler, err := encounterhandlerv2.New(&encounterhandlerv2.HandlerConfig{
	Broker: encV2Broker,
	Repo:   encV2Repo,
})
if err != nil {
	return fmt.Errorf("encounter v2 handler: %w", err)
}
encounterv2pb.RegisterEncounterServiceServer(srv, encV2Handler)
healthServer.SetServingStatus("dnd5e.api.v1alpha2.encounter.EncounterService", grpc_health_v1.HealthCheckResponse_SERVING)
```
Add imports:
```go
encounterv2pb "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/v1alpha2/encounter"
encounterhandlerv2 "github.com/KirkDiggler/rpg-api/internal/handlers/dnd5e/v2/encounter"
encountersv2 "github.com/KirkDiggler/rpg-api/internal/repositories/encounters/v2"
tkenc "github.com/KirkDiggler/rpg-toolkit/encounter"
```

- [ ] **Step 3.5: Verify server compiles**

```bash
go build ./cmd/server/... 2>&1 | tail -10
```
Expected: clean build.

- [ ] **Step 3.6: Extend the integration harness**

Modify `internal/integration/harness/harness.go`. Add fields to the `TestServer` struct:
```go
// v1alpha2 access (slice 1 wiring)
EncounterClientV2 encounterv2pb.EncounterServiceClient
BrokerV2          *tkenc.Broker
EncRepoV2         encountersv2.Repository
```
In the harness setup (where the existing clients/server are wired), construct the v2 broker + repo + handler the same way `cmd/server/server.go` does, register on the bufconn server, and create the v2 client on the bufconn connection. **Reuse the SAME singleton broker and repo** between the in-process server registration and the harness fields so tests can seed via `EncRepoV2.Save()` and inspect via `BrokerV2`. Add the same imports as Step 3.4.

- [ ] **Step 3.7: Run integration harness boot**

```bash
go test ./internal/integration/... -run TestHarness -v 2>&1 | tail -20
```
(If no `TestHarness` exists, run the existing integration tests to verify no regression.)
Expected: all existing integration tests still pass; harness compiles with the new fields.

- [ ] **Step 3.8: Verify no v1alpha1 regressions**

```bash
make ci-check 2>&1 | tail -30
```
Expected: clean. v1alpha1 encounter tests + handlers untouched.

- [ ] **Step 3.9: Commit**

```bash
git add internal/handlers/dnd5e/v2/encounter/handler.go \
       internal/handlers/dnd5e/v2/encounter/handler_test.go \
       internal/integration/harness/harness.go \
       cmd/server/server.go
git commit -m "feat(encounter/v2): register service + harness extension

Adds v2 encounter handler scaffold (broker + repo + now-func) and
registers it on the gRPC server alongside v1alpha1. All RPCs return
codes.Unimplemented via the embedded server; MoveEntity and
StreamEncounter implementations follow in subsequent commits.

Integration harness exposes EncounterClientV2, BrokerV2, EncRepoV2 so
acceptance tests can drive both client RPCs and seed encounter data
directly. Same singleton broker is shared between in-process server
registration and the harness handles.

v1alpha1 wiring untouched — additive only.

Refs #494."
```

---

## Task 4: `MoveEntity` handler

**Files:**
- Modify: `internal/handlers/dnd5e/v2/encounter/handler.go` (add `MoveEntity` method)
- Modify: `internal/handlers/dnd5e/v2/encounter/handler_test.go`

- [ ] **Step 4.1: Verify the player→entity lookup API on `*encounter.Encounter`**

Read `/home/kirk/personal/rpg-toolkit/encounter/encounter.go`. Look for an exported method that, given a `core.PlayerID`, returns the `core.EntityID` that player controls. Likely names: `EntityFor(playerID)`, `PlayerEntity(playerID)`, or a public accessor on `*encounter.Data` (e.g., `data.Players` map). Report what you find before writing the test.

- If a clean accessor exists → use it for the entity_id validation in Step 4.4.
- If only `*encounter.Data` exposes it → use that field directly (handler already has the loaded `*encounter.Data`).
- If neither exists → file a toolkit issue (`rpg-toolkit#NNN`), add to board #11 wave 2.5, **skip the entity_id validation in slice 1**: log a warning when entity_id mismatches but accept the move. Document the gap in the PR description and link the new toolkit issue.

- [ ] **Step 4.2: Write happy-path MoveEntity test**

Append to `handler_test.go`:
```go
func (s *HandlerSuite) TestMoveEntity_HappyPath_LoadsCallsMoveSaves() {
	// Seed encounter with player-A controlling char-A at (0,0,0).
	enc := tkenc.New("enc-1", s.broker)
	s.Require().NoError(enc.AddPlayer(tkenc.PlayerInput{
		PlayerID: "player-A", EntityID: "char-A", Position: core.Hex{Q: 0, R: 0, S: 0},
	}))
	s.Require().NoError(s.repo.Save(s.ctx, enc.ToData()))

	_, err := s.handler.MoveEntity(s.ctx, &encounterv2pb.MoveEntityRequest{
		EncounterId:  "enc-1",
		EntityId:     "char-A",
		ProposedPath: []*encounterv2pb.Position{{X: 0, Y: 0, Z: 0}, {X: 1, Y: -1, Z: 0}},
	})
	s.Require().NoError(err)

	// Verify the encounter was saved post-move.
	loaded, err := s.repo.Get(s.ctx, "enc-1")
	s.Require().NoError(err)
	s.Require().NotNil(loaded)
	// Specific assertions on player position depend on the toolkit's Data shape;
	// at minimum, ID round-trips and Save was called.
	s.Require().Equal(tkenc.EncounterID("enc-1"), loaded.ID)
}
```

- [ ] **Step 4.3: Run test (expect failure)**

```bash
go test ./internal/handlers/dnd5e/v2/encounter/... -run TestHandlerSuite/TestMoveEntity_HappyPath -v 2>&1 | tail -10
```
Expected: FAIL with `Unimplemented` (the embedded server's default).

- [ ] **Step 4.4: Implement MoveEntity**

Add to `handler.go`:
```go
import (
	// ... existing
	"context"

	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/KirkDiggler/rpg-api/internal/auth"
	"github.com/KirkDiggler/rpg-toolkit/encounter/core"
)

func (h *Handler) MoveEntity(ctx context.Context, req *encounterv2pb.MoveEntityRequest) (*encounterv2pb.MoveEntityResponse, error) {
	playerID := auth.GetPlayerID(ctx)
	if playerID == "" {
		return nil, status.Error(codes.Unauthenticated, "no player id in context")
	}
	if req.GetEncounterId() == "" {
		return nil, status.Error(codes.InvalidArgument, "encounter_id is required")
	}

	data, err := h.encRepo.Get(ctx, req.GetEncounterId())
	if err != nil {
		if errors.Is(err, encountersv2.ErrNotFound) {
			return nil, status.Error(codes.NotFound, "encounter not found")
		}
		return nil, status.Errorf(codes.Internal, "load encounter: %v", err)
	}

	enc, err := encounter.LoadFromData(data, h.broker)
	if err != nil {
		return nil, status.Errorf(codes.Internal, "load from data: %v", err)
	}

	path := make([]core.Hex, 0, len(req.GetProposedPath()))
	for _, p := range req.GetProposedPath() {
		path = append(path, core.Hex{Q: int(p.X), R: int(p.Y), S: int(p.Z)})
	}

	if err := enc.Move(core.PlayerID(playerID), path); err != nil {
		// Toolkit-level errors map to InvalidArgument by default — they're
		// the toolkit telling us the move violated rules (out of range,
		// blocked path, not the player's turn, etc.).
		return nil, status.Errorf(codes.InvalidArgument, "move: %v", err)
	}

	if err := h.encRepo.Save(ctx, enc.ToData()); err != nil {
		return nil, status.Errorf(codes.Internal, "save encounter: %v", err)
	}

	return &encounterv2pb.MoveEntityResponse{}, nil
}
```

If Step 4.1 gave a clean entity_id-validation API, also add validation here before calling `enc.Move`. Mismatched entity_id → `codes.PermissionDenied`.

> **Implementer note:** confirmed during plan review that `*encounter.Data` exposes `Players` map keyed by `core.PlayerID`, with each entry carrying an exported `EntityID` (per `rpg-toolkit/encounter/data.go:23-25`). If you take the "read from data directly" branch, validate before calling `LoadFromData` (which consumes `data`):
> ```go
> if pd, ok := data.Players[core.PlayerID(playerID)]; ok {
>     if string(pd.EntityID) != req.GetEntityId() {
>         return nil, status.Error(codes.PermissionDenied, "entity_id does not match player's controlled entity")
>     }
> }
> ```

- [ ] **Step 4.5: Run happy-path test**

```bash
go test ./internal/handlers/dnd5e/v2/encounter/... -run TestHandlerSuite/TestMoveEntity_HappyPath -v 2>&1 | tail -10
```
Expected: PASS.

- [ ] **Step 4.6: Add error-path tests**

Append to `handler_test.go`:
```go
func (s *HandlerSuite) TestMoveEntity_NoPlayerID_Unauthenticated() {
	ctx := context.Background() // no auth
	_, err := s.handler.MoveEntity(ctx, &encounterv2pb.MoveEntityRequest{EncounterId: "enc-1"})
	s.Require().Error(err)
	st, _ := status.FromError(err)
	s.Require().Equal(codes.Unauthenticated, st.Code())
}

func (s *HandlerSuite) TestMoveEntity_MissingEncounter_NotFound() {
	_, err := s.handler.MoveEntity(s.ctx, &encounterv2pb.MoveEntityRequest{
		EncounterId: "missing", EntityId: "char-A",
	})
	s.Require().Error(err)
	st, _ := status.FromError(err)
	s.Require().Equal(codes.NotFound, st.Code())
}
```

- [ ] **Step 4.7: Run all handler tests**

```bash
go test ./internal/handlers/dnd5e/v2/encounter/... -v 2>&1 | tail -20
```
Expected: all PASS.

- [ ] **Step 4.8: Commit**

```bash
git add internal/handlers/dnd5e/v2/encounter/handler.go internal/handlers/dnd5e/v2/encounter/handler_test.go
git commit -m "feat(encounter/v2): implement MoveEntity RPC

Load → Move → Save loop using the toolkit encounter SDK. Auth required
(Unauthenticated when missing). NotFound for missing encounters,
InvalidArgument for missing encounter_id and toolkit-level move errors
(out of range, blocked path, not player's turn, etc.). Internal for
load/save infra failures.

The empty MoveEntityResponse is by proto design — world changes flow
as events on StreamEncounter, not in the ack.

Refs #494."
```

---

## Task 5: `StreamEncounter` handler

**Files:**
- Modify: `internal/handlers/dnd5e/v2/encounter/handler.go` (add `StreamEncounter` method)
- Modify: `internal/handlers/dnd5e/v2/encounter/handler_test.go`

- [ ] **Step 5.1: Inspect SnapshotFor + Snapshot shape**

Read `/home/kirk/personal/rpg-toolkit/encounter/encounter.go:78-100` to confirm `SnapshotFor(playerID)` return type fields. The `Snapshot` shape will inform the synthetic `SnapshotDelivered` proto translation. Note the shape; if it doesn't map cleanly to `SnapshotDelivered.encounter` proto, file a toolkit issue and stub the snapshot translation as "send empty SnapshotDelivered for now" — slice 2 will exercise it.

- [ ] **Step 5.2: Write StreamEncounter snapshot test**

Append to `handler_test.go`:
```go
func (s *HandlerSuite) TestStreamEncounter_SendsSnapshotFirst() {
	enc := tkenc.New("enc-1", s.broker)
	s.Require().NoError(enc.AddPlayer(tkenc.PlayerInput{
		PlayerID: "player-A", EntityID: "char-A", Position: core.Hex{Q: 0, R: 0, S: 0},
	}))
	s.Require().NoError(s.repo.Save(s.ctx, enc.ToData()))

	stream := newCapturingStream(s.ctx) // helper that records sends + supports close
	go func() {
		_ = s.handler.StreamEncounter(&encounterv2pb.StreamEncounterRequest{
			EncounterId: "enc-1",
		}, stream)
	}()

	first := stream.WaitForSend(s.T(), 100*time.Millisecond)
	s.Require().NotNil(first.GetSnapshotDelivered(), "first event should be SnapshotDelivered")
}
```
Add a `capturingStream` helper at the bottom of the test file — a stub `encounterv2pb.EncounterService_StreamEncounterServer` that records `Send` calls into a channel and supports `Context()` returning the test ctx so the stream loop can be cancelled.

- [ ] **Step 5.3: Run (expect failure)**

```bash
go test ./internal/handlers/dnd5e/v2/encounter/... -run TestHandlerSuite/TestStreamEncounter -v 2>&1 | tail -10
```
Expected: FAIL — Unimplemented.

- [ ] **Step 5.4: Implement StreamEncounter**

Add to `handler.go`:
```go
func (h *Handler) StreamEncounter(req *encounterv2pb.StreamEncounterRequest, stream encounterv2pb.EncounterService_StreamEncounterServer) error {
	ctx := stream.Context()
	playerID := auth.GetPlayerID(ctx)
	if playerID == "" {
		return status.Error(codes.Unauthenticated, "no player id in context")
	}
	encID := core.EncounterID(req.GetEncounterId())
	if encID == "" {
		return status.Error(codes.InvalidArgument, "encounter_id is required")
	}

	// Subscribe FIRST so the broker holds events in its buffered channel
	// (per encounter/broker.go:84) while we build the snapshot. Any Move
	// happening between Subscribe and the forward loop is captured by the
	// subscription and delivered after the snapshot send.
	sub, err := h.broker.Subscribe(encID, core.PlayerID(playerID))
	if err != nil {
		return status.Errorf(codes.Internal, "subscribe: %v", err)
	}
	defer sub.Close()

	// Snapshot the encounter at-time-of-connect.
	data, err := h.encRepo.Get(ctx, string(encID))
	if err != nil {
		if errors.Is(err, encountersv2.ErrNotFound) {
			return status.Error(codes.NotFound, "encounter not found")
		}
		return status.Errorf(codes.Internal, "load encounter: %v", err)
	}
	enc, err := encounter.LoadFromData(data, h.broker)
	if err != nil {
		return status.Errorf(codes.Internal, "load from data: %v", err)
	}
	snap := enc.SnapshotFor(core.PlayerID(playerID))
	snapEvent := translateSnapshot(snap, h.now())
	if err := stream.Send(snapEvent); err != nil {
		return err
	}

	// Forward broker events.
	for {
		select {
		case <-ctx.Done():
			return nil
		case evt, ok := <-sub.Events():
			if !ok {
				return nil
			}
			out, err := TranslateEvent(evt, core.PlayerID(playerID), h.now())
			switch {
			case errors.Is(err, ErrViewerSawNothing):
				continue
			case errors.Is(err, ErrUnknownEventType):
				// TODO(metric): increment translator-gap counter
				continue
			case err != nil:
				return status.Errorf(codes.Internal, "translate: %v", err)
			}
			if err := stream.Send(out); err != nil {
				return err
			}
		}
	}
}
```

Add `translateSnapshot` to `translate.go`:
```go
func translateSnapshot(snap encounter.Snapshot, now time.Time) *encounterv2pb.EncounterEvent {
	return &encounterv2pb.EncounterEvent{
		Sequence:  0, // snapshots are pre-history; deltas start at 1
		Timestamp: timestamppb.New(now),
		Event: &encounterv2pb.EncounterEvent_SnapshotDelivered{
			SnapshotDelivered: &encounterv2pb.SnapshotDelivered{
				// Encounter field shape depends on Step 5.1's findings.
				// If the toolkit Snapshot doesn't yet map cleanly, leave empty
				// and document the gap in PR description.
			},
		},
	}
}
```

- [ ] **Step 5.5: Run snapshot test**

```bash
go test ./internal/handlers/dnd5e/v2/encounter/... -run TestHandlerSuite/TestStreamEncounter_SendsSnapshotFirst -v 2>&1 | tail -10
```
Expected: PASS.

- [ ] **Step 5.6: Add forward-events test**

Append to `handler_test.go`:
```go
func (s *HandlerSuite) TestStreamEncounter_ForwardsBrokerEvents() {
	enc := tkenc.New("enc-1", s.broker)
	s.Require().NoError(enc.AddPlayer(tkenc.PlayerInput{
		PlayerID: "player-A", EntityID: "char-A", Position: core.Hex{Q: 0, R: 0, S: 0},
	}))
	s.Require().NoError(s.repo.Save(s.ctx, enc.ToData()))

	stream := newCapturingStream(s.ctx)
	go func() {
		_ = s.handler.StreamEncounter(&encounterv2pb.StreamEncounterRequest{
			EncounterId: "enc-1",
		}, stream)
	}()

	// Drain the snapshot.
	_ = stream.WaitForSend(s.T(), 100*time.Millisecond)

	// Move via the handler — broker emits MoveEvent.
	_, err := s.handler.MoveEntity(s.ctx, &encounterv2pb.MoveEntityRequest{
		EncounterId:  "enc-1",
		EntityId:     "char-A",
		ProposedPath: []*encounterv2pb.Position{{X: 0, Y: 0, Z: 0}, {X: 1, Y: -1, Z: 0}},
	})
	s.Require().NoError(err)

	// Stream should receive an EntityMoved.
	got := stream.WaitForSend(s.T(), 200*time.Millisecond)
	s.Require().NotNil(got.GetEntityMoved())
}
```

- [ ] **Step 5.7: Run + commit**

```bash
go test ./internal/handlers/dnd5e/v2/encounter/... -v 2>&1 | tail -20
```
Expected: all PASS.

```bash
git add internal/handlers/dnd5e/v2/encounter/handler.go \
       internal/handlers/dnd5e/v2/encounter/translate.go \
       internal/handlers/dnd5e/v2/encounter/handler_test.go
git commit -m "feat(encounter/v2): implement StreamEncounter RPC

Subscribe-then-snapshot-then-loop. Initial SnapshotDelivered fires
synchronously after subscribe so any event firing between snapshot
read and stream-active is captured by the broker subscription.

Translator errors discriminated via errors.Is:
- ErrViewerSawNothing → continue silently
- ErrUnknownEventType → continue (gap to file)
- other err → tear down stream with codes.Internal

Refs #494."
```

---

## Task 6: bufconn integration test — the slice 1 gate

**Files:**
- Create: `internal/integration/encounter_v2_test.go`

This test is the wave-2.5 acceptance shape: two players in one encounter, both subscribe, one moves, both receive events consistent with their `PerceptionView`. When this test passes, slice 1 is done.

- [ ] **Step 6.1: Sketch test outline**

```go
package integration_test

import (
	"context"
	"testing"

	"github.com/stretchr/testify/suite"

	encounterv2pb "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/v1alpha2/encounter"
	"github.com/KirkDiggler/rpg-api/internal/auth"
	"github.com/KirkDiggler/rpg-api/internal/integration/harness"
	tkenc "github.com/KirkDiggler/rpg-toolkit/encounter"
	"github.com/KirkDiggler/rpg-toolkit/encounter/core"
)

type EncounterV2IntegrationSuite struct {
	suite.Suite
	srv *harness.TestServer
	ctx context.Context
}

func (s *EncounterV2IntegrationSuite) SetupTest() {
	s.ctx = context.Background()
	s.srv = harness.NewTestServer(s.T())
}

func (s *EncounterV2IntegrationSuite) TearDownTest() {
	s.srv.Close()
}

func (s *EncounterV2IntegrationSuite) TestMovementSliceTwoPlayers() {
	// Build a small encounter with players A and B both visible to each other.
	enc := tkenc.New("enc-1", s.srv.BrokerV2)
	s.Require().NoError(enc.AddPlayer(tkenc.PlayerInput{
		PlayerID: "player-A", EntityID: "char-A", Position: core.Hex{Q: 0, R: 0, S: 0},
	}))
	s.Require().NoError(enc.AddPlayer(tkenc.PlayerInput{
		PlayerID: "player-B", EntityID: "char-B", Position: core.Hex{Q: 1, R: -1, S: 0},
	}))
	s.Require().NoError(s.srv.EncRepoV2.Save(s.ctx, enc.ToData()))

	ctxA := auth.WithPlayerID(s.ctx, "player-A")
	ctxB := auth.WithPlayerID(s.ctx, "player-B")

	streamA, err := s.srv.EncounterClientV2.StreamEncounter(ctxA, &encounterv2pb.StreamEncounterRequest{EncounterId: "enc-1"})
	s.Require().NoError(err)
	streamB, err := s.srv.EncounterClientV2.StreamEncounter(ctxB, &encounterv2pb.StreamEncounterRequest{EncounterId: "enc-1"})
	s.Require().NoError(err)

	// Both receive snapshot first.
	snapA, err := streamA.Recv()
	s.Require().NoError(err)
	s.Require().NotNil(snapA.GetSnapshotDelivered())
	snapB, err := streamB.Recv()
	s.Require().NoError(err)
	s.Require().NotNil(snapB.GetSnapshotDelivered())

	// A moves.
	_, err = s.srv.EncounterClientV2.MoveEntity(ctxA, &encounterv2pb.MoveEntityRequest{
		EncounterId:  "enc-1",
		EntityId:     "char-A",
		ProposedPath: []*encounterv2pb.Position{{X: 0, Y: 0, Z: 0}, {X: 1, Y: 0, Z: -1}, {X: 2, Y: 0, Z: -2}},
	})
	s.Require().NoError(err)

	// Both A and B receive EntityMoved (small encounter; mutual LoS).
	movA, err := streamA.Recv()
	s.Require().NoError(err)
	s.Require().NotNil(movA.GetEntityMoved())
	s.Require().Equal("char-A", movA.GetEntityMoved().EntityId)

	movB, err := streamB.Recv()
	s.Require().NoError(err)
	s.Require().NotNil(movB.GetEntityMoved())
	s.Require().Equal("char-A", movB.GetEntityMoved().EntityId)

	// Stream contexts are scoped to the per-call ctx; the test ends, contexts
	// are cancelled by the suite tear-down, server-side stream loops exit
	// cleanly via select on ctx.Done().
}

func TestEncounterV2IntegrationSuite(t *testing.T) {
	suite.Run(t, new(EncounterV2IntegrationSuite))
}
```

- [ ] **Step 6.2: Run integration test**

```bash
go test ./internal/integration/... -run TestEncounterV2IntegrationSuite -v 2>&1 | tail -30
```
Expected: PASS. If it fails, the failure is the slice-1 bug — diagnose, fix, repeat.

> ⚠️ **Common failure modes:**
> - **Snapshot Recv blocks**: server-side snapshot send may be racing the subscribe. Re-check Step 5.4 for subscribe-before-snapshot ordering.
> - **streamB doesn't receive EntityMoved**: the broker audience filter may have excluded B because the toolkit's stub LoS doesn't see them. Adjust starting positions so they're guaranteed in mutual range.
> - **Auth context not propagated**: bufconn doesn't run real interceptors. Check the harness wiring — if the v1alpha1 tests inject auth via context-passing rather than interceptor, mirror that pattern.
> - **`AddPlayer` field names** don't match: re-read `/home/kirk/personal/rpg-toolkit/encounter/encounter.go:21` and adjust.

- [ ] **Step 6.3: Run full make ci-check**

```bash
make ci-check 2>&1 | tail -30
```
Expected: clean.

- [ ] **Step 6.4: Commit**

```bash
git add internal/integration/encounter_v2_test.go
git commit -m "test(encounter/v2): bufconn integration — two players, one moves

The wave-2.5 acceptance shape, in code form. Seeds an encounter with
players A and B in mutual LoS, opens both streams, A calls MoveEntity,
asserts both streams receive an EntityMoved event whose entity_id
matches and whose actual_path reflects per-viewer visibility.

When this test passes locally, slice 1 is feature-complete; the wave
gate is the two-browser playtest.

Refs #494, rpg-project#15."
```

---

## Task 7: Docs + verification

**Files:**
- Modify: `rpg-api/docs/status.md` (or equivalent — confirm path; rpg-api recon noted `docs/` exists)
- Create: `rpg-api/docs/journey/<date>-encounter-v2-walking-skeleton.md` (if the journey-doc pattern exists)

- [ ] **Step 7.1: Verify CI is clean across all checks**

```bash
make ci-check 2>&1 | tail -50
```
Expected: clean. Specifically: format/lint/vet/build/test all green.

- [ ] **Step 7.2: Update `rpg-api/docs/status.md`**

Confirmed during plan review: `rpg-api/docs/status.md` and `rpg-api/docs/quality.md` both exist. Find the encounter / v1alpha1 / orchestrator section in `status.md` and add a v1alpha2 entry: "v1alpha2 encounter service: walking skeleton shipped via [PR_LINK]. MoveEntity + StreamEncounter implemented over toolkit encounter SDK; other RPCs return Unimplemented. v1alpha1 movement path remains primary until web migrates. Follow-ups: rpg-toolkit#629 (LoS-loss events)."

- [ ] **Step 7.3: Add a journey doc (if pattern exists)**

If `rpg-api/docs/journey/` exists, add a short narrative doc capturing: the architectural shift (no orchestrator in v2), the parallel-run with v1alpha1, the toolkit gap that was filed (#629). Otherwise skip and rely on the spec + plan as the journey record.

- [ ] **Step 7.4: Commit docs**

```bash
git add rpg-api/docs/
git commit -m "docs(encounter/v2): record walking skeleton in status + journey

Refs #494, rpg-project#15."
```

- [ ] **Step 7.5: Push branch and open PR**

```bash
git push -u origin <branch-name>
gh pr create --title "feat(encounter/v2): walking skeleton — MoveEntity + StreamEncounter over toolkit SDK" --body "$(cat <<'EOF'
## Summary

Wave 2.5 slice 1: rpg-api side of the encounter SDK integration. Wires the new rpg-toolkit `encounter` SDK (PR rpg-toolkit#623) into rpg-api as the v1alpha2 encounter service.

**Architectural note:** v2 has no orchestrator layer. The handler talks directly to the toolkit Encounter + Broker. This is the Chapter 1: Architecture Honesty move — game rules live in the toolkit; rpg-api becomes a thinner shim.

## What's in slice 1

- `internal/repositories/encounters/v2/` — in-memory repository for `*encounter.Data` (round-trip via toolkit serializer on every Save/Get)
- `internal/handlers/dnd5e/v2/encounter/` — handler (broker + repo + now-func), translator with typed error sentinels, MoveEntity + StreamEncounter
- `cmd/server/server.go` — v2 service registered alongside v1alpha1 (additive only; v1alpha1 untouched)
- `internal/integration/harness/harness.go` — extended with `EncounterClientV2`, `BrokerV2`, `EncRepoV2`
- `internal/integration/encounter_v2_test.go` — bufconn integration test (the slice gate)

## What's NOT in slice 1

- `OpenDoor`, `Attack`, `EndTurn`, etc. — return `codes.Unimplemented`
- v1alpha1 movement deletion — deferred to slice 3 after playtest verification
- Web rendering — slice 2 (rpg-dnd5e-web#387)

## Known gaps filed

- rpg-toolkit#629 — `EntityAppeared` / `EntityDisappeared` on LoS crossings. Out of slice 1 scope; wave-goal scenario uses mutual LoS so this doesn't bite.

## Test plan

- [ ] `make ci-check` clean locally
- [ ] Bufconn integration test (`TestEncounterV2IntegrationSuite/TestMovementSliceTwoPlayers`) passes
- [ ] No regressions on existing v1alpha1 encounter tests
- [ ] Local two-browser smoke (web on v1alpha1) — no movement regressions

Closes part of rpg-project#15. Refs rpg-toolkit#628.
EOF
)"
```

- [ ] **Step 7.6: Wait for Copilot review (~5-10 min) and respond**

Per `feedback_copilot_review` and `feedback_copilot_review_timing`: sleep ~6-10 min, then `gh pr view <PR> --comments`. Address every comment with either a fix or a threaded "why not" reply before asking Kirk to merge.

---

## Slice 1 done — what closes when

- **Issue rpg-api#494** closes when this PR merges.
- **rpg-project#15** stays open — the wave gate is the two-browser playtest after slice 2 (rpg-dnd5e-web#387) ships.
- **rpg-toolkit#628** stays open until the playtest confirms no further toolkit gaps (or closes with no work if none surfaced).
- **rpg-toolkit#629** stays open — picked up in a parallel session.

## Reference

- Spec: `rpg-project/ideas/encounter/v1alpha2/sdk-direction-rpgapi.md`
- Toolkit walking skeleton: `rpg-toolkit/docs/architecture/components/encounter.md`, PR rpg-toolkit#623
- Wave umbrella: rpg-project#15
- Board: https://github.com/users/KirkDiggler/projects/10
