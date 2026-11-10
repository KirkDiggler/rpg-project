# Scout: mind/ inventory (rpg-toolkit, 2026-11-09)

Base: local `main` at e4ac31f8, which is 4 commits behind `origin/main` (ba6d40f2). None of those 4 commits touch `mind/` (`git log main..origin/main -- mind` is empty). Read-only survey; method-call counts are approximate (regex over files that import the package).

## 1. Module map

| module | path | latest tags (date) | src lines | test lines |
|---|---|---|---|---|
| mind/behavior | github.com/KirkDiggler/rpg-toolkit/mind/behavior | v0.20.0 (11-05), v0.21.0 (11-06), **v0.22.0 (11-08)** | 3954 root + deed 174 + sound 171 + stage 132 | 4796 root + sound 126 + stage 423, **deed 0** |
| mind/machine | .../mind/machine | v0.1.1, v0.1.2 (11-05), **v0.2.0 (11-08)**; v0.1.0 (10-31) | 1450 | 1450 |
| mind/perception | .../mind/perception | v0.2.0, v0.3.0, **v0.3.1 (10-30)** | 604 | 765 |

Consumer pins: encounter and session pin behavior v0.22.0, machine v0.2.0, perception v0.3.0 (not v0.3.1). resolution pins perception v0.3.0 only.

`mind/behavior/knowledge/` is a docs directory (README, decisions.md, runs/), not a Go package.

### behavior (root)
Owns, per `mind/behavior/README.md:12-21` and `doc.go`: composition of authored behavior instances with private progress (`Composer`), one ordered selection lane, and the weighted table evaluator (`Pick`, `Deal`). "It returns decisions; the caller gives those decisions their game meaning." Holds no map, clock, rules or behavioral policy of its own; names exactly one trigger, `KeyTime` (`doc.go`). It replaced the deleted four-judgment Mind ladder (rpg-project#465).
Entry points:
- `Pick(ctx, *PickInput)` `table.go:1654`: roll a table key with temperament, facts and a die. Returns the fired entry plus arithmetic; `ErrSilent` when nothing is eligible (except `KeyTime`, which holds).
- `Deal(ctx, *DealInput)` `table.go:1835`: deal one creature's temperament from a faction's spread.
- `Layer(base, over Table)` `table.go:199`: overlay authored orders on a default table.
- `NewComposer(*ComposerConfig)` `composer.go:70`: build from ordered `InstanceSpec{ID, Recipe, Table}` + `machine.Vocabulary`.
- `Composer.Observe` `composer.go:204`: data-only input, broadcast or addressed; refuses action requests with `ErrSelectionRequired`.
- `Composer.Decide` `composer.go:265`: preview all machines, roll only the first requesting instance's table.
- `Composer.ToData` / `LoadComposer` `composer_data.go:37,60`: persist progress plus configuration identity; refuse a changed config.
- `Composer.Inspect` / `StateOf` / `DelayWakes` `inspect.go:49,88,114`: detached per-instance state, listeners, due/wake; caller-chosen wake shift.
- `ExpandSequence` `sequence.go:86`: bind-time expansion of a `SequenceDefinition` over finite items into an ordinary `InstanceSpec`, with completion and source maps.
- `CheckParameters` `parameters.go:58`: validate producer-supplied scalars against `ParameterSpec`.
- `InstanceOutcome` `trace.go:31`: opt-in trace of selected/committed/skipped per instance on Observe/Decide.
- `ActivityCue` `activity.go:14`: opaque presentation cue carried with a spoken answer.
- Vocabulary types: `Answer`, `When`, `HeardWhen`, `Selector`, `Facts`, `Temper`/`TemperProfile`, `Name` (literal or `/pattern/`), `Search`.

### behavior/deed
"The payload vocabulary for the deeds channel: what an observer would say they saw somebody DO" (`deed/deed.go:4`). A deed lands through perception's Report door, one subject per actor and verb, and is never current (R9). Entry points: `Deed`, `Encode`/`Decode` (`deed.go:150,163`), `Subject(actor, verb)` `:75`, `FiledUnder(d)` `:105` (falls are also keyed by target, decision 2026-10-29), `Channel`, `VerbFell`.

### behavior/sound
"Where a noise meets the people who heard it" (`sound/sound.go:4`). A heard report is a kind, a place and a time, never an identity (rpg-project#569 R4). Entry points: `Land(*LandInput)` `:106`, `Held([]perception.Holding)` `:135` (decode held sounds), `Sound`, `Subject(kind)`, `Store` interface.

### behavior/stage
"Where a deed meets the people who saw it" (`stage/stage.go:4`). One function: `Land(*LandInput)` `:80` tells every witness what they saw in their own terms. Formerly held Aim/Recall/Step for the retired ladder.

### machine
"A state machine that listens: a state is the set of triggers it listens to" (`machine/doc.go`). Recipe is data: on a beat, when a guard holds, do acts, to a state. Rulebook-agnostic, bus-free, imports only core, no goroutines or timers. Persists only the `Blob`; host rebuilds indexes from `Listens`/`DueAt`.
Entry points:
- `Recipe.Compile(Vocabulary) (*Program, error)` `compile.go:59`.
- `Program.NewBlob` `blob.go:30`; `Program.Step(b, beat, ev, env, now)` `step.go:45`: first applying transition fires; returns `Stepped`.
- `Program.TraceStep` `trace.go:71`: same evaluator, with per-transition and per-guard verdicts and observed scalars.
- `Program.Listens(b)` `step.go:151`, `Program.DueAt(b, now)` `step.go:171`.
- `Due[K]` (Arm/Disarm/Pop/Len, `due.go`) and `Listeners[K]` (Set/Drop/For, `listeners.go`): host-side wake indexes.
- Guards are three-valued: `Applies`/`DoesNotApply`/`Depends` (`guard.go:70`); leaves `FactLeaf`, `ScopeLeaf`, `RoundsLeaf`.

### perception
"What an observer holds: channel-sourced testimony that may be false and may be stale" (`perception/doc.go`). Store is play/intel; this supplies the loop. Two write verbs: `Observe(Pass)` `perception.go:195` (a complete statement about one channel, enables fading) and `Report(ReportInput)` `:283` (discrete testimony, sustains nothing). Reads: `Held(observer)` `:332`, `On(observer, subject)` `:354`, `Holding.CurrentOn` `:133`. Persistence `ToData`/`Load` (`data.go`). `Qualify(channel, subject)` `:80` builds channel-qualified subjects. Contract: `docs/ideas/mind/perception/design.md` R1–R11.

## 2. Who consumes what

Importing files outside `mind/` (non-worktree): encounter 98, session 24, dungeonspec 11, castletest 4, resolution 2, cmd/freeroam-workbench 1. Nothing outside `rulebooks/dnd5e` imports any mind module.

| symbol | consumers (non-test unless marked) | rough sites |
|---|---|---|
| behavior.Pick / PickInput / PickOutput | encounter (+ stage test) | 4 / 2 / 1 |
| behavior.Deal / DealInput | encounter | 1 / 1 |
| behavior.Layer | encounter | 1 |
| behavior.NewComposer, ComposerConfig | encounter (+test) | 1-2 |
| Composer.Observe / Decide / Inspect / StateOf / DelayWakes | encounter | 2 / 1 / 2 / 4 / 1 |
| behavior.LoadComposer, ComposerData | encounter | 1 / 3 |
| behavior.InstanceOutcome | encounter | 2 |
| behavior.InstanceSpec | encounter, freeroam-workbench, castletest test, session test | ~10 |
| behavior.ExpandSequence, SequenceDefinition, ParameterSpec, SequenceEnd/Repeat/Stop | dungeonspec (`behaviors.go:223-231`); encounter tests | 1 each |
| behavior.Name | encounter, dungeonspec (+tests) | ~37 |
| behavior.Selector*, Enemy*, Threat*, Heading* words | encounter, dungeonspec | 1-5 each |
| behavior.ActivityCue | dungeonspec; encounter/session tests | 2 + tests |
| behavior.ErrSilent | encounter | 4 |
| deed.Decode / FiledUnder / Deed / Channel | encounter, stage; castletest + session tests | 1-5 each |
| sound.Land / Held / Sound | encounter | 2 / 2 / 2 |
| stage.Land | encounter | 1 |
| machine.Recipe / State / Transition / Act / Guard | encounter, dungeonspec, freeroam-workbench, behavior (+heavy test use) | Recipe 13 enc + 6 spec |
| machine.Vocabulary / BeatSpec / FactSpec / Beat | encounter, dungeonspec, behavior | Beat 24 in encounter |
| machine.Due (Arm, Disarm, Pop) | encounter (`worldtime.go:560` Pop) | 2 |
| machine.Listeners (Set, For) | encounter (`machine.go:204,218`, `wakebeats.go`) | 3 + 4 For |
| Program.Step | encounter tests, freeroam-workbench; behavior composer | 14 test |
| Program.Listens / DueAt / TraceStep / NewBlob | **behavior composer only** | 1 each |
| machine.GuardTrace / TransitionTrace | encounter | 1 each |
| perception.Sight / Holding / Presence / CurrentOn | encounter, session, resolution, dungeonspec tests | very heavy (Sight 38 enc, Holding 21 session) |
| perception.Observe / Report / Pass | encounter (+ sound, stage) | 2 / 3 / 3 |
| perception.On | resolution only | 1 |
| perception.Load / New / Data | encounter | 1 each |

Exported API with **no consumer outside its own module** (candidate unwired surface; errors listed separately):
- behavior: `CheckParameters` (+ Input/Output) used only inside `ExpandSequence`; `SequenceSource`, `ExpandSequenceOutput` type names (fields are read); `InspectComposerOutput`, `InstanceStateOutput`, `InstanceStatus`, `InstanceData`, `ObserveOutput`, `DelayWakesOutput`, `DealOutput` (named types only, values used); `AsWords`, `SearchHeadings`; `TraceNotAddressed`, `TraceSleeping` constants; `VerbAttack`, `VerbFled`, `VerbIntimidate`, `VerbPersuade` (behavior copies; encounter uses `deed.VerbFell`); `Die` interface (inferred satisfied structurally).
- sound: `Channel`, `Subject`, `Store` not referenced outside.
- stage: `Store` not referenced outside.
- machine: `Listeners.Drop`; `StepTrace`, `Verdict` type and `Applies`/`DoesNotApply`/`Depends` constants; `Op` type name. `Program.Listens`, `DueAt`, `TraceStep`, `NewBlob` reach encounter only through the composer (inferred: encounter builds its own indexes from composer `Inspect` output).
- perception: `Reach` interface (inferred: implemented, not named).
- Errors never matched outside their module: behavior `ErrBadComposition`, `ErrSelectionRequired`, `ErrUnknownInstance`, `ErrBadComposerData`, `ErrBadSequence`, `ErrBadParameter`, `ErrBadMix`, `ErrNilInput`; all machine errors except `ErrUnknownBeat` (used only by behavior); all perception errors; `deed.ErrNotADeed`; sound and stage errors.

### Run records
- `mind/behavior/knowledge/runs/2026-11-03-nilnil-2220-silence-sentinel.md`: `Pick` silence became wrapped `ErrSilent` instead of `(nil, nil)` (toolkit#2220).
- `.../2026-11-05-authored-sequences-s2.md`: `ExpandSequence` shipped with Patrol journey tests and mutants (rpg-project#871, toolkit#2284).
- `.../2026-11-06-composer-consumer-queries.md`: `Inspect`, `StateOf`, `DelayWakes`, timed-input admission respecting future wakes (rpg-project#866).
- `.../2026-11-08-composer-inspection.md`: opt-in `InstanceOutcome` traces on Observe/Decide (rpg-project#899, PR #902 R2–R3).
- `mind/machine/knowledge/runs/2026-11-08-machine-inspection.md`: `TraceStep` sharing the real evaluator; trace cost ~575 ns vs ~320 ns ordinary per tiny step (#898/#899).
- `mind/behavior/knowledge/decisions.md`: three decisions, all 2026-10-29: `When.Who` + `HeldDeed.Target` for falls; falls filed by who fell; selector `ally`. Findings/Friction/Open are empty.

## 3. tools/selectables

Module `github.com/KirkDiggler/rpg-toolkit/tools/selectables`, latest tag v0.1.4, last touched 2026-10-30 (go 1.27 chore). 1728 src lines. README: "Universal weighted random selection system ... grabbag functionality that works with any content type through Go generics", aimed at monster, treasure and encounter-activity generation.

Public shape: `SelectionTable[T comparable]` interface (`interfaces.go:14`) with `Add(item, weight)`, `AddTable` (nested tables), `Select`, `SelectMany`, `SelectUnique`, `SelectVariable(diceExpr)`, `GetItems`, `IsEmpty`, `Size`. Built by `NewBasicTable[T](BasicTableConfig)`. `SelectionContext` is a string-keyed `interface{}` bag plus a `dice.Roller`. Optional event-bus publishing of nine topic events (`topics.go`).

Consumers: only `tools/environments` (`selectables_tables.go`, `room_builder.go:515`, `selectables_types.go:61`). `tools/spawn` and `rulebooks/dnd5e` list it as `// indirect` in go.mod only. No mind module or encounter code uses it.

Overlap with a "weighted pick with eligibility" primitive: selectables has the weighted roll and dice plumbing, but **no eligibility at all**. Context-based weighting is unimplemented: `getEffectiveWeights` returns base weights with the comment "Apply context modifications (in future iterations)" (`basic_table.go:656-657`), and `doc.go:47-49` advertises an `AddConditional` method that does not exist. It also has none of what `behavior.Pick` returns as evidence (candidate list, row verdicts, arithmetic), weights live in a Go map (selection order across equal dice is map-iteration dependent, inferred from `basic_table.go:262` summing over a map), and it couples to the event bus. A shared primitive would need eligibility, deterministic order and explanation, which is what the behavior-selection PR adds inside `mind/behavior/selection`. Reusing selectables would mean rewriting its core; overlap is in name and intent only.

## 4. Other behavior branches

**`.worktrees/behavior-bindings` (feat/871-authored-sequences)** at eba23e1d, last commit 2026-11-05.
- 3 commits over local main: expand authored sequences, Patrol journey tests, docs and run record. Diff adds `sequence.go`, `sequence_template.go`, `sequence.md`, tests and three testdata JSON fixtures (+1217 lines).
- That work is already on main: PR toolkit#2284 MERGED 2026-11-06 (squash), and `mind/behavior/sequence.go` exists on main.
- Verdict: **merged and stale**. The branch commits differ only because of the squash. No overlap with behavior-selection beyond touching `doc.go`/README. Safe cleanup candidate (operator's call).

**`.worktrees/admission-discovery`** detached at e4ac31f8 (= local main), 2026-11-09.
- No commits over main, empty `git status`.
- Contains only an ignored `.claude/progress.json` + `tests.json` describing a different feature: branch `feat/829-static-obstacle-reveal`, PR #830, "ready_for_review". These look like leftovers from an earlier encounter task, unrelated to behavior admission (inferred).
- Verdict: **empty discovery worktree**, nothing to inherit. Its name suggests wake-admission investigation, but no artifacts exist.

**Also present, not in the brief:**
- `.worktrees/say-activity-behavior` (feat/say-activity-behavior) at 9d157715, 2026-10-30. PR toolkit#2206 MERGED. Stale.
- `.worktrees/review-behavior-selection` detached at aab48ec6, the same head as the open PR.

**behavior-selection note:** the branch is cut from `origin/main`, so `diff --stat main...HEAD` against local main shows 101 files. Against `origin/main` it is 14 files (+1301/-59): `mind/behavior/selection/{selection,choose,doc}.go` + tests, `table.go` refactor, integration test, `docs/ideas/behavior-selection/plan.md`, run record dated 2026-11-10.

## 5. Gaps noticed

- **`mind/behavior/deed` has no tests** (174 src lines, 0 test files). The run records note this ("deed has no tests"). `Encode`/`Decode`/`FiledUnder` are covered only indirectly through stage and encounter tests.
- **README module table omits `sound`.** `mind/behavior/README.md:12-21` lists `deed` and `stage` but not `sound`, although `doc.go` describes it.
- **Consumers pin perception v0.3.0, not v0.3.1.** That holds for encounter, session and resolution go.mod. Whether v0.3.1 matters is not verified.
- **Verb constants are duplicated.** `behavior` exports `VerbAttack/Fell/Fled/Intimidate/Persuade` (`table.go:80-100`), and `deed` exports `VerbFell`. Encounter uses `deed.VerbFell`, and behavior's verb constants have no outside users.
- **Two weighted-selection mechanisms exist in the repo.** `behavior.Pick`/`Deal` sit in `mind/behavior`, and `tools/selectables` is separate. The open PR consolidates Pick and Deal onto one primitive but leaves selectables untouched.
- **`tools/selectables` promises more than it does.** Its doc promises conditional selection (`AddConditional`, `doc.go:49`) and context weighting, and neither is implemented.
- **`tools/selectables/events_test_simple.go` is compiled into the production package.** It lacks the `_test.go` suffix but imports `testing` and testify. `test_helpers.go` likewise ships an exported `TestRoller`.
- **Most sentinel errors are never matched outside their module.** The exception is behavior's `ErrSilent`. Composer errors (`ErrSelectionRequired`, `ErrUnknownInstance`) are documented as signals for callers, but encounter never branches on them (inferred: only logged/wrapped).
- **`decisions.md` has stopped recording decisions.** It records only 2026-10-29 entries. The composer, inspection and sequence decisions from 11-05 to 11-08 live only in run records and rpg-project PRs, not in the module's decisions ledger.
- **No TODO/FIXME/XXX/HACK markers anywhere in `mind/**/*.go`.**
