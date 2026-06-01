# Active handoff — 2026-06-01: rpg-api encounter carve COMPLETE + playtest-verified (Chapter 1)

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.

## Now
Chapter 1 (Architecture Honesty), board #11, umbrella rpg-api **#574**. The v1alpha2 encounter orchestrator carve (**#582**) is **code-complete AND goal-verified**. The wave's last step is the **retro** (sweep the ledger, close #574). Director runs thin — commission + verify (`docs/teams/roles/director/`).

## Solid (verified — keep, don't re-derive)
- **Cross-repo cascade landed:** toolkit #689 (PR #690) merged → `encounter/v0.17.0` → rpg-api #584 adopted it (local replace stripped). The Persistence/Hydration contract is first-class doc (`architecture.md`/`boundaries.md`): ToData/LoadFromData is a *convention*; `LoadFromData` returns a **live bus-subscribed** entity; load-once = subscribe-once = the #684 cure.
- **The encounter orchestrator carve (#582) is COMPLETE.** All 7 verbs on `internal/orchestrators/encounter/v2`, the **Runner is deleted**, `rulebooks/dnd5e/combat` is in the **depguard deny set** (no guarded handler/orchestrator file imports any rulebook). PRs #585/#587/#588/#589/#590/#591/#592 — each verified (CI + Copilot + lane) and merged. Pattern: thin handler → `Orchestrator.<Verb>` → one private `load(ctx,in)` → toolkit verb → persist (`SyncErr`). Resolver adapters stay handler-side (the tkenc↔rulebook seam, depguard-excluded). The reaction **wire-pause is DEFERRED** (kept internal — Kirk's call).
- **GOAL VERIFIED — MCP playtest PASS (2026-06-01):** rage+combat end-to-end through v2. Decoded: offensive rage bonus flows (greataxe `amount=6` = 1+STR3+raging2); **goblin → raging-bob halved 3 → 1** (scimitar1+DEX2=3 → rage resistance floor(3/2)=1; HP 14→13); the carved EndTurn NPC-dispatch loop ran the goblin's turn; **no "modifier ID already exists"**; no regressions. Fresh env (api `eaf4046`, `encounter/v0.17.0`, `.vite` cleared, no `$unknown`). Devseed `wave-3-barbarian`.

## Open questions
- None blocking.

## Next
1. **Wave retro (closes #574)** — sweep the ledger below, capture the wave's decisions, close the wave. Director + Kirk.
2. **Then Chapter 2 (board #12): the 4 Brothers** — rides the clean rails the carve just laid.

## Ledger (follow-ups under the wave, for the retro to sweep)
- **rpg-api #586** — enforce depguard in GitHub CI (lint isn't a CI job today; the boundary guard is local-pre-commit-only).
- **rpg-toolkit #695** — SDK home for the Shield +5 AC magnitude (rule magnitude currently isolated in rpg-api's reaction adapter).
- **rpg-toolkit #694** — ADR-0031 + contract markers; now unblocked (re-dispatch with post-#689 signatures). Retro-refined: marker = drift-protection + definition-site pointer, not cold-reader discoverability.
- **Playtest caveats (minor):** (a) v2 stream surfaces no distinct `ResourceChanged` for the rage-charge decrement (only `StatusApplied`); (b) web console warn — `useEncounterStream2` unhandled event case on a zero-damage no-op reaction-check event (cosmetic reducer gap); (c) Interact routes clean but wasn't exercised vs. a real door (wave-3-barbarian has none — needs a door devseed).
- **#691** (ActivateFeature self-load), **#692** (IsDirty beyond HP), **#693** (cross-RPC held-entity / the deferred reaction wire-pause), **#583** (`ci-check` wipes uncommitted).

## Decision log (2026-05-31 → 06-01)
1. Status-doc template — CLAUDE.md owns the shape. `3aa0d76`
2. Hydration contract → first-class docs. `3aa0d76`
3. #694 marker over interface (toolkit member pushback). `#694`
4. #694 sequenced behind #689. `#694`
5. Rage-resistance gate cleared → landed #689/#582-chunk-1 (#690 → `v0.17.0` → #584).
6. Chunk 2 = full carve-out, Sequencing B; EndTurn reaction wire-pause **deferred** (kept internal — Kirk). `#582`
7. All 7 verbs carved + Runner deleted + `combat` locked in depguard deny set (#585–#592).
8. **Goal sign-off: MCP playtest PASS — rage+combat verified end-to-end through v2. #582 closed.**

Retro-as-evaluator principle captured in `director/field-notes.md` (`e9b6a37`): decide with the architecture as north star, log a retro criterion, judge in action.

## Pointers
- Board: #11 (Chapter 1, umbrella #574); #12 (Chapter 2: 4 Brothers — next).
- Design: `ideas/encounter/v1alpha2/` (`plans/{10,11}`). Contract: `architecture.md → "The Persistence & Hydration Contract"`; toolkit ADR-0030 / journey-050.
- Orchestrator: `internal/orchestrators/encounter/v2/` (one `load` per verb; resolver adapters handler-side, depguard-excluded).
- Roles: `docs/teams/roles/<role>/{prompt,field-notes}.md`.
- Repo state: toolkit `main` (`v0.17.0`); rpg-api `main` (carve complete, `eaf4046`+). Playtest stack may still be up (api/envoy/vite/chrome/redis) — fine to tear down.
