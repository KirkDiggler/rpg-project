# Combat-State Home — ONE home for HP / conditions / charges

**Design for [KirkDiggler/rpg-api#596](https://github.com/KirkDiggler/rpg-api/issues/596) — Option A**
**Status:** Design for Kirk sign-off. Direction DECIDED (Option A); this fleshes it out. NO code yet.
**Date:** 2026-07-03
**Chapter:** Foundation for Chapter 2 (Combat Verbs) — a coherent combat-state home is a prerequisite for real multi-round combat and for Beat 2 (mechanical effects: [rpg-toolkit#716](https://github.com/KirkDiggler/rpg-toolkit/issues/716) Help/Hide, [#699](https://github.com/KirkDiggler/rpg-toolkit/issues/699) Dodge).
**Validated against:** the 13 North-Star Invariants in [`../design.md`](../design.md) (§9 below).
**Builds on:** [`../gap-defensive-rage-persistence.md`](../gap-defensive-rage-persistence.md) §2 (the A/B fork) and §7 (open risks — audited in §8). Sibling topic doc under `v1alpha2/`, mirroring `take-action/`.

---

## 1. The decision, in one paragraph

**The encounter snapshot is the single home for in-combat combatant state (HP + conditions +
resource charges). The character store is the out-of-combat sheet.** While an encounter is live,
each combatant's full serialized `character.Data` lives on the snapshot as `PlayerData.DataJSON`,
**persisted on the snapshot for the encounter's lifetime** — not flushed to the character store every
RPC. The character store is read **once at encounter setup** to seed the snapshot, and written **once
at teardown** (encounter end / rest). The toolkit's held combatants are authoritative during combat
(HP/conditions/charges mutate on the live bus, consumed by the chains); the snapshot is their durable
projection between RPCs. This collapses the HP double-home, makes players use the **same** persistence
model monsters already use, and kills ActivateFeature's bespoke char-store write.

**What Option B was, and why not (recorded honestly — not re-litigating):** B = fix the store
round-trip in place (keep flushing `DataJSON` to the char store each RPC, but make conditions +
per-turn flags survive). Smaller now, but keeps two homes for HP (the observed 7-vs-14 divergence
stays latent), keeps ActivateFeature's divergent write, and leaves the next combat-mutable thing
(Dodging, Hidden, Help, exhaustion, temp HP, concentration) to rediscover the same gap. A treats the
cause; B treats the symptom. Kirk + director aligned on A (`feedback_default_to_one_system`,
`feedback_prefer_breaking_changes`).

---

## 2. Where state lives today (grounded in the code)

Combat-mutable state is scattered across **three** homes — the defect:

| State | Today's home(s) | Authoritative today |
|-------|-----------------|---------------------|
| HP | `PlayerData.HP` (int on snapshot, `encounter/data.go:109`) **and** `character.Data.HitPoints` (char store) | conflicting — snapshot 7 vs store 14 observed |
| Resource charges (rage/ki/slots) | `character.Data.Resources` / `ClassResources`, via transient `DataJSON` | char store |
| Conditions (raging + per-turn flags) | `character.Data.Conditions`, via transient `DataJSON` | char store — but stripped each turn |

**The per-verb round-trip that splits the home** (combat-capable verbs; e.g. `take_action.go`,
`end_turn.go`):

```
load(id):
  encRepo.Get(snapshot)
  SeedActorTurn      → runs toolkit StartTurn, WRITES char store   (hydrate_players.go:175)
  attachPlayerCharacterData → char store → transient PlayerData.DataJSON   (:55)
  LoadFromData       → hydrates held *character.Character per seat (subscribes conditions once)
verb: enc.TakeActionPhased(...) etc.   ← toolkit mutates held HP/conditions/charges
persist:
  enc.ToData()       → syncCombatantsToData re-serializes held state INTO PlayerData.DataJSON
  persistPlayerCharacterData → flush DataJSON → char store, THEN `pd.DataJSON = nil`  (:314, :352)
  encRepo.Save(snapshot)   ← snapshot saved WITHOUT DataJSON (cleared)
```

Two design choices in that flow are the root:

1. **`PlayerData.DataJSON` is deliberately transient** — the "#689 Q1" decision (`hydrate_players.go:3-22`
   doc block; the clear at `:352`). The char store is treated as authoritative mid-combat; HP is
   re-derived onto `PlayerData.HP` *and* into the char blob — two writes, two homes, no single source.
   **Option A reverses this decision** — argued honestly in §4.
2. **ActivateFeature bypasses the shared cascade** (`activate_feature.go` orchestrator `:83-92` + handler
   `:139-143`): the verb self-loads from `CharDataJSON` and returns `UpdatedCharData`, which the handler
   writes straight to the char store via `CharacterRepo.Update`. A second, divergent persist path.

**Asymmetry worth naming:** monsters **already** carry `MonsterData.DataJSON` durably on the snapshot
(`data.go:166-174`). Only players are round-tripped through a second store. Option A makes players match
monsters — **one** persistence model for all combatants (`feedback_default_to_one_system`).

---

## 3. Target state under Option A

### 3.1 The load / persist contract

```
SETUP (encounter creation / first combatant load):
  char store → seed PlayerData.DataJSON on the snapshot   (once)

EVERY combat-capable RPC:
  load(id):
    encRepo.Get(snapshot)   ← DataJSON is PRESENT (durable now)
    LoadFromData            ← hydrates held combatants from the snapshot's DataJSON
  verb                      ← toolkit mutates held HP/conditions/charges
  persist:
    enc.ToData()            ← held state re-serialized back into PlayerData.DataJSON
    encRepo.Save(snapshot)  ← DataJSON PERSISTS on the snapshot (no clear, no char-store write)

TEARDOWN (encounter end / rest):
  run toolkit end-of-encounter boundary on each held combatant  (§5.1)
  ToData() → char store     (once — the only mid/post-combat char-store write)
```

**Concretely, the rpg-api edits:**
- `persistPlayerCharacterData` stops flushing to the char store and stops clearing DataJSON
  (`hydrate_players.go:314-355` — delete the flush + the `pd.DataJSON = nil` at `:352`). DataJSON now
  rides `encRepo.Save` like `MonsterData.DataJSON` already does.
- `attachPlayerCharacterData` becomes **seed-if-empty**: fetch from the char store only when the seat's
  `DataJSON` is empty (encounter entry / a newly-joined player). Otherwise the snapshot's DataJSON is
  the live home and is used as-is (`:55-81`).
- **Collapse `PlayerData.HP` to a projection of `DataJSON.HitPoints`** (or drop the standalone field) —
  the riskiest single edit (§8), and the one that ends the 7-vs-14 divergence: the combat resolver's
  `player.HP -= dmg` (`combat_phased.go:358-362`) must read/write the held character's HP, not a
  parallel snapshot int.
- `SeedActorTurn` (`hydrate_players.go:175`) writes onto the **snapshot's DataJSON**, not the char store
  — it runs the toolkit's own `StartTurn` (that stays); only the write target moves.
- **ActivateFeature joins the shared model**: land [rpg-toolkit#691](https://github.com/KirkDiggler/rpg-toolkit/issues/691)
  so it uses the #689 cascade like every other verb. Drop `ActivateFeatureOutput.UpdatedCharData` and
  the handler's `CharacterRepo.Update` (`activate_feature.go:139-143`). The applied condition rides the
  snapshot.

### 3.2 What each layer owns (unchanged boundary)

- **Toolkit:** owns the held combatants, all rules, the HP/condition/charge mutations, each condition's
  **declared persistence scope** (§5.1), and the end-of-encounter/rest semantics.
- **rpg-api:** loads the snapshot, invokes the verb by reference, persists `ToData()` verbatim. It
  **never** classifies conditions, computes HP, or decides what survives teardown — it runs a toolkit
  boundary verb and writes whatever the toolkit leaves. (Watch-item: the teardown write-back must stay
  an unconditional `ToData()` write, never an rpg-api field-picker — §5.2.)

---

## 4. Encounter lifecycle — the encounter holds the runtime, and it is already paused between RPCs

**The encounter is effectively paused between every RPC today, and Option A keeps it that way.** Each
combat verb is `load(id) → act → persist(id)`: the live `*Encounter` is *constructed* by `LoadFromData`
at the start of the RPC and *discarded* after `Save`. There is no always-live in-memory encounter, no
background tick, no in-memory authority that outlives a call. Nothing in Option A assumes one — and this
is deliberate: **"the snapshot is the home" IS the pause/resume mechanism.**

**Pause/resume = the same `LoadFromData` path, at any timescale.** "Resume 200 ms later on the next RPC"
and "resume an hour / a day / a session later" are the *identical* code path: `encRepo.Get` → attach
(seed-if-empty) → `LoadFromData`. A long pause is not a special case; it is just a load whose snapshot
happens to be old. Because the snapshot carries the full combatant state (HP/conditions/charges in
DataJSON), a resumed encounter rehydrates exactly where it stopped, with conditions re-subscribed to a
fresh bus once (the #684 single-subscribe cure). The lifecycle shape (`load → verb → persist` per RPC)
is what the current code *already* does; Option A does not change *when* state is parked, only *where*
(snapshot, not a second store).

**The `#689 Q1` reversal, named and argued honestly.** The current code deliberately clears
`PlayerData.DataJSON` after each RPC (`hydrate_players.go:352`, `pd.DataJSON = nil`; doc block `:3-22`,
"PlayerData.DataJSON is TRANSIENT"). That decision optimized for *"no duplicate character state living
on the encounter"* — keep the char store the single authoritative copy, avoid a stale duplicate on the
snapshot. **Option A reverses it:** DataJSON becomes the durable home on the snapshot; the char store
becomes the seed/teardown copy. The reversal is the whole point — but its costs are real and we take
them with eyes open (these are exactly `gap §7`'s snapshot-size/staleness risks):

- *Snapshot blob grows* by a full `character.Data` per seat for the encounter's lifetime. Bounded by
  party size, GC'd with the encounter, and monsters already pay this cost. Interacts with
  [#692](https://github.com/KirkDiggler/rpg-api/issues/692) (dirty-tracking → `ToData` can gate on
  dirty later) — an efficiency follow-up, not a blocker.
- *The char store's copy goes stale during combat* — intended (it is the out-of-combat sheet), but any
  consumer reading store HP mid-combat must be re-routed through the encounter (§7 audit).

The honest trade: the `#689 Q1` "no duplicate" optimization is precisely what created the two-home
divergence and the per-turn condition strip. **One home beats no-duplicate.** We accept a bigger, GC'd
snapshot to get a single source of truth.

---

## 5. The design edges (the actual work)

### 5.1 Condition durability is a **declared property**, not a list

**The rule:** each toolkit condition **declares its persistence scope** — `EncounterScoped` (evaporates
when combat ends) or `Durable` (carried onto the character sheet, survives *out* of the encounter). The
toolkit owns this declaration (it is condition *meaning* — a rule); **rpg-api never inspects it.** At
teardown, rpg-api runs the toolkit's combat-ended boundary and persists `ToData()` verbatim; the toolkit
has already removed the encounter-scoped conditions, so the write-back carries exactly the durable ones.
Conditions are **not** uniformly encounter-scoped — a curse picked up mid-fight is the counterexample
Kirk named — so the scope must be per-condition and toolkit-declared, never an rpg-api allow-list and
never behavior the API infers.

**How the toolkit can represent the declaration.** It already has the vocabulary —
`conditions/types.go:18-32`: `DurationRounds`, `DurationMinutes`, `DurationHours`, `DurationUntilRest`,
`DurationPermanent`. Persistence scope can be *derived* from it (rounds/minutes → encounter-scoped;
hours/until_rest/permanent → durable) **or** declared as a dedicated `PersistenceScope` field on the
condition. That is a toolkit-internal representation choice; what matters for this design is that it is a
**declaration the condition owns**, consumed by the combat-ended boundary. (Today Raging/Dodging
hand-roll their lifetime via bespoke Data structs that don't yet carry `DurationType` — so wiring the
declaration uniformly is real toolkit work, not a flag flip.)

**The combat-ended boundary is a toolkit gap to add.** Conditions self-expire on **turn** boundaries
today (Dodging removes itself at its owner's next `TurnStart`, `dodging.go:84-88`; Raging on `RestEvent`
/ no-activity, `raging.go:178-202`). But combat can end **mid-turn** (last enemy dies on your turn) — a
Dodging/Hidden condition would still be live at `ToData` time. So Option A needs an `EndCombat` /
mode→`Ended` toolkit verb that publishes a "combat ended" event, mirroring the existing `RestEvent`
(`character.go:459-467`): encounter-scoped conditions remove themselves in response; durable ones don't
subscribe and survive. Same shape `LongRest` already uses.

**Worked examples of the rule:**

| Condition | Declared scope | At combat-ended boundary | On the sheet after? |
|-----------|----------------|--------------------------|---------------------|
| **Dodging** (Dodge action) | EncounterScoped (`rounds`) | removed by boundary (also self-removes next turn) | **No** |
| **Hidden** / **Help** / **Disengage** / per-turn flags (`DidAttackThisTurn`) | EncounterScoped | removed | **No** |
| **A curse** picked up mid-encounter | Durable (`until_rest`/`permanent`) | *not* removed — didn't subscribe | **Yes — carried out of the encounter** |
| **Exhaustion**-like | Durable (`until_rest`) | survives; only `LongRest` reduces it | **Yes** |
| **Poisoned** for 10 rounds vs 1 hour | scope *follows the declared duration* | 10-round → removed; 1-hour → survives | **depends on the declaration** — API still never looks |

### 5.2 Teardown write-back set

At teardown rpg-api runs the toolkit combat-ended boundary (§5.1) + optional rest verb (§5.3), then
persists `character.ToData()` **verbatim** — it picks no fields. Because the toolkit removed the
encounter-scoped conditions and nils the combat-only fields at that boundary, what it *leaves* on
`character.Data` is exactly the durable sheet:

| Field | After teardown boundary | Why |
|-------|-------------------------|-----|
| `HitPoints` / `MaxHitPoints` | written | durable; combat damage is real (§5.4) |
| `DeathSaveState` | written (if at 0 HP) | durable consequence of HP (§5.4) |
| `Resources` / `ClassResources` / `SpellSlots` | written (reconciled by rest verb if a rest happens) | consumption is real; rest restores per toolkit rules (§5.3) |
| durable conditions (curse, exhaustion, `until_rest`/`permanent`) | written | didn't subscribe to combat-ended; survive |
| encounter-scoped conditions (Dodging, Hidden, Help, per-turn flags) | **cleared by the boundary** | self-remove on combat-ended; never reach the sheet |
| `ActionEconomy` | **nil** | "nil outside combat" (`character/data.go`); `StartTurn` re-seeds next combat |

### 5.3 Charges / resources reconciliation

Consumption during combat mutates the held character's `Resources`/`ClassResources`/`SpellSlots`, which
ride the snapshot DataJSON. At teardown, two cases — **both toolkit-owned**:

- **Encounter ends without a rest:** persist the depleted pools verbatim. The sheet shows the charges
  you actually spent — correct, and continuous into the next encounter (setup re-seeds from the depleted
  sheet).
- **Teardown coincides with a short/long rest:** rpg-api invokes the toolkit's existing `ShortRest()` /
  `LongRest()` verb (`character.go:430-498`), which restores pools by `ResetType` and publishes
  `RestEvent` (durable conditions may also react — e.g. exhaustion). Then `ToData` → store.

rpg-api never computes "how much rage is left" — it persists consumed state or invokes a toolkit rest
verb. The rest-vs-no-rest choice is a session decision surfaced as a verb call, not an rpg-api
calculation (Invariant 2). Precedent: the v1 orchestrator already calls `loadedChar.LongRest(ctx)`
(`internal/orchestrators/encounter/orchestrator.go:812,3799`).

### 5.4 Death / unconscious at teardown

HP and death-save state are **durable** — they cross the teardown boundary unchanged; the encounter does
not heal on exit.

- **PC at 0 HP (unconscious, death saves)** when combat ends: write `HitPoints=0` + `DeathSaveState`.
  The `unconscious` condition (`conditions/unconscious.go`) is a consequence of HP, not a tactical
  encounter-scoped condition, so 0 HP + death saves persist. A later heal or `LongRest` (sets
  `hitPoints=maxHitPoints`, clears death saves, `character.go:436-439`) reconciles.
- **PC dead:** write the dead state. Revival is a separate out-of-combat flow.

### 5.5 Player disconnect mid-encounter — disconnect ≠ teardown

**A disconnect does not end the encounter and does not write the sheet.** The combatant stays live on the
snapshot exactly as it was; its DataJSON persists; nothing flushes to the char store. On reconnect the
player loads the snapshot — the same rejoin path as §5.6. **The state model already covers this
completely; no new state handling is needed.** (Contrast teardown, which is the *only* thing that writes
the sheet.)

**What disconnect needs decided at the game layer — out of scope here; drawing the line only:**

| Question | Layer | Status |
|----------|-------|--------|
| If it's the disconnected player's turn, does it auto-skip / auto-pass on a timer? | turn-flow policy | **game-layer — not decided here** |
| Does someone (DM / AI) control the combatant while it's disconnected? | control policy | **game-layer — not decided here** |
| How long before a disconnect escalates to a *leave* (which WOULD teardown that seat)? | session/timeout policy | **game-layer — not decided here** |
| Do other players see a "disconnected" indicator? | presence / UI | **game-layer — not decided here** |
| Is the combatant's HP/conditions/charges safe, and is reconnect lossless? | **state model** | **covered by this design — yes** |

The line: **state persistence across a disconnect is solved by the snapshot-as-home model; turn-flow,
control, and timeout-to-leave are game-layer policy this design deliberately does not settle.**

### 5.6 Crash recovery / rejoin — the load path

Under A the snapshot is self-sufficient for in-combat state, so recovery is trivially correct:

```
load(id):
  encRepo.Get(snapshot)          ← PlayerData.DataJSON is populated (durable)
  attachPlayerCharacterData:     ← seed-if-empty: DataJSON present → use it; empty → seed from char store
  LoadFromData(snapshot)         ← rehydrates held combatants; conditions subscribe once (#684 cure)
```

The char store is touched **only** at setup (seed) and teardown (write-back), never on the hot path. A
crash between two combat RPCs loses nothing: the snapshot was saved after the last committed verb and is
the only home — **no divergence is representable**, because there is no second store to disagree with.
(Contrast today: a crash between the char-store flush and `encRepo.Save` can split HP — the 7-vs-14
class.) A rejoining player reads the snapshot; a newly-*joining* player is the one seed-from-store case.

### 5.7 Migration path — clean break, ordered

Prefer a clean breaking change over dual-write phases (`feedback_prefer_breaking_changes`). There is **no
stored-data migration**: durable persistence is deferred (Invariant 13), snapshots are short-lived /
GC'd, so there is no persisted corpus to convert — only the code contract flips.

1. **Toolkit first (unblocks the API break):**
   - Land [#691](https://github.com/KirkDiggler/rpg-toolkit/issues/691) — fold ActivateFeature onto the
     held combatant (corrects its "no behavior change" premise — rage now persists).
   - Add the **combat-ended boundary** verb/event + the **declared persistence scope** on conditions (§5.1).
2. **rpg-api (the contract flip — one coherent change):**
   - Flip `DataJSON` transient → durable (drop the flush + `pd.DataJSON = nil`).
   - `attachPlayerCharacterData` → seed-if-empty; `SeedActorTurn` writes the snapshot, not the store.
   - Collapse `PlayerData.HP` → projection of `DataJSON.HitPoints` (or drop). **§8 risk.**
   - Drop ActivateFeature's `UpdatedCharData` plumbing + the handler `CharacterRepo.Update`.
   - Add the **teardown write-back** (verb / mode-hook) — see the gap below.
3. **Re-verify** via the defensive-rage integration test (`integration_barbarian_rage_test.go`) + the
   MCP playtest sign-off bar.

**Teardown has no home yet — a real gap.** The v2 `EncounterService` exposes CreateEncounter,
GetEncounter, StreamEncounter, MoveEntity, Interact, TakeAction, EndTurn, SubmitCheck, SetReactionReady,
ActivateFeature — and **no** EndEncounter / LeaveEncounter (v1alpha1 had `LeaveEncounter`, since
archived). Option A's write-back needs a home that doesn't exist: a new `EndEncounter` verb, a
`mode → Ended` hook, or a "player leaves" path. **The one genuinely new surface Option A requires — see
§9.** Interim: write back on the `mode → FREE_ROAM/Ended` transition; in the current dev flow (short
single-room combats) a missing teardown just means the sheet lags until the next explicit rest —
acceptable for the playtest, not for production.

### 5.8 Beat 2 fit — Dodging / Hidden / Help live encounter-scoped

- **Dodging** ([#699](https://github.com/KirkDiggler/rpg-toolkit/issues/699)): already self-removes at
  the owner's next `TurnStart`. Under A it lives on the snapshot DataJSON, **survives the per-RPC
  round-trip** (durable now), and self-expires via the turn machinery. **Under today's
  flush-through-store it would be stripped every turn — the exact defensive-rage bug.** Beat 2
  mechanically depends on the round-trip being fixed; A is the clean substrate.
- **Hidden** ([#716](https://github.com/KirkDiggler/rpg-toolkit/issues/716)): inherently encounter-scoped
  (hidden until you attack/move/are seen). A condition on the snapshot, expired by combat events; never
  on the sheet.
- **Help** (#716): grants advantage to an ally's next attack — encounter-scoped, keyed to a target +
  expiry. On the snapshot; removed when consumed or at a turn boundary; never on the sheet.

All three declare `EncounterScoped` (§5.1): they never survive the combat-ended boundary; rpg-api never
sees or classifies them. Tripling the condition count is *safe* precisely because the snapshot is their
home and the toolkit owns their lifetime.

---

## 6. Toolkit vs API — the boundary holds

rpg-api's role **shrinks** under A: it deletes a persist path (ActivateFeature's write), deletes a
per-RPC store flush, and stops maintaining a parallel HP int. It gains one thing: a teardown verb that
invokes a toolkit boundary and writes `ToData()` once. Every rule decision — a condition's persistence
scope, what a rest restores, whether HP is halved — stays in the toolkit. If any implementation has
rpg-api inspecting `Data.Conditions` for meaning, or computing an HP delta, that is a lane violation —
reject it (gap doc §4 lane guard).

---

## 7. Honest costs / risks — and the gap-doc §7 audit

Every open risk from `gap-defensive-rage-persistence.md §7` is resolved-by-design or carried forward
explicitly:

| gap §7 risk | Disposition here |
|-------------|------------------|
| **HP double-home collapse is the riskiest edit** | **Carried forward as the top risk (§3.1, §8).** Mitigation designed: keep `PlayerData.HP` as a *read projection* off the held character (cheap readers keep working) but make the held character the only writer; every `PlayerData.HP` site (`combat_phased.go:358-362`, `npc.go:908-921,1025`, `encounter.go:268,307`) moves to the projection in one migration step (`feedback_write_plan_for_restructures`). |
| **Snapshot size / staleness** (persisting full DataJSON reintroduces "char state on the encounter", #689 Q1) | **Resolved as an accepted, argued cost (§4).** Bounded by party size, GC'd, monsters already pay it; staleness of the store copy is intended (out-of-combat sheet). Efficiency gated by #692 dirty-tracking. |
| **Option A vs B fork** | **Resolved** — A decided (Kirk + director). |
| **`Multiplier` on additive components** (`sum(components.amount)==amount` invariant + proto doc) | **Carried forward as out-of-scope-here.** This belongs to the rage-*resistance-breakdown* work (gap §3.1/§4), not the state-home. Named so it is not dropped: whoever ships the resistance `DamageComponent.Multiplier` must update the `events.proto` sum-invariant comment. Not gated by this design. |

New risks this design introduces:
- **Stale out-of-combat sheet during combat** (§4): a deliberate consequence — any existing consumer
  that reads char-store HP mid-combat must be audited and routed through the encounter.
- **Teardown surface is net-new** (§5.7): small but real, and the piece most likely to be skipped,
  leaving the sheet permanently stale.

---

## 8. North-Star Invariant validation (all 13)

| # | Invariant | Verdict |
|---|-----------|---------|
| 1 | Web sends Refs, renders server data | **No change.** State-home is server-internal; the web never saw the char store. |
| 2 | rpg-api orchestrates by key, no rules conditionals | **Improved.** A deletes the ActivateFeature bespoke write and the ActionEconomy authoring. **Tension to guard:** teardown write-back must be an unconditional `ToData()` after a toolkit boundary — never an rpg-api field-picker (§5.1, §5.2, §6). |
| 3 | Toolkit owns all rules | **Strengthened.** Condition *persistence scope* + rest reconciliation move fully into toolkit declarations/verbs; the classification rule *is* Invariant 3. |
| 4 | protos + rpg-api one contract unit | **No new proto** for the state-home; the teardown verb (§5.7) is a first-class proto add when it lands. |
| 5 | Events stamped with game-event time | **No change.** HP/condition change events still flow from the toolkit (timestamp/correlation owned by the TakeAction wave). |
| 6 | rpg-api projects for audience, not rendering | **No change.** |
| 7 | Toolkit field with no proto counterpart = proto gap | **No change** (no event narrowing here). |
| 8 | Events carry causation | **No change.** |
| 9 | First-class resolved-action event | **No change.** |
| 10 | RPC responses are minimal acks | **Improved.** `ActivateFeatureOutput.UpdatedCharData` (a state payload on the ack) is **deleted** — state flows via the snapshot. |
| 11 | Capability is server-sent data | **No change** — economy now rides the snapshot DataJSON, still toolkit-computed. |
| 12 | Illegal actions pre-empted in the menu | **No change.** |
| 13 | Durable persistence deferred, contract forward-compatible | **Consistent, with a named nuance.** A does **not** add a new durable store (event log/replay); it changes *what the already-persisted snapshot carries* (DataJSON durable vs transient). No migration corpus. Snapshot-blob growth (§4, §7) is the price; forward-compatible with #692. |

**No invariant is violated.** Live tensions: (a) keep teardown write-back field-pick-free (Invariant 2 —
designed against); (b) the Invariant 13 nuance that the snapshot now carries more (named, bounded, GC'd).

---

## 9. Open questions that genuinely need Kirk

1. **Teardown surface (§5.7).** Option A needs a write-back home that doesn't exist:
   (a) new `EndEncounter`/`LeaveEncounter` verb, (b) hook the `mode → FREE_ROAM/Ended` transition, or
   (c) tie write-back to an explicit rest verb? **Recommendation:** hook the mode transition now
   (cheapest, the natural combat→free-roam boundary); add `LeaveEncounter` when multiplayer-leave lands.
2. **`PlayerData.HP`: projection vs delete (§3.1, §7).** Keep as a read-only projection off DataJSON
   (cheaper reads, one writer) or delete and always read HP from the held character? **Recommendation:**
   projection during migration to de-risk the cutover; consider deleting once all readers go through the
   encounter.
3. **Persistence scope: derive from `DurationType` or a dedicated field (§5.1).** Confirm the toolkit
   should grow the combat-ended boundary (mirror `RestEvent`) and declare condition scope. **My read:**
   yes to the boundary; representation (derive vs dedicated field) is a toolkit-internal call.
4. **Disconnect-to-leave timeout (§5.5).** The state model is settled; the *policy* (how long a
   disconnect stays "paused" before it escalates to a leave + teardown) is game-layer and unowned —
   flagging that it needs a decision-maker, not that this design should make it.

---

## 10. Issue breakdown (under the #596 umbrella / Chapter-1 honesty)

| # | Repo | Title |
|---|------|-------|
| 1 | rpg-toolkit | Fold ActivateFeature onto the held combatant — **is [#691](https://github.com/KirkDiggler/rpg-toolkit/issues/691)**; drop `UpdatedCharData` self-load |
| 2 | rpg-toolkit | Declare per-condition **persistence scope** + add a combat-ended boundary event (mirror `RestEvent`) so encounter-scoped conditions self-remove at end-of-combat |
| 3 | rpg-api | Make the encounter snapshot the single home: DataJSON durable, seed-if-empty, drop per-RPC store flush + clear |
| 4 | rpg-api | Collapse `PlayerData.HP` → projection of `DataJSON.HitPoints` (end the 7-vs-14 divergence) |
| 5 | rpg-api | Add teardown write-back (verb / mode-hook): run toolkit boundary + single `ToData()` → char store |
| 6 | rpg-api | Drop ActivateFeature's `CharacterRepo.Update(UpdatedCharData)` once #691 lands |
| 7 | rpg-api | Integration: assert the defensive-rage path survives across the per-RPC round-trip under the new home |

Items 3+4+6 are one logical unit (the contract flip) and can ship as one PR; 1+2 are the toolkit
prerequisites; 5 is the net-new surface (gated on §9.1); 7 is the re-verification gate. Interacts with —
does not fold in — [#692](https://github.com/KirkDiggler/rpg-api/issues/692) (IsDirty). The
`DamageComponent.Multiplier` proto/invariant item (gap §7) stays with the resistance-breakdown work,
**not** this umbrella (§7).
