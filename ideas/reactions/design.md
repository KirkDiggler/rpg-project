# Reactions, and the first one: the opportunity attack

Slice: rpg-project#316 · Journey: rpg-project#253 · Team: Platform

## Outcome

A monster that turns and runs from a fighter gets hit for it, and the combat log says why.

## Why now

`conditions.OpportunityAttackCondition` has been complete, correct and fully tested since
Wave 2.11d, and has never fired in the running game. Kirk hit it directly on 2026-08-28
("attack of opportunity is also not firing"). It is the only thing on the level-1 audit list
where something a player is entitled to *every single turn* silently does nothing.

It is also the seam two other queued items need. The movement chain has two consumers, not one:
reaction triggers and movement-cost modifiers. Prone's half-speed and difficult terrain both
live on the second one. Building the chain for OA builds the place they go.

## The panel first

There is no new panel. **The combat log is the whole surface**, and it already exists.

Today, a wolf disengaging from a fighter produces:

```
Wolf moves to (4,7).
Wolf moves to (5,7).
```

After this slice:

```
Wolf moves to (4,7).
Opportunity Attack — Grunk's greataxe hits Wolf for 7.
Wolf moves to (5,7).
```

That one label is the only thing the wire cannot say today. Everything else — the roll, the
total, the AC it beat, the damage, the weapon, the crit flag, the damage components — is
already carried by the `Struck`/`Missed` bodies, because **an opportunity attack is a strike**
and records through the same `enc.Record` path a declared swing does.

Without the label the log still tells the truth, but it reads as a bug: a fighter dealing damage
during a monster's turn with nothing saying why. So the label is in scope.

## The ruling already made

**The opportunity attack autofires**, for players and monsters alike. No prompt, no window, no
panel. Kirk, 2026-08-28:

> "having oa autofire fills our use cases right now, when that changes we can think about adding
> a panel to the user"

This supersedes Wave 2.11d ruling B4 (NPC resolved inline, player surfaced as
`InputRequired{reaction_prompt}`), whose delivery surface — the old encounter stream — was
removed with the encounter module in rpg-api#801.

The honest cost, recorded so it is not rediscovered: autofire **spends the reactor's reaction
without asking**. Nothing else competes for a reaction today, so the conflict is exactly zero.
It becomes real the day Shield, Uncanny Dodge or Hellish Rebuke lands. That is an argument for
building the prompt slice sooner, not for withholding this one — the current behaviour is not
"the player chooses", it is "the reaction never happens".

## Current reality — verified 2026-08-28 against `origin/main`

### The four seams the condition's own doc left to "the orchestrator"

| # | seam | state |
|---|---|---|
| 1 | `NewOpportunityAttackCondition` | zero non-test callers, either repo |
| 2 | `MovementChain` publish | only publisher is `combat.MoveEntity`, reachable only from the retired `combat.TurnManager`. The live path is `Move` → `runWalk` → `encounter.Step` → `spatial`, which publishes nothing |
| 3 | `gamectx.WithReactionReadiness` | zero non-test callers. `IsReactionReady` fails closed, so the gate is permanently false |
| 4 | `ReactionTriggerTopic` | two publishers (OA, Shield), zero subscribers |

`combat.canMakeOpportunityAttack` is a `return true` stub with both parameters discarded.

### The root cause, which is not on that list

**A swing loads the whole roster onto one shared bus. A walk loads only the walker.**

`attack.go`'s `compileResolutionCast` gathers every member, sorts them, and attaches each onto
one `newCallBus()`. `session/entities.go` states why in its own words: one bus per call shared
by every entity, because "a condition on one member must be able to observe what happens to
another — that is the whole reason the bus exists here, and it is **the prerequisite for
reactions**."

`Move` never builds one. Nobody but the walker is listening. Fixing seams 1–4 without this
changes nothing, which is why four independent green test suites prove the condition and not
the feature: every OA test publishes `MovementChain` by hand and installs its own readiness map.

### A fifth break nobody had counted

`character.EndTurn` sets `ReactionsRemaining = 0`. Reactions are the one slot that must *survive*
your turn — an opportunity attack happens on somebody else's turn by definition. Wiring that
method into the turn boundary would zero the slot for precisely the window in which it matters,
and OA would silently never fire behind a green suite.

It is safe today **only by accident**: `character.EndTurn` has zero non-test callers. The live
end-turn path is `encounter.EndTurn` (the clock); the sheet is left alone, so the slot seeded at
the holder's turn start persists through everyone else's turns and is refilled by
`RefreshForTurn` when their next turn comes. That is the correct behaviour, held in place by
nothing. This slice pins it with a test.

### What already exists and needs no building

- **The reaction economy is real and persisted.** `Data.ActionEconomy.ReactionsRemaining`,
  written by `ToData`, restored by `Load`, priced by `Afford`, seeded to 1 by `seedTurn`, and
  already spent by a shipped condition — Protection fighting style calls
  `SpendSlots(ActionReaction, 1)`.
- **Default readiness was already ruled.** The v1alpha2 encounter proto records it: free-cost
  reactions like OA are default-on for melee combatants; spell-cost reactions like Shield are
  default-off. `EncounterService` is not registered in `cmd/server`, so that text is a design
  record rather than a live contract — but the ruling stands and is reused, not re-derived.
- **The story log needs no new event kind.** `struck` / `missed` / `downed` already exist and
  already carry every number an OA produces.

### Two gaps the build has to answer

- **A cold sheet cannot answer "have you a reaction."** A member who has not yet taken a turn
  this fight has `actionEconomy == nil`. In initiative order that is every player before their
  first turn. See decision 1.
- **Monsters have no action economy at all.** No reaction slot, no `SlotsLeft`, nothing. The
  readiness gate has nothing to read for a monster reactor. See decision 4.

## The build, in order

Bottom-up, one module per step, each blocked on the tag below it (no `replace` directives —
MVS would compile old source against new deps).

**0 · rpg-api-protos — the label.** `Struck` gains field 12 and `Missed` field 7:

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
degrades to exactly today's rendering, which is why web is a lane issue and not a blocker.

**1 · toolkit `encounter` — carry it.** `RecordInput` gains `Reaction *ReactionIdentity`,
persisted on the outcome so replay decodes it rather than re-deriving. Encounter stays
bus-free; this is data.

**2 · toolkit `dnd5e` (session) — the slice itself.** In dependency order inside the module:

- **Cast the walk.** `Move` gathers the roster and attaches every member onto one shared
  `newCallBus()`, in sorted order, reusing the `compileResolutionCast` pattern rather than a
  second copy of it. This is the one structural change; everything else hangs off it.
- **Seat OA.** At that attach, apply `NewOpportunityAttackCondition` to every melee combatant.
  Seat it programmatically — it is universal, which is also why it is correctly absent from
  class grants.
- **Populate readiness.** `gamectx.WithReactionReadiness` from each seated member's persisted
  reaction slot, OA default-on per the existing ruling.
- **Publish per step.** `runWalk` publishes `MovementChain` for each cell with `from`/`to` and
  the threatening set, then folds it — the shape `combat.MoveEntity` already uses, lifted to
  the layer that owns the bus. **Not in `encounter`**: that module never imports `events`,
  determinism is its module law, and putting a bus in it to fire this would break that.
- **Subscribe and resolve.** Subscribe `ReactionTriggerTopic`, drain after each step, resolve
  each trigger through the existing strike path, spend the reactor's reaction, record the beat.
- **Report.** `MoveOutput` needs no new field — the beats are the report.

**3 · rpg-api — project the label.** One field through `convert.go`. No orchestrator change.

**4 · rpg-dnd5e-web — render the label.** Filed to the UI/UX lane under this slice, Todo /
Ready / unassigned. Not in this slice's critical path.

## Decisions to rule on

**1 · A cold sheet has no reaction.** A player who has not acted yet this fight cannot answer
the readiness question, so in round 1 they would not threaten anyone.
*Recommendation: light cold sheets during the walk's cast.* The ignition law says the session
lights a sheet "when an actor on the fight clock first acts", and its stated objection to
lighting earlier is that "nothing loads every combatant's sheet" at bubble formation. At this
point in the walk we have just loaded all of them, so the objection does not apply. The
alternative — accepting that OA does not work before your first turn — is a rule a player would
correctly report as a bug.

**2 · Does an opportunity attack stop the walk?** *Recommendation: no.* That is Sentinel, which
is deferred. The walk continues through the remaining cells — except when the OA drops the
walker, which needs no new mechanism: `encounter.Step` already returns an `Outcome` for an
ending that fired underfoot and `runWalk` already abandons the remaining path on one.

**3 · Two threateners, one step.** *Recommendation: sorted by member ID*, matching the C8
determinism law and the existing sorted attach order. Both get their swing; identical inputs
must produce identical stories.

**4 · Monsters have no reaction economy.** A monster reactor has nothing to meter, so an
unmetered monster OA could fire once per player who walks past it in a round — a real rules
violation that would feel worse than the current silence.
*Recommendation: give the monster's encounter member record a per-round used-reaction flag* and
meter both kinds. It is small, it is data, it lives where the round is already known.
*The alternative* is landing player-side OA only (which covers Kirk's reported case — a fleeing
wolf getting hit — and needs no monster work at all) and filing monster OA to the Monster AI
lane under journey #201. **Asymmetric OA is the one option I would not pick**: players learning
they can walk away from a wolf for free is a worse game than no OA at all.

**5 · Reach is 5ft, fixed.** Reach weapons (glaive, halberd) deferred. The condition's predicate
is already conservative here and says so.

**6 · Atomicity.** R5 applies unchanged: all fallible work before the commit, so a walk refused
at step 4 persists no opportunity attack that landed at step 2. The existing law already covers
it — "nothing is saved on a mid-walk rejection" — and this slice must not weaken it.

**7 · The reactor swings their equipped weapon**, through the same strike path a declared attack
uses. Not a special-cased unarmed poke.

## Not now

The reaction prompt and `play/interrupt` — its own slice, and `session/doc.go` already scopes it
as "Wave 5", listing the four things to re-create with rpg-toolkit#964's slice-2 commit as the
reference implementation. Shield, Uncanny Dodge, Hellish Rebuke. Reach weapons. Sentinel's
stop-the-movement. Per-cell movement metering (10ft per difficult hex, refuse entry with 5ft
left — Kirk's ruling 2026-08-28, deferred with prone). Condition gating of the reactor
(incapacitated, prone, stunned) — that rides the `Afford` veto slice.

## Done when

Kirk walks the branch and a wolf standing next to his fighter turns to run:

- the fighter swings automatically, and the log names it an opportunity attack
- the reaction is spent, and a second fleeing enemy that round gets away untouched
- it refills at the start of the fighter's next turn
- a monster that **Disengages** walks away untouched
- two adjacent threateners both get their swing, in a deterministic order
- a walk refused mid-path persists no opportunity attack that landed during it

## Learning log

- The four documented break points were real and none of them was the work. The work was that
  `Move` never built the shared bus `Attack` builds — a fact stated plainly in the package's own
  doc comment, one level away from where every investigation had been looking.
- `character.EndTurn` zeroing the reaction slot is the same defect class as
  `fail-closed-not-fail-silent`: a correct-looking method whose absent caller is the only reason
  the system works. Found by asking what refills the slot, not by reading the OA code.
