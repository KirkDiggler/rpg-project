# Structural layout patch implementation plan

## Scope and authority

Implement the operator-approved snapshot + introductions + typed component
replacement model in [the contract](layout-patch-contract.md). Correct explicit
concealment membership without losing object support for discovery. Keep existing
movement/sight ownership; no general spatial extension, room assembly, floor
surface system or generic patch language.

Inspected baselines: encounter branch ef64dc14; session branch c9a28188;
protos main a8d735e; local API/web structural worktrees. Toolkit main82bd0736 now
contains the parallel effect work and merged CloseDoor. Reconcile upstream before
provider delivery; do not downgrade its dependencies or overwrite the dirty
consumer worktrees. Existing PR1935/1947 remain the module delivery branches.

## Execution checkpoint

T1 is implemented in proto PR380, independently reviewed with findings closed and
CI green; merge/publication is the remaining binding prerequisite. T2's provider
checkpoint055995bb is pushed on toolkit1935. T3's adapter12845c58 is pushed on1947,
after reconciling current main and released CloseDoor. Full module tests/race,
vet/lint and normal hooks pass on both. These are provider checkpoints, not full
T6 acceptance or merge-readiness of the broader toolkit PRs. T4/T5 await actual
published proto bindings; no substitute/local generation is used.

## Sequence

T1 wire and T2 encounter do not depend on generated bindings. T3 consumes T2's
pushed checkpoint. T4 requires T3's pushed checkpoint and actual CI-generated
bindings from T1. T5 consumes those bindings. T6 verifies the joined path.
Publishing order: independently review/merge the proto contract to generate SDKs;
verify integration on pushed Go checkpoints; merge encounter then session, adopting
actual release tags outward through API/web before their merges. No automatic
merge authorization. One writer per worktree; no dependency overrides.

## T1 — Additive wire contract

**Owner/delivers:** rpg-api-protos; the exact replacement type/field in the contract.
**Prerequisite:** existing structural types and both reveal messages are published.
**Files:** dnd5e/api/session/v1alpha1/types.proto, events.proto;
docs/architecture/components/session-service.md. New branch/worktree from main.
**Interface:** StructuralWallOpeningsReplacement {wall_id=1, openings=2}; repeated
structural_wall_openings_replacements=12 on BOTH existing reveal messages.
Snapshot fields and existing full-row fields keep their numbers/meaning.
**Behavior/proof:** comments specify replacement/default-empty, atomic application,
known-wall baseline, recipient projection and sequence/recovery. Buf checks retain
wire compatibility. No local generator or bespoke protobuf-mechanics tests.
**Actions:** author, make format/test, diff check, publish a small PR for independent
review. CI owns generation and consumer binding publication after authorized merge.
**Verify:** from proto worktree, `make format && make test`; normal commit hook.
**Evidence:** source SHA, contract checks, PR review and actual generated SHA/tag.

## T2 — Explicit membership and recipient patch producer

**Owner/delivers:** toolkit encounter, PR1935; no new rules in session/API/web.
**Prerequisite:** contract; existing TraceFootprint, placedCells, hiddenDoorTo and
member projection. Merge current main into this clean feature worktree first.
**Files:** concealment.go (hiddenCellsOf/memberPropCells), conceal.go
(concealmentOnCell), discovery.go (discoveryDistance), search.go
(concealmentTouchesRegion), projection.go, roomknowledge.go, revealbeat.go,
door.go comments; structural_reveal.go/tests and explicit_concealment_test.go /
footprintdoors_test.go. Audit step.go and clocks.go for anonymous hidden-door
refusals; use the existing footprint traces/contributors, not a new spatial API.
**Interfaces:** floor membership comes only from c.cells. Footprint-door support
remains available through doorFootprintCells for discovery/search, without being
concealed floor or an occupancy-based discovery trigger. JSON reveal extension
uses exactly structural_wall_openings_replacements with wall_id/openings.
**Behavior:** known wall cut change emits replacement, new wall full introduction,
new door full record, unchanged rows absent. Compute from recipient before/after
AtlasFor; store event payload, do not enrich replay. Explicit cells alone drive
floor reveal payloads. Existing room knowledge is not replaced.
**Tests (new or updated):**
- concealed door with Cells empty: floor unchanged; discovery distance and region
  eligibility remain; standing on support cell alone does not disclose it;
- selected wall/door overlap never adds unrelated cells/props; explicit hidden
  cells still withhold/reveal; independent unseen rooms remain withheld;
- moving beside the door does not meet a fabricated whole-cell mask; any refusal
  attributable to an unfound door does not disclose its ID/state. Keep entity mask
  separate from floor membership, using existing authored geometry;
- new wall emits full row; known wall gaining O emits one replacement and no full
  row; two reveals produce cumulative permitted lists; no future-secret replay;
- snapshot-after equals snapshot-before plus this recipient's event payload.
**Actions:** pin failing behavior before fixes, update tests that encode inferred
membership, implement, run module gates and publish checkpoint on existing PR.
**Verify:** encounter cwd, GOTOOLCHAIN=go1.24.1 `go test ./...`,
`go test -race ./...`, `go vet ./...`, `golangci-lint run`; dungeonspec fixtures
included. If masking needs an unprovided capability, report that concrete gap,
not a silently weakened privacy assertion or revived observation rewrite.
**Evidence:** regression outputs, provider SHA/pseudo-version, updated PR scope.

## T3 — Session projection and replay adapter

**Owner/delivers:** toolkit session, PR1947; carry T2 answers unchanged.
**Prerequisites:** reconcile main/merged CloseDoor; adopt T2 via go get at its pushed
SHA. No unreleased source copied into another module.
**Files:** structural.go, room_revealed.go, types.go, events.go, go.mod/go.sum;
structural_session_test.go, events_internal_test.go, convert_internal_test.go.
**Interfaces:** seam-owned StructuralWallOpeningsReplacement with WallID string,
Openings []AtlasStructuralOpening; both reveal bodies carry the matching slice.
Private payload decoder recognizes the same JSON keys; snapshot DTOs unchanged.
**Behavior:** absent patch slice is legacy no-op; present patch with default empty
openings means clear. Refuse malformed identities, duplicate patch IDs and
same-event full-row/patch collisions as a whole body, not a partial decode.
Deep-copy slices; original-recipient replay never looks up current world state.
**Tests:** real Manager discovery/Knowledge/Story/reload snapshot-patch parity;
legacy full rows remain decodable; malformed/collision cases; no-alias assertions;
all new fields cross the seam and no encounter runtime type/proto type leaks.
**Verify:** session cwd, Go1.24.1 full tests/race/vet/lint and boundary tests.
**Evidence:** actual dependency pin, passing commands, pushed SDK checkpoint.

## T4 — API mapping and real wire proof

**Owner/delivers:** rpg-api, existing structural worktree; pure translation.
**Prerequisites:** preserve current dirty work, reconcile merged CloseDoor/current
dev without downgrades, adopt actual generated T1 bindings and pushed T3 SDK.
**Files:** internal/handlers/dnd5e/session/v1alpha1/convert_structural.go,
convert.go, structural_convert_test.go, get_knowledge_test.go; existing
internal/handlers/dnd5e/authoring/v1alpha1/structural_layout_wire_test.go;
go.mod/go.sum. A new focused integration test may live beside the real
internal/integration/session/concealed_reveal_acceptance_test.go harness.
**Interface/behavior:** map wall_id and every opening field into the new generated
record on both reveal cases. Preserve absent vs present-empty records; no local
visibility filtering, geometry calculation or new RPC. Snapshots stay complete.
**Tests:** table-test both event conversions including an empty replacement; real
compiled fixture through SDK → handler/wire conversion shows known-wall patch and
new door rather than a full changed-wall row. Snapshot agrees afterward.
**Verify:** focused session/authoring/integration tests; Go version declared by
current API, `make ci-check`, normal hooks. Use an isolated compatible linter/cache.
**Evidence:** generated/provided pins, real-seam proof, published consumer draft.

## T5 — Atomic web reducer and hydration recovery

**Owner/delivers:** web existing structural worktree; consume authoritative deltas.
**Prerequisites:** actual T1 TypeScript bindings; preserve existing dirty work and
reconcile current dev without clobbering shared authoring/CloseDoor changes.
**Files:** src/components/session/applyReveal.ts and applyReveal.test.ts;
src/api/useSessionKnowledge.ts/test.ts; StructuralLayoutEnvironment.test.tsx;
SessionEncounterView.test.tsx; package.json/lock. Reuse existing scene validation.
**Interfaces:** apply the new slice in both reveal cases. Full wall/door rows retain
upsert semantics for introductions and old events. No partial wall creation.
**Behavior:** validate the entire structural event before committing any of it;
replace known wall.openings, preserve its other fields; empty clears. Missing
baseline requests a coalesced Knowledge resnapshot through the existing queue;
error is visible, no unbounded retry. Preserve events during hydration, fence stale
scope responses, and advance the accepted-event sequence so an older replacement
cannot revert a newer list. Transport gap ordering stays with the existing stream.
**Tests:** introduction/replace/clear/absent; unknown wall with accompanying door is
atomic; duplicate/collision refusal; snapshot N with queued N+1; duplicate and
late N+1 after N+2; missing-base recovery success/failure; member switch fences old
responses; door state remains separate. Compare patched result with full snapshot.
**Verify:** targeted Vitest suites + typecheck, then supported Node22 `npm run ci-check`
at the PR boundary. No blanket full-YAML fetch or geometry-derived concealment.
**Evidence:** deterministic reducer/hook tests, consumer draft, full gate output.

## T6 — Bounded authored acceptance

**Owner/delivers:** parent integration across existing stack; no new geometry API.
**Prerequisites:** T2–T5 runnable via pushed module pins and generated bindings.
**Fixture:** one authored wall with an opening/attached door, explicit door-only
concealment, ordinary visible floor, a separately selected hidden floor cell, and
an independent prop. Use authored blocker-box placement; verify the intended
approach-side arrangement rather than assume all placements work.
**Actions/assertions:** before reveal floor under/beside the door remains present,
only selected floor is withheld, wall picture has no disclosed opening. Discovery
sends known-wall opening replacement + door introduction; patch and refetch agree.
Open/close, movement blocking and reload use the existing verbs. Check the intended
approaches and document actual supported placement limits; don't substitute a
successful unrelated-side fixture for a required approach. Exercise the regular
builder publish route, not only fixture APIs. Preserve existing sessions/content.
**Verify:** actual browser, requests/events and screenshots; no source document
fetch during play; no page errors; released-pin recheck before consumer merges.
**Evidence:** reproducible authored fixture, tested placement guidance, event and
snapshot receipts, remaining migration/UX issues explicitly identified.

## Checked requirement coverage

| Requirement | Tasks | Proof |
|---|---|---|
| Full snapshot + introductions + component replacements | T1–T5 | new wall vs known cut event; patched snapshot equality |
| Empty/default and malformed semantics | T1,T3–T5 | clear vs no-op; atomic rejection and recovery |
| Recipient privacy and replay | T2–T5 | hidden parent not named; stored first event unchanged after later discovery |
| Existing sequencing/hydration | T5 | N/N+1/N+2, duplicate, stale scope, resnapshot tests |
| Explicit concealment only | T2,T6 | floor unchanged for door-only selection; discovery support retained |
| Existing geometry / bounded placement | T2,T6 | independent contributors, no hex-sized phantom mask, authored approach proof |
| No consumer-owned rules | T3,T4,T5 | pure DTO mapping/reducer; provider/runtime assertions remain in toolkit |

## Checked provider/consumer seams

| Provider | Consumer | Contract / availability | Joined proof |
|---|---|---|---|
| T1 proto | T4 API, T5 web | additive shared type + reveal tag12; CI-generated after merge | real generated field mappings and reducer tests |
| T2 encounter | T3 session | recipient JSON key/row semantics above; pushed SHA before go get | real Manager Story/Knowledge/reload |
| T3 session | T4 API | seam-owned replacement DTO; pushed SHA before go get | real SDK→handler/wire test |
| T4 API | T5 web | existing snapshot/reveal paths and recipient seq | hydration/replay plus T6 browser |

Inspection findings: tag12 is free on both reveal messages; current producer emits
changed full rows and must change, not just the client merge. Current Knowledge
hook checks state.seq but does not advance it after applying an event: T5 must fix
that before accepting replacement patches. hiddenCellsOf and concealmentOnCell
both derive hidden membership from footprint doors: T2 covers both, while search/
discovery retain their separate support reads. Legacy full-row decoding stays.

These checks authorize execution of these bounded tasks under the approved
architecture, not a generic geometry rewrite. Implementation results belong in
PRs/receipts; the tests above are planned assertions, not claims of passes.
