# Machine inspection implementation plan

> Execute directly, task-by-task, using the executing-plans and build-slice skills. Independent review is one bounded round after the operator's early view, not a fresh implementation agent per task.

**Goal:** Inspect decisions against their exact configuration, including evaluated alternatives and actual execution, in the playtest debugger and structured API capture.

**Architecture:** [Design R1–R9](design.md), [walkthrough](README.md). Extend the existing symbols-plus-record pattern. Machine returns trace values, composer reports dispositions, encounter writes record-only evidence, API captures committed batches and web renders the same values.

**Authority / finish:** KirkDiggler approved the conversational design and continuing on 2026-11-08; [story #898](https://github.com/KirkDiggler/rpg-project/issues/898). Build/commit/draft PRs and an owned early-view stack are authorized. Visible early look precedes full walk/review. The later narrow proto publication was authorized and completed; implementation-wave merges and operator-stack changes are not authorized by this brief. Completion includes the whole joined path, normal independent review, applicable current-head gates and eventual authorized tag/re-pin release, not just Task1.

**Tech:** Go modules, existing JSON record payloads / v1alpha2 RecordService, TypeScript/Vitest/React. No new RPC is needed. **R9 chooses raw JSON:** the Struct converter rounds9007199254740993 to9007199254740992, so RecordEvent gains an additive raw byte payload. [Decision/evidence](https://github.com/KirkDiggler/rpg-project/pull/902#issuecomment-6727648247). Tasks7–8 now close this gap: official proto v0.1.263 is published and adopted, with exact live/captured verification. Implementation-wave merge authority remains separate.

## Constraints and baseline

- One toolkit module per PR; worktrees under each repository's `.worktrees/`. Preserve other sessions' environments and trees; re-inventory before acting, never recreate absent historical manifests. Only `local/machine-inspection` is owned here.
- Generic trace values, not Patrol-specific coordination. Capture performs zero additional fact reads/steps/rolls. Rejected, unknown, not evaluated and deferred are distinct.
- Original bound configuration is authoritative for symbols; private progress is separate. Machine proposals are not automatically composer commits, and selection is not completion.
- Existing after-save/best-effort record delivery remains; failures/gaps cannot claim complete evidence. API exports do not own gameplay rules or add per-verb database reads/writes under the lock.
- Analytics/store selection, new NPC behavior, 3D simulation replay and orchestration/cache optimizations are out of scope.
- Inspected: project `07516c55921c10a4ecfb0b09164167bcdb89077a`, toolkit `d40337525997f360bfc3f6896c1bc1461acedba5` (includes incoming audience interning), API dev `85c8bcc47d983f182310500e0b0c2256cdae4f63`, web dev `e2247b1e3b6cac08e16416c27f121bf2ce17aa46`.
- Evidence: machine baseline `go test -race -count=1 ./...` passes. `session/record.go` is per-session, unfiltered diagnostics, publishes after save and maps identical payloads for live/read. Sessions use unbounded server retention; `recordFeed.ts` caps visible rows at500. Its current symbol reader loses symbols when those rows age out. This cut fixes that reader lifetime without inventing a new server record store.

## Sequence / slices

1. [Machine inspection S1 #899](https://github.com/KirkDiggler/rpg-project/issues/899): Tasks1–2, generic machine/composer outcomes.
2. [Machine inspection S2 #900](https://github.com/KirkDiggler/rpg-project/issues/900): Tasks3–5, encounter evidence through committed record and API capture/gating.
3. [Machine inspection S3 #901](https://github.com/KirkDiggler/rpg-project/issues/901): Task6, live and captured inspection.

Develop each provider to green, publish its commit and pin consumers to real pseudo-versions. Do not merge to unblock development. Release later inside-out: machine → behavior → encounter → session → API → web, verifying real generated tags and re-pin gates. Recheck the API/session integration against the concurrent resident-instance changes before the joined walk; do not modify that session's worktrees.

## Review focus

- R9 boundary: exact captured integers must not be silently rounded in the reader. Live transport uses the same lossless reader as capture; missing or malformed raw data never falls back to approximate Struct.

1. Short-circuited nested guards and later transitions must not be evaluated for display (Task1).
2. A losing preview may contain a proposed transition/Set; actual progress must stay unchanged (Task2).
3. A partial/held walk must not be logged as arrived/completed (Task3).
4. Capture starting after a host restart, delayed writer shutdown or missing configuration cannot masquerade as complete history (Task5).
5. More than500 rows, reconnect, source edit and imported captures must resolve the old configuration or explicitly report it missing (Task6).

### Task 1: One-pass machine trace values

**Owner:** toolkit `mind/machine`, separate feature worktree/PR. No provider dependency changes.
**Files:** new `trace.go`, `trace_test.go`; change `step.go`, `guard.go`, `errors.go`, `doc.go`; owning `knowledge/runs/2026-11-08-machine-inspection.md`.
**Interfaces:**
- `Program.TraceStep(*TraceStepInput) (*TraceStepOutput, error)`; input `{Blob *Blob; On Beat; Event, Env any; Now uint64}`.
- Output `{Step Stepped; Trace StepTrace}`; `StepTrace{Before,After string; On Beat; Now uint64; Transitions []TransitionTrace}`.
- `TransitionTrace{Index int; On Beat; Evaluated bool; Selected bool; Guard GuardTrace}`. Nonmatching beat and later unvisited transitions are not evaluated; symbols supply their authored content.
- `GuardTrace{Evaluated bool; Verdict Verdict; Reason string; Read bool; Known bool; Value any; Scalar bool; Children []GuardTrace}`. `Value` contains only observed bool/int/string scalars, never event/env objects or mutable host aliases. `Known` preserves the reader's answer; `Scalar` says the observed value was representable. Unknown facts never export the callback's ignored value. Child positions match the guard's authored tree; unvisited children remain unevaluated.
- Existing `Program.Step` delegates to the same evaluator with no trace allocation. Nil TraceStep input → new `ErrNilInput`; other errors match Step. Both paths apply exactly the same mutations to the supplied blob.
**Tests:** `TraceSuite.TestFirstMatchAndShortCircuitReadOnce` (false then unknown then winner; later reader panics if asked), `TestNestedGuardTraceKeepsUnvisitedAndUnknownDistinct` (All/Any/Not and three-valued logic), `TestTracedAndOrdinaryStepsMatch` (state/bag/anchor/wake/acts and callback counts), `TestObservedValuesAreDetachedAndUnknownStaysUnknown` (false/zero/empty plus bad opaque callback value), `TestErrorsAndTerminalState`, `TestFlagScopeAndRoundsReadOnce`. Add trace/untraced benchmark, report measurements rather than a performance promise.
- [x] Tests first; named-interface compile RED is expected, not an unrelated environment failure.
- [x] Share evaluation, collect results only where evaluation already occurs.
- [x] Focused tests, whole-module race/lint, actual root hook; commit/push/draft PR.
**Verification:** from owning module: `go test -count=1 -run '^TestTraceSuite$' .`, `go test -race -count=1 ./...`, `golangci-lint run ./...`; root `.githooks/pre-commit` through normal commit. Mutation of a skipped reader to an extra call must fail actual count/panic assertions.

### Task 2: Composer outcomes distinguish proposal and commit

**Owner:** toolkit `mind/behavior`, its own worktree/PR; needs Task1 pushed version.
**Files:** new `trace.go`, `trace_test.go`; `composer.go`, `doc.go`, manifests and owning run record.
**Interfaces:** add `Trace bool` to `ObserveInput` / `DecideInput`, `Outcomes []InstanceOutcome` to both outputs. `InstanceOutcome{ID,Definition,Before,After string; Requested AnswerKey; Selected,Committed bool; Skipped string; Step *machine.TraceStepOutput}`. Skipped values are exported constants `sleeping`, `not_addressed`; absence means evaluated. `After` is actual committed state; `Step.Trace.After` may be only the preview's proposed state. Existing `Steps`, `Pick`, `Deferred` semantics unchanged. Disabled trace → nil Outcomes; never infer dirty state from fired transitions alone.
**Behavior:** trace each preview once with Task1; assemble dispositions only after selection/commit. No-candidate hold leaves proposed step uncommitted; explicit Hold row commits normally. Error paths commit nothing as before.
**Tests:** `ComposerTraceSuite.TestLosingProposalIsNotCommitted` (high role wins, lower proposed Set/To retained), `TestSleepingAndAddressedDeliveryAreNotFalseGuards`, `TestNoCandidateAndExplicitHoldDiffer`, `TestTraceDoesNotChangeChoicesOrReadCounts`, `TestReturnedTraceCannotMutateProgressOrConfiguration`, `TestObservationPickRefusalStillAtomic`.
- [x] RED with full authored two-role recipe/table and strict dice.
- [x] Add one-pass trace plumbing and disposition values.
- [x] Focused/full race/lint/root hook, commit/push/draft.
**Verification:** `go test -count=1 -run '^TestComposerTraceSuite$' .`; full owning-module race/lint and normal hook. Compare ToData and exact d100 counts with trace off/on.

### Task 3: Encounter symbols, decision records and execution evidence

**Owner:** toolkit encounter, separate worktree/PR; Task2 pushed version.
**Files:** new `behavior_trace.go`, `behavior_trace_test.go`; `behavior.go`, `behavior_work.go`, `clocks.go`, `wakebeats.go`, `pause.go`; tests beside real behavior/pause fixtures; manifests/run record. Inspection places observation delivery in wakebeats.go and selected-work/pause ownership in clocks.go/pause.go, not machine.go.
**Interfaces:** exported record kinds `BeatBehaviorConfig`, `BeatBehaviorDecision`, `BeatBehaviorWork`. Payloads carry `beat`, `schema:1`, actor/member, configuration identity, existing act/clock correlation. Config describes original `BehaviorInput` (not Progress). Decision carries operation/trigger and Task2 outcomes; a `pick_record` backlink names the actual `answered` record containing picker arithmetic. A suppressed no-candidate answered record carries its verdict in the diagnostic record instead; no guard/row evaluation is repeated. Work carries attachment/key, `decision_record` backlink and explicit `selected`, `partial`, `paused`, `completed` status from actual owner call-sites. Non-completion due to inactive/superseded work is explicitly abandoned, not completed. Diagnostic records have an empty audience (GM/debug only).
**Behavior:** persist `SymbolsRecorded bool` alongside the already immutable definition in BehaviorData/behaviorRuntime; emit its symbol before the first referencing trace. This covers placed members and reserves without duplicating definition hashes in MemberData/TableHash. No duplicate config on unchanged reload. The choice's time act and execution's move act are distinct: preserve `DecisionRecord string` on BehaviorWork/BehaviorWorkData through pause/reload solely as a diagnostic backlink. Custom/compelled driver bypass does not pretend the ordinary composer ran. Keep completion validation and held-work restore unchanged. New beats are diagnostics only, not physical body-state changes.
**Tests:** `BehaviorTraceSuite.TestConfigurationPrecedesDecisionAndUsesOriginalBinding`, `TestInvestigationDefersPatrolWithoutCommittingItsPreview`, `TestPartialAndPausedWalkNeverClaimCompletion`, `TestReloadKeepsSymbolsAndRollCount`, `TestDriverBypassDoesNotInventMachineEvaluation`; compare physical state, ordinary acts and exact scripted dice to existing canonical fixtures. Save/load under normal session unbounded retention; test missing-symbol input as explicit reader incompleteness, not a source rebind.
- [x] RED through real compiler/composer/world fixtures.
- [x] Capture returned values and existing work outcomes; no extra selection/movement.
- [x] Full owning-module gates and hook; benchmark record growth/trace cost, commit/push/draft.
**Verification:** focused real behavior/paused suites, `go test -race -count=1 ./...`, lint/root hook; verify record JSON contains no event/env object or unseen sound-maker from authoritative host state.

### Task 4: Session keeps new kinds record-only

**Owner:** toolkit session, separate worktree/PR; Task3 pushed version.
**Files:** `act_family.go`, `acts_test.go`, `act_fold_internal_test.go`, `behavior_launch_test.go`, `record_test.go`, manifests/run record.
**Interfaces:** existing `RecordEvent`, `RecordStream` and read/catch-up outputs stay unchanged. Register Task3 constants in the explicit record-only set. No new proto.
**Tests:** `TestBehaviorDiagnosticsAreRecordOnly`, `TestBehaviorRecordLiveMatchesCatchupAfterReload`, existing launch/continuation/compulsion tests unchanged in outcome. Assert no diagnostic family/narration leak and save-before-publish.
- [x] Add record-only/real-launch tests; RED against missing kinds.
- [x] Classify new kinds and pin encounter; do not rewrite manager/locker/cache.
- [x] Owning-module race/lint/hook, commit/push/draft.
**Verification:** full module gates, exact captured RecordEvent payload identity across live/read, no changes to ordinary acts or continuation completion.

### Task 5: Host capture and diagnostic access gate

**Owner:** API; Task4 published pin. Reconcile latest dev/resident-instance work before changing wiring.
**Files:** new `internal/orchestrators/session/record_capture.go`, `record_capture_test.go`, `record_publisher.go`; existing `orchestrator.go` (record sink wiring only), `cmd/server/server.go`; `internal/handlers/dnd5e/session/v1alpha2/{handler.go,get_record.go,stream_record.go,handler_test.go}`; integration `internal/integration/session/behavior_trace_test.go`; manifests/run record.
**Interfaces:** `RecordCapture` implements the existing SDK `RecordStream` and `Drain(context.Context) error`; config injects an `io.Writer`, bounded batch capacity and error reporting callback. `recordPublisher` fans the same immutable batch to broker and capture, joining delivery errors. Queue capacity defaults to64 commit batches, matching existing record-broker units. Queue overflow/write failure marks capture incomplete and returns/reports an explicit capture error; it never fails/rolls back the committed verb. Server injects a dedicated stdout JSONL writer; ordinary server logs remain on stderr. Orchestrator tests can inject a discard stream or capture writer without global logging. File/archive collection is downstream of this output, not a new host store. JSONL envelope `{schema:1,kind:"record",session,id,at,act,beat,fields}` plus capture status/gap envelope. Human diagnostic text is not the data contract. Writer/capture routing is host-owned. Emit a capture-start status on process startup stating that earlier history is not implied; records from an ongoing run remain inspectable with their previously captured symbol records. A partial capture lacking those records is explicitly incomplete in the reader. No bootstrap/per-verb SDK Record re-read or claimed durable exactly-once archive is introduced.
**Gate:** add `Enabled bool` to record `HandlerConfig`; deny both record methods when false before reading/subscribing. Server wires from existing `authConfig.DevMode` (`AUTH_DEV_MODE`, documented as never production); ordinary StoryService unchanged. Capture is separately configured and not controlled by this browser gate. Root server gate tests cover both modes. No deployment schema change is needed for AUTH_DEV_MODE.
**Tests:** `RecordCaptureSuite.TestWritesSameFieldsWithoutSubscribers`, `TestSlowWriterDoesNotBlockPublish`, `TestOverflowAndWriteFailureAreExplicit`, `TestDrainWaitsForAcceptedBatches`, `TestDoesNotClaimCompleteMidRunCapture`; handler deny cases and real Redis `TestMachineTraceCaptureSurvivesReload` matching record IDs/fields, config symbols and unchanged guarded continuation behavior.
- [x] Inject writer/barriers and RED tests; no sleeping to prove scheduling.
- [x] Implement bounded worker, opaque fanout, drain and server gate.
- [x] Run focused tests, `make test`, `make ci-check`; commit/push/draft; boot owned stack.
**Verification:** full API gates; actual server logs parse as JSON records; inspect same decision ID through API capture and GetRecord. Producer source edit/reload keeps original config. Capture failure is visible; no claim of durable exactly-once audit storage. Existing #649 file-read contract is linked rather than duplicated as a new event service.

### Task 6: Inspect live and captured traces with retained symbols

**Owner:** web; Task5 running stack. Exact raw-wire adoption is isolated in Task8.
**Files:** new `src/components/session/combat-experience/behaviorSymbols.ts`, `BehaviorTrace.tsx` and tests; new `src/components/session/recordCapture.ts` and tests; existing `recordFeed.ts`, `recordFeed.test.ts`, `useSessionRecordStream.ts`, `combat-experience/{RecordPanel.tsx,RecordPanel.test.tsx,StoryLog.tsx,recordSymbols.ts}` and owning run record.
**Interfaces:** `BehaviorSymbol` indexes config ID to captured bound configuration; `RecordFeedSnapshot` carries independent symbols alongside bounded rows. Feed consumes symbols before applying the500-row cap and clears them on session change, not ordinary row eviction. `readBehaviorTrace(row,symbols)` is a pure formatter of recorded values. `parseRecordCapture(text)` accepts schema1 JSONL records/status envelopes for one session, validates shape and reports gaps/missing symbols, never reexecutes guards or fetches current YAML. Mixed-session input is refused with a request to filter by session rather than merging unrelated timelines. Existing table symbol associations also survive visible row trimming.
**Tests:** `behaviorSymbols.test.ts` exact-ID resolution, unknown schema, missing config, proposed versus committed, unknown/false/not-evaluated; `recordFeed.test.ts` >500 rows and multi-page catch-up retain symbols, session switch clears, duplicate delivery does not duplicate rows; `recordCapture.test.ts` old capture remains readable after source changes, malformed/missing/gap records are explicit; RecordPanel test shows transition/role/table/execution levels without changing normal StoryLog.
- [x] Fixture-driven RED tests using actual Task3/5 captured format.
- [x] Implement symbol cache, reader and existing Debug view expansion/import.
- [x] Full `npm run ci-check`, commit/push/draft, boot owned early-view stack.
**Verification:** inspect actual real-compiler two-guard interruption/partial/resume journey, then open its captured file, locate the same record ID and configuration. Move past500 visible rows; alternatives still resolve. Development mode can inspect; released-mode server denies diagnostic RPCs while gameplay works. Operator early look precedes independent review and standing castle walk. Record tests, actual browser outcomes and unrun branches separately.

### Task 7: Publish the lossless record contract

**Owner:** rpg-api-protos, existing `dnd5e/api/session/v1alpha2/record.proto`; run record at repo knowledge/runs. Same slice #900, not a new gameplay story.
**Interface:** additive `bytes fields_json = 6`; field5 Struct remains with deprecated annotation. Raw data is exact UTF-8 JSON as written. Empty is unavailable, not an empty object. IDs/RPCs/order/audience unchanged; no exact-data fallback to Struct.
**Verification:** `make format`, `make test`, exact-head CI generation/mock/TS validation; no bespoke tests of generated mechanics. Independent review covers the field's meaning and consumer requirement. CI publishes bindings only after main merge; do not use local overrides or hand-publish generated code.
**Status:** proto#426 merged at `45a0fb36` after narrow authorization; CI published official **v0.1.263**, generated commit `cb53a2b66f6a85f4bbf0131040fa4c804ab6b891`. API's nested Go module resolves that published commit; web pins the official tag. This is not implementation-wave merge authority.

### Task 8: Adopt exact live payloads and close readiness

**Owners:** API converter/record handlers and web record feed; requires Task7's actual published SDK tag. Existing API/web PRs, not duplicate implementations.
**Files:** API `internal/handlers/dnd5e/session/v1alpha2/{convert.go,convert_test.go,handler_test.go}` and `internal/integration/session/behavior_trace_test.go`; web `recordFeed.ts`, its tests, record-capture/inspection tests and manifests; update run records.
**Interfaces:** API fills generated `RecordEvent.FieldsJson` from the original SDK `Fields` bytes, detached and unparsed. Legacy Struct may remain approximate for old generic readers, but is never the exact authority. Web reads generated `fieldsJson`, validates UTF-8 and JSON with existing lossless `parseRecordJSON`; missing/malformed/non-object raw data preserves ID/act/beat with an explicit unavailable reason, never a Struct fallback. Replace the provisional unsafe-Struct gate in the new reader with this raw-payload refusal contract.
**Tests:** `TestRecordEventRawJSONIsVerbatim` covers integer9007199254740993, false/zero/empty and raw-byte ownership; both GetRecord/StreamRecord retain original bytes; real Redis continuation/capture test compares raw fields with export. Web live/capture exact-wide-value parity; valid Struct with absent raw still unavailable; malformed UTF-8/JSON and conflicting symbols explicit; ordinary story and cache/linked-inspection tests retained.
- [x] Adopt the actual generated Go/TS tag, not an invented version.
- [x] RED→GREEN conversion/reader tests; preserve original records and no extra interpretation.
- [x] Full API make test/make ci-check, full web ci-check, current-head CI.
- [x] Update owned detached preview, verify binary/content hashes, perform affected two-client/QA-castle checks after accepted early look.
- [x] Same independent reviewer closes the new wire seam and any findings; current-head closure retains release-pin qualifications. No feature merges without operator authority.

## Visible plan checks

| Requirement | Tasks | Concrete proof |
|---|---|---|
| Exact original configuration, not edited source | 3,5,6 | Original-binding/reload test and old-capture read |
| Actual guard alternatives; no extra evaluation | 1,2 | Short-circuit panic/count controls and trace parity |
| Precedence versus weighted choice | 2,3,6 | Losing proposal retained; existing row verdicts shown |
| Actual partial/pause/completion | 3,4,6 | Real held/partial path and unchanged ordinary acts |
| One committed record path, server log independent of browser | 4,5 | Live/catch-up equality and no-subscriber capture |
| Symbols survive live row trimming / offline read | 6 | >500 rows and captured-file fixtures |
| Production client isolation; export failure honesty | 5,6 | Gate tests, bounded writer failure tests and gap UI |
| No analytics backend or cache work | All | Diff/scope review; no new RPC, store or rules interpreter |

| Provider → consumer | Produced / consumed | Dependency | Joined proof |
|---|---|---|---|
| machine → behavior | TraceStepOutput / preview Step | Task1 pseudo-version | Exact reads and state parity |
| behavior → encounter | InstanceOutcome / decision payload | Task2 pseudo-version | Real compiler/interrupt/resume fixture |
| encounter → session | New Beat constants / record-only set | Task3 pseudo-version | No narrative leak, identical RecordEvents |
| session → API | Unchanged RecordStream / opaque publisher | Task4 pseudo-version | Save/reload plus same record IDs |
| API → web | Existing record RPC + schema1 capture | Task5 server | Live/captured ID/config match |
| protos → API/web | fields_json field6 / FieldsJson / fieldsJson | Task7 published generated tag | Task8 exact wide values, byte identity, no fallback |

**Check disposition:** The existing symbols can be aged out by the500-row client cap; fix the symbol-cache lifetime rather than changing wire ordering or adding a lookup RPC. Detailed machine trace is optional so unrelated machine callers do not allocate it. Composer commit dispositions must be applied after selection; this closes the tempting but wrong use of every preview as a transition. API export stays on the existing after-save seam, not the concurrent resident-instance path. Checked task items are supported by the [final readiness receipt](https://github.com/KirkDiggler/rpg-project/issues/898#issuecomment-6731477380) and linked owning-PR evidence; release/tag re-pins are not marked complete.

## Resume

Readiness checkpoint: machine `aa9db662` / toolkit#2324, behavior `00337bc9` / toolkit#2325, encounter `676d7839` / toolkit#2326, joined session `166a0f6a` / toolkit#2328, API `c149f7a7` / api#1207 and web `f71e6258` / web#1400 have green local and exact-head remote gates. Full web transcript:10005passed/6skipped. Only proto#426 has merged and published, as official v0.1.263.

The operator accepted the early look. The same independent round closed W1 and raw-wire adoption with no new Critical/Important findings: [API closure](https://github.com/KirkDiggler/rpg-api/pull/1207#pullrequestreview-5821167418), [web closure](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1400#pullrequestreview-5821167594). No new whole-wave review is needed for unchanged code.

Project worktree `.worktrees/machine-inspection`, branch `docs/machine-inspection`. Toolkit worktrees: `machine-inspection`, `behavior-inspection`, `encounter-inspection`, `session-inspection`, each on its corresponding `feat/` branch and owning one module. API/web use `.worktrees/machine-inspection`, `feat/machine-inspection`. Task ledgers live under each `.superpowers/sdd/plan/`; public story/slices remain the shared record.

Owned preview: `local/machine-inspection`, web3048/API8138, detached `898-api-walk` atc149f7a7 and `898-web-walk` atf71e6258. Fresh owned identities completed exact two-client live/catch-up/reconnect and captured-reader checks, plus the applicable castle walk. Earlier exited actors were not restored. Exact wide9007199254740993,41live/read rows versus40published rows, source/binary hashes and qualified castle outcomes are in the [final QA/readiness record](https://github.com/KirkDiggler/rpg-project/issues/898#issuecomment-6731477380).

Next: obtain implementation merge-out authority, then release inside-out with real toolkit tags, consumer re-pins and changed-head gates. Do not restart completed provider tasks or re-review unchanged code. The other session's unmerged Redis adapter is not copied or silently adopted. Broader optimization, Studio authoring and analytics remain separate.
