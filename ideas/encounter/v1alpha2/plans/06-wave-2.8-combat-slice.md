# Wave 2.8 — Combat slice (Player attacks monster, monster attacks back, per-viewer attack outcomes)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Each inner issue is its own dispatch; this plan is the wave-shape that ties them together.

**Wave goal:** Player attacks monster under TURN_BASED, monster takes its turn back, attack outcomes project per viewer.

**Verified by playtest:** alice flips the encounter to TURN_BASED via the harness; on her turn she attacks a seeded monster (e.g. `goblin-1`); the toolkit resolves the attack and publishes per-viewer attack-outcome events through the broker; alice ends turn; the NPC's turn fires (toolkit-driven goblin acts via `monster.TakeTurn`); attack outcomes (hit/miss/damage/condition) project per viewer; alice's HP and the monster's HP update on both browsers' v2 streams; initiative cycles back to alice when the round wraps.

**Depends on:**
- Wave 2.5 (movement, per-viewer projection, broker, translator, v2 service registration) — shipped.
- Wave 2.6 (CreateEncounter, GetEncounter, snapshot replay) — shipping under rpg-project#19. By the time 2.8 implementers pick up, the harness creates encounters via RPC and snapshot replay populates initial state. 2.8 needs `CreateEncounter` to seed a monster.
- Wave 2.7 (OpenDoor / Interact RPC) — queued; not blocking 2.8 strictly, but 2.7's cause/effect-event-split pattern (rpg-api translator preserves the toolkit's deliberate two-event publish) is the same shape we use here for `Attack → Damage → Condition` chains.

## The 2.8/2.9 merge

Originally the roadmap had two waves: 2.8 (TURN_BASED foundation, EndTurn, monster turn) and 2.9 (Attack RPC, action economy, damage). After review:

- The toolkit work for both waves lives in a single new verb (`Encounter.NPCAct(npcID)` wrapping `monster.TakeTurn`, plus making attacks resolve through encounter-emitted events).
- The proto contract already has both `TakeAction` and `EndTurn` in v1alpha2 with the FailedPrecondition-when-not-turn-based gate documented (`service.proto:136-138`), and the events for both — `EntityDamaged`, `TurnStarted`, `TurnEnded`, `ModeChanged`, `StatusApplied` — already exist in `events.proto`.
- Splitting the work creates an ungrounded 2.8 milestone (mode flips and turns cycle, but nothing happens on a turn). The merged wave verifies the full combat loop end-to-end.

So 2.8 and 2.9 collapse into a single combat-slice wave. The only thing 2.10 (Interact + SubmitCheck for traps/checks) doesn't depend on this wave for is its prompt plumbing; that stays a separate wave.

## Architectural calls already made (do not relitigate)

- **Monster AI lives in toolkit.** The toolkit slice is REQUIRED, not optional. Don't propose v2-calls-v1 monster code as a fallback. The rpg-api orchestrator must dispatch to the toolkit's NPC verb when the active turn is an NPC.
- **2.8 + 2.9 merged.** This single combat-slice wave delivers the full "attack monster, monster attacks back" loop.
- **`EntityDamaged`, not `AttackResolved`, is the canonical proto event for resolved attacks.** The proto designers chose `EntityDamaged` (effect) rather than `AttackResolved` (cause+result) as the wire shape — see `events.proto:116-122`. The toolkit may still emit a separate cause-stage event for narration/animation hooks, but the rpg-api translator emits `EntityDamaged` for HP-affecting outcomes and `StatusApplied` for condition outcomes.

## Toolkit verification findings (read these before scoping the toolkit issue)

The toolkit-member dispatch should validate these against fresh reads of the pinned versions; this section is the starting position.

### Encounter SDK (`rpg-toolkit/encounter@v0.2.0`)

- **`Encounter.Data` has NO monster field today.** `data.go:18-23` declares only `ID`, `Sequence`, `Players`, `Doors`. The toolkit slice MUST add `Monsters map[core.EntityID]*MonsterData` (or equivalent), with monster position, hp, AC, action economy, and a serialized `monster.Data` payload (so `LoadFromData` can rebuild the live `*Monster` via `monster.LoadFromData`).
- **No `NPCAct`/turn verb exists.** The encounter package has `Move` and `OpenDoor` as the only verbs. Wave 2.8 introduces the third: `Encounter.NPCAct(npcID core.EntityID)` (or `EndTurn` cycling to next-actor that triggers the NPC path internally — interface decision below).
- **No mode/turn-state on `Encounter.Data`.** No initiative order, active-turn, or mode field. The toolkit slice adds these. Suggested shape: `data.Mode core.EncounterMode`, `data.Initiative []core.EntityID`, `data.ActiveIdx int`, `data.Round int`. The encounter SDK becomes the source of truth for whose turn it is — the rpg-api orchestrator calls into it, doesn't own its own copy.
- **No combat events in `encounter/events/`.** Existing events are `MoveEvent`, `DoorOpenedEvent`, `HexRevealedEvent`, `EntityAppearedEvent`, `EntityDisappearedEvent`. Wave 2.8 adds at minimum: `AttackResolvedEvent` (cause/narration), `DamageDealtEvent` (effect that maps to proto `EntityDamaged`), `ConditionAppliedEvent` (effect that maps to proto `StatusApplied`), `TurnStartedEvent`, `TurnEndedEvent`, `ModeChangedEvent`. All follow the existing `events.EncounterEvent` sealed-type + `PerPlayer` per-viewer-projection pattern (see `events/door_opened.go` as the template).

### dnd5e rulebook (`rulebooks/dnd5e@v0.55.5`)

- **`monster.TakeTurn(ctx, *TurnInput) (*TurnResult, error)` exists** at `monster/monster.go:415`. Inputs: `Bus events.EventBus`, `ActionEconomy *combat.ActionEconomy`, `Perception *PerceptionData`, `Roller dice.Roller`, `Speed int`. Output: `Actions []ExecutedAction`, `Movement []spatial.CubeCoordinate`. **Action selection (utility-scored) and movement (A* toward enemy) are already implemented.** Goblin + Scimitar action shipped; bite shipped (`actions/bite.go`).
- **`combat.ResolveAttack(ctx, *AttackInput) (*AttackResult, error)` exists** at `combat/attack.go:159`. Returns full hit/miss/damage breakdown including critical, advantage/disadvantage, all rolls. The encounter verb wraps this for player attacks too.
- **`combat.TurnManager`** at `combat/turn_manager.go` has `StartTurn` / `EndTurn`. The encounter SDK's turn cycling can either compose this or reimplement; toolkit slice picks based on whether `TurnManager`'s lifecycle (single-use, must-not-reuse-after-EndTurn) fits encounter session shape.
- **dnd5e events exist for `DamageReceivedEvent`, `HealingReceivedEvent`, `ConditionAppliedEvent`, `AttackEvent`** (in `dnd5e/events/events.go`). These are the ruleset-internal events the toolkit publishes during action resolution. The encounter SDK's task is to translate these (or capture them at the action layer) and re-emit them as encounter-scoped per-viewer events on the encounter broker. **Decision point for toolkit dispatch:** does `Encounter.NPCAct` subscribe a temporary listener to the dnd5e bus during `TakeTurn`, capture damage/condition events, and re-publish them as per-viewer encounter events? Or does the encounter SDK pass its own bus and the dnd5e events flow into it directly? Both are viable; the toolkit-member chooses based on cleaner separation.

### Gaps — confirmed not blockers; file as toolkit followups if surfaced

- **`AttackEvent` (the dnd5e cause event at `events.go:443`) has NO resolution data** — no hit/miss/critical. The bite action publishes this and stops; the resolution happens via `combat.ResolveAttack` separately. For Wave 2.8, the encounter verb calls `combat.ResolveAttack` directly for player attacks (NOT the action's `Activate`-then-resolve dance), and for NPC attacks captures the resolution from the action's emissions or wraps `TakeTurn` to record per-action resolution. The toolkit-member dispatches with this as a known shape.
- **Bite and scimitar publish `AttackEvent` but don't directly publish `DamageReceivedEvent`.** The damage publish happens elsewhere in the combat resolution chain. The toolkit-member's job is to wire up clean per-action resolution emission for both player and NPC attacks. If the gap is too large for one wave (e.g., needs a new `AttackResolution` chain in `combat`), file the gap as a separate toolkit issue, scope Wave 2.8 to whatever is buildable within a single wave, and document the deferred work.
- **`CombatantLookup`** (`combat/attack.go:99`-region) requires `attacker` and `target` `combat.Combatant` instances. The toolkit-member confirms how the encounter SDK builds these from `MonsterData`/`PlayerData` snapshots — this is plumbing, not a design question.

## Inner-work shape

### rpg-toolkit — encounter NPC verb + combat events + mode model

This is the biggest of the three slices. Sub-tasks:

1. **Add monster + mode + turn state to `encounter.Data`.**
   - `Data.Monsters map[core.EntityID]*MonsterData` with `ID`, `Position core.Hex`, `HP int`, `MaxHP int`, `AC int`, `MonsterRef string`, `MonsterDataJSON []byte` (so `monster.LoadFromData` can rehydrate the live `*Monster` per call).
   - `Data.Mode core.EncounterMode` (new enum in `encounter/core`: `ModeFreeRoam`, `ModeTurnBased`).
   - `Data.Initiative []core.EntityID`, `Data.ActiveIdx int`, `Data.Round int`.
   - `LoadFromData` rehydrates monsters lazily (or eagerly if cheaper) into `*Monster` instances bound to a fresh dnd5e bus per call.
   - `AddMonster(input MonsterInput)` fixture verb (parallel to `AddPlayer` / `AddDoor`) for tests.

2. **Add mode-flip verb: `Encounter.SetMode(mode core.EncounterMode) error`.**
   - On flip to TURN_BASED, roll initiative (toolkit owns the math), populate `Initiative`, set `ActiveIdx = 0`, `Round = 1`.
   - Publish `ModeChangedEvent` (audience: all players in encounter).
   - Publish `TurnStartedEvent` for the first actor.
   - Mode flip back to FREE_ROAM clears turn state and publishes `ModeChangedEvent`.

3. **Add player-attack verb: `Encounter.TakeAction(playerID, actionRef, target)`.**
   - Validates mode is `ModeTurnBased`; returns mode-violation error otherwise (rpg-api maps to `FailedPrecondition`).
   - Validates `playerID` is the active actor; returns turn-violation error otherwise.
   - For Wave 2.8, only an "attack" action is wired. Action ref = `{module:"dnd5e", type:"action", id:"attack"}`.
   - Builds `combat.AttackInput` from `PlayerData` (attacker) + `MonsterData` (target). Calls `combat.ResolveAttack`. Captures the `AttackResult`.
   - Mutates `MonsterData.HP` based on `AttackResult.TotalDamage` (only on hit).
   - Publishes per-viewer `AttackResolvedEvent` (cause: who attacked whom, hit, critical, rolls — for narration / animation).
   - Publishes per-viewer `DamageDealtEvent` (effect: target id, amount, hp_after) — only if `Hit`.
   - Per-viewer projection: viewers who can perceive both attacker and target see full event; viewers with one-sided LoS see partial; viewers with neither see `Visible: false` (translator drops).

4. **Add NPC turn verb: `Encounter.NPCAct(npcID core.EntityID) error`.**
   - Looks up `MonsterData[npcID]`. Errors if missing.
   - Builds `monster.TurnInput` from `MonsterData` + the encounter's player set. Specifically: `Bus = a fresh in-process dnd5e bus subscribed by the encounter SDK so it can capture damage/condition events`, `ActionEconomy = combat.NewActionEconomy(...)`, `Perception = buildPerceptionFromEncounter(...)` (closest enemies sorted by distance, blocked hexes from walls), `Roller = the encounter's roller`, `Speed = MonsterData.Speed`.
   - Calls `monster.TakeTurn`. Captures `TurnResult.Movement` and `TurnResult.Actions`.
   - For each `ExecutedAction`, the SDK has captured the dnd5e damage/healing/condition events emitted during `Activate`. Translates each into per-viewer encounter events (`AttackResolvedEvent`, `DamageDealtEvent`, `ConditionAppliedEvent`).
   - For `TurnResult.Movement`, emit a per-viewer `MoveEvent` (already supported) + reveal as needed.
   - Mutates `MonsterData.Position` to final hex from `TurnResult.Movement`.

5. **Add `Encounter.EndTurn(actorID core.EntityID) error`.**
   - Validates `actorID` is the active actor.
   - Publishes `TurnEndedEvent` for `actorID`.
   - Advances `ActiveIdx`. If wraps, increments `Round` and publishes a `RoundStartedEvent` (or just a `TurnStartedEvent` for the first actor of the new round; spec the simpler shape).
   - Publishes `TurnStartedEvent` for the new active actor.
   - **Does NOT** automatically call `NPCAct`. The rpg-api orchestrator inspects the new active actor; if it's an NPC, it calls `NPCAct` separately. This keeps the toolkit verbs single-purpose; the orchestrator does the dispatch.

6. **Add new event types in `encounter/events/`:**
   - `AttackResolvedEvent` (DoorOpenedEvent shape): `AttackerID`, `TargetID`, `Hit`, `Critical`, `AttackRoll`, `AttackBonus`, `TargetAC`, plus `PerPlayer map[PlayerID]AttackResolvedSlice{Visible bool}`.
   - `DamageDealtEvent`: `TargetID`, `SourceID`, `Amount`, `DamageType`, `HPAfter`, `MaxHP`, plus `PerPlayer{Visible bool}`.
   - `ConditionAppliedEvent`: `TargetID`, `SourceID`, `ConditionRef`, `DurationRounds`, plus `PerPlayer{Visible bool}`.
   - `ModeChangedEvent`: `From`, `To`, `Reason`. Audience: all players.
   - `TurnStartedEvent`: `ActorID`, `Round`. Audience: all players.
   - `TurnEndedEvent`: `ActorID`. Audience: all players.

7. **Tests.**
   - Unit + integration tests in `rpg-toolkit/encounter/` mirroring `integration_test.go`'s `TestSlice_OpenDoor` shape: two-player + one-monster fixture, alice attacks goblin, asserts `AttackResolvedEvent` + `DamageDealtEvent` per viewer; ends turn; goblin's `NPCAct` runs, asserts goblin moves toward alice and emits its own attack events; bob's PerceptionView gates which events bob receives.
   - Snapshot of `MonsterData` round-trips through `ToData`/`LoadFromData`.

If the wave's executor finds `monster.TakeTurn` requires significant adaptation (not enough action implementations beyond bite/scimitar, or `combat.ResolveAttack` chain doesn't compose cleanly with the NPC path), file specific separate toolkit issues for each gap and reference them from the wave 2.8 toolkit issue. The wave gates on the NPC verb working end-to-end with goblin + scimitar action only — not on filling every action gap.

### rpg-api — mode model + TakeAction/EndTurn handlers + NPC dispatch + combat translators

1. **Mode model on `encounter.Data`** — actually that's the toolkit's responsibility; the rpg-api repo just stores the toolkit data shape. The repository (`internal/repositories/encounters/v2/`) gets no schema change beyond what the toolkit's new fields force.

2. **`SetMode` flip path.** Decision: is mode-flip its own RPC (`SetMode`) or implicit (the harness adds an "ambush" trigger that sets the encounter to TURN_BASED)? The proto doesn't currently define `SetMode`. **For Wave 2.8, expose mode flip via a temporary harness-only path** — either `CreateEncounter(initial_mode=TURN_BASED)` (already in the proto, see `service.proto:18`) which is enough for the wave's playtest goal, or a dev-only method. Don't add a new RPC to the proto for Wave 2.8 unless the wave goal requires it. Mode flip mid-encounter is Wave 2.10+ scope.

3. **`TakeAction` RPC handler.** New file `internal/handlers/dnd5e/v2/encounter/take_action.go`:

   ```go
   func (h *Handler) TakeAction(ctx context.Context, req *encounterv2pb.TakeActionRequest) (*encounterv2pb.TakeActionResponse, error)
   ```

   Required behavior:
   - Auth: `auth.GetPlayerID(ctx)` → empty returns `Unauthenticated`.
   - Validation: `EncounterId`/`ActorEntityId`/`ActionRef`/`Target` empty → `InvalidArgument`.
   - Load encounter data from repo. Missing → `NotFound`.
   - `enc, err := encounter.LoadFromData(data, h.broker)` → `Internal` on error.
   - Mode gate: if `enc.Mode() != ModeTurnBased` → `FailedPrecondition` ("encounter is not turn-based").
   - For Wave 2.8, dispatch only on `action_ref.id == "attack"`. Other ids → `Unimplemented` for now.
   - Resolve target: `target.GetEntityId()` is the monster's entity id. Validate it exists in `data.Monsters`.
   - Call `enc.TakeAction(playerID, actionRef, target)`. Errors map: turn-violation → `FailedPrecondition`; unknown-action → `Unimplemented`; target-out-of-range → `FailedPrecondition`.
   - Save data. Return empty response (events deliver state changes; future scope adds `InputRequired` for things like spell-slot prompts).

4. **`EndTurn` RPC handler.** New file `internal/handlers/dnd5e/v2/encounter/end_turn.go`:

   ```go
   func (h *Handler) EndTurn(ctx context.Context, req *encounterv2pb.EndTurnRequest) (*encounterv2pb.EndTurnResponse, error)
   ```

   Required behavior:
   - Auth + validation as above.
   - Load encounter. Mode gate (`FailedPrecondition` if not TURN_BASED).
   - Call `enc.EndTurn(actorID)`. Errors → `FailedPrecondition`.
   - Save.
   - **NPC dispatch loop** (this is the orchestrator's responsibility, not the toolkit's): after EndTurn, inspect the new active actor. If it's an NPC, call `enc.NPCAct(activeActor)`, save, then recurse: if the NPC's EndTurn-equivalent (toolkit auto-ends NPC turn at end of `NPCAct` or the orchestrator calls `enc.EndTurn(npcID)` — pick one in toolkit dispatch) cycles back to a player, return; if cycles to another NPC, call `NPCAct` for that one too. Cap the loop depth (e.g., max 8 NPCs per chain) to avoid infinite cycles.
   - Return empty response.

5. **Translator extensions in `internal/handlers/dnd5e/v2/encounter/translate.go`:**
   - `*events.AttackResolvedEvent` → drop (no proto event for cause-only attacks; `EntityDamaged` is the canonical wire shape). Optionally emit nothing — narration/animation is web-side derived from `EntityDamaged` for Wave 2.8.
   - `*events.DamageDealtEvent` → `EncounterEvent_EntityDamaged` (`entity_id = TargetID`, `amount = Amount`, `damage_type = damageRefFor(...)`, `hp_after = HPAfter`, `source_entity_id = SourceID`). Per-viewer `Visible: false` → `ErrViewerSawNothing`.
   - `*events.ConditionAppliedEvent` → `EncounterEvent_StatusApplied` (`entity_id = TargetID`, `status = StatusEffectFor(...)`, `source_entity_id = SourceID`).
   - `*events.ModeChangedEvent` → `EncounterEvent_ModeChanged` (`from`, `to`, `reason`).
   - `*events.TurnStartedEvent` → `EncounterEvent_TurnStarted` (`entity_id`, `round`).
   - `*events.TurnEndedEvent` → `EncounterEvent_TurnEnded` (`entity_id`).

6. **Tests.**
   - Unit tests for handler validation/error paths (mode gate, turn-violation gate, missing target, missing actor).
   - Translator tests for each new event type (happy + viewer-saw-nothing).
   - Integration test extending `EncounterV2IntegrationSuite`: two-player + one-monster fixture; alice's `TakeAction(attack, goblin)` produces `EntityDamaged` on both streams; alice's `EndTurn` produces `TurnEnded` and triggers goblin `NPCAct` which produces `EntityDamaged` against alice (assuming hit); both streams receive consistent state.

### rpg-dnd5e-web — combat reducers + harness "attack" + "end turn" buttons + mode flip control

1. **Combat event reducers in `useEncounterState`:**
   - `applyEntityDamaged(event)`: updates the target's HP in local state. If target is the local player's character, the visible HP UI updates.
   - `applyStatusApplied(event)`: appends to the entity's status-effects list in local state.
   - `applyModeChanged(event)`: updates encounter mode in local state. UI conditionally renders TURN_BASED-only controls.
   - `applyTurnStarted(event)`: updates `activeEntityId` + `round` in local state.
   - `applyTurnEnded(event)`: optional — could be no-op if `TurnStarted` covers cycle.

2. **Dispatch extensions in `encounterStream2Dispatch.ts`:** add `entityDamaged`, `statusApplied`, `modeChanged`, `turnStarted`, `turnEnded` cases. Each forwards to corresponding `onX` callback.

3. **New RPC hooks (one file per verb):**
   - `src/api/useTakeActionV2.ts` — wraps `encounterClientV2.takeAction(...)`.
   - `src/api/useEndTurnV2.ts` — wraps `encounterClientV2.endTurn(...)`.
   - Mirror `useMoveEntityV2.ts` shape per `v2-rpc-hook-pattern`.

4. **Harness UI in `PlaytestHarness.tsx`:**
   - Mode indicator (FREE_ROAM / TURN_BASED).
   - Mode flip control: dropdown or button calling `CreateEncounter(initial_mode=TURN_BASED)` for fresh-encounter case, or a dev-only `SetMode` if added.
   - "Attack" button — text input for target entity id (the monster) + button calling `useTakeActionV2` with `action_ref={module:"dnd5e",type:"action",id:"attack"}` and `target.entityId`.
   - "End turn" button — calls `useEndTurnV2`.
   - Active-actor indicator — renders `activeEntityId` from state so users see whose turn it is.
   - HP display — alice's character HP + visible monster HP from `useEncounterState`'s entity HP map.
   - The harness must seed a monster. Mirror Wave 2.7's approach: extend `CreateEncounter`'s starter scenario (Wave 2.6 fixture) to include a `goblin-1` at a known position. Document the choice in PR description.

5. **Tests.**
   - Reducer tests for each new event type.
   - Hook tests for `useTakeActionV2` and `useEndTurnV2` (happy + error paths).
   - Dispatch test extensions.
   - Harness test for new section's render + button-click behavior with a fake stream.

---

## Inner-issue list (filed on board #11 with Wave 2.8 + Status: Todo)

- 🎮 **Tracker** (rpg-project): wave goal + sign-off + contents checklist
- **rpg-toolkit**: encounter NPC verb + mode/turn-state on encounter.Data + combat events + tests
- **rpg-api**: TakeAction + EndTurn handlers + NPC dispatch loop + combat translators + integration test
- **rpg-dnd5e-web**: combat reducers + useTakeActionV2 + useEndTurnV2 + harness combat controls
- **chore: Wave 2.8 close-the-loop** (rpg-project): file followups, update board, draft Wave 2.9 (renumbered Prompts) plan, update role context

---

## Out of scope for Wave 2.8

- Mid-encounter mode flips with narrative triggers (ambush detection, peace negotiated). The wave's mode flip is a dev-only path through `CreateEncounter(initial_mode=TURN_BASED)` or a temporary `SetMode` harness shortcut — full ambush/peace mechanics are Wave 2.10+.
- Action types beyond "attack". No spells, no Dodge/Disengage/Hide actions, no abilities. Wave 2.10+ extends `TakeAction` with more ids.
- Reactions and opportunity attacks. The toolkit's reaction machinery exists but Wave 2.8 only validates simple action → resolution → end-turn loop.
- Multiattack. Action economy capacity for multi-attack characters is in `combat.ActionEconomy` already, but Wave 2.8 ships single-attack-per-turn for goal verifiability.
- Death save mechanics. If a monster's HP hits 0, emit `EntityDied` (existing proto event) and remove from initiative. If a player's HP hits 0, leave dying-state mechanics for Wave 2.10+.
- Conditions beyond a simple "prone" or "poisoned" tag. The `StatusApplied` event ships, but the wave doesn't need rich condition-effect plumbing.
- LobbyView migration. Wave 5.
- SubmitCheck / InputRequired. Wave 2.10 (renumbered Wave 2.9 after this merge).

---

## Forward-loaded patterns (anticipated — refine in close-the-loop)

These are the new patterns Wave 2.8's execution is expected to surface. Marked `status: anticipated` until the wave's close-the-loop step verifies them against shipped reality. Inner issues mention these by name so implementers know what role context will eventually capture.

**rpg-api-member additions:**

- `pat-v2-mode-gating` *(anticipated)* — Handlers for `TakeAction` and `EndTurn` (and any future TURN_BASED-only RPC) check `enc.Mode()` first and return `codes.FailedPrecondition` if not TURN_BASED. This gate lives in the handler, not in the toolkit verb (the toolkit verb assumes the caller has already mode-gated, and produces an internal error if invariant violated; the handler converts to `FailedPrecondition`).
- `pat-v2-npc-turn-dispatch` *(anticipated)* — After `EndTurn` advances initiative, the rpg-api handler inspects the new active actor. If it's an NPC, the handler calls `enc.NPCAct(activeActor)` and recursively cycles through any subsequent NPC turns until the active actor is a player or initiative wraps. This is the orchestrator's job; the toolkit doesn't auto-fire NPC turns.
- `pat-v2-combat-event-translator` *(anticipated)* — Mapping toolkit `DamageDealtEvent` → proto `EntityDamaged`, `ConditionAppliedEvent` → proto `StatusApplied`, etc. Same translator-with-typed-errors pattern as Wave 2.5/2.7. Cause-stage events (`AttackResolvedEvent`) drop on the rpg-api floor when the proto wire shape only carries effects.

**rpg-dnd5e-web-member additions:**

- `pat-v2-combat-event-reducers` *(anticipated)* — `EntityDamaged` updates HP map keyed by entity id; `StatusApplied` appends to entity's status-effects list; `ModeChanged` updates mode in encounter state; `TurnStarted` updates `activeEntityId` + `round`. Render-time UI conditionally shows turn-based controls based on mode and disables action buttons when `activeEntityId` isn't the local player's character.
- `pat-v2-mode-flip-ui` *(anticipated)* — Wave 2.8 establishes a dev-only mode flip (creating a TURN_BASED encounter via `CreateEncounter`); future waves extend with narrative-trigger flips. The harness has a mode indicator + dev-only mode-flip control; Wave 5's LobbyView will render mode purely as a derived state and let server events drive flips.

**rpg-toolkit-member additions** (context dir exists at `docs/teams/roles/rpg-toolkit-member/context/`):

- `pat-encounter-npc-verb` *(anticipated)* — `Encounter.NPCAct(npcID)` wraps `monster.TakeTurn` from the dnd5e rulebook, building `monster.TurnInput` from encounter state and translating the dnd5e bus events into encounter-scoped per-viewer events. Mirrors the Move/OpenDoor verb shape (`encounter.go`) but composes a deeper subsystem.
- `pat-encounter-combat-event` *(anticipated)* — Combat events follow the same per-viewer + sealed-type pattern as `DoorOpenedEvent`. `AttackResolvedEvent` is cause; `DamageDealtEvent` and `ConditionAppliedEvent` are effects. Each carries `PerPlayer map[PlayerID]<Slice>{Visible bool, ...}`. The encounter SDK is responsible for projecting visibility from each player's `PerceptionView`.
- `pat-encounter-turn-state` *(anticipated)* — Mode + initiative + active-actor live on `encounter.Data`. `Encounter.SetMode` rolls initiative on flip-to-TURN_BASED; `Encounter.EndTurn` advances `ActiveIdx` and emits `TurnStarted/TurnEnded` events. Round increments when `ActiveIdx` wraps. The encounter SDK is the single source of truth for whose turn it is — orchestrators query, don't mirror.

After execution, the close-the-loop dispatch promotes verified `anticipated` entries to status-less (validated) and adds any new patterns that surfaced. If `docs/teams/roles/rpg-toolkit-member/context/patterns.json` doesn't already exist, the close-the-loop creates it.

---

## Execution sequence

1. **Wave 2.7 close-the-loop** must complete before 2.8 dispatches. That step writes verified Wave 2.7 patterns into role context — Wave 2.8's dispatches read those.
2. **rpg-toolkit issue dispatches first.** This is the longest critical-path slice because it adds new fields to `encounter.Data`, new verbs, new events, and a new monster integration. Merge a tagged toolkit release (e.g. `encounter@v0.3.0`, `dnd5e@v0.55.6` if the rulebook needs touch).
3. **rpg-api issue dispatches second.** Bumps the toolkit pin to the new version, implements `TakeAction` + `EndTurn`, integration test exercises the full attack → endturn → npc-act → endturn loop.
4. **rpg-dnd5e-web issue dispatches third.** Adds reducers + hooks + harness combat UI. Builds against the merged rpg-api version.
5. **End-to-end playtest** (cold start, both browsers, 2-tab pattern from `v2-playtest-harness`):
   - alice and bob connect to a freshly-created TURN_BASED encounter (harness flips on creation).
   - alice clicks attack on `goblin-1`; both streams render `EntityDamaged`; goblin HP drops on both browsers.
   - alice clicks end turn; both streams render `TurnEnded` for alice + `TurnStarted` for goblin.
   - goblin's NPCAct fires server-side; both streams render goblin's move + attack-resolution events; alice's HP drops on both browsers.
   - goblin auto-ends turn (or orchestrator EndTurns it); both streams render `TurnEnded` for goblin + `TurnStarted` for next initiative.
   - Round 2 begins; alice attacks again; verify state stays consistent.
6. **chore: Wave 2.8 close-the-loop** dispatches: files followups, updates board, drafts Wave 2.9 (renumbered Prompts) plan, captures verified patterns into role context (creating toolkit-member context dir if needed), retros on tracker.

---

## Reference

- Wave 2.8 tracker: rpg-project#<filled in by close-the-loop dispatch>
- rpg-toolkit issue: rpg-toolkit#<filled in by close-the-loop dispatch>
- rpg-api issue: rpg-api#<filled in by close-the-loop dispatch>
- rpg-dnd5e-web issue: rpg-dnd5e-web#<filled in by close-the-loop dispatch>
- Master plan: `rpg-project/ideas/encounter/v1alpha2/plan.md`
- Roadmap: `rpg-project/ideas/encounter/v1alpha2/roadmap.md`
- Design: `rpg-project/ideas/encounter/v1alpha2/design.md` §4 (TakeAction + EndTurn RPC shapes)
- Wave 2.7 plan (template for plan-doc shape): `rpg-project/ideas/encounter/v1alpha2/plans/05-wave-2.7-open-door.md`
- Toolkit references:
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.2.0/encounter.go` — Move + OpenDoor verbs (template)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.2.0/data.go` — Data shape (no monsters/mode/turn yet)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.2.0/events/door_opened.go` — event shape template
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/monster/monster.go:415` — `TakeTurn`
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/monster/action.go` — `TurnInput` / `TurnResult`
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/monster/actions/bite.go` — example action implementation
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/combat/attack.go:159` — `ResolveAttack`
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/combat/turn_manager.go` — `TurnManager`
- Proto: `~/go/pkg/mod/github.com/!kirk!diggler/rpg-api-protos@v0.1.94/dnd5e/api/v1alpha2/encounter/service.proto:66-138` (TakeAction, EndTurn) and `events.proto:95-184` (EntityDamaged, ModeChanged, TurnStarted, TurnEnded, StatusApplied).
- Board: https://github.com/users/KirkDiggler/projects/11
