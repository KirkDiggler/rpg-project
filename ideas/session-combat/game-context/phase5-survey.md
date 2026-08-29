# Phase 5 survey — request-shaped writes (D3/D4/D9)

Surveyed 2026-08-29 against rpg-toolkit `origin/main` @ `34746bc`. Full trace
by the survey agent; this file keeps what shapes the build.

## Headline

The keeper pattern Phase 5 extends is **already a table**. Each sheet kind has
a concrete `SheetKeeper` whose subscriptions are a list of
(topic → handler) rows (`character/sheet_keeper.go:137-153`,
`monster/load.go:199-214`), already applying `ConditionApplied`/
`ConditionRemoved`/`HealingReceived` and marking dirty. Phase 5 adds rows.
The doctrine is even written in the code twice
(`resolution/activation.go:84-88`, `gamectx/doc.go:81-93`) — this phase makes
the last four writers obey it.

## The four writers (what actually moves)

| condition | write | when it fires |
|---|---|---|
| Raging | `markDirty()` on damage-received (guarded), own turn-end (unconditional), post-attack-roll (guarded) | topic + publish phase |
| Sneak Attack | `UsedThisTurn` + `markDirty()` on damage-chain publish; reset on turn-end (guarded) | publish phase |
| Opportunity Attack | trigger publish, then `UsedThisTurn`, `purse.SpendSlots(Reaction,1)` (nil-guarded), `markDirty()`; reset on own turn-START | publish phase |
| Protection | disadvantage stage + `ReactionConsumption` append + `owner.SpendSlots(Reaction,1)`; **never calls MarkDirty** (dirties via `Character.SpendSlots → economyChanged`) | **inside a stage closure, during Execute** |

Handles that die when these go: `selfPersisting` (raging.go:68),
`protectionOwner` (fighting_style_protection.go:36) — both unexported, used
only here. `combat.Ledger` itself stays (the gate's account surface); only
the conditions' hold on it goes. `SetOwner` wiring
(`character/load.go:163-165`, `monstertraits/loader.go:285-287`) survives as
a dead type-assertion until Phase 6 deletes it.

## Findings and dispositions

### F1 — Write timing: keeper applies synchronously at publish. RULED 2026-08-29.

The bus is fully synchronous (`events/bus.go:99`); a keeper that subscribes
normally lands each request at the **identical instant** the direct call
lands today — zero ordering analysis, behavior preserved by construction.
Buffering writes to post-fold would break a **documented live dependency**:
`resolution/movement.go:133-140` relies on OA's meter having landed *during*
the fold so a second trigger in the same walk sees the spent slot. R7 is not
violated: R7's buffering governs reaction *answers* (new resolutions —
`collectTriggers` is exactly that); sheet *writes* applied by keepers have
been synchronous-at-publish since ConditionApplied, and D3/D4 say "applied by
the sheet keeper," which is this pattern. Attach order already guarantees the
keeper subscribes before any condition (`resolution/resolve.go:399-402`,
keeper at `character/load.go:135` before conditions at `:157`).

### F2 — Event shapes. RULED 2026-08-29 (“condition state change makes a
lot of sense to me. I am saying go on that”).

- **`ConditionStateChangedEvent{MemberID, ConditionRef}`** — new topic.
  Kirk's naming catch on the draft (`MarkDirtyRequested`): the draft name was
  command-shaped while every existing keeper event is fact-shaped
  (ConditionApplied, HealingReceived — things that happened). The condition
  states a fact — “my slice of your sheet changed where you can't see it” —
  and marking dirty is the keeper's own response. Both `MarkDirty()` bodies
  are an unconditional boolean set, so the payload needs nothing more; the
  ref buys log legibility for free. Keeper filters on ID like every existing
  handler.
- **Reaction spend: supersede the dead `ReactionUsedEvent`**
  (`events.go:659`, topic `:946` — zero publishers, zero subscribers,
  payload is `CharacterID`-shaped with no slot/count, so it cannot serve the
  monster half). Zero users = deletion is free (no-backcompat law). Mint
  **`SpendRequestedEvent{MemberID, ActionType, Amount, SourceRef}`** —
  mirrors the `SpendSlots(ActionReaction, 1)` call it replaces.

### F3 — The read side. RULED 2026-08-29: the member surface gains the reader.

OA gates on `purse.SlotsLeft(Reaction) > 0` (`opportunity_attack.go:272`);
Protection on `owner.SlotsLeft(Reaction) > 0`. The purse handle dies; the
readiness tenant is an **opt-in map** (`gamectx/reaction_readiness.go:18` —
who chose to react), not a slot counter, so it does not answer this.
**Recommendation: the member surface gains one reader**, D7's exact family
(D7 added `HasShieldEquipped` for Protection's other read; monster = false).
Name candidate `CanReact() bool`: character = `SlotsLeft(ActionReaction) > 0`;
**monster = true** — the truthful zero: today's code refuses only when an
economy exists and says no (`if o.purse != nil && SlotsLeft <= 0`), and a
monster has no economy to refuse (Kirk's asymmetry ruling,
`monstertraits/loader.go:282-284`). `false` means "my economy refuses," and
no economy never refuses — same truth as the nil-purse branch, now stated on
the surface instead of hidden in a nil check.

### F4 — The asymmetry made mechanical: publish both, each keeper applies what its kind holds. RULED 2026-08-29.

OA publishes **spend + dirty**; Protection publishes **spend** (its dirty
already rides the debit). Character keeper: applies spend (debit
auto-dirties via `economyChanged`) and dirty (idempotent boolean). Monster
keeper: has no economy row at all — applies dirty only; the spend request
passes it by, truthfully. Per-kind behavior today is reproduced with zero
keeper-side conditionals, and D4's "asymmetry is a keeper concern" becomes
literally which rows each keeper's table has.

### F5 — Pre-existing gap: the monster keeper has no `ConditionRemoved` subscription. PULLED INTO PR A (Kirk, 2026-08-29).

A monster's condition removal never reaches its sheet today
(`monster/load.go:199-214` — three handlers, no removal) — latent because no
production path removes a monster condition yet. PR A is editing exactly that
table, so the row and its test land there and the keepers come out
symmetric.

### F6 — Second inert artifact: `AttackChainEvent.ReactionsConsumed`.

Populated by Protection (`events.go:313`), read by nobody outside its own
test. Phase 5 doesn't touch it (it is fold-output record, not a request);
Phase 6 sweep decides keep-or-delete.

## PR split (module chain: everything lands in dnd5e, then pins)

- **PR A (dnd5e, additive):** new topics/events + keeper rows (incl. the
  monster `ConditionRemoved` row, F5) + the F3 member reader. Inert until
  published; keepers listening before any publisher exists. Supersede/delete
  `ReactionUsedEvent` here.
- **PR B (dnd5e):** the four conditions publish instead of write;
  `selfPersisting`/`protectionOwner` deleted; test fakes lose their writer
  methods. Both-ways proof per Phase 4: behavior pinned identical.
- **PR C/D (resolution, session):** pin bumps; `dirty_test.go` and
  `clockboundary_test.go` are the end-to-end truth.

## The test contract (must pass identical)

Direct handle pins to rewrite: `opportunity_attack_meter_test.go` (the whole
meter incl. purseless-monster + no-reaction-left + JSON round-trip),
`fighting_style_protection_test.go`, `integration/fighter_encounter_test.go`,
`monstertraits/monster_carries_a_condition_test.go` (the loader-wiring pin —
asserts `m.IsDirty()` through a real fold), `monster_markdirty_test.go`,
`resolution/dirty_test.go` (the isolating end-to-end: one dirty character,
`did_attack_this_turn` in the serialized blob). Indirect pins that must not
notice: `session/clockboundary_test.go`, `session/attack_test.go`
(Protection on the session stack), the raging/sneak-attack/OA suites.

## Seeds carried forward (Phase 6)

`ReactionsConsumed` keep-or-delete (F6); `OwnerAware`/`SetOwner` + both loader wirings + the
dead-assertion window closes; MarkClean vestigial + only-keeper-names-
Combatant pin (from phase4-survey.md).
