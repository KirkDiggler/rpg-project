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

## Open rulings (for Kirk)

- **Grain.** A faction knows a fact when — one member knows it (group grain,
  the kernel's default: "belief = every entity folds over facts it
  witnessed; group grain default"), or every member, or a named leader? The
  goblin chief learning it should be enough; a lone scout learning it
  probably should not flip the camp. Proposal: the faction declares a
  **knower** (the chief); the DM's tool is a dropdown of that faction's
  monsters.
- **How they learn, first.** (a) the party HANDS the intel (the handoff
  shelf — an item transfer to an NPC); (b) the party TELLS it (a verb on a
  world NPC — fadedpez's Interact/capabilities lane); (c) the camp WITNESSES
  it (the sight seam; the Wiseman walks in with the party). Proposal: (a)
  first — it reuses holdings and Loot's transfer in reverse, and needs no new
  verb semantics; (b) belongs to the NPC lane and arrives when Interact
  grows a "say/show" capability.
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
| a fact that arrives mid-fight dissolves nothing by itself (ruled? — see Kirk) | open |
| the DM authored all of it through the designer: faction, disposition, until, the intel record | screenshot + yaml round-trip |
