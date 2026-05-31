# Design: rpg-toolkit#689 — `Encounter.LoadFromData` owns combatant hydration via the cascade

**Status:** PROPOSED — pending Kirk sign-off. Design-first; no code until ratified.
**Authored by:** toolkit team-member (design pass), reframed per Kirk's "hydration = the existing `ToData`/`LoadFromData` round-trip" steer.
**Director-verified core mechanism:** `character.LoadFromData` (`rulebooks/dnd5e/character/data.go:119`) → `conditions.LoadJSON` (`:223`) → `condition.Apply(ctx, bus)` (`:231`). That single `Apply` is the subscribe point; doing it twice on one bus is the #684 double-subscribe. OA/Shield are in `conditions.LoadJSON` (`loader.go:149,156`). Both confirmed by reading the code on `main`.
**Unblocks:** rpg-api#582 (clean encounter orchestrator). Part of umbrella rpg-api#574.

---

## Executive summary

1. Hydration is **not** a new subsystem — it is the toolkit's existing `ToData`/`LoadFromData` round-trip, which **composes**. `Encounter.LoadFromData(ctx, …)` cascades into each combatant's `character.LoadFromData` / `monster.LoadFromData`, **holds the runtime entities** on the `Encounter`, and `ToData` cascades back. That single cascade is the one place conditions `Apply` to `e.bus` — the subscribe-exactly-once cure for #684.
2. The combat/movement resolvers stop receiving "IDs to re-load from" and receive the **already-held** attacker/defender as `combat.Combatant` handles; they build the cheap per-attack registry/gamectx and run the chain with **zero re-load, zero re-subscribe** — deleting the host's `resolveEntity`/`loadCharacterWithBus` and the `pendingPhasedAttack` cache hack.
3. Reaction conditions (OA/Shield) ride the cascade like every other condition — already in `conditions.LoadJSON`'s ref switch (`loader.go:149-159`), driven by the SDK-owned `ReactionReadiness` — deleting rpg-api's `conditions.New*` construction in `reaction_conditions.go`.
4. `EndTurn(ctx, …)` emits the dnd5e turn-boundary **directly on `e.bus`** (`dnd5eEvents.TurnEndTopic`) — no pluggable signaler; the SDK already imports `dnd5eEvents` (`encounter.go:13`), matching the existing one-liner, deleting the host's re-loading `publishTurnEndAndPersistReset` + its `defer Cleanup` #684 patch.
5. Clean break: `ctx` on `LoadFromData`/`EndTurn`, resolvers take held entities. Diverges from the consumer's `WithEntityHydrator` (no injected hydrator — the cascade is the mechanism) and drops the v1 `TurnSignaler` (needless ceremony given `encounter.go:13`); keeps the consumer's "resolver takes the held entity" + "delete the host's `conditions.New*`" outcomes.

---

## Verified this pass (code wins over docs)

- **The cascade already exists and is the cure.** `character.LoadFromData` (`character/data.go:119`) cascades into `conditions.LoadJSON` (`conditions/loader.go:18`, ref-routed switch) and calls `condition.Apply(ctx, bus)` per condition (`character/data.go:230-237`). #684 fires because the host calls `character.LoadFromData` on the *same* `e.bus` twice (resolver `loadCharacterWithBus` at `dnd5e_combat_resolver.go:516` + `publishTurnEndAndPersistReset` at `end_turn.go:374`). One owner, one load → cured.
- **OA + Shield are already in the loader switch:** `loader.go:149` `OpportunityAttack().ID`, `:156` `Spells.Shield().ID`. They reconstitute via the normal condition cascade — no special construction — if present in the entity's condition list (driven by readiness). Refs: `refs/conditions.go:102`, `refs/spells.go:256`.
- **Monsters do NOT auto-cascade conditions in `monster.LoadFromData`** — by design, caller-applied (`monster.go:350-353` shows the commented `monstertraits.LoadMonsterConditions(...)` call the caller is meant to make; `monster.go:43-46`). **The helper already exists** at `rulebooks/dnd5e/monstertraits/loader.go:84` — used by `monster/monsters/zombie.go` + the skeleton/zombie tests. So the cascade **wires in the existing `monstertraits.LoadMonsterConditions`** for monsters — **no new rulebook helper needed** (corrects a v2-draft claim that no helper existed; caught in review). `monster/actions.LoadMonsterActions` (`monster/actions/loader.go:114`) is already called at `npc.go:107`.
- **ADR-0027 confirmed** (`docs/adr/0027:54-57`): `combat.AttackContext` is JSON-serializable; eventBus/roller no longer in the context; the orchestrator persists it across the player-reaction RPC gap. The phased-flow reasoning relies on exactly this.
- **`encounter.go:13` already imports `dnd5eEvents`**, and `npc.go`/`activate_feature.go` already call `monster.LoadFromData`/`character.LoadFromData` inside the SDK — so cascading there is consistent with shipped precedent, not a new boundary breach.

---

## 0. Boundary stance (Kirk's call, recorded)

The SDK-side cascade through the rulebook's `LoadFromData` is **consistent** with the existing precedent (`npc.go:103` `monster.LoadFromData`; `activate_feature.go:100` `character.LoadFromData`; `encounter.go:13` `dnd5eEvents`). We will **not contort the new path to stay dnd5e-free at the cost of inconsistency.** "Fully agnostic engine" remains separately tracked, not a #689 blocker. Honest status the docs must reflect: the `encounter` SDK is **dnd5e-coupled today** (event vocabulary + loaders), and #689 makes that coupling *coherent and single-sourced* rather than scattered across the host.

---

## 1. Hydration ownership — the cascade

### What the `Encounter` holds (new fields, `encounter.go:32-49`)

```go
// Reconstructed (not serialized) at each LoadFromData — exactly like `bus`.
combatants map[encountercore.EntityID]combat.Combatant   // held runtime entities
```

`combat.Combatant` (`combat/combatant.go:54`) is the shared interface both `*character.Character` and `*monster.Monster` satisfy and is what the resolver chain looks up via `CombatantLookup`. Consistent with the SDK already importing `rulebooks/dnd5e/combat` (`npc.go:13`).

### The cascade in `LoadFromData` (`encounter.go:164-199`)

Signature change: `LoadFromData(ctx context.Context, data *Data, b *Broker, opts ...Option)`. After the maps + `e.bus` are set up:

```go
for _, pd := range e.data.Players {
    char, err := e.hydratePlayer(ctx, pd)   // character.LoadFromData(ctx, charData, e.bus)
    e.combatants[pd.EntityID] = char
}
for _, md := range e.data.Monsters {
    mon, err := e.hydrateMonster(ctx, md)    // monster.LoadFromData + LoadMonsterActions + conditions
    e.combatants[md.ID] = mon
}
```

- **Players** need the serialized char blob. `PlayerData` has none today (`data.go:101-113`). Add `PlayerData.DataJSON json.RawMessage` (`omitempty`), populated by the host — mirrors `MonsterData.DataJSON` (`data.go:161`) and `ActivateFeatureInput.CharDataJSON` (`activate_feature.go:30-36`). Keeps the SDK out of the store (no load-by-ID behind the seam).
- **Monsters:** `monster.LoadFromData(ctx, &data, e.bus)` (`monster/monster.go:304`) + `monsteractions.LoadMonsterActions` (already at `npc.go:107`) + the existing `monstertraits.LoadMonsterConditions` (`monstertraits/loader.go:84`) for the conditions. The `syncMonsterDataFromSnapshot` call (`npc.go:102`) folds in here.
- **Reaction conditions (OA/Shield)** applied as part of this cascade, driven by `e.data.ReactionReadiness[id]` — §4.

This is the **single subscribe point.** One `Encounter.LoadFromData` per RPC + a fresh `Encounter` per request ⇒ "once per encounter object" == "once per request" == the cure.

> **Model note (turn-based now; learn into a real-time server later).** This cure rests on the **simple turn-based, stateless-per-RPC** model: load → verb → persist, with the bus reconstructed fresh each `LoadFromData` (`encounter.go:31` "not serialized"). That's the right simplicity while we set up the building. A future "proper game server" could keep the encounter + bus **in memory across actions** (real-time); then the invariant shifts from "fresh per request" to "subscribe-once for the encounter's in-memory lifetime" — a learned-into evolution (pairs with the persistent-in-mem / Redis-transport direction), explicitly **not now**.

### `ToData` cascading back (`encounter.go:339`)

Today `ToData()` is literally `return e.data` — it hands back the load-time snapshot pointer, not current held-entity state. **That changes:** with held entities that mutate (e.g. `SneakAttack.UsedThisTurn`), `ToData` cascades the held combatants' `ToData()` back into `PlayerData.DataJSON` / `MonsterData.DataJSON`, **gated by the existing per-entity dirty flag** (`character.IsDirty()`/`MarkClean()`, `character.go:744-753`; `monster.go:59`) so only changed entities re-serialize. This **replaces** the host's scattered `saveAttackerConditionState` (`dnd5e_combat_resolver.go:422`) + the `publishTurnEndAndPersistReset` write-back (`end_turn.go:388-394`).

**The model (Kirk's framing):** the encounter is the **entity-aware authority** — ask it for an entity, get the state-synced runtime instance, because it holds them all. `Data` stays the **simple snapshot** and is a *serialization view* written by this cascade — not a rival read-source that drifts. The SDK owns the round-trip both directions; it still never *stores* (host saves the returned `*Data`). We deliberately keep `Data` simple now — not making the held entities the sole truth / deriving `Data` on demand (a possible later tightening).

---

## 2. Resolver seam change

New `AttackInput` fields (`combat_resolver.go:167-207`):

```go
Attacker combat.Combatant  // e.combatants[AttackerID]; nil if unhydrated (stand-in path)
Defender combat.Combatant  // e.combatants[TargetID]; nil if unhydrated
```

Stat snapshots stay (stand-in / no-data path + the SDK's HP math).

- **`CombatResolver.ResolveAttack`** (`combat_resolver.go:42`): unchanged signature, new contract — "use `input.Attacker`/`input.Defender`; MUST NOT re-load." Host casts `combat.Combatant`→concrete only where it needs richer surface (weapon, ability scores), builds the cheap `CombatantLookup`+gamectx (`dnd5e_combat_resolver.go:188-255`), runs `combat.ResolveAttack`. `resolveEntity`/`loadCharacterWithBus` (`:464-525`) delete.
- **`PhasedCombatResolver`** (`combat_resolver.go:56-73`): signatures unchanged. `ResolveAttackHit` reads `input.Attacker`; `ApplyAttackOutcome` re-derives gamectx from the held entity via `phasedCtx.AttackerID` → `e.combatants[...]`. Opaque `PhasedAttackContext.Rulebook any` carrying `*combat.AttackContext` unchanged (ADR-0027). The `pendingPhasedAttack` cache (`dnd5e_combat_resolver_phased.go:51-72,…`) — which exists only to avoid the double load between phases — deletes.
- **`MovementResolver.ResolveStep`** (`move_resolver.go:58`): `MovementStepInput` gains `Mover combat.Combatant` from `e.combatants[moverID]` (`encounter.go:489`). `applyMonsterReactionConditions` (`reaction_conditions.go:90`) folds into the cascade so the monster's OA `onMovementChain` subscriber is on the bus once.

### Phased two-call + held entities (cross-RPC resume)

`CompleteTakeAction` runs in a new RPC → new `Encounter` → the cascade re-hydrates fresh, so `phasedCtx.AttackerID` → `e.combatants[...]` resolves on the resume. The serializable `*combat.AttackContext` survives the gap as data (ADR-0027). Strictly better than today's "fresh `prepareAttack` re-load."

---

## 3. Turn-boundary signal — emit directly on `e.bus` (no signaler)

`EndTurn(ctx, actorID)` (`combat.go:146`, gains `ctx`) emits the boundary right after the broker `TurnEndedEvent` (`:160`):

```go
_ = dnd5eEvents.TurnEndTopic.On(e.bus).Publish(ctx, dnd5eEvents.TurnEndEvent{CharacterID: string(actorID)})
```

The exact one-liner already in the host's `publishTurnEndOnBus` (`end_turn.go:328-331`), now inside the SDK verb with **no character re-load around it**. Held conditions (`SneakAttackCondition` resets on `TurnEndTopic` for its own `CharacterID` — `sneak_attack.go:102-103,136-139`) reset `UsedThisTurn=false` in place. In the NPC turn-cycle the SDK's `EndTurn` runs per NPC (`end_turn.go:202`), so the signal fires there too — the host's manual `publishTurnEndOnBus(prevNPC)` (`:205`) and `publishTurnEndAndPersistReset` (`:345-395`, the `defer Cleanup` #684 patch) delete. `ToData` (§1) cascades the reset state back, so cross-RPC once-per-turn persistence works without a separate write-back.

---

## 4. Reaction conditions at the cascade

OA + Shield are already in `conditions.LoadJSON` (`loader.go:149-159`), so they ride the cascade as ordinary conditions, gated by the SDK-owned `ReactionReadiness` (`data.go:46`, seeded default-on for OA at `AddPlayer`/`AddMonster`, `encounter.go:229-231,269-271`).

- **(A, recommended)** The cascade's dnd5e-side hydrate step applies OA/Shield (`conditions.NewOpportunityAttackCondition(id).Apply(ctx, e.bus)` when readiness is seeded; Shield when ready + spellcaster heuristic). Pure relocation of `reaction_conditions.go` into the SDK cascade; the `hasFirstLevelSpellSlot` heuristic moves with it.
- (B, rejected for now) Persist OA/Shield into the character's `Data.Conditions`. Changes what the host persists and entangles encounter-scoped readiness with character-scoped condition state.

Either way rpg-api's `reaction_conditions.go` + its depguard exclusion delete; the import guard covers the whole handler+orchestrator package.

---

## 5. Migration & compatibility (clean break)

**SDK (`encounter`) — breaking:** (1) `LoadFromData` gains `ctx`; (2) `EndTurn` gains `ctx`; (3) `New(...)` likely gains `ctx` (runs the cascade; confirm at impl); (4) `AttackInput` gains `Attacker`/`Defender combat.Combatant`, `MovementStepInput` gains `Mover`; (5) `PlayerData` gains `DataJSON` (`omitempty`); (6) `ToData()` cascades held state back; (7) the encounter cascade wires the **existing** `monstertraits.LoadMonsterConditions` for monsters.

**dnd5e rulebook — no new code required:** `monstertraits.LoadMonsterConditions` already exists (`monstertraits/loader.go:84`); the encounter cascade just calls it. combat/character untouched.

**rpg-api adapter — deletes/changes (so the consumer can plan):** delete `resolveEntity`, `loadCharacterWithBus`, `rehydrateMonster` re-load, `pendingPhasedAttack` cache, `saveAttackerConditionState`, `publishTurnEndAndPersistReset`, `publishTurnEndOnBus` calls, `reaction_conditions.go` (+ depguard exclusion); resolver methods read the held entities → shrink to translation; populate `PlayerData.DataJSON` at create/seed; add `ctx` to all `tkenc.LoadFromData`/`enc.EndTurn` call sites. Single cross-repo unit (local `replace` during dev, real tag at end). New ADR + journey entry in the SDK PR.

---

## 6. Invariants preserved

- **Single broker-publish authority:** all `broker.Publish` stays in the SDK; the cascade + turn signal publish only on `e.bus`, never the broker. Host never touches the broker.
- **One mutation owner, one channel — #684 cured at the source:** the cascade is the sole load/subscribe point; the turn boundary is a publish, not a second subscribe. The `defer Cleanup` patches no longer needed on the combat path.
- **Toolkit never orchestrates/persists:** SDK does not fetch char data by ID (host supplies `DataJSON`); `ToData` returns data for the host to save.

---

## 7. Test strategy (real broker path)

1. **Subscribe-exactly-once across N attacks (headline #684 regression).** Seed a player whose `DataJSON` carries `SneakAttack`, `LoadFromData(ctx,…)` once, run `TakeActionPhased` 3× in one turn on the real `e.bus`; assert subscribed once (no "modifier ID already exists") and exactly one SneakAttack subscriber on the damage chain.
2. **Turn reset without re-load.** Held rogue attacks (sets `UsedThisTurn=true`), `EndTurn(ctx,actorID)`, assert the same held instance reads `false` and no second `character.LoadFromData` ran (spy cascade call-count).
3. **`ToData` cascades state back** — `UsedThisTurn` round-trips through `PlayerData.DataJSON`.
4. **Resolver gets the held entity** — spy asserts pointer identity, never re-loads.
5. **No-data fallback** — no `DataJSON` → nil handles → stand-in path resolves (guards existing fixtures).
6. **Consumer integration (their PR):** existing sneak-attack / shield integration tests stay green with the host re-load deleted; MCP playtest is the director-owned signoff bar.

---

## Open questions — director resolutions

- **Q1 — `PlayerData.DataJSON` persisted or transient?** **DEFAULT TAKEN: transient** (host re-attaches the blob after each `Get`, before `LoadFromData`; character store stays authoritative; matches `MonsterData.DataJSON`/`ActivateFeatureInput.CharDataJSON`). Alternative (persist on the snapshot, journey-019 self-contained) trades staleness for self-containment — flagged to Kirk; redirect if desired.
- **Q2 — monster-condition helper?** **RESOLVED: wire the *existing* `monstertraits.LoadMonsterConditions` (`monstertraits/loader.go:84`) into the encounter cascade.** It already exists (used by `zombie.go` + the skeleton/zombie tests); `monster.LoadFromData` doesn't auto-apply conditions by design (caller-applied), so the cascade calls it for monsters. **No new rulebook helper** — corrects the v2 draft's "add a helper" framing (caught in review).
- **Q3 — `ActivateFeature`'s own `defer Cleanup` re-load (`activate_feature.go:100-110`)?** **RESOLVED: follow-up issue** — same #684 class, off the combat-resolution critical path; keep #689 focused.
- **Q4 — held type `combat.Combatant` + type-assert in host, vs typed getters on the SDK?** **RESOLVED: hold `combat.Combatant`, type-assert in the host resolver** — SDK stays at the interface; host knows its concrete types.

## Pushback recorded (expert vs consumer spec)

1. **No injected `WithEntityHydrator`** — hydration is the existing composing `LoadFromData` cascade, not a parallel subsystem.
2. **No `TurnSignaler`** (reversed from the expert's own v1) — emit `dnd5eEvents.TurnEndTopic` directly; the SDK already imports it.
3. **Hold `combat.Combatant`, not opaque `any`** (reversed from v1) — §0 settles the SDK is dnd5e-coupled here; `combat.Combatant` is what the chain consumes. (`PhasedAttackContext.Rulebook` stays `any` — genuinely opaque cross-phase state.)
4. **`ToData` write-back lives in the SDK cascade**, deleting the host's `saveAttackerConditionState` + turn-end write-back. Toolkit still never *stores*.
