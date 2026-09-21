# Concealing in the World Builder — authoring the noun

**Status:** authoring design, 2026-09-21, reviewed by Kirk (via monster-ai) the same day. NOT SCHEDULED, and sequenced AFTER the runtime — see "Sequencing" below. Parent ruling:
[concealing-shape.md](concealing-shape.md) — RULED, NOT SCHEDULED ("a use case
brings the mechanism"). This doc is the authoring slice: how the World Builder
lets an author write the `concealments.<id>` noun the ruling landed on. It is
not the runtime (per-observer atlas, public reveal beat, a prop leaving the
field) and names those as out of scope rather than briefing them as presenters.

## One authoring choice made while this was written (Kirk agreed)

1. **Cells reuse the walkable paint gesture.** No new tool noun to start. The
   author paints a concealment's cells with the same brush/rectangle gesture
   the walkable set already uses, over a distinct tint that marks "this cell is
   claimed by concealment `vault`". A new "select cells for a concealment" tool
   appears only when a use case asks for it.

## Sequencing — the form lands last

Today the engine refuses `concealments` as a key it does not read, so a form
that writes it produces rooms `PutDungeon` rejects. And a key the toolkit
ACCEPTED before the per-observer atlas existed would give the author a knob
that does nothing in play — a lie to a streamer, and a fail-silent. So:

1. runtime shape in rpg-toolkit (per-observer atlas, public reveal beat) —
   brought by a use case, not scheduled;
2. `concealments` accepted and graded by dungeonspec, in the same wave;
3. THEN this form.

`cells` are a subset of the room's `walkableHexes` (parent R2). Whether a
listed cell is walkable is the engine's refusal at publish; the web carries
the list. (An earlier draft of this doc ruled cells "site-global" and able to
sit between rooms, citing the February multi-room design. That design is a
legacy tree — v4 has no `rooms[]` — and the ruling was not this doc's to
make; it is withdrawn.)

## The author always sees, the player never does — one list, two truths

Adding a cell to `concealments.<id>.cells` IS the entire act of hiding it.
There is no wall, no face, no second "hidden" flag, no and-then. Paint the room
as ordinary walkable floor, list those cells in the concealment, and the hiding
is done: an unaware observer is never sent the cells, and the web draws wall on
the boundary exactly as it does for any edge of the walkable set.

The one list carries two true facts at once, and both read the same array:

- **The author's reminder** — "these cells are `vault`'s; you hid them." Seen in
  the editor as a tint over ordinary walkable floor.
- **The observer's denial** — "these cells were withheld." The runtime atlas
  drops them for any observer who has not met R3/R5/R6.

There is deliberately no other place a "hidden" fact lives, and therefore no way
to add cells to a hidden list in one spot while forgetting to also mark the room
hidden somewhere else (R1's one noun, made concrete). The author always sees the
room at the table; the player never does until a reveal; and "what the author is
reminded is hidden" and "what the observer is denied" are the same bytes.

### Editor preview — how the author verifies "not visible to the players"

An "unaware observer" preview mode reads those same `cells`/`props` lists and
renders what a player who has found nothing would see: the concealed cells as
wall, the concealed props gone. It is a VIEW, not a second authority — it reads
the exact lists the runtime will withhold, so it cannot drift from what the
engine actually hides. Default stays author's truth (floor + tint); the preview
toggle lets the author confirm the room is invisible and the boundary is where
they intended, before pressing Play and without running the full atlas.

## The shape being authored

The ruling's shape, restated so the authoring slice reads against it:

```yaml
concealments:
  vault:
    notice: [{ ability: investigation, dc: 12 }]                                   # optional
    checks: [{ ability: perception, dc: 17 }, { ability: investigation, dc: 15 }] # REQUIRED
    cells:  [{ q: 12, r: 3 }, { q: 13, r: 3 }, { q: 12, r: 4 }]                    # subset of walkableHexes
    props:  [vault-door, inner-wall-1, heirloom]                                   # placed ids

intel:
  - { id: vault-map, reveals: { concealment: vault } }
```

`concealments` is a root key beside `factions`/`dispositions`/`intel`,
joined to the same v4 seam those opened. It is a key inside v4, not a new
version — the note already live in `singleRoomDungeon.ts` ("a `doorBindings`
wave adds a key here, not a version") applies verbatim.

## The three authoring gestures

### 1. Draw the room as always — nothing hidden goes here

Paint walkable cells (existing paint/rectangle tools), place the hidden door,
bookcase, inner wall and treasure as ordinary props. The bookcase needs no link
to the concealment (R8); the vault's cells are ordinary walkable cells. There is
no "wall" or "hide" tool. The hiding is membership named in gesture 2, nothing
else.

### 2. Name a concealment and paint/select its members

A new right-hand site noun — `Concealment` — beside `Monsters`, `Doors`,
`Policies`. It mirrors `IntelPanel`/`SitePolicies`:

- **List + form**: one entry per concealment with `id`, `checks` (required, a
  list), and `notice` (optional, same row shape). `New concealment`.
- **Cells**: painted with the walkable brush/rectangle gesture while the target
  concealment is active; owned cells render a distinct tint so "this is
  `vault`'s cells" is visible on the canvas.
- **Props**: a multi-select over placed items (placed prop/door ids — the same
  candidate list the Doors panel already reads), joined to `props: [...]`.

### 3. The checks/notice form

`checks` and `notice` are `[{ ability, dc, tool? }]` — byte-for-byte the
`RoomCheckApproach` shape a door's `locked` and a monster's `intimidate`/
`persuade` already carry. The form reuses that row editor; no third
transcription of the approach grammar.

`notice` is dice-free (resolved against a passive score, R6); that is a
rules-profile question the ruling itself names. The web CARRIES `notice` and
never grades it.

## The R7 intel coupling

R7 repurposes `reveals: { door }` into `reveals: { concealment }`. The builder
already refuses `reveals: { door }` by name today (`siteScope.ts`
`INTEL_REVEALS_DOOR_REFUSAL`, and `IntelPanel.tsx` disables the field). Authoring
the reveal is a small, precedented change, not a new surface:

- `SiteIntelReveals` widens to `{ fact } | { concealment }`, retiring `{ door }`.
- The record's "reveals a …" control gains a concealment-id picker offering the
  `concealments` map's ids.
- The door refusal sentence is replaced by concealment resolution. Because a
  concealment id is now a named thing the engine can resolve, the panel validates
  "names a real concealment" rather than refusing unconditionally as `door` does.

Intel NAMES the concealment; it never describes it. The concealment stays the
one noun.

## Validator split — web vs. engine

The refusal split is the discipline already on the page in `roomDraft.ts` and
`siteScope.ts` ("carried, not graded"; shape-only; one grammar, one owner).

**The web refuses only what it cannot represent** (`validateSiteConcealments`):

- an unknown-rejected key, a non-integral cell, a cell outside the workspace
  bound (`isCellWithinWorkspace`, the draft's own shape limit);
- a `prop` id no placed `scene.item` owns ("Declaration owner does not exist");
- `checks` missing or empty; a concealment that hides nothing;
- a cell or prop claimed by two concealments, refused naming both (R4).

**The engine judges the rest at `PutDungeon`, with a path and a sentence:**

- whether a `checks`/`notice` `ability` ref resolves;
- whether an intel `reveals.concealment` names a real concealment (R7);
- whether a cell is walkable (parent R2);
- R3's two public events and the per-observer atlas growth.

The publish panel already surfaces those sentences verbatim (`CONTRACT.md` §The
site scope, `validate_only`).

## Out of scope, named (deferred to the runtime wave)

Per the parent ruling, these are mechanisms, not presentation, and are not part
of the authoring slice. The authoring slice WRITES `cells`/`props` under a
concealment; it does not withhold them from an observer (that is the runtime
atlas, which the editor's preview mode only *imitates* against the same lists):

- **Per-observer atlas** — the API withholding cells/things per observer, and
  rooming an observer's atlas on reveal.
- **Public reveal beat** — every observer's atlas growing on R3's two events.
- **A prop leaving the field** — the bookcase swinging away. Also named because
  it is the first thing an author will ask for once they can author a
  concealment.
- **Cell adjacency (R5) and the `notice` passive tell (R6)** — runtime
  questions, not authoring. The authoring slice writes `cells` membership; it
  does not compute neighbour sets.

## Concrete landing list

1. `siteScope.ts` — `SiteConcealment` + `validateSiteConcealments` (representational
   refusals only; walkability is the engine's), joined to `SCOPE_KEYS` and
   `validateSiteScope`, "absent when none".
2. `singleRoomDungeon.ts` — `concealments` added to `ROOT_KEYS` and
   `carriesV4Keys`; no version bump.
3. `concealmentEdits.ts` + `ConcealmentPanel.tsx` — list/form on the reused
   `RoomCheckApproach` rows; placed-prop multi-select; cell membership via the
   paint gesture with an active concealment + tint; an "unaware observer"
   preview toggle reading the same `cells`/`props` lists and rendering them as
   wall / absent.
4. `IntelPanel.tsx` + `siteScope.ts` — `{ door }` → `{ concealment }`, a
   concealment-id picker, replace the unconditional door refusal with
   "names a real concealment".

## Ability vocabulary — answered

`notice`/`checks`' `ability` is the engine's sealed rulebook ref vocabulary,
exactly as `RoomCheckApproach` already uses for a door's `locked` and a
monster's `intimidate`/`persuade`. The picker's options and the engine's
sealed refs are one list; nothing looser.
