# Which architecture is an older decision describing?

Written: **2026-11-05**.
Work: [machine foundation — #859](https://github.com/KirkDiggler/rpg-project/issues/859).
Previous entry: [Behaviors for authors and generators](2026-11-05-how-can-behaviors-serve-authors-and-generators.md).
Review correction: [toolkit#2271](https://github.com/KirkDiggler/rpg-toolkit/pull/2271#issuecomment-6649755562).

## Question

Does a related ADR title or a surviving interface name make an older design a
current architectural foundation?

The operator identified distinct development eras: ADR-0016 and ADR-0014 belong
to much earlier thinking, and ADR-0043 describes a driver direction the project
abandoned. The major shift is toward `play/`, `world/`, composition, and tools
rather than applications. This changes how the proposal should argue for its
boundaries, even though the operator otherwise liked its direction.

## Evidence

**Supplied historical orientation:** the operator described the composition
shift as roughly August 2026 and recalled that ADR-0016 may even predate the
rulebooks package. That is recorded in the review comment above as reported
recollection. This pass did not establish a precise cutoff date, package-birth
chronology, or a classification of every ADR into eras.

**The mistake in the first proposal is visible.**
[ADR-0049 at its first revision](https://github.com/KirkDiggler/rpg-toolkit/blob/a16c4573829718d2e119c832eb05359029e6cc36/docs/adr/0049-behavior-is-a-composer.md#related-decisions)
said ADR-0043, including its amendment, remained the execution boundary and
used ADR-0014 as authority for the parameter-selection distinction. ADR-0016
was already labeled historical, but the surrounding framing still treated older
records as current supports based on related names and ideas.

**The current model has direct sources:**

- [`play/README.md`](https://github.com/KirkDiggler/rpg-toolkit/blob/05d75c043e990b952709b275a33fb37aa48d8c61/play/README.md)
  describes small, independent, value-returning leaves and the compositions
  that assemble them.
- [`world/doc.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/05d75c043e990b952709b275a33fb37aa48d8c61/world/doc.go)
  describes a deliberately small composer over independent tools and injected
  rulebook decisions, not a general mind application.
- [`encounter/doc.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/05d75c043e990b952709b275a33fb37aa48d8c61/rulebooks/dnd5e/encounter/doc.go)
  describes the current composition and ownership of location knowledge.
- [`turndriver.go`](https://github.com/KirkDiggler/rpg-toolkit/blob/05d75c043e990b952709b275a33fb37aa48d8c61/rulebooks/dnd5e/encounter/turndriver.go)
  still contains `Driver.Act(MonsterView) (Decision, error)`. That is evidence
  of the callback present in current code, not proof that the old driver
  architecture remains the foundation or that this exact API must never change.

Those relevant source/contracts match fetched main `3182088c`. The observed
callback and the historical architecture are distinct facts.

[Correction commit `05d75c04`](https://github.com/KirkDiggler/rpg-toolkit/commit/05d75c043e990b952709b275a33fb37aa48d8c61)
changes only ADR-0049 and its run record. The proposal now names its
composition-era foundation, labels older ADRs as history rather than parent
contracts, and carries an explicit correction to the initial draft. Older ADR
contents and statuses are unchanged. Documentation checks pass for 50 indexed
ADRs, 28 mapped modules, relative links and whitespace; the root genre checker
still reports its 44 unrelated advisory findings.

No new runtime test or experiment was performed for this provenance correction.
Earlier journal entries and receipts remain unchanged; this later entry records
what changed in the interpretation.

## Interpretation

The failure was not reading historical material. It was **borrowing authority
without establishing that the decision's assumptions still fit the architecture
we are designing within**. A plausible principle can survive while the framework
that once expressed it has been abandoned.

The tools-versus-applications distinction is consequential. An old framework's
context object, fixed role categories or decision-publishing interface can
quietly constrain a new reusable tool even if none of its code is copied.
Likewise, a callback still named Driver does not require treating the old driver
proposal as the parent of a new behavior composer.

For this proposal, the argument should start with the designer/generator need,
the current leaf/composition model, and inspected consumer contracts. Older
ADRs can explain paths taken and discarded. They cannot substitute for showing
why a boundary is appropriate now.

This is an architectural-era distinction, not a date-based rule that every
older idea is invalid. The proposed `mind/behavior` responsibility and parameter
boundary are otherwise unchanged. They remain Proposed; correcting the citations
does not accept an API or revive earlier application architecture.

## Next

Continue judging [the corrected proposal](https://github.com/KirkDiggler/rpg-toolkit/blob/05d75c043e990b952709b275a33fb37aa48d8c61/docs/adr/0049-behavior-is-a-composer.md)
through its consumer contracts: reusable behavior definitions, bindings from
studio/generation, private progress, and the retained-patrol case. If an older
idea is useful, make its applicable assumptions explicit instead of inheriting
it through a title or status label.

The S1 resume-target and concrete continuation contract remain on
[#866](https://github.com/KirkDiggler/rpg-project/issues/866). This correction
leaves those behavior choices open and changes no runtime code.
