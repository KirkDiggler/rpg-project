# Dungeon Builder: Implementation Plan

**Parent:** rpg-project#175
**Design:** `ideas/dungeon-builder/design.md` (rpg-project#170, merged)
**Tracking surface:** this plan PR stays open until the cross-repo implementation it tracks is complete.

**Goal:** Deliver the Dungeon Builder authoring loop—available at `/author`
without a dev-only gate to normally Discord-authenticated users on Kirk's server
through existing auth—and its approved geometry dialect in the ordered slices
#176–#180, while keeping the toolkit as the canonical source of dungeon rules
and geometry, the API as a projection and orchestration layer, and the web as a
renderer/editor of server-provided truth.

## Working rules

- This plan implements the approved design; it does not reopen its settled
  semantic-region, canonical-edge, runtime-wire, or YAML-version decisions.
- Develop outside-in: the product editor need names the contract, protos
  transcribe that need, API names the toolkit requirement, and toolkit provides
  canonical behavior. Merge inside-out: providers release first, consumers pin
  released versions, and the product route follows.
- One branch per repository per slice, except for the explicit Specimen Pack
  v0.1 toolkit provider loop below. Keep fixes discovered while integrating a
  slice on that slice's branch; do not split them into stacked provider PRs.
- Branch from a freshly fetched remote base: `origin/main` for
  `rpg-toolkit` and `rpg-api-protos`; `origin/dev` for `rpg-api` and
  `rpg-dnd5e-web`. API and web PRs explicitly target `dev`.
- Every contract continues to project runtime wall geometry through
  `HexRecord.edges`; neither authoring nor runtime may restore the removed
  flat `EncounterService.Space.walls` field.
- The web renders server-provided floor-plan legality and geometry. It never
  reconstructs parity, room-chain placement, edge canonicalization, movement,
  LoS, or room identity.
- Keep the design's error split: malformed requests use `InvalidArgument`;
  well-formed YAML content failures return `success=false` plus field errors;
  a genuinely unavailable `AuthoringService` is `Unimplemented`.
- Each implementation issue/PR records its parent slice and ends GitHub bodies
  and comments with `— asset-pipeline agent, on behalf of KirkDiggler`.
- The [Specimen Pack v0.1](https://github.com/KirkDiggler/rpg-project/issues/175#issuecomment-5162198168)
  is the single canonical YAML grammar and acceptance specimen. The current
  `dungeonspec`/compiler behavior is prototype scaffolding, not a
  backward-compatibility contract: if it conflicts with the specimen, the
  specimen wins. Do not create a dual dialect.
- **Staged compatibility (Kirk decision):** while strict `dungeonspec` support
  is incomplete, production authoring may use the web's explicit
  strip-before-send adapter. It is load-bearing: it accepts canonical Specimen
  v0.1 YAML and makes a temporary backend-subset projection; it is not a second
  schema. Before every
  send, it must expose an exhaustive loss report naming every dropped canonical
  field occurrence by YAML path, including dropped collection entries (not just
  aggregate counts). A passing projected request is labeled **subset validation**
  only—never full-schema or Specimen-v0.1 support. Give the adapter fixture
  coverage that proves its payload and loss report agree exactly. As each slice
  lands strict backend support, remove its fields from the drop path and pass
  them through unchanged; delete the adapter when no drops remain. This
  supersedes the prior absolute no-production-strip wording.
- The dungeon document remains `version: 1`; the specimen pack's `v0.1` is an
  independent pack version, not a schema version. Canonical syntax is not a
  promise that every field ships in this plan: retain these slices and add
  unsupported specimen capabilities progressively as their owning work lands.
- **Specimen metadata errata:** count-based
  `rooms[].obstacles[].ref/count` and `connectors[].locked.dc/ability` compile
  today. The specimen's note that `walls[].kind` mirrors a current
  `EncounterService.Space.walls` wire type is stale; `Space.walls` is
  removed/reserved and runtime edge truth is `HexRecord.edges`.

### Specimen Pack v0.1 toolkit provider loop (Kirk decision)

- `rpg-toolkit` PR #876 (the branch for `rpg-toolkit#875`) remains the one open
  toolkit provider branch/PR for related Specimen Pack v0.1 implementation
  asks. It is the deliberate exception to the per-slice branch rule; do not
  merge or tag it after each slice merely for ceremony.
- During that loop, API local development keeps **exactly one** toolkit-module
  override: the encounter module, pointed at #876. It never adds a second
  toolkit-module override.
- Merge and tag the toolkit provider once the API and web consumers stop
  requesting related toolkit changes. Then the API removes its override and
  pins the released encounter module before it is considered ready to merge.
- An unrelated toolkit unit of work uses its own issue and branch and may merge
  independently; it neither joins nor waits for this provider loop.
- Every new commit to #876 invalidates the prior provider gate. Re-review the
  exact new head and refresh applicable evidence before it is again considered
  merge-ready.

## Prerequisite: product editor route — rpg-dnd5e-web#671

Before a slice asks the web to prove product-route behavior, #671 starts fresh
from the latest `origin/dev`. It never cherry-picks from or continues the
concept branch: the concept is evidence and learning only, not implementation
authority. Existing components already on `dev` may be retained only when they
are production-suitable. #671 delivers the `/author` product route without a
dev-only gate, available to normally Discord-authenticated users on Kirk's
server through existing auth, with honest live, unavailable, invalid, and
save-ready states; comment-preserving document editing; and the save → lobby →
Walk it handoff. It defines the consumer affordances without inventing client
rules or extending the concept route.

Fixture adapters remain concept/test concerns. The product route instead uses
local API endpoint integration with the real `AuthoringService`. Keep #671 and
#678 separate and ordered: #671 delivers the product editor shell; then a
separate #678 branch, fresh from the latest `origin/dev`, delivers the Slice
#176 generated-edge web leg on that route.

**Gate and evidence:** targeted comment-round-trip/destructive-board-edit tests
and `npm run ci-check`; route screenshots/recording of each availability state;
and a real product-path save/Walk it proof once the available authoring contract
permits it. Record the flattened-versus-hex product display decision in that PR.
The route remains the integration surface for every web leg below, not a second
parallel editor.

## Slice #176 — generated wall and door truth

**Outcome:** Read-only canonical generated solid-wall and door geometry reaches
`FloorPlan` and is rendered and hit-tested identically by the 2D and 3D product
editor. No YAML `walls:` field, editing, or topology change is included.

1. **Name the consumer contract first (rpg-api-protos, `origin/main`).** Define
   an authoring-local edge message/enum in the `FloorPlan` response that preserves
   canonical endpoints, kind, and source door identity where available. It is not
   `dnd5e.api.v1alpha2.encounter.Wall`/`WallKind`; its comments explicitly map it
   to runtime `HexRecord.edges`. Run buf checks and publish the proto release.
2. **Develop the API consumer seam (rpg-api, branch from `origin/dev`).** Start
   the API slice branch after the authoring contract is reviewable. It consumes
   the published proto version, projects toolkit-owned canonical edges through
   the authoring flow, and does not re-canonicalize or derive competing geometry.
   If the existing toolkit API cannot describe every required edge, state the
   narrowly named provider requirement before implementing it.
3. **Supply canonical toolkit support (rpg-toolkit, branch from `origin/main`).**
   Add only the canonical edge export/describe support required by the API
   consumer. The toolkit remains the source of generated edge truth; generated
   dungeon and runtime behavior otherwise remain unchanged. Keep this work on
   the defined Specimen Pack v0.1 provider loop; do not merge or tag it merely
   because this slice's local support is ready.
4. **Iterate locally without committing an unpublished dependency.** During the
   provider loop, API may use `rpg-api/docs/how-to/local-toolkit-override.md` for
   **exactly one toolkit module**: the encounter module pointed at #876. The #176
   API slice branch must harden and constrain `scripts/toolkit-local-override.sh`
   so it permits exactly one module override and verifies the resulting module
   list; that supporting script change belongs on the same API slice branch, not
   a separate wave. API consumes the published proto version rather than
   overriding protos locally.

   **Release-pin validation (before opening the API PR and again at the final
   API gate):** remove the override; run the API build/test validation with
   `GOWORK=off`; prove the committed diff contains no `replace` directive; and
   check that no active `go.work`, `go.work.sum`, or local-toolkit override path
   can redirect the build. Record the commands and their output in the API PR.
   A repository-local or user/global Go workspace is not an acceptable substitute
   for this proof.

   **Decision gate:** any recovery to a separate-PR workflow is an explicit Kirk
   decision/gate; this plan does not approve it.
5. **Deliver inside-out.** Release proto support when its provider contract is
   ready. Keep related toolkit asks on #876 until the API and web consumers stop
   requesting them; then merge/tag the toolkit provider once, update API to the
   real released pin, and remove its override. After #671, land the #176 web
   integration as the separate #678 branch fresh from the latest `origin/dev`, on
   the #671 product route. The web consumes the projected edge contract and does
   not infer missing geometry.

**Gate and evidence:**

- Contract coverage proves `showcase.yaml` exposes a generated door, exterior
  wall, and interior-facing segment without duplicate/conflicting client edges.
- API integration evidence captures wire state from that fixture against real
  released provider pins, plus tests that guard the projection rather than a
  duplicate canonicalizer.
- Product evidence captures 2D and 3D `/author` screenshots/recording showing
  matching coordinates, kinds, render, and hit targets; concept/sandbox-only
  evidence does not satisfy this gate.
- Existing generated-dungeon and runtime encounter regression gates pass.

**Stop point:** do not begin authored edge editing until provider-release pins,
wire state, and reachable product-route visual evidence all pass.

### #176 technical review checklist and evidence (reviewed 2026-08-02)

- [x] Delivery order, one-branch-per-repository discipline, and the temporary
  override boundary are explicit.
- [x] Release-pin validation explicitly requires `GOWORK=off`, no committed
  `replace`, and checks for active `go.work`, `go.work.sum`, and
  local-toolkit-override redirection before both the API PR and final API gate.
- [x] The #176 API slice owns the hardening of
  `scripts/toolkit-local-override.sh`: it must allow exactly one override and
  verify the module list.
- [ ] Execution evidence is not available yet: #176 has not delivered an API
  branch, released pins, command output, or product-route proof. These remain
  merge gates, not completed evidence.
- [ ] Separate-PR workflow recovery is **not approved**. It remains an explicit
  Kirk decision/gate; the supporting script change is in the same #176 API
  slice branch, not a separate wave.

## Slice #177 — authored start marker

**Outcome:** Optional dungeon-scoped, absolute `start: [column, row]` overrides
the generated party-start anchor. Omitted or explicit `null` retains today's
behavior exactly. `FloorPlan.entrance` and `SpaceData.Entrance` keep their
existing names but report that one toolkit-resolved anchor, never competing
"authored" and "generated" fields.

### Contract and provider work

1. **Name the consumer behavior first (web, `origin/dev`).** The editor writes
   or clears the top-level absolute two-integer sequence and renders only the
   returned `FloorPlan.entrance`; it must not calculate room starts, connector
   gaps, a safe spawn line, or start legality. A room-door-row start is
   authorable, while the existing room-local `place` controls remain unavailable
   there. Save/preview and Walk it are the same product route.
2. **Transcribe the existing wire meaning (rpg-api-protos, `origin/main`).** No
   duplicate start field is introduced. Update the `FloorPlan.entrance` contract
   comment from “generator-chosen entrance” to “toolkit-resolved party-start
   anchor,” explicitly allowing an authored start in any semantic room. Preserve
   its field number and generated consumer compatibility; publish the normal
   proto release before API/web delivery.
3. **Make `dungeonspec` and encounter own the rule (rpg-toolkit,
   `origin/main`, on the Specimen Pack provider loop).** Decode `start` as an
   optional/null top-level absolute coordinate. `dungeonspec.Validate` rejects
   malformed coordinate shape, out-of-footprint/non-floor cells, connector gaps
   (including their door cell), absent/ambiguous semantic-room membership, and
   static blocking conflicts. A valid start may sit on any room's `doorRow`; do
   not loosen `place`/`boss.at` rules. Static validation deliberately does not
   predict seed-rolled walls or obstacles.

   Compile the resolved authored-or-default anchor and a toolkit-owned ordered
   party-spawn reservation into the dungeon parameters. The normal product
   configuration reserves **four** seats—the effective API default `PartyCap`,
   not a universal toolkit maximum. Service composition supplies that one
   capacity to registry compilation, preview, and runtime initialization; it is
   configuration passed to the toolkit, not API spawn math. This must be a
   dedicated party-start reservation, not a reuse of `ReservedCells`, whose ordinary placement rules
   reject the door row. Before *any* generated blocker is produced, the toolkit
   determines the anchor plus every configured seat and excludes all of them
   from interior walls, rolled obstacles, and other generated blocking-placement
   paths. The selection is deterministic and seed-independent; an insufficient
   envelope is an explicit generation error, never a moved anchor, nearest-cell
   search, generated-entrance fallback, or capacity reduction. The encounter
   exposes the first requested positions from this stored ordered reservation and
   returns an explicit requested-versus-available error when the request is too
   large.
4. **Project and orchestrate only (rpg-api, `origin/dev`).** Preview builds the
   real toolkit dungeon with the same normal four-seat configuration and projects
   toolkit `SpaceData.Entrance` verbatim as `FloorPlan.entrance`. Runtime uses
   the toolkit's ordered spawn-resolution API for `len(members)` before calling
   `AddPlayer`; delete the production `partyBase.Q + i` / `S - i` arithmetic and
   the API-owned spacing constant. If members exceed the reserved seats, return
   the toolkit’s explicit error with no partial encounter, ad-hoc placement, or
   fallback. Do not make the API derive a room, direction, or capacity rule.
5. **Deliver inside-out.** Publish the contract update, keep toolkit provider
   changes on #876 until consumers stop asking for them, then pin the released
   toolkit module in API and land the web integration on the prerequisite
   product-editor route. The preview's fixed seed may differ from a runtime
   encounter seed for rolled dressing, but both use the same seed-independent
   four-seat reservation and resolved authored anchor.

### Acceptance and stop point

- Toolkit validation covers omitted and `start: null` parity; correct absolute
  shape; every semantic-room membership; room-door-row acceptance; and rejection
  of connector gaps, out-of-footprint/non-floor coordinates, and authored
  movement-blocking prop, placed-monster, or pinned-boss conflicts.
- A named multi-seed sweep over a scattered/rolled authored-start fixture proves
  every generated map leaves the anchor and all four reserved seats usable and
  unchanged—no wall, rolled blocker, relocation, or fallback. Include an
  explicitly too-small envelope fixture that fails with the deliberate error.
- API/orchestrator coverage starts real one-, two-, three-, and four-member
  lobbies and proves positions are distinct, in toolkit order, valid in the
  actual encounter, and begin at the resolved anchor. A requested fifth member
  receives the explicit available-seat error; this documents product capacity
  four without claiming a toolkit-wide maximum.
- A real `PutDungeon(validate_only)` response/wire capture, persisted
  `SpaceData.Entrance`, and first player position from `StartEncounter` agree on
  the authored absolute cell. Product `/author` preview and Walk it capture the
  same marker and first player spawn. Repeat the same evidence for omitted and
  `start: null` fixtures to prove no behavior change.

**Stop point:** stop for a design correction if preview and runtime do not use
that same reservation, if any seed can invalidate or move it, or if an API/client
calculation rather than the toolkit selects a player position.

## Slice #178 — floor-prop hex facing

**Outcome:** Existing room-scoped **floor-prop** placements accept optional
`facing` in exactly this canonical label/value order: `E = 0`, `NE = 1`,
`NW = 2`, `W = 3`, `SW = 4`, `SE = 5`. Omitted and explicit `null` represent
absence; both are distinct from explicit `E = 0` and retain the current default
orientation.

### Contract and delivery

1. **Keep canonical YAML authoritative (web → API → toolkit).** The web writes
   only the canonical labels and does not manufacture a facing for absent/null
   input. Once this slice's strict backend support lands, the staged adapter
   removes valid room-scoped floor-prop `facing` from its drop path and submits it
   unchanged; a stripped pass cannot be claimed as facing support.
2. **Validate and persist in the toolkit.** `dungeonspec` accepts facing only
   for existing `rooms[].place[]` prop entries that are floor placements. It
   validates the six-value vocabulary and stores an optional/presence-bearing
   numeric enum so explicit `E = 0` survives compilation and runtime
   persistence. It rejects unsupported input at the supplied field path—not by
   conversion—including facing on monster placements, `rooms[].boss`, top-level
   `place`, and mounted placements (for example `mount: wall`).
3. **Project, do not reinterpret (protos/API).** Carry the toolkit-persisted
   optional value through the existing placement rendering path, adding no
   authoring `FloorPlan` placement field, placement delta, or facing echo. The
   API projects the value verbatim and must not default, map, or treat `0` as
   absent.
4. **Render with explicit precedence (web).** A present projected facing rotates
   the asymmetric floor prop; absent/null uses the existing prop-model default.
   The web does not calculate a direction, infer absence from `0`, or apply
   facing to a monster, boss, top-level, or mounted placement.

Do not broaden this slice into wall mounting, height, rectangular directions, or
AI/behavior changes.

**Gate and evidence:** direct strict path-specific validation reports the
accepted vocabulary for invalid floor-prop values and rejects each unsupported
scope above; adapter fixtures name every currently dropped unsupported occurrence
and never present a stripped pass as facing support. Toolkit persistence and API
projection tests cover explicit `E = 0`, omitted, and `null`; and product-path
screenshots or a recording visually demonstrate one asymmetric floor prop in
**all six** canonical directions in preview and runtime. Legacy omitted/null
YAML remains visually unchanged. Stop if any layer requires a client-derived
orientation semantic or a `FloorPlan` placement delta.

## Slice #179 — authored canonical wall and door edges

**Outcome:** Dungeon-scoped `walls: [{from, to, kind}]` edits canonical solid and
door edges, including inner walls, without changing semantic room identity.

Build on #176's proven read-only edge seam. Specify deterministic generated-edge
overlay behavior before provider code; initially reject collisions with
connector-generated edges. Compile authored edges into canonical runtime
`HexRecord.edges`; do not make authored doors satisfy or replace chain
connectors.

### #179 implementation discovery (source read 2026-08-04)

**Settled facts.** `WallSegmentData` endpoints are absolute cube **hex cells**,
not polygon corners. `Start == End` is the legacy blocked-cell form; the current
`Start != End` rebuild path still places a blocker over any valid in-grid `End`
cell. `DescribeGeneratedEdges` is a read/render projection, not the pairwise
movement, pathfinding, or LoS authority. Connector doors must initially retain
that legacy cell-threshold behavior: `DoorData` carries only one `Position`, and
its current passage neighbor is inferred from regions. Consequently an authored
inner edge needs a dungeon-owned, undirected boundary-crossing primitive used by
movement, pathfinding, and LoS; an authored door needs stable identity from
normalized endpoints plus reachability from either endpoint. YAML `from`/`to`
therefore name distinct, adjacent, absolute pointy-top floor cells; edges never
rename or split the existing semantic room IDs. The web consumes the
server-compiled edge list in preview and runtime, never a documentation-only
client overlay. Existing `FloorPlanEdge`/`HexRecord.edges` endpoint/kind/door-ID
shape likely suffices, so make no proto delta unless edge provenance becomes
product-visible.

**Settled defaults (#179).** `walls[].from`/`to` name absolute pointy-top
`[column,row]` coordinates; both endpoints must be distinct, adjacent, compiled
floor cells. Each edge's identity is undirected, formed by normalizing those
absolute endpoints, so a duplicate authored edge — including its reversed pair —
fails validation rather than letting conflicting authored kinds be silently
selected. The authored-vs-generated overlay is deterministic: an authored kind
(`solid` or `door`) replaces a colliding generated non-connector physical edge;
any collision with a connector-derived edge still fails validation. Exterior or
non-floor endpoint editing stays out of scope and read-only — this slice does
not expand it. An authored door's ID derives stably from the dungeon key plus
its normalized absolute endpoints; it starts closed and unlocked, and a player
may interact from either endpoint. Reachability inference, rejection, or
relocation based on semantic-region membership is out of scope: an inner
authored edge changes neither cell occupancy nor semantic room identity. No
proto delta lands unless implementation proves one is required, including any
product-visible provenance limitation that the existing `FloorPlanEdge`/
`HexRecord` wire cannot express.

**Stop conditions.**

1. Stop rather than encode an authored boundary edge as a `WallSegmentData`/cell
   blocker; the runtime must gate one boundary crossing for movement,
   pathfinding, **and** LoS, not a whole cell.
2. Stop if movement, pathfinding, and LoS cannot all consume that same
   dungeon-owned crossing primitive — rendering `FloorPlanEdge`/`HexRecord.edges`
   alone does not establish gameplay authority.
3. Stop if the single-position `DoorData` cannot be evolved or wrapped to yield
   a normalized, stable authored door ID with symmetric interaction from both
   endpoints.
4. Stop for a design decision if implementation requires connector mapping, a
   departure from the exterior/non-floor read-only scope above, reachability
   inference or relocation, or product-visible source provenance that the
   existing `FloorPlanEdge`/`HexRecord` wire cannot express.

**Gate and evidence:** clear validation for duplicate (including the reversed
pair), conflicting, non-adjacent, out-of-footprint, and connector-collision
edges; the settled authored-vs-generated overlay applied consistently; one
shared dungeon-owned edge representation; movement/LoS and door-lifecycle
evidence through the existing runtime path; an inner wall changes movement/LoS
while both cells retain the same room ID; product preview and Walk it agree.
Stop for a design decision if a connector mapping rule becomes necessary.

## Slice #180 — cell-authored semantic room regions

**Outcome:** Authors draw stable-ID, explicit occupied-cell semantic regions,
including non-rectangular regions, while retaining linear-chain topology and
connector semantics.

Build on #179's canonical-edge support. Define the rectangular-to-explicit-cell
compatibility/precedence path before implementation. Region membership—not walls
or geometric flood-fill—continues to own reveal, placement, spawning, scripting,
and archetype identity.

**Gate and evidence:** author-facing failures for empty, disconnected,
overlapping, and invalid cell sets; at least two connected non-rectangular
regions preserve IDs and linear connector requirements; adding/removing an inner
wall does not alter membership; compatibility coverage protects existing
rectangular specs; wire state and real product walkthrough agree on
cell-to-region membership. Stop for a design decision if supporting branching
topology, holes, levels, or automatic room naming becomes necessary.

## Completion and tracking

Keep this PR open as #175's cross-repo tracking surface while the prerequisite
and ordered slices run. Update it with linked per-repo issues/PRs, provider
release pins, gate evidence, and explicit stops/decisions. Close it only after
#176–#180 have each met their stated product-path evidence and the final
cross-repo retro confirms the authoring loop remains live.

— asset-pipeline agent, on behalf of KirkDiggler
