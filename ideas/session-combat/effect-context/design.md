# Effect Context — how an effect reads the world and writes itself back (v1)

**Status:** Design — one open decision for Kirk (see [Open decisions](#open-decisions)). Brainstorm and
the parked territory are in [brainstorm.md](./brainstorm.md).
**Journey:** rpg-project#253 · **Umbrella:** `ideas/session-combat/`
**Scope ruling (Kirk, 2026-08-26):** *"Right now. we need the functionality of the game context where
conditions can look up the state of the world and Mark things dirty when they're dirty."*

---

## Context — what exists today

Verified 2026-08-26 against `rpg-toolkit` `origin/main` @ `5b54360`.

**Conditions cannot read anything but geometry.** `gamectx` defines five installers; exactly one is
ever installed. `gamectx.WithGameContext` and `NewGameContext` have **zero non-test call sites**.
The toolkit says so about itself at `conditions/fighting_style_protection.go:150` — *"a live
registry the session stack never installs."* A sixth mechanism, `combat.WithCombatantLookup`, lives
in a different package with its own context key and is likewise never installed.

**Three conditions are dead in play because of it**, and they fail loudly into a fold that swallows
the error:

| condition | reads | site |
|---|---|---|
| Unarmored Defense | own ability scores | `conditions/unarmored_defense.go:191` |
| Martial Arts | own ability scores | `conditions/martial_arts.go:144`, `:257` |
| Unarmored Movement | own shield state | `conditions/unarmored_movement.go:128` |

Each does `registry, err := gamectx.RequireCharacters(ctx)` → `return c, err`.
`Character.EffectiveAC` swallows it at `character/character.go:1398-1405` (`if err == nil`), so AC
falls back to base. An errored fold drops **every** AC contributor, not just the one that failed.

**Two more are wrong or missing for a related reason.** `conditions/sneak_attack.go:257` uses
`entity type == "character"` as its ally proxy. `monstertraits/pack_tactics.go:128` is commented out
entirely.

**Nothing can mark a sheet dirty from inside an effect.** `resolution/resolve.go:496` returns only
participants where `IsDirty()`. There is no `MarkDirty`/`SetDirty` path reachable from `conditions/`
or `events/`. `sneak_attack.go:214` sets `s.UsedThisTurn = true` and nothing records that the sheet
changed; today it survives only because the attacker also paid an action, which does set dirty.

**What already works, and is the model to copy.** `gamectx.WithRoom` is installed on every path
(`resolution/resolve.go:335`), pinned structurally by `TestNoCodePathProducesARoomlessInteraction`,
and read defensively — `prone.go:284` returns `(within, known bool)` so that *"not within reach"*
and *"nobody knows where these two are standing"* stay distinguishable.

---

## The spine: two channels, and only one of them is ambient

- **Read the world and the cast → context.** Not owned by any participant. One installer, always
  present.
- **Read and write myself → a direct handle.** Owned by exactly one participant, and the only
  channel that can mutate.

This resolves the standing contradiction between `resolution/doc.go` (resolution populates game
context) and `events/events.go:122` (OwnerAware replaces the registry). They were never in conflict
— they answer different radii, and each got its half right.

---

## What we build

### 1. The owner handle — own-sheet reads, and dirty

Extend the existing `OwnerAware` opt-in (`events/events.go:138`), already proven by
`FightingStyleProtectionCondition`, which type-asserts its own narrow structural interface
(`conditions/fighting_style_protection.go:36-39`). Each condition declares what it needs:

```go
// conditions/unarmored_defense.go
type unarmoredDefenseOwner interface {
    AbilityScores() shared.AbilityScores
}
```

`*Character` already satisfies every one of these — `AbilityScores()` at `character/character.go:175`,
`HasShieldEquipped()` at `:925`. No new accessors required for the three broken conditions.

Marking dirty rides the same handle. The mechanism depends on
[the open decision](#open-decisions) below.

**Why not an instruction event.** Marking dirty is persistence bookkeeping, not a rules event. It
has no meaning to any other listener and would need a bus for something with no game semantics.

### 2. `Cast` — one installer, mandatory and singular

Resolution already holds every participant (R3 — *"pass everyone in"*) and already attaches every
effect (R5 — *"tears down every subscription it granted"*). It installs the cast on the same path
that installs the room, every time, with no conditional.

`gamectx` stays the home. It already imports `combat` (`combatant.go`, `characters.go:165`) and
`combat` does not import it, so there is no cycle. Resolution implements the interface; conditions
and monster traits read it.

### 3. It exposes questions, not fields

This is the whole corner-avoidance and the part most worth getting right:

```go
// gamectx
func WithCast(ctx context.Context, c Cast) context.Context
func CastOf(ctx context.Context) (Cast, bool)

type Cast interface {
    // Member returns a participant's combat-facing sheet.
    Member(id string) (combat.Combatant, bool)

    // Members returns every participant, in deterministic order (R4).
    Members() []string

    // IsHostile answers whether b is an enemy of a, right now.
    IsHostile(a, b string) (hostile, known bool)
}
```

`IsHostile` is implemented in v1 as *"different `MemberKind`"*. **That is still a lie** — but it is
a lie in **one function** instead of smeared across four rules. When allegiance arrives, that one
function reads a stance table and every predicate is already correct.

If instead we exposed `Member(id).Faction` and let rules compare, we would hardcode the two-sided
assumption into every predicate again, which is precisely the mistake `sneak_attack.go:257` already
made once.

**Every answer is `(value, known)`.** *"Not hostile"* and *"cannot tell"* stay distinguishable —
prone's discipline, and the one that would have prevented the AC collapse.

`Cast` speaks `combat.Combatant`, never `*Character`. `gamectx.CharacterRegistry` is
character-shaped (`GetCharacterWeapons`, `GetCharacterAbilityScores`, `GetCharacterActionEconomy`)
and structurally cannot serve monsters — which disqualifies keeping it.

### 4. Delete the dead installers

| delete | why |
|---|---|
| `gamectx.GameContext` / `CharacterRegistry` / `WithGameContext` | 0 installs; character-shaped; superseded by owner handle + `Cast` |
| `gamectx.CombatantRegistry` / `WithCombatants` | 0 installs, 0 readers |
| `gamectx.CombatState` / `WithCombatState` | 0 installs, 0 readers |
| `gamectx.ReactionReadiness` / `WithReactionReadiness` | 0 installs, 0 readers |
| `combat.CombatantLookup` / `WithCombatantLookup` | 0 installs, 0 readers; duplicate mechanism in a second package |

`gamectx` goes from a bag of six to two things that are always there: the room and the cast.

---

## What this fixes, concretely

| today | after |
|---|---|
| Unarmored Defense inert → barbarian fights at AC 11 | reads its own scores; AC 14 |
| Martial Arts inert → monk damage die and bonus unarmed strike never apply | reads its own scores |
| Unarmored Movement inert | reads its own shield state |
| Any condition erroring poisons the whole AC fold | `(value, known)`; missing data is never an error |
| Sneak Attack treats every character as an ally | asks `IsHostile` from the **target's** perspective, per RAW |
| Pack Tactics commented out | writable — the wolf's own trait finally works |
| Condition state changes silently discarded | dirty path exists; the turn clock becomes buildable |

---

## Open decisions

### D1 — How does dirty get marked?

**(a) By discipline.** The condition calls `owner.MarkDirty()` when it mutates itself. Cheap,
matches the existing dirty flags (`character/character.go:723`, `:1149`, `action_economy.go:48`),
and every future condition author has to remember.

**(b) By construction.** Resolution snapshots each participant's `ToData()` before the interaction
and compares after. Nobody marks anything; forgetting becomes impossible. Costs two serializations
per participant per interaction and requires the encoding to be deterministic.

**Recommendation: (b).** "Forgot to mark dirty" is a silent data-loss bug, and this is the same
class of move as making the room mandatory — a structural guarantee instead of a convention, in the
spirit of `TestNoCodePathProducesARoomlessInteraction` and R7. The cost is bounded (a handful of
participants per interaction) and the flag can stay as-is underneath.

**Risk to check before committing to (b):** `ToData()` must be byte-stable for unchanged state.
Map-keyed fields are the thing to verify — Go's `encoding/json` sorts map keys, but any slice built
from a map, or any timestamp/UUID minted during serialization, would make every participant look
dirty forever.

---

## Non-goals (v1)

- **No allegiance data, no stance table, no factions on the wire.** `IsHostile` returns the
  Kind-based answer. The seam exists so the truth can arrive later without touching a rule.
- **No world state, quest flags, or campaign scope.** Dungeon-scoped, per Kirk's ruling.
- **No turn/rest clock.** Dirty is its prerequisite; the clock is the next slice.
- **No feature activation.** `Verb` stays attack/move/end_turn. Separate slice.
- **No new conditions.** The 13 missing standard conditions are their own work.
- **No `HasCondition` on `Cast`.** Nothing reads it yet — the same rule `doc.go` states about not
  populating a registry nothing reads. It arrives with the first predicate that needs it, which is
  most likely "attacking a paralyzed creature."
- **No reactions or suspension.** `Cast` is a deterministic projection of `Participants`, so R4
  stays satisfiable.

---

## Sequencing

Toolkit-only. Nothing in this slice reaches protos, rpg-api, or web — no wire change, no client
change.

1. **`Cast` + mandatory install + delete the five dead installers.** One PR. Structural test that
   no code path produces a castless interaction, mirroring the roomless one.
2. **Owner handle for the three broken conditions.** Unarmored Defense, Martial Arts, Unarmored
   Movement move off `RequireCharacters` onto their own narrow owner interfaces, returning
   `(value, known)` rather than errors.
3. **Dirty**, per D1.
4. **Sneak Attack onto `IsHostile`; Pack Tactics implemented.** Proves the seam with two real
   consumers — one character rule, one monster trait.

Steps 1 and 2 are independent and can run in parallel; 3 depends on nothing but should land before
the clock slice; 4 depends on 1.

## Evidence

Engine and rules invariants, so full rigor:

- A barbarian with Unarmored Defense resolves a real strike at 10+DEX+CON, **measured through
  `resolution.Resolve`**, not through a unit test on the condition. The bug this slice fixes was
  invisible to every existing test because they all installed a registry production never does.
- A condition returning "unknown" leaves other AC contributors intact — the fold is not poisoned.
- No code path produces an interaction without a cast (structural, not by example).
- A condition mutating only its own state comes back from `Resolve` in `DirtyCharacters`.
- Sneak Attack fires when an enemy **of the target** is adjacent, and does not fire when the
  adjacent creature is an ally of the target. Under a Kind-based `IsHostile` these are still the
  two-faction answers — the test pins the *question being asked*, so it keeps passing when
  allegiance lands.
