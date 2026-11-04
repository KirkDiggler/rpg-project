# mind/machine investigation journal

Working notes for the machine exploration, kept separately from its design,
decisions and delivery tracking. Entries distinguish evidence, interpretation
and proposals; the linked issue remains the home for delivery state.

## Scope and record

- [Machine exploration — rpg-project#859](https://github.com/KirkDiggler/rpg-project/issues/859): scope, decisions and delivery record.
- [Exploration walkthrough — toolkit PR #2271](https://github.com/KirkDiggler/rpg-toolkit/pull/2271): the current designer-facing discussion and its source evidence.
- [Keep goal — rpg-project#702](https://github.com/KirkDiggler/rpg-project/issues/702): the customer experiences motivating the work.

At journal setup, PR #2271 describes exploration inside `mind/machine`, keeping
the effects prototype separate and deferring extraction until the boundary is
demonstrated. This is a dated orientation from the PR on 2026-11-04, not a new
ruling or a replacement for that record. Read the current issue/PR before resuming.

The later effects/shared-machine discussion can use these findings as evidence.
Creating this journal does not select a shared package or widen the machine
session's scope.

## Entries

- **2026-11-04 — [What should the machine foundation let a designer express?](2026-11-04-what-should-the-foundation-unlock.md)** First entry: intended capabilities, the inspected sound/memory/machine boundaries, and open questions about cooperating behaviors and recovery. Earlier work is explicitly reconstructed from linked sources; no shared architecture is selected.

## Adoption handoff

Read the [work-records skill](../../../.agents/skills/work-records/SKILL.md) and
its [journal guide](../../../.agents/skills/work-records/journal.md).

In your own `rpg-project` worktree, start a dated entry under this directory using
**Question / Evidence / Interpretation / Next**. Link the current review, inspected
source revisions and actual test receipts. Start with what you know now; label
any earlier account as a retrospective reconstruction and cite its sources.
Preserve earlier assumptions and later corrections visibly. Keep delivery
checklists and decisions in their existing owning records.

Record what the evidence could imply for effects or shared machinery as a question
or interpretation—not an accepted extraction decision. We will review the journal
together before changing that idea. Do not draft or publish a success article now.

If the skill is not yet discoverable in an already-running session, read its
explicit source path from the feature worktree/PR. After it is installed in the
canonical checkout, Pi can refresh discovery with `/reload`, then use
`/skill:work-records`. Other runtimes can read `SKILL.md` directly.
