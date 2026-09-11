# The Perception Stream — what an observer knows, and how they get it back

## Status: slices 1 and 3 SHIPPED and walked 2026-09-11. Slices 2 and 4 open. Knowledge model ruled 2026-09-12 — see that section first; it supersedes the pass-context framing.

Provider issue: [rpg-toolkit#1615](https://github.com/KirkDiggler/rpg-toolkit/issues/1615) — **closed** by slice 3; its "observed changes" clause is met. Adjacent: [rpg-api#681](https://github.com/KirkDiggler/rpg-api/issues/681) (live-push of equipment changes), [rpg-toolkit#850](https://github.com/KirkDiggler/rpg-toolkit/issues/850) (per-viewer visibility reconciliation).

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

## The knowledge model — ruled by Kirk, 2026-09-12

Three rulings from one conversation. They supersede the framing below, which
had treated the standing leak as a mechanism problem.

### 1. A global table is truth. It never feeds a per-observer view.

> *"we have a global who is up and who is down and we are referencing that to
> maintain what I see. i think that's the break… the global table is fine, but
> my player would never actually know the whole table and it should not feed
> into my intel. if i dont know someone is down, i dont know it."*

The world keeps one authoritative table of who is standing — that is correct and
stays. The defect is that `projectSightings` **joins it into `Seen` at read
time**, so a memory of a creature reports that creature's *current*
consciousness. It is the same ruling as "intel can contain lies", one layer
down: anything an observer can be wrong about comes from their testimony, never
from a lookup.

**It is three tables, not one.** The projection takes `names`, `kinds` and
`down`. The first two were *decided* — `Sighting.Name` and `.Kind` carry doc
comments ruling that naming is not a perception question. `down` was never
decided; it was inherited from before testimony existed, and it is the only one
of the three that feeds `Seen`, the struct whose whole job is what this observer
believes. Position and equipment come from the payload; standing came from the
join. `Seen` was built from two sources and only one of them was the observer's.

**The test, stated so it can be checked:**

```go
projectSeen(channel, payload)   // and nothing else
```

If that function still takes a global map, the ruling has not landed. `View`
stops calling `standingSet` for sightings entirely.

**This demotes the pass context** (below) from the point to an implementation
detail: the ruling is *stop joining at read time*, and the pass context is only
how the composition gets a legal answer at **write** time, once, when it
snapshots.

**It forces one decision: what "I do not know" looks like.** `STANDING_UNSPECIFIED`
exists and the web already defends against it. This is not a migration artifact
to backfill — **hearing makes it permanent.** You hear someone through a door,
you know they are there, and you have no idea whether they are on their feet.

**Why standing differs from name**, written down so the next reader does not
reopen it: a name does not change under you. Standing does.

Tracked: [rpg-toolkit#1668](https://github.com/KirkDiggler/rpg-toolkit/issues/1668).

### 2. The combat log narrows to what a member can perceive

> *"i think i need to think about the combat log going to everyone. I see now I
> am trying to have it both ways… the thing i think makes this game cool is the
> individual views and I am breaking that."*

Every subject and bubble beat is addressed to the full roster today. The
argument that settles it is **hearing**: "you hear fighting behind the door" is
only a moment because you did not already receive the strike events. Broadcast
the sight events and hearing becomes flavour text on information the player
already has — and illusion has nothing to bite on, because the log handed
everyone the truth.

This is [rpg-toolkit#940](https://github.com/KirkDiggler/rpg-toolkit/issues/940),
open since before the shelf existed, and the shelf was built for it:
`audienceFor` already classifies every beat, pinned by
`TestCallSiteClassification`, and its own doc says the flip "lands as a change to
THIS function's body alone. No call site moves."

**The enabler shipped in slice 1.** Narrowing `moved` used to be unsafe — a peer
walking back into view arrives on *their* movement beat, so cutting it froze
your picture of them permanently. `sighted` closes that hole.

Still to decide: bubble beats scope by **bubble membership**, not perception (in
the fight, you need the clock; not in it, not knowing is the point); the actor
always hears their own beat; exits and endings stay table-wide.

**The end-of-match transcript is a different read, not a weaker stream.** The
live stream stays per-observer; an omniscient transcript is a separate
projection over a log the server already holds in full. #940 names the mechanism
— an empty audience already means "no viewer" in `record`, the natural home for
a spectator channel.

**One honest cost:** a player alone in a corridor gets a very quiet client. That
is the tension working, but the UI has to read as *quiet* rather than *broken*.

Tracked: [rpg-toolkit#940](https://github.com/KirkDiggler/rpg-toolkit/issues/940),
ruled in a comment of 2026-09-12.

### 3. The hearing ladder — a spec for the channel, captured before it is built

Kirk's own words, because they are a better specification than anything that
will be reconstructed later:

> *"if i am in an entrance way and I have seen 0 monsters, I have no idea how
> many monsters are here. I might hear things in a room, but without sight it is
> inferred. if i hear voices that i know are not my party members then I know a
> little more. maybe if I heard the language and knew it was goblin I would know
> a little more. if i speak goblin then a little more."*

**This is why a filtered global table cannot work.** A standing table has exactly
one shape of answer — *is this named creature down*. No amount of per-member
filtering yields "there are several voices and they are arguing in Goblin." The
global table is not merely too wide, it is **the wrong shape**; each channel
yields a different *kind* of knowledge, and only testimony can hold different
kinds.

**The modelling question this opens, and it is not standing.** `intel` is keyed
by **subject** — a holding is what I know about `goblin-3`. Voices in a room
**name nobody**: not which creature, not how many. So the hearing channel needs
knowledge about an *unidentified contact*, which sight has never needed, and a
notion of **promotion** — a contact becoming a known member when you finally see
it. That promotion is the ladder above.

Nothing in the primitive blocks it: `intel.Subject` and `intel.Channel` are both
free strings and `Sight` is the only channel anyone has declared. What is absent
is the design — what a contact's payload holds, and what promotion does to the
holdings on both sides of it.

Tracked: [rpg-toolkit#1669](https://github.com/KirkDiggler/rpg-toolkit/issues/1669).
**No use case has paid for this yet**, and none should be invented for it — it is
written down so the ladder survives.

## The pass context — the mechanism, not the ruling

A single coherent per-observer write says "here is what you now perceive about X" — which needs position, standing and hands answerable **at one point**.

The composition may ask its participation capability **exactly once per pass** (law C8, pinned by `TestParticipationQuestionAndAnswerContract`). That law is load-bearing: a verb that asks twice can get two answers, because a sheet can change mid-verb, and then parts of one action disagree about who is alive. But `rebuildPercepts` — the one place snapshots are written — is reached through `refreshSight` from seven callers, and has no pass-scoped reading to draw on. Asking there is a second `Assess`, and the contract test correctly refuses it.

That is precisely why `Seen` ended up with two remembered fields and one live one.

Three shapes considered:

1. **Thread it** — a parameter through seven call sites. Works; C8 stays a rule each caller must remember, and the next capability adds another parameter to all seven.
2. **Memoise per verb** — small, but "a verb" is not something the code names, so the cache has no boundary to hang on.
3. **A pass context** — one object built at each verb entry holding that verb's single reading of the world. C8 becomes a property of the shape rather than a rule people remember, and the next capability inherits it.

**Recommended: (3).** The tell is that `sightNow()` and `equipmentNow()` are already hand-rolled approximations of it, each with a long doc comment explaining why remembering would be wrong. Its honest cost is that it touches every verb entry.

## Open — needs a ruling

- ~~**The pass context.**~~ **Ruled 2026-09-12** — see "The knowledge model". The
  ruling is that `Seen` is built from testimony alone; the pass context is only
  the write-time mechanism, and no longer a question about whether to fix this.
- **Slice boundaries.** The `sighted` beat is worth doing on its own and needs none of this. The equipment-change beat needs the doorbell from rpg-api. The cursor stitch is a third, independent piece.

## Slices

0. **Audience narrowing** — the combat log stops going to everyone
   ([#940](https://github.com/KirkDiggler/rpg-toolkit/issues/940)). Ruled
   2026-09-12; one function body, and slice 1 is its enabler.
1. **`sighted` beat** — ✅ **SHIPPED 2026-09-11.**
2. **Cursor stitch** — `GetView` returns its seq; `StreamEvents` accepts a from-seq; define the too-old answer. Fixes reconnect as a class rather than per-fact. **Open.**
3. **Equipment-change beat** — ✅ **SHIPPED 2026-09-11**, closed [#1615](https://github.com/KirkDiggler/rpg-toolkit/issues/1615).
4. **`Seen` from testimony alone** — standing into the snapshot, and the global
   join deleted ([#1668](https://github.com/KirkDiggler/rpg-toolkit/issues/1668)).
   **Ruled 2026-09-12**; no longer waiting on a decision, only on the write-time
   mechanism.

## What slices 1 and 3 turned out to be

Written after the fact, because two things were not what the plan said.

**Slice 1 needed a primitive the plan did not know was missing.** `play/intel` reported `FirstContact`, `Refreshed` and `Faded`, and the one transition a consumer acts on was not among them: a ghost becoming current again landed in `Refreshed`, indistinguishable from a subject the observer never looked away from. `Refreshed` fires every pass for every perceived subject, so a beat built on it would have been noise. Building the beat by diffing holdings in `encounter` would have been a second computation of a truth `intel` already owns and throws away one line later — so the leaf got `Reacquired`, the exact inverse of `Faded`.

**One event, three lists, not two event kinds.** `Sighted` carries `gained`, `lost` and `changed`. All three answer one question — *what do I perceive now that I did not a moment ago* — and a client's answer to all three is the same re-read of `GetView`. Kirk's ruling: *"I do not want two paths for 1 thing."* The evidence it was right: the web needed **no new refresh row** for `changed`.

**The beat names who, never what.** The server does not say a weapon was drawn. Saying so would hand a recipient a fact rather than the news that their own view is stale — and the fact is exactly what an illusion has to be able to lie about. Pinned by a test that counts the body's fields.

**A ghost-holder is never told.** `changed` is the declared members intersected with what THAT observer refreshed on this pass, so a member across the map hears nothing and — the case that matters — neither does one holding the subject as a ghost. Their testimony is a memory of an older moment and must not acquire news they never witnessed.

**The doorbell needed no new index.** An early read of rpg-api said no character→session mapping existed and slice 3 needed new infrastructure. Wrong: the lobby is that index. A player sits in at most one lobby (`GetByPlayerID`), which carries the `EncounterID` of the stack it started — the same id as the session, with members seated under their character id.

**Two writers, one path.** The first walk found equipping visible and unequipping not: `EquipItem` and `UnequipItem` are byte-identical where they write, and only one had been decorated. The fix was not the missing line — it was that "the sheet changed" and "watchers are told" were two facts kept in step by hand. They are now one `writeEquipment`, the only place equipment is persisted, so a third writer cannot repeat it.

### Shipped versions

| module | version |
| --- | --- |
| `play/intel` | v0.2.0 |
| `rulebooks/dnd5e/encounter` | v0.72.0 |
| `rulebooks/dnd5e/session` | v0.74.0 |
| `rpg-api-protos` | v0.1.185 |
| `rpg-api` | dev ([#964](https://github.com/KirkDiggler/rpg-api/pull/964)) |
| `rpg-dnd5e-web` | dev ([#1037](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/1037)) |

### Known, left deliberately

Unequipping an **already empty** slot does not refuse — it writes a no-op patch and now rings the bell for a change nobody made. It costs one spurious refetch. Closing it means deciding what a no-op write should mean, which is its own question.

— cross-team agent, on behalf of KirkDiggler
