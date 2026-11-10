# Scout: ground for Gate S2 "the entrance remembers" (rpg-project#922)

Read-only. rpg-toolkit at `e4ac31f8` (main), rpg-api at `2253b33e`, rpg-dnd5e-web at `3be3c439`.
Paths below are relative to `rpg-toolkit/rulebooks/dnd5e/encounter/` unless they name a repo.
`dungeonspec` is a package INSIDE the encounter module (only `encounter/go.mod` exists; no dungeonspec go.mod).
Encounter pins `mind/behavior v0.22.0` and `mind/machine v0.2.0` (`go.mod:8-9`); latest behavior tag is `mind/behavior/v0.23.0`.

---

## 1. How authored behavior instances are compiled and owned today

**Source grammar (dungeonspec).**
- Root `behaviorDefinitions:` map → `SingleRoomSpec.BehaviorDefinitions` (`dungeonspec/single_room.go:49`). Each is a `BehaviorDefinitionSpec` (`dungeonspec/behaviors.go:76`): either `recipe:` (a native `machine.Recipe`, plus optional `table:`) or a sequence (`ref/parameters/end/body`).
- `BehaviorValue` keeps the authored value as raw JSON (`behaviors.go:26-73`); decoded with `RejectUnknownMembers` (`behaviors.go:119-124`).
- Per-actor `behaviors: [{id, use, items?, with?}]` → `BehaviorBindingSpec` (`behaviors.go:89`), plus `behaviorState`, `blockingStates`, `posts` on the monster binding or its faction (`roomBehaviorInput`, `behaviors.go:340-368`; faction fallback when the binding has none).
- `bindBehaviorSource` (`behaviors.go:159-316`) lowers bindings to `encounter.BehaviorInput{Instances []BoundBehavior, BodyInstance}`. A concrete recipe becomes `bound.Instance.Recipe` (`:185-188`). Posts lower to `encounter.Post{Door, Against, Pass FactID, Cells}` keyed by body state (`:285-307`).
- Each binding is pre-checked through the engine: `encounter.CheckBehaviors` (`behaviors.go:245`, `behavior_binding.go:66-81`). No runtime is built and nothing is rolled.
- Validation entry for the whole document: `behaviorSourceValues` (`behaviors.go:400-452`). YAML shape walk: `behaviorBindingsShape` (`:454-497`).

**Vocabulary.**
- Bind-time: `roomBehaviorVocabulary` (`behaviors.go:370-398`) gathers every fact the room mentions (`roomFacts`), `bell:<prop>` clocks and literal `clock:<name>` strings in definitions (`behaviorClockNames`, `:499-517`). It calls `encounter.BehaviorVocabulary` (`behavior_binding.go:28-51`).
- Runtime base: `machineVocabulary()` (`machine.go:29-43`) declares beats `time, turn, heard, deed, seen, barred, fact, clock:world, clock:fight` and facts `enemy`, `heard.kind`.
- Per recipe additions: `vocabularyFor(r)` (`machine.go:45-62`) adds named `deed:<word>`, `clock:<name>` and every `fact:<id>` guard via `addGuardFacts` (`machine.go:64-86`). A `fact:<id>` reader is `world.knowsFact(me.id, id)`, which is per MEMBER (`machine.go:71-76`). The env type is `*machineEnv{e, id MemberID, ...}` (`machine.go:88-96`).
- Generic machine: `machine.Vocabulary{Facts map[FactName]FactSpec, Beats map[Beat]BeatSpec}` (`mind/machine/vocabulary.go`). `Recipe.Compile(v) (*Program, error)` (`mind/machine/compile.go:59`). `Program.Step(b *Blob, on Beat, ev, env any, now uint64) (Stepped, error)` (`mind/machine/step.go:45`). `Program.Listens(b)` (`step.go:151`), `Program.DueAt` (`step.go:171`), `Program.NewBlob()` (`blob.go:30`).
- `machine.Blob{State, Bag, WakeAt, Anchor}` (`mind/machine/blob.go:21-26`) names no recipe; the host keeps the program. A `Guard` can read a bag bool with `flag:` (`mind/machine/guard.go:13-21`), and `Act.Set`/`Act.Clear` write the bag (`recipe.go:65-76`).

**Per-member runtime and persistence.**
- `behaviorRuntime{definition digest, input, composer *behavior.Composer, byID}` (`behavior.go:76-82`). `newBehaviorRuntime` (`behavior.go:84-175`) deep-copies input, refuses picks on any beat but `time/turn/barred` (`:105-117`), builds `behavior.NewComposer` or `behavior.LoadComposer` from saved `ComposerData` (`:162-169`), and hashes the canonical input as `definition` (`:170-174`).
- `useBehaviors(m, in, data, blocking)` (`behavior.go:212-234`) installs on a `memberRecord`. On load it uses `data.Instances` and refuses a changed digest with `ErrInvalidData` (`:222-224`).
- Persisted per member as `BehaviorData{Definition, Instances, BodyInstance, Progress behavior.ComposerData, SymbolsRecorded}` (`behavior.go:38-45`), on `MemberData.Behaviors` (`data.go:2101`) and reserve rows (`data.go:527`). Written by `toData()` (`behavior.go:195-210`). Reloaded at `data.go:3504` (members) and `data.go:3560` (reserve).
- Setup/Join install sites: `encounter.go:1077`, `encounter.go:2799`, `board.go:181`.

**What S2 mirrors (inferred).** The encounter-owned interaction needs no `behavior.Composer`: it has no table and must not `pick`. The smallest faithful mirror is `machine.Recipe` → `Compile(vocab)` → one `machine.Blob` per interaction, stepped with `Program.Step`. Persist the canonical definition, its digest and the `Blob` exactly as `BehaviorData` separates definition from progress (design R3: "original bound definition and persisted `machine.Blob` remain distinct"). Refuse any `Act.Pick` in an interaction recipe at the door, like `behavior.go:105-117` does for non-time beats.

---

## 2. How options and answers are authored and compiled

**Grammar.**
- `OptionSpec{id, say, check, requires *PredicateSpec, success, failure []AnswerSpec}` (`dungeonspec/spec.go:1154-1179`). Known keys are hand-listed in `optionKeys` (`spec.go:~1183`); unknown keys are refused by name.
- Creature options live on `RoomMonsterBinding.Options` (`dungeonspec/single_room.go:527`); prop options at `single_room.go:630`; faction options on `FactionSpec.Options`.
- **A creature's own options replace its faction's wholesale**: `inherited.optionsFor` (`dungeonspec/compile.go:1507-1513`).
- `PredicateSpec` (`dungeonspec/predicate.go:51-118`) is "exactly one form" with forms round/down/fact/stance/holds/left/not. `Form()` is at `predicate.go:136`; the hand-written `UnmarshalYAML` is at `predicate.go:163`. A multi-key mapping is not a predicate. The nested-object precedent is `stance: { between, is }`.
- Option `requires` grammar: `grammar.optionRequires` (`dungeonspec/grammar.go:555-588`) admits only `fact`, `holds` or `not` of one of them. The sentence lives in `optionRequiresForms` (`grammar.go:590`).
- Compile: `optionsOf` (`compile.go:1189-1203`) → `encounter.Option`; `predicateOf` (`compile.go:645-675`) → `encounter.Trigger`; `answerOf` (`compile.go:1115-`) → `encounter.Answer`.

**Engine types.**
- `encounter.Option{ID, Say, Check, Requires Trigger, Success, Failure []Answer}` (`option.go:61-100`).
- `Trigger` is a marker interface `isTrigger()` (`field.go:1213-1215`). Kinds: `TriggerReachedPosition` (`field.go:1222`), `MemberDown` (`:1250`), `External` (`:1264`), `ExitedHolding` (`:1296`), `Left` (`:1333`), `Round` (`:1363`), `Fact` (`:1382`), `Stance` (`:1400`), `TriggerHolds` (`option.go:116`), `TriggerNot` (`not.go:48`). **There is no `all`/`and` trigger.**
- Option validation: `validateOptions` (`option.go:720-790`). Its requires switch admits nil, `TriggerFact`, `TriggerHolds` or a `not` of them (`:749-773`); any other kind is refused. `validateReadNot` is at `not.go:130`. `field.validateOptionHolds` is at `option.go:815-833`.
- Offer and door: `OptionsFor` (`option.go:178`) and `PropOptionsFor` (`:211`) → `offered` (`:225-235`) → `requirementHolds(actor, requires)` (`option.go:241-262`). The door re-asks the same question in `optionOffered` (`option.go:644-652`) from `Speak` (`option.go:436`, called at `:513`). **Today an unmet requirement is absent from the menu**; Gate S1 (rpg-project#921, OPEN, not on main: no `Met/Unmet` in encounter) is changing this evaluation.
- Trigger persistence: `TriggerData` (`data.go:615-629`), `triggerDataFrom` (`data.go:633-663`), `triggerFromData` (`data.go:666-711`, unknown kind refused). `OptionData` lives at `data.go:1374-1381` and `optionsFromData` at `:1421-1443`.

**Answers.**
- `encounter.Answer` is an alias: `Answer = behavior.Answer` (`table.go:57`). The struct is in **mind/behavior**, a separate module: fields `Weight, Say, When, Fact, Flee, Hold, Attack, Toward, Dash, Away, Search, Rows, As, From, Params, Activity` (`mind/behavior/answer.go:35-~140`). Shape validation is `(*Answer).Validate` (`mind/behavior/answer.go:206`).
- Encounter-side vocabulary: `validateEntry` (`answer.go:607-662`) rejects `when` under a social verdict (`:630-633`) and `hold/attack/toward/...` under a verdict. Groups go through `validateGroupEntry` (`:664-692`) and words through `validateWhenWords` (`:694`).
- Persisted rows: `AnswerData` (`data.go:1504`), `answerDataFrom` (`data.go:1698`).
- Applying a verdict: `Speak` picks `AnswerSuccess/Failure` rows (`option.go:547-560`) → `e.answer` (`answer.go:180-257`). That function picks a row with `behavior.Pick` (`:202-212`), then `fact` teaches every witness (`:214-220`), `flee` (`:224-228`), the answered beat (`:230`), and `say` makes a voice (`:236-240`). Back in Speak: a beaten prop line lands `clock: bell:<prop>` (`option.go:563-567`), then `spendWorldAction` for a checked line (`:580-584`).
- **The `advance` application point**: right after `e.answer` returns, in `Speak` between `option.go:560` and `:563`. `answered.pick.Answer` is the chosen row. The same spot can also see `option.ID`, `beaten` and `in.Actor`.

**Inferred consequence.** `advance:` on an answer ROW means a new field on `behavior.Answer`. That is a `mind/behavior` change: its own PR, tag and repin, under the one-module-per-PR rule. It also touches `AnswerData`, the dungeonspec `AnswerSpec` (`spec.go:1301`) and `answerOf`. The alternative keeps it encounter-only: put `advance` on the `Option` per verdict (`OptionSpec`/`Option`/`OptionData`). That loses "weighted rows advance differently", which the castle does not need. The planner must pick one.

---

## 3. How a clock/beat lands and who listens

- Listener registry: `e.listeners machine.Listeners[MemberID]` (`encounter.go:274`), rebuilt in `rebuildWake` (`machine.go:245-269`) via `relistBehavior`. Due wakes use `e.due machine.Due[MemberID]`.
- Landing: `land(wakeBeat)` appends to `e.landed` (`wakebeats.go:179-181`). `landClock(name, faction, at)` (`wakebeats.go:266-272`) lands only when somebody `listened` (`:156-162`). `landFactBeat` targets the one learner (`wakebeats.go:222-228`).
- Matching: `matchLanded` (`wakebeats.go:279-306`) → `matchOne` (`:332-361`) → `react(id MemberID, ...)` (`wakebeats.go:~400`). `react` requires a `memberRecord` with `behaviors` on the default clock. **`matchLanded` runs only inside the world-time raise** (`worldtime.go:541`, `worldtime.go:610`). Landed beats wait in `EncounterData.Landed` (`data.go:~60`) until then.
- `stepMachine(id MemberID, m *memberRecord, round)` (`machine.go:331-354`) is a paid world opportunity: it spends clock budget and runs `runIntents`. **It cannot be reused for an encounter-owned instance**, because it is keyed by member, spends budget and selects movement.
- State beat: `appendStateBeat(id MemberID, stepped)` (`machine.go:367-377`) writes `BeatState` with `member` and `from/to/reason`, audienced by `audienceFor(subjectBeat, id)`. An interaction needs its own beat or payload, with no member and a story audience (R9: private inspection separate from NPC instances).

**Inferred design for S2.** Make the advance synchronous. In `Speak`, after `e.answer`, call `program.Step(&blob, beat, nil, interactionEnv, now)` directly and append an interaction state beat. Do not land it on `e.landed`, because that would only step at the next raise and the very next `OptionsFor` would read stale state. Key the interaction's beats as its own vocabulary, such as `claim:guest:failed` and `admit`. Do NOT add them to `machineVocabulary()`, or member recipes would compile against them. If interactions should later react to world beats (clocks, facts), they need their own registry, for example a `machine.Listeners[InteractionID]`, because `Listeners` is generic over `cmp.Ordered`. S2 does not need that.

---

## 4. The mad-king castle gate content (rpg-api `content/the-mad-king.yaml`)

The file is dialect `version: 5`, single room (`:126`). Its root keys are play, templates, bundles, factions, dispositions, intel, exits, endings, room and behaviorDefinitions (`:126-1917`).
- Gate guards `gate-1` (q-3,r7, posted on the crossing) and `gate-2`, plus `captain` (q-4,r8) (`:1385-1410`). **There is no "lieutenant"; the captain is the gate speaker.** gate-1 and gate-2 carry **no options**.
- gate-1 posts on `gate-wall-door` in states alert/challenged/posted, `against: entering`, `pass: troupe-admitted` (`:1526-1563`). gate-1 recipe `gate-1-policy` starts `initial: posted`, with fact transitions on `fact:guards-turned` and `fact:gate-cowed` (`:1918-~2060`). The door binding is `gate-wall-door: closed: true, opened: {fact: gate-opened}` (`:1864-1867`).
- Captain options (`:1574-1640`), trimmed:

```yaml
      captain:
        faction: garrison
        options:
          - id: troupe
            say: We're the troupe for tonight's feast.
            check:
              - ability: performance
                dc: 13
              - ability: deception
                dc: 15
            requires:
              fact: feast-tonight
            success:
              - say: Through to the hall, then. Don't dawdle.
                fact: troupe-admitted        # the post's pass
            failure:
              - say: Troupe? There's no— GUARDS!
                fact: party-recognized       # disposition → hostile (:315)
          - id: stand-aside        # intimidation 14; success 3:1 gate-cowed / party-recognized
          - id: turn               # persuasion 15, requires king-burned-village → guards-turned
          - id: hold-the-castle    # requires guards-turned
          - id: greet              # plain line, no check
```

**What the two-claim interaction needs authored (inferred):**
- A new root `interactions:` entry, e.g. `front-gate`, with a recipe. States would be `fresh → warned-guest | warned-entertainment → admitted | arrested`, with beats like `guest-failed`, `entertainment-failed` and `admitted`. A second failure from either warned state goes to `arrested`.
- A **guest claim** option, which does not exist yet (Persuasion per R8, open), beside the existing `troupe` as the **entertainment claim**. Its failure row must stop teaching `party-recognized`, which today turns the garrison hostile at once (`:315`). The warning line "What's going on here?" replaces it on the first failure.
- Requires per claim: "interaction in {fresh, warned-other}". Today's `requires: {fact: feast-tonight}` gate on troupe would need an AND (see risk 1).
- **Speaking to another gate guard** needs the claims on gate-1 and gate-2 too. Own options replace faction options wholesale (`compile.go:1507`). The choices are: duplicate the claims onto each guard binding, or put them on the `garrison` faction. The faction route also hands them to `hall-1` and every other garrison member that has no own options.

---

## 5. Studio round-trip

- Toolkit side (engine/dungeonspec): the golden/content tests are `dungeonspec/golden_test.go`, `dungeonspec/contentgolden_test.go` and `dungeonspec/behaviors_test.go`. The castle host test is `dungeonspec/castle_host_test.go`.
- **Web studio (the "studio loads what runs" law)** lives in rpg-dnd5e-web:
  - `src/concepts/world-building/behaviorSourceRoundTrip.test.ts`. The pattern is decode → edit (`setRegionLighting`) → `stringifyRoomDraft` → `parseRoomDocumentJson` → `encodeSingleRoomDungeon`, then `toEqual` per root key, then byte-idempotence (`:16-50`). Fixtures are in `src/concepts/world-building/fixtures/`.
  - `src/concepts/encounter-studio/servedCastle.test.ts` runs against fixture `fixtures/the-mad-king-served.yaml`, which is git-blob pinned. It includes "invented future field round-trips" (`:97`) and "an option edit keeps a carried field" (`:298-340`).
  - Unknown ROOT keys are carried on `scope.unread` (`world-building/singleRoomDungeon.ts:373-375`). A new root `interactions:` therefore survives untouched. Unknown answer-row keys are carried too (`answerTableShape.ts:468-476`), so `advance:` on a row survives.
  - **But `requires` is strictly typed and REFUSED otherwise**: `validateOptionRequires` (`answerTableShape.ts:1351-1380`) fails on a mapping with more than one key (`:1356`) and on any form besides `fact`, `holds` or `not` (`:1375`). The picker type `OptionRequires` (`predicateNot.ts:50-57`) and `requiresChoice`/`requiresId` (`predicateNot.ts:73-90`) assume fact or holds. **The castle with an interaction requires would fail to import in the studio** until the web learns the new form. S2 therefore needs a web PR plus a round-trip test in `servedCastle.test.ts` style, and the served fixture refreshed.

---

## 6. Persistence shape

- `EncounterData` has **no global schema version**. Each field is `omitempty`, and absence means "none", as with `Doors`, `Reserve`, `Landed` and `Pause` (`data.go:29-170`). The only version gate is `PerceptionSubjects` (`data.go:71-73`, `requirePerceptionSubjects` `observation_subject.go:123`), which is unrelated.
- Load is `LoadEncounter` (`data.go:2822`). The style is to **refuse** malformed data with `ErrInvalidData` (multi-`%w`), never reset it. A changed behavior definition digest is refused (`behavior.go:222-224`). Retired fields are kept as types so load can refuse them (`MachineData`, `machine.go:381-388`).
- **Proposed placement:** a top-level `Interactions []InteractionData `json:"interactions,omitempty"`` beside `Doors`/`Reserve` (`data.go:~84-130`). Each entry would hold `{ID, Definition digest, Recipe/binding (canonical), Blob machine.Blob}` and be written in `snapshot()` beside `Doors` (`data.go:~2480-2500`). Load validates definition and blob together: compile the recipe, check the blob's `State` names a recipe state, and refuse a digest mismatch. A run with no interactions writes no key, so earlier blobs stay byte-identical (R9: no retrofit).
- **Authored half:** the analog is doors. `DoorInput` sits in `FieldInput.Doors` (`field.go:688`) while live state is top level. Factions and dispositions also ride `FieldInput` (`field.go:743-752`). The choice matters for the session:
  - **In `FieldInput.Interactions`**: dungeonspec puts them in `Compiled.Field`, and session `launchWorld` (`session/launch.go:373-389`) passes only `Field:` to `NewEncounter`. It would carry them with **no session code change**, only a repin.
  - **In `SetupInput.Interactions`** (`field.go:1441`): the session must add `Interactions: dungeon.Interactions` in `launchWorld`, plus a new `Compiled.Interactions` (`dungeonspec/compile.go`, `Compiled` struct).
- Session save path: it stores `*encounter.EncounterData` opaquely through the fenced checkpoint (`session/checkpoint.go:93,107,164,184`). A new omitempty field needs **no schema bump**, only the encounter repin.

---

## 7. Risks and open design points

1. **`requires` is ONE predicate, with no AND.** The issue's example `{ interaction: front-gate, state: fresh }` is two keys, which both `PredicateSpec` ("exactly one form") and the studio (`answerTableShape.ts:1356`) refuse. Troupe also already needs `fact: feast-tonight`. Options: a single nested form `{ interaction: { id: front-gate, in: [fresh, warned-guest] } }` (stance precedent), with failed-claim tracking folded into states; or an `all:` predicate, which is a wider grammar change. The `not-failed: guest-claim` term duplicates what states already say. Recommend states only, so there is one source.
2. **Judged per run, not per speaker.** `requirementHolds(actor, ...)` (`option.go:241`) is speaker-scoped by doc law ("the SPEAKER's own holdings", R9 at `option.go:52-57`). The interaction term ignores `actor` by design (R2). Its doc and the Gate S1 evaluation must say so explicitly. The interaction id must also resolve at the door: `validateOptions` has no field access, so the check that the named interaction and state exist belongs in a field-level check like `validateOptionHolds` (`option.go:815`).
3. **The door's "refused as an ending" rules.** Adding a new `Trigger` kind touches the shared `triggerDataFrom`/`triggerFromData` switch (`data.go:633-711`) used by endings, dispositions and arrivals. The new kind must be refused by name in `until`, `arrives` and `when` (`grammar.predicate`, `validateEndingTriggers`) as `holds` is. A `not` of it on `requires` should be legal, but on a waited-for site it is a liveness question, since states can move both ways.
4. **Absent vs unmet (probe law and S1).** The issue says a failed claim is "gone from the menu". If Gate S1 (#921) changes `offered` to return unmet options flagged rather than filtered, S2 must still keep the failed claim ABSENT. The open question is whether that is a different evaluation outcome or simply that the claim's requirement is unmet. Coordinate with builder-gate-s1 before planning.
5. **`advance` placement crosses a module.** On an answer row it changes `mind/behavior.Answer` (`mind/behavior/answer.go:35`), which means a separate PR and tag before encounter. On the Option per verdict it stays encounter-only. The issue text says "Answer rows gain `advance:`".
6. **Synchronous step, not landed beats.** `matchLanded` runs only in the world-time raise (`worldtime.go:541,610`). The advance must step the interaction blob inside `Speak`, so the next `OptionsFor` sees it. That keeps the Speak R5 ordering: every refusal comes before the first write, and the advance comes after the answer.
7. **Naming collisions.** `Encounter.Interact`, `InteractInput` and `InteractOutput` already exist for the world-NPC verb (`interact.go:24,39,66`). `machine.Guard` is a guard tree (`mind/machine/guard.go:13`), `Post`/`posts` is the gate hold, and the issue's "its guards" means NPC members. Use `InteractionID`/`InteractionInput`/`InteractionData`. In the YAML, avoid a `guards:` key on an interaction; name it `members:` or `speakers:` if it is needed at all in S2.
8. **Castle content gap.** The castle has no guest claim and no lieutenant. Its troupe failure teaches `party-recognized`, which turns the garrison hostile and starts the fight. The S2 authoring must change the first failure to a warning (`:1592-1594`) and decide how gate-1 and gate-2 get the claims (`compile.go:1507` replace-not-merge).
