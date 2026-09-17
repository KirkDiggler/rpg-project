# The world thinks — the world clock advances as the party moves, and creatures are driven on it

**Status:** DESIGN 2026-09-17, for Kirk's rulings R1–R5. Named as the next slice in
[shenanigans/front-room-goblin.md](../shenanigans/front-room-goblin.md) §Next:
"The world clock advances as the party moves and creatures are driven on it (Kirk: the
arrived thug should walk to the front room). This is the door `flee` opened, applied every
tick; Regroup and Alarm ride it."

**Lane:** toolkit only. No proto changes, no api or web work (the beats this slice emits all
exist: `Moved`, `Arrived`, `Tick`, `Answered`).

## The one sentence

Outside a fight, time passes because the party acts, and every creature not in a fight gets
the same time to act on, driven by the one mind it already has in a fight.

## What is true today (verified against origin/main 2026-09-17)

The clock exists and nothing a player does moves it.

| Claim in the goblin design | Verdict | Where |
|---|---|---|
| A world clock exists, owned by the encounter | TRUE. `play/clock.Tick`, one per encounter, high-water accrual by driver; members hold budgets | `play/clock/tick.go`, `encounter/encounter.go:95` |
| "Search and Unlock already spend there" | FALSE. Every world verb (Move, Search, Unlock, Intimidate, Persuade, Interact, Loot, Cast) only *reads* the clock as an `at` stamp. `Tick.Spend` and `Tick.Budget` have no caller outside `play/clock` | `encounter/step.go:141`, `search.go:97`, `intimidate.go:262` |
| Something advances the clock | Two sites only: a fight round wrapping (`Displacement: 1`, driver `"world"`) and `Encounter.Pump` | `encounter/flip.go:235`, `encounter.go:1671` |
| The world thinks | `Pump` exists (decide all, execute, one sight refresh, one tick beat) and is called by one workbench. Session never calls it and never supplies the `Decider` it consults, so a call would be a no-op | `encounter.go:1558`, `session/write.go:597` |
| A creature acts outside a fight | Only `flee`: one directed walk, full speed, away from the actor, inside the social verb, no mind consulted | `encounter/answer.go:289` `fleeFrom` |
| The tick's unit | A fight round. The mind presets count patience and fear in ticks, and the round site advances one per round | `flip.go:210-232`, `behavior/minded.go` presets |
| An arrival walks in | No. It is placed at its cell, joins the world clock at budget 0, and stands | `encounter/reserve.go:279` |
| Minds read stance | NOT BUILT. Zero `stance` references in `rulebooks/dnd5e/behavior` and `mind/behavior`; `SeenMember` carries no opposition | `turndriver.go:172-230` |

Two seams drive creatures today, and they disagree:

- **`TurnDriver.Act(MonsterView)`**, required at construction, reached only inside a bubble. Behind it is the whole mind: `Basic`, `Minded`, the retaliator/berserker/coward presets, the five-rung ladder with `Pass | Attack | Toward | Away`.
- **`Decider.Decide(Snapshot)`**, optional per member, reached only from `Pump`. `Snapshot` is a cell and holdings; the intents are `MoveTo | Hold`. It predates the mind and nothing in a session ever registers one.

ADR-0043 kept them apart because `Decider` "is scoped to *where to be*, not *what to do on a turn*". The ladder now answers both with `Toward` and `Away`, so the reason has lapsed.

Also worth keeping visible: the wire's comment on `ANSWER_WORD_FLEE` says "the mind decides what running looks like, and the steps arrive as MOVED on the creature's own turn". The code ships a directed walk with no mind. The proto describes this slice; the toolkit shipped the door.

## The shape

### 1. Time passes because the party acts (R1, R2)

The unit stays **one round**. That is what the tick already means everywhere it is read: a fight round advances one, a preset's patience of 3 is three rounds, a percept confirmed at `at` ages by rounds.

A world-clock verb pays its time **after its outcome lands**, in the encounter, through the one clock:

| Verb on the world clock | Displacement | Driver |
|---|---|---|
| Move | one per **pace**: every `SpeedFeet / 5` cells the mover walks. The remainder carries on the member (persisted) | the mover |
| Intimidate, Persuade, Search, Unlock, Interact, Loot, Cast, any verb the turn clock prices as an action | one | the actor |
| A fight round wrapping | one, **per bubble member** (each of them lived the round) | each bubble member |

Why the driver is the actor and not `"world"`: `Tick` accrues **max, not sum**, per driver. Four players each walking six cells is one round, not four. That is the design's own DOS2 spine ("the world moves as far as you do") and it only works if every cause of time names who spent it. The two existing sites use the literal `"world"`, which would make a fight the front runner forever: after ten rounds of fighting, a player's first walk would raise nothing until they had walked ten paces. Both sites change to name their members. `Pump`'s own advance goes away with `Pump` (below).

What this forecloses, said plainly: **standing still is free.** A party that talks to the goblin and then waits sees nothing happen. That is the roguelike clock the `play/clock` design chose ("advances only because players act") and the encounter's C5 ("no goroutines, no timers"). A rest, a wait verb, or a watch that passes time is a later verb that reports displacement like any other; nothing here stops it.

**Rejected shortcut, R1:** one unit per Move verb regardless of length. A one-cell step would cost a round and a twelve-cell click would cost one; the client's path length would set the world's speed. The pace rule is more bookkeeping (a residual per member) and is the truth.

### 2. The world thinks when time passes (R3)

`Pump`, `Decider`, `Snapshot`, `Intent`, `IntentMoveTo`, `IntentHold` and `JoinInput.Decider` are **deleted**. Unwired leftovers are not evidence; a second driver seam with a thinner vocabulary is exactly what a builder would extend by accident. ADR-0043's "two seams" section is amended to say why one is enough now, citing this doc.

In their place, one internal step in the encounter, `worldThinks`, runs **inside the same call** that raised the high-water, after the verb's own beats and before its sight refresh:

- every member on the world clock that is a monster, standing, and `Ready` (budget > 0) is consulted in stable member order;
- for each unit of budget it holds, it is given **a turn's worth of the world**: the same `MonsterView` a fight builds, with `Budget{AttacksLeft: 0, MovementFeet: SpeedFeet}` and `Round: 0`, run through the same `TurnDriver` and the same intent loop a fight uses (`runTurnIntents`), then `Spend(1)`;
- `Attack` returned outside a bubble is a **refusal, not a skip**: a creature that can reach an opposed member is one sight refresh already formed a fight around. Reaching this line is a bug and says so;
- `Move`/`Routed` steps are ordinary world steps (no threatened squares, no reactions), the same rule `Pump` stated;
- then the verb's single sight refresh runs, as today. A creature that walked into an opposed member's sight forms or joins a fight right there, by the shipped path, and leaves the world clock with its remaining budget gone.

A fight round wrapping is such a raise, so **creatures outside a fight move once per round of it**, in `noticeRounds`, after the round's arrivals. That is how reinforcements cross a dungeon while the party fights, and it is the primitive Alarm was already going to need.

`TurnDriver` is required at construction and re-registered at load already; session changes nothing to supply it. The `driveOneMonsterTurn(bubble, …)` family is refactored so the turn-worth loop takes a budget and an announcer of steps rather than a `*clock.Turn`. That refactor is the slice's cost.

### 3. A mind is never handed a target it is not opposed to (R4)

This is the row the goblin design marked "load-bearing the moment any neutral creature acts", and it becomes load-bearing here: `Basic.Act` walks toward the closest standing player it can see. A neutral goblin given a world turn would advance on the party.

- `SeenMember` and `RememberedMember` gain `Opposed bool`, filled by the encounter from the stance graph (`opposed(a, b)`, the same answer `classify` uses to form a fight). Truth, not belief: the creature knows whom it hates.
- `Basic` and `Minded` consider only opposed members as targets. A creature with nothing opposed in sight or memory passes.
- No stance is written on the mind. The graph owns opposition; the driver reads the projection. Regroup, which wants allies in the view, reads the same field's other value.

### 4. The arrived thug (R5)

With 1–3, a creature that saw the party and lost them **pursues** on the world clock (`Toward` a remembered position, the ladder's rung 2, unchanged), and a cowed creature keeps running while its fear lasts. Both are new behaviour paid by this slice with no new mechanism.

The arrived thug has seen nobody, so under 1–3 it stands where it was placed. Kirk's sentence needs one more thing: a reason to walk. The honest one is **a rumour**, which is the Alarm slice's mechanism: "the runner lands a rumour of the party's last-known position on allied witnesses; they rank a rumoured target and go Toward it" (intimidate.md, slice 3). Perception already keeps a `Report` distinct from a sighting.

**R5 offers two cuts:**

- **(a) Rumour on arrival, this slice, its own PR at the end of the wave.** A member that arrives on a `{fact}` predicate is handed a rumour of the cell where that fact was taught (the teaching actor's cell). Its mind ranks the rumoured contact as opposed-by-faction and walks `Toward` it. Alarm then reuses this with the runner as the source instead of the arrival. Cost: `learnFact` learns *where*, and the arrival carries it into a `Report`.
- **(b) Defer to Alarm.** The thug stands until it sees someone. The walk still shows pursuit (a fleeing goblin pursued after the party loses sight of it, or a thug that glimpsed the party and lost them) and the reinforcements-cross-the-dungeon behaviour during a fight, which is the slice's own claim.

Recommendation: **(b)**, with (a) as Alarm's first PR. Kirk named the thug, so this is his call, but (a) invents a reason to walk that no author wrote and no creature witnessed, and the arrival predicate `{fact}` is already doing the summoning. A rumour with a named carrier is the better tool.

### 5. What the wire shows

Nothing new. Creature steps arrive as `Moved` (one per cell, as `flee`'s do today); the clock reading is `Event.at` on every beat; `EVENT_KIND_TICK` already exists body-less and the world-think appends it once per raise, as `Pump` did, so the debug log shows time passing. `Moved` carries no clock discriminator; a fight walk and a world walk look the same, and they are the same thing.

## Ownership

| Noun | Charter | Holder | Singular? |
|---|---|---|---|
| The world's time | `play/clock.Tick` computes accrual; the encounter is its one holder and the only writer | encounter | one per encounter, already |
| What a verb costs in time | the turn clock's pricing is the precedent; the world clock's is the same table plus pace | encounter verbs, at their outcome | one table |
| What a creature does with time | `TurnDriver`, the one mind seam | behavior packages behind it | one seam after this slice |
| Whom a creature opposes | the stance graph | encounter, projected onto the view | already singular; the view stops hiding it |
| Where a creature believes the party is | perception holdings (sighting, memory, later rumour) | mind/perception | untouched here |

## The seven principles, run

- **Ownership before mechanism.** Time is the encounter's; the mind is the driver's; opposition is the graph's. Session adds no logic.
- **Zero values tell the truth.** A member on the world clock at budget 0 does nothing; a residual pace of 0 is a fresh walker; `Opposed: false` is "not a target", never "unknown".
- **Fail closed loudly.** `Attack` off the turn clock refuses. A `TurnDriver` is already required at construction. A world verb that cannot pay its time (member not on the clock) errors rather than skipping.
- **Assume it exists.** It did: `Tick` budgets, `Ready`, `Spend`, the max-by-driver rule, `Pump`'s two-phase contract, the mind ladder, `Toward` a ghost.
- **A gap is not a license to widen.** No wait verb, no rest, no rumour unless R5(a), no ally awareness (Regroup), no runner (Alarm), no stance on the mind.
- **Rules stay in resolution / the world stays in encounter.** No resolution change: nothing a world turn does rolls dice, because `Attack` cannot happen there.
- **Delete, don't strand.** `Decider` and `Pump` go, with the ADR amended, rather than a third seam beside two.

## Rulings

- **R1** — Time passes only because the party acts; a Move pays one round per pace, an action verb pays one round, standing still is free. *(Recommended yes; the alternative is a wait verb later, not a timer.)*
- **R2** — The unit is the round, and every advance names its member as the driver (max, not sum). The two `"world"` sites change. *(Recommended yes.)*
- **R3** — One driver seam: `Decider`/`Pump` deleted, the world thinks through `TurnDriver` with a movement-only budget, `Attack` off-turn is a refusal. ADR-0043 amended. *(Recommended yes.)*
- **R4** — The view says whom a creature opposes; a mind never targets an unopposed member; a creature with nothing to oppose passes. *(Recommended yes; without it a neutral goblin advances on the party the first time it thinks.)*
- **R5** — The arrived thug: (a) rumour on fact-arrival this slice, or (b) defer to Alarm. *(Recommended (b).)*

## Build order, once ruled

1. rpg-project: this doc merged; tracking issue.
2. rpg-toolkit, one module per PR, on pseudo-versions:
   - `rulebooks/dnd5e/encounter`: pace + action time on world verbs; drivers named; `worldThinks` on every raise incl. the round site; `Opposed` on the view; `Decider`/`Pump` deleted; ADR-0043 amended; workbench `pump` command removed.
   - `rulebooks/dnd5e/behavior`: `Basic` and `Minded` target only opposed members.
   - `rulebooks/dnd5e/session`: repin; no logic (verify the beats project; `Tick` beat mapped if it is not).
   - (R5a only) encounter: rumour on fact-arrival.
3. rpg-api and web: repin only.
4. Kirk walks the front room: persuade fails, bandits arrive, the party walks and the world moves with it; a goblin cowed into the next room is pursued or not by stance; a fight in the hall while a thug two rooms away closes one round at a time.
5. Merge inside-out.

## Done when

- Walking the party six cells advances the clock one round; four members walking together advance it once.
- A cowed creature keeps running on the party's steps until its fear lapses, through the mind, not `fleeFrom`; `fleeFrom` stays as the answer word's immediate step (its deletion is the mind-driven `flee`'s own PR, when the coward preset is on the goblin).
- A creature that lost sight of the party walks toward where it last saw them, one round per pace of the party's.
- A neutral creature given time passes.
- During a fight, a creature outside it moves once per round; if it walks into sight of an enemy it joins the fight by the shipped path.
- `Decider` has no references; `Pump` has no references; ADR-0043 says why.
- Every claim above is a unit test; the walk proves wiring.
