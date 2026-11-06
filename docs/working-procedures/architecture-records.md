# Architecture records: which skill owns which part

A reference map for connecting a significant decision to its implementation and
the surrounding architecture. **This is not a new workflow or approval gate.**
Follow the linked canonical guidance; not every change needs every artifact.

## Artifact and skill map

| Question | Artifact | Existing owner / guidance |
|---|---|---|
| What customer experience or deliverable is this for? | Story and slice issues | [story](../../.agents/skills/story/SKILL.md) |
| What are we learning while we investigate? | Dated journal entries and experiments | [work-records](../../.agents/skills/work-records/SKILL.md), [journal guide](../../.agents/skills/work-records/journal.md) |
| What did we decide, and what does it cost? | ADR in the owning repository; other design law where applicable | [decision-record guide](../../.agents/skills/work-records/decision-record.md), owning ADR conventions and [design](../../.agents/skills/design/SKILL.md) |
| How do the layers, contracts and refusals fit together? | Architecture walkthrough | [design walkthrough guide](../../.agents/skills/design/walkthrough.md) |
| What exact work and tests implement the agreed contract? | Checked implementation plan | [implementation-planning guide](../../.agents/skills/design/planning.md) |
| What is the bounded deliverable, and what actually changed? | Slice issue, code, PR and run record | [story](../../.agents/skills/story/SKILL.md), [build-slice](../../.agents/skills/build-slice/SKILL.md), owning repo instructions |
| What does the joined result demonstrate? | Applicable QA/review evidence | [walk](../../.agents/skills/walk/SKILL.md), [pr-review](../../.agents/skills/pr-review/SKILL.md) |
| How does the implementation compare with its decision and plan? | Observed implementation record | Owning repo convention; [toolkit instructions](https://github.com/KirkDiggler/rpg-toolkit/blob/main/CLAUDE.md) specify the idea-folder `implementation.md` |
| What knowledge belongs beyond this one run? | Linked knowledge entries | [ingest](../../.agents/skills/ingest/SKILL.md) |
| How do we share a readable account afterward? | Retrospective field note / article | [field-note guide](../../.agents/skills/work-records/field-note.md) |

`work-records` separates the document jobs. `design` governs architectural judgment
and the translation into a checked plan. `story` and `build-slice` connect that
plan to delivery. The owning repository controls where its implementation record
lives; the toolkit's filename is not a new workspace-wide prescription.

## The two directions a reader needs

**From the need to the result:** experience/story → investigation → decision and
walkthrough → task contracts/slices → code and verification → observed
implementation. The plan's coverage and provider/consumer tables supply the
traceability, rather than a second copy of every requirement in every document.

**From a change back to its reason:** code/test or PR → slice and plan → the ADR or
design constraint → the larger architecture and user need. The walkthrough's
ownership table, refusals and source map make that path understandable to someone
who was not in the original conversation.

These are navigation paths, not a claim that design is a one-way waterfall. New
implementation evidence can reopen a particular decision through the existing
[design gap judgment](../../.agents/skills/design/SKILL.md#gap-judgment).

## Different records are not substitutes

The canonical guides distinguish a deliberate decision from its explanation,
a plan from an observed result, and a journal from a delivery tracker. An ADR's
Accepted status does not mean its API is implemented or its integration verified.
A complete record keeps those distinctions visible through links rather than
making a polished article carry technical authority.

For planning, use the actual inspected interfaces, files, tests and consumer
seams required by the planning guide—not a list of repository names. For observed
implementation, use real outcomes, revisions/versions, verification and deviations
under the owning repo's convention—not the plan rewritten in past tense.

The shared [execution contract](../teams/roles/working-agreements.md#13-execute-the-agreed-brief-to-its-finish-line)
continues to govern agreement, finish lines, continuation and real blockers.
This map does not add another approval between an agreed design and its faithful
checked plan.

## Worked trail

The item/effects documentation work is tracked in
[#876](https://github.com/KirkDiggler/rpg-project/issues/876). Its toolkit walkthrough
and topic-specific paper-trail map connect [ADR-0048](https://github.com/KirkDiggler/rpg-toolkit/blob/main/docs/adr/0048-equipment-granted-abilities.md)
to the surrounding bus/action architecture, item consumers and effects extraction
scope. The issue links the documents and PRs; their state is not duplicated here.
