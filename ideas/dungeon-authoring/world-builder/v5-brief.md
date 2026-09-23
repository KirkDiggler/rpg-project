# v5 — a brief for a fresh look

**Status:** BRIEF, written 2026-09-24 at the end of a long session, **for a
session that did not live through it.** It carries the measured state and the
open questions. **It deliberately carries no recommendations.**

## Why this doc exists, and how to read it

Kirk, ending that session:

> "we have all kinds of mixup in your context. maybe we can create a v5 goals
> document with the way we see it could be done. we do not need to make decisions
> about how things are but I feel like we got into a bit of a mess here and I want
> a fresh look to see about getting it right. or something that will provide a
> solid foundation to build on."

**The mess was mostly in the session's context, not in the work.** Four times that
session took a real observation and inflated it into a decision the project owed:

| it found | it claimed | what was true |
|---|---|---|
| `place[].facing` refused on monsters | an expired scope fence needing a ruling | a deferral; fixing the comment was the whole job |
| one region, constant archetype/lighting | a "ceiling" needing three rulings | **no use case.** Kirk withdrew it |
| a v2 wall along a cell edge seals nothing | a live defect (rpg-toolkit#1894) | **v2 is being deleted** — it tested dead code |
| props block by centre, not coverage | v4 has the same bug | **its own test passed.** Crossing works |

**So this doc states facts that can be re-verified in minutes and asks questions
without answering them.** If a claim below cannot be re-verified, distrust it —
say so and correct it. Do not inherit a conclusion from here, including any the
last session would have given you.

**The single most useful discipline for the next session:** every claim in this
doc is checkable. Check the ones you are about to build on.

## 1. Where the code actually is (measured 2026-09-24)

- **v4 is the shape.** `dungeonspec.Load` routes `version >= 3` to the
  single-room decoder; the World Builder emits v4.
- **v2 is legacy and has no live user.** Its 5 shipped content files are the only
  production users and Kirk has ruled they are disposable (*"we do not need to
  migrate anything"*).
- **v4 has no `walls:` key.** Kirk's ruling is in the code
  (`single_room_doors.go`): *"walls are currently just props that block los and
  movement. Placing a door 'in' a wall no longer makes sense."* A wall is a
  placed prop with `blocksMovement`/`blocksLineOfSight`; a door is a prop plus a
  state.
- **Scenery is a v2 idea** (Kirk): it existed to blank the hexes under a wall when
  a room was concealed by withholding LOS. **v4 does not need it** — the
  concealment noun contains everything it hides (cells, props, doors).
- **The real reference document** is Kirk's own site,
  `~/Downloads/goblin-mind-test.yaml` (v4, 1234 lines). It is the shape to read
  before designing anything.

### What v4 authors today

`play` (`void`, `lighting`, `standing`), `factions`, `dispositions`, `intel`,
`exits`, `endings`, `scenarios`, `concealments`, and under `room.room`:
`implicitRegionId`, `walkableHexes`, `scene`, `propDeclarations`,
`arrangementDeclarations`, `monsters`, `monsterBindings`, `doorBindings`,
`propBindings`, `partyStart`.

## 2. Two constants answer the same question

Both of these decide *"how much of a cell counts"*, in the same repo, for
different nouns — and one is being deleted:

| where | constant | used by |
|---|---|---|
| `encounter/shape.go` | `CoverageThreshold = 0.5` — *"the tabletop's own rule"* | **area spells** (`MembersCovered`) |
| `dungeonspec/walls.go` | `MinStandable = 0.7` | **v2 wall sealing** (dying with v2) |

And there are **two primitives** for the same kind of question:

| primitive | answers | used by |
|---|---|---|
| `spatial.Coverage` | **a fraction per cell** | spells |
| `spatial.TraceFootprint` | two booleans (`Contact`, `Interior`) | placed props, doors |

`spatial.Coverage` **has one production caller** (`shape.go`) despite being the
richer primitive. Whether placed props should use it is **open** — see §4.

## 3. The site Kirk authored, as evidence

`goblin-mind-test.yaml`, measured:

- **61 scene items. 26 of them are the same wall asset**
  (`dnd5e:env:dark-fortress:45_wall_01`).
- Each wall carries its **own full `propDeclarations` entry** repeating
  `footprint: {width: 2.7868884801864624, depth: 0.2477882355451584}`.
- Walls are grouped under parent items labelled *"Repeated pieces"*.

Kirk, raising it:

> "I gotta imagine these props do not render all that great and efficiency is a
> thing we should be concerned with now that we can author pretty crazy sites"

and

> "even being back a shape and line to them"

**Read the file.** The cost is visible in it.

## 4. The open questions

**No answers here on purpose.** Each is a place the last session had a view, and
a view is exactly what this doc withholds.

### 4.1 What should a prop block, and which primitive decides?

Observed: props answer with `TraceFootprint` booleans; spells answer with
`Coverage` fractions thresholded at `CoverageThreshold`. Placement
(`ValidateStaticPlacements`) and standing (`CellAt`) both use **centre contact**,
so a thin wall lying along the boundary between two cells blocks **crossings**
correctly and blocks **neither** cell for standing or placement.

- Is that right (a boundary is in neither cell), or should a wall-like prop
  claim the cells it lies between?
- Should standing/placement ask the **crossing** fold, the **coverage fraction**,
  or both?
- Kirk's own 50% intuition — *"if we dont cover 50% it can be stepped on"* —
  already exists as `CoverageThreshold`. Is it the rule for props too?

Reproduced in-package (rpg-toolkit#1894): a member is accepted **standing under a
wall line** via `ValidateStaticPlacements`. Whether that is a defect or an
acceptable wart is **open**.

### 4.2 Should a wall be a noun again?

Walls-as-props **works** (Kirk: *"the rest of being a prop only is working so
far"*). The questions are where it breaks:

- **Rendering/efficiency**: 26 props for the wall run in one site. Is a run
  rendered as one thing the right shape, and does it matter now?
- **Authoring**: 26 declarations repeating one footprint, grouped under
  synthetic "Repeated pieces" parents.
- **The corner case**: Kirk, on the screenshot — *"that corner piece on the top
  left is tricky but if we dont cover 50% it can be stepped on."* A corner is two
  walls meeting; a corner cell is clipped from two sides at once.
- **Length**: Kirk — *"while it would be nice to change their length the rest of
  being a prop only is working so far."*

Kirk's framing of the standard: *"will it continue to work? is it a hard rule?
no. it is an idea that we are looking for seams where it breaks."*

### 4.3 What is the authored document's shape?

Carried over unresolved, and **each may be moot if 4.1/4.2 change**:

- **The dialect's name.** It is called single-room; Kirk finds the wording
  *"a little disturbing"* because sites have walls and doors. The name describes
  an implementation detail (one region) no author thinks in. **No use case for
  regions was found** — concealment covers hiding, and nothing needs per-area
  lighting or archetype.
- **`play.standing`, `play.void`, `play.lighting` are authored with exactly one
  legal value each** (`centre-covered`, `transparent`, `bright`), refused
  otherwise by name. Named policies with one member — extension points, or
  ceremony?
- **`intimidate`/`persuade`** live on a monster's binding for a capability Kirk
  has ruled combatants do not have, and the only user is a neutral NPC. Deleting
  them was proposed and never ruled.
- **`implicitRegionId`** is on the document. Honest while there is one region;
  what happens if that changes?
- **v2 removal** — a plan exists
  ([`v2-removal-plan.md`](./v2-removal-plan.md)) with two findings worth
  re-checking: the production files are **not** split by dialect (`grammar.go`,
  `predicate.go`, `factions.go`, `concealments.go` are shared and single-room
  calls into them), and `golden_test.go` pins the v2 tomb's **224-cell compiled
  atlas**, which makes porting the tomb a diff rather than a judgement call.

### 4.4 What is the test-room and content corpus?

Kirk, this session: *"The test dungeons we can keep it is the eight dungeons and
RPG API that is a bit much. We just need the reference room."*

- **Keep the toolkit's** test fixtures (13 under `dungeonspec/testdata/`).
- **Prune the API's** — 8 dungeons (5 under `content/`, 3 under
  `internal/dungeons/testdata/`).
- **The one surviving reference room**: *"the merchant in the front room, the two
  monsters in the second and the captain and the third."*

### 4.5 The named behavior table

Designed, not built: [`named-behavior-tables.md`](./named-behavior-tables.md).
A table declared at the site root and referenced by id, instead of pasted per
monster.

The finding it rests on is worth re-verifying: **the engine never learns a
table's name** (`behavior.Table` is a bare `map`; `tabledriver` rolls
`view.Table`; `compile.go`'s `ordersOf` is the one compile point), so a
reference resolves upstream and the compiled document is byte-identical. If that
holds, it is a web slice with no toolkit change.

## 5. Open items as of this doc

**rpg-toolkit**

- **#1894** — placement under a wall line. Focused, but see §4.1: whether it is a
  defect is open.
- **#1895** — a downed member still blocks its cell (a body in a doorway seals a
  room). `BlocksMovement` is an authored static fact and `opposed` is pure
  faction stance, so neither knows about being down. **Kirk's report; not
  reproduced.** Whether a body is passable is a **5e rules call** and rules text
  was not available in that session.
- **#1892 / #1893** — the `place[].facing` deferral comment fix (draft PR).

**rpg-project #498** — the docs from that session:
[`one-room-one-shape.md`](./one-room-one-shape.md) (v2 removal),
[`the-single-room-name.md`](./the-single-room-name.md),
[`v2-removal-plan.md`](./v2-removal-plan.md),
[`named-behavior-tables.md`](./named-behavior-tables.md).
**Read them with §1's discipline** — the last session wrote them and was
corrected twice on this branch.

**rpg-dnd5e-web #1199** — author the `when` condition's scope (`on: ally`,
`as: actor`). Not started; the engine side is merged and walkable.

## 6. Standing rulings from this session, quoted

Kept verbatim so a fresh session reads Kirk's words rather than a summary.

> "we do not need to migrate anything. we only have my example dungeon which I
> will use to understand the new shape"

> "we have way too many test rooms now. we only need the reference tomb."

> "The test dungeons we can keep it is the eight dungeons and RPG API that is a
> bit much. We just need the reference room."

> "scenery is a v2 idea … v4 does not need that … walls are just props in v4.
> while it would be nice to change their length the rest of being a prop only is
> working so far. will it continue to work? is it a hard rule? no. it is an idea
> that we are lookign for seams where it breaks."

> "i think we can call intimidate and persuade off for now"

> "I didn't think we need regions where? At least we don't have a use case for it
> yet. With our new concealment method where concealed contains everything that
> it's hiding, I don't have a use case for regions."

> "Another thing to file is a downed monster still blocks the path and if they go
> down in a door you don't get in"

> "we have a bug where we let people walk onto the space where a wall is and they
> can effectively see through it … On the other side we have that same bug on
> doors. I can stand on the spot with the door without being forced to open it
> but I believe that's in the engine"

> "I gotta imagine these props do not render all that great and efficiency is a
> thing we should be concerned with now that we can author pretty crazy sites"

— rpg-toolkit agent, on behalf of KirkDiggler