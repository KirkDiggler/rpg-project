---
name: Server at the keep's scale — implementation plan
status: PLAN derived 2026-10-31 from design.md R1–R4 and README.md, on the S1 probe report (rpg-project#753); checked against the inspected baseline below. R4 (interaction by reach) removes the TargetOptions RPC: the wire is unchanged for interaction.
authority: design.md (R1–R3, Law), README.md (ownership table and refusals), story rpg-project#753, lane rpg-project#754, S1 probe report (comment on #753)
planning guide: ../../.agents/skills/design/planning.md
---

# Server at the keep's scale — implementation plan

## Goal and scope

A keep launches, answers a player's Afford and a one-target read, and takes a
Move inside site format R1's budgets, with sessions locked by session and the
discovery-sharing preference in a character configuration store. Every slice
posts its before and after rows from `tools/browser/bench/run.sh bench` on
#753 the day it lands, and no row gets worse.

**Non-goals.** The studio's own per-commit copies and polygons; play's one
mesh per tile and GLB clone per prop; distributed locking; preferences other
than discovery sharing (O4); the approach's stopping rule beyond O1's default.

**Repositories and order.** Develop outside-in, merge inside-out.

| slice | repo / module | base | PR shape |
|---|---|---|---|
| S2 | rpg-toolkit `encounter` → `session`; rpg-dnd5e-web | main / main / dev | encounter PR (oracle, index, `WithinReach`); session PR (speak menu bounded by reach); web PR (click a far target → Move toward it); no protos or api change |
| S5 | rpg-api; rpg-api-protos; rpg-toolkit `session` | dev / main / main | api PR (locker per session — independent, first); protos PR (`SetPreference`); session PR (`CharacterConfigReader`); api PR (store) |
| S3 | rpg-toolkit `encounter` | main | one PR: atlas cache, index-backed footprints, reach-bounded sweep, no monster beats |
| S4 | rpg-toolkit `encounter` (+ `session` if a reader moves) | main | one PR: known cells have one home |

S2 and S5 are independent and may run in parallel; S3 after S2's encounter PR
(it shares `placementIndex`); S4 after S3. Merge each slice toolkit → protos →
api → web.

## Finish line

- Sub-issues S2–S5 under #753 closed by their "done means" with rows posted.
- On the bench env: `bench-keep` launches under 2 s and `bench-keep-30k` under
  10 s (from 18.6 s / 149 s); a player Afford on either under 100 ms (from
  320 s) with the menu within reach included; Move under 100 ms; the castle's
  rows unchanged or better; a castle launch beside a live keep no slower than
  alone.
- The session blob of the 10k keep under 1 MB (from 7.3 MB).
- `sharedStoreLocker` gone; `character-config:<id>` holds discovery sharing.

## Requirement labels

| label | requirement | source |
|---|---|---|
| V1 | `readSpeakMenu` offers only the creatures and props within interaction reach (3 hexes) of the member, found through `encounter.WithinReach`; nothing farther is asked about; `speakRowOnTheWorldClock` reads the same bounded menu. | R1, R4 |
| V2 | The rows keep today's shape (`Declaration.speak_options` with the Speak selector id); a target within reach that cannot be shown or does not witness the member is simply not offered. | R1, R4 |
| V3 | `encounter.shownTo(member, cells)` answers from the member's known cells in perception; `OptionsFor`/`PropOptionsFor` call no `Atlas`/`AtlasFor`. | Law Knowledge is the oracle |
| V4 | `field.placementIndex` maps cell → placements and walls; `placedCells` and `wallFootprintCells` read it; it is rebuilt only on placement change. | Law The field indexes |
| V5 | `Encounter.Atlas` returns a cached projection until a structural change (door state; placement moved, arrived, gone) invalidates it; `AtlasFor(member)` filters the cache by knowledge. | Law Project once |
| V6 | `discoverCells` tests only cells within the observer's sight reach; a monster observer writes no `cells_revealed` beat. | Law A sweep covers what can be seen; O2 |
| V7 | No `known:cells` journal fact is written; readers take the set from perception; a seat's `cells_revealed` beat carries only newly revealed cells. | Law Known cells have one home |
| V8 | rpg-api locks session verbs by session id; `sharedStoreLocker` is deleted. | R2 |
| V9 | `CharacterService.SetPreference{character, discovery_sharing}` writes `character-config:<id>` under the character guard; the session reads `DiscoverySharing` through `sdk.CharacterConfigReader`; `SessionService.SetDiscoverySharing` is deprecated in place and delegates. | R2 |
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
| G1 the approach's stop | reach: three hexes (R4); the client walks a far target into reach and the next Afford carries its rows | R4, O1 |
| G2 witness for a prop | a prop has no eyes; a prop within reach is offered on `shownTo` alone | Law Knowledge is the oracle |
| G3 what "known cells" is | `perception`'s per-observer known set as `learnCells` maintains it today; S4 makes it the only copy | Law Known cells |
| G4 the cache's writers | door open/close, `Hold`/`Drop`/arrival of a placed prop, `Launch`; each calls `invalidateAtlas()` on the encounter | Law Project once, O3 |
| G5 a monster's beat today | `appendCellsRevealedBeat` runs per learning observer; S3 gates it on the observer being a seat | O2 |
| G6 reach bound | `sightReach` already carries each observer's distances; the sweep iterates cells within the max distance of the observer's cell (hex ring) instead of `e.field.cells` | Law A sweep covers |
| G7 reach and the clocks | the bounded menu is read where the menu is read today: on the turn clock in `compileOffersFor`, on the world clock in `speakRowOnTheWorldClock`; pricing unchanged | README |
| G8 store key and shape | `character-config:<character>` JSON `{discovery_sharing: bool|null}`; absent means not chosen, as `resolveDiscoverySharing` reads today | R2, O4 |

## Tasks

### Task T1: the oracle and the placement index (S2, encounter)

**Delivers:** V3, V4.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** none.
**Files:** modify `hold.go` (`showsCellTo`, `showsAnyCellTo` → `shownTo(member MemberID, cells []spatial.Position) (bool, error)` reading perception's known set; delete the `AtlasFor` call), `option.go` (`speakableProp` uses `shownTo`), new `placement_index.go` (`placementIndex` built in `compileField` and on placement change; `placedCells`, `wallFootprintCells` read it), `atlas.go` (`placedCellsOf` reads the index), new `reach.go` (`WithinReach(member MemberID, hexes int) (members []MemberID, props []PropID, error)`: the hex ring around the member's cell through the index and the roster's cells).
**Interfaces:** `shownTo` semantics equal today's `AtlasFor`-based answer for every cell the member knows; `placementIndex.at(cell) []placementRef`; `WithinReach` lists every creature and placed or cell prop with a cell inside the ring, sorted by id, excluding the member.
**Tests:** `TestShownTo_MatchesAtlasFor` (property test on the castle and the 10k keep: for every member and every placement, `shownTo` equals the old answer); `TestPlacementIndex_RebuildsOnMove`; `TestWithinReach_Ring` (ring of 3 = 37 cells; a prop with one footprint cell inside counts; adjacent-only when hexes = 1); `BenchmarkPropOptionsFor_Keep10k` (< 1 ms, from ~550 ms); `TestOptionsFor_NoProjection` (a probe counter asserts zero `Atlas` calls).
**Verification:** `go test -race ./... && golangci-lint run ./...`; the benchmark number in the PR body.

### Task T2: the speak menu within reach (S2, session)

**Delivers:** V1, V2, G1, G2, G7.
**Owner:** rpg-toolkit `rulebooks/dnd5e/session`.
**Prerequisites:** T1 tag.
**Files:** modify `offers.go` (`readSpeakMenu` takes its targets from `encounter.WithinReach(member, InteractionReach)` instead of `enc.Witnesses` + `speakableProps`; `speakableProps` and its `enc.Atlas()` are deleted; the witness test runs per creature within reach; `const InteractionReach = 3` with its doc naming R4), `afford.go` (`speakRowOnTheWorldClock` unchanged in shape, reads the bounded menu); `speak.go`'s door unchanged; `buildSpeakOffer` unchanged.
**Interfaces:** rows carry the same selector id `Speak` validates today; a target outside reach is not a row; `Speak` on a target outside reach is refused at the door with the existing not-witnessed/not-shown reasons plus `ShortfallOutOfReach`.
**Tests:** `TestSpeakMenu_WithinReachOnly` (castle: a guard four hexes off is not offered, at three he is); `TestSpeakMenu_HiddenPropNotOffered`; `TestSpeak_OutOfReachRefused`; castletest script steps that speak move within reach first (the walk captures the spirit: the player walks up); `BenchmarkAfford_Keep10k` (< 100 ms, from 369 s).
**Verification:** module gates; benchmark numbers in the PR body; castletest green.

### Task T3: not applicable (S2, protos)

R4 keeps the interaction wire as it is: `Declaration.speak_options` carries the rows within reach. No protos change for S2. (S5's `SetPreference` is in T6.)

### Task T4: the lock (S5, api)

**Delivers:** V8.
**Owner:** rpg-api.
**Prerequisites:** none (independent of S2; merges first).
**Files:** modify `internal/orchestrators/session/orchestrator.go:150-153` (use `NewInProcessSessionLocker()` directly), delete `sharedStoreLocker` from `exploration.go`; the discovery-sharing write keeps correctness through T6 (until then it is the one cross-session write, guarded by the character key the SDK already takes around it — verify in `session/exploration.go:182`; if it does not, T4 waits for T6).
**Tests:** `TestLocker_SessionsDoNotShareAKey` (two sessions' verbs interleave); the castle integration tests.
**Verification:** `make test`, `make pre-commit`; on the bench env, a castle launch beside a live keep session takes the castle's own time.

### Task T5: walk to a far target (S2, web)

**Delivers:** V10.
**Owner:** rpg-dnd5e-web.
**Prerequisites:** T2 on a stack for the walk (no wire change).
**Files:** `combat-experience/TargetSurface.tsx` (a creature or prop beyond reach is clickable; the click issues a Move toward it, stopping within reach), `useSessionCombatExperience.ts` (the approach intent), `ConversationPanel.tsx` and `speakOptions.ts` unchanged except that an empty menu reads "nobody within reach".
**Tests:** vitest: click a far creature → Move intent toward it; after the view refresh with the creature within reach, the panel shows its rows from Afford; nothing within reach → the empty-menu text.
**Verification:** `npm run ci-check`; the operator walks the castle: click the cook from the hall, the character walks over, the cook's lines appear; stand next to the altar, its options appear without a click.

### Task T6: character configuration (S5, protos + session + api)

**Delivers:** V9, G8.
**Owner:** rpg-api-protos (`CharacterService.SetPreference`), rpg-toolkit `session` (`CharacterConfigReader` replaces `ExplorationRepository`'s write path; `SetDiscoverySharing` delegates or is removed from the SDK), rpg-api (`internal/repositories/characterconfig` Redis `character-config:<id>`; `CharacterService.SetPreference` handler under `LockCharacter`; the session's reader wired).
**Tests:** session: `TestResolveDiscoverySharing_FromReader`; api: repository round-trip, handler locks the character, `SetDiscoverySharing` (session) still works via delegation.
**Verification:** module gates; `make test`; the discovery-sharing switch in the web still flips and persists.

### Task T7: project once, index-backed footprints (S3, encounter)

**Delivers:** V5, G4.
**Owner:** rpg-toolkit `encounter`.
**Prerequisites:** T1.
**Files:** `atlas.go` (`Atlas` reads `e.atlasCache` or derives and stores; `invalidateAtlas()`), the writers per G4, `placed_props.go` / `structural_walls.go` (`footprintObservationCells` via the index).
**Tests:** `TestAtlas_CachedUntilStructuralChange` (door toggle invalidates; a Move does not); `BenchmarkAtlas_Keep30k` (< 50 ms warm, from 2.1 s); the atlas goldens byte-identical.
**Verification:** module gates; goldens; number in the PR body.

### Task T8: reach-bounded sweep, no monster beats (S3, encounter)

**Delivers:** V6, G5, G6.
**Owner:** rpg-toolkit `encounter`.
**Prerequisites:** T7 (same PR).
**Files:** `roomknowledge.go` (`discoverCells` iterates the hex ring within the observer's max sight distance), `roomreveal.go` (`appendCellsRevealedBeat` only for seats), `encounter.go` (`rebuildPercepts` unchanged in shape).
**Tests:** `TestDiscoverCells_EqualsFullSweep` (castle and keep: the bounded sweep learns the same set as the full one); `TestMonster_WritesNoRevealBeat`; `BenchmarkLaunch_Keep10k` (< 2 s, from 18.6 s) and `_Keep30k` (< 10 s, from 149 s).
**Verification:** module gates; castletest green; numbers in the PR body; the session re-pins and posts the live StartEncounter row from the bench env.

### Task T9: known cells have one home (S4, encounter + session)

**Delivers:** V7.
**Owner:** rpg-toolkit `encounter` (and `session` if a reader of `known:cells` lives there).
**Prerequisites:** T8.
**Files:** the writer of the `known:cells` journal fact (located by grep in `roomknowledge.go` / journal), its readers → perception.
**Tests:** `TestJournal_NoKnownCellsFact`; `TestBlobSize_Keep10k` (marshalled `EncounterData` under 1 MB after launch); readers' tests pass against perception.
**Verification:** module gates; the bench env's Redis blob size posted on #753.

## Requirement coverage

| requirement | tasks | proof |
|---|---|---|
| V1 | T1, T2 | `TestWithinReach_Ring`; `TestSpeakMenu_WithinReachOnly`; `BenchmarkAfford_Keep10k` |
| V2 | T2 | `TestSpeakMenu_HiddenPropNotOffered`; `TestSpeak_OutOfReachRefused` |
| V3 | T1 | `TestShownTo_MatchesAtlasFor`; `TestOptionsFor_NoProjection` |
| V4 | T1 | `TestPlacementIndex_RebuildsOnMove` |
| V5 | T7 | `TestAtlas_CachedUntilStructuralChange`; goldens |
| V6 | T8 | `TestDiscoverCells_EqualsFullSweep`; `TestMonster_WritesNoRevealBeat` |
| V7 | T9 | `TestJournal_NoKnownCellsFact`; `TestBlobSize_Keep10k` |
| V8 | T4 | `TestLocker_SessionsDoNotShareAKey`; the castle-beside-keep row |
| V9 | T6 | repository + handler tests; the web switch persists |
| V10 | T5 | vitest approach test; the operator's walk |
| V11 | every task | rows on #753 |
| K1 | T2, T7, T8 | castletest, goldens, fog tests |

## Provider/consumer seams

| provider | consumer | contract | availability | proof |
|---|---|---|---|---|
| T1 `shownTo`, `placementIndex`, `WithinReach` | T2 menu, T7 cache | same answers as today, no projection; ring of 3 | encounter tag | property test; ring test |
| T2 bounded menu | web via Afford (unchanged wire) | `Declaration.speak_options` rows for targets within reach | session tag → api re-pin | castle walk |
| T4 locker | every session verb | per-session guard | api dev | interleaving test; castle-beside-keep row |
| T6 `CharacterConfigReader` | session `resolveDiscoverySharing` | `DiscoverySharing(ctx, character) (*bool, error)` | session tag + api store | tests |
| T7 cache | T8 sweep, `AtlasFor` readers | invalidation on the G4 writers only | same PR | cache test |
| harness | every slice | rows | game-dev main | #753 comments |

## Open items

- O1 reach = 3 (R4, G1); O2 taken (G5); O3 decided by T7's first row; O4 only discovery sharing (G8).
- Blockers: none. T5 needs T2 on a stack for the operator's walk; T4 may wait for T6 if the discovery-sharing write is not already under the character guard.
