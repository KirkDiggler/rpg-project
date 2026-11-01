---
name: story
description: Use when starting, slicing or reporting any multi-slice line of work across the RPG repositories — the story issue, its slice sub-issues (one deliverable each, with what / does / how / done / checklist), the names everyone uses (story, slice S<n>, ruling R<n>), and the one place the operator looks to find the state of the work.
---

# Story and slices

A line of work the operator wants is a **story**. A story ships in
**slices**. The operator finds the state of the work by opening the story
and reading its sub-issues; nothing about the state lives anywhere the
story does not point to.

## The shapes

**Story issue** — one per story, in `rpg-project`. Its title names the
story in the operator's words. Its body holds:

- the operator's direction, recorded close to verbatim;
- the primitive: what the story adds to the game, in one paragraph;
- the ordered slice list, each line a link to its sub-issue;
- links to the design doc and its rulings (`ideas/<topic>/design.md`).

The story closes when its last slice closes. It never grows a requirement
list; a new want is a new slice or a new story.

**Slice sub-issue** — one per slice, a GitHub sub-issue of the story,
titled `<story short name> S<n>: <the deliverable in one line>`. A slice is
one deliverable: something the operator can look at and say "done" or "not
done". Its body holds, in this order:

1. **Story** — the parent issue and the slice number.
2. **What it is** — one paragraph.
3. **What it does when done** — the observable behaviour, in the operator's
   terms.
4. **How it works** — the mechanism, naming the rulings it rests on.
5. **Done means** — the bar, as bullets. Merged on tags, walked by the
   operator, the checklist ticked.
6. **Verification checklist** — `- [ ]` boxes, each "do this → expect
   that", written for the operator on :3030. This is the slice's Try-it; a
   PR's Try-it section copies from here, never the other way round.
7. **PRs** — every PR of the slice, repo and number, kept current.

A slice has no sub-issues of its own. Work that needs its own checklist is
another slice.

**Overview page** — one per story, `ideas/<topic>/overview.html`, beside the
design it maps. It is the architecture field note for the story: the
mechanism drawn (a figure a cold reader can follow without the prose), where
each piece lives in the stack, the numbers before and after, the slices and
their state, and what is open for the operator. It is a standalone page in
both themes, with a `<title>` that names the story and a
`<meta name="description">`. A push to `main` publishes it at
`https://kirkdiggler.github.io/rpg-project/architecture/<topic>/` and lists
it on `/architecture/` (`.github/workflows/publish-architecture.yml`). Update
it the day a slice moves, with the story table; its numbers are the story's
numbers, never a separate set.

## The names

- **Story** — the line of work; its issue number is the only id.
- **S<n>** — slice *n* of a story. Always written with the story's short
  name (`QA castle S2`), never bare, because every story has an S2.
- **R<n>** — a ruling in the story's `design.md` rulings table. Cited by
  its text as well as its number, because tables renumber.
- Any other letter prefix is retired. Earlier work used `E<n>`, `c<n>`,
  `W<n>` for slices; those are slices and get sub-issues when touched again.

## When to do what

- **Starting a story:** write the story issue first, before the design
  brief. The design skill's brief and walkthrough link to it.
- **A detour:** work the operator names mid-story that is not a slice (an
  upgrade, a cleanup, a fix elsewhere) is filed as its own small explicit
  issue in the owning repo and linked from the story as a prerequisite or a
  side note, in the same turn; the conversation returns to the story. A
  detour carried in chat is lost; one carried as a slice widens the story.
- **Slicing:** when the plan names the slices, create every sub-issue then,
  with sections 1–5 filled and section 6 as far as the plan knows. The
  checklist completes when the slice is built.
- **Building:** the builder's PR body carries `Part of <repo>#<story>` and
  `Closes <repo>#<slice>` for the slice it finishes (build-slice skill).
- **Waiting on the operator:** the question or checklist goes on the slice
  sub-issue (or the design PR), one line with the link in chat. The lead
  never asks "should S2 merge?" in chat without the sub-issue the operator
  can open and read.
- **Reporting:** a status to the operator names the slice by its sub-issue
  link and says which of its "done means" bullets hold.
- **Closing:** the slice closes when its "done means" bullets all hold,
  including the operator's walk. Merging the last PR does not close it.

## Mechanics

```sh
# create the sub-issue, then link it under the story
gh issue create -R KirkDiggler/rpg-project --title "<story> S<n>: <deliverable>" --body-file <file>
gh api -X POST repos/KirkDiggler/rpg-project/issues/<story>/sub_issues -F sub_issue_id=<slice issue id>
# the id is the issue's database id: gh api repos/.../issues/<n> --jq .id
```

The story's sub-issue summary (`total` / `completed`) is the progress bar;
no project board is required.

## The hazard

A slice list that lives as prose in the story's body, with "one issue each"
written beside it, is the shape that produces four PRs with no single place
saying what the slice is, how it works or what verifies it. The operator then
cannot find the slice by number and the lead asks "should S2 merge?" in chat.
A rule in prose is not a mechanism; the sub-issue is.
