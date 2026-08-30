---
name: Living World use cases
status: UC-1 ratified as the primitive-fleshing target 2026-08-30; spike not started (dnd5e tag freeze active)
purpose: tangible builds that verify the §14 kernel; each use case lists the primitives it exercises and the assertions that prove them
---

# Living World — use cases

A use case here is an executable spec: authored content as data, player
actions as generic verbs, outcomes asserted end-to-end in toolkit tests.
Zero path-specific code is the standing acceptance criterion — if a path
needs bespoke logic, the kernel is wrong, and that discovery is the point.

## UC-1: The bandit camp (ratified)

One camp, one goal, many ways in, many outcomes. Exercises every kernel
primitive at once.

### Declared content (data, not code)

- Entities: the camp; its leader (occupying the `leads` slot); bandits
  (group-grain by default); one named lieutenant (individual grain, for the
  seeing-through drama).
- Relationships: `leads(leader, camp)`, `belongs-to(bandit, camp)`,
  `hostile-to(camp, party)` initially.
- Derivations: camp allegiance follows its leader's; camp attitude is a
  fold over facts the camp has witnessed, with declared thresholds.
- Objective (method-indifferent): `camp is no longer hostile to the guild`.
- Routes are NOT declared. They emerge.

### Paths and assertions

| path | verbs composed | facts written (audience) | asserted outcome |
|---|---|---|---|
| Front door | approach, attack | assault witnessed (camp) | camp alerted; combat starts formed-up; objective via defeat |
| Back way | sneak (dice), enter | entry fact (empty camp-audience) | camp unsuspecting; combat starts surprised |
| Changeling | sneak, kill leader (empty audience), disguise-as-leader | kill (no camp witnesses); planted "leads" belief | camp allegiance follows the impostor; objective met with no camp combat |
| Diplomacy | parley, persuade (dice, expertise tilts) | persuasion facts (camp) | attitude fold crosses threshold; `hostile-to` → `allied-with`; camp fights FOR the guild afterward |
| Blown disguise | changeling path + one failed check witnessed by the lieutenant only | reveal fact (audience: lieutenant) | lieutenant hostile, rest of camp unchanged — audience grain proven |

Every path: universally attemptable (no prerequisites — the barbarian may
sneak), resolved by injected dice, journal shows attributed facts, world
state derived by fold only (nothing stored).

### Primitives exercised

Entities and slots (leader replacement); edges (allegiance flip);
audience-scoped facts (stealth, disguise, partial reveal); folds (attitude,
allegiance-follows-leader); method-indifferent predicates (the objective);
verbs (attack, sneak, kill, disguise, parley, persuade); dice injection.
The seat is NOT exercised here — UC-1 stays seatless on purpose.

### Non-goals

Rendering, wire, protos, the town, populations/templates, world goals, any
model. Toolkit-only, test-driven.

## UC-2: The hostage population (ratified as next, 2026-08-30 after #1326 merged)

Three parties, one quest template, a population of hostages. Exercises the
§12 machinery UC-1 barely touched — and serves as quest's trial: after UC-2
we know whether quest is a real package or folds into graph.

### Declared content

- Template: "rescue the hostage", population 3. A claim takes one offer off
  the board and mints an instance — the claiming party's OWN hostage
  (instance identity: name drawn from a declared list; no model anywhere).
- Instance outcomes as transitions: captive → rescued (success) or
  captive → turned (failure — the hostage joins the bandits' side: an
  allegiance edge rewrite on that individual).
- Rolled dispositions for the rescued (rule of the dice, scripted roller):
  guard / tries-to-repay / carries-word-of-another-quest — each written as
  an attributed fact.
- Successor template: "turn them back — or put them down", activating on the
  distribution predicate `captive == 0 AND turned == total` (no rescues
  remain). Its instances target the turned; redemption is the repeatable
  flip: turned → redeemed (or turned → dead).

### Assertions

| claim | proves |
|---|---|
| Party A's failure turns A's hostage only; B's and C's untouched | instance isolation — collisions dissolve (§12) |
| A claimed offer is unavailable to a second claimant; completing/failing releases nothing back | claims off the board |
| Successor activates exactly when the fold over ALL instances crosses the predicate — never early, never re-fires | distribution predicates |
| Rescued dispositions land as rolled, attributed facts | rule of the dice; successes seed content |
| A turned hostage redeemed by a successor instance reads allied again in every view | the repeatable flip |
| All world change arrives by fold over the population; nothing stored | §10 holds at population scale |

### Standing orders (per §20–21, ratified)

- Promote shared machinery upward ONLY what both scenarios demand — the
  composer (world root Act loop, Verb/Act/Emission) extracts here, from
  real duplication, not speculation. Resolver/Attempt move journal → root
  with it.
- Extract the scenario contract from the shape banditcamp and hostagecamp
  actually share (second-instance law, §21).
- Constructor errors written for the future form-filler ("this template
  needs a population size"), fail closed, nothing defaulted.

### Non-goals

Guild/tenancy infra (three party IDs suffice), the town, world goals
(UC-3), deadlines, any dnd5e changes (freeze), the seat.

## Seeds (named, not written)

- UC-3: the streamer's weekend goal — guild-scoped objective, wall-clock
  deadline, many parties folding into one needle.
