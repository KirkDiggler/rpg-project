---
name: story
description: Use when starting, slicing or reporting any line of work across the RPG repositories — the week goal, its feature stories, their technical slice issues (written for the builder: outcome, rests on, path, tests, done means, leave alone), the playtest slice that closes a feature, the holding zone for new wants, and the names everyone uses (story, slice S<n>, ruling R<n>).
---

# Week goal, stories and slices

Work is grouped by what a player or an author gains, in three levels:

- a **week goal** holds the **feature stories** being worked;
- a feature story ships in **slices**, and its last slice is a **playtest**;
- a new want waits in the **holding zone** until the operator pulls it in.

The week goal and the story are the operator's pages. A slice issue is the
builder's brief. Nothing about the state of the work lives anywhere these do
not point to.

## The shapes

**Week goal** — one issue in `rpg-project`, the operator's page for the
week. Its body holds:

- the operator's direction, recorded close to verbatim;
- a table of its feature stories, each a sub-issue, with one line of state;
- features waiting behind these, as links;
- upkeep that is not a feature (slow tests, a stale table), as links;
- the date it started.

Its rows are fixed once it is filed; a new want goes to the holding zone. It
carries a size only when the operator gives one. Velocity is read from what
closes, never guessed.

**Story issue** — one per feature, a sub-issue of the week goal. Its title
names what a player or an author has or can do when it is done, in the
operator's words. It is never named by a lane ("engine work") or by a
mechanism ("a line gives..."). Its body holds:

- the operator's direction, recorded close to verbatim;
- the feature, in one paragraph;
- a table of its parts and the state of each, updated the day a part moves:
  what is played, what is on dev, what is open, what is not started;
- the ordered slice list, each line a link to its sub-issue;
- found, not yet filed: rough edges seen while playing;
- links to the design doc and its rulings (`ideas/<topic>/design.md`), and
  its open rulings, each an issue labelled `ruling` in the repository that
  owns its consequences (design skill);
- **shown**: the frames and clips of what has been played, newest first, each
  with its date and its slice. A public repository shows only what its
  content licenses allow, which is the operator's call; the rest lives in the
  private store and is linked.

The story closes when the operator has played it. It never grows a
requirement list; a new want goes to the holding zone.

**Carried by a session.** The story is the session's page. A session working
on it starts there, keeps its table and its shown list current the day a part
moves, and reports there. What a session knows that the page does not is lost
with the session.

**Slice sub-issue** — one per slice, a GitHub sub-issue of the story,
titled `<story short name> S<n>: <the deliverable in one line>`. It is
technical and has one reader: the builder, working on a smaller model from
this issue alone. Its body holds, in this order:

1. **Story** — the parent issue and the slice number.
2. **Outcome** — one sentence.
3. **Rests on** — every fact about the code the slice assumes, numbered
   `F1`, `F2`, each with the command or the `path:line` that proves it. The
   builder checks these before writing code.
4. **Path** — the files to touch, and only those; the steps in order, with
   exact names and signatures.
5. **Tests** — the named tests, written first, and why each fails before
   the change.
6. **Done means** — commands that print pass.
7. **Leave alone** — what the slice must not do or touch, and what would
   never be accepted (the norm, not only the structure).
8. **From build-slice** — the sections of that skill that apply to this
   slice.
9. **Report** — where the rests-on line, any gap report and the run record
   go.
10. **PRs** — every PR of the slice, repo and number, kept current.

A slice issue is ready when its builder could build it without asking a
question. A path the lead cannot write with exact names is a sign the work
is not understood yet: a scout goes first (scout skill), not a builder.

A slice has no sub-issues of its own. Work that needs its own brief is
another slice.

**Playtest slice** — the last slice of every story, titled
`<story short name> playtest`. Its body is the operator's checklist: `- [ ]`
boxes, each "do this → expect that", written for the operator on :3030. A
PR's Try-it section copies from here, never the other way round.

**Holding zone** — a new want is its own issue with a `feature:<name>`
label that says what it brings. The label is created with the first want
for that feature. A want is pulled into a week goal when the operator
chooses; it never widens a story in flight.

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
- **Setting up a slice:** check the premise before writing the issue. Every
  belief the slice leans on is written into "Rests on" with its proof; a
  belief nobody has checked is a question for a scout. Open choices go to
  the operator one at a time and are closed before the issue is filed.
- **Slicing:** create each slice sub-issue when its work is set up, complete
  enough to build from. The playtest slice is created with the story.
- **Building:** the builder's PR body carries `Part of <repo>#<story>` and
  `Closes <repo>#<slice>` for the slice it finishes (build-slice skill).
- **A reported gap:** a builder that meets something its slice issue does
  not give posts a gap report on the slice issue and ends its turn with a
  `GAP` line carrying the link (build-slice skill). A **find** becomes its
  own small explicit issue in the owning repo, linked from the slice. A
  **call** goes to the operator first and becomes an issue once it is made.
  A report the lead cannot file an issue from is a fault in the form; fix
  the form.
- **A new want:** file it in the holding zone with its `feature:<name>`
  label, in the same turn, and return to the work.
- **Waiting on the operator:** a question about the work goes on the story; a
  ruling goes as an issue labelled `ruling` (design skill). Chat carries one
  line with the link. The lead never asks "should S2 merge?" in chat without a
  page the operator can open and read.
- **Reporting:** a status to the operator names the slice by its sub-issue
  link and says which of its "done means" bullets hold.
- **Closing:** a slice closes when its "done means" commands all pass and
  its PRs are merged. The story closes when the playtest slice closes: the
  operator has played it.

## Mechanics

```sh
# create the sub-issue, then link it under the story
gh issue create -R KirkDiggler/rpg-project --title "<story> S<n>: <deliverable>" --body-file <file>
gh api -X POST repos/KirkDiggler/rpg-project/issues/<story>/sub_issues -F sub_issue_id=<slice issue id>
# the id is the issue's database id: gh api repos/.../issues/<n> --jq .id

# a ruling that is the operator's waits as an issue, where its consequences live
gh label create ruling -R KirkDiggler/<repo> --description "A decision that is the operator's: one issue per open ruling, in the repository that owns it."
gh issue create -R KirkDiggler/<repo> --label ruling --assignee KirkDiggler --title "Ruling R<n>: <the question>" --body-file <file>
# the operator's list, across every repository
gh search issues --owner KirkDiggler --label ruling --state open
```

The story's sub-issue summary (`total` / `completed`) is the progress bar;
no project board is required.

## The hazard

A slice list that lives as prose in the story's body, with "one issue each"
written beside it, is the shape that produces four PRs with no single place
saying what the slice is, how it works or what verifies it. The operator then
cannot find the slice by number and the lead asks "should S2 merge?" in chat.
A rule in prose is not a mechanism; the sub-issue is.
