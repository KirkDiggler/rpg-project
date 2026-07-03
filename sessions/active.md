# Active handoff — 2026-07-03: Beat 2 designed + adversarially gated (PASS); #596 parked; OA fix merged

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.

## Now
**Chapter 2: Combat Verbs**, board **#13**, umbrella **rpg-project #54**. **Beat 2 (mechanical effects: Dodge/Help/Hide do real things) is designed and gate-PASSED** — wave doc on **rpg-project PR #73** (branch `design/beat2-mechanical-effects`, head `5a30c5b`), adversarial gate verdict PASS after one revision round (both gate comments on the PR). **Awaiting Kirk's review + merge of #73 — implementation dispatches are gated on that.** `rpg-api#596` is **PARKED** (see decision log). Director runs thin — commission + verify.

## Solid (verified — keep, don't re-derive)
- **Beat 2 scope** (from the gated doc on PR #73 — read it, don't re-derive):
  - Toolkit: construct `DodgingCondition` on `Dodge.Activate` (Rage pattern — the condition already exists incl. self-expiry, just never gets constructed).
  - Revive the dead `TurnStartTopic` publish in `encounter.seedActorTurn` — this lights up **three** dormant subscribers: Dodging, RecklessAttack, and Unconscious's auto-death-save (the last is `rpg-api#612`'s death gate from the other side — regression-check RecklessAttack).
  - New `checks` package (`MakeAbilityCheck` + `AbilityCheckChain`, mirrors `saves.MakeSavingThrow`), built via Hide's Stealth roll.
  - Help is **attack-only** this wave (check-subscriber path deferred; machinery still ships).
  - Cross-entity targeting threads the *target's* PlayerData — the condition audience projects from the ally's position, not the actor's.
  - Hidden/Helped conditions require `loader.go`/`factory.go` registration + `ToJSON` round-trip tests, or they evaporate at the first RPC.
  - Advantage/disadvantage added full-chain — the rulebook's attack resolution already computes them (confirmed dropped today at encounter `AttackOutcome`, both rpg-api resolvers, `AttackResolvedEvent`, and the protos).
  - Condition display names stay web-resolved (API forwards refs only).
  - API seeds player HP at `AddPlayer` (`#612` folded in as required scope).
  - Consume step tags toolkit + bumps api, shipping the merged OA fix (`#724`).
- **Playtest sign-off bar:** named devseed fixture `wave-2-beat2` (a Monk + a Fighter), one MCP session, three beats — Dodge → goblin attack shows visible disadvantage; Hide → Hidden status + attack interactions both ways (success path live; Hide-failure covered by a unit test with a mock roller, not the playtest); Help → condition attributed to the ally, goblin hits the Fighter for real damage, Fighter driven to 0 HP, auto death-save asserted at their next turn start (runtime-confirms `#612` + turn-start revival + Unconscious in one scene).
- **`rpg-toolkit#722`/`#723` CLOSED** — PR `rpg-toolkit#724` merged by Kirk 2026-07-03. Root cause: `getAttackerMeleeWeapon` was a hardcoded unarmed-strike stub; fix = `MeleeWeaponProvider` interface + `Character.MeleeWeapon()`/`Monster.MeleeWeapon()`. `#723` wasn't reproducible on current main — closed with regression tests locking the invariant; if the symptom recurs, check rpg-api resolver wiring first. Follow-up filed + boarded: `rpg-toolkit#725` (natural bite/claw weapons resolve as unarmed on the OA path). The fix is on toolkit `main` but **unreleased** — ships with Beat 2's version bump.
- **`rpg-api#612` filed + boarded** (from today's HP dual-home probe): player snapshot HP is never seeded (the sole `AddPlayer` call omits it) and never reconciled with the character store (no player equivalent of `syncMonsterDataFromSnapshot`); the HP-bar projection and death gate both read the snapshot copy. Static-trace evidence is in the issue; runtime confirmation is Beat 2 playtest beat 3.
- **Tick vocabulary on current main:** only `dnd5eEvents.TurnEndTopic` is live (`encounter/combat.go:362`, fires per actor incl. NPCs). `TurnStartTopic` is dead — its sole publisher (`turn_manager.go:246`) has no live callers. No round-start/end topics exist; round is a counter on the broker's `TurnStartedEvent`.
- **Conventions** (in director memory, restated): worktree-per-implementer, canonical checkouts stay on main; always set `model` explicitly on dispatches (Sonnet default, Opus for gates/deep-debug); refs as `repo#N` + link; permission prompts are terminal-only and invisible to orchestrator + Kirk — pre-authorize command sets; a status-ping met with silence ≈ a prompt-stalled agent (bit us twice today).

## Open questions / calls (verify before acting)
- **Kirk's review of rpg-project PR #73** (the wave star) — merge gates filing the Beat 2 wave issue under `#54`, the per-repo issues from the doc's breakdown, and implementation dispatches.
- **PR #72 disposition** (parked `#596` Option-A design, 4 commits incl. lifecycle/durability/cadence sections): merge-as-parked-record vs. leave open vs. close — Kirk's call, no urgency.
- rpg-project **PRs #65 and #52** are older open PRs from previous threads — reconcile/close when convenient.
- Toolkit `make pre-commit` is still broken on main (`rpg-toolkit#721`, `bc` script) — implementers run the underlying lint/test targets directly; never `--no-verify`.

## Next
1. Kirk merges PR #73 → file the Beat 2 wave issue under `#54` + per-repo issues (one per PR) from the doc's breakdown.
2. Dispatch implementers per the doc's work-by-layer plan (toolkit first; api/protos/web after the toolkit tag). Pre-authorize the Go/gh command set before dispatching.
3. Sign-off = the three-beat `wave-2-beat2` MCP playtest + participatory retro (green CI alone is insufficient).

## Decision log (2026-07-03)
1. **`rpg-api#596` parked by Kirk.** The defensive-rage motivation was debunked by a read-only code trace (rage's turn-end rule fired 5e-correctly; the persist cascade round-trips every player's condition state on current main; no `TurnStart` reset exists — the topic is dead code). Design PR #72 amended to withdraw the "state was provably lost" claim (`213c20b`) and add encounter-lifecycle/pause-resume, disconnect≠teardown, declared per-condition durability (Kirk's curse example), and persistence-cadence-is-policy sections (`3b67e88`, `e5583d5`). Unpark trigger: Beat 2 surfacing real state-ownership pain. Visible: PR #72 + `rpg-api#596`.
2. **Kirk merged rpg-toolkit#724 directly** (OA weapon fix) after personally unblocking a terminal-only permission stall on the implementer.
3. **HP dual-home probe** (commissioned when Kirk challenged the `#596` evidence) confirmed the dual-home is structurally live and found the unseeded-snapshot-HP bug → `rpg-api#612` filed, boarded, and folded into Beat 2 as required scope.
4. **Beat 2 design produced and adversarially gated** — PASS-with-required-changes (R1–R6) → revision `5a30c5b` → re-review PASS. The gate worked both directions: the designer successfully refuted one gate nit (a citation that was actually correct).
5. **Beat 2 scope decisions:** Help is attack-only this wave; Hide-failure is verified by unit test, not playtest; observer-set computation lives in the encounter SDK (gate-endorsed — "who sees whom" is spatial) with the guardrail that passive Perception is gathered via a rulebook-owned `Combatant.PassivePerception()` method, never inlined in the SDK.

## Pointers
- Wave star: `ideas/encounter/v1alpha2/mechanical-effects/design.md` (on PR #73's branch until merged) + both gate comments on the PR.
- Parked: `ideas/encounter/v1alpha2/combat-state-home/design.md` (PR #72); `gap-defensive-rage-persistence.md` §1 is superseded by PR #72's corrected motivation.
- Boards: **#13** (Chapter 2, umbrella `rpg-project#54`); **#14** (Tumult, parked). North-star invariants: `ideas/encounter/v1alpha2/design.md`.
- Roles: `docs/teams/roles/<role>/{prompt,field-notes}.md`.
