# World members — the placement seam

Normative. Reasoning and rejected options live in [brainstorm.md](brainstorm.md).

Parent: **rpg-project#311** (World NPC Foundation).

## Scope

**This slice makes a space for a world member to be placed, and nothing else.**
One can be authored into a dungeon in the builder, compiles into the encounter as
a member who is on no side, and renders on the map.

**It gives it no actions.** Interaction capabilities, the interaction verb,
adjacency, and everything the merchant can actually *do* are **FadedPez's work**
(Kirk, 2026-08-28), layered on this seam via rpg-project#310 and
[rpg-toolkit#1275](https://github.com/KirkDiggler/rpg-toolkit/issues/1275). This
document deliberately does not design them — see **Handoff**.

## Naming (Kirk's ruling, 2026-08-28)

The member kind is **`world`**; the content ref type is **`npcs`**. They differ
on purpose, because they answer different questions:

- **`MemberKind` answers "what is this in a fight?"** `player` belongs to a
  person, `monster` belongs to the fight, `world` belongs to the place. A
  positive definition — which is the point. `npc` defined the kind by negation
  ("not a player"), and monsters satisfy that too, which is how they kept
  sliding into the category.
- **The ref type answers "what content bucket is this?"**, alongside
  `dnd5e:props:pillar` and `dnd5e:monsters:skeleton`. `dnd5e:world:merchant`
  would read badly — "world" is not a category of thing.

The decisive test for the kind name is the **hired mercenary**: an ally who
fights. It is not a player and not a monster, so `npc` would have invited it in
and it would have broken all four behaviours below. `world` refuses it by name —
a mercenary belongs to a side, not to the place.

## The law this rests on

Every decision in the hostile path is an **allow-list keyed on `MemberKind`,
never a deny-list**. Nothing asks "is this member harmless?" — each site asks "is
this a player?" or "is this a monster?" and ignores everything else. A third
value is excluded from each by construction, with **none of these four sites
edited**:

| Behaviour | Decided in | Mechanism |
|---|---|---|
| Starts no fight on sight | `encounter/trigger.go` `sidesInContactOrder` | A member on no side is in no pair, and a pair is the only thing that forms or joins a bubble. |
| Does not hold a fight open | `encounter/standing.go` `fightIsDecided` | Returns `players == 0 \|\| monsters == 0`. |
| Not attackable by players | `session/attack.go`, two sites | Candidates built from `Kind == KindMonster`. |
| Ignored by monster AI | `behavior/basic.go` | `if sm.Kind != encounter.KindPlayer \|\| !sm.Standing { continue }`. |

Rule 7 exists to prove that claim rather than assume it.

## 1. The kind — `rulebooks/dnd5e/encounter`

```go
// KindWorld is a member who belongs to the PLACE rather than to a person or to
// the fight: present and visible, on no side, never hostile, never a target,
// never holding a turn. A merchant, a trainer, a quest-giver.
//
// Named for what it IS, not for what it is not. An ally who FIGHTS — a hired
// mercenary — belongs to a side and is NOT this kind, however friendly.
KindWorld MemberKind = "world"
```

1. The godoc **MUST** carry the belongs-to-the-place gloss and the mercenary
   exclusion. The set `player | monster | world` only coheres when the shared
   axis is stated; without it, `world` reads as "the environment".
2. A world member **MUST NOT** form or join a combat bubble, count toward
   whether a fight is decided, be a player's attack target, or be selected by
   monster AI.
3. It **MUST NOT** roll initiative or be driven for a turn.
4. It **MUST** be visible: it holds a position, appears in sight percepts, and
   is rendered like any other member.
5. It **MUST** occupy its cell as any member does, so it blocks movement as an
   occupied tile (rpg-project#311). A consequence of membership, **not** a
   declared property — see rule 12.
6. `Kind` **MUST NOT** change at runtime. Attackable world members and any
   mutation of `Kind` are out of scope.
7. Every remaining `switch member.Kind` and `Kind ==` comparison in `encounter`,
   `session` and `behavior` **MUST** be audited and its behaviour for `world`
   stated explicitly — including `data.go`'s `Kind == KindPlayer` branch, the
   `Kind` fields on `field.go`'s input/view structs, and `turndriver.go`. An arm
   correct by falling through **MUST** say so in a comment; silence is not
   evidence.

## 2. The ref — `rulebooks/dnd5e/refs`

8. `module.go` **MUST** gain `TypeNPCs core.Type = "npcs"` beside `TypeMonsters`.
9. At least one ref **MUST** exist for the builder to place:
   `dnd5e:npcs:merchant` is the first. Naming the ref is not naming its
   behaviour.

## 3. The placement — `encounter/dungeonspec`

`refKind` accepts exactly `props` and `monsters` and refuses everything else by
name. It gains a third type segment.

10. A ref of type `npcs` **MUST** compile to a member with `Kind == KindWorld`.
    The compiler is the one place the two vocabularies meet, and it **MUST**
    say so in a comment.
11. The placement **MUST** accept `facing`, one of the eight true-compass names,
    validated as props' is. This is the one place it follows props rather than
    monsters: a monster is refused facing because it turns in play, and a world
    member never does.
12. It **MUST** be refused `blocks_movement`, `blocks_los`, `targeting`, `boss`
    and `offset`, each by name. Refusing `blocks_movement` does not contradict
    rule 5: that field is a **prop's declaration**, and a member's occupancy is
    not declared.
13. One-placement-per-cell already holds and is unchanged.

## 4. The wire — `rpg-api-protos`

`Member.kind` (field 2) already carries `MemberKind`. The change is additive.

14. `MEMBER_KIND_WORLD = 3` **MUST** be added to the existing enum. No new field,
    no new message, no reserved tag.
15. The proto **MUST** merge before toolkit, api, or web build against it.
16. This slice **MUST NOT** add capabilities to the wire. That is FadedPez's
    contract to shape when the actions exist.

## 5. The builder — `rpg-dnd5e-web/src/author`

The palette applies a **ref-AND-GLB test**: a thing is placeable only if a
toolkit ref *and* a promoted GLB both exist. `paletteData.ts` documents the rule
and excludes refs failing either half.

17. `PaletteCategory` **MUST** gain `npcs`, matching the ref type, and the
    palette **MUST** offer every `npcs` ref passing the ref-AND-GLB test —
    verified by resolution, not asserted, as the monster and prop vocabularies
    are.
18. The `place` tool **MUST** write an `npcs` placement, and `dungeonYaml.ts`
    **MUST** round-trip it.
19. The Inspector **MUST** offer `facing` on a selection, and **MUST NOT** offer
    `offset`, `blocks_movement`, `blocks_los` or `boss`.
20. The renderer **MUST** resolve an `npcs` ref to its model the way
    `monsterModels.ts` resolves a monster's.

## 6. Assets — Assets lane, not this slice

21. A promoted merchant GLB **MUST** exist before the builder can offer one, per
    rule 17. Filed under the journey with Team **Assets** and **MUST NOT** be
    pulled into this slice.
22. The source art already exists: `SK_Chr_Merchant_01`, as
    `assets/synty/polygon-fantasy-kingdom/Source_Files/Characters/SK_Chr_Merchant_01.fbx`
    and already converted to
    `assets/synty/converted/polygon-fantasy-kingdom/SK_Chr_Merchant_01.glb`. The
    Assets work is a **promotion through the existing pipeline, not new art**.
23. `public/models/synty/npcs/` already exists and holds MONSTER art — ghosts,
    skeletons, zombies — from the polygon-dungeon promotion wave, all sharing
    one 55-joint armature. **It is a promotion-wave bucket, not a taxonomy**,
    and rpg-dnd5e-web#559's director ruling already puts identity in each
    entry's `rulesRef`, not in the path. Merchant art **MUST NOT** be filed
    there merely because the ref type is `npcs`; it belongs with its own
    promotion wave. Renaming that directory is **out of scope here** and
    recorded as a recommendation in brainstorm.md.
24. Separately filed defect: the `monk` entry in that manifest carries
    `rulesRef: null` **and** `rulesRefNote: null` — the silent gap #559
    explicitly called unacceptable.

## 7. The room — authoring only, no code

25. The vestibule **MUST** be a region of the same dungeon spec. No second zone.
26. `start` **MUST** be a cell inside it, and a `DoorSpec` **MUST** separate it
    from the first chamber. Opening that door refreshes sight, which forms a
    bubble with anything hostile beyond — existing behaviour, relied on.
27. Whether the door is locked is authoring, and **MUST NOT** become a rule.

## Handoff — what FadedPez attaches to

- A member with `Kind == KindWorld` at an authored cell and facing, addressable
  by `MemberID`.
- Reaching the client as `MEMBER_KIND_WORLD`.
- Nothing in the hostile path acting on it, proven by rule 7's audit.

Everything about what it can be *asked* — capabilities (`TALK`, `VENDOR`,
`TRAINER`, `QUEST_GIVER`, `QUEST_TARGET` are #311's suggestion, not this
document's ruling), the interaction verb, its wire shape, and whether
interaction requires adjacency — is his to design. **Note for that work:** the
existing door `Interact` has no adjacency check, so #311's "adjacent player" is
either a new rule or a shared correction to both.

## Out of scope

Actions of any kind; capabilities; the interaction verb; adjacency; vendor stock,
wallets, buying, selling; dialogue; quests; AI, turns, damage or death;
attackable world members; a disposition axis; region typing; renaming the
existing art directory.

## Acceptance

- A dungeon authored with an `npcs` placement compiles, and the member appears at
  its authored cell and facing with `Kind == KindWorld`.
- The builder offers the merchant in an `npcs` palette category, places it, and
  the YAML round-trips.
- The Inspector edits its facing and offers no prop-only or monster-only field.
- A player walks into line of sight of it and **no bubble forms**.
- A player in a running fight cannot name it as an attack target.
- A monster in a running fight never selects it as a target.
- A fight in which every monster is downed ends, with it still standing.
- It takes no turn and never appears in an initiative order.
- A player cannot walk through its cell.
- `Member.kind` reaches the client as `MEMBER_KIND_WORLD` and it renders.
