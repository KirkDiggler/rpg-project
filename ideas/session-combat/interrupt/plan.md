# Rung 3 — the player chooses: build plan

**Date:** 2026-09-06 · **Design:** `design.md` (main 38e65c84) · **Status:** building
**Surveyed at:** toolkit main `72e76efd`, protos main `2370a42`, rpg-api dev `5808a74`, web dev `be81476c`

Rungs 1 and 2 are on main and walked (encounter v0.62.0, resolution v0.33.1, session
v0.64.0, rpg-api#929, web#956). The opportunity attack fires automatically both ways.
Only the pause is missing. The surveys found four places where the design's mechanism
does not fit the code as it stands; each gets a ruling here.

## Rulings that change the design

**R1 — The reaction is spent when it is TAKEN, not when it is offered.** Today
`conditions/opportunity_attack.go:353-366` sets `UsedThisTurn` and bills the economy the
moment the trigger publishes, before the machine asks `AttackFor`. So a friend walking
past a fighter already costs the fighter's reaction (the hostility gate in
`session/mover.go:317-342` answers false too late), and a posed player who chooses
`hold` would have paid for a swing they refused. Fix the primitive: the movement machine
publishes a `ReactionTakenEvent{Reactor, Ref, Payload}` after it runs a reaction strike,
and the condition spends on that event. A trigger that nobody takes costs nothing. This
is the ally-spend bug fixed on its own merits and the thing that makes `hold` free.

**R2 — The encounter owns the paused turn.** The design threads a "remainder" out of
the drive to the session verb and back. The drive is three loops deep with five entry
points (`clocks.go:1604`, `standing.go:225`, `:1142`, `:1305`, `:1457`) and the remainder
is a dozen locals (`clocks.go:728-799`, incl. the anti-spin bound at `:478` and the
`MovementFeet` arithmetic at `:799`). The turn is the encounter's; its pause is encounter
state. `EncounterData` gains `PausedTurn`; every drive entry refuses while paused; a new
verb `ResumeTurn` continues from the announced step. The session persists only the
ledger. Restart between pose and answer: both aggregates carry their half, written
encounter-then-session as today.

**R3 — Every player reactor of one step is asked at once.** The design said one window
at a time, serially. Serial needs "who already held for this step" memory and a
re-announce; the ledger already holds several windows (`interrupt/ledger.go:111`). So
the seam poses one window per player reactor for the step, the freeze lasts until all
are answered, the last answer resumes. If a strike drops the mover, the session answers
the remaining windows `hold` on their audience's behalf (the custodian may: `By ==
Audience`) and the turn ends in the leaving cell (R6).

**R4 — The window opens only inside a monster's turn.** Player mover with a player
reactor (design row two) has no producer: players are never hostile to each other in
any shipped dungeon, and the rung-2 ally test proves the seam refuses them. Row two goes
to the shelf. A player walking past monsters stays automatic (rung 2).

## The shape, module by module (bottom-up)

### 1. `rulebooks/dnd5e` (root module) — spend on taken
- `events`: `ReactionTakenTopic` / `ReactionTakenEvent{ReactorID, Ref *core.Ref,
  Payload}`.
- `conditions/opportunity_attack.go`: `onMovementChain` publishes the trigger and stops
  there (drop `:353-366`); subscribe `ReactionTakenTopic` for own ref+member → set
  `UsedThisTurn`, `publishSpendRequested`. `UsedThisTurn` still gates the trigger (`:307`).
- Tests: a trigger nobody takes leaves `UsedThisTurn` false and the economy untouched; a
  taken one spends once. Minor tag.

### 2. `resolution` — publish taken
- `movement.go`: after a reaction strike sub-machine runs for a reactor (AttackFor
  true), publish `ReactionTakenEvent` on the interaction bus. Re-pin root. Tests: the
  ally case in the movement suite now shows no spend. Patch/minor tag.

### 3. `encounter` — pause and resume
- Sentinel `ErrStepPaused` + detail `StepPausedError{Windows []PausedWindow{Audience
  MemberID, Reaction ReactionIdentity}}` for a `Mover` to return from `Move`.
- `executeTurnIntent` Move case (`clocks.go:732-799`): on `ErrStepPaused` from
  `e.mover.Move`, store `PausedTurn{Member, Round, From, To, Remaining []Position,
  Moved int, Budget TurnBudget, Intents j, IntelDeltas, At, Audience}` in the aggregate
  (persisted in `EncounterData`; `MovementFeet` charged for cells already walked),
  append a `window_opened` beat (audience: everyone, pre-v1 full data; body: the
  windows' audiences, mover, from, to, reaction), and return "turn not over, paused"
  up through `driveOneMonsterTurn` → `driveTurnsWithParticipation` → each caller
  (each returns as it does when the drive stops at a player). `EndTurnOutput` (and the
  form/transfer/exit outputs) gain `Paused bool`.
- Guard: every drive entry returns immediately while `PausedTurn != nil`.
- `ResumeTurn(ctx) (*ResumeTurnOutput, error)`: refuses unless paused; asks standing
  first (mover dropped → turn over in the leaving cell, R6); else `stepTo` the announced
  cell WITHOUT re-announcing, movement beat, then continues the loop over `Remaining`
  (announcing each), finishes the turn's intents from stored `j` within the stored
  bound, `bubble.End`, then drives on to the next player as `EndTurn` does. Clears
  `PausedTurn` before the first step.
- Tests in `monsterturn_test.go` style with a `pausingMover`: pause at cell i persists
  and reloads (`ToData` round trip); resume finishes the walk and the turn; a dropped
  mover on resume ends in the leaving cell; a spinning driver cannot exceed the bound
  across a pause; drive entries refuse while paused. Minor tag.

### 4. `session` — ledger, freeze, pose, React, Afford, events
- Re-add `play/interrupt` to `go.mod`. `SessionData.Windows interrupt.LedgerData`
  (the tombstone at `data.go:43-48`). Ledger loaded in `openForWrite`; reject-never-crash
  on a bad stored ledger (reference `b14b8aa6^:write.go`).
- `moverSeam.Move`: when the MOVER is a monster, `AttackFor` answers false for a
  character reactor (`sheets.character != nil` at `mover.go:335`) that passes the
  hostility+melee gates and remembers it as *asked*. After `Resolve`, if any were asked:
  pose one window each (`Options: strike, hold`, `Payload: {mover, from, to, reactor,
  reaction ref, definition}`, `At: seq`), mark `scope.touched`, return
  `encounter.StepPausedError`. Monster reactors and player movers: rung-2 behaviour.
- Freeze: restore the two-opener split (`openForChange` refuses with `ErrWindowOpen`
  while `ledger.Open()` is non-empty; `React` uses `openForWrite`). Classification: every
  change verb in the 19-site table is `openForChange` except React. Reads exempt.
- `React(ctx, *ReactInput{Session, Member, DeclarationID, Choice}) (*ReactOutput{Saved,
  Delivery})`, `Choice` is `ReactChoice` string const `ReactStrike`/`ReactHold`.
  `strike`: a second `resolution.Resolve(NewMovement)` for the frozen from→to with a
  `reactionAttacks` that answers ONLY this reactor (the condition re-triggers because
  R1 left it unspent; Disengage still folds); record the struck/missed beat with its
  `Reaction`. `hold`: nothing. Then `ledger.Answer`; if no window remains open →
  `enc.ResumeTurn`. If the mover is down after a strike, answer the rest `hold` for
  their audiences, then resume (which ends the turn). Standing/sheet handling as `Move`.
- `Afford`: `VerbReact` declaration for a member with an open window, `Slot:
  SlotReaction`, `TargetKind: TargetMember`, candidates = the mover, `Reaction
  *ReactionRef{Ref, Name}`, ID encodes the window (`declaration_id.go:165` sealed byte
  set, `offers.go:869` rank). It bypasses the not-your-turn and world-clock early returns
  (`afford.go:339-360`). While any window is open, every other verb for every member is
  unavailable with the new `ShortfallWindowOpen`.
- Events: `EventWindowOpened` + body `{Audiences, Mover, From, To, Reaction}`; and the
  standing gap: `StruckBody`/`MissedBody` gain `Reaction *ReactionRef` from
  `Outcome.Reaction` (the wire field has been dead since protos#258).
- Sentinels: `ErrWindowOpen`, `ErrNoWindow`, `ErrNotAudience`, `ErrNotOffered` (names
  from the tombstone at `errors.go:511-515`).
- Tests (`mover_test.go` suite style): the done-when scene — two skeletons pass the
  fighter; first pass poses, `hold` → the skeleton finishes its walk; second pass poses,
  `strike` → beat carries the OA reaction, the second skeleton's turn finishes from
  where it stopped; the fighter's reaction is spent once; the monk with Step of the Wind
  passes untouched and no window opens; a Move during the freeze is `ErrWindowOpen`;
  Afford shows REACT to the fighter and WindowOpen shortfalls to everyone; a save/load
  between pose and answer round-trips both aggregates. Minor tag.

### 5. protos (parallel; merge once the session branch compiles the verb)
`VERB_REACT = 6`; `SHORTFALL_REASON_WINDOW_OPEN = 8`; `Declaration.reaction ReactionRef
= 14`; `enum ReactChoice {UNSPECIFIED, STRIKE, HOLD}`; `rpc React(ReactRequest{session,
member, declaration_id, choice}) returns (ReactResponse{saved, delivery})` shaped on
Activate; `EVENT_KIND_WINDOW_OPENED = 25`; `WindowOpened window_opened = 31`
`{repeated string audience, string mover, Position from, Position to, ReactionRef
reaction}`. Additive; buf lint/format/breaking green.

### 6. rpg-api (dev)
`react.go` handler on the Activate template; converters: `verbToProto`,
`shortfallReasonToProto`, `eventKindToProto` + `setEventBody` (WindowOpened), `Struck`/
`Missed` copy `Reaction`; Declaration copies `Reaction`; `statusError` maps
`ErrWindowOpen` → FailedPrecondition, `ErrNoWindow`/`ErrNotOffered` → FailedPrecondition,
`ErrNotAudience` → PermissionDenied; `ownership_test.go` rows; `Manager` interface line;
registration. Persistence untouched (opaque blobs, `redis_repos.go:28-30`). Pins bumped
to the four tags.

### 7. web (dev)
Pin bump; `src/api/useSessionReact.ts` on the attack hook's shape; `ActionDock.tsx`: a
REACT declaration is drawn ahead of the "Synchronizing" (`:319`) and "Watching" (`:335`)
returns, as two buttons Strike / Hold, and the gates at `useSessionCombatExperience.ts:351,
484` let it through; `story.ts` `windowOpened` arm with a test; `sessionRefreshKeys.ts`
row; `SessionCanvas.tsx` rings the mover (a `PathPreview` single cell beside the
attackable rings at `:543`) while the viewer holds a window.

## Order and integration
Root → resolution → encounter → session (each re-pinned to the tag below it, or to the
pushed pseudo-version while the walk runs) → protos → rpg-api → web. Builders for 1+2,
3, and 5 run in parallel now; 4 starts when 1–3 are pushed and pins their pseudo-versions
(`GOPROXY=direct GOFLAGS=-mod=mod go get <module>@<commit>`); 6 and 7 follow 4 and 5.
One PR per module, opened ready. The whole stack is walked once on a local env from
pushed branches before the toolkit PRs are merged.

## Done-when (from the design, unchanged)
Two skeletons walk past the fighter on their turns: the first pass opens a window, the
fighter holds, the skeleton continues; the second opens a window, the fighter strikes,
the log shows the opportunity attack, the skeleton's turn finishes from where it stopped.
The monk on Step of the Wind passes untouched and is never asked. A restart of rpg-api
between pose and answer changes nothing.

## Shelf (added by this plan)
- Row two (player mover, player reactor): no producer until players can be hostile.
- A timer; several windows across several steps; the option list on the wire (the verb
  implies strike/hold until a second reaction needs a third option).
- Encounter refusing its own change verbs while paused (the session freeze covers it).
