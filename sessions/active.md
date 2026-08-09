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

---

# Encounter reset — play/clock SHIPPED (2026-08-09)

## Now

The encounter reset (rpg-toolkit journey 051) has its first shipped axis: **`play/clock` v0.1.0** is merged and tagged. The old `encounter` module is bugfix-only reference application from here; new investment goes to the `play/` family.

## Solid

- Kirk's 2026-08-08 audit judged old `encounter` "an application, not a toolbox" (behavior B+, architecture C; named diseases: dual state, bus-as-return-channel). Decision: clean-room rebuild as seven orthogonal axes, no code salvage. Journey 051 + full idea-doc triplet (brainstorm/design/plan with execution audit trail) merged via toolkit PR #901.
- `play/clock` merged via toolkit PR #905 (19 commits, 99.5% coverage): Turn (initiative bubble), Tick (player-driven world clock, high-water max-not-sum fairness), Milestone return values, atomic Transfer (DOS2 fall-in), ToData/LoadTurn/LoadTick, eleven-sentinel error vocabulary. Prerequisite `core.EntityID` landed as #902 (core v0.11.0). gorelease compat gate armed by the v0.1.0 tag.
- Process: subagent-driven execution, fresh implementer + two independent reviews per task; caught self-merge/self-transfer silent corruption, an attrition-save regression, and more pre-merge (full log: toolkit `docs/ideas/play/clock/plan.md` Execution addenda). Repo-wide find: golangci v1/v2 config silently inert — toolkit#904.

## Next

- Kirk picks axis two: **Record** (consumes the milestone streams Clock returns) or **Knowledge** (fog-of-war rebuilt right). Same design-first pattern (`docs/ideas/play/<axis>/` triplet).
- First production consumer: rpg-api composes `clock.Tick` for out-of-combat monster movement (the monster-AI world tick; friend's behavior work plugs in as a pure decider). Not started.
- Deferred by design: parallel `encounter/v2` vs scaffold-and-cutover; old-module sunset — both wait for a composition module.

## Pointers

- toolkit [#901](https://github.com/KirkDiggler/rpg-toolkit/pull/901) (journey 051 + triplet) · [#905](https://github.com/KirkDiggler/rpg-toolkit/pull/905) (module) · [#902](https://github.com/KirkDiggler/rpg-toolkit/pull/902) (core.EntityID) · [#904](https://github.com/KirkDiggler/rpg-toolkit/issues/904) (lint config)
