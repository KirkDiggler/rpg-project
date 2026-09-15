---
status: DESIGN, proposed 2026-09-11
journey: rpg-project#430 · step 1: Thunderwave (Kirk 2026-09-11, "shapes into spatial will have a lot of use cases and we get forced movement to boot") · builds on: ../terrain/design.md (the field and the fold, rung 1 landed 2026-09-11) · fills the gap recorded in ../../spells/heal-and-half/design.md §"The forced move is deliberately deferred" (rpg-project#414, open)
law: resolution DESCRIBES a move, encounter COMPUTES and EXECUTES it; the fold gets a fifth reader, never a second author
---

# Directed movement — an effect moves a creature

## 0. Purpose

One tool: a **directive**. An effect says *this creature moves, this way, this
far, and this is what it pays*. The engine finds the route on the field, reads
the fold for every cell, and walks it, the way it walks a monster. No spell
holds a path search of its own, and no rule about the world enters resolution.

Customers, in the order they arrive:

| customer | policy | budget | pays | provokes | when |
|---|---|---|---|---|---|
| **Thunderwave** (first proof) | straight line away from the caster | 2 cells | nothing | no | during the cast, on a failed save |
| **Dissonant Whispers** (second) | away from the caster | the target's speed | its reaction, if available | yes | during the cast, on a failed save |
| **monster flee** (behavior lane) | away from threats | its own movement | its own turn | yes | its own turn |
| Thorn Whip | toward the caster | 2 cells | nothing | no | during the cast, on a hit |
| Command: Approach, Flee | toward / away | the target's speed | its own turn | yes | the target's next turn |

Kirk, 2026-09-11, choosing the first proof: *"we are not bound to 2014 and the
PHB. see thunder clap. we are here to prove the engine works … shapes into
spatial will have a lot of use cases and we get forced movement to boot."*
Thunderwave goes first because it pays for two tools, coverage and the
directive, where Whispers pays for one. Whispers goes second and exercises the
directive's harder fields, the reaction and the provoke. The last two rows are
named so the shape is tested against them, not built for them.

## 1. What is true today

Verified at `rpg-toolkit` `origin/main` `dd8620ec` (after terrain rung 1).

**Nothing moves a creature against its will.** Grepping push, pull, shove,
displace, knock, forced across `rulebooks/dnd5e` finds prose, a shovel, and a
wolf's knock-prone save gate. No verb.

**The consequence vocabulary is closed at three.** `ImposedEffectKind` is
`condition | damage | condition-removed` (`resolution/contest.go:73-87`). Its
one consumer is the switch in `session/castoutcome.go:131,150`, mapping to
`encounter.ResultConditionApplied` / `ResultDamageApplied`. A fourth kind
without an arm there falls to the default error, which is the right failure.

**A contest can impose more than one thing.** Producers at `contest.go:242`
(condition) and `:382` (damage) both append to the same list, so "damage AND
move on a failed save" is a third producer, not a new shape.

**The movement machine is per step and already resolves reactions inline.**
`MovementInput` (`resolution/movement.go:40`) is one step: mover, from, to, a
`ReactionAttacks` capability. `announce` (`:198`) publishes
`MovementChainEvent`, folds it, drops opportunity-attack triggers when the
folded event says `IsOAPrevented()`, and `react` (`:301`) resolves each as a
nested strike on the same bus. `outcome` (`:390`) reads from/to off the FOLDED
event, and its doc names a shove or slide as the case that makes that matter.
The machine was written expecting this design.

**Encounter's walk is the executor.** `walkCells` (`encounter/clocks.go:908`)
per cell: `e.mover.Move` (the movement machine, via session's mover seam at
`session/mover.go:139`), stop if the mover dropped, `stepTo` (`step.go:306`),
`appendMovementBeat` (`encounter.go:1291`). It stops on a pause, a refusal, or
a drop. **`Step` gates on the active turn** (`step.go:70`); a directed move
happens during somebody else's action, so the executor needs a door that the
active-turn gate does not guard.

**Reactions are spent by the reactor's condition, not by the machine.** The
opportunity-attack condition holds `UsedThisTurn`, sets it in
`onReactionTaken` (`conditions/opportunity_attack.go:395-411`) and publishes
`publishSpendRequested(ActionReaction)`; the character keeper debits
`ReactionsRemaining` (`character/action_economy.go:564`). `CanReact()` is on
the `Combatant` interface (`combat/combatant.go:129`). **A monster's keeper has
no subscription for the spend**, so a monster's reaction is never debited
today. A directive that pays a reaction must read `CanReact` and must spend
through the same request, and for monsters that is a gap named in §8.

**A moved creature looks like it walked.** The beat is
`{"beat":"moved","member":…,"position":…}` (`encounter.go:1294`); the
resolution event carries entity, from, to. Nothing says *why*. An observer
cannot tell a shove from a step, and intel is per-observer testimony, so the
cause has to travel with the beat.

**The field and the fold exist.** `spatial.Field` (`tools/spatial/v0.12.0`)
floods from sources under a passable predicate with a limit and reads a path
back. `Encounter.CellAt` (`encounter/v0.71.1`) folds sealed cells, props, and
members by stance into Blocked / PassThrough / Standable. `routeTo` and
`nearestStop` (`clocks.go`) already pick "nearest standable cell satisfying a
goal" with a stable tie-break. This design adds policies beside them.

**Dissonant Whispers is absent entirely.** No data row, no cast profile, no
condition. Thunderwave has a row (`spells/data.go:161`) and no profile, and
that row is a leftover, not a decision: Kirk, 2026-09-11, *"any spell traces
you find in there was from a different architecture than we have today and
should not be seen as a decision made … bane, true strike and the other spells
that can be slotted in a bard are the new way."* The new way is the
`castContent` profile map (`spells/cast.go:55`, six entries). A spell exists
when it has a profile there; a row or a constant without one is nothing to
build on and nothing to preserve. Half on a save does not exist: `contestMachine.resolve` returns `Done` on success
(`contest.go:668-676`) and `saves.Half` is read only by validation. Those are
heal-and-half's (#414, open) to land; §7 says what this slice borrows.

## 2. Ownership

| noun | owner | why |
|---|---|---|
| the directive: policy, anchor, budget, pays, provokes | **resolution** describes it, as an `ImposedEffect` | it is a rule about what a spell does |
| the route: which cells | **encounter** | geometry; read from the field through the fold |
| the execution: each step, each reaction, each beat | **encounter** | it already owns the walk and the step |
| the reaction spend | the reactor's keeper, via the existing spend request | economy is the sheet's |
| the beat's cause | encounter writes it; observers read it | intel is testimony |
| the outcome on the wire | session maps the kind to a result, one arm | the only consumer switch |

The fold gets its fifth reader (step, route, flee, atlas, directive). It gets
no second author: a directed step that crosses a cell the fold refuses stops
there, exactly as a chosen step would.

## 3. The directive

```go
// resolution
const ImposedMove ImposedEffectKind = "move"

// carried in ImposedEffect for Kind == ImposedMove
type MoveDirective struct {
    Policy   MovePolicy       // Away | Toward | Line
    AnchorID string           // the creature the policy is measured from (the caster)
    Budget   MoveBudget       // Cells int, or Speed bool (the mover's own speed)
    Pays     MovePays         // PaysNothing | PaysReaction
    Provokes bool             // whether the movement machine offers opportunity attacks
}
```

Zero values tell the truth: an empty `Policy` is refused at validation, a
`Budget` with neither field set is refused, `Pays` defaults to nothing and
`Provokes` to false, which is the push, the least permissive case.

**Policies, as geometry.** All three are reads of `spatial.Field` under the
fold, computed by encounter:

- **Away.** Flood from the mover, limit = budget, passable = fold ≠ Blocked.
  Among the reached Standable cells choose the one with the greatest grid
  `Distance` from the anchor; ties by shorter walk, then X then Y. Distance
  is the ruler's, not the walk's: "away from you" means far from you, and a
  cell one hex farther by the crow but ten by the corridor is still farther.
  Reached cells with distance ≤ the mover's current distance are never
  chosen; if none is greater, the mover does not move and the directive
  reports zero cells.
- **Toward.** Flood from the mover, limit = budget; among reached Standable
  cells choose the least `Distance` from the anchor, stopping adjacent if
  reachable; ties as above.
- **Line.** The cells along the grid's line from the anchor through the
  mover, continued past the mover for `budget` cells; take them in order and
  stop before the first the fold calls anything but Standable. A creature in
  the way stops a push; nothing passes through anyone. On a hex grid the line
  is the one the grid's own line-drawing gives (`GetLineOfSight`), which is
  the same line sight uses, so a push goes where a sightline would.

Encounter exposes one computation for all three, and it is the one the
behavior package will read for a monster's flee:

```go
// encounter
type RouteInput struct {
    Mover  MemberID
    Policy MovePolicy
    Anchor spatial.Position
    Budget int // cells
}
type RouteOutput struct {
    Path []spatial.Position // excludes the mover's cell, in walking order; empty when the policy yields nothing
}
func (e *Encounter) Route(in RouteInput) (RouteOutput, error)
```

`routeTo` and `routeToRemembered` stay as they are; `Route` sits beside them
and shares `floodFrom` and `nearestStop`.

## 4. The execution

```go
// encounter
type DirectInput struct {
    Mover    MemberID
    Cause    core.Ref            // the effect that moved them; travels on every beat
    Route    []spatial.Position  // from Route, trimmed by the caller to what was paid for
    Provokes bool
}
type DirectOutput struct {
    Moved   int
    StoppedBy string // "" when the whole route was walked; else the fold's refusal sentence
}
func (e *Encounter) Direct(ctx context.Context, in DirectInput) (DirectOutput, error)
```

`Direct` is `walkCells` with two differences and no third: it does not require
the mover to hold the active turn, and every beat it appends carries `Cause`.
Per cell it still calls the mover seam (so the movement machine announces the
step and, when `Provokes`, resolves opportunity attacks inline), still stops
when the mover drops mid-walk, still calls `stepTo` so the fold and the canvas
refuse exactly as they would a chosen step. A pushed creature that would land
on a pillar stops in front of it, and `StoppedBy` names the pillar.

When `Provokes` is false the announced event carries an OA-prevention source
naming the cause, so the machine's existing fold drops the triggers. No new
path through the machine; the machine already reads that flag.

**Paying.** Before `Route` is even asked, resolution checks the price. For
`PaysReaction`: `CanReact()` on the target; if false, the directive is
recorded as not taken (Dissonant Whispers says "if available") and no
movement happens. If true, the reaction is spent through the same spend
request the opportunity attack uses, attributed to the cause, before the walk
starts. A creature that starts fleeing has paid; being stopped by a wall two
cells in does not refund.

**The beat.** `appendMovementBeat` gains `cause` in its payload. A step a
creature chose has none. An observer's intel records what it saw: a creature
moving, and that something moved it. Whether the observer knows *which* spell
is a later per-observer question; the ref is on the log in full data, per the
pre-v1 rule.

## 5. The wire

Session's `imposedResult` gains one arm: `ImposedMove → encounter.ResultMoved{Path, StoppedBy}`.
The client already animates a peer's steps one cell at a time off the event
stream; a directed move arrives as the same steps. One additive proto field on
the cast target outcome carries the move result so the caster's own client can
show "it fled 6 cells and was struck once" without re-deriving it.

No new target kind. Dissonant Whispers targets one creature the caster can
see, which is Bane's single-target shape.

## 6. Thunderwave, the proof — and what it adds beyond this design

Cast profile in the bard's level-1 choice (which offers Bane alone today).
Save: Constitution. Area: a 15-foot cube on the caster's edge, which is a box
footprint anchored at the caster and oriented toward a chosen cell. The
client sends that cell as a reference; it is the point-origin target kind
ward-and-area §3 said Fireball would force, arriving one spell early and
serving both. Coverage (terrain design §3.2, pulled forward to this slice)
turns the box into cells at half. Every creature on those cells makes the
save; on a failure, two imposed effects from one contest: `2d8` thunder and
the directive `{Line, caster, 2 cells, PaysNothing, Provokes: false}`.

The walk: the bard stands in the hall with two skeletons in front and the
pillar behind one of them. Thunderwave. Both fail. The one with open floor
behind it slides two cells and stops; the one in front of the pillar slides
one and stops against it, and the story says the pillar stopped it. Nobody's
opportunity attack fires. That proves the shape, the coverage rule, the
directive's least permissive case, and the fold as the thing a push obeys.

Dissonant Whispers follows as the second proof and exercises what Thunderwave
cannot: `Away` with a speed budget, a reaction paid before the walk, and a
provoke resolved inline by the movement machine. Its walk is the skeleton
that flees the bard past the fighter and is struck for it.

## 7. What this slice borrows, said plainly

- **Half on a save.** Not built here and not this slice's to build. Until
  #414 lands, the walk declares Thunderwave (and later Whispers) `saves.Negated`
  and says so in the content row's comment. We are not bound to the 2014
  letter; the row flips to `Half` the day half exists, with no code change
  here.
- **Coverage.** Owned by the terrain design (§3.2) and pulled forward to this
  slice as its first customer, without the footprint-on-the-definition half,
  which waits for the builder. The cube is the first shape through it; cones
  and Fireball's spread follow as their spells arrive.
- **The reaction meter for monsters.** A monster's reaction is never debited
  today, so `CanReact()` on a monster answers from a meter nobody spends.
  This slice spends through the same request and asks the same query; making
  a monster's keeper honour the spend is a small, separate fix that this
  design needs and names, not one it hides.

## 8. What this does not foreclose

- **Command.** Approach and Flee are `Toward` and `Away` with `Budget: Speed`,
  paid with the target's own turn. The only new thing is who drives a
  player's turn, which is a session ruling; the directive is unchanged.
- **Monster flee.** Behavior asks `Route{Away, threats, own movement}` and
  submits the path as an ordinary `Move` intent on its own turn. It computes
  no geometry, exactly as it computes none today.
- **Dangerous terrain.** Dissonant Whispers says the fleer does not move into
  dangerous terrain. When burning ground exists (terrain rung 4), `Away`
  treats a cell whose fold carries `OnEnter` as not Standable for a
  frightened route. Today no cell is dangerous, so the clause is inert, not
  missing.
- **Pull through difficult terrain, push off ledges, flight.** Costs and
  elevation are fold facts; the policies read the fold.

## 9. Rejected alternatives

- **Computing the route in resolution** (heal-and-half's sketch). Geometry
  in the rules layer; a second author of passability the day resolution
  disagrees with the fold.
- **A push built inside Thunderwave.** The shortcut that forecloses Flee.
- **Moving the target by a client request.** The client sends references,
  never calculations; a compelled move is the engine's.
- **"Away" measured by walking distance.** Routes a fleer into the nearest
  dead end because it is far by corridor. The rule says away from *you*.
- **A new machine.** The movement machine already announces, folds,
  resolves reactions, and bills. It expected a shove.
- **Spending the reaction after the walk.** A fleer that is stopped by a wall
  still fled. Pay first.

## 10. Sequence

1. **Thunderwave.** Coverage in spatial (the embedding moves out of
   `dungeonspec/geometry.go`); the directive kind in resolution; `Route` and
   `Direct` with `Line` in encounter, beats carrying a cause; one session arm
   and the point-origin target kind; a cast profile in the bard's choice;
   the client picks a cell. Walk as §6, `Negated` per §7. Merged bottom-up,
   one module per PR.
2. **Dissonant Whispers.** `Away` with a speed budget, `PaysReaction`,
   `Provokes`. The monster reaction meter gap (§7) is fixed here or before.
3. **Monster flee** in the behavior lane, reading `Route`.
4. **Command**, after the compelled-turn ruling.

Nothing here authorises a merge, a force-push, or a shared-stack change.

## 11. Where the evidence is thin

- Not run. No production code, no test claimed passing.
- The hex line for `Line` is asserted to match `GetLineOfSight`; whether that
  line is the one a table would call "straight away" on hex has not been
  drawn out for the diagonal cases.
- Whether `walkCells` can be reused for a non-active mover without disturbing
  the pause and resume path (`pause.go:596` duplicates the movement charge)
  was not traced; `Direct` charges no turn budget, which may be the whole
  difference.
- rpg-api and rpg-dnd5e-web were not surveyed for what a `ResultMoved` needs
  on the wire beyond the steps already streamed.

— cross-team agent, on behalf of KirkDiggler
