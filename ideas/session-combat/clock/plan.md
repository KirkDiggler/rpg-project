# Plan — a boundary is an interaction

Implements `design.md`, all three questions ruled 2026-08-27. **Five build PRs**,
bottom-up, one per module.

This file is a **ledger**: it records what changed from the plan and why, the way
`effect-context/plan.md` did. Steps get ticked as they land.

---

## Global constraints

- **Kirk merges every PR.** No draft PRs.
- **One Copilot round per PR**, requested via GraphQL `requestReviews` with
  `botIds:["BOT_kgDOCnlnWA"]`, verified by reading the PR node back — `gh pr view`
  is blind to bot reviewers. Reply in every thread. Never re-request. **Skip for
  doc-only PRs and pin bumps.** Never report a PR in the same turn its review was
  requested.
- **Closing keywords fire on toolkit `main` merges and are inert on `dev`-based
  repos** (rpg-api, web). An issue spanning several PRs carries the keyword only
  on the last.
- **The grid is hex.** Square remains an option; the product runs on
  `AxialHexGrid` exclusively.
- Commit trailers, and `— platform agent, on behalf of KirkDiggler` on anything
  published to GitHub.
- No `replace` directives, no `go.work`, ever committed.

---

### Task 1: Open and board the tracking surfaces

- [x] **Step 1: Slice issue** — rpg-project#294, carrying the evidence and the scope.
- [x] **Step 2: Design PR** — rpg-project#293. Stays open through implementation;
  rulings are recorded into it as they land.
- [x] **Step 3: Sub-issue link** — #294 under journey #253 via GraphQL `addSubIssue`
  with header `GraphQL-Features: sub_issues`. *Blocked 2026-08-27 on a GraphQL
  secondary rate limit; REST covered everything else.*
- [x] **Step 4: Project 19 fields** on #294 and each build issue — Area, Kind,
  Team, Status, Initiative, Readiness. `gh project item-list` needs `--limit 1000`.
- [ ] **Step 5: Build issues**, one per module, each a sub-issue of #294.

---

### Task 2: The subject of a turn — `rulebooks/dnd5e` — ✅ **DONE 2026-08-27** · [toolkit#1258](https://github.com/KirkDiggler/rpg-toolkit/pull/1258)

**PR 1 of 5.** A rename, and nothing else. Small on purpose: it is the only change
this module needs, and it must land before anything can publish.

**Files:**
- Modify: `rulebooks/dnd5e/events/events.go` — `TurnStartEvent.CharacterID` →
  `SubjectID`, same on `TurnEndEvent`.
- Modify: the seven condition handlers that compare it — `dodging.go`,
  `reckless_attack.go`, `unconscious.go`, `helped.go`, `sneak_attack.go`,
  `disengaging.go`, `raging.go` — plus `combat/turn_manager.go`.
- Modify: the condition tests that publish these events by hand.

**Interfaces:**
- Produces: a turn event whose subject field can describe a monster.

- [x] **Step 1: Rename the field on both events**, with a doc line saying what it
  denotes — whoever is taking the turn — rather than what is currently plugged in.
- [x] **Step 2: Update every reader.** All seven are `if event.CharacterID != x.CharacterID`
  guards; the condition's OWN field stays `CharacterID`, because a condition on a
  character's sheet really is a character's. Only the event's subject is renamed.
- [x] **Step 3: `go build ./... && go vet ./... && go test ./... && golangci-lint run`.**
- [x] **Step 4: Do NOT touch `RestEvent` / `CombatEndEvent`.** Same principle, out of
  scope — design.md §3.3.

**Ledger:** *(what changed from plan, and why)*

---

### Task 3: Carry the boundary, and announce it — `rulebooks/dnd5e/encounter` — ✅ **DONE 2026-08-27** · [toolkit#1259](https://github.com/KirkDiggler/rpg-toolkit/pull/1259)

**PR 2 of 5.** The largest, and the one the design is really about.

**Files:**
- Modify: `encounter/turndriver.go` — `Announcer`, `RefusingAnnouncer`, beside the
  capabilities that already live there and under the same
  *"SUPPLIED, NEVER DEFAULTED (rpg-toolkit#1033)"* law.
- Modify: `encounter/errors.go` — `ErrRefusingAnnouncer`, mirroring
  `ErrRefusingStriker` (`errors.go:312`).
- Modify: `encounter/clocks.go` — `Boundary`, `BoundaryKind`; translate milestones
  in `driveOneMonsterTurn` (`:461`) and `EndTurn` (`:1266`); announce in `form`.
- Modify: `encounter/data.go` — `LoadEncounterInput.Announcer`, `SetupInput` too.

**Interfaces:**
- Consumes: `clock.EndOutput.Milestones` and `clock.SetOrderOutput.Milestones`,
  which already exist and are already correctly ordered.
- Produces: `Announcer`, and `EndTurnOutput.Boundaries`.

- [x] **Step 1: The capability.** `Announce(ctx, enc, crossed []Boundary) error`.
  Required at construction; a nil one is refused at load, exactly as `TurnDriver`,
  `Standing`, `Sight` and `Initiative` are. `RefusingAnnouncer` returns
  `ErrRefusingAnnouncer` — **not a no-op**, because a clock cannot advance on a
  construction-only world and a silent default would be the very bug this slice fixes.
- [x] **Step 2: Translate, do not re-derive.** `clock.Milestone` → `Boundary`, keeping
  `TurnStarted`/`TurnEnded` only. `RoundStarted` translates to nothing and loses
  nothing: `Turn.End` increments before stamping the following `TurnStarted`
  (`turn.go:129-134`).
- [x] **Step 3: Announce at the moment of crossing.** In `driveOneMonsterTurn`,
  BEFORE the driven member acts — this is the whole reason the announcer is a
  capability and not a return value (design.md §2). In `EndTurn` for the caller's
  own end. In `form` for `RoundStarted(1)`/`TurnStarted(first)`, which closes the
  gap where the first turn of every fight begins with no turn-start at all.
- [x] **Step 4: `EndTurnOutput.Boundaries` — DROPPED, deliberately.** See ledger.
- [x] **Step 5: An announcer error aborts the caller's whole verb**, like
  `TurnDriver.Act` and `Striker.Strike`, and persists nothing — nothing is saved
  until the caller's own commit.
- [x] **Step 6: A test that a driven monster's turn-start is announced BEFORE its
  strike**, not after. This is the ordering the design exists for, and it is
  invisible in behaviour today because no monster trait subscribes.

**Ledger:**

- **`EndTurnOutput.Boundaries` was cut**, and cutting it is Kirk's own Q1 ruling
  applied to my own design. Nothing consumes it. An output field with no reader is
  the same hypothetical as a topic with no subscriber — *"a proposed vocabulary
  entry with no named subscriber is a hypothetical however reasonable it sounds"* —
  and I had written it into the design one section after arguing against exactly
  that. `RoundWrapped` already carries what the wire reports.
- **The first ordering test was wrong and a mutant proved it.** It survived
  deleting the announce from `driveOneMonsterTurn` outright, while its comment
  claimed to test that site. One monster after alice gets its turn-start from
  ALICE's `EndTurn`; the drive loop never enters into it. The real case is **two
  consecutive unplayed members**, and that is what
  `TestTheSECONDDrivenMemberAlsoHearsItsTurnStartFirst` now drives. Compare
  [[mutation-testing-catches-overclaims]].
- **All three announce sites were then mutated individually**:
  `driveOneMonsterTurn` kills 2 tests, `EndTurn` kills 2, `form` kills 1. No site
  is covered only by a test that would pass without it.
- A found bug on the way: `NewEncounter` validated `Announcer` and never stored
  it, so `e.announcer` was nil at `form`. Caught by the tomb suite panicking, not
  by the validation — validation only proves the caller supplied one.
- 55 files, of which 265 changes are the mechanical `Announcer:` insert at test
  construction sites. `build`/`vet`/`test`/`lint` clean, **0 issues**.
- **Copilot round still owed** on both #1258 and #1259 — GraphQL has been rate
  limited for this whole session and `requestReviews` needs it.

---

### Task 4: The boundary machine — `rulebooks/dnd5e/resolution` — ✅ **DONE 2026-08-27** · [toolkit#1260](https://github.com/KirkDiggler/rpg-toolkit/pull/1260) MERGED

**PR 3 of 5.**

**Files:**
- Add: `resolution/boundary.go` — `NewBoundary`, `BoundaryInput`, `BoundaryOutcome`.
- Add: `resolution/boundary_test.go`.
- Modify: `resolution/resolve.go` — pass `RefusingAnnouncer` at the encounter load
  (`:289`), for the same reason `RefusingStriker` is already passed there.
- Modify: `resolution/doc.go` — the bus-effect tally moves; say so.

**Interfaces:**
- Consumes: `encounter.Boundary` as data (R2).
- Produces: a `Machine` that publishes turn boundaries in causal order.

- [x] **Step 1: One step per crossing, in order, then `Done`.**
- [x] **Step 2: The step vocabulary does NOT grow.** A crossing is "do this on the
  bus and hand me the next step" — what `Gather` already does for the contest's
  imposition. Adding a `Step` case against one example is the mistake ADR-0007
  exists to remember.
- [x] **Step 3: Record the tally.** `doc.go` names its own reopening trigger — *"if
  wave 5's reactions multiply the pure-effect steps ... the argument reverses."* A
  boundary machine multiplies exactly those. It does not force a new case today; it
  moves the ratio, and `doc.go` asked for that to be noticed rather than
  discovered later.
- [x] **Step 4: `RefusingAnnouncer` at `resolve.go:289`**, with the comment saying why
  it is not recursion: this package never calls `EndTurn` or `form`, so no clock
  advances inside a resolution.
- [x] **Step 5: `Cost: nil`.** A boundary has no payer. Documented so nobody later
  reads the nil as an omission.

**Ledger:**

---

### Task 5: The seam — `rulebooks/dnd5e/session` — ✅ **DONE 2026-08-27** · [toolkit#1261](https://github.com/KirkDiggler/rpg-toolkit/pull/1261) MERGED

**PR 4 of 5.** Where it becomes true for a player.

**Files:**
- Add: `session/announcer.go` — `announcerSeam`, modelled line for line on
  `striker.go`.
- Modify: `session/write.go` (`adopt`, `openForWrite`), `session/read.go`,
  `session/start.go` — name an announcer at every load site.
- Add: `session/boundary_test.go`.

**Interfaces:**
- Consumes: `encounter.Announcer`, `resolution.NewBoundary`, the existing
  `castFor` (`attack.go:638`) and `saveDirty` (`attack.go:684`) — neither needs a change.

- [x] **Step 1: A struct holding `*writeScope`, not a closure over one** — so it can
  be constructed before `scope.enc` exists, exactly as `strikerSeam` documents
  (`striker.go:20-31`).
- [x] **Step 2: NEVER adopt a new `*encounter.Encounter`.** Read `out.DirtyCharacters`
  / `out.DirtyMonsters` and discard the returned world: swapping `enc` from inside
  a call `enc` itself made would orphan the loop (`striker.go:44`). Conditions live
  on sheets.
- [x] **Step 3: Name an announcer at all five load sites** — real one on the write
  path, refusing everywhere else (design.md §2 table).
- [x] **Step 4: The proving tests**, behavioural, through a real `session.EndTurn`,
  read out of the PERSISTED sheet — never by publishing the event by hand, which is
  the trap this slice exists to escape:
  - a rogue's `used_this_turn` is `false` after their turn ends;
  - a barbarian who neither attacked nor was hit **stops raging**;
  - a dodging fighter is no longer dodging after their next turn starts.
- [x] **Step 5: The structural test** — no code path advances a clock without
  announcing what it crossed, read off `clocks.go`'s AST, in the shape
  `TestNoCodePathProducesACastlessInteraction` established. A behavioural suite
  cannot make this claim: the defect IS that tests supply what production does not.

**Ledger:**

---

### Task 6: Put it in the running game — rpg-api — ✅ **DONE 2026-08-27** · [rpg-api#847](https://github.com/KirkDiggler/rpg-api/pull/847) OPEN

**PR 5 of 5.** Not a footnote — a task, for the reason the last slice learned the
hard way: **rpg-api pins the toolkit**, so every toolkit PR can merge, every tag
can cut, and the game still behaves the old way until this lands.

**Files:** `rpg-api/go.mod`, `go.sum`.

- [x] **Step 1: Record what is pinned now**, before touching anything.
- [x] **Step 2: Branch from `origin/dev`** and bump `dnd5e`, `encounter`,
  `resolution`, `session` to the tags Tasks 2–5 produced. `GOPROXY=direct` if a
  nested tag has not reached the proxy — it lagged on `resolution/v0.14.0` last time.
- [x] **Step 3: `go build ./... && go vet ./... && go test ./...`.** rpg-api is the
  first consumer to take all four at once.
- [x] **Step 4: Confirm in a real run.** Local stack, start a fight, Dodge, end the
  turn, and watch the condition **lapse**. This is the acceptance for the whole
  slice and the first moment any of it is true for a player.

**Ledger:**

---

### Task 7: Close the tracking surfaces

- [ ] **Step 1: Merge bottom-up**, confirming each auto-tag before bumping the next.
- [ ] **Step 2: Close the build issues and #294**, and set Done on Project 19.
      Check rather than assume — keywords fire on toolkit `main`, not on `dev`.
- [ ] **Step 3: Merge design PR #293** — Kirk's, once the slice is delivered.
- [ ] **Step 4: Record what moved**, and give the two debts the last slice left
      homeless an issue each: the stored `armor_class` backfill (rpg-api) and the
      thirteen missing standard conditions that `pack_tactics.go:160` points at.


---

## Verified live 2026-08-27

A real fight in the reference tomb, walked through the wire on a container built
from `deps/clock-boundaries`: `CreateLobby` → `StartEncounter` → `Move` until
contact formed a bubble → `EndTurn`.

```
BEFORE  conditions: ['dnd5e:conditions:unarmored_defense', 'dnd5e:conditions:dodging']
EndTurn { "roundWrapped": true, "saved": { "written": [ "character:char_d80b367c…", … ] } }
AFTER   conditions: ['dnd5e:conditions:unarmored_defense']
```

Three things in that output are the whole chain: the round wrapped, so the
barbarian's own turn STARTED; the character sheet was **written**, and nothing
damaged it or spent its economy, so the only thing that could have made it dirty
is a condition removing itself; and Unarmored Defense survived, as a permanent
condition should.
