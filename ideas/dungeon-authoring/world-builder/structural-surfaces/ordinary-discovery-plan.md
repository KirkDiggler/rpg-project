# Builder geometry → ordinary room discovery

## Goal, authority and baseline

Operator's current instruction: proceed with the existing room-discovery path.
An undiscovered far-side room and ordinary props stay absent behind a closed opaque
boundary, without explicit concealment; opening permits discovery; closing retains
known layout in the same encounter. Secret door discovery does not see through a
closed leaf. Explicit trap concealment remains independent.

Inspected encounter2229396d; session f0a8220c; API5b8e49a2; web5b36d447 plus local
wall-interaction work (preserve it). Existing law: dungeon-intel R12–R15, whole-room
fixed discovery, not per-cell fog. Existing compiler labels all v4 floor as one
implicit region. Existing sight queries already evaluate footprint walls/doors.

Shape: authored floor + blocker geometry → builder compiler's ordinary regions →
existing encounter room discovery/journal → existing recipient Atlas/reveal DTOs.
No client visibility, new concealment membership, new proto or second LOS algorithm.
Do not infer opacity from artwork: authored blocking flags remain authoritative.
No profile/reset/source mutation of the operator's run. Existing encounter saves
keep their compiled regions/knowledge; corrected partitions apply to newly compiled
worlds. No tag guesses, source overrides or automatic merges.

## Task 1 — Pin the failed joined behavior

Owner: encounter/dungeonspec. Files: new single_room_discovery_test.go, using the
existing SingleRoomWallDoorSuite helpers in single_room_wall_doors_test.go.
Consumes real Decode/Load, NewEncounter, Canvas.IsLineOfSightBlocked, AtlasFor,
OpenDoor/CloseDoor, ToData/LoadEncounter. No fake knowledge provider.

- [ ] Authored closed footprint door + opaque wall cuts one five-cell floor strip;
  ordinary non-concealed books stand on the far side. Assert geometry blocks the
  lane but player Atlas lacks far floor/books. Opening reveals; closing/reload
  retains learned floor. Second observer initially learns only its own side.
- [ ] Record the pre-change failure before implementing.

Verification: from encounter module, GOTOOLCHAIN=go1.24.1 go test ./dungeonspec
-run 'TestSingleRoomWallDoorSuite/TestOrdinaryDiscovery' -count=1.

## Task 2 — Lower builder floor into existing discovery regions

Owner: encounter/dungeonspec; runtime roomknowledge semantics unchanged.
Files: new partition_region.go/partition_region_test.go in encounter and
single_room_regions.go in dungeonspec; modify single_room_compile.go,
projection.go, placed_props.go, prop_observation.go; extend Task1 regressions and
world-builder-v4-site.compiled.json (its existing door separates the far cell).

Interfaces: private singleRoomRegions helper consumes the compiler's FieldInput
with its single implicit region and returns ordinary []encounter.RegionInput plus
cell→region ownership. No new source field or public DTO. Preserve the existing
implicit ID for the first canonical component; additional IDs derive from that
ID and the component's canonical minimum cell. Preserve region name, archetype,
lighting and absolute cells. A single connected component stays unchanged.

Provider: encounter.PartitionRegion(PartitionRegionInput{Field FieldInput,
Region RegionID}) → PartitionRegionOutput{Components [][]spatial.Position,
Footing []spatial.Position}, error.
It uses compileField, validated/deep-copied door records, compileCanvas and existing
spatial.Field. Components are ordered in canonical axial space but return cells in
FieldInput's authored frame. No actor, endings, world journal or new LOS algorithm.
The initial CompileOnlySetup approach was discarded: obtaining static geometry
must not require a fictitious scenario ending. The constructor-owned query is the
correct seam. Invalid geometry/doors/region returns zero output plus error.

Copied door records are closed for topology only; initial state does not rename or
merge rooms and the output field retains ORIGINAL door states. An opaque pillar's
own cell joins its single unambiguous adjoining component instead of becoming a
new room. Permanent opaque footing without one unambiguous adjoining component
is returned separately and compiled as existing FieldInput.Scenery, never a phantom
region or a bridge between rooms. Doors and sight-only blockers retain owned floor;
movable/reserved props do not define fixed topology. Actual source contributors,
flags and door states are untouched. No runtime repartitioning or second geometry.

The two-sided regression exposed a second gap: centre/nearest-cell standing support
is not adequate boundary observation support. footprintObservationCells consumes
existing spatial.PlacedCoverage over declared floor cells; off-floor shapes retain
existing nearest-floor support if overlap is empty. Holding/movement support and
concealment membership do not change. Structural wall/door presence may survive on
its OWN known coverage, as fixed segments survive at a known boundary; explicit
concealment still gates each identity independently. Footprint DoorSightings ask
existing sightReach against that coverage. No new floor is learned as a consequence.
This is cell-supported observation, NOT a universal subcell target-contact solver.

Monster/party Region references follow produced ownership. Party seats remain in
the region containing partyStart; otherwise a second party member could begin in
the supposed undiscovered room. Unrelated hand-authored region compilation stays
unchanged. Errors refuse compilation, not a fallback to whole-floor disclosure.

- [ ] Implement only after the Task1 red result.
- [ ] Prove stable IDs/cells across source ordering and initial door state; no
  mutation of input door declarations; open initial door still discovers normally.
- [ ] Prove transparent/nonblocking partitions do not invent opaque walls; opaque
  independent contributors remain effective after opening a door.
- [ ] Verify shared wall/door presentation survives at the discovery frontier;
  if support-cell projection fails, name that additional producer gap explicitly
  rather than broadening disclosure or modifying LOS arbitrarily.
- [ ] Existing RoomKnowledgeSuite stays green (whole-room semantics preserved).
- [ ] No new claims about LOS-clipped fixed props inside an already learned room:
  the R13/R14 contract remains; the operator's far-room props are withheld with it.

Verification: focused dungeonspec suites, full encounter go test -race ./..., go
vet ./..., normal lint/hook with isolated TMPDIR/cache, gofmt and git diff --check.

Provider checkpoint checks: original floor/prop/observer leak reproduced in
/tmp/ordinary-discovery-red.log; 2D wall disappearance reproduced in
/tmp/ordinary-discovery-2d.log; old nearest-cell door observation mutation red in
/tmp/ordinary-discovery-frontier-red.log. Full go test ./... passes after correction.
RoomKnowledgeSuite remains unchanged/green. Additional cases cover two observers,
second closed door, automatic secret-door discovery without seeing through it,
state/order stability and party/monster ownership. Full race/vet/lint run next.

## Task 3 — Adopt and exercise the delivered path

Prerequisite: pushed green encounter checkpoint, not merge/release. Session/API
consume actual Go-minted pseudo-versions; adapt only verified interface effects.
Use existing structural_patch_acceptance_test.go to assert fresh closed-door
snapshot, later reveal and replay/reload parity through the real SDK/wire path.
No proto/web visibility changes are expected; verify rather than infer that.

Live: fresh dedicated normal playthrough with an opaque divider and ordinary books
(no concealment). Verify undiscovered floor/placed identity absent; opening reveals;
closing/reload retains. Prop asset rendering is a separate already-open issue and
must not be claimed repaired by correct identity disclosure. Preserve operator data.
Fresh independent review after gates, then actual provider release pins before
consumer merge. All merges remain operator-authorized only.

## Plan checks

| Requirement | Task | Proof |
|---|---|---|
| Closed boundary hides unknown floor/ordinary props | 1,2,3 | geometry positive control + Atlas and wire absence |
| No explicit concealment workaround | 1,3 | empty concealments fixture |
| Opening reveals, closing remembers | 1,3 | verb transitions and save/load |
| Existing whole-room behavior stays intact | 2 | unchanged RoomKnowledgeSuite |
| Original door state/identity and source flags preserved | 2 | state/order/input immutability controls |
| Party does not spawn across a closed boundary | 2 | derived seat ownership assertions |
| Reads/events/replay agree | 3 | real integration + normal browser walk |

| Provider | Consumer | Contract | Availability/proof |
|---|---|---|---|
| spatial.Field + canonical encounter canvas | encounter.PartitionRegion → dungeonspec lowering | authored-frame components under existing adjacent LOS predicate | implemented; geometry positive control and pointy/flat frame tests |
| spatial.PlacedCoverage | structural boundary presence / footprint door observation | positive-area floor support, same sightReach and explicit membership gates | two-sided boundary regression; no standing/movement change |
| compiler Regions + ownership | encounter constructor/discovery | existing RegionInput and actor Region IDs | Task2 consistency tests |
| AtlasFor/RoomRevealed | SDK/API/web | unchanged permitted region/prop/structural records | Task3 integration required |

Review F1/F2 confirmed the opaque-centre risk at33a1ad17: a permanent phantom
region withheld footing and wall/prop identity from both sides. Parent live proof
also showed19→33 out of35 floor cells. The fix classifies permanent opaque boundary
footing as scenery, preserves explicit floor secrecy independently of ordinary
footing knowledge, and admits fixed opaque boundary props on their own known
coverage. New tests assert two rooms + one scenery cell (not a third region),
visibility from both sides, no far-room leak, stable order/IDs, blocked standing,
reload parity, explicit floor concealment independence and unchanged ownership
for doors/sight-only/movable props. Same-reviewer focused closure remains required.

Open risks: finer subcell observation remains outside this correction;
compile-time performance and diagnostic paths. These are not
permission to copy visibility into the client or concealment model. Runtime source
changes remain bounded to the existing compiler/knowledge seams and require proof.
