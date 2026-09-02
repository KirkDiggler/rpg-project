# Wall grouping — walls are authored runs, not loose edges (rpg-project#355)

**Status:** design, ruled by Kirk 2026-09-02 during the walk
**Journey:** rpg-project#326 · **Issue:** rpg-project#355
**Revisits:** `wall-height.md`'s #273 ruling ("runs are DERIVED") — scoped, not overturned; see *What #273 got right*.

## Problem

Walls are hex-by-hex at every layer. A dungeon the author thinks of as **8
walls** is written as **153 edge entries**, each repeating the run's height.
Kirk, unable to fix a dungeon by hand: *"tech we only have 8 walls total but
that gui generates those."*

Three consequences, all observed rather than hypothesised:

- The author cannot find anything. Seven validation errors naming coordinates,
  and no way to locate the corresponding entries.
- **Whether a wall is purely in concealed space is now load-bearing.**
  rpg-toolkit#1419 withholds a wall wholly inside hidden space and presents one
  with a visible endpoint, so the question has real consequences — and a flat
  list of 153 pairs cannot answer it.
- Height is repeated per edge when it is a property of the run.

## Why the grouping cannot be derived — measured, not argued

Three candidate rules were run against `concealed-room.yaml` (153 wall edges,
the dungeon in Kirk's screenshot). Kirk's target is **8** — or 12 if a door
splits a run.

| rule | as authored | doors filled in |
|---|---|---|
| break at every junction (topological) | 19 (→ 9 after corner snapping) | 15 (→ **9**) |
| pass through T junctions, break on sharp turns | 11 | **7** |
| renderer's `CHAIN_TOLERANCE` straightness test | *presentation — see below* | |

**Neither bracket reaches 8, and the measurement says why.** Every one of the
**134 degree-2 corners in the file turns exactly 60°** — a room corner and a
zigzag step are the *same angle* on a hex grid. No local rule separates them.
Breaking at every junction over-splits (the tan room's divider tees into the
region-1/region-2 boundary and splits it); following through the T under-splits
(absorbing the T also absorbs a room corner, since both are 60°).

The only thing that *can* separate a corner from a zigzag is a **non-local**
straightness test in world units — `authoredWallRuns.ts`'s
`CHAIN_TOLERANCE = HEX_SIZE * 1.5`, whose own doc comment records the same
finding: *"a nominally-straight side can cycle through more than 2 of the grid's
3 edge orientations over its length without ever actually bending."* That
constant exists to tile wall meshes. Letting it group the authored file would
let a **rendering tolerance rewrite every dungeon on disk** — the inverse of
*presentation never decides mechanics*.

**Therefore the grouping is authorial and must be stored.** "The boundary is one
wall even though the divider tees into it" is intent, not topology.

Two further points settle it:

- **Storage is independent of geometry quality.** Eight strokes give eight
  groups whether or not the walls came out square. A derived grouping is
  hostage to exactly the slant and corner-snapping the builder has yet to fix
  — the geometry would have to be repaired *before* the file could read
  correctly. Stored, Kirk gets his 8 today, on the broken file.
- **The corner spurs are not sacred.** #355 recorded Kirk's deliberate corner
  overshoot as a thing grouping must not tidy away. He corrected this on the
  walk: the spurs exist to pull the corners square, *"that need goes away."*
  Five single dangling diagonals plus one connector — they disappear with
  corner snapping, and grouping owes them nothing.

## The ruling

### 1. Wall entries become groups

```yaml
walls:
  - name: north wall            # optional, carried unread
    height: 2                   # optional, the run's height, written ONCE
    edges:
      - [[5,0],[6,0]]
      - [[6,0],[7,0]]
  - [[9,4],[9,5]]               # bare pair — still legal, a run of one
  - { between: [[5,1],[6,1]], height: 2 }   # still legal, a run of one
```

Nothing on disk is forced to migrate: an ungrouped edge is a run of one, and
both existing forms keep parsing.

`name` is for the audience. Per `dungeon-builder-audience`, the users are
streamers, not engineers — an error that says *"north wall"* beats one that says
`walls[7]`.

### 2. A group carries no mechanical meaning

The compile flattens groups to the same `[]WallInput` it produces today. **The
same edge set grouped differently compiles byte-identically.** Grouping is an
authoring and reading concern with zero mechanical consequence, so a
hand-grouped file can never be *wrong* — contiguity is not validated, because a
validation error the author cannot act on is worse than no feature.

### 3. A door may stand in a wall

Today `validate.go` refuses it: *"this edge is also a wall (walls[i]), and a
door cannot stand in a wall."* That rule is why a run comes apart into pieces
and why Kirk's 8 reads as 12. Kirk on the walk: *"if a door can exist on a wall
at a location then it is even easier to read. we are back to the dream of
simply having 8."*

- The wall group **lists** the edge; the door **sits on** it.
- Compile **subtracts** door edges from the wall set, so the engine still sees
  walls and doors disjoint and the compiled output is identical to what the
  same intent produces today by omitting the edge.
- A door on an edge with no wall stays legal (a doorway in open space).
- **Deleting a door restores the wall underneath** instead of leaving a gap —
  the behaviour the current model gets wrong.

### What #273 got right, and survives

`wall-height.md` ruled that the **edge** carries the height, not "some named
run," because *"runs are DERIVED — order-free, re-derived under editing — and
giving them stored identity would re-open everything the wall engine's
order-invariance closed."*

The run it meant is the **renderer's** run, and that stays derived. The stored
group is never read by the renderer: the file flattens to edges before anything
draws it, and `computeAuthoredWallRuns` derives its own runs from those edges
exactly as today. Order-invariance is untouched. What #273 was protecting —
**the edge is the one stable unit of mechanical fact** — is preserved verbatim
by ruling 2.

The ruling is narrowed, not overturned: *a stored group has no mechanical
consequence and no renderer reads it.*

## The stack

| layer | change |
|---|---|
| **toolkit `dungeonspec`** | parse the grouped form; flatten in `wallsOf`; reverse the door-in-wall refusal; subtract door edges in compile |
| **rpg-api** | toolkit pin bump only |
| **web** | builder stores the stroke as the group; emit grouped; parse grouped back to its flat internal model |

**No protos.** Grouping never reaches the wire — the atlas carries derived
boundaries, unchanged.

**Ordering constraint:** web must not emit grouped files until the *deployed*
api can parse them, or authored dungeons stop compiling.

## Tests

- **Grouping is inert:** the same edge set, grouped three different ways,
  compiles to byte-identical `[]WallInput`. This is the ruling-2 pin.
- Both legacy forms still parse; a mixed file (bare pair + object + group)
  parses; `walls[3].edges[7]` is the error path for a grouped entry.
- **A door standing in a wall compiles**, and the compiled wall set excludes
  the door's edge — with a fixture where the wall group genuinely lists it, so
  the subtraction can be observed (`tests-that-cannot-fail`: the fixture must
  contain the contrast).
- A door on a wall-free edge still compiles, unchanged.
- Height is read once per group and applies to every edge in it.
- Round-trip: `lg-dungeon.yaml` grouped by the builder → compile → atlas
  identical to the flat file's atlas today.

## Not now

- **Wall snapping / squaring.** Kirk: *"the walls are not meant to be angled."*
  Filed separately; grouping deliberately does not depend on it.
- **Per-group offset.** Kirk floated it as the thing that makes snapping
  *"mostly unnecessary."* It lands as a field on the group entry once grouping
  exists — no new mechanism needed.
- **Door height** inside a raised wall run (already shelved by #273).
- **Surfacing a group's concealment status** (pure-visible / straddles the
  frontier / pure-concealed) in the builder. Measured on `concealed-room.yaml`:
  14 runs pure-visible, **5 straddle the frontier**, 0 pure-concealed — so the
  straddling case is the real one, and #355's worry that grouping might assume
  one status per run is well-founded. Builder affordance, not file format.

— cross-team agent, on behalf of KirkDiggler

---

## Implementation (shipped 2026-09-02)

| repo | change | version |
|---|---|---|
| **rpg-toolkit** #1429 / PR #1430 | `WallSpec` is a run; grouping inert; a door stands in a wall | `rulebooks/dnd5e/encounter/v0.47.0` |
| **rpg-api** #893 / PR #894 | pin bump only, no source change | — |
| **rpg-dnd5e-web** #900 / PR #901 | the builder keeps the stroke; `compiledWalls()` | — |

Merged in that order, with the api **deployed** before web — rpg-api compiles
every dungeon on `Put`, so a grouped file reaching the old server is a save
failure, not a degraded render.

### The evidence that mattered

- **The bump was load-bearing, not ceremonial.** The same grouped dungeon
  compiled against each pin: `v0.45.1` → `bad dungeon spec: field name not
  found in type dungeonspec.WallSpec`; `v0.47.0` → clean.
- **Grouping is inert**, pinned by regrouping the reference tomb into 1, 2, 5
  and 28 runs and getting an identical wall set every time.
- **The door subtraction is observable** because the tomb's two 14-edge lines
  run straight past the cells its doors occupy: the spec carries 30 wall edges
  and the atlas gets 28. Mutating the subtraction to a no-op killed exactly
  that test and nothing else.
- **No churn**: all 8 dungeons in `rpg-deployment/content` emit byte-identically
  to the pre-grouping emitter, checked by importing the `origin/dev` module
  side by side.

### What we learned that would change the design

- **`Validate` running `walls()` before `doors()` became load-bearing.** The
  coherence check skips crossings a wall claims ("a wall is not a way in"), so
  once a run may name a door's edge, which claim wins decides whether a secret
  room's only entrance is seen at all. Swapping the two breaks ten tests. This
  was incidental ordering before this slice and is now a rule; it is documented
  at both sites rather than left to the reader.
- **Existing dungeons stay flat, and that was not called out up front.** A
  group records a stroke, and files already on disk have none left —
  `lg-dungeon.yaml` stays 153 entries until its walls are redrawn. The design
  argued correctly that the grouping cannot be *derived*, then quietly assumed
  files would arrive grouped. They will not. Redrawing is the path (one drag
  can now cross a doorway), and it is worth saying so wherever the feature is
  described.
- **A "sacred" constraint dissolved on contact with the author.** #355 recorded
  Kirk's corner overshoots as something grouping must never tidy away, and the
  design was ready to protect them. He corrected it in one line — they exist to
  pull corners square, *"that need goes away."* The lesson is the one already
  written down: test a constraint against the person who created it before
  designing around it.
- **`npm run ci-check` in rpg-dnd5e-web is fail-open.** It printed "Some CI
  checks failed" for formatting and exited `0`. Read its output, never its exit
  code — worth a separate fix.

### Still open, deliberately

Wall snapping and squaring (*"the walls are not meant to be angled"*), the
per-group offset, and surfacing a run's concealment status in the builder. All
three land on top of runs now that runs exist. Measured on
`concealed-room.yaml`: 14 runs pure-visible, **5 straddle the frontier**, none
purely concealed — so the straddling case is the real one.

— cross-team agent, on behalf of KirkDiggler
