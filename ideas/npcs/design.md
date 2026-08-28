# NPCs — design

Normative. Reasoning and rejected options live in [brainstorm.md](brainstorm.md).

## Scope

A third `MemberKind` — `npc` — for members who are **not a side**: present on the
map, visible, never hostile, never attackable, taking no turn. Its first use case
is a merchant standing in a region authored before the dungeon's `start`, reached
through a door, shopped in free roam against
[rpg-toolkit#1275](https://github.com/KirkDiggler/rpg-toolkit/issues/1275).

## 1. The kind — `rpg-toolkit/rulebooks/dnd5e/encounter`

`MemberKind` gains one value beside `KindPlayer` and `KindMonster`.

```go
// KindNPC is a member who is on no side: present and visible, never hostile,
// never a target, never holding a turn.
KindNPC MemberKind = "npc"
```

1. An NPC **MUST NOT** form or join a combat bubble. Satisfied by
   `sidesInContactOrder` returning it in neither slice; no edit required.
2. An NPC **MUST NOT** count toward whether a fight is decided. Satisfied by
   `fightIsDecided` counting only players and monsters; no edit required.
3. An NPC **MUST NOT** be a valid attack target. Satisfied by `attack.go`
   building candidates from `Kind == KindMonster`; no edit required.
4. An NPC **MUST NOT** be driven for a turn. It never enters a bubble, so
   `driveMonsterTurns` never reaches it.
5. An NPC **MUST** be visible: it holds a position, appears in sight percepts,
   and is rendered like any other member.
6. `Kind` **MUST NOT** change at runtime. It is an authored fact for the life of
   the member.
7. Every remaining `switch member.Kind` and `Kind ==` comparison in `encounter`
   and `session` **MUST** be audited and its behaviour for `npc` stated
   explicitly — including `data.go`'s `Kind == KindPlayer` branch, the `Kind`
   fields on the input/view structs in `field.go`, and `turndriver.go`. An arm
   that is correct by falling through **MUST** say so in a comment; silence is
   not evidence.

## 2. The placement — `rulebooks/dnd5e/encounter/dungeonspec`

`refKind` accepts exactly `props` and `monsters` and refuses everything else by
name. It gains a third type segment.

8. A ref of type `npcs` **MUST** compile to a member with `Kind == KindNPC`.
9. An NPC placement **MUST** accept `facing`, one of the eight true-compass
   names, with the same validation props use. This is the one place NPCs follow
   props rather than monsters, and the reason is stated: a monster is refused
   facing because it turns in play, and an NPC never does.
10. An NPC placement **MUST** be refused `blocks_movement`, `blocks_los`,
    `targeting`, and `boss`, each by name.
11. `offset` **MUST** be refused, matching monsters — an NPC occupies its cell.
12. One-placement-per-cell already holds and is unchanged.

## 3. The wire — `rpg-api-protos`

`Member.kind` (field 2) already carries `MemberKind`. The change is additive.

13. `MEMBER_KIND_NPC = 3` **MUST** be added to the existing enum. No new field,
    no new message, no reserved tag.
14. The proto **MUST** merge before toolkit, api, or web build against it.

## 4. The room — authoring only

15. The vestibule **MUST** be a region of the same dungeon spec. No second zone.
16. `start` **MUST** be a cell inside the vestibule region.
17. A `DoorSpec` **MUST** separate the vestibule from the dungeon's first
    chamber. Opening it refreshes sight, which forms a bubble with anything
    hostile beyond it — existing behaviour, deliberately relied on.
18. Whether that door is locked is an authoring choice and **MUST NOT** be
    encoded as a rule.

## 5. Panel

19. The client **MUST** distinguish an NPC from a monster and **MUST NOT** offer
    Attack against one. The panel reads `Member.kind`; it does not infer.

## Out of scope

Attackable NPCs and a mutable `Kind`; NPC roles (`merchant`, `bystander`);
disposition as a second axis; selling, barter, purse limits, haggling; NPC
behaviour or scheduling. The vendor model itself is rpg-toolkit#1275 and is
designed there, not here.

## Acceptance

- A dungeon authored with an `npcs` placement compiles, and the NPC appears on
  the map at its authored cell and facing.
- A player walks into line of sight of the NPC and **no bubble forms**.
- A player in a running fight cannot name the NPC as an attack target.
- A fight in which every monster is downed ends, with the NPC still standing.
- The NPC takes no turn and never appears in an initiative order.
- `Member.kind` reaches the client as `MEMBER_KIND_NPC`, and the panel offers no
  Attack against it.
- The reference dungeon gains a vestibule holding `start`, a door into the first
  chamber, and a merchant; a party walks from the vestibule through the door and
  the first fight forms on the far side.
