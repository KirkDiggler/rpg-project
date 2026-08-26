# Design — a boundary is an interaction

Companion to `brainstorm.md`, which holds the evidence. This is what to build.

**Kirk rules once on this document.** Three questions were open and marked
**[RULING]**; everything else follows from contracts already in force.

**Q1 and Q2 RULED 2026-08-27: no round topic, and `play/clock` is untouched.**
See §3.1. Q3 is still open.

---

## The one sentence

`play/clock` already computes every temporal boundary correctly and hands it
back as a value. **Each layer above stops throwing it away, and the layer that
owns a bus publishes it.**

---

## 1. The shape, and why it is not free choice

```
play/clock      computes milestones, returns them        — already correct
   ↓ values
encounter       carries them instead of dropping them,
                and ANNOUNCES each one as it is crossed
   ↓ Announcer capability (synchronous, mid-verb)
session         raises a boundary interaction
   ↓ resolution.Resolve
resolution      attaches everyone (R3), publishes each
                boundary in causal order, tears down (R5)
   ↓ DirtyCharacters / DirtyMonsters
session         saveDirty — already written, already works
```

The constraints that produce this and admit no alternative:

- `play/*` **returns values and never publishes** (`play/README.md:27`), because
  *"a module that returned its results by publishing to a bus would need a live
  subscriber at the moment it ran — which is exactly the thing that does not
  survive a process restart."*
- `encounter` holds no bus (ADR-0038) but **is** where detection belongs:
  *"the composition is the first layer allowed to have an opinion about the
  game — rules and trigger detection live there, never down here."*
- `resolution` is the only place a bus exists (ADR-0038, R1).

## 2. The announcer must be a capability, not a return value

This is the load-bearing decision, and it is forced by a temporal fact.

`encounter.EndTurn` drives every consecutive unplayed member forward **inside
one call** (#1162). Those driven monsters attack during the drive — through
`Striker`, which calls `resolution.Resolve` mid-loop. So if boundaries were
merely returned and published afterwards, **every driven monster's turn-start
would be published after that monster had already attacked.**

Today that is invisible: no monster trait subscribes to a boundary. That is
exactly the shape of defect this whole slice exists to stop building — correct
by coincidence, silent when it stops being.

So `encounter` gets a capability it calls **at the moment the boundary is
crossed**, supplied at construction and never defaulted.

**This is the third instance of a pattern already in the codebase**, not a new
one:

| capability | called | implemented by | does |
|---|---|---|---|
| `Decider` | world clock tick | session | decides a monster's free-roam move |
| `TurnDriver` | fight, member's turn | session | decides a monster's turn |
| **`Announcer`** | **every clock advance** | **session** | **raises the boundary interaction** |

`Striker` is its closest twin and the precedent to copy exactly: bound to the
live `writeScope`, called mid-loop, calls `Resolve`, and — critically —
**never adopts a new `*encounter.Encounter`**, because *"swapping it out from
inside a call enc itself made would orphan that loop"* (`striker.go:44`). The
boundary interaction reads `out.DirtyCharacters` / `out.DirtyMonsters` and
discards the returned world for the same reason: conditions live on **sheets**,
not on the encounter's story-and-position state.

```go
// encounter
type Announcer interface {
    // Announce raises the boundaries one clock advance crossed, before the
    // next member acts. A Go error aborts the caller's whole verb.
    Announce(ctx context.Context, enc *Encounter, crossed []Boundary) error
}
```

Called **once per clock advance**, carrying that advance's ordered turn
boundaries — the publishable part of what `clock.EndOutput.Milestones` already
returns. Ordinary advance: `TurnEnded(a, N)`, `TurnStarted(b, N)`. On a wrap the
same two, with the second carrying `N+1`.

**Required at construction, never defaulted** — the law this codebase already
states for `TurnDriver`: *"a nil answer here would be this module guessing a
rule instead of asking for one."* A missing announcer is not "boundaries are
off," it is "conditions silently never expire," which is the current bug.

### `RefusingAnnouncer`, and the six construction sites

`Striker` already solved this and the answer is to copy it verbatim.
`encounter.RefusingStriker{}` exists precisely because some callers load an
encounter that must never drive a turn, and `read.go:383` states the rule:
*"The caller says which: a real one bound to a write verb's own scope, or
`RefusingStriker{}` for a read that must never drive a turn."*

`RefusingAnnouncer` is its twin. A no-op default would be
`fail-closed-not-fail-silent` in reverse: the absent value would not say what
the author meant, and "boundaries silently stopped" is the bug this slice is
fixing.

Every site that loads an encounter has to name one:

| site | announcer | why |
|---|---|---|
| `session/write.go:676` (`adopt`) | **real** | the write path that advances clocks |
| `session/start.go:158` (`loadAuthored`) | refusing | already passes `RefusingStriker` for the same reason |
| `session/read.go:383` | refusing / caller's choice | mirrors how `striker` is already threaded |
| `resolution/resolve.go:289` | refusing | see below |
| two `cmd/` workbenches | their own | |

**Resolution's own load takes the refusing one, and this is not a recursion
hazard.** It reads as one at first — resolution loads an encounter, an announcer
calls resolution — but `Striker`'s comment at that exact call site already
settles it: this package *"runs ONE interaction machine against a loaded
snapshot and returns — it never drives a monster's whole turn (that is
driveMonsterTurns' own job, reached through EndTurn/form, **neither of which
this package's Resolve calls**)."* No clock advances inside a resolution, so no
boundary is crossed inside one, so an announcement there is a bug — named
loudly rather than recursed into.

That leaves exactly one real announcer in the system, on exactly one path.

## 3. Layer by layer

### 3.1 `play/clock` — **unchanged** · RULED 2026-08-27

This section proposed adding a `RoundEnded` milestone to what the const block
calls *"the closed v1 milestone kind set."* **Kirk ruled against it, and the set
stays closed.**

His reasoning, and it is the right one:

> *"rage is from my turn to my turn. If I am last in initiative I keep rage
> until the end of my next turn. what would need round end?"*

Nothing does. Rage is RAW-explicit that it lapses when **your turn** ends, and
its 1-minute duration expires at the end of **your** turn on the tenth round —
which `raging.go:267` already implements by counting its own turn-ends, correctly,
because a member acts exactly once per round (`economy.go:68`).

Searching for a counter-example found none. Every duration phrasing in 5e
resolves on a turn — *"until the start of your next turn"*, *"at the end of its
turn"*, *"when you begin your turn."* **Lair actions** are the closest thing to
round-scoped and are not a boundary either: they are a turn slot at initiative
count 20, an entry in the order. **Legendary actions** reset at the start of
their turn. And a grep across every design doc in `rpg-project` and every rule in
the toolkit turns up exactly one round-scoped sentence, `economy.go:68`, which is
a statement about turn order rather than a trigger.

**So: in 5e the round is a coordinate, not a trigger.** It measures elapsed
time, and elapsed time is a number — which `TurnStartEvent` and `TurnEndEvent`
already carry as `Round int`. A subject-less round event adds nothing a field
does not already give.

This is [ADR-0007] doing its job. The counter-argument was written into this
document — *"do not extend a sealed set against hypotheticals"* — and then
leaned against anyway. The lesson is not "be more careful"; it is that **a
proposed vocabulary entry with no named subscriber is a hypothetical no matter
how reasonable it sounds.**

**The trigger for reopening**, recorded rather than left to memory: a mechanic
that fires at a boundary belonging to **no member** — a lair action modelled as a
true boundary rather than an initiative slot, or a homebrew "at the end of each
round." At that point the topic gets defined **together with its publisher and
its first subscriber**, in one PR, which is exactly what did not happen to
`TurnStartTopic`.

[ADR-0007]: https://github.com/KirkDiggler/rpg-toolkit/blob/main/docs/adr/0007-generic-trigger-system.md

### 3.2 `rulebooks/dnd5e/encounter` — stop dropping, start announcing

Encounter owns its own vocabulary (`MemberID`, not `core.EntityID`), so it
translates rather than re-exports:

```go
type BoundaryKind string
const (
    TurnStarted BoundaryKind = "turn_started"
    TurnEnded   BoundaryKind = "turn_ended"
)

type Boundary struct {
    Kind    BoundaryKind
    Subject MemberID
    Round   int
}
```

**Two kinds, because two are what get published** (§3.1). `clock`'s
`RoundStarted` milestone is translated to nothing — and loses no information,
because `Turn.End` increments the round *before* stamping the `TurnStarted` that
follows it (`turn.go:129-134`). **The round advancing is already visible as a
changed `Round` on the next turn boundary.** A separate kind would be a second
way to say the same thing.

Changes:

- `driveOneMonsterTurn` (`clocks.go:461`) stops returning `out.RoundWrapped`
  alone and carries `out.Milestones` translated to `[]Boundary`. **This is the
  driven monster's own turn ending** — the path §2 is about.
- `EndTurn` (`clocks.go:1266`) does the same for the caller's own turn end.
- `driveMonsterTurns` announces each advance **before** driving the member the
  clock landed on.
- `EndTurn` announces its own advance, then drives.
- `form` announces `RoundStarted(1)`, `TurnStarted(first, 1)` — which closes the
  gap where **the first turn of every fight begins with no turn-start at all**.
  `form` already consults `TurnDriver` synchronously; this is the same call site.
- `EndTurnOutput` gains `Boundaries []Boundary` — what was crossed, for a caller
  that wants the record. `RoundWrapped` stays: it is what the wire already
  reports and is derivable but load-bearing.

### 3.3 `rulebooks/dnd5e/events` — the vocabulary **[RULING Q3]**

**Q1 — a round topic — RULED 2026-08-27: no.** See §3.1. Nothing subscribes to a
round boundary and nothing in 5e fires on one; the round is a coordinate that
turn events already carry. `dnd5e/events` therefore gains **no new topic at
all** — this PR only renames, if Q3 says so.

**Q3 — `CharacterID` on turn events.** `TurnStartEvent`/`TurnEndEvent` name their
subject `CharacterID`. Monsters take turns; `driveMonsterTurns` ends several per
call. Publishing a monster's turn boundary into a field called `CharacterID` is
the same character-shaped defect that got `gamectx.CharacterRegistry` deleted
last slice — resolution's own doc: *"character-shaped, so it could not describe
a monster at all."*

*Leaning: rename to `SubjectID`*, matching `Milestone.Subject`. It is latent
today (no monster trait subscribes) and free now (`no-backcompat-baggage`:
rpg-api pins old versions, so renames cost nothing until adoption). It stops
being free the moment monster traits start listening.

### 3.4 `rulebooks/dnd5e/resolution` — the boundary machine

```go
type BoundaryInput struct {
    Crossed []BoundaryCrossing // resolution-owned data (R2)
}
func NewBoundary(in *BoundaryInput) (Machine, error)
```

The machine yields one bus step per crossing, in the order given, then `Done`.

**The step vocabulary does not grow.** A crossing is "do this on the bus and
hand me the next step," which is what `Gather` already does for the contest's
imposition. No new `Step` case, and nothing here should invent one.

Worth recording rather than leaving to memory: `doc.go`'s "bus-effect tally"
names its own reopening trigger — *"if wave 5's reactions multiply the
pure-effect steps ... the three-to-one ratio reverses and the argument reverses
with it."* A boundary machine multiplies exactly those steps. It does not force
a new case today; it does move the tally, and `doc.go` asked for that to be
noticed rather than discovered later.

### 3.5 `rulebooks/dnd5e/session` — the announcer seam

`announcerSeam` mirrors `strikerSeam` exactly: a struct holding a `*writeScope`
(not a closure over one, so it can be constructed before `scope.enc` exists),
reading the scope only at call time.

```go
func (a announcerSeam) Announce(ctx context.Context, enc *encounter.Encounter, crossed []encounter.Boundary) error {
    roster, err := enc.Members()          // NEVER adopts a new enc
    cast, err := a.m.castFor(ctx, a.scope, roster, nil)
    machine, err := resolution.NewBoundary(...)
    out, err := resolution.Resolve(ctx, &resolution.Input{
        World: enc.ToData(), Participants: cast, Machine: machine,
        Initiative: ..., Standing: ..., Sight: ..., TurnDriver: ...,
        Cost: nil,                        // a boundary has no payer
    })
    return a.m.saveDirty(ctx, a.scope, out)   // dirty sheets only
}
```

`castFor` (`attack.go:638`) is the existing helper and needs no change.
`Cost: nil` is a free action, the common case — noted only so nobody later reads
the nil as an omission.

## 4. What proves it

Not a test that publishes the event by hand. **That is the trap this slice
exists to escape** — `dodging_test.go:234`, `helped_test.go:153` and
`unconscious_test.go:522` all pass today by publishing what production never
does.

**Behavioural, through a real `session.EndTurn`, read out of the persisted sheet:**

| proof | why it can only pass if the whole chain works |
|---|---|
| a rogue's `used_this_turn` is `false` after their turn ends | the value can only return via `MarkDirty` → `DirtyCharacters` → `saveDirty` |
| a barbarian who neither attacked nor was hit **stops raging** | RAW rage-lapse; needs TurnEnd to arrive at all |
| a dodging fighter is no longer dodging after their next turn starts | Dodge is `experience/design.md`'s nominated proving case |
| a monster's turn-start is announced **before** that monster attacks | the ordering §2 exists for; assert on `Hooks`/step order, not on effect |

**Structural, in the shape `TestNoCodePathProducesACastlessInteraction`
established:** no code path advances a clock without announcing what it
crossed — read off the AST of `clocks.go`, because a behavioural suite
structurally cannot make that claim when the defect *is* that tests supply what
production does not.

## 5. Scope — and what is deliberately not here

**In:** turn start and turn end. The announcer capability. The fight-start
boundary (free — `form` is the same call site).

**Ruled out 2026-08-27:** round boundaries, and with them any change to
`play/clock`. See §3.1.

**Out, and why:**

- **Rest.** There is no rest verb anywhere — not in `session`, not in rpg-api.
  That is a verb to design, not an event to publish. But note the payoff: **no
  resource in the game has ever recovered** (`sheet_keeper.go:217` attaches every
  `RecoverableResource` *"so it hears rests"*), so this is the "Recover" third of
  journey **#241**, and it becomes reachable through the mechanism this slice
  builds.
- **Combat end.** `session.Dissolve` already exists with `ByDecision`/`ByDefeat`,
  so this is the nearest follow-on — genuinely just "announce it too." Left out
  only to keep one wave one shape.
- **Reactions / `Pose`.** Unbuilt, and a boundary interaction must not become the
  only thing that can publish mid-interaction.
- **Surprise.** `form` records it; a surprised creature loses its first turn.
  Turn-start announcement must eventually be able to say a turn started *and was
  skipped* — noted so the shape leaves room, not built.

## 6. Module chain

Bottom-up, one PR per module, merged in order:

```
rulebooks/dnd5e  →  encounter  →  resolution  →  session  →  rpg-api
   (rename only)     (Boundary,    (NewBoundary)  (announcer   (bump)
                      Announcer)                   seam)
```

**Five, not six.** `play/clock` drops out of the chain entirely with §3.1's
ruling — the layer that was going to change first now changes not at all. If Q3
is ruled against too, `rulebooks/dnd5e` drops out as well and the chain is four:
the whole slice is then `encounter` + `resolution` + `session` + the bump.
