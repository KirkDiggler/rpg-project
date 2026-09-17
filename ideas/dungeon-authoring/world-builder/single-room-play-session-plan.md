# Single-room session presentation bridge implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans to implement this task.

**Goal:** Preserve the encounter-owned room scene through SDK preview and member atlas reads.

**Architecture:** `session.Atlas.RoomSceneJSON` is a versioned JSON string of the canonical encounter presentation, encoded at the shared SDK projection. No copied scene DTO or exported encounter type crosses S2. The API subsequently copies this validated JSON to the proto field; the renderer strictly decodes it.

**Tech Stack:** Go, encoding/json, existing session/encounter SDK and testify suites.

**Spec:** [single-room-play.md](single-room-play.md), section 4; Kirk explicitly approved the bounded session-module extension. Issue: [toolkit#1804](https://github.com/KirkDiggler/rpg-toolkit/issues/1804).

## Global constraints

- Only `rulebooks/dnd5e/session` is writable. Other modules, especially encounter, are read-only.
- Use released encounter v0.87.0. Proto343 is merged; generated v0.1.198 contains `room_scene_json`.
- Keep one canonical scene schema, source doubles/frame/workspace, complete items/groups/supports/lights/height and nil/empty values. No actor/start markers in presentation.
- Empty SDK string means absent/legacy. Unsupported or invalid nonnil presentations fail closed, never become absence. Ask encounter.ValidateRoomScene; do not copy its schema rules.
- `Manager.Atlas` continues to use `enc.AtlasFor(member)`; `AtlasOf` uses the authored-world path. Do not substitute whole-world reads or reconstruct geometry.
- No proto/API/web changes, new gameplay, content-loading work, global tool changes, merges, tags, runtime changes or cleanup.
- Local integrated walking may consume a pushed SDK commit where owning consumer policy allows; final release pins follow actual CI tags. No committed local replacements/go.work.

## Task 1 — Carry, validate and verify the canonical snapshot

**Worktree:** `rpg-toolkit/.worktrees/1804-session-room-scene`, branch `feat/1804-session-room-scene`, base3e20eaa1.

**Modify:** module `go.mod`/`go.sum`; `types.go` (Atlas); `convert.go` (projectAtlas); `read.go` (Atlas/AtlasOf); `convert_test.go` (projection audit); existing direct projection tests in `convert_internal_test.go`.

**Create tests as needed:** `room_scene_test.go` for public SDK behavior and `room_scene_internal_test.go` for projection refusal/copy behavior. Reuse existing fake repositories/capabilities and testify patterns; do not introduce a second test framework.

**Consumes:** `encounter.Atlas.RoomScene *encounter.RoomScenePresentation`, `encounter.ValidateRoomScene(*RoomScenePresentation) []RoomSceneDefect`.

**Produces:** documented `session.Atlas.RoomSceneJSON string` with `json:"room_scene_json,omitempty"`. The payload is the canonical presentation itself, not a second envelope or scene-only subset. `projectAtlas` returns `(Atlas, error)` so integrity failures propagate to both public reads.

- [ ] Bump only this module's provider dependency and establish the red projection regressions:

```sh
cd rulebooks/dnd5e/session
GOWORK=off go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter@v0.87.0
GOWORK=off go test ./... -run 'TestConvertSuite|TestRoomScene' -count=1
```

Use the existing completeness guard as part of the red proof: the new provider fields must be consciously accounted for, not ignored by disabling the test. New tests should assert real scene values, not merely the presence of a JSON string.

- [ ] Add the SDK field and one serialization branch to the existing shared projection. The error shape below is the intended behavior (adapt formatting to local conventions):

```go
if in.RoomScene != nil {
    if defects := encounter.ValidateRoomScene(in.RoomScene); len(defects) != 0 {
        return Atlas{}, fmt.Errorf("project atlas room scene: %s: %w", defects[0], ErrInvalidWorld)
    }
    raw, err := json.Marshal(in.RoomScene)
    if err != nil {
        return Atlas{}, fmt.Errorf("project atlas room scene: %w: %w", ErrInvalidWorld, err)
    }
    out.RoomSceneJSON = string(raw)
}
```

Return `out, nil` only after successful projection. Both `Atlas` and `AtlasOf` propagate the error with their existing operation prefix and return no partial atlas. Update direct internal callers/tests for the error result; do not panic or ignore it.

- [ ] Preserve the audit mechanism: record `encounter.Atlas.RoomScene` as a rename to `RoomSceneJSON` with the S2/canonical-encoding reason. `encounter.Atlas.Placed` is engine geometry, not required for this presentation bridge: document its deliberate omission because movement/sight remain engine answers and the current wire has no raw-contributor consumer. Do not hide RoomScene in omissions or broaden the audit allow-list. If a real existing consumer requires raw Placed values, stop and report that source evidence.

- [ ] Add focused testify regressions at the shared projection and public read seams:
  - nil presentation yields empty string; an explicitly valid empty visual scene yields nonempty versioned JSON with its arrays/frame/workspace retained.
  - Full scene contains a fractional/negative posed grouped prop, a supported lit prop and non-default height. Compare decoded canonical values and selected raw numeric values, not a symmetric custom encoder/decoder pair.
  - Mutating the input graph after projection cannot alter returned JSON; mutating/overwriting a returned string cannot affect a subsequent read.
  - Invalid version, invalid graph/frame and nonfinite number are refused with ErrInvalidWorld and no partial atlas. No NaN/null/legacy fallback.
  - Build a valid ONE-region room world (not the three-region tomb fixture), start it with existing fake repositories/capabilities and known member `alice`, then compare AtlasOf with member Atlas. JSON-round-trip the stored world/session and read again via a fresh Manager over the same repositories; scene and mechanical channels survive unchanged.
  - Absent/unknown member still refuses through the public member-shaped read; legacy concealed-world tests remain green. Do not obtain full atlas as a workaround for a refused member projection.

The existing `AtlasRegionsSuite` in `atlas_regions_test.go` demonstrates Config, fakes, StartSession and member Atlas. Use those actual APIs. Provider validation already forbids concealed/multi-region room presentations; do not change that rule.

- [ ] Run focused tests, then the module gates and normal commit hooks:

```sh
GOWORK=off go test ./...
GOWORK=off go test -race ./...
GOWORK=off golangci-lint run ./...
GOWORK=off go fmt ./...
GOWORK=off go mod tidy
git diff --check
```

Parent baseline full session tests passed before the pin bump; normal `.githooks/pre-commit`, goimports and lint are present. Run no cross-module tidy/test commands. No unapproved tool installation or hook bypass.

- [ ] Commit only session-module changes, push, open the first working Draft PR to main with `Closes #1804`, exact evidence and the actual authenticated Cross-team signature. Read the PR body back and confirm it contains `— cross-team agent, on behalf of <login>` rather than a placeholder. Commit/push/publication are writer-owned, not a parent-only follow-up.
- [ ] One fresh independent review checks the exact pushed head, original tests/guards and both read paths, publishes inline findings and a verdict. Parent adjudicates; no optional-polish loop, no automatic merge.

## Downstream boundary

API still needs version-neutral dungeonspec.Load and Compiled.Key/Name, direct validated SDK JSON-to-proto carriage, definition preflight and tests. Its actual-party-seat capacity guard already exists before writes; retain and exercise it, do not add a second one. The browser Play leg remains separate and unimplemented. This bridge does not by itself make :3030 playable.
