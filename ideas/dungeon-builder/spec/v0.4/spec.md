# Dungeon YAML Spec v0.4

**Status:** PROPOSED v0.4, 2026-08-08. Not yet ratified.
**Normative.** Delta on RATIFIED v0.3 (`../v0.3/spec.md`) — restated in full per this
directory's own precedent (a platform implementer starts from this file alone, never a
diff against the prior level). See `README.md` for context, rationale, alternatives
considered, and the open ratification points this proposal carries.

## 1. Level cut

| Group | Constructs | Status |
|---|---|---|
| (a) Already-v1 chain | `rooms:` (id/archetype/width), `connectors:` (+ `locked:`), room-scoped `place:`/`boss:`/`obstacles:` | real, `dungeonspec.Validate`/`PutDungeon`, unchanged |
| (b) Already-compiling | `walls:` edge lists, `start:`, room-scoped floor-prop `facing:` | live-verified (rpg-api#769/rpg-toolkit#881) |
| (c) v0.3 Wave 0 | `canvas: {width,height}`, mode-branched validation, top-level `place:` (canvas mode only), `start:` on canvas floor, shrink-validation | LIVE VERIFIED |
| (d) v0.3 Wave 1 (exploratory) | `regions:` (`id`, `name?`, `archetype`, `cells`), scopes semantics, per-property innermost-outward resolution | implementation not started |
| (e) v0.4 — canvas floor source | canvas-mode structural floor = union of declared `regions[].cells`, not the full canvas rectangle; implicit envelope walls at every floor/void boundary; `spec:` document field as the floor-semantics discriminator | **PROPOSED, this document** |
| (f) v0.4 — smaller promotions | `height`/`offset` decor fine-positioning on non-monster placements; region-level `lighting:` resolving per §4.10.2.5's extension seam; the `wallLines:` client-projection contract | **PROPOSED, this document** |

Per-construct specs: §4.1–§4.14. Constructs not listed in this table or §2 are out of
scope for v0.4 by omission. Group (e) is the headline of this cut — see README.md's
"Kirk's ruling" section for the verbatim input it transcribes. Groups (a)–(d) are
carried forward from `../v0.3/spec.md` verbatim except where this document's §4.5/§4.10
text explicitly revises them.

## 2. Explicitly ABOVE v0.4

| Construct | Pointer |
|---|---|
| `mount: wall` (wall-mounted placement geometry, edge selection) | rpg-project#188 — `height`/`offset` graduate in this cut (§4.11); `mount: wall`'s own edge-selection semantics do not |
| `defaults:` (ref-keyed defaults + materialize-on-strip) | unfiled |
| `holes:` | deferred, not queued |
| `end:` | rpg-project#186 |
| `orientation:` | rpg-project#187 |
| `targeting:` | rpg-project#191 |
| `wallLines:` as a wire construct | **REJECTED, not merely unfiled** — see §4.13; a builder MUST compile it to `walls:` before any `PutDungeon` call, forever |
| coverage/fractional standability as a server concept | **REJECTED** — see §4.13.3; traversability is edge-based only, never a partial-coverage value |
| `rotate_degrees:` | experiment only, not a dialect candidate |

### Rejection classes

Unchanged from `../v0.3/spec.md` §2 — every construct above is rejected by exactly one
of the three mechanisms that document defines (decode-known field-path rejection;
whole-document decode failure; decode-known but `Validate`-accepted, i.e. actually in
scope). `wallLines:` and fractional coverage are rejection class 2 (never decode-known)
by this document's own design — see §4.13.

## 3. Ground rules

1. Decode is strict and single-document: `KnownFields(true)`; an unrecognized
   top-level field MUST fail the whole document; multi-document input MUST be
   rejected.
2. `version:` MUST equal `1`. Any other value MUST be rejected. This is unchanged and
   unaffected by this document: the floor-semantics change in §4.5 is carried entirely
   by the new `spec:` field (§4.14), never by the document `version:` integer — see
   README.md's "Why `spec:`, not `version: 2`" for the reasoning.
3. Every cell coordinate (`walls:`, `start:`, `place[].at`, `regions[].cells`) is
   absolute pointy-top odd-q offset `[column,row]` — the same space a compiled
   `FloorPlan` uses.
4. Physical geometry (space/canvas/floor, canonical wall/door edges) and semantic
   scope (regions, §4.10) are independent axes for every purpose EXCEPT floor
   derivation in canvas mode under `spec: "0.4"` (§4.5), where — per Kirk's ruling —
   the floor **is** the declared region set. This is a deliberate, narrow exception to
   the axis-independence rule stated in `../v0.3/spec.md` §3.4, not a repeal of it:
   regions still carry no wall/door semantics of their own (§4.10.2.4 unchanged), and
   walls still carry no region semantics of their own (§4.10.3.4 unchanged) — only
   *which cells exist as floor at all* now derives from regions, in canvas mode, under
   the v0.4 marker.
5. `spec:` (§4.14) is decode-known on every document, in both floor-source modes. Its
   only validation effect is selecting which canvas-mode floor-derivation rule §4.5
   applies; it is otherwise inert, including in room-chain mode.

## 4. Per-construct specification

### 4.1 Document metadata

```yaml
version: 1
key: shrine-hall
name: 'The Shrine Hall'
theme: crypt
height: 8
spec: '0.4'
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `version` | int | yes | MUST equal `1` |
| `key` | string | yes | `[a-z0-9-]+`; MUST match the request key |
| `name` | string | yes | non-empty |
| `theme` | string | no | — |
| `height` | int | decode-optional (missing → `0`) | room-chain mode: `Validate` requires `>= 4`. canvas mode: unvalidated and unused — `canvas.height` (§4.5) is authoritative; a value below `4` MUST NOT be rejected and MUST NOT affect canvas geometry |
| `spec` | string, optional | no | `"0.3"` \| `"0.4"`; omitted MUST behave as `"0.3"`. Any other literal value (including `"draft"`) MUST be rejected as an unrecognized enum value. Full semantics: §4.14. |

### 4.2 Room chain — `rooms:`

Carried from `../v0.3/spec.md` §4.2; rules 1–4 are byte-identical, rule 5 is new.

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
5. Room-chain mode's own generated perimeter/door envelope (rpg-api#769/
   rpg-toolkit#881) is unaffected by this document — it is the same mechanism §4.5.12
   generalizes for canvas mode, not a second one.

### 4.3 Connectors — `connectors:`

Unchanged from `../v0.3/spec.md` §4.3.

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

Unchanged from `../v0.3/spec.md` §4.4, except the field table gains `height`/`offset`
(§4.11) rows.

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
| `place[].height` | float | no | props only; see §4.11 for acceptance scope |
| `place[].offset` | `[float,float]` | no | props only; see §4.11 for acceptance scope |
| `obstacles[].ref` | string | yes | — |
| `obstacles[].count` | int | yes | `>= 1` |

1. `at` resolves to an absolute cell by adding the room's `start_column` (§4.2.2).
2. The door row (`row == height/2`) MUST be rejected for `place[].at` and `boss.at`.
3. Exactly one room MUST declare `boss:` (§4.2.3); there is no top-level `boss:` in
   v0.4 (§4.6.2).

### 4.5 Canvas — `canvas:` (rpg-project#192, Wave 0; floor source revised in v0.4)

```yaml
spec: '0.4'
canvas:
  width: 20
  height: 30
rooms: []
regions:
  - id: hall
    archetype: chamber
    cells: [[3, 3], [4, 3], [5, 3], [3, 4], [4, 4], [5, 4]]
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
   nil room reference (`../v0.3/README.md`'s "two-floor-source finding").
4. **Canvas-mode structural floor derivation branches on `spec:` (§4.14):**
   - `spec: "0.4"` — structural floor is the **canonical union of every declared
     `regions[].cells`** (§4.10), deduplicated per §4.10.3.2. `canvas.width`/
     `canvas.height` no longer directly produce floor cells; they remain the
     coordinate-legality bound (rule 6, below) and the authoring workspace's outer
     extent. A `regions: []` (or all-empty-region) document under `spec: "0.4"`
     produces an EMPTY floor-cell set — this MUST NOT be hard-rejected on that basis
     alone (rule 13, below).
   - `spec: "0.3"` or omitted — structural floor is the full canvas rectangle
     (`[0,width) × [0,height)`), unchanged from `../v0.3/spec.md` §4.5.4. This
     preserves every already-persisted v0.3 canvas document's behavior exactly, with
     no migration required.
5. Independently of rule 4: persisted authored-edge validation at `InitDungeon` time
   (the runtime-side check behind §4.7's endpoint-floor-membership requirement) MUST
   ALSO branch on `spec:` the same way. This is a second, separately-implemented
   check — extending the static path (rule 4) does not extend this one
   (`../v0.3/README.md`'s "two-floor-source finding" applies identically here: both
   `semanticFloorCells` and `semanticDungeonFloorHexes` need their own
   `spec: "0.4"` branch, not just one of them).
6. In-bounds checks otherwise run against `canvas.width`/`canvas.height`, in both
   `spec:` branches — being in-bounds is necessary but, under `spec: "0.4"`, no
   longer sufficient for floor membership (rule 4).
7. Top-level `place:` (§4.6) MUST be accepted in canvas mode and MUST remain
   rejected in room-chain mode.
8. `start:` (§4.8) resolves against the canvas floor in canvas mode, as derived by
   rule 4 for the document's own `spec:` value.
9. `regions:` MUST NOT be declared without `canvas:` present. A document declaring
   `regions:` in room-chain mode is already rejected by §4.10.3.6; a document
   declaring `regions:` with neither `rooms:` nor `canvas:` present MUST also be
   rejected — regions have no coordinate-legality bound (rule 6) to validate against
   without a canvas.
10. `FloorPlan` MUST echo `rooms: []` plus `canvas.width`/`canvas.height`, with
    authored edges present as usual, and MUST also project:
    - the canonical structural-floor cell set, derived per rule 4 for the document's
      own `spec:` value (unchanged requirement from `../v0.3/spec.md` §4.5.9, now
      explicitly spec-branched);
    - under `spec: "0.4"` only: the projected envelope edges (rule 12) as ordinary
      canonical edges, alongside any authored/generated interior edges — **projected
      explicitly, not left for a client to re-derive from floor membership**. This
      follows the walls-from-truth principle the runtime wire already committed to
      when `EncounterService.Space.walls` was retired in favor of `HexRecord.edges`
      (`../v0.3/spec.md` §4.7.5): a client that stopped deriving walls from shapes
      once should not be asked to re-derive them from floor shape now.
11. Growing `width`/`height` MUST be accepted unconditionally. Shrinking below any
    existing placement, wall endpoint, `start:`, or region cell MUST be rejected with
    a named, specific error — never a silent drop. Unchanged from `../v0.3/spec.md`
    §4.5.10; region cells were already covered there.
12. **Envelope walls (`spec: "0.4"` only).** Every hex edge with exactly one side a
    structural floor cell (rule 4) and the other side not (off-canvas, or simply
    unclaimed by any declared region) is an **implicit envelope edge**: it blocks
    movement and line of sight exactly as a generated `kind: solid` edge would.
    - This is the room-chain envelope model (§4.2.5, rpg-api#769/rpg-toolkit#881),
      generalized from "perimeter of a rectangle" to "perimeter of an arbitrary
      region-union shape." No new movement/LoS mechanism is introduced; only the
      floor shape the existing mechanism wraps changes.
    - Envelope edges MUST be projected per rule 10's third bullet — as ordinary
      canonical edges, indistinguishable on the wire from any other generated
      non-connector edge. No new wire type or edge kind is introduced.
    - An authored `walls:` entry MUST still satisfy §4.7.2 (both endpoints
      compiled floor cells). Because an envelope edge by definition has exactly one
      floor-cell side, no authored edge can ever be geometrically coincident with
      one — §4.7.2's existing endpoint rule already excludes the case; this document
      adds no separate error/no-op/merge rule for "an authored wall on the envelope"
      because the case cannot arise.
    - There is no door construct through the envelope. Passage between two floor
      areas that are not cell-adjacent is achieved exclusively by declaring floor
      over the connecting cells (extending a region, or adding a bridging region) —
      never by an edge construct. An envelope edge always has void on one side; a
      door (§4.7.4) presumes two floor endpoints to open onto, which an envelope
      edge structurally cannot provide.
    - Two regions whose cell sets are hex-adjacent connect **openly by default** —
      unchanged from `../v0.3/spec.md` §4.10.2.4 (a region boundary is semantic
      only and implies no edge). An author who wants to gate that specific
      connection draws an ordinary `walls:` door edge between the two adjacent
      floor cells — the same region-attachment-door concept `../v0.3/spec.md`
      §4.3.3/§4.10.2 already names, unaffected by this cut.
13. A canvas document under `spec: "0.4"` with `regions: []` (or only empty/
    disconnected regions with a resulting empty union) MUST validate with an empty
    floor-cell set — it MUST NOT be hard-rejected merely for having no floor,
    consistent with Wave 1's exploratory posture (rpg-project#200: disconnected and
    empty regions are runnable). Any content that requires a floor cell (`start:`,
    `place[].at`, `walls:` endpoints) is rejected by that construct's own existing
    floor-membership rule — no new blanket rejection is introduced for the
    zero-floor case itself.
14. Non-goals: no boss/monster content requirement; no locked-door grammar/binding
    decision beyond §4.7.6. Unlike `../v0.3/spec.md` §4.5.11, this document does
    **not** carry forward "a canvas document with no `regions:` treats its whole
    floor as the implicit root scope" — under `spec: "0.4"`, an unpainted cell is
    not floor at all (rule 4), so there is no floor for root to own by default; see
    §4.10.2.1's revision.

**Acceptance criteria:**
- `PutDungeon(validate_only)` against `canvas: {width,height}`, `rooms: []` decodes
  successfully and returns a compiled `FloorPlan`, in both `spec:` branches.
- Under `spec: "0.3"` or omitted `spec:`, every `../v0.3/spec.md` §4.5 acceptance
  criterion holds unchanged, byte-for-byte, against an otherwise-identical document.
- Under `spec: "0.4"`, a canvas document's structural floor equals exactly the
  canonical union of its declared `regions[].cells`; a cell inside `canvas.width`×
  `canvas.height` but claimed by no region is NOT floor and fails any
  floor-membership check the same way an out-of-canvas cell does.
- Under `spec: "0.4"`, a `walls:` edge whose `from`/`to` straddles a region-cell /
  unclaimed-cell boundary is rejected by the existing endpoint-floor-membership rule
  (§4.7.2) — the same error class as an out-of-bounds edge, not a new one.
- Under `spec: "0.4"`, every hex edge on the boundary of the region union — including
  edges that would have been canvas-rim edges under `spec: "0.3"` and were previously
  inexpressible as an authored wall (`../v0.3/README.md`'s `wallLines:` rim-edge
  finding does not recur here, since these edges are never authored at all) — is
  present in `FloorPlan`/`HexRecord.edges` as a generated `solid` edge, with no
  author action required.
- Under `spec: "0.4"`, a `regions: []` canvas document validates successfully with an
  empty floor-cell set; a subsequent `start:`/`place:`/`walls:` entry against that
  document fails with an out-of-footprint error, not a document-level rejection.
- Shrinking `width`/`height` below existing content produces a named, specific
  validation error, never a silent drop, in both `spec:` branches.
- A document declaring both non-empty `rooms:` and `canvas:` is rejected, in both
  `spec:` branches.
- A document declaring `regions:` with neither `canvas:` nor non-empty `rooms:`
  present is rejected.

### 4.6 Top-level placement — `place:` (canvas mode)

Carried from `../v0.3/spec.md` §4.6: the field table gains `height`/`offset` (§4.11)
rows matching §4.4's addition, rules 1–3 are byte-identical, and rule 4 is new.

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
2. There is no top-level `boss:` in v0.4 — `boss:` remains room-scoped only (§4.4.3).
3. `facing:` on a non-monster, `mount: floor` entry MUST be accepted (§4.9.2).
4. Under `spec: "0.4"`, `at` MUST resolve to a structural floor cell per §4.5.4 (the
   region union) — a placement outside every declared region's cells is rejected the
   same way an out-of-canvas placement is, even if it falls inside `canvas.width`×
   `canvas.height`.

### 4.7 Walls — `walls:` (edge-native; rpg-project#176/#179)

Restated in full because §4.5.12's envelope model depends on rule 2 exactly as
written; every rule below is byte-identical to `../v0.3/spec.md` §4.7 except rule 3's
added parenthetical and one added acceptance criterion, both called out below.

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
2. Both endpoints MUST be compiled floor cells of the active mode (§4.5) — under
   `spec: "0.4"` canvas mode, this means both endpoints MUST be members of the
   region-union floor (§4.5.4), not merely in-canvas-bounds.
3. An authored `kind` MUST replace a colliding *generated, non-connector* edge
   (including a §4.5.12 envelope edge, in principle — see §4.5.12's own note that
   this case cannot arise in practice, since an envelope edge never has two
   floor-cell endpoints to author over). A collision with a *connector-generated*
   edge MUST be rejected.
4. Door identity MUST derive from `(dungeon key, normalized endpoint pair)`. An
   authored door MUST start closed and unlocked. Interaction MUST be legal from
   either endpoint.
5. Runtime wall geometry is carried on `HexRecord.edges`. The flat
   `EncounterService.Space.walls` field MUST NOT exist or be reintroduced. Shared
   edges MUST be deduplicated, not assigned independently per side.
6. `lock.options` grammar (shown above) is accepted for decode/round-trip but is
   **not part of v0.4's acceptance criteria** — executable v0.4 fixtures MUST use
   unlocked doors only. Lock *behavior* is out of scope for v0.4.

**Acceptance criteria:** unchanged from `../v0.3/spec.md` §4.7, plus:
- Under `spec: "0.4"` canvas mode, an edge with one endpoint in the region union and
  one endpoint outside it (whether off-canvas or merely unclaimed) fails validation
  with the same error class as any other out-of-footprint edge.

### 4.8 Start — `start:` (rpg-project#177)

Carried from `../v0.3/spec.md` §4.8; rules 1–3 and 5–6 are byte-identical, rule 4
gains a `spec: "0.4"` clarification.

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
4. Resolves against the active floor source (§4.5) — under `spec: "0.4"` canvas
   mode, the region-union floor. A `start:` on a canvas document with an empty
   floor-cell set (§4.5.13) MUST be rejected as out-of-footprint, the ordinary
   consequence of rule 3 there, not a new rule.
5. The toolkit MUST reserve the resolved anchor plus an ordered party envelope
   (4 seats, the normal default `PartyCap`) before generating any blockers, for
   every seed. A request exceeding the reserved seat count MUST return an explicit
   requested-vs-available error, not a fallback or partial seating.
6. `FloorPlan.entrance`/`SpaceData.Entrance` remain the sole resolved-anchor wire
   field; with an authored `start:`, they MUST report the authored anchor verbatim,
   including outside an entrance-archetype room.

**Acceptance criteria:** unchanged from `../v0.3/spec.md` §4.8.

### 4.9 Facing — `place:`/`boss:` `facing:` (rpg-project#178)

Unchanged from `../v0.3/spec.md` §4.9.

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
4. Current validator error order (fact, not a v0.4 change): `facing` is checked
   before `mount`; the top-level-unsupported error fires only once no entry sets
   either.
5. Omitted/`null` MUST preserve existing orientation exactly; presence MUST be
   distinguished from absence through persistence.

**Acceptance criteria:** unchanged from `../v0.3/spec.md` §4.9.

### 4.10 Regions — `regions:` (rpg-project#180, Wave 1 — exploratory per rpg-project#200)

#### 4.10.1 Shape

Carried from `../v0.3/spec.md` §4.10.1, plus the new `lighting` property (§4.10.2.5
extension, §4.12) and a clarifying note on `cells:`'s floor-defining role under
`spec: "0.4"` (restating §4.5.4, not a new rule).

```yaml
regions:
  - id: shrine-inner
    name: 'Shrine — Inner Sanctum'
    archetype: chamber
    cells: [[9, 2], [9, 3], [9, 4], [10, 2], [10, 3], [10, 4]]
    lighting: { ambient: 0.5 }
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `id` | string | yes | unique within document |
| `name` | string | no | display label; `id` used if omitted |
| `archetype` | string | no | `entrance \| chamber \| corridor \| boss`; explicit empty string MUST be rejected; omitted resolves per §4.10.2.5 |
| `cells` | `[[int,int]]` | yes | absolute `[col,row]` pairs; under `spec: "0.4"` canvas mode, these cells collectively **define** structural floor (§4.5.4) rather than merely being validated against it; duplicate pairs are canonically deduplicated; disconnected and empty sets are legal |
| `lighting` | object, optional | no | `{ ambient: float 0..1 }`; resolves per §4.10.2.5, see §4.12 |

`cells:` is the required wire representation. `extent: {min,max}` rectangles MUST
NOT be accepted as region shape.

#### 4.10.2 Model rules

Unchanged from `../v0.3/spec.md` §4.10.2, except point 1, which this document revises
for `spec: "0.4"`.

1. **Revised for `spec: "0.4"`:** every floor cell belongs to exactly one innermost
   region scope. There are no unpainted floor cells under `spec: "0.4"` — an
   unclaimed cell is not floor at all (§4.5.4), so `../v0.3/spec.md`'s "unpainted
   cells belong to the implicit root scope" no longer has floor cells to apply to.
   Root remains meaningful purely as a **hierarchy** concept: the parent of any
   top-level declared region (rule 2, below), never as a cell owner, under
   `spec: "0.4"`. Under `spec: "0.3"` or omitted, this point is unchanged from
   `../v0.3/spec.md`: root owns every unpainted canvas-rectangle cell, exactly as
   before. Any walkable cell, including doorway/connector cells, MAY be a region
   member, in both branches.
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
   exclusively by any independently-authored `walls:` edge there (§4.7) — or, new in
   this document, by an implicit envelope edge (§4.5.12) where the boundary also
   happens to be a floor/void boundary. A region-to-region boundary that is NOT also
   a floor/void boundary carries no implicit edge of any kind, exactly as before.
5. Each region property (`archetype`, and — new in this document — `lighting`)
   resolves independently by walking the containment chain innermost → outward; the
   first scope in the chain that declares the property wins; if none declare it, the
   root default (if any) applies. For `lighting`, the document-level `lighting:`
   block (§4.12) IS root's declaration in this walk — there is no separate
   "document lighting" mechanism alongside the region resolution chain; they are the
   same mechanism, root's own entry in it.
6. **Future extension seam (non-normative).** A future enter/exit capability may
   derive transitions from changes in the scope chain. v0.4 emits and requires no
   region transition events and adds no trigger capability, unchanged from v0.3.
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
   monster/boss) whose cell falls inside the edited region — **except** that under
   `spec: "0.4"`, adding or removing a region cell inherently changes structural
   floor itself (§4.5.4), by design; "MUST NOT alter structural floor" in this point
   refers to edits that do not change any region's cell membership (rename, retarget
   `archetype`, adjust `lighting`), not to cell-membership edits, which are exactly
   how floor is authored under `spec: "0.4"`. Content whose owning region is deleted
   MUST fall back to the next enclosing scope. Connector `DoorID` (room-ID-derived,
   `compile.go:301`) and authored-edge `AuthoredDoorID` (endpoint-derived,
   `authored_edges.go:32-36`) are both region-independent and MUST NOT be
   recomputed by a region edit.

#### 4.10.3 Structural validation and persistence

Carried from `../v0.3/spec.md` §4.10.3, with two changes: rule 1's floor-boundary
check is restated against canvas bounds rather than structural floor (a region cell
can't be checked against "structural floor" when, under `spec: "0.4"`, it's what
DEFINES structural floor — §4.5.4), and rule 7 is new.

1. Hard rejection is limited to source that cannot be represented safely or
   deterministically: malformed YAML/types/coordinates; duplicate region IDs; a
   cell outside canvas bounds (§4.5.6) — under `spec: "0.3"` or omitted, this is
   equivalent to "outside structural floor," unchanged from `../v0.3/spec.md`;
   incompatible non-empty `rooms:` with `regions:`; an unsupported explicit
   archetype; and equal or partial overlap between regions, which prevents a unique
   innermost owner.
2. Within one region, duplicate cell pairs are canonically deduplicated before
   compile and persistence; no error or new diagnostic surface is required. A
   region's canonical source cells may be empty or disconnected in this exploratory
   phase.
3. Disjoint sets and strict containment resolve per §4.10.2.2. Regions need not
   tile the canvas; sparse, disconnected regions with unclaimed cells are valid —
   under `spec: "0.4"`, "unclaimed" cells are simply not floor (§4.5.4), not an
   error condition.
4. An inner wall (§4.7) MUST NOT alter a region's canonical source-cell membership.
5. Persistence retains only irreducible authored region facts: `id`, `name`,
   archetype presence/value, `lighting` presence/value, and canonical source cells.
   Parent relationships and the innermost cell index are deterministic derived
   state: recompute them on compile/load. They MUST NOT be persisted, and a
   missing or disagreeing derived cache MUST NOT reject reload.
6. A document combining non-empty `rooms:` with declared `regions:` MUST be
   rejected — a document declares exactly one topology model, never both.
7. `regions:` without `canvas:` present (and without non-empty `rooms:`) MUST be
   rejected (§4.5.9) — there is no coordinate-legality bound to validate cells
   against otherwise.

#### 4.10.4 Wire projection

Unchanged from `../v0.3/spec.md` §4.10.4.

**Authoring** — `FloorPlan` MUST project, for each declared region: its `cells:`
extent and a toolkit-derived parent region id (absent means root). This is in
addition to, not a replacement for, the per-hex runtime projection below. This
projection exposes derived scope structure; it creates no semantic validation.

**Runtime**:

| Field | Type | Semantics |
|---|---|---|
| `HexRecord.zone_id` | string | innermost region id for this cell; `""` = root |
| `Zone.parent_id` | string, optional | derived parent region id; absent = root |

1. Runtime projection MUST expose `Zone.parent_id` and per-hex innermost `zone_id`,
   fog-authorized: expose the innermost zone and required ancestor chain only for
   cells the observing player has seen. A global hidden cell-to-region map MUST NOT
   be exposed. This projection creates no semantic validation.

#### 4.10.5 Extension seam

1. This cut adds `lighting` (§4.12) as the second `RegionDoc` property beyond
   `id`/`name`/`archetype`/`cells`, resolved through the same per-property
   innermost-outward walk (§4.10.2.5) `../v0.3/spec.md` reserved this seam for. A
   future spec level adding a third property MUST resolve it the same way.

**Acceptance criteria:** unchanged from `../v0.3/spec.md` §4.10, plus:
- Under `spec: "0.4"`, a region's `cells:` collectively determine structural floor;
  a cell claimed by no declared region is absent from `FloorPlan`'s projected
  structural-floor set, indistinguishable from an out-of-canvas cell for every
  floor-membership check.
- A region's `lighting.ambient`, when declared, resolves per the same
  innermost-outward walk as `archetype`; an undeclared region inherits its parent's
  value; a region chain with no declaration at any level inherits the document-level
  `lighting.ambient` (§4.12) as root's value.
- `regions:` declared without `canvas:` present (and without non-empty `rooms:`)
  is rejected.

### 4.11 Decor fine-positioning — `height` + `offset`

Promotes TARGET-YAML.md's `height` field (decoupled from `mount`, per Kirk's
2026-08-02 ruling — see README.md) plus a new `offset` field, both scoped narrowly to
cosmetic, decor-grade positioning. Neither field has any mechanical effect: they MUST
NOT influence movement, line of sight, `blocks_movement`/`blocks_los` resolution, or
any traversability/standability computation (§4.13.3 states the equivalent
prohibition for coverage-based standability; this is the same discipline applied to
decor fields directly).

```yaml
- { ref: 'dnd5e:props:candles', at: [4, 4], height: 0.5, offset: [0.15, -0.1], blocks_movement: false, blocks_los: false }
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `height` | float | no | meters above the floor plane; unclamped (a negative value is decode-legal, purely cosmetic — see rule 3) |
| `offset` | `[float,float]` | no | intra-hex fine position, `[x,y]` each in `[-0.5, 0.5]`, fractional units of the hex's own radius from cell center; a value outside that range MUST be rejected |

1. Both fields are decode-known on any `place:` entry (room-scoped, §4.4, or
   top-level canvas-mode, §4.6) or `boss:` entry.
2. Acceptance is scoped identically to `facing:` (§4.9.2): non-monster, `mount:
   floor` (default) placements only. A monster `place:` entry, a `boss:` entry, or a
   `mount: wall` placement with `height:` or `offset:` set MUST be rejected with a
   field-path-specific error, not a decode failure and not a silent drop. (`mount:
   wall` itself remains above v0.4, §2 — a document may still decode a `mount: wall`
   value per `../v0.3/spec.md`'s class-1 rejection precedent, but `height`/`offset`
   on such an entry are rejected regardless.)
3. Neither field is validated against, or resolved relative to, any other
   placement, wall, or obstacle — two decor placements may overlap in effective
   world position with no conflict raised. This is deliberate: both fields are
   cosmetic-only (rule statement, above); a coverage/overlap check belongs to
   §4.13.3's rejected category (server-side coverage semantics), not to this
   section.
4. Omitted `height`/`offset` MUST preserve today's floor-centered, zero-height
   rendering exactly.

**Acceptance criteria:**
- A room-scoped or top-level, non-monster, `mount: floor` placement compiles with
  any combination of `height`/`offset` set, including together.
- A monster, boss, or `mount: wall` entry with `height:` or `offset:` set fails with
  a field-path-specific message, in either canvas or room-chain mode.
- An `offset` component outside `[-0.5, 0.5]` fails validation clearly, naming the
  accepted range.
- Neither field's presence or value changes any `blocks_movement`/`blocks_los`/
  traversal/envelope outcome for the placement's own cell or any other.

### 4.12 Lighting — `lighting:` (document-level, rpg-project#190) + region extension

```yaml
lighting:
  ambient: 0.8   # 0..1, document-wide default — root's value in the §4.10.2.5 walk

regions:
  - id: shrine-inner
    lighting: { ambient: 0.35 }   # overrides for this scope and its descendants
```

| Field | Type | Required | Constraints |
|---|---|---|---|
| `lighting.ambient` | float | no | `0..1`; document-level block is optional, defaults to the renderer's own baseline when absent |
| `regions[].lighting.ambient` | float | no | `0..1`; see §4.10.2.5 for resolution |

1. Document-level `lighting:` is decode-known in both floor-source modes. It is
   root's own declaration in the region `lighting` resolution chain (§4.10.2.5) —
   there are two places to read `lighting.ambient` from (the document block, and any
   region's own block) precisely because they are the same mechanism at two depths,
   not two independent mechanisms.
2. A region silent on `lighting` inherits its parent's resolved value; a top-level
   region silent on `lighting` inherits the document-level value if present, or the
   renderer's own baseline if the document-level block is also absent.
3. Per-source lighting (`sources: [...]`, individual light placements) remains
   explicitly out of scope, unchanged from rpg-project#190's own scope statement —
   `ambient` is the only knob at every level.
4. In room-chain mode, only the document-level block applies — `rooms:` has no
   region-shaped scope to extend `lighting` into. This document does not propose a
   room-scoped `lighting:` field.

**Acceptance criteria:**
- A document with only a document-level `lighting.ambient` resolves that value for
  every region (declared or root) that does not override it.
- A region declaring its own `lighting.ambient` resolves to that value for its own
  cells and every descendant region that does not itself override it, regardless of
  the document-level value.
- `ambient` outside `0..1`, at either the document or region level, fails validation
  clearly.

### 4.13 `wallLines:` — client-projection contract, and coverage-based standability

`wallLines:` (straight, footprint-bearing walls; TARGET-YAML.md's own "Straight
walls" section) is **rejected as a wire construct, permanently** — this document
resolves `../v0.3/spec.md` §2's "unfiled" status into an explicit, permanent
rejection, not merely a deferral.

1. `wallLines:` MUST NOT be decode-known on `PutDungeon`, ever. A document
   containing it MUST fail the whole-document decode (rejection class 2,
   `../v0.3/spec.md` §2's Rejection classes table).
2. An authoring tool offering `wallLines:` as an authoring convenience MUST compile
   it down to ordinary `walls:` edge entries (§4.7) before any `PutDungeon` call —
   never send `wallLines:` verbatim. The compilation MUST follow the footprint/clip
   semantics TARGET-YAML.md's own `straightWallGeometry.ts` already implements
   (footprint cells, both-clear-cell crossed edges, door-cell exclusion): for every
   hex-adjacent floor-cell pair where the line's clip test finds a genuine crossing
   (a touch does not count), emit a `walls:` entry of `kind: solid`, or `kind: door`
   at a crossing adjoining an authored `doors:` cell.
3. **Coverage-based (fractional/geometric) standability is never a server concept,
   in either floor-source mode.** The server's only traversability primitives are
   the discrete floor-cell set (§4.5) and canonical edges (§4.7, §4.5.12) — there is
   no partial-coverage value anywhere on the wire, and this document does not
   propose one. Kirk's own footprint rule ("any hex that is not 100% uncovered
   would not be traversable," `../v0.3/README.md`'s `wallLines:` note) is exactly
   the conversion from a continuous geometric fact to this discrete model, and that
   conversion MUST happen entirely client-side, before rule 2's compilation — it
   MUST NOT be deferred to, or re-derived by, the server.
4. A named, honest limitation, carried from rule 2's compilation: a wallLines
   footprint can produce an isolated fully-blocked cell with no clear neighbor to
   hang a blocking edge against (every one of its six neighbors also
   footprint-blocked) — such a cell has no direct `walls:` representation. An
   authoring tool MUST flag this case to the author rather than silently dropping
   it or approximating it with a partial edge set. This document does not resolve
   the gap; it names it so a future compilation round does not rediscover it from
   scratch (TARGET-YAML.md's own "genuine, honestly-recorded gap" for the
   grazed-edge door case is the same class of finding).
5. **Platform note, non-normative but load-bearing for capacity planning:** `walls:`
   arriving from a `wallLines:`-using builder will typically be a dense edge list —
   every crossed boundary of a multi-cell footprint, potentially dozens of entries
   for a single authored line — not the small, hand-drawn set a direct `walls:`
   authoring flow produces. §4.7's validation MUST NOT assume edge-list size is
   small or manually curated.

**Acceptance criteria:**
- `wallLines:` present anywhere in a document fails the whole-document decode.
- A `walls:` edge list compiled from a `wallLines:` footprint validates identically
  to a hand-authored edge list of the same geometry — no separate code path or
  relaxed validation for line-derived edges.
- No server-side check anywhere accepts, stores, or interprets a fractional
  coverage/partial-block value.

### 4.14 The `spec:` marker

Introduced by this document as the floor-semantics discriminator Kirk's ruling
requires (README.md's "Kirk's ruling" section) — an incompatible topology change
(§3.4's "the floor is the region" exception) carried WITHOUT bumping the document
`version:` integer (§3.2, unchanged at `1`), per the settled additive model
(rpg-project#175) and CLAUDE.md's proto-versioning guidance that a version bump is
reserved for changes too large to move every consumer in one sitting — which this
is not, given the discriminator below.

1. `spec:` is an optional, decode-known string field (§4.1's field table). Legal
   values: `"0.3"`, `"0.4"`. Omitted MUST behave identically to `spec: "0.3"`. Any
   other literal — including `"draft"` — MUST be rejected server-side as an
   unrecognized enum value (the same rejection class §4.10.1's `archetype` enum
   uses for an unsupported value).
2. `spec:`'s only validation effect is selecting the canvas-mode floor-derivation
   rule (§4.5.4). It is otherwise inert: room-chain mode MUST accept and ignore any
   legal `spec:` value with no effect on validation or compilation.
3. **`"draft"` is authoring-tool-local only, never a wire value.** An authoring
   tool's own in-memory document model MAY use `"draft"` (or any other local
   sentinel) as a badge/UI marker for "still exploring, not committed to either
   floor semantic yet" — mirroring the existing precedent of the local-only
   `version: 2` "target dialect" marker (TARGET-YAML.md: "the legacy concept marker
   is never sent to the real server"). The tool MUST resolve `"draft"` to a real
   value (`"0.3"` or `"0.4"`) before any `PutDungeon` call, the same
   strip-before-wire discipline `stripToV1Subset` already applies to every other
   target-dialect-only field. This document does not add server-side handling for
   `"draft"` — the server never sees it, by construction.
4. **Compatibility, both directions, during the transition:**
   - Every document persisted before this field existed has no `spec:` key at all.
     Under rule 1, that decodes as `spec: "0.3"` and behaves exactly as it always
     has — no migration, no re-save, no data change required.
   - A newly-authored document SHOULD declare `spec: "0.4"` explicitly once an
     authoring tool supports it, rather than relying on the implicit default —
     recommendation, not a MUST (README.md elaborates the reasoning).
   - Both floor semantics MUST be supported server-side for the duration of this
     transition window; this document does not set an end date for `spec: "0.3"`
     support, and doing so is explicitly flagged as a platform/Kirk ratification
     point in README.md, not decided here.
5. `spec:` is independent of `version:` (§3.2) and of the region model's own
   evolution (§4.10.5's extension seam) — a future spec level MAY add a third
   `spec:` value without implying anything about `version:`, and MAY extend
   `RegionDoc` without touching `spec:` at all. The two are orthogonal discriminator
   axes: `version:` gates the document DECODE shape; `spec:` gates canvas-mode floor
   SEMANTICS specifically.

**Acceptance criteria:**
- A document with no `spec:` key, or `spec: "0.3"`, produces byte-for-byte identical
  validation/compilation results to the equivalent `../v0.3/spec.md`-only document.
- A document with `spec: "0.4"` uses the region-union floor derivation (§4.5.4)
  and, in canvas mode, the envelope model (§4.5.12).
- A document with `spec: "draft"`, or any other unrecognized string, fails
  validation with the accepted vocabulary (`"0.3"`, `"0.4"`) named in the error.
- `spec:`'s value has zero effect on any room-chain-mode validation or compilation
  outcome.
