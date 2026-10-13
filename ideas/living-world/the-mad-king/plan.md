---
name: The mad king — implementation plan
status: PLAN, derived 2026-10-13 from use-case.md §4–§6; checked against inspected code (baseline below); AGREED by the operator 2026-10-13 (R8 and the assumptions settled); **wave A SHIPPED 2026-10-13** (protos v0.1.234, encounter v0.128.0, session v0.134.0, api dev f1d67fa9, web dev 84bb912a; walked, QA passed on #569); waves B–D not started
authority: use-case.md (rulings R1–R7, routes §3, primitives §4); rpg-project#569; PR #570; the operator's agreement in session 2026-10-13 ("enough to make the plan")
planning guide: ../../../.agents/skills/design/planning.md
---

# The mad king — implementation plan

## Goal and scope

Deliver the primitives the castle use case names, in the order the author
feels them, so that Kirk can author the castle in the studio and walk every
route in §3 of the use case. Four waves, each walked on the local stack
before anything merges:

| wave | delivers | labels |
|---|---|---|
| **A — Speak** | options on a binding, `requires`, one `Speak` verb, the deed is the skill, verb-keyed triggers retire, studio options editor | O1–O5, U1 |
| **B — Hearing** | the sound report, loudness per deed, hop DC vs passive Perception, `heard` and `fact` row conditions, `fact` on `time` rows, patrol by path, players hear | H1–H5, P1 |
| **C — Contact** | passive Stealth at first contact | S1 |
| **D — The castle** | the reference content, the route scenario tests, studio endings, the walk | C1–C2, U2 |

**Non-goals (from the use case, unchanged):** `allied`; per-character stance;
monster-initiated checks; the campaign journal; an authored tree; word
spreads as a mechanism (talkers are authored content once B lands); the
challenge (`stranger` band, `approach` word); a `quorum` field; a mind of
several (a list on `mind:` is small and may ride wave D if the castle wants
it; not planned until it does).

## Requirement labels

From use-case.md §4 and §6. Labels name agreed behaviour; they are not new
rulings.

| label | requirement | source |
|---|---|---|
| O1 | An option on a monster or prop binding carries `id`, `say` (the player's line), `check` (approach list), optional `requires`, and `success` / `failure` answer tables of existing answer rows. Both dialects. | R1, R2 |
| O2 | `requires` takes one predicate: `{ fact }` or `{ holds }`. An option whose predicate does not hold for the actor is not offered. | R1 |
| O3 | One `Speak` verb with an option id replaces Intimidate and Persuade on the wire, in session, in web. A creature with no options cannot be spoken to; the offer and the door agree (`ErrNoOption`). World clock free, turn clock an action, as today. | R2 |
| O4 | A successful check lands the deed of its **skill** (`intimidated` for Intimidation, `persuaded` for Persuasion, none for other skills) on witnesses. The untrained disadvantage source stays attached to the skill in resolution. | R3 |
| O5 | The verb-keyed table triggers (`intimidated`, `intimidate_failed`, `persuaded`, `persuade_failed`) retire from the table vocabulary; reference content migrates to options. | R2 |
| H1 | A heard thing is a report of `{ kind, position, when }` on a per-observer channel. Loudness is a DC per deed kind stamped by the rulebook; the DC rises per region hop (door-to-door), one more for a closed door; the listener's passive Perception answers; no roll. | R4, R5 |
| H2 | `when: { heard: { kind, within: N } }` is a table condition reading the creature's own sound holdings. | R4 |
| H3 | `fact: id` is legal on a `time` row; its audience is the hearers of the speaking creature's cell. Every `say` on a time row lands a `voice` sound. | R6 |
| H4 | `when: { fact: id }` is a table condition reading the creature's own fact holdings. | R6 |
| H5 | A player whose passive Perception beats a sound's DC receives a `Heard` beat with kind and direction; others do not. | R5, perception law |
| P1 | `toward: { path: [cells] }` walks to the next waypoint derived from position; the table holds no cursor. | R7 |
| S1 | At first contact in the "player gets the drop" arm, the mover's Stealth is checked against the watchers' passive Perception; on success no sighting lands for those watchers. | §4, toolkit#1020 |
| U1 | The studio authors options on monster and prop bindings and round-trips them; the read-never-offered `intimidate:`/`persuade:` fields go. | §5 |
| U2 | The studio authors `endings` with the predicate grammar (the castle needs two). | §5, done-when |
| C1 | The castle ships as reference content and every §3 route is an encounter scenario test. | §7 |
| C2 | Kirk authors the castle in the studio and walks each route. | §7 |

## Constraints

- rpg-toolkit: one nearest-`go.mod` module per PR; develop outside-in on
  pushed pseudo-versions; protos merge first; merge inside-out only after the
  wave is walked; CI mints tags; never hand-tag (`rpg-toolkit/CLAUDE.md`).
- Protos: additive, `[deprecated = true]`, never an in-place type change;
  `buf format -w` and `buf breaking` against main (`rpg-api-protos/CLAUDE.md`).
- Zero castle-specific code (use-case.md). Every vocabulary stays sealed and
  grows only the words this plan names.
- The audience law: a fact lands on witnesses (and, from H3, hearers); a
  faction's `until` folds on its mind. No task widens who learns.
- The table holds no state (creature-table R5); `path` is derived (P1).
- No monster rolls a die in any task here (R5; S1 uses the watchers'
  passive).
- Signatures: the charter loaded by the builder; no session or model in
  commits, PRs or comments.

## Inspected baseline (2026-10-13)

| repo / module | revision |
|---|---|
| rpg-toolkit `rulebooks/dnd5e/encounter` (includes `dungeonspec`) | v0.127.0 |
| rpg-toolkit `session` | v0.133.0 |
| rpg-toolkit `resolution` | v0.72.0 (untouched by this plan) |
| rpg-toolkit `mind/behavior` | v0.6.0 |
| rpg-toolkit `mind/perception` | v0.3.0 (untouched; reports are generic) |
| rpg-toolkit `rulebooks/dnd5e` | v0.212.0 (untouched) |
| rpg-api-protos | v0.1.233, HEAD c976ed9 |
| rpg-api `dev` | pins encounter v0.127.0, session v0.133.0 |
| rpg-dnd5e-web `dev` | fbed4863 |

File citations below are against these revisions.

## Sequence

```
A0 protos ──▶ A1 encounter ──▶ A2 session ──▶ A3 api ──▶ A4 web verb ──▶ A-walk
                                                    └──▶ A5 studio ──┘
B0 protos ──▶ B1 mind/behavior ──▶ B2 encounter ──▶ B3 session ──▶ B4 api ──▶ B5 web ──▶ B-walk
C1 encounter (needs B2's passive Perception on SheetFacts) ──▶ C-walk
D1 content + scenario tests (needs A, B, C) ──▶ D2 studio endings ──▶ D-walk (C2)
```

Development dependencies are arrows. Merge order inside every wave is
inside-out: protos → mind/behavior → encounter → session → api → web. Wave B
may be developed in parallel with wave A's api/web tail; its toolkit PRs
touch different files in encounter except `answer.go` legality and
`dungeonspec/grammar.go` — B2 rebases on A1's merged tag, never shares a
branch with it.

## Open items

| item | state |
|---|---|
| **R8 — a row's `when` is a list; every entry must hold; `not: { … }` wraps a negative.** Row ordering rejected. | **settled** (operator, 2026-10-13); B1 ships the list form, B2 parses it; D1's storm-the-keep row unblocked |
| **R9 — `requires: { fact }` is judged against the actor's own holdings.** | settled (operator, 2026-10-13); A1 |
| `requires: { holds: <prop-id> }` means the actor currently holds that placed prop. | confirmed, A1 |
| Speaking to a prop: witnesses are those who see the actor; no deed lands; the check still runs through resolution. | confirmed, A1 |
| Sound DCs (content, tuned on the walk): `boom` 5, `crash` 8, `clang` 10, `voice` 10; +5 per region hop; a closed door is one more hop. `growl` not added. | confirmed, B2 |
| Which deeds make which sound: an attack landing `clang`; a forced door `crash`; a damaging cast `boom`; every `say` `voice`. | confirmed, B2 |
| Deprecated `Intimidate`/`Persuade` RPCs answer `Unimplemented` naming `Speak`. | confirmed, A3 |

---

## Wave A — Speak

### Task A0: Protos — `Speak`, `Spoke`, options on Afford

**Delivers:** O3 wire; O1 option identity on the wire; O4 (the skill rides the beat).
**Owner:** rpg-api-protos, `dnd5e/api/session/v1alpha1`.
**Prerequisites:** none.
**Files:** `service.proto` (rpc list near :2303), `events.proto` (`EventKind` :316, oneof :577, `Intimidated` :2004, `Persuaded` :2051, `Answered` :2319, `AnswerKey` :2173), `types.proto` (`Verb` :406, `ShortfallReason` :321), `afford` messages wherever the social row is defined (follow `VERB_INTIMIDATE` usages).
**Interfaces:**
- `rpc Speak(SpeakRequest) returns (SpeakResponse)`. `SpeakRequest { session_id, actor_member_id, target { oneof: member_id | prop_id }, option_id }`. `SpeakResponse { Event event }` mirroring `PersuadeResponse`.
- `Verb` gains `VERB_SPEAK`; `VERB_INTIMIDATE` and `VERB_PERSUADE` marked `[deprecated = true]`.
- `EventKind` gains `EVENT_KIND_SPOKE`; message `Spoke { actor, target, option_id, line, skill, check { roll faces, total, dc, beaten, sources } }` — the check block is the same shape `Intimidated` carries today, copied not referenced. `Intimidated`, `Persuaded` and their kinds deprecated.
- `Answered` gains `option_id` (string, empty for `time` picks). `AnswerKey` values for the four social keys deprecated; `ANSWER_KEY_SUCCESS`/`ANSWER_KEY_FAILURE` added for option picks.
- Afford: the row for `VERB_SPEAK` lists `repeated SpeakOption { target, option_id, line, available, shortfall }` as `Declaration.speak_options` (field 23; `options` = 19 is already the cast menu — found by A0). `ShortfallReason` gains `SHORTFALL_REASON_NO_OPTION` (also closes the gap that `no_social_entry` has no proto value today, `types.proto:321`).
**Behavior:** additive only; `buf breaking` against main passes.
**Tests:** not applicable (no hand-written tests in protos); evidence is `buf lint`, `buf format --diff`, `buf breaking --against .git#branch=main`, and the generate job.

- [ ] Add the messages and enum values above with doc comments that say what absence means (`option_id` empty = a `time` pick).
- [ ] `buf format -w`; `buf lint`; `buf breaking`.
- [ ] Open the PR ready (protos PRs are not drafts); Kirk merges; CI publishes `gen/go` and the TS package.

**Verification:** from `rpg-api-protos`: `buf lint && buf format --diff --exit-code && buf breaking --against '.git#branch=main'` — all exit 0. After merge: a new tag above v0.1.233 and `gen/go` pseudo-version resolvable from rpg-api.

### Task A1: encounter — options on bindings, `requires`, `Speak`

**Delivers:** O1, O2, O4, O5 (engine side).
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter` (one module; `dungeonspec` is inside it).
**Prerequisites:** none for development (A0 is a wire concern; encounter has no proto dependency).
**Files:**
- `dungeonspec/spec.go`: `PlaceSpec` (:1581) gains `Options []OptionSpec`; `Intimidate`/`Persuade` (:1793/:1805) removed; `laterWords` (:1010) unchanged; the four `On*` social key constants (:972–998) removed.
- `dungeonspec/single_room.go`: `RoomMonsterBinding` (:390) gains `Options`, loses `Intimidate`/`Persuade` (:465/:471); `RoomPropBinding` (:529) gains `Options`.
- `dungeonspec/single_room_site.go` `monsterBindingsShape` (:164–182) and `single_room_gameplay.go` `propBindingsShape` (:223–241): `options` key added, `intimidate`/`persuade` removed.
- `dungeonspec/grammar.go`: new `options(path, []OptionSpec)` validator beside `placeOn` (:294); `wordLegality` (:348) drops the social-key sentences; `knownTableKey` (:531) refuses the four retired keys with the sentence "`intimidated` is no longer a table key: author an option".
- `dungeonspec/validate.go` (:716–726) and `single_room_site.go` `bindingChecks` (:274): call `options` instead of `approaches`.
- `dungeonspec/compile.go`: `approachesOf` (:1166) replaced by `optionsOf`; `MonsterPlacement` (:247/:253) carries `Options`; `single_room_gameplay.go` `applyPropBindings` (:571) carries prop options.
- `field.go`: `MemberInput` (:864) gains `Options []Option`, loses `Intimidate`/`Persuade` (:902/:910) and their copies (:1058–1060, :1382–1384, :1417–1419, :1641–1643) and validation (:1078/:1084); `PlacedPropInput` (`placed_props.go:94`) gains `Options`.
- `data.go` (:486/1683): `OptionData` persisted beside `CheckApproachData`.
- New `option.go`: `Option`, `OptionsFor`, `Speak`. `intimidate.go` and `persuade.go` deleted; `social()` body moves into `option.go`. `answer.go` `answerKeyFor` (:355) deleted; `answer` (:144) takes the rows to pick from rather than a key.
- `deed.go` (:39/:56): `DeedIntimidate`/`DeedPersuade` stay; a `deedForSkill(skill) (DeedVerb, bool)` mapping lives beside them.
**Interfaces:**
```go
type Option struct {
    ID       string
    Say      string            // the player's line, verbatim
    Check    []CheckApproach   // existing type, door.go:105
    Requires *Trigger          // nil = always; only TriggerFact / TriggerHolds accepted
    Success  []Answer          // existing answer rows (fact, flee, say, weight)
    Failure  []Answer
}
type TriggerHolds struct{ Prop string }   // new Trigger kind; evaluated by read, never edge-fired

// Available options of one target for one actor. Evaluates Requires against
// the ACTOR's holdings (facts the actor holds; props the actor holds).
func (e *Encounter) OptionsFor(actor, target MemberID) ([]Option, error)
func (e *Encounter) PropOptionsFor(actor MemberID, prop PropID) ([]Option, error)

type SpeakInput struct {
    Actor    MemberID
    Target   MemberID   // or Prop PropID; exactly one
    Prop     PropID
    OptionID string
    Skill    string     // the approach's ability/skill the check used
    Beaten   bool; DC, Total int; Calculation any; Roller any // as socialInput today
}
func (e *Encounter) Speak(ctx, SpeakInput) (SpeakOutput, error)
```
`SpeakOutput` carries what `social()` returns today plus the picked row.
**Behavior:**
- Validation refuses: an option without `id` or `say`; a duplicate `id` on one binding; an option with no `check`; `requires` in any form but `fact` / `holds`; `holds` naming a prop id not placed; more than one word on an answer row (reuse `answerEntry`); any retired social key on `on:`.
- `OptionsFor` returns only options whose `requires` holds for the actor; absence of options → empty slice, `nil` error (the offer side decides the shortfall).
- `Speak`: refuses `ErrUnwitnessed` as `social()` does (:253); refuses `ErrNoOption` when the id is not on the target or its `requires` does not hold for the actor; appends the `Spoke` beat; if `Beaten` and `deedForSkill(Skill)` is set, lands that deed on witnesses (:269 path); picks from `Success` or `Failure` through `answer()`; `fact` teaches witnesses via `learnFact` (`flip.go:52`); `flee` via `landFled`; spends the world action (:294). A prop target lands no deed and uses the actor's witnesses as audience.
- `Layer`/tables: the four social keys are gone from `AnswerKeys`/`TableKeys` (`table.go:210/220`); `validateEntry` (`answer.go:395`) legality: `fact`/`flee` legal on **option rows** and (after B2) on `time`.
**Tests:**
- `dungeonspec/options_test.go` (new, beside `intimidate_test.go` which is rewritten): compiles the front-room goblin authored with two options; refuses each case above with its sentence; `requires: { holds: lantern }` naming an unplaced prop refused by name; a `v4` prop binding with options compiles into `PlacedPropInput.Options`.
- `dungeonspec/unknown_key_test.go`: `on: { intimidated: … }` refused with the "author an option" sentence.
- goldens (`testdata/world-builder-v4-*.compiled.json`, `contentgolden_test.go`): regenerate; the diff is the removed `intimidate`/`persuade` fields and the added `options`.
- `encounter/option_test.go` (new, from `intimidate_test.go`'s scenes): `OptionsFor` hides an option whose `fact` the actor lacks and shows it after the actor learns it; shows a `holds` option only while the actor holds the prop; `Speak` with Intimidation beaten lands `intimidated` on every witness and none on a non-witness; with Religion beaten lands no deed; picks from `failure` rows when not beaten; `ErrNoOption` on an unknown id and on a hidden option; prop `Speak` lands no deed and still teaches the fact to the actor's witnesses.
- `encounteranswers_test.go`: the time-table tests unchanged (proves O5 removed nothing from `time`).

- [ ] Write the refusal and `OptionsFor` tests first; run `go test ./dungeonspec/ ./ -run 'Option|Speak'` and confirm RED.
- [ ] Implement; delete `intimidate.go`, `persuade.go`, `answerKeyFor`.
- [ ] Regenerate goldens; read the diff.
- [ ] `go test ./...`, `golangci-lint run`, `gorelease` (expect a MINOR bump; the PR title is `feat(encounter)!:` because `MemberInput` loses fields).
- [ ] Push; open a DRAFT; note the pushed sha for A2's `go get`.

**Verification:** from `rpg-toolkit/rulebooks/dnd5e/encounter`: `go test ./... 2>&1 | grep -E '^(ok|FAIL|---)'` all ok; `grep -rn 'intimidate:' testdata content` finds nothing outside migration notes.

### Task A2: session — `Manager.Speak`, options on Afford

**Delivers:** O3 (session side).
**Owner:** rpg-toolkit `session`.
**Prerequisites:** A1 pushed; `go get github.com/KirkDiggler/rpg-toolkit/rulebooks/dnd5e/encounter@<A1 sha>`.
**Files:** `social.go` (`socialVerb` :51, `speak` :171, `socialEntriesOf` :305) rewritten around one verb; `intimidate.go`, `persuade.go` deleted; `afford.go` (`VerbIntimidate`/`VerbPersuade` :117/127, `socialRowsOnTheWorldClock` :923, blockers :655/:868, ordering :670), `offers.go` (:300–307, `buildSocialOffer` :773, `socialShortfall` :823, `candidatesFor` :891); `types.go` (`ShortfallNoSocialEntry` :3698 → `ShortfallNoOption = "no_option"`); `errors.go` (:346 `ErrNoSocialEntry` → `ErrNoOption`); `landings.go` `landResumedSocial` (:484) keeps the pose window for Bardic Inspiration, keyed by option.
**Interfaces:**
```go
type SpeakInput struct { Session SessionID; Actor MemberID; Target *MemberID; Prop *PropID; OptionID string }
func (m *Manager) Speak(ctx, SpeakInput) (*SpeakOutput, error)
// Afford: VerbSpeak row; per candidate target, []SpeakOptionRow{Target, OptionID, Line, Available bool, Shortfall}
```
**Behavior:** the offer lists, per witness target (and per prop in reach), the options `encounter.OptionsFor` returns; a target with none is not a candidate; a row with no candidates carries `ShortfallNoOption`. World clock: free; turn clock: costs the action (unchanged from `spendOnSocial` :332). The check is built from the option's approaches exactly as `authored` did (best approach by modifier; the untrained source applies in resolution, untouched). `Manager.Speak` refuses `ErrNoOption` before anything is appended; a stale client naming a hidden option gets the same refusal.
**Tests:** `speak_test.go` (new, from `intimidate_test.go` + `persuade_test.go` scenes): offer lists only targets with options and only options whose `requires` holds; `Speak` on the world clock spends nothing; on the turn clock spends the action; Bardic Inspiration pose window still fires on a `Speak` check (`socialoffer_test.go` scene carried over); `ErrNoOption` on the door when the offer withheld it. `afford_test.go`: the `Speak` row replaces the two rows, ordering preserved at the old Intimidate slot.

- [ ] Port the scenes; RED; implement; `go test ./...`; lint; gorelease (MINOR, `feat(session)!:`).
- [ ] Push; DRAFT.

**Verification:** `go test ./... -run 'Speak|Afford' -v 2>&1 | grep -c '=== RUN'` > 0 and no FAIL.

### Task A3: rpg-api — `Speak` handler, content migration

**Delivers:** O3 and O5 (server side); A0 consumed.
**Owner:** rpg-api, `internal/handlers/dnd5e/session/v1alpha1`, `content/`.
**Prerequisites:** A0 merged (gen/go); A1, A2 pushed and `go get`-pinned; `go mod tidy` after each pin.
**Files:** `speak.go` (new, from `persuade.go:40`); `intimidate.go`/`persuade.go` handlers return `codes.Unimplemented` with "use Speak"; `convert.go` (`answeredToProto` :2152 adds `option_id`; new `spokeToProto`); `afford.go` maps the `Speak` row; `content/reference-front-room.yaml` migrated from `intimidate:`/`persuade:`/`on: intimidated…` to `options:` with the same DCs and lines; every other `content/*.yaml` that authored social keys (grep `intimidate:`) migrated.
**Interfaces:** proto `Speak` ↔ `session.Manager.Speak`; `Spoke` ↔ encounter's beat; `SHORTFALL_REASON_NO_OPTION` ↔ `ShortfallNoOption`.
**Behavior:** one conversion point; the handler validates exactly-one target; no game logic.
**Tests:** `speak_test.go` (from `persuade_test.go`); `answered_convert_test.go` gains `option_id`; a content test that compiles every `content/*.yaml` (exists: find it by `content/` in `*_test.go`) stays green with the migrated files.

- [ ] Pin; tidy; port handler and tests; migrate content; `make lint test` (or the repo's `ci-check`, grepping the log, never `$?`).
- [ ] Push to a branch off `dev`; DRAFT (pseudo-versions).

**Verification:** `go build ./... && go test ./internal/handlers/... ./content/...`; `grep -rn 'intimidate:' content/` empty.

### Task A4: web — the Speak surface

**Delivers:** O3 (client side).
**Owner:** rpg-dnd5e-web (ui-ux charter).
**Prerequisites:** A0 merged (TS package bump); A3 running on a local stack.
**Files:** `src/components/session/combat-experience/verbRegistry.ts` (:170/:179 replaced by one `[Verb.SPEAK]` entry); `ActionDock.tsx` (:101–104, :135/:139); `src/api/useSessionSpeak.ts` (new; delete the intimidate/persuade hooks); `TargetSurface.tsx` / `OrganizedActionSurface.tsx`: choosing a target with options shows its lines as the second step; `combatBeat.ts`, `story.ts`, `presentation.ts`, `debugLogLine.ts`: render `Spoke` (line, skill, roll) and `Answered` with `option_id`.
**Behavior:** the dock shows Speak only when the Afford row has an available candidate; picking a target lists its available lines, each with the skill it rolls; a line with `available=false` is not shown (the offer already hid it). Story log: *"Standre: 'We're the troupe for the feast.' — Performance 17 vs DC 13."* then the creature's answer line.
**Tests:** registry test (one entry, seven lists check from #1104 still one); a `Spoke` beat renders line and check; dock hides Speak on `NO_OPTION`.

- [ ] Bump protos package; `npx tsc` only after `npm ci` (fresh worktree trap); vitest; push; DRAFT.

**Verification:** `npm run typecheck && npm test -- --run` green; local stack screenshot of the front room with the goblin's two lines offered.

### Task A5: studio — options editor

**Delivers:** U1.
**Owner:** rpg-dnd5e-web (ui-ux).
**Prerequisites:** A1 pushed (the v4 shape and goldens are the contract); A3 for round-trip through the engine.
**Files:** `src/concepts/world-building/roomDraft.ts` (:185–214 intimidate/persuade removed; `BINDING_KEYS` :893–903 gains `options`; parse :1087–1102 replaced); `monsterOrderEdits.ts` (option add/remove/edit beside `addMonsterAnswerEntry` :197); `propBindingEdits.ts` likewise; `SitePolicies.tsx` (`CreatureOrders` :1853 gains an Options section: id, line, approaches, requires, success rows, failure rows — rows reuse `AnswerEntryRow` :427); `PropOrders.tsx` same section; `answerTableShape.ts` (`validateAnswerTable` :142) validates option rows with the engine's sentences; `singleRoomDungeon.ts` encode/decode `options`; fixtures `singleRoomV4.golden.yaml`, `worldBuilderV4Site.*`.
**Behavior:** an option is added with an id and a line; approaches pick a skill and a DC; `requires` offers "fact" (free text id, matching the `until` editor) or "holds" (a placed prop id from the room); success and failure each hold answer rows. Emitted YAML compiles under A1 (`dungeonspec`) with no refusal. The read-never-offered intimidate/persuade fields are dropped from the draft.
**Tests:** `roomDraft.test.ts` round-trips a binding with two options and a prop with one; `SitePolicies.test.tsx` adds an option and a `requires: holds`; golden YAML updated; `EncounterStudioIntegration.test.tsx` authors the front room's goblin with options and the emitted document equals the fixture.

- [ ] Fixture first; RED; implement; vitest; push; DRAFT.

**Verification:** `npm test -- --run src/concepts/world-building src/concepts/encounter-studio`; paste the emitted YAML into `dungeonspec` compile (a Go test in A1 already carries the goblin fixture; A5's fixture must be byte-identical to it or the plan notes why).

### A-walk (integration deliverable)

On the local stack from pushed pins (`dev-env.sh up`, api worktree path, web `dev`): author the front room in the studio with options (A5), launch, and walk: persuade the goblin by a line, intimidate it, fail each, see the creature's deed drive its `time` row on the next tick, see the story log carry line and roll. Record on #569 as `- [ ]` boxes. Then merge inside-out: A1 (tag) → A2 (`go get` tag, tag) → A3 (`go get` both tags, merge to `dev`) → A4/A5.

---

## Wave B — Hearing

### Task B0: Protos — `Heard`

**Delivers:** H5 wire.
**Owner:** rpg-api-protos.
**Files:** `events.proto`: `EVENT_KIND_HEARD`, `Heard { listener, kind (string, sealed in the rulebook), position (cell), hops }`; `Answered` already carries the picked word, nothing else needed for H3.
**Verification:** as A0.

### Task B1: mind/behavior — sounds, `heard`, `fact`, `path`

**Delivers:** H2, H4, P1 (vocabulary); H1's report type.
**Owner:** rpg-toolkit `mind/behavior` (one module; `deed`, `stage` are packages in it).
**Prerequisites:** none.
**Files:** new package `mind/behavior/sound`: `const Channel = "sounds"`, `Sound{Kind string; Where spatial.Position; At int}`, `Land(store, hearers, Sound)` mirroring `stage.Land` (`stage.go:79`) but never blanking (a sound carries no figures). `table.go`: `When` (:289) gains `Heard *HeardWhen{Kind string; Within int}` and `Fact string`; `holds` (:618) reads them; `Facts` (:508) gains `Sounds []Sound` and `Known []string`; `Selector` (:374) gains `Path []spatial.Position`; `Answer.When` becomes `[]When` under R8 (a single mapping still parses as a one-element list in dungeonspec), with `When.Not *When` for the negative. `answer.go`: no new words.
**Behavior:** each `When` entry sets exactly one of `Enemy`/`Deed`/`Heard`/`Fact`/`Not`, refused otherwise; an entry is eligible only when every `When` in the list holds; `Not` holds when its inner condition does not; `heard` holds when any sound of that kind in `Facts.Sounds` has `Now - At <= Within`; `fact` holds when the id is in `Known`; a `Selector` with `Path` is legal only on `toward`.
**Tests:** `table_test.go`: `heard` within/outside the window; `fact` present/absent; an entry with two conditions in one `When` refused; a two-entry list holds only when both hold; `not: { enemy: seen }` holds with no enemy in sight; `sound/land_test.go`: three hearers each hold the sound, a non-hearer does not, the sound names no actor.

- [ ] RED; implement; `go test ./...`; gorelease MINOR (`feat(mind/behavior):`); push; DRAFT.

### Task B2: encounter — loudness, hops, hearers, `fact` on time, patrol

**Delivers:** H1, H3, H4, H5 (engine), P1 (execution), passive Perception on `SheetFacts` (S1's prerequisite).
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`.
**Prerequisites:** B1 pushed and pinned; A1 merged and its tag adopted (shares `answer.go`, `grammar.go`).
**Files:**
- New `noise.go`: `soundOf(deed) (kind string, dc int, ok bool)` — the content map (attack landed → `clang` 10; door forced → `crash` 8; damaging cast delivered → `boom` 5; any `say` → `voice` 10); `hearersOf(cell, dc) []MemberID`: region hops via a graph derived once per field from `Atlas.Doorways` (`atlas.go:418`) — `hops(a, b)` BFS over regions, a closed door on the crossing edge adds one; a member hears when `PassivePerception >= dc + 5*hops`.
- `sheets.go` `SheetFacts` (:35–75) gains `PassivePerception int` (zero = cannot hear anything above DC 0; the session must supply it, fail-closed note in the doc comment) and `MemberInput` carries it.
- `deed.go` `landAttack` (:104): after landing the deed on witnesses, land the `clang` sound on hearers; door force (`door.go` where the STR route resolves) lands `crash`; the cast delivery site that lands `attacked` under both-ways R5 lands `boom` when damage was delivered.
- `answer.go`: `say` on any row lands a `voice` sound at the speaker's cell (both clocks); `fact` legal on `time` rows (`validateEntry` :395), audience = `hearersOf(speaker cell, voice DC)` ∪ visual witnesses of the speaker; `validateWhen` (:439) accepts `heard`/`fact`; `toward` with `Path` resolves the next waypoint: the waypoint after the one the creature stands on, else the nearest; walks via the existing `Route`/`Direct` toward it.
- `facts.go` `factsFor` (:89): projects the creature's `sounds` channel holdings into `Facts.Sounds` and its fact holdings into `Facts.Known`.
- `worldtime.go`: unchanged logic; the `Heard` beat for players is appended where sounds land, one per listening player member, recipient-scoped (the existing per-recipient beat path used by detection beats).
- `dungeonspec`: `WhenSpec` (:1173) parses `heard: { kind, within }` and `fact`; `SelectorSpec` parses `path: [[c,r],…]`; `grammar.go` `wordLegality` allows `fact` on `time`; `laterWords` loses `patrol` (the word is `path`, not `patrol`; `patrol` stays refused with "write toward: { path }").
**Tests:** `noise_test.go`: an attack in room A is heard in B (one open door) by a passive 15 guard and not by a passive 10 one; a closed door adds a hop; the sound's holding carries no actor. `answer_test.go`: a `time` row with `fact` teaches hearers and not a far room; a `when: { fact }` row is eligible only for a creature that holds it; `path` walks waypoint to waypoint across three ticks and resumes at the nearest after a `Routed` displacement. `worldtime_test.go`: a `Heard` beat reaches the player whose passive beats the DC only. `dungeonspec` tests: parse and refusals (`heard` without `kind`, `path` on `attack`, `patrol` sentence). Goldens regenerated.

- [ ] RED; implement; `go test ./...`; lint; gorelease MINOR; push; DRAFT.

### Task B3: session — supply passive Perception, route `Heard`

**Owner:** rpg-toolkit `session`. **Prerequisites:** B2 pushed, pinned.
**Files:** where `SheetFacts` are built for monsters and characters (follow `SheetFacts` usages; `activate.go:48 ObserverPassivePerceptions` is a stale shelf — fill `SheetFacts.PassivePerception` from `monster.Monster.PassivePerception()` (`monster/monster.go:434`) and from the character sheet, and delete the unused shelf); beat routing already per recipient.
**Tests:** a scene where a monster hears across a door; a character with passive 14 receives `Heard` and one with 9 does not.

### Task B4: rpg-api — `Heard` convert, pins. **Task B5: web** — render `Heard` in story and debug ("metal clanging, two rooms north": direction from the listener's cell to the sound's); studio TablesPanel gains `heard`, `fact` (time), `path` rows in `answerVocabulary.ts` and `answerTableShape.ts`.

### B-walk

Two rooms, a door, a guard patrolling a square, a thug in the next room. Fight the thug; the guard hears the clang, walks to the door, sees the party, shouts, the fact lands on the far room's reserve `arrives: { fact: alarm-raised }`. Close the door first; nothing hears. Merge inside-out: B1 → B2 → B3 → B4 → B5.

---

## Wave C — Contact

### Task C1: encounter — passive Stealth at first contact

**Delivers:** S1.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter`. **Prerequisites:** B2 merged (passive Perception on `SheetFacts`); `SheetFacts` gains `Stealth int` (the mover's modifier, supplied by session as B3 did).
**Files:** `trigger.go` `classify` (:205) drop arm (:253–257, :266–273): the discarded `drops` become a check: for each dropping mover, roll d20 + `Stealth` through the shared dice (die owner = the mover) against the highest `PassivePerception` among the opposed watchers in that contact; on success the pass is suppressed for those watchers this refresh and a recipient-scoped `Snuck` beat (roll, dc, beaten) goes to the mover only — protos add it in a C0 task mirroring B0; on failure the contact proceeds as today (formation, surprise rules unchanged). A later refresh with the mover on a new cell re-rolls.
**Tests:** `holdingsscenes_test.go` scene: a rogue with +7 and a scripted 15 passes a passive 13 guard, no sighting lands, no fight forms; a scripted 2 fails and the fight forms with the existing surprise outcome; the beat reaches the rogue only.
**Close:** rpg-toolkit#1020 when merged. Session supplies `Stealth` (C2, small, same shape as B3).

---

## Wave D — The castle

### Task D1: content + route scenarios

**Delivers:** C1.
**Owner:** rpg-toolkit `rulebooks/dnd5e/encounter` (tests) and rpg-api `content/` (the file).
**Prerequisites:** A, B, C merged and tagged; R8 ruled (or the storm-the-keep row authored with two rows and the assertion for it marked pending in the test).
**Files:** `rpg-api/content/the-mad-king.yaml` (v2 dialect, the §2 file; stat blocks `guard`, `guard-captain`, `commoner`, `knight`, `mad-king` authored as templates per the stat-block initiative); `encounter/madking_test.go`: one test per §3 row, driving the compiled content with scripted dice through `Speak`, movement, attacks and ticks, asserting the facts in the journal (attributed, audience-scoped), the stance folds, the endings fired, and — for the kitchen-door rows — that no sighting landed. Zero castle names in non-test Go.
**Verification:** `go test ./ -run MadKing -v | grep -c '=== RUN'` equals the number of §3 rows; the content compile test in rpg-api is green.

### Task D2: studio endings (U2)

**Owner:** rpg-dnd5e-web (ui-ux). Port `src/author/EndingsPanel.tsx` into the studio document (`fixtures/studioDocument.ts:70–76` shows the slot); the predicate editor already exists for `until`. Test: author `{ down: king }` and `{ fact: throne-taken }`, emitted YAML compiles.

### D-walk (C2)

Kirk authors the castle in the studio, shares the document, launches, and walks each §3 route on the local stack. Findings go on #569; any architectural gap returns to the use case before a fix.

---

## Requirement coverage

| requirement / scenario | task(s) | proof |
|---|---|---|
| O1 options on monster + prop, both dialects | A1, A5 | `options_test.go` compile + refusals; studio round-trip fixture byte-equal to A1's |
| O2 `requires` fact / holds, actor-scoped | A1, A2 | `option_test.go` hide/show; `speak_test.go` offer lists only holding actors' options |
| O3 one Speak verb; offer and door agree | A0, A2, A3, A4 | `ErrNoOption` at the door; dock hides on `NO_OPTION`; A-walk |
| O4 deed is the skill | A1 | Intimidation lands `intimidated`, Religion lands nothing |
| O5 social keys retire; content migrates | A1, A3, A5 | `unknown_key_test.go` sentence; `grep intimidate: content/` empty; the studio offers only `time` as a trigger and refuses the retired keys by name (found by the A5 review) |
| H1 report kind/position/when; hop DC; passive | B1, B2, B3 | `noise_test.go` door hop cases; `land_test.go` no actor |
| H2 `heard` condition | B1, B2 | `table_test.go` window; `answer_test.go` eligibility |
| H3 `fact` on time rows, hearers | B2 | `answer_test.go` teaches hearers not the far room |
| H4 `when: { fact }` | B1, B2 | `table_test.go`; `answer_test.go` |
| H5 players hear | B2, B3, B4, B5 | `worldtime_test.go` recipient; session scene; B-walk |
| P1 patrol by path | B1, B2 | three-tick walk + resume-at-nearest |
| S1 contact Stealth | C1 | rogue passes / fails scene; #1020 closed |
| U1 studio options | A5 | integration test authors the goblin |
| U2 studio endings | D2 | emitted YAML compiles |
| C1 content + every §3 route | D1 | `MadKing` test count = §3 rows |
| C2 the walk | D-walk | checklist on #569 |
| §3 "Scout" row (guard without option turns nothing) | D1 | asserted: `guards-turned` absent after speaking to `gate-1` |
| Storm the keep yields to a sighting | B1, D1 | `MadKing` turn-the-garrison row asserts the walk stops when a loyalist is seen |

Deferrals (by the use case, not by this plan): allied, per-character stance, challenge, journal, tree, word-spreads mechanism, mind list.

## Provider / consumer seams

| provider | consumer | contract | availability | proof |
|---|---|---|---|---|
| A0 protos `Speak`/`Spoke`/`SpeakOption`/`NO_OPTION` (rpg-api-protos#391) | A3 api, A4 web | names and field types above; menu field is `speak_options`; `option_id` empty = time pick | merged first; gen/go tag | A3 convert tests; A4 registry test |
| A1 `encounter.Option`, `OptionsFor`, `Speak`, `TriggerHolds` | A2 session | actor-scoped `requires`; `ErrNoOption`; `ErrUnwitnessed` | pushed sha → pseudo-version | `speak_test.go` through the real encounter |
| A1 `dungeonspec` v4 `options` shape + goldens | A5 studio | YAML key names, refusal sentences | pushed sha (goldens are the contract) | A5 fixture byte-equal to A1's |
| A2 `Manager.Speak`, Afford `Speak` row | A3 api | one target kind; `ShortfallNoOption` ↔ `NO_OPTION` | pushed sha | A3 handler tests |
| A3 api on local stack | A4, A5 | wire | running stack | A-walk |
| B1 `sound.Sound`, `When.Heard/Fact/Not`, `[]When`, `Selector.Path`, `Facts.Sounds/Known` | B2 encounter | one condition per `When` entry; list = all hold (R8) | pushed sha | B2 tests compile against it |
| B2 `SheetFacts.PassivePerception`; `Heard` beat | B3 session; C1 | zero = hears nothing (fail closed) | merged tag for C1; pushed sha for B3 | B3 scene; C1 scene |
| B0 `Heard` | B4, B5 | kind string sealed by rulebook content | merged first | convert test; story render test |
| A1 and B2 both edit `answer.go`, `grammar.go` | — | B2 branches from A1's merged tag | ordering | B2's PR base sha ≥ A1 tag |
| D1 content | D1 tests, D-walk | the §2 file under both dialects | A–C tagged | compile test + `MadKing` tests |

## Plan check findings

- `requires` needed no edge-triggered path: it is read at offer and door time, so the seam probe's "`holds` does not fit the `onX` closures" conflict does not apply. Recorded; `TriggerHolds` is documented as read-only (never fired).
- `encounter.SheetFacts` has no perception field today and `session/activate.go:48` carries an unfilled `ObserverPassivePerceptions` shelf; B2/B3 fill the former and delete the latter rather than wiring a stale shape (build on the new places).
- `patrol` stays a refused word; the author writes `path`. The use case's table sketch says `path` already.
- A-walk needs A5 before A4 is useful to Kirk (he authors, then plays); the sequence lists them in parallel and the walk waits for both.
- No task touches `resolution`. (Superseded 2026-10-13 on the wave A walk: the untrained-disadvantage source was deleted in resolution v0.73.0 by the operator's ruling; session adopts it in its own slice. Wave B's checks roll plain.)
- R8 settled as the list form before any task started; no open architectural decision remains.

— cross-team agent, on behalf of KirkDiggler
