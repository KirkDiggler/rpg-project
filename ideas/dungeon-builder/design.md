# Dungeon Builder: the in-game authoring loop

**Status:** design for review
**Parent:** rpg-project#169 (this design's issue) · relates to `ideas/dungeon-authoring/` (PR #121, M1) · supersedes M4 ("in-client tuning panel," listed as a future-design placeholder in #121) · adjacent to rpg-project#132 (Dungeon visual fidelity umbrella) and rpg-project#131 (pick-your-dungeon)

## Problem

M1 dungeon authoring (rpg-toolkit#846 `dungeonspec` + rpg-api#710 content
store) made authored YAML dungeons real; prod's default dungeon is the
authored 3-room tomb. But the authoring loop today is: edit YAML in
`RPG_CONTENT_DIR` → `docker restart rpg-api` → start a fresh encounter. It
works, and it's blind — you place `[11,3]` in a text file and find out in 3D
after a restart. Coordinate traps are invisible in text and bite every
session: odd-q hex parity, the unplaceable doorRow at `height/2`, connector
columns that belong to no room. With friends the loop is far worse: merge →
deploy → restart the Discord activity → rejoin lobby. And there is no
separate "prod" to insulate that from — the deployed game IS the dev server
running on Kirk's Discord, so any friend-facing authoring surface must
eventually run in that same environment, not a staging copy of it.

## Product frame — the loop is the product

This is not primarily a map editor; it's the game's design-iteration engine.
The intended cycle: play with friends → fail → group discussion ("would
health potions fix this?") → tweak the dungeon → replay → a fix that works
graduates into a mechanic (potions in lootable boxes; opening costs action
budget; looting becomes an incentive). Static maps are the design lab: good
static maps teach what procedural generation must match — the same "author
first, proceduralize what's proven" spine that shipped M1. Downstream
unlocks unblocked by faster iteration: pseudo-DM mode (one person authors —
traps here, monsters there, hiding places — while others play) and
play-derived feature requests for the AI/monster-behavior work (e.g.
"monsters need hiding decisions"), sourced from observed play instead of
speculation.

## Design principles

- **The editor is a view over dungeonspec YAML.** It reads and writes the
  real format and never invents its own. As the schema grows (facing, walls,
  loot, triggers), the tool grows a matching affordance — the tool is the
  forcing function that surfaces what the YAML can't yet express.
- **The YAML file remains the committable artifact; git stays canonical.**
  The editor is a faster way to produce and iterate on that file, not a
  replacement for it.
- **Loop latency is the ranking design constraint.** Every choice below is
  judged by how fast idea-to-replay gets.

## rpg-api — runtime dungeon loading (the unlock)

A dev-gated authoring surface, behind an env flag (exact name for
implementers to choose, e.g. `RPG_AUTHORING=1`), off by default. Because the
deployed game is the dev server on Kirk's Discord, enabling authoring for
friends later is a one-line env change in the deployed compose — same
environment, no new architecture.

- **`PutDungeon(key, yaml)`**: runs the toolkit's existing validate+compile
  (the logic rpg-api already imports); on success, stores the compiled spec
  in the content store with precedence uploaded → `RPG_CONTENT_DIR` →
  embedded; on failure returns structured errors (field-path-mapped,
  `InvalidArgument`) that an editor can render inline.
- **Write-through**: successful puts also write the YAML to
  `RPG_CONTENT_DIR`, so the on-disk file stays the committable artifact, an
  api restart recovers state from the dir, and on a future remote box a
  volume gives the same durability.
- **`ListDungeons()`**: returns keys + display names to feed a lobby
  dropdown. rpg-project#131 ("pick-your-dungeon") contemplated this as an
  optional RPC; this design graduates it to required. #131's caller → env →
  default-key precedence is already built in rpg-api#710, so `StartEncounter`
  needs no changes.
- The editor's play action always starts a **fresh** encounter — stale
  encounters persist in Redis across content swaps (known trap, carried
  forward unchanged).
- Error handling: with the gate off, the authoring surface is
  absent/`Unimplemented`; validation failures are `InvalidArgument` with
  structured details; unknown key on `StartEncounter` stays `NotFound` as
  today.

## rpg-dnd5e-web — the `/author` route (dev-gated)

Three panes:

- **Board**: renders the true grid — hex parity visible, the doorRow and
  other illegal cells simply not clickable, connector columns shown, rooms
  chained left-to-right as the compiler lays them out. Click to place, drag
  to move, delete; per-placement `blocks_movement` / `blocks_los` flags
  editable.
- **Palette**: the refs that actually have constructors (the toolkit#847
  monster/prop palette), plus door/start/end markers and the boss pin.
- **YAML pane**: live text beside the board, two-way — board edits update
  the YAML, YAML edits update the board. **Hard requirement:
  comment-preserving round-trip.** Authored files use comments as design
  notes (e.g. "colonnade: 8 pillars framing the center lane" in
  `showcase.yaml`); an editor that strips comments on save destroys the
  artifact. Use a CST-round-trip YAML parser (e.g. the `yaml` npm package by
  eemeli, which supports this).
- Save calls `PutDungeon`; compile errors surface inline, mapped onto board
  cells where possible. Play jumps to the lobby with the authored dungeon
  preselected in the dropdown; leaving the encounter returns to the editor.
  This is the core UX loop Kirk described: "go from my edit panel in the
  game to a lobby with that dungeon in the dropdown; come out, edit it, go
  back in."

(For later implementers: rpg-dnd5e-web work branches from `origin/development`,
not `main` — per rpg-project CLAUDE.md's base-branch table. Not relevant to
this doc PR itself.)

## Phasing

- **P1**: the rpg-api endpoint alone (Put + List + gate + write-through) —
  curl-testable before any UI exists.
- **P2**: the lobby dungeon dropdown — delivers rpg-project#131 on the way.
- **P3**: the `/author` editor MVP (board + palette + YAML pane + save +
  play).
- **P4+**: schema-driven growth, each step landing as toolkit schema change
  → new tool affordance: prop facing (the "reaper statue facing this way"
  case — `place` blocks have no facing field today), wall/shape authoring
  (Kirk: a good evolution, coming but not now — today walls are derived from
  room envelopes + connectors, and the schema is a linear room chain), loot
  containers (potions in boxes, opening costs action economy),
  triggers/traps, and eventually pseudo-DM mode.
- **Remote/distributed phase**: configuration, not architecture — flip the
  gate in the deployed compose (the Discord dev server); later possibly a
  beefier EC2 dev instance with a build manifest and a "dev door" on a
  Discord dev release routing to it.

## Supersedes M4

The M1 dungeon-authoring plan (rpg-project PR #121) listed "M4: in-client
tuning panel design" as a placeholder ("the in-client tuning panel is its
own future design (M4)"). This design absorbs and supersedes M4 — M4 was
never fleshed out; this is that fleshing-out. Stating this explicitly so
both don't get built.

## Testing

- **api**: unit tests around content precedence (uploaded → dir → embedded),
  write-through, and gate-off behavior; curl smoke test proves P1 before any
  UI.
- **editor**: round-trip tests — YAML → board model → YAML must be
  byte-stable including comments; placement legality (doorRow unclickable)
  unit-testable against the same layout rules the compiler uses.
- The loop itself is judged by live play (Kirk's live-walk + screenshots),
  consistent with how authored content is already verified — not by review
  machinery.

## Rollout

1. This design PR merges as the tracking surface for the arc below (stays
   open per the Cross-Repo Design Workflow, `plan.md` added after approval).
2. P1 — rpg-api `PutDungeon`/`ListDungeons` behind the authoring gate,
   write-through, curl-verified.
3. P2 — lobby dropdown wired to `ListDungeons`, delivering rpg-project#131.
4. P3 — `/author` editor MVP in rpg-dnd5e-web.
5. Kirk walks the full loop (edit → save → lobby → play → back to editor);
   P4+ items get their own issues off observed friction and schema growth.

— asset-pipeline agent, on behalf of KirkDiggler
