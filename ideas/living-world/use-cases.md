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

## UC-3: The weekend goal (launched 2026-08-30 after #1328 merged)

The trilogy's product-pitch proof: one guild-scope goal, many parties, one
needle, a wall-clock deadline. "Pacify the region before the weekend" —
met by any mix of methods across parties, because the needle is a fold and
folds don't ask how.

### Declared content

- A world goal: predicate over the guild journal spanning ALL parties'
  facts — e.g. region pacified = the camp no longer hostile AND every
  hostage resolved (rescued, redeemed, or dead — not captive, not turned).
  Composes the two existing scenarios; no third scenario is authored.
- A deadline: a plain timestamp compared against an INJECTED clock —
  capabilities supplied, never defaulted; no world-clock machinery (§7's
  postponement holds — this is arithmetic, not simulation).
- On met-before-deadline: one emission (`GoalMet`). What it unlocks (the
  bonus stream) is the seat's business — the QuestCompleted/rulebook split
  at guild scale. Missing the deadline emits its own fact once; late
  completion never retro-fires the unlock.

### Assertions

| claim | proves |
|---|---|
| Three parties contribute by different methods (assault / diplomacy / rescues) and one needle moves | method-indifference at world scale; many parties, one fold |
| The goal reads identically from a fresh fold of the full journal | needle is derived, never stored |
| Met at T < deadline emits GoalMet exactly once; a later observe is silent | unlock fires once |
| Unmet at deadline emits the miss once; completing after does not fire GoalMet | deadlines are honest; no retro-unlock |
| The clock is injected; constructing a goal without one refuses, worded for the form-filler | supplied-never-defaulted; checklist ruling |

### Non-goals

The seat itself, streamer tooling, tenancy/guild infra beyond party
attribution, Discord anything, new scenarios, dnd5e changes (freeze).

## UC-4: The tomb (launched 2026-08-31, after the graduation)

The configuration form's proof, and the first scenario AUTHORED against the
published `world/v0.1.0` module rather than rewired onto it — content
written exactly as a rulebook would write it, on the `scenarios/` shelf.
Kirk's framing: "we craft the scenarios and expose this configuration form
to the builder."

### Declared content

- A boss room where the captain stands over a loot chest, and a hidden
  room behind a secret door holding the artifact. The door is a graph edge
  from birth — what is scoped is KNOWLEDGE of it: a location fact whose
  audience starts as {captain}.
- The quest: recover the artifact and make it out — the single-run quest
  ruled in brainstorm §9, finally instantiated.
- Two writers of the same fact:
  - **Fight** — defeat the captain and his knowledge becomes loot; the
    location fact transfers to the party's audience. The boss-room chest
    is the fight's own reward, so door-finders still have a reason to
    fight (ruled: neither path dominates).
  - **Search** — an EXPLICIT declared search verb, margin-banded (UC-2
    machinery). Success plants the same location fact with audience = the
    searcher alone. Passive reveal ("the magic feel of things being
    revealed") is ruled POLISH — the sight seam stays reserved for its
    design round.
- Knowing is not entering: the door carries an open check. When a knower
  SUCCEEDS the open check, the door-opened fact lands with room-present
  audience — now the whole party sees it.
- Required config — the form: place the artifact, set the captain monster,
  set the door's checks (find and open, bands each). `New()` refuses each
  absence with a message written for the form-filler.

### Assertions

| claim | proves |
|---|---|
| `New()` without artifact, captain, or door checks refuses, each error naming the missing field in form-filler words | the config struct IS the builder form — checklist ruling, third instance |
| A party holding no location fact gets a View with no door in it | availability is an audience fold, not a lock: nothing is gated, yet the unknown is unactable |
| Fight path: captain down → location fact transfers → open succeeds → artifact recovered AND boss chest looted | knowledge is loot; the fight pays twice |
| Search path: search succeeds → fact audience = searcher only → open → artifact recovered with zero combat facts in the journal | same fact, different writer; the journal's silence records the skipped fight |
| Party-mates who failed (or never rolled) see no door until a knower succeeds the OPEN check, which broadcasts to those present | per-player detection from birth; success at the door is what shares it |
| A failed search writes a fact and reveals nothing; the world never rewinds | no outcome invalid; append-only holds |

### Non-goals

Passive detection (the sight seam — reserved for the design round with
Kirk; explicit search only), traps, new dnd5e machinery (freeze), the
seat, tenancy, and any change to the `world` module's API — the tomb must
be expressible with what v0.1.0 ships; friction is a finding, not a
license to grow the kernel.

## Seeds (named, not written)

- The sight seam (F15→F26): who witnesses an act, from position and
  geometry — legal to design since two scenarios worked around it; wants a
  design round with Kirk, reaching toward tools/spatial.
