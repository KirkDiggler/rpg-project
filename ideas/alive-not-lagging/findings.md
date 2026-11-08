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

Merged to rpg-toolkit main as `058c7f11` with the review's three fixes
(pure round read, growth named as net change after retention, sub-ms
averages). Rerun at that head, 5x: default walk 36.8 ms, raise 356 ms,
growth 1,866 bytes per op; posts-asleep walk 37.4 ms, raise 207 ms, growth
−287 bytes per op (retention trimming the log on a quiet keep). The S0
numbers above stand; the harness is now on main for every later entry.

## F8 — a built keep weighs 6 MB on the heap

2026-11-07 · rpg-toolkit main `3972f948` · keepgen keeps, seed 1, default
monster density (about one creature per hundred hexes), built with the
benchmark's capabilities.

Command: a throwaway test reading `runtime.MemStats.HeapInuse` after two
forced GCs before and after `NewEncounter`.

| keep | hexes | members | live encounter heap |
|---|---|---|---|
| garrison (99 with orders, 20 on routes) | 10,000 | 100 | 5.9 MB |
| same density, nobody given orders | 10,000 | 100 | 5.1 MB |
| garrison | 30,000 | 298 | 16.0 MB |

Means: a live keep held in the API process costs single-digit megabytes; the
orders and routes add under a megabyte. A hundred live sessions of the
largest keep is 1.6 GB; a handful is nothing. The 1 MB JSON record is a
sixth of the built size, which is the rebuild the cache avoids. Holds.

## F9 — rebuilding per verb costs 2.4× the CPU of a live instance, for 5 MB a game

2026-11-07 · rpg-toolkit main `3972f948` with the simulation benchmark
(rpg-toolkit#2301 at `05b4ec36`) · garrison keep, 1, 8 and 32 games served
round-robin by one process, one verb per Step, one op = the four-cell pace.

Command: `go test -run '^$' -bench BenchmarkSessionStyles -benchtime 12x .`
in `rulebooks/dnd5e/encounter`.

| style | games | per verb | resident heap | GC pause per op | allocs per op |
|---|---|---|---|---|---|
| rebuild (unmarshal, LoadEncounter, Step, marshal) | 1 | 231 ms | 2 MB | 8.4 ms | 4.36M, 616 MB |
| live (Step, marshal the checkpoint) | 1 | 97 ms | 6 MB | 3.0 ms | 983k, 194 MB |
| live, no checkpoint | 1 | 88 ms | 6 MB | 1.7 ms | 958k, 163 MB |
| rebuild | 8 | 225 ms | 8 MB | 4.2 ms | 4.35M |
| live | 8 | 95 ms | 41 MB | 0.6 ms | 986k |
| rebuild | 32 | 245 ms | 31 MB | 1.9 ms | 4.61M |
| live | 32 | 118 ms | 164 MB | 0.2 ms | 1.25M |

Means: the rebuild is 134 ms of every verb, 2.4 times the live cost and 4.4
times the allocations, bought back for about 5 MB of resident heap per live
game. The checkpoint of the whole record is 10 ms of a live verb. Thirty-two
resident keeps slow a live verb by a fifth, the collector scanning 164 MB.
The simulation has no Redis round trip; the server pays that on top of the
rebuild line. Holds.

F9, the other end of the range (rpg-toolkit#2301 at `c2d772c9`, after its review):
the server's Move loads once per path, not per cell, so a four-cell pace sent
as one verb is one rebuild, four steps and one save. Measured as
`rebuild-walk`: 492 ms per Move at one game, 494 at eight, 562 at thirty-two,
against live's 380 to 472. So a one-cell verb (a step, a bump, a swing) pays
2.4× and a four-cell Move pays 1.3×; the rebuild is 110 to 134 ms of CPU per
verb either way. The ratio is engine-only: settlement, numbering, fan-out and
the session save are paid in both styles. Rebuild's resident heap is the
at-rest figure; a verb in flight holds one built encounter besides, so peak
is the blobs plus one keep per concurrent verb.

The simulation is on rpg-toolkit main as `707f8f1e` (`BenchmarkSessionStyles`).

## F10 — who gets their sight rebuilt, and why it costs

2026-11-07 · rpg-toolkit main `707f8f1e` · garrison keep, default, nine
ops (six raises) of the four-cell pace, the rebuild probe counting every
observer list `rebuildPercepts` is handed.

| refresh | calls | observers per call | distinct | pattern |
|---|---|---|---|---|
| the player's own step | one per step, four per op | 19 | 19 | the same nineteen guards who see the player's two cells, every step |
| the wake | one per raise | 78 | 78 | twenty patrol legs reach nearly the whole garrison, every raise |

128 observer rebuilds per op. A stationary observer already skips its sweep
when its sweep key (own cell, sight range, opacity, hidden set) is unchanged,
so the cost is the movers: the player sweeps from every cell it steps to and
each patrol sweeps from every leg, and a sweep casts a ray to every cell of
the sight disk that is not yet known, which in a keep is most of the disk
behind walls, again from every cell. Members never block sight, so what a
cell sees is a function of the site, the doors, the props and the observer's
concealment knowledge, and nothing else. Holds.

## F11 — a cross-pass visible-cells cache halves the Move

2026-11-07 · rpg-toolkit main `707f8f1e` plus a spike: the floor an observer
sees from a cell is cached across passes, keyed by the existing sweep key,
and the whole cache is dropped when a signature of door states and opaque
entity cells changes · garrison keep, 40 iterations.

| keep | before | after | walk step | raise step | allocs per op |
|---|---|---|---|---|---|
| default | 432 ms | 229 ms | 41 → 18 ms | 422 → 251 ms | 984k → 729k |
| posts-asleep | 298 ms | 129 ms | 41 → 18 ms | 233 → 103 ms | 712k → 466k |

Over sixty ops: 469 hits, 114 misses, all misses in the first nine ops (one
lap of every route); 114 entries holding 69,637 cells, about 1 MB as
positions and 28 KB as bitsets. The sight-related tests pass; the module's
full suite is the oracle and runs at the time of writing. Provisional until
it passes.

Means: the player's own step drops to 18 ms and the raise to a quarter of a
second, with no change to what anyone sees. The cache is a property of the
site plus its doors and props, which is the site tier of the three-tier
shape: computed once per site version, shared by every game on it, with the
door and prop signature as the invalidation. Provisional.

F11 oracle: the whole encounter module passes with the spike in place
(`go test -count=1 ./...`: root, castletest, dungeonspec, keepgen, replay,
commands, scenarios, all ok). What anyone sees is unchanged. F11 holds.

## F12 — S2 on main: what a cell sees is cached, not recast

2026-11-07 · rpg-toolkit#2305 merged to main as `c8cb8e53` · garrison keep.

Command: `go test -run '^$' -bench BenchmarkGarrisonMove -benchtime 40x .`
in `rulebooks/dnd5e/encounter`, at the review head `a3a8e1de` (the merge is
that head squashed).

| keep | Move | walk step | raise step | allocs per op |
|---|---|---|---|---|
| default | 432 → 235 ms | 41 → 18.6 ms | 422 → 266 ms | 984k → 722k |
| posts-asleep | 298 → 130 ms | 41 → 18.1 ms | 233 → 103 ms | 712k → 466k |

The review found the separate door-and-prop epoch redundant: the sweep key
already carries the pass's opacity, which fingerprints doors, placed
footprints, opaque entities and obscuring areas, so the key is the
soundness and the opacity only frees orphaned entries. Under R4 an entry is
instance state; the site tier could hold only the authored site's answer.
The module's full suite passes at the merge. Holds; supersedes F11's
provisional numbers.

S2 merged out: session v0.166.1 (rpg-toolkit#2307, `b5f8f75e`), rpg-api dev `65f03a8c` (rpg-api#1192). The castle walk of the door invalidation is the operator's on :3030.

## F13 — routes on demand: 930 route searches per op, none read

2026-11-07 · rpg-toolkit main `c8cb8e53` (S2 in) plus a spike · garrison
keep, 40 iterations.

Every woken creature's view searched a route toward and a route away from
every member it could see, before any row was picked: 117 views and 930 A*
searches per op in the garrison keep, and not one of them read, because a
post's table holds and a patrol's table walks a route of cells. The spike
searches member routes only when a row of the member's table can read one
(a selector word on a movement verb, a search, or an `enemy` band measured
in steps); a member with no table keeps every route, since something this
package cannot see drives it.

Command: `go test -run '^$' -bench BenchmarkGarrisonMove -benchtime 40x .`

| keep | Move | raise step | walk step | allocs per op |
|---|---|---|---|---|
| default | 233 → 161 ms | 255 → 139 ms | 18 → 20 ms | 728k → 551k |
| posts-asleep | 128 → 127 ms | 100 → 96 ms | 18 → 19 ms | 466k → 450k |

Means: the raise with the whole garrison awake drops by nearly half again;
posts asleep barely move, because sleeping posts build no view. Against
F7's 432 ms the default Move is now 161 ms. The root package passes; the
module suite runs. Routes are the second value computed eagerly per visit
that a row may never read, which is R4's facts-on-demand half. Provisional
until the suite passes.

F13 holds: rpg-toolkit#2308 merged to main as `8e306531` after its review narrowed the gate to its readers (toward or dash naming a member, and the enemy step bands) and named the third case on the view's route fields. Module suite green at the merge.

S3 merged out: encounter v0.181.0, session v0.166.2 (rpg-toolkit#2309, `c9b36040`), rpg-api dev `3ed5338f` (rpg-api#1194). The castle checks on #888 are the operator's on :3030.

## F14 — four folds kept until their inputs change

2026-11-07 · rpg-toolkit main `8e306531` (S3 in) plus rpg-toolkit#2310 at
`6f6847d1` · garrison keep, 40 iterations.

With the harness reading the round through a cheap accessor instead of the
whole record, the allocation profile of a Move is honest, and four things
recomputed on every ask stand out: the blocking-door slice rebuilt on every
crossing check (20 MB a Move), the holdings fold and a member's known facts
each copying the whole world journal per question, the faction graph
refolded per stance question (17 MB), and the host's participation
assessment asked anew by every woken creature (23 MB, a hundred assessments
of one roster per raise). Each now reads back until its input changes: the
door slice by each door's reading, the three folds by the append-only
journal's length, participation for one wake pass.

Command: `go test -run '^$' -bench BenchmarkGarrisonMove -benchtime 40x .`

| keep | Move | raise step | allocs per op | bytes per op |
|---|---|---|---|---|
| default | 161 → 134 ms | 139 → 105 ms | 551k → 408k | 80 → 44 MB |
| posts-asleep | 127 → 123 ms | 96 → 88 ms | 450k → 368k | 59 → 39 MB |

Means: a quarter of the allocations and a sixth of the time, mechanically,
with what anyone sees or decides unchanged. Against F7 the default Move is
432 → 134 ms. Provisional until the suite and the review land.

F14 holds: rpg-toolkit#2310 merged to main as `ba074f9d`. The review found the door mask written in place through the partition pass's shallow copy (a shut door served as open after a load) and a mover dropped mid-walk inside a wake pass kept walking; both fixed and pinned before the merge. Module race suite green.

S3b merged out: encounter v0.181.1, session v0.166.3 (rpg-toolkit#2311, `6e265ba5`), rpg-api dev `8d8c4ef3` (rpg-api#1195). The castle checks on #889 are the operator's on :3030.
