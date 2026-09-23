# "Single room" is a limit, not a name

**Status:** FINDING 2026-09-24. Records a naming problem that turned out to be a
real ceiling, so the rename is not the fix. Not a design; the design is what
comes after the ruling.

## What Kirk said

> "The whole wording around single room is a little disturbing too. We draw walls
> and separate different areas and we have doors that can be opened."

The instinct was right, and it is tracking more than wording. **"Single room" is
accurate about the implementation today.** The name is the symptom; the ceiling
is the thing.

## The ceiling, in one line of code

`single_room_compile.go` builds the whole field as **one region**:

```go
field := encounter.FieldInput{
    Regions: []encounter.RegionInput{{
        ID:        spec.Room.Gameplay.ImplicitRegionID,
        Name:      spec.Room.Name,
        Cells:     cells,
        Archetype: "crypt",
        Lighting:  &bright,
    }},
    ...
```

Every part of that is fixed:

- **One region**, from one `Cells` list — every walkable hex is in it.
- **`ImplicitRegionID`** is a field on the spec (`RoomGameplaySource`), so the
  document itself carries the assumption that there is exactly one.
- **`Archetype: "crypt"`** and **`Lighting: bright`** are constants the author
  cannot write.

So the dialect supports:

- **walls** — yes, as derived lines with footprints, crossings and sealing.
- **doors** — yes, as footprints with state (`open`/`closed`), and a shut one
  refuses a step by name.
- **different areas** — **no.** There is one region, and a region is what the
  engine means by an area ([ADR-0044](../../../../rpg-toolkit/docs/adr/0044-regions-replace-rooms.md),
  "regions replace rooms").

Walls and doors *subdivide space physically* without producing areas the engine
knows about. That is why the wording reads wrong: an author drawing a corridor
and three chambers has made four areas, and the document can only say "one".

## Why the name and the ceiling are the same finding

They are not two problems. The name **"single room" describes the region count**,
and the region count is what is capped. Renaming the dialect without raising the
ceiling would make the code *less* honest — it would call itself general while
still compiling one region, and the next reader would trust the name.

**So the order is: decide whether the ceiling comes up, then name it for what it
is.** Naming first is the mistake this doc exists to prevent.

## What is actually blocked by the ceiling

Not geometry — the author can already *draw* separate areas with walls. What is
blocked is everything that keys off a region:

| Wants | Needs |
|---|---|
| a name for each area ("the vault", "the hall") | more than one region |
| per-area lighting | per-region `Lighting`, which exists in `RegionInput` but is one constant here |
| per-area archetype | per-region `Archetype`, same |
| "the party is in the front room" | `RegionAt`, which answers for one region today |
| a monster bound to an area, not a cell | region membership |

`RegionAt`, `MembersIn(region)` and `Region` all exist and are general
(`region.go`, `field.go`) — **the composition is ready and the dialect is not.**
That is the usual shape of this kind of gap: the lower layer has the primitive
and the authored surface cannot express it.

## What this does not mean

- **Not a bug.** Nothing is wrong; one region is a coherent slice, and the
  engine honours it.
- **Not urgent by itself.** Nothing in flight needs per-area lighting.
- **Not the wall/door defect.** That is rpg-toolkit#1894 and is separate: a wall
  along a cell edge seals nothing, so a member can stand in it. That is a
  geometry defect *within* one region and does not depend on this.

## What to rule on

1. **Does the ceiling come up** — does the v4 dialect gain authored regions
   (several `Cells` lists, each with its own name/archetype/lighting), or does it
   stay one region and the walls-and-doors story remain purely physical?
2. **If it stays one region, what is it called?** The name must describe what the
   document *is* — a floor with walls and doors — and must not imply areas the
   engine cannot see. Candidates belong in the design, not here; the point is
   that the name follows the answer.
3. **Does `ImplicitRegionID` survive at all?** If several regions arrive, it is a
   migration artifact; if one region stays, it is honest.

## Why this is worth deciding now rather than later

It is the **same class** as the rest of this session's findings, and it is the
third instance today:

- `place[]` mixes two vocabularies on one object (v2, being removed).
- `intimidate`/`persuade` are a social shape on a combatant.
- **one region is a ceiling the name admits and the document cannot exceed.**

Each is a shape that was right for its slice and is now load-bearing beyond it.
The lens says to spend the pre-pre-alpha window fixing shape. This one is cheap
to decide now — nothing depends on per-area lighting yet — and expensive once
content is authored against a one-region assumption and then has to be split.

— rpg-toolkit agent, on behalf of KirkDiggler