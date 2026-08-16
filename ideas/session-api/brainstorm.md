# API session integration — brainstorm

2026-08-15, Kirk + director session. Issue: rpg-project#227.

## The question

The session SDK closed W4 (toolkit#966): it walks, sees, fights, swings, and
disengages with zero rules of its own, stack version-aligned at
`session/v0.8.0`, `encounter/v0.9.0`, `resolution/v0.7.2`, `dnd5e/v0.94.1`.
The standing gate said: integrate rpg-api only at capability parity with the
old path. Do we wait for parity, or integrate now at today's capability —
roughly "walk around a room and fight a monster"?

## The ruling: integrate now

Kirk: *"parity was a thought I had for about 10 minutes... seeing the updates
end to end as we bring them in is very valuable."* Why the ruling is right:

- **The parity gate protected a game nobody plays.** The dungeon builder is
  unfinished, so the old path's feature surface is inventory, not a
  commitment. Regressing to walk-and-fight costs nothing.
- **Parity-before-integration inverts the feedback loop.** It builds the
  entire remaining surface (action economy, reactions, monster behavior) with
  zero real consumers, discovering seam problems last. Integrating now makes
  rpg-api the SDK's first real consumer — the claim that replaced the old
  wrapper plan, *"a real consumer used the SDK and told us where the contract
  was wrong,"* becomes payable.
- **It converts the north star into small victories.** Multi-room dungeons and
  the dungeon builder need rpg-api on the new stack anyway. After cutover,
  every toolkit capability lands as a visible increment in the running game
  instead of accumulating dark.

The original precondition (free roam *and* combat through the new stack, no
adapter to the old encounter) was met at W4's close; only the short-lived
parity refinement said wait.

## Why a new proto surface

The old `dnd5e.api.v1alpha2.encounter.EncounterService` speaks the old
toolkit's language: 22 event types through a 1905-line `translate.go`, and
RPCs (`SubmitCheck`, `SetReactionReady`, `ActivateFeature`, `Interact`) the
new stack does not have yet. Reimplementing behind it would mean translating
the new stream back into the old vocabulary and stubbing unimplementable
RPCs — the wrapper-dictates-the-contract adapter the SDK build already
refused, relocated into the proto layer.

Kirk: *"new protos clean contract for our new way of doing things."* The new
implementation is thin by construction — read inputs, hand to the session
manager — because the SDK's own seam laws (S2: no inner type crosses;
audience-projected events) leave rpg-api nothing to compute.

Per the proto-versioning trigger in `rpg-project/CLAUDE.md`, carrying two
packages costs dual maintenance. Accepted deliberately here: these are two
different contracts, not two versions of one, and the old package has a
scheduled death at cutover.

## Options considered

**Integration shape:**
1. Reimplement behind the old protos — rejected: adapter dictates the
   contract; must stub RPCs the stack can't honor.
2. New protos, rip the old path out first — rejected: leaves no playable
   route while the web catches up, and loses side-by-side comparison.
3. **New protos, build beside, cutover = the rip-out — chosen** (Kirk:
   "maybe that is the rip out the old and install the new happens").

**Creation seam:**
1. Expose `StartSession`/`Spawn` on the new service — rejected for v1:
   surface no game client needs; the lobby already owns the create moment.
2. **Lobby re-points internally — chosen.** `StartEncounter` keeps its proto
   shape and calls the session manager in-process. Lobby protos untouched.

**Stream payloads:**
1. rpg-api decodes each event payload into typed per-kind proto messages —
   rejected: re-encoding grows a translate.go back, and the SDK already owns
   the payload shapes.
2. **Bytes passthrough — chosen** (see design MUST-3/MUST-4). If payload
   typing becomes a client pain, the fix is SDK-side (published payload
   schemas), never API-side re-encoding.

## Day-one honest capability list

Works: join/spawn at start, free-roam walk (sight forms the fight bubble),
traverse doors, turn/end-turn, character-attacks-monster, dissolve by
decision, declared endings, story/status/view/atlas reads, per-audience event
stream with story resync.

Not yet, deliberately: monster attacking back (arrives with the monster
behavior work), action economy (toolkit#1035), fights ending by defeat
(toolkit#1024), stealth/surprise (toolkit#1020). These are scheduled waves,
not bugs — do not re-file them.

## Shelves and supersessions

- **A session outlives any single encounter** — `StartSessionInput.Encounter`
  is caller-supplied for exactly that reason (tomb → town → quest). The proto
  surface inherits the shelf; nothing stocks it in v1.
- **`ideas/encounter/v1alpha2/design.md`'s North-Star Invariants remain
  design inputs** (correlation ids, economy-delta push, event-spine lessons)
  for the capability train — consult when those waves arrive.
- **Chapter 2 (board 13, "Combat Verbs" on the v1alpha2 route) is superseded
  by this initiative** — flagged for Kirk's confirmation on this PR rather
  than acted on unilaterally.

## Late-breaking census (2026-08-15, same day)

Hours after this triplet was drafted, the toolkit lane's wave-4 close-out
census surfaced a world-model fork (room-first composition vs the ratified
one-canvas dungeon-builder target) plus four adjacent gaps — no self-position
read (toolkit#933), inconsistent position shapes, stateless doors vs the
tomb's authored lock, and `Traverse`'s contingent existence. Design §0 now
gates the affected field shapes on Kirk's ruling; plan W1 carries the
preconditions, including the census's forcing case (the reference tomb runs
on the new stack, entrance → hall → tomb). The rest of the design is
un-gated and stands.

**Resolved same day:** Kirk ruled — rooms stay internal to the encounter,
the seam projects absolute geometry, the wire speaks one map. See design §0.
