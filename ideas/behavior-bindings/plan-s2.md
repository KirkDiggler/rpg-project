# NPC behavior S2 implementation plan

**Goal:** [#871](https://github.com/KirkDiggler/rpg-project/issues/871): expand one authored Patrol body over two and three supplied items, then prove its behavior through real machines, the composer and weighted selection.

**Authority:** [R1–R3](design.md), [accepted example](README.md), [R3 selection](https://github.com/KirkDiggler/rpg-project/pull/868#issuecomment-6670342428). This plan translates the agreed producer-side expansion into a bounded component contract; interface spellings below are implementation choices, not additional operator rulings.

**Owner / baseline:** monster-ai; toolkit `mind/behavior`, at `d05c2fcd317e5bf979b62f4419a2b4595dd9697c`. Project record starts from `7180465`. No other Go module changes. No new dependency or consumer pin is required: behavior already uses machine v0.1.1 and core v0.12.1.

**Finish line for this execution:** source examples, production expander, deterministic joined tests, full module race/lint/hooks, assertion-level mutation controls, published PR and one independent review. Merge/tag/closure still require their existing explicit authorization and gates. No live stack is needed for this component-only checkpoint.

**Execution receipt, 2026-11-06:** [toolkit #2284](https://github.com/KirkDiggler/rpg-toolkit/pull/2284), head `eba23e1dbe6ec4ff93e82c5f2d8af55392f908dc`, implements this component plan. [Independent review](https://github.com/KirkDiggler/rpg-toolkit/pull/2284#pullrequestreview-5793883425): no Critical, Important or Minor findings; exact-head applicable CI is green. [Journal evidence and limits](../../docs/journal/mind-machine/2026-11-06-can-one-authored-body-drive-two-patrols.md). No merge/release or live adoption is claimed.

## Scope and concrete producer contract

This tool is separate from `Composer` execution. `mind/behavior` supplies a reusable producer utility because it combines a machine recipe with that module's table. It knows no route geometry, content catalog, actor roster or studio document. Existing content/bundle producers remain responsible for catalog lookup, explicit defaults, domain validation and lowering authored coordinates. They can eventually call this utility rather than replace their authoring machinery.

The component source uses **the existing machine-recipe and behavior-table JSON shapes**, with four whole-value references in a repeated body. This is not a new studio/dungeonspec field or a second client interpreter. Site/editor integration remains on #859; this slice must not claim it from Go tests.

### Public API

New file `mind/behavior/sequence.go`:

```go
type SequenceEnd string
const (
    SequenceRepeat SequenceEnd = "repeat"
    SequenceStop SequenceEnd = "stop"
)

type SequenceDefinition struct {
    Ref core.Ref `json:"ref"`
    Parameters []ParameterSpec `json:"parameters,omitzero"`
    End SequenceEnd `json:"end"`
    Body jsontext.Value `json:"body"`
}
type ExpandSequenceInput struct {
    ID string
    Definition SequenceDefinition
    Items []jsontext.Value
    Parameters map[string]any
    Vocabulary machine.Vocabulary
}
type SequenceSource struct {
    Item int
    Name string
}
type ExpandSequenceOutput struct {
    Instance InstanceSpec
    Vocabulary machine.Vocabulary
    Completions map[AnswerKey]machine.Beat
    StateSources map[string]SequenceSource
    TableSources map[AnswerKey]SequenceSource
}
func ExpandSequence(*ExpandSequenceInput) (*ExpandSequenceOutput, error)
```

`ErrBadSequence` wraps materialization refusals; nil input reuses `ErrNilInput`. Failures return no partial output and leave supplied source, values and vocabulary unchanged. The definition ref must parse through `core.ParseString`. `End` is required: no implicit repeat/stop. Items are an explicit nonempty finite list of valid JSON values; domain suitability belongs to their producer. Named scalar parameters go through existing `CheckParameters`, with its exact kinds/bounds/requiredness and no new defaults/coercion.

`Body` is a JSON object with exactly `recipe` and `table`. `recipe` uses ordinary `machine.Recipe` fields except that its ref is omitted: `SequenceDefinition.Ref` owns that identity. Its `initial`, `states` and the table's keys are authored local names. Unknown fields and extra body refs are refused, not ignored.

Four exact, single-member reference objects:

| Source value | Expansion |
|---|---|
| `{"$item": true}` | The current item's whole JSON value |
| `{"$param": "name"}` | A declared, supplied scalar value; no implicit default |
| `{"$next": true}` | The next item's initial state; last resolves to first or a compiler-owned terminal according to `End` |
| `{"$done": "table-key"}` | An item/attachment-qualified completion beat for that local table |

Ordinary strings such as `"$item"` are literal text. Inserted values are copied, never recursively interpreted as references. Unknown/malformed reference objects, including mixed reference/literal members, are refused. Structural `recipe`, `states` and `table` mappings are literal source, not replaceable program fragments. Initial-state scalar parameters are allowed, but next/completion references cannot determine the initial state.

Every emitted state, table and declared bag key is item-qualified. State destinations, picks, bag `Set`/`Clear` and nested guard `Flag` references are rewritten only from declared local names; `$next` is the explicit inter-item edge. External fact and ordinary beat names are not rewritten. The compiler-owned terminal has no authored source-map entry. The machine's anchor/wake remain instance-wide; this adds no per-step clocks or automatic resets.

Generated IDs use stable escaped local names plus item ordinal; completion beats additionally include the attachment ID. Refuse collision with a supplied vocabulary beat rather than replace its semantics. Output vocabulary is detached and includes the generated data-only completion beats. `Completions` maps selected generated table keys to those beats; the caller still delivers actual completion to `DecideOutput.Instance`. An inactive step's known completion does not match its current state's transition. This is not an async job-token/deduplication protocol across repeated cycles.

Validate the complete emitted `InstanceSpec` with the existing composer constructor before returning. No dice, fact-reader callback, world action, input delivery or progress advancement occurs during expansion. One source/parameters/binding produces deterministic materialized identity.

## Source and behavioral fixture

`testdata/sequence-patrol.json` declares one local `walk` state: on `time`, pick `move`; on `{"$done":"move"}`, transition to `{"$next":true}`. The `move` table contains one weight-1 `Toward.At` row taking `{"$item":true}`. The definition repeats.

West items: canonical positions `(0,0)` and `(4,0)`. Court items: `(10,0)`, `(12,0)`, `(14,0)`. Both local attachment IDs are `duty`, in separate composers. Expected selections, with actual completion between them: West `0,4,0`; Court `10,12,14,10`. Every singleton selection consumes exactly a d100 face 1. Missing movement budget consumes no die.

The investigation is separate authored fixture data, not a Patrol builder: qualifying clang activates it; it has precedence and two equal rows targeting `(8,1)` / `(8,-1)`, exact d200 faces 1 and 101; at two elapsed rounds it yields. Patrol must retain pending `(4,0)`. Caller-provided facts/time/completion are the test's inputs, not fake runtime orchestration.

A second definition repeats supplied spoken lines (`alpha`, `beta`) through the same primitive, with stop-at-end. It demonstrates different body content, not a definition-name branch.

## Tasks

### Task 1: Production finite expansion and refusals

**Delivers:** R1/R3 and the producer half of R2.
**Files:** create `mind/behavior/sequence.go`, `sequence_template.go`, `sequence_test.go`, `testdata/sequence-patrol.json`, `testdata/sequence-lines.json`.
**Interfaces:** produce exactly the public API above; consume `CheckParameters`, `machine.Recipe`, `Table`, `NewComposer`, `core.ParseString`, standard JSON v2/raw values.
**Behavior:** resolve finite source references; namespace local progress/table identities; preserve facts/beats; emit completion/source mappings; validate the actual result without executing it.
**Tests:** `TestSequenceSuite` methods:
- `TestExpandsDifferentLengthsFromOneDefinition`: two and three materializations feed real composer selections; source maps identify item/local name; no source rewriting.
- `TestParametersUseExistingMechanicalChecks`: integer bounds, wrong Go scalar kind, unknown/missing values; referenced optional absence is a binding error rather than an invented zero.
- `TestRefusesInvalidSourceWithoutMutation`: nil/empty ID/ref/end/items, malformed JSON, unknown fields, malformed references, unknown local state/table/bag references, invalid rows, unknown beats and completion-name collision.
- `TestInsertedValuesAndStringsStayLiteral`: no recursive interpretation or embedded/string substitution.
- `TestOutputsAreDetachedAndDeterministic`: source/input/vocabulary unchanged; changing one result cannot alter another binding; equal inputs yield equal emitted instance data.
- `TestLocalBagReferencesAreScoped`: `Set`, `Clear` and nested `Flag` follow the correct item's declared keys rather than alias another item's flags.

- [x] Write API/fixture tests first; run and record the expected initial missing-API compilation failure separately from behavioral regression evidence.
- [x] Implement the source resolver and expansion in the named files; no changes to existing composer/machine execution.
- [x] Exercise every named refusal and ownership case; fix assertion-level failures before broadening scope.
- [x] Run full module race tests and lint, then commit the working provider checkpoint and open its draft immediately.

**Verification:** from `mind/behavior`, `go test -count=1 -run '^TestSequenceSuite$' .`, then `go test -race -count=1 ./...` and `golangci-lint run ./...`. Pass means all assertions succeed; unrun checks are not completion evidence.

### Task 2: Joined Patrol paths and non-Patrol control

**Delivers:** every behavioral verification box in #871, using Task 1's production output.
**Files:** create `mind/behavior/sequence_patrol_test.go`, `testdata/sequence-investigation.json`; extend `sequence_test.go` only if joined evidence finds a materialization defect.
**Interfaces:** Task 1 output → real `ComposerConfig`, `NewComposer`, `Decide`, addressed `Observe`, `ToData`, `LoadComposer`. Existing exact scripted dice may be reused; no helper generates recipes/tables.
**Tests:** `TestSequencePatrolSuite` methods:
- `TestTwoAndThreeLegLoops`: literal target traces `0,4,0` and `10,12,14,10`, d100 face 1 each.
- `TestSelectionAndCompletionAreDifferent`: selection retains the current leg; addressed West completion leaves Court unchanged; unknown attachment errors; duplicate old-step completion cannot advance the current different step.
- `TestInvestigationResumesTheUnfinishedLeg`: real authored investigation wins with exact d200 faces 1/101, then the expanded patrol requests `(4,0)` without reset; Court receives no fabricated shared knowledge.
- `TestNoBudgetPreservesProgressAndDice`: no candidate produces hold, no roll and unchanged Patrol continuation.
- `TestReloadKeepsOriginalBindingsAndProgress`: JSON-round-trip original instances and progress, use saved vocabulary, no re-expansion; different actors resume their pending targets; changed materialized configuration is refused.
- `TestAnotherBodyUsesTheSameProducer`: lines `alpha`, `beta` followed by a terminal no-pick; no name-based factory or implicit completion on selection.

- [x] Write each batch against the real producer and inspect any failure before adjusting production code.
- [x] Run the named suite with verbose output; log readable actor/target/completion traces.
- [x] Run the full module race suite and lint; commit the joined evidence.

**Verification:** `go test -count=1 -v -run '^TestSequencePatrolSuite$' .`; `go test -race -count=1 ./...`; `golangci-lint run ./...` from the owning module.

### Task 3: Regression controls, documentation and review-ready publication

**Files:** update `mind/behavior/README.md` and `doc.go`; create `mind/behavior/knowledge/runs/2026-11-05-authored-sequences-s2.md`; add the concrete example/receipt to project #868 and #871. Do not change ADR-0049's accepted scope or claim studio adoption.
**Evidence:** mutation controls must cause assertion failures, be restored, and leave final production hashes unchanged. Candidate controls: resolve `$next` to the current initial state (normal progression fails), skip addressed observation commit (completion/progress fails), or interpret inserted marker-looking text again (literal-value protection fails). Record exactly which controls ran.

- [x] Run and restore at least two meaningful assertion-level mutation controls, then run the green full suite again.
- [x] Document source → binding → expanded output and caller completion mapping, ownership/refusals and limits.
- [x] Run full race/lint, `git diff --check`, module-isolation and actual scoped `.githooks/pre-commit` (not the unrelated root Make target).
- [x] Publish scope and actual evidence in the PR/issue; obtain one independent review under the owning project policy, address substantive findings with regression tests, and leave truthful readiness.

**Verification:** owning-module commands above; from toolkit root, `PRE_COMMIT_USE_CACHE=false` is not an established option—use the actual hook's `PRE_COMMIT_CACHE=false .githooks/pre-commit`; inspect resulting dependency/file diff. No `go.work`, `replace`, hand tag, force push or automatic merge.

## Requirement coverage

| Requirement | Task | Concrete proof |
|---|---|---|
| One data definition, differing finite inputs | 1, 2 | One JSON body; two-/three-item expansions and literal target traces |
| Independent addressed progress | 2 | West observation cannot change Court; selection alone does not advance |
| Interrupt/resume and no-budget hold | 2 | d200 1/101 investigation, pending `(4,0)` resumption, zero budget/roll assertions |
| Save/reload without rebinding | 2 | Stored original instances + progress; no expansion call or roll on load; mismatch refusal |
| No Patrol-specific builder | 1, 2 | Same production API runs motion and spoken-line definitions |
| Invalid source/values and ownership | 1 | Named refusals, nil output, unchanged inputs and detached returned data |
| Evidence suitable for review | 3 | Restored mutations, full module gates, readable traces and independent verdict |

## Provider/consumer seams

| Provider | Consumer | Contract / availability | Proof |
|---|---|---|---|
| Existing scalar checker | Sequence expansion | `CheckParameters` already shipped; no schema/default duplication | Bounds/kind/requiredness cases |
| New expander | Existing machine/composer | `InstanceSpec{ID,Recipe,Table}` plus a cloned vocabulary; no runtime extension | Real construction/selection/observe suites |
| New completion mapping | Caller inputs to existing Observe | Generated table key → beat, addressed by returned instance; actual completion remains caller-owned | Inactive-step and wrong-attachment cases |
| Existing ToData/LoadComposer | Joined reload fixture | Original materialized instances plus private blobs and original vocabulary | JSON reload/mismatch tests |
| Existing content/studio | Later producer adapter | Not changed by this component slice; no new dungeon field is accepted and ignored | Explicit scope, no claim of editor/game-path verification |

Tasks run sequentially in one worktree. No new provider module, dependency pin, proto, API event or deployment is introduced. The generic helper provides value expansion, not world validation or source catalog/default resolution. Field spellings, scoping and the four references are the concrete implementation choices documented here; no operator decision is fabricated from them.

**Plan check:** all #871 component requirements map to tests above; producer output matches the shipped composer contract; parameter checking is reused; completion delivery and configuration persistence are explicit; source/world/Studio adoption is not mislabeled as delivered. The remaining code-level choices are local algorithms and file organization within these contracts.
