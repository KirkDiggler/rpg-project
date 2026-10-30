# knowledge — rpg-project

The knowledge of this level: what binds the workspace as a whole and no single
repository. rpg-project holds no Go modules, so this level has no module map;
its seams are the repositories, drawn in [`docs/architecture.md`](../docs/architecture.md)
and governed by [`CLAUDE.md`](../CLAUDE.md).

What lives here:

- `runs/` — one run record per work PR that binds this level, plus
  [`runs/TEMPLATE.md`](runs/TEMPLATE.md), the template every repository's run
  records copy. A run record has four sections, in order: shipped, decided,
  found, mistaken. One line per entry, every line linked.
- `decisions.md` — one line per ruling that binds the workspace: the date, the
  words, the options closed, the because, the link, and where it was upheld.
  The ingest is its only writer; it arrives through a reviewed PR.
- `findings/` — case files, present tense, evidence linked, when a finding
  needs more than a line.

How to contribute: write a run record on your PR, or drop a note
(`game-dev/tools/team-memory/README.md`). The ingest routes each entry to the
lowest directory in the owning repository that contains everything it binds;
only what binds the whole workspace lands here. The standing rules are
[`ideas/team-memory/design.md`](../ideas/team-memory/design.md).
