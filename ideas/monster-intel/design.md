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

## 6. What I need ruled

1. **Is the hex the unit of a monster's memory, as it already is for a player's?** Everything
   above follows from this. Saying yes means an observation is total over the cells a creature
   perceived, and stale beliefs are corrected by ordinary looking. Saying no means we keep
   subject-keyed intel and need an explicit "I looked and it was empty" testimony — which is a
   second mechanism for the thing the player side already does with one.

2. **Can a monster hold beliefs about places as well as entities at the same time?** `Subject`
   already permits it ("a place key, an entity ID, a believed identity") and nothing uses it.
   The banging-behind-the-door case needs it; the chase-the-ghost case does not. If yes, §4's
   retirement question needs an answer; if no, "the banging" has to be modelled as an
   unidentified *entity*, which is a different kind of lie.

3. **Does the turn-clock driver get to see ghosts?** Today `MonsterView.Seen` is `Current` only,
   deliberately. Kirk's *"they would check last known position"* implies a fight-time driver
   should see them. That is a contract change to `MonsterView` and it is the one thing here that
   directly changes what the friend inheriting monster behaviour builds against.

4. **Suppress vs erase for Hide** (§5). I recommend suppress and think the argument is strong,
   but it is the difference between "hidden is a fact about me" and "hiding edits your memory",
   and that is a ruling.

Everything else in here is either already ruled (fog of war), already built (intel's update
semantics), or follows from question 1.

## 7. What this is not

Not a Hide fix — that stays a two-line change on rpg-project#300 once question 4 is answered.
Not a rewrite of `play/intel`, which is well-built and whose update semantics are already what
Kirk described. And not monster behaviour itself: this is the world model that behaviour reads,
written down before the behaviour is, so the friend who owns #201 inherits a decision rather than
an accident.

— platform agent, on behalf of KirkDiggler
