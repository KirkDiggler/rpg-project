---
name: pr-review
description: Use when performing the independent review round on a feature or rules PR in any RPG repository — one bounded, pre-playtest review round with inline findings, explicit dispositions, a published verdict, and a focused QA/playtest handoff.
---

# Independent PR Review

## Overview

The review record is the PR, not a chat message and not a local file. One
inline thread per finding so the implementing agent can answer where the
finding lives. The verdict is published, never withheld while a reviewer
runs.

## Review for the stage we are in

We are **pre-playtest**, not preparing a production release. Review protects
our architectural seams and catches concrete defects; it does not certify
that the game is bug-free. Getting a playable slice in front of people is
part of verification, not what happens after exhaustive verification.

Use the cheapest evidence that answers the actual question:

- **Code review and focused tests:** ownership, contracts, rules, hidden
  state, and failures that a normal playthrough is unlikely to expose.
- **A focused QA pass:** does the changed journey work through the real
  stack, including the affected multiplayer view? Reuse an existing walk
  when it covers the same revision and path.
- **Player playtests:** are the choices understandable, the feedback clear,
  and the game worth playing? These questions need players, not another
  speculative code-review cycle.

Do not demand exhaustive edge-case matrices, production hardening, or new
verification machinery without a concrete risk in the current slice. Missing
automation is not itself a blocker when a short QA walk answers the question.
Playtests do not excuse broken ownership or known security/data-integrity
hazards; architectural standards and required repository gates still apply.

## When this applies

- Feature PRs and rules/engine changes: one independent review round from a
  session that did **not** implement the change (rpg-project AGENTS.md).
- Doc-only PRs, pin bumps, and small mechanical fixes may skip it.
- Trigger: the PR is marked ready for review, or the human asks for review.

## Before you review — load the baseline

In order, before reading a line of the diff:

1. `rpg-project/AGENTS.md` — the lens (composeable components, deliberate
   seams, no band-aids, judge by the tool it adds).
2. The owning repository's `AGENTS.md`/`CLAUDE.md` and the nearest scoped
   instructions.
3. **The package's own declared charter** — its `doc.go`/godoc, README, and
   any design doc: stated purpose, goals, non-goals, seams, and what is in
   its lane. The charter is the review baseline. A finding is a deviation
   from what the package *says it is*, not from what the reviewer prefers;
   code that works but contradicts the declared seam is the thing that is
   wrong.
4. The issue or spec the PR claims to close. Check scope: additions beyond
   an explicit "do not add" list are findings; a defect in the spec itself
   is reported as a spec-level finding, not silently worked around.

## Read-only evidence

- Inspect the head commit in a throwaway worktree
  (`git worktree add <tmp> <head-sha>`); never move HEAD on a shared
  checkout.
- Inspect applicable gate evidence and run focused checks for unresolved
  risks. Reuse recorded results tied to the same head, inputs, and relevant
  environment; distinguish reused evidence from commands you ran yourself.
  Do not rerun a full suite merely because the reviewer is a different
  session. Required CI, hooks, and repository readiness gates still apply.
- **Check what the diff's suite mocks.** A green suite is not evidence about
  a seam the tests replace: a change whose central wiring sits behind a
  component the suite doubles has untested wiring, and the failure mode is
  invisible by construction. Ask whether any test exercises the real path.
  If none does, check for real-path QA evidence. If that is also absent,
  name the gap and the smallest useful QA step in the verdict; do not turn
  every mock into a blocking demand for another automated test.
- Support defect claims with a concrete code path or a minimal reproduction.
  Use a probe when it resolves uncertainty, not as a ritual for every finding.
  Label unverified risks as such; do not assert a bug or its absence without
  evidence. Review does not require proving that no bugs remain.
- Name the mechanism a probe trips, and check the failure comes from it. A
  probe that trips the defect through some OTHER guard — a target the intent
  refuses, an entry that never reached the roll — has not shown the filed
  defect, and a fix tested by such a probe can pass while the defect lives.
- The probe rides with the finding. A defect whose reproduction is obvious
  from prose is cheap to answer; one whose fixture is not (a view, a row, a
  disposition to flip) carries the minimal shape in the thread itself, so
  the implementing agent re-verifies the defect instead of reconstructing it
  from a paragraph.
- Never review code you did not read. If the diff is too large for one
  pass, review it in passes and say so in the verdict.

## Findings

- Calibrated severity: **Critical** (must fix: security/data-integrity hazards
  or a broken core path) / **Important** (concrete scope or contract defect,
  including an ownership/seam violation) / **Minor** (non-blocking polish or
  improvement). Explain the current consequence, not a hypothetical
  production incident. Acknowledge strengths specifically.
- Separate defects from QA/playtest questions. Presentation, feel, and
  usability uncertainty normally belong in the handoff, not blocking
  threads. A demonstrated failure of the promised journey is still a defect.
- Each finding states: file:line, what is wrong, why it matters, and the
  fix if not obvious.
- When the fix is prescriptive, find the package's existing general
  mechanism first — a shared helper, a documented convention — and
  prescribe that. A hand-copied fix where a mechanism exists is itself the
  band-aid the lens forbids, and the nearest precedent is not the
  mechanism doc.
- Judge by the tool the change adds, never by the feature it closes. Test
  the design against full D&D scale, not today's handful of features.
- Test the standing invariants: Input/Output types on every function;
  never `(nil, nil)`; zero values tell the truth; toolkit types canonical;
  events for multiplayer; ownership before mechanism.

## Publication — inline threads, not a blob

One review per pass, published with the team-derived signature (derive the
operator with `gh api user --jq .login`; use the selected charter's
`## Signature` form):

```bash
gh api repos/{owner}/{repo}/pulls/{number}/reviews --input review.json
```

`review.json`:

```json
{
  "event": "COMMENT",
  "body": "<summary verdict, signed>",
  "comments": [
    {"path": "pkg/file.go", "line": 42, "body": "Important: ..."},
    {"path": "pkg/file.go", "start_line": 40, "line": 44, "body": "..."}
  ]
}
```

- The review **body** is the summary verdict: strengths, spec-level
  findings (numbered), the verdict, the signature.
- One **inline comment per finding**, anchored to its diff line. Lines must
  be part of the diff: `line` on the RIGHT side for added/changed code,
  `side: "LEFT"` for removed code. Multi-line anchors use `start_line`.
- Findings that cannot anchor to a line (spec-level, cross-cutting) go in
  the body, numbered so threads can reference them.
- After publishing, **read the review back** and confirm each comment
  landed on the line it meant to anchor.
- Threads anchor to the head commit at review time; fixes get a focused
  closure review at the new head, not an automatic restart of the round.

## The response loop

The implementing agent answers **in each finding's thread** — never as a
top-level comment:

```bash
gh api repos/{owner}/{repo}/pulls/{number}/comments/{comment_id}/replies \
  -f body="<response>"
```

Replies follow receiving-code-review discipline: verify before agreeing,
push back with technical reasoning when the finding is wrong, no
performative agreement, no gratitude.

Every thread closes with exactly one disposition, stated in the thread:

- **Addressed** — fixed, commit SHA (or follow-up PR) linked. When one
  commit carries several fixes, the head SHA named once suffices — say
  that it does.
- **Rebutted** — the finding is wrong; the reasoning is stated. The
  reviewer concedes in-thread if persuaded.
- **Deferred** — real but not now: the reason and a linked follow-up issue,
  or a named QA/playtest check in the PR with an owner. Deferring an
  Important finding requires explicit operator acceptance recorded in the
  thread; the reviewer cannot silently waive a blocker. Critical findings
  must be fixed or rebutted before merge.

A thread with no disposition is an open finding. The review round is not
complete while one exists.

## Closure

- **One independent round, then focused closure.** Read the fixes and check
  the affected paths at the new head. State what was checked or re-run per
  thread; reuse unaffected evidence. Do not restart whole-diff review or
  recruit another reviewer merely because fixes landed.
- Reopen review only for a concrete new blocker, a regression from the fix,
  or a material scope/seam change. Record other observations as non-blocking
  follow-ups or QA/playtest questions; they do not start another cycle.
- Stop when findings have dispositions and blocking fixes are verified.
  Update the published verdict with remaining limitations and the focused
  QA/playtest handoff. A local review file is not the record.
- Keep the handoff short: the changed player journey, what to try and
  observe, who will run it, and whether it is required before merge or is
  a later player-playtest question. Link existing results rather than
  repeating the walk. Do not claim a planned pass has happened or take over
  the operator's playthrough unasked.
- Review completion is not production certification or automatic merge
  permission. Merge-ready still requires declared scope implemented,
  applicable checks and agreed pre-merge QA complete, release prerequisites
  satisfied, and no unresolved Critical or Important blockers. Later player
  playtests need not be complete before merge unless explicitly agreed.

## Deliberately not in this version

Per-team severity gates, automated AGENTS/charter conformance checklists,
and lane-ownership linting all wait for the use case that pays for them.
The charter and lens loading above is v1's enforcement; a use case brings
the mechanism.