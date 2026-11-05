# Where does interrupted behavior keep its progress?

Written: **2026-11-05**.
Work: [NPC behavior S1 — #866](https://github.com/KirkDiggler/rpg-project/issues/866),
under [#859](https://github.com/KirkDiggler/rpg-project/issues/859).
Previous entry: [Which architecture is an older decision describing?](2026-11-05-which-era-does-a-decision-belong-to.md).
Implementation: [toolkit#2277](https://github.com/KirkDiggler/rpg-toolkit/pull/2277).

## Question

The operator approved proceeding with `mind/behavior` as composer and accepted
mechanical parameter metadata as sufficient to begin. A separate quality-profile
framework is not required up front. The
[agreement and scoped contract](https://github.com/KirkDiggler/rpg-project/issues/866#issuecomment-6650897115)
turn the earlier proposal into implementation work without authorizing a merge.

Can a real composer preserve a patrol's pending work while investigation owns
an action opportunity, use the real weighted selector with controlled rolls,
and resume that work after reload? Where do parameter binding, observation,
selection and continuation belong without rebuilding the world inside mind?

## Evidence

The [component receipt](https://github.com/KirkDiggler/rpg-project/issues/866#issuecomment-6652161699)
pins code head
[`38dae150`](https://github.com/KirkDiggler/rpg-toolkit/commit/38dae150ad6ecbebb59e46856ec1e55db48d0ca2)
and the actual commands/results. Only `mind/behavior` changes as a Go module;
repository plan/map/run-record documentation accompanies it.

- [`ParameterSpec` / `CheckParameters`](https://github.com/KirkDiggler/rpg-toolkit/blob/38dae150ad6ecbebb59e46856ec1e55db48d0ca2/mind/behavior/parameters.go)
  describe and check typed scalar values, requiredness, integer bounds and
  string choices. The checker copies accepted values, preserves zero/false and
  absence, and supplies no sampler, defaults or template language.
- [`Composer`](https://github.com/KirkDiggler/rpg-toolkit/blob/38dae150ad6ecbebb59e46856ec1e55db48d0ca2/mind/behavior/composer.go)
  owns private machine blobs in authored priority order. Observe accepts
  data-only input, optionally addressed to one instance. Decide previews real
  machines and rolls only the first requesting role's table; deferred action
  candidates do not advance. Data-only updates can commit alongside the winner.
- [`ComposerData` / `LoadComposer`](https://github.com/KirkDiggler/rpg-toolkit/blob/38dae150ad6ecbebb59e46856ec1e55db48d0ca2/mind/behavior/composer_data.go)
  persist continuation and definition identity, not compiled programs or another
  mutable configuration. The caller retains the original materialized config;
  a different definition/order or malformed blob is refused.
- The [joined fixture](https://github.com/KirkDiggler/rpg-toolkit/blob/38dae150ad6ecbebb59e46856ec1e55db48d0ca2/mind/behavior/patrol_composition_test.go)
  uses checked duration metadata in a real recipe. It progresses patrol to its
  next leg, activates investigation on a supplied clang, selects different
  movement answers at faces 1 and 101 on d200, and returns to the same pending
  patrol target. The second branch runs through JSON reload. The only selection
  double is a scripted die that refuses unexpected requests.

### Findings while joining the pieces

A fact-projection callback could initially mutate a stored selector through the
picker's input. A regression failed with expected X=4 versus actual X=99.
Selected rows now are detached before that boundary, and returned results are
detached too. Copying only the final answer was too late.

Broadcast-only completion also was insufficient: two attachments of the same
definition could both advance. The selected instance ID now addresses completion
through `ObserveInput.Instance`; an empty address remains a broadcast and an
unknown one refuses without mutation. This adds the missing path to the existing
state owner, rather than creating another owner in a helper.

A no-candidate hold retains the requesting machine's pending step. An explicitly
selected Hold row is different: it is eligible authored work and can progress.
That distinction preserves both affordability and intentional waiting.

### Verification and limits

Full owning-module `go test -race -count=1 ./...`, focused suites, lint and the
actual scoped pre-commit hook pass. Head-specific CI passes, including the
changed-module test job; repository-wide `test-all` is skipped. Initial new-API
red runs were compilation failures, not behavioral proof by themselves.

Two temporary assertion-level mutations fail and were restored: committing a
deferred instance changes `ready` to `spent`, and omitting the metadata upper
bound admits 5 above max 4. The independent
[review](https://github.com/KirkDiggler/rpg-toolkit/pull/2277#pullrequestreview-5781061951)
found no Critical, Important or Minor findings, reran focused suites, and
independently killed/restored the loader's wrong-scalar mutation. The verdict
is MERGE-READY for the component checkpoint, not a merge authorization.

The measured two-role/one-row decision cost is 13,554 ns/op, 4,003 B/op and
34 allocations on this host. It includes defensive copies but excludes real
RNG, geometry and execution; it is not a keep-scale measurement.

Nothing here physically moves a guard or changes the live encounter. No studio
field, dungeonspec extension, listener index, virtual paused clock, dynamic
attachment manager, general memory policy or recovery engine is delivered.
The existing geometric patrol/dwell behavior and effects remain untouched.

## Interpretation

There is now a production continuation owner, rather than two private states
coordinated by a test helper. A losing role can keep its pending work while
another supplies the selected action, and its saved progress belongs to the
same materialized configuration after reload.

The earlier exact-interruption-cell versus unfinished-waypoint question does
not need a universal answer in this tool. The authored fixture selects its
pending waypoint; the future geometric consumer still owns the physical rejoin
policy. Treating that content question as a prerequisite for any continuation
API had conflated two layers. It remains explicit, not silently ruled for the
live keep.

The composer is deliberately one ordered selection lane. It does not promise
that every kind of simultaneous behavior can already cooperate, or that holding
a blob freezes elapsed caller time. Those limits are preferable to claiming a
general scheduler from a two-role test.

Mechanical parameter metadata is useful without a quality framework: changing
the validated investigation window changes the executed recipe. Binding is
still an explicit producer operation. This does not establish a general
editable behavior-library format or prove future generated settings are fun.

## Next

The operator can inspect the real component interface and deterministic fixture
before choosing a live adoption. Merge/release remain explicit decisions.
Subsequent work should follow a consumer through authored bindings, delivery of
its inputs, execution of the already-selected row, and persistence—without a
second roll or a reconstruction of private progress outside its owner.

The ongoing story and slice retain delivery status. This checkpoint is not a
claim that the keep's patrol/interruption journey has been demonstrated through
the multiplayer client.
