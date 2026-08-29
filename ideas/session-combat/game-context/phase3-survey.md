# Phase 3 survey — sites, signatures, PR split

Surveyed 2026-08-29 by the phase agent from origin/main tip (84abf19,
post-#1290), read-only. Director spot-checked the load-bearing counts (three
`newCallBus` call sites; `Combatant.AbilityScores` at combatant.go:77;
`Character.HasShieldEquipped` at character.go:980) before approving the split.

## Director dispositions on the flags (§5 below)

- **F1 (rpg-api's sheet AC loses Unarmored Defense at bump):** recorded as an
  adoption obligation — rpg-api's `EffectiveAC` call sites owe the R6
  migration (folds come to resolution) before it bumps past PR 2. Free today:
  rpg-api pins old versions. No Phase 3 action.
- **F2 (Standing entry strictness):** RULED by Kirk 2026-08-29 ("I agree
  visibility is what we need"): lenient for all participants with the
  warnDropped-shaped log (D10: observable, not refused) — a corrupt record
  drops from death reporting with a warn instead of erroring the whole
  check; the resolve.go "lenient monster is unreachable" comment corrected
  in the same PR (principle 6). PR 5 is unblocked.
  **AMENDED by Kirk 2026-08-29, after PR 5 found monsters have no lenient
  loader:** "this all seems premature. we would put bad data in or possibly
  not migrate existing. encounters are not long lived so this seems like we
  are wasting cycles for things that I currently do not see a way to get
  into that state." So: the character-lenient arm stays only because it is
  pre-existing pinned behavior PR 5 preserved while rerouting; the lenient
  MONSTER loader is NOT built and NOT queued — premature until a real path
  to a corrupt record exists. The asymmetry pin
  (TestAnUnreadableTraitStillRefusesAMonster) stays as the record.
- **F3 (smaller bus-free loader alternative):** already ruled. D11's ToData
  probe rejected exactly this shape — session holding a live sheet is the
  defect, not the bus per se. Entries return answers; no re-ask.
- **F4 (compileResolutionCast is two jobs):** approved as surveyed — the
  fetch half is R3 record-gathering and stays in session; the entry replaces
  only the attach half.
- **F5 (reentrancy):** approved — the invariant is held by a comment today;
  PR 7 adds the pin.
- **F6 (GetSpeedBonus shape):** decided — `SpeedBonus(ctx) (int, bool)`. The
  Input/Output law protects call sites whose inputs grow; this query has no
  inputs, and the bool is the honest zero ("no bonus known"), which an error
  the caller must swallow is not.
- **F7 (D7 timing):** lands in PR 1 as ruled — Unarmored Movement's migrated
  reader consumes it inside Phase 3; deferral would re-open a ruled line.

---

## 1. Reader sites — the three conditions

- **Unarmored Defense** — read at `conditions/unarmored_defense.go:230` inside
  `onACChain` (a CHAIN handler). Reads `AbilityScores().Modifier(CON|WIS)`.
  Via `unarmoredDefenseOwner` iface `:56-58`, field `:52`, `SetOwner` `:80`.
- **Martial Arts** — read at `conditions/martial_arts.go:173` (`onDamageChain`)
  and `:280` (`onAttackChain`), both CHAIN handlers. Reads `AbilityScores()`
  for the STR-vs-DEX swap. Via `martialArtsOwner` `:47-50`, field `:41`,
  `SetOwner` `:61`.
- **Unarmored Movement** — read at `conditions/unarmored_movement.go:159`
  inside `isUnarmored`, reached from `GetSpeedBonus()` — **not a handler, no
  ctx**. Reads `HasShieldEquipped()`. Via `unarmoredMovementOwner` `:41-45`,
  field `:36`, `SetOwner` `:55`.

All three key off their own `CharacterID`, so `CastOf(ctx).Member(ownID)`
substitutes directly. `combat.Combatant` **already carries `AbilityScores()`**
(`combat/combatant.go:77`) — UD and MA need NO surface change. Only Unarmored
Movement needs D7.

**`GetSpeedBonus()` has zero production callers.** Its only caller anywhere is
`integration/monk_encounter_test.go:951`, through a duck-typed
`interface{ GetSpeedBonus() (int, bool) }`.

### OwnerAware inventory — 3 go, 4 stay

Deleted in Phase 3 (reads): UnarmoredDefense, MartialArts, UnarmoredMovement.
Stay for Phase 5 (D3/D4/D9 writes): `RagingCondition` (`raging.go:79`,
MarkDirty), `SneakAttackCondition` (`sneak_attack.go:57`, MarkDirty),
`OpportunityAttackCondition` (`opportunity_attack.go:119`, selfPersisting +
combat.Ledger), `FightingStyleProtectionCondition`
(`fighting_style_protection.go:69`, HasShieldEquipped + SlotsLeft/SpendSlots).
Interface + both bespoke wirings STAY until Phase 6: `events/events.go:151`,
`character/load.go:163-164`, `monstertraits/loader.go:285-286`.

Test blast radius for the three: 25 `SetOwner` call sites
(martial_arts_test.go 20, unarmored_movement_test.go 3,
unarmored_defense_test.go 2). `conditions/fake_owner_test.go:20`
(`fakeConditionOwner`, 3 methods) must become a full `combat.Combatant`;
`conditions/fake_cast_test.go:23` — whose `Member` returns `(nil,false)`
today — must serve real members.

## 2. Session's call-bus sites — the three ARE the complete set

`newCallBus` defined `session/entities.go:57`. Exactly three call sites,
matching D11, no fourth:

1. `session/write.go:209` — Join → `m.loadCharacter(ctx, newCallBus(),
   in.Member)` (`entities.go:138`, uses `character.LoadFromData`).
2. `session/standing.go:193` — `standingSeam.sheetOf` →
   `character.LoadFromData(s.ctx, data, newCallBus())`.
3. `session/attack.go:615` — `compileResolutionCast` preflight loop
   (`:622-630`: `character.Load`+`Attach`, `monstertraits.LoadMonster`+
   `AttachMonster`).

**Key measurement for the pin:** `rpg-toolkit/events` is imported by exactly
ONE non-test file in session (`entities.go`) and **ZERO test files**. So the
no-bus pin can INCLUDE test files at zero cost — the strong form Phase 1
established.

Surviving live-sheet sites, bus-free, out of scope: `attack.go:548`
(`loadAttackSheet`), `standing.go:147` (`monster.Load` — moves with the
standing entry), `write.go:922` (`exitCombatIfPlayer`, a write path). An
unattached sheet cannot fold (EffectiveAC refuses, #1276), so these do not
weaken the pin.

## 3. Proposed signatures

**D7 — added to `combat.Combatant` (`combat/combatant.go`):**

```go
// HasShieldEquipped reports whether a shield is in play for this combatant.
// A monster answers false: its shield is baked into the stat-block AC it
// already reports, so there is nothing here for a rule to add.
HasShieldEquipped() bool
```

Implementors: `*character.Character` (exists, `character.go:980`),
`*monster.Monster` (new, `false`), generated `combat/mock`, and 6 test fakes
(`character/integration_test.go:84`, `combat/combatant_dirty_test.go:42`,
`combat/damage_test.go:38`, `integration/fighter_encounter_test.go:110`,
`integration/rogue_encounter_test.go:79`,
`resolution/strike_effective_ac_refusal_test.go:41` — the last lands only
when resolution bumps).

**Readers — one shared unexported helper in `conditions`,** so "cannot
answer" is spelled once:

```go
// self returns this condition's own combat-facing sheet out of the cast, and
// whether the question could be asked at all. Absent is not an error: an
// errored fold discards every other contributor along with this one.
func self(ctx context.Context, ownID string) (combat.Combatant, bool)
```

Unarmored Movement's reader gains ctx: `SpeedBonus(ctx) (int, bool)` (F6).

**Resolution entries — data in, answers out, nothing with `ToData()`
crossing:**

```go
// projection.go — ProjectCharacterOutput GROWS so Join makes one call, not two.
type ProjectCharacterOutput struct {
    ArmorClass *combat.ACBreakdown // existing
    Sheet      CharacterFacts      // new
    MainHand   *AttackFacts        // new; nil = no main-hand attack compiled
}
type CharacterFacts struct {
    ID, Name         string
    Level            int
    SpeedFeet        int // derived from race — the one value not on the record
    HitPoints        int
    MaxHitPoints     int
    ProficiencyBonus int
}
// AttackFacts is the main-hand definition as numbers. Kind's zero value is ""
// — absent, not melee; a bool would make "nothing compiled" read as ranged.
type AttackFacts struct {
    Ref       core.Ref
    Name      string
    RangeFeet int
    Kind      string // "" | "melee" | "ranged"
}

// standing.go
type StandingInput struct {
    // Participants is who to ask about. A member with no record is not asked
    // about and is never reported down — "no sheet, no death".
    Participants []Participant // the same type Resolve takes
}
type StandingOutput struct {
    // Down names the members at zero hit points or below, in cast order.
    // Empty is the ordinary answer.
    Down []string
}
func Standing(ctx context.Context, in *StandingInput) (*StandingOutput, error)

// preflight.go
type PreflightInput struct {
    Participants []Participant
    Roller       dice.Roller
}
type PreflightOutput struct {
    // Unreadable names every participant this cast could not attach, in cast
    // order. Empty means Resolve would attach all of them.
    Unreadable []ParticipantRefusal
}
type ParticipantRefusal struct {
    Member string
    Reason error
}
func Preflight(ctx context.Context, in *PreflightInput) (*PreflightOutput, error)
```

**Mechanism note on Preflight:** `attachAll` (`resolve.go:419-435`) ABORTS on
first failure, but `offers.go:303-320` needs EVERY failing member to mark the
right candidate rows. So Preflight cannot just call attachAll — it needs a
collecting pass (attach one at a time onto the same surface, gather refusals,
tear down once). Session's current loop already does that; the difference is
that it never tears the surface down, which the entry would fix.

**The no-bus pin** replacing `session/folds_live_in_resolution_test.go:61`:
`TestNoBusLivesInThisModule` — no file in this module, TEST FILES INCLUDED,
imports `rpg-toolkit/events` or `rulebooks/dnd5e/gamectx`. A fold needs a
bus; a bus needs that import; so this is name-independent where the
literal-match tripwire it replaces was not.

## 4. PR split (approved)

Modules: M1 = `rulebooks/dnd5e`; M2 = `resolution` (pins dnd5e v0.109.0);
M3 = `session` (pins resolution v0.21.0). The two halves are independent —
the M2 entries need no M1 bump. Readers run first because PR 2 closes the
honest limit Phase 2 PR-A measured and recorded (until then the door does
not affect the AC VALUE).

1. **PR 1 (M1) — the member surface answers the shield question (D7).**
   `Combatant` gains `HasShieldEquipped`; monster answers false with the
   stat-block reason; mock + 5 in-module fakes follow. Purely additive.
2. **PR 2 (M1) — Unarmored Defense reads itself off the cast.** Owner
   iface/field/SetOwner deleted; `self()` helper lands; `fakeConditionOwner`
   becomes a Combatant and `fakeCast.Member` starts serving. Parity pinned by
   VALUE (15, not stored-11, not unattached-12) on both paths, plus the
   not-in-cast branch leaving the chain untouched. The only one of the three
   with a live production fold (ProjectCharacter → Join), so it proves the
   mechanism end-to-end first.
3. **PR 3 (M1) — Martial Arts reads itself off the cast.** Two handlers, 20
   test call sites — separate so a conflict there does not hold the others.
4. **PR 4 (M1) — Unarmored Movement reads itself off the cast.**
   `GetSpeedBonus` → `SpeedBonus(ctx) (int, bool)`; the integration test's
   duck-typed literal follows. Tag M1 after this.
5. **PR 5 (M2) — the answer entries.** `ProjectCharacter` grows
   Sheet/MainHand; `Standing` and `Preflight` land with no callers. Bumps M1,
   tags M2. Gated on the F2 ruling.
6. **PR 6 (M3) — Join asks instead of loading.** `loadCharacter` and its bus
   argument go; `memberActionsFromCharacter` becomes a mapping from
   AttackFacts; `place` takes name/speed from the answer. Both tripwire pins
   (`entities_test.go:271`, `:372`) must pass byte-identical.
7. **PR 7 (M3) — standing and preflight reroute; the bus dies.**
   `standingSeam.sheetOf` and the `compileResolutionCast` attach loop move
   behind the entries; `newCallBus` deleted; `TestNoBusLivesInThisModule`
   lands and `TestNoFoldLivesInThisModule` retires; the F5 reentrancy pin
   lands. Splits as 7a (standing) / 7b (preflight + deletion + pin) if it
   runs long.

## 5. Design meets code reality — the flags (dispositions above)

1. **`character.EquipmentView` folds AC outside resolution, and rpg-api calls
   it.** `character/equipment_display.go:106` calls `c.EffectiveAC(ctx)` with
   no gamectx; no in-toolkit caller, but
   `rpg-api/internal/orchestrators/character/view.go:163` calls
   `EquipmentView(ctx)` and `orchestrator.go:947,1021` call
   `char.EffectiveAC(ctx)` directly. Today Unarmored Defense reaches those
   via SetOwner. After PR 2 it contributes NOTHING there — the no-error
   branch leaves the chain untouched — so whenever rpg-api bumps, the
   character-sheet AC silently loses Unarmored Defense: the
   ac-is-a-cached-scalar bug returning by the other door. R6 says those folds
   must come to resolution too; a consequence Phase 3 CREATES and does not
   resolve.
2. **The standing seam's policy is asymmetric and one entry must pick one.**
   `standing.go:193` loads characters LENIENTLY (pinned as deliberate) while
   `:147` loads monsters STRICTLY (`monster.Load` → ErrInvalidSession). D10
   makes strictness a property of the entry. `attachAll`'s own comment
   (`resolve.go:426-430`) states a lenient monster is UNREACHABLE by
   construction — a Standing entry with DropUnreadable makes that reachable
   for the first time and falsifies the comment.
3. **Neither Join's nor standing's bus does any folding.** Both exist only
   because `character.LoadFromData` REQUIRES a bus to attach; no chain runs
   on either. A smaller mechanism exists (export a lenient bus-free loader
   from `character`) — but it leaves session holding a live
   `*character.Character`, which D11's letter rejects.
4. **`compileResolutionCast` is two jobs and only one moves.** The fetch half
   (`attack.go:591-607`) is R3 record-gathering, legitimately session's; the
   entry replaces the attach half (`:613-638`); session keeps producing fetch
   failures into the same `resolutionDependencyFailure` list.
5. **Reentrancy: measured, currently clear, worth a pin.** `resolution.Resolve`
   touches the encounter only through `LoadEncounter`, `Canvas()`, `ToData()`;
   `encounter.standingNow` is never on Resolve's path. But session's
   `striker.go:118` and `announcer.go:81` call Resolve from inside an
   encounter pass that separately consults standing — sequential, not nested,
   under R7; today the invariant is held by a comment
   (`resolution/errors.go:33`).
6. **`GetSpeedBonus`'s signature is unconstrained by any production caller** —
   shape decided in F6.
7. **D7's timing is arguable** — resolved in F7.
