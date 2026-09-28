# Dungeon Builder Restart Survey

*2026-08-23 — read-only survey of the board, the shipped builder, and the new session stack, to restart the Dungeon Builder on composable encounters. Nothing in any repo was changed.*

## TL;DR

1. **The board already knows the next step.** Journey rpg-project#169 (Composable Dungeon Builder, In Progress, Kirk) names **rpg-api#806** — rebuild `PutDungeon` + `ListDungeons` on the session stack, honour `dungeon_key`, unblock the picker (#131) and Save & Play. Nothing has started.
2. **#806's premise is false.** It says "the web's `/author` already emits v0.4 YAML; this is the server that receives it." There are **two dialects both called v0.4** and they are different languages. #806 is a dialect decision first, a server rebuild second.
3. **Recommendation:** grow the new `rulebooks/dnd5e/encounter/dungeonspec` toward spec v0.4's authoring vocabulary and re-cut the spec against the session **atlas** (not the dead `FloorPlan`). Then the builder's preview can feed the game's own `buildScene3D`, making "what you author is what you play" structural.
4. **Process:** one design doc in `rpg-project/ideas/battlemap/dungeon-builder/` written from the panel back → Kirk rules once → protos first → toolkit / rpg-api / web in parallel → one walk → bottom-up merge.

---

## 1. What the board says

### The journeys

| # | Journey | Status | Team | Assignee |
|---|---|---|---|---|
| rpg-project#169 | Composable Dungeon Builder | In Progress | Cross-team | KirkDiggler |
| rpg-project#253 | Play the Dungeon on the Session Stack | In Progress | Cross-team | KirkDiggler |
| rpg-project#232 | Composable Attack Damage | In Progress | Platform | dammitbilly0ne |
| rpg-project#201 | Monster Behavior in the Local Dungeon | Todo / Ready | Monster AI | dammitbilly0ne |
| rpg-project#244 | Choose a Dungeon by Danger | Todo / Shaping | Cross-team | — |

All under initiative **rpg-project#231 — Four-player Level-3 Dungeon**.

### Journey #169 (rewritten 2026-08-21 — this is the freshest authority, not the memory file)

- **Outcome:** "An author can create, reopen, and edit canonical dungeon YAML through the visual builder — including multi-room structure, props, and lighting — then launch and play that authored dungeon locally without losing authored meaning."
- **Current reality:** the old encounter stack is gone from rpg-api `dev` (rpg-api#801). The builder's server side (`PutDungeon` → `dungeonregistry`, old dialect) went with it; `ListDungeons` returns `Unimplemented`. The new stack compiles the reference tomb from the new `dungeonspec` through `internal/sessionworld`.
- **Next proof:** **rpg-api#806** — the authoring write path on the new stack.
- **Done when:** "build a dungeon in the builder, then play through it" — through the real 3D game, including reopen.
- **Ledger, by leverage:** (1) rpg-api#806 → toolkit#1139 (room-local vs absolute seam) → rpg-project#131 / web#662 (picker); (2) play path — now largely landed (web#762 closed, slice 1 merged 2026-08-23, slice 2 #254 in flight on web PR #776); (3) authored meaning — #179/#176 edges, #190 lighting, #188/#204/web#737 offsets, #185 prop catalog, #189 walkthrough.
- **Explicitly stale, close or rewrite:** rpg-api#749, #751, #763, #772; rpg-project#187; web#671.

### Journey #253 (session stack) — what it says about dungeons

- Slice 1 **merged 2026-08-23** (#251): contact, walk, equip, swing, End Turn, monster passes, kill, fight-over. Live on dev.
- Slice 2 (#254, design #252): the monster's turn via `TurnDriver` + `behavior.Basic`. In flight.
- Dependencies: "Dungeon content beyond the tomb: #169 (builder) / #244 (choose by danger)."
- #227 tomb comment (2026-08-20): "the **dungeon builder still cannot author for this stack** — `PutDungeon` → `dungeonregistry` writes the OLD dialect. That is the next content-side piece of work."

### Spec v0.4 — rpg-project#206 / PR #203

- Ratified 2026-08-08. PR #203 deliberately **stays open** as the tracking surface.
- Versioning rule: v0.4 is a normative delta over v0.3; YAML stays `version: 1`; `canvas.floor_source: bounds|regions`.
- Wave A (region-union topology) issues cut: protos#217, toolkit#897, rpg-api#780, web#735. Wave B (offset + catalog): toolkit#898, protos#219, rpg-api#783, assets#44, web#737. Waves C/D not cut.
- **Not reconciled against #801.** Tranches A+B shipped into the OLD `rpg-toolkit/encounter/dungeonspec` (tag `encounter/v0.54.0`) — intact, tested, **zero consumers**.

### Board hygiene problems

- **rpg-api#806 — #169's named Next proof — is not on Project 19.** Neither are toolkit#1139, rpg-api#803, rpg-api#800, toolkit#1135.
- **#169 and #131 have zero sub-issues** — no progress rollup for the builder.
- Stale statuses: web#662 / web#666 "In Review" though PRs #665/#667 merged three weeks ago; #169 still says "the game route still speaks the deleted EncounterService" (false since 2026-08-23).
- Old-stack PRs still open against `dev`: rpg-api#782, rpg-api#794, web#748, web#742 — all target deleted packages.
- rpg-api#749 / #751 still Todo on the board though superseded.

---

## 2. What exists today

### Web — fully shipped, pointed at nothing

`rpg-dnd5e-web/src/author/` — ~150 files on `dev`, live at `/author`. Builder arc ran 2026-08-02 → 08-10, then stopped. `dev` has moved ~175 commits since with zero builder changes.

| Path | Role |
|---|---|
| `AuthorView.tsx`, `DungeonBuilderConcept.tsx:316` | route shell; `handleCreationSaveAndPlay` |
| `creation/CreationBoard.tsx` (74 KB) | 2D hex canvas — walls, region brush, markers |
| `creation/straightWallGeometry.ts` (42 KB) | wallLines → hex footprint + edge projection |
| `dungeonYaml.ts` (131 KB) | YAML/CST model; `DungeonDoc` :577; `stripToV1Subset` :2599 |
| `capabilityProbe.ts` | 17-field live probe of what the server accepts |
| `specCompat.ts:49` | `SpecCut = '0.3' \| 'draft'` — **no '0.4'** |
| `useSaveDungeon.ts:68-74` | the one write: `PutDungeon(validate_only:false)` |
| `preview3d/DungeonPreview3D.tsx` (93 KB) | 3D preview reusing the game's `SyntyHexFloor` / `PropModel` |
| `specimens/` | specimen pack labelled v0.4, encoding the **v0.3** cut |
| `CONTRACT.md` (480 KB), `TARGET-YAML.md` (134 KB) | the dialect's paper trail |

`grep -rn "session/v1alpha1" src/author/` → **zero hits.** The builder speaks only `dnd5e.api.authoring.v1alpha1`.

Also merged: the lobby dungeon picker (web#665, `useListDungeons.ts:33` → `DungeonPicker.tsx`) — now receives `Unimplemented`.

### Server — gone

Deleted by rpg-api#801/#804 (2026-08-21): `internal/dungeonregistry/`, `internal/orchestrators/authoring/`, `internal/handlers/dnd5e/authoring/`, `internal/content/` + YAMLs, and both env gates. **`RPG_AUTHORING_ENABLED` and `RPG_CONTENT_DIR` have zero code references.**

What runs now:

- `rpg-api/internal/sessionworld/reference-tomb.yaml` is `go:embed`-ed and recompiled on every `StartEncounter` (`sessionworld.go:74,151`).
- `sessionworld.compile(raw []byte)` (`:165-189`) is **already generic over bytes** — the blocker is purely "where do the bytes come from."
- `dungeon_key` is copied at `handlers/.../lobby/v1alpha1/start_encounter.go:38`, stored at `orchestrators/lobby/start_encounter_session_stack.go:35`, **never read**. A client asking for `fog-lab` silently gets the tomb.
- `ListDungeons`: no handler, no orchestrator — falls to `UnimplementedLobbyServiceServer` (`handler.go:33`).
- Room-local → absolute projection is **borrowed** via a throwaway encounter (`sessionworld.go:209-281`, ~318 µs/start). Durable fix = toolkit#1139.
- Stale docs: `start_encounter.go:33-38` describes an `effectiveKey` precedence that no longer exists; `rpg-api/docs/status.md` is pre-rip-out. `docs/architecture/components/lobby-service.md` is current.

### Protos — alive, describing a dead dialect

`rpg-api-protos/dnd5e/api/authoring/v1alpha1/service.proto` — `PutDungeon(key, yaml, validate_only) → {success, field_errors, FloorPlan}`. `FloorPlan` (`:165-208`) carries `rooms, connectors, door_row, edges, floor_cells, regions, floor_source, placements`; cells are odd-q `[column,row]`. Every commit predates the world-model wave; `archetype`, `canvas`, `regions`, `door_row` are all things the new toolkit dialect deliberately deleted. `GetDungeon` / `DeleteDungeon` never existed.

`lobby/v1alpha1`: `StartEncounterRequest.dungeon_key` (`:100`), `ListDungeons → DungeonSummary{key,name}` (`:239`). These shapes are fine.

---

## 3. The new stack's contract for a dungeon

### Architecture in one paragraph

Four layers, each forbidden from knowing the next: `play/*` leaf contracts → `rulebooks/dnd5e/encounter` (**the world**: canvas, regions, props, doors, sight, clocks) → `rulebooks/dnd5e/resolution` (the bus, per call) → `rulebooks/dnd5e/session` (**the table**: verbs, roster, events, persistence) → rpg-api (load, one verb, translate, return). Capabilities (`Standing`, `Sight`, `InitiativeRoller`, `TurnDriver`) are injected and **never defaulted** (toolkit#1033). "One map at the seam" (Kirk, #227): `Origin` is spent once at construction compiling authored rooms into one canvas (#1106); a room is a region at runtime (#1108); **walls are declared edges, never implied by room geometry** (#1130). No session process, no modes — every verb loads → acts → saves → dies.

Decisions digest: `rpg-toolkit/docs/adr/DECISIONS.md`. Map ADRs: 0035 (canvas floor mask + envelope), 0040 (Atlas carries layout). The "one map" design doc lives only on rpg-project branch `docs/227-session-api` (PR #228, open).

### The wire a builder is accountable to

`rpg-api-protos/dnd5e/api/session/v1alpha1/service.proto:446-490`:

```proto
message GetAtlasResponse {
  GridKind grid = 3;                  // ONE value for the whole map
  repeated Position cells = 4;        // axial (q,r), dungeon-absolute, sorted
  repeated AtlasProp props = 7;       // {ref, at, blocks_movement, blocks_line_of_sight}
  repeated AtlasBoundary boundaries = 6; // a wall = edge {from,to} between adjacent cells
  repeated AtlasDoorway doorways = 2; // {connection, from, to} — two cells, no state
  // (branch feat/1140-hex-layout adds HexLayout layout = 8)
}
```

> "WALLS ARE DECLARED, NOT IMPLIED BY ROOMS... Anything that authors a world for this service owes it these (rpg-toolkit#1130)."

**Not on the wire:** rooms, lights, terrain, spawn points, prop ids, prop facing. `GetAtlas` is construction truth — fetched once, cached.

### Three coordinate frames — the biggest trap

| Frame | Where | Shape |
|---|---|---|
| Authoring | `dungeonspec` YAML, `FloorPlanCell` | pointy-top **odd-q** `[col,row]`, room-local `at:` |
| Runtime wire | `session/v1alpha1.Position` | **axial (q,r)** double, absolute |
| Client | `hexLayout.ts` (builder) odd-q · `atlasWallRuns.ts` odd-r · `positionBridge.ts` axial | three schemes coexist |

Conversion happens once, at construction (`orientation.go:403-409`, offset-then-convert). Two production bugs came from this: #1141 (offset schemes), #1150 (axial basis) — invisible to round-trip tests. The builder↔game parity check in `src/author/boardGeometry.ts` **predates both fixes and is stale.**

### The toolkit's content format — it already exists

`rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec/spec.go:43-187`:

```go
type Spec struct {
	Version     int             // must be 1
	Key         string
	Void        string          // REQUIRED "opaque"|"transparent"
	Orientation string          // REQUIRED "pointy"|"flat"
	Height      int             // one height, whole dungeon
	Start       *[2]int         // REQUIRED, absolute [col,row]
	Rooms       []RoomSpec      // in layout order = geometry (west→east, one row)
	Connectors  []ConnectorSpec
}
type PlaceSpec struct {
	Ref            string  // "module:type:id"
	At             [2]int  // room-local
	BlocksMovement *bool   // REQUIRED on props, refused elsewhere
	BlocksLoS      *bool
	Targeting      *string // monsters only, opaque
	Boss           bool
}
```

Pipeline: `Load` = strict `Decode` (unknown keys fail) → `Validate` (standalone — a lint button for free) → `Compile` → `Compiled{Field encounter.FieldInput, PartyStart, Monsters}`. Live instance: `reference-tomb.yaml` (37 lines, 3 rooms, 28×8 = 224 cells). The compiler's own comments say 2-D layout (`compile.go:117`) and square grids (`:139`) are single-file changes; the composition already accepts arbitrary `Origin` and declared boundaries.

Gap: `Compiled.Field` is a `FieldInput`, but `session.StartSession` wants `*EncounterData`. Nothing joins them; the worked route is `dungeonspec/tomb_test.go:68-90` (`Load → NewEncounter → ToData()`), and rpg-api's throwaway-encounter workaround.

---

## 4. The finding: two dialects, both called "v0.4"

| | **Spec v0.4** (the builder's dialect) | **`rulebooks/.../dungeonspec`** (what the new stack runs) |
|---|---|---|
| Spec | `rpg-project/ideas/battlemap/dungeon-builder/spec/v0.4/spec.md` (536 lines, branch `origin/spec/v0.4-proposal`, PR #203) | none — `spec.go` godoc is the spec |
| Compiler | OLD `rpg-toolkit/encounter/dungeonspec` (`encounter/v0.54.0`, zero consumers) | NEW `rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec` (born 2026-08-20) |
| Topology | `canvas {width,height,floor_source: bounds\|regions}` + painted `regions[].cells` | room chain, one row west→east, hex only |
| Walls | authored `walls[]` (+ client-only `wallLines`); implicit envelope at floor/void boundary | seam walls **generated**; no wall grammar |
| Also | `holes`, `start`, `end`, `facing`, `mount`, `height`, `offset[x,y,z]`, `lighting.ambient`, `defaults`, `target/profile/params`, `theme`, `name` | `void`, `orientation`, absolute `start` all **required**; `targeting`, `boss: true` |
| Decode | — | `decode.go:51` `KnownFields(true)` — **strict-rejects every v0.4 key** |

The builder's from-scratch seed (`src/author/creation/emptyCanvasDoc.ts:57-75`) emits `spec: "0.3"`, `theme`, `canvas`, `walls`, `holes`, `end` and no `void`/`orientation` — the new decoder fails on the first key, then on three missing required fields.

The v0.3 README (`:21-26`) declared skew permanent: "the builder authors spec vN+1 while the platform implements spec vN." But this isn't vN+1 vs vN — it's two lineages.

**Ways out, nobody has ruled:**

- **(a) Grow the new dungeonspec toward v0.4's constructs** — regions as named cell sets, authored boundary edges, 2-D origins, offsets. The runtime model already *is* what v0.4 wanted (one canvas, regions, declared edges, `{ref, at, blocks_*}` props). The compiler says 2-D and square are one-file changes.
- **(b) A translation layer** — builder YAML → new dungeonspec. Lossy by construction (no authored walls to translate into).
- **(c) Re-cut the spec against what the new stack speaks** — drop canvas/regions/walls, author room chains only. Throws away the ratified v0.4 authoring model and the 42 KB of proven wall geometry.

**Recommendation: (a), with the spec re-cut on top of it.** Keep v0.4's authoring vocabulary where it maps to the runtime (regions, walls-as-edges, offset, lighting pending toolkit#1113); drop what the new stack refuses to default (`spec:` marker — v0.4 itself says it must fail decode; envelope-from-bounds); make the **atlas** the compile target and the preview payload instead of `FloorPlan`.

---

## 5. What the builder needs, in blocking order

1. **A write path against the new dialect** — `PutDungeon` → nothing today. Gap zero.
2. **A source of dungeon bytes other than `go:embed`** — `sessionworld.compile` is generic; no registry, content dir, or Redis keyspace behind it.
3. **Key → content lookup** — `dungeon_key` is dropped at `start_encounter_session_stack.go:35`. Until fixed it should at least return `NotFound`, not the wrong dungeon.
4. **`ListDungeons`** — shape exists; no handler. Design is #131's.
5. **`GetDungeon`** — reopen is in #169's Done-when and no RPC exists for it.
6. **The dialect ruling** (§4).
7. **`Compiled.Field` (`FieldInput`) → `EncounterData` bridge** — toolkit#1139; retires rpg-api's throwaway encounter.
8. **A lighting ruling** — emergent ("placing braziers IS authoring the light pools", `ideas/battlemap/dungeon-authoring/design.md:262`) vs authored render-only `lighting.ambient` (v0.4 tranche D). Toolkit reserved the slot (`void.go:66-70`, #1113) and built nothing; nothing about light is on any wire.
9. **Coordinate-frame unification in the client** — odd-q builder, odd-r wall runs, axial wire.
10. **Square grids / 2-D layout** — one-file compiler changes if wanted.

### Preview: reuse the game's renderer, structurally

`rpg-dnd5e-web/src/components/session/atlasToScene3D.ts:64-80`:

```ts
export function buildScene3D(
  atlas: Pick<GetAtlasResponse, 'cells' | 'boundaries' | 'doorways'>,
  hexSize: number
): Scene3D
```

Pure, combat-free, takes a structural `Pick<>` — the builder can hand it a synthesized atlas with no RPC. Leaf renderers (`SyntyHexFloor`, `AtlasWalls`, `PropModel`) are combat-free; `AtlasProp.ref` is already `dnd5e:props:<name>` and feeds `resolvePropVariant` directly. `DungeonPreview3D.tsx:40-56` declined to reuse the wall renderer because the only wall computer was fog-gated — **that reason is obsolete**; `atlasWallRuns.boundariesToWallRuns` is the fog-free, declared-boundary version. Blockers: the frame ambiguity; builder constructs with no atlas analogue (`wallLines`, `holes`, `mount`/`facing`/`height`); `SessionScene` hardcodes `useCameraControls`.

### Five rules the restart must not break

1. **Author rooms; the compiler makes one canvas.** Room-local `at:` stops at `dungeonspec`. Nothing past construction has a room.
2. **Never default a capability.** `void`, `orientation`, `start`, both prop blockers are required; pointer-vs-value keeps "author said nothing" distinct from "author said false."
3. **Walls are the author's obligation.** Two adjacent chambers with no drawn seam are one open room.
4. **Two lifecycles, never merged.** Authored reload reads current YAML; encounter reload reads the started snapshot (ADR-0035).
5. **Presentation is not geometry.** Meshes and offsets are cosmetic; every server-side blocker must have a visual.

---

## 6. Proposed next steps (per how-we-build)

1. **Design doc, panel-back** — `rpg-project/ideas/battlemap/dungeon-builder/design.md` (rewrite): what the builder shows → the YAML it writes → the atlas the stack serves → what `dungeonspec` must grow. One slice under #169; Kirk rules once. Subsumes #806 and settles the dialect + lighting questions on the record.
2. **Protos first** — retire `authoring.v1alpha1`'s `FloorPlan`; `PutDungeon` returns an atlas-shaped preview; add `GetDungeon`. `ListDungeons` / `dungeon_key` shapes stay.
3. **Parallel builds** — toolkit: dungeonspec growth + #1139 · rpg-api: content source, registry, `dungeon_key`, `ListDungeons`, `GetDungeon` · web: `/author` retargeted to the session atlas, preview via `buildScene3D`.
4. **Board hygiene first** — add #806 / toolkit#1139 / rpg-api#803 to Project 19 as sub-issues of #169; close rpg-api#782/#794, web#748/#742, rpg-api#749/#751; fix #169's stale "game route" line; move web#662/#666 out of In Review.

### Needs Kirk

- Dialect ruling: (a) / (b) / (c) above.
- Lighting: emergent vs authored ambient.
- PR #203: merge as history, or rewrite against the atlas.

---

## Key paths

| What | Path |
|---|---|
| Live dialect schema | `rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec/spec.go:43-187` |
| Compile pipeline | `.../dungeonspec/{decode,validate,compile}.go` |
| Reference wiring (copy this) | `.../dungeonspec/tomb_test.go:68-90` |
| Reference tomb YAML | `rpg-api/internal/sessionworld/reference-tomb.yaml` |
| The one projection | `rpg-toolkit/rulebooks/dnd5e/encounter/orientation.go:403-409` |
| Map-source path | `rpg-api/internal/orchestrators/lobby/start_encounter_session_stack.go:140-175` |
| Generic compiler seam | `rpg-api/internal/sessionworld/sessionworld.go:165-189` |
| Wire map | `rpg-api-protos/dnd5e/api/session/v1alpha1/service.proto:446-490` |
| Stale authoring contract | `rpg-api-protos/dnd5e/api/authoring/v1alpha1/service.proto` |
| Client scene seam | `rpg-dnd5e-web/src/components/session/atlasToScene3D.ts:64-80` |
| Existing builder | `rpg-dnd5e-web/src/author/` (+ `CONTRACT.md`, `TARGET-YAML.md`) |
| Ratified spec v0.4 | `rpg-project/ideas/battlemap/dungeon-builder/spec/v0.4/` on branch `spec/v0.4-proposal` |
| Decisions digest | `rpg-toolkit/docs/adr/DECISIONS.md` |
