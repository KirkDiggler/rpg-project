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

**R1 — Two faces, one owner, ONE ID.** A concealed thing has a face for the
observer who does not know and its true self. Both are authored, but only the
true thing is PLACED: the disguise has no id of its own, no declaration, no
entry in the file beyond the binding that names its look. The engine chooses
which face THIS observer receives; the client renders one thing per placed
id and never holds both. Search targets the concealed thing's own id; the
ignorant observer simply sees it wearing the disguise's face. The alternative (the web carrying the disguise and
the underlying asset together and swapping on a check) was considered and
refused: a client that already holds the door cannot be told it does not
know, and that forecloses illusion.

**R2 — Detection is per player; a state change is public.** Knowing the
bookcase is a door is yours alone. Opening it is a fact of the room: the
moment it opens, everyone sees a door where the bookcase was. Physics is
always the true thing's physics in its current state — a concealed closed
door blocks movement and sight like the wall it hides in. The disguise
changes presentation only, never collision. (A pit that "looks like floor"
has different physics; that is a trap, its own primitive, not this.)

**R3 — Search targets the disguise, and is offered on everything.** A prop
with no concealment bound to it is still searchable and yields nothing. If
Search appeared only where something hides, the verb menu would be the tell.

**R4 — `notice` is the reserved slot: a passive tell, not a gate.** Same
approach list, resolved without dice against the observer's passive score
(10 + modifier, ±5 for advantage/disadvantage, RAW), evaluated when the
observer first sights the disguised thing (the perception stream's sighted
event; no new clock). Beating it delivers a per-observer hint — "something
is off here" — and changes nothing about what verbs you have. Untrained
disadvantage under a rules profile is −5 on the passive score, as RAW.

## The shape (not built)

On the binding of the placed thing — a door binding or a prop binding, same
keying law as `doorBindings` / `propBindings`:

```yaml
concealed:
  appearsAs: "dnd5e:props:bookcase"    # a DEFINITION ref (the same kind a placed prop's `ref` carries), NOT a placed id;
                                       # omitted = absent from an ignorant observer's atlas
  notice:  [{ ability: investigation, dc: 12 }]   # passive tell, optional
  checks:  [{ ability: perception, dc: 17 }, { ability: investigation, dc: 15 }]   # Search rolls these
```

`appearsAs` names a World Builder definition because the disguise needs an
asset and nothing else — its footprint and blocking are the true thing's in
its current state (R2). It is NOT a `propDeclarations` key: in v4 that block
is keyed by the placed item's id, so pointing at it would mean "looks like
that other bookcase over there", a second placed thing that does not exist.
(The first merged version of this note said "declaration id"; Kirk caught it.)

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
