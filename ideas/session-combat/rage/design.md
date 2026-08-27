# Rage — a duration that reads the clock instead of counting

**Slice:** rpg-project#295, part 2 of 3 (combat end ✅ → **rage** → rest)
**Prerequisite:** [combat end](../combat-end/design.md), shipped 2026-08-27

---

## 0. What Kirk ruled

> *"i think 2014 rules are in play and I cannot imagine activating rage would end
> at the end of the turn activated so i think it lasts 1 full turn. I have been
> going off 2014 rules and this game does not need to follow RAW to the letter"*

So: **2014 base, with the activation turn graced.** Two sentences of rule:

1. RAW 2014 — rage lasts 1 minute; it ends early if you are knocked unconscious,
   or if your turn ends and you have neither attacked a hostile creature since
   your last turn nor taken damage since then.
2. **House rule — the turn a rage started is not checked.** Raging and then
   ending your turn without swinging does not drop it.

The letter and the divergence, each in one line, so the house rule reads as a
decision rather than as a bug.

---

## 1. The change, in one paragraph

`TurnsActive` **accumulates** — `r.TurnsActive++` on every turn end — so it is
only correct if no turn end is ever missed. `RoundActivated` **anchors**, and
elapsed rounds are recomputed from it every time. A derived value cannot drift;
an accumulated one silently can, and the whole reason this slice exists is that
turn ends were missed for months.

---

## 2. Where the anchor comes from — and where it must not

The condition needs the round it started in. Two sources exist and only one is
trustworthy.

**Rejected: `ActionEconomy.TurnNumber`.** The character *does* carry a round
number, and `Rage.Activate` could reach it. But it is a **sheet-local mirror**
that `readyForTurn` maintains, and it is **known to go stale across fights** —
that is precisely the bug Kirk hit live in rpg-project#253, where a member
recruited into a running fight began their turn with 5 of 30 feet left, the
number their *previous* fight had left on the sheet. Anchoring a duration to a
value with a documented staleness bug would reintroduce the exact class of defect
combat end was built to close.

**Chosen: the round on the boundary event.** `TurnEndEvent.Round` is stamped by
`play/clock` itself. It is the only round in the system that cannot be stale,
because it is produced by the thing that advances it.

So the condition **discovers** its anchor rather than being handed one: the first
`TurnEndEvent` for its owner establishes `RoundActivated`, and that same turn end
is the graced one. The grace and the anchor are the same event, which is why this
needs no new plumbing through `FeatureInput` and no round threaded from the
encounter into feature activation.

`RoundActivated == 0` means "not yet anchored". Zero is not a valid round —
`clock.Turn` sets `round = 1` on `SetOrder` and back to `0` only when idle — so
the sentinel is the leaf's own vocabulary for "no round", not an invented one.

### CORRECTION after review — the grace is not the anchor

The paragraph above was right about the sentinel and **wrong about using it for
two things at once.** Copilot caught it on rpg-toolkit#1266.

Keying the grace on `RoundActivated == 0` collapses *"not yet anchored"* and
*"not yet checked"* into one fact. They coincide only while rounds are valid —
and `combat.TurnManager` publishes `TurnEndEvent{SubjectID}` with **no Round at
all** (`turn_manager.go:177`), so it defaults to `0`. Such a rage never anchors,
therefore never leaves the graced branch, therefore never checks activity *or*
duration again. **Immortal rage, silently.**

That publisher has no callers — which is the reason to fix it rather than
dismiss it. It is a public type [combat end](../combat-end/design.md) §5
deliberately left in place as documented debt, so the hazard belongs to whoever
picks it up next rather than to today's call graph.

So the two facts get two fields: `SawTurnEnd` is the grace, `RoundActivated` is
the anchor. **The activity check needs no round**, so it keeps working when the
duration cannot be evaluated.

Three degraded cases, kept distinct because collapsing any two is how this went
wrong the first time:

| case | what happens | why not the alternative |
|---|---|---|
| never anchored | no cap; **anchors late** when a round finally arrives | a permanent handicap for a publisher that recovers is worse than none |
| roundless *after* anchoring | cap skipped this turn | `0` would satisfy "round went backwards" and kill a healthy rage on a malformed event |
| round **below** the anchor | ends the rage (`clock_reset`) | re-anchoring hands out a fresh ten rounds and hides the regression; ignoring leaves the difference negative so the cap never fires |

The last one is a **net under combat end**, which exists to make it unreachable.
Reaching it means that removal did not happen.

### And a guard that was asserting nothing

Mutating `event.Round > 0` off the *initial* anchor **survived the whole
suite**: assigning `0` to a field already `0` is a no-op, so zero cannot
distinguish the guard from its absence. Only a **negative** round can — and it
matters more than zero does, because a negative anchor *poisons* the arithmetic
instead of disabling it. Anchored at `-5`, a later round 1 gives `1 - (-5) == 6`
and the rage banks six rounds it never earned, while `1 < -5` never fires.

The guard was correct and unpinned. Same class as *"mutate the FIX, not only the
code it touched"* — the fix is a test, not a code change.

## 3. `onTurnEnd`, after

```go
if event.SubjectID != r.CharacterID { return nil }

if r.RoundActivated == 0 {
    // The first turn end this rage sees: anchor it, and let it pass unchecked.
    r.RoundActivated = event.Round
} else {
    if !r.DidAttackThisTurn && !r.WasHitThisTurn {
        return r.endRage(ctx, "no_combat_activity")
    }
    if event.Round-r.RoundActivated >= rageDurationRounds-1 {
        return r.endRage(ctx, "duration_expired")
    }
}

r.TurnsActive = event.Round - r.RoundActivated + 1   // display only, see §4
r.DidAttackThisTurn = false
r.WasHitThisTurn = false
r.markDirty()
```

Flags reset on the graced turn too: RAW's window is *"since your last turn"*, and
the activation turn's swing must not pay for the next turn's check.

### The cap is the SAME threshold, not a corrected one

Worth stating plainly, because I flagged it to Kirk as an off-by-one to get right
and that was imprecise. The old code did `TurnsActive++` **then** `>= 10`, so it
ended on the tenth turn end. With `TurnsActive == event.Round - RoundActivated + 1`,
`TurnsActive >= 10` ⟺ `event.Round - RoundActivated >= 9`. **Identical.** The old
threshold was right whenever it counted right.

The risk is not that the old number was wrong — it is that **changing
representation is where an off-by-one gets introduced**. Rage spans rounds R
through R+9 inclusive and ends at the end of the owner's turn in R+9. That is the
sentence the test asserts, and `rageDurationRounds = 10` is named so the `-1`
has something to be one less than.

## 4. `turns_active` stays — the web reads it, and one way is silent

Deleting the field was the obvious move and it would have **silently broken the
client**. `rpg-dnd5e-web/src/types/conditionData.ts:108`:

```ts
export function isRagingData(data: ConditionData): data is RagingData {
  return 'damage_bonus' in data && 'turns_active' in data;
}
```

A **duck-typed discriminator**. Remove `turns_active` and the guard returns false
for every rage — no error, no crash; `ConditionBadge` just falls through to its
generic branch and the rage tooltip quietly loses its damage bonus, its duration
and its resistance line.

So the field stays, with its meaning changed from **accumulated** to **derived**:
it is now *set* from the anchor on every turn end, never incremented. Same values
the client already renders (`1` after the first turn end, `10` on the last), and
it can no longer drift because nothing adds to it.

**It is display-only, and the code says so.** No rule reads it. The rules read
`RoundActivated`. That is one source of truth with one projection, not two
truths — the distinction being that a projection has a single writer and is
recomputed rather than maintained.

*(The better long-term answer is for the client to discriminate on `ref`, which
every condition blob already carries. Filed as a follow-up rather than done here
— changing the toolkit and the client's type guard in the same breath is how a
silent break becomes two.)*

## 5. What proves it

Through a real `session.EndTurn`, read out of the persisted sheet — never by
publishing a turn event by hand:

| proof | why it can only pass if the rule is right |
|---|---|
| a barbarian who rages and does nothing else is **still raging** after that turn ends | the grace — this is the RAW behaviour Kirk overrode, so it fails against the letter |
| the same barbarian **stops** raging at the end of their *next* quiet turn | the grace is one turn, not permanent amnesty |
| a barbarian who is hit each round is still raging at round R+8 and **not** at R+9 | the cap, at both edges — one assertion either side is what makes it an off-by-one test rather than a smoke test |
| `turns_active` reads `1` after the first turn end and `10` at the end | the client's contract, pinned where the toolkit can see it |
| a rage whose owner misses a turn end still expires on the right round | the derived-vs-accumulated difference, which is the entire point |

That last one is the test the old implementation could not pass, and it is worth
constructing deliberately: skip a turn end (a member spliced out and back, or a
fight that re-forms) and assert the anchor still gives the right answer.

## 6. Scope

**In:** `RoundActivated`, the grace, the derived `turns_active`, the cap
expressed against the anchor, tests, bumps.

**Out:**

- **Rest** — part 3, its own design.
- **The client's duck-type** — filed, not fixed here (§4).
- **Raging outside a fight.** No turn ends happen on the world clock, so a rage
  begun out of combat never anchors and never lapses until a rest or a fight
  ending. Unreachable today (rage is a bonus action, and the action economy is
  lit by the fight clock), noted so it is not discovered as a surprise.
