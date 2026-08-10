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

Axis two SHIPPED 2026-08-10: `play/record` v0.1.0 ([#907](https://github.com/KirkDiggler/rpg-toolkit/pull/907)) and `play/intel` v0.1.0 ([#908](https://github.com/KirkDiggler/rpg-toolkit/pull/908)) merged + tagged; docs PR [#906](https://github.com/KirkDiggler/rpg-toolkit/pull/906) merged as ratification (triplets + execution addenda, incl. the `intel.Data` namesake clause). Family now: clock + intel + record, all v0.1.0, gorelease gates armed. `play/clock` v0.1.0 shipped 2026-08-09 (toolkit #901/#905/#902). Old `encounter` bugfix-only, delete-and-tag-pin agreed. **Next:** axis three — field (= `tools/spatial`, kept as-is per journey 051; Platform maintenance: toolkit [#909](https://github.com/KirkDiggler/rpg-toolkit/issues/909) is implemented independently on [PR #913](https://github.com/KirkDiggler/rpg-toolkit/pull/913), review-ready at `64746d7` (managed membership values, observer-only bus, race removed, independent review + Copilot + CI green; Kirk merge pending). [#910](https://github.com/KirkDiggler/rpg-toolkit/issues/910) remains a separate housekeeping sweep; deferred until consumers exist: broader bus posture, per-channel occlusion, wire format) and interruption — **BUILT 2026-08-10: `play/interrupt` on toolkit [PR #914](https://github.com/KirkDiggler/rpg-toolkit/pull/914), CI green (47 tests, all four gorelease gates), awaiting Kirk review/merge + tag `play/interrupt/v0.1.0`**; triplet + full execution addenda on [PR #912](https://github.com/KirkDiggler/rpg-toolkit/pull/912) (merges as ratification — two in-build design amendments: Answer envelope = ownership transfer; R9 refinements incl. next_id-1 rejection). Ledger of open windows; custody-not-execution; no timeouts v1; Shield fixture scene proven in AC1; OA-with-option first production consumer (playable roster monk/fighter/rogue/barbarian, **level 1 only, leveling unimplemented** — Protection fighting style also available at L1). Bus evaluation ratified as journey 052 (toolkit PR #911, in review). **Next:** the **resolution axis** — rulebook-side suspendable phase machines consuming interrupt (checkpoints + enumeration per journey 052). First consumer — rpg-api composing `clock.Tick` for free-roam — not started. Deep record: toolkit `docs/journey/051-*`, `docs/ideas/play/{clock,intel,record,interrupt}/`.
