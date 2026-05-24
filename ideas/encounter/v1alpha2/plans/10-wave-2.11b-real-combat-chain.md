# Wave 2.11b — Real combat chain (rpg-api goes through dnd5e.combat.ResolveAttack; NPCAct folds onto CombatResolver)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Each inner issue is its own dispatch; this plan is the wave-shape that ties them together.

**Wave goal:** rpg-api's `StandInCombatResolver` is replaced with a real implementation that runs the rulebook's `dnd5e.combat.ResolveAttack` chain. The toolkit's `Encounter.NPCAct` is folded onto the same `CombatResolver` (so the in-package stand-in `resolveAttack` in `combat.go` is deleted). After this wave, every player and NPC attack flows through one rulebook-implemented chain — no parallel stand-in math anywhere in the stack.

**Verified by playtest:** alice + bob connect to a freshly-created TURN_BASED encounter seeded with one or more goblins (the existing Wave 2.8/2.10 fixture). On her turn, alice attacks `goblin-1`. The attack resolves through `dnd5e.combat.ResolveAttack`: rpg-api's resolver implementation rehydrates alice's `*character.Character` from the character store (Wave 2.10's character-equipping work confirms her main-hand weapon is set), looks up the goblin's `*monster.Monster` from the `MonsterData.DataJSON` blob the encounter SDK persisted, registers both as `Combatant`s in a `CombatantLookup`, threads `WithCombatantLookup` onto the resolve context, and calls `combat.ResolveAttack`. The damage breakdown and hit/miss outcome map cleanly back to the encounter SDK's `AttackOutcome`. On the goblin's turn, `Encounter.NPCAct` invokes the same `CombatResolver` (no in-package `resolveAttack` remains in `combat.go`); both browsers see the same per-strike `EntityDamaged` events they saw in 2.11a, but produced by the rulebook-real chain. The two-browser playtest from Wave 2.10 still passes (kill-one / kill-last / `EncounterEnded` / late-joiner snapshot).

**Depends on:**
- Wave 2.5–2.10 (per-viewer projection, broker, snapshot replay, OpenDoor, combat slice, prompts, death/encounter-end) — all shipped.
- **Wave 2.11a** (CombatResolver injection) — shipped: rpg-toolkit/encounter v0.6.0 (PR rpg-toolkit#644) introduced the `CombatResolver` interface + `AttackInput` / `AttackOutcome` types; rpg-api PR #519 wired `StandInCombatResolver` at all 6 LoadFromData/New sites. Wave 2.11b consumes both.
- Wave 2.10's character-store work + the existing rpg-api character orchestrator's `EquipItem` plumbing — verified to exist; the bridge needs to survive encounter rehydration (verification task in this wave, not new code).

## Why this wave exists

Wave 2.11a was a deliberately small carve-out: introduce the resolver injection seam without changing observable combat behavior. It shipped a `StandInCombatResolver` that mirrors the toolkit's pre-2.11a stand-in math (d20+bonus vs AC, damage from notation, nat-1 miss, nat-20 crit doubling the dice) so that the larger architectural shift in 2.11b — *running the real `dnd5e.combat.ResolveAttack` chain* — could land as one focused PR per logical concern, not as a 4-things-at-once merge.

After 2.11a:
- rpg-toolkit/encounter v0.6.0 has the `CombatResolver` interface but **only `Encounter.TakeAction` (player attacks) calls it**. `Encounter.NPCAct` still routes through the in-package `resolveAttack` helper in `combat.go:338-385`, which the v0.6.0 release notes flagged as a Wave 2.11b breaking-change candidate.
- rpg-api wires `StandInCombatResolver` (mimics the deleted toolkit stand-in) at all 6 LoadFromData/New sites. The combat math is duplicated in two places (toolkit's in-package `resolveAttack` for NPCs, rpg-api's `StandInCombatResolver` for players) — both stand-ins, both d20+bonus.

Wave 2.11b closes both gaps in lockstep:

1. **Replace `StandInCombatResolver` with a real `dnd5e.combat.ResolveAttack` adapter** in rpg-api. After this, player attacks emit the rulebook's full damage breakdown (ability mods, fighting-style additions, weapon properties, future Rage / Sneak Attack / Smite chains). Hit calculation goes through the AC chain (Shield spell, cover) instead of the snapshot AC field. The path that reactions / OAs / multiattack will hang off (Wave 2.11) is rulebook-real from this wave forward.
2. **Fold `Encounter.NPCAct` onto the same `CombatResolver`** in rpg-toolkit. After this, the in-package `resolveAttack` in `combat.go` is deleted. Both player and monster attack paths share one resolver injection seam; there is no second stand-in to keep in parity.

Without 2.11b, the larger Wave 2.11 (reactions / OAs / multiattack — `09-wave-2.11-combat-depth.md`) cannot ship. Reactions need `combat.ResolveAttack`'s `AttackResult.ReactionsConsumed` to flow back; OAs ride `combat.ResolveAttack` with `AttackTypeOpportunity`; multiattack is per-strike publication of rulebook attack results. All three assume the rulebook chain is the canonical resolver. 2.11b is the prerequisite that makes the larger 2.11 buildable on top of one chain instead of two stand-ins.

## Why "after 2.11a, before the larger 2.11"

The roadmap places 2.11b strictly between 2.11a (which it builds on) and the larger 2.11 (which it unblocks):

- **After 2.11a (resolver injection seam).** 2.11a defined the `CombatResolver` interface and proved it could be wired without changing behavior. 2.11b is the minimum-viable real implementation behind that interface — it changes *what the resolver computes*, but does not change *the encounter SDK's verb shape*. The wire contract is unchanged; integration tests at the rpg-api level continue to pass against both stand-in and real resolver (the rolls differ deterministically per-test, but the published events are the same shape).
- **Before the larger 2.11 (combat depth).** The larger 2.11 plan (`09-wave-2.11-combat-depth.md`) assumes `combat.ResolveAttack` is in use. That plan's reaction-prompt machinery hooks `AttackResult.ReactionsConsumed`; its OA path uses `combat.ResolveAttack(ctx, &AttackInput{AttackType: AttackTypeOpportunity, ...})`; its multiattack-per-strike audit assumes per-strike calls into the rulebook chain. Shipping any of those before the chain is wired would force them onto the stand-in, which has no concept of reactions, AC chain, or fighting styles. 2.11b is the prerequisite slice.

This wave does not touch reactions / OAs / multiattack itself — those stay scoped to the larger 2.11. 2.11b is **infrastructure for** the larger 2.11, not a partial delivery of it.

## Architectural calls already made (do not relitigate)

- **rpg-toolkit owns the combat chain. rpg-api orchestrates by reference and adapts entity shapes.** Per the boundary rule (`/home/kirk/personal/CLAUDE.md`): "API stores data. rpg-toolkit handles rules." The `CombatResolver` interface lives in rpg-toolkit/encounter; the rulebook chain (`dnd5e.combat.ResolveAttack`) lives in rpg-toolkit/rulebooks/dnd5e. rpg-api's job is to:
  - Look up the player's `*character.Character` from its character store.
  - Rehydrate the monster's `*monster.Monster` from the `MonsterData.DataJSON` the encounter SDK persisted (no new monster store — see Decision Point 1).
  - Implement the `Combatant` adapter so each entity satisfies the rulebook's `combat.Combatant` interface.
  - Build a `CombatantLookup`, thread it onto the context via `WithCombatantLookup`, and call `combat.ResolveAttack`.
  - Translate `*combat.AttackResult` back to the encounter SDK's `*AttackOutcome`.
  - **rpg-api does not implement combat math.** No d20 rolls, no AC checks, no damage formulas in rpg-api's resolver. All math runs in the rulebook.
- **`AttackInput` from the encounter SDK already carries the combat snapshots, but the real resolver ignores them in favor of `Combatant` lookup.** The encounter SDK's `AttackInput` (`combat_resolver.go:47-77`) carries `AttackerAttackBonus`, `AttackerDamageDice`, `AttackerDamageType`, `TargetAC` — these were the stand-in's source of truth. The real resolver ignores them and pulls from the looked-up `Combatant`'s `AbilityScores()`, `ProficiencyBonus()`, `AC()`, plus the equipped weapon. The encounter SDK contract is unchanged: it still passes the snapshots, the resolver implementation just doesn't use them. (Future wave may slim the contract once both call sites — TakeAction and NPCAct — are on the rulebook resolver and snapshots become dead code; out of scope here.)
- **NPCAct fold is a toolkit-side breaking change for the encounter SDK.** It's not a behavioral break (NPCs already attack the same way), but the in-package `resolveAttack` helper goes away. Tests in `rpg-toolkit/encounter` that depend on the helper directly need to either inject a stand-in resolver (the existing `alwaysHitResolver` test helper from `combat_resolver_test.go:12-32`) or be removed. Per the v0.6.0 release notes, this is flagged as an explicit breaking change — tag `encounter@v0.7.0`.
- **No new monster store.** See Decision Point 1 below.
- **Weapon-equipping is not new work.** The existing rpg-api character orchestrator already equips weapons via `EquipItem` and persists the equipment state on `character.Data`. Wave 2.10's character-store work confirms the bridge survives `character.LoadFromData` rehydration. Wave 2.11b's work here is *verification + integration test*, not implementation: assert that the encounter resolver picks up the main-hand weapon from the rehydrated `*character.Character` and passes it to `combat.AttackInput.Weapon`. If the bridge is broken, the verification issue files a bug; if the bridge is intact, the verification issue closes with a passing integration test as evidence.

## Decision Point 1 — Monster persistence (judgment call made here)

**Question:** Where does the rpg-api resolver get the `*monster.Monster` for a `Combatant` lookup when an attack lands?

**Options:**
- **(A) Persistent monster store.** Add a `monsterRepo` parallel to the character repo. Encounter creation seeds the monster store; the resolver looks up by ID; encounter teardown / re-seed manages lifecycle.
- **(B) Encounter-embedded fixture (rehydrate from `MonsterData.DataJSON`).** The encounter SDK already persists each monster as `MonsterData{DataJSON []byte}` (encounter v0.6.0 `data.go:96-104`). The full `monster.Data` round-trips. The rpg-api resolver rehydrates `monster.LoadFromData(monsterData.DataJSON)` per attack (or per encounter load) and uses the live `*monster.Monster` as the `Combatant`.

**Decision: Option B (encounter-embedded fixture).**

**Rationale:** The boundary rule says API stores data; the encounter SDK already *is* the data store for monster instances within an encounter (positions, HP, current state, multi-attack capacity). Adding a separate monster repo introduces a parallel source of truth for "what's the goblin's current HP" — the encounter has it, the new repo would also have to. Two sources of truth for the same field is the bug shape we're trying to avoid. Option B reuses the existing `MonsterData.DataJSON` blob (mirror of how `PlayerData` will eventually carry `CharacterDataJSON` per the larger 2.11 plan), keeps monster lifecycle inside the encounter (created when the encounter is created, gone when the encounter ends), and avoids the migration question of "what happens when a monster moves between encounters" (it doesn't, in this game model). When future content needs persistent monsters across encounters (e.g. recurring named NPCs, follower monsters), promoting to Option A is a future wave's call — the encounter-embedded path doesn't preclude it.

**Implementation note for the resolver issue:** rehydrate per attack is acceptable for the playtest fixture; rehydrate-once-and-cache-on-handler can be a follow-up if the playtest reveals perf cost. The encounter SDK already carries the live `*monster.Monster` in memory after `LoadFromData` returns — the resolver should pull from there if the SDK exposes a getter (verify; if not, the resolver re-rehydrates from `MonsterData.DataJSON` per attack, which is fine for playtest scale).

## Decision Point 2 — Combatant lookup scope

**Question:** Does the `CombatantLookup` cover all combatants in the encounter (registered once at encounter load), or only the two combatants involved in the current attack (registered per attack call)?

**Decision: Per-attack registration of the attacker + target only.**

**Rationale:** `combat.ResolveAttack` only looks up two IDs from context (`AttackInput.AttackerID` and `AttackInput.TargetID`). Registering all combatants is wasted setup; the rulebook never asks for the third party. Per-attack registration also keeps the resolver stateless (no cached lookup map per encounter) and makes the contract obvious in code: build the lookup, thread it on context, call `ResolveAttack`, discard the lookup. If a future feature needs broader lookup (e.g. AOE that hits multiple targets), that future call site builds a lookup over the impacted set — a different verb shape, a different decision.

## Out of scope for Wave 2.11b

- **Reactions, opportunity attacks, multiattack.** All three are scoped to the larger Wave 2.11 (`09-wave-2.11-combat-depth.md`). 2.11b ships the chain that 2.11 hangs them off; it does NOT ship the verbs themselves.
- **`AvailableAction` enumeration.** The larger 2.11 plan owns the `Encounter.AvailableActions(playerID)` enumerator and the proto `TurnState.available_actions` emission. Out of scope here.
- **`combat.ActionEconomy` persistence on PlayerData / MonsterData.** Larger 2.11. The encounter SDK can keep allocating an empty/default economy per-attack at the rpg-api resolver call site for 2.11b — sufficient for single-attack-per-turn (the only mode shipped today).
- **Rehydrating `*character.Character` from a snapshot blob on `PlayerData`.** Larger 2.11 plan flagged this as the largest single change in that wave (`PlayerData.CharacterDataJSON []byte` mirror of `MonsterData.DataJSON`). 2.11b uses the existing rpg-api character-store lookup (`character.LoadFromData(charOutput.Character.Data, bus)`) per attack. When 2.11 lands, the resolver can switch to PlayerData-embedded rehydration; the seam is clean because the resolver is the only consumer.
- **Slimming the `AttackInput` snapshot fields** (`AttackerAttackBonus`, `AttackerDamageDice`, etc.). The real resolver ignores them but the contract still carries them. A future wave can deprecate-and-remove once both call sites are on the rulebook chain and no test depends on the snapshot fields.
- **Player dying-state mechanics, death saves, TPK.** Same as Wave 2.10 — out of scope; 2.11+ territory.
- **Cross-encounter monster persistence.** Per Decision Point 1; not needed for the current game model.
- **LobbyView migration.** Wave 5. Harness is the sole UI surface during 2.11b; the wave does not touch web at all (existing per-viewer event projection just keeps working — no web change required because the wire shape is unchanged).

## Inner-work shape

### rpg-toolkit — NPCAct fold onto CombatResolver (drop in-package `resolveAttack`)

This is the toolkit-side change. Tag `encounter/v0.7.0` (breaking change vs v0.6.0 — tests that depended on the in-package `resolveAttack` need a wired resolver).

1. **Route `NPCAct`'s attack paths through `e.combatResolver.ResolveAttack`.** `npc.go:223` (`applyCapturedAttacks`) and `npc.go:400` (`npcActScripted`) are the two call sites. Both currently call `e.resolveAttack(mon.AttackBonus, target.AC, mon.DamageDice)` and apply the returned `attackResolution`. Replace with a `e.combatResolver.ResolveAttack(AttackInput{...})` call (mirror the shape from `combat.go:223-247`), translate the `*AttackOutcome` to the local `attackResolution`, continue the existing damage-application + publish flow unchanged.
2. **Decide AttackerID semantics for monster→player attacks.** `AttackInput.AttackerID` is currently `core.EntityID` typed; it carries player IDs in the player path. For NPCs, pass the `MonsterData.ID` (already `core.EntityID`). The rpg-api resolver's `CombatantLookup` distinguishes by ID prefix or by tracking which side is which when registering — no SDK change needed.
3. **Delete the in-package `resolveAttack` helper** (`combat.go:338-385`) and its `attackResolution` struct (`combat.go:325-333`). Replace internal callers with the resolver path. The `attackResolution` shape was already factored out for both player and NPC paths to share — both now share the resolver path instead.
4. **Update the encounter SDK's `ErrNoCombatResolver` guard.** `NPCAct`'s call sites must check `e.combatResolver != nil` and return `ErrNoCombatResolver` if not (same as `combat.go:218`). Update `Encounter.NPCAct`'s godoc to flag the requirement.
5. **Update tests.** Tests that used the bare in-package math (e.g., `combat_test.go` paths that asserted specific d20 outcomes) need to either inject `alwaysHitResolver` (the existing test stub from `combat_resolver_test.go:12-32`) or be replaced with resolver-wiring assertions. Audit `npc_test.go`, `combat_test.go`, `death_test.go`, `integration_test.go` for direct `resolveAttack` dependencies.
6. **Tag `encounter/v0.7.0`** with release notes flagging the breaking change: "Encounter.NPCAct now requires a wired CombatResolver (same as TakeAction since v0.6.0). The in-package resolveAttack helper has been removed. Tests that depended on it must inject a stub resolver — see combat_resolver_test.go for the pattern."

### rpg-api — Combatant adapter + CombatantLookup wiring

The infrastructure issue. Builds the bridge between rpg-api's character/monster entity shapes and the rulebook's `combat.Combatant` interface. Does NOT replace `StandInCombatResolver` (that's the next issue) — it builds the adapter that the replacement needs.

1. **Implement a `combat.Combatant` adapter for `*character.Character`.** rpg-api character orchestrator already loads a `*character.Character` via `character.LoadFromData(ctx, char.Data, bus)` at multiple sites (`orchestrator.go:246`, `:324`, `:807`, `:2431`, etc.). The `*character.Character` may already implement `Combatant` directly (the rulebook ships `combat.Combatant` and its built-in Character — verify; if so, no adapter needed for the player side, just confirm the implementation is exposed).
2. **Implement a `combat.Combatant` adapter for `*monster.Monster`.** Same verification: the rulebook's monster type may implement `Combatant` directly. If not, build a thin adapter (`type monsterCombatant struct { m *monster.Monster }` with method delegations).
3. **Implement a `combat.CombatantLookup`.** Simple struct with a `map[string]combat.Combatant` and a `Get(id string) (combat.Combatant, error)` method per the interface (`dnd5e@v0.55.5/combat/combatant.go:106-110`). Handles "not found" with a clean error.
4. **Tests:** unit tests for the adapter (HP / AC / AbilityScores / ProficiencyBonus / ApplyDamage round-trip correctly through the adapter); unit tests for the lookup (Get returns registered, Get returns NotFound for unknown ID).

### rpg-api — Real `CombatResolver` implementation (replaces StandInCombatResolver)

The headline issue. Swaps `StandInCombatResolver` for a real implementation that goes through `dnd5e.combat.ResolveAttack`.

1. **New file:** `internal/handlers/dnd5e/v2/encounter/dnd5e_combat_resolver.go` (sibling of `combat_resolver.go`). Defines `Dnd5eCombatResolver` implementing the encounter SDK's `tkenc.CombatResolver` interface.
2. **Inject dependencies:** the resolver needs the character orchestrator (to load `*character.Character` by ID) and an event bus for `combat.AttackInput.EventBus`. The encounter handler already has these via `HandlerConfig` — extend the handler config or pass them through resolver construction.
3. **`ResolveAttack(input tkenc.AttackInput) (*tkenc.AttackOutcome, error)` implementation:**
   - Build a context (background or, ideally, propagate from the calling RPC — see implementation note below on propagating context).
   - Look up the attacker:
     - If `AttackerID` matches a player's character ID, load via the character orchestrator (`character.LoadFromData(ctx, char.Data, bus)`).
     - If `AttackerID` matches a monster ID in the encounter, rehydrate from `MonsterData.DataJSON` (per Decision Point 1). The resolver needs access to the encounter's `MonsterData` map — pass via context, or have the encounter SDK pass the live `*monster.Monster` on `AttackInput` (proposed encounter-side enhancement for a follow-up; not required for 2.11b — for now, the resolver looks up via the character orchestrator for players + the encounter store for monsters).
   - Look up the target with the same logic, swapped sides (player attacking monster → target lookup is monster-side; monster attacking player → target is character-side).
   - Build a `CombatantLookup` registering both sides under their entity IDs.
   - Call `ctx = combat.WithCombatantLookup(ctx, lookup)`.
   - Build `combat.AttackInput`:
     - `AttackerID` (string form of `tkenc.AttackerID`)
     - `TargetID` (string form of `tkenc.TargetID`)
     - `Weapon`: pull from the attacker's equipped main-hand (or off-hand if `input.AttackHand == "off"`). For monsters, the rulebook's monster type carries an attack definition equivalent — verify the bridge (this is the **weapon-equipping verification** sub-task of the wave; see the dedicated issue below).
     - `EventBus`: the rpg-api's character bus (or a per-attack bus if isolation is desired — verify what the rulebook expects).
     - `Roller`: nil (rulebook default), or inject for tests.
     - `AttackHand`: from `input.AttackHand`.
     - `AttackType`: empty (defaults to `AttackTypeStandard`; reactions/OAs land in larger 2.11).
   - Call `combat.ResolveAttack(ctx, &input)`, get `*combat.AttackResult`.
   - Translate `*combat.AttackResult` to `*tkenc.AttackOutcome`:
     - `Hit = result.Hit`
     - `Critical = result.Critical`
     - `AttackRoll = result.AttackRoll`
     - `AttackBonus = result.AttackBonus`
     - `TargetAC = result.TargetAC`
     - `Damage = result.TotalDamage`
     - `DamageType = string(result.DamageType)`
   - Return.
4. **Wire the new resolver at all 6 LoadFromData/New sites** (the same sites Wave 2.11a wired `StandInCombatResolver`):
   - `internal/handlers/dnd5e/v2/encounter/take_action.go:78`
   - `internal/handlers/dnd5e/v2/encounter/create.go:47`
   - `internal/handlers/dnd5e/v2/encounter/handler.go:117`
   - `internal/handlers/dnd5e/v2/encounter/end_turn.go:83`
   - `internal/handlers/dnd5e/v2/encounter/submit_check.go:97`
   - `internal/handlers/dnd5e/v2/encounter/interact.go:71`
   - HandlerConfig flips its default from `StandInCombatResolver` to `Dnd5eCombatResolver` when constructed with the character orchestrator.
5. **Keep `StandInCombatResolver` exported for tests.** Tests that need deterministic combat outcomes (the existing `EncounterV2IntegrationSuite`'s combat assertions) can either keep using `StandInCombatResolver` or use a deterministic roller through the real resolver. Decision per test: the kill-loop tests likely want `StandInCombatResolver` with a high-bonus + high-damage stub (faster, deterministic); the Wave 2.11b integration test exercises the real resolver path.
6. **Tests:**
   - Unit tests for `Dnd5eCombatResolver` with mock character orchestrator + monster fixture — assert it builds the right `combat.AttackInput`, threads `WithCombatantLookup`, and translates `*AttackResult` correctly.
   - Integration test extending `EncounterV2IntegrationSuite`: `TestIntegration_RealCombatChain`: alice attacks goblin via `TakeAction`; resolver runs the rulebook chain; published `EntityDamaged` event matches the rulebook's damage breakdown (assert non-zero damage on a forced hit, correct damage type from alice's main-hand weapon, etc.).
   - Verify the existing Wave 2.10 tests (`TestIntegration_PlayerKillsGoblin_EncounterEnds`) still pass with the real resolver wired (may need to retune fixture HP / damage to keep the kill-in-N-attacks property — file follow-ups for HP fixture tuning if the real damage curve breaks the test, don't paper over with absurd HP).
7. **Bump rpg-toolkit/encounter pin to v0.7.0** (the NPCAct-fold release). Verify the rpg-api compiles and the existing tests pass against the bumped pin.

### rpg-api — Verify weapon-equipping bridge survives encounter rehydration

Verification issue, not implementation. The existing rpg-api character orchestrator already supports `EquipItem` (`orchestrator.go`, `service.go:50, 283-291`); Wave 2.10's character-store work is already shipped. This issue **proves the bridge is intact** end-to-end: a character whose main-hand weapon was set via `EquipItem` keeps that weapon when the encounter handler reloads them via `character.LoadFromData(ctx, char.Data, bus)` and passes them to the new `Dnd5eCombatResolver`.

1. **Read** `internal/orchestrators/encounter/orchestrator.go` around the `character.LoadFromData` call sites (`:246`, `:324`, `:807`, `:2431`) and confirm the equipment slot map round-trips through `character.Data`.
2. **Read** `internal/orchestrators/character/orchestrator.go::EquipItem` (or wherever EquipItem persists the slot) and confirm it's saved to the same `character.Data` shape that the encounter handler loads.
3. **Write an integration test** that:
   - Creates a character through the character orchestrator.
   - Equips a known weapon (e.g. a longsword) via `EquipItem`.
   - Creates an encounter with that character as a player.
   - In the encounter handler's resolver path, asserts the rehydrated `*character.Character` has the longsword equipped on the main-hand slot.
   - Asserts the resolver's `combat.AttackInput.Weapon` field is the longsword's `*weapons.Weapon`.
4. **If the bridge is broken**, file a bug in the rpg-api repo, link to this verification issue, and either (a) fix in this same PR if scope is small, or (b) declare the weapon-equipping bridge a blocker on the resolver issue and dispatch a fix-then-resume sequence.
5. **If the bridge is intact**, close the issue with the integration test as evidence; the resolver issue dispatches against the verified bridge.

### chore: Wave 2.11b close-the-loop (rpg-project)

Standard close-the-loop shape: file followups, update board, the larger Wave 2.11 (`09-wave-2.11-combat-depth.md`) flips from Blocked-on-2.11b to Active, capture verified patterns into role context, retro on tracker.

---

## Inner-issue list (filed when this kickoff package is published)

- **🎮 Tracker** (rpg-project): wave goal sentence + sign-off + contents checklist
- **rpg-toolkit**: NPCAct fold onto CombatResolver (drop in-package resolveAttack); tag `encounter/v0.7.0` (breaking change)
- **rpg-api** (1 of 3): Combatant adapter + CombatantLookup wiring (infrastructure; ships before resolver swap)
- **rpg-api** (2 of 3): Real Dnd5eCombatResolver implementation replacing StandInCombatResolver (the headline change; depends on Combatant adapter + toolkit v0.7.0 bump)
- **rpg-api** (3 of 3): Verify weapon-equipping bridge survives encounter rehydration (dependency for the resolver issue; small but load-bearing)
- **chore: Wave 2.11b close-the-loop** (rpg-project): file followups, update board, larger 2.11 → Active, capture verified patterns into role context

The Wave 2.11b kickoff dispatch files all of the above on board #11 with `Wave 2.11b` and `Status: Todo`.

---

## Forward-loaded patterns (anticipated — refine in close-the-loop)

These are the new patterns Wave 2.11b's execution is expected to surface. Marked `status: anticipated` until the wave's close-the-loop step verifies them against shipped reality.

**rpg-api-member additions:**

- `pat-v2-real-combat-resolver` *(anticipated)* — `Dnd5eCombatResolver` implements `tkenc.CombatResolver` by adapting rpg-api character/monster shapes to the rulebook's `combat.Combatant` interface, building a per-attack `CombatantLookup`, threading `WithCombatantLookup` onto context, and calling `combat.ResolveAttack(ctx, &combat.AttackInput{...})`. Returns the rulebook's `*combat.AttackResult` translated to the encounter SDK's `*AttackOutcome`. The encounter SDK's `AttackInput` snapshot fields (`AttackerAttackBonus` etc.) are ignored — all stat lookup goes through the live `Combatant`.
- `pat-v2-combatant-adapter` *(anticipated)* — rpg-api's character + monster shapes adapt to `combat.Combatant` by either direct interface satisfaction (if the rulebook's Character/Monster types already implement Combatant) or thin adapter structs. Adapters delegate `AbilityScores()` / `ProficiencyBonus()` / `AC()` / `ApplyDamage()` straight through to the underlying entity. Per-attack registration in `CombatantLookup` (attacker + target only, not all combatants).
- `pat-v2-monster-rehydration-from-encounter` *(anticipated)* — Monsters are rehydrated from the encounter's `MonsterData.DataJSON` blob, not from a separate monster repo. The encounter is the source of truth for monster instance state within an encounter; no parallel persistent monster store. Cross-encounter persistence is a future concern; this pattern is correct for the single-encounter game model shipping today.

**rpg-toolkit-member additions:**

- `pat-encounter-resolver-everywhere` *(anticipated, validated upgrade)* — Both `TakeAction` (player) and `NPCAct` (monster) routes through the same wired `CombatResolver`. The in-package `resolveAttack` helper is gone; there is no second math path. Tests inject `alwaysHitResolver` (or equivalent stub) for deterministic combat outcomes.
- `pat-encounter-v0.7.0-breaking-change-shape` *(anticipated)* — When folding a verb onto an existing interface that was introduced for one path, the SDK's release notes flag the breaking change explicitly; tests in dependents (rpg-api) wire the real resolver before the bumped pin lands; the bump PR is one focused PR per concern (toolkit fold, then rpg-api resolver swap).

After execution, the close-the-loop dispatch promotes verified `anticipated` entries to status-less (validated) and adds any new patterns that surfaced.

---

## Execution sequence

1. **Wave 2.11a close-the-loop** (chore #43, in flight) completes before 2.11b dispatches. That step writes verified Wave 2.11a patterns into role context — Wave 2.11b's dispatches read those.
2. **rpg-toolkit issue dispatches first** (NPCAct fold). Smallest blast radius (toolkit-internal change; rpg-api doesn't need to bump until step 4). Tag `encounter/v0.7.0`.
3. **rpg-api Combatant adapter issue dispatches second** (parallel-safe with step 4). Builds the `combat.Combatant` adapter for character + monster, builds the `CombatantLookup`. No behavior change shipped yet — just the infrastructure.
4. **rpg-api weapon-equipping bridge verification dispatches in parallel with step 3.** Small verification issue with an integration test as the artifact; either confirms the bridge or files a fix.
5. **rpg-api resolver swap issue dispatches fourth** (depends on steps 2, 3, 4). Bumps rpg-toolkit/encounter pin to v0.7.0; replaces `StandInCombatResolver` with `Dnd5eCombatResolver`; integration test exercises the real chain end-to-end.
6. **End-to-end playtest** (cold start, both browsers, 2-tab pattern from `v2-playtest-harness`):
   - alice + bob connect to a freshly-created TURN_BASED encounter with one or more goblins (Wave 2.8/2.10 fixture).
   - alice attacks goblin via `TakeAction`. `EntityDamaged` event lands on both browsers with damage from alice's equipped weapon (longsword damage curve, not the stand-in's `1d4`-flat fallback). Damage type matches the weapon (slashing for longsword).
   - alice ends turn. Goblin's NPC turn fires `NPCAct`; the goblin's attack runs through the same `Dnd5eCombatResolver` (NPCAct fold from step 2); `EntityDamaged` event lands on both browsers.
   - alice continues attacking until the goblin dies (Wave 2.10's death + encounter-end behavior intact).
   - All Wave 2.10 sign-off behavior continues to pass (kill-one / kill-last / `EncounterEnded` / late-joiner snapshot).
7. **chore: Wave 2.11b close-the-loop** dispatches: files followups, updates board (2.11b → Done; larger 2.11 → Active), captures verified patterns into role context, retros on tracker.

---

## Reference

- Wave 2.11b tracker: rpg-project#<filled in by this kickoff dispatch>
- rpg-toolkit issue: rpg-toolkit#<filled in by this kickoff dispatch>
- rpg-api Combatant adapter issue: rpg-api#<filled in by this kickoff dispatch>
- rpg-api resolver swap issue: rpg-api#<filled in by this kickoff dispatch>
- rpg-api weapon-equipping verification issue: rpg-api#<filled in by this kickoff dispatch>
- Wave 2.11a tracker (close-the-loop): rpg-project#43
- Wave 2.11a toolkit PR: rpg-toolkit#644 (merged — encounter v0.6.0)
- Wave 2.11a rpg-api PR: rpg-api#519 (merged — StandInCombatResolver wiring)
- Larger Wave 2.11 plan (blocked on this wave): `rpg-project/ideas/encounter/v1alpha2/plans/09-wave-2.11-combat-depth.md`
- Wave 2.10 plan (death/encounter-end — sign-off behavior 2.11b must not regress): `rpg-project/ideas/encounter/v1alpha2/plans/08-wave-2.10-death-resolution.md`
- Master plan: `rpg-project/ideas/encounter/v1alpha2/plan.md`
- Roadmap: `rpg-project/ideas/encounter/v1alpha2/roadmap.md`
- Toolkit references (verify pinned versions at dispatch):
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.6.0/combat_resolver.go` — `CombatResolver` interface, `AttackInput`, `AttackOutcome`, `ErrNoCombatResolver`
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.6.0/combat.go:189-289` — `Encounter.TakeAction` resolver call site (player path; reference shape for NPCAct fold)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.6.0/combat.go:325-385` — in-package `attackResolution` + `resolveAttack` (DELETE in 2.11b)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.6.0/npc.go:200-260` — `applyCapturedAttacks` (NPCAct call site 1)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.6.0/npc.go:380-420` — `npcActScripted` (NPCAct call site 2)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.6.0/combat_resolver_test.go:12-32` — `alwaysHitResolver` test stub pattern for tests
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/encounter@v0.6.0/data.go:85-104` — `MonsterData{DataJSON}` (rehydration source per Decision Point 1)
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/combat/attack.go:67-160` — `combat.AttackInput` + `combat.ResolveAttack` signatures
  - `~/go/pkg/mod/github.com/!kirk!diggler/rpg-toolkit/rulebooks/dnd5e@v0.55.5/combat/combatant.go:54-130` — `Combatant` interface + `CombatantLookup` + `WithCombatantLookup`
- rpg-api references (verify at dispatch):
  - `internal/handlers/dnd5e/v2/encounter/combat_resolver.go` — current `StandInCombatResolver` (to be replaced)
  - `internal/handlers/dnd5e/v2/encounter/{take_action,create,handler,end_turn,submit_check,interact}.go` — 6 LoadFromData/New sites threaded with `WithCombatResolver`
  - `internal/orchestrators/encounter/orchestrator.go:246, 324, 807, 2431` — `character.LoadFromData` call sites (resolver leverages the same path)
  - `internal/orchestrators/character/service.go:50, 283-291` — `EquipItem` orchestrator surface (weapon-equipping bridge to verify)
- Board: https://github.com/users/KirkDiggler/projects/11

## Sign-off criterion

Wave 2.11b is done when, in a cold-start two-browser playtest:

- alice + bob connect to a freshly-created TURN_BASED encounter (Wave 2.8/2.10 fixture).
- alice attacks goblin via `TakeAction`. The published `EntityDamaged` event reflects damage computed by `dnd5e.combat.ResolveAttack` — specifically, the damage matches alice's equipped main-hand weapon (verified via integration test asserting damage type / damage range).
- alice ends turn. The goblin's NPC turn runs through the same `CombatResolver` (no in-package `resolveAttack` remains in `combat.go`). `EntityDamaged` event lands on both browsers.
- All Wave 2.10 sign-off behavior continues to pass (kill-one of two goblins → encounter continues; kill-last → `EncounterEnded`; subsequent `TakeAction` returns `FailedPrecondition`; late-joiner snapshot reflects post-end state).
- The `StandInCombatResolver` may remain exported for test use, but no production code path constructs it.
- `combat.go` in `rpg-toolkit/encounter` no longer contains the in-package `resolveAttack` helper or the local `attackResolution` struct.

When the close-the-loop runs, the playtest video / screenshots become the verification artifact and the wave ships. The larger Wave 2.11 (`09-wave-2.11-combat-depth.md`) flips from Blocked-on-2.11b to Active.
