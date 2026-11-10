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

## F15 — on the live server path the garrison Move is 1.8 s, and the stream waits for the save

2026-11-08 · rpg-api `d27ccfa` (rpg-api#1197 head), session v0.167.0,
encounter v0.182.0, protos v0.1.260 · `bench-keep-garrison` (10k hexes, 99
garrison, 20 patrols, 100 members, neutral to the party) · an 8-cell
world-clock path along axial q from the file's seat (23,72), back and
forth · 30 Moves after 3 warm-ups, drained between Moves · real Redis 7.2.2
· AMD Ryzen 9 3950X. In-process from the handler's Move to the act broker's
channel: no gRPC transport, no proto or JSON encoding to the client, no
network.

Command (from `rpg-api`, branch `s1-measure` at `8ab27cc`):
```
redis-server --port 6491 --save '' --daemonize yes
RPG_MEASURE=1 RPG_MEASURE_N=30 go test ./internal/integration/session/ -run TestMeasure_AnswerAndFirstBeat -count=1 -v -timeout 60m
```

| mode | continues | answer p50 / p95 | first beat p50 / p95 | last beat p50 / p95 |
|---|---|---|---|---|
| whole answer (today) | 0/30 | 1817 / 2639 ms | 1816 / 2638 ms | 1816 / 2638 ms |
| S1, the answer first | 30/30 | 425 / 635 ms | 1852 / 2636 ms | 1852 / 2636 ms |

Redis GET floor 0.02 ms. Acts per verb for the player: 165 to 320; every
patrol step she sees is its own act. Steps in the S1 answer: 1 to 5,
depending on where the pace falls. Launch 1.4 s, compile 0.2 s.

Means, in order of weight:

1. **The live Move is 1.8 s where the toolkit bench says 134 ms.** The
   difference is the session layer and the host: the act fold per recipient
   at commit (one story fold for each of a hundred members, every verb), the
   load of a record that grows by hundreds of beats per verb, the sheet
   reads per cell, the 1 MB save. F9 named these as "paid in both styles";
   this is their price on the garrison. The toolkit numbers of F7 to F14
   stand, and they are a sixth of the live cost.
2. **The answer moved; the stream did not.** Under S1 the answer is 425 ms
   and still over the 100 ms bar; the first beat is unchanged at 1.85 s
   because delivery waits for the save (S9) and the whole verb's acts,
   including the answered steps', leave in one burst after the continuation
   saves. The first-beat bar needs the beats written before the checkpoint,
   the write tier's beats-first half, or a delivery after the answer that
   S9 does not permit today.
3. **The cost climbs 40 ms per Move.** Whole-answer Moves rose from 1.05 s
   to 2.74 s over thirty; S1 answers from 215 to 644 ms. The record grows
   each verb and something reads it whole.
4. **The act buffer lags on the garrison.** The production buffer of 256
   closed the subscriber on the first garrison verb; the test raised it to a
   million. On this keep a live client would be told it lagged and re-read
   after every verb.
5. The plain `bench-keep` forms a fight at launch, so its Move is a combat
   move; the garrison variant is the free-roam keep for this story.

Against the slice's done line (answer under 100 ms, first beat within 250
ms): not met on the live path. S1 is correct and four times faster to
answer; the bars now point at the session layer. Provisional until rpg-api#1197
merges; the measurement test lives on a branch pending R5.

## F16 — where the live Move goes: the walker's own sight, the fold for a hundred, the machines' sheets

2026-11-08 · same setup as F15 (rpg-api `d27ccfa`, session v0.167.0,
encounter v0.182.0, `bench-keep-garrison`, 8-cell path, real Redis) ·
whole-answer mode, 13 Moves (3 warm-up, 10 measured), CPU profile; Move
CPU 1389 ms against a wall p50 of 1388 ms, so the store's wait is nothing.

Command (branch `s1-measure` at `024f674`):
```
RPG_MEASURE=1 RPG_MEASURE_MODES=whole RPG_MEASURE_N=10 go test ./internal/integration/session/ -run TestMeasure_AnswerAndFirstBeat -count=1 -v -cpuprofile cpu.out
```

| phase | ms per Move | share |
|---|---|---|
| session load: GetEncounter + unmarshal 41, LoadEncounter + compile 48 | 89 | 6% |
| walk: the walker's own sight refresh after each step | 418 | 30% |
| walk: the raise, machines (resolution.Participation attaching monster sheets ≈198) | 235 | 17% |
| walk: the raise, the woken creatures' sight | 188 | 14% |
| walk: mover seam, sheet reads, admission, discovery | 60 | 4% |
| commit: the act fold, one story copy per member in EverMembers (100) | 206 | 15% |
| commit: the act fold, decoding beats to fold them | 139 | 10% |
| commit: JSON marshal + SaveEncounter | 30 | 2% |
| commit: settlement, record, view, deliver | ≈10 | <1% |

Across phases: sight is 606 ms (44%), of which line-of-sight lanes 268 ms;
JSON decoding is 282 ms (20%), spread over the fold, the load, conditions
and sight testimony. No flat function above 4.3%.

Two checks:

- **The fold is per recipient and unbounded.** `commitActs` folds a story
  for each of the hundred ever-members, and each fold copies that member's
  whole record (no FromSeq bound): 31k entries copied on the first Move,
  339k on the thirteenth, 15.5k acts produced for 100 recipients of whom
  one listens. Folding for the player only: 431 entries rising to 3,575,
  1.4 to 3.8 ms, and Move p50 1429 → 1001 ms, p95 1840 → 1371 ms, with most
  of the per-Move growth gone (951–1088 ms against 1100–1499).
- **The player's story is 92% other members' deeds.** A one-pace Move adds
  about 200 entries to her story: 108 patrol steps, 77 NPC machine answers,
  8 of her own steps, 6 sightings, 1 tick; a two-pace Move 386.

Means: the live cost is three things the toolkit bench never measured. The
walker's own sight refresh is 52 ms a step here against 18 ms for the whole
step in the keep bench, so the live refresh carries something the bench's
walker does not (testimony, conditions, the decode); the raise spends 198
ms attaching a hundred monster sheets to a participation per pass, a value
that changes only when a sheet does (R4: instance memory); and the commit
folds the record for everyone who could ever listen, from the beginning,
every verb. None of the three is the toolkit's wake. Provisional until the
measurement test is on dev under R5.

## F17 — under the walker's sight and the machines' participation

2026-11-08 · the F16 profile (`cpu-whole.out`, 13 Moves of 8 steps), pass
counts from a short uncommitted counting hack in a local session copy.

The walker's own refresh, 398 ms a Move, 50 ms a step:

| bucket | ms per Move | share |
|---|---|---|
| line-of-sight lanes (discoverCells → visibleFrom → SightLanes 120; Observe → reach 25) | 145 | 36% |
| door openings for presences: `field.doorCrossings` scans the whole keep's floor every step (isFloor 39, GetNeighbors 33); wall footprints 8 | 96 | 24% |
| sight testimony decode, once per observer per step for 99 observers (hasStrictSightFields re-parses the JSON to check fields, 40) plus heading passes | 84 | 21% |
| session seams: participation (downNow → the standing seam's 99-sheet pass) 17, conditions 6 | 23 | 6% |
| discovery and intel delta 16, per-step beats 7, bookkeeping 25 | 48 | 12% |

The raise's participation, 188 ms a Move: `resolution.Participation` attaches
all 99 monster sheets on a fresh bus for every assessment (JSON decode 30,
condition composition 36, bus subscriptions 54, teardown 25, compulsion
check 19, the in-memory sheet read 8). A one-pace Move assesses 112 times,
a two-pace Move 191: about 11k and 19k sheet attachments per Move, 13 µs
each; every assessment covers all 100 members and six in the whole run
covered the player alone.

Means: three mechanisms, three owners. The door-crossing scan is site
geometry recomputed per step, which R4 places in the compiled site. The
testimony decode is instance state re-parsed from JSON per observer per
step, which R4 places on the instance, decoded once per load. The
participation pass rebuilds a hundred sheets per machine step because the
memo S3b added is dropped by every mover's step, when only a sheet write
changes anyone's standing; attaching once per verb on one bus and
invalidating by sheet write is the session's to do. With the fold of F16,
these four are about 900 of the 1,390 ms.

F17 correction (from rpg-toolkit#2318's builder, a counter on `field.doorCrossings`): the door-crossing scan is paid once per door when doors attach at load, 36 calls per open on the garrison, none inside the timed steps; it reaches 96 ms per Move on the live path because the session loads the encounter once per verb, not because the walker pays it per step. The per-Move cost and the placement (site data) stand; the mechanism named in F17 was wrong. The S9 PR adds a load-plus-atlas benchmark that sees it (823k → 97k allocs/op) because the Move bench keeps one encounter open and cannot.

## F18 — the four session-layer slices halve the live Move; the growth remains

2026-11-08 · rpg-api dev `f890674a` (rpg-api#1197 S1, #1199 S10, #1201 repin;
session v0.169.0 with S7 and S8, encounter v0.184.0 with S9) · same setup as
F15: `bench-keep-garrison`, 8-cell world-clock path, 30 Moves after 3
warm-ups, real Redis 7.2.2, AMD Ryzen 9 3950X, load average 8–9, in-process
from the handler's Move to the act broker, production act buffer (64
commits), the measurement test at rpg-api#1202 `2d8328d`.

Command (branch of rpg-api#1202):
```
redis-server --port 6491 --save '' --daemonize yes
RPG_MEASURE=1 RPG_MEASURE_N=30 go test ./internal/integration/session/ -run TestMeasure_AnswerAndFirstBeat -count=1 -v -timeout 60m
```

| mode | answer p50 / p95 | first beat p50 / p95 | acts per verb | lagged |
|---|---|---|---|---|
| whole answer | 989 / 1280 ms | 989 / 1280 ms | 165–320 | no |
| S1, the answer first | 307 / 484 ms | 885 / 1150 ms | 165–320 | no |

| measure | F15 | F18 |
|---|---|---|
| whole Move p50 | 1817 ms | 989 ms |
| S1 answer p50 | 425 ms | 307 ms |
| first beat p50 | 1852 ms | 885 ms |

Means: the fold for listeners, the sheets once per verb, the site data and
the per-commit buffer together take the live garrison Move from 1.8 s to
1.0 s and the S1 answer from 425 to 307 ms, and the lag on the first
garrison verb is gone. Against R1 (100 ms answer, 250 ms first beat) the
answer is three times over and the first beat three and a half, and the
first beat still lands with the last beat because the verb's acts leave as
one unit after the save. The growth is not gone: the S1 answer climbs from
139–263 ms to 357–522 ms over 33 Moves and the whole Move from 599 to 1280
ms, so something still scales with the record each verb (the session's
record stream fold, the load and save of a record that grows by 200–386
entries a Move, or the catch-up read). A growth profile is next. Holds as
the story's live number at this head.

## F19 — the record is unbounded and the blob's round trip is the growth

2026-11-08 · rpg-api dev `f890674a` (session v0.169.0, encounter v0.184.0),
whole-answer mode, garrison keep, 33 Moves; wall-clock phase timings inside
`Manager.Move` from an uncommitted instrumented copy of the two modules,
load average 6–10; one-pace Moves compared.

| phase | Move 5 (1,572 entries, 4.8 MB blob) | Move 31 (8,748 entries, 24.3 MB) | per 100 entries |
|---|---|---|---|
| GetEncounter (GET + unmarshal) | 31 ms | 154 ms | +1.7 ms |
| LoadEncounter | 46 ms | 105 ms | +0.8 ms |
| walk: the walker | 270 ms | 323 ms | +0.7 ms |
| walk: the raise | 195 ms | 205 ms | +0.1 ms |
| commitRecord (whole-record copy) | 1.5 ms | 12 ms | +0.14 ms |
| commitActs | 1.4 ms | 1.4 ms | flat |
| ToData + WorldView | 6.5 ms | 18 ms | +0.16 ms |
| SaveEncounter (marshal + SET) | 38 ms | 152 ms | +1.6 ms |
| Move inside the SDK | 596 ms | 996 ms | +5.6 ms |

The record: 541 entries after Move 1, 9,358 after Move 33, about 210 added
per one-pace Move and 400 per two-pace Move; the blob 2.06 MB → 25.9 MB,
about 2.7 KB per entry and 0.56 MB per one-pace Move. Redis itself is 0.03
ms; the cost is JSON. Retention: the session launches every run with
`encounter.RetentionUnbounded` (session `launch.go`), so no trim ever runs.

Means: of the 5.6 ms a Move gains per hundred entries, 4.1 ms is the blob's
round trip (unmarshal, compile, marshal) and the blob is mostly record; the
raise is flat; the acts fold is flat since S7; the record stream's copy is
small but grows. On this keep the whole record is serialised and parsed on
every verb and never trimmed, which is the shape #770 names and R3's
instance and write tiers address: a resident instance loads nothing per
verb, and a record appended rather than rewritten is written once per beat.
2.7 KB per entry is large for a beat; what an entry is made of is the next
read. Holds.

## F20 — a record entry is its audience; the checkpoint without the record is 1 MB and flat

2026-11-08 · the saved blob after Move 10 of a whole-answer garrison run at
rpg-api dev `f890674a` (8.72 MB, 3,003 entries) and after Move 4 (4.28 MB,
1,362 entries); compact JSON as stored; the payload is a `[]byte`, so it is
stored base64.

| field of a `moved` beat (49% of entries) | bytes |
|---|---|
| audience: 100 member ids at about 24 B each | 2,420 |
| payload as stored (decoded 75 B: member, position, beat) | 114 |
| tags, correlation, id, seq, at | 166 |
| total | 2,701, of which audience 90% |

An `answered` beat (33%) is 3,125 B with a 538 B payload (rows, candidates,
creature, temper, selector), audience 77%; a `tick` 2,630 B, audience 92%;
`sighted` and `cells_revealed` carry one recipient (320 B and 7.7 KB).
Across the record: audience 6.34 MB (82.7%), payload 0.85 MB (11.1%), tags
2.4%, correlation 1.7%, id 1.3%, seq and at 0.7%. The audience is 72.7% of
the whole blob at Move 10 and about 79% at Move 33 by extrapolation.

| the blob without the record | Move 4 | Move 10 |
|---|---|---|
| field (regions 161, placed 166, prop presentations 139, walls 32) | 498 KB | 498 KB |
| perception (intel and testimony) | 346 KB | 347 KB |
| journal | 132 KB | 132 KB |
| members, doors, sight sweeps, clock, ever-members | 75 KB | 75 KB |
| everything except the record | 1.051 MB | 1.052 MB |
| the record | 3.2 MB (75%) | 7.7 MB (88%) |

Means: every beat the whole table may see spells the whole table out, a
hundred ids a beat, and that list is four fifths of the record and three
quarters of the blob; a beat for everyone named once ("everyone") takes a
`moved` entry from 2,701 to 281 B and most of F19's growth with it, before
any tier is built. The checkpoint without the record is a megabyte that
does not grow, half of it the site field that R3's site tier takes out of
the save altogether; the record is the only thing that grows, which is what
an append-only write is for. Holds.

## F21 — a cold reload is 159 ms; load plus save is a quarter of the Move

2026-11-08 · rpg-api dev `85c8bcc` plus the harness commit (merged as
rpg-api#1203, dev `3abf710d`), session v0.171.0, encounter v0.185.0,
`bench-keep-garrison`, N=20 per mode, Redis on a side port. Command:
`RPG_MEASURE=1 RPG_MEASURE_REDIS=127.0.0.1:6497 RPG_MEASURE_N=20 go test
./internal/integration/session/ -run TestMeasure_AnswerAndFirstBeat -count=1 -v`.
Run record: rpg-api `knowledge/runs/2026-11-08-alive-not-lagging-cold-reload-api.md`.

| stage | bytes | p50 | p90 |
|---|---|---|---|
| session record fetch + decode | 83 KB | 1.6 ms | 2.8 ms |
| fetch the encounter blob (GET) | 9.1 MB | 13.2 ms | 15.4 ms |
| decode (JSON → EncounterData) | | 62.8 ms | 69.0 ms |
| site build (LoadEncounter) | | 75.1 ms | 79.1 ms |
| attach (residual of a read verb: lock, driver, standing) | | 7.3 ms | 21.7 ms |
| **read load whole** | | **159.3 ms** | 176.6 ms |
| ToData | | 8.3 ms | 10.1 ms |
| JSON encode | 9.1 MB | 58.4 ms | 64.0 ms |
| persist (SET) | 9.1 MB | 7.4 ms | 9.6 ms |
| Move, whole answer | | 889.7 ms | 1221.6 ms |
| Move, S1 answer / drained | | 255.9 / 914.9 ms | 347.0 / 1215.3 ms |

Attach is a residual, not an isolated stage: the session's standing and
capability builders are unexported, so the row subtracts the other stages
from a read verb. The write-path extras of a write verb's open (ledger,
write capabilities, sheet staging, act open) are reachable only through a
verb and are not in the row. No toolkit hook was added.

Means: a miss is cheap enough to make routing an optimisation (R6): 159 ms,
of which JSON decode and the site build are 138. Load plus save is about
230 ms of an 890 ms Move; JSON alone (decode plus encode) is 121 ms, more
than the field compile. The blob grows with the record, 3.7 MB after 6
Moves to 9.1 MB after 23, so a miss gets dearer as a session ages, and the
audience table (S11, record v0.3.x) takes most of that growth out before
any residency lands. The other 650 ms is the verb itself, the walk and the
fold, which no storage tier removes: the S1 answer at 256 ms against the
100 ms bar is a walker cost (F16: 52 ms a step live against 18 on the
bench), and that is the next profile once the storage line is in.

## F22 — the fence costs one round trip; the audience table takes two thirds off the blob

2026-11-09 · rpg-api dev `109251fc` (rpg-api#1208, session v0.172.1, play/record
v0.3.0), `bench-keep-garrison`, N=20, Redis on a side port; the harness of
F21 (`RPG_MEASURE=1 RPG_MEASURE_REDIS=127.0.0.1:<port> RPG_MEASURE_N=20 go test
./internal/integration/session/ -run TestMeasure_AnswerAndFirstBeat -count=1 -v`).
Run record on rpg-api#1208.

| row | before (dev `3abf710d`, record v0.2.x) | after |
|---|---|---|
| Move, whole answer, p50 | 949 ms | 777 ms |
| S1 answer, p50 | 273 ms | 198 ms |
| blob after the walk | 9.1 MB | 2.7 MB |
| write side: encode + SET → fenced save | 24.06 ms | 24.43 ms |
| read side: plain → fenced read | 26.27 ms | 27.41 ms |

Means: the fence (one Lua compare-and-set over a sibling version key, the
world, the session record and the touched sheets in one script) costs about
one round trip, under half a millisecond on the write and one on the read.
The Move numbers are not a like-for-like pair: between the two runs the
record's audience table (S11) shrank the blob by two thirds, and the JSON
round trip shrank with it; that, not the fence, is the 170 ms. A stale
writer now fails closed with ABORTED and nothing written; a run launched
before the deploy has no version key and fails closed at its next verb by
design (the castle gets a fresh Redis at rebuild).

## F23 — a live step is a cold sweep: the per-verb load discards what a cell sees

2026-11-09 · rpg-api dev `109251fc` plus the harness of rpg-api#1211
(`34dc1d6`), session v0.172.1, encounter v0.185.0, play/record v0.3.0.
`bench-keep-garrison` (posts listening), S1 continues mode, 3 warm-up
Moves, Redis 7.2.2 on a side port, load average 4 to 5. Run record on
rpg-api#1211. Command, with `RPG_MEASURE_PATH` set to 1, 4 and 8 and a CPU
profile on the 8-cell run:
`RPG_MEASURE=1 RPG_MEASURE_REDIS=127.0.0.1:6497 RPG_MEASURE_N=20
RPG_MEASURE_MODES=continues RPG_MEASURE_RELOAD=0 RPG_MEASURE_PATH=8 go test
./internal/integration/session/ -run TestMeasure_AnswerAndFirstBeat -count=1 -v -cpuprofile cpu.out`.

**Per verb against per step.** A continued answer is the load plus the
walker's own steps up to the pace, nothing saved; the line is fitted over
continued Moves only.

| path | answer p50 by steps answered | fixed per verb | per step |
|---|---|---|---|
| 8 cells, N=20 | 1: 120 · 3: 195 · 5: 272 ms | 84 ms | 37.4 ms |
| 4 cells, N=30 | 1: 109 · 3: 185 ms | 71 ms | 38 ms |
| 1 cell, N=30 | 0: 63 ms | 63 ms before any step | |

A rerun at load average 12 gave 90 ms fixed and 42 ms per step. The verb's
open is 68 ms of CPU: the fenced fetch and JSON decode 20 ms, LoadEncounter
46 ms (field compile 14, room-local sightings 10, behaviors 7, testimony
decode 7). The intercept sits about 20 ms above the zero-step answer, so a
verb's first step costs more than the later ones. Answers that finish the
walk carry the save and the commit and add about 40 ms.

**Per answered step, CPU** (profile focused on `startWalk`, 2.99 s over
about 71 steps).

| bucket | ms per step | share | owner |
|---|---|---|---|
| the walker's sweep from each new cell; `visibleFrom` misses cast fresh (SightLanes) | 17.5 | 41% | encounter |
| discovery remainder (learnCells, sweep keys) | 2.5 | 6% | encounter |
| line of sight to each member (Perception.Observe) | 5.8 | 14% | encounter |
| observers within reach, held sight testimony | 3.1 | 7% | encounter |
| refresh remainder (presences, conditions, equipment, testimony encode, triggers) | 3.2 | 8% | encounter |
| Step bookkeeping (stepMember, beat, pace) | 0.7 | 2% | encounter |
| mover seam: `resolution.Resolve` builds a whole WorldView per step (behavior runtime 0.20 s, Log.ToData 0.12 s, perception) | 5.6 | 13% | session / resolution |
| mover seam: attachAll and teardown | 1.4 | 3% | session |
| standing seam | 1.7 | 4% | session |
| PaceDue | 0.6 | 1% | encounter |
| total | 42.1 CPU (37 to 42 wall) | | |

**Against the bench** (an uncommitted experiment bench at encounter
v0.185.0, posts-asleep keep, stub capabilities, raising steps excluded).

| variant | mid step | first step | allocs per op |
|---|---|---|---|
| the garrison bench's two cells, keep kept open | about 12 ms | 11.9 ms | 114k |
| 8-cell line, keep kept open | 19.3 ms | 17.3 ms | 259k |
| 8-cell line, keep reloaded from JSON before each op | 36.3 ms | 42.6 ms | 1.23M |

A profiled rerun at higher load gave 10.4 ms open against 28.5 ms
reloaded; `visibleFrom` spent 0.31 s kept open against 2.81 s reloaded.
The reload alone doubles the step.

Means: half the live walker step, about 17 ms, is the walker re-sweeping
cells it already swept. The visible-cells cache from S2 is instance memory
that the per-verb load throws away, so by F17's placement it is per-load
state paid per verb; the bench never sees it because it keeps one
encounter open on two cells. The raw answer to what a cell sees given
doors and props is site data (R4), so the cache belongs in the site tier
(S5) or survives in a resident instance (R6, S6); either way it is the
encounter's to place. The mover seam builds a whole-world snapshot on
every step for its resolution, about 7 ms with the attach: per-step work
over state that changes per verb, the session's and resolution's. The rest
is the walker's genuine per-step sight, about 15 ms, and the 63 to 68 ms
open per verb, which is the F21 load line.

## F24 — the site shared by hash halves the load line; path 1 is under the answer bar in the harness

2026-11-10 · rpg-api dev `2be23860` (rpg-api#1216), session v0.175.0,
resolution v0.75.0, encounter v0.188.0, play/record v0.3.0. Before = dev
`2253b33e` at the previous pins. `bench-keep-garrison` (posts listening),
both modes, N=20 per path, Redis on a side port flushed between paths. Run
record on rpg-api#1216
(`knowledge/runs/2026-11-10-alive-not-lagging-site-shared-api.md`).
Command, with `RPG_MEASURE_PATH` set to 1, 4 and 8, once per tree:
`RPG_MEASURE=1 RPG_MEASURE_REDIS=127.0.0.1:<port> RPG_MEASURE_N=20
RPG_MEASURE_PATH=8 go test ./internal/integration/session/ -run
'TestMeasure_AnswerAndFirstBeat$' -count=1 -v`.

**The whole Move and its load line.**

| row | path 1 before → after | path 4 | path 8 |
|---|---|---|---|
| Move whole, answer (AnswerWhole) | 146 → 93 ms | 595 → 210 ms | 851 → 350 ms |
| S1 continues: answer / drained | 148 / 150 → 90 / 92 ms | 207 / 607 → 108 / 210 ms | 215 / 831 → 129 / 356 ms |
| read load whole (Manager.Where) | 66.6 → 38.1 ms | 80.6 → 51.9 ms | 96.6 → 76.8 ms |
| 3 site build, warm provider (attach only) | 45.2 → 28.1 ms | | 54.0 → 41.1 ms |
| 3p stored piece GetSite, GET + decode, 507 KB | — → 8.7 ms | | — → 7.9 ms |
| 3c cold site build, fresh provider + piece | — → 49.4 ms | | — → 63.9 ms |
| Move whole, cold provider (fresh process, first verb) | — → 162 ms | | — → 971 ms |
| Move whole, warm provider (same process, next verb) | — → 131 ms | | — → 825 ms |
| encounter blob bytes | 1,876,612 → 1,378,438 | 3,557,333 → 3,059,159 | |
| 2 decode JSON → EncounterData | 16.0 → 8.6 ms | 22.7 → 15.7 ms | |
| 5b encode JSON | 15.4 → 8.9 ms | 25.8 → 17.6 ms | |

**Found.**

- The checkpoint no longer carries the field: 498 KB less per blob, and
  decode and encode halve with it. The fenced fetch and save each drop from
  about 19 to about 10 ms on path 1.
- A cold process pays the piece fetch plus the compile once per site, 50 to
  64 ms; every later verb on that site is warm. On the dev env the boot
  compiles the shelf itself: 13 files in 762 ms, the keep 153 ms, the castle
  9 ms, the rest under 2 ms each, one log line per site.
- "Site build, warm provider" is still 28 to 41 ms per verb. That is
  `LoadEncounter` attaching the instance to the site (perception, members,
  behaviors, room-local sightings), not a compile. With the field gone it is
  the largest fixed cost left in the load line, and it is what S6 (the
  resident copy) removes.
- The path 8 drained number falls from 831 to 356 ms with no change to the
  walker: the per-step re-sweep of F23 was paying for the field compile's
  discarded sight too. The remaining per-step cost is the F23 remainder.
- Against R1 in the harness: path 1 answer 93 ms is under the 100 ms bar for
  the first time; path 8 is 350 ms. First beat on path 8 (S1 continues,
  drained) is 356 ms, against a 250 ms bar after the answer. S13 is the next
  slice that moves it.

**Deploy.** A checkpoint saved before this head carries a field and no
`site_hash`; the api refuses it as INVALID_ARGUMENT, named by field. The dev
env was rebuilt on `2be23860` at 04:54 with its Redis persisted; any session
live at that moment relaunches. The persistent dev content dir still holds
thirteen files (the reference sites compile again at these pins); rpg-api#1204
reconciles that dir to the shipped set.
