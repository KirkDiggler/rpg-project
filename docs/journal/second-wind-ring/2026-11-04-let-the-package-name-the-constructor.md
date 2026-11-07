# Let the package supply the constructor's noun

Written: 2026-11-04. Work: [Keep equipment S1 — #862](https://github.com/KirkDiggler/rpg-project/issues/862).
Previous: [The consumer makes construction hard to read](2026-11-04-consumer-feedback-cleans-the-provider-api.md).

## Question

Should constructor arguments be named after the operation (`NewInput`), the
constructed type (`MachineConfig`), or the package's context (`machine.Config`)?

## Evidence

The first cleanup introduced `ComposeInput` and `machine.NewInput`. The operator
then distinguished constructors from operations and preferred the familiar Go
`thing.New(thing.Config)` form, avoiding stuttering `thing.NewThing` names.

[Provider PR #2276](https://github.com/KirkDiggler/rpg-toolkit/pull/2276) now exposes
`effects.New(effects.Config)` and `machine.New(machine.Config)`, returning
`*effects.Composed, error` and `*machine.Machine` respectively. Provider head
`67152908` passes both package race suites, vet, lint, the candle example and
benchmark smoke. The function bodies' behavior and all configuration fields
remain unchanged by this naming pass.

[Item consumer #2273](https://github.com/KirkDiggler/rpg-toolkit/pull/2273), head
`b8760f24`, uses the real provider pseudo-version
`v0.0.0-20261105070308-671529087255`. Its race suite, executable example, vet and
lint pass after updating the call. Neither PR is merged.

## Interpretation

The package already supplies the domain noun. New(Config) names construction
without repeating the result type or making a generic verb look like the thing
being configured. The previous named fields remain useful; the constructor and
configuration type names now follow the package idiom.

These Config types include runtime construction dependencies and instance data.
They are not declarations of a persisted or studio-authored item schema. This
cleanup does not choose the unresolved grant/action contract.

## Next

Inspect the cleaned construction API, then resume the consumer-seam investigation.
The earlier journal entry remains intact so the change from positional calls to
operation-named inputs and then idiomatic constructors stays visible.
