# Brainstorm — the clock: turn and round boundaries reach the rules

Kirk, 2026-08-27: *"i think the clock is next getting turn and round end events
flowing i think is really important."*

This is the map, not the design. Where the ground is already correct, this says
so; where a decision is genuinely open, it names it and stops.

---

## 1. What is actually broken

Not "the clock does not tick." The clock ticks correctly and has for a long
time. **The entire temporal-boundary vocabulary is listen-only in production.**

| topic | its only publisher | reachable from the game? |
|---|---|---|
| `TurnStartTopic`, `TurnEndTopic` | `combat.TurnManager` (`turn_manager.go:154,177`) | **No.** `NewTurnManager(` has zero call sites in the whole toolkit — tests included. Dead code from the pre-session stack. |
| a round boundary | — | **No topic is defined at all.** |
| `RestTopic` | `Character.ShortRest` / `Character.LongRest` (`character.go:477,512`) | **No.** Zero non-test callers in the toolkit or in rpg-api. |
| `CombatEndTopic` | `Character.EndCombat` (`character.go:538`) | **No.** Same — zero callers. |

Against that, the subscriber side is fully built out. Seven conditions
subscribe to a turn boundary in their `Apply`:

| condition | subscribes to | what it does there |
|---|---|---|
| `dodging` | TurnStart | removes itself at the owner's next turn |
| `reckless_attack` | TurnStart | removes itself (`Reason: "turn_start"`) |
| `unconscious` | TurnStart | **rolls the death save** |
| `helped` | TurnStart | safety-net removal (keyed on `HelperID`) |
| `sneak_attack` | TurnEnd | resets `UsedThisTurn` |
| `disengaging` | TurnEnd | removes itself |
| `raging` | TurnEnd | `TurnsActive++`, ends rage on 10 or on no combat activity |

And one more that is easy to miss: `character/sheet_keeper.go:217` attaches
every `combat.RecoverableResource` to the bus, in its own words, *"so it hears
rests"* — and each one subscribes to `RestTopic` (`recoverable_resource.go:120`).

> **Nothing in the game has ever recovered a resource.** Rage uses, Second Wind,
> Ki — spent once, gone for the life of the character. Not a bug in recovery;
> recovery is unreachable.

That is the "Recover" third of journey **#241 — See, Spend, and Recover Class
Resources**. It is not unimplemented, it is structurally impossible, and it
becomes possible the moment a boundary is published.

### 1b. This is the effect-context bug wearing the other glove

Last slice: conditions **read** an ambient channel nobody filled, and the
absent value was silently wrong (`effect-context-slice`, `fail-closed-not-fail-silent`).

This slice: conditions **listen** on a channel nobody speaks on, and silence is
indistinguishable from "the boundary did not happen."

The tell is identical, and it is worth staring at. `dodging_test.go:234`,
`helped_test.go:153`, `unconscious_test.go:522` all call
`TurnStartTopic.On(bus).Publish(...)` **by hand**. Every one of those tests
passes. Every one of them supplies the thing production does not. A green suite
is not evidence when the suite is the only publisher.

---

## 2. What is already right, and should not be rebuilt

`play/clock.Turn.End` (`turn.go:120-136`) already returns exactly the sequence a
boundary needs, already in causal order, already carrying the round:

```
TurnEnded(active, round)
   → RoundStarted(round+1)      // only when the order wrapped
   → TurnStarted(next, round)
```

`Turn.SetOrder` (`turn.go:50`) does the same when a fight forms:
`RoundStarted(1)`, `TurnStarted(first, 1)`.

`Milestone` says what it is for in its own doc: *"a temporal boundary returned —
never published — by the verb that caused it, in causal order (design R4).
Turn-emitted milestones carry the clock's Round at the moment of emission."*

**And the encounter throws every one of them away.** There is not a single
mention of `Milestone` in the whole `rulebooks/dnd5e/encounter` package outside
tests. `clocks.go:461` — inside `driveOneMonsterTurn`, where a driven monster's turn
actually ends — is literally:

```go
return seq, out.RoundWrapped, nil
```

The sequence is computed, ordered, correct — and collapsed one layer up into a
single `bool`. So the work is not "detect boundaries." It is **stop discarding
the boundaries we already detect.**

`TurnStartEvent` and `TurnEndEvent` already carry `Round int`
(`events.go:583-593`). The round *number* already flows. What has never existed
is a round *boundary*.

---

## 3. Where the bridge is allowed to live

This is constrained by contracts already in force, not open:

- **`play/*` may not publish.** Their numbered contract (`play/README.md:27`):
  a leaf *"returns its results as values, and never publishes to a bus."*
  `Milestone`'s own doc repeats it — *"returned — never published."* And the
  reason is not style: *"a module that returned its results by publishing to a
  bus would need a live subscriber at the moment it ran — which is exactly the
  thing that does not survive a process restart."* So the clock cannot do it,
  and should not.
- **`encounter` holds no bus** — but it is where detection belongs.
  ADR-0038: the composition raises interactions and stays bus-free. And
  `play/README.md:53`: *"the composition is the first layer allowed to have an
  opinion about the game — rules and trigger detection live there, never down
  here."* Surfacing the milestones is the composition doing its stated job.
- **`resolution` is the only place a bus exists.** That is the whole of ADR-0038.

Which leaves exactly one shape, and it is the one Kirk described on 2026-08-26:

> **A turn boundary becomes an interaction.**

```
play/clock       computes the milestones          (already correct)
   ↓ returns them as values
encounter        surfaces them instead of dropping them
   ↓ returns them as data
session          raises them as an interaction
   ↓ Resolve
resolution       attaches everyone (R3), publishes each milestone in order,
                 tears down (R5), hands back dirty sheets
   ↓ DirtyCharacters / DirtyMonsters
session          saveDirty — already written, already works
```

Every arrow in that diagram already exists except the two middle ones.

### Why `MarkDirty` was the prerequisite

A boundary interaction's *entire* output is condition state. No damage, no
economy, no HP change to ride along on. Before `MarkDirty` (#1254) every expiry
would have fired correctly and been silently discarded on the way out. That is
why this slice comes second and not first.

---

## 4. Use cases — walked one at a time

**U1. A rogue sneak-attacks, then the round comes back around.**
`UsedThisTurn` must be false again. Today it is set once and never cleared, so a
rogue gets sneak attack exactly once per *fight*.

**U2. A fighter Dodges; on their next turn it lapses.**
`dodging` removes itself on TurnStart. Today it never lapses — Dodge is
permanent for the rest of the encounter. (`experience/design.md` nominates Dodge
as the proving case for the generic execution contract; this is why.)

**U3. A barbarian rages and stops fighting.**
RAW: rage ends if you neither attacked nor took damage that turn. `raging`
implements this correctly on TurnEnd, and TurnEnd never arrives — so rage is
permanent, and `TurnsActive` is permanently 0.

**U4. A character drops to 0 and starts dying.**
`unconscious` rolls the death save on TurnStart. Today **death saves never
roll.** A downed character is stable-by-accident forever, which is not a rule
anyone wrote.

**U5. A round wraps.**
The order returns to its first member and the round advances. Nothing observes
this today. Nothing *needs* to yet — see Q2.

**U6. The fight ends and the party takes a short rest.**
Second Wind, Ki and rage uses come back. Today they never do, because
`RestTopic` has no reachable publisher and no session verb produces a rest at
all. Parked — see §6.

**U7. One `EndTurn` crosses several boundaries.**
`encounter.EndTurn` drives every consecutive unplayed member forward inside one
call (#1162, ADR-0043). A player ending their turn with three goblins after them
crosses **four** turn-ends, four turn-starts, and possibly a round wrap — in one
call. So the interaction publishes a *sequence*, not an event. This is exactly
what `Milestone`'s "in causal order" was built for, and it is the single
strongest argument for carrying the milestones rather than reconstructing
boundaries from `Next` and `RoundWrapped`.

**U8. A fight forms.**
`SetOrder` emits `RoundStarted(1)` and `TurnStarted(first, 1)`. If `Form` does
not publish them, the first turn of every fight begins without a turn-start —
so U2 and U4 are both wrong on turn one specifically. Whatever answers `EndTurn`
must answer `Form` too.

---

## 5. Open questions — for Kirk to rule on at design

**Q1. Does the boundary vocabulary grow a round topic, or does round stay a field?**

Turn events already carry `Round int`. A round boundary has a well-defined slot
(between the last member's turn-end and the first member's turn-start) and
`play/clock` already computes it. But *nothing subscribes to it yet*: all seven
conditions above are turn-scoped, and rage's "10 rounds" is counted correctly by
counting the owner's own turn-ends, since those happen once per round.

The case **for** defining it now: Kirk asked for it by name; it is where
round-scoped durations, legendary-action resets and lair actions will land; and
it is additive and cheap. The case **against**: ADR-0007's lesson is not to seal
an enumeration against hypotheticals, and today the only subscriber would be
nobody.

*Leaning:* define it. It is not hypothetical — the clock genuinely crosses this
boundary and currently has no way to say so, and the alternative is that the
first round-scoped rule has to invent both the topic and its publisher at once.

**Q2. If round-end exists, does `play/clock` grow a `RoundEnded` milestone?**

`play/clock` emits `RoundStarted` on the wrap and never `RoundEnded`. The
milestone const block calls itself *"the closed v1 milestone kind set."* So
either the set grows a kind, or the bridge derives "round N ended" from "round
N+1 started."

*Leaning:* grow the set. Deriving it puts a temporal rule in the wiring, and
"the rule that lives in the wiring" is the exact failure this codebase keeps
catching itself in. The set being "closed v1" is a reason to be deliberate about
adding to it, not a reason not to.

**Q3. `TurnEndEvent.CharacterID` — but monsters end turns.**

The field is character-shaped. Monsters take turns, and `driveMonsterTurns` ends
several per call. Publishing a monster's turn-end into a field named
`CharacterID` is the same defect that got `gamectx.CharacterRegistry` deleted
last slice — resolution's own doc says it was *"character-shaped, so it could
not describe a monster at all."*

Nothing subscribes to a monster's turn boundary **today**, so this is latent
rather than live. But it is free to fix now (`no-backcompat-baggage`: rpg-api
pins old versions, renames are free until adoption) and expensive to fix after
monster traits start listening.

*Leaning:* rename to a neutral id in the same PR that first publishes it.

**Q4. Does the step vocabulary have to grow?**

No — and this is worth recording rather than assuming. A boundary machine is a
sequence of "do this on the bus and hand me the next step," which is what
`Gather` already does for the contest's imposition (`doc.go`, "The bus-effect
tally"). So no new `Step` case.

But `doc.go` explicitly names the trigger for reopening that question: *"if wave
5's reactions multiply the pure-effect steps ... the three-to-one ratio reverses
and the argument reverses with it."* A boundary machine multiplies exactly those
steps. It does not force a new case, and it does move the tally — which `doc.go`
asked to have noticed rather than left to memory.

**Q5. Cost.** A boundary has no declaring actor and nothing to pay. `Input.Cost`
nil is a free action and is the common case, so this needs nothing new — noted
only so nobody later reads the nil as an omission.

---

## 6. Parked — and the corner each one must not close

Same discipline as the effect-context brainstorm: not designed here, only kept
from being painted into.

| parked | why not now | the corner it must not close |
|---|---|---|
| **Rest** (U6) | There is no rest verb anywhere — not in `session`, not in rpg-api. This is a verb to design, not an event to publish. | The boundary mechanism must not assume its boundaries come from a *clock*. A rest is a boundary raised by a player's decision, not by time advancing. |
| **Combat end** | `session.Dissolve` already exists with `ByDecision`/`ByDefeat`, so this is the *closest* follow-on — genuinely just "publish it too." | Whatever carries boundaries must be able to carry one that ends the interaction's own world, not merely advances it. |
| **Reactions / suspension** | ADR-0038's `Pose` step is unbuilt; opportunity attacks fire on movement, not on boundaries. | A boundary interaction must not become the only thing that can publish mid-interaction, or reactions will have to be bolted onto it. |
| **Legendary / lair actions** | No monster has them yet. | Q1's answer. A round boundary with no event is the corner: these fire on the round, not on anyone's turn. |
| **Concentration** | No spells with duration yet. | Durations counted in *rounds* need a round boundary that does not belong to any member's turn. Again Q1. |
| **Surprise** | `Form` records `surprised` in the beat and the story carries it; a surprised creature loses its first turn. | Turn-start publishing must be able to say a turn started *and was skipped*, or surprise will need a second mechanism. |

---

## 7. What proves it

Not a unit test that publishes the event by hand — that is precisely the trap
this slice exists to escape. The proving test drives a **real `session.EndTurn`**
and reads the boundary's effect out of the **persisted sheet**:

- a rogue's `used_this_turn` is `false` in the stored blob after their turn ends;
- a barbarian who neither attacked nor was hit is **no longer raging** after
  their turn ends;
- a dodging fighter is no longer dodging after their own next turn starts.

Each of those is a value that can only have come back through `MarkDirty` →
`DirtyCharacters` → `saveDirty`, which is the whole chain under test.

And structurally, in the shape `TestNoCodePathProducesACastlessInteraction`
established: **no code path ends a turn without raising its boundaries.** That
claim cannot be made by a behavioural suite, for the same reason it could not be
made last slice — the defect *is* that tests supply what production does not.
