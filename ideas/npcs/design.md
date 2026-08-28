# NPCs — design

Normative. Reasoning and rejected options live in [brainstorm.md](brainstorm.md).

Parent: **rpg-project#311** (World NPC Foundation). Sibling, layering on top:
**rpg-project#310** (Buy-Only Vendor Inventory) and
[rpg-toolkit#1275](https://github.com/KirkDiggler/rpg-toolkit/issues/1275).

## Scope

A third `MemberKind` — `npc` — for members who are **not a side**: present on the
map, visible, never hostile, never attackable, ignored by monster AI, taking no
turn, and able to report what it can be interacted with about.

Its first use case is a merchant standing in a region authored before the
dungeon's `start`, reached through a door, shopped in free roam.

## The law this rests on

Every decision in the hostile path is an **allow-list keyed on `MemberKind`,
never a deny-list**. A third value is therefore excluded from each by
construction:

| Behaviour | Decided in | Mechanism |
|---|---|---|
| Starts no fight on sight | `encounter/trigger.go` `sidesInContactOrder` | Returns members in neither `players` nor `monsters`; a member on no side is in no pair, and a pair is the only thing that forms or joins a bubble. |
| Does not hold a fight open | `encounter/standing.go` `fightIsDecided` | Counts players and monsters only; returns `players == 0 \|\| monsters == 0`. |
| Not attackable by players | `session/attack.go`, two sites | Candidates built from `Kind == KindMonster`. |
| Ignored by monster AI | `behavior/basic.go` | `if sm.Kind != encounter.KindPlayer \|\| !sm.Standing { continue }`. |

None of these four is edited by this design. Rule 7 exists to prove that claim
rather than assume it.

## 1. The kind — `rpg-toolkit/rulebooks/dnd5e/encounter`

```go
// KindNPC is a member who is on no side: present and visible, never hostile,
// never a target, never holding a turn.
KindNPC MemberKind = "npc"
```

1. An NPC **MUST NOT** form or join a combat bubble.
2. An NPC **MUST NOT** count toward whether a fight is decided.
3. An NPC **MUST NOT** be a valid attack target for a player.
4. An NPC **MUST NOT** be selected as a target by monster AI.
5. An NPC **MUST NOT** roll initiative or be driven for a turn. It never enters
   a bubble, so `driveMonsterTurns` never reaches it.
6. An NPC **MUST** be visible: it holds a position, appears in sight percepts,
   and is rendered like any other member.
7. An NPC **MUST** occupy its cell the way any member does — so it blocks
   movement as an occupied tile, per rpg-project#311. This is a consequence of
   being a placed member, **not** a declared property; see rule 12.
8. `Kind` **MUST NOT** change at runtime. It is an authored fact for the life of
   the member. Attackable NPCs and any mutation of `Kind` are out of scope.
9. Every remaining `switch member.Kind` and `Kind ==` comparison in `encounter`,
   `session` and `behavior` **MUST** be audited and its behaviour for `npc`
   stated explicitly — including `data.go`'s `Kind == KindPlayer` branch, the
   `Kind` fields on the input/view structs in `field.go`, and `turndriver.go`.
   An arm that is correct by falling through **MUST** say so in a comment;
   silence is not evidence.

## 2. Interaction capabilities

rpg-project#311 asks for the shape, reported and not implemented.

10. An NPC **MUST** report a set of interaction capabilities drawn from a sealed
    set: `TALK`, `VENDOR`, `TRAINER`, `QUEST_GIVER`, `QUEST_TARGET`.
11. This slice **MUST NOT** implement behaviour behind any of them. Reporting is
    the whole deliverable; the vendor capability gains behaviour in
    rpg-project#310 / rpg-toolkit#1275.

## 3. The placement — `encounter/dungeonspec`

`refKind` accepts exactly `props` and `monsters` and refuses everything else by
name. It gains a third type segment.

12. A ref of type `npcs` **MUST** compile to a member with `Kind == KindNPC`.
13. An NPC placement **MUST** accept `facing`, one of the eight true-compass
    names, validated as props' is. This is the one place NPCs follow props
    rather than monsters, and the reason is stated: a monster is refused facing
    because it turns in play, and an NPC never does.
14. An NPC placement **MUST** be refused `blocks_movement`, `blocks_los`,
    `targeting`, `boss` and `offset`, each by name. Refusing `blocks_movement`
    does not contradict rule 7: that field is a **prop's declaration** about a
    thing standing on the floor, and a member's occupancy is not declared.
15. One-placement-per-cell already holds and is unchanged.

## 4. The wire — `rpg-api-protos`

`Member.kind` (field 2) already carries `MemberKind`. The change is additive.

16. `MEMBER_KIND_NPC = 3` **MUST** be added to the existing enum. No new field,
    no new message, no reserved tag.
17. An NPC's interaction capabilities **MUST** reach the client, as a repeated
    sealed enum.
18. The proto **MUST** merge before toolkit, api, or web build against it.

## 5. The room — authoring only

19. The vestibule **MUST** be a region of the same dungeon spec. No second zone.
20. `start` **MUST** be a cell inside the vestibule region.
21. A `DoorSpec` **MUST** separate the vestibule from the dungeon's first
    chamber. Opening it refreshes sight, which forms a bubble with anything
    hostile beyond it — existing behaviour, deliberately relied on.
22. Whether that door is locked is an authoring choice and **MUST NOT** be
    encoded as a rule.

## 6. Panel

23. The client **MUST** distinguish an NPC from a monster and **MUST NOT** offer
    Attack against one. The panel reads `Member.kind`; it does not infer.

## Open question

rpg-project#311 says an NPC is interacted with by an **adjacent** player. The
existing door `Interact` has no adjacency check, so adjacency is either a new
rule for NPC interaction or a shared correction to both. **Needs a ruling before
the interaction verb is built**; it does not block the kind, the placement, or
the wire.

## Out of scope

Attackable NPCs and a mutable `Kind`; vendor stock, wallets, buying, selling,
barter, reputation pricing; dialogue trees; quest logic; trainer services; NPC
AI, turns, damage, death or drops; a disposition axis.

## Acceptance

- A dungeon authored with an `npcs` placement compiles, and the NPC appears at
  its authored cell and facing.
- A player walks into line of sight of the NPC and **no bubble forms**.
- A player in a running fight cannot name the NPC as an attack target.
- A monster in a running fight never selects the NPC as its target.
- A fight in which every monster is downed ends, with the NPC still standing.
- The NPC takes no turn and never appears in an initiative order.
- A player cannot walk through the NPC's cell.
- `Member.kind` reaches the client as `MEMBER_KIND_NPC` with its capability set,
  and the panel offers no Attack against it.
- The reference dungeon gains a vestibule holding `start`, a door into the first
  chamber, and a merchant; a party walks from the vestibule through the door and
  the first fight forms on the far side.
