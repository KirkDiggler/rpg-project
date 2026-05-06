# Phase 2 Slice 1 — Encounter SDK Walking Skeleton (rpg-toolkit)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Stand up the encounter SDK end-to-end in `rpg-toolkit` with a slim vertical slice — a sealed `EncounterEvent` taxonomy, three concrete events (`MoveEvent`, `HexRevealedEvent`, `DoorOpenedEvent`), a process-scoped `Broker` over a pluggable `Transport`, a transient `Encounter` aggregate that round-trips through `ToData`/`LoadFromData` and exposes `Move` + `OpenDoor` verbs, a basic `PerceptionView`, and an integration test that proves the publish/subscribe seam delivers per-player projected events correctly.

**Architecture:** Per the design at [`../sdk-direction.md`](../sdk-direction.md). One Go module: `rpg-toolkit/encounter/`. Internal package layout is a linear DAG to avoid import cycles:

```
encounter/types  ← encounter/events  ← encounter/perception  ← encounter (top-level)
                                                              ← encounter (broker, transport)
```

The brainstorm doc described `perception/` as a top-level module sibling to `encounter/`, but that creates a module-level dependency cycle (perception needs `Hex`/`PlayerID`/event-slice types from encounter; encounter needs perception's projection functions). For this slice, perception lives at `encounter/perception/`. Future slices can extract it to a top-level module once shared primitive types (`Hex`, `PlayerID`) move to `tools/spatial`/`core` and the cycle dissolves.

Sealed `EncounterEvent` (AWS v2 SDK marker pattern). Process-scoped `Broker` over a pluggable `Transport` (InMemoryTransport only; Redis/Kafka are future). Encounter is fully transient — `LoadFromData` per call, mutate, `ToData`, save. Per-player `PerceptionView` rides in `EncounterData`. Decoupled cause/effect events: action verbs emit a cause event (`MoveEvent`, `DoorOpenedEvent`) AND a `HexRevealedEvent` whenever vision changes — same shape regardless of cause.

**Tech Stack:** Go 1.22+, `testify/suite` for test structure, `encoding/json` for the codec. Stub line-of-sight is Manhattan-radius; real LoS is a future slice.

**Spec:** [`../sdk-direction.md`](../sdk-direction.md) is normative. The "Decisions settled" section is binding; "What this brainstorm did not settle" calls out items deliberately deferred (full event catalog, real LoS, Redis transport, gRPC handler, etc.).

**Repo:** `rpg-toolkit` at `/home/kirk/personal/rpg-toolkit`.

**Branch:** `feat/<issue>-encounter-walking-skeleton` (cut from fresh `main`). Pre-flight Task 0 files the issue and creates the branch.

**Conventions** (from `rpg-toolkit/CLAUDE.md` and the workspace `CLAUDE.md`):
- Uber's gomock for any service mocks (none required in this slice).
- Always use `testify/suite` for test suites.
- Per workspace `feedback_worktrees`: local replace directives are OK during dev but **must be stripped before push** — CI fails on any `replace` line. This slice is a single Go module, so no cross-module replace directives are needed; if a future slice introduces them, follow the strip-before-push pattern.
- No `go.work` files committed (CI rule, per `rpg-toolkit/CLAUDE.md`). Local `go.work` for dev-time multi-module work is fine but is gitignored.
- Always merge, never rebase. Always create a PR; never merge locally.
- Issue-first; no branch without an issue.

**Build commands** (verified against `rpg-toolkit/Makefile`):
- `make pre-commit` runs fmt + lint + tests + coverage **for `core` and `events` only.** It does NOT cover the new `encounter/` module. The Makefile needs a per-module update (out of scope for this slice; flagged in Task 10).
- `make test-all` discovers every `go.mod` under the tree and runs `go test -race ./...` in each. **Use this** as the slice's "all green" check.
- `make ci-check` does not exist. References to it in older docs are stale.

For each task in this plan, the test step is `cd encounter && go test -race ./...` (per-module) or `make test-all` (whole-tree).

**Granularity note:** Tasks are **phase-aligned** logical chunks, not 2–5-minute TDD micro-steps. Each task ships a coherent capability with tests, ends with `make test-all` clean, and gets a single commit. Within a task, follow TDD discipline (test first, watch fail, implement, watch pass) but don't split each TDD cycle into its own commit.

**What this slice deliberately leaves out:**
- Combat (`Attack`, `ActivateFeature`, `UseAction`, `Interact`, `SubmitCheck`, `EndTurn`).
- Action economy enforcement, turn ordering.
- Conditions and senses (no darkvision, blinded, invisibility — default sight only).
- Real LoS (Manhattan-radius stub with no walls).
- Redis or Kafka transport (InMemoryTransport only).
- gRPC handler integration (slice is proven via `go test`).
- All other event types beyond Move / HexRevealed / DoorOpened.
- Monsters (players-only encounters).
- Entity-visibility accumulation (`HexRevealedSlice.Entities` field exists for shape stability but is not emitted in slice 1).

---

## File Map

```
rpg-toolkit/encounter/                    ← one Go module
├── go.mod                                 (Task 1)
├── doc.go                                 (Task 1) package doc
├── types/                                 ← Task 1
│   ├── doc.go
│   ├── ids.go                             EncounterID, PlayerID, EntityID
│   ├── hex.go                             Hex, HexSet (custom JSON), AudienceSet
│   └── types_test.go
├── events/                                ← Task 2
│   ├── doc.go
│   ├── events.go                          EncounterEvent sealed interface
│   ├── move.go                            MoveEvent + MovePlayerSlice + MarshalJSON/UnmarshalJSON
│   ├── hex_revealed.go                    HexRevealedEvent + HexRevealedSlice + EntityVisibility
│   ├── door_opened.go                     DoorOpenedEvent + DoorOpenedPlayerSlice
│   └── events_test.go
├── perception/                            ← Task 5
│   ├── doc.go
│   ├── view.go                            PerceptionView (with future-slice fields)
│   ├── project.go                         ProjectMove, ProjectDoorOpen
│   ├── los_stub.go                        Manhattan-radius LoS (exported VisibleHexesAt)
│   └── project_test.go
├── transport.go                           (Task 3) Transport interface
├── transport_inmem.go                     (Task 3) InMemoryTransport
├── transport_inmem_test.go                (Task 3)
├── broker.go                              (Task 4) Broker, Subscription, codec
├── broker_test.go                         (Task 4)
├── data.go                                (Task 6) EncounterData
├── encounter.go                           (Tasks 6/7/8) Encounter, verbs
├── encounter_test.go                      (Tasks 6/7/8)
└── integration_test.go                    (Task 9)

docs/architecture/components/
└── encounter.md                           (Task 10)
```

Single Go module (`github.com/KirkDiggler/rpg-toolkit/encounter`) with four subpackages: `types`, `events`, `perception`, plus the top-level `encounter` package. No cross-module cycles. No replace directives needed.

---

## Task 0 — Pre-flight: file issue, cut branch

**Goal:** Issue first, branch from fresh main, ready to write code.

- [ ] **Step 1: File issue under board #11**

```bash
gh issue create \
  --repo KirkDiggler/rpg-toolkit \
  --title "Encounter SDK walking skeleton: events, broker, transient Encounter" \
  --body "$(cat <<'EOF'
Implements Phase 2 Slice 1 of the v1alpha2 encounter SDK design.

Spec: rpg-project/ideas/encounter/v1alpha2/sdk-direction.md
Plan: rpg-project/ideas/encounter/v1alpha2/plans/02-walking-skeleton.md

## Scope (in)
- New top-level `encounter/` module with subpackages: types, events, perception.
- Encounter aggregate, Broker, InMemoryTransport, Move, OpenDoor, ToData/LoadFromData, SnapshotFor.
- Sealed EncounterEvent interface, MoveEvent, HexRevealedEvent, DoorOpenedEvent.
- PerceptionView, ProjectMove, ProjectDoorOpen with stub LoS (Manhattan radius).
- Integration test proving end-to-end publish/subscribe.

## Scope (out, deliberate)
- Combat verbs, action economy, turn ordering, monsters, conditions, senses.
- Real LoS algorithm.
- Redis transport, gRPC handler.
- All other event types beyond Move / HexRevealed / DoorOpened.

## Acceptance
- \`make test-all\` clean across new packages.
- Integration test demonstrates two players, one moves and one watches; door opens; each receives correct typed events with per-player projection.
EOF
)" \
  --label type:feature \
  --project "KirkDiggler/10"
```

Capture the issue number returned (e.g. `#700`). **Substitute it everywhere `<issue>` appears in subsequent commands and commit messages.**

- [ ] **Step 2: Cut branch from fresh main**

```bash
cd /home/kirk/personal/rpg-toolkit
git checkout main
git pull
git checkout -b feat/<issue>-encounter-walking-skeleton
```

- [ ] **Step 3: Verify clean starting state**

```bash
make test-all
```

Expected: PASS. Don't proceed if main is broken — investigate first.

No commit at end of Task 0; the branch is ready for work.

---

## Task 1 — Module setup + `encounter/types/` subpackage

**Goal:** Create the Go module, package doc, and the primitives subpackage that all other internal packages will depend on. **The types subpackage exists specifically to break what would otherwise be an `encounter` ↔ `encounter/events` import cycle.**

**Files:**
- Create: `encounter/go.mod`
- Create: `encounter/doc.go`
- Create: `encounter/types/doc.go`
- Create: `encounter/types/ids.go`
- Create: `encounter/types/hex.go`
- Create: `encounter/types/types_test.go`

- [ ] **Step 1: Create the module**

```bash
cd /home/kirk/personal/rpg-toolkit
mkdir -p encounter
cd encounter
go mod init github.com/KirkDiggler/rpg-toolkit/encounter
```

Add the testify dependency:

```bash
go get github.com/stretchr/testify@latest
```

No replace directives. This slice is a single module.

- [ ] **Step 2: Write `encounter/doc.go`**

```go
// Package encounter implements the encounter SDK — the orchestrator-facing
// facade for running an encounter (combat, free-roam, social) end-to-end.
//
// An Encounter is a transient object. Game servers Load it from persisted
// state, mutate via verb methods (Move, OpenDoor, ...), serialize back via
// ToData, and save. Player-facing events flow through a process-scoped
// Broker that publishes per-player projected events through a pluggable
// Transport (InMemoryTransport, RedisTransport, ...).
//
// Internal layout:
//   encounter/types  — primitive value types (IDs, Hex, HexSet, AudienceSet)
//   encounter/events — sealed EncounterEvent interface + concrete events
//   encounter/perception — PerceptionView + projection functions
//   encounter (top-level) — Encounter aggregate, Broker, Transport
//
// See rpg-project/ideas/encounter/v1alpha2/sdk-direction.md for the design.
package encounter
```

- [ ] **Step 3: Write `encounter/types/doc.go`**

```go
// Package types holds the primitive value types shared across the encounter
// SDK's internal subpackages (events, perception) and the top-level
// encounter package. It exists specifically to keep the internal package
// graph acyclic — events and perception import types; encounter imports
// all three.
//
// These types are intentionally minimal. When Hex/grid logic moves to
// rpg-toolkit/tools/spatial in a future slice, this package will shrink
// or be absorbed.
package types
```

- [ ] **Step 4: Write `encounter/types/ids.go`**

```go
package types

// EncounterID uniquely identifies an encounter instance.
type EncounterID string

// PlayerID uniquely identifies a player seat in an encounter.
type PlayerID string

// EntityID uniquely identifies any entity in an encounter (player char,
// monster, prop, door, ...).
type EntityID string

// AudienceSet is the set of player IDs that can perceive an event.
// Slice (not map) is sufficient — the broker only iterates.
type AudienceSet []PlayerID
```

- [ ] **Step 5: Write `encounter/types/hex.go`** — including custom JSON for `HexSet`

```go
package types

import (
    "encoding/json"
    "sort"
)

// Hex is a cube-coordinate hex on the encounter grid.
// (Real spatial logic eventually moves to rpg-toolkit/tools/spatial; we use
// bare coordinates here for the slice and stub LoS computations.)
type Hex struct {
    Q, R, S int
}

// HexSet is a set of hexes with O(1) membership.
//
// JSON: HexSet marshals as a stable-ordered slice of Hex. Go's encoding/json
// cannot encode struct keys in a map directly — without custom marshaling,
// HexSet would silently round-trip as an empty object. The MarshalJSON /
// UnmarshalJSON methods below convert to/from a slice for wire format while
// preserving set semantics in memory.
type HexSet map[Hex]struct{}

// NewHexSet builds a HexSet from a slice of Hex.
func NewHexSet(hexes ...Hex) HexSet {
    out := make(HexSet, len(hexes))
    for _, h := range hexes {
        out[h] = struct{}{}
    }
    return out
}

// Has reports whether the set contains h.
func (s HexSet) Has(h Hex) bool {
    _, ok := s[h]
    return ok
}

// Slice returns the set as an unordered slice.
func (s HexSet) Slice() []Hex {
    out := make([]Hex, 0, len(s))
    for h := range s {
        out = append(out, h)
    }
    return out
}

// MarshalJSON encodes the set as a sorted slice for stable, deterministic
// wire output (sorted by Q then R then S).
func (s HexSet) MarshalJSON() ([]byte, error) {
    out := s.Slice()
    sort.Slice(out, func(i, j int) bool {
        if out[i].Q != out[j].Q {
            return out[i].Q < out[j].Q
        }
        if out[i].R != out[j].R {
            return out[i].R < out[j].R
        }
        return out[i].S < out[j].S
    })
    return json.Marshal(out)
}

// UnmarshalJSON decodes a slice of Hex back into a HexSet.
// Accepts JSON null and the empty array as the empty set.
func (s *HexSet) UnmarshalJSON(b []byte) error {
    if string(b) == "null" {
        *s = make(HexSet)
        return nil
    }
    var slice []Hex
    if err := json.Unmarshal(b, &slice); err != nil {
        return err
    }
    out := make(HexSet, len(slice))
    for _, h := range slice {
        out[h] = struct{}{}
    }
    *s = out
    return nil
}
```

- [ ] **Step 6: Write `encounter/types/types_test.go`**

```go
package types_test

import (
    "encoding/json"
    "testing"

    "github.com/KirkDiggler/rpg-toolkit/encounter/types"
    "github.com/stretchr/testify/suite"
)

type TypesSuite struct {
    suite.Suite
}

func TestTypesSuite(t *testing.T) {
    suite.Run(t, new(TypesSuite))
}

func (s *TypesSuite) TestHexSet_HasAndSlice() {
    h := types.Hex{Q: 1, R: -2, S: 1}
    set := types.NewHexSet(h, types.Hex{Q: 0, R: 0, S: 0})

    s.True(set.Has(h))
    s.False(set.Has(types.Hex{Q: 9, R: 9, S: -18}))
    s.Len(set.Slice(), 2)
}

// HexSet round-trips cleanly through JSON. This is load-bearing — the
// PerceptionView struct embeds HexSet and must persist correctly.
func (s *TypesSuite) TestHexSet_JSONRoundTrip() {
    a := types.Hex{Q: 1, R: -1, S: 0}
    b := types.Hex{Q: 2, R: -1, S: -1}
    original := types.NewHexSet(a, b)

    payload, err := json.Marshal(original)
    s.Require().NoError(err)

    var decoded types.HexSet
    s.Require().NoError(json.Unmarshal(payload, &decoded))

    s.Len(decoded, 2)
    s.True(decoded.Has(a))
    s.True(decoded.Has(b))
}

// Empty HexSet round-trips as JSON null or [] — both should decode to empty.
func (s *TypesSuite) TestHexSet_EmptyRoundTrip() {
    payload, err := json.Marshal(types.HexSet{})
    s.Require().NoError(err)

    var decoded types.HexSet
    s.Require().NoError(json.Unmarshal(payload, &decoded))
    s.Empty(decoded)

    var fromNull types.HexSet
    s.Require().NoError(json.Unmarshal([]byte("null"), &fromNull))
    s.NotNil(fromNull)
    s.Empty(fromNull)
}
```

- [ ] **Step 7: Run tests**

```bash
cd /home/kirk/personal/rpg-toolkit/encounter
go mod tidy
go test -race ./types/...
```

Expected: PASS, all subtests green.

- [ ] **Step 8: Commit**

```bash
git add encounter/
git commit -m "feat(encounter): add module skeleton + types subpackage

Initialize the encounter module. The types subpackage holds primitive
value types (EncounterID, PlayerID, EntityID, Hex, HexSet, AudienceSet)
shared across the SDK's internal layout. HexSet has custom JSON
marshaling so struct-keyed maps round-trip correctly.

The types subpackage exists to break what would otherwise be an
encounter <-> encounter/events package import cycle.

Refs #<issue>"
```

---

## Task 2 — `encounter/events/` subpackage: sealed interface + 3 concrete events

**Goal:** Define the sealed `EncounterEvent` interface (AWS v2 SDK marker pattern) and the three concrete event types this slice publishes. Each concrete keeps its `encID` and `seq` fields **unexported** but provides custom `MarshalJSON`/`UnmarshalJSON` so the JSON wire codec can round-trip without leaking construction-only fields.

**Files:**
- Create: `encounter/events/doc.go`
- Create: `encounter/events/events.go`
- Create: `encounter/events/move.go`
- Create: `encounter/events/hex_revealed.go`
- Create: `encounter/events/door_opened.go`
- Create: `encounter/events/events_test.go`

- [ ] **Step 1: Write `events/doc.go`**

```go
// Package events defines the EncounterEvent taxonomy — typed concretes
// under a sealed interface (AWS v2 SDK AttributeValue pattern).
//
// Each concrete event has its own struct, fields, and per-player slice
// type. The unexported isEncounterEvent() marker makes the interface
// externally unsatisfiable, giving compile-time bounded sum semantics.
//
// Cause vs effect:
//   - Cause events describe what happened in the world (MoveEvent,
//     DoorOpenedEvent, ConditionRemovedEvent...).
//   - Effect events describe per-player perception change (HexRevealedEvent,
//     and future HexHiddenEvent for vision-loss).
//
// See ../../sdk-direction.md for the design.
package events
```

- [ ] **Step 2: Write `events/events.go`** — the sealed interface

```go
package events

import "github.com/KirkDiggler/rpg-toolkit/encounter/types"

// EncounterEvent is the sealed sum type of events the broker carries.
//
// External packages cannot implement this interface — the marker method
// isEncounterEvent() is unexported, and only types declared in this
// package can satisfy it. Consumers type-switch on the concrete type.
type EncounterEvent interface {
    isEncounterEvent()
    EncounterID() types.EncounterID
    Sequence() uint64
    Audience() types.AudienceSet
}

// audienceFromMap derives the audience slice from a PerPlayer map's keys.
// Used by each concrete event's Audience() method.
func audienceFromMap[V any](m map[types.PlayerID]V) types.AudienceSet {
    out := make(types.AudienceSet, 0, len(m))
    for k := range m {
        out = append(out, k)
    }
    return out
}
```

- [ ] **Step 3: Write `events/move.go`** — MoveEvent with custom JSON to keep fields private

```go
package events

import (
    "encoding/json"

    "github.com/KirkDiggler/rpg-toolkit/encounter/types"
)

// MoveEvent is published when an entity moves through hexes in the encounter.
//
// Vision changes caused by the move are NOT embedded here — they ride on a
// parallel HexRevealedEvent published alongside this one. See the decoupled
// cause/effect decision in sdk-direction.md.
type MoveEvent struct {
    encID     types.EncounterID
    seq       uint64
    Mover     types.EntityID
    Path      []types.Hex
    PerPlayer map[types.PlayerID]MovePlayerSlice
}

// MovePlayerSlice is each viewer's projection of the move — which hexes
// of the path they saw the mover traverse.
type MovePlayerSlice struct {
    SeenSegments []types.Hex `json:"seen_segments"`
    // Note: vision changes from the move (newly-revealed hexes/entities)
    // are NOT embedded here — they ride on a parallel HexRevealedEvent.
    // See the decoupled cause/effect decision in sdk-direction.md.
}

// NewMoveEvent constructs a MoveEvent. The encounter is responsible for
// stamping the encounter ID and sequence number; PerPlayer is computed
// by perception.ProjectMove.
func NewMoveEvent(
    encID types.EncounterID,
    seq uint64,
    mover types.EntityID,
    path []types.Hex,
    perPlayer map[types.PlayerID]MovePlayerSlice,
) *MoveEvent {
    return &MoveEvent{
        encID:     encID,
        seq:       seq,
        Mover:     mover,
        Path:      path,
        PerPlayer: perPlayer,
    }
}

func (*MoveEvent) isEncounterEvent()                   {}
func (e *MoveEvent) EncounterID() types.EncounterID    { return e.encID }
func (e *MoveEvent) Sequence() uint64                  { return e.seq }
func (e *MoveEvent) Audience() types.AudienceSet       { return audienceFromMap(e.PerPlayer) }

// moveEventWire is the on-wire shape — used only by MarshalJSON / UnmarshalJSON.
// Keeping this private (alongside the unexported encID/seq fields) preserves
// the construction invariant: only NewMoveEvent and UnmarshalJSON can set
// the encounter ID and sequence number.
type moveEventWire struct {
    EncID     types.EncounterID                       `json:"encounter_id"`
    Seq       uint64                                  `json:"sequence"`
    Mover     types.EntityID                          `json:"mover"`
    Path      []types.Hex                             `json:"path"`
    PerPlayer map[types.PlayerID]MovePlayerSlice      `json:"per_player"`
}

// MarshalJSON exposes encID and seq under stable JSON field names without
// making the Go fields exported.
func (e *MoveEvent) MarshalJSON() ([]byte, error) {
    return json.Marshal(moveEventWire{
        EncID:     e.encID,
        Seq:       e.seq,
        Mover:     e.Mover,
        Path:      e.Path,
        PerPlayer: e.PerPlayer,
    })
}

// UnmarshalJSON populates the unexported fields from JSON.
func (e *MoveEvent) UnmarshalJSON(b []byte) error {
    var w moveEventWire
    if err := json.Unmarshal(b, &w); err != nil {
        return err
    }
    e.encID = w.EncID
    e.seq = w.Seq
    e.Mover = w.Mover
    e.Path = w.Path
    e.PerPlayer = w.PerPlayer
    return nil
}
```

- [ ] **Step 4: Write `events/hex_revealed.go`** — same pattern

```go
package events

import (
    "encoding/json"

    "github.com/KirkDiggler/rpg-toolkit/encounter/types"
)

// HexRevealedEvent is published whenever a player's vision gains hexes
// (or, eventually, newly visible entities), regardless of cause. Move,
// OpenDoor, LightChanged, ConditionRemoved (blind wearing off), etc. all
// emit HexRevealedEvent alongside their own action event.
//
// The cause stays in the parallel action event; this event describes the
// effect on perception with the same shape across all causes.
type HexRevealedEvent struct {
    encID     types.EncounterID
    seq       uint64
    PerPlayer map[types.PlayerID]HexRevealedSlice
}

// HexRevealedSlice is each viewer's projection — newly visible hexes
// and (in future slices) entities for that player.
//
// Slice 1 emits Hexes only; Entities is reserved for shape stability so
// future slices can add entity-visibility accumulation without a JSON
// migration.
type HexRevealedSlice struct {
    Hexes    types.HexSet       `json:"hexes"`
    Entities []EntityVisibility `json:"entities,omitempty"`
}

// EntityVisibility names an entity that has become visible to a player.
// Reserved for future slices; not emitted in slice 1.
type EntityVisibility struct {
    EntityID types.EntityID `json:"entity_id"`
    Position types.Hex      `json:"position"`
}

// NewHexRevealedEvent constructs a HexRevealedEvent.
func NewHexRevealedEvent(
    encID types.EncounterID,
    seq uint64,
    perPlayer map[types.PlayerID]HexRevealedSlice,
) *HexRevealedEvent {
    return &HexRevealedEvent{
        encID:     encID,
        seq:       seq,
        PerPlayer: perPlayer,
    }
}

func (*HexRevealedEvent) isEncounterEvent()                {}
func (e *HexRevealedEvent) EncounterID() types.EncounterID { return e.encID }
func (e *HexRevealedEvent) Sequence() uint64               { return e.seq }
func (e *HexRevealedEvent) Audience() types.AudienceSet    { return audienceFromMap(e.PerPlayer) }

type hexRevealedWire struct {
    EncID     types.EncounterID                          `json:"encounter_id"`
    Seq       uint64                                     `json:"sequence"`
    PerPlayer map[types.PlayerID]HexRevealedSlice        `json:"per_player"`
}

func (e *HexRevealedEvent) MarshalJSON() ([]byte, error) {
    return json.Marshal(hexRevealedWire{EncID: e.encID, Seq: e.seq, PerPlayer: e.PerPlayer})
}

func (e *HexRevealedEvent) UnmarshalJSON(b []byte) error {
    var w hexRevealedWire
    if err := json.Unmarshal(b, &w); err != nil {
        return err
    }
    e.encID = w.EncID
    e.seq = w.Seq
    e.PerPlayer = w.PerPlayer
    return nil
}
```

- [ ] **Step 5: Write `events/door_opened.go`** — same pattern

```go
package events

import (
    "encoding/json"

    "github.com/KirkDiggler/rpg-toolkit/encounter/types"
)

// DoorOpenedEvent is published when an entity opens a door in the encounter.
//
// Vision changes (newly revealed hexes through the door) ride on a parallel
// HexRevealedEvent published alongside this one — see the decoupled
// cause/effect decision.
type DoorOpenedEvent struct {
    encID     types.EncounterID
    seq       uint64
    DoorID    types.EntityID
    OpenedBy  types.EntityID
    PerPlayer map[types.PlayerID]DoorOpenedPlayerSlice
}

// DoorOpenedPlayerSlice is each viewer's projection. Visible says whether
// the player perceived the door at all (via their own LoS).
type DoorOpenedPlayerSlice struct {
    Visible bool `json:"visible"`
}

func NewDoorOpenedEvent(
    encID types.EncounterID,
    seq uint64,
    door types.EntityID,
    openedBy types.EntityID,
    perPlayer map[types.PlayerID]DoorOpenedPlayerSlice,
) *DoorOpenedEvent {
    return &DoorOpenedEvent{
        encID:     encID,
        seq:       seq,
        DoorID:    door,
        OpenedBy:  openedBy,
        PerPlayer: perPlayer,
    }
}

func (*DoorOpenedEvent) isEncounterEvent()                {}
func (e *DoorOpenedEvent) EncounterID() types.EncounterID { return e.encID }
func (e *DoorOpenedEvent) Sequence() uint64               { return e.seq }
func (e *DoorOpenedEvent) Audience() types.AudienceSet    { return audienceFromMap(e.PerPlayer) }

type doorOpenedWire struct {
    EncID     types.EncounterID                              `json:"encounter_id"`
    Seq       uint64                                         `json:"sequence"`
    DoorID    types.EntityID                                 `json:"door_id"`
    OpenedBy  types.EntityID                                 `json:"opened_by"`
    PerPlayer map[types.PlayerID]DoorOpenedPlayerSlice       `json:"per_player"`
}

func (e *DoorOpenedEvent) MarshalJSON() ([]byte, error) {
    return json.Marshal(doorOpenedWire{
        EncID: e.encID, Seq: e.seq,
        DoorID: e.DoorID, OpenedBy: e.OpenedBy,
        PerPlayer: e.PerPlayer,
    })
}

func (e *DoorOpenedEvent) UnmarshalJSON(b []byte) error {
    var w doorOpenedWire
    if err := json.Unmarshal(b, &w); err != nil {
        return err
    }
    e.encID = w.EncID
    e.seq = w.Seq
    e.DoorID = w.DoorID
    e.OpenedBy = w.OpenedBy
    e.PerPlayer = w.PerPlayer
    return nil
}
```

- [ ] **Step 6: Write `events/events_test.go`**

```go
package events_test

import (
    "encoding/json"
    "testing"

    "github.com/KirkDiggler/rpg-toolkit/encounter/events"
    "github.com/KirkDiggler/rpg-toolkit/encounter/types"
    "github.com/stretchr/testify/suite"
)

type EventsSuite struct {
    suite.Suite
}

func TestEventsSuite(t *testing.T) {
    suite.Run(t, new(EventsSuite))
}

// Each concrete satisfies the sealed EncounterEvent interface.
func (s *EventsSuite) TestConcretes_SatisfyInterface() {
    var _ events.EncounterEvent = (*events.MoveEvent)(nil)
    var _ events.EncounterEvent = (*events.HexRevealedEvent)(nil)
    var _ events.EncounterEvent = (*events.DoorOpenedEvent)(nil)
}

// MoveEvent.Audience derives from PerPlayer keys; absent players are not in audience.
func (s *EventsSuite) TestMoveEvent_AudienceFromPerPlayer() {
    e := events.NewMoveEvent("enc-1", 7, "bob",
        []types.Hex{{Q: 0, R: 0, S: 0}},
        map[types.PlayerID]events.MovePlayerSlice{
            "alice": {SeenSegments: []types.Hex{{Q: 0, R: 0, S: 0}}},
            "carol": {SeenSegments: []types.Hex{}},
        },
    )

    s.Equal(types.EncounterID("enc-1"), e.EncounterID())
    s.Equal(uint64(7), e.Sequence())
    s.ElementsMatch(types.AudienceSet{"alice", "carol"}, e.Audience())
}

// MoveEvent JSON round-trip preserves all fields, including unexported encID/seq.
func (s *EventsSuite) TestMoveEvent_JSONRoundTrip() {
    original := events.NewMoveEvent("enc-1", 42, "bob",
        []types.Hex{{Q: 1, R: -1, S: 0}, {Q: 2, R: -1, S: -1}},
        map[types.PlayerID]events.MovePlayerSlice{
            "alice": {SeenSegments: []types.Hex{{Q: 1, R: -1, S: 0}}},
        },
    )

    payload, err := json.Marshal(original)
    s.Require().NoError(err)

    var decoded events.MoveEvent
    s.Require().NoError(json.Unmarshal(payload, &decoded))

    s.Equal(types.EncounterID("enc-1"), decoded.EncounterID())
    s.Equal(uint64(42), decoded.Sequence())
    s.Equal(types.EntityID("bob"), decoded.Mover)
    s.Equal(original.Path, decoded.Path)
    s.Require().Contains(decoded.PerPlayer, types.PlayerID("alice"))
    s.Equal(original.PerPlayer["alice"].SeenSegments, decoded.PerPlayer["alice"].SeenSegments)
}

// HexRevealedEvent JSON round-trip — load-bearing because PerceptionView
// embeds HexSet via this slice and the persistence layer round-trips it.
func (s *EventsSuite) TestHexRevealedEvent_JSONRoundTrip() {
    original := events.NewHexRevealedEvent("enc-1", 8,
        map[types.PlayerID]events.HexRevealedSlice{
            "alice": {Hexes: types.NewHexSet(types.Hex{Q: 1, R: 0, S: -1})},
        },
    )

    payload, err := json.Marshal(original)
    s.Require().NoError(err)

    var decoded events.HexRevealedEvent
    s.Require().NoError(json.Unmarshal(payload, &decoded))

    s.Equal(types.EncounterID("enc-1"), decoded.EncounterID())
    s.Equal(uint64(8), decoded.Sequence())
    aliceSlice := decoded.PerPlayer["alice"]
    s.True(aliceSlice.Hexes.Has(types.Hex{Q: 1, R: 0, S: -1}))
}

// Type switch returns the concrete type.
func (s *EventsSuite) TestTypeSwitch_RecoversConcrete() {
    evts := []events.EncounterEvent{
        events.NewMoveEvent("enc-1", 1, "bob", nil, nil),
        events.NewHexRevealedEvent("enc-1", 2, nil),
        events.NewDoorOpenedEvent("enc-1", 3, "door-1", "bob", nil),
    }

    var seen []string
    for _, evt := range evts {
        switch evt.(type) {
        case *events.MoveEvent:
            seen = append(seen, "move")
        case *events.HexRevealedEvent:
            seen = append(seen, "revealed")
        case *events.DoorOpenedEvent:
            seen = append(seen, "door")
        default:
            s.FailNow("unhandled event type")
        }
    }
    s.Equal([]string{"move", "revealed", "door"}, seen)
}
```

- [ ] **Step 7: Run tests**

```bash
cd /home/kirk/personal/rpg-toolkit/encounter
go mod tidy
go test -race ./events/... ./types/...
```

Expected: PASS, all subtests green.

- [ ] **Step 8: Commit**

```bash
git add encounter/events/ encounter/go.mod encounter/go.sum
git commit -m "feat(encounter/events): add sealed EncounterEvent + 3 concretes

Sealed interface using the unexported-marker pattern (AWS v2 SDK style).
Three concrete event types for the walking skeleton: MoveEvent,
HexRevealedEvent, DoorOpenedEvent.

Each concrete keeps encID/seq fields unexported but provides
MarshalJSON/UnmarshalJSON so the JSON wire codec can round-trip without
leaking construction-only state. HexRevealedSlice.Entities is reserved
for shape stability — emitted in future slices when entity-visibility
accumulation lands.

Refs #<issue>"
```

---

## Task 3 — Transport interface + InMemoryTransport

**Goal:** Define the bytes-level pluggable pub/sub interface and an in-process implementation suitable for tests and single-process dev.

**Files:**
- Create: `encounter/transport.go`
- Create: `encounter/transport_inmem.go`
- Create: `encounter/transport_inmem_test.go`

- [ ] **Step 1: Write `transport.go`**

```go
package encounter

// Transport is the pluggable pub/sub interface the Broker uses to
// distribute event bytes across processes (or within one).
//
// Channel keys are opaque to the Transport — the Broker chooses the key
// scheme (e.g., "enc:<encID>"). Payloads are opaque bytes — encoding is
// the Broker's concern.
type Transport interface {
    Publish(channel string, payload []byte) error
    Subscribe(channel string) (TransportSubscription, error)
}

// TransportSubscription is the per-call return from Subscribe. It exposes
// a receive channel and a Close that releases resources.
type TransportSubscription interface {
    Payloads() <-chan []byte
    Close() error
}
```

- [ ] **Step 2: Write `transport_inmem.go`**

```go
package encounter

import (
    "errors"
    "sync"
)

// InMemoryTransport is a Transport that fans out within a single process,
// suitable for tests and dev. Backed by per-channel goroutines and Go channels.
//
// Not safe to use after Close. Subscribers receive only events published
// after they subscribe (no replay).
type InMemoryTransport struct {
    mu          sync.Mutex
    closed      bool
    subscribers map[string][]chan []byte
}

func NewInMemoryTransport() *InMemoryTransport {
    return &InMemoryTransport{
        subscribers: make(map[string][]chan []byte),
    }
}

func (t *InMemoryTransport) Publish(channel string, payload []byte) error {
    t.mu.Lock()
    if t.closed {
        t.mu.Unlock()
        return errors.New("transport closed")
    }
    // Snapshot subscriber channels under lock; send outside lock so a slow
    // consumer can't stall publishers or other subscribers.
    subs := append([]chan []byte(nil), t.subscribers[channel]...)
    t.mu.Unlock()

    for _, ch := range subs {
        select {
        case ch <- payload:
        default:
            // Buffered channel full — drop. Test buffers (size 64) are
            // sized to avoid this in normal use.
        }
    }
    return nil
}

func (t *InMemoryTransport) Subscribe(channel string) (TransportSubscription, error) {
    t.mu.Lock()
    defer t.mu.Unlock()
    if t.closed {
        return nil, errors.New("transport closed")
    }
    ch := make(chan []byte, 64)
    t.subscribers[channel] = append(t.subscribers[channel], ch)
    return &inMemSubscription{
        transport: t,
        channel:   channel,
        ch:        ch,
    }, nil
}

func (t *InMemoryTransport) Close() error {
    t.mu.Lock()
    defer t.mu.Unlock()
    if t.closed {
        return nil
    }
    t.closed = true
    for _, subs := range t.subscribers {
        for _, ch := range subs {
            close(ch)
        }
    }
    t.subscribers = nil
    return nil
}

type inMemSubscription struct {
    transport *InMemoryTransport
    channel   string
    ch        chan []byte
    once      sync.Once
}

func (s *inMemSubscription) Payloads() <-chan []byte { return s.ch }

func (s *inMemSubscription) Close() error {
    s.once.Do(func() {
        s.transport.mu.Lock()
        defer s.transport.mu.Unlock()
        if s.transport.subscribers == nil {
            return // transport already fully closed
        }
        subs := s.transport.subscribers[s.channel]
        for i, ch := range subs {
            if ch == s.ch {
                s.transport.subscribers[s.channel] = append(subs[:i], subs[i+1:]...)
                close(ch)
                return
            }
        }
    })
    return nil
}
```

- [ ] **Step 3: Write `transport_inmem_test.go`**

```go
package encounter_test

import (
    "testing"
    "time"

    "github.com/KirkDiggler/rpg-toolkit/encounter"
    "github.com/stretchr/testify/suite"
)

type TransportInMemSuite struct {
    suite.Suite
    transport *encounter.InMemoryTransport
}

func TestTransportInMemSuite(t *testing.T) {
    suite.Run(t, new(TransportInMemSuite))
}

func (s *TransportInMemSuite) SetupTest() {
    s.transport = encounter.NewInMemoryTransport()
}

func (s *TransportInMemSuite) TearDownTest() {
    _ = s.transport.Close()
}

func (s *TransportInMemSuite) TestPublish_DeliversToSubscriber() {
    sub, err := s.transport.Subscribe("enc:1")
    s.Require().NoError(err)
    defer sub.Close()

    s.Require().NoError(s.transport.Publish("enc:1", []byte("hello")))

    s.assertReceives(sub, []byte("hello"))
}

func (s *TransportInMemSuite) TestPublish_FansOutByChannel() {
    sub1, _ := s.transport.Subscribe("enc:1")
    sub2, _ := s.transport.Subscribe("enc:1")
    other, _ := s.transport.Subscribe("enc:2")
    defer sub1.Close()
    defer sub2.Close()
    defer other.Close()

    s.Require().NoError(s.transport.Publish("enc:1", []byte("a")))

    s.assertReceives(sub1, []byte("a"))
    s.assertReceives(sub2, []byte("a"))
    s.assertNoReceive(other)
}

func (s *TransportInMemSuite) TestSubscribe_NoReplay() {
    s.Require().NoError(s.transport.Publish("enc:1", []byte("missed")))

    sub, _ := s.transport.Subscribe("enc:1")
    defer sub.Close()

    s.assertNoReceive(sub)
}

func (s *TransportInMemSuite) TestClose_TerminatesSubscriptions() {
    sub, _ := s.transport.Subscribe("enc:1")
    s.Require().NoError(s.transport.Close())

    select {
    case _, ok := <-sub.Payloads():
        s.False(ok, "channel should be closed")
    case <-time.After(time.Second):
        s.FailNow("channel did not close in 1s")
    }
}

func (s *TransportInMemSuite) TestPublish_AfterCloseErrors() {
    s.Require().NoError(s.transport.Close())
    s.Error(s.transport.Publish("enc:1", []byte("x")))
}

// Subscription.Close is idempotent — sync.Once guards against double-close.
func (s *TransportInMemSuite) TestSubscriptionClose_Idempotent() {
    sub, _ := s.transport.Subscribe("enc:1")
    s.Require().NoError(sub.Close())
    s.Require().NoError(sub.Close())
}

func (s *TransportInMemSuite) assertReceives(sub encounter.TransportSubscription, want []byte) {
    s.T().Helper()
    select {
    case got := <-sub.Payloads():
        s.Equal(want, got)
    case <-time.After(time.Second):
        s.FailNow("did not receive payload in 1s")
    }
}

func (s *TransportInMemSuite) assertNoReceive(sub encounter.TransportSubscription) {
    s.T().Helper()
    select {
    case got, ok := <-sub.Payloads():
        if ok {
            s.FailNowf("unexpected payload", "got %s", got)
        }
    case <-time.After(50 * time.Millisecond):
        // expected
    }
}
```

- [ ] **Step 4: Run tests**

```bash
cd /home/kirk/personal/rpg-toolkit/encounter
go test -race -run TestTransportInMemSuite ./...
```

Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add encounter/transport.go encounter/transport_inmem.go encounter/transport_inmem_test.go
git commit -m "feat(encounter): add Transport interface + InMemoryTransport

Bytes-level pluggable pub/sub. InMemoryTransport fans out within one
process via Go channels — sufficient for tests and single-process dev.
Subscriber channel sends happen outside the registry lock to prevent slow
consumers from stalling publishers. Close is idempotent via sync.Once.

Redis and Kafka transports are future slices.

Refs #<issue>"
```

---

## Task 4 — Broker (Publish, Subscribe, audience filtering)

**Goal:** The process-scoped facade. Owns the subscription registry; routes by `EncounterID()` and `Audience()`. Uses a private codec to push/pull bytes through the Transport.

**Critical fixes from review (callouts in code):**
- `Subscription.events` is closed via `sync.Once` — only the Subscription owns its close. `Broker.Close()` iterates subscriptions and calls `Sub.Close()` on each (rather than closing channels directly). No double-close panic possible.
- The listener goroutine snapshots subscribers under the broker lock and sends on their channels **outside the lock**, so a slow subscriber (or a concurrent `Subscription.Close()` waiting on the broker lock) can't stall the listener.

**Files:**
- Create: `encounter/broker.go`
- Create: `encounter/broker_test.go`

- [ ] **Step 1: Write `broker.go`**

```go
package encounter

import (
    "encoding/json"
    "errors"
    "fmt"
    "sync"

    "github.com/KirkDiggler/rpg-toolkit/encounter/events"
    "github.com/KirkDiggler/rpg-toolkit/encounter/types"
)

// Broker is the process-scoped event router. Encounters publish through it;
// gRPC stream handlers subscribe through it. Internally uses a Transport
// to distribute event bytes (locally and, in future, across processes).
type Broker struct {
    transport Transport

    mu          sync.Mutex
    subscribers map[subscriberKey][]*Subscription
    listeners   map[types.EncounterID]TransportSubscription
    closed      bool
}

type subscriberKey struct {
    EncID    types.EncounterID
    PlayerID types.PlayerID
}

func NewBroker(t Transport) *Broker {
    return &Broker{
        transport:   t,
        subscribers: make(map[subscriberKey][]*Subscription),
        listeners:   make(map[types.EncounterID]TransportSubscription),
    }
}

// Publish encodes the event and writes it to the encounter's transport channel.
// Encounters call this from inside verb methods.
func (b *Broker) Publish(evt events.EncounterEvent) error {
    payload, err := encodeEvent(evt)
    if err != nil {
        return fmt.Errorf("encode event: %w", err)
    }
    return b.transport.Publish(channelFor(evt.EncounterID()), payload)
}

// Subscribe registers a per-player subscription. The returned Subscription
// delivers only events whose Audience contains playerID. Subscriptions
// outlive any single Encounter object — the transport channel is the spine.
func (b *Broker) Subscribe(encID types.EncounterID, playerID types.PlayerID) (*Subscription, error) {
    b.mu.Lock()
    defer b.mu.Unlock()
    if b.closed {
        return nil, errors.New("broker closed")
    }

    sub := &Subscription{
        broker:   b,
        encID:    encID,
        playerID: playerID,
        events:   make(chan events.EncounterEvent, 64),
    }
    key := subscriberKey{EncID: encID, PlayerID: playerID}
    b.subscribers[key] = append(b.subscribers[key], sub)

    // First subscriber for this encounter starts the listener goroutine.
    if _, ok := b.listeners[encID]; !ok {
        ts, err := b.transport.Subscribe(channelFor(encID))
        if err != nil {
            return nil, fmt.Errorf("transport subscribe: %w", err)
        }
        b.listeners[encID] = ts
        go b.listen(encID, ts)
    }
    return sub, nil
}

// Close stops all listeners and closes all subscriptions. Idempotent.
func (b *Broker) Close() error {
    b.mu.Lock()
    if b.closed {
        b.mu.Unlock()
        return nil
    }
    b.closed = true
    listeners := b.listeners
    subs := b.subscribers
    b.listeners = nil
    b.subscribers = nil
    b.mu.Unlock()

    // Close transport listeners first — listener goroutines will exit
    // when their input channels close.
    for _, ts := range listeners {
        _ = ts.Close()
    }
    // Close subscriptions after listeners are closed. Subscription.Close()
    // is idempotent (sync.Once guards the channel close).
    for _, list := range subs {
        for _, sub := range list {
            _ = sub.Close()
        }
    }
    return nil
}

// listen runs one goroutine per encounter the broker is aware of. Decodes
// events and fans out to per-player subscribers in the audience.
//
// Subscribers are snapshotted under lock; channel sends happen OUTSIDE the
// lock so a slow subscriber can't stall the listener.
func (b *Broker) listen(encID types.EncounterID, ts TransportSubscription) {
    for payload := range ts.Payloads() {
        evt, err := decodeEvent(payload)
        if err != nil {
            // Malformed payload — skip. Future: surface a metric.
            continue
        }

        b.mu.Lock()
        // Snapshot the subscribers we need to deliver to.
        type target struct {
            sub *Subscription
        }
        var targets []target
        for _, playerID := range evt.Audience() {
            for _, sub := range b.subscribers[subscriberKey{EncID: encID, PlayerID: playerID}] {
                targets = append(targets, target{sub: sub})
            }
        }
        b.mu.Unlock()

        // Send outside the lock — slow consumer doesn't stall others.
        for _, t := range targets {
            select {
            case t.sub.events <- evt:
            default:
                // Subscriber buffer full — drop. Tests size buffers high enough.
            }
        }
    }
}

// channelFor returns the transport channel key for an encounter.
func channelFor(encID types.EncounterID) string {
    return "enc:" + string(encID)
}

// Subscription is a per-player view onto an encounter's event stream.
type Subscription struct {
    broker   *Broker
    encID    types.EncounterID
    playerID types.PlayerID
    events   chan events.EncounterEvent
    once     sync.Once
}

func (s *Subscription) Events() <-chan events.EncounterEvent { return s.events }

// Close removes this subscription from the broker registry and closes
// the events channel. Idempotent — only the Subscription owns its close,
// guarded by sync.Once. Broker.Close() calls this; nobody else closes
// s.events directly.
func (s *Subscription) Close() error {
    s.once.Do(func() {
        s.broker.mu.Lock()
        if s.broker.subscribers != nil {
            key := subscriberKey{EncID: s.encID, PlayerID: s.playerID}
            subs := s.broker.subscribers[key]
            for i, sub := range subs {
                if sub == s {
                    s.broker.subscribers[key] = append(subs[:i], subs[i+1:]...)
                    break
                }
            }
        }
        s.broker.mu.Unlock()
        close(s.events)
    })
    return nil
}

// --- Codec (private to broker) ---
//
// Wire format is JSON with a top-level "_type" discriminator. Concrete event
// types do not see it. Future Transport implementations can substitute a
// different codec without changing event types.

type wireEnvelope struct {
    Type    string          `json:"_type"`
    Payload json.RawMessage `json:"payload"`
}

func encodeEvent(evt events.EncounterEvent) ([]byte, error) {
    var typeName string
    switch evt.(type) {
    case *events.MoveEvent:
        typeName = "MoveEvent"
    case *events.HexRevealedEvent:
        typeName = "HexRevealedEvent"
    case *events.DoorOpenedEvent:
        typeName = "DoorOpenedEvent"
    default:
        return nil, fmt.Errorf("unknown event type %T", evt)
    }
    payload, err := json.Marshal(evt) // each concrete provides MarshalJSON
    if err != nil {
        return nil, err
    }
    return json.Marshal(wireEnvelope{Type: typeName, Payload: payload})
}

func decodeEvent(b []byte) (events.EncounterEvent, error) {
    var env wireEnvelope
    if err := json.Unmarshal(b, &env); err != nil {
        return nil, err
    }
    switch env.Type {
    case "MoveEvent":
        var e events.MoveEvent
        if err := json.Unmarshal(env.Payload, &e); err != nil {
            return nil, err
        }
        return &e, nil
    case "HexRevealedEvent":
        var e events.HexRevealedEvent
        if err := json.Unmarshal(env.Payload, &e); err != nil {
            return nil, err
        }
        return &e, nil
    case "DoorOpenedEvent":
        var e events.DoorOpenedEvent
        if err := json.Unmarshal(env.Payload, &e); err != nil {
            return nil, err
        }
        return &e, nil
    default:
        return nil, fmt.Errorf("unknown event type %q", env.Type)
    }
}
```

- [ ] **Step 2: Write `broker_test.go`**

```go
package encounter_test

import (
    "fmt"
    "testing"
    "time"

    "github.com/KirkDiggler/rpg-toolkit/encounter"
    "github.com/KirkDiggler/rpg-toolkit/encounter/events"
    "github.com/KirkDiggler/rpg-toolkit/encounter/types"
    "github.com/stretchr/testify/suite"
)

type BrokerSuite struct {
    suite.Suite
    transport *encounter.InMemoryTransport
    broker    *encounter.Broker
}

func TestBrokerSuite(t *testing.T) {
    suite.Run(t, new(BrokerSuite))
}

func (s *BrokerSuite) SetupTest() {
    s.transport = encounter.NewInMemoryTransport()
    s.broker = encounter.NewBroker(s.transport)
}

func (s *BrokerSuite) TearDownTest() {
    _ = s.broker.Close()
    _ = s.transport.Close()
}

// Subscribers in audience receive; those outside don't.
func (s *BrokerSuite) TestPublish_RoutesByAudience() {
    aliceSub, err := s.broker.Subscribe("enc:1", "alice")
    s.Require().NoError(err)
    bobSub, err := s.broker.Subscribe("enc:1", "bob")
    s.Require().NoError(err)

    move := events.NewMoveEvent("enc:1", 1, "monster-1",
        []types.Hex{{Q: 0, R: 0, S: 0}},
        map[types.PlayerID]events.MovePlayerSlice{
            "alice": {SeenSegments: []types.Hex{{Q: 0, R: 0, S: 0}}},
            // bob absent — out of audience
        },
    )
    s.Require().NoError(s.broker.Publish(move))

    s.assertReceivesType(aliceSub, "*events.MoveEvent")
    s.assertNoReceive(bobSub)
}

// Cross-encounter isolation.
func (s *BrokerSuite) TestPublish_IsolatesEncounters() {
    sub1, _ := s.broker.Subscribe("enc:1", "alice")
    sub2, _ := s.broker.Subscribe("enc:2", "alice")

    s.Require().NoError(s.broker.Publish(events.NewMoveEvent("enc:1", 1, "x",
        nil, map[types.PlayerID]events.MovePlayerSlice{"alice": {}})))

    s.assertReceivesType(sub1, "*events.MoveEvent")
    s.assertNoReceive(sub2)
}

// Two subscriptions for the same player both receive.
func (s *BrokerSuite) TestSubscribe_MultiSubsForSamePlayer() {
    a1, _ := s.broker.Subscribe("enc:1", "alice")
    a2, _ := s.broker.Subscribe("enc:1", "alice")

    s.Require().NoError(s.broker.Publish(events.NewMoveEvent("enc:1", 1, "x",
        nil, map[types.PlayerID]events.MovePlayerSlice{"alice": {}})))

    s.assertReceivesType(a1, "*events.MoveEvent")
    s.assertReceivesType(a2, "*events.MoveEvent")
}

// Closing one subscription doesn't affect siblings; routing continues.
func (s *BrokerSuite) TestClose_RemovesOnlyClosedSub() {
    a1, _ := s.broker.Subscribe("enc:1", "alice")
    a2, _ := s.broker.Subscribe("enc:1", "alice")
    s.Require().NoError(a1.Close())

    s.Require().NoError(s.broker.Publish(events.NewMoveEvent("enc:1", 1, "x",
        nil, map[types.PlayerID]events.MovePlayerSlice{"alice": {}})))

    // a2 still receives — the meaningful assertion.
    s.assertReceivesType(a2, "*events.MoveEvent")
    // a1's channel is closed; reading returns zero value with !ok.
    select {
    case _, ok := <-a1.Events():
        s.False(ok, "a1's channel should be closed")
    case <-time.After(50 * time.Millisecond):
        s.FailNow("a1's channel did not close")
    }
}

// Subscription.Close is idempotent (sync.Once).
func (s *BrokerSuite) TestSubscriptionClose_Idempotent() {
    sub, _ := s.broker.Subscribe("enc:1", "alice")
    s.Require().NoError(sub.Close())
    s.Require().NoError(sub.Close())
}

// Broker.Close closes all subscriptions; calling Subscription.Close after
// Broker.Close is still safe (no double-close panic).
func (s *BrokerSuite) TestBrokerClose_ClosesAllSubsSafely() {
    sub, _ := s.broker.Subscribe("enc:1", "alice")
    s.Require().NoError(s.broker.Close())

    // Channel should be closed.
    select {
    case _, ok := <-sub.Events():
        s.False(ok)
    case <-time.After(50 * time.Millisecond):
        s.FailNow("sub channel did not close after broker close")
    }

    // Calling Subscription.Close after Broker.Close must not panic.
    s.Require().NoError(sub.Close())
}

// Helpers
func (s *BrokerSuite) assertReceivesType(sub *encounter.Subscription, want string) {
    s.T().Helper()
    select {
    case evt, ok := <-sub.Events():
        s.Require().True(ok, "channel closed unexpectedly")
        s.Equal(want, fmt.Sprintf("%T", evt))
    case <-time.After(time.Second):
        s.FailNow("did not receive event in 1s")
    }
}

func (s *BrokerSuite) assertNoReceive(sub *encounter.Subscription) {
    s.T().Helper()
    select {
    case evt, ok := <-sub.Events():
        if ok {
            s.FailNowf("unexpected event", "got %T", evt)
        }
    case <-time.After(50 * time.Millisecond):
        // expected
    }
}
```

- [ ] **Step 3: Run tests**

```bash
cd /home/kirk/personal/rpg-toolkit/encounter
go test -race -run TestBrokerSuite ./...
```

Expected: PASS.

- [ ] **Step 4: Commit**

```bash
git add encounter/broker.go encounter/broker_test.go
git commit -m "feat(encounter): add Broker with audience filtering + JSON codec

Process-scoped Broker over Transport. Per-player subscriptions; routes by
EncounterID and Audience derived from the event's PerPlayer keys. Subscription
ownership of channel close is enforced via sync.Once — Broker.Close calls
Subscription.Close on each rather than closing channels directly. Listener
goroutine snapshots subscribers under lock and sends outside lock.

JSON codec is private to the broker (bytes through transport, typed events
above and below). Each concrete event provides MarshalJSON/UnmarshalJSON so
unexported encID/seq round-trip without exposing fields.

Refs #<issue>"
```

---

## Task 5 — `encounter/perception/` subpackage

**Goal:** Pure projection functions and `PerceptionView` value type. Stub LoS (Manhattan radius); real LoS is a future slice.

**Files:**
- Create: `encounter/perception/doc.go`
- Create: `encounter/perception/view.go`
- Create: `encounter/perception/los_stub.go`
- Create: `encounter/perception/project.go`
- Create: `encounter/perception/project_test.go`

- [ ] **Step 1: Write `perception/doc.go`**

```go
// Package perception computes per-player vision projections for the
// encounter SDK. Pure functions, testable in isolation, no broker or
// transport dependencies.
//
// Slice 1 ships a Manhattan-radius stub for line-of-sight. Real LoS (with
// walls, lighting, and senses like darkvision/blindsight/blinded
// conditions) is a future slice under the same package.
//
// Lives as a subpackage of encounter for slice 1 to keep the module graph
// acyclic. Future slices may extract this to rpg-toolkit/perception/ once
// shared primitive types (Hex, etc.) move to tools/spatial.
package perception
```

- [ ] **Step 2: Write `perception/view.go`**

`PerceptionView` carries the spec's full shape — `KnownEntities`, `ActiveSenses`, `Conditions` are present as zero-value stubs with future-slice comments so persisted JSON is forward-compatible.

```go
package perception

import "github.com/KirkDiggler/rpg-toolkit/encounter/types"

// PerceptionView is what a single player currently knows about an encounter.
// Persisted on EncounterData; rehydrated on LoadFromData.
//
// Slice 1 only uses Position, SightRange, RevealedHexes. The remaining
// fields are reserved for shape stability — when conditions, senses, and
// entity-knowledge accumulation land in future slices, persisted JSON
// won't need a migration.
type PerceptionView struct {
    PlayerID      types.PlayerID                       `json:"player_id"`
    Position      types.Hex                            `json:"position"`
    SightRange    int                                  `json:"sight_range"`
    RevealedHexes types.HexSet                         `json:"revealed_hexes"`

    // Future-slice fields — emitted as zero values for now.
    KnownEntities map[types.EntityID]EntityKnowledge   `json:"known_entities,omitempty"`
    ActiveSenses  []Sense                              `json:"active_senses,omitempty"`
    Conditions    []types.EntityID                     `json:"conditions,omitempty"`
}

// EntityKnowledge is reserved for entity-visibility accumulation in future slices.
type EntityKnowledge struct {
    LastSeenPosition types.Hex `json:"last_seen_position"`
    Identified       bool      `json:"identified"`
}

// Sense is reserved for senses (darkvision, blindsight, ...) in future slices.
type Sense struct {
    Kind  string `json:"kind"`
    Range int    `json:"range"`
}

func NewView(playerID types.PlayerID, position types.Hex, sightRange int) *PerceptionView {
    return &PerceptionView{
        PlayerID:      playerID,
        Position:      position,
        SightRange:    sightRange,
        RevealedHexes: make(types.HexSet),
    }
}

// ApplyReveal merges newly-revealed hexes into the cumulative set. Idempotent.
func (v *PerceptionView) ApplyReveal(hexes types.HexSet) {
    if v.RevealedHexes == nil {
        v.RevealedHexes = make(types.HexSet)
    }
    for h := range hexes {
        v.RevealedHexes[h] = struct{}{}
    }
}
```

- [ ] **Step 3: Write `perception/los_stub.go`**

```go
package perception

import "github.com/KirkDiggler/rpg-toolkit/encounter/types"

// HexDistance is the cube-coordinate hex distance between two hexes.
// Exported for use by the encounter package's verbs.
func HexDistance(a, b types.Hex) int {
    dq := abs(a.Q - b.Q)
    dr := abs(a.R - b.R)
    ds := abs(a.S - b.S)
    if dq > dr {
        if dq > ds {
            return dq
        }
        return ds
    }
    if dr > ds {
        return dr
    }
    return ds
}

// VisibleHexesAt returns the hexes within sightRange of from, including from.
// STUB: ignores walls, lighting, conditions. Replaced with real LoS in a
// future slice.
func VisibleHexesAt(from types.Hex, sightRange int) types.HexSet {
    out := make(types.HexSet)
    for dq := -sightRange; dq <= sightRange; dq++ {
        for dr := -sightRange; dr <= sightRange; dr++ {
            ds := -dq - dr
            // dq and dr are in range by construction; the only filter is ds.
            if abs(ds) > sightRange {
                continue
            }
            h := types.Hex{Q: from.Q + dq, R: from.R + dr, S: from.S + ds}
            out[h] = struct{}{}
        }
    }
    return out
}

// HexNeighbors returns the six adjacent hexes (cube coords).
func HexNeighbors(h types.Hex) []types.Hex {
    return []types.Hex{
        {Q: h.Q + 1, R: h.R - 1, S: h.S},
        {Q: h.Q + 1, R: h.R, S: h.S - 1},
        {Q: h.Q, R: h.R + 1, S: h.S - 1},
        {Q: h.Q - 1, R: h.R + 1, S: h.S},
        {Q: h.Q - 1, R: h.R, S: h.S + 1},
        {Q: h.Q, R: h.R - 1, S: h.S + 1},
    }
}

func abs(x int) int {
    if x < 0 {
        return -x
    }
    return x
}
```

- [ ] **Step 4: Write `perception/project.go`**

```go
package perception

import (
    "github.com/KirkDiggler/rpg-toolkit/encounter/events"
    "github.com/KirkDiggler/rpg-toolkit/encounter/types"
)

// ProjectMove computes a viewer's move slice and reveal slice when an entity
// moves along path. Returns (moveSlice, revealSlice). Either may be nil if
// the viewer perceives nothing of the move or has no vision change.
//
// Slice 1 stub: viewer's visibility is computed from their CURRENT position.
// Real LoS will be position-aware-per-segment. Slice 1 does NOT emit
// EntityVisibility — entity-knowledge accumulation is a future slice.
func ProjectMove(
    mover types.EntityID,
    path []types.Hex,
    viewer *PerceptionView,
) (moveSlice *events.MovePlayerSlice, revealSlice *events.HexRevealedSlice) {

    if viewer == nil || len(path) == 0 {
        return nil, nil
    }
    visible := VisibleHexesAt(viewer.Position, viewer.SightRange)

    var seen []types.Hex
    for _, hex := range path {
        if visible.Has(hex) {
            seen = append(seen, hex)
        }
    }
    if len(seen) > 0 {
        moveSlice = &events.MovePlayerSlice{SeenSegments: seen}
    }
    // For someone-else's move from this viewer's perspective, the viewer's
    // own position didn't change, so under the stub LoS no new hexes are
    // revealed (the visible set is the same). Future slices handle the
    // mover-as-self case (see encounter.Move) and entity-visibility deltas.
    return moveSlice, nil
}

// ProjectDoorOpen computes per-viewer slices when a door opens.
// Stub LoS: opening a door doesn't change which hexes the viewer can see
// (no walls modeled), but if the door is in the viewer's sight range we
// emit a DoorOpenedPlayerSlice. The reveal slice covers the door's
// immediate neighbors that the viewer hadn't seen before.
func ProjectDoorOpen(
    door types.EntityID,
    doorPos types.Hex,
    openedBy types.EntityID,
    viewer *PerceptionView,
) (doorSlice *events.DoorOpenedPlayerSlice, revealSlice *events.HexRevealedSlice) {

    if viewer == nil {
        return nil, nil
    }
    visible := VisibleHexesAt(viewer.Position, viewer.SightRange)
    if !visible.Has(doorPos) {
        return nil, nil
    }

    doorSlice = &events.DoorOpenedPlayerSlice{Visible: true}

    newHexes := make(types.HexSet)
    for _, neighbor := range HexNeighbors(doorPos) {
        if visible.Has(neighbor) && !viewer.RevealedHexes.Has(neighbor) {
            newHexes[neighbor] = struct{}{}
        }
    }
    if len(newHexes) > 0 {
        revealSlice = &events.HexRevealedSlice{Hexes: newHexes}
    }
    return doorSlice, revealSlice
}
```

- [ ] **Step 5: Write `perception/project_test.go`**

```go
package perception_test

import (
    "testing"

    "github.com/KirkDiggler/rpg-toolkit/encounter/perception"
    "github.com/KirkDiggler/rpg-toolkit/encounter/types"
    "github.com/stretchr/testify/suite"
)

type ProjectSuite struct {
    suite.Suite
}

func TestProjectSuite(t *testing.T) {
    suite.Run(t, new(ProjectSuite))
}

func (s *ProjectSuite) TestProjectMove_ViewerInRange() {
    viewer := perception.NewView("alice", types.Hex{}, 5)

    path := []types.Hex{
        {Q: 1, R: 0, S: -1},
        {Q: 2, R: 0, S: -2},
        {Q: 3, R: 0, S: -3},
    }
    moveSlice, _ := perception.ProjectMove("bob", path, viewer)

    s.Require().NotNil(moveSlice)
    s.Equal(path, moveSlice.SeenSegments)
}

func (s *ProjectSuite) TestProjectMove_ViewerOutOfRange() {
    viewer := perception.NewView("alice", types.Hex{}, 2)

    path := []types.Hex{
        {Q: 5, R: -2, S: -3},
        {Q: 6, R: -2, S: -4},
    }
    moveSlice, revealSlice := perception.ProjectMove("bob", path, viewer)

    s.Nil(moveSlice)
    s.Nil(revealSlice)
}

func (s *ProjectSuite) TestProjectDoorOpen_ViewerNearDoor() {
    viewer := perception.NewView("alice", types.Hex{}, 3)

    doorPos := types.Hex{Q: 2, R: 0, S: -2}
    doorSlice, revealSlice := perception.ProjectDoorOpen("door-1", doorPos, "bob", viewer)

    s.Require().NotNil(doorSlice)
    s.True(doorSlice.Visible)
    s.Require().NotNil(revealSlice)
    s.NotEmpty(revealSlice.Hexes)
}

func (s *ProjectSuite) TestProjectDoorOpen_ViewerOutOfRange() {
    viewer := perception.NewView("alice", types.Hex{}, 1)

    doorPos := types.Hex{Q: 5, R: -2, S: -3}
    doorSlice, revealSlice := perception.ProjectDoorOpen("door-1", doorPos, "bob", viewer)

    s.Nil(doorSlice)
    s.Nil(revealSlice)
}

func (s *ProjectSuite) TestPerceptionView_ApplyRevealIdempotent() {
    viewer := perception.NewView("alice", types.Hex{}, 3)
    h := types.Hex{Q: 1, R: 0, S: -1}

    viewer.ApplyReveal(types.NewHexSet(h))
    viewer.ApplyReveal(types.NewHexSet(h))

    s.Len(viewer.RevealedHexes, 1)
    s.True(viewer.RevealedHexes.Has(h))
}
```

- [ ] **Step 6: Run tests**

```bash
cd /home/kirk/personal/rpg-toolkit/encounter
go test -race ./perception/...
```

Expected: PASS.

- [ ] **Step 7: Commit**

```bash
git add encounter/perception/
git commit -m "feat(encounter/perception): PerceptionView + projection with stub LoS

Subpackage of encounter (one module) — keeps the module graph acyclic.
PerceptionView includes future-slice fields (KnownEntities, ActiveSenses,
Conditions) as zero-value stubs so persisted JSON is forward-compatible.
Stub LoS uses Manhattan radius (no walls, no senses, no conditions); real
LoS is a future slice. ProjectMove returns nil reveal slice for slice 1
(entity-visibility accumulation deferred).

Refs #<issue>"
```

---

## Task 6 — Encounter aggregate: New, AddPlayer, AddDoor, ToData/LoadFromData, SnapshotFor

**Goal:** Transient encounter object. Round-trips cleanly through `EncounterData`. No verbs yet; those come in Tasks 7 and 8.

**Files:**
- Create: `encounter/data.go`
- Create: `encounter/encounter.go`
- Create: `encounter/encounter_test.go`

- [ ] **Step 1: Write `data.go`**

```go
package encounter

import (
    "github.com/KirkDiggler/rpg-toolkit/encounter/perception"
    "github.com/KirkDiggler/rpg-toolkit/encounter/types"
)

// EncounterData is the persisted shape of an Encounter. The orchestrator
// stores this in Redis (or any KV store) and rehydrates the live Encounter
// via LoadFromData.
//
// Slice 1 persists what's needed for Move and OpenDoor: identity, players
// (with position + perception view), doors, and a sequence counter.
// Future slices add: monsters, action economy, turn state, mode, conditions.
type EncounterData struct {
    ID       types.EncounterID                         `json:"id"`
    Sequence uint64                                    `json:"sequence"`
    Players  map[types.PlayerID]*PlayerData            `json:"players"`
    Doors    map[types.EntityID]*DoorData              `json:"doors"`
}

type PlayerData struct {
    ID       types.PlayerID              `json:"id"`
    EntityID types.EntityID              `json:"entity_id"`
    View     *perception.PerceptionView  `json:"view"`
}

type DoorData struct {
    ID       types.EntityID `json:"id"`
    Position types.Hex      `json:"position"`
    Open     bool           `json:"open"`
}

func NewEncounterData(id types.EncounterID) *EncounterData {
    return &EncounterData{
        ID:      id,
        Players: make(map[types.PlayerID]*PlayerData),
        Doors:   make(map[types.EntityID]*DoorData),
    }
}
```

- [ ] **Step 2: Write `encounter.go`**

```go
package encounter

import (
    "errors"
    "fmt"

    "github.com/KirkDiggler/rpg-toolkit/encounter/perception"
    "github.com/KirkDiggler/rpg-toolkit/encounter/types"
)

// Encounter is the transient SDK object for one ongoing encounter.
// Constructed per-call via LoadFromData; mutated by verbs; serialized via
// ToData and saved.
type Encounter struct {
    data   *EncounterData
    broker *Broker
}

// PlayerInput populates a player seat at construction / AddPlayer time.
type PlayerInput struct {
    PlayerID   types.PlayerID
    EntityID   types.EntityID
    Position   types.Hex
    SightRange int
}

func New(id types.EncounterID, b *Broker) *Encounter {
    return &Encounter{
        data:   NewEncounterData(id),
        broker: b,
    }
}

func LoadFromData(data *EncounterData, b *Broker) (*Encounter, error) {
    if data == nil {
        return nil, errors.New("nil EncounterData")
    }
    if data.Players == nil {
        data.Players = make(map[types.PlayerID]*PlayerData)
    }
    if data.Doors == nil {
        data.Doors = make(map[types.EntityID]*DoorData))
    }
    return &Encounter{data: data, broker: b}, nil
}

// AddPlayer registers a new player seat with a fresh PerceptionView.
// The player sees their starting position immediately.
func (e *Encounter) AddPlayer(input PlayerInput) error {
    if _, exists := e.data.Players[input.PlayerID]; exists {
        return fmt.Errorf("player %q already in encounter", input.PlayerID)
    }
    view := perception.NewView(input.PlayerID, input.Position, input.SightRange)
    view.ApplyReveal(perception.VisibleHexesAt(input.Position, input.SightRange))

    e.data.Players[input.PlayerID] = &PlayerData{
        ID:       input.PlayerID,
        EntityID: input.EntityID,
        View:     view,
    }
    return nil
}

// AddDoor registers a door (slice scope; future slices use a richer entity system).
func (e *Encounter) AddDoor(id types.EntityID, position types.Hex, open bool) {
    e.data.Doors[id] = &DoorData{ID: id, Position: position, Open: open}
}

// ID returns the encounter's identifier.
func (e *Encounter) ID() types.EncounterID { return e.data.ID }

// SnapshotFor returns the read-only view a player's gRPC handler ships
// on connect/reconnect.
func (e *Encounter) SnapshotFor(playerID types.PlayerID) Snapshot {
    p, ok := e.data.Players[playerID]
    if !ok || p.View == nil {
        return Snapshot{}
    }
    revealed := make(types.HexSet, len(p.View.RevealedHexes))
    for h := range p.View.RevealedHexes {
        revealed[h] = struct{}{}
    }
    return Snapshot{
        PlayerID:      playerID,
        Position:      p.View.Position,
        RevealedHexes: revealed,
    }
}

// Snapshot is the slice-1 read-only view. Future slices add visible
// entities, turn state, action economy, etc.
type Snapshot struct {
    PlayerID      types.PlayerID
    Position      types.Hex
    RevealedHexes types.HexSet
}

func (e *Encounter) ToData() *EncounterData { return e.data }

// nextSeq advances and returns the encounter's monotonic sequence counter.
func (e *Encounter) nextSeq() uint64 {
    e.data.Sequence++
    return e.data.Sequence
}
```

> **Note: there's a typo in the literal Step 2 code above** — `make(map[types.EntityID]*DoorData))` has an extra closing paren. The implementer should write `make(map[types.EntityID]*DoorData)`. Flagged here so the agent doesn't copy the typo.

- [ ] **Step 3: Write `encounter_test.go` (Task 6 portion)**

```go
package encounter_test

import (
    "encoding/json"
    "testing"

    "github.com/KirkDiggler/rpg-toolkit/encounter"
    "github.com/KirkDiggler/rpg-toolkit/encounter/types"
    "github.com/stretchr/testify/suite"
)

type EncounterSuite struct {
    suite.Suite
    transport *encounter.InMemoryTransport
    broker    *encounter.Broker
}

func TestEncounterSuite(t *testing.T) {
    suite.Run(t, new(EncounterSuite))
}

func (s *EncounterSuite) SetupTest() {
    s.transport = encounter.NewInMemoryTransport()
    s.broker = encounter.NewBroker(s.transport)
}

func (s *EncounterSuite) TearDownTest() {
    _ = s.broker.Close()
    _ = s.transport.Close()
}

func (s *EncounterSuite) TestAddPlayer_PopulatesView() {
    e := encounter.New("enc-1", s.broker)
    s.Require().NoError(e.AddPlayer(encounter.PlayerInput{
        PlayerID:   "alice",
        EntityID:   "char-alice",
        Position:   types.Hex{Q: 0, R: 0, S: 0},
        SightRange: 3,
    }))

    snap := e.SnapshotFor("alice")
    s.Equal(types.PlayerID("alice"), snap.PlayerID)
    s.Equal(types.Hex{}, snap.Position)
    s.True(snap.RevealedHexes.Has(types.Hex{}))
}

func (s *EncounterSuite) TestAddPlayer_RejectsDuplicate() {
    e := encounter.New("enc-1", s.broker)
    input := encounter.PlayerInput{PlayerID: "alice", EntityID: "char-1", SightRange: 3}
    s.Require().NoError(e.AddPlayer(input))
    s.Error(e.AddPlayer(input))
}

// ToData / LoadFromData round-trips through JSON cleanly. This test is
// load-bearing because earlier slices of this design failed JSON round-trip
// for HexSet (struct map keys) — the types subpackage's MarshalJSON now
// fixes that and this test guards against regression.
func (s *EncounterSuite) TestRoundTrip_ToDataLoadFromData() {
    e1 := encounter.New("enc-1", s.broker)
    s.Require().NoError(e1.AddPlayer(encounter.PlayerInput{
        PlayerID: "alice", EntityID: "char-1",
        Position: types.Hex{Q: 1, R: -1, S: 0}, SightRange: 5,
    }))
    e1.AddDoor("door-1", types.Hex{Q: 2, R: 0, S: -2}, false)

    payload, err := json.Marshal(e1.ToData())
    s.Require().NoError(err)

    var data encounter.EncounterData
    s.Require().NoError(json.Unmarshal(payload, &data))

    e2, err := encounter.LoadFromData(&data, s.broker)
    s.Require().NoError(err)

    s.Equal(types.EncounterID("enc-1"), e2.ID())
    snap := e2.SnapshotFor("alice")
    s.Equal(types.Hex{Q: 1, R: -1, S: 0}, snap.Position)
    s.True(snap.RevealedHexes.Has(types.Hex{Q: 1, R: -1, S: 0}),
        "RevealedHexes must round-trip — guards against HexSet JSON regression")
}

func (s *EncounterSuite) TestSnapshotFor_UnknownPlayer() {
    e := encounter.New("enc-1", s.broker)
    snap := e.SnapshotFor("nobody")
    s.Equal(encounter.Snapshot{}, snap)
}
```

- [ ] **Step 4: Run tests, commit**

```bash
cd /home/kirk/personal/rpg-toolkit/encounter
go test -race -run TestEncounterSuite ./...
```

Expected: PASS.

```bash
git add encounter/data.go encounter/encounter.go encounter/encounter_test.go
git commit -m "feat(encounter): add Encounter aggregate, AddPlayer, AddDoor, ToData/LoadFromData, SnapshotFor

Transient encounter object with data-in/data-out persistence shape.
EncounterData carries players (with PerceptionView), doors, and a
monotonic sequence counter. SnapshotFor returns a player's read-only
view used by stream-handler snapshot-on-connect.

Round-trip test guards against HexSet JSON-serialization regression.

Refs #<issue>"
```

---

## Task 7 — `Encounter.Move` verb

**Goal:** First action verb. Validates, mutates state, computes per-player projections, publishes `MoveEvent` and (when vision changed) `HexRevealedEvent`.

**Critical fix from review:** the mover's reveal delta is computed BEFORE applying the new visibility to their view, otherwise the diff is always empty. The order is: (1) compute new visibility from new position; (2) compute delta against current `RevealedHexes`; (3) emit reveal slice using the delta; (4) THEN apply delta to `RevealedHexes`.

**Files:**
- Modify: `encounter/encounter.go` (add `Move`)
- Modify: `encounter/encounter_test.go` (add Move tests)

- [ ] **Step 1: Add Move + helpers to `encounter.go`**

Add imports:

```go
"github.com/KirkDiggler/rpg-toolkit/encounter/events"
```

Add the verb:

```go
// Move applies a move action by playerID along path. Validates, mutates
// player position, and publishes the cause event (MoveEvent) plus a
// HexRevealedEvent for any viewer whose vision grew.
//
// Slice scope: no action economy, no turn-order enforcement, no
// path-contiguity validation beyond non-empty.
func (e *Encounter) Move(playerID types.PlayerID, path []types.Hex) error {
    if len(path) == 0 {
        return errors.New("empty path")
    }
    p, ok := e.data.Players[playerID]
    if !ok {
        return fmt.Errorf("player %q not in encounter", playerID)
    }

    // 1. Compute the mover's reveal delta BEFORE mutating position/view.
    //    The delta = (visible-from-new-position) MINUS (already-revealed).
    end := path[len(path)-1]
    newVisible := perception.VisibleHexesAt(end, p.View.SightRange)
    moverNewHexes := diffHexes(p.View.RevealedHexes, newVisible)

    // 2. Mutate state: position, then apply the reveal delta we just computed.
    p.View.Position = end
    p.View.ApplyReveal(moverNewHexes)

    // 3. Per-player projection.
    movePerPlayer := make(map[types.PlayerID]events.MovePlayerSlice)
    revealPerPlayer := make(map[types.PlayerID]events.HexRevealedSlice)

    // The mover always sees their own move; their reveal is the delta we
    // just computed.
    movePerPlayer[playerID] = events.MovePlayerSlice{SeenSegments: append([]types.Hex(nil), path...)}
    if len(moverNewHexes) > 0 {
        revealPerPlayer[playerID] = events.HexRevealedSlice{Hexes: moverNewHexes}
    }

    // Other players: project the move from their current view.
    for otherID, other := range e.data.Players {
        if otherID == playerID {
            continue
        }
        moveSlice, revealSlice := perception.ProjectMove(p.EntityID, path, other.View)
        if moveSlice != nil {
            movePerPlayer[otherID] = *moveSlice
        }
        if revealSlice != nil {
            if revealSlice.Hexes != nil {
                other.View.ApplyReveal(revealSlice.Hexes)
            }
            revealPerPlayer[otherID] = *revealSlice
        }
    }

    // 4. Publish — cause event always; effect event only when someone's
    //    vision changed. The two events get sequential sequence numbers.
    if err := e.broker.Publish(events.NewMoveEvent(
        e.data.ID, e.nextSeq(), p.EntityID, path, movePerPlayer,
    )); err != nil {
        return fmt.Errorf("publish move: %w", err)
    }
    if len(revealPerPlayer) > 0 {
        if err := e.broker.Publish(events.NewHexRevealedEvent(
            e.data.ID, e.nextSeq(), revealPerPlayer,
        )); err != nil {
            return fmt.Errorf("publish reveal: %w", err)
        }
    }
    return nil
}

// diffHexes returns hexes in candidate that are not already in current.
func diffHexes(current, candidate types.HexSet) types.HexSet {
    out := make(types.HexSet)
    for h := range candidate {
        if !current.Has(h) {
            out[h] = struct{}{}
        }
    }
    return out
}
```

- [ ] **Step 2: Add Move tests to `encounter_test.go`**

(Add `import "time"` and `import "fmt"` and `"github.com/KirkDiggler/rpg-toolkit/encounter/events"` to the test file.)

```go
// Move publishes MoveEvent. Mover and viewers in range get a slice; viewers
// out of range are absent from PerPlayer.
func (s *EncounterSuite) TestMove_PublishesMoveEvent() {
    e := encounter.New("enc-1", s.broker)
    s.Require().NoError(e.AddPlayer(encounter.PlayerInput{
        PlayerID: "alice", EntityID: "char-alice",
        Position: types.Hex{}, SightRange: 5,
    }))
    s.Require().NoError(e.AddPlayer(encounter.PlayerInput{
        PlayerID: "bob", EntityID: "char-bob",
        Position: types.Hex{Q: 50, R: -25, S: -25}, SightRange: 5,
    }))

    aliceSub, _ := s.broker.Subscribe("enc-1", "alice")
    bobSub, _ := s.broker.Subscribe("enc-1", "bob")

    path := []types.Hex{
        {Q: 1, R: 0, S: -1},
        {Q: 2, R: 0, S: -2},
    }
    s.Require().NoError(e.Move("alice", path))

    // Alice (mover) gets MoveEvent.
    s.assertReceivesType(aliceSub, "*events.MoveEvent")
    // Bob (out of range) gets nothing.
    s.assertNoReceive(bobSub)
}

// Move publishes HexRevealedEvent when the mover's vision grew. This test
// guards against a regression where the delta was computed AFTER applying
// reveal — making the delta always empty.
func (s *EncounterSuite) TestMove_PublishesHexRevealedEventForMover() {
    e := encounter.New("enc-1", s.broker)
    s.Require().NoError(e.AddPlayer(encounter.PlayerInput{
        PlayerID: "alice", EntityID: "char-alice",
        Position: types.Hex{}, SightRange: 2,
    }))
    aliceSub, _ := s.broker.Subscribe("enc-1", "alice")

    path := []types.Hex{{Q: 1, R: 0, S: -1}}
    s.Require().NoError(e.Move("alice", path))

    seen := collectTypes(aliceSub, 500*time.Millisecond)
    s.Contains(seen, "*events.MoveEvent")
    s.Contains(seen, "*events.HexRevealedEvent")
}

// Move validations.
func (s *EncounterSuite) TestMove_Validations() {
    e := encounter.New("enc-1", s.broker)
    s.Require().NoError(e.AddPlayer(encounter.PlayerInput{
        PlayerID: "alice", EntityID: "char-1", SightRange: 3,
    }))

    s.Error(e.Move("alice", nil), "empty path should error")
    s.Error(e.Move("nobody", []types.Hex{{}}), "unknown player should error")
}

// Helpers used across encounter_test.go and integration_test.go.
// (Placed here so they're shared within the encounter_test package.)
func (s *EncounterSuite) assertReceivesType(sub *encounter.Subscription, want string) {
    s.T().Helper()
    select {
    case evt, ok := <-sub.Events():
        s.Require().True(ok)
        s.Equal(want, fmt.Sprintf("%T", evt))
    case <-time.After(time.Second):
        s.FailNow("did not receive event in 1s")
    }
}

func (s *EncounterSuite) assertNoReceive(sub *encounter.Subscription) {
    s.T().Helper()
    select {
    case evt, ok := <-sub.Events():
        if ok {
            s.FailNowf("unexpected event", "got %T", evt)
        }
    case <-time.After(50 * time.Millisecond):
        // expected
    }
}

func collectTypes(sub *encounter.Subscription, timeout time.Duration) []string {
    var out []string
    deadline := time.After(timeout)
    for {
        select {
        case evt, ok := <-sub.Events():
            if !ok {
                return out
            }
            out = append(out, fmt.Sprintf("%T", evt))
        case <-deadline:
            return out
        }
    }
}
```

> If `BrokerSuite` already defines `assertReceivesType` / `assertNoReceive` in `broker_test.go`, that's fine — both suites are in `package encounter_test` but each suite's methods are on its own type and don't conflict. The free function `collectTypes` is shared.

- [ ] **Step 3: Run tests, commit**

```bash
cd /home/kirk/personal/rpg-toolkit/encounter
go test -race -run TestEncounterSuite ./...
```

```bash
git add encounter/encounter.go encounter/encounter_test.go
git commit -m "feat(encounter): add Move verb publishing MoveEvent + HexRevealedEvent

Move validates, computes the mover's reveal delta BEFORE applying it
(critical ordering: delta = visible-from-new-position MINUS already-
revealed), mutates state, computes per-player projections via
perception.ProjectMove for other viewers, and publishes the cause event
plus a HexRevealedEvent when any viewer's vision changed.

Refs #<issue>"
```

---

## Task 8 — `Encounter.OpenDoor` verb

**Goal:** Second action verb.

**Files:**
- Modify: `encounter/encounter.go` (add `OpenDoor`)
- Modify: `encounter/encounter_test.go` (add OpenDoor tests)

- [ ] **Step 1: Add OpenDoor to `encounter.go`**

```go
// OpenDoor applies an open-door action. Marks the door open and publishes
// the cause event (DoorOpenedEvent) plus a HexRevealedEvent for any viewer
// whose vision grew.
func (e *Encounter) OpenDoor(playerID types.PlayerID, doorID types.EntityID) error {
    p, ok := e.data.Players[playerID]
    if !ok {
        return fmt.Errorf("player %q not in encounter", playerID)
    }
    door, ok := e.data.Doors[doorID]
    if !ok {
        return fmt.Errorf("door %q not in encounter", doorID)
    }
    if door.Open {
        return fmt.Errorf("door %q already open", doorID)
    }

    door.Open = true

    doorPerPlayer := make(map[types.PlayerID]events.DoorOpenedPlayerSlice)
    revealPerPlayer := make(map[types.PlayerID]events.HexRevealedSlice)

    for viewerID, viewer := range e.data.Players {
        doorSlice, revealSlice := perception.ProjectDoorOpen(
            doorID, door.Position, p.EntityID, viewer.View,
        )
        if doorSlice != nil {
            doorPerPlayer[viewerID] = *doorSlice
        }
        if revealSlice != nil {
            if revealSlice.Hexes != nil {
                viewer.View.ApplyReveal(revealSlice.Hexes)
            }
            revealPerPlayer[viewerID] = *revealSlice
        }
    }

    if err := e.broker.Publish(events.NewDoorOpenedEvent(
        e.data.ID, e.nextSeq(), doorID, p.EntityID, doorPerPlayer,
    )); err != nil {
        return fmt.Errorf("publish door: %w", err)
    }
    if len(revealPerPlayer) > 0 {
        if err := e.broker.Publish(events.NewHexRevealedEvent(
            e.data.ID, e.nextSeq(), revealPerPlayer,
        )); err != nil {
            return fmt.Errorf("publish reveal: %w", err)
        }
    }
    return nil
}
```

- [ ] **Step 2: Add OpenDoor tests**

```go
func (s *EncounterSuite) TestOpenDoor_PublishesEvents() {
    e := encounter.New("enc-1", s.broker)
    s.Require().NoError(e.AddPlayer(encounter.PlayerInput{
        PlayerID: "alice", EntityID: "char-alice",
        Position: types.Hex{}, SightRange: 4,
    }))
    s.Require().NoError(e.AddPlayer(encounter.PlayerInput{
        PlayerID: "bob", EntityID: "char-bob",
        Position: types.Hex{Q: 50, R: -25, S: -25}, SightRange: 4,
    }))
    e.AddDoor("door-1", types.Hex{Q: 2, R: 0, S: -2}, false)

    aliceSub, _ := s.broker.Subscribe("enc-1", "alice")
    bobSub, _ := s.broker.Subscribe("enc-1", "bob")

    s.Require().NoError(e.OpenDoor("alice", "door-1"))

    seenAlice := collectTypes(aliceSub, 500*time.Millisecond)
    s.Contains(seenAlice, "*events.DoorOpenedEvent")

    seenBob := collectTypes(bobSub, 100*time.Millisecond)
    s.Empty(seenBob, "bob out of range should receive nothing")
}

func (s *EncounterSuite) TestOpenDoor_Validations() {
    e := encounter.New("enc-1", s.broker)
    s.Require().NoError(e.AddPlayer(encounter.PlayerInput{
        PlayerID: "alice", EntityID: "char-1", SightRange: 3,
    }))
    e.AddDoor("door-1", types.Hex{}, false)

    s.Error(e.OpenDoor("nobody", "door-1"))
    s.Error(e.OpenDoor("alice", "nonexistent"))
    s.Require().NoError(e.OpenDoor("alice", "door-1"))
    s.Error(e.OpenDoor("alice", "door-1"), "second open should error")
}
```

- [ ] **Step 3: Run tests, commit**

```bash
cd /home/kirk/personal/rpg-toolkit/encounter
go test -race ./...
```

```bash
git add encounter/encounter.go encounter/encounter_test.go
git commit -m "feat(encounter): add OpenDoor verb publishing DoorOpenedEvent + HexRevealedEvent

OpenDoor mutates door state, projects per-player slices via
perception.ProjectDoorOpen, and publishes DoorOpenedEvent + a
HexRevealedEvent for viewers whose vision grew.

Refs #<issue>"
```

---

## Task 9 — End-to-end integration test

**Goal:** Prove the entire walking skeleton works in one scenario. Two players, one moves and one watches; door opens; persistence round-trip; sequence monotonicity.

**Files:**
- Create: `encounter/integration_test.go`

- [ ] **Step 1: Write the integration test**

```go
package encounter_test

import (
    "encoding/json"
    "testing"
    "time"

    "github.com/KirkDiggler/rpg-toolkit/encounter"
    "github.com/KirkDiggler/rpg-toolkit/encounter/types"
    "github.com/stretchr/testify/suite"
)

type IntegrationSuite struct {
    suite.Suite
    transport *encounter.InMemoryTransport
    broker    *encounter.Broker
    enc       *encounter.Encounter
    aliceSub  *encounter.Subscription
    bobSub    *encounter.Subscription
}

func TestIntegrationSuite(t *testing.T) {
    suite.Run(t, new(IntegrationSuite))
}

func (s *IntegrationSuite) SetupTest() {
    s.transport = encounter.NewInMemoryTransport()
    s.broker = encounter.NewBroker(s.transport)
    s.enc = encounter.New("enc-walking-skel", s.broker)

    s.Require().NoError(s.enc.AddPlayer(encounter.PlayerInput{
        PlayerID: "alice", EntityID: "char-alice",
        Position: types.Hex{}, SightRange: 4,
    }))
    s.Require().NoError(s.enc.AddPlayer(encounter.PlayerInput{
        PlayerID: "bob", EntityID: "char-bob",
        Position: types.Hex{Q: 2, R: 0, S: -2}, SightRange: 4,
    }))
    s.enc.AddDoor("door-east", types.Hex{Q: 4, R: 0, S: -4}, false)

    var err error
    s.aliceSub, err = s.broker.Subscribe("enc-walking-skel", "alice")
    s.Require().NoError(err)
    s.bobSub, err = s.broker.Subscribe("enc-walking-skel", "bob")
    s.Require().NoError(err)
}

func (s *IntegrationSuite) TearDownTest() {
    _ = s.aliceSub.Close()
    _ = s.bobSub.Close()
    _ = s.broker.Close()
    _ = s.transport.Close()
}

// Alice moves toward Bob. Bob is within sight (distance 2 between her
// destination and his position). Both should receive MoveEvent.
func (s *IntegrationSuite) TestSlice_MoveTowardEachOther() {
    path := []types.Hex{{Q: 1, R: 0, S: -1}}
    s.Require().NoError(s.enc.Move("alice", path))

    aliceEvents := collectTypes(s.aliceSub, 500*time.Millisecond)
    bobEvents := collectTypes(s.bobSub, 500*time.Millisecond)

    s.Contains(aliceEvents, "*events.MoveEvent")
    s.Contains(bobEvents, "*events.MoveEvent",
        "bob within distance 2 should see alice move")
}

// Door opens at distance 4 from alice (sight range 4) and distance 2 from
// bob (sight range 4). Both should see the door event.
func (s *IntegrationSuite) TestSlice_OpenDoor() {
    s.Require().NoError(s.enc.OpenDoor("alice", "door-east"))

    aliceEvents := collectTypes(s.aliceSub, 500*time.Millisecond)
    bobEvents := collectTypes(s.bobSub, 500*time.Millisecond)

    s.Contains(aliceEvents, "*events.DoorOpenedEvent")
    s.Contains(bobEvents, "*events.DoorOpenedEvent")
}

// Persistence round-trip: serialize, "restart," replay another action,
// observe events still flow and prior reveal persisted.
func (s *IntegrationSuite) TestSlice_RoundTripPersistence() {
    s.Require().NoError(s.enc.Move("alice", []types.Hex{{Q: 1, R: 0, S: -1}}))
    drainSub(s.aliceSub, 200*time.Millisecond)
    drainSub(s.bobSub, 200*time.Millisecond)

    payload, err := json.Marshal(s.enc.ToData())
    s.Require().NoError(err)

    // Simulate restart.
    _ = s.broker.Close()
    _ = s.transport.Close()
    s.transport = encounter.NewInMemoryTransport()
    s.broker = encounter.NewBroker(s.transport)

    var loaded encounter.EncounterData
    s.Require().NoError(json.Unmarshal(payload, &loaded))
    enc2, err := encounter.LoadFromData(&loaded, s.broker)
    s.Require().NoError(err)

    aliceSub2, err := s.broker.Subscribe("enc-walking-skel", "alice")
    s.Require().NoError(err)
    defer aliceSub2.Close()

    s.Require().NoError(enc2.Move("alice", []types.Hex{{Q: 2, R: 0, S: -2}}))

    aliceEvents := collectTypes(aliceSub2, 500*time.Millisecond)
    s.Contains(aliceEvents, "*events.MoveEvent",
        "after reload, encounter publishes events through the new broker")

    snap := enc2.SnapshotFor("alice")
    s.True(snap.RevealedHexes.Has(types.Hex{Q: 1, R: 0, S: -1}),
        "reveal from round-1 move should persist through round-trip")
}

// Sequence numbers monotonic across both events of one action.
func (s *IntegrationSuite) TestSlice_SequenceMonotonic() {
    s.Require().NoError(s.enc.Move("alice", []types.Hex{{Q: 1, R: 0, S: -1}}))

    var seqs []uint64
    timeout := time.After(500 * time.Millisecond)
loop:
    for {
        select {
        case evt, ok := <-s.aliceSub.Events():
            if !ok {
                break loop
            }
            seqs = append(seqs, evt.Sequence())
            if len(seqs) >= 2 {
                break loop
            }
        case <-timeout:
            break loop
        }
    }
    s.Require().Len(seqs, 2, "expected MoveEvent + HexRevealedEvent")
    s.True(seqs[1] > seqs[0], "sequence advances between events")
}

func drainSub(sub *encounter.Subscription, timeout time.Duration) {
    deadline := time.After(timeout)
    for {
        select {
        case _, ok := <-sub.Events():
            if !ok {
                return
            }
        case <-deadline:
            return
        }
    }
}
```

- [ ] **Step 2: Run all tests**

```bash
cd /home/kirk/personal/rpg-toolkit/encounter
go test -race ./...
```

Expected: PASS, including the four integration scenarios.

- [ ] **Step 3: Commit**

```bash
git add encounter/integration_test.go
git commit -m "test(encounter): end-to-end walking-skeleton integration test

Two players, one moves and one watches; door opens; persistence round-trip;
sequence monotonicity. Verifies broker fanout, audience filtering, and
ToData/LoadFromData preserves perception view across reload.

Refs #<issue>"
```

---

## Task 10 — Toolkit harness docs

**Goal:** Per the toolkit harness rule, every component gets an architecture doc and status update.

**Files:**
- Create: `docs/architecture/components/encounter.md`
- Modify: `docs/status.md` (add Encounter SDK as new active work)
- Modify: `docs/quality.md` (add Encounter and Perception modules with grades)

- [ ] **Step 1: Write `docs/architecture/components/encounter.md`**

> **Substitute today's date for the `updated:` field.** The plan uses `<TODAY>` as a placeholder — the implementer should replace with the actual ISO date (e.g. `2026-05-06`).

```markdown
---
component: encounter
status: walking skeleton (Slice 1)
updated: <TODAY>
---

# Encounter SDK

The encounter SDK is the orchestrator-facing facade for running an encounter
end-to-end. Game servers `Load` an encounter from persisted state, mutate via
verb methods (`Move`, `OpenDoor`, ...), serialize back via `ToData`, and save.
Player-facing events flow through a process-scoped `Broker` that publishes
per-player projected events through a pluggable `Transport`.

## Module layout

One Go module: `github.com/KirkDiggler/rpg-toolkit/encounter`. Internal subpackages:

- `encounter/types` — primitive value types (`EncounterID`, `PlayerID`, `EntityID`, `Hex`, `HexSet`, `AudienceSet`).
- `encounter/events` — sealed `EncounterEvent` interface + concrete events (Slice 1: `MoveEvent`, `HexRevealedEvent`, `DoorOpenedEvent`).
- `encounter/perception` — `PerceptionView` value type + projection functions (`ProjectMove`, `ProjectDoorOpen`).
- `encounter` (top-level) — `Encounter` aggregate, `Broker`, `Transport`, `InMemoryTransport`.

The internal package graph is a linear DAG: `types ← events ← perception ← encounter`.

## Key types

- `Encounter` — transient. Constructed per-call from `EncounterData`.
- `EncounterData` — persisted shape. Carries Players (with `PerceptionView`), Doors, and a monotonic Sequence counter.
- `Broker` — process-scoped, holds in-process subscription registry, routes via Transport. One per game-server process.
- `EncounterEvent` — sealed sum interface. Concrete events implement `isEncounterEvent()`, `EncounterID()`, `Sequence()`, `Audience()`.

## Design references

- Spec: `rpg-project/ideas/encounter/v1alpha2/sdk-direction.md`
- Slice plans: `rpg-project/ideas/encounter/v1alpha2/plans/`

## Out of scope (Slice 1)

Combat verbs, action economy, conditions, senses, real LoS, Redis transport, gRPC handler, monsters, all event types beyond the three above. Entity-visibility accumulation is reserved in the type shapes (`HexRevealedSlice.Entities`, `PerceptionView.KnownEntities`) but not emitted.
```

- [ ] **Step 2: Update `docs/status.md`**

Add an entry under "Active work" for the Encounter SDK walking skeleton, and a corresponding "Recently landed" entry pointing at the merged PR (after merge).

- [ ] **Step 3: Update `docs/quality.md`**

Add rows for the new `encounter/` module (with subpackages) with conservative confidence grades (likely B for first cut — has tests, slim coverage; will rise as combat verbs and real LoS land).

- [ ] **Step 4: Update root `Makefile`** (optional, separate commit)

The toolkit `Makefile`'s `pre-commit` and `test` targets only cover `core` and `events`. They should be extended to cover the new `encounter` module. **This change is out of scope for the slice itself** — flag it as a follow-up issue rather than expanding this PR.

```bash
gh issue create \
  --repo KirkDiggler/rpg-toolkit \
  --title "Update Makefile pre-commit/test targets to include new modules" \
  --body "Currently \`make pre-commit\` and \`make test\` hard-code core+events. \`make test-all\` covers everything via \`find\` discovery. The hard-coded targets should be either replaced or extended each time a new module lands. Tracking as a follow-up to the encounter SDK slice."
```

- [ ] **Step 5: Commit**

```bash
git add docs/
git commit -m "docs(encounter): add component doc + update status/quality

Component doc per harness convention. Status reflects active Encounter SDK
walking-skeleton work; quality.md grades the new modules conservatively
pending more verbs and real LoS.

Refs #<issue>"
```

---

## Push and PR

- [ ] **Step 1: Run the full test suite**

```bash
cd /home/kirk/personal/rpg-toolkit
make test-all
```

Expected: all modules PASS, including the new `encounter` module's subpackages.

- [ ] **Step 2: Push branch**

```bash
git push -u origin feat/<issue>-encounter-walking-skeleton
```

- [ ] **Step 3: Open PR**

```bash
gh pr create --title "Encounter SDK walking skeleton (closes #<issue>)" --body "$(cat <<'EOF'
## Summary
- Sealed `EncounterEvent` taxonomy (AWS v2 SDK marker pattern). Three concretes: MoveEvent, HexRevealedEvent, DoorOpenedEvent.
- Process-scoped `Broker` over a pluggable `Transport`. `InMemoryTransport` ships now; Redis/Kafka future.
- Transient `Encounter` aggregate with `Move`/`OpenDoor` verbs, `ToData`/`LoadFromData` persistence, `SnapshotFor` for stream snapshots.
- Subpackage `encounter/perception` — pure functions, stub LoS (Manhattan-radius). Real LoS is a separate slice.
- Subpackage `encounter/types` exists specifically to break the would-be `encounter` ↔ `encounter/events` package import cycle.
- End-to-end integration test proves the seam.

## Spec / Plan
- Spec: rpg-project/ideas/encounter/v1alpha2/sdk-direction.md
- Plan: rpg-project/ideas/encounter/v1alpha2/plans/02-walking-skeleton.md

## Out of scope (deliberately)
Combat verbs, action economy, conditions, senses, real LoS, Redis transport, gRPC handler, monsters, all events beyond the three above.

## Test plan
- [x] `make test-all` clean
- [x] Integration test verifies move + door + persistence round-trip + audience filtering + sequence monotonicity
- [x] HexSet JSON round-trip test guards against struct-map-key serialization regression
- [x] Subscription double-close safety test (sync.Once)
- [ ] Copilot review (will fix or thread why-not on every comment)

🤖 Generated with [Claude Code](https://claude.com/claude-code)
EOF
)"
```

- [ ] **Step 4: Wait for Copilot review**

Per `feedback_copilot_review_timing`: turnaround is 5–10 min; sleep 6–10 min before first poll, not seconds.

- [ ] **Step 5: Address Copilot feedback per `feedback_copilot_review`**

Fix or threaded "why-not" on every comment before asking Kirk to merge.

---

## Done criteria

- All tasks above complete; commits clean, no `--no-verify`.
- `make test-all` passes locally and in CI.
- Integration test in `encounter/integration_test.go` demonstrates the four scenarios.
- Copilot review addressed; PR ready for Kirk to merge.
- Component doc landed; status.md and quality.md reflect new modules.
- Follow-up issue filed for Makefile per-module test target update.

After merge, the next slice (real LoS, then combat verbs) builds on this foundation.

---

## Known trade-offs documented but not closed

- **Sequence stamping is split across cause and effect events.** `Move` calls `nextSeq()` for `MoveEvent`, then again for `HexRevealedEvent`. If the first publish errors and returns early, the second event never gets a sequence number. Slice scope: best-effort delivery; reconnect-via-snapshot is the safety net (sdk-direction.md "did not settle" → failure semantics). Worth revisiting when failure semantics are settled.
- **Listener mutex is held across `Audience()` iteration but released before channel sends.** A subscriber that has already subscribed but whose `Audience()` call materially mutates state (it doesn't, by contract) could surprise the broker. Not an actionable concern in slice 1.
- **Subscriber buffer size 64.** Slice tests are sized to avoid drop. Real-world workloads may need larger buffers or backpressure semantics — future slice when failure semantics are settled.
