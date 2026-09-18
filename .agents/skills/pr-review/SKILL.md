---
name: pr-review
description: Use when performing the independent review round on a feature or rules PR in any RPG repository — publishes each finding as an inline review thread, runs the response loop to an explicit disposition per finding, and leaves the verdict published on the PR as the record.
---

# Independent PR Review

## Overview

The review record is the PR, not a chat message and not a local file. One
inline thread per finding so the implementing agent can answer where the
finding lives. The verdict is published, never withheld while a reviewer
runs.

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
- Run the applicable gates yourself (`go test -race`, `gofmt -l`, vet,
  lint — or the owning repo's equivalents). Do not repeat CI claims you
  did not run.
- **Check what the diff's suite mocks.** A green suite is not evidence about
  a seam the tests replace: a change whose central wiring sits behind a
  component the suite doubles has untested wiring, and the failure mode is
  invisible by construction. Ask whether any test exercises the real path.
  If none does, say so in the verdict — that is a finding about the evidence,
  not a preference about testing.
- Reproduce before asserting: run the mutation check, write the probe,
  exercise the edge case. Verification-before-completion applies to review
  claims in both directions — a bug is shown, not suspected, and so is its
  absence.
- Never review code you did not read. If the diff is too large for one
  pass, review it in passes and say so in the verdict.

## Findings

- Calibrated severity: **Critical** (must fix) / **Important** (should
  fix) / **Minor** (nice to have). Not everything is Critical. Acknowledge
  strengths specifically — accurate praise buys trust for the rest.
- Each finding states: file:line, what is wrong, why it matters, and the
  fix if not obvious.
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
- Threads anchor to the head commit at review time; a later push means the
  closure pass is a fresh review.

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

- **Addressed** — fixed, commit SHA (or follow-up PR) linked.
- **Rebutted** — the finding is wrong; the reasoning is stated. The
  reviewer concedes in-thread if persuaded.
- **Deferred** — real but not now: the reason, and the board entry that
  carries it.

A thread with no disposition is an open finding. The review round is not
complete while one exists.

## Closure

- After dispositions, the reviewer verifies fixes at the new head with a
  fresh review.
- The summary verdict is updated to reflect the dispositions. The verdict
  published on the PR **is** the record — a local review file is not.
- Merge-ready requires: declared scope implemented, applicable checks
  green, and no open Critical or Important threads.

## Deliberately not in this version

Per-team severity gates, automated AGENTS/charter conformance checklists,
and lane-ownership linting all wait for the use case that pays for them.
The charter and lens loading above is v1's enforcement; a use case brings
the mechanism.