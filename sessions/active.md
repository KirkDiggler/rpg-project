# Active handoff — 2026-06-01: rpg-api encounter carve COMPLETE + rage VERIFIED through the game path (Chapter 1)

> Shape: `CLAUDE.md → Status docs`. Living handoff — **rewritten, not appended**.

## Now
Chapter 1 (Architecture Honesty), board #11, umbrella rpg-api **#574**. The v1alpha2 encounter orchestrator carve (**#582**) is code-complete and its **goal behavior — rage, BOTH offensive and defensive halving — is VERIFIED through the harness/game UI** (controlled test, decoded + Redis-confirmed). An earlier "defensive rage is broken / false green" scare this session turned out to be a **director-introduced re-roll confound** (instructing Bob to *skip his attack* to preserve the goblin legitimately dropped rage per 5e). Wave is **UNBLOCKED** and the **witness gate is MET** — Kirk signed off on the decoded + Redis + screenshot proof (2026-06-01; no live re-run needed). **#420 MERGED** (squash `1377429`, 2026-06-01; #419 auto-closed; branch deleted). The **retro is RUN and #574 is CLOSED** (2026-06-01, OG session — see `## Retro`): goal verified end-to-end through the game path; follow-ups filed + boarded (#696, #593–#595). **One call left for Kirk:** promote vs archive **#596** (combat-mutable-state ONE HOME — recommend promote). Director runs thin — commission + verify (`docs/teams/roles/director/`).

## Solid (verified — keep, don't re-derive)
- **Cross-repo cascade landed:** toolkit #689 (PR #690) merged → `encounter/v0.17.0` → rpg-api #584 adopted it (local replace stripped). The Persistence/Hydration contract is first-class doc (`architecture.md`/`boundaries.md`): ToData/LoadFromData is a *convention*; `LoadFromData` returns a **live bus-subscribed** entity; load-once = subscribe-once = the #684 cure.
- **The encounter orchestrator carve (#582) is COMPLETE.** All 7 verbs on `internal/orchestrators/encounter/v2`, the **Runner is deleted**, `rulebooks/dnd5e/combat` is in the **depguard deny set** (no guarded handler/orchestrator file imports any rulebook). PRs #585/#587/#588/#589/#590/#591/#592 — each verified (CI + Copilot + lane) and merged. Pattern: thin handler → `Orchestrator.<Verb>` → one private `load(ctx,in)` → toolkit verb → persist (`SyncErr`). Resolver adapters stay handler-side (the tkenc↔rulebook seam, depguard-excluded). The reaction **wire-pause is DEFERRED** (kept internal — Kirk's call).
- **GOAL VERIFIED — rage works end-to-end through the harness UI (this session; `feat/419-rage-button`, api `eaf4046`/`encounter/v0.17.0`, fresh bundle no `$unknown`, devseed `wave-3-barbarian`):**
  - **Rage activates + persists** ✅ — `ActivateFeature(rage)` → `StatusApplied raging`; immediately after, the char store shows `raging` present + `rage_charges` 2→1. No `"modifier ID already exists"` anywhere across activate + attack + endturn + NPC turn.
  - **Offensive rage** ✅ — Bob's greataxe decoded `amount=17 [greataxe:12, str:3, raging:2]` — the `raging:2` component present. Goblin 30→13.
  - **Rage survives a turn where Bob attacked** ✅ — after Bob attacks + ends turn, `raging` still on the char store (correctly NOT stripped, because he took combat activity).
  - **Defensive rage halving** ✅ — raging Bob takes the goblin's scimitar: `EntityDamaged char-bob amount=1 [scimitar:1, dex:2, raging:0]` — raw 3 → **halved to 1** (floor(3/2)), HP **14→13**, confirmed in stream AND Redis, with the `raging` resistance component in the breakdown. Screenshot `/tmp/rage-DECISIVE-halving-works.png`.
  - **Carved EndTurn NPC-dispatch loop** ✅ — goblin's turn resolves server-side, round advances; no regressions.
- **The earlier "broken" run was a confound, not a bug:** when the first goblin *missed*, the director told the web member to have Bob **hold his turn** (skip attacking) to preserve the goblin and re-roll the swing. Ending a turn with no attack triggers `endRage("no_combat_activity")` — correct 5e — so the next goblin hit landed on a no-longer-raging Bob (full 3, no `raging` component). Side-by-side proof: broken run `amount=3 [scimitar:1, dex:2]` (rage ended) vs. controlled run `amount=1 [scimitar:1, dex:2, raging:0]` (rage maintained → halved). The verification gate held where it counted: the controlled test ran **before** any fix was built, so no persistence rework was built on a phantom bug.

## Open questions
- None blocking. (Live Kirk-witness of the halving is offered but optional — agent-witnessed + screenshot + Redis already prove the behavior.)

## Next
1. **Witness gate — MET ✅** (Kirk signed off on the screenshot + decoded + Redis proof, 2026-06-01; no live re-run needed).
2. **Merge #420 — DONE ✅** (squash `1377429`, 2026-06-01; #419 auto-closed; branch deleted).
3. **Wave retro (closes #574) — the OG session runs it** (Kirk's call). This session staged the agenda → see **`## Retro agenda`** below. The retro sweeps/files the ledger, evaluates the decisions in action, and closes #574.
4. **Then Chapter 2 (#12): the vertical slice** — scope the playtest north-star (4 brothers L1-playable, multi-room, locked-door gating, boss room; pure combat first) into issues, then build features on the clean rails. Next-director kickoff issue to be drafted (scope-then-build).

## Retro — RUN 2026-06-01 (OG session); Chapter 1 CLOSED
**Executed.** **#574 CLOSED** — the carve's goal behavior is verified end-to-end through the game path (rage both halves; carve mechanics; #684 cure holds). Follow-ups filed + boarded: rpg-toolkit **#696** (DamageComponent.Multiplier), rpg-api **#593** (un-skip/strengthen the resistance test), **#594** (surface attack-miss), **#595** (deterministic wave-3 devseed). **One call left for Kirk:** **#596 — combat-mutable-state ONE HOME** (filed/tracked) — *promote* (recommend: the HP 7-vs-14 divergence is a real honesty gap, likely a Chapter-2 prereq; A-vs-B design is a later pass) *vs archive*. The items below are the evaluation record (decisions judged in action).
1. **Close #574** — Chapter 1 goal behavior is verified end-to-end through the game path (rage both halves; carve mechanics; #684 cure holds).
2. **File the non-blocking follow-ups as issues under the wave** (from the ledger): toolkit `DamageComponent.Multiplier`; test-hardening (un-skip `TestRage_Resistance…` + audit siblings for Skip-hatches); silent-miss UX (`AttackMissed` signal); deterministic `wave-3-barbarian` devseed.
3. **Evaluate the decisions in action:**
   - **The re-roll confound** — a director-written repro changed the variable under test (rage on/off). Lesson is in `director/field-notes.md`; the retro confirms the process change ("does my repro touch a precondition of the thing I'm measuring?").
   - **The verification gate held** — the controlled test ran BEFORE a fix was built → no persistence rework on a phantom bug. Reinforce; is the cost/benefit right?
   - **Harness-as-the-bar is paying off** — it caught a layer-bypass false claim (the v2-client "false green") AND routed us to the confound's truth. Evaluate continued investment in playtest-as-spec (#425).
   - **Architecture-honesty: combat-mutable state has "three homes"** — `ideas/encounter/v1alpha2/gap-defensive-rage-persistence.md` observed HP diverging (snapshot 7/14 vs char store 14/14), charges on the char store, conditions transient. Rage works *today*, but is the split a latent honesty problem (does HP reconcile correctly at encounter teardown)? Decide: promote "one home for combat-mutable state" to an initiative, or archive the doc.
4. **Capture** the retro's decisions in the decision log + propagate to the board.

## Ledger (follow-ups under the wave, for the retro to sweep)
- **NON-BLOCKING — rage residue (filed at/after retro):**
  - **rpg-toolkit `DamageComponent.Multiplier`** — defensive halving *works*, but the wire can't cleanly represent the ×0.5 (shows `raging:0` + a reduced total). Add `Multiplier` (or pre/post-mitigation totals) so the breakdown is faithful and renderable, and an integration test can assert the halving directly. Enhancement, not a bug.
  - **Test-hardening** — `rpg-api .../barbarian_test.go::TestRage_Resistance_AppliedWhenTakingDamage` has `Skip()` escape hatches + asserts a multiplier the wire can't carry, so it doesn't robustly assert the (correct) halving. Strengthen to assert final-amount halving (depends on the `Multiplier` add). Audit siblings for the same Skip pattern. **NB:** behavior is correct — this is test quality, not a behavior false-green.
  - **Silent-miss UX** — an attack miss emits no event, so the player sees nothing (this is what made the bug-hunt so confusing). Surface a miss/`AttackMissed` signal. Likely related to existing stream gaps.
  - **Devseed initiative non-determinism** — `wave-3-barbarian` initiative is random; agents force `char-bob → goblin-1` in Redis to test. Make the fixture deterministic (per `feedback_devseed_fixture_per_wave`).
- **rpg-api #586** — enforce depguard in GitHub CI (lint isn't a CI job today; the boundary guard is local-pre-commit-only).
- **rpg-toolkit #695** — SDK home for the Shield +5 AC magnitude (rule magnitude currently isolated in rpg-api's reaction adapter).
- **rpg-toolkit #694** — ADR-0031 + contract markers; now unblocked (re-dispatch with post-#689 signatures). Retro-refined: marker = drift-protection + definition-site pointer, not cold-reader discoverability.
- **rpg-dnd5e-web #425** — harness must drive the v2 RPCs through the game's shared hooks. The rage button is a **RESTORE** — **PR #420** (`feat/419-rage-button`) built it via the shared `useActivateFeatureV2` hook. **MERGE-READY:** proto re-pinned `92a9d062`→`99ba9c0` (commit `0aca473`, matches rpg-api server contract `v0.0.0-20260530184527-99ba9c0608f7`), `ci-check` green, all 5 CI checks SUCCESS, mergeStateStatus CLEAN, Copilot threads resolved. Body: `Closes #419` + `Part of #425`. **MERGED ✅** (squash `1377429`, 2026-06-01; #419 closed; branch deleted). Witness gate MET (rage verified through the game path). Door devseed (Interact) = rpg-api lane, dispatched separately.
- **Game-UI v1alpha1→v2 ActivateFeature parity** — the *game's* `LobbyView` still calls v1alpha1 `useActivateFeature` (harness now on v2). Full game-UI migration off the v1alpha1 encounter client is a separate larger effort (not #425) — file/scope at the retro.
- **Playtest caveats (minor):** (a) v2 stream surfaces no distinct `ResourceChanged` for the rage-charge decrement (only `StatusApplied`); (b) web console warn — `useEncounterStream2` unhandled event case on a zero-damage no-op reaction-check event (cosmetic reducer gap).
- **#691** (ActivateFeature self-load), **#692** (IsDirty beyond HP), **#693** (cross-RPC held-entity / the deferred reaction wire-pause), **#583** (`ci-check` wipes uncommitted). _(Note: #691/#692 are NOT load-bearing for a rage bug — rage works today. The design proposal `ideas/encounter/v1alpha2/gap-defensive-rage-persistence.md` explored a "one home for combat-mutable state" model; keep it as an architecture-honesty consideration for the retro, NOT as a bug fix.)_

## Decision log (2026-05-31 → 06-01)
1. Status-doc template — CLAUDE.md owns the shape. `3aa0d76`
2. Hydration contract → first-class docs. `3aa0d76`
3. #694 marker over interface (toolkit member pushback). `#694`
4. #694 sequenced behind #689. `#694`
5. Rage-resistance gate cleared → landed #689/#582-chunk-1 (#690 → `v0.17.0` → #584).
6. Chunk 2 = full carve-out, Sequencing B; EndTurn reaction wire-pause **deferred** (kept internal — Kirk). `#582`
7. All 7 verbs carved + Runner deleted + `combat` locked in depguard deny set (#585–#592).
8. **Goal sign-off (at-its-layer): v2 orchestrator path verified** (real v2 client, decoded — #684 cure holds); #582 closed at its layer. Caveat: not yet the harness-UI bar.
9. **Kirk's call (pause): see the slice end-to-end before the retro.** Retro gated on an *observed* UI-driven harness playtest, not merge-ready PRs. "Merge-ready" ≠ "I saw it work."
10. **Defensive-rage scare → RESOLVED as a director-introduced re-roll confound (NOT a bug).** A harness re-roll that had Bob skip his attack dropped rage (`endRage("no_combat_activity")`, correct 5e), so a subsequent goblin hit looked un-halved. A controlled test (Bob attacks → rage maintained → goblin hits) showed the halving works: `amount=1 [scimitar:1, dex:2, raging:0]`, Bob 14→13, Redis-confirmed. **Director owns the error:** the "preserve the goblin / hold the turn" instruction changed the variable under test (whether Bob was raging). The "false green" framing also over-reached — the integration test is *weak* (Skip hatches + un-representable multiplier) but the behavior it nominally covers is correct. Lesson added to `field-notes.md`.
11. **Process win:** the verification gate held — the controlled test ran BEFORE accepting the design proposal, so no persistence rework was built on a phantom bug. Caught via the design agent's re-trace (endRage-on-no-attack + the integration test passing because it attacks first) → a controlled, single-variable re-test.

12. **Chapter-1 retro RUN + #574 CLOSED (2026-06-01, OG session).** The defensive-rage "bug" was a director re-roll confound, not a bug (the gate held — controlled test before any fix). Goal verified end-to-end through the game path. Follow-ups filed + boarded: #696 (toolkit Multiplier), #593/#594/#595 (rpg-api). Combat-mutable-state one-home filed as **#596** (promoted, tracked) — promote-vs-archive is the one open Kirk call. **Chapter 1 (Architecture Honesty) is complete.**

Retro-as-evaluator principle captured in `director/field-notes.md` (`e9b6a37`): decide with the architecture as north star, log a retro criterion, judge in action.

## Pointers
- Board: #11 (Chapter 1, umbrella #574); #12 (Chapter 2: 4 Brothers — next).
- Design: `ideas/encounter/v1alpha2/` (`plans/{10,11}`); `gap-defensive-rage-persistence.md` (the "one home for combat-mutable state" exploration — architecture consideration, not a bug fix). Contract: `architecture.md → "The Persistence & Hydration Contract"`; toolkit ADR-0030 / journey-050.
- Orchestrator: `internal/orchestrators/encounter/v2/` (one `load` per verb; resolver adapters handler-side, depguard-excluded).
- Roles: `docs/teams/roles/<role>/{prompt,field-notes}.md`.
- Repo state: toolkit `main` (`v0.17.0`); rpg-api `main` (carve complete, `eaf4046`+). Playtest stack may still be up (api `:50051` / envoy `:8080` / vite `:3001` / chrome `:9222` / redis) — fine to tear down.
