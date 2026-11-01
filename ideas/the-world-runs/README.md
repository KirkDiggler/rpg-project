# The world runs

Story rpg-project#766. Design: [design.md](design.md). The measurement site is
the benchmark keep's open question O1, a driven garrison
([../benchmark-keep/design.md](../benchmark-keep/design.md)).

## The scene

A keep of ten thousand hexes. A hundred creatures live in it: twenty walk
routes, a dozen stand posts, the rest sleep in barracks, sit at tables or
stand in a kitchen with nothing to do until something happens. The party
walks the outer ward.

Today every pace the party walks gives every one of the hundred a turn. Each
turn builds that creature's view of the world, rolls its `time` table, and if
it moved, rebuilds what every member of the roster can see. The barracks
sleep by rolling `hold` a hundred times a pace.

After this story a pace does this: the clock moves one round; the twenty on
routes are due and take their leg; the kitchen hears nothing and is not
asked; the barracks has a machine in state `asleep` whose only transitions
are "a sound within two regions" and "the bell", neither of which landed,
so it is never visited. The party forces a door in the ward. The crash is a
beat; the listener index says the post at the gate and the two patrols in
earshot listen for `heard`; they wake in the same raise, in seat order, roll
their rows and move to `alert`. The sight that refreshes is the sight of
whoever could see the cells they crossed.

Everything that moved is on the record as beats, the way picks already are,
with the state a creature left and the state it entered. Every roll the
world makes is on the record too, so the same keep replayed from the same
record makes the same choices, and the dice stay as random as they can be.

## What the operator sees

- The keep with a garrison that lives: patrols pass, posts stand, a forced
  door's crash wakes the gate.
- A Move on that keep costs what it costs on an empty one when nothing woke,
  and grows with how many woke, never with how many exist.
- The event log reads a creature's state change in the author's words
  (`asleep → alert, heard: crash within 2`).

## Reading order

1. [design.md](design.md) — the shape, the law, approaches, the walkthrough,
   rulings.
2. rpg-project#690 — the ruling this design applies: a creature's clock is
   content.
3. `rpg-toolkit/examples/effects/machine` — the recipe shape this design
   lifts.
