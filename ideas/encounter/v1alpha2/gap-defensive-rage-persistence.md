# Gap: Defensive Rage Resistance Never Fires (combat-mutable state has no home)

**Status:** Design proposal — awaiting director judgment + Kirk sign-off. NO code yet.
**Date:** 2026-05-31
**Wave:** Architecture Honesty (Wave 0), umbrella rpg-api#574
**Author:** rpg-api team-member (cross-repo design hat for this gap)
**Build under test:** rpg-api `main`@`eaf4046`, rpg-toolkit `encounter/v0.17.0` + `rulebooks/dnd5e v0.59.0`

---

## 1. Root cause recap (verified by live instrumentation)

A raging Barbarian (`char-bob`) takes a goblin scimitar hit resolved inside the EndTurn
NPC-dispatch loop and takes **full** damage — no rage halving, no `raging` component on the
breakdown. Instrumented diagnostic (reverted) at two vantage points proved:

- **V1 (defensive load, after `attachPlayerCharacterData`):** `char-bob` character-store
  `Data.Conditions` is **empty** (`total_conditions=0`). Nothing for the #689 cascade to
  hydrate/subscribe on `e.bus`.
- **V2 (post-`enc.EndTurn` TurnEnd boundary, pre-`enc.NPCAct`):** held combatant for
  `char-bob` re-serialized via `enc.ToData()` carries **no** raging blob.

So raging is **absent at the very first observation point** — the proximate cause is
**(A) the persistence round-trip**, not turn-end timing in isolation. But tracing the full
chain shows the deeper mechanism is an **architectural one**, and `endRage` timing is the
amplifier:

### The exact link-by-link trace (where the condition is lost)

1. **Toolkit emit is correct.** `Encounter.ActivateFeature`
   (`encounter/activate_feature.go:100-146`) loads the char onto `e.bus`, runs
   `char.ActivateAbility` → `Rage.Activate` (`rulebooks/dnd5e/features/rage.go`) publishes a
   `ConditionAppliedEvent{Condition: ragingCondition}`. The character's own
   `onConditionApplied` (`rulebooks/dnd5e/character/character.go:1060-1077`) runs
   `condition.Apply(ctx, bus)` **and** appends to `c.conditions`. `char.ToData()`
   (`character.go:994-1005`) iterates `c.conditions` and serializes each via `ToJSON()`. So
   `ActivateFeatureOutput.UpdatedCharData` **does** contain the raging blob. ✓
2. **rpg-api persist of activation is correct.** The ActivateFeature handler
   (`internal/handlers/dnd5e/v2/encounter/activate_feature.go:141-150`) unmarshals
   `UpdatedCharData` and writes it back to the character store via `CharacterRepo.Update`.
   The integration test `TestIntegration_ActivateRage_PersistsRagingCondition` asserts this
   and passes — **after ActivateFeature the store has raging.** ✓
3. **The next combat verb removes it.** On the *following* `EndTurn`, the load
   (`WithCharacterData=true`) hydrates raging from the store onto the held combatant
   (`internal/orchestrators/encounter/v2/load.go:88-92` → cascade), then `enc.EndTurn` fires
   `char-bob`'s TurnEnd boundary. `RagingCondition.onTurnEnd`
   (`rulebooks/dnd5e/conditions/raging.go:179-202`): **if `!DidAttackThisTurn &&
   !WasHitThisTurn` → `endRage("no_combat_activity")`** → publishes `ConditionRemovedEvent`
   → character's `onConditionRemoved` strips raging from `c.conditions`.
4. **rpg-api persists the now-conditionless state.** `persistWithCharacterData` →
   `enc.ToData()` → `syncCombatantsToData` re-serializes the held combatant (no raging) →
   `persistPlayerCharacterData` writes a **conditionless** `Data` back to the store
   (`hydrate_players.go:97-138`). The store loses raging — permanently, every turn Bob
   doesn't act. ✗

### Why the per-turn flags don't save it

`DidAttackThisTurn`/`WasHitThisTurn` **are** in `RagingData` JSON (`raging.go:29-30`,
`was_hit_this_turn`/`did_attack_this_turn`) so they *do* round-trip. The break is **semantic,
not serialization**: on a turn where Bob ends his turn without having attacked and without
having been hit *yet this turn*, `onTurnEnd` legitimately reads both flags false and ends
rage. In a single in-process flow (the integration test) Bob attacks (`DidAttackThisTurn=true`)
before EndTurn, so rage survives — which is exactly why the test is green while production
fails. The live encounter reached round 3 with Bob having ended prior turns; rage was stripped
on the first such EndTurn and the conditionless state was written back to the store.

> **Honest nuance for the director:** the toolkit's `onTurnEnd` rule (end rage if no combat
> activity) is *5e-correct in spirit* (rage ends if you didn't attack or take damage). The bug
> is not that rule firing — it's that **(a)** the rule sees a fresh-loaded condition whose
> per-turn flags are reset by the *order* of operations across the RPC boundary, and **(b)**
> we are round-tripping live combat condition state through the authoritative character store
> at all. Both point at the same fix: combat-mutable state needs one coherent home and the
> turn-end evaluation must run against state that survives the load.

### Lane check on the root cause

Nothing in the drop chain is rpg-api computing a rule. rpg-api faithfully loads, invokes the
verb, and persists whatever `ToData` produces. The fix must keep it that way.

---

## 2. Where combat-mutable state should live (the deciding decision)

Today, combat-mutable state is **scattered across three homes**, which is the real defect:

| State | Today's home | Authoritative? |
|-------|-------------|----------------|
| HP | encounter snapshot (`PlayerData.HP`) **and** char store (`Data.HitPoints`) | conflicting — snapshot 7, store 14 observed |
| Resource charges (rage uses) | char store (`Data.Resources`) | char store |
| Conditions (raging + per-turn flags) | char store (`Data.Conditions`), transiently | char store — but stripped each turn |

Apply **"is this actually different?"** — HP, conditions, and resource charges are **all the
same kind of thing**: per-encounter mutable combatant state that the toolkit mutates on the
live bus and that must survive `ToData`/`LoadFromData` across each RPC. They are *not* three
different problems. They should have **ONE home**.

### Recommendation: the encounter snapshot is the single home for in-encounter combatant state; the character store is the out-of-combat sheet

Concretely: while an encounter is live, the combatant's full serialized `character.Data`
(HP + conditions + resource charges) lives in the **encounter snapshot** as the transient
`PlayerData.DataJSON` — and that DataJSON is **persisted on the snapshot for the encounter's
lifetime**, not flushed-and-cleared to the character store every RPC. The character store
holds the durable sheet (identity, class, level, equipment, long-rest resource maxima) and is
written back **only at encounter teardown / long rest**, not mid-combat.

This is the model the Hydration Contract already implies: `Encounter.LoadFromData` returns a
**live, bus-subscribed** combatant; its state belongs to the encounter for the encounter's
life. The current design *deliberately clears* `PlayerData.DataJSON` after each RPC
(`hydrate_players.go:130-135`, "transient #689 Q1") and treats the char store as authoritative
mid-combat — that is the design choice that splits the home and creates the round-trip gap.

**Why this is the unifying model (not a rage special-case):**
- It collapses the HP double-home: the snapshot's `PlayerData.HP` becomes a *projection* of
  `DataJSON.HitPoints` (or we drop the separate field; see risks). One source of truth.
- Per-turn condition flags (`DidAttackThisTurn`, sneak-attack `UsedThisTurn`, etc.) survive
  because the whole live combatant survives on the snapshot — no flush to a store that doesn't
  model them.
- `endRage("no_combat_activity")` then evaluates against state that actually persisted across
  the turn, so it only fires when 5e says it should.
- ActivateFeature stops needing its own separate store write (`UpdatedCharData` →
  `CharacterRepo.Update`): the activation result rides the snapshot like everything else.

**Trade-off / the alternative (keep char store authoritative mid-combat):** the alternative is
to *fix the round-trip in place* — keep flushing `DataJSON` to the char store each RPC, but
ensure conditions + per-turn flags survive. This is less code now but **keeps two homes for
HP** (the observed 7-vs-14 divergence stays latent), keeps ActivateFeature's bespoke write
path, and leaves the next combat-mutable thing (exhaustion, concentration, temp HP) to
rediscover the same gap. It treats the symptom. The recommended model treats the cause and is
the one consistent with the Hydration Contract's "live entity owned by the encounter."

> **This is the one architecture-deciding fork — flagging for the director.** Both options
> close the bug. Option A (snapshot is the home — recommended) is a larger, cleaner change that
> unifies HP/conditions/charges and removes ActivateFeature's special persist. Option B (fix
> the store round-trip in place) is smaller but preserves the split-home smell. My read leans
> hard to A on `feedback_default_to_one_system` + `feedback_prefer_breaking_changes`, but A
> touches the persistence contract for every verb, so it is Kirk's call.

---

## 3. Toolkit changes

The toolkit owns the rules and the wire-breakdown shape. Three pieces:

### 3.1 `core.DamageComponent` gains a multiplier (so resistance is representable + testable)

`encounter/core/damage.go` `DamageComponent` is **additive-only** (Source/Amount/DamageType/
IsCritical). Rage halving is **multiplicative** and currently invisible on the wire — the
breakdown can show `[scimitar:1, dex:2]` but cannot show "raging ×0.5 resistance". Add a field
so the resolver can emit a resistance line:

```
type DamageComponent struct {
    Source     string  `json:"source"`
    Amount     int     `json:"amount"`       // signed final contribution after multiplier
    Multiplier float64 `json:"multiplier,omitempty"` // 0.5 for resistance, 2.0 vuln; 0/omitted = additive
    DamageType string  `json:"damage_type,omitempty"`
    IsCritical bool    `json:"is_critical,omitempty"`
}
```

The dnd5e combat resolver (`rulebooks/dnd5e`) populates a `raging` component carrying the
multiplier when `RagingCondition.onDamageChain` (`conditions/raging.go:246+`) applies
resistance on the defender side. **This is the piece that lets the integration test assert the
halving against the real component instead of the skip-hatch comparison.** Exact shape (line
item with `Multiplier:0.5` vs. a separate `ResistanceComponent`) is a toolkit-side detail;
recommend the single-field approach above for "one representation."

### 3.2 Combatant condition state must survive the chosen persistence model

- **If Option A (snapshot is the home):** `Encounter.ToData`/`LoadFromData` already carry
  `PlayerData.DataJSON` (which includes `Data.Conditions` via `character.ToData`). The toolkit
  change is minimal — the cascade already serializes conditions correctly (verified
  `syncCombatantsToData` → `character.ToData` → condition `ToJSON`). The toolkit is **already
  correct** for A; the work is rpg-api ceasing to clear/flush DataJSON. Confirm no toolkit
  change beyond §3.1 + §3.3.
- **If Option B (store round-trip in place):** also no new toolkit serialization work —
  conditions already round-trip. The fix is entirely the rpg-api flush ordering (§4).

### 3.3 Fold ActivateFeature onto the held combatant — **this IS rpg-toolkit#691**

#691 ("fold ActivateFeature onto the held combatant") is the **direct enabler** of the clean
fix: once ActivateFeature uses the held, cascade-hydrated combatant instead of self-loading
from `CharDataJSON` (with its `defer Cleanup`), its applied condition lands on the *same* held
entity the encounter persists — no separate `UpdatedCharData` store write, no second hydration.
Under Option A this is **required** (the activation result must ride the snapshot). Under
Option B it is strongly recommended (removes the divergent write path). **This gap does not
supersede #691 — it promotes #691 from "low priority, no behavior change" to "the mechanism
that fixes defensive rage," and corrects its premise: there IS a behavior change (rage now
persists correctly).**

### Toolkit turn-end semantics — NO change needed

`onTurnEnd` ending rage on no-combat-activity is correct 5e. Once state persists properly, it
fires only when it should. Do **not** special-case it.

---

## 4. rpg-api changes

rpg-api stays a thin shuttle. Per the chosen option:

**Option A (recommended):**
- Stop clearing `PlayerData.DataJSON` after each RPC (`hydrate_players.go`
  `persistPlayerCharacterData` currently nils it at line 135). Persist DataJSON **on the
  snapshot** for the encounter lifetime.
- `attachPlayerCharacterData` seeds DataJSON from the char store **only on first load** (when
  the snapshot has none) — i.e. encounter entry. Subsequent loads read DataJSON from the
  snapshot (the live home).
- Collapse `PlayerData.HP` to a projection of DataJSON (or stop writing it independently) to
  end the 7-vs-14 divergence.
- Drop ActivateFeature's bespoke `CharacterRepo.Update(UpdatedCharData)` once #691 lands; the
  condition rides the snapshot.
- Char store write-back happens at **encounter teardown / long-rest**, not per combat RPC.

**Option B (alternative):**
- Keep the flush, but ensure the ActivateFeature-applied condition and the per-turn flags are
  not lost: route ActivateFeature through the held combatant (#691) so its persist and the
  combat verbs' persist use one entity + one write path.

**Lane guard (applies to both):** rpg-api must **never** halve damage, read the `Multiplier`
to compute anything, or branch on `raging`. It forwards `DamageComponent` (incl. the new
`Multiplier`) straight to proto in the translator and persists opaque DataJSON. The halving
lives entirely in `RagingCondition.onDamageChain`. **Flag:** if any proposed implementation
has rpg-api inspecting `Multiplier`, reading `Data.Conditions` for meaning, or computing HP
deltas from components — that is a lane violation; reject it. The translator copying the field
proto↔toolkit is the only rpg-api touch.

---

## 5. Issue breakdown (under umbrella rpg-api#574)

| # | Repo | Title | Relationship to #691/#692 |
|---|------|-------|---------------------------|
| 1 | rpg-toolkit | `encounter/core: add Multiplier to DamageComponent so resistance/vulnerability is representable on the breakdown` | new |
| 2 | rpg-toolkit | `rulebooks/dnd5e: emit a raging resistance DamageComponent (Multiplier 0.5) from RagingCondition.onDamageChain defender path` | new |
| 3 | rpg-toolkit | `encounter: fold ActivateFeature onto the held combatant — fixes defensive-rage persistence` | **IS #691** (repurpose/expand #691; correct its "no behavior change" premise) |
| 4 | rpg-api | `encounter/v2: make the encounter snapshot the single home for in-encounter combatant state (HP+conditions+charges); stop mid-combat char-store flush` | new (Option A) — **OR** `encounter/v2: route ActivateFeature + combat verbs through one held-combatant persist path` (Option B) |
| 5 | rpg-api-protos | `v1alpha2 encounter: add multiplier to DamageComponent (mirror toolkit)` | new |
| 6 | rpg-dnd5e-web | `render resistance component (×0.5) in the damage breakdown` | new (UI; depends on #5) |
| 7 | rpg-api | `integration: un-skip + assert real raging resistance halving via the bus-resolved breakdown (no Skip hatch)` | extends the existing `integration_barbarian_rage_test.go` (test #3) |

- **Does this FIX #691?** It *implements* #691 and makes it load-bearing. #691 stays the home
  for that work; this gap reframes it.
- **Does this supersede/extend #692?** **Extends** #692. #692 (IsDirty beyond HP) is the
  efficiency-gating follow-up: once conditions/charges are dirty-tracked, the `ToData` cascade
  can gate on dirty. Not required to fix the bug (the cascade is correctly unconditional today)
  but the chosen persistence home interacts with it — note the dependency, don't fold it in.

---

## 6. Re-verification plan

### Integration test (the un-skip — issue #7)
`TestIntegration_RageResistance_HalvesGoblinDamage` already exists with the right
scenario-comparison strategy (`integration_barbarian_rage_test.go:252`), asserting
`expectedRagingDelta=4` vs `expectedBaselineDelta=8`. It currently passes **because the flow
attacks before EndTurn** (rage survives). Strengthen it to prove the *defensive* path that
production hits:
- Add a variant where Bob **ends his turn without attacking** across a real per-RPC persist
  cycle (Get→Save→Get against the repo, mirroring `goblinAttackBobHP` but with the chosen
  persistence model), then the goblin lands — assert raging is **still present** at the
  defensive load and the delta is halved.
- Assert the `DamageDealtEvent.Components` (now with `Multiplier:0.5`, Source
  `dnd5e:conditions:raging`) is present — the real bus-resolved breakdown, no Skip hatch.

### Harness-UI playtest (the sign-off bar)
Against the live devseed encounter (`dev-encounter`: Bob barbarian + goblin):
1. Bob activates Rage → raging indicator + rage charges decrement (already works).
2. Bob ends his turn **without attacking** (the defensive scenario).
3. Goblin lands a scimitar hit on screen → **Bob's HP drops by the HALVED amount** (e.g. 4
   not 8), and the on-screen / decoded breakdown carries the `raging` resistance component
   (`×0.5`).
4. Decode `EntityDamaged` via the Connect logging interceptor → `damage_breakdown` contains the
   raging multiplier line. Restart dev servers first (`feedback_playtest_restart_dev_servers`)
   to rule out a stale bundle.

Done only when the halved HP drop is observable end-to-end via MCP playtest
(`feedback_playtest_is_the_signoff_bar`).

---

## 7. Open risks

- **HP double-home collapse (Option A) is the riskiest edit.** Every verb that reads/writes
  `PlayerData.HP` must move to the DataJSON projection consistently, or the divergence flips
  sign. Needs a written migration note before execution (`feedback_write_plan_for_restructures`).
- **Snapshot size / staleness (Option A):** persisting full DataJSON on the snapshot for the
  encounter lifetime grows the blob and reintroduces a "char state on the encounter" shape the
  current design explicitly avoided (#689 Q1). Mitigated by writing the store at teardown, but
  the director should weigh this against the split-home cost.
- **The one fork that could change the whole shape:** Option A vs B (§2). If Kirk wants the
  minimal in-place fix, the issue breakdown collapses to #1, #2, #3(=#691), a smaller #4
  (Option B), #5, #6, #7 — and the HP-home unification is deferred to its own issue. **Recommend
  A; defer to Kirk.**
- **`Multiplier` on additive components:** ensure `sum(components.amount) == amount` invariant
  in the proto doc still holds when a resistance line is present — define whether the resistance
  line's `Amount` is the negative delta or the post-multiplier total. Toolkit-side detail, but
  the proto doc comment (`events.proto:136-138`) must be updated to match.
