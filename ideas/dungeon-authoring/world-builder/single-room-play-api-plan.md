# Single-room API adoption implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans to execute the test/code/check steps.

**Goal:** Validate, save and launch the complete authored v3 room through the existing registry and lobby, preserving it on the atlas wire and reload.

**Architecture:** The toolkit compiles/validates the source and owns geometry/rules. The SDK validates and serializes the canonical presentation; API conversion copies that string. The file registry keeps original YAML bytes. Existing lobby StartEncounter remains the only launch path.

**Tech Stack:** Go, released toolkit/Go protobuf SDKs, existing testify/miniredis suites.

**Spec:** [single-room-play.md](single-room-play.md). Issue: [API#1003](https://github.com/KirkDiggler/rpg-api/issues/1003).

## Global constraints

- Work only in `rpg-api/.worktrees/1003-world-builder-play`, branch `feat/1003-world-builder-play`, base origin/dev9aa9422b. Other repos/modules and the live stack are read-only.
- Providers shipped: encounter v0.87.0; session v0.94.0 (mergebea10c1b); protos v0.1.198 (generated commit6ea2b2e6dffca16528a6eda491714381a8416a92). Go may resolve the generated commit to a pseudo-version because the published proto tag is at the repository root; record that provenance, never invent a nested module tag.
- No committed replace/go.work/local-toolkit. No generated SDK edits or global tool installation.
- Full source/metadata/scene preserved, strict refusal, no lossy v2 fallback, no custom geometry, no new RPC or simplified dungeon. Keep valid legacy v2 content working.
- Source actor cells are axial, but `Compiled.PartyStart[].At` and `Compiled.Monsters[].At` are legacy offsets. Keep existing API `cellOf` / toolkit HexCellAt conversion exactly once, including negative odd rows.
- No monster stat/weapon/mind interpretation in API. Existing SDK Spawn owns defaults and behavior. Only content lookup by ref is permitted here.
- Keep the existing actual-party seat-capacity refusal before writes; do not add a competing calculator or bypass host/all-ready gates.
- No live :3030/:8110/Redis changes, seeding, assets, deployment, merges or cleanup in this implementation task.

## Task 1 — Version-neutral source and thin atlas adoption

**Modify:** `go.mod`, `go.sum`, `internal/sessionworld/sessionworld.go`, `internal/handlers/dnd5e/session/v1alpha1/convert.go`.

**Tests:** existing sessionworld suites; converter tests in `internal/handlers/dnd5e/session/v1alpha1/convert_test.go`; authoring handler tests in `internal/handlers/dnd5e/authoring/v1alpha1/handler_test.go`.

**Interfaces consumed:** `dungeonspec.Load([]byte) (Compiled,error)`, `Compiled.Key/Name/Field/PartyStart/Monsters`, `sdk.Atlas.RoomSceneJSON string`, `sessionpb.GetAtlasResponse.RoomSceneJson string`.

- [ ] Adopt only the needed released dependencies:

```sh
GOWORK=off go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter@v0.87.0 github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/session@v0.94.0
GOWORK=off go get github.com/KirkDiggler/rpg-api-protos/gen/go@6ea2b2e6dffca16528a6eda491714381a8416a92
```

- [ ] Add red tests that compile valid v3 source, retain key/name and full Field.RoomScene/Placed, preserve negative/odd-row actor positions and carry the exact nonempty canonical JSON through the shared converter. Assert legacy nil scene remains empty. Unsupported/malformed source stays an ErrBadSpec/ValidationError, not a fallback or partial world.
- [ ] Replace the v2-only Decode/Validate/Compile chain with the provider dispatch:

```go
spec, err := tkdungeonspec.Load(raw)
if err != nil {
    return nil, fmt.Errorf("load spec: %w", err)
}
```

Use `spec.Key` and `spec.Name` wherever the current function uses decoded metadata (including the multi-boss error). Do not rebuild or strip the compiled field; buildWorld already forwards `Field: field` whole. Do not change cellOf.

- [ ] Copy the already validated SDK string in AtlasToProto:

```go
RoomSceneJson: a.RoomSceneJSON,
```

Keep the existing converter signature and shared authoring/session caller. No re-marshalling via map[string]any/float32, second envelope, duplicated scene DTO or API-side schema/geometry validator.

## Task 2 — Resolve content before accepting an invalid authored room

**Modify:** `internal/sessionworld/sessionworld.go` (or a focused sibling in that package if needed).

**Consumes:** existing `monster/monsters.ByRef(ref string) (Constructor,bool)` from the toolkit rulebook. SDK itself uses this registry (`session/entities.go`); it explicitly supports lookup at author/load time. This is not a new catalog or an API rule.

- [ ] Add a red unknown-monster-ref case: a syntactically valid but unavailable ref must be refused before a registry entry or session is created. Keep valid legacy shipped dungeons green.
- [ ] For each compiled monster, resolve its ref using ByRef before returning an accepted Dungeon. Do not invoke constructors, inspect stat bundles or decide weapons/minds. Use the existing ValidationError pattern so authoring returns a body error and leaves old content intact:

```go
if _, known := monsters.ByRef(m.Ref); !known {
    return nil, &tkdungeonspec.ValidationError{Errors: []tkdungeonspec.FieldError{{
        Path: "", // document-level error unless the exact source path is known
        Message: fmt.Sprintf("monster %q references unknown monster %q", id, m.Ref),
    }}}
}
```

Here `id` is the already validated/minted member ID from memberIDFor, not a new naming algorithm. Naming the actual ID/ref is preferable to guessing an index: legacy Compiled.Monsters omits non-monster place entries. Do not parse error text or add a second YAML parser to guess paths.

This preflight is before acceptance/registry write; new and boot-loaded entries use the same Compile path. Existing SDK Join/Spawn still execute the real lifecycle. Do not widen this to legacy loadout/action validation or fix unrelated partial-I/O-failure recovery.

## Task 3 — Public registry and launch regressions (required deliverable)

**Create:** `internal/dungeons/registry_single_room_test.go`, defining `TestSingleRoomRegistrySuite` and reusing real registry/projector fixtures. A small licensed-content-free v3 fixture may live under owning testdata; only asset reference strings, never model bytes.

**Create:** `internal/orchestrators/lobby/start_encounter_single_room_test.go`, adding `TestStartEncounter_PlaysCompleteSingleRoom` and a capacity refusal case to existing `SessionStackSuite` (existing fixture setup runs real SDK/miniredis, not a mocked Spawn/Atlas).

- [ ] Registry suite: validate-only writes nothing; save/Get keeps exact original v3 bytes (including comments) and key/name; List includes correct metadata; fresh registry over the saved directory produces the same scene. Assert complete scene data against the authored fixture, not only preview/live symmetry. Bad footprint/start or unknown monster update refuses and keeps prior bytes/entry unchanged.
- [ ] Shared wire converter/authoring tests: `PutDungeonResponse.atlas` and GetAtlas use the same new field; exact JSON string/double values are retained; legacy absence remains empty. Do not test generated protobuf mechanics.
- [ ] Real launch case: seed one owned character and a ready lobby with existing suite helpers; Put the v3 room into the real registry; StartEncounter with that key; read the member atlas through the real handler/SDK. Assert nonempty full canonical presentation equals the authored fixture and preview; stable monster ID/ref and SDK-owned default action/mind data survive. Negative/odd source cell coordinates arrive at the correct axial location (derive the expected source cell, not a second copy of the offset algorithm).
- [ ] Reload/isolation: re-read persisted session/world through a fresh SDK/host read path backed by the stored data, and verify the same atlas. Publish a changed room after launch and prove the running session still has its original scene. No live Docker/Redis required; use the existing test stores.
- [ ] Capacity case: a valid one-seat source and two ready members must refuse using the existing guard before any session/encounter/character writes or EncounterStarted event. Unknown monster source must already have been refused before acceptance; prove no partial launch entry was created.

```sh
GOWORK=off go test ./internal/sessionworld ./internal/dungeons ./internal/handlers/dnd5e/authoring/v1alpha1 ./internal/handlers/dnd5e/session/v1alpha1 ./internal/orchestrators/lobby -count=1
GOWORK=off go test ./internal/dungeons -run '^TestSingleRoomRegistrySuite$' -count=1 -v
GOWORK=off go test ./internal/orchestrators/lobby -run '^TestSessionStackSuite$/TestStartEncounter_PlaysCompleteSingleRoom$' -count=1 -v
```

Confirm named tests actually execute. A passing command with no matching tests is not evidence. These public tests are writer-owned requirements, not deferred parent work.

## Publication and verification

- [ ] Update the affected authoring/lobby documentation and narrow status/quality statements where this change invalidates them; keep browser proof explicitly pending. Do not rewrite unrelated docs or raise scores without evidence.
- [ ] Parent baseline affected-package tests, format and release-pin checks passed; normal local hook was generated by `make install-hooks` and excluded locally from source control. Keep it active.
- [ ] Run normal check-only gates without bypass, capture logs and inspect actual outputs:

```sh
GOWORK=off go mod tidy
git diff --check
GOWORK=off make pre-commit
GOWORK=off make ci-check
```

Use deliberate formatting commands when needed, not hook bypass. Re-run `gofmt -l` after the last edit; the last SDK writer's claimed format pass did not match committed bytes, and CI caught it. Do not commit generated local hook files.

- [ ] Commit/push the meaningful first working checkpoint and open a Draft PR to **dev**, `Closes #1003`. Writer owns publication and must read back the exact authenticated Cross-team signature and source head. Report the Draft URL promptly, before review finishes.
- [ ] One fresh independent review publishes inline findings and a verdict at the pushed head. Parent verifies the actual test code/evidence, not merely writer attestation. No automatic optional-fix cycle or merge.

## Next boundary

Web v3 editing/codec, snapped monster/start controls, real Save/Play and full-scene runtime rendering remain unimplemented. API success is not browser acceptance. Keep the restored environment pinned until a deliberately tested handoff with a renewed save/export checkpoint if the user has made new edits.
