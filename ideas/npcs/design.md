# Hostility is a relation — one predicate, five callers

Normative. Reasoning lives in [brainstorm.md](brainstorm.md). The merchant and
`KindWorld` are **not ours** — see [handoff-placement.md](handoff-placement.md).

Parent: **rpg-project#311**. Closes **rpg-toolkit#899** and **rpg-toolkit#766**.

## Why this is our lane

Kirk, 2026-08-28: *"FadedPez does not care about hostility, they just want to
build the merchant. We cared about it because in the dungeon non players are
considered hostile. Hostility and allegiance is in our lane."*

The engine has **no hostility concept** — a grep of `rulebooks`, `play`, `tools`
and `core` finds no faction, allegiance or hostility anywhere. Each site that
needs the answer re-derives it from `MemberKind`, or does not ask at all. That is
already two open bugs.

## The rule

Kirk: *"If we allow all non-monster kinds to not be hostile to us I think we are
good, and we can stop over-engineering here."*

1. Hostility **MUST** be decided in exactly one place.
2. That place **MUST** answer: a pair is hostile when **one side is a monster and
   the other is a player**. Every other pairing is not hostile — monster/monster,
   player/player, and anything involving a non-monster kind.
3. It **MUST NOT** grow a faction model, a disposition axis, or configuration in
   this slice. The rule above is the whole answer for now.

## The shape

```go
// hostile reports whether a and b are enemies right now.
//
// Hostility is a fact about a PAIR, which is why it cannot live on MemberKind:
// a kind describes one member, and no property of one member answers "are these
// two enemies".
//
// A METHOD rather than a free function so the answer can later depend on state
// the pair does not carry -- factions, and what a side BELIEVES
// (rpg-project#305/#306). Today it reads only the two members' kinds.
func (e *Encounter) hostile(a, b MemberID) bool
```

4. It **MUST** be a method on the encounter, **MUST NOT** be reduced to a pure
   function of two members, and that reason **MUST** be in its godoc.
5. It **MUST** be symmetric. Asymmetric hostility — an ambusher hostile to a
   party not yet hostile back — is a later ruling, not an accident.
6. A member that is down **MUST** be handled exactly as today; this predicate
   answers enmity, not standing, and **MUST NOT** absorb the down check.

## The callers

7. These **MUST** be converted to ask it:

| Site | Today | After |
|---|---|---|
| `encounter/trigger.go` `sidesInContactOrder` / `classify` | partitions members into players and monsters, pairs the products | pairs the members `hostile` reports enemies |
| `encounter/standing.go` `fightIsDecided` | counts players and monsters, ends when either reaches zero | ends when no hostile pair remains standing |
| `session/attack.go`, two sites | candidates are `Kind == KindMonster` | candidates are members `hostile` reports enemies |
| `behavior/basic.go` | candidates are `Kind == KindPlayer` | same |
| `combat/movement.go` threat list | **no hostility check at all**; `canMakeOpportunityAttack` returns `true` unconditionally | a departing member only provokes from an enemy |

8. The first four conversions **MUST NOT** change behaviour, and the existing
   suite passing unchanged is the evidence.
9. The fifth **DOES** change behaviour, deliberately: it is the fix for
   rpg-toolkit#899 (*"player OAs their own party member... repeatedly downed the
   wounded character trying to retreat"*) and rpg-toolkit#766. It **MUST** ship
   with tests naming both.
10. No **new** kind is added by this slice. `KindWorld` is
    [handoff-placement.md](handoff-placement.md)'s, and it inherits a correct
    answer from rule 2 the moment it exists — a **stated fact**, not an omission.

## Out of scope

Factions; disposition; intel-driven allegiance; asymmetric hostility; `KindWorld`;
the merchant; placement; the builder; capabilities; the interaction verb;
adjacency; anything on the wire — what a client may know about who hates whom is
a perception question and is not answered here.

## Acceptance

- `hostile` is the only place the engine decides enmity; all five sites ask it.
- The existing suite passes unchanged, demonstrating the first four conversions
  are behaviour-preserving.
- A player adjacent to a moving ally does **not** opportunity-attack them
  (rpg-toolkit#899).
- A monster adjacent to a moving monster does **not** opportunity-attack it
  (rpg-toolkit#899, #766).
- A player still opportunity-attacks a departing monster, and a monster a
  departing player.
- Two monsters in sight of each other form no bubble; a fight still forms and
  still ends exactly as before.
- A member of any non-monster kind is reported hostile to nobody.
