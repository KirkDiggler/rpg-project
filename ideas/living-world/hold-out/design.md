---
status: DESIGN, proposed 2026-09-05 (supersedes the 2026-09-04 draft; the survey it carried moved to ../disposition/brainstorm.md)
journey: rpg-project#326 · brainstorm: ../disposition/brainstorm.md · tool 1: ../intel-record/design.md (shipped 2026-09-04)
law: the run's world is the only state — this slice moves two readers into it: sides, and knowledge
---

# The hold-out — design

## 0. Purpose

Three tools for the designer, proved by one scenario:

1. **Factions with a disposition** — who fights whom, and the predicate that
   changes it.
2. **The predicate** — one authorable grammar (`round | down | fact`) used by
   two fields.
3. **Arrivals** — a placement that enters the run when its predicate holds.

The scenario: a goblin camp is hostile to the party until its chief comes to
know the party saved the Wiseman. A messenger's letter carries that fact; carry
it to the chief and the camp stops attacking. Kill the chief instead and
reinforcements arrive at the gate.

## 1. The file

```yaml
factions:
  - { id: goblins, mind: chief }

dispositions:
  - { between: [goblins, party], stance: hostile, until: { fact: saved-wiseman } }

intel:
  - { id: wisemans-letter, reveals: { fact: saved-wiseman } }

place:
  - { id: chief,  ref: "dnd5e:monsters:goblin-boss", at: [12,4], faction: goblins }
  - { id: scout,  ref: "dnd5e:monsters:goblin",      at: [4,2],  faction: goblins }
  - { id: letter, ref: "dnd5e:props:scroll", at: [1,3], holdable: true,
      holds: [wisemans-letter], arrives: { round: 6 } }
  - { id: reinforcements, ref: "dnd5e:monsters:goblin", at: [1,4], count: 3,
      faction: goblins, arrives: { down: chief } }

exits:
  - { id: front-gate, at: [1,3] }

scenarios:
  hold-out: { convince: goblins }
```

## 2. Vocabulary (dungeonspec v2, additive)

| field | type | rules |
|---|---|---|
| `factions[].id` | id | unique; `party` MUST NOT be declared (reserved for the players' side) |
| `factions[].mind` | placement id | MUST name a monster placement whose `faction` is this faction |
| `place[].faction` | faction id | monsters only; MUST name a declared faction; absent → the reserved `monsters` faction |
| `dispositions[].between` | `[faction, faction]` | both MUST exist (`party` allowed); unordered; one disposition per pair |
| `dispositions[].stance` | `hostile \| neutral \| allied` | closed set |
| `dispositions[].until` | predicate | legal only with `stance: hostile`; when it holds the stance becomes `neutral` |
| `intel[].reveals` | `{ door: id } \| { fact: id }` | exactly one key; `fact` ids are plain strings, declared by mention |
| `place[].arrives` | predicate | monsters and props; the placement is in reserve until it holds; `at` MUST be floor |
| `scenarios.hold-out.convince` | `entity_ref(faction)` | the scenario's only field |

**Predicate grammar** — exactly one key:

| form | holds when | grain |
|---|---|---|
| `{ round: N }` (N ≥ 1) | any fight in the run has started round N | truth |
| `{ down: <placement id> }` | that member is Down | truth (reads Standing) |
| `{ fact: <id> }` on `until` | the faction's `mind` knows the fact | audience (the mind's) |
| `{ fact: <id> }` on `arrives` | the fact exists in the run's journal, learned by anyone | truth |

**Defaults that keep today's dungeons unchanged:** every unauthored monster is
in `monsters`; `party` and `monsters` are mutually hostile; every faction is
allied with itself; a declared monster faction with no disposition toward
`party` is hostile to it; declared monster factions are neutral to each other.

**Refusals, written for the form-filler** (dungeon): unknown faction on a
placement; a `mind` outside its faction; `party` declared; two dispositions for
one pair; `until` on a non-hostile stance; `arrives.at` not floor; an unknown
placement in `{ down }`. The dungeon ALLOWS an `until` fact no record reveals
(pre-release: show the cost). The SCENARIO refuses it: `hold-out` refuses when
no faction is bound, the bound faction has no `mind`, no hostile disposition
with `until` exists between it and `party`, or nothing reveals that fact — "a
hold-out nobody can win".

## 3. The run's world (encounter)

**MUST**

1. One journal and one graph exist from `New` and from `Load`, whether or not
   anything is concealed. Entities: members, factions, regions, doors, props.
   Edges: `belongs-to` (member → faction), structure (as today), and one
   `hostile-to` / `allied-with` edge per direction from the declared and
   default dispositions.
2. `MemberKind` stays a kind. The three side readers — formation
   (`trigger.go` `sidesInContactOrder`), `fightIsDecided`, `bubbleHasPlayer` —
   ask the graph: two members are opposed iff a `hostile-to` edge stands
   between their factions. A member in reserve or Down appears in no pair.
3. Knowledge is facts with audiences in the one journal: `known:door:<id>` and
   `holds:intel:<record>` as shipped, with the receiver as audience;
   a `fact` reveal writes `known:fact:<id>` with the receiver as audience.
   `knowsFact(member, id)` is a fold; no reader keeps a copy.
4. The disposition flip is a declared reducer over the mind's `known:fact`
   facts and an `AdoptStance` projection that rewrites both directions of the
   pair's edge to neutral. Persistence stays facts-only; the stance is derived
   on every load, never stored.
5. When a flip removes hostility between two factions and a fight is formed
   between members of those factions, the encounter dissolves it with a new
   sealed cause `ByStance()`. Members of a third faction still hostile keep
   their fight.
6. Presence transfer rides `sweepOccupancy`: a member holding a record whose
   `reveals` names a fact, standing in the region of a faction's `mind`,
   teaches the mind (the record copies; the holder keeps it).
7. A placement with `arrives` is in reserve: no cell, no turn, in no pair,
   absent from every projection for every member (the never-authored
   yardstick: a run with reserved placements projects byte-identically to one
   without them, until arrival). On the first verb after its predicate holds
   it is placed at `at` — nearest free floor cell in the same region if `at`
   is occupied — with an `arrived:<id>@<cell>` fact; the same verb's sight
   refresh then forms or joins a fight as for any member walking into view.
8. Predicates are evaluated in one place, at the end of every verb before the
   sight refresh, and at `RoundStarted`. `{ round }` outside any fight never
   holds. `{ down }` reads Standing; Standing does not move into the journal in
   this slice.
9. A faction whose `mind` is Down can no longer learn: `until: { fact }` can
   never hold for it. This is a consequence, not a loss. Mind succession is a
   shelf.
10. `scenarios.New` declares `Endings: [TriggerStance{Between, Stance ≠ hostile}]`;
    the existing withdrawal and defeat endings stay.

**Spawn** (rpg-api → session → encounter): `MemberInput.Faction` and
`MemberInput.Arrives` are hand-carried like `Holds`. Content for a reserved
monster resolves at launch; the encounter holds the member in reserve.

## 4. Resolution

`castView.IsHostile` / `IsAllied` ask the reloaded run's graph (resolution's
`Input.World` is already the encounter's data). `castRelations` and the
`cast-side` entities are deleted. Resolution's encounter pin moves from
v0.51.0 to the tag carrying §3. Sneak Attack and Pack Tactics change nothing
and answer from the run.

## 5. Session

- `Spawn` carries `Faction` and `Arrives`.
- Roster row gains `faction`; reserved members are absent from roster and
  atlas until arrival (member-scoped by absence, as concealed doors are).
- Beats: `stance` `{between: [a, b], stance}` on a flip; `arrived`
  `{id, kind: monster|prop, cell}` on arrival; the `ended` beat names the
  hold-out ending.
- Capabilities unchanged (Witness, CheckResolver, Sight, Roller, TurnDriver).

## 6. Wire (protos, additive)

- `PublicMemberInfo.faction` — string, free-form (factions are content, never
  an enum).
- Beat payloads `STANCE_CHANGED {between, stance}` and
  `ARRIVED {id, kind, cell}`.
- `PutDungeon` verbatim; `ListScenarios` gains the `faction` entity kind.

## 7. The designer (web)

- Dungeon sections beside Scenarios and Intel: **Factions** (id; mind =
  dropdown of monsters placed in that faction) and **Dispositions** (faction,
  faction, stance select, `until` = predicate editor, shown only for hostile).
- **Predicate editor**, one component used by `until` and `arrives`: form
  select (`round` → number; `down` → dropdown of placed monsters; `fact` →
  dropdown of fact ids declared by intel records, free text allowed with the
  "no record reveals this" note).
- Placement inspector: `faction` dropdown on monsters; `arrives` predicate
  editor on monsters and props.
- Intel form: `reveals` kind select (`door | fact`) and the id.
- Scenario tab: `hold-out` with the `convince` dropdown and the description.
- Play: roster coloured by faction; `stance` and `arrived` beats narrated;
  reserved placements never drawn.
- YAML round-trips byte-stable; every refusal in §2 renders inline at the
  field it names.

## 8. Rulings (proposed; Kirk vetoes on the PR)

| # | ruling | proposed |
|---|---|---|
| R1 | a flip dissolves a formed fight between the two factions | yes, `ByStance()` |
| R2 | the stance after `until` holds | `neutral` ("not hostile"); allied is authorable only as a static stance |
| R3 | presence grain | the mind's region, the yardstick Search uses |
| R4 | `party` and `monsters` | reserved; unauthored monsters are `monsters` |
| R5 | predicate grammar and grains | `round \| down \| fact`; `fact` = mind's knowledge on `until`, truth on `arrives` |
| R6 | reserved placements | spawned at launch, absent from every projection, placed on the first verb after the predicate holds |
| R7 | the dead mind | the flip is gone; consequence not loss; succession shelved |
| R8 | `until` naming an unrevealed fact | dungeon allows, scenario refuses |
| R9 | which clock `{ round }` counts | any fight in the run; outside a fight never; the world clock stays postponed |

## 9. Acceptance

| # | proof | how |
|---|---|---|
| A1 | camp `hostile until {fact}`; nobody knows; a fight forms on sight | scene |
| A2 | the letter carried into the chief's region mid-fight: stance neutral, fight dissolves, hold-out ending fires | scene + walk |
| A3 | the letter carried into the scout's region: nothing | scene |
| A4 | chief Down: reinforcements at the gate on the next verb, hostile, fight forms or joins | scene + walk |
| A5 | the letter arrives at round 6 and not before | scene |
| A6 | reserved placements: projection byte-identical to a run without them, for every member, until arrival | scene (the yardstick) |
| A7 | every pre-existing encounter, resolution, and session scene passes unchanged under the default factions | the rung-1 bar |
| A8 | after a flip, a goblin is no longer an enemy for Sneak Attack; before it, unchanged | scene |
| A9 | save after the flip, load: still neutral; no stance stored | scene |
| A10 | the whole file authored through forms; YAML round-trip byte-stable; refusals inline | screenshot + test |
| A11 | Kirk walks both branches on one stack before any PR opens | the walk |

## 10. Cut

- **Step A — sides and knowledge.** §2 without `arrives`; §3 items 1–6, 9, 10;
  §4; §5 without `arrived`; §6 without `ARRIVED`; §7 without the arrives
  field. The letter lies at the gate; no reinforcements. Walkable: hold the
  letter, carry it to the chief mid-fight, the camp turns.
- **Step B — arrivals.** The predicate grammar in full, `arrives` on props and
  monsters, reserve, `arrived`, the predicate editor. Walkable: the letter at
  round 6; kill the chief, reinforcements.
- Both steps on branches, one local stack, Kirk's walk, then PRs bottom-up:
  dungeonspec + encounter → resolution → session → protos → api → web.

## 11. Shelves (named, empty)

hand / throw the letter as a verb · a walking messenger (NPC lane, Interact
give) · betrayal: attacking a neutral faction writes a fact that flips it back
· mind succession · directed dispositions · allied-after-flip · world-clock
arrivals · behavior reading stance (Billy's record) · Standing as journal facts.
