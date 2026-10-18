---
name: Sneaking — implementation plan
status: PLAN, derived 2026-10-18 from design.md (R1–R16) and checked against the inspected baseline below; supersedes the mad king plan's Task C1; **C-facing SHIPPED 2026-10-18** (protos v0.1.239, encounter v0.133.7, session v0.137.3, api dev 829242b1, web dev 172a7a35; walked); **C-sneak SHIPPED 2026-10-18** (protos v0.1.240, encounter v0.133.9, dnd5e v0.213.0, session v0.138.0, api dev d5c6dab8, web dev eeb3550e; walked); **C-zone SHIPPED 2026-10-18** (protos v0.1.241, encounter v0.134.0, session v0.139.0, api dev 2b6669ce, web dev 0b9d37e8; walked); **C-cone SHIPPED 2026-10-19** (encounter v0.135.0, session v0.140.0, api dev TBD; web unchanged; walked)
authority: design.md rulings R1–R16, settled by the operator 2026-10-18 (rpg-project#606, agreement on PR #616)
planning guide: ../../.agents/skills/design/planning.md
---

# Sneaking — implementation plan

## Goal and scope

A player declares sneaking in free roam; each watcher's red zone triggers one
Stealth roll per entry; a win keeps the sneaker out of that watcher's sight;
the sneaking player sees the zones as a red floor. Facing ships first, cones
last. Four slices, in order, one module per PR:

| slice | delivers | slice issue | closes |
|---|---|---|---|
| **C-facing** | members carry a facing; steps, targets and authored facing set it; `Seen` carries it; the web draws it | rpg-project#582 | #582 on F4 |
| **C-sneak** | the mode, the roll at the reach seam on entering, suppression, the `Snuck` beat, half pace, the kitchen-door routes | rpg-project#606 | rpg-toolkit#1020 on S1; #606 on the S-record PR, after S4 and the walk |
| **C-zone** | the red floor: watch zones on `GetView`, drawn by the web | rpg-project#619 | #619 on the Z-record PR, after Z4 and the walk |
| **C-cone** | a supplied arc per watcher narrows its zone | rpg-project#621 | #621 on the K-record PR, after K3 and the walk |

C-sneak is independent of facing: its zone before cones is `reachesCell`
alone, and only C-cone reads facing.

**Non-goals (R16):** footfall sounds; surprise costing a turn; a monster that
sneaks; the drop arm stopping a walk; sneaking on the turn clock.

## Finish line

Every slice: its PRs merged inside-out on released tags; its one-off
studio-authored site in the encounter module's testdata with ONE scenario
test green; the walk on the castle stack (web :3030, api :8115) with the QA
checklist posted on the slice issue as `- [ ]` boxes and ticked from the walk;
the record (this plan's task marked shipped with versions, the mad king use
case's route rows updated). C-sneak additionally lands the kitchen-door
route assertions on the castle content (use case §3); #606 closes on the
S-record PR, after the walk.
Operator-owned: the walk sign-off on
each slice issue.

## Constraints

- One nearest-`go.mod` module per toolkit PR; develop outside-in on pushed
  pseudo-versions; protos merge first; inside-out merge only after the walk;
  CI mints tags (`rpg-toolkit/CLAUDE.md`).
- Protos additive, `[deprecated = true]` never in-place; `buf format -w`,
  `buf lint`, `buf breaking` against main (`rpg-api-protos/CLAUDE.md`).
- Zero castle names in non-test Go.
- The encounter never compares a Stealth total, never halves a speed and
  never rolls (C1, C8; design R8, R10).
- Fog-of-war law stands: one sight test (its R5), per seat, no client
  visibility.
- Toolkit/api/web PRs on pseudo-versions open draft; protos and this
  repository's PRs open ready.

## Inspected baseline (2026-10-18)

| repo / module | revision |
|---|---|
| rpg-toolkit main | 71282ee5 |
| `rulebooks/dnd5e/encounter` | v0.133.6 |
| `rulebooks/dnd5e/session` | v0.137.2 |
| rpg-api-protos main | 1353bb7f (v0.1.238) |
| rpg-api dev | b2a5f601 |
| rpg-dnd5e-web dev | 7212ec48 |

Inspected seams: `sight.go` (`Sight`; its doc names the stealth answer's home
beside reach in `rebuildPercepts`), `sightreach.go` (`sightReach.Reaches`,
`reachesCell`, `newSightReach`), `encounter.go` (`rebuildPercepts`),
`trigger.go` (`classify`, the drop arm), `conceal.go` (`CheckResolver`,
`ResolveCheckInput`/`Output`), `discovery.go` (`sweepDiscoveryChecks`,
`BeatDiscoveryChecked`, persisted attempts), `sheets.go` (`SheetFacts`),
`worldtime.go` (pace from `SpeedFeet`), `testimony.go` (`SightTestimony`),
`field.go` (`MemberInput`, `JoinInput`, `memberRecord`), `dungeonspec/compile.go`
(`MonsterPlacement.Facing`, `StartFacing`); session `conceal.go`
(`checkSeam`, `resolveStagedCheckPoseable`), `exploration.go` (every player
staged as a discovery observer), `sheets.go`, `launch.go`, `capabilities.go`;
protos `types.proto` (`Sighting`, `Seen`), `service.proto` (`GetViewResponse`),
`events.proto` (`EVENT_KIND_HEARD` 46, `EVENT_KIND_CELLS_REVEALED` 47,
`DiscoveryChecked`, `RollCalculation`); web `hex-grid/HexEntity.tsx`, `hex-grid/useEntityFacing.ts`,
`hex-grid/facingYaw.ts` (`facingToYaw`).

## Sequence

```
F0 protos ─▶ F1 encounter ─▶ F2 session ─▶ F3 api ─▶ F4 web ─▶ F-walk ─▶ F-record
S0 protos ─▶ S1 encounter ─▶ S2 session ─▶ S3 api ─▶ S4 web ─▶ S-walk ─▶ S-record
Z0 protos ─▶ Z1 encounter ─▶ Z2 session ─▶ Z3 api ─▶ Z4 web ─▶ Z-walk ─▶ Z-record
K1 encounter ─▶ K2 session ─▶ K-walk ─▶ K-record   (K3 studio field deferred)
```

S1 develops on the encounter tag current when it starts; it does not need
F1. If F1 and S1 are in flight together, whichever merges second catches up
by a commit on top, because both touch the persisted member shape; that is a
merge order, not a contract. Z1 needs S1 (the zone and the mode). K1 needs F1
(facing) and Z1 (the zone function it narrows). Merge order in
each slice: protos → encounter → session → api → web. No slice shares a
branch with another; each catches up by a commit on top of the merged tag.

## Open items

| item | state |
|---|---|
| R6–R16 agreed on the design PR | settled (operator, 2026-10-18); nothing blocked |
| **Risk: the castle has no watcher on the kitchen route.** The keep knights pace inside the throne room, every door is shut, and no creature opens a door (rpg-toolkit#2069), so a shut throne door hides the party from them with no sneaking at all. | S1's content step posts a watcher on the route (below); the sneak-past assertion is written to fail without sneaking |
| **Content change: the inner kitchen door is locked.** It is not locked today; the use case's forced door needs a lock. Its DC and approaches are content values, proposed in S1's PR; the operator may veto. Every route crossing that door is re-run. | S1 |
| **Content change: a watcher crosses the great hall.** `hall-1` walks a patrol leg across the hall between the kitchen door and the throne door (or a keep-patrol leg is moved there); the cells are measured on the file in S1. | S1 |
| **Risk: the forced door's crash may not reach a listener with a `heard` row.** At the shipped numbers a crash is DC 5, plus 5 per region hop, plus one hop for a shut door. A guard with passive 12 hears at DC 12 or less, so only at one hop or less; a knight with passive 10 hears only at one hop. The knights sit two shut doors away. Whether any listener with a `heard` row hears it is unverified until S1 measures it on the file; the fix, if any, is content (the listener, its row or its post), never Go. | S1 measures; the heard route's assertion is written only after |
| An unset authored facing is a rules bearing of 180 | law (design R13); F4 pins the web's default and the wire agree |
| Half speed rounds through `CellsFromFeet` | no rounding of its own in session |
| The session's seeds: arc 120 for a monster, re-arm 2 cells | session constants until content states them (design O6) |

---

## C-facing (rpg-project#582)

### Task F0: protos — facing on a sighting

**Delivers:** R13 (wire half).
**Owner:** rpg-api-protos, `dnd5e/api/session/v1alpha1/types.proto`.
**Prerequisites:** none.
**Files:** `types.proto` `message Seen` (sight-channel facts live on `Seen`, not `Sighting`: rpg-toolkit ADR-0041 and `Seen.standing`'s own doc).
**Interfaces:** `optional float facing_degrees = 4;` with the doc: bearing in
degrees, 0 is map north, clockwise, `[0, 360)`; absent means the testimony
predates facing; a memory keeps the facing it last saw, as `equipment` does.
**Behavior:** additive only.
**Tests:** none hand-written (no hand-written tests in protos).
- [ ] Add the field and doc; `buf format -w`; `buf lint`; `buf breaking --against main`; `buf generate` compiles.
**Verification:** the four buf commands green in the protos repo root.
**Completion evidence:** merged PR, released tag.
**SHIPPED 2026-10-18:** rpg-api-protos#397 → main d39a2bc6, tag v0.1.239 (gen 585633e6). `Seen.facing_degrees` is a DOUBLE. Frame: 0 is north, which is −Y of the footprint plane; 90 is +X; clockwise. A prop's angle is (facing − 90) mod 360.

### Task F1: encounter — facing as member state and testimony

**Delivers:** R4, R13.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter` (includes `dungeonspec`).
**Prerequisites:** none.
**Files:** `field.go` (`MemberInput.Facing *float64`, `JoinInput.Facing *float64`, `memberRecord.Facing float64`, `Member.Facing`), `data.go` (member data `facing`, `*float64`, nil on load seeds 180), `facing.go` (new: `FacingFromCompass(word string) (float64, error)` for the eight words n=0 … nw=315, and `bearing(from, to spatial.Position) float64` through the field's `Orientation` to map space), `step.go` (`Encounter.Step` writes the step's bearing), the table driver's walk end in `tabledriver.go` and the turn driver's attack intent in `turndriver.go` (face the intent's target member), `outcome.go` record path for an attack (`RecordTrain`: attacker faces target), `testimony.go` (`SightTestimony.Facing *float64`), `encounter.go` (`rebuildPercepts` snapshots facing), `dungeonspec/compile.go` (placement `Facing` lowered through `FacingFromCompass` into `MemberInput.Facing`; `StartFacing` exposed for the session).
**Interfaces:** `Member.Facing float64`; `SightTestimony.Facing *float64`; `FacingFromCompass` exported because content and the session need the one conversion.
**Behavior:** a member seeded with no facing faces 180; each step sets the bearing from the old cell to the new; a walk whose intent names a target member ends facing that member's cell; an attack record turns the attacker toward its target; an unknown compass word is refused at the dialect by path; facing persists and round-trips.
**Tests:**
- `dungeonspec/testdata/facing-site.yaml` (one-off studio-authored site: one room, a patrol knight on a four-waypoint path with authored `facing: e`, a guard with `toward: enemy`) and one scenario test `TestFacingSite`: the knight starts at 90; after one world round its facing equals the bearing of its first patrol step; a player walks into the guard's sight, the guard walks and stops adjacent, its facing equals the bearing to the player; the player's own sighting of the knight carries the knight's facing; a save and load keeps all three.
- `facing_test.go`: the six axial step directions under pointy and flat orientation give the six expected bearings (table); `FacingFromCompass` for all eight words and a refusal.
- [ ] Write the tests; run `go test ./ -run 'Facing' -count=1`; confirm failure.
- [ ] Implement; run `go test ./... -count=1`; goldens unchanged except fixtures that now carry facing.
**Verification:** `go test ./... -count=1` and `-race` on the castle suite in `rulebooks/dnd5e/encounter`.
**Completion evidence:** green run, PR, pseudo-version for F2.
**SHIPPED 2026-10-18:** rpg-toolkit#2073 → main 994dbba5 = encounter v0.133.7. `dungeonspec/testdata/facing-site.yaml` and `TestFacingSite` as planned. On the castle, keep-1 ends its first round at (5,-20) facing 150. Sighted beats now name every subject the rebuild changed. Rulings made here are in design.md R17–R20.

### Task F2: session — authored facing reaches play

**Delivers:** R13 (session half).
**Owner:** rpg-toolkit `rulebooks/dnd5e/session`.
**Prerequisites:** F1 pseudo-version.
**Files:** `launch.go` (placement `Facing` into `MemberInput.Facing` via the compiled member input; players seated with `JoinInput.Facing` from `StartFacing`), the sighting projection in `convert.go` (carry `SightTestimony.Facing` onto the session's `Seen`), `go.mod` encounter pin.
**Interfaces:** the session's `Seen` gains `Facing *float64`.
**Behavior:** a launched castle's knights start at their authored facing; testimony without a facing projects as absent.
**Tests:** `launch_test.go`: a placement with `facing: w` launches a member facing 270, and its sighting's `Seen` carries 270.
- [ ] Test, implement, `go test ./... -count=1`.
**Verification:** module tests green.
**Completion evidence:** PR, pseudo-version for F3.
**SHIPPED 2026-10-18:** rpg-toolkit#2075 → main 9477d715 = session v0.137.3.

### Task F3: rpg-api — convert facing

**Delivers:** R13 (api half).
**Owner:** rpg-api session v1alpha1 convert.
**Prerequisites:** F0 tag, F2 pseudo-version.
**Files:** the `Seen` conversion in the session handler's `convert.go`; `go.mod` pins.
**Behavior:** `Seen.facing_degrees` set when the toolkit `Seen` has a facing; unset otherwise.
**Tests:** convert table test: present → set, absent → unset.
**Verification:** `make pre-commit` (or the repo's gate) green.
**Completion evidence:** PR (draft until tags), pseudo-version pins replaced at merge.
**SHIPPED 2026-10-18:** rpg-api#1116 → dev 829242b1.

### Task F4: web — draw the wire's facing

**Delivers:** R13 (web half); closes #582.
**Owner:** rpg-dnd5e-web `src/components/hex-grid`.
**Prerequisites:** F0 bindings, F3 on the stack.
**Files:** `facingYaw.ts` (add `degreesToYaw(deg: number): number`, composing the existing table-angle step); `HexEntity.tsx` is the gate: it seeds and requests the heading from `seen.facingDegrees` when present, and passes the move-path heading from `useHexMovePath` only when absent (in place of `DEFAULT_HEADING_BY_TYPE` as the seed). `useEntityFacing.ts` is kept unchanged: it owns `rotation.y` and accepts a requested heading from any source.
**Behavior:** an entity whose `Seen` has a facing draws that facing; the studio and play agree for the eight compass words.
**Tests:** `facing.test.ts`: for each compass word, `degreesToYaw` of its degrees equals `facingToYaw(word)`; a `HexEntity` test (or a small heading selector beside it): with `seen.facingDegrees: 90` the requested heading is east and the move path's heading is ignored; with none, the move path's heading is used.
**Verification:** `npm run typecheck && npm test` green; screenshot of the facing site in play showing the knight facing east.
**Completion evidence:** PR body `Closes KirkDiggler/rpg-project#582`.
**SHIPPED 2026-10-18:** rpg-dnd5e-web#1298 → dev 172a7a35.

### F-walk and F-record

Walk the castle: the keep knights face along their patrol; a guard that stops
faces the player; reload keeps facings. QA checklist on #582. Record: mark F0–F4
shipped here with versions.

**SHIPPED 2026-10-18.** The castle stack runs on dev: api 829242b1, web 172a7a35. Two walks on the castle stack:
- **Wire:** the guards face their last step, and six steps matched independent bearing math. The captain stops facing the hero at 150 and 210. A reload keeps the facings.
- **Draw:** every drawn yaw equals `degreesToYaw` of the wire's facing.

Filed from the walks:
- rpg-api-protos#398: the viewer's own facing is not on the wire. The local player keeps the 2026-07-24 staging pose π, and the monster default is pinned to 180.
- rpg-toolkit#2074: a downed member still gets sighted beats.
- rpg-toolkit#1892: the v2 multi-room dialect refuses monster facing.

The final walk ran on the tagged heads and passed. The three gatehouse guards face 180. The captain stops at (-5,9) facing the hero at 210. A guard striking in place turns from 180 to 90, toward the hero. A reload keeps all of them. Two findings, filed as rpg-toolkit#2076:
- An opportunity attack looked as if it did not turn the striker. It does: the attack is recorded through `RecordTrain`, whose landing calls `faceHeld`. The stale facing on the walk came from the train closing the fight, after which no sight refresh runs (design R21). Open rpg-toolkit#2077 pins the turn.
- A walk `toward: sound` ends keeping its last step's facing rather than facing the sound's cell. Open rpg-toolkit#2077 carries the fix.

---

## C-sneak (rpg-project#606)

### Task S0: protos — the verb, the mode on the view, the beat

**Delivers:** R6, R12 (wire half).
**Owner:** rpg-api-protos.
**Files:** `service.proto` (`rpc SetSneaking(SetSneakingRequest) returns (SetSneakingResponse)`; request `{session_id, member, sneaking bool}`, response `{sneaking bool}`; `GetViewResponse.sneaking bool = 6`), `events.proto` (`EVENT_KIND_SNUCK = 48`; `message Snuck { string member = 1; string watcher = 2; int32 dc = 3; int32 total = 4; bool beaten = 5; RollCalculation calculation = 6; }` with doc: recipient-scoped to the mover; `watcher` empty when the mover does not see it; the oneof gains `Snuck snuck = <next free>`).
**Tests:** buf lint/format/breaking/generate.
**Completion evidence:** merged PR, released tag.
**SHIPPED 2026-10-18:** rpg-api-protos#399 → main 8c519a81, tag v0.1.240 (gen b52674c7).

### Task S1: encounter — mode, judgement, suppression, beat, kitchen-door routes

**Delivers:** R1–R3, R6, R7, R9–R12, R16; closes rpg-toolkit#1020 and rpg-project#606.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** R6–R16 agreed. Not F1 (see Sequence).
**Files:**
- `sneak.go` (new): `SetSneakingInput{Member MemberID; Sneaking bool}`, `SetSneakingOutput{Sneaking bool}`, `Encounter.SetSneaking`, `Encounter.Sneaking(member) bool`; `BeatSnuck = "snuck"`; `watchZone(watcher MemberID, reach sightReach, arc int) map[spatial.Position]bool` (arc 0 = 360; C-cone fills the non-zero branch); `judgeSneakers(reach sightReach, observers []MemberID, at uint64) error`; `endSneaking(member MemberID)`.
- `field.go` / `data.go`: `memberRecord.Sneaking bool`; `EncounterData.SneakVerdicts map[MemberID]map[MemberID]SneakVerdictData` with `SneakVerdictData{Beaten bool; Armed bool; InZone bool}`, discovery's `{Used, Armed}` shape (persisted; validated: both ids members, mover a player). `InZone` is the last refresh's answer, so a crossing is `!InZone && nowInZone`.
- `sheets.go`: `SheetFacts.WatchResetCells int` (zero = re-arm on the first refresh outside; negative refused). `PassivePerception` doc: the DC for hearing and for a sneaker's Stealth.
- `sightreach.go`: `sightReach` gains `sneaking map[MemberID]bool` and `suppressed map[MemberID]map[MemberID]bool`; `Reaches` answers false when `subject` is sneaking and `observer`→`subject` is suppressed or the subject's cell is outside the observer's zone. `reachesCell` is unchanged (zones, fog and discovery keep asking it).
- `encounter.go` `rebuildPercepts`: after `geometry := e.newSightReach(...)` and BEFORE `e.appendMutablePresences(presences, geometry)` (which copies the reach into `observationReach`, prop_observation.go, and that copy is what `intelLog.Observe` is handed), call `judgeSneakers(geometry, observerIDs, clockReading)` and set `sneaking` and `suppressed` on `geometry`. Judgement covers only pairs whose watcher is in this pass's observers; a single-observer pass (the concealment reveal in `conceal.go`) leaves every other pair untouched.
- `SetSneaking` spends no time and then calls `refreshSight(e.rosterIDs())`, so a declaration, and stopping inside a zone, resolve at once.
- Verbs that end the mode: `Speak` (`option.go`), `RecordCast` (`cast.go`), the attack record (`RecordTrain`), `Exit`, and `formation` in `trigger.go`: every engaged member's mode is cleared before the bubble's first turn budget reads `SheetFacts.SpeedFeet` (`clocks.go` seeds `MovementFeet` from it).
- `conceal.go` doc: `ResolveDiscoveryCheck` decides any automatic attempt the composition triggers, a sneak roll included.
- `dungeonspec/testdata/sneak-site.yaml` (one-off studio-authored site: two rooms, a doorway, a knight with passive 10 pacing the far room, a civil cook in the near room).
- `dungeonspec/testdata/world-builder-v4-castle.yaml`, a content change stated in the PR: LOCK the inner kitchen door (the use case's forced door; DC and approaches proposed in the PR); give `hall-1` (or a keep-patrol leg) a patrol path across the great hall between the kitchen door and the throne door, cells measured on the file; header lines for the two kitchen-door routes. Re-run every `TestMadKing*` route and record any that the lock or the post changes.
- `dungeonspec/castle_king_kitchen_test.go` (new).
**Interfaces:** `CheckResolver.ResolveDiscoveryCheck(&ResolveCheckInput{Member: mover, Approaches: []CheckApproach{{Ability: "stealth", DC: max(passive, 1)}}})`. Beat payload `{"beat":"snuck","member":…, "watcher":… (omitted unless the mover holds it live), "dc":…, "total":…, "beaten":…, "calculation":…}`, audience `[mover]`, tag `snuck`.
**Behavior:**
- `SetSneaking` refuses a non-player (`ErrNotMember`), a member in a bubble (`ErrNotFreeRoam`, new, or the existing turn-clock refusal if one fits), a closed run (`ErrClosed`), and an encounter whose `checkResolver` is nil (`ErrNoCheckResolver`). It spends no time and refreshes sight. Declaring the mode already held (on while on, off while off) returns the current mode and does nothing else: no roll, no re-arm, no refresh, no beat.
- On declaring: each watcher that holds the mover live (`CurrentVia` non-empty) or whose zone holds the mover's cell gets `{Beaten: false, Armed: false, InZone: <the zone's answer for the mover's cell>}` with no roll and no beat.
- `Reaches` for a sneaking subject: a beaten pair → false; a lost pair → ordinary geometry (`reachesCell`), never the zone; otherwise the zone.
- `judgeSneakers`, for each sneaking placed player (sorted) and each non-player watcher in the pass (sorted): a crossing into an armed (or verdict-less) pair → resolve, store `{Beaten, Armed: false, InZone: true}` (true because the crossing just found the cell inside), beat; every pass sets `InZone` from the zone's answer, never a constant; `Armed: true` once the mover's distance to the nearest zone cell is at least the watcher's `WatchResetCells` and, for a lost pair, `reachesCell(watcher, mover)` is also false.
- A resolver error aborts the verb atomically (R5 of the encounter).
- Ending the mode clears every verdict the mover holds.
**Tests:**
- `TestSneakSite` (the one scenario test, on `sneak-site.yaml`, scripted resolver): the rogue declares sneaking out of the knight's sight and walks through the doorway; one `snuck` beat, audience the rogue alone, names the knight; the knight's holdings never gain the rogue; no bubble forms; the walk continues to its end; the rogue leaves past the re-arm distance and re-enters, and a second beat lands; the resolver is asked exactly twice over the whole walk. Variant in the same test: a scripted loss forms the fight with the existing surprise label, and the rogue's first turn budget is full speed; a loss against the cook forms no fight and the cook holds the rogue.
- `sneak_test.go` focused cases: `SetSneaking` refusals (four); declaring in the cook's live sight gives a lost verdict, no roll, no beat, and the cook keeps holding the rogue (I1: nobody vanishes in plain view); declaring on while on, and off while off, changes no verdict, asks the resolver nothing, writes no beat and runs no refresh; declaring off inside the knight's zone after a won roll makes the knight hold the rogue in the same call (the refresh); attack, cast, speak and formation each clear the mode and its verdicts; a non-sneaking mover in the same doorway is seen exactly as before (R2); a watcher whose prior holding of the rogue is a MEMORY (a ghost from an earlier sighting) keeps that ghost after a won roll, never removed (R11); a knight whose zone edge paces over a still rogue rolls once, not once per step, until the rogue stands the re-arm distance away; a watcher the rogue cannot see gets a beat with no `watcher`; save and load between two steps does not re-roll; suppression asserted through the real `rebuildPercepts`, not a hand-built `sightReach`; a single-observer pass leaves another watcher's pair unchanged.
- `castle_king_kitchen_test.go`:
  - `TestMadKingKitchenDoorSneaksPastTheHall`: every party member declares sneaking in the kitchen out of every watcher's sight, and the resolver is scripted one roll per (member, watcher) that the path crosses (a +7 rogue with a 15, and so on, against the hall watcher's passive). The party walks from the kitchen door across the great hall to the throne door. The hall watcher's holdings never gain a party member, no bubble forms, and the party stands at the throne door. **Mutation the test must kill:** the same path with no `SetSneaking`, or with `judgeSneakers` returning without suppressing, forms the fight with the hall watcher; the test runs that variant and asserts the bubble.
  - `TestMadKingKitchenDoorHeard`: written only after S1 measures the crash's reach (Open items). Shape if reachable: the fighter forces the locked inner door; the crash lands on the listeners the measurement names; that listener's `heard` row walks it toward the door; the sneaking party is crossed into its zone; a scripted 2 loses; its `enemy: seen` row shouts; `alarm-raised` lands on hearers; reserves arrive. If the measurement shows no listener can hear it, the PR says so and the route's content fix goes to the operator.
- [ ] Tests first; `go test ./ -run 'Sneak' -count=1` fails; `go test ./dungeonspec -run 'MadKingKitchen' -count=1` fails.
- [ ] Implement; full `go test ./... -count=1`; castle suite with `-race`.
**Verification:** as above, in `rulebooks/dnd5e/encounter`.
**Completion evidence:** PR body `Closes KirkDiggler/rpg-toolkit#1020` and `Part of KirkDiggler/rpg-project#606`.
**SHIPPED 2026-10-18:** rpg-toolkit#2078 → main 30e6e87c = encounter v0.133.9; closes rpg-toolkit#1020.
- **Castle content:** the kitchen door into the hall is LOCKED. Thieves' tools, Dex DC 13, pick it silently; Str DC 15 forces it with a crash. `hall-1` walks the `hall-patrol` loop (-2,-12) → (6,-12) → (6,-15) → (1,-15), between the kitchen door and the throne door.
- **Tests:** the one-off studio-exported `testdata/sneak-site.yaml` with `TestSneakSite`; `TestMadKingKitchenDoorSneaksPastTheHall`; `TestMadKingTheKitchenDoorsCrashReachesOnlyTheHall`.
- **The crash, measured:** it reaches the hall at DC 10 and the throne room at DC 20. `TestMadKingKitchenDoorHeard` is NOT written; the route's content fix is the operator's.
- The `CheckResolver` is now kept whenever it is supplied. The lost-pair re-arm mutant survives until cones (C-cone).
- `audienceOf` moved onto the sight reach, so a sight-obscuring area now screens a deed's witnesses as it screens a sighting (design R26).
- Rulings made here are in design.md R22–R26.

### Task S2: session — the verb, staging, half pace, the event

**Delivers:** R6, R8, R10 (roll half), R12 (stream half).
**Owner:** rpg-toolkit `rulebooks/dnd5e/session`.
**Prerequisites:** S1 pseudo-version.
**Files:** `sneak.go` (new: `SetSneakingInput{Member string; Sneaking bool}`, `SetSneakingOutput{Sneaking bool}`, `Manager.SetSneaking`), the write scope's staging in `exploration.go` (stage every sneaking player for checks on every write verb, beside the discovery observers), `sheets.go` (`SpeedFeet` halved when `enc.Sneaking(id)`; `WatchResetCells` seeded 2 for a monster, 0 for a character), `view.go`/`projectView` (`ViewOutput.Sneaking`), events (`EventSnuck`, `SnuckBody`), `go.mod` pin.
**Behavior:** the roll runs through `checkSeam.ResolveDiscoveryCheck` on `m.dice`; a verb by player B that walks a knight into sneaking player A rolls for A with A's staged record; half speed only while sneaking.
**Tests:** `sneak_test.go`: the verb round-trips and persists; a sneaking player's world-clock pace is half (cells per round); a monster's walk on another player's verb rolls for the sneaker (resolver asked with the sneaker's id); the `Snuck` event reaches the sneaker's stream only.
**Verification:** `go test ./... -count=1` in the module.
**Completion evidence:** PR, pseudo-version for S3.
**SHIPPED 2026-10-18:** rpg-toolkit#2079 → main 426cc58d = session v0.138.0. dnd5e moves to v0.213.0 for monsters' passive Perception. The standing seam gains a live slot. Staging was already universal.

### Task S3: rpg-api — the handler and the conversions

**Delivers:** R6, R12 (api half).
**Owner:** rpg-api.
**Prerequisites:** S0 tag, S2 pseudo-version.
**Files:** the session v1alpha1 handler (`SetSneaking`, caller bound as in `SetDiscoverySharing`), `convert.go` (`GetViewResponse.sneaking`, `Event.snuck`), pins.
**Tests:** handler test: a caller cannot set another member's mode; convert test for `Snuck` with and without a watcher.
**Verification:** the repo's pre-commit gate.
**SHIPPED 2026-10-18:** rpg-api#1117 → dev d5c6dab8. `role_policy` gains `SetSneaking`. It also copies S1's castle file onto the shelf (`content/the-mad-king.yaml`, without the routes paragraph), so the lock and `hall-1`'s patrol are served; without that copy, S1's content change does not reach play.

### Task S4: web — the toggle and the roll

**Delivers:** R1, R5 (web half).
**Owner:** rpg-dnd5e-web.
**Prerequisites:** S0 bindings, S3 on the stack.
**Files:** a `SneakToggle` in the free-roam action bar (hidden while in a fight), the story renderer for `snuck` (the roll as a seen roll with its calculation; "something is watching" when `watcher` is empty), the session event switch.
**Tests:** toggle sends `SetSneaking` and reflects `GetViewResponse.sneaking`; story line for both `Snuck` shapes.
**Verification:** typecheck, tests, screenshot.
**SHIPPED 2026-10-18:** rpg-dnd5e-web#1299 → dev eeb3550e. The toggle sits beside Short rest; the eyebrow reads "Sneaking"; the roll shows in the toasts; spotted uses the danger tone.

### S-walk and S-record

Walk the kitchen door on the castle stack: sneak across the hall past its
watcher; open the throne door and be judged inside the throne room as the
knights' zones cross you; force the inner door and see what hears it; stop
sneaking in a watcher's zone and be seen at once; declare sneaking in plain
view of the cook and stay seen. QA checklist on #606. Record (an rpg-project
PR): mark S0–S4 shipped; the use case's kitchen-door rows asserted or their
measured gap stated; its body carries `Closes KirkDiggler/rpg-project#606`.

**SHIPPED 2026-10-18.** The castle stack runs on dev: api d5c6dab8, web eeb3550e. Walked on the castle stack (api s3-sneak beb07922 + web s4-sneak 8579a85d): the Sneak toggle reflects the server and is absent in a fight; half pace measured 3 cells per world tick against 6; crossing the great hall sneaking gave one Snuck beat against hall-1 (dc 12 — the guard template's passive — total 14, beaten) and no sighting or fight; standing inside the zone rolled nothing more, leaving past the re-arm distance and re-entering rolled once more; opening the throne door sneaking rolled against keep-1, keep-2 and the king (all won) and formed no fight until sneaking was turned off; Speak cleared the mode. Not walked: a loss against hall-1 (both rolls won), and forcing the door — the wire has no way to choose the Strength approach and the server applied dex + thieves' tools to a fighter with no tools (filed). The castle's kitchen door opens onto the hall, not the yard, so the use case's yard-side kitchen route is a content question for the operator (noted on #606). A re-walk on api f8588843 (encounter 2efd78b8) confirmed the witness fix as far as the wire shows: drawing a rapier inside hall-1's sight after a won roll drew no reaction, and a spoken line to the captain landed and ended sneaking; a Speak refused for no option keeps the mode (R7).

Filed from the walk: rpg-toolkit#2080 (Unlock applies the tool approach regardless of the picker's tools; no approach on the wire — rpg-api-protos#396 is the wire half; even the proficient rogue's picks add no tool proficiency), and the kitchen-door content question on rpg-project#606 (the castle's kitchen door opens onto the hall; the use case's yard-side kitchen door does not exist).

Kitchen-door rows: "Kitchen door" is asserted (`TestMadKingKitchenDoorSneaksPastTheHall`). For "Kitchen door, heard", the gap is measured: the forced door's crash reaches only the hall (DC 10; the throne room is DC 20), and the route's content fix is the operator's.

---

## C-zone (rpg-project#619)

### Task Z0: protos — watch zones on the view

**Delivers:** R15 (wire half).
**Files:** `service.proto`: `message WatchZone { string watcher = 1; repeated Position cells = 2; }`; `GetViewResponse.watch_zones = 7` with the doc: only while the viewer sneaks; one per watcher the viewer sees now; cells a subset of the viewer's atlas cells, sorted as `visible_cells`.
**Tests:** buf checks.
**SHIPPED 2026-10-18:** rpg-api-protos#400 → main e02cd37d, tag v0.1.241 (gen 84717fe).

### Task Z1: encounter — `WatchZonesFor`

**Delivers:** R9, R15.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** S1.
**Files:** `watchzones.go` (new: `WatchZonesInput{Member}`, `WatchZonesOutput{Zones []WatchZone}`, `WatchZone{Watcher MemberID; Cells []spatial.Position}`, `Encounter.WatchZonesFor`), reusing `newSightReach` and `watchZone` from S1; `dungeonspec/testdata/zone-site.yaml`.
**Behavior:** empty when the viewer does not sneak or is unplaced; one zone per watcher whose holding by the viewer has `CurrentVia` non-empty; cells ∩ the viewer's known cells minus cells concealed from the viewer; sorted by the atlas comparator.
**Tests:** `TestZoneSite` (one scenario): a sneaking viewer sees one knight and not a second behind a wall; one zone, the first knight's; no zone cell outside the viewer's known cells; not sneaking → no zones; the knight walks and the zone moves with it.
**Verification:** module tests.
**SHIPPED 2026-10-18:** rpg-toolkit#2081 → main b6caffca = encounter v0.134.0.
- **Shape:** `Encounter.WatchZonesFor(WatchZonesInput{Member})` returns `WatchZonesOutput{Zones []WatchZone{Watcher MemberID; Cells []spatial.Position}}`. The watchers are `watchersIn` (placed non-players) filtered by `holdsLive`. The cells are `placedSightReach().inWatchZone`, the same geometry `judgeSneakers` rolls with, ∩ the viewer's known cells − hidden cells. Cells come in atlas order, zones in watcher-id order, and the list is never nil.
- **Site:** the one-off `dungeonspec/testdata/zone-site.yaml` (a sentry knight pacing the rogue's room, a warden knight behind the wall) with `TestZoneSite` in `dungeonspec/zone_site_test.go`. One zone, the sentry's, and none for the warden. Every zone cell is known to the rogue. Far-room floor the sentry sees through the doorway but the rogue never saw is left out. The zone moves as the sentry walks. Not sneaking gives none.
- **Castle:** `TestMadKingTheHallGuardsRedFloor` in `castle_king_zone_test.go`. The hero sneaks from the kitchen into the hall; Stealth is rolled at DC 10 on entry and won (a Beaten verdict against `hall-1`). Every zone's watcher is held live, and `hall-1`'s zone holds the hero's cell. Sneak off gives none. The castle fixture is unchanged.
- **Mutants killed:** the live filter, the known cut, the sneaking guard, the zone predicate, the beaten-watcher drop. The concealment-cut mutant survives, because the site has no concealment; accepted for this slice.

### Task Z2: session — project the zones

**Files:** `projectView` (`ViewOutput.WatchZones` unchanged pass-through), pin.
**Tests:** a view of a sneaking member carries the encounter's zones byte-equal.
**SHIPPED 2026-10-18:** rpg-toolkit#2082 → main d16aee58 = session v0.139.0. `ViewOutput.WatchZones []session.WatchZone{Watcher string; Cells []spatial.Position}` (json `watch_zones`) is a field-for-field copy, not the planned pass-through: `encounter.WatchZone` never crosses the boundary (`TestNoInnerTypeCrossesTheBoundary` refused it; the allow-list is untouched). The ruling is design R27.

### Task Z3: rpg-api — convert zones

**Files:** `convert.go` for `GetViewResponse.watch_zones`. **Tests:** convert table test.
**SHIPPED 2026-10-18:** rpg-api#1118 → dev 2b6669ce. `watchZonesToProto` in `convert.go` goes through the same `positionToProto` as `visible_cells` and is wired in `viewToProto` (`get_view.go`). The table test carries a literal x/y assertion, which catches a swap made in both converters, and a zero-cell zone. The castle shelf is unchanged, verified equal to the fixture minus the routes paragraph. Pins: protos v0.1.241, encounter v0.134.0, session v0.139.0.

### Task Z4: web — the red floor

**Delivers:** R5, R15 (web half).
**Files:** `WatchZoneFloor` in the session scene, fed from the view's `watchZones`, drawn over the fog-of-war floor state; no zone math.
**Tests:** a view with two zones tints exactly their cells; an empty list tints nothing.
**Verification:** typecheck, tests, screenshot of the castle keep with the floor red at the knights' reach.
**Completion evidence:** PR body closes the slice issue.
**SHIPPED 2026-10-18:** rpg-dnd5e-web#1300 → dev 0b9d37e8. The planned `WatchZoneFloor` component does not exist; the floor shipped as a tint inside `SyntyHexFloor`. `watchZoneCells.ts` unions the sent cells through the same reader as fog (x=q, y=r via `positionToCube`) into `Scene3D.watchZoneCells`. `SyntyHexFloor` tints a tile toward one red over its fog colour: seen red, remembered a darker red. `SeatKnowledgeProbe` exposes `data-watch-zone-cells`. No zone math, no sneaking gate, no per-watcher colour; protos pin v0.1.241. The slice issue closes on the Z-record PR, after the walk.

### Z-walk and Z-record

Walk: sneak into the keep and see the knights' zones move with their patrol;
stop sneaking and the floor clears. QA checklist on the slice issue.

**SHIPPED 2026-10-18.** The castle stack runs on dev: api 2b6669ce, web 0b9d37e8. Walked on the castle stack (api z3-zones 39951923 + web z4-zones 11f07229); the checklist is ticked on #619 ([walk comment](https://github.com/KirkDiggler/rpg-project/issues/619#issuecomment-6247627767)):
- **Hall:** only `hall-1`'s zone, holding the hero's cell. One Snuck beat (DC 12, total 12, beaten).
- **Kitchen:** zones for the cook, the scullion and `hall-1`, who sees in through the open kitchen door.
- **Wire contract:** every zone cell is in GetAtlas, and the cells are sorted as `visible_cells`. Sneak off gives none. A held-but-not-current watcher (`gate-1`, seen from the courtyard) gets no zone (R15).
- **Patrol:** one patrol tick moved `hall-1`'s zone: 22 cells left it and 39 joined; the atlas was unchanged.
- **Draw:** the page's painted set equalled the wire union in all four states (0, 338, 456 and 0 cells). Screenshots: keep-red and keep-clear.

All five PRs carried a MERGE-READY comment review at their final heads: protos 92e7e7b6, encounter c77057b3, session 844f5c6c, api 372d3e7d, web bccbdd01.

Findings from the walk. These are findings, not rulings: the tests verify the system, they are not the encounter's design.
- Neutral watchers (the cook, the scullion, the garrison) get zones under R9, so the kitchen paints red.
- Watchers with passive Perception 0 get zones: ordinary sight is the zone until an arc narrows it (R22).
- A lost roll against a neutral watcher forms no fight.
- The world clock advances only on hero actions in free roam.
- The web cannot resume a seat after the browser closes; a second session was started for the draw walk.

---

## C-cone (rpg-project#621)

### Task K1: encounter — the arc

**Delivers:** R14.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** F1, Z1.
**Files:** `sheets.go` (`SheetFacts.WatchArc int`, degrees: 0 = no arc = 360, 1–360 an arc, anything else refused), `sneak.go` `watchZone` (non-zero arc keeps cells whose bearing from the watcher is within `WatchArc/2` of `Facing`, edge inclusive; the watcher's own cell is never in question), `dungeonspec/testdata/cone-site.yaml`. No arc constant anywhere in the module.
**Behavior:** the arc narrows only `watchZone`; `reachesCell`, `VisibleCellsFor` and a non-sneaking mover's sight never read it.
**Tests:** `TestConeSite` (one scenario, sheets scripted with arc 120 for the knight): a knight facing north; a sneaking rogue walking behind it is never rolled for and never seen; stepping into its front third rolls; a non-sneaking rogue behind it is seen as today; fog's visible cells for a player are the same disc with arcs on the monsters as with none; a knight answered with 0 watches all around; declaring behind a watcher that sees you keeps you seen (the cook faces north, the rogue stands south in its ordinary sight and declares: the cook keeps holding the rogue, the pair is lost with `InZone: false`, and it re-arms only after the rogue breaks the cook's line of sight and stands the re-arm distance outside its zone).
**Verification:** module tests; castle suite with `-race`.
**SHIPPED 2026-10-19:** rpg-toolkit#2084 → main b6e99991 = encounter v0.135.0 (e70634e2).
- **Shape:** `SheetFacts.WatchArc int`: 0 is no arc (360), 1–360 an arc, anything else is refused with `ErrInvalidData`. `inWatchZone` keeps the cells whose bearing from the watcher, through `facing.go`'s helper in one frame, lies within `WatchArc/2` of `Member.Facing`, edge inclusive. The watcher's own cell is always in. Only `inWatchZone` reads the arc; `reachesCell`, `Reaches` and `VisibleCellsFor` never do. An unstated facing is 180, so an unturned watcher cones south (R13).
- **Site:** the one-off `dungeonspec/testdata/cone-site.yaml` (one open room, a civil knight and a civil cook facing north, arc 120 scripted) with `TestConeSite` in `dungeonspec/cone_site_test.go`. A sneaking rogue behind the knight is never rolled for and never seen. The roll fires on the cell exactly 60° off. A rogue who is not sneaking is seen from behind. Fog is identical with arcs 120 and 0, and an arc of 0 watches all round. Declaring behind the cook that sees you keeps you seen until sight breaks and the rogue stands the re-arm distance outside. In the deaf-knight phase (passive 0) there is no cone, and the zone holds the cell behind (R29). The held-live stage guards R28.
- **Castle:** `TestMadKingTheHallGuardsBackIsNotWatched` in `castle_king_cone_test.go`. The hero waits inside the kitchen door; while the patrol faces 90 the hero is behind its back and nothing rolls; when it turns west the roll fires at DC 10. The castle fixture is unchanged; the castle host takes an optional arcs map, and a guard asserts that at least one hall cell was checked.
- **Mutants killed (10 of 11 at review, the eleventh pinned at f0110589):** drop the arc; the arc in `reachesCell`; the arc on every reach (fog); out-of-zone hiding removed; edge exclusive; re-arm without broken sight; the R28 guard removed; a declaration ignoring live holding; cones unset at declaration or on the red floor; a roll on every refresh; R29 passive 0.
- **Rulings:** design R28 (a sneaker outside the wedge is unseen only by a watcher that does not already hold them live) and R29 (a watcher with passive Perception 0 gets no cone). The reviewer's note on the held-live path is design O8.

### Task K2: session — supply the arc

**Owner:** rpg-toolkit `rulebooks/dnd5e/session`.
**Files:** `sheets.go` (`WatchArc`: for a monster, its stat block's `senses.watch_arc` once that field exists, else the session's seed 120; for a character, 0), pin.
**Tests:** a monster with no stated arc answers 120; a character answers 0.
**Closes:** the C-cone slice issue, on the K-record PR after the walk.
**SHIPPED 2026-10-19:** rpg-toolkit#2083 → main 990cebb7 = session v0.140.0 (b242ff0e). `sheets.go` supplies `WatchArc`: a monster answers its stat block's `senses.watch_arc` once that field exists, else the session seed `monsterWatchArc = 120`, one constant whose doc cites R14 and O6; a character answers 0. Tests: a monster with no stated arc answers 120; a character answers 0. The sneak suite's goblin now faces west (270), toward the door it watches: an unstated facing is 180, and the 120 arc missed the door. That is test setup only.

### Task K3: rpg-api — re-pin

**SHIPPED TBD:** rpg-api#1119 → dev TBD. A re-pin only, with no code: encounter v0.135.0, session v0.140.0, protos gen 84717fe. The castle shelf is unchanged, verified equal to the fixture minus the routes paragraph. No content suite failed under the south-facing seed. The web is unchanged: 0b9d37e8 draws what it is sent.

### Deferred: content arc (K-content) and the studio field

A monster-data field `SensesData.WatchArc` (`watch_arc`, degrees) in
`rulebooks/dnd5e/monster`, the session reading it, and a studio field on the
creature panel, all wait until content asks for a creature that watches
differently from the seed. They are a module PR each when they come. The K3
label went to the rpg-api re-pin.

### K-walk and K-record

Walk: sneak behind a patrolling knight without a roll; step into its front and
roll. Tune the session's seed (design O6) and record the value.

**SHIPPED 2026-10-19.** Walked 2026-10-18/19 on the castle stack (api k3-cone 39b06a00, then a re-walk at 14f8440e = encounter c016bc40, session c91a63ac); the checklist is ticked on #621 ([walk comment](https://github.com/KirkDiggler/rpg-project/issues/621#issuecomment-6251517700)):
- **Wedge:** `hall-1`'s zone is 48 cells, against 312 in the Z-walk. 4,935 zone cells across 10 watchers all lie within ±60° of their watcher's facing.
- **Behind the back:** over ticks 20–25 (facing 90) nothing rolled. One Snuck beat at the inclusive 60° edge (DC 12, total 18, beaten).
- **Turning:** the wedge turns with the watcher (48 → 160 cells, disjoint, each within its facing).
- **Draw:** the painted set equalled the wire in 47 browser states.
- **Re-walk:** the recipe passed, plus a new case. Seen while not sneaking, the hero breaks sight, declares, and walks in behind at 177–180° off: no roll. The guard turns to 270: one Snuck beat (seq 3577, DC 12, total 22, beaten).
- **Seed:** 120 reads right on the patrol; design O6 records the value.
- **Timing aid:** `hall-1` moves 6 cells per tick round a 19-cell loop; a tick fires every 3 sneaking cells or 6 plain cells.

All three PRs carried a MERGE-READY comment review at their final heads: encounter f0110589, session de79525f, api TBD.

Findings from the walk. These are findings, not rulings: the tests verify the system, they are not the encounter's design.
- An unstated facing is 180, so every stationary castle watcher cones at a wall or away from its door: the cook (kitchen door 90° off), the scullion (south wall), the captain and `gate-1`/`gate-2` (backs to the gate, facing the arrival road), the turnkey (cells wall, door 49–71° off) and the king (zone is its own cell, facing the throne wall). The engine is right; the content states no facing for its stationary watchers. Stated facings on the castle fixture, or a studio or stat-block field (K-content), is the operator's call.
- A lost roll against the neutral garrison forms no fight.
- The guard's own fog is not on the wire, so "his sight reaches you" rests on the hero's line of sight to him.
- Headless Playwright ignored wheel, pan and Center-on-me; screenshots were taken at 2600×1800.

---

## Requirement coverage

| requirement | task(s) | proof |
|---|---|---|
| R1 declared, free roam only | S1, S2, S4 | `SetSneaking` refusals; toggle hidden in a fight |
| R2 not sneaking unchanged | S1, K1 | non-sneaking mover seen as before in `sneak_test.go`; **shipped**: `TestConeSite` sees a rogue who is not sneaking from behind the knight |
| R3 a roll on entering, server-side | S1, S2 | `TestSneakSite` resolver asked twice over two entries; session rolls on shared dice |
| R4 facing first; patrols; stopped faces target | F1–F4 | **shipped**: `TestFacingSite` patrol and stop bearings; F-walk |
| R5 the roll seen, the red floor | S4, Z4 | story line with calculation; **red floor shipped**: Z4 paints exactly the wire union (`data-watch-zone-cells`), keep-red / keep-clear screenshots on the Z-walk |
| R6 the mode verb, state and refresh | S0–S3 | verb round-trip, persistence, stop-in-zone seen in the same call |
| R7 what ends it, full-speed fight | S1 | attack, cast, speak, formation, exit cases; first turn budget at full speed |
| R8 half pace | S2 | cells-per-round test |
| R9 the zone is sight reach | S1, Z1 | zone covers an unlooked corridor; **shipped**: `TestZoneSite` (zone from `inWatchZone`, the geometry `judgeSneakers` rolls with) and `TestMadKingTheHallGuardsRedFloor` |
| R10 entering only, plain view lost, re-arm | S1 | plain-view declaration gets no roll; pacing edge rolls once; re-entry past the distance re-rolls; save/load does not re-roll |
| R11 suppress, not erase | S1 | a memory ghost survives a won roll; suppression through the real `rebuildPercepts` |
| R12 beat to the mover, watcher only if seen | S1, S2, S3 | audience test; empty watcher case |
| R13 facing state, unauthored 180, testimony on `Seen` | F0–F4, K1 | **shipped**: round-trip; `Seen` carries facing; `HexEntity` uses it; the cone reads the unstated 180 (K-walk finding: stationary castle watchers cone south) |
| R14 supplied arc on the zone only | K1, K2 | **shipped**: `TestConeSite` (behind never rolled, 60° edge rolls, fog identical at arcs 120 and 0, arc 0 all round); `TestMadKingTheHallGuardsBackIsNotWatched`; no constant in encounter; K2 seed test (monster 120, character 0); K-walk: every zone cell within ±60° of facing |
| R15 red floor contract | Z0–Z4 | **shipped**: `TestZoneSite` (live watchers only, known cells only, none when not sneaking); `TestMadKingTheHallGuardsRedFloor`; Z3 convert table test; web tints exactly the sent cells; Z-walk: a held-but-not-current watcher gets no zone |
| R16 out of scope held | S1 | the drop arm still discards (walk continues in `TestSneakSite`) |
| Kitchen-door rows (use case §3) | S1, S-record | **shipped**: `TestMadKingKitchenDoorSneaksPastTheHall` with its mutation; heard route measured (`TestMadKingTheKitchenDoorsCrashReachesOnlyTheHall`), content fix the operator's |

## Provider / consumer seams

| provider | consumer | contract | availability | proof |
|---|---|---|---|---|
| F1 `Member.Facing`, `SightTestimony.Facing` | F2 projection onto `Seen` | degrees, `*float64` absent = no testimony | F1 pseudo-version | F2 launch test |
| F0 `Seen.facing_degrees` | F3, F4 | optional float field 4, same convention | F0 tag | F3 convert test, F4 `HexEntity` test |
| F1 `FacingFromCompass` | F2 launch, F4 parity | eight words → degrees | F1 | F4 parity table equals F1's table |
| existing `CheckResolver.ResolveDiscoveryCheck` | S1 `judgeSneakers` | approach `stealth`, DC passive | on main; session wires it always (`capabilities.go`) | S2 test with the real seam |
| S1 `Encounter.Sneaking` | S2 `sheetSeam` speed | bool per member, read at use | S1 pseudo-version | S2 pace test |
| S1 `SheetFacts.WatchResetCells` | S2 seed | int cells, 0 = no hysteresis | S1 | S1 pacing test, S2 seed test |
| S1 `snuck` beat payload | S2 `SnuckBody` → S3 `Snuck` | field names in S1 Interfaces | S1, S0 | S3 convert test |
| S2 staging of sneakers | S1 resolver calls inside any verb | staged record for every sneaking player | S2 | S2 monster-walk test |
| S1 `watchZone` | Z1 `WatchZonesFor`, K1 arc | cell set from one `sightReach` | S1 | `TestZoneSite`, `TestConeSite` |
| Z1 `WatchZone` | Z2 → Z3 → Z4 | watcher id + sorted cells; the session's own `WatchZone` from Z2 on (R27) | Z1, Z0 | Z2 field-for-field copy, Z3 convert table test, Z4 tint test |
| K1 `SheetFacts.WatchArc` | K2 seed | int degrees, 0 = 360, 1–360 an arc, else `ErrInvalidData` | K1 | K2 seed test |

## Plan check findings

- The mad king plan's C1 rolled in the drop arm after percepts were built, so
  a win could not keep the sighting from landing; S1 judges before the pass.
- The drop arm never fires on the castle, because every character and knight
  sees 120 ft; S1 does not depend on it.
- `SheetFacts` has no Stealth and gains none: the roll goes through the
  existing check seam so armour and every Stealth rule apply (design R10).
- Session already stages every player as a discovery observer when the field
  has concealments; S2 widens staging to every sneaking player regardless.
- Each slice has its own issue, per one issue per slice: C-zone's is
  rpg-project#619 and C-cone's is rpg-project#621.
- Review of the design PR: a roll on "standing in the zone" let a sneaker
  vanish from a watcher that saw them; R10 now rolls on entering only.
- The arc and re-arm distance are supplied through `SheetFacts`, seeded by the
  session; the encounter holds no tuning constant.
- Facing rides `Seen`, the sight channel's message, not `Sighting`.
- The castle has no watcher on the kitchen route behind shut doors; S1 locks
  the inner door and posts a hall watcher as stated content changes, and the
  sneak-past test carries a mutation that must fail without sneaking.
