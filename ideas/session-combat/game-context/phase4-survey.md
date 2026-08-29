# Phase 4 survey — read-only member surface (D5)

Surveyed 2026-08-29 against toolkit `origin/main` (post-#1299). Line refs are
origin/main.

## Headline

**The cast is already read-only in practice.** Zero production call sites
mutate a value obtained from `CastOf(ctx)` / `Cast.Member(...)`. Every
mutation runs through one of two lanes that D5 leaves alone:

- **Keeper lane**: resolution's sealed `*Participants` via `combatantFor`
  (strike.go:600) and `ledgerFor` (cost.go:181) — package-internal, never the
  `gamectx.Cast`.
- **Owner-handle lane**: the four remaining `OwnerAware` writers (raging,
  sneak_attack, opportunity_attack, fighting_style_protection) mutate only
  their injected owner (`MarkDirty`, `SpendSlots`). None touches the cast.
  Phase 5 converts these to request events; Phase 4 cannot break them.

So Phase 4 is **compiler enforcement of an invariant that already holds** —
principle 3's exact shape: make R2 true in the type system now, so the
unwritten future call site inherits safety (a condition author physically
cannot call `ApplyDamage` on a cast member).

## The surface today

`combat.Combatant` (combat/combatant.go:53-100), eleven methods:

- **Readers (9):** GetID · GetHitPoints · GetMaxHitPoints · AC ·
  HasShieldEquipped · IsDirty · AbilityScores · ProficiencyBonus ·
  PassivePerception
- **Mutators (2):** ApplyDamage · MarkClean

`MarkDirty` is not on the interface (concrete types + `selfPersisting` only).
`EffectiveACCalculator` is a separate reader-only widening interface.

Cast readers in production, all through `conditions/member.go`'s `member()`
or `CastOf` directly: UD/MA/UM read `AbilityScores`/`HasShieldEquipped`;
sneak_attack:282 and pack_tactics:173 read only `IsHostile`/`IsAllied`.
Session never names `combat.Combatant` at all — **unaffected**.

## Proposal

New interface `combat.Member` — the name mirrors what it is: **what
`Cast.Member` returns**. `Combatant` embeds it and keeps the writer/keeper
surface:

```go
type Member interface {        // the read surface a rule sees a member through
    GetID() string
    GetHitPoints() int
    GetMaxHitPoints() int
    AC() int
    HasShieldEquipped() bool
    AbilityScores() shared.AbilityScores
    ProficiencyBonus() int
    PassivePerception() int
}

type Combatant interface {     // Member + the keeper's bookkeeping and writes
    Member
    IsDirty() bool
    ApplyDamage(...) ...
    MarkClean()
}
```

**`IsDirty` stays off `Member` deliberately** (ownership interrogation: why
does a rule reading a member need the keeper's persistence bookkeeping? It
doesn't — zero cast-sourced callers exist; the only production IsDirty reads
are resolution's own dirty-set build at resolve.go:539/552, on concretes).
The dirty trio (IsDirty/MarkClean, + MarkDirty on concretes) is one
responsibility and it is the keeper's.

Signature changes: `gamectx.Cast.Member` returns `combat.Member`
(cast.go:60); `resolution.castView.Member` follows (cast.go:39);
`conditions/member.go` `member()` returns `combat.Member`. Widen
`GetEffectiveAC` (combatant.go:125) and `combat.IsDown` (is_down.go:25)
parameters to `Member` so cast-fed rules keep calling them.

## The aliasing escape and its pin

Cast members are the SAME objects the keepers mutate (attachAll stores the
loaded sheets; cast.go:21-26 says "a view, not a copy"). So
`m.(combat.Combatant)` or `m.(*character.Character)` on a cast member
restores full write access. Today **zero such asserts exist** (the only
Combatant-family asserts are the EffectiveACCalculator widening at
combatant.go:126 and owner-handle asserts, both out of scope).

Pin: an AST test refusing any type assertion that widens a `combat.Member`
to a writer surface, modeled on `TestOnlyTheDoorInstallsGameContext`'s
selector-scan + `TestTheAliasEscapeIsClosed`'s alias/dot-import closure.
Mechanism (which modules it scans, how it types the operand) is the build
agent's to decide with work shown; the ruling is that the pin exists and an
alias cannot defeat it.

## PR split (module boundaries force the order)

Three modules, no replace directives; resolution pins dnd5e v0.113.0,
session pins both.

1. **PR A — dnd5e**: `combat.Member`, Combatant embeds it,
   `gamectx.Cast.Member` + `conditions.member()` return types,
   GetEffectiveAC/IsDown widening, four fake casts, three
   `var _ combat.Combatant` assertions split (Member vs Combatant),
   mock regen, narrowing pin. CI tags dnd5e.
2. **PR B — resolution**: bump dnd5e pin, `castView.Member` returns
   `combat.Member`, pins beside (`TestNoCodePathProducesACastlessInteraction`
   sits adjacent, should not break). CI tags resolution.
3. **PR C — session**: pin bumps only (stack alignment; no code change —
   session never names the type).

## Dispositions

- **F1 — name**: `combat.Member`. **RULED 2026-08-29** ("I agree with both your recommendations").
- **F2 — IsDirty placement**: off Member. **RULED 2026-08-29** (same ruling).
- **F3 — `combat.DealDamage` has NO production caller** (damage.go:167;
  test-only + one integration test). Not Phase 4's business — logged for the
  Phase 6 vestigial sweep next to combat.WithRoom.
- **F4 — `CombatCharacter`** (turn_manager.go:50) embeds Combatant + adds
  ActivateCombatAbility; only fed by TurnManager construction, never a cast.
  Untouched.
- **F5 — rpg-api**: imports neither `combat.Combatant` nor `gamectx` (grepped
  clean). No downstream break.
