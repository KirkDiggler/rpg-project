# World Builder — current handoff

## Resume here

**Merged and cleaned up (Kirk authorized):** web PR1116 merged as
`11755d8ae0362130efc4d69bef0e1a5d95249c15`; API PR1004 merged as
`c49a64c12c3e4ed042bcdc26595e12ac914967df`. Removed17 clean owned worktrees
(including runtime source checkouts), three Vite previews, both dungeon stacks
(12containers/2networks/8volumes). Ports3030/3031/3032/8110/8111 are closed.
Verified Redis dumps, all volume archives, authored content, review artifacts and
original runtime evidence are preserved in game-dev
`.runtime/archives/world-builder-20260917T234755Z/` (README/result/checksums).
All33 unrelated containers were preserved; the authoring API image remains because
rpg-dev still uses it. No local branches deleted. **This project worktree is retained
because project PR446 remains OPEN.** Camera start-direction pointer/UI cleanup
are follow-ups. Older live-preview/pending-merge statements below are historical.

**Human acceptance confirmed at web86164eb9 / APId5dfed99:** Kirk confirms he can
load his saved room, roll dice, and start on his authored start hex. The slice
is functioning. DeepSeek's bounded follow-up fixed resumed-character selection,
truthful leave wording, and named/focused overwrite confirmation; parent restored
the omitted existing custom-dice preview bundle. Full web CI6778tests and hosted
checks pass; GLM verified the bounded browser surfaces. Explicit camera
start-direction pointer and other UI cleanup are follow-ups, not current blockers.
Both PR1116 and PR1004 remain OPEN/non-Draft/MERGEABLE; no merge authority given.
Complete preview3032 is pinned at86164eb9 (startup PGID2710141, verify before use).
Original3030/8110 and setup-only3031 remain untouched. Sections below retain the
historical delivery trail; their pending-walk statements are superseded here.

**Latest delivery gate:** complete web candidate [PR1116](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1116)
is pushed at `a5fd4ebd52f3269105dc68c7a637b18cf75344d1`; review closed and local/hosted
checks green, not merged. Whole review5237950102 found a real New-room overwrite
risk: the factory generated/stored a fresh draft but its in-memory commit retained
the previous identity/name. Corrected at ba830df3 with a red→green Save→New room→
same old key regression (new Get/confirmation, no second write on Cancel, complete
Undo/Redo). All four findings Addressed; scoped closure5238346399 is READY.
Final a5fd4ebd changes tests only: hosted CI exposed background validate-only
requests being mistaken for saves; reproduced deliberately before fixing the
mock, and genuinely held the launch promise. Full CI6776tests/490files pass
(1file/6tests skipped); hosted lint/type/test/security/deploy-preview all green.

API [PR1004](https://github.com/KirkDiggler/rpg-api/pull/1004) remains Ready/unmerged,
now at `d5dfed99480dac217fd8d450bf2703a10d141df1`. Current dev98b6d753 was merged;
only dependency files conflicted. Adopted its released root0.180/encounter0.88/
session0.95/proto v0.1.199 graph, retaining room presentation and upstream full-roll
transport. Required rich lifecycle suites, full API CI, normal hooks and hosted
checks pass. Config/stashes preserved. Recreated only isolated proof API8111;
every Redis/other container retained its identity/state. Fresh browser/public RPC
confirmed the same live session, exact original full scene and position (1,-5).
Receipt: `.runtime/local/dungeon-playtest/evidence/api-integrated-pins-proof.json`.

Parent browser proof retains the copied saved room (18props,4groups,139floorcells),
normally created Human Fighter and existing skeleton. Complete scene match,
movement/default Shortsword, explicit-altar crossing refusal, current→held LOS,
author-edit isolation and restart persistence were demonstrated. Durable proof:
game-dev `.runtime/local/dungeon-playtest/evidence/integrated-proof.md`.

**Next: Kirk's human walk and explicit merge decisions.** Complete Play preview
is live at `http://localhost:3032/?playerId=world-builder-play-proof`, detached
`.worktrees/1112-play-preview` at reviewed production ba830df3 (final web head
adds tests only), Vite PGID2166533 at startup. Verify actual PID/cwd before touching.
Own API8111/Redis retain the proof character (1/12HP after the reaction test),
session and content. Original3030/API8110 and setup-only3031 are unchanged. No
merge/deployment authority inferred; renew save/export checkpoint before replacing
any user-editing environment. Details below are the historical delivery trail.

**Current next slice: monsters + one playable authored room.** Kirk deferred doors
while making assets, accepted existing monster weapon/mind defaults, and explicitly
approved hex-snapped monster/party-start markers with freeform scenery.
[single-room-play.md](single-room-play.md) is **approved by Kirk**. The checked
[provider plan](single-room-play-provider-plan.md) shipped through
[toolkit#1798](https://github.com/KirkDiggler/rpg-toolkit/pull/1798), linked to toolkit#1753.
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
No browser Play claim: SDK bridge, API adoption and web consumers remain.
Kirk merged [protos#343](https://github.com/KirkDiggler/rpg-api-protos/pull/343)
at9dec1605; generated v0.1.198 carries RoomSceneJson (verified at tag6ea2b2e6).
Review5231667363 passed the source; parent corrected the PR-body signature/adoption
notes without changing a91e93e. SDK implementation is now toolkit#1804, below.
Progress ledger: that worktree's
`.superpowers/sdd/single-room-play-provider-plan/progress.md`.
Implementation mission `c506ca66-5582-4e18-a53b-3cc3068b867b` is scoped to the
writer worktree. Review mission `ee4560fc-9588-4537-8a49-0a3494b64fca` used the
separate `.worktrees/1753-room-review`; whole review, scoped closure and integration
passes are complete. Latest retained reviewer: `af174328-24ec-4809-93b6-0f8c919d6c56`
from workflow `ed0cb7d3-7d11-4360-9ec7-69cf6c26a72b`. Those provider workers are complete.
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
Current-ref map49a8fb95 is complete. Parent corrected two important scout claims:
API StartEncounter already checks actual-party capacity before writes; SDK
`types.go`/`projectAtlas` drops RoomScene, so pinning encounter alone is insufficient.
Kirk approved the bounded **session-module** bridge: [plan](single-room-play-session-plan.md),
[toolkit#1804](https://github.com/KirkDiggler/rpg-toolkit/issues/1804), worktree
`.worktrees/1804-session-room-scene`, branch `feat/1804-session-room-scene`.
Kirk merged [SDK PR1805](https://github.com/KirkDiggler/rpg-toolkit/pull/1805)
at `bea10c1b6837e39601f46f24d7025faeba6926b9`; published sessionv0.94.0 resolves
to that merge. Final accepted head was11d63b81. Whole review5231856375
required missing public SDK regressions; closure5231922264 verified the new real
preview/live/reloaded-records suite at707afff1. Parent then fixed only formatting
and the missing internal-test license header at11d63b81; local module checks and
exact-head hosted CI are green. All review threads have explicit dispositions;
parent readiness comment5709910590 records completion. No extra review loop.
Workflows8fc87967/d77adfcb are complete (mission08f29cc0, that worktree).
API adoption is now [API#1003](https://github.com/KirkDiggler/rpg-api/issues/1003),
[plan](single-room-play-api-plan.md), branch `feat/1003-world-builder-play` in
`rpg-api/.worktrees/1003-world-builder-play`, base9aa9422b (dev). Parent baseline
affected-package tests/format checks passed and normal hooks are active. Initial
workflowf8942b1c stopped at the existing linter/toolchain mismatch (childc26b46ac);
no commit/PR/review. Patch+untracked files are backed up under game-dev
`.runtime/local/dungeon-authoring/checkpoints/api1003-toolchain-20260917T083633Z/`.
Compatible official/checksummed linter2.4.0 now lives only at
`.runtime/tools/golangci-lint/2.4.0/golangci-lint-2.4.0-linux-amd64`; use scoped PATH,
not the unchanged global2.3.1 binary. A clean detached baseline reproduces four
pre-existing lint warnings; only minimal behavior-preserving lint cleanup is scoped.
Parent also found the first launch test used an empty scene and omitted the promised
reload/snapshot/capacity proof despite passing by name. Completion brief is durable
at `.runtime/local/dungeon-authoring/api1003-completion.md` in game-dev.
GLM completion9637f8dc finished API functionality/gates but exposed a real hook
isolation bug: `ci-checks.test.sh` inherited linked-worktree Git context and its
scratch fixture changed the feature ref/index and shared repo config. Worker
restored its feature base; parent backed up/restored proven common-config damage.
A later malformed worker identity-read command wrote `--show-origin` as identity;
parent proved/removes only that exact addition. Worker was restricted to source
and scratch tests; **parent owns real Git publication** after these incidents.
Durable recovery notes: `.runtime/local/dungeon-authoring/api1003-hook-recovery.md`.

Parent safely published [API PR1004](https://github.com/KirkDiggler/rpg-api/pull/1004)
at `423e7cd8b74203f575ef75e1727641a1c8fc422e` after normal hooks/CI passed. Hook
repair now clears complete Git-local context, fails closed on discovery failure,
and permanently tests both primary and linked-worktree metadata/bytes safety.
Parent also preserved seeder nil behavior and corrected staged-fixture EOFs.
Commit parent/tree matched exactly; common config, stash and unrelated refs were
unchanged; post-push config changed only the intended feature upstream. Evidence:
`.runtime/local/dungeon-authoring/checkpoints/api1003-publication-20260917T093739Z/`.

**API PR1004 is Ready, not merged**, at423e7cd8. Independent review5233922810
(child31de8f9f, workflow271678a8) found no Critical/Important findings. Its own
mutation probes killed broken scene carriage, ByRef preflight and cellOf conversion;
real public suites and hook safety checks passed. The sole Minor wording note was
explicitly Declined for this slice in reply4035436811 (correct named refusal;
no optional polish cycle). Parent verified hosted build/test green, clean worktree
and CLEAN/MERGEABLE exact head; PR is non-Draft/Project19 In Review.

**Web consumer is now implementing milestone1** under [web#1112](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1112),
UI/UX / The Dungeon / Build. Worktree `.worktrees/1112-world-builder-play`, branch
`feat/1112-world-builder-play`, basewebdev3a0ad05c. Parent installed deps/verified
normal Husky hook, typecheck and four focused baseline suites. Grounded
[setup plan](single-room-play-web-setup-plan.md) corrects scout ambiguities: no
client legality gate, no second compiled-preview pane, omitted absent start, and
no fictional pending-dungeon route (AuthorView already creates/readies/starts the
lobby and calls App). Shared worldToCube already rounds; render HEX_SIZE=1.

Initial workflow018e95f3 failed criterion1: Luna wrote a partial codec patch but
left all setup UI/history unimplemented. Parent preserved it at game-dev
`.runtime/local/dungeon-authoring/checkpoints/web1112-m1-20260917T102137Z/` and found
real codec gaps (order-sensitive play comparison, presence/type/bounds/size checks,
missing fixed-legacy/storage proof and unrelated helper/comment churn).
M1 is now published in [Draft web PR1116](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1116)
at `5b3c2b49205b845f27cfc0415000700f4ff929cd` (feature919cc2f4 plus latestdev ec833650).
Retained completion7f289386 finished97 focused tests/types/lint/format after the
30min timeout. Parent real-browser proof then caught a raw skinned-clone bug:
the mesh stayed at origin while the ring moved. Parent replaced it with the
existing ClassCharacterModel (skeleton-safe clone/calibration/idle), added a
red→green renderer-reuse guard and verified actual model/ring alignment.
One full ci-check at published head passed:485 files/6704 tests,1 file/6 tests skipped.
Full transcript inspected. M1 files unchanged through upstream integration;
proto dependency nowv0.1.199 from dev. No independent full-feature review yet.

**Pinned early setup preview: http://localhost:3031/** (Home→World Builder).
Source `.worktrees/1112-setup-preview`, detached at5b3c2b49; PGID1881252/node1881270
(verify live identity before acting). Writer stays in the separate1112 worktree.
API binding is set on Vite to8110. Original3030 remains height3438f64. Port3031 is
a separate browser origin: use exported JSON or an existing world snapshot, not
an assumption that3030 local storage is shared. M1 has no Play yet.
Durable process/evidence under game-dev `.runtime/local/dungeon-authoring/setup-preview/`.
Fresh-browser proof at the exact pinned head: richv2 upgrade/oldbytes unchanged,
skeleton0,1→move-1,2, start2,0, Undo/Redo/remove/clear/Undo, reloadexact,
invalidcurrentv3protected; no pageerrors/failedmodels. Existing private runtime
models copied only to ignored local paths; two upstream-promoted assets copied
with matching SHA receipts, no generation/sync/promotion.

M2 planning214e820a is complete; parent corrected unsafe scout assumptions:
empty proto string means legacy absence; canonical presentation has only frame/
workspace/scene, not declarations or actors; no arbitrary workshop-room key default.
[Runtime substep](single-room-play-web-runtime-plan.md) is implemented in the
writer's uncommitted patch (child57b4386e/workflow6aab2c73). Parent inspected it,
removed an unnecessary already-parsed-object decoder path (wire string or absence
only, red→green test), then ran six focused suites174tests plus typecheck green.
Invalid CURRENT nonempty JSON blocks the cached old map instead of rendering it
as valid; empty string gets legacy behavior. Shared props/floor/light leaves and
member actors remain in their established roles. Checkpoint before the next writer:
`.runtime/local/dungeon-authoring/checkpoints/web1112-runtime-before-launch/`.

[Save/launch substep](single-room-play-web-launch-plan.md) first run757b399f
(child5fe30a5a) ended with an unfinished sentence/no acceptance report. Parent
verified it had added only the shared-hook draft, not the actual UI/route wiring
or tests. Patch saved under game-dev `.runtime/local/dungeon-authoring/checkpoints/web1112-launch-partial-20260917T125106Z/`.
Retained GLM completionce9c5367 (childa11bb1e0) implemented the UI/shared flow
and reported360 focused tests green. Parent added three deferred component probes
and reproduced required bugs: an old save launches a newly selected character;
a same-ID source change still permits stale YAML saving; a new room retires the
request but strands busy state. All three are red in
`/tmp/dungeon-authoring-playable-room/web1112/launch-parent-races-red.log`.
Bounded fix e6a6dd87 completed (childc634b83b): full source/key/character/client/
mount retirement, owned cleanup and child-launch cancellation. Parent red cases
and related hook suites39tests passed; real browser subsequently found the
StrictMode live-lifetime bug, fixed with another red→green regression (40tests).
It preserves the runtime patch and adds explicit root-key/collision
handling, generation-fenced exact source save and shared existing AuthorView lobby
launch with Home-selected character. No key field in RoomDraft or arbitrary
workshop-room default; no stale redirect/source replacement. Parent owns later
safe local integration, full PR gate, publication and first whole-feature review.
Both3031 setup preview and original3030/API8110 remain unchanged.
Parent prepared a SEPARATE proof stack `rpg-local--dungeon-playtest` on loopback8111,
APIimage from clean423e7cd8, own Redis and own authored-content bind (not the
existing deployment's shared content). API healthSERVING; all prior container IDs/
running states preserved, no cache prune/seed/reset. Metadata/commands/evidence:
`.runtime/local/dungeon-playtest/state.json` in game-dev. Candidate3032 ran against
8111 for the completed full-room proof, then its exact owned Vite group was stopped
for publication. The isolated API/Redis remain. See the latest delivery gate above.
Leave3031 pinned for Kirk's edits. Original durable M1 brief remains
`.runtime/local/dungeon-authoring/web1112-m1-completion.md`.
Worker owns only source/focused tests; **parent owns all real Git publication and services**.
Parent completed M1 browser proof, its one full ci-check and Draft publication;
do not repeat that full gate before the next changed-head PR boundary. One branch/PR continues into milestone2 Save/Play and full
runtime rendering; independent feature review comes after complete scope. No
live3030/8110 change or asset pipeline mutation. API merge remains Kirk's decision.
Kirk freed HDD space and directed continuation;26GB/home was verified free, no
cleanup was performed. Live3030/8110 remain untouched.
Go proto dependency is the published v0.1.198 generated commit6ea2b2e6, which
resolves as v0.0.0-20260917055155-6ea2b2e6dffc. No local replacements needed.
Next consume that API result, then web adoption/local walking; no additional merge
authority is implied and the restored live stack remains unchanged.
Only session module is writable in that lane; encounter and other modules stay
read-only. RoomSceneJSON preserves the canonical scene without leaking an inner
type through S2. No rules or duplicate scene DTOs. API must use version-neutral
Load + Compiled.Key/Name and keep the existing capacity gate and offset conversion.
Use encounterv0.87.0/protov0.1.198; consumer branches may be exercised locally
before merge. No hand tags, cleanup or further merge authority.

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
**Kirk added DeepSeek and recommends it as an option where Sol would normally be
used.** It was not yet listed by this session's native model inventory when that
preference was recorded; recheck the exact registered provider/id before selecting
it. Do not guess an ID, stall available work or alter global model configuration.
This does not change Luna as the default writer.

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
