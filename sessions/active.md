# Active handoff — 2026-05-31: clean rpg-api encounter path (Chapter 1: Architecture Honesty)

> Shape defined in `CLAUDE.md → Status docs`. This is the living handoff — **rewritten, not appended**. Full narrative history lives in git.

## Now
Chapter 1 (Architecture Honesty) — board #11, umbrella rpg-api #574. Carving the clean **v1alpha2 encounter path** through rpg-api: a dedicated `internal/orchestrators/encounter/v2` (one private `load(id)` per method, ~20-line thin handlers, toolkit owns all rules + drives events). Director runs thin — **commission + verify, no hands-on work** (`docs/teams/roles/director/{prompt,field-notes}.md`).

## Solid (verified — keep, don't re-derive)
- **Design ratified + merged** (rpg-project PR #53): `ideas/encounter/v1alpha2/plans/{10-689-encounter-hydration-cascade,11-582-encounter-orchestrator}.md`.
- **Persistence/Hydration contract is now first-class doc** — `docs/architecture.md → "The Persistence & Hydration Contract"` + `boundaries.md`. Key truths: `ToData`/`LoadFromData` is a *convention, not an interface*; `LoadFromData` returns a **live, bus-subscribed** entity (not a plain unmarshal); **load-once = subscribe-once = the #684 cure**. Grounded in a verified read-only toolkit audit — the toolkit boundary holds (no proto / persistence / orchestration imports).
- **toolkit #689 (PR #690) MERGED** (21:07Z) — the encounter hydration cascade. CI auto-cut **`github.com/KirkDiggler/rpg-toolkit/encounter v0.17.0`** (commit `71fe432`; prior `v0.16.0`).
- **RESOLVED — rage resistance halves correctly.** Verified at the v1alpha2 API layer, decoded evidence, deterministic roller: goblin → raging barbarian **8 → 4** (raging ×0.5, `dnd5e:abilities:dex` tag); non-raging 8. The prior "unhalved = 7" was a **setup flaw** (toolkit checked out on `main` = no cascade; rpg-api doesn't even compile there), **NOT a regression**. A genuine v2 no-skip test exists and passes: `internal/handlers/dnd5e/v2/encounter/integration_barbarian_rage_test.go::TestIntegration_RageResistance_HalvesGoblinDamage`.

## Open questions
- None blocking — the rage-resistance gate cleared this session.

## Next
1. **In flight:** rpg-api `feat/582-adopt-689-cascade` (**chunk 1 of #582**) — `replace` stripped, bumped to `encounter/v0.17.0`; PR opening; awaiting Copilot/CI → **director merge**. (rpg-api member, background.)
2. **Chunk 2 (the carve-out proper):** build `internal/orchestrators/encounter/v2`, move verbs off the Runner (Sequencing B, per-RPC, delete old path per-verb), retire the Runner — per `plans/11`. Gated on chunk 1 merging.
3. **#694 (toolkit, parked):** ADR-0031 + per-package contract markers. Re-dispatch the toolkit member with **post-#689 signatures** once chunk 1 lands. Retro-refined: the marker's real value is **drift-protection + a definition-site pointer**, not cold-reader discoverability (the ADR + brain-doc cross-link does that). Was blocked-by #690 (now merged).

## Decision log (2026-05-31)
1. **Status-doc template** — CLAUDE.md owns the shape (table of contents vs. live state); this file follows it. `commit 3aa0d76`
2. **Hydration contract → first-class docs** — the false start was our docs hiding the contract, not toolkit drift; promoted into architecture.md + boundaries.md. `commit 3aa0d76`
3. **#694 design: marker over interface** — toolkit member pushed back (LoadFromData is a constructor; ToData carries a `SyncErr` side-channel; injected hydrator already rejected in journey-050 / ADR-0030). → ADR-0031 + `var _` markers. `issue #694`
4. **#694 sequenced behind #689** — contract shape, ADR number, and marked files all arrive with #689; don't document a transitional state. `logged on #694`
5. **Gate cleared → land the #689/#582 unit.** Order: merge #690 ✓ → tag `v0.17.0` ✓ → rpg-api bump + strip replace (in flight) → merge #582 chunk 1.

Operating principle captured this session → `director/field-notes.md` (`commit e9b6a37`): **the retro is the evaluator** — decide with the architecture as north star, log a retro criterion, judge in action; don't pre-validate the ideal, and don't loop Kirk for "is this the ideal shape?" sign-off.

## Pointers
- **Board:** #11 (Chapter 1, umbrella #574); #12 (Chapter 2: 4 Brothers — next, rides the clean rails).
- **Design (don't re-derive):** `ideas/encounter/v1alpha2/`; carve-out plans `plans/{10,11}`.
- **Contract:** `docs/architecture.md → "The Persistence & Hydration Contract"`; toolkit ADR-0030 / journey-050 (+ ADR-0031 when #694 lands).
- **Roles:** `docs/teams/roles/<role>/{prompt,field-notes}.md`.
- **Repo state:** toolkit on `main` (`v0.17.0` tagged); rpg-api on `feat/582-adopt-689-cascade`.
- **Follow-ups under the wave:** toolkit #691 (ActivateFeature self-load), #692 (IsDirty beyond HP), #693 (cross-RPC ApplyAttackOutcome held entity); rpg-api #583 (`make ci-check` runs `git checkout -- .` → commit before ci-check).
