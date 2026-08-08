# Monster AI — brainstorm

**Status:** brainstorming — noodles on the wall, deliberately pre-design.
**Issue:** [rpg-project#201](https://github.com/KirkDiggler/rpg-project/issues/201)
**Next artifact:** `design.md` in this folder, after this settles (per the cross-repo design workflow).
**Date:** 2026-08-07

---

## The itch

Monster AI today is one hardcoded policy: attack the closest target. Kirk's
starting ask was small — let a dungeon author set a **targeting style**
(closest / lowest HP / lowest AC) when placing a monster — but the real goal
is bigger: **a structure for monster behavior that can evolve**, because a new
teammate is joining to own this area. They should walk into seams and a ramp,
not a pile of special cases.

The gameplay vision this serves: our game leans into D&D *because* the
tactical-games shelf is crowded. The fresh air is **not seeing the whole
battlefield**. A monster that can break away, slip out of sight, and force the
party to split up and corner it. A patroller with a facing that hasn't seen
you yet. An ambush that emerges from a floor plan with two doors instead of a
script.

## What the engine already has (code archaeology, 2026-08-07)

The surprising finding: **the two ends of the targeting feature are already
built — only the middle is missing.**

| Exists | Where | Notes |
|---|---|---|
| Targeting strategies | `rpg-toolkit/rulebooks/dnd5e/monster/targeting.go`, `selectTargetByStrategy` | Exactly closest / lowest-HP / lowest-AC. Zero-value = closest |
| Persisted field | `monster.Data.Targeting` | Round-trips save/load. **Nothing ever sets it** |
| Authoring vocabulary | web dungeon-builder concept, `TARGET-YAML.md` "Monster targeting" | Proposes `targeting: lowest-health \| lowest-ac \| closest` per placement + per-ref dungeon defaults; explicitly flagged as waiting on engine support |
| Utility scoring turn loop | `monster.go` `TakeTurn` → `selectBestAction` | Flat `Score()` constants today; a clean seam |
| Perception world-model | `PerceptionData` (`action.go`) | Enemies pre-sorted by distance with HP/AC/adjacency, blocked hexes, A* inputs. `Allies` field exists, never populated |
| Real LOS + fog of war | `tools/spatial` `IsLineOfSightBlocked`, `encounter/perception` `CanSeeAt`/`VisibleHexesAt` | Players get honest fog of war; `viewerCanSee` already hides monsters from players |
| Real A* pathfinding | `tools/spatial/pathfinder.go` | Per-monster, per-turn, traversal predicates |
| Senses on monster data | `SensesData` (darkvision, blindsight, …, passive perception) | Stored, **never read by any decision code** |

And the gaps that define the work:

- **No wiring**: no `targeting:` key in `dungeonspec` YAML, no field on
  `SpawnInstruction`, nothing in protos. Every monster runs closest.
- **Movement ignores targeting**: `moveTowardEnemy` always paths to the
  *closest* enemy even when the strategy attacks someone else. A lowest-HP
  hunter that shuffles toward the tank looks broken.
- **Monsters are omniscient**: `buildPerception` hands every living player to
  the monster regardless of walls — the LOS filter players get is one
  function away and simply not applied.
- **No modes, no disposition, no factions, no morale.** `rpg-toolkit/behavior/`
  is a doc-only package (zero implementation). No out-of-combat monster
  action of any kind.
- Legacy curiosities: `rulebooks/dnd5e/dungeon/dungeon_data.go` has an
  orphaned `MonsterPlacementData{Role, CR}` with zero importers; the crypt
  path still seeds monsters from a hardcoded table and runs a scripted
  fallback (`npcActScripted`) when `DataJSON` is empty.

## The shape that emerged

### Behavior is data on the placed monster

The load-bearing decision. Personality is authored where the monster is
placed — dungeon YAML now, the dungeon builder UI later — not coded per
monster type. `targeting:` is the first field; disposition is the second;
patrol routes are a future third. Per-placement values with per-ref dungeon
defaults (the exact shape the builder concept already sketched). It flows
YAML → spec compiler → `SpawnInstruction` → `monster.Data`.

### Three layers with hard seams

1. **Perception** — what the monster knows. `PerceptionData`, made honest:
   filtered through LOS and senses instead of omniscient, allies populated,
   eventually memory ("last place I saw them"). Patrol/detection reuses this
   verbatim later.
2. **Decision** — what it wants to do, given what it knows. The layer the new
   owner owns and evolves.
3. **Actuation** — actions, movement, action economy. Exists; mostly
   untouched.

### Decision layer: a mode state machine on top, utility scoring underneath

Monsters have **modes** — `combat / fleeing / hiding` first; `idle / patrol /
alert` reserved for free roam. Within a mode, utility scoring picks the
concrete action (the existing `Score()` loop, made data-driven over time).

- Modes are easy to reason about, debug, and author.
- **Disposition is data that drives mode transitions**: "timid" = flee at
  half HP; "aggressive" = never flee, press lowest-HP targets. Aggressive/
  timid is not code — it's transition parameters.
- Alternatives considered: utility-only evolved in place (tangles once "run
  and hide" competes with "attack" in one scoring pass) and full behavior
  trees (the doc-only `behavior/` package's ambition — more power than
  needed, steeper learning curve for a new contributor).

### Same brain, two clocks

The monster's decision layer doesn't know what time it is:

- **In combat**, the initiative loop calls it (today's `NPCAct` path).
- **In exploration**, a **world tick** calls it — and the tick is the elegant
  free-roam unlock: the **roguelike clock**. The world advances *because
  players act*, not on a timer. The server already processes player
  exploration moves (fog of war updates on them); after applying a player's
  move, let out-of-combat monsters spend movement too. Event-driven,
  synchronous, deterministic, testable. **No real-time backend simulation
  needed** — this was the "non-zero chance of an elegant solution" and it
  showed up on schedule.

Multiplayer fairness (first cut): monsters accrue movement budget from the
**max** player displacement since the last tick, not the sum — four players
exploring must not make monsters move 4× speed. Roguelike energy systems are
the richer fallback if max-displacement feels wrong in play.

Standing still means nothing ticks — accepted as the price of not building a
simulation engine. **A real simulation engine stays on the books as the
escape hatch**: keep a running list of wants the tick can't serve; when that
list gets heavy enough, building the engine becomes worth it.

### Combat ↔ exploration is a two-way door

- **Encounter lapse**: combat needs an exit besides "everything died" — when
  a monster breaks line of sight, it's hidden but initiative keeps running:
  the party has **N turns to find it** before combat lapses and the world
  tick takes over. (N is a tunable; the pressure of a ticking search is part
  of the fun.)
- **Detection contract: D&D surprise rounds.** They saw you first → they get
  the surprise round; you saw them first → you get it. Feels like real D&D,
  and it's symmetric — the same rule powers sneaking up on a patrol and
  getting ambushed by the ghoul you chased.

The emergent payoff scenario, nobody scripts it: ghoul drops to `fleeing`,
breaks LOS, encounter lapses. Party pursues; each hex they advance, the ghoul
(now `alert`, remembering where you were) spends its movement on the existing
A*, loops through the side corridor, and comes in the door **behind** you.
Re-detection fires the surprise contract — and this time it saw you first.

## The sequencing insight

**The hide-and-seek payoff does not need free roam.** Players already have
honest fog of war, and `viewerCanSee` already removes unseen monsters from
their screens. So the moment monsters get a `fleeing` mode and LOS-filtered
perception, a bloodied ghoul can vanish mid-encounter and be hunted — inside
today's door-triggered combat. We can get a lot done in the
opening-the-door-triggers-combat world; free roam upgrades it rather than
gating it.

## Candidate ramp (each step builds what the next needs)

1. **Wire `targeting:` end-to-end + movement follows the chosen target.**
   YAML key → `dungeonspec` compiler → `SpawnInstruction` → the existing
   `monster.Data.Targeting`; fix `moveTowardEnemy`. Small, but touches every
   layer once — the ideal onboarding task for the new owner, and it
   establishes the behavior-is-placement-data precedent.
2. **Monster perception through LOS; populate allies.** Kills omniscience,
   makes `SensesData` mean something, introduces the perception machinery
   everything later reuses.
3. **Mode machine + disposition + flee/hide.** The hide-and-seek payoff
   slice, inside door-triggered combat.
4. **World tick + encounter lapse + surprise contract.** Free roam as a
   modest slice — no patrols or vision cones needed at first; pursuit alone
   justifies it. Patrols, facing, and vision cones become authored content
   on top later.

## Decisions (2026-08-07, Kirk)

- Behavior config is per-placement data with per-ref dungeon defaults, not
  per-monster-type code.
- Decision layer = mode state machine + utility scoring within a mode; not
  behavior trees, not utility-only.
- **The mode machine lives in `rpg-toolkit/behavior/`** — the doc-only
  package finally gets its implementation (rulebook-agnostic); the dnd5e
  monster package consumes it.
- Free roam = event-driven world tick off player movement; no real-time
  backend simulation. **Tick granularity: per hex crossed** as the starting
  point, numbers tunable. Standing still → nothing ticks; accepted tradeoff.
  A real simulation engine is the recorded escape hatch — keep the wants
  list, build it when the list justifies it.
- **Encounter lapse: the N-turn search window.** Monster breaks LOS →
  hidden, initiative continues; party has N turns to find it; then combat
  lapses to free roam. N tunable.
- **Detection: make all the check rolls.** Real stealth vs. perception
  (passive and active per the rules), not a flat unseen→surprise rule.
  `SensesData.PassivePerception` finally earns its keep. Surprise rounds
  resolve per D&D.
- **Disposition profiles are collections of parameters** — named presets
  (`aggressive`, `timid`) over raw transition parameters (flee-at-HP%,
  pursue-range). Authors pick a profile; the engine only ever sees
  parameters; a placement can override an individual knob.
- **Allies/factions wait for a solid use case.** `pack_tactics.go` (which
  already wants ally data) is the likely first consumer and the simple
  starting point when it comes.
- Sequencing: targeting wiring → LOS perception → modes/flee → world tick.
  Patrols deferred to content-on-top.
- **No fallback brain** *(2026-08-08)*: a monster must be fully implemented
  to be used — registry constructor + serialized `DataJSON`, or it doesn't
  enter the encounter. Enforce at the door (seeding/`AddMonster` rejects
  empty `DataJSON`) so a misconfigured monster fails loudly at load, not
  silently at turn time. `npcActScripted` is retired and its test fixtures
  get real monsters. (Verified 2026-08-07 that no production monster uses
  the fallback — both live seeding paths marshal `DataJSON`; it served only
  tests/fixtures.) Lands naturally with step 2's `NPCAct` rework.

## Open questions

- **Encounter lapse UX**: what players see/feel when initiative drops
  mid-hunt (and any proto surface for it) — design-time work for step 4.
- **Profile parameter set**: which knobs exist behind a profile (flee
  threshold, pursue range, preferred range, …) — design-time work for
  step 3.
- **Team designation on board 19** for the new monster-behavior owner —
  Kirk to triage.

## Out of scope (for this initiative)

- Patrol routes, facing/vision cones as *authored content* (the machinery
  arrives in steps 2–4; the content tooling comes later).
- Factions/allegiance model, morale systems, pack coordination — waiting on
  a solid use case; `pack_tactics` is the likely first consumer when one
  shows up.
- Difficult terrain / movement-cost pathfinding, cover, elevation LOS.
- Any real-time (timer-driven) simulation.
