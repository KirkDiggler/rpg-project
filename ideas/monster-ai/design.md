# Monster AI — design

**Status:** in review.
**Issue:** [rpg-project#201](https://github.com/KirkDiggler/rpg-project/issues/201) · **Brainstorm:** [`brainstorm.md`](brainstorm.md) (settled 2026-08-08)
**Date:** 2026-08-08

This is deliberately a **thin design**: the load-bearing contracts are specified
precisely; the decision layer's internals are specified only by their
constraints, because that layer belongs to the incoming monster-behavior owner
to evolve. Phase 1 (this team) builds the structure; the owner extends it.

**Design principles (Kirk, 2026-08-08):**

- The goal is not to make things work — full stop. It's composable,
  extensible components that give the game a foundation to evolve on.
- A good slice runs **internal → represented in the game**, acting in the
  seams. Visible in play, or it isn't a slice.
- **Every slice leaves a tool behind** that makes the whole fit together
  better — starting with the combat log growing from debug dump toward a
  DM's narration (§4a).
- **Leave shelves**: places where a future capability (sound, new senses,
  new modes) can land additively — chosen so they cost near-nothing now and
  never hinder current work. Shelves are called out inline below as
  **[shelf]**.

---

## Phases

- **Phase 1 — this team, before the owner arrives:** ramp steps 1–3
  (targeting wiring → honest perception → behavior package with
  modes/disposition/flee). Ends with hide-and-seek working inside
  turn-based combat.
- **Phase 2 — slice 4 (world tick / free roam): deliberately deferred.**
  Direction sketched in §6; the architectural decisions are consciously
  postponed until slices 1–3 make us comfortable in the
  perceive → decide → act loop. Built by this team or as the owner's first
  big slice — Kirk's call when the time comes.
- **Owner's extension surface (ongoing):** new modes, profile vocabulary,
  patrol/facing content, senses depth, pack tactics — everything inside the
  decision boundary (§4).

## 1. What we build on (verified 2026-08-07/08)

| Existing piece | Where | Role in this design |
|---|---|---|
| `TargetingStrategy` + `selectTargetByStrategy` | toolkit `rulebooks/dnd5e/monster` | Step 1 wires authoring to it |
| `monster.Data.Targeting` (persists, never set) | same | The field the YAML lands in |
| `PerceptionData` (enemies w/ HP/AC/distance, geometry) | same, `action.go` | The monster's entire world model — stays that way |
| LOS + fog of war (`CanSeeAt`, `VisibleHexesAt`, per-viewer broker events) | toolkit `encounter/perception`, `broker.go` | Step 2 applies it to monsters; free roam reuses it verbatim |
| A* pathfinding w/ traversal predicates | toolkit `tools/spatial` | All monster movement, both clocks |
| Sight-triggered combat entry (`checkCombatEntry` after every `Move`) | toolkit `encounter/combat.go` | Detection contract hangs here |
| Pocket-cleared combat exit → `ModeFreeRoam`, same encounter | toolkit `encounter/combat.go` | Lapse becomes a sibling exit |
| `MoveNPCSteps` — NPC movement, **no mode gate** | toolkit `encounter/npc.go` | The world tick's actuation seam (tests-only today) |
| One RPC + one stream for both phases (`MoveEntity`, `StreamEncounter`) | protos v1alpha2, rpg-api handlers | No new wire surface needed |
| Orchestrator verb conventions (one file/verb, single `load`, keyed mutex) | rpg-api `orchestrators/encounter/v2` | The tick is one more verb |

**Non-negotiable inherited invariants:** client sends references, never
calculations; orchestrators never import the rulebook or protos; toolkit owns
all rules and publishes per-viewer events itself.

## 2. Data model — behavior is placement data

Authoring (dungeon YAML, `dungeonspec`):

```yaml
defaults:                          # dungeon-wide, per-ref
  dnd5e:monsters:skeleton:
    targeting: lowest-health
    profile: aggressive

rooms:
  - id: hall
    place:
      - ref: dnd5e:monsters:ghoul
        at: [5, 3]
        targeting: closest         # per-placement override
        profile: timid
        params:                    # optional per-knob override
          flee_at_hp_pct: 60
```

- `targeting`: `closest | lowest-health | lowest-ac` (the builder concept's
  exact vocabulary).
- `profile`: a named preset that **resolves to parameters at compile time**.
  The engine stores and reads only parameters; profiles exist in authoring
  land. A placement may override individual knobs after the preset applies.
- Knob set v1 (grows under the owner): `flee_at_hp_pct`,
  `pursue_range_hexes`, `preferred_range_hexes`. Zero values = today's
  behavior (never flee, press the attack).

Flow: `PlacedEntry` → compiler → `SpawnInstruction{…, Targeting, BehaviorConfig}` →
`SeedMonsters` → constructor → `monster.Data{Targeting, Behavior *behavior.MachineConfig}`
→ `DataJSON`. **No proto changes in phase 1** — behavior rides `DataJSON`,
which the wire already carries opaquely. *(One additive exception, decided
at planning via §7's revisit clause: the decision-breadcrumb rationale is a
new `target_rationale` ref field on `ActionResolved` — the log can't say
"why" without a signal. See `plan.md`.)*

**Enforcement (decided):** a monster enters an encounter fully implemented or
not at all. `AddMonster` rejects empty `DataJSON`; `npcActScripted` is deleted
and its fixtures get real registry monsters. Fail at load, loudly — never a
silent second brain at turn time.

## 3. Three layers, hard seams

```
Perception  (encounter layer builds it; LOS/senses-filtered; the ONLY input)
    ↓  PerceptionData
Decision    (behavior/ package: mode machine + utility; owner's territory)
    ↓  intent (move path / action choice / mode change)
Actuation   (dnd5e actions, action economy, MoveNPCSteps; publishes events)
```

- **Perception** is built by the encounter layer, never by the monster —
  and it is **stimuli → knowledge, not a visibility boolean**. A monster's
  world model is a set of *beliefs with timestamps* ("saw the wizard at
  [5,3] two turns ago", "heard something by the door"), produced by typed
  stimulus channels and decaying into memory — never ground truth. Sight
  (LOS via `CanSeeAt` + `SensesData`: darkvision, blindsight, tremorsense)
  is the first channel; `Allies` gets populated; "last seen at" memory
  lives here, not in the decision layer. **[shelf — sound]**: `noise` is
  the second channel's reserved slot: interactions emit stimuli with
  intensity and falloff (a door forced with STR is loud, finessed with DEX
  is quiet), and monsters with hearing in range gain a knowledge entry.
  Costs us a typed stimulus kind + knowledge entries now; nothing else
  changes when sound lands. Symmetry note: players already have
  `View{Memory, KnownEntities, ActiveSenses}` ("reserved for future
  slices") — monster perception is the same component pointed the other
  way, not a new invention.
- **Decision** consumes a perception snapshot + behavior parameters + a
  roller; returns an intent. It **never mutates encounter state, never
  touches the event bus, never resolves rules**. Deterministic given inputs
  and roller.
- **Actuation** stays the existing action/economy/movement machinery.

## 4. The `behavior/` package (toolkit, rulebook-agnostic)

The doc-only package gets its implementation. **`behavior/` owns the
structure; rulebooks own the vocabulary.** The package never learns what
"fleeing" means — mode values are opaque to it, declared by the rulebook
that uses them:

```go
// rulebooks/dnd5e/monster declares its vocabulary:
const (
    ModeCombat  behavior.Mode = "combat"
    ModeFleeing behavior.Mode = "fleeing"
    // ModeHiding, ModeIdle, ModePatrol, ModeAlert — declared when built, not before
)
```

House style throughout (and for everything this initiative writes):
`NewSomething(cfg *SomethingConfig) (*Something, error)` for construction,
`XInput`/`XOutput` types on funcs.

```go
package behavior

type Mode string   // vocabulary belongs to rulebooks

// One machine per monster, constructed at hydration from config resolved
// at authoring-compile time (profile → knobs).
func NewMachine(cfg *MachineConfig) (*Machine, error)

type MachineConfig struct {
    Initial Mode
    Knobs   Knobs   // flee_at_hp_pct, pursue_range_hexes, …; zero values = today's behavior
    // transition-rule registration: shape is the owner's call
}

// Pure: state in, state out. Never mutates encounter state, never touches
// the bus; deterministic given input + roller.
func (m *Machine) Step(in *StepInput) (*StepOutput, error)

type StepInput struct {
    Current Mode
    Percept Snapshot   // rulebook-agnostic: self HP%, enemies (dist, seen), allies, geometry
    Roller  dice.Roller
}

type StepOutput struct {
    Mode    Mode
    Because Reason     // structured why: rule fired, inputs that fired it
}
```

### 4a. Decisions are breakdowns (observability)

The toolkit already returns rich breakdowns for attack rolls so the client
can render them; **AI decisions get the same treatment**. Every decision —
mode transition, target choice, retreat path — carries a structured
`Because`, and monster turns publish it as a decision breadcrumb alongside
the events they already emit.

Two renderings of the same breadcrumb **[shelf]**:

- **A debug view** (a Debug/Story toggle on the combat log) renders the
  mechanics raw: refs verbatim, positions and hex counts on movement,
  rationale refs including `closest`, sequence numbers —
  `target=wizard strategy=lowest-hp candidates=3` once the wire carries
  candidates. When the question is "why did it flee *there*?", the answer
  is already recorded.
- **The combat log as DM narration**: D&D doesn't show HP, so the story
  view narrates in-fiction — "the skeleton turns on the wounded wizard,"
  "the ghoul breaks and runs."

**Priority (Kirk, 2026-08-09, learned from slice-1 acceptance):** while
the game is under construction, **the debug view is the primary lens** —
withholding information during development is silly; the in-fiction voice
is a polish layer. Slice 1's acceptance run proved it: even with the
narrative clause rendering, verifying the decision took Redis dumps and a
hand-drawn diagram. Debug v1 is client-only over data already on the
stream (web#740); the candidate-snapshot wire signal is decided at
slices 2–3 per §7's revisit clause. Every later slice adds its decisions
to the same stream, and both views inherit them.

- The dnd5e monster package **adapts** `PerceptionData` → `behavior.Snapshot`
  and consults the machine at the top of `TakeTurn` (combat clock) and the
  free-roam step (tick clock). Mode then selects the utility-scoring set:
  `combat` scores attacks as today; `fleeing` scores retreat paths (away +
  toward unseen hexes); `hiding` holds position while unseen.
- **What's fixed:** the boundary above (construction + `Step` signature),
  purity, vocabulary-belongs-to-rulebooks, and the rule that disposition =
  config knobs driving transitions — never code per monster type.
- **What's the owner's:** the machine's internals, the transition-rule
  shape, scoring evolution (today's flat `Score()` constants becoming
  data-driven), new modes, new knobs. The constraint set is the contract;
  the craft is theirs.

## 5. Two clocks, one brain

- **Combat clock (exists):** `driveNPCChain` → `NPCAct` → `TakeTurn`, which
  now consults the machine first (a `fleeing` monster spends its turn
  disengaging/retreating instead of attacking), then applies targeting
  strategy for both **attack selection and movement** (fixing
  `moveTowardEnemy`'s always-closest bug — step 1).
- **Free-roam clock (slice 4 — deferred; sketch, see §6):** a new toolkit verb, e.g.
  `(*Encounter).FreeRoamTick(TickInput)` beside `npc.go` — for each living,
  un-engaged monster: build (LOS-honest) perception → machine step → if the
  intent is movement, execute via the `MoveNPCSteps` path, spending a
  **tick budget**, and publish the same per-viewer `EntityMoved` /
  appear/disappear events players already understand. Fog handles "we see
  it, it doesn't see us" with zero new client work.
- **N-turn search window & lapse (slice 3):** when every monster in the
  initiative pocket is unseen, a search counter runs; after N player turns
  unseen (N tunable, start N=3), the encounter exits combat exactly like
  the existing pocket-cleared path (`exitCombatForHeldPlayers` →
  `SetMode(ModeFreeRoam)`). Lapse is a sibling of pocket-cleared, not new
  machinery.
- **Detection & surprise (slice 4 — deferred):** real check rolls (decided) — monster
  Stealth vs. passive Perception and the reverse, using `SensesData` at
  last. Detection feeds the existing `checkCombatEntry` seam; whoever wins
  detection gets the surprise round via initiative seeding. Also run the
  tick + entry check on `OpenDoor` — closing the known gap where a player
  can stare through a fresh doorway un-flagged until someone moves.

## 6. The free-roam orchestrator (rpg-api) — direction, not commitment

**Status (Kirk, 2026-08-08): free roam deliberately carries the fewest hard
decisions in this design.** It's still fuzzy, and that's fine at this
stage — the architecture here gets decided for real only when slice 4 is
cut, informed by what slices 1–3 teach us about the
perceive → decide → act loop. Below, the **findings** are stable facts
about the codebase and will hold; everything shaped like a decision is a
**sketch**.

**Finding** — the part that surprised us in the best way: **there is no
exploration subsystem to reconcile with** — exploration is the same
encounter object, same RPC, same stream, behind `ModeFreeRoam`. So the
orchestrator side of free roam is likely **one new verb file**, by the
book:

- `internal/orchestrators/encounter/v2/exploration_tick.go` —
  `ExplorationTickInput{EncounterID, MoverID, HexesMoved}` /
  `ExplorationTickOutput{}`; single `o.load` → toolkit `FreeRoamTick` →
  persist. Per-encounter keyed mutex (same `npcDriveLocks` pattern), so
  concurrent movers can't double-tick.
- **Call site, not RPC:** invoked from `MoveEntity` post-`Move` when the
  encounter is in `ModeFreeRoam` — the exact spot `driveNPCChain` is invoked
  for the combat branch today — and from `Interact`'s door-open path. The
  client never knows the tick exists; no new proto surface. If a tick
  flips the mode (detection → combat), the existing `ModeChanged` /
  `InitiativeRolled` / `TurnStarted` events tell every client the same way
  sight-triggered entry does today.
- **Budget (sketch — brainstorm starting point, revisit at slice 4):** per
  hex crossed by the moving player; monsters
  accrue from the **max** displacement since the last tick, not the sum of
  all movers. Standing still ticks nothing. Numbers tunable; a real
  simulation engine remains the recorded escape hatch if the wants-list
  outgrows this.
- **No event bus on the tick path (finding-backed):** nothing in
  exploration uses the dnd5e bus today — fog/doors/reveals go straight to
  the per-viewer broker. The tick follows suit: broker events only. We
  deliberately do **not** route tick movement through the per-hex
  movement-resolver machinery (`iterateMovementSteps`' OA/reaction
  subscriptions) — there are no opportunity attacks out of combat. This
  makes the tick *cheaper* per hex than a player's own free-roam move is
  today. (Separate observation, not this initiative's job: player free-roam
  moves currently pay that per-hex bus tax too.)

## 7. Delivery

One in-flight PR per module; each step is one toolkit branch that
accumulates until its consumer stops asking (per the wave rules). Local-env
branch verification is the PR evidence.

Each slice runs internal → visible in the game, births a component, and
leaves a tool behind:

| Slice | Component born / tool left | Repos touched | Acceptance (local-env, visible) |
|---|---|---|---|
| 1. Targeting end-to-end | Behavior-as-placement-data rails; **first decision breadcrumbs** in the combat log | toolkit (`dungeonspec`, `rulebooks/dnd5e/monster`); content YAML in rpg-api | A `lowest-health` skeleton re-targets the wounded PC and **moves toward its chosen target**; the log states the choice in D&D voice. Rider on the same branch: `AddMonster` rejects empty `DataJSON`, `npcActScripted` deleted, fixtures get real monsters |
| 2. Perception component | Stimulus→knowledge world model (sight channel, `SensesData` live, allies, last-seen memory); **[shelf — sound]** in place | toolkit (`encounter`, `encounter/perception`) | A monster neither attacks nor paths toward what it hasn't perceived; breadcrumbs show what it believes ("last saw wizard at [5,3]"); sneaking behind a wall mid-combat actually works |
| 3. `behavior/` v1 — two modes + lapse | The `Machine` seam, born with an honest `Snapshot`; profiles→params plumbing | toolkit (`behavior`, `rulebooks/dnd5e/monster`, `encounter`) | A `timid` ghoul at low HP switches to `fleeing` (breadcrumb says why), vanishes from player screens; N-turn search window runs; lapse returns the party to free roam mid-dungeon. `hiding` is deliberately left as the owner's natural first extension |
| 4. World tick + detection + surprise *(deferred — architecture re-decided after slice 3)* | The free-roam clock (sketch: `exploration_tick.go` + `FreeRoamTick`) | toolkit (`encounter`), rpg-api | Pursuing a fled monster through rooms works; it can re-enter behind the party; detection resolves by check rolls; surprise round seeds initiative; door-open gap closed |

Slices 2–3 need no rpg-api code changes beyond content YAML and toolkit
version bumps; slice 4 is the first substantial api-side work. Protos: the
breadcrumb revisit clause fired at planning — slice 1 carries one additive
`ActionResolved.target_rationale` field (plus its translate + web log
rendering); otherwise zero expected through slice 4 (mode/initiative
events already exist), revisited only if surprise-round presentation needs
a dedicated signal.

## 8. Out of scope (unchanged from brainstorm)

Patrol routes and vision cones as authored content (machinery arrives with
steps 2–4; content tooling later, under the owner). Factions/morale/pack
coordination — waiting on a solid use case; `pack_tactics` is the likely
first consumer. Difficult terrain, cover, elevation LOS. Any timer-driven
simulation.
