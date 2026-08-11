# Active handoff — 2026-08-10

## Now

Wave 0 is LIVE VERIFIED. Wave 1 (#180) is exploratory semantic-scope work; implementation has not started.

## Solid

- Wave 0 delivered the Go proto contract as generated root `v0.1.120` / `ba7eda1f1833`, toolkit `encounter/v0.49.1`, and rpg-api#771; verification is recorded on [#192](https://github.com/KirkDiggler/rpg-project/issues/192#issuecomment-5206548237).
- Wave 1 runs structurally usable scope graphs without archetype cardinality/content/spawn enforcement. Missing semantic conventions are follow-up findings only where an existing surface permits; no diagnostics proto/API is added.
- rpg-api-protos#210 is a separate TypeScript/GitHub Release publication defect, not a Go gate. #772 (API update durability) and #885 (toolkit documentation) are separate.

## Next

Focused consistency review of PR #200; the prior independent review is invalidated. Do not cut Wave 1 implementation issues until this course correction is accepted.

## Pointers

- [#180](https://github.com/KirkDiggler/rpg-project/issues/180) · [PR #200](https://github.com/KirkDiggler/rpg-project/pull/200) · [#192 verification](https://github.com/KirkDiggler/rpg-project/issues/192#issuecomment-5206548237) · [rpg-api-protos#210](https://github.com/KirkDiggler/rpg-api-protos/issues/210)

## Lane: encounter reset (play/ family)

**Shipped through 2026-08-11:** `play/{clock,intel,record,interrupt}` all v0.1.0 merged+tagged (docs PRs #906/#911/#912 merged as ratification; journey 052 = bus keeps observation, discovery moves to enumeration). Spatial maintenance shipped by Platform (`tools/spatial/v0.9.0`). **The composition is live: `rulebooks/dnd5e/encounter` v0.1.0** (toolkit #921 merged+tagged 2026-08-11; triplet #918 merged; own module, laws C1–C8, tomb-watch scene, verified-transcript pattern + freeroam workbench + generic `scripts/verify.sh`).

**Wave 2 SHIPPED 2026-08-11:** toolkit [PR #924](https://github.com/KirkDiggler/rpg-toolkit/pull/924) merged + tagged **`rulebooks/dnd5e/encounter/v0.2.0`** (tag verified → b51d0c0); triplet #923 merged as ratification. Platform notified on rpg-api#793 — their dependency signal is green; wiring can proceed. Post-build pre-tag rounds all absorbed: Copilot (grid persists as string; endpoint presence required), Kirk's axial catch (hex = AxialHexGrid, cube math, identity projection), Platform review (nil decider reattachment holds; integral axial at every hex seam — semantic-owner fix filed as toolkit#926 for spatial ingress). Final: 288 tests, five review rounds. Delivers: connection endpoints (nine-defect validation both seams), per-room `GridShape` (hex first-class — answers Platform's square-grid pushback; spatial's bounded HexGrid validity ≡ square's, gridless is the discriminating signal), the `Traverse` verb (TransitionEntity+PlaceEntity, limbo window proven unreachable under four documented invariants), `Decide(Snapshot{Room,Position,Holdings})` + `IntentTraverse` (pursuit through doorways; found+fixed a REAL Pump aliasing bug — planned list held member value copies), the vault-chase scene, `Example_theTraverse`, two-room workbench. 259 tests; 2 sonnet + 2 Opus review rounds, ~50 mutants/probes; `gorelease` verdict = v0.2.0 exactly (one ratified breaking change: Decider interface). Triplet [PR #923](https://github.com/KirkDiggler/rpg-toolkit/pull/923) open with full execution addenda — merges as ratification. Newly pinned laws: full-tick-then-evaluate closure; ending eval is decision-order across actions / declaration-order within one; reject-never-crash covers reentrant deciders. Follow-up noted (not filed): intel `Refreshed` delta ordering nondeterministic (never observable in blobs/transcripts).

**Wiring handed to Platform:** rpg-api [#793](https://github.com/KirkDiggler/rpg-api/issues/793) — free-roam composition as a coexisting encounter type. Kirk's target refinement ON the issue: acceptance = the LOCAL dev loop (#791 override + sandboxseed), `main` stays deployed/frozen, dev may break; no parity (no doors), no dungeon-builder coupling (0.4 unverified / 0.3 unplayable), minimal direct spec; zero proto changes expected (v1alpha2 `EncounterMode.FREE_ROAM` already live; wire hex + per-hex VISIBLE/REMEMBERED ≅ intel ghosts). Local-override allowlist needs a `rulebooks/dnd5e/encounter` target. Depends on the v0.2.0 tag for multi-room.

**Next:** the resolution axis, toolkit [#916](https://github.com/KirkDiggler/rpg-toolkit/issues/916) (suspendable MOVEMENT + OA-with-option via play/interrupt; design-first). Kirk's words: "if I have fable, we should do the hard things" — read as ours now, originally routed Platform; confirm at pickup. Combat wave (encounter chaining) rides it. Monster-behavior contributor loop: buddy's ooze (damage slice {dice,type}) rides the decider seam; onboarding docs PR #920 in flight (another session).

Deep record: toolkit `docs/ideas/encounter-transitions/` (the wave-2 triplet + addenda), `docs/ideas/encounter/`, `docs/journey/051+052`.
