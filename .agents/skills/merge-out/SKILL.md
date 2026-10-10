---
name: merge-out
description: Use when a wave's PRs are all gate-approved and the walk's QA checklist is ticked — merges the wave inside-out, one toolkit module at a time, re-pinning each consumer to the tag the previous merge produced, then cleans the environment and the wave's worktrees. Mechanical; a small model can run it from the brief.
---

# Merge out a wave

## Overview

A wave is a set of PRs built on pseudo-versions of each other. Merging it is a
pipeline: merge the innermost provider, wait for its tag, re-pin the next PR to
that tag, merge it, and so on outward to rpg-api, then web. Nothing here needs
judgment except reading a check result; everything that needs judgment (gates,
the walk) happened before.

## Preconditions — refuse to start if any is false

- Every PR in the wave has a published gate verdict of MERGE-READY at its
  current head (`gh pr view <n> --json headRefOid`; compare to the review).
  The verdict is a GitHub **comment review** (`gh pr review --comment`), not
  an issue comment: the merge permission classifier looks for a review at the
  head and refuses `gh pr merge` as "Merge Without Review" otherwise
  (rpg-toolkit#2006, 2026-10-11). A Minor-fix or re-pin commit after the
  verdict needs a fresh one-line head verdict in the same shape.
- The walk's QA checklist on the tracking issue is ticked, or the operator
  said QA passed, on the record.
- **No open gap.** A walk finding that blocks approval is not a QA pass. The
  wave's PRs stay open and un-merged; the gap gets its own tracking issue,
  short design and slices, merges first, and the wave's stack comes back up on
  the merged gap for a fresh walk. Only then does this skill run. (Ruled by
  KirkDiggler, 2026-10-09, after the action-information walk found the
  stranded-seat gap, rpg-project#548.)
- The brief names the PRs **in order** with their module path, e.g.
  `rpg-toolkit#1988 rulebooks/dnd5e/encounter → #1989 rulebooks/dnd5e/resolution → #1990 rulebooks/dnd5e/session → #1991 (stacked on #1990) → rpg-api#1085 → rpg-dnd5e-web#…`.
- The brief names who re-pins each consumer (the PR's builder, by agent name,
  or you).

## Hard rules

- Never push to `main`/`master`/`dev`. Never rebase a PR branch. Never
  force-push. Catch up only by `git merge origin/<base>` as a commit on top.
- Never `--no-verify`. Never hand-write a `replace` directive; pin with
  `go get <module>@<tag>` then `go mod tidy`.
- Toolkit PRs merge **one at a time**, and you **wait for the tag** before
  touching the next (the tagger runs on `main` after each merge).
- Do not change a toolkit PR's title: its prefix (`feat`, `feat!`, `fix`,
  `chore`) picks the tag bump.
- `gh pr merge --squash --delete-branch=false`. Branch deletion is part of
  cleanup, by exact name, after the whole wave.
- A PR that reports "no checks" on GitHub conflicts with its base: merge the
  base in (commit on top), never rebase.
- All commits and comments end with the team signature line only.

## Steps, per toolkit PR in order

1. **Re-pin** every provider module this PR depends on to the tag the
   previous step produced (skip on the innermost PR):
   `cd <module dir> && go get <provider module>@<tag> && go mod tidy`,
   `go test ./... -count=1`, lint, commit `chore(<module>): pin <provider> <tag>`,
   push. If the PR was stacked on another PR's branch, retarget it to `main`
   first: `gh pr edit <n> --base main`, then `git merge origin/main`.
2. **Mark ready** if it is still a draft: `gh pr ready <n>`.
3. **Wait for CI green**: poll `gh pr checks <n>` in the foreground (never
   `tail -f`). A red `gorelease` check means the title prefix and the API
   delta disagree; stop and report, do not retitle.
4. **Merge**: `gh pr merge <n> --squash --delete-branch=false`.
5. **Wait for the tag**: poll `git ls-remote --tags origin '<module path>/v*'`
   until a new highest version appears; record it. Expect: `feat`/`feat!`
   on v0.x → minor bump; `fix`/`chore` → patch.
6. Record `<PR> → <tag>` for the close-out comment.

## rpg-api, then web

- rpg-api PR: re-pin every toolkit module to its tag (`go get ...@vX.Y.Z`
  for each, then one `go mod tidy`), test, lint, commit, push, mark ready,
  wait for green, merge into `dev` (squash). `dev → main` is the operator's
  release gate; do not touch `main`.
- rpg-api-protos PRs (if any) merge **before** rpg-api; their tag is a
  generated-code commit on top of the merge, so read the tag's content, not
  the merge sha, when pinning.
- web PR: bump the protos pin to the tag if protos changed, mark ready,
  wait, merge into `dev`.

## Close-out, in this order

1. Post one comment on the tracking issue: the `<PR> → <tag>` table, the
   rpg-api and web `dev` shas, and the follow-ups each PR body listed.
2. Tear down the walk environment: `scripts/dev-env.sh clean local/<name>`
   from the `game-dev` root.
3. Remove **only this wave's** worktrees and branches, by exact name, in each
   repo: `git worktree remove <path>` then `git branch -D <branch>` — never
   a glob.
4. Report the table and anything that did not go as the brief said.

## When to stop and report instead of continuing

- A check is red for a reason other than an unresolvable pseudo-version on a
  draft. - A tag does not appear within ten minutes of a merge.
- The re-pin changes anything beyond `go.mod`/`go.sum`.
- A merge conflict touches a non-test `.go` file.

## Worked example (toolkit#1965 tier 2 D, 2026-10-09)

`#1988 encounter → v0.120.0`; re-pin `#1989 resolution` to it → merge →
`v0.67.0`; re-pin `#1990 session` to both → merge → tag; retarget `#1991`
(stacked on #1990) to `main`, merge `origin/main`, re-pin → merge → tag;
re-pin `rpg-api#1085` to all tags → merge to `dev`. No protos or web PR in
that wave.
