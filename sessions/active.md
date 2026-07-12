# Active handoff — 2026-07-12: the game-screen rebuild is COMPLETE; frontier is The Dungeon leg

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.

## Now
**The game-screen rebuild (`ideas/game-screen-rebuild/design.md`) is complete — all four
slices shipped and playtest-verified.** Since 2026-07-06: LobbyService v1alpha1
(protos#177, api#630/#633), GameView = LobbyFlow + EncounterView on the proven stack
(web#441, #443), a real winnable fight (api#635 + toolkit#751/v0.24.4 — NPC turns, real
dice, `EncounterEnded: all_hostiles_defeated` live), fight-feel (web#446 — combat log
narrating server events verbatim + initiative tracker), and the clean slate (web#448,
−17.4k lines: LobbyView + v1 path deleted, every `*V2`/`*2` suffix renamed away). The
real game route is Home → Play → LobbyFlow → EncounterView; `/playtest` is the permanent
verification surface sharing the same hooks; `/concepts` is the separate mockup lab.

**In flight right now**: rpg-api#636 (NPC-first initiative stalls the encounter — an
implementer is building the server-side NPC-turn kick, design fork framed in the issue).

## Solid (verified — keep, don't re-derive)
- **Playtest-before-merge catches what nothing else does**: six real bugs this weekend
  invisible to CI + review — web#442 (stream abort never reconnects), api#632 (SightRange
  omitted — diagnosis CORRECTED from "missing rooms": no room/space concept exists
  anywhere on the encounter stack, visibility is a pure radius stub, ADR-0034 defers the
  spatial fold-in), api#636 (NPC-first stall), api#637 (inject-seeded first actor's
  economy empty after restart — deterministic, 2-for-2), web#444 (no resume path after
  refresh), toolkit#752 (**raging condition leaks across encounters** via persisted
  character data — bonus applies with no visible status; found because the combat log
  itemizes modifier refs).
- **Combat entry today is dev tooling**: `devseed --inject-combat --encounter-id=<id>`
  adds a goblin + flips TURN_BASED by writing Redis out-of-process; clients pick it up
  via reconnect (restart the api or wait). The REAL combat-entry trigger is Dungeon-leg
  design work; the lobby contract deliberately has no `initial_mode`.
- **Rage uses persist across encounters correctly** (charge spend, no long rest = no
  rage) — the leak in #752 is the *condition*, not the charges.
- **tools/spawn room placement is a dead stub** (`getRoomFromSpatial` always errors)
  despite "complete" status docs; doors already flow `revealed_walls` — both feed the
  walled-room design (The Dungeon leg, with ADR-0034).
- **MCP-input limitation, not a bug**: synthetic canvas events can't reach the r3f
  raycaster — playtests drive map-targeting via the component's `onEntityClick` handler;
  a human mouse click on a goblin is still owed as confirmation of the real path.
- rpg-api `make pre-commit`/`ci-check` fail on clean main (73 lint issues, rpg-api#631,
  Shelf) — lint-scope to touched packages; never `--no-verify`.

## Process (locked at the 2026-07-11 retro + this weekend's additions)
1. **Playtest-before-merge** on any PR with observable behavior — the orchestrator
   drives MCP playtests on the branch; evidence goes on the PR.
2. **Copilot respected** — and **verify it actually reviewed**: oversized PRs (e.g. the
   −17.4k slice-3) get silently skipped; a skipped review is NOT clean — gate those like
   the no-Copilot repos (rpg-project, rpg-api-protos).
3. **Every review gate carries a doc-drift axis.**
4. **No multi-line bash / heredocs in any agent work** (unallowlistable prompts hang
   Kirk); Write/Edit for files. Project allowlist (`.claude/settings.json`) covers the
   read-only set + `git rm`/`git mv`.

## Open questions (verify before acting; not findings)
- api#637's root cause — the #636 implementer was asked to observe whether the same
  load seam explains it; check their PR findings before starting #637 separately.
- Whether #636's fix lands as stream-subscribe kick (single-flight) or another seam —
  implementer investigating; my lean is recorded in the issue.

## Next (driven by board #19)
1. **rpg-api#636** (in flight) — then injected fights are reliable end-to-end.
2. **The Dungeon leg proper**: real combat entry (replaces injection), walled rooms
   (toolkit ADR-0034 + environments.QuickRoom bridge — budget the tools/spawn stub),
   monster seeding, multi-room trailblazer. `old-vs-new.md`'s gap table rows 1b–3 route
   all of it.
3. **Game Screen remainder**: HUD (gap row 4: modifiers/equipment/features), in-map
   movement on GameView (the Move button is a stub), resume (web#444).
4. **Class Kits**: toolkit#752 rage leak; the four L1 verify stories remain open.
5. Naming straggler (out of rebuild scope): `AbilityScoresSectionV2` in character
   creation still carries a suffix.

## Decision log (this stretch — full ledgers on the issues/PRs)
1. Combat entry via dev tooling, zero contract change (2026-07-11, api#634) — real
   trigger is Dungeon-leg design.
2. Toolkit gate fix over fake snapshot fields (toolkit#751): `isPlayerCombatant`
   honors hydration; no theater math in rpg-api.
3. Slice-3 survivors settled by evidence (web#448 body) — shared-looking lobby
   components were v1-only and died; character-service v1alpha1 usage stays (never in
   rebuild scope).
4. Oversized-PR Copilot skip ⇒ gate-agent rule (2026-07-12).

## Pointers
- Board **#19** (The Dungeon Run) — https://github.com/users/KirkDiggler/projects/19 —
  single work queue, reads true as of 2026-07-12.
- `ideas/game-screen-rebuild/` — design.md (complete), lobby-surface.md (contract),
  old-vs-new.md (gap table = the remaining work's map).
- Wave records: rpg-project#81 (Party Assembles retro), rpg-api#634 close-out (the
  first fight), web#448 body (deletion survivors).
- Deferred shelf: rpg-api#616 (v2 layering), rpg-api#631 (lint), rpg-toolkit#736/#740,
  ADR-0034 (walled rooms), rpg-dnd5e-web#444 (resume).
- Dev stack: api `AUTH_DEV_MODE=true go run ./cmd/server server`, web `npm run dev`
  (port 3001), chrome `scripts/rpg-chrome.sh`, cast via `go run ./cmd/devseed` (+
  `--fixture=wave-2-beat2` for charli/finn), goblin via `--inject-combat`.
- Roles: `docs/teams/roles/<role>/{prompt,field-notes}.md`.
