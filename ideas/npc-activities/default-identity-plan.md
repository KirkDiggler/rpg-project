# NPC default activity — identity and transport task contract

**Goal:** carry the author's chosen default activity to the observer's renderer
without making animation a game rule or reading fresh hidden/source data.
**Authority:** story #711 and [design R1–R8](design.md). R9's pacing foundation is
merged; this is not another presentation queue.
**Finish line:** provider/persistence/observer tests, published contract/provider
PRs and actual SDK adoption, then the studio-to-yard default-activity walk. A
wire field alone is not the player-facing feature.

## Contract

- Authored v5 monster declaration: optional `defaultActivityRef`.
- Compiled/internal/persisted value: `DefaultActivityRef string`; persisted JSON
  `default_activity_ref,omitempty` where existing member/reserve identity lives.
- Captured known/lost sight testimony: the same opaque reference. Missing legacy
  value stays empty; unknown testimony carries no activity facts. A memory must
  not acquire a newer live member's default on reload or public projection.
- Public SDK identity: `PublicMember.DefaultActivityRef string`.
- Wire: additive `PublicMemberInfo.default_activity_ref = 10` string.
- Example value: `dnd5e:activities:thoughtful`. Toolkit validates ref syntax only;
  it does not know clip names, assets, duration, speed or activity support.
- Empty/absent public value means no explicit captured override, so the consumer
  uses the appearance provider's declared default. An explicit unsupported ref
  remains a named consumer diagnostic, not another activity or model.
- This is a default presentation choice, not the current animation phase, a
  behavior table action, a stat-block choice, or a new verb/RPC.

## Inspected baseline / integration dependency

Toolkit main `ed751d26` carries encounter source v5 and session's adoption of it.
API dev still pins encounter v0.148.0/session v0.147.0; the v5 consumer migration
is API #1155 and web #1351 under project#725. Keep this field on current toolkit
main; do not backport to retired v4 or silently absorb that migration. The newly
merged Pacing Yard must be included in its conversion (notified on API #1155).
Protos main `01b8235` has public identity tags 1–9 occupied; tag 10 is available.

## Task A — additive wire contract

**Owner:** rpg-api-protos, worktree `.worktrees/npc-default-activity`, branch
`feat/npc-default-activity`.
**Files:** `dnd5e/api/session/v1alpha1/types.proto`,
`docs/architecture/components/session-service.md`.
**Produces:** `PublicMemberInfo.default_activity_ref = 10`, string, with the
capture/absence semantics above. No enum, new RPC, generated-code edits or
bespoke protoc serialization tests.

- [x] Add the field and its owning contract documentation.
- [x] `make format`, `make test` (Buf lint/format/new-file options/breaking).
- [x] Commit/push and publish the contract PR with exact semantics and checks.
- [x] Operator merged #407; CI published v0.1.246 at generated 8f2bb8a26968.
  API/web consume those actual bindings. This is the protos exception, not a
  reason to merge ordinary Go providers early.

## Task B — encounter source, persistence and captured identity

**Owner:** toolkit `rulebooks/dnd5e/encounter` module only; worktree
`.worktrees/npc-activity-identity`, branch `feat/encounter-default-activity`.
**Prerequisites:** current main/v5 source grammar; independent of generated SDK.
**Files:** `dungeonspec/{single_room.go,single_room_decode.go,single_room_compile.go,compile.go}`;
`field.go`, `encounter.go`, `data.go`, `testimony.go`, `doc.go`;
new `dungeonspec/default_activity_test.go`, `default_activity_internal_test.go`;
exact-key diagnostic expectations in `single_room_site_test.go` and
`monster_facing_test.go`.

**Behavior:** validate optional source spelling and reject explicit empty/null/
non-string/malformed refs. Carry through initial members, joins/reserves and
JSON round trips. Capture on sight refresh; never expose a fresh live ref from
an unscoped member read or repair old testimony. Preserve rule/faction/position
and the appearance choice. Existing `core.ParseString` is the syntax authority.

**Tests:**
- `TestDefaultActivityRefIsCarriedWithoutChangingRulesAppearanceOrFaction`:
  source → compiled placement → YAML round trip; only the new field differs.
- `TestMalformedExplicitDefaultActivityRefNamesItsSourcePath`: null, integer,
  empty string and malformed ref all refuse at `defaultActivityRef`.
- `TestCaptureMemoryReloadAndReobservation`: one observer sees Thoughtful,
  loses sight; live data changes to Relaxed; JSON save/load keeps old memory;
  reobservation changes only then.
- `TestReserveJoinAndLoadValidateDefaultActivity`: reserve round trip, valid
  join, invalid join refuses before mutation, invalid persisted value refuses.
- `TestStrictDefaultActivityPayload`: present ref survives private/public sight
  projection; missing legacy stays empty; untagged, explicit null/empty/wrong-type,
  malformed and unknown-state-with-ref payloads refuse.

- [x] Add the tests and observe the missing-field behavior fail.
- [x] Implement along the existing appearance path, not a second identity store.
- [x] Run focused encounter/dungeonspec checks with race checking; verify the
  source/capture omission mutants are caught at the changed observer seam.
- [x] Run the module's required gate, push the first working checkpoint, and
  immediately open its draft before consumer work. One nearest-go.mod module.

## Task C — session launch and observer projection

**Owner:** toolkit `rulebooks/dnd5e/session` only, separate worktree
`.worktrees/npc-activity-session`, branch `feat/session-default-activity`.
**Prerequisite:** Task B pushed commit `65afb038`, consumed with `go get` and a
real pseudo-version. No replace/go.work and no other module edits.
**Files:** `launch.go`, `read.go`, `types.go`, `doc.go`, `roster_test.go`, new
`default_activity_test.go`, and this module's `go.mod`/`go.sum`.
Provider adoption also requires two test fixtures to use `UntilStep` lists
(with nil remaining nil), and the existing `SeenMember`/`RememberedMember`
`Allied` facts to be carried through `turndriver.go` and its positive projection
test. Those are shape adaptations, not new alliance or disposition rules.

**Interfaces:** `MonsterPlacement.DefaultActivityRef` → `JoinInput.DefaultActivityRef`
at launch, including reserves. `SightTestimony.DefaultActivityRef` →
`PublicMember.DefaultActivityRef string` (`default_activity_ref,omitempty`) on
the public roster. The roster reads captured testimony, not live member data.
Keep the default empty for legacy observations and players with no such authored
choice. Preserve rule ref, faction, appearance and existing identity fields.

**Tests:** `TestDefaultActivityLaunchReachesActiveAndReservedMembers` asserts all
expected placements survive launch/save with the exact ref and unchanged rules;
`TestDefaultActivityMemoryDiffersFromAnotherObserversCurrentSight` changes live
data and rechecks only where visible, JSON round-trips, then asks both observers;
`TestDefaultActivityNeverSeenAndLegacyAbsenceAreNotEnriched` asserts never-seen
NPC absence, self presence and explicit empty legacy capture despite a newer
live default. The public-field allow-list adds only the new presentation ref.

- [x] Pin the pushed encounter provider and write the named failing tests.
- [x] Implement launch and captured roster projection, with no new rule or read.
- [x] Run focused tests with race checking; demonstrate that dropping launch
  carriage or enriching capture from live data fails the corresponding test.
- [x] Run this module's tests/lint, push, and open its own draft PR. Consumer
  merge later requires the encounter release tag, not this development pin.

## Task D — API adoption

**Owner:** rpg-api#1156, `.worktrees/npc-activities`, feat/npc-default-activity.
**Prerequisites available:** v5 adoption #1155 merged; proto#407 merged and CI
published v0.1.246 (generated commit 8f2bb8a26968). Consume session#2170 pushed
7e46ef97 through its real pseudo-version, never a replace/go.work.
**Files:** go.mod/go.sum; Dockerfile (published SDK requires Go 1.26);
internal/handlers/dnd5e/session/v1alpha1/get_roster.go and new
`default_activity_test.go`; owning API architecture documentation.
**Interface:** `sdk.PublicMember.DefaultActivityRef` copied unchanged into
`sessionpb.PublicMemberInfo.DefaultActivityRef` at `publicMemberToProto`.
Both GetRoster and GetKnowledge already use this single conversion point.
No new lookup, asset validation, animation decision, RPC or permission category.
**Tests:** suite `DefaultActivityHandlerSuite` exercises both actual handlers
with provider output containing Thoughtful, a syntactically valid unsupported
reference and absent legacy values. Expected wire values are identical; identity
and rules fields remain intact. Missing converter assignment must fail both
nonempty cases. Mock only the SDK manager/ownership ports, not the converter.
**Gates:** focused handler tests, full `make test` and `make ci-check`; update
any required dependency-shape fixtures explicitly, never weaken assertions.
Published as API#1157 at 4df2315 with both local gates and CI green. The pin
required UntilStep adaptations in factions/hearing tests and synchronization
with the pinned toolkit castle (existing captain alliance content); no rule
was invented. Real toolkit release tags and integration walk remain merge
prerequisites.

## Task E — web authored-value carriage

**Owner:** web `.worktrees/npc-activities`, feat/npc-default-activity, on merged
v5 codec #1351. Consume published proto v0.1.246 with its actual lockfile hash.
**Files:** `src/concepts/world-building/roomDraft.ts`, new
`roomDefaultActivity.test.ts`; package.json/package-lock.json.
**Interface:** optional `RoomMonsterPlacement.defaultActivityRef` with a pure
`setRoomMonsterDefaultActivity` edit. Strict draft decoding carries the value
through the existing JSON/v5 YAML codec; clearing deletes the key. Only reference
syntax is checked here; unknown valid refs survive, preserving future content.
Appearance, rules, bindings, location and facing stay independent.
**Tests:** exact declaration equality after JSON/YAML reload; known and unknown
refs; explicit null/non-string/empty/malformed refusal; immutable no-op and clear.
Checkpoint e1893dae: missing setter failed first; 138 focused codec/appearance
tests and TypeScript pass. The full web gate remains at the PR boundary.
Provider-backed
choices, Arrange staging/undo and renderer use belong to the following connected
unit; this task does not introduce a hard-coded Thoughtful-only control.

## Coverage / seam check

| Requirement | Task | Proof |
|---|---|---|
| Opaque authored choice, no animation rules | A/B | string ref only; syntax validation and owning docs |
| Source and reserve persistence | B | codec/join/JSON round-trip tests |
| No fresh-data enrichment of memory | B then C | changed live ref remains absent from remembered projection |
| Stable additive transport | A then C | Buf checks + generated SDK + converter assertions |
| Actual Thoughtful in studio/play | Provider + C + web adapter | still pending; not claimed by these providers |

| Provider | Consumer | Exact boundary | Availability |
|---|---|---|---|
| v5 monster declaration | compiled `MonsterPlacement` | `DefaultActivityRef` string | Task B |
| Encounter sight capture | Session roster | captured `SightTestimony.DefaultActivityRef` | pushed Task B commit |
| Session public identity | API converter | `PublicMember.DefaultActivityRef` | Task C |
| Proto contract | API/web | `default_activity_ref = 10` / generated camel-case fields | CI-published SDK after authorized merge |

Plan check: the appearance pattern already supplies the ownership and lifecycle
route. This addition does not change its source of truth. Site-format migration
and runtime animation delivery are separate dependencies, not hidden tasks in
this carrier. No consumer may claim the feature works before the generated
field, provider metadata and actual body transition adapter are joined.
