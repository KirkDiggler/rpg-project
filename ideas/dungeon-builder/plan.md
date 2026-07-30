# Dungeon Builder — Implementation Plan

**Status:** execution plan (design approved by Kirk 2026-07-30; this plan rides
the same PR per the Cross-Repo Design Workflow)
**Owner:** asset-pipeline team — spans four repos: rpg-api-protos, rpg-api,
rpg-dnd5e-web, and this repo (tracking only)
**Dependencies:** zero rpg-toolkit changes. The whole arc reuses
`dungeonspec.Load`/`Decode`/`Validate` and `Encounter.ToData()` exactly as
they exist today — verified against `rpg-toolkit/encounter/dungeonspec/*.go`
and `rpg-toolkit/encounter/data.go` while writing this plan.

## Issue / PR structure

- Seven slices below (S0, S1, S2, S3, S4a, S4b, S4c), each = **one issue in
  its owning repo + one ready-for-review PR** (never draft — memory
  `no-draft-prs`).
- Branch base: `origin/main` for rpg-api-protos; **`origin/dev`** for both
  rpg-api and rpg-dnd5e-web (rpg-project CLAUDE.md's base-branch table,
  updated by the 2026-07-28 rule — `dev` replaced web's old `development`
  that day, and rpg-api grew a matching `dev` at the same time; getting
  this wrong sends a PR at the wrong target silently). **Merge direction
  matters as much as the branch name**: every S1-S4c feature PR
  squash-merges INTO `dev` (one clean commit per feature, unchanged); the
  eventual `dev` → `main` release cut for each repo must be a real merge
  commit, never squashed — squashing that cut severs ancestry between the
  two long-lived branches and poisons the next release with false
  conflicts (CLAUDE.md's "Merging a `dev` branch into `main`" section has
  the worked example). This plan's slices only ever squash-merge into
  `dev`; the `dev` → `main` cut is a separate, later, repo-owner action
  outside this arc's scope. **`gh repo view` confirms both repos' GitHub
  default branch is still `main`**, not `dev` — `gh pr create` without an
  explicit `--base dev` on every rpg-api/rpg-dnd5e-web PR in this arc
  silently opens against `main` instead. Pass it every time for S1-S4c.
- Board 19 entry per issue: **Feature = "The Dungeon"** on every slice (fetch
  current field/option IDs with `gh project field-list 19 --owner KirkDiggler`
  rather than hardcoding them here — they're a live board, not a doc fact).
  Team/Kind suggested per slice below; Status advances
  Todo → In Progress → In Review → Done as work proceeds.
- **Opus code-review gate posted on each PR before any merge-ready call** —
  full rigor for S0-S2 (wire contract + concurrent-registry-mutation
  correctness are exactly the "engine/rules invariant" class memory
  `review-rigor-calibration` reserves full rigor for); lighter single-pass
  for S3/S4a-c presentation work, screenshot-judged per that same memory.
- Repo law per repo: rpg-api `make ci-check` + `make pre-commit` before push;
  rpg-dnd5e-web `npm run ci-check`; rpg-api-protos the five-command buf
  checklist in S0 below. Screenshot evidence on every web PR (asset-pipeline
  convention).

## Architecture decision this plan adds: the shared live registry

The design's review already established that `PutDungeon` must mutate the
*same* registry `StartEncounter` reads, concurrency-safely, with no restart
(`ideas/dungeon-builder/design.md`, "What gets mutated, precisely"). Today
that registry is a private, unguarded field on the **lobby** orchestrator
(`Orchestrator.contentSpecs map[DungeonKey]contentSpecResult`,
`rpg-api/internal/orchestrators/lobby/orchestrator.go:151`). The naive
implementation mistake — building a second, private, mutex-guarded map
inside the *new* authoring orchestrator — would compile, pass a unit test in
isolation, and still fail the design's core requirement, because
`StartEncounter` would keep reading the lobby orchestrator's untouched
original map. **This has to be one registry, shared by pointer between two
orchestrators.**

S1 extracts it into its own package:

```go
// internal/dungeonregistry/registry.go
package dungeonregistry

// Entry is one dungeon key's live state: either a successfully compiled
// spec (with its display name, captured separately -- see S1's Name note)
// or a load failure (mirrors today's contentSpecResult).
type Entry struct {
    Compiled dungeonspec.CompiledDungeon
    Name     string
    Err      error
}

type Registry struct {
    mu      sync.RWMutex
    entries map[string]Entry
}

func New(initial map[string]Entry) *Registry
func (r *Registry) Get(key string) (Entry, bool)
func (r *Registry) Put(key string, e Entry)
func (r *Registry) Keys() []DungeonSummary // key + name, for ListDungeons
```

`cmd/server/server.go` builds **one** `*dungeonregistry.Registry` at startup
(the existing `loadContentSpecs` logic moves here, extended per the Name
note below) and passes the same pointer into both `lobbyorch.Config` and the
new `authoringorch.Config`. `lobby.Orchestrator.contentSpecs` (the map field)
is replaced by `registry *dungeonregistry.Registry`; `resolveContentDungeonSpec`
becomes `o.registry.Get(string(key))`.

## S0 — rpg-api-protos: the authoring contract (arc opener)

**Why protos go first:** `PutDungeon`'s floor-plan response is rendered by
the web client — the Boundary Rule's "one conversion point at the
handler/proto boundary" means the contract has to exist before either Go
side can be written against it. One PR, additive only, carries everything
the whole arc needs so S1/S2/S3/S4a-c never block on a second protos PR.

**Issue:** rpg-api-protos, e.g. "Dungeon Builder: authoring contract —
`dungeon_key` + `ListDungeons` on LobbyService, new AuthoringService."
Board 19: Feature "The Dungeon", Team suggested **Platform** (protos is
explicitly Platform territory per rpg-project's ownership-boundary doc),
Kind **Build**.

**Branch:** `git fetch origin && git checkout -b feat/<issue>-dungeon-builder-authoring-contract origin/main`

### Checklist

- [ ] **Add `dungeon_key` to `StartEncounterRequest`** in
      `dnd5e/api/lobby/v1alpha1/service.proto` (field 2, additive):
      ```proto
      message StartEncounterRequest {
        string lobby_id = 1;
        // dungeon_key selects which authored dungeon to build the
        // encounter from. Empty uses the server's existing
        // caller -> env -> default precedence (rpg-api's lobby
        // orchestrator, StartEncounterInput.DungeonKey) -- additive,
        // every existing caller is unaffected.
        string dungeon_key = 2;
      }
      ```
- [ ] **Add `ListDungeons` to `LobbyService`** in the same file — per
      design.md's post-approval correction, this is NOT on the new
      authoring service:
      ```proto
      message ListDungeonsRequest {}

      message DungeonSummary {
        string key = 1;
        string name = 2;
      }

      message ListDungeonsResponse {
        repeated DungeonSummary dungeons = 1;
      }
      ```
      and in the `service LobbyService { ... }` block:
      ```proto
        // ListDungeons returns every dungeon key + display name the server
        // can StartEncounter from. NOT behind the authoring gate -- reads
        // content, mutates nothing; the lobby dropdown needs this with
        // authoring off (design.md's post-approval correction, 2026-07-30).
        rpc ListDungeons(ListDungeonsRequest) returns (ListDungeonsResponse);
      ```
- [ ] **New file** `dnd5e/api/authoring/v1alpha1/service.proto` — own
      subpackage, mirroring the lobby package's precedent
      (`dnd5e/api/lobby/v1alpha1`, not a flat `dnd5e/api/v1alpha1` addition).
      Justify against `docs/how-to/add-a-new-service.md`'s three-question
      checklist before writing: does it belong on an existing service? No —
      `PutDungeon` doesn't fit CharacterService/EncounterService/LobbyService's
      bounded contexts. Does it have its own ownership boundary? Yes —
      content authoring is a distinct aggregate from party/encounter state.
      Will multiple services share it? Not today, but it's not
      speculative — S1 is its live consumer in the same arc (the
      anti-pattern doc's "no consumer in flight" warning doesn't apply).

      **Reuses existing common types** rather than inventing new ones
      (`dnd5e/api/v1alpha1/common.proto` already has exactly the field
      shape needed for structured errors):
      ```proto
      syntax = "proto3";

      package dnd5e.api.authoring.v1alpha1;

      import "dnd5e/api/v1alpha1/common.proto";

      option go_package = "github.com/KirkDiggler/rpg-api-protos/gen/go/dnd5e/api/authoring/v1alpha1;authoringpb";
      option java_multiple_files = true;
      option java_package = "com.kirkdiggler.rpg.api.dnd5e.authoring.v1alpha1";

      // PutDungeonRequest submits a dungeonspec YAML for validation and
      // (unless validate_only) compilation into the live registry. key
      // must match the YAML's own declared `key:` field exactly --
      // mismatch is InvalidArgument (design.md's Key rules).
      message PutDungeonRequest {
        string key = 1;
        string yaml = 2;
        // validate_only compiles and returns floor_plan without
        // persisting or mutating the registry -- the board's live
        // per-edit preview.
        bool validate_only = 3;
      }

      // FloorPlanRoom carries values a client must NEVER re-derive
      // itself (design.md's "Grid math is server-authoritative"
      // principle): start_column is the room's absolute position in the
      // compiled left-to-right chain, not something the client computes
      // from widths + a guessed connector-gap convention.
      message FloorPlanRoom {
        string id = 1;
        string archetype = 2; // entrance|corridor|chamber|boss (toolkit RegionArchetype)
        int32 width = 3;
        int32 start_column = 4;
      }

      // FloorPlanConnector's column is the reserved gap column BETWEEN
      // its two rooms, also server-computed -- not derived from room
      // widths client-side.
      message FloorPlanConnector {
        string door_id = 1;
        bool locked = 2;
        string from_room_id = 3;
        string to_room_id = 4;
        int32 column = 5;
      }

      // FloorPlan is the compiled layout an author's board renders.
      // door_row applies uniformly to every room (dungeonspec's
      // height/2 reserved-row invariant) -- carried explicitly, not
      // hardcoded client-side, so a future toolkit change that makes it
      // non-uniform doesn't silently break the board.
      message FloorPlan {
        repeated FloorPlanRoom rooms = 1;
        repeated FloorPlanConnector connectors = 2;
        int32 height = 3;
        int32 door_row = 4;
      }

      message PutDungeonResponse {
        bool success = 1;
        // field_errors is best-effort in v1: rpg-toolkit's
        // dungeonspec.Validate returns one flat error today, not
        // per-field paths, so a compile failure surfaces as exactly one
        // ValidationError (field="") wrapping that message -- not true
        // per-cell mapping. See plan.md S1's Known v1 limitation.
        repeated dnd5e.api.v1alpha1.ValidationError field_errors = 2;
        FloorPlan floor_plan = 3;
      }

      service AuthoringService {
        // PutDungeon is absent from the server's reflection list
        // (Unimplemented) unless the server was started with the
        // authoring gate enabled -- see plan.md S1.
        rpc PutDungeon(PutDungeonRequest) returns (PutDungeonResponse);
      }
      ```
      This sketch is a concrete starting point, not a rigid mandate — the
      S0 implementer may adjust field numbers/names during `buf lint`, but
      keep the *shape*: every value a naive board implementation might
      otherwise reconstruct with arithmetic (`start_column`, `column`,
      `door_row`) is instead an explicit wire value.
- [ ] `buf format -w`
- [ ] `buf lint` — must pass clean (no `PACKAGE_VERSION_SUFFIX` /
      `RPC_RESPONSE_STANDARD_NAME` surprises; both are already excepted in
      `buf.yaml`).
- [ ] `buf format --diff --exit-code` — must pass clean.
- [ ] `buf breaking --against "https://github.com/KirkDiggler/rpg-api-protos.git#branch=main"` —
      must pass clean; everything above is additive (new fields, new
      messages, new RPC, new service — no renumbering, no renames).
- [ ] `buf generate` — must produce output; spot-check
      `ls gen/go/dnd5e/api/authoring/v1alpha1/*.pb.go` and
      `ls gen/ts/dnd5e/api/authoring/v1alpha1/*.ts`.
- [ ] Doc updates in the same PR per `add-a-new-service.md`'s checklist:
      `docs/architecture/components/authoring-service.md` (new, shape of
      `dice-service.md`), add to `docs/architecture/overview.md`'s repo
      layout block, `docs/status.md` + `docs/quality.md` confidence/grade
      rows for the new service.

**Acceptance:** all five buf commands clean; `docs/status.md` names the new
service; PR references this plan's S0. **Consumers:** rpg-api (S1, S2) and
rpg-dnd5e-web (S3, S4a-c) both regen against this once merged — name that
explicitly in the PR description since it's the reason this slice blocks
four others.

## S1 — rpg-api: the authoring surface

**Issue:** rpg-api, "Dungeon Builder: PutDungeon + authoring gate + shared
registry." Board 19: Feature "The Dungeon", Team **Platform**, Kind **Build**.
Depends on S0 merged (needs the generated `authoringpb` package).

**Branch:** `git fetch origin && git checkout -b feat/<issue>-dungeon-builder-authoring-surface origin/dev`
(rpg-api's integration branch since the 2026-07-28 base-branch rule — not
`main`; PR with `gh pr create --base dev`, see Issue/PR structure above.)

**Get the fresh protos first:**
```bash
GOPROXY=direct go get github.com/KirkDiggler/rpg-api-protos/gen/go@generated
```

### Checklist

**1. The shared registry package (the architecture decision above).**

- [ ] Write `internal/dungeonregistry/registry_test.go` FIRST: `Get` on an
      empty registry returns `ok=false`; `Put` then `Get` round-trips an
      `Entry`; `Keys()` returns summaries sorted by key (deterministic —
      match `content.OverriddenKeys`'s `sort.Strings` precedent so test
      assertions and any future log line are stable); a `-race` test that
      runs concurrent `Put` and `Get` calls and asserts no data race
      (`go test -race ./internal/dungeonregistry/...`).
      ```bash
      go test ./internal/dungeonregistry/... -run TestRegistry -v
      # expect: FAIL (package doesn't exist yet)
      ```
- [ ] Implement `internal/dungeonregistry/registry.go` per the sketch
      above (`sync.RWMutex`-guarded map; `Get`/`Keys` take `RLock`, `Put`
      takes `Lock`).
      ```bash
      go test -race ./internal/dungeonregistry/... -v
      # expect: PASS
      ```
      Commit: `feat(dungeonregistry): concurrency-safe live dungeon spec registry`.

**2. Name capture — a real gap, not an oversight.** `dungeonspec.Load`
   discards the decoded `Spec` (which carries `Name`) after compiling —
   `CompiledDungeon` only has `Params`/`Spawns`
   (`rpg-toolkit/encounter/dungeonspec/compile.go:27-35`). `ListDungeons`
   needs display names, so the registry-build step must call
   `dungeonspec.Decode(raw)` (already exported,
   `rpg-toolkit/encounter/dungeonspec/decode.go:22`) **in addition to**
   `dungeonspec.Load(raw)`, purely to read `.Name` — zero toolkit change,
   one extra already-public function call.

- [ ] Update `internal/orchestrators/lobby/dungeon_spec.go`'s
      `loadContentSpecs` (or move it into `internal/dungeonregistry` as a
      constructor helper — implementer's call, but it becomes the single
      place both `Name` capture and `CompiledDungeon` compile happen
      together) to populate `dungeonregistry.Entry.Name` from `Decode`.
      Existing test: `dungeon_spec_internal_test.go` — extend its fixtures
      to assert `Name` is captured.
- [ ] **Two existing call sites need their signatures updated together,
      not just `resolveContentDungeonSpec`.** The Architecture decision
      above already covers `resolveContentDungeonSpec`
      (`dungeon_spec.go:223`) moving from `o.contentSpecs[key]` to
      `o.registry.Get(string(key))`. The second, easy to miss: **
      `validateDungeonKeyOverride(override string, contentSpecs
      map[DungeonKey]contentSpecResult) error`**
      (`orchestrator.go:224`, called from `lobbyorch.New` right after
      `loadContentSpecs` runs) takes that same raw map as a parameter —
      once it's replaced by `*dungeonregistry.Registry`, this function's
      signature must change too (accept the registry and call `Get`
      instead of map-indexing), or it won't compile. Existing test
      coverage for this function (wherever `orchestrator_test.go` or a
      sibling covers the `RPG_DUNGEON_KEY` override validation path)
      should keep passing unchanged in behavior — only the parameter type
      changes.

**3. `internal/content`'s missing key → filename mapping.** Write-through's
   "overwrite the originating file" rule (design.md's Key rules) needs to
   know which *file* currently declares a key — `content.AllSpecs()` only
   returns `map[string][]byte` (key → bytes), the filename is discarded
   inside `buildRegistry` (`internal/content/registry.go:46-69`). This is a
   real gap, not a re-derivation of something that already exists.

- [ ] Write `internal/content/registry_internal_test.go` additions FIRST:
      a new `FilenameForKey(dir, key string) (filename string, ok bool, err error)`
      — key found → returns the exact filename that declared it; key not
      found → `ok=false`, no error; `dir` unreadable → propagates the same
      wrapped error `readOverrideDir` already produces.
      ```bash
      go test ./internal/content/... -run TestFilenameForKey -v
      # expect: FAIL (function doesn't exist)
      ```
- [ ] Implement `FilenameForKey` in `internal/content/registry.go`,
      reusing `readOverrideDir` + `headerKey` (no new YAML-decoding logic —
      it's the same header-only decode `buildRegistry` already does, just
      returning the filename instead of discarding it).
      ```bash
      go test ./internal/content/... -v
      # expect: PASS, including all pre-existing tests still green
      ```
      Commit: `feat(content): FilenameForKey for write-through targeting`.

**4. The authoring gate + `PutDungeon`.**

- [ ] New package `internal/orchestrators/authoring/`. `Config` requires
      `Registry *dungeonregistry.Registry` and `ContentDir string`
      (**not** read from `os.Getenv` inside the orchestrator — passed in
      by `cmd/server/server.go`, matching `lobbyorch.Config`'s existing
      style of explicit fields over ambient env reads). `New(cfg)` returns
      an error if `ContentDir == ""` — this is the "gate requires
      RPG_CONTENT_DIR, fails fast at construction" decision from
      design.md, enforced here, one required-field check exactly like
      `lobbyorch.New`'s existing `Config.LobbyRepo is required` pattern
      (`internal/orchestrators/lobby/orchestrator.go:162-187`).
- [ ] Write `internal/orchestrators/authoring/put_dungeon_test.go` FIRST,
      covering in this order (each its own subtest / table case):
      1. `ContentDir == ""` at `New` → construction error, no orchestrator
         returned.
      2. Key/YAML `key:` mismatch → `InvalidArgument`-mappable error,
         zero registry mutation, zero file write (assert via a spy/fake
         filesystem or a temp dir + `os.Stat` on the would-be file).
      3. Key charset: `PutDungeon(key="My Dungeon!", ...)` rejected before
         any decode/compile happens — regex `^[a-z0-9-]+$`.
      4. Invalid YAML (fails `dungeonspec.Load`) → `field_errors` has
         exactly one entry (the known v1 limitation — assert this
         explicitly, don't assert "per-field" since that's not true yet),
         registry untouched, no file write.
      5. **Ordering on failure**: inject a write-through failure (temp dir
         made read-only, or a fake writer) on an otherwise-valid spec →
         RPC fails, and the registry (assert via `Registry.Get`) is
         provably unchanged. This is the test that pins design.md's
         "write-through happens first; a write error leaves the registry
         untouched" decision — a naive implementation that swaps the
         registry before writing (or doesn't check the write error) will
         pass every OTHER test here and only fail this one.
      6. Valid spec, `validate_only=true` → `FloorPlan` populated,
         `Registry.Get(key)` still returns `ok=false` (nothing persisted),
         no file written.
      7. Valid spec, `validate_only=false` → file written to
         `ContentDir/<key>.yaml` (new key) or the *originating* file
         (existing key, via `FilenameForKey`), registry updated, AND a
         second `PutDungeon` call with a DIFFERENT valid YAML for the SAME
         key overwrites the same file (assert file count in `ContentDir`
         doesn't grow — this is the shadow-file-prevention test).
      8. No-restart visibility: `PutDungeon` succeeds, then a direct
         `Registry.Get(key)` call (standing in for the next
         `StartEncounter`) sees the new compiled spec — same test process,
         no server restart, proving the shared-pointer architecture above
         actually works end to end at this layer.
      ```bash
      go test ./internal/orchestrators/authoring/... -v
      # expect: FAIL on every case (package/method don't exist yet)
      ```
- [ ] Implement `put_dungeon.go`: decode (key-mismatch + charset checks
      first — cheap, no compile needed), `dungeonspec.Load` (compile +
      validate), on success build `FloorPlan` from
      `compiled.Params.Regions`/`compiled.Params.Connectors` (walking the
      chain to compute `start_column`/`column` — this is the ONE place in
      the whole arc allowed to do that arithmetic, because it's the
      producer, not a consumer, of the compiled layout) plus a throwaway
      `Encounter.InitDungeon(compiled.Params)` call the same way
      `dungeonspec.WorkbenchReport` does
      (`rpg-toolkit/encounter/dungeonspec/workbench.go:42-58`) — reuse
      that exact pattern, don't reinvent it. Then: write-through via
      `os.WriteFile` to the target path from `FilenameForKey` (or
      `<key>.yaml` if new) FIRST; only on success call `Registry.Put`.
      ```bash
      go test ./internal/orchestrators/authoring/... -v
      # expect: PASS, all 8 cases
      go test -race ./internal/orchestrators/authoring/... -v
      # expect: PASS
      ```
      Commit: `feat(authoring): PutDungeon orchestrator — gate, key rules, write-then-swap ordering`.

**5. `ListDungeons` on the lobby orchestrator** (per design.md's
   post-approval correction — this lives in `internal/orchestrators/lobby/`,
   ungated, alongside `StartEncounter`, not in the new `authoring` package).

- [ ] Write a test in `internal/orchestrators/lobby/` FIRST: empty registry
      → empty list, no error; populated registry → summaries for every
      *successfully compiled* key (a key with a load error, per
      `contentSpecResult`'s existing disabled-key handling, is excluded —
      matching how `resolveContentDungeonSpec` already treats a broken
      key as FOUND-but-disabled rather than silently absent for
      `StartEncounter`; `ListDungeons` should still omit it from the
      player-facing list, since offering an unplayable dungeon in a
      dropdown is worse than a temporarily-shorter list).
- [ ] Implement, sourcing from `o.registry.Keys()`.
      Commit: `feat(lobby): ListDungeons — ungated, reads the shared registry`.

**6. Handlers + wiring.**

- [ ] `internal/handlers/dnd5e/authoring/v1alpha1/put_dungeon.go` — outside-in:
      start `codes.Unimplemented`, then wire to the orchestrator once it
      exists (rpg-api's own CLAUDE.md's outside-in workflow).
- [ ] `internal/handlers/dnd5e/lobby/v1alpha1/list_dungeons.go` — new file,
      same directory as the existing `create_lobby.go`/`join_lobby.go`
      siblings.
- [ ] `cmd/server/server.go`: build the shared `*dungeonregistry.Registry`
      once (replacing the inline `loadContentSpecs` call currently feeding
      `lobbyorch.Config` directly, per the Architecture decision above);
      pass it into `lobbyorch.Config.Registry` and (conditionally) into a
      new `authoringorch.Config.Registry`. Register `AuthoringService`
      **only when the gate env var is set**:
      ```go
      const authoringEnabledEnvVar = "RPG_AUTHORING_ENABLED"

      if os.Getenv(authoringEnabledEnvVar) != "" {
          contentDir := os.Getenv("RPG_CONTENT_DIR") // authoringorch.New fails fast if empty
          authoringOrch, err := authoringorch.New(&authoringorch.Config{
              Registry:   sharedRegistry,
              ContentDir: contentDir,
          })
          if err != nil {
              return fmt.Errorf("authoring orchestrator: %w", err)
          }
          authoringHandlerImpl, err := authoringhandler.New(&authoringhandler.HandlerConfig{Orchestrator: authoringOrch})
          if err != nil {
              return fmt.Errorf("authoring handler: %w", err)
          }
          authoringv1alpha1pb.RegisterAuthoringServiceServer(srv, authoringHandlerImpl)
          healthServer.SetServingStatus("dnd5e.api.authoring.v1alpha1.AuthoringService", grpc_health_v1.HealthCheckResponse_SERVING)
      }
      ```
      With the gate off, `AuthoringService` is never registered — `grpcurl
      -plaintext localhost:50051 list` simply won't show it, and any call
      against it gets gRPC's own `Unimplemented`, not a custom check. This
      is the cheapest, most honest way to satisfy design.md's "absent/
      Unimplemented" error-handling line.
- [ ] Update `docs/how-to/run-locally.md`'s env var table: add
      `RPG_AUTHORING_ENABLED` and `RPG_CONTENT_DIR`'s role in the
      authoring path (it already exists for content override, but its
      "required when authoring is on" behavior is new).

**7. Curl (really: grpcurl) smoke test — P1's promised zero-UI proof.**

- [ ] **Prerequisite: Redis running locally** — `cmd/server`'s
      `mustRedisClient()` (`cmd/server/server.go:367-`) builds a Redis
      client at startup and fails construction without one, exactly like
      every other local run of this server
      (`docs/how-to/run-locally.md`'s own Prerequisites section):
      ```bash
      docker run -d --name rpg-redis -p 6379:6379 redis:alpine
      ```
- [ ] New script `scripts/smoke-authoring.sh` (or inline in the PR
      description, implementer's call). **Both `AuthoringService.PutDungeon`
      and `LobbyService.ListDungeons` require authentication** —
      `internal/auth/interceptor.go`'s `skipAuthMethods` exempts only the
      gRPC health check and reflection, nothing else, so this needs
      `AUTH_DEV_MODE=true` at startup and an `Authorization: Dev
      <player-id>` header on every data call, matching
      `docs/how-to/run-locally.md:31,54-64`'s existing pattern exactly
      (including its `server` subcommand — `go run ./cmd/server server`,
      not just `./cmd/server`):
      ```bash
      RPG_AUTHORING_ENABLED=1 RPG_CONTENT_DIR=/tmp/dungeon-authoring-smoke \
        AUTH_DEV_MODE=true go run ./cmd/server server &

      grpcurl -plaintext localhost:50051 list
      # expect: dnd5e.api.authoring.v1alpha1.AuthoringService now appears
      # (no auth needed -- reflection is exempt)

      grpcurl -plaintext \
        -H "Authorization: Dev player-1" \
        -d '{"key": "smoke-test", "yaml": "version: 1\nkey: smoke-test\nname: Smoke Test\nheight: 6\nrooms:\n  - id: a\n    archetype: entrance\n    width: 6\n  - id: b\n    archetype: boss\n    width: 6\nconnectors:\n  - from: a\n    to: b\n", "validate_only": false}' \
        localhost:50051 \
        dnd5e.api.authoring.v1alpha1.AuthoringService/PutDungeon
      # expect: success=true, floor_plan populated, and:
      cat /tmp/dungeon-authoring-smoke/smoke-test.yaml
      # expect: the exact YAML just submitted

      grpcurl -plaintext \
        -H "Authorization: Dev player-1" \
        -d '{}' localhost:50051 \
        dnd5e.api.lobby.v1alpha1.LobbyService/ListDungeons
      # expect: "smoke-test" / "Smoke Test" now in the list, with NO
      # restart between the PutDungeon call and this one
      ```
- [ ] Include this transcript (or the script + its actual output) in the
      PR description as P1's evidence — this is the "curl-testable before
      any UI exists" proof design.md promised.

### Known v1 limitation to state plainly in the PR

`field_errors` is one flat entry per compile failure, not true per-field
mapping — `dungeonspec.Validate` returns a single `error`
(`rpg-toolkit/encounter/dungeonspec/validate.go:58`), formatted with
room/ref context baked into the string
(e.g. `room %q: place %q at %v is on the reserved row`), not a structured
field path. S4a/S4c's "compile errors surface inline, mapped onto board
cells where possible" (design.md) is honestly satisfied by showing that one
message somewhere visible — not by highlighting the specific offending
cell. A future toolkit enhancement to return structured per-field errors is
P4+ follow-up work if the UX gap turns out to matter in practice, not
something this arc should invent unilaterally.

**Acceptance:** all listed tests green including `-race`; `make ci-check`
clean; the grpcurl transcript above in the PR; Opus gate posted before
merge-ready.

## S2 — rpg-api: `dungeon_key` plumbing on `StartEncounter`

**Issue:** rpg-api, "Dungeon Builder: wire dungeon_key on StartEncounter."
Board 19: Feature "The Dungeon", Team **Platform**, Kind **Build**. Small,
independent of S1 (only needs S0's regenerated protos) — can land in
parallel with S1 if convenient, but ordered after in this plan since S3's
dropdown is more useful once both are live.

**Branch:** `git fetch origin && git checkout -b feat/<issue>-dungeon-builder-start-encounter-key origin/dev`
(not `main` — same rpg-api integration-branch rule as S1; `--base dev`.)

### Checklist

- [ ] Write a test FIRST in
      `internal/handlers/dnd5e/lobby/v1alpha1/start_encounter_test.go` (or
      the orchestrator's own test if the mapping belongs there): a request
      with `dungeon_key` set populates `StartEncounterInput.DungeonKey`;
      an empty `dungeon_key` leaves it at the zero value (today's
      behavior, unchanged); precedence order per-lobby key wins over
      `RPG_DUNGEON_KEY` env override wins over the legacy default — this
      is exactly what
      `internal/orchestrators/lobby/start_encounter.go:109-121`'s
      `effectiveKey` logic already implements, this slice only needs to
      stop leaving `in.DungeonKey` at zero.
      ```bash
      go test ./internal/handlers/dnd5e/lobby/... -run TestStartEncounter -v
      # expect: FAIL (dungeon_key not yet read)
      ```
- [ ] One-line handler change: map `req.DungeonKey` →
      `StartEncounterInput.DungeonKey`.
      ```bash
      go test ./internal/handlers/dnd5e/lobby/... -v
      # expect: PASS
      ```
      Commit: `feat(lobby): wire StartEncounterRequest.dungeon_key to the orchestrator`.
- [ ] `make ci-check`.

**Acceptance:** grpcurl smoke test using an **embedded** dungeon key —
`reference-tomb` (the Kirk-authored 3-room tomb shipped as embedded content,
rpg-project#132) or another key already present in
`internal/content/dungeons/*.yaml` — not S1's `"smoke-test"` key. S2 is
independent of S1 and may land first; an acceptance test that only works
after S1's PutDungeon has run wouldn't stand alone. Start a lobby, call
`StartEncounter` with `dungeon_key: "reference-tomb"`, confirm (via logs or
a follow-up query) the encounter used that spec, not the legacy crypt
default. Opus gate before merge-ready (small surface, still touches the
key-resolution precedence — worth the pass).

## S3 — rpg-dnd5e-web: the lobby dungeon dropdown

Delivers rpg-project#131. **Issue:** rpg-dnd5e-web, "Dungeon Builder: lobby
dungeon picker (ListDungeons)." Board 19: Feature "The Dungeon", Team
suggested **UI/UX** (a list/dropdown, no 3D rendering — team-lead/Kirk can
retriage at board-entry time if this reads differently). Depends on S0
(regenerated TS SDK) and S2 (so the picked key actually takes effect).

**Branch:** `git fetch origin && git checkout -b feat/<issue>-dungeon-builder-lobby-picker origin/dev`
— **`origin/dev`, not `main`** (rpg-project CLAUDE.md's base-branch table;
`dev` replaced web's old `development` on 2026-07-28; getting this wrong is
silent and surfaces later as a phantom conflict). PR with
`gh pr create --base dev` — GitHub's default branch for this repo is still
`main`, so an unqualified `gh pr create` targets the wrong branch.

Current pin, for reference (`package.json`, checked while writing this plan):
`"@kirkdiggler/rpg-api-protos": "github:KirkDiggler/rpg-api-protos#v0.1.114"`
— S0's merge publishes a newer tag; bump past `v0.1.114` to pick it up.

```bash
gh release list -R KirkDiggler/rpg-api-protos --limit 5   # find the tag S0 published
npm i --save github:KirkDiggler/rpg-api-protos#v0.1.<N>   # N > 114
# GitHub-tag deps don't always update package-lock.json cleanly -- if the
# commit hash in package-lock.json doesn't match, per rpg-dnd5e-web
# CLAUDE.md: rm -rf node_modules package-lock.json && npm install
```

### Checklist

- [ ] Write a component test FIRST for the new dropdown (wherever the
      lobby host-controls component lives) asserting: renders one option
      per `ListDungeons` result; selecting one and starting the encounter
      sends that key on `StartEncounterRequest.dungeon_key`; host-only
      (matches `StartEncounter`'s existing host-only RPC gating — no new
      client-side auth logic, this is presentation only).
- [ ] Implement the dropdown, wired to a `useListDungeons` hook (or
      whatever this repo's existing hook-naming convention is — check
      `src/api/hooks.ts` for the pattern `useListCharacters`/`useListDrafts`
      already follow and match it exactly).
      ```bash
      npm run ci-check
      ```
      Commit: `feat(lobby): dungeon picker wired to ListDungeons`.
- [ ] Screenshot evidence: the lobby with the dropdown populated
      (`node tools/browser/screenshot.mjs <lobby-url> <out.png>` from
      `game-dev`'s harness), plus a live-play screenshot confirming the
      selected dungeon actually loaded.

**Acceptance:** Kirk live-verifies (rpg-project#131's own bar); screenshots
attached. Light single-pass review per the calibration memory — this is
presentation, not an engine invariant.

## S4a — rpg-dnd5e-web: the `/author` route + board

**Issue:** rpg-dnd5e-web, "Dungeon Builder: /author route — board rendering
the compiled floor plan." Board 19: Feature "The Dungeon", Team suggested
**Assets** (reuses the hex-grid/hex-math seams the asset-pipeline team
already owns — `src/rendering/FloorBuilder.ts`, `src/hooks/wallRuns.ts`;
retriage if this reads as UI/UX instead). Depends on S0 + S1 (needs
`PutDungeon` with `validate_only` live).

**Branch:** `git fetch origin && git checkout -b feat/<issue>-dungeon-builder-author-route-board origin/dev`
(not `development` — renamed 2026-07-28; `--base dev` on the PR.)

Bump `@kirkdiggler/rpg-api-protos` first — same commands as S3's proto-update
step (this slice needs the generated `AuthoringService`/`PutDungeon` TS
types, not just `ListDungeons`).

### Design decision this slice makes: 2D board, not the 3D scene

Design.md didn't specify 2D vs 3D for the board. Given "loop latency is the
ranking design constraint" (design.md's own principle), S4a renders a 2D
top-down hex grid (SVG or canvas — implementer's choice) rather than
spinning up the full Three.js encounter scene just to click-place refs. It
still must not reimplement hex math: reuse the existing odd-q/hex-position
helpers already in `src/rendering/FloorBuilder.ts` and `src/hooks/wallRuns.ts`
for hex-to-screen positioning, the same way S1's `FloorPlan` response means
the *board* never re-derives chain layout. Full 3D preview is explicitly
out of scope / a P4+ idea if it ever turns out to matter.

### Checklist

- [ ] Add `/author` to the existing view-selection pattern (this codebase
      has **no react-router** — routes are a `type AppView = 'home' | ... `
      union plus a URL-param check, like `/playtest` (`?encounterId=`) and
      `/concepts` (`?concept=`) in `src/App.tsx:19-42`). A naive
      implementer reaching for `react-router` here will produce code that
      matches nothing else in the app — don't.
      ```tsx
      const hasAuthorDeepLink = (): boolean =>
        typeof window !== 'undefined' &&
        new URLSearchParams(window.location.search).has('author');
      ```
      **Decision (coordinating-session review, post-design-approval):
      this is a RUNTIME gate, not a build-time one — unlike `/playtest`
      and `/concepts`, do NOT add `import.meta.env.MODE === 'development'`
      here.** The deployed environment is a prebuilt production image
      (rpg-deployment's `docker-compose.prod.yml` pulling a ghcr image) —
      a `MODE === 'development'` check compiles `/author` out of that
      image entirely, which makes design.md's "remote/distributed phase
      is configuration, not architecture — flip the gate in the deployed
      compose" claim false for the web half: there'd be no route left to
      flip on. Instead:
      - `/author` is **lazily code-split** (`React.lazy`/dynamic `import()`)
        in every build, dev and prod alike — visiting the URL param always
        fetches the chunk, but it never bloats the main app bundle. Ships
        as dead weight in a prod build nobody's using authoring on yet;
        acceptable pre-pre-alpha, revisit if bundle-size ever becomes a
        real complaint.
      - On mount, before rendering the editor, probe the api's authoring
        surface with a minimal call:
        ```ts
        // key is deliberately empty -- it fails PutDungeon's charset
        // validation before any decode/compile ever runs, so this is a
        // pure liveness probe, never a real write. Do NOT "fix" this
        // into a valid key.
        await putDungeon({ key: '', yaml: '', validateOnly: true });
        ```
        its actual response content is irrelevant, only the RPC's
        existence is being tested — three-way split on the outcome, not a
        two-way one:
        - `Unimplemented` → the gate is off server-side → render a plain
          "authoring disabled on this server" state instead of the
          editor.
        - `Unavailable` / a transport failure (server unreachable) → a
          distinct "can't reach the server" state with a retry action —
          NOT the editor. A two-way "anything but Unimplemented mounts
          the editor" split would mount it against a backend that can't
          even be reached, let alone serve a save.
        - Anything else, including the expected `InvalidArgument` from
          the deliberately-invalid probe payload → the service exists and
          answers → mount the real editor.
      - Net effect: **one env var, `RPG_AUTHORING_ENABLED` on rpg-api,
        governs both halves.** Flipping it in the deployed compose is
        the whole remote-phase story, exactly as design.md claims — now
        actually true end to end, not just true for the api half.
- [ ] Add `yaml` as a **direct** dependency (`npm install yaml`) — it's
      already present *transitively* in `package-lock.json` (`^2.8.2`,
      pulled in by something else), but a transitive version isn't a
      contract; this design's "hard requirement: comment-preserving
      round-trip" needs a pinned direct dependency.
- [ ] Write round-trip tests FIRST (new `src/author/yamlRoundTrip.test.ts`
      or similar): parse a fixture YAML with a comment (reuse or mirror
      `showcase.yaml`'s "colonnade: 8 pillars framing the center lane"
      style comment as the fixture), mutate one field via the CST
      `Document` API, re-serialize, assert the comment survived AND the
      byte output is stable when nothing changes (parse → stringify with
      no edits must be byte-identical to the input).
      ```ts
      import { parseDocument } from 'yaml';
      const doc = parseDocument(fixtureYaml);
      // doc.toString() with zero mutations must equal fixtureYaml exactly
      ```
      ```bash
      npm test -- yamlRoundTrip
      # expect: FAIL (module doesn't exist)
      ```
- [ ] Implement the YAML pane's parse/mutate/stringify layer against the
      `Document` CST API.
      ```bash
      npm test -- yamlRoundTrip
      # expect: PASS
      ```
- [ ] Write a board contract test FIRST (fixture: a recorded `FloorPlan`
      response, e.g. captured from S1's grpcurl smoke test): the board
      renders exactly the rooms/connectors/door_row the fixture specifies;
      changing the fixture's `door_row` changes which row is unclickable
      — **without any code change**, proving legality isn't hardcoded.
      This is the test design.md's polish pass specifically asked for:
      "a contract test against a recorded response fixture, not a
      reimplementation of the compiler's layout rules."
      ```bash
      npm test -- board
      # expect: FAIL
      ```
- [ ] Implement the board component reading `FloorPlan` (via `PutDungeon`
      with `validate_only: true`, called on every YAML-pane edit,
      debounced — the live per-edit preview design.md describes).
      ```bash
      npm run ci-check
      ```
      Commit: `feat(author): /author route — board renders server-compiled FloorPlan, comment-preserving YAML pane`.
- [ ] Screenshot: the `/author` route open against a real dungeon YAML,
      board and YAML pane both visible.

**Acceptance:** round-trip tests green (byte-stable, comments survive);
board contract test green; screenshot attached. Light single-pass review.

## S4b — rpg-dnd5e-web: the palette

**Issue:** rpg-dnd5e-web, "Dungeon Builder: /author palette — props/monsters,
flag gating, rolled-content panel." Board 19: Feature "The Dungeon", Team
**Assets** (propManifest.ts is this team's existing surface). Depends on
S4a (needs the board + YAML pane to place into).

**Branch:** `git fetch origin && git checkout -b feat/<issue>-dungeon-builder-author-palette origin/dev`
(not `development`; `--base dev` on the PR.)

### Checklist

- [ ] Palette source: `src/components/hex-grid/propManifest.ts`'s existing
      `resolvePropVariant`/keys index for props; check the monster
      equivalent (toolkit#847's palette — locate the web-side monster ref
      list the same way, likely a sibling file or the character-creation
      class-model constants pattern) for monsters. **Read-only overlays**
      for door/start/entrance markers (derived from `FloorPlan.connectors`
      and each room's `archetype == "entrance"` — not draggable palette
      items; design.md's palette-honesty correction). The **boss pin** is
      the one draggable exception (`boss.at` is real schema).
- [ ] Write tests FIRST: dragging a monster ref onto a cell sets
      `blocks_movement`/`blocks_los` controls to **disabled** (gray, not
      just unchecked) — dungeonspec validation rejects both flags on
      monster placements
      (`rpg-toolkit/encounter/dungeonspec/validate.go:332-337`); dragging a
      prop enables them. A count-based `obstacles:` entry (no `at:`) never
      appears as a draggable/clickable board item — it renders only in an
      off-board "rolled content" list, unchanged by board interaction.
      `place:` is never offered as a palette action when the target room's
      `pattern: scattered` (validation rejects the combination outright —
      `validate.go:280`).
      ```bash
      npm test -- palette
      # expect: FAIL
      ```
- [ ] Implement. Prop-ref existence gap (design.md, already documented):
      the palette itself is the only guard against a nonexistent prop ref
      — there's no server-side registry check today, so this slice's
      palette-driven placement (never hand-typed) is what keeps the "save
      green, renders nothing" failure mode from reaching authors in
      practice.
      ```bash
      npm run ci-check
      ```
      Commit: `feat(author): palette — propManifest source, ref-type-gated flags, read-only door/start overlays`.
- [ ] Screenshot: palette open, a prop placed with editable flags, a
      monster placed with disabled flags, the rolled-content panel showing
      an unplaced `obstacles:` entry.

**Acceptance:** tests green; screenshot set attached. Light single-pass
review.

## S4c — rpg-dnd5e-web: save + play handoff

**Issue:** rpg-dnd5e-web, "Dungeon Builder: /author save (PutDungeon) +
play handoff." Board 19: Feature "The Dungeon", Team suggested **UI/UX**
(the flow/handoff logic, not asset rendering — retriage if it reads
differently). Depends on S4a, S4b, and S3 (the lobby dropdown play jumps
to).

**Branch:** `git fetch origin && git checkout -b feat/<issue>-dungeon-builder-author-save-play origin/dev`
(not `development`; `--base dev` on the PR.)

### Checklist

- [ ] Write tests FIRST: Save calls `PutDungeon` with `validate_only:
      false`; a `field_errors` response (the one-flat-entry v1 shape from
      S1) surfaces as a visible inline message — do NOT write a test
      asserting per-cell error highlighting, since S1's known limitation
      makes that untrue; assert instead that the message is legible and
      doesn't silently vanish. Play, after a successful save, navigates to
      the lobby with the just-saved key preselected in S3's dropdown.
      Leaving that encounter (any exit path — victory, defeat, or
      Abandon) returns to `/author`, not `/home`. Play always starts a
      **fresh** encounter (Redis persists stale ones across content swaps
      — design.md's named trap, unchanged by this slice, just don't
      regress it).
      ```bash
      npm test -- authorSave
      # expect: FAIL
      ```
- [ ] Implement.
      ```bash
      npm run ci-check
      ```
      Commit: `feat(author): save via PutDungeon, play handoff to preselected lobby`.
- [ ] Screenshot / short clip: edit → save → lobby-with-dropdown-preselected
      → play → leave → back at `/author`.

**Acceptance:** tests green; the handoff screenshot sequence attached.
Light single-pass review — this is the slice Kirk's live walk (below)
exercises end to end, so treat the walk itself as the real acceptance gate.

## Review rigor (calibrated)

S0-S2 carry real invariants — a wire contract four repos depend on, and
concurrent mutation of server state that a bad implementation could get
wrong in a way no single unit test catches (the shared-registry pointer
architecture). Full Opus gate, no shortcuts. S3/S4a-c are presentation over
an already-locked contract: light single-pass review, judged primarily by
the screenshot evidence and Kirk's live walk, per memory
`review-rigor-calibration`.

## Traps for the implementer (do not rediscover)

- **The shared registry must be ONE object, passed by pointer to both
  orchestrators.** A private map inside the new authoring orchestrator
  compiles, might even pass its own unit tests, and still fails the
  design's core "no restart" requirement. See the Architecture decision
  section above.
- **Write-through before registry swap, always.** A `PutDungeon` that
  updates the registry before confirming the file write can leave a spec
  playable now and gone after the next restart. S1's test case 5 pins
  this.
- **`CompiledDungeon` has no `Name` field.** Call `dungeonspec.Decode`
  separately to get it — don't try to thread a name through `Load`'s
  return value, it isn't there.
- **`content.AllSpecs()` returns key→bytes, not key→filename.**
  Write-through's "overwrite the originating file" rule needs a new
  `FilenameForKey` — it doesn't already exist.
- **`dungeonspec.Validate` returns one flat error, not structured
  field-path errors.** Don't design a UI that promises per-cell error
  highlighting in v1 — it can't be built honestly yet.
- **doorRow is `height/2` for every room, uniformly** — but the FloorPlan
  response carries it explicitly anyway (`FloorPlan.door_row`); the board
  reads that field, it does not hardcode the formula.
- **Chain layout (`start_column`/connector `column`) is server-computed in
  `FloorPlan`, never re-derived client-side** from room widths — that's
  exactly the "room-chain offsets" class of math design.md's server-
  authoritative principle forbids the client from doing.
- **This codebase has no react-router.** `/author`'s view *selection* is a
  `URLSearchParams` + `AppView` union check, same pattern as `/playtest`
  and `/concepts` — reaching for a router here produces code that matches
  nothing else in the app. Its *visibility*, unlike those two, is NOT the
  same: `/author` is gated at runtime by probing the api, not at build
  time by `import.meta.env.MODE` — a prebuilt prod image must still be
  able to serve it once the server-side flag flips. See S4a.
- **`yaml` is only a transitive dependency today** — add it directly;
  don't rely on whatever version another package happens to pull in.
- **odd-q hex parity is server-side only** — the board never steps
  same-row/same-column to find a neighbor; it renders positions the
  server already computed.
- **Redis persists stale encounters across content swaps** — the editor's
  play action must always start a fresh encounter, every slice that
  touches play/StartEncounter should re-confirm this, not just S4c.
- **`buf breaking` is blocking in rpg-api-protos CI** — everything in S0
  is additive by design specifically so this passes clean with no
  `breaking-change-approved` label needed.
- **rpg-api and rpg-dnd5e-web both branch from and PR into `origin/dev`,
  not `main`** — every S1-S4c slice above says this explicitly for a
  reason: a near-identical mistake (branching web from a stale/wrong base)
  already happened once in this workspace (2026-07-26), and the base
  itself moved again since (2026-07-28: web's `development` renamed to
  `dev`, rpg-api grew a matching `dev`). `gh repo view` confirms both
  repos' GitHub default branch is still `main`, so `gh pr create` needs
  an explicit `--base dev` every time, not just a correctly-cut branch.
- **Squash into `dev`, never squash `dev` → `main`.** Every feature PR in
  this arc squash-merges; the release cut is someone else's later action
  and must be a real merge commit — mixing the two breaks the next
  release (rpg-project CLAUDE.md's "Merging a `dev` branch into `main`,"
  worked example from the 2026-07-28 incident this rule came from). Not
  this arc's job to perform, but worth knowing why `dev` exists as a
  separate branch at all.
- **No draft PRs** — every slice's PR opens ready-for-review, or it skips
  Kirk's review queue silently.

## Pointers

- Design: `design.md` in this folder (Kirk-approved, 2026-07-30, including
  the post-approval ListDungeons-ungating correction this plan carries).
- Shared registry precedent for "why not just a mutex in the new package":
  `rpg-api/internal/orchestrators/lobby/orchestrator.go:145-260` (today's
  single-owner `contentSpecs` field, being generalized).
- `dungeonspec` package: `rpg-toolkit/encounter/dungeonspec/` — `Load`
  (compile.go), `Decode` (decode.go), `Validate` (validate.go),
  `WorkbenchReport` (workbench.go, the existing compile-and-describe
  precedent `PutDungeon`'s floor-plan step mirrors).
- Existing grpcurl workflow: `rpg-api/docs/how-to/run-locally.md`.
- Proto conventions: `rpg-api-protos/docs/how-to/add-a-new-service.md`,
  `run-buf-checks-locally.md`.
- Web hex math to reuse, not reinvent: `src/rendering/FloorBuilder.ts`,
  `src/hooks/wallRuns.ts`.
- rpg-project base-branch table + Cross-Repo Design Workflow:
  `rpg-project/CLAUDE.md`.

## Acceptance — the live walk

Per design.md's own Testing section: the loop itself is judged by live
play, not review machinery. When S0-S4c are all merged, the plan's
acceptance bar is Kirk walking the full loop in the running game: open
`/author`, edit a dungeon (place a prop, adjust a monster, watch the board
update live), save, land in the lobby with that dungeon preselected in the
S3 dropdown, play it, leave, land back at `/author`. Any friction found
here — not a review comment — is what decides what P4+ picks up first.

— asset-pipeline agent, on behalf of KirkDiggler
