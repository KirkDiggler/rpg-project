# Active handoff — 2026-07-16: Dungeon wave 1 landed everywhere; frontier is toolkit-seam adoption (api#650), then wave 2

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.
> Context: the 07-13/15 session died unrecoverably mid-work; this rewrite reconciles
> the stale 07-12 handoff against board #19 + a full repo-state sweep (2026-07-16).

## Now
**The Dungeon wave 1 is merged across all four code repos.** Toolkit: walled rooms +
wall-aware LOS/movement + inline combat entry (toolkit#757 via PR#759, 07-13). API:
walls on the wire + goblins seeded at StartEncounter (rpg-api#644 via PR#645, 07-16).
Web: walls threaded to the map (rpg-dnd5e-web#451 via PR#452) plus render fixes
(wall-cell PR#455, movement-border/camera PR#459). The night of 07-15 also landed a
toolkit event-spine push: EntityAppeared/Disappeared on moves (PR#762) and on
AddMonster (PR#768), room-aware spawn (PR#770), **InitiativeRolledEvent published by
the toolkit** (PR#771), ExitCombat wired into checkEncounterEnd (PR#775); protos
deprecated the dead StreamEncounterRequest fields (protos PR#179).

**In flight right now**: rpg-api#650 (https://github.com/KirkDiggler/rpg-api/issues/650)
— adopt the toolkit roster event + room-aware spawn in rpg-api (deletes the
api-synthesized InitiativeRolled + retry). The dead session left coherent uncommitted
WIP in worktree `adopt-toolkit-seams-650` (translate.go/handler.go); an implementer
was dispatched 07-16 to checkpoint-commit, finish, and PR it.

## Solid (verified — keep, don't re-derive)
- Everything in the 07-12 handoff's Solid list still stands (playtest-before-merge
  catches what CI can't; combat entry via devseed injection is dev tooling — wave 1's
  inline combat entry is the real trigger's foundation; MCP input can't reach the r3f
  raycaster, playtests drive `onEntityClick`).
- **rpg-api#637 closed "working as designed"** (07-16): the empty-economy-after-restart
  symptom was the intentional `viewerControlsActiveActor` audience gate; the only real
  bug was UI text → spun out as rpg-dnd5e-web#458.
- **rpg-api#636 closed** (07-12, fixed by PR#638): NPC turns now driven at combat entry.
- rpg-api `make pre-commit`/`ci-check` still fail on clean main (73 lint issues,
  rpg-api#631) — lint-scope to touched packages; never `--no-verify`.

## Process (carried from the 2026-07-11 retro; unchanged)
1. **Playtest-before-merge** on any PR with observable behavior; evidence on the PR.
2. **Copilot respected — and verify it actually reviewed** (oversized PRs get silently
   skipped); rpg-project + rpg-api-protos get a review-agent gate instead.
3. Every review gate carries a **doc-drift axis**.
4. **No multi-line bash / heredocs in any agent work**; Write/Edit for files.

## Open questions (verify before acting; not findings)
- **rpg-api#647** grew a second manifestation (non-atomic two-write persist can lose a
  Sequence bump on abrupt restart) — real bug, no fix landed; needs a seam decision,
  candidate for the next gap-closer.
- Fresh, uncommented tail-end findings from the dead session: rpg-toolkit#772/#773/#774
  and rpg-api#646/#648 — triage before wave 2 scoping.
- **~30 stale worktrees** across the repos; a sweep content-diffed most as fully merged
  (safe to prune) but a handful need a human look (toolkit `adr-0034-amend`,
  `adr-encounter`, `combat-gate-hydration`; web `fix-sheet-features`). Awaiting Kirk.
- **Orphaned protos commit** (worktree `lobby-service`, 07-06): "drop spoofable
  player_id from StreamLobbyRequest" — a security-flavored breaking change never PR'd.
  Deliberate abandon or lost work? Kirk to call (proto policy default is
  deprecate-don't-delete).

## Next (driven by board #19)
1. **rpg-api#650** (in flight) — toolkit-seam adoption; playtest gate: initiative
   roster reaches the client via the toolkit event on combat entry.
2. **Dungeon wave 2** (rpg-api#648, unstarted) — scope against
   `ideas/the-dungeon/design.md` (rpg-project PR#87): real multi-room run, locked-door
   gating, per-room monster seeding.
3. **rpg-api#649** (reconnect-fidelity umbrella) — dead stream fields deprecated
   (protos PR#179 merged); real resume-by-sequence was deferred pending the roster
   event, which #650 now adopts.
4. **rpg-api#647** — event-then-load race + two-write persist; needs its seam decision.
5. Toolkit follow-ups: #763 (LOS asymmetry), #766 (ally reaction in FREE_ROAM),
   #772/#773/#774 (untriaged); web#444 (resume), web#458 (UI text).

## Decision log (this stretch — full ledgers on the issues/PRs)
1. 2026-07-16: dead session's uncommitted #650 WIP rescued (checkpoint-commit first),
   implementer dispatched to finish — visible on the issue/PR + board #19.
2. 2026-07-16: rpg-project PR#88 (the dead session's restart-point handoff, written
   against 07-13 state) superseded by this rewrite — closed with rationale.
3. 2026-07-16: board #19 statuses reconciled to issue reality (late-night 07-15 merges
   had left Done work showing Todo) — board-sync pass, logged on the board.

## Pointers
- Board **#19** (The Dungeon Run) — https://github.com/users/KirkDiggler/projects/19 —
  single work queue, reconciled 2026-07-16.
- `ideas/the-dungeon/design.md` (PR#87) — The Dungeon leg design;
  `ideas/game-screen-rebuild/` — complete, kept for reference (old-vs-new.md gap table
  still maps HUD/movement/resume remainders).
- Wave records: toolkit PR#759 (walls core), rpg-api PR#645 (wire), web PR#452/#455/#459.
- Deferred shelf: rpg-api#616 (v2 layering), #631 (lint), #646 (mock-regen hazard),
  toolkit#736/#740/#754/#760-note, web#444 (resume), ADR-0034 (spatial fold-in).
- Dev stack: api `AUTH_DEV_MODE=true go run ./cmd/server server`, web `npm run dev`
  (port 3001), chrome `scripts/rpg-chrome.sh`, cast `go run ./cmd/devseed`
  (+ `--fixture=wave-2-beat2`), goblin via `--inject-combat`.
- Roles: `docs/teams/roles/<role>/{prompt,field-notes}.md`.
