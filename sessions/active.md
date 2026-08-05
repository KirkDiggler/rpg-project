# Active handoff — 2026-08-04

## Now

Documentation-only reconciliation for Dungeon Builder #175/#180 on PR #181. The docs gate is in progress under the approved early-merge exception; implementation has not started in this scope.

## Solid

- Backend history #176–#179 is delivered evidence: [rpg-api-protos#206](https://github.com/KirkDiggler/rpg-api-protos/pull/206); [rpg-toolkit#881](https://github.com/KirkDiggler/rpg-toolkit/pull/881) and tags `tools/spatial/v0.6.0`, `rulebooks/dnd5e/v0.71.0`, `encounter/v0.48.0`; [rpg-api#769](https://github.com/KirkDiggler/rpg-api/pull/769); delivery evidence [comment 5183123668](https://github.com/KirkDiggler/rpg-project/issues/179#issuecomment-5183123668). The GitHub issues remain OPEN; they are not reopened or rewritten. Web is independently owned.
- #180 is now the semantic-scope wave: flat absolute region extents, derived containment/index, root/unpainted area, optional archetype inheritance, persistence checks, authoring projection, and fog-safe `Zone.parent_id`.
- Canvas is a separate Wave 0 prerequisite: source YAML supplies positive `canvas` dimensions, origin `[0,0]`, half-open bounds; toolkit derives complete odd-q structural floor, and authoring projection exposes canonical cells and edges.
- Physical multi-option locks use OR semantics and are outside Wave 0/#180 acceptance; executable fixtures use unlocked doors. Lock behavior is a later #175 follow-up.
- Web implementation is independently owned and excluded from #180.

## Open questions / gates

- Independent gate must verify the exact local diff and GitHub bodies/comments, including the v0.2 permalink and one PR closing keyword.
- No canvas/#180 implementation or implementation-test evidence is claimed.

## Next

Independent gate review, then Kirk decides whether to commit, push, and merge PR #181 under the recorded exception. This dispatch leaves changes uncommitted and unpushed.

## Decision log

- 2026-08-04 — settlement authority is Specimen Pack v0.2 comment `5185751479` plus the rewritten #175 and #180 bodies; earlier platform comments `5185403172`/`5185403199` were reconciliation proposals only.
- 2026-08-04 — WORK SESSION STARTED `5185733969` records the approved PR #181 early-merge exception; implementation remains a later dispatch.

## Pointers

- [#175](https://github.com/KirkDiggler/rpg-project/issues/175) · [#180](https://github.com/KirkDiggler/rpg-project/issues/180) · [PR #181](https://github.com/KirkDiggler/rpg-project/pull/181)
- `ideas/dungeon-builder/{design,plan}.md`
