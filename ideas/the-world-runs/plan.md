---
name: The world runs — implementation plan
status: PLAN derived 2026-10-31 from design.md R1–R8 (all ruled on rpg-project#768) and README.md; checked against the inspected baseline below. Task contracts are written in full for S1–S5; S6–S8 carry their contracts at the seam level and are detailed when their slice is reached, as a plan amendment.
authority: design.md (Law, §1–§8, R1–R8), README.md, story rpg-project#766, lane rpg-project#768, slice issues #771–#778
planning guide: ../../.agents/skills/design/planning.md
---

# The world runs — implementation plan

## Goal and scope

A creature on a clock is a machine whose state declares what it listens to;
a raise of the clock wakes only who is due or addressed; clocks and fights
are plural; the record replays from its own rolls. Measured on the benchmark
keep with a driven garrison, before and after, on #766.

**Non-goals.** An async engine. How the encounter is saved (rpg-project#770).
Wall-clock time. Reconciling two clocks' stamps. Changing how sight is
computed (server-at-scale S7, rpg-project#762, is the bound this story
leans on).

**Repositories and order.** Develop outside-in, merge inside-out. Toolkit:
one module per PR.

| slice | repo / module | base | PR shape |
|---|---|---|---|
| S1 #771 | rpg-toolkit `encounter` (dungeonspec/keepgen); rpg-api content; game-dev bench | main / dev / main | keepgen PR; api fixture PR; harness key PR |
| S2 #772 | rpg-toolkit `mind/machine` (new) | main | one PR, auto-tagged `mind/machine/v0.1.0` |
| S3 #773 | rpg-toolkit `encounter` → `session` → rpg-api | main / main / dev | encounter PR on the machine tag; session + api re-pin PRs |
| S4 #774 | rpg-toolkit `encounter` (+ dungeonspec words); rpg-api content | main / dev | encounter PR; castle content PR |
| S5 #775 | rpg-toolkit `encounter` | main | one PR, tests |
| S6 #776 | rpg-api-protos; rpg-api; rpg-dnd5e-web | main / dev / dev | protos → api → web |
| S7 #777 | rpg-toolkit `encounter` (+ dungeonspec); rpg-api content | main / dev | encounter PR; fixture PR |
| S8 #778 | rpg-toolkit `encounter` (→ session/api if the wire names a fight) | main | encounter PR, then follow-ups |

S1 and S2 are independent and run in parallel. S3 needs S2's tag and S1's
fixture. S4 after S3. S5 after S3 (the replay needs the transition beat).
S6 after S4 (the words it authors). S7 and S8 after S4, independent of each
other.

## Finish line

- Sub-issues #771–#778 closed by their "done means", rows posted on #766.
- R8 on the garrison keep (`bench-keep-garrison`: 99 creatures with
  orders, 20 on routes; the default density places 99 monsters at 10k): a Move that wakes nobody costs the plain keep's
  Move (server-at-scale's 100 ms bar, which also needs #762); the Move's
  cost grows with how many woke, never with the roster; a replay from the
  record reproduces the picks; the castle plays unchanged on the default
  recipe; the tomb ticks on its own clock; two fights merge through an
  opened door.
- Expected curve, so each slice's row is read honestly: after S1, the
  garrison Move costs the plain keep's Move plus 99 turns plus 21
  full-roster refreshes. After S3 the census and the 79 posts' turns are
  still paid on the default recipe (a `time: hold` table is a `watch`
  state); the saving is the census. After S4 the posts are authored
  `posted` and never visited; the 20 patrols' turns remain. After #762 the
  21 refreshes are bounded. R8's "wakes nobody" row is S4's row with #762
  in.

## Requirement labels

| label | requirement | source |
|---|---|---|
| W1 | A raise visits only members popped from the due heap or matched by the listener index; no function walks `clock.Members()` on a raise. | R1, Law |
| W2 | Woken machines step in `clock.Members()` order (sorted) inside the raising call; no goroutine; a step's landed beats are matched in the same pass bounded by `intentBound`. | R2 |
| W3 | `mind/machine` is rulebook-agnostic, imports no bus, and exposes `Compile`, `Step`, `Listens`, `DueAt`, `Due`, `Listeners`. | R3 |
| W4 | A member with a `Table` and no recipe runs the default recipe; the castle harness and its goldens pass unchanged except for new `state` beats (none on the default recipe, which has one state). | R4 |
| W5 | `MemberData.Clock` names the clock (absent = `world`); the due heap is keyed by clock; a beat stamped on a non-world clock carries `clock`. | R5 |
| W6 | `on: clock` with `world`, `fight`, `bell:<ref>` is a beat kind; the fight round raise lands `clock: fight` for the faction engaged; the castle reserves run as an authored recipe. | R5, S4 |
| W7 | No seeded roller; a replay roller reads the record's rolls back; the replay test passes on the castle and the garrison; every world roll is on the record. | R6 |
| W8 | Nothing in S1–S8 reads or writes a new persistence shape. | R7 |
| W9 | Rows on #766 before and after each slice; none worse. | R8 |
| W10 | A sound, deed, sighting or fact landed in a raise wakes its listeners in that raise, scope answered through `placementIndex`. | §3 |
| W11 | An area that declares a clock gets a named `Tick`; its members raise it; a transfer moves a member between clocks. | R5, S7 |
| W12 | Two fights form out of each other's reach and merge on contact. | R5, S8 |
| K1 | `castletest` end-to-end tests, `worldtime_test.go`, `tablebeat_test.go`, keepgen's `goldenKeep` SHA (regenerated with the option off) keep passing. | must keep working |

## Constraints

- Toolkit: `go test -race ./...`, `golangci-lint run ./...` per module; tidy
  clean; no `go.work`, no local `replace` in a PR (`rpg-toolkit/CLAUDE.md`);
  consumers pin by `go get <module>@<commit>` then by tag; tags minted on
  merge by `scripts/auto-tag-modules.sh` (a new module gets `v0.1.0`).
- A new module must appear as a node in the root `knowledge/map.md`
  (`scripts/check-knowledge-map.sh`, CI job `knowledge-map`).
- Protos: deprecate, never break. rpg-api: `make test`, `make pre-commit`;
  `internal/dungeons/bench_fixtures_test.go` regenerates fixtures from the
  pinned keepgen. Web: `npm run ci-check`.
- Team signature only; one review round per slice; the visible slices
  staged on :3030 for the operator's walk before merge-out.

## Inspected baseline

| repo | revision | symbols |
|---|---|---|
| rpg-toolkit | origin/main ea629b31 (encounter v0.160.0, session v0.151.0, mind/behavior v0.17.2, encounter pins behavior v0.17.0) | `worldtime.go`: `advanceWorld` :66, `elapseWorld` :98, `seatOnWorldClock` :139, `spendWorldAction` :169, `spendWorldPace` :210, `spendRound` :258, `worldThinks` :308, `worldThinkers` :381, `thinkFor` :424, `appendTickBeat` :480, `onWorldClock` :577, `dealTemperFor` :544. `clocks.go`: `ClockKind` :27, `bubbleFor` :147, `runIntents` :587 (calls `spendDwell` :598, `appendPickBeat` :630), `appendPickBeat` :712, `intentBound` :1332, `buildMonsterView` :2082, `settleWalk` :2014, `form` :2740, `TransferInput` :2863, `Transfer` :2912. `encounter.go`: `audienceFor` :1403, `refreshSight` :1664. `field.go`: `memberRecord` :1530. `data.go`: `EncounterData.Clock` :46, `Bubbles` :56, `MemberData` :2067, `snapshot` :2277, `LoadEncounter` :2744, `clock.LoadTick` :3168. `dwell.go`: `dwell` :43, `dwellDataFrom` :59, `dwellFromData` :68, `dwelling` :87, `spendDwell` :99. `facing.go`: `faceWalkEnd` :300 sets `Dwell` :306. `tablebeat.go`: `BeatTable` :47, `describeTable` :70. `beats.go`: `BeatTick` :30. `noise.go`: `makeSound` :352, `hearersOf` :283. `deed.go`: `landDeed` :263. `flip.go`: `learnFact` :52, `settleStances` :90, `noticeRounds` :274. `standing.go`: `noticeDown` :56. `reserve.go`: `arrivals` :257. `tabledriver.go`: `TableDriver` :67, `Act` :90. `turndriver.go`: `Driver` :46, `TurnIntent` :470. `play/clock/tick.go`: `Tick` :17, `Members` :97 (sorted), `TickData` :198. `mind/behavior/table.go`: `Table` :180, `When` :432, `PickInput` :1605, `Pick` :1646. `dungeonspec`: `RoomMonsterBinding` single_room.go:425 (`Faction` :428, `Table` :449, `On` :461, `Temper` :466), `FactionSpec` spec.go:413, `AnswerSpec` :1271, `SelectorSpec` :1966 (`Path`, `PathHex`). keepgen: `Options` keepgen.go:41, `Generate` :107, `placedMonster` place.go:71, `placeMonsters` :352, emit of `monsterDeclarations` emit.go:164, `OptionsFromHeader` emit.go:216, `goldenKeep` emit_test.go:80, CLI `cmd/keepgen/main.go`. `examples/effects/machine`: recipe.go, validate.go, guard.go, machine.go (reference only). `castletest/`: `castle_king_test.go` helpers, `castle_king_keep_test.go`, `castle_king_gate_test.go`, `castle_king_bench_test.go`. |
| rpg-api | origin/dev 0c634859 | `content/bench/{bench-keep,bench-keep-30k}.yaml` (header = regen command), `internal/dungeons/bench_fixtures_test.go` `TestBenchFixtures_MatchGenerator`, `cmd/server/server.go` :635 (`✓ %s completed in %s`), `content/the-mad-king.yaml` (tables :251, factions :457, bindings :1647, patrol bundle :229) |
| game-dev | main | `tools/browser/bench/run.sh` (default keys), `play.mjs` :88 (Move by floor click), `report.mjs` :133 (api log rows; `BUDGETS.moveServerMs`), `scripts/dev-env.sh` bench env |

## Sequence

1. S1 T1 → T2 → T3 (rows). S2 T4 in parallel.
2. S3 T5 (encounter, on `mind/machine@<commit>` then the tag) → T6 (session/api re-pin, stage :3030, walk castle + garrison) → rows.
3. S4 T7 → T8 (content) → walk → rows. S5 T9 after T5.
4. S6, S7, S8: contracts below; detailed on reaching them.

Merge each slice inside-out; re-pin tags as each lands; rows posted the day
the slice merges.

## Open items

- Resolved: the harness measures one Move row per key from the api log;
  "quiet" versus "patrols due" is read across keys and slices (finish-line
  curve above), not as two rows on one key.
- Resolved: a stamp with no `clock` field means `world` (zero values tell
  the truth); `clock` is written only on a non-world clock (W5), so S3
  changes no golden.
- Resolved (S3 build finding): `Dwell` stays exactly as today. A dwelling
  creature is still due every round; its patrol row holds and rows above it
  fire (`dwell.go`, `TestARowAboveThePatrolInterruptsTheDwellAndLeavingEndsIt`).
  Converting it to `WakeAt` would have made a standing guard deaf to a
  clang, a row getting worse (R4). `WakeAt` is set only by an authored
  `wake_in` act (S4); S3 never sets it and `DueAt` honours it if present.
- Deferred by design: the bus adapter of the example; the studio's recipe
  editor until S6; stamps across clocks (non-goal).
- Blocker for R8's final row: rpg-project#762 (observer-bounded refresh),
  tracked on #753; this plan does not reimplement it.

## Task contracts

### Task 1 (S1): keepgen places a garrison with orders

**Delivers:** S1 #771; the fixture R8 is measured on.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter/dungeonspec/keepgen`.
**Prerequisites:** none.
**Files:** modify `keepgen/keepgen.go` (`Options`), `keepgen/place.go`
(`placedMonster`, `placeMonsters`), `keepgen/emit.go` (emit of
`monsterDeclarations`, `OptionsFromHeader`), `keepgen/emit_test.go`
(`goldenKeep` stays for the option off; a second SHA for the garrison),
`cmd/keepgen/main.go` (flags); new `keepgen/garrison.go`,
`keepgen/garrison_test.go`.
**Interfaces:**
- `Options.Garrison int` (creatures given orders, default 0),
  `Options.Patrols int` (of those, how many walk a route, default 0);
  `Patrols <= Garrison <= monsters placed` else `ErrBadOptions`.
- CLI flags `-garrison N -patrols N`; `OptionsFromHeader` parses them back.
- Emitted document, with the option on: one faction `garrison` (`temper:
  {soldier: 2, coward: 1}`), tables `post` (`time: [{hold: {}}]`) and
  `patrol-<i>` (`time: [{when: {enemy: none}, toward: {path: [c1, c2,
  c3]}}]`, a three-cell loop of floor cells in the monster's own region
  reachable by `LegLine`), and `monsterBindings: {<id>: {faction: garrison,
  table: post|patrol-<i>}}` for the first `Garrison` monsters in placement
  order, the first `Patrols` of them on routes. A route is three waypoints
  `[c1, c2, c3]`: the dialect refuses a retraced cell and `patrolStep` loops
  back to the first waypoint on its own. `Garrison` may not exceed the
  monsters placed (99 at 10k hexes under the default density).
**Behavior:** deterministic under `Seed`; the option off emits today's
bytes (golden unchanged); the document compiles (`Prove`) and every route
cell is floor in its region.
**Tests:** `TestGarrison_EmitsBindings` (garrison 5, patrols 2 → 5
bindings, 2 patrol tables with 4-cell loops, 1 `post`); `TestGarrison_
OffIsGolden` (SHA equals `goldenKeep`); `TestGarrison_RouteCellsAreFloor`
(every path cell in `Prove`'s floor set and in the monster's region);
`TestGarrison_Refuses` (patrols > garrison; garrison > monsters);
`TestOptionsFromHeader_Garrison` round trip.

- [ ] Add the tests; run `go test ./dungeonspec/keepgen/` and see the new ones fail.
- [ ] Implement `garrison.go`, the option, the emit, the flags.
- [ ] `go test -race ./... && golangci-lint run ./...` in `rulebooks/dnd5e/encounter`.
- [ ] Regenerate the garrison SHA; leave `goldenKeep` untouched.

**Verification:** from `rpg-toolkit/rulebooks/dnd5e/encounter`: `go run
./cmd/keepgen -hexes 10000 -seed 1 -garrison 99 -patrols 20 -key
bench-keep-garrison -out /tmp/x.yaml` exits 0 (`Generate` proves its own
bytes before writing; there is no separate prove flag).
**Completion evidence:** PR link, test names green, the fixture's header
line.

### Task 2 (S1): the garrison fixture and the harness key

**Delivers:** S1 #771.
**Owner:** rpg-api `content/bench/`; game-dev `tools/browser/bench/`.
**Prerequisites:** Task 1 merged and tagged (encounter tag), rpg-api pinned
to it.
**Files:** new `rpg-api/content/bench/bench-keep-garrison.yaml` (header
`keepgen -hexes 10000 -seed 1 -garrison 99 -patrols 20 -key
bench-keep-garrison -out content/bench/bench-keep-garrison.yaml`); modify
`internal/dungeons/bench_fixtures_test.go` if it enumerates files by name;
game-dev `tools/browser/bench/run.sh` (default keys gain
`bench-keep-garrison`), `report.mjs` (no change expected; confirm the Move
row renders for the new key).
**Interfaces:** the key `bench-keep-garrison`; room name distinct from the
other keeps (toolkit #2205's naming, else the picker duplicates).
**Behavior:** `TestBenchFixtures_MatchGenerator` regenerates and matches;
the bench env serves the key; a Move on it produces a `✓ Move completed in`
line.
**Tests:** the fixture test; a harness dry run.

- [ ] Generate the fixture from the pinned keepgen; run the fixture test.
- [ ] Add the key to `run.sh`; run `run.sh bench bench-keep bench-keep-garrison`.
- [ ] Post the rows (file/toolkit/server/play, both keys) on #766 with the castle's.

**Verification:** `make test` in rpg-api; the report file under `--out`.
**Completion evidence:** rows on #766; PR links.

### Task 3 (S1): the two numbers by hand

**Delivers:** S1 #771's "Move (patrols due)" versus "Move (quiet)" reading
the harness cannot take alone, and the pre-S3 raise profile.
**Owner:** game-dev job tmp (not committed); findings on #771.
**Prerequisites:** Task 2 on the bench env.
**Files:** none committed. A `go test -bench` in `castletest`-style on the
garrison fixture: `BenchmarkGarrisonMove` in `rulebooks/dnd5e/encounter`
(new file `garrison_bench_test.go`, committed with Task 1 if ready, else
with Task 5), loading the fixture, seating a player, calling `Step` four
cells; `-cpuprofile` to name the top frames (expected: `worldThinkers`,
`buildMonsterView`, `rebuildPercepts`).
**Interfaces:** none.
**Behavior:** a number and a profile.
**Tests:** the benchmark itself.

- [ ] Run the benchmark on the garrison and on the plain keep; record ns/op and the top three frames.
- [ ] Post on #771 and the #766 table.

**Verification:** `go test -run xxx -bench GarrisonMove -benchtime 5x
-cpuprofile` from `rulebooks/dnd5e/encounter`.
**Completion evidence:** the comment with numbers and frames.

### Task 4 (S2): `mind/machine`

**Delivers:** W3; S2 #772.
**Owner:** rpg-toolkit `mind/machine`, new module
`github.com/KirkDiggler/rpg-toolkit/mind/machine`, go 1.27.0, deps `core`
only (plus testify).
**Prerequisites:** none.
**Files:** new `mind/machine/{go.mod,go.sum,doc.go,recipe.go,guard.go,
vocabulary.go,compile.go,step.go,blob.go,due.go,listeners.go,errors.go}`
and `_test.go` for each; `knowledge/map.md` gains `MACH["mind/machine"]`
in the `MIND` subgraph with one seam line ("encounter supplies the
vocabulary; the machine never imports a rulebook or a bus").
**Interfaces:**

```go
type Beat string
type Recipe struct {
    Ref     core.Ref             `json:"ref"`
    Initial string               `json:"initial"`
    Bag     Bag                  `json:"bag,omitzero"`
    States  map[string]State     `json:"states"`
}
type State struct { Transitions []Transition `json:"transitions,omitzero"`; Terminal bool `json:"terminal,omitzero"` }
type Transition struct { On Beat; When *Guard; To string; Do []Act; Reason string }
type Guard struct { All, Any []Guard; Not *Guard; Fact *FactLeaf; Flag string; Scope *ScopeLeaf; Rounds *RoundsLeaf }
type ScopeLeaf struct { Within int `json:"within,omitzero"`; Regions int `json:"regions,omitzero"` } // 0 = unscoped
type Act struct { Pick string; WakeIn int; Set *SetAct; Clear []string; Anchor bool }
type Vocabulary struct { Facts map[FactName]FactSpec; Beats map[Beat]BeatSpec }
type FactSpec struct { Kind ScalarKind; Read func(env any) (any, bool) }
type BeatSpec struct { Timed bool; InScope func(ev any, s ScopeLeaf) bool; Round func(ev any) (uint64, bool) }
func (r Recipe) Compile(v Vocabulary) (*Program, error)
type Blob struct { State string `json:"state"`; Bag Bag `json:"bag,omitzero"`; WakeAt *uint64 `json:"wake_at,omitzero"`; Anchor *uint64 `json:"anchor,omitzero"` }
func (p *Program) NewBlob() Blob
type Stepped struct { Fired bool; Acts []Act; From, To, Reason string }
func (p *Program) Step(b *Blob, on Beat, ev any, env any, now uint64) (Stepped, error)
type Listen struct { Beat Beat; Scope *ScopeLeaf }
func (p *Program) Listens(b *Blob) []Listen
func (p *Program) DueAt(b *Blob, now uint64) (uint64, bool)
type Due[K cmp.Ordered] struct{ /* heap of (at, K), stable by K */ }
func (d *Due[K]) Arm(k K, at uint64); func (d *Due[K]) Disarm(k K); func (d *Due[K]) Pop(upTo uint64) []K; func (d *Due[K]) Len() int
type Listener[K cmp.Ordered] struct { Key K; Scope *ScopeLeaf }
type Listeners[K cmp.Ordered] struct{ /* beat → []Listener in K order */ }
func (l *Listeners[K]) Set(k K, ls []Listen); func (l *Listeners[K]) Drop(k K); func (l *Listeners[K]) For(b Beat) []Listener[K]
```

Exact semantics the encounter depends on:
- `Step` consults only `b.State`'s transitions, in order; the first whose
  `On == on` and whose guard answers `Applies` fires; `Do` is returned, not
  executed (the host executes `Pick`; the machine applies `Set`, `Clear`,
  `Anchor` (sets `b.Anchor = now`) and `WakeIn` (sets `b.WakeAt = now +
  n`) itself); if `To != ""`, `b.State = To` and `From/To` are set; a
  `Timed` beat (`time`) at or after `b.WakeAt` clears it whether or not a
  transition fires (so the host's catch-up loop terminates; a `WakeIn` on
  the firing transition arms it again).
- `DueAt`: a terminal state → `(0, false)`; else if `b.WakeAt != nil` →
  `(*WakeAt, true)`; else if the state has a transition on a `Timed` beat →
  `(now+1, true)`; else `(0, false)`.
- `Listens`: every non-timed `On` of the current state with its `Scope`
  leaf, deduplicated by beat; a state with a `WakeAt` still listens.
- `Pop(upTo)` returns keys with `at <= upTo` in `(at, K)` order and removes
  them; `Arm` replaces an earlier entry for `k`.
- Compile refuses: unknown beat (`ErrUnknownBeat`), unknown fact
  (`ErrUnknownFact`), `To` naming no state (`ErrUnknownState`), `Initial`
  missing, `WakeIn < 0` or `WakeIn == 0` (`ErrBadAct`), a `Scope` on a
  beat whose spec has no `InScope` (`ErrNoScope`), a `Rounds` guard on a
  beat without `Round`. A state with no transitions compiles (terminal or
  a sink). A cyclic `do` is not a compile refusal: the machine's acts never
  raise a beat, only a host-executed `Pick` can, so the bound is the host's
  (`intentBound`, Task 5).
**Behavior:** as above; no bus; no goroutine; JSON round trip of `Recipe`
and `Blob` with `encoding/json/v2` as the example does.
**Tests:** `TestStep_FirstFiringWins`, `TestStep_GuardDependsDoesNotFire`,
`TestStep_WakeInArmsAndTimedClears`, `TestDueAt_NoTimedNoWake`,
`TestDueAt_WakeBeatsTimed`, `TestListens_ChangesWithState`,
`TestCompile_Refusals` (table-driven over the seven refusals),
`TestDue_PopOrderAndRearm`, `TestListeners_SetDropFor`,
`TestRecipe_JSONRoundTrip`, `TestBlob_JSONRoundTrip`; a
mutation-style check that a guard flipped to `Not` changes the fired
transition.

- [ ] Write the tests first against the signatures above; see them fail to compile, then fail.
- [ ] Implement; `go test -race ./... && golangci-lint run ./...` in `mind/machine`; `go mod tidy` clean.
- [ ] Add the `knowledge/map.md` node; `scripts/check-knowledge-map.sh`.
- [ ] One review round; merge; confirm the tag `mind/machine/v0.1.0`.

**Verification:** the commands above; CI's `knowledge-map` and `tagger`
jobs green.
**Completion evidence:** PR link, tag.

### Task 5 (S3): the encounter wakes who is due

**Delivers:** W1, W2, W4, W5 (member and heap keyed by clock; stamps
unchanged); S3 #773.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** Task 4 (`go get mind/machine@<commit>` during the
build, the tag before ready); Task 1 (fixture for the numbers).
**Files:** modify `field.go` (`memberRecord` gains `Machine machine.Blob`,
`Clock string`), `data.go` (`MemberData.Machine *MachineData{State, Bag,
WakeAt, Anchor}`, `MemberData.Clock string ",omitempty"`; `snapshot`,
`LoadEncounter`: build the blob, rebuild `due`/`listeners`; `Dwell`
unchanged), `worldtime.go` (`worldThinks` → `wake`; delete
`worldThinkers`, `thinkFor`; keep `appendTickBeat`, `seatOnWorldClock`
arms the heap), `clocks.go` (`runIntents` keeps calling `spendDwell`;
`Transfer` disarms on entering a bubble, re-arms on leaving), `dwell.go`
and `facing.go` unchanged, new `machine.go` (vocabulary, `defaultRecipe`,
program cache, `enter`, `BeatState`), `beats.go` (`BeatState = "state"`),
`encounter.go` (`Encounter.due machine.Due[MemberID]`, `listeners
machine.Listeners[MemberID]`, `programs map[string]*machine.Program`);
tests `machine_test.go`, `worldtime_test.go` (amend), `castletest` (run).
**Interfaces:**
- Vocabulary this slice supplies: beat `time` (`Timed: true`, `Round` =
  the clock's round); facts none yet (S4 adds). The default recipe:
  `Recipe{Ref: "dnd5e:machines:default", Initial: "watch", States: {"watch":
  {Transitions: [{On: "time", Do: [{Pick: "time"}]}]}}}`, compiled once
  per `Encounter`.
- `wake(clockName string) error`: `at := highWater`; `woke := due.Pop(at)`;
  for each in order: skip if not on the world clock; `stepped :=
  program.Step(&m.Machine, "time", tickEv{at}, env, at)`; for `Pick k`:
  the unit `thinkFor` ran today (`sheetOf`, `TurnBudget{MovementFeet:
  SpeedFeet}`, `runIntents(nil, id, m, 0, &budget, 0, bound, ClockWorld)`,
  `clock.Spend` 1); then re-arm from `DueAt(&m.Machine, at)`; if
  `stepped.To != ""`, append `BeatState{member, from, to, reason}` and
  `listeners.Set(member, program.Listens(&m.Machine))`. Loop `Pop` until
  the heap head is past `at` (catch-up). Then today's closing
  `refreshSight(rosterIDs())` (until #762).
- Program lookup: `programFor(m *memberRecord) *machine.Program` returns
  the default when `m.Table` is non-empty, else nil (a creature with no
  table has no machine and is never armed: same as today's filter).
- `MachineData{State string; Bag map[string]any; WakeAt *uint64; Anchor *uint64}`; absent `Machine` on load means the default blob (`State: ""` → `Initial`).
**Behavior:** every `worldtime_test.go` scenario passes unchanged (two
movers advance once; a fight round is a round for every fighter; a
creature with nothing opposed holds and the beat says so; a spent creature
is not asked again; an alarm mid-tick completes the tick); a dwelling
creature is still due and hears a clang mid-dwell (`dwell_test.go`
unchanged); a member joining a bubble is popped from the heap and re-armed at
dissolve; a creature with a `hold`-only table is still due every round
(R4, by design); `wake` never calls `clock.Members()` (W1 test by a
counting fake is not possible on the concrete `Tick`; assert instead that
a member with no table and a member in `asleep` (test recipe with no
`time`) are not visited: no `BeatTable`, no view built, via a hook counter
in tests).
**Tests:** `TestWake_VisitsOnlyDue` (3 creatures: table, no table, test
recipe without `time` → one pick beat), `TestWake_CatchUpPopsTwice`,
`TestWake_StateBeatAndRelisten`, `TestLoad_WakeAtRoundTrip` (a record
with `WakeAt` set loads as not due until it),
`TestTransfer_DisarmsAndRearms`, `TestDefaultRecipe_IsTodaysTurn`
(golden: the castle harness `TestMadKingKeepFacesAlongItsPatrol` and
`TestMadKingTheAlarmCarriesToTheKeenEar` unchanged), `BenchmarkGarrisonMove`
before/after.

- [ ] Add the tests; see `TestWake_VisitsOnlyDue` fail on today's loop.
- [ ] Implement; delete `worldThinkers`/`thinkFor`; `dwell.go` untouched.
- [ ] `go test -race ./... && golangci-lint run ./...` in encounter; `castletest` green; goldens unchanged.
- [ ] Pin the `mind/machine` tag; one review round; merge; encounter tag.

**Verification:** the commands above; `go test ./castletest/ -run
TestMadKing`; the benchmark numbers.
**Completion evidence:** PR, tag, before/after ns/op on #766.

### Task 6 (S3): re-pin, stage, walk

**Delivers:** S3 #773 walked.
**Owner:** rpg-toolkit `session` (pin), rpg-api (pin + `go version -m`
check), game-dev `envs/local/castle.env` for the operator's :3030.
**Prerequisites:** Task 5 tagged.
**Files:** `rulebooks/dnd5e/session/go.mod`; `rpg-api/go.mod`;
`envs/local/castle.env` (`RPG_API_REF` to the stack branch for the walk,
back to `dev` after merge-out).
**Interfaces:** none new.
**Behavior:** castle on :3030 plays as before; garrison keep on the bench
env shows patrols walking after a Move.
**Tests:** session and api suites; the walk checklist on #773.

- [ ] `go get` the encounter tag in session; PR; tag. Same in api; stage `stack/the-world-runs-s3` on :3030 (`dev-env.sh up castle`); verify the binary's module list with `go version -m`.
- [ ] Operator walks #773's checklist; then merge-out; rows on #766.

**Verification:** `go version -m` shows the encounter and session tags;
the walk's "looks good" on #773.
**Completion evidence:** PR links, tags, the walk comment.

### Task 7 (S4): triggers with scope, clocks as content

**Delivers:** W6, W10; S4 #774 engine half.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter` (+ `dungeonspec`).
**Prerequisites:** Task 5 merged.
**Files:** modify `machine.go` (vocabulary gains beats `heard`, `deed`,
`seen`, `fact`, `clock`; `InScope` through `placementIndex` distance /
region adjacency; facts `enemy` (seen/remembered/none), `fact:<name>`
read from `knownFacts`), new `wakebeats.go` (`e.landed []wakeBeat{Beat,
Cell, Subject, Faction, At, Ev}` appended by `makeSound`, `landDeed`,
`learnFact`, sighting changes in `refreshSight`, and `spendRound` landing
`clock: fight` for every faction engaged; `wake` matches `landed` against
`listeners.For(beat)` with `InScope`, then clears), `dungeonspec`
(`RoomMonsterBinding.States`, `FactionSpec.States` →
`StatesSpec map[string]StateSpec{Transitions []TransitionSpec{On, When
*WhenSpec, Within, To, Do []ActSpec{Pick, WakeIn}}}`, `Initial`; compile
to `machine.Recipe`; refusals surfaced with paths like today's table
errors), `tablebeat.go` unchanged.
**Interfaces:**
- Beat events: `heardEv{Cell, Maker, DC}`, `deedEv{Verb, Actor, Target,
  Where}`, `seenEv{Observer, Subject}`, `factEv{Member, Fact}`,
  `clockEv{Name, Round, Faction}`.
- `clock` beat names: `world` (every raise), `fight` (every `spendRound`
  for the members of the factions in that bubble), `bell:<ref>` (a prop
  deed on `<ref>`).
- A step fired by a beat other than `time` that returns `Pick k` runs the
  same unit as `time` but does not `Spend` the clock (it is a reaction,
  the creature's next `time` is unchanged); a `To` re-lists.
**Behavior:** a sound in a raise wakes `posted` listeners within scope in
that raise; `asleep` with `heard within 2` is never visited by a far sound;
`clock: fight` lands once per round per engaged faction; the reserves
recipe (`waiting: on clock fight → pick march`) walks one leg per fight
round and returns to `waiting` at dissolve (`on: clock world` → `to:
posted`). Compile refusals for a state naming an unknown beat or a `within`
on `fact`.
**Tests:** `TestWake_SoundWakesListenersSameRaise`,
`TestWake_AsleepIgnoresFarSound`, `TestWake_ClockFightOncePerFaction`,
`TestReserves_MarchOnFightRounds` (castle harness, replaces the `arrives`
path's assertion for the gate reserves, keeping `arrives` itself),
`TestDungeonspec_StatesCompile`, `TestDungeonspec_StatesRefusals`.

- [ ] Tests; implement; encounter suite; castle harness.
- [ ] One review round; merge; tag.

**Verification:** as Task 5; `go test ./dungeonspec/`.
**Completion evidence:** PR, tag.

### Task 8 (S4): castle content and the walk

**Delivers:** S4 #774 visible half; W9 rows.
**Owner:** rpg-api `content/the-mad-king.yaml` (and the toolkit copy
`dungeonspec/testdata/world-builder-v4-castle.yaml` in Task 7's PR).
**Prerequisites:** Task 7 tagged; api pinned.
**Files:** the two castle YAMLs: faction `garrison` gains `states`
(`posted` → `alert` on `heard within 6`, back after 3 rounds via `rounds`
guard with `Anchor`); the reserves' binding gains the `waiting` recipe;
the garrison fixture's 79 posts gain `states: {initial: posted, posted:
{transitions: [{on: heard, within: 6, to: alert}, ...]}}` in keepgen's
emit (flag `-posts-listen`, default on when `-garrison` is set).
**Interfaces:** content words from Task 7.
**Behavior:** per #774's checklist.
**Tests:** api content gate (`make test`), walk.

- [ ] Content PRs; stage; operator walks #774; merge-out; rows (castle; garrison keep's Move with posts asleep).

**Verification:** rows on #766; the walk comment.
**Completion evidence:** PR links.

### Task 9 (S5): replay from the record

**Delivers:** W7; S5 #775.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter` (test package
`replaytest/` beside `castletest/`).
**Prerequisites:** Task 5 (state beats), Task 1 (fixture).
**Files:** new `replaytest/replay_roller.go` (`RecordRoller{rolls
[]int}` built from the record's `BeatTable` bodies' die results, deal
beats and attack beats in log order; `Roll` returns the next; `RollN`
likewise; exhausted → `ErrReplayExhausted`), `replaytest/replay_test.go`.
**Interfaces:** `dice.Roller`; the beat bodies' roll fields (read through
the existing body builders' JSON).
**Behavior:** load a record at round 0 (`LoadEncounterInput.Roller =
RecordRoller`), re-issue the verbs the log names in order (Step, Speak,
EndTurn), and assert the landed log equals the original. A roll the record
lacks makes the test name the beat and the verb.
**Tests:** `TestReplay_CastleGateScript` (the script scenario from
`castle_king_script_test.go`), `TestReplay_GarrisonTwentyRounds`.

- [ ] Write the roller and the castle test; find and fix any world roll not on the record (each a one-line beat body addition, reviewed in the same PR).
- [ ] Garrison test; CI; review; merge.

**Verification:** `go test ./replaytest/`.
**Completion evidence:** PR; list of rolls that were missing (if any) on #775.

### S6 #776, S7 #777, S8 #778 — seam contracts, detailed when reached

Detailed 2026-11-01 on reaching S6 (plan amendment, after S4 merged out).

**S6a: the state change on the panel** (the slice's visible half; no new verb).

- **Task T10 (session):** `BeatState` folds into `FamilyState` as `StateChange{Party: member, Change: StateMachine, Machine: &MachineChange{From, To, Reason}}`; `BeatWakeCut` folds into `FamilyClock` as a `ClockEntry` naming the cut and its dropped count. Both leave `recordOnly` (R16's table). `StateKind` gains `StateMachine`. Tests: `TestEveryBeatHasAFamily` stays green with the two removed from `recordOnly`; `TestFold_StateBeat` (the castle reserve's `waiting → marching, the gate is fighting` becomes one state piece for every seat); `TestFold_WakeCut`.
- **Task T11 (protos):** `dnd5e/api/session/v1alpha2/act.proto`: `STATE_KIND_MACHINE` appended to `StateKind`; `message MachineChange { string from = 1; string to = 2; string reason = 3; }`; `MachineChange machine = 8` on `StateChange`. Additive only (protos deprecate, never break). Buf lint and breaking checks green.
- **Task T12 (api):** `convert_act.go`: `sdk.StateMachine → STATE_KIND_MACHINE`, `Machine` copied; pins session and protos tags. Tests: the converter's golden gains one state change.
- **Task T13 (web):** `describeAct.ts` renders `StateKind.MACHINE` as `<Name>: <from> → <to> (<reason>)`, the creature's name from the party ref as other state lines do; `actRefreshKeys.ts` maps it to no refresh. The log line is the walk's evidence for the gate post.
- **Walk (S6a):** castle on :3030: force the gate door from outside the fight → the log reads `Gate guard: posted → alert (heard something)` in that round; start the gate fight → `Reserve: waiting → marching (the gate is fighting)`; end it → `marching → waiting (the fight is over)`.

**S6b: the studio authors states** (second half, its own PRs, after S6a walks): the studio's creature editor gains the `states:` words S4's `dungeonspec` compiles (initial, states, transitions with on/when/within/regions/rounds/to/do/reason), round-tripping through the same YAML the table editor writes; the content gate refuses what the door refuses (S4's binding-path check). Walk: #776's checklist step 1 (`asleep` on a barracks creature).

Seams: T10 before T12 (api pins the session tag); T11 before T12 and T13 (generated bindings); T13 last. Merge inside-out: toolkit → protos → api → web.

- **S6.** Protos: `encounter/v1alpha1` beat `StateChanged{member, from,
  to, reason}` additive beside `Table`; api converts `BeatState`; web
  renders the line and the studio's creature editor gains `states` from
  Task 7's words. Owner per repo; walk on :3030.
- **S7.** `EncounterData.Clocks map[string]clock.TickData` beside `Clock`
  (absent map = `{world: Clock}`); `memberRecord.Clock`; `spendWorldPace`/
  `spendWorldAction` raise `clockOf(member)`; `due` keyed `(clock, at)`;
  beats on a non-world clock carry `clock`; `dungeonspec` `areas: [{name,
  regions: [...], clock: true}]`; `Transfer` between ticks at a declared
  portal/stair cell; the fixture: a keep with an underground area.
- **S8.** `form` drops the `len(e.bubbles) > 0` join short-circuit when
  the contact is out of reach of every running bubble (reach = the
  larger of sight and interaction reach of the members in contact); keeps
  the per-member overlap check; `Transfer` gains `Bubble` addressing by a
  member; `merge(a, b)` seats the later bubble's order after the earlier
  one's, surprised flags preserved, at the leading clock's now; dissolve
  per bubble. Wire: the panel already renders one order; two orders need
  the session's projection to name the fight per seat (session + api +
  web follow-ups, scoped when reached).

## Visible plan checks

### Requirement coverage

| requirement / scenario | task(s) | proof |
|---|---|---|
| W1 no roster walk on a raise | T5 | `TestWake_VisitsOnlyDue`; `worldThinkers` deleted (grep in review) |
| W2 seat order, one pass, bounded | T5, T7 | `TestWake_CatchUpPopsTwice` (order asserted on three members); `intentBound` reused |
| W3 the primitive | T4 | the eleven tests; `go list -deps` shows no `events` |
| W4 default recipe, castle unchanged | T5 | castle harness green with unchanged goldens; `TestDefaultRecipe_IsTodaysTurn` |
| W5 clock named on member and heap | T5 (member, heap), S7 (stamps) | `MemberData.Clock` round trip test in T5; S7 contract |
| W6 clock beats, reserves recipe | T7, T8 | `TestWake_ClockFightOncePerFaction`, `TestReserves_MarchOnFightRounds`; walk #774 |
| W7 replay, no seed | T9 | `TestReplay_*`; `grep -r Seeded` empty |
| W8 no new persistence shape | all | review checklist item; `EncounterData` gains fields, never a store |
| W9 rows per slice | T2, T3, T5, T8 | comments on #766 |
| W10 same-raise triggers with scope | T7 | `TestWake_SoundWakesListenersSameRaise`, `TestWake_AsleepIgnoresFarSound` |
| W11 area clocks | S7 | contract; detailed on reaching |
| W12 plural fights | S8 | contract; detailed on reaching |
| K1 existing suites | T1 (golden off), T5, T7 | CI |
| README scene: a crash wakes the gate same raise | T7, T8 | walk #774 |
| README: sleeping barracks never visited | T7, T8 | `TestWake_AsleepIgnoresFarSound`; garrison row after T8 |
| R8 "wakes nobody" row | T8 + rpg-project#762 | the garrison Move row on #766 after both |

Deferrals with authority: stamps across clocks (non-goal, design); S6–S8
detail (this plan's status line).

### Provider/consumer seams

| provider | consumer | contract | availability | proof |
|---|---|---|---|---|
| T1 keepgen `Options.Garrison/Patrols`, emit bindings | T2 fixture; T3 bench | header flags round-trip through `OptionsFromHeader`; `monsterBindings` + `tables` + `factions` compile | T1 merged + tagged before T2 | `TestOptionsFromHeader_Garrison`; `TestBenchFixtures_MatchGenerator` |
| T4 `machine.Program.Step/Listens/DueAt`, `Due`, `Listeners` | T5 encounter | signatures in T4; `Step` returns `Pick` unexecuted; `DueAt` `now+1` for a timed state | T4 commit for the build, tag before T5 ready | `TestWake_*` compile against the tag; `TestStep_WakeInArmsAndTimedClears` ↔ `TestLoad_DwellBecomesWakeAt` |
| T5 `BeatState`, `MachineData`, `MemberData.Clock` | T6 session/api (pass-through), T9 replay, S6 protos | beat tag `state`; JSON shapes above; absent = default | T5 tag | session/api suites; `TestReplay_*` reads `state` beats |
| T7 beats + `dungeonspec` `states` words | T8 content, S6 studio | `StatesSpec` field names; compile refusals pathed | T7 tag | `TestDungeonspec_StatesCompile`; api content gate |
| `placementIndex` (server-at-scale S2, shipped) | T7 `InScope` | cell → placements; distance by `spatial` | present at ea629b31 | `TestWake_AsleepIgnoresFarSound` |
| rpg-project#762 observer-bounded refresh | R8 row | `refreshSight` audience | not yet; tracked on #753 | the final garrison row |
| `clock.Tick.Members()` sorted | T5 wake order | sorted order | present | `TestWake_CatchUpPopsTwice` |

Shared files: T5 and T7 both edit `machine.go`, `worldtime.go`; T7 starts
after T5 merges (sequence). T1 and T5 both touch encounter; T1 is confined
to `dungeonspec/keepgen` and merges first.

Scan findings and disposition: `W1` cannot be proved by a counting fake on
the concrete `Tick`, so it is proved by the absence of beats and views for
non-due members plus the deletion of `worldThinkers` (recorded above). The
"quiet Move" number is a curve across slices, not one row (Open items).
The first draft changed `Dwell` to "not due while dwelling"; the S3 build
found `dwell_test.go` pins a dwelling guard hearing a clang, so the dwell
stays as today (Open items). The scan had looked only for hold beats and
missed the rows above the patrol row: the lesson is to read the owning
package's own tests for a semantic before planning to change it.
