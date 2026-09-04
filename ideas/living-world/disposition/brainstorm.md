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
