# Explicit Tabletop Death Saves — Implementation Record

**Status:** In progress
**Started:** 2026-09-03
**Living design PR:** rpg-project#357
**Delivery slice:** rpg-project#359

This file records facts learned while implementing `design.md`. It does not
silently rewrite the approved intent: each unknown is stated with the ruling
used to continue, its evidence, and the cost if the ruling is wrong.

## Released providers

### Root D&D life state and authoritative Death Save

- Issue: rpg-toolkit#1437
- PR: rpg-toolkit#1442
- Merge: `737fafbcf55a13ba7d2b844cfdfc5329e7df4a8d`
- Release: `rulebooks/dnd5e/v0.132.0`

Observed implementation facts:

- Conscious, Dying, Stabilized, Dead and Defeated are derived rather than
  persisted as a second enum.
- One capacity-only Death Save is banked per eligible Dying turn.
- Natural 20 restores one HP while preserving the normal active-turn slots.
- Positive damage at zero advances failures directly in the character keeper;
  zero/immune damage and damage to Dead do not.
- Dead healing is refused before Hit Dice or Second Wind roll/spend.
- Death Save state is cloned at load/export and spent capacity cannot resurrect
  through either ledger bridge.
- Legacy condition and direct mutation doors are inert; Character data is the
  sole progress authority.

Review history: three fix rounds closed legacy mutation/healing order,
persistence aliasing/spent-capacity resurrection, and one malformed test roller.
Full root tests/lint and CI passed.

## Encounter participation — released

- Issue: rpg-toolkit#1438
- PR: rpg-toolkit#1453
- Final branch head: `680c25b82e2e0b4353f8fbb4a69e9ff0a4f682cf`
- Merge: `f5592a80062a78664f421893bcdcde4bc2f73119`
- Release: `rulebooks/dnd5e/encounter/v0.50.0`

The first implementation established a rulebook-neutral participation answer:
Down narration, Contact, and turn behavior (`Wait`, `AutoPass`, `Remove`) are
independent; PartyDefeated is supplied. Dying retains its slot, Stabilized
auto-passes, Dead/Defeated leaves initiative while remaining on map/roster, and
Death Save Story detail is closed primitive data.

### Unknown 1 — Remove and Contact can contradict

**Evidence:** Both Copilot and the independent branch review showed that a
member answered `Turn:Remove, Contact:true` could still form or join a bubble.
At first light a removed goblin produced a turn bubble, then the next EndTurn
wrote defeat — a fight formed with a corpse.

**Ruling:** Refuse `Remove + Contact` as an incoherent capability answer. Keep
the post-removal defeat census exclusion as defense in depth.

**Cost if wrong:** A future rule that wants a member removed from initiative
while still forming hostile contact needs a new explicit state instead of this
combination.

### Unknown 2 — a drive loop can carry stale participation through strikes

**Evidence:** The loop captured AutoPass before a driven strike; a nested
interaction changed the member to Wait, but the outer loop still auto-passed the
old answer. Current monster targeting makes the case latent, not impossible by
contract.

**Ruling:** Re-assess at the top of every drive-loop iteration. An interaction
that may change character state ends the useful lifetime of the prior
assessment.

**Cost if wrong:** One additional provider assessment occurs per driven slot.
The alternative risks skipping a player from stale state.

### Unknown 3 — stabilization during Record advanced before dice reveal

**Evidence:** Recording the active Dying player's stabilizing Death Save made
the capability answer AutoPass, so `Record` ended the turn immediately. The
approved flow requires the physical die to settle before the client follows the
provider's `END_TURN` continuation.

**Ruling:** AutoPass applies when a slot becomes active through a clock boundary,
not to the already-active member whose mid-turn Record changed state. Death Save
Record retains that active slot until settlement continuation.

**Cost if wrong:** A non-Death-Save rule that stabilizes the active member must
also state whether its current turn is deferred or immediately passed.

### Unknown 4 — recovery invalidates the old “down told once” ledger

**Evidence:** Down → recovered → down again produced one Down beat because Story
remembered only that the member had ever been narrated down inside retention.

**Ruling:** The already-designed `OutcomeDeathSave` detail with `Recovered:true`
resets Down told-ness for that actor. The next fall can narrate Down again. No
new Up event is added in this slice; a future non-Death-Save healing/recovery
verb must carry its own durable recovery fact.

**Cost if wrong:** Consumers wanting a general Up event still need an additive
wire decision later, but the current natural-20 path remains truthful without
inventing that event now.

### Unknown 5 — winning can strand a Dying ally on the world clock

**Evidence:** With one conscious player and one Dying ally, removing the final
hostile dissolved the bubble. The Dying ally moved to the world clock, where the
approved Death Save declaration cannot exist.

**Ruling:** Add a supplied group answer meaning “keep this turn order alive.”
The D&D provider will set it while an eligible Dying member still needs Death
Save turns and at least one party member remains conscious. Encounter does not
infer why. PartyDefeated still wins when nobody is conscious.

A Death Save Record that changes the active player to Stabilized or Conscious
defers same-call dissolution/auto-pass until the approved continuation:

- ordinary result while still Dying: keep the bubble;
- third success: settle/reveal, then automatic EndTurn may dissolve;
- natural 20: keep the active turn; later EndTurn may dissolve;
- death/party defeat: terminal participation may advance/close immediately.

**Cost if wrong:** The turn bubble can outlive hostile contact while saves are
pending. That is deliberate tabletop scheduling, not a claim that combatants
remain hostile.

All five unknowns above were implemented on the encounter branch. Focused/full/race/lint/CI gates passed. GLM scoped re-review found no remaining Critical/Important issue; the single initial Copilot pass and Claude branch review were answered before merge.

### Unknown 6 — full StatusView is too strict for participation

**Evidence:** The first resolution implementation used `Character.StatusView`
to obtain life state and Death Save progress. GLM review showed that a character
carrying a valid, loadable Shield condition attaches successfully but is
intentionally rejected by the no-magic display catalog. The old Standing read
answered that character from HP; the new Participation read would abort every
verb instead.

**Ruling:** Publish a narrow root character participation view containing only
derived life state and detached Death Save progress. Resolution consumes that
released provider. Do not copy the three-save threshold into resolution and do
not add spell status to the no-magic display catalog merely to make this read
work.

- Follow-up issue: rpg-toolkit#1469
- Resolution issue waiting on it: rpg-toolkit#1439

**Cost if wrong:** One extra root release and public read surface. The
alternative silently couples a combat-critical life-state read to unrelated UI
catalog completeness.

## Review-process ruling

Published PRs receive one initial Copilot review. Fixes receive scoped local
re-review; Copilot is not repeatedly re-requested. GLM is available for local
adversarial task reviews and found the first post-removal census defect before
publication.

## Remaining delivery

1. resolution participation and Death Save entries;
2. session declaration/executor/current-state projection;
3. proto transcription;
4. thin API acceptance;
5. web command, shared die, public progress and narration;
6. real multiplayer verification and final observed-results update here.
