# The monster's turn — the driver decides, the composition executes

**Status:** DRAFT for Kirk's ruling, 2026-08-23.
**Journey:** rpg-project#91 → umbrella to be filed as a sub-issue of #91 (slice 2 of the combat turn; slice 1 = #251).
**Owner-to-be:** the Monster AI initiative (#201, assigned to dammitbilly0ne). Kirk: *"hook in the simple monster turn that they could build on. we want to simplify their starting point so getting the basic actions would be good."*
**Reused, not re-derived:** ideas/monster-ai (PR #202): perception → decision → actuation as three hard seams, the decision layer "never mutates encounter state, never touches the event bus, never resolves rules"; behavior as placement data (`targeting:`). ADR-0043 (a monster's turn has a driver; `TurnDriver` is a capability the host supplies). Slice 1's typed beats (`moved`, `struck`, `missed`, `turn_ended`).

## 1. The turn, as the player lives it

Same panel as slice 1. On End Turn: *"Skeleton's turn."* → the skeleton **walks toward you** (you see it move) → if it reaches you, *"Skeleton attacks you — 14 vs AC 16, miss."* (or a hit, with damage) → *"Round 2, your turn."* A skeleton that sees nobody stands still. Nothing else changes on the panel; the beats already exist.

## 2. What exists / what is missing (session v0.21.8)

| Need | Today | Work |
|---|---|---|
| Driver input | `Act(member string)` — an id, nothing else | a **MonsterView** built by the composition (the brainstorm's "perception is the only input") |
| Driver output | `Pass` only (sealed) | an **Intent**: `Pass` \| `Attack{Target, Action}` \| `Move{Path}` |
| Executing an attack for a monster | `session.Attack` refuses `KindMonster`; `resolution.AttackFromMonsterAction` compiles bite/melee | actuation inside the drive loop via the same strike machine players use (reach from the action's `Reach`) |
| Executing a move for a monster | `session.Move` prices off a character sheet (`ErrNoCharacter`); `encounter.Step` is kind-agnostic | actuation inside the drive loop via `Step`, bounded by `Speed.Walk` |
| Monster economy | none (ledger lives on character sheets) | v1: the loop enforces *one attack + ≤ speed feet per turn*; no ledger |
| Perception for the driver | `encounter.View` is kind-agnostic; `Distance` exists; sight is a flat 4 cells for everyone | MonsterView = sightings with position, standing, distance, in-reach-per-action; sight range stays flat (gap noted) |
| Wire | `moved`, `struck`, `missed`, `turn_ended` typed | **no proto change** (web: refetch view on others' `moved`; pace the monster beats) |
| Behavior data | `monster.Data.Targeting` (closest / lowest-health / lowest-ac), skeleton has none set | the basic driver honors `Targeting`; default closest |

## 3. The seam (this is what the friend inherits)

```go
// rulebooks/dnd5e/encounter (the clock's owner keeps the loop; ADR-0043 unchanged)
type TurnDriver interface {
    // Act is called repeatedly during one driven turn, with a fresh view each time,
    // until it returns Pass or the turn's budget is spent.
    Act(view MonsterView) (Intent, error)
}
type MonsterView struct {
    Self      MemberID; Position spatial.Position; SpeedFeet int; MovementLeftFeet int; ActionLeft bool
    Actions   []ActionView          // from monster.Data.Actions: Ref, Name, ReachFeet, Kind(melee|ranged)
    Seen      []SeenMember          // what THIS monster perceives: Member, Kind, Standing, Position, DistanceCells, InReach map[action]bool
    Targeting monster.TargetingStrategy
    Round     int
}
type Intent interface{ isIntent() }        // sealed: Pass, Attack{Target MemberID; Action Ref}, Move{Path []spatial.Position}
```
- **The composition executes.** `driveMonsterTurns` (encounter/clocks.go) becomes: build view → `Act` → execute → repeat. `Move` → `Step` per cell (refuses beyond MovementLeft, out of order, through a wall — the same rules a player's walk meets). `Attack` → a **Striker capability supplied by session** (never defaulted): `Strike(ctx, enc, attacker, target, action) (StrikeReport, error)` bound to the *same loaded encounter* (no second load, no re-entrancy), compiled via `AttackFromMonsterAction`, reach gated by the action's reach, recorded as `struck`/`missed` with `Attack{Ref,Name,DamageType}`. `Pass` → end the turn as today.
- **A driver cannot break a rule.** Every intent passes through the same gates as a player's verb; a bad intent is refused back to the driver (`ErrBadIntent` with the reason) and the loop ends the turn — never aborts the player's verb.
- **The basic driver** (`rulebooks/dnd5e/behavior/basic.go`, the friend's starting file): pick target by `Targeting` among `Seen` that are players and UP; if in reach and action left → `Attack`; else if movement left → `Move` one step along the shortest path toward the target (the composition's path, not the driver's); else `Pass`. ~40 lines. Everything the friend adds (modes, utility scoring, memory, placement profiles) is a better `Act`; nothing below it changes.

## 4. What each repo builds (proto first = nothing to merge; toolkit → api → web in parallel)
- **toolkit**: `MonsterView`/`Intent`/`Striker` in encounter; the loop; `behavior.Basic`; session supplies `Striker` from its strike machinery and adapts the friend's driver; skeleton data gets `Targeting: closest` by default. Also #1167 (export the Pass driver) closes naturally. Tests: tomb — skeleton 4 cells away, fighter ends turn → skeleton moves to adjacent, attacks, turn ends; skeleton with nobody seen → passes; a driver returning an out-of-reach Attack → refused, turn ends, player's EndTurn still succeeds.
- **rpg-api**: supply `session.Config.TurnDriver = behavior.Basic{}` (one line; the friend later supplies theirs through the same field).
- **web**: refetch GetView on `moved` for members other than you; monster-beat pacing for `moved`/`struck`/`missed` (web#561); no new shapes.

## 5. Open for Kirk
- **Loop location**: keep the loop in `encounter` with a session-supplied `Striker` capability (recommended — ADR-0043's placement stands, rules stay in resolution/session), vs. moving the drive loop into `session`. The first is one capability; the second re-homes three choke points (EndTurn, form, transfer/exit).
- **Budget as a loop rule vs a monster ledger**: loop rule for v1 (recommended); a ledger when monsters get bonus actions/reactions.
- **Monster sight**: stays a flat 4 cells this slice (senses-aware sight is its own item).
