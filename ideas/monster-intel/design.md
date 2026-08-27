# What a monster knows — reconciling two knowledge models before behaviour is built on one

**Journey:** #201 (Monster Behavior in the Local Dungeon) · **Prompted by:** Kirk, while
ruling Hide's observers on rpg-project#300

Kirk's framing, which is the whole design in three sentences:

> we know if something has visual Intel on me right? those that don't I would be invisible. I
> could turn a corner get out of view and go stealth. we will be adding the ghost Intel to a
> monster decision like they would check last known position but if I am stealth they got
> nothing on me

and then the part that turned out to be load-bearing:

> when the ghost is checked and not there the subject would get updated right? ... if we go open
> the door and we see pots and pans on the ground we would update that banging to be pots and
> pans on the floor

---

---

## The thesis (Kirk, and it is why this document exists)

> **Intel can be lied to is the game.** deception steering the monsters wrong is what it's about.
> the monster will be distracted but eventually it can correct by checking their Intel and
> updating as they go.

Everything below is in service of that sentence, and it is worth reading the rest with it in
hand, because it changes what several sections are *for*.

**A lie and a stale ghost are the same object.** From the monster's side there is no difference
between "I saw him there and he left" and "something made me believe he is there" — both are a
belief with a position, held sincerely, and neither is true any more. The monster cannot tell
them apart and **does not need to**, because the same act resolves both: go and look.

That means the ghost-correction loop this document spends its length on is not hygiene. **It is
the mechanism that makes deception playable**, and it is playable precisely because it is
imperfect in both directions:

- A lie that **never** expires is not a tactic, it is a broken monster. If a false belief could
  not be corrected, one thrown rock disables a guard permanently and the dungeon stops being a
  place.
- A lie that is **instantly** seen through is not a tactic either. If the monster reconciled
  against world truth it would never be fooled at all, and deception would have nothing to act
  on.

The correction loop sits between those, and gives a lie a **duration nobody has to tune**: it
buys exactly as long as it takes the monster to walk over and check. Distance is the cost. A
decoy thrown across a large room buys more than one dropped at your feet, because the monster
has further to walk — and that falls out of geometry the player controls, the same way §7's
"where the ghost sits depends on when sight broke" does.

### What this settles

- **The deception-overwrites-sight question** (§5a, "the one place that gets interesting") is
  answered in the affirmative by the thesis. If deception steering monsters wrong is the point,
  a lie must be able to land on a monster that is looking at you. The counter is not resistance,
  it is *checking* — which the model already provides.
- **"Nothing may reconcile intel against world truth"** stops being a purity rule and becomes the
  load-bearing one. Any reconciliation pass, however well meant, deletes the game.
- **A false belief must be correctable only by later testimony**, exactly like a true one. That
  is not consistency for its own sake — it is what makes being fooled fair, because the player
  and the monster are subject to the same physics.

### And it is nearly free

`Report` already *"lands discrete testimony as HELD"* on an open `Channel` vocabulary, and the
module already *"treats all identically"*. **A thrown rock is a `Report` on a sound channel to
whoever could hear it, carrying a position that is not yours.** No new verb, no new state, no
special case in any decider — it arrives as an ordinary belief and gets acted on and corrected by
ordinary rungs. The design's job is to not break that, not to build it.

---

> **Read §5a first.** Sections 2 and 3 argue toward a hex-total model and pose a choice between
> it and subject-keyed intel. Kirk ruled that choice a false one — the answer is both layers,
> answering different questions, and intel is a **belief store** rather than a projection of
> truth. The argument is kept because the reasoning still holds for the *place* layer and
> explains why the ghost-correction problem is real; the conclusion moved.

## 0. Why now, and why this is not a Hide ticket

Hide's Stealth check is rolled against an empty observer list today, so it cannot fail
(rpg-project#300, pinned by `TestAcceptance_HideLandsButItsCheckHasNothingToBeat`). Wiring that
list is a two-line change. **It is also the wrong place to start**, because the list is a
question about what a creature knows, and we have two different answers to that question living
in the repo right now.

Monster behaviour is next. Whatever we do not decide here, it will inherit.

---

## 1. From the monster's decision back

The seam is already better than it needs to be, and that is worth saying before criticising it.
A monster's decision is **already knowledge-only, by contract**:

> `Snapshot` is a decider's complete input: where it stands, plus its OWN held intel. The
> anti-wall-hack contract (C2) extends to placement, not just holdings — a decider learns where
> IT stands, never where any other member stands except through Holdings' sighted percepts.

There are two decision seams, and they take different shapes of knowledge:

| seam | clock | input | knowledge |
|---|---|---|---|
| `encounter.Decider` | world (free roam) | `Snapshot{Position, Holdings []intel.Holding}` | **all** holdings, ghosts included |
| `encounter.TurnDriver` | turn (in a fight) | `MonsterView{Seen []SeenMember, Budget, Actions}` | `Status == intel.Current` **only** |

`MonsterView.Seen`'s own doc is explicit about the second:

> A stale "held" memory (intel.Held, a ghost: known but not currently sustained) is not a target
> this member can act on THIS turn, so it never appears here; a driver that wants to chase a
> last-known position needs no special case for that, because Seen simply will not contain it
> once the sighting lapses.

**So "chase the last-known position" is not a feature we need to add to the model — it is a
feature the turn seam deliberately withholds.** The ghost exists; the fight-time driver is not
shown it. That is a defensible v1 (a monster in a fight acts on what it can see) and it is
exactly the thing Kirk wants to change.

---

## 2. We already have two knowledge models, and they disagree

### 2.1 The monster's: subject-keyed, incremental

`play/intel` stores `map[observer]map[subject]holding`. A subject is
*"an opaque, caller-chosen identification within an observer's fidelity: a place key, an entity
ID, a believed identity."*

- `Surveil(observer, channel, percept)` lands a percept and **fades** anything previously current
  via that channel and absent from it: the channel leaves `CurrentVia`, and when `CurrentVia`
  empties the subject becomes `Held` — *"the ghost goblin"*, in the module's own words.
- New testimony on a known subject **overwrites** payload, channel and timestamp.

### 2.2 The player's: hex-keyed, total

`ideas/fog-of-war/design.md` — an **approved** design — models the same problem with a different
unit, and states the difference as its own load-bearing decision:

> Case 8 is the load-bearing one. It is the reason a visible record must state its **full
> contents** rather than only its additions.

and, correcting an earlier draft of itself:

> An earlier draft of case 7 had a monster walking out of sight and freezing on the last hex the
> viewer saw it. **That is not what happens.** If the hex it left stays visible, the viewer
> watches it leave and receives a `VISIBLE` record with `contents: []` — no ghost, correctly,
> because they saw it go. What freezes a memory is *the hex* ceasing to be visible while
> something stands on it, not the entity moving.
>
> **Memory is per-hex**, so it is hex visibility that governs freezing; entity movement never
> produces a ghost on its own.

### 2.3 The state of play

| | ruled | built | on the current stack |
|---|---|---|---|
| fog of war (hex-total) | **yes** | a web concept only | **no** — `HexKnowledgeChanged` lives on `v1alpha2/encounter`, the service the rip-out deleted |
| intel (subject-incremental) | never against the above | **yes** | yes |

**The model that is ruled is not built; the model that is built was never ruled against it.** And
monster behaviour is about to be built on the second one.

---

## 3. The load-bearing difference is TOTALITY, and it answers Kirk's question directly

> *when the ghost is checked and not there the subject would get updated right?*

**Today: no.** The fade pass removes a channel from `CurrentVia` and, when it empties, marks the
subject faded. **It never touches the payload.** So a monster walks to where it last saw you,
looks, sees nothing — and its intel still says you are at (19,3), with nothing recording that it
went and checked and was wrong.

Two states that should drive completely different behaviour are currently identical:

| state | what it should mean | today |
|---|---|---|
| ghost never re-checked | "last I knew, he was there" → go look | `Held`, payload = (19,3) |
| ghost checked, nothing there | "he was there, he's gone" → that lead is spent | `Held`, payload = (19,3) |

A monster would path to the same empty corner forever, arrive, see nothing, and still believe it.

**The fog-of-war model does not have this problem, and not by accident.** Its answer is that an
observation is *total over a place*: you observe a hex and the record states its complete
contents, so looking at an empty corner writes `contents: []` and the stale belief is corrected —
not because anything was deleted, but because **the new observation said everything.**

That is the reconciliation this design proposes: **make a monster's observation total over
somewhere, rather than incremental over subjects.**

Note what this does NOT require: a delete. Fog of war is explicit that
*"knowledge only ever grows or gets replaced... Deletion is the one operation a later observation
cannot correct."* Correcting a stale belief is a replacement, not a removal — which matters,
because `intel` has no verb that removes a holding and should not grow one.

---

## 4. Kirk's door, and what it says about subjects

> we heard a noise banging that is our Intel. if we go open the door and we see pots and pans on
> the ground we would update that banging to be pots and pans on the floor. if we open the door
> and saw fighting we would update banging with there's a fight

**The update half already works.** One holding per `(observer, subject)`; `Report` and `Surveil`
both overwrite payload and provenance on a known subject. Hearing banging then seeing pots
replaces the payload and flips the channel from hearing to sight. `Subject` being explicitly
allowed to be *"a place key"* is what makes "behind the north door" a legal thing to hold a
belief about — and nothing in the codebase uses it that way yet.

**The two outcomes are different in kind, and that is the second question:**

- **pots and pans** — the belief resolves to *nothing hostile*. The lead is explained and spent.
- **a fight** — the belief resolves *into other subjects*. You now hold intel on skeletons; the
  "banging" subject has been explained by things that are not it.

So: when investigating one subject resolves it into others, does the original **retire**, persist
as a **place fact**, or get overwritten with **"explained"**? Nothing answers this today.

The hex-total model answers the first case for free (observe the place, contents are pots) and
answers the second only if entity subjects and place subjects can coexist — which is §6's
question.

---

## 5. What Hide means in this model

With totality in place, Kirk's tactic works without Hide doing anything special:

1. **Turn the corner.** The monster's next observation of the corridor is total and you are not
   in it — your live sighting fades to a ghost, and if the monster then *looks* at the place it
   last saw you, the ghost is corrected rather than left standing.
2. **Go stealth.** Hide's Stealth check is rolled against the passive Perception of everyone who
   currently holds **live** sight of you (`CurrentVia` non-empty). Break line of sight first and
   that set is empty — which is why auto-success is *correct* rather than a bug, once the empty
   set is **computed** instead of assumed.
3. **"They got nothing on me."** A successful Hide should mean the next observation that would
   otherwise re-acquire you **does not**.

Point 3 is the one real new mechanic, and it has a fork:

- **Erase** — remove the holding. Truest to "nothing on me", and it is the one thing fog of war
  says never to do.
- **Suppress** — hidden makes you absent from percepts. The monster's next total observation of
  your hex says `contents: []`, which corrects its belief *through the ordinary path* rather than
  by writing into its head.

**Recommendation: suppress, by omission from the percept.** It keeps `hidden` a fact about *you*
like every other condition, it needs no new intel verb, it produces the ghost-correction for
free, and it means "hidden" and "behind a wall" are the same mechanism — which is what a player
means when they say the two are interchangeable tactics.

---

## 5a. RULED (Kirk, in the thread that produced this): intel is belief, not fact

> if enemy is the subject the monster checks the ghost, that subject would be updated to at min
> not here or unknown. I think monster needs the ghost memory to act on. **Intel isn't a fact, it
> is what the operator knows or believes.** it can be lied to like charm or deception could have
> false things injected in

This corrects §3's lean and dissolves what was question 1. It was posed as *hex-total OR
subject-keyed*, and that was a false choice — **fog of war is already two layers**, and says so:

> A placement carries `entity_id`... The entity it references carries what the server is willing
> to disclose to *this* viewer... **entities are a separate collection.**

So the model is both, answering different questions:

| layer | question | behaviour |
|---|---|---|
| **place** | "what is on this hex / in this room?" | TOTAL. An observation states full contents; looking at an empty room corrects a stale belief |
| **subject** | "what do I believe about this creature?" | PERSISTENT. Survives not-seeing, degrades, and can be wrong |

**Kirk's correction is that the subject layer must not be erased by the place layer.** Under a
pure hex model, arriving at an empty room replaces the belief with "empty" and the monster loses
the hunt — it no longer knows there is anyone to look for. That is exactly backwards: the
observation should degrade *where*, not delete *who*.

So a subject's position belief takes a third state beyond `Current`/`Held`:

- **`Current`** — sustained by a live channel. I see him, he is there.
- **`Held`** — a ghost. I saw him at X; I have not seen him since; **X is still my best guess.**
- **(new) disbelieved / unknown** — I went to X and he was not there. I still know he exists and
  I am still hunting him; **I no longer have a position.**

That third state is what turns "walk to the ghost" from a loop into a search, and it is what the
behaviour ladder's rung 4 needs to be more than `Pass` (see `acting-on-ghosts.md` §4).

### Current intel cannot conflict — and that is structural, not a rule to enforce

Kirk: *"you cannot have conflicting current Intel, it is either there or not."*

**Already true and unrepresentable otherwise.** Storage is
`map[observer]map[subject]holding` — one holding, one payload. There is no shape that can hold
"he is at A" and "he is at B" simultaneously, so the invariant needs no guard, no validation, and
no test. It is a property of the data structure.

`CurrentVia` being a *set* does not weaken it: multiple channels can **sustain** one belief, but
they all sustain the same payload. Sight and hearing agreeing that he is at A is one belief with
two supports, not two beliefs.

### The one place that gets interesting, given §5a

Every landing is an **unconditional overwrite**:

```go
h.payload = payloadCopy
h.channel = in.Channel
h.at = in.At
h.currentVia[in.Channel] = struct{}{}
```

So the invariant is kept by **last-writer-wins**, with no notion of one channel outranking
another. Combined with "intel can be lied to", that means **a deception channel can overwrite
what the observer is currently looking at.**

Which may be exactly right — being fooled while staring straight at something is what an illusion
IS — or exactly wrong, if the intent is that seeing beats being told. **This is a ruling, not a
defect**, and it is the natural consequence of the module's own *"intel treats all identically"*.
It needs answering the day a rulebook lands a non-perceptual channel, and not before; recorded
here so that day starts from a decision rather than a surprise.

Note what this does NOT need: channel precedence, confidence scores, or belief merging. The
question is only whether some channels may not overwrite some others, and the cheapest honest
answer — if one is wanted — is a rule the *rulebook* applies before it reports, not a ranking
this module learns.

### And it can be lied to, which is the part that makes this irreducible

If intel were a projection of world truth it could be derived and never stored. It is not, and
Kirk's charm/deception case is the proof: a belief can be **injected** by something that is not
an observation at all.

**The module already supports this and nothing uses it.** `Report` — *"lands discrete testimony
as HELD. Unknown subjects create new holdings; known subjects are overwritten"* — is the
injection point, and `Channel` is deliberately open:

> Sight is the one predeclared channel; vocabulary is open — physical channels get physics from
> the stage, supernatural from rulebooks; intel treats all identically.

So a `charm` or `deception` channel is legal today, needs no new verb, and lands a belief the
observer holds as sincerely as anything it saw. **"Intel treats all identically" is the design
already agreeing with Kirk** — the module was built to not know whether it is being told the
truth.

Two consequences worth stating so they are not discovered later:

1. **Nothing may reconcile intel against world truth.** Not a debug read, not a test helper, not
   a "fix up stale holdings" pass. The moment anything does, a lie becomes impossible to tell and
   charm/deception have nowhere to live. C2 already forbids the read direction; this forbids the
   write direction.
2. **A false belief must be correctable only the way a true one is** — by later testimony. Which
   is the same rule fog of war already keeps: knowledge only ever grows or gets replaced.

## 6. What I need ruled

~~1. Is the hex the unit of a monster's memory?~~ **Answered in §5a** — both layers, answering
   different questions. It was a false choice.

~~2. Can a monster hold beliefs about places as well as entities?~~ **Yes, by the same answer.**
   `Subject` already permits it and fog of war already separates placements from entities. What
   remains open from §4 is narrower: when investigating one subject resolves it into others (the
   banging turns out to be a fight), does the original **retire**, become a **place fact**, or
   get overwritten with **"explained"**? Only the last needs no new mechanism.

1. **What exactly does "not here" record?** §5a establishes the third state; its shape is open,
   and Kirk named the fork: *"unknown or missing is a value that could be decided on."*

   **Recommendation: explicit, never absent — and this codebase has already ruled this exact
   class of question, three times, in the same direction.**

   > `NO OMITEMPTY: false is an ANSWER, not an absence` — Declaration.Available
   >
   > `Remaining:0 is a real answer (nothing left this turn) and must not collide with "this verb
   > carries no such number at all"` — Declaration.Remaining
   >
   > `A projection that lets Go's "" fall through to proto's 0 would report every banked swing as
   > "the producer forgot to say"` — Slot

   Here the collision is not hypothetical. `Sighting.Seen` is a pointer today, and nil already
   means *"this is not sight-channel knowledge"* — its own doc says a nil on a sight holding is
   **"NOT a legal state a caller should plan for"**. So representing "I looked and he was not
   there" as a missing position would make the two indistinguishable: *no sight knowledge* and
   *sight knowledge that says I do not know where* are opposite facts, and one of them is the
   whole point of the third state.

   Three states, kept apart on purpose:

   | belief | means | a driver does |
   |---|---|---|
   | no holding at all | I do not know this creature exists | nothing — it is not on the list |
   | holding, position believed | `Current` (see him) or `Held` (ghost) | rung 1/2, or rung 3 |
   | holding, **position unknown** | I know him, I went and looked, I have no cell | rung 4 — **search**, not shrug |

   **The open part is the richer variant**, and it is a genuine choice rather than a default: does
   "unknown" also record *where it is not* — the rooms already cleared? That is real information
   a search can use, it is what makes a monster look like it is hunting rather than wandering,
   and it is the difference between the pillar-camping fix and an actual search behaviour. It is
   also more state, and it is the sort that goes stale on its own (a cleared room stops being
   cleared the moment he could have walked back into it). **I would ship the plain version first
   and let the collaborator find out whether they want the cleared set** — §6's staleness question
   is the same shape and has the same answer.

2. **Does the turn-clock driver get to see ghosts?** Today `MonsterView.Seen` is `Current` only,
   deliberately. Kirk's *"they would check last known position"* implies a fight-time driver
   should see them. That is a contract change to `MonsterView` and it is the one thing here that
   directly changes what the friend inheriting monster behaviour builds against.

3. **Suppress vs erase for Hide** (§5). I recommend suppress and think the argument is strong,
   but it is the difference between "hidden is a fact about me" and "hiding edits your memory",
   and that is a ruling.

Everything else in here is either already ruled (fog of war and now §5a), already built (intel's
update semantics, and `Report` as the injection point nothing uses yet), or follows from those.

## 7. What this is not

Not a Hide fix — that stays a two-line change on rpg-project#300 once question 4 is answered.
Not a rewrite of `play/intel`, which is well-built and whose update semantics are already what
Kirk described. And not monster behaviour itself: this is the world model that behaviour reads,
written down before the behaviour is, so the friend who owns #201 inherits a decision rather than
an accident.

— platform agent, on behalf of KirkDiggler
