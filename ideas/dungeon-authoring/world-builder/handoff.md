# World Builder — current handoff

## Resume here

**Current next slice: monsters + one playable authored room.** Kirk deferred doors
while making assets, accepted existing monster weapon/mind defaults, and explicitly
approved hex-snapped monster/party-start markers with freeform scenery.
[single-room-play.md](single-room-play.md) is **approved by Kirk**. The checked
[provider plan](single-room-play-provider-plan.md) is published; toolkit#1753 is
In Progress with [Draft toolkit#1798](https://github.com/KirkDiggler/rpg-toolkit/pull/1798).
All four provider milestones are implemented and reviewed at
`f13570448b9f24176122107ad89fd726593aee85` in
`rpg-toolkit/.worktrees/1753-encounter-footprints`, branch
`feat/1753-encounter-footprints`, base3a9dbf6e. Parent runtime/load/member-atlas
and v3 compiler probes pass; hosted required checks green. Source validation,
shared footprints, full compile and presentation persistence are present.
**Kirk merged PR1798** at `3e20eaa1a318827d034d32d47d0d19b2830c92b9`;
released `rulebooks/dnd5e/encounter/v0.87.0` resolves to that merge. Whole review
5230677406, closure5231061232 and main-integration verification5231155443 are
published. The Important source-path error was fixed at37f8f937; Minor follow-ups
are explicitly tracked in toolkit#1803. Kirk confirmed fixed Crypt floor/style,
no theme selector (contract16d0ff6). Main's Persuade update was merged into the
feature branch atf1357044, retaining both reserve standing and new memberFacts
validation; module/race/lint/parent probes and hosted checks passed.
No browser Play claim: proto/API/session adoption and web consumers remain.
Progress ledger: that worktree's
`.superpowers/sdd/single-room-play-provider-plan/progress.md`.
Implementation mission `c506ca66-5582-4e18-a53b-3cc3068b867b` is scoped to the
writer worktree. Review mission `ee4560fc-9588-4537-8a49-0a3494b64fca` used the
separate `.worktrees/1753-room-review`; whole review, scoped closure and integration
passes are complete. Latest retained reviewer: `af174328-24ec-4809-93b6-0f8c919d6c56`
from workflow `ed0cb7d3-7d11-4360-9ec7-69cf6c26a72b`. No active workers.
Reports are bound under home-directory subagent-artifacts; temporary recovery
map `/tmp/dungeon-authoring-playable-room/execution.json` names current evidence.
Parent corrected draft source shapes/frame math and plan module boundaries,
existing PartyStart/Load contracts, and C1 monster-resolution ownership.

Height shipped in [web#1105](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1105)
via Kirk's auto-merge at `258b180501812d4d0e5c01d50d4a00654dae8563`;
issue1103 is Closed / Project19 Done. Reviewed height head `3438f64b` had no
Critical/Important findings; owner base update `c3d307b4` and merge retained all
height feature files. Optional notes are in #1106. Kirk's verdict: “works great”.
Height uses numeric percent + Apply, single Undo, group-aware selection and
complete persistence; Shift-click remains sufficient (Ctrl-click was dropped).

**Workspace delivery landed.** With Kirk's explicit approval,
[web PR#1099](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1099) merged into dev
at `de29612c12fbd703169cb958e45231a0e47c5558` (2026-09-16 09:24:05Z).
[web#1097](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1097) is Closed /
Project19 Done, UI/UX / The Dungeon / Build. No active workers.

Normal Home → World Builder opens Rooms, retaining Prop compositions through the
same editor. Publication additionally fixed Back-to-main confirmation. Review
5220658824 found no Critical/Important findings; five Minor threads are explicitly
deferred to Project19 Todo [web#1100](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1100).
Local gate and hosted checks passed; reviewed and merged trees are identical.

- Publication worktree: `/home/kirk/game-dev/rpg-dnd5e-web/.worktrees/1097-world-builder-publication`
- Branch: `feat/1097-world-builder-workspace`
- Published/reviewed head: `2f5692ecc06b3bcb6db3f1fdcc95bf239795cc0c`
- Last configured preview: `/home/kirk/game-dev/rpg-dnd5e-web/.worktrees/1103-piece-height`,
  detached at `3438f64bfda06a27ec2ce1ca03ea2e51d7f64a47`; includes Back protection
  and height. Feature branch lives in `.worktrees/1103-height-publication`.
- Temporary `/tmp/dungeon-authoring-*` recovery notes/evidence **expired between
  sessions**. Don't assume old paths exist or infer game data loss from their
  absence. Canonical Git docs, GitHub reviews and home-directory subagent
  artifacts remain the durable records. Current temporary design record is
  `/tmp/dungeon-authoring-playable-room/execution.json`.

**Immediate next step:** Implement the remaining consumers for an early local
integrated walk; Kirk explicitly chose local-first, not a hosted deployment gate.
A read-only current-ref map is running (workflow49a8fb95, missionde121643, cwdgame-dev)
before owning issues/worktrees and scoped implementation. Use encounterv0.87.0;
consumer branches may be exercised locally before their PRs merge. No hand tags,
cleanup, further merge authority or re-approval of the existing design.

**Local restoration checkpoint:** Kirk confirmed the room saved/exported. Resumed
only the existing six `rpg-local--dungeon-authoring` containers with their identities
unchanged; no recreation, reseeding, asset sync or other-stack starts. :8110 health
SERVING and :3030 World Builder rendered in a fresh browser. Web still pinned to
height3438f64; Vite PGID567088 (verify liveness before acting). Durable evidence:
`.runtime/local/dungeon-authoring/checkpoints/restore-20260917T052754Z/` in game-dev.
This restores the existing authoring tool, **not** the pending Play feature. Renew
the save/export checkpoint before a later handoff if Kirk has made new edits. Do not redo shipped UI or revive the
lossy1753 proposal. No cleanup, new merge or environment-change authority.

## Delivery style — current human direction

Time/token budget matters. Parent owns scope, precise prompts, integration and
verification. **Luna first** (`worker` with explicit
`openai-codex/gpt-5.6-luna:medium`), fresh context for new slices. GLM 5.3 Flash is
available for focused review/alternate work. Sol/Astra are not routine coders.
Kirk also has other Ollama models; verify exact registry IDs before using them,
and don't stall available work to configure optional models or alter globals.

Working URL + Kirk's hands-on acceptance come early. Keep essential data-safety
regressions, normal hooks and required CI. No duplicate per-task/spec/quality
review seats, automatic minor-fix cycles, or exhaustive-polish detours. Defer
nonblocking work explicitly. Ordinary implementation details don't need repeated
approval; scope, data-risk, merge and concrete blockers still belong with Kirk.

## What #1097 contains and what was verified

Only one mode's editor is mounted, keyed by mode. Switching has explicit
**Switch editor / Cancel switch** confirmation. Local mode drafts and shared
arrangements survive round trips; world-edit Cancel preserves in-memory changes.
The app route fills width/height, uses internal panel scrolling, and has World
Builder rather than First Look chrome. Concepts and the legacy playable Dungeon
Builder remain available until the new tool has gameplay parity.

Evidence:
- `/tmp/dungeon-authoring-workspace/execution.json` — recovery map.
- `/tmp/dungeon-authoring-workspace/recovery/fixed-browser/partial-before-api-host.json`
  — same-page 1600×1000 → 960×640, no document overflow, hidden-delete and shared
  arrangement checks passed at current code.
- `/tmp/dungeon-authoring-workspace/recovery/world-leave/receipt.json` and
  `runner.json` — actual API8110 binding, existing snapshot read, Cancel preserves
  unsaved world edits/local bytes, no API writes/errors, owned server stopped.
- Parent verified normal entry on3030: `vite-handoff-1097/browser-ready.json` and
  `3030-workspace.png` under the same local task directory.
- Final full PR gate passed at2f5692ec; hosted checks green, focused reviewer tests
  passed. Logs and dispositions are under `/tmp/dungeon-authoring-workspace/publication/`.
  An earlier gate caught an App navigation assertion that needed the new explicit
  confirmation; that test was updated before the final changed-head gate.

Important scope limit: the browser proof establishes mode Cancel, not every
possible navigation/unload path. Do not claim comprehensive navigation safety.

## Live environment — preserve it

- UI **http://localhost:3030/**; API **http://localhost:8110**.
- Named stack `local/dungeon-authoring`; root manifest
  `/home/kirk/game-dev/envs/local/dungeon-authoring.env`.
- Runtime authority: `/home/kirk/game-dev/.runtime/local/dungeon-authoring/state.env`.
  Last manifest read: WEB_SOURCE is `1103-piece-height`, VITE_PGID `2193450`.
  Verify actual process identity/liveness before any operation; these are not
  promises that a process survives a session restart.
- API/Redis containers were not restarted during handoff; IDs/start times/restart
  counts matched during the height Vite-only handoff. Its former temporary
  rollback/proof files were under `/tmp/dungeon-authoring-height/`; see the
  expiration warning above.
- **Before another live reload/source switch, have Kirk save/export unsaved
  world-origin edits.** Browser-local bytes don't protect unsaved in-memory edits
  to an opened world snapshot.
- Keep hostname/port: localhost3030 owns his local drafts. Named Redis is
  ephemeral (`RPG_REDIS_PERSIST=0`); world snapshots do not survive an environment
  reset by promise. Do not run `dev-env up/down` just to change web code.
- Workspace commands run from `/home/kirk/game-dev`. Shared dev3001/8080 and other
  stacks are not ours. No worktree/branch/data cleanup requested.

## Delivery record

- Spatial geometry: toolkit#1749, `tools/spatial/v0.14.0` — continuous footprint
  placement/coverage/trace. Sight lanes: toolkit#1752, `tools/spatial/v0.15.0`.
- Room editor: web#1070 → dev `5b0a0485`; issue1068 closed. Free placement,
  groups/supports/lights, walkability paint/rectangle, local footprints/flags,
  safe local storage/history. Workspace presets6/12 →10/20 →14/28 (631 centres),
  editor capacity only. Original composer ±12 preserved; room fog removed.
- Repeat: web#1083 → dev `950cdb29`; issue1080 closed. Whole copies along a free
  X/Z line become ordinary props/group with one Undo, not a live run object.
- Floor: web#1089 → dev `3da02a55`; issue1088 closed. Existing basic floor is a
  continuous whole-workspace underlay, separate from walkability; aligned border,
  world-anchored UVs, loading-safe pointer target. Optional future floor pieces
  can reveal that underlay through gaps.
- Room save/reopen: web#1093 → dev `e7bdc058`; issue1090 closed. Typed
  `{kind:'room-authoring-draft',version:1,draft:RoomDraft}` snapshots through existing
  CompositionService opaque-JSON storage. Full draft retained, room docs excluded
  from prop palettes, stale opens fenced, local autosave restored by successful
  local Save/Reload/Import, oversized saves refused. No new API/toolkit/RPC.
  [Closure](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1093#pullrequestreview-5218832284).
- [web#1094](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1094) holds deferred
  snapshot-label/copy/coverage notes. Not on the critical path.

**These are room authoring drafts, NOT playable DungeonSpec.** Local RoomDraft v2
and canonical dungeon YAML v2 are different contracts. Composition storage accepts
opaque JSON; existing PutDungeon compiles strict canonical YAML before writing.
Don't claim Publish/Play, movement/LOS enforcement or gameplay lighting is wired.

## Next product sequence

1. Workspace and grounded visual height shipped (#1099, #1105).
2. **Now:** approve the single-room contract, then real monster placement using
   existing defaults, party start and faithful Publish → Play. Whole scene remains
   document-local; the approved architecture already rejects ephemeral-only
   composition IDs as the sole room copy. Monster AI Mind is not faction knowledge
   Mind. Runtime actors remain cell-native; do not invent continuous spawns.
3. Doors deferred while Kirk makes/bakes assets. Preserve prior findings for
   later: encounter supports one door over multiple edges, old DoorSpec only one
   crossing, and encounter.CloseDoor lacks the current SDK/API exposure.
4. Concealment/intel, multi-room traversal and #1094/#1100/#1106 polish remain off
   the critical path. No placeholder asset or provider work.

World first, optional named areas later. No forced region-first placement, fake
per-hex entities, shadow anchors or lossy visual-ref-only export. Keep complete
pieces/transforms/groups/supports/lights and explicit gameplay declarations.

Toolkit#1753 has been resumed under the approved complete-room contract; its
previously clean worktree was fast-forwarded from1773806f to3a9dbf6e before work.
Do not revive its rejected lossy proposal. `Room.GetEntityPosition` is cell-native, not planar feet.
Existing PutDungeon(validate_only)/PutDungeon/GetDungeon and choose/play are the
preferred future seams; a new RPC needs a concrete reason.

## Recovery lessons — don't repeat these

- Parent now mechanically prepares each writer's locked dependencies, normal
  Husky support and baseline typecheck. The initial1090 Luna commit lacked setup;
  its “pre-existing errors / hooks ran” claims were corrected, history preserved.
- Keeping both editors mounted and hiding one with CSS was unsafe: Rooms Delete
  deleted the selected hidden prop and autosaved that loss. Isolated before/after
  evidence is in `recovery/confirmed-blocker/`; current single-active design fixes it.
- The large GLM browser harness continued after entry failure and was paused.
  **Do not resume it:** run0594f016-d4f5-4bf4-829c-fcfde03b5bfd. No active fleet now.
- The last missing-fixture failure was a launcher error: API_HOST was set on the
  probe, not Vite. Fixture existed. Parent's `run-world-leave.py` sets server env,
  verifies actual API binding, fails fast and stops only its owned process group.
- First-look worker: `f84b8bed-b429-4461-b6ea-f0438c9f0d94` (Luna), completed mission
  `fbffc886-4842-4542-8a2c-325d9e000b7c` belongs to the preview worktree. Publication
  used worker `dd090fd1-f7c8-4fd3-aa83-fda5b6fad5ab` and independent reviewer
  `f304388c-089e-4324-ba2d-01108ea0dc17`; its completed mission
  `76bbb3ac-f00d-463b-83ea-37add41c380c` belongs to the publication worktree.
  Mission IDs are cwd-scoped: cross-worktree attachment fails before launch.
  Check actual resumability/status instead of copying an old mission blindly.
- Native async completion wakes the parent: no polling loops. Infrastructure
  failures require exact failure/state evidence and same-protocol recovery, not
  unapproved agent CLI/foreground fallback. One writer per worktree.
- Workflow inline output may append a human receipt: don't parse the whole string
  as JSON. Use explicit machine payload/structured output or bound artifact.
  Omit/normalize undefined optional fields before JSON emit/return.
- Keep scratch outside web, or format ignored Markdown before ci-check. Never
  bypass hooks. One full gate at PR boundary, not redundant full suites.

Cross-repo design/tracking remains [project PR#446](https://github.com/KirkDiggler/rpg-project/pull/446),
branch `idea/169-world-builder-dungeons` in its named project worktree. Original
July design is historical; current design home is this world-builder directory.
Kirk owns ongoing licensed asset ingestion. Never commit licensed GLBs publicly,
change global runtime configuration, or overwrite another task's active.md.
