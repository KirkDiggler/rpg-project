# Active handoff — 2026-07-11: Party Assembles wave closed at retro; final weekend — ship toward a real fight on GameView

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.

## Now
**The Party Assembles wave is CLOSED at retro on rpg-project#81** (2026-07-11). What
shipped, all merged: LobbyService v1alpha1 contract (rpg-api-protos#177,
`EncounterService.CreateEncounter` deleted — `StartEncounter` is the sole construction
path), the full API vertical (rpg-api#630 + #633), and **GameView** — the real front
end's first slice — on the playtest-proven stack (rpg-dnd5e-web#441 + #443). Verified by
the bar itself: 4 browsers, 4 adventurers, mutual sight, each player their own view. The
"4 real clients in one v2 encounter" story is Done on board #19.

**Context that shapes everything: this is the final weekend (through 2026-07-13).**
Ship over scaffolding; leave every artifact successor-ready. Goal for the window:
**a real fight on the GameView route** (attack a goblin, see damage, end turns, kill it,
via MCP playtest) — combat-entry survey dispatched 2026-07-11; design bite then
implementation follow from its findings.

## Solid (verified — keep, don't re-derive)
- **There is no room/space concept anywhere on the encounter stack** — not in
  `tkenc.Encounter` (bare `core.Hex` positions), not in devseed. Visibility is a pure
  per-player `SightRange` radius (`perception.VisibleHexesAt`, a stub that ignores
  walls — none exist). The harness never had rooms; it had visibility. ADR-0034
  (accepted, unimplemented) defers the spatial consolidation. Full trace: rpg-api#632
  comments + `ideas/game-screen-rebuild/old-vs-new.md`.
- **`tools/spawn`'s room-placement entry points are dead stubs** (`getRoomFromSpatial`
  always errors) despite "complete" status docs — The Dungeon leg must budget for it.
- **Doors already flow `revealed_walls`/`removed_walls`** (`DoorData` on `tkenc.Data`) —
  partial wall plumbing to study for the walled-room design.
- **GameView and /playtest share hooks/components** — that sharing is what makes MCP
  verification proof of the game path. `/playtest` is permanent. LobbyView still exists
  but is unreachable; slice 3 (delete legacy + drop all `*V2`/`*2` suffixes) not yet run.
- **v1alpha1 surface still runs server-side** and the old UI runs from git — the
  old-vs-new comparison (`ideas/game-screen-rebuild/old-vs-new.md`) maps every old-way
  capability to a boarded leg; use its gap table, don't re-derive.
- rpg-api `make pre-commit`/`ci-check` still fail on clean main (73 lint issues) —
  rpg-api#631, Shelf. Lint-scope to touched packages; never `--no-verify`.

## Process (locked at the 2026-07-11 retro — Kirk's calls)
1. **Playtest-before-merge** on any wave PR with observable behavior — both
   playtest-found bugs this wave (web#442, api#632) were invisible to CI and review.
2. **Copilot is respected** — it caught two real bugs this wave (negative PartyCap,
   non-atomic Redis write); disposition every comment, never dismiss as noise.
3. **Every review gate carries a doc-drift axis** (docs-vs-code claims checked per PR);
   for repos without Copilot (rpg-project, rpg-api-protos) the gate is the only reviewer.
4. **No multi-line bash anywhere** (unallowlistable → permission prompts hang Kirk):
   Write/Edit for file content, single-line commands, constraint goes in every agent
   brief. Read-only allowlist lives in project `.claude/settings.json`.

## Open questions (verify before acting; not findings)
- Combat entry: how a lobby-built encounter enters TURN_BASED with a monster — survey
  in flight; the lobby contract deliberately dropped `initial_mode`, don't re-add it
  casually.
- rpg-project#83 (visibility-diagnosis doc correction) — open at handoff time.

## Next (driven by board #19)
1. **Combat entry on the new stack** (in flight) — opens The Dungeon leg.
2. **The Dungeon trailblazer** (multi-room on v2, toolkit-led): inherits ADR-0034 +
   the tools/spawn stub budget + the doors wall-plumbing precedent.
3. **Game Screen polish** once there's a fight to render: HUD, combat log, initiative
   overlay, movement-range highlight — gap rows 4–8 in `old-vs-new.md`.
4. **Slice 3** (delete LobbyView + v1 hooks, rename pass) — mechanical, any time.

## Decision log (this wave — full detail on rpg-project#81)
1. Clean slate, no shim; `/playtest` permanent; no version-suffixed names (2026-07-06).
2. Separate LobbyService at its own `lobby/v1alpha1` (service-first, own version clock).
3. `StartEncounter` subsumes `CreateEncounter` — one construction path.
4. #632 diagnosis corrected mid-wave: visibility (one field), not geometry — real rooms
   deferred to The Dungeon leg.
5. Retro action items 1–4 above (2026-07-11).

## Pointers
- Board **#19** (The Dungeon Run) — https://github.com/users/KirkDiggler/projects/19 —
  single work queue, reads true as of 2026-07-11.
- **rpg-project#81** — Party Assembles umbrella: final ledger + retro (the wave record).
- `ideas/game-screen-rebuild/` — design.md (parent), lobby-surface.md (contract incl.
  edge-case policies), old-vs-new.md (verified gap table with board routing).
- Deferred shelf: rpg-api#616 (v2 layering), rpg-api#631 (lint), rpg-toolkit#736/#740,
  ADR-0034 (walled-room prerequisite).
- Roles: `docs/teams/roles/<role>/{prompt,field-notes}.md`.
