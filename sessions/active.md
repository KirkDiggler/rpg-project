# Active handoff — 2026-07-05: Beat 2 closed; next session opens with brainstorming + planning

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.

## Now
**Beat 2 (mechanical effects: Dodge/Help/Hide + the effects paved road) is CLOSED**, retro'd on **rpg-project #75** 2026-07-05. Final sign-off was a 5/5 MCP playtest verify on pure `main` across all four repos. Beat 2 shipped as **14 PRs across 4 repos** (toolkit, api, web, protos). The full ledger and retro live on **#75** — read there, this doc doesn't duplicate it. Director runs thin — commission + verify.

## Solid (verified — keep, don't re-derive)
- **The paved-road pattern for effects** (the reusable spine Beat 2 discovered/hardened): activation verb → `ConditionAppliedTopic` → condition registration (`loader.go`/`factory.go`) → per-tick subscriber wiring → the event pipeline table (toolkit topic → api translate → proto → web dispatch) → render. Full detail is in the **#75 retro comment** — that's the canonical writeup, not this doc.
- **Death-save arc is fully client-visible end to end** — ghost-events-to-rendered-nat-20-revival, confirmed via playtest.
- **Permissions issue root-caused and fixed at the user-settings level, 2026-07-05** — was blocking implementer dispatches; not a per-agent or per-repo problem.
- Everything is on `main` across all repos, matching the 5/5 playtest verify.
- **`rpg-toolkit#721`** — `make pre-commit` is broken on toolkit main (bc syntax error in the coverage script), boarded on #13. With the next effort being toolkit-led, implementers must run the underlying lint/test targets directly — never `--no-verify`.

## Open questions / calls (verify before acting; not findings)
- Retro learnings are captured as **retro material only** — Kirk's explicit call at close-out was NOT to adopt them as process mandates yet ("proposals a little early"). They feed the upcoming planning session instead.
- `rpg-project PR #72` (combat-mutable-state ONE HOME) — **merged 2026-07-05** as the parked design record; `rpg-api#596` itself remains parked/open, unchanged by Beat 2.

## Next (three groupings for the next session, per Kirk's close-out direction on #75)
1. **Toolkit-led effort:** get the four playable classes — Monk, Rogue, Fighter, Barbarian — fully playable at level 1, fully playtested; Fighter is the meatiest (all the fighting styles). The work list comes from a project board Kirk and the director build together at the start of next session — each item driven end-to-end from toolkit through to MCP playtest verification.
2. **Web overhaul:** clean-slate reset on the playtest components — **`rpg-dnd5e-web#432`** is the seed — with a real design phase this time, not an incremental patch. The board gets a game-front-end section enabling piece-by-piece verifiable cutover; sessions learn from the components the playtest harness generates and adjust as they go — the real game-screen side isn't set up yet, so starting early is deliberate, to learn sooner.
3. **Board redesign** — no longer a future abstract item: it's the concrete board Kirk + director build together at the start of next session, telling implementation stories (trailblazer vs. paved-road slices, infra→unlocks relationships) instead of a flat backlog. Groupings 1 and 2 both hang off it.
- The **structural window** queued for whenever planning schedules it: the **ADR-0034 restructure** (rpg-toolkit), **`rpg-api#616`** (encounter/v2 layering — orchestration living in the handler package instead of Chapter-1 layering), and the **snapshot-vs-live dual-home family** (`rpg-toolkit#736` AC dual-homed, `rpg-toolkit#740` stale HP snapshot on the NPC-targeting path, and the `rpg-api#612`-class of bugs generally). Scheduling relative to the three groupings above is a planning-session decision, not decided here.

## Decision log (2026-07-05)
1. **Beat 2 closed at retro** on rpg-project#75 — the retro + ledger there is the record of what shipped and what was learned; this doc only points to it.
2. **Action-item adoption deferred to planning** — Kirk: "proposals a little early." Retro findings are inputs to the next planning session, not standing process.
3. **PR rpg-project#72 merged as the parked design record**, right after close-out (2026-07-05) — the issue `rpg-api#596` itself stays parked; no new decision on its disposition.

## Pointers
- **rpg-project#75** — Beat 2 wave issue: full ledger (14 PRs / 4 repos) + retro comment (paved-road pattern, death-save arc, permissions fix).
- **ADR-0034** — `rpg-toolkit/docs/adr/0034-where-encounter-logic-lives.md` — queued for restructure in the structural window.
- Board **#13** (Chapter 2: Combat Verbs) — https://github.com/users/KirkDiggler/projects/13 — due for the redesign discussed above.
- Follow-up shelf: `rpg-api#616`, `rpg-api#596` (parked; design record merged as rpg-project#72), `rpg-toolkit#736`, `rpg-toolkit#740`, `rpg-dnd5e-web#432`.
- Roles: `docs/teams/roles/<role>/{prompt,field-notes}.md`.
