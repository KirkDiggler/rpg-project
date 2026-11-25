# Secret passages: fade to black, come back somewhere else

Status: idea, pre-architecture. Kirk, 2026-10-25. Castle story rpg-project#639.

## The use case

In the keep, the kitchen staff tell you about a secret passage. You take it:
the screen fades to black and you come back in a different part of the
castle. Two ways the map could then read:

1. **Relative, unconnected.** The new area appears where it really is on the
   one map, beside what you already explored; you can see roughly where you
   came from but not how the two connect. (This is what the fog of war does
   today for any cells you never saw.)
2. **A new map until the door opens.** What you explored before is hidden;
   the new area reads as a fresh map. When you open the door that joins the
   two, the old view comes back and the castle is one again.

Both are wanted as options. Neither should need a second map.

## What exists

- **Fog of war is per-observer sight testimony of cells** (discoveries). A cell
  you never saw is not drawn. A passage you walked unseeing leaves no
  discoveries, so its cells never appear. Option 1 is already the behaviour
  when the destination lies on the same atlas.
- **Discovery sharing** (the "Share discoveries" checkbox) decides whose
  discoveries a player draws; it does not decide which discovered cells are
  drawn.
- **Props carry state and can be hidden**; a door is a prop. A `fact` can gate
  a table row and a speech verb can grant one.
- **The world clock already moves members** without a walk the player drives
  (compelled moves, arrivals).

## The shape that looks elegant

Draw the **connected component** of the viewer's discovered cells that
contains the viewer, through discovered connections (doors, floor adjacency).
That is a rendering rule, not a new concept:

- Before the passage is known from the new side, the old area is a second
  component. Option 2 is "draw my component only"; option 1 is "draw every
  component, in place". The author (or the player) picks per site; the engine
  and the record are identical under both.
- Open the door from the new side and the two components join. The old view
  returns because the facts now connect, not because a flag flipped.
- The fade to black is a **traversal unseen**: the party is moved along cells
  it does not observe, so no discoveries are written for them. Same primitive
  as a compelled move with sight off; nothing special to the passage.
- The secret door is a hidden prop. The kitchen's tip is a `fact` from a
  speech verb that lets you find or use it. Finding it from the OTHER side is
  the same prop discovered by a different observer, so a party split across
  the passage reads it the same way.

## Open

- Where the component rule lives: a client projection over the discoveries
  it already holds (no protocol change) or a server-side view with the option
  in the session. The client reading looks right: the facts are already
  there, the choice is presentation.
- Whether "every component in place" should dim the ones you are not in.
- Whether an author can declare a passage as "unseen traversal" in the
  dialect, or whether it is a door prop with a `fade` on use.
- Stairs and levels are the same question one axis further; not for now.
