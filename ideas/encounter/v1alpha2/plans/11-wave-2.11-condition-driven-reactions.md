# Wave 2.11 — Condition-driven reactions (foundation + three-phase chain + OA-as-condition + player-controlled reactions)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Each inner issue is its own dispatch; this plan is the wave-shape that ties them together.
>
> Supersedes `09-wave-2.11-combat-depth.md` (kept for archaeological reference). Replaces the invented `ReactionPromptEvent` model with the conditions-as-chain-subscribers + gamectx-as-state-source model the toolkit already implements.

**Wave goal:** Reactions in this game are conditions that subscribe to the attack/movement chain at the right window. Opportunity attacks become a default `OpportunityAttackCondition` that every melee combatant has Apply()'d once at character/monster rehydration. Player-controlled reactions (Shield spell as the canonical proof) are **opt-in** via per-character readiness flags; when a readied reaction triggers, the server runs the attack as **two discrete RPC phases** with a prompt window between them — phase 1 resolves the hit (toolkit chain runs end-to-end, no pause), the server scans for `ReactionTrigger` events on the result, pushes prompts to ready reactors, waits for responses, then runs phase 2 to apply the outcome (toolkit chain runs end-to-end again with reaction modifiers baked in). When no ready reactions are eligible, the attack runs single-phase end-to-end as it does today. The Wave 2.11a/b cleanup (per-attack bus, missing gamectx, attacker-and-target-only registry) ships as the foundation slice of this wave so the seam is the persistent encounter-level bus from the moment reactions land.

**Verified by playtest:** alice (rogue) + bob (wizard) connect to a freshly-created TURN_BASED encounter seeded with at least one melee monster (e.g. goblin) and at least one boss-tier multiattack monster.
- alice has Sneak Attack Apply()'d on her character; on her turn she attacks the goblin while bob is adjacent to the goblin; the published `EntityDamaged` event reflects sneak attack dice (verified via integration test asserting damage > base weapon roll). The attack runs single-phase end-to-end (no prompt) — Sneak Attack fires inline on the damage chain.
- alice ends turn. The goblin moves out of alice's reach: alice has the default-on `OpportunityAttackCondition` ready (melee combatants default-ready); the move-resolution publishes a `ReactionTrigger` event; the server pushes an `InputRequired{reaction_prompt}` event to alice's stream; she clicks "Take" in the harness; the second-phase resolution dispatches an OA through `combat.ResolveAttack` with `AttackTypeOpportunity`; both browsers see `EntityDamaged` against the goblin; her `economy.reactions_remaining` decrements.
- bob has not toggled "ready Shield" — when the boss attacks him, the attack runs single-phase end-to-end. No prompt arrives; if the roll hits, damage applies immediately; if it misses, no further events.
- bob toggles "ready Shield" via the readiness panel (`SetReactionReady{character_id: bob, reaction_ref: shield, ready: true}`). The boss attacks bob with a strike that *would* hit. The phase-1 hit-resolution publishes a `ReactionTrigger` for Shield (because bob now has Shield ready AND the trigger predicate matches: would hit, +5 AC would deflect); the server pushes an `InputRequired{reaction_prompt}` showing "Cast Shield (1st-level slot, +5 AC retroactive)?"; bob clicks Take; the server invokes phase-2 resolution with the Shield modifier baked into the attack context; phase 2 recomputes effective AC, the strike misses; both browsers see "Goblin attacks Bob — miss (Shield)"; bob's `economy.reactions_remaining` decrements + a 1st-level spell slot is consumed; bob's Shield readiness auto-clears (one-shot for spell-cost reactions).
- alice's Sneak Attack `UsedThisTurn` flag persists across her individual attack within a turn (cannot sneak attack twice on the same turn even with Extra Attack), and resets at her next `TurnEnd`. Verified by integration test: alice attacks twice in one turn (Extra Attack capacity), only one strike includes sneak attack dice; on her next turn, sneak attack dice land again on the first eligible strike.

**Depends on:**
- Wave 2.5–2.10 (per-viewer projection, broker, snapshot replay, OpenDoor, combat slice, prompts, death/encounter-end) — all shipped.
- Wave 2.11a (CombatResolver injection seam) — shipped.
- Wave 2.11b (real `combat.ResolveAttack` chain wired for both player and NPC attacks; in-package `resolveAttack` deleted; weapon-equipping bridge verified) — shipped.
- **ADR-0027 promoted from Proposed to Accepted (2026-05-10)** — the architecture this wave builds against. See `rpg-toolkit/docs/adr/0027-attack-resolution-and-reactions.md`.
- **ADR-0025 (gamectx pattern, Accepted)** — the state-query seam this wave threads through the resolver path so Sneak Attack / Protection / Shield can call `gamectx.RequireRoom(ctx)` and `gamectx.RequireCharacters(ctx)` without crashing.

## Why this wave exists (and why it isn't the old plan)

The original Wave 2.11 plan (`09-wave-2.11-combat-depth.md`, superseded) invented a `ReactionPromptEvent` shape and a "PromptKindReaction" condition machinery in the encounter SDK. That plan was written before two architectural artifacts the toolkit now relies on were fully visible:

1. **ADR-0027 (three-phase attack model + reactions-as-subscribers)** — predicted the Phase 0 / Phase 1 / Phase 2 / Phase 3 chain windows. Promoted to Accepted 2026-05-10 because the chain pattern (`PublishWithChain` → modifiers → `Execute` → `ReactionsConsumed` published) is shipped and conditions like `FightingStyleProtection` already use it.
2. **ADR-0025 (gamectx pattern)** — Accepted and shipped. Conditions query `gamectx.RequireRoom(ctx)` / `gamectx.RequireCharacters(ctx)` instead of bloating events.

Two parallel exploration agents confirmed (2026-05-10) that:

- `combat.ResolveAttack` calls `attackChain.PublishWithChain(ctx, ...)` and `Execute(ctx, attackEvent)`. Conditions subscribe via `dnd5eEvents.AttackChain.On(bus).SubscribeWithChain(ctx, handler)`. The reaction window pattern is **already** the chain — the encounter SDK doesn't need a parallel "prompt machinery."
- Sneak Attack (`conditions/sneak_attack.go`) and Protection (`conditions/fighting_style_protection.go`) are the canonical examples — both subscribe to a chain, both call `gamectx.RequireRoom(ctx)` / `gamectx.RequireCharacters(ctx)`, both modify the chain event at a specific stage.
- `Character.LoadFromData` (`character/data.go:220`) already iterates persisted conditions and calls `condition.Apply(ctx, bus)` for each. The lifecycle is shipped; conditions self-attach when a character rehydrates.
- The Disengaged condition (`conditions/disengaging.go:131-157`) subscribes to `MovementChain`, appends to `OAPreventionSources` at `StageConditions`. The OA-as-condition direction is partially shipped — Disengage **prevents** OAs as a condition; the OA *trigger* itself still lives inline in `combat.MoveEntity`'s `triggerOpportunityAttack` (`combat/movement.go:332-381`).

**The bugs the new plan absorbs:** Wave 2.11a/b shipped a `Dnd5eCombatResolver` path that has latent issues which would surface the moment a player condition (Sneak Attack, Shield, Protection) was Apply()'d through the v2 path:

- **Per-attack bus.** `Dnd5eCombatResolver.ResolveAttack` (`rpg-api/internal/handlers/dnd5e/v2/encounter/dnd5e_combat_resolver.go:165`) calls `bus := events.NewEventBus()` per attack. Conditions are Apply()'d during `character.LoadFromData` (also called per attack at `:257`), so the subscription dies the moment `ResolveAttack` returns. **Sneak Attack's `UsedThisTurn` resets every attack** — a rogue with Extra Attack would sneak-attack twice per turn. **Protection's reaction consumption never persists** — the reaction is "spent" inside an ephemeral bus, then the bus is gone, then on the next attack the action-economy reads "reaction available" again.
- **gamectx never populated.** The resolver's `ResolveAttack` calls `combat.ResolveAttack(resolveCtx, ...)` where `resolveCtx = combat.WithCombatantLookup(ctx, reg)` — only the combatant lookup is threaded. There is no `gamectx.WithGameContext(ctx, gctx)` wrapping the room or the character registry. **Sneak Attack would crash** the chain at `gamectx.RequireRoom(ctx)` returning `ErrNoGameContext`. **Protection would crash** at `gamectx.RequireCharacters(ctx)` for the same reason.
- **CombatantRegistry too narrow.** `Dnd5eCombatResolver.ResolveAttack` registers only attacker + target (`dnd5e_combat_resolver.go:149-150`). Protection's eligibility check needs to find allies — when Protection's bob is the third party (not attacker, not target), the lookup misses him. The decision in Wave 2.11b ("per-attack registration of attacker + target only") is correct for `combat.Combatant` lookups but is the wrong scope for `gamectx`'s `CharacterRegistry`. The gamectx CharacterRegistry needs the full encounter-scope view.

These bugs haven't surfaced because no character in the v2 path Apply()'d a condition that needed them. They'd surface on the first Sneak Attack landing through the v2 stack. **Kirk's call:** these patches will look different after the new plan lands, so we fold them into the wave's foundation slice rather than patch first and rewrite second.

## Architectural calls already made (do not relitigate)

- **Reactions are conditions that subscribe to the attack/movement chain at the right window.** No parallel "reaction machinery" in the encounter SDK. The encounter SDK's role is to hold the persistent bus and the gamectx; the rulebook conditions self-attach. Per ADR-0027 + the shipped code (`sneak_attack.go`, `fighting_style_protection.go`, `disengaging.go`).
- **gamectx is the state-query seam.** Conditions query `gamectx.RequireRoom(ctx)` / `gamectx.RequireCharacters(ctx)` / `gamectx.IsReactionReady(charID, reactionRef)` for game state. The encounter SDK populates gamectx (room + character registry + reaction-readiness state) and threads it onto the resolver context per `combat.ResolveAttack` call. Per ADR-0025.
- **The bus is encounter-scoped, not attack-scoped.** Conditions Apply()'d at character rehydration stay subscribed for the lifetime of the encounter. State that conditions hold (Sneak Attack's `UsedThisTurn`, action-economy mutations) persists across attacks. This is the foundation slice's primary correction to the Wave 2.11a/b implementation.
- **`combat.ResolveAttack` runs as discrete RPC phases, not as one monolithic operation.** Phase 1 (`ResolveAttackHit`) runs the attack chain end-to-end and returns `{attackContext, hit, originalAC, ...}`. The toolkit chain runs cleanly start-to-finish — no pause, no mid-stage interrupt, no in-flight chain state to persist. Phase 2 (`ApplyAttackOutcome`) takes the phase-1 attack context plus a list of reaction modifiers and runs the outcome chain end-to-end (recompute effective AC with reaction bonuses, apply damage if still a hit, publish damage events). Each phase is atomic. The "pause" lives between RPC calls — exactly where state already persists naturally (the encounter snapshot). The server (rpg-api) becomes the explicit orchestrator: runs phase 1, scans for `ReactionTrigger` events, asks reactor questions, runs phase 2 with answers baked in. **The chain primitive does NOT need pause-resume capability** — that complexity moves out of the toolkit and lives at the RPC boundary where it composes naturally with the existing snapshot persistence.
- **Reactions are opt-in via per-character readiness flags.** Default behavior: no reaction prompts ever fire. A condition handler checks `gamectx.IsReactionReady(charID, reactionRef)` before emitting a `ReactionTrigger` event. If the reaction isn't readied, the handler is a no-op and the attack runs single-phase. Default readiness varies by reaction type:
  - **Free-cost reactions (OA)** → default-on for melee combatants. Nobody wants to opt in every fight to a free reaction.
  - **Spell-cost reactions (Shield, Counterspell)** → default-off. Players opt in by toggling readiness, preventing accidental slot burns.
  - **Class-feature reactions (Uncanny Dodge, Hellish Rebuke)** → one-shot or persistent depending on the feature. Spell-cost reactions auto-clear after firing (one-shot); free reactions stay ready until the player cancels (persistent).
  - **Web UI** exposes a "ready reactions" panel for the active character with toggle-able reactions.
  This is a deliberate UX simplification vs RAW 5e (which lets you cast Shield reactively without pre-declaring). The trade is "ambush flexibility" for "no constant prompt mashing." The right call for asynchronous multiplayer without a DM.
- **The OA trigger ports to a condition.** `combat.MoveEntity`'s inline `triggerOpportunityAttack` becomes (or composes with) an `OpportunityAttackCondition` that every melee combatant has Apply()'d by default with `ready: true`. The geometry check (`findThreateningEntities`, `isLeavingThreatRange`) ports into the condition's `MovementChain` subscriber. The reaction-resource check + the call to `combat.ResolveAttack` with `AttackTypeOpportunity` move into the condition handler. Sentinel / Polearm Master / Mage Slayer become variants that subscribe to the same chain with their own modifiers.
- **NPC reactions auto-fire; player reactions prompt.** The condition handler is the same code; the only difference is "is this reactor controlled by a player AND does the reaction need a prompt." For NPC reactors, the condition's auto-resolve callback runs immediately during phase 1 (the AI just takes the OA / Shield-equivalent), so phase 1's result already includes the modifier and phase 2 can run inline without a prompt window. For player reactors with the reaction readied, the condition emits a `ReactionTrigger` event the server reads after phase 1 returns; the server pushes an `InputRequired{reaction_prompt}` event on the reactor's per-viewer stream and waits for `SubmitCheck` to invoke phase 2.
- **Reaction prompts ride the per-viewer event stream as a NEW event type** (`InputRequired{reaction_prompt}`), not the response of the calling RPC. This is the architectural break from Wave 2.9's "prompts ride responses" rule: the reactor isn't the caller. The mover's `MoveEntity` response (or attacker's `TakeAction` response) returns the phase-1 result normally; the reactor's stream receives the prompt as an additional event. Server `pendingPrompts` is keyed by **reactor playerID**, not caller playerID.
- **Toolkit owns the chain, the conditions, the OA geometry, the reaction-resource math, the readiness predicate.** rpg-api orchestrates: holds the encounter-scoped bus, populates gamectx (including the readiness map), runs phase 1 → scans events → pushes prompts → awaits → runs phase 2. **No reaction-eligibility logic in rpg-api.** Per the boundary rule.
- **Reaction readiness state lives on the encounter (or character runtime state on the encounter).** A `map[entityID]map[reactionRef]bool` set + cleared via the new `SetReactionReady` RPC (or equivalent player-state RPC). Persisted on the encounter snapshot so it round-trips naturally. The readiness map is part of what the server hands to gamectx.
- **Per-strike multiattack publication is preserved from the old plan** — that part of `09-wave-2.11-combat-depth.md` was correct (the issue was the reaction-machinery shape, not the multiattack audit). The audit slice rides this wave or the follow-up depending on the split.
- **`AvailableAction` enumeration and `TurnState.available_actions` emission are decoupled from this wave.** The old plan threaded reactions + multiattack + AvailableActions surfacing into one wave; the new plan keeps the focus on the reaction architecture. AvailableActions surfacing is its own wave (proposed 2.11e below) so reactions can ship without dragging the class-action surface scope along.

## Decision points

### Decision Point 1 — Reaction-pause shape

**Question:** When a player-controlled reactor has a readied reaction whose predicate matches at an attack/movement boundary, the resolver flow must pause, the prompt must be pushed to the reactor, the flow must resume on response. Where is the pause? What is its primitive?

**Decision: Discrete RPC phases + opt-in reaction readiness.**

**The shape:**

`combat.ResolveAttack` is split into two discrete operations that the server invokes as separate RPC-driven calls:

```
Phase 1: ResolveAttackHit(attacker, target, weapon)
         → returns AttackContext { roll, originalAC, hit: bool, attacker, target, weapon, ... }
         → toolkit chain runs cleanly start-to-end (no pauses, no in-flight state to persist)
         → during this phase, condition handlers may PUBLISH a ReactionTrigger event on the
           encounter bus when their predicate matches AND gamectx.IsReactionReady(charID, ref)
           returns true. The chain itself does not pause; the trigger is just an event the
           server reads after the chain returns.

[Server (rpg-api) reads ReactionTrigger events from the phase-1 result]
[For each event: push InputRequired{reaction_prompt} onto the reactor's per-viewer stream,
 record pendingPrompts[reactorPlayerID]]
[Defender responds via SubmitCheck{take_reaction: bool}, OR auto-skip if no ready reactions
 triggered (no prompt, no pause)]

Phase 2: ApplyAttackOutcome(attackContext, reactions: [Shield, ...])
         → AC recomputed with reaction modifiers (Shield → +5)
         → may retroactively miss; if hit, damage chain runs and damage applied
         → toolkit chain runs cleanly start-to-end (no pauses)
```

Each phase is atomic. No chain state to persist mid-execution. The "pause" lives between RPC calls — exactly where state already persists naturally (the encounter snapshot). The server orchestrates because in this game there is no DM.

**Opt-in readiness solves prompt clunkiness:**

If every attack always pushed a prompt window, players would mash "continue" forever. So reactions are opt-in:

- **Default: no prompt.** Single-phase attack. 99% of attacks run start-to-finish in one operation (the server inspects phase-1 events, sees no `ReactionTrigger`, immediately invokes phase 2 inline without a prompt window).
- **Player toggles "ready Shield" / "hold OA" / "ready Counterspell"** → a flag flips on the character (`SetReactionReady` RPC). State lives in the encounter snapshot (or character runtime state on the encounter).
- **Condition handler checks `gamectx.IsReactionReady(charID, reactionRef)` before publishing `ReactionTrigger`.** Not ready → no event, no prompt, attack proceeds single-phase. Ready → event published, server splits into two-phase.
- **Default readiness:**
  - **OA**: default-on for melee combatants (it's free; nobody wants to opt in every fight).
  - **Spell-cost reactions** (Shield, Counterspell): default-off (don't accidentally burn slots).
  - **Class-feature reactions** (Uncanny Dodge, Hellish Rebuke): one-shot or persistent depending on the feature.
- **One-shot vs persistent:** spell-cost reactions auto-clear after firing; free reactions stay ready until the player cancels.

**The architecture is "always two-phase capable, but only splits when a ready reaction triggers."** The reactor's *declaration* is what activates the prompt path; absence of any ready reactions means the chain runs end-to-end with zero detour.

**Generalizes to every reaction:**

| Reaction | Trigger predicate | Where the split happens |
|---|---|---|
| OA | "you're moving out of my reach" | between move-resolved and movement continues |
| Shield | "you're being hit and +5 AC would deflect" | between hit-resolved and damage applied |
| Uncanny Dodge | "you're about to take damage" | between hit-confirmed and damage applied |
| Hellish Rebuke | "you took damage" | between damage-applied and turn continues |
| Counterspell | "spell being cast within 60ft" | between spell-declared and spell-resolved |
| Sentinel speed-to-zero | "your OA hit" | between OA-resolved and movement continues |

The chain primitive doesn't need pause-resume. Conditions still subscribe to phase events; when their predicate matches AND readiness is set, they emit a `ReactionTrigger` event the server reads after the chain returns. The chain runs end-to-end on each operation.

**Implementation note for the chain slice:**

The `ReactionTrigger` event shape is roughly:
```go
type ReactionTriggerEvent struct {
    ReactorID    string      // who can react
    ConditionRef core.Ref    // which condition / reaction (Shield, OA, etc.)
    TriggerKind  string      // "post_roll", "movement_oa", "post_damage", etc.
    Window       Phase       // which phase of the resolver published this
    SourceEntity string      // who/what triggered this (the attacker for Shield, the mover for OA)
    Payload      any         // type-specific context (the AttackContext for Shield, the move event for OA)
}
```

NPC reactors don't need a prompt — the condition's auto-resolve callback runs immediately during phase 1, applies the modifier directly to the attack context (so phase 2 sees the modifier baked in), and the server skips the prompt path. Player reactors with the readiness flag set emit the event; the server picks it up.

The phase 2 entry point (`ApplyAttackOutcome`) takes the phase-1 attack context plus a list of reaction modifiers and runs cleanly. No chain state needs to be reconstructed because each phase builds its result from inputs, not from in-flight state.

**Alternatives considered (rejected):**

- **Option A — Synchronous mid-chain pause.** The chain processor would store its execution state mid-chain (current event, remaining stages, queued reactions) into encounter persistence, return a sentinel error (`ErrChainPaused`), and the reactor's `SubmitCheck` would resume from the stored state. **Rejected because** serializing in-flight chain state means serializing a mutable Go struct with registered closures (`modifyAttack func(...)` chain stages). That's not JSON-round-trippable without significant refactoring of the chain primitive itself, and it introduces a fragile resume path that's hard to reason about (what if the encounter changed between pause and resume?).

- **Option B — Declarative reaction-request batching at chain phase boundaries.** Conditions append `PendingReactionRequests` to chain events; after each phase the chain processor inspects the field and pauses if non-empty. **Rejected because** Shield's actual mechanic is retroactive — "the attack just rolled 19 vs my AC 14, +5 AC means it's 19 vs 19, re-evaluate hit." Implementing this with batching means the chain has to be **reversible**: the hit-decision stage already executed at the time the post-roll publish happens, so applying Shield's AC bonus requires re-running the hit decision. That either means the chain has a "pre-decision phase" + "decision phase" split (which is just a phase boundary by another name) or the chain has reversible stages (much harder than reversible RPC calls). Either way the design is wrestling against the chain's natural shape.

The discrete-phases design beats both because it doesn't pause anything inside the chain. Each phase runs end-to-end. Conditions stay normal subscribers. The split is at the RPC boundary — where the snapshot persistence already lives — not inside the chain. And opt-in readiness means the split rarely happens, so the common case stays single-phase.

### Decision Point 2 — Reaction RPC shape

**Question:** How does the reactor respond to a reaction prompt on the wire? And how is reaction readiness toggled?

The two-phase orchestration changes the natural shape here. The reaction prompt is the thing that arrives on the reactor's stream after phase 1; the call that the reactor makes to take/skip the reaction is the trigger that causes the server to invoke phase 2 with the player's choice baked in. The wire surface needs **two** capabilities, not one:

1. **Take/skip a prompted reaction** (response to an `InputRequired{reaction_prompt}` on the stream). This is the Wave 2.9-style prompt-response shape.
2. **Toggle reaction readiness** (out-of-band, not in response to a prompt). The player flips "ready Shield" / "hold OA" before any attack happens; this changes the readiness map so future condition handlers will publish triggers.

**Options:**

- **(A) Reuse `SubmitCheck` for the take/skip + new `SetReactionReady` RPC for toggling.** `SubmitCheck` gains an additive `take_reaction: bool` field (when set, interpreted as a reaction prompt response). A new RPC `SetReactionReady{character_id, reaction_ref, ready: bool}` toggles the readiness map. Two additive proto changes total: oneof variant in `InputRequired`, additive field in `SubmitCheck`, plus the new RPC.
- **(B) New `SubmitReaction` RPC + new `SetReactionReady` RPC.** Dedicated wire surface for both reaction-specific decisions. Cleaner separation but two new RPCs to design.
- **(C) Fold `SetReactionReady` into an existing player-state RPC** (if one fits — e.g., a `SetCharacterPreferences` RPC that already manages similar toggles). Probably no such RPC exists today.

**Decision: Option A for Wave 2.11d.**

**Rationale:**
- For the **take/skip** wire shape: the prompt is yes/no + an implicit reaction-ref (the prompt itself carries the ref). That fits `SubmitCheck` extended with a `take_reaction` semantic, same upgrade path Wave 2.9 set for `SubmitCheck` itself ("we'll add a richer RPC if the prompt model outgrows it"). Shield in 2.11d doesn't ask the player to pick a slot level (always 1st); it asks "cast Shield: yes/no." When a future wave adds Counterspell (pick a slot level) or reaction targeting (Polearm Master picks which adjacent enemy to OA), promote to `SubmitReaction`.
- For the **readiness toggle**: a dedicated `SetReactionReady` RPC is the cleanest fit. It's not a response to a prompt, it's a player intent that mutates encounter state. Folding it into `SubmitCheck` would overload the call. Folding it into a generic player-state RPC is theoretically nice but no such RPC exists, so we'd be inventing one anyway.

**Implementation note for the protos slice:**

- Add `ReactionPrompt { core.Ref reaction_ref; string trigger_kind; string trigger_source_entity_id; string target_entity_id; string display_text; }` as a new oneof variant in `InputRequired` (`types.proto:284-317`).
- Add `optional bool take_reaction` field to `SubmitCheckRequest`. When set, server interprets the response as a reaction take/skip and invokes phase 2; when not set (existing callers), behavior is unchanged.
- Add a new RPC: `SetReactionReady(SetReactionReadyRequest{ encounter_id, character_id, reaction_ref, ready: bool }) returns (SetReactionReadyResponse{ ... })`. The server updates the encounter's readiness map, persists the snapshot, and publishes a confirmation event on the stream so all viewers (or just the readying character) see the readiness state change in the UI.

### Decision Point 3 — OA scope for Wave 2.11

**Question:** The OA-as-condition port + the OA *prompt* path + the Shield-as-condition implementation are all genuinely new code. How much ships in 2.11?

**Recommendation: Wave 2.11 ships:**

1. **Foundation:** persistent bus + gamectx + broadened registry (the 2.11a/b cleanup folded in). gamectx populated with the encounter's reaction-readiness map so condition handlers can call `gamectx.IsReactionReady(charID, reactionRef)`.
2. **Discrete attack phases:** split `combat.ResolveAttack` into `ResolveAttackHit` (phase 1) + `ApplyAttackOutcome` (phase 2). Each runs the appropriate chain end-to-end. Phase 1 produces an `AttackContext` carrying everything phase 2 needs; phase 2 takes the context plus a list of reaction modifiers and produces the final outcome (hit/miss reconsidered with bonuses, damage applied if still a hit, events published).
3. **`ReactionTrigger` event** the chain emits (via condition handlers) when a reaction's predicate matches AND `gamectx.IsReactionReady` returns true. Server-readable after the chain returns.
4. **Reaction readiness state on encounter (or character runtime state on encounter).** A `map[entityID]map[reactionRef]bool`. Persisted on the encounter snapshot. Set via `SetReactionReady` RPC. Read by gamectx (via `IsReactionReady`).
5. **OA-as-condition:** port `combat.MoveEntity`'s `triggerOpportunityAttack` into an `OpportunityAttackCondition`. Every melee combatant Apply()'s it on rehydration with `ready: true` by default (free reaction; default-on). NPC OAs auto-fire (current behavior, the auto-resolve callback runs inline during phase 1). Player OAs publish a `ReactionTrigger` event from the condition; the server pushes the prompt and waits for `SubmitCheck` to invoke phase 2.
6. **Shield as a condition:** subscribes to the post-hit boundary, returns a `ReactionTrigger` event when the attack would hit but Shield would miss it AND the wizard has Shield readied. Take path → consume reaction + spell slot, reaction modifier in phase-2 input bumps effective AC by 5, hit re-evaluated. Skip path → proceed without modifier. Shield's readiness auto-clears after firing (one-shot for spell-cost reactions).
7. **Reaction prompt translator + per-viewer projection + server orchestrator.** Reactor-only visibility. `pendingPrompts[reactorPlayerID]`. `SubmitCheck` invokes phase 2 with the player's choice baked in.
8. **`SetReactionReady` RPC + handler + readiness panel in web.** Default readiness configured per reaction type (OA default-on for melee combatants; Shield default-off; etc.).
9. **Multiattack per-strike audit** — verify the per-strike publication path from Wave 2.11b's resolver swap still emits separate `EntityDamaged` events per strike. This is verification + a small fix if needed, not new code.

**Wave 2.11 does NOT ship:**

- Sentinel / Polearm Master / Mage Slayer / Counterspell / Hellish Rebuke. Same condition pattern + same opt-in readiness machinery, file as 2.11 followups when surfaced. Each is a 1-week slice once the foundation is in.
- `AvailableAction` enumeration and `TurnState.available_actions` emission. Decoupled to its own wave (proposed 2.11e below).
- Per-reaction-type custom NPC strategy. NPCs always auto-take available reactions in 2.11.
- Reaction queue ordering (multi-reactor at same trigger). Deterministic-first reactor handles it; multi-reaction polish is future.
- Reaction timeouts on player prompts. Sit forever until response; future wave adds clock.
- Configurable per-character readiness defaults beyond the per-reaction-type defaults (OA on, Shield off). Future polish.

### Decision Point 4 — Single tracker or two-wave split

**Question:** Foundation + discrete attack phases + OA-as-condition + Shield + reaction prompts + readiness toggle + multiattack audit is a large scope. One tracker or two?

**Recommendation: Split into 2.11c (foundation + discrete phases) and 2.11d (player reactions + OA + Shield + readiness).**

**Rationale:** The foundation slice (persistent bus + gamectx + broadened registry) and the discrete-phase split of `combat.ResolveAttack` (phase 1 hit / phase 2 outcome) are pure-toolkit + pure-rpg-api wiring with no new player-facing behavior. They could ship as their own wave and be verified by an integration test (Sneak Attack now persists `UsedThisTurn` across attacks; Protection now consumes reaction once; gamectx-requiring conditions don't crash; the discrete-phase split runs phase 1 then phase 2 inline when no reactions are ready, producing the same final result as today's monolithic call). Shipping the foundation independently de-risks the player-reaction work that follows: when 2.11d's readiness + prompt machinery lands, the discrete phases + bus + gamectx it depends on are already proven in production.

The split also matches the producer-pattern: each wave gets one tracker, one playtest goal. 2.11c's playtest goal is "Sneak Attack works through the v2 path (alice rogue + ally bob next to goblin → sneak attack damage in EntityDamaged); attacks still resolve correctly through the new discrete-phase split." 2.11d's playtest goal is "Wizard with `ready Shield` toggled receives a prompt when hit, casts Shield, retroactively avoids the hit. Player without readied reactions sees no prompts. OA fires automatically when player moves out of monster reach (default-on readiness for melee combatants)."

**The rest of this plan documents the FULL Wave 2.11 scope as one logical unit.** The split into 2.11c and 2.11d is recommended for execution; both trackers reference this plan. If during dispatch the foundation slice surfaces enough complexity to warrant standing alone, 2.11c ships first; 2.11d's tracker stays open as the follow-up. If the foundation lands cleanly and player-reaction work is small, 2.11c and 2.11d can collapse to a single tracker — that's a director-call at dispatch.

## Inner-work shape

### Slice 1 — rpg-toolkit foundation (encounter-scoped bus + gamectx wiring + broadened registry + readiness state)

This is the cleanup of Wave 2.11a/b's latent bugs plus the readiness-state foundation, now the foundation of the new wave. It ships before any reaction work.

1. **Encounter-scoped bus.** `tkenc.Encounter` (encounter SDK) holds an `*events.EventBus` for the lifetime of the encounter. `LoadFromData` constructs the bus once. `Encounter.TakeAction` / `Encounter.NPCAct` / `Encounter.Move` use the same bus across all calls. Conditions Apply()'d during character/monster rehydration subscribe to this bus and stay subscribed.
   - Verify: extend the `tkenc.Data` shape if needed so the bus is reconstructable on `LoadFromData` without losing condition state. Conditions that stored `UsedThisTurn` etc. need to round-trip; `character.Data` already persists conditions as JSON blobs (`character/data.go:208-229`). The bus does not persist; it's reconstructed and conditions re-Apply when the character rehydrates. State they depend on (UsedThisTurn) is in the condition's JSON blob.
   - Open question for the implementer: does `tkenc.Encounter` reconstruct the bus on every `LoadFromData`, or is `LoadFromData` called once per encounter session? The current `Dnd5eCombatResolver` flow (`dnd5e_combat_resolver.go:245-262`) suggests per-attack `LoadFromData` for the character — that's wrong now. The handler needs to load the encounter's bus once at encounter rehydration, then pass it through. **Implementer's call:** how exactly the handler gets a stable bus per encounter is an implementation detail; the constraint is that conditions Apply()'d on bus B at time T are still subscribed to bus B at time T+1 within the same encounter.

2. **Wire gamectx in `Dnd5eCombatResolver.ResolveAttack`.**
   - Construct a `gamectx.GameContext` populated with a `CharacterRegistry` covering all combatants in the encounter (not just attacker + target), plus the **reaction-readiness map** (see step 4).
   - Wrap the room: `ctx = gamectx.WithGameContext(ctx, gctx)`.
   - The room comes from the encounter's spatial state. `tkenc.Encounter` already has the room reference (used by `combat.MoveEntity` calls); expose it on whatever surface the resolver can read. **Open question:** does the resolver get the room via `Dnd5eCombatResolverConfig` (set at handler setup) or via the per-request `NewDnd5eCombatResolverForData(cfg, data)` (the room would need to be on `tkenc.Data` for this — verify)? Either works; the implementer picks.
   - The `combat.WithCombatantLookup(ctx, reg)` call already happens; `gamectx.WithGameContext` is added alongside. The chain handler context becomes: `ctx = gamectx.WithGameContext(combat.WithCombatantLookup(ctx, reg), gctx)`.

3. **Broaden the gamectx CharacterRegistry to encounter-scope.**
   - The `combat.CombatantLookup` registry stays per-attack (attacker + target) per Wave 2.11b's decision — that's the rulebook chain's contract.
   - The `gamectx.CharacterRegistry` is encounter-scope: it covers every PlayerData + every MonsterData in the encounter. Built once per `Dnd5eCombatResolver` construction (or memoized; encounter content doesn't change mid-attack).
   - The registry's `GetCharacterWeapons / GetCharacterAbilityScores / GetCharacterActionEconomy` calls map to the rehydrated `*character.Character` for player IDs and to the rehydrated `*monster.Monster` for monster IDs. `monster.Monster` doesn't have an action-economy in the same shape; for monsters, return a stub that satisfies the interface (or use a proper action-economy if `monster.Monster` ships one). **Implementer's call** based on the shipped monster API.

4. **Reaction readiness state (NEW).**
   - Add a `ReactionReadinessMap` shape to gamectx (or to the encounter and exposed via gamectx — implementer's call). Shape is roughly `map[entityID]map[reactionRef]bool`. A `true` entry means "this character has this reaction readied; condition handlers should publish a `ReactionTrigger` event when their predicate matches."
   - Expose `gamectx.IsReactionReady(charID string, reactionRef core.Ref) bool` as the helper conditions call. Returns `false` for any unknown entry (not-ready is the safe default).
   - The map is populated from the encounter snapshot (the encounter persists readiness state per-character; see Slice 5 for the rpg-api side that mutates it via `SetReactionReady` RPC).
   - Default seeding at character/monster rehydration: when the encounter SDK applies `OpportunityAttackCondition` (step 5 below) to a melee combatant, also seed `readiness[charID][refs.Conditions.OpportunityAttack()] = true`. Spell-cost reaction conditions (Shield) seed `false`.

5. **Auto-Apply default reaction conditions on every combatant.**
   - `OpportunityAttackCondition` (defined in slice 3) is Apply()'d for every melee-capable player + every melee monster at character/monster rehydration. The condition is the OA *trigger* — every combatant that can reach an enemy has one. Apply with `ready: true` (default-on for OA per the readiness defaults; reflected in the readiness map seeding above).
   - Where this Apply happens: for players, in `character.LoadFromData`'s condition-loop if it's persisted as a condition on the character; alternatively, the encounter SDK adds it programmatically when it rehydrates a player into the encounter. **Implementer's call** — whichever shape composes more cleanly with the existing pattern.
   - For monsters, similar: the `monster.LoadFromData` path adds the OA condition (or the encounter SDK does it).

6. **Tests.**
   - Unit: persistent-bus test — Apply a condition with state (`UsedThisTurn`-style), trigger two attacks, assert state persists.
   - Unit: gamectx-populated test — call `Dnd5eCombatResolver.ResolveAttack` with a Sneak Attack condition Apply()'d, assert the chain executes without `ErrNoGameContext`.
   - Unit: broadened-registry test — Apply Protection on a third party, assert `gamectx.RequireCharacters(ctx).GetCharacterWeapons(thirdPartyID)` returns the third party's weapons.
   - Unit: readiness-map test — `gamectx.IsReactionReady(unknownChar, anyRef)` returns false; `IsReactionReady(charWithOA, OA-ref)` returns true after default-seeding.
   - Integration: extend the v2 encounter integration suite with `TestSlice_SneakAttackPersistsUsedThisTurn`: alice (rogue with Sneak Attack + Extra Attack) attacks twice in one turn, only one strike's damage exceeds the base weapon roll; on her next turn, the first strike includes sneak attack again.

### Slice 2 — rpg-toolkit discrete attack phases + ReactionTrigger event

The discrete-phase split from Decision Point 1. **No chain pause-resume primitive** — the chain primitive doesn't change. The split happens at the `combat.ResolveAttack` API surface; each phase invokes the chain end-to-end.

1. **Split `combat.ResolveAttack` into discrete phase functions.**
   - **`combat.ResolveAttackHit(ctx, *ResolveAttackHitInput) (*AttackContext, error)`** — phase 1. Runs the attack chain end-to-end (the existing `attackChain.PublishWithChain(ctx, ...)` + `Execute(ctx, attackEvent)` flow), produces an `AttackContext` carrying:
     - The attacker, target, weapon refs.
     - The roll result (natural d20 + modifiers).
     - The `originalAC` (target's AC before any reactions).
     - `wouldHit` boolean computed against `originalAC`.
     - Any chain-stage modifiers that the resolver-side phase already accumulated (advantage flags, situational modifiers, sneak-attack-eligibility flag, etc.).
     - Anything else phase 2 needs to make a final decision and apply damage.
   - **`combat.ApplyAttackOutcome(ctx, *ApplyAttackOutcomeInput) (*AttackResult, error)`** — phase 2. Takes the phase-1 `AttackContext` plus a `[]ReactionModifier` slice (could be empty for the no-reactions path). Recomputes effective AC = `originalAC + sum(reactionModifiers.ACBonus)`. Re-evaluates hit/miss against effective AC. If hit, runs the damage chain end-to-end (the existing `damageChain` + `Execute` flow) and publishes `EntityDamaged`. If miss (retroactive miss from Shield, etc.), publishes a miss event.
   - The existing `combat.ResolveAttack` becomes a thin wrapper: `ResolveAttackHit` followed immediately by `ApplyAttackOutcome` with no reaction modifiers. Callers that don't care about reactions (NPC vs NPC; toolkit-internal callers; tests) keep working unchanged.

2. **`ReactionTrigger` event.**
   - Define a new event type (likely `ReactionTriggerEvent` or `ReactionTriggerTopic`) the chain handlers publish on the encounter bus when their predicate matches AND `gamectx.IsReactionReady(reactorID, conditionRef)` returns true.
   - Shape:
     ```go
     type ReactionTriggerEvent struct {
         ReactorID    string      // who can react
         ConditionRef core.Ref    // which condition (Shield, OA, etc.)
         TriggerKind  string      // "post_hit", "movement_oa", "post_damage", etc.
         SourceEntity string      // who/what triggered this
         Payload      any         // type-specific context (the AttackContext for Shield, the move event for OA)
     }
     ```
   - Conditions publish this event during their normal chain handler logic. The chain runs to completion either way; the event is just additional output.
   - For NPC reactors, the condition's auto-resolve path skips the event entirely and applies the modifier directly to the in-flight chain (preserving current behavior). The split between "publish event for player" and "auto-resolve for NPC" is decided by gamectx context (whether the reactor is a player-controlled entity); implementer picks the exact seam.

3. **No chain pause-resume primitive.** The previous plan called for in-process chain pauses with persisted in-flight state. **That is no longer needed.** The chain runs end-to-end on each phase. The "pause" lives between phase 1 and phase 2 — at the RPC boundary in rpg-api — where the encounter snapshot already persists naturally.

4. **`ReactionModifier` shape (input to phase 2).**
   - When phase 2 is invoked with reactions, each `ReactionModifier` carries:
     - `ConditionRef` — which reaction this modifier comes from (Shield, etc.).
     - The actual modification (e.g., `ACBonus int`, or a more general shape if needed for future reactions).
     - Resource-consumption metadata so phase 2 can call back into the condition to consume the spell slot / reaction (or the orchestrator does that before invoking phase 2 — implementer's call).

5. **Encounter SDK surface for the orchestrator.**
   - `Encounter.ResolveAttackHit(input) (*AttackContext, []ReactionTriggerEvent, error)` — wraps `combat.ResolveAttackHit`, drains the encounter bus's `ReactionTrigger` events from this call's window, returns both. The orchestrator (rpg-api) reads the events to decide whether to push prompts.
   - `Encounter.ApplyAttackOutcome(attackContext, reactionModifiers) (*AttackResult, error)` — wraps `combat.ApplyAttackOutcome`. Called either inline immediately after phase 1 (no reactions) or after the orchestrator has collected reactor responses and translated them into modifiers.
   - Same shape for movement: `Encounter.ResolveMoveStep` could publish `ReactionTrigger` events for OA; the orchestrator reads them and either auto-completes the move (no readied OAs) or pauses for the player to take/skip.
   - **Open question:** does the encounter SDK expose phase 1 + phase 2 as separate verbs on `Encounter`, or as a single verb (`TakeAction`) that returns either a final result or a pending-reaction shape that requires a follow-up call? Either works. The discrete-phase split is the load-bearing decision; the surface shape composes either way.

6. **Tests.**
   - Unit: `ResolveAttackHit` runs the chain end-to-end and produces an `AttackContext` with `originalAC` and `wouldHit`.
   - Unit: `ApplyAttackOutcome` with no reaction modifiers reproduces today's behavior (effective AC = originalAC, hit/miss matches phase 1, damage applied if hit).
   - Unit: `ApplyAttackOutcome` with a `ReactionModifier{ACBonus: 5}` retroactively turns a hit into a miss when `roll < originalAC + 5`.
   - Unit: condition handler publishes a `ReactionTriggerEvent` on the bus when its predicate matches AND `gamectx.IsReactionReady` returns true.
   - Unit: condition handler does NOT publish a `ReactionTriggerEvent` when readiness is false (predicate match alone is not enough).
   - Unit: NPC reactor's auto-resolve path applies the modifier inline during phase 1 and does NOT publish a `ReactionTriggerEvent` (no prompt needed for NPCs).
   - Round-trip test: encounter snapshot persists/restores reaction-readiness map.

### Slice 3 — rpg-toolkit OpportunityAttackCondition (port from inline)

1. **New condition type:** `OpportunityAttackCondition` in `rulebooks/dnd5e/conditions/opportunity_attack.go`. Subscribes to `MovementChain`. On every move event:
   - If the moving entity is this combatant, return early (you don't OA your own movement).
   - Check `gamectx.RequireRoom(ctx)` for positions; port `isLeavingThreatRange` (or call it as a shared helper) to decide if this combatant threatens the move's `FromPosition` and not `ToPosition`.
   - If `event.IsOAPrevented()` is true (Disengaging or other prevention source), return early.
   - Check `gamectx.RequireCharacters(ctx).GetCharacterActionEconomy(self).CanUseReaction()` for reaction availability.
   - **Check `gamectx.IsReactionReady(self.CharacterID, refs.Conditions.OpportunityAttack())`.** If not ready → return early (no event published, attack proceeds single-phase). The default-on readiness for melee combatants (seeded in Slice 1) means this typically passes for melee characters.
   - For NPC reactors: invoke the auto-resolve path inline — call `combat.ResolveAttack` with `AttackTypeOpportunity` directly (preserves current NPC behavior), no `ReactionTriggerEvent` published.
   - For player reactors with all checks passing: publish a `ReactionTriggerEvent{ ReactorID: self, ConditionRef: refs.Conditions.OpportunityAttack(), TriggerKind: "movement_oa", SourceEntity: event.MoverID, Payload: <move event details> }` on the encounter bus.

2. **Refactor `combat.MoveEntity`.**
   - Inline `triggerOpportunityAttack` becomes the NPC auto-resolve path inside the OA condition (or stays as the helper the condition calls). The geometry check (`findThreateningEntities`, `isLeavingThreatRange`) stays in `combat.MoveEntity` or moves into a shared helper the condition uses.
   - The end state: `combat.MoveEntity` publishes the movement chain, conditions decide whether to OA, NPC OAs auto-resolve inline, player OAs cause `ReactionTriggerEvent`s on the bus that the orchestrator reads after the move call returns. The move call itself doesn't pause.

3. **Apply on every combatant by default.**
   - As described in Slice 1: every melee-capable player and every melee monster has `OpportunityAttackCondition` Apply()'d at rehydration with `ready: true`. Decision: the condition is **not** persisted in `character.Data.Conditions` (it's a default for all combatants, not a per-character choice); instead, the encounter SDK programmatically Apply()'s it during rehydration. This avoids polluting persisted character data with a condition that's universal.

4. **Tests.**
   - Unit: OA condition auto-fires on a leave-threat-range movement when the threatener is an NPC (auto-resolve path; no `ReactionTriggerEvent` published).
   - Unit: OA condition publishes a `ReactionTriggerEvent` when the threatener is a player reactor with OA readiness on.
   - Unit: OA condition does NOT publish a `ReactionTriggerEvent` (and does not auto-fire) when readiness is off, even if the predicate matches.
   - Unit: OA condition does NOT fire when the mover is Disengaging (the Disengaging condition's `OAPreventionSources` short-circuits the OA condition's eligibility check).
   - Unit: OA condition does NOT fire when reaction is unavailable (action economy says no).
   - Integration: extend the v2 integration test from Slice 1 with `TestSlice_PlayerOAOnNPCMove`: alice's character has the OA condition (default-on); goblin moves out of alice's reach; alice's reactor stream receives an OA prompt; alice takes; OA resolves; goblin's HP drops. And `TestSlice_PlayerOANotPromptedWhenReadinessOff`: alice toggles OA readiness off; goblin moves out of reach; no prompt arrives; goblin completes movement.

### Slice 4 — rpg-toolkit Shield spell as a condition

The canonical proof for player-controlled reactions modifying outcomes via the discrete-phase split.

1. **New condition type:** `ShieldSpellCondition` in `rulebooks/dnd5e/conditions/shield_spell.go`. Subscribes to the phase-1 attack chain (within `ResolveAttackHit`) at the post-roll boundary.
   - On every chain event where this character is the target and the roll has been computed:
     - Check `event.TotalAttack < event.TargetAC` → return early (already a miss).
     - Check `event.TotalAttack >= event.TargetAC + 5` → return early (still hits even with Shield).
     - Check `gamectx.RequireCharacters(ctx).GetCharacterActionEconomy(self).CanUseReaction()`.
     - Check the character has at least one available 1st-level (or higher) spell slot via the existing resource system — ResourceID convention from the rulebook.
     - **Check `gamectx.IsReactionReady(self.CharacterID, refs.Spells.Shield())`.** If not ready → return early. (Default-off for spell-cost reactions; player must toggle "ready Shield" first.)
   - For NPC reactors with Shield (rare in 2.11d but possible — the boss caster): invoke auto-resolve inline — apply the +5 AC modifier to the in-flight attack chain event, consume reaction + slot, no `ReactionTriggerEvent` published.
   - For player reactors with all checks passing: publish a `ReactionTriggerEvent{ ReactorID: self, ConditionRef: refs.Spells.Shield(), TriggerKind: "post_hit", SourceEntity: event.AttackerID, Payload: <relevant attack context> }` on the encounter bus.
   - **Important:** the chain itself does NOT pause. It runs to completion using the original AC (no Shield modifier). Phase 1's `AttackContext` reflects `wouldHit` against the original AC. The Shield modifier is applied in phase 2 if the player takes the reaction.

2. **Phase 2 applies the AC modifier.** `combat.ApplyAttackOutcome` (Slice 2) takes the `[]ReactionModifier` slice; if a `ReactionModifier{ConditionRef: Shield, ACBonus: 5}` is present, effective AC = `attackContext.OriginalAC + 5`, hit re-evaluated. If `event.TotalAttack < effectiveAC` → miss; otherwise hit and proceed to damage.

3. **One-shot readiness clear.** When Shield's reaction-take path runs (either NPC auto-resolve or player Take), after consuming the reaction + spell slot, the condition handler also clears `gamectx.SetReactionReady(self.CharacterID, refs.Spells.Shield(), false)` (or signals to the orchestrator to do so via the consumption metadata on the modifier — implementer's call). Spell-cost reactions are one-shot per readiness toggle; the player must re-ready Shield to use it again. Free reactions (OA) stay ready until canceled.

4. **Refs + serialization.** Add `refs.Spells.Shield()`. Implement `ToJSON` / `loadJSON` / register in `conditions/loader.go`. This condition IS persisted on a wizard's character (it represents "I have Shield prepared"). The readiness flag is NOT persisted on the condition; it lives on the encounter's readiness map.

5. **Tests.**
   - Unit: Shield publishes a `ReactionTriggerEvent` when readiness is on, the attack would hit, and Shield would deflect.
   - Unit: Shield does NOT publish (and chain proceeds inline) when readiness is off, even if all other predicates match.
   - Unit: Shield does NOT trigger if the character has no reaction available.
   - Unit: Shield does NOT trigger if no 1st-level slot remains.
   - Unit: Shield does NOT trigger if the attack already misses without it.
   - Unit: Shield does NOT trigger if the attack would still hit even with +5 AC (don't waste the reaction).
   - Unit: phase 2 with Shield modifier present + originalAC + roll within the [originalAC, originalAC+5) range produces a miss; the same roll without the modifier produces a hit.
   - Unit: Shield's auto-clear path turns readiness off after firing.
   - Integration: extend v2 integration suite with `TestSlice_PlayerShieldsAttack`: bob (wizard with Shield Apply()'d + 1st-level slot + Shield readied) is targeted by goblin's attack with a hit-but-deflectable roll; phase-1 `ReactionTriggerEvent` arrives on bob's stream as a prompt; bob takes; phase 2 applies modifier; attack misses; reaction + slot consumed; bob's Shield readiness auto-cleared.

### Slice 5 — rpg-api reaction orchestrator + readiness handler + per-viewer projection

1. **Server-as-orchestrator for two-phase attacks.** The handler for `TakeAction(attack)` (and `MoveEntity` for OA-triggering moves) follows this pattern:
   - Invoke phase 1: `Encounter.ResolveAttackHit(attackInput)` returns `(*AttackContext, []ReactionTriggerEvent, error)`.
   - **If `len(triggerEvents) == 0`**: invoke phase 2 inline: `Encounter.ApplyAttackOutcome(attackContext, nil)` → final result. Return to caller normally. This is the common case (no readied reactions match).
   - **If trigger events present**: for each event:
     - Resolve the reactor playerID from the entity ID via the encounter's player → entity mapping.
     - Translate the event into an `EncounterEvent_InputRequired{reaction_prompt: {...}}` per the additive proto change.
     - Per-viewer project: visible only to the reactor.
     - Record `pendingPrompts[reactorPlayerID] = { attackContext, pendingTriggers, ... }` so `SubmitCheck` knows what to do next.
     - Persist the encounter snapshot (carrying the in-flight phase-1 `AttackContext` + outstanding pending reactions). The snapshot is the resume-state — exactly the persistence shape that already exists.
   - Return to the caller with the phase-1 result on the wire response and the prompts on the reactor stream(s). The caller's RPC does NOT block; it returns and the reactor responds asynchronously.

2. **`SubmitCheck` invokes phase 2.** When the reactor's `SubmitCheck{take_reaction: true|false}` arrives:
   - Look up `pendingPrompts[reactorPlayerID]` to find the in-flight `AttackContext` and the matching `ReactionTriggerEvent`.
   - If take: build a `ReactionModifier` from the event (Shield → `{ACBonus: 5, consume: {reaction, slot1}}`); consume reaction + spell slot via the encounter SDK; if Shield, clear readiness for that reaction (one-shot).
   - If skip: build no modifier.
   - Invoke phase 2: `Encounter.ApplyAttackOutcome(attackContext, modifiers)` → final result.
   - Publish the phase-2 events (`EntityDamaged` if hit, miss event if Shield deflected).
   - Clear `pendingPrompts[reactorPlayerID]`.
   - If multiple reactors had pending prompts on the same attack (rare in 2.11d but possible), wait for all responses before invoking phase 2 with the full modifier list. Implementer picks the resolution order if 2.11d hits this case; multi-reactor polish is otherwise deferred.

3. **`SetReactionReady` handler.** New RPC handler:
   - Validates the request (encounter exists, character belongs to the requesting player, reaction_ref is known).
   - Updates the encounter's readiness map (`encounter.ReactionReadiness[charID][ref] = ready`).
   - Persists the snapshot.
   - Publishes a confirmation event on the stream so the reactor's UI reflects the new readiness state. (Optional broadcast vs reactor-only — implementer's call; broadcast is fine if it's just a UI hint.)

4. **`pendingPrompts` keyed by reactor playerID.** Existing `pendingPrompts` map (Wave 2.9) extended to support reaction-keyed entries. Wave 2.9's keyed-by-caller pattern stays for skill checks; this adds a parallel keyed-by-reactor entry shape (or unifies — implementer's call). The map value carries enough state to invoke phase 2 on resume (the `AttackContext`, the trigger event, the original caller for tracing).

5. **NPC dispatch loop guard.** When an NPC's action triggers a player reaction prompt (e.g., NPC moves out of player reach, player has OA readied), the NPC dispatch loop must:
   - Detect `pendingPrompts[reactorPlayerID]` was set by phase 1.
   - Pause the NPC dispatch loop, return from `EndTurn` with the current state.
   - On the reactor's `SubmitCheck` resume + phase-2 completion, the NPC dispatch loop continues from where it stopped.
   - For NPC reactions (auto-fire), no pause needed — phase 1's auto-resolve path applies the modifier inline and phase 2 runs immediately.

6. **Tests.**
   - Translator unit tests for `ReactionTriggerEvent` → `EncounterEvent_InputRequired{reaction_prompt}` per-viewer projection (visible only to reactor).
   - Unit test: `TakeAction` with no readied reactions on either side runs phase 1 + phase 2 inline (single-RPC behavior matches today's monolithic call).
   - Unit test: `TakeAction` against a reactor with readied Shield runs phase 1, returns to caller, pushes prompt to reactor; reactor's `SubmitCheck` invokes phase 2.
   - Unit test: `SetReactionReady` updates the readiness map and persists the snapshot.
   - Integration: `TestIntegration_PlayerOAReactionPromptedAndResolved`: alice's character has OA condition with default-on readiness; goblin moves out of reach; alice's stream receives `InputRequired{reaction_prompt}`; alice's `SubmitCheck{take_reaction: true}` triggers phase-2 OA resolution; goblin's HP drops; alice's reactions_remaining decrements.
   - Integration: `TestIntegration_PlayerOAReactionSkipped`: alice clicks Skip; phase 2 runs without OA; goblin completes movement.
   - Integration: `TestIntegration_PlayerOANotPromptedWhenReadinessOff`: alice toggles OA readiness off via `SetReactionReady`; goblin moves; no prompt; goblin completes movement single-phase.
   - Integration: `TestIntegration_PlayerShieldReactionPromptedAndResolved`: bob readies Shield; goblin attack would hit; phase 1 publishes trigger event; bob's stream receives prompt; bob's `SubmitCheck{take_reaction: true}` triggers phase 2 with Shield modifier; attack misses; reaction + slot consumed; bob's Shield readiness auto-cleared.
   - Integration: `TestIntegration_PlayerShieldNotPromptedWhenReadinessOff`: bob has Shield Apply()'d but readiness is off (default); goblin attacks with hit-but-deflectable roll; no prompt arrives; attack hits inline.
   - Integration: `TestIntegration_NPCMultiattack_PerStrikeEvents`: seed multiattack monster; both streams receive N separate `EntityDamaged` events for N strikes.
   - Integration: `TestIntegration_NPCDispatchPausesOnPlayerReaction`: NPC's move triggers player's OA prompt; NPC dispatch loop pauses; reactor's SubmitCheck resumes; NPC turn continues.

### Slice 6 — rpg-api-protos additive `reaction_prompt` oneof variant + `SetReactionReady` RPC

1. **Verify** existing `InputRequired` / `SubmitCheck` shape covers the wave's needs (likely yes — it was designed extensibly in 2.9).
2. **Add** `ReactionPrompt` message + add as a new oneof variant in `InputRequired`:
   ```proto
   message ReactionPrompt {
     core.Ref reaction_ref = 1;            // dnd5e:conditions:opportunity_attack, dnd5e:spells:shield, etc.
     string trigger_kind = 2;              // "movement_oa", "post_hit", "post_damage"
     string trigger_source_entity_id = 3;  // who/what triggered (the attacker for Shield, the mover for OA)
     string target_entity_id = 4;          // the reaction's intended target
     string display_text = 5;              // optional pre-formatted UI string
   }
   ```
3. **Add** the additive field to `SubmitCheckRequest`:
   ```proto
   optional bool take_reaction = N;  // when set on a reaction_prompt response, true = take, false = skip
   ```
4. **Add new RPC** `SetReactionReady`:
   ```proto
   rpc SetReactionReady(SetReactionReadyRequest) returns (SetReactionReadyResponse);

   message SetReactionReadyRequest {
     string encounter_id = 1;
     string character_id = 2;
     core.Ref reaction_ref = 3;  // which reaction to toggle
     bool ready = 4;              // true = ready, false = unready
   }

   message SetReactionReadyResponse {
     // Empty for now; success implied by no error. Future: return resulting readiness state for confirmation.
   }
   ```
5. **Buf-managed regenerate**, push to `generated` branch as usual.
6. **Tag a release** (e.g. `v0.1.95`).

### Slice 7 — rpg-dnd5e-web reaction modal + readiness panel

1. **Reaction prompt modal.** Reuse Wave 2.9's prompt-modal infrastructure. Switch case for `pendingPrompt.kind === "reaction"`:
   - Render trigger description from `display_text` (or compose from `trigger_kind` + `trigger_source_entity_id` if `display_text` is empty):
     - For OA: "Goblin-1 is leaving your reach. Take opportunity attack?"
     - For Shield: "Goblin-1 attacks you (rolled 19 vs AC 14). Cast Shield (+5 AC, 1st-level slot)?"
   - Render two buttons: "Take" + "Skip".
   - Take/Skip dispatch via `useSubmitCheckV2` with the `take_reaction` semantic from the protos additive change. Take = `take_reaction: true`; Skip = `take_reaction: false`. Modal clears on response.
   - Per-viewer routing: the prompt arrives only on the reactor's stream (server-side per-viewer projection). Web does not need to filter.

2. **Ready-reactions panel (NEW).** A panel adjacent to the active character's UI listing reactions the character can ready, with a toggle per reaction:
   - For each known reaction the character has Apply()'d (OA, Shield, Counterspell, etc.), render a row with:
     - Reaction name + cost indicator ("Free" for OA, "1st-level slot" for Shield).
     - Toggle showing current readiness (on/off).
     - Brief one-line description ("React when an enemy leaves your reach", "React to cast +5 AC when hit").
   - Toggle dispatches `useSetReactionReady` (new hook backing the new RPC) with `{character_id, reaction_ref, ready: !current}`.
   - Default-state hint: OA shows ON by default for melee combatants; Shield shows OFF until toggled. The web reflects whatever the snapshot says (server is source of truth).
   - One-shot reactions auto-clear after firing — the panel re-renders the toggle as OFF after Shield fires (the snapshot will reflect this).

3. **Bump proto pin** to `@v0.1.95`.

4. **Bump rpg-api pin** as needed for the integration to work.

5. **Tests.**
   - Reducer test: incoming `EncounterEvent_InputRequired{reaction_prompt}` populates `pendingPrompt` with `kind: "reaction"`.
   - Hook test: Take button dispatches `useSubmitCheckV2` with `take_reaction: true`; Skip with `take_reaction: false`.
   - Hook test: ready-reactions toggle dispatches `useSetReactionReady` with the right ref + ready flag.
   - Harness test: modal renders the trigger description + Take + Skip buttons; click triggers correct dispatch.
   - Harness test: ready-reactions panel renders rows for OA + Shield; toggling Shield dispatches the readiness RPC; subsequent attack on the wizard with a hit-but-deflectable roll triggers a Shield prompt; toggling Shield off again means the same attack pattern produces no prompt.

### Slice 8 — Multiattack per-strike audit

1. **Verify** `Dnd5eCombatResolver.ResolveAttack` (Wave 2.11b) emits a separate `EntityDamaged` per multiattack strike when a multiattack monster's `monster.TakeTurn` fires multiple `ResolveAttack` calls. The translator should not aggregate.
2. **If aggregation found:** fix to publish per-strike. Add an integration test: `TestIntegration_NPCMultiattack_PerStrikeEvents`.
3. **No new toolkit work required** if the per-strike publication path from 2.11b's resolver swap is intact. This slice is verification + a small fix at most.

### Slice 9 — chore: Wave 2.11 close-the-loop (rpg-project)

Standard close-the-loop: file followups (Sentinel / Polearm Master / Counterspell / Hellish Rebuke / Uncanny Dodge as separate condition implementations on the same primitive + opt-in readiness), update board, draft Wave 2.11e (AvailableActions surfacing) plan if not already split, capture verified patterns into role context, retro on tracker.

## Out of scope for Wave 2.11

- **Sentinel / Polearm Master / Mage Slayer / Counterspell / Hellish Rebuke / Uncanny Dodge.** Each is a 1-week slice on the same condition pattern. File as 2.11 followups (separate issues on board #11 with `Wave 2.11 followup` label).
- **`AvailableAction` enumeration and `TurnState.available_actions` emission.** Decoupled to its own wave (2.11e proposed). Reactions can ship without dragging the class-action surface scope along.
- **Multi-reactor ordering.** If two reactors are eligible at the same window, the deterministic-first one resolves; the rest see "no longer eligible." Multi-reaction polish (priority? offer all reactors? player chooses order?) is a future wave.
- **Reaction timeouts on player prompts.** Sit forever until response.
- **NPC strategic reaction decisions.** NPCs always Take available reactions. Future polish: per-NPC AI hints ("save reaction for X").
- **Auto-take preference for player OAs.** All OAs prompt in 2.11. Future polish: per-character "always take OAs" config.
- **Spell slot UI.** Shield consumes a slot, but the slot UI in the harness is minimal (informational only — "you have N 1st-level slots"). Full spell-management UI is a future spell-system wave.
- **TPK / player dying-state / death saves.** Same as Wave 2.10 — a player at HP=0 still emits `EntityDied` but isn't removed from initiative. Death-save mechanics are a separate wave.
- **LobbyView migration.** Wave 5. Harness is the sole UI for 2.11.

## Forward-loaded patterns (anticipated — refine in close-the-loop)

**rpg-toolkit-member additions:**

- `pat-encounter-persistent-bus` *(anticipated)* — The encounter holds a single `*events.EventBus` for its lifetime. Conditions Apply()'d during character/monster rehydration stay subscribed across all attacks/moves/turns within the encounter. State they hold (Sneak Attack's UsedThisTurn, action-economy mutations) persists. The handler does NOT construct a fresh bus per-attack.
- `pat-encounter-gamectx-wired` *(anticipated)* — `combat.ResolveAttack` runs with `gamectx.WithGameContext(ctx, gctx)` populated with a CharacterRegistry covering all encounter combatants, a Room reference, and the encounter's reaction-readiness map. Conditions calling `gamectx.RequireRoom(ctx)` / `gamectx.RequireCharacters(ctx)` / `gamectx.IsReactionReady(charID, ref)` find the state they need.
- `pat-discrete-attack-phases` *(anticipated)* — `combat.ResolveAttack` is split into `ResolveAttackHit` (phase 1: chain runs end-to-end, returns `AttackContext`) and `ApplyAttackOutcome` (phase 2: chain runs end-to-end with reaction modifiers, returns final result). Each phase is atomic. The "pause" lives at the RPC boundary, not inside the chain. The chain primitive does NOT have pause-resume capability — that complexity moves out of the toolkit.
- `pat-reaction-trigger-event` *(anticipated)* — When a condition handler's predicate matches AND `gamectx.IsReactionReady` returns true, the handler publishes a `ReactionTriggerEvent` on the encounter bus. The chain runs to completion either way; the event is just additional output the orchestrator reads after the chain returns. NPC reactors skip the event and apply the modifier inline (auto-resolve); player reactors with readiness on emit the event.
- `pat-condition-as-reaction-window-subscriber` *(anticipated)* — Reactions are conditions that subscribe to a chain at the right window. The condition's handler publishes a `ReactionTriggerEvent` when a player reactor is eligible AND has readied the reaction; applies the modification directly via auto-resolve when an NPC reactor is eligible.
- `pat-opt-in-reaction-readiness` *(anticipated)* — Reactions are opt-in via per-character readiness flags in the encounter snapshot. Default readiness varies by reaction type: free-cost (OA) default-on for melee combatants, spell-cost (Shield) default-off, class-feature reactions per-feature. One-shot reactions auto-clear after firing; persistent reactions stay ready until canceled. Conditions check `gamectx.IsReactionReady(charID, ref)` before publishing a `ReactionTriggerEvent`. This trades RAW 5e flexibility for "no constant prompt mashing" — the right call for asynchronous multiplayer without a DM.
- `pat-opportunity-attack-as-condition` *(anticipated)* — Every melee combatant has `OpportunityAttackCondition` Apply()'d with `ready: true` by default. The condition subscribes to `MovementChain`, checks geometry + reaction availability + `gamectx.IsReactionReady` via gamectx, publishes a `ReactionTriggerEvent` for player reactors or auto-resolves inline for NPC reactors. `combat.MoveEntity` no longer triggers OAs inline — it publishes the movement chain and lets the condition decide.

**rpg-api-member additions:**

- `pat-v2-encounter-bus-singleton` *(anticipated)* — The handler holds the encounter's `*events.EventBus` per-encounter (not per-request). All resolver / move calls within the same encounter use the same bus. `Dnd5eCombatResolver` is constructed against this bus, not against `events.NewEventBus()` per attack.
- `pat-v2-server-as-orchestrator` *(anticipated)* — The handler is the explicit orchestrator for two-phase attacks. Invoke phase 1 → scan returned `ReactionTriggerEvent`s → if none, invoke phase 2 inline (single-phase common case); if events present, push prompts to reactors and persist `pendingPrompts[reactorPlayerID]` carrying the in-flight `AttackContext` for resume. There is no DM in this game; the server fills that role.
- `pat-v2-reaction-prompt-stream-event` *(anticipated, deliberate evolution of pat-v2-prompt-issuing-rpc-shape)* — Reaction prompts ride the per-viewer event stream (NEW event variant: `EncounterEvent_InputRequired{reaction_prompt}`), projected to the reactor only. The mover/attacker's RPC response returns normally with phase-1 results; the reactor's stream gets the prompt as an additional event. `pendingPrompts[reactorPlayerID]` (not caller). `SubmitCheck{take_reaction}` from the reactor invokes phase 2 with the player's choice baked in.
- `pat-v2-set-reaction-ready-rpc` *(anticipated)* — A dedicated RPC (`SetReactionReady{character_id, reaction_ref, ready: bool}`) toggles per-character readiness flags on the encounter. Persisted on the snapshot. Read by gamectx via `IsReactionReady`. The web exposes this as a "ready reactions" panel on the active character's UI.
- `pat-v2-npc-dispatch-pause-on-reaction` *(anticipated)* — NPC dispatch loop pauses when an NPC's action triggers a player reaction prompt (phase 1 produced a `ReactionTriggerEvent` for a player reactor). Same shape as Wave 2.9's "pause for skill check"; resumes on the reactor's `SubmitCheck` invoking phase 2.

**rpg-dnd5e-web-member additions:**

- `pat-v2-reaction-prompt-modal` *(anticipated)* — `pendingPrompt.kind === "reaction"` switch case in the prompt modal. Renders trigger description + Take / Skip buttons. Take/Skip dispatches via `useSubmitCheckV2` with the take_reaction semantic. Modal clears on response.
- `pat-v2-ready-reactions-panel` *(anticipated)* — A panel adjacent to the active character's UI listing reactions the character has Apply()'d, each with a toggle dispatching `useSetReactionReady`. Default OA on for melee combatants; default Shield off. One-shot reactions render OFF after firing (the snapshot reflects the auto-clear). The panel is the player-facing surface for the opt-in readiness model.

After execution, the close-the-loop dispatch promotes verified `anticipated` entries to status-less (validated) and adds any new patterns that surfaced.

## Inner-issue list (filed when this kickoff package is published)

If split into 2.11c + 2.11d (recommended):

- **🎮 Wave 2.11c: Foundation + discrete attack phases** (rpg-project tracker #46)
- **rpg-toolkit #647 (2.11c slice 1)**: encounter-scoped bus + gamectx wiring + broadened registry + reaction-readiness map — folds Wave 2.11a/b cleanup into the foundation. Tag `encounter/v0.8.0`.
- **rpg-toolkit #648 (2.11c slice 2)**: split `combat.ResolveAttack` into `ResolveAttackHit` + `ApplyAttackOutcome` discrete operations + `ReactionTriggerEvent` published by condition handlers when predicate matches AND `gamectx.IsReactionReady`. NO chain pause-resume primitive — chain runs end-to-end per phase. Tag `rulebooks/dnd5e@v0.56.0`.
- **rpg-api #530 (2.11c slice 3)**: bump toolkit pins; encounter-scoped bus on the handler; gamectx wiring through `Dnd5eCombatResolver` (room + character registry + reaction-readiness map). Integration test: Sneak Attack persists UsedThisTurn across attacks; phase-1+phase-2 inline produces same results as today's monolithic call when no reactions are ready.
- **chore: Wave 2.11c close-the-loop** (rpg-project)

- **🎮 Wave 2.11d: OA-as-condition + Shield + opt-in reactions** (rpg-project tracker #47)
- **rpg-toolkit #649 (2.11d slice 1)**: `OpportunityAttackCondition` ports `combat.MoveEntity`'s OA trigger into the condition pattern. Default-Apply for every melee combatant with `ready: true`. Condition handler checks `gamectx.IsReactionReady` before publishing `ReactionTriggerEvent`.
- **rpg-toolkit #650 (2.11d slice 2)**: `ShieldSpellCondition` subscribes to post-hit, publishes `ReactionTriggerEvent` when readied + would-deflect; phase 2 applies AC modifier on take. Default readiness off; auto-clears after firing (one-shot).
- **rpg-api-protos #156 (2.11d)**: additive `reaction_prompt` oneof variant in `InputRequired` + `SubmitCheck.take_reaction` field + new `SetReactionReady` RPC. Tag `v0.1.95`.
- **rpg-api #531 (2.11d)**: server-as-orchestrator (phase 1 → scan trigger events → push prompts → phase 2 on response); reaction prompt translator + per-viewer projection; `pendingPrompts[reactorPlayerID]`; `SubmitCheck` invokes phase 2; `SetReactionReady` handler; NPC-dispatch-pause-on-reaction. Integration tests: player OA, player OA opt-out, player Shield, player Shield not-readied default.
- **rpg-dnd5e-web #407 (2.11d)**: reaction modal (Take/Skip) + ready-reactions panel (toggles backed by `useSetReactionReady`) + harness wiring.
- **rpg-toolkit (2.11d slice 3, optional)**: multiattack per-strike audit if the verification surfaces aggregation.
- **chore: Wave 2.11d close-the-loop** (rpg-project): file followups for Sentinel / Polearm Master / Counterspell / Hellish Rebuke / Uncanny Dodge as separate per-condition issues; draft 2.11e (AvailableActions surfacing).

If kept as a single tracker (2.11) — same slices, all one tracker, one chore.

## Execution sequence

1. **Wave 2.11b close-the-loop** (chore #45, in flight) completes before 2.11c dispatches. That step writes verified Wave 2.11b patterns into role context; this wave's dispatches read those.
2. **rpg-toolkit (2.11c slice 1) dispatches first.** Foundation: encounter-scoped bus + gamectx + readiness map. Smallest blast radius. Tag `encounter/v0.8.0`.
3. **rpg-toolkit (2.11c slice 2) dispatches second.** Discrete attack phases (`ResolveAttackHit` + `ApplyAttackOutcome`) + `ReactionTriggerEvent`. Tag `rulebooks/dnd5e@v0.56.0`.
4. **rpg-api (2.11c slice 3) dispatches third.** Bumps toolkit pins; verifies Sneak Attack works through v2 + the discrete-phase split runs correctly inline; integration test.
5. **chore: 2.11c close-the-loop** dispatches; 2.11d kickoff package files at this step.
6. **rpg-toolkit (2.11d slice 1: OA-as-condition) dispatches.** Condition handler checks readiness before publishing trigger.
7. **rpg-toolkit (2.11d slice 2: Shield) dispatches.** Parallel-safe with OA slice; both are condition implementations on the same primitive + same readiness check.
8. **rpg-api-protos (2.11d) dispatches.** Additive proto change: `reaction_prompt` oneof + `SubmitCheck.take_reaction` + `SetReactionReady` RPC.
9. **rpg-api (2.11d) dispatches.** Bumps protos + toolkit pins; server-as-orchestrator for two-phase attacks; reaction translator; readiness handler; NPC-dispatch-pause-on-reaction.
10. **rpg-dnd5e-web (2.11d) dispatches.** Reaction modal + ready-reactions panel + harness wiring.
11. **End-to-end playtest** per the wave goal sentence.
12. **chore: 2.11d close-the-loop** dispatches.

## Reference

- Wave 2.11 trackers (filed by this kickoff dispatch):
  - rpg-project#46: `🎮 Wave 2.11c: Foundation + three-phase chain`
  - rpg-project#47: `🎮 Wave 2.11d: OA-as-condition + Shield + player reactions`
  - (or single tracker if not split — fill in once filed)
- Master plan: `rpg-project/ideas/encounter/v1alpha2/plan.md`
- Roadmap: `rpg-project/ideas/encounter/v1alpha2/roadmap.md`
- Design: `rpg-project/ideas/encounter/v1alpha2/design.md`
- Superseded plan: `rpg-project/ideas/encounter/v1alpha2/plans/09-wave-2.11-combat-depth.md`
- Wave 2.11b plan: `rpg-project/ideas/encounter/v1alpha2/plans/10-wave-2.11b-real-combat-chain.md`
- Wave 2.10 plan: `rpg-project/ideas/encounter/v1alpha2/plans/08-wave-2.10-death-resolution.md`
- **ADR-0027 (Accepted 2026-05-10)**: `rpg-toolkit/docs/adr/0027-attack-resolution-and-reactions.md`
- **ADR-0025 (Accepted)**: `rpg-toolkit/docs/adr/0025-gamectx-pattern.md`
- Toolkit references (verify pinned versions at dispatch):
  - `rulebooks/dnd5e/conditions/sneak_attack.go` — canonical chain-subscriber pattern with `gamectx.RequireRoom(ctx)`
  - `rulebooks/dnd5e/conditions/fighting_style_protection.go` — canonical chain-subscriber pattern with `gamectx.RequireCharacters(ctx)` + `ReactionsConsumed`
  - `rulebooks/dnd5e/conditions/disengaging.go` — canonical `MovementChain` subscriber with `OAPreventionSources`
  - `rulebooks/dnd5e/combat/attack.go` — `combat.ResolveAttack` chain pattern (`PublishWithChain` + `Execute`); `ReactionsConsumed` consumption
  - `rulebooks/dnd5e/combat/movement.go` — `combat.MoveEntity` + `triggerOpportunityAttack` (target of the OA-port slice)
  - `rulebooks/dnd5e/character/data.go:208-229` — `LoadFromData` condition Apply loop (the rehydration seam)
  - `rulebooks/dnd5e/gamectx/` — package surface (registry interfaces, require helpers)
- rpg-api references:
  - `internal/handlers/dnd5e/v2/encounter/dnd5e_combat_resolver.go` — current per-attack bus + missing gamectx (foundation slice fixes both)
  - `internal/handlers/dnd5e/v2/encounter/combatant.go` — current `CombatantRegistry` (per-attack scope; gamectx CharacterRegistry needs broader scope)
- Board: https://github.com/users/KirkDiggler/projects/11

## Sign-off criterion

Wave 2.11 (or 2.11d if split) is done when, in a cold-start two-browser playtest:

- alice (rogue) + bob (wizard) connect to a freshly-created TURN_BASED encounter.
- alice attacks the goblin while bob is adjacent to the goblin; the published `EntityDamaged` reflects sneak attack damage. The attack runs single-phase (no readied reactions match).
- alice attacks twice in one turn (Extra Attack); only one strike includes sneak attack damage; on her next turn, the first eligible strike includes sneak attack again (UsedThisTurn persisted + reset correctly).
- alice has OA readied by default (default-on for melee combatants). A goblin moves out of alice's reach; alice's stream receives `InputRequired{reaction_prompt}` for OA; alice clicks Take in the harness; phase 2 dispatches an OA (`EntityDamaged` against goblin); alice's `economy.reactions_remaining` decrements.
- alice toggles OA readiness off via the ready-reactions panel; another goblin moves out of her reach; no prompt arrives; goblin completes movement single-phase.
- bob has NOT toggled "ready Shield" by default. A goblin attacks bob with a roll that would hit; the attack runs single-phase, hits, damage applies. No prompt arrives.
- bob toggles "ready Shield" via the ready-reactions panel. A goblin attacks bob with a roll that would hit (and that Shield would deflect); bob's stream receives `InputRequired{reaction_prompt}` for Shield; bob clicks Take; phase 2 applies the +5 AC modifier; attack misses; bob's `economy.reactions_remaining` decrements + 1st-level slot is consumed; bob's Shield readiness auto-clears (panel reflects OFF).
- All Wave 2.10 + 2.11a/b sign-off behavior continues to pass (kill-one / kill-last / `EncounterEnded`; real `combat.ResolveAttack` still computes damage from equipped weapons; NPCAct still fires through the resolver).

When the close-the-loop runs, the playtest video / screenshots become the verification artifact and the wave ships.
