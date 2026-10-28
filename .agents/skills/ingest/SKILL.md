---
name: ingest
description: Use when turning captured notes and committed run records into knowledge — the judgment step between `ingest.mjs collect` and `ingest.mjs apply` in game-dev's team-memory tool. Routes each entry to the lowest level that contains everything it binds, picks its section, writes the dated and linked line, and declines with a reason. The tool makes no classification judgment; this skill is the whole of it, so the routing can be reviewed and argued with like any other rule.
---

# Ingest — the judgment

`ingest.mjs collect` gathered a batch; `ingest.mjs apply` will write it. Between
them, you decide where each entry belongs and what line it becomes. The tool
makes no classification judgment. This file is the whole of the judgment, so the
routing can be reviewed and argued with like any other rule. The tool and its
commands are documented in `game-dev/tools/team-memory/README.md`.

The rules this applies are the team-memory standing rules 4–8 and 13–15:
[`ideas/team-memory/design.md`](../../../ideas/team-memory/design.md#standing-rules).

## Read

The batch file `envs/local/notes/batches/<batch>.json` holds:

- `notes`: every inbox note, verbatim.
- `runs`: every committed run record without a `processed:` line, verbatim.
- `routes`: the file each entry's scope implies. A scope is a hint about where
  the writer stood, not a verdict.
- `scopeMisses`: scopes the vocabulary does not know. They route to
  `team:cross-team`; say so in the report so the vocabulary gets fixed.
- `targets`: the current contents of every implied file and of each level
  above it, with `knowledge` (the level exists) and `leaf` (no Go module sits
  beneath it).

Open the code an entry names before you place it. A line about a module is
checked against that module, not against the note's memory of it.

## Place each entry

1. **Find what it binds.** Name the modules, packages or seams the entry
   constrains. Craft that binds no code (how a Team reviews, walks,
   dispatches, gates) binds nothing below rpg-project.
2. **Route to the lowest level that contains everything it binds.** One module:
   that module's `knowledge/`. Two modules of one level: that level's
   `knowledge/`. Layers of a repo: the repo root. Two repos, or no code at all:
   a role's `memory.md` in rpg-project (`team:<role>`; `cross-team` when no
   single role owns it). Never write above the lowest binding level; never
   park a general rule in one module because the writer stood there.
3. **Pick the section.**
   - An operator ruling, a choice made between options → **Decisions**, with
     its because.
   - A fact about the code or the world as it is → **Findings**.
   - A clash with an existing line, or between two entries in the batch →
     **Friction**, naming both. Never adjudicate; never rewrite the old line.
   - A question, a direction to test against, a thing to measure → **Open**.
   - In a role `memory.md`: **Lessons**, **Friction** or **Open**.
4. **Dedupe against what is there.** An exact duplicate is declined, naming the
   line it duplicates. A reworded duplicate that says the same thing is
   declined the same way. One that shifts the meaning, even slightly, becomes
   a Friction line naming both; prefer Friction to a silent drop. A later note
   that says it supersedes or completes an earlier one in the batch is merged
   into one line crediting both.
5. **Decline with a reason** only what binds nothing, duplicates a line, or is
   already carried by a standing rule. Every note and run in the batch is
   either placed or declined; `apply` refuses a proposal that drops one.

## Write the line

Each line is one line, dated, linked; every evidence string appears in it.

- Decisions:
  `<date> · <the words> · options: <the options it closed> · because: <why> · <link> · upheld: <links, or none yet>`.
  The date is the ruling's date. *Upheld* names later PRs, reviews or rulings
  that reached for the old way and were stopped; testimony without links says
  so rather than inventing them. The because is what the decision depends on,
  so that a later question can ask whether it still holds.
- Findings, Friction, Open in a `decisions.md`: `<date> · <the words> · <link>`.
  A finding that needs more than a line gets a case file
  (`<level>/knowledge/findings/<slug>.md`, present tense, evidence linked) and
  a Findings line that links it.
- A role `memory.md` line follows that file's form:
  `<the lesson, in given/because form> — <date> (note <id>, <link>)`.

Write the words in the present tense and in the writer's sense. Keep
attribution ("the operator, 2026-10-27") when the entry is a ruling; it is the
evidence that it was ruled.

## Write the proposal

`envs/local/notes/batches/<batch>.proposal.json`:

```json
{
  "batch": "<batch>",
  "lines": [
    { "target": "<repo>/<path>", "section": "Decisions|Findings|Friction|Open|Lessons",
      "line": "...", "evidence": ["<url or ref that appears in the line>"],
      "from": ["<note id>", "<repo>:<run path>"] }
  ],
  "cases": [ { "target": "<repo>/<level>/knowledge/findings/<slug>.md", "content": "...", "from": ["..."] } ],
  "maps": { "<repo>/<level dir>": "<the PR where a person draws that level's map>" },
  "declined": [ { "from": "<note id or run>", "reason": "..." } ]
}
```

`target` is `<repo>/<level>/knowledge/decisions.md` or
`rpg-project/docs/teams/roles/<role>/memory.md`; nothing else is accepted, and a
`map.md` never is. A level that does not exist yet is created by `apply`:

- a **leaf** level (no Go module beneath it) gets a README and `decisions.md`;
- a **non-leaf** level is created only when `maps` names the PR where a person
  draws its map; otherwise its lines are held at the nearest existing level
  above, marked "routed from", with a Friction line saying the map is missing.

Then run `ingest.mjs apply <proposal> --team <role>` with the Team you are
signing as. The knowledge PRs it opens are reviewed by the operator; `collect`
archives the batch's notes once they merge and returns them to the inbox if a
PR closes unmerged.

## The hazard

The judgment is the only place a general lesson can get trapped in one
module's directory, or a module's quirk can be parked at a seam level. Both
are mirrors (design walkthrough, "The two level hazards are mirrors"). Read
what the entry binds before reading where the writer stood.
