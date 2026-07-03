# Mechanical Effects — Beat 2

**Chapter 2 · v1alpha2 encounter route**
**Validated against:** the North-Star Invariants in [`../design.md`](../design.md). Self-check at the
end of this doc; awaiting the adversarial review gate before it becomes the implementer's star.
**Status:** wave content — draft for review.
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
tickets. They aren't. Reading the actual code surfaced five pieces of unequal size, one of which is
net-new toolkit spine. Naming them here so none can hide behind "implement Help/Hide/Dodge" again.

### 1. Dodge is (almost) free — reuse the proven pattern, don't reinvent it

`DodgingCondition` (`rulebooks/dnd5e/conditions/dodging.go`) is fully implemented and correct — it
already subscribes `AttackChain` (disadvantage to attackers), `SavingThrowChain` (DEX-save advantage),
and `TurnStartTopic` (self-removal). `Dodge.Activate()` (`combatabilities/dodge.go:78-101`) already
publishes `DodgeActivatedEvent`. Nothing constructs the condition or applies it.

The fix is **not** a new wiring mechanism. Rage already proves the pattern end to end:
`Rage.Activate()` (`rulebooks/dnd5e/features/rage.go:105-146`) constructs `*conditions.RagingCondition`
and publishes it via `dnd5eEvents.ConditionAppliedTopic` with `Condition: ragingCondition` set. Two
subscribers already exist and require **zero new plumbing**:
- `Character.onConditionApplied` (`character/character.go:1062-1078`, wired in `subscribeToEvents` at
  line 1019) filters on `event.Target.GetID() == c.id` and calls `event.Condition.Apply(ctx, c.bus)` —
  this is what actually subscribes `RagingCondition`'s (and would subscribe `DodgingCondition`'s)
  handlers onto the bus.
- `subscribeConditions` / `applyActivatedConditions` (`encounter/npc.go:1187`,
  `encounter/activate_feature.go:205`, already invoked by `takeCharacterAction` at
  `encounter/take_character_action.go:70-83`) bridges the same `ConditionAppliedEvent` to the broker
  so the client sees it.

So Dodge's entire fix is: `Dodge.Activate()` additionally constructs
`conditions.NewDodgingCondition(owner.GetID())` and publishes it via `ConditionAppliedTopic` —
identical shape to Rage's four lines. `DodgeActivatedEvent` stays (still useful as a narration signal);
`ConditionAppliedEvent` is added alongside it. This needs two small taxonomy additions: `ConditionType`
has no `ConditionDodging` value (`events.go:18-70` — Raging, RecklessAttack, FightingStyle exist,
Dodging/Hidden don't), and `ConditionSource` has only `ConditionSourceClass`/`ConditionSourceFeature`
(`events.go:73-80` — no `ConditionSourceCombatAbility`). Both are one-line additions.

### 2. Help needs cross-entity condition targeting (new capability, small blast radius)

Every existing self-applying condition (Raging, RecklessAttack, and now Dodging above) targets its own
activator. Help's condition targets **someone else** — the ally. Three real gaps, all precise:

- **No target reaches the ability at all.** `CombatAbilityInput` (`combatabilities/input.go`) carries
  `Bus`, `ActionEconomy`, `ActionHolder`, `Speed`, `ExtraAttacks` — no target. `ActivateAbilityInput`
  (`character/action_economy_types.go:116`) carries only `AbilityRef`. Compare
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
- **`applyActivatedConditions` hardcodes the wrong target.** `encounter/activate_feature.go:210-212`:
  `targetID := actorID` — always. This is correct today only because every condition that currently
  flows through it self-applies (Dodge, once wired, still self-applies). Help's condition does not:
  the broker-visible "who has this condition" projection must read `cond.Target.GetID()`, not the
  actor. Left unfixed, a helped ally's condition would render as if the *helper* had it — a real
  observability bug, not a hypothetical. **This is in-scope plumbing for Beat 2**, surfaced by Help
  specifically (Dodge and Hide both self-apply and wouldn't have caught it).
- **Target-kind is already anticipated on the wire.** `targetKindForRef`
  (`character/action_economy.go:174-193`) already tags `refs.CombatAbilities.Help()` as
  `TargetKindSingleEntity` (grouped with Attack) and Dodge/Disengage/Hide as `TargetKindSelf`. The menu
  plumbing from Beat 1 already expects this — it's the activation path that's missing the wiring, not
  the wire contract.

### 3. Hide needs check machinery the toolkit doesn't have yet — this is the largest piece

There is no ability-check resolution in the toolkit today. `saves.MakeSavingThrow`
(`rulebooks/dnd5e/saves/saves.go`) is the only precedent: roller + modifier + chain-sourced
advantage/disadvantage/bonuses + nat-1/nat-20 detection, firing `SavingThrowChainEvent` through
`SavingThrowChain` when a bus is present. `Character.GetSkillModifier(skill)`
(`character/character.go:184-198`) already computes the Stealth modifier (ability + proficiency); there
is no `Character.PassivePerception()` (only `Monster.Data.PassivePerception` exists as a static field,
no accessor, and no `Combatant` interface method — `combat/combatant.go:54-79` has no perception
method at all).

**Named toolkit spine addition:** a `checks` package mirroring `saves`, with `MakeAbilityCheck`
(same roll/advantage/disadvantage/chain shape as `MakeSavingThrow`) and a new
`AbilityCheckChainEvent` / `AbilityCheckChain` topic in `events.go` (same shape as
`SavingThrowChainEvent`: `CheckerID`, `Skill`, `DC`, `AdvantageSources`/`DisadvantageSources`/
`BonusSources`). This is not speculative infrastructure — Help's advantage grant (below) is the first
real subscriber, so the chain earns its place by serving this wave's own two verbs, not a hypothetical
future one. `PassivePerception()` is added to `Character` (`10 + WIS mod + proficiency`, mirroring
`GetSkillModifier`) and to `Monster` (from the existing static field), and to the `Combatant` interface
so Hide's check can compare against any observer type generically.

**Observer set — a boundary call, not a deferred choice.** Hide's Stealth check needs to know who's
watching. The toolkit's `encounter` SDK package already has hex positions and an LOS gate
(`encounter/perception.CanSeeAt`, currently a range-only stub — "ignores walls, lighting, conditions,"
per its own doc comment) — it's already used to gate `publishAttackOutcome`'s audience. The rulebook
layer (`rulebooks/dnd5e`) has no spatial awareness and shouldn't grow one. So: the **encounter SDK**
(still toolkit-owned, not rpg-api) computes the observer set — opposing combatants within
`perception.CanSeeAt` of the hider — and passes their ids + passive Perception values into the
ability as input, the same way it already computes attacker/target visibility for damage events. This
is the existing encounter-owns-position / rulebook-owns-rules split, applied one step earlier than
usual (pre-check instead of post-resolution) — not a new pattern.

**Simplification, stated plainly:** Hide succeeds only if the Stealth total beats the *highest*
passive Perception among current observers — binary, not per-observer. RAW is per-observer (hidden
from some, seen by others); that needs the wire's tiered-visibility / entity-override machinery from
`design.md` §2 ("Hidden traps and illusions are modeled as entity-overrides") extended to characters,
which is a v1alpha2 wire feature this wave does not build. Named as deferred below, not silently cut.

### 4. Advantage/disadvantage is computed and then dropped on the floor — a #594-shaped bug

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
narration) to `AttackOutcome`; copy them in both rpg-api resolvers; add the same fields to
`AttackResolvedEvent` and populate them in `publishAttackOutcome`
(`encounter/combat.go:489-533`); add matching proto fields; project verbatim; render in the web's
attack breakdown. This is cheap precisely because the hard part (computing it) is already done and
correct — it only reads as large because it touches four files across two repos.

### 5. Turn-start is dead code, and Dodge's self-removal depends on it

`DodgingCondition.onTurnStart` (`conditions/dodging.go:201-225`) subscribes to `dnd5eEvents
.TurnStartTopic` to remove itself at the start of the dodging character's next turn — correct RAW.
But the **live** turn-start path, `seedActorTurn` (`encounter/turn_economy.go:39-55`, called from both
`SetMode`'s first-actor seed and `EndTurn`'s next-actor seed), only calls `char.StartTurn(...)` to
mutate the economy — it never publishes `TurnStartTopic` on `e.bus`. The only code that *does* publish
it is `combat.TurnManager.StartTurn` (`rulebooks/dnd5e/combat/turn_manager.go:246`) — legacy, no live
callers in the encounter SDK. Without a live publish, Dodging (and any future turn-start-scoped
condition, including a turn-scoped Hidden if this wave were to add one) never expires — it would sit
disadvantaging attackers forever.

**Named toolkit scope, not an implementer surprise:** revive the publish inside `seedActorTurn`,
mirroring `EndTurn`'s existing `dnd5eEvents.TurnEndTopic.On(e.bus).Publish(...)` at `combat.go:362`
(same function, same bus, same not-best-effort error handling — a failed turn-boundary publish should
fail the turn-start the same way a failed turn-end publish already does).

---

## RAW, precisely — and what Beat 2 implements vs defers

| Verb | RAW (2014 PHB p.192) | Beat 2 implements | Beat 2 defers |
|---|---|---|---|
| **Dodge** | Attacks against you have disadvantage (if you can see the attacker); you have advantage on DEX saves. Until the start of your next turn. | Disadvantage to all attackers, advantage on DEX saves, removed at next-turn-start. | The "if you can see the attacker" clause — no attacker-visibility gate (matches Dodging's existing code, which already ignores it; not new scope). |
| **Help** | Grant advantage on an ally's next ability check (if helping a task), OR on an ally's attack roll against a foe within 5 ft of you (if helping combat). Before the start of your next turn. | Target an ally; grant advantage on their next attack roll or ability check (whichever comes first), consumed on that roll; safety-net removal at the **helper's** next turn if unused. | The 5-ft-of-a-common-foe adjacency requirement for the combat branch, and the "must be helping with a task you could actually help with" gate for the check branch — no spatial/task validation this wave (mirrors Beat 1's stance on unvalidated ability preconditions). |
| **Hide** | Dexterity (Stealth) check vs each observer's passive Perception (or their active Perception check). Success: Hidden — attacks against you have advantage... wait, disadvantage; your attacks have advantage. Ends on attacking, making noise, or being seen. | One Stealth check (via new `checks.MakeAbilityCheck`) vs the *highest* passive Perception among opposing combatants within the encounter SDK's LOS stub. Binary success/fail. On success: advantage on the hider's attacks, disadvantage on attacks against them; removed when the hider makes their own attack. | Per-observer partial-hidden state (deferred to the wire's entity-override machinery, not built this wave); active-search Perception checks as a spot-attempt action; "seen while hidden" and "made noise" as break triggers (no LOS-change feed from rpg-api into the toolkit yet — see Wave item 3 above). |

---

## Resolved decisions

| Decision | Resolution |
|---|---|
| **Dodge mechanism** | Reuse the Rage `ConditionAppliedEvent`-with-`Condition` pattern verbatim. No new plumbing. |
| **Cross-entity condition targeting** | `ConditionAppliedEvent.Target` already accepts any `core.Entity`; the gap is that nothing constructs one for a non-self target and `applyActivatedConditions` assumes self. Fix both — this is the one reusable capability Help needs and future ally-targeted effects (Bless, Bardic Inspiration) will also need. |
| **Ability-check machinery** | Build `checks.MakeAbilityCheck` + `AbilityCheckChain` now, justified by Help's advantage grant being a real, immediate subscriber — not spec'd ahead of need. |
| **Hide's observer set** | Computed by the encounter SDK (toolkit-owned, has positions) via the existing `perception.CanSeeAt` stub, passed into the ability as input. Rulebook layer stays spatially blind. |
| **Hide's success granularity** | Binary (beat the highest passive Perception), not per-observer. Per-observer deferred to future wire work. |
| **Advantage/disadvantage visibility** | Fixed at the source: `AttackOutcome` gains the fields `AttackResult` already computes; not recomputed, just no longer dropped. |
| **Turn-start revival** | Lands in `seedActorTurn`, mirroring `EndTurn`'s existing `TurnEndTopic` publish exactly — same function shape, same failure semantics. |
| **Persistence** | Deferred (Invariant 13), unchanged from Beat 1. `rpg-api#596` stays parked; this wave assumes the current persist cascade round-trips condition state correctly (verified by code trace, not re-litigated here). |

---

## Work by layer

| Layer | Work |
|---|---|
| **rpg-toolkit** *(the core of the wave)* | `Dodge.Activate` publishes `ConditionAppliedEvent`; new `ConditionDodging`/`ConditionHidden`/`ConditionHelped` types + `ConditionSourceCombatAbility`; new `refs.Conditions.Hidden()`/`.Helped()`. Thread `TargetID`/`Target core.Entity` through `ActivateAbilityInput` → `CombatAbilityInput` → `Help.Activate`. Fix `applyActivatedConditions`'s hardcoded `targetID := actorID`. New `checks` package (`MakeAbilityCheck`) + `AbilityCheckChainEvent`/`AbilityCheckChain`. `Character.PassivePerception()` + `Monster.PassivePerception()` + `Combatant` interface method. New `HiddenCondition` (AttackChain, both directions) and `HelpedCondition` (AttackChain + AbilityCheckChain, single-use-or-helper's-next-turn removal). Encounter SDK computes Hide's observer set via `perception.CanSeeAt` and resolves Help's ally `core.Entity`. `AttackOutcome` + `AttackResolvedEvent` gain `HasAdvantage`/`HasDisadvantage`/source refs; both rpg-api-side resolver call sites are unaffected here (toolkit only adds the fields — rpg-api copies them, see below). Revive `TurnStartTopic` publish in `seedActorTurn`. |
| **rpg-api-protos** | Advantage/disadvantage fields (+ source `Ref`s) on the attack-resolved event. `display_name`/`icon_hint` entries for the new condition refs (Dodging, Hidden, Helped) wherever that table lives. No new RPCs — this wave is entirely event/projection surface. |
| **rpg-api** | Copy `result.HasAdvantage`/`result.HasDisadvantage` into `AttackOutcome` in **both** `dnd5e_combat_resolver.go:149-157` and `dnd5e_combat_resolver_phased.go:169-179` (currently silently dropped). Project the new attack-resolved fields verbatim (zero rules conditionals). Add condition display metadata for Hidden/Helped. **Seed player HP at `AddPlayer`** (`internal/handlers/dnd5e/v2/encounter/create.go:55-60`) and reconcile on hydrate, mirroring `syncMonsterDataFromSnapshot` (`encounter/npc.go:200-229`) — `rpg-api#612`, required for this wave's "player gets hit" scenario to show a correct HP bar / working death gate, not deferred. Bump `encounter`/`rulebooks/dnd5e` to include `#724` (OA weapon threading, merged 2026-07-03, unreleased) as part of the version bump this wave ships anyway. |
| **rpg-dnd5e-web** | Render status effects on entities (Dodging/Hidden/Helped icons — the wire contract already exists per `design.md` §2/§5, `StatusApplied`; this wave populates it with real data for the first time on these three refs). Render advantage/disadvantage in the attack-roll breakdown, with the source condition's display name ("disadvantage: Dodging"). Render the HP bar and death gate honestly once `rpg-api#612` seeds real values — this is verification, not new UI work (the components exist per `PlaytestHarness.tsx`/`EconomyBar.tsx`). |

---

## The devseed fixture + playtest script (the sign-off bar)

**New named fixture: `wave-2-beat2`.** Parameterized devseed (per `feedback_devseed_fixture_per_wave` —
extend the existing devseed parameterization, don't hand-roll a new cast). Cast: reuse the Monk from
`wave-2-monk` plus a second player character (any of the other three brothers — Fighter recommended,
simplest to read a "gets hit" beat off) and one goblin with a real attack bonus, positioned so the
goblin can reach the Fighter in one move.

**Script (single MCP session, three beats):**
1. **Dodge beat.** Fighter takes Dodge. Goblin attacks the Fighter. Assert: attack-resolved event
   carries `has_disadvantage: true` with `Dodging` as the source; the web's attack breakdown shows it;
   the roll uses 2d20-take-lower.
2. **Hide beat.** Monk takes Hide (goblin as the only observer, positioned so passive Perception is
   knowable and can be tuned to force one success and one failure run — run the fixture with a goblin
   whose passive Perception straddles the Monk's likely Stealth total so both branches are exercised
   across two runs, or seed the roller deterministically for the failure case). On success: Hidden
   status renders on the Monk; the goblin's subsequent attack against the Monk carries
   `has_disadvantage: true`. On the Monk's next attack: advantage is visible, then Hidden clears
   (verify the status-removed event/render).
3. **Help + hit beat.** Monk takes Help targeting the Fighter. Assert the broker-visible condition is
   attributed to the **Fighter** (not the Monk — this is the regression test for the
   `applyActivatedConditions` fix). Fighter attacks the goblin with advantage visible. Goblin attacks
   the Fighter and **hits** — this is the "player gets hit" beat: assert the Fighter's HP bar reflects
   real damage (not 0/0) and, if driven to 0, the death-gate events fire correctly (`rpg-api#612`
   runtime confirmation).

---

## Issue breakdown (one issue per PR, rolling up to a new wave issue under `rpg-project#54`)

A new **wave issue** ("Wave: Mechanical Effects") is filed rolling up under the umbrella
`rpg-project#54`, mirroring how #54 itself served as Beat 1's wave issue with four layer issues rolling
up to it. Layer issues:

- **rpg-toolkit** — likely 2-3 PRs given the size difference between pieces:
  - Dodge: `#699` (already filed) — publish `ConditionAppliedEvent`, add `ConditionDodging`/
    `ConditionSourceCombatAbility`, revive `TurnStartTopic` publish in `seedActorTurn` (bundled here
    since Dodge is the only verb that needs it this wave).
  - Help + Hide: `#716` (already filed) — target-threading plumbing, `checks` package +
    `AbilityCheckChain`, `HiddenCondition`, `HelpedCondition`, `PassivePerception()`, observer-set
    computation, `applyActivatedConditions` fix. This is the wave's largest PR; splitting
    target-threading (a small, mechanical, low-risk plumbing PR) from the two new conditions (rules
    logic) is a reasonable session-sized cut if one session can't hold it all.
  - AttackResolved fidelity: a new toolkit issue for `AttackOutcome`/`AttackResolvedEvent` gaining
    advantage/disadvantage fields — separable from the condition work, same shape as Beat 1's
    "event-faithfulness work ships as its own PR" precedent.
- **rpg-api-protos** — a new issue for the advantage/disadvantage proto fields + condition display
  metadata.
- **rpg-api** — a new issue bundling: copy advantage/disadvantage into both resolver call sites,
  project verbatim, condition display metadata, the version bump (pulling in `#724`). `rpg-api#612`
  (player HP seeding) is filed separately (already exists) but is **required for this wave's sign-off
  playtest**, so it rolls up here too rather than floating independently.
- **rpg-dnd5e-web** — a new issue for status-effect rendering + attack-breakdown advantage/disadvantage
  display.

---

## Non-goals / deferred (consolidated)

- Per-observer partial-Hidden state (needs wire entity-override machinery beyond this wave).
- Active-search Perception checks as a player-triggered spot attempt.
- "Seen while hidden" / "made noise" break triggers requiring an LOS-change feed from rpg-api into the
  toolkit (no such feed exists yet).
- Help's RAW adjacency/task-validity constraints (unvalidated, same stance as Beat 1 took on
  unvalidated ability preconditions generally).
- Dodge's "if you can see the attacker" clause (pre-existing gap in the already-shipped
  `DodgingCondition` code, not new to this wave).
- Real LOS (walls, lighting) — `perception.CanSeeAt` stays the range-only stub; Hide's observer-set
  computation inherits that limitation, named not hidden.
- `rpg-api#596` (persistence one-home migration) — parked, unrelated, not touched.
- `rpg-toolkit#722`/`#723` (OA damage/correlation bugs) — already fixed and merged as `#724`; consumed
  via version bump, not re-investigated here.
- Any condition beyond Dodging/Hidden/Helped. Sneak Attack, Frightened, Poisoned, etc. are out of
  scope; this wave is scoped to the three verbs the 2026-07-01 playtest evidence named.

## Open items (tracked under the wave, not blockers)

- **Unsubscribe-on-first-use timing.** `HiddenCondition`/`HelpedCondition` both need "consume on next
  qualifying roll" semantics that existing self-terminating conditions (Dodging, Raging) don't quite
  have a precedent for (they remove on a *different* event — turn boundary or rest — not the same
  chain event that granted the bonus). The exact unsubscribe-from-within-your-own-handler mechanics
  are an implementation detail for the implementer to work out, flagged here so it isn't rediscovered
  mid-session.
- **Deterministic roller for the Hide-failure playtest branch.** Exercising both the success and
  failure path of a Stealth-vs-passive-Perception check in one MCP session may need a seeded/mock
  roller hook in the devseed harness, if one doesn't already exist for this purpose.
- **Reaction-pause interplay** — cross-referenced from Beat 1, still not this wave's concern, but
  Dodge's disadvantage-to-attacker interacts with the same `AttackChain` reactions (Shield) subscribe
  to; no known conflict, just flagged as adjacent surface.

## Related / inherited

- Builds on Beat 1 (`rpg-project#54`, `ideas/encounter/v1alpha2/take-action/design.md`) — the
  resolved-action event, correlation id, and target-kind menu plumbing are reused as-is, not
  re-derived.
- Closes: `rpg-toolkit#716`, `rpg-toolkit#699`.
- Required for sign-off, ships alongside: `rpg-api#612` (player HP seeding).
- Consumed, not re-worked: `rpg-toolkit#724` (OA weapon threading, merged 2026-07-03).
- Adjacent, explicitly not this wave: `rpg-toolkit#722`/`#723` (already resolved by #724, cross-
  referenced only), `rpg-api#596` (parked).

---

## North-Star Invariants self-check

1. **Web computes nothing.** Web renders `has_advantage`/`has_disadvantage` and status effects as
   server-sent booleans/refs; it never infers disadvantage from "an attacker is Dodging" client-side.
2. **rpg-api authors zero rules strings.** Condition display metadata is a lookup table keyed by
   `Ref`, not a conditional; advantage/disadvantage fields are copied verbatim from the toolkit result.
3. **Toolkit owns the resolution.** The Stealth check, the opposed comparison, the advantage/
   disadvantage grants, and the observer-set computation (via the toolkit-owned encounter SDK, not
   rpg-api) all live in rpg-toolkit.
4. **Protos + rpg-api are one unit.** The advantage/disadvantage + condition-metadata proto work is
   named as first-class in the issue breakdown, not folded silently into the toolkit PR description.
5-8. **Events are canonical, causation-carrying.** The new fields ride the existing correlation-id/
   timestamp spine from Beat 1 — no new correlation mechanism invented. `ConditionAppliedEvent` for
   Help correctly names its `Target` (the ally), satisfying causation even for a non-self effect.
9. **First-class resolved-action event.** Already satisfied by Beat 1's `ActionResolvedEvent`; this
   wave adds effect fidelity, not a new resolved-action shape.
10. **RPC responses stay minimal acks.** Unchanged — no new RPCs this wave.
11-12. **Capability / menu.** `targetKindForRef` already anticipates Help's `TargetKindSingleEntity`;
   this wave fulfills what the menu already promised rather than changing the menu contract.
13. **Persistence deferred.** Unchanged; `rpg-api#596` stays parked. One guard-rail honored: no new
   toolkit interface (e.g. `CombatResolver`, `CombatAbilityInput`) bakes in a per-RPC persistence
   assumption — the new `Target core.Entity` field and `checks` package are pure in-memory/event-bus
   additions, not persistence-shaped.

**Tension not fully resolved:** Invariant 3 says the toolkit owns all rules, including "what does an
entity see." Hide's observer-set computation living in the encounter SDK (still `rpg-toolkit`, but a
different layer than `rulebooks/dnd5e`) is consistent with the letter of that invariant but is a
judgment call about *which toolkit layer* — flagging for the review gate rather than presenting it as
settled, since it's the first wave to need position-aware rules input for a rulebook-layer check.
