# NPC activity integration — execution plan

**Goal:** configure a thoughtful prisoner in the studio and preserve a clean
exit → visible walk → return-to-default sequence in encounters.
**Authority:** [design](design.md), [walkthrough](README.md), story #711; R9
selects completion-driven presentation, not body-only timing.
**Execution:** direct, task-by-task under build-slice; no implementer fanout.
**Finish line:** integrated local early view, focused seam checks, required PR
boundary gates, and review/merge handoff. Merges and release remain operator-owned.

## Constraints and baseline

- Existing knowledge/authority updates remain immediate. Only presentation waits.
- One presentation cursor, one movement owner; no independent gameplay queue.
- Unseen/remembered/catch-up motion must not replay; session changes and renderer
  disappearance must release/cancel presentation waits rather than deadlock.
- The known folded-thumb repeat seam and published activity compatibility remain
  provider release work, not hidden by runtime fallback.
- Assets checkpoint: private PR #329, preserved at `84a6d2a` (no harness change).
- Web implementation baseline: dev `a9889339`, worktree `.worktrees/npc-activities`.
  Focused baseline: 52 tests passed across pacing, beat queue, move controller,
  and `useHexMovePath`.
- Full story requires provider/default-ref transport as well as web playback.
  The executable task contract below is the first integration unit; later units
  receive equally concrete contracts after their current provider sources are
  inspected. This document does not pretend those units are already build-ready.

## Sequence

1. **Completion-driven walk presentation** (executable contract below).
2. **Provider activity metadata and release candidates:** promote supported
   activities without silently breaking existing appearance/style/skin custody.
   Carry explicitly supported appearances and named failures, not clip-name guesses.
3. **Authored default identity:** placement codec → compiled content → launch/
   persistence → captured observation → roster → additive proto → thin API adapter.
   Preserve default absence semantics and observation snapshots.
4. **Studio selection and body sequencer:** consume provider/default identity,
   gate visible route advancement on walking readiness, restore the configured
   default, and apply shared presentation speed.
5. **Integrated early view and release handoff:** thoughtful prisoner save/load,
   ordinary movement, remembered/unseen lifecycle; no release until operator look.

Development can proceed on pushed provider commits where required; consumers
remain draft until successful provider tags/pins replace those development pins.
One toolkit module per PR. No phase completion is the whole story's completion.

## Task 1: Completion-driven walk presentation

**Delivers:** R4/R5/R7/R9's presentation seam before adding new clip selection.
The existing remote walk is released by the story cursor; its actual completion
holds subsequent paced story/turn items. World-paced walks use the same completion
path without inventing turn announcements. Local player's RPC route remains on
its current command path.

**Owner:** rpg-dnd5e-web, session presentation and hex movement.
**Prerequisites:** existing Act walk paths, move sequence/reached acknowledgements,
`ClassCharacterModel` legacy idle/walk, current dev dependencies.

**Files / interfaces:**
- New `src/components/session/walkPresentation.ts`: `PresentWalk` callback accepts
  `{act: Act, complete(outcome): void}`; outcomes are `painted`, `unavailable`,
  `cancelled`; returns an unsubscribe function. Presentation-only, no RPC.
- `combat-experience/useCombatStoryPacing.ts`: consume `session`, `onPresentWalk`
  and host-derived `presentableMembers`; queued motion has terminal lifecycle
  validity, so disappearance/reappearance cannot revive an old route;
  expose `heldMovementMembers: ReadonlySet<string>` alongside story/result/notice.
  Waiting is fenced by session/member scope and exact active request. No timer
  substitutes for a presented walk's completion. The existing 300 ms rhythm can
  remain a gap between completed items, never a completion guess.
- `monsterBeatQueue.ts`: distinguish turn and world presentation. World walks
  are ordered without `playing`/`passed`/idle-turn notices. Existing fight
  classification remains the authority for turn announcements.
- `useMoveController.ts`: make `beginRoute` return the allocated sequence
  synchronously, retaining the pure `moveController` state transformations.
  Scope routes to session/member; `forgetRoute(member, seq)` discards only the
  exact unavailable route. Keep sequence counters after discard within a scope
  so an old acknowledgement cannot match a later reappearance.
- New `useWalkPresentation.ts`: bridge the cursor request to `beginRoute` and
  match `member/seq/reached` completion; skip unavailable/unseen/empty routes;
  cancel subscriptions on scope changes and release when presentation disappears.
- `combat-experience/useSessionCombatExperience.ts`: forward the callback/session
  and expose the hold set; it does not reimplement movement ownership.
- `SessionEncounterView.tsx`: remove eager remote route dispatch from the
  delivered-act funnel; retain immediate knowledge/authority updates. Connect
  the walk bridge and completion acknowledgement; pass the hold set to canvas.
- `SessionCanvas.tsx`, `HexEntity.tsx`, `useHexMovePath.ts`: add
  `deferPositionReconciliation` for live bodies with queued/active walks. Prevent
  newer authoritative snapshots from snapping an already displayed body ahead
  of its queued route. Initial mounting still places the body normally; active
  interpolation remains owned by the movement hook; remembered bodies bypass
  this hold and never replay stale motion. The same held-member set protects
  pending watcher-zone overlays; do not let an overlay jump ahead while its
  body is queued but has not started interpolating.

**Behavior / lifecycle:**
- Walk begins only when released by the existing cursor. A completed exit will
  later gate the same movement hook's advancement, not create another route.
- Later story/turn rows stay hidden while the exact walk is outstanding.
- New requests do not reset the active completion gate. Stale callbacks do not
  release another walk or another session/member's queue.
- Non-live catch-up does not animate. World walkers receive no fake combat turn.
- Loss of visibility/presentation settles the active wait as unavailable and
  permanently discards eligible queued motion. Its story position remains;
  reappearance requires a fresh walk. Actual scene-error/canvas availability,
  not a cached last-good scene alone, supplies this lifecycle fact.
- Initial appearance is not animation from an unknown prior location.

**Tests and checks:**
- Extend `useCombatStoryPacing.test.tsx`: pending walk holds the following attack
  and turn end despite advancing timers; exact completion releases them; a world
  walk waits without turn notices; stale completion after session/member change
  is fenced; synchronous unavailable completion cannot wedge the cursor; catch-up
  does not invoke the renderer.
- New `useWalkPresentation.test.tsx`: exact sequence/end-of-route completion,
  stale/partial acknowledgement refusal, unseen/empty/unavailable skip, lifecycle
  cancellation and no delayed replay.
- New/extended `useMoveController` hook check: two begins in one React batch
  allocate distinct sequences synchronously.
- Extend `useHexMovePath.test.ts`: a pending presented walk holds current pixels
  while authoritative position changes; initial mount still positions; release
  restores ordinary reconciliation; real route starts from held pixels.
- Update `SessionEncounterView.test.tsx`/canvas seam assertions: authoritative
  updates occur on delivery, remote movement goes through presentation callback,
  completion reaches the bridge, held members reach the correct live entities.

- [x] Write focused failing tests against existing behavior.
- [x] Implement the callback/hold/lifecycle seam without changing rule authority.
- [x] Run focused tests and TypeScript build; boot a dedicated early-view web.
- [x] Report the head/visible behavior for operator look before walk/review.
- [x] Run PR-boundary `npm run ci-check`, record limitations, commit and publish
  the slice PR linked to its issue. Keep it draft if integration/pins are incomplete.

**Commands:** from the web worktree, `npm run test:run --` the named test files;
`npm run typecheck`; PR boundary `npm run ci-check`. Test outputs, source head,
and early-view address belong to the slice issue.

## Coverage and seam checks

| Requirement | Unit | Concrete proof |
|---|---|---|
| Durable authored work | Preservation #328 / PR #329 | hash-bound donors/models, portable lab, compatibility refusal |
| Body/story/turn completion order | Task 1 | walk blocks later rows/turn marker until exact acknowledgement |
| Authority remains immediate | Task 1 | delivery still applies knowledge and invalidation before presentation |
| No stale/unseen replay | Task 1 | scope, unavailable, remembered and catch-up tests |
| No pixel snap before queued walk | Task 1 | held-position hook and canvas seam checks |
| Authored thoughtful default | Units 2–4 | provider, save/load, observation and wire contracts still to implement |
| Exit/readiness and shared speed | Unit 4 | production body/movement adapter, not the private lab alone |
| User-visible finish line | Unit 5 | studio → encounter → walk → thoughtful walkthrough |

| Provider | Consumer | Produced / consumed contract | Availability | Proof |
|---|---|---|---|---|
| Delivered Act run | Existing story cursor | `Act`, live/catch-up metadata | Existing | queue tests |
| Story cursor | Walk bridge | `PresentWalk` request + scoped completion/unsubscribe | Task 1 | delayed/stale/synchronous completion tests |
| Move controller | Walk bridge | allocated sequence and painted route progress | Task 1 | batched allocation and exact acknowledgement |
| Cursor hold set | Canvas / movement hook | live-member `deferPositionReconciliation` | Task 1 | snapshots cannot jump ahead of queued presentation |
| Activity adapter | Movement readiness | deferred until Unit 4 | Not built | named as remaining, not claimed by Task 1 |

**Plan check:** the existing appearance pattern supplies the intended content and
observation route but not the new field; no test or prototype result is treated
as that integration. Task 1 preserves its owner seams and removes eager remote
movement rather than leaving two dispatch paths. Remaining provider/transport
units are explicitly unfinished, not delegated to guesses inside this task.
