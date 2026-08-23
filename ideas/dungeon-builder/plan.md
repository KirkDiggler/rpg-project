# Dungeon Builder on the Session Stack — plan

**Design:** `design.md` (rpg-project PR #255, rulings 2026-08-23) · **Slice:** rpg-project#256 under journey #169 · **Process:** `how-we-build` — proto merges first; toolkit / rpg-api / web build in parallel against it; Kirk walks the branch once; merge bottom-up.

One PR per module (`module-level-work-breakdown`). Each PR lists its files, the interfaces it owns, the tests that gate it, and the command that proves it. Implementers read the owning repo's AGENTS.md first; nothing here overrides it.

## Order

```
P  protos        ──────────────┐
T1 toolkit encounter (regions) │ parallel after P merges
T2 toolkit dungeonspec v2      │  (T2 depends on T1's tag; T3 on T1+T2)
T3 toolkit session (atlas)     │
A  rpg-api registry + RPCs     │  (pins T1/T2/T3 branch pseudo-versions via scripts/dev-env.sh pin)
W  web /author + preview       │  (runs against branch api)
                               └─ Kirk's walk → merge T1→T2→T3→A→W, re-pinning real tags
```

---

## P — rpg-api-protos: replace authoring, grow the atlas

**Branch:** `feat/256-dungeon-authoring-v2` · **Base:** `main` · **Evidence:** `buf lint`, `buf format -d` clean, `buf breaking` (authoring is a replace — expect and accept the break on `authoring.v1alpha1`; session is additive), generate compiles. No hand-written tests (`no-handwritten-tests-in-protos`).

**Files**
- Rewrite `dnd5e/api/authoring/v1alpha1/service.proto`; delete `dnd5e/api/authoring/v1alpha1/testdata/`.
- Modify `dnd5e/api/session/v1alpha1/types.proto` (add `AtlasRegion`, `Lighting`).
- Doc comment on `AtlasRegion.archetype`: the law — an archetype never decides mechanics (start, blocking, sight, intensity); only what the assets show and play.
- Modify `dnd5e/api/session/v1alpha1/service.proto` (`GetAtlasResponse.regions = 9`).

**Contract**
```proto
// authoring/v1alpha1/service.proto
service AuthoringService {
  rpc PutDungeon(PutDungeonRequest) returns (PutDungeonResponse);
  rpc GetDungeon(GetDungeonRequest) returns (GetDungeonResponse);
}
message PutDungeonRequest  { string key = 1; string yaml = 2; bool validate_only = 3; }
message PutDungeonResponse {
  repeated FieldError errors = 1;                       // empty ⇒ compiled; atlas set
  dnd5e.api.session.v1alpha1.GetAtlasResponse atlas = 2;
}
message FieldError { string path = 1; string message = 2; }
message GetDungeonRequest  { string key = 1; }
message GetDungeonResponse { string yaml = 1; }

// session/v1alpha1/types.proto
message Lighting { double intensity = 1; }    // 0..1, the dimmer on top of the archetype
message AtlasRegion {
  string id = 1; string name = 2;
  repeated Position cells = 3;                // absolute axial, sorted, same frame as GetAtlasResponse.cells
  string archetype = 4;                       // presentation ref the assets resolve (lighting kind, audio); never mechanics
  Lighting lighting = 5;
}
// session/v1alpha1/service.proto — GetAtlasResponse
repeated AtlasRegion regions = 9;             // every floor cell appears in exactly one region
```
Doc comments carry the design's sentences: regions replace rooms; walls are declared; lighting is a world fact, not a render hint.

**Done when:** tag minted on merge; `go get` of the generated module resolves from `main`. Post the tag on rpg-project#256.

---

## T1 — rpg-toolkit `rulebooks/dnd5e/encounter`: regions replace rooms

**Branch:** `feat/256-regions-replace-rooms` · **Module:** `rulebooks/dnd5e/encounter` (breaking `!` bump) · **Issue:** file `rpg-toolkit` issue "encounter: RegionInput replaces RoomInput (rpg-project#256)" as sub-issue of #256.

**Files**
- Modify `field.go`: delete `RoomInput`, `ConnectionInput`; add `RegionInput`; `FieldInput{Canvas, Regions, Doors}`.
- Modify `encounter.go` (`compileCanvas` ~902–970, `Setup` validation ~1277–1356): build the floor mask from region cells; refuse overlap, empty region, door edge off-floor, boundary endpoints non-adjacent.
- Modify `region.go`: `regionAt` reads the per-cell owner map built at compile; `Region(id)` returns `{ID, Name, Cells, Lighting}`.
- Modify `orientation.go`: delete `absoluteOf` (no origin to add); `HexCellAt` stays the one conversion.
- Modify `data.go`: `FieldData{Canvas, Regions []RegionData, Doors}`; `RegionData{ID, Name, Cells []Position, Archetype, Lighting}`; rename the JSON keys (fail-loud load, per the 2026-08-17 ruling — old blobs land nowhere).
- Modify `atlas.go`: `Atlas.Regions []AtlasRegion{ID, Name, Cells, Archetype, Lighting}`, cells sorted with the same comparator as `Atlas.Cells`.
- Modify `doc.go` world laws: W2 "regions never overlap" replaces "rooms never overlap, Origins integral"; W3 "doorways kiss" becomes "a door edge joins two adjacent floor cells".
- Delete the room-chain tests (`canvas_test.go` rooms cases); add `region_test.go`, `atlas_regions_test.go`.
- ADR: `docs/adr/0044-regions-replace-rooms.md` + one line in `DECISIONS.md`: *a region is a named set of absolute cells carrying per-area world facts (lighting now); rooms, origins and connections were the chain's vocabulary and are gone.*

**Interfaces**
```go
type Lighting struct{ Intensity float64 }       // [0,1]; carried unread by the composition

type RegionInput struct {
    ID       string
    Name     string
    Cells     []spatial.Position   // authored offset [col,row], absolute
    Archetype string               // REQUIRED non-empty; presentation ref, carried unread
    Lighting  *Lighting            // REQUIRED (nil refused — #1033)
}
type FieldInput struct {
    Canvas  CanvasInput            // Void, Orientation — unchanged
    Regions []RegionInput
    Doors   []DoorInput            // unchanged: one state over N DoorEdge, absolute
    Walls   []spatial.Boundary     // authored edges, absolute, offset frame — moved up from RoomInput.Boundaries
}
type AtlasRegion struct {
    ID string; Name string; Cells []spatial.Position; Archetype string; Lighting Lighting
}
```
Sentinels in `errors.go`: `ErrRegionEmpty`, `ErrRegionOverlap`, `ErrRegionArchetypeMissing`, `ErrRegionLightingMissing`, `ErrEdgeNotAdjacent`, `ErrEdgeOffFloor`, `ErrDoorEdgeOffFloor`.

**Tests that gate**
- `TestSetup_RegionsMakeTheFloor`: three regions → `Atlas().Cells` equals their union, sorted; `RegionAt(cell)` answers the owner for every cell; void cell → `false`.
- `TestSetup_RefusesOverlap` / `_RefusesEmptyRegion` / `_RefusesMissingLighting`: each sentinel by name at `Setup` AND at `LoadEncounter`.
- `TestAtlas_RegionsCarryLighting`: archetype + intensity round-trip through `ToData` → `Load` → `Atlas`.
- `TestEdges_MustBeAdjacentUnderOrientation`: same `[col,row]` pair adjacent under pointy, not under flat → `ErrEdgeNotAdjacent` only for flat (the discriminator — one formula per orientation, not a swapped pair).
- Existing combat fixtures (`tomb_test.go` in `dungeonspec`, any `encounter` fixture that built rooms) rebuilt as regions in the same PR — `go test ./...` green is the gate, not a subset.
- Mutation pass on `region.go` / `compileCanvas` per `mutation-testing-catches-overclaims`.

**Command:** `cd rulebooks/dnd5e/encounter && go test -race ./... && golangci-lint run`.

---

## T2 — rpg-toolkit `rulebooks/dnd5e/encounter/dungeonspec`: version 2

**Branch:** `feat/256-dungeonspec-v2` (stacked on T1 or same PR if T1 is small enough to review together — implementer's call, Kirk reviews once either way).

**Files**
- Rewrite `spec.go`: `Spec{Version, Key, Name, Orientation, Void, Regions []RegionSpec, Start *[2]int, Walls []EdgeSpec, Doors []DoorSpec, Place []PlaceSpec}`.
- Rewrite `decode.go` (still strict: `KnownFields(true)`, one document, `version: 2` only).
- Rewrite `validate.go`: path-addressed errors — `type FieldError struct{ Path, Message string }`, `Validate(spec) []FieldError` — every refusal names the YAML path (`regions[1].cells[0][3]`, `walls[3]`, `doors[0].edges[1]`, `place[7].blocks_los`, `start`).
- Rewrite `compile.go`: delete `anchorsOf`, `seamWall`, `doorwayRow`, `seamProbeSpan`; `Compile(spec) (Compiled, error)` builds `encounter.FieldInput` directly — regions verbatim, walls `Boundary{From, To, BlocksMovement: true, BlocksLineOfSight: true}`, doors `DoorInput{ID: key+"/"+door.ID, Edges, State}`; `PartyStart` = seats fanned from `start` by spatial distance as today; `Monsters` unchanged.
- Replace `reference-tomb` fixture in `testdata/` with the version-2 authoring; add `testdata/tomb-second-skeleton.yaml` (the rpg-project#254 variant) so slice 2's fixture lives here once.
- Golden: `testdata/reference-tomb.atlas.json` captured from the **v1** compile BEFORE v1 is deleted (first commit of the PR), then the v2 fixture must produce it (minus the new `regions` field, compared separately).

**YAML grammar** — exactly design §2:
```go
type RegionSpec struct {
    ID string `yaml:"id"`; Name string `yaml:"name"`
    Archetype string `yaml:"archetype"`               // REQUIRED non-empty
    Lighting *LightingSpec `yaml:"lighting"`          // REQUIRED
    Cells [][][2]int `yaml:"cells"`                   // rows of [col,row]; flattened on compile
}
type LightingSpec struct{ Intensity *float64 `yaml:"intensity"` }   // REQUIRED, [0,1]
type EdgeSpec [2][2]int                               // [[col,row],[col,row]]
type DoorSpec struct {
    ID string `yaml:"id"`; Edges []EdgeSpec `yaml:"edges"`
    Locked *LockSpec `yaml:"locked,omitempty"`; Closed bool `yaml:"closed,omitempty"`
}
type PlaceSpec struct { /* unchanged from v1; At is now ABSOLUTE */ }
```

**Tests that gate**
- `TestGolden_ReferenceTombV2MatchesV1Atlas` — the forcing case.
- `TestValidate_PathsNameTheThing` — table: overlap, wall non-adjacent, wall off-floor, door edge in `walls` too, prop without `blocks_los`, `start` on void, `intensity` 1.2, missing `archetype`, unknown key → each expected `Path`.
- `TestDecode_RefusesVersion1` — the deleted dialect is refused by name, not parsed hopefully.
- `TestCompile_DoorInsideARegionIsLegal`.
- `TestSecondSkeletonFixtureCompiles` — rpg-project#254's fixture, so slice 2 keeps running.

**Command:** `cd rulebooks/dnd5e/encounter && go test -race ./dungeonspec/... && golangci-lint run`. Tag on merge; post the tag on #256.

---

## T3 — rpg-toolkit `rulebooks/dnd5e/session`: atlas carries regions

**Branch:** `feat/256-atlas-regions` · **Module:** `rulebooks/dnd5e/session` (additive).

**Files**
- Modify `types.go`: `Atlas.Regions []AtlasRegion` (mirror of encounter's), JSON `regions`.
- Modify the atlas projection (where `Atlas` is built from `encounter.Atlas()`): copy regions through; cells already absolute axial — no conversion (the symmetric-bug lesson: there is exactly one place cells become axial, and it is not here).
- Test `atlas_regions_test.go`: `GetAtlas` on a started tomb returns three regions whose cells union to `Atlas.Cells` and whose archetypes and intensities are the authored ones; pixel-formula check on one named cell of an L-shaped region under both layouts.

**Command:** `cd rulebooks/dnd5e/session && go test -race ./... && golangci-lint run`. Pin T1/T2 by pseudo-version until their tags mint.

---

## A — rpg-api: the content registry, the RPCs, the key

**Branch:** `feat/806-dungeon-registry` off `dev` · **Issue:** rpg-api#806 (rewrite its body to this scope; it is the sub-issue of #256).

**Files**
- Create `internal/dungeons/registry.go`:
  ```go
  type Entry struct{ Key, Name string; YAML []byte; Compiled dungeonspec.Compiled; Atlas session.Atlas }
  type Registry interface {
      List(ctx) ([]Summary, error)                 // {Key, Name}
      Get(ctx, key) (Entry, error)                 // ErrNotFound
      Put(ctx, key string, yaml []byte, validateOnly bool) (PutResult, error)  // {Errors []FieldError, Atlas}
  }
  func NewFileRegistry(dir string, authoring bool) (Registry, error)  // loads every *.yaml at boot; a file that does not compile FAILS construction
  ```
  Put: decode → validate → compile → (if !validateOnly) write `dir/<key>.yaml` atomically (temp + rename) → swap the entry under a mutex. Puts serialised per key. `key` must equal the YAML's `key` (`InvalidArgument`).
- Create `content/reference-tomb.yaml` (the v2 file from T2's testdata); delete the `go:embed` in `internal/sessionworld/sessionworld.go`; `sessionworld` becomes "compile bytes → world" only — delete the throwaway-encounter projection (`sessionworld.go:209-281`) and `bench_test.go`'s borrowed-projection bench.
- Modify `cmd/server/server.go:253`: register `AuthoringService` when `RPG_AUTHORING_ENABLED=1` (requires `RPG_CONTENT_DIR`, fail at construction if unset); registry constructed unconditionally (the tomb must load with authoring off).
- Create `internal/handlers/dnd5e/authoring/v1alpha1/{handler,put_dungeon,get_dungeon}.go`; `internal/orchestrators/authoring/`.
- Modify `internal/orchestrators/lobby/start_encounter_session_stack.go:26-35,140-175`: `dungeon_key` → `registry.Get`; empty key → `reference-tomb`; unknown → `codes.NotFound`. Delete the "Unused today" comment.
- Modify `internal/handlers/dnd5e/lobby/v1alpha1/`: implement `ListDungeons` from `registry.List` (ungated); fix the stale `effectiveKey` comment in `start_encounter.go:33-38`.
- Modify `internal/handlers/dnd5e/session/v1alpha1/convert.go` (~459): `atlasToProto` copies `Regions`.
- Docs: `docs/architecture/components/lobby-service.md` (ListDungeons source), new `docs/architecture/components/authoring-service.md`; delete the pre-rip-out sections of `docs/status.md` that describe `resolveDungeonSpec`/`dungeonSpecs`.
- `docker-compose` (rpg-deployment, separate PR): `RPG_CONTENT_DIR=/content`, `./content` volume, `RPG_AUTHORING_ENABLED=1` on the dev box — the flip that lights the Home button.

**Tests that gate**
- `registry_test.go`: boot refuses a non-compiling file (error names the file and the path); `Put(validateOnly)` never writes; `Put` writes then `Get` returns the bytes unchanged; concurrent `Put`s on one key serialise (race detector); `List` after `Put` includes the new key.
- Handler tests: `StartEncounter{dungeon_key:"nope"}` → `NotFound`; empty key → tomb; `GetAtlas` on a started custom dungeon returns its cells and regions.
- Existing session handler suite green.

**Command:** `make test && make lint` (per rpg-api AGENTS.md); `scripts/dev-env.sh pin` for T1/T2/T3 pseudo-versions until tags exist. Image build for Kirk's walk per `local-dev-loop`.

---

## W — rpg-dnd5e-web: `/author` on the atlas

**Branch:** `feat/256-author-v2` off `dev` · **Issue:** file `rpg-dnd5e-web` issue "author: emit dungeonspec v2, preview on the atlas (rpg-project#256)" as sub-issue of #256.

**Files**
- Regenerate protos (`npm run protos` or the repo's equivalent) against P's tag.
- `src/author/dungeonYaml.ts`: replace the `DungeonDoc` model with the v2 shape (`regions[].cells` rows, `walls`, `doors`, `place` absolute, `archetype`, `lighting.intensity`); emitter writes cells sorted, one row per line (the diff convention). Delete `stripToV1Subset`, the `spec:` handling, `wallLines`.
- Delete: `src/author/specCompat.ts`, `capabilityProbe.ts`, `hexLayout.ts` (odd-q), `boardGeometry.ts`, `creation/straightWallGeometry.ts`, `creation/hexCorner.ts`, `specimens/`, `CONTRACT.md`, `TARGET-YAML.md`, `preview3d/`'s own wall/floor renderers.
- `src/author/creation/CreationBoard.tsx`: region brush, wall tool (edge click), door tool (edge click/drag), start, place — all in **axial** via `src/components/session/positionBridge.ts`; offset `[col,row]` is produced only at emit time by one function `toOffset(orientation, axial)` whose inverse is pinned by a pixel-formula test against `hexMath.ts`.
- `src/components/session/atlasToScene3D.ts`: extend `buildScene3D` to take `layout` (it assumes pointy today) and `props` (returns `Scene3D.props` for `PropModel`); update `SessionCanvas` to pass both — the game and the builder call the same function with the same arguments.
- `src/author/preview3d/DungeonPreview3D.tsx`: render `PutDungeonResponse.atlas` through the extended `buildScene3D` + `SyntyHexFloor` + `AtlasWalls` + `PropModel`; camera wrapper so `SessionScene`'s `useCameraControls` is not required (orbit + the game's tactical rig).
- `src/author/usePutDungeonPreview.ts`, `useSaveDungeon.ts`: new proto; `useAuthoringGate.ts`: probe = `GetDungeon("reference-tomb")` succeeds ⇒ authoring on (fix the stale env-var message).
- `src/author/RegionPanel.tsx`: `id`, `name`, `archetype` (select from the assets' profile list — a static list in W, the catalog is Not now), intensity slider (0–1).
- `src/api/useListDungeons.ts` / `DungeonPicker.tsx`: unchanged shape, now live.
- `Save & Play`: after `PutDungeon` succeeds, `StartEncounter{lobby_id, dungeon_key}` via `useStartLobbyEncounter.ts` and navigate to the game route.
- Error display: `FieldError.path` → highlight the cell/edge/placement it names; `Save` disabled while errors exist.

**Tests that gate**
- `dungeonYaml.test.ts`: emit → parse round-trip is byte-identical; cells sorted one row per line.
- `offsetBridge.test.ts`: the discriminator — one named cell of an L-shaped region, both orientations, world position from the builder equals `cubeToWorld` of the atlas cell the server returns for it (fixture captured from T2's golden).
- `DungeonPreview3D.test.tsx`: given a fixture atlas, the scene has N floor tiles, M wall runs, one door gap — same numbers `SessionCanvas` produces for the same atlas.
- `CreationBoard.test.tsx`: wall tool on a non-adjacent pair is a no-op; region brush never paints a cell into two regions.
- Visual: `node tools/browser/screenshot.mjs` of `/author` with the tomb loaded, and of the game route on the same dungeon — attached to the PR for Kirk's walk.

**Command:** `npm test && npm run lint && npm run build` (per web AGENTS.md); run against the branch api on `:50051`, web on `:3003`.

---

## Kirk's walk (once, before any merge)

Branch api image on `:50051` (content dir seeded with the v2 tomb), branch web on `:3003`, fresh lobby.

1. Home shows the Dungeon Builder button (probe via `GetDungeon`).
2. `/author` → New → paint two regions, different lighting → wall between them → a door in the wall, locked DC 12 → start cell → a pillar (`blocks_los: true`) and a skeleton → preview shows the same walls the game will.
3. Save & Play → the 3D route opens on it; walk to the door; the pillar blocks sight; the dark region reads darker.
4. Lobby → picker lists `reference-tomb` and the new key; start the tomb → identical to today.
5. `/author` → Open the new key → canvas matches; YAML pane matches what was saved.

A wall in the walk is a design signal: back to `design.md`, not a patch.

## Merge (bottom-up, one sitting)

T1 → tag → T2 → tag → T3 → tag → A (re-pin real tags, merge to `dev`) → W (`dev`) → rpg-deployment compose flip. Close rpg-api#806, toolkit#1139 (seam removed), rpg-project#131 (picker live), toolkit#1113 (lighting landed on regions). Update #169's Current reality and Next proof.

## Not in this plan

Lighting `ref`, audio, facing/height/offsets, region nesting, square grids, `DeleteDungeon`, prop catalog authoring, walkthrough, DM mode, hosted deployment — all design §8.
