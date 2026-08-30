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

### Reconciliation with pre-v1 full-data-down-the-log

Pre-v1, a passive gate decides whether the **party** learns something — not
who. Full data still flows down the log; the reveal arrives *attributed*
("Standre notices a draft from the east wall"). Per-player secrecy — ranger
sees it, barbarian doesn't — is exactly what stocks the `audienceFor` shelf
(ideas/perceive) at v1.0. So passive gates ship with zero perception-limiting
infrastructure, and they are the first real customer that will eventually
justify stocking that shelf. The rulings stack instead of fighting.

Named honestly: pre-v1 the secret is only *cosmetically* hidden — the client
holds the data and declines to render it. Fine among friends; real secrecy is
audienceFor's job later.

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
- **Mechanics:** room-entry evaluation; first qualifying member reveals;
  reveal is one-way state plus one attributed event down the log.
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
