# Active handoff — 2026-07-05: Beat 2 closed; next session opens with brainstorming + planning

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.

## Now
**Beat 2 (mechanical effects: Dodge/Help/Hide + the effects paved road) is CLOSED**, retro'd on **rpg-project #75** 2026-07-05. Final sign-off was a 5/5 MCP playtest verify on pure `main` across all four repos. Beat 2 shipped as **14 PRs across 4 repos** (toolkit, api, web, protos). The full ledger and retro live on **#75** — read there, this doc doesn't duplicate it. Director runs thin — commission + verify. The next-steps board was built with Kirk same day — **The Dungeon Run**, board #19, https://github.com/users/KirkDiggler/projects/19 — journey-shaped (Party Assembles / Class Kits / The Dungeon / Game Screen legs + Dungeon Night capstone), built from three evidence sweeps (toolkit L1 kits, creation journey, co-op dungeon journey).

## Solid (verified — keep, don't re-derive)
- **The paved-road pattern for effects** (the reusable spine Beat 2 discovered/hardened): activation verb → `ConditionAppliedTopic` → condition registration (`loader.go`/`factory.go`) → per-tick subscriber wiring → the event pipeline table (toolkit topic → api translate → proto → web dispatch) → render. Full detail is in the **#75 retro comment** — that's the canonical writeup, not this doc.
- **Death-save arc is fully client-visible end to end** — ghost-events-to-rendered-nat-20-revival, confirmed via playtest.
- **Permissions issue root-caused and fixed at the user-settings level, 2026-07-05** — was blocking implementer dispatches; not a per-agent or per-repo problem.
- Everything is on `main` across all repos, matching the 5/5 playtest verify.
- **`rpg-toolkit#721`** — `make pre-commit` is broken on toolkit main (bc syntax error in the coverage script), boarded on #13. With the next effort being toolkit-led, implementers must run the underlying lint/test targets directly — never `--no-verify`.
- **Two stacks, one destination:** v1alpha1 owns lobby/multi-room/boss-victory (defeat detection is dead code — TPK can't trigger), v2 owns all Beat-1/2-hardened combat + Redis persistence but has **no** join/multi-room/boss. Strategic call: v2 is the chassis, v1 is a parts donor, destination is deleting v1. Toolkit's `spatial.BasicRoomOrchestrator` / `tools/spawn` / rulebooks-dungeon have zero rpg-api imports (verified).
- **Rogue creation is hard-blocked in the web UI** — ListClasses never advertises Expertise and there's no web picker for it; backend write path is proven fine by integration test.

## Open questions / calls (verify before acting; not findings)
- Retro learnings are captured as **retro material only** — Kirk's explicit call at close-out was NOT to adopt them as process mandates yet ("proposals a little early"). They feed the upcoming planning session instead.
- `rpg-project PR #72` (combat-mutable-state ONE HOME) — **merged 2026-07-05** as the parked design record; `rpg-api#596` itself remains parked/open, unchanged by Beat 2.

## Next (driven by board #19)
Work now flows from board #19 — wave 1 (quick-win fixes: Rage RAW gaps, Ki-at-L1, character-sheet features, Rogue creation unblock — issues filed, see board) then the two critical-path trailblazers (4-player lobby on v2; multi-room on v2, toolkit-led) get design phases before implementation.
- The **structural window** queued for whenever planning schedules it: the **ADR-0034 restructure** (rpg-toolkit), **`rpg-api#616`** (encounter/v2 layering — orchestration living in the handler package instead of Chapter-1 layering), and the **snapshot-vs-live dual-home family** (`rpg-toolkit#736` AC dual-homed, `rpg-toolkit#740` stale HP snapshot on the NPC-targeting path, and the `rpg-api#612`-class of bugs generally). Scheduling relative to the three groupings above is a planning-session decision, not decided here.

## Decision log (2026-07-05)
1. **Beat 2 closed at retro** on rpg-project#75 — the retro + ledger there is the record of what shipped and what was learned; this doc only points to it.
2. **Action-item adoption deferred to planning** — Kirk: "proposals a little early." Retro findings are inputs to the next planning session, not standing process.
3. **PR rpg-project#72 merged as the parked design record**, right after close-out (2026-07-05) — the issue `rpg-api#596` itself stays parked; no new decision on its disposition.

## Pointers
- Board **#19** (The Dungeon Run) — https://github.com/users/KirkDiggler/projects/19 — the current board; wave 1 issues filed, see Next.
- **rpg-project#75** — Beat 2 wave issue: full ledger (14 PRs / 4 repos) + retro comment (paved-road pattern, death-save arc, permissions fix).
- **ADR-0034** — `rpg-toolkit/docs/adr/0034-where-encounter-logic-lives.md` — queued for restructure in the structural window.
- Board **#13** (Chapter 2: Combat Verbs) — https://github.com/users/KirkDiggler/projects/13 — prior Chapter-2 board, Beat 1/2 record.
- Follow-up shelf: `rpg-api#616`, `rpg-api#596` (parked; design record merged as rpg-project#72), `rpg-toolkit#736`, `rpg-toolkit#740`, `rpg-dnd5e-web#432`.
- Roles: `docs/teams/roles/<role>/{prompt,field-notes}.md`.
