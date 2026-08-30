# Reactions, and the first one: the opportunity attack

Slice: rpg-project#316 · Journey: rpg-project#253 · Team: Platform

## Outcome

A monster that turns and runs from a fighter gets hit for it, and the combat log says why.

## This is re-grown work, not new work

**The opportunity attack used to fire.** It worked on the old encounter system, driven by
`combat.MoveEntity` under `combat.TurnManager`, which resolved OAs inline and returned them as
`MoveEntityResult.OAsTriggered`. The rip-out (rpg-api#801) removed the driver, not the rule. The
condition, its geometry, its Disengage short-circuit and its six-case suite all survived intact
because they were never the part that broke.

Kirk, 2026-08-28: *"OA did fire in the game, it was just the old encounter system that was
ripped out. A lot of what we have been adding is in this category."*

That is the honest framing for this whole stretch of work, and it changes what "done" looks
like. We are not inventing a reaction system. We are re-growing a seam the composable encounter
stack has not yet been given, and the test of the answer is whether it is *composable* — whether
the next thing that wants to notice a step needs any wiring at all.

## The panel first

There is no new panel. **The combat log is the whole surface**, and it already exists.

```
Wolf moves to (4,7).
Opportunity Attack — Grunk's greataxe hits Wolf for 7.
Wolf moves to (5,7).
```

That one label is the only thing the wire cannot say today. Every number — roll, total, the AC
it beat, damage, weapon, crit, damage components — is already carried by the `Struck`/`Missed`
bodies, because **an opportunity attack is a strike** and records through the same `enc.Record`
path a declared swing does.

Without the label the log still tells the truth, but it reads as a bug: a fighter dealing damage
during a monster's turn with nothing saying why. So the label is in scope.

## The rulings already made

**Autofire.** No prompt, no window, no panel. Kirk, 2026-08-28:

> "having oa autofire fills our use cases right now, when that changes we can think about adding
> a panel to the user"

This supersedes Wave 2.11d ruling B4 (NPC inline, player prompted via
`InputRequired{reaction_prompt}`), whose delivery surface went with the encounter module.

The cost, recorded so it is not rediscovered: autofire spends the reactor's reaction without
asking. Nothing else competes for a reaction today, so the conflict is exactly zero. It becomes
real when Shield or Uncanny Dodge lands — an argument for building the prompt slice sooner, not
for withholding this one. The current behaviour is not "the player chooses", it is "the reaction
never happens".

**A walk loads everything.** Kirk, 2026-08-28:

> "a walk should load everything. if there is a trap along the path it will need to be loaded. i
> think the idea we know what to load on the bus will hamstring us as we want to add new things
> in"

This is decisive and it is already **R3**, a law this package wrote down and then did not apply
to `Move`. `session/announcer.go` states it for the boundary path: *"everyone in, applicability
is the effect's own predicate... deciding it out here would put a rule in the wiring."* A cast
selected by what we currently know how to trigger is a cast that silently omits the next thing.

**The condition meters itself.** Kirk, 2026-08-28: *"the OA condition should be able to track
uses this turn kind of thing like how rage does."* See "How OA knows it has been used".

## Current reality — verified 2026-08-28 against `origin/main`

> **Superseded 2026-08-30** — re-verified after the game-context phases (#318)
> landed: three of the four break points below are FIXED, `NewMovement` exists
> with an 8-test suite, and the remaining break moved (zero production callers,
> and the monster-turn path was never named here). Current reality lives in
> [survey-2026-08-30.md](./survey-2026-08-30.md), with six rulings — all ruled
> by Kirk 2026-08-30. This section stays as the record of what the design was
> shaped against.

### Where the old driver's job went, and did not land

| # | seam | state |
|---|---|---|
| 1 | `NewOpportunityAttackCondition` | zero non-test callers, either repo |
| 2 | `MovementChain` publish | only publisher is `combat.MoveEntity`, reachable only from the retired `combat.TurnManager`. The live path is `Move` → `runWalk` → `encounter.Step` → `spatial`, which publishes nothing |
| 3 | `gamectx.WithReactionReadiness` | zero non-test callers. `IsReactionReady` fails closed, so the gate is permanently false |
| 4 | `ReactionTriggerTopic` | two publishers (OA, Shield), zero subscribers |

`combat.canMakeOpportunityAttack` is a `return true` stub with both parameters discarded.

### The root cause, which is not on that list

**A walk loads only the walker.** Every other interaction in this package loads the whole cast:
`announcerSeam.Announce` does it for a clock boundary, `NewActivation` does it for a feature,
`compileResolutionCast` does it for a swing. `session/entities.go` says why — one bus per call
shared by every entity, because "a condition on one member must be able to observe what happens
to another... it is **the prerequisite for reactions**."

`Move` is the only write verb that never builds one. Nobody but the walker is listening, so
there is no one for a `MovementChain` publish to reach. This is why four green suites prove the
condition and not the feature: every OA test publishes the chain by hand and installs its own
readiness map.

### A fifth break nobody had counted

`character.EndTurn` sets `ReactionsRemaining = 0` — zeroing the one slot that must *survive* your
turn, since an opportunity attack happens on somebody else's turn by definition. Wiring that
method into the turn boundary would make OA silently never fire behind a green suite.

It is safe today **only by accident**: `character.EndTurn` has zero non-test callers. The live
end-turn path is `encounter.EndTurn` (the clock) and the sheet is left alone. This slice pins it
with a test.

### What already exists and needs no building

- **The self-metering pattern.** `SneakAttackCondition` carries `UsedThisTurn bool` in its
  persisted JSON, clears it on its owner's turn boundary, and checks it in its own predicate. Its
  doc explains why it is persisted rather than runtime: every call reconstructs the condition
  from JSON, so a runtime-only flag resets on every RPC.
- **The boundary that clears it.** `resolution.NewBoundary` publishes `TurnStartTopic` /
  `TurnEndTopic` to the whole attached cast — shipped with the clock slice (rpg-project#295).
- **The story log needs no new event kind.** `struck` / `missed` / `downed` already carry every
  number an OA produces.
- **Default readiness was already ruled.** The v1alpha2 encounter proto records it: free-cost
  reactions like OA default-on for melee combatants, spell-cost reactions like Shield default-off.
  `EncounterService` is not registered in `cmd/server`, so that text is a design record rather
  than a live contract — but the ruling stands and is reused, not re-derived.

## The seam: a walk step is an interaction

The composable answer is not a bespoke bus inside `runWalk`. It is a third sibling.

`resolution` already has two machines that are not actions: `NewBoundary` (time happened) and
`NewActivation` (a member used something they carry). `NewActivation`'s own doc names the shape
they share — *"attach everyone, do ONE thing on the interaction's own bus, collect dirty
sheets."*

**A step is that shape.** `resolution.NewMovement` publishes one `MovementChainEvent` for one
cell — `from`, `to`, the threatening set — folds the chain, and returns what the chain and its
subscribers produced. `runWalk` calls it per cell, exactly where it already calls
`encounter.Step`.

This satisfies Kirk's ruling structurally rather than by discipline:

- **Attachment is `attachAll`, the same one every other machine uses.** Nobody decides who is
  "relevant to movement". A trap, a hazard aura, a Sentinel feat, an ally's Protection — each is
  attached and each answers for itself, which is R3.
- **`encounter` stays bus-free.** It never imports `events`; determinism is its module law. The
  publish lives in `resolution`, where every other publish lives, and `session` orchestrates.
- **The next thing to notice a step needs no wiring.** It subscribes to `MovementChain` and it is
  already attached. That is the test of whether this was built right.

## How OA knows it has been used

Per Kirk's ruling, the condition tracks its own uses, `SneakAttackCondition`-style:

```go
type OpportunityAttackConditionData struct {
    Ref          *core.Ref `json:"ref"`
    CharacterID  string    `json:"character_id"`
    UsedThisTurn bool      `json:"used_this_turn"`   // new
}
```

Set when it publishes a trigger; cleared when the condition's own holder starts their turn
(`TurnStartTopic`, `SubjectID == CharacterID`) — the RAW anchor, since a creature regains a spent
reaction at the start of each of its turns.

**This is what makes the design work for monsters**, and it is why it is the right call and not
just a smaller one. Monsters have **no action economy at all** — no reaction slot, nothing for a
readiness gate to read. A meter that lives on the character sheet could never cover them, and an
unmetered monster OA would fire once per player who walks past it in a round. Self-metering gives
both kinds the same rule from the same place, with zero monster-economy work.

It also dissolves the cold-sheet problem: a member who has not yet taken a turn this fight has
`actionEconomy == nil` and cannot answer "have you a reaction", but their OA condition answers
for itself and defaults to unused.

## The build, in order

> **Amended 2026-08-30** — steps 2 and 3 below predate the game-context phases.
> Step 2 is already built (`resolution.NewMovement` + the self-metering OA
> condition shipped with toolkit#1281/#1282). Step 3's "subscribe
> `ReactionTriggerTopic`, drain per step" is now **forbidden** by session's
> no-bus pin and unnecessary — `NewMovement` drains its own triggers; readiness
> is derived by resolution's one door, and the OA spends via `SpendRequested`,
> not `SpendSlots`. The slice also gained a second seam: an `encounter.Mover`
> capability beside `Striker`, so a monster's turn provokes the same OA a
> player's walk does (ruling R1). The current build order is
> [plan.md](./plan.md).

Bottom-up, one module per step, each blocked on the tag below it (no `replace` directives — MVS
would compile old source against new deps).

**0 · rpg-api-protos — the label.** `Struck` gains field 12, `Missed` field 7:

```proto
// ReactionRef names a reaction an outcome was taken as, when it was not a
// declared action. Absent on an ordinary swing.
message ReactionRef {
  // The full core.Ref.String(), e.g. "dnd5e:conditions:opportunity_attack".
  string ref = 1;
  // Display name, e.g. "Opportunity Attack".
  string name = 2;
}
```

Additive, `buf breaking` guards it, no hand-written tests. A client that ignores the field
renders exactly as today, which is why web is a lane issue and not a blocker.

**1 · toolkit `encounter` — carry it.** `RecordInput` gains `Reaction *ReactionIdentity`,
persisted on the outcome so replay decodes it rather than re-deriving. Data, not a bus.

**2 · toolkit `resolution` — `NewMovement`.** The third sibling: attach everyone, publish one
`MovementChainEvent`, fold, collect dirty sheets. Plus `UsedThisTurn` on the OA condition and its
`TurnStartTopic` reset.

**3 · toolkit `dnd5e` (session) — wire it.** Light every member's economy when the fight forms;
`runWalk` resolves each cell through `NewMovement`;
seat the OA condition on every combatant at attach; populate `WithReactionReadiness` (OA
default-on); subscribe `ReactionTriggerTopic`, drain per step, resolve each trigger through the
existing strike path, record the beat. `MoveOutput` needs no new field — the beats are the report.

**4 · rpg-api — project the label.** One field through `convert.go`. No orchestrator change.

**5 · rpg-dnd5e-web — render the label.** Filed to the UI/UX lane under this slice, Todo / Ready
/ unassigned. Not on this slice's critical path.

## Decisions to rule on

**RULED 2026-08-28 · Every combatant is lit when the bubble forms.**

> "characters should start with their full economy I think they just cant consume it if its not
> their turn. when we go into a combat bubble, all players should have economy"

This closes the cold-sheet gap and **supersedes the lazy ignition** `session/economy.go` records:
*"THE SESSION LIGHTS THE SHEET WHEN AN ACTOR ON THE FIGHT CLOCK FIRST ACTS. Not when the bubble
forms, because nothing loads the sheets then."*

The objection in that second sentence is a real cost rather than a disagreement: lighting at
formation means a fight starting must LOAD every member's sheet, seed it, and save it — where
today it loads nobody. That is the work, and it belongs in the session step beside `runWalk`.

Two things this ruling deliberately keeps apart, because conflating them would make reactions
impossible:

- **Granting** the economy happens for everyone at formation.
- **Consuming** it out of turn is refused by the TURN GATE, which already exists — `encounter.Step`
  answers `ErrNotActive` and `Afford` blocks the non-turn verbs.
- **A reaction is the exception by definition**: it is spent on somebody else's turn, which is why
  the opportunity attack spends its slot directly through `SpendSlots` rather than through the
  turn-gated door.

**1 · Does the character's reaction slot still get spent? RULED YES 2026-08-30 (R3)** — both
meters, already built by #318 D4 + toolkit#1281: `UsedThisTurn` covers everyone, the published
`SpendRequested(ActionReaction)` keeps OA and Protection mutually exclusive on characters.
The original framing, for the record: the condition's own flag is the meter
that works for everyone. The character economy has a real `ReactionsRemaining` that Protection
fighting style already competes for.
*Recommendation: spend both where both exist.* The flag is OA's own once-per-round and covers
monsters; spending `ActionReaction` on a character keeps OA and Protection mutually exclusive,
which they are in the rules. Two meters with two distinct jobs, not redundancy — but it is worth
your eye, because the alternative (flag only) is simpler and would let a fighter both Protect an
ally and take an OA in the same round.

**2 · Does an opportunity attack stop the walk?** *Recommendation: no.* That is Sentinel, which
is deferred. The walk continues — except when the OA drops the walker, which needs no new
mechanism: `encounter.Step` already returns an `Outcome` for an ending that fired underfoot, and
`runWalk` already abandons the remaining path on one.

> **Amended 2026-08-30 (R6):** the no-new-mechanism claim was wrong — a downed walker is a
> STANDING change, `runWalk` reads only `Outcome`/`Formed`, and standing is batched once per
> walk on the documented grounds that "a Move cannot down or revive anyone", which an OA makes
> false. Ruled: standing is re-asked per step, and the walk stops when a reaction's strike
> downs the mover. The invariant comment is revised, not quietly contradicted.

**3 · Two threateners, one step.** *Recommendation: sorted by member ID*, matching the C8
determinism law and the existing sorted attach order. Both get their swing.

**4 · Reach is 5ft, fixed.** Reach weapons deferred; the predicate is already conservative and
says so.

**5 · Atomicity.** R5 unchanged: all fallible work before the commit, so a walk refused at step 4
persists no opportunity attack that landed at step 2. The existing law already covers it —
"nothing is saved on a mid-walk rejection" — and this slice must not weaken it.

**6 · The reactor swings their equipped weapon**, through the same strike path a declared attack
uses. Not a special-cased unarmed poke.

## Not now

The reaction prompt and `play/interrupt` — its own slice; `session/doc.go` already scopes it as
"Wave 5" and names rpg-toolkit#964's slice-2 commit as the reference implementation. Shield,
Uncanny Dodge, Hellish Rebuke. Reach weapons. Sentinel's stop-the-movement. Per-cell movement
metering (10ft per difficult hex, refuse entry with 5ft left — Kirk's ruling 2026-08-28, deferred
with prone). Condition gating of the reactor (incapacitated, prone, stunned) — rides the `Afford`
veto slice.

## Done when

Kirk walks the branch and a wolf standing next to his fighter turns to run:

- the fighter swings automatically, and the log names it an opportunity attack
- the reaction is spent, and a second fleeing enemy that round gets away untouched
- it refills at the start of the fighter's next turn
- a **player** who **Disengages** walks away untouched *(restated 2026-08-30, R4 — monsters
  have no route to the Disengaging condition; giving them one is a separate question)*
- two adjacent threateners both get their swing, in a deterministic order
- a walk refused mid-path persists no opportunity attack that landed during it
- **and the composability test**: a new condition that wants to notice a step subscribes to
  `MovementChain` and needs no change to `runWalk` to be heard

## Learning log

- "OA never fired" was wrong and worth correcting: it fired on the old stack. The rip-out took
  the driver and left the rule. Much of the current work is this shape — re-growing seams the
  composable stack has not been given — and describing it as missing features misreads both the
  cause and the size.
- The four documented break points were real and none of them was the work. The work was that
  `Move` never loads the cast every other interaction loads — a law (R3) the package had already
  written down for the boundary path and not applied here.
- Selecting what to put on the bus is the antipattern. `announcer.go` had already said so:
  "deciding it out here would put a rule in the wiring."
- `character.EndTurn` zeroing the reaction slot is the same defect class as
  `fail-closed-not-fail-silent` — a correct-looking method whose absent caller is the only reason
  the system works. Found by asking what refills the slot, not by reading the OA code.
