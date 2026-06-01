# Active handoff — 2026-06-01: rpg-api encounter carve COMPLETE + playtest-verified (Chapter 1)

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.

## Now
Chapter 1 (Architecture Honesty), board #11, umbrella rpg-api **#574**. The v1alpha2 encounter orchestrator carve (**#582**) is **code-complete AND goal-verified**. The wave's last step is the **retro** (sweep the ledger, close #574). Director runs thin — commission + verify (`docs/teams/roles/director/`).

## Solid (verified — keep, don't re-derive)
- **Cross-repo cascade landed:** toolkit #689 (PR #690) merged → `encounter/v0.17.0` → rpg-api #584 adopted it (local replace stripped). The Persistence/Hydration contract is first-class doc (`architecture.md`/`boundaries.md`): ToData/LoadFromData is a *convention*; `LoadFromData` returns a **live bus-subscribed** entity; load-once = subscribe-once = the #684 cure.
- **The encounter orchestrator carve (#582) is COMPLETE.** All 7 verbs on `internal/orchestrators/encounter/v2`, the **Runner is deleted**, `rulebooks/dnd5e/combat` is in the **depguard deny set** (no guarded handler/orchestrator file imports any rulebook). PRs #585/#587/#588/#589/#590/#591/#592 — each verified (CI + Copilot + lane) and merged. Pattern: thin handler → `Orchestrator.<Verb>` → one private `load(ctx,in)` → toolkit verb → persist (`SyncErr`). Resolver adapters stay handler-side (the tkenc↔rulebook seam, depguard-excluded). The reaction **wire-pause is DEFERRED** (kept internal — Kirk's call).
- **GOAL VERIFIED — MCP playtest PASS (2026-06-01):** rage+combat end-to-end through v2. Decoded: offensive rage bonus flows (greataxe `amount=6` = 1+STR3+raging2); **goblin → raging-bob halved 3 → 1** (scimitar1+DEX2=3 → rage resistance floor(3/2)=1; HP 14→13); the carved EndTurn NPC-dispatch loop ran the goblin's turn; **no "modifier ID already exists"**; no regressions. Fresh env (api `eaf4046`, `encounter/v0.17.0`, `.vite` cleared, no `$unknown`). Devseed `wave-3-barbarian`. **Method caveat:** driven via the *real v2 Connect client* (Chrome `evaluate_script`), **not the harness UI** — the harness has no rage/ActivateFeature button (rpg-dnd5e-web#425). This proves the v2 orchestrator + event-stream **at its layer** (legit for the rpg-api carve); the **harness/game-UI path for rage is NOT yet proven**.

## Open questions
- None blocking.

## Next
1. **See it end-to-end, THEN retro (Kirk's call, 2026-06-01):** the rage button is a **RESTORE** — built in PR **rpg-dnd5e-web#420** (`feat/419-rage-button`, CI-green, Copilot-addressed) and stranded unmerged when the #582 carve pulled focus to rpg-api. Web member is staging it to merge-ready (re-pin the web proto to match rpg-api's current server contract). **On resume:** merge #420 → re-run the harness playtest **UI-driven** (rage button → `ActivateFeature` → goblin damage halves *on screen*) so **Kirk witnesses the full game path end-to-end**. *That observed run is the gate for the retro — not "merge-ready."* (+ door devseed for Interact = rpg-api lane, queued under #425.)
2. **Then the wave retro (closes #574)** — sweep the ledger, capture decisions. Director + Kirk.
3. **Then Chapter 2 (#12): the vertical slice** — scope the playtest north-star (4 brothers L1-playable, multi-room, locked-door gating, boss room; pure combat first) into issues, then build features on the clean rails. Next-director kickoff issue to be drafted (scope-then-build).

## Ledger (follow-ups under the wave, for the retro to sweep)
- **rpg-api #586** — enforce depguard in GitHub CI (lint isn't a CI job today; the boundary guard is local-pre-commit-only).
- **rpg-toolkit #695** — SDK home for the Shield +5 AC magnitude (rule magnitude currently isolated in rpg-api's reaction adapter).
- **rpg-toolkit #694** — ADR-0031 + contract markers; now unblocked (re-dispatch with post-#689 signatures). Retro-refined: marker = drift-protection + definition-site pointer, not cold-reader discoverability.
- **rpg-dnd5e-web #425** — harness must drive the v2 RPCs through the game's shared hooks. Diagnosis: the rage button is a **RESTORE** — **PR #420** (`feat/419-rage-button`) built it via the shared `useActivateFeatureV2` hook, CI-green + Copilot-addressed, stranded unmerged. **DONE staging (2026-06-01): PR #420 is MERGE-READY** — proto re-pinned `92a9d062`→`99ba9c0` (commit `0aca473`, matches rpg-api server contract `v0.0.0-20260530184527-99ba9c0608f7`; superset adding `ResourceChanged`), `npm install` refreshed lock, `ci-check` green, all 5 CI checks SUCCESS, mergeStateStatus CLEAN, 2 Copilot threads still resolved. Body: `Closes #419` + `Part of #425`. **Director + Kirk merge (not me).** Remaining gate: UI-driven re-playtest (rage button → ActivateFeature through harness) closes the #582 "harness-verified" caveat. Door devseed (Interact) = rpg-api lane, dispatched separately.
- **Game-UI v1alpha1→v2 ActivateFeature parity** — the *game's* `LobbyView` still calls v1alpha1 `useActivateFeature` (harness now on v2). Full game-UI migration off the v1alpha1 encounter client is a separate larger effort (not #425) — file/scope at the retro.
- **Playtest caveats (minor):** (a) v2 stream surfaces no distinct `ResourceChanged` for the rage-charge decrement (only `StatusApplied`); (b) web console warn — `useEncounterStream2` unhandled event case on a zero-damage no-op reaction-check event (cosmetic reducer gap).
- **#691** (ActivateFeature self-load), **#692** (IsDirty beyond HP), **#693** (cross-RPC held-entity / the deferred reaction wire-pause), **#583** (`ci-check` wipes uncommitted).

## Decision log (2026-05-31 → 06-01)
1. Status-doc template — CLAUDE.md owns the shape. `3aa0d76`
2. Hydration contract → first-class docs. `3aa0d76`
3. #694 marker over interface (toolkit member pushback). `#694`
4. #694 sequenced behind #689. `#694`
5. Rage-resistance gate cleared → landed #689/#582-chunk-1 (#690 → `v0.17.0` → #584).
6. Chunk 2 = full carve-out, Sequencing B; EndTurn reaction wire-pause **deferred** (kept internal — Kirk). `#582`
7. All 7 verbs carved + Runner deleted + `combat` locked in depguard deny set (#585–#592).
8. **Goal sign-off: MCP playtest verified the v2 orchestrator path** (real v2 client, decoded — rage halving 3→1, #684 cure holds); #582 closed *at its layer*. **Caveat:** driven via the v2 client, **not the harness UI** (harness has no rage button → rpg-dnd5e-web#425); harness/game-UI path for rage not yet proven. Open: whether "done" requires the harness-UI bar.

9. **Kirk's call (pause): see the slice end-to-end before the retro.** The retro is gated on an *observed* UI-driven harness playtest (restored rage button → ActivateFeature → on-screen halving), not on merge-ready PRs. "Merge-ready" ≠ "I saw it work."

Retro-as-evaluator principle captured in `director/field-notes.md` (`e9b6a37`): decide with the architecture as north star, log a retro criterion, judge in action.

## Pointers
- Board: #11 (Chapter 1, umbrella #574); #12 (Chapter 2: 4 Brothers — next).
- Design: `ideas/encounter/v1alpha2/` (`plans/{10,11}`). Contract: `architecture.md → "The Persistence & Hydration Contract"`; toolkit ADR-0030 / journey-050.
- Orchestrator: `internal/orchestrators/encounter/v2/` (one `load` per verb; resolver adapters handler-side, depguard-excluded).
- Roles: `docs/teams/roles/<role>/{prompt,field-notes}.md`.
- Repo state: toolkit `main` (`v0.17.0`); rpg-api `main` (carve complete, `eaf4046`+). Playtest stack may still be up (api/envoy/vite/chrome/redis) — fine to tear down.
