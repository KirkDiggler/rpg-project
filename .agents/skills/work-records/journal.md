# Journal: the investigation unfolding

A journal captures useful units of learning while the work happens. Write when a
question, observation, experiment or interpretation changes—not once per tool
call, and not to manufacture a daily activity log.

## Home and continuity

Use an existing journal for the topic. Otherwise use
`rpg-project/docs/journal/<topic>/README.md` as a small index and
`YYYY-MM-DD-<question>.md` for entries. Several questions on one day can have
separate entries. The index describes scope and links entries, the owning issue,
and relevant decisions; delivery status stays in the issue.

Work in the session's own repository worktree. Another session may supply links
or a handoff, but it does not write into the investigator's working tree.

## Entry shape

Begin with the date written, a question-led title and a link to the work's issue
or PR. Mark reconstructed earlier work as **Retrospective reconstruction** and
name the period it describes; the written date remains today.

Use these four sections, kept as short as the evidence allows:

1. **Question** — what are we trying to understand, and which user need makes it
   worth asking? Distinguish agreed scope from a possible direction.
2. **Evidence** — what did we read, build or observe? Link specific source paths
   and revisions, review comments, commands/results or experiment receipts. Label
   supplied reports as reports and sketches as proposals. State what was not
   exercised. Do not turn an unrun command into a result.
3. **Interpretation** — what do we think the evidence means? Name uncertainty,
   plausible alternatives and how it changes the earlier view. Link the earlier
   entry. An interpretation can be that more evidence is needed.
4. **Next** — the next question or bounded test that could distinguish the
   alternatives. Link delivery work rather than duplicating its checklist. If the
   investigation is finished, link its decision or state its unresolved outcome.

An unknown is useful content. Do not fill an empty evidence section with an
inference merely to complete the format.

## Preserve the path

Leave earlier assumptions intact. Add a later entry linking backward; an earlier
entry can gain a short dated forward link. For a factual error, add a visible
correction naming the evidence; do not silently rewrite what was believed.
Secrets or licensed material are not history to preserve publicly: stop further
sharing and follow the repository/operator's remediation process.

## Example of the distinction

For the motivating question, “Could equipment grant an existing ability rather
than implement its own version?”:

- **Question:** Can a ring grant Second Wind through a reusable composition?
- **Evidence:** Record the actual consumer example and its receipt here. If none
  exists yet, say “No consumer example has been run”; the proposed ring is not
  evidence of working behavior.
- **Interpretation:** Sharing behavior is a hypothesis; grant ownership and use
  lifetime still need examination.
- **Next:** Exercise grant/use/removal in a bounded example, and record what that
  reveals before selecting a shared abstraction.

This illustrates a journal entry, not a prescribed architecture or settled game
rule. The investigation supplies the actual test cases and conclusions.
