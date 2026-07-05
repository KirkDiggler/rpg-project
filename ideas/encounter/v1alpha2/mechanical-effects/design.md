# Mechanical Effects — Beat 2

**Chapter 2 · v1alpha2 encounter route**
**Validated against:** the North-Star Invariants in [`../design.md`](../design.md). First adversarial
review pass (2026-07-03): **PASS-WITH-REQUIRED-CHANGES** — six required changes (R1-R6) plus three
accuracy nits (N1-N3), all folded into this revision; see the self-check at the end of this doc for the
resolution of each. Awaiting re-review before it becomes the implementer's star.
**Status:** wave content — revision addressing required-changes, pending re-review.
**Scope anchors:** `KirkDiggler/rpg-toolkit#716` (Help + Hide resolve as economy-only no-ops),
`KirkDiggler/rpg-toolkit#699` (wire `DodgeActivated` → `DodgingCondition.Apply`). Both already filed
under the umbrella `KirkDiggler/rpg-project#54`.

---

## Why this is Beat 2

Beat 1 (TakeAction) proved the spine: any `action_ref` routes through the toolkit's real menu/economy
engine, spends correctly, and leaves a canonical resolved-action event. The 2026-07-01 playtest
(`wave-2-monk`) that verified this also produced the honest follow-up: **Help, Hide, and Dodge spend
the action and emit `actionResolved` — and then do nothing.** No check roll, no condition, no
disadvantage on the attacker. The economy is honest; the *game* isn't yet. Beat 2 closes that gap for
these three verbs specifically (the ones already surfaced by playtest evidence), not the whole
condition catalog.

## Goal behavior (the done bar)

> Dodge, Help, and Hide each produce a real, observable mechanical effect — not just an economy
> debit. Proven via **MCP playtest**: Dodge gives the next attacker disadvantage (visible in the
> attack breakdown); Hide rolls a Stealth check and, on success, grants the hider advantage / imposes
> disadvantage on attacks between them and their observers; Help grants an ally advantage on their
> next attack or check. The same session verifies a **player getting hit** (all prior scenarios were
> player-kills-monster) so the HP bar / death gate get their first real exercise. The **retro across
> all sessions closes the wave** — not green CI.

---

## The real shape of the work

The first read of #716/#699 makes this sound like three independent, symmetric "wire the condition"
tickets. They aren't. Reading the actual code, and a subsequent adversarial review pass, surfaced six
pieces of unequal size — one is net-new toolkit spine, two are bugs the existing condition-bridging
code has that only a *cross-entity* effect like Help exposes. Naming them here so none can hide behind
"implement Help/Hide/Dodge" again.

### 1. Dodge is (almost) free — reuse the proven pattern, don't reinvent it

`DodgingCondition` (`rulebooks/dnd5e/conditions/dodging.go`) is fully implemented and correct — it
already subscribes `AttackChain` (disadvantage to attackers), `SavingThrowChain` (DEX-save advantage),
and `TurnStartTopic` (self-removal). `Dodge.Activate()` (`rulebooks/dnd5e/combatabilities/dodge.go:78-101`)
already publishes `DodgeActivatedEvent`. No *combat-time activation path* constructs or applies the
condition — it has two non-combat constructors already (`rulebooks/dnd5e/conditions/factory.go:100`,
`rulebooks/dnd5e/conditions/loader.go:136`), used for persistence reconstitution, not activation.

The fix is **not** a new wiring mechanism. Rage already proves the pattern end to end:
`Rage.Activate()` (`rulebooks/dnd5e/features/rage.go:105-146`) constructs `*conditions.RagingCondition`
and publishes it via `dnd5eEvents.ConditionAppliedTopic` with `Condition: ragingCondition` set. Two
subscribers already exist and require **zero new plumbing**:
- `Character.onConditionApplied` (`rulebooks/dnd5e/character/character.go:1062-1078`, wired in
  `subscribeToEvents` at line 1019) filters on `event.Target.GetID() == c.id` and calls
  `event.Condition.Apply(ctx, c.bus)` — this is what actually subscribes `RagingCondition`'s (and would
  subscribe `DodgingCondition`'s) handlers onto the bus.
- `subscribeConditions` / `applyActivatedConditions` (`encounter/npc.go:1187`,
  `encounter/activate_feature.go:205`, already invoked by `takeCharacterAction` at
  `encounter/take_character_action.go:70-83`) bridges the same `ConditionAppliedEvent` to the broker
  so the client sees it — see §2 below for a real gap in this bridge that Help exposes.

So Dodge's entire fix is: `Dodge.Activate()` additionally constructs
`conditions.NewDodgingCondition(owner.GetID())` and publishes it via `ConditionAppliedTopic` —
identical shape to Rage's four lines. `DodgeActivatedEvent` stays (still useful as a narration signal);
`ConditionAppliedEvent` is added alongside it. This needs two small taxonomy additions: `ConditionType`
has no `ConditionDodging` value (`rulebooks/dnd5e/events/events.go:18-70` — Raging, RecklessAttack,
FightingStyle exist, Dodging/Hidden don't), and `ConditionSource` has only
`ConditionSourceClass`/`ConditionSourceFeature` (`rulebooks/dnd5e/events/events.go:73-80` — no
`ConditionSourceCombatAbility`). Both are one-line additions. `DodgingCondition` is already registered
for round-trip (`rulebooks/dnd5e/conditions/loader.go:135`, `rulebooks/dnd5e/conditions/factory.go:99`,
`ToJSON`/`loadJSON` at `dodging.go:120-137`) — this piece needs no new work. Compare §3 below, where
the two *new* conditions this wave adds do need that registration built from scratch.

### 2. Help needs cross-entity condition targeting — the label AND the audience (new capability)

Every existing self-applying condition (Raging, RecklessAttack, and now Dodging above) targets its own
activator. Help's condition targets **someone else** — the ally. Four real gaps, all precise (the
fourth was missed in the first draft of this doc and is the one the review gate caught):

- **No target reaches the ability at all.** `CombatAbilityInput` (`rulebooks/dnd5e/combatabilities/input.go`)
  carries `Bus`, `ActionEconomy`, `ActionHolder`, `Speed`, `ExtraAttacks` — no target. `ActivateAbilityInput`
  (`rulebooks/dnd5e/character/action_economy_types.go:116`) carries only `AbilityRef`. Compare
  `ExecuteActionInput` (same file, line 130-133), which already carries `TargetID string` for strikes —
  that's the precedent to mirror, not invent. The encounter's own dispatch
  (`encounter/take_character_action.go:44-101`) calls
  `char.ActivateAbility(ctx, &ActivateAbilityInput{AbilityRef: tRef})` — the `target ActionTarget`
  parameter already sitting in scope is never passed through. `help.go`'s doc comment says exactly this
  today: *"the ally target is not yet carried through the character `ActivateAbility` path... documented
  gap"*. Fix: add `TargetID` to `ActivateAbilityInput` (mirroring `ExecuteActionInput`), add `Target
  core.Entity` to `CombatAbilityInput` (the ability needs the entity, not just an id, because it
  publishes `ConditionAppliedEvent{Target: ...}` — see below), and have `takeCharacterAction` resolve
  `target.EntityID` to a `core.Entity` via the existing `e.heldCharacter(id)` helper
  (`encounter/turn_economy.go:63`) before dispatch.
- **`applyActivatedConditions` hardcodes the wrong target *label*.** `encounter/activate_feature.go:211`:
  `targetID := actorID` — always. Left unfixed, a helped ally's condition would render as if the
  *helper* had it — a real observability bug, not a hypothetical.
- **`applyActivatedConditions` also hardcodes the wrong *audience*, and this is the gap the review
  gate found that the first draft missed.** The same function (`activate_feature.go:221-227`) computes
  the broker-visible audience — who is allowed to see the condition-applied event — from
  `actor.View.Position` via `e.viewerCanSee(...)`, and `takeCharacterAction` only has the **actor's**
  `player *PlayerData` in scope (`take_character_action.go:44-46, 81`) — it has no handle on the
  target's `PlayerData`/`View` at all. `e.heldCharacter(id)` (`encounter/turn_economy.go:63-69`)
  returns a `*character.Character`, which carries **no position**; positions live on `PlayerData.View`.
  So for a Help condition on an ally, the correct audience is "who can see the *ally*," not "who can see
  the helper" — and under this wave's own deferral of Help's 5-ft-adjacency RAW rule (see the RAW
  table below), helper and ally can be arbitrarily far apart in the encounter, so this isn't a
  theoretical edge case; it's the normal case. **Fix:** `takeCharacterAction` must resolve and pass the
  **target's `PlayerData`** (not just its `core.Entity`) through to `applyActivatedConditions`, which
  reads each captured `cond.Target.GetID()` for the label (fixing the first gap) *and* projects the
  viewer audience from the target's `View.Position` (fixing this one) — both fixes land in the same
  function, on the same `cond` loop variable, in the same PR.
- **Target-kind is already anticipated on the wire.** `targetKindForRef`
  (`rulebooks/dnd5e/character/action_economy.go:174-193`) already tags `refs.CombatAbilities.Help()` as
  `TargetKindSingleEntity` (grouped with Attack) and Dodge/Disengage/Hide as `TargetKindSelf`. The menu
  plumbing from Beat 1 already expects this — it's the activation path that's missing the wiring, not
  the wire contract.

### 3. Hide needs check machinery the toolkit doesn't have yet — this is the largest piece

There is no ability-check resolution in the toolkit today. `saves.MakeSavingThrow`
(`rulebooks/dnd5e/saves/saves.go`) is the only precedent: roller + modifier + chain-sourced
advantage/disadvantage/bonuses + nat-1/nat-20 detection, firing `SavingThrowChainEvent` through
`SavingThrowChain` when a bus is present. `Character.GetSkillModifier(skill)`
(`rulebooks/dnd5e/character/character.go:184-198`) already computes the Stealth modifier (ability +
proficiency); there is no `Character.PassivePerception()` (only `Monster.Data.PassivePerception` exists
as a static field, no accessor, and no `Combatant` interface method —
`rulebooks/dnd5e/combat/combatant.go:54-79` has no perception method at all).

**Named toolkit spine addition:** a `checks` package mirroring `saves`, with `MakeAbilityCheck`
(same roll/advantage/disadvantage/chain shape as `MakeSavingThrow`) and a new
`AbilityCheckChainEvent` / `AbilityCheckChain` topic in `events.go` (same shape as
`SavingThrowChainEvent`: `CheckerID`, `Skill`, `DC`, `AdvantageSources`/`DisadvantageSources`/
`BonusSources`). This is justified by Hide alone — the check function and chain topic exist because
Hide needs a real Stealth roll, the same way `MakeSavingThrow`'s chain fires whether or not any
condition currently subscribes to it. **Scope note (post-review, see Resolved Decisions):** the chain
fires for Hide's own roll this wave; whether anything *subscribes* to it (Help's ability-check
advantage) is decided separately below and is **not** assumed here. `PassivePerception()` is added to
`Character` (`10 + WIS mod + proficiency`, mirroring `GetSkillModifier`) and to `Monster` (from the
existing static field), and to the `Combatant` interface so Hide's check can compare against any
observer type generically.

> **Guardrail (from the review gate, encode verbatim):** passive Perception is a *rulebook*
> computation. The encounter SDK **gathers** each observer's value by calling
> `Combatant.PassivePerception()` — it must never inline the `10 + WIS + proficiency` formula itself.
> If an implementer computes that formula inside the `encounter` package instead of calling the
> rulebook method, that crosses the boundary this wave is otherwise careful about. This is the one
> place position-aware code (the SDK) and rules-aware code (the rulebook) touch, and the seam is the
> interface method, not a copy of the arithmetic.

**Observer set — a boundary call, endorsed by the review gate with the guardrail above.** Hide's
Stealth check needs to know who's watching. "Who can physically see whom" is a spatial/LOS question;
`rulebooks/dnd5e` has no positions and genuinely cannot answer it. The toolkit's `encounter` SDK
package already has hex positions and an LOS gate (`encounter/perception.CanSeeAt`, currently a
range-only stub — "ignores walls, lighting, conditions," per its own doc comment) — it's already used
to gate `publishAttackOutcome`'s audience. So: the **encounter SDK** (still toolkit-owned, not
rpg-api) computes the observer set — opposing combatants within `perception.CanSeeAt` of the hider —
and *gathers* their passive Perception via `Combatant.PassivePerception()` (never recomputing it), the
same way it already computes attacker/target visibility for damage events. The **rules verdict stays
in the rulebook**: the Stealth roll, the "beat the highest" comparison, and the advantage/disadvantage
grants all live in `rulebooks/dnd5e`. This is the existing encounter-owns-position /
rulebook-owns-rules split, applied one step earlier than usual (pre-check instead of
post-resolution) — not a new pattern.

**Simplification, stated plainly:** Hide succeeds only if the Stealth total beats the *highest*
passive Perception among current observers — binary, not per-observer. RAW is per-observer (hidden
from some, seen by others); that needs the wire's tiered-visibility / entity-override machinery from
`design.md` §2 ("Hidden traps and illusions are modeled as entity-overrides") extended to characters,
which is a v1alpha2 wire feature this wave does not build. Named as deferred below, not silently cut.

### 4. The two new conditions must be registered, or they evaporate at the next RPC boundary

`DodgingCondition` round-trips today because it's registered in **both**
`rulebooks/dnd5e/conditions/loader.go:135` (reload from persisted JSON) and
`rulebooks/dnd5e/conditions/factory.go:99` (construct-from-ref), plus has `ToJSON`/`loadJSON`
(`rulebooks/dnd5e/conditions/dodging.go:120-137`). This is not optional plumbing — the encounter persists between RPCs, and both
new flows this wave adds *cross an RPC/turn boundary*: Hide → (the hider gets attacked on a later turn)
and Help → (the ally attacks on a later action). If `HiddenCondition`/`HelpedCondition` are built
without the same registration, the condition is silently dropped on the very next load — the effect
would evaporate before the attack that's supposed to observe it, and the playtest script below would
fail in a way that looks like a rules bug but is actually a missing loader-table entry.

**Named toolkit scope:** give `HiddenCondition` and `HelpedCondition` `ToJSON`/`loadJSON` and register
both in `loader.go` + `factory.go`, mirroring `DodgingCondition` exactly. Add a round-trip test for
each (serialize → reload → still subscribed and functioning) as an explicit test-plan item, not an
implicit assumption folded into "add the condition."

### 5. Advantage/disadvantage is computed and then dropped on the floor — a #594-shaped bug

The "attacker has disadvantage visible" requirement turned out to already be data the toolkit
computes and then loses in translation — the same anti-pattern Invariant 6/7 exist to catch.
`combat.AttackResult` (`rulebooks/dnd5e/combat/attack.go:131-153`) already carries `HasAdvantage` and
`HasDisadvantage` (line 142-143) as real fields, correctly populated by `ResolveAttack`. But:
- `encounter.AttackOutcome` (`encounter/combat_resolver.go:230-250`, the `CombatResolver` interface's
  return contract) has no such fields.
- Both of rpg-api's `CombatResolver` implementations drop them explicitly when building the literal:
  `internal/handlers/dnd5e/v2/encounter/dnd5e_combat_resolver.go:149-157` and
  `dnd5e_combat_resolver_phased.go:169-179` both copy `Hit`/`Critical`/`AttackRoll`/`AttackBonus`/
  `TargetAC`/`Damage`/`DamageType` from `result` — never `result.HasAdvantage` /
  `result.HasDisadvantage`, which sit right there unused.
- `encounter.AttackResolvedEvent` (`encounter/events/attack_resolved.go`) has no such fields either.
- No proto field exists for it at all (`rpg-api-protos` has zero matches for advantage/disadvantage on
  the encounter surface).

Fix, three hops, no new rules logic: add `HasAdvantage bool`, `HasDisadvantage bool` (plus a
`[]*core.Ref` naming the granting/imposing condition(s), e.g. `refs.Conditions.Dodging()`, for
narration) to `encounter.AttackOutcome`; copy them in both rpg-api resolvers; add the same fields to
`encounter.AttackResolvedEvent` and populate them in `publishAttackOutcome`
(`encounter/combat.go:489-533`); add matching proto fields; project verbatim; render in the web's
attack breakdown. This is cheap precisely because the hard part (computing it) is already done and
correct — it only reads as large because it touches four files across two repos.

### 6. Turn-start is dead code — and reviving it lights up three subscribers, not one, and one of them is this wave's death gate

`DodgingCondition.onTurnStart` (`rulebooks/dnd5e/conditions/dodging.go:201-225`) subscribes to
`dnd5eEvents.TurnStartTopic` to remove itself at the start of the dodging character's next turn —
correct RAW. But the **live** turn-start path, `seedActorTurn` (`encounter/turn_economy.go:39-55`,
called from **both** `SetMode`'s first-actor seed at `encounter/combat.go:291` and `EndTurn`'s
next-actor seed at `encounter/combat.go:377`), only calls `char.StartTurn(...)` to mutate the economy —
it never publishes `TurnStartTopic` on `e.bus`. The only code that *does* publish it is
`combat.TurnManager.StartTurn` (`rulebooks/dnd5e/combat/turn_manager.go:246`) — legacy, no live callers
in the encounter SDK.

**This is not Dodge-local — three live subscribers wake up the moment the publish is revived, and the
design must name all three, not just the one this wave's verb needs:**

1. `rulebooks/dnd5e/conditions/dodging.go:85-86` — Dodging's self-removal (the reason this wave needs
   the revival at all).
2. `rulebooks/dnd5e/conditions/reckless_attack.go:73-74,181` — Barbarian Reckless Attack's
   self-removal. Out of scope for this wave's verbs, but it starts firing correctly (or, if there's a
   latent bug, starts firing incorrectly) the instant the publish goes live. **Add a regression check**
   that Reckless Attack still behaves correctly once turn-start is live — this wave doesn't touch
   Reckless Attack's code, but it does change whether the event that governs its lifecycle actually
   arrives.
3. `rulebooks/dnd5e/conditions/unconscious.go:60-61,145-182` — **auto-rolls death saves at the start of
   the unconscious character's turn.** This is not a side hazard: it is the actual death-save mechanism
   behind this wave's own "player gets hit → HP bar / death gate" playtest beat (`rpg-api#612`). Before
   this revival, an unconscious character's death saves never auto-roll on the live path — meaning
   `rpg-api#612`'s runtime confirmation (does the death gate actually fire?) was **silently depending
   on this same dead code** without the first draft of this doc naming the dependency. The two are the
   same fix, seen from two different verbs.

**Fix, and the connection made explicit:** revive the publish inside `seedActorTurn`, mirroring
`EndTurn`'s existing `dnd5eEvents.TurnEndTopic.On(e.bus).Publish(...)` at `combat.go:362` (same
function, same bus, same not-best-effort error handling — a failed turn-boundary publish should fail
the turn-start the same way a failed turn-end publish already does). The revived call publishes
`dnd5eEvents.TurnStartEvent{CharacterID: <the actor whose turn is starting>, Round: e.data.Round}` — the
same actor id `seedActorTurn` already receives as its `actorID` parameter — so all three subscribers'
`event.CharacterID != <mine>` guards key correctly with no new plumbing beyond the publish itself. The
playtest script (below) is updated to fold the Unconscious/death-save check into the same Beat 3 that
drives the Fighter to 0 HP, rather than treating turn-start revival and the death gate as unrelated
work items.

---

## RAW, precisely — and what Beat 2 implements vs defers

| Verb | RAW (2014 PHB p.192) | Beat 2 implements | Beat 2 defers |
|---|---|---|---|
| **Dodge** | Attacks against you have disadvantage (if you can see the attacker); you have advantage on DEX saves. Until the start of your next turn. | Disadvantage to all attackers, advantage on DEX saves, removed at next-turn-start. | The "if you can see the attacker" clause — no attacker-visibility gate (matches Dodging's existing code, which already ignores it; not new scope). |
| **Help** | Grant advantage on an ally's next ability check (if helping a task), OR on an ally's attack roll against a foe within 5 ft of you (if helping combat). Before the start of your next turn. | **Attack branch only** (post-review scope decision, see R4 below): target an ally, grant advantage on their next *attack roll*, consumed on that roll, safety-net removal at the **helper's** next turn if unused. | **The ability-check branch entirely** — no advantage on ability checks this wave (moved from "implemented" to "deferred" after the review gate found it unverifiable under this wave's own playtest script; see Resolved Decisions). The 5-ft-of-a-common-foe adjacency requirement for the attack branch that *is* shipped — no spatial validation this wave (mirrors Beat 1's stance on unvalidated ability preconditions). |
| **Hide** | Dexterity (Stealth) check vs each observer's passive Perception (or their active Perception check). Success: Hidden — attacks against you have advantage... wait, disadvantage; your attacks have advantage. Ends on attacking, making noise, or being seen. | One Stealth check (via new `checks.MakeAbilityCheck`, which also lands the shared `AbilityCheckChain` topic Help's deferred branch will eventually use) vs the *highest* passive Perception among opposing combatants within the encounter SDK's LOS stub. Binary success/fail. On success: advantage on the hider's attacks, disadvantage on attacks against them; removed when the hider makes their own attack. | Per-observer partial-hidden state (deferred to the wire's entity-override machinery, not built this wave); active-search Perception checks as a spot-attempt action; "seen while hidden" and "made noise" as break triggers (no LOS-change feed from rpg-api into the toolkit yet — see Wave item 3 above); the Hide-*failure* branch is verified by a toolkit unit test, not the live MCP playtest (see R6 in the fixture section). |

---

## Resolved decisions

| Decision | Resolution |
|---|---|
| **Dodge mechanism** | Reuse the Rage `ConditionAppliedEvent`-with-`Condition` pattern verbatim. No new plumbing. |
| **Cross-entity condition targeting — label** | `ConditionAppliedEvent.Target` already accepts any `core.Entity`; the gap is that nothing constructs one for a non-self target and `applyActivatedConditions` assumes self for the `targetID` label. Fix — this is the one reusable capability Help needs and future ally-targeted effects (Bless, Bardic Inspiration) will also need. |
| **Cross-entity condition targeting — audience (R1, added post-review)** | `applyActivatedConditions` also projects the broker audience from the *actor's* position, not the *target's*. `takeCharacterAction` must resolve and pass the target's `PlayerData` (not just its `core.Entity`) through, so audience is computed from `target.View.Position`. Ships in the same PR as the label fix — they're the same bug, caught by the same review. |
| **New-condition persistence (R3, added post-review)** | `HiddenCondition`/`HelpedCondition` get `ToJSON`/`loadJSON` + `loader.go`/`factory.go` registration, mirroring `DodgingCondition`, with a round-trip test each. Named toolkit scope, not an implicit "add the condition" assumption — without it the effects silently don't survive the RPC boundary the fixture's own beats cross. |
| **Ability-check machinery — build vs. don't (R4/Judgment 2, refined post-review)** | **Split, per the review gate's ruling:** build `MakeAbilityCheck` + the `AbilityCheckChain` topic now — justified by Hide's own Stealth roll alone, the same way `MakeSavingThrow`'s chain fires whether or not a subscriber exists. Do **not** ship `HelpedCondition`'s ability-check-advantage *subscriber* this wave — the fixture's Beat 3 only exercises Help's attack branch, so the check branch would ship unverified. Help is scoped to the attack branch only (see RAW table); the check branch + its `HelpedCondition` subscriber are deferred to a future wave whose fixture actually plays through it. |
| **Hide's observer set** | Computed by the encounter SDK (toolkit-owned, has positions) via the existing `perception.CanSeeAt` stub, passed into the ability as input. Rulebook layer stays spatially blind. **Guardrail (endorsed by the review gate):** the SDK *gathers* passive Perception via `Combatant.PassivePerception()` — it never inlines the `10+WIS+prof` formula itself. That formula is rulebook-owned; the SDK only calls the accessor. |
| **Hide's success granularity** | Binary (beat the highest passive Perception), not per-observer. Per-observer deferred to future wire work. |
| **Hide-failure playtest coverage (R6, added post-review)** | The live orchestrator uses a crypto-random roller with no deterministic seam in `devseed` (test-only mock rollers don't reach the live path). Rather than add a roller seam to `devseed` for this wave, the MCP playtest sign-off exercises **Hide-success only**; the Hide-*failure* branch (Stealth total below passive Perception) is covered by a toolkit unit test on `checks.MakeAbilityCheck` with a mock roller — the same pattern `saves.MakeSavingThrow`'s tests already use. This is a decided scope cut, not an open item. |
| **Advantage/disadvantage visibility** | Fixed at the source: `AttackOutcome` gains the fields `AttackResult` already computes; not recomputed, just no longer dropped. |
| **Condition display metadata (R5, added post-review)** | Stays **web-side**, matching current code (`translate.go:494-496`: rpg-api emits `StatusEffect.Source` as a bare `Ref`; the web resolves `display_name`/`icon_hint` from its own lookup table). rpg-api does **not** grow a `Ref → display_name` table for Hidden/Helped — that would be rpg-api authoring a user-facing rules string regardless of whether it's shaped as a conditional or a lookup table (Invariant 2). The web's existing lookup table gains entries for the two new refs; that's a web-side data change, not new architecture. |
| **Turn-start revival — scope (R2, refined post-review)** | Lands in `seedActorTurn`, mirroring `EndTurn`'s existing `TurnEndTopic` publish exactly — same function shape, same failure semantics. **Named as lighting up three subscribers** (Dodging, Reckless Attack, Unconscious's auto-death-save), not framed as Dodge-local. The Unconscious subscriber is load-bearing for this wave's own death-gate playtest beat — the revival and `rpg-api#612` are the same fix from two angles, not two unrelated work items. |
| **Persistence** | Deferred (Invariant 13), unchanged from Beat 1. `rpg-api#596` stays parked; this wave assumes the current persist cascade round-trips condition state correctly (verified by code trace, not re-litigated here) — except for the two brand-new condition types, which need their own registration per the row above. |

---

## Work by layer

| Layer | Work |
|---|---|
| **rpg-toolkit** *(the core of the wave)* | `Dodge.Activate` publishes `ConditionAppliedEvent`; new `ConditionDodging`/`ConditionHidden`/`ConditionHelped` types + `ConditionSourceCombatAbility`; new `refs.Conditions.Hidden()`/`.Helped()`. Thread `TargetID`/`Target core.Entity` through `ActivateAbilityInput` → `CombatAbilityInput` → `Help.Activate`. Fix `applyActivatedConditions`'s hardcoded `targetID := actorID` **and** its actor-sourced audience projection — thread the target's `PlayerData` through so audience comes from the target's position (R1). New `checks` package (`MakeAbilityCheck`) + `AbilityCheckChainEvent`/`AbilityCheckChain`, fired by Hide's own roll. `Character.PassivePerception()` + `Monster.PassivePerception()` + `Combatant` interface method (SDK calls this, never inlines the formula). New `HiddenCondition` (AttackChain, both directions) and `HelpedCondition` (**AttackChain only this wave** — no `AbilityCheckChain` subscriber, per R4/Judgment 2). **`ToJSON`/`loadJSON` + `loader.go`/`factory.go` registration for both new conditions, with round-trip tests (R3)** — not implicit. Encounter SDK computes Hide's observer set via `perception.CanSeeAt` and resolves Help's ally `core.Entity` **and `PlayerData`**. `AttackOutcome` + `AttackResolvedEvent` gain `HasAdvantage`/`HasDisadvantage`/source refs; both rpg-api-side resolver call sites are unaffected here (toolkit only adds the fields — rpg-api copies them, see below). Revive `TurnStartTopic` publish in `seedActorTurn`, stamping `CharacterID` = the actor whose turn is starting — **document the three live subscribers this wakes (Dodging, Reckless Attack, Unconscious death-saves) and add a Reckless Attack regression check (R2).** |
| **rpg-api-protos** | Advantage/disadvantage fields (+ source `Ref`s) on the attack-resolved event. No new condition-display fields — `StatusEffect.Source` (a bare `Ref`) is already sufficient; display resolution stays web-side (R5). No new RPCs — this wave is entirely event/projection surface. |
| **rpg-api** | Copy `result.HasAdvantage`/`result.HasDisadvantage` into `AttackOutcome` in **both** `dnd5e_combat_resolver.go:149-157` and `dnd5e_combat_resolver_phased.go:169-179` (currently silently dropped). Project the new attack-resolved fields verbatim (zero rules conditionals). **No condition display metadata added here (R5)** — rpg-api forwards the condition `Ref` unchanged, same as today. **Seed player HP at `AddPlayer`** (`internal/handlers/dnd5e/v2/encounter/create.go:55-60`) and reconcile on hydrate, mirroring `syncMonsterDataFromSnapshot` (`encounter/npc.go:200-229`) — `rpg-api#612`, required for this wave's "player gets hit" scenario to show a correct HP bar / working death gate, not deferred, and now explicitly tied to the turn-start revival (R2) that makes the Unconscious death-save mechanism live. Bump `encounter`/`rulebooks/dnd5e` to include `#724` (OA weapon threading, merged 2026-07-03, unreleased) as part of the version bump this wave ships anyway. |
| **rpg-dnd5e-web** | Render status effects on entities (Dodging/Hidden/Helped icons — the wire contract already exists per `design.md` §2/§5, `StatusApplied`; this wave populates it with real data for the first time on these three refs). **Add `display_name`/`icon_hint` entries for Hidden/Helped to the web's existing ref-keyed lookup table (R5)** — the same pattern already used for other conditions, not new architecture. Render advantage/disadvantage in the attack-roll breakdown, with the source condition's display name ("disadvantage: Dodging"). Render the HP bar and death gate honestly once `rpg-api#612` seeds real values and turn-start revival makes death saves live — this is verification, not new UI work (the components exist per `PlaytestHarness.tsx`/`EconomyBar.tsx`). |

---

## The devseed fixture + playtest script (the sign-off bar)

**New named fixture: `wave-2-beat2`.** Parameterized devseed (per `feedback_devseed_fixture_per_wave` —
extend the existing devseed parameterization, don't hand-roll a new cast). Cast: reuse the Monk from
`wave-2-monk` plus a second player character (any of the other three brothers — Fighter recommended,
simplest to read a "gets hit" beat off) and one goblin with a real attack bonus, positioned so the
goblin can reach the Fighter in one move.

**R6 scope decision, stated once here:** this script exercises Hide's **success** path only, live. The
**failure** path (Stealth total below passive Perception) is not reproducible deterministically on the
live orchestrator (crypto-random roller, no seeded-roller seam in `devseed`) and is covered instead by
a toolkit unit test on `checks.MakeAbilityCheck` with a mock roller. Position the goblin's passive
Perception comfortably below the Monk's expected Stealth total so the live success path is reliable,
not probabilistic.

**Script (single MCP session, three beats — Beat 3 now explicitly a two-part beat per R2/R4):**
1. **Dodge beat.** Fighter takes Dodge. Goblin attacks the Fighter. Assert: attack-resolved event
   carries `has_disadvantage: true` with `Dodging` as the source; the web's attack breakdown shows it;
   the roll uses 2d20-take-lower; Dodging clears at the start of the Fighter's next turn (the revived
   `TurnStartTopic` publish — assert the status-removed event/render, not just the grant).
2. **Hide beat.** Monk takes Hide (goblin as the only observer, passive Perception seeded below the
   Monk's expected Stealth total for a reliable success). Assert: Hidden status renders on the Monk;
   the goblin's subsequent attack against the Monk carries `has_disadvantage: true`. On the Monk's next
   attack: advantage is visible, then Hidden clears (verify the status-removed event/render).
3. **Help + hit + death-gate beat.** Monk takes Help targeting the Fighter (**attack branch only** —
   the prompt/target is the foe the Fighter is about to attack, per R4's scoped-down Help). Assert the
   broker-visible condition is attributed to the **Fighter** (not the Monk — the regression test for
   the `applyActivatedConditions` label fix) and that a **third party** (a different viewer than the
   Fighter or Monk, if the fixture's cast allows, or explicitly reasoned about via the audience
   computation) sees it based on the **Fighter's** position, not the Monk's — the regression test for
   the audience fix (R1). Fighter attacks the goblin with advantage visible, consuming the Helped
   condition. Goblin attacks the Fighter and **hits** — the "player gets hit" beat: assert the
   Fighter's HP bar reflects real damage (not 0/0). Drive the Fighter to 0 HP and confirm the
   **Unconscious condition's auto-death-save fires at the start of the Fighter's next turn** (via the
   same revived `TurnStartTopic` publish from Beat 1's turn-start work) — this is the concrete,
   in-script confirmation of `rpg-api#612` and the R2 turn-start blast radius, not a separate
   unconnected check.

---

## Issue breakdown (one issue per PR, rolling up to a new wave issue under `rpg-project#54`)

A new **wave issue** ("Wave: Mechanical Effects") is filed rolling up under the umbrella
`rpg-project#54`, mirroring how #54 itself served as Beat 1's wave issue with four layer issues rolling
up to it (see N3 below on the umbrella-framing nit). Layer issues:

- **rpg-toolkit** — likely 2-3 PRs given the size difference between pieces:
  - Dodge + turn-start revival: `#699` (already filed) — publish `ConditionAppliedEvent`, add
    `ConditionDodging`/`ConditionSourceCombatAbility`, revive `TurnStartTopic` publish in
    `seedActorTurn`. **Bundled here because Dodge is the verb whose self-removal needs it, but the PR
    description and tests must cover all three live subscribers** (Dodging, Reckless Attack regression
    check, Unconscious death-save behavior) **per R2** — not scoped as if only Dodge exists.
  - Help + Hide: `#716` (already filed) — target-threading plumbing (label **and** audience, per R1),
    `checks` package + `AbilityCheckChain` topic (fired by Hide's roll only, per R4),
    `HiddenCondition`, `HelpedCondition` (AttackChain subscriber only, no check-branch this wave),
    `PassivePerception()` on `Character`/`Monster`/`Combatant`, observer-set computation,
    `applyActivatedConditions` fix, **and loader/factory registration + round-trip tests for both new
    conditions (R3)**. This is the wave's largest PR; splitting target-threading (a small, mechanical,
    low-risk plumbing PR) from the two new conditions + their registration (rules logic) is a
    reasonable session-sized cut if one session can't hold it all.
  - AttackResolved fidelity: a new toolkit issue for `AttackOutcome`/`AttackResolvedEvent` gaining
    advantage/disadvantage fields — separable from the condition work, same shape as Beat 1's
    "event-faithfulness work ships as its own PR" precedent.
- **rpg-api-protos** — a new issue for the advantage/disadvantage proto fields only. No condition
  display-metadata field is added (R5) — the existing `StatusEffect.Source` `Ref` is sufficient.
- **rpg-api** — a new issue bundling: copy advantage/disadvantage into both resolver call sites,
  project verbatim, the version bump (pulling in `#724`). No condition display metadata added here
  (R5). `rpg-api#612` (player HP seeding) is filed separately (already exists) but is **required for
  this wave's sign-off playtest** and is now explicitly connected to the turn-start revival (R2), so it
  rolls up here too rather than floating independently.
- **rpg-dnd5e-web** — a new issue for status-effect rendering (including new lookup-table entries for
  Hidden/Helped display names/icons, per R5) + attack-breakdown advantage/disadvantage display.

---

## Non-goals / deferred (consolidated)

- Help's ability-check-advantage branch and its `HelpedCondition` subscriber (R4) — ships attack-branch
  only this wave; the check branch needs a fixture beat that actually exercises it, deferred to a wave
  that has one.
- The Hide-*failure* branch as a live MCP playtest assertion (R6) — covered by a toolkit unit test on
  `checks.MakeAbilityCheck` instead; no deterministic-roller seam added to `devseed` this wave.
- Per-observer partial-Hidden state (needs wire entity-override machinery beyond this wave).
- Active-search Perception checks as a player-triggered spot attempt.
- "Seen while hidden" / "made noise" break triggers requiring an LOS-change feed from rpg-api into the
  toolkit (no such feed exists yet). **Combined effect, named explicitly (N2):** with these breaks
  deferred and no turn-start expiry for Hidden, a hidden character who only moves (never attacks) stays
  mechanically Hidden indefinitely, and a character standing in the open still reads Hidden if nothing
  ever cleared it. The playtest script's tested path (the Monk clears Hidden via its own attack) doesn't
  hit either edge case — naming both here so an implementer doesn't rediscover them as surprises, and so
  nobody mistakes "the playtest passed" for "these edges are handled."
- Help's RAW adjacency requirement for the attack branch that *is* shipped (unvalidated, same stance as
  Beat 1 took on unvalidated ability preconditions generally).
- Dodge's "if you can see the attacker" clause (pre-existing gap in the already-shipped
  `DodgingCondition` code, not new to this wave).
- Real LOS (walls, lighting) — `perception.CanSeeAt` stays the range-only stub; Hide's observer-set
  computation inherits that limitation, named not hidden.
- Condition display-name/icon metadata living in rpg-api (R5) — deliberately not built; stays web-side,
  matching current code.
- `rpg-api#596` (persistence one-home migration) — parked, unrelated, not touched.
- `rpg-toolkit#722`/`#723` (OA damage/correlation bugs) — already fixed and merged as `#724`; consumed
  via version bump, not re-investigated here.
- Any condition beyond Dodging/Hidden/Helped. Sneak Attack, Frightened, Poisoned, etc. are out of
  scope; this wave is scoped to the three verbs the 2026-07-01 playtest evidence named.

## Open items (tracked under the wave, not blockers)

- **Unsubscribe-on-first-use timing.** `HiddenCondition` (self-removal on its own next attack) and
  `HelpedCondition` (single-use-or-helper's-next-turn removal, attack branch only now) both need
  "consume on next qualifying roll" semantics that existing self-terminating conditions (Dodging,
  Raging) don't quite have a precedent for (they remove on a *different* event — turn boundary or
  rest — not the same chain event that granted the bonus). The exact unsubscribe-from-within-your-own-
  handler mechanics are an implementation detail for the implementer to work out, flagged here so it
  isn't rediscovered mid-session.
- **Reaction-pause interplay** — cross-referenced from Beat 1, still not this wave's concern, but
  Dodge's disadvantage-to-attacker interacts with the same `AttackChain` reactions (Shield) subscribe
  to; no known conflict, just flagged as adjacent surface.

## Related / inherited

- Builds on Beat 1 (`rpg-project#54`, `ideas/encounter/v1alpha2/take-action/design.md`) — the
  resolved-action event, correlation id, and target-kind menu plumbing are reused as-is, not
  re-derived.
- Closes: `rpg-toolkit#716`, `rpg-toolkit#699`.
- Required for sign-off, ships alongside: `rpg-api#612` (player HP seeding) — now explicitly tied to
  the turn-start revival's Unconscious subscriber (R2), not an independent fix.
- Consumed, not re-worked: `rpg-toolkit#724` (OA weapon threading, merged 2026-07-03).
- Adjacent, explicitly not this wave: `rpg-toolkit#722`/`#723` (already resolved by #724, cross-
  referenced only), `rpg-api#596` (parked).
- **N3 (accuracy nit, not required):** `rpg-project#54`'s issue title is literally "Wave: TakeAction
  end-to-end" (Beat 1's own wave issue) yet doubles as the standing Chapter 2 umbrella — `#716`/`#699`
  are already filed as "part of #54" under that dual use. Nesting Beat 2's new wave issue under it is
  consistent with that existing precedent but muddy; a dedicated chapter-umbrella issue (separate from
  any single beat's wave issue) would be cleaner. Not required for this wave; noted for whoever next
  restructures the board.

---

## North-Star Invariants self-check

This section reflects the review gate's actual 13-point walk, not a re-derivation — PASS items are
restated briefly; the three the gate marked AT RISK / PASS-with-caveat are resolved by the required
changes above and the resolution is stated inline.

1. **Web computes nothing.** PASS. Web renders `has_advantage`/`has_disadvantage` and status effects as
   server-sent booleans/refs; it never infers disadvantage from "an attacker is Dodging" client-side.
2. **rpg-api authors zero rules strings.** Was AT RISK (the first draft proposed a `Ref → display_name`
   table living in rpg-api, which is rules-string authoring regardless of table-vs-conditional shape).
   **Resolved by R5:** condition display metadata stays web-side, matching current code
   (`translate.go:494-496`); rpg-api forwards the bare `Ref` unchanged. Advantage/disadvantage fields
   remain a verbatim copy, which was never at risk.
3. **Toolkit owns the resolution.** PASS, with the guardrail below made explicit rather than assumed.
   The Stealth check, the opposed comparison, and the advantage/disadvantage grants all live in
   `rulebooks/dnd5e`. The observer-set computation (who's watching) lives in the toolkit-owned encounter
   SDK because it needs positions the rulebook layer doesn't have — **endorsed by the review gate**,
   conditioned on passive Perception being *gathered* via `Combatant.PassivePerception()`, never
   recomputed inline in the SDK. That guardrail is now stated explicitly in §3 and the Resolved
   Decisions table, not left implicit.
4. **Protos + rpg-api are one unit.** PASS. The advantage/disadvantage proto work is named as
   first-class in the issue breakdown; no condition-metadata proto work exists to fold in (R5 keeps it
   web-side, so there's nothing here to hide).
5-8. **Events are canonical, causation-carrying.** PASS. The new fields ride the existing
   correlation-id/timestamp spine from Beat 1 — no new correlation mechanism invented.
   `ConditionAppliedEvent` for Help correctly names its `Target` (the ally) for causation, and — new
   since the first draft — the *audience* projection for that event now also reads from the target's
   position, not the actor's (R1), so the event is both causally and perceptually correct.
9. **First-class resolved-action event.** PASS/N-A. Already satisfied by Beat 1's `ActionResolvedEvent`;
   this wave adds effect fidelity, not a new resolved-action shape.
10. **RPC responses stay minimal acks.** PASS. Unchanged — no new RPCs this wave.
11-12. **Capability / menu.** PASS. `targetKindForRef` already anticipates Help's
   `TargetKindSingleEntity`; this wave fulfills what the menu already promised rather than changing the
   menu contract.
13. **Persistence deferred, contract forward-compatible.** Was PASS-with-caveat (no new interface bakes
   in per-RPC persistence, but the two brand-new conditions were not named as needing loader/factory
   registration to survive the existing persist cascade). **Resolved by R3:** `HiddenCondition` and
   `HelpedCondition` get `ToJSON`/`loadJSON` + `loader.go`/`factory.go` registration and a round-trip
   test each, mirroring `DodgingCondition`, named explicitly as toolkit scope rather than assumed.
   `rpg-api#596` stays parked, unaffected.

**Judgment calls, both resolved by the review gate (not left open):**
- **Invariant-3 placement** (Hide's observer set in the encounter SDK): **endorsed**, with the
  `Combatant.PassivePerception()` guardrail above. No longer framed as an unresolved tension in this
  revision — the gate's ruling is the answer.
- **`checks`-package scope** (build vs. over-build): **split** — build `MakeAbilityCheck` + the
  `AbilityCheckChain` topic now, justified by Hide's own roll alone; do not ship `HelpedCondition`'s
  check-branch subscriber this wave, since the fixture never exercises it (R4). Encoded throughout the
  doc above, not just here.
