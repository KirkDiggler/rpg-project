# Single-room atlas transport implementation plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development or superpowers:executing-plans to implement this task.

**Goal:** Carry the complete canonical room presentation over the existing authoring/session atlas response.

**Architecture:** Add one JSON string to GetAtlasResponse. The toolkit owns the typed versioned payload; API/web consumers validate it at their boundaries. This is not a new scene system or mechanical channel.

**Tech Stack:** protobuf, Buf, generated Go and Connect-ES/TypeScript SDKs.

**Spec:** [single-room-play.md](single-room-play.md), section 4. Issue: [protos#342](https://github.com/KirkDiggler/rpg-api-protos/issues/342).

## Global constraints

- Provider shipped: encounter v0.87.0, toolkit#1798, merge3e20eaa1.
- Full scene, declared source frame/workspace and doubles retained; no authored monster/start markers in the presentation.
- Atlas/session mechanical channels stay authoritative. No mesh rules, duplicate pose DTOs, or new RPC.
- Empty string means no room presentation (legacy). Nonempty malformed, null or unsupported-version payloads must be refused by consumers, not silently rendered as legacy.
- Commit only proto source; generated SDK output stays ignored. Normal pipeline alone publishes the generated branch and tags.
- Local generation may serve an isolated integrated walk before consumer PR merges. No merge, tag, runtime handoff or asset changes authorized here.

## Task 1 — Add and verify the presentation field

**Worktree:** `rpg-api-protos/.worktrees/342-room-scene-atlas`, branch `feat/342-room-scene-atlas`, base ac54ef8f.

**Modify:** `dnd5e/api/session/v1alpha1/service.proto`, `GetAtlasResponse` after `start = 13`.

**Consumes:** encounter `RoomScenePresentation{Version, Frame, Workspace, Scene}`; version 1 JSON uses `version`, `coordinateFrame`, `workspace`, `scene`.

**Produces:** generated Go `RoomSceneJson string` and TS `roomSceneJson: string`; existing PutDungeonResponse.atlas and GetAtlasResponse use the same field.

- [ ] Check tag14 remains unused. Add the additive source field and concise contract comments:

```proto
  // Optional presentation snapshot: versioned JSON of the toolkit's canonical
  // RoomScenePresentation, including its source coordinate frame, workspace,
  // and complete visual scene (groups, supports, lights and per-piece height).
  // Preserve JSON numbers without narrowing through legacy float pose fields.
  // No author monster/start markers or substitute gameplay state live here;
  // atlas/session mechanical channels remain authoritative.
  // Empty means absent/legacy. Consumers must refuse malformed or unsupported
  // nonempty payloads rather than silently falling back to legacy rendering.
  string room_scene_json = 14;
```

- [ ] Run the normal schema and generated SDK gates (no bespoke tests of protoc mechanics):

```sh
buf format -w --disable-symlinks
buf lint --disable-symlinks
buf format --diff --exit-code --disable-symlinks
buf breaking --disable-symlinks --against 'https://github.com/KirkDiggler/rpg-api-protos.git#branch=main'
buf generate --disable-symlinks
make mocks
GOWORK=off make compile-go
make compile-ts
git diff --check
```

Baseline lint/format/build already passed and npm dependencies are installed. The repo has no active Git hooks; do not install global tooling or bypass any hooks that exist at commit time.

- [ ] Commit only the proto source, push the feature branch and open a Draft PR to main with `Closes #342`, exact gate evidence, downstream consumer notes and Cross-team signature derived from the authenticated login. Report the first PR link immediately; do not wait for review to make it visible.
- [ ] One fresh independent review verifies the additive contract and evidence at the exact pushed head. Parent adjudicates findings; defer nonblocking polish rather than starting an optional fix cycle. No full repeated generated-code test campaign.
- [ ] Return generated output locations for local consumer adoption; do not publish hand-made tags, push generated, merge or switch the live stack.

## Following consumers (separate scoped execution)

The current-ref map identifies API registry/metadata/atlas conversion/start preflight and web codec/setup/Play/full-scene rendering. They remain unimplemented; this proto task does not claim a playable room. Their detailed change/test plans must be grounded in the actual source before their writers run.
