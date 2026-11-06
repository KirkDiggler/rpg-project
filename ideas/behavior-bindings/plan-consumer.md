# Live Patrol consumer implementation plan — conditional draft

> For execution after the consumer contract is agreed: use the repository's build-slice workflow and direct task-by-task execution unless delegation is separately authorized. This request is planning only.

**Goal:** Complete [NPC behavior S1's live adoption, #866](https://github.com/KirkDiggler/rpg-project/issues/866), under [story #859](https://github.com/KirkDiggler/rpg-project/issues/859): one authored Patrol definition, two-/three-waypoint guards, real movement, qualifying disturbance, retained resumption and save/reload, with source surviving Studio edits.

**Architecture:** [Consumer design](consumer-design.md), [walkthrough](consumer.md), inherited [R1–R3](design.md). Recommended R4 uses one composer for an opted-in actor and an explicitly designated instance for existing body/post state. Legacy actors remain unchanged. Encounter owns input/opportunity/execution/completion; session/API carry and store toolkit values.

**Tech stack:** Go module boundaries in toolkit; existing dungeonspec YAML and canonical behavior JSON; React/TypeScript Studio codecs; existing authoring/session RPCs and API integration harness.

**Readiness: BLOCKED on R4.** This is a concrete conditional task plan for the recommended cut, not a checked execution authorization. The coverage/seam pass below identifies that blocker rather than burying it in an implementation step. Choosing the two-runner alternative requires revising Tasks 1–3 before execution.

## Global constraints and finish line

- Consume released `mind/behavior/v0.20.0`; any needed provider extension gets its own nearest-go.mod PR and real pushed-commit pseudo-version, followed by its actual tag before consumer merge.
- No runtime sequence interpreter, Patrol-specific factory, second weighted roll, omniscient knowledge projection, source rebind on reload, general scheduler or effects work.
- Preserve existing queued-beat timing, world/fight budgets, pause accounting and body/post behavior for legacy actors. `barred` is the existing synchronous exception; other queued inputs are not flushed by it.
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

No new consumer tests ran. Web/API paths were inspected only. No dependencies were installed or stack started.

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
4. A profile coexists with creature-state blocking/post settings; physical permission has one designated state source, not a mirrored blob (Task 2, blocked on R4).
5. A valid new source field survives import/edit/local save/export/server compile; legacy/unknown fields and explicit absence remain truthful (Tasks 3–4).

## Proposed interface table — conditional on recommended R4

Names below are the exact target for this draft, not claims these APIs already exist.

| Owner | Proposed interface | Meaning |
|---|---|---|
| `mind/behavior` | `(*Composer).Inspect(*InspectComposerInput) (*InspectComposerOutput, error)` | `Now uint64` in; `Instances []InstanceStatus` out. Each status carries ID, State, Definition digest, copied Listens, optional DueAt and WakeAt. Pure derived read over the fixed instance list. |
| `mind/behavior` | `(*Composer).StateOf(*InstanceStateInput) (*InstanceStateOutput, error)` | ID → State + Definition digest, O(1) and detached strings; avoids copying every listener on each body-permission read. Unknown ID wraps existing ErrUnknownInstance. |
| `mind/behavior` | `(*Composer).DelayWakes(*DelayWakesInput) (*DelayWakesOutput, error)` | `Span uint64` → `Shifted int`; shift only non-nil WakeAt under existing encounter elapse policy, never anchors. Preflight overflow; error leaves every blob unchanged. No owned clock/timer. |
| `encounter` | `BehaviorSlot{Bound *BoundBehavior, Creature *CreatureBehavior}` | Exactly one arm. Bound carries InstanceSpec + Completions. Creature carries an ID and explicitly imports already-layered creature recipe/table after normal defaults resolve. |
| `encounter` | `BehaviorInput{Slots []BehaviorSlot; BodyInstance string}` | Optional pointer on MemberInput/JoinInput/MonsterPlacement; nil means legacy. Non-nil empty means no roles, not a hidden legacy fallback. BodyInstance identifies the imported state source when blocking/posts need one. |
| `encounter` | `BehaviorData{Instances []behavior.InstanceSpec; Completions map[string]map[behavior.AnswerKey]machine.Beat; BodyInstance string; Progress behavior.ComposerData}` | Materialized canonical saved values; no callbacks/source registry. No second live Machine blob/Table/Recipe copy on a composed member. |
| `encounter` | `MonsterView.HasBehaviors bool`; `Decision.UseBehaviors bool` | Default TableDriver returns explicit delegation without rolling for an opted-in actor; exactly one of delegated choice or concrete intent is valid. Existing compulsion/custom-driver wrapper runs first. |
| `encounter` | `IntentForAnswer(*IntentForAnswerInput) (*IntentForAnswerOutput, error)` | Existing MonsterView + chosen Answer → existing TurnIntent; extract current translation, no random selection or new rules. |
| `encounter` | `BehaviorWork{Instance, Definition, State string; Key behavior.AnswerKey; On machine.Beat; Goal *spatial.Position; Kind BehaviorWorkKind}` | Caller-owned claim for the selected action. Initial completion kinds are exact-cell arrival and delivered line/explicit hold; unsupported mapped completions refuse at configuration ingress rather than guess. |
| `encounter` | `TurnPauseData.Behavior *BehaviorWorkData` | Persist that selected claim across the existing held turn. Validate ID/digest/key/beat/target against saved configuration. Nil retains existing non-behavior pause meaning; no claim of cryptographic integrity against deleting an entire optional configuration. |
| `dungeonspec` | `BehaviorDefinitionSpec`, `BehaviorBindingSpec` | The source union in the walkthrough; definitions are site-local, bindings ordered, `items: cells` uses existing RoomCell owner and `with` uses the scalar schema. No invented defaults. |

For the explicit creature import, retain the existing runtime defaults/layering once at installation, materialize that imported InstanceSpec, then persist it with the other instances. Do not save both the old authoritative Machine blob and its imported composer clone. If legacy state/orders are authored but no explicit slot consumes them, refuse the conflict instead of ignoring source. Only the designated body instance's transitions feed existing physical state projections/BeatState; private duty states are not broadcast as extra client knowledge.

## Task 1: Composer introspection and wake adaptation

**Delivers:** Read-only scheduling/body-state access and the existing encounter's explicit time-jump operation; no new scheduler.
**Owner:** toolkit `mind/behavior`, separate provider PR. **Prerequisite:** R4 agreement.
**Files:** create `mind/behavior/inspect.go`, `inspect_test.go`, `wake_test.go`; modify `composer.go` only for explicit wake mutation if needed; update `doc.go`/README/run record.
**Interfaces:** the three methods/types above; use stored programs/blobs, not recompile snapshots in the encounter or add a second progress store.
**Tests:** `TestComposerInspectSuite`:
- Two instances with `heard` scopes 2 and 8 retain both descriptions; mutating returned scope data changes no instance.
- Active timed instance reports its next due; a terminal/untimed instance reports none; future wake is preserved; StateOf returns only the addressed instance's state/digest.
- Empty composition returns a valid empty result; nil input and unknown instance produce existing named errors.
- DelayWakes(5) shifts wakes 8→13, leaves nil wakes/anchors/state/bags unchanged; overflow refuses with byte-identical snapshots.

- [ ] Write named tests, observe meaningful red, implement the methods and run full module gates.
- [ ] Publish provider commit for consumers; no provider merge is needed to start local integration.

**Verification:** `cd mind/behavior && go test -race -count=1 ./... && golangci-lint run ./...`; actual scoped hook. Consumer pins pushed commit during development, real provider tag before merge.

## Task 2: Encounter ownership, one selection and actual completion

**Delivers:** The real state/execution/persistence path. **Owner:** toolkit `rulebooks/dnd5e/encounter`, same PR as Task 3 because they are one release unit. **Prerequisites:** Task 1 provider pin; R4.
**Create:** `behavior.go`, `behavior_test.go`, `behavior_work.go`, `behavior_work_test.go`, `behavior_data_test.go` in the encounter module.
**Modify inspected files/symbols:** `field.go` MemberInput/memberRecord; `encounter.go` Join/Board copies and construction; `data.go` MemberData/ToData/Load; `machine.go` program/wake adapters and state projection; `wakebeats.go` land/match/re-arm; `noise.go` makeSound/landHeard; `tabledriver.go` translation; `clocks.go` runIntents/executeTurnIntent/record table description; `pause.go` turnPause/TurnPauseData/copy/validation/finishPausedIntent; `post.go` and `cellfacts.go` state reads. Keep old branches unchanged for nil BehaviorInput.

**Execution contract:**
- Install one Composer plus original BehaviorData configuration for opted-in actors. Resolve explicit creature import once; physical state reads StateOf(BodyInstance), never a copied blob.
- Derive due/listener indexes from Inspect. Preserve all role listeners; a conservative unscoped actor-level union may dispatch to the composer, whose guards recheck exact scope. Never concatenate into first-per-beat and lose a broader scope. Rebuild indexes at load, do not persist them or scan the roster on every clock raise.
- Preserve owed-round ordering and elapse semantics. Use existing world high-water/delivered event timestamps consistently; do not compare a world anchor to an unrelated fight-round counter. Keep clock.kind available as an encounter-supplied fact so Patrol/Investigate eligibility is authored for world opportunities and an explicit baseline covers combat.
- A queued input remains queued; barred is still synchronous and targeted. The caller explicitly distinguishes data-only observation from an existing permitted reaction/time opportunity. Do not treat ordinary observation or completion as another paid turn. Reuse the same permission/budget path for legacy and composed decisions.
- Add the delivered kind to wakeBeat/wakeEv and their saved form; `heard.kind` reads that occurrence. Older queued inputs lacking kind are unknown, not guessed. Never project its private Subject as an unseen maker identity.
- Keep Driver consultation first, preserving `session.compelledDriver` precedence. The default TableDriver returns `UseBehaviors` without rolling when HasBehaviors is true; concrete compelled/custom-driver decisions bypass the composer. Refuse a Decision that both delegates and supplies an intent/pick. The encounter's private `selectDecision` resolves a delegation by calling Composer.Decide once with the real view/facts/temper and existing Capabilities.Roller, then IntentForAnswer. Do not call TableDriver.Act again afterward. A missing Roller retains the existing named refusal, not a local RNG.
- Deliver an occurrence once. Any same-budget reconsult after a nonterminal intent is a new action-selection opportunity, not redelivery of the original heard/barred input. Preserve the existing reaction/world/turn permission distinction explicitly in the private selection context.
- Record the chosen table from its actual instance, with instance-qualified record identity, not `m.Table[chosen.Key]`. Reuse existing answer/movement projections; do not expose private role blobs in the client.
- Build BehaviorWork for a real eligible mapped result. A no-candidate hold carries no completion. Complete exact-cell Toward/Dash only after actual target arrival, including an already-there eligible selection; partial/blocked/downed/held outcomes do not complete. A delivered line/explicit hold has its own explicit completion kind, not a fake cell.
- Carry the work claim through repeated pauses and JSON reload; resume the held intent without selection. On completion validate its saved attachment/configuration/state/target relationship and deliver the mapped Observe once; update indexes, but grant no extra opportunity. Other or stale inactive-step claims cannot advance a different role.
- Invariant on errors remains existing encounter policy: caller discards the encounter rather than save partial mutation; do not invent rollback.

**Named assertions:** `TestBehaviorConsumerSuite` and `TestBehaviorPauseSuite`:
- Real two-/three-point actors with 10 feet per opportunity move at most two cells each; a leg farther away stays pending until arrival. No competing move, no second selection roll.
- Clang reaches only lawful listeners; voice/unknown do not activate investigation. A clang then voice queued before a raise retains both occurrence kinds in landing order.
- Investigation owns movement for its authored interval, then resumes the unfinished waypoint, not a geometrically chosen nearest point.
- A blocked goal keeps its index; remove the obstruction and the next priced opportunity continues. Already-there eligible work completes once; no budget does not.
- Reaction pause after one cell saves exact work, remaining budget and act; reload/resume causes no new roll; second pause carries the same work; drop or changed floor does not fabricate arrival.
- Different scopes on the same beat are not lost. No-listener/untimed/terminal actors remain off appropriate indexes.
- Imported body state opens/closes blocking/posts exactly as before; other role states do not; legacy `TestTheHoldIsAStateAtAPlace`, `TestABumpMatchesOnlyItsOwnBeat`, `TestBystandersHearTheAnswerOnTheNextRaise` remain green.
- Original configuration survives changed source; corrupt IDs/state/definition/completion maps/non-null held claims are refused. Old data with no profile follows the old loader.
- Enter/leave combat preserves pending duty and uses the authored baseline; a commanded turn executes the existing imposed intent with no composer selection/roll/progress change. Custom-driver concrete decisions and nil-profile legacy actors retain their existing path. Same-budget reconsult does not replay the triggering input.

- [ ] Add tests before production changes; extract translation with existing TableDriver tests as parity controls.
- [ ] Implement ownership, dispatch, completion and pause/persistence together; publish meaningful checkpoints as drafts.
- [ ] Exercise real routing/Mover callbacks and stored EncounterData, not fake a successful position in a helper.

**Verification:** root encounter focused new suites plus named legacy controls; `go test -race -count=1 ./...`, `golangci-lint run ./...`, scoped hook. Record focused mutation failures for dropped completion claim, double selection and false arrival; restore before final gates.

## Task 3: Site source binding and complete content fixtures

**Owner:** same encounter module/PR as Task 2. **Create:** `dungeonspec/behaviors.go`, `behaviors_test.go`, complete `testdata/behavior-patrol-consumer.yaml`. **Modify:** `single_room.go`/decode/compile/site grammar, `spec.go`/`compile.go` MonsterPlacement, existing bundle helpers only where contracts genuinely match.
**Interfaces:** root `behaviorDefinitions`; optional ordered binding `behaviors`, explicit `behaviorState`; proposed source types → BehaviorInput. Native sequence/body payloads are carried as authored data, not interpreted by API/web.
**Behavior:** site-local named definitions; strict union/unknown-field/ref/value errors with source paths; absent list keeps legacy/inheritance, explicit list replaces wholesale. `cells` items use existing RoomCell/roomCells/hexCell, validated as floor and lowered once. Validate emitted answer vocabulary/selectors with their actual owner; no second pathfinder or borrowed stateless-route overlap rule. With values are explicit; required missing values refuse. Definitions and use/binding values remain unexpanded in source serialization.
**Tests:** `TestBehaviorSourceSuite` — one source definition binds two/three items; complete compiled values reach MonsterPlacement; bad ref, mixed union arms, duplicate IDs, wrong parameter kind, missing required value, off-floor cell, malformed body, missing body-state source and unconsumed legacy fields refuse by exact source path; body/table mismatch fails before launch. Source decode/marshal retains uses, order, explicit empty versus absent and unknown carried Studio fields. Existing castle without profiles compiles identically.

- [ ] Commit complete fixtures, not the walkthrough's outline with omitted bodies.
- [ ] Use the real production loader/expander and pass its output into Task 2's actor construction.
- [ ] Verify changed data against the Studio round-trip in Task 4 before declaring this new source field complete.

**Verification:** `go test -count=1 ./dungeonspec` plus focused joined encounter/castle tests; full owning-module gates as Task 2. No separate PR for a nested directory without its own go.mod.

## Task 4: Launch, source preservation and real API boundary

**Owners / PR boundaries:** toolkit `rulebooks/dnd5e/session`; web dev; API dev. These are separate PRs with one shared complete fixture and joined acceptance evidence, not three independent proof claims.

**Session files:** `launch.go` carries `MonsterPlacement.Behaviors` to `JoinInput`; `driver.go`, `compelled.go`, `capabilities.go` and the field-coverage ledger in `convert_test.go` preserve compulsion-first/default-driver delegation without leaking runtime objects through the host view. Existing Capabilities.Roller supplies shared dice; read capabilities still refuse rolling. Existing load-act-save paths keep EncounterData canonical. Create `behavior_patrol_test.go` and extend launch/boundary tests for the new plain-data field. No host-owned Composer or behavior logic. Update `go.mod/go.sum` via pushed provider commit, then actual tag at merge-out.

**Web files:** `src/concepts/world-building/{siteScope.ts,roomDraft.ts,singleRoomDungeon.ts,WorldBuildingConcept.tsx}`; create `behaviorDefinitions.ts`, `BehaviorDefinitionsPanel.tsx`, `BehaviorBindingsEditor.tsx` and focused tests. Reuse existing body text editing and route-picker/parameter controls; retain source references, binding order, unknown fields and original source-splice behavior. Client validation is structural/editor feedback only; server owns semantic refusal. Do not manufacture rule calculations or block a draft save on an unimplemented client judgment.

**API files:** module pins; `internal/orchestrators/authoring/behaviors_test.go`; `internal/integration/session/behavior_patrol_acceptance_test.go`. Existing PutDungeon/GetDungeon YAML transport, registry loader and sessionworld provider carry the new toolkit data. No new service/RPC is planned; if one becomes necessary, return the concrete contract gap rather than silently adding it.

**Named proofs:**
- `TestLaunchCarriesBehaviorConfiguration`: ordinary compiled dungeon → normal Launch → stored/reloaded EncounterData, with original config and each actor's continuation; no test-seeded runtime object.
- `behaviorDefinitions.test.ts` / `singleRoomDungeon.test.ts`: import complete fixture and current served castle; edit West's route using existing controls, save draft, reload, export → definition/reference/order/other fields preserved. Export compiles on the new server; refusal paths point to the edited binding.
- `BehaviorBindingsEditor.test.tsx`: change two to three route items, reorder authored roles, retain explicit empty versus absent; no client execution/expansion required.
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
- [ ] Walk actual positions: both loop lengths; only the hearing guard investigates; blocked/partial legs retain progress; interruption resumes the pending waypoint; reload/held resume preserves work; body/post behavior and legacy guards remain correct.
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
| Existing body/post and legacy semantics | 1, 2 | R4's designated state source; existing held-post/barred tests unchanged |
| Studio/source/compile round trip | 3, 4 | Served castle + new fixture import/edit/store/export/server compile |
| Real player-visible acceptance | 5 | Exact-content local stack and #866 walk receipt, not simulated geometry |

## Provider/consumer seam check

| Provider → consumer | Produced vs consumed contract | Availability / proof |
|---|---|---|
| Released sequence tool → source compiler | Existing ExpandSequence input/output, scalar parameters and mapped completions | v0.20.0 exists; Task 3 adapter tests |
| Proposed Composer reads/wake shift → encounter | Exact Task 1 status/state/digest/listener/due values | New provider, blocked on R4; scope/alias/overflow tests |
| Source compiler → session → encounter install | BehaviorInput ordered slots and explicit body source | New plain-data fields, Task 3/4 launch tests; no session rules |
| Encounter selection → translator → execution | Already-selected Answer, real TurnIntent, typed BehaviorWork | Task 2 one-roll/arrival tests; pause copy and loader must carry same claim |
| Saved encounter → LoadComposer | Original materialized Instances/completions/body source + ComposerData | No source lookup; corruption/reload/held claim tests |
| Server source grammar ↔ Studio codecs | Same named fields, explicit presence/order, server-owned errors | Same complete fixture and current served castle; Task 4 joined round trip |
| Provider tags → API/web walk | Real resolvable pins, normal images, actual served content | Task 5 hash/pin evidence; no local source override |

**Check findings:** R4 is a real ownership/public-contract blocker, not a missing helper to improvise. The inspected code also proves that turn-end is not arrival, heard kind is not currently delivered, listener concatenation loses scopes, and current table-description lookup assumes one member table. These are assigned to explicit tasks rather than left behind “wire it up.” No runtime implementation is authorized from this conditional draft until R4 and the resulting interface table are reconciled with the operator's decision.
