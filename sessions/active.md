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

`play/clock` v0.1.0 shipped 2026-08-09 — toolkit [#901](https://github.com/KirkDiggler/rpg-toolkit/pull/901) (journey 051 + design triplet), [#905](https://github.com/KirkDiggler/rpg-toolkit/pull/905) (module), [#902](https://github.com/KirkDiggler/rpg-toolkit/pull/902) (core.EntityID); the v0.1.0 tag armed the gorelease gate. Old `encounter` is bugfix-only. **Next:** Kirk picks axis two (Record or Knowledge), same `docs/ideas/play/<axis>/` triplet pattern; first consumer — rpg-api composing `clock.Tick` for monster free-roam — not started. Deep record lives in toolkit `docs/journey/051-*` and `docs/ideas/play/clock/` (all decisions, reversals, execution addenda).
