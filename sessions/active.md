# Session handoff — 2026-05-30: Clean-slate reset of the rpg-api encounter vertical

**For the next session:** read this, then `CLAUDE.md`, then `ideas/encounter/v1alpha2/{design,orchestrator-design,sdk-direction-rpgapi}.md`. You are continuing a **clean-slate rebuild of rpg-api's encounter path**. Most decisions below are made — reconcile + build, don't re-derive.

## ⏩ Update — late 2026-05-30 (Phase 1 MERGED + MCP-playtest-verified)

- ✅ Toolkit **#687/#688** + proto **#168** — merged (unchanged).
- ✅ **#576** (first runner attempt, `feat/575`) and **#575** — closed/superseded.
- 🟢 **rpg-api #578 (`feat/577-encounter-load-rebuild`) — CI GREEN + director-verified, NOT yet merged.** Lands the clean slice: `encounter.Load` single load path (kills the #684 double-subscribe by construction), generic `ActivateFeature` via the toolkit verb, `ResourceChanged` translate, depguard import-guard — **plus** the monster-damage one-model fix below. Closes #577.
- 🐛→✅ **#579 (goblin dealt 5, should be 8) — FIXED in #578.** rpg-api's `syntheticMonsterWeapon`/`extractBaseDice` stripped the goblin's `+2` (its DEX mod) and the resolver added STR(−1). One-model fix: feed the toolkit the **real scimitar** (finesse) so its existing logic picks DEX → `1d6+2 = 8`. Hack deleted. shield=8, rage baseline=8 / raging=4. (Lesson: *"is this actually different? no → same system"* — a prior agent hid this by changing the test to 5; rejected.)
- 📋 Known follow-ups: **#580** (pin mockgen — the version mismatch that false-alarmed pre-commit); **71 PRE-EXISTING lint issues** (not CI-gated — main is green; separate cleanup); **EndTurn** still uses `publishTurnEndAndPersistReset` (`Cleanup`-patched, not yet fully migrated to the runner — Phase 2); stale comments in `dnd5e_combat_resolver_test.go` referencing the deleted funcs (tiny).
- 🆕 **Director role** defined (`docs/teams/roles/director/prompt.md`); `CLAUDE.md` now points every session here first.

**✅ SIGNED OFF + MERGED.** MCP playtest (web team-member drove) verified end-to-end: rage indicator lit (`StatusApplied`); raging greataxe breakdown carried `raging:2`; goblin → raging-bob `EntityDamaged amount=3` with `dnd5e:abilities:dex:2` (the correct finesse ability — the fix confirmed live), raw 6 halved by Rage resistance → 3; **no "modifier ID already exists."** #578 merged; #577/#579 closed.

**NEXT:**
- The **4 Brothers (board #12)** can now ride the clean rails — but fix the **deterministic-initiative + leading-NPC dispatch** gap first (**api #581**); it blocks repeatable playtests (the team-member had to hand-edit Redis to make the player go first).
- **Phase 2 of the clean slice:** migrate `EndTurn` fully onto the runner (currently `Cleanup`-patched, not migrated). Umbrella **#574** stays open for this.
- Playtest follow-ups: web **#422** (RageCharges display — proto-pin bump + ResourceChanged consumer), **#423** (double stream-event delivery — dedup race), **#424** (miss is stream-silent); + **#580** (pin mockgen).
- The **web team-member now owns driving the MCP playtest** (its first run logged in `docs/teams/roles/rpg-dnd5e-web-member/context/`).

---

## What happened (short version)

Began as "verify the 4 brothers (Chapter 2) are L1-playable." Pulled to chapter altitude, audited rpg-api, found it had drifted: **business logic leaked into rpg-api** (rage tier table, charge math, condition construction in the `ActivateFeature` handler), the **event-bus load path is scattered**, and a **double-apply bug class (#684)** lives in it. Kirk's call: **clean-slate the rpg-api encounter vertical** — build the clean path properly. We keep losing our way there (it's leaked to web too — switch statements worse than the rage one).

**Critical correction:** the `/home/kirk/personal/rpg-project` we'd been working in was a **stale, disconnected fragment** (no `.git`, missing `CLAUDE.md`/`docs/`/most of `ideas/`) — NOT the real repo. We re-derived design that already existed. Fixed now: canonical cloned; fragment archived at `/home/kirk/personal/rpg-project.local-archive-2026-05-30`. **Always work from the git clone.**

## The architecture (converged — reconcile with `ideas/encounter/v1alpha2/orchestrator-design.md`)

- **`encounter` orchestrator (rpg-api), one method per RPC:** `.Move`, `.Attack`, `.ActivateFeature`, `.EndTurn`, …
- Each method opens with a private **`load(id string)` — // loads all entities related to the encounter (room, monsters, players, traps) + their conditions onto the bus, once; also wires the event hookup.** ONE load path ⇒ the #684-class double-subscribe is structurally impossible.
- **Handler = pure translation:** auth → proto→toolkit → call orch method → toolkit→proto → return. ~20 lines, zero rules, zero `rulebooks/dnd5e/{conditions,resources,classes,weapons,armor}` imports (import-guard enforces).
- **Events are primary** (multiplayer; single-player rides the same path): **toolkit drives the events; rpg-api converts toolkit events → proto events** (translate layer). RPC responses are lean acks.
- **Boundary (locked):** rpg-api owns repos + orchestration ("what's in the encounter, where it lives"); toolkit owns hydration + rules ("how it comes alive, what it means") and has NO repos.
- **Clean-code bar (Kirk):** simple, isolated, cleanly-named, delegated — a `.load(id)` you trust by its name and don't have to read. Readable flows of trustable units.

## Why clean-slate (the bug that proved it)

Activate Rage → goblin attacks → DamageChain throws `"modifier ID already exists"`: the rage damage modifier subscribes **twice** on one bus, because loading is scattered (encounter load + lazy combat-resolver `loadCharacterWithBus` + `applyReactionConditions` + `charCache`). Unfixable-in-place; the single `load()` cures the class. The incremental "runner over existing handlers" attempt hit this wall — hence the rebuild.

## Cross-repo state (what's real, end of session)

- ✅ **rpg-api-protos #168** — `ResourceChanged` event — **MERGED**.
- ✅ **rpg-toolkit #687 / PR #688** — `enc.ActivateFeature` verb (calls the toolkit's existing rage rules via `character.ActivateAbility`; bridges `ConditionApplied` + `ResourceChanged` to the broker) + `ResourceChangedEvent` broker type + a #684-class bus-teardown guard (`defer char.Cleanup`) + regression test — **MERGED**. Encounter module re-tagged on merge.
- 🔁 **rpg-api #576** — the half-attempt (`EncounterRunner` over existing handlers) — OPEN, RED, **superseded** by the clean-slate. **CLOSE it**; salvage into the rebuild: the clean `ActivateFeature` handler shape, the `ResourceChanged` translate case, the import-guard (`.golangci.yml` depguard), the `wave-3-barbarian` devseed.
- ✅ **rpg-api #568** — prior hand-rolled Wave-3 ActivateFeature — already CLOSED (superseded).
- Board **#11** (Architecture Honesty): umbrella **#574** + sub-issues. Board **#12** (4 Brothers / Chapter 2) consumes the clean rails.

## Operating model (durable — also in memory under `feedback_*`)

- Toolkit is good enough. **Gap-closer:** find a gap → pause implementers → fix toolkit → resume.
- **"Lean" = build what we need, done properly** (full player flow, really tested) — NOT minimize/defer/make-it-work.
- **Autonomy:** the director drives, incl. **merging PRs**, conditioned on logging every decision as a visible fact (board / PR comments / this handoff). Grows over time via layered review.
- **Layered review:** Copilot (code repos only — NOT protos/project) + the team-member's own `/code-review` self-pass + a code-review agent for protos/project + director review; **MCP playtest is the signoff bar.**
- **Roles:** `docs/teams/roles/<role>/prompt.md` (+ `context/` json — the team-memory system). `rpg-api-member/prompt.md` already encodes the boundary / no-game-logic. **FOLD IN** (don't duplicate) what we drafted: explicit **pushback duty**, **modular/replaceable/extensible + simple/deliberate/explicit/contained** mindset, **"the game server is where we learn → surface a toolkit helper, never inline logic,"** the **four-question done-gate**, and **stop-and-report-if-blocked**. Drafts: `…local-archive-2026-05-30/roles/{rpg-api,rpg-toolkit}.md`.
- Permission allowlist is comprehensive; dispatched agents must stop-and-report if blocked (invisible-stall fix).

## Next steps

1. **Read** `ideas/encounter/v1alpha2/{design,orchestrator-design,sdk-direction-rpgapi,plan,roadmap}.md` — the established clean-path design. Reconcile our findings INTO it (the single-`load` fix, the merged toolkit verb, `ResourceChanged` + import-guard). Do not re-derive.
2. Fold the role-mindset additions into `docs/teams/roles/rpg-api-member/prompt.md` (+ `rpg-toolkit-member`).
3. Build the clean rpg-api encounter vertical (orchestrator + `load()` + thin handlers + translate). Close **#576**, salvaging its good parts.
4. Verify the 4 brothers against `docs/qa-checklists/{barbarian,fighter,monk,rogue}.md` — the FULL combat round (both directions; defenses observably reduce damage; Monk UD = 10+DEX+WIS, Barb UD = 10+DEX+CON), via MCP playtest.
5. The `wave-0-architecture-honesty/{design,plan}.md` in the archive are a from-scratch redo — input to the reconciliation, not the canonical design.

## Pointers

- Pre-reconciliation design we posted: rpg-api issue **#574** comment.
- Archive (fragment): `/home/kirk/personal/rpg-project.local-archive-2026-05-30/` — wave-0 design+plan, role drafts, chapter-2 docs, old Wave-2.11d session log.
- Memory captured this session: `feedback_gap_closer_sessions`, `feedback_api_thin_watch_bus_double_apply`, `feedback_lean_means_what_we_need_done_right`, `feedback_implementer_role_encodes_boundary`, `feedback_invisible_permission_stalls`, `feedback_autonomy_with_decision_log`, `feedback_copilot_coverage`.
