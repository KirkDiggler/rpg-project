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

Four rungs, in order. A driver takes the first that applies.

| # | condition | intent | built today? |
|---|---|---|---|
| 1 | live sighting, in reach, attack left | `Attack{Target, Action}` | **yes** — `behavior.Basic` does exactly this |
| 2 | live sighting, not in reach, movement left | `Move{Path[:1]}` toward it | **yes** |
| 3 | **no live sighting, a ghost, movement left** | `Move` toward **the remembered cell** | **no** — this is the slice |
| 4 | nothing to act on | `Pass{}` | yes |

Rungs 1, 2 and 4 are `behavior.Basic` as it stands — 88 lines, and the package doc already calls
it *"a real, working driver... a foundation to extend, not a finished decision system."* **Rung 3
is the whole of this work at the behaviour layer.** Everything else here is what rung 3 needs
underneath it.

### Why the order is not negotiable

Rung 1 before 2 because an attack you can make is worth more than a better position. Rung 2
before 3 because **a live sighting always beats a memory** — a monster that walked toward a ghost
while its target stood in front of it would look broken, and that is the ordering bug this table
exists to prevent.

---

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
   the property that lets this ship incrementally, which is what Kirk asked for.

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

**With totality (`design.md` §3):** the observation is total over what it perceived, the stale
belief is corrected, the ghost is gone on the next tick, and the driver falls to rung 4 — or to
rung 3 on a *different* ghost. The loop closes on its own, with no code in the driver.

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
  thinking wanted actually earns its place.
- **Whether a ghost is worth a turn at all** versus holding a chokepoint.

---

## 7. What proves it

Not a unit test on the ladder — that only proves the ladder.

**Kirk gets behind a pillar and the monster comes looking.** Specifically: it holds live sight,
he breaks line of sight, its next turn moves it toward the cell he was last seen in, it arrives,
and it **stops believing he is there** rather than standing on the cell repeating itself.

The second half is the one that fails without §4, and it is invisible in any test that never
lets the monster arrive.

— platform agent, on behalf of KirkDiggler
