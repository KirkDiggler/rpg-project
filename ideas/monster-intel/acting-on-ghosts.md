# Acting on a ghost — the decision ladder a monster climbs

**Companion to** `design.md` (the world model). **Journey:** #201 · **Slice:** #305

This one is written **for whoever picks up monster behaviour**. `design.md` decides what a
monster knows; this decides what it does about it, and names precisely which parts are already
built, which are a contract change, and which are theirs to choose.

Kirk's spec, verbatim, because it is already the ladder:

> I can get behind a pillar and the monster forgets about me. the monster should attack if they
> see me and can, but should move if they cannot but have a ghost of me.

---

## 1. The ladder

**Revised by Kirk after the first draft**, and the revision makes it shorter:

> if we have current then that wins, if we do not but we have the ghost, we act on that. if the
> monster is to target closest then if the ghost was closest, it would pursue.

The first draft had four rungs with live sightings strictly above ghosts. That is wrong, and the
second sentence is why: **choosing what to move toward is a TARGETING decision, and the candidate
set includes ghosts.** A ladder that always preferred a live sighting would override the
rulebook's own strategy — which is the one thing this seam is careful not to do (`Targeting` is
opaque here by C1).

So: three rungs, and only the first is a fixed precedence.

| # | condition | intent | built? |
|---|---|---|---|
| 1 | a **live** sighting is in reach and an attack is left | `Attack{Target, Action}` | **yes** |
| 2 | movement left, and the targeting strategy picks a winner from **live sightings ∪ ghosts** | `Move` toward it | **partly** — the strategy exists, the ghosts do not |
| 3 | nothing to pursue | `Pass{}` | yes |

**Rung 1 is live-only by necessity, not by preference.** You cannot attack a memory, so no
strategy can rank a ghost into it. That is the whole of "current wins": when a target is in
reach and you can hit it, nothing else is under consideration.

**Rung 2 is where the strategy earns its name.** `MonsterView.Targeting` already carries the
rulebook's own word for this and is deliberately opaque to the seam. Under `closest`, a ghost two
cells behind the monster genuinely outranks a live target ten cells ahead — and Kirk's call is
that this is correct: it clears the near lead first.

Worth naming the case that will look wrong in play, so whoever tunes it recognises it rather than
treats it as a bug: **a ghost behind the monster and a visible enemy in front means it turns its
back on someone it can see.** That is not the ladder misbehaving, it is `closest` being a blunt
strategy. The fix, if it wants one, belongs in the strategy — `closest-live-then-closest-ghost`
is a legal strategy name and the seam would never know the difference. **Do not fix it by putting
live above remembered in the ladder**, because that quietly makes every future strategy a liar.

## 2. What `MonsterView` has to grow, and why it is a real contract change

Today `MonsterView.Seen` is `Status == intel.Current` **only**, and says so on purpose:

> A stale "held" memory (intel.Held, a ghost: known but not currently sustained) is not a target
> this member can act on THIS turn, so it never appears here.

So rung 3 has nothing to read. The driver cannot climb to it, because the view withholds the
rung.

**Proposal: a second, separate collection.**

```go
type MonsterView struct {
    // ...
    Seen        []SeenMember   // Status == intel.Current — unchanged
    Remembered  []GhostMember  // Status == intel.Held
}
```

**A separate field rather than a `Status` flag on `SeenMember`.** Three reasons, and the third is
the one that matters:

1. Every existing driver keeps working unchanged, and keeps its current meaning — `Seen` still
   means "act on this now". A flag would silently change what `Seen` means for code already
   written against it.
2. The two carry genuinely different facts (§3), so one struct would be half-empty either way.
3. **A driver that ignores `Remembered` entirely is still correct** — just less clever. That is
   the property that lets this ship incrementally, which is what Kirk asked for. It is also
   exactly `behavior.Basic` today: it ranks `Seen` by distance and would simply be handed a
   larger candidate set.

---

## 3. A ghost's geometry is not a sighting's geometry

This is the part most likely to be got wrong by analogy, so it is stated rather than implied.

`SeenMember.Path` has a precise contract:

> ENDS ON THE NEAREST CELL... FROM WHICH THIS SIGHTING IS WITHIN REACH of this member's own
> longest-reaching action: NEVER the sighting's own occupied cell.

That is exactly right for a target you intend to hit. **It is wrong for a ghost.** Nothing is
standing on the remembered cell — that is the entire point — so "stop where you could reach it"
stops the monster short of the place it wanted to look at, for no reason.

```go
type GhostMember struct {
    ID       MemberID
    Kind     MemberKind
    Position spatial.Position  // the remembered cell — where they WERE
    At       uint64            // the clock reading when it was last refreshed
    DistanceCells float64
    Path     []spatial.Position // ENDS ON Position — you are going to look at it
}
```

**No `InReach`.** A ghost cannot be attacked, and offering the map would invite exactly that.
Rung 1 reads `Seen`; there is nothing for `Remembered` to answer about reach.

**`At` is the one genuinely new fact**, and it is what makes a driver more than reflexive: it is
how a monster tells *"he was there a moment ago"* from *"he was there a minute ago."* A
collaborator can ignore it and be correct; a good driver will use it to decide whether a lead is
worth walking across the room for.

---

## 4. The arrival case, and why `design.md` has to land first

The monster walks to the remembered cell. It looks. Nothing is there.

### Arriving is surveilling, and that is already true

Kirk: *"Only 1 entry per subject means I would update the ghost to nothing. that would initiate a
new surveil and possibly find a new target. nothing come back from surveil then the decider would
need to handle it."*

**All three of those already hold, and the middle one is free.** `refreshSight` runs on every
step (`step.go:126`), so a monster walking to a remembered cell is surveilling the whole way —
it does not arrive and then look, it looks continuously and the arrival is just the last one. Two
consequences:

- The correction may land **before** it arrives. Walk far enough to see the remembered cell and
  the belief updates from there; the monster never has to stand on the spot.
- New subjects land on the same pass. The walk toward a stale lead is also how it finds the next
  one — Kirk's *"possibly find a new target"* needs no code.

And *"nothing comes back from surveil"* is rung 3: no live sighting, no ghost worth pursuing,
`Pass` — which is precisely the decider's problem and not the model's. §6 keeps it there.

**With the ruled model (`design.md` §5a):** the observation is total over what it perceived, so
the *place* belief is corrected — and the *subject* belief survives with its position downgraded
to "not here / unknown". The monster still knows there is someone to hunt; it just no longer
knows where. It falls to rung 4, or to rung 3 on a *different* ghost. The loop closes on its own,
with no code in the driver.

**That the subject survives is the whole difference between a search and a shrug.** A model that
deleted the belief along with the position would leave a monster that has forgotten it was
chasing anybody — which is not what a player means when they say they lost it.

**Without totality:** the fade pass never touched the payload, the ghost still says (19,3), and
the monster stands on the empty cell **still believing you are on it**. It re-issues rung 3
against a cell it is already standing on, forever.

**So rung 3 is not safe to build before `design.md` question 1 is ruled.** A driver written
against today's intel produces the pillar-camping bug, and it will look like a behaviour bug in
the collaborator's code when it is a world-model bug underneath them. That is the single most
useful thing in this document.

---

## 5. Introducing it slowly

Kirk: *"we are introducing this slowly."* Three steps, each shippable and each with its own
proof:

**Step 1 — the world model.** `design.md`'s question 1. Nothing in `behavior` changes. Proof: a
monster that looks at an empty remembered cell no longer believes anything is on it.

**Step 2 — the view.** `MonsterView.Remembered`, populated from `Status == intel.Held`. Still
nothing in `behavior` changes, and **`Basic` keeps passing its tests untouched** — which is the
check that the field is genuinely additive. Proof: a driver that asks for ghosts gets them; one
that does not is unaffected.

**Step 3 — the rung.** A driver that climbs to 3. Whether that is `Basic` growing a rung or a
new `Tracker` beside it is the collaborator's call — the package doc already anticipates
*"a driver for one of those cases is another type in this package, not a different seam."*

Steps 1 and 2 are ours. Step 3 is theirs, and it is one function.

---

## 6. What the collaborator inherits, and what is theirs

**Inherited, do not re-litigate:**

- A monster's decision is knowledge-only by contract (C2). A driver never sees live truth about
  anyone else, and must not be given a way to.
- Perception is data. `MonsterView` is plain data end to end; a driver never touches a live
  `*Encounter`.
- The geometry is precomputed — distance, reach, and pathing all arrive on the view, in the
  units the driver needs, so no driver converts feet to cells or asks the encounter anything.
- Live beats remembered (§1).

**Theirs to decide:**

- **Staleness policy.** Is a ghost from twenty rounds ago still worth walking to? `At` is
  provided; no threshold is imposed, because the right one is a feel question.
- **Choosing between ghosts.** Nearest, freshest, or the one matching a `Targeting` strategy.
  `MonsterView.Targeting` already carries the rulebook's own word for this and is deliberately
  opaque to the seam (C1).
- **Giving up.** After arriving and finding nothing, does the monster search, return to a post,
  or wander? All three are rung 4 today. This is where the mode machine the original monster-AI
  thinking wanted actually earns its place — and note that under §5a it arrives at rung 4 still
  **holding the subject**, so "search for someone I know is here" is expressible without any new
  state.
- **Whether a ghost is worth a turn at all** versus holding a chokepoint.

---

## 7. Worked example: Kirk ducks through the second door

Kirk's own dungeon has a double door and a second way out, and he asked what a monster chasing
his ghost does when he ducks into another room. It is the right question, because it is the first
case where the ladder alone is not enough.

### The mechanics that decide it, verified rather than assumed

A door registers **one flag on both**: `blocks := d.state.blocks()` is applied to
`BlocksMovement` *and* `BlocksLineOfSight` on every edge it owns. So:

- **An open door does not block sight.** A monster in room A can see through an open doorway into
  room B, along whatever sightline the geometry allows.
- **A closed door blocks both**, at the boundary.
- `SeenMember.Path` and `GhostMember.Path` both route *"against this composition's own walls,
  doors and floor — the same geometry `Encounter.Step` enforces"*, so a path never routes through
  a closed door and a monster never walks a route it could not walk.

### The walkthrough

1. **Monster holds live sight of Kirk in room A.** Rung 1 or 2.
2. **Kirk steps through the doorway into room B, door left open.** The monster may *still see him*
   — an open door occludes nothing. **No ghost yet, and the ladder never leaves rung 2.** This is
   worth stating because it is the case people expect to produce a ghost and it does not.
3. **Kirk moves out of the sightline** — around the corner in room B, or the door swings shut.
   *Now* sight lapses, `CurrentVia` empties, and the holding becomes a ghost.
4. **Rung 3: the monster walks to the remembered cell.**
5. **It arrives and looks.**

### What step 5 tells it, and this is the payoff of totality

Under `design.md`'s model an observation is total over **what the monster perceived**, not over
the one cell it walked to. Standing in the room, it perceives the room — so what it learns is not
*"he is not on that tile"* but **"he is not in this room."**

That is a categorically better fact, and it is what turns the next decision from flailing into
searching: the monster is now standing in a room it knows is empty, with two exits it can see,
and no live sighting. Rung 4 today. **This is exactly where the "giving up" decision from §6
stops being abstract** — the collaborator's search behaviour has real material to work with,
because the world model handed it a negative fact instead of a stale positive one.

### The part that makes a ghost worth more or less

**A ghost's position is where he was last SEEN, not where he went.** In Kirk's case that
distinction is everything:

| how sight was lost | ghost sits at | what it tells the monster |
|---|---|---|
| watched him step into the doorway, then lost him | the doorway | **which door he took** — genuinely informative |
| lost him behind a pillar first, *then* he used a door | the pillar | nothing about either door |

So the same mechanic produces a smart-looking monster or a stupid-looking one depending on
geometry the player controls — which is the good kind of emergent, and it is the argument for
`GhostMember.At`: a ghost that is one tick old and sitting in a doorway is a strong lead; a ghost
that is ten ticks old and sitting in the middle of a room is barely one.

### The concrete instance, from Kirk's own dungeon

Kirk played it and described the position: last seen on the **red carpet** in the room above,
then through a doorway in the dividing wall, then through a **second** door into the room on the
other side of an interior wall. The monster is left in the first room below; he is in the second.

His expectation: *"I would expect that monster to go look where they last saw me on the red
carpet."*

**That is rung 3 exactly, and the thing that makes it a good test is that the carpet is the wrong
way.** The monster walks back up through the doorway, away from where he actually is, because
that is the only place it has any evidence about. He gets away — not because the monster is
stupid, but because it is acting correctly on stale information that he made stale on purpose.
A model where the monster drifted toward him instead would be a model that leaked his position.

Two things this instance surfaces that the abstract version did not:

**The ladder is re-evaluated every turn, so the walk is not a commitment.** If the route back to
the carpet ever puts him in the monster's sightline — through the open doorway, say — rung 2
beats rung 3 on that turn and it turns and comes for him. That is `live beats remembered` (§1)
doing real work rather than being a tidy-sounding rule.

**Where the ghost sits depends on when sight broke, and he can feel the difference.** He says he
was last seen *on the carpet*, so the ghost is on the carpet and the monster searches the wrong
room entirely. Had the monster instead watched him step into the doorway, the ghost would sit in
the doorway — and it would arrive to find an empty threshold with a clear view of where he went
next. Same mechanic, two very different outcomes, decided by whether he broke sight before or
after committing to the door.

### What this does NOT justify

**The monster must not be told which door Kirk used** unless it saw him use it. That is the C2
anti-wall-hack contract, and the temptation here is real — "the monster knows its own dungeon, so
it knows where the doors go" is true and irrelevant. Static topology at construction time is
fine (`Snapshot`'s doc says so explicitly). *Kirk's* position, or a trail he left, is not.

A monster that guesses the wrong door and searches the wrong room is **correct behaviour**, and
is the whole reason ducking through a second door is a tactic worth having.

## 8. The discriminating test — Kirk's double door, as a fixture

Journey #201's **Done when** is *"toolkit tests discriminate the chosen behavior from the
fallback policy, and a monster is observed making that intentional decision through the local dev
dungeon."* Kirk's ask — *"a test with my double door structure playing out like we see it"* — is
that sentence made concrete. This is the fixture, written before the code so it is the target
rather than a description of whatever gets built.

### Geometry

Three regions, matching the dungeon Kirk played:

```
        ┌─────────────────┐
        │   CARPET room   │   <- he is seen here first
        └────────┬────────┘
                 │  door A  (in the dividing wall)
        ┌────────┴────────┐
        │  LEFT   │ RIGHT │   <- interior wall between them
        │  room   │ room  │      door B connects them
        └─────────┴───────┘
```

- **Door A** joins CARPET to LEFT.
- **Door B** joins LEFT to RIGHT — the "second door".
- The interior wall blocks sight between LEFT and RIGHT (it must, or there is no test).

### Sequence

| # | act | required state after |
|---|---|---|
| 1 | monster in LEFT, hero in CARPET, door A open, line of sight along it | monster holds hero `Current`, position = a CARPET cell |
| 2 | hero steps LEFT, then through door B into RIGHT | sight breaks at the interior wall |
| 3 | — | monster holds hero **`Held`** — a ghost, position still the **CARPET** cell |
| 4 | monster's turn | it declares `Move` toward the CARPET cell — **away from the hero** |
| 5 | monster steps until it perceives the carpet | belief updates: hero still known, **position unknown** |
| 6 | monster's next turn | no `Current`, no position — it does **not** re-declare a move to the carpet |

### What makes it discriminating

**`behavior.Basic` fails at step 4 by passing.** It ranks `Seen` only, `Seen` is empty once sight
breaks, so it returns `Pass{}` — the monster stands in LEFT doing nothing while the hero walks
away. A ghost-aware driver moves. **That is the discrimination the journey asks for, in one step
of one test.**

**Step 6 is the one that discriminates the WORLD MODEL rather than the driver**, and it is the
reason this test is worth more than a unit test on the ladder. Without absence-as-testimony
(`design.md` §3), step 5 leaves the ghost intact — so at step 6 the driver correctly re-declares
a move to a cell it is already standing on, and does so forever. **A test that stops at step 4
passes with the pillar-camping bug fully present.**

### What it must NOT assert

- **Not** that the monster then finds the hero. It should not: he is behind a wall it cannot see
  through, and a monster that drifted toward him would be leaking his position (§7).
- **Not** which door it tries next, or that it tries one at all. That is the collaborator's
  search behaviour (§6) and this test predates it.
- **Not** a fixed number of turns. Distance is fixture geometry, and pinning turn counts makes
  the test about the map instead of the behaviour.

### The deception variant, once the above is green

The same fixture, with step 1 replaced: **no sighting ever happens.** Instead a `Report` on a
sound channel lands a belief on the monster naming a CARPET cell the hero has never been to.

Steps 4 through 6 must play out **identically** — walk to it, find nothing, downgrade to unknown.
If they do, the thesis holds in code: a lie and a stale ghost are the same object, and one
correction path serves both. If the two need different handling, something upstream has assumed
intel is true.

### The local observation

The journey wants both halves. The local one is Kirk's, and he has already described it: he ducks
through the second door and the monster goes and searches the carpet room. The test above is what
makes that reproducible rather than anecdotal, and the test is the thing that can fail in CI.

## 9. What proves it

Not a unit test on the ladder — that only proves the ladder.

**Kirk gets behind a pillar and the monster comes looking.** Specifically: it holds live sight,
he breaks line of sight, its next turn moves it toward the cell he was last seen in, it arrives,
and it **stops believing he is there** rather than standing on the cell repeating itself.

The second half is the one that fails without §4, and it is invisible in any test that never
lets the monster arrive.

**Then the double-door case (§7/§8), in his own dungeon**, because it is the one that proves the
model rather than the ladder: he ducks through the second door, the monster follows the ghost,
arrives, and comes away knowing **the room is empty** — not standing on a tile still believing he
is on it. Whether it then picks the right door is not the test; that it has a negative fact to
search from, is.

— platform agent, on behalf of KirkDiggler
