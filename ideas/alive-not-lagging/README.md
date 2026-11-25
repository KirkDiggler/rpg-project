# Alive, not lagging

Story rpg-project#879 · lane #880 · [design.md](design.md) is the law ·
[findings.md](findings.md) is the ledger.

## What this folder holds

- **design.md** — the shape, the law, the rulings table and what is open.
  Changes with the operator's rulings on the lane.
- **findings.md** — every measurement this story makes, appended in order,
  each with its head, keep, command and numbers. A finding is never edited;
  a later one supersedes it by name.
- **overview.html** — the architecture field note for the story, published
  to Pages when a slice moves (story skill). Not yet written.
- **plan.md** — added beside the design once R1–R5 are ruled.

## How to add a finding

1. Run the measurement on a named head of the owning repo, with the command
   written out, on the garrison keep unless the finding is about the castle.
2. Append `## F<n> — <one line>` to `findings.md` with date, head, keep,
   command, a table for the numbers, one "Means:" sentence and "Holds" or
   "Superseded by F<m>".
3. If the finding changes the story table on #879, change it the same day,
   with the same numbers.

## The harness

Until S0 lands, the numbers come from `rulebooks/dnd5e/encounter`:

```sh
go test -run '^$' -bench BenchmarkGarrisonMove -benchtime 10x -benchmem .
go test -run '^$' -bench 'BenchmarkGarrisonMove/default' -benchtime 20x -cpuprofile cpu.prof .
go tool pprof -top -cum -nodecount=40 encounter.test cpu.prof
```

S0 makes the benchmark report its phases and the blob, and the live bench
(rpg-project#740's harness) report the answer and the first beat.
