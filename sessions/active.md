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

`play/clock` v0.1.0 shipped 2026-08-09 (toolkit #901/#905/#902). Axis two built 2026-08-09: `play/record` (the story log) on [#907](https://github.com/KirkDiggler/rpg-toolkit/pull/907) and `play/intel` (the belief store) on [#908](https://github.com/KirkDiggler/rpg-toolkit/pull/908) — both open ready-for-review, CI green; docs PR [#906](https://github.com/KirkDiggler/rpg-toolkit/pull/906) (both triplets + execution addenda, incl. the namesake-collision naming clause `intel.Data`) merges alongside as ratification. Old `encounter` is bugfix-only, delete-and-tag-pin agreed. **Next:** Kirk reviews/merges #907 + #908 + #906, then tags `play/record/v0.1.0` and `play/intel/v0.1.0` (second-merged rebases a one-hunk compat.yml conflict); axis three candidates: field (the stage) or interruption (suspension-as-value seed answered in-session); first consumer — rpg-api composing `clock.Tick` for free-roam — not started. Deep record: toolkit `docs/journey/051-*`, `docs/ideas/play/{clock,intel,record}/`.
