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

### Combat ↔ exploration is a two-way door

- **Encounter lapse**: combat needs an exit besides "everything died" — when
  no monster and no player have mutual awareness, initiative drops and the
  world tick takes over.
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

## Tentative decisions (to confirm at design time)

- Behavior config is per-placement data with per-ref dungeon defaults, not
  per-monster-type code. *(2026-08-07)*
- Decision layer = mode state machine + utility scoring within a mode; not
  behavior trees, not utility-only. *(2026-08-07)*
- Free roam = event-driven world tick off player movement; no real-time
  backend simulation. *(2026-08-07)*
- Detection resolves via D&D surprise rounds; encounter lapse returns to
  exploration when mutual awareness is lost. *(2026-08-07)*
- Sequencing: targeting wiring → LOS perception → modes/flee → world tick.
  Patrols deferred to content-on-top. *(2026-08-07)*

## Open questions

- **Where does the mode machine live?** Inside `rulebooks/dnd5e/monster`, or
  does the doc-only `rpg-toolkit/behavior/` package finally get its
  implementation (rulebook-agnostic)? Leaning: start in the monster package,
  extract if a second rulebook ever wants it.
- **Encounter lapse mechanics**: what exactly ends initiative (all monsters
  unseen for N rounds? immediately?), and what do players see/feel in the UI
  when combat lapses? Any proto surface needed?
- **Surprise round details**: flat "unseen → surprise" or real stealth
  vs. passive perception checks? (`SensesData.PassivePerception` is sitting
  right there.)
- **Tick granularity**: per-hex-crossing? distance accumulator? What happens
  to monster budget when players stand still (nothing, presumably — that's
  the roguelike deal)?
- **The scripted fallback path** (`npcActScripted` when `DataJSON` is empty)
  — do we still need it once behavior matures, or is it retired as part of
  step 1/2?
- **Disposition vocabulary**: named profiles (`aggressive`, `timid`,
  `defensive`) vs. raw transition parameters (flee-at-HP%, pursue-range)?
  Named profiles author better; parameters compose better. Maybe profiles =
  presets over parameters.
- **Allies/factions**: populating `PerceptionData.Allies` is step 2, but a
  real faction model (monsters vs. monsters? charmed allies?) is explicitly
  out of scope — when does it stop being deferrable? (`pack_tactics.go`
  already wants it.)
- **Team designation on board 19** for the new monster-behavior owner —
  Kirk to triage.

## Out of scope (for this initiative)

- Patrol routes, facing/vision cones as *authored content* (the machinery
  arrives in steps 2–4; the content tooling comes later).
- Factions/allegiance model, morale systems, pack coordination.
- Difficult terrain / movement-cost pathfinding, cover, elevation LOS.
- Any real-time (timer-driven) simulation.
