# Holdings — each member's own version of the world

**Status:** design, ruled in conversation 2026-09-12. Not yet built.
**Tracking:** this PR stays open until the rungs below have landed.
**Neighbours:** [`perceive/`](../perceive/design.md) (the #940 audience shelf) ·
[`monster-intel/`](../monster-intel/) (acting on a ghost) ·
[`perception-stream/`](../perception-stream/design.md) (delivery) · `fog-of-war/`

---

## 1. The ruling

> *"we have a global who is up and who is down and we are referencing that to
> maintain what I see. i think that's the break. if we are using that to build
> what I see then I should have my own who I know who is up and who is down.
> the global table is fine, but my player would never actually know the whole
> table and it should not feed into my intel. if i dont know someone is down, i
> dont know it."*
>
> — Kirk, 2026-09-12

And the constructive half, from the same conversation:

> *"if I have a holding on goblin-1 I do not get any info from the world ledger
> on that."*
>
> *"the things I know or think I know will never come from the ledger."*

That is the whole design. Everything below is what it costs to make true.

### What is deliberately NOT a law

Kirk, same session:

> *"for the record I say a lot of things, sometimes I even think they may be
> laws. I never mean for them to be interpreted that way and they often are. we
> have 'laws' that come back and bite us all the time."*

Two specific non-laws, recorded so nobody codifies them later:

- **"If a holding is current, it should match the ledger"** is true today and
  must NOT become a test. A charmed observer's *current* holding has to be
  allowed to disagree with the world, or we re-foreclose illusion with an
  assertion. The honest invariant is *a current holding equals what this pass
  wrote*, which is identical in every case we have and still leaves the door
  open.
- **Turn order is global**, and that is a decision for now, not a principle.
  The door left open — *you know a slot exists but not who is in it* — is
  recorded as over-the-top at this stage. The table analogy Kirk gave: everyone
  hears the DM and is told when their character didn't.

### A worked example of the cost

`encounter.go:1984` carries a comment saying `SightTestimony.Down` cannot be
written because the pass has no participation reading to draw on and
composition law C8 permits only one ask per pass. That was true when written.
`rebuildPercepts` has asked `sightNow()` and `equipmentNow()` once before its
loop since equipment landed — the C8-compliant pattern arrived and nobody
re-read the comment. The claim was then copied, in spirit, into
`session/convert.go`'s `projectSeen` doc.

One remark became a law, the law became a doc comment, the comment reached a
second repo, and it has been telling both repos the fix is impossible ever
since. **rpg-toolkit#1668 has had no blocker for a whole wave.**

Treat a blocking comment as evidence to re-check, never as a settled answer.

---

## 2. Vocabulary

| word | meaning |
|---|---|
| **ledger** | the world's truth. One row per member. Never read by anything addressed to one member. |
| **holdings** | what one member knows or believes. `play/intel`'s existing `map[observer]map[subject]`. |
| **`Actual`** | one row of the ledger — what is true of a member this pass. |
| **`Observed`** | one row of a member's holdings — what an observer perceived of a subject. Same columns. |
| **ghost** | a holding with `CurrentVia` empty: held, sustained by no channel. A memory. |

`ledger` and `holdings` are Kirk's words and supersede earlier drafts that used
"table", "sheet" (collides with the character sheet — the one thing you are
never wrong about) and "view"/"testimony" (both already taken in code).

---

## 3. What already exists

We have built this machinery five times in five places and never named the
thing it was machinery for.

| what | where | direction |
|---|---|---|
| `play/intel` — per-observer, payload-opaque, ghosts, `Reacquired` | `play/intel` | ✅ the core |
| `SightTestimony` — the row, for sight | `encounter/testimony.go` | ✅ prototype of `Observed` |
| capabilities supplied, never defaulted (rpg-toolkit#1033) | `Sight`, `Equipment`, `Participation` | ✅ the asking side is already right |
| monsters decide from holdings, never the world | `decider.go`, `intel/doc.go` | ✅ "declare from holdings" proven |
| interaction requires a **current** holding | `interact.go:167` | ✅ same rule, one verb |
| negative evidence, in miniature | `testimony.go:273` `correctArrivedLocations` | ✅ the verb exists |
| the `sighted` beat | `encounter/sightedbeat.go` | ✅ the delivery half |
| `audienceFor` / `beatClass` | `encounter.go:1196` | ⏸ built, returns full roster — the shelf for #940 |
| `projection.go`, member-scoped `GetDoors`/`GetAtlas` | structure | ❌ same need, solved **read-time** |

The last row is the tell. The need for per-member knowledge was strong enough
to build a second mechanism, and it got built the other way round — filtering
truth at read time — because there was no named thing to build it into. A
read-time filter can only ever be right, which forecloses the illusory wall
exactly as reading position live foreclosed the illusory ogre.

The same is true one layer up: `perceive`, `monster-intel`, `perception-stream`
and `fog-of-war` are four folders that are all facets of this one primitive,
written before it had a name.

---

## 4. The defect, which is one defect

| property | what's wrong | issues |
|---|---|---|
| **incomplete** | the row has no standing, no name, no kind | #1668 + two unfiled |
| **not exclusive** | readers reach past the row | #1668, #940, `projection.go` |
| **batched** | one write per walk, not one per observation | #1670 |
| **only positive** | looking somewhere and seeing nothing is discarded | unfiled |

### 4.1 Incomplete

`SightTestimony.Down` exists and is never written (§1). Worse, two siblings of
standing were never even filed:

```go
projectSightings(in []intel.Holding, names map[string]string,
                 kinds map[string]MemberKind, down map[string]bool)
```

Three global maps. `down[subject]` is #1668. `names[subject]` and
`kinds[subject]` are the identical defect.

Equipment already moved into the row for exactly this reason and name/kind did
not, though they are the same kind of fact:

- **Name in the row is what makes disguise possible.** A doppelgänger's whole
  point is that what you saw was a lie. Name from a global map forecloses it.
- **Kind in the row is the fidelity case.** A shape at 100 feet in dim light —
  do you know it is a hobgoblin *specifically*? Global says yes, from the first
  glimpse.

It is invisible today because a goblin is always named "Goblin", so the join is
*correct*. The defect appears the moment something can lie.

### 4.2 Not exclusive

`session/read.go` computes `standingSet(...)` over the whole roster and stamps
it onto every one of a member's sightings, live or memory. Reading the ledger
was fine. Reading it **for one member** is the defect.

### 4.3 Batched

`walkPath` appends a `moved` beat per cell; `settleWalk` runs **one**
`refreshSight` per intent. So a fleeing monster's four steps reach observers as
four beats and one testimony write — which, landing after it was occluded,
records `lost` while the last testified position is still the START cell. That
is rpg-toolkit#1670. Players are unaffected only because `session/move.go`
steps them one `Step` at a time.

The precedent for the fix is in the same loop: `standingNow()` is already asked
per cell, with the comment *"The batched once-per-walk answer elsewhere in this
rulebook rests on a move being unable to down anyone; announcing steps is what
made that false."*

### 4.4 Only positive

`correctArrivedLocations` does exactly the right thing — finds held sight
holdings and `Report`s them back as `SightTestimony{State: LocationUnknown}` —
but its own doc says it is *"deliberately narrower than a general sight
correction."* It fires only when **all three** hold: after a **driven**
fight-time move, for the **mover** as observer, for ghosts remembered at
**exactly the cell the mover landed on**.

So: *"I got shoved into a square where I thought you were, and you're not
here."* Walking in normally does not do it. Looking across the room at the cell
you remember does not do it.

Without the general rule, ghosts accumulate over a dungeon into memories the
observer has no way to correct. **Looking somewhere and seeing nothing is
information**, and we throw it away.

---

## 5. The shape

### 5.1 Why three maps become one row

Each map is one supplied capability's answer:

| capability | question | answer |
|---|---|---|
| `Sight` | how far can each member see? | `map[MemberID]int` |
| `Equipment` | what is each member holding? | `map[MemberID]*HeldEquipment` |
| `Participation` → `Standing` | which members are down? | `map[MemberID]bool` |

**Asking separately is correct and stays.** Different owners, different
questions, each asked once per pass (C8). Collapsing them into one
"tell-me-everything" capability would put the encounter back in the business of
knowing what a member *is*.

What is wrong is that the answers are **never composed**. They travel as N
parallel maps to the point of use and every consumer re-does the join by hand.
Four costs:

1. **Nothing ties them to one instant.** `projectSightings` takes three maps as
   three arguments; a stale one type-checks.
2. **Every new column is a new parameter at every seam** — the signature grows
   linearly with the model, at each of three layers.
3. **No value anywhere means "what is true of goblin-1."**
4. **`map[MemberID]bool` cannot say "not observed."** The bool has no name, so
   the map carries the meaning — which is why it reads as a verb. And the third
   state has to be invented at the conversion into `*bool`, in a place that
   does not know it. A noun carries all three from the start.

### 5.2 The types

```go
// Actual — what is true of one member this pass. The ledger's row.
// Composed from the capability answers immediately after asking them.
type Actual struct {
    Position  spatial.Position
    Standing  Standing        // Up | Downed — a noun, not a bare bool
    Equipment *HeldEquipment
    Name      string
    Kind      MemberKind
}

// Observed — what one observer perceived of one subject. Same columns.
// Every field a pointer, because "not observed" is a real and different
// answer from "observed to be the zero value".
type Observed struct {
    State     LocationState   // known | unknown
    Position  *spatial.Position
    Standing  *Standing
    Equipment *HeldEquipment
    Name      *string
    Kind      *MemberKind
}
```

The pass becomes one sentence: **for each observer, for each member, copy the
columns this observer can perceive.** *"Project the global table for each
member"* stops being an analogy and becomes the implementation.

### 5.3 They live in separate containers, deliberately

`Actual` is **one per member**. `Observed` is **one per (observer, subject)
pair** — Alice's view of goblin-1 and Bob's are different rows.

A single `map[MemberID]{Actual, Observed}` has nowhere to put the second key.
A nested form does close — `Holdings[M] = {Actual: truth about M, Observed:
map[subject]Observed}` — but we are still splitting them, for the reason that
is the point of the exercise: **if truth and belief share a struct, reaching
for truth from a per-observer path is a field access.** In separate containers
it is a visible lookup someone has to justify. The bug we are fixing was easy
to write and invisible to read; the new shape should make it awkward.

So: `map[MemberID]*Actual` for the ledger, and `Observed` stays exactly where
`intel` already puts it. **`play/intel` does not change.** It stays a leaf,
payload-opaque; `Actual` and `Observed` are encounter-owned, as
`SightTestimony` already is.

---

## 6. When the ledger IS read

Three things, and "what I see" is none of them.

1. **To write the holdings.** The refresh pass reads the ledger to decide what
   each observer perceives and what to snapshot. The ledger is the **write
   source**, not a read fallback — which is why "only source" vs "override"
   was never a real fork.
2. **To resolve.** *Declare from your holdings, resolve against the ledger.*
   You may only target what you hold; what your swing actually hits is the
   world's business. Already true in `decider.go` and `interact.go:167`.
3. **To be you.** Your own hit points, cell and sheet. `rebuildPercepts` skips
   self outright, so you never hold intel about yourself. Not a leak —
   ownership. Your sheet was never perception.

Plus the readers that are not players: the freeroam workbench, persistence,
adjudication. Truth by definition.

### The test

> The ledger may be read by the **engine**. Never by anything addressed to
> **one member**.

Checkable at the call site by asking who the answer is for. It catches the bug
we have, and — unlike "current holdings must match" — nothing about it forbids
a pass that writes a lie.

---

## 7. How it scales

| what grows | today | folded |
|---|---|---|
| **new column** (facing, conditions, HP band) | capability + map + parameter at 3 seams + testimony field + allowlist key + proto field | capability + field on `Actual` + one line in the copy |
| **new channel** (hearing, tremorsense) | nowhere to put it — nothing answers *"what does this channel yield"* | the copy gains a per-channel answer per column; the hearing ladder becomes a **table**, not a rewrite |
| **new fidelity rule** (dim light, stealth #1020, disguise) | scattered | all are *"this column doesn't copy, or copies a lie"* — one place |
| **new subject kind** (doors, props) | a whole second mechanism | different columns, same copy step |

**The copy step becomes the only place perception lives**, and every direction
we grow, grows there.

### Costs, named

- One `Actual` per member per pass — and rung 4 wants per-cell passes, so
  ×cells. Measurable; `encounter/voidcost_internal_test.go` is the home.
- `sightPayloadFields` is a **closed allowlist** that refuses unknown keys, so
  every column is a deliberate coordinated change. That is right for testimony
  — a key we do not understand must not land in something that then reads as
  confident — but columns are never free, which argues for settling the set
  now rather than discovering it four at a time.
- `Actual` must never reach a player-facing seam (§6).

---

## 8. Rungs

Each is shippable on its own.

| # | rung | closes |
|---|---|---|
| 1 | **Fold** — capability answers become `map[MemberID]*Actual`; `standingNow()` joins the pass. Pure refactor, no behaviour change. | unblocks #1668 |
| 2 | **The row** — `Observed` carries standing, name, kind. | #1668 + the two unfiled siblings |
| 3 | **Exclusivity** — session reads holdings only; the three global joins in `projectSightings` are deleted. | makes the rule enforced rather than intended |
| 4 | **Per observation** — testimony per cell; negative evidence generalized out of `correctArrivedLocations`. | #1670, both halves |
| 5 | **Channel projection** — a beat degrades by the channel that carried it. | #940; makes the hearing ladder buildable |
| 6 | **Structure** — doors and props get `Actual`/`Observed`. | retires `projection.go` |

Rung 5 is worth stating precisely, because it changes what #940 *is*: the
question stops being *"do I get the beat"* and becomes *"what does this beat
look like through the channel that carried it to me."*

| the ledger's beat | by sight | by hearing |
|---|---|---|
| `struck` goblin→Alice, 6, cell (12,7) | "The goblin hits Alice for 6" | "Steel on steel, and a woman cries out" |
| `downed` goblin | "The goblin drops" | "Something heavy hits the floor" |
| `moved` goblin | the cell | nothing, or "footsteps, closer" |

And a product constraint that is also a design argument: forty lines of *"you
hear a blow land"* is worse than silence. Hearing should write a **row** —
subject `"beyond the north door"`, payload *"fighting, several voices, one you
don't know"*, updated as it goes — not a stream. If that holds, **the log is a
rendering of holdings changes** and there is no second mechanism anywhere.

`intel` is already shaped for it: `Subject` is documented as *"opaque,
caller-chosen identification within an observer's fidelity"*, so a hearing
contact is legitimately keyed `"north-door"` rather than a monster ID.

---

## 9. Doors left open, not scheduled

- **Subjects never merge.** Hearing through the north door is subject
  `north-door`; a goblin you have seen is subject `goblin-1`; a second door is
  a third row. Holdings honestly record separate things you know. The inference
  that they are one creature is the **player's**, not the engine's — the same
  place Kirk put *"voices I know aren't my party"* and *"if I speak goblin"*.
  This removes a reconciliation problem an earlier draft had invented.
- **Speech as a channel.** *"Three goblins, one's down!"* is ordinary testimony
  written into your holdings — and can be wrong, or a lie. Falls out of the
  model for free. Not scheduled.
- **A motion column.** *"Was it fleeing or did it stop?"* — a ghost today is a
  position, so *"stopped in that doorway"* and *"ran through it and I lost
  them"* are the same row. Real, and deliberately not built: rung 4 gets the
  ghost onto the right cell, which is most of the value, and *"what they were
  doing"* generalizes to casting, drinking and readying — a much bigger column
  than it looks.
- **Turn-order slots without occupants** (§1).
- **Swinging at a ghost.** Under *declare from holdings, resolve against the
  ledger*, you may legally declare an attack on a memory and be told you hit
  air. Not work — a consequence, and the sharpest test that the split is real.

## 10. What does not change

- `play/intel` — no API change. Leaf, payload-opaque, no `context.Context`.
- The capabilities stay separate and supplied. Ask separately, fold locally.
- Composition law C8 — one ask per capability per pass — is respected by every
  rung. Rung 1 adds a third ask *beside* the existing two, which is the
  established pattern, not an exception to it.
