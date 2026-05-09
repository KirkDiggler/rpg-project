# Wave 2.10 — Death + Encounter Resolution (HP=0 removes entity, last hostile down ends encounter)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Each inner issue is its own dispatch; this plan is the wave-shape that ties them together.

**Wave goal:** Player attacks monster; monster HP hits 0; monster dies and is removed from initiative + visible state; once the last hostile is down, the encounter ends per viewer.

**Verified by playtest:** alice + bob connect to a freshly-created TURN_BASED encounter seeded with two goblins (`goblin-1`, `goblin-2`). On her turn, alice attacks `goblin-1` until HP hits 0. Both browsers receive `EntityDied{entity_id: goblin-1, killer: alice}` then `EntityRemoved{entity_id: goblin-1, reason: "destroyed"}`. The goblin disappears from the entities table on both tabs. Initiative cycles past the dead goblin's slot — alice ends turn, the next active actor is `goblin-2` (not the dead goblin), and the orchestrator dispatches NPCAct against `goblin-2`. After alice kills `goblin-2`, both browsers receive `EncounterEnded{reason: "all_hostiles_defeated"}` and surface some end-state indicator (mode flips to FREE_ROAM or the harness shows an "encounter ended" banner). With one goblin only the same shape applies — kill it and the encounter ends immediately.

**Depends on:**
- Wave 2.5 (per-viewer projection, broker, translator, v2 service registration) — shipped.
- Wave 2.6 (CreateEncounter, GetEncounter, snapshot replay) — shipped.
- Wave 2.7 (Interact RPC for unlocked doors) — shipped.
- Wave 2.8 (TURN_BASED, TakeAction/EndTurn, NPC dispatch loop, combat events) — shipped. Wave 2.10 extends the combat slice with the death + end-of-encounter terminal events. The HP-mutation path in `Encounter.TakeAction` (toolkit) and `Encounter.NPCAct` (toolkit) is where death detection hooks in.
- Wave 2.9 (Locked doors, AttemptUnlock, SubmitCheck, pending-prompt machinery) — shipping under rpg-project#35. Not strictly blocking 2.10 (orthogonal verb surface), but its `pat-v2-pending-prompt-state` discipline confirms the toolkit-as-source-of-truth pattern this wave reuses for "is the encounter over?" state.

## Why this wave exists

Wave 2.8's "out of scope" list explicitly deferred death:

> "Death save mechanics. If a monster's HP hits 0, emit `EntityDied` (existing proto event) and remove from initiative. If a player's HP hits 0, leave dying-state mechanics for Wave 2.10+."

Today (post-2.8), `Encounter.TakeAction` clamps `monster.HP` to 0 (`combat.go:200-202`) but does not emit a death event, does not remove the dead monster from `Initiative`, and does not advance state when a turn lands on a dead actor. The encounter has no terminal state — fights "end" only by the orchestrator stopping the loop, with no event the web can render. The proto contract has carried `EntityDied`, `EntityRemoved`, and `EncounterEnded` since v0.1.93 (`events.proto:111`, `:116`, `:181`) waiting for an emitter.

Wave 2.10 closes that gap: it makes HP=0 a meaningful state transition. A dead monster fires `EntityDied`, then `EntityRemoved`, then is gone from initiative + entities. When the last hostile dies, the encounter fires `EncounterEnded` and stops dispatching turns. This is the prerequisite for "boss room at the end" (north-star goal): without an end signal, the playtest can't transition between rooms.

## Why "after prompts, before combat depth"

The roadmap places Death + Encounter Resolution after prompts (2.9) and before combat depth (2.11) because:

- **After 2.9 (prompts).** Wave 2.9 finishes the contract surface that was forward-loaded in Phase 1 (every v2 RPC has at least one playable surface). Wave 2.10's terminal events are the *next* contract surface that's been waiting since v0.1.93 (`EntityDied`, `EntityRemoved`, `EncounterEnded`) — same "validate the forward-loaded contract" shape as 2.9 was for `InputRequired` / `SubmitCheck`. Sequenced together, these two waves close the v2 contract surface.
- **Before 2.11 (combat depth — reactions, OAs, multiattack).** Reactions and multiattack make individual fights *richer*, but they don't change whether a fight can end. Death + encounter resolution is what makes combat *playable from start to finish*. Without it, the playtest can never move past the first encounter; with it, every subsequent combat-depth feature (reactions, multiattack, more action types) is layered on a working terminal-state model rather than racing terminal-state plumbing into a combat-depth wave.
- **Before 2.12 (multi-room flow).** Multi-room flow needs "this room's fight is over, the door to the next room is now traversable." That door-state transition is a downstream consequence of `EncounterEnded` (or some equivalent room-cleared signal). Wave 2.10 is what gives 2.12 a clean predicate to gate room transitions on.
- **Before 2.13 (party scaling).** Party scaling is mostly a fixture/lobby concern; it doesn't affect terminal-state plumbing. But scaling to 4 in a fight that *can't end* would just amplify the playtest dead-end. Death + encounter resolution must come before "more players in a fight that ends."

## Architectural calls already made (do not relitigate)

- **Death + entity removal + encounter-end live in the toolkit.** Per the boundary rule (`CLAUDE.md`): toolkit knows what "dead" means. The encounter SDK is the single source of truth for which entities are alive and whether the encounter is over. The rpg-api orchestrator does NOT inspect `monster.HP` and decide; it calls a toolkit verb (or reads toolkit-emitted events) and reflects the toolkit's decisions. **Anything game-logic — what counts as dead, when an encounter ends, who's hostile to whom — lives in toolkit, not rpg-api.**
- **`EntityDied` is the canonical proto event for HP=0.** Per `events.proto:111`. Toolkit emits an internal death event; the rpg-api translator maps it to `EncounterEvent_EntityDied`. The `killer_entity_id` is optional (carried when known — e.g., for a TakeAction kill, the attacker; for environmental damage, omitted).
- **`EntityRemoved` is the visibility/state-removal event.** Per `events.proto:116`. Death and removal are intentionally two events: `EntityDied` is the *narrative* moment (animations, log lines, "killed by alice"); `EntityRemoved` is the *state* mutation (the entity is gone from the entities table on the wire). Wave 2.10 emits both — the web can render the death animation on `EntityDied` and remove the entity on `EntityRemoved`. For Wave 2.10 the two events fire back-to-back; future waves may delay `EntityRemoved` (corpses persist for a round, looting, etc.).
- **`EncounterEnded` ends the *encounter-side* lifecycle, not the *server-side* lifecycle.** The encounter persists in storage with `Mode = ModeFreeRoam` (or a new terminal mode) so a second connect still receives the snapshot showing the end state. The server doesn't tear down the encounter on `EncounterEnded` — that's the lobby's call. For Wave 2.10, the practical effect of `EncounterEnded` is: orchestrator stops dispatching NPC turns, future TakeAction/EndTurn against this encounter return `FailedPrecondition` ("encounter ended"), and `EncounterEnded` is the terminal stream event web renders.
- **Hostile-set is opinionated by the toolkit, not the orchestrator.** Wave 2.10's "all hostiles defeated" predicate is `len(data.Monsters) == 0` after dead-monster removal. Players are not hostile to other players; monsters are hostile to all players. Future waves may introduce factions / friendly NPCs, but Wave 2.10 ships the simple "monsters are hostile, nothing else is." This decision is encoded in the toolkit, not the orchestrator.
- **Player death is out of scope** (see Out of scope below). When a *player* hits HP=0, Wave 2.10 emits `EntityDied` for the player but does NOT emit `EntityRemoved` and does NOT count the player as gone from initiative. Player dying-state (death saves, downed-but-stable, etc.) is Wave 2.11+ territory. For Wave 2.10, a player at HP=0 is an unhandled state — the encounter continues; future waves model the dying-state machinery. The encounter-end predicate is monster-count-based, so all-players-down does NOT end the encounter in Wave 2.10 (TPK handling is also Wave 2.11+).

## Toolkit verification findings (read these before scoping the toolkit issue)

The toolkit-member dispatch should validate these against fresh reads of the pinned versions; this section is the starting position.

### Encounter SDK (`rpg-toolkit/encounter@v0.4.0`)

- **No `EntityDied` event exists.** `encounter/events/` carries `attack_resolved`, `condition_applied`, `damage_dealt`, `door_opened`, `entity_appeared`, `entity_disappeared`, `hex_revealed`, `mode_changed`, `move`, `turn_ended`, `turn_started`. Wave 2.10 adds at minimum: `EntityDiedEvent` (cause/narration), `EntityRemovedEvent` (state mutation), `EncounterEndedEvent` (terminal). All follow the existing sealed-type + `PerPlayer` per-viewer-projection pattern (see `events/damage_dealt.go` and `events/mode_changed.go` as templates).
- **No death detection in `TakeAction`.** `combat.go:198-203` mutates `monster.HP` and clamps to 0 after a hit but stops there. The SDK does NOT publish anything death-related, does NOT remove the monster from `Initiative`, does NOT mutate `data.Monsters`. Wave 2.10 extends `TakeAction`'s post-hit path: if `monster.HP == 0`, publish `EntityDiedEvent` (cause), publish `EntityRemovedEvent` (effect), remove from `data.Monsters`, splice out of `data.Initiative` (and adjust `ActiveIdx` if the dead actor was after the active position), then check the encounter-end predicate.
- **`EntityDisappeared` exists but is the wrong event for death.** The existing `entity_disappeared` event signals LoS-driven visibility loss (entity moved out of viewer's perception view). Death is a different concept: the entity is gone for everyone, not just one viewer. Use `EntityRemoved` for death-driven state removal so consumers can disambiguate.
- **`Encounter.NPCAct` is the symmetric path.** When an NPC's attack drops a player to HP=0, the same `EntityDiedEvent` should fire — but `EntityRemoved` MUST NOT fire for players in Wave 2.10 (per architectural calls above). The toolkit verb should know the difference; tagging entities by kind (PlayerData vs MonsterData) is sufficient. NPCAct path lives in `npc.go`; check `/home/kirk/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.4.0/npc.go` for the actual NPC turn flow and where damage-to-player happens.
- **No encounter-status / encounter-ended state on `data.Data`.** The `Data` struct (`data.go:17-36`) tracks `Mode`, `Initiative`, `ActiveIdx`, `Round`, `PendingPrompts` — nothing terminal. Wave 2.10 either:
  - **Option A:** Add `data.Status core.EncounterStatus` enum (`StatusUnspecified, StatusActive, StatusEnded`) — terminal state is its own field, doesn't conflict with mode.
  - **Option B:** Add a new `core.EncounterMode` value (`ModeEnded`) so encounter-end is just another mode. Cleaner because mode-gated verbs already check mode and can naturally reject when mode is `ModeEnded`.
  - **Recommendation:** Option B (new mode value `ModeEnded`). Reuses existing mode-gating in `combat.go:126-128`, doesn't introduce a parallel enum, fits the "mode is the lifecycle field" framing already in place. Toolkit-member confirms during dispatch.
- **Initiative removal needs `ActiveIdx` adjustment.** When splicing a dead actor out of `Initiative`, if the dead actor's index is `< ActiveIdx`, decrement `ActiveIdx` (we shifted everyone left). If `== ActiveIdx`, that's only valid when the dead actor IS the current actor (NPC just died on its own turn — edge case, probably can't happen since NPC dies from a player attack, not its own). If `> ActiveIdx`, no adjustment needed. EndTurn's wrap logic handles the case where Initiative shrinks. The toolkit-member should test this carefully — off-by-one bugs here are visible in playtest.
- **Encounter-end predicate.** After any death, check: `len(data.Monsters) == 0` (all hostiles defeated). If true, set mode to `ModeEnded`, clear `Initiative` + `ActiveIdx` + `Round`, publish `EncounterEndedEvent{reason: "all_hostiles_defeated"}` with all players as audience. Future waves may add other end conditions (boss-only kill ends encounter, time-out, fled, etc.); 2.10 ships the all-monsters-dead predicate.
- **No `Encounter.RemoveEntity` / `Encounter.KillEntity` verb exists today.** Wave 2.10 either (a) inlines the death+removal+end-check into `TakeAction` and `NPCAct`'s post-damage paths, or (b) factors a new private `encounter.killEntity(entityID)` helper that both call. Option (b) is cleaner; option (a) gets there sooner. Toolkit-member picks; either is fine.

### dnd5e rulebook

- **Out of scope for Wave 2.10.** No rulebook-level death machinery is needed — the encounter SDK's HP=0 detection is sufficient for "monster is dead, remove it." Future waves wiring death saves / dying-state for *players* will need rulebook integration (death save mechanics, stabilization, exhaustion). Wave 2.10 ships with the encounter SDK alone.

### Gaps — confirmed not blockers; file as toolkit followups if surfaced

- **No "killer attribution" beyond direct attacker.** `EntityDiedEvent` carries `killerID` from the attacker that landed the killing blow. Indirect kills (poison damage applied last round, environmental damage) aren't modeled in 2.10 because the only damage source is direct attacks. When future waves add DoT or environmental damage, killer attribution may need a chain (last-applier? source of the original effect?). File as a toolkit followup if the question surfaces during dispatch.
- **No body / corpse modeling.** Dead monster is gone; it doesn't leave a corpse hex, doesn't drop loot, can't be re-targeted. Loot is a future concern; corpse hexes are a future concern (could matter for cover / area effects). File as toolkit followups if needed.
- **NPC-on-NPC damage is not modeled.** Wave 2.10's NPC turn dispatch (`NPCAct`) only attacks players. If future waves introduce factions and let monsters fight each other, the death detection generalizes — but the encounter-end predicate would need to be smarter than "no monsters left" (could be "no hostiles left" with hostility per-faction). Out of scope for 2.10.

## Inner-work shape

### rpg-toolkit — death detection + entity removal + encounter-end + 3 new events + ModeEnded

This is the largest of the three slices. Sub-tasks:

1. **Add `core.ModeEnded` (recommended) or `data.Status` field.**
   - If `ModeEnded`: extend `core.EncounterMode` enum, update `String()`, document semantics ("encounter terminal — no further verbs accepted"). All combat verbs (`TakeAction`, `EndTurn`, `NPCAct`) check `mode == ModeEnded` and return a new sentinel `ErrEncounterEnded` (mapped by rpg-api to `FailedPrecondition`).
   - If `data.Status`: add field, default `StatusActive`, set to `StatusEnded` at encounter-end. Verbs check status instead of mode.
   - Toolkit-member picks Option A (Status) or Option B (ModeEnded) at dispatch time, documents choice in PR.

2. **Add 3 new events in `encounter/events/`:**
   - `EntityDiedEvent` (`damage_dealt.go` shape): `EntityID core.EntityID`, `KillerID core.EntityID` (empty when unknown), plus `PerPlayer map[PlayerID]EntityDiedSlice{Visible bool}`. Audience: all players who could see the dying entity OR the killer (LoS to either side, mirroring attack-event projection).
   - `EntityRemovedEvent`: `EntityID core.EntityID`, `Reason string` ("destroyed" for HP=0; "fled" / "transformed" reserved for future). Audience: all players (state mutation is global — even players who couldn't see the death need to know the entity is gone from the state).
   - `EncounterEndedEvent`: `Reason string` ("all_hostiles_defeated"; future: "fled", "negotiated"). Audience: all players in the encounter.
   - Each event implements `EncounterEvent` (sealed marker, `EncounterID`, `Sequence`, `Audience`). Each ships with `*Wire` round-trip JSON shape per existing convention (`damage_dealt.go:67-109`).

3. **Add `killEntity(monsterID core.EntityID, killerID core.EntityID)` private helper on Encounter.**
   - Validates `monsterID` exists in `data.Monsters`. (No-op or error if missing — already removed.)
   - Builds per-viewer projection for `EntityDiedEvent` (visibility derived from monster's last-known position + killer's position).
   - Publishes `EntityDiedEvent`.
   - Deletes from `data.Monsters`.
   - Splices out of `data.Initiative`; if removed-index `< ActiveIdx`, decrement `ActiveIdx`.
   - Publishes `EntityRemovedEvent{reason: "destroyed"}` with all players as audience.
   - Calls `checkEncounterEnd()` (new private helper).

4. **Add `checkEncounterEnd()` private helper on Encounter.**
   - If `len(data.Monsters) == 0`: set mode to `ModeEnded` (or `Status = StatusEnded`), clear `Initiative + ActiveIdx + Round`, publish `EncounterEndedEvent{reason: "all_hostiles_defeated"}` with all players as audience. Returns `true`.
   - Else returns `false`.

5. **Hook `killEntity` into the post-damage path in `TakeAction` (`combat.go:197-214`) and `NPCAct` (`npc.go` — locate post-damage publish).**
   - After publishing `DamageDealtEvent`, check if target HP == 0. If yes and target is a monster, call `killEntity(targetID, attackerID)`.
   - For NPCAct (NPC attacks player): if player HP == 0, publish `EntityDiedEvent` (player as entity, monster as killer), but do NOT remove the player from initiative and do NOT publish `EntityRemovedEvent`. Player dying-state is deferred. (Document in code comment + PR.)

6. **EndTurn handles the case where `ActiveIdx` lands on a dead-and-removed actor.** With `killEntity` decrementing `ActiveIdx` correctly, this should naturally work — but the toolkit-member should add a unit test exercising the "monster dies on alice's attack, alice ends turn, next active actor is the *next* monster (not the dead one)" case.

7. **Tests.**
   - Unit + integration tests in `rpg-toolkit/encounter/`. Extend `integration_test.go` with `TestSlice_MonsterDies`: two-player + one-monster fixture, alice attacks goblin enough times to kill (force a hit + max damage via test roller), asserts `EntityDiedEvent` + `EntityRemovedEvent` per viewer, asserts `data.Monsters` is empty, asserts `data.Mode == ModeEnded`, asserts `EncounterEndedEvent` published.
   - `TestSlice_OneOfTwoMonstersDies`: two-player + two-monster fixture, alice kills `goblin-1`, asserts `EntityDied` + `EntityRemoved` for goblin-1 only, asserts `data.Monsters` has only `goblin-2` left, asserts `data.Initiative` is correctly spliced (goblin-1 gone, goblin-2 still in), asserts `data.Mode == ModeTurnBased` (encounter NOT ended), asserts no `EncounterEndedEvent` published.
   - `TestSlice_PlayerDies`: NPCAct kills alice, asserts `EntityDiedEvent` published for alice with goblin as killer, asserts `data.Players[alice]` is still present, asserts no `EntityRemovedEvent` for alice, asserts encounter does not end.
   - Snapshot of post-end `data.Data` round-trips through `ToData`/`LoadFromData` (verify the new mode/status field serializes correctly).

### rpg-api — translator extensions + encounter-end response handling + integration test

1. **Translator extensions in `internal/handlers/dnd5e/v2/encounter/translate.go`:**
   - `*events.EntityDiedEvent` → `EncounterEvent_EntityDied` (`entity_id = EntityID`, `killer_entity_id = KillerID` if non-empty). Per-viewer `Visible: false` → `ErrViewerSawNothing` (drop on the floor).
   - `*events.EntityRemovedEvent` → `EncounterEvent_EntityRemoved` (`entity_id = EntityID`, `reason = Reason`). Always-broadcast (no per-viewer filtering — global state mutation).
   - `*events.EncounterEndedEvent` → `EncounterEvent_EncounterEnded` (`reason = Reason`). Always-broadcast.

2. **`TakeAction` / `EndTurn` handlers** — add the new sentinel error mapping:
   - `errors.Is(err, encounter.ErrEncounterEnded)` → `codes.FailedPrecondition` with message "encounter has ended".

3. **NPC dispatch loop** (`end_turn.go` — Wave 2.8 introduced this) needs a guard: after each `enc.NPCAct` / `enc.EndTurn` cycle, if the encounter has ended (mode == ModeEnded / status == StatusEnded), break out of the loop. The toolkit's killEntity / checkEncounterEnd may end the encounter mid-NPC-chain (e.g., an NPC's attack kills the last hostile? — won't happen in 2.10 because monsters don't attack monsters, but the guard is cheap insurance and forward-loads for factions).

4. **Snapshot delivery on connect** (Wave 2.6 path) — verify that `MakeSnapshot` correctly reflects an ended encounter when a player reconnects after `EncounterEnded`. Snapshot should show `mode = ENDED` (or equivalent) and the entities table should reflect the post-removal state. This is verification, not new code, *unless* the snapshot helper hardcodes assumptions that break for ended encounters. Toolkit-member confirms during integration test.

5. **Tests.**
   - Translator tests for each new event type (happy + viewer-saw-nothing for `EntityDied`; always-broadcast for `EntityRemoved` + `EncounterEnded`).
   - Handler test: `TakeAction` after `EncounterEnded` returns `FailedPrecondition`.
   - Integration test extending `EncounterV2IntegrationSuite`: two-player + one-goblin fixture; alice attacks goblin until HP=0 (using a deterministic roller / large damage dice); both streams receive `EntityDied`, `EntityRemoved`, `EncounterEnded` in order; subsequent `TakeAction` from alice returns `FailedPrecondition`.
   - Multi-monster integration test: two-player + two-goblin fixture; alice kills `goblin-1`, asserts `EntityDied` + `EntityRemoved` for goblin-1 only on both streams, no `EncounterEnded`; alice ends turn, NPC dispatch skips dead goblin and attacks alice with `goblin-2`; alice kills `goblin-2`, asserts `EncounterEnded` fires on both streams.

### rpg-dnd5e-web — entity-removal reducer + encounter-ended UI cue + dispatch + harness multi-monster fixture trigger

1. **Dispatch extensions in `encounterStream2Dispatch.ts`:** add `entityDied`, `entityRemoved`, `encounterEnded` cases. Each forwards to corresponding `onX` callback.

2. **Reducers in `useEncounterState`:**
   - `applyEntityDied(event)`: optional — could be a no-op for state purposes (the entity is still rendered until `EntityRemoved`). Useful for surfacing a transient "killed!" log entry / animation hook. Wave 2.10 keeps this minimal: log + emit a UI event for the harness to render a one-line message ("alice killed goblin-1").
   - `applyEntityRemoved(event)`: removes the entity from the entities table in local state. UI immediately reflects entity gone. Cover the case where a tab joins late and the snapshot reflects post-removal state — reducer must be idempotent against missing entity.
   - `applyEncounterEnded(event)`: sets `encounterStatus = "ended"` (or equivalent) in local state. UI conditionally renders an end-state indicator based on this field.

3. **Harness UI in `PlaytestHarness.tsx`:**
   - "Encounter ended" banner — rendered when `encounterStatus === "ended"`. Simple text (e.g., "Encounter ended: all hostiles defeated") with the reason from the event.
   - Disable the attack / end-turn buttons when `encounterStatus === "ended"` (web doesn't gate game state per `feedback_no_logic_in_web`, but disabling buttons that would 4xx is courtesy UX — the server will still reject if they're clicked, which is the source of truth).
   - Multi-monster fixture: extend `CreateEncounter`'s starter scenario to seed `goblin-1` AND `goblin-2` so the kill-one-monster vs. kill-encounter-ends paths can both be exercised. Document the choice in PR description.
   - Optional log line — small in-harness log of recent events ("alice killed goblin-1", "encounter ended"). Useful for playtest verification; future polish.

4. **Tests.**
   - Reducer tests for each new event type (entity removed clears entry; encounter-ended sets status; idempotency tests).
   - Hook + harness tests for "encounter-ended banner renders" + "buttons disable when encounter ended."
   - Dispatch tests for new event types.

### rpg-api / rpg-dnd5e-web seed scripts — multi-monster fixture (small)

The harness fixture currently seeds one goblin (Wave 2.8). Wave 2.10 needs the fixture to support two monsters so the multi-monster paths can be exercised. This is a small change in either rpg-api's `CreateEncounter` starter scenario OR the harness's create-encounter request. Likely a sub-task of the rpg-api or rpg-dnd5e-web issue, not a separate issue — toolkit/api/web triage decides where it lives during dispatch.

---

## Inner-issue list (to be filed when wave 2.10 kicks off — NOT filed in this PR)

Per Kirk's preference (file at wave kickoff, not at plan-draft time), these are listed for reference. The Wave 2.9 close-the-loop conversation files them when ready to dispatch.

- **TBD: 🎮 Tracker** (rpg-project): wave goal sentence + sign-off + contents checklist
- **TBD: rpg-toolkit**: encounter death detection (HP=0 → EntityDied) + entity removal from data.Monsters + initiative splice (with ActiveIdx adjustment) + encounter-end predicate + new mode (`ModeEnded`) or status field + 3 new events (EntityDied, EntityRemoved, EncounterEnded) + integration tests for one-monster-dies and last-monster-dies and player-dies cases. Tag a new release (e.g. `encounter@v0.5.0`).
- **TBD: rpg-api**: translator extensions (EntityDied, EntityRemoved, EncounterEnded) + sentinel-error mapping for `ErrEncounterEnded` → `FailedPrecondition` + NPC-dispatch-loop guard against ended encounters + integration test exercising kill-one-of-two, kill-last, post-end TakeAction rejected. Bump toolkit pin to the new release.
- **TBD: rpg-dnd5e-web**: dispatch + reducers (entityRemoved + encounterEnded) + harness encounter-ended banner + multi-monster fixture support + reducer/harness tests.
- **TBD: chore: Wave 2.10 close-the-loop** (rpg-project): file followups, update board, draft Wave 2.11 (combat depth — reactions / OAs / multiattack) plan, capture verified patterns into role context (death + encounter-end patterns).

(Optional, depending on dispatch decisions):
- **TBD: rpg-api or rpg-dnd5e-web seed-script update**: multi-monster starter scenario fixture. Folded into one of the above issues.

---

## Out of scope for Wave 2.10

- **Player death / dying-state mechanics.** Death saves, downed-but-stable, exhaustion, stabilization, healing-from-zero. When a player hits HP=0 in 2.10, `EntityDiedEvent` fires (so the web can surface it) but the player is NOT removed from initiative and the encounter does NOT auto-end. Full dying-state is Wave 2.11+.
- **TPK (total party kill) handling.** If all players die before all monsters die, the encounter does NOT auto-end in Wave 2.10. The encounter-end predicate is monster-count-based only. TPK as an explicit end condition with `EncounterEnded{reason: "tpk"}` is Wave 2.11+ territory (depends on player dying-state model).
- **Reactions and opportunity attacks.** Wave 2.11 territory. A monster moving past a player triggers nothing in 2.10.
- **Multiattack.** A player or monster takes one attack per turn. Wave 2.11.
- **Action types beyond "attack".** No spells, no Dodge / Disengage / Hide / Help. Wave 2.11+.
- **Rich condition effects.** `StatusApplied` fires (Wave 2.8 ships the event), but conditions aren't tracked for "are you incapacitated" / "are you stunned and can't act" gating. Wave 2.11+.
- **Death animations / VFX.** Web renders the entity as gone; no death animation orchestration. Visual polish ships later.
- **Loot / corpse / lootable-body modeling.** Dead monsters are gone. No loot drop, no corpse hex. Future content.
- **Multi-encounter chaining / room transitions.** `EncounterEnded` is a per-encounter terminal event. Wiring "encounter ended → unlock door to next room" is Wave 2.12 (multi-room flow).
- **`AttemptUnlock` / SubmitCheck on a dead monster** (e.g., locked chest is a "monster" in some sense). Out of scope; unrelated.
- **LobbyView migration.** Wave 5. Harness is the sole UI surface for encounter-end in Wave 2.10.

---

## Forward-loaded patterns (anticipated — refine in close-the-loop)

These are the new patterns Wave 2.10's execution is expected to surface. Marked `status: anticipated` until the wave's close-the-loop step verifies them against shipped reality.

**rpg-api-member additions:**

- `pat-v2-terminal-encounter-state` *(anticipated)* — Encounter-end is a server-side terminal state held by the toolkit (`mode == ModeEnded` or `status == StatusEnded`, decision made during dispatch). Verbs that mutate the encounter check terminal state first and return `FailedPrecondition` with `ErrEncounterEnded`. `EncounterEndedEvent` is published once when the predicate first goes true; subsequent inspections of the encounter reflect terminal state (and snapshot replay shows it for late-joiners).
- `pat-v2-cause-effect-death-events` *(anticipated)* — Death is two events on the wire: `EntityDied` (cause / narrative — animations, log lines, killer attribution) and `EntityRemoved` (effect / state mutation — entity gone from world). Same cause-effect split as `AttackResolved` + `DamageDealt` from Wave 2.8. Wave 2.10 fires them back-to-back; future waves may delay `EntityRemoved` for corpse / loot mechanics. The web handles them independently — `EntityRemoved` is the canonical "drop entity from local state" event; `EntityDied` is optional-render UX.
- `pat-v2-translator-broadcast-event` *(anticipated)* — Some translated events (`EntityRemoved`, `EncounterEnded`) are global state mutations and bypass the per-viewer projection — they always broadcast to all players in the encounter. The translator's per-viewer code path either skips projection or always returns "visible: true" for these. Distinct from per-viewer events (`EntityDamaged`, `Move`) which derive audience from LoS.

**rpg-dnd5e-web-member additions:**

- `pat-v2-entity-removal-reducer` *(anticipated)* — `EntityRemoved` is the event the entities-table reducer drops the entry on. Reducer is idempotent (no-op if the entity is already missing — handles snapshot-after-removal). `EntityDied` is optional-render (transient log line / animation hook), not a state-mutation reducer.
- `pat-v2-encounter-status-reducer` *(anticipated)* — `useEncounterState` exposes `encounterStatus: "active" | "ended"` set by `EncounterEnded` event handler. UI conditionally renders end-state indicators (banners, button-disabled hints) based on this field. Snapshot replay sets it on connect when joining an already-ended encounter.

**rpg-toolkit-member additions:**

- `pat-encounter-death-detection` *(anticipated)* — HP-mutating verbs (`TakeAction`, `NPCAct`) check post-mutation HP and call a private `killEntity(id, killerID)` helper when an entity's HP hits 0. The helper publishes `EntityDied` (per-viewer LoS projection) + deletes from the data map + splices initiative (with `ActiveIdx` adjustment) + publishes `EntityRemoved` (broadcast) + calls `checkEncounterEnd()`. Same encapsulation pattern as `OpenDoor` (Wave 2.7) — a single private helper owns the multi-step state mutation, callers don't reimplement.
- `pat-encounter-terminal-mode` *(anticipated)* — Encounter terminal state is a new mode value (`ModeEnded`) — or alternatively a `Status` field; toolkit-member picks at dispatch. `ModeEnded` reuses the existing mode-gating in combat verbs; verbs return `ErrEncounterEnded` (sentinel) when called after end. Mode change to `ModeEnded` clears `Initiative + ActiveIdx + Round`. Recommendation is `ModeEnded` for parsimony; document choice in PR.
- `pat-encounter-end-predicate` *(anticipated)* — Encounter-end predicate is `len(data.Monsters) == 0` for Wave 2.10. Encapsulated in `checkEncounterEnd()` so future waves swap the predicate (boss-only kill, fled-from, negotiated peace, time-out) without touching the kill path. Predicate evaluation is a pure function over `data.Data`; publish-side effects only happen when the predicate first goes true.

After execution, the close-the-loop dispatch promotes verified `anticipated` entries to status-less (validated) and adds any new patterns that surfaced.

---

## Execution sequence

1. **Wave 2.9 close-the-loop** must complete before 2.10 dispatches. That step writes verified Wave 2.9 patterns into role context — Wave 2.10's dispatches read those.
2. **rpg-toolkit issue dispatches first.** This is the longest critical-path slice (3 new events, mode/status field, kill helper, end predicate, ActiveIdx splice math, integration tests). Tag a release (e.g. `encounter@v0.5.0`).
3. **rpg-api issue dispatches second.** Bumps the toolkit pin, adds the 3 translator entries, the sentinel-error mapping, the NPC-loop terminal-state guard, and integration tests exercising kill-one-of-two + kill-last + post-end-rejected.
4. **rpg-dnd5e-web issue dispatches third.** Adds dispatch + reducers + harness banner + multi-monster fixture support. Builds against the merged rpg-api version.
5. **End-to-end playtest** (cold start, both browsers, 2-tab pattern from `v2-playtest-harness`):
   - alice and bob connect to a freshly-created TURN_BASED encounter with two goblins.
   - alice attacks `goblin-1` until HP=0 (may take multiple turns; bob can pass / move while waiting). Both browsers receive `EntityDied{goblin-1}` + `EntityRemoved{goblin-1}`. Goblin-1 disappears from both entities tables.
   - alice ends turn; NPC dispatch finds the next active actor is `goblin-2` (NOT the dead goblin); `goblin-2` takes its turn (attacks alice), both browsers receive normal damage events. Initiative continues correctly with the dead goblin gone.
   - alice kills `goblin-2`. Both browsers receive `EntityDied`, `EntityRemoved`, then `EncounterEnded{reason: "all_hostiles_defeated"}`. Both browsers show the end-state indicator. Buttons are disabled (or just visibly noisy on click).
   - alice attempts another `TakeAction`; server returns `FailedPrecondition` with the encounter-ended message.
6. **chore: Wave 2.10 close-the-loop** dispatches: files followups, updates board, drafts Wave 2.11 (combat depth) plan, captures verified patterns into role context, retros on tracker.

---

## Reference

- Wave 2.10 tracker: rpg-project#<filled in by close-the-loop dispatch>
- rpg-toolkit issue: rpg-toolkit#<filled in by close-the-loop dispatch>
- rpg-api issue: rpg-api#<filled in by close-the-loop dispatch>
- rpg-dnd5e-web issue: rpg-dnd5e-web#<filled in by close-the-loop dispatch>
- Master plan: `rpg-project/ideas/encounter/v1alpha2/plan.md`
- Roadmap: `rpg-project/ideas/encounter/v1alpha2/roadmap.md`
- Design: `rpg-project/ideas/encounter/v1alpha2/design.md`
- Wave 2.8 plan (combat slice — closest predecessor for verb shape + NPC dispatch + ActiveIdx mechanics): `rpg-project/ideas/encounter/v1alpha2/plans/06-wave-2.8-combat-slice.md`
- Wave 2.9 plan (prompts — most recent template + cause/effect-event-split discipline): `rpg-project/ideas/encounter/v1alpha2/plans/07-wave-2.9-prompts.md`
- Proto contract:
  - `dnd5e/api/v1alpha2/encounter/events.proto:111-114` — `EntityDied{entity_id, optional killer_entity_id}`
  - `dnd5e/api/v1alpha2/encounter/events.proto:116-119` — `EntityRemoved{entity_id, reason}`
  - `dnd5e/api/v1alpha2/encounter/events.proto:181-183` — `EncounterEnded{reason}`
  - `dnd5e/api/v1alpha2/encounter/events.proto:32-33, 49` — oneof field-number wiring (entity_died = 23, entity_removed = 24, encounter_ended = 44)
- Toolkit references (verify pinned versions at dispatch):
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.4.0/data.go` — `Data` shape (no terminal-state field; needs ModeEnded or Status)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.4.0/combat.go:197-214` — `TakeAction` post-damage path (where killEntity hooks in)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.4.0/combat.go:115-156` — `EndTurn` (Initiative + ActiveIdx semantics; splice math reference)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.4.0/npc.go` — NPCAct (where player-HP-hits-zero handling also needs to fire EntityDied without removing)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.4.0/events/damage_dealt.go` — event template (sealed type + PerPlayer + Wire round-trip)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.4.0/events/mode_changed.go` — broadcast-event template
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.4.0/core/mode.go` — `EncounterMode` enum (where `ModeEnded` would be added)
- Board: https://github.com/users/KirkDiggler/projects/11

## Sign-off criterion

Wave 2.10 is done when, in a cold-start two-browser playtest:

- alice and bob connect to a freshly-created TURN_BASED encounter seeded with two goblins
- alice attacks `goblin-1` until HP=0; both browsers receive `EntityDied` + `EntityRemoved`; goblin-1 disappears from both entities tables
- alice ends turn; NPC dispatch correctly cycles to `goblin-2` (not the dead goblin); `goblin-2` takes its turn; both browsers see the turn cycle proceed normally
- alice kills `goblin-2`; both browsers receive `EntityDied` + `EntityRemoved` + `EncounterEnded`; both browsers show an encounter-ended indicator
- subsequent attempts to `TakeAction` against the ended encounter return `FailedPrecondition`
- a single-goblin variant (kill the lone monster) ends the encounter immediately on that kill — no soft-block waiting for additional verbs
- a late-joining tab (open a third browser after the encounter ended) sees the snapshot reflect the ended state and the entities table reflect the post-removal world

When the close-the-loop runs, the playtest video / screenshots become the verification artifact and the wave ships.
