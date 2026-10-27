# Brainstorm: the world's time while a fight runs

Working notes, 2026-10-27. Measured on rpg-toolkit origin/main c355102d
(encounter v0.148.0), castle harness, sight 60 ft, two player characters.
Probe source and logs: the job's tmp dir (`clock-*`), not committed.

## The model as the engine runs it (read, not guessed)

1. One world clock (`play/clock` Tick). Each driver keeps its own progress;
   the high-water is the MAX over drivers, not a sum (tick.go:145-151).
2. A raise of the high-water grants +delta budget to every member on the
   world clock; only a raise makes the world think (worldtime.go:66).
3. A world-clock player pays one round per action-priced verb and one round
   per pace (speed cells walked). Standing still is free.
4. Anyone in a fight bubble pays nothing on those paths.
5. A fight round wrap above round 1 advances each fighter by one under its own
   name (spendRound, flip.go:298). The world thinks only if a fighter is the
   front-runner.
6. World thinkers skip bubble members; each spends its whole budget, one
   full-speed no-attack turn per unit (thinkFor); then one sight refresh,
   which can form or join the fight.
7. Arrivals (reserves) and joins are seated at the high-water with budget 0;
   seating raises nothing.
8. Into the fight: Transfer moves membership only; progress is kept;
   stragglers go to the end of the order. Out: dissolve rejoins the world at
   budget 0 with its own progress; it catches up only when somebody raises.
9. ONE bubble per encounter (clocks.go:2673); any later contact anywhere joins
   the running fight (trigger.go:442).
10. Every beat, deed, percept and sound is stamped with the high-water.
11. A sound lands on hearers immediately; a listener acts on it only at its
    next table consult, i.e. the next raise. `heard within N` is measured in
    world rounds.

## Scenarios (A fights at the open gate; B is the roamer, out of sight)

| scenario | world turns per fight round | reserves | high-water after round 8 |
|---|---|---|---|
| S1 B still | 1 | 6 cells/round; join by sight after r2; adjacent after r4 | 8 (A 8, B 0) |
| S2 B walks 6/round | 1 (raised by B's pace mid-round; the wrap raises nothing) | as S1 | 8 |
| S2 B walks 12/round | 2 | 12 cells in r1, join r1 | 16 |
| S2 B walks 30/round | 5 (hall-1, keep-1 moved 30 cells/round) | two turns, then join | 40 |
| S3 B 10 paces ahead, then still | 0 for r1–r10, then 1 | frozen 10 rounds; first move after r11 | 10 until r11 |
| S4 as S3, B walks in on r3 | 1 raise at the join, then 1/round | unfrozen from r3; B seated at its own 11, last in order | |
| S5 clang in the fight, world-clock hearers | heard at once; acted at the next raise | S1: end of r1; S3: r11; S4: r3 | |
| S6 B meets hall-1 in the kitchen on r2 | no second bubble | B and hall-1 join A's fight 21 cells away | |

Fight beat stamps for rounds 1–5: S1 1,2,3,4,5; S2-30 5,10,15,20,25; S3
10,10,10,10,10. Every `within N` window inside the fight (heard, attacked,
deeds) is therefore set by the roamer: it expires five times faster under
S2-30 and never ages under S3.

Deciding lines: the max-by-driver raise (tick.go:146) and the per-pace raise
(spendWorldPace) for S2; the wrap raising only when a fighter leads
(flip.go:298) for S3; Transfer keeping B's progress for S4; "act at the next
raise" for S5; one bubble (clocks.go:2673) and join-anywhere (trigger.go:442)
for S6.

## Four findings, two of them Kirk's

- **The roamer expedites.** Linear in B's paces. (Kirk's first.)
- **The place freezes.** For exactly B's lead. (Kirk's second.)
- **Fight memory rides the roamer.** "Attacked within 2 rounds", "heard
  within 2" and the record's stamps inside a fight are measured on a clock
  the roamer drives. Under the freeze nothing ages; under a running roamer
  everything is forgotten in a round.
- **Reactions wait for the roamer.** A guard who heard the clang acts when
  the world next moves, which is the roamer's call.

And one structural fact to keep in view: there is one fight per encounter,
and contact anywhere joins it, so B's own skirmish across the castle runs in
A's initiative.

## What the rules say

Outside initiative there are no turns; time is narrative. Inside, a round is
six seconds for EVERYONE in the scene, fighters and the ones running toward
it alike. A DM with one player in a fight and another walking the halls puts
the walker in initiative too: a pace a round. Nobody gets to walk five rounds
while the fight sits.

## Candidate models

**(a) Today: one shared clock paced by the front-runner.**
- Keeps: one total order; deterministic record; monotonic stamps; a roamer at
  one pace a round or slower changes nothing.
- Breaks: everything above.
- Size: none.

**(b) The fight sets the world's clock while it runs.** One world round per
fight round for everyone outside; arrivals and joins seated at the fight's
time; a roamer's paces during a fight capped at one per round.
- Fixes: expedite, freeze, fight memory, reaction latency (one round).
- Breaks / to decide: what the cap does with a roamer's longer click
  (refuse, cut at a pace and resume next round, bank, or free: free is an
  exploit, bank defers the expedite, cut is a de facto initiative for the
  roamer, which is what the rules say anyway). A lead taken BEFORE the fight
  is double-counted unless every driver is shifted together when the fight
  forms (elapseWorld already shifts all drivers; the fight's formation would
  seat the fighters at the high-water, which is (d1)).
- Keeps one clock, so determinism holds.
- Size: medium. The wrap's raise path, a persisted per-round pace count, the
  seating at formation, pacing tests.

**(c) Per-bubble local clocks, reconciled on contact.**
- Fixes: true concurrent fights (S6); no cross-area expediting or freezing.
- Breaks: stamps from different clocks stop being comparable (every `within`,
  every percept age, the record's order); contact needs a reconciliation rule
  (fast-forward the lagging side by simulating it, or make the leading side
  wait), both new semantics; lifting one-bubble touches Form, Transfer
  addressing, dissolve.
- Size: large. Not this story.

**(d1) Small: seat fighters at the high-water on every wrap** (the
seatOnWorldClock pattern inside spendRound), so each fight round raises
exactly one.
- Fixes: the freeze; arrivals are already at the fight's time because the
  high-water IS the fight's time.
- Breaks: does not cap the roamer (S2 stays linear); a fighter's own progress
  is overwritten ("each fighter lived it" becomes "each fighter is at the
  world's now", which is the rules' reading).
- Size: small.

**(d2) Small: cap world-clock pace raises while a bubble runs** at one per
wrap, remainder carried on the walk.
- Fixes: S2 only. With (d1) it is (b) without new arrival logic.
- Size: small to medium (a persisted per-round raise count).

## Weighing

| | (a) today | (b) fight sets the clock | (c) per-bubble | (d1)+(d2) |
|-|-|-|-|-|
| roamer expedites | **yes** | no | no | no |
| place freezes | **yes** | no | no | no |
| fight memory honest | **no** | yes | per clock | yes |
| reaction latency | roamer's | 1 round | 1 round | 1 round |
| one deterministic order | yes | yes | **no** | yes |
| two fights at once | no | no | **yes** | no |
| size | none | medium | **large** | small+small |

**Leaning: (b), built as (d1)+(d2).** When a fight runs, the fight is the
clock: every wrap raises the world by exactly one, every fighter and arrival
sits at the world's now, and a roamer who walks walks a pace a round, cut
there and resumed after the next wrap. One clock, one order, the record
stays deterministic. Concurrent fights (c) stay a later story; today a
roamer's skirmish joins the running initiative, which is what a DM at one
table would do too.

## Where the trade-offs bite

- **The roamer's click.** Cutting a long walk at one pace per round is honest
  but visible: the client must say "the fight is on; you move a pace a round"
  and resume the walk after the wrap, or the player sees a refused click.
  Banking is invisible but defers the expedite to when the fight ends.
- **A lead taken before the fight.** Seating fighters at the high-water on
  formation (d1) means A's first round is stamped 11 if B walked ten paces
  first. That is the rules' reading (time passed for everyone), and the
  memory windows then age from there. The alternative, shifting B back, is
  rewriting B's past.
- **Reactions.** One round of latency for a world-clock listener is the price
  of consulting tables at raises. Fine; today it is the roamer's whim.
- **Determinism of the record.** (b) keeps the single stamp line; the replay
  bar (rpg-project#649) stays satisfiable.

## Questions for Kirk, in the order they matter

1. Direction: the fight sets the world's clock while it runs, (b) as (d1)+(d2)?
2. The roamer's long click during a fight: cut at one pace and resume after
   the next round (recommended), or bank the rest?
3. A lead taken before the fight: fighters seated at the world's now when the
   fight forms (recommended; A's first round may be stamped 11), or shift the
   roamer back?
4. Hold: concurrent fights stay one initiative per encounter until a scene
   needs two?

## Ruled 2026-10-27

Kirk: "I think we want to remain flexible in this time. Ideally having a
roamer trigger non-combat monsters' turns is a configuration we can set. Our
users will be using the encounter studio to build worlds; they should have
the ultimate control. So for now: yes, roamers can trigger NPCs operating on
the same world clock. Maybe that's it. The NPC clocks, if shared on the world
clock, would be affected by it; other clocks would have their own trigger
points."

So: none of (b), (c), (d) as an engine law. The clock a creature runs on is
CONTENT. Today's behaviour is the default: the world clock is driven by paces
(a roamer's included) and by fight rounds when a fighter leads, and every
creature on it moves with it. A world builder who wants reinforcements to
live the fight's six seconds, or a watcher who only stirs on an alarm, binds
that group to a clock whose trigger is the fight round, or the alarm. No
engine change now. What stays true under the default, and is the author's
to change: a world-clock creature does not move while nobody raises the
clock (the freeze), and memory windows age on the creature's own clock.

The first use case for an authored clock is the castle's own reserves
(rpg-project#661): "arrive within the rounds the distance implies" is a
fight-round trigger. It waits for the story that brings it. Concurrent
fights remain one initiative per encounter until a scene needs two.
