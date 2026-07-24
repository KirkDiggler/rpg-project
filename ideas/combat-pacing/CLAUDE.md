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
  envelope/state seam, but no Platform issue gets filed until Kirk reviews the
  concept's `CONTRACT.md` and confirms a concrete requirement. Assets is
  consultation-only unless a future round chooses a 3D die or new art.

## Key facts (don't re-derive)
- **Not a fixed triple.** Real event shapes vary: a declared strike can produce
  `ActionResolved` → `AttackResolved` → `EntityDamaged` (hit only); an opportunity
  attack can produce `AttackResolved` (+ optional `EntityDamaged`) with **no**
  `ActionResolved`; a non-attack `ActionResolved` (dodge, dash, potion, etc.) can have
  no attack event; status events aren't guaranteed to correlate; and there's no
  documented, universal completion marker for a correlation group — only an
  actor-only, TakeAction-only `TurnStateChanged` that can incidentally close a
  declared strike's group, saying nothing to spectators or opportunity attacks.
  Correlation/cardinality/completeness are contract questions for round one's
  `CONTRACT.md` to verify, not facts to assume.
- **Invariant:** authoritative HP/economy/turn state applies immediately on receipt.
  Choreography delays presentation only — never state.
- Whenever a roll-bearing event does arrive, it's already resolved before the wire —
  **no roll-started event** → suspense is client-side theater over a decided result.
- **Live metadata delivery needs two confirmed, parallel Platform prerequisites.**
  `encounterStreamDispatch.ts` passes only `payload.value`, discarding the envelope
  (`correlation_id`, `sequence`, `timestamp`). Round one logged this as a candidate, not
  a blocker, because round one is fixture-first (no live stream). It became relevant the
  moment a later round (rpg-dnd5e-web#581, promoting this concept into the live game)
  needed it: Kirk reviewed the candidate and confirmed the client-only callback contract,
  filed as **rpg-dnd5e-web#582** (Board 19, Team=Platform). Root-cause verification then
  confirmed that `rpg-api` also drops toolkit-authored `CorrelationID()`/`OccurredAt()`
  when translating `DamageDealtEvent`/`ConditionAppliedEvent` into `EntityDamaged`/
  `StatusApplied`; that distinct API-only correction is **rpg-api#701**. #701 and #582
  may land in parallel and are independently landable, but **both are required by #581**
  before live attack-effect grouping. The approved split contract lives in `design.md` §9:
  #701 projects effect metadata onto envelopes; #582 passes envelope metadata verbatim to
  typed callbacks.
- `CONTRACT.md` lifecycle (per PR #557): records evidence/observations/candidate gaps
  during the concept; Platform requests get filed only after Kirk reviews the concept
  and confirms a candidate as a real, scoped need — never pre-authored.

## State
See `design.md` — beat model, dice options (round one compares token-anchored vs
center-stage placement), pacing knobs, contract questions and their CONTRACT.md
lifecycle, the round-one `/concepts` scope following the PR #557 fixture-first pattern,
decided items, what's still open beyond round one, and (§9) the approved two-prerequisite
metadata delivery contract confirmed by Kirk's review: rpg-api#701 plus rpg-dnd5e-web#582,
both required before rpg-dnd5e-web#581 live grouping.

See `plan.md` — the approved design's round-one `rpg-dnd5e-web` implementation
plan: 5 TDD tasks (fixtures → `useBeatSequencer` → `BeatStage` →
`CombatPacingConcept` wiring → CONTRACT.md/docs/visual-verification/PR),
exact file map, and the file's own self-review.
</content>
