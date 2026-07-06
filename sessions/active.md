# Active handoff — 2026-07-05 (end of day): Wave 1 complete; board #19 is the work queue

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.

## Now
**Wave 1 (the quick-win fixes off board #19) is COMPLETE** — all five issues fixed, merged by Kirk same day: `rpg-toolkit#745`/`#746` (PRs #747/#748), `rpg-dnd5e-web#436`/`#437` (PRs #438/#439), `rpg-api#625` (PR #626). rulebooks/dnd5e v0.65.0 tagged and delivered into rpg-api (PR #628 merged) — Rage melee-STR gate, STR advantage under Rage, and the Ki L2 gate are live in the game. Toolkit docs close-the-loop merged (#749). Board #19 (https://github.com/users/KirkDiggler/projects/19) is the single work queue and reads true.

## Solid (verified — keep, don't re-derive)
- **The paved-road pattern for effects** (the reusable spine Beat 2 discovered/hardened): activation verb → `ConditionAppliedTopic` → condition registration (`loader.go`/`factory.go`) → per-tick subscriber wiring → the event pipeline table (toolkit topic → api translate → proto → web dispatch) → render. Full detail is in the **#75 retro comment** — that's the canonical writeup, not this doc.
- **Death-save arc is fully client-visible end to end** — ghost-events-to-rendered-nat-20-revival, confirmed via playtest.
- **Permissions issue root-caused and fixed at the user-settings level, 2026-07-05** — was blocking implementer dispatches; not a per-agent or per-repo problem.
- Everything is on `main` across all repos, matching the 5/5 playtest verify.
- **`rpg-toolkit#721`** — `make pre-commit` is broken on toolkit main (bc syntax error in the coverage script), boarded on #13. With the next effort being toolkit-led, implementers must run the underlying lint/test targets directly — never `--no-verify`.
- **Two stacks, one destination:** v1alpha1 owns lobby/multi-room/boss-victory (defeat detection is dead code — TPK can't trigger), v2 owns all Beat-1/2-hardened combat + Redis persistence but has **no** join/multi-room/boss. Strategic call: v2 is the chassis, v1 is a parts donor, destination is deleting v1. Toolkit's `spatial.BasicRoomOrchestrator` / `tools/spawn` / rulebooks-dungeon have zero rpg-api imports (verified).
- **Rogue creation verified end-to-end in a real browser** (full creation flow, expertise picks, finalize) — but expertise math is invisible in play: the proto's `Proficiencies` flattens Expert to plain proficient at the wire; filed as **`rpg-api#627`** (Party Assembles / Fix / Todo on board #19), recommended shape = server-computed per-skill modifiers (`AbilityModifiers` precedent) which also removes the web's client-side skill-modifier calculation (boundary violation).
- **`DamageChainEvent` now carries `IsMelee`** (added for the Rage gate — future damage-chain modifiers rely on it instead of re-deriving).
- **No leveling path exists**: the public draft flow hardcodes level 1 (surfaced by the Ki fix — L2 behavior is unit-tested only).
- **rpg-api's `make pre-commit`/`ci-check` fail on clean main** (73 pre-existing lint issues; GitHub CI only runs `go test -race`) — same shape as `toolkit#721`, unfixed.

## Open questions / calls (verify before acting; not findings)
- Retro learnings are captured as **retro material only** — Kirk's explicit call at close-out was NOT to adopt them as process mandates yet ("proposals a little early"). They feed the upcoming planning session instead.
- `rpg-project PR #72` (combat-mutable-state ONE HOME) — **merged 2026-07-05** as the parked design record; `rpg-api#596` itself remains parked/open, unchanged by Beat 2.

## Next (driven by board #19)
Work now flows from board #19. Next session opens with either **(a)** the four class-kit Verify playtest stories on board #19 — now unblocked by the v0.65.0 bump; needs a wave-1-verify devseed fixture naming the cast — or **(b)** the design phase for one of the two critical-path trailblazers (4-player lobby on v2; multi-room on v2, toolkit-led). Kirk's call at session start. `rpg-api#627` is the remaining Party Assembles fix.
- The **structural window** queued for whenever planning schedules it: the **ADR-0034 restructure** (rpg-toolkit), **`rpg-api#616`** (encounter/v2 layering — orchestration living in the handler package instead of Chapter-1 layering), and the **snapshot-vs-live dual-home family** (`rpg-toolkit#736` AC dual-homed, `rpg-toolkit#740` stale HP snapshot on the NPC-targeting path, and the `rpg-api#612`-class of bugs generally). Scheduling relative to the three groupings above is a planning-session decision, not decided here.

## Decision log (2026-07-05, end of day)
1. **Kirk merged all wave-1 PRs same-day** — `rpg-toolkit#747`/`#748`, `rpg-dnd5e-web#438`/`#439`, `rpg-api#626`, plus the v0.65.0 bump PR #628 and toolkit docs close-the-loop #749.
2. **`rpg-api#627` filed after the expertise re-trace corrected a wrong first diagnosis** — toolkit was innocent; the proto boundary drops the skill level (Expert flattens to proficient at the wire).
3. **v1-vs-v2 chassis call stands as written earlier** (see Solid: two stacks, one destination) — no new decision here.

## Pointers
- Board **#19** (The Dungeon Run) — https://github.com/users/KirkDiggler/projects/19 — the current board; wave 1 complete, see Now/Next.
- **`rpg-api#627`** — Party Assembles fix: expertise math invisible at the wire (`Proficiencies` flattens Expert); the remaining item from wave 1's Rogue-creation verify.
- **rpg-project#75** — Beat 2 wave issue: full ledger (14 PRs / 4 repos) + retro comment (paved-road pattern, death-save arc, permissions fix).
- **ADR-0034** — `rpg-toolkit/docs/adr/0034-where-encounter-logic-lives.md` — queued for restructure in the structural window.
- Board **#13** (Chapter 2: Combat Verbs) — https://github.com/users/KirkDiggler/projects/13 — prior Chapter-2 board, Beat 1/2 record.
- Follow-up shelf: `rpg-api#616`, `rpg-api#596` (parked; design record merged as rpg-project#72), `rpg-toolkit#736`, `rpg-toolkit#740`, `rpg-dnd5e-web#432`.
- Old open `rpg-dnd5e-web` PRs (#370, #38, dependabot batch) are unrelated pre-existing housekeeping — not from this thread.
- Roles: `docs/teams/roles/<role>/{prompt,field-notes}.md`.
