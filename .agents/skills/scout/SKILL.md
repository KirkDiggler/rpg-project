---
name: scout
description: Use when a fact about the code, the content or the issues has to be checked before work is set up or a slice issue is written — how to ask a small, cheap session one closed question and get quoted evidence back.
---

# Scout

Work is set up on checked facts. A **scout** is a small read-only session
that checks one. It is the cheapest session there is, and it stays cheap
only while its question is closed and its reading is narrow.

## What a scout is for

- A closed question of fact: "is this line in the served castle?", "does
  this function exist, and what is its signature?", "what state is this
  pull request in?".
- Never a judgment: "what is missing to author this?" stays with the lead,
  or becomes several closed questions.

## The brief

- **One place to look.** A directory, a file, or one command. When the
  place is not known, that is the first question: "which file holds X?".
- **Quoted answers.** Ask for the lines with `path:line`, or the command
  and its output. "Not found" is an answer; a summary is not.
- **Filtered lists.** Ask for a count, or for the rows that match. An
  unfiltered list fills the scout and the reader.
- **A word limit and the words "read-only".**
- **Text, not pages.** A page built for the operator is heavy to read. Point
  a scout at the issue, the file or the command that holds the same facts.

## Running one

- Pin the smallest model that can quote, explicitly (working agreements,
  section 9). Use a read-only or search-only agent type where the harness
  has one; it starts lighter than a general one.
- Low effort is enough to quote. Raise it only when the question needs
  reasoning, and say why.
- Independent questions go to separate scouts at the same time.

## Using the answer

- Read the quoted lines, not the scout's words about them.
- A checked fact goes into the slice issue's "Rests on" with the command or
  `path:line` that proved it, so the builder can run the same check.
- When the same question is asked a second time, write the script that
  prints the answer. A script costs nothing to wait on.
