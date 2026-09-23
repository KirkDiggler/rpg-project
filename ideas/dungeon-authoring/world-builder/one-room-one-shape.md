# The v2 dialect is legacy — one room, one shape

**Status:** DESIGNED 2026-09-24, for ruling. Written to make a foundation
decision before more is built on it. Records a measurement, two rulings that
follow from it, and the pruning they permit.

## The one sentence

**There are two authored shapes for a room and only one of them has a user:
v4's `monsters:` + `monsterBindings` is the shape, and v2's flat `place[]` —
which mixes prop fields and monster fields on one object — is legacy surface to
remove rather than carry.**

## How this was found

Kirk, 2026-09-24, reading the new named-tables design:

> "we have props that have one set of properties. we have monsters which have
> their own set of properties. we set these properties on the slice that says
> where things are placed. seems like we should have propBindings with prop
> properties, monsterBindings with the monster properties. placement can have
> simple placement properties. at this hex, facing this way"

That is not a new proposal. **It is the v4 design**, already shipped — and the
confusion that produced it is the finding: two dialects are alive at once and
they disagree about where a creature's facts go.

## What is actually true today (measured 2026-09-24)

### The builder writes v4

```
room.room.monsters[].{id, ref, cell, faction}      WHERE it stands, whose side
room.room.monsterBindings.<id>.{on, temper,        WHAT it does  (six keys)
                                actions, holds,
                                intimidate, persuade,
                                arrives}
room.room.propDeclarations.<id>                     what a prop IS (its shape)
room.room.propBindings.<id>.{holdable, holds,       a prop's own state
                             arrives}
```

`monsterBindings` is *not* the four keys the wiring comment lists — it is
**six**, and `on`, `temper` and `actions` are already there
(`single_room_site.go` `monsterBindingsShape`). A creature's behaviour table
already lives in its binding.

### v2 is the flat shape, and nothing live uses it

`Load` routes by version (`compile.go`):

```go
if err == nil && version.Version >= 3 {
    decoded, err := DecodeSingleRoom(...)
    return CompileSingleRoom(...)
}
spec, err := Decode(raw)   // v2, the legacy path
```

Every authored room in the tree is **v2 or v3/v4**, and the v3+ ones take the
single-room path. Concretely:

| Where | v2 files | v3/v4 files |
|---|---|---|
| rpg-api `content/` (shipped) | 5 | 0 |
| rpg-api `internal/dungeons/testdata/` | 0 | 3 |
| rpg-toolkit `dungeonspec/testdata/` | 5 | 8 |

**The five shipped v2 files are the only production users, and Kirk has ruled
they are disposable:** *"we do not need to migrate anything. we only have my
example dungeon which I will use to understand the new shape"* — and, on the
test rooms, *"we have way too many test rooms now. we only need the reference
tomb."*

So `place[]`'s 18 flat keys, prop fields and monster fields side by side, are
**surface with no customer.** Measured in `validate.go`, **ten refusal rules**
exist purely to keep the two vocabularies apart on one object — four refusing
prop fields on monsters (`holdable`, `blocks_movement`, `blocks_los`, `offset`),
six refusing monster fields on props (`targeting`, `actions`, `intimidate`,
`persuade`, `on`, `boss`). v4 needs none of them: a prop's fields are on a prop's
declaration and a monster's are on a monster's binding, so the crossing cannot
be written.

**That is the shaky foundation.** Not the bindings — the older flat object they
were introduced to replace, still shipped and still tested.

## Ruling 1 — `intimidate:` and `persuade:` come off

Kirk, 2026-09-24: *"i think we can call intimidate and persuade off for now. it
doesnt really belong on a monster and when it does show up it will come with
more than those 2."*

**Agreed, and the code already half-says so.** `RoomMonsterBinding.Holds`'s own
comment states the rule the two keys violate:

> **ORDERS, NOT IDENTITY**, which is why it is here and not on the `monsters:`
> entry (rule 1).

`intimidate`/`persuade` are a **social offer on a combatant's orders block** —
two check specs, with approach lists and DCs — for a capability Kirk has already
ruled combatants do not have:

> "Combatants cannot be intimidated or persuaded. That is for neutral factions
> or members… we can postpone those for hostile monsters."

The only fixture that uses them is the front-room goblin, which is a **neutral
NPC**, not a combatant.

**The second sentence is the stronger argument.** When social arrives it will
not be two scalar DC fields; rpg-project#494 already ruled *"the social offer
comes from the NPC."* Keeping the current shape means committing to a
placeholder that the real design replaces — and carrying it in two dialects, in
content, and in the builder, for a capability nothing can reach.

**Disposition:** removed from `RoomMonsterBinding` and `PlaceSpec`, **refused by
name with where they went** rather than silently dropped — `laterWords`'
courtesy, not a bare unknown key.

## Ruling 2 — v2's flat `place[]` is removed, not migrated

Kirk: *"we do not need to migrate anything."*

**Disposition:** the v2 dialect is deleted once the reference tomb is v4. Its
five shipped content files go with it, and the test rooms are pruned to the
reference tomb.

This is the "give the lower layer the primitive it lacks" fork taken the other
way: here the primitive **exists** (v4) and the legacy shape is what should go.
Migrating v2 would preserve a shape nothing needs and keep every crossing
refusal alive.

## What this permits: prune the test rooms

Kirk: *"we have way too many test rooms now. we only need the reference tomb."*

Measured, the toolkit carries **13** rooms under `dungeonspec/testdata/` and
rpg-api carries **8** more across `content/` and `internal/dungeons/testdata/`.
That is 21 rooms for one engine. The count is itself the symptom: rooms
accumulated per slice, each with its own goldens, and **the set is now larger
than what a reader can hold**.

The pruning rule this doc proposes, for ruling: **one room per dialect-era that
still ships.** The reference tomb is the v4 room; the reference tomb's atlas
golden is its record. Every other room is either (a) a case that belongs as a
unit test over a small fixture, or (b) already covered by the tomb.

Not decided here and deliberately: the exact per-file disposition of the 21.
That is mechanical once the two rulings land, and doing it in this doc would
bury the ruling under an inventory.

## What this does NOT decide

- **Where a named behaviour table's reference lives.** It lands in
  `monsterBindings.<id>` (`table: <id>`), which is consistent with `on` already
  being there — but the root key's name is still open
  ([`named-behavior-tables.md`](./named-behavior-tables.md)).
- **`place[].facing` on a monster** (rpg-toolkit#1892). In v4 the natural home
  is the `monsters:` entry beside `cell` — the placement half, exactly Kirk's
  "at this hex, facing this way". That ruling is #1892's and this doc does not
  make it.
- **Whether v3 keeps working.** v3 takes the single-room path already, so it is
  not v2's problem; whether it survives is its own question.
- **The asset ref.** Same binding seam (`monsterBindings.<id>.asset`), same open
  naming question.

## The order this implies

1. **Rule on this doc** — the two dispositions and the pruning rule.
2. **v2 removal** — one toolkit PR: delete the dialect, its refusals, its tests
   and its five content files; prune the test rooms to the reference tomb.
3. **`intimidate`/`persuade` refusal** — may ride with (2) or land separately,
   since both touch the same binding and the same goldens.
4. **Then** the named table (rpg-project#498) and web#1199 land on the shape
   that stays.

Doing 4 before 2 means writing new keys into a shape that is about to lose a
dialect, and #1199's scope work would be authored into `place[]` in tests that
are being deleted.

## Why this is worth the churn now

The lens asks what a shortcut makes permanently impossible. Here the answer runs
the other way: **keeping v2 makes the shape permanently ambiguous.** Every new
key must be added in two places, every crossing refusal must be maintained, and
every reader must learn which of two objects a creature's facts live on. The
churn is bounded — five content files and a test room set — and Kirk has ruled
those disposable. That is what makes this cheap now and expensive later.

— rpg-toolkit agent, on behalf of KirkDiggler