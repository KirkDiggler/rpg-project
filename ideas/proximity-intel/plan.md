# Automatic discovery — implementation plan

## Goal, authority and boundaries

Implement [design.md](design.md), R1–R17, recorded in
[rpg-project#523](https://github.com/KirkDiggler/rpg-project/issues/523) and
[design PR #524](https://github.com/KirkDiggler/rpg-project/pull/524).
The operator's final scope decisions remove Search, put failed checks through
sharing, and make authored checks immutable after dungeon load.

**Playtest/review checkpoint (2026-10-04):** the operator confirmed sharing and
requested cleanup; `local/discovery` is now removed (former web 3023 / API 8093).
Source worktrees/commits and local content/log archive are preserved. Protos #373
is merged/generated as v0.1.220. Review heads are encounter #1931 (`66306c9f`),
session #1930 (`d140e2a1`), API #1072 (`95db96bd`) and web #1219 (`9bf39d09`), all
pushed, with green current code-PR CI. The walk used API `23f50fb2` / web
`22fb3ceb`; subsequent changes clean unused checksums/lint and align obsolete
Search/header tests with the walked behavior. Full API and web local gates pass
(web: 7349 passed, 5 skipped). Native rendered smoke confirms an automatic failed Perception check, no
Search button, no second attempt after an observed departure/return, and private
sharing surviving reload. The smoke used `level-up-fighter`, not the fresh
sandbox fighter/barbarian offered to the operator. Broader party/authoring,
edge-case acceptance and independent-review work remains pending, following the
operator's walk-first direction. No provider merge, deployment or whole-wave completion
is implied by this local checkpoint.

**In scope:** automatic authored discovery rolls at distance <= 1 hex; one try
by default; explicitly configured repeat allowance, reset distance default 3;
per-run or retained attempt lifetime; real character checks; sharing and failure
logs for the loaded party; per-character sharing preference default on; removal
of the whole-region Search path; optional-content acceptance proof.

**Not in scope:** passive-score evaluation, automatically unlocking/forcing,
general party sight aggregation, new trap mechanics, live dungeon authoring,
late-join backfill, disconnect recovery, a new LOS system or pub/sub transport.
Existing skill approaches remain data, not a Perception-only switch.

### Inspected baseline

All source inspection uses dedicated `.worktrees/proximity-intel` worktrees.
Rebase/recheck these seams if providers change before implementation.

| Repository | Revision | Relevant state |
| --- | --- | --- |
| rpg-toolkit | `3955eaf7094c56a713ae055f44600c125081c4ce` | Individual room/object knowledge merged; encounter and session independently released |
| rpg-api | `66a36417f0c501bd5237230f625b3b59bd9ab957` | encounter v0.110.0, session v0.113.0; #1068 merged |
| rpg-api-protos | `e80efe0fb99255259b247676f60b9a2c2bf804a8` | session knowledge contract; generated release v0.1.219 |
| rpg-dnd5e-web | `af4ee892304671d02cdec9bb6323d5152d9f698b` | #1217 merged; uses v0.1.219 |
| rpg-project | plan branch based on `c0010a0` | Design skill and implementation-planning guide loaded |

The earlier #523 baseline described #508 as in flight. Its provider/consumer PRs
are now merged: toolkit#1923/#1924, api#1068, web#1217. Do not implement against
the old full-live-atlas behavior.

## Contract decisions derived from the agreement

These are the concrete interfaces the tasks implement, not existing APIs unless
explicitly called existing. Any inability to preserve their behavior is a plan
finding, not permission to silently substitute another contract.

### C1 — One authored discovery check, alternative approaches

The existing `ConcealmentInput.Checks` / `ConcealmentSpec.Checks` list contains
alternative approaches to **one** discovery check. Keep the existing resolver's
best-listed-approach choice and one attempt per concealment, not one per row or
hidden member. The World Builder labels this as one automatic discovery check
with its approaches; retry settings live beside those approaches, not on the
site/world and not on the lock's active checks.

The policy checkpoint adds `ConcealmentInput.Attempts *DiscoveryPolicyInput`,
normalizes through `ResolveDiscoveryPolicy`, and stores a detached effective
`ConcealmentData.Attempts *DiscoveryPolicy`. V4 `attempts` has optional fields:

- `max`: positive integer; absent = 1. This is the total permitted attempts,
  including the first, not a count of additional retries.
- `reset_hexes`: integer > 1; absent = 3. No fractional distance or in-range
  reset. Used only when another attempt is permitted.
- `lifetime`: `character` or `run`; absent = `character` (single try retained,
  with reset on a new run requiring an explicit author choice).

Use pointer/presence fields at decoding boundaries so explicit zero, negatives,
unknown lifetime, null, and wrong-shaped policy can be refused rather than
silently treated as omitted. An empty mapping omits all fields and therefore
requests the same single-try defaults as an empty Go policy input. Normalize once in the owning compiler. The existing
v2 lowering has no retry editor; it produces the same single-try defaults.
Do not add unlimited retries or a second counter until a use case asks for them.

The dormant `notice` list **continues to mean literal passive-score content**;
it is not the new roll path. Preserve its carry/validation behavior and make
that distinction legible in the builder. `checks` becomes automatic; Search's
name and invocation are removed.

### C2 — Distance, attempts and carry-forward

Encounter owns the state machine and all geometry. Measure the minimum existing
grid distance to the concealment's hidden floor, hidden prop footprint cells,
and member-door support (edge endpoints or footprint cells). Reuse
`hiddenCellsOf`, `memberPropCells`, and door geometry rather than rectangle or
hex arithmetic in session/web. No LOS, movement-blocking or path-reach gate.

For each player/check: `Used`, `Armed`, and learned/succeeded knowledge. A new,
unknown check is armed. At distance <= 1, with allowance remaining and armed,
resolve once, increment Used and disarm, even on failure. At distance >= X,
re-arm only if repeats are allowed and allowance remains. Return to <= 1 to roll.
No extra world round/action is charged by the automatic roll. Read/load/refetch
never rolls. Already learned secrets do not consume another attempt.

Keep the live state on `EncounterData`; save/load preserves both count and armed
state. `character` lifetime exports count and learned discovery as an
encounter-owned persistence value (`DiscoveryMemoryData`); `run` lifetime is not
exported across runs. Import carried memory before a character's first discovery
sweep. A new visit does not manufacture a failed attempt or refill a retained
count. Re-entry can arm only an explicitly repeatable check; it cannot exceed
its maximum.

Use the existing compiler-minted `<dungeon-key>/<concealment-id>` as opaque check
identity, plus character identity and the host's trusted world namespace. Do not
hash DC/settings into identity: the same authored ID is the same retained check;
a distinct authored ID is distinct content. No live-edit migration exists.
Loaded `EncounterData.Field` remains the source for this run even if a registry
entry changes. Preserve known secrets in older blobs; absent automatic-attempt
state means no recorded automatic attempts, never fabricated historical Search
outcomes.

### C3 — Sharing and recipients

For this slice, loaded means **placed player members currently in the encounter**,
not active WebSocket subscribers. `Exit` removes eligibility; future `Join` does
not retroactively join an old audience. This uses the existing roster boundary
and deliberately does not implement the deferred disconnect edge.

Capture one recipient set for each attempt: actor only when private, otherwise
actor plus all current placed player members. Use that same set for the
check-result beat and successful discovery teaching. Never recompute it during
replay from today's roster or toggle. No monster audience or transitive sharing
loop. Changing sharing to on affects future discoveries, not a backlog.

Success uses the existing `revealConcealmentTo` writer per entitled recipient,
including its recipient-specific atlas patch and lawful observation refresh.
It does not copy the entire sender's sight store or teach unseen mutable world
state. Existing independent perception of open doors remains intact.

Persist the preference per character, not in browser local storage. Represent
omission honestly with `PrivateDiscoveries=false` meaning the agreed shared
initial policy; expose an explicit `sharing` bool in reads and toggle responses.
Both explicit false sharing and the default survive reload. Toggling does not
spend time, roll, unlearn, or alter an old event's audience.

### C4 — Host seam, persistence and wire

New session-owned persistence aggregate `ExplorationData`:
`Character string`, `PrivateDiscoveries bool`, and
`Checks map[string]encounter.DiscoveryMemoryData`. This is a persistence-type
boundary exception, not a runtime encounter object exposed to the host.

New `ExplorationRepository` capability, required for retained discovery and
sharing-preference operations but not an unconditional new constructor
requirement on existing hosts, with key-value methods
`GetExploration(ctx, characterID) (*ExplorationData, error)` and
`SaveExploration(ctx, *ExplorationData) error`. Missing uses `ErrNotFound`;
`nil,nil` is a repository defect. The SDK initializes genuinely missing profiles
under the agreed defaults. The implemented API store follows the existing
canonical character ID/authority rather than inventing a second identity from an
ambient player/world selector. Records do not inherit the expiring session TTL.
This does not close the separate world-isolation initiative. No new game rule goes
in the Redis adapter. `ExplorationData` contains opaque provider memory; the SDK
carries it and encounter interprets/merges it. API supplies the capability
explicitly. A world/operation requiring it must refuse before dice or mutation
when it is absent; never silently downgrade retained attempts to run-only state.
This follows the SDK's existing optional-capability adoption contract rather
than turning a compatible-looking Config addition into a universal runtime break.

New SDK verb:
`SetDiscoverySharing(ctx, *SetDiscoverySharingInput) (*SetDiscoverySharingOutput, error)`;
input is `{Session, Member string; Sharing bool}`, output is
`{Sharing bool; Saved SaveReport; Delivery DeliveryReport}`. Require a placed
player member. `KnowledgeOutput` adds `DiscoverySharing bool` for the caller's
own preference. No client-supplied audience, check ID, count or outcome.

New typed beat/event `discovery_checked` / `DiscoveryCheckedBody`:
`Member string`, `Ability string`, `Beaten bool`, `Total int`,
`Calculation *RollCalculation`. Copy the existing resolver's result; do not
recalculate it. No concealed subject/check ID, location, authored label or DC
is on the failure-bearing beat. Rich arithmetic remains provider data; there
is no interactive dice-release requirement for these automatic rolls.

Wire mirrors the SDK: `DiscoveryChecked` fields member=1, ability=2, beaten=3,
total=4, calculation=5; add `EVENT_KIND_DISCOVERY_CHECKED=42` and Event body tag
48 (unallocated at the inspected baseline; recheck before editing).
`GetKnowledgeResponse.discovery_sharing=7` is an **optional bool** so explicit
private is distinguishable from an older server lacking the field. Current
producer always sets it. Add `SetDiscoverySharing` request session=1, member=2,
optional sharing=3 (missing is invalid); response sharing=1, saved=2, delivery=3.
The generated contract retains Search as a deprecated wire tombstone during
consumer migration. The API removes its handler, so the inherited implementation
returns UNIMPLEMENTED and cannot roll. Automatic toolkit hosts also refuse Search;
older toolkit hosts without the automatic capability retain their prior contract.
The game UI no longer wires the Search action.

## Sequence and release order

Develop the web consumer contract/tests first (T5's isolated fixtures), then the
wire T3 and providers T1/T2 with the API T4; this exposes consumer needs without
waiting for toolkit releases. This is one wave, with one branch per owning
repository; toolkit releases still require **one nearest-go.mod module per PR**.
The encounter and session task branches/PRs must therefore be separate release
units, with the same wave recorded in #523.

Protos needs its operator-authorized merge/generated release before real Go/TS
consumers can compile. No plan entry itself authorizes merging. Develop consumers
against pushed toolkit commits with real Go pseudo-versions, using the same graph
locally and in CI. No local replaces, go.work or source-copy build loop. Walk the
integrated wave before toolkit publication.
Then publish encounter, adopt its minted tag in session, publish session, repin
API, and release API/web against the published contracts. Re-run joined checks
after replacing development pins. Do not hand-tag or merge to make development
possible. No independent worker delegation is requested by this plan.

## T0 — Complete-operation serialization prerequisite

**Status:** implemented, verified, draft/unmerged:
[toolkit#1930](https://github.com/KirkDiggler/rpg-toolkit/pull/1930) at `f6cec060`,
[API#1072](https://github.com/KirkDiggler/rpg-api/pull/1072) at `d604f13f`.
This is not automatic-discovery completion or release authorization.

**Contract:** SDK `Config.Locker SessionLocker` is optional only for hosts that
already serialize externally. `LockSession(ctx, *LockSessionInput{Session})`
returns `*LockSessionOutput{Release func()}`. Every public session-shaped Manager
operation acquires before repository access and defers release through its final
save/delivery or error. Reads and StartSession participate; AtlasOf and unseated
character operations do not. No mutex, lock data or timers live in the SDK.
Invalid successful lock responses refuse, and callback re-entry into the same
session is prohibited. Nil capability is not a concurrency guarantee.

**Ownership/files:** toolkit session `locking.go`, `locking_test.go`,
`locking_coverage_test.go`, Config/Manager in `session.go`, entry points in the
36 session-shaped methods, `doc.go`, `repositories.go`, sentinel allow-list.
API `internal/orchestrators/session/locker.go`, `locker_test.go`,
`orchestrator.go`, and `internal/integration/session/session_serialization_test.go`.

**Host implementation:** an in-process, cancellable keyed locker with reference
cleanup and idempotent release. API injects it at construction; custom/shared
coordinators can be supplied without a verb facade. All callers of one
Orchestrator share it. Separate managers over the same sessions must share the
coordination domain; independent process-local lockers do not protect replicas.
No cross-session character/profile exclusion or transaction guarantee is claimed.

**Proof:** SDK public-surface tests were red before guarding; full module race
suite and pinned lint now pass. Actual commit hook passes. Root make pre-commit
still hits the unrelated Core coverage-extraction defect (#769). API regression
pauses Move before encounter save: before wiring, a read sees the old position
and the next move refuses from that stale origin; after wiring, a waiting read
cancels and the next move observes the committed position. Covers one manager
and two managers sharing a coordinator. Host unit tests cover cancellation,
independent keys, idempotence, 12x100 contended updates and idle cleanup. Three
race repetitions, `make pre-commit` and `make ci-check` pass.

**Dependencies:** API pins the actual pushed session pseudo-version
`v0.113.1-0.20261003051441-f6cec060ed4c`. Replace it with a real provider release
before consumer merge. Independent review, release and deployment are pending.

## T1 — Authored policy and automatic discovery composition

**Checkpoint:** authoring/persistence policy implemented in draft
[toolkit#1931](https://github.com/KirkDiggler/rpg-toolkit/pull/1931), `2efc1234`,
branch `feat/523-automatic-discovery`, worktree `.worktrees/proximity-intel-encounter`.
Root policy and YAML refusal/round-trip tests pass; full encounter race suite,
pinned lint and actual hook pass. The two content pictures only gain omitted
Attempts fields; geometry is unchanged. That initial checkpoint was followed by `96d12c3b` / `66306c9f`: the automatic
sweep, counters/re-arm state, shared/private audiences, and own retained-memory
restore are implemented. Supplying DiscoveryCheckResolver enables them and
refuses Search. Runtime tests and the local SDK/browser path exercise this
capability; older hosts' fixtures remain compatible. Full wave acceptance and
review are still separate from this module checkpoint.

**Delivers:** R1–R4, R6–R13, R15–R17; C1–C3.
**Owner:** toolkit `rulebooks/dnd5e/encounter` (including its dungeonspec package).
**Prerequisites:** existing CheckResolver and individual knowledge writers.
**Existing files:** `concealment.go`, `conceal.go`, `search.go`, `encounter.go`
(`refreshSightDeclaring`, `Join`), `step.go`, `clocks.go`, `data.go`,
`field.go` (`MemberInput`, `JoinInput`); `dungeonspec/single_room.go`,
`dungeonspec/single_room_concealments.go`, `dungeonspec/concealments.go`.
**New files:** `discovery.go`, `discovery_test.go`, `discovery_memory.go`,
`discovery_memory_test.go`, `dungeonspec/discovery_policy_test.go`.

**Interfaces:** C1 policy; C2 per-check `DiscoveryMemoryData{Used uint32,
Learned bool}`; add `RetainedDiscoveries map[ConcealmentID]DiscoveryMemoryData`
and `PrivateDiscoveries bool` to the player `JoinInput`/setup `MemberInput`.
Join imports this state after member registration and before first sweep.
`DiscoveryMemory(*DiscoveryMemoryInput{Member})` returns
`*DiscoveryMemoryOutput{Checks map[ConcealmentID]DiscoveryMemoryData}` for
character-lifetime entries only. Add `SetDiscoverySharing` and
`DiscoverySharing` at composition level with member-shaped Input/Output types;
C4 typed beat. Default sharing for a new member is on. Import is self-knowledge,
not a new party share, so joining never broadcasts a history backlog.

**Behavior:** the composition's existing refresh/placement lifecycle runs one
proximity sweep for players, after a successful placement/move and before fight
formation consequences. It must work for public Step, directed movement and
Join, not only a web Move call. Guard initialization/loading: constructors used
by authoring and `LoadEncounter` must not roll merely by validating/restoring.
Use sorted player/check IDs for deterministic multiple-check order. Successful
reveal may refresh perception but must not recursively launch another sweep.
Re-evaluate eligibility before each roll because an earlier shared discovery
may already have taught a later candidate.

Delete the `Search` method, types and region-sweep helper after replacing their
callers/tests. Do not retain it as a hidden attempt-budget bypass or spend a
world action on automatic checks. Export only persistence data, not host storage.

**Tests (new `DiscoverySuite` unless noted):**
- `TestOneHexRollsButTwoDoesNot`, including player initially placed in range.
- `TestBlockedHexDoesNotGateDiscovery`; movement into that blocker still refuses.
- `TestDefaultAttemptSurvivesDepartureAndReload`; a second player has its own try.
- `TestThreeHexRearmAndFiniteBudget`: max=2, X=3; 1→2→1 stays at one roll;
  1→3 rolls nothing at 3; 3→1 rolls exactly once; later loops never exceed two.
  Parameterize X=4 to prove the author setting is read.
- `TestRearmCountsDistanceNotSteps`; movement within the ring cannot re-arm.
- `TestMultipleMembersOfOneSecretDoNotMultiplyAttempts`.
- `TestSharingAudiencesAreCapturedOnce`: private versus shared success AND
  failure; actor always included; monsters/exited/future players excluded;
  toggle-off preserves earlier knowledge and old log audiences.
- `TestKnownSecretDoesNotReroll`; no bulk revelation of another room's contents.
- `TestRunAndCharacterLifetimes`, `TestLoadedPolicyDoesNotReadAuthoringAgain`.
- `TestFailedResolverWritesNoAttemptOrResult` (discard errored instance).
- policy suite: omitted defaults, positive overrides, explicit zero/negative,
  unknown lifetime, reset <=1 refusals; YAML→compiled→data→load round-trip;
  dormant `Notice` remains unread and is not a source of dice.

- [ ] Add failing feature cases; retain appropriate existing privacy tests.
- [ ] Implement policy, state machine, captured audiences and carry-forward.
- [ ] Remove Search; replace search-driven fixtures with real proximity moves.
- [ ] Update `doc.go` and owning concealment comments to the new rules.
- [ ] Run focused cases, then full module race suite and pinned lint.

**Verification:** from this module:
`GOWORK=off go test -mod=readonly -race -count=1 ./...` and
`golangci-lint run ./...` with the repository-pinned toolchain. Expected PASS;
no fixture may replace real proximity detection with an explicit reveal call
when it claims to test automatic discovery.
**Evidence:** encounter PR, named assertion outputs, exact provider commit/tag.

## T2 — SDK wiring, durable exploration and removal of Search

**Delivers:** R1–R3, R5–R8, R10, R12–R17; C2–C4.
**Owner:** toolkit `rulebooks/dnd5e/session`.
**Prerequisites:** T1 buildable provider; existing `resolution.MakeCheck` and
`ResumeCheck` (no new resolution module behavior required).
**Existing files:** `session.go`, `repositories.go`, `write.go` (`Join`,
`openForWrite`, `writeScope`, `commit`, `persist`), `conceal.go`, `move.go`,
`start.go`, `knowledge.go`, `events.go`, `types.go`, `search.go`, and the module's
shared repository/test fixtures, `boundary_test.go`, `doc.go`.
**New files:** `exploration.go`, `exploration_test.go`, `discovery_sharing.go`,
`discovery_sharing_test.go`, `automatic_discovery_test.go`.

**Interfaces:** conditionally required ExplorationRepository capability and C4
public types/verb; add the
new persistence type to the intentional boundary allow-list. Project the new
beat through both live delivery and Story with existing dense recipient Seq.

**Behavior:** stage real player records before any composition call that can
trigger discovery, not only when a hidden object is encountered. Extend common
write-scope setup and Join staging rather than installing a Move-only resolver.
Re-use `resolveStagedCheck`'s existing noninteractive resolution, including its
existing Keep response to optional check offers; do not introduce an interrupt
prompt or charge Search's world action.

A check can change a character record. Preserve one coherent per-verb record:
update staged data after DirtyCharacter, and when the checker is `scope.walker`,
make the resulting record the walker's record before the subsequent movement
payment/save. Do not overwrite movement, conditions or check resources with an
older staged copy. Reuse existing actor/adoption write paths where applicable.

Before Join can sweep, hydrate the character's private/shared setting and
retained DiscoveryMemory. On successful verbs export provider-owned changes to
the exploration aggregate, then save encounter/session/cursors before publishing.
Report partial persistence through existing SaveReport/SaveError; never claim
transactional rollback that the SDK does not provide. A failed store operation
must not be converted to a gameplay failed check. Reads do not stage or roll.
No expiry of a session may erase the separate retained profile.

**Tests:** `AutomaticDiscoverySuite` proves world-clock Move, turn-clock Move,
Join, directed move/resume, multiple secrets in one path, save/reload between
outward and return legs, and API-independent sheet-based rolls for Religion,
Investigation, Survival and Perception. Assert no extra action/round charge,
correct movement payment, and no resurrected consumed condition across two rolls.
`ExplorationSuite` proves retained versus per-run counters, missing versus broken
repository responses, own learned knowledge carry-forward, profile isolation,
partial-save reports and no publication before saves. `DiscoverySharingSuite`
proves explicit off persistence, toggle ownership/member refusals, snapshot bool,
no catch-up on on-toggle/late Join, identical captured audiences in live/Story.

- [ ] Introduce the repository capability, enforce it at discovery-world/verb
  admission before mutation, and update the affected host/constructor fixtures.
- [ ] Wire staging, current record reuse and durable state through the common seam.
- [ ] Add toggle/snapshot/result projection and delete SDK Search types/method.
- [ ] Replace existing Search test setup with proximity or lawful fixture knowledge.
- [ ] Run focused suites, boundary/no-rule/no-bus tests, full module race and lint.

**Verification:** from session:
`GOWORK=off go test -mod=readonly -race -count=1 ./...`;
`golangci-lint run ./...`. Record provider revision and committed pins. A compiling
SDK using a local override is not proof of released dependency adoption.
**Evidence:** session PR, shared-state/resource regression results, host seam checks.

## T3 — Wire contract and generated release

**Delivers:** R3, R5–R7, R10, R16; C4.
**Owner:** rpg-api-protos, `dnd5e/api/session/v1alpha1`.
**Prerequisites:** C4 consumer contract; generated release needed by T4/T5.
**Files:** `service.proto`, `events.proto`; owning session docs if they describe
Search. No hand-edited generated code or proto serialization tests.

**Interfaces/behavior:** exactly C4. Preserve recipient-local envelope fields and
existing `RollCalculation`. Remove Search completely; keep active Unlock untouched.
This is an intentional alpha RPC removal, not an excuse to suppress other Buf
findings. Follow `docs/how-to/breaking-change-workflow.md`; apply an intentional
breaking-change approval only through its documented review process.

- [ ] Add event, snapshot preference and toggle RPC; remove Search contract.
- [ ] Recheck all new tags against the current branch before assigning them.
- [ ] Run `make format`, `make test`; enumerate expected Search-removal findings.
- [ ] Verify generation/SDK CI and wait for the operator-authorized merge/tag.

**Verification:** repository `make format && make test`; breaking detection must
report only the deliberately retired Search contract. CI owns generated bindings.
**Evidence:** contract PR and generated tag, not a locally authored fake version.

## T4 — API storage, authentication and event translation

**Delivers:** R2–R3, R5–R8, R10, R15–R17; C4.
**Owner:** rpg-api.
**Prerequisites:** T2 SDK + T3 generated contract; provider development pins allowed
until the final released-pin gate.
**Existing files:** `internal/orchestrators/session/orchestrator.go`,
`internal/handlers/dnd5e/session/v1alpha1/{handler.go,convert.go,get_knowledge.go,search.go}`,
`internal/handlers/dnd5e/session/v1alpha1/mock/mock_manager.go`,
`internal/orchestrators/lobby/start_encounter_session_stack.go` only if constructor
wiring needs it; `go.mod`, `go.sum`, `internal/auth/role_policy.go` contract tests.
**New files:** `internal/orchestrators/session/exploration_repo.go` and
`exploration_repo_test.go`; handler `discovery_sharing.go` and
`discovery_sharing_test.go`.

**Interfaces:** adapter implements only C4's repository; use `worldcontext`'s
trusted value for namespace, never a client world ID. Unknown context refuses,
not a fallback into a global key. Store opaque SDK JSON with no session TTL.
The handler's consumer Manager interface adds SetDiscoverySharing, loses Search;
regenerate its gomock rather than editing it. Authenticated caller must control
the exact member seat for reads/updates; an unrelated member/session is refused.
Explicitly missing toggle bool is InvalidArgument, not false. The response is
only the requested preference/save/delivery, never peers' discovery state.

**Behavior:** event mapping is field-for-field. Both GetStory and StreamEvents
must spell the new event rather than UNKNOWN/drop. No API proximity tests,
DC comparison, count mutation, audience reconstruction or subscriber filtering.
Delete Search handler and its test contract; direct old Search calls cannot roll.
Update the exact SessionService method list in `internal/auth/role_policy.go`:
remove Search, add SetDiscoverySharing as player-authorized, and update its
interceptor tests. Otherwise the new handler compiles but the role gate refuses
it. Preserve SDK partial-save/delivery errors. Loaded roster, not Broker subscriber
count, determines C3; no new online-presence registry.

**Tests:** `ExplorationRepositorySuite` uses miniredis: same character/site key in
two trusted worlds remains distinct; round-trip private + count + learned state;
no session TTL; nil data/empty identity/corrupt JSON/backend failure refuse.
Handler suite: caller/seat authorization, omitted/false/true toggle, SDK refusal
translation, snapshot preference. Conversion cases assert result skill/total/
calculation unchanged and no hidden target/DC fields. Live and Story tests assert
the same recipient-specific result with no sequence holes for excluded players.

- [ ] Wire profile repository, constructor fixtures and host world context.
- [ ] Add handler/mapping, regenerate consumer mocks, remove Search.
- [ ] Run focused packages and T6 integration against actual SDK providers.
- [ ] Adopt released tags and run `make ci-check` before the PR boundary.

**Verification:** repository commands:
`go test -race ./internal/orchestrators/session ./internal/handlers/dnd5e/session/v1alpha1`;
`go test -race ./internal/integration/session -run 'TestAutomaticDiscoveryAcceptance'`;
`make ci-check` for final API validation. First-run failures caused by a missing
new provider/generated tag are prerequisites, not a reason to invent local types.
**Evidence:** API PR, authenticated transport/storage cases, pinned release versions.

## T5 — World Builder controls and player UI

**Delivers:** R1–R5, R9–R13, R15–R17.
**Owner:** rpg-dnd5e-web.
**Prerequisites:** C1/C4 fixtures first, then T3 bindings and T4 API.
**Existing files:** `src/concepts/world-building/{ConcealmentPanel.tsx,CheckApproachRows.tsx,siteScope.ts,concealmentEdits.ts,roomDraft.ts}` and their carry/persistence tests;
`src/api/useSessionKnowledge.ts`, `src/api/useSessionSearch.ts` (delete);
`src/components/session/{SessionEncounterView.tsx,searchNotice.ts,sessionRefreshKeys.ts,debugLogLine.ts}`;
`src/components/session/combat-experience/{CombatExperience.tsx,types.ts,story.ts,presentation.ts}`;
`package.json`, `package-lock.json`.
**New files:** `src/api/useSetDiscoverySharing.ts` and test;
`src/concepts/world-building/DiscoveryCheckPolicy.tsx` and test.

**Interfaces:** C1 authored fields round-trip as content without client game
validation replacing publish validation. C4 optional wire sharing value must be
present for an editable toggle; an older/invalid producer does not masquerade as
private. Toggle sends intent only. No client proximity loop or dice request.

**Behavior:** show automatic check approaches (skill/DC), total attempts default
1, lifetime, and reset-distance control default 3 when repeats are enabled.
Explain the dormant passive-score field separately; do not silently reinterpret
or delete carried author content. Inline author guidance: mandatory progress
must remain possible with every discovery roll failed.
Remove Search hook/button/pending notice/props/tests and region lookup if it has
no remaining consumer; do not remove shared map helpers merely because Search
used them. Add an accessible per-character sharing toggle fed by the snapshot
and successful API result, with disabled-in-flight/error handling.
Render failed checks as `<character>: Failed <skill> check` without target names;
success may use the same result model, while reveal geometry still comes from
recipient-scoped knowledge events. No blocking manual dice-throw gesture.

**Tests:** `DiscoveryCheckPolicy.test.tsx`: default1, opt-in max2/reset3, explicit
reset4, per-check isolation, export/import round-trip; published content errors
remain visible. `SessionEncounterView.test.tsx`: no Search, private/shared
snapshot, successful/failed toggle, reconnect restoring the explicit preference.
`story.test.ts` / `presentation.test.ts`: failure uses actual skill and roller,
no target/DC fabrication, richer calculation carried rather than recomputed;
reveal and result are separate events. Update concealment persistence/carry tests
so parsing or exporting cannot silently drop retry settings.

- [ ] Build consumer fixtures and controls before wiring live transport.
- [ ] Add contract consumption and remove all old Search UI/client code.
- [ ] Run focused tests and typecheck while developing.
- [ ] Run `npm run ci-check` once at the PR boundary and perform T6 browser walk.

**Verification:** from web: `npm run test:run -- <changed-test-paths>`;
`npm run typecheck`; final `npm run ci-check`. Install the generated proto tag
with npm and commit both package manifest and lockfile.
**Evidence:** web PR, authoring round-trip tests and rendered local-game proof.

## T6 — Joined acceptance, publication and cleanup

**Delivers:** all rulings together, especially R4 and no Search backdoor.
**Owner:** cross-repo integration; fixture in API, tests with its real session
handler, browser proof through the root checkout's workspace launcher.
**Prerequisites:** T1–T5 integrated; dedicated local stack, not another lane's dev.
**Files:** new API `content/reference-discovery-checks.yaml` and
`internal/integration/session/automatic_discovery_acceptance_test.go`; reuse
`acceptance_test.go`'s `newAcceptanceHarnessWithDice`. Add fixture registry/listing
through the existing content loader if needed, not a second launch path.
Local-only browser automation follows `tools/browser/_job_<topic>.mjs` naming;
no new game-dev feature/commit is needed merely to run the proof.

**Fixture/interface:** A and B loaded, C joins later; optional concealed props/
doors with Perception, Investigation, Religion and Survival approaches; one
single-try secret and one max2/X3 secret; an unobstructed required objective/exit.
A blocker on a checked footprint proves adjacency does not require walking onto
it. Use real character data; controlled dice select deterministic fail/success.
The authored definition is cloned on launch and never hot-edited.

**Acceptance (`TestAutomaticDiscoveryAcceptance` suite + real browser walk):**
1. All checks forced to fail: expected log lines, no hidden IDs, objective/exit
   still reachable. No Search affordance or callable gameplay backdoor.
2. A private failure/success reaches only A; B's own attempt remains independent.
3. With A sharing, loaded B receives both the failure result and success intel;
   late C receives neither old event nor old discovery by a backfill operation.
   C can independently observe an opened door under existing sight rules.
4. Toggle off: B keeps prior knowledge but gets no subsequent private attempt.
5. Default one try survives departure/re-entry and actual repository reload.
   Repeat max2/X3 follows the exact 2→1 versus 3→1 matrix; reload on the outward
   leg preserves re-armed state; a third cycle grants nothing.
6. New run: retained memory preserves its used count; an explicitly per-run
   policy starts fresh. Characters/worlds/check identities remain isolated.
7. Multiple cells in one Move and a turn-clock Move cannot skip the near cell,
   reroll on every sight refresh, spend a Search action, or overwrite movement
   payment/condition changes. A read/reconnect cannot roll.
8. Launch a second run after editing source policy: each run retains the
   definition it loaded. Do not mutate a live run to perform this proof.
9. Stream and GetStory agree on each original recipient; no current roster or
   toggle retroactively expands historical audiences. Save errors publish no
   purported committed discovery. Do not claim new disconnect recovery.

**Verification:** API `go test -race ./internal/integration/session -run
'TestAutomaticDiscoveryAcceptance'`; run the real listed fixture with two browser
identities and a late third. Start/stop via root-checkout `scripts/dev-env.sh`
after reading its local-stack guide; screenshot via the root browser tool and
inspect the rendered image. Record exact builds, fixture, steps and screenshots.
Do not equate fake-roll unit cases or seed setup with browser proof.

- [ ] Execute joined API acceptance and browser walk; record failures/corrections.
- [ ] Run each owning repo's final gate once at its PR boundary.
- [ ] Complete the required independent feature review round and publish verdicts;
  delegation is not pre-authorized by this plan.
- [ ] Obtain operator merge authorization, publish providers inside-out, replace
  development pins, and rerun joined smoke checks on actual released versions.
- [ ] Update owning Search/concealment/authoring docs and close #523 only when
  its full scope is proved, not when the first provider merges.

**Evidence:** linked implementing PRs and actual tags, API results, rendered
all-fail/private/shared/retry walkthrough, review verdict, pending gaps named.

## Visible plan checks

### Requirement coverage

| Requirement / scenario | Tasks | Concrete proof |
| --- | --- | --- |
| R1/R9 automatic skill rolls; no active-action side effects | T1,T2,T5,T6 | OneHex; four skill cases; no unlock/force; no manual dice gesture |
| R2/R8/R13 one try, optional repeats, retained/run lifetime | T0,T1,T2,T4,T6 | Guarded same-session operations; DefaultAttempt; ThreeHex; RunAndCharacterLifetimes; real Redis reload/new run |
| R3/R16 failure log follows sharing, Search removed | T1–T6 | Captured audiences, typed conversion, story prose; old RPC/UI removed |
| R4 every failed roll still permits completion | T5,T6 | Builder guidance and deterministic all-fail objective/exit walk |
| R5/R7 sharing on by default, off durable, no unlearning | T1,T2,T4,T5,T6 | Profile/snapshot/toggle tests; browser off then reload |
| R6/R10 share discovery, loaded recipients only | T1,T2,T4,T6 | Existing reveal writer; captured recipient set; late C gets no history |
| R11 proximity independent of blockers | T1,T6 | BlockedHex still rolls; illegal movement still refuses |
| R12 policy belongs to each check | T1,T5,T6 | Two checks with different policy; YAML/compiler/persistence round-trip |
| R15 default X3, override and return trigger | T1,T2,T5,T6 | 2→1 no roll; 3→1 roll; override4; save between legs; exhausted budget |
| R17 frozen authored definition | T1,T2,T6 | LoadedPolicy; two independently launched definitions |
| R14 explicitly deferred recovery | no new task | No backfill/presence/reconnect service is introduced |

### Provider/consumer seams

| Provider | Consumer | Produced vs consumed contract | Availability | Joined proof |
| --- | --- | --- | --- | --- |
| T0 API locker | T0 SDK entry points | LockSession named Input/Output, guard through read/act/save/delivery | Implemented, unmerged #1930/#1072 | Real overlapping Move/read over miniredis; shared-coordinator managers |
| Existing resolution.MakeCheck | T2 checkSeam | Beaten/Applied/Total/Calculation and DirtyCharacter | Present; reads real skills and conditions | Multiple automatic checks preserve sheet changes and movement |
| T5 authored YAML | T1 dungeonspec | C1 `attempts` plus unchanged approach list | New producer/parser together | Export→compile→load then distance-driven attempt |
| T1 encounter | T2 session | C2 memory, C3 audience, discovery_checked | New provider APIs specified above | SDK Move/Join, persistence, live+Story |
| T2 ExplorationRepository | T4 Redis adapter | C4 SDK data, exact Get/Save/missing contract | New required capability | Trusted-world isolation and cross-run reload |
| T2 SDK event/toggle/read | T3 proto + T4 converter | C4 exact fields; no concealed target/DC | New generated contract | Handler/event equality; no lost false sharing value |
| T3/T4 response and events | T5 UI | Typed result, optional explicit sharing bool, existing reveal patch | Needs generated tag and API | Native toggle/log/reveal walk, no client rolls |
| Existing recipient record/stream | T1–T5 | Audiences fixed at append, recipient-local dense Seq | Present | Excluded/late character receives no history or seq hole |

### Findings and disposition

1. **Search staging is not reusable by merely calling it from Move.** Covered by
   T2's common write-scope/Join staging and coherent actor record test; no rule
   is moved into session and no hidden-object-dependent sheet lookup is accepted.
2. **Literal passive Notice is a different contract.** Preserve it; C1/T5 name
   automatic Checks separately. No silent field repurposing.
3. **The memory provider wave is merged.** Baseline advanced; T1 uses its reveal
   writer and T5 uses `useSessionKnowledge`, not the old live atlas.
4. **Cross-run attempts cannot live only in EncounterData.** C2/C4/T2/T4 add the
   paid-for durable profile seam; an expiring session blob is insufficient.
5. **No live-edit machinery is needed.** The existing StartSession copy/load
   boundary already supports R17. Test that boundary rather than invent hot edits.
6. **Settings, counters and authored policy are not player intel payloads.** Keep
   private counter/check IDs out of public failure events and player snapshots.
7. **Session-scoped serialization is proved by T0, with explicit limits.**
   Operations sharing the host coordinator no longer race a session's
   load-act-save. This does not make SDK writes a multi-repository transaction,
   coordinate unseated character writers or serialize distinct sessions touching
   the same durable exploration profile. T2/T4 must preserve the latter's
   consistency as part of their profile contract, not infer it from a session
   lock. No crash-atomic or uncoordinated-replica guarantee is claimed.
8. **Plan readiness:** gameplay decisions are settled and the immediate
   session-write prerequisite is implemented. T1 can proceed on its named
   contract. T2/T4 must complete their durable-profile concurrency contract before
   that integration is called execution-ready; this is a bounded technical seam,
   not a new gameplay vote. No whole-wave completion/release claim is made.

### Checks actually run during planning

At toolkit `3955eaf7`, each from its owning module with committed dependencies:

```sh
# rulebooks/dnd5e/encounter
GOWORK=off go test -mod=readonly -race -count=1 . \
  -run '^(TestRoomKnowledgeSuite|TestConcealmentSuite|TestConcealSuite)$'
# rulebooks/dnd5e/session
GOWORK=off go test -mod=readonly -race -count=1 . -run '^TestKnowledgeSuite$'
```

Both PASS. They establish the current knowledge/concealment baseline, not the
new automatic-check feature. No API/web feature tests, native walkthrough,
full-repository gate, review or release has been run for this wave.
