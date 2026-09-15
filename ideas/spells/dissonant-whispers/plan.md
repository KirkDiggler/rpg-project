---
status: PLAN for ./design.md, written 2026-09-11
design: ./design.md · directive: ../../battlemap/directed-movement/design.md · previous spell: ../thunderwave/plan.md (the format and the walk manifest)
journeys: rpg-project#430 · rpg-project#243
---

# Dissonant Whispers — the plan, one module per PR, merged bottom-up

> **For agentic workers:** REQUIRED SUB-SKILL: superpowers:subagent-driven-development or superpowers:executing-plans, task by task. Every builder reads `rpg-toolkit/CLAUDE.md` and the module's own `CLAUDE.md`/`AGENTS.md` first. Draft PR on the first working push. Test first; run the test and SEE it fail before implementing; **never commit red** (the pre-commit hook runs the module's tests). Name the suite in `-run` and grep `=== RUN`. Never pin a collection's length. No session URLs anywhere. Read the two survey facts tables below before touching a file: every line number is from `origin/main` `9cc74df2` and may have drifted by a few lines.

**Goal:** the bard whispers at one creature within 60 feet; it makes a Wisdom save; on a success it takes half of 3d6 psychic, rounded down, with a trace that explains the halving; on a failure it takes all of it, spends its reaction, and runs as far from the bard as its speed allows by the ruler, provoking opportunity attacks as it goes, with a player reactor asked and the walk held until they answer.

**Architecture:** half as one more additive component in the damage trace; `Away` as a speed-limited flood on the existing field with the ruler as the goal; a monster reaction meter on the monster's keeper replacing the opportunity-attack condition's once-per-turn flag; a held directive beside the held turn with a sibling continue-verb; the two resolution refusal arms and the one session refusal deleted, not routed around.

**Modules and order.** `rulebooks/dnd5e` (root) → `rulebooks/dnd5e/encounter` → `rulebooks/dnd5e/resolution` → `rulebooks/dnd5e/session` → `rpg-api`. No proto change; no web change. Root and encounter are independent and may be built in parallel; resolution pins root; session pins all three. Each toolkit PR pins the one below it at a pushed pseudo-version while drafting and at the minted tag before leaving draft (`game-dev/scripts/bump-toolkit-pin.sh`, run from `/home/kirk/game-dev`). Check `git log origin/<branch>..origin/main -- <module>` before handing out a pseudo-version; a branch cut early can fall behind a same-day tag.

**Facts every task relies on** (verified 2026-09-11 at rpg-toolkit `9cc74df2`, rpg-api `dev` `fbe0bc8`, web `dev` `b2d9140c`):

| fact | where |
|---|---|
| `Half` accepted at the gate; refused in `CastProfile.Validate`, `ConditionApplication.Validate`, and `validateConditionGate` (called from `Start` before the condition split) | `saves/gate.go:206`; `combat/actions/cast.go:224`; `combat/actions/attack.go:259`; `resolution/contest.go:253,694` |
| success branch returns `Done` before delivery; failure chain damage → report → follow-ups → condition → removal → move, as tail-calling closures | `resolution/contest.go:857-859, 861-928` |
| `applyPreparedDamage` → `rollContestDamage` builds `DamageComponent`s → `damageCalculation` builds the trace FROM them; guard `FinalDamage total == calculation.Total` | `contest.go:405-465, 573-596, 604-654, 426-436` |
| trace: `RollComponent{Source, Dice, Modifier *int, SubtractDice}`; modifier-only component legal; source needs a valid `Ref` and a `Name`; sum is per-component additive | `events/roll_trace.go:104-118, 219-280` |
| `FinalDamage` sums per type, drops a group ≤ 0; `DamageComponent.Total()` adds a nil-dice modifier as-is | `combat/final_damage.go:74-131`; `events/events.go:256-265` |
| trace guards downstream: `Calculation.Total == Requested`; read-back refuses no calculation | `encounter/activation.go:434`; `session/events.go:886-896` |
| `MovePolicy{MoveLine}`, `MovePays{PaysNothing, PaysReaction}`, `CastMove{Policy, Cells, Speed, Pays, Provokes}` + `Validate` (exactly one budget) | `combat/actions/move.go:21-108` |
| `MoveDirective{Policy, AnchorID, Cells, Speed, Pays, Provokes}`; `validateMove`'s two refusal arms; `imposeMove` with `pushed`/`stayed`; `directiveFor` | `resolution/contest.go:126-152, 486-508, 539-564`; `resolution/action.go:650-663` |
| refusal tests to delete: `TestADirectiveNothingCanExecuteIsRefused` (two subtests) | `resolution/contest_move_test.go:219-238` |
| encounter `MovePolicy{MoveLine}`, `RouteInput{Mover, Policy, Anchor, Budget}`, `RouteOutput{Path, StoppedBy}`, `Route`, `routeLine`, `stoppedBy` (the PassThrough "is occupied by" case) | `encounter/directive.go:48-225` |
| `DirectInput{Mover, Cause, Route, Provokes}`, `DirectOutput{Moved, StoppedBy, IntelDeltas}`, `Direct` (refuses paused member; refuses a pause with `ErrStepPaused`) | `directive.go:228-370` |
| `walkPath(ctx, mover, m, path, audience, at, cause, forced) walkResult{moved, dropped, paused, from, to, pending}`; `pending = path[i:]`, announced cell first | `encounter/clocks.go:883-1033` |
| `floodFrom` (no Limit), `nearestStop` (Standable only, dist then scan order), `beforeInScanOrder`; `Encounter.Distance` = the grid ruler | `clocks.go:1326-1396`; `encounter.go:1069` |
| `spatial.FieldInput.Limit` bounds `Dist ≤ Limit` inclusive; `PathTo` excludes the source | `tools/spatial/field.go:19-24, 87-105, 123` |
| `pausedTurn{member, round, from, to, remaining, moved, budget, intent, bound, at, audience}`; `PausedTurnData`; converters; `validatePausedTurn`; `Paused()`, `PausedMember()`; `appendWindowOpenedBeat(*pausedTurn, windows)`; `ResumeTurn` (no input); `finishPausedIntent` (standing first; announced cell by hand; rest via `walkCells`; re-hold with accumulated `moved`) | `encounter/pause.go:98-224, 249-296, 306-340, 396-506, 557-630` |
| data wiring pattern: field `paused_turn,omitempty`, `ToData`, `validate` before construction, restore after `buildWorld` | `encounter/data.go:118, 1333, 2071-2078, 2308` |
| `Paused()` as whole-encounter freeze in four places; per-member gates in two | `clocks.go:303, 329, 1413, 1872`; `step.go:109`; `directive.go:333` |
| `StepPausedError{Windows []PausedWindow}` | `pause.go:35-78` |
| session `castPush`, `routeCastPushes` (Speed refusal at `:98-105`; `Budget: push.move.Cells` at `:116`; writes `Moved`/`StoppedBy` from the route), `walkCastPushes` (discards `DirectOutput`), `routePolicy` | `session/castmove.go:51-182` |
| `rosterPositions`; `encounter.Member.SpeedFeet`; `CellsFromFeet` | `session/reach.go:15-21`; `encounter/field.go:1349`; `encounter/units.go:33` |
| cast verb order: `castOutcome` :373 → `routeCastPushes` :384 → adopt/saveDirty → `RecordCast` :402 → `walkCastPushes` :425 (error → `reportUnrecorded`) → `commit` :429 | `session/cast.go:373-429` |
| `castOutcome` collects pushes; `imposedResult`'s move arm sets no `Moved`/`StoppedBy`; `ResultMoved{Moved: 0, StoppedBy: x}` validates | `session/castoutcome.go:36-76, 199-218`; `encounter/activation.go:506-523` |
| `React`: `openForWrite`; `strikeForWindow` replays via `offerStep` with a zero `MoveStep` and `only`; the gate `if len(open) == 0 && scope.enc.Paused() { ResumeTurn }`; windows written after the resume | `session/react.go:118-210, 288-307` |
| `moverSeam.Move` asks players only when the mover is a monster; `pose` writes the ledger and returns `StepPausedError`; `forcedBy` reads `Cause` only when `Forced` | `session/mover.go:69-103, 277-290, 305-348` |
| `Monster.CanReact()` hard-coded true; `Monster` has no mutable blob; `Data` fields; `loadMonster`; `ToData`; keeper `Apply` has five rows; a test proves the keeper ignores a spend | `monster/monster.go:198-208, 24-63, 634-649`; `monster/data.go:16-52`; `monster/load.go:90-106, 178-248`; `monster/load_test.go:553` |
| OA condition: `UsedThisTurn` sites (:47, :223-226, :234-239, :267, :279, :317, :401, :410); `Apply` subscriptions in order MovementChain, TurnStart, Rest, ReactionTaken; `canReact` via `gamectx`; `stateChanged` | `conditions/opportunity_attack.go:39-93, 104-142, 160-207, 222-240, 263-281, 306-421` |
| OA meter tests (fates in the survey) and the five outside packages that read `used_this_turn` | `conditions/opportunity_attack_meter_test.go`; `character/long_rest_conditions_integration_test.go:254-358`; `conditions/long_rest_registry_test.go:373-384`; `resolution/long_rest_test.go:130-135,301`; `session/join_long_rest_test.go:548-567`; `monstertraits/monster_carries_a_condition_test.go:79-211` |
| `carryingFreeReactions` seats the OA condition on a monster; its doc states the asymmetry | `monstertraits/loader.go:404-488` |
| `SpendRequestedEvent{MemberID, ActionType, Amount, SourceRef}`; sole subscriber the character keeper; `publishSpendRequested`; `TurnStartEvent{SubjectID, Round}` published by the boundary for every crossing | `events/events.go:748-781, 609-612`; `character/sheet_keeper.go:171`; `conditions/requests.go:59-67`; `resolution/boundary.go:105-110` |
| `Character.SpendSlots` has no floor; `seedTurn` reseeds `ReactionsRemaining: 1` from `StartTurn` | `character/ledger.go:97-116`; `character/action_economy.go:89-104` |
| a `Gather` step's `run` receives the interaction bus; `movementMachine.bill` publishes on it; `combatantFor(cast, id)` gives a `combat.Combatant` with `CanReact()` | `resolution/movement.go:409-430`; `contest.go:539-564, 971` |
| Vicious Mockery row (the single-target WIS template); Thunderwave row and its `Negated` comment; `cast_test.go:292` "half on a save does not exist yet" | `spells/cast.go:372-403, 244-297`; `spells/cast_test.go:270-304` |
| spell constants; refs (`spellThunderwave`, `Thunderwave()`, `spellByID`); `SpellData` NOT required to cast | `spells/types.go:64-65`; `refs/spells.go:56, 245, 546-550`; `spells/cast.go:426-447` |
| bard pick `Count: 2`, `{Bane, Thunderwave}`; the count tracks the catalogue | `character/choices/requirements.go:429-450` |
| directive test fixture: `lineScene(withPillar)`, row-0 corridor, `goblin` at (1,0) with `SpeedFeet: 30`, `cellAt`, `pointyCanvas`, `wall(...)`, `recordingMover`, `cellOfMember`, `movedBeats`; the refusal loop `{"", "away", "toward", "sideways"}` | `encounter/directive_test.go:42-147, 317-335`; `regionfixtures_test.go` |
| sandbox seed: `thunderwaveRef` const :46; `SpellRefs: []string{baneRef, thunderwaveRef}` :198; `FinalizeDraft` asserts | `rpg-api internal/sandboxseed/sandboxseed.go:34-46, 190-264` |
| web: reaction window UI exists (`reactionWindow.ts:39-64`, `ActionDock.tsx:386-423`); MEMBER cast arms on click | `useSessionCombatExperience.ts:667-716` |

**Out of scope, on purpose:** the movement beat's cause on the wire and the route-vs-walk distance (protos#329 family, viz lane); two provoking pushes from one cast; a `SpellData` row (add one for the catalogue if the builder wants it; it gates nothing); toolkit#1662 (duplicate picks).

---

## PR 1 — `rulebooks/dnd5e` (root): half's vocabulary, `MoveAway`, the meter, the content

Branch `feat/dissonant-whispers-root`; title `feat(dnd5e): Dissonant Whispers — half narrowed to damage-only, MoveAway, a monster reaction meter, the profile`. Three commits, in this order, so each is green alone.

### Task 1.1: `Half` is permitted for damage-only gates; `MoveAway` exists

**Files:** modify `combat/actions/cast.go`, `combat/actions/cast_test.go`, `combat/actions/move.go`, `combat/actions/move_test.go`; leave `combat/actions/attack.go:259` as it is (an attack rider is a condition, and a halved condition stays meaningless).

- [ ] **Step 1: failing tests** (suite in `cast_test.go`):

```go
func (s *CastProfileSuite) TestHalfIsPermittedForADamageOnlyGate() {
	p := s.gatedDamageProfile() // WIS gate, one damage pool, no Effects, no Move
	p.Save.OnSuccess = saves.Half
	s.NoError(p.Validate())
}

func (s *CastProfileSuite) TestHalfIsRefusedWhenAConditionIsDelivered() {
	p := s.gatedConditionProfile() // the Vicious Mockery shape
	p.Save.OnSuccess = saves.Half
	err := p.Validate()
	s.Error(err)
	s.Contains(err.Error(), "half on a save is only for damage")
}
```

and in `move_test.go`: `CastMove{Policy: MoveAway, Speed: true, Pays: PaysReaction, Provokes: true}.Validate()` is nil; `Policy: "toward"` still refused.

- [ ] **Step 2: run, see them fail** (`Half` refused; `MoveAway` undefined).
- [ ] **Step 3: implement.** In `cast.go:224-226`: replace the refusal with

```go
if p.Save.OnSuccess == saves.Half && len(p.Effects) > 0 {
	return fmt.Errorf("half on a save is only for damage: this cast delivers a condition, and half a condition means nothing")
}
```

(`Half` with damage and no effects passes; `Negated` unchanged; `p.Save.Validate()` still rejects unknown words.) Rewrite the `Save` field doc at `cast.go:92-95`: half arrived with Dissonant Whispers; it is permitted only for damage. In `move.go`: `const MoveAway MovePolicy = "away"` with a doc in the file's voice (away = the reached standable cell farthest from the anchor by the ruler, within the budget; the executor is encounter's `Route`), and the `Validate` switch gains `case MoveLine, MoveAway:`. Update the `MovePolicy` type doc (`:16-20`): two values now; "toward" still waits for Thorn Whip.

- [ ] **Step 4: run, PASS; commit** `feat(actions): Half is permitted for a damage-only cast gate; MoveAway joins the policies`

### Task 1.2: the monster reaction meter replaces the once-per-turn flag

**Files:** modify `monster/monster.go`, `monster/data.go`, `monster/load.go`, `monster/load_test.go`, `monster/attach_rollback_test.go`; `conditions/opportunity_attack.go`, `conditions/opportunity_attack_meter_test.go`, `conditions/opportunity_attack_test.go`, `conditions/long_rest_registry_test.go`; `monstertraits/loader.go`, `monstertraits/monster_carries_a_condition_test.go`; `character/ledger.go`, `character/ledger_test.go` (or the nearest), `character/long_rest_conditions_integration_test.go`; `events/events.go` (docs only); `combat/combatant.go` (doc only).

- [ ] **Step 1: failing tests.** In `monster/load_test.go`, invert the test at `:553`: after `AttachMonster`, publishing `SpendRequestedEvent{MemberID: m.GetID(), ActionType: ActionReaction, Amount: 1}` makes `m.CanReact()` false and `m.IsDirty()` true; publishing `TurnStartEvent{SubjectID: m.GetID()}` makes it true again; another subject's turn start leaves it false; `RestEvent{CharacterID: m.GetID(), RestType: ResetLongRest}` clears it; `ToData().ReactionSpent` round-trips through `LoadFromData`. In `conditions/opportunity_attack_meter_test.go`, rewrite per the survey's fate table: `TestAMonsterReactsWithNoPurseAndIsStillMetered` becomes `TestAMonsterIsMeteredByItsKeeper` (the fake owner's `CanReact` flips false after the spend row; the second mover gets no trigger); delete `TestTheMeterSurvivesTheJSONRoundTrip` and `TestOpportunityAttackMeterResetsOnLongRest`; the two rollback-count tests count the two remaining subscriptions (MovementChain, ReactionTaken); every `s.True(oa.UsedThisTurn)`/`s.False(...)` assertion becomes an assertion on the fake keeper's meter. In `character/ledger_test.go` (or wherever `SpendSlots` is tested): spending a reaction twice leaves `ReactionsRemaining` at 0, not −1, and `TestATakenReactionSpendsExactlyOnce` keeps its meaning through that floor. In the four outside packages, the `used_this_turn` readers: character-side tests are deleted with a one-line commit note (a character's reaction was always the slot); `monstertraits/monster_carries_a_condition_test.go:188-211` asserts the OA condition is still carried on the sheet and that `ReactionSpent` is on the monster blob, not the condition's; `resolution/long_rest_test.go:130-135,301` and `session/join_long_rest_test.go:548-567` and `conditions/long_rest_registry_test.go:373-384` drop the OA-flag case (the registry test's `longRestReset` row for OA goes; if the registry requires every condition to declare a rest behaviour, OA declares none).
- [ ] **Step 2: run, see the monster tests fail** (`CanReact` true after a spend; no `ReactionSpent`).
- [ ] **Step 3: implement.**
  - `monster/data.go`: `ReactionSpent bool \`json:"reaction_spent,omitempty"\`` with a doc: the one reaction meter a monster has; true from a spend until its next turn start or a long rest; absent on every earlier blob and read as false. `loadMonster` reads it; `ToData` writes it; check `TestKnownRoundTripGaps` still lists only `Features`/`Inventory`.
  - **Ruled by Kirk 2026-09-11:** the monster keeper SUBSCRIBES to turn start to clear the meter ("monster doesn't have the action economy really … subscribing to the turn start to replace his reaction is fine"). A character's slot is reseeded by the verb session calls; a monster has no economy and no such verb. When a monster grows an economy (the monster-with-instructions foundation), the meter becomes one field of it and moves to that verb.
  - `monster/monster.go`: field `reactionSpent bool`; `CanReact() bool { return !m.reactionSpent }` with the old eight-line doc replaced by one that says the ruling reversed on 2026-09-11 and why (Whispers spends it from outside its own condition). Handlers `onSpendRequested` (MemberID match, `ActionReaction` only, sets true, `m.dirty = true`), `onTurnStart` (SubjectID match, clears, dirty only if it changed), `onRest` (CharacterID match and long rest, same). `SheetKeeper.Apply` gains three rows in `track`; the rollback test counts eight.
  - `conditions/opportunity_attack.go`: delete `UsedThisTurn` from both structs and the JSON; delete `onTurnStart`, `onRest`, and their two subscriptions; `onMovementChain` loses the `:317` gate (the `canReact` gate at `:324` is the meter); `onReactionTaken` loses the flag write and the dedup line, keeps the spend publish, drops `stateChanged` (nothing on the condition changes any more). Rewrite the struct doc (`:71-74`, `:86-89`) and the `canReact` doc (`:108-134`): one meter per creature, the keeper's. `ToJSON`/`loadJSON` shrink to `{ref, member_id}`.
  - `character/ledger.go` `SpendSlots`: the reaction arm floors at zero (`if c.actionEconomy.ReactionsRemaining > 0 { -= n }` clamped), with a comment: the meter is the guard, since the flag that used to dedup a double bill is gone.
  - `monstertraits/loader.go:444-455`: delete the "a monster has no action economy" paragraph; `carryingFreeReactions` still seats the condition.
  - Docs: `events/events.go:758-769` (the keeper-may-have-no-row paragraph), `conditions/requests.go:56-58`, `combat/combatant.go:111-129`.
- [ ] **Step 4: run the root module's tests; commit** `feat(monster): a reaction meter on the monster's keeper replaces the opportunity attack's once-per-turn flag (Kirk 2026-09-11)`

### Task 1.3: the profile, the constants, the ref, the pick, Thunderwave's flip

**Files:** modify `spells/types.go`, `refs/spells.go`, `spells/cast.go`, `spells/cast_test.go`, `character/choices/requirements.go` and its test; optionally `spells/data.go`.

- [ ] **Step 1: failing tests** (`spells/cast_test.go`): `CastDefinition({Spell: DissonantWhispers, SpellSaveDC: 13})` returns a definition whose profile has `RangeFeet 60`, `CastTargetOneCreature` 1/1, WIS gate `OnSuccess == saves.Half`, one `3d6` psychic pool, no effects, `Move == &CastMove{Policy: MoveAway, Speed: true, Pays: PaysReaction, Provokes: true}`, cost one level-1 slot (extend `TestBaneAndThunderwaveSpendTheSameSlot` to loop three). Thunderwave's test at `:292` flips to `saves.Half` with the message "half exists now; the row promised to flip". The bard pick test asserts membership of all three and `Count == 3`.
- [ ] **Step 2: run, see them fail.**
- [ ] **Step 3: implement.** `spells/types.go`: `DissonantWhispers Spell = "dissonant-whispers"` in the level-1 block. `refs/spells.go`: `spellDissonantWhispers`, `DissonantWhispers()`, and the `spellByID` entry. `spells/cast.go`: constants `DissonantWhispersRangeFeet = 60`, `DissonantWhispersDamage = "3d6"`; the row from design §7 with comments in the file's voice (the price and the provoke are the two fields Thunderwave zeroed; the budget is the mover's legs). Thunderwave's `OnSuccess` becomes `saves.Half` and its comment becomes one line: half arrived with Whispers. `requirements.go:444-450`: `Count: 3`, `Options: {Bane, Thunderwave, DissonantWhispers}`, label "Choose 3 supported 1st-level spells"; the comment's "two today" becomes "three today".
- [ ] **Step 4: run, PASS; commit** `feat(spells): Dissonant Whispers — the profile; Thunderwave saves for half; the bard's 3 of 3`
- [ ] **Step 5: push; draft PR.** Expected tag `rulebooks/dnd5e/v0.156.0` (feature bump; the title's `feat` prefix drives it).

---

## PR 2 — `encounter`: `Away`, the held directive, `ResumeDirective`

Branch `feat/away-and-held-directive`; title `feat(encounter): MoveAway routes by the ruler on a speed-limited flood; a provoking directive that pauses is held and resumed`. Independent of PR 1 (encounter's policy set is its own).

### Task 2.1: `Route` with `MoveAway`

**Files:** modify `encounter/directive.go`, `encounter/directive_test.go`, `encounter/clocks.go` (`floodFrom` gains a limit parameter).

- [ ] **Step 1: failing tests** in `DirectiveTestSuite`. Build a second scene `awayScene()`: an open 7×5 room via `rectRegion`, caster at `cellAt(3,2)`, goblin at `cellAt(4,2)`, a wall segment via `wall(...)` making a two-cell dead-end pocket at the goblin's side that bends back toward the caster. Tests:

```go
func (s *DirectiveTestSuite) TestAwayEndsAtTheReachedCellFarthestByTheRuler() {
	enc := s.awayScene()
	out, err := enc.Route(encounter.RouteInput{Mover: goblin, Policy: encounter.MoveAway, Anchor: s.casterCell, Budget: 6})
	s.Require().NoError(err)
	s.NotEmpty(out.Path)
	end := out.Path[len(out.Path)-1]
	start := s.cellOfMember(goblin)
	s.Greater(enc.Distance(s.casterCell, end), enc.Distance(s.casterCell, start))
	for _, cell := range reachableStandableWithin(enc, goblin, start, 6) { // test helper over enc.Route with Budget 6 is circular; use a flood via the exported Field on the grid + enc.CellAt
		s.LessOrEqual(enc.Distance(s.casterCell, cell), enc.Distance(s.casterCell, end), "no reached standable cell is farther than the chosen end")
	}
	s.LessOrEqual(len(out.Path), 6)
	for i := 1; i < len(out.Path); i++ { s.True(enc.GetGrid().IsAdjacent(out.Path[i-1], out.Path[i])) }
}

func (s *DirectiveTestSuite) TestAwayDoesNotTakeTheDeadEndThatBendsBack() {
	// the pocket's far cell is 5 by walking and 1 by the ruler; the open floor wins
}

func (s *DirectiveTestSuite) TestAwayPinnedIsAnEmptyRouteThatSaysSo() {
	// caster at the mouth of a one-cell pocket the goblin stands in
	s.Empty(out.Path); s.Contains(out.StoppedBy, "nowhere farther")
}

func (s *DirectiveTestSuite) TestAwayTiesAreStable() { /* two equidistant ends: the shorter walk wins; equal walks: scan order; run twice, same answer */ }
```

Delete `"away"` from the refusal loop at `:330-335`.

- [ ] **Step 2: run, see `MoveAway` undefined.**
- [ ] **Step 3: implement.** `const MoveAway MovePolicy = "away"` beside `MoveLine`, doc: "toward" still refused. `Route`: the policy check becomes a switch; `case MoveAway: return e.routeAway(in.Mover, from, in.Anchor, in.Budget), nil`. `floodFrom(mover, from, forgiven, limit int)`: pass `Limit: limit` (existing callers pass 0). `routeAway`:

```go
func (e *Encounter) routeAway(mover MemberID, from, anchor spatial.Position, budget int) RouteOutput {
	if budget == 0 { return RouteOutput{} }
	field, ok := e.floodFrom(mover, from, nil, budget)
	if !ok { return RouteOutput{StoppedBy: "the field could not be flooded"} }
	here := e.Distance(anchor, from)
	var best spatial.Position; bestFar, bestDist, found := 0.0, 0, false
	for cell, dist := range field.Dist {
		if cell == from { continue }
		if e.CellAt(CellAtInput{Cell: cell, Mover: mover}).Passage != PassageStandable { continue }
		far := e.Distance(anchor, cell)
		if far <= here { continue }
		if !found || far > bestFar || (far == bestFar && (dist < bestDist || (dist == bestDist && beforeInScanOrder(cell, best)))) {
			best, bestFar, bestDist, found = cell, far, dist, true
		}
	}
	if !found {
		return RouteOutput{StoppedBy: fmt.Sprintf("nowhere farther from %v within %d cells", anchor, budget)}
	}
	path, _ := field.PathTo(best)
	return RouteOutput{Path: path}
}
```

Doc it: away is measured by the ruler, not the walk (directed-movement §3, rejected alternative); may cross an ally, may not stop on one (the same law as `nearestStop`); an empty path with `StoppedBy` is the pinned case and is distinguishable from a bad policy by construction. Update `ErrUnsupportedPolicy`'s doc (`errors.go:580-591`): two policies now.

- [ ] **Step 4: run, PASS; commit** `feat(encounter): MoveAway — the reached standable cell farthest from the anchor by the ruler, within the budget`

### Task 2.2: the held directive and `ResumeDirective`

**Files:** create `encounter/held.go`, `encounter/held_test.go`; modify `encounter/directive.go`, `encounter/pause.go` (generalise the window beat), `encounter/data.go`, `encounter/encounter.go` (the live field), `encounter/clocks.go` and `encounter/pause.go` (`Paused()`/`PausedMember()` second source).

- [ ] **Step 1: failing tests** (`held_test.go`, reuse `lineScene` with a `pausingMover` test double that returns `&StepPausedError{Windows: [...]}` on the first announce and records thereafter):

```go
func (s *HeldTestSuite) TestAProvokingDirectiveThatPausesIsHeldNotRefused() {
	out, err := enc.Direct(ctx, DirectInput{Mover: goblin, Cause: whispersRef, Route: route.Path, Provokes: true})
	s.Require().NoError(err)
	s.True(out.Paused); s.Equal(0, out.Moved)
	s.Equal(goblin, enc.PausedMember()); s.True(enc.Paused())
	// the window beat is on the story with the cause
	// the goblin has not moved
	// ToData().HeldDirective round-trips through FromData and PausedMember() survives it
}

func (s *HeldTestSuite) TestResumeDirectiveStepsTheAnnouncedCellWithoutASecondAnnounceAndWalksTheRest() {
	// after the hold: ResumeDirective → Moved == len(route.Path); the mover double saw ONE announce for cell 0 and one per later cell; every moved beat carries the cause; PausedMember() == ""
}

func (s *HeldTestSuite) TestResumeDirectiveOnADroppedMoverClearsTheHold() { /* standing says down → Moved 0, hold cleared, no step */ }
func (s *HeldTestSuite) TestASecondPauseAccumulatesMoved() { /* double pauses on cell 0 and again on cell 2: second hold has Moved 2; final resume reports the whole route */ }
func (s *HeldTestSuite) TestEndTurnIsRefusedWhileADirectiveIsHeld() { s.ErrorIs(err, ErrTurnPaused) }
func (s *HeldTestSuite) TestResumeDirectiveWithNothingHeldIsNotPaused() { s.ErrorIs(err, ErrNotPaused) }
func (s *HeldTestSuite) TestAHeldDirectiveOnAClosedEncounterIsRefusedAtLoad() { /* validateHeldDirective mirrors validatePausedTurn's refusals */ }
```

- [ ] **Step 2: run, see `Paused` field / `ResumeDirective` undefined.**
- [ ] **Step 3: implement**, copying `pause.go`'s pattern field for field:

```go
// held.go
type heldDirective struct {
	member    MemberID
	from, to  spatial.Position
	remaining []spatial.Position // announced cell first; never empty
	moved     int
	at        uint64
	audience  []MemberID
	cause     core.Ref
	forced    bool
}
type HeldDirectiveData struct { Member MemberID; From, To PositionData; Remaining []PositionData; Moved int `json:",omitempty"`; At uint64 `json:",omitempty"`; Audience []MemberID `json:",omitempty"`; Cause string; Forced bool `json:",omitempty"` } // json tags in the blob's snake_case
func heldDirectiveDataFrom(*heldDirective) *HeldDirectiveData
func heldDirectiveFrom(*HeldDirectiveData) (*heldDirective, error) // cause parsed; invalid ref refused
func validateHeldDirective(d *HeldDirectiveData, isMember func(MemberID) bool) error // the same refusals as validatePausedTurn minus intent/bound/budget; cause must be a valid ref
func (e *Encounter) ResumeDirective(ctx context.Context) (DirectOutput, error)
```

`Encounter` gains `heldDirective *heldDirective` beside `pausedTurn`; `EncounterData.HeldDirective *HeldDirectiveData \`json:"held_directive,omitempty"\``; `ToData`/`FromData`/validation wired at the four sites `PausedTurn` uses (`data.go:118, 1333, 2071-2078, 2308`), including the refusal of a hold and a paused turn both present, and a hold on a closed encounter. `Paused()` returns `e.pausedTurn != nil || e.heldDirective != nil`; `PausedMember()` returns whichever is set. `Direct`: on `res.paused != nil`, build the hold from `res` (`moved: res.moved`, `cause: in.Cause`, `forced: !in.Provokes`), append the window beat (generalise `appendWindowOpenedBeat` to take `(member, from, to, windows, cause core.Ref)`; the paused-turn caller passes a zero cause; a valid cause is added to the payload as `"cause"`, additive), settle sight for the cells walked, return `DirectOutput{Paused: true, Moved: res.moved, IntelDeltas: deltas}`. `DirectOutput` gains `Paused bool` with a doc: true means the walk is held on a window and `ResumeDirective` finishes it. `ResumeDirective`: `ErrClosed`; `ErrNotPaused` when no hold; `ErrNotMember` when gone; **clear the hold before the first step** (the same reason as `pause.go:426-430`); `standingNow()` first, down ⇒ return `{Moved: h.moved}` with the hold cleared; step `h.to` by hand (`stepTo` + `action.cause = h.cause` + `appendMovementBeat`), refused ⇒ done; then `walkPath(ctx, h.member, m, h.remaining[1:], h.audience, h.at, h.cause, h.forced)`; a second pause re-holds with `moved: h.moved + 1 + res.moved` and appends the window beat again; `settleWalk`; return `DirectOutput{Moved: h.moved + 1 + res.moved, StoppedBy: <as Direct computes it>, Paused: <re-held>}`. Update `Direct`'s doc (`directive.go:302-311`): the day has come.

- [ ] **Step 4: run the module; commit** `feat(encounter): a directive that pauses for a reactor is held, and ResumeDirective finishes the walk with its cause`
- [ ] **Step 5: push; draft PR.** Expected tag `encounter/v0.74.0`.

---

## PR 3 — `resolution`: half delivered, the price paid, the refusals deleted

Branch `feat/half-and-the-price`; title `feat(resolution): a made save delivers half through the same report; a priced move spends the reaction first; Speed and PaysReaction are executable`. Pin root at PR 1's pseudo-version.

### Task 3.1: `validateGate`, and the success branch delivers half

**Files:** modify `resolution/contest.go`, `resolution/contest_test.go` (or the cast-action tests where a gated damage cast is driven end to end), `resolution/action.go` if the success path needs the profile's gate.

- [ ] **Step 1: failing tests**, through `Resolve` with the real driver (mirror the existing gated-damage cast test):

```go
func (s *CastActionSuite) TestAMadeSaveAgainstAHalfGateDealsHalfRoundedDown() {
	// roller scripted: save succeeds; 3d6 rolls 4,5,4 = 13
	// outcome: one ImposedDamage on the saver, Amount 6, Requested 6;
	// Calculation.Components: the 3d6 component, then a modifier-only component with Modifier -7,
	//   Source.Ref == the spell, Source.Name == "Dissonant Whispers", Source.Label == "halved by a successful save";
	// Calculation.Total == 6; ValidateRollCalculation passes; no ImposedMove; no condition
}
func (s *CastActionSuite) TestHalfDamageStillOwesAConcentrationCheck() {
	// saver is concentrating; save succeeds; the outcome's FollowUps contains one CON check at max(10, half/2)
}
func (s *CastActionSuite) TestAMadeSaveAgainstANegatedGateStillDeliversNothing() { /* Sacred Flame regression */ }
func (s *CastActionSuite) TestAHalfGateWithAConditionIsRefusedAtTheContest() { /* ErrBadGate, message names "damage only" */ }
```

- [ ] **Step 2: run, see them fail** (success returns `Done` with nothing imposed).
- [ ] **Step 3: implement.** Rename `validateConditionGate` → `validateGate`; its `Negated` check becomes: `Half` permitted only when `m.in.Damage` is non-empty and no condition/removal is declared (pass what it needs, or check in `Start` after `hasCondition` is known and keep the name honest). In `resolve` (`contest.go:857-859`):

```go
if outcome.Succeeded {
	if m.in.Gate.OnSuccess != saves.Half {
		return Done{Outcome: outcome}, nil
	}
	return applyPreparedDamage(m.in.Damage, m.in.Roller, m.in.Cause, m.in.SourceName, m.cast, m.in.SaverID, halved, func(applied ImposedEffect) (Step, error) {
		outcome.Imposed = append(outcome.Imposed, applied)
		return reportDamage(/* the same input as the failure branch */, func(ctx context.Context, ups []dnd5eEvents.FollowUp) (Step, error) {
			return runFollowUps(ctx, ups, 0, m.in.Roller, collectFollowUp, func(context.Context) (Step, error) { return Done{Outcome: outcome}, nil })
		}), nil
	}), nil
}
```

`applyPreparedDamage` gains a `halved bool` parameter (the failure branch passes false). Inside, after `rollContestDamage` and before `damageCalculation`: when halved, `sum := total of components; half := sum / 2; components = append(components, dnd5eEvents.DamageComponent{DamageType: components[0].DamageType, Source: DamageSourceSpell, Roll: dnd5eEvents.RollComponent{Source: dnd5eEvents.RollSource{Ref: cause.EffectRef, Name: sourceName, Label: "halved by a successful save"}, Modifier: ptr(half - sum)}})`. `damageCalculation` builds the trace from the components unchanged, so all three guards hold; `describeDamage` gains "(halved)" when halved so the step name and the imposed description say so. Rewrite the "THE SUCCESS BRANCH IS UNTOUCHED" docs (`contest.go:846-848`, `ContestOutcome.Imposed` at `:213-216`): a made save against a half gate imposes half.

- [ ] **Step 4: run, PASS; commit** `feat(resolution): a made save against a Half gate deals half, rounded down, as one more component the trace explains`

### Task 3.2: the price, and the two refusal arms deleted

**Files:** modify `resolution/contest.go`, `resolution/contest_move_test.go`.

- [ ] **Step 1: failing tests.** Delete `TestADirectiveNothingCanExecuteIsRefused`'s two subtests. Add, driving `Resolve`:

```go
func (s *ContestMoveSuite) TestAPricedMoveSpendsTheReactionBeforeItIsDescribed() {
	// target is a character with ReactionsRemaining 1; save fails; damage lands
	// outcome.Imposed: ImposedDamage, then ImposedMove{Move.Speed true, Pays reaction}; NotTaken == ""
	// the target's CanReact() is now false (the spend request reached the keeper on the interaction bus)
}
func (s *ContestMoveSuite) TestAPricedMoveWithNoReactionIsRecordedAsNotTaken() {
	// ReactionsRemaining 0: ImposedDamage, then ImposedMove with NotTaken == "has no reaction to spend"; Move still carried so the reader knows what was asked
}
func (s *ContestMoveSuite) TestAMonsterPaysFromItsMeter() { /* monster target; after the cast, monster.CanReact() is false */ }
func (s *ContestMoveSuite) TestADroppedTargetIsNotAskedToPay() { /* damage drops it: no ImposedMove at all, CanReact unchanged */ }
func (s *ContestMoveSuite) TestASpeedBudgetIsDescribedAsIs() { /* Speed true survives to the ImposedMove */ }
```

- [ ] **Step 2: run, see them fail** (`validateMove` refuses; `NotTaken` undefined).
- [ ] **Step 3: implement.** Delete the two arms at `contest.go:497-505` and rewrite the doc at `:475-485` (the successor arrived). `ImposedEffect` gains `NotTaken string \`json:"not_taken,omitempty"\`` (doc: why an imposed move was not carried out; empty when it was; today the only reason is the price). Insert `payForMove` between `deliverRemoval` and `deliverMove` (`contest.go:861-884`):

```go
payForMove := func() (Step, error) {
	if m.in.Move == nil || m.in.Move.Pays != combatActions.PaysReaction { return deliverMove() }
	return Gather{
		name: "pay for " + describeMove(*m.in.Move),
		run: func(ctx context.Context, bus events.EventBus) (Step, error) {
			target, err := combatantFor(m.cast, m.in.SaverID)
			if err != nil { return nil, err }
			if combat.IsDown(target) { return done() }           // the dropped are not asked
			if !target.CanReact() {
				moved := *m.in.Move
				outcome.Imposed = append(outcome.Imposed, ImposedEffect{Kind: ImposedMove, Ref: cloneCoreRef(m.in.Cause.EffectRef), Description: describeMove(moved), RecipientID: m.in.SaverID, Move: &moved, NotTaken: "has no reaction to spend"})
				return done()
			}
			if err := dnd5eEvents.SpendRequestedTopic.On(bus).Publish(ctx, dnd5eEvents.SpendRequestedEvent{MemberID: m.in.SaverID, ActionType: coreCombat.ActionReaction, Amount: 1, SourceRef: cloneCoreRef(m.in.Cause.EffectRef)}); err != nil { return nil, err }
			return deliverMove()
		},
	}, nil
}
```

and `deliverRemoval`'s two continuations call `payForMove()` instead of `deliverMove()`. Doc it beside `imposeMove`: paid first, walked after; a wall does not refund (directed-movement §4, §9).

- [ ] **Step 4: run, PASS; commit** `feat(resolution): a move priced at a reaction is paid before it is described; NotTaken records a price that could not be paid; the two refusal arms are gone`
- [ ] **Step 5: pin root at its tag once PR 1 merges; push; draft PR.** Expected tag `resolution/v0.42.0`.

---

## PR 4 — `session`: the speed budget, `Away` crossed, `NotTaken` mapped, the cast that pauses, the react that resumes

Branch `feat/whispers-walk`; title `feat(session): a speed budget from the roster, Away crossed to encounter, a cast whose flee pauses returns Paused, and React resumes a held directive`. Pin encounter, resolution and root at pseudo-versions; at tags before ready.

### Task 4.1: the speed budget, the crossing, `NotTaken`

**Files:** modify `session/castmove.go`, `session/castoutcome.go`, `session/reach.go` (or beside `rosterPositions`), tests beside them.

- [ ] **Step 1: failing tests** (drive `Manager.Cast` end to end with a bard whose profile is Whispers and a goblin with `SpeedFeet 30`): the goblin fails; the cast beat carries `ResultMoved{Moved: n ≤ 6}`; the goblin's cell changed by `n`; every moved beat carries the spell as cause; a goblin with `SpeedFeet 25` routes at most 5. A goblin whose reaction is spent (spend it first through the keeper) gets `ResultMoved{Moved: 0, StoppedBy: "has no reaction to spend"}` and does not move. `routePolicy(combatActions.MoveAway)` yields `encounter.MoveAway`.
- [ ] **Step 2: run, see the Speed refusal.**
- [ ] **Step 3: implement.** Delete `castmove.go:98-105`. Beside `rosterPositions`, `rosterSpeeds(roster) map[string]int` (id → `SpeedFeet`). In `routeCastPushes`: `budget := push.move.Cells; if push.move.Speed { budget = encounter.CellsFromFeet(speeds[string(push.target)]) }`. `routePolicy` gains `case combatActions.MoveAway: return encounter.MoveAway, true`; its doc's "one member each" becomes "two". In `castOutcome` (`:57-68`): a push is collected only when `applied.NotTaken == ""`; in `imposedResult`'s move arm, `StoppedBy: imposed.NotTaken` (zero `Moved`, which validates).
- [ ] **Step 4: run, PASS; commit** `feat(session): a speed budget is read off the roster row; Away is crossed; a move not taken is a zero result that says why`

### Task 4.2: a cast whose flee pauses; React resumes a held directive

**Files:** modify `session/cast.go`, `session/castmove.go`, `session/react.go`, `session/types.go` (`CastOutput`), `session/mover.go` (doc only), tests: `session/cast_test.go`, `session/react_test.go`.

- [ ] **Step 1: failing tests:** a fighter (player) stands beside the goblin's flee path; the bard casts Whispers; the goblin fails. `Cast` returns `CastOutput{Paused: true}` with no error; the session record has one open window for the fighter; the cast beat and the window beat are on the story; the goblin has not moved past the announced cell. Then `React{Member: fighter, Choice: ReactStrike}`: the fighter's attack beat lands on the goblin (reaction identity "Opportunity Attack"), the walk finishes (moved beats with the cause follow), `PausedMember() == ""`, the session is no longer frozen. A second scenario with `ReactHold`: no attack beat; the walk finishes. A third: the bard themself is adjacent and hostile with a melee weapon; the bard is asked on their own turn, answers strike, hits. A fourth: two player reactors on one step: two windows; both answered before the walk resumes (`holdRemainingWindows` untouched).
- [ ] **Step 2: run, see `Cast` error with `ErrStepPaused` after the record.**
- [ ] **Step 3: implement.** `walkCastPushes` returns `(paused bool, err error)`: a `DirectOutput.Paused` is not an error; the loop stops at the first pause (one target for this spell; note the multi-push gap in the doc). `cast.go:421-429`: on `paused`, skip nothing, fall through to `commit`, set `CastOutput.Paused = true` (new field, doc mirroring `EndTurnOutput.Paused`). `react.go:190-197`:

```go
if len(open) == 0 {
	switch {
	case scope.enc.HeldDirective():  // new bool beside Paused(); or PausedMember() != "" && !scope.enc.TurnPaused()
		if _, err := scope.enc.ResumeDirective(ctx); err != nil { return nil, fmt.Errorf("react: %w", translate(err)) }
	case scope.enc.Paused():
		if _, err := scope.enc.ResumeTurn(ctx); err != nil { ... }
	}
}
```

(Encounter needs one exported discriminator; add `Encounter.HeldDirective() bool` in PR 2 if the builder there has not, and pin.) Rewrite the comment at `react.go:293-298`: the replay is correct for a provoking directive because the seam reads no cause when the step is not forced; the test above is the proof. `mover.go:277-283` doc: the day came.

- [ ] **Step 4: run the module; commit** `feat(session): a cast whose flee pauses for a player commits and returns Paused; React resumes the held directive`
- [ ] **Step 5: pins to tags; push; draft PR.** Expected tag `session/v0.76.0`.

---

## PR 5 — `rpg-api`: the seed knows the spell

Branch `feat/dissonant-whispers` off `dev`. Pin the four toolkit modules at their tags (pseudo-versions while walking).

- [ ] `sandboxseed.go:46`: `dissonantWhispersRef = "dnd5e:spells:dissonant-whispers"`; `:198`: `SpellRefs: []string{baneRef, thunderwaveRef, dissonantWhispersRef}`. It must land with root's `Count: 3` or `FinalizeDraft` fails. No handler change: the cast request already carries a member target; `MoveImposed` already carries `moved_cells`/`stopped_by`; the reaction window already reaches the client.
- [ ] Handler tests unchanged; the seed's own asserts prove the pick. Draft PR to `dev`.

## `rpg-dnd5e-web`

Nothing. A member-target cast arms on click; the reaction window draws ahead of the dock. If the walk shows the window not opening for the caster's own swing, that is a finding on the toolkit, not the client.

---

## The walk

`game-dev/envs/local/whispers.env` (ports unclaimed by any other manifest: check `envs/local/*.env` and `ss -ltn` first; `local/thunderwave` still holds :8096/:3015), `RPG_API_PATH` at PR 5's worktree pinned to the toolkit pseudo-versions, `RPG_DND5E_WEB_PATH` at `dev`.

In the reference tomb: the bard beside a skeleton, the fighter beside its open side. Cast Whispers on it.

1. It saves: the damage row shows 3d6, a "halved by a successful save" line, and the halved total; it does not move.
2. It fails: full damage; the cast beat says moved n; the skeleton runs toward the far corner; the fighter is asked and swings; the beats read cast, window, attack, moved, moved… ; on the skeleton's next turn it takes no opportunity attack when the bard walks away from it.
3. The bard adjacent with a weapon: the bard is asked on their own turn and swings.
4. Thunderwave on a skeleton that saves: half of 2d8 and no push.

Record the walk on #430 and #437.

## Self-review against the design

- §3 half: PR 1.1 (vocabulary), PR 3.1 (delivery + trace component + report), PR 1.3 (Thunderwave flips). Acceptance 1–3.
- §4 Away: PR 1.1 (content word), PR 2.1 (route), PR 4.1 (crossing, speed budget). Acceptance 4–6.
- §5 price and meter: PR 1.2 (meter, flag deleted, floor), PR 3.2 (paid before described, `NotTaken`), PR 4.1 (mapped). Acceptance 7–8, 12.
- §6 provoke and held walk: PR 2.2 (hold, resume, freeze), PR 4.2 (cast returns Paused, React resumes, replay proven). Acceptance 9–11.
- §7 cast: PR 1.3, PR 5. Acceptance 13 by every module's regression run.
- Types consistent across PRs: `actions.MoveAway`, `encounter.MoveAway/HeldDirectiveData/ResumeDirective/DirectOutput.Paused/HeldDirective()`, `resolution.ImposedEffect.NotTaken/validateGate`, `session.CastOutput.Paused/rosterSpeeds`, `monster.Data.ReactionSpent`.
- Known thin spots carried from design §11: the zero-versus-zero guard on a 1-point pool halved; two provoking pushes from one cast; the resume traced by survey, not run.

— cross-team agent, on behalf of KirkDiggler
