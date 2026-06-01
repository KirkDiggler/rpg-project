# TakeAction — Wave 1 (the birth wave)

**Chapter 2 · v1alpha2 encounter route**
**Validated against:** the North-Star Invariants in [`../design.md`](../design.md). Reviewed by a
3-lens adversarial panel on 2026-06-01 (boundary / completeness / event-fidelity); findings folded
in below. Decisions resolved with Kirk inline.
**Status:** wave content — becomes the implementer's star on Kirk's sign-off.

---

## Why this is the birth wave

Rage (ActivateFeature) proved the alpha2 path works end-to-end. TakeAction is the first
**verb-shaped** wave of Chapter 2, and the decisions here set the patterns every later verb inherits
(Interact, SubmitCheck, reactions, move). We get the boundary right once, here, and reuse it.

## Goal behavior (the done bar)

> A character takes its turn honestly — any *available* action including a **bonus action**, economy
> enforced server-side, the full story on the wire. Proven via **MCP playtest**: an action *and* a
> bonus action are taken, the economy decrements, **misses are visible**, and the
> `available_actions` menu drives the UI. The same wiring makes plain attack honest too. The **retro
> across all sessions closes the wave** — not green CI.

---

## The real shape of the work (what the review surfaced)

The first draft hid the central task behind a tidy phrase. The honest picture:

### The core task is unifying two toolkit worlds

Today the encounter **verb** path (`rpg-toolkit .../encounter/combat_phased.go`) only knows `"attack"`
and reads a `combat.ActionEconomy`. The **rich action menu** (`.../character/buildAvailableActions`,
driven by *granted capacities*) lives in the **character** package and is **never consulted by the
encounter path**. "Toolkit computes the menu" (Invariant 11) means *reconciling these two worlds and
deciding which is canonical*. **This is the largest piece of the wave** — it is named here so it
can't hide behind "organize action handling" again.

### Pillar 3 is gated on additive toolkit-spine changes (not "translation")

The event work is net-new in the engine, not a wiring tweak. The doc must not pretend otherwise:

- **A resolved-action event** carrying `action_ref` + `economy_consumed`. Today `AttackResolvedEvent`
  has neither, and the publish site (`rpg-toolkit .../encounter/combat.go:~291` `publishAttackOutcome`) isn't even handed the
  action ref — it must be threaded down. Plus a **new proto event** in the `EncounterEvent` oneof:
  there is no `ActionResolved`/`AttackResolved` variant today, so "un-suppress" alone throws
  `ErrUnknownEventType` (Invariant 9).
- **A correlation id** on the toolkit `EncounterEvent` interface *and* the proto envelope. Today the
  only link between `AttackResolved` and `DamageDealt` is adjacent `nextSeq()` values — fragile and
  implicit (Invariant 8).
- **A timestamp** on the toolkit event interface + an injected clock. Today the spine carries only
  `Sequence`; rpg-api stamps `now` at translate-time, which is delivery order, not game order
  (Invariant 5).

**Dependency order:** the proto event + the toolkit struct fields land **before** rpg-api stops
suppressing — otherwise the un-suppress yields `ErrUnknownEventType`.

---

## Resolved decisions (with Kirk, 2026-06-01)

| Decision | Resolution |
|---|---|
| **Menu/economy refresh** | **Push** a `TurnState`/economy delta event; the web updates the menu live off the stream (Invariant 12). No polling. |
| **Bonus-action source** | **Monk Martial Arts** unarmed strike — a single class feature granting a bonus-action capacity with no off-hand-weapon dependency, so it unblocks the Monk (already in the playtest cast) soonest. |
| **Rejection transport** | Pre-empt illegal actions in the menu (`available=false` + toolkit-authored `unavailable_reason`) so the UI never offers them; structural/turn errors stay gRPC status codes. **No new rejection field on the response.** |
| **Targeting** | Add a **target-kind** hint to the toolkit `AvailableAction` *and* proto `AvailableAction`, so the UI raises the right prompt (self / single-entity / position / area) without knowing rules. Front-load the target in `ActionTarget` this wave; reserve `InputRequired` for genuine mid-resolution prompts. |
| **Multiattack / capacity** | Mirror the toolkit's two-level model (an action *grants* an `attacks` capacity; each strike *consumes* one). Pin the exact action-vs-strike wire shape during the toolkit unification — Fighter Extra Attack inherits it. |
| **Correlation id + timestamp on the spine** | **Add now** (Invariants 5, 8) — the forward-compat price of a toolkit-owned combat log. |
| **Persistence** | Deferred (Invariant 13). The event contract is shaped to be forward-compatible. |

---

## Work by layer

| Layer | Work |
|-------|------|
| **rpg-toolkit** *(the core of the wave)* | Reconcile the `encounter` verb path with the `character` action-economy/menu; expose the computed **action menu as data** (`ref`, `display_name`, `available`, `unavailable_reason`, slot, **target-kind**) — toolkit-authored, no API computation. Implement non-attack actions (Dodge / Dash / Help / Hide) + the Monk Martial Arts bonus-action strike. **Validate + deduct** the economy while resolving. Add to the event spine: a resolved-action event with `action_ref` + `economy_consumed`, a **correlation id**, and a **timestamp** stamped at publish. Own the combat log. |
| **rpg-api-protos** | New `ActionResolved` event in the `EncounterEvent` oneof; **correlation id + confirm game-time timestamp** on the envelope; **target-kind** on `AvailableAction`; the economy/`TurnState` **delta event** for push refresh. |
| **rpg-api** | Route **any** `action_ref` through TakeAction (drop the attack-only gate); **project** `TurnState.economy` + `available_actions` from the toolkit **field-for-field, zero rules conditionals**; **push** the delta event on economy change; stop suppressing the resolved-action event; faithful translation only. **Never deducts, never decides the menu, never authors the strings** (Invariants 2, 6). |
| **rpg-dnd5e-web** | Render `available_actions` grouped by economy slot; use **target-kind** to raise the right targeting prompt; render the resolved-action beat (incl. **misses**); show economy remaining; consume the pushed delta. |

### Known leak to clean — do **not** copy it

`rpg-api .../v2/encounter/activate_feature.go:~50-112` already has rpg-api constructing
`ActionEconomyData{1,1,1, Movement:30}` and injecting it when unset — the API authoring rules state
(an Invariant 2 violation that predates this wave). This wave **moves economy seeding into the
toolkit's StartTurn**; do not replicate the injection inside TakeAction.

### Separable PRs (wave = grouping, not a PR)

The **event-faithfulness** work (resolved-action event + causation + timestamp; fixes #594 silent
misses) and the **economy/menu unification** are loosely coupled and can ship as separate PRs under
this wave umbrella, in any session-sized chunks.

---

## Operating model — how we run this wave

- **Wave = grouping, not a PR and not a session.** The umbrella issue is the unit of tracking;
  multiple issues + PRs across repos roll up to it. A session takes on whatever it can hold.
- **Living progress ledger** under the wave gives the next session/agent immediate bearings.
- **Retro accumulates across all sessions** and is the closing artifact. Mid-wave gaps not worth
  pausing for become follow-up issues *under* the wave; the retro captures them all.

## Open items (tracked under the wave, not blockers)

- **Movement units** — hexes vs feet for `movement_remaining` (north-star §7 defers it; the economy
  delta event surfaces the question).
- **Idempotency** — a retried `TakeAction` (network) could double-decrement; no idempotency key
  today. Note as a known rough edge.
- **Reaction-pause interplay** — `take_action.go:~156` single-reactor enforcement; reactions are a
  later wave, cross-referenced so it isn't re-derived.

## Related / inherited

- Builds on the rage / ActivateFeature verification (same alpha2 route).
- Closes/touches: **#594** (surface attack misses); economy present-but-unwired in proto + toolkit.
- Adjacent, not blocking: **#595** (deterministic initiative).
- Explicitly **not** this wave: durable event log / replay / metrics projections; reactions
  (`SetReactionReady` — its own wave); the full combat-log-as-product toolkit feature (this wave only
  keeps events faithful + correlated enough that it's buildable later).
