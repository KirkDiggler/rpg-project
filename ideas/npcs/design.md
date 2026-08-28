# NPCs — the placement seam

Normative. Reasoning and rejected options live in [brainstorm.md](brainstorm.md).

Parent: **rpg-project#311** (World NPC Foundation).

## Scope

**This slice makes a space for an NPC to be placed, and nothing else.** A NPC can
be authored into a dungeon in the builder, compiles into the encounter as a
member who is on no side, and renders on the map.

**It gives the NPC no actions.** Interaction capabilities, the interaction verb,
adjacency, and everything the merchant can actually *do* are **FadedPez's work**
(Kirk, 2026-08-28), layered on this seam via rpg-project#310 and
[rpg-toolkit#1275](https://github.com/KirkDiggler/rpg-toolkit/issues/1275). This
document deliberately does not design them — see **Handoff** below.

## The law this rests on

Every decision in the hostile path is an **allow-list keyed on `MemberKind`,
never a deny-list**. Nothing asks "is this member harmless?" — each site asks "is
this a player?" or "is this a monster?" and ignores everything else. A third
value is therefore excluded from each by construction, with **none of these four
sites edited**:

| Behaviour | Decided in | Mechanism |
|---|---|---|
| Starts no fight on sight | `encounter/trigger.go` `sidesInContactOrder` | A member on no side is in no pair, and a pair is the only thing that forms or joins a bubble. |
| Does not hold a fight open | `encounter/standing.go` `fightIsDecided` | Returns `players == 0 \|\| monsters == 0`. |
| Not attackable by players | `session/attack.go`, two sites | Candidates built from `Kind == KindMonster`. |
| Ignored by monster AI | `behavior/basic.go` | `if sm.Kind != encounter.KindPlayer \|\| !sm.Standing { continue }`. |

Rule 6 exists to prove that claim rather than assume it.

## 1. The kind — `rulebooks/dnd5e/encounter`

```go
// KindNPC is a member who is on no side: present and visible, never hostile,
// never a target, never holding a turn.
KindNPC MemberKind = "npc"
```

1. An NPC **MUST NOT** form or join a combat bubble, count toward whether a
   fight is decided, be a player's attack target, or be selected by monster AI.
2. An NPC **MUST NOT** roll initiative or be driven for a turn.
3. An NPC **MUST** be visible: it holds a position, appears in sight percepts,
   and is rendered like any other member.
4. An NPC **MUST** occupy its cell as any member does, so it blocks movement as
   an occupied tile (rpg-project#311). A consequence of membership, **not** a
   declared property — see rule 11.
5. `Kind` **MUST NOT** change at runtime. Attackable NPCs and any mutation of
   `Kind` are out of scope.
6. Every remaining `switch member.Kind` and `Kind ==` comparison in `encounter`,
   `session` and `behavior` **MUST** be audited and its behaviour for `npc`
   stated explicitly — including `data.go`'s `Kind == KindPlayer` branch, the
   `Kind` fields on `field.go`'s input/view structs, and `turndriver.go`. An arm
   correct by falling through **MUST** say so in a comment; silence is not
   evidence.

## 2. The ref — `rulebooks/dnd5e/refs`

7. `module.go` **MUST** gain `TypeNPCs core.Type = "npcs"` beside `TypeMonsters`.
8. At least one NPC ref **MUST** exist for the builder to place. The merchant is
   the first. Naming the ref is not naming its behaviour.

## 3. The placement — `encounter/dungeonspec`

`refKind` accepts exactly `props` and `monsters` and refuses everything else by
name. It gains a third type segment.

9. A ref of type `npcs` **MUST** compile to a member with `Kind == KindNPC`.
10. An NPC placement **MUST** accept `facing`, one of the eight true-compass
    names, validated as props' is. This is the one place NPCs follow props
    rather than monsters: a monster is refused facing because it turns in play,
    and an NPC never does.
11. An NPC placement **MUST** be refused `blocks_movement`, `blocks_los`,
    `targeting`, `boss` and `offset`, each by name. Refusing `blocks_movement`
    does not contradict rule 4: that field is a **prop's declaration**, and a
    member's occupancy is not declared.
12. One-placement-per-cell already holds and is unchanged.

## 4. The wire — `rpg-api-protos`

`Member.kind` (field 2) already carries `MemberKind`. The change is additive.

13. `MEMBER_KIND_NPC = 3` **MUST** be added to the existing enum. No new field,
    no new message, no reserved tag.
14. The proto **MUST** merge before toolkit, api, or web build against it.
15. This slice **MUST NOT** add capabilities to the wire. That is FadedPez's
    contract to shape when the actions exist.

## 5. The builder — `rpg-dnd5e-web/src/author`

The palette applies a **ref-AND-GLB test**: a thing is placeable only if a
toolkit ref *and* a promoted GLB both exist. `paletteData.ts` documents the rule
and excludes refs failing either half.

16. `PaletteCategory` **MUST** gain `npcs`, and the palette **MUST** offer every
    NPC ref that passes the ref-AND-GLB test — verified by resolution, not
    asserted, exactly as the monster and prop vocabularies are.
17. The `place` tool **MUST** write an `npcs` placement, and `dungeonYaml.ts`
    **MUST** round-trip it.
18. The Inspector **MUST** offer `facing` on a selected NPC, and **MUST NOT**
    offer `offset`, `blocks_movement`, `blocks_los` or `boss`.
19. The renderer **MUST** resolve an NPC ref to its model the way
    `monsterModels.ts` resolves a monster's.

## 6. Assets dependency — Assets lane, not this slice

20. A promoted merchant GLB **MUST** exist before the builder can offer one, per
    rule 16. This is filed under the journey with Team **Assets** and **MUST
    NOT** be pulled into this slice.

## 7. The room — authoring only, no code

21. The vestibule **MUST** be a region of the same dungeon spec. No second zone.
22. `start` **MUST** be a cell inside it, and a `DoorSpec` **MUST** separate it
    from the first chamber. Opening that door refreshes sight, which forms a
    bubble with anything hostile beyond — existing behaviour, relied on.
23. Whether the door is locked is authoring, and **MUST NOT** become a rule.

## Handoff — what FadedPez attaches to

This slice's deliverable, stated as the seam rather than as his design:

- A member exists with `Kind == KindNPC`, at an authored cell and facing.
- It is addressable by `MemberID` and reaches the client as
  `MEMBER_KIND_NPC`.
- Nothing in the hostile path acts on it, proven by rule 6's audit.

Everything about what an NPC can be *asked* — capabilities (`TALK`, `VENDOR`,
`TRAINER`, `QUEST_GIVER`, `QUEST_TARGET` are #311's suggestion, not this
document's ruling), the interaction verb, its wire shape, and whether
interaction requires adjacency — is his to design. **Note for that work:** the
existing door `Interact` has no adjacency check, so #311's "adjacent player"
is either a new rule or a shared correction to both.

## Out of scope

NPC actions of any kind; capabilities; the interaction verb; adjacency; vendor
stock, wallets, buying, selling; dialogue; quests; NPC AI, turns, damage or
death; attackable NPCs; a disposition axis; region typing.

## Acceptance

- A dungeon authored with an `npcs` placement compiles, and the NPC appears at
  its authored cell and facing.
- The builder offers the merchant in an `npcs` palette category, places it, and
  the YAML round-trips.
- The Inspector edits its facing and offers no prop-only or monster-only field.
- A player walks into line of sight of it and **no bubble forms**.
- A player in a running fight cannot name it as an attack target.
- A monster in a running fight never selects it as a target.
- A fight in which every monster is downed ends, with the NPC still standing.
- It takes no turn and never appears in an initiative order.
- A player cannot walk through its cell.
- `Member.kind` reaches the client as `MEMBER_KIND_NPC` and it renders.
