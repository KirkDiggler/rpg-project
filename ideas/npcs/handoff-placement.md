# Handoff — placing a world member (FadedPez's lane)

**This is not our slice.** Kirk, 2026-08-28: *"I think we turn over the
`KindWorld` addition to them too."* Everything here is research, offered as
input rather than instruction — the shape is yours to choose.

Our lane is hostility only: [design.md](design.md). The one thing it guarantees
you is that **any non-monster kind is hostile to nobody**, decided by
`(*Encounter).hostile` rather than by four switches happening not to name your
value. So a `KindWorld` member is non-hostile as a stated fact, and stays on the
free-roam clock as a consequence — a member reaches a turn clock only by
entering a combat bubble, and bubbles form only from hostile pairs.

Parent: **rpg-project#311**. Vendor behaviour: **rpg-project#310** /
**rpg-toolkit#1275**.

## What the kind would need

`MemberKind` is a string enum with `player` and `monster`. Kirk's ruling on the
third value:

```go
// KindWorld is a member who belongs to the PLACE rather than to a person or to
// a fighting side: a merchant, a trainer, a quest-giver.
//
// It says where a member BELONGS, and deliberately NOT that it is harmless. A
// shopkeeper who turns on a caught pickpocket is still KindWorld; what would
// change is the hostile() relation, not what he is.
KindWorld MemberKind = "world"
```

Why `world` and not `npc`: `npc` defines the kind by **negation** — "not a
player" — which monsters satisfy too, so the category has no edge. The case that
settled it is the **hired mercenary**, an ally who fights: not a player, not a
monster, so `npc` would have welcomed it in. `world` refuses it by name, because
a mercenary belongs to a side rather than to the place.

Sites switching on `Kind` for reasons other than enmity are worth a correctness
check under a new value — `data.go`'s `Kind == KindPlayer` branch, `clocks.go`'s
player checks, `field.go`'s input/view structs, `turndriver.go`. A bounded check,
not a refactor.

Note also: `Kind` is never assigned after construction anywhere in the engine
today. That is a fact about the code, **not** a rule — we deliberately did not
write an immutability invariant, so a turning shopkeeper stays possible.

## The ref

- `rulebooks/dnd5e/refs/module.go` defines type constants; it would gain
  `TypeNPCs core.Type = "npcs"` beside `TypeMonsters`.
- `dnd5e:npcs:merchant` is Kirk's ruling for the ref. The ref type names a
  **content bucket** beside `dnd5e:props:pillar`; `MemberKind` answers what a
  member is in a fight. Two vocabularies on purpose — `dnd5e:world:merchant`
  reads badly, since "world" is not a category of thing.
- **`merchant` is a TYPE, not a look** (Kirk). One ref renders as several GLBs,
  which the existing `MONSTER_REF_MODELS: Record<string, string[]>` +
  `pickStableCandidateIndex` already supports, and rpg-dnd5e-web#559 ruled that
  art-to-ref is not 1:1. That candidate array is **positionally indexed**, so
  reordering it silently restyles every placed entity.

## The placement — `encounter/dungeonspec`

`refKind` in `validate.go` accepts exactly `props` and `monsters` and refuses
everything else **by name**; it would gain one segment. `PlaceSpec` carries
`Ref`, `At`, `BlocksMovement`, `BlocksLoS`, `Facing`, `Offset`, `Targeting`,
`Boss`.

Suggested rules, with the reasoning so you can disagree with it:

- **Accept `facing`** — the eight true-compass names, validated as props' are.
  Monsters are refused facing because they turn dynamically in play; a world
  member never does, so the authored value is the only one there will ever be.
  (Kirk: *"they have no behavior so will need a facing."*)
- **Refuse `blocks_movement` / `blocks_los`** — those are a *prop's declaration*.
  A member occupies its cell without declaring it, so #311's "world NPCs may
  block movement like an occupied tile" is satisfied without the field.
- **Refuse `targeting`, `boss`, `offset`.** `offset` is refused only because
  refusing is the reversible default; if a merchant wants nudging behind a
  counter, that is a ruling to make with the case in hand.
- One-placement-per-cell already holds.

## The wire

`Member.kind` is **already on the wire** (field 2, enum `MemberKind` with
`UNSPECIFIED/PLAYER/MONSTER`). So this is one additive value —
`MEMBER_KIND_WORLD = 3`. No new field, no new message, no reserved tag. Protos
merge before toolkit/api/web build against them.

## The builder — `rpg-dnd5e-web/src/author`

`Palette.tsx` / `paletteData.ts` / `Inspector.tsx` / `dungeonYaml.ts`, with a
`place` tool already in `types.ts`.

**The constraint that costs the most:** the palette applies a **ref-AND-GLB
test** — a thing is offered only when a toolkit ref *and* a promoted GLB both
exist, verified by resolution rather than asserted. `paletteData.ts` documents
refs excluded for failing either half (`ghoul`/`skeleton-archer` have refs and no
art; `ghost`/`specter` have art and no ref).

So: a `PaletteCategory` of `npcs`, the `place` tool writing the placement,
`dungeonYaml.ts` round-tripping it, the Inspector offering `facing` and not the
prop-only fields, and a model-resolution path beside `monsterModels.ts`.

## Assets — Assets lane

- **The art already exists.** `SK_Chr_Merchant_01` ships in the Synty Fantasy
  Kingdom pack and is **already converted** to
  `assets/synty/converted/polygon-fantasy-kingdom/SK_Chr_Merchant_01.glb`. The
  task is a promotion through the same pipeline that promoted the four player
  classes, not a hunt for new art.
- **Do not file it under `public/models/synty/npcs/`** just because the ref type
  is `npcs`. That directory holds MONSTER art from the polygon-dungeon wave —
  seven models on one 55-joint armature. It is a **promotion-wave bucket, not a
  taxonomy**, and rpg-dnd5e-web#559 already puts identity in each entry's
  `rulesRef` rather than the path. Renaming it would swap one aspirational label
  for another and rot immediately.
- Defect found in passing: that manifest's `monk` entry carries `rulesRef: null`
  **and** `rulesRefNote: null` — the silent gap #559 called unacceptable.

## The room — authoring, zero code

Kirk's framing: a region *before* the dungeon's `start`, entered through a real
door, with the merchant in it. No second zone, no new world concept.

A region carrying `start`, and a `DoorSpec` between it and the first chamber.
The behaviour Kirk wanted — *"if there are monsters in the room this is how it
would really play out"* — is existing behaviour: opening a door refreshes sight,
and anything hostile beyond it enters the contact pass. Whether that door is
**locked** is an authoring choice; locked-with-a-DC already exists and makes the
vestibule a threshold rather than a hallway.

## Two findings, not decisions

- The existing door `Interact` has **no adjacency check**, so #311's "interacted
  with by an adjacent player" is either a new rule for interaction or a shared
  correction to both.
- #311 suggests capabilities `TALK` / `VENDOR` / `TRAINER` / `QUEST_GIVER` /
  `QUEST_TARGET`. We deliberately did **not** specify them — they describe what
  an NPC can be *asked*, which is your half.
