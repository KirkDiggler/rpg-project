# The run's end: doors, the locked door, and the boss

**Status:** RULED (Kirk, 2026-08-25) — building. The design+plan PR (rpg-project#269) stays open through implementation.
**Journey:** rpg-project#253. This slice is the journey's own Done-when, verbatim: *"reaches and opens the locked door (toolkit#1135 as a fiction beat), and the run ends with a recorded outcome."*
**Measure:** a party walks the tomb, is refused by the tomb door as fiction — *locked, DC 12* — beats the lock, opens the door, drops the skeleton-captain, and the screen says the run is over. Everyone leaves the bubble and lands back in the game screen. Verified live by Kirk on branches before merge.
**Design already ruled, reused not re-derived:** a door is state on the wall, one state over a set of edges (toolkit#1123, Kirk 2026-08-19); DC is carried, not interpreted — `encounter.Unlock` is TOLD `Beaten`, it never compares (doorverbs.go); death dissolves the BUBBLE, not the encounter (toolkit#959 fork (c)) — this slice adds an ending *trigger*, it does not touch that ruling; endings are declared or the encounter cannot exist (`ErrNoEnding` liveness law); full data down the log until v1.0 (Kirk 2026-08-24) — no perception limiting on door beats or roll detail; the combat panel and additional actions are ideas/combat-turn's lane — this slice does not add action costs or panel machinery.

This document is the contract for ONE feature across four repos. The proto section merges first; everything else builds against it in parallel (memory: how-we-build).

## 1. The run, as the party lives it

1. **Entrance → hall.** The entrance-hall doorway is authored open; you walk through the gap. Unchanged.
2. **The tomb door is shut.** The `hall-tomb` door renders as a closed leaf in its gap, and the walk preview refuses to path through it. (Today the client believes every doorway is passable — `atlasPath.ts` doc: *"Doorway pairs — always crossable regardless of any boundary"* — and submits the walk; the server refuses with a placement error whose door state is text.)
3. **The refusal is fiction, not a defect.** Walking into it anyway gets a sentence: *"The door is locked — DC 12."* A wall stays *"You can't go that way."* A malformed coordinate stays a client bug. Three cases, three answers (toolkit#1135).
4. **Try the lock.** Click the door from an adjacent cell: the panel offers *Try the lock — DC 12 (dex)*. The roll happens on the server; the beat is public: *"Aldric picks the lock — 17 vs DC 12. The door swings open."* or *"The lock holds — 9 vs DC 12."* Failure changes nothing and is retryable; a beaten lock leaves the door OPEN, not merely unlocked (doorverbs ruling, already built).
5. **The room answers.** The door opening refreshes sight — the composition already does this — and the captain's room comes into view; opening a door can form a fight (`FormedBubble` is already on the door verbs' outputs) and the fight arrives the way fights already arrive: on the stream.
6. **The captain drops and the run is over — in free roam.** Today: `fightEnded{BY_DEFEAT}` and the encounter stays open forever. New: the tomb's authored `boss: true` becomes a declared ending. The order is ruled (Kirk, 2026-08-25): the fight dissolves FIRST — `fightEnded{BY_DEFEAT}`, the clock returns to WORLD — and then the encounter closes with `Outcome{ending: "boss-down"}` and a table-wide `ended` beat carrying its key. The run ends standing in the world, not inside a bubble.
7. **The end screen.** The panel is back to *Free roam* when the outcome overlay arrives — *The tomb is cleared* — with one button: **Leave**. The overlay sits on the world, not on the combat panel. Leaving drops you back in the game screen. `withdrawn` (external, already declared) remains the other way out and lands on the same screen with its own sentence.

Everything above renders from the wire. The client computes no rule: not the DC, not the roll, not what ends the run.

## 2. Contract: panel element → wire → today → work

| Panel element | Wire (session v1alpha1) | Today | Work |
|---|---|---|---|
| Door drawn open/closed/locked | `GetDoors` (new read): `DoorInfo{door, state, lock{dc, ability}}` — edges already on `GetAtlas.doorways` by door id | `AtlasDoorway` is stateless by design ("what state that door is in is Doors' business, not a snapshot's"); `Encounter.Doors()` exists, session does not re-export it | toolkit session read; protos; api; web |
| Walk preview refuses shut doors | client-side, from `GetDoors` state over the atlas's doorway edges | pathing treats every doorway as crossable | web |
| Walking into a locked door says why | `Move` fails FAILED_PRECONDITION *"door hall-tomb is locked, DC 12"* | composition refuses with `ErrBadPlacement`, state in text; session's `ErrLocked` translation exists but nothing feeds it from a walk (toolkit#1135) | toolkit encounter sentinel; session translate; api error map |
| Open a shut door | `OpenDoor{session, member, door}` | `encounter.OpenDoor` built and tomb-tested; no seam verb | toolkit session; protos; api; web |
| Try a lock | `Unlock{session, member, door}` → `{beaten, total, dc, door}` | `encounter.Unlock` demands a told `Beaten bool`; `checks.MakeAbilityCheck` computes exactly that verdict; nothing connects them | toolkit session rolls the check; protos; api; web |
| Everyone hears the door | typed event body `door{door, state, actor, dc, total, beaten}` | composition already appends a `"door"` beat (whole roster hears it — fine until v1.0); session `kindFor` has no case → arrives as `unknown` | toolkit session `EventDoor`; protos; api; web |
| The run ends when the boss drops | declared at setup: `TriggerMemberDown{Member: <boss id>}` | triggers are `External` and `ReachedPosition`, full stop; `Boss` is authored, validated, compiled, and dropped — its own comment: *"recorded on the placement so the wave that adds the trigger has the fact already flowing"* | toolkit encounter trigger arm; api sessionworld declares it |
| Client learns the run ended, and how | typed event body `ended{ending}`; `GetStatus` for the full outcome | `EVENT_KIND_ENDED` has no body arm; the key travels only in `payload`, which clients must not decode | protos; toolkit session; api; web |
| End screen, leave the bubble | web: outcome overlay on `ended` → **Leave** → game screen | none: `GameView` never clears `sessionId`, `GetStatus` is never called, `ended` only refetches Afford | web |

## 3. The proto, whole (dnd5e.api.session.v1alpha1, additive)

```proto
// ---- types.proto ----

// A door's state is encounter state, not atlas geometry. GetAtlas carries a
// door's edges (AtlasDoorway, construction-truth, cached once); this carries
// what the door is doing right now. Locked doors are public knowledge down
// to the DC — full data until v1.0.
enum DoorState {
  DOOR_STATE_UNSPECIFIED = 0;
  DOOR_STATE_OPEN = 1;
  DOOR_STATE_CLOSED = 2;
  DOOR_STATE_LOCKED = 3;
}

message DoorLock {
  int32 dc = 1;        // authored difficulty, carried not interpreted
  string ability = 2;  // opaque rulebook ref, e.g. "dex"
  string tool = 3;     // opaque item ref, empty when no tool is named
}

message DoorInfo {
  string door = 1;      // same id AtlasDoorway.connection carries
  DoorState state = 2;
  DoorLock lock = 3;    // set only while state is LOCKED
}

// ---- service.proto ----

rpc GetDoors(GetDoorsRequest) returns (GetDoorsResponse);
rpc OpenDoor(OpenDoorRequest) returns (OpenDoorResponse);
rpc Unlock(UnlockRequest) returns (UnlockResponse);

message GetDoorsRequest { string session = 1; }
message GetDoorsResponse { repeated DoorInfo doors = 1; }

message OpenDoorRequest {
  string session = 1;
  string member = 2;   // who pushes it open — the beat's actor
  string door = 3;
}
message OpenDoorResponse { DoorInfo door = 1; }
// A door that was locked refuses with FAILED_PRECONDITION naming the DC.
// Opening a door can start a fight; that arrives where fights already
// arrive — on the stream — not in this response.

message UnlockRequest {
  string session = 1;
  string member = 2;   // whose hands, whose ability modifier
  string door = 3;
}
message UnlockResponse {
  bool beaten = 1;
  int32 total = 2;     // what was rolled, public — full data until v1.0
  int32 dc = 3;
  DoorInfo door = 4;   // OPEN when beaten; unchanged and retryable when not
}

// ---- events.proto: two new body arms ----

message DoorChanged {
  string door = 1;
  DoorState state = 2;
  string actor = 3;    // empty when nobody did it (authored state at open)
  // dc, total and beaten are set only on unlock-attempt beats — the
  // composition's door beat carries {dc, beaten} on those and nothing on a
  // plain open/close. A beat with dc = 0 is a plain open/close.
  int32 dc = 4;
  int32 total = 5;
  bool beaten = 6;
}

message Ended {
  string ending = 1;   // the declared key that fired: "boss-down", "withdrawn", "abandoned"
}
```

`EVENT_KIND_DOOR` joins the kind enum; `EVENT_KIND_ENDED` finally gets its body (the same additive shape `joined`/`exited` got in protos#242). The old `EventTraversed` headstone stays honored: this kind HAS a producer before it has a name on the wire.

## 4. What each repo builds

**rpg-toolkit — encounter module (one PR).**
- `TriggerMemberDown{Member MemberID}` — a third `Trigger` arm: the ending fires when that member's standing reaches down. Evaluated in `noticeDown`, the one place the composition learns someone is at zero, AFTER the existing bubble logic — a boss death both dissolves the fight (existing ruling, untouched) and closes the encounter (new). The beat order is part of the contract (Kirk, 2026-08-25): `downed` → `bubble-dissolved` → `ended` — the run ends on the world clock. Pin it in the tomb test. Member filter symmetry with `TriggerReachedPosition`: EMPTY means any PLAYER member — the party-wipe ending — so the arm is complete even though the tomb only declares the boss today. Validated at setup like the others (a member-down ending naming a member is fine even when that member joins later — same contract ReachedPosition's filter already has).
- toolkit#1135: `Step` returns `ErrLocked` (with door id and DC in the message) for a locked door, and a shut-door sentinel distinct from `ErrBadPlacement` for a merely-closed one. `ErrBadPlacement` goes back to meaning what its name says.
- Nothing: a failed unlock already beats — `Unlock` deliberately re-states the door as itself so there is one path through `setDoorState`, and the beat carries `{dc, beaten:false}`. The miss is already as much fiction as the hit; the seam only has to project it.

**rpg-toolkit — session module (second PR, pins encounter).**
- Verbs: `OpenDoor`, `Unlock`, and the `Doors` read, mirroring the composition's. `Unlock` is where the verdict lives: load the acting member's sheet, `checks.MakeAbilityCheck` with the lock's ability modifier, tell the composition `Beaten`. The told-not-compared law holds — the composition still never compares; the session, which IS allowed to know 5e, does. Modifier is the ability modifier only; tool proficiency is shelved with the tomb's lock authoring (`ability: dex`, no tool).
- `EventDoor` kind + `kindFor`/`bodyFor` arms for the `door` beat; a `bodyFor` arm for `ended` carrying the key.
- `Move` into a locked door translates to `session.ErrLocked` — the stranded seam comment finally has its source.

**rpg-api-protos.** Section 3, one PR, first merge. Evidence: buf lint/format/breaking + generation compiling — no hand-written tests.

**rpg-api.**
- `Manager` interface + three handlers (`GetDoors`, `OpenDoor`, `Unlock`), seated-gate like `GetRoster` for the read, `callerActingAs` for the verbs; `ErrLocked` → FAILED_PRECONDITION in the error map.
- `sessionworld`: `EndingBossDown = "boss-down"` beside `EndingWithdrawn`; the boss placement's `MemberID` (already computed) becomes `EndingInput{Key: EndingBossDown, Trigger: TriggerMemberDown{Member: bossID}}` at setup. The `Boss` field's "carried and not yet acted on" comment comes out — this is the wave it was waiting for.
- Stream: project the two new bodies.

**rpg-dnd5e-web.**
- `useSessionDoors`: fetch once, update from `door` events (same shape as the roster hook: refetch on beat, keep last on failure).
- `AtlasWalls`: the leaf renders by state — closed/locked shows the leaf in the gap, open swings it aside (or omits it, whatever reads at tomb scale).
- Pathing: doorway edges are crossable only when their door is OPEN.
- Click a shut door from an adjacent cell → *Open*; a locked one → *Try the lock — DC 12 (dex)*. Beat lines for door events from typed facts, both outcomes.
- `ended` → outcome overlay (headline by ending key: "boss-down" = *The tomb is cleared*; "withdrawn"/"abandoned" = the quieter sentence) → **Leave** → existing `onBack` path. `GetStatus` on mount of the overlay for the full outcome. Server-side teardown stays the existing lazy liveness (`get_my_active_lobby` already reads a closed session as "no active lobby") — no new teardown machinery.

## 5. Process

Proto PR first. Toolkit encounter PR, then session PR pinned on it, in sequence (one in-flight PR per module); rpg-api and web build behind the merged proto in parallel. One Copilot round each. Kirk walks the branch: the full measure in §0, two browsers. Merges bottom-up; issues closed manually with pointer comments; board Done.

## 6. Ruled (Kirk, 2026-08-25)

1. **Trigger shape:** `TriggerMemberDown{Member}`, ReachedPosition's filter symmetry (empty = any player = the party-wipe ending, declared by nobody yet). "Boss" stays a content word; the composition only knows "this member's death ends things".
2. **Where the boss ending is declared:** sessionworld, from the compiled `Boss` flag. `dungeonspec` untouched; the yaml grows an `endings:` section when the builder (#169) needs authored variety.
3. **Ending-key vocabulary:** keys stay content strings ("boss-down", "withdrawn", "abandoned"); the client maps key → sentence. No won/lost enum on the wire.
4. **The unlock check:** the session rolls it (ability modifier only; proficiency/tools shelved), the composition stays told-not-compared. Door verbs cost nothing and work on either clock; action economy for doors belongs to the combat-turn lane.
5. **Doors read shape:** a dedicated `GetDoors`. The atlas stays construction-truth cached once; the view stays the sight channel; door state is neither.
6. **The run ends in free roam:** the bubble dissolves before the encounter closes. On the wire, `fightEnded{BY_DEFEAT}` precedes `ended{"boss-down"}`, and the clock a client reads after the end is WORLD. The outcome overlay sits on free roam, never on the combat panel.

## Shelves (named, not hidden)

- `CloseDoor` on the wire (toolkit has it; nothing in the tomb needs it).
- Door knowledge by audience — today the whole roster hears every door beat (toolkit's own "#1020 is where asymmetric perception belongs").
- Tool proficiency / thieves' tools on the unlock modifier.
- The party-wipe ending (`TriggerMemberDown{}` empty-member) — declared when death saves arrive.
- Action cost for door verbs inside a bubble — combat-turn lane.
- Post-run persistence (XP, loot, a run ledger) — the outcome is recorded on the encounter and read via GetStatus; nothing else writes anywhere yet.
