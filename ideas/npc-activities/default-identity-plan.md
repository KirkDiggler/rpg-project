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

- [ ] Add the field and its owning contract documentation.
- [ ] `make format`, `make test` (Buf lint/format/new-file options/breaking).
- [ ] Commit/push and publish the contract PR with exact semantics and checks.
- [ ] Obtain operator merge authorization; wait for CI-generated SDK/tag before
  API/web consume the field. This is the documented protos exception, not a
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

- [ ] Add the tests and observe the missing-field behavior fail.
- [ ] Implement along the existing appearance path, not a second identity store.
- [ ] Run focused encounter/dungeonspec checks with race checking; verify the
  source/persistence omission mutant is caught at the changed observer seam.
- [ ] Run the module's required gate, push the first working checkpoint, and
  immediately open its draft before consumer work. One nearest-go.mod module.

## Task C — session projection and consumer adoption (next contract)

Session gets its own module PR and a real pushed encounter commit pin. It passes
compiled defaults at launch (including reserves) and projects only decoded
observer testimony to `PublicMember.DefaultActivityRef`, with a two-observer/
reload test and the public-boundary allow-list updated. API then copies that
field through the existing roster/knowledge converter using the generated SDK.
No new permission-policy RPC classification is needed because no RPC is added.
The concrete session/API task contract is completed from their current sources
before edits; consumer v5 adoption and SDK availability remain explicit.

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
