# Disposition — hostile until they learn better

**Status:** BRAINSTORM, opened 2026-09-04 in session with Kirk. Tool 2 under the
north star on rpg-project#326: "we are here to build tools that can be used to
tell stories." Tool 1 (the intel record) is building on branches (#374).

## The use case, in Kirk's words

> "2 seems pretty interesting and gets us in the graph. I could even see they
> are hostile until they get intel. find out it was us that saved the Wiseman."

A goblin camp is hostile to the party. The party carries — or learns, or is
handed — the fact that they saved the Wiseman. When the camp comes to know
that fact, its disposition toward the party flips, and the fight that would
have formed does not. "Making the goblin camp not hostile falls out of this."

## Why it is the same three nouns

- **Fact:** `saved:wiseman`, an audience-scoped journal fact. It reaches the
  camp as intel (tool 1): a record whose `reveals` gains a `fact` key, held,
  handed, told or witnessed.
- **Predicate:** "the camp knows `saved:wiseman`" — a fold over the camp's
  facts, the same shape as a gate, an objective, an ending.
- **Verb / effect:** the disposition edge `goblins → party` flips from hostile
  to friendly in the world graph. Nothing in behavior has to change for the
  fight not to form: fight formation already asks `IsHostile`, and `IsHostile`
  already reads the graph (rung 1, rpg-toolkit#1352). This is rung 2 — the
  stance writer — with a real writer at last.

## What exists (ground truth, no re-derivation)

- world/graph holds relations; resolution's castView reads a FIXED two-side
  table from it (rung 1). Rung 2 = replace the fixed table with a fact-reading
  one; `IsHostile`/`IsAllied` bodies never change again (integration.md).
- The run composes a world (journal + graph) per encounter; facts are
  per-member with audiences; monsters' authored intel becomes holdings (tool 1).
- hostagecamp (UC-2) proved `AdoptStance` flips a relation mid-run; the
  world module's `Concealed`/`Reveals`/`Pierces` show how a declaration and a
  fact meet.
- Two knowledge models for monsters exist today: our holdings, Billy's
  MonsterView memory. Disposition reads only the graph, so it does NOT need
  them unified — that is what makes tool 2 cheaper than tool 3 (reactions).

## The DM's tools (what the designer exposes)

1. **Factions** as a thing you can name in the file (`factions: [goblins]`)
   and assign a monster placement to (`faction: goblins`). Today "kinds" —
   character-side vs monster-side — is the whole table.
2. **A disposition** between two factions, authored (`hostile | neutral |
   friendly`), with an optional **until**: a fact that flips it
   (`hostile until: saved-wiseman`). This is the predicate, authored as data.
3. **Intel that reveals a fact** (`reveals: { fact: saved-wiseman }`), placed,
   handed, or told — the ways a faction comes to know.

## Ruled (Kirk, same session)

- **The chief is the camp's mind.** "goblins listen to their chief — we only
  need to convince the chief." A faction names one knower; the camp knows
  when he knows. A scout learning it flips nothing.
- **Two ways to convince him, both legal:** "we throw it at the chief, or be
  in the same room as the chief holding the letter." PRESENCE first — a
  holder of the letter standing in the chief's region is enough, no verb;
  THROW/hand second — a verb, later.
- **The hold-out mission** (Kirk): "a messenger with a letter shows up in n
  turns." Survive until then; get the letter to the chief; the camp turns.
  That is the CLOCK tool arriving with its own use case: something scheduled
  to appear at turn N.

## The hold-out, as the party lives it

1. The party is in the goblin camp; the camp is `hostile until:
   saved-wiseman`. A fight forms on sight. They hold out.
2. At turn N the messenger arrives at the entrance carrying the letter — a
   holdable prop whose intel record `reveals: { fact: saved-wiseman }`.
   (First cut: the letter APPEARS at the entrance at turn N and the beat
   says a messenger brought it; a walking, talking messenger is the NPC
   lane's Interact "give" capability, shelved.)
3. Somebody Holds the letter and reaches the chief's room while carrying it
   — presence — or throws it to him (later).
4. The chief knows `saved-wiseman` → the edge `goblins → party` flips to
   friendly → **the fight between those two factions dissolves**, the way
   a boss's death dissolves it today; goblins stop being targets and stop
   targeting. Whether they then trade, follow, or sulk is Billy's lane.

## Design consequences this names

- **A disposition flip mid-fight dissolves the fight between the flipped
  factions** — encounter's bubble logic, ours; the same seam a death uses.
  A fight with a third hostile faction in it keeps going for that faction.
- **Presence transfer**: "the chief knows what is carried into his room" is
  a fold over (holder's region == chief's region) ∧ (holder holds a record)
  — evaluated on every arrival the way concealment's sweep runs on every
  sight refresh; the record is COPIED to the chief (intel copies).
- **Timed arrival**: a placement (prop now, NPC later) with `arrives:
  { turn: N }` — the clock tool's first instance; the hostage's "turns until
  turned" is its second.

## Open rulings (for Kirk)

- ~~Grain~~ — ruled: the chief.
- ~~How they learn first~~ — ruled: presence, then throw.
- **Does the flip dissolve a fight already formed?** Proposed yes (above);
  confirm.
- **Where does the messenger come from?** A prop that appears at turn N (no
  new lane) vs an NPC that arrives and gives (fadedpez's Interact). Proposed
  the prop first, the NPC when the capability exists.
- **Where the flip lives.** The graph (an edge fact written when the
  predicate holds) — not on the monster, not in behavior. Behavior reads the
  graph if it wants to (Billy's lane); fight formation already does.

## Lanes

Ours: factions/disposition in the file, the predicate, the graph writer
(rung 2), fight formation honouring it. fadedpez: telling an NPC. Billy:
behavior that reads disposition (a formerly hostile goblin that now trades,
follows, or ignores) — a seam question to put on his record before any
reaction work.

## Acceptance (draft)

| item | proof |
|---|---|
| a camp authored `hostile until: saved-wiseman` forms a fight on sight while nobody in it knows the fact | scene |
| the chief receives the intel (handed) → the edge flips → walking into the camp forms no fight | scene, and the walk |
| a scout receives it → nothing flips (grain) | scene |
| the letter carried into the chief's room mid-fight flips the edge and the fight dissolves | scene, and the walk |
| the letter carried into a scout's room flips nothing | scene |
| the messenger's letter appears at the entrance at turn N and not before | scene on the clock |
| the DM authored all of it through the designer: faction, disposition, until, the intel record | screenshot + yaml round-trip |

## Round 2 (2026-09-05, in session with Kirk) — the second branch, and the law

**The scenario gained its other half.** Kirk: "take our scenario idea for the
chief getting the delivered intel to stop the attack. alternatively if the
chief is killed then reinforcements could arrive. this use case will help us
setup both the capability in the encounter system and the dungeon builder
ui." Two branches, one file:

- deliver the intel to the chief → the camp stops attacking;
- kill the chief instead → reinforcements arrive at the gate.

This answers the open "dead mind" ruling by dissolving it: the chief's death is
a CONSEQUENCE with its own arrival, not a losing condition. The camp can no
longer be turned (nobody is left to know), the party can still withdraw or
fight it out, and the author chose that trade-off on purpose.

**The predicate is the designer's noun.** Both new fields are the same shape:
`until:` on a disposition and `arrives:` on a placement each take a predicate,
and the same three forms serve both — `{ round: N }`, `{ down: <placement> }`,
`{ fact: <id> }`. The second instance arrives inside the slice, which is what
earns the grammar a place in the descriptor (the second-instance law). Grain
is the one trap: `{ fact }` on a disposition is judged against the faction's
MIND (audience grain, "does the chief know it"); on an arrival it is judged
against the world (truth grain, "was it ever learned") — arrivals are physical,
and physical state folds on truth, the 2026-08-31 two-grains ruling.

**The law as written (Kirk adopted the style; the sentence is this one):**

> The run's world is the only state. Content declares it, verbs append to it,
> readers fold over it, the projection presents it. No reader keeps a copy.

Test: no reader may build a journal or a graph, or keep a relation table, of
its own. On main today that test names four structures — the encounter's
concealment world (`conceal.go:209`, built only when the field conceals), its
holdings journal (`holdings.go:146`, facts with no audience), its monster
memory (`turndriver.go:76`, Billy's lane), and resolution's fixed two-node
table (`cast.go:211`). Spirit: a designer's line must reach the table through
one state, so that "who is hostile" and "who knows what" have one answer.
The law lands one reader per slice, each pulled by a use case that walks — the
encounter application died of being rewritten feature by feature into one
program, and a rewrite of all the readers at once is the same mistake with a
bigger surface. This slice moves two readers: sides, and knowledge. Monster
memory moves when Billy's use case pulls it, on his record.

**Ground truth the design rests on (code survey 2026-09-04/05, toolkit main
23eda87):**

- Fight formation never asks `IsHostile`. `trigger.go:353` sorts contact by
  `MemberKind`; `standing.go:306` and `clocks.go:290/386` count the same two
  kinds. There is no faction on a member and no `party` anywhere.
- `IsHostile`/`IsAllied` (`resolution/cast.go:99/121`) have two callers, both
  targeting conditions, and read `castRelations` — a `sync.OnceValue` graph
  with two `cast-side` entities folded over an empty journal.
- The import direction is encounter ← resolution ← session. Resolution's
  `Input.World` IS the whole `encounter.EncounterData` (`resolve.go`), built
  by the session at four call sites (activate, announcer, attack, striker):
  resolution already receives the run's world every time and rebuilds an
  encounter from it. The session never speaks the world module's vocabulary
  (`world` is an indirect dependency of session).
- `EncounterData.World` (`data.go:63`) already persists the journal's facts;
  the graph reseeds from the field on load. Stances are never stored.
- Runtime edge rewriting in world/graph is declarative only: a reducer raises
  a flag from facts, an `AdoptStance` projection rewrites the edge on the next
  fold (hostagecamp's pattern). `dissolveBubble(bubble, cause)` is the single
  dissolve path with a sealed cause set.
- Rounds exist only inside a fight bubble; the world clock outside counts
  feet; props are construction-truth but slice 2's drop places one at runtime
  through a fact and a projection. Monsters enter through `Spawn`, at launch,
  with content resolved in rpg-api; every fact about one is hand-carried.
- The only per-member row on the wire is the roster's `PublicMemberInfo`
  (closed `MemberKind` enum, no faction, side, or stance anywhere).
- `sweepOccupancy` inside `refreshSight` runs on every verb and at Load.

## Round 3 (2026-09-05) — reuse, and the hub

**Not new, three times.** "Disposition" was Kirk's own word in the 2026-08-30
dream sessions (rescued NPCs with dice-rolled dispositions; the hostage camp
rolled them); fadedpez's NPC module has `DispositionPolicy` (per NPC, "not
pairwise hostility", one value: neutral); and world/graph already has
`FactionOf` — the far end of the membership edge — used by its reducers and
projections. Faction is a kernel primitive being surfaced, not invented.

**The predicate is the Trigger set.** The encounter's endings are already
predicates (`field.go:793`: ReachedPosition, MemberDown, External,
ExitedHolding — sealed, liveness-validated, evaluated where each event is
noticed). `{ down: chief }` is `TriggerMemberDown` with a designer's spelling.
So `until` and `arrives` are the set's second and third consumers, `round` /
`fact` / `stance` are new Trigger types, and `endings[]` becomes authorable
with the same grammar — at which point the hold-out scenario's one field is
sugar and the package has nothing left to do. Kirk's principle, in his words:
"while we are building this use case out we want reusable primitives. if we
get that right we can make things we never thought of. so our new properties
can be tested out against other surfaces." The surface test is now a design
step: every primitive names a second surface from our own backlog (the design
§0/§9), and the one that failed — `mind`, against a hostage camp where each
captive turns alone — was resolved by Kirk keeping the hub at faction level
("factions.mind seems like an easy way for word to spread through a given
faction… at a faction level it makes sense") with the singleton default
(a faction of one has its member as mind).

**The break named:** "local goblins get the message and the mind gets it a
turn later" cannot be a stance fact — the stance is a faction-pair edge, so
the camp is hostile or it is not. It can be a KNOWLEDGE fact: members learn
now, the mind learns N turns later (a scheduled fact — the arrival primitive
applied to a fact), the edge flips when the mind knows. Shelved as "word
spreads" (design §11) with take-it-back-to-town and trade-for-the-next-quest,
both of which land on the campaign journal (integration rung 3).
