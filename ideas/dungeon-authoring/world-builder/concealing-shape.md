# Concealing: the shape we leave room for

**Status:** RULED 2026-09-21 (Kirk), NOT SCHEDULED. No slice, no build. This
note exists so the next person who touches concealment starts from these
rulings rather than from scratch. It is the sites-layer conversation that the
v4 gameplay shape deferred at R3 (`v4-gameplay-shape.md`), written down while
it was fresh. Nothing here is a commitment to build; "a use case brings the
mechanism" still applies.

## What exists today (v2, engine)

- A door may carry `concealed: [{ ability, dc }]` — the approach grammar,
  beaten by any listed route. A region may be `concealed: true`.
- Search is the verb; a beaten check reveals to the searcher.
- An intel record may `reveals: { door }`, the second way to know.
- Detection is per observer from birth (pre-v1 wire law).
- v4 refuses `reveals: { door }` until this layer lands (R3).

Not verified while writing this: what the atlas wire sends today for a
concealed door or region, and whether the api filters the atlas per observer.
That is the first check at slice time, not a ruling.

## Rulings

**R1 — The disguise is PLACED by the World Builder.** A concealed thing has
a face for the observer who does not know and its true self. The face is an
ordinary placed prop with its own id, footprint, facing and blocking —
lining a bookcase up over a door in a wall is authoring only the author can
do (Kirk, 2026-09-21). The engine chooses what THIS observer receives per
placed id; the client never holds anything the engine did not send. The
alternative (the web carrying the disguise and the underlying asset together
and swapping on a check) was considered and refused: a client that already
holds the door cannot be told it does not know, and that forecloses illusion.
(This note first said the disguise was a face with no id of its own; Kirk
corrected it — a face cannot be lined up.)

**R2 — Detection is per player; a state change is public.** Knowing the
bookcase is a door is yours alone. Opening it is a fact of the room: the
moment it opens, everyone sees a door where the bookcase was. Physics is
always the true thing's physics in its current state — a concealed closed
door blocks movement and sight like the wall it hides in. The disguise
is real too: a closed door and the bookcase over it both block, and the
author lines them up. Everyone sees the bookcase until the door OPENS —
knowing there is a door behind it does not make it vanish, it makes Open
appear on it for you. Opening removes the disguise for everyone. (A pit that
"looks like floor" has different physics; that is a trap, its own primitive,
not this.)

**R3 — Search targets the disguise, and is offered on everything.** Search
on a prop resolves to whatever that prop conceals; a prop that conceals
nothing is still searchable and yields nothing. If
Search appeared only where something hides, the verb menu would be the tell.

**R4 — `notice` is the reserved slot: a passive tell, not a gate.** Same
approach list, resolved without dice against the observer's passive score
(10 + modifier, ±5 for advantage/disadvantage, RAW), evaluated when the
observer first sights the disguised thing (the perception stream's sighted
event; no new clock). Beating it delivers a per-observer hint — "something
is off here" — and changes nothing about what verbs you have. Untrained
disadvantage under a rules profile is −5 on the passive score, as RAW.

## The shape (not built)

The disguise is placed like any prop; the concealment sits on the CONCEALED
thing's binding, so the DCs have one home whether or not a disguise exists:

```yaml
room:
  propDeclarations:
    bookcase-1: { blocksMovement: true, blocksLineOfSight: true, footprint: {...} }   # the disguise, placed and faced by the author
  doorBindings:
    vault-door:
      closed: true
      concealed:
        by: bookcase-1                                  # optional; omitted = absent from an ignorant observer's atlas ("looks like a wall")
        notice: [{ ability: investigation, dc: 12 }]    # passive tell, optional
        checks: [{ ability: perception, dc: 17 }, { ability: investigation, dc: 15 }]   # Search on the bookcase rolls these
```

A prop binding may carry the same `concealed` block (a lever behind a
tapestry). Nothing in the file proves the bookcase actually covers the door;
by ruling that is the author's job. A slice may warn when a disguise's
footprint touches none of the concealed thing's cells — validation, not shape.

**The mechanism this asks for later, named so nobody briefs it as
presentation:** a placed prop LEAVING the field when the door opens. Props
can arrive today; nothing removes one. That is the cost the use case pays.

Three ways to know, one fact: beat `checks` by Search, hold intel that
reveals it, or watch someone open it (R2).

## Divergence from RAW, stated

RAW has one DC; the passive score is a floor on it, and a character whose
passive beats it notices the secret door outright. This shape has two lists,
so the rogue gets a "hm" and still has to search. An author who wants RAW
sets `notice` equal to `checks` and gets the collapse. A session-level rules
profile may later pick the default; that is a rules-profile question, not a
concealment one.

## Out of scope, named

- Traps, pits, anything whose physics differs from its face.
- Sharing a detection with the party by any means other than opening it.
  Strictly per observer; "trust me, there's a door here" is said out loud.
- False tells (a `notice` that lies). The per-observer testimony shape
  allows it later; nothing here asks for it.
