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

## Seeds (named, not written)

- UC-2: the hostage population — templates, instances, distribution
  predicates, successor activation ("all turned" wakes the redemption arc).
- UC-3: the streamer's weekend goal — guild-scoped objective, wall-clock
  deadline, many parties folding into one needle.
