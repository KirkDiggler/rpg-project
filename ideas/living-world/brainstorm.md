---
name: Living World
status: brainstorm ratified in conversation 2026-08-30; no implementation started (dnd5e tag freeze active)
related: ideas/perceive/design.md (audienceFor shelf), ideas/fog-of-war/ (viewer-scoped knowledge concept, hex era), rpg-project#201 (monster behavior lane)
---

# Living World — the game beyond the fight

Brainstormed 2026-08-30 (Kirk + technical director) during the monster-behavior
tag freeze. The dream: this is a D&D game, not a combat dungeon clearer. When
you play D&D at a table, most of the fun is outside combat — reading an NPC,
spotting what others miss, choosing the clever route past an obstacle. Local AI
models keep getting cheaper to run; the long arc has one in the core, pushing
the story forward. This document records the dream, the ladder we climb toward
it, and the rulings already made.

## 1. The core ruling: the AI is a client in the DM seat, never the core

The worker framing is "an AI runs the story." The architect framing, ruled
here: the rules engine stays the sole authority — dice, DCs, consequences, the
bus — and the AI occupies the **DM seat** as one more actor speaking the same
wire protocol the players speak. It proposes verbs; the engine disposes.

What this one ruling buys:

- **Hallucination is contained** to what the verbs allow. The model cannot
  invent an outcome the engine wouldn't grant any client.
- **The model is swappable** — local model, cloud model, or a scripted
  behavior table — without touching the engine. The floor is a good game with
  zero AI; the ceiling is a good game with one.
- **Every verb built for humans is a lever for the AI**, and vice versa.
  Nothing is built twice.
- Billy's monster-behavior lane (#201) is this exact pattern at embryo scale:
  an actor deciding actions through the same interface everyone else uses. The
  DM seat is the monster seat, grown up.

Corollary, ruled by Kirk: **improviser over authored content comes first.**
Humans author modules — rooms, NPCs, secrets, plot beats; the AI (eventually)
runs them and improvises connective tissue. A generative DM that invents the
story itself is really ambitious and is something we work *into*, not try out
of the gate.

## 2. The ladder — each rung playable without the next

1. **Passive gates** (v1 spine, ruled). Authored secrets carry check
   thresholds; the engine auto-reveals as the party encounters them. Zero new
   UX — information flows down the log to the right people.
2. **Offered rolls.** The scene offers a check when a character qualifies
   ("Roll Insight — something feels off"). Dice get rolled; needs an
   offer/decline panel flow. This is the rung where multi-route gates (§4)
   become load-bearing, because choice needs an actor.
3. **Declared intent.** The player says what they attempt ("I search the
   desk") and the game maps it to a check. The true tabletop feel — and the
   rung where the local model first enters, as an *arbiter* parsing intent
   into existing verbs.
4. **Improviser over authored content.** The model voices NPCs, adjudicates
   flavor, improvises connective tissue between authored beats.
5. **Generative DM** (someday, explicitly deferred). Authored content becomes
   seed material rather than the spine.

The supporting systems do not need the AI; the AI needs them. Rungs 1–2 ship
with no model in the loop. That de-risks the whole dream.

## 3. Passive gates — the v1 spine (ruled)

A secret in authored content is information whose audience is "whoever
qualifies." Trigger model interrogated:

- Per-step proximity — atmospheric, but rides the walk loop and spams
  evaluation. **Shelf, not v1.**
- Explicit search verb — that's rung 3 (declared intent). Not v1.
- **Room entry — wins.** On a character entering a room, evaluate that room's
  unrevealed secrets against the passives of members present, once. Coarse,
  deterministic, cheap.

Passive score = 10 + skill modifier, **derived, never stored** (zero values
tell the truth; the advantage-to-+5 rule is a shelf). The gate reads the party
as it actually is — no assumed classes; whoever carries Perception carries the
party's eyes.

### Perception is per-player (REVISED same day — supersedes the first cut)

First cut said: party-level reveal with attribution, per-player deferred to
v1.0. **Kirk's ruling, later the same day: "our events are player detected."**
The player who made the check sees the door; players who did not succeed do
not. A trap shows only for the players who passed the check. Detection is
individual, and so is the reveal.

How this sits with the pre-v1 full-data ruling (rulings carry their scope —
this narrows, it does not reverse): combat and table beats keep the
everyone-audience until v1.0; **detection beats are qualifier-scoped from
birth**, making the secret door the `audienceFor` shelf's first stocked
customer (ideas/perceive) — exactly the one-function policy change the shelf
was built to make cheap. The surviving pre-v1 letter: the raw dungeon payload
still reaches every client until v1.0 tightens the wire, so secrecy is
enforced at the beat/render layer, not cryptographically — fine among
friends; the debug feed remains a dev view of everything.

The fun hiding in the ruling, named: per-player reveals create social
gameplay for free. The rogue who sees the trap is the one who yells "wait—"
over voice. The game never models information sharing; friends share by
talking. The asymmetry costs nothing and produces the table moment.

## 4. Gates are plural from birth (ruled 2026-08-30)

Kirk: the same obstacle should offer **multiple routes with different
consequence profiles**. DEX finesses the stuck door quietly; STR forces it —
and alerts monsters one door away. Perception or Insight can reach the same
secret by different senses.

Rulings that fall out:

- **On the wire, a gate is a repeated list of routes** —
  `{skill-or-ability, dc, consequences}` — and v1 passive content simply
  authors one-route gates. Costs nothing now; avoids a wire reshape at rung 2.
- **A consequence is not failure.** STR-forcing *succeeds and* makes noise.
  Route cost and route outcome are separate ideas. This is what turns checks
  from a stat tax into a choice — which is where the D&D fun lives.
- **Noise is an event on the bus with a graph scope.** The dungeon already
  knows door adjacency, so "one door away" is graph distance — no new
  geometry. Monsters reacting to noise is the monster-ai lane's side of a
  clean seam: we emit; their behavior subscribes. Load-everything-on-the-bus
  as already written.

## 5. First slice — secret door in the tomb (forcing case, ruled)

Panel back:

- **What friends see:** party enters the burial chamber. Log:
  *"Standre's keen eyes catch a seam in the east wall — a hidden door."* A
  door that wasn't rendered fades in; from there it is just a door (the
  run-ending work already made doors open). Behind it, a room the party would
  otherwise never have found.
- **Authoring:** dungeon builder marks a door `secret` with a gate. Two fields
  on an existing noun, not a new noun.
- **Mechanics:** room-entry evaluation; every present member is evaluated
  individually; reveal is one-way state plus one attributed event whose
  audience is the qualifiers (per-player, per the §3 revision).
- **Wire:** gate routes on the door declaration, one `SecretRevealed` event.
  Additive, small.

Nothing builds during the freeze. The journey files as an unadopted starter;
slice issues get filed when the freeze lifts and Kirk adopts.

## 6. Supporting systems, seen as seams

- **Skill checks** — same mechanical shape as attacks and saves: roll +
  modifiers + advantage machinery the toolkit already has. One composable
  check mechanic, not twelve features.
- **NPC interaction** — the transcript is the combat log's sibling; a
  conversation is a scene with actors on the bus.
- **Puzzles** — world state + interactables + a win condition; a lever is a
  door with a different predicate. Dungeon-builder props are the substrate.
- **Story state** — a quest/journal of durable facts the DM seat reads and
  writes. The one genuinely *new* noun; everything else generalizes existing
  ones.

## 7. Shelves left named and empty

- Proximity-triggered gates (per-step, "as you pass the east wall…").
- Offered-roll panel flow (rung 2 UX).
- Intent parsing (rung 3; where the local model enters).
- Noise reactions (monster-ai lane's side of the seam).
- NPC scenes / dialogue transcripts.
- The quest/journal noun.
- audienceFor stocking (per-player secrecy at v1.0 — ideas/perceive).
- Advantage → +5 on passive scores.
- The objective/goal component (promote the run-ending predicate when the
  second goal type arrives — see §9).
- World places as the dungeon graph at a different zoom (noodle: village
  square is a room, the road is a corridor; travel reuses graph, walk, and
  door-distance noise). Written down, not claimed.
- Escortable friendly NPCs (rescue archetype = behavior-driven actor — the
  monster-ai lane's pattern, third recurrence of the seat).
- Shared / overlapping worlds (journal facts are party-attributed from birth
  so a cross-party fold stays possible — see §10; nothing built until a real
  use case).
- Fact identifier vocabulary (wants the typed-ref work — ideas/typed-ref-vocabulary).
- Authored content variants gated on journal facts (the gate shape, campaign
  timescale).
- **The world clock — postponed by explicit ruling** (Kirk 2026-08-30): "a
  number of solutions… designing one now will be us pretending we know what
  we need from a world clock." The journal accepts facts from any writer, so
  whatever clock arrives later plugs in as one more writer. Decide when a
  real need forces it.
- Traps as detection payloads (second customer of per-player reveal; trigger
  mechanics couple to combat and wait for the freeze to lift).

Per the shelf rule: nothing gets stocked until a real use case arrives.

## 8. Not now

- Any model integration (rungs 3–5).
- Perception limiting of any kind (pre-v1 ruling holds).
- Puzzles beyond door-shaped predicates.
- Anything touching frozen dnd5e module tags (Billy's freeze, #201).

## 9. Quests — core components outside the rulebook (amended 2026-08-30)

Second dream session, same day. Beyond the dungeon: quests — a goal, a story
wrapper, a resolution ("return what the goblins stole", "rescue the prisoner
from the bandits"). Kirk's probe: is quest machinery core toolkit, outside the
rulebook? Ruled: yes, almost all of it, split by a test we already use —
**if it can be expressed over refs and bus events without knowing any 5e
semantics, it is core.**

Core (rulebook-free):

- **A quest is a subscriber, not an actor.** It never drives anything — it
  watches the bus (which carries the whole cast by law) and advances when
  facts flow past: "entity X downed", "item Y possessed by party", "actor
  reached location Z". Objectives are predicates over events; progress is a
  ledger. Rulebook-agnostic by construction because it references refs, not
  rules.
- **Lifecycle**: offered → accepted → active → completed/failed. One small
  state machine per quest.
- **The journal** — durable story facts quests write and the future DM seat
  reads (§6's one new noun; quests are its first writer).

Rulebook (dnd5e):

- What a reward *is* (XP, gold, treasure tables). Core emits
  `QuestCompleted`; the rulebook subscribes and grants — same emit/react
  shape as noise → monsters.
- Check resolution on quest steps. Note: **the §4 gate recurs here
  identically** — "persuade the barkeep to reveal the camp" is the same
  repeated-routes gate as the stuck door. One component, two customers
  already: the strongest evidence the gate is core.

Two anchor observations:

- **Quest v0 already shipped and wasn't called that.** The run-ending work —
  boss down + reach the exit — is a goal predicate with a resolution.
  "Retrieve the heirloom and get out" is the second instance of the shape,
  and second instances are when generalizing becomes legal. We promote the
  run-goal to a component; we do not invent a quest engine.
- **Quest archetypes decompose into seams already named**: retrieval = item
  possession + gates; rescue = escortable behavior-driven NPC; delivery =
  places + travel (world-graph noodle, §7).

Ruled (Kirk 2026-08-30): **the first real quest is single-run.** "Retrieve
the heirloom from the tomb and make it out" lives entirely inside the
existing run — no new persistence, no world map. It forces exactly two new
things: the objective component and item possession. The village/return half
stays authored flavor at run end. Cross-run quests (accept in the village,
return for the reward) are the explicitly-named second rung, gated on the
journal noun being earned.

## 10. The journal and quest trees (amended 2026-08-30, second dream round)

Kirk's ask: new systems — quest trees, persistent changes based on player
outcomes. NPCs are FadedPez's feature; not dreamed here.

**Ruling: we do not architect a quest tree. We architect a journal, and the
tree emerges from predicates over it.** The failure mode is the authored
node-and-edge tree plus `savedVillage=true` flag soup. Instead a quest's
*availability* is a predicate over durable facts — exactly as an objective is
a predicate over run events and a gate is a predicate over party capability.
Author "the chief's revenge requires fact: goblin-chief-spared" and trees,
DAGs, exclusive branches, and converging paths all emerge with no tree stored
anywhere. The tree is a visualization of the predicate graph, not an engine
noun.

One predicate concept, three timescales:

| shape        | asks                          | horizon      |
|--------------|-------------------------------|--------------|
| gate         | can you pass, right now?      | the moment   |
| objective    | did it happen?                | the run      |
| availability | what is true of the world?    | the campaign |

**The journal is the existing law at a bigger clock.** Everything flows down
the combat log; the log is the run's memory. The journal is the log that
never gets thrown away: append-only, *attributed* facts — who did what, in
which run. "The village is burned" is never a stored boolean; it is derived
by folding facts ("bandits razed the village, run 7" … "village rebuilt, run
11"). Immutable history, derived present — event sourcing, which is what the
bus + log + projection stack already is. Zero values tell the truth: absence
of a fact means it never happened. Journal writes ride the game-context law:
writes are request events.

**The soul of it: no outcome is invalid — every resolution writes history and
the world proceeds.** Failing a quest is not a dead end; it is a fact the
world reacts to (the prisoner not rescued becomes the bandit lieutenant). A
real DM never reloads the save; an append-only journal means the architecture
*cannot* — persistent consequence by construction, not by discipline.

Downstream reuse, no new machinery: content reacting to facts is the gate
again (authored variants gated on journal predicates); the rung-4 improviser
reads the same journal — it is exactly what the DM seat needs to know the
campaign; fact identifiers want the typed-ref vocabulary.

**Ruled (Kirk 2026-08-30): the journal is campaign-scoped — one per
persistent party.** Each table's world diverges and lives with its own
outcomes. Facts are party-attributed from birth so a shared or overlapping
world remains a possible future *fold*, not a rewrite — the shared-world
shelf (§7), same move as gates-plural-from-birth: schema anticipates, content
doesn't.

## 11. Guild worlds and the streamer's seat (2026-08-30 dream session)

**The Discord server is the world**: guild = tenant = world = billing unit —
one noun, four jobs. Seats are the product: a cap and a concurrent limit per
guild. This re-scopes §10 same-day: ONE guild journal, facts attributed to
party and players — the party-attribution shelf paid off immediately (a
re-scoping, not a rewrite).

**The streamer is the first occupant of the DM seat.** The seat designed for
an AI gets a paying human first: set goals, announce, grant unlocks — all
verbs. The containment law bounds streamer chaos exactly as it bounds model
hallucination, and streamers road-test the seat's verb set for months before
a model inherits it. Full tenancy ladder, ascending intelligence: **dice
tables → streamer → local model** — dice are the seat's floor occupant (when
the world needs a small decision nobody authored, roll on a table) and ship
free in v1.

**World goals** are quest machinery at guild scope: predicates over guild
journal facts, many parties contributing, folded — plus a *wall-clock*
deadline ("take the stronghold before the weekend" → bonus-stream unlock).
This validates §7's world-clock postponement: the world advances by
authority (the seat rules time passed) and by player action, never by
simulation. Precedent: WoW's Ahn'Qiraj gate — community goal, opening event
as spectacle; unlock moments are stream content by construction.

**The town** is a shared hub feeding instanced runs — parties form, claim,
dispatch, return; outcomes flow to the guild journal. Discord itself supplies
voice/presence/community, so the town is a lobby with bodies and a quest
board, not a second simulation. Rescued NPCs populate it with **rolled
dispositions** (some become guards, some try to repay, some carry word of an
alternate quest — the rule of the dice): successes seed content exactly as
failures do.

**Population sizing: the streamer sets it; our tools advise.** What a good
content ratio for a thousand seats even is, is our homework — encoded as
defaults and guidance in the seat's tooling, never as automation. Authority
in the seat, competence in the tools.

## 12. Populations, not trees (ruled)

Quests come **off the board**: a claim mints an instance — your party's
hostage is *your* hostage (same template, instance identity; naming the
instances is the AI's cheapest, lowest-stakes first job — a list suffices
when the model is absent). The world holds a **population** of such
individuals, and world-level change is a **fold over the population**: when
every hostage has turned and none remain to save, the template exhausts and
its successor activates ("turn them back, or take them out").

The shape is ecological, not tree-like: populations of entities; a quest is
a state transition one individual makes (captive → rescued / turned; turned
→ redeemed / dead); new quests **activate on distribution predicates**.
Nobody authors a tree — the community's aggregate performance walks the
world through states no one scripted. Failure at scale manufactures act two:
the turned hostages are its antagonist roster. Collisions between parallel
parties dissolve: instances own their individuals; the shared world moves
only by fold.

## 13. Routes: nothing is gated, everything is a check (ruled)

**No route has prerequisites.** Everyone can attempt anything — the
barbarian in the goblin costume is a legitimate play; proficiency and
expertise tilt the dice, and the dice decide. "Gate" survives only for
passive detection thresholds (§3): *attempts are open; noticing is earned.*

Playstyle expression is intrinsic content (Kirk's correction recorded: even
outcome-identical routes differ — fighting, sneaking, performing are the
point). Routes still adjust outcomes modestly — a route's mechanical output
is the encounter's initial state (alerted / surprised / unsuspecting), and
notably **attitude**: a disguised approach can change how the camp sees the
players. Routes also **open verbs**: in disguise you can work on the minds
of the camp's leaders — decisions unavailable on other routes. Depth is the
DM's choice, not a system ceiling: we build composable tools that go as deep
as the DM wants.

Stealth and disguise are the §3 detection seam *flipped*: monster perception
vs player stealth (proximity), monster insight vs player deception
(interaction). One bidirectional mechanism; the monster side lands in the
monster-ai lane's behavior reads.

## 14. The kernel: relationships and audience-scoped facts (ratified)

Kirk: relationships are central. Authors declare **structure and
derivation, never methods**:

- **Relationships as first-class data** — typed edges (*leads, belongs-to,
  hostile-to*) and **roles as slots** ("leader of the camp" is a position an
  entity occupies). Edges change only via facts.
- **Derivations** — declared folds ("this camp's allegiance follows its
  leader's"; attitude thresholds).
- **Objectives as predicates over derived state, indifferent to method** —
  "camp X no longer hostile" is reached by assault, infiltration, leader
  replacement, or conversion; a flipped camp then *fights for you* free of
  charge, because ally behavior is behavior reading the same fold.

The changeling and the diplomatic flip both emerge from generic verbs ×
declared structure × dice, with zero path-specific code — the immersive-sim
method (Hitman, Deus Ex), made persistent, multiplayer, and DM-seated.

**Belief is the §3 ruling generalized: every entity folds over the facts it
witnessed. Knowledge is the audience of facts.** Stealth = controlling the
audience of your own events (the quiet kill is a kill fact with an empty
goblin-audience — the camp behaves as if the chief lives because the fact
never reached their fold). Disguise = planting an unbacked fact in their
feed ("this is your leader"). Seeing through it = the reveal reaching one
guard's audience and no one else's. Audiences default to **group grain**
(the camp witnesses as a unit); individual grain only when drama demands.
No belief database anywhere.

The index card: **entities · relationship edges and slots · audience-scoped
facts · folds · predicates · a handful of verbs · the seat.**

**The elegance wager, named:** Larian (DOS2, BG3) beat this problem with
four hundred artisans exhaustively authoring reactions. That road is closed
to us and we do not take it. Our bet: *derive* reactions from folds, let
real humans in the world generate drama, and rent runtime intelligence —
dice, streamer, model — for the long tail. A tabletop module is thirty pages
where BG3 is a hundred gigabytes, because a DM at the table fills the gaps;
we are building the table, not the movie.

## 15. Verification: the bandit camp use case (Kirk 2026-08-30)

Excitement tempered on purpose: the unknown unknowns are invisible, and
ideas get verified by building against something tangible. **The bandit camp
with many ways in and many outcomes is the use case** — it exercises every
kernel primitive at once. Written as `use-cases.md` beside this document.

Build shape: **prototype fashion — a new toolkit module as a spike/holding
place that can evolve into something real.** Rulebook-free by the refs test;
dice injected (capabilities supplied, never defaulted); outside the frozen
dnd5e tag namespaces. The lasting artifact is the use case itself as
executable tests — the camp's paths as data plus generic verbs, asserted
end-to-end — so the implementation underneath stays disposable while the
spec accumulates. Slice filing waits for Kirk's adoption.

## 16. The world module (named and ruled 2026-08-30)

Kirk ratified the name: **`world`** — one toolkit module, home of three
internal packages in strict one-way layering:

```
journal  <-  graph  <-  quest
(memory)     (structure     (goals)
              + present)
```

- `world/journal` — append-only, attributed, audience-scoped facts. Depends
  on nothing. Defines the base vocabulary (fact, audience, attribution),
  which flows UP the stack — no shared kernel exists beside the layering.
- `world/graph` — entities, typed edges, slots, declared derivations that
  fold journal facts into present state. Never stores what it can derive.
- `world/quest` — templates, instances, claims, lifecycle, predicates over
  derived state (including distribution predicates and guild-scope goals).
  Watches; never acts.

**Rulings:**

- **The root package stays empty** (doc comment only). `world` is a home and
  a shipping unit — one go.mod, one tag stream while shapes are provisional —
  never a composer. Rulebooks import `world/journal|graph|quest`, never
  `world`. Anything that asks to live at the root must answer "which of the
  three owns you?" — it always has an answer. The `world.World` facade is a
  named empty shelf, earned only if UC-1 shows call sites wiring the triple
  by hand.
- **Dissolution clause (falsifiable):** if the three packages ever graduate
  to their own modules, `world` dissolves without residue — an empty shell
  must not exist. The names survive (`world/graph` becomes `graph`).
- **Dependency law (mechanical):** generic tools never import rulebooks;
  rulebooks compose tools. `world` passes the refs test, so the arrow is
  `rulebooks/* → world`, forever. The game's composer remains the rulebook
  and session layer; `world` supplies parts, never assembles the machine.
- **A rulebook builds its world through exactly three touchpoints — declare,
  inject, subscribe:** declare content in world's terms (camps, slots,
  derivations, populations — data, not code); inject resolution at the one
  resolver seam ("resolve this attempt" — where the d20 enters; world never
  learns what got rolled); subscribe to emissions (`QuestCompleted` → the
  rulebook decides it means 300 XP). A different rulebook is different
  declarations and a different resolver; the machinery doesn't change.
- Start as ONE module with three internal packages, not three modules:
  package boundaries are free to redraw, module boundaries mint tag
  ceremony. UC-1 votes on the seams before any split.

## 17. Settlement: real packages, practical example (2026-08-30, closes the naming)

Kirk's mechanical test, adopted: **real = the rulebook imports it.**

- `world/journal`, `world/graph`, `world/quest` pass — three real packages.
  `world` itself is a module path: a namespace and a tag stream, imported by
  no one (§16's empty root, restated in the test's terms — nothing changed).
- **The toy-rulebook idea is retracted** (Kirk: "I do not want to prove the
  generic case if it doesn't fit with our current"). Genericity is a review
  discipline — the refs test on every diff — not a deliverable. Warhammer /
  Across the Obelisk remain the charter's thought experiment, never code.
- **`examples/world` is a separate package by mechanical necessity, not
  preference**: the practical example imports both `world/*` and `dnd5e` to
  wire declare/inject/subscribe for real. Inside the `world` module those
  imports would land a rulebook dependency in world's go.mod — violating the
  §16 arrow in writing. So the example lives above both, and go.mod itself
  tells the truth about who depends on whom.
- `examples/world` is three artifacts in one: UC-1's executable spec (the
  bandit camp, five paths, real dnd5e checks resolving them — every gap the
  wiring exposes is a finding), the practical tutorial for the next rulebook
  author, and the only place tools and rulebook meet before adoption.
- `world`'s internal tests use ordinary stub resolvers — test doubles, unit
  hygiene, not genericity sneaking back in.
- Guard until Kirk's graduation walk: no module other than `examples/world`
  imports `world/*` (one CI check; parading prevented mechanically).

## 18. Correction: the temp home is examples (Kirk 2026-08-30, supersedes §17's placement)

Kirk's call, and he was right three rounds earlier than the record shows:
**the whole spike — kernel packages AND the bandit camp — starts under
`examples/world` as one module.** The reasoning that wanted "one module while
seams are provisional" (§16) is satisfied *more* strongly there: seams
maximally fluid, no tag stream minting promises, and location is the import
guard — example code is unadoptable by construction, no CI rule needed.

- Inside the temp home: `journal`, `graph`, `quest` packages stay
  import-clean of dnd5e (review-enforced; no go.mod boundary exists yet).
  Only the bandit-camp wiring touches the rulebook.
- **Graduation**: seams proven by UC-1 + Kirk's walk → the three packages
  move out to `world/journal|graph|quest`, and the go.mod dependency arrow
  is established in writing at that moment. All §16 final-form rulings
  (empty root, dissolution clause, declare/inject/subscribe) apply at
  graduation, unchanged. §17's "real = the rulebook imports it" test also
  stands — it is the graduation criterion.
- Process note, recorded at Kirk's call-out: his `examples/` suggestion was
  compatible with the stated goals from the first round; the intervening
  counter-structures (x/world, world-as-real-module) defended a position
  rather than checking it. The record keeps them as rejected options.

## 19. Build start (Kirk 2026-08-30)

- **Graduation gains its real test (Kirk):** when the components break out to
  `world/*`, the example gets **rewired to import them exactly as a rulebook
  would** — declare/inject/subscribe across real module boundaries, proven
  from the consumer side, not asserted.
- Process ruled light: example code only — the design lives in the slice
  issue(s), no design.md ceremony. This brainstorm + use-cases.md remain the
  source of truth; the issue distills.
- Delegation: Opus agents implement; Fable holds design dialogue and
  verification gates.

## 20. After the spike: the composer returns, content is packages (2026-08-30)

**Spike verdict absorbed** (rpg-toolkit PR #1326, findings F1–F18 on its
body): journal↔graph carved true — the audience-scoped fact IS the knowledge
model; the blown disguise cost zero new logic. quest is real but thin. The
missing joint is the acting verb (F7): the kernel defined a Resolver nothing
kernel-side called, and the rulebook-free Executor lived homeless in the
example.

**The composer amendment (Kirk's original instinct, vindicated by the
code):** `world`'s root is not empty — it is the composer, and it is SMALL:
assembly plus the one write door. `Config{Graph, Verbs, Quests, Resolver}`
declares and injects; `Act()` runs the loop the spike hand-rolled (verb →
resolver if contested → branch → subject + audience → append → quests
observe); `View(observer)` is the read door. Verb/Act/Emission promote from
banditcamp (already proven rulebook-free by test); Resolver/Attempt move
from journal to the root; Outcome stays journal vocabulary (facts carry
it). Under §17's own realness test the composer root is what a rulebook
imports — `world` becomes real by the test that ruled it empty. §16's
empty-root line is amended, its god-object fear answered by size: the
composer owns the act loop and nothing else. Status: sketched and grounded
in the branch; ratification at Kirk's walk of #1326.

**Proving against real rulebook components — the resolver is a ratchet.**
Already real: sheets, proficiency/expertise, real d20s, a bus the resolver
refuses to run without. Rungs, each swapping into the same seam while the
five UC-1 tests stay untouched: (1) opposed checks assembled from existing
passives (F1, post-freeze) — the static DC 13 becomes "contested by the
watchers"; (2) "disguised" becomes a real dnd5e condition subscribing to
AbilityCheckChain (F4 — its first production subscriber); (3) monsters get
Skills so the camp rolls back (F5). Proof deepens in the rulebook; the spec
holds still. Plus §19's graduation rewiring test. `dice.Scripted` upstream
is freeze-free (dice is not a frozen module).

**Content is packages (ruled):** scenarios are Go modules like banditcamp —
~90% declaration literals, hard bits welcome — compile-checked, shipping
with their own tests (UC-1 is the scenario AND its proof). No data schema is
invented ahead of need (the world-clock lesson): write the next scenarios as
packages, let the recurring shape reveal the schema, extract it the day a
non-engineer author needs it. The boundary law: **the author decides the
format** — engineers get packages; the streamer's seat gets data (goals,
dials, deadlines) because its tooling is a console, not a compiler.

## 21. F7 settled by splitting it (Kirk + probe, 2026-08-30)

F7 was two questions wearing one name, split at ownership:

- **The act loop** (lookup → resolve → branch → subject/audience → append)
  belongs to the **generic world composer** (§20). Evidence: the spike's
  Executor contains zero camp knowledge (imports journal only, test-pinned);
  scenario-owned loops would fork audience discipline per scenario.
- **The verb declarations** (approach, difficulty, emissions) belong to the
  **scenario** — meaning is content; `banditcamp.Verbs()` already reads
  right. The composer runs verbs it never defined; the camp defines verbs
  it never runs.
- **The scenario contract** — what a content package hands the composer
  (graph declarations + verbs + quest templates; never the resolver) — is
  the third thing Kirk's probe surfaced. It exists implicitly as the
  declarative part of world.Config. Formalizing it (named interface vs
  convention) waits for **UC-2, the hostage camp, per the second-instance
  law**: the recurring shape extracts the contract; we don't invent it.

Status: proposed settlement; ratification at Kirk's walk of #1326.

### §21 addendum (Kirk, same day): scenarios end at the builder; hold here

"Scenario" is one noun in two eras: today a Go content package (engineers
author); eventually a dungeon-builder artifact — the builder's existing
geometry half plus a grown living-world half (entities, slots, verbs, quest
goals). The package era discovers the field list the builder's UI will
present; when the builder is the author, scenarios are data by construction
(the author-decides-the-format law closing its own loop). Forward note
only — no builder work now. Kirk: hold at this footing, nothing further
until stable. dice.Scripted upstream DECLINED — the test-file roller is
sufficient; do not re-pitch.

### §21 addendum 2 (Kirk): the package is the checklist

Refinement of the two-era scenario: the Go package defines the STRUCTURE of
a bandit camp — you must have a leader, you must place them on the map, you
must declare this and that — and the constructor enforces it: fail closed at
New(), nothing defaulted (the spike's ErrNoJournal/ErrNoBus/ErrIncompleteVerb
pattern, extended from wiring to content). The required-config struct IS the
future builder's form; the constructor's refusals ARE its inline guidance
("this camp needs a leader placed"). Short-run Go structure = authoring the
builder's checklist in the one medium that can also prove the checklist
right. Writing rule: constructor error messages are written for the future
form-filler, not the Go debugger — builder UX on layaway.

### Ratification note (2026-08-30): Kirk merged rpg-toolkit#1326 — the walk
verdict. §20 (composer, small root) and §21 (loop/verbs/contract split) are
RATIFIED. Next: UC-2 per use-cases.md; the composer and scenario contract
extract inside UC-2 from real duplication.

### Ratification note 2 (2026-08-30): Kirk merged rpg-toolkit#1328. Ratified
with it: the composer as built, the three evidence-forced promotions
(dnd5eresolver, scripted, margin-banded verbs), the scenario contract as a
STRUCT (resolver absent by type), and quest as a REAL package (the Tally
argument). First run of the rewiring test PASSED. F20 (two precedence lists
must agree, unchecked) and F26 (sight seam — now twice worked around,
second-instance legal) carry forward. Next: UC-3 per use-cases.md; sight
seam reserved for a design round with Kirk.
