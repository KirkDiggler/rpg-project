# Effect Context Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Give an effect one mandatory way to read the world and the cast, and one way to record that it changed itself — replacing five ambient registries that nothing installs, and reviving three conditions that are dead in live play.

**Architecture:** Two channels. Ambient context carries what is *true* about things nobody owns (the room, the cast); a direct owner handle carries reads and writes of the effect's *own* sheet. `Cast` exposes relational questions rather than fields, so allegiance can arrive later without touching a predicate. Resolution installs the cast on the same path it installs the room, unconditionally.

**Tech Stack:** Go 1.24/1.25, testify, per-module semver tags on `rpg-toolkit`.

**Spec:** `ideas/session-combat/effect-context/design.md` · **Territory:** `ideas/session-combat/effect-context/brainstorm.md`

## Global Constraints

- Toolkit-only. No protos, no rpg-api, no web. No wire change and no client change in this slice.
- Module chain is strict: `rulebooks/dnd5e` → `rulebooks/dnd5e/resolution` → `rulebooks/dnd5e/session`. Deliver in that order; each downstream module bumps to the tag the previous merge produced.
- **One build issue per module, one *in-flight* PR per module.** `dnd5e` carries two PRs (Tasks 2 and 3) under a single issue — sequential, never concurrent, because they touch the same package tree.
- Both `dnd5e` PRs merge before `resolution` bumps, so the chain runs **once**. Between those merges the rogue temporarily loses adjacency-driven Sneak Attack (`known=false` while nothing installs a cast) — an intermediate tag that is never consumed by a running game. Do not bump `resolution` mid-way to "test it".
- `resolution` currently pins `dnd5e v0.99.0` and `session` pins `dnd5e v0.99.0` / `resolution v0.12.0`, while `dnd5e` is already at `v0.100.0`. Bump to the tag this work produces, not to `v0.100.0`.
- Every `Cast` accessor returns `(value, known bool)`. Missing data is **never** an error and never poisons a chain fold. This is the discipline at `conditions/prone.go:284`; copy it exactly.
- No rule may compare faction identifiers itself. Rules ask `IsHostile`. A predicate that reaches for `Kind`, entity type, or a faction field is the defect this slice exists to remove.
- `Cast` speaks `combat.Combatant`, never `*character.Character`. Monsters and characters are the same shape at this seam.
- Import direction is fixed and verified: `gamectx` imports `combat`; `combat` does **not** import `gamectx`; `resolution` imports `gamectx`. Do not introduce a back-edge.
- Dirty is marked **by discipline** (`owner.MarkDirty()`) — ruled by Kirk 2026-08-26. The snapshot-diff belongs in a test harness, never on a production path.
- Branch from `origin/main` after `git fetch origin`. Never commit `replace`, `go.work`, or a local module path.
- Use normal GitHub CI and one Copilot review per PR. Do not create internal review loops for already-written code.
- Do not touch `gamectx.WithRoom`. It is installed, structurally pinned, and correct.

---

### Task 1: Open and board the tracking surfaces — ✅ **DONE 2026-08-26**

- [x] **Step 1: Slice issue** — rpg-project#287 *design: how an effect reads the world and writes itself back*
- [x] **Step 2: Build issues, one per module** — rpg-toolkit#1251 (dnd5e), #1252 (resolution), #1253 (session)
- [x] **Step 3: All four linked as sub-issues of journey rpg-project#253** (35 sub-issues total)
- [x] **Step 4: All four boarded on Project 19, verified by read-back**

| item | Status | Area | Kind | Team | Readiness | Initiative |
|---|---|---|---|---|---|---|
| rpg-project#287 | Todo | The Dungeon | Decide | Platform | Ready | Four-player Level-3 Dungeon |
| rpg-toolkit#1251 | Todo | The Dungeon | Build | Platform | Ready | Four-player Level-3 Dungeon |
| rpg-toolkit#1252 | Todo | The Dungeon | Build | Platform | **Blocked** | Four-player Level-3 Dungeon |
| rpg-toolkit#1253 | Todo | The Dungeon | Build | Platform | **Blocked** | Four-player Level-3 Dungeon |

#1252 and #1253 are Blocked on purpose: each waits on the tag the module before it produces. Move
to Ready as that tag lands.

**Board coordinates**, for whoever picks this up:

```
PROJECT_ID  PVT_kwHOAASbwc4Bcj4v
Status      PVTSSF_lAHOAASbwc4Bcj4vzhXLtvM   Todo=397864df  In Progress=a434eab1  In Review=9dac2cae  Done=e4d8ce42
Area        PVTSSF_lAHOAASbwc4Bcj4vzhXLt3s   The Dungeon=0e68c572  Game Screen=99f1a1b1
Kind        PVTSSF_lAHOAASbwc4Bcj4vzhXLt3w   Build=ea162471  Decide=2ccd98be  Fix=38b89d82  Verify=ab287333
Team        PVTSSF_lAHOAASbwc4Bcj4vzhYbEWs   Platform=f9c87bc7  UI/UX=de75cd8d
Initiative  PVTSSF_lAHOAASbwc4Bcj4vzhf2z3s   Four-player=20c80cbf
Readiness   PVTSSF_lAHOAASbwc4Bcj4vzhf2z6Y   Ready=51997600  Shaping=9f972e92  Blocked=2b2c7c9e
```

**Always verify a board write by reading it back, never by the mutation's own response.**

---

### Task 2: The two channels — ✅ **DONE** · rpg-toolkit#1251, PR A → [toolkit#1254](https://github.com/KirkDiggler/rpg-toolkit/pull/1254)

**Landed 2026-08-26.** 527 insertions, 2355 deletions. build + vet + test (27 packages) + golangci-lint all green;
Copilot review requested once and verified by node query. Board: In Review.

**What changed from the plan, and why:**
- `ReactionReadiness` was **kept**, not deleted — two live readers and a correct fail-closed default. See design.md.
- The test count landed at 10 modified + 1 deleted, and three files were left alone.
- `TestUnarmoredDefense_ACChainWithoutGameContext` turned out to **assert the bug** — it expected AC 13 and
  called the missing +2 an "API WIRING REQUIREMENT" nothing had ever met. It is inverted and renamed
  `TestUnarmoredDefense_ACChainNeedsNoGameContext`, asserting 15 on a completely bare context.
- Two Dueling tests built weapon sets nobody read; they now state `TwoHanded`/`OffHandWeaponRef` on the event
  so they exercise the rule instead of passing on an empty event.
- `gamectx/ability_scores.go` and `require.go` also went (only the deleted registries used them), and an
  orphaned `combatantRegistry` helper in `character/integration_test.go` went with the lookup it fed.

**Files:**
- Create: `rulebooks/dnd5e/gamectx/cast.go`
- Modify: `rulebooks/dnd5e/events/events.go:138` (owner contract)
- Modify: `rulebooks/dnd5e/character/character.go` (expose `MarkDirty`)
- Modify: `rulebooks/dnd5e/conditions/unarmored_defense.go:191`, `martial_arts.go:144,257`, `unarmored_movement.go:128`
- Delete: `rulebooks/dnd5e/gamectx/gamectx.go`, `characters.go`, `combatant.go`, `combat.go`, `require.go`, `ability_scores.go`
- **Keep:** `gamectx/room.go` and `gamectx/reaction_readiness.go` — readiness fails closed *by design* and has two live readers; see design.md
- Modify: `rulebooks/dnd5e/combat/combatant.go:113-135` (drop `CombatantLookup`)
- Modify: 10 test files; delete 1 (`gamectx/gamectx_test.go`). A 12th, `session/attack_test.go`, is in another module — Task 5.

**Interfaces:**
- Consumes: `combat.Combatant`, `shared.AbilityScores`.
- Produces: `gamectx.Cast`, `gamectx.WithCast`, `gamectx.CastOf`, `MarkDirty` on the owner contract.

- [x] **Step 1: Branch and establish the baseline**

```bash
cd /home/kirk/game-dev/rpg-toolkit
git fetch origin
git worktree add /home/kirk/.pi/worktrees/rpg-toolkit/effect-context-dnd5e \
  -b feat/effect-context-channels origin/main
cd /home/kirk/.pi/worktrees/rpg-toolkit/effect-context-dnd5e/rulebooks/dnd5e
go build ./... && go test ./... 2>&1 | tail -20
```

Expected: green. Do not proceed past a failing baseline.

- [x] **Step 2: Define `Cast` — questions, not fields**

```go
// Package gamectx — cast.go
//
// Cast is what an effect may ask about the OTHER participants in the
// interaction it is resolving. It is installed by resolution on every
// path, exactly like the room, and dies with that call.
type Cast interface {
	// Member returns a participant's combat-facing sheet.
	Member(id string) (combat.Combatant, bool)

	// Members returns every participant, in deterministic order (R4).
	Members() []string

	// IsHostile answers whether b is an enemy of a, right now.
	//
	// known is false when the question cannot be answered at all —
	// distinct from a confident "no". Collapsing the two invents a rule
	// out of missing data; see conditions/prone.go:284.
	IsHostile(a, b string) (hostile, known bool)
}

func WithCast(ctx context.Context, c Cast) context.Context
func CastOf(ctx context.Context) (Cast, bool)
```

No implementation here — `dnd5e` defines the question, `resolution` answers it.

- [x] **Step 3: Add `MarkDirty` to the owner contract and to `*Character`**

Follow the `FightingStyleProtectionCondition` precedent (`conditions/fighting_style_protection.go:36-70`): each condition declares its own narrow, structurally-satisfied owner interface and type-asserts it in `SetOwner`. An owner of the wrong shape is ignored, never an error.

`*Character` already satisfies the read side — `AbilityScores()` at `character/character.go:175`, `HasShieldEquipped()` at `:925`. Only `MarkDirty()` is new; it sets the same `c.dirty` flag as `character.go:723`.

- [x] **Step 4: Move the three dead conditions onto the owner handle**

Each currently does `registry, err := gamectx.RequireCharacters(ctx)` → `return c, err`. Replace with a read off the owner, and **return the chain unchanged when the owner is absent** — never an error.

This is the behavioural fix. `Character.EffectiveAC` swallows fold errors at `character/character.go:1398-1405`, so today an errored condition silently drops *every* AC contributor.

- [x] **Step 5: Delete the five dead installers**

`gamectx.GameContext`/`CharacterRegistry`/`WithGameContext`/`RequireCharacters`/`Characters`, `CombatantRegistry`/`WithCombatants`, `CombatState`/`WithCombatState`, and `combat.CombatantLookup`/`WithCombatantLookup`/`GetCombatantFromContext`.

**Do NOT delete `ReactionReadiness`.** It has two live readers (`conditions/opportunity_attack.go:167`, `conditions/shield_spell.go:186`) and its absent-value behaviour is correct on purpose — *"Not-ready is the safe default... preventing accidental spell-slot burns"* (`gamectx/reaction_readiness.go:37-41`). Fail-closed-by-design is not the same defect as fail-silent-by-accident.

```bash
grep -rn "RequireCharacters\|WithGameContext\|NewGameContext\|WithCombatants\|WithCombatState\|WithCombatantLookup\|GetCombatantFromContext" --include='*.go' . | grep -v _test.go
```

Expected: no output. Keep `WithRoom`, `Room`, `RequireRoom`.

- [x] **Step 6: Rewrite the 10 test files that installed a registry production never installs, and delete 1**

**Modify (10):**
```
conditions/fighting_style_dueling_test.go      integration/barbarian_encounter_test.go
conditions/fighting_style_protection_test.go   integration/fighter_encounter_test.go
conditions/martial_arts_test.go                integration/monk_encounter_test.go
conditions/unarmored_defense_test.go           integration/rogue_encounter_test.go
conditions/unarmored_movement_test.go          character/integration_test.go
```
**Delete (1):** `gamectx/gamectx_test.go` — its subjects are gone.

**Leave alone (3):** `conditions/opportunity_attack_test.go`, `conditions/shield_spell_test.go`, `gamectx/reaction_readiness_test.go` — these use only reaction readiness, which stays.

`character/integration_test.go` and `integration/barbarian_encounter_test.go` touch only `combat.WithCombatantLookup`, not gamectx.

**These are the tests that hid the bug.** Every one of them installs a registry by hand, which is why Unarmored Defense passed in CI and failed in the game. Replace the hand-installed registry with the owner handle. `gamectx/gamectx_test.go` and `reaction_readiness_test.go` go away with their subjects.

Note: `integration/` cannot import `resolution` (that is the dependency direction), so nothing in this module can drive a real `Resolve`. **Do not try to prove the end-to-end behaviour here — that is Task 5.**

- [x] **Step 7: Gates and PR**

```bash
go build ./... && go vet ./... && go test ./... && golangci-lint run
```

Open one PR against `origin/main`, ready for review (never draft), one Copilot round.

---

### Task 3: The two consumers — ✅ **DONE** · rpg-toolkit#1251, PR B → [toolkit#1255](https://github.com/KirkDiggler/rpg-toolkit/pull/1255)

**Landed 2026-08-26.** 515 insertions, 100 deletions; all gates green; one Copilot round requested and verified.

**What changed from the plan:**
- **`Cast` gained `IsAllied`.** Pack Tactics needs "one of the attacker's *allies*", and `!IsHostile` is not
  that once neutral exists. Brought by its consumer, per doc.go's own rule about not building ahead of readers.
- `checkSneakAttackConditions` also returned an **error** into the damage fold when it could not read the
  room — the same fold-poisoning shape #1254 fixed on the AC chain. Both predicates now return plain bools.
- `TestPackTacticsGrantsAdvantage` was **the second test in this slice found asserting nothing it promised** —
  it checked only that the attacker and target IDs survived the fold, under a comment conceding the real
  check was the game server's job. It now asserts advantage.

**Files:**
- Modify: `rulebooks/dnd5e/conditions/sneak_attack.go:236-266`
- Modify: `rulebooks/dnd5e/monstertraits/pack_tactics.go:109-140`

**Interfaces:**
- Consumes: `gamectx.Cast`, `gamectx.Room`.
- Produces: two predicates that ask relational questions instead of guessing from entity type.

- [x] **Step 1: Sneak Attack asks `IsHostile` from the target's perspective**

`sneak_attack.go:257` currently reads:

```go
// Check if any "character" type entity (ally) is near the target
```

RAW is *"another **enemy of the target** is within 5 feet of it."* Ask `cast.IsHostile(targetID, otherID)` — measured from the target, not the attacker. With two factions this gives the same answers as today; with three it stops being wrong.

`known=false` means no adjacency bonus. Never an error.

- [x] **Step 2: Implement Pack Tactics**

`pack_tactics.go:128` is a stub: *"TODO: In full implementation, check if ally is adjacent to target."* Positions come from `gamectx.Room` (already installed and read by four predicates); allyness comes from `cast.IsHostile(attackerID, otherID) == false`.

The rule's *"and not incapacitated"* clause **cannot be expressed** — Incapacitated is one of the 13 missing standard conditions. Implement the adjacency half and leave a named TODO pointing at the conditions catalogue; do not invent a substitute.

- [x] **Step 3: Unit tests against a fake cast, then gates and PR**

Both predicates are tested in-module with a hand-built `gamectx.Cast` fake — that is legitimate here because the fake stands in for a seam that *is* installed in production, unlike the registries being deleted. The production install is proven in Task 5.

Same gates as Task 2. This merges before `resolution` bumps.

---

### Task 4: Answer the question — rpg-toolkit#1252 (`rulebooks/dnd5e/resolution`)

**Files:**
- Create: `rulebooks/dnd5e/resolution/cast.go`
- Modify: `rulebooks/dnd5e/resolution/resolve.go:335` (install beside the room)
- Modify: `rulebooks/dnd5e/resolution/doc.go:~200` (the "No game context is installed" paragraph is now wrong)
- Modify: `rulebooks/dnd5e/resolution/go.mod`

**Interfaces:**
- Consumes: `resolution.Participants`, the tag Task 3 produced.
- Produces: the one and only `gamectx.Cast` implementation, installed unconditionally.

- [ ] **Step 1: Bump to the tag the dnd5e merges produced**

```bash
go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e@<tag> && go mod tidy
```

Expected: `resolution/go.mod` names the new tag. Do not use a pseudo-version.

- [ ] **Step 2: Implement `Cast` over `Participants`**

`Member` and `Members` are direct reads — resolution already holds every participant (R3, *"pass everyone in"*). `Members` returns sorted order, matching R4's determinism requirement so a suspension can be resumed into the same world it left.

`IsHostile` v1 is *"different `MemberKind`"*. **This is still a lie, and it is deliberately confined to one function** — the whole reason `Cast` exposes questions rather than fields. When allegiance lands, only this body changes. Say so in the doc comment, and name `brainstorm.md` as the reason.

- [ ] **Step 3: Install it beside the room, unconditionally**

```go
ctx = gamectx.WithRoom(ctx, room)
ctx = gamectx.WithCast(ctx, castOf(cast))
```

- [ ] **Step 4: Pin it structurally**

Mirror `TestNoCodePathProducesARoomlessInteraction` with `TestNoCodePathProducesACastlessInteraction`. Structural, not by example — an optional ambient dependency is exactly the defect this slice removes, and a test that only covers the happy path re-creates it.

- [ ] **Step 5: The dirty harness — in tests only**

Ruled by Kirk 2026-08-26: production marks dirty by discipline; the diff belongs in a test.

Build a helper that wraps `Resolve`, snapshots every participant's `ToData()` before and after, and fails if anything changed without coming back in `DirtyCharacters`/`DirtyMonsters`. It catches conditions nobody wrote a focused test for — including `sneak_attack.go:214`, which sets `UsedThisTurn` and today survives only because the attacker also paid an action.

Watch for false positives: `ToData()` must be byte-stable for unchanged state. Slices built from map iteration, or any value minted during serialization, will make every participant look dirty forever.

- [ ] **Step 6: Correct `doc.go`**

The paragraph beginning *"No game context is installed"* and the line *"The other four registries stay empty until a predicate that reads one arrives with its own consumer"* are both false after this task. Replace with what is now true: two installers, both mandatory, and the four speculative ones deleted. This paragraph is the reason the drift went unnoticed — leaving it stale would repeat that.

- [ ] **Step 7: Gates and PR.** Same as Task 2.

---

### Task 5: Prove it — rpg-toolkit#1253 (`rulebooks/dnd5e/session`)

**Files:**
- Modify: `rulebooks/dnd5e/session/go.mod`
- Modify: `rulebooks/dnd5e/session/attack_test.go`
- Create: an end-to-end test beside `session/attack.go:262` / `session/striker.go:114`

**Interfaces:**
- Consumes: the `resolution` and `dnd5e` tags from Tasks 3 and 4.
- Produces: the only proof that reaches a real fight.

- [ ] **Step 1: Bump both dependencies and fix the one stale test**

`session/attack_test.go` installs a deleted registry. `session` is where `resolution.Resolve` is actually called, from `attack.go:262` (player attacks) and `striker.go:114` (monster strikes).

- [ ] **Step 2: The test that would have caught this**

A barbarian with Unarmored Defense resolves a real attack **through `Manager.Attack`**, and the target AC used is 10+DEX+CON — not 10+DEX.

This is the whole point of the slice. Every existing test of Unarmored Defense passes today while the rule does not work in the game, because they install a registry production never installs. **A unit test on the condition cannot prove this; only a test that goes through the seam can.**

- [ ] **Step 3: A monk and a rogue too**

Martial Arts through a real unarmed strike. Sneak Attack firing on an enemy-of-the-target adjacency and not firing on an ally-of-the-target adjacency — under a Kind-based `IsHostile` these are still the two-faction answers, so the test pins **the question being asked** and keeps passing when allegiance lands.

- [ ] **Step 4: Gates and PR.** Same as Task 2.

---

### Task 6: Close the tracking surfaces

- [ ] **Step 1: Merge inside-out** — both `dnd5e` PRs, then `resolution`, then `session`. Kirk merges; confirm each auto-tag appeared before bumping the next module.
- [ ] **Step 2: Confirm the fix in a real run.** Start the local stack, play a barbarian, and read the AC on the dock. It should be 14, not 11. This is what rpg-api#842 was filed against; if it now reads 14, say so on that issue.
- [ ] **Step 3: Close rpg-project#287 and rpg-toolkit#1251/#1252/#1253 and set it Done on Project 19.** Closing keywords do not fire on `dev`-based repos but **do** fire on toolkit `main` merges — check rather than assume.
- [ ] **Step 4: Merge design PR #286** once the slice is delivered, per the convention that a design PR stays open through implementation.
- [ ] **Step 5: Record what moved.** `brainstorm.md`'s parked table is the next slice's agenda — the clock is unblocked the moment dirty lands, and it is the natural successor.
