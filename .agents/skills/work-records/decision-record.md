# Decision record: a deliberate architectural choice

An ADR makes a choice explicit and gives future readers the context needed to
understand or revisit it. It can begin as Proposed while investigation is still
underway; it is not a polished explanation invented after implementation.

## Start with the owner

Locate the repository that owns the architectural consequence. Read its ADR
README, template, relevant existing decisions and index before choosing a path,
number or status. For toolkit decisions, those live in `rpg-toolkit/docs/adr/`;
its README and `DECISIONS.md` are the starting points. Do not copy their procedure
into another repository or create a competing ADR in the journal.

Use the existing `design` skill when the work requires design or implementation
planning. An ADR links those contracts; it does not silently replace them or
create a second authority for the same invariant. If an existing decision is
being reconsidered, identify it before proposing a replacement.

## Content to carry through the owning template

- **Context:** the concrete user need, current behavior and constraints. Link the
  investigation and distinguish inspected implementation from intended design.
- **Alternatives:** the viable choices, including keeping the present shape when
  viable; why alternatives are rejected or remain open. Keep the tempting ones
  visible so future readers can revisit the choice honestly.
- **Decision:** the proposed or accepted choice, its scope and the responsibility
  boundaries it establishes. Record acceptance only through the owning process.
- **Consequences:** benefits, costs, limits and what becomes harder. Describe
  expected consequences as expectations, not measured results. Name evidence or
  changed conditions that would justify revisiting the decision.

Tests and examples inform the decision; they do not automatically accept it.
One passing consumer cannot establish an untested consumer's requirements.
An open question remains open rather than becoming an implicit API contract.

## Connect without duplicating

Link journal entries and reproducible examples as evidence. The issue owns the
implementation checklist; the ADR owns the architectural choice. A later journal
entry can report what implementation revealed and link a reconsidered ADR.

When a decision changes, follow the repository's supersession/deprecation
convention, preserve the earlier rationale and connect the old and new records.
Update the owning decision index as required. Do not leave two accepted,
contradictory descriptions of the same boundary.

Before handoff, check that a reader can tell which statements are evidence,
which are proposals and which have been accepted—and can reach the source for
each without depending on this session's memory.
