# Handoff: room-authoring checkpoint → style-driven walls

## Delivery mode for this push — Kirk's current direction

Time/token budget is critical. The parent supplies tight briefs, coordinates
seams, and verifies worker evidence rather than running elaborate ceremony.
Default implementation worker: `ollama/glm-5.3-flash:cloud`, in fresh context for
new slices, through the existing worker/subagent protocol. Do not change global
agent/provider configuration. Escalate concrete blockers, not speculative risks.

Deliver useful end-to-end slices and a working URL early. Kirk manually verifies
the important visible behavior. Keep focused tests for the changed behavior and
non-visible/data-loss risks, normal hooks and required CI. Batch the required
independent review at a useful PR checkpoint; no duplicate per-task/spec/quality
review seats or automatic minor-fix cycles. Defer nonblocking polish and exhaustive
coverage explicitly rather than holding the working tool for them. Ordinary
implementation details do not need another approval round; scope/data-risk/merge
and real blockers still belong with Kirk.

Next delivery priority is a faithful build → save → reopen → play loop, not more
editor polish unless a walk establishes a real need. Backend design still must
preserve complete authored meaning; the rejected lossy proposal is not revived.

## Latest planning checkpoint

**Repeat placement delivered:** [web#1083](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1083)
merged into dev as `950cdb295b9c886fb7f39e3c49bc30617eb4f6c1`; web#1080 is closed /
Done. Kirk accepted the look/group movement, then updated the base and enabled
auto-merge. Hosted checks passed at integrated `38e4bac9`; parent verified all
repeat-feature files unchanged from independently closed `e72f13fd`. Six Minor
findings were fixed; the cleanup-removal finding was rebutted and the reviewer
conceded. [Closure](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1083#pullrequestreview-5208124536).
The first proof used an already-cataloged barricade, not a new provider enrollment.
The bounds-report double scaling was fixed without changing rendered asset scale.

**Floor underlay reviewed:** [web#1089](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1089)
implements #1088 at `bcccb92de1557588e63d7c504228c8e7b5d66fe8`. Whole-workspace
basic floor is visual only, with corrected outline alignment and no walkability
or persistence change. Kirk accepted its look. All three review findings are
Addressed in [closure](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1089#pullrequestreview-5217547381);
local and hosted checks passed. Kirk offered to take it out of draft and merge;
parent recommended doing so, with a base update if required (GitHub last reported
behind). Do not call it merged until GitHub confirms.

Source/live :3030: `rpg-dnd5e-web/.worktrees/1088-floor-underlay`, branch
`concept/1088-floor-underlay`, reviewed head `bcccb92d`. API/Redis and drafts must
survive. Local recovery map: `/tmp/dungeon-authoring-floor/execution.json`.
No cleanup or backend/provider implementation has started.

## Start here after compaction

Kirk requested a compact before the next chapter. **Do not start wall work as
part of this handoff.** The current UI checkpoint is merged; the next chapter
needs fresh, narrowly scoped context rather than resuming a giant old worker.

**New intent, not yet a settled design:** Kirk dislikes the single visual style
of existing walls. He is importing multiple area-style asset sets, including
walls. He wants enclosing a space and placing/repeating those assets to feel
simple and repeatable, while retaining World Builder's free manipulation.
Explore style/kit selection and reusable wall/enclosure placement. Do not assume
one dungeon-wide style, compulsory regions, a particular draw-vs-stamp gesture,
or that all incoming assets have already been promoted. Kirk owns the ongoing
imports; do not duplicate or disturb that pipeline.

## What shipped

| Slice | Delivered / evidence |
|---|---|
| Spatial footprint geometry | toolkit #1747 / [PR #1749](https://github.com/KirkDiggler/rpg-toolkit/pull/1749), `tools/spatial/v0.14.0`: `FootprintPlacement`, `PlacedCoverage`, `TraceFootprint` |
| Shared sight lanes | toolkit #1751 / [PR #1752](https://github.com/KirkDiggler/rpg-toolkit/pull/1752), `tools/spatial/v0.15.0`: `SightLanes`; BasicRoom retains its established sight behavior |
| Room-authoring UI concept | web #1068 / [PR #1070](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1070), merged into **dev** as `5b0a0485d8b5c04fcb99170991a0526049efc711`; issue closed / Project 19 Done |

UI reviewed head: `645bde32e7c747b48af5c2cebed6ca29bb15ed2d`. Reviewed and merged
repository trees are identical (`bf9658befe5163ed28aafd3b83374f0e501a397c`). Final
local `npm run ci-check`, hosted checks, and post-merge CI passed.
[GLM closure](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1070#pullrequestreview-5204890648)
records all three findings Addressed: failed-load autosave data loss, no-op
history, and brush pointer ownership.

Kirk released first: web [PR #1072](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1072)
merged to main at 02:49:19Z on 2026-09-15 (`fa05f690`); our UI merged to dev at
02:52:26Z. At verification, main did not contain `roomDraft.ts`. Production
release/deployment was Kirk's operation; we verified Git ordering, not a fresh
production health check.

**The merge hold is historical:** Kirk subsequently performed the merge. There
is no pending merge and no standing permission to merge future PRs. No branch,
worktree, environment, or data cleanup was requested.

## Live environment — preserve it

- **UI:** http://localhost:3030/?concept=room-authoring
- **API:** http://localhost:8110
- Named stack: `local/dungeon-authoring`; manifest:
  `game-dev/envs/local/dungeon-authoring.env`.
- Web source currently is `rpg-dnd5e-web/.worktrees/1088-floor-underlay`, branch
  `concept/1088-floor-underlay`, local reviewed head `bcccb92d`. Runtime
  manifest/state remains authoritative if a subsequent handoff occurs.
  Older #1068 worktree is also preserved.
- API is an isolated **dev baseline**, not the parked toolkit consumer branch.
- Use **localhost**, not another hostname/port, for Kirk's browser-local drafts.
- Shared dev is **:3001/:8080**, not our feature environment. Do not disturb it
  or the separate organized-HUD stack (:3024/:8104).
- Named Redis is ephemeral (`RPG_REDIS_PERSIST=0`). The helper permits persistence
  only for default `dev`; setting 1 here caused an early startup failure and was
  corrected. Room-draft durability is browser storage/export-owned.
- Workspace commands run from `/home/kirk/game-dev`, e.g.
  `./scripts/dev-env.sh status local/dungeon-authoring`. Do not run them from a
  game-dev worktree. Do not restart/repoint/tear down the stack during compaction;
  plan any later branch/environment handoff deliberately, preserving data.

## What the UI actually does

One reused World Builder (`roomMode`), not a second editor:

- Paint/Erase walkable-ground declarations and Rectangle selection in world X/Z.
- Painted fills use shared pointy `hexCorners`; Rectangle previews complete hexes
  by centre-in-box, commits once on release, and cancels safely.
- Free prop placement, surface support, groups, arrangements, lights, transforms,
  explicit movement/LOS declarations and owner-local rectangular footprints.
- Separate room-draft persistence, stable ID remapping and combined undo/redo.
- Explicit **Expand workspace**: prototype steps are hex-radius/XZ-limit **6/12 →
  10/20 → 14/28**, maximum 631 candidate centres. Extent is editor capacity, not
  a region or world-layout rule. Expansion moves/paints nothing.
- Original composer/library stays at ±12; room-only widened validation is explicit.
  Other existing guards remain (Y 0–8, 200 props, 80 groups, 500k JSON).
- Room-only distance fog is disabled: old fog 15–31 hid the larger workspace at
  far zoom. Original composer atmosphere and game lighting remain unchanged.
- Failed stored-draft loads preserve raw bytes across StrictMode/effect replay
  until successful explicit recovery/reset/save/import intent; notices stay useful.

**Local room-draft JSON v2 is NOT canonical dungeon YAML v2.** It retains the
complete WorldScene and room/gameplay metadata, with explicit scene units/frame
and workspace. It migrates local v1 drafts safely. The existing engine
`dungeonspec` version 2 is a different, older region/cell-based format.

There is still **no canonical YAML export or Save & Play integration for this
new draft**. Its declarations are not yet consumed by gameplay collision/LOS/
cover. A wall tool, secret-room UI, region manager, and in-game pickup/drop are
also outside this UI slice. Existing game mechanics still work on legacy content;
do not rebuild them or describe the new concept as the finished dungeon builder.

## Parked backend work

[toolkit #1753](https://github.com/KirkDiggler/rpg-toolkit/issues/1753) is parked
(Todo), with **no implementation or PR**. Worktree
`rpg-toolkit/.worktrees/1753-encounter-footprints`, branch
`feat/1753-encounter-footprints`, clean at `1773806f`.

Its initial proposal was not approved: a visual-ref-only v3 would lose composition
pieces/supports/groups/lights. The now-real UI payload should drive the next
contract. Also, `spatial.Room.GetEntityPosition` returns cell coordinates, not
planar feet; do not relabel units or invent shadow anchors/fake per-hex entities.
A precise separate continuous-prop read was discussed, not implemented.

Agreed direction:
- Reuse existing `PutDungeon` validation/save, `GetDungeon`, and choose/play RPCs.
- Build the world first; paint walkability independently; annotate useful areas
  afterward. Do not make regions compulsory floor/prop containers.
- Current engine regions bundle lighting/style, concealment/search, membership
  queries, seating/placement and floor ownership. That coupling is not a new
  authoring requirement. Secret areas remain useful later.
- Prop standing policy should be adjustable: centre-covered initially; percentage
  coverage as an alternative. Centre uses stationary `TraceFootprint.Contact`,
  percentage uses `PlacedCoverage`; thin crossing is separate. Half cover is
  another future rulebook proof, not this percentage.
- Valid unused definitions are allowed. No automatic loss of recoverable data.

## Files and work records

- Parent journey: [rpg-project #169](https://github.com/KirkDiggler/rpg-project/issues/169).
- Cross-repo design/tracking [PR #446](https://github.com/KirkDiggler/rpg-project/pull/446)
  remains open. Owned project worktree: `rpg-project/.worktrees/169-world-builder-dungeons`,
  branch `idea/169-world-builder-dungeons`.
- Design home: `ideas/dungeon-authoring/world-builder/`. The original July
  `ideas/dungeon-authoring/{design.md,plan.md,visual-walkthrough.html}` is preserved
  historical material; do not overwrite or implement it as current truth.
- Current web: `src/concepts/world-building/{WorldBuildingConcept,
  WorldBuildingViewport,WorldBuildingInteraction,roomDraft,roomHexGeometry,
  sceneState,serialization}` and `src/concepts/ConceptsView.tsx`.
- Current merged UI is in dev: start the next line of work in a **fresh issue
  worktree from current origin/dev**, after checking instructions/board. Do not
  keep adding unrelated wall work to the completed #1068 branch.
- Old execution details: UI worktree `.superpowers/sdd/room-first-look/` and local
  `/tmp/dungeon-authoring-ui/{execution,merge-authority,review-state}.json`.
  These are recovery evidence, not a second project board.

## Working preferences and traps

- **Bring Kirk in early.** Real branch URL/new behavior before broad polishing.
  One writer, focused tests, one meaningful full gate at PR boundary, one bounded
  independent review; no automatic review/fix loops.
- Reviews use **GLM 5.3 Flash** (`glm-reviewer`,
  `ollama/glm-5.3-flash:cloud`) and
  `rpg-project/.agents/skills/pr-review/SKILL.md`. Publish individual findings as
  inline threads and a verdict on the PR; keep evidence claims honest.
- Web requires `npm run ci-check` before PR creation/update; do not run an extra
  standalone full suite first. Prettier scans ignored Markdown notes too: keep
  scratch outside the web tree or format those notes before the gate.
- Native async completion wakes the parent; no waiting/polling loops. Subagent
  infrastructure failures stop the lane: capture state/diff and recover through
  the same protocol, not an unapproved CLI-agent fallback.
- Toolkit: one nearest Go module per PR; serial provider release before consumer
  pins; real CI-issued tags, no hand tags or parallel dependent PR stack.
- Licensed source/GLBs remain local/private (`rpg-game-assets` canonical). Check
  the incoming styles and owning asset instructions before touching anything.
- Re-check Git/current source when returning. Do not copy shared policy into
  AGENTS or overwrite another line's `active.md` continuity.
