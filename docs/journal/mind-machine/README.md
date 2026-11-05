# mind/machine investigation journal

Working notes for the machine exploration, kept separately from its design,
decisions and delivery tracking. Entries distinguish evidence, interpretation
and proposals; the linked issue remains the home for delivery state.

## Scope and record

- [Machine exploration — rpg-project#859](https://github.com/KirkDiggler/rpg-project/issues/859): scope, decisions and delivery record.
- [Exploration walkthrough — toolkit PR #2271](https://github.com/KirkDiggler/rpg-toolkit/pull/2271): the current designer-facing discussion and its source evidence.
- [Keep goal — rpg-project#702](https://github.com/KirkDiggler/rpg-project/issues/702): the customer experiences motivating the work.
- [Mind architecture walkthrough](https://github.com/KirkDiggler/rpg-toolkit/blob/de47f4e4d7db7ee6f6fbb7a7a65d67390b712a6a/docs/architecture/components/mind.md): start here for the high-level shape, machine/composer zoom-ins, and the guard traced through both. This is the explanatory guide, not another journal or decision authority.
- [Accepted ADR-0049: behavior as a parameterized composer](https://github.com/KirkDiggler/rpg-toolkit/blob/81b5ed0f4a475fa95b6c6fce768ad4a5d9ed6d52/docs/adr/0049-behavior-is-a-composer.md): the agreed responsibility and mechanical parameter-metadata direction; implementation/adoption remain separately evidenced.

At journal setup, PR #2271 describes exploration inside `mind/machine`, keeping
the effects prototype separate and deferring extraction until the boundary is
demonstrated. This is a dated orientation from the PR on 2026-11-04, not a new
ruling or a replacement for that record. Read the current issue/PR before resuming.

The later effects/shared-machine discussion can use these findings as evidence.
Creating this journal does not select a shared package or widen the machine
session's scope.

## Entries

- **2026-11-04 — [What should the machine foundation let a designer express?](2026-11-04-what-should-the-foundation-unlock.md)** First entry: intended capabilities, the inspected sound/memory/machine boundaries, and open questions about cooperating behaviors and recovery. Earlier work is explicitly reconstructed from linked sources; no shared architecture is selected.

- **2026-11-04 — [Can the existing machine express the guard trace?](2026-11-04-can-the-existing-machine-express-the-guard-trace.md)** Executable unit probe: the single path passes against the existing kernel; two intentional mutations demonstrate that the test catches broken behavior. Controlled-roll selection is the next consumer boundary, not part of this result.

- **2026-11-05 — [What must a patrol remember to resume?](2026-11-05-what-must-a-patrol-remember.md)** The next slice is agreed. Inspection and existing tests show that current patrols derive progress from position and abandon a departed waypoint's dwell; the retained-resume target is a specific open choice.

- **2026-11-05 — [How can the same behaviors serve authors and generators?](2026-11-05-how-can-behaviors-serve-authors-and-generators.md)** Procedural generation makes parameter suitability distinct from validity. Existing generation tests provide a baseline; ADR-0049 records the proposed composer boundary and alternatives without selecting APIs or accepting the architecture.

- **2026-11-05 — [Which architecture is an older decision describing?](2026-11-05-which-era-does-a-decision-belong-to.md)** Review corrects the proposal's use of older ADRs as current authority. The composition-era contracts become its basis; earlier records and journal entries remain intact, and no calendar cutoff is invented.

- **2026-11-05 — [Where does interrupted behavior keep its progress?](2026-11-05-where-does-interrupted-behavior-keep-progress.md)** The first production component checkpoint joins real machines and weighted selection, retains private progress across reload, and passes an independent review. It does not claim live encounter or studio adoption.

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
