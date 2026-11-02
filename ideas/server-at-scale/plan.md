---
name: Server at the keep's scale — implementation plan
status: PLAN derived 2026-10-31 from design.md R1–R4 and README.md, on the S1 probe report (rpg-project#753); checked against the inspected baseline below; amended in the design PR's review round. R4 (interaction by reach) removes the TargetOptions RPC: the wire is unchanged for interaction, and the reach refusal reuses `TARGET_OUT_OF_REACH`.
authority: design.md (R1–R4, Law), README.md (ownership table and refusals), story rpg-project#753, lane rpg-project#754, S1 probe report (comment on #753)
planning guide: ../../.agents/skills/design/planning.md
---

# Server at the keep's scale — implementation plan

## Goal and scope

A keep launches, answers a player's Afford with the menu within reach, and
takes a Move inside site format R1's budgets, with sessions locked by session and the
discovery-sharing preference in a character configuration store. Every slice
posts its before and after rows from `tools/browser/bench/run.sh bench` on
#753 the day it lands, and no row gets worse.

**Non-goals.** The studio's own per-commit copies and polygons; play's one
mesh per tile and GLB clone per prop; distributed locking; preferences other
than discovery sharing (O4); a reach other than R4's constant.

**Repositories and order.** Develop outside-in, merge inside-out.

| slice | repo / module | base | PR shape |
|---|---|---|---|
| S2 | rpg-toolkit `encounter` → `session`; rpg-dnd5e-web | main / main / dev | encounter PR (oracle, index, `WithinReach`); session PR (speak menu bounded by reach); web PR (click a far target → Move toward it); no protos or api change |
| S5 | rpg-api-protos; rpg-toolkit `session`; rpg-api | main / main / dev | protos PR (`SetPreference`); session PR (`CharacterConfigReader`, verb removed); api PR (store, then the locker per session) |
| S3 | rpg-toolkit `encounter` | main | one PR: index-backed projection, reach-bounded sweep cold and warm, no monster beats |
| S4 | rpg-toolkit `encounter` | main | one PR: known-cells facts as row runs; beats carry only the newly shown delta |

S2 and S5 are independent and may run in parallel, except that the api's
locker change (T4) merges only after the preference has left the session's
write path (T6). S3 after S2's encounter PR (it shares `placementIndex`); S4
after S3. Merge each slice toolkit → protos → api → web.

## Finish line

- Sub-issues S2–S5 under #753 closed by their "done means" with rows posted.
- On the bench env (numbers are defaults the operator may move on the design
  PR): `bench-keep` launches under 2 s and `bench-keep-30k` under 10 s (from
  18.6 s / 149 s); a player Afford on either under 100 ms (from 320 s) with the
  menu within reach included; Move under 100 ms on both (R1 of site format);
  the castle's rows unchanged or better; a castle launch beside a live keep no
  slower than alone.
- The session blob of the 10k keep under 1 MB (from 7.3 MB).
- `sharedStoreLocker` gone; `character-config:<id>` holds discovery sharing.

## Requirement labels

| label | requirement | source |
|---|---|---|
| V1 | `readSpeakMenu` offers only the creatures and props within interaction reach (3 hexes) of the member, found through `encounter.WithinReach`; nothing farther is asked about; `speakRowOnTheWorldClock` reads the same bounded menu. | R1, R4 |
| V2 | The rows keep today's shape (`Declaration.speak_options` with the Speak selector id); a target within reach that the member does not see now, or that does not witness the member, is simply not offered; `Speak` at a target beyond reach is refused through `refuseOutOfReach` with `ErrOutOfRange` → `ShortfallTargetOutOfReach` → wire `TARGET_OUT_OF_REACH`, all of which exist. | R1, R4 |
| V3 | `encounter.shownTo(member, cells)` answers "not in `undiscoveredFrom(member).cells`", the mask built once per menu read, equal to today's `AtlasFor`-based answer; a creature within reach is offered only if the member's sightings hold it; `OptionsFor`/`PropOptionsFor`/`readSpeakMenu` call no `Atlas`/`AtlasFor`. | Law Knowledge is the oracle |
| V4 | `field.placementIndex` maps cell → authored placements and walls, built with the field in O(cells + footprint cells) and never written after; `placedCells`, `wallFootprintCells` and `placedCellsOf` read it, and a dropped placement's cells come from `propPlacements` at query time as `placedNow().stands` does. | Law The field indexes |
| V5 | `Encounter.Atlas` costs O(cells + placements + walls) with no per-placement scan of the cells; nothing is cached across verbs and nothing derived is stored; the atlas goldens are byte-identical. | Law The field indexes |
| V6 | `discoverCells` tests only cells within the observer's sight reach, cold (launch) and warm (every step's `refreshSight`); a monster observer writes its `known:cells` facts and no `cells_revealed` beat. | Law A sweep covers what can be seen; O2 |
| V7 | `known:cells` facts are written as row runs and folded by `cellFold` from runs; a seat's `cells_revealed` beat carries only the newly shown atlas delta; readers of known cells are unchanged. | Law Knowledge is written once |
| V8 | rpg-api locks session verbs by session id; `sharedStoreLocker` is deleted. | R2 |
| V9 | `CharacterService.SetPreference{character, discovery_sharing}` writes `character-config:<id>` under the character guard; the session's `prepareExploration` reads `DiscoverySharing(ctx, character)` through `sdk.CharacterConfigReader` at every write verb and restores the encounter's per-member record (`RestoreDiscovery`), as today; the session SDK's `SetDiscoverySharing` verb, `encounter.SetDiscoverySharing` and `ExplorationRepository` are removed; `SessionService.SetDiscoverySharing` stays on the wire deprecated, and its api handler writes the configuration store; a legacy `character-exploration:v1:<id>` record is read once as the initial value through `resolveDiscoverySharing`'s branches (explicit choice wins; a record with no choice reads `!private`; no record means not chosen). | R2, O4 |
| V12 | A Move on the 10k and 30k keeps completes under 100 ms server time, measured by the harness's Move row; its warm `refreshSight` is reach-bounded (V6) and its data load/save is the compact form (V7). | Law A step costs its reach; site format R1 |
| V10 | The web turns a click on a creature or prop beyond reach into a Move toward it; the menu for targets within reach keeps coming from Afford's rows. | R1, R4 |
| V11 | Before and after rows for the castle and both keeps posted per slice; none worse. | Law No row gets worse |
| K1 | The castle's QA routes, goldens, hearing, concealment, fog and the Speak act's door checks keep working. | must keep working |

## Constraints

- Toolkit: one module per PR; `go test -race ./...`, `golangci-lint run ./...`; tidy/fmt clean; tags minted on merge; the session re-pins the encounter tag before its own PR goes ready.
- Protos: deprecate, never break; `buf` gates; bindings published on merge.
- rpg-api: `make test`, `make pre-commit`; pins by tag; the registry and goldens unchanged.
- rpg-dnd5e-web: `npm run ci-check`; head current with dev.
- Team signature only.

## Inspected baseline

| repo | revision | note |
|---|---|---|
| rpg-toolkit | origin/main ea629b31 (encounter v0.157.0; session main = v0.149.0 + go.mod) | symbols and lines per the S1 report: `offers.go:139/289/867/928`, `afford.go:549/627`, `option.go:162/195/254`, `hold.go:288/389`, `atlas.go:463/515`, `placed_props.go:412/1056`, `structural_walls.go:487`, `roomknowledge.go:140/156/180/202`, `roomreveal.go:40`, `encounter.go:1753`, `board.go:92`, `session/exploration.go:19/50/182`, `session/session.go:43` |
| rpg-api | origin/dev 5a5c4d6d | `internal/orchestrators/session/locker.go`, `exploration.go:55` (`sharedStoreLocker`), `orchestrator.go:150-153`; handlers `afford.go`, `speak.go`, `discovery_sharing.go` |
| rpg-api-protos | origin/main | `session/v1alpha1/service.proto` `Afford` :2350, `Speak` :2405, `SetDiscoverySharing` :2277; `types.proto` `Declaration.speak_options` :518; `dnd5e/api/v1alpha1/character.proto` `CharacterService` :17 |
| rpg-dnd5e-web | origin/dev a389b5a1 | `combat-experience/speakOptions.ts`, `ConversationPanel.tsx`, `TargetSurface.tsx`, `useSessionCombatExperience.ts`, `api/useSessionSpeak.ts` |
| game-dev | main 20af2cc2 | harness |

Numbers (S1 report): Afford 10k 320 s live / 655 projections; launch 18.6 s / 149 s, 84% `rebuildPercepts`; `Atlas` 329 ms / 2.1 s, `placedCells` 96% at 30k; blob 7.3 MB / 27 MB, 88–91% per-member knowledge; `sharedStoreLocker` one key.

## Gaps resolved under the design's authority

| gap | resolution | authority |
|---|---|---|
| G1 the approach's stop | reach: three hexes (R4), measured by `Encounter.Distance` as `refuseOutOfReachCell` measures; the client walks a far target into reach and the next Afford carries its rows | R4, O1 |
| G2 witness for a prop | a prop has no eyes; a prop within reach is offered on `shownTo` alone | Law Knowledge is the oracle |
| G3 what "known cells" is | the journal's `known:cells` facts, written by `learnCells` as deltas and folded by `cellFold`; perception has no cell concept, so the home stays where it is and only its encoding changes | Law Knowledge is written once |
| G4 why no atlas cache | the api loads the encounter from its data on every verb, so an in-memory cache lives one verb and a persisted one is stored derived data (refused as site format R8 refuses it); the index makes the projection linear instead | Law The field indexes |
| G5 a monster's beat today | `appendCellsRevealedBeat` runs per learning observer; S3 gates it on the observer being a seat; the monster's `known:cells` facts still persist, so a reloaded monster skips no knowledge | O2 |
| G6 reach bound | `sightReach` carries each observer's max distance; the sweep iterates the hex ring of radius sight + 1 around the observer's cell (sight = the Sight capability's per-member range, 24 cells at the session's default, never the 3-hex interaction reach; the extra ring for cells whose centre lies within sight of an edge) instead of `e.field.cells`, cold and warm | Law A sweep covers |
| G7 reach and the clocks | the bounded menu is read where the menu is read today: on the turn clock in `compileOffersFor`, on the world clock in `speakRowOnTheWorldClock`; pricing unchanged | README |
| G8 store key and shape | `character-config:<character>` JSON `{discovery_sharing: bool|null}`; absent means not chosen; a legacy `character-exploration:v1:<character>` record is read once as the initial value through `resolveDiscoverySharing`'s three branches and never written again | R2, O4 |
| G9 the approach has no server verb | `MoveRequest` takes a client-composed path; the client composes the path toward the target and stops within reach, as it composes every move today | R4, README |

## Tasks

### Task T1: the oracle and the placement index (S2, encounter)

**Delivers:** V3, V4.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** none.
**Files:** modify `hold.go` (`showsCellTo`, `showsAnyCellTo` → `shownTo(member MemberID, mask undiscovered, cells []spatial.Position) bool`, "no cell in `mask.cells`"; the mask from `undiscoveredFrom(member)` built once by the caller per read; delete the `AtlasFor` call), `option.go` (`speakableProp` uses `shownTo`), new `placement_index.go` (`placementIndex` built in `compileField`, static; `placedCells`, `wallFootprintCells` read it), `atlas.go` (`placedCellsOf` reads the index), new `reach.go` (`WithinReach(member MemberID, reach int) (members []MemberID, props []PropID, error)`: cells within `reach` by `Encounter.Distance` from the member's cell, through the index and the roster's cells, kept only if the member sees them now per `VisibleCellsFor`); modify `option.go`'s `Speak` (`SpeakInput.Range int`; after the existing shown and witness checks, in Hold's order so a hidden far target still answers "no such thing" and never "out of range", `Speak` calls `refuseOutOfReach("speak", speaker, target, in.Range)` for a creature and `refuseOutOfReachCell` for a prop, both in `loot.go`).
**Interfaces:** `shownTo` equals today's `AtlasFor`-based answer for every cell (known under the mask, footing included as the projection includes it); `placementIndex.at(cell) []placementRef`, built in `compileField` in one pass over authored footprints and walls, never written after; a dropped placement's cells are computed from `propPlacements` at query time as `placedNow().stands` already does; `WithinReach` lists every creature the member's sightings hold and every placed or cell prop with a cell the member sees now within reach, sorted by id, excluding the member; `SpeakInput.Range` zero means 1, as every reach does.
**Behavior:** no caller of `shownTo` or `WithinReach` projects an atlas; an unsighted creature within reach is absent from the result, not refused.
**Tests:** `TestShownTo_MatchesAtlasFor` (property test on the castle and the 10k keep: for every member and every placement, including one on concealed floor, `shownTo` equals the old answer); `TestPlacementIndex_StaticAndDroppedAtQuery` (a dropped prop's cells come from its record, the index unchanged); `TestWithinReach_Ring` (ring of 3 = 37 cells; a prop with one footprint cell inside counts; an unsighted creature inside is absent; a known prop behind a wall inside is absent; adjacent-only when reach = 1); `TestSpeak_OutOfRange` (`ErrOutOfRange` at four hexes, admitted at three); `BenchmarkPropOptionsFor_Keep10k` (< 1 ms, from ~550 ms); `BenchmarkAtlas_Keep30k` (< 100 ms, from 2.1 s, through the index); `TestOptionsFor_NoProjection` (a probe counter asserts zero `Atlas` calls); atlas goldens byte-identical.
**Verification:** `go test -race ./... && golangci-lint run ./...`; the benchmark numbers in the PR body.
**Completion evidence:** PR link, benchmark lines, golden diff empty.

### Task T2: the speak menu within reach (S2, session)

**Delivers:** V1, V2, G1, G2, G7.
**Owner:** rpg-toolkit `rulebooks/dnd5e/session`.
**Prerequisites:** T1 tag.
**Files:** modify `offers.go` (`readSpeakMenu` takes its targets from `encounter.WithinReach(member, InteractionReach)` instead of `enc.Witnesses` + `speakableProps`; `speakableProps` and its `enc.Atlas()` are deleted; the witness test runs per creature within reach; `const InteractionReach = 3` with its doc naming R4), `afford.go` (`speakRowOnTheWorldClock` unchanged in shape, reads the bounded menu), `speak.go` (passes `Range: InteractionReach` to `encounter.Speak`; maps `ErrOutOfRange` to `ShortfallTargetOutOfReach` as Hold does); `buildSpeakOffer` unchanged. Castletest: the survivors script speaks to `keep-2` from about ten hexes and two refusal-control steps speak to the captain from afar — each moves the speaker within reach first, and the refusal controls assert their original reason, which holds because reach is checked after the door's shown and witness checks; the #639 QA script gains "walk up, then speak".
**Interfaces:** rows carry the same selector id `Speak` validates today; a target outside reach is not a row; `Speak` on a target outside reach is refused at the door with `ShortfallTargetOutOfReach` (existing).
**Behavior:** the menu lists only targets within reach that are sighted (creatures) or shown (props); the Speak row keeps its slot, price and availability as today, so the web's Speak button is unchanged; a Speak at a target out of reach is refused at the door.
**Tests:** `TestSpeakMenu_WithinReachOnly` (castle: a guard four hexes off is not offered, at three he is); `TestSpeakMenu_HiddenPropNotOffered`; `TestSpeakMenu_UnsightedCreatureNotOffered`; `TestSpeak_OutOfReachRefused`; castletest script steps that speak move within reach first (the walk captures the spirit: the player walks up; the QA castle script #639 gains the step); `BenchmarkAfford_Keep10k` (< 100 ms, from 369 s).
**Completion evidence:** PR link, benchmark line, the #639 script step added.
**Verification:** module gates; benchmark numbers in the PR body; castletest green.

### Task T3: not applicable (S2, protos)

The wire already carries everything S2 needs: `Declaration.speak_options` for the rows within reach and `TARGET_OUT_OF_REACH` for the door's refusal. S5's `SetPreference` is T6.

### Task T4: the lock (S5, api)

**Delivers:** V8.
**Owner:** rpg-api.
**Prerequisites:** T6 merged (the preference has left the session's write path).
**Files:** modify `internal/orchestrators/session/orchestrator.go:150-153` (use `NewInProcessSessionLocker()` directly), delete `sharedStoreLocker` from `exploration.go`.
**Behavior:** two sessions' verbs interleave; a 10k keep's Afford no longer delays a castle launch.
**Tests:** `TestLocker_SessionsDoNotShareAKey` (two sessions' verbs interleave); the castle integration tests.
**Verification:** `make test`, `make pre-commit`; on the bench env, a castle launch beside a live keep session takes the castle's own time.
**Completion evidence:** PR link; the castle-beside-keep row on #753.

### Task T5: walk to a far target (S2, web)

**Delivers:** V10.
**Owner:** rpg-dnd5e-web.
**Prerequisites:** T2 on a stack for the walk (no wire change).
**Files:** `combat-experience/TargetSurface.tsx` (a creature or prop beyond reach is clickable; the click issues a Move toward it, stopping within reach), `useSessionCombatExperience.ts` (the approach intent), `ConversationPanel.tsx` and `speakOptions.ts` unchanged except that an empty menu reads "nobody within reach".
**Behavior:** the client composes the path toward the target (G9) and stops within reach; the menu comes from Afford's rows unchanged.
**Tests:** vitest: click a far creature → a composed Move path toward it stopping within reach; after the view refresh with the creature within reach, the panel shows its rows from Afford; nothing within reach → the empty-menu text.
**Verification:** `npm run ci-check`; the operator walks the castle: click the cook from the hall, the character walks over, the cook's lines appear; stand next to the altar, its options appear without a click.

### Task T6: character configuration (S5, protos + session + api)

**Delivers:** V9, G8.
**Owner:** rpg-api-protos (`CharacterService.SetPreference{character_id, optional bool discovery_sharing}`), rpg-toolkit `session` (`Config.CharacterConfig CharacterConfigReader` with `DiscoverySharing(ctx, character) (*bool, error)` replaces `Config.Explorations`; `prepareExploration` reads it and calls `encounter.RestoreDiscovery` at every write verb as today; `Manager.SetDiscoverySharing` and `ExplorationRepository` are removed) and `encounter` (`SetDiscoverySharing` removed; `RestoreDiscovery` and `DiscoverySharing` stay), rpg-api (`internal/repositories/characterconfig` Redis `character-config:<id>`; `CharacterService.SetPreference` handler under `LockCharacter`; `SessionService.SetDiscoverySharing` handler kept, deprecated, writing the same store for the caller's seated character; a legacy `character-exploration:v1:<id>` record read once as the initial value, G8).
**Behavior:** a preference change takes effect at the member's next write verb, when `prepareExploration` restores it; the web's switch keeps working through the deprecated RPC until it moves to `SetPreference`.
**Tests:** session: `TestResolveDiscoverySharing_FromReader` (nil → not chosen; true/false); api: repository round-trip, `SetPreference` locks the character, the deprecated handler writes the store, legacy record read once.
**Verification:** module gates; `make test`; the discovery-sharing switch in the web still flips and persists across sessions.
**Completion evidence:** PR links (protos, session tag, api).

### Task T7: the projection through the index (S3, encounter)

**Delivers:** V5, G4.
**Owner:** rpg-toolkit `encounter`.
**Prerequisites:** T1 (the index exists; T1 already routes `placedCellsOf`). T7 routes the remaining per-projection scans: `structural_walls.go` `wallFootprintCells` → `footprintObservationCells` (26–31% of launch) and `atlas.go:663`'s wall walk read the index; the sight-pass memo `placedCellsOf` (`placed_props.go:1056`) becomes the index.
**Behavior:** `Atlas()` is O(cells + placements + walls): it copies the cells and walks them once for `Sealed`, and no placement or wall scans the cells; its output is byte-identical.
**Tests:** `BenchmarkAtlas_Keep30k` (< 50 ms, from 2.1 s); the atlas goldens byte-identical; a probe counter asserts no per-placement cell scan.
**Verification:** module gates; goldens; number in the PR body.
**Completion evidence:** PR link, benchmark line.

### Task T8: reach-bounded sweep, no monster beats (S3, encounter)

**Delivers:** V6, G5, G6.
**Owner:** rpg-toolkit `encounter`.
**Prerequisites:** T7 (same PR).
**Files:** `roomknowledge.go` (`discoverCells` iterates the hex ring of radius sight + 1 around the observer, cold and warm; `learnCells` unchanged), `roomreveal.go` (`appendCellsRevealedBeat` only for seats; the session's own fold of monster beats, if any, reads none), `encounter.go` (`rebuildPercepts` unchanged in shape).
**Behavior:** a warm step's `refreshSight` costs each observer its ring; a monster's facts persist and its reload skips no knowledge (G5).
**Tests:** `TestDiscoverCells_EqualsFullSweep` (castle and keep: the bounded sweep learns the same set as the full one, cold and after a step); `TestMonster_WritesNoRevealBeat`; `TestMonster_ReloadKeepsKnowledge`; `BenchmarkLaunch_Keep10k` (< 2 s, from 18.6 s) and `_Keep30k` (< 10 s, from 149 s); `BenchmarkRefreshSightWarm_Keep10k` (< 50 ms, from 1,169 ms).
**Verification:** module gates; castletest green; numbers in the PR body; the session re-pins and posts the live StartEncounter and Move rows from the bench env.
**Completion evidence:** PR link, benchmark lines, rows on #753.

**Finding (toolkit #2212, 2026-10-31):** the bound is sight + 1, not reach + 1: `seesCell` already refused every cell beyond the observer's sight range, so bounding the sweep alone moved launch by nothing. The 18.6 s sat elsewhere, all three fixed in the same PR: every sight ray tested all 654 placements on the keep (authored placements are now bucketed by position in T1's placement index and rays, steps and cell checks ask only the nearby ones); every monster that learned cells built two full projections for a reveal beat nobody reads (monsters write none); fight formation folded the faction graph once per member pair (now once per faction pair); plus a one-pass ray memo for the warm refresh. Launch 10k 18.6 s → 1.37 s, 30k 149 s → 4.14 s, warm refresh 1,169 → 33 ms, blob after launch 7.28 → 2.16 MB (10k).

### Task T9: knowledge written once, compactly (S4, encounter)

**Delivers:** V7, V12 (data half).
**Owner:** rpg-toolkit `encounter`.
**Prerequisites:** T8.
**Files:** `roomknowledge.go` (`learnCells` writes the `known:cells` fact's cells as row runs, `cellFold` reads runs and the legacy one-per-cell form), `data.go` (`validateCellsKnown` decodes runs at load as the fold does), `roomreveal.go` (a seat's beat carries only cells and placements not already in the seat's known set).
**Behavior:** the facts and beats decode to the same known sets as before; the 10k keep's marshalled `EncounterData` after launch is under 1 MB; the per-verb load falls with it.
**Tests:** `TestKnownCellsFact_RunsRoundTrip` (legacy and runs decode equal); `TestRevealBeat_DeltaOnly`; `TestBlobSize_Keep10k` (< 1 MB after launch); the fog and reveal tests unchanged.
**Verification:** module gates; the bench env's Redis blob size and the Move row posted on #753.
**Completion evidence:** PR link, blob bytes before/after, rows.

### Task T10: a refresh costs its reach (S7 #762, encounter)

**Delivers:** V12's Move row; the-world-runs R8's "wakes nobody" row.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** T8 (the warm sweep is bounded per observer); the-world-runs S4 (the wake pass is the raise's only roster walk).
**Finding:** after T8 one observer's warm refresh is ~33 ms, and a Move on the keep still costs seconds because the callers refresh every observer: `worldtime.go` refreshes `rosterIDs()` after a raise that moved anyone; `hold.go`, `reserve.go`, `rest.go`, `sneak.go`, `doorverbs.go`, `board.go` do the same. A player's own step refreshes `audienceFor` (players), so the roster-wide cost is paid on the raise and on every monster move inside it.
**Files:** `encounter.go` (`observersOf(cells []Position, declared []MemberID) []MemberID`: every placed member whose cell is within the sight bound (the Sight capability's max range, the same bound T8's sweep uses) of any cell in `cells`, plus `declared`, sorted by id; built from the canvas's entity positions, never from `rosterIDs()` filtered after the fact); `worldtime.go` (the wake pass collects the from/to cells of every member a step moved and refreshes `observersOf(those)`); `hold.go`, `reserve.go`, `rest.go`, `sneak.go` (the actor's from/to cells); `doorverbs.go` (the door's cells: a door changes every ray that crosses it, so its observers are those within the bound of the door); `board.go` unchanged (launch is the one look).
**Interfaces:** `observersOf` is the only way a verb names its refresh audience besides `audienceFor`; `refreshSight(e.rosterIDs())` remains legal only in `board.go` and is named there as the launch's one look.
**Behavior:** an observer farther than the sight bound from every changed cell is not refreshed, and its percepts are unchanged by construction (nothing it could see moved); a fight forms the same, because the members who came into view are within the bound of the mover by definition.
**Tests:** `TestObserversOf_EqualsFullRefresh` (property: castle and 10k keep, random steps and door openings, the bounded refresh's percepts equal the full refresh's for every member); `TestObserversOf_FarObserverUntouched` (a probe counter on `rebuildPercepts` sees no call for a member beyond the bound); `TestDoor_RefreshesRayCrossers` (a door opening refreshes an observer 20 hexes away whose line crosses it and not one 30 hexes away); `BenchmarkGarrisonMove` posts-asleep row (< 500 ms/op from 1.82 s; the raise's 20 patrols each pay their own ring); the harness Move row on the 10k keep with a raise (< 1 s from 3.6–4.5 s; the 100 ms bar then needs the patrols' turns themselves, measured and posted as the next finding, not assumed).
**Verification:** rows on #753 and #766 the day it lands; none worse.
**Completion evidence:** PR link, the two rows.

### Task T11: a route costs its reach (S7 #762, encounter)

**Delivers:** V12's Move row, the part T10 left; R10 on the lane (#754, 2026-11-01: "A and work to C").
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** T10 merged (same module; never stack).
**Finding (T10 build, rpg-toolkit#2224):** ~89% of a patrol's Step is `buildMonsterView → cellQuery.routeTo → floodFrom`, an unbounded flood of the field (`limit 0`) per mover per turn to find the nearest stop for one goal; the refresh T10 bounded is ~10%. The flood memo lives inside one query read, and the session reloads the encounter from its blob on every verb.
**Files:** `clocks.go` (`cellQuery.routeTo` and `routeToRemembered` for a single goal cell or a goal predicate with a known target: a goal-directed search, A* over the hex grid with the hex distance as its admissible heuristic, the same walker rules, neighbour order and tie-breaks as the flood, so the path is deterministic (C8) and equals the flood's path wherever costs are unequal; where the flood's nearest-stop scan matters (a goal predicate with many satisfying cells, e.g. "within reach of X"), the search targets the predicate's anchor and stops at the first satisfying cell it settles, with the same distance/X/Y tie order); `directive.go` (`routeToward` keeps its budgeted flood: it is already bounded).
**Interfaces:** `routeTo`'s signature and contract unchanged; the flood stays for budgeted and multi-source questions.
**Behavior:** a route visits cells near its path, not the field; where two paths tie, the chosen one may differ from the flood's; every such golden change is listed in the PR with the two paths, and any that changes a castle walk's spirit is a finding, not a silent update.
**Tests:** `TestRouteTo_EqualsFlood` (property: castle and the 10k keep, random movers and goals; the A* path has the flood path's length and both satisfy the goal; identical where the flood's path is unique); `TestRouteTo_Deterministic` (asked twice, same answer); `TestRouteTo_VisitsNearThePath` (a probe counter: cells settled ≤ k × path length on the open keep, k stated); `BenchmarkGarrisonMove` default and posts-asleep on main and branch, 20x, same machine (contract: posts-asleep < 500 ms/op from ~2.0 s); the harness Move row on the 10k keep with a raise (< 1 s from 3.6–4.5 s).
**Verification:** rows on #753 and #766 the day it lands; none worse.
**Completion evidence:** PR link, the two rows, the golden list.

**C, the memory-resident instance,** is rpg-project#770's design, not a task here: it would let the flood memo and the placement index live across verbs; R10 names it as where this line of work goes next.

### Task T12: one load per verb (S7 #762, session)

**Delivers:** V12's Move row, the largest remaining piece.
**Owner:** rpg-toolkit `rulebooks/dnd5e/resolution` then `rulebooks/dnd5e/session`, two PRs, one module each.
**Prerequisites:** none; independent of encounter PRs.
**Build finding (2026-11-02):** the reload is inside resolution: `Input.World` is `EncounterData` and `resolveOn` calls `LoadEncounter` on every `Resolve`; resolution only reads the result. Attack-shaped verbs then `adopt` the round-tripped `Output.World` in the session, a third load. So: (1) resolution's `Input` gains `Live`, the narrow read interface resolution uses (satisfied by `*encounter.Encounter`), exactly one of `Live`/`World` set, `Output.World` empty when live, resolution still mutates nothing (asserted); (2) the session passes the verb's live encounter and `land()` skips `adopt` when the world came back live.
**Finding (probe, 2026-11-02, #753 comment 6568366323):** a one-cell Move on the 10k keep loads the whole encounter twice: once at the verb's open (`LoadEncounter`, 87 ms, of which partition derivation ~45 ms) and again inside the step's announcement, where `resolution.Resolve` is handed a `WorldView()` that rebuilds an encounter from the saved data (~85 ms of the 92 ms announcement). The two loads are 58% of the ~300 ms.
**Files:** the session's step/announce path (where `WorldView()` is built for `resolution`): the view must be backed by the encounter instance the verb already holds, never by a second `LoadEncounter` from data. The seam stays: resolution still receives a view interface and knows nothing of the encounter; the session, which owns both, hands it the live one.
**Interfaces:** `resolution`'s view interface unchanged; the session's view adapter takes an `*encounter.Encounter` (or the narrow interface it already satisfies) instead of `EncounterData`.
**Behavior:** every verb loads the encounter once; the announcement reads the same instance the step mutated, so what resolution sees is the post-step world (as the reload saw it), byte-identical acts and events.
**Tests:** a probe counter on `LoadEncounter` calls per verb (Move, Speak, EndTurn, door verbs, and Attack, Cast, Activate, React, today three): exactly one; the castle harness and session goldens unchanged; the session-level Move benchmark on the garrison record (the probe's method: load → step → announce → save, median of 45) before and after, both in the PR.
**Verification:** the harness Move row on the 10k keep (expected ~215 ms from ~300).
**Completion evidence:** PR link, the row.

### Task T13: partitions derived when asked (S7 #762, encounter)

**Delivers:** V12's Move row, the next piece after T12.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** T12 measured; no other encounter PR in flight (never stack).
**Finding:** `LoadEncounter` derives the field's partitions on every load (~45 ms of 87 on the 10k keep); a plain Move reads partitions only where a sound hops or a door changes.
**Files:** the field's partition derivation becomes lazy (computed on first read, memoized for the instance's life; never stored: the law "the field indexes its placements, no stored derived data" holds, and the memo dies with the verb until #770's instance lives longer).
**Tests:** `TestPartitions_DerivedOnce` (probe counter), every partition test unchanged, `BenchmarkLoadEncounter_Keep10k` before/after.
**Verification:** the harness Move row (expected ~170 ms); the remaining ~100 ms is the bounded refresh's reveal flood (~43 ms) and JSON (~27 ms), which are #770's ground.

## Requirement coverage

| requirement | tasks | proof |
|---|---|---|
| V1 | T1, T2 | `TestWithinReach_Ring`; `TestSpeakMenu_WithinReachOnly`; `BenchmarkAfford_Keep10k` |
| V2 | T2 | `TestSpeakMenu_HiddenPropNotOffered`; `TestSpeak_OutOfReachRefused` |
| V3 | T1, T2 | `TestShownTo_MatchesAtlasFor`; `TestOptionsFor_NoProjection`; `TestSpeakMenu_UnsightedCreatureNotOffered` |
| V4 | T1 | `TestPlacementIndex_UpdatesOnMove` |
| V5 | T1, T7 | `BenchmarkAtlas_Keep30k`; goldens; no-cell-pass counter |
| V6 | T8 | `TestDiscoverCells_EqualsFullSweep`; `TestMonster_WritesNoRevealBeat`; `TestMonster_ReloadKeepsKnowledge` |
| V7 | T9 | `TestKnownCellsFact_RunsRoundTrip`; `TestRevealBeat_DeltaOnly`; `TestBlobSize_Keep10k` |
| V12 | T8, T9, T10, T11, T12, T13 | `BenchmarkRefreshSightWarm_Keep10k`; the harness Move row on both keeps |
| V8 | T4 | `TestLocker_SessionsDoNotShareAKey`; the castle-beside-keep row |
| V9 | T6 | repository + handler tests; `TestResolveDiscoverySharing_FromReader`; the web switch persists |
| V10 | T5 | vitest approach test; the operator's walk |
| V11 | every task | rows on #753 |
| K1 | T2, T7, T8 | castletest, goldens, fog tests |

## Provider/consumer seams

| provider | consumer | contract | availability | proof |
|---|---|---|---|---|
| T1 `shownTo`, `placementIndex`, `WithinReach`, Speak's reach | T2 menu, T7 projection | same answers as today, no projection; ring of 3; reach after shown and witness | encounter tag | property test; ring test; `TestSpeak_OutOfRange` |
| T2 bounded menu | web via Afford (unchanged wire) | `Declaration.speak_options` rows for targets within reach | session tag → api re-pin | castle walk |
| T4 locker | every session verb | per-session guard | api dev | interleaving test; castle-beside-keep row |
| T6 `CharacterConfigReader` | session `resolveDiscoverySharing` | `DiscoverySharing(ctx, character) (*bool, error)` | session tag + api store | tests |
| T1/T7 index | `Atlas`, `AtlasFor`, `WithinReach`, the sweep memo | cell → placements and walls, linear build, writer-updated | encounter tag | goldens; no-cell-pass counter |
| T6 `CharacterConfigReader` | T4 locker | the preference is off the session's write path | T6 merged before T4 | locker test; `make test` |
| harness | every slice | rows | game-dev main | #753 comments |

## Open items

- O1 reach = 3 (R4, G1), measured by the encounter's one distance; the walls question is the walk's; O2 taken (G5); O3 row runs, the first blob after T9 decides whether beats follow; O4 only discovery sharing (G8), the session's verb removed.
- Finish-line numbers are defaults the operator may move on the design PR.
- Blockers: none. T5 needs T2 on a stack for the operator's walk; T4 waits for T6.
