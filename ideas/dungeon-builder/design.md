# Dungeon Builder: the in-game authoring loop

**Status:** design for review
**Parent:** rpg-project#169 (this design's issue) · relates to `ideas/dungeon-authoring/` (PR #121, M1) · supersedes M4 ("in-client tuning panel," listed as a future-design placeholder in #121) · adjacent to rpg-project#132 (Dungeon visual fidelity umbrella) and rpg-project#131 (pick-your-dungeon)

## Problem

M1 dungeon authoring (rpg-toolkit#846 `dungeonspec` + rpg-api#710 content
store) made authored YAML dungeons real; the default dungeon on the deployed
dev server (Kirk's Discord) is the authored 3-room tomb. But the authoring
loop today is: edit YAML in `RPG_CONTENT_DIR` → `docker restart rpg-api` → start
a fresh encounter. It works, and it's blind — you place `[11,3]` in a text
file and find out in 3D after a restart. Coordinate traps are invisible in
text and bite every session: odd-q hex parity, the unplaceable doorRow at
`height/2`, connector columns that belong to no room. With friends the loop
is far worse: merge → deploy → restart the Discord activity → rejoin lobby.
And there is no separate "prod" to insulate that from — the deployed game IS
the deployed dev server (Kirk's Discord), so any friend-facing authoring
surface must eventually run in that same environment, not a staging copy of
it.

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
- **Grid math is server-authoritative — the Boundary Rule applies to the
  editor too.** The client never reimplements room-chain offsets, odd-q
  parity, or doorRow placement to decide what's clickable. The toolkit
  already produces exactly this server-side for the same reason the
  workbench CLI exists (`dungeonspec.WorkbenchReport`,
  `rpg-toolkit/encounter/dungeonspec/workbench.go`): compile a spec at a
  seed and describe the resulting floor plan without a client. The board
  renders that server-computed layout; it never derives one.

## rpg-api — runtime dungeon loading (the unlock)

A dev-gated authoring surface, behind an env flag (exact name for
implementers to choose, e.g. `RPG_AUTHORING=1`), off by default. Because the
deployed game is the dev server on Kirk's Discord, enabling authoring for
friends later is a one-line env change in the deployed compose — same
environment, no new architecture.

**Decision: the gate requires `RPG_CONTENT_DIR`.** Enabling the authoring
env flag without `RPG_CONTENT_DIR` also set is a construction-time failure,
not a degraded mode — the server fails fast with a clear error rather than
starting in a silently weaker state. Write-through (below) carries two
guarantees at once: the on-disk file stays the committable artifact, and an
api restart recovers state from the dir. A memory-only fallback would drop
both guarantees without telling anyone. Locally the dir is always set
already; on the deployed box it's a volume path.

- **`PutDungeon(key, yaml)`**: runs the toolkit's existing validate+compile
  (the logic rpg-api already imports); on success, updates the *running*
  orchestrator's compiled-spec registry and stores the compiled spec in the
  content store with precedence uploaded → `RPG_CONTENT_DIR` → embedded; on
  failure returns structured errors (field-path-mapped, `InvalidArgument`)
  that an editor can render inline. **What gets mutated, precisely:** the
  registry `StartEncounter` reads (`Orchestrator.contentSpecs`,
  `internal/orchestrators/lobby/dungeon_spec.go`) is built ONCE at
  construction time in `lobby.New`, from `loadContentSpecs` — an immutable
  startup snapshot today. `PutDungeon` must update that running registry
  with concurrency-safe reads (an `RWMutex`, or an atomic swap of a new
  immutable map — implementer's choice) so its effect is visible to the
  *very next* `StartEncounter`, with no restart. A `PutDungeon` whose effect
  waits for a restart defeats the entire point of this design — that
  failure mode is explicitly excluded.
- **Key rules**: `PutDungeon` rejects (`InvalidArgument`) any mismatch
  between its `key` argument and the YAML's own declared `key:` field.
  Write-through targets the *originating* file: if a file already in
  `RPG_CONTENT_DIR` declares that key, overwrite that file; otherwise create
  `<key>.yaml`. This matters because the content registry indexes files by
  declared key, not filename (`internal/content/registry.go`) — a second
  file that declares an already-used key is silently dropped, and which
  file survives depends on read/iteration order. Writing through to the
  originating file is what prevents `PutDungeon` from ever creating that
  shadow case. Separately: `key` is constrained to `[a-z0-9-]` slugs and
  rejected otherwise — this isn't just tidiness, `key` names a server-side
  file write, so it's a safety constraint on top of a naming one.
- **Write-through**: successful puts also write the YAML to
  `RPG_CONTENT_DIR` per the key rules above, so the on-disk file stays the
  committable artifact, an api restart recovers state from the dir, and on
  a future remote box a volume gives the same durability.
- **Response contract — grid math never leaves the server.** `PutDungeon`'s
  success response returns the compiled floor plan: laid-out rooms, cell
  legality, connector/door positions, entrance. A dry-run mode (a
  `validate_only` flag on `PutDungeon`, or a sibling `Preview` RPC —
  implementer's choice) returns the same compiled floor plan without
  persisting, so the board gets live per-edit feedback by asking the server,
  never by recomputing layout client-side. `dungeonspec.WorkbenchReport`
  (`rpg-toolkit/encounter/dungeonspec/workbench.go`) already does exactly
  this compile-and-describe step outside a server, so no new toolkit
  capability is needed — `PutDungeon`/`Preview` project the same data over
  the wire.
- **`ListDungeons()`**: returns keys + display names to feed a lobby
  dropdown. rpg-project#131 ("pick-your-dungeon") contemplated this as an
  optional RPC; this design graduates it to required.
- **`StartEncounter` DOES need proto/handler changes** (correcting an
  earlier assumption in this design): the orchestrator's caller → env →
  default-key precedence already exists (`StartEncounterInput.DungeonKey`,
  `internal/orchestrators/lobby/start_encounter.go`), but no proto field
  feeds it today — `StartEncounterRequest` (rpg-api-protos
  `dnd5e/api/lobby/v1alpha1/service.proto`) carries only `lobby_id`, and
  `RPG_DUNGEON_KEY` is a process-wide override, not a per-lobby choice. P2
  therefore includes: an **rpg-api-protos change** adding `dungeon_key` to
  `StartEncounterRequest`, handler plumbing in rpg-api to populate
  `StartEncounterInput.DungeonKey` from it, and a proto regen consumed by
  both rpg-api and rpg-dnd5e-web. rpg-api-protos is its own repo with its
  own release cycle and is the slowest link in this chain — naming it here
  is deliberate, not an oversight.
- The editor's play action always starts a **fresh** encounter — stale
  encounters persist in Redis across content swaps (known trap, carried
  forward unchanged).
- Error handling: with the gate off, the authoring surface is
  absent/`Unimplemented`; validation failures are `InvalidArgument` with
  structured details; unknown key on `StartEncounter` stays `NotFound` as
  today.

## rpg-dnd5e-web — the `/author` route (dev-gated)

Three panes:

- **Board**: renders the grid the server just compiled (see the response
  contract above — the board never derives layout itself) — hex parity
  visible, the doorRow and other illegal cells simply not clickable,
  connector columns shown, rooms chained left-to-right as the compiler lays
  them out. Click to place, drag to move, delete; per-placement
  `blocks_movement` / `blocks_los` flags editable **on prop placements
  only** — dungeonspec validation rejects both flags on monster placements,
  so the board gates those controls by ref type rather than letting an
  author set a value the server will reject. Count-based `obstacles:`
  entries (rolled, not placed) have no coordinates until a seed rolls them:
  the board preserves them verbatim in the YAML and lists them in an
  off-board "rolled content" panel instead of pretending they have a
  position. `place` combined with `pattern: scattered` is rejected by
  validation today, so the board doesn't offer that combination either.
- **Palette**: the refs that actually have constructors (the toolkit#847
  monster/prop palette). **Door, start, and end markers are not
  schema-expressible today** — connectors are `{From, To, Locked}`, doors
  sit at the derived doorRow, and the entrance is generator-chosen, none of
  which is an authorable coordinate. In P3 these render as **read-only
  overlays** derived from the compiled layout, not palette items an author
  places; authorable door/start/end placement is P4+ schema work, per this
  doc's own principle that the tool grows a control only when the schema
  grows a matching field. The **boss pin** (`boss.at`) is the one
  exception — it's already authorable placement today. Separately: props
  have no toolkit-side registry, so validation checks only a placed ref's
  type segment, not that the specific ref exists — a hand-typed prop ref
  can save green and silently render nothing. The practical prop list lives
  in the web client's own `propManifest.ts`
  (`src/components/hex-grid/propManifest.ts`), which is the P3 palette
  source; a shared prop registry that closes this gap is follow-up work,
  not part of this design. "Compile errors surface inline" (below)
  therefore does not cover prop-ref existence.
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
- **P2**: the rpg-api-protos `dungeon_key` field on `StartEncounterRequest`
  + handler plumbing + regen, then the lobby dungeon dropdown — delivers
  rpg-project#131 on the way. Protos is its own repo/release cycle and the
  slowest link in this phase.
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
3. P2 — rpg-api-protos `dungeon_key` field + handler plumbing + regen,
   lobby dropdown wired to `ListDungeons`, delivering rpg-project#131.
4. P3 — `/author` editor MVP in rpg-dnd5e-web.
5. Kirk walks the full loop (edit → save → lobby → play → back to editor);
   P4+ items get their own issues off observed friction and schema growth.

— asset-pipeline agent, on behalf of KirkDiggler
