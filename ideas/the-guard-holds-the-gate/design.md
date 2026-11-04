---
status: DESIGN, R1–R5 RULED 2026-11-03 and R6 and R7 RULED 2026-11-04 on rpg-project#835; gesture law amended 2026-11-03 on rpg-project#841; R5 (a post holds a passage) from the S1 walk; plan in plan.md
story: rpg-project#836
lane: rpg-project#835
---

# The guard holds the gate — a posted body blocks

**Kirk, 2026-11-03 (#835):** "yeah i think A makes sense. we will want to
trigger a say i think if we try moving past the guards 'who there buddy, can
I help you?'"

Players walk past the castle's gate guards today. The gate door is shut and
OpenDoor opens it; the guards are neutral, so their cells are pass-through;
none of their posts is on the gate's one crossing; and the won stand-aside
teaches `gate-cowed`, which nothing reads. Blocking is not a new mechanism.
The step fold already answers Blocked, PassThrough or Standable from world
state, and a walk already stops short with a public reason. This story puts
that state on the right thing.

The S1 walk moved the cut. A body holds one cell, and the castle gate is a
two-cell opening: the party stepped through beside the posted guard. R5
moves the hold from the guard's cell to the passage his post names. The
use case to develop against is the keep's front gate (below).

## Shape

```mermaid
flowchart LR
    subgraph web["the player (web)"]
        P["walk into the guard<br/>planned through a seen,<br/>non-hostile blocking body"]
    end
    subgraph guard["the guard (encounter)"]
        S["machine state<br/>posted · challenged · cowed · turned"]
        B["blocks his cell?<br/>read from the state"]
        H["holds a passage?<br/>the door his state names"]
        S --> B
        S --> H
    end
    P -->|Move| F{{"step fold<br/>occupantPassage"}}
    P -->|Move| E{{"crossing, per mover<br/>a held door's edges"}}
    B --> F
    H --> E
    E -->|"held, mover not allied"| BR
    F -->|"Blocked by a body"| BR["barred beat<br/>mover · body · cell"]
    F -->|"Blocked by a wall, door, field"| ST["step refused<br/>walk stops short"]
    F -->|PassThrough| OK["step taken"]
    BR -->|"matched alone, at once"| G["the body in the way<br/>on: barred → pick barred"]
    G --> SAY["answered beat<br/>'Whoa there, buddy…'<br/>voice sound"]
    G --> OUT["the stop: its beats,<br/>any fight formed, the reason"]
    OUT -->|Move reads it| P
    FACT["fact: gate-cowed / guards-turned<br/>(learned by the witnesses)"] --> S
    S -->|"cowed, turned: his table walks<br/>him off the crossing"| W["the picture matches"]
```

## The seams, read not guessed

Read at the baseline pinned in plan.md. Toolkit paths are under
`rulebooks/dnd5e/`.

| fact | where |
|---|---|
| A cell answers one of three passages: Blocked, PassThrough (cross, never stop), Standable. | `encounter/cellfacts.go:12-27` |
| The fold is derived on every call and never stored; its creature rows are the 2014 rules: a nonhostile space is pass-through. | `encounter/cellfacts.go:115-135` |
| A member's cell is Blocked when it blocks or is hostile; a downed monster is Standable; anyone else is PassThrough. | `encounter/cellfacts.go:528-540` (`occupantPassage`) |
| The member contributor asks the canvas entity's `BlocksMovement()`. | `encounter/cellfacts.go:303-330` |
| That flag is a plain field set once at creation from `MemberInput.BlocksMovement` and carried on the record. | `encounter/encounter.go:3048-3050`, `:992`, `:1015`, `:1261`, `:2601-2632`; `encounter/board.go:167` |
| Spatial's placement check reads the same entity flag. | `tools/spatial/room.go:452` (`CanPlaceEntity`) |
| An observer's route reads the blocking it holds, not the live flag. | `encounter/observedpassage.go:43` |
| Every monster with a table runs a machine: its authored recipe, else the default. | `encounter/machine.go:339-350` (`programFor`) |
| The authored recipe is stored on the member and on a reserve. | `encounter/data.go:2166` (`MemberData.Recipe`), `:525` (`ReserveData.Recipe`) |
| A step reads the crossing first, then the masquerade, then the destination: Blocked is refused; PassThrough is refused only on the walk's last step. | `encounter/step.go:218-410`, `:334-336` |
| A refused step returns no output, only the error. | `encounter/step.go:138-141` |
| A body-blocked destination wraps `ErrBadPlacement`, so its public reason is "movement stopped at an obstruction"; the private sentence names the body. | `encounter/step.go:347-352`; `encounter/walkend.go:35-48`; `encounter/cellfacts.go:478-503` |
| The session reads only the obstruction's public reason and returns `stopped`. | `session/move.go:455-461` |
| The session checks movement before a step and pays only after it succeeds. | `session/move.go:446`, `:473` |
| A driven walk swallows the same refusals and stops silently. | `encounter/step.go:440-452` |
| A route and a step read the same fold, so a route never plans through a Blocked cell. | `encounter/route_search.go:58-110`; `encounter/clocks.go:2545-2588` (`walkerEdge`) |
| The web plans around a Blocked occupant and a shut door edge, and sends nothing when no path exists. | rpg-dnd5e-web `src/components/session/occupantPassages.ts:14`, `src/components/session/atlasPath.ts:171-174`, `:177-187` |
| A door is state on the wall: open, closed, locked; a creature's own walk opens a shut one and never a locked one; no key or fact opens a lock. | `encounter/door.go:14-70`, `:83-93`, `:148-198`; `encounter/clocks.go:1609`, `:1632-1640`; `encounter/walkdoor.go:13-60`; `encounter/doorverbs.go:363-485` |
| A player's walk refuses a shut door by name. | `encounter/step.go:281-286`, `:377-400` |
| A fact is known per member; dialogue `requires` reads the speaker's own facts. | `encounter/world.go:480-487`; `encounter/option.go:236-262` |
| A fact an answer teaches lands on the witnesses of the line. | `encounter/option.go:396-398` |
| A machine reads `fact:<id>` as "this creature holds it"; `fact` lands on the member who learned it. | `encounter/machine.go:735-739`; `encounter/wakebeats.go:211-217` |
| The machine vocabulary: `time`, scoped `heard`/`deed`/`seen`, unscoped `fact` and `clock:*`, open words `deed:<word>`, `clock:<name>`, `fact:<id>`. | `encounter/machine.go:85-135`, `:700-735` |
| A recipe state carries only its transitions; the primitive knows no rule. | `mind/machine/recipe.go:39-42` |
| The authored dialect compiles `states:` to a recipe inside the encounter module. | `encounter/dungeonspec/states.go:19-80` |
| Landed beats wait for the next raise: time has not passed, so nobody acts on them yet. A reaction pick spends nothing. | `encounter/wakebeats.go:34-50` |
| A targeted beat is matched on the one member it names; a broadcast beat on every listener in scope, the doer excepted. | `encounter/wakebeats.go:318-352` (`matchOne`) |
| A pick's answer reaches its voice after the pick beat: `runIntents`, then `answerTime`. | `encounter/clocks.go:588`, `:628-638`; `encounter/answer.go:254` |
| Every line said on an answer row is a voice: its hearers are the hearers of that sound. | `encounter/doc.go:142-149`; `encounter/answer.go:239-275` |
| The session decodes an `answered` beat into a table answer, and the wire and the web already carry it. | `session/act_family.go:725-765`; `session/act_bodies.go:394-415`; rpg-api-protos `act.proto:104`; rpg-dnd5e-web `describeAct.ts:478` |
| A footprint door's crossings are known from its geometry, whatever its state. | `encounter/placed_props.go:749-787` (`doorCrossings`) |
| The crossing fold is memoized per crossing, the same answer for every mover; the masquerade is the per-mover check beside it. | `encounter/cellfacts.go:178-205`; `encounter/placed_props.go:1067-1073`; `encounter/step.go:310` |
| A member's blocking is read from its current state (S1, shipped). | `encounter/machine.go:322-345` (`memberBlocks`) |
| A sighting's passage is the observer's own answer: the encounter folds what the observer holds, the session puts it on the sighting. | `encounter/observedpassage.go:14-46`; `session/read.go:457-476`; rpg-api-protos `session/v1alpha1/types.proto:1067` (`Passage passage = 11`) |
| The web knows each door per member (`DoorInfo`: id, state, lock) and plans a doorway's edges shut unless its door is known open. | rpg-api-protos `session/v1alpha1/types.proto:2639-2646`; rpg-dnd5e-web `src/components/session/atlasPath.ts:58-80`, `:100-121` |
| No condition stops a walk today; prone leaves movement out. | `conditions/prone.go:49-52` |
| Resolution's `Prevented` is unset by anything; it is reserved for Sentinel. | `resolution/movement.go:114-119`, `:545-548` |

The castle (rpg-api `content/the-mad-king.yaml`, mirrored byte for byte in
the toolkit's `encounter/dungeonspec/testdata/world-builder-v4-castle.yaml`,
which the castle tests read, `encounter/castletest/castle_king_test.go:42`):

| fact | where |
|---|---|
| The stand-aside line is the captain's: Intimidation DC 14; a win (weight 3) answers "…go on, then. I saw nothing." and teaches `gate-cowed`; a win (weight 1) and a loss teach `party-recognized`. | `:1723-1737` |
| The turn is Persuasion DC 15 and teaches `guards-turned`. | `:1738-1747` |
| The garrison is neutral to the party until `guards-turned` (allied) or `party-recognized` (hostile). | `:524-531` |
| `gate-1` (-7,10), `gate-2` (-3,10), the captain (-4,8); only `gate-1` has states (`posted`, `alert`). | `:1544-1565`, `:1681-1707` |
| The gate patrols' loops, and their `guards-turned` march. | `:272-327` |
| The reserves are told to stay off the gate's one crossing. | `:474-478` |
| The gate door is shut and teaches `gate-opened` when opened. | `:2012-2015` |
| The crossing is the cell beneath the gate (where a party stands to open it) and the courtyard cell past it; the patrol posts flank it. | `encounter/castletest/castle_king_script_test.go:53-98` |
| The gate is an opening two cells wide with a double door, `gate-wall-door`. | mirror `:725-731` |
| `gate-1` holds his post by state (S1): `posted` and `alert` block; his `post` key runs to a clang, draws him toward a voice heard this round, and else walks him back to the post. | mirror `:1717-1758`; rows `:321-330` |
| A wall-attached door compiles as a footprint door with no edges, under the castle's qualified id; the atlas lists doorways only from edges. | `encounter/dungeonspec/single_room_doors.go:300-312`; `encounter/castletest/castle_king_test.go:71-72`; `encounter/atlas.go:628-634` |
| A state's `when` reads `enemy`, `fact` and `not`; a sound is a beat a state listens for, never a condition. | `encounter/dungeonspec/states.go:49`, `:337-358` (`guardOf`) |

## The use case: the keep's front gate

`ideas/keep-dungeon/design.md` is the living paper design this story
develops against. Its front gate:

> Four guards check arrivals at an already-open gate (`:276`).
>
> The open doorway remains impassable to the party until admitted, the
> guards are defeated, or the party establishes entry elsewhere. Reaching
> the entrance causes the guards to halt the party and initiate the
> admission interaction. Backing away is possible; slipping through the
> unresolved encounter is not. (`:280-283`)
>
> Defeating the whole gate group opens access; killing one guard does not
> clear the encounter. (`:321-322`)
>
> Successful admission or established entry through another route removes
> the front-entry restriction for the rest of the run. (`:327-328`)
>
> After entry, living gate guards reposition along their route to the
> ballroom. (`:332`)
>
> Unalerted patrol members normally assume the party belongs. They do not
> stop every unfamiliar character for a new entry check. (`:684-685`)

| keep behaviour | this story |
|---|---|
| An open doorway impassable until resolved | builds: a post holds the door's passage, open or shut (R5) |
| Reaching the entrance halts the party | builds: the held step stops the walk by name (S1b); the bump speaks (S2) |
| Killing one guard does not clear it | builds: the hold belongs to the post; any posted member holds it |
| Release by admission, entry elsewhere, defeat | builds: a fact ends the post; defeat empties it |
| Backing away is possible | holds already: only the held edges stop a step |
| Unalerted patrol members do not halt anyone | holds already: only a holding state holds |
| The admission dialogue, its attempts and DCs | not built: content and dialogue |
| Arrest into the prison | not built |
| Guards repositioning to ballroom posts after entry | not built as content; a state change and a table walk carry it |

## Law

**Block by state on the world, never by an invisible permit.** A thing the
player may not do is a thing they can see and act on: a shut door, a body in
the way, a shout. The one permit-shaped thing stays where it is: `requires`
on a dialogue line. Resolution's `Prevented` stays reserved for Sentinel.

**Seeing and blocking are two axes.** Sneaking answers the seeing axis:
an unseen mover is not noticed, and that is all. The body axis is answered
by emptying the cell, for a body that holds only its cell, or by emptying
the post, for a post that holds a passage. The levers already exist, and
each does both, because a held passage needs its holder on his post: a
row that walks him off (a voice or clang he goes to, a pursuit, a dash, a
search) draws him away when his table says so; `gate-cowed` cows him; a
displaced move pushes him; a fact relieves him; a downed body is not in the
way. A posted body never leaves by time: his `time` pick while posted
stands the post, and only a beat (a sound, a fact, a push, a fall) empties
it. An unseen mover who walks into a posted body is stopped like anyone
else.

**A body blocks by its state.** A creature whose states mark one or more
states `blocks` blocks exactly while its current state is one of them. A
creature with no such mark blocks as its creation said (`BlocksMovement`),
as today, whether it runs an authored recipe, the default one, or none. The
answer is read from the state on every fold and by every reader (the fold,
the canvas's placement check, the board, what observers hold), never stored
beside the state, so a state change opens or closes the cell in the beat it
happens. The marked states are stored with the recipe they name. The fold is
unchanged: blocking or hostile is Blocked; a downed monster is Standable;
any other body is PassThrough.

**A post holds a passage, not a cell.** A state may name the door whose
opening its post holds, and the post's cells: where a holder stands to
hold it (by default the cell the member is placed at). The opening's edges
are closed, as a shut door's are, to every mover not allied with the
holder; an ally crosses. His body still blocks its own cell. A state that
names no opening holds its cell only. The opening's edges are the door's
own: an edge door's edges, a footprint door's crossings read from its
geometry. So a hold works on an open door, a shut one and any width.

**The hold is a state at a place.** A post holds its passage while a
holder is in a holding state and standing on one of the post's cells. Any
leaving empties it at once, whatever moved him (a voice he walks to, a
clang, a pursuit, a dash, a push), because he is no longer at his post;
he holds again the step he stands there. Alert is a holding state in the
castle content: a guard who hears something and looks round from his post
still holds. Cowed and turned hold nothing; a downed or dead member holds
nothing. What triggers a draw stays open for the use case; the effect is
settled by R4 and R5 together, so the sneak crosses on `gate-2`'s blind
beat while `gate-1` is drawn off. The post's cells include the cell beside
the opening, which is how the gate is barred where the guard is not
standing. A stranger on his only post cell keeps him from retaking the
hold until the cell is free; a post with two cells avoids it.

**A walk never opens a held door.** The hold is read before a walk's door
pass: a shut door its post holds is not opened by a walk, and the stop
names the holder, not the door.

**A held door refuses its verbs.** OpenDoor, CloseDoor and Unlock on a door
a post holds are refused to any actor not allied with a holder, naming the
nearest holder the actor observes ("Guard bars the door"), else
"someone bars the door", as the crossing is. Unlock reads the hold before
any check is rolled or any help is asked for. The hold is read after reach and before the
door's own state, so a held locked door names the guard, not the lock. The
cells in front of the door stay reachable: that is where the conversation
happens. The attempt is the bump: under S2 it writes `barred` and his table
answers. An ally, and the holder himself, open the door as before.

**The holder is the post.** Several members may hold one opening: it is
held while any of them is in a holding state, up and on a post cell. Killing one guard
does not open the gate; emptying the post does.

**A post holds both ways unless its setting says a direction.** By default
a post bars its opening to movers crossing either way. Its setting may
name one direction: `against: entering` bars a crossing that starts on the
post's side (the region its cells stand in) and ends on the far side;
`against: leaving` bars the reverse; `against: both` is the default. The
door verbs follow the same setting from the side the actor stands on.
A post naming a direction must have one side per crossing: every crossing
of its door has exactly one cell in the post's region, because regions do
not follow walls; a post that fails this is refused.

**A pass fact walks through a hold.** A post's setting may name a fact
(`pass: <fact>`). A mover who knows it crosses the held opening, and works
its door, as an ally does; everyone else is barred. The fact is the
mover's own, read from what the mover has learned, so a party that came in
another way and never learned it meets the guard on the way out. The
barred still get the guard's question under S2. In the castle the troupe
passes by its fact while the guard stays posted, the keep's shape: the
guards stay at the gate and admit the guests. Cowing stays with the
stand-aside.

Both are settings on the post, not engine rules, because a post is
authored content that the studio configures (Kirk on R5: "ideally it is
all configurable in the studio").

**A held step names the holder.** A step across a held edge stops with a
reason that names the holder the mover observes ("Guard bars the way"),
even when the step is beside him, and "someone bars the way" when the mover
observes none. Under S2 the nearest holder is the bump's addressee. The
hold is per mover, so it is read beside the masquerade, never in the
crossing memo every mover shares. Provoking and facing take nothing new
from it.

**A posted body departs from the 2014 rule on purpose.** A nonhostile
creature's space is pass-through unless the creature is posted to hold it.
RAW is not the authority here.

**The gesture is walking into him.** The web prefers the way round: it
plans around a seen body whose passage is Blocked, as today. It plans
through a seen, non-hostile one, as if it were pass-through, only when no
route round exists, and sends the walk; the hover preview draws a path into
him only then. Clicking a body's cell means approach to speak, so the
gesture is clicking past him where he is the only way, as at the gate's one
crossing. The first cut also planned through him when his cell was the
goal; the lead dropped that leg on the rpg-project#841 review (2026-11-03):
"clicking a body's cell already means 'approach to speak' in the web, an
earlier operator choice this story does not reverse. The planner's second
plan therefore drops its 'his cell is the goal' leg and keeps only 'no route
round exists'."
The server owns the step and stops it at the body's cell.
The web never second-guesses that answer: it stops the walk where the
server says. A hostile body stays unplannable, as today.

**A barred step names the body.** A step refused by a body, on its cell or
on an edge its post holds, says who stands
in the way, as a shut door says it is shut. A body the mover does not
observe is "someone". A wall, a door and a field keep their own sentences.

**The bump is a beat the body hears.** A player's step refused by a body
writes a `barred` beat (mover, the body in the way, the cell), on every
bump. The body in the way is its addressee: the beat is targeted on him and
matched on him alone, at once, in the Move that bumped. That is the one
exception to "landed beats wait for the next raise": a bump is a touch on
one creature now, not time passing for the world. Every other landed beat
still waits. His machine may answer on `barred` with a table key, and that
key's row speaks as any row speaks: an `answered` beat, the voice's sound,
the record. Whoever else reacts hears that voice through `heard`, when time
next passes. A driven walk routes around a body and never writes `barred`.

**A bump is a stop, not a refusal.** In S1 a body stop is an obstruction
like a shut door. Once the bump writes, the step answers it with output,
not an error: the beats it wrote, the intel they changed, any fight formed
or ending reached, and the stop's reason. The session folds that output
like any step's and reports the walk stopped.

**A bump spends nothing.** On the world clock nothing is spent. On the turn
clock the refused step is not a step, so no movement is paid; the steps
already taken stay.

**Standing aside is both state and walk.** A fact that ends the post moves
the creature to a state that does not block, in the beat the fact lands, so
a cowed or turned guard never blocks. His table walks him off the crossing
on his next beat so the picture matches. The engine guarantees the state;
the step aside is content.

**Hostility still blocks.** A hostile body is Blocked whatever its state.
Downed bodies, allies and pass-through bodies are unchanged.

## Rulings

| # | question | options | ruled | scope | date |
|---|---|---|---|---|---|
| R1 | What holds the gate? | A a posted guard blocks the crossing; `blocksMovement` becomes machine state; `gate-cowed` and `guards-turned` turn it off; the door stays as it is. B the door is barred by the garrison and a fact unbars it (rejected: it gates the door, not the guard; the guards stay decorative, and a locked gate tells "nobody is home" where the story is "a man who says no"). C both, as two slices (rejected: the door half has no use case yet, and a slice without one is not cut). | A (Kirk: "yeah i think A makes sense. we will want to trigger a say i think if we try moving past the guards 'who there buddy, can I help you?'") | a member's blocking; the castle gate | 2026-11-03 |
| R2 | How does the bump speak? | A the bump is something the guard hears: a `barred` beat, scoped like `heard`; his `on: barred` row says his line on his own machine. B the stop reason carries a string (rejected: no voice, no wake, no record; a tooltip, not a guard). C the session plays Speak at the barrier (rejected: it inverts who speaks first). | A (Kirk: "A sounds great") | the machine vocabulary; the Move answer; the wire | 2026-11-03 |
| R3 | What is standing aside? | A state only (rejected: bodies overlap for a step). B walk only (rejected: a delayed walk still bars). C both: the state flips at once and his table walks him off the crossing. | C (Kirk: "oh yeah both sound good. I was having a hard time picking") | a posted member's states; castle content | 2026-11-03 |
| R4 | The gate sneak against the post. | Today (wave C, #606 piece 4) the gate is a sneak: both gate guards walk a loop, and two world rounds of every four both stand at their beat posts with their backs to the gate (the loop's blind beat, `the-mad-king.yaml:296-301`); nobody stands in the crossing. A the post holds the crossing continuously while `posted`: one held body (`gate-1` in the crossing) and one wandering eye (`gate-2` keeps the loop); `posted` never leaves by time; the gate is talked, cowed or turned through, or sneaked through as a harder route: the posted body is emptied first (drawn off by a noise if his table says so, cowed, pushed, downed), then the crossing is taken on `gate-2`'s blind beat; piece 4's sneak test is re-aimed to exactly that (cost: the sneak needs two moves, not one window). B the crossing is one post of `gate-1`'s loop (rejected: the gate is held some rounds of four, a civil party that waits walks through, and "a man who says no" becomes a matter of timing). C a sneaker slips past a posted body on a check (rejected for now: it answers the body axis with a seeing roll; deferred as a verb of its own). Recommended A; the lane's wording for the sneak: draw `gate-1` off, cross on `gate-2`'s blind beat. | A (Kirk: "i like it") | `gate-1`'s post, `gate-2`'s loop; the castle's sneak and open-the-gate steps | 2026-11-03 |
| R5 | What does a post hold? | The S1 walk (Kirk on the stack, 2026-11-03, rpg-project#841): the gate is a two-cell opening with a double door, so the party stepped through beside the posted `gate-1`; and a talk with the pedlar drew him off his post (his `post` row draws toward a voice heard this round). R1's cut was a posted guard blocking the crossing as his one cell. A a post holds a PASSAGE: a posted body bars the edges of the opening his post names, as a shut door does, for anyone not allied; the reason names him even beside him; his body still blocks its cell; a post naming no opening holds its cell only; the hold is a state, not a distance (amended on the #846 review: a state at a place, read while the holder stands on a post cell); the holder is the post, so a group may hold one gate. B two bodies, one per cell (rejected: a workaround; it breaks at three cells, and a drawn-off guard still opens his half). C narrow the castle gate to one cell (rejected: backwards; the content bends to the engine). | A (Kirk: "it is A and i think we will want a use case to develop until we make any decisions. ideally it is all configurable in the studio. this content is great since it really tests and reveals the seams and our decisions are either empowering or restrictive.") | a post's hold on an opening; the castle gate; the keep's front gate | 2026-11-03 |
| R6 | Which way does a post hold? | S1b built the hold both ways, as the law read: a party inside the castle cannot walk out through the gate while `gate-1` is posted. Castle QA routes that leave through the gate (five call sites in the cells, companion, keep and gate tests) stayed green only through a named test-only switch, `castleHost.gateUnheld`. The use case is the keep's front gate: its guards "check arrivals" (`ideas/keep-dungeon/design.md:276`), and a successful admission removes the restriction for the run (`:327-328`). A both ways (recommended: a gate guard challenges strangers leaving as well as arriving). B one authored side (fits "check arrivals" literally; costs an authored direction and a side rule). C both ways but a known pass (A plus a release fact). | A with two knobs on the post's setting: both ways by default; a direction, `against: both \| entering \| leaving`, for the cases that guard one way; and a pass fact, `pass: <fact>`, that walks a mover who knows it through while everyone else is barred (the keep's admission teaches it to the guests; a party that sneaked in by the kitchen never learned it and meets the guard on the way out). The barred still get S2's question. The castle routes that leave through the gate are rewritten honestly and `gateUnheld` is deleted. (Kirk: "so what if the party upon entering under a guest guise they get a fact that would override the guard from barring it. i think if the guards are guarding people from entering it would not apply to the other direction. something would happen, questions at minimum, but i think the encounter designer would use facts to control that" then "so i think there could be cases where we guard one direction. i think having that built in as an option on the setting would be best") | a post's setting: direction and pass fact; the castle gate; the keep's front gate | 2026-11-04 |
| R7 | What does a held door do to its verbs? | Found on Kirk's S1b walk (2026-11-04): he opened the held gate from the cell beside the guard, and walked through. A OpenDoor, CloseDoor and Unlock on a held door are refused to anyone not allied with a holder, naming him ("Guard bars the door"), as the crossing is; the cells in front stay reachable, since that is where the conversation happens; the attempt is the bump (under S2 it writes `barred` and his table answers "what do you want?"); the keep's two admission attempts and the arrest are content on top of S2 (a `challenged` state counting attempts, an arrest row), out of this story. B the post holds the approach cells (rejected: a second kind of hold, and it makes the keep's "reaching the entrance causes the guards to halt the party" impossible, since nobody reaches it). C nobody can open a held door, guard included (rejected: the guard's side can never open his gate, even for the troupe). | A (Kirk: "I should not be able to open that door. what we want is to block the space in front of the door so no one can attempt to open it. long term we will want to trigger a say on the npc asking us what we want. after a couple more attempts the scenario is to have us arrested." then "A works for me") | the door verbs on a held door; the castle gate; the keep's front gate | 2026-11-04 |

## Assumptions, not ruled (open to veto on the design PR)

- **A repeated bump is the guard's states' business.** The engine writes
  `barred` every time; content moves `posted` to `challenged` for a few
  rounds with a shorter row, as `posted` moves to `alert` today.
- **The barrier is the addressee; bystanders hear his voice.** A bump is a
  touch, not a sound; the guard's shout is the sound, and it already reaches
  bystanders. The rejected cut is a broadcast `deed:barred`: it needs a word
  in `mind/behavior`'s deed vocabulary (another module, and a wider table
  `when:` vocabulary); a deed's subject is the actor, so the barrier's
  identity is lost; and it would make the touch a sound.
- **The gate holds one body and one eye (R4 A).** `gate-1` holds the
  cell beneath the gate, in front of the shut door, continuously while
  `posted`; `gate-2` keeps the loop. The stand-aside line stays on the
  captain; whether the barring guard carries it too is a walk finding.

## Findings, not ruled

- **A body's refusal is generic today.** A body-blocked step wraps
  `ErrBadPlacement` and its public reason is "movement stopped at an
  obstruction" (`encounter/walkend.go:41-43`). S1 gives a body its own
  sentinel and sentence, checked first; the driven walk's silent list
  (`encounter/step.go:447`) takes the new sentinel.
- **Observers hold blocking, and the web plans from it.** A route planned
  from observation reads the blocking it holds
  (`encounter/observedpassage.go:43`), and the web plans from the same
  passage. A state change refreshes what its observers hold.
- **The canvas holds a plain flag.** `memberEntity` keeps a bool spatial
  reads for placement. It reads through the encounter instead, so the
  canvas and the fold cannot disagree.
- **The state binding lives in the encounter.** `mind/machine` stays
  rulebook-agnostic (its `State` has transitions only), so `blocks` is the
  encounter's word: the dialect's `blocks: true` on a state compiles into
  the member's set of blocking states, stored beside its recipe. A bag key
  set by acts was the other cut; it can drift from the state when one
  transition forgets to set it.
- **`gate-cowed` lands on the witnesses.** The barring guard steps aside on
  `fact:gate-cowed` only when he saw the captain cowed. A guard who did not
  see it holds the gate; that is the story, and a test proves it.
- **The post moves two castle steps.** The sneak dash ends on the cell
  beneath the gate (`TestMadKingScriptASneakWindowAtTheDoor`), and opening
  the gate stands there too (`TestMadKingScriptOpeningTheGateBringsTheFarRoom`);
  the walk skill's castle steps 2 and 5 say the same. Under R4 both are
  re-aimed in the slice that posts the guard.
- **A bare opening has no id the encounter holds.** The castle's gate
  opening carries a door (`gate-wall-door`), and the encounter knows doors
  by id; an opening with no door compiles to no record. A post names a door
  in S1b. The keep's "already-open gate" is a door authored open.
- **Today every leaving is a table row, not a state.** `gate-1`'s `post`
  key runs him to a clang, draws him toward a voice, and walks him back
  (mirror `:321-330`); a push moves no state. Because the hold reads where
  he stands, none of these needs a state of its own, and S1b adds no state
  and no guard word for them. If content later wants a state to change on
  a kind of sound, it rides a `heard:<kind>` beat, landed as `deed:<word>`
  is; a sound never becomes a state's `when`.
- **The castle's gate is a footprint door.** Its crossings are not in the
  atlas today, so the web cannot mark it shut or held. S1b lists a
  footprint door's crossings as atlas doorways marked `crossing` (one
  additive field), which also lets the web plan a shut footprint door as
  shut. The web draws a doorway gap, leaf and wall cut for every doorway
  today; a `crossing` doorway draws nothing, because the door is already
  drawn as its placed asset.
- **The captain does not react to the voice.** He has no states and no
  `heard` row; turning him toward the gate is not in this story.

## What we are not building

- The keep's admission attempts and the arrest after repeated attempts
  (R7): content on top of S2, a `challenged` state counting attempts and
  an arrest row.

- A barred, locked or fact-unbarred gate door (R1 B).
- A permit on movement, a `requires` on a cell, or `Prevented` set by
  anything but a future Sentinel.
- Grapple, restrained, difficult terrain or any other condition that stops
  a walk.
- A bump that costs anything, or an opportunity attack for one.
- The session auto-playing a dialogue line (R2 C).
- A size-difference exception to pass-through.
- The captain's reaction to the guard's voice.
- The keep's admission dialogue, its attempts and checks; arrest; the gate
  guards' walk to ballroom posts as content (the use case section).
- A hold on an opening with no door.
- A slip-past check (R4 C). If it comes, it is a verb of its own with its
  own cost, never a rule of sneaking.

## Open

- **What distracts a post** (content, waits for the use case; Kirk on
  R5). Evidence from the keep: its gate guards halt the party
  (`ideas/keep-dungeon/design.md:281-283`), and nothing in the keep draws a
  posted guard off for speech. Recommendation, not ruled: a posted guard
  answers a voice from where he stands; a clang, or a shout out of his
  sight, draws him; the sneak lures with a clang. Under the hold at a
  place, whatever the call, a draw that walks him off his post opens the
  passage.

- **The barring guard's own stand-aside line.** A walk finding.

— cross-team agent, on behalf of KirkDiggler
