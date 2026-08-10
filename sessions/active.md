# Active handoff — 2026-08-09

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

Axis two SHIPPED 2026-08-10: `play/record` v0.1.0 ([#907](https://github.com/KirkDiggler/rpg-toolkit/pull/907)) and `play/intel` v0.1.0 ([#908](https://github.com/KirkDiggler/rpg-toolkit/pull/908)) merged + tagged; docs PR [#906](https://github.com/KirkDiggler/rpg-toolkit/pull/906) merged as ratification (triplets + execution addenda, incl. the `intel.Data` namesake clause). Family now: clock + intel + record, all v0.1.0, gorelease gates armed. `play/clock` v0.1.0 shipped 2026-08-09 (toolkit #901/#905/#902). Old `encounter` bugfix-only, delete-and-tag-pin agreed. **Next:** axis three — field (= `tools/spatial`, kept as-is per journey 051; maintenance issues filed for Platform: toolkit [#909](https://github.com/KirkDiggler/rpg-toolkit/issues/909) orchestrator index bus-dependency, [#910](https://github.com/KirkDiggler/rpg-toolkit/issues/910) housekeeping sweep — one module, batch as one PR; deferred until consumers exist: bus posture, per-channel occlusion, wire format) and interruption (suspension-as-value seed answered in-session; event-bus evaluation done 2026-08-10, feeds the interrupt brainstorm); first consumer — rpg-api composing `clock.Tick` for free-roam — not started. Deep record: toolkit `docs/journey/051-*`, `docs/ideas/play/{clock,intel,record}/`.
