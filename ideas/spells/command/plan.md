---
status: PLAN, 2026-09-12
design: ./design.md (approved; first slice = Approach, Flee, Grovel)
surveys: survey-command.md, survey-command-plan.md (job tmp; every file:line below is from origin/main 2026-09-12 — rulebooks/dnd5e v0.157.0, encounter v0.74.0, resolution v0.42.0, session v0.76.0)
---

# Command — implementation plan

Seven PRs, merged inside-out: **protos → root → encounter → resolution →
session → rpg-api → web**. Root and encounter build in parallel once the
proto tag exists. Each PR is built in
its own worktree by one builder and reviewed by a different agent before
merge. A builder who finds the plan wrong says so in the PR and stops;
they do not improvise a seam.

## 0. Corrections to the design the survey forced

Recorded here, and as §12 of the design, so the reasoning stays visible.

| design said | code says | what the plan does |
|---|---|---|
| §3 first draft: a declaration per word, no proto change | the selector is a SHA-256 of the whole marshaled `Definition` (`session/declaration_id.go:257-261`) so the word would have had to be hashed in; Kirk read the flow back: crowded dock, client logic, a band-aid | the option is a CAST-TIME INPUT like the aimed cell (`CastInput.Cell`, `session/cast.go:74`; `CastRequest.cell = 6`): `Declaration.Options` lists the menu, `CastInput.Option` carries the choice, two additive proto fields |
| §4 "caster filled by resolution", §11 "templating has no precedent" | `CounterpartKey` is the precedent (`combat/actions/cast.go:165-175`, `resolution/action.go:718-745`); Vicious Mockery uses it | `CounterpartKey: "caster_id"`; the word reaches the parameters by a sibling `OptionKey` bound the same way |
| §4 "a second Command replaces the first" | no replacement exists; both sheets append (`character.go:1186`, `monster.go:405`). ~~Bane from two casters stacks two −1d4 today~~ — **WRONG, caught by the resolution reviewer 2026-09-12:** `BanedContributionGroup` already makes them non-stacking and removal compares full addresses; keying replacement on the REF would have let a second caster's Bane end the first caster's concentration | resolution removes an existing instance with the same ADDRESS on the recipient before publishing the new one; bit-identical for Commanded; Bane's two-caster case untouched; the gateless path does not see it (shelf) |
| §5.1 "session decides Driven in `participationNow`" | that name is encounter's (`encounter/participation.go:85`), whose validation switch (`:130-141`) rejects unknown values; session's is `standingSeam.participation` (`session/participation.go:62`) + `encounterParticipation` (`:156-178`), and it already holds each record's raw `Conditions` | both switches gain `Driven`; session answers it from the raw conditions with one new helper |
| §5.2 "`turnDriverSeam` becomes two layers holding a sheet map" | the seam is built ONCE at `session/session.go:168` with nothing session-scoped in hand | the compelled driver is built per verb at the three encounter-construction sites (`read.go:560`, `start.go:167`, `write.go:1194`), the lifetime rule `standingSeam` already follows (`standing.go:49-66`) |
| §5.3 `imposeCondition` | the body is `publishCondition` (`resolution/contest.go:416`) via `prepareCondition` (`:332`) and `publishPreparedCondition` (`:387`) | names corrected |
| §5.4 `Routed` "terminal" | a paused turn resumes into `runTurnIntents` at the next intent index and asks the driver again (`clocks.go:500-567`, `pause.go:421+`) | `pausedTurn` gains `terminal bool` (persisted); a resumed routed walk ends the turn without a further `Act` |

## 1. Facts every builder needs

- **Rules:** one worktree per builder under `rpg-toolkit/.worktrees/<name>`;
  plain `git worktree add`, never `EnterWorktree`. Never `--no-verify`, never
  any `git -c` flag. Commit only green. Name the suite in `-run` and grep
  `=== RUN`. Never pin a collection's length. Draft PR on first push,
  title prefix drives the tag bump (`feat:` minor). Commit trailer
  `Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>`; no session
  URLs anywhere.
- **The spell text (recalled, not cited):** 60 ft, one creature, Wisdom
  save, no effect on a success. Approach: "moves toward you by the shortest
  and most direct route, ending its turn if it moves within 5 feet of you."
  Flee: "spends its turn moving away from you by the fastest available
  means." Grovel: "falls prone and then ends its turn."
- **Conditions:** factory `conditions.CreateFromRef` (`conditions/factory.go:
  16-28`, closed switch `:70-119`); loader map `conditions/loader.go:17`
  (Blade Ward row `:179-185`); display map `conditions/display.go:~40-84`
  (Blade Ward `:74`); refs `refs/conditions.go:53,163`. Blade Ward
  (`conditions/blade_ward.go`) is the model: struct `:77-94`, ctor
  `:108-117`, `Apply` `:124` (refuses double-apply), `onTurnEnd` `:243-253`,
  `end` `:266-279` (publishes `ConditionRemoved` FIRST, then removes),
  `ToJSON`/`loadJSON` `:282-301`. Pinning test
  `TestEveryConditionRefMatchesItsToJSON` (`character/character.go:1222`).
- **Cast → condition:** `prepareCondition` (`resolution/contest.go:332-367`,
  generic factory call `:357-362`, `Parameters` passed verbatim as
  `Config`); `bindCounterpart` (`resolution/action.go:718-745`) writes the
  counterpart id under `CounterpartKey` into the parameters; recipient/
  counterpart chosen at `action.go:684-687`; gated casts bind too (Bane is
  gated and uses it; pinning test `resolution/cast_action_test.go:543`).
- **Offers:** `buildCastOffers` (`session/casts.go:65-129`) loops known refs
  and calls `spells.CastDefinition` (`spells/cast.go:469-490`) ONCE per
  spell; `compileCastOffer` (`casts.go:149`); `sortCastOffers`
  (`casts.go:342-346`) sorts by `Spell.Ref` only; `Cast` regenerates offers
  and string-matches the id (`session/cast.go:282-288`,
  `offers.go:805-828`). Precedent for one entry → many rows: Attack's
  off-hand/Martial Arts (`offers.go:368-442`); for two cast rows:
  `session/cast_test.go:469 TestTwoCastableCantripsAreTwoRows`.
- **Participation:** encounter values `encounter/participation.go:14-31`,
  validation switch `:130-141`, drive switch `encounter/clocks.go:395-411`
  (a `Wait` player STOPS the drive at `:403-405`). Session:
  `standingSeam.participation` (`session/participation.go:62-140`) gets
  records from `recordsFor` (`session/standing.go:163-216`) carrying
  `character.Data`/`monster.Data`, both with `Conditions []json.RawMessage`
  (`character/data.go:105`, `monster/data.go:42`); mapping at
  `encounterParticipation` (`:156-178`). Every condition's JSON carries a
  `ref` field (`blade_ward.go:283-288`).
- **Driving:** `driveOneMonsterTurn` (`clocks.go:469-486`, budget
  `MovementFeet: m.SpeedFeet` at `:477`); `runTurnIntents` (`:500-567`,
  view `:508`, `Act` `:513`, held-turn return `:530-531`);
  `executeTurnIntent` switch (`:757-862`: `Pass` `:762-763`, `Move`
  `:787-857` incl. charge `:810`, pause `:821-840`, zero-progress-ends
  `:857`); `endDrivenTurn` (`:575-628`); `walkPath(ctx, mover, m, path,
  audience, at, cause core.Ref, forced bool)` (`:935-1010`); `pausedTurn`
  (`pause.go:98`, persisted `pause.go:173/200`, `data.go:1344/2339`);
  `ResumeTurn` (`pause.go:421+`). Session seam: `turnDriverSeam`
  (`session/turndriver.go:203-232`), twins `TurnIntent` `:156-193`,
  projection `:251-293`; built at `session/session.go:168`; handed on at
  `read.go:560`, `start.go:167`, `write.go:1194`.
- **Routing:** `Route` switch `encounter/directive.go:169-181`; `routeAway`
  `:261-305` (flood with `Limit`, strictly-farther scan, pinned sentence
  `:292-294`); `floodFrom` `clocks.go:1334-1355` (Limit 0 = unbounded);
  `routeTo` `clocks.go:1268-1286` + `nearestStop` `:1376-1396` (the BFS
  `SeenMember.Path` uses, `:1202-1204`); `ErrUnsupportedPolicy`
  `encounter/errors.go:580`; test to edit
  `encounter/directive_test.go:328 TestAnyPolicyButTheTwoIsRefused`.
- **Policy/budget sites:** `combat/actions/move.go` (`MoveLine` `:27`,
  `MoveAway` `:35`, `CastMove` `:67-88`, `Validate` `:92-120` with the ONE
  exclusivity expression `:109`); `resolution/contest.go` (`MoveDirective`
  `:127-153`, `validateMove` `:563-576` with the `declared` literal
  `:564-567`, `describeMove` `:585-601`); `resolution/action.go:650-663`
  `directiveFor`; `session/castmove.go:234-243` `routePolicy`, budget
  `:107-110`.
- **Test doubles that record `Act`:** `scriptedDriver`
  (`encounter/monsterturn_test.go:329-345`, records `calls`);
  `recordingBehavior` (`session/monster_turn_test.go:784-798`).
- **rpg-api:** seed `internal/sandboxseed/sandboxseed.go:204` (+ comment
  `:36-49`, consts `:34-52`); creation test derives the count
  (`internal/integration/character/creation_test.go:1795`, add a
  `Contains`); `nobodyDown.Assess` (`internal/sessionworld/sessionworld.go:
  655`) answers `Wait` and never needs `Driven`; converter switch on spell
  identity at `internal/handlers/dnd5e/v1alpha1/character/converters.go:
  ~3249` may need a `Command` arm (check).

## 2. PR 0 — rpg-api-protos

Branch `feat/cast-options` off `main`. Additive only, per the protos repo's
rules (buf lint/format/breaking; generate compiles; no hand-written tests).

- `dnd5e/api/session/v1alpha1/types.proto`, `message Declaration`
  (`:1031`): after `cost = 18`:
  ```proto
  // The menu a cast offers when the spell has one (Command's words). The
  // choice is a cast-time input like `cell`: the client sends one id in
  // CastRequest.option. Empty means the cast has no choice.
  repeated CastOption options = 19;
  ```
  and `message CastOption { string id = 1; string label = 2; }` beside
  `SpellRef` (`:1242`), the label authored by the spell, never derived.
- `dnd5e/api/session/v1alpha1/service.proto`, `message CastRequest`
  (`:579`): after `cell = 6`:
  ```proto
  // The id of one of the selected declaration's options. Set when — and
  // only when — the declaration lists options, where it is required.
  string option = 7;
  ```
- Merge and tag before PR 5 pins; PRs 1–4 need nothing from it.

## 3. PR 1 — dnd5e root (`rulebooks/dnd5e`)

Worktree `dw-root` is gone; use `cmd-root`. Branch `feat/command-content`.
Title `feat(spells): Command — the word, the compulsion, Toward, the turn budget`.

### 1.1 `combat/actions`: the option menu, the option key, Toward, the turn budget

- `cast.go` after `Concentration` (`:126`):
  ```go
  // Options is the menu a cast offers when the spell has one: Command's
  // words. The choice is a cast-time input, the way an aimed cell is: the
  // declaration lists the menu, the request carries one id, and the engine
  // writes it into the parameters of every effect that names an OptionKey.
  // Absent means the spell has no choice, which is every profile before
  // Command.
  Options []CastOption `json:"options,omitempty"`
  ```
  ```go
  type CastOption struct {
      ID    string `json:"id"`    // "approach"
      Label string `json:"label"` // "Approach", the client's word
  }
  ```
  `Validate`: ids non-empty and unique, labels non-empty; `Options`
  non-empty with no `Effects` row carrying an `OptionKey` is refused (a
  menu nothing reads). `func (p CastProfile) HasOption(id string) bool`.
- `CastEffect` after `CounterpartKey` (`:175`):
  ```go
  // OptionKey names the parameter the chosen option is written under, the
  // way CounterpartKey names the other party's. Empty means the effect does
  // not read the option.
  OptionKey string `json:"option_key,omitempty"`
  ```
  `Validate`: an `OptionKey` on a profile with no `Options` is refused.
- `move.go`: `MoveToward MovePolicy = "toward"` after `MoveAway` (`:35`),
  doc: shortest walking route to a cell adjacent to the anchor, stopping
  there; if none, the reachable cell nearest the anchor by ruler. Rewrite
  the `MoveLine` doc line "'toward' is still waiting for Thorn Whip"
  (`:16-21`): Command brought it. `CastMove` gains
  `Turn bool \`json:"turn,omitempty"\`` after `Speed` (`:79`): "the mover's
  remaining movement on its own turn; only meaningful when the move IS the
  mover's turn, which resolution's Obey produces and no cast does." `Validate`
  (`:92-120`): policy switch adds `MoveToward`; the exclusivity expression
  at `:109` becomes a count: exactly one of `Cells > 0`, `Speed`, `Turn`.
  Tests in `combat/actions/cast_test.go` and a new `move_test.go`: each of
  the three budgets alone passes, any two refused, none refused, `Toward`
  accepted; `Options` without a key refused; key without `Options` refused;
  `HasOption`.

### 1.2 `conditions`: `Commanded`

- `refs/conditions.go`: `conditionCommanded` (`ID: "commanded"`) beside
  `:53`, accessor `Commanded()` beside `:163`.
- `conditions/commanded.go`, mirroring `blade_ward.go` shape for shape:
  ```go
  type CommandedConditionData struct {
      Ref          *core.Ref `json:"ref"`
      MemberID     string    `json:"member_id"`
      SourceRef    string    `json:"source_ref"`
      CasterID     string    `json:"caster_id"`
      Word         string    `json:"word"`
      TurnEndsLeft int       `json:"turn_ends_left"`
  }
  ```
  Exported getters `CasterID()`, `Word()`. Constructor
  `NewCommandedCondition(memberID, sourceRef, casterID, word string, turnEnds int)`
  refuses empty caster, empty word, non-positive count. `Apply` subscribes
  `TurnEndTopic` (own `SubjectID` only, count down, `end(ctx, "expired")`)
  and `CombatEndTopic` (`end(ctx, "combat ended")`). `end` publishes
  `ConditionRemoved` first, then `Remove`, exactly `blade_ward.go:266-279`.
  Contributes to no roll.
- Factory row (`factory.go:70-119`): config struct
  `commandedConfig{CasterID string \`json:"caster_id"\`; Word string \`json:"word"\`; TurnEnds int \`json:"turn_ends"\`}`;
  builder refuses each missing field, never defaults.
- Loader row (`loader.go:17`), display row (`display.go`, Name
  "Commanded").
- `conditions/holds.go`: the one helper session needs:
  ```go
  // HoldsRef reports whether one of a sheet's stored condition blobs is
  // the given ref, reading only the "ref" field every ToJSON writes. It
  // loads nothing and attaches nothing.
  func HoldsRef(stored []json.RawMessage, ref *core.Ref) (bool, error)
  ```
  and `DecodeCommanded(stored []json.RawMessage) (*CommandedConditionData, bool, error)`
  returning the first Commanded blob's data (the driver's read).
- Tests `conditions/commanded_test.go` mirroring `blade_ward_test.go:137,
  154, 164, 175, 191`: survives nothing (count 1: the owner's first turn end
  takes it); somebody else's turn end does not; combat end takes it; exact
  wire shape; hand-authored blob keeps its fields; ctor refusals; `HoldsRef`
  true/false/garbage.

### 1.3 `spells`: the profile

- `spells/cast.go`: consts `CommandRangeFeet = 60`, `CommandTurnEnds = 1`,
  `CommandCasterParameter = "caster_id"`, `CommandWordParameter = "word"`.
  `castContent` row `Command` per design §6 with `Options`
  `{approach, flee, grovel}` and one effect
  `{Recipient: CastRecipientTarget, Ref: *refs.Conditions.Commanded(),
  Parameters: {"turn_ends":1}, CounterpartKey: CommandCasterParameter,
  OptionKey: CommandWordParameter}`. Comments carry §7's three rulings
  (undead diverged until goblins; language deferred; Drop deferred on the
  weapon-posing initiative) and that Halt is a one-row addition.
  `spells.CastDefinition` (`:469-490`) is unchanged: one definition, whose
  profile carries the menu.
- `spells/data.go`: a `Command` row (level 1, enchantment, V, 1 round; the
  bard requirement test reads `Level`).
- The bard's level-1 pick: wherever Whispers made it 3 of 3
  (`character/choices/requirements.go` near `:672`), 4 of
  `{Bane, Thunderwave, DissonantWhispers, Command}`. Comment: the count
  tracks the catalogue and stops at four (rpg-toolkit#1661). The domain
  cleric grant (`subclass_modifications.go:257`) now names a spell that
  exists.
- Tests `spells/cast_test.go` (suite `CastContentSuite`): Command's
  profile carries the gate, three options, the effect with both keys; every
  other profile has no options (loop the catalogue, no counts pinned); the
  requirements test for the bard's pick.

### 1.4 Gate

`go build ./... && go test ./...` in `rulebooks/dnd5e`; the hook runs on
`.go` changes. Reviewer: mutation the exclusivity count (allow two budgets)
and the `Options`-without-`OptionKey` refusal; both must be caught.

## 4. PR 2 — encounter

Worktree `cmd-encounter`, branch `feat/command-driven-turn`, title
`feat(encounter): Driven participation, Routed intent, Toward`. Pins root at
PR 1's pseudo-version (`go get …/rulebooks/dnd5e@<sha>`; a go.mod-only
commit runs no hook, run build+tests by hand).

### 2.1 `TurnParticipationDriven`

- `participation.go:14-31`: add the constant with design §5.1's doc.
- `participationNow` validation switch `:130-141`: accept it.
- `driveTurnsWithParticipation` `clocks.go:395-411`: a `Driven` arm that
  falls through to `driveOneMonsterTurn` (`:414`) for any `Kind`. The
  `Wait`+player stop at `:403-405` is untouched.
- Tests `participation_test.go`: a `Driven` player is driven (the scripted
  driver records one call, the turn ends, `"turn-ended"` beat); a `Wait`
  player still stops the drive; `participationNow` accepts `Driven` and
  still refuses an unknown word.

### 2.2 `MoveToward` in `Route`

- `directive.go`: `MoveToward` const after `MoveAway` (`:81`); `Route`
  switch (`:169-181`) gains `case MoveToward: return e.routeToward(...)`.
  Anchor on the mover: the empty route (already there), not a refusal.
- `routeToward(mover, from, anchor, budget)`: flood with `Limit: budget`
  (`floodFrom`, as `routeAway :266`); goal = a STANDABLE reached cell with
  `e.Distance(cell, anchor) <= 1` and fewest steps (`nearestStop`
  `:1376`); if `from` already satisfies the goal, empty path, no
  `StoppedBy`. If no goal cell is reached: the reached standable cell with
  the least `e.Distance(anchor, cell)` strictly less than the mover's own;
  ties fewer steps then scan order; none ⇒ empty path,
  `StoppedBy "nowhere nearer to <anchor> within n cells"`. Budget 0 ⇒
  empty route (mirrors `routeAway :262-264` semantics: say why in the doc;
  zero is not unbounded here).
- Tests `directive_test.go`: edit `:328 TestAnyPolicyButTheTwoIsRefused` to
  "the three"; Toward stops adjacent and not on the anchor; takes the
  shorter of two corridors; blocked corridor ⇒ nearest-by-ruler fallback;
  pinned ⇒ empty + sentence; may cross an ally, may not stop on one;
  budget respected. Reviewer mutants: drop the `Standable` filter; make
  fallback `<=` instead of `<`.

### 2.3 `Routed` intent, terminal

- `turndriver.go`: `Routed{Policy MovePolicy; Anchor MemberID}` with
  `isTurnIntent()` and design §5.4's doc verbatim, plus: "This is the
  fourth case the sealed vocabulary's doc said should earn its own ADR; the
  ADR is the Command design (rpg-project ideas/spells/command)."
- `executeTurnIntent` (`clocks.go:757-862`) new arm before `default`:
  anchor must be a placed member (else `ErrBadIntent` ⇒ turn ends);
  `cells := CellsFromFeet(budget.MovementFeet)`; `route := e.Route(RouteInput{Mover, Policy, Anchor: anchorCell, Budget: cells})`
  (an error is a Go error); empty path ⇒ `done=true` with no beat (the
  `"turn-ended"` beat follows; `StoppedBy` is returned to the driver's
  caller in the drive output only if a field already exists for it, else
  dropped, and the test pins which); else `walkPath(ctx, activeID, m, path, audience, at, in.Cause, false)`
  where `Cause` is a new `Routed.Cause core.Ref` (required; `ErrNoCause`
  like `Direct`), charge `:810`-style, `settleWalk`, pause handling as
  `Move` `:821-840` **with `terminal: true`**, then `return true`.
- `pausedTurn` (`pause.go:98`): `terminal bool`, persisted
  (`pausedTurnDataFrom :173`, `pausedTurnFrom :200`, data tags
  `terminal,omitempty`). `ResumeTurn`'s finisher (`pause.go:541/583`):
  when `terminal`, after the remaining walk (which may re-pause and keep
  the flag) end the turn via `endDrivenTurn` instead of re-entering
  `runTurnIntents`.
- Tests `monsterturn_test.go` with `scriptedDriver`: Routed Toward walks and
  ends the turn with the brain asked once; Routed Away likewise; empty
  route ends the turn; a pause mid-walk holds with `terminal` persisted
  (round-trip `ToData/Load`), `ResumeTurn` finishes the walk and ends the
  turn with no further `Act` (assert `len(driver.calls)==1`); a Move-paused
  turn still resumes into another `Act` (terminal false pinned); `Routed`
  with no cause refused. `TurnIntent` twin tests as the existing ones.

### 2.4 Gate

`go build ./... && go test ./...` in `encounter`. Reviewer: mutant "resume
ignores terminal" must be caught; mutant "Routed charges no budget" must be
caught (a second Routed in the same turn would walk again).

## 5. PR 3 — resolution

Worktree `cmd-resolution`, branch `feat/command-obey`, title
`feat(resolution): Obey, option binding, same-ref replacement, Toward and the turn budget`.
Pins root at PR 1's sha.

### 3.1 The option reaches the parameters

- `ActionInput` (`action.go`, the struct `NewAction` takes at
  `session/cast.go:315-328`) gains `Option string`: the chosen id, already
  validated by session against the profile; resolution re-checks
  `definition.Cast.HasOption` and refuses a mismatch or an empty option on
  a profile with `Options` (fail closed twice, once per owner).
- `bindOption(effect, option string) (json.RawMessage, error)` beside
  `bindCounterpart` (`:718-745`), same shape: no `OptionKey` ⇒ verbatim;
  else write the id under the key. Call it where `bindCounterpart` is
  called on BOTH paths (gateless `:688`; find the gated one via
  `cast_action_test.go:543`'s pin).
- Test `cast_action_test.go`: the word is written where content said on a
  gated cast; a profile with options and an empty input option is refused;
  an id not in the menu is refused; unkeyed effects untouched.

### 3.2 Same-ref replacement

- `publishPreparedCondition` (`contest.go:387-404`): before publishing, if
  the recipient's sheet (the cast's `Participant`, character or monster,
  `GetConditions()`) holds a condition with the same `Ref()`, publish
  `ConditionRemovedEvent{MemberID, ConditionRef, Reason: "replaced"}` first.
  Doc: "THE EFFECTS OF THE SAME SPELL DO NOT STACK. One instance of a ref
  per member; the newer replaces the older. Bane from two casters stacked
  two −1d4 before this (2026-09-12); Command's second word replacing the
  first is the use case that brought the rule." The `ImposedEffect` for the
  replaced one: an `ImposedConditionRemoved` (exists, `contest.go:102`)
  with description "replaced by …" so the trace says it.
- Tests `contest_test.go`/`cast_action_test.go`: Command twice on one
  target leaves one Commanded and one removal in the trace; Bane from two
  casters leaves BOTH standing and both concentrations held; a different
  ref does not remove; a monster recipient works the same. (Corrected
  2026-09-12: the first draft said "Bane twice ⇒ one Baned", on a wrong
  premise; see §0.)

### 3.3 `Obey`

- `resolution/obey.go`:
  ```go
  type ObeyInput struct {
      Cast    *Cast   // the same cast the announcer resolves with; the bus, the sheets
      Member  string  // the compelled creature
      CasterID string
      Word    string
      Source  core.Ref // dnd5e:conditions:commanded
  }
  type ObeyOutput struct { Effects []ImposedEffect }
  func Obey(ctx context.Context, in *ObeyInput) (*ObeyOutput, error)
  ```
  Closed switch on `Word`: `"approach"` ⇒ `ImposedEffect{Kind: ImposedMove,
  Move: &MoveDirective{Policy: MoveToward, AnchorID: caster, Turn: true,
  Provokes: true}, RecipientID: member, Ref: source}`; `"flee"` ⇒ the same
  with `MoveAway`; `"grovel"` ⇒ `prepareCondition` for Prone with
  `sourceRef` = the commanded ref, then `publishPreparedCondition` on the
  cast's bus (the prone condition is APPLIED here, on the bus, as a cast's
  would be), returning its `ImposedCondition`; unknown ⇒ `ErrBadAction`
  naming the word. Halt is not a case (design §7.4). Doc: `Obey` moves
  nobody, exactly as `imposeMove` moves nobody.
- `MoveDirective` (`contest.go:127-153`): `Turn bool`; `validateMove`'s
  `declared` literal (`:564-567`) copies it; `directiveFor`
  (`action.go:650-663`) copies it (always false from a cast, but copied so
  a future cast that sets it is not silently dropped); `describeMove`
  (`:585-601`) says "of their own turn's movement" for `Turn`.
- Tests `obey_test.go`: each word's effect shape; grovel applies Prone on
  the bus with the commanded source; unknown word refused; `Turn` survives
  `validateMove`; `contest_move_test.go`: a `Toward` directive is accepted
  where `Away` is.

### 3.4 Gate

`go build ./... && go test ./...` in `resolution`. Reviewer: mutant "bind
option on gateless path only" caught by a gated test; mutant "replacement
publishes after apply" caught (order pinned: removal seq < applied seq).

## 6. PR 4 — session

Worktree `cmd-session`, branch `feat/command-compelled-turn`, title
`feat(session): the cast option, Driven, the compelled driver, Routed across the seam`.
Pins root, encounter, resolution at PRs 1–3 (check the branch is not behind
a same-day tag before pinning pseudo-versions).

### 4.1 The menu on the declaration, the choice on the input

- `Declaration` (`session/afford.go:191-297`): `Options []CastOption`
  (session's own twin `{ID, Label}`, beside `SpellRef` at `types.go:2356`),
  copied from `profile.Options` in `compileCastOffer` (`casts.go:149`,
  next to `MinTargets/MaxTargets` at `:286`). Empty for every other spell.
  The selector is unchanged: one Command row, one id.
- `CastInput` (`cast.go:25-75`): `Option string` after `Cell` (`:74`), doc
  in the cell's voice: "the id of one of the selected declaration's
  Options; required when it lists any, refused when it lists none, refused
  when not listed." Validation in `Cast` right after `selectCompiledOffer`
  (`:288`), before `resolution.NewAction` (`:315`), which receives it as
  `ActionInput.Option`.
- Tests `cast_test.go`: a bard knowing Command gets ONE row whose
  `Options` are the three words in content order; casting with
  `Option: "flee"` lands `Commanded{word: flee, caster: bard}` on a failed
  save (`conditions_test.go` pattern), nothing on a made save; casting
  Command with no option is refused; with `"halt"` refused; casting Bane
  with an option is refused; `declaration_id_test.go`'s golden unmoved
  (the definition did not change shape for existing spells, and Command's
  menu is content, so its own id is stable).

### 4.2 `Driven`

- `encounterParticipation` (`participation.go:156-178`) takes the record's
  raw conditions; after the switch: if `Turn == Wait` and
  `conditions.HoldsRef(stored, refs.Conditions.Commanded())` ⇒ `Driven`.
  `AutoPass`/`Remove` outrank it. Thread the blobs from `recordsFor`'s
  records (`:65`, `standing.go:163-216`) — both `character.Data` and
  `monster.Data` have `Conditions`.
- Tests `participation_test.go`: a commanded conscious player ⇒ `Driven`; a
  commanded monster ⇒ `Driven`; a commanded dying player ⇒ `AutoPass`; a
  garbage blob ⇒ error, not `Wait` (fail closed).

### 4.3 The compelled driver, per verb

- `session/compelled.go`:
  ```go
  // compelledDriver answers for a member that holds a compulsion and
  // otherwise hands the turn to the host's driver. Built per verb beside
  // standingSeam because it reads the same records, and for the same
  // lifetime reason (standing.go:49-66).
  type compelledDriver struct {
      next   encounter.TurnDriver          // m.turnDriver, the seam
      lookup func(encounter.MemberID) (*conditions.CommandedConditionData, bool, error)
      obey   func(ctx context.Context, member encounter.MemberID, data *conditions.CommandedConditionData) (*resolution.ObeyOutput, error)
  }
  ```
  `Act(view)`: `lookup(view.Self)`; error ⇒ return it (aborts the verb,
  fail closed); not held ⇒ `next.Act(view)`; held ⇒ `obey(...)`, then: one
  `ImposedMove` ⇒ `encounter.Routed{Policy: routePolicy(move.Policy),
  Anchor: MemberID(move.AnchorID), Cause: refs.Conditions.Commanded()}`;
  no move ⇒ `encounter.Pass{}`; anything else ⇒ `ErrBadTurnOutcome`.
  `obey` is built the way `announcerSeam` builds its resolve
  (`announcer.go:85` neighbourhood): the cast with sheets and bus, then
  `resolution.Obey`, then `saveDirty` so the prone condition persists.
- Wire at `read.go:560`, `start.go:167`, `write.go:1194`:
  `TurnDriver: m.compelledDriverFor(scope)` returning the wrapper around
  `m.turnDriver`. The other `m.turnDriver` reads (activate, announcer,
  attack, cast, mover, react, striker) are resolution capabilities and stay
  as they are.
- `turndriver.go`: session twin `Routed{Policy, Anchor, Cause}`
  (`:156-193` neighbourhood) and the seam's switch (`:208-228`) crosses it;
  `routePolicy` (`castmove.go:234-243`) learns `MoveToward`.
- Tests `monster_turn_test.go` (suite `MonsterTurnTestSuite`) and a new
  `command_turn_test.go`: acceptance 2, 4–12 from the design, end to end
  through `Cast` then `EndTurn`: Approach walks a monster to the bard with
  `recordingBehavior` recording ZERO views for that turn; Flee walks away;
  Grovel leaves Prone and ends; a commanded PLAYER is driven at `EndTurn`
  of the previous member and their `Move` afterwards is `ErrNotYourTurn`
  until their next turn; the walk past a player opens a window, `React`
  resumes and ends the turn; the condition is gone after; a downed
  commanded creature auto-passes; a second Command replaces the first
  (`Commanded.Word` is the newer). Reviewer: mutant "compelled driver calls
  next when held" caught by the zero-views assertion.

### 4.4 Gate

`go build ./... && go test ./...` in `session`. Reviewer full rigor: this
PR carries the new verb semantics.

## 7. PR 5 — rpg-api

Branch `feat/command` in `rpg-api/.worktrees/cmd-api` off `dev`. Pin the
protos tag and the four toolkit TAGS (verify each tag's content by commit
and a grep) with `game-dev/scripts/bump-toolkit-pin.sh <go.mod dir>
<module> <tag>` from `/home/kirk/game-dev`, three separate args, once per
module; it commits and pushes itself.

- Declaration → proto converter (`internal/handlers/dnd5e/session/v1alpha1/
  convert*.go`, the `Spell` copy near `convert_test.go:524`'s fixture):
  copy `Options` out. Cast handler
  (`internal/handlers/dnd5e/session/v1alpha1/cast.go:54`): `Option:
  req.GetOption()` beside `DeclarationID`. Tests beside the existing ones.
- `sandboxseed.go:52` `commandRef = "dnd5e:spells:command"`; `:204` fourth
  element; comment `:36-49` now says the count reached four.
- `creation_test.go:1795` `s.Contains(..., commandRef)`; sweep
  `bane_playthrough_acceptance_test.go:124`, `sandboxseed_test.go`,
  `gallery.go` for duplicated lists.
- `converters.go:~3249`: check the spell-identity switch; add a `Command`
  arm only if the creation flow maps spells to an enum there.
- Full suite; `make ci-check` only after committing (rpg-api#795).

## 8. PR 6 — rpg-dnd5e-web

Branch `feat/cast-options` off `dev`, after the protos tag and the api
are on dev. One change: when a declaration's `options` is non-empty, the
cast flow asks for one before sending and puts its id in `option`. Read
the cast send sites in `useSessionCombatExperience.ts` (`:1059`, `:1155`,
`:1265`) and the dock (`ActionDock.tsx:531-561`, `castLabel.ts`). No
grouping, no inference: draw the menu that was sent. Tests beside
`castFlow.test.tsx`. Kirk's dock note (multi-target "Cast at Targets"
belongs in the action panel, not the top menu) is a separate web issue,
not this PR.

## 9. Walk manifest

`local/command` env (copy `envs/local/whispers.env`, ports 8098/3017,
`RPG_API_PATH` at the api worktree). Cast Approach on the far ghoul,
end turn, watch it walk to the bard and stop; Flee on an adjacent
skeleton past the fighter, answer the window; Grovel and see Prone on the
sheet; recast on a commanded creature and read "replaced" in the log. Kirk
walks once.

## 10. Self-review

- Every design section has a task: §3 → PR 0/1.1/1.3/3.1/4.1/PR 5/PR 6; §4 → 1.2/3.1/3.2; §5.1
  → 2.1/4.2; §5.2 → 4.3; §5.3 → 3.3; §5.4 → 2.2/2.3/4.3; §6 → 1.3; §7 →
  comments in 1.3; §8 acceptance → 4.3 tests; §10 → the PR order.
- Types agree: `CastOption{ID, Label}` in content, session and proto;
  `CastProfile.Options` + `HasOption`; `CastEffect.OptionKey`;
  `CastInput.Option` → `ActionInput.Option`; `Declaration.Options`; `MoveToward`; `CastMove.Turn`/`MoveDirective.Turn`;
  `TurnParticipationDriven`; `Routed{Policy, Anchor, Cause}` in both
  vocabularies; `pausedTurn.terminal`; `conditions.HoldsRef` /
  `DecodeCommanded`; `resolution.Obey/ObeyInput/ObeyOutput`;
  `Routed.Cause`.
- Not in this plan, on purpose: Halt, Drop, undead, language, a
  turn-started beat, dock grouping or layout.
