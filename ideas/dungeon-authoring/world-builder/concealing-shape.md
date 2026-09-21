# Concealing: one noun, everything hidden belongs to it

**Status:** RULED 2026-09-21 (Kirk). **BUILDING** from 2026-09-21 (Kirk: "let's implement the backend the builder can follow") — see "Engine mapping" and "Wave" below. This
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

Verified 2026-09-21 (scout report, cited in the wave section): the atlas IS
per observer already — `Encounter.AtlasFor(member)` withholds a hidden
region's cells/props/walls and masks an unfound concealed door's edge as an
ordinary wall at the neighbouring run's height; the session SDK and
rpg-api's `GetAtlas` are member-scoped; reveal beats are per recipient.
Knowledge is a journal fact (`known:door:<id>`, `known:region:<id>`) folded
through a graph, one reveal writer (`revealDoorTo`/`revealRegionTo`), five
causes (Search find, perceived open, crossed, looted intel, seen occupied).
Footprint doors may NOT be concealed today (refused by name). Placed props
are withheld wholesale from the atlas and are not on the wire
(rpg-api-protos#351).

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

## Engine mapping — RULED 2026-09-21 (Fable, on Kirk's "implement the backend")

The noun becomes the engine's ONE concealment primitive; the two flags it
replaces are retired (no backcompat baggage — nothing runs on this).

- **E1 — `encounter.ConcealmentInput{ID, Checks, Notice, Cells, Doors, Props}`
  on `FieldInput.Concealments`.** `DoorInput.Concealed` and
  `RegionInput.Concealed` are REMOVED. One knowledge fact kind
  `known:concealment:<id>` replaces `known:door`/`known:region`; one reveal
  writer; the graph pierce per concealment. `AtlasFor` withholds the
  concealment's cells (and everything standing on them), withholds member
  props, masks member door edges as wall; footprint member doors are withheld
  like props and their cells masked — the "nothing has built yet" refusal is
  built. Wide/edge doors keep working.
- **E2 — Search is unchanged in shape** (`SearchInput{Member, Region}`): it
  rolls the `checks` of every unknown concealment TOUCHING the region — a
  concealment cell adjacent to a region cell, or a member door with an edge
  in the region — one roll per concealment, not per door.
- **E3 — Reveal causes, as today, re-targeted at the concealment:** Search
  find; looted/held intel `reveals: { concealment }`; crossing into its
  cells; perceiving a member door standing open; perceiving a creature on
  its cells. **Divergence from R3 as Kirk phrased it ("everyone would see
  it"):** the engine's standing law is "revealed to whoever PERCEIVES it"
  through the witness seam, per observer, and this wave keeps that law. A
  member across the dungeon learns when they see it. Kirk may overrule to
  strictly public; it is one line.
- **E4 — Beat:** one new per-recipient beat `concealment_revealed` carrying
  the concealment id, its cells, props, member doorways, boundaries, the
  segments DIFFERENCE, the sealed REPLACEMENT, and the touched `regions` as
  a REPLACEMENT (an unaware observer's region entry has the concealment's
  cells dropped, or is withheld when every cell is hidden; on reveal each
  touched region arrives whole) — the `region_revealed` payload shape, plus
  doors. Protos: rpg-api-protos#352. `door_revealed` and `region_revealed` retire
  when nothing emits them. Protos: additive `EVENT_KIND_CONCEALMENT_REVEALED`
  + body; the old kinds deprecated, never changed in place.
- **E5 — dungeonspec, both dialects, one lowering.** v4: root
  `concealments` (shape above) → `FieldInput.Concealments`; `intel.reveals:
  { concealment }` accepted, `{ door }` stays refused in v4 by name. v2:
  each `concealed: true` region lowers to a concealment with its cells and
  every concealed door touching it as a member (checks = union, "beaten by
  any listed route"); a concealed door touching no concealed region lowers
  to a cell-less concealment with one member (a hidden crossing); v2
  `reveals: { door }` lowers to the concealment holding that door. The v2
  goldens change accordingly and the change is pinned, not hidden.
- **E6 — `notice` (slice 2):** a second fact kind `noticed:concealment:<id>`;
  the `CheckResolver` seam gains `ResolvePassive(member, approaches)` and the
  session answers 10 + skill modifier ±5 (Perception exists; Investigation
  is `GetSkillModifier(skills.Investigation)`); evaluated inside
  `sweepConcealment` for observers with line of sight to a cell adjacent to
  the concealment; per-recipient beat `concealment_noticed` naming the
  concealment and nothing else. Slice 2 starts after slice 1 lands.

## Wave

| # | Repo / module | Content | Gate |
|---|---|---|---|
| P0 | rpg-api-protos | `EVENT_KIND_CONCEALMENT_REVEALED` + `ConcealmentRevealed` body; old two kinds `[deprecated = true]` | READY PR, merges first |
| P1 | rpg-toolkit `rulebooks/dnd5e/encounter` (+ dungeonspec) | E1–E5 | DRAFT until walk |
| P2 | rpg-toolkit `rulebooks/dnd5e/session` | pin, beat decode, seam | DRAFT on pseudo-version |
| P3 | rpg-api dev | pins + event conversion | DRAFT on pseudo-version |
| P4 | slice 2: E6 across the same four | | after P1–P3 |

**The World Builder lane's landing item, load-bearing:** on the authored-room
render path the web draws the scene from `GetDungeon` (the AUTHOR's truth,
fetched once) and never draws atlas walls. Concealment cannot reach the
player's picture on v4 until the player view derives floor, walls and placed
things from the per-observer atlas — which needs placed props on the wire
(rpg-api-protos#351) and the atlas's `segments`/`sealed` honoured. That is
theirs; this wave makes the atlas say the right thing.

## Mechanisms this asks for later, named so nobody briefs them as presentation

- Placed props on the per-observer atlas wire (rpg-api-protos#351) — the
  thing that lets the v4 picture honour a concealment.
- A placed prop LEAVING the field (the bookcase swinging away when the door
  opens). Props can arrive today; nothing removes one. Not needed for the
  shape above; named because it is the first thing an author will ask for.

## Out of scope, named

- Traps, pits, anything whose physics differs from its face.
- Nesting or overlapping concealments (R4).
- Sharing a detection by any means other than R3's two public events.
- False tells (a `notice` that lies). The per-observer testimony shape
  allows it later; nothing here asks for it.
