# Can the existing machine express the guard trace?

Written: **2026-11-04**.
Work: [NPCs that feel alive — #859](https://github.com/KirkDiggler/rpg-project/issues/859).
Previous entry: [What should the foundation unlock?](2026-11-04-what-should-the-foundation-unlock.md).

## Question

The previous entry proposed a paper trace. The operator asked to make the
next step executable through a unit test, emphasizing that tests can control
rolls. Can the current machine express the first part of the guard journey
without changing its core, and can the test detect a real behavioral break?

The [bounded probe](https://github.com/KirkDiggler/rpg-project/issues/859#issuecomment-6644100984)
uses caller-supplied information: patrol, hear a qualifying clang, search,
receive a relevant report, then select a challenge. This is one authored path,
not a commitment to an architecture for concurrent roles or recovery.

## Evidence

The executable source is
[`guard_journey_test.go` at `9f3e64d0`](https://github.com/KirkDiggler/rpg-toolkit/blob/9f3e64d03700db8292116b3a537ac32887a896b6/mind/machine/guard_journey_test.go),
on [toolkit PR #2271](https://github.com/KirkDiggler/rpg-toolkit/pull/2271).
The real `Recipe.Compile`, `Program.Step`, `Listens` and blob JSON serialization
run; there is no replacement machine in a test helper. Vocabulary readers
simply read the fixture's supplied facts, including whether each is known.

### The authored trace

| Supplied input | Expected observable result |
|---|---|
| Time at round 10, initially patrolling | Request the `patrol` behavior table. |
| Nearby `clang`, still at round 10 | Enter `searching`, now listen for `intel`; request no table execution. |
| Time at round 11 | Request the `search` table. |
| Known relevant escaped-party report, still at round 11 | Enter `questioning`; request no table execution. |
| Time at round 12 | Request the `challenge` table. |

The state names, six-unit sound scope and response are test content, not new
keep rules. Information inputs changing state without returning a pick is a
choice of this recipe. It does not decide when the live encounter delivers
those inputs or what a physical action costs.

Five suite methods, with seven subcases in the two table-driven methods, also
exercise a voice instead of a clang, a clang beyond the scope boundary, missing
sound category, a nonmatching or unknown report, independent instances sharing
a compiled program, and resuming the search after a blob JSON round trip.

### Runs and negative controls

The [experiment receipt](https://github.com/KirkDiggler/rpg-project/issues/859#issuecomment-6644178404)
records the commands and their limits:

- **Initial run:** the new suite passes against unchanged production code.
- **Missing state-write control:** temporarily retain the old state instead of
  assigning the transition target. The journey test fails: its next pick is
  `patrol`, not `search`, and its listener remains `heard`, not `intel`.
- **False-guard control:** temporarily skip only `Depends`, allowing
  `DoesNotApply` to fire. The nonmatching-report test fails: it fires and
  requests `challenge` rather than staying with `search`.
- **After restoration:** complete `go test -race -count=1 ./...` in
  `mind/machine` passes (1.028s); the focused verbose suite passes;
  `golangci-lint run ./...` reports 0 issues. The scoped pre-commit hook passes
  with its own cache disabled. Production `step.go` and module pins are unchanged.

Both negative controls compiled and failed assertions; neither is committed.
This is a design/characterization probe with sensitivity checks, **not a claim
that a missing production feature went red and was implemented**.

No roll occurs in `mind/machine`. The test controls facts, event values and
clock inputs. A returned `Pick` names a table; the consumer must still select a
row and execute its meaning. We did not test an actual die, behavior-table
selection or a physical action here.

Nor did we exercise spatial recipient selection, live sound/report delivery,
report provenance, memory retention, concurrent roles, dialogue eligibility,
full encounter save/load, multiplayer presentation or recovery. The blob round
trip is not evidence for all those persistence boundaries.

## Interpretation

**The existing kernel is sufficient for this single-path trace when its caller
supplies the vocabulary.** There is no evidence from this experiment that we
need a new core API merely to switch behavior in response to information.
That narrows the earlier question; it does not settle the larger architecture.

The unit probe is useful as an executable authoring example and a regression
check: it observes the next behavior requested, not just a transition result
that could claim success while leaving the machine in the old state. Known
false and unknown inputs are deliberately separate cases.

It also keeps a boundary visible. The caller knows what a clang or relevant
report means. The machine knows how to evaluate the supplied guard and advance
state. A live composition still needs to deliver those inputs, preserve whose
knowledge they represent, and decide when to run the requested behavior.
Passing this test cannot hide those missing connections.

The operator's controlled-roll testing direction fits the next boundary:
**real selection with supplied dice, not randomness added to the machine and
not a fake picker returning the answer the test expects**. Exact scripted rolls
can select hand-checked branches while retaining the real eligibility and
weight calculation. No seeded production simulation is implied.

This result supports continuing in place. It does not select extraction, prove
multiple roles can coexist, or justify a new transaction tool. The broader
hypotheses in the [first entry](2026-11-04-what-should-the-foundation-unlock.md#interpretation)
remain open.

## Next

Follow the returned `Pick` across one consumer boundary: a small authored
behavior table with explicit, hand-checked roll outcomes. Exercise the real
selector, check which choices are eligible, and make the scripted roller refuse
unexpected requests so extra rolls cannot quietly change the trace.

That should distinguish what the current machine-plus-table composition already
provides from what role coordination would add. Recording/report semantics and
recovery remain separate unanswered questions, not policies to implement inside
a convenient test helper. Scope and any further design decisions remain on
[#859](https://github.com/KirkDiggler/rpg-project/issues/859).
