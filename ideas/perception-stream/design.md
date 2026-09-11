# The Perception Stream — what an observer knows, and how they get it back

## Status: peer equipment SHIPPED and walked 2026-09-10. Stream model agreed with Kirk 2026-09-10/11; not yet sliced.

Provider issue: [rpg-toolkit#1615](https://github.com/KirkDiggler/rpg-toolkit/issues/1615) — **open**, because its "observed changes" clause is not met. Adjacent: [rpg-api#681](https://github.com/KirkDiggler/rpg-api/issues/681) (live-push of equipment changes), [rpg-toolkit#850](https://github.com/KirkDiggler/rpg-toolkit/issues/850) (per-viewer visibility reconciliation).

North star: **what a player knows about another creature is that player's own testimony — taken when they looked, theirs to keep, and capable of being wrong.** Not a read of the other creature's sheet.

## The ruling that decided the shape

Kirk, 2026-09-10, rejecting a client-reads-the-sheet design:

> *"i had thought about having the character sheet available for the client to look from but our Intel is a wild thing. it can contain lies if a player is under a spell they could think the monster is holding a toy. so we need to communicate the Seen on events so each player can get their own view"*

This is a **capability** argument, not a purity one. A live read of the subject can only ever return the truth, so it forecloses illusion, disguise and enchantment permanently rather than deferring them. Anything an observer can be *wrong* about must be snapshotted per observer.

The corollary, from the same conversation: *"we are not here to make things work… that will never happen if we put a band aid in."* A shortcut here does not cost tidiness, it costs a category of game we intend to build.

## What exists today — verified 2026-09-10 by walking a live two-player session

| repo | what landed | version |
| --- | --- | --- |
| rpg-api-protos | `Seen.equipment` + `SeenEquipment` | v0.1.181 |
| rpg-toolkit encounter | sight payload generalised from location to **`SightTestimony`**; `Equipment` capability refused at the door | v0.70.0 |
| rpg-toolkit resolution | carries the capability (required, never consulted) | v0.39.0 |
| rpg-toolkit session | `Seen.Equipment` read from the snapshot; `equipmentSeam` | v0.72.0 |
| rpg-api | passthrough + `internal/converters/assetref` | dev |
| rpg-dnd5e-web | peers draw their observed hands | dev |

**Three claims stay distinct all the way to the client**, and the distinction is the point:

- holding something — `{"offHand": "dnd5e:item:shield"}`
- hands observed, and empty — `{}`
- **no hands to observe** (no sheet behind the subject) — key absent entirely

A client that collapses the last two draws a peer whose data has not arrived as a peer standing there unarmed.

**Identity split:** the toolkit carries **bare item ids** (`"longsword"`). rpg-api mints `dnd5e:item:*`, the rpg-game-assets manifest namespace, which deliberately flattens two the rules keep apart — a shield is `dnd5e:armor:shield` to armour class and one `item` to the thing that puts a model in a hand. The engine never learns the asset namespace.

### The walk that proved the memory model

| step | observer's view of the subject |
| --- | --- |
| both in the entrance | `current` — longsword + shield |
| subject walks through the doorway | `held`, `currentVia=None` — ghost, keeps last-seen cell **and** hands |
| longsword unequipped **while out of sight** | ghost **unchanged** — still longsword |
| subject steps back into view | `current` — shield only. Sword gone. |

Row three is #1615's *"a ghost must not disclose unseen equipment changes"*, satisfied **structurally** rather than by a guard. Row four is why re-acquisition needs no extra machinery: **a sight refresh is a full re-snapshot, not a diff.** Preserve that property — a merge-instead-of-replace optimisation would let a re-acquired subject keep a fact nobody re-checked.

## Two known gaps, both diagnosed

**1. A swap under a watching peer's nose is invisible.** Equipment is snapshotted only when sight refreshes, and `EquipItem` never touches the session — so nothing re-looks. Forcing a refresh makes the change appear instantly, which is how the cause was confirmed.

`EquipItem` stays the **single writer** (Kirk: *"I do not want two paths for 1 thing"*); the session gets **told**. A session-verb swap was considered — it is a real 5e object interaction and would get action economy right — and rejected because it would give equipment two write paths, in and out of an encounter.

**2. `Seen.Standing` reads live and stamps onto ghosts**, so a ghost discloses a standing change it never witnessed — the exact leak equipment avoids. Same struct, same seam, one field behaving differently from its neighbours. Both doc comments now state the defect rather than describing behaviour that was never implemented. Blocked on the pass context below.

## The design — a per-observer stream with a cursor

Kirk's frame, 2026-09-11:

> *"there is usually a cursor and a snapshot since cursor value"*

**Connect** = snapshot + cursor. **Then** = tail from exactly that cursor. **Reconnect** = the same call. There is no special reconnect path, and that is the whole point.

### Four of the five pieces already exist

- per-recipient dense `Seq`, persisted per member — the cursor, and the "gap oracle" that means a beat you cannot see leaves no hole in your stream
- `Story(AfterSeq)` — replay from a cursor
- `StreamEvents` — the live tail
- `intel` holdings — the fold: the per-observer materialised view

### The missing piece is the stitch

`GetViewResponse` carries **no seq**, and `StreamEventsRequest` **cannot start from one**. So a client cannot join snapshot to tail without a gap or a double-apply. Today the client papers over it by re-reading the snapshot on triggers it happens to know about — which works exactly until a trigger is missing, and then the picture is stale indefinitely. That is the reconnect symptom, stated precisely.

### Rules for the build

1. **One act writes both.** One audience computation, two effects: update the holdings for that audience, emit to that same audience. A fold write that can happen without an emit is how the two drift, and that drift is the entire bug class.
2. **Not event-sourcing.** Keep the fold primary; make the emit non-optional at the write site. Replaying hundreds of beats to rebuild a picture is worse than reading it, and the fold is small and already persisted. Same guarantee, none of the machinery.
3. **The snapshot's seq is the exact seq the tail resumes at.** Off by one in either direction is a missed event or a duplicate, and both are invisible until they are not.
4. **Cursors expire.** Retention bounds replay, so the server must be able to answer "your cursor is too old, take a snapshot," and the client must accept that. Without it a long disconnect either replays forever or silently gets a partial picture.

### Filtering is deliberately deferred

Kirk: cheating is not important now. So the change beat **carries no equipment** and broadcasts to everyone; the client refetches its own view, and `GetView` returns only what that observer perceives. The wire leaks *that* something changed, never *what*.

This is cheap to reverse because `audienceFor` already sits at every append site returning the full roster — the shelf built in slice 4 — and dense `Seq` means narrowing later leaves no holes. Two beats already narrow today (concealed-region movement, reveals).

**The expensive part is not the server policy, it is the client's habit.** If the client's rule becomes "I receive everything and decide for myself what I may know," then enabling filtering later is not a flag — every one of those decisions becomes dead code and the client has to learn a different model mid-flight. So the client's rule stays **"render what the server says I perceive."**

## The pass context — the enabler, not an alternative

A single coherent per-observer write says "here is what you now perceive about X" — which needs position, standing and hands answerable **at one point**.

The composition may ask its participation capability **exactly once per pass** (law C8, pinned by `TestParticipationQuestionAndAnswerContract`). That law is load-bearing: a verb that asks twice can get two answers, because a sheet can change mid-verb, and then parts of one action disagree about who is alive. But `rebuildPercepts` — the one place snapshots are written — is reached through `refreshSight` from seven callers, and has no pass-scoped reading to draw on. Asking there is a second `Assess`, and the contract test correctly refuses it.

That is precisely why `Seen` ended up with two remembered fields and one live one.

Three shapes considered:

1. **Thread it** — a parameter through seven call sites. Works; C8 stays a rule each caller must remember, and the next capability adds another parameter to all seven.
2. **Memoise per verb** — small, but "a verb" is not something the code names, so the cache has no boundary to hang on.
3. **A pass context** — one object built at each verb entry holding that verb's single reading of the world. C8 becomes a property of the shape rather than a rule people remember, and the next capability inherits it.

**Recommended: (3).** The tell is that `sightNow()` and `equipmentNow()` are already hand-rolled approximations of it, each with a long doc comment explaining why remembering would be wrong. Its honest cost is that it touches every verb entry.

## Open — needs a ruling

- **The pass context**, as above. Nothing currently shipped depends on it; the standing leak has existed since standing shipped.
- **Slice boundaries.** The `sighted` beat is worth doing on its own and needs none of this. The equipment-change beat needs the doorbell from rpg-api. The cursor stitch is a third, independent piece.

## Slices, smallest first

1. **`sighted` beat** — emit on perception change, where the composition already knows who newly sees whom. No new questions, no pass context needed.
2. **Cursor stitch** — `GetView` returns its seq; `StreamEvents` accepts a from-seq; define the too-old answer. Fixes reconnect as a class rather than per-fact.
3. **Equipment-change beat** — rpg-api tells the session after an equip; session re-looks, writes testimony for live-sight observers, emits the nudge. Closes #1615.
4. **Pass context**, then standing into the snapshot — closes the ghost leak and makes `Seen` uniform.

— cross-team agent, on behalf of KirkDiggler
