# "Single room" is a name for an implementation detail

**Status:** FINDING 2026-09-24, CORRECTED the same day. This doc first claimed the
name hid a **ceiling** — that the dialect's one region was a capability gap
needing three rulings. **Kirk ruled there is no use case for regions and no need
for them** (concealment now contains what it hides), and the correction is right:
a capability nothing wants is not a gap. What survives is the naming point, on
its own and much smaller.

## What Kirk said

> "The whole wording around single room is a little disturbing too. We draw walls
> and separate different areas and we have doors that can be opened."

> "I didn't think we need regions where? At least we don't have a use case for it
> yet. With our new concealment method where concealed contains everything that
> it's hiding, I don't have a use case for regions."

## What is actually true

The dialect compiles **one region** (`single_room_compile.go`):

```go
Regions: []encounter.RegionInput{{
    ID: spec.Room.Gameplay.ImplicitRegionID,
    Name: spec.Room.Name,
    Cells: cells,
    Archetype: "crypt",
    Lighting: &bright,
}},
```

And an author can already:

- **draw walls** — derived lines with footprints, crossings and sealing;
- **place doors** — footprints with `open`/`closed` state;
- **separate areas physically** — walls divide space, and that is what an author
  means by "a different area".

So "single room" is **not** describing a limitation an author hits. It is
describing **the region count**, which is an implementation fact no author
thinks in. Walls and doors are how areas are made, and they work.

## Why the wording still reads wrong (the whole surviving finding)

The name answers a question nobody asked, and it **understates the dialect**:

- An author drawing a corridor and three chambers has made four areas, and the
  document says "one room". It reads as a limit even though nothing is refused.
- **The author-facing surface already says "room"** in the places an author
  reads — `RoomGameplaySource`, `room.room`, `RoomMonsterBinding`,
  `propDeclarations`. The *dialect identifier* is the odd one out.
- It is a **migration artifact**: the name distinguished this dialect from v2's
  three-room chain. With v2 being removed
  ([`one-room-one-shape.md`](./one-room-one-shape.md)), the name distinguishes
  nothing — there is no other dialect to be "single" against.

**That last one is the real find.** The name is residual vocabulary from a
comparison that is about to stop existing. When v2 goes, "single room" has no
referent.

## What to do about it

**Rename, and name it for what an author writes.** The candidates belong in the
work, not here — but the constraint does not: the name must describe *a floor
with walls and doors*, not a count of engine nouns. `room` is already the word
the surface uses, so the honest options are variants of that.

**Sequencing: after v2 is removed.** Until then "single" means "not the v2
chain", and the rename would lose that distinction mid-flight.

## What this doc got wrong, kept visible

The first version of this doc turned the region count into a **ceiling** with
three rulings attached — raise it, or name it, or drop `ImplicitRegionID`. That
was over-design, and Kirk's correction is the cleaner read:

- **A use case brings the mechanism.** There is no use case for per-area
  lighting, per-area archetype, or a monster bound to an area. Concealment,
  built since, contains what it hides — so even hiding does not want a region.
- **Regions are not what an author means by an area.** Walls are. So the missing
  noun was never missing.
- **I read a hardcoded constant as a gap.** `Archetype: "crypt"` and
  `Lighting: bright` are constants because nothing has needed them to vary. That
  is a value not yet paid for, which is the stated standard — not a defect.

The lesson is the one this project already writes down: **a finding is not a
ruling.** I found a real fact (one region, constants, an `ImplicitRegionID`) and
promoted it to a decision the project owed, when the honest response was "noted,
no use case, move on". The naming observation was the whole of the finding; the
ceiling was invented around it.

## Not this doc

- **The wall bug** — a wall along a cell edge seals nothing, so a member stands
  in it and sees through. rpg-toolkit#1894. Separate, and real.
- **The door half of it** — see rpg-toolkit#1894's thread: `Step` onto a shut
  leaf IS refused for the geometry it was tested in, and the geometry it was not
  tested in is unguarded.
- **v2 removal** — [`v2-removal-plan.md`](./v2-removal-plan.md).

— rpg-toolkit agent, on behalf of KirkDiggler