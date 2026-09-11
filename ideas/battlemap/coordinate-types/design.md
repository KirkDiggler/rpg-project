---
name: Coordinate Types — Local vs Absolute Cube Positions
description: Type-safe distinction between room-local and dungeon-absolute coordinates, replacing ad-hoc transforms with a canonical bridge
updated: 2026-05-02
status: design complete — ready for plan.md
confidence: high — all open questions decided
---

# Coordinate Types: Local vs Absolute

## Problem

Round 2 of the multi-room dungeon work produced **5 separate "forgot to translate coordinates" fixes** (PRs #459, #461, #463, #466, #467) and a 6th in `mergeNewRoomMonsters` (PR #468). Each fix was correct in isolation, but the pattern reveals the actual cause:

**The compiler can't catch us.** Room-local positions and dungeon-absolute positions use the same `Position` type. Anyone writing new code that crosses the boundary has to remember to translate. Round 2 proved discipline alone isn't enough — site #6 was always going to be missed, and site #7 will be too.

Per `rpg-api-member/context/discoveries.json` (disc-009), there are actually **three** `Position` types in play with different signatures:
- `entities.Position` — `float64` X, Y, Z
- `dungeon.Position` — `int` X, Y, Z
- `apiv1alpha1.Position` (proto) — `float64` X, Y, Z

The int/float64 mismatch forces casts at every boundary, hiding the local/absolute confusion behind a more obvious type-conversion concern.

**Decided:** Same PR as the type-safety refactor. The two are inseparable — discipline-typed coords whose underlying signature still mismatches at the proto boundary would just move the bugs. (See migration plan for the "add alongside, remove old before live test" constraint.)

## Goals

- **Type-level distinction** between local (room-relative) and absolute (dungeon-relative) coordinates. Compiler refuses to substitute one for the other.
- **Single canonical bridge.** One function (`Module.LocalToAbsolute`) is the only legal way to cross spaces. No ad-hoc element-wise math at call sites.
- **Cube-only.** Everything is hex cube coordinates with the invariant `X + Y + Z = 0`. No 2D vestiges.
- **Stay in rpg-api for now.** The dungeon component lives in `rpg-api/internal/components/dungeon/` until we've learned more. Future graduation to rpg-toolkit becomes an import-path change.

## Non-goals

- **Flat/2D coordinates.** Toolkit may eventually need square-grid coords (per rpg-toolkit team-member's pathfinder gap finding); that's a separate concern.
- **Persistence design.** The three in-memory repos are tracked separately (issue TBD).
- **Proto-types-out-of-orchestrator.** Related but distinct work (issue TBD); design here doesn't depend on it.
- **Dungeon component graduation.** Tracked separately (issue TBD); this design just makes the future move cleaner.

## Design

### Types

```go
package dungeon

// LocalPosition is room-relative cube. Invariant: X+Y+Z=0.
// Has no meaning without an associated room.
type LocalPosition struct{ X, Y, Z int }

// AbsolutePosition is dungeon-relative cube. Invariant: X+Y+Z=0.
type AbsolutePosition struct{ X, Y, Z int }

// Constructors enforce the cube invariant by computing one axis from the other two.
func NewLocalPosition(x, z int) LocalPosition       // y = -x - z
func NewAbsolutePosition(x, z int) AbsolutePosition // y = -x - z
```

**Decided:** Add new types alongside the existing `Position` types and migrate call sites incrementally. The old types are not removed until every call site is migrated. Live testing happens **only after** the old types are fully removed — anything else risks half-migrated state where some sites use new and some use old, and bugs hide in the seam.

### Translation

`Module` follows the toolkit pattern: it carries dungeon state internally, but it's loaded from data and serializes back to data. The data orchestrator (rpg-api) owns persistence.

```go
type Module struct {
    // holds room origins, room data, etc., keyed by roomID
}

// Load and save data — toolkit pattern.
type Data struct { /* serializable shape */ }

func LoadFromData(d *Data) (*Module, error)
func (m *Module) ToData() *Data

// Translation — the only legal bridge between local and absolute.
func (m *Module) LocalToAbsolute(roomID string, p LocalPosition) (AbsolutePosition, error)
func (m *Module) AbsoluteToLocal(roomID string, p AbsolutePosition) (LocalPosition, error)
```

Translation is element-wise cube addition:

```
abs.X = local.X + origin.X
abs.Y = local.Y + origin.Y
abs.Z = local.Z + origin.Z
// invariant survives: (lx+ly+lz) + (ox+oy+oz) = 0 + 0 = 0
```

The current `cubeY := -cubeX - cubeZ` line in `mergeNewRoomMonsters` disappears — that's a 2D-vestige tell.

**Decided:** Module carries state. Per Kirk: "the module like all of toolkit should have a `LoadFromData` and `ToData` functionality. The data orchestrator is responsible for persisting it." This pattern also incidentally resolves `disc-006` — `RoomData` currently typed as `interface{}` in `EncounterData` becomes a concrete `dungeon.Data` struct with the toolkit's standard serialization.

### Origin storage

Unchanged from today. `Dungeon.RoomOrigins map[string]Position` populated by BFS during generation, where `Position` becomes `AbsolutePosition`.

## Migration plan

Single PR. Three internal phases, each its own commit:

1. **Define types and `Module` in `dungeon` package.** `LocalPosition` and `AbsolutePosition` types with cube-invariant constructors. `Module` with `LoadFromData` / `ToData` / `LocalToAbsolute` / `AbsoluteToLocal`. Documented as the only public bridge. No call sites converted yet — these types coexist with the existing `dungeon.Position`, `entities.Position`, and proto `Position` types during migration.
2. **Audit and migrate room-local constructions.** Every place that builds a position inside a room (generator, perimeter walls, monster placement) produces honest cube — set `.Z`, derive `.Y`, never store partial. The compiler catches anything that doesn't fit under the new types.
3. **Migrate transform sites.** Every place that crosses local→absolute calls `Module.LocalToAbsolute`. The 5 Round 2 fix sites become natural call sites. Hand-rolled element-wise math is removed. **Then remove `dungeon.Position` and `entities.Position`** — at this point all call sites are migrated.

**Constraint (per Kirk):** the old types are not removed until every call site has been migrated, AND **no live testing happens until the old types are gone**. Half-migrated state where some code uses new and some uses old hides bugs in the seam. The PR is "ready to test" only after `git grep entities.Position` and `git grep dungeon.Position` return no usages.

Round 2's PR #468 is explicitly superseded — its 5 fixes become "the places we already noticed something was wrong." The audit-and-migrate sweep catches the silent ones too.

## Audit candidates (from rpg-api-member)

Verified-by-grep before implementation:
- `internal/components/dungeon/generator.go` — monster + obstacle placement
- `internal/components/dungeon/toolkit/perimeter.go` — wall segment construction
- `internal/orchestrators/encounter/orchestrator.go` — `buildRoomLayoutProto`, `buildRoomsMap`, `addMonstersToEntityMap`, `mergeNewRoomMonsters`, `getCurrentRoomOrigin`
- `internal/handlers/dnd5e/v1alpha1/encounter/handler.go` — `shiftRoomToAbsolute` (~9 call sites)
- `internal/handlers/dnd5e/v1alpha1/encounter/converters.go` — `dungeonPositionToProto` and friends

## Out of scope (referenced for completeness)

- Square-grid coordinate types (future, in toolkit)
- Dungeon component graduation rpg-api → rpg-toolkit (future, this design makes it an import-path change)
- Path-finding integration (toolkit's `SimplePathFinder` is hex-only — separate gap)

## Decisions

1. ✅ **Int/float64 unification: same PR.** Inseparable from the type-safety refactor.
2. ✅ **Add new types alongside, migrate, then remove old.** No live testing until the old types are gone.
3. ✅ **Module carries state.** Toolkit pattern: `LoadFromData` / `ToData`; data orchestrator owns persistence.
4. ✅ **`entities.Position` is removed.** The game traffics in absolute coords only; `dungeon.AbsolutePosition` replaces it. Local positions are an internal dungeon-component concept.

5. ✅ **`apiv1alpha1.Position` becomes `int32` X/Y/Z and represents `AbsolutePosition` directly.** No float64 anywhere. Hex cube coords are inherently integer (tile-based gameplay), and the float was accidental. Movement is path-based — a sequence of tile transitions; "we know where we're going" means a stop signal cancels the path, but never produces a fractional position. The handler converter is three field copies; structurally `pb.Position == dungeon.AbsolutePosition`, kept conceptually distinct only for the architectural boundary (proto types don't escape the handler).

   This is a breaking proto change. Acceptable because (a) the game isn't playable end-to-end, so blast radius is small, and (b) per project preference, breaking changes during migration beat dual-path coexistence (see `feedback_prefer_breaking_changes`). Sequence `rpg-api-protos #139` (`buf breaking` becomes blocking) before this change so any other proto breaks are flagged at the same time.

## Open questions

(none)

## Citations

- Round 2 fix PRs: #459, #461, #463, #466, #467, #468 (all superseded by this design)
- rpg-api issue #471 (this design's tracked work)
- rpg-api existing #399 (Unified Dungeon Coordinate System) and #402 (Simplify dungeon orchestrator) — adjacent concerns; per feature-manager triage, #471's done state makes #402 trivial; #399 should be cross-linked once this design is finalized
- rpg-api-member discoveries: `disc-009` (three Position types with int/float64 mismatch), `disc-006` (RoomData as `interface{}` resolved by Module's `LoadFromData`/`ToData`)
- rpg-toolkit team-member finding: square-grid pathfinder gap (issue #614)
- Workspace patterns: `feedback_ideas_structure.md`, `feedback_ideas_folder.md`
