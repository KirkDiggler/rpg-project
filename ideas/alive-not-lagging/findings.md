# Findings ledger — alive, not lagging

Story rpg-project#879. Every entry: what was measured, the head, the keep,
the command, the numbers, what it means, and whether it still holds. Append;
never edit a number in place. A later entry supersedes an earlier one by
name.

The garrison keep: keepgen, 10,000 hexes, seed 1, 99 garrison, 20 of them on
routes, `posts-asleep` is the same keep with the posts listening for a sound
instead of asking the clock every round. One player paces four cells beside
the party start, one walk per op; two ops in three raise the world clock.

## F1 — the save is half site geometry

2026-11-07 · rpg-toolkit main `3972f948` · garrison keep, default, one Move in.

Command: `BenchmarkGarrisonMove/default` with the blob marshalled after
`NewEncounter` and written to a file; shares by top-level key of
`EncounterData` JSON.

| shape | bytes | share | changes |
|---|---|---|---|
| field (regions, placed, walls, presentations) | 497,777 | 51% | never after launch |
| perception, per observer | 230,099 | 24% | when a view changes |
| journal, per observer | 123,281 | 13% | when a view changes |
| log | 84,828 | 9% | append only |
| members | 19,130 | 2% | the member touched |
| doors, clock, sweeps, ever_members | ~20,000 | 2% | small |
| total | 975,214 | | |

Means: the first shape to leave the save is the field; the roster that
actually changes every verb is under 40 KB. Holds.

## F2 — the behavior cutover adds its configuration to every member

2026-11-07 · toolkit#2293 head `c1cbf623` vs main `3972f948` · garrison keep.

Command: `go test -run '^$' -bench BenchmarkGarrisonMove -benchtime 10x -benchmem .`
in `rulebooks/dnd5e/encounter`, with `blob-bytes` reported after setup.

| keep | main blob | head blob |
|---|---|---|
| default | 975,681 | 1,048,884 |
| posts-asleep | 1,001,147 | 1,066,185 |

Means: +73 KB for 119 behaving members, about 615 bytes each, because each
member's materialized recipe and tables are saved beside its progress. The
runtime already digests the configuration; storing it once by digest is a
dedup, not a shape. Holds.

## F3 — the behavior cutover costs a third of a Move

2026-11-07 · same heads and command as F2, 10 iterations, two runs each.

| keep | main Move | head Move |
|---|---|---|
| default | 343–351 ms | 444–467 ms |
| posts-asleep | 251 ms | 356–363 ms |

Means: see F4 for where. Holds.

## F4 — the composer is free; the wake rebuilds sight twice as often

2026-11-07 · same heads · `BenchmarkGarrisonMove/default`, 20 iterations,
`-cpuprofile`, read with `go tool pprof -top -cum` and `-peek`.

| per Move | main | head |
|---|---|---|
| total | 347 ms | 445 ms |
| sight rebuilt inside the wake (`wakePass → refreshSight`) | 97 ms | 213 ms |
| stepping machines in the wake (`wakePass → stepMachine`) | 127 ms | 117 ms |
| the player's own sight after moving (`Step → refreshSight`) | 145 ms | 146 ms |
| `behavior.(*Composer).Decide`, all of `mind/behavior` | 0 | 1.5 ms |
| `IsLineOfSightBlocked` and below | 250 ms | 357 ms |

Means: all of the new latency is sight rebuilt once more per creature
opportunity; the machines cost the same; three quarters of a Move is line of
sight, ray by ray. The design's law that a step costs no perception follows
from this entry. Holds.

## F5 — a Move allocates 170 MB

2026-11-07 · main `3972f948` and head `c1cbf623` · `-benchmem`, and
`-memprofile` on the head read with `-sample_index=alloc_space`.

| keep | main | head |
|---|---|---|
| default | 169 MB, 984k allocs per Move | 188 MB, 1.2M allocs |
| posts-asleep | 95 MB, 712k allocs | 111 MB, 915k allocs |

About a fifth of CPU samples are the garbage collector. The largest in-loop
allocators: `spatial.(*AxialHexGrid).GetLineOfSight` through
`CanonicalBoundaryRay` (587 MB over 10 ops), `field.blockingDoorFootprints`
(364 MB), `GetNeighbors` (452 MB cumulative), `journal.(*Journal).All` (203
MB, the whole journal copied per verb by `world/graph.StateFor` and
`holdings.fold`).

Means: rays and door footprints allocate per cell per observer; the journal
is folded whole on every verb. Both are mechanical. Holds.

## F6 — the live keep, as the earlier stories left it

Reference numbers from rpg-project#753 and #766, not remeasured here: live
keep plain Move 300 → 165 ms after server-at-scale S7 and the-world-runs S4;
raise 4 s → 1.25 s; the 100 ms bar left open; the save 1.3 MB per verb, of
which JSON is about 65 ms (#770). This story's live numbers land under R5's
harness and supersede this entry.

## F7 — the player's own step is 41 ms; the raise is the rest

2026-11-07 · rpg-toolkit main `3972f948` with the S0 benchmark
(rpg-toolkit#2298 at `ecac6530`) · garrison keep, both variants.

Command: `go test -run '^$' -bench BenchmarkGarrisonMove -benchtime 10x .`
in `rulebooks/dnd5e/encounter`.

| keep | ns/op | walk step | raise step | raises/op | blob | growth/op | allocs/op |
|---|---|---|---|---|---|---|---|
| default | 432 ms | 41.4 ms | 421.6 ms | 0.7 | 969,186 | 2,950 | 984k, 169 MB |
| posts-asleep | 298 ms | 41.1 ms | 232.6 ms | 0.7 | 994,652 | 1,843 | 712k, 95 MB |

Means: a step that does not raise the clock, the walk and the player's own
sight, costs 41 ms on both keeps and is already under the 100 ms bar; a step
that raises pays nine tenths of its time for the garrison. The answer the
design moves ahead of the raise is the 41 ms step. Holds.
