# Counted say activities — implementation plan

**Authority:** [story #746](https://github.com/KirkDiggler/rpg-project/issues/746), including the operator's response-time/count agreement and [discard ruling](https://github.com/KirkDiggler/rpg-project/issues/746#issuecomment-6536601319).
**Goal:** configure an NPC response's activity and sustain count in Studio, trigger the response in Play, see it play and return to default.
**Architecture:** carry the cue alongside the selected answer through the existing record/act path. Rendering owns execution through the existing sequencer and mixer; neither the selection algorithm nor gameplay waits for it.
**Execution:** direct implementation, failing-first focused tests, one bounded independent review round at the appropriate release boundaries. No implementer delegation authorized. No new merge authorization.
**Finish:** real Studio save/reopen/launch and two-client visible response demonstration; local/hosted gates and linked PRs; operator acceptance and explicit merge authorization remain required. Do not stop at a provider-only unit result.

## Constraints and decisions

- R1: start when the selected NPC response appears, not on the question click.
- R2: N is a positive whole sustain-cycle count; Enter/Exit do not count.
- R3: return to the configured default through the existing transition machinery. If already at that activity, do not manufacture a visible exit/re-entry merely to return to the same pose.
- R4: a newer request wins; Enter/Exit complete. Movement discards unfinished cycles permanently. Arrival returns to default, never the interrupted response.
- R5: preserve one presentation cursor, one mixer owner and one position writer. No second gameplay/animation queue, guessed timers or text-to-animation lookup.
- Existing visibility/lifecycle fences apply: no catch-up animation, remembered-body animation or revival after surface/session loss; stale callbacks cannot affect a new request.
- Only published, provider-supported nonmoving activities are offered. Unsupported imported references remain named diagnostics, not substitutions. No new licensed assets or rig changes are needed.
- Routes, anticipatory timing and indefinite response poses are out of scope.
- Builder choices: omission means no cue; present cue requires its reference and count. Count is bounded by the wire's uint32 representation (1..4294967295), not an arbitrary UI limit. A repeated delivery is not a new cue; a new response is.

## Inspected baseline and ownership

Web `43a8e089` (dev), toolkit `ceda8ddd` (main), protos `025e905` (main), API dev `69478de`. Reconcile actual heads before opening PRs. Toolkit encounter now declares Go 1.27; do not downgrade it to avoid adoption work. Record required toolchain/provider pin updates when the actual module graph is consumed.

| Owner | Contract | Refusal |
|---|---|---|
| Studio | authored answer's optional `activity: {ref, cycles}` | no guessed cue from text; invalid partial cues cannot be published |
| mind/behavior | optional typed cue on the picked Answer | no asset catalog, playback or new selection weight |
| encounter/dungeonspec | strict content lowering and blob/record carriage | no omission across save/reload; no invented activity |
| session | captured cue on TableAnswer | no reread of current content to enrich an old act |
| protos/API | additive typed output and literal conversion | no clip timing, eligibility or new rules in API |
| web presentation | reveal cue with the actual response; live/visible lifecycle | no RPC-only trigger, catch-up replay or hidden-body animation |
| existing sequencer/mixer | counted sustain, cancellation, return | no duration estimate; movement remains the sole position writer |

## Shared seam contract

Authored YAML/JSON: `activity: {ref: "dnd5e:activities:thoughtful", cycles: 2}` beside the NPC answer's `say`. Omitted activity leaves existing behavior unchanged. Empty reference, missing count, zero, negative, fraction, overflow, unknown fields and a cue without a spoken line are rejected structurally. Unknown nonempty activity references remain opaque to the engine and are diagnosed against the actual appearance provider in the consumer.

Proposed Go value `behavior.ActivityCue { Ref string; Cycles uint32 }`; `Answer.Activity *ActivityCue`. Encounter owns explicit persisted `ActivityCueData` and dialect `ActivityCueSpec` with `ref`/`cycles`, and copies through nested rows/bundles. The selected `answered` record has optional `activity: {ref,cycles}`. Session exposes its own value `ActivityCue { Ref string; Cycles uint32 }` on `TableAnswer.Activity`.

Proto adds `ActivityCue { string ref = 1; uint32 cycles = 2; }` and `TableAnswer.activity = 12`. Existing field numbers remain unchanged. API maps the optional message literally in `tableAnswerToProto`, including the nested `Speech.answer` use.

Proposed client request `ActivityCueRequest { id: string; activityRef: string; cycles: number }`. Identity comes from the delivered act/segment, never the text or a wall clock. The sequencer accepts `requestCue(request, returnActivityRef)`; a regular request/stop/reset discards a pending cue. Use native mixer repetition/completion boundaries for finite sustain. New same-activity requests get a fresh full-cycle budget without skipping an in-progress Enter/Exit. Return to the same default sustains without an unnecessary transition. Final method details must preserve these contracts, not create a second scheduler.

## Tasks and proofs

### T1 — web authoring contract and counted playback

**Owner:** web; no new SDK required for the isolated units.
**Files:** modify `src/concepts/world-building/answerTableShape.ts`, `SitePolicies.tsx`, `sitePolicyEdits.ts` only if needed for immutable carriage; `src/npc-activities/activityTypes.ts`, `activitySequencer.ts`, `activityPlayback.ts`. New tests `sayActivityShape.test.ts`, `SayActivityFields.test.tsx`, `activityCuePlayback.test.ts`; extend existing sequencer tests.
**Interfaces:** authored activity shape above; existing `validateAnswerRows`, `AnswerEntryRow`; new cue request above. UI provides an optional activity picker plus positive count beside Say. Clearing the activity removes the whole cue, not the line; ordinary row edits preserve it. Preserve undo/redo and JSON/YAML round-trip through the existing document store.
**Behavior/proofs:** `carries a cue through answer validation`; `rejects malformed cues by field`; `ordinary rows remain unchanged`; `edits and clears a cue without changing Say`; `counts two full sustain cycles, excluding Enter/Exit`; `returns to default`; `movement discards remaining cycles`; `new response replaces the count`; `same default does not force exit`; `stop/reset and stale completion cannot revive a cue`. Real THREE mixer tests must include a large delta and single-cycle completion, not just hand-called loop callbacks. Do not retain count state in a second animation owner.
- [x] Add and run failing shape/playback tests.
- [x] Implement shape, controls and existing-mixer extension.
- [x] Run focused tests, document outcomes and commit a meaningful checkpoint.
**Checks:** from web worktree, `npm ci`; `npm run test:run -- src/concepts/world-building/sayActivityShape.test.ts src/concepts/world-building/SayActivityFields.test.tsx src/npc-activities/activityCuePlayback.test.ts src/npc-activities/activitySequencer.test.ts src/npc-activities/activityPlayback.test.ts`; full `npm run ci-check` at PR boundary.
**Evidence:** observed pre-change rejection/missing behavior, passing focused assertions; full UI integration remains T6, not claimed by these units.

### T2 — published wire contract

**Owner:** protos. **Prerequisite:** T1's consumer contract; no dependency on toolkit implementation.
**Files:** `dnd5e/api/session/v1alpha2/act.proto`, `docs/architecture/components/session-service.md` (alongside the observed default activity contract; the similarly named presentation-service document owns dice throws, not acts).
**Interface:** ActivityCue and optional TableAnswer field 12 above. Document absence, positive count, scope, cancellation and provider ownership; no enum of asset names.
- [x] Add additive contract and owning documentation.
- [x] `make format && make test`; inspect additive diff; commit/push PR, verify CI generation/SDK validation.
- [x] Operator merged #410; published v0.1.248/d3deef84da30 adopted in consumers.
**Evidence:** contract gates and generated-SDK CI, not bespoke tests of protobuf serialization. This is the normal proto early-publication exception, not authorization to merge other providers.

### T3 — rulebook-neutral selected-answer carrier

**Owner:** toolkit `mind/behavior`, separate worktree/PR/module.
**Files:** `mind/behavior/answer.go`, `doc.go`; new `activity.go`, `activity_test.go`; existing group/table tests if serialization golden changes.
**Interface:** Answer.Activity and ActivityCue above. Selection returns the authored cue exactly, with unchanged choice arithmetic. Groups carry cues on selected leaf rows, not a second competing group action. Validate required line/reference/positive count; no known-animation registry.
- [x] Failing suite cases cover selected weighted/nested picks, required fields and legacy absence (ActivitySuite); legacy compiled absence regression added when encounter goldens exposed null leakage.
- [x] Implement, run `go test -race ./...` and repository lint gate in this module, update godoc, push a provider checkpoint.
**Evidence:** selection and malformed-input assertions; published commit available for downstream `go get`, not a local replace.

### T4 — content, persistence and observed record

**Owner:** toolkit encounter, existing `say-activity-encounter` worktree; consume T3's pushed commit.
**Files:** `rulebooks/dnd5e/encounter/dungeonspec/spec.go`, `compile.go`, the same `AnswerSpec.UnmarshalYAML` strict decoder used when v5 JSON is read as YAML; `data.go`, `answer.go`, `table.go`, `doc.go`; new `dungeonspec/say_activity_test.go`, `say_activity_test.go`.
**Interface:** typed authored/persisted cue; optional selected `answered.activity`. Carry through both options and nested row conversions, preserving existing witness audiences and omitted old blobs. Emit only the picked cue, not candidates' cues or current live content.
- [x] Failing tests cover YAML/JSON-v5 compile/roundtrip, malformed input, selected direct/nested response after JSON reload, absent unselected cue and existing record audience. Files: `dungeonspec/say_activity_test.go`, `say_activity_test.go`.
- [x] Adopt pushed behavior commit, implement, `go mod tidy`, focused packages then full module race/lint gates.
**Checks:** module cwd `go test ./dungeonspec -run SayActivity`; `go test . -run SayActivity`; `go test -race ./...`; `golangci-lint run ./...` from that module (repository `make lint` only covers Core/Events, so it is not this gate).
**Evidence:** reload tests are mandatory; castle-only tests do not exercise the API's every-verb blob round trip. Push actual provider commit for T5/T6.

### T5 — session act carriage

**Owner:** toolkit session, separate worktree/PR/module; consume T4 pushed commit.
**Files:** `rulebooks/dnd5e/session/act_bodies.go`, `act_family.go`, `doc.go`; `act_fold_internal_test.go`, `acts_test.go`.
**Interface:** session ActivityCue/TableAnswer.Activity above, copied exclusively from the captured record, including nested Speech.Answer. Absence remains nil.
- [x] Failing assertions in `say_activity_test.go` and `say_activity_internal_test.go`: actual Speak-produced act after mutating live authored data, old record absence, exact ref/count and malformed-record rejection. `beat_decode.go` owns strict nested cue decoding.
- [x] Adopt actual provider pin, implement decode/fold, focused act tests then complete module race/lint gates; push.
**Checks:** module cwd `go test . -run 'TestActFoldSuite/TestSayActivity|TestSpeakWorldSuite/TestSayActivity'`; `go test -race ./...` and owning lint gate.
**Evidence:** real Speak-to-Act seam, not only a manually constructed DTO.

### T6 — joined API, Studio launch and live presentation

**Owner:** API and web, separate repository PRs. **Prerequisites:** published SDK from T2; actual pushed T4/T5 pins; T1 UI/playback.
**API files:** `internal/handlers/dnd5e/session/v1alpha2/convert_act.go`, `convert_act_test.go`; `go.mod`, `go.sum`, Docker toolchain if required; a small untracked `content/say-activity-walk.yaml` for the isolated walkthrough plus a committed engine fixture/regression. The existing canonical castle receives an authored example through the owning encounter provider before its API copy is synchronized; the operator's running castle is not edited/restarted.
**Web files:** `package.json`/lock; `src/components/session/combat-experience/useCombatStoryPacing.ts`, `useSessionCombatExperience.ts`, `types.ts` as needed; `SessionEncounterView.tsx`, `SessionCanvas.tsx`, `HexEntity.tsx`, `ClassCharacterModel.tsx`; new `src/npc-activities/sayActivityPresentation.ts` with focused tests, or an equivalent narrowly scoped adapter in the existing presentation owner.
**Interfaces:** API conversion preserves optional ref/count exactly. At the existing cursor's reveal point, extract Speech.Answer or a spoken TableAnswer's cue and target Answer.Who, not the player who asked. Use live delivery metadata and current presentable-member set; never trigger from RPC results or scanning historical story. At most one current cue per actor; completion/discard is identity-fenced. No cue waits for gameplay or blocks the whole story until all cycles finish. A movement command cancels the actor's cue before the existing exit/readiness gate runs.
**Tests:** API direct and nested TableAnswer mapping, no cue, distinct counts/refs. Web reveal ordering behind a pending walk; response and cue release together; duplicate delivery versus genuinely new response; both clients; no animation for catch-up, hidden/remembered/downed bodies; loss before reveal, surface removal, session change, stale completion; movement interruption followed by arrival does not resurrect the cue. Studio actual save/reopen/launch retains the cue and normal look/style/skin.
- [x] Add focused failing seam tests; implement adapter and wiring, adopt real SDK and pushed module pins, tidy.
- [x] Run API `make test && make ci-check`, web `npm run ci-check`; verify no private asset bytes enter public diffs.
- [x] Isolated QA: default Relaxed human NPC, deterministic question response with Thoughtful x2; configure in Studio, save/reopen/Play, ask and observe Enter → two sustain cycles → Exit → Relaxed on two clients. Repeat with longer N and movement interruption, then verify no revival after arrival/fresh launch. Use actual model/mixer evidence and correct deployed content hash.
- [x] Publish evidence/PRs; one bounded independent review; fixes get failing-first regressions, no new broad campaign.
**Evidence:** joined launch/stream/render proof; unit green alone is not the deliverable. Allocate isolated ports after inspecting current listeners; do not reuse or restart castle or retained authoring lab.

## Development and release sequence

Develop the consumer contract/counting first, then wire schema, behavior → encounter → session → API → joined web. Use pushed pseudo-version providers during development; no replace/go.work/source-copy overrides. Protos require explicit early merge permission for generated SDK publication. After visible acceptance and separate wave merge authorization: behavior tag → encounter tag → session tag → API released pins/dev → web SDK/dev. No private asset release is needed. No game main release is included.

## Plan checks

| Requirement | Tasks | Proof |
|---|---|---|
| Author response activity/count and preserve document | T1,T4,T6 | strict shape, real Studio save/reopen/launch |
| Cue belongs to selected response | T3,T4,T5,T6 | selection/reload/record/Act/conversion assertions |
| Starts with response appearing, multiplayer | T6 | existing-cursor release test and two-client walk |
| N sustain cycles and return | T1,T6 | real mixer finite-count tests and observed x2 |
| Discard on movement | T1,T6 | cancel/arrival/no-revival regression and walk |
| No old/unseen/replaced request revival | T1,T6 | serial/identity/lifecycle tests |
| No alternate rules/presentation owner | T3–T6 | literal carriage and existing mixer/cursor/position ownership |

| Provider | Consumer | Matched contract / availability | Joined proof |
|---|---|---|---|
| T1 authored shape | T4 dialect | optional activity/ref/cycles, same rejection/absence semantics | Studio export compiled and reloaded |
| T3 behavior | T4 encounter | typed optional cue, pushed provider commit | real selected answer |
| T4 record | T5 session | captured activity/ref/cycles, absence nil | Speak-to-Act |
| T2 wire + T5 session | T6 API/web | field12 optional ActivityCue, exact ref/count | conversion plus real stream |
| T6 reveal | T1 sequencer | stable request id, ref, cycles, default, discard | real renderer walkthrough |

Self-check: do not let the shared Say editor advertise a cue that its delivery path drops; carry spoken TableAnswer as well as nested Speech.Answer. Do not use the old blanket sustain-Infinity mixer setup for finite cues. Native mixer completion must handle single cycles and large updates. Do not restore the cancelled budget after movement. Do not treat an API-only pin or a preview's cached content as integration proof.

## Current checkpoint — QA/review complete; waiting only for merge authorization

Authorization request: https://github.com/KirkDiggler/rpg-project/issues/746#issuecomment-6540819332
Final checklist: https://github.com/KirkDiggler/rpg-dnd5e-web/issues/1363#issuecomment-6540819150
Durable evidence/verifier: `docs/case-files/npc-say-activities/` at project commit d1f2731.

Current published heads (all applicable local/hosted gates green):
- behavior toolkit#2206: 9d1577156f7bb74fd9e003876ace6ce80ff757ae.
- encounter toolkit#2207: 9e81f6ecdce1ad42236ae29665b2a1f1798023a2.
- session toolkit#2208: 2cb2d4d9dde9733b8865da4aebbe2e85e49732ce.
- API#1163: 3d7d945b0429f5b66249524ba2714f442e01e8e2.
- web#1365: d4e75340f8089085ac86ea74b48d035cc67ab6ad.

Head-specific COMMENT MERGE-READY reviews: 5721871387 /5721871472 /
5721871537 /5721871600 /5721871677 respectively, qualified on release pins and
authorization. Three findings reproduced/fixed and resolved: encounter
r4430731575 (cloneAnswer, 3ca599cc); web r4430731658 (downed queue, 64c13596),
r4430731665 (failed-body delivery, 1ffca2db). No active agents or review rounds.

Operator verified two cycles. Final QA authored three via native Studio
undo/redo/save/reopen, retained03/B, launched it, and checked both clients. Both
cancelled a20-cycle cue on movement without revival; refresh/catch-up and fresh
launch passed. Final web gate is9507 passed/6 skipped. Canonical castle prisoner-1
has two cycles on either existing where-is-the-king verdict; an actual castle
compile/reload/Speak regression and full castle gates pass. Only two Activity
objects changed in the narrowly regenerated castle golden; all words/facts/DCs/
looks stayed unchanged. No unrelated live castle navigation is claimed.

During final gates, main/dev advanced. Session caught up merged CharacterConfigReader
work without conflicts and pinned encounter9e81. API retained newer generated SDK
4717a4416c26 (v0.1.249), merged dev host/config/locking work, resolved only dependency
files, adopted session2cb2 and canonical content. Web caught up approved artwork/
benchmark/envelope changes automatically. Refreshed gates passed; two-client and
lifecycle QA were rerun on those final API/web heads. Do not downgrade these bases
when re-pinning. Web still uses published proto v0.1.248, which includes its cue contract.

Preserve operator preview3046/8136 (APIe679a5b /web31c76fe6), authoring lab3042,
old pacing3044 and castle3030/8115. Final QA3047/8137 is isolated and serves current
heads via detached `.worktrees/say-activity-qa`. QA mounted walk hash0c50b648…
and castle hash81e7a4c9… are recorded in the evidence README. The untracked walk
file remains only in API builder/QA and managed content; hold it outside the
shipped shelf for API gates, restore afterward. Never commit it to API content.

Temporary drivers are root game-dev `.worktrees/say-activity/tools/browser/_job_say_activity_*`;
metadata/model screenshots also remain in /tmp/say-activity-final-qa. The small
public evidence archive contains observable metadata and the2D Studio controls,
not licensed models/textures/tracks. The diagnostic web refresh-probe worktree
was removed; review worktrees remain for ordinary wave cleanup. Keep preview
resources out of that cleanup until the operator asks otherwise.

NEXT after explicit permission: read merge-out; verify heads/verdicts; publish
behavior tag → encounter released behavior pin/tag → session released encounter
pin/tag → API actual tags/dev → web dev. Refresh affected gates/head verdicts;
no game release/dev→main promotion. Plans/evidence branch remains available for
its documentation close-out, not an implicit sixth implementation merge.

## Historical progress (superseded by the checkpoint above)

- Agreement and discard ruling recorded on #746.
- Baseline inspected; implementation not yet started when this plan was written.
- Owning issues: web#1363 (T1/T6), protos#409 (T2), toolkit#2201 (T3), toolkit#2202 (T4), toolkit#2203 (T5), API#1161 (T6).
- T1 checkpoints: web `67ef9ef1` shape (17 expected failures then 35 focused passes), `95c976af` counted mixer (13 expected failures then 22 focused passes), `74c05723` real editor controls (3 expected failures then 8 focused passes), `cc800a5c` same-default completion identity assertion. Draft web#1365 is published at cc800a5c. Full ci-check: 9441 passed / 6 skipped, 720 files passed / 1 skipped, formatting/lint/typecheck/build green. The fresh worktree lacks the ignored dice-tray manifest, so its conditional real-provider observation test is skipped. No skips were added. Runtime stream/scene wiring and Studio persistence/launch proof remain OPEN.
- T2: protos#410 at `4ec4bae6ee01e6415f48d92a3b15b58ff2eac1f8`, local contract checks and hosted SDK generation/validation green. One independent bounded contract review found no defects; COMMENT verdict 5720261283 published/read back. No full-feature acceptance is claimed.
- CURRENT: KirkDiggler verified Thoughtful played two cycles (web#1363 comment6539659476). This accepts that observed playback, not the remaining authoring/multiplayer/interruption checks. One backend and one web independent review pass are running; final QA uses a separate local/say-activity-qa on3047/8137 with detached `.worktrees/say-activity-qa` API/web, leaving the operator preview3046/8136 unchanged. Proto #410 merged at 443397c3610a73a5bfb1f7f356641ef2acb2a8b2; generated v0.1.248 dereferences to d3deef84da300089d10cce686f04d2ed4d0ea98f. Other PRs remain draft/unmerged; no additional merge authorization.
- Behavior #2206: 9d157715, full race/lint/CI green. Encounter #2207: 5f9f3d65, full race/lint/CI green. Session #2208: 3c8a3472, full race/lint/CI green. API #1163: e679a5b, full make test/ci-check and hosted CI green. All consume real pushed pins, not source overrides.
- Web #1365: 31c76fe6, live cursor→scene→body wiring and request-consumption acknowledgement. Full local gate: 9448 passed / 6 skipped, 721 files passed / 1 skipped; hosted checks green. One regression was caught by the existing refresh-coalescing test: unconditional no-op layout dispatch split batching. Baseline and SDK-only probes passed; adding the cursor failed; removing that dispatch passed. Guarding dispatch by actual unavailable cues restores all 167 affected integration/cursor tests and the full gate. No test weakened.
- EARLY PREVIEW: http://localhost:3046/?playerId=toolkit-sandbox-bard ; API8136; env local/say-activity. API detached `.worktrees/say-activity-preview` @e679a5b; web detached same basename @31c76fe6. Root game-dev `.worktrees/say-activity` contains the manifest symlinked into root envs/local. Builder trees are not served.
- Untracked API `content/say-activity-walk.yaml` (builder + preview + mounted deployment copies) SHA256 f6e88d61bf3691995e3e334d65cf483f20b23455ccd6c04e89accbd25c228a7c, verified inside rpg-local--say-activity-rpg-api-1:/content/. Registry compiles 13 files. Served assets reproduce committed pins. `restart`, not repeated up, preserved Redis after mounting current content.
- Early smoke only: real Fighter question → captured response → Thoughtful Enter/Sustain/Exit → Relaxed, no page errors; sampled sustain lasts ~5.16s for two cycles. Native Studio opens the reference and shows Thoughtful/2 on success and failure. Screenshots inspected. This is NOT the final two-client/cancellation/Studio-save-launch QA.
- Early evidence: /tmp/say-activity-preview/{samples.json,thoughtful.png,returned.png,studio.png,studio-text.txt,page-errors.json}. Drivers in game-dev `.worktrees/say-activity/tools/browser/_job_say_activity_{smoke,studio}.mjs`. Driver corrections: wait for asynchronous Replace; real labels include `line 1 of walker-b success entry`. Wrong initial container name was a hash-command mistake, corrected before claiming mounted equality.
- Worktrees additionally: toolkit `.worktrees/say-activity-behavior`, `.worktrees/say-activity-session`; API `.worktrees/say-activity`; proto reviewer `.worktrees/say-activity-review` clean. Reviewers use dedicated detached review worktrees. Authoring lab3042, earlier pacing3044 and operator castle3030/8115 untouched. Do not reset the preview while the operator is looking.
- REMAINING: actual native save/reopen/undo/redo/custom-look launch and two-client counted/cancelled/no-revival walk; applicable castle example/check; one bounded independent provider/consumer review, focused fixes and final gates; explicit merge authorization then released re-pins/inside-out merges. Do not label the whole slice done from these smoke checks.
- T2 documentation location corrected after reading the actual owners: the presentation-service page is for dice throws, so the cue belongs beside activity identity in session-service.md.
- Reconcile exact nested JSON decoder/wire adapter and module lint commands in the owning task before editing; these are implementation-location checks, not new behavior decisions.
