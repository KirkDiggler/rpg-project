> **SUPERSEDED 2026-05-10** — by `11-wave-2.11-condition-driven-reactions.md`. The original plan invented a `ReactionPromptEvent` shape that fights the ADR-0027 three-phase chain pattern + the ADR-0025 gamectx pattern that the toolkit already implements. It also folded multiattack + AvailableActions surfacing + reactions into one wave that was too large to dispatch coherently. Kept for archaeological reference; do not implement against this plan.
>
> The replacement reuses the toolkit's existing seams: conditions self-subscribe to the chain (per `sneak_attack.go`, `fighting_style_protection.go`, `disengaging.go`), `gamectx` carries Room + CharacterRegistry into the chain (per ADR-0025), and reactions are condition handlers returning reaction-requests that the chain processor batches between phases (per ADR-0027). The Wave 2.11a/b cleanup (per-attack bus, missing gamectx, attacker-and-target-only registry) is folded into the new wave's foundation slice rather than patched separately.

# Wave 2.11 — Combat depth (reactions, opportunity attacks, multiattack — and class-specific action availability surfaced in the harness)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Each inner issue is its own dispatch; this plan is the wave-shape that ties them together.

**Wave goal:** Player attacks a multiattack-capable monster (e.g. boss with 2-3 attacks per turn). Monster moves out of melee range, triggering an opportunity attack reaction; player can choose to take or skip the OA. The harness surfaces class-specific action availability for the active player (monk's unarmed strike bonus action, fighter's fighting style, etc.) by rendering the toolkit's per-character `AvailableAction` list, so class-feature scenarios that exist as toolkit unit tests can be driven end-to-end through the playtest panel.

**Verified by playtest:** alice + bob connect to a freshly-created TURN_BASED encounter seeded with a multiattack-capable monster and at least one boss-tier (or `Bite + Bite` style) enemy. On her turn, the harness shows alice an `available_actions` list reflecting her class — for a fighter, "Attack" + active fighting style indicator; for a monk, "Attack" + "Unarmed Strike (bonus)" + "Flurry of Blows" if granted. alice attacks; the boss takes its multiattack on its turn (2-3 attacks resolve as separate `EntityDamaged` events back-to-back, with `economy.capacities["attacks"]` reflecting the remaining-count between strikes). When alice (or any player) moves out of an enemy's melee reach, the enemy's reaction triggers an opportunity attack: server emits an `InputRequired{reaction_prompt}` to the *reacting actor's* controller (or auto-resolves for NPC reactions), the OA resolves as an `EntityDamaged` event with attribution, and the moved entity's path is interrupted (`MovementInterruption{REASON_OPPORTUNITY_ATTACK}`). Both browsers see the OA outcome consistently; the reactor's `economy.reactions_remaining` decrements; reactions reset at the reactor's turn start.

**Depends on:**
- Wave 2.5 (per-viewer projection, broker, translator, v2 service registration) — shipped.
- Wave 2.6 (CreateEncounter, GetEncounter, snapshot replay) — shipped.
- Wave 2.7 (Interact RPC for unlocked doors) — shipped.
- Wave 2.8 (TURN_BASED, TakeAction/EndTurn, NPC dispatch loop, combat events) — shipped. Wave 2.11 extends the combat slice with capacity-aware attack resolution, reaction triggering, and class-action surfacing on `TurnState`.
- Wave 2.9 (Locked doors, AttemptUnlock, SubmitCheck, pending-prompt machinery) — shipped. Wave 2.11 reuses the pending-prompt model for *reaction* prompts (a new prompt kind: "reaction trigger — take it or pass"). Server pushes the prompt to the reactor (which may or may not be the current active actor); reactor responds via `SubmitCheck` (or a new `SubmitReaction` RPC, decision below).
- Wave 2.10 (Death + Encounter Resolution) — shipping under rpg-project#39 / chore #40. Not strictly blocking 2.11's surface, but the terminal-state predicate (`ModeEnded`, `ErrEncounterEnded`) means reaction handlers must check terminal state before publishing OA outcomes.

## Why this wave exists

Wave 2.10's "out of scope" list explicitly deferred this surface:

> "Reactions and opportunity attacks. Wave 2.11 territory. A monster moving past a player triggers nothing in 2.10."
> "Multiattack. A player or monster takes one attack per turn. Wave 2.11."
> "Action types beyond 'attack'. No spells, no Dodge / Disengage / Hide / Help. Wave 2.11+."

The proto contract has carried capacity-aware action economy and class-action surfacing since v0.1.93 (`types.proto:236-265` — `TurnState.economy`, `TurnState.available_actions`, `ActionEconomy.capacities map<string, int32>` with toolkit-defined keys "attacks", "off_hand_attacks", "flurry_strikes") waiting for an emitter. The toolkit has the deepest pre-existing surface of any wave so far:

- **`combat.ActionEconomy`** (`combat/action_economy.go:11-23`) tracks `ReactionsRemaining`, `AttacksRemaining`, `OffHandAttacksRemaining`, `FlurryStrikesRemaining` — exactly the capacities the proto already maps.
- **`combat.MoveEntity`** (`combat/movement.go:120-250`) walks a path step-by-step, calls `findThreateningEntities` at each hex, calls `triggerOpportunityAttack` when the mover leaves a threatener's reach, returns `OpportunityAttackResult` and a `MovementInterruption`-shaped path-cut. The OA path even publishes `dnd5eEvents.ReactionUsedEvent` (`combat/attack.go:296-302`) when a reaction is consumed during an attack chain.
- **`monster.actions.MultiattackAction`** (`monster/actions/multiattack.go`) executes multiple attacks in sequence with a single action-economy `CostAction`. `monster.TakeTurn` already sequences these.
- **`character.Character.AvailableActions()`** + **`AvailableAbility`** (`character/action_economy.go:79`, `character/action_economy_types.go:9-27`) returns the per-character list of "what can this actor do right now," with `CanUse bool`, `Reason string`, and `Ref *core.Ref` — exactly the proto's `AvailableAction{ref, display_name, available, unavailable_reason}` shape.
- **`fightingstyles.go`** declares all 6 fighting styles (Archery, Defense, Dueling, GreatWeaponFighting, Protection, TwoWeaponFighting) with mechanical descriptions; Protection is explicitly a reaction.
- **`actions/flurry_strike.go`**, **`actions/martial_arts_bonus_strike.go`**, **`actions/off_hand_strike.go`** exist as monk + fighting-style action implementations, gated by `combat.CapacityType` (the proto's "capacities" map keys).

The encounter SDK (v0.5.0) does **not** yet wire any of this together for player actions. `Encounter.TakeAction` (`combat.go:189-249`) uses a stand-in internal `resolveAttack` against `PlayerData.AttackBonus` + `PlayerData.DamageDice` (flat snapshot fields), not the rulebook's `combat.ResolveAttack` chain. There is no per-player `Character` rehydration, no `combat.ActionEconomy` allocation per turn, no movement-with-OA path, and no `AvailableAction` enumeration helper. Today the encounter SDK delivers single-attack-per-turn for both players and NPCs (NPCs *can* multiattack via `monster.TakeTurn` + `MultiattackAction` because the NPC path uses the rulebook directly — but the encounter publishes one aggregate result, not per-attack events).

Wave 2.11 closes that gap end-to-end:

1. **Combat-depth verbs.** Reactions (with prompt machinery), opportunity attacks (movement-triggered), multiattack (per-strike events on the wire). All driven from the toolkit, exposed through the proto's already-built capacity model.
2. **Class-specific action-availability surfacing.** The toolkit's `Character.AvailableActions()` becomes the canonical source for `TurnState.available_actions`. The encounter SDK rehydrates a `Character` per player at turn start, owns the per-turn `combat.ActionEconomy`, and exposes a per-actor enumeration that the rpg-api translator maps to the proto's `AvailableAction` repeated field.

These thread together because reactions/OAs/multiattack are class-feature-driven (Sentinel feat, Polearm Master, Shield spell, Protection fighting style). Surfacing class actions in the harness is the *verification mechanism* for the new combat-depth verbs — without an `available_actions` list, the harness has no way to drive a fighter's "use Protection reaction" or a monk's "use Flurry of Blows after Attack" path.

## Why "after death-resolution, before multi-room flow"

The roadmap places combat-depth after Wave 2.10 (Death + Encounter Resolution) and before Wave 2.12 (multi-room flow) because:

- **After 2.10 (death + encounter resolution).** Combat depth assumes fights can end. Wave 2.10 ships the terminal-state predicate (`ModeEnded`, `ErrEncounterEnded`, `EncounterEnded` event); 2.11 consumes it (reaction handlers must check terminal state; OAs against a dying mover must skip if the encounter ended mid-step). Without 2.10's terminal model, a multi-attack chain or reaction sequence has no clean "stop here, encounter is over" semantic.
- **Before 2.12 (multi-room flow).** Multi-room is mostly room-transition plumbing (encounter-ended → next-room door unlocks, party regroups, fresh encounter created). It does not depend on combat depth — single-attack-per-turn fights would still play multi-room. But a *playtest-credible* boss room (north-star: "boss room at the end") needs multiattack and reactions to feel like a boss; Wave 2.11 unlocks "boss-shaped behavior" before multi-room makes the transitions real.
- **Before 2.13 (party scaling to 4).** Scaling to 4 players amplifies whatever combat behavior exists. Adding 2 more players to single-attack flat fights doesn't pressure-test the system; adding them to multiattack + OA fights does. Combat depth + 4-player exposes coordination patterns (who eats the OA, who positions for Protection, who flurries the downed enemy). Sequenced this way, scaling is the *verification load* for combat depth, not a parallel uncertainty.

## Architectural calls already made (do not relitigate)

- **All combat depth lives in the toolkit.** Per the boundary rule (`CLAUDE.md`): toolkit knows what reactions, OAs, multiattack, and class actions *are*. The encounter SDK orchestrates their composition; the rulebook implements the rules; the rpg-api orchestrator pushes events and translates capacity. **No reaction-eligibility logic, OA-trigger geometry, multiattack sequencing, or class-action enumeration ships in rpg-api or web.** rpg-api forwards toolkit-published events; web renders the `available_actions` list and disables buttons whose `available` is false.
- **`AvailableAction` is the canonical surface for "what can this actor do now."** The proto already defines it (`types.proto:260-265`). The toolkit's `Character.AvailableActions()` (returning `[]AvailableAction` with `Ref`, `Name`, `CanUse`, `Reason`) is the source. Wave 2.11 wires the encounter SDK to enumerate per-actor and the rpg-api translator to map to the proto. **No new RPC** is needed to fetch the list — it ships as part of `TurnState` on every snapshot + after every state change. (A push-on-change event may be added if the snapshot frequency is wrong; spec defers that decision until execution surfaces the need.)
- **Reactions ride the pending-prompt model from Wave 2.9.** A reaction trigger (e.g. enemy moving past you, enemy attacking your ally with Protection eligible) creates an `InputRequired{reaction_prompt}` on the *reactor's* private channel. The reactor resolves via `SubmitCheck` (or a new `SubmitReaction` — see Decision Point below). The toolkit's `combat.MoveEntity` already publishes `ReactionUsedEvent` when a reaction is consumed during an attack chain; Wave 2.11 wires the *prompt* phase before consumption.
- **Cause-effect event split applies to OA.** Same discipline as Wave 2.8 (`AttackResolved` → `DamageDealt`) and Wave 2.10 (`EntityDied` → `EntityRemoved`):
  - **Cause:** `OpportunityTriggeredEvent` (toolkit-internal, narration only — "alice provoked an OA from goblin-1"). May map to no proto event in 2.11 (web derives narration from the effect chain) — decision deferred.
  - **Effect:** `EntityDamagedEvent` (existing) for the OA's damage outcome. `MovementInterruption{REASON_OPPORTUNITY_ATTACK}` on the `EntityMoved` event (existing) for the path cut.
- **Multiattack publishes per-strike events, not aggregate.** Each attack in a multiattack chain emits its own `EntityDamaged` (so the web can animate hit-by-hit, the log reads "goblin-1 bites alice for 3, claws alice for 2, claws bob for 4"). The proto's `economy.capacities["attacks"]` decrements between strikes — the snapshot after each strike reflects the remaining-count. **Atomicity at the wire level is per-strike, not per-multiattack-chain.** A reaction triggered mid-multiattack (e.g. someone Shield-spells a strike) interrupts the chain naturally because the chain is composed of independent strikes.
- **Class-action surfacing is render-layer-only in the harness.** The harness renders `available_actions` and emits buttons; it does not re-implement availability logic. Per `feedback_no_logic_in_web`, the harness *never* gates on game state (e.g., "is this a monk?") — it renders whatever the server pushed in `TurnState.available_actions` and trusts the `available` + `unavailable_reason` fields. If the server pushed "Flurry of Blows: available=true", the harness shows the button enabled. If the server pushed "Flurry of Blows: available=false, reason='requires Attack action this turn'", the button is disabled with tooltip.
- **Player Character rehydration is the encounter SDK's responsibility, not rpg-api.** Today `PlayerData` carries flat `HP/MaxHP/AC/AttackBonus/DamageDice`. Wave 2.11 either (a) extends `PlayerData` with a serialized `character.Data` blob (mirror of how `MonsterData.DataJSON` works), or (b) introduces a `CharacterResolver` interface (parallel to the Wave 2.9 `CharacterResolver` for ability modifiers) that rpg-api implements by loading from its character storage. **Recommendation:** Option (a) — keep the encounter snapshot self-contained, mirror the monster pattern. Toolkit-member confirms during dispatch.

### Decision Point — Reaction RPC shape

Two viable shapes for the reaction-resolution wire surface, decided at execution time:

- **Option A — Reuse `SubmitCheck`.** Add `reaction_prompt` as a new oneof variant in `InputRequired`. Reactor responds with `SubmitCheck{roll: 0}` (no roll for "take the reaction") or skips by sending nothing (timeout? out-of-scope for 2.11; reactor must actively respond). Pros: no proto change beyond an additive oneof; reuses Wave 2.9 plumbing. Cons: `SubmitCheck` semantics imply a *check* — reactions are decisions, not skill rolls. Naming may confuse.
- **Option B — New `SubmitReaction` RPC.** Dedicated request with `take: bool, reaction_ref: Ref`. Pros: clearer intent on the wire, future room for reaction-specific options (which target to apply Shield to, which spell to cast as a reaction). Cons: parallel pending-prompt machinery, more proto surface to verify.

**Recommendation:** Option A for Wave 2.11 (additive oneof variant on `InputRequired`, reuse `SubmitCheck`). Cheaper to ship, validates the "InputRequired carries any prompt kind" hypothesis from Wave 2.9. If a future wave (defensive spells, reaction-with-targeting like Counterspell) outgrows the shape, that wave promotes to Option B. Toolkit-member + rpg-api-member confirm during dispatch.

## Toolkit verification findings (read these before scoping the toolkit issue)

The toolkit-member dispatch should validate these against fresh reads of the pinned versions; this section is the starting position.

### Encounter SDK (`rpg-toolkit/encounter@v0.5.0`)

- **No reaction events exist.** `encounter/events/` carries the Wave 2.10 set (death/removal/end) plus existing combat events. Wave 2.11 adds: `ReactionPromptEvent` (caller-private — published to the reactor only, ignored by other viewers), `OpportunityTriggeredEvent` (cause/narration — optional, may be skipped if proto doesn't carry it), `MultiattackStartedEvent` (cause/narration — optional, signals "the chain is starting" so web can cluster animations).
- **Player attack uses stand-in resolution, NOT `combat.ResolveAttack`.** `combat.go:189-249` runs an internal `resolveAttack` against flat `PlayerData` snapshot fields. Wave 2.11 must replace this with the rulebook chain so reactions, fighting styles, and modifier breakdowns flow through. This is the largest single change in the wave — it forces `PlayerData` to carry rehydrate-able `Character` state (or a `CharacterResolver`).
- **No movement-with-OA in `Encounter.Move`.** `encounter.Move` (look at `encounter.go`) is a simple position update; it does NOT call `combat.MoveEntity`. Wave 2.11 hooks `combat.MoveEntity` for TURN_BASED-mode moves so OAs trigger naturally. FREE_ROAM movement stays unchanged (no OAs outside combat). This means the Move verb branches on mode.
- **Multiattack flows through the NPC path already (per-attack events from monster.TakeTurn), but the encounter SDK collapses them.** `npc.go:120-128` calls `applyCapturedAttacks(mon, *captured)` after `monster.TakeTurn` returns — captured slice contains every dnd5e `AttackEvent` from each multiattack strike, but the publish loop must verify it emits a separate `DamageDealtEvent` per strike (not aggregate). Wave 2.11 audits this and ensures per-strike publication; if `applyCapturedDamage` aggregates, fix it.
- **Per-turn `combat.ActionEconomy` is created per-call, not persisted.** `npc.go:101` calls `combat.NewActionEconomy()` fresh each NPC turn. For players, no economy exists today. Wave 2.11 either (a) persists `combat.ActionEconomy` on `PlayerData` / `MonsterData` and resets on turn-start, or (b) keeps fresh-per-call but exposes a query helper for the rpg-api translator to read mid-turn capacity. **Recommendation:** Option (a) — persist on data, mutate during turn, reset at `TurnStarted`. Mirrors the existing pattern of "snapshot fields owned by encounter SDK."
- **`Encounter.TakeAction` only knows action ref `"attack"`.** `combat.go:206-208` rejects any other id with `ErrUnsupportedAction`. Wave 2.11 extends the dispatcher to route by `actionRef.id` to the matching toolkit action implementation. The dispatch table itself can live in the encounter SDK (a map of `id → ActionHandler`) or be delegated to a per-Character lookup via `Character.GetAction(ref)`. Toolkit-member picks; the per-Character lookup is more extensible (rulebook-defined, not encounter-defined) but requires the Character to be loaded.
- **No `Encounter.AvailableActions(playerID)` enumerator.** Wave 2.11 adds this verb. Returns `[]AvailableActionInfo` (a serializable struct mirroring the proto `AvailableAction`) computed from the loaded `Character` + current `combat.ActionEconomy`. Called by the rpg-api translator each time `TurnState` is built (snapshot or after action).

### dnd5e rulebook (`rpg-toolkit/rulebooks/dnd5e@v0.55.5`)

- **`combat.MoveEntity` is OA-complete** (`combat/movement.go:120-250`). Drop-in for the encounter SDK's TURN_BASED move path. Returns `MoveEntityResult` with `OAsTriggered []OpportunityAttackResult`, which the encounter SDK translates to per-viewer events.
- **`combat.ActionEconomy` is feature-complete** for primary resources + capacities. `OffHandAttacksRemaining`, `FlurryStrikesRemaining` already exist. New capacities (e.g., reaction-granted-action-economy from Action Surge) add new int fields without churning the proto (proto's `capacities` is a map).
- **`character.AvailableActions()` is the surface.** Returns `[]character.AvailableAction` with `Ref`, `Name`, `CanUse`, `Reason`. Maps directly to proto `AvailableAction{ref, display_name, available, unavailable_reason}`. **Note:** The character package also has `AvailableAbility` (action/bonus_action/reaction-consuming) AND `AvailableAction` (granted-capacity-consuming) as two separate slices in `StartTurnOutput`. The proto's single `available_actions` repeated field flattens both. Encounter SDK enumerator concatenates them at translation time. (Or the proto evolves to carry two lists — additive change, defer.)
- **`character.GetActions()` (ActionHolder interface)** returns the live `Action` instances for execute. Same Character serves both enumeration (for UI) and execution (for resolve). Wave 2.11's `Encounter.TakeAction` dispatches via `Character.GetAction(ref).Activate(...)` (or equivalent — verify the exact API at dispatch).
- **`combatabilities/attack.go`, `dash.go`, `disengage.go`, `dodge.go`** exist as the standard PHB action implementations. Wave 2.11's first surfacing target is **Attack** (already wired via stand-in) plus **at least one class-feature variant** (monk Flurry of Blows OR fighter fighting-style indicator) to validate the surfacing path. Full Dodge/Disengage/Hide/Dash + all 6 fighting styles + all monk features is too broad for one wave — file as 2.11 followups what doesn't ship.
- **`fightingstyles.Protection`** is the only fighting style that's *itself* a reaction. Wave 2.11 may surface fighting styles as passive `AvailableAction` entries (display-only, `available: false, unavailable_reason: "passive — always on"`) so the harness can show "Defense (+1 AC)" or "Dueling (+2 dmg)" as informational. Decision deferred to dispatch — the proto field is general enough to carry passives; Kirk's framing ("test the actions available, ... one of the fighting stances for fighter etc.") suggests passives belong in the list.
- **`monster.actions.MultiattackAction`** is shipped and wired into `monster.TakeTurn`. Wave 2.11's task on the monster side is verifying per-strike event publication, not building multiattack itself.

### Gaps — confirmed not blockers; file as toolkit followups if surfaced

- **No "reaction prompt timeout" model.** A reaction prompt sits forever until the reactor responds. For a playtest with synchronous players this is fine; later waves may need a turn-clock. Out of scope for 2.11.
- **No "passive reactions" auto-resolution.** Some reactions (e.g., Hellish Rebuke trigger) require a decision; others (e.g., Sentinel-style "speed becomes 0") are automatic if the actor has a reaction available. Wave 2.11 ships *opt-in* OA (reactor confirms or skips). Auto-OA can ship later as a per-feature config — file as a followup if the playtest reveals "I never want to skip an OA, just take it."
- **No reaction queue ordering.** If two NPCs both have OAs eligible against the same mover at the same step, the toolkit needs a deterministic order. `combat.MoveEntity` already iterates threateners deterministically (id-sorted, per the existing code); verify and document. Multi-reaction handling under multiplayer is complex — defer to playtest discovery.
- **No NPC reaction prompts.** NPC reactions auto-resolve (no prompt — the AI takes the OA if available). Wave 2.11 ships only player-controlled reaction prompts. NPC-decided reactions ride the existing `monster.TakeTurn` path. Player-on-NPC OAs (player moves out of monster reach) auto-trigger the monster's reaction without a prompt.
- **No `available_actions` push-on-change event.** Wave 2.11 ships AvailableAction in the snapshot + on each `TurnStarted` event (refresh). If the playtest reveals stale-state issues (e.g., capacity changes mid-turn aren't reflected without a fresh snapshot), file a toolkit followup to add `AvailableActionsChanged` event or reduce TurnState into a `TurnStateChanged` event.

## Inner-work shape

### rpg-toolkit — combat depth + AvailableAction enumeration + Character rehydration on PlayerData (LARGEST slice — likely larger than Wave 2.10's toolkit slice)

This is the biggest of the slices, by a meaningful margin. Sub-tasks:

1. **PlayerData carries rehydrate-able Character.** Add `CharacterDataJSON []byte` (mirror of `MonsterData.DataJSON`). `LoadFromData` (or per-call rehydration) builds a live `*character.Character` per turn / per action. Existing flat snapshot fields (`HP`, `AC`, `AttackBonus`, `DamageDice`) become deprecated-but-kept for legacy callers; new players seed via Character. Document migration in PR.

2. **Persist `combat.ActionEconomy` on encounter data.** Add `ActionEconomy *combat.ActionEconomyData` (or an inline serializable shape) to both `PlayerData` and `MonsterData`. Mutated during turn; reset at `TurnStarted` via `Reset()`. Speed populated from the actor's character/monster speed.

3. **Replace stand-in `resolveAttack` with `combat.ResolveAttack`.** `combat.go:189-249`'s `Encounter.TakeAction` rebuilds a `combat.AttackInput` from the rehydrated Character and target combatant, calls `combat.ResolveAttack`, captures the `AttackResult`. Damage application + `EntityDamaged` publish stays in encounter SDK (since per-viewer projection lives there). Reactions consumed during the chain (`AttackResult.ReactionsConsumed`) get translated into per-viewer `ReactionUsedEvent`-equivalent encounter events.

4. **Hook `combat.MoveEntity` for TURN_BASED moves.** `Encounter.Move` branches: FREE_ROAM uses existing simple position update; TURN_BASED calls `combat.MoveEntity` with `WithRoom(ctx, room)` + the encounter's `EventBus`. Captures `MoveEntityResult.OAsTriggered`; for each OA, publishes per-viewer encounter events (cause + effect split). Movement is interrupted at the OA hex if the actor died from the OA.

5. **Reaction prompt machinery.**
   - New prompt kind: `PromptKindReaction` in `prompts.go`. Carries `ReactionRef *core.Ref`, `TriggerKind string` ("opportunity_attack", "protection", future kinds), `TriggerSourceID core.EntityID` (who/what triggered it), `TargetID core.EntityID` (e.g., the moving entity), no DC/Ability/Tool.
   - When `combat.MoveEntity` reports an OA-eligible step *for a player-controlled reactor*, the encounter SDK pauses the move, sets `data.PendingPrompts[reactorPlayerID] = {kind: reaction, ...}`, publishes `ReactionPromptEvent` to the reactor only, and returns control to the orchestrator. The mover's path is paused mid-step; on `SubmitCheck` (or `SubmitReaction` per Decision Point), the encounter SDK either consumes the reaction + resolves the OA (calling `combat.ResolveAttack` with `AttackTypeOpportunity`) or skips and resumes movement.
   - For NPC-controlled reactors (NPC moves out of NPC reach — won't happen in 2.11 because monsters don't move during player turns; or NPC reaction to player move), auto-resolve without a prompt. The toolkit captures the OA result and publishes the same effect events.

6. **`Encounter.AvailableActions(playerID core.PlayerID)` enumerator.**
   - Loads the player's Character, reads current `ActionEconomy`, returns `[]AvailableActionInfo` (new struct: `Ref *core.Ref`, `Name string`, `Available bool`, `UnavailableReason string`).
   - Concatenates `Character.AvailableAbilities()` (action-economy-consuming) and `Character.AvailableActions()` (capacity-consuming) into one slice.
   - Optionally includes passive features (fighting styles) as informational entries. Decision per dispatch.

7. **Multiattack per-strike event audit.** Verify `npc.go::applyCapturedAttacks` + `applyCapturedDamage` publish a separate `AttackResolved` + `DamageDealt` per strike from a multiattack `monster.TakeTurn`. If they aggregate, fix to publish per-strike. Ensure the per-strike events include strike index / sequence so the web can render them in order.

8. **Tests.**
   - Unit: per-strike event publication for multiattack NPCs (use a `Bite + Bite` monster fixture or build a test multiattack monster).
   - Integration: extend `integration_test.go` with `TestSlice_OAOnPlayerMove`: two-player + one-monster fixture, alice moves from in-reach to out-of-reach of goblin, OA prompt pushed to alice (but the OA is actually goblin's reaction — re-check: OA is the *threatener's* reaction, not the mover's. So the prompt goes to whoever controls the threatener; for player-vs-NPC moves, NPC reactions auto-resolve. So this test is *NPC OAs alice* and the OA auto-fires). Build a separate test: `TestSlice_OAOnNPCMove_PlayerReacts`: alice has Sentinel feat (or a stand-in reaction-on-leave-reach feature), goblin moves out of alice's reach, prompt pushed to alice, alice confirms, OA resolves. Also `TestSlice_PlayerMultiattack`: alice's character has Extra Attack capacity (set economy.AttacksRemaining = 2), takes two strikes per turn, verify per-strike events + capacity decrement.
   - Integration: `TestSlice_AvailableActionsForFighter` and `TestSlice_AvailableActionsForMonk` — seed a fighter Character + a monk Character, assert the enumerator returns the expected actions per class (fighter: Attack, fighting-style passive; monk: Attack, Unarmed Strike bonus, Flurry of Blows if granted).
   - Snapshot of `PlayerData` + `combat.ActionEconomy` round-trips through `ToData`/`LoadFromData`.

### rpg-api-protos — verification + minor additive (likely additive `reaction_prompt` oneof variant in InputRequired)

This wave's proto issue is **verification + a single additive oneof variant**, not a redesign:

1. **Verify** `types.proto:236-265` (`TurnState`, `ActionEconomy`, `AvailableAction`) carries the shape Wave 2.11 needs. Confirm `ActionEconomy.capacities` map is sufficient for "attacks", "off_hand_attacks", "flurry_strikes", and forward-compatible for new keys without proto change.
2. **Verify** `MovementInterruption.REASON_OPPORTUNITY_ATTACK` (`types.proto:272-274`) carries enough info — the `triggered_by` field (Ref) can identify the threatening entity. Confirm that's sufficient for web narration.
3. **Verify** `EntityDamaged` (`events.proto:96-102`) is sufficient to carry an OA's damage outcome. Confirm `source_entity_id` works as the OA-attacker attribution.
4. **Add** `reaction_prompt` as a new oneof variant in `InputRequired` (currently has `skill_check`, `dialogue`, `target_select`). Shape: `ReactionPrompt { Ref reaction_ref; string trigger_kind; string trigger_source_entity_id; string target_entity_id; }`. Decision per Decision Point above (Option A: this additive change + reuse SubmitCheck; Option B: also add `SubmitReaction` RPC). Default to Option A; if Option B during execution, file the second additive change as a separate proto issue.
5. **If gaps surface during web/api implementation** (e.g., per-strike events need a `multiattack_index` field on `EntityDamaged` to cluster animations), file additive proto changes as separate issues. The wave's proto issue is the verification baseline + the one additive `reaction_prompt`.
6. **Tag a release** for the additive change (e.g. `v0.1.95`).

### rpg-api — emit AvailableActions in TurnState + reaction prompt translator + multiattack per-strike forwarding + integration test

1. **Emit `AvailableAction` list in `buildTurnState`.** `project.go:146` currently leaves `available_actions` empty. Wave 2.11 calls the toolkit's new `Encounter.AvailableActions(playerID)` (or equivalent), maps each `AvailableActionInfo` to a proto `AvailableAction{ref, display_name, available, unavailable_reason}`. Builds per-active-actor (the snapshot for player X shows X's actions when X is the active actor; otherwise an empty list or the active actor's actions, depending on UX decision). Recommendation: the snapshot always shows the *active actor's* available actions, because only the active actor's UI cares; other players' UIs see "waiting for turn." Per-viewer projection: each viewer receives the same list (active actor's actions are not private).

2. **Populate `ActionEconomy.capacities` map.** Read from the toolkit's persisted `combat.ActionEconomyData` on the active actor. Map known capacity types to string keys ("attacks", "off_hand_attacks", "flurry_strikes"). New keys flow without translator change.

3. **Reaction prompt translator.** New translator entry: `*events.ReactionPromptEvent` → caller-private `InputRequired{reaction_prompt: {...}}`. Same caller-private model as Wave 2.9's skill check — does NOT publish on the broadcast stream; rides the response of whichever RPC triggered the reaction (`MoveEntity` for OAs). **Wait — OAs are triggered by the *mover's* MoveEntity, but the prompt goes to the *reactor*.** This breaks the Wave 2.9 model where the prompt rides the response of the calling RPC. The reactor isn't the caller. Decision: the reaction prompt rides the *stream* (caller-private == reactor-private, projected per-viewer with Visible: true only for the reactor). The mover's `MoveEntityResponse` returns normally (with `MovementInterruption{REASON_OPPORTUNITY_ATTACK}` on the resulting `EntityMoved` event); the reactor's stream receives an additional `ReactionPrompted`-shaped event (or, equivalently, a server push of `InputRequired` shape via a new event type). **This is an architectural extension of Wave 2.9's "prompts ride responses, not the stream" rule** — reactions break that rule because the prompt recipient isn't the caller. Document as a deliberate evolution; toolkit-member + rpg-api-member confirm in dispatch.

   Practically: add a new event type `EncounterEvent_ReactionPrompted` (additive proto change; carries `InputRequired{reaction_prompt}` payload). Per-viewer projection emits the event ONLY to the reactor's stream (Visible: false for everyone else). Server's `pendingPrompts[reactorPlayerID]` is set the same way Wave 2.9 sets it. `SubmitCheck` from the reactor resolves it.

4. **NPC dispatch loop guard for reaction prompts.** After `enc.NPCAct`, if the NPC's move triggered a player reaction prompt, the orchestrator must NOT advance to the next NPC turn until the player resolves the reaction. The toolkit's `NPCAct` returns "pending reaction prompt" sentinel; orchestrator pauses NPC chain, returns from `EndTurn`, lets the reactor respond via `SubmitCheck`, and the reactor's `SubmitCheck` resumes the NPC chain (if more NPCs are queued). This is the most architecturally complex part of the rpg-api slice.

5. **Multiattack per-strike forwarding.** Verify the existing per-event translator path (Wave 2.8 / 2.10 shape) doesn't cluster per-strike `AttackResolved` / `DamageDealt` events into one. Each toolkit event becomes one proto `EntityDamaged` event with its own sequence number. Add an integration test asserting a multiattack NPC produces N proto events for N strikes.

6. **Tests.**
   - Translator tests for `ReactionPromptEvent` → `EncounterEvent_ReactionPrompted` per-viewer projection.
   - Translator tests for `available_actions` population in TurnState (across snapshot + post-action paths).
   - Integration test extending `EncounterV2IntegrationSuite`: `TestIntegration_PlayerMovesPastNPC_OAFires`: alice moves out of goblin's reach, both streams receive `EntityMoved{interruption: OPPORTUNITY_ATTACK}` + `EntityDamaged` from the OA. (Goblin's OA auto-fires; no player prompt needed for NPC reactor.)
   - Integration test: `TestIntegration_NPCMovesPastPlayer_PlayerOAPrompt`: configure alice with a reaction-on-leave-reach feature (or use a stand-in test feature), goblin moves out of alice's reach, alice's stream receives `ReactionPrompted`, alice submits via `SubmitCheck` (with `take: true` semantic — exact wire shape per Decision Point), OA resolves as `EntityDamaged` against goblin.
   - Integration test: `TestIntegration_NPCMultiattack_PerStrikeEvents`: seed a multiattack-capable monster (e.g., test "Bite + Bite" monster with `MultiattackConfig{Attacks: ["bite", "bite"]}`), alice ends turn, NPC's multiattack publishes 2 separate `EntityDamaged` events on both streams.
   - Integration test: `TestIntegration_TurnState_AvailableActionsForFighter`: seed a fighter character with Attack + Defense fighting-style, alice connects, snapshot's `TurnState.available_actions` includes "Attack" enabled and (decision-pending) Defense as passive entry.

### rpg-dnd5e-web — available-actions panel + reaction prompt modal + multiattack indicator + harness multiattack-monster fixture

1. **Available-actions panel in `PlaytestHarness.tsx`.**
   - When `useEncounterState`'s `turnState.activeEntityId === localPlayerEntityId` and `turnState.availableActions` is non-empty: render a panel listing each action with name + button. Disabled buttons show `unavailableReason` as tooltip.
   - Each action button calls `useTakeActionV2` with `action_ref = action.ref` and a target picker (entity-id text input for now; same harness pattern as Wave 2.8).
   - Show `economy.actionsRemaining`, `bonusActionsRemaining`, `reactionsRemaining`, `movementRemaining` + the `capacities` map as a small status panel (e.g., "Actions: 1/1 | Bonus: 0/1 | Reactions: 1/1 | Movement: 30ft | Attacks: 0 | Off-hand: 0 | Flurry: 0").

2. **Reaction prompt modal.**
   - Reuse the Wave 2.9 prompt-modal infrastructure. New `pendingPrompt.kind = "reaction"` switch case: render the trigger description ("Goblin-1 is leaving your reach. Take opportunity attack?"), two buttons: "Take" + "Skip".
   - "Take" calls `useSubmitCheckV2` with the take semantic (exact shape per Decision Point — likely `roll: 0` with a side flag, or a separate `useSubmitReactionV2` hook).
   - "Skip" calls the same with the skip semantic. Both clear the modal.

3. **Multiattack indicator.**
   - When `economy.capacities["attacks"] > 1` (or other capacity > 1) on the snapshot delivered to the active actor: show a "Multiattack: X attacks remaining" indicator near the available-actions panel.
   - Each `EntityDamaged` from a multiattack NPC arrives as a separate event; web reducer applies them in order. Optionally, the harness logs each strike with strike index ("Goblin-1 bites alice for 4", "Goblin-1 bites alice for 2").

4. **Per-strike event logging.** Extend the harness's optional log line (Wave 2.10) to render each `EntityDamaged` event as its own log entry. Helps verify per-strike publication during playtest.

5. **Harness multiattack-monster fixture.** Extend `CreateEncounter`'s starter scenario to seed a multiattack-capable monster (e.g., a test "boss-bite" monster with `MultiattackConfig{Attacks: ["bite", "bite"]}` configured via the rulebook). Document the choice in PR description. May require seeding a different monster ref than the Wave 2.8/2.10 goblin.

6. **Tests.**
   - Reducer tests for `availableActions` updates from `TurnStarted` / snapshot.
   - Reducer tests for `economy.capacities` updates between strikes.
   - Hook tests for reaction-resolve hook (whichever the Decision Point chooses).
   - Harness test for available-actions panel renders + button-click dispatches `useTakeActionV2`.
   - Harness test for reaction modal renders on `ReactionPrompted` event + Take/Skip buttons dispatch correctly.

### chore: Wave 2.11 close-the-loop (rpg-project)

Standard close-the-loop shape: file followups, update board, draft Wave 2.12 (multi-room flow) plan, capture verified patterns into role context, retro on tracker.

---

## Inner-issue list (to be filed when wave 2.11 kicks off — NOT filed in this PR)

Per Kirk's preference (file at wave kickoff, not at plan-draft time), these are listed for reference. The Wave 2.10 close-the-loop conversation (already in flight under chore #40) files them when ready to dispatch.

- **TBD: 🎮 Tracker** (rpg-project): wave goal sentence + sign-off + contents checklist
- **TBD: rpg-toolkit** (likely larger than Wave 2.10's): combat depth — replace stand-in `resolveAttack` with `combat.ResolveAttack` chain; persist `combat.ActionEconomy` on PlayerData + MonsterData; rehydrate Character on PlayerData; hook `combat.MoveEntity` for TURN_BASED moves (OA-complete); add reaction prompt machinery (new `PromptKindReaction`, pause-resume movement, NPC-auto vs player-prompt path); add `Encounter.AvailableActions(playerID)` enumerator; audit + ensure per-strike multiattack publication; integration tests for OA-on-player-move + OA-on-NPC-move-with-player-reaction + multiattack per-strike + AvailableActions for fighter + AvailableActions for monk. Tag `encounter@v0.6.0`.
- **TBD: rpg-api-protos**: verify `TurnState` / `ActionEconomy` / `AvailableAction` / `MovementInterruption{REASON_OPPORTUNITY_ATTACK}` shape sufficient (likely yes — they were forward-loaded). Add `reaction_prompt` oneof variant to `InputRequired`. Add `EncounterEvent_ReactionPrompted` event type (caller-private projection). Tag `v0.1.95`.
- **TBD: rpg-api**: emit `AvailableAction` list + `economy.capacities` map in `buildTurnState`; add `ReactionPromptEvent` → `EncounterEvent_ReactionPrompted` translator with reactor-only per-viewer projection; pause NPC dispatch loop on pending player reaction prompt; resume on `SubmitCheck` (or `SubmitReaction` per Decision Point); multiattack per-strike forwarding audit; integration tests for OA player-move + OA NPC-move-player-reacts + multiattack per-strike + AvailableActions populated correctly.
- **TBD: rpg-dnd5e-web**: available-actions panel + economy/capacity status panel + reaction prompt modal (Take/Skip) + multiattack indicator + per-strike event logging + harness multiattack-monster fixture (extend starter scenario or add a "boss-bite" seed).
- **TBD: chore: Wave 2.11 close-the-loop** (rpg-project): file followups, update board, draft Wave 2.12 (multi-room flow — encounter-ended unlocks next room) plan, capture verified patterns into role context (combat depth + AvailableAction surfacing patterns).

(Optional, depending on dispatch decisions):
- **TBD: rpg-api-protos**: if Decision Point chooses Option B, file `SubmitReaction` RPC as a second additive proto change.

---

## Out of scope for Wave 2.11

- **Player dying-state mechanics.** Death saves, downed-but-stable, exhaustion, stabilization, healing-from-zero. Wave 2.10 explicitly deferred this; Wave 2.11 does NOT pick it up. A player at HP=0 still emits `EntityDied` (Wave 2.10) but does NOT have death-save flow. Death saves are a separate wave (likely 2.13 or its own).
- **TPK (total party kill) handling.** Same as Wave 2.10 — encounter-end predicate is monster-count-based; players-all-down does NOT end encounter. Future.
- **Action types beyond Attack + at least one class-feature variant.** Wave 2.11 ships `Attack` (replacing the stand-in) plus enough class-feature surface to validate the harness can drive monk's Flurry of Blows OR fighter's fighting style. Full Dodge/Disengage/Hide/Dash/Help, full spells, full conditions-affecting-actions (incapacitated, stunned, prone) — file as 2.11 followups what doesn't ship within the wave's playtest goal.
- **Spells.** Spell slots, spell selection, concentration tracking — out of scope. Action surface for spells (`Cast Spell` action ref) ships, but only as a placeholder. Spell-system depth is a multi-wave initiative.
- **Reactions beyond opportunity attack and (optionally) Protection fighting style.** Shield spell, Hellish Rebuke, Counterspell, Sentinel feat, Polearm Master OA — not shipping in 2.11 unless trivially in scope. The reaction prompt machinery is general enough to extend; 2.11 ships the machinery + OA validates it.
- **Reaction queue / multiple simultaneous reactions.** If two reactors are eligible for the same trigger, Wave 2.11 fires the deterministic-first reactor and ignores the rest. Multi-reaction handling (priority? simultaneous? player chooses?) is a future polish.
- **Reaction timeouts.** Pending reaction prompts persist until resolved or the encounter ends. No auto-skip after N seconds.
- **Auto-take-OA preference.** All OAs require explicit Take/Skip from the reactor's controller in Wave 2.11 (player reactors only — NPC reactions auto-take). Per-character "always take OAs" preference is future polish.
- **Multiattack chain interruption.** A multiattack chain runs to completion in Wave 2.11. Mid-chain reactions (e.g., Shield-spell after the first hit) extend the model in a future wave. The wire shape (per-strike events) is forward-compatible with chain interruption.
- **Multi-room flow.** `EncounterEnded` per-encounter terminal is shipped (Wave 2.10); wiring next-room transitions is Wave 2.12.
- **LobbyView migration.** Wave 5. Harness is the sole UI surface for the new combat-depth verbs in Wave 2.11.
- **Optimization / capacity push-on-change.** Wave 2.11 ships AvailableActions in snapshot + on `TurnStarted`. Mid-turn capacity changes (e.g., used a bonus action, capacity now decremented) reflect in the next snapshot or via the existing event (TurnStarted-equivalent). If playtest reveals stale-state issues, file a followup for `AvailableActionsChanged` event.

---

## Forward-loaded patterns (anticipated — refine in close-the-loop)

These are the new patterns Wave 2.11's execution is expected to surface. Marked `status: anticipated` until the wave's close-the-loop step verifies them against shipped reality.

**rpg-api-member additions:**

- `pat-v2-available-actions-emitter` *(anticipated)* — `buildTurnState` calls `enc.AvailableActions(activeActorPlayerID)` and maps to proto `AvailableAction[]`. Same per-viewer projection as TurnState (broadcast — all players see the active actor's actions; only the active actor's UI uses them to render buttons). Snapshot includes; `TurnStarted` event implicitly refreshes via subsequent snapshot delivery (no separate refresh event in 2.11).
- `pat-v2-capacity-map-emission` *(anticipated)* — `ActionEconomy.capacities` map populated from `combat.ActionEconomyData.Granted` (toolkit's persisted capacity bag). Keys are toolkit-defined strings ("attacks", "off_hand_attacks", "flurry_strikes"). New capacities flow without translator change — the map is the extension point.
- `pat-v2-reaction-prompt-stream-event` *(anticipated)* — Reaction prompts evolve the Wave 2.9 "prompts ride responses" rule because the prompt recipient is not the caller. Reactions ride a NEW per-viewer event (`EncounterEvent_ReactionPrompted`) projected to the reactor only (Visible: false for everyone else). Server's `pendingPrompts[reactorPlayerID]` is set the same way as skill-check prompts; `SubmitCheck` (or `SubmitReaction`) resolves it. **This is a deliberate evolution of `pat-v2-prompt-issuing-rpc-shape`; document the extension.**
- `pat-v2-npc-dispatch-pause-on-reaction` *(anticipated)* — NPC dispatch loop pauses when an NPC's action triggers a player reaction prompt. Toolkit's `NPCAct` returns a "pending reaction" sentinel; orchestrator returns from `EndTurn`, lets the reactor respond, the reactor's `SubmitCheck` resumes the NPC chain. Loop depth cap (Wave 2.8 introduced) still applies; pause-resume doesn't reset it.
- `pat-v2-per-strike-multiattack-events` *(anticipated)* — Multiattack chains publish per-strike events on the wire (separate `EntityDamaged` per strike, with capacity decrement reflected between strikes). The translator does NOT cluster; web renders strike-by-strike. Forward-compatible with mid-chain reactions in future waves.

**rpg-dnd5e-web-member additions:**

- `pat-v2-available-actions-panel` *(anticipated)* — Harness renders `turnState.availableActions` for the active player only; each entry is a button (enabled if `available`, disabled with tooltip if not). Dispatch via `useTakeActionV2` with the action's ref. Web does NOT compute availability — pure render of server-pushed list.
- `pat-v2-capacity-status-display` *(anticipated)* — Action economy + capacities rendered as a small status panel (text labels + counts). Updates from snapshot + post-action snapshot delivery. Web does NOT compute remaining; pure render of server-pushed `economy.capacities` map.
- `pat-v2-reaction-prompt-modal` *(anticipated)* — `pendingPrompt.kind === "reaction"` switch case in the prompt modal. Renders trigger description + Take / Skip buttons. Take/Skip dispatches via `useSubmitCheckV2` with the take/skip semantic. Modal clears on response.
- `pat-v2-multiattack-strike-rendering` *(anticipated)* — Multiattack NPC strikes arrive as separate `EntityDamaged` events; web applies each in order, optionally logs each strike. No aggregation in the reducer; per-strike state mutation matches per-strike events.

**rpg-toolkit-member additions:**

- `pat-encounter-character-rehydration` *(anticipated)* — `PlayerData.CharacterDataJSON []byte` mirrors `MonsterData.DataJSON`. `LoadFromData` rehydrates a live `*character.Character` per turn / per action. Encounter SDK is the single source of truth for which character snapshot is canonical; rpg-api saves the JSON blob, doesn't reconstruct.
- `pat-encounter-action-economy-persistence` *(anticipated)* — `combat.ActionEconomyData` persisted on `PlayerData` and `MonsterData`. Mutated during turn; reset at `TurnStarted` via `combat.ActionEconomy.Reset()`. Speed populated from actor's Character / Monster.
- `pat-encounter-combat-resolve-chain` *(anticipated)* — `Encounter.TakeAction` calls `combat.ResolveAttack` (not stand-in) for player attacks, captures `AttackResult.ReactionsConsumed`, translates to per-viewer `ReactionUsedEvent`-equivalent encounter events. Reactions consumed during attack chain (e.g., Shield spell against the attack) flow through the existing chain machinery.
- `pat-encounter-move-with-oa` *(anticipated)* — `Encounter.Move` branches on mode: FREE_ROAM uses simple position update; TURN_BASED calls `combat.MoveEntity` with `WithRoom(ctx, room)` + the encounter's bus. OA outcomes (`MoveEntityResult.OAsTriggered`) translated to per-viewer encounter events. Movement interruption (path cut at OA hex) flows as `MovementInterruption{REASON_OPPORTUNITY_ATTACK}` on `EntityMovedEvent`.
- `pat-encounter-reaction-prompt-pause-resume` *(anticipated)* — When an OA-eligible step is reached AND the reactor is player-controlled, encounter SDK pauses the move, sets `data.PendingPrompts[reactorPlayerID]` (kind: reaction), publishes `ReactionPromptEvent` to the reactor only, returns. `SubmitCheck` from the reactor either consumes the reaction + resolves the OA or skips and resumes. NPC reactors auto-resolve without pausing.
- `pat-encounter-available-actions-enumerator` *(anticipated)* — `Encounter.AvailableActions(playerID)` returns `[]AvailableActionInfo` computed from the loaded Character + current `combat.ActionEconomy`. Concatenates `Character.AvailableAbilities()` (action-economy-consuming) + `Character.AvailableActions()` (capacity-consuming). Optionally includes passive features (fighting styles) as informational entries. Called by rpg-api translator each TurnState build.

After execution, the close-the-loop dispatch promotes verified `anticipated` entries to status-less (validated) and adds any new patterns that surfaced.

---

## Execution sequence

1. **Wave 2.10 close-the-loop** (chore #40, in flight) must complete before 2.11 dispatches. That step writes verified Wave 2.10 patterns into role context — Wave 2.11's dispatches read those.
2. **rpg-api-protos issue dispatches first** (additive `reaction_prompt` oneof variant + `EncounterEvent_ReactionPrompted` event type). Tag `v0.1.95`. Likely closes within an hour or two; the contract was forward-loaded for the rest.
3. **rpg-toolkit issue dispatches second.** Largest critical-path slice. Replace stand-in attack resolution, persist ActionEconomy, rehydrate Character on PlayerData, hook combat.MoveEntity for TURN_BASED moves (OA-complete), reaction prompt machinery (pause-resume), AvailableActions enumerator, multiattack per-strike audit. Tag `encounter@v0.6.0` (or higher).
4. **rpg-api issue dispatches third.** Bumps proto + toolkit pins. Emits AvailableActions + capacity map. Adds reaction prompt translator + reactor-only projection. NPC dispatch pause-resume on reaction. Multiattack per-strike forwarding audit. Integration tests cover OA player-move (NPC auto-OA), OA NPC-move-player-reacts (player reaction prompt + Take/Skip), multiattack per-strike, AvailableActions for fighter + monk.
5. **rpg-dnd5e-web issue dispatches fourth.** Available-actions panel, economy/capacity status panel, reaction modal, multiattack indicator, per-strike logging, multiattack-monster fixture.
6. **End-to-end playtest** (cold start, both browsers, 2-tab pattern from `v2-playtest-harness`):
   - alice + bob connect to a freshly-created TURN_BASED encounter seeded with at least one multiattack-capable boss-tier monster. alice's character is a fighter (with a Defense fighting style passive); bob's character is a monk (with Martial Arts + Flurry of Blows granted).
   - alice's turn: harness shows AvailableActions panel — Attack enabled, Defense passive shown as informational. alice clicks Attack on the boss; combat resolves; capacity reflects (if alice has Extra Attack, AttacksRemaining decrements after the strike).
   - alice ends turn. boss's turn: multiattack fires; both browsers receive 2-3 separate `EntityDamaged` events (whoever the boss targeted), boss's `economy.capacities["attacks"]` decrements between strikes (visible to active actor's status panel on its own turn — bob/alice see boss's HP changes per strike).
   - boss tries to move out of alice's reach (or alice tries to move out of boss's reach):
     - Player-moves-out-of-NPC-reach → NPC's auto-OA fires; alice's `EntityMoved` carries `MovementInterruption{REASON_OPPORTUNITY_ATTACK}`; both browsers see `EntityDamaged` from the OA.
     - NPC-moves-out-of-Player-reach (where alice has a reaction-eligible feature wired) → alice's stream receives `ReactionPrompted` (modal pops); alice clicks Take or Skip; if Take, OA resolves as `EntityDamaged` against the boss with alice's reaction consumed.
   - bob's turn: harness shows Flurry of Blows in AvailableActions (after bob has spent the Attack action that grants Flurry). bob takes Flurry; bonus-action capacity decrements; both browsers see the Flurry strikes.
   - Verification: per-strike events arrive in order, capacities reflect correctly, reaction prompt only goes to the reactor, NPC dispatch pauses correctly during a player reaction prompt and resumes on SubmitCheck.
7. **chore: Wave 2.11 close-the-loop** dispatches: files followups, updates board, drafts Wave 2.12 (multi-room flow) plan, captures verified patterns into role context, retros on tracker.

---

## Reference

- Wave 2.11 tracker: rpg-project#<filled in by close-the-loop dispatch>
- rpg-toolkit issue: rpg-toolkit#<filled in by close-the-loop dispatch>
- rpg-api-protos issue: rpg-api-protos#<filled in by close-the-loop dispatch>
- rpg-api issue: rpg-api#<filled in by close-the-loop dispatch>
- rpg-dnd5e-web issue: rpg-dnd5e-web#<filled in by close-the-loop dispatch>
- Master plan: `rpg-project/ideas/encounter/v1alpha2/plan.md`
- Roadmap: `rpg-project/ideas/encounter/v1alpha2/roadmap.md`
- Design: `rpg-project/ideas/encounter/v1alpha2/design.md`
- Wave 2.8 plan (combat slice — closest predecessor for ActionEconomy + ResolveAttack + NPC dispatch + per-viewer combat events): `rpg-project/ideas/encounter/v1alpha2/plans/06-wave-2.8-combat-slice.md`
- Wave 2.9 plan (prompts — pending-prompt model that reactions extend): `rpg-project/ideas/encounter/v1alpha2/plans/07-wave-2.9-prompts.md`
- Wave 2.10 plan (death + encounter end — terminal-state predicate this wave's reaction handlers must check; cause/effect-event-split discipline): `rpg-project/ideas/encounter/v1alpha2/plans/08-wave-2.10-death-resolution.md`
- **Memory:** `project_playtest_may2026.md` — north-star (4-player multi-room boss-fight) AND Kirk's specific framing for Wave 2.11 (2026-05-09): "When we get to reactions we will want to test the actions available, disarmed attack for monk, one of the fighting stances for fighter etc. we have it pretty extensively tested in the toolkit and it'd be great to have that same functionality tested in our play test panel." This is the framing that requires Wave 2.11 to thread combat-depth verbs and class-action surfacing together.
- Proto contract:
  - `dnd5e/api/v1alpha2/encounter/types.proto:236-242` — `TurnState{economy, available_actions}` (already shipped, awaiting emitter)
  - `dnd5e/api/v1alpha2/encounter/types.proto:247-256` — `ActionEconomy{actions, bonus, reactions, movement, capacities map}` (capacities map is the extension point)
  - `dnd5e/api/v1alpha2/encounter/types.proto:260-265` — `AvailableAction{ref, display_name, available, unavailable_reason}` (canonical surface for class actions)
  - `dnd5e/api/v1alpha2/encounter/types.proto:268-282` — `MovementInterruption{REASON_OPPORTUNITY_ATTACK}` (already shipped, awaiting emitter)
  - `dnd5e/api/v1alpha2/encounter/types.proto:284-317` — `InputRequired` oneof (Wave 2.11 adds `reaction_prompt` variant)
  - `dnd5e/api/v1alpha2/encounter/events.proto:21-53` — EncounterEvent oneof (Wave 2.11 adds `ReactionPrompted` variant)
  - `dnd5e/api/v1alpha2/encounter/events.proto:96-102` — `EntityDamaged{entity_id, amount, damage_type, hp_after, source_entity_id}` (carries OA outcomes; already sufficient)
- Toolkit references (verify pinned versions at dispatch):
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.5.0/combat.go:189-249` — current `Encounter.TakeAction` (uses stand-in `resolveAttack`, must replace with `combat.ResolveAttack` chain)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.5.0/encounter.go` — `Encounter.Move` (must branch on mode for OA path)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.5.0/npc.go:90-128` — `Encounter.NPCAct` (verify per-strike multiattack publication; auto-OA reaction path)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.5.0/data.go:42-56` — `PlayerData` (extend with CharacterDataJSON + ActionEconomyData)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.5.0/data.go:96-115` — `MonsterData` (extend with ActionEconomyData)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.5.0/prompts.go:51-80` — `PromptKind` enum + `PendingPrompt` (extend with `PromptKindReaction`)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/combat/action_economy.go` — `combat.ActionEconomy` (Reactions, Attacks, OffHandAttacks, FlurryStrikes — already complete)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/combat/movement.go:120-250` — `combat.MoveEntity` (OA-complete; drop into encounter SDK's TURN_BASED move path)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/combat/attack.go:159-302` — `combat.ResolveAttack` (use for player attacks; captures `ReactionsConsumed` for the reaction-during-attack-chain path)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/character/action_economy.go:79` — `Character.AvailableActions()` (canonical source for proto's AvailableAction list; concatenated with AvailableAbilities)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/character/action_economy_types.go` — `AvailableAction` + `AvailableAbility` shapes
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/actions/flurry_strike.go` — monk Flurry of Blows (one of the class-feature surface targets)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/actions/martial_arts_bonus_strike.go` — monk Martial Arts bonus action
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/fightingstyles/fightingstyles.go` — fighter fighting styles (passive features for the surface)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/monster/actions/multiattack.go` — multiattack monster action (already wired into `monster.TakeTurn`)
- Board: https://github.com/users/KirkDiggler/projects/11

## Sign-off criterion

Wave 2.11 is done when, in a cold-start two-browser playtest:

- alice (fighter) and bob (monk) connect to a freshly-created TURN_BASED encounter with a multiattack-capable boss monster
- on alice's turn, the harness shows alice's AvailableActions panel: Attack (enabled), Defense fighting-style (passive informational entry — or whichever exact surface the dispatch chooses)
- on bob's turn (after spending Attack), the harness shows bob's Flurry of Blows (or Martial Arts bonus strike) as an enabled bonus action; bob takes it; both browsers see the strike
- the boss's turn fires multiattack: 2-3 separate `EntityDamaged` events arrive on both streams, each with its own sequence; the boss's `economy.capacities["attacks"]` reflects the decrement (visible per snapshot)
- when alice (or bob) moves out of an enemy's melee reach, the enemy's auto-OA fires: `EntityMoved` carries `MovementInterruption{REASON_OPPORTUNITY_ATTACK}`; `EntityDamaged` from the OA arrives on both streams
- when the boss moves out of alice's reach (with alice's reaction-eligible feature wired), alice's stream receives `ReactionPrompted`; alice clicks Take or Skip; on Take, OA resolves with reaction consumed
- alice's `economy.reactions_remaining` decrements; resets at alice's next turn start
- subsequent attempts to use a consumed action / capacity are gated server-side (button disabled with `unavailable_reason` tooltip in the harness)

When the close-the-loop runs, the playtest video / screenshots become the verification artifact and the wave ships.
