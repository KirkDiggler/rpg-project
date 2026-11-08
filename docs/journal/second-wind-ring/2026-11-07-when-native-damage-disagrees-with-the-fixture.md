# 2026-11-07 — What did the damage fixture fail to exercise?

Written during [native Raging QA, S12 #884](https://github.com/KirkDiggler/rpg-project/issues/884), after Kirk's early look: “raging is acting like normal.”

## Question

Does sharing the condition preserve the intended sustain behavior when damage comes from the actual native resolution path, rather than a test publishing an event by hand?

## Evidence

The [independent round](https://github.com/KirkDiggler/rpg-toolkit/pull/2299#pullrequestreview-5815655472) found no concrete implementation defect. Native QA then created two fresh Barbarians through draft/finalize RPCs and used two browser clients. Activation spent only the actor's charge, the first boundary/reload preserved progress, and the four effect rows retained their distinct domains and source IDs. A missed attack sustained Rage; combat end removed it; short rest did not refill the class's long-rest pool.

One real NPC hit exposed something those checks did not. The captain dealt 10 slashing; Raging correctly reduced it to 5, taking the actor from 7 HP to 2. The saved runtime nevertheless said `was_hit_this_turn:false`. [Finding #2312](https://github.com/KirkDiggler/rpg-toolkit/issues/2312) carries the exact act and saved-state evidence.

The vocabulary subscribed to legacy DamageReceived. Resolution deliberately publishes the post-apply DamageTaken fact instead, because publishing the legacy topic can apply damage twice. The handwritten Rage had the same mismatch; its migrated unit fixtures manually emitted the old event and kept passing.

The strengthened root test consumed grace before publishing the real notification and failed. A new resolution test used an actual NPC strike: damage was resisted and applied once, but the saved flag stayed false and Rage ended after reload. Both pass with the corrected binding in root c00e0d76. The consuming modules and API adopted it and passed their full gates. No legacy publisher or per-source Rage implementation was added. The affected live recheck and focused independent closure remain outstanding.

## Interpretation

The architectural boundary was already explicit; the fixture did not exercise it. Copying the old subscriber and its tests preserved a bug, not the intended gameplay. The useful correction is not “test more carefully”: let a real damage-applying machine produce the fact, then assert what survives persistence and a new interaction.

This narrows the confidence in the [earlier reader checkpoint](2026-11-07-following-raging-through-native-readers.md) without erasing it. The verified identity, payment and first-boundary facts still matter. They did not establish damage-only sustain, and an independent review was not a substitute for that native observation.

## Next

Recheck the affected native path on the corrected stack, finish the remaining castle/rest observations, and close only the new finding through focused review. [S12](https://github.com/KirkDiggler/rpg-project/issues/884) remains the delivery record; no merge or completed-wave claim follows from these source fixes.

— cross-team agent, on behalf of KirkDiggler
