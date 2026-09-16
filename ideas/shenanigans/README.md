# Shenanigans — rolls that change minds

**Status:** OPENED 2026-09-16 in session with Kirk. The folder holds the primitive; each verb gets its own file as it is designed. First customer: [intimidate.md](intimidate.md).

## Kirk's north star, in his words

> "It all comes down to the roll. You want to persuade somebody, make a roll. We wanted to see somebody make the roll, and the outcome really shapes the encounter. D&D is the most fun when shenanigans ensue."

> "People will be authoring these dungeons and they can plant whatever they think could lead to the most shenanigans. Having shenanigans is our goal. That is what sets D&D apart. There may be tactical dungeon crawlers; we are here to bring out the players and find interesting ways to win the battle. Disguising yourself as the lieutenant and pretending to take command of the army is one of the things we can do."

## The primitive

A **shenanigan is a check whose success changes what a monster believes.** Not what it *is* (hit points, position, conditions — the combat verbs own those) but what it holds to be true about the party, and therefore what it does next.

Three things already exist and the primitive only joins them:

| Piece | What it is | Where it lives (verified 2026-09-16) |
|---|---|---|
| The roll | An authored check: a list of approaches, each an ability or skill and a DC. Search, Unlock and Hide already run one; it poses for offers such as Bardic Inspiration. Characters are the only checkers (rpg-project#351). | `resolution.MakeCheck`, `encounter.CheckApproach`, `dungeonspec.CheckSpec` |
| The mind's memory | A **deed** — `{Verb, Actor, Target, Where}` — landed on every witness whose senses reach the actor's cell. The driver never reads live state; it reads what the monster holds (rule A2). The preset (retaliator, berserker, coward) decides what a held deed is worth. | `mind/behavior/deed`, `encounter/deed.go` `landAttack`, `rulebooks/dnd5e/behavior` |
| The world's memory | A **fact** — an authored id a record `reveals`, learned per member, that a disposition's `until: { fact: … }` waits for. The faction's mind learning it flips the stance. | `encounter/world.go`, `dungeonspec` dispositions, the hold-out |

A shenanigan's success does two lands from one seam: it lands a deed on the witnesses (the *individual* half — this goblin is cowed) and, if the author asked, teaches a fact to the same witnesses (the *faction* half — the camp learns its sergeant was cowed). Neither store is copied into the other. That bridge, "word spreads", is on the living-world shelf (ideas/living-world/disposition, round 3) and stays there until a verb pays for it.

## The vocabulary the author plants

Each verb is a mind capability, added on its own use case, never a general "social system" (memory: use case brings the mechanism). Filed now so the siblings are in view; none but the first is designed.

| Verb | Check | What the mind learns | Who reads it | State |
|---|---|---|---|---|
| **Intimidate** | CHA (Intimidation) vs authored DC | "this one frightened me" | coward keeps away; berserker takes it as a provocation; retaliator shrugs | [intimidate.md](intimidate.md) — first slice |
| Persuade | CHA (Persuasion) | "this one is not my enemy" | needs a *stand-down* the ladder does not have (Pass while the party is in reach) | not designed |
| Deceive / disguise | CHA (Deception) vs monster insight | "this one is the lieutenant" | the camp's stance toward *that character*; §13 of the living-world brainstorm: a disguise opens verbs | not designed; needs a per-character stance, today stance is a faction pair |
| Take command | Deception, then orders | "the lieutenant said hold" | a mind that takes an *order* as an intent | not designed; the far end of the ladder |
| The runner | none — the monster's own | "there is a fight at the gate" | an ally who did not see it | "word spreads" on the living-world shelf: a hearsay testimony source |

## Laws this folder keeps

- **The outcome is testimony, never state.** A shenanigan writes into what the monster holds; the preset says what it is worth. No verb sets a flee flag.
- **The audience is the witnesses.** Who learns is answered by the witness seam (living-world §22, `witnessesOf`): sight and line of sight from the actor's cell. A scared goblin does not turn the camp unless the author planted the fact that says so.
- **Nothing is gated; everything is a check** (living-world §13). Every character may attempt every verb on every monster. The DC is the monster's, authored or derived, never a lock on the attempt.
- **The roll is seen.** The die goes through the shared dice path and the beat carries the total, the DC and whether it beat it, the way `DoorChanged` does.
