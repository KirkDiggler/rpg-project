# Places, ways and the engine's read-back — implementation plan, wave one

## Header

- **Goal and scope.** Put the engine's read-back in the studio's inspector, make refusals visible from anywhere, and have the engine say which places join. Wave one is slices rpg-project#967, #968, #969 and #971. Slices #970, #972, #973, #974 and #975 are planned when wave one's interfaces are on `dev`; they are in scope for the story, not for this plan.
- **Finish line.** Each task's PR merged on tags (protos, toolkit) or on `dev` (api, web); the operator's stack shows the Guard Captain's routine on click and a refusal count in the header; the measurements #967 asks for are written on that issue.
- **Authority.** [design.md](design.md) R25, R30, R31, R34; the brief and agreement on rpg-project#966.
- **Constraints.** Protos change additively. One toolkit module per PR. The studio writes no sentence about what will run (R25). No geometry message beside the atlas (R31). An unedited castle downloads byte for byte. Studio tests find controls by accessible name; a renamed label is a deliberate change with its test. Build rules: `.agents/skills/build-slice/SKILL.md`.
- **Inspected baseline.** toolkit `main` 4ff62fe5, api `dev` 20af6ffd, web `dev` 285152a2, protos `main` 5f3c307.
- **Sequence.** T1 → (protos tag) → T2 → T3. T4 and T5 start at once and depend on nothing above. T3 and T4 both touch `EncounterStudioWorkspace.tsx`; whichever opens second merges `origin/dev` after the first lands.
- **Open items.** The read-back's size on the wire is answered by T2's measurement. Whether open floor and a doorless gap are told apart is answered on #971 by T5; until then both are one kind.

## Tasks

### Task 1: the read-back message on the wire

**Delivers:** R25, R31 (slice #967, protos half).
**Owner:** rpg-api-protos, `dnd5e/api/authoring/v1alpha1/service.proto`.
**Prerequisites:** none.
**Files:** modify `dnd5e/api/authoring/v1alpha1/service.proto`.
**Interfaces:** new messages mirroring `dungeonspec/readback`'s exported JSON shape exactly: `ReadBack{string key=1; string name=2; repeated ReadBackSection sections=3; repeated ReadBackUnsaid not_yet_said=4}`, `ReadBackSection{string title=1; repeated ReadBackGroup groups=2}`, `ReadBackGroup{string title=1; ReadBackSubject about=2; repeated ReadBackStatement statements=3}`, `ReadBackStatement{string kind=1; ReadBackSubject about=2; string sentence=3; string path=4; repeated ReadBackConnection connects=5}`, `ReadBackSubject{string kind=1; string id=2}`, `ReadBackConnection{string kind=1; string id=2; string role=3}`, `ReadBackUnsaid{string path=1; string kind=2; string why=3}`. New field `ReadBack read_back = 3` on `PutDungeonResponse`. Kinds and roles are strings, never enums: the engine adds kinds without a wire change.
**Behavior:** the field is absent when the file does not compile or is not a site document. Its doc comment says so, and says it is sentences, not a second projection of the site.
**Tests:** none hand-written (repo rule). `buf lint`, `buf breaking` against `main`, `make generate`, `make test`.
**Verification:** the repo's gates pass; the generated TypeScript exports `ReadBack` from the authoring package.
**Completion evidence:** PR on `main`; after merge, the tag and the `gen/go` pseudo-version, reported to the lead.

### Task 2: the api says the read-back on the check

**Delivers:** R25 (slice #967, api half).
**Owner:** rpg-api, `internal/dungeons` and the authoring handler.
**Prerequisites:** T1's tag.
**Files:** `internal/dungeons/registry.go` (`compileEntry`), the `Entry` it returns, the `Put` result, `internal/handlers/.../put_dungeon.go` (the handler that maps `out.Errors` and the atlas), `go.mod` (protos pin).
**Interfaces:** consumes `readback.Of(raw []byte) (*readback.ReadBack, error)` from `github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter/dungeonspec/readback` (present at the pinned `v0.215.1`). Produces `PutDungeonResponse.read_back`, mapped field for field.
**Behavior:** on a compile with no errors, validate-only or a save, the response carries the read-back. On any refusal it carries none. `readback.ErrNotASite` is not an error to the caller: the response carries no read-back. A read-back failure after a clean compile is logged and the response carries none; it never turns a clean check into a refusal.
**Tests:** `TestPutDungeon_ValidateOnly_CarriesReadBack` (the castle: statements present, one with `about {creature, captain}` and kind prefixed `routine` or `creature`, `not_yet_said` empty); `TestPutDungeon_Refused_CarriesNoReadBack` (a line with both `say` and `do`); `TestPutDungeon_OlderDialect_CarriesNoReadBack`.
**Verification:** `make test` in rpg-api. Measure on the castle, in a benchmark or a timed test, the added time per check and the response's encoded size with and without the read-back; write both on rpg-project#967.
**Completion evidence:** PR on `dev`; the two numbers on the issue.

### Task 3: the inspector speaks the engine's sentences

**Delivers:** R25 (slice #968). Closes rpg-project#944.
**Owner:** rpg-dnd5e-web, `src/concepts/encounter-studio` and `src/concepts/world-building`.
**Prerequisites:** T1's tag in `package.json`; T2 on the stack.
**Files:** `src/author/authoringRpc.ts` (`usePutDungeonPreview`: keep the last read-back), `src/concepts/world-building/useRoomPublishing.ts`, `src/concepts/encounter-studio/ReadBack.tsx`, `StudioInspector.tsx` (the mounts for creature and prop, `readBackRefHandler`), `refusalJump.ts` (export the path-to-target function used by `refusalJumps`); delete `src/concepts/world-building/readBack.ts` and `readBack.test.ts`, and move the tests that call `readBackOf` (`captainGoesInRoundTrip`, `shoutRoundTrip`, `statesRoundTrip`, `actionAugments`, `factionOptions`, `predicateNot`) to assert on the document or drop the read-back assertion with the reason in the PR.
**Interfaces:** consumes `PutDungeonResponse.readBack`. The studio session exposes `{readBack, readBackIsCurrent}`: `readBackIsCurrent` is false from the first edit after a successful check until the next successful check.
**Behavior:** for a selected creature or prop, show the statements of every group whose `about` is `{creature|prop, <id>}`, in the engine's order, under the group's title. A connection of kind creature, prop, door or region is a jump to that selection; fact and topic open the Facts section on that id; conversation opens Interactions; a statement's path is a jump through the refusal map when it resolves elsewhere. No statement text is altered. When `readBackIsCurrent` is false the panel says the sentences are from the last draft that compiled. With no read-back at all it says none is available and why (not compiled yet; authoring off).
**Tests:** `ReadBack.test.tsx`: renders the captain's routine statements from a recorded castle read-back fixture; a fact connection is a button that calls the jump; the stale notice shows when not current; nothing renders for a subject with no group. `EncounterStudioIntegration.test.tsx`: selecting the captain shows text from the fixture's `routine` statement.
**Verification:** the repo's test, lint and typecheck gates; the stack boots and the castle's captain reads back his routine.
**Completion evidence:** PR on `dev` with a Try-it copied from #968's checklist.

### Task 4: a refusal is seen from anywhere

**Delivers:** R34 (slice #969).
**Owner:** rpg-dnd5e-web, `src/concepts/encounter-studio`.
**Prerequisites:** none.
**Files:** `EncounterStudioWorkspace.tsx` (header; the `RowRefusalsContext` provider now wraps Tables and Bundles only), `StudioPublish.tsx` (the refusal list today), `studioPublishing.ts` (`currentRefusals`), `refusalJump.ts`.
**Interfaces:** none new on the wire. The header reads the same `currentRefusals` and `refusalJumps` the Save panel reads.
**Behavior:** the header shows the count of current refusals on both homes and both views, absent at zero. Activating it lists each refusal in the engine's words with its jump. The row-refusal provider wraps the whole workspace so a row on the Factions section or in the inspector shows its own refusal. The Save panel's list stays.
**Tests:** `EncounterStudioWorkspace.test.tsx`: with one refusal the header names the count on Build and on Encounter; activating it lists the sentence; the jump selects the creature. Integration: a refused creature line shows its refusal in the inspector.
**Verification:** the repo's gates; the stack boots; a line with `say` and `do` counts one from the map side.
**Completion evidence:** PR on `dev` with a Try-it copied from #969's checklist.

### Task 5: the engine says which places join

**Delivers:** R30, R31 (slice #971).
**Owner:** rpg-toolkit, module `rulebooks/dnd5e/encounter` (the read-back package and what it needs exported beside `PartitionFloor`).
**Prerequisites:** none.
**Files:** `rulebooks/dnd5e/encounter/dungeonspec/readback/` (a new `joins.go`, `place.go`, `build.go`, `readback.go` for the kinds), and an exported helper in `rulebooks/dnd5e/encounter` only if door crossings cannot be had without constructing an `Encounter`.
**Interfaces:** three new kinds: `place.join.door`, `place.join.open`, `place.join.passage`. Each statement is `About{Kind:"join", ID:<stable id>}`, in a group of its own under THE PLACE. Connections: a physical join carries `{region, <a>, "joins"}` and `{region, <b>, "joins"}`, and a door join also `{door, <id>, "by"}`. A passage carries `{region, <to>, "to"}`, `{region, <from>, "from"}` where its carrier has a place (a prop's region; an arrest's area), and the carrier as `{prop|creature|conversation, <id>, "by"}`. The join's state is not restated: it is read from the door's, post's and concealment's existing statements through the connection. The id is stable across runs for an unchanged file and built from the two region ids and the door or carrier id.
**Behavior:** door joins come from each door's two sides looked up to their regions. Open joins come from two regions that share a partition of the floor (`PartitionFloor` treats every door as closed, so a shared partition means no wall between them): one statement per pair. Passages come from every `relocate` and every arrest and jail. A door whose two sides are one region says no join. A region in no join is said so in one statement of kind `place.region.unjoined`. Sentences follow the package's style: subject first, ids beside names, no pronoun.
**Tests:** on the castle: `TestJoins_Castle_GateDoorJoinsGatehouseAndCourtyard` (assert the two regions the map shows either side of `gate-wall-door`; read the site, do not assume), `TestJoins_Castle_GrateIsAPassageToChapelAndCells`, `TestJoins_Castle_ArrestIsAPassageToCells`, `TestJoins_Castle_EveryRegionIsJoinedOrSaidUnjoined`; on a small site under `testdata/`: `TestJoins_OpenFloor_TwoRegionsNoWall`, `TestJoins_WallBetween_NoJoin`, `TestJoins_StableIDs`. The package's "not yet said" and golden tests stay green or are regenerated with the diff explained.
**Verification:** the module's gates under race; `go run ./cmd/readback` on the castle prints the joins. Write on #971 whether a doorless gap can be told from open floor with what the compiled site holds, and at what cost.
**Completion evidence:** PR on `main`; the castle's join list pasted in the PR.

## Requirement coverage

| Requirement / scenario | Task | Proof |
|---|---|---|
| R25: the inspector's sentences are the engine's | T1, T2, T3 | T2's castle test; T3's fixture render and integration test; the captain on the stack |
| R25: a draft that does not compile shows the last sentences, marked | T3 | `ReadBack.test.tsx` stale notice |
| R25: one path-to-selection map | T3 | the path jump uses `refusalJump.ts`'s exported function |
| R31: one new field, no geometry message | T1, T5 | the proto diff; joins arrive as statements with no proto change |
| R30: the engine says place, join, kind | T5 | the castle and small-site tests |
| R34: a refusal is seen from anywhere | T4 | header tests on both homes |
| #967: size and time measured | T2 | numbers on the issue |
| The castle round-trips byte for byte | T3, T4 | `servedCastle.test.ts` stays green |

## Seams

| Seam | Producer | Consumer | Contract | Checked by |
|---|---|---|---|---|
| read-back on the wire | T1 | T2, T3 | the messages in T1, mirroring `readback`'s JSON | generation compiles in api and web |
| engine read-back → api | toolkit `readback.Of` at `v0.215.1` | T2 | `(*ReadBack, error)`; `ErrNotASite` | T2's three tests |
| api → studio | T2 | T3 | `readBack` present only on a clean compile | T3 integration on the stack |
| join statements | T5 | later slice #972 | kinds, subject and connections in T5 | T5's tests; no wire change needed |
| refusals → header | existing `currentRefusals` | T4 | unchanged | T4's tests |
