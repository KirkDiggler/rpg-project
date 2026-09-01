---
name: Concealed door — plan
status: added 2026-09-01 after the design was ruled whole; this PR stays open through implementation, gaining implementation.md as the ledger (dungeon-builder pattern)
purpose: implementation plan for slice 1 — a builder-authored concealed door, found by room-targeted search, opened to those present
---

# Concealed door — plan

*Companion to design.md in this folder. Journey: rpg-project#326.
Process: how-we-build — proto merges first, parallel builds, Kirk walks
the branch once, merge bottom-up (toolkit → api → web). One branch per
repo for the whole wave; integration findings land on that branch, not
as new PRs.*

## A simplification found in the ground truth

`dungeonspec.DoorSpec` already carries the open check: `LockSpec{DC,
Ability, Tool}`, held opaquely ("does a DEX check of 12 succeed" is a
rule the spec never interprets). The open *path* is the one the
run-ending work already built, and a concealed door composes with plain,
closed, or locked underneath. One amendment from the multi-approach
ruling: `LockSpec` is single-approach today, and a check is now a **list
of accepted approaches, each with its own DC** (Strength to force or
Dexterity-with-tools to pick; Perception or Investigation to find). So
the wave grows the find half born as an approach list, and generalizes
`LockSpec` to the same shape — an in-place break, consumers moved in the
same wave, per the proto-versioning trigger (few consumers, one
sitting).

## Board filing

- Adopt journey #326 (update its outcome text to explicit-search-first
  per the design's reconciliation note).
- One slice issue per owning repo under #326, filed at branch-cut time —
  one issue per PR, per board rules.

## Waves

### Wave 0 — protos (the contract; merges first)

- Door message grows the concealment declaration mirroring the spec:
  concealed marker + find check as a list of approaches (each: ability/
  skill + DC); the lock's open check generalizes to the same list shape
  (in-place break, consumers moved in the same wave).
- A room-targeted **search intent** (the player names a room, nothing
  else — they cannot target what they do not know exists).
- Door-reveal **detection beats addressed per recipient** —
  `Event.recipient` and broker routing already exist end to end; the
  standing per-player-detection-from-birth ruling names doors as the
  case. Expect message shapes, not plumbing.

### Wave 1 — toolkit (the engine; the wave's center of gravity)

- **dungeonspec**: `DoorSpec` gains the find check (`Concealed`), born
  as an approach list; `LockSpec` generalizes to the same shape.
  Fail-closed validation worded for the form-filler — a concealed door
  with no find approach refuses at compile.
- **The run composes its world** (ruled: the dungeon run IS the world).
  At run start, journal + graph are built with the spec's concealed
  doors as concealed graph declarations. World data rides the session's
  Input/Output as data, exactly the way `EncounterData` does — no new
  persistence machinery.
- **The search verb**, load-act-save through the session seam: sweeps
  the targeted room's concealed declarations, rolls each find check via
  the humble dnd5e resolver (ruled) against the searcher's real skills,
  writes the location fact with audience = the searcher alone. Multiple
  listed approaches resolve by the character's best (the player cannot
  be offered a choice that leaks what is hidden; see the design's
  flagged interpretation). Empty
  room and failed check resolve identically — the answer never leaks
  the question. Rules and trigger detection live in the composition
  layer per the layering laws; the session owns no rules.
- **Witness**: the session's existing sight (`session/sight.go`,
  squareSeam LOS, the shared 120ft ruling) implements the world's
  Witness capability — §22's game rung, verbatim. Opening a found door
  writes the door-opened fact witnessed by those present.
- **Projection**: a concealed, unknown door is absent from a
  non-knower's view; a knower's view carries it; the opened door is
  present state, visible to anyone who can see it (§22's two grains —
  perceiving present state is not witnessing past events).
- Kernel API changes only if adoption forces one — that is a finding to
  bring back to the design PR first.

### Wave 2 — rpg-api (the translator)

- Route the search intent by key; translate world beats verbatim;
  persist the run's world data on the run record. No rules, no
  interpretation — if this wave grows logic, the toolkit wave missed a
  requirement.

### Wave 3 — web (the proof)

- **Builder**: door declaration gains a concealed toggle and a find
  check authored as approach rows (add an approach: ability/skill +
  DC); the lock editor grows the same rows. The open half otherwise
  reuses the existing lock/closed authoring.
- **Game**: the search verb on the action surface, targeting the room
  the player stands in; concealed unknown doors absent from the scene;
  per-player reveal beats rendered (the knower sees the door appear;
  on open, everyone present sees it).

## Done-when

The design's done-when, observed through the named local dev path: a
builder-authored dungeon holds a concealed door; a searching player
finds it alone; a knower's successful open reveals it to everyone
present; a party that never searches finishes the run without ever
seeing it. Kirk walks the branch once; merge bottom-up; a wall in the
walk is a design signal, not a patch signal.
