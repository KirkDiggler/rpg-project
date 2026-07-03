# Combat-State Home — ONE home for HP / conditions / charges

**Design for [KirkDiggler/rpg-api#596](https://github.com/KirkDiggler/rpg-api/issues/596) — Option A**
**Status:** Design for Kirk sign-off. Direction DECIDED (Option A); this fleshes it out. NO code yet.
**Date:** 2026-07-03
**Chapter:** Foundation for Chapter 2 (Combat Verbs) — a coherent combat-state home is a prerequisite for real multi-round combat and for Beat 2 (mechanical effects: [rpg-toolkit#716](https://github.com/KirkDiggler/rpg-toolkit/issues/716) Help/Hide, [#699](https://github.com/KirkDiggler/rpg-toolkit/issues/699) Dodge).
**Validated against:** the 13 North-Star Invariants in [`../design.md`](../design.md) (§7 below).
**Builds on:** [`../gap-defensive-rage-persistence.md`](../gap-defensive-rage-persistence.md) §2 (the original A/B fork). This is a sibling topic doc under `v1alpha2/`, mirroring `take-action/` — the gap doc diagnosed the bug; this doc is the home design.

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

**What Option B was, and why not (not re-litigating — recorded honestly):** B = fix the store
round-trip in place (keep flushing `DataJSON` to the char store each RPC, but ensure conditions +
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
| Resource charges (rage/ki/slots) | `character.Data.Resources` / `ClassResources` in the char store, via transient `DataJSON` | char store |
| Conditions (raging + per-turn flags) | `character.Data.Conditions` in the char store, via transient `DataJSON` | char store — but stripped each turn |

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

1. **`PlayerData.DataJSON` is deliberately transient** (`hydrate_players.go:3-22`, "#689 Q1"): attached
   before `LoadFromData`, cleared after the flush (`:352`). The char store is treated as authoritative
   mid-combat. HP is re-derived onto `PlayerData.HP` and *also* into the char blob — two writes, two
   homes, no single source.
2. **ActivateFeature bypasses the shared cascade** (`activate_feature.go` orchestrator `:83-92` + handler
   `:139-143`): the verb self-loads from `CharDataJSON` and returns `UpdatedCharData`, which the handler
   writes straight to the char store via `CharacterRepo.Update`. A second, divergent persist path.

**Asymmetry worth naming:** monsters **already** carry `MonsterData.DataJSON` durably on the snapshot
(`data.go:166-174`, `LoadFromData` rehydrates per call). Only players are round-tripped through a
second store. Option A makes players match monsters — **one** persistence model for all combatants
(`feedback_default_to_one_system`).

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
  run toolkit end-of-encounter boundary on each held combatant  (§4.1)
  ToData() → char store     (once — the only mid/post-combat char-store write)
```

**Concretely, the rpg-api edits:**
- `persistPlayerCharacterData` stops flushing to the char store and stops clearing DataJSON
  (`hydrate_players.go:314-355` — delete the flush + the `pd.DataJSON = nil` at `:352`). DataJSON now
  rides `encRepo.Save` like `MonsterData.DataJSON` already does.
- `attachPlayerCharacterData` becomes **seed-if-empty**: fetch from the char store only when the seat's
  `DataJSON` is empty (encounter entry / a newly-joined player). Otherwise the snapshot's DataJSON is
  the live home and is used as-is (`:55-81`).
- **Collapse `PlayerData.HP` to a projection of `DataJSON.HitPoints`** (or drop the standalone field).
  This is the riskiest single edit (§6) and the one that ends the 7-vs-14 divergence: the combat
  resolver's `player.HP -= dmg` (`combat_phased.go:358-362`) must read/write the held character's HP,
  not a parallel snapshot int. One source, projected for anyone who needs a cheap read.
- `SeedActorTurn` (`hydrate_players.go:175`) writes onto the **snapshot's DataJSON**, not the char
  store — it's a first-actor economy heal, and economy is combat-only state that belongs on the
  snapshot now. (It runs the toolkit's own `StartTurn`; that stays — only the write target moves.)
- **ActivateFeature joins the shared model**: land [rpg-toolkit#691](https://github.com/KirkDiggler/rpg-toolkit/issues/691)
  (fold ActivateFeature onto the held combatant) so it uses the #689 cascade like every other verb.
  Drop `ActivateFeatureOutput.UpdatedCharData` and the handler's `CharacterRepo.Update`
  (`activate_feature.go:139-143`). The applied condition rides the snapshot.

### 3.2 What each layer owns (unchanged boundary)

- **Toolkit:** owns the held combatants, all rules, the HP/condition/charge mutations, condition
  lifetime (which conditions self-remove at which boundary), and the end-of-encounter/rest semantics.
- **rpg-api:** loads the snapshot, invokes the verb by reference, persists `ToData()` verbatim. It
  **never** classifies conditions, never computes HP, never decides what survives teardown — it runs a
  toolkit boundary verb and writes whatever the toolkit leaves. (Watch-item: the teardown write-back
  must stay an unconditional `ToData()` write, never an rpg-api field-picker — §4.2.)

---

## 4. The seven edges (the actual work)

### 4.1 Teardown write-back set + the condition-classification rule

**The classification rule — one line:** *A condition is encounter-scoped iff it removes itself at a
turn/combat boundary; that decision lives in the condition's own `Apply`/subscribe logic in the
toolkit, never in an rpg-api allow-list.* rpg-api does **not** classify. The toolkit already works this
way:

- `DodgingCondition` subscribes to `TurnStart` and **removes itself at the start of its owner's next
  turn** (`conditions/dodging.go:84-88`, `onTurnStart` → `ConditionRemovedEvent`).
- `RagingCondition` removes itself on a `RestEvent` and on `onTurnEnd` (no-activity / duration expiry)
  (`conditions/raging.go:178-202`; `character.go:459-467` publishes `RestEvent` on `LongRest`/`ShortRest`,
  "conditions react, e.g. RagingCondition removes itself").
- The toolkit already has a **duration vocabulary** — `conditions/types.go:18-32`: `DurationRounds`,
  `DurationMinutes`, `DurationHours`, `DurationUntilRest`, `DurationPermanent`. This is the generic hook
  the toolkit *can* use to implement the boundary reaction uniformly (today raging/dodging hand-roll it
  via bespoke Data structs — `RagingData`, `DodgingData` — which don't yet carry `DurationType`).

**The one toolkit gap this exposes:** conditions self-expire on **turn** boundaries, but combat can end
**mid-turn** (last enemy dies on your turn) — a Dodging/Hidden condition would still be live on the held
character at `ToData` time. So Option A needs a **combat-ended boundary**, mirroring the existing
`RestEvent`: an `EndCombat` / mode→`Ended` toolkit verb that publishes a "combat ended" event;
encounter-scoped conditions unsubscribe/remove themselves in response, durable ones don't subscribe and
survive. This is the toolkit-owned classification mechanism — the same shape as `LongRest` already uses.

**The write-back SET, stated as a rule (not a case list):** at teardown rpg-api runs the toolkit
boundary verb, then persists `character.ToData()` **verbatim**. Because the toolkit removed the
encounter-scoped conditions and nils the combat-only fields at that boundary, the write-back is
unconditional — rpg-api picks no fields. What the toolkit *leaves* on `character.Data` after the
boundary is exactly the durable sheet:

| Field | After teardown boundary | Why |
|-------|-------------------------|-----|
| `HitPoints` / `MaxHitPoints` | written | durable sheet state; combat damage is real (§4.3) |
| `DeathSaveState` | written (if at 0 HP) | durable consequence of HP, not a tactical condition (§4.3) |
| `Resources` / `ClassResources` / `SpellSlots` | written (reconciled by rest verb if a rest happens) | consumption is real; rest restores per toolkit rules (§4.2) |
| durable conditions (exhaustion, curses, diseases, `until_rest`/`permanent`) | written | didn't subscribe to combat-ended; survive |
| encounter-scoped conditions (Dodging, Hidden, Help, Disengage, per-turn flags) | **cleared by the boundary** | self-remove on combat-ended; never reach the sheet |
| `ActionEconomy` | **nil** | "nil outside combat" (`character/data.go`); `StartTurn` re-seeds next combat |

### 4.2 Charges / resources reconciliation

Consumption during combat mutates the held character's `Resources`/`ClassResources`/`SpellSlots`, which
ride the snapshot DataJSON. At teardown two cases, **both toolkit-owned**:

- **Encounter ends without a rest:** persist the depleted pools verbatim. The sheet now shows the
  charges you actually spent — correct, and continuous into the next encounter (setup re-seeds from the
  now-depleted sheet).
- **Teardown coincides with a short/long rest** (party rests after clearing the room): rpg-api invokes
  the toolkit's existing `ShortRest()` / `LongRest()` verb on the held character
  (`character.go:430-498`), which restores pools by `ResetType` (`RestoreToFull`, hit-dice half-recovery)
  and publishes `RestEvent` (durable conditions like exhaustion may also react). Then `ToData` → store.

rpg-api never computes "how much rage is left" — it either persists consumed state or invokes a toolkit
rest verb. The rest-vs-no-rest choice is a session decision surfaced as a verb call, not an rpg-api
calculation (Invariant 2). Precedent exists: the v1 encounter orchestrator already calls
`loadedChar.LongRest(ctx)` (`internal/orchestrators/encounter/orchestrator.go:812,3799`).

### 4.3 Death / unconscious at teardown

HP and death-save state are **durable** — they cross the teardown boundary unchanged; the encounter
does not heal on exit.

- **PC at 0 HP (unconscious, making death saves)** when combat ends: write `HitPoints=0` +
  `DeathSaveState`. The sheet reflects "downed." The `unconscious` condition
  (`conditions/unconscious.go`) is a consequence of HP, not a tactical encounter-scoped condition, so 0
  HP + death-save state persist. A later heal or `LongRest` (which sets `hitPoints=maxHitPoints` and
  clears death saves, `character.go:436-439`) reconciles.
- **PC dead** (failed death saves / massive damage): write the dead state. Revival is a separate
  out-of-combat flow.

### 4.4 Mid-encounter rejoin / crash recovery — the load path

Under A the snapshot is self-sufficient for in-combat state, so recovery is trivially correct:

```
load(id):
  encRepo.Get(snapshot)          ← PlayerData.DataJSON is populated (durable)
  attachPlayerCharacterData:     ← seed-if-empty: DataJSON present → use it; empty → seed from char store
  LoadFromData(snapshot)         ← rehydrates held combatants from snapshot DataJSON; conditions
                                    subscribe to the bus exactly once (the #684 single-subscribe cure)
```

The char store is touched **only** at setup (seed) and teardown (write-back), never on the hot path. A
crash between two combat RPCs loses nothing: the snapshot was saved after the last committed verb and is
the only home — **no divergence is representable**, because there is no second store to disagree with.
(Contrast today: a crash between the char-store flush and `encRepo.Save` can split HP — the 7-vs-14
class.) A rejoining player reads the snapshot; a newly-*joining* player is the one seed-from-store case.

**Consequence to flag:** during combat the char store's `HitPoints` is intentionally **stale** (last
refreshed at the previous teardown). Any consumer wanting live combat HP must read it **through the
encounter**, not the char store. An out-of-combat character-sheet UI reading the store mid-combat sees
the pre-encounter HP — by design (the store is the out-of-combat sheet).

### 4.5 Migration path — clean break, ordered

Prefer a clean breaking change over dual-write phases (`feedback_prefer_breaking_changes`; incomplete
migrations are the bigger bug source here). There is **no stored-data migration**: durable persistence
is deferred (Invariant 13), encounter snapshots are short-lived / GC'd, so there is no persisted corpus
to convert — only the code contract flips.

1. **Toolkit first (unblocks the API break):**
   - Land [#691](https://github.com/KirkDiggler/rpg-toolkit/issues/691) — fold ActivateFeature onto the
     held combatant (the enabler; corrects its "no behavior change" premise — rage now persists).
   - Add the **combat-ended boundary** verb/event (§4.1), mirroring `RestEvent`, so encounter-scoped
     conditions self-remove at end-of-combat (not only at next turn).
2. **rpg-api (the contract flip — one coherent change):**
   - Flip `DataJSON` transient → durable (`persistPlayerCharacterData`: drop the flush + the
     `pd.DataJSON = nil`).
   - `attachPlayerCharacterData` → seed-if-empty; `SeedActorTurn` writes the snapshot, not the store.
   - Collapse `PlayerData.HP` → projection of `DataJSON.HitPoints` (or drop the field). **§6 risk.**
   - Drop ActivateFeature's `UpdatedCharData` plumbing + the handler `CharacterRepo.Update`.
   - Add the **teardown verb** (see gap below) that runs the toolkit boundary + the single write-back.
3. **Re-verify** via the barbarian-rage integration test (the defensive path,
   `integration_barbarian_rage_test.go`) + the MCP playtest sign-off bar.

**Teardown has no home yet — a real gap.** The v2 `EncounterService` exposes CreateEncounter,
GetEncounter, StreamEncounter, MoveEntity, Interact, TakeAction, EndTurn, SubmitCheck, SetReactionReady,
ActivateFeature — and **no** EndEncounter / LeaveEncounter (v1alpha1 had `LeaveEncounter`, since
archived). So Option A's "write back at teardown" needs a home that does not exist: either a new
`EndEncounter` verb, a `mode → Ended` transition hook, or a "player leaves" path. **This is the one
genuinely new surface Option A requires — call it out for Kirk (§8).** Until it lands, the safe interim
is: write back on the existing `mode → FREE_ROAM`/`Ended` transition and on any future `LeaveEncounter`;
in the current dev flow (short single-room combats) a missing teardown just means the sheet lags until
the next explicit rest — acceptable for the playtest, not for production.

### 4.6 Beat 2 fit — Dodging / Hidden / Help live encounter-scoped

Beat 2 tripling the conditions in play is exactly why A matters now:

- **Dodging** ([#699](https://github.com/KirkDiggler/rpg-toolkit/issues/699) Dodge→DodgingCondition):
  already self-removes at the owner's next `TurnStart` (`dodging.go:84-88`). Under A it lives on the
  held character's conditions on the snapshot DataJSON, **survives the per-RPC round-trip** (DataJSON is
  durable now), and self-expires via the turn machinery. **Under today's flush-through-store it would be
  stripped every turn — the exact defensive-rage bug.** Beat 2 mechanically depends on the round-trip
  being fixed; A is the clean substrate.
- **Hidden** ([#716](https://github.com/KirkDiggler/rpg-toolkit/issues/716) Hide): inherently
  encounter-scoped (hidden from combatants until you attack/move/are seen). A condition on the snapshot,
  expired by combat events; never written to the sheet.
- **Help** (#716): grants advantage to an ally's next attack — an encounter-scoped effect keyed to a
  target + expiry. On the snapshot as a condition/effect; removed when consumed or at a turn boundary;
  never on the sheet.

All three are "encounter-scoped conditions" by the §4.1 rule: they subscribe to turn/combat events and
remove themselves; they never survive the combat-ended boundary; rpg-api never sees or classifies them.
Tripling the condition count is *safe* precisely because the snapshot is their home and the toolkit owns
their lifetime.

---

## 5. Toolkit vs API — the boundary holds

rpg-api's role **shrinks** under A: it deletes a persist path (ActivateFeature's write), deletes a
per-RPC store flush, and stops maintaining a parallel HP int. It gains one thing: a teardown verb that
invokes a toolkit boundary and writes `ToData()` once. Every rule decision — what a condition's lifetime
is, what a rest restores, whether HP is halved — stays in the toolkit. No new rules conditional enters
rpg-api. (If any implementation has rpg-api inspecting `Data.Conditions` to decide what to keep, or
computing an HP delta, that is a lane violation — reject it, per the gap doc §4 lane guard.)

---

## 6. Honest costs / risks

- **HP double-home collapse is the riskiest edit.** Every site that reads/writes `PlayerData.HP`
  (`combat_phased.go:358-362`, `npc.go:908-921,1025`, `encounter.go:268,307`, plus the projection/
  translate reads) must move to the DataJSON-backed HP consistently, or the divergence flips sign
  instead of closing. Needs the field-by-field migration note above executed carefully
  (`feedback_write_plan_for_restructures`). Mitigation: keep `PlayerData.HP` as a **read projection**
  populated from the held character on `ToData` (so cheap readers keep working) but make the held
  character the only writer.
- **Snapshot blob growth.** Persisting full `character.Data` on the snapshot for the encounter lifetime
  grows the stored blob and reintroduces the "char state on the encounter" shape #689 Q1 deliberately
  avoided. This is the honest cost of one home. Mitigation: it's bounded by party size, GC'd with the
  encounter, and monsters already do it. Interacts with [#692](https://github.com/KirkDiggler/rpg-api/issues/692)
  (IsDirty beyond HP): once conditions/charges are dirty-tracked, `ToData` can gate on dirty — an
  efficiency follow-up, not a blocker.
- **Stale out-of-combat sheet during combat** (§4.4): a deliberate consequence, but any existing
  consumer that reads char-store HP mid-combat must be audited and routed through the encounter.
- **Teardown surface is net-new** (§4.5): the write-back needs a verb/hook that doesn't exist. Small,
  but real, and it's the piece most likely to be skipped and leave the sheet permanently stale.

---

## 7. North-Star Invariant validation (all 13)

| # | Invariant | Verdict |
|---|-----------|---------|
| 1 | Web sends Refs, renders server data | **No change.** State-home is server-internal; the web never saw the char store. |
| 2 | rpg-api orchestrates by key, no rules conditionals | **Improved.** A deletes the ActivateFeature bespoke write and the ActionEconomy authoring. **Tension to guard:** the teardown write-back must be an unconditional `ToData()` after a toolkit boundary — never an rpg-api field-picker. Stated as a hard rule (§4.1, §5). |
| 3 | Toolkit owns all rules | **Strengthened.** Condition lifetime + rest reconciliation move fully into the toolkit boundary verb; the classification rule *is* Invariant 3. |
| 4 | protos + rpg-api one contract unit | **No new proto** for the state-home itself; the teardown verb (§4.5) is a first-class proto add when it lands, not a minimize-away. |
| 5 | Events stamped with game-event time | **No change.** HP/condition change events still flow from the toolkit as today (TakeAction wave owns the timestamp/correlation work). |
| 6 | rpg-api projects for audience, not rendering | **No change.** |
| 7 | Toolkit field with no proto counterpart = proto gap | **No change** (no event narrowing here). |
| 8 | Events carry causation | **No change** (owned by the TakeAction wave). |
| 9 | First-class resolved-action event | **No change.** |
| 10 | RPC responses are minimal acks | **Improved.** `ActivateFeatureOutput.UpdatedCharData` (a state payload on the response) is **deleted** — state flows via the snapshot, not the ack. |
| 11 | Capability is server-sent data | **No change** (menu/economy owned by TakeAction; economy now rides the snapshot DataJSON, still toolkit-computed). |
| 12 | Illegal actions pre-empted in the menu | **No change.** |
| 13 | Durable persistence deferred, contract forward-compatible | **Consistent, with a nuance to name.** A does **not** add a new durable store (event log/replay) — it changes *what the already-persisted snapshot carries* (DataJSON durable vs transient). No migration corpus. The snapshot-blob-growth cost (§6) is the price; it's forward-compatible with #692 dirty-tracking. |

**No invariant is violated.** The only live tensions are (a) keeping teardown write-back field-pick-free
(Invariant 2 — designed against) and (b) the Invariant 13 nuance that the snapshot now carries more
(named, bounded, GC'd).

---

## 8. Open questions that genuinely need Kirk

1. **Teardown surface (§4.5).** Option A needs a write-back home that doesn't exist. Preference:
   (a) a new `EndEncounter`/`LeaveEncounter` verb, (b) hook the `mode → FREE_ROAM/Ended` transition, or
   (c) tie write-back to an explicit rest verb? Recommendation: hook the mode transition now (cheapest,
   already the natural combat→free-roam boundary) and add an explicit `LeaveEncounter` when
   multiplayer-leave lands. This is the one net-new surface — worth a Kirk decision.
2. **`PlayerData.HP`: projection vs delete (§3.1, §6).** Keep it as a read-only projection off DataJSON
   (cheaper reads, one writer) or delete it entirely and always read HP from the held character?
   Recommendation: keep as projection during the migration to de-risk the cutover, then consider
   deleting once all readers go through the encounter.
3. **Combat-ended boundary is a toolkit add (§4.1).** Confirm the toolkit should grow an `EndCombat`
   event mirroring `RestEvent` (my read: yes — it's the clean, existing-pattern way to make
   encounter-scoped conditions self-clear at end-of-combat, not just at next turn). Not a symmetric
   fork; flagging only because it's a toolkit-scope decision.

---

## 9. Issue breakdown (under the #596 umbrella / Chapter-1 honesty)

| # | Repo | Title |
|---|------|-------|
| 1 | rpg-toolkit | Fold ActivateFeature onto the held combatant — **is [#691](https://github.com/KirkDiggler/rpg-toolkit/issues/691)**; drop `UpdatedCharData` self-load |
| 2 | rpg-toolkit | Add a combat-ended boundary event (mirror `RestEvent`) so encounter-scoped conditions self-remove at end-of-combat |
| 3 | rpg-api | Make the encounter snapshot the single home for in-combat state: DataJSON durable, seed-if-empty, drop per-RPC store flush |
| 4 | rpg-api | Collapse `PlayerData.HP` → projection of `DataJSON.HitPoints` (end the 7-vs-14 divergence) |
| 5 | rpg-api | Add teardown write-back (verb/mode-hook): run toolkit boundary + single `ToData()` → char store |
| 6 | rpg-api | Drop ActivateFeature's `CharacterRepo.Update(UpdatedCharData)` once #691 lands |
| 7 | rpg-api | Integration: un-skip / assert the defensive-rage path survives across the per-RPC round-trip under the new home |

Items 3+4+6 are one logical unit (the contract flip) and can ship as one PR; 1+2 are the toolkit
prerequisites; 5 is the net-new surface (gated on the §8.1 decision); 7 is the re-verification gate.
Interacts with — does not fold in — [#692](https://github.com/KirkDiggler/rpg-api/issues/692) (IsDirty).
