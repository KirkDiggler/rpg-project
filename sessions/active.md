# Active handoff — 2026-07-01: brain reconciled; Chapter 2 TakeAction (Beat 1) code-complete on main, awaiting playtest + retro

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.

## Now
**Chapter 2: Combat Verbs**, board **#13**, umbrella **rpg-project #54**. Verb-shaped waves on the
v1alpha2 encounter route; the 4 brothers are the *cast* that verifies each. **Beat 1 = TakeAction
end-to-end is CODE-COMPLETE and merged to `main` across all four repos** (commits confirmed 2026-07-01
— see Solid). The wave's remaining **done-bar is NOT met**: per our own rule, green CI ≠ done — Beat 1
still needs its **end-to-end MCP playtest sign-off** and the **participatory retro** to close. Chapter 1
(Architecture Honesty, #574) is closed. One Chapter-1 call carries over: **#596** (combat-mutable-state
ONE HOME — promote vs archive). Director runs thin — commission + verify (`docs/teams/roles/director/`).

_This session (2026-07-01) reconciled the brain repo after the Tumult/C++ detour — see Decision log._

## Solid (verified — keep, don't re-derive)
- **North star** = `ideas/encounter/v1alpha2/design.md`, carrying the **★ North-Star Invariants** (13,
  in Boundary / Events / Capability / Persistence), hardened 2026-06-01. **Every wave validates against
  this before implementation; a violation is a blocker.** Validated wave doc:
  `ideas/encounter/v1alpha2/take-action/design.md`; decision ledger `take-action/decisions.md` (D1–D19).
- **TakeAction Beat 1 landed cleanly — all four repos on `main`, in sync with origin, zero open PRs on
  the three backend repos (git-verified 2026-07-01):**
  - **protos** — #170/#171 (contract: `ActionResolved`, `correlation_id`, `target_kind`,
    `TurnStateChanged`), #172 latest.
  - **toolkit** — #697 core, delivered via #698 (event-faithfulness spine), #700 (menu-as-data), #701
    (verb unification), #702/#703 (Help/Hide), #705 (`TurnStateChangedEvent`), #707 (Granted nil-map
    fix). Head `9cecba2`.
  - **api** — #597, delivered via #599 (seeding gap-closer), #600 (un-suppress resolved events + project
    menu/economy), #602 (turn-start snapshot projection). Head `897d4fd`.
  - **web** — #426, delivered via #427 (render server-driven action menu + live economy). Head `5d6d2be`.
- **What Beat 1 delivers (the unification):** one **general, ref-keyed verb dispatch** — any action ref
  routes through the same path, no per-ref special-casing (killed the `if ref.ID != "attack"` world and
  connected it to the character action-menu/granted-capacity world). Economy deducted in the toolkit; a
  **resolved-action event** (`ActionResolved`, with `action_ref` + `economy_consumed` + correlation id +
  timestamp) published; the **menu + economy delta pushed** to the client (`TurnStateChanged`); the web
  **renders what it's told** (menu-as-data + live economy bar) — no client-side legality gating. Proven
  in code for attack, Monk Martial Arts bonus strike, Help, Hide.
- **Repo state:** toolkit / api / web / protos all `main` = origin, no local dev `replace` overrides,
  no open PRs on the backend repos. Brain repo (`rpg-project`) `main` reconciled — see Decision log.

## Open questions / carried-over calls (verify before acting; not findings)
- **#596** combat-mutable-state ONE HOME — promote vs archive (recommend *promote*; likely a deeper
  combat-state prereq, not a Beat-1/Beat-2 blocker). Doc: `ideas/encounter/v1alpha2/gap-defensive-rage-persistence.md`.
- **Dodge mechanical effect** — toolkit **#699** (verb dispatch lands Dodge; the mechanical AC/save
  effect is the follow-up).
- **Movement units** (hexes vs feet for `movement_remaining`); **idempotency** on a retried `TakeAction`
  (double-decrement). Both deferred rough edges under #54.

## Next (the immediate gated step)
1. **VERIFY Beat 1 end-to-end via MCP playtest** — the real done-bar. Drive TakeAction through the game
   path (attack + Monk Martial Arts bonus strike + live economy/menu render). Restart dev servers +
   clear the vite cache first (a `$unknown` proto field = stale bundle, not a code bug). Green CI +
   merged PRs are **not** sufficient to close the wave.
2. **Participatory retro WITH Kirk** to close Beat 1 (umbrella #54 / board #13); reconcile issue/board
   closure. Also close the lingering **board #11 #50** (Wave 2.11e reactions) which never got formally closed.
3. **Beat 2 — seed the remaining action refs** (dash, disengage, help, hide, move) through the existing
   verb path (factory entries; no new dispatch machinery). Then per-action mechanical effects.
4. **Kirk decides #596.**

## Decision log
1. **Brain repo reconciled after the Tumult/C++ detour (2026-07-01).** Local `main` was ahead 9 / behind
   14 vs origin with a dirty tree (redundant TakeAction paperwork + an older staged Tumult snapshot +
   genuine local-only notes). Snapshotted everything to a safety stash, **merged origin/main** (conflict-
   free — the 9 local doc commits and 14 remote commits touched disjoint files), preserved the genuine
   local-only work (encounter `design.md`, `gap-defensive-rage-persistence.md`, team-context JSONs,
   director field-notes, CLAUDE.md tweaks — commit `a9dd776`), and dropped the redundant/superseded
   uncommitted files. **Push to origin gated on Kirk.**
2. **State correction:** the prior June-1 handoff understated reality (it read `#597 paused`, gap-closers
   "in parallel"). Git shows every Beat-1 PR merged 3–4 weeks ago. Trust the code; this doc now reflects it.
3. **Tumult (C++ combat engine) parked, not lost** — lives on its own board **#14** + pushed branches +
   origin/main (`ideas/tumult/`). Explicitly de-prioritized: current front is rpg-toolkit + the co-op
   dungeon explorer.
4. **Done-bar reaffirmed:** a wave closes on observed end-to-end MCP playtest + participatory retro, not
   on merged PRs (`feedback_playtest_is_the_signoff_bar`, `feedback_retros_are_participatory`).

## Pointers
- Boards: **#13** (Chapter 2: Combat Verbs, umbrella #54); **#11** (Chapter 1, umbrella #574, closed);
  **#14** (Tumult, parked).
- Design: `ideas/encounter/v1alpha2/{design,orchestrator-design,plan,roadmap}.md`;
  `take-action/{design,decisions}.md`; `gap-defensive-rage-persistence.md` (the #596 exploration).
- Orchestrator: `rpg-api internal/orchestrators/encounter/v2/` (one `load` per verb;
  `take_action.go`/`move_entity.go`/`end_turn.go`/`interact.go`). Toolkit verbs:
  `encounter/{combat,combat_phased,turn_state}.go`. Web: `ActionMenu.tsx`, `EconomyBar.tsx`,
  `useEncounterState.ts`.
- Roles: `docs/teams/roles/<role>/{prompt,field-notes}.md`.
