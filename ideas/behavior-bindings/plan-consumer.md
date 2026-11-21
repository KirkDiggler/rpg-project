# Live Patrol consumer implementation plan

> Historical planning checkpoint, 2026-11-06: R4–R9 were settled; Task 1 was implemented/reviewed and Tasks 2–3 were in progress. The implementation snapshots and interface targets below describe that planning stage, not current delivery status.

## Shipped execution checkpoint — 2026-11-08

Tasks 1–4 and authorized adoption are complete; #866 is closed. The
[release receipt](https://github.com/KirkDiggler/rpg-project/issues/866#issuecomment-6719508279)
records generated tags, merge commits, fresh-head gates and follow-ups.

- Encounter [toolkit#2293](https://github.com/KirkDiggler/rpg-toolkit/pull/2293): **v0.185.0**.
- Session [toolkit#2296](https://github.com/KirkDiggler/rpg-toolkit/pull/2296): **v0.171.0**, consuming the released encounter.
- API [api#1191](https://github.com/KirkDiggler/rpg-api/pull/1191): merged to **dev** with released providers.
- Studio [web#1397](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1397): merged to **dev**; existing codecs preserve source, without dedicated behavior panels.

The [joined optimized-stack receipt](https://github.com/KirkDiggler/rpg-project/issues/866#issuecomment-6718917429)
distinguishes real two-client/UI/castle outcomes from controlled tests. The
canonical runtime was checked together with early-answer continuations,
subscriber-local folding, participation memoization, site geometry/sight caches
and commit-unit delivery. The same independent round closed its bounded
integration deltas. Remaining latency work and named minor/planner/observability
follow-ups are separate; consumer main is still the operator's release gate.

The body below is the historical implementation plan, not remaining work. Its
interface snapshots and progress notes do not supersede the released owner code.
Documentation companions #877/#868 remain separate open records.

**Goal:** Complete [NPC behavior S1's live adoption, #866](https://github.com/KirkDiggler/rpg-project/issues/866), under [story #859](https://github.com/KirkDiggler/rpg-project/issues/859): one authored Patrol definition, two-/three-waypoint guards, real movement, qualifying disturbance, retained resumption and save/reload, with source surviving Studio edits.

**Architecture:** [Consumer design R4–R9](consumer-design.md), [walkthrough](consumer.md), inherited [R1–R3](design.md). Machine-driven actors use one canonical composer runtime and new-format source. R9 removes old-format compatibility and normalization machinery; the playtest site and fixtures are rebuilt. Body/post state has one designated instance. Typed delegation preserves compulsion; world/fight opportunities are distinct. YAML plus Studio preservation is the first delivery, not new editors.

**Tech stack:** Go module boundaries in toolkit; existing dungeonspec YAML and canonical behavior JSON; React/TypeScript Studio codecs; existing authoring/session RPCs and API integration harness.

**Historical planning readiness (superseded above):** R4–R8 and the source/lane contracts carry the agreed target. [R9](https://github.com/KirkDiggler/rpg-project/pull/877#issuecomment-6698028412) explicitly removes backwards compatibility: rebuild content/fixtures for the new format instead of adding migration adapters. The migration-parity prerequisite is deleted. Task 1 is implemented and independently reviewed; continue Tasks 2–3 on its pushed commit, keeping concrete gameplay/ownership checks and normal review/release gates.

## Global constraints and finish line

- Consume released `mind/behavior/v0.20.0`; any needed provider extension gets its own nearest-go.mod PR and real pushed-commit pseudo-version, followed by its actual tag before consumer merge.
- No runtime sequence interpreter, Patrol-specific factory, second weighted roll, omniscient knowledge projection, source rebind on reload, general scheduler or effects work.
- Preserve the agreed queued-beat timing, world/fight budgets, pause accounting and body/post gameplay boundaries in the new runtime. `barred` is the existing synchronous exception; other queued inputs are not flushed by it.
- No new dungeon field is accepted and silently ignored. The compiler, live consumer and Studio round-trip support ship in the same adoption wave.
- First content uses zero dwell/facing additions. Existing position-derived route/dwell behavior is not migrated. The new fixture's authored pending target is its resumption destination; no global spatial-rejoin policy is invented.
- Publish one PR per changed toolkit module, plus API/web adoption PRs. Ready-for-review is distinct from merge-ready. No merge, deployment or operator-stack changes are authorized by planning.
- Finish means the named real-path tests, owning-module gates, source round-trip, served-content verification, local two-guard walk and independent review are recorded. Close #866 only after authorized integration and its live checklist, not on a provider-only result.

## Inspected baseline and evidence

| Repository | Base inspected |
|---|---|
| toolkit main | `9922e218d7942a8cf048af46cc2090f2e77d4fd1` |
| web dev | `2150256f9e067c285c7723daf12f4ec54a03bc77` |
| API dev | `590a201ac490bbbd4ca0c2e593e958d3e5f33463` |
| project planning base | `5e1eb58d486933fe12cc058eae1141602d630448`, stacked on documentation PR #868 |

Existing focused encounter baseline passes **0.030s**:

```sh
cd rulebooks/dnd5e/encounter
go test -count=1 -v -run '^(TestHeldSuite|TestADwellReloadedWhilePausedSpendsNone|TestABumpMatchesOnlyItsOwnBeat|TestBystandersHearTheAnswerOnTheNextRaise|TestTheHoldIsAStateAtAPlace)$' .
```

At this inspected baseline, no new consumer tests ran; web/API paths were inspected only and no stack started. Later implementation evidence is recorded under each task and its PR.

Observed seams that drive this plan:

1. `TableDriver.Act` both rolls and translates; its private `intentFor` translates without needing randomness. Calling `Act` after `Composer.Decide` would roll twice.
2. `executeTurnIntent` returns turn-end, not goal completion. Every non-paused `Routed` end returns true, including partial/blocked outcomes; `walkIntent` returns a Pass when already on the target.
3. `turnPause` / `TurnPauseData` preserve movement, budget, act and terminal status, but no behavior attachment/key/completion goal.
4. `MemberInput.Machine`, `BlockingStates`, `Posts`, `MemberData.Recipe/Machine` and `session.Launch` share one existing creature-state source. R4 must resolve its relationship to private behavior instances.
5. `machine.Listeners.Set` keeps the first entry per beat. Concatenating per-role scopes can silently lose another role's listener; aggregate explicitly or retain instance identity.
6. `wakeBeat` carries location/subject/time but no heard kind. The S2 fixture's `heard.kind` fact is not supplied by current `machineVocabulary`; it must read the delivered occurrence, not guess from a later holding.
7. Studio already carries unknown root fields via `SiteScope.unread` and re-splices original YAML, but that is preservation—not a behavior editing or server-consumption contract.
8. `session.compelledDriver.Act` must run before ordinary choice: commanded turns never consult the brain. `Capabilities.Roller` already supplies the world's shared die, so no second dice capability is needed. Bypassing Driver in order to call the composer would bypass compulsion.

## Review focus

The named tests below must cover these normal-use failure classes:

1. The walk stops short because of budget, a body/door or a reaction window; do not mistake turn-end for arrival (Task 2).
2. Multiple roles listen for the same beat with different scopes; first-listener deduplication must not drop one (Tasks 1–2).
3. A held walk is saved, source is edited, and the run reloads; resume the selected work against original configuration without another roll (Tasks 2–4).
4. A profile coexists with creature-state blocking/post settings; physical permission has one designated state source, not a mirrored blob (Task 2; R4 is settled).
5. A valid current-format source field survives import/edit/local save/export/server compile; unrelated fields and explicit absence remain truthful. Old-format compatibility is not a requirement (Tasks 3–4).

## Implementation interfaces

The behavior provider APIs below exist at #2292's pushed commit. Encounter/source interfaces remain implementation targets unless their task's checkpoint says otherwise.

| Owner | Proposed interface | Meaning |
|---|---|---|
| `mind/behavior` | `(*Composer).Inspect(*InspectComposerInput) (*InspectComposerOutput, error)` | `Now uint64` in; `Instances []InstanceStatus` out. Each status carries ID, State, Definition digest, copied Listens, distinct `Timed []machine.Beat` in transition order, and optional DueAt/WakeAt. Pure derived read over the fixed instance list; the caller distinguishes which timed opportunity it schedules. |
| `mind/behavior` | `(*Composer).StateOf(*InstanceStateInput) (*InstanceStateOutput, error)` | ID → State + Definition digest, O(1) and detached strings; avoids copying every listener on each body-permission read. Unknown ID wraps existing ErrUnknownInstance. |
| `mind/behavior` | `(*Composer).DelayWakes(*DelayWakesInput) (*DelayWakesOutput, error)` | `Span uint64` → `Shifted int`; shift only non-nil WakeAt under existing encounter elapse policy, never anchors. Preflight overflow; error leaves every blob unchanged. No owned clock/timer. |
| `encounter` | `BoundBehavior{Instance behavior.InstanceSpec; Completions map[behavior.AnswerKey]machine.Beat}` | Canonical materialized instance and its completion mapping. No legacy creature-import arm. |
| `encounter` | `BehaviorInput{Instances []BoundBehavior; BodyInstance string}` | Canonical input on MemberInput/JoinInput/MonsterPlacement. No roles means no ordinary behavior, not an old-runner fallback. BodyInstance defaults to the sole instance; multiple instances require an explicit source for state-bound blocking/posts. |
| `encounter` | `BehaviorData{Instances []behavior.InstanceSpec; Completions map[string]map[behavior.AnswerKey]machine.Beat; BodyInstance string; Progress behavior.ComposerData}` | Materialized canonical saved values; no callbacks/source registry. No second live Machine blob/Table/Recipe copy on a composed member. |
| `encounter` | `Delegated struct{}` implements sealed `TurnIntent` | Ordinary decision source delegates without rolling; compulsion/custom driver may return a concrete intent first. No HasBehaviors/UseBehaviors mode flags. Reject Delegated with an existing Pick; resolve it before executeTurnIntent. |
| `encounter` | `IntentForAnswer(*IntentForAnswerInput) (*IntentForAnswerOutput, error)` | Existing MonsterView + chosen Answer → existing TurnIntent; extract current translation, no random selection or new rules. |
| `encounter` | `BehaviorWork{Instance, Definition, State string; Key behavior.AnswerKey; On machine.Beat; Goal *spatial.Position; Kind BehaviorWorkKind}` | Caller-owned claim for the selected action. Initial completion kinds are exact-cell arrival and delivered line/explicit hold; unsupported mapped completions refuse at configuration ingress rather than guess. |
| `encounter` | `TurnPauseData.Behavior *BehaviorWorkData` | Persist that selected claim across the existing held turn. Validate ID/digest/key/beat/target against saved configuration. Nil retains existing non-behavior pause meaning; no claim of cryptographic integrity against deleting an entire optional configuration. |
| `dungeonspec` | `BehaviorDefinitionSpec`, `BehaviorBindingSpec` | The source union in the walkthrough; definitions are site-local, bindings ordered, `items: cells` uses existing RoomCell owner and `with` uses the scalar schema. No invented defaults. |

Install canonical instances from the rebuilt source and persist canonical configuration/progress only. Do not implement Machine/Table import sugar, an old-save converter or a second live Machine blob. Old unsupported representations fail explicitly. Only the designated body instance feeds physical state projection/BeatState; private duty progress is not extra client knowledge. Tests establish the new authored reaction, terminal, wake, combat and body/post contracts—not byte or behavioral parity with experimental representations.

## Task 1: Composer introspection and wake adaptation

**Delivers:** Read-only scheduling/body-state access and the existing encounter's explicit time-jump operation; no new scheduler.
**Owner:** toolkit `mind/behavior`, separate provider PR. **Prerequisite:** settled R4–R7.
**Files:** create `mind/behavior/inspect.go`, `inspect_test.go`, `wake_test.go`; modify `composer.go` preview only to respect an instance's future wake on timed input; update `doc.go`/README/run record.
**Interfaces:** the three methods/types above; use stored programs/blobs, not recompile snapshots in the encounter or add a second progress store.
**Tests:** `TestComposerInspectSuite`:
- Two instances with `heard` scopes 2 and 8 retain both descriptions; mutating returned scope data changes no instance.
- Active timed instance reports its next due; a terminal/untimed instance reports none; future wake is preserved; current-state Timed names distinguish world `time` from fight `turn`; StateOf returns only the addressed instance's state/digest.
- Empty composition returns a valid empty result; nil input and unknown instance produce existing named errors.
- DelayWakes(5) shifts wakes 8→13, leaves nil wakes/anchors/state/bags unchanged; zero span changes none; overflow wraps `ErrBadWakeDelay` and refuses with byte-identical snapshots.
- `TestTimedInputDoesNotStepASleepingInstance`: a high-priority role sleeps until 8 while another role is due at 3; time 3 selects only the awake role, without touching the sleeper or its dice. Time 8 can select it. Non-timed delivered input can still wake it. This caller/composer boundary is missing today: Program.Step clears a due wake but does not itself suppress an early timed call.

- [x] Write named tests, observe meaningful red, implement the methods and run full module gates.
- [x] Publish provider commit for consumers; no provider merge is needed to start local integration.

**Checkpoint:** [toolkit #2292](https://github.com/KirkDiggler/rpg-toolkit/pull/2292),
`64cf9672540f1340703901a3cb89c07dce7a8273`: full module race/lint/hook and
applicable CI pass. [Independent COMMENT review](https://github.com/KirkDiggler/rpg-toolkit/pull/2292#pullrequestreview-5808481734)
reads all eight files, reports no findings and kills early-wake, alias and overflow
mutants. MERGE-READY for provider scope only; not merged or released.

**Verification:** `cd mind/behavior && go test -race -count=1 ./... && golangci-lint run ./...`; actual scoped hook. Consumer pins pushed commit during development, real provider tag before merge.

## Task 2: Encounter ownership, one selection and actual completion

**Delivers:** The real state/execution/persistence path. **Owner:** toolkit `rulebooks/dnd5e/encounter`, same PR as Task 3 because they are one release unit. **Prerequisites:** Task 1 provider pin; settled R4–R7.
**Current checkpoint:** [draft toolkit #2293](https://github.com/KirkDiggler/rpg-toolkit/pull/2293)
at `440c681c44cb69bb17df6e7c32fd43a5b8653cc1` pins the provider and adds
`IntentForAnswer` plus occurrence-owned heard kind through JSON reload. Exact
d200 face 101 preserves the selected target with no second roll; clang/voice
kind loss was observed as an assertion failure and fixed. Full owning-module
race/lint/scoped-hook pass. The old runner is still active at this preparatory
checkpoint; replacement, completion/pause, canonical source and live proof remain.

**Create:** `behavior.go`, `behavior_test.go`, `behavior_work.go`, `behavior_work_test.go`, `behavior_data_test.go` in the encounter module.
**Modify inspected files/symbols:** `field.go` MemberInput/memberRecord; `encounter.go` Join/Board copies and construction; `data.go` MemberData/ToData/Load; `machine.go` program/wake adapters and state projection; `wakebeats.go` land/match/re-arm; `noise.go` makeSound/landHeard; `tabledriver.go` translation; `clocks.go` runIntents/executeTurnIntent/record table description; `pause.go` turnPause/TurnPauseData/copy/validation/finishPausedIntent; `post.go` and `cellfacts.go` state reads. Replace the old live machine runner; update/rebuild affected fixtures against canonical input. Do not keep an old-format adapter or nil-input execution fork.

**Execution contract:** [selection-lane boundary](selection-lane-contract.md). The actor has one action-selection lane per offered opportunity, not one combined machine or a universal resource scheduler.
- Install the same Composer-based runtime for machine-driven actors from canonical input; remove the Machine/Table compatibility path. Empty/no-policy actors gain no invented behavior. Resolve body-state source once (sole instance by default); physical state reads StateOf(BodyInstance), never a copied blob.
- Derive due/listener indexes from Inspect. Preserve all role listeners; a conservative unscoped actor-level union may dispatch to the composer, whose guards recheck exact scope. Never concatenate into first-per-beat and lose a broader scope. Rebuild indexes at load, do not persist them or scan the roster on every clock raise.
- Preserve owed-round ordering and elapse semantics. Use existing world high-water/delivered timestamps consistently, not an unrelated fight-round counter. Deliver world opportunity `time` and fight opportunity `turn` separately; both may be timed inputs, but the world due adapter filters current-state Timed names and never arms a fight-only role merely for world time. Keep `clock:world`/`clock:fight` as notifications, not action opportunities. Author the rebuilt site's world-state and ordinary combat policy explicitly. No defensive clock.kind guard is required on every Patrol.
- A queued input remains queued; barred is still synchronous and targeted. The caller explicitly distinguishes data-only observation from an existing permitted reaction/time opportunity. Do not treat ordinary observation or completion as another paid turn. Use the one canonical permission/budget path.
- Add the delivered kind to wakeBeat/wakeEv and their saved form; `heard.kind` reads that occurrence. Queued inputs lacking kind are unknown, not guessed. Never project its private Subject as an unseen maker identity.
- Keep Driver consultation first, preserving `session.compelledDriver` precedence. The ordinary source returns typed `Delegated{}` without rolling; concrete imposed/custom intents bypass the composer. Refuse Delegated with an existing Pick and refuse an unresolved Delegated at the executor. Private `selectDecision` resolves it by calling Composer.Decide once with the correct opportunity, real view/facts/temper and existing Capabilities.Roller, then IntentForAnswer. Do not call TableDriver.Act again afterward. Missing Roller retains the existing refusal, not a local RNG.
- Deliver an occurrence once. Any same-budget reconsult after a nonterminal intent is a new action-selection opportunity, not redelivery of the original heard/barred input. Preserve the existing reaction/world/turn permission distinction explicitly in the private selection context.
- Record the chosen table from its actual instance, with instance-qualified record identity, not `m.Table[chosen.Key]`. Reuse existing answer/movement projections; do not expose private role blobs in the client.
- Build BehaviorWork for a real eligible mapped result. A no-candidate hold carries no completion. Complete exact-cell Toward/Dash only after actual target arrival, including an already-there eligible selection; partial/blocked/downed/held outcomes do not complete. A delivered line/explicit hold has its own explicit completion kind, not a fake cell.
- Carry the work claim through repeated pauses and JSON reload; resume the held intent without selection. On completion validate its saved attachment/configuration/state/target relationship and deliver the mapped Observe once; update indexes, but grant no extra opportunity. Other or stale inactive-step claims cannot advance a different role.
- Invariant on errors remains existing encounter policy: caller discards the encounter rather than save partial mutation; do not invent rollback.

**Named assertions:** `TestBehaviorConsumerSuite` and `TestBehaviorPauseSuite`:
- `TestNeedsProgressDoesNotCompeteForMovement`: needs' data-only updates continue independently while investigation/duty requests compete for the offered action lane; no actor-wide state cross-product.
- `TestDataOnlyProgressSurvivesSelectedRoleHold`: no-candidate hold retains that role, rolls nothing and does not fall through; other data-only changes in the successful Decide can commit.
- `TestNotificationCannotAuthorizeSelection`: Observe with a Pick refuses atomically; the adapter cannot automatically retry it as a permitted decision.
- `TestCompletionDoesNotGrantAnotherOpportunity`: addressed completion advances only intended work and does not invoke another selection.
- Real two-/three-point actors with 10 feet per opportunity move at most two cells each; a leg farther away stays pending until arrival. No competing move, no second selection roll.
- Clang reaches only lawful listeners; voice/unknown do not activate investigation. A clang then voice queued before a raise retains both occurrence kinds in landing order.
- Investigation owns movement for its authored interval, then resumes the unfinished waypoint, not a geometrically chosen nearest point.
- A blocked goal keeps its index; remove the obstruction and the next priced opportunity continues. Already-there eligible work completes once; no budget does not.
- Reaction pause after one cell saves exact work, remaining budget and act; reload/resume causes no new roll; second pause carries the same work; drop or changed floor does not fabricate arrival.
- Different scopes on the same beat are not lost. No-listener/untimed/terminal actors remain off appropriate indexes.
- Designated body state controls blocking/posts; other role states do not. Rebuild the fixtures behind `TestTheHoldIsAStateAtAPlace`, `TestABumpMatchesOnlyItsOwnBeat` and `TestBystandersHearTheAnswerOnTheNextRaise` for canonical input, preserving the gameplay assertions rather than the old representation.
- Current-format original configuration survives source edits; corrupt IDs/state/definition/completion maps/non-null held claims are refused. Unsupported old saved forms fail explicitly; no migration/repair or silent reset is implemented.
- `TestWorldDutyIsNotOfferedAFightTurn`: an unguarded world Patrol receives no fight opportunity, makes no roll and keeps progress; a body naming both beats works in both. Existing `clock:fight` notification does not become a turn. Enter/leave combat retains the pending duty and normal combat policy.
- Commanded turns use the existing imposed intent without composer selection/roll/progress change; custom concrete decisions keep their precedence. Same-budget reconsult does not replay the triggering input.
- `TestCanonicalReactionsAndBodyState`, `TestWorldAndFightPoliciesAreExplicit` and `TestInvalidCanonicalActionRequestsAreRefused` establish the new source contract. Remove compatibility/parity assertions whose only purpose was preserving abandoned formats.

- [ ] Add tests before production changes; move existing useful translation assertions to the canonical selected-answer path, rebuilding setup where needed.
- [ ] Implement ownership, dispatch, completion and pause/persistence together; publish meaningful checkpoints as drafts.
- [ ] Exercise real routing/Mover callbacks and stored EncounterData, not fake a successful position in a helper.

**Verification:** root encounter focused new suites plus rebuilt gameplay controls; `go test -race -count=1 ./...`, `golangci-lint run ./...`, scoped hook. Record focused mutation failures for dropped completion claim, double selection and false arrival; restore before final gates.

## Task 3: Site source binding and complete content fixtures

**Owner:** same encounter module/PR as Task 2. **Create:** `dungeonspec/behaviors.go`, `behaviors_test.go`, complete `testdata/behavior-patrol-consumer.yaml`. **Modify:** `single_room.go`/decode/compile/site grammar, `spec.go`/`compile.go` MonsterPlacement, existing bundle helpers only where contracts genuinely match.
**Interfaces:** root `behaviorDefinitions`; optional ordered binding `behaviors`, explicit `behaviorState`; proposed source types → BehaviorInput. [Source → instance contract](source-instance-contract.md) is the precise identity/value/handoff/refusal reference for this task. Native sequence/body payloads are carried as authored data, not interpreted by API/web.
**Behavior:** site-local named definitions; strict union/unknown-field/ref/value errors with source paths; inheritance resolves to a canonical ordered list; explicit lists replace it wholesale, and no roles means no ordinary behavior. `cells` items use existing RoomCell/roomCells/hexCell, validated as floor and lowered once. Validate emitted answer vocabulary/selectors with their actual owner; no second pathfinder or borrowed stateless-route overlap rule. With values are explicit; required missing values refuse. Definitions and use/binding values remain unexpanded in source serialization.
**Tests:** `TestBehaviorSourceSuite` — one source definition binds two/three items; complete compiled values reach MonsterPlacement; bad ref, duplicate IDs, wrong parameter kind, missing required value, off-floor cell, malformed body, missing body-state source and unsupported old-format fields refuse by exact source path; body/table mismatch fails before launch. Source decode/marshal retains uses, order, explicit empty versus absent and unknown carried Studio fields. The playtest site and affected fixtures are rebuilt with explicit canonical behavior configuration; old source/compiled-byte compatibility is not an acceptance gate.

- [ ] Add the boundary contract's named identity, actor-local uniqueness, source round-trip, single cell-lowering, detachment and source-path refusal assertions (`TestBindingsPreserveDefinitionAndAttachmentIdentity`, `TestDuplicateAttachmentIsLocalToActor`, `TestSourceRoundTripKeepsUsesAndValues`, `TestCellItemsAreLoweredOnce`, `TestBindingsCannotAliasDefinitionOrEachOther`, `TestBindingRefusalsNameTheOriginalSource`).
- [ ] Commit complete fixtures, not the walkthrough's outline with omitted bodies.
- [ ] Use the real production loader/expander and pass its output into Task 2's actor construction.
- [ ] Verify changed data against the Studio round-trip in Task 4 before declaring this new source field complete.

**Verification:** `go test -count=1 ./dungeonspec` plus focused joined encounter/castle tests; full owning-module gates as Task 2. No separate PR for a nested directory without its own go.mod.

## Task 4: Launch, source preservation and real API boundary

**Owners / PR boundaries:** toolkit `rulebooks/dnd5e/session`; web dev; API dev. These are separate PRs with one shared complete fixture and joined acceptance evidence, not three independent proof claims.

**Session files:** `launch.go` carries `MonsterPlacement.Behaviors` to `JoinInput`; `driver.go`, `compelled.go`, `capabilities.go` and the field-coverage ledger in `convert_test.go` preserve compulsion-first/default-driver delegation without leaking runtime objects through the host view. Existing Capabilities.Roller supplies shared dice; read capabilities still refuse rolling. Existing load-act-save paths keep EncounterData canonical. Create `behavior_patrol_test.go` and extend launch/boundary tests for the new plain-data field. No host-owned Composer or behavior logic. Update `go.mod/go.sum` via pushed provider commit, then actual tag at merge-out.

**Web files:** start with `src/concepts/world-building/singleRoomDungeon.test.ts` and the existing source/draft round-trip tests. Source content is authored in YAML; do not add BehaviorDefinitionsPanel, BehaviorBindingsEditor or a new editing DSL. Reuse root unread carry, binding unknown-field carry and source splicing. Change `siteScope.ts`, `roomDraft.ts` or `singleRoomDungeon.ts` only if a real round-trip case exposes dropped data. Server remains the semantic validator.

**API files:** module pins; `internal/orchestrators/authoring/behaviors_test.go`; `internal/integration/session/behavior_patrol_acceptance_test.go`. Existing PutDungeon/GetDungeon YAML transport, registry loader and sessionworld provider carry the new toolkit data. No new service/RPC is planned; if one becomes necessary, return the concrete contract gap rather than silently adding it.

**Named proofs:**
- `TestLaunchCarriesBehaviorConfiguration`: ordinary compiled dungeon → normal Launch → stored/reloaded EncounterData, with original config and each actor's continuation; no test-seeded runtime object.
- `singleRoomDungeon.test.ts` / existing serialization tests: import the complete authored YAML fixture and current served castle, make an ordinary supported Studio edit, save/reload/export → behavior definitions, bindings, item values/order, explicit empty/absent and unrelated fields survive. Repeat with a source YAML change from two to three items. Export compiles on the actual new server. The test, not an assumption, decides whether a small codec fix is needed.
- `TestPutDungeonBehaviorRoundTrip`: API stores/returns author source, not expanded runtime state; bad binding is toolkit-provided field error.
- `TestBehaviorPatrolAcceptance`: normally launch fixture through API; perform real game verbs advancing time, produce actual qualifying sound, save/reload between verbs and through one held walk; inspect actual positions and lawful audienced outcomes. Player/observer read does not receive another actor's private continuation.

- [ ] Develop outside-in with real pushed-commit pins and unchanged existing transport.
- [ ] Run focused checks, then the full owning-repository boundary gate before PR publication/update.
- [ ] Record exact tested content hash and provider pins for the joined path.

**Verification:** session module `go test -race -count=1 ./...` + lint; API `make test` and `make ci-check`; web focused Vitest during development, exactly one full `npm run ci-check` at PR boundary with its output. Proto work is not planned because current YAML and canonical saved-data transport suffice on inspected evidence.

## Task 5: Visible walk, release ordering and closure

**Owner:** cross-team coordination with monster-ai and Studio/API owners; no new rules here.
**Artifact:** one small open walk site cut from the served castle layout, kept local per walk policy; complete committed toolkit fixture remains the engine proof. Fold verified content into the standing QA castle as required by the walk contract, not a second permanent test world.

- [ ] Boot a builder-owned environment on non-operator ports using exact branch/pseudo-version pins; verify served content hash, not merely repository YAML.
- [ ] Put the visible head/try-it in front of the operator before independent review; operator look or explicit go precedes the walk/review stage.
- [ ] Walk actual positions: both loop lengths; only the hearing guard investigates; blocked/partial legs retain progress; interruption resumes the pending waypoint; reload/held resume preserves work; designated body/post behavior and the rebuilt site's other guards meet their authored contracts.
- [ ] Record evidence per #866 checklist, complete one independent round per feature PR, and resolve findings. No full-story completion from component-only tests.
- [ ] After explicit merge authorization: behavior provider → CI tag → encounter → tag → session → tag → API/web adoption. Re-pin and rerun affected gates at every real tag; project records follow. No hand tags, replacement modules or force pushes.

No release happens as a development prerequisite. If R4 leads to no provider extension, remove Task 1 and its pin step explicitly rather than keep an unused tool.

## Requirement coverage check

| Requirement / scenario | Tasks | Concrete proof |
|---|---|---|
| One authored definition, two real routes | 3, 4, 5 | Real loader/expander → normal Launch → observed positions |
| Observer-safe qualifying disturbance | 2, 4, 5 | Delivered kind/order/unknown tests and one lawful hearer in live walk |
| One selected answer and real movement | 2, 4 | No-second-roll test, actual router/Mover budget and outcomes |
| Partial/blocked/interrupted progress | 2, 4, 5 | No completion before target; retain goal; resume without nearest-waypoint reset |
| Pause/reload/original configuration | 2, 4 | Typed held claim and normal persisted encounter through reaction/next verb |
| Body/post and current-format gameplay | 1, 2 | Designated state source, explicit reaction/terminal/combat cases and rebuilt post/barred controls |
| Typed delegation and separated opportunities | 1, 2 | No flag/value mode, compulsion precedence, fight-only/world-only/both-beat tests; sleeping role does not run early |
| Studio/source/compile round trip | 3, 4 | Served castle + new fixture import/edit/store/export/server compile |
| Real player-visible acceptance | 5 | Exact-content local stack and #866 walk receipt, not simulated geometry |

## Provider/consumer seam check

| Provider → consumer | Produced vs consumed contract | Availability / proof |
|---|---|---|
| Released sequence tool → source compiler | Existing ExpandSequence input/output, scalar parameters and mapped completions | v0.20.0 exists; Task 3 adapter tests |
| Composer reads/wake shift → encounter | Exact Task 1 status/state/digest/listener/due values | #2292 implemented/reviewed; #2293 pins its pushed commit; use in the runtime remains Task 2 |
| Source compiler → session → encounter install | BehaviorInput ordered instances and explicit body source | New plain-data fields, Task 3/4 launch tests; no session rules |
| Encounter selection → translator → execution | Already-selected Answer, real TurnIntent, typed BehaviorWork | Task 2 one-roll/arrival tests; pause copy and loader must carry same claim |
| Saved encounter → LoadComposer | Original materialized Instances/completions/body source + ComposerData | No source lookup; corruption/reload/held claim tests |
| Server source grammar ↔ Studio codecs | Same named fields, explicit presence/order, server-owned errors | Same complete fixture and current served castle; Task 4 joined round trip |
| Provider tags → API/web walk | Real resolvable pins, normal images, actual served content | Task 5 hash/pin evidence; no local source override |

**Check findings:** The operator settled R4–R7 by adopting the review changes. This revision removes the permanent opt-in execution fork, bool/value delegation, per-duty combat guard and first-cut editor scope. The inspected code also proves turn-end is not arrival, heard kind is absent, listener concatenation loses scopes, table description assumes one table, and per-instance wake admission is needed when another role is active. Named tasks/tests cover those seams. R9 removes old-format compatibility and migration parity from scope. Rebuild the playtest site/fixtures and test the agreed current-format gameplay contracts. The plan is executable; proceed through its bounded tasks without another general approval gate.
