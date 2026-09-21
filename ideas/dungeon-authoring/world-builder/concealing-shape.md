# Concealing: one noun, everything hidden belongs to it

**Status:** RULED 2026-09-21 (Kirk), NOT SCHEDULED. No slice, no build. This
note exists so the sites-layer work deferred at v4 R3 (`v4-gameplay-shape.md`)
starts from these rulings when a use case brings it. "A use case brings the
mechanism" still applies.

## How we got here (the corrections, kept visible)

1. First sketch: the web holds a concealed asset and the underlying asset and
   swaps on a check. Refused: a client that already holds the door cannot be
   told it does not know, and that forecloses illusion. The engine decides
   what each observer receives.
2. Second: a "face" on the concealed thing (`appearsAs`). Refused: a face
   cannot be lined up; lining a bookcase up over a door is authoring only the
   author can do.
3. Third: hidden floor derived by reachability through unknown doors.
   Refused: derivation is v2's trickiness in new clothes (a concealed region,
   a concealed door and a wall run all had to agree, and when they did not,
   nothing on the page said why). The table view is the author's truth; what
   is hidden should be something they drew.
4. Landed: a concealment is a NOUN with an id and the checks; cells, props
   and doors are hidden by belonging to it.

## What exists today (v2, engine)

A door may carry `concealed: [{ ability, dc }]`; a region may be
`concealed: true`; Search is the verb; intel may `reveals: { door }`;
detection is per observer from birth; reveal-segments exist for growing an
observer's atlas. v4 refuses `reveals: { door }` (R3 of the v4 shape) — its
true target is a concealment, see below.

Not verified while writing this: what the atlas wire sends today for a
concealed door or region, and whether the api filters the atlas per observer.
First check at slice time, not a ruling.

## Rulings

**R1 — One noun.** `concealments.<id>` carries the checks and lists what it
hides: cells, and placed things by id. Nothing else in the file says
"hidden". No `concealed: true` on a region, no `concealed` on a door binding,
no face, no derivation.

**R2 — Cells stay in `walkableHexes`.** The author sees the vault at the
table. An unaware observer is not sent those cells, and the web draws wall on
the boundary exactly as it does for any edge of the walkable set. Nobody
places a wall to hide a room; omission is the wall. Whether a listed cell is
walkable is the ENGINE's refusal at publish, not the web's — the web carries
the list, the engine grades it (the same split `doorBindings` keeps).

**R3 — Detection is per observer; a concealment goes PUBLIC on two
events.** (a) A door belonging to it is opened. (b) Any observer sees a
creature standing on one of its cells — a party member walking through the
wall is the illusion breaking for the watcher. Until then, knowing is yours
alone: you are sent the cells and the things, you get Open on the door,
others still see wall.

**R4 — No overlap.** A cell, prop or door belongs to at most one
concealment; listed twice is refused by the validator naming both lines.
Nesting (a closet inside the vault) is the widening a use case may bring.

**R5 — Search is offered everywhere and rolls the concealment's checks.**
Search from a cell TOUCHING a concealment (adjacent to one of its cells, or
to a placed thing it hides) rolls that concealment's `checks`; Search where
nothing hides yields nothing. The verb menu is never the tell.

**R6 — `notice` is the passive tell, not a gate.** Same approach list,
resolved without dice against the observer's passive score (10 + modifier,
±5 for advantage/disadvantage, RAW) when the observer first sights a cell
touching the concealment (the perception stream's sighted event; no new
clock). Beating it delivers a per-observer hint — "something is off here" —
and changes no verb. Untrained disadvantage under a rules profile is −5 on
the passive score, as RAW. Kirk: "the ability to add notice is good enough
for now."

**R7 — Intel names the concealment.** `reveals: { concealment: vault }`.
This is what `reveals: { door }` was reaching for, and it is no longer about
doors: it reveals the cells and everything hidden with them.

**R8 — A door is optional.** Cells with no door are a secret alcove: reveal
the concealment and the boundary opens where the author left the gap. A door
listed in a concealment is a door that also happens to be hidden — its
`doorBindings` (closed, locked) are unchanged.

## The shape (not built)

```yaml
concealments:
  vault:
    notice: [{ ability: investigation, dc: 12 }]                                  # optional
    checks: [{ ability: perception, dc: 17 }, { ability: investigation, dc: 15 }] # REQUIRED
    cells:  [{ q: 12, r: 3 }, { q: 13, r: 3 }, { q: 12, r: 4 }]                    # subset of walkableHexes; the engine grades that at publish
    props:  [vault-door, inner-wall-1, heirloom]                                   # placed ids: doors, wall props, anything

intel:
  - { id: vault-map, reveals: { concealment: vault } }
```

Refusals the engine owes: an unknown cell (not in `walkableHexes`); an
unknown placed id; a cell or id in two concealments (R4); a concealment with
no `checks`; a concealment that hides nothing; intel naming no such
concealment. Internal walls are wall props in this dialect and need nothing
special. A bookcase in front of the hidden door is an ordinary placed prop
with no link to the concealment.

## Divergence from RAW, stated

RAW has one DC; the passive score is a floor on it, and a character whose
passive beats it notices the secret door outright. This shape has two lists,
so the rogue gets a "hm" and still has to search. An author who wants RAW
sets `notice` equal to `checks`. A session-level rules profile may pick the
default later; that is a rules-profile question, not a concealment one.

## Mechanisms this asks for later, named so nobody briefs them as presentation

- A per-observer atlas: the api withholding cells and placed things per
  observer, and growing an observer's atlas on reveal (reveal-segments is
  the precedent).
- A public reveal beat (R3) that every observer's atlas grows on.
- A placed prop LEAVING the field (the bookcase swinging away when the door
  opens). Props can arrive today; nothing removes one. Not needed for the
  shape above; named because it is the first thing an author will ask for.

## Out of scope, named

- Traps, pits, anything whose physics differs from its face.
- Nesting or overlapping concealments (R4).
- Sharing a detection by any means other than R3's two public events.
- False tells (a `notice` that lies). The per-observer testimony shape
  allows it later; nothing here asks for it.
