# Dungeon YAML Spec v0.4 — context and rationale

[`spec.md`](spec.md) is the normative **PROPOSED** delta over ratified v0.3. This
README explains the pragmatic v0.x transport cut, its evidence, and how it relates to
delivered v0.3. It does not add grammar.

## What v0.4 is

v0.4 is an authoring/transport shape for the next playable-scene increment, not a 2.0
ownership constitution. It defines fields, preservation/projection, applicability,
merge behavior where transport needs one, and cosmetic-versus-mechanical boundaries.
It deliberately does not decide asset-pipeline governance, future behavior vocabulary,
future provider internals or long-term platform ownership.

The four additive tranches are:

| Tranche | Pragmatic v0.4 shape |
|---|---|
| A — topology | canvas bounds versus explicit region-union floor, canonical mask/envelope, draft/run lifecycle |
| B — placement | optional verbatim `offset: [x,y,z]` |
| C — monster config | opaque optional `target`, `profile`, `params` plus exact-ref defaults |
| D — lighting | optional inherited normalized ambient override, absent when unauthored |

v0.3 Wave 0 and Wave 1 are both LIVE VERIFIED:
[Wave 0](https://github.com/KirkDiggler/rpg-project/issues/192#issuecomment-5206548237)
and
[Wave 1](https://github.com/KirkDiggler/rpg-project/issues/180#issuecomment-5219362342).
v0.4 remains PROPOSED and claims no implementation.

## Authority and three axes

v0.4 is honestly a normative **delta** over
[`../v0.3/spec.md`](../v0.3/spec.md). Ratified v0.3 governs every omitted construct.

Three axes stay separate:

1. **v0.4** — documentation/review cut;
2. **`version: 1`** — unchanged YAML decode version;
3. **`canvas.floor_source: bounds | regions`** — canvas topology only, omission
   resolving to legacy `bounds`.

There is no YAML `spec:` or `draft` marker. Placement offset, monster config, and
lighting do not depend on which floor-source mode is active.

## Topology remains the strong core

Kirk's direction remains:

> “I think the floor is the region and we can see what is inside the walls. I think
> the canvas is available space in the builder but only regions carry into the game.”

`floor_source: regions` compiles the deduplicated union of `regions[].cells`. Canvas
dimensions remain workspace/coordinate-legality bounds. Omission remains the v0.3
rectangle so existing content never silently shrinks.

### One mask and explicit envelope truth

The compiled canonical mask feeds validation, placement, pathing, target reachability,
start/PartyCap seating, walls, LoS, reveal/fog, encounter initialization, and
projection. Subsystems do not independently recreate a rectangle or region union.

Envelope wire reuses existing flat `FloorPlan.edges {from,to,kind}` pairs. Exactly one
envelope endpoint is floor and owns the edge; the other is the adjacent void
coordinate, including off-canvas coordinates. Pair order is nonsemantic. Runtime
attaches the edge only to the owning floor `HexRecord`. This handles outer boundary
and interior void without a new wire message or client derivation.

### Draft validity versus runnable validity

`validate_only` accepts structurally valid empty, too-small, and disconnected masks
and always projects their floor/envelope. `FloorPlan.entrance` is absent until an
anchor has a complete PartyCap seating envelope inside the same structural component;
omitted-start candidates use deterministic canonical order.

Non-validate requires nonempty floor, present entrance/seats, and every floor cell in
the entrance component. The two-island specimen intentionally separates “can seat a
party on island A” from “the authored floor is runnable as one component.”

Union edits compile the complete candidate. Prior derived entrance/seats are discarded
and freshly recomputed; they are not dependencies and may move. A strict write fails
only when the candidate is not runnable or explicitly authored start/content/wall is
invalidated. Nothing moves to root or widens back to bounds.

### Two persistence lifecycles

Authored load/write recompiles YAML and atomically replaces registered content.
Encounter creation snapshots the exact mask/edges/config it starts with. Encounter
reload checks snapshot consistency only and never compares current authored YAML.
Future encounters see edits; running encounters retain their started world.

### Region equality

Same-region duplicate cells canonicalize. Between regions, equal or partial overlap
rejects. That includes two empty regions: one empty region is legal, but two empties
are equal sets and invalid.

## Placement offset: the deliberately small composition transport

The real-model work in web issue
[#728](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/728), PR
[#729](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/729), and its
[evidence](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/dev/docs/evidence/prop-composition-728.md)
showed that a builder sometimes needs a small authored translation and that copying a
complete old-asset transform is a poor replacement model. v0.4 takes only the useful
transport lesson:

```yaml
offset: [0.05, 0.0, 0.2]
```

The triple is three finite game-world `[x,y,z]` components relative to the canonical
placement origin. It stays world-axis-aligned, is preserved/served verbatim, and the
builder/game apply the same values. Existing ref/at/facing/mount applicability is
unchanged.

Offset is cosmetic. It cannot change owning cell, blockers, pathing, range,
targetability, LoS, visibility, fog, or interaction identity. A client editing `ref`
may retain, change, or remove offset in the complete submitted document; the server
has no replacement command or implicit policy.

That is the whole v0.4 composition contract. There is no anchor object, server
replacement/stable placement ID, asset metadata, or raw matrix on the wire.

## Monster config: opaque messenger fields

Monster room/canvas placements and boss may carry:

```yaml
target: wounded
profile: cautious
params:
  retreat_threshold: 0
  announce: false
```

`target` and `profile` are nonempty opaque strings. `params` maps nonempty keys to
JSON scalar bool/string/finite number. Null/list/object values reject. Zero, false,
and empty-string param values remain present. Props reject all three fields.

Exact-ref monster defaults use the same shape. Transport merge is deliberately small:
constructor/base → exact-ref default → placement/boss. Later target/profile replaces
when present; params merge key-wise with later keys winning. Omitted/empty params
contribute no keys and clear nothing.

The accepted target/profile strings, param keys, and value constraints are defined and
validated by the applicable monster provider, not by v0.4. The compiler passes opaque
values into compile/spawn config and reports rejection at source paths. The API merely
carries fields. v0.4 does not enumerate strategies/profiles/knobs, define ranges,
require a discovery protocol, special typed authoring union, or resolved behavior
preview.

Authoring projection echoes source path/ref/absolute cell plus authored target/profile/
params. Spawn/encounter data receives the merged config. Mutable runtime mode, memory,
perception, counters, chosen entity target, path, and other observations stay out of
YAML.

## Lighting: optional authored override

Document/region `lighting.ambient` is an absolute normalized `[0,1]` renderer-intensity
override. Inheritance selects the nearest authored value walking innermost → outward →
root. If no scope authors one, effective override remains absent and the current
renderer/theme baseline remains in force; different existing surfaces need not share
a numeric default.

Authoring projection echoes raw presence and optional inherited effective override.
Authorized visible/remembered runtime hexes carry the optional effective override only
where one exists. Hidden lighting remains absent.

Lighting is render-only. It cannot affect floor, walls, LoS, senses, visibility,
reveal, fog authorization, target selection, or pathing.

## Normal capability behavior: validate, do not downgrade

No named capability-key protocol is needed. The builder submits the exact candidate to
`AuthoringService.PutDungeon(validate_only=true)`. Unsupported fields or values fail
through normal source-path errors; success returns the specified projection.

The client does not strip unsupported fields, substitute bounds floor, calculate
opaque monster behavior, or erase offset/lighting to make a document pass. It either
submits the exact authored semantics or hard-stops.

## Client-local boundary

`wallLines`, continuous footprints, fractional coverage, draft/raw masks, matrices/
world transforms, temporary IDs, selection/gizmo state, snap previews, and uncommitted
nudges remain client-local. A builder may compile them losslessly into canonical
region cells, `walls:`, or committed `offset` before validation.

The platform accepts canonical YAML and must not normatively cite a particular
TypeScript implementation. Fractional coverage is never server standability. If local
state cannot compile losslessly, the client stops instead of approximating it.

## Reconciliation with delivered v0.3

[`../../design.md`](../../design.md) and [`../../plan.md`](../../plan.md) remain the
delivered v0.3 Wave 0/Wave 1 authority. Their rectangle floor, semantic-only region,
unpainted root, and nonmutating-region statements remain correct under omitted/
`floor_source: bounds`. Their “no lighting/geometry changes in #180” boundary remains
true because #180 is complete.

If ratified, explicit `floor_source: regions` narrowly changes those topology facts:
region union becomes floor, union edits revalidate authored content, and unpainted
cells are void. This scopes the conflict without rewriting completed delivery history.

## Review state

The prior non-shape “ratification gates” were governance rather than ambiguities in
the v0.4 field/transport shape, and are removed. This document remains **PROPOSED** until a bounded consistency review confirms
the simplified messenger shapes and topology rules.

**Genuine remaining spec-shape questions:** none currently identified. Review may
still find one; resolve any concrete ambiguity in PR #203 before changing the status.
Do not flip to RATIFIED merely because the artificial gates are gone.

## Acceptance pointers

Normative specimens are in
[`spec.md` §9](spec.md#9-cross-layer-acceptance-specimens):

- irregular floor with interior void and one-sided pair envelope;
- too-small draft and two islands;
- placement offset preserved/changed across a client ref edit;
- opaque behavior default/placement/boss merge with scalar presence; and
- ambient override inheritance with present/absent and fog/mechanical assertions.

Owning tracking issue: [#206](https://github.com/KirkDiggler/rpg-project/issues/206).
PR #203 remains the single review/tracking surface through downstream implementation.
No implementation issues are created by this documentation revision.

## Pointers

- [`../v0.3/spec.md`](../v0.3/spec.md) and
  [`../v0.3/README.md`](../v0.3/README.md) — ratified baseline.
- [`../../design.md`](../../design.md) and [`../../plan.md`](../../plan.md) — delivered
  v0.3 scope.
- [PR #203](https://github.com/KirkDiggler/rpg-project/pull/203) — proposed v0.4 review
  and tracking surface.
- [PR #202 platform handoff](https://github.com/KirkDiggler/rpg-project/pull/202#issuecomment-5228001955)
  — behavior context; v0.4 intentionally keeps the values opaque.
- [Web #728](https://github.com/KirkDiggler/rpg-dnd5e-web/issues/728),
  [PR #729](https://github.com/KirkDiggler/rpg-dnd5e-web/pull/729), and
  [evidence](https://github.com/KirkDiggler/rpg-dnd5e-web/blob/dev/docs/evidence/prop-composition-728.md)
  — real-model evidence motivating a small offset transport, not a governance model.
