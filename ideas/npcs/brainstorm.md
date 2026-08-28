# NPCs — the third kind, and the room before the dungeon

The WHY. Normative shapes live in [design.md](design.md).

## The ask

Kirk, 2026-08-28: a room *before* the main dungeon start, entered through a real
door, with a merchant standing in it. Not a second zone — a region of the same
dungeon. The merchant is an NPC and **not hostile**. It happens in free roam.
The destination is [rpg-toolkit#1275](https://github.com/KirkDiggler/rpg-toolkit/issues/1275),
the vendor/NPC inventory foundation.

Two motivations were bundled at the start and came apart under reading:

- *"We cannot set the type on the regions — we had entrance, chamber, corridor."*
- *"I was thinking the boss room needed something."*

Both dissolved. See **Rejected** below.

## What we found

`MemberKind` is a two-value enum — `KindPlayer`, `KindMonster` — and every place
that decides hostile behaviour switches on it **exhaustively, with no default
arm**. A third value therefore inherits the right answer in three separate
places without any of them being edited:

| Behaviour | Where it is decided | Why a third kind is already correct |
|---|---|---|
| Starts no fight on sight | `encounter/trigger.go` `sidesInContactOrder` | Partitions standing members into `players` and `monsters`. Its own doc states the law: *"a member who is not on a side is in no pair, and a pair is the only thing that forms or joins a bubble."* |
| Does not hold a fight open | `encounter/standing.go` `fightIsDecided` | Counts players and monsters, returns `players == 0 \|\| monsters == 0`. An uncounted kind cannot keep a decided fight running. |
| Cannot be attacked | `session/attack.go` (two sites) | The candidate target set is built with `if member.Kind == KindMonster`. A non-monster is never a candidate; naming one returns `ErrStaleDeclaration`. |

This is the strongest argument that the seam is right: we are not adding a
concept, we are **adding the value the existing law was already written to
tolerate**. The cost is an audit, not a redesign.

## Rejected

**Region typing (entrance / chamber / corridor).** `RegionSpec` v2 carries
`ID`, `Name` (carried, never read), `Archetype`, `Lighting`, `Cells`. Region
kinds were removed deliberately: v1 *derived* `start` from an archetype, and the
spec comment names that as "the shape of defaulting rpg-toolkit#1033 forbids."
`Archetype` is the surviving label and is ruled **presentation only, never a
mechanic** (rpg-project#256). Kirk withdrew the ask once the boss case dissolved.

**A region type for the boss room.** Not needed — `PlaceSpec.Boss` already
exists and `validate.go` enforces one boss per region ("region %q already names
%q as its boss"). The boss is a placement fact, not an area fact.

**A hostility / faction / disposition model.** This is the wrong axis. Kirk:
*"if it was ally, hostile then neutral makes sense — monster, player and npc seem
right to me."* The enum answers **what a member is**, not how it feels about you.
Disposition is a second axis that would need its own use case; nothing has one.

**Merchant placed as a monster.** `classify` loops `players × monsters`, so a
merchant carrying `KindMonster` forms a bubble the instant anybody sees him.
This is why the third kind is required rather than merely tidy.

**A separate zone / scene for the shop.** The whole point of putting the room
inside the dungeon spec is avoiding a second world concept, its load path, and
its camera rules. One compile, one map.

## Rulings (Kirk, 2026-08-28)

1. **The merchant cannot be attacked.** *"That may come, but is definitely not a
   launch goal and probably not needed."* `Kind` therefore stays a fixed authored
   fact and never mutates in play — which is what keeps this slice small.
2. **The kinds are `player`, `monster`, `npc`.**
3. **NPCs carry an authored facing.** *"They have no behavior so will need a
   facing."* Monsters are refused facing because they turn dynamically in play;
   an NPC never does, so the authored value is the only one there will ever be.
4. **`bystander` is a role *within* npc**, alongside `merchant` — *"that is
   something else, we don't have a use case for yet."*

## Shelves left empty

Named attachment points with nothing on them, per the standing rule that the
shelf matters and its contents do not until a real case arrives:

- **NPC role** (`merchant`, `bystander`, quest-giver). The kind says *not a
  side*; the role would say *what this one is for*. Nothing reads a role today.
- **Attackable NPCs**, and with them a mutable `Kind` — deliberately deferred.
- **Disposition**, if a member ever needs to change sides.
- **Interaction beyond the shop** — the verb seam is shared, the vendor is its
  first caller.
- **An authored `offset` on an NPC.** design.md refuses it, matching monsters,
  because refusing is the reversible default: allowing it later is additive,
  withdrawing it is not. If authoring wants a merchant nudged behind a counter,
  that is a ruling to make then, with the case in hand.

## Open, and not blocking

Whether the door into the dungeon proper is **locked** or merely closed is an
authoring choice, not a code one — locked-with-a-DC already exists. A locked
front door makes the vestibule a deliberate threshold rather than a hallway.
