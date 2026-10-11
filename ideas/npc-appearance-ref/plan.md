# NPC appearance implementation plan

## Goal, authority and baseline

Implement the operator-approved [contract](design.md) and [walkthrough](README.md):
Studio chooses a model independently of stats/faction, saved source carries it,
perception captures it, and Web renders the observer's projected choice. No new
stats or faction wire shape or source-document lookup in play. The base appearance
slice keeps art unchanged; the operator-approved palette extension below adds
atlas-only complete looks while retaining original default assets.
The stat-block initiative is project#555; this wave does not own its template
assembly or panel. Consumer issue: Web#1253.

Inspected baselines: toolkit `47404a8c`, protos `68ced5f`, project `60448a2`;
Web local `4c435d12` atop Studio `a1cc52a7`, with asset provider `9a2c348`.
The actual roster read is `session/read.go:rosterFrom`; sight captures in
`encounter.go:rebuildPercepts`; canonical payloads are `testimony.go`.
Current code keeps known-location memories on sight loss (no new unknown-location
rewrite is added). Existing unknown payloads remain empty. Source module scopes
are separate release units, so encounter and session have separate PRs.

## Task 1 — additive wire contract

Owner: protos. Extend `dnd5e/api/session/v1alpha1/types.proto` with
`PublicMemberInfo.appearance_ref = 9`, a string whose empty value keeps legacy
body resolution. Clarify that monster_ref remains rules identity. Document the
new field in `docs/architecture/components/session-service.md`.
No generated SDK edits and no stats/faction fields. Run `make format && make test`.
Open the contract PR; generated SDK publication after its operator merge is the
real consumer-build prerequisite.

## Task 2 — encounter source/persistence/observation

Owner: toolkit encounter module only. Files: `dungeonspec/single_room.go`,
`single_room_decode.go`, `single_room_compile.go`, `compile.go`;
`field.go`, `data.go`, `encounter.go`,
`testimony.go`, and owning godoc/tests.

Add optional authored `appearanceRef` to room monster declarations and compiled
MonsterPlacement. Legacy v2 PlaceSpec grammar stays unchanged (it refuses the new
key instead of silently dropping it); its compiled appearance remains empty.
Validate with the existing core ref parser; no catalog lookup.
Refuse malformed/non-string/empty explicit source values. Carry through the room
and shared compiler path without changing rules ref, faction, stats or placement.
Add opaque AppearanceRef to MemberInput/JoinInput, internal member record and
MemberData including reserve persistence; do not expose it as fresh public truth
on the unscoped Member read. Capture it on known SightTestimony at sight refresh.
Legacy/unknown testimony has no new override; unknown payloads still refuse facts.

Tests: source decode/compile round trip (four-part Kingdom ref), malformed source
ref path, unchanged rules/faction, member and reserve round trip, sight snapshot
survives loss/reload and does not consult changed live appearance, strict legacy/
malformed payload behavior. Run focused dungeonspec/encounter suites with race
checking. Commit/push this module and open its draft before consumer work.

## Task 3 — session launch and public projection

Owner: toolkit session module. Consume Task 2's pushed commit via real Go
pseudo-version, never replace/go.work. Files: `launch.go`, `read.go`, `types.go`,
`doc.go`, `roster_test.go`, launch tests and module pins.

Launch passes compiled appearance into existing JoinInput (including reserve).
PublicMember gains AppearanceRef. rosterFrom reads it exclusively from decoded
observer sight testimony; never-seen NPCs remain absent, legacy missing appearance
remains empty, remembered appearance is never filled from live member data.
Stats/sheet and faction projection stay unchanged.
Tests: authored placement → launch → saved world → observer roster; two observers
with different knowledge; remembered old ref after live-data mutation/reload;
unknown/legacy absence and unchanged faction/rules. Run focused roster/launch/
knowledge tests with race checks; push and open its own module PR.

## Task 4 — API transport

Owner: API. Prerequisites: generated SDK from Task 1; pushed session commit from
Task 3. Files: `internal/handlers/dnd5e/session/v1alpha1/get_roster.go`, corresponding
converter/knowledge tests, go.mod/go.sum. Copy AppearanceRef field exactly through
the shared converter; no new endpoint, stats calculation, asset resolver or read.
Test present and empty refs, and reuse by GetKnowledge. Run focused handler tests
and the required `make ci-check` before pushing. Keep integration draft on
pseudo-version pins until provider release tags are adopted.

## Task 5 — Studio and play consumer

Owner: Web. Files: room draft types/validation/edit helper and JSON/YAML codecs,
`WorldBuildingConcept.tsx` and its actor controls, `RoomActorMarkers.tsx`, existing
session actor rendering/resolver, `SessionCanvas.tsx`, `sessionRefreshKeys.ts`,
associated tests and generated SDK dependency pins. Reuse the catalog already
adopted in `4c435d12`; no new asset renderer or stat-block/faction controls.

Choose/clear optional appearance on an existing actor through one document/history
transaction. Exact catalog lookup controls standing/downed URL; explicit missing
refs show a diagnostic, absent override keeps the old rule-ref resolver. Preserve
valid unknown imported refs. Roster refresh on relevant sight knowledge updates
must deliver the captured appearance (inspect current refresh-key mechanism,
not a polling loop). Require actual generated SDK fields, not a patched SDK/cast.
Tests: selection/no-op/clear/undo, serialization/reload, stats/faction invariance,
unknown diagnostic, default/override/downed render and observed roster refresh.
Run focused tests while developing; one `npm run ci-check` at the PR boundary.

## Task 6 — QA and publication order

Use a normal local stack with pushed provider pins and CI-owned proto SDKs. Walk
Studio select → save/reload → launch → see/move/downed; inspect a remembered NPC
without enriching it from current source. Verify unknown ref diagnostics and old
rooms. Report a boundary not run as pending. One bounded independent review,
then focused fixes and the operator QA pass; no repeated broad review rounds.

Merge only on operator authorization: proto contract/SDK; encounter tag → session
released pin/tag → API pins → Web SDK/API integration. Source-only design record
can accompany the wave; there is no automatic merge in this plan.

## Palette extension — provider #303

The operator approved publishing Appearance / Style / Skin palette controls after
the base path's QA. `npc_palette_looks.py` uses the existing atlas resize and
embedded-image replacement helpers, pins every packed source image and base,
checks non-image semantics and publishes explicit `look` family/style/skin metadata.
Default 01_A refs/bytes stay unchanged; 187 additional pairs make 204 Kingdom looks.
The Web catalog validates family/default/tuple identity and the Arrange form stages
one complete look ref; no extra actor/proto fields or runtime material mutation.
The detailed provider task/proof contract is in provider issue #303.

Verification: palette/source corruption and metadata tests, complete provider gates,
family/default/choice tests, actual owner undo/reload, and a native Style03/SkinB
save-and-play walk using the already-integrated observer path. One PR-boundary Web
gate and a bounded review of this extension only; no backend re-review.

## Coverage and seam check

| Requirement | Tasks | Proof |
|---|---|---|
| Model independent of rules/stats/faction | 2,3,5 | Invariance tests and Studio QA |
| Source save/load and reserve survive | 2,3,5 | Decode/compile, persistence, launch and codec tests |
| Observer-safe memory | 2,3,5,6 | Captured testimony, two-observer/reload test, knowledge refresh |
| Exact four-part ref and clear failure | 1,2,4,5 | Ref validation, converter, unknown render test |
| No licensed content copied public | 5 | Generated metadata and ignored runtime sync only |

| Provider | Consumer | Contract / availability / proof |
|---|---|---|
| dungeonspec MonsterPlacement | session Launch | AppearanceRef string; Task 2 pushed pin; launch test |
| encounter SightTestimony | session rosterFrom | Captured opaque ref; Task 2 pin; memory test |
| session PublicMember | API converter | AppearanceRef string; Task 3 pin; converter test |
| proto PublicMemberInfo | API/Web | appearance_ref tag 9; CI SDK after merge; real consumer builds |
| Web room source | API/toolkit compiler | optional appearanceRef on monster declaration; source-carriage QA |
| public roster | existing Web renderer | exact ref → pinned catalog; no editor-document join; render/refresh tests |

No new persistence repository, gameplay events, stats definitions, faction policy
or asset-generation seam is required. Exact implementation details may be refined
from the named callers without changing these ownership boundaries.
