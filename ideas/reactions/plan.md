# Reactions / #316 — plan (the HOW)

**Status:** Auto-approved per Kirk's plan ruling (2026-08-28): a plan is an
agent-handoff artifact; the design is what Kirk rules. All six survey rulings
(R1–R6) ruled 2026-08-30 — see [survey-2026-08-30.md](./survey-2026-08-30.md),
which is the current-reality source. If implementation reality diverges,
**stop and surface on design PR rpg-project#317** — never patch the
divergence locally. Each landed PR appends findings to `implementation.md`.

## Ground rules for implementing agents

- One in-flight PR per module; serial, each blocked on the tag below it; no
  `replace` directives. Toolkit module order: protos → dnd5e → encounter →
  resolution → session → rpg-api.
- Before claiming anything unreachable or unused, enumerate what REACHES it —
  callers AND constructors — and record the command behind each inventory
  (Phase 6 retro rule).
- Pins are mutation-proven: show the pin failing under the exact breakage it
  guards before trusting it.
- Deletion PRs carry suites-identical evidence (sorted PASS-set diff +
  counts) and skip the independent review; behavior-surface PRs get the
  substitute review round. Self-classify in the handoff; reclassify out loud
  if a deletion PR grows behavior.
- Conventional prefixes mint tags on merge (feat! = minor, fix/chore =
  patch). Never hand-tag. No draft PRs. Kirk merges; hand each PR to the
  director merge-ready.
- Do not resurrect anything in survey F9 (TurnManager family, MoveEntity,
  combat.WithRoom, findThreateningEntities, …). The pins listed there are
  law.

## The build, in order

**P0 · rpg-api-protos — the label.** `ReactionRef{ref, name}` message;
`Struck` and `Missed` each gain a `reaction` field (design proposed 12 and 7
— verify the numbers are actually free before using them). Additive; `buf
lint/format/breaking` + generate compiling is the whole evidence bar (no
hand-written tests in protos — Kirk 2026-08-22). Base: origin/main.

**P1 · toolkit `rulebooks/dnd5e` — clean the field the fold reads.**
- Delete `MovementChainEvent.ThreateningEntities` (R5) and its only writers
  (disengaging_test.go). Zero-caller evidence recorded.
- Fix the two stale doc paragraphs in `conditions/opportunity_attack.go`
  (`:54`, `:243`) naming the deleted `Encounter.MoveEntity` as the drainer —
  the drainer is `resolution/movement.go`'s collectTriggers.
- Pin `character.EndTurn`'s reaction-zeroing hazard: a source-reading test
  that fails the day something starts calling it (the design's "fifth
  break": zero callers is the only reason OA can survive a turn boundary).
- Classification: deletion + docs + pin — expect suites-identical.

**P2 · toolkit `encounter` — carry the label, open the seam.**
- `RecordInput` gains `Reaction *ReactionIdentity` (`{Ref, Name}`),
  persisted on the outcome, presence-checked the way `AttackIdentity` is,
  never validated for meaning (C1).
- **The `Mover` capability (R1):** interface beside `Striker`; required at
  construction with the same validation; a `RefusingMover` for
  construction-only paths (mirror `RefusingStriker`); `executeTurnIntent`'s
  `Move` case calls it per cell before/instead of bare `stepTo` — the Mover
  announces the step through resolution, then the encounter takes it.
  Breaking construction change: every `NewEncounter`/`LoadEncounter` site in
  session, resolution, and both workbenches updates when those modules bump.
  feat! — minor tag.
- Keep encounter bus-free and ctx-free on its verbs: the capability takes
  the same shape Striker does (`context.Background()` at the call site, same
  comment).

**P3 · toolkit `resolution` — bump + adapt.** Expected small: adopt the new
dnd5e/encounter tags, update workbench constructors for the required Mover
(`RefusingMover` where movement can't happen). `NewMovement` itself is
built; do NOT reshape it. If nothing else is forced, this is a pin-bump PR.

**P4 · toolkit `session` — wire both paths (the bulk).**
- `runWalk`: per cell, snapshot `scope.enc.ToData()` (mover still at From) →
  `Resolve(NewMovement{From, To, Reactions, Roller})` → discard `out.World`
  (striker never-adopt pattern — runWalk is mid-loop over the live enc) →
  `saveDirty` → record each reaction's beat with the new `Reaction` identity
  → THEN `scope.enc.Step(cell)`. Announce-before-step is the machine's
  contract (reach is checked against where the target still is).
- `moverSeam`: implements `encounter.Mover`, struct-holding-`*writeScope` on
  the strikerSeam pattern verbatim, so a monster's turn walks through the
  same `NewMovement`.
- ONE `ReactionAttacks` implementation answering "what does reactor X swing"
  for both kinds (players via the equipped-weapon path session already uses;
  monsters via their Actions, as strikerSeam reads them).
- **Economy at formation (R2, blocking):** fight formation loads, seeds, and
  saves every member's sheet; the lazy-ignition rule and its comment in
  `session/economy.go` are superseded, not contradicted in place.
- **Stop the walk when a reaction downs the mover (R6):** standing re-asked
  per step; revise the "a Move cannot down or revive anyone" comment in
  `move.go` — the invariant changed, the record follows same-day.
- Restore the Disengage E2E player-side (R4): a player with Disengaging
  walks away from an adjacent goblin untouched; without it, the goblin's
  scimitar connects. This is also the first test anywhere that proves the
  REAL OpportunityAttackCondition fires inside a real fold (survey F2's
  named gap) — via the real loader grant, not a hand-published chain.
- Cost stays whole-walk up front; `Cost` nil into `Resolve` (the walk is
  already priced). Per-cell metering, prone, difficult terrain: shelf (F8).

**P5 · rpg-api — project the label.** One field through `stepToProto`/
`setEventBody` in `convert.go`, plus the toolkit tag adoption that carries
it. **Flag before churning:** the pin jump is large (dnd5e v0.105.3 →
current, session v0.35.1 → v0.40.3+, spanning the whole game-context
rework). If the bump breaks more than the label projection, STOP and
surface the breakage list on #317 rather than absorbing it silently.
Base: origin/dev.

**P6 · rpg-dnd5e-web — FILE, don't build.** One wire-anchored issue under
journey #253: render the reaction label on Struck/Missed beats (and note
out-of-turn ordering). Todo, unassigned, references the proto field by name.

## Done when

Design's Done-when, as amended (player-side Disengage, R4; walk stops when
the mover drops, R6). Kirk walks the branch: wolf flees, fighter swings
automatically, log names it, reaction spent and refills on the fighter's
turn, two threateners swing in deterministic order, mid-walk rejection
persists nothing, and a new condition subscribing to `MovementChain` needs
no `runWalk` change to be heard.
