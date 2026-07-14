# Active handoff — 2026-07-14: SESSION RESTART POINT — dungeon wave 1 is ⅔ landed; resume at the api wave

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.
> **This handoff is a deliberate restart point**: Kirk is restarting the directing
> session to pick up Claude Code updates. The next session resumes EXACTLY here.

## Now — resume point (do this first)
**The Dungeon wave 1 ("walk into a room, the fight starts") is two-thirds merged:**
- ✅ Toolkit (rpg-toolkit#759, merged + auto-tagged **encounter/v0.25.0**,
  **tools/spawn/v0.2.0**): encounters hold a walled space (SpaceData{Width,Height,Walls}
  snapshot-persisted, room rebuilt at LoadFromData), wall-aware LOS/movement, **inline
  combat-entry self-transition** (player↔monster sightline → TURN_BASED, mirrors
  checkEncounterEnd), spawn engine unblocked (both stubs wired). Gate-verified; two
  probed bugs fixed in-PR (wall-cell dedup; late monsters join initiative).
- ✅ Web (rpg-dnd5e-web#452, merged): walls flow snapshot/GeometryRevealed → store →
  HexGrid (ShadedHexWall). Degrades gracefully while the server sends none.
- ▶️ **API wave NOT started — this is the resume point**: rpg-api#644 (boarded,
  In Progress). The prior implementer stalled overnight with zero work product and was
  terminated; the issue body + the merged design carry the full brief. Scope: bump
  encounter v0.25.0 + tools/spawn v0.2.0 (bump rides this PR), StartEncounter creates
  the encounter WITH a room + seeds 2 goblins via the spawn engine **outside the
  party's initial line of sight** (combat must start on a Move forming sight, not at
  spawn — if the api needs a visibility answer for placement, the toolkit must provide
  the seam; do NOT duplicate LOS math in rpg-api), projector populates Space.Walls
  (project.go:189-197 — zero proto changes). Integration test per the issue.
- Then: **the wave-closing MCP playtest** on the api branch (walls render, goblins
  hidden, step around a corner → combat by rule) → merge flag → wave 1 done.

## Solid (verified — keep, don't re-derive)
- **Foundation audit (2026-07-13, full evidence on the session record)**: v2 rails are
  clean; debts #1 (live v1 stack — DELETED, rpg-api#643, −36,294 lines, lint 70→29),
  #3 (web client-side rule math — DELETED, web#450), #6 (conversion sprawl — died with
  #1) are closed. #2 (Hex/spatial duplication) partially executed by #759; #4 contract
  lies half-fixed once Space.Walls is populated (the api wave) — remaining: dead
  StreamEncounterRequest.player_id/last_seen_sequence (implement-or-deprecate; the
  lobby stream's no-client-id pattern is the backport model); #5 (dungeon-gen in
  rpg-api's components/dungeon — untouched, relocation rides multi-room later).
- **The dev demo flow** (until the api wave lands): lobby → StartEncounter →
  `devseed --inject-combat [--inject-combat-npc-first] --encounter-id=<id>` → restart
  api (clients reconnect into the fight). After the api wave: no injection needed.
- Open bug shelf, all evidence-backed: rpg-api#637 (inject-seeded first-actor economy
  empty after restart, 3 repros), rpg-toolkit#754 (snapshots carry no active
  conditions — invisible hydrated statuses), rpg-dnd5e-web#444 (no resume after
  refresh), rpg-api#641 (flaky Rogue test, v1-era, dies with a deterministic seed).
- Gate wave-2 notes on #759 review: NPC-direction event visibility still radius-only
  (leaks through walls, matters multi-room); movement paths must be dense (endpoints
  can skip walls).
- rpg-api lint debt is now **29** (was 73; rpg-api#631).

## Process (locked; all in director memory + here for the restart)
1. Playtest-before-merge on observable behavior; evidence on the PR.
2. Copilot respected; VERIFY it actually reviewed (>20k-line PRs get skipped —
   gate those with an Opus agent; the gate runs on centerpiece PRs regardless).
3. Every review gate carries a doc-drift axis.
4. No multi-line bash/heredocs in agent work (unallowlistable prompts hang Kirk);
   Write/Edit for files. Project allowlist covers read-only + git rm/mv.
5. Bumps ride the implementation PR that needs them; standalone chore bumps only for
   toolkit-only fixes with nothing api-side queued.
6. Implementer worktrees: FRESH directory names (a reused dir caused confusion);
   verify-then-delete with survivors lists on deletion waves; four-question gate
   (Goal/Pattern/Test/Pushback) on every report.

## Next (after the api wave + playtest close wave 1)
1. Dungeon wave 2 (design's list): multi-room + doors as wall-geometry links, full
   ADR-0034 consolidation, rulebooks/dnd5e/dungeon placement data as SpawnConfig,
   faction model, per-viewer wall reveal, dungeon settings.
2. Reconnect-fidelity wave: #637 + #754 + web#444 + the dead stream fields — one theme.
3. Game Screen polish: HUD (old-vs-new gap row 4), in-map movement (Move button is a
   stub — "movement lands in Beat 2" label is stale).

## Pointers
- Board **#19** — https://github.com/users/KirkDiggler/projects/19 — reads true as of
  2026-07-14: rpg-api#644 In Progress (the resume point), everything else Done/Todo
  per leg.
- `ideas/the-dungeon/design.md` (merged, forks resolved: INLINE combat-entry check;
  fixed 2-goblin wave-1 seeding) + `ideas/game-screen-rebuild/old-vs-new.md` (gap map).
- Wave records: toolkit#759 body + its gate review (the deviation record:
  SpaceData carries Width/Height); rpg-api#643 (v1 deletion); rpg-project#81 retro.
- Dev stack: api `AUTH_DEV_MODE=true go run ./cmd/server server` (50051), web
  `npm run dev` (3001), chrome `scripts/rpg-chrome.sh` (9222), cast
  `go run ./cmd/devseed` (+`--fixture=wave-2-beat2`).
- Roles: `docs/teams/roles/<role>/{prompt,field-notes}.md`.
