---
name: work-records
description: Use when capturing an investigation in a work journal, recording or revisiting an architecture decision (ADR), or preparing a retrospective field note or article from development work.
---

# Work records

Keep the investigation, the deliberate decision, the delivery record and the
readable story connected, with each document doing its own job.

## Choose the document

Read the selected guide before writing. Paths below are relative to this skill.

| Need | Document / guide | Owns |
|---|---|---|
| Capture what we are learning now | [Journal](journal.md) | Question, evidence, interpretation, next |
| Propose, accept or revisit an architectural choice | [Decision record](decision-record.md) | Context, alternatives, decision, consequences |
| Share what happened after an investigation | [Field note](field-note.md) | A readable, evidence-grounded retrospective |
| Define delivery scope or report progress | Existing `story` skill | Issues, slices and their delivery state |
| Design or plan a changed capability | Existing `design` skill | Design agreement, contracts and implementation planning |

Start with the artifact the operator needs; the table is not a requirement to
produce every kind of document. A journal can exist without an ADR or article.

## Shared contract

- Evidence says what was inspected, reported or exercised, with source links
  and limits. Interpretation says what that evidence might mean. A proposal is
  not an accepted decision, and an example is not production delivery.
- Link to the owning issue, ADR, design or test receipt. Keep one authoritative
  home for each decision and delivery status; the journal is not a second backlog.
- Preserve changes of mind. Later entries explain what changed and point back;
  earlier uncertainty remains readable. Correct factual errors explicitly.
- Adoption during ongoing work starts with today's entry. Earlier material is
  a **retrospective reconstruction**, dated when written and linked to its sources.
- Use the operator's stated intent; do not invent personal experiences, motives,
  quotations or a hiring pitch. Clear writing can retain uncertainty and failure.
- Repository visibility and publication are separate choices. A public repo is
  already public; creating a record does not authorize Pages publication.

Follow repository instructions for placement, attribution and review. This skill
adds no acceptance gate and grants no permission to change another session's work.

## Common mistakes

| Mistake | Use instead |
|---|---|
| Polishing yesterday into today's conclusion | A new, linked entry explaining the change |
| Filling in missing tests or history | An explicit evidence gap or sourced reconstruction |
| Calling an investigation a success story | An account of its actual outcome, including uncertainty |
| Copying issue checklists into the journal | Link the issue; capture the learning behind the work |
| Replacing a living story overview with an essay | Keep the overview current under `story`; write the retrospective separately |
