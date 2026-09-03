# Wall geometry — plan

**Status:** agent handoff, 2026-09-03. Honest to `wall-geometry-design.md`
as ruled (Kirk: *"i think we are good… let's continue"*). Where the plan
found a gap in the design it says so in §0 and the design is amended in
the same commit.
**Journey:** rpg-project#169. Slice issues filed at branch cut, one per repo
per slice, each a sub-issue of #169 and a Project 19 item.
**Stance:** pre-release, no game running on this, optimized for flexibility.
Build what the design says and nothing it does not; refuse only hazards;
show costs; leave the shelves empty.

## 0. Gaps found while planning — design amended

- **The wire must say which cells nobody can stand on.** Slice 1 gets away
  without it (scenery = a cell in `cells` and in no region). Slice 2 cannot:
  a cell a wall seals keeps its region, so region membership no longer
  implies standable. `GetAtlasResponse` gains `repeated Position sealed`,
  additive, in slice 2's proto. Design §5.2 amended.
- **Geometry lives in the authoring compiler and nowhere else.** Every
  derived fact the runtime or the projection needs — a wall's footprint,
  the cells it seals, a door's crossing — is computed once in `dungeonspec`
  and carried in `FieldInput`. The runtime never embeds a hex. Design §4.2
  C9 already says this; the plan holds the line by giving `FieldInput`
  the fields to carry it.

## 1. Slice 1 — scenery floor

Order: toolkit → tag → rpg-api pin → web. Web builds in parallel against a
branch api. Kirk walks once; merge bottom-up.

### 1.1 rpg-toolkit — module `rulebooks/dnd5e/encounter` (one PR)

`dungeonspec`:
- `Spec.Scenery` — rows of `[col,row]`, same decode as `RegionSpec.Cells`,
  emitted after `regions`.
- Validate: F1 (no cell in both scenery and a region; no duplicate),
  F2 (monster refs and `start` not on scenery; prop refs allowed).
- Validate C2: wall and door edges may stand on scenery.
- Validate C4: the concealment walk. Rewrite `ways` as a flood: from each
  region cell, follow non-wall crossings through scenery cells (only
  scenery — never through another region's cells) until a cell of another
  region is reached; a way is concealed iff **any** crossing along it is a
  concealed door (the flood stops at walls and concealed doors; the same
  answer from either end). *Ruled 2026-09-03: the earlier "first crossing
  out of the origin region" depended on direction and refused a secret room
  whose own door is the concealed one.* Refusal names the scenery cell the
  path enters and the room. Visible-reach flood extends through scenery the
  same way. Tests beyond A2/A3: the concealed door on the visible room's
  edge (legal, A2's yardstick); a wall standing inside the strip (not a
  way; legal); a scenery area touching two visible rooms and one hidden
  room, refused through whichever visible room's way is bare.
- Compile: `FieldInput.Scenery []Cell` (new).

`encounter` runtime:
- `compileField`: `scenery` set; `cells` = owner keys ∪ scenery; `owner`
  unchanged; `standable(cell)` = present in `owner`. `stepMember`
  unchanged in behaviour (already refuses ownerless).
- Props: `compileField` accepts a prop on a scenery cell. Monsters and
  seats refuse (already: not owned).
- Sight: wherever LOS classifies a cell as void, a scenery cell is floor
  (transparent) regardless of `Void`. Find the classification in the sight
  seam and the square/hex LOS helpers; add the case; pin with A5.
- Projection C5/C6: `hiddenFrom` unchanged; masquerade pass 3 skips a
  crossing whose visible-side cell is scenery.
- Atlas: `Cells` includes scenery; `Regions` unchanged.
- Persistence: `Scenery` rides `EncounterData` like regions do; Load →
  same `compileField`.

Tests (each able to fail; name the scene, assert the cell):
- A1: every `rpg-deployment/content/*.yaml` and `reference-tomb.yaml`
  compiles to a byte-identical `Compiled` vs a committed golden.
- A2: yardstick twin — non-knower `AtlasFor` of "secret behind a scenery
  strip" equals `AtlasFor` of the twin with the secret region deleted.
- A3: visible room · scenery · hidden room · no wall → refusal naming the
  scenery cell; add the wall → compiles.
- A4: monster on scenery refused by name; prop on scenery compiles and is in
  the atlas.
- A5: step onto scenery refused; two members across a scenery gap see each
  other (`Void: opaque` set, to prove scenery is transparent on its own).
- Mutation pass on C4 (drop the scenery hop) and on the sight case (drop
  the scenery branch): A3 and A5 must go red.

### 1.2 rpg-api

- Pin the encounter tag. No handler change: `PutDungeon` ships the YAML
  verbatim; `GetAtlas` translates `Cells`/`Regions` as today.

### 1.3 rpg-dnd5e-web — builder

- `DungeonDoc.scenery: Axial[]`; parse/emit (`dungeonYaml.ts`), emitted
  after regions.
- Brush 2.1/2.2: a "Scenery" entry beside the room brush; paint moves a
  cell out of its region; painting a room over scenery moves it in; erase →
  void with today's cascade.
- 2D: scenery hatch. 3D preview: no change (server atlas tiles all cells).
- 2.4: `placeAt`/`setStart` refuse monster/start on scenery in place with
  the reason; props allowed.
- 2.5: compiler refusals that name a cell highlight it (the existing
  error-to-cell path; extend the coordinate parser if the message shape is
  new).
- **Concealment mirror (found during the build, 2026-09-03).** The builder
  carries a client-side mirror of the concealment walk (`deriveConcealment`
  / `buildRegionGraph`, from rpg-dnd5e-web#893) that ratchets
  `concealed: true` onto regions from the door graph. It walks region-cell
  to region-cell crossings only, so a scenery strip would make two joined
  rooms read as disconnected and the ratchet would conceal a room the
  server can walk into (which A3 then refuses). Extend the graph to flood
  through scenery exactly as design C4 does — interior cells all scenery,
  never a third region's cells; two regions are joined iff some way between
  them has no concealed door on any crossing (ruled 2026-09-03; the earlier
  "first crossing out of the origin region" depended on direction and the
  mirror must agree with the server). Test scenes shaped like toolkit A3 (joined through
  scenery: connected; wall added: separated). Keeping an existing
  derivation true is in scope; the plan was silent only because the mirror
  was not known when it was written.
- Tests: parse/emit round-trip with scenery; brush state transitions;
  placement refusals; screenshot of the strip behind a wall in 3D preview.

## 2. Slice 2 — walls as lines

Order: protos → toolkit (dungeonspec + encounter) → rpg-api → web. Web and
api build in parallel against pinned branches. Content rewritten before the
pair form is deleted from anything that boots. Kirk walks once; merge
bottom-up.

### 2.1 rpg-api-protos — `dnd5e/api/session/v1alpha1/service.proto`

- `message AxialPoint { double q = 1; double r = 2; }`
- `message AtlasSegment { AxialPoint from = 1; AxialPoint to = 2; double height = 3; }`
- `GetAtlasResponse`: `repeated AtlasSegment segments = 10;`
  `repeated Position sealed = 11;`
- `AtlasProp` offset comment: bounding-box fractions (x widths east, y
  heights south), no field change.
- Evidence: buf lint/format/breaking, generate compiles. No hand tests.

### 2.2 rpg-toolkit — module `rulebooks/dnd5e/encounter` (one PR)

`dungeonspec` — the dialect:
- `WallSpec` becomes `{start, end, height, name}` with `start`/`end` as
  `PositionSpec{Cell [2]int; Offset [2]float64}`. The pair form is
  **deleted**: a `walls[]` entry carrying `edges` is refused at the header
  naming the form (F4).
- `DoorSpec.At PositionSpec` replaces `Edges` (F12).
- Positions: the closed set per orientation (§3.3, seven). F8 refusal by
  name. Compare as exact floats (all dyadic).
- Direction F13: `atan2` of end−start in the embedding, snapped to 30°
  with a tolerance of 1e-9 rad, else refused naming the angle.
- `geometry.go` (new, package-private): centre, corners, bounding box of
  an axial cell under orientation, circumradius 1; position → point;
  segment/segment intersection; Sutherland–Hodgman half-plane clip;
  polygon area. **This is the toolkit's first world-unit geometry.** A
  doc comment names `tools/spatial/room.go`'s corner-rule sight test as the
  second customer and says promotion to `tools/spatial` waits for it.
- Derivations per wall: crossings C7, footprint C8 (floor cells only),
  standability C10 with `const MinStandable = 0.7` in one place, sealed
  cells = footprint cells below it (owner kept). C2 line form: refuse a wall
  whose footprint is empty.
- Doors C14: the crossing of the side the position is the midpoint of;
  F10 exactly-one-wall; F11a a door between two sealed cells is legal.
- C12: monster/start on a sealed cell refused naming the wall.
- Compile → `FieldInput`: `Walls []WallInput` (pairs, as today — the
  runtime's mechanical truth), `Segments []SegmentInput{From, To AxialPointF;
  Height; Footprint []Cell; DoorIDs}`, `Sealed []Cell` (owned, unstandable),
  `Scenery` from slice 1. Doorways as today.

`encounter` runtime:
- `compileField`: `standable(cell)` = in `owner` and not in `sealed`.
  `stepMember` uses it. `cells` = owner ∪ scenery.
- Sight: a sealed cell is floor for classification; the wall through it
  blocks as a boundary already does.
- Projection: C18 footing — for every segment presented to the recipient,
  its footprint cells enter the recipient's atlas as ownerless floor even if
  their owner is hidden; C19 — a concealed door in a segment presents the
  whole segment with no gap; `maskHeight` reads `Segment.Height` and the
  run reconstruction is deleted.
- Atlas: `Segments`, `Sealed` (per recipient), `Cells` incl. footing.
- Persistence: segments and sealed ride `EncounterData`; Load recompiles
  standability from them, never from geometry.

Content and fixtures in the same PR:
- Every dungeonspec/encounter/session test fixture in the pair form is
  rewritten in the line form. A throwaway converter (job tmp, not
  committed) maps each pair run to the nearest legal line: the run's first
  and last crossings → the thin line through their midpoints when one
  exists, else the thick line; output reviewed by eye on the board before
  commit. The converter is deleted with the pair form.
- `rpg-api/internal/sessionworld/reference-tomb.yaml` rewritten the same
  way (lands in the api PR, pinned to this tag).

Tests:
- A6: the rewritten tomb compiles to the same `cells` and the same blocked
  crossings as the committed pre-rewrite golden, and each door opens the
  same crossing.
- A7: the 6×6 room → four segments, 46 blocked crossings.
- A8: quarter line and midpoint line leave every adjacent cell standable at
  0.7; a flat-side line seals exactly the odd-row cells on it; a square
  inside corner standable; a hexagonal corner sealed.
- A9: each refusal in the design's list, one scene each, message names the
  thing.
- A10: concealed door in a segment — non-knower atlas: full segment, no
  doorway, footing present; knower: gap. Twin identical.
- A11: hugging layout with a quarter line — non-knower sees floor both
  sides.
- A12: corners: thin at a midpoint (60/90/120), thick at a centre, thin to
  thick at a flat-side midpoint; each closes on the board (two segments
  sharing an endpoint exactly).
- Mutation pass: flip `MinStandable` to 0.5 and to 0.8 — A8 must go red
  both ways; drop the footing pass — A10 must go red.

### 2.3 rpg-api

- Pin protos + encounter (+ session if it re-exports the atlas). Translate
  `Segments` and `Sealed` in the atlas handler. Rewritten
  `reference-tomb.yaml` lands here (A13 walk by Kirk).
- `rpg-deployment/content`: the eight legacy files are deleted; Kirk
  re-authors what he wants in the new builder. Note on the deployment PR:
  the content volume must be emptied on the box or the api refuses to boot
  on the first legacy file it loads (rpg-api#886 is the quarantine
  alternative — not built here).

### 2.4 rpg-dnd5e-web

Builder (`src/author/`):
- `WallDoc = {start, end, height?, name?}`; `DoorDoc.at`; parse/emit;
  positions table per orientation; pair-form documents refused at load
  with the compiler's message.
- Picker 2.6/2.7: click a hex → its seven positions; pick start; the
  designer draws the twelve rays from it, each trimmed to positions that
  make a legal end, coloured by what the line seals (grey cells previewed)
  and thin/thick told apart; pick end. Joining by picking an existing end
  copies its position. No angle snap, no freehand.
- 2.8: door = click a midpoint on a wall.
- 2.9 "wall this boundary": one gesture along a room boundary picks the
  thin line's ends; a toggle picks the thick one.
- Live cost: from the server compile (`PutDungeon validate_only` already
  round-trips at 400 ms); `sealed` cells hatch. No local mirror.
- `wallGesture.ts`'s `tautPath`/angle snap, `boardWallRuns.ts`
  `boardWallScene`, `src/hooks/authoredWallRuns.ts` (`CHAIN_TOLERANCE`),
  and the run-chaining in `atlasWallRuns.ts` are **deleted**; the board and
  the game draw `segments`.
- Prop offset: `atlasToScene3D.ts` multiplies by bounding-box width/height
  instead of `hexSize`; the 2D board likewise.

Game route (`src/components/session/`):
- Walls render from `segments` (straight meshes, door gaps from doorways
  projected onto the segment); masquerade segments look the same.
- Footing cells tile as floor (they arrive in `cells`).

Tests: parse/emit round-trip; picker offers exactly the design's rays and
ends for a slanted midpoint, a flat-side midpoint, a centre; corner join
writes identical positions; screenshot pair: the tomb before (pair form,
fitted) and after (segments) look the same; a prop at offset `[0.5, 0]`
sits on the side, not inside.

## 3. Walk

Branch api on `:50051`, branch web on `:3003`, fresh lobby after an image
swap. Kirk authors one room with thin walls and one with thick, a door on
each kind, a scenery strip behind a secret, and plays it in multiplayer.
The tell (black slivers / masquerade outline) must be gone in both maps.

## 4. Merge order

Slice 1: toolkit → tag → api → web.
Slice 2: protos → tag → toolkit → tag → api (with the tomb) → web.
`Closes #N` is inert on dev-based repos; close by hand. Never `git add -A`
in rpg-project.

## 5. Not built (shelves, from the design §8)

Room templates · "walkable anyway" · more positions · multi-prop per hex ·
cliff edges · transparent-void sight gap (file on toolkit at branch cut) ·
a designer-local standability mirror.

— cross-team agent, on behalf of KirkDiggler
