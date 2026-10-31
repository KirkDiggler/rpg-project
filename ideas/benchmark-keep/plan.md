---
name: Benchmark keep — implementation plan
status: PLAN derived 2026-10-30 from design.md R1–R3 and README.md; checked against the inspected baseline below; amended the same day from the design PR's review round (bench build, supported envelope, walls per shared edge, header options, timing line).
authority: design.md (R1–R3, Law), README.md (ownership table and refusals), story rpg-project#740, lane rpg-project#739
planning guide: ../../.agents/skills/design/planning.md
---

# Benchmark keep — implementation plan

## Goal and scope

Two generated fixtures, the keep at 10,000 hexes and the same keep at the
30,000-hex ceiling, propped and garrisoned from palettes, sit on the api
shelf, open in the studio and play from the lobby. One harness command
records the same table for the castle and both fixtures and posts it on the
story issue. The baseline table and its hot-spot list open the two follow-on
stories.

**Non-goals.** No rendering, studio or server optimisation (follow-on
stories). No minds or patrols on placed monsters (O1). No change to the
dialect, the protos or the atlas wire shape (site-format O5 stays open). No
hand-authored content.

**Repositories and order.** Develop outside-in, merge inside-out.

| slice | repo / module | base | PR shape |
|---|---|---|---|
| S0 | rpg-toolkit `rulebooks/dnd5e/encounter` + rpg-project docs | main | one toolkit PR (`cmd/convert-v4`), one docs PR (how-to); overnight-coder sized |
| S1 | rpg-toolkit `rulebooks/dnd5e/encounter` | main | one PR: `RowsOf`, `keepgen` package, `cmd/keepgen`, benches; tag minted on merge |
| S2 | rpg-api; rpg-dnd5e-web | dev; dev | api PR: pin the S1 tag, two fixtures, currency test, registry timing line. Web PR: the supported envelope's caps, every open and render refusal checked against the ceiling |
| S3 | rpg-dnd5e-web; game-dev | dev; main | web PR: `isDevLike()` predicate, bench build mode, marks and probe mounts. game-dev PR: `envs/bench.env`, bench web mode, `tools/browser/bench/` |
| S4 | rpg-project | — | baseline comment on #740, hot-spot list, two follow-on story issues |

S2's web PR needs the S1 generator's ceiling output to measure the envelope;
it develops on S1's pushed commit and merges after the S1 tag exists. S3's
game-dev PR needs S2 on dev to run end to end. Merge order S1 → S2 api →
S2 web → S3 web → S3 game-dev → S4.

## Finish line

- Sub-issues S0–S4 under #740 closed by their "done means"; the story table
  updated the day each moves.
- S1 tagged; the api PR on the tag, no pseudo-version.
- `bench-keep` and `bench-keep-30k` listed under References on a stack built
  from dev/dev; both open in the studio without refusal; both start from the
  lobby picker with one seeded character.
- `tools/browser/bench/run.sh bench` completes on the operator's machine on a
  hardware renderer and prints three tables; the first baseline comment is on #740 with the
  commit of every repo it measured.
- A hot-spot list on #740 naming, per layer, the rows furthest over R1's
  budgets, and two follow-on story issues (playtime rendering; studio) that
  link to it.

## Requirement labels

| label | requirement | source |
|---|---|---|
| B1 | A plan file carries rooms `{id, name, kind, width, depth, side, lighting}`, `doors`, `courtyard`, `grounds`, `partyStart`, `exits`, `palettes`; a missing or unknown field is refused by name. | Law Plan, R1 |
| B2 | `Layout` scales room dimensions by `min(1.5, sqrt(hexes/planHexes))` and sizes the grounds so the floor equals the requested count exactly; the same room ids, doors and names at both sizes. | Law One keep, R2 |
| B3 | A wall is one line per shared edge of two regions, with quarter-line geometry so no wall covers a cell centre; each plan door is one opening with an attached door bound `{}`. | R2, Law One keep |
| B4 | Props are placed per kind palette at `props` per 100 room hexes and `grounds-props` per 100 grounds and courtyard hexes; two thirds carry `propDeclarations` that block movement, tall ones block sight; none on a door threshold, the party start, an exit or a monster cell. | Law Props, R3, Defaults |
| B5 | Monsters are declared per kind palette at `monsters` per 100 floor hexes with a `startingCell`; no bindings. | Law Monsters |
| B6 | Output is version 5 with canonical row runs, deterministic for equal inputs, with a header comment recording the command line. | Law Proof, README Emit |
| B7 | The generator loads its output and refuses to write when Load fails, the compiled `PartyStart` seats fewer than four, or any region is unreachable from the start through doors. | Law Proof |
| B8 | The ceiling fixture's geometry (regions + walls) is under 100 KB. | site-format R1 via R2 |
| B9 | The two fixtures are on the api shelf, keys equal file names, listed and openable; an api test proves each equals the pinned generator's output for its recorded options. | Law Shelf |
| B10 | The studio supports at least 4,000 items and 2,500,000 envelope characters; every refusal it applies on open and render (items, characters, wall pieces per wall, catalog) admits the ceiling fixture at default density. | Law Supported envelope, O6 |
| B11 | The web exposes `performance.mark` points (behind `?perfMarks`) and the perf probe (behind `?perfProbe`) in current play, the studio canvas and the studio 3D view, whenever the build is development-like. | Law Harness, Law Bench build |
| B12 | One command brings up a bench env on its own ports, writes the fixtures into that env's content dir, builds the web in bench mode, drives studio and play on a hardware renderer and refuses a software one, and prints one table per fixture with R1 budgets beside the rows. | Law Harness |
| B13 | The table carries: file (bytes, geometry bytes, envelope chars, hexes, regions, props, monsters); toolkit (compile, derivation, load ms); server (registry compile ms from the timing line, GetAtlas ms and bytes, validate-only PutDungeon ms, Move ms); studio (open → first paint ms, polygon count, stroke latency ms, preview round-trip ms, regions-list render ms, JS heap); play (atlas → first frame ms, frame p50/p95 ms, draw calls, triangles, geometries, JS heap, renderer string). | Law Harness |
| B16 | A `bench` vite mode builds a minified bundle in which `isDevLike()` is true; `production` keeps it false; `api/auth.ts`, `App.tsx` and the probe mounts read the predicate and nothing else reads `MODE` for a gate. | Law Bench build, O5 |
| B14 | Each run is a comment on #740; the story body links the latest; the hot-spot list is drawn from it. | Law Where numbers live |
| B15 | A `convert-v4` command converts an old-dialect export in place with the existing converter, and a how-to names it. | story S0 |
| K1 | Castle goldens, the QA castle routes and every existing shelf file keep compiling and playing. | must keep working |

## Constraints

- Toolkit: one nearest-`go.mod` module per PR; `go test -race ./...` and `golangci-lint run ./...` in the module; clean tree after `go mod tidy` and `go fmt`; commands live under the module's `cmd/`, run with `go run`.
- rpg-api: `make test`; `make pre-commit`; pin by `GOWORK=off go get <module>@<tag>` then `go mod tidy`; a ready PR carries no pseudo-version; the registry refuses boot on any non-compiling shelf file.
- rpg-dnd5e-web: `npm run ci-check` at the PR boundary; head current with dev before merge.
- game-dev: workspace scripts run from the root checkout; `envs/local/` is ignored, a committed env is `envs/<name>.env`; :3030 is the operator's port.
- Prop refs come from the web's pinned catalog (`src/generated/worldAssetCatalog.ts`), not the provider's newer one.
- Team signature only on commits, PRs, issues and comments.

## Inspected baseline

| repo | revision | note |
|---|---|---|
| rpg-toolkit | origin/main ceda8ddd | encounter module past v0.156.0; `bigSite` at `dungeonspec/site_scale_test.go`; `cellRowsOf` in `dungeonspec/convert_v4.go`; an unrelated `rowsOf` in `dungeonspec/compile.go`; two `cmd/` workbenches |
| rpg-api | origin/dev 69478de | 12 shelf files; castle 61,837 B, 1,586 hexes, 4 props, 18 monsters; no registry timing line |
| rpg-dnd5e-web | origin/dev 43a8e089 | caps `MAX_ITEMS = 200`, `MAX_JSON_LENGTH = 500_000`, `MAX_WALL_PIECES_PER_WALL = 200` (throws at render); probe gated on `MODE !== 'development'` and mounted on the legacy route only; dev player and `?playerId` gated on development mode in `api/auth.ts` and `App.tsx`; pinned catalog 268 refs + 44 legacy; regions list in `encounter-studio/LayoutRoomList.tsx` |
| rpg-game-assets | main b4032db | provider catalog 314 |
| game-dev | main bc22cd4 | `scripts/dev-env.sh` starts vite dev; `tools/browser/screenshot.mjs` |

Probe reports (case file): the design PR thread on rpg-project.

## Gaps resolved under the design's authority

| gap | resolution | authority |
|---|---|---|
| G1 the plan's hex total at keep scale | `planHexes` is the plan's sum of rooms, courtyard and grounds at scale 1 and the embedded plan is drawn so it equals 10,000; the keep fixture is therefore scale 1 exactly, and `-hexes` above it scales rooms by the rule and the grounds absorb the rounding | Law One keep |
| G2 a prop footprint may straddle a region edge | a prop belongs to the room it was drawn in; its footprint stays inside that room's rectangle by construction (placement cells are inset by the footprint's half-extent) | Law Props |
| G3 "tall blocks sight" needs a rule | the palette entry says `blocksLineOfSight`; the generator applies it as written; the default palettes mark shelves, statues, clothes racks and crate stacks as sight-blocking | Law Props |
| G4 seats for four | `Prove` reads `len(compiled.PartyStart)` from `dungeonspec.Load` and requires at least four; no second seat derivation | Law Proof |
| G5 reachability rule needs a mechanism | `Prove` floods the compiled floor from the start, crossing every door (all open), and compares the reached set to the union of regions | Law Proof |
| G6 fixture keys | `bench-keep`, `bench-keep-30k`; `^[a-z0-9-]+$` holds | Law Shelf |
| G7 which monsters | the castle's refs: `guard`, `knight`, `commoner`, `prisoner`; barracks and gatehouse take guards and knights, service rooms commoners, the prison prisoners | Law Monsters |
| G8 lighting per room | the plan gives each room an intensity; default 1 for halls, 0.6 for service rooms and passages, 0.3 for the crypt, mausoleum and hidden stash; grounds 1 | Law Plan |
| G9 production web in dev-env | `RPG_WEB_MODE=preview` runs `npm run build` then `npx vite preview --host --port <web port>` with the same `VITE_API_HOST` | Law Harness |
| G10 play by key from a script | the lobby picker labels a reference `<name> (reference)`; the driver picks by name; `?playerId=` resumes after reload | README |
| G11 palette refs | full refs from the pinned catalog, by room kind. Halls: `dnd5e:props:fantasy-kingdom:table_fancy_01`, `…:table_fancy_03`, `…:table_fancy_07`, `…:chair_fancy_01`, `…:chair_fancy_02`, `…:bench_seat_01`, `…:statue_knight_01`. Service: `…:barrel_01`, `…:crate_stack_01`, `…:crate_stack_02`, `…:sack_stack_01`, `…:shelf_03`, `…:bread_crate_01`. Quarters: `…:bed_01`, `…:bed_02`, `…:bed_03`, `…:side_table_01`, `…:chest_01`, `…:clothes_rack_01_preset_01`. Library: `…:shelf_03`, `…:books_01`, `…:books_02`, `…:books_03`, `…:books_04`. Chapel: `…:bench_seat_pew_01`, `dnd5e:props:dark-fortress:altar_01`, `dnd5e:props:dark-fortress:candle_stand_02`. Prison: `dnd5e:props:dark-fortress:bench_01`, `dnd5e:props:dark-fortress:barricade_01`. Grounds and courtyard: `…:barrel_01`, `…:crate_stack_03`, `…:statue_plinth_01`. `…` is `dnd5e:props:fantasy-kingdom`. Sight-blocking: shelves, statues, clothes racks, crate stacks. | Law Plan, README refusal |
| G12 which gates the bench build opens | the dev player fallback (`api/auth.ts`), the `?playerId` override and dev routes (`App.tsx`), the perf probe and the marks; all through `isDevLike()`; `client.ts`/`streamLogging.ts` logging keeps its own development-only check because the harness does not want it | Law Bench build |

## Tasks

### Task T0: `convert-v4` command and how-to (S0)

**Delivers:** B15.
**Owner:** rpg-toolkit encounter module (`cmd/convert-v4`); rpg-project docs (`docs/howto/convert-site-v4.md`).
**Prerequisites:** none; `dungeonspec.ConvertV4` exists.
**Files:** create `rulebooks/dnd5e/encounter/cmd/convert-v4/main.go`, `main_test.go`; create `rpg-project/docs/howto/convert-site-v4.md`.
**Interfaces:** `go run ./cmd/convert-v4 -in <file> [-out <file>]`; default writes in place; exit 1 with the converter's error on refusal.
**Behavior:** reads the file, calls `ConvertV4`, writes the result, then `dungeonspec.Load`s the result and refuses to write if it fails. A version-5 input is reported as already converted and left unchanged.
**Tests:** `TestConvertV4Command_ConvertsFixture` on a copy of the v4 testdata castle: output loads, byte-equal to the golden the existing `convert_v4_test.go` produces; `TestConvertV4Command_RefusesV5` leaves the file untouched.

- [ ] Add the two tests; run; expect compile failure (no command).
- [ ] Implement the command.
- [ ] `go test -race ./cmd/convert-v4/ ./dungeonspec/` in the module; `golangci-lint run ./...`.
- [ ] Write the how-to: where an export comes from, the command, what a refusal means.

**Verification:** from `rulebooks/dnd5e/encounter`: `go run ./cmd/convert-v4 -in /tmp/x.yaml` on a v4 file prints the output path; on a v5 file prints "already version 5".
**Completion evidence:** PR links, test output.

### Task T1: export the row-run encoder (S1)

**Delivers:** B6 (one spelling). Record: the converter's golden tests prove the rename changes no output.
**Owner:** rpg-toolkit encounter `dungeonspec`.
**Prerequisites:** none.
**Files:** modify `dungeonspec/convert_v4.go` (rename the unexported `cellRowsOf` to the exported `CellRows` and move it to `dungeonspec/single_room_cells.go` beside `ExpandRows`); modify `dungeonspec/regions_helper_test.go` and the converter's callers. The unrelated `rowsOf(entries []AnswerSpec, …)` in `dungeonspec/compile.go` keeps its name; `CellRows` cannot be confused with it.
**Interfaces:** `func CellRows(cells []RoomCell) []RoomCellRow`; rows ascending by r, spans ascending, inclusive, merged; empty input → nil.
**Tests:** `TestCellRows_Canonical` (shuffled input → the same rows as sorted input; `ExpandRows(CellRows(x), "t", sink)` with a collecting `errSink` reports nothing and yields the set of x); `TestCellRows_Empty`.

- [ ] Tests first; implement; run `go test -race ./dungeonspec/`.

**Verification:** as above.

### Task T2: `keepgen` package (S1)

**Delivers:** B1–B7, G1–G5, G7, G8, G11.
**Owner:** rpg-toolkit encounter `dungeonspec/keepgen`.
**Prerequisites:** T1.
**Files:** create `dungeonspec/keepgen/plan.go` (schema + strict decode), `plan/keep.yaml` (embedded default; FadedPez's rooms as in rpg-project#739; palettes per G11), `layout.go`, `place.go`, `emit.go`, `prove.go`, `keepgen.go` (`Generate`), tests beside each.
**Interfaces:**
```go
type Options struct {
    Hexes          int     // target floor count
    Seed           int64
    PropDensity    float64 // props per 100 room hexes, default 12.5
    GroundsDensity float64 // props per 100 grounds hexes, default 2.5
    MonsterDensity float64 // monsters per 100 floor hexes, default 1
    Key            string  // file key, e.g. bench-keep
    CommandLine    string  // recorded in the header comment
}
type Report struct { Hexes, Regions, Walls, Doors, Props, Blocking, Monsters int; GeometryBytes int; Scale float64 }
func LoadPlan(r io.Reader) (*Plan, error)
func DefaultPlan() *Plan
func Generate(p *Plan, o Options) ([]byte, Report, error)
func OptionsFromHeader(doc []byte) (Options, error) // the header comment → the Options that made it
```
**Behavior:** `Generate` runs Layout → Place → Emit → Prove; on a Prove failure it returns the error and no bytes. Equal `(p, o)` → equal bytes. Density zero means none. `Hexes` below the plan's scale-1 total is refused ("plan needs at least N hexes").
**Tests:**
- `TestLoadPlan_RefusesUnknownField`, `TestLoadPlan_RefusesDoorToUnknownRoom`.
- `TestLayout_ScalesAndFills`: `Hexes: 10000` → `Report.Scale == 1` and floor count 10,000; `Hexes: 30000` → floor count 30,000 exactly, `Scale == 1.5`; every room width/depth ≤ 1.5× plan; room ids and door pairs equal the keep-scale layout's.
- `TestPlace_Walls`: each pair of adjacent regions yields exactly one wall; no wall segment contains a cell centre of the compiled floor; every plan door yields one opening with an attached door whose binding has neither `closed` nor `locked`; no wall in the 30k output lays more than 200 render pieces at the studio's piece length (the value read from the web's `structuralWallEditing.ts` at planning, recorded as a constant with its source).
- `TestPlace_KeepsThresholdsClear`: no prop footprint cell on a door threshold, the start, an exit or a monster cell (iterate placements against the compiled floor).
- `TestPlace_Densities`: at defaults on 10,000 hexes, props within ±10% of expected per kind, blocking ≈ two thirds, monsters within ±10%.
- `TestEmit_Deterministic`: two calls → equal bytes; `TestEmit_HeaderCarriesCommand`.
- `TestProve_RefusesUnreachable`: a plan whose `doors` omit one room → error naming the room; `TestProve_RefusesSeats`: start room shrunk to one cell → error naming the seat count from `compiled.PartyStart`.
- `TestGenerate_Loads`: both default sizes load with `dungeonspec.Load`; region count equals plan rooms + courtyard + grounds + implicit.
- `TestGenerate_Golden`: the SHA-256 of the 10k output at seed 1 equals a pinned constant, so any shift in the emitter's output is visible in the toolkit PR that causes it (and tells the api to regenerate).
- `TestOptionsFromHeader_RoundTrip`: `OptionsFromHeader(out)` equals the `Options` that produced `out`.

- [ ] Tests first by file; implement; `go test -race ./dungeonspec/keepgen/`.

**Verification:** `go test -race ./... && golangci-lint run ./...` in the module.

### Task T3: `cmd/keepgen` (S1)

**Delivers:** B6 header, the command surface.
**Owner:** rpg-toolkit encounter `cmd/keepgen`.
**Prerequisites:** T2.
**Files:** create `cmd/keepgen/main.go`, `main_test.go`.
**Interfaces:** flags `-plan <path|embedded> -hexes 10000 -seed 1 -props 12.5 -grounds-props 2.5 -monsters 1 -key bench-keep -out <file>`; prints the `Report` as one line; exit 1 on a Prove error.
**Tests:** `TestMain_WritesAndReports` to a temp file; `TestMain_RefusesBelowPlan`.

- [ ] Tests; implement; run.

**Verification:** `go run ./cmd/keepgen -hexes 30000 -key bench-keep-30k -out /tmp/k.yaml` prints a report with `hexes=30000` and the file loads.

### Task T4: benches and the budget pin (S1)

**Delivers:** B8; toolkit rows of B13.
**Owner:** rpg-toolkit encounter `dungeonspec`.
**Prerequisites:** T2.
**Files:** modify `dungeonspec/site_scale_test.go`: add `TestKeepCeiling_GeometryUnder100KB` (generate 30k at defaults; assert regions+walls bytes < 100 KB); add `BenchmarkFieldLoad/keep` and `/keep-30k` sub-benchmarks over generated bytes; keep `bigSite` as the uniform pin.
**Tests:** as named.
**Verification:** `go test ./dungeonspec/ -run 'KeepCeiling' -bench 'FieldLoad' -benchtime 3x`; record the three numbers (load, field, partitions) per size on S1's sub-issue.

### Task T5: fixtures on the shelf (S2, api)

**Delivers:** B9, K1.
**Owner:** rpg-api.
**Prerequisites:** S1 tag.
**Files:** modify `go.mod` (pin encounter tag); create `content/bench-keep.yaml`, `content/bench-keep-30k.yaml` (generated with `-seed 1`, defaults, keys as named); create `internal/dungeons/bench_fixtures_test.go`; modify `internal/dungeons/registry.go` (`NewFileRegistry` measures its compile loop) and `cmd/server/server.go` (one log line `content registry: compiled <n> files in <ms>ms`).
**Interfaces:** the test calls `keepgen.OptionsFromHeader(file)` then `keepgen.Generate(keepgen.DefaultPlan(), opts)` and asserts byte equality with the shelf file, header included; bytes rather than decoded documents, deliberately, because the header is part of the artifact the harness copies. The two bench keys are the only files the test names.
**Tests:** `TestBenchFixtures_MatchGenerator` (both files); the existing registry boot test keeps passing with the two files present (no count is pinned).
**Behavior:** the timing line is the server row "registry compile ms" in B13.

- [ ] Pin; generate; add test; `make test`; `make pre-commit`.

**Verification:** `make test`; a local server boots and `ListDungeons` carries both keys.

### Task T6: caps from the yardstick (S2, web)

**Delivers:** B10, O6.
**Owner:** rpg-dnd5e-web `concepts/world-building`.
**Prerequisites:** the ceiling fixture bytes (from S1's pushed commit).
**Files:** modify `serialization.ts` (`MAX_ITEMS = 4000`, `MAX_JSON_LENGTH = 2_500_000`, each with a comment naming the Supported envelope); `structuralWallEditing.ts` only if the ceiling's longest wall exceeds `MAX_WALL_PIECES_PER_WALL` (T2's `TestPlace_Walls` is meant to keep it under); note in `roomDraft.ts` that the paint cap `MAX_FLOOR_HEXES = 65536` already admits 30k and that `contentGrid`'s `MAX_GRID_*` clamp rather than refuse; add `fixtures/bench-keep-30k.yaml` as a test fixture, kept current by hand on regeneration (README trade-off; the harness's live Open on the shelf is the real currency check).
**Interfaces:** the bounds are the Law's, not a multiple of the fixture; the fixture test proves it sits under them.
**Tests:** `draftFile.test.ts`: "imports the ceiling fixture with no refusal" and asserts its item count and envelope length are each under half the bound; a render test mounts `LayoutViewport` with the ceiling draft and asserts no refusal notice (wall pieces, grid clamp); `serialization.test.ts`: one over `MAX_ITEMS` is refused.
**Behavior:** Open and Import YAML of the ceiling fixture succeed; the render path shows no refusal.

- [ ] Measure: `readSingleRoomDungeon` on the fixture in a test prints `items` and `json.length`; set the bounds; run every open and render refusal against the fixture; tests; `npm run ci-check`.

**Verification:** the fixture opens from References on a stack built from the api PR and this branch.

### Task T7: bench build mode, marks and probe mounts (S3, web)

**Delivers:** B11, B16, G12.
**Owner:** rpg-dnd5e-web.
**Prerequisites:** none (independent of T6; merges after it).
**Files:** create `src/devMode.ts` (`export function isDevLike(): boolean` over `import.meta.env.MODE`); modify `api/auth.ts` (`getAuthDecision` dev fallback reads `isDevLike()`), `App.tsx` (`isDevelopment` / `devPlayerIdOverride` read `isDevLike()`), `dev/DevPerfProbe.tsx` and `components/game/EncounterMap.tsx` (probe gate reads `isDevLike()`); add `"build:bench": "vite build --mode bench"` to `package.json` scripts (vite accepts any mode; `.env.bench` is not needed). Marks: `concepts/world-building/singleRoomDungeon.ts` (`studio:open:decoded`), `WorldBuildingConcept.tsx` (`studio:open:imported`, `studio:stroke:commit`), `concepts/encounter-studio/LayoutViewport.tsx` (`studio:open:first-paint` after the grid's first commit; `studio:stroke:painted` in a frame callback after commit), `concepts/encounter-studio/LayoutRoomList.tsx` (`studio:regions:rendered` after its commit), `concepts/world-building/useRoomPublishing.ts` / `author/authoringRpc.ts` (`studio:preview:sent`, `studio:preview:returned`), `api/useSessionAtlas.ts` (`play:atlas:requested`, `play:atlas:received`), `components/session/SessionEncounterView.tsx` (`play:scene:built`), `components/session/SessionCanvas.tsx` (`play:first-frame`; mount `DevPerfProbe` in `SessionScene` behind `?perfProbe`), `concepts/world-building/WorldBuildingViewport.tsx` (mount `DevPerfProbe` behind `?perfProbe`).
**Interfaces:** `isDevLike()` is true for modes `development` and `bench`, false otherwise; it is the only place a gate reads `MODE` (client logging keeps its own development check, G12). Mark names exactly as listed, under `performance.mark`, fired only when `isDevLike()` and `?perfMarks` is present; the probe's existing `PERF_PROBE_RESULT:` line and `window.__rpgPerfProbeLatest`, plus the renderer string from `WEBGL_debug_renderer_info` added to its result.
**Tests:** `devMode.test.ts` (three modes); `auth.test.ts` dev fallback under `bench`; a vitest spy on `performance.mark` for the studio open path (decoded, imported, regions, first-paint marks fire in order); the probe mount render test with the query param.

- [ ] Tests; implement; `npm run ci-check`; `npm run build:bench` produces a bundle and `npm run build` still does.

**Verification:** `vite preview` of the bench bundle with `?playerId=x&perfMarks` signs in as the dev player and `performance.getEntriesByType('mark')` after opening the castle lists the open marks in order; the production bundle with the same query stays unauthenticated.

### Task T8: bench env and production web mode (S3, game-dev)

**Delivers:** B12 (env half), G9.
**Owner:** game-dev.
**Prerequisites:** none.
**Files:** create `envs/bench.env` (`RPG_API_REF=dev`, `RPG_DND5E_WEB_REF=dev`, `RPG_API_HOST_PORT=8161`, `RPG_WEB_HOST_PORT=3061`, `RPG_WEB_MODE=bench`, `RPG_REDIS_PERSIST=0`); modify `scripts/dev-env.sh` (`RPG_WEB_MODE` read; `bench` branch runs `npm run build:bench` then `npx vite preview --host --port $RPG_WEB_HOST_PORT` with the same `VITE_API_HOST`; `status` shows the mode). The env's runtime root is `.runtime/bench/` (only `local/*` names live under `.runtime/local/`).
**Interfaces:** `scripts/dev-env.sh up bench` serves the bench bundle on :3061 against the api on :8161.
**Tests:** `scripts/verify-workspace.sh` still passes.

**Verification:** the served index references built assets and not vite's dev client; `?playerId=x` signs in.

### Task T9: drivers and report (S3, game-dev)

**Delivers:** B12, B13.
**Owner:** game-dev `tools/browser/bench/`.
**Prerequisites:** T5–T8 on dev/main.
**Files:** create `tools/browser/bench/run.sh`, `studio.mjs`, `play.mjs`, `report.mjs`, `README.md`.
**Interfaces:**
- `run.sh <env> [keys…]` (default keys `the-mad-king bench-keep bench-keep-30k`): copies `rpg-api/content/<key>.yaml` into `.runtime/<env>/sources/rpg-deployment/content/`, restarts the api, runs `go test -bench FieldLoad` in the toolkit checkout, runs both drivers per key, scrapes the api log for `completed in`, calls `report.mjs`, prints the tables to stdout and writes `bench-<date>.md` into the directory named on the command line.
- Both drivers launch Chromium headed (`headless: false`) by default, or headless with `--use-gl=angle --use-angle=gl --ignore-gpu-blocklist` when `BENCH_HEADLESS=1`; each reads the `WEBGL_debug_renderer_info` unmasked renderer string from a probe canvas before measuring and exits non-zero with the string when it contains `SwiftShader` or `llvmpipe`; the string lands in the table.
- `studio.mjs <url> <key>`: Home → "Open Encounter Studio" → "Open from server" → "Open <key>" → "Replace with <key>"; reads marks (including `studio:regions:rendered`), counts `svg polygon`, performs one 20-cell brush stroke and reads the stroke marks, reads the preview marks, reads `performance.memory` where present.
- `play.mjs <url> <key> <playerId>`: Home → Play → lobby → picker "<name> (reference)" → Ready → Start; reload with `?perfProbe&perfProbeMs=8000&perfMarks`; reads marks and the probe result; one Move via the UI and the api log carries its time.
- `report.mjs`: joins the inputs into one markdown table per key with a `budget` column from R1 (file < 100 KB geometry; Move < 100 ms; frame p95 ≤ 16.7 ms).
**Tests:** `report.mjs` has a fixture-driven test (`node --test`) that renders a known input to the expected table.

**Verification:** `tools/browser/bench/run.sh bench` prints three tables; the castle rows are within a factor of two of the numbers already known (castle load 6.6 ms; Move about 190 ms on the castle bench).

### Task T10: baseline and hot-spot list (S4)

**Delivers:** B14; the finish line.
**Owner:** rpg-project (issue comments and two new story issues).
**Prerequisites:** T9 on main, S2 on dev.
**Behavior:** run `run.sh bench` three times; post the median table per key as one comment on #740 with every repo's commit; write the hot-spot list under it: per layer, the rows furthest over budget, each with the file the probe reports name; open "Encounter playtime rendering" and "Encounter studio performance" stories that link the comment; update the #740 roadmap table.

**Verification:** the comment exists; the story body links it; both follow-on issues link back.

## Requirement coverage

| requirement | tasks | proof |
|---|---|---|
| B1 | T2 | `TestLoadPlan_*` |
| B2 | T2 | `TestLayout_ScalesAndFills` |
| B3 | T2 | `TestPlace_Walls` |
| B4 | T2 | `TestPlace_KeepsThresholdsClear`, `TestPlace_Densities` |
| B5 | T2 | `TestPlace_Densities` |
| B6 | T1, T2, T3 | `TestCellRows_*`, `TestEmit_*`, `TestGenerate_Golden`, `TestMain_WritesAndReports` |
| B7 | T2 | `TestProve_*` |
| B8 | T4 | `TestKeepCeiling_GeometryUnder100KB` |
| B9 | T5 | `TestBenchFixtures_MatchGenerator`; `ListDungeons` on a local stack |
| B10 | T2, T6 | `TestPlace_Walls` (pieces); ceiling import and render tests; Open on a stack |
| B11 | T7 | marks spy test; probe render test |
| B16 | T7, T8 | `devMode.test.ts`, `auth.test.ts`; the bench bundle signs in on :3061 and the production bundle does not |
| B12 | T8, T9 | `run.sh bench` end to end |
| B13 | T4, T5, T7, T9 | report fixture test; the printed tables; the registry timing line |
| B14 | T10 | the comment on #740 |
| B15 | T0 | `TestConvertV4Command_*` |
| K1 | T5 | `make test` (goldens, castle tests) |

## Provider/consumer seams

| provider | consumer | contract | availability | proof |
|---|---|---|---|---|
| T1 `CellRows` | T2 `Emit` | `[]RoomCell → []RoomCellRow` canonical | same PR | `TestCellRows_Canonical`; `TestGenerate_Loads` |
| T2 `Generate`, `OptionsFromHeader` | T3 command, T4 benches, T5 api test | `(Plan, Options) → bytes, Report, error`; header → `Options`; equal inputs equal bytes; golden hash pins the output | same PR; api via tag | `TestEmit_Deterministic`; `TestGenerate_Golden`; `TestOptionsFromHeader_RoundTrip`; `TestBenchFixtures_MatchGenerator` |
| T5 registry timing line | T9 `report.mjs` | `content registry: compiled <n> files in <ms>ms` | api dev after T5 | castle "registry compile ms" row populated |
| T7 `isDevLike()` + bench mode | T8 env, T9 drivers | `vite build --mode bench` → dev player, `?playerId`, marks, probe on | web dev after T7 | the bench bundle signs in; production stays shut |
| T2 `TestPlace_Walls` piece bound | T6 render refusal | longest wall ≤ 200 pieces at the studio's piece length | same PR | the ceiling render test shows no refusal |
| T2 palette refs (G11) | T6 Open path | `assetRef` present in `WORLD_BUILDING_CATALOG_BY_REF` | web pinned catalog read at planning | ceiling import test |
| T5 shelf files | T9 `run.sh` copy, T6 Open, play | key equals file name; version 5 lists | api dev | `ListDungeons` carries both |
| T7 marks and probe | T9 drivers | mark names as listed; `PERF_PROBE_RESULT:` line | web dev | `report.mjs` renders non-empty studio and play rows |
| T8 env | T9 `run.sh` | `:3061` web, `:8161` api, bench bundle, runtime root `.runtime/bench/` | game-dev main | the built-assets check; the copy lands where the api mounts it |
| api `logFunc` lines | T9 | `✓ <method> completed in <ms>ms` | exists on dev | castle Move row populated |

## Open items

- Resolved assumptions: G1–G12 above.
- Defaults awaiting the operator on the design PR: the bench build mode (O5) and the supported envelope's bounds (O6); a veto changes T7's gate list or T6's two constants and nothing else.
- Agreed deferrals: O1 driven garrison; O3 autosave at scale (recorded as a finding if the ceiling's autosave fails); O4 runs on the wire.
- Blockers: none. T9 waits on T5–T8 being on their branches.
