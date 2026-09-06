# The opportunity attack reaches the player — design

**Date:** 2026-09-06
**Status:** Design. Three rungs; rung 1 is in flight, rungs 2 and 3 are cut here.
**Umbrella:** `ideas/session-combat/`. Continues rpg-project#316 (movement reaches the rules)
under journey rpg-project#253.
**Depends on:** rpg-toolkit#1321 (`encounter.Mover`), rebased on main as its successor PR.

---

## What prompted this

> *"if I see a couple monsters that are going to pass me because my friend behind me is
> hurt, I might neglect a hit on one and want to target the second so there is a need here.
> I think getting that in would be a clean slice."* — Kirk, 2026-09-06

And the framing that makes it the right first customer for the interrupt spine:

> *"we are only playtesting now. there are no stakes in the dungeon runs and we are really
> just testing out the systems."*

So the opportunity attack becomes the first honest producer of a window — the thing
`session/doc.go` says wave 5 brings — and it gets the smallest window that lets a player
choose.

## The facts this stands on (toolkit main `3477eb44`)

- **No opportunity attack fires on the live stack today.** `resolution.NewMovement` has
  no caller. The player's walk is `runWalk` stepping the encounter cell by cell
  (`session/move.go:416`); the monster's walk is `executeTurnIntent`'s `Move` case
  (`encounter/clocks.go:718`). Neither announces a step.
- **The movement machine is symmetric.** `MovementInput{Mover, MoverKind, From, To,
  Reactions, Roller}`; it asks `Reactions.AttackFor(reactorID)` per reactor at the trigger
  and resolves each as a strike sub-machine (`resolution/movement.go:20-37, 280`). A wolf
  passing a fighter and a fighter passing a wolf are one machine.
- **#1321 is the trigger.** `Mover` beside `Striker`, required at both doors; the monster
  loop announces each step before taking it and asks standing per cell; `RecordInput.Reaction`
  names what a beat was taken *as*. The PR does not implement `Mover` in the session.
- **The session's `Striker` seam is the template** for its `Mover`: `strikerSeam{m, scope}`
  at `session/striker.go:32`, installed at `write.go:1103`, `RefusingStriker` at the
  inspect-only doors (`start.go:167`, `read.go:563`).
- **The interrupt module is built and tested**: `Ledger`, `Window`, `Pose(Audience,
  Options, Payload, At)`, `Answer(Window, By, Choice)`, `PendingFor`, `LedgerData` for
  persistence (`play/interrupt/ledger.go`, `data.go`). The session's adapter to it was
  deleted with its only producer; the four things to re-create are listed at
  `session/doc.go:206-216`.
- **The wire already has the reaction slot**: `Struck.reaction` (`ReactionRef`) in
  `session/v1alpha1/events.proto:567-589`.
- **Free reactions are default-on by ruling** (`resolution/truth.go:107-128`): OA readiness
  is installed for melee combatants without asking. This design changes that for
  players, and only for players.

## Rung 1 — the trigger (in flight)

#1321 rebased onto main on `feat/316-mover-capability-r2`: 7 conflicting files, ~100 new
construction sites take a `Mover`. Behavior surface unchanged; the seven mutation proofs
re-run. Encounter minor tag. Nothing observable changes in play yet.

## Rung 2 — the opportunity attack fires (small, walkable)

**Session implements `Mover`.** `moverSeam{m, scope}.Move(ctx, enc, mover, from, to)`:

1. Build `ReactionAttacks` off the cast the way the striker seam builds a strike: for each
   member with OA readiness, a melee attack definition, **and a hostile stance toward the
   mover** (the cast view's `IsHostile`, live since rpg-project#375), `AttackFor` answers
   that definition. Monsters and players alike. This is where rpg-toolkit#899 (OA ignores
   hostility) and #766 (a reaction fires between allies in free roam) close: the run's
   fold answers, not kind.
2. `resolution.Resolve` with `NewMovement{Mover, MoverKind, From, To, Reactions, Roller}`,
   `World: scope.enc.WorldView()`, no cost (a reaction's price is on the reactor's ledger
   and is charged where the strike is compiled — as today for `Attack`).
3. For each `ReactionOutcome`, record the struck/missed beat through `Encounter.Record`
   with `Reaction: {Ref: refs.Conditions.OpportunityAttack(), Name}`. The beat reaches the
   client through the existing `Struck.reaction` field.
4. Dirty sheets back to the scope as every verb does.

**Two callers, one rule.** The monster loop calls `Mover.Move` per cell (#1321). `runWalk`
calls it per cell before `enc.Step` — the PR's loop comment names this debt. Announce
before the step in both, so the reactor's reach is checked against where the mover still
stands (#1321's first pinned ordering).

**Disengage is already answered, before anyone is asked.** The machine folds the
`MovementChain` first and reads triggers off the fold; the `Disengaging` condition
subscribes to that chain and marks the opportunity attack prevented
(`conditions/disengaging.go:59-69`, `resolution/movement.go:78`). Step of the Wind applies
Disengage for a ki point (`features/step_of_the_wind.go:21`). So a disengaging mover
produces no trigger, and in rung 3 no window opens — the question is never asked and then
refused.

**Done-when.** On the local stack: a wolf walks out of the fighter's reach on its turn and
the log shows the fighter's strike marked as an opportunity attack; the fighter walks out
of a wolf's reach and takes the bite; the monk spends a ki point on Step of the Wind and
walks out of the same wolf's reach untouched. All automatic. rpg-api and web need no change beyond
re-pinning: the field is on the wire already, and the dock can label a struck beat by its
reaction ref.

Modules: toolkit `session` (one PR); rpg-api re-pin; web shows the label (optional in this
rung).

## Rung 3 — the player chooses (the first window)

**Where the pause lands.** Three cases, one machine:

| Mover | Reactor | Who answers | Pause |
|---|---|---|---|
| player | monster | the monster's mind, instantly | none |
| player | player | the reactor | inside the walker's `Move` verb — the shape the old spine handled for the perception pose |
| monster | player | the reactor | inside the monster's turn, mid-walk — **new** |

Kirk's case is the third row. The monster drive today runs a whole turn inside one verb
call and persists nothing until commit; it is re-entrant at turn granularity (stops at the
first player member, resumes on the next `EndTurn`). This rung makes it re-entrant at
**cell** granularity for exactly one reason: a player is being asked something.

**Mechanism.**

1. `moverSeam.Move` finds a player reactor with OA readiness. Instead of answering
   `AttackFor`, it **poses**: `Ledger.Pose{Audience: reactor, Options: [strike, hold],
   Payload: {mover, from, to, reactor, attack definition}, At: seq}`. The seam returns
   `ErrWindowOpen` (a new sentinel) with the window id.
2. The monster loop treats `ErrWindowOpen` as a **checkpoint, not a malfunction**: it stops
   before the step, leaves the mover on `from`, and returns the remainder of the turn —
   the unplayed path and the budget — to the verb that was driving it. The verb writes the
   ledger and the remainder into the session aggregate (the "frozen value", `doc.go:211`)
   and commits. Encounter first, session second, as before.
3. **Verb freeze.** While a window is open, every change verb except `Answer` is refused
   with `ErrWindowOpen`; reads are exempt. The classification is the one at the #964
   slice-2 commit, restored.
4. **`Answer`** — a new session verb, `AnswerInput{Session, Member, Window, Choice}`.
   `strike`: run the strike for that reactor through the movement machine's own path and
   record the beat with its reaction; `hold`: record nothing. Then **resume**: take the
   step that was announced, and hand the remainder back to `driveMonsterTurns` to finish
   the turn. Standing is asked after the strike as #1321 already does, so a mover the
   reaction drops falls in the cell it was leaving.
5. Player-mover, player-reactor (row two) is the same pose and the same answer; the
   remainder is the walker's own remaining path, and `Move` returns "suspended at cell i"
   the way it returns "stopped on a formed bubble" today.

**On the wire.** ADR-0042's rule: the backend tells the client what it can do, in
declarations. So an open window is a **declaration** for its audience — `VERB_REACT`,
`available: true`, candidates = the options, target = the mover — surfaced by `Afford`
exactly as attacks are, and answered by one RPC, `React{DeclarationID, Choice}`, which is
`Answer` in proto clothing. Two beats: `WINDOW_OPENED` (audience, mover, options) and the
existing struck/missed with its reaction. Every other member's Afford answers "a window is
open" as the shortfall reason on their frozen verbs.

**Client.** The dock shows the REACT declaration with two buttons. The canvas rings the
mover. Nothing else.

**Rulings, made here for playtest, revisit before stakes exist:**

- **One window at a time.** Two monsters passing two fighters in one step pose serially;
  the second waits on the first's answer.
- **Default is `strike`.** A window has no timer; an absent player freezes the fight, and
  in playtest that is the right behaviour — the freeze is visible and nobody loses
  anything. When a timer arrives, it answers `strike`, so an absent player still swings.
- **Only players are asked.** Monsters keep default-on readiness and answer instantly.
  Costed reactions (Shield) stay default-off and out of scope; they become the second
  customer of this exact window.
- **Readiness stays as it is.** The player is asked *whether*, not *whether ever*; the
  "ready my reaction" toggle of the old stack does not return.

**Done-when.** On the local stack: the monk spends a ki point on Step of the Wind and walks
past two wolves untouched; two wolves walk past the fighter on their turns; the
first pass opens a window, the fighter chooses `hold`, the wolf continues; the second pass
opens a window, the fighter chooses `strike`, the beat shows the opportunity attack, and
the wolf's turn finishes from where it stopped. A restart of rpg-api between the pose and
the answer changes nothing.

Modules, bottom-up: toolkit `encounter` (checkpoint return from the drive; remainder
type), toolkit `session` (ledger load, freeze, `Answer`, resume; `Afford` REACT
declaration; window beat), protos (`VERB_REACT`, `React` RPC, `WindowOpened`), rpg-api
(pass-through), web (dock buttons, ring). One PR per module; integrate on a local stack from
pushed branches before opening PRs.

## Shelf

- A timer on the window, with `strike` as its answer.
- Several windows from one step (the pack passes the line).
- Sentinel (a reaction that stops the step: `MovementOutcome.Prevented` exists and nothing
  sets it).
- Shield on the post-attack-roll boundary; Counterspell on the cast declaration — the same
  window, a costed option, the readiness opt-in.
- Traps noticing a wanderer on the world clock (#1321's own note at the free-roam pump).
