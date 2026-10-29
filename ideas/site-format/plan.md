---
name: Site format — implementation plan
status: PLAN derived 2026-10-29 from design.md R1–R7 and README.md; checked against the inspected baseline below; gaps resolved under the design's authority are listed in "Gaps resolved"
authority: design.md (R1–R7, Law), README.md (ownership table and refusals), story rpg-project#725
planning guide: ../../.agents/skills/design/planning.md
---

# Site format — implementation plan

## Goal and scope

The single-room dialect, the studio and the shipped content move to the shape
the design states: the floor is the union of authored regions, each region's
cells are row runs, the canvas is unbounded, authored regions own presentation
and wall partitions own mechanics. The compiled cell set of every existing
content file is unchanged, proven by the goldens. The castle on :3030 opens in
the studio with nine named, lit regions and plays as before.

**Non-goals** (R7, each its own story under R1's yardstick): the compiled-field
cache, region-graph pathfinding, per-region render batching. Also not here:
sound on a region (O1), placed rooms (O3), runs on the wire (O5), the v2
multi-room dialect (untouched), protos (untouched).

**Repositories and order.** Develop outside-in, merge inside-out.

| slice | repo / module | base | PR shape |
|---|---|---|---|
| S1 | rpg-toolkit `rulebooks/dnd5e/encounter` | main | one PR, tag `encounter/v0.150.0` expected |
| S2 | rpg-toolkit `rulebooks/dnd5e/session` | main | one PR: re-pin encounter tag + convert its fixture |
| S3 | rpg-api | dev | one PR: pin both tags, convert 14 files, recapture goldens, fix 3 tests |
| S4 | rpg-dnd5e-web | dev | two stacked PRs: codec + draft model, then canvas + brush |

S4 develops against the contract in T1 from day one (fixtures); its preview
needs an api built from S3's branch. Merge order S1 → S2 → S3 → S4.

## Finish line

- Four slice sub-issues under #725 closed by their "done means".
- S1 tagged; S2 and S3 ready on tags, no pseudo-versions; S4 on dev.
- Castle stack on dev/dev serves the converted castle; the operator walked the
  checklist on S4's sub-issue (regions named and lit in the studio, a brushed
  reassignment round-trips, Save & Play shows the current region's authored
  name, a clang crosses the yard-gatehouse edge with zero door hops).
- R1 file budget measured: the 30k generated site's geometry under 100 KB,
  recorded on S1's sub-issue with the compile time beside it.
- rpg-dnd5e-web#1284 closed by S4.

## Requirement labels

| label | requirement | source |
|---|---|---|
| F1 | The dialect carries `room.room.regions`, each `{id, name, lighting, cells}`; `walkableHexes` does not exist; the floor is the union. | Law Floor, Region |
| F2 | `cells` is a list of rows `{r, q: [[qStart, qEnd], ...]}`, canonical: rows ascending by r, spans ascending, inclusive, non-overlapping, non-touching. A non-canonical list is refused. | R2 |
| F3 | A cell in two regions is refused with both paths. | Law Floor |
| F4 | `implicitRegionId` names the region unassigned paint lands in. A region may be listed with no cells and the compiler skips it. | Law Floor; gap G3 |
| F5 | `room.workspace` does not exist; `room.coordinateFrame` stays. No decode bound depends on a workspace. | R3 |
| F6 | The compiler emits one `RegionInput` per authored region with its own lighting; partitions are derived over the whole floor from walls and doors at construction and at every load, never stored and never on the wire; door footing is subtracted from the region that owns it. | R4, R8, Law Derivation |
| F7 | Hearing hops, seat derivation, arrival-cell search and the intel flip (`sweepPresence`) read partitions. The atlas, `RegionAt`, member and outcome regions, `Seat.Region`, `MonsterPlacement.Region` and reveal grouping read authored regions. An authored edge adds no hop. | R4 |
| F8 | Every existing content file converts once; each `.compiled.json` cell set, props, walls, doors, exits and start are unchanged; the union of `regions[].cells` is unchanged while the regions themselves regroup from partitions to authored regions; `monsters[].region` and `party_start[].region` carry authored ids. | R6, Law Equality |
| F9 | Root `version: 5`; versions 3 and 4 are refused by name. | R6; gap G6 |
| F10 | The studio paints into the active region, the implicit region by default; brushing with another region active reassigns; a region has a name and a lighting intensity; the map label is the name at the region's centroid; notes stay. | R5 |
| F11 | The studio draws its grid from the content bounding box plus a margin; zoom 1 fits the content; the view is not stored; a cell anywhere is paintable. | R3 |
| F12 | Server refusals under `room.room.regions[i]` jump to that region; under `.cells[j]` to the region as well. | README refusals (errors point at the thing) |
| F13 | A generated 30,000-hex site's YAML geometry is under 100 KB. | R1 |
| K1 | Fog per seat, cells-revealed beats, door footing, concealment, exits and the QA castle routes keep working. | must keep working |

## Constraints

- Hex axial `{q, r}` pointy-top; offset `[x, y]` refused in authored cells (D1a).
- Toolkit: one nearest-`go.mod` module per PR; gates `go test -race ./...` and `golangci-lint run ./...` in the module; CI fails on a dirty tree after `go mod tidy` and `go fmt`.
- rpg-api: `make test`; `make release-pin-check`; pins by `GOWORK=off go get <module>@<tag>` then `go mod tidy`; a ready PR carries no pseudo-version.
- rpg-dnd5e-web: `npm run ci-check` once at the PR boundary; vitest; no `--no-verify`.
- Server stores YAML verbatim; no layer rewrites a FieldError path.
- No session link, model name or co-author trailer; team signature only.

## Inspected baseline

| repo | revision | note |
|---|---|---|
| rpg-toolkit | origin/main 78a28eff = encounter v0.149.0 | session pins encounter v0.144.0 in its go.mod |
| rpg-api | origin/dev d4a85aea | pins per its go.mod (S3 reads the exact lines) |
| rpg-api-protos | origin/main 01b8235 | untouched |
| rpg-dnd5e-web | origin/dev 96992073 | studio code read at a9889339; the one commit between is a knowledge ingest with no studio diff |

Probe reports (case file): the story's design PR #726 thread.

## Gaps resolved under the design's authority

| gap | resolution | authority |
|---|---|---|
| G1 cells grammar | rows list `{r, q: [[a, b], ...]}` rather than a mapping keyed by r, so both YAML codecs read integer keys the same way and a repaint diffs per row | R2 |
| G2 lighting | required on every region in the dialect, `lighting: {intensity: 0..1}` (the encounter's word, as the v2 dialect spells it); the studio writes 1 by default and the implicit region is authored like any other | R4, "zero values tell the truth" |
| G3 empty regions | `implicitRegionId` stays required and names a listed region or none; any region may carry `cells: []` (declared, unpainted) and contributes no `RegionInput`; a site whose regions are all empty is refused at `room.room.regions` with "no region owns a cell" | Law Floor |
| G4 region presentation | the studio's look and construction live under `scene`, keyed by region id, as props already split `scene.items` from `room.propDeclarations`; `scene.authoringRegions` retires; `scene.mapLabels` keeps only notes (labels linked to no region); a region's label position is its cells' centroid, computed | R2, R5, "nothing derived is written" |
| G5 partitions | derived over the union floor at field construction and at every load from the persisted authored regions, walls and doors; never stored in `FieldData` (its charter holds authored truth only); ids internal (`p<n>`), never on the wire; the `<id>/space/<q>,<r>` naming retires; the castle's full load measures 94 ms today and the compiled-field cache sibling removes the per-verb repeat | R4, R8, ADR-0044 |
| G6 versions | root `version: 5`; the decoder refuses 3 and 4 with a sentence naming #725; the studio's `carriesV4Keys` retires and always writes 5; the draft envelope bumps to 6 and refuses 5 by name (precedent: 3 and 4) | R6 |
| G7 which division a reader uses | a reader that decides an outcome uses the partition: hearing (`links`, `hopsFrom`, `hearersOf`), seat derivation (`deriveSingleRoomSeats` confined to the party start's partition), `arrivalCell`, the intel flip (`sweepPresence`), door footing (`joinOpaqueFooting`); a reader that reports a place uses the authored region: atlas, `RegionAt`, member/outcome region, `Seat.Region`, `MonsterPlacement.Region`, reveal grouping, `MembersIn`; the builder lists every reader with its side in the PR body | R4 |
| G8 converter | a node-level rewrite that preserves comments and key order outside the changed keys; automatic authoring regions (witness-bound) are refused by the converter and converted by hand, listing each in the PR | R6 |

## Sequence

```
S1 encounter (T1–T7) ──tag──▶ S2 session (T8) ──tag──▶ S3 api (T9–T10) ──branch api──▶ S4 web walk
S4 web T11–T12 develop in parallel on fixtures from T1's contract
merge: S1 → S2 → S3 → S4; castle stack rebuilt on dev/dev after S3; operator walks on S4
```

Development dependency: S4 T11 needs only the YAML contract (T1) and the
converted castle (T6). S4's preview needs S3's api branch. Merge dependency: S2
needs S1's tag; S3 needs both tags; S4 needs S3 on dev for the walk.

---

## S1 — the dialect and the two divisions (rpg-toolkit, encounter module)

### Task 1: Dialect v5 types and decode

**Delivers:** F1, F2, F3, F4, F5, F9.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter/dungeonspec`.
**Prerequisites:** none.
**Files:** modify `single_room.go` (`RoomSource`: drop `Workspace yaml.Node`; `RoomGameplaySource`: replace `WalkableHexes []RoomCell` with `Regions []RoomRegion`), `single_room_decode.go` (`gameplayShape`, `gameplayValues`, root version check), `single_room_lowering.go` (`readRoom`: drop the workspace read and `WorkspaceHexRadius`/`WorkspaceKnown`), `single_room_concealments.go` and `single_room_site.go` (floor set from regions), `unknown_key.go` only if the new types need a form entry; new `single_room_cells.go` (run expansion); tests `single_room_decode_test.go`, `single_room_lowering_test.go`, new `single_room_cells_test.go`.
**Interfaces:**

```go
type RoomRegion struct {
    ID       string            `yaml:"id" json:"id"`
    Name     string            `yaml:"name" json:"name"`
    Lighting RoomLighting      `yaml:"lighting" json:"lighting"`
    Cells    []RoomCellRow     `yaml:"cells" json:"cells"`
}
type RoomLighting struct { Intensity float64 `yaml:"intensity" json:"intensity"` }
type RoomCellRow struct {
    R int      `yaml:"r" json:"r"`
    Q [][2]int `yaml:"q" json:"q"` // inclusive [qStart, qEnd]
}
// ExpandRows returns the cells of a region in canonical axial order and the
// first FieldError when the rows are not canonical.
func ExpandRows(rows []RoomCellRow, path string, add errSink) []RoomCell
```

Authored YAML:

```yaml
version: 5
room:
  room:
    implicitRegionId: room-1-region
    regions:
      - id: region-gatehouse
        name: Gatehouse
        lighting: { intensity: 0.3 }
        cells:
          - r: -6
            q: [[-2, 2]]
          - r: -5
            q: [[-3, 3], [6, 9]]
```

**Behavior:** `regions` required, non-empty list (`room.room.regions` "is required" / "must not be null" / "must not be empty"); at least one region owns a cell ("no region owns a cell"). Per region: `id` non-empty string, unique (`errDuplicateID` at the second); `name` string (may be empty); `lighting.intensity` required number in [0,1] ("must be between 0 and 1"); `cells` required list, may be empty. Per row: `r` integer; `q` non-empty list of two-integer pairs ("must be a pair of integers"); `qStart <= qEnd` ("must not be descending"); spans ascending and non-touching ("spans must be ascending and merged", at `room.room.regions[i].cells[j].q[k]`); rows ascending by r with no repeat ("rows must be ascending by r", at `.cells[j]`). A cell owned by two regions: "already belongs to region %q" at `room.room.regions[i].cells[j].q[k]` (the later one). `implicitRegionId` required non-empty; when a region carries that id it is the implicit one; otherwise no error. An empty region yields no `RegionInput` at compile. Root `version` must be 5: 3 and 4 refused at `version` with "versions 3 and 4 are retired; regions are the floor (rpg-project#725)". `room.workspace` present: refused by `KnownFields` as `room.workspace` "unknown key" (verify the translation in `unknown_key.go` yields that path). Concealment cells and site `at:` cells must be floor cells (union of regions), same messages as today.
**Tests:**

- `TestExpandRows_CanonicalCastleRow`: `{r: -6, q: [[-2, 2]]}` → five cells q=-2..2.
- `TestExpandRows_RefusesTouchingSpans`: `[[1, 3], [4, 6]]` → one error "spans must be ascending and merged" at `...q[1]`.
- `TestExpandRows_RefusesDescendingSpan`, `_RefusesUnsortedRows`, `_RefusesRepeatedRow`, `_RefusesNonIntegerPair`.
- `TestDecode_RegionsRequired`, `TestDecode_RegionLightingRequiredAndBounded` (1.5 refused), `TestDecode_CellInTwoRegions` (path of the second), `TestDecode_ImplicitRegionIdMayBeUnlisted`, `TestDecode_EmptyRegionIsAccepted`, `TestDecode_AllRegionsEmptyRefused`, `TestDecode_Version4Refused` (message names #725), `TestDecode_WorkspaceRefused`.
- `TestCompile_EmptyRegionContributesNothing` (compile path: an empty region is absent from the atlas).
- Existing lowering tests for "outside the workspace floor" and radius refusals: delete, with a sentence in the PR body (R3 removes the bound).

- [ ] Write the tests above; see them fail on missing types.
- [ ] Implement types, expansion, shape and value walks.
- [ ] `go test -race ./dungeonspec/ -run 'TestExpandRows|TestDecode'` green.
- [ ] Update the `dungeonspec` package doc and the `RoomSource` type comment (workspace gone; regions are the floor).

**Verification:** `cd rulebooks/dnd5e/encounter && go test -race ./dungeonspec/ -run 'TestExpandRows|TestDecode'`; expected: all named tests pass; `golangci-lint run ./dungeonspec/` clean.
**Completion evidence:** test names and pass output in the PR; the YAML example above decoding in a test.

### Task 2: Compile one region per authored region; partitions over the floor

**Delivers:** F6, F7 (compiler half), K1.
**Owner:** rpg-toolkit encounter (`dungeonspec/single_room_compile.go`, `single_room_regions.go`; `encounter/compilefield.go`, `partition_region.go`, `data.go`, `field.go`, `noise.go`, `reserve.go`, `flip.go`).
**Prerequisites:** T1.
**Files:** modify `single_room_compile.go` (build `[]encounter.RegionInput` from `spec.Room.Gameplay.Regions` with `axialOffset` per expanded cell, `Archetype: "crypt"`, `Lighting: &encounter.Lighting{Intensity: r.Lighting.Intensity}`; seat derivation stays confined to the party start's partition through `PartitionAt`, while `Seat.Region` and `MonsterPlacement.Region` take `RegionAt`); rewrite `single_room_regions.go` (`singleRoomRegions` partitions the union of all regions, no longer `field.Regions[0]`, and no longer replaces `field.Regions`; the footing it returns is subtracted from the owning region's cells and appended to scenery as today; a region emptied by footing is refused at `room.room.regions[i].cells` with "every cell is wall footing"); encounter: new `partitions.go` (`PartitionFloor(in PartitionFloorInput) (PartitionFloorOutput, error)` over every region's cells with the same passability as `PartitionRegion`; `Field` gains `partitionOf map[Position]PartitionID` and `partitionCells map[PartitionID][]Position`, `PartitionAt(cell) PartitionID`, both filled by `compileField` at construction and again by the load path from the persisted regions, walls and doors; `FieldData` unchanged per R8); `noise.go` (`links`, `hopsFrom`, `hearersOf` key on `PartitionAt`); `reserve.go` `arrivalCell` (search `partitionCells`); `flip.go` `sweepPresence` (compare `PartitionAt`); `compilefield.go` (`maxFieldCells` unchanged; refusals unchanged). Keep `PartitionRegion` for its other callers or delete if `singleRoomRegions` was its only caller (inspect; say which in the PR).
**Interfaces:** `PartitionID` is `string`, values `p0`, `p1`, … in canonical axial order of each component's first cell; never serialised to protos or session types. `RegionAt`, `Member.Region`, `MemberOutcome.Region`, `MembersIn`, `AtlasRegion` keep their types and now carry authored ids. `Seat.Region` and `MonsterPlacement.Region` (dungeonspec outputs) carry the authored id.
**Behavior:** a sound in the courtyard with a listener in the gatehouse, no wall between: zero hops. The same with a shut door between two partitions: one hop plus the closed-door hop as today. An arrival cell that is taken searches the same partition, not the authored region. The atlas lists authored regions with their own lighting and names, sorted by id.
**Tests:**

- `TestPartitionFloor_TwoRegionsOneOpenSpace` (encounter): two RegionInputs sharing an open edge, no walls → one partition; `PartitionAt` equal for a cell of each.
- `TestHearing_AuthoredEdgeAddsNoHop` (`wall_door_hearing_test.go` style): two regions, no wall, voice DC 5, listener passive 10 → heard; with a shut door between them → not heard at the same DC (one hop +5 as today).
- `TestAtlas_ListsAuthoredRegionsWithTheirLighting`: two regions intensity 1 and 0.3 → two `AtlasRegion` entries with those intensities and names.
- `TestRegionAt_IsTheAuthoredRegion`: a cell painted into `region-gatehouse` answers that id even when its partition spans the yard.
- `TestFieldLoad_DerivesTheSamePartitions`: compile → `EncounterData` → load → `PartitionAt` identical for every cell, with `FieldData` carrying no partition key.
- `TestCompile_FootingLeavesItsRegion`: a region whose cells include an ambiguous-footing cell loses that cell to scenery; a region made of footing only is refused with the named message.
- Existing `door_threshold_region_test.go`, `wall_door_hearing_test.go`, castletest gate/cells tests: keep passing, with `RegionAt` comparisons now meaning authored regions; where a test compared partitions through `RegionAt` (gatehouse vs courtyard "different region"), rewrite it against `PartitionAt` or against hearing hops and say so in the PR.

- [ ] Write the tests; fail on missing `PartitionFloor`/`PartitionAt`.
- [ ] Implement; run `go test -race ./...` for the module.
- [ ] Record the reader-to-division table (G7) in the PR body.

**Verification:** `cd rulebooks/dnd5e/encounter && go test -race ./... && golangci-lint run ./...`; castletest suite under `-race` within its 10 min CI budget (baseline about 198 s).
**Completion evidence:** the G7 table, test output, castle suite time, and the field load time on the castle with partition derivation beside the time without it (a benchmark on `compileField` from `FieldData`), posted on S1's sub-issue as R8's measurement.

### Task 3: The converter

**Delivers:** F8 (tooling), G8.
**Owner:** rpg-toolkit encounter `dungeonspec`.
**Prerequisites:** T1.
**Files:** new `dungeonspec/convert_v4.go` (`ConvertV4(src []byte) ([]byte, []ConvertNote, error)`), new `dungeonspec/convert_v4_test.go`, a test-only entry `TestConvertV4Testdata` gated by `CONTENT_CONVERT=write` that rewrites every `testdata/**/*.yaml` carrying `walkableHexes` in place.
**Interfaces:** node-level rewrite on `yaml.Node`: set root `version` to 5; delete `room.workspace`; build `room.room.regions` from `scene.authoringRegions` (explicit only: id, name from the linked `mapLabels` text, lighting from `lighting.background` or 1, cells as canonical rows) plus the implicit region (`implicitRegionId`, name = `room.name`, intensity 1) owning every walkable cell in no explicit region; delete `room.room.walkableHexes`; delete `scene.authoringRegions`; move `wallFinish` per region to `scene.regionPresentation[<id>].wallFinish`; keep in `scene.mapLabels` only labels with no `labelId` link; an automatic region or a label linked to one → `ConvertNote` and the file is left unconverted with error. Comments and other keys preserved; the walkable header comment block is edited by hand per file where it describes `walkableHexes`.
**Behavior:** converted file decodes under T1; its compiled floor equals the original's (T5 proves).
**Tests:** `TestConvertV4_CastleRoundTripsTheFloor` (convert `world-builder-v4-castle.yaml` in memory; decode; the union of region cells equals the original walkable set; nine named regions plus the implicit one; lighting 1 everywhere); `TestConvertV4_RefusesAutomaticRegion`; `TestConvertV4_PreservesComments` (a comment line before `walls:` survives byte-for-byte).

- [ ] Tests, then implement.
- [ ] `CONTENT_CONVERT=write go test ./dungeonspec/ -run TestConvertV4Testdata`; hand-fix any file the converter refused; list them in the PR.

**Verification:** `grep -rl walkableHexes rulebooks/dnd5e/encounter` returns nothing; `go test -race ./dungeonspec/`.
**Completion evidence:** the list of 25 converted files and any hand-converted ones.

### Task 4: Test fixtures and inline YAML in tests

**Delivers:** F8 (test surface).
**Owner:** rpg-toolkit encounter.
**Prerequisites:** T1, T3.
**Files:** `single_room_decode_test.go` (8 string swaps), `single_room_lowering_test.go` (4, plus workspace-radius cases removed), `v4_root_tables_test.go` (2), `prop_bindings_test.go` and `v4_gameplay_test.go` (inline full YAML → v5 with one region), `v4_concealments_test.go`, `v4_run_test.go`; Go-field builders `single_room_compile_test.go`, `single_room_discovery_test.go`, `single_room_walls_test.go`, `door_threshold_floor_test.go`, `door_threshold_region_test.go`, `wall_door_hearing_test.go`, `seat_scale_test.go` (`openFloorSpec` builds `Regions` with rows instead of `WalkableHexes`, and stops rewriting a workspace line; `world-builder-v3.yaml`'s exact workspace line requirement goes).
**Behavior:** every test's intent kept; a test whose only point was the workspace bound is deleted and named in the PR.
**Tests:** the existing suite.

- [ ] Convert; `go test -race ./...` green.

**Verification:** as T2. **Completion evidence:** list of deleted tests with reasons.

### Task 5: Goldens prove the floor is unchanged

**Delivers:** F8, Law Equality.
**Owner:** rpg-toolkit encounter `dungeonspec/contentgolden_test.go`.
**Prerequisites:** T2, T3, T4.
**Files:** new test `TestConvertedContentKeepsItsFloor` reading each `testdata/*.compiled.json` at the base commit (captured once into `testdata/pre-v5/*.compiled.json` by a script step in the PR, or compared by `git show origin/main:<path>` in the test; choose the committed copy and delete it in the same PR after the comparison test has run in CI once; say which); then regenerate the 25 goldens with `CONTENT_GOLDEN=write`.
**Behavior:** for every content file, `cells`, `props`, `boundaries`, `doorways`, `exits`, `start`, `placed`, `doors`, structural walls and doors, intel, scenarios, concealments, endings, factions, dispositions, templates are byte-identical to the pre-v5 golden; the union of `regions[].cells` is identical while the grouping changes (castle: 8 partitions → 10 authored regions); every region's lighting is 1 unless the source authored a lighting; `monsters[].region` and `party_start[].region` carry authored ids and the same cells as before.
**Tests:** `TestConvertedContentKeepsItsFloor` as above with a per-key diff listing.

- [ ] Capture pre-v5 goldens; write the comparison; run; regenerate goldens; commit the regenerated goldens and the comparison's output in the PR body.

**Verification:** `go test -race ./dungeonspec/ -run 'TestEveryContentFile|TestConvertedContentKeepsItsFloor'`.
**Completion evidence:** the per-file diff summary (expected: only region grouping and ids change; the union is equal).

### Task 6: The 30k fixture and the file budget

**Delivers:** F13, R1 file budget.
**Owner:** rpg-toolkit encounter `dungeonspec/scale_test.go`.
**Prerequisites:** T1, T2.
**Files:** add `bigSite(regions, cols, rows, wallsPerRegion int) []byte` emitting v5 YAML (rooms of `cols×rows` tiled in a grid with a wall and a door between neighbours, one region per room, alternating lighting 1 / 0.4); `TestBigSite_GeometryUnder100KB` (30,000 hexes: 60 regions of 25×20); `BenchmarkCompileSingleRoom_30k`.
**Behavior:** the YAML's bytes for `regions` plus `walls` under 100,000; compile succeeds; the benchmark time is recorded on S1's sub-issue.
**Tests:** as named.

- [ ] Implement; run `go test -race ./dungeonspec/ -run TestBigSite -bench Compile.*30k -benchtime 3x`.

**Verification:** command above. **Completion evidence:** bytes and ns/op posted on the sub-issue.

### Task 7: Docs, gates, tag

**Delivers:** S1 ready.
**Owner:** rpg-toolkit encounter.
**Prerequisites:** T1–T6.

- [ ] `dungeonspec` doc.go and `encounter` doc.go: regions vs partitions, the two divisions, the persisted partitions.
- [ ] `go fmt ./... && go mod tidy && golangci-lint run ./... && go test -race ./...`; tree clean.
- [ ] PR ready; independent review at head; merge; tag `rulebooks/dnd5e/encounter/v0.150.0` per the toolkit's release procedure.

**Verification:** CI green; tag published. **Completion evidence:** tag name on #725.

## S2 — session follows (rpg-toolkit, session module)

### Task 8: Re-pin and convert the session fixture

**Delivers:** S2 ready.
**Owner:** rpg-toolkit `rulebooks/dnd5e/session`.
**Prerequisites:** S1 tag.
**Files:** `go.mod` (encounter → v0.150.0), `testdata/world-builder-v4-castle-kitchen.yaml` (convert with T3's converter via a one-off `go run` from the encounter module, or copy the encounter testdata's converted twin), `launch_template_test.go` (`castleKitchenFixture` path unchanged), `castle_acts_test.go` (loads the castle from the pinned encounter module; passes once pinned).
**Behavior:** `AtlasRegion` pass-through unchanged; reveal bodies carry authored ids.
**Tests:** existing suite; add `TestReveal_GroupsByAuthoredRegion` only if an existing test pinned a `/space/` id (none found; verify).

- [ ] Pin; convert; `go test -race ./...`; `golangci-lint run ./...`; ready; review; merge; tag session.

**Verification:** module gates. **Completion evidence:** session tag on #725.

## S3 — shipped content (rpg-api)

### Task 9: Pin and convert

**Delivers:** F8 on the shelf; K1.
**Owner:** rpg-api.
**Prerequisites:** S1 and S2 tags.
**Files:** `go.mod` (encounter, session tags); convert 14 files: `content/{action-augment-walk,creature-facts-walk,reference-discovery-checks,reference-pursuit,the-mad-king}.yaml`, `internal/dungeons/testdata/{castle-kitchen,creature-facts-walk,placed-table-room,workshop-one-seat,workshop-room}.yaml`, `internal/handlers/dnd5e/authoring/v1alpha1/testdata/structural_layout.yaml`, `internal/integration/session/testdata/{ordinary_discovery,structural_patch,walled_seats}.yaml` (the castle and castle-kitchen are copied from the toolkit's converted testdata, as `TestCastleCopiesAgree` requires; the rest via the converter); fix `internal/orchestrators/lobby/start_encounter_single_room_test.go` (the `bytes.Replace` of `{q: -1, r: 1}, ` becomes an edit of the row `q` span, e.g. replace `[[-1, 1]]` for that row; keep "the edit changed the file" and the 8→7 expectation), its one-region assertions and `sessionworld/workshop_test.go` `TestTheImplicitRegionIsOneBrightCryptRoom` (one authored region, id `room-1-region`, intensity 1: still true for the workshop), `ordinary_discovery_acceptance_test.go` (5 cells, reveal of 1 region of 3 cells: still true; verify).
**Behavior:** boot compiles every content file; `ListDungeons`/`GetDungeon` unchanged.
**Tests:** existing; `TestCastleCopiesAgree` passes against the toolkit's converted castle.

- [ ] Pin; convert; `make test`; `make release-pin-check`.

### Task 10: Recapture atlas goldens and clean the local stacks

**Delivers:** S3 ready; the castle stack serves the converted castle.
**Owner:** rpg-api; game-dev (local env).
**Prerequisites:** T9.
**Files:** `internal/dungeons/testdata/atlas/*.atlas.json` for the five converted content files (recapture per the file's documented manual procedure, from the pinned build; the PR body lists the expected diff: castle regions 8 partitions → 10 authored regions (nine named plus the implicit), names authored, lighting 1); `atlas_fixture_test.go` doc comment gains the recapture line.
**Behavior:** every shipped site answers its golden.
**Local stacks (not in the PR):** delete old-dialect files from every mounted content copy before `up` (`.runtime/local/castle/sources/rpg-deployment/content/` holds 7, two never shipped; `.runtime/dev/...` and `npc-activities` similar), or the api refuses to boot; the dev Redis site `dungeon:world:test-world` / `the-mad-king-s-castle` is old-dialect and fails on open: delete the hash field (pre-release, R6).

- [ ] Recapture; `make test`; PR ready; review; merge to dev; rebuild the castle stack on dev/dev; confirm the castle opens from the shelf.

**Verification:** `make test`; castle stack boots; `grpcurl` validate-only of `content/the-mad-king.yaml` returns no errors.
**Completion evidence:** golden diff summary; stack up on :3030/:8115.

## S4 — the studio (rpg-dnd5e-web)

### Task 11: Draft model, codec, fixtures (PR 1)

**Delivers:** F1–F5, F9, F10 (model half), F12, rpg-dnd5e-web#1284.
**Owner:** rpg-dnd5e-web `src/concepts/world-building`, `src/concepts/encounter-studio`.
**Prerequisites:** T1's contract; T3's converted castle for the fixture.
**Files:** `roomDraft.ts` (`RoomGameplayData.regions: RoomRegion[]` replaces `walkableHexes`; `RoomDraft.workspace` removed; `defaultRoom` creates the implicit region with `lighting: {intensity: 1}` and no cells; `updateWalkableHexes` becomes `paintCells(draft, cells, regionId)` and `eraseCells(draft, cells)`; `walkableCellsInWorldRectangle` without a workspace; `isCellWithinWorkspace` callers removed; `ROOM_DRAFT_ENVELOPE_VERSION = 6`, 5 refused by name); new `regionRuns.ts` (`toRows(cells): RoomCellRow[]`, `fromRows(rows): RoomHexCell[]`, canonical, with the same refusals as the toolkit's `ExpandRows`); `authoringRegions.ts` (gameplay `RoomRegion {id, name, lighting: {intensity}, cells}` in `room.room.regions`; presentation `RegionPresentation {wallFinish?, construction?: unknown}` under `scene.regionPresentation` keyed by id, where `construction` is carried verbatim and unread per O2; `AuthoringRegion` boundary kinds retire; `labelId` retires); `serialization.ts` (`validateMapLabels` without a workspace bound: a fixed `MAX_WORLD_COORD`; labels are notes only); `singleRoomDungeon.ts` (`version: 5` always; `carriesV4Keys` removed; encoder writes `regions` with `cells` as rows, in the key order id, name, lighting, cells; decoder reads v5 only, 3/4 refused by name); `workspaceGeometry.ts` (`RoomWorkspace` retires; keep the odd-r maths and `MAX_ROOM_WORKSPACE_HEXES` as a paint cap `MAX_FLOOR_HEXES = 65536`); `encounter-studio/overviewFrame.ts` (the existing `contentBounds(draft)` reads the union of region cells; `workspaceContentBounds.ts` retires with the workspace); `refusalJump.ts` (`room.room.regions[i]` and `.cells[j]...` → the region, kind `label` renamed `region` in `StudioSelection`); fixtures: `world-building/fixtures/singleRoomV4.golden.yaml`, `singleRoomHearing.golden.yaml`, `singleRoomOptions.golden.yaml`, `worldBuilderV4Site.yaml`, `worldBuilderV4Patrols.yaml`, `world-building/fixtures/samples/01–04*.yaml`, `encounter-studio/fixtures/{creature-facts-walk,reference-discovery-checks,the-mad-king}.yaml` (the castle copied verbatim from rpg-api's converted content), `encounter-studio/fixtures/{castleS4,castleWorkspace,castleReadBack,regionLighting}.ts`, `encounter-studio/fixtures/fakeStudioSession.tsx`; tests touching `walkableHexes` (42 files; the integration test alone has 28 references).
**Interfaces:** the YAML of T1 byte-for-byte for the castle: `exportDraftYaml(readDraftYaml(castle)) === castle` stays the round-trip test. Selection kind `region` replaces `label` for regions; `StudioRegionEditing` becomes `{createRegion(name), renameRegion, setRegionLighting(id, intensity), removeRegion(id)}` (cells of a removed region fall to the implicit region).
**Behavior:** a draft with no workspace serialises and parses; a region with no cells round-trips (`defaultRoom`'s implicit region is written as `cells: []`); the castle fixture round-trips byte-identical; envelope v5 refused with "rebuild, no migration (rpg-project#725)"; the 500k cap stays and the castle's envelope is under 60k characters (assert in a test, replacing #1284's failing case with a 6,208-hex floor that fits).
**Tests:** `regionRuns.test.ts` (canonical rows, refusals mirroring T1); `roomDraft.test.ts` (paint into a region, reassign, erase to no region, removal falls to implicit); `singleRoomDungeon.test.ts` (goldens regenerated and `toBe`-pinned; v4 refused); `encounter-studio/draftFile.test.ts` castle round-trip; `refusalJump.test.ts` (`room.room.regions[2].cells[4].q[0]` → region id of index 2); `serialization.test.ts` (`6,208-hex floor envelope under MAX_JSON_LENGTH`).

- [ ] Tests first per file; implement; `npm run test:run` green; `npm run typecheck`.

**Verification:** `npm run ci-check` at the PR boundary.
**Completion evidence:** test names; castle round-trip; #1284 closed by the PR body.

### Task 12: Unbounded canvas and the region brush (PR 2, stacked on PR 1)

**Delivers:** F10, F11, the operator's walk.
**Owner:** rpg-dnd5e-web `src/concepts/encounter-studio`, `world-building`.
**Prerequisites:** T11; an api from S3's branch for the preview.
**Files:** `LayoutViewport.tsx` (`LayoutGrid` draws cells of `contentBounds(draft)` expanded by `LAYOUT_GRID_MARGIN = 6` cells (distinct from `author/creation/canvasGeometry.ts`'s `GRID_MARGIN`), minimum 24×24 around the origin when empty; fill by region colour from `overviewColours.regionColours`; the grid grows as paint approaches an edge; `pickLayoutCell` without a workspace: any cell); `layoutGeometry.ts` / `overviewFrame.ts` (`fitExtent` = content bounds plus margin; zoom 1 fits it; frame state unchanged, not persisted); `EncounterStudioWorkspace.tsx` (`activeRegionId` state, default the implicit region; `layoutTool` paint/erase/rectangle write through `paintCells(cells, activeRegionId)`; the `region` tool and `regionTool` state retire; `commitFloor` carries the region); `LayoutRoomList.tsx` (the Rooms list becomes the regions list: swatch, name, hex count, a radio for active; **New region** creates one and makes it active); `StudioInspector.tsx` region view (Rename, Lighting intensity %, Remove; hex view shows "Region: <name>" and "Make active"); `StudioDimensions.tsx` retires with the room view's Size section; `RegionBoundaryOverlay.tsx` (fill by region cells); `MapLabelOverlay.tsx` (region names at centroid, notes at their location); in `world-building`: `WorkspaceCellOverlay.tsx`, `workspaceFloorGeometry.ts`, `WorkspaceFloorUnderlay.tsx`, `WorldBuildingViewport.tsx` `makeHexLines` (grid from content bounds); play: `useDungeonScene.ts` and `RoomSceneEnvironment.tsx` stop reading `workspace` (floor disc from content bounds); `WorldBuildingConcept.tsx` (`commitFloor`, undo unchanged); `docs/how-to/encounter-studio-verification.md` (new steps).
**Behavior:** with the castle open, the Rooms list shows nine named regions and the implicit one with counts summing to 1,586; selecting Gatehouse and painting over courtyard cells moves them (counts change, floor unchanged); erasing removes floor; the exported YAML shows the moved cells under the gatehouse's rows; the preview atlas lists the regions with their intensities; zoom-to-fit frames the content.
**Tests:** `LayoutViewport.test.tsx` (grid cells = bounds + margin; empty draft shows the default grid), `overviewFrame.test.ts` (fit = content), `EncounterStudioIntegration.test.tsx` (paint into active region; reassign; new region becomes active; remove falls to implicit; region rename renames the map label), `StudioInspector.test.tsx` (region view fields), `regionLighting.test.ts` (intensity from the region).

- [ ] Tests first; implement; `npm run ci-check`.
- [ ] Boot on the builder's ports against S3's api; report the head for the early view; then the walk.

**Verification:** `npm run ci-check`; the walk checklist on S4's sub-issue, on :3030 after S3 is on dev.
**Completion evidence:** screenshots of the regions list and a reassignment; the exported castle diff; the operator's tick.

---

## Visible plan checks

### Requirement coverage

| requirement | tasks | proof |
|---|---|---|
| F1 regions are the floor | T1, T2, T11 | `TestDecode_RegionsRequired`; `TestAtlas_ListsAuthoredRegionsWithTheirLighting`; `roomDraft.test.ts` paint/erase |
| F2 canonical rows | T1, T11 | `TestExpandRows_*`; `regionRuns.test.ts` |
| F3 no cell in two regions | T1 | `TestDecode_CellInTwoRegions` |
| F4 implicit region | T1, T11 | `TestDecode_ImplicitRegionIdMayBeUnlisted`; removal-falls-to-implicit test |
| F5 no workspace | T1, T11, T12 | `TestDecode_WorkspaceRefused`; castle round-trip without `workspace` |
| F6 one RegionInput per region; partitions derived | T2 | `TestPartitionFloor_TwoRegionsOneOpenSpace`; `TestFieldLoad_DerivesTheSamePartitions`; `TestCompile_FootingLeavesItsRegion` |
| F7 readers on the right division | T2 | `TestHearing_AuthoredEdgeAddsNoHop`; `TestRegionAt_IsTheAuthoredRegion`; G7 table in the PR |
| F8 floor unchanged | T3, T5, T9, T10 | `TestConvertedContentKeepsItsFloor`; `TestCastleCopiesAgree`; atlas goldens |
| F9 version 5 | T1, T11 | `TestDecode_Version4Refused`; v4 refused in `singleRoomDungeon.test.ts` |
| F10 brush into active region | T11, T12 | integration tests named in T12 |
| F11 grid from content | T12 | `LayoutViewport.test.tsx`, `overviewFrame.test.ts` |
| F12 refusals jump to the region | T11 | `refusalJump.test.ts` |
| F13 30k under 100 KB | T6 | `TestBigSite_GeometryUnder100KB` |
| K1 fog, beats, doors, QA castle | T2, T9 | castletest suite; api integration fixtures; the operator's walk |

Deferrals (by design): O1 sound, O3 placed rooms, O5 runs on the wire, the three sibling budgets.

### Provider/consumer seams

| provider | consumer | contract | availability | proof |
|---|---|---|---|---|
| T1 dialect v5 | T11 codec | the YAML in T1 (keys, order, row grammar, version 5) | T1 first; T11 may start from the written contract and the converted castle | castle round-trip byte-equal on both sides (`TestConvertV4_CastleRoundTripsTheFloor`; `draftFile.test.ts`) |
| T1 decode errors | T11 `refusalJump` | paths `room.room.regions[i].cells[j].q[k]` and messages | rpg-api passes paths unchanged (inspected) | `refusalJump.test.ts` with a path produced by T1's tests |
| T2 `AtlasRegion` (authored) | session pass-through → api convert → web `atlasToScene3D`, `applyReveal`, `atlasRegion.ts` | same fields; ids and names now authored | no proto change | api atlas goldens (T10); web `regionLighting.test.ts` |
| T3 converter | T8, T9 fixtures | converted bytes decode under T1 and keep the floor | T3 before S2/S3 | `TestCastleCopiesAgree`; T5 |
| S1 tag | S2 go.mod; S3 go.mod | `rulebooks/dnd5e/encounter/v0.150.0` | published by toolkit CI | `make release-pin-check` |
| S3 api branch | T12 preview and walk | PutDungeon validate-only on v5 | S3 built before the walk | the walk checklist |

File-sharing: T1–T7 are one branch, sequential. T11 and T12 are two PRs, T12 stacked on T11; retarget T12 before merging T11's base if T11 merges first.

### Scan findings

- Every consumed name is produced by a named task or inspected as existing (`axialOffset`, `PartitionRegion`'s passability, `regionColours`, `refusalJumps`, `KnownFields`).
- The only user-visible outcome not covered by a unit test is "the current region name reads Gatehouse in play"; it is on the walk checklist.
- No placeholder remains; G1–G8 settle every decision a builder would otherwise invent. G5 follows R8 (derive, never store); its measurement on S1's sub-issue is what would reopen it.
- T12 is large; it is one deliverable (the studio authors the new shape) and splits only into its two PRs.
