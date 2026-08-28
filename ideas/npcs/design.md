# Place a world member — the minimal slice

Normative. Reasoning, rejected options and recommendations for later live in
[brainstorm.md](brainstorm.md).

Parent: **rpg-project#311** (World NPC Foundation).

## Scope

Kirk, 2026-08-28: *"We only want to be able to place a KindWorld npc into the
dungeon that can exist on a free roam clock that is not hostile to us.
Everything else will be managed by Pez. I do not want to overreach."*

So: **one enum value, one ref type, one placement, one wire value, and a builder
affordance.** Nothing else.

Explicitly **not** this slice: hostility as a relation, factions, the
interaction verb, capabilities, adjacency, vendor behaviour, and any change to
how the engine decides enmity. All of that is **FadedPez's**, via
rpg-project#310 and [rpg-toolkit#1275](https://github.com/KirkDiggler/rpg-toolkit/issues/1275).

## How non-hostility and the free-roam clock are achieved

**By adding a value the existing hostile-path switches do not name — and by
changing none of them.**

Every site that decides enmity is an allow-list keyed on `MemberKind`:
`sidesInContactOrder` partitions into players and monsters; `fightIsDecided`
counts only those two; `session/attack.go` builds candidates from
`Kind == KindMonster`; `behavior/basic.go` filters `Kind != KindPlayer`. A value
none of them names is excluded from all of them.

The free-roam clock follows from the same fact and is not separately arranged: a
member is moved to a turn clock only by entering a combat bubble, bubbles form
only from hostile pairs, so a member in no pair stays on the world clock.

**This is true today and it is not a guarantee.** It holds because no world
member is ever hostile to anyone. The moment one can be — a shopkeeper who
catches a pickpocket — non-hostility has to become a stated fact rather than an
omission. **That is Pez's to build**, and a recommended shape is recorded in
brainstorm.md. This slice **MUST NOT** build it, and **MUST NOT** write a rule
that forecloses it.

## 1. The kind — `rulebooks/dnd5e/encounter`

```go
// KindWorld is a member who belongs to the PLACE rather than to a person or to
// a fighting side: a merchant, a trainer, a quest-giver.
//
// It says where a member BELONGS, and deliberately NOT that it is harmless.
// Nothing hostile is authored as this kind TODAY, which is why the hostile-path
// switches can simply not name it -- but a shopkeeper who turns on a caught
// pickpocket would still be KindWorld. When that arrives, non-hostility becomes
// a stated fact rather than an omission.
KindWorld MemberKind = "world"
```

1. The godoc **MUST** say the kind describes belonging and **not** harmlessness,
   and **MUST** name the turned-shopkeeper case.
2. This slice **MUST NOT** modify `sidesInContactOrder`, `fightIsDecided`,
   `session/attack.go`'s candidate build, or `behavior/basic.go`. Adding the
   value is the whole change.
3. This slice **MUST NOT** state that `Kind` is immutable. Nothing assigns it
   after construction today; that is a fact about the code, not a rule, and a
   later model may want to change it.
4. Sites that switch on `Kind` for reasons **other** than enmity **MUST** be
   checked for a correct answer under the new value — `data.go`'s
   `Kind == KindPlayer` branch, `clocks.go`'s player checks, `field.go`'s
   input/view structs, `turndriver.go`. This is a **bounded check for
   correctness, not a refactor**; where the existing behaviour is already right,
   nothing changes.

## 2. The ref — `rulebooks/dnd5e/refs`

5. `module.go` **MUST** gain `TypeNPCs core.Type = "npcs"` beside `TypeMonsters`.
6. `dnd5e:npcs:merchant` **MUST** exist. `MemberKind` answers what a member is in
   a fight; the ref type names a content bucket beside `dnd5e:props:pillar`. The
   compiler is the one place the two vocabularies meet and **MUST** say so.
7. **`merchant` is a TYPE, not a look.** One ref **MUST** be renderable as
   several GLBs, per the existing `MONSTER_REF_MODELS: Record<string, string[]>`
   + `pickStableCandidateIndex` pattern and rpg-dnd5e-web#559's ruling that
   art-to-ref is not 1:1. That array is positionally indexed, so its order is
   load-bearing and **MUST NOT** be reshuffled.

## 3. The placement — `encounter/dungeonspec`

8. A ref of type `npcs` **MUST** compile to a member with `Kind == KindWorld`.
   `refKind` accepts `props` and `monsters` today and refuses everything else by
   name; it gains one segment.
9. The placement **MUST** accept `facing`, one of the eight true-compass names,
   validated as props' — a world member never turns in play, so the authored
   value is the only one there will be.
10. It **MUST** be refused `blocks_movement`, `blocks_los`, `targeting`, `boss`
    and `offset`, each by name.
11. One-placement-per-cell already holds and is unchanged.

## 4. The wire — `rpg-api-protos`

12. `MEMBER_KIND_WORLD = 3` **MUST** be added to the existing `MemberKind` enum.
    `Member.kind` is already on the wire: no new field, no new message.
13. The proto **MUST** merge before toolkit, api or web build against it.

## 5. The builder — `rpg-dnd5e-web/src/author`

The palette applies a **ref-AND-GLB test**: placeable only if a toolkit ref *and*
a promoted GLB both exist.

14. `PaletteCategory` **MUST** gain `npcs`, offering every `npcs` ref that passes
    that test — verified by resolution, not asserted.
15. The `place` tool **MUST** write an `npcs` placement and `dungeonYaml.ts`
    **MUST** round-trip it.
16. The Inspector **MUST** offer `facing`, and **MUST NOT** offer `offset`,
    `blocks_movement`, `blocks_los` or `boss`.
17. The renderer **MUST** resolve an `npcs` ref to one of its candidate models
    the way `monsterModels.ts` does.

## 6. Assets — Assets lane, not this slice

18. A promoted merchant GLB **MUST** exist before the builder can offer one.
    Filed under the journey with Team **Assets**.
19. Source art already exists: `SK_Chr_Merchant_01`, converted to
    `assets/synty/converted/polygon-fantasy-kingdom/SK_Chr_Merchant_01.glb`. A
    **promotion, not new art**.
20. `public/models/synty/npcs/` already holds MONSTER art from the
    polygon-dungeon wave — a promotion-wave bucket, not a taxonomy, with
    identity in each entry's `rulesRef` per #559. Merchant art **MUST NOT** be
    filed there merely because the ref type is `npcs`. Renaming is out of scope.

## 7. The room — authoring only, no code

21. The vestibule **MUST** be a region of the same dungeon spec. No second zone.
22. `start` **MUST** be a cell inside it, and a `DoorSpec` **MUST** separate it
    from the first chamber.
23. Whether that door is locked is authoring, and **MUST NOT** become a rule.

## Handoff — everything else is FadedPez's

This slice delivers a member with `Kind == KindWorld`, at an authored cell and
facing, addressable by `MemberID`, on the world clock, reaching the client as
`MEMBER_KIND_WORLD`, that nothing in the hostile path acts on.

His: capabilities, the interaction verb and its wire shape, adjacency, vendor
behaviour — **and hostility**, if an NPC is ever to turn. brainstorm.md records
a recommended shape for that (a pairwise predicate) and the two open bugs it
would also close, as **input, not instruction**.

Two things for him, found while scoping:

- The existing door `Interact` has **no adjacency check**, so #311's "adjacent
  player" is either a new rule or a shared correction to both.
- rpg-toolkit#899 and #766 are open bugs from the engine having no hostility
  concept; whatever he builds for a turning NPC is the same seam.

## Acceptance

- A dungeon authored with an `npcs` placement compiles; the member appears at its
  authored cell and facing with `Kind == KindWorld`.
- **No hostile-path file is modified** — verified by the diff.
- The member stays on the world clock and never appears in an initiative order.
- A player walks into line of sight of it and no bubble forms.
- A player in a running fight cannot name it as an attack target.
- A monster in a running fight never selects it as a target.
- A fight in which every monster is downed ends, with it still standing.
- A player cannot walk through its cell.
- The builder offers the merchant in an `npcs` palette category, places it, the
  YAML round-trips, and one ref resolves to several candidate looks.
- `Member.kind` reaches the client as `MEMBER_KIND_WORLD` and it renders.
