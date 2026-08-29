# Combat end — the boundary that ends the world it is announced in

**Slice:** rpg-project#295, part 1 of 3 (combat end → rage → rest)
**Status:** design, awaiting rulings
**Predecessor:** [the clock slice](../clock/design.md) — shipped 2026-08-27

---

## 0. Why this one first

Rage is the reason, and the dependency is arithmetic rather than taste.

`RoundActivated` — the field the rage rewrite turns on — is subtracted from
`event.Round` to get elapsed rounds. **Round numbers are per-fight.**
`clock.Turn.SetOrder` sets `t.round = 1` for every bubble
(`play/clock/turn.go:48`) and `clock.Turn.Dissolve` sets it back to `0`
(`turn.go:301`). There is no session-global round counter and there never was.

So a rage that outlives its own fight carries a `RoundActivated` from a clock
that no longer exists. In the next fight, round 1, `1 - 3 = -2`: the 10-round
cap never fires and the rage is permanent. **Ending rage when the fight ends is
what makes the subtraction sound by construction**, rather than by a rule
somebody has to remember to write.

That is a real prerequisite, not a sequencing preference.

---

## 1. What is actually there today

The clock slice found the turn vocabulary listen-only. Combat end is the same
shape, one door over.

| piece | where | state |
|---|---|---|
| `CombatEndEvent{CharacterID}` | `events/events.go:672` | defined |
| `CombatEndTopic` | `events/events.go:929` | defined |
| **publisher** | `character/character.go:532` `Character.EndCombat` | **zero callers, anywhere, tests included** |
| **subscriber** | `conditions/raging.go:173` → `onCombatEnd` (`:317`) | attached on every rage, has never once fired |

`grep -rn "EndCombat" --include="*.go"` across the toolkit and rpg-api returns
exactly one line: the definition itself. **Rage has never ended because combat
ended.** It has ended by rest, by unconsciousness, and by the RAW activity check
— never by the fight being over.

### 1.1 The one subscriber decides the shape

```go
// conditions/raging.go:317
func (r *RagingCondition) onCombatEnd(ctx context.Context, event dnd5eEvents.CombatEndEvent) error {
	// Only end rage if this is our character
	if event.CharacterID != r.CharacterID {
		return nil
	}
	return r.endRage(ctx, "combat_ended")
}
```

**This settles the fan-out question before it can be argued.** Every attached
effect in a fight hears every publish on the interaction's one bus (R1), and
this handler answers "is this about me?" by comparing IDs. A single subject-less
"the fight ended" event would match **nobody**, and rage would go on exactly as
it does today.

So combat end is **one boundary per member**, which is also what keeps
`Boundary.Subject`'s stated invariant — *"Never empty: both kinds are about
somebody"* — true rather than newly qualified. That the existing invariant
survives untouched is the strongest available signal the fan-out belongs here.

### 1.2 The corner the clock slice left open, and whether it held

The clock brainstorm parked combat end with a named constraint:

> *"Whatever carries boundaries must be able to carry one that ends the
> interaction's own world, not merely advances it."*

**It held.** Three things had to be true and all three are:

1. `announcerSeam.Announce` (`session/announcer.go:58`) is written against
   `[]encounter.Boundary` and never inspects `Kind`. A new kind costs it nothing.
2. `resolution.NewBoundary` dispatches through `boundaryTopics`, a **sealed map**
   (`resolution/boundary.go:102`) — *"if one grows, this refuses until the other
   does."* The new kind is one map entry, and the seal means forgetting it is a
   loud refusal at the door rather than a silent no-publish.
3. The post-dissolve world is a legal world. `enc.Members()` still answers (a
   dissolve re-homes members to the world clock, it does not un-member them) and
   `enc.ToData()` is an ordinary out-of-combat snapshot.

**This section is the design's own report card on the last one.** The corner was
written down as a guess; it is now checked, and the answer is that the seam is
reusable unchanged. `session` therefore has no production change in this slice at
all — only tests and pins.

---

## 2. Where it announces, and the re-entrancy question

### 2.1 One choke point, already

`dissolveBubble` (`encounter/dissolve.go:170`) is the single place a fight ends,
and its own doc says so:

> *"Both endings run exactly this: the same re-homing, the same prune, the same
> beat. Only the cause differs."*

Two callers, both of which must announce:

| caller | cause | reached from |
|---|---|---|
| `Encounter.Dissolve` (`dissolve.go:157`) | `ByDecision` | a host verb — `session.Dissolve` |
| `noticeDown` (`standing.go:232`) | `ByDefeat` | **the composition noticing**, mid-verb |

Announcing inside `dissolveBubble` covers both by construction. Announcing at the
call sites would be two chances to forget, and one of them is not a call site a
host controls.

### 2.2 The defeat path runs inside a Strike — and that is fine

This was the one thing worth checking before committing to the shape, because
`ByDefeat` fires from a stack several frames deep inside another verb:

```
session.EndTurn
└─ openForWrite
   └─ enc.EndTurn
      └─ driveMonsterTurns → driveOneMonsterTurn → executeTurnIntent
         └─ striker.Strike                          (clocks.go:558)
            └─ strikerSeam.Strike                   (session/striker.go:56)
               ├─ resolution.Resolve  ── the swing ──── RETURNS
               ├─ saveDirty                        ──── RETURNS
               └─ enc.Record                          (session/striker.go:139)
                  └─ noticeDown → fightIsDecided → dissolveBubble(ByDefeat)
                     └─ announce → resolution.Resolve  ── the boundary
```

**`enc.Record` is called after the strike's own `resolution.Resolve` has
returned** (`striker.go:113` resolves, `:134` saves, `:139` records). So the
boundary's `Resolve` starts on a stack with no other `Resolve` on it. **No
re-entrancy.**

That is not luck; it is the shape `striker.go` already committed to — resolve the
swing, persist it, *then* tell the world what happened. Recording inside the
resolution would have made this slice impossible without moving it first.

The nesting that *does* remain — a `Resolve` inside `enc.EndTurn` inside
`openForWrite` — is exactly what `strikerSeam` already does on every monster
swing. Nothing new is being asked of the stack.

**Worth pinning as a test rather than a comment**, because it is a property of
`striker.go`'s ordering that a future refactor could quietly reverse.

### 2.3 Announce last, after the beat

`dissolveBubble` ends with `appendClockBeat("bubble-dissolved")` and returns.
The announce goes **after the beat, immediately before the return** — the same
order `driveOneMonsterTurn` already uses (`clocks.go:453` appends, `:468`
announces).

The story says the fight ended before anything reacts to the fight having ended.
That also preserves Kirk's beat-order ruling from rpg-project#269 §6.6 —
`down → bubble-dissolved → ended` — since the close (`TriggerMemberDown`) happens
later in `noticeDown`, after the bubble logic.

An announce failure returns the error and fails the caller's whole verb, which is
`Announcer`'s stated contract and matches `Striker` exactly.

---

## 3. The other "fight ended" mechanism, and why both stay

`session` already has an end-of-fight cleanup: `exitDissolvedCombatants`
(`write.go:837`), run from `commit` (`:776`) **before** the save. It clears the
action economy of every player whose fight this call dissolved, and it finds them
by re-reading `bubble-dissolved` beats out of the story.

Two mechanisms for "the fight ended" is the two-truths shape this codebase keeps
catching itself in, so it is worth stating plainly why this is not that:

|  | `exitDissolvedCombatants` | the combat-end boundary |
|---|---|---|
| **what** | one flag: `Character.ExitCombat` / `InCombat` | whatever any attached effect decides |
| **when** | at `commit`, after the verb's work | at the crossing, mid-verb |
| **how** | re-reads the story's beats | the bus, like every other boundary |
| **who decides** | `session` | each condition, for itself |

They are not two accounts of one fact. One resets the action-economy ignition
(`readyForTurn`'s `!sheet.InCombat()` branch); the other is the rules layer
hearing that time did something. **Neither can do the other's job**: the boundary
cannot reach `InCombat` because `session` holds no bus and `resolution` holds no
`writeScope`; `exitDissolvedCombatants` cannot expire a condition because it
loads a sheet outside any interaction.

### 3.1 Ordering is safe, and it is safe for a checkable reason

They both write the same sheet in the same verb, in this order:

1. boundary announce → `saveDirty` → `SaveCharacter` (rage removed, persisted)
2. `commit` → `exitDissolvedCombatants` → `exitCombatIfPlayer` (`write.go:906`)
   → `fetchCharacterData` → `SaveCharacter`

Step 2's read is `m.characters.GetCharacter` **direct, uncached**
(`entities.go:80`), so it sees step 1's write. The economy clear lands on top of
the rage removal rather than under it.

`saveWalker` already dedupes the report entry for exactly this case — its own doc
names *"a killing swing's own saveDirty, then this sheet readied and saved again
for something later in the same verb"* (`move.go:353`). This slice adds one more
instance of a pattern that was already found and fixed.

**This is an assumption with a shelf life** — it is true because the fetch is
uncached today. It gets a test rather than a paragraph.

---

## 4. Layer by layer

### 4.1 `play/clock` — unchanged

The `Dissolved` milestone already exists (`milestone.go:28`) and
`Turn.Dissolve` already emits it with the round it ended on (`turn.go:304`).
Nothing to add.

**And the fan-out does not belong here.** A clock dissolving is one thing that
happened to a clock; the milestone carries no `Subject` and should not. That a
fight's ending is something that happens *to each member* is composition
knowledge — the same kind of knowledge as `MemberID` vs `core.EntityID`. Pushing
per-member `Dissolved` milestones into the leaf would put a rulebook's opinion in
a package that is not allowed to have one, and would add a sixth module to the
chain to do it.

The closed v1 milestone set stays closed, as ruled 2026-08-27.

### 4.2 `rulebooks/dnd5e/events` — the rename

```go
type CombatEndEvent struct {
    SubjectID string // whoever's fight just ended — a character, or a monster in it
}
```

Kirk's rule from the clock slice, applied to the next event that needs it:

> *"I think subject is better. we should be naming things for what they represent
> not what we currently have plugged in"*

Same reasoning as `TurnStartEvent`/`TurnEndEvent`: monsters are in the fight that
ended, so the field is not character-shaped. Free now (`no-backcompat-baggage`;
rpg-api holds zero references to `CombatEnd*`), expensive once anything on a
monster subscribes.

Touches: `raging.go:319`, `character.go:537`, and the two `raging_test.go` sites.

### 4.3 `rulebooks/dnd5e/encounter` — a third kind, and the fan-out

```go
const (
    TurnStarted BoundaryKind = "turn_started"
    TurnEnded   BoundaryKind = "turn_ended"
    CombatEnded BoundaryKind = "combat_ended"   // new
)
```

`BoundaryKind`'s doc currently opens *"TWO KINDS, because two are what anything
publishes."* It becomes three, and the paragraph explaining why there is still no
**round** boundary stays exactly as it is — that reasoning is untouched by this.

The fan-out, next to `boundariesFrom` and deliberately separate from it:

```go
// combatEndBoundaries fans one fight's ending out to the members it ended for.
//
// NOT part of boundariesFrom, because it cannot be: the leaf's Dissolved
// milestone carries the round but no subject — a clock knows it emptied, not
// who it emptied of — and the roster arrives beside the milestones rather than
// inside them. The fan-out is this composition's own knowledge.
//
// Per member rather than once for the fight because the only subscriber that
// exists compares the event's subject against its own owner
// (conditions/raging.go:317). A subject-less ending would match nobody.
func combatEndBoundaries(ms []clock.Milestone, members []core.EntityID) ([]Boundary, error)
```

It returns an **error** when `ms` holds no `clock.Dissolved`. `bubble.Dissolve`
just returned success, so its absence is a broken contract, not an empty result —
and a silent empty here is precisely how `CombatEndTopic` came to have a
subscriber and no publisher. Fail closed, loudly.

`boundariesFrom` gains an **explicit** `case clock.Dissolved: continue` with a
comment pointing at `combatEndBoundaries`, so that dropping it reads as a
decision rather than as the `default` arm swallowing something.

`dissolveBubble` announces after its beat:

```go
crossed, cerr := combatEndBoundaries(out.Milestones, out.Members)
if cerr != nil { return nil, fmt.Errorf("dissolve: %w", cerr) }
if aerr := e.announceBoundaries(crossed); aerr != nil {
    return nil, fmt.Errorf("dissolve announce: %w", aerr)
}
```

`announce(ms []clock.Milestone)` (`clocks.go:516`) splits: the translating half
keeps its name, and the half that calls the capability and skips an empty set
becomes `announceBoundaries([]Boundary)`, which both paths share. No behaviour
change to the three existing call sites.

### 4.4 `rulebooks/dnd5e/resolution` — one map entry

```go
encounter.CombatEnded: func(ctx context.Context, bus events.EventBus, b encounter.Boundary) error {
    return dnd5eEvents.CombatEndTopic.On(bus).Publish(ctx, dnd5eEvents.CombatEndEvent{
        SubjectID: string(b.Subject),
    })
},
```

`CombatEndEvent` carries no round — combat ending is not a coordinate in a clock
that no longer exists. `Boundary.Round` is still populated and still meaningful
(*which* round it ended on, for the record); it simply has nothing to be
published into.

Nothing else in the package changes. The machine, `Gather`, and
`BoundaryOutcome.Announced` all already do the right thing for N crossings.

### 4.5 `rulebooks/dnd5e/session` — **no production change**

The seam is generic. This PR is the acceptance tests and the pin bumps, and the
fact that it is only that is §1.2's evidence.

### 4.6 `rpg-api` — bump

No new capability, so no wiring. `RefusingAnnouncer` in `sessionworld.go` stays
right: that world never dissolves anything either.

---

## 5. Open questions — for Kirk

**Q1. Do monsters get a combat-end boundary?**

The fan-out covers `DissolveOutput.Members`, which is everyone the fight held —
players, monsters, and the dead of both.

*Leaning: yes, all of them.* It costs a few no-op publishes (nothing on a monster
subscribes to `CombatEndTopic` today), and the alternative is a filter in the
composition that decides which members count as "in" a fight — a rule in the
wiring, and the wrong layer to hold it. R3 says pass everyone in and let the
effect's own predicate decide.

**Q2. Does `Character.EndCombat` get deleted?**

It is a public method with zero callers that publishes a combat-end for one
character in isolation. After this slice it is a second publisher of a topic that
finally has a real one — and a footgun: calling it ends that character's rage
without any fight having ended.

*Leaning: leave it, this slice.* The clock slice left `combat/turn_manager.go:154`
publishing turn boundaries under exactly the same circumstances, and consistency
beats a drive-by. **But name it as a debt rather than let it sit** — both dead
publishers should go in one pass, and that pass is not this one.

**Q3. Does a member *leaving* a running fight get a combat-end?**

No, and the mechanism already handles it correctly — recorded so it does not get
re-litigated. A body spliced out by `noticeDown`, or a straggler transferred to
the world clock, has not had their combat end: the fight goes on. RAW 2014 ends
their rage anyway, through the activity check, because they have neither attacked
nor been hit since their last turn. Unconsciousness ends it immediately through
`onConditionApplied`. **The boundary is about the fight ending, not about a member
leaving it.**

---

## 6. What proves it

The same discipline as the clock slice, and for the same reason: **`raging_test.go:730`
and `:774` already pass by publishing `CombatEndEvent` by hand.** They are green
today against a publisher that does not exist. No new test may do that.

Behavioural, through real session verbs, read out of the **persisted sheet**:

| proof | what it can only pass if |
|---|---|
| a raging barbarian is no longer raging after `session.Dissolve` | the decided ending announces at all |
| a raging barbarian is no longer raging when the **last monster drops mid-`EndTurn`** | §2.2 — the self-dissolve announces from inside a Strike, on a live stack |
| a barbarian who was **not** the member named in `DissolveInput` still stops raging | §1.1 — the fan-out reaches every member, not just the one addressed |
| the same sheet's action economy is **also** clear afterwards | §3.1 — `exitDissolvedCombatants` reads the post-boundary sheet, not a stale one |

Structural, in the shape `TestNoCodePathLoadsAWorldWithoutNamingAnAnnouncer`
established:

- **`dissolveBubble` has exactly two callers, and both are covered above.** An
  AST assertion on the count, so a third way to end a fight cannot be added
  without this test failing and pointing at the two named tests.
- **`boundaryTopics` covers every declared `BoundaryKind`.** The seal refuses an
  unknown kind at runtime; this makes the gap a build-time failure instead, which
  is where "a kind with no publisher" should have been caught the first time.

Every test then gets mutated, per standing practice — the clock slice's first
ordering test survived its own mutant and had to be rewritten.

---

## 7. Scope

**In:** the `CombatEnded` kind, the per-member fan-out, the announce in
`dissolveBubble`, the `SubjectID` rename, one topic-table entry, tests, bumps.

**Out:**

- **Rage's rewrite.** Part 2. This slice makes `RoundActivated` arithmetic sound
  (§0); it does not yet write it.
- **Rest.** Part 3, and its own design — there is no rest verb anywhere.
- **Deleting the two dead publishers.** Q2.
- **`arcade_recovery.go:51`'s stale comment**, which describes *"the encounter
  package's end-of-combat condition sweep (rpg-toolkit#753)"* — a mechanism that
  left with the old encounter module (rpg-api#801). The comment is now
  load-bearing prose about something that does not exist. Noted, not fixed here.

## 8. Module chain

```
rulebooks/dnd5e  →  encounter        →  resolution  →  session      →  rpg-api
  (SubjectID)       (CombatEnded,       (one map       (tests +        (bump)
                     fan-out,            entry)         pins only)
                     announce)
```

Five, again — and `play/clock` stays out of it for the second slice running.
