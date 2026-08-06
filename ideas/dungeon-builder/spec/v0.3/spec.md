# Dungeon YAML Spec v0.3

**Status:** RATIFIED v0.3, 2026-08-05.
**Normative.** See `README.md` for context, rationale, and the ratification record.

## 1. Level cut

| Group | Constructs | Status |
|---|---|---|
| (a) Already-v1 chain | `rooms:` (id/archetype/width), `connectors:` (+ `locked:`), room-scoped `place:`/`boss:`/`obstacles:` | real, `dungeonspec.Validate`/`PutDungeon`, unchanged |
| (b) Already-compiling | `walls:` edge lists, `start:`, room-scoped floor-prop `facing:` | live-verified (rpg-api#769/rpg-toolkit#881) |
| (c) Wave 0 / #192 | `canvas: {width,height}`, mode-branched validation, top-level `place:` (canvas mode only), `start:` on canvas floor, `FloorPlan` canvas echo, shrink-validation | LIVE VERIFIED |
| (d) Wave 1 / #180 | `regions:` (`id`, `name?`, `archetype`, `cells`), full scopes semantics | implementation not started |

Per-construct specs: §4.1–§4.10. Constructs not listed in this table or §2 are out of
scope for v0.3 by omission. This table states construct membership only — delivery
order follows `plan.md`'s two-wave structure (Wave 0 prerequisite, then Wave 1 =
#180), unchanged by this spec.

All six ratification points from the pre-ratification draft are resolved and stated
below as normative rules — no OPEN markers remain in this document. See
`README.md`'s Ratification record for the decision history.

## 2. Explicitly ABOVE v0.3

| Construct | Pointer |
|---|---|
| `wallLines:` (straight walls) | unfiled |
| `defaults:` (ref-keyed defaults + materialize-on-strip) | unfiled |
| `holes:` | deferred, not queued |
| `end:` | rpg-project#186 |
| `orientation:` | rpg-project#187 |
| `mount:` + `height:` (z-axis) | rpg-project#188 |
| `lighting:` | rpg-project#190 |
| `targeting:` | rpg-project#191 |
| `rotate_degrees:` | experiment only, not a dialect candidate |

### Rejection classes

Every construct in §2 is rejected by exactly one of three mechanisms:

| Class | Mechanism | Example |
|---|---|---|
| 1 | Decode-known; `Validate` rejects at a field path | `mount:` — `PlacedEntry.Mount` decodes (`dungeonspec/spec.go:86`); rejected by path (`validate.go:351-353`) |
| 2 | Not in schema; whole-document decode failure | `height:` on a placement — not a `PlacedEntry` field |
| 3 | Decode-known; `Validate` accepts | every construct in §4 |

§4.9's requirement that `mount: wall` fail with a field-path error (not a
whole-document decode failure) depends on `mount:` being class 1, not class 2.

## 3. Ground rules

1. Decode is strict and single-document: `KnownFields(true)`; an unrecognized
   top-level field MUST fail the whole document; multi-document input MUST be
   rejected.
2. `version:` MUST equal `1`. Any other value MUST be rejected.
3. Every cell coordinate (`walls:`, `start:`, `place[].at`, `regions[].cells`) is
   absolute pointy-top odd-q offset `[column,row]` — the same space a compiled
   `FloorPlan` uses.
4. Physical geometry (space/canvas/floor, canonical wall/door edges) and semantic
   scope (regions, §4.10) are independent axes. Neither derives the other.

## 4. Per-construct specification

### 4.1 Document metadata

```yaml
version: 1
key: shrine-hall
name: 'The Shrine Hall'
theme: crypt
height: 8
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `version` | int | yes | MUST equal `1` |
| `key` | string | yes | `[a-z0-9-]+`; MUST match the request key |
| `name` | string | yes | non-empty |
| `theme` | string | no | — |
| `height` | int | decode-optional (missing → `0`) | room-chain mode: `Validate` requires `>= 4`. canvas mode: unvalidated and unused — `canvas.height` (§4.5) is authoritative; a value below `4` MUST NOT be rejected and MUST NOT affect canvas geometry |

### 4.2 Room chain — `rooms:`

```yaml
rooms:
  - id: antechamber
    archetype: entrance   # entrance | chamber | corridor | boss
    width: 6
```

1. `rooms:` MUST contain at least 2 entries.
2. `start_column[i] = start_column[i-1] + width[i-1] + 1` (server-computed; the `+1`
   reserves a connector-gap column).
3. Exactly one room MUST have `archetype: boss` and MUST declare `boss:`. No other
   room MAY declare `boss:`.
4. A document combining non-empty `rooms:` with `regions:` is rejected (§4.10.3.6).
   Region archetypes add no boss-cardinality, entrance, or spawn-rule enforcement
   to the room-chain mode.

### 4.3 Connectors — `connectors:`

```yaml
connectors:
  - { from: antechamber, to: shrine }
  - { from: shrine, to: vault, locked: { dc: 12, ability: dex } }
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `from` | room id | yes | MUST equal `rooms[i].id` for connector index `i` |
| `to` | room id | yes | MUST equal `rooms[i+1].id` for connector index `i` |
| `locked` | object | no | `{dc, ability}`; omitted = unlocked |

1. `connectors:` MUST contain exactly `len(rooms) - 1` entries.
2. `from`/`to` are not independently authorable — only `locked:` varies per entry.
3. A region-attachment door (§4.10.2) MUST NOT satisfy, replace, or count toward
   this requirement.

### 4.4 Room-scoped placement — `place:` / `boss:` / `obstacles:`

```yaml
rooms:
  - id: hall
    obstacles:
      - { ref: 'dnd5e:props:bone-pile', count: 3 }
    place:
      - { ref: 'dnd5e:props:brazier', at: [1, 1], blocks_movement: true, blocks_los: false }
    boss:
      ref: 'dnd5e:monsters:skeleton-captain'
      at: [4, 3]
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `place[].ref` / `boss.ref` | string | yes | `dnd5e:props:*` or `dnd5e:monsters:*` |
| `place[].at` / `boss.at` | `[int,int]` | yes | room-local `[col,row]`; MUST NOT be the door row |
| `place[].blocks_movement` | bool | no | props only; MUST be rejected on a monster ref |
| `place[].blocks_los` | bool | no | props only; MUST be rejected on a monster ref |
| `place[].facing` | enum | no | see §4.9 for acceptance scope |
| `obstacles[].ref` | string | yes | — |
| `obstacles[].count` | int | yes | `>= 1` |

1. `at` resolves to an absolute cell by adding the room's `start_column` (§4.2.2).
2. The door row (`row == height/2`) MUST be rejected for `place[].at` and `boss.at`.
3. Exactly one room MUST declare `boss:` (§4.2.3); there is no top-level `boss:` in
   v0.3 (§4.6.2).

### 4.5 Canvas — `canvas:` (rpg-project#192, Wave 0)

```yaml
canvas:
  width: 20   # positive integer; columns [0, width)
  height: 30  # positive integer; rows [0, height)
rooms: []
```

1. A document is in exactly one of two floor-source modes: **room-chain mode**
   (`rooms:` non-empty) or **canvas mode** (`canvas:` present, `rooms: []`).
2. A document combining non-empty `rooms:` with `canvas:` MUST be rejected — a
   document declares exactly one floor-source mode, never both.
3. In canvas mode, the following checks MUST be skipped entirely: the `height >= 4`
   check; the `len(rooms) >= 2` check; `connectors:` count-equals-`len(rooms)-1`;
   boss cardinality; boss-axis; the M1 monster/boss.at restriction; boss-ref
   validation. These seven MUST be skipped together — skipping boss cardinality
   without the remaining boss-cluster checks produces undefined behavior against a
   nil room reference.
4. In canvas mode, static validation of `walls:` (§4.7) and `start:` (§4.8) MUST use
   the canvas-derived floor (`[0,width) × [0,height)`) as their floor source, not the
   room-chain floor.
5. Independently of rule 4: persisted authored-edge validation at `InitDungeon` time
   (the runtime-side check behind §4.7's endpoint-floor-membership requirement) MUST
   ALSO use a canvas-derived floor source in canvas mode. This is a second,
   separately-implemented check — extending the static path (rule 4) does not
   extend this one.
6. In-bounds checks otherwise run against `canvas.width`/`canvas.height`.
7. Top-level `place:` (§4.6) MUST be accepted in canvas mode and MUST remain
   rejected in room-chain mode.
8. `start:` (§4.8) resolves against the canvas floor in canvas mode.
9. `FloorPlan` MUST echo `rooms: []` plus `canvas.width`/`canvas.height`, with
   authored edges present as usual, and MUST also project the canonical
   structural-floor cells derived from the canvas dimensions.
10. Growing `width`/`height` MUST be accepted unconditionally. Shrinking below any
    existing placement, wall endpoint, `start:`, or region cell MUST be rejected with
    a named, specific error — never a silent drop.
11. Non-goals: no semantic-region/zone requirement (a canvas document with no
    `regions:` treats its whole floor as the implicit root scope, §4.10.2.1); no
    boss/monster content requirement; no locked-door grammar/binding decision beyond
    §4.7.6.

**Acceptance criteria:**
- `PutDungeon(validate_only)` against `canvas: {width,height}`, `rooms: []` decodes
  successfully and returns a compiled `FloorPlan`.
- An in-bounds canvas document validates successfully; every room-chain-only rule is
  skipped; out-of-bounds content fails clearly against canvas dimensions.
- `FloorPlan` for a canvas document carries the projected canonical structural-floor
  cell set, not dimensions alone.
- A canvas document with top-level `height: 1` validates successfully.
- A document declaring both non-empty `rooms:` and `canvas:` is rejected.
- Shrinking `width`/`height` below existing content produces a named, specific
  validation error, never a silent drop.

### 4.6 Top-level placement — `place:` (canvas mode)

```yaml
canvas: { width: 20, height: 30 }
rooms: []
place:
  - { ref: 'dnd5e:props:pillar', at: [10, 15] }
  - { ref: 'dnd5e:props:altar', at: [8, 8], facing: W }
  - { ref: 'dnd5e:monsters:skeleton-captain', at: [5, 18] }
```

Same field table as §4.4's `place[]`, except `at` is unconditionally absolute
(no room `start_column` offset) and there is no owning room.

1. MUST be accepted in canvas mode; MUST remain rejected in room-chain mode.
2. There is no top-level `boss:` in v0.3 — `boss:` remains room-scoped only (§4.4.3).
3. `facing:` on a non-monster, `mount: floor` entry MUST be accepted (§4.9.2).

### 4.7 Walls — `walls:` (edge-native; rpg-project#176/#179)

```yaml
walls:
  - { from: [7, 0], to: [7, 1], kind: solid }
  - { from: [7, 4], to: [7, 5], kind: door }
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `from` | `[int,int]` | yes | absolute `[col,row]`; distinct from and hex-adjacent to `to`; MUST be a floor cell of the active mode (§4.5) |
| `to` | `[int,int]` | yes | same constraints as `from` |
| `kind` | enum | yes | `solid \| door` |
| `lock.options[]` | array | no; if present, `>= 1` entry | see below |
| `lock.options[].ability` | string | yes (if `lock` present) | valid ability identifier |
| `lock.options[].dc` | int | yes (if `lock` present) | `1–30` |

1. Edge identity is undirected (normalized endpoint pair). A duplicate — including
   the reversed pair — MUST be rejected.
2. Both endpoints MUST be compiled floor cells of the active mode (§4.5).
3. An authored `kind` MUST replace a colliding *generated, non-connector* edge. A
   collision with a *connector-generated* edge MUST be rejected.
4. Door identity MUST derive from `(dungeon key, normalized endpoint pair)`. An
   authored door MUST start closed and unlocked. Interaction MUST be legal from
   either endpoint.
5. Runtime wall geometry is carried on `HexRecord.edges`. The flat
   `EncounterService.Space.walls` field MUST NOT exist or be reintroduced. Shared
   edges MUST be deduplicated, not assigned independently per side.
6. `lock.options` grammar (shown above) is accepted for decode/round-trip but is
   **not part of v0.3's acceptance criteria** — executable v0.3 fixtures MUST use
   unlocked doors only. Lock *behavior* is out of scope for v0.3.

**Acceptance criteria:**
- Duplicate (including reversed), non-adjacent, and out-of-footprint edges MUST fail
  validation clearly.
- An authored door edge MUST NOT satisfy, replace, or count toward a chain
  `connectors:` requirement.
- An inner authored edge changes movement/LoS on both sides while every affected
  cell retains its existing region membership unchanged (§4.10.3.6).

### 4.8 Start — `start:` (rpg-project#177)

```yaml
start: [1, 3]   # absolute [column, row]; optional/null
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `start` | `[int,int]` or `null` | no | absolute `[col,row]`; member of exactly one room's/region's floor footprint |

1. Omitted or `null` MUST preserve today's generator-selected entrance exactly.
2. MAY be in any semantic room/region regardless of archetype.
3. The door row is legal for `start:` only (contrast §4.4.2). Connector-gap cells,
   including the connector door cell, MUST be rejected.
4. Resolves against the active floor source (§4.5).
5. The toolkit MUST reserve the resolved anchor plus an ordered party envelope
   (4 seats, the normal default `PartyCap`) before generating any blockers, for
   every seed. A request exceeding the reserved seat count MUST return an explicit
   requested-vs-available error, not a fallback or partial seating.
6. `FloorPlan.entrance`/`SpaceData.Entrance` remain the sole resolved-anchor wire
   field; with an authored `start:`, they MUST report the authored anchor verbatim,
   including outside an entrance-archetype room.

**Acceptance criteria:**
- Omitted/`null` fixtures retain today's entrance/spawn/generation behavior exactly.
- A door-row start in any semantic room/region validates; malformed shapes,
  out-of-footprint cells, connector gaps, and ambiguous/no-room membership are
  rejected.
- `PutDungeon(validate_only)` wire `FloorPlan.entrance`, persisted
  `SpaceData.Entrance`, and the first real `StartEncounter` player position MUST
  agree on the authored absolute cell.

### 4.9 Facing — `place:`/`boss:` `facing:` (rpg-project#178)

```yaml
- { ref: 'dnd5e:props:statue-reaper', at: [4, 1], facing: SE }
```

| Value | `E` | `NE` | `NW` | `W` | `SW` | `SE` |
|---|---|---|---|---|---|---|
| Index | 0 | 1 | 2 | 3 | 4 | 5 |

1. `facing:` is decode-known on every `place:`/`boss:` entry type.
2. Acceptance is scoped to non-monster, `mount: floor` (default) placements — MAY be
   room-scoped, or top-level in canvas mode (§4.6). The capability belongs to the
   placement (a floor prop), not to which list it lives in.
3. A monster `place:` entry, a `boss:` entry, or a `mount: wall` placement with
   `facing:` set MUST be rejected with a field-path-specific error (e.g.
   `"facing only supported on floor props"`), not a decode failure and not a silent
   drop.
4. Current validator error order (fact, not a v0.3 change): `facing` is checked
   before `mount`; the top-level-unsupported error fires only once no entry sets
   either.
5. Omitted/`null` MUST preserve existing orientation exactly; presence MUST be
   distinguished from absence through persistence.

**Acceptance criteria:**
- All six values compile and render at the expected 60° orientations on a
  room-scoped, non-monster, floor-mounted placement.
- All six values compile and render identically on a canvas-mode top-level,
  non-monster, floor-mounted placement.
- An unknown value fails validation with the accepted vocabulary named in the error.
- A monster, boss, or `mount: wall` entry with `facing:` set fails with the
  field-path-specific message above, in either mode.

### 4.10 Regions — `regions:` (rpg-project#180, Wave 1)

#### 4.10.1 Shape

```yaml
regions:
  - id: shrine-inner
    name: 'Shrine — Inner Sanctum'
    archetype: chamber
    cells: [[9, 2], [9, 3], [9, 4], [10, 2], [10, 3], [10, 4]]
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `id` | string | yes | unique within document |
| `name` | string | no | display label; `id` used if omitted |
| `archetype` | string | no | `entrance \| chamber \| corridor \| boss`; explicit empty string MUST be rejected; omitted resolves per §4.10.2.5 |
| `cells` | `[[int,int]]` | yes | absolute `[col,row]` pairs on structural floor; duplicate pairs are canonically deduplicated; disconnected and empty sets are legal |

`cells:` is the required wire representation. `extent: {min,max}` rectangles MUST
NOT be accepted as region shape.

#### 4.10.2 Model rules

1. Every cell belongs to exactly one innermost region scope. Unpainted cells belong
   to the implicit root scope. Any walkable cell, including doorway/connector cells,
   MAY be a region member.
2. Hierarchy MUST be derived from cell-set containment only — no `parent:` field.
   For any two declared regions: disjoint cell sets = siblings (valid); one a strict
   subset of the other = child/parent (valid); any other overlap (equal or partial)
   MUST be rejected. A region's parent = the smallest strict superset among declared
   regions, or root if none. An empty region has root as its parent and owns no
   cell. A cell's innermost region = the smallest declared region containing it.
3. Regions MAY nest to arbitrary depth.
4. A region boundary exists wherever region membership changes (including
   transitions to/from root). This boundary is semantic only and MUST NOT be
   compiled into a `FloorPlanEdge` record. Traversability at a boundary is governed
   exclusively by any independently-authored `walls:` edge there (§4.7).
5. Each region property (currently: `archetype`) resolves independently by walking
   the containment chain innermost → outward; the first scope in the chain that
   declares the property wins; if none declare it, the root default (if any)
   applies.
6. Region enter/exit events fire only for the specific boundary crossed — entering a
   child region while already inside its parent MUST NOT fire a parent-exit event;
   the reverse holds on exit.
7. `archetype` is an optional, inheriting scope property. An explicit value MUST be
   one of `entrance | chamber | corridor | boss`; zero, one, or multiple explicitly
   declared regions of each value are runnable. `archetype: boss` labels a boss
   semantic scope only: it does not identify or create a boss entity/cell, mark a
   monster as boss, or add spawn behavior. A top-level `place:` monster remains an
   explicit ordinary placement. Missing semantic roles or conventions are non-blocking
   follow-up findings where an existing surface can report them; this wave adds no
   diagnostics proto/API and makes none an acceptance requirement.
8. Region create/edit/delete operations MUST NOT alter structural floor, canonical
   wall/door edges, door identity, or the validity of any content (placement/
   monster/boss) whose cell falls inside the edited region. Content whose owning
   region is deleted MUST fall back to the next enclosing scope. Connector `DoorID`
   (room-ID-derived, `compile.go:301`) and authored-edge `AuthoredDoorID`
   (endpoint-derived, `authored_edges.go:32-36`) are both region-independent and
   MUST NOT be recomputed by a region edit.

#### 4.10.3 Structural validation and persistence

1. Hard rejection is limited to source that cannot be represented safely or
   deterministically: malformed YAML/types/coordinates; duplicate region IDs; a
   cell outside structural floor; incompatible non-empty `rooms:` with `regions:`;
   an unsupported explicit archetype; and equal or partial overlap between regions,
   which prevents a unique innermost owner.
2. Within one region, duplicate cell pairs are canonically deduplicated before
   compile and persistence; no error or new diagnostic surface is required. A
   region's canonical source cells may be empty or disconnected in this exploratory
   phase.
3. Disjoint sets and strict containment resolve per §4.10.2.2. Regions need not
   tile the canvas; sparse, disconnected regions with unclaimed cells are valid.
4. An inner wall (§4.7) MUST NOT alter a region's canonical source-cell membership.
5. Persistence retains only irreducible authored region facts: `id`, `name`,
   archetype presence/value, and canonical source cells. Parent relationships and
   the innermost cell index are deterministic derived state: recompute them on
   compile/load. They MUST NOT be persisted, and a missing or disagreeing derived
   cache MUST NOT reject reload.
6. A document combining non-empty `rooms:` with declared `regions:` MUST be
   rejected — a document declares exactly one topology model, never both.

#### 4.10.4 Wire projection

**Authoring** — `FloorPlan` MAY project, for each declared region: its `cells:`
extent and a toolkit-derived parent region id (absent means root). This is in
addition to, not a replacement for, the per-hex runtime projection below.

**Runtime**:

| Field | Type | Semantics |
|---|---|---|
| `HexRecord.zone_id` | string | innermost region id for this cell; `""` = root |
| `Zone.parent_id` | string, optional | derived parent region id; absent = root |

1. Runtime zone metadata MUST be fog-authorized: expose the innermost zone and
   required ancestor chain only for cells the observing player has seen. A global
   hidden cell-to-region map MUST NOT be exposed.

#### 4.10.5 Extension seam

1. This cut adds no properties to `RegionDoc` beyond `id`/`name`/`archetype`/
   `cells`. A future spec level adding one MUST resolve it through the same
   per-property innermost-outward walk (§4.10.2.5).

**Acceptance criteria:**
- A structurally usable graph compiles and reloads with zero, one, or multiple
  explicitly declared `entrance`, `boss`, `chamber`, and `corridor` regions, and
  with disconnected or empty regions. Missing roles are non-blocking and need no
  new diagnostic wire surface.
- Duplicate IDs, out-of-floor cells, unsupported explicit archetypes, equal/partial
  overlaps, and incompatible non-empty `rooms:` plus `regions:` reject clearly.
  Duplicate cells within one region canonicalize to one source cell.
- Parent-ID/innermost-index derivation is deterministic across compile/load and
  repaint/delete, without persisting derived state or rejecting a missing/disagreeing
  derived cache.
- `archetype: boss` labels a semantic scope only; no boss entity/cell, monster
  marker, content placement rule, or spawn behavior is required. Ordinary top-level
  `place:` monsters remain ordinary placements.
- Authoring `FloorPlan` may project each declared region's canonical `cells:` extent
  and toolkit-derived parent id.
- Runtime projection may expose `Zone.parent_id` and per-hex innermost `zone_id`,
  fog-authorized, with no hidden-extent disclosure.
- No region boundary appears as a `FloorPlanEdge` record unless an independent
  `walls:` entry (§4.7) also exists on that same edge.
