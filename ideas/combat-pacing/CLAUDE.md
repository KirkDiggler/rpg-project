# Idea: Combat Pacing & Dice

The attack loop ("swing, miss, end turn") is mechanically correct but emotionally flat.
Give it *beats*: choreograph the reveal of an already-resolved server roll into named,
timed steps (wind-up → roll → verdict → impact), with dice as a real on-screen object and
crits/nat-1s that break the frame. Suspense is theatrical — the server has already rolled,
no player has seen it, so a paced reveal reads as genuine suspense to the whole co-op table.

## Charter
Game-UX (rpg-dnd5e-web#525). Not yet its own issue — this is ideation.

## Key facts (don't re-derive)
- The attack story is a correlated triple sharing `EncounterEvent.correlation_id`:
  `ActionResolved` → `AttackResolved` (fires on MISS too) → `EntityDamaged` (hit only).
- All resolved before the wire; **no roll-started event** → suspense is client-side theater.
- Blocker to fix first: `encounterStreamDispatch.ts` passes only `payload.value`, discarding
  the envelope (`correlation_id`, `sequence`, `timestamp`) the reassembly needs. Client-side
  fix, no proto change.

## State
See `design.md` — beat model, dice options (rec: 2D die over the acting token), pacing knobs,
wire asks, `/concepts` "Dice & Pacing" prototype plan, and open questions for Kirk.
</content>
