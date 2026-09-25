---
name: design
description: Use when a session designs anything — a new capability, a changed shape, a rules or schema change, an ideas/ design doc, a refactor that moves a seam. Governs the conversation (probe before propose, shape before detail) and the artifact (shape, law, rulings, open) so the doc stays law and the case file stays in the PR.
---

# Design

## The order

1. **Probe before propose.** Measure the current state first — read the code,
   run the check, read the owning package's godoc and design doc. A design
   argued against an unmeasured state is a guess wearing a plan's clothes.
2. **Shape before detail.** Give the operating human the shape first — a
   mermaid diagram of the structure or flow — and confirm it before
   elaborating. The shape gives the details a place to live in the reader's
   head; detail that arrives first has nowhere to go.
3. **Approaches before design.** Two or three, with trade-offs and a
   recommendation. YAGNI ruthlessly — remove anything no ruling asked for.
4. **One question at a time**, multiple choice preferred. A design
   conversation is steered, not surveyed.
5. **The ratchet.** Complexity discovered mid-design upgrades the
   conversation. Say so; nothing downgrades silently.

## The artifact

A design doc carries four sections and nothing else:

1. **Shape** — mermaid first: the one picture that *is* the design. Diagrams
   for structure and flow; prose for naming and word choices.
2. **Law** — the rules, present tense, each carrying at most one
   why-sentence. The doc states invariants; it does not narrate how they came
   to be.
3. **Rulings** — a status table. Columns: `ID`, `status`, `scope`,
   `ruled by`, `date`. Statuses: `settled`, `deferred-until-X`, `open`,
   `superseded`. Rulings number into the conversation as R1, R2, … No
   verbatim quotes — a quote reads as authority to a future session and
   outlives its meaning.
4. **Open** — the unsettled questions, named per item, so a settled question
   is not re-opened and an unsettled one is not mistaken for settled.

Everything else is the **case file**: the transcript, the approaches that
lost, the evidence table pinned to a commit, the corrections. It lives in the
issue or the PR, one hop from the doc — visible for the archaeology, absent
from every future session's context. Everything a loaded doc carries is
co-equal input; law and case file compete for the same salience, and the case
file loses on purpose. Evidence obeys the same rule: the invariant is stated
in the doc; `file:line @ commit` rides with the ruling that established it.
The tell is greppable and the check makes it a mechanism: run
`game-dev/scripts/check-doc-genres.sh` before publishing the design PR — a
finding is the smell of case file in law, exempt only by the operator's
inline `<!-- case-file -->` ruling.

## The gate

No implementation before the design doc is ruled on by the operating human —
the doc as a PR, rulings entered in the table with attribution. A ruling
records its scope: one that binds a single doc's shape must say so, or it
will be read as doctrine.

Where the doc lives: cross-repo designs in
`rpg-project/ideas/<topic>/design.md` (that PR stays open as the tracking
surface); toolkit-scoped designs in `rpg-toolkit/docs/ideas/<name>/`; other
repos by their own convention.

## The language

Roles, not names: the session, the operator, the owning repo. Identity lives
in the record — ruling rows carry `ruled by` — never in procedure. Attributed
quotes may stand where they already exist, as history; new procedural text
assumes nothing about who is at the wheel.

## Deliberately not in this version

Classification ladders, staged approval gates, spec directories and vocabulary
borrowed from other processes. The house gate is the one that exists: the
operator rules on the PR'd design doc.