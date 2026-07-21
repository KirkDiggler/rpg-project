# Idea: Combat Pacing & Dice

The attack loop ("swing, miss, end turn") is mechanically correct but emotionally flat.
Give it *beats*: choreograph the reveal of an already-resolved server roll into named,
timed steps (Cue → Throw → Verdict → Impact → Release), with dice as a real on-screen
object and crits/nat-1s that break the frame. Suspense is theatrical — the server has
already rolled, no player has seen it, so a paced reveal reads as genuine suspense to
the whole co-op table.

## Ownership & routing
- **Primary:** rpg-dnd5e-web#561 (Board 19, Feature=Game Screen, Team=UI/UX). Broader
  charter: rpg-dnd5e-web#525 (Game-UX).
- UI/UX owns round-one concept + presentation. Platform owns any eventual stream/
  envelope/state seam, but no Platform issue gets filed until the concept's
  `CONTRACT.md` names a concrete requirement. Assets is consultation-only unless a
  future round chooses a 3D die or new art.

## Key facts (don't re-derive)
- **Not a fixed triple.** Real event shapes vary: a declared strike can produce
  `ActionResolved` → `AttackResolved` → `EntityDamaged` (hit only); an opportunity
  attack can produce `AttackResolved` (+ optional `EntityDamaged`) with **no**
  `ActionResolved`; a non-attack `ActionResolved` can have no attack event; status
  events aren't guaranteed to correlate; correlation groups have no terminal marker.
  Correlation/cardinality/completeness are contract questions for round one's
  `CONTRACT.md`, not facts to assume.
- **Invariant:** authoritative HP/economy/turn state applies immediately on receipt.
  Choreography delays presentation only — never state.
- Whenever a roll-bearing event does arrive, it's already resolved before the wire —
  **no roll-started event** → suspense is client-side theater over a decided result.
- Known gap, not a round-one blocker: `encounterStreamDispatch.ts` passes only
  `payload.value`, discarding the envelope (`correlation_id`, `sequence`, `timestamp`).
  Round one is fixture-first (no live stream), so this only matters once a later round
  promotes fixtures to a real reassembler — log it in `CONTRACT.md`, don't fix it now.

## State
See `design.md` — beat model, dice options (round one compares token-anchored vs
center-stage placement), pacing knobs, contract questions (not asks), the round-one
`/concepts` scope following the PR #557 fixture-first pattern, decided items, and what's
still open beyond round one.
</content>
