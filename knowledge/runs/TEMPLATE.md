# Run — <YYYY-MM-DD> — <story short name> S<n> <topic>

Slice [<repo>#<slice>](https://github.com/KirkDiggler/<repo>/issues/<slice>) of story [rpg-project#<story>](https://github.com/KirkDiggler/rpg-project/issues/<story>). Base: `<repo>` `<base branch>` at [`<sha>`](https://github.com/KirkDiggler/<repo>/commit/<sha>).

One line per entry. Every line ends with a link: a PR, an issue comment, a commit, or a design section anchor. This file rides on the work PR, under the `knowledge/runs/` of the lowest directory that contains everything the PR changed. The ingest reads it once and adds a `processed:` line at the top; nobody else edits it after merge.

## Shipped

What landed, with the PR. One line per deliverable the slice issue names.

- `knowledge/` at three levels with README, map and an empty decisions file — [rpg-toolkit#2136](https://github.com/KirkDiggler/rpg-toolkit/pull/2136).

## Decided

Every judgment a reader could have made differently: the words, the options closed, the because, and the rule or ruling it rests on. A decision the operator made in conversation names them and the date.

- `examples/*` modules are drawn at the root in their own group, because every module appears in exactly one map and `examples/` has no map of its own; the tagger already skips them — [design rule 15](https://github.com/KirkDiggler/rpg-project/blob/main/ideas/team-memory/design.md#standing-rules).

## Found

Facts the run established that were not known before it: measurements, drift, a doc that disagrees with the code, a seam described two ways. Present tense, numbers where there are numbers.

- The old overview diagram draws 24 nodes for 29 modules; 7 modules are missing and 2 nodes name deleted modules — [overview at base](https://github.com/KirkDiggler/rpg-toolkit/blob/c355102d/docs/architecture/overview.md#L33-L82).

## Mistaken

What went wrong during the run and what let it through, or `none observed`. A correction made before the PR opened still belongs here; the fix is the evidence.

- The first node extractor read any quoted label, which would have taken a subgraph title as a module; the check now skips `subgraph` lines and a fixture proves it — [rpg-toolkit#2136](https://github.com/KirkDiggler/rpg-toolkit/pull/2136).
